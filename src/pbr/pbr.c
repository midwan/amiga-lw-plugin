/*
 * PBR.C -- PBR-lite Shader Plugin for LightWave 3D
 *
 * Combines Fresnel reflection, roughness (normal perturbation),
 * ambient occlusion (ray-based), and metallic mode into a single
 * physically-based shader for LightWave 5.x on AmigaOS.
 *
 * Uses AllocMem/FreeMem and custom helpers — no libnix runtime.
 */

#include <splug.h>
#include <lwran.h>
#include <lwpanel.h>
#include <lwmath.h>

#include <string.h>

#include <proto/exec.h>
#include <exec/memory.h>

extern struct ExecBase *SysBase;
extern double sqrt(double);

/* ----------------------------------------------------------------
 * Memory helpers
 * ---------------------------------------------------------------- */

static void *
plugin_alloc(unsigned long size)
{
	unsigned long *p;
	p = (unsigned long *)AllocMem(size + 4, MEMF_PUBLIC | MEMF_CLEAR);
	if (!p) return 0;
	*p = size + 4;
	return (void *)(p + 1);
}

static void
plugin_free(void *ptr)
{
	unsigned long *p;
	if (!ptr) return;
	p = ((unsigned long *)ptr) - 1;
	FreeMem(p, *p);
}

/* ----------------------------------------------------------------
 * Integer/string helpers
 * ---------------------------------------------------------------- */

static void
int_to_str(int val, char *buf, int buflen)
{
	char tmp[12];
	int  i = 0, neg = 0, len;

	if (val < 0) { neg = 1; val = -val; }
	if (val == 0) { tmp[i++] = '0'; }
	else {
		while (val > 0 && i < 11) {
			tmp[i++] = (char)('0' + (val % 10));
			val /= 10;
		}
	}
	len = neg + i;
	if (len >= buflen) len = buflen - 1;
	if (neg) buf[0] = '-';
	{
		int j;
		for (j = 0; j < i && (neg + j) < buflen - 1; j++)
			buf[neg + j] = tmp[i - 1 - j];
	}
	buf[len] = '\0';
}


/* ----------------------------------------------------------------
 * Math helpers
 * ---------------------------------------------------------------- */

static double
pow_int(double base, int exp)
{
	double result = 1.0;
	int i;
	if (exp < 0) return 0.0;
	for (i = 0; i < exp; i++)
		result *= base;
	return result;
}

static void
vec_normalize(double v[3])
{
	double len = sqrt(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
	if (len > 0.00001) {
		v[0] /= len;
		v[1] /= len;
		v[2] /= len;
	}
}

/*
 * Deterministic 3D hash for roughness perturbation.
 * Returns a value in -1.0 to 1.0 range.
 */
static double
hash3d(double x, double y, double z, unsigned int seed)
{
	unsigned int ix = (unsigned int)((x + 1000.0) * 731.0) & 0xFFFFu;
	unsigned int iy = (unsigned int)((y + 1000.0) * 541.0) & 0xFFFFu;
	unsigned int iz = (unsigned int)((z + 1000.0) * 379.0) & 0xFFFFu;
	unsigned int h = ix * 73856093u ^ iy * 19349669u ^ iz * 83492791u ^ seed;
	h = (h >> 13) ^ h;
	h = h * (h * 15731u + 789221u) + 1376312589u;
	return ((double)(h & 0x7FFFu) / (double)0x3FFFu) - 1.0;
}

/* ----------------------------------------------------------------
 * Types
 * ---------------------------------------------------------------- */

typedef struct {
	/* Fresnel */
	double ior;
	int    reflPower;
	int    affectMirror;
	int    affectTrans;
	int    affectDiffuse;
	int    diffPower;
	double f0;

	/* Roughness */
	int    roughEnabled;
	int    roughAmount;      /* 0-100, used as percentage */

	/* Ambient Occlusion */
	int    aoEnabled;
	int    aoSamples;        /* 4, 8, or 16 */
	int    aoRadius;         /* stored as radius * 100 (centimeters) */
	int    aoStrength;       /* 0-100 */

	/* Metallic */
	int    metallic;         /* 0-100 intensity */

	/* Specular */
	int    affectSpecular;

	/* Blurred Reflections */
	int    blurReflEnabled;
	int    blurReflSamples;  /* 4, 8, or 16 */
	int    blurReflAmount;   /* 0-100 cone spread */

	/* Environment Sampling */
	int    envEnabled;
	int    envSamples;       /* 4, 8, or 16 */
	int    envStrength;      /* 0-100 */
} PBRInst;

/* ----------------------------------------------------------------
 * Globals
 * ---------------------------------------------------------------- */

static MessageFuncs *msg;

static double pow5_lut[101];
static int pow5_ready = 0;

/* ----------------------------------------------------------------
 * Precompute F0 from IOR
 * ---------------------------------------------------------------- */

static void
compute_f0(PBRInst *inst)
{
	double r;
	if (inst->ior < 1.0) inst->ior = 1.0;
	r = (inst->ior - 1.0) / (inst->ior + 1.0);
	inst->f0 = r * r;
}

/* ----------------------------------------------------------------
 * AO hemisphere sample directions (pre-normalized)
 * 6 axis + 8 diagonals + 2 extras = 16 total
 * ---------------------------------------------------------------- */

static const double ao_dirs[16][3] = {
	{ 0.000,  1.000,  0.000},
	{ 1.000,  0.000,  0.000},
	{-1.000,  0.000,  0.000},
	{ 0.000,  0.000,  1.000},
	{ 0.000,  0.000, -1.000},
	{ 0.000, -1.000,  0.000},
	{ 0.577,  0.577,  0.577},
	{-0.577,  0.577,  0.577},
	{ 0.577, -0.577,  0.577},
	{ 0.577,  0.577, -0.577},
	{-0.577, -0.577,  0.577},
	{-0.577,  0.577, -0.577},
	{ 0.577, -0.577, -0.577},
	{-0.577, -0.577, -0.577},
	{ 0.707,  0.707,  0.000},
	{ 0.000,  0.707,  0.707}
};

static const double hemi_dirs[16][3] = {
	{ 0.000,  1.000,  0.000},
	{ 0.577,  0.577,  0.577},
	{-0.577,  0.577,  0.577},
	{ 0.577,  0.577, -0.577},
	{-0.577,  0.577, -0.577},
	{ 0.707,  0.707,  0.000},
	{-0.707,  0.707,  0.000},
	{ 0.000,  0.707,  0.707},
	{ 0.000,  0.707, -0.707},
	{ 0.383,  0.924,  0.000},
	{-0.383,  0.924,  0.000},
	{ 0.000,  0.924,  0.383},
	{ 0.000,  0.924, -0.383},
	{ 0.408,  0.816,  0.408},
	{-0.408,  0.816,  0.408},
	{ 0.408,  0.816, -0.408}
};

/* ----------------------------------------------------------------
 * Handler callbacks
 * ---------------------------------------------------------------- */

XCALL_(static LWInstance)
Create(LWError *err)
{
	PBRInst *inst;
	XCALL_INIT;

	inst = (PBRInst *)plugin_alloc(sizeof(PBRInst));
	if (!inst) return 0;

	inst->ior           = 1.5;
	inst->reflPower     = 5;
	inst->affectMirror  = 1;
	inst->affectTrans   = 1;
	inst->affectDiffuse = 1;
	inst->diffPower     = 5;
	inst->roughEnabled  = 0;
	inst->roughAmount   = 20;
	inst->aoEnabled     = 0;
	inst->aoSamples     = 4;
	inst->aoRadius      = 100;
	inst->aoStrength    = 50;
	inst->metallic      = 0;
	inst->affectSpecular = 0;
	inst->blurReflEnabled = 0;
	inst->blurReflSamples = 4;
	inst->blurReflAmount  = 30;
	inst->envEnabled    = 0;
	inst->envSamples    = 4;
	inst->envStrength   = 50;
	compute_f0(inst);

	if (!pow5_ready) {
		int j;
		for (j = 0; j <= 100; j++) {
			double t = j / 100.0;
			pow5_lut[j] = t * t * t * t * t;
		}
		pow5_ready = 1;
	}

	return inst;
}

XCALL_(static void)
Destroy(PBRInst *inst)
{
	XCALL_INIT;
	if (inst) plugin_free(inst);
}

XCALL_(static LWError)
Copy(PBRInst *from, PBRInst *to)
{
	XCALL_INIT;
	*to = *from;
	return 0;
}

static const char *
parse_int(const char *s, int *val)
{
	int neg = 0;
	*val = 0;
	while (*s == ' ') s++;
	if (*s == '-') { neg = 1; s++; }
	while (*s >= '0' && *s <= '9') {
		*val = *val * 10 + (*s - '0');
		s++;
	}
	if (neg) *val = -*val;
	return s;
}

static void
append_int(char *buf, int *pos, int val)
{
	char tmp[12];
	int i;
	int_to_str(val, tmp, 12);
	if (*pos > 0) buf[(*pos)++] = ' ';
	for (i = 0; tmp[i]; i++)
		buf[(*pos)++] = tmp[i];
	buf[*pos] = '\0';
}

XCALL_(static LWError)
Load(PBRInst *inst, const LWLoadState *ls)
{
	char buf[128];
	const char *p;
	int v;
	XCALL_INIT;

	buf[0] = '\0';
	(*ls->read)(ls->readData, buf, 127);
	buf[127] = '\0';
	if (buf[0] == '\0') {
		(*ls->read)(ls->readData, buf, 127);
		buf[127] = '\0';
	}
	if (!buf[0]) return 0;

	p = buf;
	p = parse_int(p, &v); inst->ior = v / 1000.0;
	p = parse_int(p, &v); inst->reflPower = v;
	p = parse_int(p, &v); inst->affectMirror = v;
	p = parse_int(p, &v); inst->affectTrans = v;
	p = parse_int(p, &v); inst->affectDiffuse = v;
	p = parse_int(p, &v); inst->diffPower = v;
	p = parse_int(p, &v); inst->roughEnabled = v;
	p = parse_int(p, &v); inst->roughAmount = v;
	p = parse_int(p, &v); inst->aoEnabled = v;
	p = parse_int(p, &v); inst->aoSamples = v;
	p = parse_int(p, &v); inst->aoRadius = v;
	p = parse_int(p, &v); inst->aoStrength = v;
	p = parse_int(p, &v); inst->metallic = v;
	if (inst->metallic == 1) inst->metallic = 100;
	p = parse_int(p, &v); inst->affectSpecular = v;
	p = parse_int(p, &v); inst->blurReflEnabled = v;
	p = parse_int(p, &v); inst->blurReflSamples = v;
	p = parse_int(p, &v); inst->blurReflAmount = v;
	p = parse_int(p, &v); inst->envEnabled = v;
	p = parse_int(p, &v); inst->envSamples = v;
	p = parse_int(p, &v); inst->envStrength = v;

	compute_f0(inst);
	return 0;
}

XCALL_(static LWError)
Save(PBRInst *inst, const LWSaveState *ss)
{
	char buf[128];
	int pos = 0;
	XCALL_INIT;

	append_int(buf, &pos, (int)(inst->ior * 1000.0));
	append_int(buf, &pos, inst->reflPower);
	append_int(buf, &pos, inst->affectMirror);
	append_int(buf, &pos, inst->affectTrans);
	append_int(buf, &pos, inst->affectDiffuse);
	append_int(buf, &pos, inst->diffPower);
	append_int(buf, &pos, inst->roughEnabled);
	append_int(buf, &pos, inst->roughAmount);
	append_int(buf, &pos, inst->aoEnabled);
	append_int(buf, &pos, inst->aoSamples);
	append_int(buf, &pos, inst->aoRadius);
	append_int(buf, &pos, inst->aoStrength);
	append_int(buf, &pos, inst->metallic);
	append_int(buf, &pos, inst->affectSpecular);
	append_int(buf, &pos, inst->blurReflEnabled);
	append_int(buf, &pos, inst->blurReflSamples);
	append_int(buf, &pos, inst->blurReflAmount);
	append_int(buf, &pos, inst->envEnabled);
	append_int(buf, &pos, inst->envSamples);
	append_int(buf, &pos, inst->envStrength);

	(*ss->write)(ss->writeData, buf, pos);

	return 0;
}

XCALL_(static LWError)
Init(PBRInst *inst)
{
	XCALL_INIT;
	compute_f0(inst);
	return 0;
}

XCALL_(static void)
Cleanup(PBRInst *inst) { XCALL_INIT; }

XCALL_(static LWError)
NewTime(PBRInst *inst, LWFrame f, LWTime t)
{
	XCALL_INIT;
	return 0;
}

XCALL_(static unsigned int)
Flags(PBRInst *inst)
{
	unsigned int f = 0;
	XCALL_INIT;

	f |= LWSHF_NORMAL;

	if (inst->metallic > 0)
		f |= LWSHF_MIRROR | LWSHF_DIFFUSE | LWSHF_SPECULAR;

	if (inst->aoEnabled)
		f |= LWSHF_DIFFUSE | LWSHF_LUMINOUS | LWSHF_RAYTRACE;

	if (inst->blurReflEnabled)
		f |= LWSHF_COLOR | LWSHF_MIRROR | LWSHF_RAYTRACE;

	if (inst->envEnabled)
		f |= LWSHF_COLOR | LWSHF_LUMINOUS | LWSHF_RAYTRACE;

	return f;
}

XCALL_(static void)
Evaluate(PBRInst *inst, ShaderAccess *sa)
{
	double cosAngle, oneMinusCos, fresnel;

	XCALL_INIT;

	cosAngle = sa->cosine;
	if (cosAngle < 0.0) cosAngle = -cosAngle;
	if (cosAngle > 1.0) cosAngle = 1.0;
	oneMinusCos = 1.0 - cosAngle;

	/* --- Roughness: perturb surface normal --- */
	if (inst->roughEnabled && inst->roughAmount > 0) {
		double scale = inst->roughAmount / 1000.0;
		double nx = hash3d(sa->oPos[0], sa->oPos[1], sa->oPos[2], 0u) * scale;
		double ny = hash3d(sa->oPos[0], sa->oPos[1], sa->oPos[2], 7919u) * scale;
		double nz = hash3d(sa->oPos[0], sa->oPos[1], sa->oPos[2], 15737u) * scale;

		sa->wNorm[0] += nx;
		sa->wNorm[1] += ny;
		sa->wNorm[2] += nz;
		vec_normalize(sa->wNorm);
	}

	/* --- Metallic: blend between dielectric and metallic behavior --- */
	if (inst->metallic > 0) {
		double met = inst->metallic / 100.0;
		int lutIdx = (int)(oneMinusCos * 100.0);
		if (lutIdx < 0) lutIdx = 0;
		if (lutIdx > 100) lutIdx = 100;
		fresnel = inst->f0 + (1.0 - inst->f0) * pow5_lut[lutIdx];
		if (fresnel > 1.0) fresnel = 1.0;

		sa->mirror = sa->mirror + (1.0 - sa->mirror) * fresnel * met;
		sa->diffuse *= 1.0 - 0.95 * met;
		sa->specular = sa->specular + (1.0 - sa->specular) * fresnel * met;
	}

	/* --- Ambient Occlusion: cast rays in hemisphere --- */
	if (inst->aoEnabled && inst->aoSamples > 0 && sa->rayCast) {
		double aoRadius = inst->aoRadius / 100.0;
		double aoStr = inst->aoStrength / 100.0;
		int    nSamples = inst->aoSamples;
		int    hits = 0;
		int    tested = 0;
		int    i;
		double pos[3], dir[3], dot, dist;

		/* Offset ray origin slightly along normal to avoid self-hit */
		pos[0] = sa->wPos[0] + sa->wNorm[0] * 0.001;
		pos[1] = sa->wPos[1] + sa->wNorm[1] * 0.001;
		pos[2] = sa->wPos[2] + sa->wNorm[2] * 0.001;

		if (nSamples > 16) nSamples = 16;

		for (i = 0; i < nSamples; i++) {
			dir[0] = ao_dirs[i][0];
			dir[1] = ao_dirs[i][1];
			dir[2] = ao_dirs[i][2];

			/* Only cast rays into the hemisphere facing the normal */
			dot = dir[0]*sa->wNorm[0] + dir[1]*sa->wNorm[1]
			    + dir[2]*sa->wNorm[2];
			if (dot <= 0.0) {
				/* Flip direction to face the right hemisphere */
				dir[0] = -dir[0];
				dir[1] = -dir[1];
				dir[2] = -dir[2];
			}

			dist = (*sa->rayCast)(pos, dir);
			tested++;

			if (dist > 0.0 && dist < aoRadius)
				hits++;
		}

		if (tested > 0) {
			double occlusion = (double)hits / (double)tested;
			double aoFactor = 1.0 - (occlusion * aoStr);
			if (aoFactor < 0.0) aoFactor = 0.0;

			sa->diffuse *= aoFactor;
			sa->luminous *= aoFactor;
		}
	}

	/* --- Blurred Reflections: cone-traced rays around reflection dir --- */
	if (inst->blurReflEnabled && inst->blurReflSamples > 0
	    && sa->rayTrace && sa->mirror > 0.001) {
		double viewDir[3], reflDir[3], dot_vn;
		double spread, px, py, pz;
		double dir[3], col[3], pos[3];
		double accR = 0.0, accG = 0.0, accB = 0.0;
		int    nSamp = inst->blurReflSamples;
		int    validSamples = 0;
		int    i;

		if (nSamp > 16) nSamp = 16;
		spread = inst->blurReflAmount / 200.0;
		if (spread < 0.001) spread = 0.001;

		viewDir[0] = sa->wPos[0] - sa->raySource[0];
		viewDir[1] = sa->wPos[1] - sa->raySource[1];
		viewDir[2] = sa->wPos[2] - sa->raySource[2];
		vec_normalize(viewDir);

		dot_vn = viewDir[0]*sa->wNorm[0] + viewDir[1]*sa->wNorm[1]
		       + viewDir[2]*sa->wNorm[2];
		reflDir[0] = viewDir[0] - 2.0 * dot_vn * sa->wNorm[0];
		reflDir[1] = viewDir[1] - 2.0 * dot_vn * sa->wNorm[1];
		reflDir[2] = viewDir[2] - 2.0 * dot_vn * sa->wNorm[2];
		vec_normalize(reflDir);

		pos[0] = sa->wPos[0] + sa->wNorm[0] * 0.001;
		pos[1] = sa->wPos[1] + sa->wNorm[1] * 0.001;
		pos[2] = sa->wPos[2] + sa->wNorm[2] * 0.001;

		for (i = 0; i < nSamp; i++) {
			px = hash3d(sa->oPos[0], sa->oPos[1], sa->oPos[2],
			            50000u + (unsigned int)i * 7919u) * spread;
			py = hash3d(sa->oPos[0], sa->oPos[1], sa->oPos[2],
			            60000u + (unsigned int)i * 6271u) * spread;
			pz = hash3d(sa->oPos[0], sa->oPos[1], sa->oPos[2],
			            70000u + (unsigned int)i * 5381u) * spread;

			dir[0] = reflDir[0] + px;
			dir[1] = reflDir[1] + py;
			dir[2] = reflDir[2] + pz;
			vec_normalize(dir);

			col[0] = col[1] = col[2] = 0.0;
			(*sa->rayTrace)(pos, dir, col);
			accR += col[0];
			accG += col[1];
			accB += col[2];
			validSamples++;
		}

		if (validSamples > 0) {
			double invN = 1.0 / (double)validSamples;
			double mirrorWt = sa->mirror;
			sa->color[0] = sa->color[0] * (1.0 - mirrorWt)
			             + accR * invN * mirrorWt;
			sa->color[1] = sa->color[1] * (1.0 - mirrorWt)
			             + accG * invN * mirrorWt;
			sa->color[2] = sa->color[2] * (1.0 - mirrorWt)
			             + accB * invN * mirrorWt;
			sa->mirror = 0.0;
		}
	}

	/* --- Environment Sampling: hemisphere rayTrace for indirect light --- */
	if (inst->envEnabled && inst->envSamples > 0 && sa->rayTrace) {
		double envStr = inst->envStrength / 100.0;
		int    nSamp = inst->envSamples;
		double accR = 0.0, accG = 0.0, accB = 0.0;
		double totalWeight = 0.0;
		int    i;
		double pos[3];

		if (nSamp > 16) nSamp = 16;

		pos[0] = sa->wPos[0] + sa->wNorm[0] * 0.001;
		pos[1] = sa->wPos[1] + sa->wNorm[1] * 0.001;
		pos[2] = sa->wPos[2] + sa->wNorm[2] * 0.001;

		for (i = 0; i < nSamp; i++) {
			double dir[3], col[3], dot, dist;

			dir[0] = hemi_dirs[i][0];
			dir[1] = hemi_dirs[i][1];
			dir[2] = hemi_dirs[i][2];

			dot = dir[0]*sa->wNorm[0] + dir[1]*sa->wNorm[1]
			    + dir[2]*sa->wNorm[2];
			if (dot <= 0.0) {
				dir[0] = -dir[0];
				dir[1] = -dir[1];
				dir[2] = -dir[2];
				dot = -dot;
			}

			col[0] = col[1] = col[2] = 0.0;
			dist = (*sa->rayTrace)(pos, dir, col);
			(void)dist;

			accR += col[0] * dot;
			accG += col[1] * dot;
			accB += col[2] * dot;
			totalWeight += dot;
		}

		if (totalWeight > 0.0) {
			double invW = envStr / totalWeight;
			sa->color[0] += accR * invW;
			sa->color[1] += accG * invW;
			sa->color[2] += accB * invW;
			if (sa->color[0] > 1.0) sa->color[0] = 1.0;
			if (sa->color[1] > 1.0) sa->color[1] = 1.0;
			if (sa->color[2] > 1.0) sa->color[2] = 1.0;
			sa->luminous += envStr * 0.3;
			if (sa->luminous > 1.0) sa->luminous = 1.0;
		}
	}
}

/* ----------------------------------------------------------------
 * Interface
 * ---------------------------------------------------------------- */

static const char *aoSampleItems[] = { "Off", "2", "4", "8", 0 };
static int aoSampleValues[] = { 0, 2, 4, 8 };

XCALL_(static int)
Interface(
	long       version,
	GlobalFunc *global,
	PBRInst    *inst,
	void       *serverData)
{
	LWPanelFuncs *panl;
	LWPanelID     pan;
	LWControl    *ctlIOR, *ctlMetallic;
	LWControl    *ctlRoughEn, *ctlRoughAmt;
	LWControl    *ctlAOSamp, *ctlAORadius, *ctlAOStr;
	LWControl    *ctlBlurSamp, *ctlBlurAmt;
	LWControl    *ctlEnvSamp, *ctlEnvStr;
	int           aoIdx, blurIdx, envIdx;
	char          infoBuf[80];

	XCALL_INIT;
	if (version != 1)
		return AFUNC_BADVERSION;

	msg = (MessageFuncs *)(*global)("Info Messages", GFUSE_TRANSIENT);
	panl = (LWPanelFuncs *)(*global)(PANEL_SERVICES_NAME, GFUSE_TRANSIENT);

	if (panl) {
		static LWPanControlDesc desc;
		static LWValue ival = {LWT_INTEGER};
		static LWValue fval = {LWT_FLOAT};
		(void)fval;

		pan = PAN_CREATE(panl, "PBR v" PLUGIN_VERSION " (c) D. Panokostas");
		if (!pan) goto fallback;

		ctlIOR      = FLOAT_CTL(panl, pan, "IOR");
		ctlMetallic = SLIDER_CTL(panl, pan, "Metallic", 150, 0, 100);
		ctlRoughEn  = BOOL_CTL(panl, pan, "Roughness");
		ctlRoughAmt = INT_CTL(panl, pan, "");
		ctlAOSamp   = POPUP_CTL(panl, pan, "AO", aoSampleItems);
		ctlAORadius = FLOAT_CTL(panl, pan, "Radius");
		ctlAOStr    = SLIDER_CTL(panl, pan, "AO Strength", 150, 0, 100);
		ctlBlurSamp = POPUP_CTL(panl, pan, "Blur Refl", aoSampleItems);
		ctlBlurAmt  = INT_CTL(panl, pan, "Spread");
		ctlEnvSamp  = POPUP_CTL(panl, pan, "Env Light", aoSampleItems);
		ctlEnvStr   = INT_CTL(panl, pan, "Strength");

		{
			int rowH, halfX, cy, cx, shift;
			rowH = CON_H(ctlRoughEn);
			halfX = PAN_GETW(panl, pan) / 2;
			shift = 0;

			cy = CON_Y(ctlRoughEn);
			MOVE_CON(ctlRoughAmt, halfX, cy);
			shift += rowH;

			cy = CON_Y(ctlAOSamp); cx = CON_X(ctlAOSamp);
			MOVE_CON(ctlAOSamp, cx, cy - shift);
			cy = CON_Y(ctlAOSamp);
			MOVE_CON(ctlAORadius, halfX, cy);
			shift += rowH;

			cy = CON_Y(ctlAOStr); cx = CON_X(ctlAOStr);
			MOVE_CON(ctlAOStr, cx, cy - shift);

			cy = CON_Y(ctlBlurSamp); cx = CON_X(ctlBlurSamp);
			MOVE_CON(ctlBlurSamp, cx, cy - shift);
			cy = CON_Y(ctlBlurSamp);
			MOVE_CON(ctlBlurAmt, halfX, cy);
			shift += rowH;

			cy = CON_Y(ctlEnvSamp); cx = CON_X(ctlEnvSamp);
			MOVE_CON(ctlEnvSamp, cx, cy - shift);
			cy = CON_Y(ctlEnvSamp);
			MOVE_CON(ctlEnvStr, halfX, cy);

			cy = CON_Y(ctlEnvStr);
			PAN_SETH(panl, pan, cy + 3 * rowH);
		}

		/* Set values */
		SET_FLOAT(ctlIOR, inst->ior);
		SET_INT(ctlMetallic, inst->metallic);
		SET_INT(ctlRoughEn, inst->roughEnabled);
		SET_INT(ctlRoughAmt, inst->roughAmount);
		aoIdx = inst->aoEnabled
		      ? ((inst->aoSamples <= 2) ? 1 : (inst->aoSamples <= 4) ? 2 : 3)
		      : 0;
		SET_INT(ctlAOSamp, aoIdx);
		{
			double r = inst->aoRadius / 100.0;
			SET_FLOAT(ctlAORadius, r);
		}
		SET_INT(ctlAOStr, inst->aoStrength);
		blurIdx = inst->blurReflEnabled
		        ? ((inst->blurReflSamples <= 2) ? 1 : (inst->blurReflSamples <= 4) ? 2 : 3)
		        : 0;
		SET_INT(ctlBlurSamp, blurIdx);
		SET_INT(ctlBlurAmt, inst->blurReflAmount);
		envIdx = inst->envEnabled
		       ? ((inst->envSamples <= 2) ? 1 : (inst->envSamples <= 4) ? 2 : 3)
		       : 0;
		SET_INT(ctlEnvSamp, envIdx);
		SET_INT(ctlEnvStr, inst->envStrength);

		if ((*panl->open)(pan, PANF_BLOCKING | PANF_CANCEL)) {
			GET_FLOAT(ctlIOR, inst->ior);
			GET_INT(ctlMetallic, inst->metallic);
			GET_INT(ctlRoughEn, inst->roughEnabled);
			GET_INT(ctlRoughAmt, inst->roughAmount);
			GET_INT(ctlAOSamp, aoIdx);
			inst->aoEnabled = (aoIdx > 0) ? 1 : 0;
			inst->aoSamples = (aoIdx > 0 && aoIdx < 4) ? aoSampleValues[aoIdx] : 8;
			{
				double r;
				GET_FLOAT(ctlAORadius, r);
				inst->aoRadius = (int)(r * 100.0);
			}
			GET_INT(ctlAOStr, inst->aoStrength);
			GET_INT(ctlBlurSamp, blurIdx);
			inst->blurReflEnabled = (blurIdx > 0) ? 1 : 0;
			inst->blurReflSamples = (blurIdx > 0 && blurIdx < 4) ? aoSampleValues[blurIdx] : 8;
			GET_INT(ctlBlurAmt, inst->blurReflAmount);
			GET_INT(ctlEnvSamp, envIdx);
			inst->envEnabled = (envIdx > 0) ? 1 : 0;
			inst->envSamples = (envIdx > 0 && envIdx < 4) ? aoSampleValues[envIdx] : 8;
			GET_INT(ctlEnvStr, inst->envStrength);

			/* Clamp */
			if (inst->ior < 1.0) inst->ior = 1.0;
			if (inst->ior > 5.0) inst->ior = 5.0;
			if (inst->reflPower < 1) inst->reflPower = 1;
			if (inst->reflPower > 10) inst->reflPower = 10;
			if (inst->diffPower < 1) inst->diffPower = 1;
			if (inst->diffPower > 10) inst->diffPower = 10;
			if (inst->roughAmount < 0) inst->roughAmount = 0;
			if (inst->roughAmount > 100) inst->roughAmount = 100;
			if (inst->aoRadius < 1) inst->aoRadius = 1;
			if (inst->aoStrength < 0) inst->aoStrength = 0;
			if (inst->aoStrength > 100) inst->aoStrength = 100;
			if (inst->blurReflAmount < 0) inst->blurReflAmount = 0;
			if (inst->blurReflAmount > 100) inst->blurReflAmount = 100;
			if (inst->envStrength < 0) inst->envStrength = 0;
			if (inst->envStrength > 100) inst->envStrength = 100;

			compute_f0(inst);
		}

		PAN_KILL(panl, pan);
		return AFUNC_OK;
	}

fallback:
	if (!msg)
		return AFUNC_BADGLOBAL;

	{
		char nb[12];
		int iorFixed = (int)(inst->ior * 100.0);
		strcpy(infoBuf, "IOR:");
		int_to_str(iorFixed / 100, nb, 12); strcat(infoBuf, nb);
		strcat(infoBuf, ".");
		int_to_str(iorFixed % 100, nb, 12);
		if (iorFixed % 100 < 10) strcat(infoBuf, "0");
		strcat(infoBuf, nb);
		if (inst->metallic) strcat(infoBuf, " Metal");
		if (inst->roughEnabled) {
			strcat(infoBuf, " Rough:");
			int_to_str(inst->roughAmount, nb, 12);
			strcat(infoBuf, nb);
		}
		if (inst->aoEnabled) strcat(infoBuf, " +AO");
		if (inst->blurReflEnabled) strcat(infoBuf, " +Blur");
		if (inst->envEnabled) strcat(infoBuf, " +Env");
	}
	(*msg->info)("PBR Shader", infoBuf);
	return AFUNC_OK;
}

/* ----------------------------------------------------------------
 * Activation
 * ---------------------------------------------------------------- */

XCALL_(int)
Activate(
	long         version,
	GlobalFunc  *global,
	void        *local,
	void        *serverData)
{
	ShaderHandler *h = (ShaderHandler *)local;
	XCALL_INIT;

	if (version < 1)
		return AFUNC_BADVERSION;

	h->create   = (void *)Create;
	h->destroy  = (void *)Destroy;
	h->load     = (void *)Load;
	h->save     = (void *)Save;
	h->copy     = (void *)Copy;
	h->init     = (void *)Init;
	h->cleanup  = (void *)Cleanup;
	h->newTime  = (void *)NewTime;
	h->evaluate = (void *)Evaluate;
	h->flags    = (void *)Flags;

	msg = (MessageFuncs *)(*global)("Info Messages", GFUSE_TRANSIENT);
	if (!msg)
		return AFUNC_BADGLOBAL;

	return AFUNC_OK;
}

/* ----------------------------------------------------------------
 * Server description
 * ---------------------------------------------------------------- */

ServerRecord ServerDesc[] = {
	{ "ShaderHandler",   "PBR",
	  (ActivateFunc *)Activate },
	{ "ShaderInterface", "PBR",
	  (ActivateFunc *)Interface },
	{ 0 }
};

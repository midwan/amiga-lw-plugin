/*
 * SOFTSHADOW.C -- Soft Shadow Shader Plugin for LightWave 3D
 *
 * Adds penumbra to raytrace shadows by multi-sampling light positions.
 * For each surface point in hard shadow, casts rays toward offset
 * positions around the light source. Partially visible lights produce
 * soft penumbra edges.
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

static int
str_to_int(const char *s)
{
	int val = 0, neg = 0;
	while (*s == ' ') s++;
	if (*s == '-') { neg = 1; s++; }
	while (*s >= '0' && *s <= '9') {
		val = val * 10 + (*s - '0');
		s++;
	}
	return neg ? -val : val;
}

/* ----------------------------------------------------------------
 * Math helpers
 * ---------------------------------------------------------------- */

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

static void
vec_cross(const double a[3], const double b[3], double out[3])
{
	out[0] = a[1]*b[2] - a[2]*b[1];
	out[1] = a[2]*b[0] - a[0]*b[2];
	out[2] = a[0]*b[1] - a[1]*b[0];
}

static unsigned int
hash3d_uint(double x, double y, double z, unsigned int seed)
{
	unsigned int ix = (unsigned int)((x + 1000.0) * 731.0) & 0xFFFFu;
	unsigned int iy = (unsigned int)((y + 1000.0) * 541.0) & 0xFFFFu;
	unsigned int iz = (unsigned int)((z + 1000.0) * 379.0) & 0xFFFFu;
	unsigned int h = ix * 73856093u ^ iy * 19349669u ^ iz * 83492791u ^ seed;
	h = (h >> 13) ^ h;
	h = h * (h * 15731u + 789221u) + 1376312589u;
	return h;
}

/* ----------------------------------------------------------------
 * Pre-computed unit circle offsets for light sampling (8 directions)
 * ---------------------------------------------------------------- */

static const double circle_offs[8][2] = {
	{ 1.000,  0.000},
	{ 0.707,  0.707},
	{ 0.000,  1.000},
	{-0.707,  0.707},
	{-1.000,  0.000},
	{-0.707, -0.707},
	{ 0.000, -1.000},
	{ 0.707, -0.707}
};

/* ----------------------------------------------------------------
 * Types
 * ---------------------------------------------------------------- */

typedef struct {
	int    samples;
	int    lightSize;   /* stored as size * 1000 */
	int    strength;    /* 0-100 */
	double curTime;
} SoftShadInst;

/* ----------------------------------------------------------------
 * Globals
 * ---------------------------------------------------------------- */

static MessageFuncs *msg;
static LWItemInfo   *itemInfo;
static LWLightInfo  *lightInfo;

/* ----------------------------------------------------------------
 * Handler callbacks
 * ---------------------------------------------------------------- */

XCALL_(static LWInstance)
Create(LWError *err)
{
	SoftShadInst *inst;
	XCALL_INIT;

	inst = (SoftShadInst *)plugin_alloc(sizeof(SoftShadInst));
	if (!inst) return 0;

	inst->samples   = 4;
	inst->lightSize = 500;
	inst->strength  = 80;
	inst->curTime   = 0.0;

	return inst;
}

XCALL_(static void)
Destroy(SoftShadInst *inst)
{
	XCALL_INIT;
	if (inst) plugin_free(inst);
}

XCALL_(static LWError)
Copy(SoftShadInst *from, SoftShadInst *to)
{
	XCALL_INIT;
	*to = *from;
	return 0;
}

XCALL_(static LWError)
Load(SoftShadInst *inst, const LWLoadState *ls)
{
	char buf[32];
	XCALL_INIT;

	buf[0] = '\0';
	(*ls->read)(ls->readData, buf, 32);
	if (buf[0] == '\0')
		(*ls->read)(ls->readData, buf, 32);
	if (buf[0]) inst->samples = str_to_int(buf);

	buf[0] = '\0'; (*ls->read)(ls->readData, buf, 32);
	if (buf[0]) inst->lightSize = str_to_int(buf);

	buf[0] = '\0'; (*ls->read)(ls->readData, buf, 32);
	if (buf[0]) inst->strength = str_to_int(buf);

	return 0;
}

XCALL_(static LWError)
Save(SoftShadInst *inst, const LWSaveState *ss)
{
	char buf[32];
	XCALL_INIT;

	int_to_str(inst->samples, buf, 32);
	(*ss->write)(ss->writeData, buf, strlen(buf));

	int_to_str(inst->lightSize, buf, 32);
	(*ss->write)(ss->writeData, buf, strlen(buf));

	int_to_str(inst->strength, buf, 32);
	(*ss->write)(ss->writeData, buf, strlen(buf));

	return 0;
}

XCALL_(static LWError)
Init(SoftShadInst *inst) { XCALL_INIT; return 0; }

XCALL_(static void)
Cleanup(SoftShadInst *inst) { XCALL_INIT; }

XCALL_(static LWError)
NewTime(SoftShadInst *inst, LWFrame f, LWTime t)
{
	XCALL_INIT;
	inst->curTime = t;
	return 0;
}

XCALL_(static unsigned int)
Flags(SoftShadInst *inst)
{
	XCALL_INIT;
	return LWSHF_DIFFUSE | LWSHF_LUMINOUS | LWSHF_RAYTRACE;
}

XCALL_(static void)
Evaluate(SoftShadInst *inst, ShaderAccess *sa)
{
	LWItemID  lightID;
	double    lightPos[3], toLight[3], dist;
	double    tangent1[3], tangent2[3], up[3];
	double    str, lsize;
	int       nSamp, i;

	XCALL_INIT;

	if (!itemInfo || !sa->rayCast || inst->samples < 1)
		return;

	str   = inst->strength / 100.0;
	lsize = inst->lightSize / 1000.0;
	nSamp = inst->samples;
	if (nSamp > 8) nSamp = 8;

	lightID = (*itemInfo->first)(LWI_LIGHT, LWITEM_NULL);

	while (lightID) {
		double lcolor[3], ldir[3];
		int    lit, blocked, tested;
		double pos[3];

		lit = (*sa->illuminate)(lightID, sa->wPos, ldir, lcolor);

		if (lit) {
			lightID = (*itemInfo->next)(lightID);
			continue;
		}

		if (lcolor[0] < 0.001 && lcolor[1] < 0.001 && lcolor[2] < 0.001) {
			lightID = (*itemInfo->next)(lightID);
			continue;
		}

		(*itemInfo->param)(lightID, LWIP_W_POSITION,
		                   inst->curTime, lightPos);

		toLight[0] = lightPos[0] - sa->wPos[0];
		toLight[1] = lightPos[1] - sa->wPos[1];
		toLight[2] = lightPos[2] - sa->wPos[2];
		dist = sqrt(toLight[0]*toLight[0] + toLight[1]*toLight[1]
		          + toLight[2]*toLight[2]);
		if (dist < 0.001) {
			lightID = (*itemInfo->next)(lightID);
			continue;
		}
		toLight[0] /= dist;
		toLight[1] /= dist;
		toLight[2] /= dist;

		up[0] = 0.0; up[1] = 1.0; up[2] = 0.0;
		if (toLight[1] > 0.99 || toLight[1] < -0.99) {
			up[0] = 1.0; up[1] = 0.0; up[2] = 0.0;
		}
		vec_cross(toLight, up, tangent1);
		vec_normalize(tangent1);
		vec_cross(toLight, tangent1, tangent2);

		pos[0] = sa->wPos[0] + sa->wNorm[0] * 0.001;
		pos[1] = sa->wPos[1] + sa->wNorm[1] * 0.001;
		pos[2] = sa->wPos[2] + sa->wNorm[2] * 0.001;

		blocked = 0;
		tested  = 0;

		for (i = 0; i < nSamp; i++) {
			double offx, offy, dir[3], hitDist;
			int    idx;

			idx = (int)(hash3d_uint(sa->oPos[0], sa->oPos[1],
			            sa->oPos[2], (unsigned int)i * 4919u) % 8u);
			offx = circle_offs[idx][0] * lsize;
			offy = circle_offs[idx][1] * lsize;

			dir[0] = toLight[0] + tangent1[0]*offx + tangent2[0]*offy;
			dir[1] = toLight[1] + tangent1[1]*offx + tangent2[1]*offy;
			dir[2] = toLight[2] + tangent1[2]*offx + tangent2[2]*offy;
			vec_normalize(dir);

			hitDist = (*sa->rayCast)(pos, dir);
			tested++;

			if (hitDist > 0.0 && hitDist < dist * 0.99)
				blocked++;
		}

		if (tested > 0 && blocked < tested) {
			double visible = (double)(tested - blocked) / (double)tested;
			double boost = visible * str;
			sa->diffuse  += boost * 0.7;
			sa->luminous += boost * 0.3;
			if (sa->diffuse > 1.0)  sa->diffuse = 1.0;
			if (sa->luminous > 1.0) sa->luminous = 1.0;
		}

		lightID = (*itemInfo->next)(lightID);
	}
}

/* ----------------------------------------------------------------
 * Interface
 * ---------------------------------------------------------------- */

static const char *sampleItems[] = { "2", "4", "8", 0 };
static int sampleValues[] = { 2, 4, 8 };

XCALL_(static int)
Interface(
	long            version,
	GlobalFunc     *global,
	SoftShadInst   *inst,
	void           *serverData)
{
	LWPanelFuncs *panl;
	LWPanelID     pan;
	LWControl    *ctlSize, *ctlSamp, *ctlStr;
	int           sampIdx;

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

		pan = PAN_CREATE(panl, "SoftShadow v" PLUGIN_VERSION
		                       " (c) D. Panokostas");
		if (!pan) goto fallback;

		ctlSize = FLOAT_CTL(panl, pan, "Light Size");
		ctlSamp = POPUP_CTL(panl, pan, "Samples", sampleItems);
		ctlStr  = SLIDER_CTL(panl, pan, "Strength", 150, 0, 100);

		{
			double s = inst->lightSize / 1000.0;
			SET_FLOAT(ctlSize, s);
		}
		sampIdx = (inst->samples <= 2) ? 0
		        : (inst->samples <= 4) ? 1 : 2;
		SET_INT(ctlSamp, sampIdx);
		SET_INT(ctlStr, inst->strength);

		if ((*panl->open)(pan, PANF_BLOCKING | PANF_CANCEL)) {
			double s;
			GET_FLOAT(ctlSize, s);
			inst->lightSize = (int)(s * 1000.0);
			GET_INT(ctlSamp, sampIdx);
			inst->samples = (sampIdx < 3)
			              ? sampleValues[sampIdx] : 4;
			GET_INT(ctlStr, inst->strength);

			if (inst->lightSize < 1) inst->lightSize = 1;
			if (inst->strength < 0) inst->strength = 0;
			if (inst->strength > 100) inst->strength = 100;
		}

		PAN_KILL(panl, pan);
		return AFUNC_OK;
	}

fallback:
	if (!msg)
		return AFUNC_BADGLOBAL;

	{
		char infoBuf[48];
		char nb[12];
		strcpy(infoBuf, "Size:");
		int_to_str(inst->lightSize, nb, 12);
		strcat(infoBuf, nb);
		strcat(infoBuf, " Samp:");
		int_to_str(inst->samples, nb, 12);
		strcat(infoBuf, nb);
		strcat(infoBuf, " Str:");
		int_to_str(inst->strength, nb, 12);
		strcat(infoBuf, nb);
	}
	(*msg->info)("SoftShadow", "Settings shown above");
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

	itemInfo = (LWItemInfo *)(*global)("LW Item Info", GFUSE_TRANSIENT);
	lightInfo = (LWLightInfo *)(*global)("LW Light Info", GFUSE_TRANSIENT);

	return AFUNC_OK;
}

/* ----------------------------------------------------------------
 * Server description
 * ---------------------------------------------------------------- */

ServerRecord ServerDesc[] = {
	{ "ShaderHandler",   "SoftShadow",
	  (ActivateFunc *)Activate },
	{ "ShaderInterface", "SoftShadow",
	  (ActivateFunc *)Interface },
	{ 0 }
};

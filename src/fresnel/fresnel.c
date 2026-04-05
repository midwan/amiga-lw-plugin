/*
 * FRESNEL.C -- Layout Shader Plugin
 *
 * Physically-based Fresnel effect for LightWave 3D surfaces.
 * Increases reflectivity and decreases transparency at glancing
 * angles using Schlick's approximation.
 *
 * F = F0 + (1 - F0) * (1 - cos(theta))^power
 *
 * where F0 = ((ior - 1) / (ior + 1))^2
 *
 * NOTE: Uses AllocMem/FreeMem instead of malloc/free because
 * -nostartfiles skips libnix heap initialization.
 */

#include <splug.h>
#include <lwran.h>
#include <lwpanel.h>

#include <string.h>

#include <proto/exec.h>
#include <exec/memory.h>

extern struct ExecBase *SysBase;

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
 * Integer/string helpers (avoid libnix sprintf/sscanf)
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
 * Math helpers (avoid libnix pow/fabs)
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

/* ----------------------------------------------------------------
 * Types
 * ---------------------------------------------------------------- */

#define FRESNEL_DEFAULT_IOR    1.5
#define FRESNEL_DEFAULT_POWER  5

typedef struct {
	double ior;            /* index of refraction (1.0 - 5.0)    */
	int    reflPower;      /* reflection Fresnel exponent (1-10)  */
	int    affectMirror;   /* modify mirror/reflection            */
	int    affectTrans;    /* modify transparency                 */
	int    affectDiffuse;  /* modify diffuse (inverse Fresnel)    */
	int    diffPower;      /* diffuse Fresnel exponent (1-10)     */
	double f0;             /* precomputed F0 from IOR             */
} FresnelInst;

/* ----------------------------------------------------------------
 * Globals
 * ---------------------------------------------------------------- */

static MessageFuncs *msg;

/* ----------------------------------------------------------------
 * Precompute F0 from IOR
 * ---------------------------------------------------------------- */

static void
compute_f0(FresnelInst *inst)
{
	double r;

	if (inst->ior < 1.0) inst->ior = 1.0;
	r = (inst->ior - 1.0) / (inst->ior + 1.0);
	inst->f0 = r * r;
}

/* ----------------------------------------------------------------
 * Handler callbacks
 * ---------------------------------------------------------------- */

XCALL_(static LWInstance)
Create(LWError *err)
{
	FresnelInst *inst;

	XCALL_INIT;

	inst = (FresnelInst *)plugin_alloc(sizeof(FresnelInst));
	if (!inst)
		return 0;

	inst->ior           = FRESNEL_DEFAULT_IOR;
	inst->reflPower     = FRESNEL_DEFAULT_POWER;
	inst->affectMirror  = 1;
	inst->affectTrans   = 1;
	inst->affectDiffuse = 1;
	inst->diffPower     = FRESNEL_DEFAULT_POWER;
	compute_f0(inst);

	return inst;
}

XCALL_(static void)
Destroy(FresnelInst *inst)
{
	XCALL_INIT;
	if (inst) plugin_free(inst);
}

XCALL_(static LWError)
Copy(FresnelInst *from, FresnelInst *to)
{
	XCALL_INIT;
	*to = *from;
	return 0;
}

XCALL_(static LWError)
Load(FresnelInst *inst, const LWLoadState *ls)
{
	char buf[32];

	XCALL_INIT;

	if (ls->ioMode != LWIO_SCENE)
		return 0;

	buf[0] = '\0';
	(*ls->read)(ls->readData, buf, 32);
	if (buf[0] == '\0')
		(*ls->read)(ls->readData, buf, 32);
	if (buf[0])
		inst->ior = str_to_int(buf) / 1000.0;

	buf[0] = '\0';
	(*ls->read)(ls->readData, buf, 32);
	if (buf[0])
		inst->reflPower = str_to_int(buf);

	buf[0] = '\0';
	(*ls->read)(ls->readData, buf, 32);
	if (buf[0])
		inst->affectMirror = str_to_int(buf);

	buf[0] = '\0';
	(*ls->read)(ls->readData, buf, 32);
	if (buf[0])
		inst->affectTrans = str_to_int(buf);

	buf[0] = '\0';
	(*ls->read)(ls->readData, buf, 32);
	if (buf[0])
		inst->affectDiffuse = str_to_int(buf);

	buf[0] = '\0';
	(*ls->read)(ls->readData, buf, 32);
	if (buf[0])
		inst->diffPower = str_to_int(buf);

	compute_f0(inst);
	return 0;
}

XCALL_(static LWError)
Save(FresnelInst *inst, const LWSaveState *ss)
{
	char buf[32];

	XCALL_INIT;

	if (ss->ioMode != LWIO_SCENE)
		return 0;

	int_to_str((int)(inst->ior * 1000.0), buf, 32);
	(*ss->write)(ss->writeData, buf, strlen(buf));

	int_to_str(inst->reflPower, buf, 32);
	(*ss->write)(ss->writeData, buf, strlen(buf));

	int_to_str(inst->affectMirror, buf, 32);
	(*ss->write)(ss->writeData, buf, strlen(buf));

	int_to_str(inst->affectTrans, buf, 32);
	(*ss->write)(ss->writeData, buf, strlen(buf));

	int_to_str(inst->affectDiffuse, buf, 32);
	(*ss->write)(ss->writeData, buf, strlen(buf));

	int_to_str(inst->diffPower, buf, 32);
	(*ss->write)(ss->writeData, buf, strlen(buf));

	return 0;
}

XCALL_(static LWError)
Init(FresnelInst *inst)
{
	XCALL_INIT;
	compute_f0(inst);
	return 0;
}

XCALL_(static void)
Cleanup(FresnelInst *inst)
{
	XCALL_INIT;
}

XCALL_(static LWError)
NewTime(FresnelInst *inst, LWFrame f, LWTime t)
{
	XCALL_INIT;
	return 0;
}

XCALL_(static unsigned int)
Flags(FresnelInst *inst)
{
	unsigned int f = 0;

	XCALL_INIT;

	if (inst->affectMirror)  f |= LWSHF_MIRROR;
	if (inst->affectTrans)   f |= LWSHF_TRANSP;
	if (inst->affectDiffuse) f |= LWSHF_DIFFUSE;

	return f;
}

XCALL_(static void)
Evaluate(FresnelInst *inst, ShaderAccess *sa)
{
	double cosAngle, oneMinusCos, fresnel;

	XCALL_INIT;

	/*
	 * sa->cosine is the cosine of the angle between the
	 * surface normal and the incoming ray direction.
	 */
	cosAngle = sa->cosine;
	if (cosAngle < 0.0) cosAngle = -cosAngle;
	if (cosAngle > 1.0) cosAngle = 1.0;

	oneMinusCos = 1.0 - cosAngle;

	/* Reflection and transparency use reflPower */
	if (inst->affectMirror || inst->affectTrans) {
		fresnel = inst->f0 + (1.0 - inst->f0)
		          * pow_int(oneMinusCos, inst->reflPower);
		if (fresnel > 1.0) fresnel = 1.0;
		if (fresnel < 0.0) fresnel = 0.0;

		if (inst->affectMirror)
			sa->mirror = sa->mirror
			             + (1.0 - sa->mirror) * fresnel;

		if (inst->affectTrans)
			sa->transparency *= (1.0 - fresnel);
	}

	/* Diffuse uses its own power (inverse: reduce at glancing angles) */
	if (inst->affectDiffuse) {
		double diffFresnel = inst->f0 + (1.0 - inst->f0)
		                     * pow_int(oneMinusCos, inst->diffPower);
		if (diffFresnel > 1.0) diffFresnel = 1.0;
		if (diffFresnel < 0.0) diffFresnel = 0.0;

		sa->diffuse *= (1.0 - diffFresnel);
	}
}

/* ----------------------------------------------------------------
 * Interface
 * ---------------------------------------------------------------- */

XCALL_(static int)
Interface(
	long         version,
	GlobalFunc  *global,
	FresnelInst *inst,
	void        *serverData)
{
	LWPanelFuncs *panl;
	LWPanelID     pan;
	LWControl    *ctlIOR, *ctlReflPow, *ctlMirror, *ctlTrans;
	LWControl    *ctlDiffuse, *ctlDiffPow;
	int           iorFixed;
	char          infoBuf[64];

	XCALL_INIT;
	if (version != 1)
		return AFUNC_BADVERSION;

	msg = (MessageFuncs *)(*global)("Info Messages", GFUSE_TRANSIENT);

	panl = (LWPanelFuncs *)
		(*global)(PANEL_SERVICES_NAME, GFUSE_TRANSIENT);

	if (panl) {
		static LWPanControlDesc desc;
		static LWValue ival = {LWT_INTEGER};
		static LWValue fval = {LWT_FLOAT};
		(void)fval;

		pan = PAN_CREATE(panl, "Fresnel Shader");
		if (!pan)
			goto fallback;

		ctlIOR     = FLOAT_CTL(panl, pan, "Index of Refraction");
		ctlMirror  = BOOL_CTL(panl, pan, "Affect Reflection");
		ctlReflPow = SLIDER_CTL(panl, pan, "Reflection Power",
		                        150, 1, 10);
		ctlTrans   = BOOL_CTL(panl, pan, "Affect Transparency");
		ctlDiffuse = BOOL_CTL(panl, pan, "Affect Diffuse");
		ctlDiffPow = SLIDER_CTL(panl, pan, "Diffuse Power",
		                        150, 1, 10);

		SET_FLOAT(ctlIOR, inst->ior);
		SET_INT(ctlMirror, inst->affectMirror);
		SET_INT(ctlReflPow, inst->reflPower);
		SET_INT(ctlTrans, inst->affectTrans);
		SET_INT(ctlDiffuse, inst->affectDiffuse);
		SET_INT(ctlDiffPow, inst->diffPower);

		if (PAN_POST(panl, pan)) {
			GET_FLOAT(ctlIOR, inst->ior);
			GET_INT(ctlMirror, inst->affectMirror);
			GET_INT(ctlReflPow, inst->reflPower);
			GET_INT(ctlTrans, inst->affectTrans);
			GET_INT(ctlDiffuse, inst->affectDiffuse);
			GET_INT(ctlDiffPow, inst->diffPower);

			if (inst->ior < 1.0) inst->ior = 1.0;
			if (inst->ior > 5.0) inst->ior = 5.0;
			if (inst->reflPower < 1) inst->reflPower = 1;
			if (inst->reflPower > 10) inst->reflPower = 10;
			if (inst->diffPower < 1) inst->diffPower = 1;
			if (inst->diffPower > 10) inst->diffPower = 10;

			compute_f0(inst);
		}

		PAN_KILL(panl, pan);
		return AFUNC_OK;
	}

fallback:
	/* No panels — show current settings */
	if (!msg)
		return AFUNC_BADGLOBAL;

	iorFixed = (int)(inst->ior * 100.0);
	strcpy(infoBuf, "IOR: ");
	{
		char nb[12];
		int_to_str(iorFixed / 100, nb, 12);
		strcat(infoBuf, nb);
		strcat(infoBuf, ".");
		int_to_str(iorFixed % 100, nb, 12);
		if (iorFixed % 100 < 10) strcat(infoBuf, "0");
		strcat(infoBuf, nb);
	}
	strcat(infoBuf, "  RPow: ");
	{
		char nb[12];
		int_to_str(inst->reflPower, nb, 12);
		strcat(infoBuf, nb);
	}
	strcat(infoBuf, "  DPow: ");
	{
		char nb[12];
		int_to_str(inst->diffPower, nb, 12);
		strcat(infoBuf, nb);
	}
	(*msg->info)("Fresnel Shader", infoBuf);

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
	{ "ShaderHandler",   "Fresnel",
	  (ActivateFunc *)Activate },
	{ "ShaderInterface", "Fresnel",
	  (ActivateFunc *)Interface },
	{ 0 }
};

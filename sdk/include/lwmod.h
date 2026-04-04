/*
 * LWSDK Header File
 * Copyright 1995  NewTek, Inc.
 */
#ifndef LW_MOD_H
#define LW_MOD_H

#include <lwbase.h>

typedef int             DynaType;
#define DY_NULL         0
#define DY_STRING       1
#define DY_INTEGER      2
#define DY_FLOAT        3
#define DY_DISTANCE     4
#define DY_VINT         5
#define DY_VFLOAT       6
#define DY_VDIST        7
#define DY_BOOLEAN      8
#define DY_CHOICE       9
#define DY_SURFACE      10
#define DY_FONT         11
#define DY_TEXT         12
#define DY_LAYERS       13
#define DY_CUSTOM       14
#define DY__LAST        DY_CUSTOM
typedef int              EltOpLayer;
#define OPLYR_PRIMARY    0
#define OPLYR_FG         1
#define OPLYR_BG         2
#define OPLYR_SELECT     3
#define OPLYR_ALL        4
#define OPLYR_EMPTY      5
#define OPLYR_NONEMPTY   6
typedef int              EltOpSelect;
#define OPSEL_GLOBAL     0
#define OPSEL_USER       1
#define OPSEL_DIRECT     2
typedef struct st_Vertex        *PntID;
typedef struct st_Polygon       *PolID;
typedef struct st_EditState     *EditStateRef;
typedef struct st_PointInfo {
	PntID            pnt;
	void            *userData;
	int              layer;
	int              flags;
	double           position[3];
} PointInfo;
typedef struct st_PolygonInfo {
	PolID            pol;
	void            *userData;
	int              layer;
	int              flags;
	int              numPnts;
	const PntID     *points;
	const char      *surface;
} PolygonInfo;
typedef int             EDError;
#define EDERR_NONE      0
#define EDERR_NOMEMORY  1
#define EDERR_BADLAYER  2
#define EDERR_BADSURF   3
#define EDERR_USERABORT 4
#define EDERR_BADARGS   5
typedef EDError         PointScanFunc (void *, const PointInfo *);
typedef EDError         PolyScanFunc (void *, const PolygonInfo *);
typedef struct st_PBoundCv {
	PolID            curve;
	int              start, end;
} PBoundCv;
typedef struct st_DyValString {
	DynaType         type;
	char            *buf;
	int              bufLen;
} DyValString;
typedef struct st_DyValInt {
	DynaType         type;
	int              value;
	int              defVal;
} DyValInt;
typedef struct st_DyValFloat {
	DynaType         type;
	double           value;
	double           defVal;
} DyValFloat;
typedef struct st_DyValIVector {
	DynaType         type;
	int              val[3];
	int              defVal;
} DyValIVector;
typedef struct st_DyValFVector {
	DynaType         type;
	double           val[3];
	double           defVal;
} DyValFVector;
typedef struct st_DyValCustom {
	DynaType         type;
	int              val[4];
} DyValCustom;
typedef union un_DynaValue {
	DynaType         type;
	DyValString      str;
	DyValInt         intv;
	DyValFloat       flt;
	DyValIVector     ivec;
	DyValFVector     fvec;
	DyValCustom      cust;
} DynaValue;
#define DYERR_NONE                0
#define DYERR_MEMORY            (-1)
#define DYERR_BADTYPE           (-2)
#define DYERR_BADSEQ            (-3)
#define DYERR_BADCTRLID         (-4)
#define DYERR_TOOMANYCTRL       (-5)
#define DYERR_INTERNAL          (-6)
#define PPDF_SELECT     (1<<0)
#define PPDF_DELETE     (1<<1)
#define PPDF_CCEND      (1<<2)
#define PPDF_CCSTART    (1<<3)
#define PPDF_CURVE      (1<<4)
#define PPDF_DETAIL     (1<<5)
typedef struct st_MeshEditOp {
	EditStateRef      state;
	int               layerNum;
	void            (*done) (EditStateRef, EDError, int selm);

	int             (*pointCount) (EditStateRef, EltOpLayer, int mode);
	int             (*polyCount)  (EditStateRef, EltOpLayer, int mode);
	EDError         (*pointScan) (EditStateRef, PointScanFunc *,
				      void *, EltOpLayer);
	EDError         (*polyScan)  (EditStateRef, PolyScanFunc *,
				      void *, EltOpLayer);
	PointInfo *     (*pointInfo)  (EditStateRef, PntID);
	PolygonInfo *   (*polyInfo)   (EditStateRef, PolID);
	int             (*polyNormal) (EditStateRef, PolID, double[3]);
	PntID           (*addPoint) (EditStateRef, double *xyz);
	PolID           (*addPoly)  (EditStateRef, const char *surf,
				     int numPnt, const PntID *);
	PolID           (*addCurve) (EditStateRef, const char *surf,
				     int numPnt, const PntID *, int flags);
	EDError         (*addQuad)  (EditStateRef, PntID, PntID,
				     PntID, PntID);
	EDError         (*addTri)   (EditStateRef, PntID, PntID, PntID);
	EDError         (*addPatch) (EditStateRef, int nr, int nc, int lr,
				     int lc, PBoundCv *r0, PBoundCv *r1,
				     PBoundCv *c0, PBoundCv *c1);
	EDError         (*remPoint) (EditStateRef, PntID);
	EDError         (*remPoly)  (EditStateRef, PolID);
	EDError         (*pntMove) (EditStateRef, PntID, const double *);
	EDError         (*polSurf) (EditStateRef, PolID, const char *);
	EDError         (*polPnts) (EditStateRef, PolID, int, const PntID *);
	EDError         (*polFlag) (EditStateRef, PolID, int mask, int value);
} MeshEditOp;
#define EDSELM_CLEARCURRENT     (1<<0)
#define EDSELM_SELECTNEW        (1<<1)
#ifndef LW_PRERELEASE
 #define EDSELM_FORCEVRTS       (1<<2)
 #define EDSELM_FORCEPOLS       (1<<3)
#endif
#define EDCOUNT_ALL              0
#define EDCOUNT_SELECT           1
#define EDCOUNT_DELETE           2
typedef int             CommandCode;
typedef struct st_DyChoiceHint {
	const char      *item;
	int              value;
} DyChoiceHint;

typedef struct st_DyBitfieldHint {
	char             code;
	int              bitval;
} DyBitfieldHint;
typedef struct st_DynaStringHint {
	DyChoiceHint    *chc;
	DyBitfieldHint  *bits;
} DynaStringHint;
typedef struct st_DynaRequest   *DynaRequestID;
typedef struct st_DyReqStringDesc {
	DynaType         type;
	int              width;
} DyReqStringDesc;
typedef struct st_DyReqChoiceDesc {
	DynaType         type;
	const char     **items;
	int              vertical;
} DyReqChoiceDesc;
typedef struct st_DyReqTextDesc {
	DynaType         type;
	const char     **text;
} DyReqTextDesc;
typedef union un_DyReqControlDesc {
	DynaType         type;
	DyReqStringDesc  string;
	DyReqChoiceDesc  choice;
	DyReqTextDesc    text;
} DyReqControlDesc;
typedef MeshEditOp *    MeshEditBegin (int pntBuf, int polBuf,
					EltOpSelect);
typedef struct st_ModCommand {
	void             *data;
	const char       *argument;
	CommandCode     (*lookup)  (void *, const char *cmdName);
	int             (*execute) (void *, CommandCode cmd,
				    int argc, const DynaValue *argv,
				    EltOpSelect, DynaValue *result);
	MeshEditBegin    *editBegin;
} ModCommand;
typedef int             DynaConvertFunc (const DynaValue *,
					 DynaValue *,
					 const DynaStringHint *);
typedef struct st_DynaReqFuncs {
	DynaRequestID   (*create)   (const char *);
	int             (*addCtrl)  (DynaRequestID, const char *,
				     DyReqControlDesc *);
	DynaType        (*ctrlType) (DynaRequestID, int);
	int             (*valueSet) (DynaRequestID, int, DynaValue *);
	int             (*valueGet) (DynaRequestID, int, DynaValue *);
	int             (*post)     (DynaRequestID);
	void            (*destroy)  (DynaRequestID);
} DynaReqFuncs;
typedef struct st_DynaMonitorFuncs {
	Monitor *       (*create) (const char *, const char *);
	void            (*destroy) (Monitor *);
} DynaMonitorFuncs;
typedef struct st_CustomCommandFuncs {
	int             (*listAdd) (const char *name,
				    const char *server,
				    const char *arg);
	void            (*listRem) (const char *name);
	int             (*funGet) (int n, char *server,
				   char *arg, int bufLen);
	void            (*funSet) (int n, const char *server,
				   const char *arg);
} CustomCommandFuncs;
typedef struct st_StateQueryFuncs {
	int             (*numLayers) (void);
	unsigned int    (*layerMask) (EltOpLayer);
	const char *    (*surface) (void);
	unsigned int    (*bbox) (EltOpLayer, double *minmax);
} StateQueryFuncs;
typedef struct st_SurfaceListFuncs {
	const char *    (*next)    (const char *name);
	void            (*create)  (const char *name);
	void            (*rename)  (const char *name,
				    const char *newName);
	void *          (*getData) (const char *name, int *size);
	void            (*setData) (const char *name, int size,
				    void *data);
} SurfaceListFuncs;
typedef struct st_FontListFuncs {
	int             (*count) (void);
	int             (*index) (const char *name);
	const char *    (*name)  (int index);
	int             (*load)  (const char *filename);
	void            (*clear) (int index);
} FontListFuncs;

#endif

/* lwpanel.h -- LWPanelServices Global-class plugin support file        
 *      by Arnie Cachelin, copyright 1995,1996 NewTek Inc.
 *      6/28/96 - handle fn given EVNT_ flags arg       
 *      9/3/96  - define LWPANELS_API_VERSION   ,PAN_VERSION, CUSTPUPUP type
 *      1/22/97 - api v8, HSLIDER
 *      1/29/97 - api v9, PANF_PASSALLKEYS -- let Enter/Esc keys through to panels
 *              4/24/97 - api v10, CTL_RANGEMIN, CTL_RANGEMAX tags for sldiers, POPDOWN, DRAGBUT controls , WBUTTONs,WPOPUP, 
 *                      width controlled buttons! 
 *              5/10/97 - CTL_ACTIVATE tag for strings, width control on Item popups
 *      6/11/97 - api v11 PAN_RESULT tag for getting exit status from non-blocking panels
 *               6/24/97 - api v12 PAN_HOSTDISPLAY tag for getting the "Host Display Info" global info for panel
 *               9/25/97 - api v13 DRAGAREA_CTL, PAN_USERKEYUPS, PAN_QUALIFIERS added!
 *              11/17/97 - api v14 Added ListBox control type,  takes size, count,itemname callback f'ns , Border control type,.
 * 1/7/98  -- api v15 Added PAN_TO_FRONT for happy fun
  * 3/9/98  -- api v16 PANF_ABORT, TabChoice control
*/

#define PANEL_SERVICES_NAME     "LWPanelServices"       
#define LWPANELS_API_VERSION    16      

// former lwmod.h stuff
typedef int             LWType;
#define LWT_NULL         0
#define LWT_STRING       1
#define LWT_INTEGER      2
#define LWT_FLOAT        3
#define LWT_DISTANCE     4
#define LWT_VINT         5
#define LWT_VFLOAT       6
#define LWT_VDIST        7
#define LWT_BOOLEAN      8
#define LWT_CHOICE       9
#define LWT_SURFACE      10      // NOT Used , modeler compatibility only
#define LWT_FONT         11      // NOT Used , modeler compatibility only
#define LWT_TEXT         12      // NOT Used , modeler compatibility only
#define LWT_LAYERS       13      // NOT Used , modeler compatibility only
#define LWT_CUSTOM       14      // NOT Used , modeler compatibility only
#define LWT_RANGE              15
#define LWT_LWITEM          16
#define LWT_PERCENT      17
#define LWT_POPUP                18
#define LWT_AREA                 19
#define LWT__LAST        LWT_AREA

#ifdef _QD3D_PANEL
        #include <CodeFragments.h>
        #include <QD3D.h>
        #include <QD3DDrawContext.h>
        #include <QD3DTransform.h>
        #include <QD3DRenderer.h>
        #include <QD3DGeometry.h>
        #include <QD3DShader.h>
        #include <QD3DCamera.h>
        #include <QD3DErrors.h>
        #include <QD3DGroup.h>
        #include <QD3DLight.h>
        #include <QD3DView.h>
        #include <QD3DMath.h>


        /*
         * QuickDraw 3D panel preview data
         * Main QD3D data structure.  Provides default implementation as well as 
         * user data space (qd3d_data), an optional user specified renderer, and
         * an optional user specified init function that takes as its param a 
         * pointer to this QD3D_anPreview struct and occurs AFTER the default 
         * initialization.
         */
                typedef struct st_QD3DPanPreview {
                        TQ3ViewObject                   qView;
                        TQ3DrawContextObject    qCtxt;
                        TQ3CameraObject                 qCam;
                        TQ3ShaderObject                 qShPh;
                        TQ3Area                                 qPane;   
                        TQ3Matrix4x4                    qRot, qZoom, qMove;
                        TQ3Vertex3D                             *qv3;
                        TQ3AttributeSet                 *qv3a;
                        TQ3GroupObject                  qObject;
                        TQ3StyleObject                  qBack, qFill, qInterpol;
                        TQ3Point3D                              qObjOrigin;
                        CWindowPtr                              wp;                                                     // private - hands off!
                        void                                    *qd3d_data;                                     // user data space (optional)
                        int                                             qv3n;
                        int                                             inited;
                        void                                    (*render)(void* data);          // user defined render function (optional)
                        void                                    (*myInit)(void* initData);      // used to init data after the default
                } QD3DPanPreview;                                                                               // init - change camera or object here!
                
        #define QD3D_GET_DATA(ctl, qd) ((*ctl->get)(ctl, CTL_VALUE, &ival), qd = (QD3DPanPreview *)ival.intv.value)
        #define QD3D_RENDER(ctl) (ival.intv.value=2, (*ctl->set)(ctl, CTL_VALUE, &ival))
#endif

typedef struct st_LWValString {
        LWType         type;
        char            *buf;
        int              bufLen;
} LWValString;
typedef struct st_LWValInt {
        LWType         type;
        int              value;
        int              defVal;
} LWValInt;
typedef struct st_LWValFloat {
        LWType         type;
        double           value;
        double           defVal;
} LWValFloat;
typedef struct st_LWValIVector {
        LWType         type;
        int              val[3];
        int              defVal;
} LWValIVector;
typedef struct st_LWValFVector {
        LWType         type;
        double           val[3];
        double           defVal;
} LWValFVector;
typedef struct st_LWValCustom {
        LWType         type;
        int              val[4];
} LWValCustom;
typedef union un_LWValue {
        LWType         type;
        LWValString      str;
        LWValInt         intv;
        LWValFloat       flt;
        LWValIVector     ivec;
        LWValFVector     fvec;
        LWValCustom      cust;
} LWValue;

#ifndef LW_MOD_H

#define DyValString             LWValString
#define DyValInt                LWValInt
#define DyValFloat              LWValFloat
#define DyValIVector    LWValIVector
#define DyValFVector    LWValFVector
#define DyValCustom             LWValCustom
#define DynaValue               LWValue

#define DY_NULL         LWT_NULL    
#define DY_STRING       LWT_STRING  
#define DY_INTEGER      LWT_INTEGER 
#define DY_FLOAT        LWT_FLOAT   
#define DY_DISTANCE     LWT_DISTANCE
#define DY_VINT         LWT_VINT    
#define DY_VFLOAT       LWT_VFLOAT  
#define DY_VDIST        LWT_VDIST   
#define DY_BOOLEAN      LWT_BOOLEAN 
#define DY_CHOICE       LWT_CHOICE  
#define DY_SURFACE      LWT_SURFACE 
#define DY_FONT         LWT_FONT    
#define DY_TEXT         LWT_TEXT    
#define DY_LAYERS       LWT_LAYERS  
#define DY_CUSTOM       LWT_CUSTOM  
#define DY_RANGE              LWT_RANGE
#define DY_LWITEM             LWT_LWITEM
#define DY_PERCENT      LWT_PERCENT
#define DY_POPUP        LWT_POPUP
#define DY_AREA       LWT_AREA
#define DY__LAST        DY_AREA
#else
#define DY_RANGE              LWT_RANGE
#define DY_LWITEM             LWT_LWITEM
#define DY_PERCENT      LWT_PERCENT
#define DY_POPUP        LWT_POPUP
#define DY_AREA       LWT_AREA
#define DY__LAST        DY_AREA
#endif

typedef struct st_LWPanStringDesc {
        LWType       type;
        int          width;
} LWPanStringDesc;
typedef struct st_LWPanChoiceDesc {
        LWType       type;
        const char   **items;
        int          vertical;
} LWPanChoiceDesc;
typedef struct st_LWPanTextDesc {
        LWType       type;
        const char   **text;
} LWPanTextDesc;
typedef struct st_LWPanRangeDesc {
        LWType          type;
        int                     width;
        int                     min;
        int                     max;
} LWPanRangeDesc;
typedef struct st_LWPanAreaDesc {
        LWType          type;
        int                     width;
        int                     height;
} LWPanAreaDesc;
typedef struct st_LWPanLWItemDesc {
        LWType          type;
        void            *global;
        int                     itemType;
        char            **list;
        int                     count;
        int                     width;
} LWPanLWItemDesc;
typedef struct st_LWPanPopupDesc {
        LWType          type;
        int                     width;
        int                     (*countFn)(void *);
        char            *(*nameFn)(void *, int);
} LWPanPopupDesc;
typedef struct st_LWPanListBoxDesc {
        LWType          type;
        int                     width,visItems, top;  // w,height in items
        int                     (*countFn)(void *);
        char            *(*nameFn)(void *, int);
} LWPanListBoxDesc;
typedef union un_LWPanControlDesc {
        LWType                  type;
        LWPanStringDesc         string;
        LWPanChoiceDesc         choice;
        LWPanTextDesc           text;
        LWPanRangeDesc          range;
        LWPanAreaDesc           area;
        LWPanLWItemDesc         lwitem;
        LWPanPopupDesc          popup;
        LWPanListBoxDesc        listbox;
} LWPanControlDesc;

typedef unsigned short   DualKey;
typedef LWPanControlDesc                ControlDesc;

#define LWI_IMAGE       3+1 // LWI_BONE+1        // for LWItem controls
#define LWI_ANY         LWI_IMAGE+1

typedef enum {   // Tags to get/set panel attributes
        PAN_X, PAN_Y, PAN_W, PAN_H, PAN_MOUSEX, PAN_MOUSEY, 
        PAN_FLAGS, PAN_USERDRAW, PAN_USERACTIVATE,
        PAN_USEROPEN, PAN_USERCLOSE, PAN_TITLE,PAN_USERDATA, PAN_USERKEYS,
        PAN_LWINSTANCE, PAN_MOUSEBUTTON, PAN_MOUSEMOVE  ,PAN_PANFUN, PAN_VERSION, PAN_RESULT,
        PAN_HOSTDISPLAY, PAN_USERKEYUPS, PAN_QUALIFIERS, PAN_TO_FRONT
} pTag;

typedef enum { DR_ERASE, DR_RENDER, DR_GHOST, DR_REFRESH } DrMode;

typedef enum {           // Tags to get/set panel attributes
        CTL_X, CTL_Y, CTL_W, CTL_H, CTL_VALUE,
        CTL_HOTX, CTL_HOTY, CTL_HOTW, CTL_HOTH, CTL_LABELWIDTH,
        CTL_MOUSEX, CTL_MOUSEY, 
        CTL_FLAGS, CTL_USERDRAW, CTL_USEREVENT, CTL_LABEL, CTL_USERDATA,
        CTL_PANEL, CTL_PANFUN,  // use these 2 to get your panelID and functions!
        CTL_RANGEMIN, CTL_RANGEMAX,             // get/set slider range params  API 10 and later
        CTL_ACTIVATE   // set only, to actvate/enter string controls   API 10 and later
} cTag;

typedef enum { MX_HCHOICE, MX_VCHOICE ,MX_POPUP } MxType;

typedef void *LWControlID;       

// bits for flags Set with CTL_FLAGS
#define CTLF_DISABLE                    0x100    // Read-Only ,i.e. check if button has been drawn w/DR_GHOST , or erase
#define CTLF_INVISIBLE                  0x200 // Read-Only        ... erased
#define CTLF_GHOST                              CTLF_DISABLE    

typedef struct st_LWControl {
        void            (*draw)(LWControlID, DrMode);
        void            (*get)(LWControlID, cTag, LWValue *);
        void            (*set)(LWControlID, cTag, LWValue *);
        void            *priv_data;     
} LWControl;

typedef void *LWPanelID;

typedef void (*PanHook)(LWPanelID, void *);
typedef void (*PanKeyHook)(LWPanelID, void *, DualKey);    
typedef void (*PanMouseHook)(LWPanelID, void *, int, int, int);  // see input qualifier bits       
typedef void (*PanDrawHook)(LWPanelID, void *, DrMode);    
typedef void (*CtlDrawHook)(LWControlID, void *,DrMode);           
typedef void (*CtlEventHook)(LWControlID, void *);

typedef struct st_DisplayMetrics {
        int              width, height;
        int              pixX, pixY;
        int              maxColors, depth;
        int              textHeight;
        int              textAscent;
} DisplayMetrics;

#define TRUECOLOR(dm)   (dm->depth ? 1:0)
#define PALETTESIZE(dm) (dm->depth ? 1<<24:1<<dm->depth)

typedef struct st_DrawFuncs {
        void    (*drawPixel)(LWPanelID,int,int,int);                    // color,x,y
        void    (*drawRGBPixel)(LWPanelID,int,int,int,int,int);                 // r,g,b,x,y
        void    (*drawLine)(LWPanelID,int,int,int,int,int);     // color,x,y,x2,y2
        void    (*drawBox)(LWPanelID,int,int,int,int,int);      // color,x,y,w,h
        void    (*drawRGBBox)(LWPanelID,int,int,int,int,int,int,int);   // r,g,b,x,y,w,h
        void    (*drawBorder)(LWPanelID,int,int,int,int,int);// indent,x,y,w,h          (h==0 -> divider)
        int     (*textWidth)(LWPanelID,char *);// text
        void    (*drawText)(LWPanelID,char *,int,int,int);// text,color,x,y
        const DisplayMetrics    *(*dispMetrics)();
} DrawFuncs;

#define PANF_BLOCKING           1  // open waits for close, or returns immediately
#define PANF_CANCEL                     2  // Continue or Continue/Cancel
#define PANF_FRAME                      4  // Windowing System Title bar
#define PANF_MOUSETRAP          8  // Please track mouse input and call mouse callbacks  
#define PANF_PASSALLKEYS        16 // Pass Enter/Esc keys to panel, instead of closing with OK/Cancel
#define PANF_ABORT                      32 // Abort instead of Continue, usw w/ PANF_CANEL!!

#define EVNT_BLOCKING   1 // 1<<0  i.e. bit flags  handle() method waits for input, or gives immediate return
#define EVNT_ALL        2 // 1<<1  if set, handle() method will poll entire queue of events, 
// w/EVNT_BLOCKING handle() won't return 'til panel is closed , w/out EVNT_BLOCKING, EVNT_ALL is irrelevant

typedef struct st_LWPanelFuncs {
        LWPanelID       (*create)(char *, void *);
        void                    (*destroy)(LWPanelID);
        int                     (*open)(LWPanelID,int);  // flags int (see flag bits  PANF_ etc.)
        int                     (*handle)(LWPanelID, int);       //  process input manually after non-blocking open, use EVNT_ bits to process async. or synchronously
        void                    (*draw)(LWPanelID, DrMode);
        void                    (*close)(LWPanelID);
        void                    (*get)(LWPanelID, pTag, void *);
        void                    (*set)(LWPanelID, pTag, void *);
        LWControl       *(*addControl)(LWPanelID pan, char *type, ControlDesc *data, char *label);
        LWControl       *(*nextControl)(LWPanelID pan, LWControlID ctl);
        DrawFuncs       *drawFuncs;
        void            *user_data;      // do whatcha like
        GlobalFunc      *globalFun;      // please set this to your GlobalFunction pointer.. at least for file req.
} LWPanelFuncs;


#define PAN_CREATE(pfunc,tit) ((*pfunc->create)(tit,pfunc))
#define PAN_POST(pfunc,pan) ((*pfunc->open)(pan,PANF_BLOCKING|PANF_CANCEL|PANF_FRAME))
#define PAN_KILL(pfunc,pan) ((*pfunc->destroy)(pan))

#define HCHOICE_CTL(pfunc,pan,tit,ls) (desc.type=LWT_CHOICE,desc.choice.vertical=MX_HCHOICE,\
                          desc.choice.items=ls,\
                          (*pfunc->addControl) (pan, "ChoiceControl", &desc, tit))
#define VCHOICE_CTL(pfunc,pan,tit,ls) (desc.type=LWT_CHOICE,desc.choice.vertical=MX_VCHOICE,\
                          desc.choice.items=ls,\
                          (*pfunc->addControl) (pan, "ChoiceControl", &desc, tit))
#define POPUP_CTL(pfunc,pan,tit,ls) (desc.type=LWT_CHOICE,desc.choice.items=ls,desc.choice.vertical=3,\
                          (*pfunc->addControl) (pan, "PopupControl", &desc, tit))
#define WPOPUP_CTL(pfunc,pan,tit,ls,w) (desc.type=LWT_CHOICE,desc.choice.vertical=w, desc.choice.items=ls,\
                          (*pfunc->addControl) (pan, "PopupControl", &desc, tit))
#define FLOAT_CTL(pfunc,pan,tit)  (desc.type=LWT_FLOAT,\
                                                                        (*pfunc->addControl) (pan, "FloatControl", &desc, tit))
// RO means 'read-only', uneditable text gadget
#define FLOATRO_CTL(pfunc,pan,tit)  (desc.type=LWT_FLOAT,\
                                                                        (*pfunc->addControl) (pan, "FloatInfoControl", &desc, tit))
#define FVEC_CTL(pfunc,pan,tit)         (desc.type=LWT_VFLOAT,\
                                                                        (*pfunc->addControl) (pan, "FVecControl", &desc, tit))
#define FVECRO_CTL(pfunc,pan,tit)       (desc.type=LWT_VFLOAT,\
                                                                        (*pfunc->addControl) (pan, "FVecInfoControl", &desc, tit))
#define DVEC_CTL(pfunc,pan,tit)         (desc.type=LWT_VDIST,\
                                                                        (*pfunc->addControl) (pan, "FVecControl", &desc, tit))
#define DIST_CTL(pfunc,pan,tit)   (desc.type=LWT_DISTANCE,\
                                                                        (*pfunc->addControl) (pan, "FloatControl", &desc, tit))

// a popup list of LW items (i.e. lights, objects, bones, ..search 'LWI_' )
#define ITEM_CTL(pfunc,pan,tit,glb,typ)   (desc.type=LWT_LWITEM, desc.lwitem.global=glb, desc.lwitem.itemType=typ,\
                                                                        desc.lwitem.list=NULL, desc.lwitem.count=0,desc.lwitem.width=0, \
                                                                        (*pfunc->addControl) (pan, "LWListControl", &desc, tit) )
// a popup list of LW items (i.e. lights, objects, bones, ..search 'LWI_' )
#define WITEM_CTL(pfunc,pan,tit,glb,typ,w)   (desc.type=LWT_LWITEM,desc.lwitem.global=glb, desc.lwitem.itemType=typ,\
                                                                        desc.lwitem.list=NULL,desc.lwitem.count=0,desc.lwitem.width=w,  \
                                                                        (*pfunc->addControl) (pan, "LWListControl", &desc, tit) )
// a 'do something now' button
#define BUTTON_CTL(pfunc,pan,tit)   (desc.type=LWT_INTEGER,\
                                                                        (*pfunc->addControl) (pan, "ButtonControl", &desc, tit))
#define WBUTTON_CTL(pfunc,pan,tit,w)   (desc.type=LWT_AREA,desc.area.width=w,desc.area.height=0,\
                                                                        (*pfunc->addControl) (pan, "ButtonControl", &desc, tit))
// a checkbox
#define BOOL_CTL(pfunc,pan,tit)   (desc.type=LWT_BOOLEAN,\
                                                                        (*pfunc->addControl) (pan, "BoolControl", &desc, tit))
// a selectable text button
#define BOOLBUTTON_CTL(pfunc,pan,tit)   (desc.type=LWT_BOOLEAN,\
                                                                        (*pfunc->addControl) (pan, "BoolButtonControl", &desc, tit))
#define WBOOLBUTTON_CTL(pfunc,pan,tit,w)   (desc.type=LWT_AREA,desc.area.width=w,desc.area.height=0,\
                                                                        (*pfunc->addControl) (pan, "BoolButtonControl", &desc, tit))
#define INT_CTL(pfunc,pan,tit)    (desc.type=LWT_INTEGER,\
                                                                        (*pfunc->addControl) (pan, "NumControl", &desc, tit))
#define INTRO_CTL(pfunc,pan,tit)    (desc.type=LWT_INTEGER,\
                                                                        (*pfunc->addControl) (pan, "NumInfoControl", &desc, tit))

#define SLIDER_CTL(pfunc,pan,tit,w,mn,mx)    (desc.type=LWT_RANGE,desc.range.width=w,\
                                                                        desc.range.min=mn,desc.range.max=mx,\
                                                                        (*pfunc->addControl)(pan, "SliderControl", &desc, tit))

#define VSLIDER_CTL(pfunc,pan,tit,w,mn,mx)    (desc.type=LWT_RANGE,desc.range.width=w,\
                                                                        desc.range.min=mn,desc.range.max=mx,\
                                                                        (*pfunc->addControl)(pan, "VSlideControl", &desc, tit))

#define HSLIDER_CTL(pfunc,pan,tit,w,mn,mx)    (desc.type=LWT_RANGE,desc.range.width=w,\
                                                                        desc.range.min=mn,desc.range.max=mx,\
                                                                        (*pfunc->addControl)(pan, "HSlideControl", &desc, tit))

#define UNSLIDER_CTL(pfunc,pan,tit,w,mn,mx)    (desc.type=LWT_RANGE,desc.range.width=w,\
                                                                        desc.range.min=mn,desc.range.max=mx,\
                                                                        (*pfunc->addControl)(pan, "UnboundSlideControl", &desc, tit))

#define MINISLIDER_CTL(pfunc,pan,tit,w,mn,mx)    (desc.type=LWT_RANGE,desc.range.width=w,\
                                                                        desc.range.min=mn,desc.range.max=mx,\
                                                                        (*pfunc->addControl)(pan, "MiniSliderControl", &desc, tit))

#define IVEC_CTL(pfunc,pan,tit)    (desc.type=LWT_VINT,\
                                                                        (*pfunc->addControl) (pan, "VecControl", &desc, tit))
#define IVECRO_CTL(pfunc,pan,tit)    (desc.type=LWT_VINT,\
                                                                        (*pfunc->addControl) (pan, "VecInfoControl", &desc, tit))
#define RGB_CTL(pfunc,pan,tit)    (desc.type=LWT_VINT,\
                                                                        (*pfunc->addControl) (pan, "RGBControl", &desc, tit))
#define MINIRGB_CTL(pfunc,pan,tit)    (desc.type=LWT_VINT,\
                                                                        (*pfunc->addControl) (pan, "MiniRGBControl", &desc, tit))
#define MINIHSV_CTL(pfunc,pan,tit)    (desc.type=LWT_VINT,\
                                                                        (*pfunc->addControl) (pan, "MiniHSVControl", &desc, tit))
#define RGBVEC_CTL(pfunc,pan,tit)    (desc.type=LWT_VINT,\
                                                                        (*pfunc->addControl) (pan, "RGBVecControl", &desc, tit))
#define STR_CTL(pfunc,pan,tit,w)        (desc.type=LWT_STRING,desc.string.width=w,\
                          (*pfunc->addControl) (pan, "EditControl", &desc, tit))
#define STRRO_CTL(pfunc,pan,tit,w)      (desc.type=LWT_STRING,desc.string.width=w,\
                          (*pfunc->addControl) (pan, "InfoControl", &desc, tit))
// file name string + file-requester !
#define FILE_CTL(pfunc,pan,tit,w)       (desc.type=LWT_STRING,desc.string.width=w,\
                          (*pfunc->addControl) (pan, "FileControl", &desc, tit))
// width in pixels not char.s here
#define FILEBUTTON_CTL(pfunc,pan,tit,w)         (desc.type=LWT_STRING,desc.string.width=w,\
                          (*pfunc->addControl) (pan, "FileButtonControl", &desc, tit))
#define TEXT_CTL(pfunc,pan,tit,sts)     (desc.type=LWT_TEXT,desc.text.text=sts,\
                                                                        (*pfunc->addControl) (pan, "TextControl", &desc, tit))
#define AREA_CTL(pfunc,pan,tit,w,h)    (desc.type=LWT_AREA,desc.area.width=w,desc.area.height=h,\
                                                                        (*pfunc->addControl)(pan, "AreaControl", &desc, tit))
#define PALETTE_CTL(pfunc,pan,tit,w,h)    (desc.type=LWT_RANGE,desc.area.width=w,desc.area.height=h,\
                                                                        (*pfunc->addControl)(pan, "PaletteControl", &desc, tit))
#define OPENGL_CTL(pfunc,pan,tit,w,h)    (desc.type=LWT_AREA,desc.area.width=w,desc.area.height=h,\
                                                                        (*pfunc->addControl)(pan, "OpenGLControl", &desc, tit))
#define CUSTPOPUP_CTL(pfunc,pan,tit,w,name,num)    (desc.type=LWT_POPUP,desc.popup.width=w,desc.popup.nameFn=name,\
                                                                        desc.popup.countFn=num,\
                                                                        (*pfunc->addControl)(pan, "CustomPopupControl", &desc, tit))
#define CANVAS_CTL(pfunc,pan,tit,w,h)    (desc.type=LWT_AREA,desc.area.width=w,desc.area.height=h,\
                                                                        (*pfunc->addControl)(pan, "CanvasControl", &desc, tit))

#define POPDOWN_CTL(pfunc,pan,tit,ls)     (desc.type=LWT_CHOICE,desc.choice.vertical=MX_POPUP,\
                          desc.choice.items=ls,\
                          (*pfunc->addControl) (pan, "PopDownControl", &desc, tit))
// get intvector w/(dx,dy,count)
#define DRAGBUT_CTL(pfunc,pan,tit,w,h)    (desc.type=LWT_AREA,desc.area.width=w,desc.area.height=h,(*pfunc->addControl)(pan, "DragButtonControl", &desc, tit))
#define VDRAGBUT_CTL(pfunc,pan,tit)    (desc.type=LWT_AREA,desc.area.width=0,desc.area.height=-1,(*pfunc->addControl)(pan, "DragButtonControl", &desc, tit))
#define HDRAGBUT_CTL(pfunc,pan,tit)    (desc.type=LWT_AREA,desc.area.width=-1,desc.area.height=0,(*pfunc->addControl)(pan, "DragButtonControl", &desc, tit))
#define DRAGAREA_CTL(pfunc,pan,tit,w,h)    (desc.type=LWT_AREA,desc.area.width=w,desc.area.height=h,(*pfunc->addControl)(pan, "DragAreaControl", &desc, tit))
#define LISTBOX_CTL(pfunc,pan,tit,w,vis,name,num)    (desc.type=LWT_POPUP,desc.listbox.width=w,desc.listbox.nameFn=name,\
                                                                        desc.listbox.countFn=num,desc.listbox.visItems=vis,\
                                                                        (*pfunc->addControl)(pan, "ListBoxControl", &desc, tit))
#define PERCENT_CTL(pfunc,pan,tit)    (desc.type=LWT_RANGE,\
                                                                        desc.range.min=0,desc.range.max=100,\
                                                                        (*pfunc->addControl)(pan, "PercentControl", &desc, tit))

#define BORDER_CTL(pfunc,pan,tit,w,h)    (desc.type=LWT_AREA,desc.area.width=w,desc.area.height=h,\
                                                                        (*pfunc->addControl)(pan, "BorderControl", &desc, tit))

#define TABCHOICE_CTL(pfunc,pan,tit,ls) (desc.type=LWT_CHOICE,desc.choice.vertical=MX_HCHOICE,\
                          desc.choice.items=ls,\
                          (*pfunc->addControl) (pan, "TabChoiceControl", &desc, tit))


/*
  The folowing require a few global LWValue variables initialized as
  appropriate types.

- - - - - - CUT HERE - - - - -
static LWValue ival={LWT_INTEGER},ivecval={LWT_VINT},           // required by macros in lwpanel.h
  fval={LWT_FLOAT},fvecval={LWT_VFLOAT},sval={LWT_STRING};
- - - - - - CUT HERE - - - - -

*/



#define SET_STR(ctl,s,l) (sval.str.buf=s,sval.str.bufLen=l,(*ctl->set)(ctl, CTL_VALUE, &sval))
#define SET_INT(ctl,n) (ival.intv.value=n,(*ctl->set)(ctl, CTL_VALUE, &ival))
#define SET_FLOAT(ctl,f) (fval.flt.value=f,(*ctl->set)(ctl, CTL_VALUE, &fval))
#define SET_FVEC(ctl,x,y,z) (fvecval.fvec.val[0]=x,fvecval.fvec.val[1]=y,fvecval.fvec.val[2]=z,\
    (*ctl->set)(ctl, CTL_VALUE, &fvecval))
#define SET_IVEC(ctl,x,y,z) (ivecval.ivec.val[0]=x,ivecval.ivec.val[1]=y,ivecval.ivec.val[2]=z,\
    (*ctl->set)(ctl, CTL_VALUE, &ivecval))
#define SETV_FVEC(ctl,v) (fvecval.fvec.val[0]=v[0],fvecval.fvec.val[1]=v[1],fvecval.fvec.val[2]=v[2],\
    (*ctl->set)(ctl, CTL_VALUE, &fvecval))
#define SETV_IVEC(ctl,v) (ivecval.ivec.val[0]=v[0],ivecval.ivec.val[1]=v[1],ivecval.ivec.val[2]=v[2],\
    (*ctl->set)(ctl, CTL_VALUE, &ivecval))

#define GET_STR(ctl,s,l) (sval.str.buf=s,sval.str.bufLen=l,(*ctl->get)(ctl, CTL_VALUE, &sval))
#define GET_INT(ctl,n) ((*ctl->get)(ctl, CTL_VALUE, &ival),n=ival.intv.value)
#define GET_FLOAT(ctl,f) ((*ctl->get)(ctl, CTL_VALUE, &fval),f=fval.flt.value)
#define GET_FVEC(ctl,x,y,z) ((*ctl->get)(ctl, CTL_VALUE, &fvecval),x=fvecval.fvec.val[0],y=fvecval.fvec.val[1],z=fvecval.fvec.val[2])
#define GETV_FVEC(ctl,v) ((*ctl->get)(ctl, CTL_VALUE, &fvecval),v[0]=fvecval.fvec.val[0],v[1]=fvecval.fvec.val[1],v[2]=fvecval.fvec.val[2])
#define GET_IVEC(ctl,x,y,z) ((*ctl->get)(ctl, CTL_VALUE, &ivecval),x=ivecval.ivec.val[0],y=ivecval.ivec.val[1],z=ivecval.ivec.val[2])
#define GETV_IVEC(ctl,v) ((*ctl->get)(ctl, CTL_VALUE, &ivecval),v[0]=ivecval.ivec.val[0],v[1]=ivecval.ivec.val[1],v[2]=ivecval.ivec.val[2])

#define CON_X(ctl)      ((*ctl->get)(ctl, CTL_X, &ival),ival.intv.value)        
#define CON_Y(ctl)      ((*ctl->get)(ctl, CTL_Y, &ival),ival.intv.value)        
#define CON_W(ctl)      ((*ctl->get)(ctl, CTL_W, &ival),ival.intv.value)        
#define CON_H(ctl)      ((*ctl->get)(ctl, CTL_H, &ival),ival.intv.value)        
#define CON_LW(ctl)     ((*ctl->get)(ctl, CTL_LABELWIDTH, &ival),ival.intv.value)       
#define CON_PAN(ctl)    ((*ctl->get)(ctl, CTL_PANEL, &ival),ival.intv.value)    
#define CON_PANFUN(ctl) ((*ctl->get)(ctl, CTL_PANFUN, &ival),ival.intv.value)   
#define CON_HOTX(ctl)   ((*ctl->get)(ctl, CTL_HOTX, &ival),ival.intv.value)     
#define CON_HOTY(ctl)   ((*ctl->get)(ctl, CTL_HOTY, &ival),ival.intv.value)     
#define CON_HOTW(ctl)   ((*ctl->get)(ctl, CTL_HOTW, &ival),ival.intv.value)     
#define CON_HOTH(ctl)   ((*ctl->get)(ctl, CTL_HOTH, &ival),ival.intv.value)     
#define CON_MOUSEX(ctl) ((*ctl->get)(ctl, CTL_MOUSEX, &ival),ival.intv.value)   
#define CON_MOUSEY(ctl) ((*ctl->get)(ctl, CTL_MOUSEY, &ival),ival.intv.value)   
#define MOVE_CON(ctl,x,y)       ( (ival.intv.value=x,(*ctl->set)(ctl, CTL_X, &ival)),(ival.intv.value=y,(*ctl->set)(ctl, CTL_Y, &ival)) )       
#define CON_SETEVENT(ctl,f,d) ( (ival.intv.value=(int)d,(*ctl->set)(ctl,CTL_USERDATA,&ival)), (ival.intv.value=(int)f,(*ctl->set)(ctl,CTL_USEREVENT,&ival)) )
#define ERASE_CON(ctl)  ((*ctl->draw)(ctl,DR_ERASE))
#define REDRAW_CON(ctl) ((*ctl->draw)(ctl,DR_REFRESH))
#define GHOST_CON(ctl)  ((*ctl->draw)(ctl,DR_GHOST))
#define RENDER_CON(ctl) ((*ctl->draw)(ctl,DR_RENDER))
#define UNGHOST_CON(ctl) ((*ctl->draw)(ctl,DR_RENDER))
#define ACTIVATE_CON(ctl)       (ival.intv.value=0, (*ctl->set)(ctl, CTL_ACTIVATE, &ival))      

#define PAN_GETX(pfunc,pan) ((*pfunc->get)(pan,PAN_X,(void *)&(ival.intv.value)),ival.intv.value)
#define PAN_GETY(pfunc,pan) ((*pfunc->get)(pan,PAN_Y,(void *)&(ival.intv.value)),ival.intv.value)
#define PAN_GETW(pfunc,pan) ((*pfunc->get)(pan,PAN_W,(void *)&(ival.intv.value)),ival.intv.value)
#define PAN_GETH(pfunc,pan) ((*pfunc->get)(pan,PAN_H,(void *)&(ival.intv.value)),ival.intv.value)
#define PAN_SETH(pfunc,pan,h)   ( (ival.intv.value=h,(*pfunc->set)(pan, PAN_H, &(ival.intv.value))) )
#define PAN_SETW(pfunc,pan,w)   ( (ival.intv.value=w,(*pfunc->set)(pan, PAN_W, &(ival.intv.value))) )
#define MOVE_PAN(pfunc,pan,x,y) ( (ival.intv.value=x,(*pfunc->set)(pan, PAN_X, &(ival.intv.value))),(ival.intv.value=y,(*pfunc->set)(pan, PAN_Y, &(ival.intv.value))) ) 
#define PAN_SETDATA(pfunc,pan,d)  ( ival.intv.value=(int)d,(*pfunc->set)(pan,PAN_USERDATA,(void *)&(ival.intv.value)) )
#define PAN_SETDRAW(pfunc,pan,d)  ( ival.intv.value=(int)d,(*pfunc->set)(pan,PAN_USERDRAW,(void *)&(ival.intv.value)) )
#define PAN_SETKEYS(pfunc,pan,d)  ( ival.intv.value=(int)d,(*pfunc->set)(pan,PAN_USERKEYS,(void *)&(ival.intv.value)) )
#define PAN_GETVERSION(pfunc,pan) ((*pfunc->get)(pan,PAN_VERSION,(void *)&(ival.intv.value)),ival.intv.value)

// Window and Display Manager   stuff
//              colors
#define RGB_(r,g,b)     ((long)(((r) << 16) | ((b) << 8) | (g)))
#define SYSTEM_Ic(i)    (4096 + i)
#define USER_Ic(i)      SYSTEM_Ic(40 + i)
#define Ic_NONE         SYSTEM_Ic(0)
#define Ic_INVERT       SYSTEM_Ic(1)
#define Ic_BG           SYSTEM_Ic(2)
#define Ic_NORMAL       SYSTEM_Ic(3)
#define Ic_HEADLINE     SYSTEM_Ic(4)
#define Ic_INFO_BG      SYSTEM_Ic(5)
#define Ic_INFO_IMG     SYSTEM_Ic(6)
#define Ic_EDIT_BG      SYSTEM_Ic(7)
#define Ic_EDIT_IMG     SYSTEM_Ic(8)
#define Ic_EDIT_SEL     SYSTEM_Ic(9)
#define Ic_GLINT        SYSTEM_Ic(10)
#define Ic_SHADOW       SYSTEM_Ic(11)


        #define Nc_BLACK        SYSTEM_Ic(10)
        #define Nc_SHADOW       SYSTEM_Ic(11)
        #define Nc_LOWERED      SYSTEM_Ic(12)
        #define Nc_BG           Ic_BG
        #define Nc_RAISED       SYSTEM_Ic(13)
        #define Nc_HILIGHT      SYSTEM_Ic(14)
        #define Nc_0_STAT       SYSTEM_Ic(15)
        #define Nc_1_STAT       SYSTEM_Ic(16)
        #define Nc_2_STAT       SYSTEM_Ic(17)
        #define Nc_3_STAT       SYSTEM_Ic(18)
        #define Nc_4_STAT       SYSTEM_Ic(19)
        #define Nc_5_STAT       SYSTEM_Ic(20)
        #define Nc_0_DLOG       SYSTEM_Ic(21)
        #define Nc_1_DLOG       SYSTEM_Ic(22)
        #define Nc_2_DLOG       SYSTEM_Ic(23)
        #define Nc_3_DLOG       SYSTEM_Ic(24)
        #define Nc_4_DLOG       SYSTEM_Ic(25)
        #define Nc_5_DLOG       SYSTEM_Ic(26)
        #define Nc_0_DOIT       SYSTEM_Ic(27)
        #define Nc_1_DOIT       SYSTEM_Ic(28)
        #define Nc_2_DOIT       SYSTEM_Ic(29)
        #define Nc_3_DOIT       SYSTEM_Ic(30)
        #define Nc_4_DOIT       SYSTEM_Ic(31)
        #define Nc_5_DOIT       SYSTEM_Ic(32)
        #define Nc_0_DRAG       SYSTEM_Ic(33)
        #define Nc_1_DRAG       SYSTEM_Ic(34)
        #define Nc_2_DRAG       SYSTEM_Ic(35)
        #define Nc_3_DRAG       SYSTEM_Ic(36)
        #define Nc_4_DRAG       SYSTEM_Ic(37)
        #define Nc_5_DRAG       SYSTEM_Ic(38)



#define COLOR_BG        Ic_BG   // 0
#define COLOR_DK_YELLOW Ic_EDIT_IMG
#define COLOR_DK_GREY   Ic_SHADOW //2
#define COLOR_LT_YELLOW Ic_EDIT_SEL     // 3
#define COLOR_MD_GREY   Ic_B_FACE // 4
#define COLOR_LT_GREY   Ic_GLINT //5
#define COLOR_BLACK     Ic_NORMAL  // 6
#define COLOR_WHITE     Ic_HEADLINE      // 7

//                      input qualifiers
typedef int              InputMode;
#define IQ_CTRL         1
#define IQ_SHIFT        2
#define IQ_ALT          4
#define IQ_CONSTRAIN    8
#define IQ_ADJUST       16
#define MOUSE_LEFT      32
#define MOUSE_MID       64
#define MOUSE_RIGHT     96
#define MOUSE_DOWN              128
#define RCLICK(code)            ( (code&MOUSE_LEFT) && (code&MOUSE_MID) ? 1:0)
#define LCLICK(code)            ( (code&MOUSE_LEFT) && (!(code&MOUSE_MID)) ? 1:0)
#define MCLICK(code)            ( (!(code&MOUSE_LEFT)) && (code&MOUSE_MID) ? 1:0)

//                      keys
/*
DualKey codes in the range of 1 to 255 are mapped alphabetic keys using the character encoding native to the host system.  Also in this
range are the special keys which map to control codes, like tab and CR.
Non-alphabetic keys are mapped into the 0x1000 to 0x2000 range according to the following table.  F11 and F12 will never be generated
on the Amiga, and OPAREN and CPAREN are only generated on the Amiga.


Input processing can be done directly on the DualKey events by
providing callbacks for this purpose.  A callback will be called for
each key-down or key-up event that results in a DualKey.  Note that
some will result in two DualKey events (thus the name), so clients
should be prepared to get two events for some of the same key ups or
downs.
*/
#define DK_CHAR(c)      ((DualKey) (c))
#define DK_F1           ((DualKey) 0x1001)
#define DK_F2           ((DualKey) 0x1002)
#define DK_F3           ((DualKey) 0x1003)
#define DK_F4           ((DualKey) 0x1004)
#define DK_F5           ((DualKey) 0x1005)
#define DK_F6           ((DualKey) 0x1006)
#define DK_F7           ((DualKey) 0x1007)
#define DK_F8           ((DualKey) 0x1008)
#define DK_F9           ((DualKey) 0x1009)
#define DK_F10          ((DualKey) 0x100A)
#define DK_F11          ((DualKey) 0x100B)
#define DK_F12          ((DualKey) 0x100C)

#define DK_SC_LEFT      ((DualKey) 0x1010)
#define DK_SC_RIGHT     ((DualKey) 0x1011)
#define DK_SC_UP        ((DualKey) 0x1012)
#define DK_SC_DOWN      ((DualKey) 0x1013)

#define DK_DELETE       ((DualKey) 0x1020)
#define DK_HELP         ((DualKey) 0x1021)
#define DK_RETURN       ((DualKey) 0x1022)

#define DK_TOP_0        ((DualKey) 0x1030)
#define DK_TOP_1        ((DualKey) 0x1031)
#define DK_TOP_2        ((DualKey) 0x1032)
#define DK_TOP_3        ((DualKey) 0x1033)
#define DK_TOP_4        ((DualKey) 0x1034)
#define DK_TOP_5        ((DualKey) 0x1035)
#define DK_TOP_6        ((DualKey) 0x1036)
#define DK_TOP_7        ((DualKey) 0x1037)
#define DK_TOP_8        ((DualKey) 0x1038)
#define DK_TOP_9        ((DualKey) 0x1039)

#define DK_PAD_0        ((DualKey) 0x1040)
#define DK_PAD_1        ((DualKey) 0x1041)
#define DK_PAD_2        ((DualKey) 0x1042)
#define DK_PAD_3        ((DualKey) 0x1043)
#define DK_PAD_4        ((DualKey) 0x1044)
#define DK_PAD_5        ((DualKey) 0x1045)
#define DK_PAD_6        ((DualKey) 0x1046)
#define DK_PAD_7        ((DualKey) 0x1047)
#define DK_PAD_8        ((DualKey) 0x1048)
#define DK_PAD_9        ((DualKey) 0x1049)

#define DK_PAD_OPAREN   ((DualKey) 0x1050)
#define DK_PAD_CPAREN   ((DualKey) 0x1051)
#define DK_PAD_SLASH    ((DualKey) 0x1052)
#define DK_PAD_STAR     ((DualKey) 0x1053)
#define DK_PAD_DASH     ((DualKey) 0x1054)
#define DK_PAD_PLUS     ((DualKey) 0x1055)
#define DK_PAD_DECIMAL  ((DualKey) 0x1056)
#define DK_PAD_ENTER    ((DualKey) 0x1057)

#define DK_ALT          ((DualKey) 0x1060)
#define DK_SHIFT        ((DualKey) 0x1061)
#define DK_CTRL         ((DualKey) 0x1062)



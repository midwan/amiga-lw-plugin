// gui_help.c -- Some handy functions for organizing LW Panel Services panels
// Arnie Cachelin, Newtek Inc.
// 1/31/96

#include <math.h>
#include <splug.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <lwran.h>
#include <lwpanel.h>
#include <gui_help.h>

// return largest LabelWidth in panel
static LWPanControlDesc   desc;  // required by macros in lwpanel.h
static LWValue ival={LWT_INTEGER},ivecval={LWT_VINT},           // required by macros in lwpanel.h
  fval={LWT_FLOAT},fvecval={LWT_VFLOAT},sval={LWT_STRING};

int maxLabel(LWPanelFuncs *pf,LWPanelID pan)
{
        LWControl       *ctl;
        int     mw=0,w;
        for(ctl = (*pf->nextControl)(pan,NULL); ctl; ctl = (*pf->nextControl)(pan,ctl) )
        {
                w = CON_LW(ctl);
                if(w>mw) mw=w;
        }
        return(mw);
}

// return largest LabelWidth in panel
int maxWidth(LWPanelFuncs *pf,LWPanelID pan)
{
        LWControl       *ctl;
        int     mw=0,w;
        for(ctl = (*pf->nextControl)(pan,NULL); ctl; ctl = (*pf->nextControl)(pan,ctl) )
        {
                w = CON_W(ctl);
                if(w>mw) mw=w;
        }
        return(mw);
}

void ctlCenter(int lw,LWControl *con)
{
        int x,y;
        if(con)
        {
                x = CON_X(con);
                y = lw - CON_W(con);
                x += y/2;
                y = CON_Y(con);
                MOVE_CON(con,x,y);
        }
}

// Move control to match LabelWidth
void ctlVAlign(int lw,LWControl *con)
{
        int x,y;
        if(con)
        {
                x = CON_X(con);
                x += lw - CON_LW(con);
                y = CON_Y(con);
                MOVE_CON(con,x,y);
        }
}

// Move control to below the first
void ctlStack(LWControl *con1,LWControl *con2)
{
        int x,y;
        if(con1 && con2)
        {
                y = CON_Y(con1);
                y += CON_H(con1);
                x = CON_X(con2);
                MOVE_CON(con2,x,y);
        }
}

// Move up to 6 controls to behind the first.  Use NULL for unneeded controls.
void ctlOneLine(LWControl *con1,LWControl *con2,LWControl *con3,LWControl *con4,LWControl *con5,LWControl *con6)
{
        int x,y;
        if(con1 && con2)
        {
                x = CON_X(con1);
                y = CON_Y(con1);
                x += CON_W(con1);
                MOVE_CON(con2,x,y);
                if(!con3) return;
                x += CON_W(con2);
                MOVE_CON(con3,x,y);
                if(!con4) return;
                x += CON_W(con3);
                MOVE_CON(con4,x,y);
                if(!con5) return;
                x += CON_W(con4);
                MOVE_CON(con5,x,y);
                if(!con6) return;
                x += CON_W(con5);
                MOVE_CON(con6,x,y);
                x += CON_W(con6);
        }
}

void ctlOutLine(LWControl *con1,LWControl *con2,LWControl *con3,LWControl *con4,LWControl *con5,LWControl *con6)
{
        int x,y,x2,y2,w,h,tx;
        LWPanelFuncs *panl;
        LWPanelID               panID;
        DrawFuncs       *df;
        if(!(panl = (LWPanelFuncs *)CON_PANFUN(con1)) )
                return;
        if(!(panID = (void *)CON_PAN(con1))     )
                return;
        df = panl->drawFuncs;
        if(con1)
        {
                x = CON_X(con1);
                y = CON_Y(con1);
                w = CON_W(con1);
                h = CON_H(con1);
                x2 = x+w;
                y2 = y+h;
                if(con2)
                {
                        tx = CON_X(con2);
                        if(tx<x) x=tx;
                        tx += CON_W(con2);
                        if(tx>x2) x2=tx;
                        tx = CON_Y(con2);
                        if(tx<y) y=tx;
                        tx += CON_H(con2);
                        if(tx>y2) y2=tx;
                }
                if(con3)
                {
                        tx = CON_X(con3);
                        if(tx<x) x=tx;
                        tx += CON_W(con3);
                        if(tx>x2) x2=tx;
                        tx = CON_Y(con3);
                        if(tx<y) y=tx;
                        tx += CON_H(con3);
                        if(tx>y2) y2=tx;
                }
                if(con4)
                {
                        tx = CON_X(con4);
                        if(tx<x) x=tx;
                        tx += CON_W(con4);
                        if(tx>x2) x2=tx;
                        tx = CON_Y(con4);
                        if(tx<y) y=tx;
                        tx += CON_H(con4);
                        if(tx>y2) y2=tx;
                }
                if(con5)
                {
                        tx = CON_X(con5);
                        if(tx<x) x=tx;
                        tx += CON_W(con5);
                        if(tx>x2) x2=tx;
                        tx = CON_Y(con5);
                        if(tx<y) y=tx;
                        tx += CON_H(con5);
                        if(tx>y2) y2=tx;
                }
                if(con6)
                {
                        tx = CON_X(con6);
                        if(tx<x) x=tx;
                        tx += CON_W(con6);
                        if(tx>x2) x2=tx;
                        tx = CON_Y(con6);
                        if(tx<y) y=tx;
                        tx += CON_H(con6);
                        if(tx>y2) y2=tx;
                }
                w = x2 - x;
                h= y2 - y;
                (*df->drawBorder)(panID,1,x,y,w,h);
        }
}

void ctlMarkUp(LWControl *con1)
{
        int x,y,lw,w,h,x2,y2;
        LWPanelFuncs *panl;
        LWPanelID               panID;
        DrawFuncs       *df;
        if(!(panl = (LWPanelFuncs *)CON_PANFUN(con1)) )
                return;
        if(!(panID = (void *)CON_PAN(con1))     )
                return;
        df = panl->drawFuncs;
        if(con1)
        {
                x = CON_X(con1);
                y = CON_Y(con1);
                w = CON_W(con1);
                h = CON_H(con1);
                lw = CON_LW(con1);
                x2 = x+w;
                y2 = y+h;
                (*df->drawLine)(panID,Ic_HEADLINE,x,y,x2,y);
                (*df->drawLine)(panID,Ic_HEADLINE,x,y2,x2,y2);
                (*df->drawLine)(panID,Ic_HEADLINE,x,y,x,y2);
                (*df->drawLine)(panID,Ic_HEADLINE,lw+x,y,lw+x,y2);
                (*df->drawLine)(panID,Ic_HEADLINE,x2,y,x2,y2);
                x = CON_HOTX(con1);
                y = CON_HOTY(con1);
                w = CON_HOTW(con1);
                h = CON_HOTH(con1);
                x2 = x+w;
                y2 = y+h;
                (*df->drawLine)(panID,Ic_EDIT_SEL,x,y,x2,y);
                (*df->drawLine)(panID,Ic_EDIT_SEL,x,y2,x2,y2);
                (*df->drawLine)(panID,Ic_EDIT_SEL,x,y,x,y2);
                (*df->drawLine)(panID,Ic_EDIT_SEL,x2,y,x2,y2);
        }
}

void panelMarkUp(LWPanelFuncs *panl, LWPanelID  panID)
{
        LWControl *ctl = NULL;
        while ( ctl = (*panl->nextControl)(panID,ctl))
                ctlMarkUp(ctl);

}

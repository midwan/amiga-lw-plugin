// Gui_Help.h -- prototypes for utility functions

int maxLabel(LWPanelFuncs *,LWPanelID );
int maxWidth(LWPanelFuncs *,LWPanelID );
void ctlCenter(int ,LWControl *);
void ctlVAlign(int ,LWControl *);
void ctlStack(LWControl *,LWControl *);
void ctlOneLine(LWControl *,LWControl *,LWControl *,LWControl *,LWControl *,LWControl *);
void ctlOutLine(LWControl *,LWControl *,LWControl *,LWControl *,LWControl *,LWControl *);
void ctlMarkUp(LWControl *);
void panelMarkUp(LWPanelFuncs *, LWPanelID);

/*
 * Contributed by Steve Day / Linear Designs
 * 8th May 1998
 */

#define VALIGN_TOP  0
#define VALIGN_MIDDLE 1
#define VALIGN_BOTTOM 2

void ctrlAlignH(int, LWControl *,LWControl *,LWControl *,LWControl *,LWControl *,LWControl *);

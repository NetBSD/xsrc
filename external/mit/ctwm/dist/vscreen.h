/*
 * Copyright 1992 Claude Lecommandeur.
 */
#ifndef _CTWM_VSCREEN_H
#define _CTWM_VSCREEN_H

struct VirtualScreen {
	int   x, y, w, h;             /* x,y relative to XineramaRoot */
	Window window;
	/* Boolean main; */
	struct WorkSpaceWindow *wsw;
	struct VirtualScreen *next;
};

void InitVirtualScreens(ScreenInfo *scr);
#ifdef VSCREEN
VirtualScreen *findIfVScreenOf(int x, int y);
#endif
char *CtwmGetVScreenMap(Display *display, Window rootw);
bool CtwmSetVScreenMap(Display *display, Window rootw,
                       struct VirtualScreen *firstvs);

void DisplayWin(VirtualScreen *vs, TwmWindow *tmp_win);
void ReparentFrameAndIcon(TwmWindow *tmp_win);
void Vanish(VirtualScreen *vs, TwmWindow *tmp_win);

#endif /* _CTWM_VSCREEN_H */

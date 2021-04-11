/*
 * Copyright 1992 Claude Lecommandeur.
 */

#ifndef _CTWM_WORKMGR_H
#define _CTWM_WORKMGR_H

/* General creation/WSM drawing */
void InitWorkSpaceManagerContext(void);
void ConfigureWorkSpaceManager(void);
void CreateWorkSpaceManager(void);
void PaintWorkSpaceManager(VirtualScreen *vs);
void WMgrHandleExposeEvent(VirtualScreen *vs, XEvent *event);

void WMgrToggleState(VirtualScreen *vs);
void WMgrSetMapState(VirtualScreen *vs);
void WMgrSetButtonsState(VirtualScreen *vs);

/* Events */
void WMgrHandleKeyReleaseEvent(VirtualScreen *vs, XEvent *event);
void WMgrHandleKeyPressEvent(VirtualScreen *vs, XEvent *event);
void WMgrHandleButtonEvent(VirtualScreen *vs, XEvent *event);

/* Map state handling bits */
void WMapMapWindow(TwmWindow *win);
void WMapDeIconify(TwmWindow *win);
void WMapIconify(TwmWindow *win);
void WMapSetupWindow(TwmWindow *win, int x, int y, int w, int h);
void WMapRaiseLower(TwmWindow *win);
void WMapLower(TwmWindow *win);
void WMapRaise(TwmWindow *win);
void WMapRestack(WorkSpace *ws);

/* Map state drawing / state */
void WMapUpdateIconName(TwmWindow *win);
void WMapRedrawName(VirtualScreen *vs, WinList *wl);

void WMapAddWindow(TwmWindow *win);
void WMapAddWindowToWorkspace(TwmWindow *win, WorkSpace *ws);
void WMapRemoveWindow(TwmWindow *win);
void WMapRemoveWindowFromWorkspace(TwmWindow *win, WorkSpace *ws);

/* Util */
bool WMapWindowMayBeAdded(TwmWindow *win);

#endif /* _CTWM_WORKMGR_H */

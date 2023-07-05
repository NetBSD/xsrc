/*
 * Copyright 2014 Olaf Seibert
 */

#ifndef _CTWM_EWMH_H
#define _CTWM_EWMH_H

/*
 * Switch for experimental code to treat a Desktop window as a Root
 * window for the purposes of key and button bindings.
 * It doesn't work as nicely as I hoped though; maybe I'll get some
 * better idea.
 */
/* #define EWMH_DESKTOP_ROOT */

typedef enum EwmhWindowType {
	wt_Normal,
	wt_Desktop,
	wt_Dock,
} EwmhWindowType;

/*
 * The window is to reserve space at the edge of the screen.
 */
typedef struct EwmhStrut {
	struct EwmhStrut *next;
	struct TwmWindow *win;

	int left;
	int right;
	int top;
	int bottom;
} EwmhStrut;

#define EWMH_HAS_STRUT                  0x0001

#define EWMH_STATE_MAXIMIZED_VERT       0x0010  /* for _NET_WM_STATE */
#define EWMH_STATE_MAXIMIZED_HORZ       0x0020  /* for _NET_WM_STATE */
#define EWMH_STATE_FULLSCREEN           0x0040  /* for _NET_WM_STATE */
#define EWMH_STATE_SHADED               0x0080  /* for _NET_WM_STATE */
#define EWMH_STATE_ABOVE                0x0100  /* for _NET_WM_STATE */
#define EWMH_STATE_BELOW                0x0200  /* for _NET_WM_STATE */
#define EWMH_STATE_ALL                  0xFFF0

/*
 * OTP priorities of the window types we recognize
 */
/* Initial vals for these types, if the user hasn't set something else */
#define EWMH_PRI_DESKTOP                -8
#define EWMH_PRI_DOCK                    4

/* STATE_FULLSCREEN windows with focus get jammed here */
#define EWMH_PRI_FULLSCREEN              6

/* STATE_ABOVE/BELOW get +/- this to what they would be otherwise */
#define EWMH_PRI_ABOVE                   2

void EwmhInit(void);
bool EwmhInitScreenEarly(ScreenInfo *scr);
void EwmhInitScreenLate(ScreenInfo *scr);
#ifdef VSCREEN
void EwmhInitVirtualRoots(ScreenInfo *scr);
#endif
void EwmhTerminate(void);
void EwmhSelectionClear(XSelectionClearEvent *sev);
bool EwmhClientMessage(XClientMessageEvent *msg);
Image *EwmhGetIcon(ScreenInfo *scr, TwmWindow *twm_win);
int EwmhHandlePropertyNotify(XPropertyEvent *event, TwmWindow *twm_win);
void EwmhSet_NET_WM_DESKTOP(TwmWindow *twm_win);
void EwmhSet_NET_WM_DESKTOP_ws(TwmWindow *twm_win, WorkSpace *ws);
int EwmhGetOccupation(TwmWindow *twm_win);
void EwmhUnmapNotify(TwmWindow *twm_win);
void EwmhAddClientWindow(TwmWindow *new_win);
void EwmhDeleteClientWindow(TwmWindow *old_win);
void EwmhSet_NET_CLIENT_LIST_STACKING(void);
void EwmhSet_NET_ACTIVE_WINDOW(Window w);
void EwmhGetProperties(TwmWindow *twm_win);
int EwmhGetInitPriority(TwmWindow *twm_win);
bool EwmhHasBorder(TwmWindow *twm_win);
bool EwmhHasTitle(TwmWindow *twm_win);
bool EwmhOnWindowRing(TwmWindow *twm_win);
void EwmhSet_NET_FRAME_EXTENTS(TwmWindow *twm_win);
void EwmhSet_NET_SHOWING_DESKTOP(int state);
void EwmhSet_NET_WM_STATE(TwmWindow *twm_win, int changes);

#endif /* _CTWM_EWMH_H */

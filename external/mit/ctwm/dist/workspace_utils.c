/*
 * Various workspace handling and utilities.
 */

#include "ctwm.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <X11/Xatom.h>

#include "animate.h"
#include "clicktofocus.h"
#include "ctwm_atoms.h"
#include "drawing.h"
#include "functions.h"
#include "iconmgr.h"
#include "image.h"
#include "otp.h"
#include "screen.h"
#include "vscreen.h"
#include "win_ops.h"
#include "win_utils.h"
#include "workspace_manager.h"
#include "workspace_utils.h"

#ifdef EWMH
#  include "ewmh_atoms.h"
#endif


/*
 * XXX I'm not sure this should be here; maybe it's more of a per-screen
 * thing, and so should be in the Screen struct?
 */
bool useBackgroundInfo = false;


/*
 * Move the display (of a given vs) over to a new workspace.
 */
void
GotoWorkSpace(VirtualScreen *vs, WorkSpace *ws)
{
	TwmWindow            *twmWin;
	WorkSpace            *oldws, *newws;
	WList                *wl, *wl1;
	WinList              *winl;
	XSetWindowAttributes attr;
	long                 eventMask;
	IconMgr              *iconmgr;
	Window               oldw;
	Window               neww;
	TwmWindow            *focuswindow;
	VirtualScreen        *tmpvs;

	if(! Scr->workSpaceManagerActive) {
		return;
	}
	for(tmpvs = Scr->vScreenList; tmpvs != NULL; tmpvs = tmpvs->next) {
		if(ws == tmpvs->wsw->currentwspc) {
			XBell(dpy, 0);
			return;
		}
	}
	oldws = vs->wsw->currentwspc;
	newws = ws;
	if(oldws == newws) {
		return;
	}

	/* XXX X-ref CTAG_BGDRAW in CreateWorkSpaceManager() and below */
	if(useBackgroundInfo && ! Scr->DontPaintRootWindow) {
		if(newws->image == NULL) {
			XSetWindowBackground(dpy, vs->window, newws->backcp.back);
		}
		else {
			XSetWindowBackgroundPixmap(dpy, vs->window, newws->image->pixmap);
		}
		XClearWindow(dpy, vs->window);
	}

	/* If SaveWorkspaceFocus is on, save the focus of the last window. */
	if(Scr->SaveWorkspaceFocus) {
		oldws->save_focus = Scr->Focus;
	}

	focuswindow = NULL;
	/* For better visual effect, the order or map/unmap is important:
	   - map from top to bottom.
	   - unmap from bottom to top.
	   - unmap after mapping.
	   The guiding factor: at any point during the transition, something
	   should be visible only if it was visible before the transition or if
	   it will be visible at the end.  */
	OtpCheckConsistency();

	for(twmWin = OtpTopWin(); twmWin != NULL;
	                twmWin = OtpNextWinDown(twmWin)) {

		if(OCCUPY(twmWin, newws)) {
			if(!twmWin->vs) {
				DisplayWin(vs, twmWin);
			}
#ifdef EWMH
			if(OCCUPY(twmWin, oldws)) {
				/*
				 * If the window remains visible, re-order the workspace
				 * numbers in NET_WM_DESKTOP.
				 */
				EwmhSet_NET_WM_DESKTOP_ws(twmWin, newws);
			}
#endif
		}
	}

	for(twmWin = OtpBottomWin(); twmWin != NULL;
	                twmWin = OtpNextWinUp(twmWin)) {
		if(twmWin->vs == vs) {
			if(!OCCUPY(twmWin, newws)) {

				Vanish(vs, twmWin);
#ifdef VSCREEN
				/*
				 * Now that the window has Vanished from one virtual screen,
				 * check to see if it is wanted on another one.
				 * This is relatively rare, so don't bother with the
				 * top-to-bottom order here.
				 */
				if(Scr->numVscreens > 1) {
					VirtualScreen *tvs;
					for(tvs = Scr->vScreenList; tvs != NULL; tvs = tvs->next) {
						if(tvs == vs) { /* no, not back on the old one */
							continue;
						}
						if(OCCUPY(twmWin, tvs->wsw->currentwspc)) {
							DisplayWin(tvs, twmWin);
							break;
						}
					}
				}
#endif
			}
			else if(twmWin->hasfocusvisible) {
				focuswindow = twmWin;
				SetFocusVisualAttributes(focuswindow, false);
			}
		}
	}
	OtpCheckConsistency();

	/*
	   Reorganize icon manager window lists
	*/
	for(twmWin = Scr->FirstWindow; twmWin != NULL; twmWin = twmWin->next) {
		wl = twmWin->iconmanagerlist;
		if(wl == NULL) {
			continue;
		}
		if(OCCUPY(wl->iconmgr->twm_win, newws)) {
			continue;
		}
		wl1 = wl;
		wl  = wl->nextv;
		while(wl != NULL) {
			if(OCCUPY(wl->iconmgr->twm_win, newws)) {
				break;
			}
			wl1 = wl;
			wl  = wl->nextv;
		}
		if(wl != NULL) {
			wl1->nextv = wl->nextv;
			wl->nextv  = twmWin->iconmanagerlist;
			twmWin->iconmanagerlist = wl;
		}
	}
	wl = NULL;
	for(iconmgr = newws->iconmgr; iconmgr; iconmgr = iconmgr->next) {
		if(iconmgr->first) {
			wl = iconmgr->first;
			break;
		}
	}
	CurrentIconManagerEntry(wl);
	if(focuswindow) {
		SetFocusVisualAttributes(focuswindow, true);
	}
	vs->wsw->currentwspc = newws;
	if(Scr->ReverseCurrentWorkspace && vs->wsw->state == WMS_map) {
		MapSubwindow *msw = vs->wsw->mswl [oldws->number];
		for(winl = msw->wl; winl != NULL; winl = winl->next) {
			WMapRedrawName(vs, winl);
		}
		msw = vs->wsw->mswl [newws->number];
		for(winl = msw->wl; winl != NULL; winl = winl->next) {
			WMapRedrawName(vs, winl);
		}
	}
	else if(vs->wsw->state == WMS_buttons) {
		ButtonSubwindow *bsw = vs->wsw->bswl [oldws->number];
		PaintWsButton(WSPCWINDOW, vs, bsw->w, oldws->label, oldws->cp, off);
		bsw = vs->wsw->bswl [newws->number];
		PaintWsButton(WSPCWINDOW, vs, bsw->w, newws->label, newws->cp,  on);
	}
	oldws->iconmgr = Scr->iconmgr;
	Scr->iconmgr = newws->iconmgr;

	/* XXX X-ref CTAG_BGDRAW in CreateWorkSpaceManager() and above */
	oldw = vs->wsw->mswl [oldws->number]->w;
	neww = vs->wsw->mswl [newws->number]->w;
	if(useBackgroundInfo) {
		if(oldws->image == NULL || Scr->NoImagesInWorkSpaceManager) {
			XSetWindowBackground(dpy, oldw, oldws->backcp.back);
		}
		else {
			XSetWindowBackgroundPixmap(dpy, oldw, oldws->image->pixmap);
		}
	}
	else {
		if(Scr->workSpaceMgr.defImage == NULL || Scr->NoImagesInWorkSpaceManager) {
			XSetWindowBackground(dpy, oldw, Scr->workSpaceMgr.defColors.back);
		}
		else {
			XSetWindowBackgroundPixmap(dpy, oldw, Scr->workSpaceMgr.defImage->pixmap);
		}
	}
	attr.border_pixel = Scr->workSpaceMgr.defBorderColor;
	XChangeWindowAttributes(dpy, oldw, CWBorderPixel, &attr);

	if(Scr->workSpaceMgr.curImage == NULL) {
		if(Scr->workSpaceMgr.curPaint) {
			XSetWindowBackground(dpy, neww, Scr->workSpaceMgr.curColors.back);
		}
	}
	else {
		XSetWindowBackgroundPixmap(dpy, neww, Scr->workSpaceMgr.curImage->pixmap);
	}
	attr.border_pixel =  Scr->workSpaceMgr.curBorderColor;
	XChangeWindowAttributes(dpy, neww, CWBorderPixel, &attr);

	XClearWindow(dpy, oldw);
	XClearWindow(dpy, neww);

	eventMask = mask_out_event(Scr->Root, PropertyChangeMask);

	XChangeProperty(dpy, Scr->Root, XA_WM_CURRENTWORKSPACE, XA_STRING, 8,
	                PropModeReplace, (unsigned char *) newws->name, strlen(newws->name));
#ifdef EWMH
	{
		long number = newws->number;
		/*
		 * TODO: this should probably not use Scr->Root but ->XineramaRoot.
		 * That is the real root window if we're using virtual screens.
		 * Also, on the real root it would need values for each of the
		 * virtual roots, but that doesn't fit in the EWMH ideas.
		 */
		XChangeProperty(dpy, Scr->Root, XA__NET_CURRENT_DESKTOP,
		                XA_CARDINAL, 32,
		                PropModeReplace, (unsigned char *) &number, 1);
	}
#endif /* EWMH */

	restore_mask(Scr->Root, eventMask);

	/*    XDestroyWindow (dpy, cachew);*/
	if(Scr->ChangeWorkspaceFunction.func != 0) {
		char *action;
		XEvent event;

		action = Scr->ChangeWorkspaceFunction.item ?
		         Scr->ChangeWorkspaceFunction.item->action : NULL;
		ExecuteFunction(Scr->ChangeWorkspaceFunction.func, action,
		                (Window) 0, NULL, &event, C_ROOT, false);
	}

	/* If SaveWorkspaceFocus is on, try to restore the focus to the last
	   window which was focused when we left this workspace. */
	if(Scr->SaveWorkspaceFocus && newws->save_focus) {
		twmWin = newws->save_focus;
		if(OCCUPY(twmWin, newws)) {     /* check should not even be needed anymore */
			WarpToWindow(twmWin, false);
		}
		else {
			newws->save_focus = NULL;
		}
	}

	/* keep track of the order of the workspaces across restarts */
	CtwmSetVScreenMap(dpy, Scr->Root, Scr->vScreenList);

	XSync(dpy, 0);
	if(Scr->ClickToFocus || Scr->SloppyFocus) {
		set_last_window(newws);
	}
	MaybeAnimate = true;
}



/*
 * Various frontends to GotoWorkSpace()
 */

/*
 * Simplify redundant checks.  If no multiple workspaces, or no vs given
 * to the func, there's nothing to do.
 */
#define GWS_CHECK do { \
                if(! Scr->workSpaceManagerActive) {   \
                        return;                       \
                }                                     \
                if(!vs) {                             \
                        return;                       \
                }                                     \
        } while(0)

void
GotoWorkSpaceByName(VirtualScreen *vs, const char *wname)
{
	WorkSpace *ws;

	GWS_CHECK;

	ws = GetWorkspace(wname);
	if(ws == NULL) {
		return;
	}
	GotoWorkSpace(vs, ws);
}


void
GotoWorkSpaceByNumber(VirtualScreen *vs, int workspacenum)
{
	WorkSpace *ws;

	GWS_CHECK;

	for(ws = Scr->workSpaceMgr.workSpaceList; ws != NULL; ws = ws->next) {
		if(ws->number == workspacenum) {
			break;
		}
	}
	if(ws == NULL) {
		return;
	}
	GotoWorkSpace(vs, ws);
}


void
GotoPrevWorkSpace(VirtualScreen *vs)
{
	WorkSpace *ws1, *ws2;

	GWS_CHECK;

	ws1 = Scr->workSpaceMgr.workSpaceList;
	if(ws1 == NULL) {
		return;
	}
	ws2 = ws1->next;

	while((ws2 != vs->wsw->currentwspc) && (ws2 != NULL)) {
		ws1 = ws2;
		ws2 = ws2->next;
	}
	GotoWorkSpace(vs, ws1);
}


void
GotoNextWorkSpace(VirtualScreen *vs)
{
	WorkSpace *ws;

	GWS_CHECK;

	ws = vs->wsw->currentwspc;
	ws = (ws->next != NULL) ? ws->next : Scr->workSpaceMgr.workSpaceList;
	GotoWorkSpace(vs, ws);
}


void
GotoRightWorkSpace(VirtualScreen *vs)
{
	WorkSpace *ws;
	int number, columns, count;

	GWS_CHECK;

	ws      = vs->wsw->currentwspc;
	number  = ws->number;
	columns = Scr->workSpaceMgr.columns;
	count   = Scr->workSpaceMgr.count;
	number++;
	if((number % columns) == 0) {
		number -= columns;
	}
	else if(number >= count) {
		number = (number / columns) * columns;
	}

	GotoWorkSpaceByNumber(vs, number);
}


void
GotoLeftWorkSpace(VirtualScreen *vs)
{
	WorkSpace *ws;
	int number, columns, count;

	GWS_CHECK;

	ws      = vs->wsw->currentwspc;
	number  = ws->number;
	columns = Scr->workSpaceMgr.columns;
	count   = Scr->workSpaceMgr.count;
	number += (number % columns) ? -1 : (columns - 1);
	if(number >= count) {
		number = count - 1;
	}
	GotoWorkSpaceByNumber(vs, number);
}


void
GotoUpWorkSpace(VirtualScreen *vs)
{
	WorkSpace *ws;
	int number, lines, columns, count;

	GWS_CHECK;

	ws      = vs->wsw->currentwspc;
	number  = ws->number;
	lines   = Scr->workSpaceMgr.lines;
	columns = Scr->workSpaceMgr.columns;
	count   = Scr->workSpaceMgr.count;
	number -=  columns;
	if(number < 0) {
		number += lines * columns;
		/* If the number of workspaces is not a multiple of nr of columns */
		if(number >= count) {
			number -= columns;
		}
	}
	GotoWorkSpaceByNumber(vs, number);
}


void
GotoDownWorkSpace(VirtualScreen *vs)
{
	WorkSpace *ws;
	int number, columns, count;

	GWS_CHECK;

	ws      = vs->wsw->currentwspc;
	number  = ws->number;
	columns = Scr->workSpaceMgr.columns;
	count   = Scr->workSpaceMgr.count;
	number +=  columns;
	if(number >= count) {
		number %= columns;
	}
	GotoWorkSpaceByNumber(vs, number);
}

#undef GWS_CHECK



/*
 * Show the background (by hiding all windows) or undo it.
 * f.showbackground, also can be called via EWMH bits.
 *
 * newstate is the desired showing state.
 * Pass -1 to toggle, 1 to show the background,
 * or 0 to re-show the windows.
 *
 * XXX Doesn't really belong here; more of a functions.c-ish thing
 * probably.  But left here for the moment.
 */
void
ShowBackground(VirtualScreen *vs, int newstate)
{
	static int state = 0;
	TwmWindow *twmWin;

	if(newstate == state) {
		return;
	}

	if(state) {
		for(twmWin = Scr->FirstWindow; twmWin != NULL; twmWin = twmWin->next) {
			if(twmWin->savevs == vs) {
				DisplayWin(vs, twmWin);
			}
			twmWin->savevs = NULL;
		}
		state = 0;
	}
	else {
		for(twmWin = Scr->FirstWindow; twmWin != NULL; twmWin = twmWin->next) {
			if(twmWin->vs == vs
#ifdef EWMH
			                /* leave wt_Desktop and wt_Dock visible */
			                && twmWin->ewmhWindowType == wt_Normal
#endif /* EWMH */
			  ) {
				twmWin->savevs = twmWin->vs;
				Vanish(vs, twmWin);
			}
		}
		state = 1;
	}
#ifdef EWMH
	EwmhSet_NET_SHOWING_DESKTOP(state);
#endif /* EWMH */
}


/*
 * Get the name of the currently active WS.  Used in Execute() for
 * sub'ing in $currentworkspace in executing commands.
 */
char *
GetCurrentWorkSpaceName(VirtualScreen *vs)
{
	if(! Scr->workSpaceManagerActive) {
		return (NULL);
	}
	if(!vs) {
		vs = Scr->vScreenList;
	}
	return vs->wsw->currentwspc->name;
}


/*
 * Find workspace by name
 */
WorkSpace *
GetWorkspace(const char *wname)
{
	WorkSpace *ws;

	/* Guard */
	if(!wname) {
		return (NULL);
	}

	/* Check by label */
	for(ws = Scr->workSpaceMgr.workSpaceList; ws != NULL; ws = ws->next) {
		if(strcmp(ws->label, wname) == 0) {
			return ws;
		}
	}

	/* Check by name */
	for(ws = Scr->workSpaceMgr.workSpaceList; ws != NULL; ws = ws->next) {
		if(strcmp(ws->name, wname) == 0) {
			return ws;
		}
	}

	/* Nope */
	return NULL;
}

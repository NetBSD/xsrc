/*
 * Functions related to icon managers and the workspace manager.
 */

#include "ctwm.h"

#include "functions_internal.h"
#include "iconmgr.h"
#include "icons.h"
#include "otp.h"
#include "screen.h"
#include "vscreen.h"
#include "win_iconify.h"
#include "win_utils.h"
#include "workspace_manager.h"


/*
 * Moving around in the icon manager.
 *
 * XXX These backend funcs in iconmgr.c are passed func directly.  That's
 * a bit of a layering violation; they should maybe be changed to have
 * their own idea of directionality...
 */
DFHANDLER(upiconmgr)
{
	MoveIconManager(func);
}
DFHANDLER(downiconmgr)
{
	MoveIconManager(func);
}
DFHANDLER(lefticonmgr)
{
	MoveIconManager(func);
}
DFHANDLER(righticonmgr)
{
	MoveIconManager(func);
}
DFHANDLER(forwiconmgr)
{
	MoveIconManager(func);
}
DFHANDLER(backiconmgr)
{
	MoveIconManager(func);
}

/* XXX These two functions (Move/MoveMapped) really should be merged... */
DFHANDLER(forwmapiconmgr)
{
	MoveMappedIconManager(func);
}
DFHANDLER(backmapiconmgr)
{
	MoveMappedIconManager(func);
}

/* Moving between icon managers */
DFHANDLER(nexticonmgr)
{
	JumpIconManager(func);
}
DFHANDLER(previconmgr)
{
	JumpIconManager(func);
}


/*
 * Showing/hiding the icon managers
 */
DFHANDLER(showiconmgr)
{
	IconMgr   *i;
	WorkSpace *wl;

	/*
	 * XXX I don't think this is right; there can still be icon managers
	 * to show even if we've never set any Workspaces {}.  And
	 * HideIconManager() doesn't have this extra condition either...
	 */
	if(! Scr->workSpaceManagerActive) {
		return;
	}

	if(Scr->NoIconManagers) {
		return;
	}

	for(wl = Scr->workSpaceMgr.workSpaceList; wl != NULL; wl = wl->next) {
		for(i = wl->iconmgr; i != NULL; i = i->next) {
			/* Don't show iconmgr's with nothing in 'em */
			if(i->count == 0) {
				continue;
			}

			/* If it oughta be in a vscreen, show it */
			if(visible(i->twm_win)) {
				/* IM window */
				SetMapStateProp(i->twm_win, NormalState);
				XMapWindow(dpy, i->twm_win->w);
				OtpRaise(i->twm_win, WinWin);
				XMapWindow(dpy, i->twm_win->frame);

				/* Hide icon */
				if(i->twm_win->icon && i->twm_win->icon->w) {
					XUnmapWindow(dpy, i->twm_win->icon->w);
				}
			}

			/* Mark as shown */
			i->twm_win->mapped = true;
			i->twm_win->isicon = false;
		}
	}
}


/*
 * f.hideiconmanager is split into an external function (which is also
 * exported) because it also gets called when f.delete{,ordestroy} is
 * called on an icon manager.
 *
 * This hides all the icon managers in all the workspaces, and it doesn't
 * leave icons behind, so it's _not_ the same as just iconifying, and
 * thus not implemented by just calling Iconify(), but by doing the
 * hiding manually.
 */
DFHANDLER(hideiconmgr)
{
	HideIconManager();
}

void
HideIconManager(void)
{
	IconMgr   *i;
	WorkSpace *wl;

	if(Scr->NoIconManagers) {
		return;
	}

	for(wl = Scr->workSpaceMgr.workSpaceList; wl != NULL; wl = wl->next) {
		for(i = wl->iconmgr; i != NULL; i = i->next) {
			/* Hide the IM window */
			SetMapStateProp(i->twm_win, WithdrawnState);
			XUnmapWindow(dpy, i->twm_win->frame);

			/* Hide its icon */
			if(i->twm_win->icon && i->twm_win->icon->w) {
				XUnmapWindow(dpy, i->twm_win->icon->w);
			}

			/* Mark as pretend-iconified, even though the icon is hidden */
			i->twm_win->mapped = false;
			i->twm_win->isicon = true;
		}
	}
}


/*
 * And sorting
 */
DFHANDLER(sorticonmgr)
{
	bool save_sort = Scr->SortIconMgr;

	Scr->SortIconMgr = true;

	if(context == C_ICONMGR) {
		SortIconManager(NULL);
	}
	else if(tmp_win->isiconmgr) {
		SortIconManager(tmp_win->iconmgrp);
	}
	else {
		XBell(dpy, 0);
	}

	Scr->SortIconMgr = save_sort;
}



/*
 * Now functions related to the workspace manager
 */

/*
 * Showing/hiding it
 */
DFHANDLER(showworkspacemgr)
{
	if(! Scr->workSpaceManagerActive) {
		return;
	}

	DeIconify(Scr->currentvs->wsw->twm_win);
	OtpRaise(Scr->currentvs->wsw->twm_win, WinWin);
}

DFHANDLER(hideworkspacemgr)
{
	if(! Scr->workSpaceManagerActive) {
		return;
	}

	Iconify(Scr->currentvs->wsw->twm_win, eventp->xbutton.x_root - 5,
	        eventp->xbutton.y_root - 5);
}

DFHANDLER(toggleworkspacemgr)
{
	if(! Scr->workSpaceManagerActive) {
		return;
	}

	if(Scr->currentvs->wsw->twm_win->mapped) {
		Iconify(Scr->currentvs->wsw->twm_win, eventp->xbutton.x_root - 5,
		        eventp->xbutton.y_root - 5);
	}
	else {
		DeIconify(Scr->currentvs->wsw->twm_win);
		OtpRaise(Scr->currentvs->wsw->twm_win, WinWin);
	}
}


/*
 * Flipping around map/button state
 */
DFHANDLER(togglestate)
{
	WMgrToggleState(Scr->currentvs);
}

DFHANDLER(setbuttonsstate)
{
	WMgrSetButtonsState(Scr->currentvs);
}

DFHANDLER(setmapstate)
{
	WMgrSetMapState(Scr->currentvs);
}

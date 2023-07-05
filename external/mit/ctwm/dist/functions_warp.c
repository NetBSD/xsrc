/*
 * Functions related to the warp ring
 *
 * There are functions that are _named_ like warp-ring funcs, but aren't
 * really, and so aren't here.  Some examples are f.warphere which is
 * workspaces-related, and f.warptoscreen which is screen-related.
 *
 * There are also funcs that aren't really ring related, but I've put
 * here because they're still warping to window related, like f.warpto /
 * f.warptoiconmgr.
 */

#include "ctwm.h"

#include <string.h>

#include "functions_internal.h"
#include "iconmgr.h"
#include "list.h"
#include "otp.h"
#include "screen.h"
#include "win_iconify.h"
#include "win_ring.h"
#include "win_utils.h"


static void WarpAlongRing(XButtonEvent *ev, bool forward);


DFHANDLER(warpto)
{
	TwmWindow *tw;
	int len;

	len = strlen(action);

#ifdef WARPTO_FROM_ICONMGR
	/* XXX should be iconmgrp? */
	if(len == 0 && tmp_win && tmp_win->iconmgr) {
		printf("curren iconmgr entry: %s", tmp_win->iconmgr->Current);
	}
#endif /* #ifdef WARPTO_FROM_ICONMGR */
	for(tw = Scr->FirstWindow; tw != NULL; tw = tw->next) {
		if(!strncmp(action, tw->name, len)) {
			break;
		}
		if(match(action, tw->name)) {
			break;
		}
	}
	if(!tw) {
		for(tw = Scr->FirstWindow; tw != NULL; tw = tw->next) {
			if(!strncmp(action, tw->class.res_name, len)) {
				break;
			}
			if(match(action, tw->class.res_name)) {
				break;
			}
		}
		if(!tw) {
			for(tw = Scr->FirstWindow; tw != NULL; tw = tw->next) {
				if(!strncmp(action, tw->class.res_class, len)) {
					break;
				}
				if(match(action, tw->class.res_class)) {
					break;
				}
			}
		}
	}

	if(tw) {
		if(Scr->WarpUnmapped || tw->mapped) {
			if(!tw->mapped) {
				DeIconify(tw);
			}
			WarpToWindow(tw, Scr->RaiseOnWarp);
		}
	}
	else {
		XBell(dpy, 0);
	}
}


DFHANDLER(warptoiconmgr)
{
	TwmWindow *tw, *raisewin = NULL;
	int len;
	Window iconwin = None;

	len = strlen(action);
	if(len == 0) {
		if(tmp_win && tmp_win->iconmanagerlist) {
			raisewin = tmp_win->iconmanagerlist->iconmgr->twm_win;
			iconwin = tmp_win->iconmanagerlist->icon;
		}
		else if(Scr->iconmgr->active) {
			raisewin = Scr->iconmgr->twm_win;
			iconwin = Scr->iconmgr->active->w;
		}
	}
	else {
		for(tw = Scr->FirstWindow; tw != NULL; tw = tw->next) {
			if(strncmp(action, tw->icon_name, len) == 0) {
				if(tw->iconmanagerlist &&
				                tw->iconmanagerlist->iconmgr->twm_win->mapped) {
					raisewin = tw->iconmanagerlist->iconmgr->twm_win;
					break;
				}
			}
		}
	}

	if(raisewin) {
		OtpRaise(raisewin, WinWin);
		XWarpPointer(dpy, None, iconwin, 0, 0, 0, 0, 5, 5);
	}
	else {
		XBell(dpy, 0);
	}
}

/* Taken from vtwm version 5.3 */
DFHANDLER(ring)
{
	if(WindowIsOnRing(tmp_win)) {
		/* It's in the ring, let's take it out. */
		UnlinkWindowFromRing(tmp_win);
	}
	else {
		/* Not in the ring, so put it in. */
		AddWindowToRing(tmp_win);
	}
	/*tmp_win->ring.cursor_valid = false;*/
}


DFHANDLER(warpring)
{
	switch(((char *)action)[0]) {
		case 'n':
			WarpAlongRing(&eventp->xbutton, true);
			break;
		case 'p':
			WarpAlongRing(&eventp->xbutton, false);
			break;
		default:
			XBell(dpy, 0);
			break;
	}
}


/*
 * Synthetic function: this is used internally as the action in some
 * magic menus like the TwmWindows et al.
 */
DFHANDLER(winwarp)
{
	tmp_win = (TwmWindow *)action;

	if(! tmp_win) {
		return;
	}
	if(Scr->WarpUnmapped || tmp_win->mapped) {
		if(!tmp_win->mapped) {
			DeIconify(tmp_win);
		}
		WarpToWindow(tmp_win, Scr->RaiseOnWarp);
	}
}


/*
 * Backend util for f.warpring
 */
static void
WarpAlongRing(XButtonEvent *ev, bool forward)
{
	TwmWindow *r, *head;

	if(Scr->RingLeader) {
		head = Scr->RingLeader;
	}
	else if(!(head = Scr->Ring)) {
		return;
	}

	if(forward) {
		for(r = head->ring.next; r != head; r = r->ring.next) {
			if(r->mapped && (Scr->WarpRingAnyWhere || visible(r))) {
				break;
			}
		}
	}
	else {
		for(r = head->ring.prev; r != head; r = r->ring.prev) {
			if(r->mapped && (Scr->WarpRingAnyWhere || visible(r))) {
				break;
			}
		}
	}

	/*
	 * Note: (Scr->Focus == NULL) is necessary when we move to (or
	 * are in) a workspace that has a single window, and we're not
	 * on that window (but the window is head), and we want f.warpring
	 * to warp to it.
	 * Generalised that is also true if we are on a window but it is
	 * not on the ring.
	 * TODO: on an empty screen, it still moves the mouse cursor...
	 */

	if(r != head
	                || Scr->Focus == NULL
	                || !WindowIsOnRing(Scr->Focus)) {
		TwmWindow *p = Scr->RingLeader, *t;

		Scr->RingLeader = r;
		WarpToWindow(r, true);

		if(p && p->mapped &&
		                (t = GetTwmWindow(ev->window)) &&
		                p == t) {
			p->ring.cursor_valid = true;
			p->ring.curs_x = ev->x_root - t->frame_x;
			p->ring.curs_y = ev->y_root - t->frame_y;
#ifdef DEBUG
			/* XXX This is the Tmp_win [now] internal to the event code? */
			fprintf(stderr,
			        "WarpAlongRing: cursor_valid := true; x := %d (%d-%d), y := %d (%d-%d)\n",
			        Tmp_win->ring.curs_x, ev->x_root, t->frame_x, Tmp_win->ring.curs_y, ev->y_root,
			        t->frame_y);
#endif
			/*
			 * The check if the cursor position is inside the window is now
			 * done in WarpToWindow().
			 */
		}
	}
}

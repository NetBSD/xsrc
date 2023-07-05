/*
 * Functions related to manipulating windows.
 *
 * This doesn't include stuff related to changing their occupation (in
 * functions_workspaces.c), or moving/resizing (in
 * functions_win_moveresize.c), or a few other cases of things
 * peripherally involving windows.
 */

#include "ctwm.h"

#include <stdlib.h>

#include "colormaps.h"
#include "ctwm_atoms.h"
#include "events.h"
#include "event_handlers.h"
#include "functions.h"
#include "functions_defs.h"
#include "functions_internal.h"
#include "icons.h"
#include "occupation.h"
#include "otp.h"
#include "parse.h"
#include "screen.h"
#include "win_decorations.h"
#include "win_iconify.h"
#include "win_ops.h"
#include "win_utils.h"
#include "workspace_manager.h"



/*
 *************************************************************
 *
 * Moving windows on/off the AutoRaise/AutoLower lists
 *
 *************************************************************
 */
DFHANDLER(autoraise)
{
	tmp_win->auto_raise = !tmp_win->auto_raise;
	if(tmp_win->auto_raise) {
		++(Scr->NumAutoRaises);
	}
	else {
		--(Scr->NumAutoRaises);
	}
}

DFHANDLER(autolower)
{
	tmp_win->auto_lower = !tmp_win->auto_lower;
	if(tmp_win->auto_lower) {
		++(Scr->NumAutoLowers);
	}
	else {
		--(Scr->NumAutoLowers);
	}
}




/*
 *************************************************************
 *
 * Raising and lowering
 *
 *************************************************************
 */

/* Separate function because raise and raiseorsqueeze can both need it */
static void
raise_handler(EF_FULLPROTO)
{
	/* check to make sure raise is not from the WindowFunction */
	if(tmp_win->icon && (w == tmp_win->icon->w) && Context != C_ROOT)  {
		OtpRaise(tmp_win, IconWin);
	}
	else {
		OtpRaise(tmp_win, WinWin);
		WMapRaise(tmp_win);
	}
}

DFHANDLER(raise)
{
	raise_handler(EF_ARGS);
}

DFHANDLER(raiseorsqueeze)
{
	/* FIXME using the same double-click ConstrainedMoveTime here */
	if((eventp->xbutton.time - last_time) < ConstrainedMoveTime) {
		Squeeze(tmp_win);
		return;
	}
	last_time = eventp->xbutton.time;

	/* intentional fall-thru into F_RAISE */
	raise_handler(EF_ARGS);
}

DFHANDLER(lower)
{
	if(tmp_win->icon && (w == tmp_win->icon->w)) {
		OtpLower(tmp_win, IconWin);
	}
	else {
		OtpLower(tmp_win, WinWin);
		WMapLower(tmp_win);
	}
}

DFHANDLER(raiselower)
{
	if(!WindowMoved) {
		if(tmp_win->icon && w == tmp_win->icon->w) {
			OtpRaiseLower(tmp_win, IconWin);
		}
		else {
			OtpRaiseLower(tmp_win, WinWin);
			WMapRaiseLower(tmp_win);
		}
	}
}


/*
 * Smaller raise/lower
 */
DFHANDLER(tinyraise)
{
	/* check to make sure raise is not from the WindowFunction */
	if(tmp_win->icon && (w == tmp_win->icon->w) && Context != C_ROOT) {
		OtpTinyRaise(tmp_win, IconWin);
	}
	else {
		OtpTinyRaise(tmp_win, WinWin);
		WMapRaise(tmp_win);
	}
}

DFHANDLER(tinylower)
{
	/* check to make sure raise is not from the WindowFunction */
	if(tmp_win->icon && (w == tmp_win->icon->w) && Context != C_ROOT) {
		OtpTinyLower(tmp_win, IconWin);
	}
	else {
		OtpTinyLower(tmp_win, WinWin);
		WMapLower(tmp_win);
	}
}


/*
 * Raising/lowering a non-targetted window
 */
DFHANDLER(circleup)
{
	OtpCirculateSubwindows(Scr->currentvs, RaiseLowest);
}

DFHANDLER(circledown)
{
	OtpCirculateSubwindows(Scr->currentvs, LowerHighest);
}




/*
 *************************************************************
 *
 * Iconification and its inverse.
 *
 *************************************************************
 */
static void
iconify_handler(EF_FULLPROTO)
{
	if(tmp_win->isicon) {
		DeIconify(tmp_win);
	}
	else if(func == F_ICONIFY) {
		Iconify(tmp_win, eventp->xbutton.x_root - 5,
		        eventp->xbutton.y_root - 5);
	}
}

DFHANDLER(deiconify)
{
	iconify_handler(EF_ARGS);
}
DFHANDLER(iconify)
{
	iconify_handler(EF_ARGS);
}


/*
 * This is a synthetic function; it only exists as an action in some
 * magic menus like TwmWindows (x-ref f.winwarp as well).  It acts as a
 * sort of deiconify, so I've stuck it here.
 */
DFHANDLER(popup)
{
	/*
	 * This is a synthetic function; it exists only to be called
	 * internally from the various magic menus like TwmWindows
	 * etc.
	 */
	tmp_win = (TwmWindow *)action;
	if(! tmp_win) {
		return;
	}
	if(Scr->WindowFunction.func != 0) {
		ExecuteFunction(Scr->WindowFunction.func,
		                Scr->WindowFunction.item->action,
		                w, tmp_win, eventp, C_FRAME, false);
	}
	else {
		DeIconify(tmp_win);
		OtpRaise(tmp_win, WinWin);
	}
}




/*
 *************************************************************
 *
 * Focus locking
 *
 *************************************************************
 */
DFHANDLER(focus)
{
	if(!tmp_win->isicon) {
		if(!Scr->FocusRoot && Scr->Focus == tmp_win) {
			FocusOnRoot();
		}
		else {
			InstallWindowColormaps(0, tmp_win);
			SetFocus(tmp_win, eventp->xbutton.time);
			Scr->FocusRoot = false;
		}
	}
}

DFHANDLER(unfocus)
{
	FocusOnRoot();
}




/*
 *************************************************************
 *
 * Window destruction
 *
 *************************************************************
 */
static void
SendDeleteWindowMessage(TwmWindow *tmp, Time timestamp)
{
	send_clientmessage(tmp->w, XA_WM_DELETE_WINDOW, timestamp);
}

DFHANDLER(delete)
{
	if(tmp_win->isiconmgr) {     /* don't send ourself a message */
		HideIconManager();
		return;
	}
	if(tmp_win->iswspmgr
#ifdef WINBOX
	                || tmp_win->iswinbox
#endif
	                || (Scr->workSpaceMgr.occupyWindow
	                    && tmp_win == Scr->workSpaceMgr.occupyWindow->twm_win)) {
		XBell(dpy, 0);
		return;
	}
	if(tmp_win->protocols & DoesWmDeleteWindow) {
		SendDeleteWindowMessage(tmp_win, EventTime);
		if(ButtonPressed != -1) {
			XEvent kev;

			XMaskEvent(dpy, ButtonReleaseMask, &kev);
			if(kev.xbutton.window == tmp_win->w) {
				kev.xbutton.window = Scr->Root;
			}
			XPutBackEvent(dpy, &kev);
		}
		return;
	}
	XBell(dpy, 0);
}

DFHANDLER(destroy)
{
	if(tmp_win->isiconmgr || tmp_win->iswspmgr
#ifdef WINBOX
	                || tmp_win->iswinbox
#endif
	                || (Scr->workSpaceMgr.occupyWindow
	                    && tmp_win == Scr->workSpaceMgr.occupyWindow->twm_win)) {
		XBell(dpy, 0);
		return;
	}
	XKillClient(dpy, tmp_win->w);
	if(ButtonPressed != -1) {
		XEvent kev;

		XMaskEvent(dpy, ButtonReleaseMask, &kev);
		if(kev.xbutton.window == tmp_win->w) {
			kev.xbutton.window = Scr->Root;
		}
		XPutBackEvent(dpy, &kev);
	}
}

DFHANDLER(deleteordestroy)
{
	if(tmp_win->isiconmgr) {
		HideIconManager();
		return;
	}
	if(tmp_win->iswspmgr
#ifdef WINBOX
	                || tmp_win->iswinbox
#endif
	                || (Scr->workSpaceMgr.occupyWindow
	                    && tmp_win == Scr->workSpaceMgr.occupyWindow->twm_win)) {
		XBell(dpy, 0);
		return;
	}
	if(tmp_win->protocols & DoesWmDeleteWindow) {
		SendDeleteWindowMessage(tmp_win, EventTime);
	}
	else {
		XKillClient(dpy, tmp_win->w);
	}
	if(ButtonPressed != -1) {
		XEvent kev;

		XMaskEvent(dpy, ButtonReleaseMask, &kev);
		if(kev.xbutton.window == tmp_win->w) {
			kev.xbutton.window = Scr->Root;
		}
		XPutBackEvent(dpy, &kev);
	}
}




/*
 *************************************************************
 *
 * Messing with OnTopPriority bits
 *
 *************************************************************
 */
static void
otp_priority_handler(EF_FULLPROTO)
{
	WinType wintype;
	int pri;
	char *endp;

	if(tmp_win->icon && w == tmp_win->icon->w) {
		wintype = IconWin;
	}
	else {
		wintype = WinWin;
	}
	switch(func) {
		case F_PRIORITYSWITCHING:
			OtpToggleSwitching(tmp_win, wintype);
			break;
		case F_SETPRIORITY:
			pri = (int)strtol(action, &endp, 10);
			OtpSetPriority(tmp_win, wintype, pri,
			               (*endp == '<' || *endp == 'b') ? Below : Above);
			break;
		case F_CHANGEPRIORITY:
			OtpChangePriority(tmp_win, wintype, atoi(action));
			break;
		case F_SWITCHPRIORITY:
			OtpSwitchPriority(tmp_win, wintype);
			break;
	}

	/*
	 * Stash up our current flags if there aren't any set yet.  This is
	 * necessary because otherwise the EWMH prop we [may] stash below
	 * would be taken as gospel on restart, when it shouldn't be.
	 */
	OtpStashAflagsFirstTime(tmp_win);

#ifdef EWMH
	/*
	 * We changed the priority somehow, so we may have changed where it
	 * sits relative to the middle.  So trigger rechecking/setting of the
	 * _STATE_{ABOVE,BELOW}.  (_ABOVE in changes arg covers both)
	 */
	EwmhSet_NET_WM_STATE(tmp_win, EWMH_STATE_ABOVE);
#endif /* EWMH */
}
DFHANDLER(priorityswitching)
{
	otp_priority_handler(EF_ARGS);
}
DFHANDLER(switchpriority)
{
	otp_priority_handler(EF_ARGS);
}
DFHANDLER(setpriority)
{
	otp_priority_handler(EF_ARGS);
}
DFHANDLER(changepriority)
{
	otp_priority_handler(EF_ARGS);
}




/*
 *************************************************************
 *
 * Some misc utilities
 *
 *************************************************************
 */
DFHANDLER(saveyourself)
{
	if(tmp_win->protocols & DoesWmSaveYourself) {
		send_clientmessage(tmp_win->w, XA_WM_SAVE_YOURSELF, EventTime);
	}
	else {
		XBell(dpy, 0);
	}
}

DFHANDLER(colormap)
{
	/* XXX Window targetting; should this be on the Defer list? */
	if(strcmp(action, COLORMAP_NEXT) == 0) {
		BumpWindowColormap(tmp_win, 1);
	}
	else if(strcmp(action, COLORMAP_PREV) == 0) {
		BumpWindowColormap(tmp_win, -1);
	}
	else {
		BumpWindowColormap(tmp_win, 0);
	}
}

DFHANDLER(refresh)
{
	XSetWindowAttributes attributes;
	unsigned long valuemask;

	valuemask = CWBackPixel;
	attributes.background_pixel = Scr->Black;
	w = XCreateWindow(dpy, Scr->Root, 0, 0,
	                  Scr->rootw,
	                  Scr->rooth,
	                  0,
	                  CopyFromParent, CopyFromParent,
	                  CopyFromParent, valuemask,
	                  &attributes);
	XMapWindow(dpy, w);
	XDestroyWindow(dpy, w);
	XFlush(dpy);

}

DFHANDLER(winrefresh)
{
	if(context == C_ICON && tmp_win->icon && tmp_win->icon->w)
		w = XCreateSimpleWindow(dpy, tmp_win->icon->w,
		                        0, 0, 9999, 9999, 0, Scr->Black, Scr->Black);
	else
		w = XCreateSimpleWindow(dpy, tmp_win->frame,
		                        0, 0, 9999, 9999, 0, Scr->Black, Scr->Black);

	XMapWindow(dpy, w);
	XDestroyWindow(dpy, w);
	XFlush(dpy);
}




/*
 *************************************************************
 *
 * Window squeezing related bits
 *
 *************************************************************
 */
DFHANDLER(squeeze)
{
	Squeeze(tmp_win);
}

DFHANDLER(unsqueeze)
{
	if(tmp_win->squeezed) {
		Squeeze(tmp_win);
	}
}


DFHANDLER(movetitlebar)
{
	Window grabwin;
	Window rootw;
	int deltax = 0, newx = 0;
	int origX;
	int origNum;
	SqueezeInfo *si;

	PopDownMenu();
	if(tmp_win->squeezed ||
	                !tmp_win->squeeze_info ||
	                !tmp_win->title_w ||
	                context == C_ICON) {
		XBell(dpy, 0);
		return;
	}

	/* If the SqueezeInfo isn't copied yet, do it now */
	if(!tmp_win->squeeze_info_copied) {
		SqueezeInfo *s = malloc(sizeof(SqueezeInfo));
		if(!s) {
			return;
		}
		*s = *tmp_win->squeeze_info;
		tmp_win->squeeze_info = s;
		tmp_win->squeeze_info_copied = true;
	}
	si = tmp_win->squeeze_info;

	if(si->denom != 0) {
		int target_denom = tmp_win->frame_width;
		/*
		 * If not pixel based, scale the denominator to equal the
		 * window width, so the numerator equals pixels.
		 * That way we can just modify it by pixel units, just
		 * like the other case.
		 */

		if(si->denom != target_denom) {
			float scale = (float)target_denom / si->denom;
			si->num *= scale;
			si->denom = target_denom; /* s->denom *= scale; */
		}
	}

	/* now move the mouse */
#ifdef WINBOX
	if(tmp_win->winbox) {
		XTranslateCoordinates(dpy, Scr->Root, tmp_win->winbox->window,
		                      eventp->xbutton.x_root, eventp->xbutton.y_root,
		                      &eventp->xbutton.x_root, &eventp->xbutton.y_root, &JunkChild);
	}
#endif
	/*
	 * the event is always a button event, since key events
	 * are "weeded out" - although incompletely only
	 * F_MOVE and F_RESIZE - in HandleKeyPress().
	 */

	/*
	 * XXX This var may be actually unnecessary; it's used only
	 * once as an arg to a later X call, but during that time I
	 * don't believe anything can mutate eventp or anything near
	 * the root.  However, due to the union nature of XEvent,
	 * it's hard to be sure without more investigation, so I
	 * leave the intermediate var for now.
	 *
	 * Note that we're looking inside the XButtonEvent member
	 * here, but other bits of this code later look at the
	 * XMotionEvent piece.  This should be further investigated
	 * and resolved; they can't both be right (though the
	 * structure of the structs are such that almost all the
	 * similar elements are in the same place, at least in
	 * theory).
	 */
	rootw = eventp->xbutton.root;

	EventHandler[EnterNotify] = HandleUnknown;
	EventHandler[LeaveNotify] = HandleUnknown;

	if(!Scr->NoGrabServer) {
		XGrabServer(dpy);
	}

	grabwin = Scr->Root;
#ifdef WINBOX
	if(tmp_win->winbox) {
		grabwin = tmp_win->winbox->window;
	}
#endif
	XGrabPointer(dpy, grabwin, True,
	             ButtonPressMask | ButtonReleaseMask |
	             ButtonMotionMask | PointerMotionMask, /* PointerMotionHintMask */
	             GrabModeAsync, GrabModeAsync, grabwin, Scr->MoveCursor, CurrentTime);

#if 0   /* what's this for ? */
	if(! tmp_win->icon || w != tmp_win->icon->w) {
		XTranslateCoordinates(dpy, w, tmp_win->frame,
		                      eventp->xbutton.x,
		                      eventp->xbutton.y,
		                      &DragX, &DragY, &JunkChild);

		w = tmp_win->frame;
	}
#endif

	DragWindow = None;

	XGetGeometry(dpy, tmp_win->title_w, &JunkRoot, &origDragX, &origDragY,
	             &DragWidth, &DragHeight, &DragBW,
	             &JunkDepth);

	origX = eventp->xbutton.x_root;
	origNum = si->num;

	if(menuFromFrameOrWindowOrTitlebar) {
		/* warp the pointer to the middle of the window */
		XWarpPointer(dpy, None, Scr->Root, 0, 0, 0, 0,
		             origDragX + DragWidth / 2,
		             origDragY + DragHeight / 2);
		XFlush(dpy);
	}

	while(1) {
		long releaseEvent = menuFromFrameOrWindowOrTitlebar ?
		                    ButtonPress : ButtonRelease;
		long movementMask = menuFromFrameOrWindowOrTitlebar ?
		                    PointerMotionMask : ButtonMotionMask;

		/* block until there is an interesting event */
		XMaskEvent(dpy, ButtonPressMask | ButtonReleaseMask |
		           EnterWindowMask | LeaveWindowMask |
		           ExposureMask | movementMask |
		           VisibilityChangeMask, &Event);

		/* throw away enter and leave events until release */
		if(Event.xany.type == EnterNotify ||
		                Event.xany.type == LeaveNotify) {
			continue;
		}

		if(Event.type == MotionNotify) {
			/* discard any extra motion events before a logical release */
			while(XCheckMaskEvent(dpy,
			                      movementMask | releaseEvent, &Event)) {
				if(Event.type == releaseEvent) {
					break;
				}
			}
		}

		if(!DispatchEvent2()) {
			continue;
		}

		if(Event.type == releaseEvent) {
			break;
		}

		/* something left to do only if the pointer moved */
		if(Event.type != MotionNotify) {
			continue;
		}

		/* get current pointer pos, useful when there is lag */
		XQueryPointer(dpy, rootw, &eventp->xmotion.root, &JunkChild,
		              &eventp->xmotion.x_root, &eventp->xmotion.y_root,
		              &JunkX, &JunkY, &JunkMask);

		FixRootEvent(eventp);
#ifdef WINBOX
		if(tmp_win->winbox) {
			XTranslateCoordinates(dpy, Scr->Root, tmp_win->winbox->window,
			                      eventp->xmotion.x_root, eventp->xmotion.y_root,
			                      &eventp->xmotion.x_root, &eventp->xmotion.y_root, &JunkChild);
		}
#endif

		if(!Scr->NoRaiseMove && Scr->OpaqueMove && !WindowMoved) {
			OtpRaise(tmp_win, WinWin);
		}

		deltax = eventp->xmotion.x_root - origX;
		newx = origNum + deltax;

		/*
		 * Clamp to left and right.
		 * If we're in pixel size, keep within [ 0, frame_width >.
		 * If we're proportional, don't cross the 0.
		 * Also don't let the nominator get bigger than the denominator.
		 * Keep within [ -denom, -1] or [ 0, denom >.
		 */
		{
			int wtmp = tmp_win->frame_width; /* or si->denom; if it were != 0 */
			if(origNum < 0) {
				if(newx >= 0) {
					newx = -1;
				}
				else if(newx < -wtmp) {
					newx = -wtmp;
				}
			}
			else if(origNum >= 0) {
				if(newx < 0) {
					newx = 0;
				}
				else if(newx >= wtmp) {
					newx = wtmp - 1;
				}
			}
		}

		si->num = newx;
		/* This, finally, actually moves the title bar */
		/* XXX pressing a second button should cancel and undo this */
		SetFrameShape(tmp_win);
	}

	/*
	 * The ButtonRelease handler will have taken care of
	 * ungrabbing our pointer.
	 */
	return;
}

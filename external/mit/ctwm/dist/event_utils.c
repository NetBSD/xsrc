/*
 * Various util-ish functions for event handling.
 *
 * Many, probably most, are internal to the event code, but a few are
 * used elsewhere.
 */

#include "ctwm.h"

#include <stdio.h>

#include "event_handlers.h"
#include "event_internal.h"
#include "events.h"
#include "list.h"
#include "otp.h"
#include "screen.h"
#include "vscreen.h"
#include "win_iconify.h"
#include "workspace_manager.h"


static ScreenInfo *FindScreenInfo(Window w);


void
AutoRaiseWindow(TwmWindow *tmp)
{
	OtpRaise(tmp, WinWin);

	if(ActiveMenu && ActiveMenu->w) {
		XRaiseWindow(dpy, ActiveMenu->w);
	}
	XSync(dpy, 0);
	enter_win = NULL;
	enter_flag = true;
	raise_win = tmp;
	WMapRaise(tmp);
}

void
SetRaiseWindow(TwmWindow *tmp)
{
	enter_flag = true;
	enter_win = NULL;
	raise_win = tmp;
	leave_win = NULL;
	leave_flag = false;
	lower_win = NULL;
	XSync(dpy, 0);
}

void
AutoPopupMaybe(TwmWindow *tmp)
{
	if(LookInList(Scr->AutoPopupL, tmp->name, &tmp->class)
	                || Scr->AutoPopup) {
		if(OCCUPY(tmp, Scr->currentvs->wsw->currentwspc)) {
			if(!tmp->mapped) {
				DeIconify(tmp);
				SetRaiseWindow(tmp);
			}
		}
		else {
			tmp->mapped = true;
		}
	}
}

void
AutoLowerWindow(TwmWindow *tmp)
{
	OtpLower(tmp, WinWin);

	if(ActiveMenu && ActiveMenu->w) {
		XRaiseWindow(dpy, ActiveMenu->w);
	}
	XSync(dpy, 0);
	enter_win = NULL;
	enter_flag = false;
	raise_win = NULL;
	leave_win = NULL;
	leave_flag = true;
	lower_win = tmp;
	WMapLower(tmp);
}


/*
 * WindowOfEvent - return the window about which this event is concerned; this
 * window may not be the same as XEvent.xany.window (the first window listed
 * in the structure).
 */
Window
WindowOfEvent(XEvent *e)
{
	/*
	 * Each window subfield is marked with whether or not it is the same as
	 * XEvent.xany.window or is different (which is the case for some of the
	 * notify events).
	 */
	switch(e->type) {
		case KeyPress:
		case KeyRelease:
			return e->xkey.window;                       /* same */
		case ButtonPress:
		case ButtonRelease:
			return e->xbutton.window;                 /* same */
		case MotionNotify:
			return e->xmotion.window;                  /* same */
		case EnterNotify:
		case LeaveNotify:
			return e->xcrossing.window;                 /* same */
		case FocusIn:
		case FocusOut:
			return e->xfocus.window;                       /* same */
		case KeymapNotify:
			return e->xkeymap.window;                  /* same */
		case Expose:
			return e->xexpose.window;                        /* same */
		case GraphicsExpose:
			return e->xgraphicsexpose.drawable;      /* same */
		case NoExpose:
			return e->xnoexpose.drawable;                  /* same */
		case VisibilityNotify:
			return e->xvisibility.window;          /* same */
		case CreateNotify:
			return e->xcreatewindow.window;            /* DIFF */
		case DestroyNotify:
			return e->xdestroywindow.window;          /* DIFF */
		case UnmapNotify:
			return e->xunmap.window;                    /* DIFF */
		case MapNotify:
			return e->xmap.window;                        /* DIFF */
		case MapRequest:
			return e->xmaprequest.window;                /* DIFF */
		case ReparentNotify:
			return e->xreparent.window;              /* DIFF */
		case ConfigureNotify:
			return e->xconfigure.window;            /* DIFF */
		case ConfigureRequest:
			return e->xconfigurerequest.window;    /* DIFF */
		case GravityNotify:
			return e->xgravity.window;                /* DIFF */
		case ResizeRequest:
			return e->xresizerequest.window;          /* same */
		case CirculateNotify:
			return e->xcirculate.window;            /* DIFF */
		case CirculateRequest:
			return e->xcirculaterequest.window;    /* DIFF */
		case PropertyNotify:
			return e->xproperty.window;              /* same */
		case SelectionClear:
			return e->xselectionclear.window;        /* same */
		case SelectionRequest:
			return e->xselectionrequest.requestor;  /* DIFF */
		case SelectionNotify:
			return e->xselection.requestor;         /* same */
		case ColormapNotify:
			return e->xcolormap.window;              /* same */
		case ClientMessage:
			return e->xclient.window;                 /* same */
		case MappingNotify:
			return None;
	}
	return None;
}


void
FixRootEvent(XEvent *e)
{
	if(Scr->Root == Scr->RealRoot) {
		return;
	}

	switch(e->type) {
		case KeyPress:
		case KeyRelease:
			e->xkey.x_root -= Scr->rootx;
			e->xkey.y_root -= Scr->rooty;
			e->xkey.root    = Scr->Root;
			break;
		case ButtonPress:
		case ButtonRelease:
			e->xbutton.x_root -= Scr->rootx;
			e->xbutton.y_root -= Scr->rooty;
			e->xbutton.root    = Scr->Root;
			break;
		case MotionNotify:
			e->xmotion.x_root -= Scr->rootx;
			e->xmotion.y_root -= Scr->rooty;
			e->xmotion.root    = Scr->Root;
			break;
		case EnterNotify:
		case LeaveNotify:
			e->xcrossing.x_root -= Scr->rootx;
			e->xcrossing.y_root -= Scr->rooty;
			e->xcrossing.root    = Scr->Root;
			break;
		default:
			break;
	}
}


/* Move this next to GetTwmWindow()? */
ScreenInfo *
GetTwmScreen(XEvent *event)
{
	ScreenInfo *scr;

	if(XFindContext(dpy, event->xany.window, ScreenContext,
	                (XPointer *)&scr) == XCNOENT) {
		scr = FindScreenInfo(WindowOfEvent(event));
	}

	return scr;
}


/***********************************************************************
 *
 *  Procedure:
 *      FindScreenInfo - get ScreenInfo struct associated with a given window
 *
 *  Returned Value:
 *      ScreenInfo struct
 *
 *  Inputs:
 *      w       - the window
 *
 ***********************************************************************
 */
static ScreenInfo *
FindScreenInfo(Window w)
{
	XWindowAttributes attr;
	int scrnum;

	attr.screen = NULL;
	if(XGetWindowAttributes(dpy, w, &attr)) {
		for(scrnum = 0; scrnum < NumScreens; scrnum++) {
			if(ScreenList[scrnum] != NULL &&
			                (ScreenOfDisplay(dpy, ScreenList[scrnum]->screen) ==
			                 attr.screen)) {
				return ScreenList[scrnum];
			}
		}
	}

	return NULL;
}


void
SynthesiseFocusOut(Window w)
{
	XEvent event;

#ifdef TRACE_FOCUS
	fprintf(stderr, "Synthesizing FocusOut on %x\n", w);
#endif

	event.type = FocusOut;
	event.xfocus.window = w;
	event.xfocus.mode = NotifyNormal;
	event.xfocus.detail = NotifyPointer;

	XPutBackEvent(dpy, &event);
}


void
SynthesiseFocusIn(Window w)
{
	XEvent event;

#ifdef TRACE_FOCUS
	fprintf(stderr, "Synthesizing FocusIn on %x\n", w);
#endif

	event.type = FocusIn;
	event.xfocus.window = w;
	event.xfocus.mode = NotifyNormal;
	event.xfocus.detail = NotifyPointer;

	XPutBackEvent(dpy, &event);

}


/*
 * This is actually never called anywhere in event code, but it digs into
 * the innards of events to do somewhat scary things.
 */
void
SimulateMapRequest(Window w)
{
	Event.xmaprequest.window = w;
	HandleMapRequest();
}

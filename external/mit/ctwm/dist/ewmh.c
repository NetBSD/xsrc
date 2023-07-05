/*
 * Copyright 2014 Olaf Seibert
 */

/*
 * Implements some of the Extended Window Manager Hints, as (extremely
 * poorly) documented at
 * http://standards.freedesktop.org/wm-spec/wm-spec-1.3.html .
 * In fact, the wiki page that refers to that as being the current version
 * (http://www.freedesktop.org/wiki/Specifications/wm-spec/)
 * neglects to tell us there are newer versions 1.4 and 1.5 at
 * http://standards.freedesktop.org/wm-spec/wm-spec-1.5.html
 * (which has a listable directory that I discovered by accident).
 * The same wiki page also has lots of dead links to a CVS repository.
 * Nevertheless, it is where one ends up if one starts at
 * http://www.freedesktop.org/wiki/Specifications/ .
 *
 * EWMH is an extension to the ICCCM (Inter-Client Communication
 * Conventions Manual).
 * http://tronche.com/gui/x/icccm/
 *
 * To fill in lots of details, the source code of other window managers
 * has been consulted.
 */

#include "ctwm.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>
#include <assert.h>

#include <X11/Xatom.h>
#include <X11/extensions/shape.h>

#include "ctwm_atoms.h"
#include "ctwm_shutdown.h"
#include "ewmh_atoms.h"
#include "screen.h"
#include "events.h"
#include "event_handlers.h"
#include "functions_defs.h"
#include "icons.h"
#include "otp.h"
#include "image.h"
#include "list.h"
#include "functions.h"
#include "occupation.h"
#include "r_layout.h"
#include "util.h"
#include "vscreen.h"
#include "win_iconify.h"
#include "win_ops.h"
#include "win_resize.h"
#include "win_utils.h"
#include "workspace_utils.h"

/* #define DEBUG_EWMH */

Atom XEWMHAtom[NUM_EWMH_XATOMS];

#define NET_WM_STATE_REMOVE        0    /* remove/unset property */
#define NET_WM_STATE_ADD           1    /* add/set property */
#define NET_WM_STATE_TOGGLE        2    /* toggle property  */

#define _NET_WM_MOVERESIZE_SIZE_TOPLEFT      0
#define _NET_WM_MOVERESIZE_SIZE_TOP          1
#define _NET_WM_MOVERESIZE_SIZE_TOPRIGHT     2
#define _NET_WM_MOVERESIZE_SIZE_RIGHT        3
#define _NET_WM_MOVERESIZE_SIZE_BOTTOMRIGHT  4
#define _NET_WM_MOVERESIZE_SIZE_BOTTOM       5
#define _NET_WM_MOVERESIZE_SIZE_BOTTOMLEFT   6
#define _NET_WM_MOVERESIZE_SIZE_LEFT         7
#define _NET_WM_MOVERESIZE_MOVE              8   /* movement only */
#define _NET_WM_MOVERESIZE_SIZE_KEYBOARD     9   /* size via keyboard */
#define _NET_WM_MOVERESIZE_MOVE_KEYBOARD    10   /* move via keyboard */
#define _NET_WM_MOVERESIZE_CANCEL           11   /* cancel operation */

static Image *ExtractIcon(ScreenInfo *scr, unsigned long *prop, int width,
                          int height);
static void EwmhClientMessage_NET_WM_DESKTOP(XClientMessageEvent *msg);
static void EwmhClientMessage_NET_WM_STATE(XClientMessageEvent *msg);
static void EwmhClientMessage_NET_ACTIVE_WINDOW(XClientMessageEvent *msg);
static void EwmhClientMessage_NET_WM_MOVERESIZE(XClientMessageEvent *msg);
static void EwmhClientMessage_NET_CLOSE_WINDOW(XClientMessageEvent *msg);
static XEvent synth_btnevent_for_moveresize(TwmWindow *twm_win);
static unsigned long EwmhGetWindowProperty(Window w, Atom name, Atom type);
static void EwmhGetStrut(TwmWindow *twm_win, bool update);
static void EwmhRemoveStrut(TwmWindow *twm_win);
static void EwmhSet_NET_WORKAREA(ScreenInfo *scr);
static int EwmhGet_NET_WM_STATE(TwmWindow *twm_win);
static void EwmhClientMessage_NET_WM_STATEchange(TwmWindow *twm_win, int change,
                int newVal);

#define ALL_WORKSPACES  0xFFFFFFFFU

static void SendPropertyMessage(Window to, Window about,
                                Atom messagetype,
                                long l0, long l1, long l2, long l3, long l4,
                                long mask)
{
	XEvent e;

	e.xclient.type = ClientMessage;
	e.xclient.message_type = messagetype;
	e.xclient.display = dpy;
	e.xclient.window = about;
	e.xclient.format = 32;
	e.xclient.data.l[0] = l0;
	e.xclient.data.l[1] = l1;
	e.xclient.data.l[2] = l2;
	e.xclient.data.l[3] = l3;
	e.xclient.data.l[4] = l4;

	XSendEvent(dpy, to, False, mask, &e);
}

static void EwmhInitAtoms(void)
{
	XInternAtoms(dpy, XEWMHAtomNames, NUM_EWMH_XATOMS, False, XEWMHAtom);
}

static bool caughtError;

static int CatchError(Display *display, XErrorEvent *event)
{
	caughtError = true;
	return 0;
}

void EwmhInit(void)
{
	EwmhInitAtoms();
}

/*
 * Force-generate some event, so that we know the current time.
 *
 * Suggested in the ICCCM:
 * http://tronche.com/gui/x/icccm/sec-2.html#s-2.1
 */

static void GenerateTimestamp(ScreenInfo *scr)
{
	XEvent event;
	struct timespec tosleep;
	int timeout = 200;          /* 0.2 seconds in ms */
	int found;

	/* Sleep in 10ms chunks */
	tosleep.tv_sec  = 0;
	tosleep.tv_nsec = (10 * 1000 * 1000);

	if(EventTime > 0) {
		return;
	}

	XChangeProperty(dpy, scr->icccm_Window,
	                XA_WM_CLASS, XA_STRING,
	                8, PropModeAppend, NULL, 0);

	while(timeout > 0) {
		found = XCheckTypedWindowEvent(dpy, scr->icccm_Window, PropertyNotify, &event);
		if(found) {
			break;
		}
		nanosleep(&tosleep, NULL);
		timeout -= 10;
	}

	if(found) {
#ifdef DEBUG_EWMH
		fprintf(stderr, "GenerateTimestamp: time = %ld, timeout left = %d\n",
		        event.xproperty.time, timeout);
#endif /* DEBUG_EWMH */
		if(EventTime < event.xproperty.time) {
			EventTime = event.xproperty.time;
		}
	}
}

/*
 * Perform the "replace the window manager" protocol, as vaguely hinted
 * at by the ICCCM section 4.3.
 * http://tronche.com/gui/x/icccm/sec-4.html#s-4.3
 *
 * We do want to run through this even if we're not running with
 * --replace ourselves, because it also sets up the bits for other
 * invocations to --replace us.
 *
 * TODO: convert the selection to atom VERSION.
 */
static bool EwmhReplaceWM(ScreenInfo *scr)
{
	char atomname[32];
	Atom wmAtom;
	Window selectionOwner;

	snprintf(atomname, sizeof(atomname), "WM_S%d", scr->screen);
	wmAtom = XInternAtom(dpy, atomname, False);

	selectionOwner = XGetSelectionOwner(dpy, wmAtom);
	if(selectionOwner == scr->icccm_Window) {
		selectionOwner = None;
	}

	if(selectionOwner != None) {
		XErrorHandler oldHandler;

		/*
		 * Check if that owner still exists, and if it does, we want
		 * StructureNotify-kind events from it.
		 */
		caughtError = false;
		oldHandler = XSetErrorHandler(CatchError);

		XSelectInput(dpy, selectionOwner, StructureNotifyMask);
		XSync(dpy, False);

		XSetErrorHandler(oldHandler);

		if(caughtError) {
			selectionOwner = None;
		}
	}

	/*
	 * If something else is owning things and we're not running
	 * --replace, give up now and return failure.
	 */
	if(selectionOwner != None && !CLarg.ewmh_replace) {
#ifdef DEBUG_EWMH
		fprintf(stderr, "A window manager is already running on screen %d\n",
		        scr->screen);
#endif
		return false;
	}

	/* We're asked to --replace, so try to do so */
	XSetSelectionOwner(dpy, wmAtom, scr->icccm_Window, CurrentTime);

	if(XGetSelectionOwner(dpy, wmAtom) != scr->icccm_Window) {
		fprintf(stderr, "Did not get window manager selection on screen %d\n",
		        scr->screen);
		return false;
	}

	/*
	 * If there was a previous selection owner, wait for it
	 * to go away.
	 */

	if(selectionOwner != None) {
		int timeout = 10 * 1000;        /* 10 seconds in ms */
		XEvent event;
		struct timespec tosleep;

		/* Sleep in 100ms chunks */
		tosleep.tv_sec  = 0;
		tosleep.tv_nsec = (100 * 1000 * 1000);

		while(timeout > 0) {

			int found = XCheckTypedWindowEvent(dpy, selectionOwner, DestroyNotify, &event);
			if(found) {
				break;
			}
			nanosleep(&tosleep, NULL);
			timeout -= 100;
		}

		if(timeout <= 0) {
			fprintf(stderr, "Timed out waiting for other window manager "
			        "on screen %d to quit\n",
			        scr->screen);
			return false;
		}
	}

	/*
	 * Send a message to confirm we're now managing the screen.
	 * ICCCM, end of chapter 2, section "Manager Selections".
	 * http://tronche.com/gui/x/icccm/sec-2.html#s-2.8
	 *
	 * ICCCM says StructureNotifyMask,
	 * OpenBox says SubstructureNotifyMask.
	 */

	GenerateTimestamp(scr);

	SendPropertyMessage(scr->XineramaRoot, scr->XineramaRoot,
	                    XA_MANAGER, EventTime, wmAtom, scr->icccm_Window, 0, 0,
	                    StructureNotifyMask);

	return true;
}

/*
 * This function is called very early in initialisation.
 *
 * Only scr->screen and scr->XineramaRoot are valid: we want to know if
 * it makes sense to continue with the full initialisation.
 *
 * Create the ICCCM window that owns the WM_Sn selection.
 */
bool EwmhInitScreenEarly(ScreenInfo *scr)
{
	XSetWindowAttributes attrib;

#ifdef DEBUG_EWMH
	fprintf(stderr, "EwmhInitScreenEarly: XCreateWindow\n");
#endif
	attrib.event_mask = PropertyChangeMask;
	attrib.override_redirect = True;
	scr->icccm_Window = XCreateWindow(dpy, scr->XineramaRoot,
	                                  -100, -100, 1, 1, 0,
	                                  CopyFromParent, InputOutput,
	                                  CopyFromParent,
	                                  CWEventMask | CWOverrideRedirect,
	                                  &attrib);

	XMapWindow(dpy, scr->icccm_Window);
	XLowerWindow(dpy, scr->icccm_Window);

#ifdef DEBUG_EWMH
	fprintf(stderr, "EwmhInitScreenEarly: call EwmhReplaceWM\n");
#endif
	if(!EwmhReplaceWM(scr)) {
		XDestroyWindow(dpy, scr->icccm_Window);
		scr->icccm_Window = None;

#ifdef DEBUG_EWMH
		fprintf(stderr, "EwmhInitScreenEarly: return false\n");
#endif
		return false;
	}

#ifdef DEBUG_EWMH
	fprintf(stderr, "EwmhInitScreenEarly: return true\n");
#endif
	return true;
}

/*
 * This initialisation is called late, when scr has been set up
 * completely.
 */
void EwmhInitScreenLate(ScreenInfo *scr)
{
	long data[2];

	/* Set _NET_SUPPORTING_WM_CHECK on root window */
	data[0] = scr->icccm_Window;
	XChangeProperty(dpy, scr->XineramaRoot,
	                XA__NET_SUPPORTING_WM_CHECK, XA_WINDOW,
	                32, PropModeReplace,
	                (unsigned char *)data, 1);

	/*
	 * Set properties on the window;
	 * this also belongs with _NET_SUPPORTING_WM_CHECK
	 */
	XChangeProperty(dpy, scr->icccm_Window,
	                XA__NET_WM_NAME, XA_UTF8_STRING,
	                8, PropModeReplace,
	                (unsigned char *)"ctwm", 4);

	data[0] = scr->icccm_Window;
	XChangeProperty(dpy, scr->icccm_Window,
	                XA__NET_SUPPORTING_WM_CHECK, XA_WINDOW,
	                32, PropModeReplace,
	                (unsigned char *)data, 1);

	/*
	 * Add supported properties to the root window.
	 */
	data[0] = 0;
	data[1] = 0;
	XChangeProperty(dpy, scr->XineramaRoot,
	                XA__NET_DESKTOP_VIEWPORT, XA_CARDINAL,
	                32, PropModeReplace,
	                (unsigned char *)data, 2);

	data[0] = scr->rootw;
	data[1] = scr->rooth;
	XChangeProperty(dpy, scr->XineramaRoot,
	                XA__NET_DESKTOP_GEOMETRY, XA_CARDINAL,
	                32, PropModeReplace,
	                (unsigned char *)data, 2);

	EwmhSet_NET_WORKAREA(scr);

	if(scr->workSpaceManagerActive) {
		data[0] = scr->workSpaceMgr.count;
	}
	else {
		data[0] = 1;
	}

	XChangeProperty(dpy, scr->XineramaRoot,
	                XA__NET_NUMBER_OF_DESKTOPS, XA_CARDINAL,
	                32, PropModeReplace,
	                (unsigned char *)data, 1);

	if(scr->workSpaceManagerActive) {
		/* TODO: this is for the first Virtual Screen only... */
		/*data[0] = scr->workSpaceMgr.workSpaceWindowList->currentwspc->number; */
		data[0] = 0;
	}
	else {
		data[0] = 0;
	}
	XChangeProperty(dpy, scr->XineramaRoot,
	                XA__NET_CURRENT_DESKTOP, XA_CARDINAL,
	                32, PropModeReplace,
	                (unsigned char *)data, 1);

	EwmhSet_NET_SHOWING_DESKTOP(0);

	long supported[30];
	int i = 0;

	supported[i++] = XA__NET_SUPPORTING_WM_CHECK;
	supported[i++] = XA__NET_DESKTOP_VIEWPORT;
	supported[i++] = XA__NET_NUMBER_OF_DESKTOPS;
	supported[i++] = XA__NET_CURRENT_DESKTOP;
	supported[i++] = XA__NET_DESKTOP_GEOMETRY;
	supported[i++] = XA__NET_WM_ICON;
	supported[i++] = XA__NET_WM_DESKTOP;
	supported[i++] = XA__NET_CLIENT_LIST;
	supported[i++] = XA__NET_CLIENT_LIST_STACKING;
	supported[i++] = XA__NET_WM_WINDOW_TYPE;
	supported[i++] = XA__NET_WM_WINDOW_TYPE_NORMAL;
	supported[i++] = XA__NET_WM_WINDOW_TYPE_DESKTOP;
	supported[i++] = XA__NET_WM_WINDOW_TYPE_DOCK;
	supported[i++] = XA__NET_WM_STRUT;
	supported[i++] = XA__NET_WM_STRUT_PARTIAL;
	supported[i++] = XA__NET_SHOWING_DESKTOP;
	supported[i++] = XA__NET_WM_STATE;
	supported[i++] = XA__NET_WM_STATE_MAXIMIZED_VERT;
	supported[i++] = XA__NET_WM_STATE_MAXIMIZED_HORZ;
	supported[i++] = XA__NET_WM_STATE_FULLSCREEN;
	supported[i++] = XA__NET_ACTIVE_WINDOW;
	supported[i++] = XA__NET_WORKAREA;
	supported[i++] = XA__NET_WM_MOVERESIZE;
	supported[i++] = XA__NET_WM_STATE_SHADED;
	supported[i++] = XA__NET_WM_STATE_ABOVE;
	supported[i++] = XA__NET_WM_STATE_BELOW;
	supported[i++] = XA__NET_CLOSE_WINDOW;

	XChangeProperty(dpy, scr->XineramaRoot,
	                XA__NET_SUPPORTED, XA_ATOM,
	                32, PropModeReplace,
	                (unsigned char *)supported, i);
}


#ifdef VSCREEN
/*
 * Set up the _NET_VIRTUAL_ROOTS property, which indicates that we're
 * using virtual root windows.
 * This applies only if we have multiple virtual screens.
 *
 * Also record this as a supported atom in _NET_SUPPORTED.
 *
 * Really, our virtual screens (with their virtual root windows) don't quite
 * fit in the EWMH idiom. Several root window properties (such as
 * _NET_CURRENT_DESKTOP) are more appropriate on the virtual root windows. But
 * that is not where other clients would look for them.
 *
 * The idea seems to be that the virtual roots as used for workspaces (desktops
 * in EWMH terminology) are only mapped one at a time.
 */
void EwmhInitVirtualRoots(ScreenInfo *scr)
{
	int numVscreens = scr->numVscreens;

	if(numVscreens > 1) {
		long *data;
		long d0;
		VirtualScreen *vs;
		int i;

		data = calloc(numVscreens, sizeof(long));

		for(vs = scr->vScreenList, i = 0;
		                vs != NULL && i < numVscreens;
		                vs = vs->next, i++) {
			data[i] = vs->window;
		}

		XChangeProperty(dpy, scr->XineramaRoot,
		                XA__NET_VIRTUAL_ROOTS, XA_WINDOW,
		                32, PropModeReplace,
		                (unsigned char *)data, numVscreens);

		/* This might fail, but what can we do about it? */

		free(data);

		d0 = XA__NET_VIRTUAL_ROOTS;
		XChangeProperty(dpy, scr->XineramaRoot,
		                XA__NET_SUPPORTED, XA_ATOM,
		                32, PropModeAppend,
		                (unsigned char *)&d0, 1);
	}
}
#endif


static void EwmhTerminateScreen(ScreenInfo *scr)
{
	XDeleteProperty(dpy, scr->XineramaRoot, XA__NET_SUPPORTED);

	/*
	 * Don't delete scr->icccm_Window; let it be deleted automatically
	 * when we terminate the X server connection. A replacement window
	 * manager may want to start working immediately after it has
	 * disappeared.
	 */
}

/*
 * Clear everything that needs to be cleared before we exit.
 */

void EwmhTerminate(void)
{
	int scrnum;
	ScreenInfo *scr;

	for(scrnum = 0; scrnum < NumScreens; scrnum++) {
		if((scr = ScreenList[scrnum]) == NULL) {
			continue;
		}
		EwmhTerminateScreen(scr);
	}
}

/*
 * Event handler: lost the WM_Sn selection
 * (that's the only selection we have).
 */

void EwmhSelectionClear(XSelectionClearEvent *sev)
{
#ifdef DEBUG_EWMH
	fprintf(stderr, "sev->window = %x\n", (unsigned)sev->window);
#endif
	DoShutdown();
}

/*
 * When accepting client messages to the root window,
 * be accepting and accept both the real root window and the
 * current virtual screen.
 *
 * Should perhaps also accept any other virtual screen.
 */
bool EwmhClientMessage(XClientMessageEvent *msg)
{
	if(msg->format != 32) {
		return false;
	}

	/* Messages regarding any window */
	if(msg->message_type == XA__NET_WM_DESKTOP) {
		EwmhClientMessage_NET_WM_DESKTOP(msg);
		return true;
	}
	else if(msg->message_type == XA__NET_WM_STATE) {
		EwmhClientMessage_NET_WM_STATE(msg);
		return true;
	}
	else if(msg->message_type == XA__NET_ACTIVE_WINDOW) {
		EwmhClientMessage_NET_ACTIVE_WINDOW(msg);
		return true;
	}
	else if(msg->message_type == XA__NET_WM_MOVERESIZE) {
		EwmhClientMessage_NET_WM_MOVERESIZE(msg);
		return true;
	}
	else if(msg->message_type == XA__NET_CLOSE_WINDOW) {
		EwmhClientMessage_NET_CLOSE_WINDOW(msg);
		return true;
	}

	/* Messages regarding the root window */
	if(msg->window != Scr->XineramaRoot &&
	                msg->window != Scr->Root) {
#ifdef DEBUG_EWMH
		fprintf(stderr, "Received unrecognized client message: %s\n",
		        XGetAtomName(dpy, msg->message_type));
#endif
		return false;
	}

	if(msg->message_type == XA__NET_CURRENT_DESKTOP) {
		GotoWorkSpaceByNumber(Scr->currentvs, msg->data.l[0]);
		return true;
	}
	else if(msg->message_type == XA__NET_SHOWING_DESKTOP) {
		ShowBackground(Scr->currentvs, msg->data.l[0] ? 1 : 0);
		return true;
	}
	else {
#ifdef DEBUG_EWMH
		fprintf(stderr, "Received unrecognized client message about root window: %s\n",
		        XGetAtomName(dpy, msg->message_type));
#endif
	}

	return false;
}

/*
 * The format of the _NET_WM_ICON property is
 *
 * [0] width
 * [1] height
 *     height repetitions of
 *         row, which is
 *              width repetitions of
 *                      pixel: ARGB
 * repeat for next size.
 *
 * Some icons can be 256x256 CARDINALs which is 65536 CARDINALS!
 * Therefore we fetch in pieces and skip the pixels of large icons
 * until needed.
 *
 * First scan all sizes. Keep a record of the closest smaller and larger
 * size. At the end, choose from one of those.
 * Finally, go and fetch the pixel data.
 */

Image *EwmhGetIcon(ScreenInfo *scr, TwmWindow *twm_win)
{
	int fetch_offset;
	Atom actual_type;
	int actual_format;
	unsigned long nitems, bytes_after;
	unsigned long *prop;

	int wanted_area;
	int smaller, larger;
	int offset;
	int smaller_offset, larger_offset;
	int i;

	int width, height;

	fetch_offset = 0;
	if(XGetWindowProperty(dpy, twm_win->w, XA__NET_WM_ICON,
	                      fetch_offset, 8 * 1024, False, XA_CARDINAL,
	                      &actual_type, &actual_format, &nitems,
	                      &bytes_after, (unsigned char **)&prop) != Success || nitems == 0) {
		return NULL;
	}

	if(actual_format != 32) {
		XFree(prop);
		return NULL;
	}

#ifdef DEBUG_EWMH
	fprintf(stderr, "_NET_WM_ICON data fetched\n");
#endif
	/*
	 * Usually the icons are square, but that is not a rule.
	 * So we measure the area instead.
	 *
	 * Approach wanted size from both directions and at the end,
	 * choose the "nearest".
	 */
	wanted_area = Scr->PreferredIconWidth * Scr->PreferredIconHeight;
	smaller = 0;
	larger = 999999;
	smaller_offset = -1;
	larger_offset = -1;
	i = 0;

	for(;;) {
		offset = i;

		int w = prop[i++];
		int h = prop[i++];
		int size = w * h;

		const int area = w * h;

#ifdef DEBUG_EWMH
		fprintf(stderr, "[%d+%d] w=%d h=%d\n", fetch_offset, offset, w, h);
#endif


		if(area == wanted_area) {
#ifdef DEBUG_EWMH
			fprintf(stderr, "exact match [%d+%d=%d] w=%d h=%d\n", fetch_offset, offset,
			        fetch_offset + offset, w, h);
#endif /* DEBUG_EWMH */
			smaller_offset = fetch_offset + offset;
			smaller = area;
			larger_offset = -1;
			break;
		}
		else if(area < wanted_area) {
			if(area > smaller) {
#ifdef DEBUG_EWMH
				fprintf(stderr, "increase smaller, was [%d]\n", smaller_offset);
#endif /* DEBUG_EWMH */
				smaller = area;
				smaller_offset = fetch_offset + offset;
			}
		}
		else {   /* area > wanted_area */
			if(area < larger) {
#ifdef DEBUG_EWMH
				fprintf(stderr, "decrease larger, was [%d]\n", larger_offset);
#endif /* DEBUG_EWMH */
				larger = area;
				larger_offset = fetch_offset + offset;
			}
		}

		if(i + size + 2 > nitems) {
#ifdef DEBUG_EWMH
			fprintf(stderr, "not enough data: %d + %d > %ld \n", i, size, nitems);
#endif /* DEBUG_EWMH */

			if(i + size + 2 <= nitems + bytes_after / 4) {
				/* we can fetch some more... */
				XFree(prop);
				fetch_offset += i + size;
				if(XGetWindowProperty(dpy, twm_win->w, XA__NET_WM_ICON,
				                      fetch_offset, 8 * 1024, False, XA_CARDINAL,
				                      &actual_type, &actual_format, &nitems,
				                      &bytes_after, (unsigned char **)&prop) != Success) {
					continue;
				}
				i = 0;
				continue;
			}
			break;
		}
		i += size;
	}

	/*
	 * Choose which icon approximates our desired size best.
	 */
	int area = 0;
	ALLOW_DEAD_STORE(area); // all branches below init it

	if(smaller_offset >= 0) {
		if(larger_offset >= 0) {
			/* choose the nearest */
#ifdef DEBUG_EWMH
			fprintf(stderr, "choose nearest %d %d\n", smaller, larger);
#endif /* DEBUG_EWMH */
			if((double)larger / wanted_area > (double)wanted_area / smaller) {
				offset = smaller_offset;
				area = smaller;
			}
			else {
				offset = larger_offset;
				area = larger;
			}
		}
		else {
			/* choose smaller */
#ifdef DEBUG_EWMH
			fprintf(stderr, "choose smaller (only) %d\n", smaller);
#endif /* DEBUG_EWMH */
			offset = smaller_offset;
			area = smaller;
		}
	}
	else if(larger_offset >= 0) {
		/* choose larger */
#ifdef DEBUG_EWMH
		fprintf(stderr, "choose larger (only) %d\n", larger);
#endif /* DEBUG_EWMH */
		offset = larger_offset;
		area = larger;
	}
	else {
		/* no icons found at all? */
#ifdef DEBUG_EWMH
		fprintf(stderr, "nothing to choose from\n");
#endif /* DEBUG_EWMH */
		XFree(prop);
		return NULL;
	}

	/*
	 * Now fetch the pixels.
	 */

#ifdef DEBUG_EWMH
	fprintf(stderr, "offset = %d fetch_offset = %d\n", offset, fetch_offset);
	fprintf(stderr, "offset + 2 + area = %d fetch_offset + nitems = %ld\n",
	        offset + 2 + area, fetch_offset + nitems);
#endif /* DEBUG_EWMH */
	if(offset < fetch_offset ||
	                offset + 2 + area > fetch_offset + nitems) {
		XFree(prop);
		fetch_offset = offset;
#ifdef DEBUG_EWMH
		fprintf(stderr, "refetching from %d\n", fetch_offset);
#endif /* DEBUG_EWMH */
		if(XGetWindowProperty(dpy, twm_win->w, XA__NET_WM_ICON,
		                      fetch_offset, 2 + area, False, XA_CARDINAL,
		                      &actual_type, &actual_format, &nitems,
		                      &bytes_after, (unsigned char **)&prop) != Success) {
			return NULL;
		}
	}

	i = offset - fetch_offset;
	width = prop[i++];
	height = prop[i++];
#ifdef DEBUG_EWMH
	fprintf(stderr, "Chosen [%d] w=%d h=%d area=%d\n", offset, width, height, area);
#endif /* DEBUG_EWMH */
	assert(width * height == area);

	Image *image = ExtractIcon(scr, &prop[i], width, height);

	XFree(prop);

	return image;
}

static uint16_t *buffer_16bpp;
static uint32_t *buffer_32bpp;

static void convert_for_16(int w, int x, int y, int argb)
{
	int r = (argb >> 16) & 0xFF;
	int g = (argb >>  8) & 0xFF;
	int b = (argb >>  0) & 0xFF;
	buffer_16bpp [y * w + x] = ((r >> 3) << 11) + ((g >> 2) << 5) + (b >> 3);
}

static void convert_for_32(int w, int x, int y, int argb)
{
	buffer_32bpp [y * w + x] = argb & 0x00FFFFFF;
}

static Image *ExtractIcon(ScreenInfo *scr, unsigned long *prop, int width,
                          int height)
{
	XImage *ximage;
	void (*store_data)(int w, int x, int y, int argb);
	int x, y, transparency;
	int rowbytes;
	unsigned char *maskbits;

	GC gc;
	Pixmap pixret;
	Pixmap mask;
	Image *image;
	int i;

	ximage = NULL;

	/** XXX sort of duplicated from util.c:LoadJpegImage() */
	if(scr->d_depth == 16) {
		store_data = convert_for_16;
		buffer_16bpp = malloc(width * height * sizeof(buffer_16bpp[0]));
		buffer_32bpp = NULL;
		ximage = XCreateImage(dpy, CopyFromParent, scr->d_depth, ZPixmap, 0,
		                      (char *) buffer_16bpp, width, height, 16, width * 2);
	}
	else if(scr->d_depth == 24 || scr->d_depth == 32) {
		store_data = convert_for_32;
		buffer_32bpp = malloc(width * height * sizeof(buffer_32bpp[0]));
		buffer_16bpp = NULL;
		ximage = XCreateImage(dpy, CopyFromParent, scr->d_depth, ZPixmap, 0,
		                      (char *) buffer_32bpp, width, height, 32, width * 4);
	}
	else {
#ifdef DEBUG_EWMH
		fprintf(stderr, "Screen unsupported depth for 32-bit icon: %d\n", scr->d_depth);
#endif /* DEBUG_EWMH */
		XFree(prop);
		return NULL;
	}
	if(ximage == NULL) {
#ifdef DEBUG_EWMH
		fprintf(stderr, "cannot create image for icon\n");
#endif /* DEBUG_EWMH */
		XFree(prop);
		return NULL;
	}

	transparency = 0;
	rowbytes = (width + 7) / 8;
	maskbits = calloc(height, rowbytes);

	/*
	 * Copy all ARGB pixels to the pixmap (the RGB part), and the bitmap (the
	 * Alpha, or opaqueness part). If any pixels are transparent, we're going
	 * to need a shape.
	 */
	i = 0;
	for(y = 0; y < height; y++) {
		for(x = 0; x < width; x++) {
			unsigned long argb = prop[i++];
			store_data(width, x, y, argb);
			int opaque = ((argb >> 24) & 0xFF) >= 0x80; /* arbitrary cutoff */
			if(opaque) {
				maskbits [rowbytes * y + (x / 8)] |= 0x01 << (x % 8);
			}
			else {
				transparency = 1;
			}
		}
	}

	gc = DefaultGC(dpy, scr->screen);
	pixret = XCreatePixmap(dpy, scr->Root, width, height, scr->d_depth);
	XPutImage(dpy, pixret, gc, ximage, 0, 0, 0, 0, width, height);
	XDestroyImage(ximage);  /* also frees buffer_{16,32}bpp */
	ximage = NULL;

	mask = None;
	if(transparency) {
		mask = XCreatePixmapFromBitmapData(dpy, scr->Root, (char *)maskbits,
		                                   width, height, 1, 0, 1);
	}
	free(maskbits);

	image = AllocImage();
	image->width  = width;
	image->height = height;
	image->pixmap = pixret;
	image->mask   = mask;

	return image;
}

/*
 * Handle a PropertyNotify on _NET_WM_ICON.
 */
static void EwmhHandle_NET_WM_ICONNotify(XPropertyEvent *event,
                TwmWindow *twm_win)
{
	unsigned long valuemask;                /* mask for create windows */
	XSetWindowAttributes attributes;        /* attributes for create windows */
	Icon *icon = twm_win->icon;
	int x;

#ifdef DEBUG_EWMH
	fprintf(stderr, "EwmhHandlePropertyNotify: NET_WM_ICON\n");
#endif /* DEBUG_EWMH */
	/*
	 * If there is no icon yet, we'll look at this property
	 * later, if and when we do create an icon.
	 */
	if(!icon || icon->match != match_net_wm_icon) {
#ifdef DEBUG_EWMH
		fprintf(stderr, "no icon, or not match_net_wm_icon\n");
#endif /* DEBUG_EWMH */
		return;
	}

	Image *image = EwmhGetIcon(Scr, twm_win);

	/* TODO: de-duplicate with handling of XA_WM_HINTS */
	{
		Image *old_image = icon->image;
		icon->image = image;
		FreeImage(old_image);
	}


	if(twm_win->icon->bm_w) {
		XDestroyWindow(dpy, twm_win->icon->bm_w);
	}

	valuemask = CWBackPixmap;
	attributes.background_pixmap = image->pixmap;

	x = GetIconOffset(twm_win->icon);
	twm_win->icon->bm_w =
	        XCreateWindow(dpy, twm_win->icon->w, x, 0,
	                      twm_win->icon->width,
	                      twm_win->icon->height,
	                      0, Scr->d_depth,
	                      CopyFromParent, Scr->d_visual,
	                      valuemask, &attributes);

	if(image->mask) {
		XShapeCombineMask(dpy, twm_win->icon->bm_w, ShapeBounding, 0, 0, image->mask,
		                  ShapeSet);
		XShapeCombineMask(dpy, twm_win->icon->w,    ShapeBounding, x, 0, image->mask,
		                  ShapeSet);
	}
	else {
		XRectangle rect;

		rect.x      = x;
		rect.y      = 0;
		rect.width  = twm_win->icon->width;
		rect.height = twm_win->icon->height;
		XShapeCombineRectangles(dpy, twm_win->icon->w, ShapeBounding, 0,
		                        0, &rect, 1, ShapeUnion, 0);
	}
	XMapSubwindows(dpy, twm_win->icon->w);
	RedoIconName(twm_win);
}

/*
 * Handle a PropertyNotify on _NET_WM_STRUT(PARTIAL).
 */
static void EwmhHandle_NET_WM_STRUTNotify(XPropertyEvent *event,
                TwmWindow *twm_win)
{
	EwmhGetStrut(twm_win, true);
}

/*
 * Handle a _NET_WM_STATE ClientMessage.
 */
static int atomToFlag(Atom a)
{
#ifdef DEBUG_EWMH
# define CRWARN(x) fprintf(stderr, "atomToFlag: ignoring " #x "\n")
#else
# define CRWARN(x) (void)0
#endif
#define CHKNRET(st) \
        if(a == XA__NET_WM_##st) { \
                if(LookInNameList(Scr->EWMHIgnore, #st)) { \
                        CRWARN(st); \
                        return 0; \
                } \
                return EWMH_##st; \
        }

	/* Check (potentially ignoring) various flags we know */
	CHKNRET(STATE_MAXIMIZED_VERT);
	CHKNRET(STATE_MAXIMIZED_HORZ);
	CHKNRET(STATE_FULLSCREEN);
	CHKNRET(STATE_SHADED);
	CHKNRET(STATE_ABOVE);
	CHKNRET(STATE_BELOW);

#undef CHKNRET
#undef CRWARN

	/* Else we don't recognize it */
	return 0;
}


/*
 * Handle the NET_WM_STATE client message.
 * It can change 1 or 2 values represented in NET_WM_STATE.
 * The second change is allowed
 * specifically for (re)setting horizontal and vertical maximalisation in
 * one go. Treat that as a special case.
 */
static void EwmhClientMessage_NET_WM_STATE(XClientMessageEvent *msg)
{
	Window w = msg->window;
	TwmWindow *twm_win;
	int change, change1, change2, newValue;

	twm_win = GetTwmWindow(w);

	if(twm_win == NULL) {
		return;
	}

	/*
	 * Due to EWMHIgnore, it's possible to wind up with change1=0 and
	 * change2=something, so swap 'em if that happens.
	 */
	change1 = atomToFlag(msg->data.l[1]);
	change2 = atomToFlag(msg->data.l[2]);
	if(change1 == 0 && change2 != 0) {
		change1 = change2;
		change2 = 0;
	}
	change = change1 | change2;

	switch(msg->data.l[0]) {
		case NET_WM_STATE_REMOVE:
#ifdef DEBUG_EWMH
			printf("NET_WM_STATE_REMOVE: ");
#endif
			newValue = 0;
			break;
		case NET_WM_STATE_ADD:
#ifdef DEBUG_EWMH
			printf("NET_WM_STATE_ADD: ");
#endif
			newValue = change;
			break;
		case NET_WM_STATE_TOGGLE:
#ifdef DEBUG_EWMH
			printf("NET_WM_STATE_TOGGLE: ");
#endif
			newValue = ~twm_win->ewmhFlags & change;
			break;
		default:
#ifdef DEBUG_EWMH
			printf("invalid operation in NET_WM_STATE: %ld\n", msg->data.l[0]);
#endif
			return;
	}
#ifdef DEBUG_EWMH
	printf("%s and %s\n", XGetAtomName(dpy, msg->data.l[1]),
	       XGetAtomName(dpy, msg->data.l[2]));
#endif

	/*
	 * Special-case the horizontal and vertical zoom.
	 * You can turn them both on or both off, but no other combinations
	 * are done as a unit.
	 */
	if(change == (EWMH_STATE_MAXIMIZED_VERT | EWMH_STATE_MAXIMIZED_HORZ) &&
	                (newValue == 0 || newValue == change)) {
		EwmhClientMessage_NET_WM_STATEchange(twm_win, change, newValue);
	}
	else {
		EwmhClientMessage_NET_WM_STATEchange(twm_win, change1, newValue & change1);
		if(change2 != 0) {
			EwmhClientMessage_NET_WM_STATEchange(twm_win, change2, newValue & change2);
		}
	}
}

/*
 * change - bitmask of settings that possibly change. Only one bit is
 *                      set in this, with the possible exception of
 *                      *MAXIMIZED_{VERT,HORIZ} which can be set together.
 * newValue - bits with the new values; only valid bits are the ones
 *                      in change.
 */
static void EwmhClientMessage_NET_WM_STATEchange(TwmWindow *twm_win, int change,
                int newValue)
{
	/* Now check what we need to change */

	if(change & (EWMH_STATE_MAXIMIZED_VERT | EWMH_STATE_MAXIMIZED_HORZ |
	                EWMH_STATE_FULLSCREEN)) {
		int newZoom = ZOOM_NONE;

		switch(newValue) {
			case 0:
				newZoom = twm_win->zoomed;      /* turn off whatever zoom */
				break;
			case EWMH_STATE_MAXIMIZED_VERT:
				newZoom = F_ZOOM;
				break;
			case EWMH_STATE_MAXIMIZED_HORZ:
				newZoom = F_HORIZOOM;
				break;
			case EWMH_STATE_MAXIMIZED_HORZ | EWMH_STATE_MAXIMIZED_VERT:
				newZoom = F_FULLZOOM;
				break;
			case EWMH_STATE_FULLSCREEN:
				newZoom = F_FULLSCREENZOOM;
				break;
		}
		fullzoom(twm_win, newZoom);
	}
	else if(change & EWMH_STATE_SHADED) {
#ifdef DEBUG_EWMH
		printf("EWMH_STATE_SHADED: newValue: %d old: %d\n", newValue,
		       twm_win->ewmhFlags & EWMH_STATE_SHADED);
#endif
		if((twm_win->ewmhFlags & EWMH_STATE_SHADED) ^ newValue) {
			/* Toggle the shade/squeeze state */
#ifdef DEBUG_EWMH
			printf("EWMH_STATE_SHADED: change it\n");
#endif
			Squeeze(twm_win);
		}
	}
	else if(change & (EWMH_STATE_ABOVE | EWMH_STATE_BELOW)) {
		/*
		 * Other changes call into ctwm code, which in turn calls back to
		 * EWMH code to update the ewmhFlags and the property.  This one
		 * we handle completely internally.
		 */
		unsigned omask = 0, oval = 0;
		const int prepri = OtpEffectivePriority(twm_win);

		/* Which bits are we changing and what to? */
#define DOBIT(fld) do { \
          if(change   & EWMH_STATE_##fld) { omask |= OTP_AFLAG_##fld; } \
          if(newValue & EWMH_STATE_##fld) { oval  |= OTP_AFLAG_##fld; } \
        } while(0)

		DOBIT(ABOVE);
		DOBIT(BELOW);

#undef DOBIT

		/* Update OTP as necessary */
		OtpSetAflagMask(twm_win, omask, oval);
		if(OtpEffectivePriority(twm_win) != prepri) {
			OtpRestackWindow(twm_win);
		}

		/* Set the EWMH property back on the window */
		EwmhSet_NET_WM_STATE(twm_win, change);
	}
}

/*
 * Handle the _NET_ACTIVE_WINDOW client message.
 * Pagers would send such a message to "activate" a window.
 *
 * What does "activate" really mean? It isn't properly described.
 *
 * Let's presume that it means that the window is de-iconified and gets
 * focus.  The mouse may be moved to it (but not all window button apps
 * do that).  But is it always raised or should that depend on the
 * RaiseOnWarp option?
 */
static void EwmhClientMessage_NET_ACTIVE_WINDOW(XClientMessageEvent *msg)
{
	Window w = msg->window;
	TwmWindow *twm_win;

	twm_win = GetTwmWindow(w);

	if(twm_win == NULL) {
		return;
	}

	if(!twm_win->mapped) {
		DeIconify(twm_win);
	}
#if 0
	WarpToWindow(twm_win, Scr->RaiseOnWarp /* True ? */);
#else
	/*
	 * Keep the mouse pointer where it is (typically the dock).
	 * WarpToWindow() would change the current workspace if needed to go
	 * to the window. But pagers would only send this message for
	 * windows in the current workspace, I expect.
	 */
	if(Scr->RaiseOnWarp) {
		AutoRaiseWindow(twm_win);
	}
	SetFocus(twm_win, msg->data.l[1]);
#endif
}

/*
 * Ugly implementation of _NET_WM_MOVERESIZE.
 *
 * window = window to be moved or resized
 * message_type = _NET_WM_MOVERESIZE
 * format = 32
 * data.l[0] = x_root
 * data.l[1] = y_root
 * data.l[2] = direction
 * data.l[3] = button
 * data.l[4] = source indication
 */
static void EwmhClientMessage_NET_WM_MOVERESIZE(XClientMessageEvent *msg)
{
	TwmWindow *twm_win;

	twm_win = GetTwmWindow(msg->window);

	if(twm_win == NULL) {
		return;
	}

	if(!twm_win->mapped) {
		DeIconify(twm_win);
	}

	switch(msg->data.l[2]) {
		case _NET_WM_MOVERESIZE_SIZE_TOPLEFT:
		case _NET_WM_MOVERESIZE_SIZE_TOP:
		case _NET_WM_MOVERESIZE_SIZE_TOPRIGHT:
		case _NET_WM_MOVERESIZE_SIZE_RIGHT:
		case _NET_WM_MOVERESIZE_SIZE_BOTTOMRIGHT:
		case _NET_WM_MOVERESIZE_SIZE_BOTTOM:
		case _NET_WM_MOVERESIZE_SIZE_BOTTOMLEFT:
		case _NET_WM_MOVERESIZE_SIZE_LEFT:
		case _NET_WM_MOVERESIZE_SIZE_KEYBOARD: {
			/* Fake up as if we triggered f.resize on it */
			XEvent xevent = synth_btnevent_for_moveresize(twm_win);

			/*
			 * The resize won't actually start until we cross the cursor
			 * over a border.  Perhaps we should find the nearest corner,
			 * and pre-warp the cursor there?  That may be less friendly
			 * for the user, since it might not be as predictable, and
			 * having the cursor zoom off without warning is probably a
			 * little surprising...
			 */
			ExecuteFunction(F_RESIZE, "", twm_win->frame, twm_win,
			                &xevent, C_WINDOW, false);

			/*
			 * It's not guaranteed that this actually began as a button
			 * press, but our implementation is expecting the button to
			 * be held down all through the move, so that
			 * ExecuteFunction() won't actually end until there's a
			 * button release of some kind, which will trigger
			 * HandleButtonRelease() properly.  So we don't need to call
			 * it ourselves.
			 */

			break;
		}
		case _NET_WM_MOVERESIZE_MOVE:
		case _NET_WM_MOVERESIZE_MOVE_KEYBOARD: {
			/* Fake up as if we triggered f.move on it */
			XEvent xevent = synth_btnevent_for_moveresize(twm_win);
			ExecuteFunction(F_MOVE, "", twm_win->frame, twm_win,
			                &xevent, C_WINDOW, false);

			/* X-ref HandleButtonRelease() discussion above */
			break;
		}
		case _NET_WM_MOVERESIZE_CANCEL:
			/*
			 * TODO: check if the twm_win is the same.
			 * TODO: check how to make this actually work.
			 *
			 * As currently implemented, I don't believe we ever need to
			 * do anything here.  All the needed cleanup should happen in
			 * our ButtonRelease handler.
			 */
			break;
	}
}

static XEvent
synth_btnevent_for_moveresize(TwmWindow *twm_win)
{
	XEvent xevent;
	Window root = twm_win->parent_vs->window;
	Window child = twm_win->w;
	int x_root = twm_win->frame_x;
	int y_root = twm_win->frame_y;
	int x_win  = 0;
	int y_win  = 0;
	unsigned int dummy_mask;

	/* Find the pointer */
	XQueryPointer(dpy, twm_win->frame, &root, &child, &x_root, &y_root,
	              &x_win, &y_win, &dummy_mask);

	/* Synthesize a button event */
	xevent.type = ButtonPress;
	xevent.xbutton.root = root;
	xevent.xbutton.window = child;
	xevent.xbutton.x_root = x_root;
	xevent.xbutton.y_root = y_root;
	xevent.xbutton.x = x_win;
	xevent.xbutton.y = y_win;
	xevent.xbutton.time = EventTime;

	return xevent;
}


/*
 * Implementation of _NET_CLOSE_WINDOW
 *
 * window = window to be closed
 * message_type = _NET_CLOSE_WINDOW
 * format = 32
 * data.l[0] = timestamp
 * data.l[1] = source indication:
 *             0 Clients that support only older version of this spec
 *             1 for normal applications, and
 *             2 for pagers and other Clients that represent direct user actions
 * data.l[2] = 0
 * data.l[3] = 0
 * data.l[4] = 0
 */
static void EwmhClientMessage_NET_CLOSE_WINDOW(XClientMessageEvent *msg)
{
	TwmWindow *tmp_win;

	tmp_win = GetTwmWindow(msg->window);

	if(tmp_win != NULL) {
		ButtonPressed = -1;
		ExecuteFunction(F_DELETE, NULL, msg->window, tmp_win,
		                (XEvent *)msg, C_NO_CONTEXT, 0);
	}
}

/*
 * Handle any PropertyNotify.
 */
int EwmhHandlePropertyNotify(XPropertyEvent *event, TwmWindow *twm_win)
{
	if(event->atom == XA__NET_WM_ICON) {
		EwmhHandle_NET_WM_ICONNotify(event, twm_win);
		return 1;
	}
	else if(event->atom == XA__NET_WM_STRUT_PARTIAL ||
	                event->atom == XA__NET_WM_STRUT) {
		EwmhHandle_NET_WM_STRUTNotify(event, twm_win);
		return 1;
	}
	else if(event->atom == XA__NET_WM_NAME) {
		char *prop = GetWMPropertyString(twm_win->w, XA__NET_WM_NAME);
		if(prop == NULL) {
			FreeWMPropertyString(twm_win->names.net_wm_name);
			twm_win->names.net_wm_name = NULL;
			apply_window_name(twm_win);
			return 1;
		}

		if(twm_win->names.net_wm_name != NULL
		                && strcmp(twm_win->names.net_wm_name, prop) == 0) {
			/* No change, just free and skip out */
			free(prop);
			return 1;
		}

		/* It's changing, free the old and bring in the new */
		FreeWMPropertyString(twm_win->names.net_wm_name);
		twm_win->names.net_wm_name = prop;

		/* Kick the reset process */
		apply_window_name(twm_win);
		return 1;
	}
	else if(event->atom == XA__NET_WM_ICON_NAME) {
		char *prop = GetWMPropertyString(twm_win->w, XA__NET_WM_ICON_NAME);
		if(prop == NULL) {
			FreeWMPropertyString(twm_win->names.net_wm_icon_name);
			twm_win->names.net_wm_icon_name = NULL;
			apply_window_icon_name(twm_win);
			return 1;
		}

		if(twm_win->names.net_wm_icon_name != NULL
		                && strcmp(twm_win->names.net_wm_icon_name, prop) == 0) {
			/* No change, just free and skip out */
			free(prop);
			return 1;
		}

		/* It's changing, free the old and bring in the new */
		FreeWMPropertyString(twm_win->names.net_wm_icon_name);
		twm_win->names.net_wm_icon_name = prop;

		/* Kick the reset process */
		apply_window_icon_name(twm_win);
		return 1;
	}

	return 0;
}

/*
 * Set the _NET_WM_DESKTOP property for the current workspace.
 */
void EwmhSet_NET_WM_DESKTOP(TwmWindow *twm_win)
{
	WorkSpace *ws;

	VirtualScreen *vs = twm_win->vs;
	if(vs != NULL) {
		ws = vs->wsw->currentwspc;
	}
	else {
		ws = NULL;
	}

	EwmhSet_NET_WM_DESKTOP_ws(twm_win, ws);
}

/*
 * Set the _NET_WM_DESKTOP property for the given workspace.
 */
void EwmhSet_NET_WM_DESKTOP_ws(TwmWindow *twm_win, WorkSpace *ws)
{
	unsigned long workspaces[MAXWORKSPACE];
	int n = 0;

	if(!Scr->workSpaceManagerActive) {
		workspaces[n++] = 0;
	}
	else if(twm_win->occupation == fullOccupation) {
		workspaces[n++] = ALL_WORKSPACES;
	}
	else {
		/*
		 * Our windows can occupy multiple workspaces ("virtual desktops" in
		 * EWMH terminology) at once. Extend the _NET_WM_DESKTOP property
		 * by setting it to multiple CARDINALs if this occurs.
		 * Put the currently visible workspace (if any) first, since typical
		 * pager apps don't know about this.
		 */
		int occupation = twm_win->occupation;

		/*
		 * Set visible workspace number.
		 */
		if(ws != NULL) {
			int wsn = ws->number;

			workspaces[n++] = wsn;
			occupation &= ~(1 << wsn);
		}

		/*
		 * Set any other workspace numbers.
		 */
		if(occupation != 0) {
			int i;
			int mask = 1;

			for(i = 0; i < MAXWORKSPACE; i++) {
				if(occupation & mask) {
					workspaces[n++] = i;
				}
				mask <<= 1;
			}
		}
	}

	XChangeProperty(dpy, twm_win->w,
	                XA__NET_WM_DESKTOP, XA_CARDINAL,
	                32, PropModeReplace,
	                (unsigned char *)workspaces, n);
}


static unsigned long EwmhGetWindowProperty(Window w, Atom name, Atom type)
{
	Atom actual_type;
	int actual_format;
	unsigned long nitems, bytes_after;
	unsigned long *prop;
	unsigned long value;

	if(XGetWindowProperty(dpy, w, name,
	                      0, 1, False, type,
	                      &actual_type, &actual_format, &nitems,
	                      &bytes_after, (unsigned char **)&prop) != Success) {
		return 0;
	}

	if(actual_format == 32) {
		if(nitems >= 1) {
			value = prop[0];
		}
		else {
			value = 0;
		}
	}
	else {
		value = 0;
	}

	XFree(prop);
	return value;
}

/*
 * Simple function to get multiple properties of format 32.
 * If it fails, returns NULL, and *nitems_return == 0.
 */

static unsigned long *EwmhGetWindowProperties(Window w, Atom name, Atom type,
                unsigned long *nitems_return)
{
	Atom actual_type;
	int actual_format;
	unsigned long bytes_after;
	unsigned long *prop;

	if(XGetWindowProperty(dpy, w, name,
	                      0, 8192, False, type,
	                      &actual_type, &actual_format, nitems_return,
	                      &bytes_after, (unsigned char **)&prop) != Success) {
		*nitems_return = 0;
		return NULL;
	}

	if(actual_format != 32) {
		XFree(prop);
		prop = NULL;
		*nitems_return = 0;
	}

	return prop;
}

int EwmhGetOccupation(TwmWindow *twm_win)
{
	unsigned long nitems;
	unsigned long *prop;
	int occupation;

	occupation = 0;

	prop = EwmhGetWindowProperties(twm_win->w,
	                               XA__NET_WM_DESKTOP, XA_CARDINAL, &nitems);

	if(prop) {
		int i;
		for(i = 0; i < nitems; i++) {
			unsigned int val = prop[i];
			if(val == ALL_WORKSPACES) {
				occupation = fullOccupation;
			}
			else if(val < Scr->workSpaceMgr.count) {
				occupation |= 1 << val;
			}
			else {
				occupation |= 1 << (Scr->workSpaceMgr.count - 1);
			}
		}

		occupation &= fullOccupation;

		XFree(prop);
	}

	return occupation;
}

/*
 * The message to change the desktop of a window doesn't recognize
 * that it may be in more than one.
 *
 * Therefore the following heuristic is applied:
 * - the window is removed from the workspace where it is visible (if any);
 * - it is added to the given workspace;
 * - other occupation bits are left unchanged.
 *
 * If asked to put it on a too high numbered workspace, put it on
 * the highest possible.
 */
static void EwmhClientMessage_NET_WM_DESKTOP(XClientMessageEvent *msg)
{
	Window w = msg->window;
	TwmWindow *twm_win;
	int occupation;
	VirtualScreen *vs;
	unsigned int val;

	twm_win = GetTwmWindow(w);

	if(twm_win == NULL) {
		return;
	}

	occupation = twm_win->occupation;

	/* Remove from visible workspace */
	if((vs = twm_win->vs) != NULL) {
		occupation &= ~(1 << vs->wsw->currentwspc->number);
	}

	val = (unsigned int)msg->data.l[0];

	/* Add to requested workspace (or to all) */
	if(val == ALL_WORKSPACES) {
		occupation = fullOccupation;
	}
	else if(val < Scr->workSpaceMgr.count) {
		occupation |= 1 << val;
	}
	else {
		occupation |= 1 << (Scr->workSpaceMgr.count - 1);
	}

	ChangeOccupation(twm_win, occupation);
}

/*
 * Delete all properties that should be removed from a withdrawn
 * window.
 */
void EwmhUnmapNotify(TwmWindow *twm_win)
{
	XDeleteProperty(dpy, twm_win->w, XA__NET_WM_DESKTOP);
}

/*
 * Add a new window to _NET_CLIENT_LIST.
 * Newer windows are always added at the end.
 *
 * Look at new_win->iconmanagerlist as an optimization for
 * !LookInList(Scr->IconMgrNoShow, new_win->name, &new_win->class)).
 */
void EwmhAddClientWindow(TwmWindow *new_win)
{
	if(Scr->ewmh_CLIENT_LIST_size == 0) {
		return;
	}
	if(new_win->iconmanagerlist != NULL &&
	                !new_win->iswspmgr &&
	                !new_win->isiconmgr) {
		Scr->ewmh_CLIENT_LIST_used++;
		if(Scr->ewmh_CLIENT_LIST_used > Scr->ewmh_CLIENT_LIST_size) {
			long *tp;
			int tsz = Scr->ewmh_CLIENT_LIST_size;

			Scr->ewmh_CLIENT_LIST_size *= 2;
			tp = realloc(Scr->ewmh_CLIENT_LIST,
			             sizeof(long) * Scr->ewmh_CLIENT_LIST_size);
			if(tp == NULL) {
				Scr->ewmh_CLIENT_LIST_size = tsz;
				fprintf(stderr, "Unable to allocate memory for EWMH client list.\n");
				return;
			}
			Scr->ewmh_CLIENT_LIST = tp;
		}
		if(Scr->ewmh_CLIENT_LIST) {
			Scr->ewmh_CLIENT_LIST[Scr->ewmh_CLIENT_LIST_used - 1] = new_win->w;
		}
		else {
			Scr->ewmh_CLIENT_LIST_size = 0;
			fprintf(stderr, "Unable to allocate memory for EWMH client list.\n");
			return;
		}
		XChangeProperty(dpy, Scr->Root, XA__NET_CLIENT_LIST, XA_WINDOW, 32,
		                PropModeReplace, (unsigned char *)Scr->ewmh_CLIENT_LIST,
		                Scr->ewmh_CLIENT_LIST_used);
	}
}

void EwmhDeleteClientWindow(TwmWindow *old_win)
{
	int i;

	if(old_win->ewmhFlags & EWMH_HAS_STRUT) {
		EwmhRemoveStrut(old_win);
	}

	/*
	 * Remove the window from _NET_CLIENT_LIST.
	 */
	if(Scr->ewmh_CLIENT_LIST_size == 0) {
		return;
	}
	for(i = Scr->ewmh_CLIENT_LIST_used - 1; i >= 0; i--) {
		if(Scr->ewmh_CLIENT_LIST[i] == old_win->w) {
			memmove(&Scr->ewmh_CLIENT_LIST[i],
			        &Scr->ewmh_CLIENT_LIST[i + 1],
			        (Scr->ewmh_CLIENT_LIST_used - 1 - i) * sizeof(Scr->ewmh_CLIENT_LIST[0]));
			Scr->ewmh_CLIENT_LIST_used--;
			if(Scr->ewmh_CLIENT_LIST_used &&
			                (Scr->ewmh_CLIENT_LIST_used * 3) < Scr->ewmh_CLIENT_LIST_size) {
				Scr->ewmh_CLIENT_LIST_size /= 2;
				Scr->ewmh_CLIENT_LIST = realloc(Scr->ewmh_CLIENT_LIST,
				                                sizeof((Scr->ewmh_CLIENT_LIST[0])) * Scr->ewmh_CLIENT_LIST_size);
				/* memory shrinking, shouldn't have problems */
			}
			break;
		}
	}
	/* If window was not found, there is no need to update the property. */
	if(i >= 0) {
		XChangeProperty(dpy, Scr->Root, XA__NET_CLIENT_LIST, XA_WINDOW, 32,
		                PropModeReplace, (unsigned char *)Scr->ewmh_CLIENT_LIST,
		                Scr->ewmh_CLIENT_LIST_used);
	}
}

/*
 * Similar to EwmhAddClientWindow() and EwmhDeleteClientWindow(),
 * but the windows are in stacking order.
 * Therefore we look at the OTPs, which are by definition in the correct order.
 */

void EwmhSet_NET_CLIENT_LIST_STACKING(void)
{
	int size;
	unsigned long *prop;
	TwmWindow *twm_win;
	int i;

	/* Expect the same number of windows as in the _NET_CLIENT_LIST */
	size = Scr->ewmh_CLIENT_LIST_used + 10;
	prop = calloc(size, sizeof(unsigned long));
	if(prop == NULL) {
		return;
	}

	i = 0;
	for(twm_win = OtpBottomWin();
	                twm_win != NULL;
	                twm_win = OtpNextWinUp(twm_win)) {
		if(twm_win->iconmanagerlist != NULL &&
		                !twm_win->iswspmgr &&
		                !twm_win->isiconmgr) {
			prop[i] = twm_win->w;
			i++;
			if(i > size) {
				fprintf(stderr, "Too many stacked windows\n");
				break;
			}
		}
	}

	if(i != Scr->ewmh_CLIENT_LIST_used) {
		fprintf(stderr, "Incorrect number of stacked windows: %d (expected %d)\n",
		        i, Scr->ewmh_CLIENT_LIST_used);
	}

	XChangeProperty(dpy, Scr->Root, XA__NET_CLIENT_LIST_STACKING, XA_WINDOW, 32,
	                PropModeReplace, (unsigned char *)prop, i);

	free(prop);
}

void EwmhSet_NET_ACTIVE_WINDOW(Window w)
{
	unsigned long prop[1];

	prop[0] = w;

	XChangeProperty(dpy, Scr->Root, XA__NET_ACTIVE_WINDOW, XA_WINDOW, 32,
	                PropModeReplace, (unsigned char *)prop, 1);
}

/*
 * Get window properties as relevant when the window is initially mapped.
 *
 * So far, only NET_WM_WINDOW_TYPE and _NET_WM_STRUT_PARTIAL.
 * In particular, most of the initial value of _NET_WM_STATE is ignored. TODO.
 *
 * Also do any generic initialisation needed to EWMH-specific fields
 * in a TwmWindow.
 */
void EwmhGetProperties(TwmWindow *twm_win)
{
	twm_win->ewmhFlags = 0;

	Atom type = EwmhGetWindowProperty(twm_win->w, XA__NET_WM_WINDOW_TYPE, XA_ATOM);

	if(type == XA__NET_WM_WINDOW_TYPE_DESKTOP) {
		twm_win->ewmhWindowType = wt_Desktop;
	}
	else if(type == XA__NET_WM_WINDOW_TYPE_DOCK) {
		twm_win->ewmhWindowType = wt_Dock;
	}
	else {
		twm_win->ewmhWindowType = wt_Normal;
	}
	EwmhGetStrut(twm_win, false);
	/* Only the 3 listed states are supported for now */
	twm_win->ewmhFlags |= EwmhGet_NET_WM_STATE(twm_win) &
	                      (EWMH_STATE_ABOVE | EWMH_STATE_BELOW | EWMH_STATE_SHADED);
}

/* Only used in initially mapping a window */
int EwmhGetInitPriority(TwmWindow *twm_win)
{
	switch(twm_win->ewmhWindowType) {
		case wt_Desktop:
			return EWMH_PRI_DESKTOP;
		case wt_Dock:
			return EWMH_PRI_DOCK;
		default:
			return 0;
	}
}

bool EwmhHasBorder(TwmWindow *twm_win)
{
	switch(twm_win->ewmhWindowType) {
		case wt_Desktop:
		case wt_Dock:
			return false;
		default:
			return true;
	}
}

bool EwmhHasTitle(TwmWindow *twm_win)
{
	switch(twm_win->ewmhWindowType) {
		case wt_Desktop:
		case wt_Dock:
			return false;
		default:
			return true;
	}
}

bool EwmhOnWindowRing(TwmWindow *twm_win)
{
	switch(twm_win->ewmhWindowType) {
		case wt_Desktop:
		case wt_Dock:
			return false;
		default:
			return true;
	}
}

/*
 * Recalculate the effective border values from the remembered struts.
 * Interestingly it is not documented how to do that.
 * Usually only one dock is present on each side, so it shouldn't matter
 * too much, but I presume that maximizing the values is the thing to do.
 */
static void EwmhRecalculateStrut(void)
{
	int left   = 0;
	int right  = 0;
	int top    = 0;
	int bottom = 0;
	EwmhStrut *strut = Scr->ewmhStruts;

	while(strut != NULL) {
		left   = max(left,   strut->left);
		right  = max(right,  strut->right);
		top    = max(top,    strut->top);
		bottom = max(bottom, strut->bottom);

		strut  = strut->next;
	}

	Scr->BorderLeft   = left;
	Scr->BorderRight  = right;
	Scr->BorderTop    = top;
	Scr->BorderBottom = bottom;

	// Bordered layout may have changed
	Scr->BorderedLayout = RLayoutCopyCropped(Scr->Layout,
	                      left, right, top, bottom);
	if(Scr->BorderedLayout == NULL) {
		Scr->BorderedLayout = Scr->Layout;        // nothing to crop
	}

	EwmhSet_NET_WORKAREA(Scr);
}
/*
 * Check _NET_WM_STRUT_PARTIAL or _NET_WM_STRUT.
 * These are basically automatic settings for Border{Left,Right,Top,Bottom}.
 *
 * If any values are found, collect them in a list of strut values
 * belonging to Scr.  When a window is added or removed that has struts,
 * the new effective value must be calculated.  The expectation is that
 * at most a handful of windows will have struts.
 *
 * If update is true, this is called as an update for an existing window.
 */
static void EwmhGetStrut(TwmWindow *twm_win, bool update)
{
	unsigned long nitems;
	unsigned long *prop;
	EwmhStrut *strut;

	prop = EwmhGetWindowProperties(twm_win->w,
	                               XA__NET_WM_STRUT_PARTIAL, XA_CARDINAL,
	                               &nitems);
	if(prop == NULL) {
		prop = EwmhGetWindowProperties(twm_win->w,
		                               XA__NET_WM_STRUT, XA_CARDINAL,  &nitems);
		if(prop == NULL) {
			return;
		}
	}


	if(nitems < 4) {
#ifdef DEBUG_EWMH
		/* This happens a lot, despite returning Success ??? */
		printf("struts: prop = %p, nitems = %ld\n", prop, nitems);
#endif
		XFree(prop);
		return;
	}
#ifdef DEBUG_EWMH
	printf("struts: left %ld, right %ld, top %ld, bottom %ld\n",
	       prop[0], prop[1], prop[2], prop[3]);
#endif

	/*
	 * If there were no struts before, the user configured margins are set
	 * in Border{Left,Right,Top,Bottom}. In order not to lose those values
	 * when recalculating them, convert them to struts for a dummy window.
	 */

	if(Scr->ewmhStruts == NULL &&
	                (Scr->BorderLeft |
	                 Scr->BorderRight |
	                 Scr->BorderTop |
	                 Scr->BorderBottom) != 0) {
		strut = calloc(1, sizeof(EwmhStrut));
		if(strut == NULL) {
			XFree(prop);
			return;
		}

		strut->next   = NULL;
		strut->win    = NULL;
		strut->left   = Scr->BorderLeft;
		strut->right  = Scr->BorderRight;
		strut->top    = Scr->BorderTop;
		strut->bottom = Scr->BorderBottom;

		Scr->ewmhStruts = strut;
	}

	strut = NULL;

	/*
	 * Find the struts of the window that we're supposed to be updating.
	 * If not found, there is no problem: we'll just allocate a new
	 * record.
	 */

	if(update) {
		strut = Scr->ewmhStruts;

		while(strut != NULL) {
			if(strut->win == twm_win) {
				break;
			}
			strut = strut->next;
		}
	}

	/*
	 * If needed, allocate a new struts record and link it in.
	 */
	if(strut == NULL) {
		strut = calloc(1, sizeof(EwmhStrut));
		if(strut == NULL) {
			XFree(prop);
			return;
		}

		strut->next = Scr->ewmhStruts;
		Scr->ewmhStruts = strut;
	}

	strut->win    = twm_win;
	strut->left   = prop[0];
	strut->right  = prop[1];
	strut->top    = prop[2];
	strut->bottom = prop[3];

	XFree(prop);

	/*
	 * Mark this window as having contributed some struts.
	 * This can be checked and undone when the window is deleted.
	 */
	twm_win->ewmhFlags |= EWMH_HAS_STRUT;

	EwmhRecalculateStrut();
}

/*
 * Remove the struts associated with the given window from the
 * remembered list. If found, recalculate the effective borders.
 */
static void EwmhRemoveStrut(TwmWindow *twm_win)
{
	EwmhStrut **prev = &Scr->ewmhStruts;
	EwmhStrut *strut = Scr->ewmhStruts;

	while(strut != NULL) {
		if(strut->win == twm_win) {
			twm_win->ewmhFlags &= ~EWMH_HAS_STRUT;

			*prev = strut->next;
			free(strut);

			EwmhRecalculateStrut();

			break;
		}
		prev = &strut->next;
		strut = strut->next;
	}
}


/**
 * Set _NET_FRAME_EXTENTS property.
 * This tells the client how much space is being taken up by the window
 * decorations.  Some clients may need this information to position other
 * windows on top of themselves.  e.g., Firefox's form autofill and
 * context menu will be positioned a bit wrong (high, by the height of
 * the titlebar) without this.
 */
void EwmhSet_NET_FRAME_EXTENTS(TwmWindow *twm_win)
{
	long data[4];
	const long w = twm_win->frame_bw3D + twm_win->frame_bw;

	data[0] = w; // left
	data[1] = w; // right
	data[2] = twm_win->title_height + w; // top
	data[3] = w; // bottom

	XChangeProperty(dpy, twm_win->w,
	                XA__NET_FRAME_EXTENTS, XA_CARDINAL,
	                32, PropModeReplace,
	                (unsigned char *)data, 4);
}


void EwmhSet_NET_SHOWING_DESKTOP(int state)
{
	unsigned long prop[1];

	prop[0] = state;

	XChangeProperty(dpy, Scr->XineramaRoot, XA__NET_SHOWING_DESKTOP, XA_CARDINAL,
	                32,
	                PropModeReplace, (unsigned char *)prop, 1);
}

/*
 * Set _NET_WM_STATE.
 *
 * TwmWindow.ewmhFlags keeps track of the atoms that should be in
 * the list, so that we don't have to fetch or recalculate them all.
 *
 * XXX It's not clear that the theoretical performance gain and edge-case
 * bug avoidance of the 'changes' arg is worth the complexity and
 * edge-case bug creation it brings.  Consider removing it.
 */
void EwmhSet_NET_WM_STATE(TwmWindow *twm_win, int changes)
{
	unsigned long prop[10];
	int flags;
	int i;

	if(changes & EWMH_STATE_MAXIMIZED_VERT) {
		int newFlags = 0;

		switch(twm_win->zoomed) {
			case F_FULLZOOM:
				newFlags = EWMH_STATE_MAXIMIZED_VERT |
				           EWMH_STATE_MAXIMIZED_HORZ;
				break;
			case F_ZOOM:
			case F_LEFTZOOM:
			case F_RIGHTZOOM:
				newFlags = EWMH_STATE_MAXIMIZED_VERT;
				break;
			case F_HORIZOOM:
			case F_TOPZOOM:
			case F_BOTTOMZOOM:
				newFlags = EWMH_STATE_MAXIMIZED_HORZ;
				break;
			case F_FULLSCREENZOOM:
				newFlags = EWMH_STATE_FULLSCREEN;
				break;
		}

		twm_win->ewmhFlags &= ~(EWMH_STATE_MAXIMIZED_VERT |
		                        EWMH_STATE_MAXIMIZED_HORZ |
		                        EWMH_STATE_FULLSCREEN);
		twm_win->ewmhFlags |= newFlags;
	}

	if(changes & EWMH_STATE_SHADED) {
		if(twm_win->squeezed) {
			twm_win->ewmhFlags |= EWMH_STATE_SHADED;
		}
		else {
			twm_win->ewmhFlags &= ~EWMH_STATE_SHADED;
		}
	}

	if(changes & (EWMH_STATE_ABOVE | EWMH_STATE_BELOW)) {
		int pri = OtpEffectiveDisplayPriority(twm_win);

		twm_win->ewmhFlags &= ~(EWMH_STATE_ABOVE | EWMH_STATE_BELOW);

		/*
		 * If it's a DOCK or DESKTOP, and where we expect those to be, we
		 * consider that there's nothing to tell it.  Otherwise, we tell
		 * it ABOVE/BELOW based on where it effectively is.
		 */
		if(twm_win->ewmhWindowType == wt_Dock && pri == EWMH_PRI_DOCK) {
			pri = 0;
		}
		if(twm_win->ewmhWindowType == wt_Desktop && pri == EWMH_PRI_DESKTOP) {
			pri = 0;
		}

		if(pri > 0) {
			twm_win->ewmhFlags |= EWMH_STATE_ABOVE;
		}
		else if(pri < 0) {
			twm_win->ewmhFlags |= EWMH_STATE_BELOW;
		}
	}

	flags = twm_win->ewmhFlags;
	i = 0;

	if(flags & EWMH_STATE_MAXIMIZED_VERT) {
		prop[i++] = XA__NET_WM_STATE_MAXIMIZED_VERT;
	}
	if(flags & EWMH_STATE_MAXIMIZED_HORZ) {
		prop[i++] = XA__NET_WM_STATE_MAXIMIZED_HORZ;
	}
	if(flags & EWMH_STATE_FULLSCREEN) {
		prop[i++] = XA__NET_WM_STATE_FULLSCREEN;
	}
	if(flags & EWMH_STATE_SHADED) {
		prop[i++] = XA__NET_WM_STATE_SHADED;
	}
	if(flags & EWMH_STATE_ABOVE) {
		prop[i++] = XA__NET_WM_STATE_ABOVE;
	}
	if(flags & EWMH_STATE_BELOW) {
		prop[i++] = XA__NET_WM_STATE_BELOW;
	}

	XChangeProperty(dpy, twm_win->w, XA__NET_WM_STATE, XA_ATOM, 32,
	                PropModeReplace, (unsigned char *)prop, i);
}

/*
 * Get the initial state of _NET_WM_STATE.
 *
 * Only some of the flags are supported when initially creating a window.
 */
static int EwmhGet_NET_WM_STATE(TwmWindow *twm_win)
{
	int flags = 0;
	unsigned long *prop;
	unsigned long nitems;
	int i;

	prop = EwmhGetWindowProperties(twm_win->w, XA__NET_WM_STATE, XA_ATOM, &nitems);

	if(prop) {
		for(i = 0; i < nitems; i++) {
			flags |= atomToFlag(prop[i]);
		}

		XFree(prop);
	}

	return flags;
}

/*
 * Set _NET_WORKAREA.
 */
static void EwmhSet_NET_WORKAREA(ScreenInfo *scr)
{
	unsigned long prop[4];

	/* x */ prop[0] = scr->BorderLeft;
	/* y */ prop[1] = scr->BorderTop;
	/* w */ prop[2] = scr->rootw - scr->BorderLeft - scr->BorderRight;
	/* h */ prop[3] = scr->rooth - scr->BorderTop - scr->BorderBottom;

	XChangeProperty(dpy, Scr->XineramaRoot, XA__NET_WORKAREA, XA_CARDINAL, 32,
	                PropModeReplace, (unsigned char *)prop, 4);
}

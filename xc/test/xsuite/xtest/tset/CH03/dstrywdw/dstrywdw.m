/*
 
Copyright (c) 1990, 1991  X Consortium

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of the X Consortium shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from the X Consortium.

 *
 * Copyright 1990, 1991 by UniSoft Group Limited.
 * 
 * Permission to use, copy, modify, distribute, and sell this software and
 * its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation, and that the name of UniSoft not be
 * used in advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.  UniSoft
 * makes no representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 *
 * $XConsortium: dstrywdw.m,v 1.11 94/04/17 21:03:03 rws Exp $
 */
>>TITLE XDestroyWindow CH03
void

Display	*display = Dsp;
Window	w;
>>EXTERN

#define	ON_WINDOW	0x0001
#define	ON_PARENT	0x0002

extern	char	*STreeSgl[];
extern	int 	NSTreeSgl;

>>ASSERTION Good A
A call to xname destroys the specified window and all of its
subwindows.
>>STRATEGY
Create base window.
Create window with inferiors on base window to destroy.
Call xname on window.
Verify that the window no longer exists.
Verify that all inferiors of the window no longer exist.
Verify by pixel checking that all traces of the windows have been removed from
the base window.
>>CODE
Window	parent;
struct	buildtree	*btlist;
int 	i;

	parent = defdraw(display, VI_WIN);
	btlist = buildtree(display, parent, STreeSgl, NSTreeSgl);

	/*
	 * Get the window that we are going to destroy and draw a pattern
	 * on it.
	 */
	w = btlist[1].wid;
	pattern(display, w);

	/*
	 * Check that each window exists.
	 * Touch each window with MapWindow, check for no error.
	 */
	for (i = 1; i < NSTreeSgl; i++) {
		CATCH_ERROR(display);
		XMapWindow(display, btlist[i].wid);
		RESTORE_ERROR(display);
		if (GET_ERROR(display) != Success) {
			delete("Error in setting up window tree");
			return;
		} else
			CHECK;
	}

	XCALL;

	/* 
	 * Check that parent window is now clear, all traces of the window
	 * have gone.
	 */
	if (checkclear(display, parent))
		CHECK;
	else {
		report("Parent window was not clear after destroying its inferior");
		FAIL;
	}

	/*
	 * Check that we now get a BadWindow on the window and all its inferiors.
	 */
	for (i = 1; i < NSTreeSgl; i++) {
		CATCH_ERROR(display);
		XMapWindow(display, btlist[i].wid);
		RESTORE_ERROR(display);
		if (GET_ERROR(display) != BadWindow) {
			report("The window '%s' was still accessible", btlist[i].name);
			FAIL;
		} else
			CHECK;
	}
	CHECKPASS(2*(NSTreeSgl-1)+1);

>>ASSERTION Good A
When a call to xname destroys a window and the
window is mapped, then the window is unmapped first and an
.S UnmapNotify 
event is generated.
>>STRATEGY
Create a base window and a window with inferiors on the base window.
Enable events on each window.
Call xname to destroy a window.
Check that an UnmapNotify event was generated on the destroyed window
	and it's the parent window. 
Verify that the correct windows had notifications
>>CODE
XEvent	ev;
XUnmapEvent	good;
XUnmapEvent	*ump;
Window	parent;
struct	buildtree	*btlist;
struct	buildtree	*btp;
int 	i;

/* Create a base window and a window with inferiors on the base window. */
	parent = defdraw(display, VI_WIN);
	btlist = buildtree(display, parent, STreeSgl, NSTreeSgl);

	/* Set window that we are to destroy */
	w = btlist[1].wid;

/* Enable events on each window. */
	for (i = 0; i < NSTreeSgl; i++)
		XSelectInput(display, btlist[i].wid, ALLEVENTS);
	XSelectInput(display, parent, ALLEVENTS);

/* Call xname to destroy a window. */
	XCALL;

	good.type = UnmapNotify;
	good.serial = 0;
	good.send_event = False;
	good.display = display;
	good.from_configure = False;

/* Check that an UnmapNotify event was generated on the destroyed window */
/* 	and it's the parent window only.  */
	while(XCheckTypedEvent(display, UnmapNotify, &ev)) {
		ump = (XUnmapEvent *)&ev;
		debug(1,"event=0x%x, window=0x%x", ump->event, ump->window);

		btp = btwtobtp(btlist, ump->window);
		if (btp == (struct buildtree *)0) {
			FAIL;
			report("Event received on window that was not part of the test");
			report("Window ID is %x0x", ump->window);
			continue;
		} else
			CHECK;

		if (ump->window == ump->event) {
			/* Event received on window itself */
			if (btp->uflags & ON_WINDOW) {
				report("Repeat UnmapNotify event received on window '%s'",
					btp->name);
				FAIL;
			} else
				CHECK;
			btp->uflags |= ON_WINDOW;
		} else {

			/* Check event was received on the parent */
			if (ump->event == parent) {
				/*
				 * This is the Unmap reported to the parent because it was
				 * selecting SubstructureNotifyMask.
				 */
				if (ump->window != w) {
					report("UnmapNotify event was reported to the parent that on as not on the test window");
					FAIL;
				} else
					CHECK;
			} else if (btp->parent->wid != ump->event) {
				report("UnmapNotify event was reported on a window that was not the parent");
				FAIL;
			} else if (btp->uflags & ON_PARENT) {
				report("Repeat UnmapNotify event received on parent for window '%s'",
					btp->name);
				FAIL;
			} else
				CHECK;
			btp->uflags |= ON_PARENT;
		}

		/*
		 * Event and window have been dealt with - now check the other
		 * fields in the event.
		 */
		good.event = ump->event;
		good.window = ump->window;

		if (checkevent((XEvent*)&good, &ev)) {
			report("Error in event");
			FAIL;
		} else
			CHECK;

	}

	for (i = 1; i < NSTreeSgl; i++) {

		debug(2, "Checking window %s",btlist[i].name);

		if (btlist[i].wid == w) {
			if ((btlist[i].uflags & ON_WINDOW)==0) {
				FAIL;
				report("Unmap event was not received on window '%s'",
					btlist[i].name);
			} else
				CHECK;

			if ((btlist[i].uflags & ON_PARENT)==0) {
				FAIL;
				report("Unmap event was not received on the parent of window '%s'",
					btlist[i].name);
			} else
				CHECK;
		} else {
			if (btlist[i].uflags & ON_WINDOW) { 
				FAIL;
				report("Unmap event was received on window '%s'",
					btlist[i].name);
			} else
				CHECK;

			if (btlist[i].uflags & ON_PARENT) { 
				FAIL;
				report("Unmap event was received on the parent of window '%s'",
					btlist[i].name);
			} else
				CHECK;
		}
	}

	CHECKPASS(2*3 + 2*(NSTreeSgl-1));

>>ASSERTION Good A
On a call to xname a
.S DestroyNotify 
event is generated for each window that is destroyed
such that for any given window being destroyed,
.S DestroyNotify 
is generated on any inferiors of that window before being generated on
the given window itself.
>>STRATEGY
Create base window.
Create window on base window to destroy.
Create stack of windows on this window.
Enable events on each window.
Call xname on window.
Verify that a DestroyNotify event is generated for each window.
Verify that a DestroyNotify event is generated on the parent of each window.
>>CODE
XEvent	ev;
XDestroyWindowEvent	good;
XDestroyWindowEvent	*dnp;
Window	parent;
struct	buildtree	*btlist;
struct	buildtree	*btp;
int 	i;

	parent = defdraw(display, VI_WIN);
	btlist = buildtree(display, parent, STreeSgl, NSTreeSgl);

	/* Window to be destroyed */
	w = btlist[1].wid;

	/*
	 * Unmap a window and make sure that everything has happened before we
	 * enable events.
	 */
	btp = btntobtp(btlist, "A1");
	btp->opts |= BT_UNMAP;
	XUnmapWindow(display, btp->wid);
	XSync(display, False);

	for (i = 0; i < NSTreeSgl; i++)
		XSelectInput(display, btlist[i].wid, ALLEVENTS);
	XSelectInput(display, parent, ALLEVENTS);

	XCALL;

	good.type = DestroyNotify;
	good.serial = 0;
	good.send_event = False;
	good.display = display;

	/*
	 * This loop continues until there are no more events left.  (In case
	 * there are extra or repeated events).  The loop should be traversed
	 * 2*(NSTreeSgl-1) times.
	 */
	for (i = 0; ; i++) {
		if (XCheckTypedEvent(display, DestroyNotify, &ev) == False)
			break;
		dnp = (XDestroyWindowEvent*)&ev;

		debug(1, "event=0x%x, window=0x%x", dnp->event, dnp->window);

		/*
		 * If window == event then this is the event from StructureNotify,
		 * if they are different then this is the event from the
		 * SubstructureNotify.  Keep track of events received and check
		 * that event is actualy set to the parent of window when they
		 * are different.
		 * Check that a destroy event has not yet been received on the
		 * parent window.
		 * Finally check the other members of the Event.
		 */

		btp = btwtobtp(btlist, dnp->window);
		if (btp == (struct buildtree *)0) {
			report("Event received for a window that was not part of the test");
			report("  Window ID was 0x%x", dnp->window);
			FAIL;
			break;
		}

		debug(1, "DestroyNotify received for window %s", btp->name);

		if (dnp->window == dnp->event) {
			if (btp->uflags & ON_WINDOW) {
				report("Repeat DestroyNotify event received on window '%s'",
					btp->name);
				FAIL;
			} else if (btp->parent->uflags & ON_WINDOW) {
				/* Test ordering */
				report("DestroyNotify event received on parent before child");
				FAIL;
			} else
				CHECK;
			btp->uflags |= ON_WINDOW;
		} else {

			/* Check event was received on the parent */
			if (dnp->event == parent) {
				/*
				 * This is the Unmap reported to the parent because it was
				 * selecting SubstructureNotifyMask.
				 */
				if (dnp->window != w) {
					report("UnmapNotify event was reported on a window that was not the parent");
					FAIL;
				} else
					CHECK;
			} else if (btp->parent->wid != dnp->event) {
				report("DestroyNotify event was reported on a window that was not the parent");
				FAIL;
			} else if (btp->uflags & ON_PARENT) {
				report("DestroyNotify event already received on parent for window '%s'",
					btp->name);
				FAIL;
			} else
				CHECK;
			btp->uflags |= ON_PARENT;
		}

		/* Check other fields in event */
		good.event = dnp->event;
		good.window = dnp->window;

		if (checkevent((XEvent*)&good, &ev)) {
			report("Error in event");
			FAIL;
		} else
			CHECK;

	}

	/* Verify that the window and all subwindows had both notifications */
	for (i = 1; i < NSTreeSgl; i++) {

		if ((btlist[i].uflags & ON_WINDOW) == 0) {
			report("DestroyNotify event was not received on subwindow '%s'",
				btlist[i].name);
			FAIL;
		} else
			CHECK;
			
		if ((btlist[i].uflags & ON_PARENT) == 0) {
			report("DestroyNotify event was not received on the parent of subwindow '%s'",
				btlist[i].name);
			FAIL;
		} else
			CHECK;
	}

	CHECKPASS((2*2*(NSTreeSgl-1)) + 2*(NSTreeSgl-1));

>>ASSERTION Good A
When the specified window is a root window, then no windows are destroyed.
>>STRATEGY
Create a window.
Attempt to destroy root window.
Verify that the root and our window are still valid.
>>CODE
Window	ourwin;
XWindowAttributes	atts;

	/*
	 * What would happen if the root really was destroyed?!
	 * Just do a couple of simple checks to make sure that nothing
	 * bad has happened.
	 */
	ourwin = defdraw(display, VI_WIN);
	w = DefaultRootWindow(display);

	XCALL;

	CATCH_ERROR(display);
	XGetWindowAttributes(display, DRW(display), &atts);
	RESTORE_ERROR(display);
	if (GET_ERROR(display) == BadWindow) {
		report("Root window was destroyed");	/* ! */
		FAIL;
	} else
		CHECK;

	CATCH_ERROR(display);
	XGetWindowAttributes(display, ourwin, &atts);
	RESTORE_ERROR(display);
	if (GET_ERROR(display) == BadWindow) {
		report("A child of the root window was destroyed when attempting to destroy root window");
		FAIL;
	} else
		CHECK;

	CHECKPASS(2);

>>ASSERTION Good A
When a call to xname
uncovers part of any window that was formerly obscured, then
either
.S Expose
events are generated or the contents are restored from backing store.
>>STRATEGY
Create base window.
Call setforexpose() on base window.
Create window on base to destroy.
Destroy window.
Verify expose processing or backing store with exposecheck().
>>CODE
Window	parent;
struct	area	area;

	parent = defdraw(display, VI_WIN);
	setforexpose(display, parent);
	XSelectInput(display, parent, ExposureMask);

	setarea(&area, 25, 35, 17, 46);
	w = crechild(display, parent, &area);

	XCALL;

	if (exposecheck(display, parent))
		CHECK;
	else {
		report("Neither Expose events or backing store processing");
		report("could correctly restore the window contents.");
		FAIL;
	}

	CHECKPASS(1);
>>ASSERTION Bad A
.ER BadWindow

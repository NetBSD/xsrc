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

 * Copyright 1990, 1991 UniSoft Group Limited.
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
 * $XConsortium: dstrysbws.m,v 1.10 94/04/17 21:03:03 rws Exp $
 */
>>TITLE XDestroySubwindows CH03
void

Display	*display = Dsp;
Window	w;
>>EXTERN

#define	ON_WINDOW	0x0001
#define	ON_PARENT	0x0002

>>ASSERTION Good A
A call to xname destroys all inferior windows of the specified window,
>># This ordering seem dubious, since it is not the same as the ordering
>># for the events.
in bottom-to-top stacking order.
>>STRATEGY
Create stack of windows.
Call xname on window.
Verify that this window still exists.
Verify that all inferiors no longer exist.
Verify by pixel checking that all traces of the windows have been removed from
the parent.
>>EXTERN
extern	char	*STreeGen[];
extern	int 	NSTreeGen;
extern	char	*STreeOlsib[];
extern	int 	NSTreeOlsib;
>>CODE
struct	buildtree	*btp;
int 	i;

	w = defdraw(display, VI_WIN);
	btp = buildtree(display, w, STreeGen, NSTreeGen);

	/*
	 * Check that each window exists.
	 * Touch each window with MapWindow, check for no error.
	 */
	for (i = 0; i < NSTreeGen; i++) {
		CATCH_ERROR(display);
		XMapWindow(display, btp[i].wid);
		RESTORE_ERROR(display);
		if (GET_ERROR(display) != Success) {
			delete("Error in setting up window tree");
			return;
		} else
			CHECK;
	}

	XCALL;

	/* Check that parent window is now clear */
	if (checkclear(display, w))
		CHECK;
	else {
		report("Parent window was not clear after destroying its inferiors");
		FAIL;
	}

	/*
	 * Check that we now get a BadWindow on all the subwindows.
	 * (and not on the parent)
	 */
	for (i = 0; i < NSTreeGen; i++) {
		CATCH_ERROR(display);
		XMapWindow(display, btp[i].wid);
		RESTORE_ERROR(display);
		if (i == 0) {
			if (GET_ERROR(display) == BadWindow) {
				report("The parent window was destroyed");
				FAIL;
			} else
				CHECK;
		} else {
			if (GET_ERROR(display) != BadWindow) {
				report("An inferior was still accessible");
				FAIL;
			} else
				CHECK;
		}
	}
	CHECKPASS(2*NSTreeGen+1);

>>ASSERTION Good A
When a call to xname destroys a subwindow and the
subwindow is mapped, then the subwindow is unmapped first and an
.S UnmapNotify 
event is generated.
>>STRATEGY
Create stack of windows.
Unmap one window.
Enable events on each window.
Call xname on window.
Verify that an UnmapNotify event is generated for each mapped subwindow.
Verify that an UnmapNotify event is generated on the parent of each mapped
subwindow.
Verify that no UnmapNotify event is generated for the unmapped window.
Verify all fields of events.
>>CODE
XEvent	ev;
XUnmapEvent	good;
XUnmapEvent	*ump;
struct	buildtree	*btlist;
struct	buildtree	*btp;
int 	i;

	w = defdraw(display, VI_WIN);
	btlist = buildtree(display, w, STreeGen, NSTreeGen);

	/*
	 * Unmap window and make sure that everything has happened before we
	 * enable events.
	 */
	btp = btntobtp(btlist, "A");
	btp->opts |= BT_UNMAP;
	XUnmapWindow(display, btp->wid);
	XSync(display, False);

	for (i = 0; i < NSTreeGen; i++)
		XSelectInput(display, btlist[i].wid, ALLEVENTS);

	XCALL;

	/*
	 * Set up the known good event.
	 */
	good.type = UnmapNotify;
	good.serial = 0;
	good.send_event = False;
	good.display = display;
	good.from_configure = False;

	/*
	 * Loop until no events left.  Should go round twice, for window B only.
	 */
	for (i = 0; ; i++) {
		if (XCheckTypedEvent(display, UnmapNotify, &ev) == False)
			break;
		ump = (XUnmapEvent*)&ev;

		debug(1, "event=0x%x, window=0x%x", ump->event, ump->window);

		/*
		 * If window == event then this is the event from StructureNotify,
		 * if they are different then this is the event from the
		 * SubstructureNotify.  Keep track of events received and check
		 * that event is actually set to the parent of window when they
		 * are different.
		 * Finally check the other members of the Event.
		 */

		btp = btwtobtp(btlist, ump->window);
		if (btp == (struct buildtree *)0) {
			report("Event received on window that was not part of test");
			report("  Window ID is 0x%x", ump->window);
			FAIL;
			break;
		}
		debug(1, "Unmap received for window '%s'", btp->name);

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
			if (btp->parent->wid != ump->event) {
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

	/* Verify that no unmaps were recorded for the parent window */
	if (btlist[0].uflags != 0) {
		report("Unmap events reported on parent window");
		FAIL;
	} else
		CHECK;

	/* Verify that all subwindows had both notifications */
	for (i = 1; i < NSTreeGen; i++) {

		/*
		 * Check that windows that were already unmapped did not get
		 * an event.
		 */
		if (btlist[i].opts & BT_UNMAP) {
			if (btlist[i].uflags & ON_WINDOW) {
				report("Unmap event received for unmapped window '%s'",
					btlist[i].name);
				FAIL;
			} else
				CHECK;
			if (btlist[i].uflags & ON_PARENT) {
				report("Unmap event received on parent for unmapped window '%s'"
					, btlist[i].name);
				FAIL;
			} else
				CHECK;
			continue;
		}

		/*
		 * Check that Unmap events are received on the expected window.
		 */
		if (btlist[i].parent->wid == w) {
			if ((btlist[i].uflags & ON_WINDOW) == 0) {
				report("Unmap event was not received on subwindow '%s'",
					btlist[i].name);
				FAIL;
			} else
				CHECK;
			
			if ((btlist[i].uflags & ON_PARENT) == 0) {
				report("Unmap event was not received on the parent of subwindow '%s'",
					btlist[i].name);
				FAIL;
			} else
				CHECK;
		} else {
			if (btlist[i].uflags & ON_WINDOW) {
				report("Unexpected Unmap event was received on subwindow '%s'",
					btlist[i].name);
				FAIL;
			} else
				CHECK;
			
			if (btlist[i].uflags & ON_PARENT) {
				report("Unexpected Unmap event was received on the parent of subwindow '%s'",
					btlist[i].name);
				FAIL;
			} else
				CHECK;
		}
			
	}

	CHECKPASS((2*2) + 1 + 2*(NSTreeGen-1));

>>ASSERTION Good A
On a call to xname a
.S DestroyNotify
event is generated for each window that is destroyed
with events being generated for all inferiors of that window before being
generated for the given window.
>>STRATEGY
Create stack of windows.
Enable events on each window.
Call xname on window.
Verify that a DestroyNotify event is generated for each subwindow.
Verify that a DestroyNotify event is generated on the parent of each subwindow.
>>CODE
XEvent	ev;
XDestroyWindowEvent	good;
XDestroyWindowEvent	*dnp;
struct	buildtree	*btlist;
struct	buildtree	*btp;
int 	i;

	w = defdraw(display, VI_WIN);
	btlist = buildtree(display, w, STreeGen, NSTreeGen);

	/*
	 * Unmap a window and make sure that everything has happened before we
	 * enable events.
	 */
	btp = btntobtp(btlist, "A1");
	btp->opts |= BT_UNMAP;
	XUnmapWindow(display, btp->wid);
	XSync(display, False);

	for (i = 0; i < NSTreeGen; i++)
		XSelectInput(display, btlist[i].wid, ALLEVENTS);

	XCALL;

	good.type = DestroyNotify;
	good.serial = 0;
	good.send_event = False;
	good.display = display;

	/*
	 * This loop continues until there are no more events left.  (In case
	 * there are extra or repeated events).  The loop should be traversed
	 * 2*(NSTreeGen-1) times.
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
		 * that event is actually set to the parent of window when they
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
			if (btp->parent->wid != dnp->event) {
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

	/* Verify that no destroy events were recorded for the parent window */
	if (btlist[0].uflags != 0) {
		report("DestroyNotify events reported on parent window");
		FAIL;
	} else
		CHECK;

	/* Verify that all subwindows had both notifications */
	for (i = 1; i < NSTreeGen; i++) {

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

	CHECKPASS((2*2*(NSTreeGen-1)) + 1 + 2*(NSTreeGen-1));

>>ASSERTION Good A
When a call to xname
uncovers part of any window that was formerly obscured, then
either
.S Expose
events are generated or the contents are restored from backing store.
>>STRATEGY
Create stack of windows.
Set up window with setforexpose().
Enable events on parent window.
Call xname on window.
Verify that window was restored from backing store, or that expose events were
generated, using exposecheck().
>>CODE

	w = defdraw(display, VI_WIN);
	setforexpose(display, w);

	buildtree(display, w, STreeOlsib, NSTreeOlsib);
	if (isdeleted())
		return;

	XSelectInput(display, w, ALLEVENTS);

	XCALL;

	XSelectInput(display, w, NoEventMask);

	if (exposecheck(display, w))
		CHECK;
	else {
		report("Neither Expose events or backing store processing");
		report("could correctly restore the window contents.");
		FAIL;
	}

	CHECKPASS(1);
>>ASSERTION Bad A
.ER BadWindow

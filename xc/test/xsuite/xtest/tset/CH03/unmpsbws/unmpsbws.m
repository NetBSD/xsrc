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
 * $XConsortium: unmpsbws.m,v 1.7 94/04/17 21:03:23 rws Exp $
 */
>>TITLE XUnmapSubwindows CH03
void

Display	*display = Dsp;
Window	w;
>>EXTERN

/*
 * Tree of children (which are unmapped) and other descendents (which are
 * not).  One window is already unmapped.
 */
static	char	*T1[] = {
	".",
	"A . (5,5) 30x10",
	  "A1 A (2,2) 12x4",
	"B . (10,20) 10x26",
	  "B1 B (1,5) 5x4",
	"C . (66,16) 17x45",
	  "C1 C (4,10) 10x5",
	  "C2 C (4,30) 10x5",
	"D . (37,32) 15x8 unmap",
	"E . (30,58) 9x14",
	"F . (2,70) 21x19",
	  "F1 F (3,3) 8x7",
};
#define	NT1	(NELEM(T1))		/* Number of windows */
#define	NT1SUB	(NELEM(T1)-1)	/* Number of subwindows */
#define	NT1MAPSUB	5	/* A, B, C, E, F */

>>ASSERTION Good A
A call to xname unmaps all subwindows of the specified window in bottom to top
stacking order.
>>STRATEGY
Create subwindows.
Draw on subwindows.
Unmap subwindows.
Verify that map state of each child is IsUnmapped.
Verify that map state of inferiors of children is IsUnviewable.
Verify that subwindow have disappeared from screen.
>>CODE
struct	buildtree	*bt;
XWindowAttributes	atts;
int 	i;

	w = defwin(display);
	bt = buildtree(display, w, T1, NT1);

	for (i = 1; i < NT1; i++)
		pattern(display, bt[i].wid);

	XCALL;

	for (i = 1; i < NT1; i++) {
		XGetWindowAttributes(display, bt[i].wid, &atts);

		if (bt[i].parent == bt) {
			/* This is a direct child of the test window */
			if (atts.map_state != IsUnmapped) {
				report("After unmap of window '%s', map-state was %s, expecting IsUnmapped",
					bt[i].name, mapstatename(atts.map_state));
				FAIL;
			} else
				CHECK;
		} else {
			/* This is an inferior of a child */
			if (atts.map_state != IsUnviewable) {
				report("After unmap of window '%s', map-state was %s, expecting IsUnviewable",
					bt[i].name, mapstatename(atts.map_state));
				FAIL;
			} else
				CHECK;
		}
	}

	if (checkclear(display, w))
		CHECK;
	else {
		report("Window did not disappear after unmapping");
		FAIL;
	}

	CHECKPASS(NT1SUB+1);
>>ASSERTION Good A
>># QUESTION:	Are the UnmapNotify events ordered? c.f. XMapSubwindows
>>#			kieron
A call to xname generates an
.S UnmapNotify 
event on each subwindow that was originally mapped.
>>STRATEGY
Create parent window.
Create inferior windows.
Select SubstructureNotify on parent.
Select StructureNotify on inferiors.
Call xname.
Verify that UnmapNotify is received on child windows.
Verify that UnmapNotify is received on parent for each child.
Verify that not events are received for other inferiors.
>>EXTERN

#define	GOT_WINDOW	01
#define	GOT_PARENT	02

>>CODE
XEvent	ev;
XUnmapEvent	*ump;
XUnmapEvent	good;
struct	buildtree	*bt;
struct	buildtree	*btp;
int 	i;

	w = defwin(display);
	bt = buildtree(display, w, T1, NT1);

	for (i = 1; i < NT1; i++) {
		pattern(display, bt[i].wid);
		XSelectInput(display, bt[i].wid, StructureNotifyMask);
	}

	XSelectInput(display, w, SubstructureNotifyMask);

	XCALL;

	for (i = 1; i < NT1; i++)
		XSelectInput(display, bt[i].wid, NoEventMask);
	XSelectInput(display, w, NoEventMask);

	/* This loop is traversed twice for each mapped child */
	while (getevent(display, &ev)) {
		if (ev.type != UnmapNotify) {
			report("Event of type %s was received unexpectedly", eventname(ev.type));
			FAIL;
			continue;
		} else
			CHECK;

		ump = (XUnmapEvent*)&ev;
		btp = btwtobtp(bt, ump->window);
		if (btp == NULL) {
			report("Event received for a window that was not in the test");
			FAIL;
			continue;
		}

		/*
		 * This checks the window and event fields.  As events arrive they
		 * are recorded in the bt structure.
		 */
		if (ump->event == ump->window) {
			/* Event arrived on the window */
			if (btp->uflags & GOT_WINDOW) {
				report("Repeat event received on window '%s'", btp->name);
				FAIL;
			} else {
				btp->uflags |= GOT_WINDOW;
				CHECK;
			}
		} else if (ump->event != w) {
			/* Event arrived somewhere unexpected */
			report("Event received for window '%s' on a window other than the parent", btp->name);
			FAIL;
		} else {
			/* Event arrived on the parent */
			if (btp->uflags & GOT_PARENT) {
				report("Repeat event received on parent for window '%s'", btp->name);
				FAIL;
			} else {
				btp->uflags |= GOT_PARENT;
				CHECK;
			}
		}

		/* Check the rest of the fields */
		good.type = UnmapNotify;
		good.serial = 0L;
		good.send_event = False;
		good.display = display;
		good.event = ump->event;
		good.window = ump->window;
		good.from_configure = False;

		if (checkevent((XEvent*)&good, &ev))
			FAIL;
		else
			CHECK;
	}

	for (i = 1; i < NT1; i++) {
		/*
		 * If this is a mapped child then should have events for
		 * window and parent.
		 * Otherwise and for other inferiors - no events.
		 */
		if (bt[i].parent == bt && !(bt[i].opts & BT_UNMAP)) {
			/* child */
			if (bt[i].uflags & GOT_WINDOW)
				CHECK;
			else {
				report("Did not receive unmap event for window '%s'", bt[i].name);
				FAIL;
			}

			if (bt[i].uflags & GOT_PARENT)
				CHECK;
			else {
				report("Did not receive unmap event on parent for window '%s'", bt[i].name);
				FAIL;
			}
		} else {
			/* Other inferior or unmapped */
			if (bt[i].uflags & GOT_WINDOW) {
				report("Received unexpected unmap event for window '%s'", bt[i].name);
				if (bt[i].opts & BT_UNMAP)
					report("  window was already unmapped");
				FAIL;
			} else
				CHECK;

			if (bt[i].uflags & GOT_PARENT) {
				/* Getting here is very wrong */
				report("Received unexpected unmap event on parent of window '%s'", bt[i].name);
				if (bt[i].opts & BT_UNMAP)
					report("  window was already unmapped");
				FAIL;
			} else
				CHECK;
		}
	}

	CHECKPASS(3*2*NT1MAPSUB+2*NT1SUB);
>>ASSERTION Good A
When all the subwindows are already unmapped, then a call to xname
has no effect.
>>STRATEGY
Create parent window.
Create inferiors.
Unmap all children.
Select StructureNotify on inferiors.
Call xname to unmap subwindows again.
Verify that no UnmapNotify event is received on window.
Verify that no change occurs on screen.
>>CODE
struct	buildtree	*bt;
XImage	*imp;
int 	n;
int 	i;

	w = defwin(display);
	bt = buildtree(display, w, T1, NT1);

	XCALL;

	for (i = 1; i < NT1; i++) {
		pattern(display, bt[i].wid);
		XSelectInput(display, bt[i].wid, StructureNotifyMask);
	}
	imp = savimage(display, w);

	XCALL;

	for (i = 1; i < NT1; i++)
		XSelectInput(display, bt[i].wid, NoEventMask);

	n = XPending(display);
	if (n != 0) {
		report("Received event when subwindows already unmapped");
		FAIL;
	} else
		CHECK;

	if (compsavimage(display, w, imp))
		CHECK;
	else {
		report("Screen contents changed when unmapped windows were unmapped again");
		FAIL;
	}

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
Enable expose events.
Create subwindows to unmap.
Call xname to unmap subwindows.
Verify expose or backing store restore occurred with exposecheck().
>>CODE

	w = defwin(display);
	(void) buildtree(display, w, T1, NT1);

	setforexpose(display, w);
	XSelectInput(display, w, ExposureMask);

	XCALL;

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

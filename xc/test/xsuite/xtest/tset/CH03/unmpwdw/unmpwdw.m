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
 * $XConsortium: unmpwdw.m,v 1.7 94/04/17 21:03:24 rws Exp $
 */
>>TITLE XUnmapWindow CH03
void

Display	*display = Dsp;
Window	w;
>>ASSERTION Good A
A call to xname unmaps the specified window.
>>STRATEGY
Create window.
Draw on window.
Unmap window.
Verify that map state is IsUnmapped.
Verify that window has disappeared from screen.
>>CODE
Window	base;
struct	area	area;
XWindowAttributes	atts;

	base = defwin(display);
	setarea(&area, 10, 10, 50, 50);
	w = crechild(display, base, &area);
	pattern(display, w);

	XCALL;

	XGetWindowAttributes(display, w, &atts);
	if (atts.map_state != IsUnmapped) {
		report("After unmap map-state was %s, expecting IsUnmapped",
			mapstatename(atts.map_state));
		FAIL;
	} else
		CHECK;

	if (checkclear(display, base))
		CHECK;
	else {
		report("Window did not disappear after unmapping");
		FAIL;
	}

	CHECKPASS(2);
>>ASSERTION Good A
When the specified window is mapped, then a call to xname generates an
.S UnmapNotify
event.
>>STRATEGY
Create parent window.
Create child window.
Select SubstructureNotify on parent.
Select StructureNotify on child.
Call xname to unmap child window.
Verify that UnmapNotify is received on window.
Verify that UnmapNotify is received on parent.
>>CODE
Window	base;
struct	area	area;
XWindowAttributes	atts;
XEvent	ev;
XUnmapEvent	*ump;
XUnmapEvent	good;
int 	got_parent = 0;
int 	got_window = 0;
int 	n;

	base = defwin(display);
	setarea(&area, 10, 10, 50, 50);
	w = crechild(display, base, &area);
	pattern(display, w);

	XSelectInput(display, base, SubstructureNotifyMask);
	XSelectInput(display, w, StructureNotifyMask);

	XCALL;

	XSelectInput(display, base, NoEventMask);
	XSelectInput(display, w, NoEventMask);

	n = XPending(display);
	if (n != 2) {
		report("Wrong number of events received got %d, expecting 2", n);
		FAIL;
	} else
		CHECK;

	got_window = 0;
	got_parent = 0;
	while (getevent(display, &ev)) {
		if (ev.type != UnmapNotify) {
			report("Event of type %s was received unexpectedly", eventname(ev.type));
			FAIL;
			continue;
		} else
			CHECK;

		ump = (XUnmapEvent*)&ev;

		if (ump->event == ump->window) {
			got_window++;
		} else if (ump->event != base) {
			report("Event received on a window other than the parent");
			FAIL;
		} else {
			got_parent++;
		}
		good.type = UnmapNotify;
		good.serial = 0L;
		good.send_event = False;
		good.display = display;
		good.event = ump->event;
		good.window = w;
		good.from_configure = False;

		if (checkevent((XEvent*)&good, &ev))
			FAIL;
		else
			CHECK;
	}

	if (got_window > 1) {
		report("Repeat unmap notify event on window");
		FAIL;
	} else if (got_window == 0) {
		report("UnmapNotify event was not received on window");
		FAIL;
	} else
		CHECK;

	if (got_parent > 1) {
		report("Repeat unmap notify event on parent window");
		FAIL;
	} else if (got_parent == 0) {
		report("UnmapNotify event was not received on parent window");
		FAIL;
	} else
		CHECK;

	XGetWindowAttributes(display, w, &atts);
	if (atts.map_state != IsUnmapped) {
		report("After unmap map-state was %s, expecting IsUnmapped",
			mapstatename(atts.map_state));
		FAIL;
	} else
		CHECK;

	if (checkclear(display, base))
		CHECK;
	else {
		report("Window did not disappear after unmapping");
		FAIL;
	}

	CHECKPASS(1+2*2+4);
>>ASSERTION Good A
When the specified window is already unmapped, then a call to xname
has no effect.
>>STRATEGY
Create parent window.
Create child window.
Select StructureNotify on child.
Call xname to unmap child window.
Verify that no UnmapNotify event is received on window.
>>CODE
Window	base;
struct	area	area;
XWindowAttributes	atts;
int 	n;

	base = defwin(display);
	setarea(&area, 10, 10, 50, 50);
	w = creunmapchild(display, base, &area);
	pattern(display, w);

	XSelectInput(display, w, StructureNotifyMask);

	XCALL;

	XSelectInput(display, w, NoEventMask);

	n = XPending(display);
	if (n != 0) {
		report("Received event when window already unmapped");
		FAIL;
	} else
		CHECK;

	XGetWindowAttributes(display, w, &atts);
	if (atts.map_state != IsUnmapped) {
		report("After unmap map-state was %s, expecting IsUnmapped",
			mapstatename(atts.map_state));
		FAIL;
	} else
		CHECK;

	CHECKPASS(2);
>>ASSERTION Good A
When there is a viewable child of
.A window ,
then after a call to xname the child window is no longer viewable.
>>STRATEGY
Create base window.
Create parent window as inferior of base.
Create child window as inferior of parent.
Check it is Viewable.
Unmap parent window.
Verify that map-state of child window is IsUnviewable.
>>CODE
Window	base;
Window	ch;
struct	area	area;
XWindowAttributes	atts;

	base = defwin(display);
	setarea(&area, 10, 10, 50, 50);
	w = crechild(display, base, &area);
	pattern(display, w);

	setarea(&area, 5, 5, 10, 10);
	ch = crechild(display, w, &area);

	(void) XGetWindowAttributes(display, ch, &atts);
	if (atts.map_state != IsViewable) {
		delete("Before unmapping parent map-state of child was %s, expecting IsViewable", mapstatename(atts.map_state));
		return;
	} else
		CHECK;

	XCALL;

	(void) XGetWindowAttributes(display, ch, &atts);
	if (atts.map_state != IsUnviewable) {
		report("After unmap map-state was %s, expecting IsUnviewable",
			mapstatename(atts.map_state));
		FAIL;
	} else
		CHECK;

	if (checkclear(display, base))
		CHECK;
	else {
		report("Window did not disappear after unmapping");
		FAIL;
	}

	CHECKPASS(3);
>>ASSERTION Good A
When a call to xname
uncovers part of any window that was formerly obscured, then
either
.S Expose
events are generated or the contents are restored from backing store.
>>STRATEGY
Create base window.
Call setforexpose() on base window.
Create child window to unmap.
Call xname to unmap window.
Verify expose or backing store restore occurred with exposecheck().
>>CODE
Window	base;
struct	area	area;
XWindowAttributes	atts;

	base = defwin(display);

	setarea(&area, 10, 10, 50, 50);
	w = crechild(display, base, &area);
	pattern(display, w);

	setforexpose(display, base);
	XSelectInput(display, base, ExposureMask);

	XCALL;

	XGetWindowAttributes(display, w, &atts);
	if (atts.map_state != IsUnmapped) {
		report("After unmap map-state was %s, expecting IsUnmapped",
			mapstatename(atts.map_state));
		FAIL;
	} else
		CHECK;

	if (exposecheck(display, base))
		CHECK;
	else {
		report("Neither Expose events or backing store processing");
		report("could correctly restore the window contents.");
		FAIL;
	}

	CHECKPASS(2);
>>ASSERTION Bad A
.ER BadWindow

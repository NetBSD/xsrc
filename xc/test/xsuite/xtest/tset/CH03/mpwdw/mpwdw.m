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
 * $XConsortium: mpwdw.m,v 1.10 94/04/17 21:03:11 rws Exp $
 */
>>TITLE XMapWindow CH03
void

Display	*display = Dsp;
Window	w;
>>EXTERN

static char	*T1[] = {
	".",
	"A . (10, 10) 70x70 unmap",
	  "A1 A (20, 2) 20x5",
		"A1a A1 (10,1) 5x3",
	  "B A (10, 10) 50x50 unmap",
		"C B (10, 10) 30x30 unmap",
};

>>ASSERTION Good A
A call to xname maps the specified window.
>>STRATEGY
Create unmapped window.
Set window background to W_FG.
Map window.
Verify map state is IsViewable.
Verify that window shows on screen.
>>CODE
Window	base;
XWindowAttributes	atts;
struct	area	area;

	base = defwin(display);

	setarea(&area, 10, 10, 30, 30);
	w = creunmapchild(display, base, &area);
	XSetWindowBackground(display, w, W_FG);

	XCALL;

	(void) XGetWindowAttributes(display, w, &atts);
	if (atts.map_state != IsViewable) {
		report("map state after map was %s, expecting %s",
			mapstatename(atts.map_state), mapstatename(IsViewable));
		FAIL;
	} else
		CHECK;

	if (checkarea(display, base, &area, W_FG, W_BG, CHECK_ALL))
		CHECK;
	else {
		report("Mapped window was displayed incorrectly or was not seen on screen");
		FAIL;
	}

	CHECKPASS(2);

>>ASSERTION def
When a call to xname is made on a window that has no unmapped
ancestors, then the window becomes viewable.
>>ASSERTION Good A
When a call to xname is made on a window that has an unmapped ancestor,
then the window does not become viewable.
>>STRATEGY
Create stack of unmapped windows.
Map one of these window that has an unmapped ancestor.
Verify map state is IsUnviewable.
Verify that nothing becomes visible.
>>CODE
Window	base;
struct	buildtree	*btp;
XWindowAttributes	atts;

	base = defwin(display);
	btp = buildtree(display, base, T1, NELEM(T1));

	/* We shall map window B which has A as an unmapped ancestor */
	w = btntow(btp, "B");

	XCALL;

	(void) XGetWindowAttributes(display, w, &atts);
	if (atts.map_state != IsUnviewable) {
		report("map state after map was %s, expecting %s",
			mapstatename(atts.map_state), mapstatename(IsUnviewable));
		FAIL;
	} else
		CHECK;

	if (checkarea(display, base, (struct area *)0, W_BG, W_BG, CHECK_ALL))
		CHECK;
	else {
		report("Somthing became visible after mapping window with unmapped ancestors");
		FAIL;
	}

	CHECKPASS(2);

>>ASSERTION Good A
When a call to xname is made on a window that is the only unmapped ancestor
of an inferior window that has previously been mapped,
then that inferior window becomes viewable.
>>STRATEGY
Create stack of windows with mapped windows as inferiors of an unmapped window.
Check that map-state of inferiors is IsUnviewable.
Map the ancestor window.
Verify that map-state of inferiors is IsViewable.
Verify that windows become visible on the screen.
>>CODE
Window	base;
Window	ch1;
Window	ch2;
struct	buildtree	*btp;
XWindowAttributes	atts;

	base = defwin(display);
	btp = buildtree(display, base, T1, NELEM(T1));

	ch1 = btntow(btp, "A1");
	ch2 = btntow(btp, "A1a");

	XGetWindowAttributes(display, ch1, &atts);
	if (atts.map_state != IsUnviewable) {
		report("map state for A1 was %s, expecting %s",
			mapstatename(atts.map_state), mapstatename(IsUnviewable));
		FAIL;
	} else
		CHECK;

	XGetWindowAttributes(display, ch2, &atts);
	if (atts.map_state != IsUnviewable) {
		report("map state for A1a was %s, expecting %s",
			mapstatename(atts.map_state), mapstatename(IsUnviewable));
		FAIL;
	} else
		CHECK;

	w = btntow(btp, "A");

	XCALL;

	XGetWindowAttributes(display, ch1, &atts);
	if (atts.map_state != IsViewable) {
		report("map state for A1 was %s, expecting %s",
			mapstatename(atts.map_state), mapstatename(IsViewable));
		FAIL;
	} else
		CHECK;

	XGetWindowAttributes(display, ch2, &atts);
	if (atts.map_state != IsViewable) {
		report("map state for A1a was %s, expecting %s",
			mapstatename(atts.map_state), mapstatename(IsViewable));
		FAIL;
	} else
		CHECK;

	PIXCHECK(display, base);

	CHECKPASS(5);
>>ASSERTION Good A
When the window is already mapped,
then a call to xname has no effect.
>>STRATEGY
Create window.
Map window.
Enable events.
Attempt to map window again.
Verify that no difference occurred on screen.
Verify that map-state is still IsViewable.
Verify that no events are generated.
>>CODE
Window	base;
struct	area	area;
XWindowAttributes	atts;
XImage	*imp;

	base = defwin(display);
	setarea(&area, 10, 10, 40, 40);
	w = creunmapchild(display, base, &area);
	XSetWindowBackground(display, w, W_FG);

	XCALL;

	imp = savimage(display, w);

	XSelectInput(display, w, ALLEVENTS);

	XCALL;

	XSelectInput(display, w, NoEventMask);

	if (compsavimage(display, w, imp))
		CHECK;
	else {
		report("Difference occurred on screen after re-mapping window");
		FAIL;
	}

	(void) XGetWindowAttributes(display, w, &atts);
	if (atts.map_state != IsViewable) {
		report("map state after map was %s, expecting %s",
			mapstatename(atts.map_state), mapstatename(IsViewable));
		FAIL;
	} else
		CHECK;

	if (XPending(display) != 0) {
		report("Re-mapping window produced event");
		FAIL;
	} else
		CHECK;

	CHECKPASS(3);

>>ASSERTION Good A
When the
.M override_redirect
attribute of the window is
.S False
and some other client has selected
.S SubstructureRedirectMask
on the parent window, then a
.S MapRequest
event is generated, and the call to xname does not map the window.
>>STRATEGY
Create base window.
Create unmapped window on base window.
Set override-redirect attribute to False.
Create second client.
Select SubstructureRedirect for second client on base window.
Map window.
Verify that the MapRequest event is generated on second client.
Verify that map-state is still IsUnmapped.
>>CODE
Display	*client2;
Window	base;
XSetWindowAttributes	setatts;
XWindowAttributes	atts;
XMapRequestEvent	good;
XEvent	ev;
struct	area	area;
int 	n;

	base = defwin(display);
	setarea(&area, 20, 20, 40, 40);
	w = creunmapchild(display, base, &area);
	setatts.override_redirect = False;
	XChangeWindowAttributes(display, w, CWOverrideRedirect, &setatts);
	if (isdeleted())
		return;

	client2 = opendisplay();
	XSelectInput(client2, base, SubstructureRedirectMask);
	XSync(client2, True);

	XCALL;
	XSync(client2, False);

	good.type = MapRequest;
	good.serial = 0;
	good.send_event = False;
	good.display = client2;
	good.parent = base;
	good.window = w;

	n = getevent(client2, &ev);
	if (n != 1) {
		report("Expecting one event on second client, received %d", n);
		FAIL;
	} else if (checkevent((XEvent *)&good, &ev))
		FAIL;
	else
		CHECK;

	(void) XGetWindowAttributes(display, w, &atts);
	if (atts.map_state != IsUnmapped) {
		report("map state after map was %s, expecting %s",
			mapstatename(atts.map_state), mapstatename(IsUnmapped));
		FAIL;
	} else
		CHECK;

	CHECKPASS(2);
>>ASSERTION Good A
When the override-redirect attribute of the window is
.S True ,
then on a call to xname the window is mapped and a
.S MapNotify
event is generated.
>>STRATEGY
Create base window.
Create unmapped window on base window.
Set override-redirect attribute to True.
Select StructureNotify events on window.
Create second client.
Select SubstructureRedirect and SubstructureNotify for second client on base window.
Map window.
Verify that a MapNotify event is generated on parent.
Verify that a MapNotify event is generated on window.
Verify that the map_state of the window is IsViewable.
>>CODE
Display	*client2;
Window	base;
XSetWindowAttributes	setatts;
XWindowAttributes	atts;
XMapEvent	good;
XEvent	ev;
struct	area	area;
int 	n;

	base = defwin(display);
	setarea(&area, 20, 20, 40, 40);
	w = creunmapchild(display, base, &area);
	setatts.override_redirect = True;
	XChangeWindowAttributes(display, w, CWOverrideRedirect, &setatts);
	/*
	 * Would normally select ALLEVENTS to test for spurious events.
	 * This cannot be done here, since we would possibly get valid 
	 * EnterNotify's etc.
	 */
	XSelectInput(display, w, StructureNotifyMask);
	if (isdeleted())
		return;

	client2 = opendisplay();
	XSelectInput(client2, base,SubstructureRedirectMask|SubstructureNotifyMask);
	XSync(client2, True);

	XCALL;
	XSync(client2, False);

	/*
	 * Check for Map event on parent.
	 */
	good.type = MapNotify;
	good.serial = 0;
	good.send_event = False;
	good.display = client2;
	good.event = base;
	good.window = w;
	good.override_redirect = True;

	n = getevent(client2, &ev);
	if (n != 1) {
		report("Expecting one event on second client, received %d", n);
		FAIL;
	} else if (checkevent((XEvent*)&good, &ev))
		FAIL;
	else
		CHECK;

	/*
	 * Now check for map notify on the window itself.
	 */
	good.display = display;
	good.event = w;

	n = getevent(display, &ev);
	if (n != 1) {
		report("Expecting one Map notify event, received %d events", n);
		FAIL;
	} else if (checkevent((XEvent*)&good, &ev))
		FAIL;
	else
		CHECK;

	(void) XGetWindowAttributes(display, w, &atts);
	if (atts.map_state != IsViewable) {
		report("map state after map was %s, expecting %s",
			mapstatename(atts.map_state), mapstatename(IsViewable));
		FAIL;
	} else
		CHECK;

	CHECKPASS(3);
>>ASSERTION Good A
When no other client has selected
.S SubstructureRedirectMask
on the parent window, then on a call to xname the window is
mapped and a
.S MapNotify
event is generated.
>>STRATEGY
Create base window.
Create unmapped window on base window.
Set override-redirect attribute to False.
Select StructureNotify events on window.
Create second client.
Select SubstructureNotify for second client on base window.
Map window.
Verify that a MapNotify event is generated on parent.
Verify that a MapNotify event is generated on window.
Verify that the map_state of the window is IsViewable.
>>CODE
Display	*client2;
Window	base;
XSetWindowAttributes	setatts;
XWindowAttributes	atts;
XMapEvent	good;
XEvent	ev;
struct	area	area;
int 	n;

	base = defwin(display);
	setarea(&area, 20, 20, 40, 40);
	w = creunmapchild(display, base, &area);
	setatts.override_redirect = False;
	XChangeWindowAttributes(display, w, CWOverrideRedirect, &setatts);
	/*
	 * Would normally select ALLEVENTS to test for spurious events.
	 * This cannot be done here, since we would possibly get valid 
	 * EnterNotify's etc.
	 */
	XSelectInput(display, w, StructureNotifyMask);
	if (isdeleted())
		return;

	client2 = opendisplay();
	XSelectInput(client2, base, SubstructureNotifyMask);
	XSync(client2, True);

	XCALL;
	XSync(client2, False);

	/*
	 * Check for Map event on parent.
	 */
	good.type = MapNotify;
	good.serial = 0;
	good.send_event = False;
	good.display = client2;
	good.event = base;
	good.window = w;
	good.override_redirect = False;

	n = getevent(client2, &ev);
	if (n != 1) {
		report("Expecting one event on second client, received %d", n);
		FAIL;
	} else if (checkevent((XEvent*)&good, &ev))
		FAIL;
	else
		CHECK;

	/*
	 * Now check for map notify on the window itself.
	 */
	good.display = display;
	good.event = w;

	n = getevent(display, &ev);
	if (n != 1) {
		report("Expecting one Map notify event, received %d events", n);
		FAIL;
	} else if (checkevent((XEvent*)&good, &ev))
		FAIL;
	else
		CHECK;

	(void) XGetWindowAttributes(display, w, &atts);
	if (atts.map_state != IsViewable) {
		report("map state after map was %s, expecting %s",
			mapstatename(atts.map_state), mapstatename(IsViewable));
		FAIL;
	} else
		CHECK;

	CHECKPASS(3);
>>ASSERTION Good A
When the window or one of its inferiors becomes viewable after a call to xname
and no earlier contents for it are remembered, then
the window is tiled with its background.
>>STRATEGY
>># Have already tested this with a plain background, but take the
>># opportunity to test with a background tile.
Create window.
Set background to tile.
Map window.
Verify that window is tiled.
Re-run with an inferior window.
>>CODE
Window	base;
Window	win;
Pixmap	tile;

	base = defwin(display);

	w = creunmapchild(display, base, (struct area *)0);

	tile = maketile(display, w);
	XSetWindowBackgroundPixmap(display, w, tile);

	XCALL;

	if (checktile(display, w, (struct area *)0, 0, 0, tile))
		CHECK;
	else {
		report("Window not tiled with its background correctly");
		FAIL;
	}

	/*
	 * Create an unmapped window and a mapped inferior of that window.
	 * When we map the window the inferior becomes viewable and so it
	 * should be tiled.
	 */
	base = defwin(display);
	w = creunmapchild(display, base, (struct area *)0);
	win = crechild(display, w, (struct area *)0);
	XSetWindowBackgroundPixmap(display, win, tile);

	XCALL;

	if (checktile(display, win, (struct area *)0, 0, 0, tile))
		CHECK;
	else {
		report("Inferior window not tiled with its background correctly");
		FAIL;
	}

	CHECKPASS(2);
>>ASSERTION Good A
>># This is inconsistant with other stuff? In create.
>># (I think that if the depth is different from the parent, then the
>># screen contents are undefined. From ANSI. ..sr)
When the background of the window is undefined then the existing screen
contents from the parent or an inferior of the parent are not altered,
and zero or more
.S Expose
events are generated.
>>STRATEGY
Create window as parent.
Draw pattern on parent.
Create inferior window and draw pattern on that too.
Create unmapped inferior that overlaps other inferior.
Set background pixmap to None to undefine background.
Map window.
Verify that screen contents have not been altered.

Unmap window.
Select for expose events.
Call setforexpose() to draw into window (only effective if backing store in use)
Call xname to map window.
Verify that window was restored from backing store or received expose
events to cover the window with exposecheck().
>>CODE
Window	base;
Window	ch1;
XImage	*imp;
struct	area	area;

	base = defwin(display);
	pattern(display, base);

	setarea(&area, 7, 7, 50, 50);
	ch1 = crechild(display, base, &area);
	pattern(display, ch1);

	/* Save original contents */
	imp = savimage(display, base);

	setarea(&area, 5, 5, 70, 40);
	w = creunmapchild(display, base, &area);
	XSetWindowBackgroundPixmap(display, w, None);

	XCALL;

	if (compsavimage(display, base, imp))
		CHECK;
	else {
		report("Screen contents altered by mapping window with undefined background");
		FAIL;
	}

	/*
	 * Check expose events.  Unmap the window and draw into it with
	 * setforexpose(), (this will normally have no effect unless backing
	 * store is active).  Map the window and check that either
	 *  - there are zero expose events and backing store has preserved the
	 *    contents.
	 *  - there are enough expose events to cover the window.
	 */
	XUnmapWindow(display, w);

	XSelectInput(display, w, ExposureMask);
	setforexpose(display, w);
	/* because setforexpose() changes the background */
	XSetWindowBackgroundPixmap(display, w, None);

	XCALL;

	if (exposecheck(display, w))
		CHECK;
	else {
		report("Neither Expose events or backing store processing");
		report("could correctly restore the window contents.");
		FAIL;
	}

	CHECKPASS(2);
>>ASSERTION Good B 3
When the background of the window is undefined then the existing screen
contents not from the parent or an inferior of the parent are undefined,
and zero or more
.S Expose
events are generated.
>>ASSERTION Good D 3
If backing store is supported:
When backing store has been maintained while a particular
window was unmapped, then no
.S Expose
events are generated for that window.
>>STRATEGY
If backing store is supported
  Create window.
  Set backing store hint to Always.
  Write into window.
  Unmap window.
  Enable Expose events.
  Map window.
  If window contents are unaltered.
	Backing store was maintained.
	Verify that no Expose events were received.
  else
	Backing store was not maintained.
	Test result is UNTESTED.
else
  Test result is UNSUPPORTED.
>>CODE
XSetWindowAttributes	setatts;
XEvent	ev;

	if (DoesBackingStore(DefaultScreenOfDisplay(display)) != Always) {
		unsupported("Backing store not supported while unmapped");
		return;
	}

	w = defwin(display);
	setatts.backing_store = Always;
	XChangeWindowAttributes(display, w, CWBackingStore, &setatts);

	dset(display, w, W_FG);
	XUnmapWindow(display, w);
	XSelectInput(display, w, ExposureMask);

	XCALL;

	XSelectInput(display, w, NoEventMask);

	if (checkarea(display, w, (struct area *)0, W_FG, W_FG, CHECK_ALL|CHECK_DIFFER)) {
		/*
		 * Backing store apeared to be active during the unmap.
		 */
		if (XCheckTypedWindowEvent(display, w, Expose, &ev)) {
			/* Received Exposes */
			report("Expose received when backing store was active");
			FAIL;
		} else
			CHECK;
	} else {
		/*
		 * There is nothing wrong here, but it is possibly of interest.
		 */
		trace("NOTE: Server claims to support backing store but it was not active in the test");
		UNTESTED;
	}

	CHECKPASS(1);
>>ASSERTION Good D 3
If backing store is supported:
When the server elects to now maintain backing store for a window,
then an
.S Expose
event for the whole window is generated.
>>STRATEGY
If backing store is supported.
  Create small window.
  Ask for backing store when mapped.
  Create (and map) windows that would obscure the test window.
  Enable Expose events.
  Map window.
  If no Expose
	Untested - maybe got backing store always.
  Verify Expose event is for whole window.
else
  Untested.
>>CODE
int 	n;
Window	base;
Window	ch1;
XEvent	ev;
struct	area	area;
struct	area	charea;
XExposeEvent	*ep;
XSetWindowAttributes	setatts;

	if (DoesBackingStore(DefaultScreenOfDisplay(display)) == NotUseful) {
		unsupported("Backing store is not supported");
		return;
	}

	base = defwin(display);

	setarea(&area, 0, 0, 70, 70);
	w = creunmapchild(display, base, &area);
	setatts.backing_store = WhenMapped;
	XChangeWindowAttributes(display, w, CWBackingStore, &setatts);

	/* Create a child that will cover the test window */
	setarea(&charea, 0, 0, 20, 20);
	ch1 = crechild(display, base, &charea);

	XSelectInput(display, w, ExposureMask);
	XCALL;
	XSelectInput(display, w, NoEventMask);

	n = XPending(display);
	if (n == 0) {
		trace("No Expose events on mapping window with backing-store WhenMapped");
		UNTESTED;
	} else
		CHECK;

	if (n == 1) {
		(void) getevent(display, &ev);
		if (ev.type != Expose) {
			/* Only Expose events were enabled */
			delete("Unexpected event received (%s)", eventname(ev.type));
		} else {
			ep = (XExposeEvent*)&ev;
			if (ep->x == area.x && ep->y == area.y && ep->width == area.width && ep->height == area.height) {
				/* Full window Expose received */
				CHECK;
			} else {
				/*
				 * Since there was only one event and it does not cover
				 * the whole window then it is incorrect.
				 */
				report("Expose event covered wrong area");
				report("x=%d, y=%d, width=%d, height=%d", ep->x, ep->y, ep->width, ep->height);
				FAIL;
			}
		}
	} else {
		trace("Did not appear to get backing store");
		UNTESTED;
	}
	CHECKPASS(2);

>>ASSERTION Good B 3
When the server elects to not maintain backing store for a
window, then
.S Expose
events are generated only for visible regions.
>># NOTE: 'are' reported is from ANSI.  Spec says 'may be' reported.
>># stick with 'are'.	kieron
>>STRATEGY
Create small window.
Ask for no backing store.
Create (and map) windows that would obscure the test window.
Map window.
If Expose event is for whole window.
  Untested - maybe got backing store.
else
  Verify that only visible regions got Expose events.
>>CODE
int 	n;
Window	base;
Window	ch1;
struct	area	area;
struct	area	charea;
XSetWindowAttributes	setatts;

	base = defwin(display);

	setarea(&area, 0, 0, 70, 70);
	w = creunmapchild(display, base, &area);
	setatts.backing_store = NotUseful;
	XChangeWindowAttributes(display, w, CWBackingStore, &setatts);

	/* Create a child that will cover the test window */
	setarea(&charea, 0, 0, 20, 20);
	ch1 = crechild(display, base, &charea);

	XSelectInput(display, w, ExposureMask);
	XCALL;
	XSelectInput(display, w, NoEventMask);

	n = XPending(display);
	if (n == 0) {
		trace("No Expose events on mapping window with backing-store NotUseful");
		UNTESTED;
	} else
		CHECK;

	if (n == 1) {
		trace("Appeared to get backing store when asked for NotUseful");
		UNTESTED;
	} else {
		/*
		 * Remove the child window.  Clear the test window, and then
		 * redraw it according to the Expose events.  This should leave
		 * the part that was not visible after the map in the background
		 * pixel.
		 */
		XUnmapWindow(display, ch1);
		dclear(display, w);
		exposefill(display, w);
		if (checkarea(display, w, &charea, W_BG, W_FG, CHECK_ALL))
			CHECK;
		else {
			report("Some part of non-visible regions received Expose events");
			report("  or not all visible regions exposed");
			FAIL;
		}
	}

	CHECKPASS(2);

>>ASSERTION Bad A
.ER BadWindow

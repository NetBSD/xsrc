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
 * $XConsortium: mpsbws.m,v 1.11 94/04/17 21:03:07 rws Exp $
 */
>>TITLE XMapSubwindows CH03
void

Display	*display = Dsp;
Window	w;
>>EXTERN

>>#	  "A1 A (15, 5) 5x5 unmap",
>>#	  "A2 A (3, 15) 5x5 unmap",
>>#	  "E1 E (10,10) 20x20 unmap",
>>#	  "E1a E1 (10,10) 5x5 unmap",

#define	WINDOW_NOTIFY	001
#define	PARENT_NOTIFY	002

char	*T1[] = {
	".",
	"A . (10,10) 40x40 unmap",
	"B . (3,40) 40x40 unmap",
	"C . (20,22) 40x40 unmap",
	"D . (45,23) 40x40 unmap",
	"E . (30,30) 40x40 unmap",
};

>>ASSERTION Good A
A call to xname maps all subwindows of the specified window in
top to bottom stacking order.
>>STRATEGY
Create unmapped subwindows of a window.
Map subwindows.
Verify map state of each subwindow is IsViewable.
Verify that windows show on screen.
>>CODE
XWindowAttributes	atts;
struct	buildtree	*btlist;
int 	i;

	w = defwin(display);
	btlist = buildtree(display, w, T1, NELEM(T1));

	XCALL;

	for (i = 1; i < NELEM(T1); i++) {
		(void) XGetWindowAttributes(display, btlist[i].wid, &atts);
		if (atts.map_state != IsViewable) {
			report("map state after map was %s, expecting %s",
				mapstatename(atts.map_state), mapstatename(IsViewable));
			report("  Window name was %s", btlist[i].name);
			FAIL;
		} else
			CHECK;
	}

	PIXCHECK(display, w);

	CHECKPASS(1+NELEM(T1)-1);

>>ASSERTION def
When a call to xname is made on a window that is mapped and
has no unmapped ancestors, then the
subwindows become viewable.
>>ASSERTION Good A
When a call to xname is made on a window that is unmapped or
has an unmapped ancester,
then the subwindows do not become viewable.
>>STRATEGY
Create stack of unmapped windows.
Call xname on one of these window that has an unmapped ancestor.
Verify map state of each subwindow is IsUnviewable.
Verify that nothing becomes visible.
>>CODE
Window	base;
struct	buildtree	*btp;
XWindowAttributes	atts;
int 	i;

	base = defwin(display);

	/* Create an unmapped test window */
	w = creunmapchild(display, base, (struct area *)0);
	btp = buildtree(display, w, T1, NELEM(T1));

	XCALL;

	for (i = 1; i < NELEM(T1); i++) {
		(void) XGetWindowAttributes(display, btp[i].wid, &atts);
		if (atts.map_state != IsUnviewable) {
			report("map state after map was %s, expecting %s",
				mapstatename(atts.map_state), mapstatename(IsUnviewable));
			FAIL;
		} else
			CHECK;
	}

	if (checkarea(display, base, (struct area *)0, W_BG, W_BG, CHECK_ALL))
		CHECK;
	else {
		report("Somthing became visible after mapping subwindows with unmapped ancestors");
		FAIL;
	}

	CHECKPASS(1+NELEM(T1)-1);

>>ASSERTION Good A
When all the subwindows are already mapped,
then a call to xname has no effect.
>>STRATEGY
Create subwindows.
Map subwindow.
Enable events.
Attempt to map subwindows again.
Verify that no difference occurred on screen.
Verify that map-state is still IsViewable.
Verify that no events are generated.
>>CODE
Window	base;
XWindowAttributes	atts;
struct	buildtree	*bt;
XImage	*imp;
int 	i;

	base = defwin(display);
	bt = buildtree(display, base, T1, NELEM(T1));

	w = base;
	XCALL;

	imp = savimage(display, base);

	for (i = 0; i < NELEM(T1); i++)
		XSelectInput(display, bt[i].wid, ALLEVENTS);

	XCALL;

	for (i = 0; i < NELEM(T1); i++)
		XSelectInput(display, bt[i].wid, NoEventMask);

	if (compsavimage(display, base, imp))
		CHECK;
	else {
		report("Difference occurred on screen after re-mapping subwindows");
		FAIL;
	}

	for (i = 1; i < NELEM(T1); i++) {
		(void) XGetWindowAttributes(display, bt[i].wid, &atts);
		if (atts.map_state != IsViewable) {
			report("map state after map was %s, expecting %s",
				mapstatename(atts.map_state), mapstatename(IsViewable));
			report("  window name %s", bt[i].name);
			FAIL;
		} else
			CHECK;
	}

	if (XPending(display) != 0) {
		report("Re-mapping subwindow produced event");
		FAIL;
	} else
		CHECK;

	CHECKPASS(2+NELEM(T1)-1);

>>ASSERTION Good A
When the
.M override_redirect
attribute of a subwindow is
.S False 
and some other client has selected
.S SubstructureRedirectMask 
on its parent window, then a
.S MapRequest 
event is generated, and the call to xname does not map the subwindow.
>>STRATEGY
Create base window.
Create unmapped subwindows on base window.
Set override-redirect attribute to False on each subwindow.
Create second client.
Select SubstructureRedirectMask for second client on base window.
Call xname on base window.
Verify that the MapRequest event is generated for each subwindow.
Verify that map-state is still IsUnmapped.
>>CODE
Display	*client2;
XSetWindowAttributes	setatts;
XWindowAttributes	atts;
XMapRequestEvent	good;
XMapRequestEvent	*mrp;
struct	buildtree	*bt;
struct	buildtree	*btp;
XEvent	ev;
int 	i;

	w = defwin(display);
	bt = buildtree(display, w, T1, NELEM(T1));

	for (i = 1; i < NELEM(T1); i++) {
		setatts.override_redirect = False;
		XChangeWindowAttributes(display, bt[i].wid, CWOverrideRedirect, &setatts);
	}

	if (isdeleted())
		return;

	client2 = opendisplay();
	XSelectInput(client2, w, SubstructureRedirectMask);
	XSync(client2, True);

	XCALL;
	XSync(client2, False);

	good.type = MapRequest;
	good.serial = 0;
	good.send_event = False;
	good.display = client2;
	good.parent = w;

	/* Should go through this loop NELEM(T1)-1 times */
	while (getevent(client2, &ev)) {

		mrp = (XMapRequestEvent*)&ev;
		btp = btwtobtp(bt, mrp->window);
		if (btp == NULL) {
			report("Event received for unknown window");
			FAIL;
			continue;
		}
		if (mrp->parent != w) {
			report("Map request received on a window other than the parent");
			FAIL;
		} else
			CHECK;

		btp->uflags++;
		good.window = btp->wid;

		if (checkevent((XEvent *)&good, &ev)) {
			report("Bad event on window '%s'", btp->name);
			FAIL;
		} else
			CHECK;
	}

	for (i = 1; i < NELEM(T1); i++) {

		if (bt[i].uflags == 0) {
			report("No map request for window '%s'", bt[i].name);
			FAIL;
		} else if (bt[i].uflags > 1) {
			report("Repeat map request for window '%s'", bt[i].name);
			FAIL;
		} else
			CHECK;

		(void) XGetWindowAttributes(display, bt[i].wid, &atts);
		if (atts.map_state != IsUnmapped) {
			report("map state after map was %s, expecting %s",
				mapstatename(atts.map_state), mapstatename(IsUnmapped));
			FAIL;
		} else
			CHECK;
	}

	CHECKPASS(4*(NELEM(T1)-1));
>>ASSERTION Good A
When the
.M override-redirect
attribute of a subwindow is
.S True
and the subwindow is not already mapped,
then the window is mapped and a
.S MapNotify 
event is generated.
>>STRATEGY
Create base window.
Create unmapped subwindows on base window.
Set override-redirect attribute to True on each subwindow.
Select StructureNotify events on each subwindow.
Create second client.
Select SubstructureRedirect and SubstructureNotify for second client on base window.
Call xname on base window.
Verify that a MapNotify event is generated on parent for each subwindow.
Verify that a MapNotify event is generated on each subwindow.
Verify that the map_state of each subwindow is IsViewable.
>>CODE
Display	*client2;
Window	base;
XSetWindowAttributes	setatts;
XWindowAttributes	atts;
XMapEvent	good;
XMapEvent	*mp;
struct	buildtree	*bt;
struct	buildtree	*btp;
XEvent	ev;
int 	i;

	base = defwin(display);
	bt = buildtree(display, base, T1, NELEM(T1));

	w = base;

	for (i = 1; i < NELEM(T1); i++) {
		setatts.override_redirect = True;
		XChangeWindowAttributes(display, bt[i].wid, CWOverrideRedirect, &setatts);
		/*
		 * Would normally select ALLEVENTS to test for spurious events.
		 * This cannot be done here, since we would possibly get valid 
		 * EnterNotify's etc.
		 */
		XSelectInput(display, bt[i].wid, StructureNotifyMask);
	}

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

	/* Should go through this loop NELEM(T1)-1 times */
	while (getevent(client2, &ev)) {

		mp = (XMapEvent*)&ev;
		btp = btwtobtp(bt, mp->window);
		if (btp == NULL) {
			report("Event received for unknown window");
			FAIL;
			continue;
		}
		if (mp->event != base) {
			report("Map request received on a window other than the parent");
			FAIL;
		} else
			CHECK;

		if (btp->uflags & PARENT_NOTIFY) {
			report("Repeat event received on parent for window '%s'", btp->name);
			FAIL;
		} else {
			btp->uflags |= PARENT_NOTIFY;
			CHECK;
		}

		good.window = btp->wid;

		if (checkevent((XEvent *)&good, &ev)) {
			report("Bad event on window '%s'", btp->name);
			FAIL;
		} else
			CHECK;
	}

	/*
	 * Now check for map notify on the window itself.
	 */
	good.display = display;

	/* Should go through this loop NELEM(T1)-1 times */
	while (getevent(display, &ev)) {

		mp = (XMapEvent*)&ev;
		btp = btwtobtp(bt, mp->window);
		if (btp == NULL) {
			report("Event received for unknown window");
			FAIL;
			continue;
		}

		if (btp->uflags & WINDOW_NOTIFY) {
			report("Repeat map notify event received on window '%s'", btp->name);
			FAIL;
		} else {
			btp->uflags |= WINDOW_NOTIFY;
			CHECK;
		}

		good.window = btp->wid;
		good.event = btp->wid;

		if (checkevent((XEvent *)&good, &ev)) {
			report("Bad event on window '%s'", btp->name);
			FAIL;
		} else
			CHECK;
	}

	for (i = 1; i < NELEM(T1); i++) {

		if (bt[i].uflags & WINDOW_NOTIFY)
			CHECK;
		else {
			report("No map notify received on window '%s'", bt[i].name);
			FAIL;
		}
		if (bt[i].uflags & PARENT_NOTIFY)
			CHECK;
		else {
			report("No map notify received on parent of window '%s'", bt[i].name);
			FAIL;
		}
		(void) XGetWindowAttributes(display, w, &atts);
		if (atts.map_state != IsViewable) {
			report("map state after map was %s, expecting %s",
				mapstatename(atts.map_state), mapstatename(IsViewable));
			FAIL;
		} else
			CHECK;
	}

	CHECKPASS(8*(NELEM(T1)-1));
>>ASSERTION Good A
When no other client has selected
.S SubstructureRedirectMask
on the parent window and the window is not already mapped,
then the window is mapped and a
.S MapNotify
event is generated.
>>STRATEGY
Create base window.
Create unmapped subwindows on base window.
Set override-redirect attribute to False on each subwindow.
Select StructureNotify events on each subwindow.
Create second client.
Select SubstructureNotify for second client on base window.
Call xname on base window.
Verify that a MapNotify event is generated on parent for each subwindow.
Verify that a MapNotify event is generated on each subwindow.
Verify that the map_state of each subwindow is IsViewable.
>>CODE
Display	*client2;
Window	base;
XSetWindowAttributes	setatts;
XWindowAttributes	atts;
XMapEvent	good;
XMapEvent	*mp;
struct	buildtree	*bt;
struct	buildtree	*btp;
XEvent	ev;
int 	i;

	base = defwin(display);
	bt = buildtree(display, base, T1, NELEM(T1));

	w = base;

	for (i = 1; i < NELEM(T1); i++) {
		setatts.override_redirect = False;
		XChangeWindowAttributes(display, bt[i].wid, CWOverrideRedirect, &setatts);
		/*
		 * Would normally select ALLEVENTS to test for spurious events.
		 * This cannot be done here, since we would possibly get valid 
		 * EnterNotify's etc.
		 */
		XSelectInput(display, bt[i].wid, StructureNotifyMask);
	}

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

	/* Should go through this loop NELEM(T1)-1 times */
	while (getevent(client2, &ev)) {

		mp = (XMapEvent*)&ev;
		btp = btwtobtp(bt, mp->window);
		if (btp == NULL) {
			report("Event received for unknown window");
			FAIL;
			continue;
		}
		if (mp->event != base) {
			report("Map request received on a window other than the parent");
			FAIL;
		} else
			CHECK;

		if (btp->uflags & PARENT_NOTIFY) {
			report("Repeat event received on parent for window '%s'", btp->name);
			FAIL;
		} else {
			btp->uflags |= PARENT_NOTIFY;
			CHECK;
		}

		good.window = btp->wid;

		if (checkevent((XEvent *)&good, &ev)) {
			report("Bad event on window '%s'", btp->name);
			FAIL;
		} else
			CHECK;
	}

	/*
	 * Now check for map notify on the window itself.
	 */
	good.display = display;

	/* Should go through this loop NELEM(T1)-1 times */
	while (getevent(display, &ev)) {

		mp = (XMapEvent*)&ev;
		btp = btwtobtp(bt, mp->window);
		if (btp == NULL) {
			report("Event received for unknown window");
			FAIL;
			continue;
		}

		if (btp->uflags & WINDOW_NOTIFY) {
			report("Repeat map notify event received on window '%s'", btp->name);
			FAIL;
		} else {
			btp->uflags |= WINDOW_NOTIFY;
			CHECK;
		}

		good.window = btp->wid;
		good.event = btp->wid;

		if (checkevent((XEvent *)&good, &ev)) {
			report("Bad event on window '%s'", btp->name);
			FAIL;
		} else
			CHECK;
	}

	for (i = 1; i < NELEM(T1); i++) {

		if (bt[i].uflags & WINDOW_NOTIFY)
			CHECK;
		else {
			report("No map notify received on window '%s'", bt[i].name);
			FAIL;
		}
		if (bt[i].uflags & PARENT_NOTIFY)
			CHECK;
		else {
			report("No map notify received on parent of window '%s'", bt[i].name);
			FAIL;
		}
		(void) XGetWindowAttributes(display, w, &atts);
		if (atts.map_state != IsViewable) {
			report("map state after map was %s, expecting %s",
				mapstatename(atts.map_state), mapstatename(IsViewable));
			FAIL;
		} else
			CHECK;
	}

	CHECKPASS(8*(NELEM(T1)-1));
>>ASSERTION Good A
When one of the subwindows becomes viewable after a call to xname
and no earlier contents for it are remembered, then
the subwindow is tiled with its background.
>>STRATEGY
>># Have already tested this with a plain background, but take the
>># opportunity to test with a background tile.
Create subwindows.
Create tile.
Set background of each subwindow to tile.
Map subwindows.
Verify that subwindows are tiled.
Re-run with an inferior window.
>>CODE
Window	base;
Window	win;
Window	win2;
Pixmap	tile;

	base = defwin(display);

	win = creunmapchild(display, base, (struct area *)0);

	w = base;

	tile = maketile(display, win);
	XSetWindowBackgroundPixmap(display, win, tile);

	XCALL;

	if (checktile(display, win, (struct area *)0, 0, 0, tile))
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
	win = creunmapchild(display, base, (struct area *)0);
	win2 = crechild(display, win, (struct area *)0);
	w = base;
	XSetWindowBackgroundPixmap(display, win2, tile);

	XCALL;

	if (checktile(display, win2, (struct area *)0, 0, 0, tile))
		CHECK;
	else {
		report("Inferior window not tiled with its background correctly");
		FAIL;
	}

	CHECKPASS(2);
>>ASSERTION Good A
>># This is inconsistant with other stuff? In create.
When the background of the subwindow is undefined then the existing screen
contents from the parent or an inferior of the parent are not altered,
and zero or more 
.S Expose 
events are generated.
>>STRATEGY
Create window as parent.
Draw pattern on parent.
Create inferior window and draw pattern on that too.
Create unmapped subwindows that overlap inferior.
Set background pixmap to None on each subwindow.
Map subwindows.
Verify that screen contents have not been altered.

Unmap subwindows.
For each subwindow
	Select for expose events.
	Call setforexpose() to draw into window (for if backing store in use)
Call xname to map subwindows.
For each window
	Verify that window was restored from backing store or received expose
	events that cover the window with exposecheck().
>>CODE
Window	base;
Window	ch1;
struct	buildtree	*bt;
struct	area	area;
XImage	*imp;
int 	i;

	base = defwin(display);
	pattern(display, base);

	w = base;

	setarea(&area, 7, 7, 50, 50);
	ch1 = crechild(display, base, &area);
	pattern(display, ch1);

	/* Save original contents */
	imp = savimage(display, base);

	bt = buildtree(display, base, T1, NELEM(T1));

	for (i = 1; i < NELEM(T1); i++)
		XSetWindowBackgroundPixmap(display, bt[i].wid, None);

	XCALL;

	if (compsavimage(display, base, imp))
		CHECK;
	else {
		report("Screen contents altered by mapping window with undefined background");
		FAIL;
	}

	/*
	 * Check expose events.  Unmap the windows and draw into them with
	 * setforexpose(), (this will normally have no effect unless backing
	 * store is active).  Map the window and check that either
	 *  - there are zero expose events and backing store has preserved the
	 *    contents.
	 *  - there are enough expose events to cover the window.
	 */
	XUnmapSubwindows(display, w);

	for (i = 1; i < NELEM(T1); i++) {
		XSelectInput(display, bt[i].wid, ExposureMask);
		setforexpose(display, bt[i].wid);
		/* because setforexpose() changes the background */
		XSetWindowBackgroundPixmap(display, bt[i].wid, None);
		/*
		 * Border width is set to zero, because otherwise the border potentialy
		 * confuses things.
		 */
		XSetWindowBorderWidth(display, bt[i].wid, None);
	}

	XCALL;

	for (i = 1; i < NELEM(T1); i++)
		exposefill(display, bt[i].wid);
	for (i = 1; i < NELEM(T1); i++) {
		if (exposecheck(display, bt[i].wid))
			CHECK;
		else {
			report("Neither Expose events or backing store processing");
			report("could correctly restore the window contents.");
			FAIL;
		}
	}

	CHECKPASS(1 + (NELEM(T1)-1));
>>ASSERTION Good B 3
When the background of the subwindow is undefined then the existing screen
contents not from the parent or an inferior of the parent are undefined,
and zero or more 
.S Expose 
events are generated.
>>ASSERTION Good D 3
If backing store is supported:
When backing store has been maintained while a particular
subwindow was unmapped, then no 
.S Expose 
events are generated for that subwindow.
>>STRATEGY
If backing store is supported
  Create base window.
  Create child of this window to unmap.
  Set backing store hint of child to Always.
  Write into child window.
  Unmap child window.
  Enable Expose events.
  Map child window by calling xname on base window.
  If child window contents are unaltered.
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
Window	base;
Window	ch;

	if (DoesBackingStore(DefaultScreenOfDisplay(display)) != Always) {
		unsupported("Backing store not supported while unmapped");
		return;
	}

	base = defwin(display);
	ch = crechild(display, base, (struct area *)0);

	setatts.backing_store = Always;
	XChangeWindowAttributes(display, ch, CWBackingStore, &setatts);

	dset(display, ch, W_FG);
	XUnmapWindow(display, ch);
	XSelectInput(display, ch, ExposureMask);

	w = base;
	XCALL;

	XSelectInput(display, ch, NoEventMask);

	if (checkarea(display, ch, (struct area *)0, W_FG, W_FG, CHECK_ALL|CHECK_DIFFER)) {
		/*
		 * Backing store apeared to be active during the unmap.
		 */
		if (XCheckTypedWindowEvent(display, ch, Expose, &ev)) {
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
When the server elects to now maintain backing store for a subwindow, 
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
Window	ch;
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
	ch = creunmapchild(display, base, &area);
	setatts.backing_store = WhenMapped;
	XChangeWindowAttributes(display, ch, CWBackingStore, &setatts);

	/* Create a child that will cover the test window */
	setarea(&charea, 0, 0, 20, 20);
	ch1 = crechild(display, base, &charea);

	XSelectInput(display, ch, ExposureMask);
	w = base;
	XCALL;
	XSelectInput(display, ch, NoEventMask);

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

>>ASSERTION Good A
When the server elects to not maintain backing store for a
subwindow, then
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
Window	ch;
Window	ch1;
struct	area	area;
struct	area	charea;
XSetWindowAttributes	setatts;

	base = defwin(display);

	setarea(&area, 0, 0, 70, 70);
	ch = creunmapchild(display, base, &area);
	setatts.backing_store = NotUseful;
	XChangeWindowAttributes(display, ch, CWBackingStore, &setatts);

	/* Create a child that will cover the test window */
	setarea(&charea, 0, 0, 20, 20);
	ch1 = crechild(display, base, &charea);

	XSelectInput(display, ch, ExposureMask);
	w = base;
	XCALL;
	XSelectInput(display, ch, NoEventMask);

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
		dclear(display, ch);
		exposefill(display, ch);
		if (checkarea(display, ch, &charea, W_BG, W_FG, CHECK_ALL))
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

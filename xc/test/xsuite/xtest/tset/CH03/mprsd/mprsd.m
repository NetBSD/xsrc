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
 * $XConsortium: mprsd.m,v 1.9 94/04/17 21:03:06 rws Exp $
 */
>>TITLE XMapRaised CH03
void

Display	*display = Dsp;
Window	w;
>>EXTERN

static char	*T1[] = {
	".",
	"A . (10,10) 40x40",
	"B . (15,15) 40x40",
	"C . (20,20) 40x40 unmap",
	"D . (25,25) 40x40",
	"E . (30,30) 40x40",
};
/* Number of direct children of parent in T1 */
#define	NT1CHILD	5

static char	*T2[] = {
	".",
	"A . (20,20) 40x40 unmap",
	  "B A (5,5) 20x20",
	    "C B (9,4) 5x5",
	"sib . (65, 45) 9x9",
};
#define	NT2CHILD	2

>>ASSERTION Good A
A call to xname maps the specified
window and raises it to the top of the stacking order.
>>STRATEGY
Create windows, one of which is unmapped in the middle of the stacking order.
Map window.
Verify map state is IsViewable.
Verify that window shows on screen.
Verify that window is at top of stacking order.
>>CODE
int 	so;
Window	base;
struct	buildtree	*btp;
struct	buildtree	*btlist;
XWindowAttributes	atts;

	base = defwin(display);

	btlist = buildtree(display, base, T1, NELEM(T1));

	w = btntow(btlist, "C");
	if (isdeleted())
		return;

	XCALL;
	(void) XGetWindowAttributes(display, w, &atts);
	if (atts.map_state != IsViewable) {
		report("map state after map was %s, expecting %s",
			mapstatename(atts.map_state), mapstatename(IsViewable));
		FAIL;
	} else
		CHECK;

	PIXCHECK(display, base);

	so = stackorder(display, w);
	if (so != NT1CHILD-1) {
		report("Window was not raised to the top, stack pos %d, expecting %d",
			so, NT1CHILD-1);
		FAIL;
	} else
		CHECK;

	CHECKPASS(3);

>>ASSERTION def
When a call to xname is made on a window that has no unmapped
ancestors, then the window becomes viewable.
>>ASSERTION Good A
When a call to xname is made on a window that
has an unmapped ancestor, then the window does not become
viewable.
>>STRATEGY
Create windows with an unmapped ancestor.
Map one of these window that has an unmapped ancestor.
Verify map state is IsUnviewable.
Verify that nothing new becomes visible.
>>CODE
Window	base;
XImage	*imp;
struct	buildtree	*btp;
XWindowAttributes	atts;

	base = defwin(display);
	btp = buildtree(display, base, T2, NELEM(T2));

	/* We shall map window B which has A as an unmapped ancestor */
	w = btntow(btp, "B");

	imp = savimage(display, base);

	XCALL;

	(void) XGetWindowAttributes(display, w, &atts);
	if (atts.map_state != IsUnviewable) {
		report("map state after map was %s, expecting %s",
			mapstatename(atts.map_state), mapstatename(IsUnviewable));
		FAIL;
	} else
		CHECK;

	if (compsavimage(display, base, imp))
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
struct	buildtree	*ch1;
struct	buildtree	*ch2;
struct	buildtree	*btp;
XWindowAttributes	atts;

	base = defwin(display);
	btp = buildtree(display, base, T2, NELEM(T2));

	ch1 = btntobtp(btp, "B");
	ch2 = btntobtp(btp, "C");

	XGetWindowAttributes(display, ch1->wid, &atts);
	if (atts.map_state != IsUnviewable) {
		report("map state for inferior %s was %s, expecting %s",
			ch1->name,mapstatename(atts.map_state), mapstatename(IsUnviewable));
		FAIL;
	} else
		CHECK;

	XGetWindowAttributes(display, ch2->wid, &atts);
	if (atts.map_state != IsUnviewable) {
		report("map state for inferior %s was %s, expecting %s",
			ch2->name,mapstatename(atts.map_state), mapstatename(IsUnviewable));
		FAIL;
	} else
		CHECK;

	w = btntow(btp, "A");

	XCALL;

	XGetWindowAttributes(display, ch1->wid, &atts);
	if (atts.map_state != IsViewable) {
		report("map state for inferior %s was %s, expecting %s",
			ch1->name, mapstatename(atts.map_state), mapstatename(IsViewable));
		FAIL;
	} else
		CHECK;

	XGetWindowAttributes(display, ch2->wid, &atts);
	if (atts.map_state != IsViewable) {
		report("map state for inferior %s was %s, expecting %s",
			ch2->name, mapstatename(atts.map_state), mapstatename(IsViewable));
		FAIL;
	} else
		CHECK;

	PIXCHECK(display, base);

	CHECKPASS(5);
>># ** The following passed the review but appears bogus - the window is
>>#    still raised.
>># ASSERTION Good A
>># When the window is already mapped, then a call to xname has no effect.
>>ASSERTION Good A
When the override-redirect attribute of the window is
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

	/* There is also a configure request */
	ev.type = -1;
	while (getevent(client2, &ev) > 0) {
		if (ev.type == MapRequest) {
			if (checkevent((XEvent *)&good, &ev))
				FAIL;
			else
				CHECK;
			break;
		}
	}
	if (ev.type != MapRequest) {
		report("No MapRequest event generated");
		FAIL;
	} else
		CHECK;

	(void) XGetWindowAttributes(display, w, &atts);
	if (atts.map_state != IsUnmapped) {
		report("map state after map was %s, expecting %s",
			mapstatename(atts.map_state), mapstatename(IsUnmapped));
		FAIL;
	} else
		CHECK;

	CHECKPASS(3);

>>ASSERTION Good A
When the override-redirect attribute of the window is 
.S False 
and some other client has selected 
.S SubstructureRedirectMask 
on the parent window, then a 
.S ConfigureRequest 
event is generated, and the call to xname does not restack the window.
>>STRATEGY
Create base window.
Create unmapped window on base window.
Set override-redirect attribute to False.
Create sibling to test window so that stacking order test is meaningful.
Create second client.
Select SubstructureRedirect for second client on base window.
Map window.
Verify that a ConfigureRequest event is generated on second client.
Verify that stacking order is unchanged.
>>CODE
Window	base;
Window	ch;
Display	*client2;
XSetWindowAttributes	setatts;
XConfigureRequestEvent	good;
struct	area	area;
struct	area	charea;
int 	origso;
XEvent	ev;
int 	n;

	base = defwin(display);
	setarea(&area, 20, 20, 40, 40);
	w = creunmapchild(display, base, &area);
	setatts.override_redirect = False;
	XChangeWindowAttributes(display, w, CWOverrideRedirect, &setatts);

	setarea(&charea, 65, 45, 9, 9);
	ch = crechild(display, base, &charea);

	origso = stackorder(display, w);
	if (isdeleted())
		return;

	client2 = opendisplay();
	XSelectInput(client2, base, SubstructureRedirectMask);
	XSync(client2, True);

	XCALL;
	XSync(client2, False);

	good.type = ConfigureRequest;
	good.serial = 0;
	good.send_event = False;
	good.display = client2;
	good.parent = base;
	good.window = w;
	good.x = area.x;
	good.y = area.y;
	good.width = area.width;
	good.height = area.height;
	good.border_width = 0;

	good.above = None;
	good.detail = Above;
	good.value_mask = CWStackMode;

	/* There is also a map request */
	ev.type = -1;
	while (getevent(client2, &ev) > 0) {
		if (ev.type == ConfigureRequest) {
			if (checkevent((XEvent *)&good, &ev))
				FAIL;
			else
				CHECK;
			break;
		}
	}
	if (ev.type != ConfigureRequest) {
		report("No ConfigureRequest event generated");
		FAIL;
	} else
		CHECK;

	if (stackorder(display, w) != origso) {
		report("Stacking order changed");
		FAIL;
	} else
		CHECK;

	CHECKPASS(3);

>>ASSERTION Good A
When the
.M override-redirect
attribute of the window is
.S True ,
then the window is mapped and a
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
Verify that a ConfigureNotify event is generated on parent.
Verify that a ConfigureNotify event is generated on window.
Verify that the map_state of the window is IsViewable.
>>CODE
Window	base;
Display	*client2;
XSetWindowAttributes	setatts;
XWindowAttributes	atts;
XConfigureEvent	confgood;
XMapEvent	mapgood;
struct	buildtree	*btlist;
struct	buildtree	*btp;
struct	area	area;
XEvent	ev;
int 	gotmap;
int 	gotconf;
int 	n;

	base = defwin(display);
	btlist = buildtree(display, base, T2, NELEM(T2));

	w = btntow(btlist, "A");
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

	mapgood.type = MapNotify;
	mapgood.serial = 0;
	mapgood.send_event = False;
	mapgood.display = client2;
	mapgood.event = base;
	mapgood.window = w;
	mapgood.override_redirect = True;

	btp = btwtobtp(btlist, w);
	confgood.type = ConfigureNotify;
	confgood.serial = 0;
	confgood.send_event = False;
	confgood.display = client2;
	confgood.event = base;
	confgood.window = w;
	confgood.x = btp->x;
	confgood.y = btp->y;
	confgood.width = btp->width;
	confgood.height = btp->height;
	confgood.border_width = 0;

	confgood.above = btntow(btlist, "sib");
	confgood.override_redirect = True;

	n = XPending(client2);
	if (n != 2) {
		report("Expecting two events on second client, received %d", n);
		FAIL;
	} else
		CHECK;

	gotmap = 0;
	gotconf = 0;
	while (getevent(client2, &ev) > 0) {
		if (ev.type == MapNotify) {
			gotmap++;
			if (checkevent((XEvent*)&mapgood, &ev))
				FAIL;
			else
				CHECK;
		} else if (ev.type == ConfigureNotify) {
			gotconf++;
			if (checkevent((XEvent*)&confgood, &ev))
				FAIL;
			else
				CHECK;
		} else {
			report("Unexpected event (%s) received", eventname(ev.type));
			FAIL;
		}
	}
	if ((gotconf != 1 || gotmap != 1) && fail == 0) {
		report("Repeated events received");
		FAIL;
	} else
		CHECK;

	/*
	 * Now check for map notify on the window itself.
	 */
	mapgood.display = display;
	mapgood.event = w;

	confgood.display = display;
	confgood.event = w;

	n = XPending(display);
	if (n != 2) {
		report("Expecting two events on window, received %d", n);
		FAIL;
	} else
		CHECK;

	gotmap = 0;
	gotconf = 0;
	while (getevent(display, &ev) > 0) {
		if (ev.type == MapNotify) {
			gotmap++;
			if (checkevent((XEvent*)&mapgood, &ev))
				FAIL;
			else
				CHECK;
		} else if (ev.type == ConfigureNotify) {
			gotconf++;
			if (checkevent((XEvent*)&confgood, &ev))
				FAIL;
			else
				CHECK;
		} else {
			report("Unexpected event (%s) received", eventname(ev.type));
			FAIL;
		}
	}
	if ((gotconf != 1 || gotmap != 1) && fail == 0) {
		report("Repeated events received");
		FAIL;
	} else
		CHECK;

	(void) XGetWindowAttributes(display, w, &atts);
	if (atts.map_state != IsViewable) {
		report("map state after map was %s, expecting %s",
			mapstatename(atts.map_state), mapstatename(IsViewable));
		FAIL;
	} else
		CHECK;

	if (stackorder(display, w) != NT2CHILD-1) {
		report("Window not raised to the top of the stacking order");
		FAIL;
	} else
		CHECK;

	CHECKPASS(10);

>>ASSERTION Good A
When no other client has selected
.S SubstructureRedirectMask
on the parent window, then on a call to xname
the window is mapped and a
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
Verify that a ConfigureNotify event is generated on parent.
Verify that a ConfigureNotify event is generated on window.
Verify that the map_state of the window is IsViewable.
Verify that window is at top of stacking order.
>>CODE
Window	base;
Display	*client2;
XSetWindowAttributes	setatts;
XWindowAttributes	atts;
XConfigureEvent	confgood;
XMapEvent	mapgood;
struct	buildtree	*btlist;
struct	buildtree	*btp;
struct	area	area;
XEvent	ev;
int 	gotmap;
int 	gotconf;
int 	n;

	base = defwin(display);
	btlist = buildtree(display, base, T2, NELEM(T2));

	w = btntow(btlist, "A");
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

	mapgood.type = MapNotify;
	mapgood.serial = 0;
	mapgood.send_event = False;
	mapgood.display = client2;
	mapgood.event = base;
	mapgood.window = w;
	mapgood.override_redirect = False;

	btp = btwtobtp(btlist, w);
	confgood.type = ConfigureNotify;
	confgood.serial = 0;
	confgood.send_event = False;
	confgood.display = client2;
	confgood.event = base;
	confgood.window = w;
	confgood.x = btp->x;
	confgood.y = btp->y;
	confgood.width = btp->width;
	confgood.height = btp->height;
	confgood.border_width = 0;

	confgood.above = btntow(btlist, "sib");
	confgood.override_redirect = False;

	n = XPending(client2);
	if (n != 2) {
		report("Expecting two events on second client, received %d", n);
		FAIL;
	} else
		CHECK;

	gotmap = 0;
	gotconf = 0;
	while (getevent(client2, &ev) > 0) {
		if (ev.type == MapNotify) {
			gotmap++;
			if (checkevent((XEvent*)&mapgood, &ev))
				FAIL;
			else
				CHECK;
		} else if (ev.type == ConfigureNotify) {
			gotconf++;
			if (checkevent((XEvent*)&confgood, &ev))
				FAIL;
			else
				CHECK;
		} else {
			report("Unexpected event (%s) received", eventname(ev.type));
			FAIL;
		}
	}
	if ((gotconf != 1 || gotmap != 1) && fail == 0) {
		report("Repeated events received");
		FAIL;
	} else
		CHECK;

	/*
	 * Now check for map notify on the window itself.
	 */
	mapgood.display = display;
	mapgood.event = w;

	confgood.display = display;
	confgood.event = w;

	n = XPending(display);
	if (n != 2) {
		report("Expecting two events on window, received %d", n);
		FAIL;
	} else
		CHECK;

	gotmap = 0;
	gotconf = 0;
	while (getevent(display, &ev) > 0) {
		if (ev.type == MapNotify) {
			gotmap++;
			if (checkevent((XEvent*)&mapgood, &ev))
				FAIL;
			else
				CHECK;
		} else if (ev.type == ConfigureNotify) {
			gotconf++;
			if (checkevent((XEvent*)&confgood, &ev))
				FAIL;
			else
				CHECK;
		} else {
			report("Unexpected event (%s) received", eventname(ev.type));
			FAIL;
		}
	}
	if ((gotconf != 1 || gotmap != 1) && fail == 0) {
		report("Repeated events received");
		FAIL;
	} else
		CHECK;

	(void) XGetWindowAttributes(display, w, &atts);
	if (atts.map_state != IsViewable) {
		report("map state after map was %s, expecting %s",
			mapstatename(atts.map_state), mapstatename(IsViewable));
		FAIL;
	} else
		CHECK;

	if (stackorder(display, w) != NT2CHILD-1) {
		report("Window not raised to the top of the stacking order");
		FAIL;
	} else
		CHECK;

	CHECKPASS(10);
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
When the background of the window is undefined
then the existing screen contents 
from the parent or an inferior of the parent are not altered, 
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
When the background of the window is undefined
then the existing screen contents 
not from the parent or an inferior of the parent are undefined,
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

>>ASSERTION Good A
When the server elects to not maintain backing store for a
window, then
.S Expose
events are generated only for visible regions.
>># NOTE: 'are' reported is from ANSI.  Spec says 'may be' reported.
>>#	I'd try with 'are' and see if anyone notices. - kieron
>>STRATEGY
Create small window.
Create child covering this window for use as the test window.
Ask for no backing store.
Create (and map) window that would obscure the test window.
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

	/*
	 * Because the window is raised to the top of the stacking order
	 * then we must ensure that the test window is not a sibling of
	 * the window used to obscure it.  This is done by working with a
	 * grandchild of the base.
	 */
	setarea(&area, 0, 0, 70, 70);
	w = crechild(display, base, &area);
	w = creunmapchild(display, w, (struct area *)0);
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

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
 * $XConsortium: rprntwdw.m,v 1.7 94/04/17 21:06:46 rws Exp $
 */
>>TITLE XReparentWindow CH07
void
xname
Display	*display = Dsp;
Window	w;
Window	parent;
int 	x = RP_X;
int 	y = RP_Y;
>>EXTERN

#define	RP_X	12
#define	RP_Y	15

char	*Treedesc[] = {
	"base borders",
	"A base (1,3) 50x30",
	  "A1 A (10,1) 4x5",
	  "A2 A (20,6) 4x5",
	"B base (4, 40) 20x20",
	  "B1 B (3,4) 10x10",

};

static	struct	buildtree	*Tree;
static	Window	WinA;
static	Window	WinB;
static	Window	WinBase;

static void
reparentinit()
{
Window	win;

	tpstartup();

	win = defwin(Dsp);
	Tree = buildtree(Dsp, win, Treedesc, NELEM(Treedesc));
	WinA = btntow(Tree, "A");
	WinB = btntow(Tree, "B");
	WinBase = btntow(Tree, "base");
}

>># Set up the startup function.
>>SET tpstartup reparentinit

>>ASSERTION Good A
A call to xname
removes the specified window from its current position in the hierarchy,
and inserts it as the child of the specified parent window at the
position specified by
.A x
and
.A y .
>>STRATEGY
Build a window tree.
Call xname to reparent window.
Verify that parent of window has changed.
Verify that the old parent no longer has the specified window as a child.
>>CODE
Window	dummy;
Window	par;
Window	*children;
int 	nchild;

	parent = WinA;
	w = WinB;

	XCALL;

	/*
	 * Check new parent.
	 */
	if (XQueryTree(display, WinB, &dummy, &par, &children, (unsigned*)&nchild) == False) {
		delete("XQueryTree on window failed");
		return;
	}

	if (par != parent) {
		report("New parent was incorrect");
		report(" Got 0x%x, expecting 0x%x", par, parent);
		FAIL;
	} else
		CHECK;

	if (nchild > 0 && children)
		XFree((char*) children);

	/*
	 * Check that window is not still a child of the old parent.
	 */
	if (XQueryTree(display, WinBase, &dummy, &par, &children, (unsigned*)&nchild) == False) {
		delete("XQueryTree on old parent failed");
		return;
	}
	while (nchild-- > 0) {
		if (children[nchild] == WinB)
			break;
	}

	if (nchild == -1)
		CHECK;
	else {
		report("window was still a child of the old parent after reparenting");
		FAIL;
	}

	if (nchild > 0 && children)
		XFree((char*) children);

	CHECKPASS(2);
>>ASSERTION Good A
When the specified window is mapped,
then a call to xname
automatically performs an
.F UnmapWindow
first, and an
.S UnmapNotify
event is generated.
>>STRATEGY
Build a window tree.
Enable StructureNotify events on window.
Enable SubstructureNotify events on original parent.
Reparent window.
Verify that an UnmapNotify is generated on the old parent.
Verify that an UnmapNotify is generated on the window.
>>EXTERN

#define	ON_WINDOW	0x1	/* event received on window itself */
#define	ON_PARENT	0x2	/* event received on parent window */
#define	ON_NEWPARENT 0x4	/* event received on new parent window */

>>CODE
XUnmapEvent	good;
XUnmapEvent	*ump;
XEvent	ev;
int 	got = 0;

	parent = WinA;
	w = WinB;

	XSelectInput(display, WinBase, SubstructureNotifyMask);
	XSelectInput(display, w, StructureNotifyMask);

	XCALL;

	defsetevent(good, display, UnmapNotify);
	good.window = w;
	good.from_configure = False;

	got = 0;
	while (getevent(display, &ev)) {
		if (ev.type != UnmapNotify)
			continue;
		ump = (XUnmapEvent*)&ev;
		if (ump->event == ump->window) {
			/* On the window itself */
			if (got & ON_WINDOW) {
				report("Repeated unmap event on window");
				FAIL;
			} else
				CHECK;
			got |= ON_WINDOW;
			good.event = WinB;
		} else {
			if (got & ON_PARENT) {
				report("Repeated unmap event on parent window");
				FAIL;
			} else
				CHECK;
			got |= ON_PARENT;
			good.event = WinBase;
		}
		if (checkevent((XEvent*)&good, &ev))
			FAIL;
		else
			CHECK;
	}

	if (!(got & ON_PARENT)) {
		report("UnmapNotify event not received on parent window");
		FAIL;
	} else
		CHECK;
	if (!(got & ON_WINDOW)) {
		report("UnmapNotify event not received on window");
		FAIL;
	} else
		CHECK;
	
	CHECKPASS(6);
>>ASSERTION Good A
The window is placed at the top of the stacking order with respect to
sibling windows.
>>STRATEGY
Build a window tree.
Reparent window.
Verify that it has been placed at the top of the stacking order.
>>CODE
int 	n;
int 	nsibs;
struct	buildtree	*btp;

	parent = WinA;
	w = WinB;

	XCALL;

	/* Count the number of siblings that window should have */
	nsibs = 0;
	for (btp = Tree+1; btp < Tree+NELEM(Treedesc); btp++) {
		if (btp->parent->wid == WinA)
			nsibs++;
	}

	/*
	 *This is the stacking order number that the window should have after
	 * reparenting
	 */
	n = stackorder(display, w);
	if (n != nsibs) {
		report("Reparented window had incorrect stacking order");
		report(" Got %d, expecting %d", n, nsibs);
		FAIL;
	} else
		CHECK;

	CHECKPASS(1);
>>ASSERTION Good A
A call to xname generates a
.S ReparentNotify
event on the window and on both the old and new parent windows.
>>STRATEGY
Build a window tree.
Enable SubstructureNotify events on new parent.
Enable SubstructureNotify events on old parent.
Enable StructureNotify events on window.
Reparent window.
Verify ReparentNotify event generated on new parent.
Verify ReparentNotify event generated on old parent.
Verify ReparentNotify event generated on reparented window.
>>CODE
XReparentEvent	good;
XReparentEvent	*rpp;
XEvent	ev;
int 	got = 0;

	parent = WinA;
	w = WinB;

	XSelectInput(display, WinA, SubstructureNotifyMask);
	XSelectInput(display, WinBase, SubstructureNotifyMask);
	XSelectInput(display, w, StructureNotifyMask);

	XCALL;

	defsetevent(good, display, ReparentNotify);
	good.event = 0;
	good.window = w;
	good.parent = WinA;
	good.x = x;
	good.y = y;
	good.override_redirect = False;

	got = 0;
	while (getevent(display, &ev)) {
		if (ev.type != ReparentNotify)
			continue;
		rpp = (XReparentEvent*)&ev;
		if (rpp->event == rpp->window) {
			/* On the window itself */
			if (got & ON_WINDOW) {
				report("Repeated unmap event on window");
				FAIL;
			} else
				CHECK;
			got |= ON_WINDOW;
			good.event = WinB;
		} else if (rpp->event == WinA) {
			if (got & ON_NEWPARENT) {
				report("Repeated unmap event on new parent window");
				FAIL;
			} else
				CHECK;
			got |= ON_NEWPARENT;
			good.event = WinA;
		} else if (rpp->event == WinBase) {
			if (got & ON_PARENT) {
				report("Repeated unmap event on old parent window");
				FAIL;
			} else
				CHECK;
			got |= ON_PARENT;
			good.event = WinBase;
		} else {
			report("Event received on window other than old or new parent");
			FAIL;
		}
		if (checkevent((XEvent*)&good, &ev))
			FAIL;
		else
			CHECK;
	}

	if (!(got & ON_NEWPARENT)) {
		report("UnmapNotify event not received on new parent window");
		FAIL;
	} else
		CHECK;
	if (!(got & ON_PARENT)) {
		report("UnmapNotify event not received on old parent window");
		FAIL;
	} else
		CHECK;
	if (!(got & ON_WINDOW)) {
		report("UnmapNotify event not received on window");
		FAIL;
	} else
		CHECK;
	
	CHECKPASS(9);

>>ASSERTION Good A
When the specified window was originally mapped,
then a call to xname automatically performs a
.S MapWindow
request on it after the window is reparented,
and a
>># does this really happen?
>># Split into two assertions??
.S MapRequest
or
.S MapNotify
event is generated.
>># If the new parent has SubstructureRedirect then we would expect
>># a MapRequest, rather than a MapNotify.
>>STRATEGY
Build window tree.
Enable StructureNotify on window.
Enable SubstructureNotify on new parent.
Reparent window with xname.
Verify that MapNotify is received on window.
Verify that MapNotify is received on parent.

Build window tree.
Create second client.
Enable SubstructureRedirect for second client on new parent.
Reparent window with xname.
Verify that MapRequest is received for second client on parent.
>>CODE
XMapEvent	good;
XMapEvent	*mp;
XMapRequestEvent	mreqgood;
XEvent	ev;
Display	*client2;
int 	n;
int 	got = 0;

	parent = WinA;
	w = WinB;

	XSelectInput(display, parent, SubstructureNotifyMask);
	XSelectInput(display, w, StructureNotifyMask);

	XCALL;

	defsetevent(good, display, MapNotify);
	good.window = w;
	good.override_redirect = False;

	got = 0;
	while (getevent(display, &ev)) {
		if (ev.type != MapNotify)
			continue;
		mp = (XMapEvent*)&ev;
		if (mp->event == mp->window) {
			/* On the window itself */
			if (got & ON_WINDOW) {
				report("Repeated map event on window");
				FAIL;
			} else
				CHECK;
			got |= ON_WINDOW;
			good.event = WinB;
		} else {
			if (got & ON_NEWPARENT) {
				report("Repeated map event on parent window");
				FAIL;
			} else
				CHECK;
			got |= ON_NEWPARENT;
			good.event = WinA;
		}
		if (checkevent((XEvent*)&good, &ev))
			FAIL;
		else
			CHECK;
	}

	if (!(got & ON_NEWPARENT)) {
		report("MapNotify event not received on parent window");
		FAIL;
	} else
		CHECK;
	if (!(got & ON_WINDOW)) {
		report("MapNotify event not received on window");
		FAIL;
	} else
		CHECK;

	if (isdeleted())
		return;

	client2 = opendisplay();
	XSelectInput(client2, parent, SubstructureRedirectMask);
	XSync(client2, True);

	XCALL;
	XSync(client2, False);

	defsetevent(mreqgood, client2, MapRequest);
	mreqgood.parent = parent;
	mreqgood.window = WinB;

	n =  getevent(client2, &ev);
	if (n != 1) {
		report("Expecting one MapRequest event, got %d events", n);
		FAIL;
	} else
		CHECK;
	if (ev.type != MapRequest) {
		report("Expecting one MapRequest event, got event type %s", eventname(ev.type));
		FAIL;
	} else
		CHECK;

	if (checkevent((XEvent*)&mreqgood, &ev))
		FAIL;
	else
		CHECK;

	CHECKPASS(9);

>>ASSERTION Good A
When a call to xname uncovers part of any window that was formerly
obscured, then either
.S Expose
events are generated or the contents are restored from backing store.
>>STRATEGY
Build window tree.
Use WinB as old parent and B1 and window.
Enable expose events on old parent window.
Set up old parent window with setforexpose.
Call xname to reparent window.
Verify correct expose processing with checkexpose().
>>CODE

	parent = WinA;
	w = btntow(Tree, "B1");

	XSelectInput(display, WinB, ExposureMask);
	setforexpose(display, WinB);

	XCALL;

	if (exposecheck(display, WinB))
		CHECK;
	else {
		report("Contents of window not restored correctly by Expose or backing store");
		FAIL;
	}

	CHECKPASS(1);

>># Go back to default tpstartup
>>SET tpstartup tpstartup
>>ASSERTION Good B 1
A call to xname may generate
.S Expose
events for regions uncovered by the initial
.S UnmapWindow
request that are immediately obscured by the final
.S MapWindow
request.
>>ASSERTION Bad C
>># Added conditional clause to approved assertion - DPJ Cater 29/8/91
If multiple screens are supported:
When the new parent window is not on the same screen as
the old parent window, then a
.S BadMatch
error occurs.
>>STRATEGY
If multiple screens supported
  Create window on default screen.
  Create a new parent window on alternate screen.
  Attempt to reparent to alternate screen.
  Verify BadMatch error occurs.
else
  UNSUPPORTED
>>CODE BadMatch
Window	w;

	if (config.alt_screen == -1) {
		unsupported("Multiple screens not supported");
		return;
	}

	w = defwin(display);
	parent = defdraw(display, VI_ALT_WIN);

	XCALL;

	if (geterr() == BadMatch)
		PASS;
	else
		FAIL;
>>ASSERTION Bad A
When the new parent window is the specified window, then a
.S BadMatch
error occurs.
>>STRATEGY
Create window.
Call xname with both window parameters set to this window.
Verify BadMatch error.
>>CODE BadMatch

	w = defwin(display);
	parent = w;

	XCALL;

	if (geterr() == BadMatch)
		PASS;
	else
		FAIL;
>>ASSERTION Bad A
When the new parent window is an inferior of the
specified window, then a
.S BadMatch
error occurs.
>>STRATEGY
>>SET tpstartup reparentinit
Build window tree.
Set w.
Set parent to an inferior of w.
Call xname.
Verify that a BadMatch error occurs.
>>CODE BadMatch

	w = WinA;
	parent = btntow(Tree, "A1");

	XCALL;

	if (geterr() == BadMatch)
		PASS;
	else
		FAIL;
>>ASSERTION Bad C
If multiple window depths are supported:
When the specified window has a
.S ParentRelative
background, and the new parent window does not have the same depth as the
specified window, then a
.S BadMatch
error occurs.
>>STRATEGY
If only one window depth is supported
  UNSUPPORTED
Create window.
Set background to parent relative.
Create new parent window with depth not equal to window.
Call xname.
Verify that BadMatch error occurs.
>>CODE BadMatch
XVisualInfo	*vp;
unsigned int	depth;

	depth = DefaultDepth(display, DefaultScreen(display));
	for (resetvinf(VI_WIN); nextvinf(&vp); ) {
		if (!w && vp->depth == depth) {
			w = makewin(display, vp);
			if (parent)
				break;
		} else if (!parent && vp->depth != depth) {
			parent = makewin(display, vp);
			if (w)
				break;
		}
	}

	if (!parent) {
		unsupported("Only one depth window supported");
		return;
	}

	/*
	 * At the time of writing we do not have a machine on which
	 * the following code can be tested.
	 */
	XSetWindowBackgroundPixmap(display, w, ParentRelative);

	XCALL;

	if (geterr() == BadMatch)
		PASS;
	else
		FAIL;
>>ASSERTION Bad A
.ER BadWindow
>>ASSERTION Bad A
When the new parent is
.S InputOnly
and the window is
.S InputOutput ,
then a
.S BadMatch
error occurs.
>>STRATEGY
Create an InputOnly window as the parent.
Create window.
Call xname.
Verify that a BadMatch error occurs.
>>SET tpstartup tpstartup
>>CODE BadMatch

	parent = iponlywin(display);
	w = defwin(display);

	XCALL;

	if (geterr() == BadMatch)
		PASS;
	else
		FAIL;	/* already done */

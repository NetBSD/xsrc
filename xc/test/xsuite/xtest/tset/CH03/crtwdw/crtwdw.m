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
 * $XConsortium: crtwdw.m,v 1.12 94/04/17 21:03:02 rws Exp $
 */
>>TITLE XCreateWindow CH03
Window

Display	*display = Dsp;
Window	parent = DRW(Dsp);
int 	x = 50;
int 	y = 60;
unsigned int 	width = 20;
unsigned int 	height = 17;
unsigned int 	border_width = 2;
int 	depth = DefaultDepth(Dsp, DefaultScreen(Dsp));
unsigned int 	class = InputOutput;
Visual	*visual = DefaultVisual(Dsp, DefaultScreen(Dsp));
unsigned long	valuemask = 0;
XSetWindowAttributes	*attributes = &Atts;
>>SET fail-return
>>SET startup fontstartup
>>SET cleanup fontcleanup
>>EXTERN

static XSetWindowAttributes	Atts;

static Window
interceptXCW(display, parent, x, y, width, height, border_width, depth, class, visual, valuemask, attributes)
Display	*display;
Window	parent;
int 	x;
int 	y;
unsigned int 	width;
unsigned int 	height;
unsigned int 	border_width;
int 	depth;
unsigned int 	class;
Visual	*visual;
unsigned long	valuemask;
XSetWindowAttributes	*attributes;
{
Colormap	cm;
Visual	*vis;
XWindowAttributes	atts;

	/*
	 * Problem:  The colormap has to match the visual, however the default
	 * is CopyFromParent which is of no use when the parent has a different
	 * type to the window that is being created.
	 * If parent, visual, depths are not default and the test is not already
	 * setting the colormap and class is InputOutput then set an appropriate
	 * one here.
	 */
	if ((valuemask & CWColormap) == 0 &&
			(
			visual != DefaultVisual(Dsp, DefaultScreen(Dsp)) ||
			parent != DefaultRootWindow(Dsp) ||
			depth != DefaultDepth(Dsp, DefaultScreen(Dsp))
			)
		) {

		if (visual == CopyFromParent || class == CopyFromParent)
			XGetWindowAttributes(display, parent, &atts);

		if (visual == CopyFromParent) {
			vis = atts.visual;
		} else
			vis = visual;

		if (class == InputOutput || (class == CopyFromParent && atts.class == InputOutput)) {
			cm = makecolmap(display, vis, AllocNone);
			attributes->colormap = cm;
			valuemask |= CWColormap;
		}
	}

        /*
         * Set the border pixel value if not set to ensure that a depth
         * mismatch does not occur (since the default BorderPixel is
         * CopyFromParent) causing a BadMatch.
	 * We only need to do this for InputOutput windows, of course,
	 * otherwise we get a BadMatch for an attempted draw on an
	 * InputOnly window.
 	 * Also we must never ever do this if we are using a border pixmap.
         */
	if ( (valuemask & CWBorderPixmap) == 0 && 
		(valuemask & CWBorderPixel) == 0 &&
		( visual != DefaultVisual(Dsp, DefaultScreen(Dsp)) ||
	 	 parent != DefaultRootWindow(Dsp) ||
		 depth != DefaultDepth(Dsp, DefaultScreen(Dsp))
		))
	{

		if (class == CopyFromParent)
			XGetWindowAttributes(display, parent, &atts);

		if (class == InputOutput ||
			(class == CopyFromParent &&
			atts.class == InputOutput))
		{

			attributes->border_pixel = W_FG;
			valuemask |= CWBorderPixel;
		}
        }

	return XCreateWindow(display, parent, x, y, width, height, border_width, depth, class, visual, valuemask, attributes);
}
#define XCreateWindow interceptXCW

>>ASSERTION Good A
A call to xname creates an unmapped subwindow for a specified parent window
and returns the window ID of the created window.
>>STRATEGY
Call xname.
Do simple checks on returned id.
>>CODE
Window	win;

	win = XCALL;
	if (win & 0xe0000000) {
		report("Window ID has some of top three bits set");
		FAIL;
	} else {
		XDestroyWindow(display, win);
		PASS;
	}


>>ASSERTION Good A
The window attributes specified in
.A valuemask
are set to the values in the
.A attributes
structure.
>>STRATEGY
Create window with all attributes set to non-default values.
Get the window attributes with XGetWindowAttributes.
Verify that they are the same as the ones set.
The attributes that can't be checked in this way are the subject of other
  assertions.
>>EXTERN
#include	"cursorfont.h"
>>CODE
XSetWindowAttributes	atts;
XWindowAttributes	getatts;
Window	win;
Colormap	cm;
Cursor		curs;
int 	n;

	cm = XCreateColormap(display, DRW(display), DefaultVisual(display, DefaultScreen(display)), AllocNone);
	curs = XCreateFontCursor(display, XC_coffee_mug);

	atts.bit_gravity = SouthEastGravity;
	atts.win_gravity = EastGravity;
	atts.backing_store = WhenMapped;
	atts.backing_planes = 0xaaaaaaaa;
	atts.backing_pixel = 1;
	atts.save_under = True;
	atts.event_mask = PropertyChangeMask;
	atts.do_not_propagate_mask = KeyPressMask;
	atts.override_redirect = True;
	atts.colormap = cm;
	atts.cursor = curs;

	attributes = &atts;
	valuemask = CWBitGravity|CWWinGravity|CWBackingStore|CWBackingPlanes|
		CWBackingPixel|CWSaveUnder|CWEventMask|CWDontPropagate|
		CWOverrideRedirect|CWColormap|CWCursor;

	win = XCALL;

	XGetWindowAttributes(display, win, &getatts);
	if (isdeleted())
		return;

	n = checkatts(attributes, &getatts, valuemask);
	if (n > 0) {
		report("There %s %d incorrect attribute%s", 
					(n>1)?"were":"was", n, (n>1)?"s":"");
		FAIL;
	} else if (n < 0) {
		/* already reported a path check error in checkatts */
		return;
	} else
		CHECK;

	XDestroyWindow(display, win);
	CHECKPASS(1);

>>EXTERN

static int
checkatts(setatts, getatts, vmask)
XSetWindowAttributes	*setatts;
XWindowAttributes	*getatts;
unsigned long 	vmask;
{
int 	pass = 0, fail = 0;

	/*
	 * Maybe we should alway check everything??? (No vmask)
	 */
	if ((vmask&CWBitGravity) && setatts->bit_gravity != getatts->bit_gravity) {
		report("bit_gravity got %s, expected %s",
			bitgravityname(getatts->bit_gravity),
			bitgravityname(setatts->bit_gravity));
		FAIL;
	} else
		CHECK;

	if ((vmask&CWWinGravity) && setatts->win_gravity != getatts->win_gravity) {
		report("window_gravity got %s, expected %s",
			wingravityname(getatts->win_gravity),
			wingravityname(setatts->win_gravity));
		FAIL;
	} else
		CHECK;

	if ((vmask&CWBackingStore) && setatts->backing_store != getatts->backing_store) {
		report("backing_store got %s, expected %s",
			backingstorename(getatts->backing_store),
			backingstorename(setatts->backing_store));
		FAIL;
	} else
		CHECK;

	if ((vmask&CWBackingPlanes) && setatts->backing_planes != getatts->backing_planes) {
		report("backing_planes got 0x%x, expected 0x%x",
			getatts->backing_planes,
			setatts->backing_planes);
		FAIL;
	} else
		CHECK;

	if (setatts->backing_pixel != getatts->backing_pixel) {
		report("backing_pixel got 0x%x, expected 0x%x",
			getatts->backing_pixel,
			setatts->backing_pixel);
		FAIL;
	} else
		CHECK;

	if ((vmask&CWSaveUnder) && setatts->save_under != getatts->save_under) {
		report("save_under got %s, expected %s",
			boolname(getatts->save_under),
			boolname(setatts->save_under));
		FAIL;
	} else
		CHECK;

	if ((vmask&CWEventMask) && setatts->event_mask != getatts->your_event_mask) {
		report("event_mask got %s, expected %s",
			eventmaskname(getatts->your_event_mask),
			eventmaskname(setatts->event_mask));
		FAIL;
	} else
		CHECK;

	if ((vmask&CWDontPropagate) && setatts->do_not_propagate_mask != getatts->do_not_propagate_mask) {
		report("do_not_propagate_mask got %s, expected %s",
			eventmaskname(getatts->do_not_propagate_mask),
			eventmaskname(setatts->do_not_propagate_mask));
		FAIL;
	} else
		CHECK;

	if ((vmask&CWOverrideRedirect) && setatts->override_redirect != getatts->override_redirect) {
		report("override_redirect got %s, expected %s",
			boolname(getatts->override_redirect),
			boolname(setatts->override_redirect));
		FAIL;
	} else
		CHECK;

	if ((vmask&CWColormap) && setatts->colormap != getatts->colormap) {
		report("colormap got 0x%x, expected 0x%x",
			getatts->colormap,
			setatts->colormap);
		FAIL;
	} else
		CHECK;

	if (fail == 0 && pass == 10)
		return(0);
	else {
		if (fail)
			return(fail);
		else
			delete("Path check error in checkatts");
	}
	return(-1);
}

>>ASSERTION Good A
Window attributes that are not specified in
.A valuemask
are set to their default values.
>>STRATEGY
Create window.
Check default values:
bit-gravity: Forget
win-gravity: NorthWest
backing-store: NotUseful
backing-planes: all ones
backing-pixel: zero
save-under: False
event-mask: empty set
do-not-propagate-mask: empty set
override-redirect: False
colormap: CopyFromParent
>>CODE
XSetWindowAttributes	atts;
XWindowAttributes	getatts;
Window	win;
int 	n;

	valuemask = 0;
	win = XCALL;

	/* For the parent colormap */
	XGetWindowAttributes(display, DRW(display), &getatts);

	atts.bit_gravity = ForgetGravity;
	atts.win_gravity = NorthWestGravity;
	atts.backing_store = NotUseful;
	atts.backing_planes = 0xffffffff;
	atts.backing_pixel = 0;
	atts.save_under = False;
	atts.event_mask = 0;
	atts.do_not_propagate_mask = 0;
	atts.override_redirect = False;
	atts.colormap = getatts.colormap;

	if (XGetWindowAttributes(display, win, &getatts) == False) {
		delete("Could not get window attributes");
		return;
	} else
		CHECK;

	n = checkatts(&atts, &getatts, (long)~0);
	if (n > 0) {
		report("There %s %d incorrect attribute%s", 
					(n>1)?"were":"was", n, (n>1)?"s":"");
		FAIL;
	} else if (n < 0) {
		/* already reported a path check error in checkatts */
		return;
	} else
		CHECK;

	CHECKPASS(2);

>>ASSERTION Good A
On a call to xname a
.S CreateNotify
event is generated on the parent window.
>>STRATEGY
Create parent window.
Select SubstructureNotifyMask on parent window.
Create window.
Select NoEventMask on parent window.
Verify that a single CreateNotify event was generated.
Verify that the returned event structure was correct.
>>CODE
XSetWindowAttributes	atts;
XCreateWindowEvent	ge;
XEvent	event;
Window	win;
int 	n;

	parent = defdraw(display, VI_WIN);

	XSelectInput(display, parent, SubstructureNotifyMask);

	trace("depth is %d", depth);
	trace("visual class is %s", displayclassname(visual->class));
	win = XCALL;

	XSelectInput(display, parent, NoEventMask);

	if (getevent(Dsp, &event) != 1) {
		report("Expecting one event");
		FAIL;
		return;
	} else
		CHECK;

	/* Set up a good event structure of what we are expecting */
	ge.type = CreateNotify;
	ge.send_event = False;
	ge.display = display;
	ge.parent  = parent;
	ge.window  = win;
	ge.x = x;
	ge.y = y;
	ge.width   = width;
	ge.height  = height;
	ge.border_width = border_width;
	ge.override_redirect = False;

	n = checkevent((XEvent*)&ge, &event);
	if (n == 0) {
		CHECK;
	} else {
		report("Event incorrect");
		FAIL;
	}

	CHECKPASS(2);
>>ASSERTION Good A
The created window is placed on top in the stacking order
with respect to siblings.
>>STRATEGY
Create a parent window.
Create two overlapping sibling windows.
Verify that second window is at top of stacking order.
Verify that first sibling is next in the stacking order.
>>CODE
Window	w1, w2;
int 	n;

	parent = defdraw(display, VI_WIN);
	if (isdeleted())
		return;
	else
		CHECK;

	trace("depth is %d", depth);
	trace("visual class is %s", displayclassname(visual->class));
	w1 = XCALL;
	w2 = XCALL;

	if ((n = stackorder(display, w1)) != 0) {
		report("Stacking order for w1 was %d, expected 0", n);
		FAIL;
	} else if ((n = stackorder(display, w2)) != 1) {
		report("Stacking order for w2 was %d, expected 1", n);
		FAIL;
	} else
		CHECK;

	XDestroyWindow(display, w1);
	XDestroyWindow(display, w2);
	CHECKPASS(2);

>>ASSERTION Good A
The created window is not displayed.
>>STRATEGY
Create window to use as parent.
Set background to other than W_BG.
Create window with xname.
Verify that parent window is still clear.
>>CODE
XVisualInfo	*vp;
Window	win;

	for (resetvinf(VI_WIN); nextvinf(&vp); ) {
		parent = (Window)makedrawable(display, vp);

		attributes->background_pixel = W_FG;
		valuemask = CWBackPixel;
		win = XCALL;

		if (checkclear(display, parent))
			CHECK;
		else {
			report("Created window was visible");
			FAIL;
		}
		XDestroyWindow(display, win);
	}
	CHECKPASS(nvinf());

>>ASSERTION Good A
When
.A class
is
.S CopyFromParent ,
then the class is taken from the parent window.
>>STRATEGY
Create InputOutput window to use as parent.
Create window with class CopyFromParent.
Verify that class of created window is InputOutput.
Create InputOnly window to use as parent.
Create window with class CopyFromParent.
Verify that class of created window is InputOnly.
>>CODE
Status	s;
Window	win;
XWindowAttributes	atts;

	class = CopyFromParent;
	/*
	 * Have to set depth and border_width to zero as well,
	 * otherwise BadMatch errors will occur.
	 */
	depth = 0;
	border_width = 0;
	parent = defdraw(display, VI_WIN);
	win = XCALL;

	s = XGetWindowAttributes(display, win, &atts);
	if (s == False) {
		delete("XGetWindowAttributes failed");
		return;
	} else
		CHECK;

	if (atts.class != InputOutput) {
		report("Class was %s, expecting InputOutput", classname(atts.class));
		FAIL;
	} else
		CHECK;

	XDestroyWindow(display, win);

	/* Now Input Only */
	parent = iponlywin(display);
	win = XCALL;

	s = XGetWindowAttributes(display, win, &atts);
	if (s == False) {
		delete("XGetWindowAttributes failed");
		return;
	} else
		CHECK;

	if (atts.class != InputOnly) {
		report("Class was %s, expecting InputOnly", classname(atts.class));
		FAIL;
	} else
		CHECK;

	XDestroyWindow(display, win);
	CHECKPASS(4);

>>ASSERTION Good A
When
.A depth
is
.S CopyFromParent
and
.A class
is
.S InputOutput ,
then the depth is taken from the parent window.
>>STRATEGY
Create InputOutput window to use as parent.
Create window with depth CopyFromParent.
Verify that depth of created window is same as the parent.
>>CODE
Status	s;
Window	win;
XWindowAttributes	atts;
int 	parentdepth;

	depth = CopyFromParent;
	parent = defdraw(display, VI_WIN);
	win = XCALL;

	parentdepth = getdepth(display, parent);

	s = XGetWindowAttributes(display, win, &atts);
	if (s == False) {
		delete("XGetWindowAttributes failed");
		return;
	} else
		CHECK;

	if (atts.depth != parentdepth) {
		report("Depth was %d, expecting %d", atts.depth, parentdepth);
		FAIL;
	} else
		CHECK;

	XDestroyWindow(display, win);
	CHECKPASS(2);

>>ASSERTION Good A
When
.A visual
is
.S CopyFromParent ,
then the visual is taken from the parent window.
>>STRATEGY
Create window to use as parent.
Create window with visual CopyFromParent.
Verify that visual of created window is same as the parent.
>>CODE
Status	s;
Window	win;
XWindowAttributes	atts;
XVisualInfo	*vp;

	depth = CopyFromParent;
	visual = CopyFromParent;
	for (resetvinf(VI_WIN); nextvinf(&vp); ) {
		parent = makewin(display, vp);

		win = XCALL;

		s = XGetWindowAttributes(display, win, &atts);
		if (s == False) {
			delete("XGetWindowAttributes failed");
			return;
		} else
			CHECK;

		if (atts.visual != vp->visual) {
			report("Visual was not same as parent");
			FAIL;
		} else
			CHECK;

		XDestroyWindow(display, win);
	}

	CHECKPASS(2*nvinf());

>>ASSERTION Good A
The window is located such that
the upper left outer corner is at the
co-ordinate specified by
.A x
and
.A y
relative to the inside of the border of the parent window.
>>STRATEGY
Create window with border and background pixel W_FG.
Map created window.
Verify position by direct check with checkarea.
>>CODE
Window	win;
struct	area	area;

	border_width = 5;
	parent = defdraw(display, VI_WIN);
	attributes->border_pixel = W_FG;
	attributes->background_pixel = W_FG;
	valuemask = CWBackPixel|CWBorderPixel;

	win = XCALL;

	XMapWindow(display, win);
	if (isdeleted())
		return;

	setarea(&area, x, y, width+2*border_width, height+2*border_width);

	if (checkarea(display, parent, &area, W_FG, W_BG, CHECK_ALL))
		CHECK;
	else {
		report("Window created in wrong position or with wrong size");
		FAIL;
	}

	XDestroyWindow(display, win);
	CHECKPASS(1);
	
>>INCLUDE commattr.mc
>>ASSERTION Bad A
When
.A class
is
.S InputOutput
and the specified visual type and depth are not supported for the screen,
then a
.S BadMatch
error occurs.
>>STRATEGY
Find depth not supported by screen.
Set depth to the unsupported depth.
Set visual to a supported type
(doesn't seem possible to test unsupported visual portably)
Set class to InputOutput.
Attempt to create window.
Verify that BadMatch error occurs.
>>CODE BadMatch
XVisualInfo	*vp;
int 	supported;

	for (depth = 1; ; depth++) {
		supported = 0;
		for (resetvinf(VI_WIN); nextvinf(&vp); ) {
			if (vp->depth == depth)
				supported = 1;
		}
		if (!supported)
			break;
	}

	class = InputOutput;

	(void) XCALL;

	if (geterr() == BadMatch)
		PASS;
	else
		FAIL;	/* Already reported */

>>ASSERTION Bad A
When
.A class
is
.S InputOutput
and the parent window has class
.S InputOnly ,
then a
.S BadMatch
error occurs.
>>STRATEGY
Create input only window with iponlywin().
Set class to InputOutput.
Create window with the input only window as parent.
Verify that a BadMatch error occurs.
>>CODE BadMatch

	parent = iponlywin(display);
	class = InputOutput;

	(void) XCALL;

	if (geterr() == BadMatch)
		PASS;
	else
		FAIL;

>>ASSERTION Bad A
When
.A class
is
.S InputOnly
and the depth is not zero,
then a
.S BadMatch
error occurs.
>>STRATEGY
Set class to InputOnly.
Set depth to 1.
Set border_width to 0 to avoid BadMatch error due to border width.
Call xname.
Verify that BadMatch error occurs.
>>CODE BadMatch

	class = InputOnly;
	depth = 1;
	border_width = 0;

	(void) XCALL;

	if (geterr() == BadMatch)
		PASS;
	else
		FAIL;
	
>>ASSERTION Bad B 1
When
.A class
is
.S InputOnly
and the specified visual type is not supported by the screen, then a
.S BadMatch
error occurs.
>>ASSERTION Bad A
When the
.A width
or
.A height
is zero, then a
.S BadValue
error occurs.
>>STRATEGY
Set width and height in turn to be zero.
Call xname.
Verify that a BadValue error occurs.
>>CODE BadValue

	width = 0;
	height = 10;

	(void) XCALL;

	if (geterr() == BadValue)
		CHECK;
	else
		FAIL;

	width = 10;
	height = 0;

	(void) XCALL;

	if (geterr() == BadValue)
		CHECK;
	else
		FAIL;

	CHECKPASS(2);
>>ASSERTION Bad B 1
When the specified visual type is invalid, then a
.S BadValue
error occurs.
>>ASSERTION Bad A
.ER BadValue class InputOutput InputOnly CopyFromParent
>>ASSERTION Bad B 3
.ER BadAlloc

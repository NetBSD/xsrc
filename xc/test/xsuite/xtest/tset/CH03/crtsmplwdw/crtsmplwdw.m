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
 * $XConsortium: crtsmplwdw.m,v 1.11 94/04/17 21:03:00 rws Exp $
 */
>>TITLE XCreateSimpleWindow CH03
Window
XCreateSimpleWindow(display, parent, x, y, width, height, border_width, border, background)
Display *display = Dsp;
Window  parent = DefaultRootWindow(Dsp);
int 	x = 10;
int 	y = 20;
unsigned int width = 5;
unsigned int height = 5;
unsigned int border_width = 3;
unsigned long border = 1;
unsigned long background = 0;
>>ASSERTION Good A
A call to
.S XCreateSimpleWindow 
creates an unmapped
.S InputOutput 
subwindow
with a parent window of
.A parent
and returns the window ID of the created window.
>>STRATEGY
Call XCreateSimpleWindow.
Get window attributes.
Verify that map_state is IsUnmapped.
Verify that the class is InputOutput.
Call XQueryTree and check parent is parent.
>>CODE
Window	w;
XWindowAttributes atts;
Status	s;
Window	junk;
Window	father;
Window	*children;
unsigned int	nchildren;

	w = XCALL;

	s = XGetWindowAttributes(Dsp, w, &atts);
	if (s == False) {
		delete("XGetWindowAttributes failed");
		return;
	} else
		CHECK;

	trace("Window created with class %s", classname(atts.class));

	if (atts.class == InputOutput) {
		CHECK;
	} else {
		report("expected class InputOutput got %s", classname(atts.class));
		FAIL;
	}

	trace("Window created with map_state %s", mapstatename(atts.map_state));

	if (atts.map_state == IsUnmapped) {
		CHECK;
	} else {
		report("expected map_state IsUnmapped got %s", mapstatename(atts.map_state));
		FAIL;
	}

	s = XQueryTree(Dsp, w, &junk, &father, &children, &nchildren);
	if (s == False) {
		delete("XQueryTree failed");
		return;
	} else
		CHECK;

	if (father != parent) {
		report("expected parent to be %lx but was %lx", parent, father);
		FAIL;
	} else
		CHECK;

	if (nchildren != 0)
		XFree((char*)children);
	XDestroyWindow(Dsp, w);
	CHECKPASS(5);

>>ASSERTION Good A
On a call to xname a
.S CreateNotify 
event is generated on the parent window.
>>STRATEGY
Call XCreateSimpleWindow.
Verify that a CreateNotify event is received with correct values.
>>CODE event
Window	w;
XEvent	event;
XCreateWindowEvent	ge;
int 	n;

	/*
	 * Create a window as the parent for this test just to insulate
	 * us from window managers a bit.
	 */
	parent = defdraw(display, VI_WIN);
	if (isdeleted())
		return;
	else
		CHECK;

	XSelectInput(Dsp, parent, ALLEVENTS);

	w = XCALL;

	XSelectInput(Dsp, parent, NoEventMask);

	if (getevent(Dsp, &event) != 1 || event.type != CreateNotify) {
		report("Expecting one CreateNotify event");
		FAIL;
		return;
	} else
		CHECK;

	/* Set up a good event structure of what we are expecting */
	ge.type = CreateNotify;
	ge.display = display;
	ge.parent  = parent;
	ge.window  = w;
	ge.x = x;
	ge.y = y;
	ge.width   = width;
	ge.height  = height;
	ge.border_width = border_width;
	ge.override_redirect = False;

	n = checkevent((XEvent *)&ge, &event);
	if (n == 0) {
		CHECK;
	} else {
		report("error in %d field%s of event", n, (n!=1)?"s":"");
		FAIL;
	}

	XDestroyWindow(Dsp, w);
	CHECKPASS(3);

>>ASSERTION Good A
The created window is placed on top in the stacking order with respect to 
siblings.
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

	w1 = XCALL;
	w2 = XCALL;

	if ((n = stackorder(Dsp, w1)) != 0) {
		report("Stacking order for w1 was %d, expected 0", n);
		FAIL;
	} else if ((n = stackorder(Dsp, w2)) != 1) {
		report("Stacking order for w2 was %d, expected 1", n);
		FAIL;
	} else
		CHECK;

	XDestroyWindow(Dsp, w1);
	XDestroyWindow(Dsp, w2);
	CHECKPASS(2);

>>ASSERTION Good A
>># This probably shouldn't be here.
Any part of the window that extends outside its parent window is clipped.
>>STRATEGY
Create window to use as parent.
Create a subwindow on that with crechild.
Create a window that would extend outside previous window.
Set background to W_FG.
Map all windows.
Pixmap verify that window is clipped.
>>CODE
Window	top;
Window	clipwin;
Window	w;
XVisualInfo	*vp;
struct	area	area;


	for (resetvinf(VI_WIN); nextvinf(&vp); ) {
		top = (Window)makedrawable(display, vp);
		setarea(&area, 5, 5, 40, 40);
		clipwin = crechild(display, top, &area);

		parent = clipwin;
		x = 20; y = 20;
		width = 100; height = 100;
		background = W_FG;
		w = XCALL;

		XMapWindow(display, w);
		if (isdeleted())
			return;
		else
			CHECK;

		PIXCHECK(display, top);
		XDestroyWindow(Dsp, w);
	}
	CHECKPASS(2*nvinf());

>>ASSERTION Good A
The created window is not displayed.
>>STRATEGY
Create window to use as parent.
Set background to other than W_BG.
Create window with xname.
Verify that parent window is still clear.
>>CODE
XVisualInfo	*vp;
Window	w;

	for (resetvinf(VI_WIN); nextvinf(&vp); ) {
		parent = (Window)makedrawable(display, vp);

		background = W_FG;
		w = XCALL;

		if (checkclear(display, parent))
			CHECK;
		else {
			report("Created window was visible");
			FAIL;
		}
		XDestroyWindow(Dsp, w);
	}
	CHECKPASS(nvinf());

>>ASSERTION Good A
The depth, class and visual are inherited from the parent window.
>>STRATEGY
Create window.
Verify that values of depth, class and visual are same as in parent.
>>CODE
Status	s;
Window	w;
XWindowAttributes	childatts;
XWindowAttributes	parentatts;

	w = XCALL;

	s = XGetWindowAttributes(Dsp, w, &childatts);
	if (s == False) {
		delete("XGetWindowAttributes failed");
		return;
	} else
		CHECK;

	s = XGetWindowAttributes(Dsp, DefaultRootWindow(Dsp), &parentatts);
	if (s == False) {
		delete("XGetWindowAttributes failed");
		return;
	} else
		CHECK;

	if (childatts.visual != parentatts.visual) { /* XXX have to compare */
		report("Visual is different to parent");
		FAIL;
	} else if (childatts.class != parentatts.class) {
		report("Class was different to parent");
		FAIL;
	} else if (childatts.depth != parentatts.depth) {
		report("Depth was different to parent");
		FAIL;
	} else
		CHECK;

	XDestroyWindow(Dsp, w);
	CHECKPASS(3);

>>ASSERTION Good A
All window attributes, other than depth, class, visual, background
and border, 
have their default values.
>>STRATEGY
Call XCreateSimpleWindow.
Get the window attributes
Verify that these are the default values
>>CODE
Window	w;
XWindowAttributes	atts;
XWindowAttributes	parentatts;
Status	s;

	w = XCALL;
	s = XGetWindowAttributes(Dsp, w, &atts);
	if (s == False) {
		delete("XGetWindowAttributes failed");
		return;
	} else
		CHECK;

	s = XGetWindowAttributes(Dsp, DefaultRootWindow(Dsp), &parentatts);
	if (s == False) {
		delete("XGetWindowAttributes failed");
		return;
	} else
		CHECK;

	/* background-pixmap - None */
	/* border-pixmap - CopyFromParent */

	if (atts.bit_gravity != ForgetGravity) {
		report("bit_gravity was %s, expecting ForgetGravity"
			, bitgravityname(atts.bit_gravity));
		FAIL;
	} else
		CHECK;

	if (atts.win_gravity != NorthWestGravity) {
		report("win_gravity was %s, expecting NorthWestGravity"
			, wingravityname(atts.win_gravity)
			);
		FAIL;
	} else
		CHECK;

	if (atts.backing_store != NotUseful) {
		report("backing_store was %s, expecting NotUseful"
			, backingstorename(atts.backing_store));
		FAIL;
	} else
		CHECK;

	if (atts.backing_planes != 0xffffffff) {
		report("backing_planes was 0x%x, expecting all ones", atts.backing_planes);
		FAIL;
	} else
		CHECK;

	if (atts.backing_pixel != 0) {
		report("backing_pixel was 0x%x, expecting 0", atts.backing_pixel);
		FAIL;
	} else
		CHECK;

	if (atts.save_under != False) {
		report("save_under was 0x%x, expecting False", atts.save_under);
		FAIL;
	} else
		CHECK;

	if (atts.your_event_mask != NoEventMask) {
		report("your_event_mask was %s, expecting NoEventMask"
			, eventmaskname(atts.your_event_mask));
		FAIL;
	} else
		CHECK;

	if (atts.do_not_propagate_mask != NoEventMask) {
		report("do_not_propagate_mask was %s, expecting NoEventMask"
			, eventmaskname(atts.do_not_propagate_mask));
		FAIL;
	} else
		CHECK;

	if (atts.override_redirect != False) {
		report("override_redirect was %d, expecting False"
			, atts.override_redirect);
		FAIL;
	} else
		CHECK;

	if (atts.colormap != parentatts.colormap) {
		report("colormap was 0x%x, expecting 0x%x"
			, atts.colormap, parentatts.colormap);
		FAIL;
	} else
		CHECK;

	/* Cursor -- None */

	XDestroyWindow(Dsp, w);
	CHECKPASS(12);

>>ASSERTION Good B 1
The cursor is taken from the parent window.
>>STRATEGY
If extended testing is required:
  Create a parent window.
  Set the parents cursor to a non-default cursor.
  Verify that the parent's cursor was set correctly.
  Create a child window using xname.
  Map the child window.
  Warp the pointer to the child window.
  Verify that the current cursor is that of the parent.
  Set the parents cursor to a different cursor.
  Verify that the parent's cursor was set correctly.
  Verify that the current cursor has changed to that of the parent.
>>CODE
Window	w;
Cursor	pcur;

	/* If extended testing is required: */
	if(noext(0))
		return;

	pcur = makecur(display);

		/* Create a parent window. */
	parent = defwin(display);
	
		/* Set the parents cursor to a non-default cursor. */
	XDefineCursor(display, parent, pcur);

		/* Verify that the parent's cursor was set correctly. */
	if(curofwin(display, pcur, parent) == False) {
		delete("XDefineCursor() did not set the parent's window correctly.");
		return;
	} else
		CHECK;

		/* Create a child window using xname. */
	x = 10;
	y = 10;
	w = XCALL;
		/* Map the child window. */
	XMapWindow(display, w);

		/* Warp the pointer to the child window. */
	(void) warppointer(display, w, 0,0);

		/* Verify that the current cursor is that of the parent. */
	if(spriteiswin(display, parent) == False) {	
		report("The cursor used for the child window was not that of its parent.");
		FAIL;
	} else
		CHECK;

	pcur = makecur2(display);

		/* Set the parents cursor to a different cursor. */
	XDefineCursor(display, parent, pcur);

		/* Verify that the parent's cursor was set correctly. */
	if(curofwin(display, pcur, parent) == False) {
		delete("XDefineCursor() did not set the parent's window correctly.");
		return;
	} else
		CHECK;

		/* Verify that the current cursor has changed to that of the parent. */
	if(spriteiswin(display, parent) == False) {	
		report("The cursor used for the child window was not that of its parent.");
		FAIL;
	} else
		CHECK;

	CHECKPASS(4);

>>ASSERTION Bad B 1
.ER BadAlloc
>>ASSERTION Bad A
.ER BadMatch inputonly
>>ASSERTION Bad A
When
.A width
or
.A height
is zero, then a
.S BadValue
error occurs.
>>STRATEGY
Create window with width of 0
Verify that BadValue error occurs
Create window with height of 0
Verify that BadValue error occurs
Create window with height of 0 and width of 0
Verify that BadValue error occurs
>>CODE BadValue
Window w;

	/* First check for width 0 */
	width = 0;
	height = 10;

	w = XCALL;
	if (geterr() == BadValue) {
		CHECK;
	} else {
		report("Width 0 did not give BadValue.");
		FAIL;
	}

	/* Now check for height 0 */
	width = 10;
	height = 0;

	w = XCALL;

	if (geterr() == BadValue) {
		CHECK;
	} else {
		report("Height 0 did not give BadValue.");
		FAIL;
	}

	/* Now check for height 0 && width 0 */
	width = 0;
	height = 0;

	w = XCALL;

	if (geterr() == BadValue) {
		CHECK;
	} else {
		report("Height 0 and width 0 did not give BadValue.");
		FAIL;
	}

	CHECKPASS(3);

>>ASSERTION Bad A
.ER BadWindow

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
 * $XConsortium: gtgmtry.m,v 1.7 94/04/17 21:03:30 rws Exp $
 */
>>TITLE XGetGeometry CH04
Status

Display *display = Dsp;
Drawable d;
Window  *root_return = &root;
int *x_return = &x;
int *y_return = &y;
unsigned int *width_return = &width;
unsigned int *height_return = &height;
unsigned int *border_width_return = &border_width;
unsigned int *depth_return = &depth;
>>EXTERN
#define XGG_BorderWidth 5
#define XGG_X 25
#define XGG_Y 35
#define XGG_Width 20
#define XGG_Height 15

Window root;
int x;
int y;
unsigned int width;
unsigned int height;
unsigned int border_width;
unsigned int depth;
>>ASSERTION Good A
When the
.A drawable
is an
.S InputOutput
window, then a call to xname returns the root window in
.A root_return ,
the x and y coordinates of the upper-left outer corner relative to
the parent window origin in
.A x_return 
and
.A y_return ,
the inside width and height in
.A width_return
and
.A height_return ,
the border width in
.A border_width_return ,
and the depth of the window in
.A depth_return .
>>STRATEGY
For each visual and depth:
	Create a parent window
	Create a test window
	Call xname to obtain the test window geometry
	Verify that the values returned were as expected
>>CODE
XVisualInfo *vp;
struct area ap;

/* For each visual and depth: */
	for(resetvinf(VI_WIN);  nextvinf(&vp); ) {
		Window parent, w;

/* 	Create a parent window */
		ap.x = 7;
		ap.y = 11;
		ap.width = 9;
		ap.height = 13;

		parent = makewin(display, vp);

/* 	Create a test window */
		w = mkwinchild(display, vp, &ap, 0, parent, XGG_BorderWidth);

/* 	Call xname to obtain the test window geometry */
		root = (Window)0;
		x = -1;
		y = -1;
		width = 255;
		height = 255;
		border_width = 255;
		depth = 255;

		d = (Drawable)w;
		XCALL;

/* 	Verify that the values returned were as expected */
		if (root != DefaultRootWindow(display)) {
			FAIL;
			report("%s did not return the expected root window",
				TestName);
			trace("Expected root=%0x", DefaultRootWindow(display));
			trace("Returned root=%0x", root);
		} else
			CHECK;

		if (x != ap.x) {
			FAIL;
			report("%s did not return the expected x coordinate",
				TestName);
			trace("Expected x=%d", ap.x);
			trace("Returned x=%d", x);
		} else
			CHECK;
		
		if (y != ap.y) {
			FAIL;
			report("%s did not return the expected y coordinate",
				TestName);
			trace("Expected y=%d", ap.y);
			trace("Returned y=%d", y);
		} else
			CHECK;

		if (width != ap.width) {
			FAIL;
			report("%s did not return the expected width",
				TestName);
			trace("Expected width=%u", ap.width);
			trace("Returned width=%u", width);
		} else
			CHECK;

		if (height != ap.height) {
			FAIL;
			report("%s did not return the expected height",
				TestName);
			trace("Expected height=%u", ap.height);
			trace("Returned height=%u", height);
		} else
			CHECK;

		if (border_width != XGG_BorderWidth) {
			FAIL;
			report("%s did not return the expected border_width",
				TestName);
			trace("Expected border_width=%u", XGG_BorderWidth);
			trace("Returned border_width=%u", border_width);
		} else
			CHECK;

		if (depth != vp->depth) {
			FAIL;
			report("%s did not return the expected depth",
				TestName);
			trace("Expected depth=%u", vp->depth);
			trace("Returned depth=%u", depth);
		} else
			CHECK;
	}

	CHECKPASS(7*nvinf());
>>ASSERTION Good A
When the
.A drawable
is an
.S InputOnly
window, then a call to xname returns the root window in
.A root_return ,
the x and y coordinates of the upper-left outer corner relative to
the parent window origin in
.A x_return
and
.A y_return ,
the inside width and height of the window in
.A width_return
and
.A height_return ,
and sets 
.A border_width_return
and
.A depth_return
to zero.
>>STRATEGY
Create a parent window
Create an InputOnly test window
Call xname to obtain the test window geometry
Verify that the returned values were as expected
>>CODE
Window parent, w;
XSetWindowAttributes atts;

/* Create a parent window */
	parent = defwin(display);

/* Create an InputOnly test window */
	atts.override_redirect = config.debug_override_redirect;
	w = XCreateWindow(display, parent, XGG_X, XGG_Y, XGG_Width, XGG_Height,
			0, 0, InputOnly, CopyFromParent,
			CWOverrideRedirect, &atts);
	regid(display, (union regtypes *)&w, REG_WINDOW);

/* Call xname to obtain the test window geometry */
	root = (Window)0;
	x = -1;
	y = -1;
	width = 255;
	height = 255;
	border_width = 255;
	depth = 255;

	d = (Drawable)w;
	XCALL;

/* Verify that the returned values were as expected */
	if (root != DefaultRootWindow(display)) {
		FAIL;
		report("%s did not return the expected root window",
			TestName);
		trace("Expected root=%0x", DefaultRootWindow(display));
		trace("Returned root=%0x", root);
	} else
		CHECK;

	if (x != XGG_X) {
		FAIL;
		report("%s did not return the expected x coordinate",
			TestName);
		trace("Expected x=%d", XGG_X);
		trace("Returned x=%d", x);
	} else
		CHECK;
	
	if (y != XGG_Y) {
		FAIL;
		report("%s did not return the expected y coordinate",
			TestName);
		trace("Expected y=%d", XGG_Y);
		trace("Returned y=%d", y);
	} else
		CHECK;

	if (width != XGG_Width) {
		FAIL;
		report("%s did not return the expected width",
			TestName);
		trace("Expected width=%u", XGG_Width);
		trace("Returned width=%u", width);
	} else
		CHECK;

	if (height != XGG_Height) {
		FAIL;
		report("%s did not return the expected height",
			TestName);
		trace("Expected height=%u", XGG_Height);
		trace("Returned height=%u", height);
	} else
		CHECK;

	if (border_width != 0) {
		FAIL;
		report("%s did not return the expected border_width",
			TestName);
		trace("Expected border_width=%u", 0);
		trace("Returned border_width=%u", border_width);
	} else
		CHECK;

	if (depth != 0) {
		FAIL;
		report("%s did not return the expected depth",
			TestName);
		trace("Expected depth=%u", 0);
		trace("Returned depth=%u", depth);
	} else
		CHECK;

	CHECKPASS(7);
>>ASSERTION Good A
When the
.A drawable
is a
.S pixmap ,
then a call to xname returns the root window in
.A root_return ,
the inside width and height in
.A width_return
and
.A height_return ,
the depth of the pixmap in
.A depth_return ,
and sets
.A x_return ,
.A y_return ,
and 
.A border_width_return
to zero.
>>STRATEGY
For each depth:
	Create a parent window
	Create a test pixmap
	Call xname to obtain the test pixmap geometry
	Verify that the values returned were as expected
>>CODE
XVisualInfo *vp;

/* For each depth: */
	for(resetvinf(VI_PIX);  nextvinf(&vp); ) {
		Window parent;
		Pixmap pm;

/* 	Create a parent window */
		parent = makewin(display, vp);

/* 	Create a test pixmap */
		pm = XCreatePixmap(display, parent,
			XGG_Width, XGG_Height, vp->depth);
		regid(display, (union regtypes *)&pm, REG_PIXMAP);

/* 	Call xname to obtain the test pixmap geometry */
		root = (Window)0;
		x = -1;
		y = -1;
		width = 255;
		height = 255;
		border_width = 255;
		depth = 255;

		d = (Drawable)pm;
		XCALL;

/* 	Verify that the values returned were as expected */
		if (root != DefaultRootWindow(display)) {
			FAIL;
			report("%s did not return the expected root window",
				TestName);
			trace("Expected root=%0x", DefaultRootWindow(display));
			trace("Returned root=%0x", root);
		} else
			CHECK;

		if (x != 0) {
			FAIL;
			report("%s did not return the expected x coordinate",
				TestName);
			trace("Expected x=%d", 0);
			trace("Returned x=%d", x);
		} else
			CHECK;
		
		if (y != 0) {
			FAIL;
			report("%s did not return the expected y coordinate",
				TestName);
			trace("Expected y=%d", 0);
			trace("Returned y=%d", y);
		} else
			CHECK;

		if (width != XGG_Width) {
			FAIL;
			report("%s did not return the expected width",
				TestName);
			trace("Expected width=%u", XGG_Width);
			trace("Returned width=%u", width);
		} else
			CHECK;

		if (height != XGG_Height) {
			FAIL;
			report("%s did not return the expected height",
				TestName);
			trace("Expected height=%u", XGG_Height);
			trace("Returned height=%u", height);
		} else
			CHECK;

		if (border_width != 0) {
			FAIL;
			report("%s did not return the expected border_width",
				TestName);
			trace("Expected border_width=%u", 0);
			trace("Returned border_width=%u", border_width);
		} else
			CHECK;

		if (depth != vp->depth) {
			FAIL;
			report("%s did not return the expected depth",
				TestName);
			trace("Expected depth=%u", vp->depth);
			trace("Returned depth=%u", depth);
		} else
			CHECK;
	}

	CHECKPASS(7*nvinf());
>>ASSERTION Bad A
.ER BadDrawable

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
 * $XConsortium: fllplygn.m,v 1.8 94/04/17 21:05:26 rws Exp $
 */
>>TITLE XFillPolygon CH06
void

Display	*display = Dsp;
Drawable d;
GC		gc;
XPoint	*points = defpoints;
int 	npoints = sizeof(defpoints)/sizeof(XPoint);
int 	shape = Complex;
int 	mode = CoordModeOrigin;
>>EXTERN
#define E_TEST

static XPoint	defpoints[] = {
	{5, 5},
	{40, 10},
	{80, 10},
	{65, 50},
	{15, 65},
	{5, 5},
};

>>ASSERTION Good A
A call to xname
fills the region enclosed by the polygonal path
with vertex list
.A points
in drawable
.A d .
>>STRATEGY
Fill polygon
Pixmap verify.
>>CODE
XVisualInfo	*vp;

	for (resetvinf(VI_WIN_PIX); nextvinf(&vp); ) {
		d = makewin(display, vp);
		gc = makegc(display, d);

		XCALL;

		PIXCHECK(display, d);
	}

	CHECKPASS(nvinf());

>>ASSERTION Good A
When the last point in the vertex list does not coincide with the
first point, then the path is closed automatically.
>>STRATEGY
Exclude the last point in the list.
Verify the same result as having the last point coincident with the first.
>>CODE
XVisualInfo	*vp;
XImage	*xip;

	for (resetvinf(VI_WIN_PIX); nextvinf(&vp); ) {
		d = makewin(display, vp);
		gc = makegc(display, d);

		npoints = sizeof(defpoints)/sizeof(XPoint);
		XCALL;

		xip = savimage(display, d);

		npoints--;
		XCALL;

		if (compsavimage(display, d, xip))
			CHECK;
		else {
			report("Path not automatically closed properly");
			FAIL;
		}
	}

	CHECKPASS(nvinf());

>>ASSERTION Good A
A call to xname does not draw a pixel of the region more than once.
>>STRATEGY
Draw polygon.
Set function to GXxor
Draw same polygon again.
Verify that the window is blank.
>>CODE
XVisualInfo	*vp;

	for (resetvinf(VI_WIN_PIX); nextvinf(&vp); ) {
		d = makewin(display, vp);
		gc = makegc(display, d);

		XCALL;

		XSetFunction(display, gc, GXxor);

		XCALL;

		if (checkarea(display, d, (struct area *)0, W_BG, W_BG, CHECK_ALL))
			CHECK;
		else {
			report("Points drawn twice");
			FAIL;
		}
	}

	CHECKPASS(nvinf());

>>ASSERTION Good A
When
.A mode
is
.S CoordModeOrigin ,
then all coordinates are taken relative to the origin.
>>STRATEGY
Draw simple shape with origin relative co-ordinates.
Verify that it was drawn correctly.
>>CODE
XVisualInfo	*vp;
struct	area	area;
static	XPoint pnt[] = {
	{10, 10},
	{40, 10},
	{40, 30},
	{10, 30},
};

	for (resetvinf(VI_WIN_PIX); nextvinf(&vp); ) {
		d = makewin(display, vp);
		gc = makegc(display, d);

		points = pnt;
		npoints = sizeof(pnt)/sizeof(XPoint);
		mode = CoordModeOrigin;
		XCALL;

		setarea(&area, pnt[0].x, pnt[0].y, pnt[1].x-pnt[0].x, pnt[2].y-pnt[0].y);
		if (checkarea(display, d, &area, W_FG, W_BG, CHECK_ALL))
			CHECK;
		else {
			report("incorrect drawing with CoordModeOrigin");
			FAIL;
		}
	}

	CHECKPASS(nvinf());

>>ASSERTION Good A
When
.A mode
is
.S CoordModePrevious ,
then all coordinates after the first are taken relative to the previous point.
>>STRATEGY
Draw simple shape using relative coordinates.
Verify correct shape drawn.
>>CODE
XVisualInfo	*vp;
struct	area	area;
static	XPoint pnt[] = {
	{10, 10},
	{30, 0},
	{0, 20},
	{-30, 0},
};

	for (resetvinf(VI_WIN_PIX); nextvinf(&vp); ) {
		d = makewin(display, vp);
		gc = makegc(display, d);

		points = pnt;
		npoints = sizeof(pnt)/sizeof(XPoint);
		mode = CoordModePrevious;
		XCALL;

		setarea(&area, pnt[0].x, pnt[0].y, pnt[1].x, pnt[2].y);
		if (checkarea(display, d, &area, W_FG, W_BG, CHECK_ALL))
			CHECK;
		else {
			report("incorrect drawing with CoordModePrevious");
			FAIL;
		}
	}

	CHECKPASS(nvinf());

>>ASSERTION def
When
.A shape
is
.S Complex ,
and the path does not
self-intersect, then the region is filled according to the
.M fill_style .
>>ASSERTION def
When
.A shape
is
.S Complex
and the path self-intersects, then the region is filled according to the
.M fill_rule
and
.M fill_style .
>>ASSERTION Good A
When
.A shape
is
.S Nonconvex ,
and the path does not
self-intersect, then the region is filled according to the
.M fill_style .
>>STRATEGY
Set shape to Nonconvex.
Draw a non convex but non self intersecting polygon.
Pixmap verify.
>>CODE
XVisualInfo	*vp;
static	XPoint	nonconv[] = {
	{5, 5},
	{40, 10},
	{80, 10},
	{65, 50},
	{15, 65},
	{40, 35},
	{5, 5},
};

	for (resetvinf(VI_WIN_PIX); nextvinf(&vp); ) {
		d = makewin(display, vp);
		gc = makegc(display, d);

		points = nonconv;
		npoints = sizeof(nonconv)/sizeof(XPoint);
		shape = Nonconvex;
		XCALL;

		PIXCHECK(display, d);
	}

	CHECKPASS(nvinf());

>>ASSERTION Good A
When
.A shape
is
.S Convex ,
and the path is convex, then the region is filled according to the
.M fill_style .
>>STRATEGY
Set shape to Convex.
Draw convex shape.
Pixmap verify.
>>CODE
XVisualInfo	*vp;
static	XPoint	convpnts[] = {
	{10, 10},
	{80, 10},
	{70, 50},
	{20, 70},
};

	for (resetvinf(VI_WIN_PIX); nextvinf(&vp); ) {
		d = makewin(display, vp);
		gc = makegc(display, d);

		points = convpnts;
		npoints = sizeof(convpnts)/sizeof(XPoint);
		shape = Convex;
		XCALL;

		PIXCHECK(display, d);
	}

	CHECKPASS(nvinf());

>>ASSERTION gc
On a call to xname the GC components
.M function ,
.M plane-mask ,
.M fill-style ,
.M fill-rule ,
.M subwindow-mode ,
.M clip-x-origin ,
.M clip-y-origin ,
and
.M clip-mask
are used.
>>ASSERTION gc
On a call to xname the GC mode-dependent components
.M foreground ,
.M background ,
.M tile ,
.M stipple ,
.M tile-stipple-x-origin
and
.M tile-stipple-y-origin
are used.
>>ASSERTION Good A
When
.M fill_rule
is
.S EvenOddRule
and
.M fill_style
is
.S FillSolid
and
.M clip_mask
is
.S None
and an infinite ray with a given point as origin crosses
the path an odd number of times,
then a pixel is drawn at the point.
>>STRATEGY
Set EvenOddRule.
Draw self intersecting polygon.
Pixmap verify.
>>EXTERN

/*
 * A complex self intersecting shape for use in testing the fill_rule.
 * The lines have simple slopes.
 */
static XPoint	compshape[] = {
	{10, 10},
	{80, 10},
	{100, 30},
	{65, 60},
	{65, 30},
	{85, 60},
	{30, 50},
	{30, 80},
	{60, 80},
	{60, 5},
	{70, 5},
	{70, 100},
	{10, 100},
};
>>CODE
XVisualInfo	*vp;

	for (resetvinf(VI_WIN_PIX); nextvinf(&vp); ) {
		d = makewin(display, vp);
		gc = makegc(display, d);

		XSetFillRule(display, gc, EvenOddRule);

		points = compshape;
		npoints = sizeof(compshape)/sizeof(XPoint);
		shape = Complex;
		XCALL;

		PIXCHECK(display, d);
	}

	CHECKPASS(nvinf());

>>ASSERTION Good A
When
.M fill_rule
is
.S WindingRule
and
.M fill_style
is
.S FillSolid
and
.M clip_mask
is
.S None
and an infinite ray with a given point as origin crosses
an unequal number of clockwise and counterclockwise directed path segments,
then a pixel is drawn at the point.
>>STRATEGY
Set fill_rule to WindingRule.
Draw complex self-intersecting shape.
Pixmap verify.
>>CODE
XVisualInfo	*vp;

	for (resetvinf(VI_WIN_PIX); nextvinf(&vp); ) {
		d = makewin(display, vp);
		gc = makegc(display, d);

		XSetFillRule(display, gc, WindingRule);

		points = compshape;
		npoints = sizeof(compshape)/sizeof(XPoint);
		shape = Complex;
		XCALL;

		PIXCHECK(display, d);
	}

	CHECKPASS(nvinf());

>>ASSERTION def
When the center of a pixel is on the boundary,
and the boundary is not horizontal,
and the polygon interior is immediately in the x increasing direction,
then a pixel is drawn at the point.
>>ASSERTION def
When the center of a pixel is on the boundary,
and the boundary is horizontal,
and the polygon interior is immediately in the y increasing direction,
then a pixel is drawn at the point.
>>ASSERTION Bad A
.ER BadDrawable
>>ASSERTION Bad A
.ER BadGC
>>ASSERTION Bad A
.ER BadMatch inputonly
>>ASSERTION Bad A
.ER BadMatch gc-drawable-depth
>>ASSERTION Bad A
.ER BadMatch gc-drawable-screen
>>ASSERTION Bad A
.ER Value shape Complex Convex Nonconvex
>>ASSERTION Bad A
.ER Value mode CoordModeOrigin CoordModePrevious
>># HISTORY steve Completed	Written in new format and style
>># HISTORY kieron Completed	Global and pixel checking to do - 19/11/90
>># HISTORY dave Completed	Final checking to do - 21/11/90

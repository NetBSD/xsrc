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
 * $XConsortium: drwarcs.m,v 1.12 94/04/17 21:04:42 rws Exp $
 */
>>TITLE XDrawArcs CH06
void

Display	*display = Dsp;
Drawable	d;
GC		gc;
XArc	*arcs = defarcs;
int 	narcs = NELEM(defarcs);
>>EXTERN

#define	DEG(n)	(n*64)	/* Convert from degrees to units used by call */

static XArc	defarcs[] = {
	{5, 5, 80, 80, 0, DEG(360)},
	{10, 16, 70, 70, DEG(45), DEG(45)},
	{15, 14, 60, 60, DEG(93), DEG(121)},
	{15, 24, 50, 50, DEG(93), DEG(-6)},
	{30, 44, 40, 30, DEG(0), DEG(360)},
	{35, 35, 30, 10, DEG(0), DEG(180)},
};

/*
 * This will draw a straight line for horizontal and vertical lines
 * for other lines, draw a hor line using the horizontal extent.
 */
static void
drawline(ax1, ay1, ax2, ay2)
int	 ax1, ay1, ax2, ay2;
{
XArc	arc;
int 	negw = 0;
int 	negh = 0;
int 	pass = 0, fail = 0;

	arc.x = ax1; arc.y = ay1;

	if (ax2 > ax1) {
		arc.width = ax2-ax1;
	} else {
		arc.width = ax1-ax2;
		arc.x -= arc.width;
		negw = 1;
	}
	if (ay2 > ay1) {
		arc.height = ay2-ay1;
	} else {
		arc.height = ay1-ay2;
		arc.y -= arc.height;
		negh = 1;
	}

	if (arc.width == 0) {
		/* Vertical line */
		if (negh) {
			arc.angle1 = DEG(270);
			arc.angle2 = DEG(180);
		} else {
			arc.angle1 = DEG(90);
			arc.angle2 = DEG(180);
		}
	} else if (arc.height == 0) {
		/* Horizontal line */
		if (negw) {
			arc.angle1 = DEG(0);
			arc.angle2 = DEG(180);
		} else {
			arc.angle1 = DEG(180);
			arc.angle2 = DEG(180);
		}
	} else {
		/* Horizontal line */
		if (negw) {
			arc.angle1 = DEG(0);
			arc.angle2 = DEG(180);
		} else {
			arc.angle1 = DEG(180);
			arc.angle2 = DEG(180);
		}
		arc.height = 0;
	}

	arcs = &arc;
	narcs = 1;
	XCALL;
}

static void
setfordash()
{
static	XArc	darcs[] = {
	{20, 20, 50, 0, DEG(180), DEG(180)},
	{70, 20, 0, 60, DEG(90), DEG(180)},
	{30, 40, 20, 0, DEG(180), DEG(180)},
	};

	arcs = darcs;
	narcs = NELEM(darcs);
}

>>ASSERTION Good A
A call to xname draws
.A narcs
circular or elliptical arcs in the drawable
.A d ,
each specified by the corresponding
member of the
.A arcs
list.
>>STRATEGY
Draw arcs.
Pixmap verify.
>>CODE
XVisualInfo *vp;

	for (resetvinf(VI_WIN_PIX); nextvinf(&vp); ) {
		d = makewin(display, vp);
		gc = makegc(display, d);
		XCALL;
		PIXCHECK(display, d);
	}
	CHECKPASS(nvinf());

>>ASSERTION def
The centre of each circle or ellipse is the centre of the specified rectangle,
with top left corner at
.M x
and
.M y
and the major and minor axes are specified by the rectangle's
.M width
and
.M height .
>>ASSERTION def
The start of the arc is specified by the
.M angle1
component, in units of degrees * 64, relative to the three -o'clock
position from the centre.
>>ASSERTION def
The path and extent of the arc relative to the
start of the arc is specified by the
.M angle2
component, in units of degrees * 64.
>>ASSERTION Good A
When the angles are positive,
then a call to xname draws the arc in the counterclockwise direction.
>>STRATEGY
Draw arc with positive angles.
Pixmap verify.
>>CODE
XVisualInfo *vp;
static	XArc a[] = {
	{5, 5, 80, 80, DEG(45), DEG(45)},
};

	arcs = a;
	narcs = 1;

	for (resetvinf(VI_WIN_PIX); nextvinf(&vp); ) {
		d = makewin(display, vp);
		gc = makegc(display, d);
		XCALL;
		PIXCHECK(display, d);
	}
	CHECKPASS(nvinf());

>>ASSERTION Good A
When the angles are negative,
then a call to xname draws the arc in the clockwise direction.
>>STRATEGY
Call XDrawArcs.
Pixmap verify.
>>CODE
XVisualInfo *vp;
static	XArc a[] = {
	{5, 5, 80, 80, DEG(90), DEG(90)},
	{5, 5, 80, 80, DEG(90), DEG(-90)},
	{5, 5, 80, 80, DEG(-90), DEG(90)},
	{5, 5, 80, 80, DEG(-90), DEG(-90)},
};

	arcs = a;
	narcs = 4;

	for (resetvinf(VI_WIN_PIX); nextvinf(&vp); ) {
		d = makewin(display, vp);
		gc = makegc(display, d);
		XCALL;
		PIXCHECK(display, d);
	}
	CHECKPASS(nvinf());

>>ASSERTION Good A
When the magnitude of angle2 is greater than 360 degrees, 
then it is truncated to 360 degrees.
>>STRATEGY
Set gc function to GXxor.
Call XDrawArcs with angle2 greater than 360.
Pixmap check.
>>CODE
XVisualInfo *vp;
static	XArc a[] = {
	{5, 5, 80, 80, DEG(40), DEG(400)},
};

	arcs = a;
	narcs = NELEM(a);

	for (resetvinf(VI_WIN_PIX); nextvinf(&vp); ) {
		d = makewin(display, vp);
		gc = makegc(display, d);
		XSetFunction(display, gc, GXxor);
		XCALL;
		PIXCHECK(display, d);
	}
	CHECKPASS(nvinf());

>>ASSERTION def
An arc specified as %[ ~x, ~y, ~width , ~height, ~angle1, ~angle2 ]%, 
has the origin of the major and minor axes at 
% [ x +^ {width over 2} , ~y +^ {height over 2} ]%, 
the infinitely thin path describing the entire circle or ellipse 
intersects the horizontal axis at % [ x, ~y +^ {height over 2} ]% and 
% [ x +^ width , ~y +^ { height over 2 }] %
and the path intersects the vertical axis
at % [ x +^ { width over 2 } , ~y ]% and 
% [ x +^ { width over 2 }, ~y +^ height ]%.
>>ASSERTION def
When a wide line with line-width lw is used in a call to xname,
then the bounding outlines for filling are given
by the two infinitely thin paths consisting of all points whose perpendicular
distance from the path of the circle/ellipse is equal to lw/2.
>># NOTE		kieron	Arc definition will change soon, so defer.
>>ASSERTION def
The
.M cap_style
and
.M join_style
are applied the same as for a line corresponding to the tangent of the
circle/ellipse at the endpoint.
>>ASSERTION def
On a call to xname the angles are interpreted
in the effectively skewed coordinate system of the ellipse.
>>ASSERTION Good A
A call to xname does not draw a pixel more than once.
>>STRATEGY
Set gc function to GXxor.
Call XDrawArcs.
Pixmap verify.
>>CODE
XVisualInfo *vp;

	for (resetvinf(VI_WIN_PIX); nextvinf(&vp); ) {
		d = makewin(display, vp);
		gc = makegc(display, d);
		XSetFunction(display, gc, GXxor);
		XCALL;
		PIXCHECK(display, d);
	}
	CHECKPASS(nvinf());

>>ASSERTION Good A
When an arc is drawn with one endpoint and a clockwise extent
and another with the other endpoint and an equivalent counterclockwise extent,
then a call to xname draws the same pixels in each case.
>>STRATEGY
Set gc function to GXxor.
Call XDrawArcs.
Revese the sign of all angle2.
Call XDrawArcs to redraw.
Verify that drawable is clear.
>>CODE
XVisualInfo *vp;
static	XArc a[] = {
	{5, 5, 80, 80, 0, DEG(360)},
	{15, 15, 70, 70, DEG(45), DEG(45)},
	{20, 20, 60, 60, DEG(93), DEG(121)},
	{25, 25, 50, 50, DEG(93), DEG(-6)},
	{30, 30, 40, 30, DEG(0), DEG(360)},
	{35, 35, 30, 10, DEG(0), DEG(180)},
};
XArc	*ap;

	arcs = a;
	narcs = NELEM(a);

	for (resetvinf(VI_WIN_PIX); nextvinf(&vp); ) {
		d = makewin(display, vp);
		gc = makegc(display, d);
		XSetFunction(display, gc, GXxor);
		XCALL;

		for (ap = a; ap < &a[narcs]; ap++) {
			ap->angle1 += ap->angle2;
			ap->angle2 = -ap->angle2;
		}

		XCALL;

		if (!checkarea(display, d, (struct area *)0, W_BG, W_BG, CHECK_IN)) {
			report("Drawing same arcs backwards did not draw same pixels");
			FAIL;
		} else
			CHECK;
	}
	CHECKPASS(nvinf());

>>ASSERTION Good A
When the last point in one arc coincides with the first point in the following 
arc, then the two arcs will join.
>>STRATEGY
Draw arcs that meet at 90deg.
Verify that join area is filled in.
>>CODE
XVisualInfo *vp;
static	XArc a[] = {
	{10, 10, 60, 60, DEG(90), DEG(-90)},
	{40, 40, 60, 60, DEG(90), DEG(90)},
};
struct	area	area;

	arcs = a;
	narcs = NELEM(a);

	for (resetvinf(VI_WIN_PIX); nextvinf(&vp); ) {
		d = makewin(display, vp);
		gc = makegc(display, d);
		XSetFunction(display, gc, GXxor);
		XSetLineAttributes(display, gc, 5, LineSolid, CapButt, JoinMiter);

		XCALL;
		setarea(&area, 70, 40, 2, 2);
		/* Check that the miter has been drawn */
		if (checkarea(display, d, &area, W_FG, 0, CHECK_IN))
			CHECK;
		else {
			report("The two arcs did not join");
			FAIL;
		}
	}
	CHECKPASS(nvinf());

>>ASSERTION Good A
When the first point in the first arc coincides with the last point in the last 
arc, then the two arcs will join.
>>STRATEGY
Draw four arcs that meet at 90deg and join at the ends.
Pixmap verify.
>>CODE
XVisualInfo *vp;
static	XArc a[] = {
	{10, 10, 60, 60, DEG(90), DEG(-90)},
	{40, 40, 60, 60, DEG(90), DEG(90)},
	{-20, 40, 60, 60, DEG(0), DEG(90)},
	{-20, -20, 60, 60, DEG(-90), DEG(90)},
};
struct	area	area;

	arcs = a;
	narcs = NELEM(a);

	for (resetvinf(VI_WIN_PIX); nextvinf(&vp); ) {
		d = makewin(display, vp);
		gc = makegc(display, d);
		XSetFunction(display, gc, GXxor);
		XSetLineAttributes(display, gc, 5, LineSolid, CapButt, JoinMiter);

		XCALL;
		setarea(&area, 38, 8, 2, 2);
		/* Check that the miter has been drawn */
		if (checkarea(display, d, &area, W_FG, 0, CHECK_IN))
			CHECK;
		else
			FAIL;
	}
	CHECKPASS(nvinf());

>>ASSERTION Good A
When two arcs join and the
.M line_width
is greater than zero and the arcs intersect, 
then a call to xname does not draw a pixel more than once.
>>STRATEGY
Set gc function to GXxor.
Draw arcs that join.
Check that there are no holes.
>>CODE
XVisualInfo *vp;
static	XArc a[] = {
	{10, 10, 60, 60, DEG(90), DEG(-90)},
	{60, 20, 20, 20, DEG(-90), DEG(-180)},
};
XImage	*imp;

	arcs = a;
	narcs = NELEM(a);

	for (resetvinf(VI_WIN_PIX); nextvinf(&vp); ) {
		d = makewin(display, vp);
		gc = makegc(display, d);
		XSetLineAttributes(display, gc, 5, LineSolid, CapButt, JoinMiter);

		XCALL;
		imp = savimage(display, d);

		dclear(display, d);
		XSetFunction(display, gc, GXxor);

		XCALL;

		if (compsavimage(display, d, imp))
			CHECK;
		else {
			report("Pixels were drawn twice");
			FAIL;
		}
	}
	CHECKPASS(nvinf());

>>ASSERTION Good A
When either axis is zero, then a call to xname draws a horizontal or vertical
line.
>>STRATEGY
Draw arc with width zero.
Verify directly that the pixels drawn form a vertical line.
Draw arc with height zero.
Verify directly that the pixels drawn form a horizontal line.
>>CODE
XVisualInfo *vp;
static	XArc a[] = {
	{5, 5, 0, 0, DEG(0), DEG(360)},
};
struct	area	area;

	arcs = a;
	narcs = 1;

	for (resetvinf(VI_WIN_PIX); nextvinf(&vp); ) {
		d = makewin(display, vp);
		gc = makegc(display, d);
		XSetLineAttributes(display, gc, 5, LineSolid, CapButt, JoinMiter);

		a[0].width = 0;
		a[0].height = 40;

		XCALL;

		/*
		 * There should be a box width 5 height 40, with the center of
		 * the box top at 5,5; ie TLH corner at 3,5
		 */
		setarea(&area, 3, 5, 5, 40);
		if (checkarea(display, d, &area, W_FG, W_BG, CHECK_ALL))
			CHECK;
		else {
			report("A straight line was not drawn when the arc had width 0");
			FAIL;
		}
		
		a[0].width = 40;
		a[0].height = 0;

		dclear(display, d);
		XCALL;

		/*
		 * There should be a box width 40 height 5, with the center of
		 * the box LH side at 5,5; ie TLH corner at 5,3
		 */
		setarea(&area, 5, 3, 40, 5);
		if (checkarea(display, d, &area, W_FG, W_BG, CHECK_ALL))
			CHECK;
		else {
			report("A straight line was not drawn when the arc had height 0");
			FAIL;
		}

	}
	CHECKPASS(2*nvinf());

>>ASSERTION Good B 1
Angles are computed based solely on the coordinate system and ignore the
aspect ratio.
>>ASSERTION gc
On a call to xname the GC components
.M function ,
.M plane-mask ,
.M line-width ,
.M line-style ,
.M cap-style ,
.M join-style ,
.M fill-style ,
.M subwindow-mode ,
.M clip-x-origin ,
.M clip-y-origin
and
.M clip-mask
are used.
>>ASSERTION gc
On a call to xname the GC mode-dependent components
.M foreground ,
.M background ,
.M tile ,
.M stipple ,
.M tile-stipple-x-origin ,
.M tile-stipple-y-origin ,
.M dash-offset 
and
.M dash-list
are used.
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
>># HISTORY steve Completed	Written in new format and style
>># HISTORY kieron Completed	Global and pixel checking to do - 19/11/90
>># HISTORY dave Completed	Final checking to do - 21/11/90

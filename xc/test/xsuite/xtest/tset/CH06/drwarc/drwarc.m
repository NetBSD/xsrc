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
 * $XConsortium: drwarc.m,v 1.9 94/04/17 21:04:37 rws Exp $
 */
>>TITLE XDrawArc CH06
void

Display	*display = Dsp;
Drawable d;
GC		gc;
int 	x = 10;
int 	y = 10;
unsigned int 	width = 60;
unsigned int 	height = 60;
int 	angle1 = DEG(270);
int 	angle2 = DEG(321);
>>EXTERN
#define DEG(n)  (n*64)  /* Convert from degrees to units used by call */

/*
 * This will draw a straight line for horizontal and vertical lines
 * for other lines, draw a hor line using the horizontal extent.
 */
static void
drawline(ax1, ay1, ax2, ay2)
int	 ax1, ay1, ax2, ay2;
{
int 	negw = 0;
int 	negh = 0;
int 	pass = 0, fail = 0;

	x = ax1; y = ay1;

	if (ax2 > ax1) {
		width = ax2-ax1;
	} else {
		width = ax1-ax2;
		x -= width;
		negw = 1;
	}
	if (ay2 > ay1) {
		height = ay2-ay1;
	} else {
		height = ay1-ay2;
		y -= height;
		negh = 1;
	}

	if (width == 0) {
		/* Vertical line */
		if (negh) {
			angle1 = DEG(270);
			angle2 = DEG(180);
		} else {
			angle1 = DEG(90);
			angle2 = DEG(180);
		}
	} else if (height == 0) {
		/* Horizontal line */
		if (negw) {
			angle1 = DEG(0);
			angle2 = DEG(180);
		} else {
			angle1 = DEG(180);
			angle2 = DEG(180);
		}
	} else {
		/* Horizontal line */
		if (negw) {
			angle1 = DEG(0);
			angle2 = DEG(180);
		} else {
			angle1 = DEG(180);
			angle2 = DEG(180);
		}
		height = 0;
	}

	XCALL;
}

static void
setfordash()
{
	x = 20; y = 20;
	width = 49; height = 0;
	angle1 = DEG(180);
	angle2 = DEG(180);
}
>>ASSERTION Good A
A call to xname draws a single circular or elliptical arc in the drawable
.A d
as specified by
.A x ,
.A y ,
.A width ,
.A height ,
.A angle1
and
.A angle2 .
>>STRATEGY
Draw simple arc.
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
>>ASSERTION def
The centre of the circle or ellipse is the centre of the specified rectangle,
with top left corner at
.A x
and
.A y
and the major and minor axes are specified by the rectangle's
.A width
and
.A height .
>>ASSERTION def COM
The start of the arc is specified by the
.A angle1
argument, in units of degrees * 64, relative to the three -o'clock
position from the centre.
>>ASSERTION def COM
The path and extent of the arc relative to the
start of the arc is specified by the
.A angle2
argument, in units of degrees * 64.
>>ASSERTION Good A
When the angles are positive,
then a call to xname draws the arc in the counterclockwise direction.
>>STRATEGY
Draw arc with positive angle2.
Pixmap verify.
>>CODE
XVisualInfo	*vp;

	for (resetvinf(VI_WIN_PIX); nextvinf(&vp); ) {
		d = makewin(display, vp);
		gc = makegc(display, d);

		angle2 = DEG(30);
		XCALL;

		PIXCHECK(display, d);
	}

	CHECKPASS(nvinf());
>>ASSERTION Good A
When the angles are negative,
then a call to xname draws the arc in the clockwise direction.
>>STRATEGY
Draw arc with negative angle2.
Pixmap verify.
>>CODE
XVisualInfo	*vp;

	for (resetvinf(VI_WIN_PIX); nextvinf(&vp); ) {
		d = makewin(display, vp);
		gc = makegc(display, d);

		angle2 = DEG(-30);
		XCALL;

		PIXCHECK(display, d);
	}

	CHECKPASS(nvinf());
>>ASSERTION Good A
When the magnitude of angle2 is greater than 360 degrees, 
then it is truncated to 360 degrees.
>>STRATEGY
Set function to GXxor.
Draw arc with angle2 equal to 400 degrees.
Check result is same as angle2 equal to 360 degrees.
>>CODE
XVisualInfo	*vp;
XImage	*sav;

	for (resetvinf(VI_WIN_PIX); nextvinf(&vp); ) {
		d = makewin(display, vp);
		gc = makegc(display, d);

		angle2 = DEG(360);
		XCALL;
		sav = savimage(display, d);
		dclear(display, d);

		XSetFunction(display, gc, GXxor);
		angle2 = DEG(400);
		XCALL;

		if (compsavimage(display, d, sav))
			CHECK;
		else {
			report("angle2 was not truncated to 360");
			FAIL;
		}

	}

	CHECKPASS(nvinf());
>>ASSERTION def COM
An arc specified as %[ ~x, ~y, ~width , ~height, ~angle1, ~angle2 ]%, 
has the origin of the major and minor axes at 
% [ x +^ {width over 2} , ~y +^ {height over 2} ]%, 
the infinitely thin path describing the entire circle or ellipse 
intersects the horizontal axis at % [ x, ~y +^ {height over 2} ]% and 
% [ x +^ width , ~y +^ { height over 2 }] %
and the path intersects the vertical axis
at % [ x +^ { width over 2 } , ~y ]% and 
% [ x +^ { width over 2 }, ~y +^ height ]%.
>>ASSERTION def COM
When a wide line with line-width lw is used in a call to xname,
then the bounding outlines for filling are given
by the two infinitely thin paths consisting of all points whose perpendicular
distance from the path of the circle/ellipse is equal to lw/2.
>># NOTE		kieron	Arc definition will change soon, so defer.
>>ASSERTION def COM
The
.M cap_style
>>#and
>>#.M join_style
is applied the same as for a line corresponding to the tangent of the
circle/ellipse at the endpoint.
>>ASSERTION def COM
On a call to xname the angles are interpreted
in the effectively skewed coordinate system of the ellipse.
>>ASSERTION Good A
A call to xname does not draw a pixel more than once.
>>STRATEGY
Set gc function to GXcopy.
Call XDrawArc.
Set gc function to GXxor.
Call XDrawArc.
Verify that the result is the same in both cases.
>>CODE
XVisualInfo *vp;
XImage	*sav;

	for (resetvinf(VI_WIN_PIX); nextvinf(&vp); ) {
		d = makewin(display, vp);
		gc = makegc(display, d);

		XCALL;
		sav = savimage(display, d);
		dclear(display, d);

		XSetFunction(display, gc, GXxor);
		XCALL;
		if (compsavimage(display, d, sav))
			CHECK;
		else {
			report("A Pixel was drawn more than once");
			FAIL;
		}
	}
	CHECKPASS(nvinf());

>>ASSERTION Good A
When an arc is drawn with one endpoint and a clockwise extent
and another with the other endpoint and an equivalent counterclockwise extent,
then a call to xname draws the same pixels in each case.
>>STRATEGY
Draw arc and save result.
Draw arc in oposite direction.
Verify that result is the same.
>>CODE
XVisualInfo	*vp;
XImage	*sav;
int 	savang1;
int 	savang2;

	savang1 = angle1;
	savang2 = angle2;
	for (resetvinf(VI_WIN_PIX); nextvinf(&vp); ) {
		d = makewin(display, vp);
		gc = makegc(display, d);

		angle1 = savang1;
		angle2 = savang2;

		XCALL;
		sav = savimage(display, d);
		dclear(display, d);

		/* Reverse direction */
		angle1 += angle2;
		if (angle1 > DEG(360))
			angle1 -= DEG(360);
		angle2 = -angle2;

		XCALL;

		if (compsavimage(display, d, sav))
			CHECK;
		else {
			report("Drawing same arc backwards did not draw same pixels");
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
XVisualInfo	*vp;
struct	area	area;

	for (resetvinf(VI_WIN_PIX); nextvinf(&vp); ) {
		d = makewin(display, vp);
		gc = makegc(display, d);

		width = 0;
		height = 30;
		setarea(&area, x, y, 1, height);
		XCALL;

		if (checkarea(display, d, &area, W_FG, W_BG, CHECK_ALL))
			CHECK;
		else {
			report("A straight line was not drawn when the arc had width 0");
			FAIL;
		}

		width = 30;
		height = 0;
		setarea(&area, x, y, width, 1);
		dclear(display, d);
		XCALL;

		if (checkarea(display, d, &area, W_FG, W_BG, CHECK_ALL))
			CHECK;
		else {
			report("A straight line was not drawn when the arc had height 0");
			FAIL;
		}
	}

	CHECKPASS(2*nvinf());
>>ASSERTION def COM
Angles are computed based solely on the coordinate system and ignore the
aspect ratio.
>>ASSERTION gc
On a call to xname the GC components
.M function ,
.M plane-mask ,
.M line-width ,
.M line-style ,
.M cap-style ,
>># This can not be right?
>># .M join-style ,
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
>># HISTORY steve Removed references to join-style.

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
 * $XConsortium: drwlns.m,v 1.12 94/04/17 21:04:55 rws Exp $
 */
>>TITLE XDrawLines CH06
void
XDrawLines(display, d, gc, points, npoints, mode)
Display	*display = Dsp;
Drawable	d;
GC  	gc;
XPoint	*points = defpoints;
int 	npoints = ndefpoints;
int 	mode = CoordModeOrigin;
>>EXTERN
/* These points will be used if we are not very interested in what is used */
static 	XPoint	defpoints[] = {
	{5, 35},
	{20, 30},
	{10, 5},
	{70, 40},
	{20, 50},
};
static	int 	ndefpoints = sizeof(defpoints)/sizeof(XPoint);

/* Lines that cross */
static XPoint	crosspts[] = {
	{10, 10},
	{30, 15},
	{15, 40},
	{20, 3},
	{25, 40},
	{10, 28},
	{30, 28},
};

static void
drawline(x1, y1, x2, y2)
int 	x1, y1, x2, y2;
{
XPoint	pnts[2];
int 	pass = 0, fail = 0;

	pnts[0].x = x1;
	pnts[0].y = y1;
	pnts[1].x = x2;
	pnts[1].y = y2;

	points = pnts;
	npoints = 2;

	XCALL;
}

void
setfordash()
{
static XPoint pnts[] = {
	{20, 20},
	{70,  20},
	{70,  80},
	};
	points = pnts;
	npoints = NELEM(pnts);
}

>>ASSERTION Good A
A call to xname
draws
.A npoints \-1
lines between each pair of points (point[i], point[i+1]) 
in 
.A points
in the drawable
.A d .
>>STRATEGY
Draw line with multiple segments.
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

>>ASSERTION Good B 1
A call to xname draws the lines in the order listed in the array.
>>ASSERTION Good A
The lines join at all intermediate points.
>>STRATEGY
Set line-width component of GC to 6.
Set line-style component of GC to LineSolid.
Set cap-style component of GC to CapButt.
Set join-style component of GC to JoinRound.
Draw line with multiple segments.
Pixmap verify.
(Other combinations of GC components are tested under join_style assertions).
>>CODE
XVisualInfo	*vp;

	for (resetvinf(VI_WIN_PIX); nextvinf(&vp); ) {

		d = makewin(display, vp);
		gc = makegc(display, d);

		/*
		 * Just try some defaults here, different join styles
		 * tested elsewhere
		 */
		XSetLineAttributes(display, gc, 6, LineSolid, CapButt, JoinRound);

		XCALL;
		PIXCHECK(display, d);
	}
	CHECKPASS(nvinf());

>>ASSERTION Good A
When the first and last points coincide, then the first and last
lines join.
>>STRATEGY
Set line-width component of GC to 4.
Set line-style component of GC to LineSolid.
Set cap-style component of GC to CapButt.
Set join-style component of GC to JoinRound.
Draw line with multiple segments and coincident end points.
Pixmap verify.
>>CODE
XVisualInfo	*vp;
static XPoint	pnts[] = {
	{6, 6},
	{20, 8},
	{10, 30},
	{6, 6},
};

	for (resetvinf(VI_WIN_PIX); nextvinf(&vp); ) {
		d = makewin(display, vp);
		gc = makegc(display, d);

		points = pnts;
		npoints = sizeof(pnts)/sizeof(XPoint);
		XSetLineAttributes(display, gc, 4, LineSolid, CapButt, JoinRound);

		XCALL;
		PIXCHECK(display, d);
	}
	CHECKPASS(nvinf());

>>ASSERTION Good A
A call to xname does not draw each pixel of a particular line
more than once.
>>STRATEGY
Set function component of GC to GXxor.
Draw line with multiple segments.
Pixmap verify.
>>CODE
XVisualInfo	*vp;

	for (resetvinf(VI_WIN_PIX); nextvinf(&vp); ) {

		d = makewin(display, vp);
		gc = makegc(display, d);
		XSetFunction(display, gc, GXxor);

		XCALL;
		PIXCHECK(display, d);
	}
	CHECKPASS(nvinf());

>>ASSERTION Good A
When thin lines intersect, then
the intersecting pixels are drawn multiple times.
>>STRATEGY
Set function component of GC to GXxor.
Set line-width component of GC to 0.
Set line-style component of GC to LineSolid.
Set cap-style component of GC to CapButt.
Set join-style component of GC to JoinRound.
Draw line with horizontal and vertical segments crossing.
Verify pixel at crossing point is set to background pixel value.
>>CODE
/*
 * Horizontal and vertical lines that cross at 15, 20.
 * Strictly the spec does not even require that horizontal and
 * vertical thin lines are drawn properly.
 */
static XPoint thincross[] = {
	{5, 20},
	{30, 20},
	{15, 40},
	{15, 5},
};
XVisualInfo	*vp;
struct	area	area;

	for (resetvinf(VI_WIN_PIX); nextvinf(&vp); ) {

		d = makewin(display, vp);
		gc = makegc(display, d);
		XSetLineAttributes(display, gc, 0, LineSolid, CapButt, JoinRound);

		points = thincross;
		npoints = sizeof(thincross)/sizeof(XPoint);

		setarea(&area, 15, 20, 1, 1);
		/*
		 * The only purpose of this check is to show that the pixel
		 * is indeed drawn when the function is copy.  This verifies
		 * that we are looking at the right pixel.
		 */
		XSetFunction(display, gc, GXcopy);
		XCALL;
		if (checkarea(display, d, &area, W_FG, 0, CHECK_IN)) {
			CHECK;
		} else {
			delete("Intersecting point was not set with GXcopy");
			return;
		}
		dclear(display, d);

		XSetFunction(display, gc, GXxor);
		XCALL;
		if (checkarea(display, d, &area, W_BG, 0, CHECK_IN) == True)
			CHECK;
		else {
			report("intersecting pixel not drawn multiple times with thin line");
			FAIL;
		}

	}

	CHECKPASS(2*nvinf());
	
>>ASSERTION Good A
When wide lines intersect, then the intersecting pixels are drawn only once.
>>STRATEGY
Set function component of GC to GXxor.
Set line-width component of GC to 3.
Set line-style component of GC to LineSolid.
Set cap-style component of GC to CapButt.
Set join-style component of GC to JoinRound.
Draw line with horizontal and vertical segments crossing.
Pixmap verify.
>>CODE
XVisualInfo	*vp;

	for (resetvinf(VI_WIN_PIX); nextvinf(&vp); ) {

		d = makewin(display, vp);
		gc = makegc(display, d);
		XSetFunction(display, gc, GXxor);
		XSetLineAttributes(display, gc, 3, LineSolid, CapButt, JoinRound);

		points = crosspts;
		npoints = sizeof(crosspts)/sizeof(XPoint);

		XCALL;
		PIXCHECK(display, d);
	}
	CHECKPASS(nvinf());

>>ASSERTION def
>># The required test is in the first test purpose.
When
.A mode
is
.S CoordModeOrigin ,
then all coordinates are taken relative to the origin.
>>ASSERTION Good A
When
.A mode
is
.S CoordModePrevious ,
then all coordinates after the first are taken relative to the previous point.
>>STRATEGY
Set mode to CoordModePrevious.
Draw line with multiple segments.
Pixmap verify.
>>CODE
XVisualInfo	*vp;

	for (resetvinf(VI_WIN_PIX); nextvinf(&vp); ) {

		d = makewin(display, vp);
		gc = makegc(display, d);

		mode = CoordModePrevious;

		XCALL;
		PIXCHECK(display, d);
	}
	CHECKPASS(nvinf());

>>ASSERTION gc
On a call to xname the GC components
.M function ,
.M plane-mask ,
.M line-width ,
.M line-style ,
.M cap-style ,
.M fill-style ,
.M subwindow-mode ,
.M clip-x-origin ,
.M clip-y-origin ,
.M join-style
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
>>ASSERTION Bad A
.ER Value mode CoordModeOrigin CoordModePrevious
>># HISTORY steve Completed	Written in new format and style
>># HISTORY kieron Completed	Global and pixel checking to do - 19/11/90
>># HISTORY dave Completed	Final checking to do - 21/11/90

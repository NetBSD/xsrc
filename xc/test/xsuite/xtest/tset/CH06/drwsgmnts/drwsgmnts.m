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
 * $XConsortium: drwsgmnts.m,v 1.7 94/04/17 21:05:09 rws Exp $
 */
>>TITLE XDrawSegments CH06
void

Display	*display = Dsp;
Drawable	d;
GC		gc;
XSegment *segments = defsegs;
int 	nsegments = NSEGS;
>>EXTERN

XSegment	defsegs[] = {
	{10, 10, 50, 40},
	{25, 7, 60, 15},
	{10, 10, 25, 7},
	{10, 40, 40, 10},
};

#define	NSEGS	2	/* Number of segments that don't touch */
#define NSEGSJOIN 3	/*   + extra segment to test joins */
#define NSEGSCROSS 4/*   + extra segment to test intersecting lines */

static void
drawline(ax1, ay1, ax2, ay2)
int 	ax1, ay1, ax2, ay2;
{
XSegment segs[1];
int 	pass = 0, fail = 0;

	segs[0].x1 = ax1; segs[0].y1 = ay1;
	segs[0].x2 = ax2; segs[0].y2 = ay2;
	segments = segs;
	nsegments = 1;
	XCALL;
}

static void
setfordash()
{
static	XSegment segs[2];
	segs[0].x1 = 20; segs[0].y1 = 20;
	segs[0].x2 = 70; segs[0].y2 = 20;

	segs[1].x1 = 80; segs[1].y1 = 20;
	segs[1].x2 = 80; segs[1].y2 = 80;
	segments = segs;
	nsegments = 2;
}

>>ASSERTION Good A
A call to xname
draws
.A nsegments
lines between the points
.A "" ( x1 , y1 )
and
.A "" ( x2 , y2 )
in 
.A segments
in the drawable
.A d .
>>STRATEGY
Draw segments.
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
A call to xname draws the lines in the order listed in the array of
.S XSegment 
structures.
>>ASSERTION Good A
A call to xname does not perform joining at coincident endpoints.
>>STRATEGY
Set line-width component of GC to 4.
Set line-style component of GC to LineSolid.
Set cap-style component of GC to CapButt.
Set join-style component of GC to JoinRound.
Draw segments which include joins.
Pixmap verify.
>>CODE
XVisualInfo	*vp;

	for (resetvinf(VI_WIN_PIX); nextvinf(&vp); ) {
		d = makewin(display, vp);
		gc = makegc(display, d);
		nsegments = NSEGSJOIN;
		XSetLineAttributes(display, gc, 4, LineSolid, CapButt, JoinRound);
		XCALL;
		PIXCHECK(display, d);
	}
	CHECKPASS(nvinf());
>>ASSERTION Good A
A call to xname does not draw each pixel for a particular line more than once.
>>STRATEGY
Set function component of GC to GXxor.
Draw segments.
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
When lines intersect, then the intersecting pixels are drawn multiple times.
>>STRATEGY
Set function component of GC to GXxor.
Set line-width component of GC to 3.
Set line-style component of GC to LineSolid.
Set cap-style component of GC to CapButt.
Set join-style component of GC to JoinRound.
Draw segments which include joins and intersections.
Pixmap verify.
>>CODE
XVisualInfo	*vp;

	for (resetvinf(VI_WIN_PIX); nextvinf(&vp); ) {
		d = makewin(display, vp);
		gc = makegc(display, d);
		XSetFunction(display, gc, GXxor);
		XSetLineAttributes(display, gc, 3, LineSolid, CapButt, JoinRound);
		nsegments = NSEGSCROSS;
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
.ER Match gc-drawable-depth
>>ASSERTION Bad A
.ER BadMatch gc-drawable-screen
>># HISTORY steve Completed	Written in new format and style
>># HISTORY kieron Completed	Global and pixel checking to do - 19/11/90
>># HISTORY dave Completed	Final checking to do - 21/11/90

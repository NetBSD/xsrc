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
 * $XConsortium: drwln.m,v 1.5 94/04/17 21:04:49 rws Exp $
 */
>>TITLE XDrawLine CH06
void
XDrawLine(display, d, gc, x1, y1, x2, y2);
Display		*display = Dsp;
Drawable	d;
GC		gc;
int 	x1 = 10;
int 	y1 = 10;
int 	x2 = 31;
int 	y2 = 23;
>>EXTERN
static void
setfordash()
{
	x1 = 20; y1 = 20;
	x2 = 70; y2 = 20;
}
static void
drawline(ax1, ay1, ax2, ay2)
int 	ax1, ay1, ax2, ay2;
{
int 	pass = 0, fail = 0;

	x1 = ax1; y1 = ay1;
	x2 = ax2; y2 = ay2;
	XCALL;
}
	
>>ASSERTION Good A
A call to xname
draws a line between the pair of points
.A "" ( x1 , y1 )
and
.A "" ( x2 , y2 )
in the drawable
.A d .
>>STRATEGY
Draw line
Pixmap check
>>CODE
XVisualInfo *vp;

	for (resetvinf(VI_WIN_PIX); nextvinf(&vp); ) {

		d = makewin(display, vp);
		gc = makegc(display, d);

		XCALL;
		PIXCHECK(display, d);
	}
	CHECKPASS(nvinf());

>>ASSERTION Good A
A call to xname does not perform joining at coincident endpoints.
>>STRATEGY
Draw line
Draw line with one end point the same
Pixmap check
>>CODE
XVisualInfo *vp;

	for (resetvinf(VI_WIN_PIX); nextvinf(&vp); ) {

		x1 = 10; y1 = 10;
		x2 = 31; y2 = 23;

		d = makewin(display, vp);
		gc = makegc(display, d);
		XSetLineAttributes(display, gc, 6, LineSolid, CapButt, JoinMiter);

		XCALL;

		x1 = x2;
		y1 = y2;
		x2 = 10;
		y2 = 25;


		XCALL;

		PIXCHECK(display, d);
	}
	CHECKPASS(nvinf());

>>ASSERTION Good A
A call to xname does not draw a pixel more than once.
>>STRATEGY
Set function to GXxor
Draw line
Pixmap check
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
When lines intersect, then the intersecting pixels are drawn multiple times.
>>STRATEGY
Draw line
Set function to GXxor
Draw another intersecting line
Pixmap check
>>CODE
XVisualInfo *vp;

	for (resetvinf(VI_WIN_PIX); nextvinf(&vp); ) {

		x1 = 10; y1 = 10;
		x2 = 31; y2 = 23;

		d = makewin(display, vp);
		gc = makegc(display, d);
		XSetLineAttributes(display, gc, 6, LineSolid, CapButt, JoinMiter);

		XCALL;

		XSetFunction(display, gc, GXxor);

		x1 = 10; y1 = 30;
		x2 = 30; y2 = 10;

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
.M  clip-y-origin ,
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
.M dash-offset ,
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

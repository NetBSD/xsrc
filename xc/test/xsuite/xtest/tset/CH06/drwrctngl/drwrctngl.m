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
 * $XConsortium: drwrctngl.m,v 1.10 94/04/17 21:05:02 rws Exp $
 */
>>TITLE XDrawRectangle CH06
void

Display	*display = Dsp;
Drawable	d;
GC		gc;
int 	x = 10;
int 	y = 7;
unsigned int 	width = 60;
unsigned int 	height = 34;
>>EXTERN
#ifdef MIN 
#undef MIN
#endif
#define MIN(a,b) (((a)<(b)) ? (a) : (b))

#ifdef MAX 
#undef MAX
#endif
#define MAX(a,b) (((a)>(b)) ? (a) : (b))

static void
drawline(ax1, ay1, ax2, ay2)
int 	ax1, ay1, ax2, ay2;
{
int 	pass = 0, fail = 0;
int	x2 = MAX(ax1,ax2), y2 = MAX(ay1,ay2);
	/*
	 * Draw a rectangle that has the two given points as vertices.
	 * Some tests then do not apply or need slight modification.
	 * Sort them as the protocol specifies rects as top-left and w & h
	 * so drawing a rectangle "in reverse" isn't possible.
	 */
	x = MIN(ax1,ax2); y = MIN(ay1,ay2);
	width = x2 - x;
	height = y2 - y;
	XCALL;
}
#undef MIN
#undef MAX

static void
setfordash()
{
	x = 20; y = 20;
	width = 50; height = 50;
}
>>ASSERTION Good A
A call to xname draws the outline of the rectangle specified by
.A x ,
.A y ,
.A width
and 
.A height
in the drawable
.A d .
>>STRATEGY
Draw rectangle.
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
A call to xname draws the rectangle as if a five-point PolyLine protocol
request were specified for the rectangle in the order
.S "" [ x , y "], "
.S "" [ x+width , y "], "
.S "" [ x+width , y+height "], "
.S "" [ x , y+height "], "
.S "" [ x , y "]."
>># NOTE	kieron	Testing ordering again...????
>>ASSERTION def
All four
corners of the rectangle join.
>>ASSERTION Good A
A call to xname does not draw a pixel more than once.
>>STRATEGY
Set function to GXxor
Draw rectangle
Pixmap verify
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

>>ASSERTION gc
On a call to xname the GC components
.M function ,
.M plane-mask ,
.M line-width ,
.M line-style ,
.M join-style ,
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
.ER BadMatch gc-drawable-depth
>>ASSERTION Bad A
.ER BadMatch gc-drawable-screen
>># HISTORY steve Completed	Written in new format and style
>># HISTORY kieron Completed	Global and pixel checking to do - 19/11/90
>># HISTORY dave Completed	Final checking to do - 21/11/90

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
 * $XConsortium: ststt.m,v 1.9 94/04/17 21:04:22 rws Exp $
 */
>>TITLE XSetState CH05
void
XSetState(display, gc, foreground, background, function, plane_mask)
Display *display = Dsp;
GC gc;
unsigned long foreground = W_FG;
unsigned long background = W_BG;
int function = GXcopy;
unsigned long plane_mask = AllPlanes;
>>SET need-gc-flush
>>ASSERTION Good A
A call to xname sets the 
.M foreground ,
.M background , 
.M plane_mask
and 
.M function
components of the specified GC to the values of the
.A foreground ,
.A background , 
.A plane_mask
and 
.A function
arguments.
>>STRATEGY
*Test background.
Create window.
Create GC with LineStyle = LineDoubleDash, bg = BlackPixel , fg = WhitePixel, fn = GXcopy, line_width = 1
Draw a horizonal line (sufficiently long for even and odd dashes) 
Verify pixel at (1, 0) is bg with XGetImage and XGetPixel.
Set gc bg component to fg value using XSetState.
Redraw same line (should fill in the gaps exactly)
Verify pixel at (1, 0) is fg with XGetImage and XGetPixel.

*Test foreground.
Create window.
Create GC with bg = BlackPixel, fg = WhitePixel, fn = GXcopy.
Set pixel at (0, 0) with XDrawPoint.
Verify pixel at (0, 0) is fg with XGetImage and XGetPixel.
Set fg component of GC to bg with XSetState.
Set pixel at (0, 0) with XDrawPoint.
Verify pixel at (0, 0) is bg with XGetImage and XGetPixel.

*Test plane_mask.
Create window.
Create GC with bg = BlackPixel, fg = WhitePixel, plane_mask = 0.
Verify pixel at (0, 0) is bg with XGetImage and XGetPixel.
Set pixel at (0, 0) with XDrawPoint.
Verify pixel at (0, 0) is bg with XGetImage and XGetPixel.
Set plane_mask component of GC to AllPlanes with XSetState.
Set pixel at (0, 0) with XDrawPoint.
Verify pixel at (0, 0) is fg with XGetImage and XGetPixel.

*Test function.
Create window.
Create GC with bg = BlackPixel, fg = WhitePixel, fn = GXcopy.
Set pixel at (0, 0) to WhitePixel with XDrawPoint.
Verify pixel at (0, 0) is WhitePixel with XGetImage and XGetPixel.
Set fg to (BlackPixel xor WhitePixel)
Set fn component of GC to GXxor with XSetState.
Set pixel at (0, 0) to  fg with XDrawPoint.
Verify pixel at (0, 0) is BlackPixel with XGetImage and XGetPixel.
>>CODE
XVisualInfo *vp;
XGCValues values;
Window win;

	resetvinf(VI_WIN);
	nextvinf(&vp);

	/* Test background */

	win = makewin(display, vp);
	values.function = GXcopy;
	values.foreground = W_FG;
	values.background = W_BG;
	values.line_style = LineDoubleDash;
	values.line_width = 1;
	values.dashes = 1; /* [1, 1] */
	gc = XCreateGC(display, win, (GCFunction | GCLineStyle | GCDashList | GCForeground | GCBackground | GCLineWidth), &values);

	XDrawLine(display, win, gc, 0, 0, 2, 0);

	if( ! checkpixel(display, win, 1, 0, W_BG)){
		delete("Pixel at (1, 0) was not set to W_BG.");
		return;
	} else
		CHECK;
	
	background = W_FG;
	XCALL;
	background = W_BG;

	XDrawLine(display, win, gc, 0, 0, 2, 0);
	if( ! checkpixel(display, win, 1, 0, W_FG)){
		report("Pixel at (1, 0) was not set to W_FG.");
		FAIL;
	} else
		CHECK;
	
	/* Test foreground */

	win = makewin(display, vp);
	values.foreground = W_FG;
	values.background = W_BG;
	gc = XCreateGC(display, win, GCForeground | GCBackground, &values);

	XDrawPoint(display, win, gc, 0, 0);

	if( ! checkpixel(display, win, 0, 0, W_FG)) {
		delete("Pixel at (0, 0) was not set to W_FG.");
		return;
	} else
		CHECK;

	foreground = W_BG;
	XCALL;
	foreground = W_FG;

	XDrawPoint(display, win, gc, 0, 0);

	if( ! checkpixel(display, win, 0, 0, W_BG)) {
		report("Pixel at (0, 0) was not set to W_BG.");
		FAIL;
	} else
		CHECK;

	/* Test plane_mask */

	win = makewin(display, vp);
	values.foreground = W_FG;
	values.background = W_BG;
	values.plane_mask = 0;

	gc = XCreateGC(display, win, (GCPlaneMask | GCForeground | GCBackground), &values);

	if( ! checkpixel(display, win, 0, 0, W_BG)) {
		delete("Pixel at (0, 0) was not set to W_BG.");
		return;
	} else
		CHECK;

	XDrawPoint(display, win, gc, 0, 0);

	if( ! checkpixel(display, win, 0, 0, W_BG)) {
		delete("Pixel at (0, 0) was not left as W_BG.");
		return;
	} else
		CHECK;

	plane_mask = AllPlanes;
	XCALL;

	XDrawPoint(display, win, gc, 0, 0);
	if( ! checkpixel(display, win, 0, 0, W_FG)) {
		report("Pixel at (0, 0) was not set to W_FG.");
		FAIL;
	} else
		CHECK;

	/* Test function */

	win = makewin(display, vp);
	values.foreground = W_FG;
	values.background = W_BG;
	gc = XCreateGC(display, win, GCForeground | GCBackground, &values);

	XDrawPoint(display, win, gc, 0, 0);

	if( ! checkpixel(display, win, 0, 0, W_FG)) {
		delete("Pixel at (0, 0) was not set to W_FG.");
		return;
	} else
		CHECK;

	values.foreground = W_FG ^ W_BG;
	XSetForeground(display, gc, values.foreground);

	function = GXxor;
	XCALL;
	function = GXcopy;

	XDrawPoint(display, win, gc, 0, 0);

	if( ! checkpixel(display, win, 0, 0, W_BG)) {
		report("Pixel at (0, 0) was not set to W_BG.");
		FAIL;
	} else
		CHECK;
	
	CHECKPASS(9);

>>ASSERTION Bad B 1
.ER Alloc
>>ASSERTION Bad A
.ER GC
>>ASSERTION Bad A
.ER Value function GXclear GXand GXandReverse GXcopy GXandInverted GXnoop GXxor GXor GXnor GXequiv GXinvert GXorReverse GXcopyInverted GXorInverted GXnand GXset
>># HISTORY cal Completed	Written in new format and style
>># HISTORY kieron Completed	Global and pixel checking to do - 19/11/90
>># HISTORY dave Completed	Final checking to do - 21/11/90
>># HISTORY cal Action		Writing code.

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
 * $XConsortium: stfllstyl.m,v 1.6 94/04/17 21:04:09 rws Exp $
 */
>>TITLE XSetFillStyle CH05
void
XSetFillStyle(display, gc, fill_style)
Display *display = Dsp;
GC gc;
int fill_style = FillStippled;
>>SET need-gc-flush
>>ASSERTION Good A
A call to xname sets the
.M fill_style
component of the specified GC to the value of the
.A fill_style
argument.
>>STRATEGY
Create a window.
Create GC with fill_style = FillSolid, stipple = {0} , fg = WhitePixel, bg = BlackPixel.
Draw a filled rectangle (0, 0) (1, 1) with XFillRectangle.
Verify pixel at (0, 0) is fg with XGetImage and XGetPixel.
Set fill style to FillOpaqueStippled with XSetFillStyle.
Draw a filled rectangle (0, 0) (1, 1) with XFillRectangle.
Verify pixel at (0, 0) is bg with XGetImage and XGetPixel.
>>CODE
XVisualInfo	*vp;
Window		win;
XGCValues	values;
Pixmap		stip;
GC		sgc;

	resetvinf(VI_WIN);
	nextvinf(&vp);
	win = makewin(display, vp);
	stip = XCreatePixmap( display, win, 1, 1, 1); 	
	values.foreground = 0;
	sgc = XCreateGC(display, stip, GCForeground, &values);
	XDrawPoint(display, stip, sgc, 0, 0);

	values.foreground = W_FG;
	values.background = W_BG;
	values.fill_style = FillSolid;
	values.stipple = stip;

	gc = XCreateGC(display, win, GCForeground | GCBackground | GCFillStyle | GCStipple, &values);
	XFillRectangle(display, win, gc, 0, 0, 1, 1);

	if( ! checkpixel(display, win, 0, 0, W_FG)) {
		delete("Pixel at (0, 0) was not set to foreground.");
		return;
	} else 
		CHECK;

	fill_style = FillOpaqueStippled;
	XCALL;

	XFillRectangle(display, win, gc, 0, 0, 1, 1);

	if( ! checkpixel(display, win, 0, 0, W_BG)) {
		report("Pixel at (0, 0) was not set to background.");
		FAIL;
	} else 
		CHECK;
	
	CHECKPASS(2);

>>ASSERTION Bad B 1
.ER Alloc
>>ASSERTION Bad A
.ER GC
>>ASSERTION Bad A
.ER Value fill_style FillSolid FillTiled FillStippled FillOpaqueStippled
>># HISTORY cal Completed	Written in new format and style
>># HISTORY kieron Completed	Global and pixel checking to do - 19/11/90
>># HISTORY dave Completed	Final checking to do - 21/11/90
>># HISTORY cal Action		Writing code.

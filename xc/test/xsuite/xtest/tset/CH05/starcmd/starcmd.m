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
 * $XConsortium: starcmd.m,v 1.6 94/04/17 21:04:03 rws Exp $
 */
>>TITLE XSetArcMode CH05
void
XSetArcMode(display, gc, arc_mode)
Display *display = Dsp;
GC gc;
int arc_mode = ArcChord;
>>SET need-gc-flush
>>ASSERTION Good A
A call to xname sets the
.M arc_mode
component of the specified GC to that specified by the
.A arc_mode
argument.
>>STRATEGY
Create window.
Create GC with arc_mode = ArcPieSlice, fill_style = FillSolid, fg = WhitePixel, bg = BlackPixel.
Draw a filled arc from 270 to 90 using XFillArc.
Verify (arc origin-1, arc origin-1) is set to bg.
Set arc_mode to ArcChord with XSetArcMode.
Draw a filled arc from 270 to 90 using XFillArc.
Verify (arc origin-1, arc origin-1) is set to fg.
>>CODE
XVisualInfo *vp;
Window	win;
XGCValues values;
struct area ar;

	resetvinf(VI_WIN);
	nextvinf(&vp);
	win = makewin(display, vp);

	values.foreground = W_FG;
	values.background = W_BG;
	values.arc_mode = ArcPieSlice;
	values.fill_style = FillSolid;

	gc = XCreateGC(display, win, GCForeground | GCBackground | GCArcMode | GCFillStyle, &values);

	XFillArc(display, win, gc, 0, 0, 40, 40, 90*64, -270*64);

	if( ! checkpixel(display, win, 19, 19, W_BG) ) {
		report("Pixel at (19, 19) was not set to background.");
		return;
	} else
		CHECK;

	arc_mode = ArcChord;
	XCALL;

	XFillArc(display, win, gc, 0, 0, 40, 40, 90* 64, - 270 * 64);

	if( ! checkpixel(display, win, 19, 19, W_FG) ) {
		report("Pixel at (19, 19) was not set to foreground.");
		FAIL;
	} else
		CHECK;

	CHECKPASS(2);

>>ASSERTION Bad B 1
.ER Alloc
>>ASSERTION Bad A
.ER GC
>>ASSERTION Bad A
.ER Value arc_mode ArcChord ArcPieSlice
>># HISTORY cal Completed	Written in new format and style
>># HISTORY kieron Completed	Global and pixel checking to do - 19/11/90
>># HISTORY dave Completed	Final checking to do - 21/11/90
>># HISTORY cal	Action		Writing code.

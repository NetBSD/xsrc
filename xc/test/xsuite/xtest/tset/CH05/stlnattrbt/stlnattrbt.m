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
 * $XConsortium: stlnattrbt.m,v 1.6 94/04/17 21:04:12 rws Exp $
 */
>>TITLE XSetLineAttributes CH05
void
XSetLineAttributes(diplay, gc, line_width, line_style, cap_style, join_style)
Display *display = Dsp;
GC gc;
unsigned int line_width;
int line_style;
int cap_style;
int join_style;
>>SET need-gc-flush
>>ASSERTION Good A
A call to xname sets the 
.M line_width ,
.M line_style ,
.M cap_style
and
.M join_style
components of the specified GC to the values of the
.A line_width ,
.A line_style ,
.A cap_style
and
.A join_style
arguments.
>>STRATEGY

*Line_Width
Create a window.
Create a GC with line_width = 1, fg = WhitePixel, bg = BlackPixel.
Draw horizontal line from (0, 1) to (0, 2) using XDrawLine.
Verify pixel at 0, 2 is set to bg.
Set line_width component of GC to 2 with XSetLineAttributes.
Draw horizontal line from (0, 1) to (2, 1) using XDrawLine.
Verify pixel at 0, 2 is set to WhitePixel using XGetimage and XGetPixel.

*Line_Style
Create a window.
Create GC with line_style = LineDoubleDash, fg = WhitePixel, bg = BlackPixel, dashes = 1, fn = GXcopy.
Draw line from (0, 0) to (1, 0) with XDrawLines.
Draw dashed line using XDrawLine.
Verify pixel at (1, 0) is bg with XGetImage and XGetPixel
Set GC line_style component to LineSolid with XSetLineAttributes.
Redraw same line.
Verify pixel is set to bg.

*Cap_Style
Create a window.
Create GC with cap_style = CapButt, fg = WhitePixel, bg = BlackPixel, line_width = 0.
Draw lines from (0, 0) to (1, 0) using XDrawLines.
Verify pixel at (2, 0) is bg using XGetImage and XGetPixel.
Set cap_style component of GC to CapProjecting.
Draw line from (0, 0) to (1, 0) using XDrawLines.
Verify pixel at (2, 0) is fg using XGetImage and XGetPixel.

*Join_Style
Create a Window.
Create GC with line_width = 3, Join_style = JoinBevel, bg =BlackPixel, fg = WhitePixel, fn = GXxor.
Draw two joined lines (0, 1) , (2, 1) , (2, 3) using XDrawLines.
Verify Pixel at (0, 4) is bg using XGetImage and XGetPixel.
Set join_style component of GC to JoinMiter using XSetLineAttributes.
Draw two joined lines (0, 1) , (2, 1) , (2, 3) using XDrawLines.
Verify Pixel at (0, 4) is fg using XGetImage and XGetPixel.
>>EXTERN
XPoint Jpoints[] = { {0, 1}, {2, 1}, {2, 3} };
>>CODE
XVisualInfo *vp;
XGCValues values;
GC gc;
Window win;

	resetvinf(VI_WIN);
	nextvinf(&vp);
	win = makewin(display, vp);
	values.foreground = W_FG;
	values.background = W_BG;
	values.line_width = 1;


	gc = XCreateGC(display, win, (GCLineWidth | GCForeground | GCBackground), &values);

	XDrawLine(display, win, gc, 0, 1, 2, 1);

	if( ! checkpixel(display, win, 0, 2, W_BG)) {
		delete("Pixel at (0, 2) was not set to background.");
		return;
	} else
		CHECK;

	line_width = 10;
	XCALL;

	XDrawLine(display, win, gc, 0, 1, 2, 1);

	if( ! checkpixel(display, win, 0, 2, W_FG)) {
		report("Pixel at (0, 2) was not set to foreground.");
		FAIL;
	} else
		CHECK;

/* Line Style */

	values.line_style = LineDoubleDash;
	values.dashes = 1;

	gc =  XCreateGC(display, win, (GCLineWidth | GCForeground | GCBackground | GCLineStyle | GCDashList), &values);

	win = makewin(display, vp);

	XDrawLine(display, win, gc, 0, 0, 10, 0);

	if( ! checkpixel(display, win, 0, 1, W_BG)) {
		delete("Pixel at (0, 1) was not set to background.");
		return;
	} else
		CHECK;

	line_style = LineSolid;
	XCALL;

	XDrawLine(display, win, gc, 0, 0, 10, 0);

	if( ! checkpixel(display, win, 0, 1, W_FG)) {
		report("Pixel at (0, 1) was not set to foreground.");
		FAIL;
	} else
		CHECK;

/* Cap Style */


	values.cap_style = CapNotLast;
	values.line_width = 0;

	gc =  XCreateGC(display, win, (GCLineWidth | GCForeground | GCBackground | GCCapStyle), &values);
	win = makewin(display, vp);
	XDrawLine(display, win, gc, 0, 0, 2, 0);

	if( ! checkpixel(display, win, 2, 0, W_BG)) {
		delete("Pixel at (2, 0) was not set to background.");
		return;
	} else
		CHECK;
	
	line_width = 0;
	cap_style = CapButt;
	XCALL;
	XDrawLine(display, win, gc, 0, 0, 2, 0);

	if( ! checkpixel(display, win, 2, 0, W_FG)) {
		report("Pixel at (2, 0) was not set to foreground.");
		FAIL;
	} else
		CHECK;

/* Join Style */

	values.join_style = JoinBevel;
	values.line_width = 3;

	gc =  XCreateGC(display, win, (GCLineWidth | GCForeground | GCBackground | GCJoinStyle), &values);
	win = makewin(display, vp);
	XDrawLines(display, win, gc, Jpoints, sizeof(Jpoints) / sizeof(XPoint), CoordModeOrigin);

	if( ! checkpixel(display, win, 3, 0, W_BG)) {
		delete("Pixel at (3, 0) was not set to background.");
		return;
	} else
		CHECK;

	line_width = 3;
	join_style = JoinMiter;
	XCALL;
	XDrawLines(display, win, gc, Jpoints, sizeof(Jpoints) / sizeof(XPoint), CoordModeOrigin);

	if( ! checkpixel(display, win, 3, 0, W_FG)) {
		report("Pixel at (3, 0) was not set to foreground.");
		FAIL;
	} else
		CHECK;

	CHECKPASS(8);

>>ASSERTION Bad B 1
.ER Alloc
>>ASSERTION Bad A
.ER GC
>>ASSERTION Bad A
.ER Value line_style  LineSolid LineOnOffDash LineDoubleDash
>>ASSERTION Bad A
.ER Value cap_style CapNotLast CapButt CapRound CapProjecting
>>ASSERTION Bad A
.ER Value join_style JoinMiter JoinRound JoinBevel
>># HISTORY cal Completed	Written in new format and style
>># HISTORY kieron Completed	Global and pixel checking to do - 19/11/90
>># HISTORY dave Completed	Final checking to do - 21/11/90
>># HISTORY cal Action		Writing code.

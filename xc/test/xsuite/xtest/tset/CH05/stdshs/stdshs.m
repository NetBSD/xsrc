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
 * $XConsortium: stdshs.m,v 1.8 94/04/17 21:04:07 rws Exp $
 */
>>TITLE XSetDashes CH05
void
XSetDashes(display, gc, dash_offset, dash_list, n)
Display *display = Dsp;
GC gc;
int dash_offset;
char *dash_list = dashes;
int n = sizeof(dashes)/sizeof(char);
>>EXTERN
char dashes[] = { 2, 1};
char odashes[] = {1, 3, 2};
char edashes[] = {1, 3, 2, 1, 3, 2};
char errdashes[] = {1, 0, 1};
>>SET need-gc-flush
>>ASSERTION Good A
A call to xname sets the 
.M dash_offset
and
.M dash_list
attributes for dashed line styles
in the specified GC to the values of the
.A dash_offset
and
.A dash_list
arguments.
>>STRATEGY
Create Window.
Create GC with dashes component = 1 , LineStyle = LineDoubleDash, fg = WhitePixel, bg = BlackPixel.
Set pixel at (1, 0) with XDrawPoint.
Draw line from (0, 0) to (2, 0) with XDrawLines.
Verify pixel at (1, 0) is bg using XGetImage and XGetpixel.
Set dashes component of GC to [2, 1] with XSetDashes.
Draw line from (0, 0) to (2, 0) using XDrawLine.
Verify pixel at (1, 0) is fg using XGetImage and XGetPixel.
>>CODE
XVisualInfo *vp;
XGCValues values;
GC gc;
Window win;
int twidth;

	for(twidth = 0; twidth < 2; twidth++) {
		resetvinf(VI_WIN);
		nextvinf(&vp);
		win = makewin(display, vp);

		values.foreground = W_FG;
		values.background = W_BG;
		values.line_style = LineDoubleDash;
		values.line_width = twidth;
		values.dashes = 1; /* [1, 1] */


		gc = XCreateGC(display, win, (GCLineWidth |GCLineStyle | GCDashList | GCForeground | GCBackground), &values);

		XDrawPoint(display, win, gc, 1, 0);
		XDrawLine(display, win, gc, 0, 0, 2, 0);

		if( ! checkpixel(display, win, 1, 0, W_BG)) {
			delete("Pixel at (1, 0) was not set to background (width %d)", twidth);
			return;
		} else
			CHECK;

		dash_offset = 0;
		dash_list= dashes;
		n = sizeof(dashes)/sizeof(char);
		XCALL;

		XDrawLine(display, win, gc, 0, 0, 2, 0);

		if( ! checkpixel(display, win, 1, 0, W_FG)) {
			report("Pixel at (1, 0) was not set to foreground (width %d)", twidth);			
			FAIL;
		} else
			CHECK;

	}
	CHECKPASS(4);

>>ASSERTION Good A
The initial and alternate elements of the
.A dash_list
argument specify the lengths of the even dashes and 
the second and alternate elements specify the lengths of the odd dashes.
>>STRATEGY
For zero width and non-zero width lines:
    Create GC with line_style = LineDoubleDashed.
    Set dashes component of GC to [2, 1] with XSetDashes.
    Draw horizontal line from (0, 0) to (4, 0) using XDrawLine.
    Verify pixels at (0, 0) and (1, 0) are fg using XGetImage and XGetPixel.
    Verify pixel at (2, 0) is bg using XGetImage and XGetPixel.
    Verify pixel at (3, 0) is fg using XGetImage and XGetPixel.
    Draw vertical line from (0, 1) to (0, 5) using XDrawLine.
    Verify pixels at (0, 1) and (0, 2) are fg using XGetImage and XGetPixel.
    Verify pixel at (0, 3) is bg using XGetImage and XGetPixel.
    Verify pixel at (0, 4) is fg using XGetImage and XGetPixel.
>>CODE
XVisualInfo *vp;
GC gc;
XGCValues values;
Window win;
int twidth;


	for(twidth=0; twidth < 2; twidth++) {
		resetvinf(VI_WIN);
		nextvinf(&vp);
		win = makewin(display, vp);
	
		values.foreground = W_FG;
		values.background = W_BG;
		values.line_style = LineDoubleDash;
		values.line_width = twidth;
	
		gc = XCreateGC(display, win, (GCLineWidth | GCLineStyle | GCForeground | GCBackground ), &values);
	
		dash_offset = 0;
		dash_list = dashes;
		n = sizeof(dashes)/sizeof(char);
	
		XCALL;

		XDrawLine(display, win, gc, 0, 0, 4, 0);
	
		if( (checkpixels(display, win, 0, 0, 1, 0, 2, W_FG) == True) &&
		    (checkpixel(display, win, 2, 0, W_BG) == True) &&
		    (checkpixel(display, win, 3, 0, W_FG) == True) )
			CHECK;
		else {
			report("Horizontal dashing incorrect for width %d.", twidth);
			FAIL;
		}

		XDrawLine(display, win, gc, 0, 1, 0, 5);
	
		if(  (checkpixels(display, win, 0, 1, 0, 1, 2, W_FG) == True) &&
		     (checkpixel(display, win, 0, 3, W_BG) == True) &&
		     (checkpixel(display, win, 0, 4, W_FG) == True) )
			 CHECK;
		 else {
			 report("Vertical dashing incorrect for width %d.", twidth);
			 FAIL;
		 }

	}
	CHECKPASS(4);

>>ASSERTION Good A
When an odd-length list is specified, then it is
concatenated with itself to produce an even-length list.
>>STRATEGY
Create GC with line_style = LineDoubleDashed.
Set dashes component of GC to [1, 3, 2] with XSetDashes.
Draw horizontal line from (0, 0) to (10, 0) using XDrawLine.
Set dashes component of GC to [1, 3, 2, 1, 3, 2] with XSetDashes.
Draw horizontal line from (0, 1) to (10, 1) using XDrawLine.
Verify that the two lines are identical using XGetImage and XGetPixel.
>>CODE
XVisualInfo *vp;
XGCValues values;
XImage *image;
Window win;
int i;

	resetvinf(VI_WIN);
	nextvinf(&vp);
	win = makewin(display, vp);

	values.foreground = W_FG;
	values.background = W_BG;
	values.line_style = LineDoubleDash;
	gc = XCreateGC(display, win, (GCLineStyle | GCForeground | GCBackground), &values);

	dash_offset = 0;
	dash_list = odashes;
	n = sizeof(odashes)/sizeof(char);
	XCALL;

	XDrawLine(display, win, gc, 0, 0, 10, 0);


	dash_offset = 0;
	dash_list = edashes;
	n = sizeof(edashes)/sizeof(char);
	XCALL;

	XDrawLine(display, win, gc, 0, 1, 10, 1);

	image = XGetImage(display, win, 0, 0, 10, 2, AllPlanes, ZPixmap);

	for(i=0; i<10; i++)
		if(XGetPixel(image, i, 0) != XGetPixel(image, i, 1)) {
			report("Pixel at (%d,0) was not the same as pixel at (%d,1).", i, i);
			FAIL;
		} else
			CHECK;

	CHECKPASS(10);

>>ASSERTION Bad B 1
.ER Alloc
>>ASSERTION Bad A
.ER GC
>>ASSERTION Bad A
When the specified dash list is empty, then a
.S BadValue 
error occurs.
>>STRATEGY
Call XSetDashes with dash_list set to NULL and n set to 0.
Verify that a BadValue error occurs.
>>CODE BadValue

	gc = XCreateGC(display, DRW(display), 0L, (XGCValues*) 0);

	dash_offset = 0;
	dash_list = (char *)NULL;
	n = 0;
	XCALL;

	if(geterr() == BadValue)
		PASS;
	else
		FAIL;

>>ASSERTION Bad A
When an element of the dash list is 0, then a
.S BadValue 
error occurs.
>>STRATEGY
Set dashes component of GC to [1, 0, 1] with XSetDashes.
Verify that a BadValue error occurs.
>>CODE BadValue
int screen;
GC gc;

	screen = DefaultScreen(display);
	gc = XCreateGC(display, RootWindow(display, screen), 0L, (XGCValues*) 0);
	dash_offset = 0;
	dash_list = errdashes;
	n = sizeof(errdashes)/sizeof(char);
	XCALL;

	if(geterr() == BadValue)
		PASS;
	else
		FAIL;

>># HISTORY cal Completed	Written in new format and style
>># HISTORY kieron Completed	Global and pixel checking to do - 19/11/90
>># HISTORY dave Completed	Final checking to do - 21/11/90
>># HISTORY cal Action		Writing code.

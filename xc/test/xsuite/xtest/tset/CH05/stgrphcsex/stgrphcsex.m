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
 * $XConsortium: stgrphcsex.m,v 1.8 94/04/17 21:04:12 rws Exp $
 */
>>TITLE XSetGraphicsExposures CH05
void
XSetGraphicsExposures(display, gc, graphics_exposures)
Display *display = Dsp;
GC gc;
Bool graphics_exposures = False;
>>SET need-gc-flush
>>ASSERTION Good A
A call to xname sets the
.M graphics_exposures
component of the specified GC to the value of the
.A graphics_exposures
argument.
>>STRATEGY
Create window.
Create GC with graphics_exposures = True.
Flush event queue with XSync.
Copy from out of bounds of window using XCopyArea.
Verify that a GraphicsExpose event was generated, 
Set graphics_exposures = False with XSetGraphicsExposures.
Flush event queue with XSync.
Copy from in bounds of window using XCopyArea.
Verify that no event was generated.
>>CODE
XEvent event;
XVisualInfo *vp;
Window	win;
XGCValues	values;
struct area ar;

	resetvinf(VI_WIN);
	nextvinf(&vp);
	win = makewin(display, vp);
	values.foreground = W_FG;
	values.background = W_BG;
	values.graphics_exposures = True;
	gc = XCreateGC(display, win, GCForeground | GCBackground| GCGraphicsExposures, &values);

	XSync(display, True);
	XCopyArea(display, win, win, gc, -5, -5, 5, 5, 0, 0);
	XSync(display, False);

	if( getevent(display, &event) == 0 ) {
		delete("No graphics expose event was generated.");
		return;
	}

	if(event.type == GraphicsExpose)
		CHECK;
	else {
		delete("Event was not of type GraphicsExpose.");
		return;
	}

	graphics_exposures = False;
	XCALL;	

	XSync(display, True);
	XCopyArea(display, win, win, gc, 5, 5, 10, 10, 0, 0);
	XSync(display, False);

	if(getevent(display, &event) != 0) {
		report("An Event was generated.");
		FAIL;
	} else
		CHECK;

	CHECKPASS(2);

>>ASSERTION Bad B 1
.ER Alloc
>>ASSERTION Bad A
.ER GC
>>ASSERTION Bad A
.ER Value graphics_exposures True False
>># HISTORY cal Completed	Written in new format and style
>># HISTORY kieron Completed	Global and pixel checking to do - 19/11/90
>># HISTORY cal	Action		Writing code.

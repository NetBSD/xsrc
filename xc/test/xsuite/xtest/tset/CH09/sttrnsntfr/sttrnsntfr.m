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
 * $XConsortium: sttrnsntfr.m,v 1.7 94/04/17 21:08:59 rws Exp $
 */
>>TITLE XSetTransientForHint CH09

XSetTransientForHint(display, w, prop_window)
Display	*display = Dsp;
Window	w = DRW(Dsp);
Window	prop_window = DRW(Dsp);
>>EXTERN
#include	"Xatom.h"
>>ASSERTION Good A
A call to xname sets the WM_TRANSIENT_FOR property for the window
.A w
to be of type
.S WINDOW ,
format 32 and to have value set
to the window specified by the 
.A prop_window
argument.
>>STRATEGY
Create a window with XCreateWindow.
Set the WM_TRANSIENT_FOR property using XSetTransientForHint.
Obtain the WM_TRANSIENT_FOR property using XGetWindowProperty.
Verify that the property type is WINDOW.
Verify that the property format is 32.
Verify that the property value was correct.
>>CODE
Window		win, pwin;
XVisualInfo	*vp;
Window		*retwin = (Window *) NULL;
unsigned long	leftover, nitems, len;
int		actual_format;
Atom		actual_type;

	resetvinf(VI_WIN);
	nextvinf(&vp);
	win = makewin(display, vp);
	pwin = makewin(display, vp);	

	w = win;
	prop_window = pwin;
	XCALL;

	if (XGetWindowProperty(display, win, XA_WM_TRANSIENT_FOR, 0L, 1L, False,
	    AnyPropertyType, &actual_type, &actual_format, &nitems, &leftover, (unsigned char **) &retwin) != Success) {
		delete("XGetWindowProperty() did not return Success.");
		return;
	} else
		CHECK;

	if(leftover != 0) {
		report("The leftover elements numbered %lu instead of 0", leftover);
		FAIL;
	} else
		CHECK;

	if(actual_format != 32) {
		report("The format of the WM_TRANSIENT_FOR property was %lu instead of 32", actual_format);
		FAIL;
	} else
		CHECK;

	if(actual_type != XA_WINDOW) {
		report("The type of the WM_TRANSIENT_FOR property was %lu instead of WINDOW (%lu)", actual_type, (long) XA_WINDOW);
		FAIL;
	} else
		CHECK;
	
	if(nitems != 1) {
		report("The WM_TRANSIENT_FOR property comprised %ul elements instead of 1", nitems);
		FAIL;
	} else
		CHECK;

	if( retwin == (Window *) NULL) {
		report("No value was set for the WM_TRANSIENT_FOR property.");
		FAIL;

	} else {

		if(*retwin != pwin) {
			report("The WM_TRANSIENT_FOR property had value %lx instead of %lx", (long) *retwin, (long) pwin);
			FAIL;
		} else
			CHECK;

		XFree((char*)retwin);
	}

	CHECKPASS(6);



>>ASSERTION Bad B 1
.ER BadAlloc 
>>ASSERTION Good A
When the window specified by the
.A w
argument does not name a
valid window, then a
.S  BadWindow
error occurs.
>>STRATEGY
Obtain a bad window ID.
Set the WM_TRANSIENT_FOR property for the bad window using XSetTransientForHint.
Verify that a BadWindow error was generated.
>>CODE BadWindow
XVisualInfo	*vp;

	resetvinf(VI_WIN);
	nextvinf(&vp);
	prop_window = makewin(display, vp);

	w = badwin(display);
	
        XCALL;

        if (geterr() == BadWindow)
                PASS;
        else
                FAIL;
>># Kieron	Completed	Review




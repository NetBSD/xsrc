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
 * $XConsortium: stwmclrmpw.m,v 1.7 94/04/17 21:09:01 rws Exp $
 */
>>TITLE XSetWMColormapWindows CH09
Status
XSetWMColormapWindows(display, w, colormap_windows, count)
Display	*display = Dsp;
Window	w = DRW(Dsp);
Window	*colormap_windows = &window;
int count = 1;
>>EXTERN
#include	"Xatom.h"
Window	window;
>>ASSERTION Good A
A call to xname sets the
.S WM_COLORMAP_WINDOWS
property for the window
.A w
to be of type
.S WINDOW ,
format 32, and to have value set to the
.A count
windows specified by the
.A colormap_windows
argument
and returns non-zero.
>>STRATEGY
Create a window with XCreateWindow.
Set the WM_COLORMAP_WINDOWS property using XSetWMColormapWindows.
Verify that the call returned non-zero.
Obtain the WM_COLORMAP_WINDOWS atom using XInternAtom.
Obtain the WM_COLORMAP_WINDOWS property using XGetWindowProperty.
Verify that the property type is WINDOW.
Verify that the property format is 32.
Verify that the returned number of elements was correct.
Verify that the property value was correct.
>>CODE
Status		status;
Window		rwin;
XVisualInfo	*vp;
Atom		xa_wm_colormap_windows;
Atom		actual_type;
int		actual_format;
unsigned long	nitems;
unsigned long	leftover;
int		nwins = 7;
Window		pwins[7];
Window		*rwins = (Window *) NULL;
Window		*wp;
int		rcnt = 0;
int		i;

	for(i=0; i<nwins; i++)
		pwins[i] = (Window) i;

	resetvinf(VI_WIN);
	nextvinf(&vp);
	w = makewin(display, vp);

	colormap_windows = pwins;
	count = nwins;
	status = XCALL;

	if(status == 0) {
		report("%s() returned zero.", TestName);
		FAIL;
	} else
		CHECK;

	if((xa_wm_colormap_windows = XInternAtom(display, "WM_COLORMAP_WINDOWS", True)) == None) {
		delete("The \"WM_COLORMAP_WINDOWS\" string was not interned.");
		return;
	} else
		CHECK;

	if(XGetWindowProperty(display, w, xa_wm_colormap_windows,
                            0L, (long) nwins, False,
                            AnyPropertyType, &actual_type, &actual_format,
                            &nitems, &leftover, (unsigned char **) &rwins) != Success) {
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
		report("The format of the WM_COLORMAP_WINDOWS property was %lu instead of 32", actual_format);
		FAIL;
	} else
		CHECK;

	if(actual_type != XA_WINDOW) {
		report("The type of the WM_COLORMAP_WINDOWS property was %lu instead of WINDOW (%lu)", actual_type, (long) XA_WINDOW);
		FAIL;
	} else
		CHECK;

	
	if( rwins == (Window *) NULL) {
		report("No value was set for the WM_COLORMAP_WINDOWS property.");
		FAIL;
	} else {

		CHECK;
		if(nitems != nwins) {
			report("The WM_COLORMAP_WINDOWS property comprised %ul elements instead of %d", nitems, nwins);
			FAIL;
		} else {

			if(actual_format == 32) {
				CHECK;
				for(i = 0, wp = rwins; i<nwins; i++, wp++)
					if( *wp != pwins[i]) {
						report("Element %d of the WM_COLORMAP_WINDOWS value was %lu instead of %lu",
							i+1, (long) *wp, (long) pwins[i]);
						FAIL;
					} else
						CHECK;
			}
		}			
		XFree((char*)rwins);
	}

	CHECKPASS(8 + nwins);

>>ASSERTION Bad B 1
When the atom name \(lqWM_COLORMAP_WINDOWS\(rq cannot be interned, then
a call to xname returns zero.
>>ASSERTION Bad B 1
.ER BadAlloc 
>>ASSERTION Good A
.ER BadWindow 
>># Kieron	Action	Review

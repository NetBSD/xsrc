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
 * $XConsortium: gtwmhnts.m,v 1.8 94/04/17 21:08:46 rws Exp $
 */
>>TITLE XGetWMHints CH09
XWMHints *
XGetWMHints(display, w)
Display	*display = Dsp;
Window	w = DRW(Dsp);
>>EXTERN
#include	"Xatom.h"
>>ASSERTION Good A
When the WM_HINTS property has been set for the window
.A w ,
and is format 32, length \(>= 9 elements, and of type
.S WM_HINTS ,
then a call to xname returns a pointer to a
.S XWMHints
structure which contains the window manager hints for the window
and can be freed with XFree.
>>STRATEGY
Create a window with XCreateWindow.
Set the WM_HINTS property for the window with XSetWMHints.
Obtain the WM_HINTS property values with XGetWMHints.
Verify the result is non-NULL.
Verify that the returned values are correct.
Release the allocated memory using XFree.
>>CODE
Window		win;
XVisualInfo	*vp;
XWMHints	hints;
XWMHints	*hints_ret;

	resetvinf(VI_WIN);
	nextvinf(&vp);
	win = makewin(display, vp);

	hints.flags = AllHints;
	hints.input = True;
	hints.initial_state = IconicState;
	hints.icon_pixmap =  1L;
	hints.icon_window = 1L;
	hints.icon_x = 13;
	hints.icon_y = 7;
	hints.icon_mask = 1L;
	hints.window_group = 1L;

	XSetWMHints(display, win, &hints);

	w = win;
	hints_ret = XCALL;

	if(hints_ret == (XWMHints *) 0) {
		report("XGetWMHints() returned NULL.");
		FAIL;
		return; /* failure invalidates rest of test, and no freeing */
	} else
		CHECK;

	if(hints_ret->flags != AllHints) {
		report("The flags component was %lu instead of AllHints.", hints_ret->flags);
		FAIL;
	} else
		CHECK;

	if(hints_ret->input != True) {
		report("The hints_ret component of the XWMHints structure was %d instead of True.", (Bool) hints_ret->input);
		FAIL;
	} else
		CHECK;

	if(hints_ret->initial_state != IconicState) {
		report("The initial_state component of the XWMHints structure was %d instead of IconicState.",
			hints_ret->initial_state);
		FAIL;
	} else
		CHECK;

	if(hints_ret->icon_pixmap !=  1L) {
		report("The icon_pixmap component of the XWMHints structure was %lu instead of 1.", hints_ret->icon_pixmap);
		FAIL;
	} else
		CHECK;

	if(hints_ret->icon_window != 1L) {
		report("The icon_window component of the XWMHints structure was %lu instead of 1.", hints_ret->icon_window);
		FAIL;
	} else
		CHECK;

	if(hints_ret->icon_x != 13) {
		report("The icon_x component of the XWMHints structure was %d instead of 13.", hints_ret->icon_x);
		FAIL;
	} else
		CHECK;

	if(hints_ret->icon_y != 7) {
		report("The icon_y component of the XWMHints structure was %d instead of 7.", hints_ret->icon_y);
		FAIL;
	} else
		CHECK;

	if(hints_ret->icon_mask != 1L) {
		report("The icon_mask component of the XWMHints structure was %lu instead of 1.", hints_ret->icon_mask);
		FAIL;
	} else
		CHECK;

	if(hints_ret->window_group != 1L) {
		report("The window_group component of the XWMHints structure was %lu instead of 1.", hints_ret->window_group);
		FAIL;
	} else
		CHECK;

	XFree((char*)hints_ret);

	CHECKPASS(10);

>>ASSERTION Good A
When the WM_HINTS property has not been set for the window
.A w ,
or is format other than 32, length < 9 elements, or of type other than
.S WM_HINTS ,
then a call to xname returns NULL.
>>STRATEGY
Create a window with XCreateWindow.
Obtain the value of the unset WM_HINTS property with XGetWMHints.
Verify that the call returned NULL.

Create a window with XCreateWindow.
Set the WM_HINTS property with format 16 and type WM_HINTS and size 9 using XChangeProperty.
Obtain the value of the WM_HINTS property with XGetWMHints.
Verify that the call returned NULL.

Create a window with XCreateWindow.
Set the WM_HINTS property with format 32 type WM_ATOM and size 9 using XChangeProperty.
Obtain the value of the WM_HINTS property with XGetWMHints.
Verify that the call returned NULL.

Create a window with XCreateWindow.
Set the WM_HINTS property with format 32 type WM_HINTS and size 1 using XChangeProperty.
Obtain the value of the WM_HINTS property with XGetWMHints.
Verify that the call returned NULL.

>>CODE
XVisualInfo	*vp;
XWMHints	hints;
XWMHints	*hints_ret;

	resetvinf(VI_WIN);
	nextvinf(&vp);

	w = makewin(display, vp);
/* Property unset */

	hints_ret = XCALL;

	if(hints_ret != (XWMHints *) NULL){	
		report("XGetWMHints() did not return NULL with the WM_HINTS property unset.");
		FAIL;
	} else
		CHECK;

	w = makewin(display, vp);
/* format 16 */
	XChangeProperty(display, w, XA_WM_HINTS, XA_WM_HINTS, 16, PropModeReplace, (unsigned char *) &hints, 9);

	hints_ret = XCALL;

	if(hints_ret != (XWMHints *) NULL){	
		report("XGetWMHints() did not return NULL with the WM_HINTS property format set to 16.");
		FAIL;
	} else
		CHECK;

	w = makewin(display, vp);
/* type ATOM */
	XChangeProperty(display, w, XA_WM_HINTS, XA_ATOM, 32, PropModeReplace, (unsigned char *) &hints, 9);

	hints_ret = XCALL;

	if(hints_ret != (XWMHints *) NULL){	
		report("XGetWMHints() did not return NULL with the WM_HINTS type set to ATOM.");
		FAIL;
	} else
		CHECK;


	w = makewin(display, vp);
/* size 1 */
	XChangeProperty(display, w, XA_WM_HINTS, XA_WM_HINTS, 32, PropModeReplace, (unsigned char *) &hints, 1);

	hints_ret = XCALL;

	if(hints_ret != (XWMHints *) NULL){	
		report("XGetWMHints() did not return NULL with the WM_HINTS size set to 1.");
		FAIL;
	} else
		CHECK;

	CHECKPASS(4);

>>ASSERTION Bad A
.ER BadWindow
>># Kieron	Completed	Review

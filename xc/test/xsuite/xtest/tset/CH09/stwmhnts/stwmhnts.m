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
 * $XConsortium: stwmhnts.m,v 1.10 94/04/17 21:09:02 rws Exp $
 */
>>TITLE XSetWMHints CH09

XSetWmHints(display, w, wmhints)
Display		*display = Dsp;
Window		w = DRW(Dsp);
XWMHints	*wmhints = &dummyhints;
>>EXTERN
#include	"Xatom.h"
#define		NumPropWMHintsElements 9
XWMHints	dummyhints = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };
>>ASSERTION Good A
A call to xname sets the WM_HINTS property
for the window
.A w
to be of type
.S WM_HINTS ,
format 32 and to have value set
to the hints specified in the
.S XWMHints
structure named by the
.A wmhints
argument.
>>STRATEGY
Create a window with XCreateWindow.
Set the WM_HINTS property for the window with XSetWMHints.
Verify type and format are XA_WM_HINTS and 32, respectively.
Verify that the property value was correctly set with XGetWindowProperty.
>>CODE
Window		win;
XVisualInfo	*vp;
XWMHints	hints;
long		*hints_ret;
unsigned long	leftover, nitems;
int		actual_format;
Atom		actual_type;


	resetvinf(VI_WIN);
	nextvinf(&vp);
	win = makewin(display, vp);

	hints.flags = AllHints;
	hints.input = True;
	hints.initial_state = IconicState;
	hints.icon_pixmap =  154376L;
	hints.icon_window = 197236L;
	hints.icon_x = 13;
	hints.icon_y = 7;
	hints.icon_mask = 146890L;
	hints.window_group = 137235L;

	w = win;
	wmhints = &hints;
	XCALL;

	if (XGetWindowProperty(display, win, XA_WM_HINTS, 0L, (long)NumPropWMHintsElements,
	    False, AnyPropertyType, &actual_type, &actual_format,  &nitems, &leftover,
	    (unsigned char **)&hints_ret) != Success) {
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
		report("The format of the WM_HINTS property was %lu instead of 32", actual_format);
		FAIL;
	} else
		CHECK;

	if(actual_type != XA_WM_HINTS) {
		report("The type of the WM_HINTS property was %lu instead of XA_WM_HINTS (%lu)", actual_type, XA_WM_HINTS);
		FAIL;
	} else
		CHECK;

	if(nitems != NumPropWMHintsElements) {
		report("The number of elements comprising the WM_HINTS property was %lu instead of %lu.",
				nitems, (unsigned long) NumPropWMHintsElements);
		FAIL;
	} else
		CHECK;

	if(hints_ret[0] != hints.flags) {
		report("The flags component was %lu instead of %lu.", hints_ret[0], hints.flags);
		FAIL;
	} else
		CHECK;

	if(hints_ret[1] != hints.input) {
		report("The hints_ret component of the XWMHints structure was %lu instead of %d.", hints_ret[1], hints.input);
		FAIL;
	} else
		CHECK;

	if(hints_ret[2] != hints.initial_state) {
		report("The initial_state component of the XWMHints structure was %lu instead of %d.",
			hints_ret[2], hints.initial_state);
		FAIL;
	} else
		CHECK;

	if(hints_ret[3] != hints.icon_pixmap) {
		report("The icon_pixmap component of the XWMHints structure was %lu instead of %lu.", hints_ret[3], hints.icon_pixmap);
		FAIL;
	} else
		CHECK;

	if(hints_ret[4] != hints.icon_window) {
		report("The icon_window component of the XWMHints structure was %lu instead of %lu.", hints_ret[4], hints.icon_window);
		FAIL;
	} else
		CHECK;

	if(hints_ret[5] != hints.icon_x) {
		report("The icon_x component of the XWMHints structure was %ld instead of %d.", hints_ret[5], hints.icon_x);
		FAIL;
	} else
		CHECK;

	if(hints_ret[6] != hints.icon_y) {
		report("The icon_y component of the XWMHints structure was %ld instead of %d.", hints_ret[6], hints.icon_y);
		FAIL;
	} else
		CHECK;

	if(hints_ret[7] != hints.icon_mask) {
		report("The icon_mask component of the XWMHints structure was %lu instead of %lu.", hints_ret[7], hints.icon_mask);
		FAIL;
	} else
		CHECK;

	if(hints_ret[8] != hints.window_group) {
		report("The window_group component of the XWMHints structure was %lu instead of %lu.", hints_ret[8], hints.window_group);
		FAIL;
	} else
		CHECK;

	XFree((char*)hints_ret);

	CHECKPASS(14);

>>ASSERTION Bad B 1
.ER BadAlloc 
>>ASSERTION Bad A
.ER BadWindow 
>># Completed	Kieron	Review

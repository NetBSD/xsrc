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
 * $XConsortium: ftchnm.m,v 1.7 94/04/17 21:08:36 rws Exp $
 */
>>TITLE XFetchName CH09
Status
XFetchName(display, w, window_return_name)
Display	*display = Dsp;
Window	w = DRW(display);
char	**window_return_name = &winname;
>>EXTERN
#include	"Xatom.h"
char	*winname = "XtestJunkName";
>>ASSERTION Good A
When the WM_NAME property has been set for the
window specified by the
.A w
argument and has type
.S STRING
and format 8, then a call to xname returns in the
.A window_name_return
argument, which can be freed with XFree, the null-terminated
name of the window, and returns non-zero.
>>STRATEGY
Create a window with XCreateWindow
Set the name of the window to XtestWindowName with XStoreName
Obtain the name of the window with XFetchName
Verify that the returned name is the one that was set.
Release the allocated name using XFree.
>>CODE
Status		status;
Window		win;
XVisualInfo	*vp;
char		*wname = "XtestWindowName";
char		*wnameret;

	resetvinf(VI_WIN);
	nextvinf(&vp);
	win = makewin(display, vp);

	XStoreName(display, win, wname);

	w = win;
	window_return_name = &wnameret;
	status = XCALL;

	if(status == 0) {
		report("XFetchName() returned 0");
		FAIL;
	} else
		CHECK;

	if(strcmp(wname, (wnameret != NULL) ? wnameret : "NULL") != 0) {
		report("Window name was \"%s\" instead of \"%s\".", wnameret, wname);
		FAIL;
	} else
		CHECK;

	if (wnameret != NULL)
		XFree(wnameret);

	CHECKPASS(2);

>>ASSERTION Good A
When the WM_NAME property has not been set for the window specified by the
.A w
argument, or has format other than 8 or has type other than
.S STRING ,
then a call to xname
sets the 
.A window_name_return
argument to NULL, and returns zero.
>>STRATEGY
Create a window with XCreateWindow.
Obtain the value of the WM_NAME property with XFetchName.
Verify that the call returned zero.
Verify that the returned name was set to NULL.

Create a window with XCreateWindow.
Set the WM_NAME property with format 32 and type STRING using XChangeProperty.
Obtain the value of the WM_NAME property with XFetchName.
Verify that the call returned zero.
Verify that the returned name was set to NULL.

Create a window with XCreateWindow.
Set the WM_NAME property with format 8 type ATOM using XChangeProperty.
Obtain the value of the WM_NAME property with XFetchName.
Verify that the call returned zero.
Verify that the returned name was set to NULL.

>>CODE
Status		status;
Window		win;
XVisualInfo	*vp;
char		*name = "XTWindowName";
char		*wnameret;

	resetvinf(VI_WIN);
	nextvinf(&vp);
	win = makewin(display, vp);

/* property unset */
	w = win;
	window_return_name = &wnameret;
	status = XCALL;


	if(status != 0) {
		report("XFetchName() did not return 0 when the WM_NAME property was not set.");
		FAIL;
	} else
		CHECK;

	if(wnameret != NULL) {
		report("Window name was \"%s\" instead of NULL when the WM_NAME property was not set.", wnameret);
		FAIL;
	} else
		CHECK;

	win = makewin(display, vp);
/* format wrong */
	XChangeProperty(display, win, XA_WM_NAME, XA_STRING, 32, PropModeReplace, (unsigned char *) name, strlen(name));
	w = win;
	window_return_name = &wnameret;
	status = XCALL;

	if(status != 0) {
		report("XFetchName() did not return 0 when the WM_NAME property format was 32.");
		FAIL;
	} else
		CHECK;

	if(wnameret != NULL) {
		report("Window name was \"%s\" instead of NULL when the WM_NAME property format was 32.", wnameret);
		FAIL;
	} else
		CHECK;


	win = makewin(display, vp);
/* type wrong */
	XChangeProperty(display, win, XA_WM_NAME, XA_ATOM, 8, PropModeReplace, (unsigned char *) name, strlen(name));
	w = win;
	window_return_name = &wnameret;
	status = XCALL;

	if(status != 0) {
		report("XFetchName() did not return 0 when the WM_NAME property type was ATOM.");
		FAIL;
	} else
		CHECK;

	if(wnameret != NULL) {
		report("Window name was \"%s\" instead of NULL when the WM_NAME property type was ATOM.", wnameret);
		FAIL;
	} else
		CHECK;

	CHECKPASS(6);

>>ASSERTION Bad A
.ER BadWindow
>># Kieron	Completed	Review.

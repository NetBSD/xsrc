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
 * $XConsortium: gticnnm.m,v 1.8 94/04/17 21:08:39 rws Exp $
 */
>>TITLE XGetIconName CH09
Status
XGetIconName(display, w, icon_name_return)
Display	*display = Dsp;
Window	w = DRW(Dsp);
char	**icon_name_return = &iconname;
>>EXTERN
#include	"Xatom.h"
char	*iconname = "XtestIC";
>>ASSERTION Good A
When the
.S WM_ICON_NAME
property for the window
.A w
has been set,
and has format 8 and type
.S STRING ,
then a call to xname returns that name, which can
be freed with XFree, in the
.A icon_name_return
argument and returns non-zero.
>>STRATEGY
Create a window with XCreateWindow.
Set the icon name for the window with XSetIconName.
Obtain the icon name with XGetIconName.
Verify that the returned name is that which was set.
Realease the allocated memory using XFree.
>>CODE
Window	win;
char	*iconname = "Xtest Icon Name";
char	*iconnameret;
Status	status;
char	**list_return;
int	count_return;
XTextProperty	tp, rtp;
XVisualInfo	*vp;

	resetvinf(VI_WIN);
	nextvinf(&vp);
	win = makewin(display, vp);

	XSetIconName(display, win, iconname);

	w = win;
	icon_name_return = &iconnameret;
	status = XCALL;

	if(status == False) {
		report("XGetIconName() returned False.");
		FAIL;
	} else
		CHECK;

	if(strcmp(iconname, (iconnameret != NULL) ? iconnameret : "NULL") != 0) {
		report("XGetIconName() returned \"%s\" instead of \"%s\"", iconnameret, iconname);
		FAIL;
	} else
		CHECK;

	if(iconnameret != NULL) 
		XFree(iconnameret);

	CHECKPASS(2);

>>ASSERTION Good A
When the
.S WM_ICON_NAME
property for the window
.A w
has not been set,
or has format other than 8 or type other than
.S STRING ,
then a call to xname sets the string in the
.A icon_name_return
argument to NULL and returns zero.
>>STRATEGY
Create a window with XCreateWindow.
Obtain the value of the WM_ICON_NAME property with XGetIconName.
Verify that the call returned zero.
Verify that the returned name was set to NULL.

Create a window with XCreateWindow.
Set the WM_ICON_NAME property with format 16 and type STRING using XChangeProperty.
Obtain the value of the WM_ICON_NAME property with XGetIconName.
Verify that the call returned zero.
Verify that the returned name was set to NULL.

Create a window with XCreateWindow.
Set the WM_ICON_NAME property with format 8 type ATOM using XChangeProperty.
Obtain the value of the WM_ICON_NAME property with XGetIconName.
Verify that the call returned zero.
Verify that the returned name was set to NULL.

>>CODE
char	*iconnameret = "XTestUninit";
char	*name = "XTestIconName";
Status	status;
char	**list_return;
int	count_return;
XVisualInfo	*vp;

	resetvinf(VI_WIN);
	nextvinf(&vp);
	w = makewin(display, vp);

/* Property unset */

	icon_name_return = &iconnameret;
	status = XCALL;

	if(status != False) {
		report("XGetIconName() did not return False when the WM_ICON_NAME property was unset.");
		FAIL;
	} else
		CHECK;

	if( iconnameret != NULL) {
		report("XGetIconName() returned \"%s\" instead of NULL when the WM_ICON_NAME was unset.", iconnameret);
		FAIL;
	} else
		CHECK;

	w = makewin(display, vp);

/* format 16 */
	XChangeProperty(display, w, XA_WM_ICON_NAME, XA_STRING, 16, PropModeReplace, (unsigned char *) name, strlen(name));

	status = XCALL;

	if(status != False) {
		report("XGetIconName() did not return False when the WM_ICON_NAME property format was 16.");
		FAIL;
	} else
		CHECK;

	if( iconnameret != NULL) {
		report("XGetIconName() returned \"%s\" instead of NULL when the WM_ICON_NAME property format was 16.", iconnameret);
		FAIL;
	} else
		CHECK;

	w = makewin(display, vp);
/* type XA_ATOM */
	XChangeProperty(display, w, XA_WM_ICON_NAME, XA_ATOM, 8, PropModeReplace, (unsigned char *) name, strlen(name));
	status = XCALL;

	if(status != False) {
		report("XGetIconName() did not return False when the WM_ICON_NAME property type was ATOM.");
		FAIL;
	} else
		CHECK;

	if( iconnameret != NULL) {
		report("XGetIconName() returned \"%s\" instead of NULL when the WM_ICON_NAME property type was ATOM", iconnameret);
		FAIL;
	} else
		CHECK;

	CHECKPASS(6);

>>ASSERTION Bad A
.ER BadWindow
>># Action	Completed	Review

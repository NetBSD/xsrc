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
 * $XConsortium: gtwmszhnts.m,v 1.9 94/04/17 21:08:50 rws Exp $
 */
>>TITLE XGetWMSizeHints CH09
Status
XGetWMSizeHints(display, w, hints_return, supplied_return, property)
Display		*display = Dsp;
Window		w = DRW(Dsp);
XSizeHints	*hints_return = &sizehints;
long		*supplied_return = &supret;
Atom		property = XA_WM_NORMAL_HINTS;
>>EXTERN
#include	"Xatom.h"
#define		NumPropSizeElements 18
#define		OldNumPropSizeElements 15
static XSizeHints	sizehints = {0,1,2,3,4,5,6,7,8,9,10, {11,12} ,  {13,14},  15, 16, 17};
static XSizeHints	rhints =    {100,101,102,103,104,105,106,107,108,109,110, {111,112} ,  {113,114},  115, 116, 117};
static long		supret;
>>ASSERTION Good A
When the property specified by the
.A property
argument is set on the window
.A w
with a
.M type
of WM_SIZE_HINTS, a
.M format
of 32 and is long enough to contain either a pre-ICCCM
structure or a new size hints structure, then a call to
xname sets the components of the
.S XSizeHints
structure named by the
.A hints_return
clear
argument and returns non-zero.
>>STRATEGY
Create a window with XCreateWindow.
Set the property WM_NORMAL_HINTS with XSetWMSizeHints.
Obtain the value of the WM_NORMAL_HINTS property using XGetWMSizeHints.
Verify that the function did not return False.
Verify that the entire structure has been returned.
>>CODE
Status		status;
Window		win;
long		rsupp;
XVisualInfo	*vp;

	resetvinf(VI_WIN);
	nextvinf(&vp);
	win = makewin(display, vp);

	XSetWMSizeHints(display, win, &sizehints, property);

	w = win;
	hints_return = &rhints;
	supplied_return = &rsupp;
	status = XCALL;

	if( status == False ) {
		report("XGetWMSizeHints() returned False.");
		FAIL;
	} else
		CHECK;
	
	if(rhints.flags != 0) {
		report("The flags component of the XSizeHints structure was %lu instead of 0.", rhints.flags);
		FAIL;
	} else
		CHECK;

	if(rhints.x != 1) {
		report("The x component of the XSizeHints structure was %d instead of 1.", rhints.x);
		FAIL;
	} else
		CHECK;

	if(rhints.y != 2) {
		report("The y component of the XSizeHints structure was %d instead of 2.", rhints.y);
		FAIL;
	} else
		CHECK;

	if(rhints.width != 3) {
		report("The width component of the XSizeHints structure was %d instead of 3.", rhints.width);
		FAIL;
	} else
		CHECK;

	if(rhints.height != 4) {
		report("The height component of the XSizeHints structure was %d instead of 4.", rhints.height);
		FAIL;
	} else
		CHECK;

	if(rhints.min_width != 5) {
		report("The min_width component of the XSizeHints structure was %d instead of 5.", rhints.min_width);
		FAIL;
	} else
		CHECK;

	if(rhints.min_height != 6) {
		report("The min_height component of the XSizeHints structure was %d instead of 6.", rhints.min_height);
		FAIL;
	} else
		CHECK;

	if(rhints.max_width != 7) {
		report("The max_width component of the XSizeHints structure was %d instead of 7.", rhints.max_width);
		FAIL;
	} else
		CHECK;

	if(rhints.max_height != 8) {
		report("The max_height component of the XSizeHints structure was %d instead of 8.", rhints.max_height);
		FAIL;
	} else
		CHECK;

	if(rhints.width_inc != 9) {
		report("The width_inc component of the XSizeHints structure was %d instead of 9.", rhints.width_inc);
		FAIL;
	} else
		CHECK;

	if(rhints.height_inc != 10) {
		report("The height_inc component of the XSizeHints structure was %d instead of 10.", rhints.height_inc);
		FAIL;
	} else
		CHECK;

	if((rhints.min_aspect.x != 11) || (rhints.min_aspect.y != 12)){
		report("The min_aspect components of the XSizeHints structure were %d, %d instead of 11, 12.",
			rhints.min_aspect.x, rhints.min_aspect.y);
		FAIL;
	} else
		CHECK;

	if((rhints.max_aspect.x != 13) || (rhints.max_aspect.y != 14)){
		report("The max_aspect components of the XSizeHints structure were %d, %d instead of 13, 14.",
			rhints.max_aspect.x, rhints.max_aspect.y);
		FAIL;
	} else
		CHECK;

	if(rhints.base_width != 15) {
		report("The base_width component of the XSizeHints structure was %d instead of 15.", rhints.base_width);
		FAIL;
	} else
		CHECK;

	if(rhints.base_height != 16) {
		report("The base_height component of the XSizeHints structure was %d instead of 16.", rhints.base_height);
		FAIL;
	} else
		CHECK;

	if(rhints.win_gravity != 17) {
		report("The win_gravity component of the XSizeHints structure was %d instead of 17", rhints.win_gravity);
		FAIL;
	} else
		CHECK;

	CHECKPASS(17);


>>ASSERTION Good A
When the property specified by the
.A property
argument is not set on the window
.A w ,
or is not of
.M type
WM_SIZE_HINTS, or is not of 
.M format
32, or the property is not long enough to contain either a pre-ICCCM
structure or a new size hints structure, then a call to
xname returns zero.
>>STRATEGY
Create a window with XCreateWindow.
Obtain the property WM_NORMAL_HINTS with XGetWMSizeHints.
Verify that the call returned zero.

Create a window with XCreateWindow.
Set the WM_NORMAL_HINTS property of format 32 type WM_SIZE_HINTS with OldNumPropSizeElements-1 elements XChangeProperty.
Obtain the property WM_NORMAL_HINTS with XGetWMSizeHints.
Verify that the call returned zero.

Create a window with XCreateWindow.
Set the WM_NORMAL_HINTS property of format 16 type WM_SIZE_HINTS with NumPropSizeElements elements with XChangeProperty.
Obtain the property WM_NORMAL_HINTS with XGetWMSizeHints.
Verify that the call returned zero.

Create a window with XCreateWindow.
Set the WM_NORMAL_HINTS property of format 32 type STRING with NumPropSizeElements elements with XChangeProperty.
Obtain the property WM_NORMAL_HINTS with XGetWMSizeHints.
Verify that the call returned zero.
>>CODE
Status		status;
Window		win;
XSizeHints	rhints;
long		rsupp;
XSizeHints	dummy; /* Big enough (by definition). */
XVisualInfo	*vp;

	resetvinf(VI_WIN);
	nextvinf(&vp);

	win = makewin(display, vp);

	w = win;
	hints_return = &rhints;
	supplied_return = &rsupp;
	status = XCALL;
	if(status != False) {
		report("XGetWMSizeHints() did not return False when the WM_NORMAL_HINTS property was unset.");
		FAIL;
	} else
		CHECK;

	win = makewin(display, vp);
	XChangeProperty(display, win, XA_WM_NORMAL_HINTS, XA_WM_SIZE_HINTS, 32, PropModeReplace, (unsigned char *)&dummy, OldNumPropSizeElements-1);
	w = win;
	hints_return = &rhints;
	supplied_return = &rsupp;
	status = XCALL;
	if(status != False) {
		report("XGetWMSizeHints() did not return False when size was %lu 32-bit elements.", OldNumPropSizeElements-1);
		FAIL;
	} else
		CHECK;

	win = makewin(display, vp);
	XChangeProperty(display, win, XA_WM_NORMAL_HINTS, XA_WM_SIZE_HINTS, 16, PropModeReplace, (unsigned char *)&dummy, NumPropSizeElements);
	w = win;
	status = XCALL;

	if(status != False) {
		report("XGetWMSizeHints() did not return False when format was 16.");
		FAIL;
	} else
		CHECK;

	win = makewin(display, vp);
	XChangeProperty(display, win, XA_WM_NORMAL_HINTS, XA_STRING, 32, PropModeReplace, (unsigned char *)&dummy, NumPropSizeElements);
	w = win;
	status = XCALL;

	if(status != False) {
		report("XGetWMSizeHints() did not return False when type was STRING.");
		FAIL;
	} else
		CHECK;

	CHECKPASS(4);

>>ASSERTION Good A
When a call to xname returns successfully and a pre-ICCCM size hints property is read,
then the 
.A supplied_return
argument contains the bits
.S "(USPosition | USSize | PPosition | PSize | PMinSize | PMaxSize | PResizeInc | PAspect)."
>>STRATEGY
Create a window with XCreateWindow.
Set the property WM_NORMAL_HINTS with size OldNumPropSizeElements using XChangeProperty.
Obtain the value of the WM_NORMAL_HINTS property using XGetWMSizeHints.
Verify that the value returned in supplied_return was (USPositionUSSize|PPosition|PSize|PMinSize|PMaxSize|PResizeInc|PAspect).
>>CODE
Status		status;
Window		win;
XSizeHints	rhints;
long		rsupp;
XSizeHints	dummy; /* Big enough (by definition). */
XVisualInfo	*vp;
unsigned long	flagval = (USPosition | USSize | PPosition | PSize | PMinSize | PMaxSize | PResizeInc | PAspect) ;

	resetvinf(VI_WIN);
	nextvinf(&vp);
	win = makewin(display, vp);

	XChangeProperty(display, win, XA_WM_NORMAL_HINTS, XA_WM_SIZE_HINTS, 32, PropModeReplace, (unsigned char *)&dummy, OldNumPropSizeElements);
	w = win;
	hints_return = &rhints;
	supplied_return = &rsupp;
	status = XCALL;
	if(status == False) {
		report("XGetWMSizeHints() returned False.");
		FAIL;
	} else
		CHECK;

	if( rsupp != flagval ) {
		report("supplied_return was %lu instead of (USPosition | USSize | PPosition | PSize | PMinSize | PMaxSize | PResizeInc | PAspect) (%lu)", rsupp, flagval);
		report("for a pre-ICCCM structure.");
		FAIL;
	} else
		CHECK;

	CHECKPASS(2);

>>ASSERTION Good A
When the property specified by the
.A property
argument is large enough to contain the base size 
and window gravity fields, then the
.A supplied_return
argument contains the bits
.S "(PBaseSize | PWinGravity | USPosition | USSize | PPosition | PSize | PMinSize | PMaxSize | PResizeInc | PAspect)."
>>STRATEGY
Create a window with XCreateWindow.
Set the property WM_NORMAL_HINTS with size OldNumPropSizeElements using XChangeProperty.
Obtain the value of the WM_NORMAL_HINTS property using XGetWMSizeHints.
Verify that the value returned in supplied_return was (PBaseSize|PWinGravity|USPosition|USSize|PPosition|PSize|PMinSize|PMaxSize|PResizeInc|PAspect).
>>CODE
Status		status;
Window		win;
XSizeHints	rhints;
long		rsupp;
XSizeHints	dummy; /* Big enough (by definition). */
XVisualInfo	*vp;
long		flagval = (PBaseSize | PWinGravity | USPosition | USSize | PPosition | PSize | PMinSize | PMaxSize | PResizeInc | PAspect);

	resetvinf(VI_WIN);
	nextvinf(&vp);
	win = makewin(display, vp);

	XChangeProperty(display, win, XA_WM_NORMAL_HINTS, XA_WM_SIZE_HINTS, 32, PropModeReplace, (unsigned char *)&dummy, NumPropSizeElements);
	w = win;
	hints_return = &rhints;
	supplied_return = &rsupp;
	status = XCALL;
	if(status == False) {
		report("%s() returned False.", TestName);
		FAIL;
	} else
		CHECK;

	if( rsupp != flagval) {
		report("supplied_return was %lu instead of (PBaseSize | PWinGravity | USPosition | USSize | PPosition | PSize | PMinSize | PMaxSize | PResizeInc | PAspect) (%lu)", rsupp, flagval);
		report("for an ICCCMv1 structure.");
		FAIL;
	} else
		CHECK;

	CHECKPASS(2);

>>ASSERTION Bad A
.ER BadAtom 
>>ASSERTION Bad A
.ER BadWindow 

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
 * $XConsortium: stwmszhnts.m,v 1.11 94/04/17 21:09:13 rws Exp $
 */
>>TITLE XSetWMSizeHints CH09
void
XSetWMSizeHints(display, w, hints, property)
Display		*display = Dsp;
Window		w = DRW(Dsp);
XSizeHints	*hints = &sizehints_0;
Atom		property = XA_WM_NORMAL_HINTS;
>>EXTERN
#define		NumPropSizeElements 18       /* ICCCM v. 1 */
#include	"Xatom.h"
static XSizeHints	sizehints_0 = { 0,0,0,0,0,0,0,0,0,0,0, {0,0}, {0,0}, 0,0,0};
static XSizeHints	sizehints_1 = { 0,1,2,3,4,5,6,7,8,9,10, {11,12}, {13,14}, 15, 16, 17};
>>ASSERTION Good A
A call to xname sets the
property, specified by the
.A property
argument, for the window
.A w ,
to be of type
.S WM_SIZE_HINTS ,
format 32 and to have value set to
the hints in the
.S XSizeHints
structure named by the
.A hints
argument.
>>STRATEGY
Create a window using XCreateWindow.
Set the property WM_NORMAL_HINTS using XSetWMSizeHints.
Obtain the value of the WM_NORMAL_HINTS property using XGetWindowProperty.
Verify that the property format is 32.
Verify that the property type is WM_SIZE_HINTS.
Verify that the property value is correct.
>>CODE
Window		win;
XVisualInfo	*vp;
Atom		rtype;
int		rformat;
unsigned long	ritems, rbytes, *uls = NULL;
XSizeHints	pp;
int		i;

	resetvinf(VI_WIN);
	nextvinf(&vp);
	win = makewin(display, vp);

	w = win;	
	hints = &sizehints_1;
	XCALL;

	if( XGetWindowProperty(display, win, XA_WM_NORMAL_HINTS, 0L,
			(long) NumPropSizeElements, False,
			AnyPropertyType, &rtype, &rformat, &ritems, &rbytes,
			(unsigned char **) &uls) != Success ) {
		delete("XGetWindowProperty() did not return Success.");
		return;
	} else
		CHECK;

	if( rtype !=  XA_WM_SIZE_HINTS ) {
		report("WM_NORMAL_HINTS property was type %lu instead of WM_SIZE_HINTS (%lu)",
			(unsigned long) rtype, (unsigned long) XA_WM_SIZE_HINTS);
		FAIL;
	} else
		CHECK;

	if( rformat !=  32 ) {
		report("WM_NORMAL_HINTS property was format %d instead of 32.", rformat);
		FAIL;
	} else
		CHECK;

	/* unpack from the array of unsigned longs into pp */
	pp.flags = uls[i=0];

	pp.x = (int)uls[++i]; /* obsolete for new window mgrs, but clients */
	pp.y = (int)uls[++i];
	pp.width = (int)uls[++i];
	pp.height = (int)uls[++i]; /* should set so old wm's don't mess up */

	pp.min_width = (int)uls[++i]; pp.min_height = (int)uls[++i];
	pp.max_width = (int)uls[++i]; pp.max_height = (int)uls[++i];
    	pp.width_inc = (int)uls[++i]; pp.height_inc = (int)uls[++i];
	pp.min_aspect.x = (int)uls[++i]; pp.min_aspect.y = (int)uls[++i];
	pp.max_aspect.x = (int)uls[++i]; pp.max_aspect.y = (int)uls[++i];

	pp.base_width = (int)uls[++i];	/* added by ICCCM version 1 */
	pp.base_height = (int)uls[++i];	/* added by ICCCM version 1 */
	pp.win_gravity = (int)uls[++i];	/* added by ICCCM version 1 */

	if(pp.flags != 0) {
		report("The flags component of the XSizeHints structure was %lu instead of 0.", pp.flags);
		FAIL;
	} else
		CHECK;

	if(pp.x != 1) {
		report("The x component of the XSizeHints structure was %d instead of 1.", pp.x);
		FAIL;
	} else
		CHECK;

	if(pp.y != 2) {
		report("The y component of the XSizeHints structure was %d instead of 2.", pp.y);
		FAIL;
	} else
		CHECK;

	if(pp.width != 3) {
		report("The width component of the XSizeHints structure was %d instead of 3.", pp.width);
		FAIL;
	} else
		CHECK;

	if(pp.height != 4) {
		report("The height component of the XSizeHints structure was %d instead of 4.", pp.height);
		FAIL;
	} else
		CHECK;

	if(pp.min_width != 5) {
		report("The min_width component of the XSizeHints structure was %d instead of 5.", pp.min_width);
		FAIL;
	} else
		CHECK;

	if(pp.min_height != 6) {
		report("The min_height component of the XSizeHints structure was %d instead of 6.", pp.min_height);
		FAIL;
	} else
		CHECK;

	if(pp.max_width != 7) {
		report("The max_width component of the XSizeHints structure was %d instead of 7.", pp.max_width);
		FAIL;
	} else
		CHECK;

	if(pp.max_height != 8) {
		report("The max_height component of the XSizeHints structure was %d instead of 8.", pp.max_height);
		FAIL;
	} else
		CHECK;

	if(pp.width_inc != 9) {
		report("The width_inc component of the XSizeHints structure was %d instead of 9.", pp.width_inc);
		FAIL;
	} else
		CHECK;

	if(pp.height_inc != 10) {
		report("The height_inc component of the XSizeHints structure was %d instead of 10.", pp.height_inc);
		FAIL;
	} else
		CHECK;

	if((pp.min_aspect.x != 11) || (pp.min_aspect.y != 12)){
		report("The min_aspect components of the XSizeHints structure were %d, %d instead of 11, 12.",
			pp.min_aspect.x, pp.min_aspect.y);
		FAIL;
	} else
		CHECK;

	if((pp.max_aspect.x != 13) || (pp.max_aspect.y != 14)){
		report("The max_aspect components of the XSizeHints structure were %d, %d instead of 13, 14.",
			pp.max_aspect.x, pp.max_aspect.y);
		FAIL;
	} else
		CHECK;

	if(pp.base_width != 15) {
		report("The base_width component of the XSizeHints structure was %d instead of 15.", pp.base_width);
		FAIL;
	} else
		CHECK;

	if(pp.base_height != 16) {
		report("The base_height component of the XSizeHints structure was %d instead of 16.", pp.base_height);
		FAIL;
	} else
		CHECK;

	if(pp.win_gravity != 17) {
		report("The win_gravity component of the XSizeHints structure was %d instead of 17", pp.win_gravity);
		FAIL;
	} else
		CHECK;

	XFree((char*)uls);
	CHECKPASS(19);

>>ASSERTION Bad B 1
.ER BadAlloc
>>ASSERTION Bad A
.ER BadAtom
>>ASSERTION Bad A
.ER BadWindow 
>># Kieron	Action	Review

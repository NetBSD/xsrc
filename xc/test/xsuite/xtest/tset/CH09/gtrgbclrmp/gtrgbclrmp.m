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
 * $XConsortium: gtrgbclrmp.m,v 1.11 94/04/17 21:08:41 rws Exp $
 */
>>TITLE XGetRGBColormaps CH09
Status
XGetWMColormaps(display, w, std_colormap_return, count_return, property)
Display			*display = Dsp;
Window			w = DRW(Dsp);
XStandardColormap	**std_colormap_return = &scolormap;
int			*count_return = &cntret;
Atom			property = XA_ATOM;
>>EXTERN

#include		"Xatom.h"
static XStandardColormap	*scolormap;
static int			cntret;
static XStandardColormap	scmp1 = { (Colormap)  1,  2L,  3L,  4L,  5L,  6L,  7L,  8L, (VisualID) -1, (XID) -2 };
static XStandardColormap	scmp2 = { (Colormap) 11, 12L, 13L, 14L, 15L, 16L, 17L, 18L, (VisualID) 19, (XID) 20 };
static XStandardColormap	scmp3 = { (Colormap) 21, 22L, 23L, 24L, 25L, 26L, 27L, 28L, (VisualID) 21, (XID) 22 };

>>#
>># COMMENT:
>># This assertion does not match the Xlib code.
>># I think it maybe ought to read 8 * n or 9 * n or 10 * n elements
>># for positive integer n.
>># Until the review period has expired this test will avoid the
>># problem.
>>#
>># Note also that a list of colormaps that are not all of 10
>># elements in size will definitely fail with the current
>># Xlib. Hence Only single non-ICCCCCCCCCM structures can be
>># obtained from the server without failure.
>>#
>># Cal 29/05/91
>>#
>>ASSERTION Good A
When the property named by the
.A property
argument is set on the window named by the
.A w
argument, is of 
.M type
RGB_COLOR_MAP, is of
.M format
32, and is of length n1*8 + n2*9 + n3*10 elements (where n1,n2,n3 \(>= 0 and
n1+n2+n3 \(>= 1), then
a call to xname returns in the
.A std_colormap_return
argument the RBG colormap definitions, which can be freed with XFree,
and in
.A count_return
the number of structures and returns non-zero.
>>STRATEGY
Create a window using XCreateWindow.
Set the RGB_DEFAULT_MAP property using XSetRGBColormaps.
Obtain the value of the RGB_DEFAULT_MAP property using XGetRGBColormaps.
Verify that the call did not return False.
Verify that the value was correct.
Free the allocated memory using XFree.
>>CODE
Status			status;
int			ncmp = 3;
XStandardColormap	scmp[3];
XStandardColormap	*rscmp = (XStandardColormap *) NULL;
XStandardColormap	*cmpp;
int			rncmp = 0;
int			i;
XVisualInfo		*vp;
Atom			prop = XA_RGB_DEFAULT_MAP;

	scmp[0] = scmp1;
	scmp[1] = scmp2;
	scmp[2] = scmp3;

	resetvinf(VI_WIN);	
	nextvinf(&vp);

	w = makewin(display, vp);
	XSetRGBColormaps(display, w, scmp, ncmp, prop);

	std_colormap_return = &rscmp;
	count_return = &rncmp;
	property = prop;
	status = XCALL;

	if(status == False) {
		delete("%s returned False.", TestName);
		return;
	} else
		CHECK;

	if(rscmp == (XStandardColormap *) NULL) {	
		report("The returned list of XStandardColormap structures was NULL.");
		FAIL;
	} else {

		CHECK;
		if( rncmp != ncmp) {
			report("%d XStandardColormap structures were returned instead of %d.", rncmp, ncmp);
			FAIL;
		} else {

			CHECK;
			
			for(i = 0, cmpp = rscmp; i < ncmp; i++, cmpp++) {

				if(cmpp->colormap != scmp[i].colormap) {
					report("The colormap component of the XStandardColormap structure %d was incorrect.", i);
					FAIL;
				} else
					CHECK;
			
				if(cmpp->red_max != scmp[i].red_max)  {
					report("The red_max component of the XStandardColormap structure %d was incorrect.", i);
					FAIL;
				} else
					CHECK;
			
				if(cmpp->red_mult != scmp[i].red_mult) {
					report("The red_mult component of the XStandardColormap structure %d was incorrect.", i);
					FAIL;
				} else
					CHECK;
			
				if(cmpp->green_max !=scmp[i].green_max) {
					report("The green_max component of the XStandardColormap structure %d was incorrect.", i);
					FAIL;
				} else
					CHECK;
			
				if(cmpp->green_mult != scmp[i].green_mult) {
					report("The green_mult component of the XStandardColormap structure %d was incorrect.", i);
					FAIL;
				} else
					CHECK;
			
				if(cmpp->blue_max != scmp[i].blue_max) {
					report("The blue_max component of the XStandardColormap structure %d was incorrect.", i);
					FAIL;
				} else
					CHECK;
			
				if(cmpp->blue_mult != scmp[i].blue_mult) {
					report("The blue_mult component of the XStandardColormap structure %d was incorrect.", i);
					FAIL;
				} else
					CHECK;
			
				if(cmpp->base_pixel != scmp[i].base_pixel) {
					report("The base_pixel component of the XStandardColormap structure %d was incorrect.", i);
					FAIL;
				} else
					CHECK;
			
				if(cmpp->visualid != scmp[i].visualid) {
					report("The visualid component of the XStandardColormap structure %d was incorrect.", i);
					FAIL;
				} else
					CHECK;
			
				if(cmpp->killid != scmp[i].killid ) {
					report("The killid component of the XStandardColormap structure %d was incorrect.", i);
					FAIL;
				} else
					CHECK;
			
			}
		}
		XFree((char*)rscmp);
	}

	CHECKPASS(ncmp * 10 + 3);

>>#
>># COMMENT:
>>#    See above COMMENT
>>#
>>ASSERTION Good A
When the property named by the
.A property
argument is not set on the window named by the
.A w
argument, or is not of 
.M type
RGB_COLOR_MAP, or is not of
.M format
32, or is not 
of length n1*8 + n2*9 + n3*10 elements (where n1,n2,n3 \(>= 0 and
n1+n2+n3 \(>= 1), then
a call to xname does not set the
.A std_colormap_return
or
.A count_return
arguments and returns zero.
>>STRATEGY
Create a window with XCreateWindow.
Obtain the value of the unset RGB_COLOR_MAP property using XGetRGBColormaps.
Verify that the call returned False.
Verify that the std_colormap_return argument was not changed.
Verify that the count_return argument was not changed.

Create a window with XCreateWindow.
Set the RGB_COLOR_MAP property to have format 8.
Obtain the value of the RGB_COLOR_MAP property using XGetRGBColormaps.
Verify that the call returned False.
Verify that the std_colormap_return argument was not changed.
Verify that the count_return argument was not changed.

Create a window with XCreateWindow.
Set the RGB_COLOR_MAP property to have type ATOM.
Obtain the value of the RGB_COLOR_MAP property using XGetRGBColormaps.
Verify that the call returned False.
Verify that the std_colormap_return argument was not changed.
Verify that the count_return argument was not changed.

Create a window with XCreateWindow.
Set the RGB_COLOR_MAP property to have size 7 elements.
Obtain the value of the RGB_COLOR_MAP property using XGetRGBColormaps.
Verify that the call returned False.
Verify that the std_colormap_return argument was not changed.
Verify that the count_return argument was not changed.

>>CODE
Status			status;
char			*when;
XStandardColormap	*rscmp = (XStandardColormap *) NULL;
int			rncmp = 0;
XVisualInfo		*vp;
Atom			prop = XA_RGB_BEST_MAP;

	resetvinf(VI_WIN);
	nextvinf(&vp);
	std_colormap_return = &rscmp;
	count_return = &rncmp;
	property = prop;

	w = makewin(display, vp);
/* Property unset */

	rscmp = (XStandardColormap *) -1;
	rncmp = -1;
	status = XCALL;

	when = "when the RGB_BEST_MAP property was not set.";

	if( status != False ) {
		report("%s() did not return False %s", TestName, when);
		FAIL;
	} else
		CHECK;

	if(rscmp != (XStandardColormap *) -1) {
		report("The std_colormap_return argument was updated %s", when);
		FAIL;
	} else
		CHECK;

	if(rncmp != -1) {
		report("The count_return argument was updated %s", when);
		FAIL;
	} else
		CHECK;

	when = "when the RGB_BEST_MAP property format was 8.";

	w = makewin(display, vp);
/* format 8 */
 	XChangeProperty(display, w, prop,  XA_RGB_COLOR_MAP, 8, PropModeReplace, (unsigned char *) &scmp1, 10);

	rscmp = (XStandardColormap *) -1;
	rncmp = -1;
	status = XCALL;

	if( status != False ) {
		report("%s() did not return False %s", TestName, when);
		FAIL; 
	} else
		CHECK;

	if(rscmp != (XStandardColormap *) -1) {
		report("The std_colormap_return argument was updated %s", when);
		FAIL;
	} else
		CHECK;

	if(rncmp != -1) {
		report("The count_return argument was updated %s", when);
		FAIL;
	} else
		CHECK;

	when = "when the RGB_BEST_MAP property type was ATOM.";

	w = makewin(display, vp);
/* type ATOM */
 	XChangeProperty(display, w, prop,  XA_ATOM, 32, PropModeReplace, (unsigned char *) &scmp1, 10);

	rscmp = (XStandardColormap *) -1;
	rncmp = -1;
	status = XCALL;

	if( status != False ) {
		report("%s() did not return False %s.", TestName, when);
		FAIL;
	} else
		CHECK;

	if(rscmp != (XStandardColormap *) -1) {
		report("The std_colormap_return argument was updated %s", when);
		FAIL;
	} else
		CHECK;

	if(rncmp != -1) {
		report("The count_return argument was updated %s", when);
		FAIL;
	} else
		CHECK;

	when = "when the RGB_BEST_MAP property number of elements was 7.";

	w = makewin(display, vp);
/* Bad number of elements */
 	XChangeProperty(display, w, prop,  XA_RGB_COLOR_MAP, 32, PropModeReplace, (unsigned char *) &scmp1, 7);

	rscmp = (XStandardColormap *) -1;
	rncmp = -1;
	status = XCALL;

	if( status != False ) {
		report("%s() did not return False %s.", TestName, when);
		FAIL;
	} else
		CHECK;

	if(rscmp != (XStandardColormap *) -1) {
		report("std_colormap_return argument was updated %s", when);
		FAIL;
	} else
		CHECK;

	if(rncmp != -1) {
		report("The count_return argument was updated %s", when);
		FAIL;
	} else
		CHECK;

	CHECKPASS(12);

>>ASSERTION Good A
When the visualid is not specified in the property colormaps, then the
.M visualid
of the returned structures is set to the default
visual for the screen of the window
.A w .
>>STRATEGY
Create a window using XCreateWindow.
Set the RGB_GRAY_MAP property to have 8 elements using XChangeProperty.
Obtain the default visual ID using DefaultVisual.
Obtain the value of the RGB_GRAY_MAP property using XGetRGBColormaps.
Verify that the value of the visual component of the returned XStandardColormap 
  structure was the same as the default visual.
>>CODE
Status			status;
XStandardColormap	*rscmp = (XStandardColormap *) NULL;
int			rncmp = 0;
XVisualInfo		*vp;
Visual			*vi;
VisualID		vid;
Atom			prop = XA_RGB_GRAY_MAP;

	resetvinf(VI_WIN);
	nextvinf(&vp);
	std_colormap_return = &rscmp;
	count_return = &rncmp;
	property = prop;

	w = makewin(display, vp);
 	XChangeProperty(display, w, prop,  XA_RGB_COLOR_MAP, 32, PropModeReplace, (unsigned char *) &scmp1, 8);

	status = XCALL;
	vi = XDefaultVisual(display, XDefaultScreen(display));
	vid = XVisualIDFromVisual(vi);

	if(status == False) {
		delete("%s() returned False.", TestName);
		return;
	} else
		CHECK;

	if(vid != rscmp->visualid) {
		report("%s() did not report the default visual in the returned visualid component.", TestName);
		FAIL;
	} else
		CHECK;

	XFree((char*)rscmp);

	CHECKPASS(2);

>>ASSERTION Good A
When the killid is not specified in the property colormaps, then the
.M killid
of the returned structures is set to None.
>>STRATEGY
Create a window using XCreateWindow.
Set the RGB_GRAY_MAP property to have 9 elements using XChangeProperty.
Obtain the value of the RGB_GRAY_MAP property using XGetRGBColormaps.
Verify that the value of the killid component of the returned XStandardColormap 
  structure was set to None.
>>CODE
Status			status;
XStandardColormap	*rscmp = (XStandardColormap *) NULL;
int			rncmp = 0;
XVisualInfo		*vp;
Atom			prop = XA_RGB_GRAY_MAP;

	resetvinf(VI_WIN);
	nextvinf(&vp);
	std_colormap_return = &rscmp;
	count_return = &rncmp;
	property = prop;

	w = makewin(display, vp);
 	XChangeProperty(display, w, prop,  XA_RGB_COLOR_MAP, 32, PropModeReplace, (unsigned char *) &scmp1, 9);

	status = XCALL;

	if(status == False) {
		delete("%s() returned False.", TestName);
		return;
	} else
		CHECK;

	if(rscmp->killid != None) {
		report("%s() did not set the returned killid component to None.", TestName);
		FAIL;
	} else
		CHECK;

	XFree((char*)rscmp);

	CHECKPASS(2);

>>ASSERTION Bad A
.ER BadAtom 
>>ASSERTION Bad A
.ER BadWindow 
>># Kieron	Action	Review

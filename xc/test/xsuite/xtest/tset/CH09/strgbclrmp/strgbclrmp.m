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
 * $XConsortium: strgbclrmp.m,v 1.7 94/04/17 21:08:57 rws Exp $
 */
>>TITLE XSetRGBColormaps CH09
void
XSetRGBColormaps(display, w, std_colormap, count, property)
Display			*display = Dsp;
Window			w = DRW(Dsp);
XStandardColormap	*std_colormp = &sclrmp;
int			count = 1;
Atom			property = XA_RGB_DEFAULT_MAP;
>>EXTERN
#include		"Xatom.h"
static XStandardColormap	sclrmp;
static XStandardColormap	scmp1 = { (Colormap)  1,  2L,  3L,  4L,  5L,  6L,  7L,  8L, (VisualID) 9L, (XID) 10L };
>>ASSERTION Good A
A call to xname sets the property, specified by
the
.A property
argument, for the window
.A w
to be of type
.S RGB_COLOR_MAP ,
format 32, and to have value set to the
.A count
.S XStandardColormap
structures specified by the
.A std_colormap
argument.
>>STRATEGY
Create a window using XCreateWindow.
Set the RGB_DEFAULT_MAP property using XSetRGBColormaps.
Obtain the RGB_DEFAULT_MAP property using XGetWindowProperty.
Verify that the property type is RGB_COLOR_MAP.
Verify that the property format is 32.
Verify that property value is correct.
>>CODE
Atom			actual_type;
int			actual_format;
unsigned long		leftover;
unsigned long		nitems;
XVisualInfo		*vp;
XStandardColormap	*rcmap = (XStandardColormap *) NULL;

	resetvinf(VI_WIN);
	nextvinf(&vp);
	w = makewin(display, vp);

	std_colormp = &scmp1;
	XCALL;

	if( XGetWindowProperty(display, w, property, 0L, 40L, False, XA_RGB_COLOR_MAP, &actual_type,
				&actual_format, &nitems, &leftover, (unsigned char **) &rcmap) != Success ) {
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
		report("The format of the RGB_DEFAULT_MAP property was %lu instead of 32", actual_format);
		FAIL;
	} else
		CHECK;

	if(actual_type != XA_RGB_COLOR_MAP) {
		report("The type of the RGB_DEFAULT_MAP property was %lu instead of RGB_DEFAULT_MAP (%lu)", actual_type,
				 (long) XA_RGB_DEFAULT_MAP);
		FAIL;
	} else
		CHECK;

	if( rcmap == (XStandardColormap *) NULL) {
		report("No value was obtained for the RGB_DEFAULT_MAP property.");
		FAIL;
	} else {

		CHECK;
		
		if(nitems != 10) {
			report("The RGB_DEFAULT_MAP property comprised %d elements instead of %d", nitems, 10);
			FAIL;
		} else
			if(actual_format == 32) {
				CHECK;
  				if(rcmap->colormap != scmp1.colormap) {
					report("The colormap component of the XStandardColormap structure %d was incorrect.");
					FAIL;
				} else
					CHECK;
			
				if(rcmap->red_max != scmp1.red_max)  {
					report("The red_max component of the XStandardColormap structure %d was incorrect.");
					FAIL;
				} else
					CHECK;
			
				if(rcmap->red_mult != scmp1.red_mult) {
					report("The red_mult component of the XStandardColormap structure %d was incorrect.");
					FAIL;
				} else
					CHECK;
			
				if(rcmap->green_max != scmp1.green_max) {
					report("The green_max component of the XStandardColormap structure %d was incorrect.");
					FAIL;
				} else
					CHECK;
			
				if(rcmap->green_mult != scmp1.green_mult) {
					report("The green_mult component of the XStandardColormap structure %d was incorrect.");
					FAIL;
				} else
					CHECK;
			
				if(rcmap->blue_max != scmp1.blue_max) {
					report("The blue_max component of the XStandardColormap structure %d was incorrect.");
					FAIL;
				} else
					CHECK;
			
				if(rcmap->blue_mult != scmp1.blue_mult) {
					report("The blue_mult component of the XStandardColormap structure %d was incorrect.");
					FAIL;
				} else
					CHECK;
			
				if(rcmap->base_pixel != scmp1.base_pixel) {
					report("The base_pixel component of the XStandardColormap structure %d was incorrect.");
					FAIL;
				} else
					CHECK;
			
				if(rcmap->visualid != scmp1.visualid) {
					report("The visualid component of the XStandardColormap structure %d was incorrect.");
					FAIL;
				} else
					CHECK;
			
				if(rcmap->killid != scmp1.killid ) {
					report("The killid component of the XStandardColormap structure %d was incorrect.");
					FAIL;
				} else
					CHECK;
			
			}
			XFree((char *) rcmap);
		}

	CHECKPASS(16);

>>ASSERTION Bad A
.ER BadAlloc
>>ASSERTION Bad A
.ER BadAtom
>>ASSERTION Bad A
.ER BadWindow 
>># Kieron	Action	Review

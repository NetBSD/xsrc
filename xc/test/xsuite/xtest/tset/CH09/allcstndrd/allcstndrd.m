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
 * $XConsortium: allcstndrd.m,v 1.7 94/04/17 21:08:28 rws Exp $
 */
>>TITLE XAllocStandardColormap CH09
XStandardColormap *
XAllocStandardColormap()
>>#
>># Added note to mention that the allocated structure can
>># be freed with XFree.
>>#
>># Cal 29/05/91
>>ASSERTION Good A
A call to xname allocates and returns a pointer to a
.S XStandardColormap
structure, which can be freed with
.S XFree
in which each component
is set to zero.
>>STRATEGY
Allocate a XStandardColormap using XAllocStandardColormap.
Verify that the function did not return NULL.
Verify that each component of the XStandardColormap structure was 0.
Release the allocated structure using XFree.
>>CODE
XStandardColormap	*scmp = (XStandardColormap *) NULL;

	scmp = XAllocStandardColormap();

	if(scmp == (XStandardColormap *) NULL) {
		delete("%s() returned NULL.", TestName);
		return;
	} else
		CHECK;

	if(scmp->colormap != (Colormap) 0) {
		report("The colormap component of the XStandardColormap structure was non-zero.");
		FAIL;
	} else
		CHECK;

	if(scmp->red_max != 0L) {
		report("The red_max component of the XStandardColormap structure was non-zero.");
		FAIL;
	} else
		CHECK;

	if(scmp->red_mult != 0L) {
		report("The red_mult component of the XStandardColormap structure was non-zero.");
		FAIL;
	} else
		CHECK;

	if(scmp->green_max != 0L) {
		report("The green_max component of the XStandardColormap structure was non-zero.");
		FAIL;
	} else
		CHECK;

	if(scmp->green_mult != 0L) {
		report("The green_mult component of the XStandardColormap structure was non-zero.");
		FAIL;
	} else
		CHECK;

	if(scmp->blue_max != 0L) {
		report("The blue_max component of the XStandardColormap structure was non-zero.");
		FAIL;
	} else
		CHECK;

	if(scmp->blue_mult != 0L) {
		report("The blue_mult component of the XStandardColormap structure was non-zero.");
		FAIL;
	} else
		CHECK;

	if(scmp->base_pixel != 0L) {
		report("The base_pixel component of the XStandardColormap structure was non-zero.");
		FAIL;
	} else
		CHECK;

	if(scmp->visualid != (VisualID) 0) {
		report("The visualid component of the XStandardColormap structure was non-zero.");
		FAIL;
	} else
		CHECK;

	if(scmp->killid != (XID) 0) {
		report("The killid component of the XStandardColormap structure was non-zero.");
		FAIL;
	} else
		CHECK;

	XFree((char*)scmp);

	CHECKPASS(11);

>>ASSERTION Bad B 1
When insufficient memory is available, then
a call to xname returns NULL.
>># Kieron	Action	Review

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
 * $XConsortium: crtrgn.m,v 1.9 94/04/17 21:09:25 rws Exp $
 */
>>TITLE XCreateRegion CH10
Region
XCreateRegion()
>>ASSERTION Good A
A call to xname returns a new, empty region.
>>STRATEGY
Create a region using xname.
Verify that the call did not return NULL.
Verify that the region is empty using XEmptyRegion.
Obtain the smallest rectangle enclosing the region using XClipBox.
Verify that the returned rectangle has width and height zero.
Union a rectangle with the region using XUnionRectWithRegion.
Obtain the smallest rectangle bounding the region using XClipBox.
Verify that the bounding box is the same as the original rectangle.
Destroy the region using XDestroyRegion.
>>CODE
static	XRectangle	rect = { -99, 666, 198, 3 };
XRectangle		bbox;
Region			reg;

	reg = XCALL;

	if(reg == (Region) NULL) {
		delete("%s() returned NULL.", TestName);
		return;
	} else
		CHECK;

	if( XEmptyRegion(reg) != True) {
		report("XEmptyRegion() returned True.");
		FAIL;
	} else
		CHECK;

	XClipBox(reg, &bbox);
	if( (bbox.width != 0) || (bbox.height != 0)) {
		report("%s() returned a region of width %d and height %d instead of 0.", TestName, bbox.width, bbox.height);
		FAIL;
	} else
		CHECK;

	XUnionRectWithRegion(&rect, reg, reg);
	XClipBox(reg, &bbox);	
	
	if((rect.x != bbox.x) || (rect.y != bbox.y) || (rect.width != bbox.width) || (rect.height != bbox.height) ) {
		report("%s() returned the rectangle x %d, y %d, height %d, width %d, instead of x %d, y %d, height %d, width %d.",
                        "XClipBox",
			rect.x, rect.y, rect.width, rect.height,
			bbox.x, bbox.y, bbox.width, bbox.height );
		FAIL;
	} else
		CHECK;

	XDestroyRegion(reg);
	CHECKPASS(4);

>>ASSERTION Bad B 1
When sufficient storage cannot be allocated, 
then a call to xname returns NULL.

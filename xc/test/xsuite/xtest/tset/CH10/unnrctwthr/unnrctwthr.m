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
 * $XConsortium: unnrctwthr.m,v 1.8 94/04/17 21:10:37 rws Exp $
 */
>>TITLE XUnionRectWithRegion CH10

XUnionRectWithRegion(rectangle, src_region, dest_region_return)
XRectangle	*rectangle;
Region		src_region;
Region		dest_region_return;
>>ASSERTION Good A
A call to xname computes the union of the rectangle
.A rectangle
and the region
.A src_region
and stores the result in
.A dest_region_return .
>>STRATEGY
Create a region using XCreateRegion.
Set the region to a rectangle using xname.
Obtain the extents of the region using XClipBox.
Verify that the position and dimension of the region is the same as that of the rectangle.
>>CODE
static XRectangle	rect = { 7,9, 23,37 };
XRectangle		rrect;
Region 			R1;


	R1 = makeregion();
	rectangle = &rect;
	src_region = R1;
	dest_region_return = R1;
	XCALL;

	XClipBox(R1, &rrect);

	if( (rect.x != rrect.x) || (rect.y != rrect.y) || (rect.width != rrect.width) || (rect.height != rrect.height)) {
		report("%s() set a rectangle x - %d y - %d width - %d height - %d",
			 TestName,
			 rrect.x, rrect.y, rrect.width, rrect.height);
		report("instead of x - %d y - %d width - %d height - %d.",
			 rect.x, rect.y, rect.width, rect.height);

		FAIL;
	} else
		PASS;

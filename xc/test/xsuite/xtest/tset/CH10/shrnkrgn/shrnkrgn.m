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
 * $XConsortium: shrnkrgn.m,v 1.7 94/04/17 21:10:32 rws Exp $
 */
>>TITLE XShrinkRegion CH10

XShrinkRegion(r, dx, dy)
Region	r;
int	dx;
int	dy;
>>#
>># COMMENT :	I think there ought to be some sort of assertion about
>>#		the Xlib's modification of a region's offsets to keep 
>>#		the center of the region in the same position.
>># Cal.
>>#
>># REPLY:	MIT asked us to reword the assertion, and I attempted to do
>>#		this in a simple manner as below. I am sure I haven't covered
>>#		your point fully, but since the assertions are now approved,
>>#		we should test just these assertions.
>># Dave
>>#
>># COMMENT :	These new assertions prohibit the testing of enlargement of a region
>>#		on one axis and contraction on the other.
>># Cal.
>>#
>>ASSERTION Good A
When 
.A dx 
and 
.A dy 
are zero, then a call to xname does not change
the size of region 
.A r .
>>STRATEGY
Create two regions using XCreateRegion.
Verify that the calls did not return NULL.
Set the regions to the same rectangle using XUnionRectWithRegion.
Shrink the first region with parameters dx = dy = 0.
Verify that the two regions are identical using XEqualRegion.
>>CODE
Region			R1;
Region			R2;
static XRectangle	rect = { 7,5, 23, 45};

	if( ((R1 = XCreateRegion()) == (Region) NULL) ||
	    ((R2 = XCreateRegion()) == (Region) NULL) ){
		delete("XCreateRegion() returned NULL.");
		return;
	} else
		CHECK;

	XUnionRectWithRegion(&rect, R1, R1);
	XUnionRectWithRegion(&rect, R2, R2);

	r = R1;
	dx = dy = 0;
	XCALL;

	if( XEqualRegion(R1, R2) != True ) {
		report("%s() changed a region when dx and dy were zero.", TestName);
		FAIL;
	} else
		PASS;

	XDestroyRegion(R1);	
	XDestroyRegion(R2);

>>ASSERTION Good A
When 
.A dx 
and 
.A dy 
are both positive, and a point x, y is outside region 
.A r ,
then call to xname changes the size of the region 
.A r 
such that
the point x, y remains outside region 
.A r ,
and some point x1, y1 previously
inside region 
.A r 
is now outside region 
.A r .
>>STRATEGY
Create regions R1, R2 and R3 using XCreateRegion.
Verify that none of the calls returned NULL.
Set region R1 to a rectangle using XUnionRectWithRegion.
Copy region R1 to R2 using XUnionRegion.
Shrink R2 using xname.
Set R3 to the difference of the union and intersection of R1 and R2 using XXorRegion.
Verify that R3 is not empty using XEmptyRegion.
Set R3 to be the union of R2 and R3 using XUnionRegion.
Verify that R3 is the same as R1 using XEqualRegion.
>>CODE
static XRectangle	rect = { 5,7, 100, 200};
Region			R1;
Region			R2;
Region			R3;

	R1 = XCreateRegion();
	R2 = XCreateRegion();
	R3 = XCreateRegion();

	if( (R1 == (Region) NULL) || (R2 == (Region) NULL) || R3 == (Region) NULL) {
		delete("XCreateRegion() returned NULL.");
		return;
	} else
		CHECK;

	XUnionRectWithRegion(&rect, R1, R1);
	XUnionRegion(R1, R1, R2);

	dx = dy = 1;
	r = R2;
	XCALL;	

	XXorRegion(R1, R2, R3);

	if(XEmptyRegion(R3) == True) {
		report("%s() did not change the size of the source region.", TestName);
		FAIL;
		return;
	} else
		CHECK;

	XUnionRegion(R2, R3, R3);

	if( XEqualRegion(R1, R3) == False) {
		report("%s() did not produce a shrunk region contained in its source.", TestName);
		FAIL;
	} else
		CHECK;

	XDestroyRegion(R1);
	XDestroyRegion(R2);
	XDestroyRegion(R3);

	CHECKPASS(3);

>>ASSERTION Good A
When 
.A dx 
and 
.A dy 
are both negative, and a point x, y is in region 
.A r ,
then call to xname changes the size of the region
.A r 
such that
the point x, y remains in region
.A r ,
and some point x1, y1 previously
outside region
.A r 
is now inside region
.A r .
>>STRATEGY
Create regions R1, R2 and R3 using XCreateRegion.
Verify that none of the calls returned NULL.
Set region R1 to a rectangle using XUnionRectWithRegion.
Copy region R1 to R2 using XUnionRegion.
Enlarge R2 using xname.
Set R3 to the difference of the union and intersection of R1 and R2 using XXorRegion.
Verify that R3 is not empty using XEmptyRegion.
Set R3 to be the union of R2 and R3 using XUnionRegion.
Verify that R3 is the same as R2 using XEqualRegion.
>>CODE
static XRectangle	rect = { 5,7, 100, 200};
Region			R1;
Region			R2;
Region			R3;

	R1 = XCreateRegion();
	R2 = XCreateRegion();
	R3 = XCreateRegion();

	if( (R1 == (Region) NULL) || (R2 == (Region) NULL) || R3 == (Region) NULL) {
		delete("XCreateRegion() returned NULL.");
		return;
	} else
		CHECK;

	XUnionRectWithRegion(&rect, R1, R1);
	XUnionRegion(R1, R1, R2);

	dx = dy = -1;
	r = R2;
	XCALL;	

	XXorRegion(R1, R2, R3);

	if(XEmptyRegion(R3) == True) {
		report("%s() did not change the size of the source region.", TestName);
		FAIL;
		return;
	} else
		CHECK;

	XUnionRegion(R2, R3, R3);

	if( XEqualRegion(R2, R3) == False) {
		report("%s() did not produce an enlarged region containing its source.", TestName);
		FAIL;
	} else
		CHECK;

	XDestroyRegion(R1);
	XDestroyRegion(R2);
	XDestroyRegion(R3);

	CHECKPASS(3);

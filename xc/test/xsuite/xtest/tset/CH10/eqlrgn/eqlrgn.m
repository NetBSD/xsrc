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
 * $XConsortium: eqlrgn.m,v 1.6 94/04/17 21:09:30 rws Exp $
 */
>>TITLE XEqualRegion CH10
Bool
XEqualRegion(r1, r2)
Region	r1;
Region	r2;
>>EXTERN
static	XRectangle	rect1 = {0,0, 7,13 };
static	XRectangle	rect2 = {5,5, 10,15 };
static	XRectangle	rect3 = {0,0, 17,20 };
static	XRectangle	rect4 = {-1,-1, 1,1 };
>>ASSERTION Good A
When the regions
.A r1
and
.A r2
have the same offset, size and shape, then a call to xname returns
.S True.
>>STRATEGY
Create two regions using XCreateRegion.
Set one region to the union of three rectangles one of which covers the other two using XUnionRectWithRegion.
Set the other region to the all encompassing region using XUnionRectWithRegion.
Verify that the call to xname returns True.
>>CODE
Bool	res;
Region	R1;
Region	R2;

	R1 = makeregion();
	R2 = makeregion();
	if(isdeleted()) return;

	XUnionRectWithRegion(&rect1, R1, R1);
	XUnionRectWithRegion(&rect3, R1, R1);
	XUnionRectWithRegion(&rect2, R1, R1);
	XUnionRectWithRegion(&rect3, R2, R2);

	r1 = R1;
	r2 = R2;
	res = XCALL;

	if(res != True) {
		report("%s() did not return True for two equal regions.", TestName);
		FAIL;
	} else
		PASS;

>>ASSERTION Good A
When the regions
.A r1
and
.A r2
do not have the same offset, then a call to xname returns
.S False .
>>STRATEGY
Create regions R1 R2 using XCreateRegion.
Set R1 and R2 to the same region using XUnionRectWithRegion.
Offset R2 using XOffsetRegion.
Verify that a call to xname returns False.
>>CODE
Bool	res;
Region	R1;
Region	R2;

	R1 = makeregion();
	R2 = makeregion();

	if(isdeleted()) return;

	XUnionRectWithRegion(&rect1, R1, R1);
	XUnionRectWithRegion(&rect1, R2, R2);
	XOffsetRegion(R2,1,0);

	r1 = R1;
	r2 = R2;
	res = XCALL;

	if(res != False) {
		report("%s() did not return False for regions with a different offset.", TestName);
		FAIL;
	} else
		PASS;

>>ASSERTION Good A
When the regions
.A r1
and
.A r2
do not have the same size, then a call to xname returns
.S False .
>>STRATEGY
Create regions R1 R2 using XCreateRegion.
Set R1 and R2 to the same region using XUnionRectWithRegion.
Change the size of R2 using XShrinkRegion.
Verify that a call to xname returns False.
>>CODE
Bool	res;
Region	R1;
Region	R2;

	R1 = makeregion();
	R2 = makeregion();

	if(isdeleted()) return;

	XUnionRectWithRegion(&rect1, R1, R1);
	XUnionRectWithRegion(&rect1, R2, R2);
	XShrinkRegion(R2, 1, 0);

	r1 = R1;
	r2 = R2;
	res = XCALL;

	if(res != False) {
		report("%s() did not return False for regions with a different size.", TestName);
		FAIL;
	} else
		PASS;

>>ASSERTION Good A
When the regions
.A r1
and
.A r2
do not have the same shape, then a call to xname returns
.S False .
>>STRATEGY
Create regions R1 R2 using XCreateRegion.
Set R1 and R2 to the same region using XUnionRectWithRegion.
Change R2 with disjoint rectangle using XUnionRectWithRegion.
Verify that a call to xname returns False.
>>CODE
Bool	res;
Region	R1;
Region	R2;

	R1 = makeregion();
	R2 = makeregion();

	if(isdeleted()) return;

	XUnionRectWithRegion(&rect1, R1, R1);
	XUnionRectWithRegion(&rect1, R2, R2);
	XUnionRectWithRegion(&rect4, R2, R2);

	r1 = R1;
	r2 = R2;
	res = XCALL;

	if(res != False) {
		report("%s() did not return False for two different regions.", TestName);
		FAIL;
	} else
		PASS;

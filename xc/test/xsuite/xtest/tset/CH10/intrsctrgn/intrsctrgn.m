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
 * $XConsortium: intrsctrgn.m,v 1.7 94/04/17 21:09:38 rws Exp $
 */
>>TITLE XIntersectRegion CH10

XintersectRegion(sra, srb, dr_return)
Region	sra;
Region	srb;
Region	dr_return;
>>ASSERTION Good A
A call to xname computes the intersection of the regions
.A sra
and
.A srb
and replaces the value of
.A dr_return
with the result.
>>STRATEGY
Create regions R1, R2 and R3 using XCreateRegion.
Verify that none of the calls returned NULL.
Set R1 to a polygon using XPolygonRegion.
Set R2 to a polygon which intersects the first using XPolygonRegion.
Obtain the intersection of R1 and R2 into R3 using xname.
Verify that the result is not empty using XEmptyRegion.

Test that ((R1 ^ R2) | (R1 & R2)) == (R1 | R2):
   Obtain union of R1 and R2 into Ru using XUnionRegion.
   Obtain exclusive-or of R1 and R2 into Rx using XXorRegion.
   Obtain union of Rx and R3 using XUnionRegion.
   Verify that result equals Ru using XEqualRegion.

Create three regions using XCreateRegion.
Set two of the regions to disjoint polygons using XPolygonRegion.
Obtain the intersection of the regions using xname.
Verify that the returned region was empty using XEmptyRegion.
>>CODE
static	XPoint	noninter1[] = { {-25,-5}, {-15,-10}, {-10,-25}, {-20,-35}, {-30,-30}, {-35,-20}, {-30,-10} };
static	XPoint	inter1[] = { {25,5}, {15,10}, {10,25}, {20,35}, {30,30}, {35,20}, {30,10} };
static	XPoint	inter2[] = { {25,20}, {37,22}, {40,30}, {35,37}, {30,40}, {25,38}, {18,32}, {20,25} };
Region		R1;
Region		R2;
Region		Ri;
Region		Rt;
Region		Ru;
Region		Rx;

	R1 = makeregion();
	R2 = makeregion();
	Ri = makeregion();
	Rt = makeregion();
	Ru = makeregion();
	Rx = makeregion();

	if(isdeleted()) return;

	R1 = XPolygonRegion( inter1, NELEM(inter1), WindingRule );
	R2 = XPolygonRegion( inter2, NELEM(inter2), WindingRule );
	XUnionRegion(R1, R2, Ru);

	sra = R1;
	srb = R2;
	dr_return = Ri;	
	XCALL;

	if( XEmptyRegion(Ri) == True ) {
		report("%s() returned the empty region for two intersecting regions.", TestName);
		FAIL;
	} else
		CHECK;

	XXorRegion(R1, R2, Rx);
	XUnionRegion(Rx, Ri, Rt);
	
	if(XEqualRegion(Ru, Rt) != True) {
		report("%s() returned a region not completely contained in a source region.", TestName);
		FAIL;
	} else
		CHECK;

	R1 = makeregion();
	R2 = makeregion();
	Ri = makeregion();

	sra = R1;
	srb = R2;
	dr_return = Ri;	
	XCALL;

	if( XEmptyRegion(Ri) != True ) {
		report("%s() returned a non-empty region for two non-intersecting intersecting regions.", TestName);
		FAIL;
	} else
		CHECK;

	CHECKPASS(3);

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
 * $XConsortium: unnrgn.m,v 1.7 94/04/17 21:10:38 rws Exp $
 */
>>TITLE XUnionRegion CH10

XUnionRegion(sra, srb, dr_return)
Region		sra;
Region		srb;
Region		dr_return;
>>ASSERTION Good A
A call to xname computes the union of the regions
.A sra
and
.A srb
and places the result in the region
.A dr_return .
>>STRATEGY
Create regions R1 R2 R3 using XCreateRegion.
Set regions R1 and R2 to right-angled triangles joined along the hypotenuse using XPolygonRegion.
Set R3 to the rectangle formed by joining the triangles using XUnionRectWithRegion.
Obtain the union of R1 and R2 using xname.
Verify that union is the same as R3 using XEqualRegion.

Create a region using XCreateRegion.
Set the region to an overlapping set of 3x3 rectangular regions using xname.
Verify that the result is the correct rectangular region using XEqualRegion.
>>CODE
int			i;
int			j;
Region			R1;
Region			R2;
Region			R3;
Region			Rt;
static XPoint		tri1[] = { {5,5}, {30,5}, {5,20} };
static XPoint		tri2[] = { {5,20}, {30,20}, {30,5} };
static XRectangle	rect1 = { 5,5, 25,15};
XRectangle		rect2;
XRectangle		rrect;

	R1 = makeregion();
	R2 = makeregion();
	R3 = makeregion();
	Rt = makeregion();

	if(isdeleted()) return;

	R1 = XPolygonRegion(tri1, NELEM(tri1), WindingRule);
	R2 = XPolygonRegion(tri2, NELEM(tri2), WindingRule);

	sra = R1;
	srb = R2;
	dr_return = R3;
	XCALL;

	XUnionRectWithRegion(&rect1, Rt, Rt);

	if( XEqualRegion(Rt, R3) != True) {
		report("%s() did not unify two triangular regions into the correct rectangular region.", TestName);
		FAIL;
	} else
		CHECK;


	R2 = makeregion();

	srb = R2;
	dr_return = R2;
	rect2.width=3;
	rect2.height=3;
	for(j=5; j<18; j++) {
		rect2.y=j;
		for(i=5; i<28; i++) {

			if((R1 = XCreateRegion()) == (Region) NULL) {
				delete("XCreateRegion() returned NULL.");
				return;
			} else
				if(i==27 && j==17) CHECK;

			rect2.x=i;
			XUnionRectWithRegion(&rect2, R1, R1);
			sra = R1;
			XCALL;
			XDestroyRegion(R1);
		}
	}
	
	if( XEqualRegion(Rt, R2) != True) {
		report("%s() did not unify a set of over-lapping rectangular regions into the correct rectangular region.", TestName);
		FAIL;
	} else
		CHECK;

	CHECKPASS(3);

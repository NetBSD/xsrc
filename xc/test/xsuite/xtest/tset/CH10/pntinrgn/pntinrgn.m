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
 * $XConsortium: pntinrgn.m,v 1.8 94/04/17 21:09:54 rws Exp $
 */
>>TITLE XPointInRegion CH10
Bool
XPointInRegion(r, x, y)
Region	r;
int	x;
int	y;
>>EXTERN
static	XPoint	lines[] = { {20,20}, {35,10}, {55,10}, {80,20}, {90,35}, {85,50}, {75,65}, {50,70}, {30,65}, {20,60}, {15,40}, {20,20} };
static	XPoint	ply1[] = { {20,20}, {35,10}, {55,10}, {80,20}, {90,35}, {85,50}, {75,65}, {50,70}, {30,65}, {20,60}, {15,40} };

>>ASSERTION Good A
When the point specified by the
.A x
and
.A y
arguments is contained in the region
.A r ,
then a call to xname returns
.S True .
>>STRATEGY
Create a region using XCreateRegion.
Set the region to a polygon using XPolygonRegion.
Obtain the smallest rectangle covering the region using XClipBox.
For each  point in the covered area:
   Determine whether the point is in the region using xname.
   Obtain the union of a region formed from the point and the region.
   If the point is in the region:
      Verify that the union is the same as the initial region using XEqualRegion.
>>CODE
Region		R;
Region		Rp;
Bool		res;
XRectangle	rrect;
int		maxx;
int		maxy;

	R = makeregion();
	if(isdeleted()) return;
	R = XPolygonRegion(ply1, NELEM(ply1), WindingRule);
	
	XClipBox(R, &rrect);

	rrect.height = 1;
	rrect.width  = 1;
	r = R;
	maxx = 10 + rrect.x + rrect.width;
	maxy = 10 + rrect.y + rrect.height;
	for(x = rrect.x - 10; x <= maxx; x++) {
		rrect.x = x;
		for(y = rrect.y - 10; y <= maxy; y++) {
			rrect.y = y;
			res = XCALL;
			Rp = XCreateRegion();
			XUnionRectWithRegion(&rrect, Rp, Rp);
			XUnionRegion(R, Rp, Rp);

			if(XEqualRegion(R, Rp) == True) {
				if(res != True) {
					report("%s() did not return True",
						TestName);
					report("when point %d %d was in the specified region",
						x, y);
					FAIL;
				} 
			} 
			if((x == maxx) && (y == maxy))
				CHECK;
			XDestroyRegion(Rp);
		}
	}

	CHECKPASS(1);

>>ASSERTION Good A
When the point specified by the
.A x
and
.A y
arguments is not contained in the region
.A r ,
then a call to xname returns
.S False .
>>STRATEGY
Create a region using XCreateRegion.
Set the region to a polygon using XPolygonRegion.
Obtain the smallest rectangle covering the region using XClipBox.
For each  point in the covered area:
   Determine whether the point is in the region using xname.
   Obtain the union of a region formed from the point and the region.
   If the point is not in the region:
      Verify that the union is not the same as the initial region using XEqualRegion.
>>CODE
Region		R;
Region		Rp;
Bool		res;
XRectangle	rrect;
int		maxx;
int		maxy;

	R = makeregion();
	if(isdeleted()) return;
	R = XPolygonRegion(ply1, NELEM(ply1), WindingRule);
	
	XClipBox(R, &rrect);

	rrect.height = 1;
	rrect.width  = 1;
	r = R;
	maxx = 10 + rrect.x + rrect.width;
	maxy = 10 + rrect.y + rrect.height;
	for(x = rrect.x - 10; x <= maxx; x++) {
		rrect.x = x;
		for(y = rrect.y - 10; y <= maxy; y++) {
			rrect.y = y;
			res = XCALL;
			Rp = XCreateRegion();
			XUnionRectWithRegion(&rrect, Rp, Rp);
			XUnionRegion(R, Rp, Rp);

			if(XEqualRegion(R, Rp) != True) {
				if(res != False) {
					report("%s() did not return False",
						TestName);
					report("when point %d %d was not in the specified region",
						x, y);
					FAIL;
				}
			}
			if((x == maxx) && (y == maxy))
					CHECK;
			XDestroyRegion(Rp);
		}
	}

	CHECKPASS(1);


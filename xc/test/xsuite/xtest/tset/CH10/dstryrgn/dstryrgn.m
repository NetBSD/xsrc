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
 * $XConsortium: dstryrgn.m,v 1.5 94/04/17 21:09:29 rws Exp $
 */
>>TITLE XDestroyRegion CH10

XDestroyRegion(r)
Region	r;
>>ASSERTION Bad B 1
A call to xname deallocates the storage associated with the region
.A r .
>>STRATEGY
Create a gc using XCreateGC.
For 20 iterations:
   Create a region using XPolygonRegion.
   Set the gc clip mask to the region using XSetRegion.
   Destroy the region using xname.
>>CODE
int		i;
int		loopcount = 20;
GC		gc;
static int	rule[] = { EvenOddRule, WindingRule };
static XPoint	points[] = { 	{20,20}, {35,10}, {55,10}, {80,20}, {90,35}, {85,50}, {75,65}, {50,70}, {30,65}, {20,60}, {15,40},
				{75,60}, {60,45}, {55,60}, {30,55}, {35,35}, {15,40}, {20,18}, {45,10}, {70,15}, {55,30}, {80,45},
				{85,40}, {30,20}, {20,55}, {30,69}, {55,35}, {85,44}, {10,43}, {14,66} };

	gc = makegc(Dsp, DRW(Dsp));
	for(i=0; i<loopcount; i++) {
		r = XPolygonRegion(points, NELEM(points), rule[i % 2]);
		XSetRegion(Dsp, gc, r);
		XCALL;
		CHECK;
	}

	CHECKUNTESTED(loopcount);

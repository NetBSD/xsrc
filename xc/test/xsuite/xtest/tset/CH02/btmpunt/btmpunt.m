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
 * $XConsortium: btmpunt.m,v 1.7 94/04/17 21:02:00 rws Exp $
 */
>>SET   macro
>>TITLE XBitmapUnit CH02
int
XBitmapUnit(display)
Display	*display = Dsp;
>>ASSERTION Good A
A call to xname returns an integer 8, 16, or 32  which is less than the 
bitmap pad value and which is the size in bits of a scanline unit belonging to bitmaps.
>>STRATEGY
Obtain the bitmap unit from a XImage structure using XGetImage.
Obtain the bitmap unit using xname.
Verify that the values are the same.
>>CODE
int	bunt;
int	pad;
XImage	*xi;

	bunt = XCALL;	

	if( (bunt != 8) && (bunt != 16) && (bunt != 32) ) {
		report("%s() returned illegal value %d.", TestName, bunt);
		FAIL;
	} else
		CHECK;

	pad = XBitmapPad(display);

	if( bunt > pad ) {
		report("%s() returned %d which is not less than the bitmap pad of %d.", TestName, bunt, pad);
		FAIL;
	} else
		CHECK;

	xi = XGetImage(display, DRW(display), 0,0, 1,1, XAllPlanes(), ZPixmap);

	if(xi->bitmap_unit != bunt) {
		report("%s() returned %d instead of %d.", TestName, bunt, xi->bitmap_unit);
		FAIL;
	} else
		CHECK;

	CHECKPASS(3);

	XFree((char *) xi);

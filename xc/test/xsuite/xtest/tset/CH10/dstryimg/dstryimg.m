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
 * $XConsortium: dstryimg.m,v 1.4 94/04/17 21:09:28 rws Exp $
 */
>>TITLE XDestroyImage CH10
int
XDestroyImage(ximage)
XImage	*ximage;
>>ASSERTION Good B 1
A call to xname frees both the structure and image data specified by the
.A ximage
argument.
>>STRATEGY
For 20 iterations:
   Create an image using XGetImage on the root window.
   Access the region data using XAddPixel.
   Free the structure using xname.
   Create an image using XCreateImage.
   Free the structure using xname.
>>CODE
int 	i;
int	bp;
static  int	fmat[2] = {XYPixmap, ZPixmap};

	bp = BitmapPad(Dsp);

	for(i=0; i<20; i++) {

		ximage = XGetImage(Dsp, RootWindow(Dsp, 0), 0,0, 12,13, AllPlanes, fmat[ i%2 ]);
		if(ximage == (XImage *) NULL){
			delete("XGetImage() returned NULL.");
			return;
		} else
			if(i == 19)
				CHECK;
		
		XAddPixel(ximage, 1<<29 - 7);
		XCALL;

		ximage = XCreateImage(Dsp, DefaultVisual(Dsp, 0), 32, fmat [ 1 - i%2 ], 5, (char *) NULL, 13,23, 32, 0);

		if(ximage == (XImage *) NULL){
			delete("XCreateImage() returned NULL.");
			return;
		} else
			if(i == 19)
				CHECK;

		XCALL;

	} 

	CHECKUNTESTED(2);

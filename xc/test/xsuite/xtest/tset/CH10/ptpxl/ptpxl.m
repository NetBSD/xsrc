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
 * $XConsortium: ptpxl.m,v 1.5 94/04/17 21:09:58 rws Exp $
 */
>>TITLE XPutPixel CH10
int
XPutPixel(ximage, x, y, pixel)
XImage		*ximage;
int		x;
int		y;
unsigned long	pixel;
>>EXTERN
int bltimage(im, w, h, dep)
XImage		*im;
unsigned int	w;
unsigned int	h;
int		dep;
{
int		i;
unsigned long	mask;
unsigned long	rpixel;

	ximage = im;
	mask = (1<<dep) - 1;
	for(y=0; y<h; y+=10)
		for(x=0; x<w; x+=10) {
			for(i = 0; i <= (15 & mask); i++) {
				pixel =  mask & (i | i<<4 | i<<12 | i<<20);
				startcall(Dsp);
				if (isdeleted())
					return(0);
				ValueReturn = XPutPixel(ximage, x, y, pixel);
				if (ValueReturn == 0) {
					report("%s() returned 0", TestName);
					return(0);
				}
				endcall(Dsp);
				if (geterr() != Success) {
					report("Got %s, Expecting Success", errorname(geterr()));
					return(0);
				}
				if((rpixel = XGetPixel(ximage, x, y)) != pixel) {
					report("XGetPixel() returned %lx instead of %lx.", rpixel, pixel);
					return(0);
				}
			}
		}

	return(1);
}		
>>ASSERTION Good A
When the image
.A ximage
contains the coordinate
.A x,y ,
and the
.A pixel
argument is in normalised format, then a call to xname overwrites
.A x,y
with the value
.A pixel .
>>STRATEGY
For all supported drawables:
   Create a drawable.
   For XYPixmap and ZPixmap:
      Obtain an XImage structure using XGetImage.
      For a range of pixel values over the drawable's depth:
         Set pixels using xname.
         Verify that the pixels are correctly set using XGetPixel.
>>CODE
XVisualInfo	*vi;
int		npix;
unsigned int	width;
unsigned int	height;
Pixmap		pm;
Window		win;
int		i;
static int	fmats[2] = { XYPixmap, ZPixmap };
	

	for(resetvinf(VI_PIX); nextvinf(&vi);) {

		pm = makepixm(Dsp, vi);
		getsize(Dsp, pm, &width, &height);
		width = width > 17 ? 17 : width;
		height = height > 19  ? 19 : height;

		for(i=0; i<2; i++) {
			ximage = XGetImage(Dsp, pm, 0,0, width, height, AllPlanes, fmats[i]);
			if( ximage == (XImage *) NULL ) {
				delete("XGetImage() returned NULL.");
			} else {
				if(bltimage(ximage, width, height, vi->depth) == 0) 
					FAIL;
				else
					CHECK;

				XDestroyImage(ximage);
			}

		}
	}
	npix = nvinf();

	for(resetvinf(VI_WIN); nextvinf(&vi);) {

		win = makewin(Dsp, vi);
		getsize(Dsp, win, &width, &height);
		width = width > 17 ? 17 : width;
		height = height > 19  ? 19 : height;

		for(i=0; i<2; i++) {
			ximage = XGetImage(Dsp, win, 0,0, width, height, AllPlanes, fmats[i]);
			if(ximage == (XImage *) NULL) {
				delete("XGetImage() returned NULL.");
			} else {
				if(bltimage(ximage, width, height, vi->depth) == 0) 
					FAIL;
				else
					CHECK;

				XDestroyImage(ximage);
			}
		}

	}

	CHECKPASS(2 * (npix + nvinf()));

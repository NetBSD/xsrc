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
 * $XConsortium: addpxl.m,v 1.6 94/04/17 21:09:21 rws Exp $
 */
>>TITLE XAddPixel CH10

XAddPixel(ximage, value)
XImage	*ximage;
long	value;
>>EXTERN
mpattern(display, d, w, h, dep)
Display		*display;
Drawable	d;
unsigned int	w;
unsigned int	h;
int		dep;
{
int		i;
int		j;
int		mod;
GC		gc;
unsigned long	val;

	gc = makegc(display, d);
	mod = 1<<dep;
	val = 0;
	for(j=0; j<h; j++)
		for(i=0; i<w; i++) {
			XSetForeground(display, gc, val);
			XDrawPoint(display, d, gc, i, j);
			val = (val + 1) % mod;
		}
}

int
mcheck(xi, w, h, dep)
XImage		*xi;
unsigned int	w;
unsigned int	h;
int		dep;
{
int		i;
int		j;
int		mod;
unsigned long	val;

	mod = 1<<dep;
	val = value;
	for(j=0; j<h; j++)
		for(i=0; i<w; i++) {
			if(XGetPixel(ximage, i, j) != val){
				return 0;
			}
			val = (val + 1) % mod;
		}
	return 1;
}
>>ASSERTION Good A
A call to xname adds the
.A value
argument to every pixel in the
.A ximage
argument.
>>STRATEGY
For all supported drawables:
   Create a drawable.
   Initialise the drawable's pixels.
   For ZPixmap and XYPixmap:
	   Obtain an ximage from the drawable using XGetImage.
	   Add the drawables depth - 1 to every image pixel using xname.
	   Verify that the ximage pixels all set correctly using XGetPixel.
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
		mpattern(Dsp, pm, width, height, vi->depth);

		for(i=0; i<2; i++) {
			ximage = XGetImage(Dsp, pm, 0,0, width, height, AllPlanes, fmats[i]);
			if( ximage == (XImage *) NULL ) {
				delete("XGetImage() returned NULL.");
				return;
			} else {

				value = (1 << vi->depth) - 1;
				XCALL;
				if(mcheck(ximage, width, height, vi->depth) == 0) {
					report("XImage structure was not correct.");
					FAIL;
				} else
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
		mpattern(Dsp, win, width, height, vi->depth);

		for(i=0; i<2; i++) {
			ximage = XGetImage(Dsp, win, 0,0, width, height, AllPlanes, fmats[i]);
			if(ximage == (XImage *) NULL) {
				delete("XGetImage() returned NULL.");
				return;
			} else {
				value = (1 << vi->depth) - 1;
				XCALL;
				if(mcheck(ximage, width, height, vi->depth) == 0) {
					report("XImage structure was not correct.");
					FAIL;
				} else
					CHECK;

				XDestroyImage(ximage);
			}
		}

	}

	CHECKPASS(2 * (npix + nvinf()));

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
 * $XConsortium: savimage.c,v 1.13 94/04/17 21:01:04 rws Exp $
 */

#include	"stdio.h"
#include	"xtest.h"
#include	"Xlib.h"
#include	"Xutil.h"
#include	"xtestlib.h"
#include	"pixval.h"

extern	int 	Errnum;	/* Number of error record */

/*
 * Save an image of the drawable.  Basically this is just a XGetImage
 * on the whole area of the drawable.
 */
XImage *
savimage(disp, d)
Display	*disp;
Drawable	d;
{
unsigned int 	width;
unsigned int 	height;
XImage	*imp;

	getsize(disp, d, &width, &height);
	imp = XGetImage(disp, d, 0, 0, width, height, AllPlanes, ZPixmap);

	regid(disp, (union regtypes *)&imp, REG_IMAGE);

	return(imp);
}


/*
 * Compare an image to what is currently on the drawable.
 * This is common code for the routines compsavimage() and diffsavimage().
 */
static Status
compsavcommon(disp, d, im, diff)
Display	*disp;
Drawable d;
XImage	*im;
int 	diff;	/* True if difference expected */
{
XImage	*newim;
int 	x, y;
unsigned int 	width, height;
unsigned long	pix1, pix2;

	getsize(disp, d, &width, &height);
	newim =  XGetImage(disp, d, 0, 0, width, height, AllPlanes, ZPixmap);
	if (newim == (XImage*)0) {
		delete("XGetImage failed");
		return(False);
	}

	for (y = 0; y < height; y++) {
		for (x = 0; x < width; x++) {
			pix1 = XGetPixel(im, x, y);
			pix2 = XGetPixel(newim, x, y);

			if (pix1 != pix2) {
				if (!diff) {
					char	name[32];

					/*
					 * A difference was not expected so dump out expected
					 * and obtained images.
					 */
					report("Pixel mismatch at (%d, %d) (%d - %d)"
						, x, y, pix1, pix2);
					sprintf(name, "Err%04d.err", Errnum++);
					report("See file %s for details", name);
					(void) unlink(name);
					dumpimage(newim, name, (struct area *)0);
					dumpimage(im, name, (struct area *)0);
					XDestroyImage(newim);
				}
				return(False);
			}
		}
	}
	XDestroyImage(newim);
	return(True);
}

Status
compsavimage(disp, d, im)
Display	*disp;
Drawable d;
XImage	*im;
{
	return compsavcommon(disp, d, im, 0);
}

Status
diffsavimage(disp, d, im)
Display	*disp;
Drawable d;
XImage	*im;
{
	return compsavcommon(disp, d, im, 1);
}


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
 * $XConsortium: checkimg.c,v 1.7 94/04/17 21:00:38 rws Exp $
 */

#include	"xtest.h"
#include	"Xlib.h"
#include	"Xutil.h"
#include	"xtestlib.h"
#include	"pixval.h"

/*
 * Check that each pixel within the specified area is set to
 * inpix and that each pixel outside the area is set to outpix.
 * If flags are CHECK_IN only the inside is checked.
 * If flags are CHECK_OUT only the outside is checked.
 * flags of 0 or CHECK_ALL check both.
 * If ap is NULL then the whole image is checked.
 */
Status
checkimg(im, ap, inpix, outpix, flags)
XImage *im;
struct area *ap;
long inpix;
long outpix;
int flags;
{
register int x, y;
int xorig;
int yorig;
unsigned int fullwidth;
unsigned int fullheight;
register unsigned long pix;
struct area area;
int inloopflag = 0;

	if (flags == 0)
		flags = CHECK_ALL;
	if ((flags & CHECK_ALL) == 0) {
		report("assert error in checkimg()");
		printf("assert error in checkimg()\n");
		exit(1);
	}

	fullwidth = im->width;
	fullheight = im->height;

	/*
	 * If a NULL ap has been given then check the whole image.
	 */
	if (ap == (struct area *)0) {
		ap = &area;
		ap->x = 0;
		ap->y = 0;
		ap->width = fullwidth;
		ap->height = fullheight;
		flags = CHECK_IN;
	}

	if ((flags & CHECK_ALL) == CHECK_IN) {
		xorig = ap->x;
		yorig = ap->y;
		fullwidth = ap->width;
		fullheight = ap->height;
	} else {
		xorig = 0;
		yorig = 0;
	}

	for (y = 0; y < fullheight; y++) {
		for (x = 0; x < fullwidth; x++) {
			inloopflag = 1;
			pix = XGetPixel(im, x, y);
			if (x+xorig >= ap->x && x+xorig < ap->x+ap->width && y+yorig >= ap->y && y+yorig < ap->y+ap->height) {
				if (pix != inpix && (flags & CHECK_IN)) {
					report("Incorrect pixel on inside of area at point (%d, %d): 0x%x != 0x%x",
						x, y, pix, inpix);
					return(False);
				}
			} else {
				if (pix != outpix && (flags & CHECK_OUT)) {
					report("Incorrect pixel on outside of area at point (%d, %d): 0x%x != 0x%x",
					x, y, pix, outpix);
					return(False);
				}
			}
		}
	}

	/* This is to catch bugs */
	if (inloopflag == 0) {
		delete("No pixels checked in checkimg - internal error");
		return(False);
	}
	return(True);
}

/*
 * checkimgstruct() -	verify width, height, and format fields
 *			of an image structure
 */
Status
checkimgstruct(im, depth, width, height, format)
XImage *im;
unsigned int depth;
unsigned int width;
unsigned int height;
int format;
{
	if (im->depth != depth) {
		report("Incorrect depth (%d != %d)", im->depth, depth);
		return(False);
	}
	if (im->width != width) {
		report("Incorrect width (%d != %d)", im->width, width);
		return(False);
	}
	if (im->height != height) {
		report("Incorrect height (%d != %d)", im->height, height);
		return(False);
	}
	if (im->format != format) {
		report("Incorrect format (%d != %d)", im->format, format);
		return(False);
	}
	return(True);
}

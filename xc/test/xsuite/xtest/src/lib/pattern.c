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
 * $XConsortium: pattern.c,v 1.10 94/04/17 21:01:01 rws Exp $
 */

#include	"xtest.h"
#include	"Xlib.h"
#include	"Xutil.h"
#include	"xtestlib.h"
#include	"pixval.h"

/*
 * Scribble on the drawable.  A series of vertical lines are drawn starting
 * at (0,0) then at (5,0) (10,0) etc.
 */
void
pattern(disp, d)
Display	*disp;
Drawable	d;
{
unsigned int 	width;
unsigned int 	height;
int 	x;
GC  	gc;

	gc = XCreateGC(disp, d, 0, (XGCValues*)0);
	XSetState(disp, gc, W_FG, W_BG, GXcopy, AllPlanes);

	getsize(disp, d, &width, &height);

	for (x = 0; x < width; x += 5)
		XDrawLine(disp, d, gc, x, 0, x, height);
	
	XFreeGC(disp, gc);
}

/*
 * Check that the pattern that is drawn in pattern() is unchanged.
 * This is done by direct pixel validation with GetImage.
 * If ap is non-NULL then validation is restricted to that area
 * with the origin the origin of the area.
 */
Status
checkpattern(disp, d, ap)
Display	*disp;
Drawable	d;
struct	area	*ap;
{
XImage	*imp;
int 	x, y;
unsigned long	pix;
struct	area	area;
	
	if (ap == (struct area *)0) {
		ap = &area;
		ap->x = ap->y = 0;
		getsize(disp, d, &ap->width, &ap->height);
	}

	imp = XGetImage(disp, d, ap->x, ap->y, ap->width, ap->height, AllPlanes, ZPixmap);
	if (imp == (XImage*)0) {
		report("Get Image failed in checkpattern()");
		return(False);
	}

	for (y = 0; y < ap->height; y++) {
		for (x = 0; x < ap->width; x++) {
			pix = XGetPixel(imp, x, y);
			if (pix != ((x%5 == 0)? W_FG: W_BG)) {
				report("Bad pixel in pattern at (%d, %d)", x, y);
				return(False);
			}
		}
	}
	return(True);
}

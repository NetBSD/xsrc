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
 * $XConsortium: makepixm.c,v 1.5 94/04/17 21:00:54 rws Exp $
 */

#include	"xtest.h"
#include	"Xlib.h"
#include	"Xutil.h"
#include	"xtestlib.h"
#include	"tet_api.h"
#include	"pixval.h"

#define	BORDER	1

/*
 * Creates a general purpose pixmap that can be used within the
 * test suite.  The pixmap is cleared to W_BG.
 */
Pixmap
makepixm(disp, vp)
Display	*disp;
XVisualInfo	*vp;
{
GC  	gc;
Pixmap	pm;

	pm = XCreatePixmap(disp, DRW(disp), W_STDWIDTH, W_STDHEIGHT, vp->depth);
	gc = makegc(disp, pm);

	/*
	 * Set the forground to the background pixel so that we can clear the
	 * pixmap using fill rectangle.
	 */
	XSetForeground(disp, gc, W_BG);
	XFillRectangle(disp, pm, gc, 0, 0, W_STDWIDTH+1, W_STDHEIGHT+1);

	/* register pixmap, will be conditional */
	regid(disp, (union regtypes *)&pm, REG_PIXMAP);

	return(pm);
}

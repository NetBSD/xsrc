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
 * $XConsortium: clip-x-or.mc,v 1.11 94/04/17 21:14:37 rws Exp $
 */

>># This file does both the x and y clip-mask origin components.
>>#
>>ASSERTION Good A
The clip origin coordinates
.M clip-x-origin
and
.M clip-y-origin
are interpreted relative to the
origin of the destination drawable specified in the graphics
operation.
>>STRATEGY
Create Pixmap and set clip-mask with it.
Vary clip origin
Verify nothing is drawn outside the clip_mask based on the origin.
Pixmap verify results inside the cliparea.
>>CODE
XVisualInfo	*vp;
Pixmap	cmopixmap;
struct	area	area;
unsigned int 	cmowidth;
unsigned int 	cmoheight;
int 	divsize;
int 	i, j;

	divsize = 3;

	for (resetvinf(VI_WIN_PIX); nextvinf(&vp); ) {
#ifdef A_DRAWABLE2
		winpair(A_DISPLAY, vp, &A_DRAWABLE, &A_DRAWABLE2);
#if T_XCopyPlane
		dset(A_DISPLAY, A_DRAWABLE, ~0L);
#else
		dset(A_DISPLAY, A_DRAWABLE, W_FG);
#endif
#else
		A_DRAW = makewin(A_DISPLAY, vp);
#endif
		A_GC = makegc(A_DISPLAY, A_DRAW);
#ifdef A_IMAGE
		A_IMAGE = makeimg(A_DISPLAY, vp, ZPixmap);
		dsetimg(A_IMAGE, W_FG);
#endif

		/*
		 * Get size of the pixmap.  It is divsize smaller on all sides
		 * than the window.
		 */
		getsize(A_DISPLAY, A_DRAW, &cmowidth, &cmoheight);
		cmowidth /= divsize;
		cmoheight /= divsize;
		debug(2, "clip-mask height=%d, width=%d", cmowidth, cmoheight);

		/*
		 * Create a pixmap that is about divsize^2 of the area of
		 * the window.
		 */
		cmopixmap = XCreatePixmap(A_DISPLAY, A_DRAW, cmowidth, cmoheight, 1);
		dset(A_DISPLAY, cmopixmap, 1L);

		XSetClipMask(A_DISPLAY, A_GC, cmopixmap);
		XFreePixmap(A_DISPLAY, cmopixmap);

		for (i = 0; i < divsize; i++) {
			for (j = 0; j < divsize; j++) {
				setarea(&area, i*cmowidth, j*cmoheight, cmowidth, cmoheight);
				debug(2, "Origin at (%d,%d)", area.x, area.y);
				XSetClipOrigin(A_DISPLAY, A_GC, area.x, area.y);
				XCALL;
				if (checkarea(A_DISPLAY,A_DRAW,&area,W_BG,W_BG,CHECK_OUT))
					CHECK;
				else {
					report("Drawing occurred outside clip_mask");
					FAIL;
				}
				PIXCHECK(A_DISPLAY, A_DRAW);
				dclear(A_DISPLAY, A_DRAW);
			}
		}
	}

	CHECKPASS(2*divsize*divsize*nvinf());


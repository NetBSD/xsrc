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
 * $XConsortium: foregroun.mc,v 1.16 94/04/17 21:14:43 rws Exp $
 */
>>ASSERTION Good A
The foregound component of the gc is used to determine the foreground
pixel value.
>>STRATEGY
Set foreground to various values.
Pixmap verify the results.
>>CODE
static unsigned long fglist[] = {
	0, 0x1, 0x2, 0x3, 0x4, 0x6, 0x7, 0x10, 0x33, 0x81,
	0xa3, 0xff, 256, 300, 1000,
	0x111111, 0x400200, 0x777777,
	};
XVisualInfo	*vp;
long	fgind;
unsigned long	fg;
int 	nchecks = 0;

	for (resetvinf(VI_WIN_PIX); nextvinf(&vp); ) {
#ifdef A_DRAWABLE2
		winpair(A_DISPLAY, vp, &A_DRAWABLE, &A_DRAWABLE2);
#if T_XCopyPlane
		dset(A_DISPLAY, A_DRAWABLE, ~0);
#else
		dset(A_DISPLAY, A_DRAWABLE, W_FG);
#endif
#else
		A_DRAW = makewin(A_DISPLAY, vp);
#endif
		A_GC = makegc(A_DISPLAY, A_DRAW);
#ifdef A_IMAGE
		/* XYBitmap full of 1's will do a foreground fill */
		A_IMAGE = makeimg(A_DISPLAY, vp, XYBitmap);
		dsetimg(A_IMAGE, 1L);
#endif

		for (fgind = 0; fgind < sizeof(fglist)/sizeof(int); fgind++) {

			fg = fglist[fgind];
			if (fg > DEPTHMASK(vp->depth))
				break;

			nchecks++;
			trace("Testing fg pixel of %d", fg);
#if T_XCopyArea
			dset(A_DISPLAY, A_DRAWABLE, fg);
#else
			XSetForeground(A_DISPLAY, A_GC, fg);
#endif

			XCALL;

			PIXCHECK(A_DISPLAY, A_DRAW);

			dclear(A_DISPLAY, A_DRAW);
		}
	}

	CHECKPASS(nchecks);
>>ASSERTION Good A
The value for
.M foreground
is truncated to the depth of the GC.
>>STRATEGY
Set foreground to value with all bits set.
Find a non background pixel.
Check that this is truncated to the depth.
>>CODE
XVisualInfo	*vp;
XImage	*imp;
unsigned long	pix;
unsigned long	fgfg;
int 	found;
int 	i_x, i_y;
unsigned int 	i_width, i_height;

	for (resetvinf(VI_WIN_PIX); nextvinf(&vp); ) {
#ifdef A_DRAWABLE2
		winpair(A_DISPLAY, vp, &A_DRAWABLE, &A_DRAWABLE2);
#if T_XCopyPlane
		dset(A_DISPLAY, A_DRAWABLE, ~0);
#else
		dset(A_DISPLAY, A_DRAWABLE, W_FG);
#endif
#else
		A_DRAW = makewin(A_DISPLAY, vp);
#endif
		A_GC = makegc(A_DISPLAY, A_DRAW);
#ifdef A_IMAGE
		/* XYBitmap full of 1's will do a foreground fill */
		A_IMAGE = makeimg(A_DISPLAY, vp, XYBitmap);
		dsetimg(A_IMAGE, 1L);
#endif

		XSetForeground(A_DISPLAY, A_GC, ~0L);

		XCALL;

		imp = savimage(A_DISPLAY, A_DRAW);
		getsize(A_DISPLAY, A_DRAW, &i_width, &i_height);
		found = 0;
		/*
		 * Look for a non background pixel, if there is one then this
		 * should be the foreground truncated to the depth.
		 */
		for (i_y = 0; i_y < i_height; i_y++) {
			for (i_x = 0; i_x < i_width; i_x++) {
				pix = XGetPixel(imp, i_x, i_y);
				if (pix != W_BG) {
					found = 1;
					if (pix == DEPTHMASK(vp->depth)) {
						CHECK;
					} else {
						report("Pixel was set to %d, expecting %d", pix, DEPTHMASK(vp->depth));
						FAIL;
					}
					i_y = i_height;	/* So that we leave loop XXX */
					break;
				}
			}
		}
		if (found == 0) {
			if (W_BG == (~0 & DEPTHMASK(vp->depth))) {
				report("WARNING: W_BG != 0; this should not be the case");
				CHECK;
			} else {
				report("After setting foreground to (~0)");
				report("no non-background pixel was drawn");
				FAIL;
			}
		}

		dclear(A_DISPLAY, A_DRAW);

		/* Set all bits other than the bottom one */
		fgfg = (~0)^1;
#if T_XCopyArea
		dset(A_DISPLAY, A_DRAWABLE, fgfg);
#else
		XSetForeground(A_DISPLAY, A_GC, fgfg);
#endif

		XCALL;

		imp = savimage(A_DISPLAY, A_DRAW);
		getsize(A_DISPLAY, A_DRAW, &i_width, &i_height);
		found = 0;
		/*
		 * Look for a non background pixel, if there is one then this
		 * should be the foreground truncated to the depth.
		 */
		for (i_y = 0; i_y < i_height; i_y++) {
			for (i_x = 0; i_x < i_width; i_x++) {
				pix = XGetPixel(imp, i_x, i_y);
				if (pix != W_BG) {
					found = 1;
					if (pix == (fgfg & DEPTHMASK(vp->depth))) {
						CHECK;
					} else {
						report("Pixel was set to %d, expecting %d", pix, (fgfg & DEPTHMASK(vp->depth)));
						FAIL;
					}
					i_y = i_height;	/* So that we leave loop XXX */
					break;
				}
			}
		}
		if (found == 0) {
			/*
			 * This should be the case on monochrome displays
			 */
			if (W_BG == (fgfg & DEPTHMASK(vp->depth))) {
				CHECK;
			} else {
				report("After setting foreground to (~0)^1");
				report("no non-background pixel was drawn");
				FAIL;
			}
		}
	}

	CHECKPASS(2*nvinf());

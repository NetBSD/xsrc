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
 * $XConsortium: backgroun.mc,v 1.16 94/04/17 21:14:35 rws Exp $
 */
>>ASSERTION Good A
The background component of the gc is used to determine the background
pixel value.
>>STRATEGY
Set GC component background to various values.
Pixmap verify the results.
>>EXTERN

#if !defined(T_XDrawImageString) && !defined(T_XCopyPlane) && !defined(T_XPutImage) && !defined(T_XCopyArea)
#define NEED_STIPPLE
#endif

#if defined(T_XCopyPlane)
#define CLEAR_TO_ZERO
#endif

>>CODE
static unsigned long bglist[] = {
	0, 0x1, 0x2, 0x3, 0x4, 0x6, 0x7, 0x10, 0x33, 0x81,
	0xa3, 0xff, 256, 300, 1000,
	0x111111, 0x400200, 0x777777,
	};
XVisualInfo	*vp;
#ifdef NEED_STIPPLE
Pixmap	stip;
char	*stipbit = "\0";
#endif
long	bgind;
unsigned long	bg;
int 	nchecks = 0;

#ifdef NEED_STIPPLE
	stip = XCreateBitmapFromData(A_DISPLAY, DRW(A_DISPLAY), stipbit, 1, 1);
#endif

	for (resetvinf(VI_WIN_PIX); nextvinf(&vp); ) {
#ifdef A_DRAWABLE2
		winpair(A_DISPLAY, vp, &A_DRAWABLE, &A_DRAWABLE2);
#ifdef CLEAR_TO_ZERO
		dset(A_DISPLAY, A_DRAWABLE, 0L);
#else
		dset(A_DISPLAY, A_DRAWABLE, W_FG);
#endif
#else
		A_DRAW = makewin(A_DISPLAY, vp);
#endif
		A_GC = makegc(A_DISPLAY, A_DRAW);
#ifdef A_IMAGE
		/* Bitmap full of 0's will cause a bg fill */
		A_IMAGE = makeimg(A_DISPLAY, vp, XYBitmap);
		dsetimg(A_IMAGE, 0L);
#endif

		for (bgind = 0; bgind < NELEM(bglist); bgind++) {

			bg = bglist[bgind];
			if (bg > DEPTHMASK(vp->depth))
				break;

			trace("Testing bg pixel of %d", bg);
			nchecks++;

#ifdef NEED_STIPPLE
			/*
			 * Set a stipple to all zero's and set the fill style
			 * to be FillOpaqueStippled.  This will mean that everything
			 * should be drawn in the background colour.
			 * (different for XCopyPlane, which doesn't use fill style,
			 * nor does XPutImage or XCopyArea).
			 */
			XSetStipple(A_DISPLAY, A_GC, stip);
			XSetFillStyle(A_DISPLAY, A_GC, FillOpaqueStippled);
#endif
/* now set the background to bg and do the call. If it's XCopyPlane/XPutImage then
 * we have cleared the src drawable/image to zero so we will always find 0 in the
 * selected plane so we will fill with bg. If we're XDrawImageString then
 * we do the equivalent of the background fill anyway,  if we're
 * XCopyArea then we have to do a dset of src with bg otherwise we'll have
 * used FillOpaqueStippled and a stipple of a lone zero, only, to get the
 * same effect of a background fill.
 */
#if T_XCopyArea
			dset(A_DISPLAY, A_DRAWABLE, bg);
#else
			XSetBackground(A_DISPLAY, A_GC, bg);
#endif

			XCALL;

			PIXCHECK(A_DISPLAY, A_DRAW);

			dclear(A_DISPLAY, A_DRAW);
		}
	}

#ifdef NEED_STIPPLE
	XFreePixmap(A_DISPLAY, stip);
#endif

	CHECKPASS(nchecks);
>>ASSERTION Good A
The value for
.M background
is truncated to the depth of the GC.
>>STRATEGY
Set GC component background to value with all bits set.
If not (XDrawImageString  || XCopyPlane || XPutImage || XCopyArea)
  Set GC component stipple to all zeros
  Set GC component FillStyle to FillOpaqueStippled.
else if (XCopyPlane || XPutImage)
  Fill source drawable/image-bitmap with 0's to ensure we draw with background
else if XCopyArea
  Fill source drawable/image-bitmap with all 1's
Draw item.
Find a pixel with a value other than W_BG.
Check that this is truncated to the depth.
>>CODE
XVisualInfo	*vp;
XImage	*imp;
#ifdef NEED_STIPPLE
Pixmap	stip;
char	*stipbit = "\0\0\0\0";
#endif
unsigned long	pix;
int 	found;
int 	i_x, i_y;
unsigned int 	i_width, i_height;

#ifdef NEED_STIPPLE
	stip = XCreateBitmapFromData(A_DISPLAY, DRW(A_DISPLAY), stipbit, 1, 1);
#endif

	for (resetvinf(VI_WIN_PIX); nextvinf(&vp); ) {
#ifdef A_DRAWABLE2
		winpair(A_DISPLAY, vp, &A_DRAWABLE, &A_DRAWABLE2);
#ifdef CLEAR_TO_ZERO
		dset(A_DISPLAY, A_DRAWABLE, 0L);
#else
		dset(A_DISPLAY, A_DRAWABLE, W_FG);
#endif
#else
		A_DRAW = makewin(A_DISPLAY, vp);
#endif
		A_GC = makegc(A_DISPLAY, A_DRAW);
#ifdef A_IMAGE
		/* bitmap of all 0's will cause bg "fill" */
		A_IMAGE = makeimg(A_DISPLAY, vp, XYBitmap);
		dsetimg(A_IMAGE, 0L);
#endif

#ifdef NEED_STIPPLE
		/*
		 * Set a stipple to all zero's and set the fill style
		 * to be FillOpaqueStippled.  This will mean that everything
		 * should be drawn in the background colour.
		 */
		XSetStipple(A_DISPLAY, A_GC, stip);
		XSetFillStyle(A_DISPLAY, A_GC, FillOpaqueStippled);
#endif

/* now set the background to ~0 and do the call. If it's XCopyPlane/XPutImage then
 * we have cleared the src drawable/image to zero so we will always find 0 in the
 * selected plane so we will fill with ~0. If we're XDrawImageString then
 * we do the equivalent of the background fill anyway, if we're XCopyArea
 * then we need to fill the src will all 1's, otherwise we'll have
 * used FillOpaqueStippled and a stipple of a lone zero, only, to get the
 * same effect of a background fill.
 */
#if T_XCopyArea
		dset(A_DISPLAY, A_DRAWABLE, ~0L);
#else
		XSetBackground(A_DISPLAY, A_GC, ~0L);
#endif

		XCALL;

		imp = savimage(A_DISPLAY, A_DRAW);
		getsize(A_DISPLAY, A_DRAW, &i_width, &i_height);
		found = 0;
		for (i_y = 0; i_y < i_height; i_y++) {
			for (i_x = 0; i_x < i_width; i_x++) {
				pix = XGetPixel(imp, i_x, i_y);
				if (pix != W_BG) {
					found = 1;
					if (pix == DEPTHMASK(vp->depth)) {
						CHECK;
					} else {
						report("Pixel was set to %d, expecting %d", pix, DEPTHMASK(vp->depth));
#ifdef NEED_STIPPLE
						report("FillOpaqueStippled is also used in this test, and could affect the result");
#endif
						FAIL;
					}
					i_y = i_height;	/* So that we leave loop XXX */
					break;
				}
			}
		}
		if (found == 0) {
			report("Nothing was drawn");
#ifdef NEED_STIPPLE
			report("FillOpaqueStippled is also used in this test, and could affect the result");
#endif
			FAIL;
		}
	}
#ifdef NEED_STIPPLE
	XFreePixmap(A_DISPLAY, stip);
#endif

	CHECKPASS(nvinf());


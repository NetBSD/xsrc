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
 * $XConsortium: fill-styl.mc,v 1.9 94/04/17 21:14:39 rws Exp $
 */
>>EXTERN

#ifndef tile_width
#define tile_width 19
#define tile_height 19
static unsigned char tile_bits[] = {
   0xff, 0xff, 0x07, 0x80, 0x0f, 0x00, 0x67, 0x30, 0x07, 0x9b, 0xcf, 0x06,
   0xe8, 0xbf, 0x00, 0x94, 0x4f, 0x01, 0x54, 0x50, 0x01, 0x2a, 0xa7, 0x02,
   0xaa, 0xaf, 0x02, 0xaf, 0xaf, 0x07, 0xaa, 0xaf, 0x02, 0x2a, 0xa7, 0x02,
   0x54, 0x50, 0x01, 0x94, 0x4f, 0x01, 0xe8, 0xbf, 0x00, 0x98, 0xcf, 0x00,
   0x63, 0x30, 0x06, 0x80, 0x0f, 0x00, 0xff, 0xf8, 0x07};
#endif
#ifndef stipple_width
#define stipple_width 24
#define stipple_height 11
static unsigned char stipple_bits[] = {
   0xff, 0x0f, 0x00, 0x3f, 0xf0, 0xff, 0xcf, 0xff, 0x03, 0xf7, 0x0f, 0xfc,
   0xff, 0xf1, 0x0f, 0x78, 0x7e, 0xf0, 0x80, 0x8f, 0x1f, 0x2a, 0xf0, 0xe1,
   0x80, 0x07, 0x1e, 0xaa, 0xff, 0xe0, 0x80, 0xff, 0x0f};
#endif
>>ASSERTION Good A
When
.M fill_style
is
.S FillSolid ,
then on a call to xname the source pixel for the drawing operation is
.M foreground .
>>STRATEGY
Set fill-style to FillSolid.
Do drawing operation.
Pixmap verify.
>>CODE
XVisualInfo	*vp;

	for (resetvinf(VI_WIN_PIX); nextvinf(&vp); ) {
		A_DRAWABLE = makewin(A_DISPLAY, vp);
		A_GC = makegc(A_DISPLAY, A_DRAWABLE);

		XSetFillStyle(A_DISPLAY, A_GC, FillSolid);
		XCALL;

		PIXCHECK(A_DISPLAY, A_DRAWABLE);
	}

	CHECKPASS(nvinf());
>>ASSERTION Good A
When
.M fill-style
is
.S FillTiled ,
then on a call to xname the source for the drawing operation is
.M tile .
>>STRATEGY
Create a tile of appropriate depth.
If depth is greater than one, set the fg and bg in the tile
to interesting values.
Set fill-style to FillTiled.
Part 1.
Do drawing operation.
If FillRectangle operation
  Directly check using checktile.
else
  Pixmap verify.

Part 2.
Do drawing operation.
Reverse fg and bg in tile.
Set gc function to xor.
Repeat drawing operation.
Verify that result is the same as drawing solid line with xor
of the fg and bg.
(This also verifies for depth 1 screens that the background is being
drawn)

>>CODE
XVisualInfo	*vp;
XImage	*fssav;
Pixmap	fstile, fstile2;
unsigned long	fsfg, fsbg;

	for (resetvinf(VI_WIN_PIX); nextvinf(&vp); ) {
		A_DRAWABLE = makewin(A_DISPLAY, vp);
		A_GC = makegc(A_DISPLAY, A_DRAWABLE);

		setwidth(A_DISPLAY, A_GC, (unsigned)8);

		/* 1 */
		fsfg = W_FG;
		fsbg = W_BG;
		if (vp->depth > 1) {
			fsfg ^= 0x2;
			fsbg ^= 0x2;
		}
		fstile = XCreatePixmapFromBitmapData(A_DISPLAY, A_DRAWABLE
			, (char*)tile_bits, tile_width, tile_height
			, fsfg, fsbg
			, vp->depth
			);
		XSetTile(A_DISPLAY, A_GC, fstile);

		XSetFillStyle(A_DISPLAY, A_GC, FillTiled);

		XCALL;

#if defined(T_XFillRectangle)
		{
		struct	area area;

			setarea(&area, x, y, width, height);
			if (checktile(A_DISPLAY, A_DRAWABLE, &area, 0, 0, fstile))
				CHECK;
			else {
				report("Direct test of tiled area failed");
				FAIL;
			}
		}
#else
		PIXCHECK(A_DISPLAY, A_DRAWABLE);
#endif

		/* 2 */
		dclear(A_DISPLAY, A_DRAWABLE);

		XSetFillStyle(A_DISPLAY, A_GC, FillSolid);
		XSetForeground(A_DISPLAY, A_GC, fsfg^fsbg);
		XCALL;
		fssav = savimage(A_DISPLAY, A_DRAWABLE);

		/*
		 * Draw with foreground and background as before.
		 */
		dclear(A_DISPLAY, A_DRAWABLE);
		XSetFillStyle(A_DISPLAY, A_GC, FillTiled);
		XSetTile(A_DISPLAY, A_GC, fstile);
		XCALL;

		/*
		 * Draw with background and foreground reversed.
		 * xor it in.
		 */
		fstile2 = XCreatePixmapFromBitmapData(A_DISPLAY, A_DRAWABLE
			, (char*)tile_bits, tile_width, tile_height
			, fsbg, fsfg
			, vp->depth
			);
		XSetFillStyle(A_DISPLAY, A_GC, FillTiled);
		XSetTile(A_DISPLAY, A_GC, fstile2);
		XSetFunction(A_DISPLAY, A_GC, GXxor);
		XCALL;

		if (compsavimage(A_DISPLAY, A_DRAWABLE, fssav))
			CHECK;
		else {
			report("Reversing tile fg and bg did not draw solid line");
			FAIL;
		}
		XFreePixmap(A_DISPLAY, fstile);
		XFreePixmap(A_DISPLAY, fstile2);
	}

	CHECKPASS(2*nvinf());

>>ASSERTION Good A
When
.M fill_style
is
.S FillOpaqueStippled ,
then on a call to xname the source for the drawing operation is
a tile with the same width and height as
.M stipple ,
but with
.M background
everywhere
.M stipple
has a zero and with
.M foreground
everywhere
.M stipple
has a one.
>>STRATEGY
Create stipple.
Set fill-style to FillOpaqueStippled.
Do drawing operation.
Pixmap verify the results.

Reverse foreground and background pixels.
Combine drawing with previous drawing.
Verify that the results are the same as FillSolid with foreground
 equal to combination of fg and bg pixels.
>>CODE
XVisualInfo	*vp;
XImage	*fssav;
Pixmap	fsstip;
unsigned long	fsfg, fsbg;

	for (resetvinf(VI_WIN_PIX); nextvinf(&vp); ) {
		A_DRAWABLE = makewin(A_DISPLAY, vp);
		A_GC = makegc(A_DISPLAY, A_DRAWABLE);

		setwidth(A_DISPLAY, A_GC, (unsigned)8);

		/* 1 */
		fsfg = W_FG;
		fsbg = W_BG;
		if (vp->depth > 1) {
			fsfg ^= 0x2;
			fsbg ^= 0x2;
			XSetForeground(A_DISPLAY, A_GC, fsfg);
			XSetBackground(A_DISPLAY, A_GC, fsbg);
		}
		fsstip = XCreateBitmapFromData(A_DISPLAY, A_DRAWABLE
			, (char*)stipple_bits, stipple_width, stipple_height
			);
		XSetStipple(A_DISPLAY, A_GC, fsstip);

		XSetFillStyle(A_DISPLAY, A_GC, FillOpaqueStippled);

		XCALL;

		PIXCHECK(A_DISPLAY, A_DRAWABLE);

		/* 2 */
		dclear(A_DISPLAY, A_DRAWABLE);

		XSetFillStyle(A_DISPLAY, A_GC, FillSolid);
		XSetForeground(A_DISPLAY, A_GC, fsfg^fsbg);
		XCALL;
		fssav = savimage(A_DISPLAY, A_DRAWABLE);
		XSetForeground(A_DISPLAY, A_GC, fsfg);

		/*
		 * Draw with foreground and background as before.
		 */
		dclear(A_DISPLAY, A_DRAWABLE);
		XSetFillStyle(A_DISPLAY, A_GC, FillOpaqueStippled);
		XSetStipple(A_DISPLAY, A_GC, fsstip);
		XCALL;

		/*
		 * Draw with background and foreground reversed.
		 * xor it in.
		 */
		XSetForeground(A_DISPLAY, A_GC, fsbg);
		XSetBackground(A_DISPLAY, A_GC, fsfg);
		XSetFillStyle(A_DISPLAY, A_GC, FillOpaqueStippled);
		XSetStipple(A_DISPLAY, A_GC, fsstip);
		XSetFunction(A_DISPLAY, A_GC, GXxor);
		XCALL;

		if (compsavimage(A_DISPLAY, A_DRAWABLE, fssav))
			CHECK;
		else {
			report("Reversing fg and bg did not draw solid line");
			FAIL;
		}
		XFreePixmap(A_DISPLAY, fsstip);
	}

	CHECKPASS(2*nvinf());

>>ASSERTION Good A
When
.M fill_style
is
.S FillStippled ,
then on a call to xname the source for the drawing operation is
.M foreground
masked by
the stipple pattern tiled in a single plane.
>>STRATEGY
Create stipple
Set fill-style to FillStippled.
Set background to foreground pixel to verify it is not affecting things.
Do drawing. 
Pixmap verify.
>>CODE
XVisualInfo	*vp;
Pixmap	fsstip;

	for (resetvinf(VI_WIN_PIX); nextvinf(&vp); ) {
		A_DRAWABLE = makewin(A_DISPLAY, vp);
		A_GC = makegc(A_DISPLAY, A_DRAWABLE);

		setwidth(A_DISPLAY, A_GC, (unsigned)8);

		/* 1 */
		fsstip = XCreateBitmapFromData(A_DISPLAY, A_DRAWABLE
			, (char*)stipple_bits, stipple_width, stipple_height
			);
		XSetStipple(A_DISPLAY, A_GC, fsstip);
		XSetFillStyle(A_DISPLAY, A_GC, FillStippled);
		/* To show it has no effect */
		XSetBackground(A_DISPLAY, A_GC, W_FG);

		XCALL;

		PIXCHECK(A_DISPLAY, A_DRAWABLE);

		XFreePixmap(A_DISPLAY, fsstip);

	}

	CHECKPASS(nvinf());


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
 * $XConsortium: ts-x-orig.mc,v 1.6 94/04/17 21:14:49 rws Exp $
 */
/*
 * The symbols stipple_* and tile_* that do not occur in this file
 * are defined in fill-style.mc which is always included along with
 * this one.
 */
>>ASSERTION Good A
The tile/stipple origin coordinates
.M ts-x-origin
and
.M ts-y-origin
are interpreted relative to the
origin of the destination drawable specified in the graphics
operation.
>>STRATEGY
Create a stipple pixmap.
Set stipple component of GC to pixmap using XSetStipple.
For various tile/stipple origins:
	Set tile/stipple origin using XSetTSOrigin.
	Set fill-style to FillStippled using XSetFillStyle.
	Clear drawable.
	Do graphics operation.
	Pixmap verify.
Create a tile with depth of drawable.
Set tile component of GC to tile using XSetTile.
For various tile/stipple origins:
	Set tile/stipple origin using XSetTSOrigin.
	Set fill-style to FillTiled using XSetFillStyle.
	Clear drawable.
	Do graphics operation.
	Pixmap verify.
>>CODE
XVisualInfo	*vp;
Pixmap	tsostip;
Pixmap	tsotile;

	for (resetvinf(VI_WIN_PIX); nextvinf(&vp); ) {
		A_DRAWABLE = makewin(A_DISPLAY, vp);
		A_GC = makegc(A_DISPLAY, A_DRAWABLE);

		tsostip = XCreateBitmapFromData(A_DISPLAY, A_DRAWABLE
			, (char*)stipple_bits, stipple_width, stipple_height
			);
		XSetStipple(A_DISPLAY, A_GC, tsostip);
		XFreePixmap(A_DISPLAY, tsostip);
		XSetFillStyle(A_DISPLAY, A_GC, FillStippled);

		XSetTSOrigin(A_DISPLAY, A_GC, 9, 17);
		dclear(A_DISPLAY, A_DRAWABLE);
		XCALL;
		PIXCHECK(A_DISPLAY, A_DRAWABLE);

		XSetTSOrigin(A_DISPLAY, A_GC, 0xd8f2, 0x4321);
		dclear(A_DISPLAY, A_DRAWABLE);
		XCALL;
		PIXCHECK(A_DISPLAY, A_DRAWABLE);

		XSetTSOrigin(A_DISPLAY, A_GC, -3, 7);
		dclear(A_DISPLAY, A_DRAWABLE);
		XCALL;
		PIXCHECK(A_DISPLAY, A_DRAWABLE);

		/* Now Tiles */
		tsotile = XCreatePixmapFromBitmapData(A_DISPLAY, A_DRAWABLE
			, (char*)tile_bits, tile_width, tile_height
			, W_FG, W_BG
			, vp->depth
			);
		XSetTile(A_DISPLAY, A_GC, tsotile);
		XFreePixmap(A_DISPLAY, tsotile);
		XSetFillStyle(A_DISPLAY, A_GC, FillTiled);

		XSetTSOrigin(A_DISPLAY, A_GC, 9, 17);
		dclear(A_DISPLAY, A_DRAWABLE);
		XCALL;
		PIXCHECK(A_DISPLAY, A_DRAWABLE);

		XSetTSOrigin(A_DISPLAY, A_GC, 0xd8f2, 0x4321);
		dclear(A_DISPLAY, A_DRAWABLE);
		XCALL;
		PIXCHECK(A_DISPLAY, A_DRAWABLE);

		XSetTSOrigin(A_DISPLAY, A_GC, -3, 7);
		dclear(A_DISPLAY, A_DRAWABLE);
		XCALL;
		PIXCHECK(A_DISPLAY, A_DRAWABLE);
	}

	CHECKPASS(6*nvinf());

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
 * $XConsortium: clip-mask.mc,v 1.12 94/04/17 21:14:36 rws Exp $
 */
>>ASSERTION Good A
When the
.M clip_mask
is set to
.S None ,
then the pixels are always drawn regardless of the clip origin.
>>STRATEGY
Set clip mask to None
Verify that things are still drawn.
>>CODE
XVisualInfo	*vp;
XImage	*cmsav;

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

		XCALL;
		cmsav = savimage(A_DISPLAY, A_DRAW);
		dclear(A_DISPLAY, A_DRAW);

		/*
		 * Not very interesting, because this should be the default
		 * anyway.
		 */
		XSetClipMask(A_DISPLAY, A_GC, None);

		XCALL;

		if (compsavimage(A_DISPLAY, A_DRAW, cmsav))
			CHECK;
		else {
			report("Clip mask of None changed graphics");
			FAIL;
		}
	}

	CHECKPASS(nvinf());

>>ASSERTION Good A
When pixels are outside the area covered by the
.M clip_mask ,
then they are not drawn.
>>STRATEGY
Create a pixmap depth 1.
Fill it with all ones.
Set this pixmap as the clip_mask.
Verify that nothing is drawn outside the clip-mask area.
Use a pixmap size of 5x5 to ensure that all tests have some part of the
drawing outside the clip-mask.
>>CODE
XVisualInfo	*vp;
Pixmap	cmpm;
struct	area	area;

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

		setarea(&area, 0, 0, 5, 5);
		cmpm = XCreatePixmap(A_DISPLAY, A_DRAW, area.width, area.height, 1);
		dset(A_DISPLAY, cmpm, 1L);
		XSetClipMask(A_DISPLAY, A_GC, cmpm);
		XFreePixmap(A_DISPLAY, cmpm);

		XCALL;

		if (checkarea(A_DISPLAY, A_DRAW, &area, 0L, W_BG, CHECK_OUT))
			CHECK;
		else {
			report("Drawing outside clip mask");
			FAIL;
		}
	}

	CHECKPASS(nvinf());
>>ASSERTION Good A
When pixels have a
.M clip_mask
bit set to 1, and they would be drawn,
then they are drawn.
>>STRATEGY
Do graphics operation.
Save the image on the drawable.
Clear drawable.
Create a pixmap depth 1 that is the same size as the window.
Set all bits in pixmap to 1.
Set GC component clip-mask to pixmap using XSetClipMask.
Do graphics operation.
Verify that the images drawn were the same.

Set pixmap to a pattern.
Do graphics operation.
Pixmap verify the result.
>>CODE
#define stipple_width 24
#define stipple_height 11
static unsigned char stipple_bits[] = {
   0xff, 0x0f, 0x00, 0x3f, 0xf0, 0xff, 0xcf, 0xff, 0x03, 0xf7, 0x0f, 0xfc,
   0xff, 0xf1, 0x0f, 0x78, 0x7e, 0xf0, 0x80, 0x8f, 0x1f, 0x2a, 0xf0, 0xe1,
   0x80, 0x07, 0x1e, 0xaa, 0xff, 0xe0, 0x80, 0xff, 0x0f};
XVisualInfo	*vp;
Pixmap	cmpixmap;
Pixmap	cmstip;
XImage	*cmsav;
GC		cmgc;
unsigned int 	cmwidth, cmheight;
int 	i, j;

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

		XCALL;
		cmsav = savimage(A_DISPLAY, A_DRAW);
		if (isdeleted())
			return;
		/* Must clear the drawable - otherwise there is no way 
		of proving clip mask of all 1's allowed drawing */
		dclear(A_DISPLAY, A_DRAW);

		getsize(A_DISPLAY, A_DRAW, &cmwidth, &cmheight);
		cmpixmap = XCreatePixmap(A_DISPLAY, A_DRAW, cmwidth, cmheight, 1);
		if (isdeleted())
			return;
		dset(A_DISPLAY, cmpixmap, 1L);

		XSetClipMask(A_DISPLAY, A_GC, cmpixmap);

		trace("Clip mask of all 1's");
		XCALL;
		if (compsavimage(A_DISPLAY, A_DRAW, cmsav))
			CHECK;
		else {
			report("fail on clip_mask of all 1's");
			FAIL;
		}
		dclear(A_DISPLAY, A_DRAW);

		cmstip = XCreateBitmapFromData(A_DISPLAY, A_DRAW, (char*)stipple_bits,
			stipple_width, stipple_height);
		cmgc = makegc(A_DISPLAY, cmstip);
		if (isdeleted())
			return;

		for (i = 0; i < cmwidth; i += stipple_width) {
			for (j = 0; j < cmheight; j += stipple_height) {
				XCopyArea(A_DISPLAY, cmstip, cmpixmap, cmgc, 0, 0,
					stipple_width, stipple_height, i, j);
			}
		}
		XSetClipMask(A_DISPLAY, A_GC, cmpixmap);

		trace("Clip mask with pattern");
		XCALL;

		PIXCHECK(A_DISPLAY, A_DRAW);

		XFreePixmap(A_DISPLAY, cmpixmap);
		XFreePixmap(A_DISPLAY, cmstip);
	}

	CHECKPASS(2*nvinf());
>>ASSERTION Good A
When pixels have a
.M clip_mask
bit set to 0,
then they are not drawn.
>>STRATEGY
Create a pixmap depth 1 that is the same size as the window.
Set all bits in pixmap to 0.
Set GC component clip_mask to pixmap using XSetClipMask.
Do graphics operation.
Verify that nothing is drawn.
>>CODE
XVisualInfo	*vp;
Pixmap	cmpixmap;
unsigned int 	cmwidth, cmheight;

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

		getsize(A_DISPLAY, A_DRAW, &cmwidth, &cmheight);
		cmpixmap = XCreatePixmap(A_DISPLAY, A_DRAW, cmwidth, cmheight, 1);
		if (isdeleted())
			return;
		dset(A_DISPLAY, cmpixmap, 0L);

		XSetClipMask(A_DISPLAY, A_GC, cmpixmap);

		trace("Clip mask of all 0's");
		XCALL;
		if (checkclear(A_DISPLAY, A_DRAW))
			CHECK;
		else {
			report("Pixels were drawn with clip_mask of all zeros");
			FAIL;
		}

		XFreePixmap(A_DISPLAY, cmpixmap);
	}

	CHECKPASS(nvinf());

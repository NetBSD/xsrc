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
 * $XConsortium: line-widt.mc,v 1.13 94/04/17 21:14:46 rws Exp $
 */
>># >>ASSERTION Good A
>># 	suppressed during drafting stage
>># When unmodified by the
>># .M join-style
>># or
>># .M cap-style ,
>># then the bounding box of a wide line with endpoints [x1, y1], [x2, y2] and
>># width w is a rectangle with vertices at the real coordinates
>># 	[x1-(w*sn/2), y1+(w*cs/2)], [x1+(w*sn/2), y1-(w*cs/2)],
>># 	[x2-(w*sn/2), y2+(w*cs/2)], [x2+(w*sn/2), y2-(w*cs/2)]
>># where sn is the sine of the angle of the line
>># and cs is the cosine of the angle of the line.
>>ASSERTION Good A
When 
.M line_width 
is greater than or equal to one, and 
the center of a pixel is fully inside the boundary,
then the pixel is drawn.
>>STRATEGY
Draw a variety of lines with various widths.
Pixmap verify.
>>CODE
XVisualInfo	*vp;
static	struct	linedata {
	unsigned int 	width;
	int 	x1;
	int 	y1;
	int 	x2;
	int 	y2;
} linedata[] = {
	{1, 15, 10, 50, 30},
	{5, 50, 4, 43, 21},
	{8, 70, 6, 55, 27},
	{2, 13, 13, 15, 24},
	{15, 10, 51, 49, 60},
	};
struct	linedata	*lp;

#define NLINEDATA (sizeof(linedata)/sizeof(linedata[0]))

	for (resetvinf(VI_WIN_PIX); nextvinf(&vp); ) {
		A_DRAWABLE = makewin(A_DISPLAY, vp);
		A_GC = makegc(A_DISPLAY, A_DRAWABLE);

		for (lp = linedata; lp < &linedata[NLINEDATA]; lp++) {

			setwidth(A_DISPLAY, A_GC, lp->width);
			drawline(lp->x1, lp->y1, lp->x2, lp->y2);
		}
		PIXCHECK(A_DISPLAY, A_DRAWABLE);
	}

	CHECKPASS(nvinf());
>>ASSERTION Good A
When 
.M line_width 
is greater than or equal to one, and 
the center of the pixel is exactly on the boundary,
and the boundary is not horizontal, and
the interior is immediately to its right (x increasing direction),
then the pixel is drawn.
>>STRATEGY
Draw sloping line.
Verify that a pixel on the boundary is set.
(More complicated cases are covered by pixel verification elsewhere.)
>>CODE
#if T_XDrawRectangle || T_XDrawRectangles || T_XDrawArc || T_XDrawArcs
>># Next line should be left blank

	report("This test purpose does not apply to %s", TestName);
	report("as %s cannot generate sloping lines", TestName);
	tet_result(TET_NOTINUSE);
#else
XVisualInfo	*vp;

	for (resetvinf(VI_WIN_PIX); nextvinf(&vp); ) {
		A_DRAWABLE = makewin(A_DISPLAY, vp);
		A_GC = makegc(A_DISPLAY, A_DRAWABLE);

		setwidth(A_DISPLAY, A_GC, (unsigned)10);
		drawline(6, 13, 14, 7);

		/*
		 * Pixel at 13,14 should be unset whereas the one at 7,6 should
		 * be set.
		 */
		if (checkpixel(A_DISPLAY, A_DRAWABLE, 13, 14, W_BG))
			CHECK;
		else {
			report("pixel at 13,14 was set");
			FAIL;
		}
		if (checkpixel(A_DISPLAY, A_DRAWABLE, 7, 6, W_FG))
			CHECK;
		else {
			report("pixel at 7,6 was not set");
			FAIL;
		}
	}

	CHECKPASS(2*nvinf());
#endif
>>ASSERTION Good A
When 
.M line_width 
is greater than or equal to one, and 
the center of the pixel is exactly on the boundary,
and the boundary is horizontal, and
the interior or the boundary is immediately below 
(y increasing direction), and the interior or the boundary is immediately
to the right (x increasing direction),
then the pixel is drawn.
>>STRATEGY
Draw horizontal line.
Verify that pixels on boundary with interior below are set.
Verify that pixels on boundary with interior above are not set.
Verify that pixels on boundary with interior to the right are set.
Verify that pixels on boundary with interior to the left are not set.
>>CODE
XVisualInfo	*vp;
XImage	*lwimp;
unsigned int 	lwwidth = 8;
int 	begx = 10, begy = 10, len = 60;
int 	i;

	for (resetvinf(VI_WIN_PIX); nextvinf(&vp); ) {
		A_DRAWABLE = makewin(A_DISPLAY, vp);
		A_GC = makegc(A_DISPLAY, A_DRAWABLE);

		setwidth(A_DISPLAY, A_GC, lwwidth);

		drawline(begx, begy, begx+len, begy);

		lwimp = savimage(A_DISPLAY, A_DRAWABLE);
		for (i = 0; i < len; i++) {
			if (XGetPixel(lwimp, begx+i, begy-lwwidth/2) == W_FG)
				CHECK;
			else {
				report("Pixel with interior below was not drawn");
				FAIL;
			}
		}
		for (i = 0; i < len; i++) {
			if (XGetPixel(lwimp, begx+i, begy+lwwidth/2) == W_BG)
				CHECK;
			else {
				report("Pixel with interior above was drawn");
				FAIL;
			}
		}

		for (i = 0; i < lwwidth; i++) {
			if (XGetPixel(lwimp, 10, begy-lwwidth/2+i) == W_FG)
				CHECK;
			else {
				report("Pixel with interior to the right was not drawn");
				FAIL;
			}
		}
		for (i = 0; i < lwwidth; i++) {
			if (XGetPixel(lwimp, 70, begy-lwwidth/2+i) == W_BG)
				CHECK;
			else {
				report("Pixel with interior to the left was drawn");
				FAIL;
			}
		}

	}

	CHECKPASS(nvinf()*(2*len+2*lwwidth));
>>ASSERTION Good B 2
When 
.M line_width
is zero, then a one pixel wide line is drawn using an
unspecified, device-dependent algorithm.
>>STRATEGY
Draw horizontal zero width line.
Issue warning message if line is not one pixel in width.
Report assertion UNTESTED.
>>CODE
XVisualInfo	*vp;
struct	area	area;

	for (resetvinf(VI_WIN_PIX); nextvinf(&vp); ) {
		A_DRAWABLE = makewin(A_DISPLAY, vp);
		A_GC = makegc(A_DISPLAY, A_DRAWABLE);

		setwidth(A_DISPLAY, A_GC, (unsigned)0);
		drawline(10, 10, 70, 10);

		setarea(&area, 10, 10, 61, 1);
		if (!checkarea(A_DISPLAY, A_DRAWABLE,
				&area, W_FG, W_BG, CHECK_ALL)) {
			/*
			 * The spec doesn't even require that horizontal lines be drawn
			 * properly.  So we can only warn about suprises.
			 */
			trace("Horizontal thin line was not drawn one pixel in width as expected");
		}
	}

	tet_result(TET_UNTESTED);
>>ASSERTION Good A
When 
.M line_width
is zero,
and a line is drawn unclipped from [x1, y1] to [x2, y2] and
another line is drawn unclipped from [x1+dx, y1+dy] to [x2+dx, y2+dy], 
and a point [x, y] is touched by drawing the first line,
then point [x+dx, y+dy] is touched by drawing the second line.
>>STRATEGY
Draw thin line.
Save image with origin based on line position.
Draw thin line displaced by fixed amount.
Save image with origin in same position relative to line.
Compare images.
>>CODE
XVisualInfo	*vp;
XImage	*lwim1, *lwim2;
int 	x1, x2, y1, y2;
int 	lwx, lwy;
unsigned int 	lwwidth = 70;
unsigned int 	lwheight = 50;

	for (resetvinf(VI_WIN_PIX); nextvinf(&vp); ) {
		A_DRAWABLE = makewin(A_DISPLAY, vp);
		A_GC = makegc(A_DISPLAY, A_DRAWABLE);

		x1 = 10; y1 = 10;
		x2 = 63; y2 = 43;

		setwidth(A_DISPLAY, A_GC, (unsigned)0);

		drawline(x1, y1, x2, y2);
		lwim1 = XGetImage(A_DISPLAY, A_DRAWABLE, x1, y1, lwwidth, lwheight, AllPlanes, ZPixmap);

		if (isdeleted())
			return;

		x1 += 12;
		x2 += 12;

		y1 += 17;
		y2 += 17;

		dclear(A_DISPLAY, A_DRAWABLE);
		drawline(x1, y1, x2, y2);
		lwim2 = XGetImage(A_DISPLAY, A_DRAWABLE, x1, y1, lwwidth, lwheight, AllPlanes, ZPixmap);

		if (isdeleted())
			return;

		for (lwy = 0; lwy < lwheight; lwy++) {
			for (lwx = 0; lwx < lwwidth; lwx++) {
				if (XGetPixel(lwim1, lwx, lwy) != XGetPixel(lwim2, lwx, lwy)) {
					report("Thin line differed after displacement");
					report("Point is %d,%d", lwx, lwy);
					FAIL;

					lwy = lwheight;
					break;
				}
			}
		}
		CHECK;
	}

	CHECKPASS(nvinf());

>>ASSERTION def
When 
.M line_width
is zero,
and a line is drawn unclipped from [x1, y1] to [x2, y2] and
another line is drawn unclipped from [x1+dx, y1+dy] to [x2+dx, y2+dy], 
and a point [x, y] is not touched by drawing the first line,
then point [x+dx, y+dy] is not touched by drawing the second line.
>>ASSERTION def
>># This is done in clip-mask
When a line is clipped, and the point is inside the clipping region, and
the point would be touched by the unclipped line, then the pixel is drawn.

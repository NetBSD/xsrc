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
 * $XConsortium: line-styl.mc,v 1.15 94/04/17 21:14:46 rws Exp $
 */
>>EXTERN
static	int 	linestyles[] = {LineSolid, LineOnOffDash, LineDoubleDash};
static	int 	capstyles[] = {CapNotLast, CapButt, CapProjecting, CapRound};

static char	lsdashes[] = {
	14,14};

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
#ifdef stipple_width
#define stipple_width 24
#define stipple_height 11
static unsigned char stipple_bits[] = {
   0xff, 0x0f, 0x00, 0x3f, 0xf0, 0xff, 0xcf, 0xff, 0x03, 0xf7, 0x0f, 0xfc,
   0xff, 0xf1, 0x0f, 0x78, 0x7e, 0xf0, 0x80, 0x8f, 0x1f, 0x2a, 0xf0, 0xe1,
   0x80, 0x07, 0x1e, 0xaa, 0xff, 0xe0, 0x80, 0xff, 0x0f};
#endif
>>#ASSERTION
>># 		suppressed during drafting of assertions
>>#It is recommended that this property be true for thin lines, 
>>#but this is not required.
>>#ASSERTION
>># 		suppressed during drafting of assertions
>>#A line-width of zero may differ from a line-width of one in terms of
>>#which pixels are drawn.
>>ASSERTION def
When the
.M line_style
is
.S LineSolid ,
then the full path of the line is drawn.
>>ASSERTION Good A
When the
.M line_style
is
.S LineOnOffDash ,
then
.M cap_style
applies to 
all internal ends of the individual dashes,
except 
.S CapNotLast
is treated as 
.S CapButt . 
>>STRATEGY
Set graphics coordinates for dashed lines 
	(includes horizontal and vertical cases,
	and includes joins and caps where relevant).
Set the line_style of the GC to LineOnOffDash using XChangeGC.
Set the dash_list of the GC to using XSetDashes.
For cap_style CapNotLast, CapButt, CapProjecting, CapRound:
	Set the cap_style of the GC using XChangeGC.
	Draw paths.
	Pixmap verify.
	Clear drawable.
>>CODE
XVisualInfo	*vp;
int 	i;
int 	caps;
unsigned int	lw;

	for (resetvinf(VI_WIN_PIX); nextvinf(&vp); ) {
		A_DRAWABLE = makewin(A_DISPLAY, vp);
		A_GC = makegc(A_DISPLAY, A_DRAWABLE);

		setfordash();
		for(lw=4; lw <= 5; lw++) { /* odd and even line widths */
			setwidth(A_DISPLAY, A_GC, lw);
			XSetDashes(A_DISPLAY, A_GC, 0, lsdashes, 2);
			setlinestyle(A_DISPLAY, A_GC, LineOnOffDash);

			for (i = 0; i < NELEM(capstyles); i++) {

				caps = capstyles[i];
				trace("LineOnOffDash with %s width %u", capstylename(caps), lw);
				setcapstyle(A_DISPLAY, A_GC, caps);
				XCALL;
				PIXCHECK(A_DISPLAY, A_DRAWABLE);
				dclear(A_DISPLAY, A_DRAWABLE);
			}
			dclear(A_DISPLAY, A_DRAWABLE);
		}
	}

	CHECKPASS(2*NELEM(capstyles)*nvinf());
>>ASSERTION def
When
.M line_style 
is
.S LineSolid 
and
.M fill_style
is
.S FillSolid ,
then on a call to xname the source pixel for the drawing operation is
.M foreground .
>>ASSERTION def
When
.M line_style 
is
.S LineSolid 
and
.M fill_style
is
.S FillTiled ,
then on a call to xname the source for the drawing operation is
.M tile .
>>ASSERTION def
When
.M line_style 
is
.S LineSolid 
and
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
>>ASSERTION def
When
.M line_style 
is
.S LineSolid 
and
.M fill_style
is
.S FillStippled ,
then on a call to xname the source for the drawing operation is
.M foreground
masked by
the stipple pattern tiled in a single plane.
>>ASSERTION def
When
.M line_style 
is
.S LineOnOffDash 
and the dash is odd,
then nothing is drawn.
>>ASSERTION def
When
.M line_style 
is
.S LineOnOffDash 
and the dash is even and
.M fill_style
is
.S FillSolid ,
then on a call to xname the source pixel for the drawing operation is
.M foreground .
>>ASSERTION Good A
When
.M line_style 
is
.S LineOnOffDash 
and the dash is even and
.M fill_style
is
.S FillTiled ,
then on a call to xname the source for the drawing operation is
.M tile .
>>STRATEGY
Create a tile with depth of drawable.
Set graphics coordinates for dashed lines 
	(includes horizontal and vertical cases,
	and includes joins and caps where relevant).
If depth is greater than one, set the fg and bg in the tile
	to interesting values.
Set the tile component of the GC to tile using XSetTile.
Set fill-style to FillTiled using XSetFillStyle.
Set line-style to LineOnOffDash using XChangeGC.
Do graphics operation.
Pixmap verify.
>>CODE
XVisualInfo	*vp;
Pixmap	fstile;
unsigned long	fsfg, fsbg;
unsigned int	lw;

	for (resetvinf(VI_WIN_PIX); nextvinf(&vp); ) {
		A_DRAWABLE = makewin(A_DISPLAY, vp);
		A_GC = makegc(A_DISPLAY, A_DRAWABLE);

		setfordash();

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
		setlinestyle(A_DISPLAY, A_GC, LineOnOffDash);

		for(lw=5; lw <= 6; lw++) { /* odd and even widths */
			setwidth(A_DISPLAY, A_GC, lw);

			XCALL;

			PIXCHECK(A_DISPLAY, A_DRAWABLE);

			dclear(A_DISPLAY, A_DRAWABLE);
		}
		XFreePixmap(A_DISPLAY, fstile);
	}

	CHECKPASS(2*nvinf());

>>ASSERTION Good A
When
.M line_style 
is
.S LineOnOffDash 
and the dash is even and
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
Create a stipple bitmap.
Set graphics coordinates for dashed lines 
	(includes horizontal and vertical cases,
	and includes joins and caps where relevant).
Set the stipple component of the GC to stipple using XSetStipple.
Set fill-style to FillOpaqueStippled using XSetFillStyle.
Set line-style to LineOnOffDash using XChangeGC.
Do graphics operation.
Pixmap verify.
>>CODE
XVisualInfo	*vp;
Pixmap	fsstip;
unsigned int	lw;

	for (resetvinf(VI_WIN_PIX); nextvinf(&vp); ) {
		A_DRAWABLE = makewin(A_DISPLAY, vp);
		A_GC = makegc(A_DISPLAY, A_DRAWABLE);

		setfordash();

		fsstip = XCreateBitmapFromData(A_DISPLAY, A_DRAWABLE
			, (char*)stipple_bits, stipple_width, stipple_height
			);
		XSetStipple(A_DISPLAY, A_GC, fsstip);

		XSetFillStyle(A_DISPLAY, A_GC, FillOpaqueStippled);
		setlinestyle(A_DISPLAY, A_GC, LineOnOffDash);
		for(lw=5; lw <= 6; lw++) { /* odd and even widths */
			setwidth(A_DISPLAY, A_GC, lw);

			XCALL;

			PIXCHECK(A_DISPLAY, A_DRAWABLE);

			dclear(A_DISPLAY, A_DRAWABLE);
		}
		XFreePixmap(A_DISPLAY, fsstip);
	}

	CHECKPASS(2*nvinf());

>>ASSERTION Good A
When
.M line_style 
is
.S LineOnOffDash 
and the dash is even and
.M fill_style
is
.S FillStippled ,
then on a call to xname the source for the drawing operation is
.M foreground
masked by
the stipple pattern tiled in a single plane.
>>STRATEGY
Create a stipple bitmap.
Set graphics coordinates for dashed lines 
	(includes horizontal and vertical cases,
	and includes joins and caps where relevant).
Set the stipple component of the GC to stipple using XSetStipple.
Set fill-style to FillStippled using XSetFillStyle.
Set line-style to LineOnOffDash using XChangeGC.
Do graphics operation.
Pixmap verify.
>>CODE
XVisualInfo	*vp;
Pixmap	fsstip;
unsigned int	lw;

	for (resetvinf(VI_WIN_PIX); nextvinf(&vp); ) {
		A_DRAWABLE = makewin(A_DISPLAY, vp);
		A_GC = makegc(A_DISPLAY, A_DRAWABLE);

		setfordash();

		fsstip = XCreateBitmapFromData(A_DISPLAY, A_DRAWABLE
			, (char*)stipple_bits, stipple_width, stipple_height
			);
		XSetStipple(A_DISPLAY, A_GC, fsstip);

		XSetFillStyle(A_DISPLAY, A_GC, FillStippled);
		setlinestyle(A_DISPLAY, A_GC, LineOnOffDash);
		for(lw=5; lw <= 6; lw++) { /* odd and even widths */
			setwidth(A_DISPLAY, A_GC, lw);

			XCALL;

			PIXCHECK(A_DISPLAY, A_DRAWABLE);

			dclear(A_DISPLAY, A_DRAWABLE);
		}
		XFreePixmap(A_DISPLAY, fsstip);
	}

	CHECKPASS(2*nvinf());

>>ASSERTION Good A
When
.M line_style 
is
.S LineDoubleDash
and the dash is even and
.M fill_style
is
.S FillSolid ,
then on a call to xname the source pixel for the drawing operation is
.M foreground .
>>STRATEGY
Draw complete path with line-style LineSolid and save results.
Clear drawable.
Draw same path with line-style LineDoubleDash
Reverse foreground and background.
Set gc function to GXor.
Draw same path again over previous drawing.
Check that the combined result is equivalent to using FillSolid.
>>CODE
XVisualInfo	*vp;
XImage	*lsimp;
unsigned int 	lw;

	for (resetvinf(VI_WIN_PIX); nextvinf(&vp); ) {
		A_DRAWABLE = makewin(A_DISPLAY, vp);
		A_GC = makegc(A_DISPLAY, A_DRAWABLE);

		for(lw=4; lw <= 5; lw++) { /* odd and even widths */
			/*
			 * By reversing the foreground and background pixels we should be
			 * able to fill in the whole line. First save the whole line.
			 */
			trace("Draw with LineSolid, width %u, and save result", lw);
			XSetLineAttributes(A_DISPLAY, A_GC, lw, LineSolid, CapButt, JoinMiter);
			XSetForeground(A_DISPLAY, A_GC, W_FG|W_BG);	/* Draw with OR of fg&bg */
			XCALL;
			XSetForeground(A_DISPLAY, A_GC, W_FG);

			lsimp = savimage(A_DISPLAY, A_DRAWABLE);
			dclear(A_DISPLAY, A_DRAWABLE);

			/* Now call with DoubleDash */
			XSetDashes(A_DISPLAY, A_GC, 0, lsdashes, 2);
			XSetLineAttributes(A_DISPLAY, A_GC, lw, LineDoubleDash, CapButt, JoinMiter);
			XCALL;

			/* Reverse fg&bg and or in line */
			XSetFunction(A_DISPLAY, A_GC, GXor);
			XSetForeground(A_DISPLAY, A_GC, W_BG);
			XSetBackground(A_DISPLAY, A_GC, W_FG);
			XCALL;
			XSetForeground(A_DISPLAY, A_GC, W_FG);
			XSetBackground(A_DISPLAY, A_GC, W_BG);

			trace("compare to result of drawing lines with LineDoubleDash");
			if (compsavimage(A_DISPLAY, A_DRAWABLE, lsimp))
				CHECK;
			else {
				report("Reversing fg and bg did not complete line correctly");
				FAIL;
			}
			/* assume we're not allowed to draw nothing, complain if so */
			if (!checkarea(A_DISPLAY, A_DRAWABLE,
				(struct area *)0, W_BG, W_BG, CHECK_IN | CHECK_DIFFER))
				CHECK;
			else {
				report("%s didn't draw anything when ORing together even and odd dashes", TestName);
				FAIL;
			}
			dclear(A_DISPLAY, A_DRAWABLE);
		}

	}

	/*
	 * Now do direct checking on vertical or horizontal lines
	 */
	for (resetvinf(VI_WIN_PIX); nextvinf(&vp); ) {
		A_DRAWABLE = makewin(A_DISPLAY, vp);
		A_GC = makegc(A_DISPLAY, A_DRAWABLE);

		setfordash();
		XSetDashes(A_DISPLAY, A_GC, 0, lsdashes, 2);

		for(lw=4; lw <=5; lw++) { /* odd/even widths */
			trace("Direct check of LineDoubleDash on vertical/horizontal lines width %u", lw);
			XSetLineAttributes(A_DISPLAY, A_GC,
				lw, LineDoubleDash, CapButt, JoinMiter);
			XCALL;
			PIXCHECK(A_DISPLAY, A_DRAWABLE);
			dclear(A_DISPLAY, A_DRAWABLE);
		}
	}

	CHECKPASS(6*nvinf());

>>ASSERTION Good A
When
.M line_style 
is
.S LineDoubleDash
and the dash is even and
.M fill_style
is
.S FillTiled ,
then on a call to xname the source for the drawing operation is
.M tile .
>>STRATEGY
Create a tile with depth of drawable.
Set graphics coordinates for dashed lines 
	(includes horizontal and vertical cases,
	and includes joins and caps where relevant).
If depth is greater than one, set the fg and bg in the tile
	to interesting values.
Set the stipple component of the GC to stipple using XSetStipple.
Set the tile component of the GC to tile using XSetTile.
Set fill-style to FillTiled using XSetFillStyle.
Set line-style to LineDoubleDash using XChangeGC.
Do graphics operation.
Pixmap verify.
>>CODE
XVisualInfo	*vp;
Pixmap	fstile;
unsigned long	fsfg, fsbg;
unsigned int	lw;

	for (resetvinf(VI_WIN_PIX); nextvinf(&vp); ) {
		A_DRAWABLE = makewin(A_DISPLAY, vp);
		A_GC = makegc(A_DISPLAY, A_DRAWABLE);

		setfordash();

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
		setlinestyle(A_DISPLAY, A_GC, LineDoubleDash);
		for(lw=4; lw <= 5; lw++) { /* odd and even widths */
			setwidth(A_DISPLAY, A_GC, lw);

			XCALL;

			PIXCHECK(A_DISPLAY, A_DRAWABLE);
			dclear(A_DISPLAY, A_DRAWABLE);
		}
		XFreePixmap(A_DISPLAY, fstile);
	}

	CHECKPASS(2*nvinf());

>>ASSERTION Good A
When
.M line_style 
is
.S LineDoubleDash
and the dash is even and
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
Create a stipple bitmap.
Set graphics coordinates for dashed lines 
	(includes horizontal and vertical cases,
	and includes joins and caps where relevant).
Set the stipple component of the GC to stipple using XSetStipple.
Set fill-style to FillOpaqueStippled using XSetFillStyle.
Set line-style to LineDoubleDash using XChangeGC.
Do graphics operation.
Pixmap verify.
>>CODE
XVisualInfo	*vp;
Pixmap	fsstip;
unsigned int	lw;

	for (resetvinf(VI_WIN_PIX); nextvinf(&vp); ) {
		A_DRAWABLE = makewin(A_DISPLAY, vp);
		A_GC = makegc(A_DISPLAY, A_DRAWABLE);

		setfordash();

		fsstip = XCreateBitmapFromData(A_DISPLAY, A_DRAWABLE
			, (char*)stipple_bits, stipple_width, stipple_height
			);
		XSetStipple(A_DISPLAY, A_GC, fsstip);

		XSetFillStyle(A_DISPLAY, A_GC, FillOpaqueStippled);
		setlinestyle(A_DISPLAY, A_GC, LineDoubleDash);
		for(lw=4; lw <= 5; lw++) { /* odd and even widths */
			setwidth(A_DISPLAY, A_GC, lw);

			XCALL;

			PIXCHECK(A_DISPLAY, A_DRAWABLE);
			dclear(A_DISPLAY, A_DRAWABLE);
		}
		XFreePixmap(A_DISPLAY, fsstip);
	}

	CHECKPASS(2*nvinf());

>>ASSERTION Good A
When
.M line_style 
is
.S LineDoubleDash
and the dash is even and
.M fill_style
is
.S FillStippled ,
then on a call to xname the source for the drawing operation is
.M foreground
masked by
the stipple pattern tiled in a single plane.
>>STRATEGY
Create a stipple bitmap.
Set graphics coordinates for dashed lines 
	(includes horizontal and vertical cases,
	and includes joins and caps where relevant).
Set the stipple component of the GC to stipple using XSetStipple.
Set fill-style to FillStippled using XSetFillStyle.
Set line-style to LineDoubleDash using XChangeGC.
Do graphics operation.
Pixmap verify.
>>CODE
XVisualInfo	*vp;
Pixmap	fsstip;
unsigned int	lw;

	for (resetvinf(VI_WIN_PIX); nextvinf(&vp); ) {
		A_DRAWABLE = makewin(A_DISPLAY, vp);
		A_GC = makegc(A_DISPLAY, A_DRAWABLE);

		setfordash();
		XSetDashes(A_DISPLAY, A_GC, 0, lsdashes, 2);

		fsstip = XCreateBitmapFromData(A_DISPLAY, A_DRAWABLE
			, (char*)stipple_bits, stipple_width, stipple_height
			);
		XSetStipple(A_DISPLAY, A_GC, fsstip);

		XSetFillStyle(A_DISPLAY, A_GC, FillStippled);
		setlinestyle(A_DISPLAY, A_GC, LineDoubleDash);
		for(lw=4; lw <= 5; lw++) { /* odd and even widths */
			setwidth(A_DISPLAY, A_GC, lw);

			XCALL;

			PIXCHECK(A_DISPLAY, A_DRAWABLE);
			dclear(A_DISPLAY, A_DRAWABLE);
		}
		XFreePixmap(A_DISPLAY, fsstip);
	}

	CHECKPASS(2*nvinf());

>>ASSERTION def
When
.M line-style
is
.S LineDoubleDash
and the dash is odd and
.M fill_style
is
.S FillSolid ,
then on a call to xname the source for the drawing operation is
.M background .
>>ASSERTION def
When
.M line-style
is
.S LineDoubleDash
and the dash is odd and
.M fill_style
is
.S FillTiled ,
then on a call to xname the source for the drawing operation is
.M tile .
>>ASSERTION def
When
.M line-style
is
.S LineDoubleDash
and the dash is odd and
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
>>ASSERTION def
When
.M line-style
is
.S LineDoubleDash
and the dash is odd and
.M fill_style
is
.S FillStippled ,
then on a call to xname the source for the drawing operation is
.M background
masked by
the stipple pattern tiled in a single plane.
>>ASSERTION Good A
A call to xname does not draw each pixel of a particular line more
than once.
>>STRATEGY
For each line-style
  Draw line with gc function GXcopy.
  Save image.
  Clear drawable.
  Draw line with gc function GXxor
  Verify that the image is the same as that saved.
>>CODE
XVisualInfo	*vp;
XImage	*lsimp;
int 	i;
int 	ls;
unsigned int 	lw;

	for (resetvinf(VI_WIN_PIX); nextvinf(&vp); ) {
		A_DRAWABLE = makewin(A_DISPLAY, vp);
		A_GC = makegc(A_DISPLAY, A_DRAWABLE);

		setfordash();
		XSetDashes(A_DISPLAY, A_GC, 0, lsdashes, 2);

		for(lw=4; lw <= 5; lw++) { /* odd and even widths */
			setwidth(A_DISPLAY, A_GC, lw);

			for (i = 0; i < NELEM(linestyles); i++) {
				XSetFunction(A_DISPLAY, A_GC, GXcopy);

				ls = linestyles[i];

				trace("Drawing line style %s width %u", linestylename(ls), lw);
				setlinestyle(A_DISPLAY, A_GC, ls);
				XCALL;
				lsimp = savimage(A_DISPLAY, A_DRAWABLE);
				dclear(A_DISPLAY, A_DRAWABLE);

				XSetFunction(A_DISPLAY, A_GC, GXxor);
				XCALL;
				if (compsavimage(A_DISPLAY, A_DRAWABLE, lsimp))
					CHECK;
				else {
					report("Pixels drawn more than once for %s", linestylename(ls));
					FAIL;
				}
				/* assume we're not allowed to draw nothing, complain if so */
				if (!checkarea(A_DISPLAY, A_DRAWABLE,
					(struct area *)0, W_BG, W_BG, CHECK_IN | CHECK_DIFFER))
					CHECK;
				else {
					report("%s didn't draw anything in Xor mode", TestName);
					FAIL;
				}
				dclear(A_DISPLAY, A_DRAWABLE);
			}
			dclear(A_DISPLAY, A_DRAWABLE);
		}
	}

	CHECKPASS(4*NELEM(linestyles)*nvinf());

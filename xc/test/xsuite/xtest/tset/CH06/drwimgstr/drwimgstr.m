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
 * $XConsortium: drwimgstr.m,v 1.11 94/04/17 21:04:45 rws Exp $
 */
>>TITLE XDrawImageString CH06
void

Display	*display = Dsp;
Drawable d;
GC		gc;
int 	x = 3;
int 	y = 30;
char	*string = "A bCdElMnO";
int 	length = strlen(string);
>>EXTERN

/* Needed because function not used */
#define	A_DRAW	A_DRAWABLE

static	Font	Xtfonts[XT_NFONTS];

>>SET startup localstartup
>>SET cleanup fontcleanup
static void
localstartup()
{
	fontstartup();
	if(Dsp) {
		openfonts(Xtfonts, XT_NFONTS);
		setgcfont(Xtfonts[1]);
	}
}

>>ASSERTION Good A
A call to xname first fills the destination rectangle with the
.M background
pixel value and next draws a
.A string
of 8-bit characters, selected from the
.A gc 's
.M font ,
using the
.M foreground
pixel value.
>>STRATEGY
Reverse foreground and background pixel values in the gc.
For each font
  Set the font into the gc.
  Draw string
  Pixel verify.
>>CODE
XVisualInfo	*vp;
int 	fn;
unsigned int 	width;

	for (resetvinf(VI_WIN_PIX); nextvinf(&vp); ) {
		d = makewin(display, vp);
		gc = makegc(display, d);

		XSetForeground(display, gc, W_BG);
		XSetBackground(display, gc, W_FG);

		getsize(display, d, &width, (unsigned int*)0);

		for (fn = 0; fn < XT_NFONTS; fn++) {

			XSetFont(display, gc, Xtfonts[fn]);
			if (fn == 4 || fn == 6)
				x = width-4;
			else
				x = 3;

			XCALL;

			PIXCHECK(display, d);
			dclear(display, d);
		}
	}

	CHECKPASS(XT_NFONTS*nvinf());
>>ASSERTION def Good A
>># NOTE is it always the Top-left corner, what if font is R-to-L ??? Ask Bob?
The upper-left corner of the filled rectangle is at
[
.A x ,
.A y
- font-ascent],
with width equal to the sum of the per-character
.M width s,
and with height
font-ascent + font-descent, where
font-ascent and font-descent
are the logical ascent and descent of the
.M font .
>>ASSERTION Good A
The origin of the string is at
[
.A x ,
.A y
].
>>STRATEGY
Vary x and y
  Draw string
Pixmap verify.
>>CODE
XVisualInfo	*vp;
unsigned int 	width, height;

	string = "v";
	length = 1;

	for (resetvinf(VI_WIN_PIX); nextvinf(&vp); ) {
		d = makewin(display, vp);
		gc = makegc(display, d);

		getsize(display, d, &width, &height);

		for (x = 0; x < width; x += 20) {
			for (y = 0; y < width; y += 20) {
				XCALL;
			}
		}

		PIXCHECK(display, d);
	}

	CHECKPASS(nvinf());

>>ASSERTION Good A
The effective
.M function 
is 
.S GXcopy , 
and the effective
.M fill_style
is
.S FillSolid . 
>>STRATEGY
Set gc funtion to GXxor
Set fill-style to FillTiled
Set tile.
Verify that these settings have no effect.
>>CODE
XVisualInfo	*vp;
XImage	*sav;
Pixmap	tile;

	for (resetvinf(VI_WIN_PIX); nextvinf(&vp); ) {
		d = makewin(display, vp);
		gc = makegc(display, d);

		XSetForeground(display, gc, W_BG);
		XSetBackground(display, gc, W_FG);

		XCALL;
		sav = savimage(display, d);
		dclear(display, d);

		/* Make pixmap with same depth as drawable under test */
		tile = makepixm(display, vp);
		pattern(display, tile);

		XSetFunction(display, gc, GXxor);
		XSetFillStyle(display, gc, FillTiled);
		XSetTile(display, gc, tile);

		XCALL;
		XCALL;

		if (compsavimage(display, d, sav))
			CHECK;
		else {
			report("Effective function and fill style test failed");
			FAIL;
		}
	}

	CHECKPASS(nvinf());

>>ASSERTION Good B 3
When the font is defined with 2-byte matrix indexing, then
each byte is used as a byte2 with a byte1 of zero.
>>ASSERTION gc
On a call to xname the GC components
.M plane-mask ,
.M font ,
.M subwindow-mode ,
.M clip-x-origin ,
.M clip-y-origin ,
and
.M clip-mask
are used.
>>ASSERTION gc
On a call to xname the GC mode-dependent components
.M foreground
and
.M background
are used.
>>ASSERTION Bad A
.ER BadDrawable
>>ASSERTION Bad A
.ER BadGC
>>ASSERTION Bad A
.ER BadMatch inputonly
>>ASSERTION Bad A
.ER BadMatch gc-drawable-depth
>>ASSERTION Bad A
.ER BadMatch gc-drawable-screen
>># HISTORY kieron Completed	Check format and pass ac

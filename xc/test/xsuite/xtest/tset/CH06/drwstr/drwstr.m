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
 * $XConsortium: drwstr.m,v 1.9 94/04/17 21:05:12 rws Exp $
 */
>>TITLE XDrawString CH06
void

Display	*display = Dsp;
Drawable d;
GC		gc;
int 	x = 2;
int 	y = 20;
char	*string = "AbCdEfGhIjKlMnOpQrStUv";
int 	length = strlen(string);
>>EXTERN

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

static	void
fillbuf(bp)
char	*bp;
{
int 	i;

	for (i = 0; i < 256; i++)
		*bp++ = i;
}
>>ASSERTION Good A
On a call to xname
the image of each 8-bit character in the
.A string ,
as defined by the
.M font 
in the
.A gc ,
is treated as an
additional mask for a fill operation on the
.A drawable .
>>STRATEGY
For each font
  Draw string to draw each character in font.
  Pixmap verify.
>>CODE
XVisualInfo	*vp;
char	buf[256];
unsigned int 	width, height;
int 	ncheck;
int 	c;
int 	fn;

	fillbuf(buf);

	for (resetvinf(VI_WIN_PIX); nextvinf(&vp); ) {
		d = makewin(display, vp);
		gc = makegc(display, d);

		getsize(display, d, &width, &height);

		ncheck = 0;
		for (fn = 0; fn < XT_NFONTS; fn++) {
			XSetFont(display, gc, Xtfonts[fn]);

			if (fn == 4 || fn == 6)
				x = width - 4;
			else
				x = 0;

			for (c = 0; c < 256; ) {
				debug(1, "Chars from %d...", c);
				for (y = 20; y < height; y += 20) {
					if (c < 256) {
						string = buf+c;
						length = (256-c < 8)? 256-c: 8;
						c += 8;

						XCALL;
					}
				}
				debug(1, "..to char %d", c);

				ncheck++;
				PIXCHECK(display, d);
				dclear(display, d);
			}
		}
	}

	CHECKPASS(ncheck*nvinf());

>>ASSERTION Good B 3
When the font is defined with 2-byte matrix indexing, then
each byte is used as a byte2 with a byte1 of zero.
>>ASSERTION Good A
The origin of the string is at
[
.A x ,
.A y
].
>>STRATEGY
Vary x and y.
Draw string with single character.
Pixmap verify.
>>CODE
XVisualInfo	*vp;
unsigned int 	width, height;

	string = "z";
	length = 1;

	for (resetvinf(VI_WIN_PIX); nextvinf(&vp); ) {
		d = makewin(display, vp);
		gc = makegc(display, d);

		getsize(display, d, &width, &height);

		for (x = 0; x < width; x += 15) {
			for (y = 0; y < height; y += 15) {
				XCALL;
			}
		}

		PIXCHECK(display, d);
	}

	CHECKPASS(nvinf());

>>ASSERTION def
The drawable is modified only where the font character has a bit set to 1.
>>ASSERTION gc
On a call to xname the GC components
.M function ,
.M plane-mask ,
.M fill-style ,
.M font ,
.M subwindow-mode ,
.M clip-x-origin ,
.M clip-y-origin
and
.M clip-mask
are used.
>>ASSERTION gc
On a call to xname the GC mode-dependent components
.M foreground ,
.M background ,
.M tile ,
.M stipple ,
.M tile-stipple-x-origin
and
.M tile-stipple-y-origin
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
>># HISTORY kieron Completed	Checked and passed ac

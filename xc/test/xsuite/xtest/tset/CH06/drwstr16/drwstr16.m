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
 * $XConsortium: drwstr16.m,v 1.10 94/04/17 21:05:14 rws Exp $
 */
>>TITLE XDrawString16 CH06
void

Display	*display = Dsp;
Drawable d;
GC		gc;
int 	x = 2;
int 	y = 30;
XChar2b	*string = defstr;
int 	length = NELEM(defstr);
>>EXTERN

static	XChar2b	defstr[] = {
	{1, 74},
	{1, 79},
	{48, 120},
	{36, 116},
	{36, 80},
	{48, 43},
	{33, 45},
};

static	Font	Xtfonts[XT_NFONTS];

>>SET startup localstartup
>>SET cleanup fontcleanup
static void
localstartup()
{
	fontstartup();
	if(Dsp) {
		openfonts(Xtfonts, XT_NFONTS);
		setgcfont(Xtfonts[2]);
	}
}
>>ASSERTION Good A
On a call to xname
the image of each 2-byte or 16-bit character in the
.A string ,
as defined by the
.M font 
in the
.A gc ,
is treated as an
additional mask for a fill operation on the
.A drawable .
>>STRATEGY
Draw string in different fonts
Pixmap verify.
>>CODE
XVisualInfo	*vp;

	for (resetvinf(VI_WIN_PIX); nextvinf(&vp); ) {
		d = makewin(display, vp);
		gc = makegc(display, d);

		XSetFont(display, gc, Xtfonts[2]);
		x = 2;
		y = 30;
		XCALL;

		XSetFont(display, gc, Xtfonts[6]);
		x = 100;
		y = 60;
		XCALL;

		PIXCHECK(display, d);
	}

	CHECKPASS(nvinf());

>>ASSERTION Good B 3
When fonts are defined with linear indexing rather than 2-byte matrix indexing,
then each 
.S XChar2b 
structure is interpreted as a 16-bit number with byte1 as the 
most-significant byte.
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
static XChar2b	ti[] = {
	{33, 89},
	};
unsigned int 	width, height;

	string = ti;
	length = NELEM(ti);

	for (resetvinf(VI_WIN_PIX); nextvinf(&vp); ) {
		d = makewin(display, vp);
		gc = makegc(display, d);

		getsize(display, d, &width, &height);

		for (x = 0; x < width; x += 20) {
			for (y = 0; y < height; y += 20) {
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
.M clip-y-origin ,
and
.M clip-mask
are used.
>>ASSERTION gc
On a call to xname the GC mode-dependent components
.M foreground ,
.M background ,
.M tile ,
.M stipple ,
.M tile-stipple-x-origin ,
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
>># HISTORY kieron Completed	Check format and pass ac

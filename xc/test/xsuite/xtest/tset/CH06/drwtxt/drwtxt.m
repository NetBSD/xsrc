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
 * $XConsortium: drwtxt.m,v 1.10 94/04/17 21:05:16 rws Exp $
 */
>>TITLE XDrawText CH06
void

Display	*display = Dsp;
Drawable d;
GC		gc;
int 	x = 0;
int 	y = 20;
XTextItem	*items = deftext;
int 	nitems = NELEM(deftext);
>>EXTERN

static XTextItem	deftext[] = {
	{"hello", 5, 0, None},
	{"world", 5, 20, None},
};

/*
 * Since we are not interested in testing loading fonts in this test then
 * they will all be opened at the beginning, and no test will be run
 * if they cannot be loaded.
 */
static Font	Xtfonts[XT_NFONTS];

>>SET startup localstartup
>>SET cleanup fontcleanup
static void
localstartup()
{
	fontstartup();
	if(Dsp) {
		openfonts(Xtfonts, XT_NFONTS);
		deftext[0].font = Xtfonts[1];
	}
}

static void
fillbuf(bp)
char	*bp;
{
int 	i;

	for (i = 0; i < 256; i++)
		*bp++ = i;
}

>>ASSERTION Good A
On a call to xname each of the text
.A items ,
specifying a string
.M chars
of 8-bit characters
from a
.M font
with interstring spacing given by
.M delta ,
is drawn in turn.
>>STRATEGY
Draw all the characters between 0&255 in all the xtest fonts, by setting
up XTestItem structs to point to groups of characters at a time.
Pixmap verify.
>>EXTERN
#define	T1_NITEMS 3
#define	T1_GROUPSIZE 3
>>CODE
XVisualInfo	*vp;
XTextItem	ti[T1_NITEMS];
char	buf[256];
int 	c;
int 	delta;
unsigned int 	width, height;
int 	i;
int 	fn;
int 	ncheck = 0;

	fillbuf(buf);

	for (resetvinf(VI_WIN_PIX); nextvinf(&vp); ) {
		d = makewin(display, vp);
		gc = makegc(display, d);

		for (i = 0; i < T1_NITEMS; i++)
			ti[i].font = None;

		x = 4;

		items = ti;
		nitems = T1_NITEMS;
		delta = 0;

		getsize(display, d, &width, &height);
		for (fn = 0; fn < XT_NFONTS; fn++) {
			trace("Using font xtfont%d\n", fn);
			ti[0].font = Xtfonts[fn];
			/*
			 * Fonts 4 and 6 are in the Right to Left direction.
			 * When the font info structures are implemented could use
			 * them here.
			 */
			if (fn == 4 || fn == 6)
				x = width - 4;
			else
				x = 4;

			for (c = 0; c < 256; ) {
				debug(1, "Chars from %d...", c);
				for (y = 20; y < height; y += 20) {
					for (i = 0; i < T1_GROUPSIZE; i++) {
						if (c < 256) {
							ti[i].chars = buf+c;
							ti[i].nchars = (256-c<=T1_GROUPSIZE)? 256-c: T1_GROUPSIZE;
							c += T1_GROUPSIZE;
							ti[i].delta = delta;
							if (delta++ >= 7)
								delta = -2;
						}
					}
					XCALL;
				}
				debug(1, "..to char %d", c);
				/*
				 * Since font2 does not have a row zero then nothing
				 * should be printed.
				 */
				if (fn == 2 && !checkclear(display, d)) {
					report("Something was drawn when using xtfont2");
					FAIL;
				}
				ncheck++;
				PIXCHECK(display, d);
				dclear(display, d);
			}
		}

	}

	CHECKPASS(ncheck);

>>ASSERTION Good B 3
When the font is defined with 2-byte matrix indexing, then
each byte is used as a byte2 with a byte1 of zero.
>>ASSERTION Good A
A
.M font
other than 
.S None 
in one of the
.A items
causes the
.M font
to be stored in the GC
and used for subsequent text.
>>STRATEGY
Set a different font in each of the members of a XTextItem array.
Draw text using XTextItem array.
Pixmap verify.
>>CODE
XVisualInfo	*vp;
XTextItem	ti[XT_NFONTS];
int 	fn;

	items = ti;
	nitems = NELEM(ti);
	x = 20;
	y = 50;

	/*
	 * Some fonts are left-to-right some left-to-right so this makes
	 * an interesting mess.
	 */
	for (fn = 0; fn < XT_NFONTS; fn++) {
		ti[fn].chars = "\104";	/* hex 44, dec 68 */
		ti[fn].nchars = 1;
		ti[fn].delta = 2;
		ti[fn].font = Xtfonts[fn];
	}

	for (resetvinf(VI_WIN_PIX); nextvinf(&vp); ) {
		d = makewin(display, vp);
		gc = makegc(display, d);

		XCALL;

		PIXCHECK(display, d);
	}

	CHECKPASS(nvinf());

>>ASSERTION Good A
On a call to xname the final drawing position of the string
.M chars
is given by adding
.M delta
to the x coordinate of the initial position of the string origin.
>>STRATEGY
For a series of positive and negative values of delta
  Draw character string
  Verify that character drawn in specified place.
>>CODE
XVisualInfo	*vp;
static XTextItem	ti[] = {
	{"\1", 1, 30, None},
	{"\1", 1, -25, None},
	{"\1", 1, 36, None},
};
struct	area	area;
int 	pos;
int 	i;

	ti[0].font = Xtfonts[0];
	items = ti;
	nitems = NELEM(ti);

	for (resetvinf(VI_WIN_PIX); nextvinf(&vp); ) {
		d = makewin(display, vp);
		gc = makegc(display, d);

		XCALL;

		pos = x;
		for (i = 0; i < NELEM(ti); i++) {
			pos += ti[i].delta;

			/*
			 * Char 1 in font0 is a 10x10 block, so check that it is in
			 * drawn directly.
			 */
			setarea(&area, pos, y-10, 10, 10);
			if (checkarea(display, d, &area, W_FG, W_BG, CHECK_IN))
				CHECK;
			else {
				report("character not drawn in expected place");
				FAIL;
			}

			pos += 10;	/* Width of char1 */
		}
	}

	CHECKPASS(NELEM(ti)*nvinf());
>>ASSERTION Good A
The initial position of the first string's origin is at [
.A x ,
.A y
].
>>STRATEGY
Vary x and y co-ordinates
Draw string
Pixmap verify.
>>CODE
XVisualInfo	*vp;
XTextItem	ti;
unsigned int 	width, height;

	ti.chars = "AB";
	ti.nchars = 2;
	ti.delta = 0;
	ti.font = Xtfonts[1];

	items = &ti;
	nitems = 1;

	for (resetvinf(VI_WIN_PIX); nextvinf(&vp); ) {
		d = makewin(display, vp);
		gc = makegc(display, d);

		getsize(display, d, &width, &height);
		for (x = 0; x < width; x += 23) {
			for (y = 18; y < height; y += 18) {
				XCALL;
			}
		}

		PIXCHECK(display, d);
	}

	CHECKPASS(nvinf());

>>ASSERTION def
The initial position of the second and subsequent strings
is displaced in the x direction
by a distance equal to the width of the previous string
from the final drawing position of the previous string.
>>ASSERTION def
Each character image, as defined by the
.M font
in the GC, is treated as an
additional mask for a fill operation on the drawable.
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
>>ASSERTION Bad A
.ER BadFont bad-font 
>>STRATEGY
Place a bad font ID into the font member of a XTextItem array.
Draw text using XTextItem array.
Verify that a BadFont error occurs.
>>CODE BadFont

	seterrdef();

	deftext[0].font = badfont(display);

	XCALL;
	if (geterr() == BadFont)
		PASS;
	else
		FAIL;

>>ASSERTION Bad A
When a text item generates a 
.S BadFont 
error, then either the text of 
the previous items is drawn or nothing is drawn.
>>STRATEGY
Place None into the font member of the first XTextItem array.
Set nitems to 1.
Draw text using XTextItem array.
Save image on drawable.
Place a bad font ID into the font member of the second XTextItem array.
Set nitems to 2.
Draw text using XTextItem array.
Verify that either the saved image was drawn or nothing was drawn.
>>CODE
XVisualInfo	*vp;
XImage	*image1;
XImage	*image2;

	seterrdef();

	resetvinf(VI_WIN_PIX); 
	nextvinf(&vp);
	d = makewin(display, vp);
	gc = makegc(display, d);

	/* Assume drawable clear at start of test purpose, and save image */
	image2 = savimage(display, d);

	deftext[0].font = None;
	items = deftext;
	nitems = 1;

	startcall(Dsp);
	if (isdeleted())
		return;
	XDrawText(display, d, gc, x, y, items, nitems);
	endcall(Dsp);
	if (geterr() != Success) {
		delete("Got %s, Expecting Success", errorname(geterr()));
		return;
	} 

	/* Draw text from previous item and save image */
	image1 = savimage(display, d);
	dclear(display, d);

	deftext[1].font = badfont(display);
	items = deftext;
	nitems = 2;

	startcall(Dsp);
	if (isdeleted())
		return;
	XDrawText(display, d, gc, x, y, items, nitems);
	endcall(Dsp);
	if (geterr() != BadFont) {
		delete("Got %s, Expecting BadFont", errorname(geterr()));
		return;
	}

	if (compsavimage(display, d, image1))
		PASS;
	else {
		if (compsavimage(display, d, image2))
			PASS;
		else
		{
			report("When a BadFont error occurred, text was drawn");
			report("differing from the text of the previous item");
			FAIL;
		}
	}


>># HISTORY kieron Completed	Check format and pass ac

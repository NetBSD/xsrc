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
 * $XConsortium: drwtxt16.m,v 1.10 94/04/17 21:05:18 rws Exp $
 */
>>TITLE XDrawText16 CH06
void

Display	*display = Dsp;
Drawable d;
GC		gc;
int 	x = 2;
int 	y = 18;
XTextItem16	*items = deftext;
int 	nitems = NELEM(deftext);
>>EXTERN

static	XChar2b	  chlist1[] = {
	{0, 24},
	{33, 0},
	{33, 45},
	{1, 56},
	{48, 120},
	{36, 116},
	{89, 94},
	{1, 100},
	{1, 240},
};
static	XChar2b	  chlist2[] = {
	{ 1, 89},
	{ 0, 23},
	{ 1, 190},
	{ 3, 89},
	{ 4, 99},
	{48, 43},
	{48, 54 },
	{48, 75},
	{36, 80},
};
static	XChar2b	  chlist3[] = {
	{48, 54},
};

static	XTextItem16	deftext[] = {
	{chlist1, NELEM(chlist1), 0, None},
	{chlist2, NELEM(chlist2), 0, None},
};

/* List of the 16 bit fonts */
static	int 	list16font[] = {
	2, 6};

static	Font	Xtfonts[XT_NFONTS];
>>SET startup localstartup
>>SET cleanup fontcleanup
static void
localstartup()
{
	fontstartup();
	if(Dsp) {
		openfonts(Xtfonts, XT_NFONTS);
		deftext[0].font = Xtfonts[2];
	}
}

>>ASSERTION Good A
On a call to xname each of the text
.A items ,
specifying a string
.M chars
of 16-bit or 2-byte characters
from a
.M font
with interstring spacing given by
.M delta ,
is drawn in turn.
>>STRATEGY
For each 2 byte font.
  Set font in gc with XSetFont.
  Call XDrawText16.
  Move drawing position down.
Pixmap verify
>>CODE
XVisualInfo	*vp;
int 	fn;
unsigned int 	width;
static	XTextItem16	ti[] = {
	{chlist1, NELEM(chlist1), 0, None},
	{chlist2, NELEM(chlist2), 0, None},
};

	items = ti;
	nitems = NELEM(ti);

	for (resetvinf(VI_WIN_PIX); nextvinf(&vp); ) {
		d = makewin(display, vp);
		gc = makegc(display, d);

		y = 0;
		getsize(display, d, &width, (unsigned int*)0);
		for (fn = 0; fn < NELEM(list16font); fn++) {
			y += 20;
			/* This is a reverse font */
			if (list16font[fn] == 6)
				x = width - 8;
			else
				x = 2;
			XSetFont(display, gc, Xtfonts[list16font[fn]]);
			XCALL;
		}

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
For each 16 bit font
  Set font member to Font ID.
  Call XDrawText16.
Pixmap verify.
>>CODE
XVisualInfo	*vp;
static	XTextItem16	ti[] = {
	{chlist1, NELEM(chlist1), 0, None},
	{chlist2, NELEM(chlist2), 0, None},
};
int 	i;
int 	fn;
unsigned int 	width;

	items = ti;
	nitems = NELEM(ti);

	for (resetvinf(VI_WIN_PIX); nextvinf(&vp); ) {
		d = makewin(display, vp);
		gc = makegc(display, d);

		getsize(display, d, &width, (unsigned int*)0);

		x = 2;
		y = 18;
		for (i = 0; i < NELEM(list16font); i++) {
			fn = list16font[i];

			if (fn == 6 || fn == 4)
				x = width-4;
			else
				x = 2;

			ti[0].font = Xtfonts[fn];
			XCALL;
			y += 18;
		}

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
Draw with various values of delta.
Pixmap verify.
>>CODE
XVisualInfo	*vp;
static	XTextItem16	ti = {
	chlist3, NELEM(chlist3), 0, None
};
int 	i;

	ti.font = Xtfonts[2];
	items = &ti;
	nitems = 1;

	for (resetvinf(VI_WIN_PIX); nextvinf(&vp); ) {
		d = makewin(display, vp);
		gc = makegc(display, d);

		for (i = 0; i < 80; i+= 16) {
			ti.delta = i;

			XCALL;
		}

		PIXCHECK(display, d);
	}

	CHECKPASS(nvinf());
>>ASSERTION def
The initial position of the first string's origin is at [
.A x ,
.A y
].
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
Cant be tested by the generic test.
Place a bad font ID into the font member of a XTextItem array.
Draw text using XTextItem array.
Check for BadFont.
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
	XDrawText16(display, d, gc, x, y, items, nitems);
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
	XDrawText16(display, d, gc, x, y, items, nitems);
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

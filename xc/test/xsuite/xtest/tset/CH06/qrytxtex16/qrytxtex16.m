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
 * $XConsortium: qrytxtex16.m,v 1.12 94/04/17 21:05:47 rws Exp $
 */
>>TITLE XQueryTextExtents16 CH06
>># Xlib.h says return is an int, spec does not mention return.
Status
xname
Display	*display = Dsp;
XID 	font_ID;
XChar2b	*string = ch1;
int 	nchars = NELEM(ch1);
int 	*direction_return = &direction;
int 	*font_ascent_return = &font_ascent;
int 	*font_descent_return = &font_descent;
XCharStruct	*overall_return = &overall;
>>SET startup fontstartup
>>SET cleanup fontcleanup
>>EXTERN

static	XChar2b	ch1[] = {
	{0, 34}, {0, 54}, {0, 89}, {0, 100}, {0, 130}, {0, 201}, {0, 234}, {0, 255},
	{1, 34}, {1, 54}, {1, 89}, {1, 100}, {1, 130}, {1, 201}, {1, 234}, {1, 255},
	{2, 34}, {2, 54}, {2, 89}, {2, 100}, {2, 130}, {2, 201}, {2, 234}, {2, 255},
	{33, 34}, {33, 54}, {33, 89}, {33, 100}, {33, 130}, {33, 201}, {33, 234}, {33, 255},
	{36, 34}, {36, 54}, {36, 89}, {36, 100}, {36, 130}, {36, 201}, {36, 234}, {36, 255},
	{38, 34}, {38, 54}, {38, 89}, {38, 100}, {38, 130}, {38, 201}, {38, 234}, {38, 255},
	{48, 34}, {48, 54}, {48, 89}, {48, 100}, {48, 130}, {48, 201}, {48, 234}, {48, 255},
	{120, 34}, {120, 54}, {120, 89}, {120, 100}, {120, 130}, {120, 201}, {120, 234}, {120, 255},
};

static	int 	direction;
static	int 	font_ascent;
static	int 	font_descent;
static	XCharStruct	overall;
>>ASSERTION Good A
>># Improved the wording a bit from that approved ....
>># When the font with font_ID
>># .A font_ID
When the
.A font_ID
argument
is a valid GContext resource,
then a call to xname returns the bounding box of the specified 16-bit or 2-byte
character string,
.A string ,
as rendered in
the font in the corresponding GC's
.M font
field.
>>STRATEGY
Create drawable
Create gc usable with the drawable.
For each xtest font
  Load font
  Set font into gc.
  Set font_ID to be the GContext from the gc
  XQueryTextExtents16.
  Verify by direct calculation from the metrics.
>>CODE
extern	struct	fontinfo	fontinfo[];
extern	int 	nfontinfo;
int 	i;
char	buf[256];
int 	good_direction;
int 	good_font_ascent;
int 	good_font_descent;
XCharStruct	good_overall;
Drawable	d;
GC		gc;
Font	font;

	d = defdraw(display, VI_WIN_PIX);
	gc = makegc(display, d);
	if (isdeleted())
		return;

	for (i = 0; i < nfontinfo; i++) {
		font = XLoadFont(display, fontinfo[i].name);
		XSetFont(display, gc, font);

		font_ID = XGContextFromGC(gc);
		XCALL;

		txtextents16(fontinfo[i].fontstruct, string, nchars, &good_direction,
			&good_font_ascent, &good_font_descent, &good_overall);

		/*
		 * Don't check this because not well enough defined.
		 * Just check that it is one of the allowed values.
		 */
		if (direction != FontLeftToRight && direction != FontRightToLeft) {
			report("Font %s - Direction was %d", fontinfo[i].name);
			FAIL;
		} else
			CHECK;

		if (good_font_ascent != font_ascent) {
			report("Font %s: font ascent was %d, expecting %d",
				fontinfo[i].name, font_ascent, good_font_ascent);
			FAIL;
		} else
			CHECK;
		if (good_font_descent != font_descent) {
			report("Font %s: font descent was %d, expecting %d",
				fontinfo[i].name, font_descent, good_font_descent);
			FAIL;
		} else
			CHECK;
		if (good_overall.lbearing != overall.lbearing) {
			report("Font %s: lbearing was %d, expecting %d",
				fontinfo[i].name, overall.lbearing, good_overall.lbearing);
			FAIL;
		} else
			CHECK;
		if (good_overall.rbearing != overall.rbearing) {
			report("Font %s: rbearing was %d, expecting %d",
				fontinfo[i].name, overall.rbearing, good_overall.rbearing);
			FAIL;
		} else
			CHECK;
		if (good_overall.ascent != overall.ascent) {
			report("Font %s: ascent was %d, expecting %d",
				fontinfo[i].name, overall.ascent, good_overall.ascent);
			FAIL;
		} else
			CHECK;
		if (good_overall.descent != overall.descent) {
			report("Font %s: descent was %d, expecting %d",
				fontinfo[i].name, overall.descent, good_overall.descent);
			FAIL;
		} else
			CHECK;
		if (good_overall.width != overall.width) {
			report("Font %s: width was %d, expecting %d",
				fontinfo[i].name, overall.width, good_overall.width);
			FAIL;
		} else
			CHECK;
	}
	CHECKPASS(8*nfontinfo);
>>ASSERTION Good A
>># Improved the wording a bit from that approved ....
>># When the font with font_ID
>># .A font_ID
When the
.A font_ID
argument
is a valid Font resource,
then a call to xname returns the bounding box of the specified 16-bit or 2-byte
character string,
.A string ,
as rendered in
the font
with font ID
.A font_ID .
>>STRATEGY
For each xtest font
  Load font
  XQueryTextExtents16.
  Verify by direct calculation from the metrics.
>>CODE
extern	struct	fontinfo	fontinfo[];
extern	int 	nfontinfo;
int 	i;
char	buf[256];
int 	good_direction;
int 	good_font_ascent;
int 	good_font_descent;
XCharStruct	good_overall;
Font	font;

	for (i = 0; i < nfontinfo; i++) {
		font = XLoadFont(display, fontinfo[i].name);
		if (isdeleted()) {
			delete("Could not load font '%s'", fontinfo[i].name);
			return;
		}

		font_ID = font;
		XCALL;

		txtextents16(fontinfo[i].fontstruct, string, nchars, &good_direction,
			&good_font_ascent, &good_font_descent, &good_overall);

		/*
		 * Don't check this because not well enough defined.
		 * Just check that it is one of the allowed values.
		 */
		if (direction != FontLeftToRight && direction != FontRightToLeft) {
			report("Font %s - Direction was %d", fontinfo[i].name);
			FAIL;
		} else
			CHECK;

		if (good_font_ascent != font_ascent) {
			report("Font %s: font ascent was %d, expecting %d",
				fontinfo[i].name, font_ascent, good_font_ascent);
			FAIL;
		} else
			CHECK;
		if (good_font_descent != font_descent) {
			report("Font %s: font descent was %d, expecting %d",
				fontinfo[i].name, font_descent, good_font_descent);
			FAIL;
		} else
			CHECK;
		if (good_overall.lbearing != overall.lbearing) {
			report("Font %s: lbearing was %d, expecting %d",
				fontinfo[i].name, overall.lbearing, good_overall.lbearing);
			FAIL;
		} else
			CHECK;
		if (good_overall.rbearing != overall.rbearing) {
			report("Font %s: rbearing was %d, expecting %d",
				fontinfo[i].name, overall.rbearing, good_overall.rbearing);
			FAIL;
		} else
			CHECK;
		if (good_overall.ascent != overall.ascent) {
			report("Font %s: ascent was %d, expecting %d",
				fontinfo[i].name, overall.ascent, good_overall.ascent);
			FAIL;
		} else
			CHECK;
		if (good_overall.descent != overall.descent) {
			report("Font %s: descent was %d, expecting %d",
				fontinfo[i].name, overall.descent, good_overall.descent);
			FAIL;
		} else
			CHECK;
		if (good_overall.width != overall.width) {
			report("Font %s: width was %d, expecting %d",
				fontinfo[i].name, overall.width, good_overall.width);
			FAIL;
		} else
			CHECK;
	}
	CHECKPASS(8*nfontinfo);
>>ASSERTION def
When the
font is defined with linear indexing rather than 2-byte matrix indexing,
then each 
.S XChar2b 
structure is interpreted as a 16-bit number with byte1 as the 
most-significant byte.
>>ASSERTION def
The
.M ascent
field of
.A overall
is set to the maximum of the ascent metrics 
of all characters in the string.
>>ASSERTION def
The
.M descent
field of
.A overall
is set to the maximum of the descent metrics
of all characters in the string.
>>ASSERTION def
The
.M width
field of
.A overall
is set to the sum of the character-width metrics 
of all characters in the string.
>>ASSERTION def
The
.M lbearing
field of
.A overall
is set to the minimum L of all characters in the string, where for each
character L is the left-side-metric plus the sum of the character
widths of all preceding characters in the string.
>>ASSERTION def
The
.M rbearing
field of
.A overall
is set to the maximum R of all characters in the string, where for each
character R is the right-side-bearing metric plus the sum of the
character widths of all preceding characters in the string.
>>ASSERTION def
The font_ascent_return argument is set to the logical ascent of the font,
the font_descent_return argument is set to the logical descent of the font and
the direction_return argument is set to either FontLeftToRight
or FontRightToLeft.
>>ASSERTION def
When the font has no defined default character, then
undefined characters in the string are taken to have all zero metrics.
>>ASSERTION def
Characters with all zero metrics are ignored.
>>ASSERTION def
When the font has no defined default_char, then
the undefined characters in the string are also ignored.
>>ASSERTION Bad A
.ER BadFont bad-fontable
>>STRATEGY
Pass a bad font
Verify BadFont error occurs
Pass a bad gc
Verify BadFont error occurs
>>CODE BadFont

	font_ID = badfont(display);

	XCALL;
	if (geterr() == BadFont)
		CHECK;
	else {
		report("No BadFont occurred with a bad font");
		FAIL;
	}

	font_ID = XGContextFromGC(badgc(display));	/* XXX */
	XCALL;
	if (geterr() == BadFont)
		CHECK;
	else {
		report("No BadFont occurred with a bad gc");
		FAIL;
	}
	CHECKPASS(2);

>># This is wrong ..sr
>>#>>ASSERTION Bad A
>>#.ER BadGC 
>># HISTORY kieron Completed	Reformat and tidy to ca pass

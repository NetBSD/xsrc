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
 * $XConsortium: crtglyphcr.m,v 1.16 94/04/17 21:04:32 rws Exp $
 */
>>TITLE XCreateGlyphCursor CH06
Cursor
XCreateGlyphCursor (display, source_font, mask_font, source_char, mask_char, foreground_color, background_color)
Display *display = Dsp;
Font source_font;
Font mask_font;
unsigned int source_char;
unsigned int mask_char;
XColor *foreground_color = mkcolor(1);
XColor *background_color = mkcolor(0);
>>SET startup fontstartup
>>SET cleanup fontcleanup
>>EXTERN
static char xtfont0[] = "xtfont0";	/* known font, used for source font */
static unsigned int good_char0 = 1;	/* known defined glyph in xtfont0 */
static unsigned int goodblank_char0 = 1;/* blank defined glyph in xtfont0  */
static unsigned int bad_char0 = 0;	/* undefined glyph xtfont0 */
static unsigned int bounding0 = 4;	/* diff. bounding box than good_char0 */

static char xtfont1[] = "xtfont1";	/* font sometimes used for mask font */
static unsigned int good_char1 = 1;	/* known defined glyph in xtfont1 */

static char xtfont6[] = "xtfont6";	/* known 2-byte matrix font */
static unsigned int good_char6 = 504;	/* known defined glyph in xtfont6 */

/*
 * mkcolor() -	return a pointer to a color structure.
 *		flag indicates whether or not color is foreground
 */
static XColor *
mkcolor(flag)
{
	static	XColor	fore;
	static	XColor	back;
	static	int	first = 1;

	if (first)
	{
		first = 0;

		fore.pixel = BlackPixel(display, DefaultScreen(display));
		XQueryColor(display, DefaultColormap(display, DefaultScreen(display)), &fore);
		back.pixel = WhitePixel(display, DefaultScreen(display));
		XQueryColor(display, DefaultColormap(display, DefaultScreen(display)), &back);
	}
	return(flag ? &fore : &back);
}
>>ASSERTION Good B 1
A call to xname creates a
.S Cursor
with colours defined by
.A foreground_color
and
.A background_color
and returns its ID.
>>STRATEGY
Load xtfont0 using XLoadFont.
Call XCreateGlyphCursor with foreground colour W_FG and
background colour W_BG.
Verify that XCreateGlyphCursor returns non-zero.
>>CODE
Cursor qstat;

/* Load xtfont0 using XLoadFont. */
	source_font = XLoadFont(display, xtfont0);
	mask_font = source_font;
	source_char = good_char0;
	mask_char = good_char0;

/* Call XCreateGlyphCursor with foreground colour W_FG and */
/* background colour W_BG. */
	qstat = XCALL;

/* Verify that XCreateGlyphCursor returns non-zero. */
	if (qstat == (Cursor) 0) {
		report("Returned wrong value %ld", (long) qstat);
		FAIL;
	} else
		CHECK;

	CHECKUNTESTED(1);
>>ASSERTION Good B 1
The
.A source_char
specifies the glyph in the
.A source_font
from which the source bitmap is obtained.
>>STRATEGY
Load xtfont0 using XLoadFont.
Call XCreateGlyphCursor with source_char set to
a known defined glyph in font xtfont0.
Verify that XCreateGlyphCursor returns non-zero.
>>#
>># This code is identical to the code for the previous assertion...pc
>>#
>>CODE
Cursor qstat;

/* Load xtfont0 using XLoadFont. */
	source_font = XLoadFont(display, xtfont0);
	mask_font = source_font;
	source_char = good_char0;
	mask_char = good_char0;

/* Call XCreateGlyphCursor with source_char set to */
/* a known defined glyph in font xtfont0. */
	qstat = XCALL;

/* Verify that XCreateGlyphCursor returns non-zero. */
	if (qstat == (Cursor) 0) {
		report("Returned wrong value %ld", (long) qstat);
		FAIL;
	} else
		CHECK;

	CHECKUNTESTED(1);
>>ASSERTION Good B 1
The
.A mask_char
specifies the glyph in the
.A mask_font
from which the mask bitmap is obtained.
>>STRATEGY
Load xtfont0 using XLoadFont.
Load xtfont1 using XLoadFont.
Call XCreateGlyphCursor with mask_char set to
a known defined glyph in font xtfont1.
Verify that XCreateGlyphCursor returns non-zero.
>>CODE
Cursor qstat;

/* Load xtfont0 using XLoadFont. */
	source_font = XLoadFont(display, xtfont0);
/* Load xtfont1 using XLoadFont. */
	mask_font = XLoadFont(display, xtfont1);

	source_char = good_char0;
	mask_char = good_char1;

/* Call XCreateGlyphCursor with mask_char set to */
/* a known defined glyph in font xtfont1. */
	qstat = XCALL;

/* Verify that XCreateGlyphCursor returns non-zero. */
	if (qstat == (Cursor) 0) {
		report("Returned wrong value %ld", (long) qstat);
		FAIL;
	} else
		CHECK;

	CHECKUNTESTED(1);
>>ASSERTION Good B 1
When
.A mask_char
is zero, then all pixels of the source are displayed.
>>STRATEGY
Load xtfont0 using XLoadFont.
Load xtfont1 using XLoadFont.
Call XCreateGlyphCursor with mask_char set to zero.
Verify that XCreateGlyphCursor returns non-zero.
>>CODE
Cursor qstat;

/* Load xtfont0 using XLoadFont. */
	source_font = XLoadFont(display, xtfont0);
/* Load xtfont1 using XLoadFont. */
	mask_font = XLoadFont(display, xtfont1);

	source_char = good_char0;
	mask_char = 0;

/* Call XCreateGlyphCursor with mask_char set to zero. */
	qstat = XCALL;

/* Verify that XCreateGlyphCursor returns non-zero. */
	if (qstat == (Cursor) 0) {
		report("Returned wrong value %ld", (long) qstat);
		FAIL;
	} else
		CHECK;

	CHECKUNTESTED(1);
>>ASSERTION Good B 1
When
.A mask_char
and
.A mask_font
are non-zero and a given bit in the mask bitmap is 1, then
the corresponding pixel of the source is displayed.
>>STRATEGY
Load xtfont0 using XLoadFont.
Load xtfont1 using XLoadFont.
Call XCreateGlyphCursor with mask_font set to non-zero and
mask_char corresponding to a glyph in mask_font which
has at least one bit set to non-zero.
Verify that XCreateGlyphCursor returns non-zero.
>>#
>># This code is identical to the code in an assertion which,
>># at one time, was located two assertions previous.
>>#
>>CODE
Cursor qstat;

/* Load xtfont0 using XLoadFont. */
	source_font = XLoadFont(display, xtfont0);
/* Load xtfont1 using XLoadFont. */
	mask_font = XLoadFont(display, xtfont1);

	source_char = good_char0;
	mask_char = good_char1;

/* Call XCreateGlyphCursor with mask_font set to non-zero and */
/* mask_char corresponding to a glyph in mask_font which */
/* has at least one bit set to non-zero. */
	qstat = XCALL;

/* Verify that XCreateGlyphCursor returns non-zero. */
	if (qstat == (Cursor) 0) {
		report("Returned wrong value %ld", (long) qstat);
		FAIL;
	} else
		CHECK;

	CHECKUNTESTED(1);
>>ASSERTION Good B 1
When
.A mask_char
and
.A mask_font
are non-zero and a given bit in the mask bitmap is 0, then
the corresponding pixel of the source is not displayed.
>>STRATEGY
Load xtfont0 using XLoadFont.
Load xtfont1 using XLoadFont.
Call XCreateGlyphCursor with mask_font set to non-zero and
mask_char corresponding to a glyph in mask_font which
has at least one bit set to zero.
Verify that XCreateGlyphCursor returns non-zero.
>>#
>># This code is identical to that for the previous assertion.
>>#
>>CODE
Cursor qstat;

/* Load xtfont0 using XLoadFont. */
	source_font = XLoadFont(display, xtfont0);
/* Load xtfont1 using XLoadFont. */
	mask_font = XLoadFont(display, xtfont1);

	source_char = good_char0;
	mask_char = good_char1;

/* Call XCreateGlyphCursor with mask_font set to non-zero and */
/* mask_char corresponding to a glyph in mask_font which */
/* has at least one bit set to zero. */
	qstat = XCALL;

/* Verify that XCreateGlyphCursor returns non-zero. */
	if (qstat == (Cursor) 0) {
		report("Returned wrong value %ld", (long) qstat);
		FAIL;
	} else
		CHECK;

	CHECKUNTESTED(1);
>>ASSERTION Good B 1
When a bit in the source bitmap is 1, then
.A foreground_color
is used.
>>STRATEGY
Load xtfont0 using XLoadFont.
Load xtfont1 using XLoadFont.
Call XCreateGlyphCursor with glyph with at least
one bit in the source bitmap set to 1.
Verify that XCreateGlyphCursor returns non-zero.
>>#
>># This code is identical to that for the previous assertion.
>>#
>>CODE
Cursor qstat;

/* Load xtfont0 using XLoadFont. */
	source_font = XLoadFont(display, xtfont0);
/* Load xtfont1 using XLoadFont. */
	mask_font = XLoadFont(display, xtfont1);

	source_char = good_char0;
	mask_char = good_char1;

/* Call XCreateGlyphCursor with glyph with at least */
/* one bit in the source bitmap set to 1. */
	qstat = XCALL;

/* Verify that XCreateGlyphCursor returns non-zero. */
	if (qstat == (Cursor) 0) {
		report("Returned wrong value %ld", (long) qstat);
		FAIL;
	} else
		CHECK;

	CHECKUNTESTED(1);
>>ASSERTION Good B 1
When a bit in the source bitmap is 0, then
.A background_color
is used.
>>STRATEGY
Load xtfont0 using XLoadFont.
Load xtfont1 using XLoadFont.
Call XCreateGlyphCursor with source char corresponding to glyph which
has at least one bit set to zero.
Verify that XCreateGlyphCursor returns non-zero.
>>CODE
Cursor qstat;

/* Load xtfont0 using XLoadFont. */
	source_font = XLoadFont(display, xtfont0);
/* Load xtfont1 using XLoadFont. */
	mask_font = XLoadFont(display, xtfont1);

	source_char = goodblank_char0;
	mask_char = good_char1;

/* Call XCreateGlyphCursor with source char corresponding to glyph which */
/* has at least one bit set to zero. */
	qstat = XCALL;

/* Verify that XCreateGlyphCursor returns non-zero. */
	if (qstat == (Cursor) 0) {
		report("Returned wrong value %ld", (long) qstat);
		FAIL;
	} else
		CHECK;

	CHECKUNTESTED(1);
>>ASSERTION Good B 1
When
.A mask_char
is non-zero, then its glyph
.M origin
is positioned coincidently with
that of
.A source_char .
>>STRATEGY
Load xtfont0 using XLoadFont.
Load xtfont1 using XLoadFont.
Call XCreateGlyphCursor with non-zero mask_char.
Verify that XCreateGlyphCursor returns non-zero.
>>#
>># This code is identical to that for the previous assertion.
>>#
>>CODE
Cursor qstat;

/* Load xtfont0 using XLoadFont. */
	source_font = XLoadFont(display, xtfont0);
/* Load xtfont1 using XLoadFont. */
	mask_font = XLoadFont(display, xtfont1);

	source_char = goodblank_char0;
	mask_char = good_char1;

/* Call XCreateGlyphCursor with non-zero mask_char. */
	qstat = XCALL;

/* Verify that XCreateGlyphCursor returns non-zero. */
	if (qstat == (Cursor) 0) {
		report("Returned wrong value %ld", (long) qstat);
		FAIL;
	} else
		CHECK;

	CHECKUNTESTED(1);
>>ASSERTION Good B 1
The
.M origin
of the
.A source_char
defines the hotspot.
>>#
>># Don't think we need to repeat yet another test for this case...pc
>>#
>>ASSERTION Good B 1
The
.A source_char
and
.A mask_char
need not have the same bounding box metrics.
>>STRATEGY
Load xtfont0 using XLoadFont.
Call XCreateGlyphCursor with source_char and mask_char
with different bounding boxes.
Verify that XCreateGlyphCursor returns non-zero.
>>CODE
Cursor qstat;

/* Load xtfont0 using XLoadFont. */
	source_font = XLoadFont(display, xtfont0);
	mask_font = source_font;
	source_char = good_char0;
	mask_char = bounding0;
/* Call XCreateGlyphCursor with source_char and mask_char */
/* with different bounding boxes. */

	qstat = XCALL;

/* Verify that XCreateGlyphCursor returns non-zero. */
	if (qstat == (Cursor) 0) {
		report("Returned wrong value %ld", (long) qstat);
		FAIL;
	} else
		CHECK;

	CHECKUNTESTED(1);
>>ASSERTION Good B 1
When a cursor is created by a call to xname,
and the
.A source_font
or
.A mask_font
argument is freed be a subsequent call to
.F XFreeFont ,
then the cursor is unaffected.
>>STRATEGY
Load xtfont0 using XLoadQueryFont.
Verify that XLoadQueryFont returns non-zero.
Load xtfont1 using XLoadQueryFont.
Verify that XLoadQueryFont returns non-zero.
Call XCreateGlyphCursor with foreground colour W_FG and background colour W_BG.
Verify that XCreateGlyphCursor returns non-zero.
Call XFreeFont for xtfont0.
Call XFreeFont for xtfont1.
>>CODE
Cursor qstat;
XFontStruct *font_struct0;
XFontStruct *font_struct1;

/* Load xtfont0 using XLoadQueryFont. */
	font_struct0 = XLoadQueryFont(display, xtfont0);
/* Verify that XLoadQueryFont returns non-zero. */
	if (font_struct0 == (XFontStruct *) 0) {
		delete("Could not load and query font \"%s\".", xtfont0);
		return;
	}
	else
		CHECK;
/* Load xtfont1 using XLoadQueryFont. */
	font_struct1 = XLoadQueryFont(display, xtfont1);
/* Verify that XLoadQueryFont returns non-zero. */
	if (font_struct1 == (XFontStruct *) 0) {
		delete("Could not load and query font \"%s\".", xtfont1);
		return;
	}
	else
		CHECK;
	source_font = font_struct0->fid;
	mask_font = font_struct1->fid;
	source_char = good_char0;
	mask_char = good_char1;
/* Call XCreateGlyphCursor with foreground colour W_FG and background colour W_BG. */

	qstat = XCALL;

/* Verify that XCreateGlyphCursor returns non-zero. */
	if (qstat == (Cursor) 0) {
		report("Returned wrong value %ld", (long) qstat);
		FAIL;
	} else
		CHECK;

/* Call XFreeFont for xtfont0. */
	XFreeFont(display, font_struct0);
/* Call XFreeFont for xtfont1. */
	XFreeFont(display, font_struct1);

	CHECKUNTESTED(3);
>>ASSERTION Good B 1
For 2-byte matrix fonts,
the 16-bit value should be formed with the byte1 member
in the most significant byte and the byte2 member in the
least significant byte.
>>STRATEGY
Load 2-byte matrix font xtfont6 using XLoadFont.
Call XCreateGlyphCursor with foreground colour W_FG and
background colour W_BG.
Verify that XCreateGlyphCursor returns non-zero.
>>CODE
Cursor qstat;

/* Load 2-byte matrix font xtfont6 using XLoadFont. */
	source_font = XLoadFont(display, xtfont6);
	mask_font = source_font;
	source_char = good_char6;
	mask_char = good_char6;
/* Call XCreateGlyphCursor with foreground colour W_FG and */
/* background colour W_BG. */

	qstat = XCALL;

/* Verify that XCreateGlyphCursor returns non-zero. */
	if (qstat == (Cursor) 0) {
		report("Returned wrong value %ld", (long) qstat);
		FAIL;
	} else
		CHECK;

	CHECKUNTESTED(1);
>>ASSERTION Bad B 1
.ER BadAlloc
>>ASSERTION Bad A
When the
.A source_char 
argument is not
a defined glyph in
.A source_font ,
then a
.S BadValue
error occurs.
>>STRATEGY
Load xtfont0 using XLoadFont.
Call XCreateGlyphCursor with a source_char which is
an undefined glyph in source_font.
Verify that a BadValue error occurs.
>>CODE BadValue
Cursor qstat;

/* Load xtfont0 using XLoadFont. */
	source_font = XLoadFont(display, xtfont0);
	mask_font = source_font;
	source_char = bad_char0;
	mask_char = good_char0;

/* Call XCreateGlyphCursor with a source_char which is */
/* an undefined glyph in source_font. */
	XCALL;

/* Verify that a BadValue error occurs. */
	if (geterr() == BadValue)
		PASS;
	else
		FAIL;
>>ASSERTION Bad A
When
.A mask_font
is non-zero and
.A mask_char
is non-zero and not a defined glyph in
.A mask_font ,
then a
.S BadValue
error occurs.
>>STRATEGY
Load xtfont0 using XLoadFont.
Call XCreateGlyphCursor with a mask_char which is
an undefined glyph in mask_font.
Verify that a BadValue error occurs.
>>CODE BadValue
Cursor qstat;

/* Load xtfont0 using XLoadFont. */
	source_font = XLoadFont(display, xtfont0);
	mask_font = source_font;
	source_char = good_char0;
	mask_char = bad_char0;

/* Call XCreateGlyphCursor with a mask_char which is */
/* an undefined glyph in mask_font. */
	XCALL;

/* Verify that a BadValue error occurs. */
	if (geterr() == BadValue)
		PASS;
	else
		FAIL;
>>ASSERTION Bad A
.ER BadFont bad-font
>>#HISTORY peterc Completed Updated as per RTCB#3

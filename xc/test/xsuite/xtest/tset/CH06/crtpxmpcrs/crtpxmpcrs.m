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
 * $XConsortium: crtpxmpcrs.m,v 1.15 94/04/17 21:04:32 rws Exp $
 */
>>TITLE XCreatePixmapCursor CH06
Cursor
XCreatePixmapCursor(display, source, mask, foreground_color, background_color, x, y)
Display *display = Dsp;
Pixmap source;
Pixmap mask;
XColor *foreground_color = mkcolor(1);
XColor *background_color = mkcolor(0);
unsigned int x = 0;
unsigned int y = 0;
>>EXTERN
static XVisualInfo depth1;

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
from the
.A source
and 
.A mask
pixmaps
with colours defined by
.A foreground_color
and
.A background_color
and hotspot position given by
.A x
and
.A y
and returns the cursor ID.
>>STRATEGY
Create source pixmap.
Create mask pixmap.
Call XCreatePixmapCursor with foreground colour W_FG and
background colour W_BG and hotspot at (0,0).
Verify that XCreatePixmapCursor returns non-zero.
>>CODE
Cursor qstat;

	depth1.depth = 1;
/* Create source pixmap. */
	source = makepixm(display, &depth1);

/* Create mask pixmap. */
	mask = makepixm(display, &depth1);

/* Call XCreatePixmapCursor with foreground colour W_FG and */
/* background colour W_BG and hotspot at (0,0). */
	qstat = XCALL;

/* Verify that XCreatePixmapCursor returns non-zero. */
	if (qstat == (Cursor) 0) {
		report("Returned incorrect value %ld", (long) qstat);
		FAIL;
	} else
		CHECK;
	
	CHECKUNTESTED(1);
>>ASSERTION Good B 1
When
.A mask
is
.S None ,
then all pixels of the
.A source
are displayed.
>>STRATEGY
Create source pixmap.
Call XCreatePixmapCursor with mask None.
Verify that XCreatePixmapCursor returns non-zero.
>>CODE
Cursor qstat;

	depth1.depth = 1;
/* Create source pixmap. */
	source = makepixm(display, &depth1);

	mask = None;
/* Call XCreatePixmapCursor with mask None. */
	qstat = XCALL;

/* Verify that XCreatePixmapCursor returns non-zero. */
	if (qstat == (Cursor) 0) {
		report("Returned incorrect value %ld", (long) qstat);
		FAIL;
	} else
		CHECK;
	
	CHECKUNTESTED(1);
>>ASSERTION Good B 1
When
.A mask
is other than
.S None
and a given bit in the
.A mask
bitmap is 1, then
the corresponding pixel of
.A source
is displayed.
>>STRATEGY
Create source pixmap.
Create mask pixmap.
Call XCreatePixmapCursor with mask other than none and
mask with at least one bit set.
Verify that XCreatePixmapCursor returns non-zero.
>>CODE
Cursor qstat;

	depth1.depth = 1;
/* Create source pixmap. */
	source = makepixm(display, &depth1);
/* Create mask pixmap. */
	mask = makepixm(display, &depth1);

/* Call XCreatePixmapCursor with mask other than none and */
/* mask with at least one bit set. */
	qstat = XCALL;

/* Verify that XCreatePixmapCursor returns non-zero. */
	if (qstat == (Cursor) 0) {
		report("Returned incorrect value %ld", (long) qstat);
		FAIL;
	} else
		CHECK;
	
	CHECKUNTESTED(1);
>>ASSERTION Good B 1
When
.A mask
is other than
.S None
and a given bit in the
.A mask
bitmap is 0, then
the corresponding pixel of
.A source
is not displayed.
>>STRATEGY
Create source pixmap.
Create mask pixmap.
Call XCreatePixmapCursor with mask other than none and
mask with at least one bit not set.
Verify that XCreatePixmapCursor returns non-zero.
>>CODE
Cursor qstat;

	depth1.depth = 1;
/* Create source pixmap. */
	source = makepixm(display, &depth1);
/* Create mask pixmap. */
	mask = makepixm(display, &depth1);
	dset(display, (Drawable) mask, (unsigned long) 0);

/* Call XCreatePixmapCursor with mask other than none and */
/* mask with at least one bit not set. */
	qstat = XCALL;

/* Verify that XCreatePixmapCursor returns non-zero. */
	if (qstat == (Cursor) 0) {
		report("Returned incorrect value %ld", (long) qstat);
		FAIL;
	} else
		CHECK;
	
	CHECKUNTESTED(1);
>>ASSERTION Good B 1
When a bit in the
.A source
bitmap is 1, then
.A foreground_color
is used.
>>STRATEGY
Create source pixmap.
Create mask pixmap.
Call XCreatePixmapCursor with source bitmap containing
at least one bit set to 1.
Verify that XCreatePixmapCursor returns non-zero.
>>CODE
Cursor qstat;

	depth1.depth = 1;
/* Create source pixmap. */
	source = makepixm(display, &depth1);
/* Create mask pixmap. */
	mask = makepixm(display, &depth1);

/* Call XCreatePixmapCursor with source bitmap containing */
/* at least one bit set to 1. */
	qstat = XCALL;

/* Verify that XCreatePixmapCursor returns non-zero. */
	if (qstat == (Cursor) 0) {
		report("Returned incorrect value %ld", (long) qstat);
		FAIL;
	} else
		CHECK;
	
	CHECKUNTESTED(1);
>>ASSERTION Good B 1
When a bit in the
.A source
bitmap is 0, then
.A background_color
is used.
>>STRATEGY
Create source pixmap.
Create mask pixmap.
Call XCreatePixmapCursor with source bitmap containing
at least one bit set to 0.
Verify that XCreatePixmapCursor returns non-zero.
>>CODE
Cursor qstat;

	depth1.depth = 1;
/* Create source pixmap. */
	source = makepixm(display, &depth1);
	dset(display, (Drawable) source, 0);
/* Create mask pixmap. */
	mask = makepixm(display, &depth1);

/* Call XCreatePixmapCursor with source bitmap containing */
/* at least one bit set to 0. */
	qstat = XCALL;

/* Verify that XCreatePixmapCursor returns non-zero. */
	if (qstat == (Cursor) 0) {
		report("Returned incorrect value %ld", (long) qstat);
		FAIL;
	} else
		CHECK;
	
	CHECKUNTESTED(1);
>>ASSERTION Good B 1
When a
.S Cursor
is created by a call to xname,
and the
.A source
and/or
.A mask
argument is freed by a subsequent call to
.F XFreePixmap ,
then the cursor is unaffected.
>>STRATEGY
Create source pixmap.
Create mask pixmap.
Call XCreatePixmapCursor with mask other than none and
mask with at least one bit set.
Verify that XCreatePixmapCursor returns non-zero.
Call XFreePixmap with source pixmap.
Call XFreePixmap with mask pixmap.
>>CODE
Cursor qstat;

/* Create source pixmap. */
	source = XCreatePixmap(display, DRW(display), 2, 2, 1);
/* Create mask pixmap. */
	mask = XCreatePixmap(display, DRW(display), 2, 2, 1);

/* Call XCreatePixmapCursor with mask other than none and */
/* mask with at least one bit set. */
	qstat = XCALL;

/* Verify that XCreatePixmapCursor returns non-zero. */
	if (qstat == (Cursor) 0) {
		report("Returned incorrect value %ld", (long) qstat);
		FAIL;
	} else
		CHECK;
	
/* Call XFreePixmap with source pixmap. */
	XFreePixmap(display, source);

/* Call XFreePixmap with mask pixmap. */
	XFreePixmap(display, mask);
	
	CHECKUNTESTED(1);
>># Assertion should mention that test is only
>># appropriate for servers which support depth!= 1
>>ASSERTION Bad C
If multiple window depths are supported:
When
.A source
does not have depth one, then a
.S BadMatch 
error occurs.
>>STRATEGY
If only one depth supported report UNSUPPORTED.
Create source pixmap of depth not equal to one.
Create mask pixmap.
Call XCreatePixmapCursor with source pixmap.
Verify that BadMatch error occurred.
>>CODE BadMatch
XVisualInfo *vp;
Cursor qstat;

	depth1.depth = 1;
/* If only one depth supported report UNSUPPORTED. */
	for (resetvinf(VI_PIX); nextvinf(&vp); ) {
		if (vp->depth != 1) {
			depth1.depth = vp->depth;
			break;
		}
	}
	if (depth1.depth == 1) {
                unsupported("Only one depth of window is supported");
		return;
	}
/* Create source pixmap of depth not equal to one. */
	source = makepixm(display, &depth1);
/* Create mask pixmap. */
	depth1.depth = 1;
	mask = makepixm(display, &depth1);

/* Call XCreatePixmapCursor with source pixmap. */
	XCALL;

/* Verify that BadMatch error occurred. */
	if (geterr() == BadMatch)
		PASS;
	else
		FAIL;
>>ASSERTION Bad A
When
.A mask
is not
.S None
and
.A mask
does not have depth one, then a
.S BadMatch 
error occurs.
>>STRATEGY
If only one depth supported report UNSUPPORTED.
Create mask pixmap of depth not equal to one.
Create source pixmap.
Call XCreatePixmapCursor with mask pixmap.
Verify that BadMatch error occurred.
>>CODE BadMatch
XVisualInfo *vp;
Cursor qstat;

	depth1.depth = 1;
/* If only one depth supported report UNSUPPORTED. */
	for (resetvinf(VI_PIX); nextvinf(&vp); ) {
		if (vp->depth != 1) {
			depth1.depth = vp->depth;
			break;
		}
	}
	if (depth1.depth == 1) {
		report("Only one depth supported");
		tet_result(TET_UNSUPPORTED);
		return;
	}
/* Create mask pixmap of depth not equal to one. */
	mask = makepixm(display, &depth1);
/* Create source pixmap. */
	depth1.depth = 1;
	source = makepixm(display, &depth1);

/* Call XCreatePixmapCursor with mask pixmap. */
	XCALL;

/* Verify that BadMatch error occurred. */
	if (geterr() == BadMatch)
		PASS;
	else
		FAIL;
>>ASSERTION Bad A
When
.A mask
is not
.S None
and both
.A source
and
.A mask
are not the same size, then a
.S BadMatch 
error occurs.
>>STRATEGY
Create source pixmap of size 1x1.
Create mask pixmap of size 2x2.
Call XCreatePixmapCursor with mask pixmap.
Verify that BadMatch error occurred.
>>CODE BadMatch
Cursor qstat;

	depth1.depth = 1;
/* Create source pixmap of size 1x1. */
	source = XCreatePixmap(display, DRW(display), 1, 1, 1);
	regid(display, (union regtypes *)&source, REG_PIXMAP);
/* Create mask pixmap of size 2x2. */
	mask = XCreatePixmap(display, DRW(display), 2, 2, 1);
	regid(display, (union regtypes *)&mask, REG_PIXMAP);

/* Call XCreatePixmapCursor with mask pixmap. */
	XCALL;

/* Verify that BadMatch error occurred. */
	if (geterr() == BadMatch)
		PASS;
	else
		FAIL;
>>ASSERTION Bad A
When the hotspot position is not a point within the
.A source
pixmap,
then a
.S BadMatch 
error occurs.
>>STRATEGY
Create source pixmap.
Create mask pixmap.
Call XCreatePixmapCursor with hotspot outside the source pixmap.
Verify that BadMatch error occurred.
>>CODE BadMatch
Cursor qstat;

	depth1.depth = 1;
/* Create source pixmap. */
	source = makepixm(display, &depth1);
/* Create mask pixmap. */
	mask = makepixm(display, &depth1);

	x = 2 * W_STDWIDTH + 1;
	y = 2 * W_STDHEIGHT + 1;

/* Call XCreatePixmapCursor with hotspot outside the source pixmap. */
	XCALL;

/* Verify that BadMatch error occurred. */
	if (geterr() == BadMatch)
		PASS;
	else
		FAIL;
>>ASSERTION Bad B 1
.ER BadAlloc 
>>ASSERTION Bad A
.ER BadPixmap 
>>#HISTORY peterc Completed Updated as per RTCB#3
>>#HISTORY peterc Completed Wrote STRATEGY and CODE

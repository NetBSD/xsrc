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
 * $XConsortium: frcrsr.m,v 1.9 94/04/17 21:05:31 rws Exp $
 */
>>TITLE XFreeCursor CH06
void
XFreeCursor(display, cursor)
Display *display = Dsp;
Cursor cursor;
>>SET startup fontstartup
>>SET cleanup fontcleanup
>>ASSERTION Good B 1
When another resource ID references the 
.A cursor ,
then a call to xname
deletes the association between the
.A cursor
resource ID 
and the specified cursor.
>>STRATEGY
Create cursor and cursor2 as same cursors.
Create window.
Define cursor for window.
Call XFreeCursor with cursor.
Call XFreeCursor with cursor2.
>>CODE
Cursor cursor2;
Window w;
XVisualInfo *vp;
unsigned int shape;


	/* UNSUPPORTED is not allowed */
	shape = config.fontcursor_good;
	if (shape == -1) {
		delete("A value of UNSUPPORTED is not allowed for XT_FONTCURSOR_GOOD");
		return;
	}

	for (resetvinf(VI_WIN); nextvinf(&vp); ) {
/* Create cursor and cursor2 as same cursors. */
		cursor = XCreateFontCursor(display, shape);
		cursor2 = XCreateFontCursor(display, shape);

/* Create window. */
		w = makewin(display, vp);

/* Define cursor for window. */
		XDefineCursor(display, w, cursor);

/* Call XFreeCursor with cursor. */
		XCALL;
		if (geterr() == Success)
			CHECK;
		else
			FAIL;

/* Call XFreeCursor with cursor2. */
		cursor = cursor2;
		XCALL;

		if (geterr() == Success)
			CHECK;
		else
			FAIL;
	}
	
	CHECKUNTESTED(2*nvinf());
>>ASSERTION Good B 1
When no other resource ID references the 
.A cursor ,
then a call to xname
deletes the association between the
.A cursor
resource ID 
and the specified cursor,
and the cursor storage is freed.
>>STRATEGY
Create cursor and cursor2 as same cursors.
Call XFreeCursor with cursor.
Call XFreeCursor with cursor2.
>>CODE
Cursor cursor2;
unsigned int shape;

	/* UNSUPPORTED is not allowed */
	shape = config.fontcursor_good;
	if (shape == -1) {
		delete("A value of UNSUPPORTED is not allowed for XT_FONTCURSOR_GOOD");
		return;
	}

/* Create cursor and cursor2 as same cursors. */
	cursor = XCreateFontCursor(display, shape);
	cursor2 = XCreateFontCursor(display, shape);

/* Call XFreeCursor with cursor. */
	XCALL;
	if (geterr() == Success)
		CHECK;
	else
		FAIL;

/* Call XFreeCursor with cursor2. */
	cursor = cursor2;
	XCALL;

	if (geterr() == Success)
		CHECK;
	else
		FAIL;
	
	CHECKUNTESTED(2);
>>ASSERTION Bad A
.ER BadCursor 
>># HISTORY kieron Completed    Check format and pass ac
>>#HISTORY peterc Completed Wrote STRATEGY and CODE

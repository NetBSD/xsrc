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
 * $XConsortium: frfnt.m,v 1.10 94/04/17 21:05:32 rws Exp $
 */
>>TITLE XFreeFont CH06
void

Display	*display = Dsp;
XFontStruct *font_struct;
>>SET startup fontstartup
>>SET cleanup fontcleanup
>>EXTERN
#define FNAME	"xtfont1"
#define	TEXTSTRING "AbyZ?@"
>>ASSERTION Good A
When another resource references the font with ID
.M fid 
in the
.A font_struct 
argument,
then a call to xname frees all storage associated with the
.A font_struct
structure and destroys the association between the Font ID and the font.
>>STRATEGY
Only the part about destroying the association between Font ID and font
is testable.
Create font_struct.
Save font ID from font_struct.
Call XFreeFont.
Create scratch drawable and gc.
Set font into gc.
Attempt to draw some text.
Verify that a BadFont error occurred.
>>CODE
Font	font;
Drawable	d;
GC		gc;

	font_struct = XLoadQueryFont(display, FNAME);
	if (isdeleted() || font_struct == NULL) {
		delete("Failed to load %s, check that xtest fonts are installed",FNAME);
		return;
	}
	font = font_struct->fid;

	XCALL;

	/* Now try to use the font, should get BadFont */
	d = defdraw(display, VI_WIN_PIX);
	gc = makegc(display, d);

	if (isdeleted())
		return;

	/*
	 * Since gc's can be cached then error can occur anytime between the
	 * XSetFont and the XSync
	 */
	reseterr();
	XSetErrorHandler(error_status);
	XSetFont(display, gc, font);
	XDrawString(display, d, gc, 30, 30, TEXTSTRING, strlen(TEXTSTRING));
	XSync(display, False);
	XSetErrorHandler(unexp_err);

	if (geterr() == BadFont) {
		PASS;
	} else {
		report("Association between Font ID and font was not destroyed");
		FAIL;
	}

>>ASSERTION Good B 3
When no other resource ID references the font with ID
.M fid 
in the
.A font_struct 
argument,
then a call to xname frees all storage associated with the
.A font_struct
structure, destroys the association between the Font ID and the font
and that font is unloaded.
>>ASSERTION Bad A
.ER BadFont bad-font
>>STRATEGY
Call XLoadQueryFont to get font_struct.
Unload the font using the font ID.
Call XFreeFont.
Verify that a BadFont error is generated.
>>CODE BadFont

	font_struct = XLoadQueryFont(display, FNAME);
	if (isdeleted() || font_struct == NULL) {
		delete("Failed to load %s, check that xtest fonts are installed",FNAME);
		return;
	}
	XUnloadFont(display, font_struct->fid);

	XCALL;

	if (geterr() == BadFont)
		PASS;
	else
		FAIL;

>>#HISTORY	Kieron	Completed	Reformat and tidy to ca pass

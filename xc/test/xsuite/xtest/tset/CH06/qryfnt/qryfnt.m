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
 * $XConsortium: qryfnt.m,v 1.9 94/04/17 21:05:46 rws Exp $
 */
>>TITLE XQueryFont CH06
XFontStruct *

Display	*display = Dsp;
XID 	font_ID;
>>SET startup fontstartup
>>SET cleanup fontcleanup
>>EXTERN
extern	int 	checkfsp();
extern	struct	fontinfo fontinfo[];
extern	int 	nfontinfo;
>>ASSERTION Good A
>># Improved the wording a bit from that approved ....
>># When the font with font_ID
>># .A font_ID
When the
.A font_ID
argument
is a valid Font resource,
then a call to xname returns a pointer to an
.S XFontStruct 
structure which contains information on
the font
with font ID
.A font_ID .
>>STRATEGY
For each xtest font
  Load font with XLoadFont.
  Set font_ID to font
  Call XQueryFont.
  Verify returned XFontStruct with known good one.
>>CODE
XFontStruct	*fsp;
Font	font;
int 	i;

	for (i = 0; i < nfontinfo; i++) {
		trace("Loading font %s", fontinfo[i].name);
		font = XLoadFont(Dsp, fontinfo[i].name);
		if (isdeleted())
			return;

		font_ID = font;
		fsp = XCALL;
		if (checkfsp(fsp, fontinfo[i].fontstruct, *fontinfo[i].string))
			CHECK;
		else {
			report("Returned XFontStruct was incorrect");
			FAIL;
		}
	}
	CHECKPASS(nfontinfo);
>>ASSERTION Good A
When the
.A font_ID
argument
specifies a
.S GContext ,
then a call to xname returns a pointer to an
.S XFontStruct 
structure which contains information on
the font in the corresponding GC's
.M font
field.
>>STRATEGY
Load font with 
>>CODE
XFontStruct	*fsp;
Font	font;
GC		gc;
Drawable	d;
int 	i;

	d = defdraw(Dsp, VI_WIN_PIX);
	gc = makegc(Dsp, d);
	if (isdeleted())
		return;

	for (i = 0; i < nfontinfo; i++) {
		trace("Loading font %s", fontinfo[i].name);
		font = XLoadFont(Dsp, fontinfo[i].name);
		if (isdeleted())
			return;

		XSetFont(Dsp, gc, font);

		font_ID = XGContextFromGC(gc);
		fsp = XCALL;
		if (checkfsp(fsp, fontinfo[i].fontstruct, *fontinfo[i].string))
			CHECK;
		else {
			report("Returned XFontStruct was incorrect");
			FAIL;
		}
	}
	CHECKPASS(nfontinfo);
>>ASSERTION Good A
When the
.A font_ID
argument
does not name a valid GContext or Font resource,
then a call to xname returns
.S NULL .
>>STRATEGY
Obtain a bad font ID.
Call XQueryFont.
Verify that null is returned.
>>CODE
Font	font;
XFontStruct	*fsp;

	font_ID = badfont(Dsp);
	fsp = XCALL;

	if (fsp != NULL) {
		report("A non-NULL pointer was returned");
		FAIL;
	} else
		PASS;
		
>># The following has been removed, because it is not true. ..sr
>># >>ASSERTION Bad A
>># .ER BadFont bad-fontable
>># HISTORY kieron Completed	Reformat and tidy to ca pass

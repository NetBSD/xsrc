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
 * $XConsortium: frfntnms.m,v 1.8 94/04/17 21:05:34 rws Exp $
 */
>>TITLE XFreeFontNames CH06
void

char	**list;
>>SET startup fontstartup
>>SET cleanup fontcleanup
>>ASSERTION Good B 3
>># As resolved by MIT, this test is now done using 
>># the return value from XListFonts and XListFontsWithInfo.
>>#
>># Some X11R4 implementations may give memory faults when the return 
>># value of XListFontsWithInfo is passed to XFreeFontNames, but this 
>># is an implementation fault which should be fixed in X11R5.
>>#
>># DPJ Cater	5/4/91
When 
.A list
is a list of font names 
returned by a call to XListFonts,
then a call to xname
frees
.A list
and the font names specified by
.A list .
>>STRATEGY
Get list of names with XListFonts.
Call XFreeFontNames to free list of names.
Verify that no error occurred.
Get list of names with XListFontsWithInfo.
Call XFreeFontNames to free list of names.
Verify that no error occurred.
Result is UNTESTED, unless an error should occur.
>>CODE
int 	count;
XFontStruct	*info;

	list = XListFonts(Dsp, "xtfont?", 4, &count);
	if (list == NULL) {
		delete("XListFonts failed");
		return;
	}

	XCALL;

	if (geterr() == Success)
		CHECK;
	else {
		report("Got %s, Expecting Success", errorname(geterr()));
		FAIL;
	}

	list = XListFontsWithInfo(Dsp, "xtfont?", 4, &count, &info);
	if (list == NULL) {
		delete("XListFontsWithInfo failed");
		return;
	}

	XCALL;

	if (geterr() == Success)
		CHECK;
	else {
		report("Got %s, Expecting Success", errorname(geterr()));
		FAIL;
	}

	CHECKUNTESTED(2);

>># HISTORY kieron Completed	Reformat and tidy to ca pass

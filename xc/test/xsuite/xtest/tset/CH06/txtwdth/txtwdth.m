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
 * $XConsortium: txtwdth.m,v 1.8 94/04/17 21:05:55 rws Exp $
 */
>>TITLE XTextWidth CH06
int

XFontStruct	*font_struct;
char	*string;
int 	count;
>>ASSERTION Good A
A call to xname returns the sum of the character-width metrics of all
characters in the 8-bit character string,
.A string ,
as rendered in the font referenced by
.A font_struct .
>>STRATEGY
The known good font information structures are used so that these tests
  are isolated from XLoadQueryFont.
Make a string consisting of all characters from 0 to 255.
Call XTextWidth.
Verify by direct calculation from the metrics.
>>CODE
extern	struct	fontinfo	fontinfo[];
extern	int 	nfontinfo;
int 	i;
int 	width;
int 	calcwidth;
char	buf[256];

	for (i = 0; i < 256; i++)
		buf[i] = i;
	string = buf;
	count  = 256;

	for (i = 0; i < nfontinfo; i++) {
		font_struct = fontinfo[i].fontstruct;

		width = XCALL;

		calcwidth = txtwidth(font_struct, (unsigned char *)string, count);

		if (width != calcwidth) {
			report("Font %s - width was %d, expecting %d", fontinfo[i].name,
				width, calcwidth);
			FAIL;
		} else
			CHECK;
	}
	CHECKPASS(nfontinfo);
>># HISTORY kieron Completed    Reformat and tidy to ca pass

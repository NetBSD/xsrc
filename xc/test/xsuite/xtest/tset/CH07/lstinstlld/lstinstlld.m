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

 * Copyright 1990, 1991 UniSoft Group Limited.
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
 * $XConsortium: lstinstlld.m,v 1.6 94/04/17 21:06:40 rws Exp $
 */
>>TITLE XListInstalledColormaps CH07
Colormap *
xname
Display	*display = Dsp;
Window	w;
int 	*num_return = &Num;
>>EXTERN

/* Variable to hold the return value in */
static	int 	Num;

>>ASSERTION Good A
A call to xname returns a list, that can be
freed with
.F XFree ,
of the currently installed colourmaps for the screen
of the specified window and the number of such colourmaps
in
.A num_return .
>>STRATEGY
Call xname.
Verify that the number of installed colour maps is between the min and
max limits.
Free return value with XFree.
>>CODE
Colormap	*cmp;
Screen	*screen;

	w = DRW(display);
	cmp = XCALL;

	if (cmp == NULL) {
		report("NULL was returned");
		FAIL;
		return;
	} else
		CHECK;

	screen = ScreenOfDisplay(display, DefaultScreen(display));
	if (*num_return < 1 ||
		*num_return > MaxCmapsOfScreen(screen)) {
		report("Number of installed colourmaps was outside range [1, MaxCmapsOfScreen]");
		FAIL;
	} else
		CHECK;

	XFree((char*)cmp);

	CHECKPASS(2);
>>ASSERTION Bad A
.ER BadWindow

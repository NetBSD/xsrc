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
 * $XConsortium: rclrcrsr.m,v 1.11 94/04/17 21:05:49 rws Exp $
 */
>>TITLE XRecolorCursor CH06
void
XRecolorCursor(display, cursor, foreground_color, background_color)
Display *display = Dsp;
Cursor cursor;
XColor *foreground_color = mkcolor(1);
XColor *background_color = mkcolor(0);
>>SET startup fontstartup
>>SET cleanup fontcleanup
>>EXTERN

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
A call to xname changes the color of the specified cursor,
.A cursor ,
to the specified
.A foreground_color
and
.A background_color .
>>STRATEGY
Create cursor.
Call XRecolorCursor to change foreground to W_BG and
background to W_FG.
>>CODE

/* Create cursor. */
	cursor = makecur(display);

/* Call XRecolorCursor to change foreground to W_BG and */
/* background to W_FG. */

	XCALL;

	if (geterr() != Success)
		FAIL;
	else
		CHECK;

	CHECKUNTESTED(1);
>>ASSERTION Good B 1
When the cursor is being displayed on a screen, then
the change is visible immediately.
>>ASSERTION Bad A
.ER BadCursor 
>># HISTORY kieron Completed    Reformat and tidy to ca pass
>># HISTORY peterc Completed Wrote STRATEGY and CODE

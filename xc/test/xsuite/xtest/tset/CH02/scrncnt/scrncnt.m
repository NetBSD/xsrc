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
 * $XConsortium: scrncnt.m,v 1.9 94/04/17 21:02:43 rws Exp $
 */
>>SET   macro
>>TITLE XScreenCount CH02
int
XScreenCount(display)
Display	*display = Dsp;
>>#
>># COMMENT:	Could maybe assume numbering scheme for screen to be 0,1,2...
>>#		then try to open each one using XOpenDisplay with a modified
>>#		XT_DISPLAY value.
>>#
>># Cal 24/7/91
>># REPLY:	This is now done in the appropriate test for XOpenDisplay.
>># Dave 20/9/91
>>#
>>ASSERTION Good A
A call to xname returns the number of available screens on the server connection
specified by the
.A display
argument.
>>STRATEGY
Obtain the number of available screens using xname.
Verify that the number of screens is that given in parameter XT_SCREEN_COUNT.
>>CODE
int	scrn;
int	s = config.screen_count;

	if (s < 0) {
		delete("Parameter XT_SCREEN_COUNT not set.");
		return;
	}

	scrn = XCALL;
	if (scrn != s) {
		report("%s() returns %d available screen%s.", 
					TestName, scrn, scrn > 1 ? "s" : "");
		report("Expected %d available screen%s.", 
					s, s > 1 ? "s" : "");
		FAIL;
	} else
		CHECK;

	CHECKPASS(1);

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
 * $XConsortium: frgc.m,v 1.7 94/04/17 21:03:56 rws Exp $
 */
>>TITLE XFreeGC CH05
void
XFreeGC(display, gc)
Display *display = Dsp;
GC gc;
>>ASSERTION Good A
A call to xname destroys the GC specified by the
.A gc
argument, and all associated storage.
>>STRATEGY
Create a GC with XCreateGC.
Free the GC with XFreeGC.
Set the clip mask of the gc with XSetClipMask.
Verify that a BadGC error occurred.
>>CODE

	gc = XCreateGC(display, DRW(display), 0L, (XGCValues*)0);
	XCALL;

	startcall(Dsp);

	XSetClipMask(display, gc, None);
	
	endcall(Dsp);

	if(geterr() != BadGC) {
		report("After a successful call to XFreeGC, a call to XSetClipMask");
		report("did not give a BadGC error but instead gave %s", errorname(geterr()));
		FAIL;
	} else
		CHECK;

	CHECKPASS(1);

>>ASSERTION Good A
.ER BadGC
>>#HISTORY	Cal 	Completed	Written in new format and style 7/12/90.
>>#HISTORY	Kieron	Completed		<Have a look>
>>#HISTORY	Cal	Action		Writing code.

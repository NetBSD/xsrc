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
 * $XConsortium: rmvfrmsvst.m,v 1.5 94/04/17 21:06:43 rws Exp $
 */
>>TITLE XRemoveFromSaveSet CH07
void
xname
Display	*display = Dsp;
Window	w;
>>ASSERTION Good A
A call to xname
removes the specified window from the client's save-set.
>>STRATEGY
Create client1.
Add two windows to client1 save-set.
Remove one of the windows with xname.
Verify that window was removed by destroying client1.
>>CODE
Display	*client1;
Window	win;
Window	base;
Window	w1, w2;
XWindowAttributes	atts;
struct	area	area;

	client1 = XOpenDisplay(config.display);

	base = defwin(Dsp);
	if (isdeleted())
		return;

	setarea(&area, 10, 10, 10, 10);
	win = crechild(client1, base, &area);
	XSync(client1, False);

	w1 = crechild(Dsp, win, &area);
	w2 = crechild(Dsp, win, &area);
	if (isdeleted())
		return;
	XAddToSaveSet(client1, w1);
	XAddToSaveSet(client1, w2);

	w = w1;
	display = client1;
	XCALL;

	XCloseDisplay(client1);

	CATCH_ERROR(Dsp);
	if (XGetWindowAttributes(Dsp, w1, &atts) == True) {
		report("Non save-set window was not destroyed");
		FAIL;
	} else
		CHECK;
	if (XGetWindowAttributes(Dsp, w2, &atts) == False) {
		report("Save-set window was destroyed");
		FAIL;
	} else
		CHECK;
	RESTORE_ERROR(Dsp);

	CHECKPASS(2);

>>ASSERTION Bad A
When the specified window was created by the client, then a
.S BadMatch 
error occurs.
>>STRATEGY
Create window.
Call xname with the window and the client used to create it.
Verify that a BadMatch error occurs.
>>CODE BadMatch

	w = defwin(display);
	XCALL;

	if (geterr() == BadMatch)
		PASS;
	else
		FAIL;
>>ASSERTION Bad A
.ER BadWindow 

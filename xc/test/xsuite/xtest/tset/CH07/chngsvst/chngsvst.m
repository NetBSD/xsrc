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
 * $XConsortium: chngsvst.m,v 1.5 94/04/17 21:06:15 rws Exp $
 */
>>TITLE XChangeSaveSet CH07
void
xname()
Display *display = Dsp;
Window	w;
int 	change_mode = SetModeInsert;
>># All things to do with what actually happens when the client
>># dies are in addtsvst/addtsvst.m.
>>ASSERTION Good A
When a call to xname is made with a
.A change_mode
of
.S SetModeInsert ,
then the specified window is inserted into the client's save-set.
>>STRATEGY
Create client1.
Client1 creates win window.
Create test window as inferior of win window using different client.
Call xname with change_mode SetModeInsert.
Verify that window has been added by destroying client1.
>>CODE
Display	*client1;
XWindowAttributes	atts;
Window	win;
Window	base;
struct	area	area;

	client1 = XOpenDisplay(config.display);

	base = defwin(Dsp);
	if (isdeleted())
		return;

	setarea(&area, 10, 10, 10, 10);
	win = crechild(client1, base, &area);
	XSync(client1, False);

	w = crechild(Dsp, win, &area);
	display = client1;
	change_mode = SetModeInsert;

	XCALL;

	XCloseDisplay(client1);

	CATCH_ERROR(Dsp);
	if (XGetWindowAttributes(Dsp, w, &atts) == False) {
		report("Save-set window was destroyed");
		FAIL;
	} else
		CHECK;
	RESTORE_ERROR(Dsp);

	CHECKPASS(1);

>>ASSERTION Good A
When a call to xname is made with a
.A change_mode
of
.S SetModeDelete ,
then the specified window is deleted from the client's save-set.
>>STRATEGY
Create client1.
Create win window with client1.
Create two inferiors of win window using different client.
Add both to save set.
Remove one of them from save-set with xname.
Destroy client1.
Verify that window that was removed is destroyed.
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
	change_mode = SetModeDelete;
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
When the specified window was created by the same client, then a
.S BadMatch
error occurs.
>>STRATEGY
Create window.
Call xname with window and same client.
Verify that a BadMatch error occurs.
>>CODE BadMatch

	w = defwin(display);

	XCALL;

	if (geterr() == BadMatch)
		PASS;
	else
		FAIL;
>>ASSERTION Bad A
.ER BadValue change_mode SetModeInsert SetModeDelete
>>ASSERTION Bad A
.ER BadWindow

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
 * $XConsortium: flsh.m,v 1.14 94/04/17 21:07:39 rws Exp $
 */
>>TITLE XFlush CH08
void
XFlush(display)
Display *display = Dsp;
>>ASSERTION Good A
A call to xname flushes the output buffer.
>>STRATEGY
Create client2.
Discard all events on the event queue.
Create pixmap.
Empty the buffer.
Ensure the server has dealt with anything flushed to it: do XSync()
Call XFreePixmap from client2.
Call XSync from client2 to insure all potential errors arrive.
Verify that an error did occur.
Discard all events on the event queue.
Create pixmap.
Call XFlush with display.
Empty the buffer.
Ensure the server has dealt with anything flushed to it: do XSync()
Check for unexpected errors after flushing.
Call XFreePixmap from client2.
Call XSync from client2 to insure all potential errors arrive.
Verify that an error did not occur.
>>CODE
Pixmap	pm;
Display *client2;

/* Create client2. */
	client2 = opendisplay();
	if (client2 == (Display *) NULL) {
		delete("Can not open display");
		return;
	}
	else
		CHECK;
/* Discard all events on the event queue. */
	XSync(display, True);
/* Create pixmap. */
	/* avoid using makepixm() */
	pm = XCreatePixmap(display, DRW(display), 10, 10, 1);

/* Empty the buffer. */
	(void)XTestDiscard(display);
/* Ensure the server has dealt with anything flushed to it: do XSync() */
	XSync(display, False);
/* Call XFreePixmap from client2. */
	_startcall(client2);
	XFreePixmap(client2, pm);
/* Call XSync from client2 to insure all potential errors arrive. */
	XSync(client2, False);
	_endcall(client2);
/* Verify that an error did occur. */
	if (geterr() == Success) {
		report("Flushing appears to happen automatically");
		UNTESTED;
		/* Without this CHECK we get path count errors. */
		CHECK;
	}
	else
		CHECK;
/* Discard all events on the event queue. */
	XSync(display, True);
/* Create pixmap. */
	/* avoid using makepixm() */
	pm = XCreatePixmap(display, DRW(display), 10, 10, 1);
/* Call XFlush with display. */
	_startcall(display);
	XFlush(display);
/* Empty the buffer. */
	(void)XTestDiscard(display);
/* Ensure the server has dealt with anything flushed to it: do XSync() */
	XSync(display, False);
	_endcall(display);
/* Check for unexpected errors after flushing. */
	if (geterr() != Success) {
		delete("Unexpected error '%s' after XFlush.", errorname(geterr()));
		XFreePixmap(display, pm);
		return;
	}
	else
		CHECK;
/* Call XFreePixmap from client2. */
	_startcall(client2);
	XFreePixmap(client2, pm);
/* Call XSync from client2 to insure all potential errors arrive. */
	XSync(client2, False);
	_endcall(client2);
/* Verify that an error did not occur. */
	if (geterr() != Success) {
		report("Flushing did not occur - error '%s'.", errorname(geterr()));
		FAIL;
	}
	else
		CHECK;

	CHECKPASS(4);

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
 * $XConsortium: ungrbsrvr.m,v 1.4 94/04/17 21:07:05 rws Exp $
 */
>>TITLE XUngrabServer CH07
void

Display	*display = Dsp;
>>ASSERTION Good A
A call to xname
resumes processing of requests and close downs on other connections.
>>EXTERN

#include	"Xatom.h"

>>STRATEGY
Create second connection client2.
Call XGrabServer on default display.
Create another process.
  In created process:
  Perform a XChangeProperty request for client2.
Wait sufficient time to allow client2 request to be processed.
Check that no PropertyChange event has been produced.
Call xname to Ungrab server.
Wait sufficient time to allow client2 request to be processed.
Verify that event has now been received.
>>EXTERN

static Display	*client2;

static Window	win;

static int 	waittime;

>>CODE
int 	ret;

	client2 = opendisplay();

	win = defwin(Dsp);
	XSelectInput(display, win, PropertyChangeMask);
	XSelectInput(client2, win, PropertyChangeMask);

	waittime = 2*config.speedfactor + 5;

	if (isdeleted())
		return;

	XGrabServer(display);
	XSync(display, False);

	ret = tet_fork(cproc, pproc, waittime, 1);
	/* Test now continues in pproc */

	if (ret == TIMEOUT_EXIT) {
		delete("Child process timed out");
	}
>>EXTERN
static void
pproc()
{
XEvent	ev;
int 	pass = 0, fail = 0;

	/* Allow time for change prop to occur */
	sleep(waittime);

	if (XCheckWindowEvent(display, win, PropertyChangeMask, &ev)) {
		delete("A request was processed for other than the grabbing client");
		FAIL;
	} else
		CHECK;

	/*
	 * Release the grab and wait a bit to allow the requests on the other
	 * connection to go ahead.
	 */
	XCALL;

	/* Allow time for change prop to occur */
	sleep(waittime);
	if (XCheckWindowEvent(display, win, PropertyChangeMask, &ev))
		CHECK;
	else {
		report("Requests were not processed after server grab was released.");
		FAIL;
	}

	CHECKPASS(2);
}
>>EXTERN
/*
 * Perform operation on client2 while display has the server grabbed.
 */
static void
cproc()
{
long 	val;
Atom	name;

	val = 5;

	settimeout(waittime*3);

	name = XInternAtom(client2, "name", False);
	XChangeProperty(client2, win, name, XA_INTEGER, 32, PropModeReplace,
		(unsigned char *)&val, 1);
	XFlush(client2);

	cleartimeout();

	exit(0);
}

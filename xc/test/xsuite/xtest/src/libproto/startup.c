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
 * $XConsortium: startup.c,v 1.10 94/04/17 21:01:40 rws Exp $
 */

#include "stdio.h"
#include "unistd.h"
#include "string.h"
#include "tet_api.h"
#include "xtest.h"
#include "Xlib.h"
#include "Xutil.h"
#include "xtestlib.h"
#include "XstlibInt.h"

/*
 * Generic startup routine for tests.  All tests in general call
 * this routine unless there is a good reason not to.
 * 
 * This version of startup.c is for the X protocol test suite.
 */

#define LINELEN 1024
char    buf[LINELEN];

Display *Dsp;

extern	int 	ntests;

void
protostartup()
{
int 	i;
char	*disp;
char	*cp;
extern	char	*TestName;
extern	struct	tet_testlist	tet_testlist[];

/* error handlers from libxtest */
extern	int	io_err(),	unexp_err();

	/*
	 * Set the debug level first (it is used in initconfig).
	 */
	if ((cp = tet_getvar("XT_DEBUG")) != NULL) {
	extern	int 	DebugLevel;

		setdblev(atoi(cp));
	} else {
		setdblev(0);
	}

	/*
	 * Obtain and check all configuration parameters.
	 */
	initconfig();

	/*
	 * Set local variables from configuration parameters.
	 */
	checkconfig();

	/*
	 * Put out the NAME info line for the report generator.
	 */
        (void) sprintf(buf, "TRACE:NAME: %s", TestName);
        tet_infoline(buf);

	/*
	 * Pause a while in case the X server is resetting.
	 */
	reset_delay();

	/*
	 * Get the display to use and open it.
	 */
	disp = tet_getvar("XT_DISPLAY");

	if (disp == (char*)0) {
		cancelrest("XT_DISPLAY not set");
		return;
	}

	/* 
	 * In the X Protocol tests, Dsp is not used - the main reason for 
	 * opening a connection here is to prevent further server resets
	 * between test purposes.
	 */
	(void) XSetErrorHandler(unexp_err); /* unexp_err() can rely
				on Dsp as not called unless set */
	(void) XSetIOErrorHandler(io_err); /* io_err() mustn't & doesn't.... */
	Dsp = XOpenDisplay(disp);

	if (Dsp == (Display *)0) {
		report("Could not open display.  Can not continue.");
		for (i = 0; i < ntests; i++)
			tet_testlist[i].testfunc = aborttest;
		return;
	}

	/* Make sure that screen saver hasn't cut in */
	XResetScreenSaver(Dsp);

	/*
	 * Sync and clear out queue.
	 */
	XSync(Dsp, True);

}

/*
 * Cleanup function called at the end of the test purposes.
 */
void
protocleanup()
{
	if (Dsp) {
		/* At present this causes needless problems... */
#ifndef GENERATE_PIXMAPS
		/* about to exit anyway... */
		(void) close(ConnectionNumber(Dsp));
#endif
	}
}

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
 * $XConsortium: startcall.c,v 1.9 94/04/17 21:01:06 rws Exp $
 */

#include	"xtest.h"
#include	"stdio.h"
#include	"Xlib.h"
#include	"Xutil.h"
#include	"tet_api.h"
#include	"xtestlib.h"
#include	"pixval.h"

/*
 * This routine is similar to startcall except that no XSync() occurs.
 * Various tests assume that this routine merely sets up the error
 * handler stuff.  This routine should not generate protocol requests.
 */
void
_startcall(disp)
Display	*disp;
{
extern	int	error_status();
	/* Reset the error status */
	reseterr();

	/*
	 * Set error handler to trap errors that occur on this call
	 * Should be LAST.
	 */
	XSetErrorHandler(error_status);
}

/*
 * This routine contains setup procedures that should be called
 * before each time that the routine under test is called.
 */
void
startcall(disp)
Display	*disp;
{
	XSync(disp, True);

	_startcall(disp);
}

/*
 * This routine is similar to endcall() except that XSync() is
 * not called.
 * Various tests assume that this routine merely un-sets up the error
 * handler stuff.  This routine should not generate protocol requests.
 */
void
_endcall(disp)
Display	*disp;
{
extern	int 	unexp_err();

	/*
	 * Go back to the unexpected error handler.
	 */
	XSetErrorHandler(unexp_err);

	/* A debuging aid - pause for CR after displaying window */
	if (config.debug_pause_after) {
	int 	c;
	extern	int 	tet_thistest;

		printf("Test %d: Hit return to continue...", tet_thistest);
		fflush(stdout);
		while ((c = getchar()) != '\n' && c != EOF)
			;
	}
}

/*
 * The endcall() routine does all common cleanup code that
 * should be called after the routine under test has been
 * called.  XSync() needs to be called before this.
 */
void
endcall(disp)
Display	*disp;
{

	XSync(disp, False);

	_endcall(disp);
}

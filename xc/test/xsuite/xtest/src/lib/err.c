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
 * $XConsortium: err.c,v 1.13 94/04/17 21:00:44 rws Exp $
 */

#include	<unistd.h>
#include	"xtest.h"
#include	"tet_api.h"
#include 	"Xlib.h"
#include 	"Xutil.h"
#include	"xtestlib.h"
#include	"pixval.h"

static	int 	Error;	/* Place to save error code */
static	int	Resourceid; /* Place to save BadValues */

/*
 * This is the error handler that is used when the routine under
 * test is called.  The error type is saved in Error, for later
 * examination.
 */
int
error_status(disp, errevent)
Display	*disp;
XErrorEvent *errevent;
{

	/*
	 * The test for Success means that the first error will be recorded
	 * rather than the last.  This is usually the most intuitively
	 * related to the problem.  However there is no guarantee when
	 * multiple errors occur which will get reported and in what order.
	 * So all tests look for at most one error.
	 */
	if (Error == Success) {
		Error = errevent->error_code;
		Resourceid = errevent->resourceid;
	}
	trace("Received error type %s", errorname(errevent->error_code));
	trace("Request was %s", 
	    protoname (errevent->request_code | (errevent->minor_code << 8)));
	return(0);	/* This is not used (?) */

}

#define TEXTLEN	200

/*
 * This error handler deals with errors that occur when
 * routines other than the one under test are called.
 * It is always an error to get here.
 */
int
unexp_err(disp, errevent)
Display	*disp;
XErrorEvent *errevent;
{
char	text[TEXTLEN];

	XGetErrorText(disp, errevent->error_code, text, TEXTLEN);

	report("Unexpected error %s", errorname(errevent->error_code));
	report(text);
	report("Protocol request was %s",
	    protoname (errevent->request_code | (errevent->minor_code << 8)));

	/* Cause to test to not pass */
	delete("Unexpected Xlib error");
}

/*
 * Error handler that is called upon a fatal i/o error.
 * This is defined to return int even though it is not meant
 * to return.
 */
int
io_err(disp)
Display	*disp;
{
	delete("A fatal I/O error occurred");

	/* Attempt to prevent any more tests from running */
	tccabort("Any following results can not be relied upon");
	exit(TET_UNRESOLVED);
}

/*
 * Get the error code that was saved by a previous call to
 * error_status.
 */
int
geterr()
{
	return(Error);
}

/*
 * Get the resourceid that was saved by a previous call to
 * error_status. 
 */
int
getbadvalue()
{
	return(Resourceid);
}

/*
 * Reset Error to Success.
 */
void
reseterr()
{
	Error = Success;
}

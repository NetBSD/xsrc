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
 * $XConsortium: delete.c,v 1.6 94/04/17 21:00:41 rws Exp $
 */
/*LINTLIBRARY*/

#include	"stdio.h"
#include	"xtest.h"
#include	"tet_api.h"
#include	"Xlib.h"
#include	"Xutil.h"
#include	"xtestlib.h"
#include	"pixval.h"

#if TEST_ANSI
#include <stdarg.h>
#else
#include <varargs.h>
#endif

#define	LINELEN	1024

/*
 * Routines to control deleting of tests.
 */

extern	int 	tet_thistest;

static int 	TestDeleted = True;

/*
 * Call this routine to delete the current test.  If there is a message
 * it is output using report().  If not then a generic message is output
 * as a fail safe measure.
 */
/*VARARGS1*/

void
#if TEST_ANSI
delete(char *mess, ... )
#else
delete(mess, va_alist)
char	*mess;
va_dcl
#endif
{
char	buf[LINELEN];
va_list args;

#if TEST_ANSI
	va_start(args, mess);
#else
	va_start(args);
#endif


	if (mess && *mess) {
		(void) vsprintf(buf, mess, args);
		report(buf);
	} else
		report("Test deleted");

	/*
	 * Can not call tet_result from within a startup routine.
	 */
	if (tet_thistest)
		tet_result(TET_UNRESOLVED);
	TestDeleted = True;
}

/*
 * Return True if deleted, False if not.  All conections other than Dsp
 * should be sync'ed.
 */
int
isdeleted()
{
extern	Display	*Dsp;

	/*
	 * The default display is flushed so that errors (which are one of the
	 * main reasons that a test will be deleted unexpectedly) have a
	 * chance to be processed by the server.
	 */
	XSync(Dsp, False);
	return(TestDeleted);
}

/*
 * Reset the delete status back to false.
 */
void
resetdelete()
{
	TestDeleted = False;
}

/*
 * Cancel all remaining tests in this test case.  They will
 * not be initiated by the TCM.  The current test is not affected,
 * that should be marked as deleted with delete() if that is desired.
 * Can also be called from startup routines - none of the tests will be
 * run.
 */
void
cancelrest(reason)
char	*reason;
{
extern	int 	ntests;
int 	i;

	for (i = tet_thistest+1; i <= ntests; i++)
		tet_delete(i, reason);
}

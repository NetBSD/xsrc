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
 * $XConsortium: report.c,v 1.15 94/04/17 21:01:03 rws Exp $
 */
/*LINTLIBRARY*/
/*
 * Reporting functions these implement a higher level
 * on top of tet_infoline.
 * 
 *  REPORT -- report(char *fmt, ...)
 *    A description of something that went wrong.
 *  TRACE -- trace(char *fmt, ...)
 *    A 'I am here' or description of somthing that happened
 *    that is not an error in itself.
 *  DEBUG -- debug(char *fmt, ...)
 *    Debug lines -- Interface to be decided
 *  CHECK -- ???
 *    Path trace line.
 */

#include	"stdio.h"
#include	"xtest.h"
#include	"Xlib.h"
#include	"Xutil.h"
#include	"xtestlib.h"
#include	"tet_api.h"

#if TEST_ANSI
#include <stdarg.h>
#else
#include <varargs.h>
#endif

#define	LINELEN	1024

static int 	DebugLevel = 0;

extern	struct	config	config;

/*VARARGS1*/

void
#if TEST_ANSI
report(char *fmt, ...)
#else
report(fmt, va_alist)
char	*fmt;
va_dcl
#endif
{
char	buf[LINELEN];
va_list	args;

#if TEST_ANSI
	va_start(args, fmt);
#else
	va_start(args);
#endif

	(void) strcpy(buf, "REPORT:");
	(void) vsprintf(buf+strlen("REPORT:"), fmt, args);
	tet_infoline(buf);

	va_end(args);

}

/*VARARGS1*/

void
#if TEST_ANSI
trace(char *fmt, ...)
#else
trace(fmt, va_alist)
char	*fmt;
va_dcl
#endif
{
char	buf[LINELEN];
va_list	args;

	if (config.option_no_trace)
		return;

#if TEST_ANSI
	va_start(args, fmt);
#else
	va_start(args);
#endif

	(void) strcpy(buf, "TRACE:");
	(void) vsprintf(buf+strlen("TRACE:"), fmt, args);
	tet_infoline(buf);

	va_end(args);
}

/*VARARGS1*/

void
#if TEST_ANSI
check(char *fmt, ...)
#else
check(fmt, va_alist)
char	*fmt;
va_dcl
#endif
{
char	buf[LINELEN];
va_list	args;

	if (config.option_no_check)
		return;

#if TEST_ANSI
	va_start(args, fmt);
#else
	va_start(args);
#endif

	(void) strcpy(buf, "CHECK:");
	(void) vsprintf(buf+strlen("CHECK:"), fmt, args);
	tet_infoline(buf);

	va_end(args);
}

/*VARARGS2*/

void
#if TEST_ANSI
debug(int lev, char *fmt, ...)
#else
debug(lev, fmt, va_alist)
int 	lev;
char	*fmt;
va_dcl
#endif
{
char	buf[LINELEN];
va_list	args;

	if (lev > DebugLevel)
		return;

#if TEST_ANSI
	va_start(args, fmt);
#else
	va_start(args);
#endif

	(void) strcpy(buf, "DEBUG:");
	(void) vsprintf(buf+strlen("DEBUG:"), fmt, args);
	tet_infoline(buf);

	va_end(args);
}

/*
 * This formats its arguments as in report().  It also issues the result
 * code MIT_TET_ABORT.  This causes the TCM to give up, and also the TCC if
 * running under it.  It should be used when something is so badly wrong
 * that there is no point continuing any more tests.
 */

/*VARARGS1*/

void
#if TEST_ANSI
tccabort(char *fmt, ...)
#else
tccabort(fmt, va_alist)
char	*fmt;
va_dcl
#endif
{
char	buf[LINELEN];
va_list	args;

#if TEST_ANSI
	va_start(args, fmt);
#else
	va_start(args);
#endif

	(void) strcpy(buf, "REPORT:");
	(void) vsprintf(buf+strlen("REPORT:"), fmt, args);
	tet_infoline(buf);

	va_end(args);

	tet_result(MIT_TET_ABORT);

}

void
setdblev(n)
int 	n;
{
	DebugLevel = n;
}

int
getdblev()
{
	return DebugLevel;
}

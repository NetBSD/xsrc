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
 * $XConsortium: rpt.c,v 1.5 94/04/17 21:01:03 rws Exp $
 */
/*LINTLIBRARY*/

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

/*
 * Routines to handle reporting of test result codes and associated messages.
 * If there is a message it is output using report().  
 * If not then a generic message is output as a fail safe measure.
 */

/*
 * Call this routine to report the current test as unsupported.  
 */
/*VARARGS1*/

void
#if TEST_ANSI
unsupported(char *mess, ... )
#else
unsupported(mess, va_alist)
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
		report("Test unsupported");

	UNSUPPORTED;
}

/*
 * Call this routine to report the current test as notinuse.  
 */
/*VARARGS1*/

void
#if TEST_ANSI
notinuse(char *mess, ... )
#else
notinuse(mess, va_alist)
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
		report("Test not in use");

	NOTINUSE;
}

/*
 * Call this routine to report the current test as untested.  
 */
/*VARARGS1*/

void
#if TEST_ANSI
untested(char *mess, ... )
#else
untested(mess, va_alist)
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
		report("Test is untested");

	UNTESTED;
}

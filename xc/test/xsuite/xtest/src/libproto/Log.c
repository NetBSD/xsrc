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
 * $XConsortium: Log.c,v 1.8 94/04/17 21:01:14 rws Exp $
 */
/*
 * ***************************************************************************
 *  Copyright 1988 by Sequent Computer Systems, Inc., Portland, Oregon       *
 *                                                                           *
 *                                                                           *
 *                          All Rights Reserved                              *
 *                                                                           *
 *  Permission to use, copy, modify, and distribute this software and its    *
 *  documentation for any purpose and without fee is hereby granted,         *
 *  provided that the above copyright notice appears in all copies and that  *
 *  both that copyright notice and this permission notice appear in          *
 *  supporting documentation, and that the name of Sequent not be used       *
 *  in advertising or publicity pertaining to distribution or use of the     *
 *  software without specific, written prior permission.                     *
 *                                                                           *
 *  SEQUENT DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING *
 *  ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL *
 *  SEQUENT BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR  *
 *  ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,      *
 *  WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,   *
 *  ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS      *
 *  SOFTWARE.                                                                *
 * ***************************************************************************
 */

#include "XstlibInt.h"
#include "stdlib.h"
#include "tet_api.h"

extern char *TestName;

/*
 *  The routines dealing with log files won't return any error status, since
 *  it was their job to log errors in the first place.  In case of trouble,
 *  they will write on stderr.
 */

void
Log_Open () {
    /*
     * In the T7 test suite, this function really opened a log file.
     */
}


/*VARARGS1*/
void Log_Err (a, b, c, d, e, f, g, h, i, j, k)
char   *a, *b, *c, *d, *e, *f, *h, *i, *j, *k;
{
    ++Xst_error_count;
    Log_Msg (a, b, c, d, e, f, g, h, i, j, k);
}

/*VARARGS1*/
void Log_Err_Detail (a, b, c, d, e, f, g, h, i, j, k)
char   *a, *b, *c, *d, *e, *f, *h, *i, *j, *k;
{
    Log_Trace (a, b, c, d, e, f, g, h, i, j, k);
}

/*VARARGS1*/
void Log_Msg (a, b, c, d, e, f, g, h, i, j, k)
char   *a, *b, *c, *d, *e, *f, *h, *i, *j, *k;
{
    /* Use the TET reporting mechanism developed in the revised test suite */
    report(a, b, c, d, e, f, g, h, i, j, k);
}



/*VARARGS1*/
void Log_Trace (a, b, c, d, e, f, g, h, i, j, k)
char   *a, *b, *c, *d, *e, *f, *h, *i, *j, *k;
{
    /* Use the TET reporting mechanism developed in the revised test suite */
    trace(a, b, c, d, e, f, g, h, i, j, k);
}

/*VARARGS1*/
void Log_Del (a, b, c, d, e, f, g, h, i, j, k)
char   *a, *b, *c, *d, *e, *f, *h, *i, *j, *k;
{
    /* Use the TET reporting mechanism developed in the revised test suite */
    /* ++Xst_delete_count; @* incremented in the xproto delete() */
    delete(a, b, c, d, e, f, g, h, i, j, k);
}

/*VARARGS1*/
void Log_Debug (a, b, c, d, e, f, g, h, i, j, k)
char   *a, *b, *c, *d, *e, *f, *h, *i, *j, *k;
{
    /* Use the TET reporting mechanism developed in the revised test suite */
    debug(1, a, b, c, d, e, f, g, h, i, j, k);
}

/*VARARGS1*/
void Log_Debug2 (a, b, c, d, e, f, g, h, i, j, k)
char   *a, *b, *c, *d, *e, *f, *h, *i, *j, *k;
{
    /* Use the TET reporting mechanism developed in the revised test suite */
    debug(2, a, b, c, d, e, f, g, h, i, j, k);
}

/*VARARGS1*/
void Log_Debug3 (a, b, c, d, e, f, g, h, i, j, k)
char   *a, *b, *c, *d, *e, *f, *h, *i, *j, *k;
{
    /* Use the TET reporting mechanism developed in the revised test suite */
    debug(3, a, b, c, d, e, f, g, h, i, j, k);
}

/* support for trimming debug output if debuglevel is less than 3 */

static int some_counter = 0;
/* maximum number of lines for each variable length request */
#define SOME_LIMIT 25
/* debug level at which this maximum is ignored */
#define THRESHOLD_FOR_ALL 4

void
Reset_Some()
{
    some_counter = 0;
}

/*VARARGS1*/
void Log_Some (a, b, c, d, e, f, g, h, i, j, k)
char   *a, *b, *c, *d, *e, *f, *h, *i, *j, *k;
{
    /* Use the TET reporting mechanism developed in the revised test suite */
    if (++some_counter > SOME_LIMIT && getdblev() < THRESHOLD_FOR_ALL) {
	if (some_counter == (SOME_LIMIT + 1)) {
	    debug(2,"\t..... %d lines printing limit exceeded,\n", SOME_LIMIT);
	    debug(2,"\t\t(increase XT_DEBUG to %d for all lines.)\n", THRESHOLD_FOR_ALL);
	} else {
	    return;
	}
    } else
	debug(2, a, b, c, d, e, f, g, h, i, j, k);
}

int
Log_Close () 
{

    /* 
     * In the T7 test suite, this function really closed a log file.
     * Now this is done in the TET, but we can use this function as follows:
     * All test purposes call Log_Close when the test purpose is complete.
     * Therefore, this is the place where TET result codes can be assigned
     * in failure cases.
     */
    /* any messages hopefully already done, so just a summary */
    if(Xst_untested_count != 0) {
	report("Test %s untested with %d %s.", TestName, Xst_untested_count,
		(Xst_untested_count == 1) ? "reason" : "reasons");
	tet_result(TET_UNTESTED);
	return(EXIT_FAILURE);
    }
    if(Xst_delete_count != 0) {
	report("Test %s unresolved with %d %s.", TestName, Xst_delete_count,
		(Xst_delete_count == 1) ? "reason" : "reasons");
	tet_result(TET_UNRESOLVED);
	return(EXIT_FAILURE);
    }
    if(Xst_error_count != 0) {
	report("Test %s failed with %d %s.", TestName, Xst_error_count,
		(Xst_error_count == 1) ? "error" : "errors");
	tet_result(TET_FAIL);
	return(EXIT_FAILURE);
    }
    else
	return(EXIT_SUCCESS);
}

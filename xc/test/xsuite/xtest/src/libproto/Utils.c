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
 * $XConsortium: Utils.c,v 1.5 94/04/17 21:01:34 rws Exp $
 * $XFree86: xc/test/xsuite/xtest/src/libproto/Utils.c,v 3.1 1994/08/01 12:24:24 dawes Exp $
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

/* UNIX/DYNIX library for X Server tests.  
 *
 * Contains routines useful for isolating operating system dependencies.
 * DYNIX routines should be identical to UNIX in general.  VMS or other
 * operating systems will need an equivalent library.
 *
 */

#include "XstlibInt.h"
#include <signal.h>

char *  asctime();


/*
 * Return a character string with the current date and time.
 */

char *
Get_Date()
{
	struct  tm *tp;
	struct  tm *localtime();
	time_t tim;
	static char buf[26];

	tim = time(0);
	tp = localtime(&tim);
	(void) strcpy(buf, asctime(tp));
	buf[24] = '\0';
	return(buf);
}

#if !defined(__bsdi__) && !defined(__386BSD__) && !defined(__FreeBSD__) && !defined(__NetBSD__) && !defined(__linux__)
/*
 * These are routines found in BSD and not found in SYSV.
 */

void
bcopy (b1, b2, length)
register unsigned char *b1, *b2;
register length;
{
    if (b1 < b2) {
	b2 += length;
	b1 += length;
	while (length--) {
	    *--b2 = *--b1;
	}
    }
    else {
	while (length--) {
	    *b2++ = *b1++;
	}
    }
}

bcmp (b1, b2, length)
register unsigned char *b1, *b2;
register length;
{
    while (length--) {
	if (*b1++ != *b2++) return 1;
    }
    return 0;
}

void
bzero (b, length)
register unsigned char *b;
register length;
{
    while (length--) {
	*b++ = '\0';
    }
}

/* Find the first set bit
 * i.e. least signifigant 1 bit:
 * 0 => 0
 * 1 => 1
 * 2 => 2
 * 3 => 1
 * 4 => 3
 */

int
ffs(mask)
unsigned int	mask;
{
    register i;

    if ( ! mask ) return 0;
    i = 1;
    while (! (mask & 1)) {
	i++;
	mask = mask >> 1;
    }
    return i;
}
#endif

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

 * Copyright 1990, 1991 and UniSoft Group Limited.
 * 
 * Copyright 1993 by the Hewlett-Packard Company.
 *
 * Permission to use, copy, modify, distribute, and sell this software and
 * its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation, and that the names of HP, and UniSoft not be
 * used in advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.  HP, and UniSoft
 * make no representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 *
 * $XConsortium: RcvExtErr.c,v 1.4 94/04/17 21:01:18 rws Exp $
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

/*
 *	$Header: /cvsroot/xsrc/xc/test/xsuite/xtest/src/libproto/Attic/RcvExtErr.c,v 1.1.1.1 1997/03/15 06:19:55 scottr Exp $
 */

#ifndef lint
static char rcsid[]="$Header: /cvsroot/xsrc/xc/test/xsuite/xtest/src/libproto/Attic/RcvExtErr.c,v 1.1.1.1 1997/03/15 06:19:55 scottr Exp $";
#endif

#define ERROR_HEADER	4	/* size of constant header */
#define XInputNumErrors 5
#include "XstlibInt.h"

extern int XInputFirstError;

int
Rcv_Ext_Err(rp,rbuf,client)
xError *rp;
char rbuf[];
int client;
{
	int err;
	int needswap = Xst_clients[client].cl_swap;
	char *rbp = rbuf;
	int valid = 1;			/* assume all is OK */

	rbp += ERROR_HEADER;

	err = rp->errorCode - XInputFirstError;
	if (err>=0 && err < XInputNumErrors){
		switch (err) {
		case XI_BadDevice:
		case XI_BadMode:
		case XI_BadClass:
			((xError *)rp)->resourceID = unpack4(&rbp,needswap);
			((xError *)rp)->minorCode = unpack2(&rbp,needswap);
			((xError *)rp)->majorCode = unpack1(&rbp);
			break;
		default:
			DEFAULT_ERROR;
			break;
		}
	}
	else 	{
		DEFAULT_ERROR;
	}
	return valid;
}

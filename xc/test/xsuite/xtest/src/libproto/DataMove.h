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
 * $XConsortium: DataMove.h,v 1.4 94/04/17 21:01:12 rws Exp $
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
 *	Macros for byte swapping
 *
 *	Derived from X.V11R1 server code.
 *
 */

/*	swap, copy long to long	*/
#define swapcpl(src, dst) \
                 ((char *) &(dst))[0] = ((char *) &(src))[3];\
                 ((char *) &(dst))[1] = ((char *) &(src))[2];\
                 ((char *) &(dst))[2] = ((char *) &(src))[1];\
                 ((char *) &(dst))[3] = ((char *) &(src))[0]

/*	swap, copy short to short	*/
#define swapcps(src, dst)\
		 ((char *) &(dst))[0] = ((char *) &(src))[1];\
		 ((char *) &(dst))[1] = ((char *) &(src))[0]

/*	swap, copy longptr to longptr	*/
#define swapcplp(srcp, dstp) \
                 ((dstp))[0] = ((srcp))[3];\
                 ((dstp))[1] = ((srcp))[2];\
                 ((dstp))[2] = ((srcp))[1];\
                 ((dstp))[3] = ((srcp))[0]

/*	swap, copy shortptr to shortptr	*/
#define swapcpsp(srcp, dstp)\
		 ((dstp))[0] = ((srcp))[1];\
		 ((dstp))[1] = ((srcp))[0]

/*	swap, inplace long (using char temp)	*/
#define swapinl(x, n) n = ((char *) &(x))[0];\
		 ((char *) &(x))[0] = ((char *) &(x))[3];\
		 ((char *) &(x))[3] = n;\
		 n = ((char *) &(x))[1];\
		 ((char *) &(x))[1] = ((char *) &(x))[2];\
		 ((char *) &(x))[2] = n

/*	swap, inplace short (using char temp)	*/
#define swapins(x, n) n = ((char *) &(x))[0];\
		 ((char *) &(x))[0] = ((char *) &(x))[1];\
		 ((char *) &(x))[1] = n

/*	swap, inplace longptr (using char temp)	*/
#define swapinlp(x, n) n = ((x))[0];\
		 ((x))[0] = ((x))[3];\
		 ((x))[3] = n;\
		 n = ((x))[1];\
		 ((x))[1] = ((x))[2];\
		 ((x))[2] = n

/*	swap, inplace shortptr (using char temp)	*/
#define swapinsp(x, n) n = ((x))[0];\
		 ((x))[0] = ((x))[1];\
		 ((x))[1] = n

/*
 *	macros for packing/unpacking shorts/longs 
 *	these are here to centralize any issues with word alignment,
 *	byte addressing, etc.
 */

/*	nonswap, copy shortptr to shortptr	*/
#define nonswapcpsp(srcp, dstp)\
		 ((dstp))[1] = ((srcp))[1];\
		 ((dstp))[0] = ((srcp))[0]


/*	nonswap, copy longptr to longptr	*/
#define nonswapcplp(srcp, dstp) \
                 ((dstp))[3] = ((srcp))[3];\
                 ((dstp))[2] = ((srcp))[2];\
                 ((dstp))[1] = ((srcp))[1];\
                 ((dstp))[0] = ((srcp))[0]

/*
 *	pads a number of bytes up to next multiple of 4
 */
#define padup(n)	((((n)+3)>>2)<<2)

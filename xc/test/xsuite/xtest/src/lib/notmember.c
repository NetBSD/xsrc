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
 * $XConsortium: notmember.c,v 1.12 94/04/17 21:00:59 rws Exp $
 */

#include	"xtest.h"
#include	"Xlib.h"
#include	"Xutil.h"
#include	"xtestlib.h"
#include	"pixval.h"


static unsigned long masks[] = {
	/* order is important, from smallest to biggest */
	0x000000FF, 0x0000FFFF, 0xFFFFFFFF
};

static unsigned long biggest[] = {
	/* order is important, must match masks[] above */
	0x0000007F, 0x00007FFF, 0x7FFFFFFF
};

/*
 * Guess the largest value that will fit into the encoding based on the
 * maximum value of the parameter.
 */
static unsigned long
guess_largest(un, is_bit_set)
	unsigned long un;
	Bool is_bit_set;
{
	int i;

	for (i=0; i < NELEM(masks); i++)
		if (un < masks[i]) {
			if (is_bit_set)
				return masks[i];
			else
				return biggest[i];
		}

	/*
	 * if we got here we're in trouble, even 32-bits isn't enough
	 */
	delete("INTERNAL ERROR in guess largest with 0x%lx", un);
	return 0;
}

/*
 * Returns a list of numbers that are not members of given list.
 * The the returned values include:
 *   one greater than max in list.
 *   one less than min in list.
 *   a negative number not in list.
 *   a large number
 * 
 * An array of long size NM_LEN must be supplied for ret.
 * 
 * It is assumed that the numbers in the list are 'small' ie don't approach
 * LONG_MAX/MIN.
 *
 * Some attempt to infer possible encoding (1,2 or 4 bytes) from list of
 * values. Complain - and delete - if it looks as if something won't fit.
 * All done to avoid having unrepresentable values that get trimmed by the
 * encoding so they don't look bad after all.
 *
 * Negative numbers should be truncated down to some very large number
 * when the encoding is CARD*, so should be OK.
 */
int
notmember(list, len, ret)
int 	*list;
int 	len;
long	*ret;
{
long	*rp;
long 	min, max;
long	negative;
long 	large;
int 	i;

	max = min = list[0];
	negative = -1;
	large = 0;

	for (i = 0; i < len; i++) {
		if (list[i] < min)
			min = list[i];
		if (list[i] > max)
			max = list[i];
		if (min-1 <= negative)
			negative = min-2;
	}

	large = (long) guess_largest((unsigned long)max, False);

	rp = ret;
	*rp++ = max+1;	/* one greater than max in list */
	*rp++ = min-1;		/* one less than min in list */
	*rp++ = negative;	/* a negative number not in list */
	*rp++ = large;		/* a largish number */

	/*
	 * Sanity check to check that NM_LEN was OK.
	 */
	if (rp-ret > NM_LEN) {
		delete("INTERNAL error.  NM_LEN not large enough in notmember");
		return(NM_LEN);
	}

	return(rp-ret);

}

/*
 * Like notmember() except that it works with a list of bit positions
 * rather than integers.
 * Bits that do not occur in the list are filled into the return
 * array.
 */
int
notmaskmember(list, len, ret)
unsigned long	*list;
int 	len;
unsigned long	*ret;
{
unsigned long	bit;
unsigned long	large;
unsigned long	allbits;
int 	i;

	allbits = 0;

	for (i = 0; i < len; i++)
		allbits |= list[i];

	large = guess_largest(allbits, True);

	i = 0;
	for (bit = 1; bit && bit < large; bit <<= 1) {
		if (!(bit & allbits))
			ret[i++] = bit;
		if (i >= NM_LEN)
			break;
	}
	if (i == 0)
		delete("No spare bits in notmaskmember");
	return(i);
}

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
 * $XConsortium: bitcount.c,v 1.5 94/04/17 21:00:34 rws Exp $
 */
/*
 * Explanation:
 * First we add 32 1-bit fields to get 16 2-bit fields.
 * Each 2-bit field is one of 00, 01, or 10 (binary).
 * We then add all the two-bit fields to get 8 4-bit fields.
 * These are all one of 0000, 0001, 0010, 0011, or 0100.
 *
 * Now we can do something different, becuase for the first
 * time the value in each k-bit field (k now being 4) is small
 * enough that adding two k-bit fields results in a value that
 * still fits in the k-bit field.  The result is four 4-bit
 * fields containing one of {0000,0001,...,0111,1000} and four
 * more 4-bit fields containing junk (sums that are uninteresting).
 * Pictorially:
 *	    n = 0aaa0bbb0ccc0ddd0eee0fff0ggg0hhh
 *	 n>>4 = 00000aaa0bbb0ccc0ddd0eee0fff0ggg
 *	  sum = 0aaaWWWWiiiiXXXXjjjjYYYYkkkkZZZZ
 * where W, X, Y, and Z are the interesting sums (each at most 1000,
 * or 8 decimal).  Masking with 0x0f0f0f0f extracts these.
 *
 * Now we can change tactics yet again, because now we have:
 *	    n = 0000WWWW0000XXXX0000YYYY0000ZZZZ
 *	 n>>8 = 000000000000WWWW0000XXXX0000YYYY
 * so	  sum = 0000WWWW000ppppp000qqqqq000rrrrr
 * where p and r are the interesting sums (and each is at most
 * 10000, or 16 decimal).  The sum `q' is junk, like i, j, and
 * k above; but it is not necessarry to discard it this time.
 * One more fold, this time by sixteen bits, gives
 *	    n = 0000WWWW000ppppp000qqqqq000rrrrr
 *	n>>16 = 00000000000000000000WWWW000ppppp
 * so	  sum = 0000WWWW000ppppp000sssss00tttttt
 * where s is at most 11000 and t is it most 100000 (32 decimal).
 *
 * Now we have t = r+p = (Z+Y)+(X+W) = ((h+g)+(f+e))+((d+c)+(b+a)),
 * or in other words, t is the number of bits set in the original
 * 32-bit longword.  So all we have to do is return the low byte
 * (or low 6 bits, but `low byte' is typically just as easy if not
 * easier).
 *
 * This technique is also applicable to 64 and 128 bit words, but
 * 256 bit or larger word sizes require at least one more masking
 * step.
 *
 * Author: Gene Olsen
 */
unsigned int
bitcount(n)
	register unsigned long n;
{

	n = (n & 0x55555555) + ((n >> 1) & 0x55555555);
	n = (n & 0x33333333) + ((n >> 2) & 0x33333333);
	n = (n + (n >> 4)) & 0x0f0f0f0f;
	n += n >> 8;
	n += n >> 16;
	return ((unsigned int) (n & 0xff));
}

/*
 * Given a pixel value and a mask return a word with masked bits
 * all squeezed into bottom bits of word (i.e. (0xF1, 0x78) -> 0x0E ).
 * Used for checking reults of getting images/subimages with plane masks.
 */
unsigned long
getpix(m, p)
unsigned long	m;
unsigned long	p;
{
	unsigned long	pix = 0;
	unsigned long	bit = 1;
	unsigned long	nbit;
	unsigned long	bb = 1;

	while (bit) {
		if (p & bit) {
			if (m & bit)
				pix |= bb;
			bb <<= 1;
		}
		nbit = bit << 1;
		if (nbit == bit) /* don't trust some compilers */
			break;
		bit = nbit;
	}
	debug(1, "getpix: Pixel is %lx from %lx and %lx.", pix, m, p);
	return pix;
}

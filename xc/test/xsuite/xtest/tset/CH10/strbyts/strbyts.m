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
 * $XConsortium: strbyts.m,v 1.8 94/04/17 21:10:34 rws Exp $
 */
>>TITLE XStoreBytes CH10

XStoreBytes(display, bytes, nbytes)
Display	*display = Dsp;
char	*bytes = buff;
int	nbytes = NELEM(buff);
>>EXTERN
static char	buff[1217];
>>ASSERTION Good A
A call to xname stores
.A nbytes
bytes from the
.A bytes
argument into cut buffer zero.
>>STRATEGY
Store bytes in cut buffer 0 using xname.
Obtain the contents of cut buffer 0 using XFetchBuffer.
Verify that the buffer contents is correct.
>>CODE
char	*bp;
char	*bpr = (char *) NULL;
int	i;
int	br;

	for(i=NELEM(buff), bp = buff; i>0; *bp++ = (char) (i & 255), i--);

	XCALL;

	bp = XFetchBuffer(display, &br, 0);

	if(bp == (char *) NULL) {
		report("%s() did not set buffer 0 to contain any data.", TestName);
		FAIL;
	} else {
		CHECK;

		if(br != NELEM(buff)) {
			report("%s() set buffer 0 to contain %d bytes instead of %d.", TestName, br, NELEM(buff));
			FAIL;
		} else {
			CHECK;

			if(memcmp(bp, buff, NELEM(buff)) != 0) {
				report("%s() set buffer 0 to contain the wrong data.", TestName);
				FAIL;
			} else
				CHECK;
		}

		XFree(bp);
	}

	CHECKPASS(3);

>>ASSERTION Bad B 1
.ER BadAlloc 

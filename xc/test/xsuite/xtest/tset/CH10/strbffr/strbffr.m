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
 * $XConsortium: strbffr.m,v 1.9 94/04/17 21:10:33 rws Exp $
 */
>>TITLE XStoreBuffer CH10

XStoreBuffer(display, bytes, nbytes, buffer)
Display		*display = Dsp;
char		*bytes;
int		nbytes;
int		buffer;
>>ASSERTION Good A
A call to xname stores
.A nbytes
bytes from the
.A bytes
argument into the cut buffer specified by the
.A buffer
argument.
>>STRATEGY
For each cut buffer 0 to 7:
   Store different data into each buffer using xname.
For each cut buffer 0 to 7:
   Obtain the contents of the buffer using XFetchBuffer.
   Verify that the data is correct.
>>CODE
char	*bp;
char	*rbp;
int	nr;
int	len;
int 	i;
int 	j;
struct	bstrct {
	char	*data;
	int	len;
}	bfrs[8], *bptr;

	for(i=0, bptr=bfrs; i<8; i++, bptr++) {

		len = 1+i*123;
		bptr->len = len;

		if((bptr->data = (char*)malloc(len)) == (char *) NULL) {
			delete("malloc() returned NULL.");
			return;
		} else
			CHECK;

		for(j=len, bp=bptr->data; j>0; *bp++ = (j) %  (256 - i), j--);

		bytes = bptr->data;
		nbytes = len;
		buffer = i;
		XCALL;
	}

	for(i=0, bptr=bfrs; i<8; i++, bptr++) {
		
		rbp = XFetchBuffer(display, &nr, i);

		if(rbp == (char *) NULL) {
			report("%s() did not set buffer %d to contain any data.", TestName, i);
			FAIL;
		} else {
			CHECK;

			if(bptr->len != nr) {
				report("%s() set buffer %d to contain %d bytes instead of %d.", TestName, i, nr, bptr->len);
				FAIL;
			} else {
				CHECK;

				if(memcmp(rbp, bptr->data, nr) != 0) {
					report("%s() set buffer %d to contain the wrong data.", TestName, i);
					FAIL;
				} else
					CHECK;
			}

			free(bptr->data);
			XFree(rbp);
		}
	}


	CHECKPASS(8 + 8 * (3));

>>ASSERTION Bad B 1
.ER BadAlloc 

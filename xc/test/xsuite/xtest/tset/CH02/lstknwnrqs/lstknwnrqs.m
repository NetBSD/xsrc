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
 * $XConsortium: lstknwnrqs.m,v 1.6 94/04/17 21:02:29 rws Exp $
 */
>>SET   macro
>>TITLE XLastKnownRequestProcessed CH02
unsigned long
XLastKnownRequestProcessed(display)
Display	*display = Dsp;
>>ASSERTION Good B 1
A call to xname returns the serial number of the last request known to have
been processed by the server over the connection specified by the
.A display
argument .
>>STRATEGY
Obtain the serial number of the last request processed by the server using xname.
Obtain the serial number of the next request to be sent using XNextRequest.
Verify that the two serial numbers are not the same.
>>CODE
unsigned long	sno, nno;

	sno = XCALL;
	nno = XNextRequest(display);
	report("Last processed serial number was %lu, next serial number is %lu.", sno, nno);
	if(sno == nno) {
		report("The next serial number (%lu) was not different to the current serial number.", nno);
		FAIL;
	} else
		CHECK;

	CHECKUNTESTED(1);

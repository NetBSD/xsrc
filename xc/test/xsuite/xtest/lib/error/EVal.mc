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
 * $XConsortium: EVal.mc,v 1.6 94/04/17 21:14:59 rws Exp $
 */
>>### Value
>>STRATEGY
Obtain a sequence of values which are not in the list specified by VALUE_LIST.
For each value:
  Call test function with this value in the VALUE_ARG argument.
  Verify that a BadValue error occurs.
>>CODE BadValue
int 	i;
int 	n;
NOTMEMTYPE
long	vals[NM_LEN];

	seterrdef();

	n = NOTMEMBER(VALUE_LIST, NELEM(VALUE_LIST), vals);

	for (i = 0; i < n; i++) {

		debug(1, "Trying arg of %d", vals[i]);

		VALUE_ARG = vals[i];
		XCALL;

		if (geterr() == BadValue)
			CHECK;
		else {
			trace("Value of %d did not give BadValue", vals[i]);
			FAIL;
		}
	}

	CHECKPASS(n);

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
 * $XConsortium: rmstrtqrk.m,v 1.5 94/04/17 21:10:25 rws Exp $
 */
>>TITLE XrmStringToQuark CH10
XrmQuark

char *string;
>>SET startup rmstartup
>>ASSERTION Good A
A call to xname returns a quark allocated to represent
.A string .
>>STRATEGY
Call xname to allocate a quark for a string.
Call XrmQuarkToString to obtain representation for the quark.
Verify the quark represents the string.
>>CODE
char *s = "stq_one";
XrmQuark ret;
char *rep;

/* Call xname to allocate a quark for a string. */
	string = s;
	ret = XCALL;

/* Call XrmQuarkToString to obtain representation for the quark. */
	rep = XrmQuarkToString(ret);

#ifdef TESTING
	rep = "barfed";
#endif

/* Verify the quark represents the string. */
	if((rep==(char *)NULL) || strcmp(s,rep)) {
		FAIL;
		report("%s did not allocate a quark representing the string",
			TestName);
		report("Returned quark was: %d", (int)ret);
		report("Expected representation: %s", s);
		report("Returned representation: %s",
			(rep==(char *)NULL?"NULL pointer":rep));
	} else
		CHECK;

	CHECKPASS(1);

>>ASSERTION Good A
When a quark already exists for
.A string ,
then a call to xname returns that quark.
>>STRATEGY
Call xname to allocate a quark for a string.
Call xname to allocate a quark for the string again.
Verify that the quarks were the same.
>>CODE
char *s = "stq_two";
XrmQuark ret1, ret2;

/* Call xname to allocate a quark for a string. */
	string = s;
	ret1 = XCALL;

/* Call xname to allocate a quark for the string again. */
	ret2 = XCALL;

/* Verify that the quarks were the same. */
	if (ret1 != ret2) {
		FAIL;
		report("%s did not return the same quark to represent",
			TestName);
		report("the same string.");
		report("1st quark return: %d", (int) ret1);
		report("2nd quark return: %d", (int) ret2);
	} else
		CHECK;

	CHECKPASS(1);

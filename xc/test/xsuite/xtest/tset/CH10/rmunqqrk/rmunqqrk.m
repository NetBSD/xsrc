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
 * $XConsortium: rmunqqrk.m,v 1.6 94/04/17 21:10:27 rws Exp $
 */
>>TITLE XrmUniqueQuark CH10
XrmQuark

>>SET startup rmstartup
>>ASSERTION Good A
A call to xname returns a quark that does not represent any string
known to the resource manager.
>>STRATEGY
Call xname to obtain a unique quark.
Verify the quark does not represent a string in the resource manager.
Call xname to obtain another quark.
Verify this is a distinct quark.
>>CODE
XrmQuark ret1, ret2;
char *str;

/* Call xname to obtain a unique quark. */
	ret1 = XCALL;

/* Verify the quark does not represent a string in the resource manager. */
	str = XrmQuarkToString( ret1 );
	if (str != (char *)NULL) {
		FAIL;
		report("%s did not return a quark not representing a string.",
			TestName);
		report("XrmQuarkToString Expected: NULL pointer");
		report("XrmQuarkToString Returned: '%s'", str);
	} else
		CHECK;

/* Call xname to obtain another quark. */
	ret2 = XCALL;

/* Verify this is a distinct quark. */
	if (ret1 == ret2) {
		FAIL;
		report("%s returned indistinct quarks on consecutive calls.",
			TestName);
		report("1st quark: %d", (int)ret1);
		report("2nd quark: %d", (int)ret2);
	} else
		CHECK;

	CHECKPASS(2);

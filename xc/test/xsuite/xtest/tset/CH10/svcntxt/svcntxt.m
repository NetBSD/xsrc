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
 * $XConsortium: svcntxt.m,v 1.5 94/04/17 21:10:36 rws Exp $
 */
>>TITLE XSaveContext CH10
int

Display *display = Dsp;
Window w = defwin(display);
XContext context = XUniqueContext();
caddr_t data;
>>EXTERN

static char *xsc_ctxt ="set context";
static char *xsc_ctxt2 ="set context two";

>>ASSERTION Good A
A call to xname sets the table entry for context data
for display 
.A display ,
window
.A w
and context type
.A context 
to the specified value
.A data ,
and returns zero.
>>STRATEGY
Call xname to enter the context data.
Verify that zero was returned.
Call XFindContext to verify that the context data was added correctly.
>>CODE
int a;
int ret;
caddr_t b;

/* Call xname to enter the context data. */
	data = (caddr_t) xsc_ctxt;
	ret = XCALL;

/* Verify that zero was returned. */
	if (ret != 0) {
		FAIL;
		report("%s returned non-zero when expected zero.",
			TestName);
		report("Returned value: %s", contexterrorname(ret));
	} else
		CHECK;

/* Call XFindContext to verify that the context data was added correctly. */
	a = XFindContext(display, w, context, &b);
	if (a != 0) {
		FAIL;
		report("XFindContext failed to find the context saved by %s", TestName);
		report("XFindContext returned %s", contexterrorname(a));
	} else {
		CHECK;

		if (b != (caddr_t)xsc_ctxt) {
			FAIL;
			report("XFindContext returned an unexpected context.");
			report("Expected context: %0x", (unsigned int)xsc_ctxt);
			report("Returned context: %0x", (unsigned int)b);
		} else
			CHECK;
	}

	CHECKPASS(3);

>>ASSERTION Good A
When there is previously saved context data
for display
.A display ,
window
.A w
and context type
.A context ,
then a call to xname replaces the previously saved context data
with the specified value
.A data ,
and returns zero.
>>STRATEGY
Call xname to set the context data.
Verify that zero was returned.
Call xname to reset the context data.
Verify that zero was returned.
Call XFindContext to verify that the context data was added correctly.
>>CODE
int a;
int ret;
caddr_t b;

/* Call xname to set the context data. */
	data = (caddr_t) xsc_ctxt;
	ret = XCALL;

/* Verify that zero was returned. */
	if (ret != 0) {
		FAIL;
		report("%s returned non-zero when expected zero when setting",
			TestName);
		report("context information.");
		report("Returned value: %s", contexterrorname(ret));
	} else
		CHECK;

/* Call xname to reset the context data. */
	data = (caddr_t) xsc_ctxt2;
	ret = XCALL;

/* Verify that zero was returned. */
	if (ret != 0) {
		FAIL;
		report("%s returned non-zero when expected zero when resetting",
			TestName);
		report("context information.");
		report("Returned value: %s", contexterrorname(ret));
	} else
		CHECK;

/* Call XFindContext to verify that the context data was added correctly. */
	a = XFindContext(display, w, context, &b);
	if (a != 0) {
		FAIL;
		report("XFindContext failed to find the context saved by %s", TestName);
		report("XFindContext returned %s", contexterrorname(a));
	} else {
		CHECK;

		if (b != (caddr_t)xsc_ctxt2) {
			FAIL;
			report("XFindContext returned an unexpected context.");
			report("Original context: %0x", (unsigned int)xsc_ctxt);
			report("Expected context: %0x", (unsigned int)xsc_ctxt2);
			report("Returned context: %0x", (unsigned int)b);
		} else
			CHECK;
	}

	CHECKPASS(4);
>>ASSERTION Bad B 1
When there is insufficient memory, then a call to xname returns
.S XCNOMEM .

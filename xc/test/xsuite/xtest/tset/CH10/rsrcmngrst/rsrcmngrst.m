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
 * $XConsortium: rsrcmngrst.m,v 1.5 94/04/17 21:10:28 rws Exp $
 */
>>TITLE XResourceManagerString CH10
char *
XResourceManagerString(display)
Display	*display = Dsp;
>>EXTERN
#include	"Xatom.h"
>>ASSERTION Good A
A call to xname returns the value of the RESOURCE_MANAGER
property on the root window of screen zero at the time
.A display
was opened.
>>STRATEGY
Set the value of the RESOURCE_MANAGER property to "XTest.test.resource:value" using XChangeProperty.
Open display using XOpenDisplay.
Set the value of the RESOURCE_MANAGER property to "XTest.changed.resource:value" using XChangeProperty.
Obtain the value of the RESOURCE_MANAGER property at the time display was opened using xname.
Verify that the call returned "XTest.test.resource:value".
>>CODE
char	*pval1 = "XTest.test.resource:value";
char	*pval2 = "XTest.changed.resource:value";
char	*res;

	XChangeProperty (Dsp, RootWindow(Dsp, 0), XA_RESOURCE_MANAGER, XA_STRING, 8, PropModeReplace, (unsigned char *)pval1, 1+strlen(pval1));
	XSync(Dsp, False);
	display = opendisplay();
	XChangeProperty (display, RootWindow(display, 0), XA_RESOURCE_MANAGER, XA_STRING, 8, PropModeReplace, (unsigned char *)pval2, 1+strlen(pval2));

	res = XCALL;

	if( res == (char *) NULL) {
		report("%s() returned NULL.", TestName);
		FAIL;
	} else {
		CHECK;
		if(strcmp(res, pval1) != 0) {
			report("%s() returned \"%s\" instead of \"%s\".", TestName, res, pval1);
			FAIL;
		} else
			CHECK;
	}

	CHECKPASS(2);

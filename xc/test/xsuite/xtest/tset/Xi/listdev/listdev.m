/*
 * Copyright 1993 by the Hewlett-Packard Company.
 *
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

 * Copyright 1990, 1991 UniSoft Group Limited.
 *
 * Permission to use, copy, modify, distribute, and sell this software and
 * its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation, and that the names of HP, and UniSoft not be
 * used in advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.  HP, and UniSoft
 * make no representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 *
 * $XConsortium: listdev.m,v 1.8 94/09/06 20:50:52 dpw Exp $
 */

>>TITLE XListInputDevices XINPUT
XDeviceInfo *

Display	*display = Dsp;
int *ndevices_return = &ndevices;
>>EXTERN
int ndevices;

>>ASSERTION Good B 3
A successful call to xname lists all the input devices and returns
a pointer to a list of DeviceInfo structures.
>>STRATEGY
Call xname.
>>CODE
XDeviceInfo *list;
int ximajor, first, err;

	if (!XQueryExtension (display, INAME, &ximajor, &first, &err))
	    {
	    untested("%s: Input extension not supported.\n", TestName);
	    return;
	    }

	list = XCALL;
	if (list==NULL)
	    FAIL;
	else
	    PASS;

>>ASSERTION Good B 3
A successful call to ListInputDevices returns the number of
input devices in the ndevices_return parameter.
>>STRATEGY
Call xname.
>>CODE
XDeviceInfo *list;
int ximajor, first, err;

	if (!XQueryExtension (display, INAME, &ximajor, &first, &err))
	    {
	    untested("%s: Input extension not supported.\n", TestName);
	    return;
	    }

	list = XCALL;
	if (list==NULL)
	    FAIL;
	else
	    {
	    CHECK;
	    if (ndevices >= 2)
		CHECK;
	    else
		{
		report("%s: failed to get DeviceInfo information for at least the core input devices",TestName);
		FAIL;
		}
	    }
	CHECKPASS(2);


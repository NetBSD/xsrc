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
 * $XConsortium: opendev.m,v 1.8 94/09/06 20:51:20 dpw Exp $
 */

>>TITLE XOpenDevice XINPUT
XDevice *

Display	*display = Dsp;
XID deviceid;
>>EXTERN
extern ExtDeviceInfo Devs;

>>ASSERTION Good B 3
A successful call to xname opens the specified input device and returns
a pointer to a Device structure.
>>STRATEGY
Call xname.
>>CODE
XDevice *dev;

	if (!Setup_Extension_DeviceInfo(KeyMask))
	    {
	    untested("%s: No input extension key device.\n", TestName);
	    return;
	    }
	deviceid = Devs.Key->device_id;
	dev = XCALL;
	if (geterr()!=Success)
	    FAIL;
	else
	    PASS;

>>ASSERTION Good B 3
An input device may be opened by xname more than once.
>>STRATEGY
Call xname.
>>CODE
XDevice *dev;
int i;

	if (!Setup_Extension_DeviceInfo(KeyMask))
	    {
	    untested("%s: No input extension key device.\n", TestName);
	    return;
	    }
	deviceid = Devs.Key->device_id;
	for (i=0; i<257; i++)
	    {
	    dev = XCALL;
	    if (geterr()!=Success)
		FAIL;
	    else
		CHECK;
	    }
	CHECKPASS(257);

>>ASSERTION Bad B 3
A call to xname with an invalid deviceid returns a BadDevice error.
>>STRATEGY
Call xname.
>>CODE baddevice
XDevice *dev;
XID	baddevice;
int ximajor, first, err;

	if (!XQueryExtension (display, INAME, &ximajor, &first, &err))
	    {
	    untested("%s: Input extension not supported.\n", TestName);
	    return;
	    }
	BadDevice(display, baddevice);
	deviceid = -1;
	dev = XCALL;
	if (geterr()!=baddevice)
	    FAIL;
	else
	    PASS;

>>ASSERTION Bad B 3
A call to xname with the deviceid of the X keyboard or pointer
returns a BadDevice error.
>>STRATEGY
Call xname.
>>CODE baddevice
int 	i, ndevs;
XID	baddevice;
XDeviceInfo *list;
XDevice *ret;

	if (!Setup_Extension_DeviceInfo(KeyMask))
	    {
	    untested("%s: No input extension key device.\n", TestName);
	    return;
	    }
	BadDevice(display, baddevice);
	list = XListInputDevices (display, &ndevs);
	for (i=0; i<ndevs; i++,list++)
	    if (list->use == IsXKeyboard)
		{
		deviceid = list->id;
		break;
		}

	ret = XCALL;
	if (geterr()!=baddevice)
	    FAIL;
	else
	    PASS;


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
 * Copyright 1993 by the Hewlett-Packard Company.
 * Copyright 1990, 1991 by UniSoft Group Limited.
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
 * $XConsortium: getfctl.m,v 1.7 94/09/06 20:54:12 dpw Exp $
 */
>>TITLE XGetFeedbackControl XINPUT
XFeedbackState *
xname()
Display	*display = Dsp;
XDevice	*device;
int	*num_feedbacks_return = &Nfeed;
>>EXTERN
extern ExtDeviceInfo Devs;
int Nfeed;

>>ASSERTION Good B 3
A call to xname
returns in Nfeed the number of feedbacks supported by the device.
>>STRATEGY
Call xname to get the feedbacks supported by this device.
UNTESTED touch test only.
>>CODE
int i, j, ndevices;
XDeviceInfoPtr list;
XInputClassInfo *ip;
XFeedbackState *state;

	if (!Setup_Extension_DeviceInfo(KeyMask))
	    {
	    untested("%s: No input extension key device.\n", TestName);
	    return;
	    }
	list = XListInputDevices (display, &ndevices);
	for (i=0; i<ndevices; i++,list++)
	    if (list->use == IsXExtensionDevice)
		{
		device = XOpenDevice (display, list->id);
		for (j=0, ip=device->classes; j<device->num_classes; j++,ip++)
		    if (ip->input_class == FeedbackClass)
			{
			state = XCALL;
			trace("Number of feedbacks reported as %d", Nfeed);
			}
		}

	report("There is no reliable test method, but a touch test was performed");

	UNTESTED;
>>ASSERTION Bad B 3
A call to xname
returns a BadDevice error if an invalid device is specified.
>>CODE baddevice
XID 	baddevice;
XDevice bogus;
XFeedbackState *state;
int ximajor, first, err;

	if (!XQueryExtension (display, INAME, &ximajor, &first, &err))
	    {
	    untested("%s: Input extension not supported.\n", TestName);
	    return;
	    }

	BadDevice(display,baddevice);
	bogus.device_id = -1;
	device = &bogus;
	state = XCALL;

	if (geterr() == baddevice)
		CHECK;
	else {
		report("No BadDevice for invalid device");
		FAIL;
		}
	CHECKPASS(1);
>>ASSERTION Bad B 3
A call to xname
returns a BadMatch error if the device has no feedbacks.
>>CODE BadMatch
XFeedbackState *state;

	if (!Setup_Extension_DeviceInfo(NFeedMask))
	    {
	    untested("%s: No input extension device without feedbacks.\n", TestName);
	    return;
	    }
	device = Devs.NoFeedback;
	state = XCALL;

	if (geterr() == BadMatch)
		PASS;
	else {
		report("No BadMatch error for device with no feedbacks");
		FAIL;
		}

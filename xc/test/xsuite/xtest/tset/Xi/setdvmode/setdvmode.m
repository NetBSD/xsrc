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

 * Copyright 1993 by the Hewlett-Packard Company.
 *
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
 * $XConsortium: setdvmode.m,v 1.7 94/09/06 21:09:59 dpw Exp $
 */
>>TITLE XSetDeviceMode XINPUT
void

Display	*display = Dsp;
XDevice *device;
int mode = Absolute;
>>EXTERN
extern ExtDeviceInfo Devs;

>>ASSERTION Good B 3
A call to xname will change the mode of a device from Absolute to 
Relative and vice versa.
>>STRATEGY
Make the call with an valid device.
>>CODE 
int i, ret;
XDeviceState *state;
XInputClass *cp;

	if (!Setup_Extension_DeviceInfo(DModMask))
	    {
	    untested("%s: Required input extension device not present.\n", TestName);
	    return;
	    }

	device = Devs.DvMod;
	ret = XCALL;
	state = XQueryDeviceState(display, device);
	cp = state->data;
	for (i=0; i<state->num_classes; i++)
	    {
	    if (cp->class == ValuatorClass)
		if (((XValuatorState *)cp)->mode == Absolute)
		    CHECK;
		else
		    {
		    report("%s: Couldn't set Absolute mode\n",TestName);
		    FAIL;
		    }
	    cp = (XInputClass *) ((char *) cp + cp->length);
	    }

	mode = Relative;
	ret = XCALL;
	state = XQueryDeviceState(display, device);
	cp = state->data;
	for (i=0; i<state->num_classes; i++)
	    {
	    if (cp->class == ValuatorClass)
		if (((XValuatorState *)cp)->mode == Relative)
		    CHECK;
		else
		    {
		    report("%s: Couldn't set Relative mode\n",TestName);
		    FAIL;
		    }
	    cp = (XInputClass *) ((char *) cp + cp->length);
	    }
	CHECKPASS(2);

>>ASSERTION Good B 3
A call to xname will return a status of AlreadyGrabbed if a another
client has the device grabbed.
>>STRATEGY
Grab the device from another client.
Make the call with an valid device.
>>CODE 
int ret;
Display	*client2;
Window grab_window;

	if (!Setup_Extension_DeviceInfo(DModMask))
	    {
	    untested("%s: Required input extension device not present.\n", TestName);
	    return;
	    }
	device = Devs.DvMod;
        grab_window = defwin(Dsp);

	XGrabDevice(Dsp, Devs.DvMod, grab_window, True, 0, 
		NULL, GrabModeAsync, GrabModeAsync, CurrentTime);
	if (isdeleted()) {
		delete("Could not set up initial grab");
		return;
	}

	if ((client2 = opendisplay()) == NULL)
		return;

	display = client2;
	ret = XCALL;

	if (ret == AlreadyGrabbed)
		PASS;
	else
		FAIL;

>>ASSERTION Bad D 1
A call to SetDeviceMode will fail with a BadMatch  error  if
a device is specified that does not support this request.

>>ASSERTION Bad D 1
A call to SetDeviceMode will fail with a DeviceBusy status if
another client already has the device open with a mode other
than the one specified.

>>ASSERTION Bad B 3
A call to xname will fail with a BadMatch error if an valid device
that has no valuators is specified.
>>STRATEGY
Make the call with an valid device that has no valuators.
>>CODE BadMatch

	if (!Setup_Extension_DeviceInfo(NValsMask))
	    {
	    untested("%s: No input extension device without valuators.\n", TestName);
	    return;
	    }
	device = Devs.NoValuators;
	XCALL;

	if (geterr() == BadMatch)
		PASS;
	else
		FAIL;
>>ASSERTION Bad B 3
A call to xname will fail with a BadDevice error if an invalid device
is specified.
>>STRATEGY
Make the call with an invalid device.
>>CODE baddevice
XDevice nodevice;
XID baddevice;
int ximajor, first, err;

	if (!XQueryExtension (display, INAME, &ximajor, &first, &err)) {
	    untested("%s: Input extension not supported.\n", TestName);
	    return;
	    }

	BadDevice (display, baddevice);
	nodevice.device_id = -1;
	device = &nodevice;

	XCALL;

	if (geterr() == baddevice)
		PASS;
	else
		FAIL;

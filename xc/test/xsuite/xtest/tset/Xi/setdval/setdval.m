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
 * $XConsortium: setdval.m,v 1.7 94/09/06 21:09:15 dpw Exp $
 */
>>TITLE XSetDeviceValuators XINPUT
void

Display	*display = Dsp;
XDevice *device;
int *valuators = tvals;
int first_valuator=0;
int num_valuators=1;
>>EXTERN
extern ExtDeviceInfo Devs;
extern int NumValuators;
int tvals[256];

>>ASSERTION Good B 3
A call to xname will change the value of the valuators on the device.
>>STRATEGY
Make the call with a valid device.
>>CODE 
int i, j, k, l, ret, ndevices, count=1;
XDeviceState *state;
XValuatorState *vs;
XDeviceInfo *list;
XAnyClassPtr any;
XAxisInfoPtr a=NULL;

	if (!Setup_Extension_DeviceInfo(DValMask))
	    {
	    untested("%s: Required input extension device not present.\n", TestName);
	    return;
	    }
	device = Devs.DvVal;
	XSetDeviceMode(display, Devs.DvVal, Absolute);
	state = XQueryDeviceState(display, device);
	vs = (XValuatorState *) state->data;
	for (i=0; i<state->num_classes; i++)
	    {
	    if (vs->class == ValuatorClass)
		if (vs->mode == Absolute)
		    CHECK;
		else
		    {
		    report("%s: Couldn't set Absolute mode\n",TestName);
		    FAIL;
		    }
	    vs = (XValuatorState *) ((char *) vs + vs->length);
	    }

	list = XListInputDevices(display, &ndevices);
	for (i=0,num_valuators=0; i<ndevices; i++,list++)
	    {
	    if (list->id!=Devs.DvVal->device_id)
		continue;
	    any = (XAnyClassPtr) (list->inputclassinfo);
	    for (j=0,l=0; j<list->num_classes; j++)
		{
		if (any->class == ValuatorClass)
		    {
		    a = ((XValuatorInfo *) any)->axes;
		    num_valuators += ((XValuatorInfo *) any)->num_axes;
		    for (k=0; k<((XValuatorInfo *)any)->num_axes; k++)
			tvals[l++] = (a+k)->max_value;
		    }
	        any = (XAnyClassPtr) ((char *) any + any->length);
		}
	    }
	ret = XCALL;
	state = XQueryDeviceState(display, device);
	vs = (XValuatorState *) state->data;
	for (i=0; i<state->num_classes; i++)
	    {
	    if (vs->class == ValuatorClass)
		{
		for (j=0; j<vs->num_valuators; j++)
		    if (*(vs->valuators + j) == tvals[j])
			{
			CHECK;
			count++;
			}
		    else
			{
			report("%s: Wrong value for valuator %d, got %d wanted %d\n",TestName,j,*(vs->valuators+j),tvals[j]);
			FAIL;
			}
		break;
		}
	    vs = (XValuatorState *) ((char *) vs + vs->length);
	    }
	CHECKPASS(count);

>>ASSERTION Good B 3
A call to SetDeviceValuators will not affect the value of
valuators outside the specified range.
>>STRATEGY
Call xname to set one of the valuators.
Verify that the others are unchanged.
>>CODE 
int i, j, k, l, ret, ndevices, count=1;
XDeviceState *state;
XValuatorState *vs;
XDeviceInfo *list;
XAnyClassPtr any;
XAxisInfoPtr a=NULL;

	if (!Setup_Extension_DeviceInfo(DValMask))
	    {
	    untested("%s: Required input extension device not present.\n", TestName);
	    return;
	    }
	device = Devs.DvVal;
	num_valuators=0;
	XSetDeviceMode(display, Devs.DvVal, Absolute);
	state = XQueryDeviceState(display, device);
	vs = (XValuatorState *) state->data;
	for (i=0; i<state->num_classes; i++)
	    {
	    if (vs->class == ValuatorClass)
		if (vs->mode == Absolute)
		    CHECK;
		else
		    {
		    report("%s: Couldn't set Absolute mode\n",TestName);
		    FAIL;
		    }
	    vs = (XValuatorState *) ((char *) vs + vs->length);
	    }

	list = XListInputDevices(display, &ndevices);
	for (i=0; i<ndevices; i++,list++)
	    {
	    if (list->id!=Devs.DvVal->device_id)
		continue;
	    any = (XAnyClassPtr) (list->inputclassinfo);
	    for (j=0,l=0; j<list->num_classes; j++)
		{
		if (any->class == ValuatorClass)
		    {
		    a = ((XValuatorInfo *) any)->axes;
		    num_valuators += ((XValuatorInfo *) any)->num_axes;
		    for (k=0; k<((XValuatorInfo *) any)->num_axes;k++)
			tvals[l++] = (a+k)->min_value;
		    }
	        any = (XAnyClassPtr) ((char *) any + any->length);
		}
	    }
	ret = XCALL;
	state = XQueryDeviceState(display, device);
	vs = (XValuatorState *) state->data;
	for (i=0; i<state->num_classes; i++)
	    {
	    if (vs->class == ValuatorClass)
		{
		for (j=0; j<vs->num_valuators; j++)
		    if (*(vs->valuators + j) == tvals[j])
			{
			CHECK;
			count++;
			}
		    else
			{
			report("%s: Wrong value for valuator %d, got %d wanted %d\n",TestName,j,*(vs->valuators+j),tvals[j]);
			FAIL;
			}
		break;
		}
	    vs = (XValuatorState *) ((char *) vs + vs->length);
	    }
	num_valuators = 1;
	tvals[0] = a->max_value;
	ret = XCALL;
	state = XQueryDeviceState(display, device);
	vs = (XValuatorState *) state->data;
	for (i=0; i<state->num_classes; i++)
	    {
	    if (vs->class == ValuatorClass)
		{
		if (*vs->valuators == a->max_value)
		    {
		    CHECK;
		    count++;
		    }
		else
		    {
		    report("%s: Wrong value for valuator 0, got %d wanted %d\n",TestName,*vs->valuators,tvals[0]);
		    FAIL;
		    }
		for (j=1; j<vs->num_valuators; j++)
		    if (*(vs->valuators + j) == tvals[j])
			{
			CHECK;
			count++;
			}
		    else
			{
			report("%s: Wrong value for valuator %d, got %d wanted %d\n",TestName,j,*(vs->valuators+j),tvals[j]);
			FAIL;
			}
		break;
		}
	    vs = (XValuatorState *) ((char *) vs + vs->length);
	    }
	CHECKPASS(count);

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

	if (!Setup_Extension_DeviceInfo(DValMask))
	    {
	    untested("%s: Required input extension device not present.\n", TestName);
	    return;
	    }
	device = Devs.DvVal;
        grab_window = defwin(Dsp);

	XGrabDevice(Dsp, Devs.DvVal, grab_window, True, 0, 
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

>>ASSERTION Bad B 3
A call to xname will fail with a BadValue error if too many valuators
are specified.
>>STRATEGY
Specify a first_valuator equal to the number of valuators - 1.
>>CODE BadValue

	if (!Setup_Extension_DeviceInfo(DValMask))
	    {
	    untested("%s: Required input extension device not present.\n", TestName);
	    return;
	    }
	device = Devs.DvVal;

	first_valuator = NumValuators-1;
	num_valuators = 2;
	XCALL;

	if (geterr() == BadValue)
		PASS;
	else
		FAIL;
>>ASSERTION Bad B 3
A call to xname will fail with a BadMatch error if an valid device
that has no valuators is specified.
>>STRATEGY
Make the call with an valid device that has no valuators.
>>CODE BadMatch

	if (!Setup_Extension_DeviceInfo(NValsMask))
	    {
	    untested("%s: Required input extension device not present.\n", TestName);
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

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
 * $XConsortium: chgdvctl.m,v 1.7 94/09/06 21:03:20 dpw Exp $
 */
>>TITLE XChangeDeviceControl XINPUT
int
xname()
Display	*display = Dsp;
XDevice	*device;
int control;
XDeviceControl *f = (XDeviceControl *) &dctl;
>>EXTERN
extern ExtDeviceInfo Devs;
static XDeviceResolutionControl dctl;
static int tval[255];

>>ASSERTION Good B 3
A valid call to xname, specifying the DEVICE_RESOLUTION control,
changes the resolutions reported for the device.
>>STRATEGY
Do a valid ChangeDeviceControl request, specifying DEVICE_RESOLUTION.
Verify that the resolutions are changed.
>>CODE
int i,j,k,l, count=0, ndevices;
XDeviceResolutionState *state;
XAnyClassPtr any;
XDeviceInfo *list;

    if (!Setup_Extension_DeviceInfo(DCtlMask))
	{
	untested("No input device that supports %s.\n", TestName);
	return;
	}
    
    /* Get initial values for resolutions so we can be sure we've
       changed them. */

    device = Devs.DvCtl;
    state = (XDeviceResolutionState *) 
	XGetDeviceControl (display, device, DEVICE_RESOLUTION);
    control = DEVICE_RESOLUTION;
    dctl.length = sizeof(XDeviceResolutionControl);
    dctl.control = DEVICE_RESOLUTION;
    dctl.first_valuator = 0;
    dctl.num_valuators = 0;
    dctl.resolutions = tval;

    list = XListInputDevices(display, &ndevices);
    for (i=0,l=0; i<ndevices; i++,list++)
	{
	if (list->id!=Devs.DvCtl->device_id)
	    continue;
	any = (XAnyClassPtr) (list->inputclassinfo);
	for (j=0; j<list->num_classes; j++)
	    {
	    if (any->class == ValuatorClass)
		{
		XAxisInfoPtr a = ((XValuatorInfo *) any)->axes;
		dctl.num_valuators += ((XValuatorInfo *) any)->num_axes;
		for (k=0;k<((XValuatorInfo *) any)->num_axes; k++,l++,a++)
		    if (k%2)
		        if (*(state->resolutions + l) != a->min_value)
			    tval[l] = a->min_value;
			else if (*(state->resolutions + l) != a->max_value)
			    tval[l] = a->max_value;
			else
			    {
			    report("%s: Can't change resolutions; minval=maxval=resolution: %d %d %d\n",TestName,a->min_value,a->max_value,*(state->resolutions +l));
			    UNTESTED;
			    }
		    else
		        if (*(state->resolutions + l) != a->max_value)
			    tval[l] = a->max_value;
			else if (*(state->resolutions + l) != a->min_value)
			    tval[l] = a->min_value;
			else
			    {
			    report("%s: Can't change resolutions; minval=maxval=resolution\n",TestName);
			    UNTESTED;
			    }
		}
	    any = (XAnyClassPtr) ((char *) any + any->length);
	    }
	}
    XCALL;
    if (geterr() == Success)
	CHECK;
    else
	FAIL;
    
    state = (XDeviceResolutionState *) 
	XGetDeviceControl (display, device, DEVICE_RESOLUTION);
    for(i=0; i<state->num_valuators; i++)
	if (*(state->resolutions + i) == tval[i])
	    {
	    CHECK;
	    count++;
	    }
	else
	    {
	    report("%s: resolution %d was %d expected %d.\n",
		TestName, i, *(state->resolutions+i), tval[i]);
	    FAIL;
	    }

    list = XListInputDevices(display, &ndevices);
    for (i=0,l=0; i<ndevices; i++,list++)
	{
	if (list->id!=Devs.DvCtl->device_id)
	    continue;
	any = (XAnyClassPtr) (list->inputclassinfo);
	for (j=0; j<list->num_classes; j++)
	    {
	    if (any->class == ValuatorClass)
		{
		XAxisInfoPtr a = ((XValuatorInfo *) any)->axes;
		for (k=0; k<((XValuatorInfo *) any)->num_axes; k++,l++,a++)
		    if (a->resolution == tval[l])
			{
			CHECK;
			count++;
			}
		    else
			{
			report("%s: XListInputDevices returned %d, wanted (%d)\n",TestName,a->resolution,tval[l]);
			FAIL;
			}
		}
	    any = (XAnyClassPtr) ((char *) any + any->length);
	    }
	}
	CHECKPASS(count+1);

>>ASSERTION Good B 3
A successful call to ChangeDeviceControl, specifying the
Device_Resolution control, does not affect the
resolutions of valuators outside the specified range.
>>STRATEGY
Do a valid ChangeDeviceControl request, specifying DEVICE_RESOLUTION.
Specify that only 1 axis should have its resolution changed
Verify that the other resolutions are unchanged.
>>CODE
int i,j,k,l, count=0, ndevices;
XDeviceResolutionState *state, *sstate;
XAnyClassPtr any;
XDeviceInfo *list;

    if (!Setup_Extension_DeviceInfo(DCtlMask))
	{
	untested("No input device that supports %s.\n", TestName);
	return;
	}
    /* Get initial values for resolutions so we can be sure we've
       changed them. */

    device = Devs.DvCtl;
    sstate = (XDeviceResolutionState *) 
	XGetDeviceControl (display, device, DEVICE_RESOLUTION);
    control = DEVICE_RESOLUTION;
    dctl.length = sizeof(XDeviceResolutionControl);
    dctl.control = DEVICE_RESOLUTION;
    dctl.first_valuator = 0;
    dctl.resolutions = tval;

    list = XListInputDevices(display, &ndevices);
    for (i=0,l=0; i<ndevices; i++,list++)
	{
	if (list->id!=Devs.DvCtl->device_id)
	    continue;
	any = (XAnyClassPtr) (list->inputclassinfo);
	for (j=0; j<list->num_classes; j++)
	    {
	    if (any->class == ValuatorClass)
		{
		XAxisInfoPtr a = ((XValuatorInfo *) any)->axes;
		if (((XValuatorInfo *) any)->num_axes <= 1)
		    {
		    report("%s: Unable to test that only 1 axis is changed\n",
			TestName);
		    UNTESTED;
		    }
		dctl.num_valuators = 1;
		tval[0] = a->min_value;
		if (*sstate->resolutions != a->min_value)
		    tval[0] = a->min_value;
		else if (*sstate->resolutions != a->max_value)
		    tval[0] = a->min_value;
		else
		    {
		    report("%s: Unable to test that 1 axis is changed.\n", TestName);
		    UNTESTED;
		    }
		}
	    any = (XAnyClassPtr) ((char *) any + any->length);
	    }
	}
    XCALL;
    if (geterr() == Success)
	CHECK;
    else
	FAIL;
    
    state = (XDeviceResolutionState *) 
	XGetDeviceControl (display, device, DEVICE_RESOLUTION);
    if (*state->resolutions == tval[0])
	{
	CHECK;
	count++;
	}
    else
	{
	report("%s: Axis 0 resolution could not be set.\n",TestName);
	FAIL;
	}
    for(i=1; i<state->num_valuators; i++)
	if (*(state->resolutions + i) == *(sstate->resolutions + i))
	    {
	    CHECK;
	    count++;
	    }
	else
	    {
	    report("%s: axis %d RESOLUTION was incorrectly set.\n",TestName,i);
	    FAIL;
	    }

    list = XListInputDevices(display, &ndevices);
    for (i=0; i<ndevices; i++,list++)
	{
	if (list->id!=Devs.DvCtl->device_id)
	    continue;
	any = (XAnyClassPtr) (list->inputclassinfo);
	for (j=0; j<list->num_classes; j++)
	    {
	    if (any->class == ValuatorClass)
		{
		XAxisInfoPtr a = ((XValuatorInfo *) any)->axes;
		if (a->resolution == tval[0])
			{
			CHECK;
			count++;
			a++;
			}
		    else
			{
			report("%s: XListInputDevices returned %d instead of the value returned by XGetDeviceControl (%d)\n",TestName,a->resolution,*(state->resolutions));
			FAIL;
			}
		for (k=1; k<((XValuatorInfo *) any)->num_axes; k++,a++)
		    if (a->resolution == *(sstate->resolutions+k))
			{
			CHECK;
			count++;
			}
		    else
			{
			report("%s: XListInputDevices returned %d instead of the value set by XChangeDeviceControl (%d)\n",TestName,a->resolution,*(sstate->resolutions+k));
			FAIL;
			}
		break;
		}
	    any = (XAnyClassPtr) ((char *) any + any->length);
	    }
	}
	CHECKPASS(count+1);


>>ASSERTION Good B 1
A call to ChangeDeviceControl, specifying a device control
state that conflicts with one in use by another client for
the same device will fail with a status of DeviceBusy.

>>ASSERTION Bad B 3
A call to xname, specifying the DEVICE_RESOLUTION control,
and resolutions beyond the minimum and maximum allowed, will
result in a BadValue error.
>>STRATEGY
Do a valid ChangeDeviceControl request, specifying DEVICE_RESOLUTION,
and resolutions beyond the minimum and maximum allowed.
Verify that a BadValue error occurs.
>>CODE BadValue
int ret, i,j,k, count=0, ndevices;
XDeviceResolutionState *state;
XAnyClassPtr any;
XDeviceInfo *list;

    if (!Setup_Extension_DeviceInfo(DCtlMask | ValMask))
	{
	untested("No input device that supports %s.\n", TestName);
	return;
	}
    device = Devs.DvCtl;
    control = DEVICE_RESOLUTION;
    dctl.first_valuator = 0;
    dctl.resolutions = tval;

    list = XListInputDevices(display, &ndevices);
    for (i=0; i<ndevices; i++,list++)
	{
	if (list->id!=Devs.DvCtl->device_id)
	    continue;
	any = (XAnyClassPtr) (list->inputclassinfo);
	for (j=0; j<list->num_classes; j++)
	    {
	    if (any->class == ValuatorClass)
		{
		XAxisInfoPtr a = ((XValuatorInfo *) any)->axes;
		dctl.num_valuators = ((XValuatorInfo *) any)->num_axes;
		for (k=0; k<((XValuatorInfo *) any)->num_axes; k++,a++)
		    tval[k] = k % 2 ? a->min_value-1 : a->max_value+1;
		}
	    any = (XAnyClassPtr) ((char *) any + any->length);
	    }
	}
    ret = XCALL;
    if (geterr() == BadValue)
	PASS;
    else
	{
	report("%s: Got %d instead of BadValue\n",TestName,ret);
	FAIL;
	}
>>ASSERTION Bad B 3
A call to xname, specifying the DEVICE_RESOLUTION control,
and a device that does not support having its resolution changed,
will result in a BadMatch error.
>>STRATEGY
Do a valid ChangeDeviceControl request, specifying DEVICE_RESOLUTION,
and a device with valuators that does not support having its resolution
changed
>>CODE BadMatch
int ret, i,j,k, count=0, ndevices;
XDeviceResolutionState *state;
XAnyClassPtr any;
XDeviceInfo *list;

    if (!Setup_Extension_DeviceInfo(NDvCtlMask))
	{
	untested("No valuator device that doesn't support %s.\n", TestName);
	return;
	}
    device = Devs.NDvCtl;
    control = DEVICE_RESOLUTION;
    dctl.first_valuator = 0;
    dctl.resolutions = tval;

    list = XListInputDevices(display, &ndevices);
    for (i=0; i<ndevices; i++,list++)
	{
	if (list->id!=device->device_id)
	    continue;
	any = (XAnyClassPtr) (list->inputclassinfo);
	for (j=0; j<list->num_classes; j++)
	    {
	    if (any->class == ValuatorClass)
		{
		XAxisInfoPtr a = ((XValuatorInfo *) any)->axes;
		dctl.num_valuators = ((XValuatorInfo *) any)->num_axes;
		for (k=0; k<((XValuatorInfo *) any)->num_axes; k++,a++)
		    tval[k] = k % 2 ? a->min_value : a->max_value;
		break;
		}
	    any = (XAnyClassPtr) ((char *) any + any->length);
	    }
	}
    ret = XCALL;
    if (geterr() == BadMatch)
	PASS;
    else
	{
	report("%s: Got %d instead of BadMatch\n",TestName,ret);
	FAIL;
	}
>>ASSERTION Bad B 3
If an invalid device is specified, a BadDevice error will result.
>>STRATEGY
Do a ChangeDeviceControl request, specifying an invalid device.
>>CODE baddevice
XDevice bogus;
XID baddevice;
int ximajor, first, err, ret, val;

    if (!XQueryExtension (display, INAME, &ximajor, &first, &err))
	{
	untested("%s: Input extension not supported.\n", TestName);
	return;
	}

    BadDevice (display, baddevice);
    bogus.device_id = 128;
    device = &bogus;
    control = DEVICE_RESOLUTION;
    dctl.length = sizeof(XDeviceResolutionControl);
    dctl.control = DEVICE_RESOLUTION;
    dctl.num_valuators = 1;
    dctl.first_valuator = 0;
    dctl.resolutions = &val;
    tval[0] = tval[1] = 0;
    ret = XCALL;
    XSync(display,0);
    if (geterr() == baddevice)
	PASS;
    else
	FAIL;

>>ASSERTION Bad B 3
If an valid device with an invalid Control is specified, 
a BadValue error will result.
>>STRATEGY
Do a ChangeDeviceControl, specifying an valid device with an invalid
Control.
>>CODE BadValue
int ret;

    if (!Setup_Extension_DeviceInfo(DCtlMask))
	{
	untested("No device that supports %s.\n", TestName);
	return;
	}
    device = Devs.DvCtl;
    control = -1;
    dctl.first_valuator = 0;
    dctl.num_valuators = 1;
    tval[0] = tval[1] = 0;
    ret = XCALL;
    if (geterr() == BadValue)
	PASS;
    else
	FAIL;

>>ASSERTION Bad B 3
If an valid device with an invalid DeviceControl id is specified,
a BadMatch error will result.
>>STRATEGY
Do a ChangeDeviceControl, specifying an valid device with an id of 255.
>>CODE BadMatch
int ret;

    if (!Setup_Extension_DeviceInfo(NValsMask))
	{
	untested("%s: No device without valuators.\n", TestName);
	return;
	}
    device = Devs.NoValuators;
    control = DEVICE_RESOLUTION;
    dctl.first_valuator = 0;
    dctl.num_valuators = 1;
    tval[0] = tval[1] = 0;
    ret = XCALL;
    if (geterr() == BadMatch)
	PASS;
    else
	FAIL;

>>ASSERTION Bad B 3
If xname is invoked with the DEVICE_RESOLUTION control, and the 
expression (first_valuator + num_valuators) is greater than the
number of axes reported by the device, a BadValue error will result.
>>STRATEGY
Do a ChangeDeviceControl, specifying a first valuator equal to the 
number of valuators on the device, with a non-zero num_valuators
parameter
>>CODE BadValue
int ret;

    if (!Setup_Extension_DeviceInfo(DCtlMask))
	{
	untested("No device that supports %s.\n", TestName);
	return;
	}
    device = Devs.DvCtl;
    control = DEVICE_RESOLUTION;
    dctl.length = sizeof(XDeviceResolutionControl);
    dctl.control = DEVICE_RESOLUTION;
    dctl.first_valuator = 255;
    dctl.num_valuators = 2;
    tval[0] = tval[1] = 0;
    dctl.resolutions = tval;
    ret = XCALL;
    if (geterr() == BadValue)
	PASS;
    else
	FAIL;

>>ASSERTION Bad B 3
If xname is invoked with the DEVICE_RESOLUTION control, and the 
device is already grabbed by another client, a status of AlreadyGrabbed
will be returned.
>>STRATEGY
Grab the target device.
Do a ChangeDeviceControl.  Verify that a status of AlreadyGrabbed
is returned.
>>CODE
int ret;
Display *client2;
XID devicemotionnotify;
XEventClass devicemotionnotifyclass;
Window w;

    if (!Setup_Extension_DeviceInfo(DCtlMask))
	{
	untested("No device that supports %s.\n", TestName);
	return;
	}
    device = Devs.DvCtl;
    control = DEVICE_RESOLUTION;
    dctl.first_valuator = 0;
    dctl.num_valuators = 1;
    tval[0] = tval[1] = 0;
    dctl.resolutions = tval;

    if ((client2 = opendisplay()) == 0)
	return;
    w = defwin(client2);

    DeviceMotionNotify(Devs.DvCtl, devicemotionnotify, devicemotionnotifyclass);
    ret = XGrabDevice(client2, Devs.DvCtl, w, False, 1, 
	&devicemotionnotifyclass, GrabModeAsync, GrabModeAsync, CurrentTime);
    XSync(display,0);
    if (ret != Success)
	{
	report("%s: Could not set up initial grab",TestName);
	FAIL;
	}
    else
	CHECK;

    device=Devs.DvCtl;
    ret = XCALL;
    XSync(display,0);
    if (ret == AlreadyGrabbed)
	CHECK;
    else
	{
	report("%s: Expected AlreadyGrabbed, got %d", TestName, ret);
	FAIL;
	}
    CHECKPASS(2);

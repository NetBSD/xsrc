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
 * $XConsortium: closdev.m,v 1.10 94/04/17 21:13:09 rws Exp $
 */

>>TITLE XCloseDevice XINPUT
int

Display	*display = Dsp;
XDevice *device;
>>EXTERN
extern ExtDeviceInfo Devs;

>>ASSERTION Good B 3
A successful call to xname closes the requested device.
>>SET return-value Success
>>STRATEGY
Call xname to close a device.
>>CODE
int ret;

	if (!Setup_Extension_DeviceInfo(KeyMask))
	    {
	    untested("%s: No input extension key device.\n", TestName);
	    return;
	    }
	device = Devs.Key;
	ret = XCALL;
	if (ret==Success)
	    PASS;
	else
	    FAIL;
	Close_Extension_Display();

>>ASSERTION Good B 3
If a device is actively grabbed by a client, a successful call to
xname releases the active grab.
>>STRATEGY
Actively grab device.
Call xname to close the device.
Try to grab the device from another client.
>>CODE
int 	ret;
Window	w;
XID devicekeypress, saveid;
XEventClass devicekeypressclass;
Display *client1;
XDevice *dev2;

	if (!Setup_Extension_DeviceInfo(KeyMask))
	    {
	    untested("%s: No input extension key device.\n", TestName);
	    return;
	    }

	client1 = opendisplay();
	if (client1 == (Display *) NULL) {
		delete("Can not open display");
		return;
	}
	else
		CHECK;

	device = Devs.Key;
	w = defwin(display);
	DeviceKeyPress(device, devicekeypress, devicekeypressclass);
	XGrabDevice(display, device, w, True, 1, &devicekeypressclass, 
	    GrabModeAsync, GrabModeAsync, CurrentTime);
	XSync (display,0);
	if (isdeleted()) {
		delete("Could not set up initial grab");
		return;
	}
	dev2 = XOpenDevice(display,Devs.Key->device_id);

	ret = XCALL;
	if (ret==Success)
	    CHECK;
	else
	    FAIL;

	XGrabDevice(client1, dev2, w, True, 1, &devicekeypressclass, 
	    GrabModeAsync, GrabModeAsync, CurrentTime);
	XSync (client1,0);
	if (isdeleted()) {
		delete("Could not set up second grab");
		return;
	}
	else
		CHECK;
	CHECKPASS(3);
	Close_Extension_Display();

>>ASSERTION Good B 3
If a passive grab is specified by a client for a device a successful 
call to CloseDevice releases the passive grab.
>>STRATEGY
Set up a passive grab on a device.
Call xname to close the device.
Set up a passive grab for the same key/modifier combination.
>>CODE
int 	ret;
Window	w;
XID devicekeypress;
XEventClass devicekeypressclass;
Display *client1;
XDevice *dev2;
int Min_KeyCode, Max_KeyCode, numkeys;

	if (!Setup_Extension_DeviceInfo(KeyMask))
	    {
	    untested("%s: No input extension key device.\n", TestName);
	    return;
	    }
	client1 = opendisplay();
	if (client1 == (Display *) NULL) {
		delete("Can not open display");
		return;
	}
	else
		CHECK;

	MinMaxKeys (client1, Devs.Key, &Min_KeyCode, &Max_KeyCode, &numkeys);
	device = Devs.Key;
	dev2 = XOpenDevice(client1,Devs.Key->device_id);
	w = defwin(display);
	DeviceKeyPress(device, devicekeypress, devicekeypressclass);
	XGrabDeviceKey(display, device, Min_KeyCode, AnyModifier, NULL,
		w, True, 1, &devicekeypressclass, GrabModeAsync, GrabModeAsync);
	XSync (display,0);

	ret = XCALL;
	if (ret==Success)
	    CHECK;
	else
	    FAIL;

	ret = XGrabDeviceKey(client1, dev2, Min_KeyCode, AnyModifier, NULL,
		w, True, 1, &devicekeypressclass, GrabModeAsync, GrabModeAsync);
	XSync (client1,0);
	if (ret != GrabSuccess){
		report("Could not set up second grab");
		FAIL;
	}
	else
		CHECK;
	CHECKPASS(3);
	Close_Extension_Display();

>>ASSERTION Good B 3
If a device is frozen and events have been enqueued, a successful call 
to CloseDevice thaws the device and releases the queued events.
>>CODE
int 	ret, count=0;
Window	w, w2;
XID saveid;
XEventClass dkpclass, dkrclass;
XID dkp, dkr;
Display *client1;
XDevice *dev2=NULL;
XEvent ev;
int Min_KeyCode, Max_KeyCode, numkeys;

	if (!Setup_Extension_DeviceInfo(KeyMask))
	    {
	    untested("%s: No input extension key device.\n", TestName);
	    return;
	    }
	device = Devs.Key;
	client1 = opendisplay();
	if (client1 == (Display *) NULL) {
		delete("Can not open display");
		return;
	}
	else
	    {
	    CHECK;
	    count++;
	    }
	MinMaxKeys (client1, Devs.Key, &Min_KeyCode, &Max_KeyCode, &numkeys);
	dev2 = XOpenDevice(client1,Devs.Key->device_id);

	w = defwin(display);
	w2 = defwin(client1);
	(void) warppointer(client1, w2, 1, 1);
	DeviceKeyPress(dev2, dkp, dkpclass);
	DeviceKeyRelease(dev2, dkr, dkrclass);
	XSelectExtensionEvent(client1, w2, &dkpclass, 1);
	XGrabDevice(display, device, w, True, 0, NULL, GrabModeSync, 
	    GrabModeAsync, CurrentTime);
	XSync (display,0);
	XSync (client1,0);
	if (isdeleted()) {
		delete("Could not set up initial grab");
		return;
	}
	if (noext(1))
		return;

	devicekeypress(client1, dev2, Min_KeyCode);
	devicekeyrel(client1, dev2, Min_KeyCode);
	devicekeypress(client1, dev2, Min_KeyCode);
	devicekeyrel(client1, dev2, Min_KeyCode);
	devicekeypress(client1, dev2, Min_KeyCode);
	devicekeyrel(client1, dev2, Min_KeyCode);
	devicekeypress(client1, dev2, Min_KeyCode);
	devicekeyrel(client1, dev2, Min_KeyCode);
	XSync (client1,0);

	ret = XCALL;
	if (ret==Success)
	    {
	    CHECK;
	    count++;
	    }
	else
	    FAIL;

	XSync (client1,0);
	while (XPending(client1)) 
	    {
	    XNextEvent(client1, &ev);
	    if (ev.type==dkp || ev.type==dkr)
		CHECK;
	    else
		FAIL;
	    }
	devicerelkeys(dev2);
	CHECKPASS(count+4);
	Close_Extension_Display();


>>ASSERTION Good B 3
If more than one client has opened a device, a successful call to 
CloseDevice does not affect access to the device by other clients.
>>CODE
int 	i,ret;
Window	w;
Display *client1;
XDevice *dev2;
XEventClass dkpclass, dkrclass;
XID dkp, dkr;
XEvent ev;
int Min_KeyCode, Max_KeyCode, numkeys;

	if (!Setup_Extension_DeviceInfo(KeyMask))
	    {
	    untested("%s: No input extension key device.\n", TestName);
	    return;
	    }
	if (noext(1))
	    return;
	device = Devs.Key;
	client1 = opendisplay();
	if (client1 == (Display *) NULL) {
		delete("Can not open display");
		return;
	}
	else
		CHECK;
	MinMaxKeys (client1, Devs.Key, &Min_KeyCode, &Max_KeyCode, &numkeys);
	dev2 = XOpenDevice(client1,Devs.Key->device_id);

	w = defwin(client1);
	(void) warppointer(client1, w, 1, 1);
	DeviceKeyPress(dev2, dkp, dkpclass);
	DeviceKeyRelease(dev2, dkr, dkrclass);
	XSelectExtensionEvent(client1, w, &dkpclass, 1);
	XSync (client1,0);

	ret = XCALL;
	if (ret==Success)
	    CHECK;
	else
	    FAIL;

	for (i=0; i<4; i++)
	    {
	    devicekeypress(client1, dev2, Min_KeyCode);
	    devicekeyrel(client1, dev2, Min_KeyCode);
	    }
	XSync (client1,0);

	while (XPending(client1)) 
	    {
	    XNextEvent(client1, &ev);
	    if (ev.type==dkp || ev.type==dkr)
		CHECK;
	    else
		FAIL;
	    }
	devicerelkeys(dev2);
	CHECKPASS(6);
	Close_Extension_Display();

>>ASSERTION Bad B 3
A call to xname closes the requested device, causing any subsequent request
that requires a Device to fail with a BadDevice error.
>>SET return-value Success
>>STRATEGY
Call xname to close a device.
Call other input device extension requests.
Verify a BadDevice error occurs.
>>CODE
XID baddevice, devicekeypress;
XDeviceInfo *list;
int ret, i, j, ndevices, revert, nfeed, mask, ksyms_per;
int nevents, mode, evcount, valuators, min, max, count=0;
Window focus, w;
Time time;
XKbdFeedbackControl feedctl;
KeySym ksyms;
XModifierKeymap *modmap;
unsigned char bmap[8];
XDeviceResolutionControl dctl;
XEventClass devicekeypressclass;
XEvent ev;
XDevice bogus;
XAnyClassPtr any;

	w = defwin(display);
	list = XListInputDevices (display, &ndevices);
	device = NULL;
	for (i=0; i<ndevices; i++,list++)
	    if (list->use == IsXExtensionDevice) {
		any = (XAnyClassPtr) (list->inputclassinfo);
		for (j=0; j<list->num_classes; j++) {
		    if (any->class == KeyClass) {
			device = XOpenDevice (display, list->id);
			min = ((XKeyInfo *) any)->min_keycode;
			max = ((XKeyInfo *) any)->max_keycode;
			break;
		    }
	    	    any = (XAnyClassPtr) ((char *) any + any->length);
		}
	    }
	if (!device) {
	    untested("required input extension device is not present\n");
	    return;
	}
	bogus.device_id = device->device_id;
	DeviceKeyPress(device, devicekeypress, devicekeypressclass);
	modmap = XGetDeviceModifierMapping(display, device);
	BadDevice (display, baddevice);

	ret = XCALL;				/* close the device */
	XSync (display,0);
	XSetErrorHandler(error_status);

	device = &bogus;
	XSetDeviceMode(display, device, Absolute);
	XSync (display,0);
	if (geterr() == baddevice)
		{
		CHECK;
		count++;
		}
	else
		{
		report("XSetDeviceMode did not return BadDevice\n");
		FAIL;
		}
	XGetDeviceMotionEvents(display, device, CurrentTime, CurrentTime,
	    &nevents, &mode, &evcount);
	XSync (display,0);
	if (geterr() == baddevice)
		{
		CHECK;
		count++;
		}
	else
		{
		report("XGetDeviceMotionEvents did not return BadDevice\n");
		FAIL;
		}

	XChangeKeyboardDevice(display, device);
	XSync (display,0);
	if (geterr() == baddevice)
		{
		CHECK;
		count++;
		}
	else
		{
		report("XChangeKeyboardDevice did not return BadDevice\n");
		FAIL;
		}

	XChangePointerDevice(display, device, 0, 1);
	XSync (display,0);
	if (geterr() == baddevice)
		{
		CHECK;
		count++;
		}
	else
		{
		report("XChangePointerDevice did not return BadDevice\n");
		FAIL;
		}

	XGrabDevice(display, device, w, True, 1, &devicekeypressclass,
	   GrabModeAsync, GrabModeAsync, CurrentTime);
	if (geterr() == baddevice)
		{
		CHECK;
		count++;
		}
	else
		{
		report("XGrabDevice did not return BadDevice\n");
		FAIL;
		}

	XUngrabDevice(display, device, CurrentTime);
	XSync (display,0);
	if (geterr() == baddevice)
		{
		CHECK;
		count++;
		}
	else
		{
		report("XUngrabDevice did not return BadDevice\n");
		FAIL;
		}

	XGrabDeviceKey(display, device, AnyKey, AnyModifier, NULL, 
	   w, True, 0, NULL, GrabModeAsync, GrabModeAsync);
	if (geterr() == baddevice)
		{
		CHECK;
		count++;
		}
	else
		{
		report("XGrabDeviceKey did not return BadDevice\n");
		FAIL;
		}

	XUngrabDeviceKey(display, device, AnyKey, AnyModifier, NULL, w);
	XSync (display,0);

	if (geterr() == baddevice)
		{
		CHECK;
		count++;
		}
	else
		{
		report("XUngrabDeviceKey did not return BadDevice\n");
		FAIL;
		}

	XGrabDeviceButton(display, device, AnyButton, AnyModifier, NULL, 
	   w, True, 0, NULL, GrabModeAsync, GrabModeAsync);
	if (geterr() == baddevice)
		{
		CHECK;
		count++;
		}
	else
		{
		report("XGrabDeviceButton did not return BadDevice\n");
		FAIL;
		}

	XUngrabDeviceButton(display, device, AnyButton, AnyModifier, NULL, w);
	XSync (display,0);
	if (geterr() == baddevice)
		{
		CHECK;
		count++;
		}
	else
		{
		report("XUngrabDeviceButton did not return BadDevice\n");
		FAIL;
		}

	XAllowDeviceEvents(display, device, AsyncAll, CurrentTime);
	XSync (display,0);
	if (geterr() == baddevice)
		{
		CHECK;
		count++;
		}
	else
		{
		report("XAllowDeviceEvents did not return BadDevice\n");
		FAIL;
		}

	XGetDeviceFocus(display, device, &focus, &revert, &time);
	if (geterr() == baddevice)
		{
		CHECK;
		count++;
		}
	else
		{
		report("XGetDeviceFocus did not return BadDevice\n");
		FAIL;
		}

	XSetDeviceFocus(display, device, None, RevertToNone, CurrentTime);
	XSync (display,0);
	if (geterr() == baddevice)
		{
		CHECK;
		count++;
		}
	else
		{
		report("XSetDeviceFocus did not return BadDevice\n");
		FAIL;
		}

	XGetFeedbackControl(display, device, &nfeed);
	if (geterr() == baddevice)
		{
		CHECK;
		count++;
		}
	else
		{
		report("XGetFeedbackControl did not return BadDevice\n");
		FAIL;
		}

	feedctl.class = KbdFeedbackClass;
	feedctl.percent = 0;
	mask = DvPercent;
	XChangeFeedbackControl(display, device, mask, (XFeedbackControl*) &feedctl);
	XSync (display,0);
	if (geterr() == baddevice)
		{
		CHECK;
		count++;
		}
	else
		{
		report("XChangeFeedbackControl did not return BadDevice\n");
		FAIL;
		}

	XGetDeviceKeyMapping(display, device, min, 1, &ksyms_per);
	if (geterr() == baddevice)
		{
		CHECK;
		count++;
		}
	else
		{
		report("XGetDeviceKeyMapping did not return BadDevice\n");
		FAIL;
		}

	XChangeDeviceKeyMapping(display, device, min, 1, &ksyms, 1);
	XSync (display,0);
	if (geterr() == baddevice)
		{
		CHECK;
		count++;
		}
	else
		{
		report("XChangeDeviceKeyMapping did not return BadDevice\n");
		FAIL;
		}

	XGetDeviceModifierMapping(display, device);
	if (geterr() == baddevice)
		{
		CHECK;
		count++;
		}
	else
		{
		report("XGetDeviceModifierMapping did not return BadDevice\n");
		FAIL;
		}

	XSetDeviceModifierMapping(display, device, modmap);
	XSync (display,0);
	if (geterr() == baddevice)
		{
		CHECK;
		count++;
		}
	else
		{
		report("XSetDeviceModifierMapping did not return BadDevice\n");
		FAIL;
		}

	XGetDeviceButtonMapping(display, device, bmap, 8);
	if (geterr() == baddevice)
		{
		CHECK;
		count++;
		}
	else
		{
		report("XGetDeviceButtonMapping did not return BadDevice\n");
		FAIL;
		}

	XSetDeviceButtonMapping(display, device, bmap, 8);
	XSync (display,0);
	if (geterr() == baddevice)
		{
		CHECK;
		count++;
		}
	else
		{
		report("XSetDeviceButtonMapping did not return BadDevice\n");
		FAIL;
		}

	XQueryDeviceState(display, device);
	if (geterr() == baddevice)
		{
		CHECK;
		count++;
		}
	else
		{
		report("XQueryDeviceState did not return BadDevice\n");
		FAIL;
		}

	XSetDeviceValuators(display, device, &valuators, 0, 1);
	XSync (display,0);
	if (geterr() == baddevice)
		{
		CHECK;
		count++;
		}
	else
		{
		report("XSetDeviceValuators did not return BadDevice\n");
		FAIL;
		}

	XDeviceBell(display, device, 0, 0, 100);
	XSync (display,0);
	if (geterr() == baddevice)
		{
		CHECK;
		count++;
		}
	else
		{
		report("XDeviceBell did not return BadDevice\n");
		FAIL;
		}

	XGetDeviceControl(display, device, DEVICE_RESOLUTION);
	if (geterr() == baddevice)
		{
		CHECK;
		count++;
		}
	else
		{
		report("XGetDeviceControl did not return BadDevice\n");
		FAIL;
		}

	dctl.length = sizeof(XDeviceResolutionControl);
	dctl.control = DEVICE_RESOLUTION;
	dctl.num_valuators=1;
	dctl.first_valuator=0;
	dctl.resolutions = &valuators;
	XChangeDeviceControl(display, device, DEVICE_RESOLUTION, (XDeviceControl *) &dctl);
	XSync (display,0);
	if (geterr() == baddevice)
		{
		CHECK;
		count++;
		}
	else
		{
		report("XChangeDeviceControl did not return BadDevice\n");
		FAIL;
		}

	ev.type = devicekeypress;
	XSendExtensionEvent(display, device, PointerWindow, True, 0, NULL,
	    &ev);
	XSync(display,0);
	if (geterr() == baddevice)
		{
		CHECK;
		count++;
		}
	else
		{
		report("XSendExtensionEvent did not return BadDevice\n");
		FAIL;
		}

	XSetErrorHandler(unexp_err);
	CHECKPASS(count);

>>ASSERTION Bad B 3
A call to xname will fail with a BadDevice error if an invalid device
is specified.
>>STRATEGY
Make the call with an invalid device.
>>CODE baddevice
XID baddevice;
int ret;

	if (!Setup_Extension_DeviceInfo(KeyMask))
	    {
	    untested("%s: No input extension key device.\n", TestName);
	    return;
	    }
	BadDevice (display, baddevice);
	Devs.Key->device_id = -1;
	device = Devs.Key;

	ret = XCALL;

	if (geterr() == baddevice)
		PASS;
	else
		FAIL;

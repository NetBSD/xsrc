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
 * $XConsortium: chgkbd.m,v 1.9 94/09/06 20:50:19 dpw Exp $
 */
>>TITLE XChangeKeyboardDevice XINPUT
void

Display	*display = Dsp;
XDevice *device;
>>EXTERN
extern ExtDeviceInfo Devs;
extern int MinKeyCode;

>>ASSERTION Good B 3
A call to xname changes the X keyboard.
>>STRATEGY
Change the X keyboard.  Verify via XListInputDevices that the keyboard
was changed.
>>EXTERN
verify_kbd(dpy, id)
	Display *dpy;
	int id;
	{
	XDeviceInfo *list;
	int i, ndevices;

	list = XListInputDevices (display, &ndevices);
	for (i=0; i<ndevices; i++,list++)
	    if (list->use==IsXKeyboard)
		if (list->id == id)
		    return(True);
		else
		    return(False);
	if (i==ndevices)
	    return(False);
	}

>>CODE
XDeviceInfo *list;
int i, ndevices, savid;

	if (!Setup_Extension_DeviceInfo(KeyMask))
	    {
	    untested("%s: No input extension key device.\n", TestName);
	    return;
	    }
	list = XListInputDevices (display, &ndevices);
	for (i=0; i<ndevices; i++,list++)
	    if (list->use==IsXKeyboard)
		savid = list->id;

	device = Devs.Key;
	XCALL;
	if (verify_kbd(display, Devs.Key->device_id))
	    CHECK;
	else
	    {
	    report("%s: Couldn't change X keyboard\n",TestName);
	    FAIL;
	    }

	device = XOpenDevice(display, savid);
	XCALL;
	if (verify_kbd(display, savid))
	    CHECK;
	else
	    {
	    report("%s: Couldn't restore X keyboard\n",TestName);
	    FAIL;
	    }
	CHECKPASS(2);

>>ASSERTION Good B 3
Termination of the client that changed the keyboard does not affect
which input device is the X keyboard.
>>STRATEGY
Change the keyboard to a new device.
Terminate the client that made the change.
Verify that the X keyboard remains the same.
>>CODE
Display	*client1, *client2;
XDeviceInfo *list;
int i, ndevices, savid;

	if (!Setup_Extension_DeviceInfo(KeyMask))
	    {
	    untested("%s: No input extension key device.\n", TestName);
	    return;
	    }
	list = XListInputDevices (display, &ndevices);
	for (i=0; i<ndevices; i++,list++)
	    if (list->use==IsXKeyboard)
		savid = list->id;

	device = Devs.Key;
/* Create client1, without causing resource registration. */
	if (config.display == (char *) NULL) {
		delete("config.display not set");
		return;
	}
	else
		CHECK;
	client1 = XOpenDisplay(config.display);
	if (client1 == (Display *) NULL) {
		delete("Couldn't create client1.");
		return;
	}
	else
		CHECK;

	display = client1;
	XCALL;
	XCloseDisplay(display);

	if ((client2 = opendisplay()) == NULL)
		return;

	if (verify_kbd(client2, Devs.Key->device_id))
	    CHECK;
	else
	   {
    	   report("%s: Couldn't change X keyboard\n",TestName);
	   FAIL;
	   }

	device = XOpenDevice(display, savid);
	XCALL;
	if (verify_kbd(display, savid))
	    CHECK;
	else
	    {
	    report("%s: Couldn't restore X keyboard\n",TestName);
	    FAIL;
	    }
	CHECKPASS(4);

>>ASSERTION Good B 3
After a successful call to ChangeKeyboardDevice, the focus state of
the new keyboard is the same as that of the old keyboard.
>>STRATEGY
Create a window and set the keyboard focus to it.
Change the keyboard to a new device.
Verify that the X keyboard focus is the same as it was for the old device.
>>CODE
XDeviceInfo *list;
int i, ndevices, revert, savid;
Window focus, w;
Time time;

	if (!Setup_Extension_DeviceInfo(KeyMask))
	    {
	    untested("%s: No input extension key device.\n", TestName);
	    return;
	    }
	device = Devs.Key;
	list = XListInputDevices (display, &ndevices);
	for (i=0; i<ndevices; i++,list++)
	    if (list->use==IsXKeyboard)
		savid = list->id;
	w = defwin(display);
	XSetDeviceFocus(display, device, None, RevertToNone, CurrentTime);
	XGetDeviceFocus(display, device, &focus, &revert, &time);
	if (focus != None || revert != RevertToNone)
	    {
	    report("%s: Unable to set focus for new kbd",TestName);
	    FAIL;
	    }
	else
	    CHECK;

	XSetInputFocus (display, w, RevertToPointerRoot, CurrentTime);
	XGetInputFocus (display, &focus, &revert);
	if (focus != w || revert != RevertToPointerRoot)
	    {
	    report("%s: Unable to set focus for old kbd");
	    FAIL;
	    }
	else
	    CHECK;

	XCALL;

	if (verify_kbd(display, Devs.Key->device_id))
	    CHECK;
	else
	   {
    	   report("%s: Couldn't change X keyboard\n",TestName);
	   FAIL;
	   }

	XGetInputFocus (display, &focus, &revert);
	if (focus != w || revert != RevertToPointerRoot)
		{
		report("%s: New kbd focus != old kbd focus: %x %x %x %x",TestName, focus, w, revert, RevertToPointerRoot);
	        FAIL;
		}
	    else
		CHECK;

	device = XOpenDevice(display, savid);
	XCALL;
	if (verify_kbd(display, savid))
	    CHECK;
	else
	    {
	    report("%s: Couldn't restore X keyboard\n",TestName);
	    FAIL;
	    }
	CHECKPASS(5);

>>ASSERTION Good B 3

>>ASSERTION Bad B 3
A call to xname will fail with a status of 
.S AlreadyGrabbed 
if some other client has grabbed the new device.
>>STRATEGY
Grab the new device.
Create client2.
Attempt to change the keyboard to the new device.
Verify AlreadyGrabbed error status
>>CODE
int ret;
Display	*client2;
Window grab_window;

	if (!Setup_Extension_DeviceInfo(KeyMask))
	    {
	    untested("%s: No input extension key device.\n", TestName);
	    return;
	    }
	device = Devs.Key;
        grab_window = defwin(Dsp);

	XGrabDevice(Dsp, Devs.Key, grab_window, True, 0, 
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
		CHECK;
	else
		FAIL;

	CHECKPASS(1);
	XSync(display,0);

>>ASSERTION Bad B 3
An attempt to call any input device extension request that requires
a Device specifying the new X keyboard will result in a BadDevice error.
>>STRATEGY
Change the keyboard to a new device.
Verify that all input device extension requests that require a Device pointer
fail with a BadDevice error, when the new keyboard is specified.
>>CODE
XID baddevice, devicekeypress;
XDeviceInfo *list;
int i, ndevices, revert, nfeed, mask, ksyms_per;
int nevents, mode, evcount, valuators, count=0;
Window focus, w;
Time time;
XKbdFeedbackControl feedctl;
KeySym ksyms;
XModifierKeymap *modmap;
unsigned char bmap[8];
XDeviceResolutionControl dctl;
XEventClass devicekeypressclass;
XEvent ev;


	if (!Setup_Extension_DeviceInfo(KeyMask))
	    {
	    untested("%s: No input extension key device.\n", TestName);
	    return;
	    }
	w = defwin(display);
	device = Devs.Key;
	DeviceKeyPress(device, devicekeypress, devicekeypressclass);
	modmap = XGetDeviceModifierMapping(display, device);
	BadDevice (display, baddevice);
	XCALL;

	list = XListInputDevices (display, &ndevices);
	for (i=0; i<ndevices; i++,list++)
	    if (list->use==IsXKeyboard)
		if (list->id == Devs.Key->device_id)
		    {
		    CHECK;
		    count++;
		    break;
		    }
		else
		    {
	    	    report("%s: Couldn't change X keyboard\n",TestName);
		    FAIL;
		    break;
		    }
	if (i==ndevices)
	    {
	    report("%s: Failed to find X keyboard\n",TestName);
	    FAIL;
	    }

	XSetErrorHandler(error_status);
	XOpenDevice(display, device->device_id);
	if (geterr() == baddevice)
		{
		CHECK;
		count++;
		}
	else
		FAIL;

	XCloseDevice(display, device);
	if (geterr() == baddevice)
		{
		CHECK;
		count++;
		}
	else
		FAIL;

	XSetDeviceMode(display, device, Absolute);
	if (geterr() == baddevice)
		{
		CHECK;
		count++;
		}
	else
		FAIL;

	XGetDeviceMotionEvents(display, device, CurrentTime, CurrentTime,
	    &nevents, &mode, &evcount);
	if (geterr() == baddevice)
		{
		CHECK;
		count++;
		}
	else
		FAIL;

	XChangeKeyboardDevice(display, device);
	if (geterr() == baddevice)
		{
		CHECK;
		count++;
		}
	else
		FAIL;

	XChangePointerDevice(display, device, 0, 1);
	if (geterr() == baddevice)
		{
		CHECK;
		count++;
		}
	else
		FAIL;

	XGrabDevice(display, device, w, True, 1, &devicekeypressclass,
	   GrabModeAsync, GrabModeAsync, CurrentTime);
	if (geterr() == baddevice)
		{
		CHECK;
		count++;
		}
	else
		FAIL;

	XUngrabDevice(display, device, CurrentTime);
	if (geterr() == baddevice)
		{
		CHECK;
		count++;
		}
	else
		FAIL;

	XGrabDeviceKey(display, device, AnyKey, AnyModifier, NULL, 
	   w, True, 0, NULL, GrabModeAsync, GrabModeAsync);
	if (geterr() == baddevice)
		{
		CHECK;
		count++;
		}
	else
		FAIL;

	XUngrabDeviceKey(display, device, AnyKey, AnyModifier, NULL, w);

	if (geterr() == baddevice)
		{
		CHECK;
		count++;
		}
	else
		FAIL;

	XGrabDeviceButton(display, device, AnyButton, AnyModifier, NULL, 
	   w, True, 0, NULL, GrabModeAsync, GrabModeAsync);
	if (geterr() == baddevice)
		{
		CHECK;
		count++;
		}
	else
		FAIL;

	XUngrabDeviceButton(display, device, AnyButton, AnyModifier, NULL, w);
	if (geterr() == baddevice)
		{
		CHECK;
		count++;
		}
	else
		FAIL;

	XAllowDeviceEvents(display, device, AsyncAll, CurrentTime);
	if (geterr() == baddevice)
		{
		CHECK;
		count++;
		}
	else
		FAIL;

	XGetDeviceFocus(display, device, &focus, &revert, &time);
	if (geterr() == baddevice)
		{
		CHECK;
		count++;
		}
	else
		FAIL;

	XSetDeviceFocus(display, device, None, RevertToNone, CurrentTime);
	if (geterr() == baddevice)
		{
		CHECK;
		count++;
		}
	else
		FAIL;

	XGetFeedbackControl(display, device, &nfeed);
	if (geterr() == baddevice)
		{
		CHECK;
		count++;
		}
	else
		FAIL;

	feedctl.class = KbdFeedbackClass;
	feedctl.percent = 0;
	mask = DvPercent;
	XChangeFeedbackControl(display, device, mask, (XFeedbackControl *) &feedctl);
	if (geterr() == baddevice)
		{
		CHECK;
		count++;
		}
	else
		FAIL;

	XGetDeviceKeyMapping(display, device, MinKeyCode, 1, &ksyms_per);
	if (geterr() == baddevice)
		{
		CHECK;
		count++;
		}
	else
		FAIL;

	XChangeDeviceKeyMapping(display, device, MinKeyCode, 1, &ksyms, 1);
	if (geterr() == baddevice)
		{
		CHECK;
		count++;
		}
	else
		FAIL;

	XGetDeviceModifierMapping(display, device);
	if (geterr() == baddevice)
		{
		CHECK;
		count++;
		}
	else
		FAIL;

	XSetDeviceModifierMapping(display, device, modmap);
	if (geterr() == baddevice)
		{
		CHECK;
		count++;
		}
	else
		FAIL;

	XGetDeviceButtonMapping(display, device, bmap, 8);
	if (geterr() == baddevice)
		{
		CHECK;
		count++;
		}
	else
		FAIL;

	XSetDeviceButtonMapping(display, device, bmap, 8);
	if (geterr() == baddevice)
		{
		CHECK;
		count++;
		}
	else
		FAIL;

	XQueryDeviceState(display, device);
	if (geterr() == baddevice)
		{
		CHECK;
		count++;
		}
	else
		FAIL;

	XSetDeviceValuators(display, device, &valuators, 0, 1);
	if (geterr() == baddevice)
		{
		CHECK;
		count++;
		}
	else
		FAIL;

	XDeviceBell(display, device, 0, 0, 100);
	if (geterr() == baddevice)
		{
		CHECK;
		count++;
		}
	else
		FAIL;

	XGetDeviceControl(display, device, DEVICE_RESOLUTION);
	if (geterr() == baddevice)
		{
		CHECK;
		count++;
		}
	else
		FAIL;

	dctl.length = sizeof(XDeviceResolutionControl);
	dctl.control = DEVICE_RESOLUTION;
	dctl.num_valuators=1;
	dctl.first_valuator=0;
	dctl.resolutions = &valuators;
	XChangeDeviceControl(display, device, DEVICE_RESOLUTION, (XDeviceControl *) &dctl);
	if (geterr() == baddevice)
		{
		CHECK;
		count++;
		}
	else
		FAIL;

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
		FAIL;

	XSetErrorHandler(unexp_err);
	CHECKPASS(count);
>>ASSERTION Bad B 1
If the implementation does not support use of the specified device as
the X keyboard, a BadDevice error will result.
>>ASSERTION Bad B 3
A call to xname will fail with a BadDevice error if an invalid device
is specified.
>>STRATEGY
Make the call with an invalid device.
>>CODE baddevice
XDevice nodevice;
XID baddevice;
int ximajor, first, err;

	if (!XQueryExtension (display, INAME, &ximajor, &first, &err))
	    {
	    untested("%s: Input extension not supported.\n", TestName);
	    return;
	    }
	BadDevice (display, baddevice);
	nodevice.device_id = -1;
	device = &nodevice;

	XCALL;

	if (geterr() == baddevice)
		CHECK;
	else
		FAIL;

	CHECKPASS(1);
>>ASSERTION Bad B 3
A call to xname will fail with a status of 
.S GrabFrozen
if the device is frozen by the grab of some other client.
>>STRATEGY
Grab the new device when it is frozen by a grab of another device.
Create client2.
Attempt to change the keyboard to the new device.
>>CODE
int ret;
Display	*client2;
Window grab_window;

	if (!Setup_Extension_DeviceInfo(KeyMask | NKeysMask))
	    {
	    untested("%s: Required input extension devices not present.\n", 
		TestName);
	    return;
	    }
	device = Devs.Key;
        grab_window = defwin (display);

	XGrabDevice(Dsp, Devs.NoKeys, grab_window, True, 0, 
		NULL, GrabModeSync, GrabModeSync, CurrentTime);
	if (isdeleted()) {
		delete("Could not set up initial grab");
		return;
	}

	if ((client2 = opendisplay()) == NULL)
		return;

	display = client2;
	ret = XCALL;

	if (ret == GrabFrozen)
		CHECK;
	else
		{
		report("Device was not frozen by the grab of another device,ret=%d",ret);
		FAIL;
		}

	CHECKPASS(1);
>>ASSERTION Bad B 3
A call to xname will fail with a BadMatch error if the specified
device has no keys.
>>STRATEGY
Attempt to change the keyboard to a device that has no keys.
Verify BadMatch
>>CODE BadMatch
Display	*client2;
Window grab_window;

	if (!Setup_Extension_DeviceInfo(NKeysMask))
	    {
	    untested("%s: No input extension device without keys.\n", TestName);
	    return;
	    }
	device = Devs.NoKeys;

	XCALL;

	if (geterr() == BadMatch)
		CHECK;
	else
		FAIL;

	CHECKPASS(1);

/*
 * Copyright 1993 by the Hewlett-Packard Company..
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
 * $XConsortium: grbdev.m,v 1.7 94/04/17 21:12:42 rws Exp $
 */

>>TITLE XGrabDevice XINPUT
int

Display	*display = Dsp;
XDevice	*device;
Window	grab_window;
Bool	owner_events = True;
unsigned int 	event_count = 0;
XEventClass *event_list = NULL;
int 	this_device_mode = GrabModeAsync;
int 	other_devices_mode = GrabModeAsync;
Time	time = CurrentTime;
>>EXTERN

extern int MinKeyCode;
extern ExtDeviceInfo Devs;

/*
 * For all these tests note that the grab_window is automatically destroyed
 * at the end of the test, and therefore the grab is released.
 */

/*
 * Get the window that the pointer is currently in, if the pointer
 * is in a child of the given window. Otherwise it returns None.
 */
Window
getpointerwin(disp, win)
Display	*disp;
Window	win;
{
Window	child;
Window	wtmp;
int 	itmp;
unsigned uitmp;
Bool 	s;

	s = XQueryPointer(disp, win, &wtmp, &child, &itmp, &itmp, &itmp, &itmp
		, &uitmp);

	if (!s)
		delete("Could not get pointer window");

	return(child);
}

>>ASSERTION Good B 3
A successful call to xname actively grabs control of the device and returns
.S GrabSuccess .
>>SET return-value GrabSuccess
>>STRATEGY
Call xname.
Verify that it returns GrabSuccess.
>>CODE
int 	ret;

	if (!Setup_Extension_DeviceInfo(AnyMask))
	    {
	    untested("%s: No input extension key device.\n", TestName);
	    return;
	    }
	grab_window = defwin(display);
	device = Devs.Any;
	ret = XCALL;

	if (ret != GrabSuccess) {
		report("GrabSuccess was not returned");
		FAIL;
	} else
		CHECK;

	CHECKPASS(1);
>>ASSERTION Good B 3
A call to xname
overrides any active device grab by this client.
>>STRATEGY
Create two windows.
Call xname with one of the windows.
Call xname with the other window.
>>CODE
Window	w1, w2;
int ret;

	w1 = defwin(display);
	w2 = defwin(display);

	grab_window = w1;

	if (!Setup_Extension_DeviceInfo(AnyMask))
	    {
	    untested("%s: No input extension key device.\n", TestName);
	    return;
	    }
	device = Devs.Any;
	XCALL;

	ret = XCALL;

	if (ret == Success)
		PASS;
	else {
		report("A second grab did not override the first");
		FAIL;
	}


>>ASSERTION Good B 3
When a successful call  to  GrabDevice  is  made,  DeviceFocusIn
and DeviceFocusOut events are generated and sent to clients
requesting them.
>>STRATEGY
Call xname.
Verify that DeviceFocus events are received.
>>CODE
int 	ret;
XID	dfi, dfo;
XEventClass dficlass, dfoclass;
XEvent	ev;

	if (!Setup_Extension_DeviceInfo(AnyMask))
	    {
	    untested("%s: Required input extension device not present.\n", TestName);
	    return;
	    }
	grab_window = defwin(display);
	device = Devs.Any;
	DeviceFocusIn(device, dfi, dficlass);
	DeviceFocusOut(device, dfo, dfoclass);
	XSelectExtensionEvent(display, grab_window, &dficlass, 1);
	XSelectExtensionEvent(display, RootWindow(display,0), &dfoclass, 1);
	XSync (display,0);
	ret = XCALL;

	if (ret != GrabSuccess) {
		report("GrabSuccess was not returned");
		FAIL;
	} else
		CHECK;
	XSync (display,0);
	while (XPending(display)) 
	    {
	    XNextEvent(display, &ev);
	    if (ev.type==dfi || ev.type==dfo)
		CHECK;
	    else
		FAIL;
	    }

	CHECKPASS(5);

>>ASSERTION Bad B 3
If an event class from a device other than the grab_device
are specified on a call to GrabDevice, a BadClass error will
result.
>>STRATEGY
Call xname, specifying event classes from another device.
Verify that a BadClass error occurs.
>>CODE badclass
int 	ret;
XID	dkp,badclass;
XEventClass dkpclass;
XEvent	ev;

	if (!Setup_Extension_DeviceInfo(KeyMask | NKeysMask))
	    {
	    untested("%s: Required input extension device not present.\n", TestName);
	    return;
	    }
	BadClass(display,badclass);
	grab_window = defwin(display);
	device = Devs.Key;
	DeviceKeyPress(Devs.Key, dkp, dkpclass);
	dkpclass &= 0xff;
	dkpclass |= Devs.NoKeys->device_id << 8;
	event_list = &dkpclass;
	event_count = 1;
	ret = XCALL;

	if (geterr() != badclass) {
		report("BadClass was not returned");
		FAIL;
	} else
		PASS;

>>ASSERTION Good B 3
When
.A other_devices_mode
is
.S GrabModeAsync ,
then device event processing is unaffected by activation of the grab.
>>STRATEGY
Call xname with other_devices_mode set to GrabModeAsync.
Select events from another extension input device.
Verify that events can still be received from the other device.
>>CODE
XID dkr;
XEventClass dkrclass;
XEvent ev;

	if (!Setup_Extension_DeviceInfo(KeyMask | NKeysMask))
	    {
	    untested("%s: Required input extension device not present.\n", 
		TestName);
	    return;
	    }
	device = Devs.NoKeys;

	grab_window = defwin(display);
	(void) warppointer(display, grab_window, 1, 1);
	DeviceKeyRelease(Devs.Key, dkr, dkrclass);
	XSelectExtensionEvent(display, grab_window, &dkrclass, 1);
	other_devices_mode = GrabModeAsync;

	event_count=0;
	event_list=NULL;
	XCALL;

	if (noext(1))
	    {
	    report("There is no reliable test method, but a touch test was performed");
	    return;
	    }
	devicekeypress (display, Devs.Key, MinKeyCode);
	devicekeyrel (display, Devs.Key, MinKeyCode);
	XSync(display,0);
	while (XPending(display)) 
	    {
	    XNextEvent(display, &ev);
	    if (ev.type==dkr)
		CHECK;
	    }
	devicerelkeys (Devs.Key);
	CHECKPASS(1);
>>ASSERTION Good B 3
When
.A other_devices_mode
is
.S GrabModeSync ,
then the state of the other devices, as seen by
client applications,
appears to freeze, and no further device events are generated
until the grabbing client calls
.S XAllowDeviceEvents
or until the device grab is released.
>>STRATEGY
Call xname with other_devices_mode set to GrabModeSync.
Verify that no events are received from other devices until
XAllowDeviceEvents is called or until the device grab is released.
>>CODE
int ret;
XID dkr;
XEventClass dkrclass;
XEvent ev;

	if (!Setup_Extension_DeviceInfo(KeyMask | NKeysMask))
	    {
	    untested("%s: Required input extension device not present.\n", 
		TestName);
	    return;
	    }
	device = Devs.NoKeys;

	grab_window = defwin(display);
	(void) warppointer(display, grab_window, 1, 1);
	DeviceKeyRelease(Devs.Key, dkr, dkrclass);
	XSelectExtensionEvent(display, grab_window, &dkrclass, 1);
	other_devices_mode = GrabModeSync;
	event_count=0;
	event_list=NULL;
	ret = XCALL;

	if (noext(1))
	    {
	    report("There is no reliable test method, but a touch test was performed");
	    return;
	    }
	devicekeypress (display, Devs.Key, MinKeyCode);
	devicekeyrel (display, Devs.Key, MinKeyCode);
	XSync(display,0);
	while (XPending(display)) 
	    {
	    XNextEvent(display, &ev);
	    if (ev.type==dkr)
		{
		report("%s: Got events from other devices when they should be frozen.\n",TestName);
		FAIL;
		}
	    }

	/* Verify that XAllowDeviceEvents releases the queued events. */

	XAllowDeviceEvents(display, device, AsyncAll, CurrentTime);
	XSync(display,0);
	while (XPending(display)) 
	    {
	    XNextEvent(display, &ev);
	    if (ev.type==dkr)
		CHECK;
	    }

	ret = XCALL;
	devicekeypress (display, Devs.Key, MinKeyCode);
	devicekeyrel (display, Devs.Key, MinKeyCode);
	XSync(display,0);
	while (XPending(display)) 
	    {
	    XNextEvent(display, &ev);
	    if (ev.type==dkr)
		{
		report("%s: Got events from other devices when they should be frozen.\n",TestName);
		FAIL;
		}
	    }
	XUngrabDevice(display, device, CurrentTime);
	XSync(display,0);
	while (XPending(display)) 
	    {
	    XNextEvent(display, &ev);
	    if (ev.type==dkr)
		CHECK;
	    }

	CHECKPASS(2);
	devicerelkeys (Devs.Key);
	other_devices_mode = GrabModeAsync;
>>ASSERTION Good B 1
When a successful call to xname is made, then the last-device-grab
time is set to the specified time with
.S CurrentTime
replaced by the current X server time.

>>ASSERTION Good B 3
When the
.A grab_window
window becomes not viewable during an active device grab,
then the grab is released.
>>STRATEGY
Create new client, client2.
Create grab window.
Create spare window 'win' that does not overlap with the other two.
Enable events on win for client2.
Grab device.
Unmap grab_window.
Verify that grab is released by provoking device events for client2.

Re-map grab_window.
Grab device.
Verify that grab is released by provoking device events for client2.
>>CODE
Display	*client2;
Window	win;
XEvent	ev;
int dmn;
XEventClass dmnc;

	if (!Setup_Extension_DeviceInfo(ValMask| AnyMask))
	    {
	    untested("%s: No input extension key device.\n", TestName);
	    return;
	    }
	DeviceMotionNotify(Devs.Valuator, dmn, dmnc);
	(void) warppointer(display, DRW(display), 0, 0);

	client2 = opendisplay();

	grab_window = defwin(display);
	win = defwin(display);

	XSelectInput(client2, win, EnterWindowMask);
	XSelectExtensionEvent(client2, win, &dmnc, 1);
	XSync(client2, True);

	device = Devs.Valuator;
	event_list = &dmnc;
	event_count = 1;
	XCALL;

	XUnmapWindow(display, grab_window);

	/*
	 * Warp into win and force all events to be received.
	 * If the grab has been released then this will generate
	 * an event for client2.
	 */
	(void) warppointer(display, win, 0, 0);
	XSync(client2, False);

	if (XCheckWindowEvent(client2, win, PointerMotionMask|EnterWindowMask, &ev))
		PASS;
	else {
		report("Grab was not released when grab_window was unmapped");
		FAIL;
	}

	/* Clear any extra events */
	XSync(client2, True);

>>ASSERTION Bad B 3
When the
.A grab_window
is not viewable, then a call to xname fails and returns
.S GrabNotViewable .
>>STRATEGY
Create unmapped grab window.
Attempt to grab device.
Verify that xname fails and returns GrabNotViewable.
>>CODE
int 	ret;

	if (!Setup_Extension_DeviceInfo(AnyMask))
	    {
	    untested("%s: No input extension key device.\n", TestName);
	    return;
	    }
	grab_window = defwin(display);
	XUnmapWindow(display, grab_window);

	device = Devs.Any;
	ret = XCALL;
	if (ret == GrabNotViewable)
		PASS;
	else {
		report("Return value was %s, expecting GrabNotViewable", grabreplyname(ret));
		FAIL;
	}
>>ASSERTION Bad B 3
When the device is actively grabbed by some other client, then a call to xname
fails and returns
.S AlreadyGrabbed .
>>STRATEGY
Create client2.
Grab device with client2.
Attempt to grab device with default client.
Verify that xname fails and returns AlreadyGrabbed.
>>CODE
Display	*client1;
Display	*client2;
int 	ret;

	if (!Setup_Extension_DeviceInfo(AnyMask))
	    {
	    untested("%s: No input extension key device.\n", TestName);
	    return;
	    }
	if ((client2 = opendisplay()) == 0)
		return;

	grab_window = defwin(Dsp);
	if (isdeleted())
		return;

	display = client2;
	device = Devs.Any;
	XCALL;
	XSync(display,0);

	if ((client1 = opendisplay()) == 0)
		return;
	display = client1;
	ret = XCALL;
	if (ret == AlreadyGrabbed)
		CHECK;
	else {
		report("Return value was %s, expecting AlreadyGrabbed", grabreplyname(ret));
		FAIL;
	}

	CHECKPASS(1);
>>ASSERTION Bad B 3
When the device is frozen by an active grab of another client, then
a call to xname fails and returns
.S GrabFrozen .
>>STRATEGY
Create client2.
Grab another device and freeze the target device with client2.
Attempt to grab device with default client.
Verify that xname fails and returns GrabFrozen.
>>CODE
Display	*client2;
int 	ret;

	if (!Setup_Extension_DeviceInfo(AnyMask))
	    {
	    untested("%s: No input extension key device.\n", TestName);
	    return;
	    }
	if ((client2 = opendisplay()) == 0)
		return;

	grab_window = defwin(display);

	if (XGrabKeyboard(client2, grab_window, True, GrabModeSync, GrabModeSync, CurrentTime) != GrabSuccess) {
		delete("Could not freeze device by grabbing keyboard");
		return;
	}
	
	device = Devs.Any;
	ret = XCALL;
	if (ret == GrabFrozen)
		CHECK;
	else {
		report("Return value was %s, expecting GrabFrozen", grabreplyname(ret));
		FAIL;
	}

	XUngrabKeyboard (display, CurrentTime);
	CHECKPASS(1);
>>ASSERTION Bad B 3
When the specified time is earlier than the last-device-grab time or later
than the current X server time, then a call to xname fails and returns
.S GrabInvalidTime .
>>STRATEGY
Grab device with a given time.
Release grab.
Grab device with earlier time.
Verify that xname fails and returns GrabInvalidTime.
Get current server time.
Grab device with later time.
Verify that xname fails and returns GrabInvalidTime.
>>CODE
int 	ret;

	if (!Setup_Extension_DeviceInfo(AnyMask))
	    {
	    untested("%s: No input extension key device.\n", TestName);
	    return;
	    }
	grab_window = defwin(display);

	/* get time from the server */
	time = gettime(display);

	/* This sets the last-device-grab time */
	device = Devs.Any;
	XCALL;
	XUngrabDevice(display, device, time);

	time -= 100;
	ret = XCALL;
	if (ret == GrabInvalidTime)
		CHECK;
	else {
		report("Return value was %s, expecting GrabInvalidTime",
			grabreplyname(ret));
		FAIL;
	}
	XUngrabDevice(display, device, time);

	/*
	 * Get current time again and add several minutes to get a time in the
	 * future.
	 */
	time = gettime(display);
	time += ((config.speedfactor+1) * 1000000);

	ret = XCALL;
	if (ret == GrabInvalidTime)
		CHECK;
	else {
		 report("Returned valued was %s, expecting GrabInvalidTime", grabreplyname(ret));
		 FAIL;
	}

	CHECKPASS(2);
>>ASSERTION Bad B 3
If xname is invoked with an invalid event class, 
a BadClass error will result.
>>STRATEGY
Verify that xname fails and returns BadValue.
>>CODE badclass
int 	ret;
XID badclass;
XEventClass	bogus[2];

	if (!Setup_Extension_DeviceInfo(AnyMask))
	    {
	    untested("%s: No input extension key device.\n", TestName);
	    return;
	    }
	BadClass(display, badclass);
	bogus[0]=-1;
	bogus[1]=-1;
	event_list = bogus;
	event_count = 2;
	grab_window = defwin(display);

	device = Devs.Any;
	ret = XCALL;
	if (geterr() == badclass)
		PASS;
	else
		FAIL;
>>ASSERTION Bad B 3
If xname is invoked with an invalid this_devices_mode, 
a BadValue error will result.
>>STRATEGY
Verify that xname fails and returns BadValue.
>>CODE BadValue
int 	ret;

	if (!Setup_Extension_DeviceInfo(AnyMask))
	    {
	    untested("%s: No input extension key device.\n", TestName);
	    return;
	    }
	grab_window = defwin(display);
	this_device_mode = -1;

	device = Devs.Any;
	ret = XCALL;
	if (geterr() == BadValue)
		PASS;
	else
		FAIL;
>>ASSERTION Bad B 3
If xname is invoked with an invalid other_devices_mode, 
a BadValue error will result.
>>STRATEGY
Verify that xname fails and returns BadValue.
>>CODE BadValue
int 	ret;

	if (!Setup_Extension_DeviceInfo(AnyMask))
	    {
	    untested("%s: No input extension key device.\n", TestName);
	    return;
	    }
	grab_window = defwin(display);
	other_devices_mode = -1;

	device = Devs.Any;
	ret = XCALL;
	if (geterr() == BadValue)
		PASS;
	else
		FAIL;
>>ASSERTION Bad B 3
If xname is invoked with an invalid ownerEvents, a BadValue
error will result.
>>STRATEGY
Verify that xname fails and returns BadValue.
>>CODE BadValue
int 	ret;

	if (!Setup_Extension_DeviceInfo(AnyMask))
	    {
	    untested("%s: No input extension key device.\n", TestName);
	    return;
	    }
	grab_window = defwin(display);
	owner_events = -1;

	device = Devs.Any;
	ret = XCALL;
	if (geterr() == BadValue)
		PASS;
	else
		FAIL;
>>ASSERTION Bad B 3
If xname is invoked with an invalid window id, a BadWindow
error will result.
>>STRATEGY
Verify that xname fails and returns BadWindow.
>>CODE BadWindow
int 	ret;

	if (!Setup_Extension_DeviceInfo(AnyMask))
	    {
	    untested("%s: No input extension key device.\n", TestName);
	    return;
	    }
	grab_window = 0;

	device = Devs.Any;
	ret = XCALL;
	if (geterr() == BadWindow)
		PASS;
	else
		FAIL;
>>ASSERTION Bad B 3
If xname is invoked with an invalid device id, a BadDevice
error will result.
>>STRATEGY
Verify that xname fails and returns BadDevice.
>>CODE baddevice
int 	ret;
XDevice bogus;
XID baddevice;

	if (!Setup_Extension_DeviceInfo(AnyMask))
	    {
	    untested("%s: No input extension key device.\n", TestName);
	    return;
	    }
	grab_window = defwin(display);
	BadDevice(display,baddevice);

	bogus.device_id = 128;
	device = &bogus;
	ret = XCALL;
	if (geterr() == baddevice)
		PASS;
	else
		FAIL;

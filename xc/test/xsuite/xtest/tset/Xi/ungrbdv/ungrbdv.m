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
 * $XConsortium: ungrbdv.m,v 1.7 94/09/06 21:05:17 dpw Exp $
 */
>>TITLE XUngrabDevice XINPUT
void

Display	*display = Dsp;
XDevice *device;
Time time=CurrentTime;
>>EXTERN
extern int MinKeyCode;
extern ExtDeviceInfo Devs;

>>ASSERTION Good B 3
When the client has actively grabbed the  device,  then  a
call  to UngrabDevice releases the device and any queued
events.
>>STRATEGY
Create grab window.
Grab and freeze device.
If no extensions:
  Touch test xname.
else
  Press and release key.
  Call xname.
  Verify that events are released.
  Create new window.
  Verify that device events can be received on it.
>>CODE
Window	win;
int 	n;
int 	first;
XEvent	ev;
XEventClass dkclass[2];
XID dkp, dkr;

	if (!Setup_Extension_DeviceInfo(KeyMask))
	    {
	    untested("%s: No input extension key device.\n", TestName);
	    return;
	    }
	device = Devs.Key;
	DeviceKeyPress(device, dkp, dkclass[0]);
	DeviceKeyRelease(device, dkr, dkclass[1]);
	win = defwin(display);
	XSelectExtensionEvent(display, win, dkclass, 2);

	XGrabDevice(display, device, win, False, 0, NULL, GrabModeSync, GrabModeAsync, time);

	if (noext(0)) {
		XCALL;
		untested("There is no reliable test method, but a touch test was performed");
		return;
	} else
		CHECK;

	(void) warppointer(display, win, 10, 10);
	XSync(display,1);
	devicekeypress(display, Devs.Key, MinKeyCode);
	devicekeyrel(display, Devs.Key, MinKeyCode);
	XSync(display,0);

	if (XPending(display)) {
		report("Got events while device was meant to be frozen");
		FAIL;
	} else
		CHECK;

	XCALL;
	XSync(display,0);

	n = getevent(display, &ev);
	if (n != 2) {
		report("Expecting two events to be released after grab");
		report("  got %d", n);
		FAIL;
	} else {
		first = ev.type;
		(void) getevent(display, &ev);

		if (ev.type != dkp && first != dkp) {
			report("Did not get DeviceKeyPress event after releasing grab");
			FAIL;
		} else
			CHECK;
		if (ev.type != dkr && first != dkr) {
			report("Did not get DeviceKeyRelease event after releasing grab");
			FAIL;
		} else
			CHECK;
	}

	win = defwin(display);
	XSelectExtensionEvent(display, win, &dkclass[0], 1);
	(void) warppointer(display, win, 5, 5);
	devicekeypress(display, Devs.Key, MinKeyCode);
	if (XPending(display)) {
		XNextEvent(display, &ev);
		if (ev.type == dkp)
		CHECK;
	} else {
		report("Device grab was not released");
		FAIL;
	}

	CHECKPASS(5);

	devicekeyrel(display, Devs.Key, MinKeyCode);
	devicerelkeys(Devs.Key);
	restoredevstate();

>>ASSERTION Good B 3
When the specified time is earlier than	the  last-device-
grab time or is later than the current X server time, then a
call to UngrabDevice does not release the device and any
queued events.
>>STRATEGY
Get current time.
Grab device and freeze other devices with this time.
Call xname with earlier time.
Verify that the pointer is still frozen and therefore device grab is not 
released.

Get current time and add several seconds to get future time.
Call xname with this time.
Verify that the pointer is still frozen and therefore keyboard grab is not 
released.
>>EXTERN

static Status
ispfrozen()
{
Window	win;
XEvent	ev;

	win = defwin(display);
	XSelectInput(display, win, PointerMotionMask);

	(void) warppointer(display, win, 0, 0);
	(void) warppointer(display, win, 1, 1);

	if (XCheckWindowEvent(display, win, PointerMotionMask, &ev))
		return(False);
	else
		return(True);
}

>>CODE
Window	win;

	if (!Setup_Extension_DeviceInfo(KeyMask))
	    {
	    untested("%s: No input extension key device.\n", TestName);
	    return;
	    }
	device = Devs.Key;
	win = defwin(display);
	time = gettime(display);

	XGrabDevice(display, device, win, False, 0, NULL, GrabModeSync, GrabModeSync, time);

	time -= 1;
	XCALL;

	if (ispfrozen())
		CHECK;
	else {
		report("Grab released for time earlier than last-device-grab time");
		FAIL;
	}

	time = gettime(display);
	time += (config.speedfactor+1) * 1000000;

	XCALL;

	if (ispfrozen())
		CHECK;
	else {
		report("Grab release for time later than current X server time");
		FAIL;
	}

	CHECKPASS(2);
>>ASSERTION Good B 3
A call to  UngrabDevice  generates  DeviceFocusIn  and  DeviceFocusOut
events  as  though  the	focus  were to change from the grab
window to the current focus window.
>>STRATEGY
Create grab window.
Create a focus window and set focus to that window.
Grab device.
Enable events on windows.
Call xname to release device.
Verify that DeviceFocusIn and DeviceFocusOut events are received.
>>CODE
Window	grabwin;
Window	win;
Window	ofocus;
XEvent	ev;
XDeviceFocusInEvent	figood;
XDeviceFocusOutEvent	fogood;
int 	orevert;
Time	otime;
XID dfi, dfo;
XEventClass dfclass[2];

	if (!Setup_Extension_DeviceInfo(AnyMask))
	    {
	    untested("%s: Required input extension device not present.\n", TestName);
	    return;
	    }
	device = Devs.Any;
	DeviceFocusIn(device, dfi, dfclass[0]);
	DeviceFocusOut(device, dfo, dfclass[1]);
	/*
	 * Save current input focus to pose as little inconvenience as
	 * possible.
	 */
	XGetDeviceFocus(display, device, &ofocus, &orevert, &otime);

	grabwin = defwin(display);
	win = defwin(display);
	XSetDeviceFocus(display, device, win, RevertToNone, CurrentTime);
	if (isdeleted()) {
		report("Could not set up focus");
		return;
	}
	XGrabDevice(display, device, grabwin, False, 1, dfclass, GrabModeSync, GrabModeSync, time);

	XSelectExtensionEvent(display, grabwin, dfclass, 2);
	XSelectExtensionEvent(display, win, dfclass, 2);

	XCALL;

	/*
	 * Set up the expected good events.
	 */
	defsetevent(figood, display, dfi);
	figood.window = win;
	figood.mode = NotifyUngrab;
	figood.detail = NotifyNonlinear;
	figood.deviceid = device->device_id;

	defsetevent(fogood, display, dfo);
	fogood.window = grabwin;
	fogood.mode = NotifyUngrab;
	fogood.detail = NotifyNonlinear;
	fogood.deviceid = device->device_id;

	if (getevent(display, &ev) == 0 || ev.type != dfo) {
		report("Did not get expected DeviceFocusOut event");
		FAIL;
	} else
		CHECK;

	if (checkevent((XEvent*)&fogood, &ev))
		FAIL;
	else
		CHECK;

	if (getevent(display, &ev) == 0 || ev.type != dfi) {
		report("Did not get expected DeviceFocusIn event");
		FAIL;
	} else
		CHECK;

	if (checkevent((XEvent*)&figood, &ev))
		FAIL;
	else
		CHECK;

	/* Reset old focus */
	XSetDeviceFocus(display, device, ofocus, orevert, CurrentTime);
	XSync(display,0);

	CHECKPASS(4);

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
		PASS;
	else
		FAIL;

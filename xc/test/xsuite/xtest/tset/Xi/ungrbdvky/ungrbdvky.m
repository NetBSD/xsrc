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
 * $XConsortium: ungrbdvky.m,v 1.8 94/04/17 21:12:50 rws Exp $
 */
>>TITLE XUngrabDeviceKey XINPUT
void

Display	*display = Dsp;
XDevice *device;
int 	keycode = AnyKey;
unsigned int 	modifiers = 0;
XDevice *modifier_device;
Window	grab_window = defwin(display);
>>EXTERN
extern ExtDeviceInfo Devs;
extern MinKeyCode, MaxKeyCode;
>>ASSERTION Good B 3
If NULL is specified for the modifier device on a call to xname,
the X keyboard is used.
>>STRATEGY
Set up a passive grab on a key, using NULL for the modifier device.
Specify a set of modifiers.
Activate the grab.
Specify NULL for the modifier device.
>>CODE
int ret;
unsigned int mods;

	if (!Setup_Extension_DeviceInfo(KeyMask))
	    {
	    untested("%s: No input extension key device.\n", TestName);
	    return;
	    }
	if (noext(0))
	    return;
	device = Devs.Key;
	modifier_device = NULL;
	warppointer(display, grab_window, 1, 1);

	mods = wantmods(display, 2);		/* verify grab uses core kbd */
	modpress(display, mods);
	XSync (display, 0);

	modifiers = ShiftMask | LockMask;
	ret = XGrabDeviceKey(display, device, MinKeyCode, modifiers, 
		modifier_device, grab_window, False, 0, NULL, GrabModeAsync, 
		GrabModeAsync);
	if (ret == Success)
	    CHECK;
	else
	    {
	    report("Failed to establish grab using core modifiers.\n");
	    FAIL;
	    }

	devicekeypress(display, device, MinKeyCode);
	XSync(display,0);

	if (dgrabbed(device, grab_window))
	    CHECK;
	else
	    {
	    report("Failed to activate grab using core modifiers.\n");
	    FAIL;
	    }
	devicekeyrel(display, device, MinKeyCode);
	modrel(display, mods);
	XSync(display,0);

	XCALL;				/* cancel grab using core kbd */
	XSync(display,0);

	modpress(display, mods);
	XSync (display, 0);
	devicekeypress(display, device, MinKeyCode);
	XSync(display,0);

	if (dgrabbed(device, grab_window))
	    {
	    report("Failed to cancel grab using core modifiers.\n");
	    FAIL;
	    }
	else
	    CHECK;
	devicekeyrel(display, device, MinKeyCode);
	modrel(display, mods);
	devicerelkeys(device);
	XSync(display,0);
	CHECKPASS(3);
>>ASSERTION Good B 3
When the specified key/modifier combination has been grabbed by this
client, then a call to xname releases the grab.
>>STRATEGY
Grab key.
Touch test.
>>EXTERN
static Bool dgrabbed(dev, win)
	XDevice *dev;
	Window win;
	{
	int ret;
	Display *client1;

	if ((client1 = opendisplay()) == 0) {
		delete("Could not open display");
		return;
	}

	ret = XGrabDevice (client1, dev, win, False, 0, NULL, GrabModeAsync, GrabModeAsync, CurrentTime);
	XSync(display,0);
	if (ret == AlreadyGrabbed)
	    return(True);
	else
	    {
	    XUngrabDevice(client1, dev, CurrentTime);
	    XSync(client1,0);
	    return(False);
	    }
	}

>>CODE

	if (!Setup_Extension_DeviceInfo(KeyMask | ModMask))
	    {
	    untested("%s: No input extension key device.\n", TestName);
	    return;
	    }
	device = Devs.Key;
	modifier_device = Devs.Mod;
	modifiers=AnyModifier;
	XGrabDeviceKey(display, device, keycode, modifiers, device, grab_window,
		False, 0, NULL, GrabModeAsync, GrabModeAsync);

	if (noext(1))
	    {
	    report("Test results could not be verified, but a touch test was done.\n");
	    XCALL;
	    XSync(display,0);
	    return;
	    }
	warppointer(display, grab_window, 1, 1);
	devicekeypress (display, device, MinKeyCode);
	XSync (display, 0);

	if (dgrabbed(device, grab_window))
	    CHECK;
	else
	    {
	    report("Failed to activate key grab.\n");
	    FAIL;
	    }

	devicekeyrel (display, device, MinKeyCode);
	XSync (display, 0);

	XCALL;
	XSync(display,0);

	devicekeypress (display, device, MinKeyCode);
	XSync (display, 0);
	if (dgrabbed(device, grab_window))
	    {
	    FAIL;
	    report("Failed to cancel key grab.\n");
	    }
	else
	    CHECK;
	CHECKPASS(2);
	devicekeyrel (display, device, MinKeyCode);
	devicerelkeys(device);
	XSync(display,0);

>>ASSERTION Good B 3
A
.A modifiers
argument of
.S AnyModifier
releases all grabs by this client for the specified key and all possible
modifier combinations.
>>STRATEGY
Touch test using AnyModifier.
>>CODE
int ret;
unsigned int mods;

	if (!Setup_Extension_DeviceInfo(KeyMask))
	    {
	    untested("%s: No input extension key device.\n", TestName);
	    return;
	    }
	if (noext(0))
	    return;
	device = Devs.Key;
	modifier_device = NULL;
	modifiers = AnyModifier;
	warppointer(display, grab_window, 1, 1);

	modifiers = ShiftMask;
	ret = XGrabDeviceKey(display, device, AnyKey, modifiers, 
		modifier_device, grab_window, False, 0, NULL, GrabModeAsync, 
		GrabModeAsync);
	if (ret == Success)
	    CHECK;
	else
	    {
	    report("Failed to establish key grab.\n");
	    FAIL;
	    }

	modifiers = ShiftMask | LockMask;
	ret = XGrabDeviceKey(display, device, AnyKey, modifiers, 
		modifier_device, grab_window, False, 0, NULL, GrabModeAsync, 
		GrabModeAsync);
	if (ret == Success)
	    CHECK;
	else
	    {
	    report("Failed to establish second key grab.\n");
	    FAIL;
	    }

	mods = wantmods(display, 1);		/* activate first grab */
	modpress(display, mods);
	XSync (display, 0);
	devicekeypress(display, device, MinKeyCode);
	XSync(display,0);

	if (dgrabbed(device, grab_window))
	    CHECK;
	else
	    {
	    report("Failed to activate key grab.\n");
	    FAIL;
	    }
	devicekeyrel(display, device, MinKeyCode);
	modrel(display, mods);
	XSync(display,0);

	mods = wantmods(display, 2);		/* activate second grab */
	modpress(display, mods);
	XSync (display, 0);
	devicekeypress(display, device, MinKeyCode);
	XSync(display,0);

	if (dgrabbed(device, grab_window))
	    CHECK;
	else
	    {
	    report("Failed to activate second key grab.\n");
	    FAIL;
	    }
	devicekeyrel(display, device, MinKeyCode);
	modrel(display, mods);
	XSync(display,0);

	modifiers = AnyModifier;
	XCALL;
	XSync(display,0);

	mods = wantmods(display, 1);		/* activate first grab */
	modpress(display, mods);
	XSync (display, 0);
	devicekeypress(display, device, MinKeyCode);
	XSync(display,0);

	if (dgrabbed(device, grab_window))
	    {
	    FAIL;
	    report("Failed to cancel first key grab.\n");
	    }
	else
	    CHECK;
	devicekeyrel(display, device, MinKeyCode);
	modrel(display, mods);
	XSync(display,0);

	mods = wantmods(display, 2);		/* activate second grab */
	modpress(display, mods);
	XSync (display, 0);
	devicekeypress(display, device, MinKeyCode);
	XSync(display,0);

	if (dgrabbed(device, grab_window))
	    {
	    FAIL;
	    report("Failed to cancel second key grab.\n");
	    }
	else
	    CHECK;
	devicekeyrel(display, device, MinKeyCode);
	devicerelkeys(device);
	modrel(display, mods);
	CHECKPASS(6);

>>ASSERTION Good B 3
A
.A keycode
argument of
.S AnyKey
releases all grabs by this client for the specified modifiers and all keys.
>>STRATEGY
Touch test using AnyKey.
>>CODE
int ret;
unsigned int mods;

	if (!Setup_Extension_DeviceInfo(KeyMask))
	    {
	    untested("%s: No input extension key device.\n", TestName);
	    return;
	    }
	if (noext(0))
	    return;
	device = Devs.Key;
	modifier_device = NULL;
	keycode = AnyKey;
	warppointer(display, grab_window, 1, 1);

	modifiers = ShiftMask;
	ret = XGrabDeviceKey(display, device, MinKeyCode, modifiers, 
		modifier_device, grab_window, False, 0, NULL, GrabModeAsync, 
		GrabModeAsync);
	if (ret == Success)
	    CHECK;
	else
	    {
	    report("Failed to establish key grab.\n");
	    FAIL;
	    }

	modifiers = ShiftMask;
	ret = XGrabDeviceKey(display, device, MinKeyCode+1, modifiers, 
		modifier_device, grab_window, False, 0, NULL, GrabModeAsync, 
		GrabModeAsync);
	if (ret == Success)
	    CHECK;
	else
	    {
	    report("Failed to establish second key grab.\n");
	    FAIL;
	    }

	mods = wantmods(display, 1);		/* activate first grab */
	modpress(display, mods);
	XSync (display, 0);
	devicekeypress(display, device, MinKeyCode);
	XSync(display,0);

	if (dgrabbed(device, grab_window))
	    CHECK;
	else
	    {
	    report("Failed to activate first key grab.\n");
	    FAIL;
	    }
	devicekeyrel(display, device, MinKeyCode);
	modrel(display, mods);
	XSync(display,0);

	mods = wantmods(display, 1);		/* activate second grab */
	modpress(display, mods);
	XSync (display, 0);
	devicekeypress(display, device, MinKeyCode+1);
	XSync(display,0);
	if (dgrabbed(device, grab_window))
	    CHECK;
	else
	    {
	    report("Failed to activate second key grab.\n");
	    FAIL;
	    }
	devicekeyrel(display, device, MinKeyCode+1);
	modrel(display, mods);
	XSync(display,0);

	XCALL;

	mods = wantmods(display, 1);		/* activate first grab */
	modpress(display, mods);
	XSync (display, 0);
	devicekeypress(display, device, MinKeyCode);
	XSync(display,0);

	if (dgrabbed(device, grab_window))
	    {
	    report("Failed to cancel first key grab.\n");
	    FAIL;
	    }
	else
	    CHECK;
	devicekeyrel(display, device, MinKeyCode);
	modrel(display, mods);
	XSync(display,0);

	mods = wantmods(display, 1);		/* activate second grab */
	modpress(display, mods);
	XSync (display, 0);
	devicekeypress(display, device, MinKeyCode+1);
	XSync(display,0);

	if (dgrabbed(device, grab_window))
	    {
	    report("Failed to cancel second key grab.\n");
	    FAIL;
	    }
	else
	    CHECK;
	devicekeyrel(display, device, MinKeyCode+1);
	modrel(display, mods);
	devicerelkeys(device);
	CHECKPASS(6);

>>ASSERTION Good B 3
A call to xname has no effect on an active grab.
>>STRATEGY
Establish an active grab.
Verify the device is grabbed.
Call xname.
Verify the device is still grabbed.
>>CODE
int ret;

	if (!Setup_Extension_DeviceInfo(KeyMask))
	    {
	    untested("%s: Required input extension device not present.\n", TestName);
	    return;
	    }
	device = Devs.Key;
	modifier_device = NULL;

	ret = XGrabDevice (display, device, grab_window, False, 0, NULL, GrabModeAsync, GrabModeAsync, CurrentTime);
	if (dgrabbed(device, grab_window))
	    CHECK;
	else
	    {
	    report("Failed to establish active grab.\n");
	    FAIL;
	    }

	ret = XCALL;			/* attempt to cancel grab */
	XSync(display,0);

	if (dgrabbed(device, grab_window))
	    CHECK;
	else
	    {
	    report("Cancelled active grab by calling XUngrabDeviceKey.\n");
	    FAIL;
	    }
	CHECKPASS(2);
>>ASSERTION Bad B 3
When the specified keycode is not in the range
specified by min_keycode and max_keycode in the connection setup or
.S AnyKey ,
then a
.S BadValue
error occurs.
>>STRATEGY
Get min and max keycodes.
Attempt to grab key less than the minimum.
Verify that a BadValue error occurs.
If the maximum is less than 255
  Attempt to grab key greater than the maximum
  Verify a BadValue error occurs.
>>CODE BadValue

	keycode = MinKeyCode - 1;
	if (!Setup_Extension_DeviceInfo(KeyMask))
	    {
	    untested("%s: No input extension key device.\n", TestName);
	    return;
	    }
	device = Devs.Key;
	modifier_device = NULL;
	XCALL;

	if (geterr() == BadValue)
		CHECK;

	if (MaxKeyCode < 255) {
		keycode = MaxKeyCode + 1;

		XCALL;

		if (geterr() == BadValue)
			CHECK;
	} else
		CHECK;

	CHECKPASS(2);
>>ASSERTION Bad B 3
A call to xname with an invalid modifiers parameter results in a
BadValue error.
>>CODE BadValue

	if (!Setup_Extension_DeviceInfo(KeyMask))
	    {
	    untested("%s: No input extension key device.\n", TestName);
	    return;
	    }
	device = Devs.Key;
	modifier_device = NULL;
	modifiers = -1;
	XCALL;

	if (geterr() == BadValue)
		CHECK;

	CHECKPASS(1);
>>ASSERTION Bad B 3
A call to xname with an invalid window parameter results in a
BadWindow error.
>>CODE BadWindow

	if (!Setup_Extension_DeviceInfo(KeyMask))
	    {
	    untested("%s: No input extension key device.\n", TestName);
	    return;
	    }
	device = Devs.Key;
	modifier_device = NULL;
	grab_window = 0;
	XCALL;

	if (geterr() == BadWindow)
		CHECK;

	CHECKPASS(1);
>>ASSERTION Bad B 3
A call to xname specifying a device that has no keys results in a
BadMatch error.
>>CODE BadMatch

	if (!Setup_Extension_DeviceInfo(KeyMask | NKeysMask))
	    {
	    untested("%s: Required input extension device not present.\n", TestName);
	    return;
	    }
	device = Devs.NoKeys;
	modifier_device = Devs.Key;
	XCALL;

	if (geterr() == BadMatch)
		PASS;
	else
		FAIL;
>>ASSERTION Bad B 3
A call to xname specifying a modifier device that has no keys results in a
BadMatch error.
>>CODE BadMatch

	if (!Setup_Extension_DeviceInfo(KeyMask | NKeysMask))
	    {
	    untested("%s: Required input extension device not present.\n", TestName);
	    return;
	    }
	device = Devs.Key;
	modifier_device = Devs.NoKeys;
	XCALL;

	if (geterr() == BadMatch)
		CHECK;

	CHECKPASS(1);
>>ASSERTION Bad B 3
A call to xname specifying an invalid modifier device results in a
BadDevice error.
>>CODE baddevice
XID baddevice;
XDevice bogus;

	if (!Setup_Extension_DeviceInfo(KeyMask))
	    {
	    untested("%s: No input extension key device.\n", TestName);
	    return;
	    }
	BadDevice(display, baddevice);
	device = Devs.Key;
	modifier_device = &bogus;
	bogus.device_id = 128;
	XCALL;

	if (geterr() == baddevice)
	    CHECK;
	else
	    FAIL;

	CHECKPASS(1);
>>ASSERTION Bad B 3
A call to xname specifying an invalid grab device results in a
BadDevice error.
>>CODE baddevice
XID baddevice;
XDevice bogus;

	if (!Setup_Extension_DeviceInfo(KeyMask))
	    {
	    untested("%s: No input extension key device.\n", TestName);
	    return;
	    }

	BadDevice(display, baddevice);
	modifier_device = Devs.Key;
	device = &bogus;
	bogus.device_id = 128;
	XCALL;

	if (geterr() == baddevice)
	    CHECK;
	else
	    FAIL;

	CHECKPASS(1);

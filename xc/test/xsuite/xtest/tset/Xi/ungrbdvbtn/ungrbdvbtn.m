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
 * $XConsortium: ungrbdvbtn.m,v 1.9 94/04/17 21:12:51 rws Exp $
 */
>>TITLE XUngrabDeviceButton XINPUT
void

Display	*display = Dsp;
XDevice *device;
int 	button = AnyButton;
unsigned int 	modifiers = AnyModifier;
XDevice *modifier_device;
Window	grab_window = defwin(display);
>>EXTERN
extern ExtDeviceInfo Devs;

>>ASSERTION Good B 3
A constant of NULL can be specified as the modifier device.  This causes
the X keyboard to be used.
>>STRATEGY
Set up a passive grab on a button, using NULL for the modifier device.
Specify a set of modifiers.
Activate the grab.
Specify NULL for the modifier device.
>>CODE
int ret;
unsigned int mods;

	if (!Setup_Extension_DeviceInfo(BtnMask))
	    {
	    untested("%s: No input extension button device.\n", TestName);
	    return;
	    }
	if (noext(1))
	    return;
	device = Devs.Button;
	modifier_device = NULL;
	warppointer(display, grab_window, 1, 1);
	XSync(display,0);

	mods = wantmods(display, 2);		/* verify grab uses core kbd */
	modifiers = mods;
	ret = XGrabDeviceButton(display, device, Button1, modifiers, 
		modifier_device, grab_window, False, 0, NULL, GrabModeAsync, 
		GrabModeAsync);
	if (ret == Success)
	    CHECK;
	else
	    {
	    report("Failed to establish grab using core modifiers.\n");
	    FAIL;
	    }

	modpress(display, mods);
	XSync (display, 0);
	devicebuttonpress(display, Devs.Button, Button1);
	XSync(display,0);

	if (dgrabbed(device, grab_window))
	    CHECK;
	else
	    {
	    report("Failed to activate grab using core modifiers.\n");
	    FAIL;
	    }
	devicebuttonrel(display, Devs.Button, Button1);
	modrel(display, mods);
	XSync(display,0);

	XCALL;				/* cancel grab using core kbd */
	XSync(display,0);

	modpress(display, mods);
	XSync (display, 0);
	devicebuttonpress(display, Devs.Button, Button1);
	XSync(display,0);

	if (dgrabbed(device, grab_window))
	    {
	    report("Failed to cancel grab using core modifiers.\n");
	    FAIL;
	    }
	else
	    CHECK;
	devicebuttonrel(display, Devs.Button, Button1);
	devicerelbuttons(Devs.Button);
	modrel(display, mods);
	relalldev();
	XUngrabDeviceButton(display, device, Button1, modifiers, 
		modifier_device, grab_window);
	XSync(display,0);
	CHECKPASS(3);

>>ASSERTION Good B 3
When the specified button/modifier combination has been grabbed by this
client, then a call to xname releases the grab.
>>STRATEGY
Grab a button.
Activate the grab.
Try to grab the device, verify that AlreadyGrabbed is returned.
Call xname to release the grab.
Try to grab the device, verify that the grab was released.
>>EXTERN
static Bool dgrabbed(dev, win)
	XDevice *dev;
	Window win;
	{
	int ret;
	Display *client1;

/* Create client1, without causing resource registration. */
	if (config.display == (char *) NULL) {
		delete("config.display not set");
		return;
	}
	client1 = XOpenDisplay(config.display);
	if (client1 == (Display *) NULL) {
		delete("Couldn't create client1.");
		return;
	}

	ret = XGrabDevice (client1, dev, win, False, 0, NULL, GrabModeAsync, GrabModeAsync, CurrentTime);
	XSync(client1,0);
	if (ret == AlreadyGrabbed)
	    {
	    XCloseDisplay(client1);
	    return(True);
	    }
	else
	    {
	    XUngrabDevice(client1, dev, CurrentTime);
	    XSync(client1,0);
	    XCloseDisplay(client1);
	    return(False);
	    }
	}

>>CODE
int ret;

	if (!Setup_Extension_DeviceInfo(KeyMask | BtnMask))
	    {
	    untested("%s: Required input extension device not present.\n", TestName);
	    return;
	    }
	device = Devs.Button;
	modifier_device = Devs.Key;
	modifiers = AnyModifier;
	XGrabDeviceButton(display, device, button, modifiers, modifier_device, 
		grab_window, False, 0, NULL, GrabModeAsync, GrabModeAsync);

	if (noext(1))
	    {
	    report("Test results could not be verified, but a touch test was done.\n");
	    XCALL;
	    XSync(display,0);
	    return;
	    }
	warppointer(display, grab_window, 1, 1);
	devicebuttonpress (display, Devs.Button, Button1);
	XSync (display, 0);

	if (dgrabbed(device, grab_window))
	    CHECK;
	else
	    {
	    report("Failed to activate button grab.\n");
	    FAIL;
	    }

	devicebuttonrel (display, Devs.Button, Button1);
	XSync (display, 0);

	XCALL;
	XSync(display,0);

	devicebuttonpress (display, Devs.Button, Button1);
	XSync (display, 0);
	if (dgrabbed(device, grab_window))
	    {
	    FAIL;
	    report("Failed to cancel button grab.\n");
	    }
	else
	    CHECK;
	CHECKPASS(2);
	devicebuttonrel (display, Devs.Button, Button1);
	devicerelbuttons(Devs.Button);
	XSync(display,0);

>>ASSERTION Good B 3
A
.A modifiers
argument of
.S AnyModifier
releases all grabs by this client for the specified button and all possible
modifier combinations.
>>STRATEGY
Set up several grabs with different modifiers.
Verify that they can be activated.
Call xname with AnyModifier.
Verify that all grabs are released.
>>CODE
int ret;
unsigned int mods;

	if (!Setup_Extension_DeviceInfo(KeyMask | BtnMask))
	    {
	    untested("%s: Required input extension device not present.\n", TestName);
	    return;
	    }
	if (noext(1))
	    return;
	device = Devs.Button;
	modifier_device = NULL;
	warppointer(display, grab_window, 1, 1);

	mods = wantmods(display, 1);
	modifiers = mods;
	ret = XGrabDeviceButton(display, device, Button1, modifiers, 
		modifier_device, grab_window, False, 0, NULL, GrabModeAsync, 
		GrabModeAsync);
	if (ret == Success)
	    CHECK;
	else
	    {
	    report("Failed to establish button grab.\n");
	    FAIL;
	    }

	mods = wantmods(display, 2);
	modifiers = mods;
	ret = XGrabDeviceButton(display, device, Button1, modifiers, 
		modifier_device, grab_window, False, 0, NULL, GrabModeAsync, 
		GrabModeAsync);
	if (ret == Success)
	    CHECK;
	else
	    {
	    report("Failed to activate button grab.\n");
	    FAIL;
	    }

	mods = wantmods(display, 1);		/* activate first grab */
	modpress(display, mods);
	XSync (display, 0);
	devicebuttonpress(display, Devs.Button, Button1);
	XSync(display,0);

	if (dgrabbed(device, grab_window))
	    CHECK;
	else
	    {
	    report("Failed to activate button grab.\n");
	    FAIL;
	    }
	devicebuttonrel(display, Devs.Button, Button1);
	modrel(display, mods);
	XSync(display,0);

	mods = wantmods(display, 2);		/* activate second grab */
	modpress(display, mods);
	XSync (display, 0);
	devicebuttonpress(display, Devs.Button, Button1);
	XSync(display,0);

	if (dgrabbed(device, grab_window))
	    CHECK;
	else
	    {
	    report("Failed to activate second button grab.\n");
	    FAIL;
	    }
	devicebuttonrel(display, Devs.Button, Button1);
	modrel(display, mods);
	XSync(display,0);

	modifiers = AnyModifier;
	XCALL;
	XSync(display,0);

	mods = wantmods(display, 1);		/* activate first grab */
	modpress(display, mods);
	XSync (display, 0);
	devicebuttonpress(display, Devs.Button, Button1);
	XSync(display,0);

	if (dgrabbed(device, grab_window))
	    {
	    FAIL;
	    report("Failed to cancel first button grab.\n");
	    }
	else
	    CHECK;
	devicebuttonrel(display, Devs.Button, Button1);
	modrel(display, mods);
	XSync(display,0);

	mods = wantmods(display, 2);		/* activate second grab */
	modpress(display, mods);
	XSync (display, 0);
	devicebuttonpress(display, Devs.Button, Button1);
	XSync(display,0);

	if (dgrabbed(device, grab_window))
	    {
	    FAIL;
	    report("Failed to cancel second button grab.\n");
	    }
	else
	    CHECK;
	devicebuttonrel(display, Devs.Button, Button1);
	devicerelbuttons(Devs.Button);
	modrel(display, mods);
	CHECKPASS(6);

>>ASSERTION Good B 3
A
.A button
argument of
.S AnyButton
releases all grabs by this client for the specified modifiers and all buttons.
>>STRATEGY
Touch test using AnyButton.
>>CODE
int ret;
unsigned int mods;

	if (!Setup_Extension_DeviceInfo(KeyMask | BtnMask))
	    {
	    untested("%s: Required input extension device not present.\n", TestName);
	    return;
	    }
	if (noext(1))
	    return;
	device = Devs.Button;
	modifier_device = NULL;
	button = AnyButton;
	warppointer(display, grab_window, 1, 1);

	mods = wantmods(display, 1);
	modifiers = mods;
	ret = XGrabDeviceButton(display, device, Button1, modifiers, 
		modifier_device, grab_window, False, 0, NULL, GrabModeAsync, 
		GrabModeAsync);
	if (ret == Success)
	    CHECK;
	else
	    {
	    report("Failed to establish button grab.\n");
	    FAIL;
	    }

	ret = XGrabDeviceButton(display, device, Button2, modifiers, 
		modifier_device, grab_window, False, 0, NULL, GrabModeAsync, 
		GrabModeAsync);
	if (ret == Success)
	    CHECK;
	else
	    {
	    report("Failed to establish second button grab.\n");
	    FAIL;
	    }

	modpress(display, mods);
	XSync (display, 0);
	devicebuttonpress(display, Devs.Button, Button1);
	XSync(display,0);

	if (dgrabbed(device, grab_window))
	    CHECK;
	else
	    {
	    report("Failed to activate first button grab.\n");
	    FAIL;
	    }
	devicebuttonrel(display, Devs.Button, Button1);
	modrel(display, mods);
	XSync(display,0);

	modpress(display, mods);
	XSync (display, 0);
	devicebuttonpress(display, Devs.Button, Button2);
	XSync(display,0);
	if (dgrabbed(device, grab_window))
	    CHECK;
	else
	    {
	    report("Failed to activate second button grab.\n");
	    FAIL;
	    }
	devicebuttonrel(display, Devs.Button, Button2);
	modrel(display, mods);
	XSync(display,0);

	XCALL;

	modpress(display, mods);
	XSync (display, 0);
	devicebuttonpress(display, Devs.Button, Button1);
	XSync(display,0);

	if (dgrabbed(device, grab_window))
	    {
	    report("Failed to cancel first button grab.\n");
	    FAIL;
	    }
	else
	    CHECK;
	devicebuttonrel(display, Devs.Button, Button1);
	modrel(display, mods);
	XSync(display,0);

	modpress(display, mods);
	XSync (display, 0);
	devicebuttonpress(display, Devs.Button, Button2);
	XSync(display,0);

	if (dgrabbed(device, grab_window))
	    {
	    report("Failed to cancel second button grab.\n");
	    FAIL;
	    }
	else
	    CHECK;
	devicebuttonrel(display, Devs.Button, Button2);
	devicerelbuttons(Devs.Button);
	modrel(display, mods);
	XSync(display,0);
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

	if (!Setup_Extension_DeviceInfo(BtnMask))
	    {
	    untested("%s: Required input extension device not present.\n", TestName);
	    return;
	    }
	device = Devs.Button;
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
	    report("Cancelled active grab by calling XUngrabDeviceButton.\n");
	    FAIL;
	    }
	CHECKPASS(2);
>>ASSERTION Bad B 3
A call to xname specifying an invalid set of modifiers results in a 
BadValue error.
>>CODE BadValue

	if (!Setup_Extension_DeviceInfo(BtnMask))
	    {
	    untested("%s: Required input extension device not present.\n", TestName);
	    return;
	    }
	device = Devs.Button;
	modifier_device = NULL;
 	modifiers = -1;
	XCALL;
	XSync(display,0);

	if (geterr() == BadValue)
	    CHECK;
	else
	    FAIL;

	CHECKPASS(1);

>>ASSERTION Bad B 3
A call to xname specifying an invalid grab window results in a 
BadWindow error.
>>CODE BadWindow


	if (!Setup_Extension_DeviceInfo(BtnMask))
	    {
	    untested("%s: Required input extension device not present.\n", TestName);
	    return;
	    }
	device = Devs.Button;
	modifier_device = NULL;
	grab_window = 0;
	XCALL;
	XSync(display,0);

	if (geterr() == BadWindow)
	    CHECK;
	else
	    FAIL;

	CHECKPASS(1);

>>ASSERTION Bad B 3
A call to xname specifying a device with no buttons results in a 
BadMatch error.
>>CODE BadMatch

	if (!Setup_Extension_DeviceInfo(NBtnsMask | KeyMask))
	    {
	    untested("%s: Required input extension device not present.\n", TestName);
	    return;
	    }
	device = Devs.NoButtons;
	modifier_device = Devs.Key;
	XCALL;
	XSync(display,0);

	if (geterr() == BadMatch)
	    CHECK;
	else
	    FAIL;

	CHECKPASS(1);
>>ASSERTION Bad B 3
A call to xname specifying an modifier device with no keys results in a 
BadMatch error.
>>CODE BadMatch

	if (!Setup_Extension_DeviceInfo(NKeysMask | BtnMask))
	    {
	    untested("%s: Required input extension device not present.\n", TestName);
	    return;
	    }
	device = Devs.Button;
	modifier_device = Devs.NoKeys;
	XCALL;
	XSync(display,0);

	if (geterr() == BadMatch)
	    CHECK;
	else
	    FAIL;

	CHECKPASS(1);
>>ASSERTION Bad B 3
A call to xname specifying an invalid device results in a 
BadDevice error.
>>CODE baddevice
XID baddevice, savedevice;

	if (!Setup_Extension_DeviceInfo(BtnMask))
	    {
	    untested("%s: Required input extension device not present.\n", TestName);
	    return;
	    }
	BadDevice(display, baddevice);
	device = Devs.Button;
	savedevice = device->device_id;
	device->device_id = -1;
	modifier_device = Devs.Key;

	XCALL;
	XSync(display,0);

	if (geterr() == baddevice)
	    CHECK;
	else 
	    FAIL;

	CHECKPASS(1);
	device->device_id = savedevice;
>>ASSERTION Bad B 3
A call to xname specifying an invalid modifier device results in a 
BadDevice error.
>>CODE baddevice
XID baddevice;
XDevice bogus;

	if (!Setup_Extension_DeviceInfo(BtnMask))
	    {
	    untested("%s: Required input extension device not present.\n", TestName);
	    return;
	    }
	BadDevice(display, baddevice);
	device = Devs.Button;
	modifier_device = &bogus;
	bogus.device_id = 128;

	XCALL;
	XSync(display,0);

	if (geterr() == baddevice)
	    CHECK;
	else 
	    FAIL;

	CHECKPASS(1);

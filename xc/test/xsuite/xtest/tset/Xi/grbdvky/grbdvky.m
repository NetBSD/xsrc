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
 * $XConsortium: grbdvky.m,v 1.7 94/04/17 21:12:41 rws Exp $
 */
>>TITLE XGrabDeviceKey XINPUT
void

Display	*display = Dsp;
XDevice *device;
int 	keycode = AnyKey;
unsigned int 	modifiers = AnyModifier;
XDevice *modifier_device;
Window	grab_window = defwin(display);
Bool	owner_events = False;
int 	event_count = 0;
XEventClass *event_list;
int 	this_device_mode = GrabModeAsync;
int 	other_devices_mode = GrabModeAsync;
>>EXTERN
extern int MinKeyCode;
extern ExtDeviceInfo Devs;

>>ASSERTION Good B 3
A call to xname establishes a passive grab on the key device that is activated
in the future by
the specified key being logically pressed,
the specified modifier keys being logically down,
no other modifier keys being logically down,
the
.A grab_window
being the focus window or an ancestor of the focus window
or being a descendant of the focus window that contains the pointer
and
a passive grab on the same key combination not existing on any
ancestor of
.A grab_window .
>>STRATEGY
Touch test.
>>CODE
int ret;
Display *client1;

	if (!Setup_Extension_DeviceInfo(KeyMask))
	    {
	    untested("%s: Input extension key device not present.\n", TestName);
	    return;
	    }
	if ((client1 = opendisplay()) == 0) {
		delete("Could not open display");
		return;
	}
	device = Devs.Key;
	modifier_device = NULL;
	XCALL;
	ret = XCALL;
	XSync(display,0);

	if (noext(1))
	    return;
	warppointer(display, grab_window, 1, 1);
	devicekeypress (display, Devs.Key, MinKeyCode);
	XSync (display, 0);

	ret = XGrabDevice(client1, Devs.Key, grab_window, True, 0, NULL,
	    GrabModeAsync, GrabModeAsync, CurrentTime);
	if (ret != AlreadyGrabbed)
	    FAIL;
	else
	    PASS;
	devicekeyrel (display, Devs.Key, MinKeyCode);
	devicerelkeys (Devs.Key);
	XSync (display, 0);

>>ASSERTION Good B 3
When the conditions for activating the grab are otherwise satisfied
and the key device is already grabbed,
then no active grab is established.
>>STRATEGY
Establish an active grab on the target device.  Then establish a passive
grab from another client.  Press a key to activate the grab.
If the passive grab worked, the client that established the passive grab
should be able to replace it with an active grab.  If it failed, the active
grab should also fail with an error of AlreadyGrabbed.
>>CODE
int ret;
Display *client1;

	if (!Setup_Extension_DeviceInfo(KeyMask))
	    {
	    untested("%s: Required input extension devices not present.\n", TestName);
	    return;
	    }
	if ((client1 = opendisplay()) == 0) {
		delete("Could not open display");
		return;
	}
	device = Devs.Key;
	modifier_device = Devs.Key;

	ret = XGrabDevice(client1, device, grab_window, True, 0, NULL,
	    GrabModeAsync, GrabModeAsync, CurrentTime);
	if (ret != Success)
	    FAIL;
	else
	    CHECK;

	ret = XCALL;
	if (ret != Success)
	    FAIL;
	else
	    CHECK;

	if (noext(1))
	    {
	    untested("Test extension not present - touch test only.\n");
	    return;
	    }
	warppointer(display, grab_window, 1, 1);
	devicekeypress (display, device, MinKeyCode);
	XSync (display, 0);

	ret = XGrabDevice(display, device, grab_window, True, 0, NULL,
	    GrabModeAsync, GrabModeAsync, CurrentTime);
	if (ret != AlreadyGrabbed)
	    FAIL;
	else
	    CHECK;
	CHECKPASS(3);
	devicekeyrel (display, device, MinKeyCode);
	devicerelkeys (device);
	XSync(display,0);

>>ASSERTION Good B 3
When the conditions for activating the grab are satisfied
and the grab subsequently becomes active, then
the last-device-grab time is set to the time at which the key was pressed.
>>STRATEGY
If extensions are available:
  Set and activate grab.
  Check activated.
  Check activating event received.
  Check event type and event window are DeviceKeyPress and grab_window.
  Attempt UngrabDevice at time just before event time.
  Check no longer grabbed.
  Release grab and key.
else
  Report untested.
>>CODE
Display *client1;
XEvent	ev;
XID	dkp, dkr;
XEventClass dkpclass, dkrclass;
XDeviceKeyPressedEvent	good;
XWindowAttributes	atts;
int 	ret, n;

	if (noext(1))
		return;
	else
		CHECK;

	if (!Setup_Extension_DeviceInfo(KeyMask))
	    {
	    untested("%s: Required input extension devices not present.\n", TestName);
	    return;
	    }

	if ((client1 = opendisplay()) == 0) {
		delete("Could not open display");
		return;
	}
	device = Devs.Key;
	XSync(display, True);
	XCALL;

	DeviceKeyPress(device, dkp, dkpclass);
	DeviceKeyRelease(device, dkr, dkrclass);
	XSelectExtensionEvent(display, grab_window, &dkpclass, 1);

	(void) warppointer(display, grab_window, 1, 1);
	XSync(display, True);	/* Discard any events */
	devicekeypress(display, device, MinKeyCode);
	XSync(display, False);

	ret = XGrabDevice(client1, Devs.Key, grab_window, True, 0, NULL,
	    GrabModeAsync, GrabModeAsync, CurrentTime);
	if (ret != AlreadyGrabbed)
	    {
	    report("Failed to activate grab, ret was %d.\n",ret);
	    FAIL;
	    }
	else
	    PASS;

	n = getevent(display, &ev);
	if (n)
		CHECK;
	else {
		report("No events received for activating grab");
		XUngrabDeviceKey (display, device, AnyKey, AnyModifier, NULL, grab_window);
		devicekeyrel(display, device, MinKeyCode);
		devicerelkeys(device);
		FAIL;
		return;
	}

	XGetWindowAttributes(display, grab_window, &atts);
	defsetevent(good, display, dkp);
	good.window = grab_window;
	good.root = DRW(display);
	good.subwindow = None;
	good.time = ((XDeviceKeyPressedEvent*)&ev)->time;
	good.x = 1;
	good.y = 1;
	good.x_root = good.x + atts.x + atts.border_width;
	good.y_root = good.y + atts.y + atts.border_width;
	good.state = modifiers;
	good.keycode = MinKeyCode;
	good.same_screen = True;
	good.deviceid = Devs.Key->device_id;

	if (check_ext_event((XEvent*)&good, &ev) == 0)
		CHECK;
	else
		FAIL;

	trace("Grabbed at time 0x%lx.",(unsigned long) good.time);
	XUngrabDevice(display, device, good.time - 1);
	XSync(display,0);
	ret = XGrabDevice(client1, Devs.Key, grab_window, True, 0, NULL,
	    GrabModeAsync, GrabModeAsync, CurrentTime);
	if (ret != AlreadyGrabbed)
	    {
	    report("Last device grab time set earlier than reported event time.\n");
	    FAIL;
	    }
	else
	    CHECK;

	XUngrabDevice(display, device, good.time);
	XSync(display,0);
	ret = XGrabDevice(client1, Devs.Key, grab_window, True, 0, NULL,
	    GrabModeAsync, GrabModeAsync, CurrentTime);
	if (ret == AlreadyGrabbed)
	    {
	    report("Last device grab time set later than reported event time.\n");
	    FAIL;
	    }
	else
	    CHECK;
	XUngrabDeviceKey (display, device, AnyKey, AnyModifier, NULL, grab_window);
	devicekeyrel(display, device, MinKeyCode);
	devicerelkeys(device);
	CHECKPASS(5);

>>ASSERTION Good B 3
When the grab subsequently becomes active and later
the logical state of the
device has the specified key released,
then the active grab is terminated automatically.
>>#(independent of the logical state of the modifier keys).
>>STRATEGY
If extensions are available:
  Place passive grab with xname.
  Activate grab with simulated device events.
  Simulate pressing some modifier keys.
  Release the key.
  Verify that the grab has been released.
else
  Report untested.
>>CODE
unsigned int 	mods, ret;
Display *client1;

	if (noext(1))
		return;

	if ((client1 = opendisplay()) == 0) {
		delete("Could not open display");
		return;
	}
	if (!Setup_Extension_DeviceInfo(KeyMask))
	    {
	    untested("%s: Required input extension devices not present.\n", TestName);
	    return;
	    }
	device = Devs.Key;
	XCALL;

	devicekeypress(display, device, MinKeyCode);
	mods = wantmods(display, 3);
	modpress(display, mods);
	XSync(display,0);

	devicekeyrel(display, device, MinKeyCode);
	XSync(display,0);

	ret = XGrabDevice(client1, device, grab_window, True, 0, NULL,
	    GrabModeAsync, GrabModeAsync, CurrentTime);
	if (ret != Success)
	    {
	    report("Grab was not released, when key was released and ");
	    report("  modifier keys were down");
	    FAIL;
	    }
	else
	    PASS;
	modrel(display, mods);
	devicerelkeys(device);
	relalldev();

>>ASSERTION Good B 3
A call to xname overrides all previous passive grabs by the same client on the
same key combinations on the same window.
>>STRATEGY
If extensions are available:
  Place a passive grab with this_device_mode = GrabModeSync.
  Place a passive grab as before but with this_device_mode = GrabModeAsync.
  Move pointer to grab_window and activate grab.
  Verify that the device is not frozen, and thus the second
  grab overrode the first.
else
  Report untested.
>>CODE
XID	dkp, dkr;
XEventClass dkpclass, dkrclass;
XEvent ev;

	if (noext(1))
		return;

	if (!Setup_Extension_DeviceInfo(KeyMask))
	    {
	    untested("%s: Required input extension devices not present.\n", TestName);
	    return;
	    }
	device = Devs.Key;
	DeviceKeyPress(device, dkp, dkpclass);
	DeviceKeyRelease(device, dkr, dkrclass);
	XSelectExtensionEvent(display, grab_window, &dkpclass, 1);
	XSync(display,0);
	warppointer(display, grab_window, 1, 1);

	this_device_mode = GrabModeSync;
	XCALL;

	/* Try to override first grab */
	this_device_mode = GrabModeAsync;
	XCALL;
	XSync(display,0);

	devicekeypress(display, device, MinKeyCode);
	devicekeyrel(display, device, MinKeyCode);
	devicekeypress(display, device, MinKeyCode+1);
	devicekeyrel(display, device, MinKeyCode+1);
	devicekeypress(display, device, MinKeyCode+2);
	devicekeyrel(display, device, MinKeyCode+2);
	XSync(display,0);

	while (XPending(display)) 
	    {
	    XNextEvent(display, &ev);
	    if (ev.type==dkp)
		CHECK;
	    else
		FAIL;
	    }

	devicerelkeys(device);
	CHECKPASS(3);
>>ASSERTION Good B 3
When the
.A modifiers
argument is
.S AnyModifier ,
then this is equivalent to separate calls to xname for all
possible modifier combinations including no modifiers.
>>STRATEGY
If extensions are available:
  Place passive grab with a modifiers of AnyModifier.
  Press a bunch of modifier keys.
  Press key to activate grab.
  Verify that grab is activated.
  Release keys.

  Press key (no modifiers).
  Verify that grab is active.
else
  Perform touch test.
  Report untested.
>>CODE
unsigned int 	ret, mods;
Display *client1;

	if (!Setup_Extension_DeviceInfo(KeyMask))
	    {
	    untested("%s: Required input extension devices not present.\n", TestName);
	    return;
	    }
	if ((client1 = opendisplay()) == 0) {
		delete("Could not open display");
		return;
	}
	modifiers = AnyModifier;
	device = Devs.Key;
	modifier_device = NULL;
	XCALL;

	if (noext(1)) {
		untested("There is no reliable test method, but a touch test was performed");
		return;
	}

	mods = wantmods(display, 4);
	modpress(display, mods);

	/*
	 * modifiers was AnyModifier, several modifier keys are held down.
	 */
	warppointer(display, grab_window, 1, 1);
	devicekeypress(display, device, MinKeyCode);
	XSync(display,0);
	ret = XGrabDevice(client1, device, grab_window, True, 0, NULL,
	    GrabModeAsync, GrabModeAsync, CurrentTime);
	if (ret != AlreadyGrabbed) 
	    {
	    report("Grab not activated for AnyModifier");
	    report("  Modifiers used %s", keymaskname((unsigned long)mods));
	    FAIL;
	    }
	else
	    CHECK;

	/* Release all keys and modifiers */
	devicekeyrel(display, device, MinKeyCode);
	relalldev();
	XSync(display,0);

	ret = XGrabDevice(client1, device, grab_window, True, 0, NULL,
	    GrabModeAsync, GrabModeAsync, CurrentTime);
	if (ret == AlreadyGrabbed) 
	    {
	    delete("Could not release grab for second part of test");
	    FAIL;
	    }
	else
	    {
	    XUngrabDevice(client1, device, CurrentTime);
	    XSync(client1,0);
	    CHECK;
	    }

	devicekeypress(display, device, MinKeyCode);
	XSync(display,0);
	ret = XGrabDevice(client1, device, grab_window, True, 0, NULL,
	    GrabModeAsync, GrabModeAsync, CurrentTime);
	if (ret != AlreadyGrabbed) 
	    {
	    report("Grab with AnyModifier was not activated by a key press with");
	    report("  no modifiers");
	    FAIL;
	    }
	else
	    CHECK;
	CHECKPASS(3);
	devicekeyrel(display, device, MinKeyCode);
	devicerelkeys(device);
	XSync(display,0);
>>ASSERTION Good B 3
It is not required that all modifiers specified have
currently assigned KeyCodes.
>>STRATEGY
If extensions are available:
  Get a modifier mask.
  Remove the keycode for the modifier from the map.
  Call xname to set up a passive grab with that modifier.
  Reset the keycode in the modifier map.
  Verify that the grab can be activated with the newly set modifier.
else
  Report untested.
>>CODE
XModifierKeymap	*mmap;
XModifierKeymap	*newmap;
int 	i, ret;
Display *client1;

	if (noext(1))
		return;

	if (!Setup_Extension_DeviceInfo(KeyMask))
	    {
	    untested("%s: Required input extension devices not present.\n", TestName);
	    return;
	    }
	if ((client1 = opendisplay()) == 0) {
		delete("Could not open display");
		return;
	}
	device = Devs.Key;
	modifier_device = NULL;
	modifiers = wantmods(display, 1);
	if (modifiers == 0) {
		untested("No available modifier keys");
		return;
	} else
		CHECK;

	mmap = XGetModifierMapping(display);
	if (mmap == NULL) {
		delete("Could not get modifier map");
		return;
	} else
		CHECK;

	/*
	 * Remove all the modifiers mappings.
	 */
	newmap = XNewModifiermap(mmap->max_keypermod);
	for (i = 0; i < newmap->max_keypermod*8; i++)
		newmap->modifiermap[i] = NoSymbol;

	if (XSetModifierMapping(display, newmap) == MappingSuccess)
		CHECK;
	else {
		delete("Could not remove modifier mapping");
		return;
	}

	/*
	 * Now we have a modifier that has no keycode - set up a passive grab.
	 */
	XCALL;
	XSync(display,0);

	/*
	 * Reset the modifier map, and try to activate the grab.
	 */
	if (XSetModifierMapping(display, mmap) == MappingSuccess)
		CHECK;
	else {
		delete("Could not reset modifier mapping");
		return;
	}

	warppointer(display, grab_window, 1, 1);
	XSync(display,0);
	modpress(display, modifiers);
	devicekeypress(display, device, MinKeyCode);
	XSync(display,0);

	ret = XGrabDevice(client1, device, grab_window, True, 0, NULL,
	    GrabModeAsync, GrabModeAsync, CurrentTime);
	if (ret != AlreadyGrabbed) 
	    {
	    report("Passive grab not set when the modifier did not have a current keycode");
	    FAIL;
	    }
	else
	    CHECK;

	CHECKPASS(5);
	devicekeyrel(display, device, MinKeyCode);
	devicerelkeys(device);
	relalldev();
>>ASSERTION Good B 3
When the
.A keycode
argument is
.S AnyKey ,
then this is equivalent to separate calls to xname for
all possible KeyCodes.
>>STRATEGY
Establish a passive grab for AnyKey on an input extension device.
Press each of the keys on that device.
Verify that pressing each of the keys activates the grab.
>>CODE
int i, j, ret, ndevices, nkeys, count=0;
Display *client1;
XDeviceInfo *list;
XAnyClassPtr any;

	if (!Setup_Extension_DeviceInfo(KeyMask))
	    {
	    untested("%s: Required input extension devices not present.\n", TestName);
	    return;
	    }
	if ((client1 = opendisplay()) == 0) {
		delete("Could not open display");
		return;
	}
	device = Devs.Key;
	list = XListInputDevices(display, &ndevices);
	for (i=0; i<ndevices; i++, list++)
	    if (list->id == Devs.Key->device_id)
		{
		any = (XAnyClassPtr) (list->inputclassinfo);
		for (j=0; j<list->num_classes; j++)
		    {
		    if (any->class == KeyClass)
			{
			nkeys = ((XKeyInfo *) any)->num_keys;
			break;
			}
		    any = (XAnyClassPtr) ((char *) any + any->length);
		    }
		break;
		}
	modifier_device = NULL;
	warppointer(display, grab_window, 1, 1);
	for (i=0;i<nkeys;i++)
	    {
	    ret = XCALL;
	    XSync(display,0);

	    if (noext(1))
		return;
	    devicekeypress (display, device, MinKeyCode + i);
	    XSync (display, 0);

	    ret = XGrabDevice(client1, device, grab_window, True, 0, NULL,
		GrabModeAsync, GrabModeAsync, CurrentTime);
	    if (ret != AlreadyGrabbed)
		FAIL;
	    else
		{
		CHECK;
		count++;
		}
	    XUngrabDeviceKey(display, device, AnyKey, AnyModifier,
		NULL, grab_window);
	    XSync(display,0);
	    }
	for (i=0;i<nkeys;i++)
	    devicekeyrel(display, device, MinKeyCode + i);
	devicerelkeys(device);
	CHECKPASS(count);
>>ASSERTION Good B 3
When the event window for an active grab becomes not viewable, then the
grab is released automatically.
>>STRATEGY
Establish a passive grab on an input device extension device.
Activate the grab by pressing a key on the device.
Verify that the grab is active by trying to establish another active grab
from a different client, and verifying that AlreadyGrabbed is returned.
Make the grab window non-viewable.
Attempt another active grab and verify that it works this time.
>>CODE
int ret;
Display *client1;

	if (!Setup_Extension_DeviceInfo(KeyMask))
	    {
	    untested("%s: Required input extension devices not present.\n", TestName);
	    return;
	    }
	if ((client1 = opendisplay()) == 0) {
		delete("Could not open display");
		return;
	}
	device = Devs.Key;
	modifier_device = NULL;
	ret = XCALL;
	XSync(display,0);

	if (noext(1))
	    return;
	warppointer(display, grab_window, 1, 1);
	devicekeypress (display, device, MinKeyCode);
	XSync (display, 0);

	ret = XGrabDevice(client1, device, grab_window, True, 0, NULL,
	    GrabModeAsync, GrabModeAsync, CurrentTime);
	if (ret != AlreadyGrabbed)
	    FAIL;
	else
	    CHECK;
	XUnmapWindow(display,grab_window);
	XSync(display,0);

	grab_window = defwin(display);
	ret = XGrabDevice(client1, device, grab_window, True, 0, NULL,
	    GrabModeAsync, GrabModeAsync, CurrentTime);
	if (ret != Success)
	    FAIL;
	else
	    CHECK;
	devicekeyrel (display, device, MinKeyCode);
	devicerelkeys(device);
	XSync(display,0);
	CHECKPASS(2);
>>ASSERTION Good B 3
NULL may be specified as the modifier device.  This will cause the X keyboard
to be used as the modifier device.
>>STRATEGY
Specify NULL as the modifier device.
>>CODE
Display *client1;
int minkey, maxkey, numkeys, ret;

	if (!Setup_Extension_DeviceInfo(KeyMask))
	    {
	    untested("%s: Input extension key device not present.\n", TestName);
	    return;
	    }
	if ((client1 = opendisplay()) == 0) {
		delete("Could not open display");
		return;
	}
	device = Devs.Key;
	modifier_device = NULL;
	modifiers = wantmods(display, 1);
	if (modifiers == 0) {
		untested("No available modifier keys");
		return;
	} else
		CHECK;
	XCALL;
	if (noext(1))
	    {
	    untested("%d: No XTest extension, can't complete test.",TestName);
	    return;
	    }
	MinMaxKeys(display, device, &minkey, &maxkey, &numkeys);
	warppointer(display, grab_window, 1, 1);
	modpress(display, modifiers);
	devicekeypress (display, Devs.Key, minkey);
	XSync (display, 0);

	ret = XGrabDevice(client1, Devs.Key, grab_window, True, 0, NULL,
	    GrabModeAsync, GrabModeAsync, CurrentTime);
	if (ret != AlreadyGrabbed)
	    {
	    report("%s: Grab failed to activate.", TestName);
	    FAIL;
	    }
	else
	    CHECK;

	modrel(display, modifiers);
	devicekeyrel (display, Devs.Key, minkey);
	devicerelkeys (Devs.Key);
	relalldev();
	XUngrabDeviceKey(display, device, AnyKey, AnyModifier,
		NULL, grab_window);
	CHECKPASS(2);

>>ASSERTION Bad B 3
When the specified keycode is not in the range
specified by min_keycode and max_keycode in the connection setup or
.S AnyKey ,
then a
.S BadValue
error occurs.
>>STRATEGY
Call xname with keycode less than min_keycode.
Verify that a BadValue error occurs.
Call xname with keycode greater than max_keycode if it is less than 255.
Verify that a BadValue error occurs.
>>CODE BadValue
extern int 	MinKeyCode, MaxKeyCode;

	if (!Setup_Extension_DeviceInfo(KeyMask))
	    {
	    untested("%s: Input extension key device not present.\n", TestName);
	    return;
	    }
	device = Devs.Key;
	modifier_device = Devs.Key;

	keycode = MinKeyCode-1;
	XCALL;

	if (geterr() == BadValue)
		CHECK;

	/*
	 * Since the protocol only has one byte for the key then this
	 * assertion cannot be tested when max_keycode is 255.
	 */
	if (MaxKeyCode < 255) {

		keycode = MaxKeyCode+1;

		XCALL;
		XSync(display,0);

		if (geterr() == BadValue)
			CHECK;
	} else
		CHECK;

	CHECKPASS(2);
>>ASSERTION Bad B 3
When an invalid value is specified for the modifiers parameter,
a BadValue error results.
>>CODE BadValue

	if (!Setup_Extension_DeviceInfo(KeyMask))
	    {
	    untested("%s: Input extension key device not present.\n", TestName);
	    return;
	    }
	device = Devs.Key;
	modifier_device = Devs.Key;

	modifiers = ~0;
	XCALL;
	XSync(display,0);

	if (geterr() == BadValue)
	    CHECK;
	else
	    FAIL;
	CHECKPASS(1);
>>ASSERTION Bad B 3
When an invalid value is specified for the owner_events parameter, a
BadValue error results.
>>CODE BadValue

	if (!Setup_Extension_DeviceInfo(KeyMask))
	    {
	    untested("%s: Input extension key device not present.\n", TestName);
	    return;
	    }
	device = Devs.Key;
	modifier_device = Devs.Key;

	owner_events = -1;
	XCALL;
	XSync(display,0);

	if (geterr() == BadValue)
	    CHECK;
	else
	    FAIL;
	CHECKPASS(1);
>>ASSERTION Bad B 3
When an invalid value is specified for the this_device_mode parameter, a
BadValue error results.
>>CODE BadValue

	if (!Setup_Extension_DeviceInfo(KeyMask))
	    {
	    untested("%s: Input extension key device not present.\n", TestName);
	    return;
	    }
	device = Devs.Key;
	modifier_device = Devs.Key;

	this_device_mode = -1;
	XCALL;

	if (geterr() == BadValue)
	    CHECK;
	else
	    FAIL;
	CHECKPASS(1);
>>ASSERTION Bad B 3
When an invalid value is specified for the other_devices_mode parameter, a
BadValue error results.
>>CODE BadValue

	if (!Setup_Extension_DeviceInfo(KeyMask))
	    {
	    untested("%s: Input extension key device not present.\n", TestName);
	    return;
	    }
	device = Devs.Key;
	modifier_device = Devs.Key;

	other_devices_mode = -1;
	XCALL;

	if (geterr() == BadValue)
	    CHECK;
	else
	    FAIL;
	CHECKPASS(1);
>>ASSERTION Bad B 3
.ER Access grab
>>STRATEGY
Grab key/modifier.
Create client2.
Attempt to grab same key modifier for client2.
Verify BadAccess error.
>>CODE BadAccess
Display	*client2;

	if (!Setup_Extension_DeviceInfo(KeyMask))
	    {
	    untested("%s: Input extension key device not present.\n", TestName);
	    return;
	    }
	device = Devs.Key;
	modifier_device = Devs.Key;

	XGrabDeviceKey(Dsp, device, keycode, modifiers, modifier_device,
		grab_window, owner_events, event_count, event_list,
		this_device_mode, other_devices_mode);
	XSync (Dsp,0);
	if (isdeleted()) {
		delete("Could not set up initial grab");
		return;
	}

	if ((client2 = opendisplay()) == NULL)
		return;

	display = client2;
	XCALL;
	XSync (Dsp,0);

	if (geterr() == BadAccess)
		CHECK;
	else
		FAIL;

	CHECKPASS(1);
>>ASSERTION Bad B 3
When an invalid window parameter is specified on an xname protocol request,
a BadWindow error results.
>>CODE BadWindow

	if (!Setup_Extension_DeviceInfo(KeyMask))
	    {
	    untested("%s: Input extension key device not present.\n", TestName);
	    return;
	    }
	device = Devs.Key;
	modifier_device = Devs.Key;

	grab_window = 0;
	XCALL;
	XSync (Dsp,0);

	if (geterr() == BadWindow)
	    CHECK;
	else
	    FAIL;
	CHECKPASS(1);
>>ASSERTION Bad B 3
When an modifier_device is specified that has no keys, a
BadMatch error results.
>>CODE BadMatch

	if (!Setup_Extension_DeviceInfo(KeyMask | NKeysMask))
	    {
	    untested("%s: Required input extension devices not present.\n", TestName);
	    return;
	    }
	device = Devs.Key;
	modifier_device = Devs.NoKeys;
	XCALL;
	XSync (Dsp,0);

	if (geterr() == BadMatch)
	    CHECK;
	else
	    FAIL;

	CHECKPASS(1);
>>ASSERTION Bad B 3
When an invalid modifier_device is specified in an xname protocol request, a
BadValue error results.
>>CODE baddevice
XID baddevice;
XDevice bogus;

	if (!Setup_Extension_DeviceInfo(KeyMask))
	    {
	    untested("%s: Input extension key device not present.\n", TestName);
	    return;
	    }
	BadDevice(display, baddevice);
	device = Devs.Key;
	modifier_device = &bogus;
	bogus.device_id = 128;
	XCALL;
	XSync (Dsp,0);

	if (geterr() == baddevice)
	    CHECK;
	else
	    FAIL;

	CHECKPASS(1);
>>ASSERTION Bad B 3
When an invalid device is specified in an xname protocol request, a
BadValue error results.
>>CODE baddevice
XID baddevice;
XDevice bogus;

	if (!Setup_Extension_DeviceInfo(KeyMask))
	    {
	    untested("%s: Input extension key device not present.\n", TestName);
	    return;
	    }
	BadDevice(display, baddevice);
	modifier_device = Devs.Key;
	device = &bogus;
	bogus.device_id = 128;
	XCALL;
	XSync (Dsp,0);

	if (geterr() == baddevice)
	    CHECK;
	else
	    FAIL;

	CHECKPASS(1);
>>ASSERTION Bad B 3
When an invalid event class is specified, a
BadValue error results.
>>STRATEGY
Specify an invalid event class.
>>CODE badclass
Display	*client2;
XEventClass eclass = -1;
XID badclass;

	if (!Setup_Extension_DeviceInfo(KeyMask))
	    {
	    untested("%s: Input extension key device not present.\n", TestName);
	    return;
	    }
	BadClass(display,badclass);
	device = Devs.Key;
	modifier_device = Devs.Key;
	event_count = 1;
	event_list = &eclass;

	XCALL;
	XSync (Dsp,0);

	if (geterr() == badclass)
		CHECK;
	else
		FAIL;

	CHECKPASS(1);

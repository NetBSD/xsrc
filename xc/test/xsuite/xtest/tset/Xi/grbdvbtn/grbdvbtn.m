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
 * $XConsortium: grbdvbtn.m,v 1.7 94/04/17 21:12:44 rws Exp $
 */
>>TITLE XGrabDeviceButton XINPUT
void

Display	*display = Dsp;
XDevice *device;
int 	button = AnyButton;
unsigned int 	modifiers = AnyModifier;
XDevice *modifier_device = NULL;
Window	grab_window = defwin(display);
Bool	owner_events = True;
int 	event_count = 0;
XEventClass *event_list;
int 	this_device_mode = GrabModeAsync;
int 	other_devices_mode = GrabModeAsync;
>>EXTERN
extern ExtDeviceInfo Devs;
extern int NumButtons;

>>ASSERTION Good B 3
A call to xname establishes a passive grab on the button device that is 
activated in the future by
the specified button being logically pressed,
the specified modifier keys being logically down,
no other modifier keys being logically down,
the
.A grab_window
being the focus window or an ancestor of the focus window
or being a descendant of the focus window that contains the pointer
and
a passive grab on the same button combination not existing on any
ancestor of
.A grab_window .
>>STRATEGY
Call xname.  Verify that the grab was correctly installed by pressing
a button to activate it, then attempting to grab the device from another
client.  Verify that the grab fails with an error of AlreadyGrabbed.
>>CODE
int ret;
Display *client1;

	if (!Setup_Extension_DeviceInfo(BtnMask | ModMask))
	    {
	    untested("%s: Required input extension devices not present.\n", TestName);
	    return;
	    }
	if ((client1 = opendisplay()) == 0) {
		delete("Could not open display");
		return;
	}
	device = Devs.Button;
	modifier_device = Devs.Mod;
	ret = XCALL;
	XSync(display,0);

	if (noext(1))
	    return;
	warppointer(display, grab_window, 1, 1);
	devicebuttonpress (display, Devs.Button, Button1);
	XSync (display, 0);

	ret = XGrabDevice(client1, Devs.Button, grab_window, True, 0, NULL,
	    GrabModeAsync, GrabModeAsync, CurrentTime);
	if (ret != AlreadyGrabbed)
	    FAIL;
	else
	    PASS;
	devicebuttonrel (display, Devs.Button, Button1);
	devicerelbuttons (Devs.Button);
	XUngrabDeviceButton(display, device, AnyButton, AnyModifier, modifier_device, grab_window);

>>ASSERTION Good B 3
When the conditions for activating the grab are otherwise satisfied
and the button device is already grabbed,
then no active grab is established.
>>STRATEGY
Establish an active grab on the target device.  Then establish a passive
grab from another client.  Press a button to activate the grab.
If the passive grab worked, the client that established the passive grab
should be able to replace it with an active grab.  If it failed, the active
grab should also fail with an error of AlreadyGrabbed.
>>CODE
int ret;
Display *client1;

	if (!Setup_Extension_DeviceInfo(BtnMask | ModMask))
	    {
	    untested("%s: Required input extension devices not present.\n", TestName);
	    return;
	    }
	if ((client1 = opendisplay()) == 0) {
		delete("Could not open display");
		return;
	}
	device = Devs.Button;
	modifier_device = Devs.Mod;

	ret = XGrabDevice(client1, Devs.Button, grab_window, True, 0, NULL,
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
	devicebuttonpress (display, Devs.Button, Button1);
	XSync (display, 0);

	ret = XGrabDevice(display, Devs.Button, grab_window, True, 0, NULL,
	    GrabModeAsync, GrabModeAsync, CurrentTime);
	if (ret != AlreadyGrabbed)
	    FAIL;
	else
	    CHECK;
	devicebuttonrel (display, Devs.Button, Button1);
	devicerelbuttons (Devs.Button);
	XUngrabDeviceButton(display, device, AnyButton, AnyModifier, modifier_device, grab_window);
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
  Press button to activate grab.
  Verify that grab is activated.
  Release button and keys.

  Press button (no modifiers).
  Verify that grab is active.
else
  Perform touch test.
  Report untested.
>>CODE
unsigned int 	ret, mods;
Display *client1;

	if (!Setup_Extension_DeviceInfo(BtnMask | ModMask))
	    {
	    untested("%s: Required input extension devices not present.\n", TestName);
	    return;
	    }
	if ((client1 = opendisplay()) == 0) {
		delete("Could not open display");
		return;
	}
	modifiers = AnyModifier;
	device = Devs.Button;
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
	devicebuttonpress(display, Devs.Button, Button1);
	XSync(display,0);
	ret = XGrabDevice(client1, Devs.Button, grab_window, True, 0, NULL,
	    GrabModeAsync, GrabModeAsync, CurrentTime);
	if (ret != AlreadyGrabbed) 
	    {
	    report("Grab not activated for AnyModifier");
	    report("  Modifiers used %s", keymaskname((unsigned long)mods));
	    FAIL;
	    }
	else
	    CHECK;

	/* Release all buttons and modifiers */
	modrel(display, mods);
	devicebuttonrel(display, Devs.Button, Button1);
	relalldev();
	XSync(display,0);

	ret = XGrabDevice(client1, Devs.Button, grab_window, True, 0, NULL,
	    GrabModeAsync, GrabModeAsync, CurrentTime);
	if (ret == AlreadyGrabbed) 
	    {
	    delete("Could not release grab for second part of test");
	    FAIL;
	    }
	else
	    {
	    XUngrabDevice(client1, Devs.Button, CurrentTime);
	    XSync(client1,0);
	    CHECK;
	    }

	devicebuttonpress(display, Devs.Button, Button1);
	XSync(display,0);
	ret = XGrabDevice(client1, Devs.Button, grab_window, True, 0, NULL,
	    GrabModeAsync, GrabModeAsync, CurrentTime);
	if (ret != AlreadyGrabbed) 
	    {
	    report("Grab with AnyModifier was not activated by a button press with");
	    report("  no modifiers");
	    FAIL;
	    }
	else
	    CHECK;
	devicebuttonrel(display, Devs.Button, Button1);
	devicerelbuttons (Devs.Button);
	XUngrabDeviceButton(display, device, AnyButton, AnyModifier, modifier_device, grab_window);
	CHECKPASS(3);

>>ASSERTION Good B 3
When the conditions for activating the grab are satisfied, then the
last-device-grab time is set to the time at which the button was pressed
and a
.S DeviceButtonPress
event is generated.
>>STRATEGY
If extensions are available:
  Call xname to place passive grab.
  Enable events on grab window.
  Move pointer into grab window.
  Activate grab with simulated device events.
  Verify that a DeviceButtonPress event is generated.
else
  Report untested.
>>CODE
XEvent	ev;
XID	dbp, dbr;
XEventClass dbpclass, dbrclass;
XDeviceButtonPressedEvent	good;
XWindowAttributes	atts;
int 	n;

	if (noext(1))
		return;

	if (!Setup_Extension_DeviceInfo(BtnMask | ModMask))
	    {
	    untested("%s: Required input extension devices not present.\n", TestName);
	    return;
	    }
	device = Devs.Button;
	DeviceButtonPress(device, dbp, dbpclass);
	DeviceButtonRelease(device, dbr, dbrclass);
	XCALL;

	XSelectExtensionEvent(display, grab_window, &dbrclass, 1);

	(void) warppointer(display, grab_window, 1, 1);
	XSync(display, True);	/* Discard any events */
	devicebuttonpress(display, Devs.Button, Button1);

	XGetWindowAttributes(display, grab_window, &atts);
	n = getevent(display, &ev);
	if (n)
		CHECK;
	else {
		report("No events received");
		FAIL;
		return;
	}

	defsetevent(good, display, dbp);
	good.window = grab_window;
	good.root = DRW(display);
	good.subwindow = None;
	good.time = ((XDeviceButtonPressedEvent*)&ev)->time;
	good.x = 1;
	good.y = 1;
	good.x_root = good.x + atts.x + atts.border_width;
	good.y_root = good.y + atts.y + atts.border_width;
	good.state = modifiers;
	good.button = Button1;
	good.same_screen = True;

	if (checkevent((XEvent*)&good, &ev) == 0)
		CHECK;
	else
		FAIL;

	devicebuttonrel(display, Devs.Button, Button1);
	devicerelbuttons (Devs.Button);
	XUngrabDeviceButton(display, device, AnyButton, AnyModifier, modifier_device, grab_window);
	CHECKPASS(2);
>>ASSERTION Good B 3
When the grab subsequently becomes active and later the logical state of the
pointer has all buttons released, then the active grab
is terminated automatically.
>># independent of the state of the logical modifier keys.
>>STRATEGY
If extensions are available:
  Place passive grab with xname.
  Activate grab with simulated device events.
  Simulate pressing some modifier keys.
  Release the button.
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
	if (!Setup_Extension_DeviceInfo(BtnMask | ModMask))
	    {
	    untested("%s: Required input extension devices not present.\n", TestName);
	    return;
	    }
	device = Devs.Button;
	XCALL;

	devicebuttonpress(display, Devs.Button, Button1);
	mods = wantmods(display, 3);
	modpress(display, mods);
	XSync(display,0);

	devicebuttonrel(display, Devs.Button, Button1);
	XSync(display,0);

	ret = XGrabDevice(client1, Devs.Button, grab_window, True, 0, NULL,
	    GrabModeAsync, GrabModeAsync, CurrentTime);
	if (ret != Success)
	    {
	    report("Grab was not released, when button was released and ");
	    report("  modifier keys were down");
	    FAIL;
	    }
	else
	    PASS;
	modrel(display, mods);
	devicerelbuttons (Devs.Button);
	relalldev();
	XUngrabDeviceButton(display, device, AnyButton, AnyModifier, modifier_device, grab_window);

>>ASSERTION Good B 3
A call to xname overrides all previous passive grabs by the same
client on the same button/modifier combinations on the same window.
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
int count = 0;
XID	dbp, dbr;
XEventClass dbpclass, dbrclass;
XEvent ev;

	if (noext(1))
		return;

	if (!Setup_Extension_DeviceInfo(BtnMask | ModMask))
	    {
	    untested("%s: Required input extension devices not present.\n", TestName);
	    return;
	    }
	device = Devs.Button;
	DeviceButtonPress(device, dbp, dbpclass);
	DeviceButtonRelease(device, dbr, dbrclass);
	XSelectExtensionEvent(display, grab_window, &dbpclass, 1);
	XSync(display,0);
	warppointer(display, grab_window, 1, 1);

	this_device_mode = GrabModeSync;
	XCALL;

	/* Try to override first grab */
	this_device_mode = GrabModeAsync;
	XCALL;
	XSync(display,0);

	devicebuttonpress(display, Devs.Button, Button1);
	if (NumButtons > 1)
	    devicebuttonpress(display, Devs.Button, Button2);
	if (NumButtons > 2)
	    devicebuttonpress(display, Devs.Button, Button3);
	XSync(display,0);

	while (XPending(display)) 
	    {
	    XNextEvent(display, &ev);
	    count++;
	    if (ev.type==dbp)
		CHECK;
	    else
		FAIL;
	    }

	devicerelbuttons(Devs.Button);
	XUngrabDeviceButton(display, device, AnyButton, AnyModifier, modifier_device, grab_window);
	CHECKPASS(count);

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

	if (!Setup_Extension_DeviceInfo(BtnMask | ModMask))
	    {
	    untested("%s: Required input extension devices not present.\n", TestName);
	    return;
	    }
	if ((client1 = opendisplay()) == 0) {
		delete("Could not open display");
		return;
	}
	device = Devs.Button;
	modifiers = wantmods(display, 1);
	modifier_device = NULL;
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

	if ((ret=XSetModifierMapping(display, newmap)) == MappingSuccess)
		CHECK;
	else {
		delete("Could not remove modifier mapping,ret=%d",ret);
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
	devicebuttonpress(display, Devs.Button, Button1);
	XSync(display,0);

	ret = XGrabDevice(client1, Devs.Button, grab_window, True, 0, NULL,
	    GrabModeAsync, GrabModeAsync, CurrentTime);
	if (ret != AlreadyGrabbed) 
	    {
	    report("Passive grab not set when the modifier did not have a current keycode");
	    FAIL;
	    }
	else
	    CHECK;

	CHECKPASS(5);
	modrel(display, modifiers);
	devicebuttonrel(display, Devs.Button, Button1);
	devicerelbuttons (Devs.Button);
	relalldev();
	XUngrabDeviceButton(display, device, AnyButton, AnyModifier, modifier_device, grab_window);

>>ASSERTION Good B 3
When the
.A button
argument is
.S AnyButton ,
then this is equivalent to separate calls to xname for
all possible buttons.
>>STRATEGY
Establish a passive grab for AnyButton on an input extension device.
Press each of the buttons on that device.
Verify that pressing each of the buttons activates the grab.
>>CODE
int i, j, ret, ndevices, nbtns, count=0;
Display *client1;
XDeviceInfo *list;
XAnyClassPtr any;

	if (!Setup_Extension_DeviceInfo(BtnMask | ModMask))
	    {
	    untested("%s: Required input extension devices not present.\n", TestName);
	    return;
	    }
	if ((client1 = opendisplay()) == 0) {
		delete("Could not open display");
		return;
	}
	device = Devs.Button;
	list = XListInputDevices(display, &ndevices);
	for (i=0; i<ndevices; i++, list++)
	    if (list->id == Devs.Button->device_id)
		{
		any = (XAnyClassPtr) (list->inputclassinfo);
		for (j=0; j<list->num_classes; j++)
		    {
		    if (any->class == ButtonClass)
			{
			nbtns = ((XButtonInfo *) any)->num_buttons;
			break;
			}
		    any = (XAnyClassPtr) ((char *) any + any->length);
		    }
		break;
		}
	modifier_device = Devs.Mod;
	for (i=1;i<nbtns;i++)
	    {
	    ret = XCALL;
	    XSync(display,0);

	    if (noext(1))
		return;
	    warppointer(display, grab_window, 1, 1);
	    devicebuttonpress (display, Devs.Button, i);
	    XSync (display, 0);

	    ret = XGrabDevice(client1, Devs.Button, grab_window, True, 0, NULL,
		GrabModeAsync, GrabModeAsync, CurrentTime);
	    if (ret != AlreadyGrabbed)
		{
		report("Pressing a button did not activate the grab.");
		FAIL;
		}
	    else
		{
		CHECK;
		count++;
		}
	    XUngrabDeviceButton(display, Devs.Button, AnyButton, AnyModifier,
		NULL, grab_window);
	    XSync(display,0);
	    devicebuttonrel (display, Devs.Button, i);
	    }
	devicerelbuttons (Devs.Button);
	CHECKPASS(count);

>>ASSERTION Good B 3
When the event window for an active grab becomes not viewable, then the
grab is released automatically.
>>STRATEGY
Establish a passive grab on an input device extension device.
Activate the grab by pressing a button on the device.
Verify that the grab is active by trying to establish another active grab
from a different client, and verifying that AlreadyGrabbed is returned.
Make the grab window non-viewable.
Attempt another active grab and verify that it works this time.
>>CODE
int ret;
Display *client1;

	if (!Setup_Extension_DeviceInfo(BtnMask | ModMask))
	    {
	    untested("%s: Required input extension devices not present.\n", TestName);
	    return;
	    }
	if ((client1 = opendisplay()) == 0) {
		delete("Could not open display");
		return;
	}
	device = Devs.Button;
	modifier_device = Devs.Mod;
	ret = XCALL;
	XSync(display,0);

	if (noext(1))
	    return;
	warppointer(display, grab_window, 1, 1);
	devicebuttonpress (display, Devs.Button, Button1);
	XSync (display, 0);

	ret = XGrabDevice(client1, Devs.Button, grab_window, True, 0, NULL,
	    GrabModeAsync, GrabModeAsync, CurrentTime);
	if (ret != AlreadyGrabbed)
	    FAIL;
	else
	    CHECK;
	XUnmapWindow(display,grab_window);
	XSync(display,0);

	grab_window = defwin(display);
	ret = XGrabDevice(client1, Devs.Button, grab_window, True, 0, NULL,
	    GrabModeAsync, GrabModeAsync, CurrentTime);
	if (ret != Success)
	    FAIL;
	else
	    CHECK;
	devicebuttonrel (display, Devs.Button, Button1);
	devicerelbuttons (Devs.Button);
	XUngrabDeviceButton(display, device, AnyButton, AnyModifier, modifier_device, grab_window);
	CHECKPASS(2);
>>ASSERTION Good B 3
NULL may be specified as the modifier device.  This will cause the X keyboard
to be used as the modifier device.
>>STRATEGY
Specify NULL as the modifier device.
>>CODE
Display *client1;
int ret;

	if (!Setup_Extension_DeviceInfo(BtnMask))
	    {
	    untested("%s: Input extension button device not present.\n", TestName);
	    return;
	    }
	if ((client1 = opendisplay()) == 0) {
		delete("Could not open display");
		return;
	}
	device = Devs.Button;
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
	warppointer(display, grab_window, 1, 1);
	modpress(display, modifiers);
	devicebuttonpress (display, Devs.Button, Button1);
	XSync (display, 0);

	ret = XGrabDevice(client1, Devs.Button, grab_window, True, 0, NULL,
	    GrabModeAsync, GrabModeAsync, CurrentTime);
	if (ret != AlreadyGrabbed)
	    {
	    report("%s: Grab failed to activate.", TestName);
	    FAIL;
	    }
	else
	    CHECK;

	modrel(display, modifiers);
	devicebuttonrel (display, Devs.Button, Button1);
	devicerelbuttons (Devs.Button);
	relalldev();
	XUngrabDeviceButton(display, device, AnyButton, AnyModifier,
		NULL, grab_window);
	CHECKPASS(2);
>>ASSERTION Good B 3
If a modifier device other than the core keyboard is specified, pressing the
specified button while the specified modifiers are down on the core keyboard 
will not cause the grab to be activated.
>>STRATEGY
Set up a grab with a modifier device other than the core keyboard.
Press the equivalent modifiers on the core keyboard
Press the specified button.
Verify that the grab is not activated.
>>CODE
int i, ret, coremask, devmask;
Display *client1;

	if (!Setup_Extension_DeviceInfo(BtnMask | ModMask))
	    {
	    untested("%s: Required input extension devices not present.\n", TestName);
	    return;
	    }

	if ((client1 = opendisplay()) == 0) {
		delete("Could not open display");
		return;
	}
	device = Devs.Button;
	coremask = wantmods(display, 3);
	devmask = wantdevmods(display, Devs.Mod, 3);
	modifiers = coremask & devmask;
	if (!modifiers)
	    {
	    report("Can't find equivalent modifiers on core and extension devs.");
	    UNTESTED;
	    return;
	    }
	else
	    CHECK;
	modifier_device = Devs.Mod;
	ret = XCALL;
	XSync(display,0);

	if (noext(1))
	    {
	    report("%d: No XTest extension, can't complete test.",TestName);
	    UNTESTED;
	    return;
	    }
	warppointer(display, grab_window, 1, 1);
	modpress(display, modifiers);
	devicebuttonpress (display, Devs.Button, Button1);
	XSync (display, 0);

	ret = XGrabDevice(client1, Devs.Button, grab_window, True, 0, NULL,
	    GrabModeAsync, GrabModeAsync, CurrentTime);
	if (ret == AlreadyGrabbed)
	    {
	    report("Pressing modifiers on the core keyboard activated the grab.");
	    FAIL;
	    }
	else
	    CHECK;

	modrel(display, modifiers);
	devicebuttonrel (display, Devs.Button, Button1);
	devicerelbuttons (Devs.Button);
	relalldev();
	CHECKPASS(2);
>>ASSERTION Bad B 3
.ER BadValue modifiers mask ShiftMask LockMask ControlMask Mod1Mask Mod2Mask Mod3Mask Mod4Mask Mod5Mask AnyModifier
>>STRATEGY
Call xname with a bad value for the modifiers argument.
Verify that a BadValue error occurs.
>>CODE BadValue

	if (!Setup_Extension_DeviceInfo(BtnMask))
	    {
	    untested("%s: No input extension button device.\n", TestName);
	    return;
	    }
	device = Devs.Button;
	modifier_device = NULL;
	modifiers = ~0;

	button = 1;
	XCALL;

	if (geterr() == BadValue)
		CHECK;

	else
		FAIL;

	CHECKPASS(1);
>>ASSERTION Bad B 3
.ER BadValue owner_events True False
>>STRATEGY
Call xname with a bad value for the owner_events argument.
Verify that a BadValue error occurs.
>>CODE BadValue

	if (!Setup_Extension_DeviceInfo(BtnMask))
	    {
	    untested("%s: No input extension button device.\n", TestName);
	    return;
	    }
	device = Devs.Button;
	modifier_device = NULL;
	owner_events = 2;

	XCALL;

	if (geterr() == BadValue)
		CHECK;

	else
		FAIL;

	CHECKPASS(1);
>>ASSERTION Bad B 3
.ER BadValue this_device_mode GrabModeSync GrabModeAsync
>>STRATEGY
Call xname with a bad value for the this_device_mode argument.
Verify that a BadValue error occurs.
>>CODE BadValue

	if (!Setup_Extension_DeviceInfo(BtnMask))
	    {
	    untested("%s: No input extension button device.\n", TestName);
	    return;
	    }
	device = Devs.Button;
	modifier_device = NULL;
	this_device_mode = 2;

	XCALL;

	if (geterr() == BadValue)
		CHECK;

	else
		FAIL;

	CHECKPASS(1);
>>ASSERTION Bad B 3
.ER BadValue other_devices_mode GrabModeSync GrabModeAsync
>>STRATEGY
Call xname with a bad value for the other_devices_mode argument.
Verify that a BadValue error occurs.
>>CODE BadValue

	if (!Setup_Extension_DeviceInfo(BtnMask))
	    {
	    untested("%s: No input extension button device.\n", TestName);
	    return;
	    }
	device = Devs.Button;
	modifier_device = NULL;
	other_devices_mode = 2;

	XCALL;

	if (geterr() == BadValue)
		PASS;

	else
		FAIL;

>>ASSERTION Bad B 3
.ER Access grab
>>STRATEGY
Grab a button.
Create new client, client1.
Attempt to grab same button with client1.
Verify that a BadAccess error occurs.
>>CODE BadAccess
Display	*client1;

	if (!Setup_Extension_DeviceInfo(ModMask | BtnMask))
	    {
	    untested("%s: Required input extension devices not present.\n", TestName);
	    return;
	    }
	device = Devs.Button;
	modifier_device = Devs.Mod;

	XGrabDeviceButton(Dsp, device, button, modifiers, modifier_device,
		grab_window, owner_events, event_count, event_list,
		this_device_mode, other_devices_mode);
	XSync (Dsp,0);
	if (isdeleted()) {
		delete("Could not set up initial grab");
		return;
	}

	if ((client1 = opendisplay()) == 0) {
		delete("Could not open display");
		return;
	}

	display = client1;
	XCALL;

	if (geterr() == BadAccess)
		PASS;
	else
		FAIL;
>>ASSERTION Bad B 3
When a call to xname is made specifying an invalid window,
a BadWindow error will result.
>>STRATEGY
Specify an invalid window as the target window.
>>CODE BadWindow

	if (!Setup_Extension_DeviceInfo(BtnMask))
	    {
	    untested("%s: No input extension button device.\n", TestName);
	    return;
	    }
	device = Devs.Button;
	modifier_device = NULL;
	grab_window = 0;

	XCALL;
	if (geterr() == BadWindow)
		PASS;
	else
		FAIL;

>>ASSERTION Bad B 3
When a call to xname is made specifying a device with no keys as the
modifier device
a BadMatch error will result.
>>STRATEGY
Specify a device with no keys as the modifier device.
>>CODE BadMatch
Display	*client2;

	if (!Setup_Extension_DeviceInfo(BtnMask | NKeysMask))
	    {
	    untested("%s: Required input extension devices not present.\n", TestName);
	    return;
	    }
	device = Devs.Button;
	modifier_device = Devs.NoKeys;

	XCALL;
	XSync(display,0);
	if (geterr() == BadMatch)
		PASS;
	else
		FAIL;

>>ASSERTION Bad B 3
When a call to xname is made specifying an invalid grab device,
a BadDevice error will result.
>>STRATEGY
Specify an invalid device.
>>CODE baddevice
XDevice bogus;
XID baddevice;

	if (!Setup_Extension_DeviceInfo(BtnMask))
	    {
	    untested("%s: No input extension button device.\n", TestName);
	    return;
	    }
	BadDevice(display, baddevice);
	device = &bogus;
	bogus.device_id = 128;
	modifier_device = Devs.Mod;

	XCALL;
	XSync(display,0);

	if (geterr() == baddevice)
		PASS;
	else
		FAIL;

>>ASSERTION Bad B 3
When a call to xname is made specifying an invalid modifier device,
a BadDevice error will result.
>>STRATEGY
Specify an invalid modifier device.
>>CODE baddevice
XDevice bogus;
XID baddevice;

	if (!Setup_Extension_DeviceInfo(BtnMask))
	    {
	    untested("%s: No input extension button device.\n", TestName);
	    return;
	    }
	BadDevice(display, baddevice);
	device = Devs.Button;
	modifier_device = &bogus;
	bogus.device_id = 128;

	XCALL;
	XSync(display,0);

	if (geterr() == baddevice)
		PASS;
	else
		FAIL;

>>ASSERTION Bad B 3
When a call to xname is made specifying an invalid event class,
a BadClass error will result.
>>STRATEGY
Specify an invalid event class.
>>CODE badclass
Display	*client2;
XEventClass eclass = -1;
XID badclass;

	if (!Setup_Extension_DeviceInfo(ModMask | BtnMask))
	    {
	    untested("%s: Required input extension devices not present.\n", TestName);
	    return;
	    }
	BadClass(display,badclass);
	device = Devs.Button;
	modifier_device = Devs.Mod;
	event_count = 1;
	event_list = &eclass;

	XCALL;
	XSync(display,0);

	if (geterr() == badclass)
		PASS;
	else
		FAIL;


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
 * $XConsortium: alldvevnts.m,v 1.6 94/04/17 21:13:06 rws Exp $
 */
>>TITLE XAllowDeviceEvents XINPUT
void

Display	*display = Dsp;
XDevice *device;
int	event_mode = AsyncThisDevice;
Time	time  = CurrentTime;
>>EXTERN
#include <stdio.h>
extern ExtDeviceInfo Devs;

/*
 * A window for use as a grab window in the freeze and freezecheck
 * routines.
 */
static	Window	grabwin;
static	XID dmn, dbp, dbr;
static	XEventClass class[3];

/*
 * Verify devices and create grab window.
 */

static int
grabstartup()
{

	if (Dsp==(Display *)NULL)
		return;

	if (!Setup_Extension_DeviceInfo(BtnMask))
	    {
	    untested("Required extension device not available for %s.\n", TestName);
	    return(0);
	    }
	grabwin = defwin(Dsp);
	device = Devs.Button;
	DeviceMotionNotify(Devs.Button, dmn, class[0]);
	DeviceButtonPress(device, dbp, class[1]);
	DeviceButtonRelease(device, dbr, class[2]);
	XSelectExtensionEvent(Dsp, grabwin, class, 3);
	(void) warppointer(Dsp, grabwin, 1, 1);
	XSync(Dsp,0);
	return (1);
}
/*
 * Destroy the grab window.
 */

static void
grabcleanup()
{
	if (Dsp)
	    XDestroyWindow(Dsp, grabwin);
}

/*
 * Grab and freeze the device.
 */
static void
grabfreezedevice(disp, time)
Display	*disp;
Time	time;
{

	device = Devs.Button;
	XGrabDevice(disp, device, grabwin, True, 3, 
	    class, GrabModeSync, GrabModeAsync, time);
}

/*
 * Return True if the device is frozen.  We generate a motion event on the
 * device and check to see if we can receive it.  If we receive it,
 * then the device is not frozen.
 */
static	Bool
ispfrozen(disp)
Display	*disp;
{
int i, axisval=0, ret=True;
XDeviceState *state;
XInputClass *data;
XEvent ev;
Window w;

	 XSync(display, True); /* Flush previous events */
	SimulateDeviceMotionEvent(display, Devs.Button, False, 1, &axisval, 0);
	XSync(display,0);
	while (XPending(display))
	    {
	    XNextEvent(display, &ev);
	    if (ev.type == dmn)
		ret = False;
	    }
	return(ret);
}

>>ASSERTION Good B 3
When the specified time is earlier than the last-grab
time of the most recent active grab for the client or
later than the current X server time, then a call to xname has no effect.
>>STRATEGY
Grab and freeze device with a given time.
Call xname with earlier time and AsyncThisDevice.
Verify that the device is still frozen.
Get current server time.
Call xname with a later time.
Verify that the device is still frozen.
>>CODE

	/* get time from the server */
	if (!grabstartup())
	    {
	    UNTESTED;
	    return;
	    }

	if (noext(0))
	    return;
	device = Devs.Button;
	time = gettime(display);
	grabfreezedevice(display, time);

	time -= 100;
	XCALL;

	if (ispfrozen(display))
		CHECK;
	else {
		report("Events allowed when time was earlier than last-grab time");
		FAIL;
	}

	/*
	 * Get current time again and add several minutes to get a time in the
	 * future.
	 */
	time = gettime(display);
	time += ((config.speedfactor+1) * 1000000);
	XCALL;

	if (ispfrozen(display))
		CHECK;
	else {
		report("Events allowed when time was later than current server time");
		FAIL;
	}

	XUngrabDevice(display, device, CurrentTime);
	XSync(display,0);
	CHECKPASS(2);
>>ASSERTION Good B 3
When the
.A event_mode
argument is
.S AsyncThisDevice
and the device is frozen by the client,
then device event processing is resumed.
>>STRATEGY
Freeze device.
Call xname with event_mode AsyncThisDevice.
Verify that device is not frozen.
>>CODE

	if (!grabstartup())
	    {
	    UNTESTED;
	    return;
	    }
	if (noext(0))
	    return;
	device = Devs.Button;
	grabfreezedevice(display, time);

	event_mode = AsyncThisDevice;
	XCALL;
	XSync(display,0);

	if (ispfrozen(display)) {
		report("Device was not released after AsyncThisDevice");
		FAIL;
	} else
		CHECK;

	XUngrabDevice(display, device, CurrentTime);
	XSync(display,0);
	CHECKPASS(1);
>>ASSERTION Good B 3
When the
.A event_mode
argument is
.S AsyncThisDevice
and the device is frozen twice by the client on behalf of two separate grabs,
then a call to xname thaws for both grabs.
>>STRATEGY
Freeze device with XGrabPointer.
Freeze device with XGrabKeyboard.
Call xname with event_mode AsyncThisDevice.
Verify that device is not frozen.
>>CODE

	if (!grabstartup())
	    {
	    UNTESTED;
	    return;
	    }
	if (noext(0))
	    return;
	device = Devs.Button;
	XGrabPointer(display, grabwin, False, NoEventMask, GrabModeSync, 
		GrabModeSync, None, None, CurrentTime);
	XGrabKeyboard(display, grabwin, False, GrabModeSync, GrabModeSync,
		CurrentTime);

	if (isdeleted())
		return;

	event_mode = AsyncThisDevice;
	XCALL;

	if (ispfrozen(display)) {
		report("Device was not released from double grab after AsyncThisDevice");
		FAIL;
	} else
		CHECK;

	CHECKPASS(1);
>>ASSERTION Good B 3
When the
.A event_mode
argument is
.S AsyncThisDevice
and the device is not frozen by the client,
then a call to xname has no effect.
>>STRATEGY
Call xname with event_mode AsyncThisDevice.
Verify device is not frozen.
>>CODE

	if (!grabstartup())
	    {
	    UNTESTED;
	    return;
	    }
	if (noext(0))
	    return;
	device = Devs.Button;
	event_mode = AsyncThisDevice;

	XCALL;
	if (!ispfrozen(display))
		PASS;
	else {
		report("device was frozen after AsyncThisDevice");
		FAIL;
	}
>>ASSERTION Good B 3
When the
.A event_mode
argument is
.S SyncThisDevice
and the device is frozen and actively grabbed by the client, then
device event processing is resumed normally until the next
.S DeviceButtonPress
or
.S DeviceButtonRelease
event is reported to the client, at which time
the device again appears to freeze, unless the reported event causes
the device grab to be released.
>>STRATEGY
Grab and freeze the device.
Call xname with event_mode SyncThisDevice.
Verify that device is not frozen.
If test extension available:
  Press a button.
  Verify that the device is frozen.
>>CODE

	if (!grabstartup())
	    {
	    UNTESTED;
	    return;
	    }
	if (noext(0))
	    return;
	device = Devs.Button;
	grabfreezedevice(display, time);
	if (ispfrozen(display))
		CHECK;
	else {
		delete("Could not freeze device");
		return;
	}

	event_mode = SyncThisDevice;
	XCALL;

	if (ispfrozen(display)) {
		report("Device was not released after SyncThisDevice");
		FAIL;
	} else
		CHECK;

	if (noext(1) || nbuttons() <= 1) {
		CHECKUNTESTED(2);
		return;
	}

	/* If extension we can go on */
	devicebuttonpress(display, Devs.Button, Button1);
	if (ispfrozen(display))
		CHECK;
	else {
		report("Device was not re-frozen by a button press");
		FAIL;
	}

	devicebuttonrel(display, Devs.Button, Button1);
	devicerelbuttons(Devs.Button);
	XUngrabDevice(display, device, CurrentTime);
	XSync(display,0);
	CHECKPASS(3);

>>ASSERTION Good B 3
When the
.A event_mode
argument is
.S SyncThisDevice
and the device is not frozen by the client or the device is not grabbed by
the client,
then a call to xname has no effect.
>>STRATEGY
Call xname with event_mode SyncThisDevice.
Verify device is not frozen.
>>CODE

	if (!grabstartup())
	    {
	    UNTESTED;
	    return;
	    }
	if (noext(0))
	    return;
	device = Devs.Button;
	event_mode = SyncThisDevice;

	XCALL;
	if (!ispfrozen(display))
		PASS;
	else {
		report("device was frozen after SyncThisDevice with no initial freeze");
		FAIL;
	}
>>ASSERTION Good B 3
When the
.A event_mode
argument is
.S SyncThisDevice
and the device is frozen twice by the client on behalf of two separate
grabs, then a call to xname thaws for both grabs.
>>STRATEGY
Freeze device with XGrabDevice.
Freeze device with XGrabKeyboard.
Call xname with event_mode SyncThisDevice.
Verify that device is not frozen.
>>CODE
int ret;

	if (!grabstartup())
	    {
	    UNTESTED;
	    return;
	    }
	if (noext(0))
	    return;
	device = Devs.Button;
	XGrabDevice(display, device, grabwin, True, 3, 
	    class, GrabModeSync, GrabModeAsync, time);
	XGrabKeyboard(display, grabwin, False, GrabModeSync, GrabModeSync,
		CurrentTime);

	if (isdeleted())
		return;

	event_mode = SyncThisDevice;
	ret = XCALL;

	if (ispfrozen(display)) {
		report("device was not released after SyncThisDevice");
		FAIL;
	} else
		CHECK;

	CHECKPASS(1);
>>#NUM 008
>>ASSERTION Good B 3
When the
.A event_mode
argument is
.S ReplayThisDevice
and the device is actively grabbed by the client and is frozen as
the result of an event having been sent to the client, then
the device grab is released and the event is completely reprocessed
as though
any passive grabs at or above the grab window of the grab just released
were not present.
>>STRATEGY
Touch test for replay device.
If extensions are available:
  Create window.
  Create child of this window.
  Set passive grabs on both these windows.
  Warp device into child window.
  Press button to activate the grab.
  Check that parent window has the grab.
  Set event_mode to ReplayThisDevice.
  Call xname.
  Verify that the child window now has the grab.
>>CODE
int ret;
XEvent  ev;
XButtonPressedEvent    *bpp;
Window  chwin;
struct  area    area;

	if (!grabstartup())
	    {
	    UNTESTED;
	    return;
	    }
	device = Devs.Button;
	event_mode = ReplayThisDevice;
	if (noext(1)) {
		XCALL;

		untested("There is no reliable test method, but a touch test was performed");
		return;
	} else
		CHECK;

	/*
	 * Set up a device freeze as a result of a button press.
	 */
	(void) warppointer(display, grabwin, 1, 1);
	setarea(&area, 50, 50, 5, 5);
	chwin = crechild(display, grabwin, &area);

	XSelectExtensionEvent(display, grabwin, class, 3);
	XGrabDeviceButton(display, Devs.Button, Button1, AnyModifier,
		NULL, grabwin,
		False, 3, class, GrabModeSync, GrabModeAsync);
	XGrabDeviceButton(display, Devs.Button, Button1, AnyModifier,
		NULL, chwin,
		False, 3, class, GrabModeSync, GrabModeAsync);

	/*
	 * Activate the grab.
	 */
	XSync(display, True);	/* Discard any events */
	(void) warppointer(display, chwin, 1, 1);
	devicebuttonpress(display, Devs.Button, Button1);
	XSync(display,False);

	/*
	 * Check that the grab was activated and that it occurs on the parent
	 * window.
	 */
	ret = XPending(display);
	if (XPending(display)) {
	        XNextEvent(display, &ev);
	/*
	if (XCheckTypedEvent(display, devicebuttonpress, &ev)) {
	*/
		bpp = (XButtonPressedEvent*)&ev;
		if (bpp->window == grabwin)
			CHECK;
		else if (bpp->window == chwin) {
			delete("Child window had the grab");
			return;
		} else {
			delete("Could not get grab on parent window");
			return;
		}
	} else {
		report("Did not get a button event when trying to activate grab");
		FAIL;
		return;
	}

	/* Do the ReplayThisDevice */
	XCALL;

	/*
	 * The effect should be as if the button were pressed again
	 * but without the passive grab on the parent window.  So this
	 * time the child should pick up the grab.
	 */
	ret = XPending(display);
	if (XPending(display)) {
	        XNextEvent(display, &ev);
		bpp = (XButtonPressedEvent*)&ev;
		if (bpp->window == chwin)
			CHECK;
		else if (bpp->window == grabwin) {
			report("Parent window had the grab after a ReplayThisDevice");
			FAIL;
		} else {
			report("After ReplayThisDevice the grab on the child did not activate");
			FAIL;
		}
	} else {
		report("Did not get a button event when trying to activate grab");
		FAIL;
	}

	devicebuttonrel(display, Devs.Button, Button1);
	devicerelbuttons(Devs.Button);
	XUngrabDeviceButton(display, Devs.Button, Button1, AnyModifier,
		NULL, grabwin);
	XUngrabDeviceButton(display, Devs.Button, Button1, AnyModifier,
		NULL, chwin);
	CHECKPASS(3);
>>ASSERTION Good B 3
When the
.A event_mode
argument is
.S ReplayThisDevice
and the device is not frozen as a result of an event
or the device is not grabbed by the client,
then a call to xname has no effect.
>>STRATEGY
Call xname with event_mode ReplayThisDevice.
Verify device is not frozen.
>>CODE

	if (!grabstartup())
	    {
	    UNTESTED;
	    return;
	    }
	if (noext(0))
	    return;
	device = Devs.Button;
	event_mode = ReplayThisDevice;
	(void) warppointer(display, grabwin, 1, 1);

	XCALL;
	if (!ispfrozen(display))
		PASS;
	else {
		report("device was frozen after ReplayThisDevice");
		FAIL;
	}
>>ASSERTION Good B 3
When the
.A event_mode
argument is
.S ReplayThisDevice
and the device is frozen twice by the client on behalf of two separate
grabs, then a call to xname thaws for both grabs.
>>STRATEGY
If extensions are available:
  Freeze device with XGrabKeyboard.
  Set up passive grab.
  Freeze device by activating grab with a button press.
  Call xname with event_mode of ReplayThisDevice.
  Verify that device was released.
>>CODE
int 	key;

	if (!grabstartup())
	    {
	    UNTESTED;
	    return;
	    }
	device = Devs.Button;
	if (noext(1))
		return;

	(void) warppointer(display, grabwin, 1, 1);
	XSync(display,0);

	key = getkeycode(display);
	XGrabKey(display, key, 0, grabwin, False, GrabModeSync, GrabModeAsync);
	XGrabDeviceButton(display, device, Button1, AnyModifier, NULL,  grabwin,
		False, 0, NULL, GrabModeSync, GrabModeAsync);
	devicebuttonpress(display, Devs.Button, Button1);
	keypress(display, key);
	XSync(display,0);

	event_mode = ReplayThisDevice;
	XCALL;

	if (ispfrozen(display)) {
		report("device was not released after ReplayThisDevice");
		report("  and the device was frozen by two grabs.");
		FAIL;
	} else
		CHECK;

	keyrel(display, key);
	devicebuttonrel(display, Devs.Button, Button1);
	CHECKPASS(1);

>>ASSERTION Good B 3
When the
.A event_mode
argument is
.S SyncAll
and both the grabbed device and the other devices are frozen by the client,
then event processing for both devices continues normally until the next
.S DeviceButtonPress ,
.S DeviceButtonRelease ,
.S DeviceKeyPress ,
or
.S DeviceKeyRelease
event is reported to the client for a grabbed device
at which time all devices again appear to freeze,
unless the reported event causes the grab to be released.
>>STRATEGY
If no extensions:
  Touch test for SyncAll.
else
  Create grabwindow.
  Select events on grab window.
  Freeze and grab device by calling XGrabDevice.
  Freeze and grab keyboard by calling XGrabKeyboard.
  Check that device is frozen.

  Call xname with event_mode of SyncAll.
  Verify that device has been released.

  Press button.
  Verify that device is frozen.
  Verify that keyboard is frozen.

  Call xname with event_mode of SyncAll.
  Check device released.
  Release button.
  Verify that device is frozen.
  Verify that keyboard is frozen.

  Call xname with event_mode of SyncAll.
  Check device released.
  Press key.
  Verify that device is frozen.
  Verify that keyboard is frozen.

  Call xname with event_mode of SyncAll.
  Check device released.
  Release key.
  Verify that device is frozen.
  Verify that keyboard is frozen.
>>EXTERN

/*
 * Returns True if the keyboard is frozen.
 */
static
iskfrozen(display)
Display	*display;
{
XEvent	ev;
Window	win;
int 	res;
int 	key;

	XSync(display, True); /* Flush previous events */
	key = getkeycode(display);

	/*
	 * Try to provoke a keypress on win.
	 */
	win = defwin(display);
	XSelectInput(display, win, KeyPressMask);
	(void) warppointer(display, win, 1, 1);
	keypress(display, key);
	if (XCheckMaskEvent(display, (long)KeyPressMask, &ev))
		res = False;
	else
		res = True;

	return(res);
}

/*
 * Set up for SyncAll tests grab and freeze both device and keyboard.
 */
bothset()
{

	device = Devs.Button;
	XUngrabDevice(display, device, CurrentTime);
	XUngrabKeyboard(display, CurrentTime);

	(void) warppointer(display, grabwin, 5, 5);

	XGrabDevice(display, device, grabwin, False, 3, 
		class, GrabModeSync, GrabModeAsync, CurrentTime);

	XGrabKeyboard(display, grabwin, False, GrabModeSync, GrabModeSync,
		    CurrentTime);

	if (!ispfrozen(display)) {
	    delete("Could not freeze device");
	    return;
	    }
	/*
	 * Can't check for the keyboard being frozen here since that requires
	 * pressing a key - and that would release the grab.
	 */
    }

>>CODE
int 	key;

	if (!grabstartup())
	    {
	    UNTESTED;
	    return;
	    }
	event_mode = SyncAll;

	if (noext(1)) {
	    XCALL;

	untested("There is no reliable test method, but a touch test was performed");
	    return;
	} else
	    CHECK;


	bothset();
	XCALL;

	if (ispfrozen(display)) {
	    report("SyncAll did not release device and keyboard");
	    FAIL;
	} else
	    CHECK;

	/* 1. Button press */
	devicebuttonpress(display, Devs.Button, Button1);
	if (ispfrozen(display))
	    CHECK;
	else {
	    report("device was not re-frozen by a button press after SyncAll");
	    FAIL;
	}
	if (iskfrozen(display))
	    CHECK;
	else {
	    report("Keyboard was not re-frozen by a button press after SyncAll");
	    FAIL;
	}

	/* Allow events again for next part */
	bothset();
	XCALL;
	if (ispfrozen(display)) {
	    report("SyncAll did not release device and keyboard");
	    FAIL;
	} else
	    CHECK;

	/* 2. Button release */
	devicebuttonrel(display, Devs.Button, Button1);
	if (ispfrozen(display))
	    CHECK;
	else {
	    report("device was not re-frozen by a button release after SyncAll");
	    FAIL;
	}
	if (iskfrozen(display))
	    CHECK;
	else {
	    report("Keyboard was not re-frozen by a button release after SyncAll");
	    FAIL;
	}

	/* Allow events again for next part */
	bothset();
	XCALL;
	if (ispfrozen(display)) {
	    report("SyncAll did not release device and keyboard");
	    FAIL;
	} else
	    CHECK;

	/* 3. Press key. */
	key = getkeycode(display);
	keypress(display, key);
	if (ispfrozen(display))
	    CHECK;
	else {
	    report("device was not re-frozen by a key press after SyncAll");
	    FAIL;
	}
	if (iskfrozen(display))
	    CHECK;
	else {
	    report("Keyboard was not re-frozen by a key press after SyncAll");
	    FAIL;
	}

	/* Allow events again for next part */
	bothset();
	XCALL;
	if (ispfrozen(display)) {
	    report("SyncAll did not release device and keyboard");
	    FAIL;
	} else
	    CHECK;

	/* 4. Key release. */
	keyrel(display, key);
	if (ispfrozen(display))
	    CHECK;
	else {
	    report("device was not re-frozen by a key release after SyncAll");
	    FAIL;
	}
	if (iskfrozen(display))
	    CHECK;
	else {
	    report("Keyboard was not re-frozen by a key release after SyncAll");
	    FAIL;
	}

	devicerelbuttons(Devs.Button);
	relalldev();
	CHECKPASS(13);
>>ASSERTION Good B 3
When the
.A event_mode
argument is
.S SyncAll
and an event has caused the grab on one device to be released
and a subsequent event is reported for another device that does
not cause the grab to be released,
then all devices are again frozen.
>>STRATEGY
If extension available:
  Grab and freeze Keyboard with XGrabKeyboard.
  Set passive grab on button with device_mode GrabModeSync.
  Activate device grab.

  Call xname with event_mode SyncAll.
  Release button to release device grab.
  Check device not frozen.

  Press key.
  Verify that device and keyboard are frozen.
>>CODE
int 	key;

	if (noext(1))
	    return;

	if (!grabstartup())
	    {
	    UNTESTED;
	    return;
	    }
	device = Devs.Button;
	XGrabKeyboard(display, grabwin, False, GrabModeSync, GrabModeSync,
	    CurrentTime);
	XGrabButton(display, Button1, 0, grabwin,
	    False, PointerMotionMask, GrabModeSync, GrabModeSync,
	    None, None);

	(void) warppointer(display, grabwin, 1, 1);
	devicebuttonpress(display, Devs.Button, Button1);

	if (ispfrozen(display))
	    CHECK;
	else {
	    delete("Could not freeze device and keyboard");
	    return;
	}

	event_mode = SyncAll;
	XCALL;

	/*
	 * Release device grab.
	 */
	devicebuttonrel(display, Devs.Button, Button1);
	if (ispfrozen(display)) {
	    report("device remained frozen after releasing button");
	    FAIL;
	} else
	    CHECK;

	key = getkeycode(display);
	keypress(display, key);
	if (ispfrozen(display))
	    CHECK;
	else {
		report("device was not re-frozen by an event from the keyboard after the device grab was released.");
		FAIL;
	}
	if (iskfrozen(display))
		CHECK;
	else {
		report("Keyboard was not re-frozen by an event from the keyboard after the device grab was released");
		FAIL;
	}

	relalldev();
	devicerelbuttons(Devs.Button);
	CHECKPASS(4);
>>ASSERTION Good B 3
When the
.A event_mode
argument is
.S SyncAll
and the grabbed device or the other devices are frozen twice
by the client on behalf of two separate grabs,
then a call to xname thaws for both grabs.
>>STRATEGY
If extensions are available:
  Grab and freeze device.
  Grab keyboard and freeze device.

  Call xname with event_mode of SyncAll.
  Verify that device and keyboard are thawed.
>>CODE

	if (noext(0))
		return;

	if (!grabstartup())
	    {
	    UNTESTED;
	    return;
	    }
	device = Devs.Button;
	XGrabDevice(display, device, grabwin, True, 3, 
	    class, GrabModeSync, GrabModeAsync, CurrentTime);
	XGrabKeyboard(display, grabwin, False, GrabModeSync, GrabModeSync,
		CurrentTime);

	event_mode = SyncAll;
	XCALL;

	if (ispfrozen(display)) {
		report("device was not thawed by SyncAll when device was frozen");
		report("  on behalf of two grabs");
		FAIL;
	} else
		CHECK;
	if (iskfrozen(display)) {
		report("Keyboard was not thawed by SyncAll when device was frozen");
		report("  on behalf of two grabs");
		FAIL;
	} else
		CHECK;

	CHECKPASS(2);
>>ASSERTION Good B 3
When the
.A event_mode
argument is
.S SyncAll
and neither the grabbed device or the other devices are frozen by the client
or none are grabbed by the client, then a call to xname has no effect.
>>STRATEGY
If extensions available:
  Call xname with SyncAll.
  Press button.
  Verify that device and keyboard are not frozen.
>>CODE

	if (noext(0))
		return;

	if (!grabstartup())
	    {
	    UNTESTED;
	    return;
	    }
	device = Devs.Button;
	event_mode = SyncAll;
	XCALL;

	devicebuttonpress(display, Devs.Button, Button1);
	if (ispfrozen(display)) {
		report("device was frozen by button press after SyncAll");
		report("  even though there were no grabs active");
		FAIL;
	} else
		CHECK;
	if (iskfrozen(display)) {
		report("Keyboard was frozen by button press after SyncAll");
		report("  even though there were no grabs active");
		FAIL;
	} else
		CHECK;

	devicerelbuttons(Devs.Button);
	CHECKPASS(2);
>>ASSERTION Good B 3
When the
.A event_mode
argument is
.S AsyncAll
and the grabbed device and the other devices are frozen by the
client, then event processing for all devices is resumed normally.
>>STRATEGY
If no extensions:
  Touch test for AsyncAll.
else
  Grab and freeze keyboard and device.
  Call xname with AsyncAll.
  Verify that device is released.
  Verify that keyboard is released.
>>CODE

	if (!grabstartup())
	    {
	    UNTESTED;
	    return;
	    }
	if (noext(0))
	    return;
	event_mode = AsyncAll;
	device = Devs.Button;
	if (noext(0)) {
		XCALL;
		untested("There is no reliable test method, but a touch test was performed");
		return;
	} else
		CHECK;

	bothset();
	XCALL;

	if (ispfrozen(display)) {
		report("device remained frozen after AsyncAll");
		FAIL;
	} else
		CHECK;
	if (iskfrozen(display)) {
		report("Keyboard remained frozen after AsyncAll");
		FAIL;
	} else
		CHECK;

	CHECKPASS(3);
>>ASSERTION Good B 3
When the
.A event_mode
argument is
.S AsyncAll
and either the grabbed device or another device is not frozen by the client,
then a call to xname has no effect.
>>STRATEGY
If extensions available:
  Grab and freeze device.
  Call xname with AsyncAll.
  Verify that device is not released.
>>CODE

	if (noext(0))
		return;

	if (!grabstartup())
	    {
	    UNTESTED;
	    return;
	    }
	device = Devs.Button;
	grabfreezedevice(display, CurrentTime);

	event_mode = AsyncAll;
	XCALL;

	if (ispfrozen(display))
		CHECK;
	else {
		report("device was released by AsyncAll, although keyboard was not frozen");
		FAIL;
	}
	XUngrabDevice(display, device, CurrentTime);
	XSync(display,0);
	CHECKPASS(1);
>>ASSERTION Good B 3
When the
.A event_mode
argument is
.S AsyncAll
and the grabbed device or other devices
is frozen twice by the client on behalf of two separate grabs,
then a call to xname thaws for both grabs.
>>STRATEGY
If extensions available:
  Freeze device and keyboard by calling XGrabPointer.
  Freeze device and keyboard again by calling XGrabKeyboard.

  Call xname with AsyncAll.
  Verify that device and keyboard are not frozen.
>>CODE

	if (noext(0))
		return;

	if (!grabstartup())
	    {
	    UNTESTED;
	    return;
	    }
	device = Devs.Button;
	XGrabDevice(display, device, grabwin, True, 3, 
	    class, GrabModeSync, GrabModeAsync, CurrentTime);
	XGrabKeyboard(display, grabwin, False, GrabModeSync, GrabModeSync,
		CurrentTime);

	event_mode = AsyncAll;
	XCALL;

	if (ispfrozen(display)) {
		report("device remained frozen after AsyncAll");
		report("  when it was frozen twice");
		FAIL;
	} else
		CHECK;
	if (iskfrozen(display)) {
		report("Keyboard remained frozen after AsyncAll");
		report("  when it was frozen twice");
		FAIL;
	} else
		CHECK;

	CHECKPASS(2);
>>ASSERTION Good B 3
When the
.A event_mode
argument is
.S AsyncThisDevice ,
.S SyncThisDevice ,
or
.S ReplayThisDevice ,
then a call to xname has no effect on the
processing of keyboard events.
>>STRATEGY
If extensions are available:
  Grab and freeze the keyboard.
  For each event_mode AsyncThisDevice SyncThisDevice ReplayThisDevice
    Call xname.
    Verify that keyboard is still frozen.
>>CODE
static	int 	modes[] = {
	AsyncThisDevice, SyncThisDevice, ReplayThisDevice};
int 	i;

	if (noext(0))
		return;

	if (!grabstartup())
	    {
	    UNTESTED;
	    return;
	    }
	device = Devs.Button;
	XGrabKeyboard(display, grabwin, False, GrabModeAsync, GrabModeSync,
		CurrentTime);

	for (i = 0; i < NELEM(modes); i++) {
		event_mode = modes[i];
		XCALL;
		if (iskfrozen(display))
			CHECK;
		else {
			report("Keyboard was released when event_mode was %s",
				alloweventmodename(modes[i]));
			FAIL;
		}
	}

	CHECKPASS(NELEM(modes));
>>ASSERTION Good B 3
When the
.A event_mode
argument is
.S AsyncOtherDevices
then a call to xname has no effect on the
processing of device events.
>>STRATEGY
Grab and freeze device.
  Call xname with mode AsyncOtherDevices.
  Verify that device is still frozen.
>>CODE

	if (!grabstartup())
	    {
	    UNTESTED;
	    return;
	    }
	if (noext(0))
	    return;
	device = Devs.Button;
	grabfreezedevice(display, time);

	event_mode = AsyncOtherDevices;
	XCALL;
	if (ispfrozen(display))
		PASS;
	else {
		report("device was released when event_mode was AsyncOtherDevices");
		FAIL;
	}
	XUngrabDevice(display, device, CurrentTime);
	XSync(display,0);

>>ASSERTION Bad B 3
A call to xname will fail with a BadDevice error if an invalid device
is specified.
>>STRATEGY
Make the call with an invalid device.
>>CODE baddevice
XDevice nodevice;
XID baddevice;
int ret;

	if (!grabstartup())
	    {
	    UNTESTED;
	    return;
	    }
	BadDevice (display, baddevice);
	nodevice.device_id = -1;
	device = &nodevice;

	ret = XCALL;

	if (geterr() == baddevice)
		PASS;
	else
		FAIL;

>>ASSERTION Bad B 3
A call to xname will fail with a BadValue error if the specified
mode is invalid.
>>STRATEGY
Invoke xname with an invalid mode.
Verify BadValue
>>CODE BadValue
Display	*client2;
int ret;

	if (!grabstartup())
	    {
	    UNTESTED;
	    return;
	    }
	device = Devs.Button;
	event_mode = -1;

	ret = XCALL;

	if (geterr() == BadValue)
		PASS;
	else
		FAIL;

>>ASSERTION Good B 3
Touch test for SyncAll mode.
>>STRATEGY
>>CODE
int ret;

	if (!grabstartup())
	    {
	    UNTESTED;
	    return;
	    }
	device = Devs.Button;
	event_mode = SyncAll;

	ret = XCALL;

	if (geterr() == Success)
		PASS;
	else
		FAIL;

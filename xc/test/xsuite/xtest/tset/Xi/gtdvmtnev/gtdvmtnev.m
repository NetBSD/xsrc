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
 * $XConsortium: gtdvmtnev.m,v 1.14 94/09/06 21:11:08 dpw Exp $
 */
>>TITLE XGetDeviceMotionEvents XINPUT
XDeviceTimeCoord *
XGetDeviceMotionEvents(display, device, start, stop, nevents_return, mode_return, axis_count_return)
Display *display = Dsp;
XDevice *device;
Time start = CurrentTime;
Time stop = CurrentTime;
int *nevents_return = &_nevents_return;
int *mode_return = &_mode_return;
int *axis_count_return = &_axis_count_return;
>>EXTERN
static	int	_nevents_return;
static	int	_mode_return;
static	int	_axis_count_return;
extern	ExtDeviceInfo Devs;
>>ASSERTION Good D 1
If
the implementation supports a more complete
history of the device motion than is reported by event notification:
a call to xname
returns all events in the motion history buffer
that fall between the
.A start
and
.A stop
times, inclusive,
and sets
.A nevents_return
to the number of events returned.
>>STRATEGY
If a device motion buffer is not supported, return.
>>CODE
/* If a device motion buffer is not supported, return. */
	if (!config.displaymotionbuffersize)
		unsupported("Device motion buffer is not supported.");
	else
		untested("There is no known portable test method for this assertion");
>>ASSERTION Good C
If
the implementation does not support a more complete
history of device motion than is reported by event notification:
a call to xname returns no events.
>>STRATEGY
If a device motion buffer is supported, return.
Call XGetDeviceMotionEvents.
Verify that no events were returned.
>>CODE
XDeviceTimeCoord *tc;

/* If a device motion buffer is supported, return. */
	if (config.displaymotionbuffersize != 0) {
		report("Device motion buffer is supported.");
		UNSUPPORTED;
		return;
	}
	else
		CHECK;
	if (!Setup_Extension_DeviceInfo(ValMask))
	    {
	    untested("%s: No input extension valuator device.\n", TestName);
	    return;
	    }
	start = 0;
	stop = CurrentTime;
	*nevents_return = 1;
/* Call XGetDeviceMotionEvents. */
	device = Devs.Valuator;
	tc = XCALL;
/* Verify that no events were returned. */
	if (tc != (XDeviceTimeCoord *) NULL) {
		report("Returned 0x%x, expected NULL", tc);
		FAIL;
		XFree((char*)tc);
	}
	else
		CHECK;
	if (*nevents_return != 0) {
		report("Returned %d, expected 0", *nevents_return);
		FAIL;
	}
	else
		CHECK;
	CHECKPASS(3);
>>ASSERTION Good B 3
When
.A start
is later than
.A stop ,
then a call to xname returns no events.
>>STRATEGY
Set stop to current time.
Call XGetDeviceMotionEvents with start greater than stop.
Verify that no events were returned.
>>CODE
int i, val=0;
XDeviceTimeCoord *tc;

	if (!Setup_Extension_DeviceInfo(ValMask))
	    {
	    untested("%s: No input extension valuator device.\n", TestName);
	    return;
	    }
	if (noext(0))
	    return;
/* Set stop to current time. */
	stop = gettime(display);
/* Call XGetDeviceMotionEvents with start greater than stop. */
	start = stop + 1;
	device = Devs.Valuator;
	for (i=0; i<20; i++)
	    SimulateDeviceMotionEvent (display, Devs.Valuator, False, 1, &val, 0);
	tc = XCALL;
/* Verify that no events were returned. */
	if (tc != (XDeviceTimeCoord *) NULL) {
		report("Returned 0x%x, expected NULL", tc);
		XFreeDeviceMotionEvents(tc);
		FAIL;
	}
	else
		CHECK;
	if (*nevents_return != 0) {
		report("Returned %d, expected 0", *nevents_return);
		FAIL;
	}
	else
		CHECK;
	CHECKPASS(2);
>>ASSERTION Good B 3
When
.A start
is in the future,
then a call to xname returns no events.
>>STRATEGY
Set stop to current time.
Set start to a future time.
Call XGetDeviceMotionEvents.
Verify that no events were returned.
>>CODE
int i, val=0;
XDeviceTimeCoord *tc;

	if (!Setup_Extension_DeviceInfo(ValMask))
	    {
	    untested("%s: No input extension valuator device.\n", TestName);
	    return;
	    }
/* Set stop to current time. */
	stop = CurrentTime;
/* Set start to a future time. */
	start = gettime(display) + 10000;
/* Call XGetDeviceMotionEvents. */
	device = Devs.Valuator;
	for (i=0; i<20; i++)
	    SimulateDeviceMotionEvent (display, Devs.Valuator, False, 1, &val, 0);
	tc = XCALL;
/* Verify that no events were returned. */
	if (tc != (XDeviceTimeCoord *) NULL) {
		report("Returned 0x%x, expected NULL", tc);
		XFreeDeviceMotionEvents(tc);
		FAIL;
	}
	else
		CHECK;
	if (*nevents_return != 0) {
		report("Returned %d, expected 0", *nevents_return);
		FAIL;
	}
	else
		CHECK;

/* Set stop to future time. */
	stop = gettime(display) + 10000;
/* Set start to a past time. */
	start = gettime(display) - 10000;
/* Call XGetDeviceMotionEvents. */
	device = Devs.Valuator;
	for (i=0; i<20; i++)
	    SimulateDeviceMotionEvent (display, Devs.Valuator, False, 1, &val, 0);
	tc = XCALL;
/* Verify that no events were returned. */
	if (tc != (XDeviceTimeCoord *) NULL) {
		report("Returned 0x%x, expected NULL", tc);
		XFreeDeviceMotionEvents(tc);
		FAIL;
	}
	else
		CHECK;
	if (*nevents_return != 0) {
		report("Returned %d, expected 0", *nevents_return);
		FAIL;
	}
	else
		CHECK;
	CHECKPASS(4);
>>ASSERTION Good B 3
A call to xname will update the DeviceMotionHint state and cause another
device motion event to be sent to interested clients, when more device motion
events are generated.
>>STRATEGY
Select DeviceMotionNotify and DevicePointerMotionHint classes.
Generate some motion events, verify only one is sent.
Now call xname.
Generate some motion events, verify another one is sent.
>>CODE
XID dmn, dmnh;
XEventClass classes[2];
Window w;
int axes=0, n, ret, i;
XEvent ev;
XDeviceMotionEvent *d;

	if (!Setup_Extension_DeviceInfo(ValMask))
	    {
	    untested("%s: Required input extension device not present.\n", 
		TestName);
	    return;
	    }
	if (noext(0))
	    return;
	device = Devs.Valuator;
/* Set stop to current time. */
	stop = CurrentTime;
/* Set start to a past time. */
	start = gettime(display);
	DeviceMotionNotify(device, dmn, classes[0]);
	DevicePointerMotionHint(device, dmnh, classes[1]);
	w = defwin(display);
	XSelectExtensionEvent(display, w, classes, 2);
	warppointer (display, w, 1, 1);

	XSync(display,1);
	for (i=0; i<10; i++)
	    SimulateDeviceMotionEvent(display, device, False, 1, &axes, 0);
	XSync(display,0);

	n = getevent(display, &ev);
	if (n != 1)
	    {
	    report("Expecting one event with DeviceMotionHint, got %d",n);
	    FAIL;
	    }
	else
	    CHECK;
	if (ev.type != dmn)
	    {
	    report("Expecting DeviceMotionNotify event, got event type %d",
		ev.type);
	    FAIL;
	    }
	else
	    CHECK;
	d = (XDeviceMotionEvent *) &ev;
	if (d->is_hint != True)
	    {
	    report("Expecting is_hint = True, was False");
	    FAIL;
	    }
	else
	    CHECK;

	XCALL;
	if ((ret = geterr()) == Success)
		CHECK;
	else
	    {
	    report("Expecting Success, got error %d", ret);
	    FAIL;
	    }

	XSync(display,1);
	for (i=0; i<10; i++)
	    SimulateDeviceMotionEvent(display,  device, False, 1, &axes, 0);
	XSync(display,0);

	n = getevent(display, &ev);
	if (n != 1)
	    {
	    report("Expecting one event with DeviceMotionHint, got %d",n);
	    FAIL;
	    }
	else
	    CHECK;
	if (ev.type != dmn)
	    {
	    report("Expecting DeviceMotionNotify event, got event type %d",
		ev.type);
	    FAIL;
	    }
	else
	    CHECK;
	d = (XDeviceMotionEvent *) &ev;
	if (d->is_hint != True)
	    {
	    report("Expecting is_hint = True, was False");
	    FAIL;
	    }
	else
	    CHECK;

	CHECKPASS(7);
>>ASSERTION Good B 1
>>#NOTE	This is not testable since the motion history buffer is filled
>>#NOTE in by DDX.
A call to xname with
.A stop
in the future,
is equivalent to specifying
a value of
.S CurrentTime
for
.A stop .
>>ASSERTION Good D 1
On a call to xname
the
.M x
and
.M y
members of the events returned
are set to the coordinates of the device
and the
.M time
member is set to the time the device reached this coordinate.
>>ASSERTION Bad B 3
When xname is invoked with a device that has no valuators, a BadMatch error 
results.
>>STRATEGY
Specify a device with no valuators.
Verify that a BadMatch error results.
>>CODE BadMatch
XDeviceTimeCoord *tc;

	if (!Setup_Extension_DeviceInfo(NValsMask))
	    {
	    untested("%s: No input extension device without valuators.\n", TestName);
	    return;
	    }
	device = Devs.NoValuators;
/* Set stop to current time. */
	stop = CurrentTime;
/* Set start to a future time. */
	start = gettime(display) + 10000;
/* Call XGetDeviceMotionEvents. */
	tc = XCALL;

	if (geterr() == BadMatch)
		PASS;
	else
		FAIL;
>>ASSERTION Bad B 3
When xname is invoked with an invalid device, a BadDevice error 
results.
>>STRATEGY
Specify an invalid device.
Verify that a BadDevice error results.
>>CODE baddevice
XID baddevice;
XDevice nodevice;
XDeviceTimeCoord *tc;
int ximajor, first, err;

	if (!XQueryExtension (display, INAME, &ximajor, &first, &err))
	    {
	    untested("%s: Input extension not supported.\n", TestName);
	    return;
	    }

	BadDevice(display,baddevice);
	nodevice.device_id = -1;
	device = &nodevice;
/* Set stop to current time. */
	stop = CurrentTime;
/* Set start to a future time. */
	start = gettime(display) + 10000;
/* Call XGetDeviceMotionEvents. */
	tc = XCALL;

	if (geterr() == baddevice)
		PASS;
	else
		FAIL;

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
 * $XConsortium: slextevnt.m,v 1.17 94/09/06 20:52:11 dpw Exp $
 */
>>TITLE XSelectExtensionEvent XINPUT
void
XSelectExtensionEvent(display, w, class, count)
Display *display = Dsp;
Window w;
XEventClass *eclass;
int ecount;
>>EXTERN
extern ExtDeviceInfo Devs;
extern int MinKeyCode;
extern int NumButtons;
XDevice *focusdevice;
Display *fdisplay;

getfocusdevice()
{
int i, j, ndevices;
XInputClassInfo *ip;
XDeviceInfo *list;
XDevice *device;
int ximajor, first, err;

	fdisplay = opendisplay();
	if (!XQueryExtension (fdisplay, INAME, &ximajor, &first, &err)) {
	    untested("%s: Input extension not supported.\n", TestName);
	    return;
	    }

	list = XListInputDevices (fdisplay, &ndevices);
	for (i=0; i<ndevices; i++, list++)
	    if (list->use == IsXExtensionDevice)
		{
		device = XOpenDevice(fdisplay, list->id);
		for (j=0, ip=device->classes; j<device->num_classes; j++, ip++)
		    if (ip->input_class == FocusClass)
			{
			focusdevice = device;
			break;
			}
		}
}
>>ASSERTION Good B 3
A call to xname
requests that the X server report the events for window
.A w
matching
.A event_mask .
>>STRATEGY
Create client1.
Create window with client1.
Select no events with client1 on this window.
Create client2.
Select DeviceFocusIn events with client2 on this window.
Map window.
XSync(.., False) on both clients to ensure generated events have come in.
Verify that client1 received no events.
Verify that client2 received a single MapNotify event for this window.
Verify that client2 received no other events.
>>CODE
Display *client1;
Display *client2;
XEvent	event;
int	n;
XEventClass noextensionevent, devicefocusin;
int	type;

/* Create client1. */
	client1 = opendisplay();
	if (client1 == (Display *) NULL) {
		delete("Can not open display");
		return;
	}
	else
		CHECK;
	getfocusdevice();
	if (!focusdevice) {
	    report("%s: Required input devices not present\n",TestName);
	    UNTESTED;
	    return;
	    }
	NoExtensionEvent (focusdevice, type, noextensionevent);
/* Create window with client1. */
	w = mkwin(client1, (XVisualInfo *) NULL, (struct area *) NULL, False);
/* Select no events with client1 on this window. */
	BASIC_STARTCALL(client1);
	XSelectExtensionEvent(client1, w, &noextensionevent, 1);
	BASIC_ENDCALL(client1, Success);
/* Create client2. */

	client2 = opendisplay();
	if (client2 == (Display *) NULL) {
		delete("Can not open display");
		return;
	}
	else
		CHECK;
/* Select MapNotify events with client2 on this window. */
	BASIC_STARTCALL(client2);
	DeviceFocusIn (focusdevice, type, devicefocusin);
	XSelectExtensionEvent(client2, w, &devicefocusin, 1);
	BASIC_ENDCALL(client2, Success);
/* Map window. */
	XSync(client1, True);
	XSync(client2, True);
	XMapWindow(client1, w);
	XSetDeviceFocus (client1, focusdevice, w, RevertToPointerRoot, CurrentTime);
/* XSync(.., False) on both clients to ensure generated events have come in. */
	XSync(client1, False);
	XSync(client2, False);
	trace("Mapping window with DeviceFocusIn selected, expecting DeviceFocusIn.");
/* Verify that client1 received no events. */
	if ((n=XPending(client1)) > 0) {
		XNextEvent(client1, &event);
		report("%d unexpected event%s (first %s) %s delivered to client1.",
			n, (n==1)?"":"s", eventname(event.type), (n==1)?"was":"were");
		FAIL;
	}
	else
		CHECK;
/* Verify that client2 received a single DeviceFocusIn event for this window. */
	if ((n=XPending(client2)) > 0) {
		XNextEvent(client2, &event);
		if (event.type != type)
		    {
		    report("%d unexpected event%s (first %s) %s delivered to client1.",
			n, (n==1)?"":"s", eventname(event.type), (n==1)?"was":"were");
		    FAIL;
		    }
		else
		    CHECK;
	}
	else
		CHECK;
/* Verify that client2 received no other events. */
	if ((n=XPending(client2)) > 0) {
		XNextEvent(client2, &event);
		report("%d unexpected event%s (first %s) %s delivered to client2.",
			n, (n==1)?"":"s", eventname(event.type), (n==1)?"was":"were");
		FAIL;
	}
	else
		CHECK;

	CHECKPASS(5);
>>ASSERTION Good B 3
A call to xname
overrides the event mask attribute set during any previous call to
xname.
>>STRATEGY
Create window with no events selected.
Call XGetSelectedExtensionEvents to get event mask for this window.
Verify event mask is as expected.
Call XSelectExtensionEvent to change event mask to DeviceFocusIn.
Call XGetSelectedExtensionEvents to get new event mask for this window.
Verify event mask changed as expected.
Call XSelectExtensionEvent to change event mask to NoEventMask.
Call XGetSelectedExtensionEvents to get event mask for this window.
Verify event mask is as expected.
Call XSelectExtensionEvent to change event mask to ALLEVENTS.
Call XGetSelectedExtensionEvents to get new event mask for this window.
Verify event mask changed as expected.
Call XSelectExtensionEvent to change event mask to StructureNotifyMask.
Call XGetSelectedExtensionEvents to get new event mask for this window.
Verify event mask changed as expected.
>>CODE
int type, this_client_count, all_clients_count;
XEventClass *this_client, *all_clients;
XEventClass noextensionevent, devicefocusin;


/* Create window with no events selected. */
	getfocusdevice();
	if (!focusdevice) {
	    report("%s: Required input devices not present\n",TestName);
	    UNTESTED;
	    return;
	    }
	DeviceFocusIn (focusdevice, type, devicefocusin);
	NoExtensionEvent (focusdevice, type, noextensionevent);
	w = mkwin(display, (XVisualInfo *) NULL, (struct area *) NULL, False);
/* Call XGetSelectedExtensionEvents to get event mask for this window. */
	if (XGetSelectedExtensionEvents(display, w, &this_client_count,
		&this_client, &all_clients_count, &all_clients)) {
		delete("A call to XGetSelectedExtensionEvents failed.");
		return;
	}
	else
		CHECK;
/* Verify event mask is as expected. */
	if (this_client_count != 0) {
		delete("Unexpected event mask value.");
		return;
	}
	else
		CHECK;
/* Call XSelectExtensionEvent to change event mask to DeviceFocusIn. */
	eclass = &devicefocusin;
	ecount = 1;
	XCALL;
/* Call XGetSelectedExtensionEvents to get new event mask for this window. */
	if (XGetSelectedExtensionEvents(display, w, &this_client_count,
		&this_client, &all_clients_count, &all_clients)) {
		delete("A call to XGetSelectedExtensionEvents failed.");
		return;
	}
	else
		CHECK;
/* Verify event mask changed as expected. */
	if (*this_client != devicefocusin) {
		report("Event mask incorrect first call.");
		FAIL;
	}
	else
		CHECK;
/* Call XChangeWindowAttributes to change event mask to NoEventMask. */
	eclass = &noextensionevent;
	ecount = 1;
	XCALL;
/* Call XGetSelectedExtensionEvents to get event mask for this window. */
	if (XGetSelectedExtensionEvents(display, w, &this_client_count,
		&this_client, &all_clients_count, &all_clients)) {
		delete("A call to XGetSelectedExtensionEvents failed.");
		return;
	}
	else
		CHECK;
/* Verify event mask is as expected. */
	if (this_client_count != 0) {
		delete("Unexpected (non-empty) event mask value after XSelectExtensionEvent.");
		return;
	}
	else
		CHECK;
/* Call XSelectExtensionEvent to change event mask to ALLEVENTS. */
	eclass = &devicefocusin;
	ecount = 1;
	XCALL;
/* Call XGetSelectedExtensionEvents to get new event mask for this window. */
	if (XGetSelectedExtensionEvents(display, w, &this_client_count,
		&this_client, &all_clients_count, &all_clients)) {
		delete("A call to XGetSelectedExtensionEvents failed.");
		return;
	}
	else
		CHECK;
/* Verify event mask changed as expected. */
	if (*this_client != devicefocusin) {
		report("Event mask incorrect after second call.");
		FAIL;
	}
	else
		CHECK;
/* Call XSelectExtensionEvent to change event mask to StructureNotifyMask. */
	eclass = &devicefocusin;
	ecount = 1;
	XCALL;
/* Call XGetSelectedExtensionEvents to get new event mask for this window. */
	if (XGetSelectedExtensionEvents(display, w, &this_client_count,
		&this_client, &all_clients_count, &all_clients)) {
		delete("A call to XGetSelectedExtensionEvents failed.");
		return;
	}
	else
		CHECK;
/* Verify event mask changed as expected. */
	if (*this_client != devicefocusin) {
		report("Event mask incorrect after third call.");
		FAIL;
	}
	else
		CHECK;

	CHECKPASS(10);

>>ASSERTION Good B 3
A call  to  xname does not change the events selected
for other clients.
>>STRATEGY
Create client1.
Create window with client1.
Select NoExtensionEvent events with client1 on this window.
Call XGetSelectedEvents to get selected events for client1 for window.
Verify events are as expected.
Create client2.
Select all events with client2 on this window.
Call XGetSelectedExtensionEvents to get selected events for client2 for window.
Verify events are as expected.
Call XGetSelectedEvents to get selected events for client1 for window.
Verify selected events have not changed.
Select DeviceKeyPress events with client1 on this window.
Call XGetSelectedEvents to get selected events for client1 for window.
Verify events are as expected.
Call XGetSelectedEents to get selected events for client2 for window.
Verify selected events have not changed.
>>CODE
Display *client1;
Display *client2;
int tcount, acount;
XEventClass *this_client, *all_clients, dkpclass, nevclass, classes[5];
XID dkp, nev, tmp;

	if (!Setup_Extension_DeviceInfo(KeyMask))
	    {
	    untested("%s: No input extension key device.\n", TestName);
	    return;
	    }
	DeviceKeyPress(Devs.Key, dkp, dkpclass);
	NoExtensionEvent(Devs.Key, nev, nevclass);
	classes[0] = dkpclass;
	DeviceKeyRelease(Devs.Key, tmp, classes[1]);
	DeviceStateNotify(Devs.Key, tmp, classes[2]);
	DeviceMappingNotify(Devs.Key, tmp, classes[3]);
	ChangeDeviceNotify(Devs.Key, tmp, classes[4]);
/* Create client1. */
	client1 = opendisplay();
	if (client1 == (Display *) NULL) {
		delete("Can not open display");
		return;
	}
	else
		CHECK;
/* Create window with client1. */
	w = mkwin(client1, (XVisualInfo *) NULL, (struct area *) NULL, False);
/* Select NoEventMask events with client1 on this window. */
	BASIC_STARTCALL(client1);
	XSelectExtensionEvent(client1, w, &nevclass, 1);
	BASIC_ENDCALL(client1, Success);
/* Call XGetSelectedExtensionEvents to get event mask for client1 for window. */
	if (XGetSelectedExtensionEvents(client1, w, &tcount, &this_client, 
	    &acount, &all_clients)) {
		delete("A call to XGetSelectedExtensionEvents failed.");
		return;
	}
	else
		CHECK;
/* Verify event mask is as expected. */
	if (tcount != 0) {
		delete("Unexpected events, count %d instead of 0.", tcount);
		return;
	}
	else
		CHECK;
/* Create client2. */
	client2 = opendisplay();
	if (client2 == (Display *) NULL) {
		delete("Can not open display");
		return;
	}
	else
		CHECK;
/* Select all events with client2 on this window. */
	BASIC_STARTCALL(client2);
	XSelectExtensionEvent(client2, w, classes, 5);
	BASIC_ENDCALL(client2, Success);
/* Call XGetSelectedExtensionEvents to get event mask for client2 for window. */
	if (XGetSelectedExtensionEvents(client2, w, &tcount, &this_client, 
	    &acount, &all_clients)) {
		delete("A call to XGetSelectedExtensionEvents failed.");
		return;
	}
	else
		CHECK;
/* Verify selected events are as expected. */
	if (tcount != 5) {
		delete("Unexpected events selected.");
		return;
	}
	else
		CHECK;
/* Call XGetSelectedExtensionEvents to get event mask for client1 for window. */
	if (XGetSelectedExtensionEvents(client1, w, &tcount, &this_client, 
	    &acount, &all_clients)) {
		delete("A call to XGetSelectedExtensionEvents failed.");
		return;
	}
	else
		CHECK;
/* Verify selected events have not changed. */
	if (tcount != 0) {
		delete("Unexpected events selected.");
		return;
	}
	else
		CHECK;
/* Select DeviceKeyPress events with client1 on this window. */
	BASIC_STARTCALL(client1);
	XSelectExtensionEvent(client1, w, &dkpclass, 1);
	BASIC_ENDCALL(client1, Success);
/* Call XGetSelectedExtensionEvents to get event mask for client1 for window. */
	if (XGetSelectedExtensionEvents(client1, w, &tcount, &this_client, 
	    &acount, &all_clients)) {
		delete("A call to XGetSelectedExtensionEvents failed.");
		return;
	}
	else
		CHECK;
/* Verify selected events have not changed. */
	if (tcount != 1) {
		delete("Unexpected events selected.");
		return;
	}
	else
		CHECK;
/* Call XGetSelectedExtensionEvents to get event mask for client2 for window. */
	if (XGetSelectedExtensionEvents(client2, w, &tcount, &this_client, 
	    &acount, &all_clients)) {
		delete("A call to XGetSelectedExtensionEvents failed.");
		return;
	}
	else
		CHECK;
/* Verify selected events have not changed. */
	if (tcount != 5) {
		delete("Unexpected events selected.");
		return;
	}
	else
		CHECK;

	CHECKPASS(12);
>>ASSERTION Good A
When multiple clients make a call to xname
requesting the same event on the same window
and that window is the event window for the requested event,
then the event is reported to each client.
>>STRATEGY
Create client1.
Create window with client1.
Select DeviceKeyPress events with client1 on this window.
Create client2.
Select DeviceKeyPress events with client2 on this window.
Map window.
XSync(.., False) on both clients to ensure generated events have come in.
Verify that client1 received a single DeviceKeyPress event for this window.
Verify that client1 received no other events.
Verify that client2 received a single DeviceKeyPress event for this window.
Verify that client2 received no other events.
>>CODE
Display *client1;
Display *client2;
XEvent	event;
int	n;
XEventClass dkpclass;
XID dkp;

	if (!Setup_Extension_DeviceInfo(KeyMask))
	    {
	    untested("%s: No input extension key device.\n", TestName);
	    return;
	    }
	if (noext(1))
	    return;
	DeviceKeyPress(Devs.Key, dkp, dkpclass);
/* Create client1. */
	client1 = opendisplay();
	if (client1 == (Display *) NULL) {
		delete("Can not open display");
		return;
	}
	else
		CHECK;
/* Create window with client1. */
	w = mkwin(client1, (XVisualInfo *) NULL, (struct area *) NULL, False);
/* Select DeviceKeyPress events with client1 on this window. */
	BASIC_STARTCALL(client1);
	XSelectExtensionEvent(client1, w, &dkpclass, 1);
	BASIC_ENDCALL(client1, Success);
/* Create client2. */
	client2 = opendisplay();
	if (client2 == (Display *) NULL) {
		delete("Can not open display");
		return;
	}
	else
		CHECK;
/* Select DeviceKeyPress events with client2 on this window. */
	BASIC_STARTCALL(client2);
	XSelectExtensionEvent(client2, w, &dkpclass, 1);
	BASIC_ENDCALL(client2, Success);
/* Map window. */
	XSync(client1, True);
	XSync(client2, True);
	XMapWindow(client1, w);
	warppointer(display, w, 1, 1);
	devicekeypress(client1, Devs.Key, MinKeyCode);
/* XSync(.., False) on both clients to ensure generated events have come in. */
	XSync(client1, False);
	XSync(client2, False);
	trace("Mapping window with DeviceKeyPress selected, expecting DeviceKeyPress.");
/* Verify that client1 received a single DeviceKeyPress event for this window.*/
	if (!XCheckTypedWindowEvent(client1, w, dkp, &event)) {
		report("Selected event was not delivered to client1.");
		FAIL;
	}
	else
		CHECK;
/* Verify that client1 received no other events. */
	if ((n=XPending(client1)) > 0) {
		XNextEvent(client1, &event);
		report("%d unexpected event%s (first %s) %s delivered to client1.",
			n, (n==1)?"":"s", eventname(event.type), (n==1)?"was":"were");
		FAIL;
	}
	else
		CHECK;
/* Verify that client1 received a single DeviceKeyPress event for this window.*/
	if (!XCheckTypedWindowEvent(client2, w, dkp, &event)) {
		report("Selected event was not delivered to client2.");
		FAIL;
	}
	else
		CHECK;
/* Verify that client2 received no other events. */
	if ((n=XPending(client2)) > 0) {
		XNextEvent(client2, &event);
		report("%d unexpected event%s (first %s) %s delivered to client2.",
			n, (n==1)?"":"s", eventname(event.type), (n==1)?"was":"were");
		FAIL;
	}
	else
		CHECK;

	devicerelkeys(Devs.Key);
	CHECKPASS(6);

>>ASSERTION Good B 3
If the DeviceButtonPress event class is specified, automatic passive
grabs are not done for the requesting client.
>>STRATEGY
>>CODE
XID dbp, dmn, dbr;
XEventClass class[3];
Window w;
int axes[2];
XEvent event;

	if (noext(1))
	    return;
	if (!Setup_Extension_DeviceInfo(BtnMask))
	    {
	    untested("%s: No input extension button device.\n", TestName);
	    return;
	    }
	DeviceButtonPress(Devs.Button, dbp, class[0]);
	DeviceMotionNotify(Devs.Button, dmn, class[1]);
	DeviceButtonRelease(Devs.Button, dbr, class[2]);
	w = defwin(display);
	XSelectExtensionEvent(display, w, class, 3);
	warppointer(display, w, 1, 1);
	XSync(display, 0);
	devicebuttonpress(display, Devs.Button, Button1);
	axes[0]=1;
	SimulateDeviceMotionEvent(display, Devs.Button, False, 1, axes, 0);
	devicebuttonrel(display, Devs.Button, Button1);
	XSync(display, 0);
/* Verify that client1 received a DeviceButtonPress event for this window.*/
	if (!XCheckTypedWindowEvent(display, w, dbp, &event)) {
		report("1: Button press event was not delivered to client.");
		FAIL;
	}
	else
		CHECK;
/* Verify that client1 received a DeviceMotion event for this window.*/
	if (!XCheckTypedWindowEvent(display, w, dmn, &event)) {
		report("2: Motion event was not delivered to client.");
		FAIL;
	}
	else
		CHECK;
/* Verify that client1 received a DeviceButtonRelease event for this window.*/
	if (!XCheckTypedWindowEvent(display, w, dbr, &event)) {
		report("3: Button release event was not delivered to client.");
		FAIL;
	}
	else
		CHECK;

/* Now press the button and warp the pointer out of the window.*/
/* If no automatic grab is done, this client won't receive the other events.*/
	devicebuttonpress(display, Devs.Button, Button1);
	warppointer(display, w, 200, 200);
	SimulateDeviceMotionEvent(display, Devs.Button, False, 1, axes, 0);
	devicebuttonrel(display, Devs.Button, Button1);
	XSync(display, 0);
	if (!XCheckTypedWindowEvent(display, w, dbp, &event)) {
		report("4: Button press event was not delivered to client.");
		FAIL;
	}
	else
		CHECK;
	if (XCheckTypedWindowEvent(display, w, dmn, &event)) {
		report("5: Motion event was incorrectly delivered to client.");
		FAIL;
	}
	else
		CHECK;
	if (XCheckTypedWindowEvent(display, w, dbr, &event)) {
		report("6: Button release event was incorrectly delivered to client.");
		FAIL;
	}
	else
		CHECK;
	devicerelbuttons(Devs.Button);
	axes[0]=0;
	SimulateDeviceMotionEvent(display, Devs.Button, False, 1, axes, 0);
	XSync(display,1);
	CHECKPASS(6);

>>ASSERTION Good B 3
If the DeviceButtonPressGrab event class is specified in addition to the
DeviceButtonPress event class, automatic passive grabs are done for the 
requesting client.
>>STRATEGY
>>CODE
XID dbp, dmn, dbr, dbpg;
XEventClass class[4];
Window w;
int axes[2];
XEvent event;

	if (noext(1))
	    return;
	if (!Setup_Extension_DeviceInfo(BtnMask))
	    {
	    untested("%s: No input extension button device.\n", TestName);
	    return;
	    }
	DeviceButtonPress(Devs.Button, dbp, class[0]);
	DeviceMotionNotify(Devs.Button, dmn, class[1]);
	DeviceButtonRelease(Devs.Button, dbr, class[2]);
	DeviceButtonPressGrab(Devs.Button, dbpg, class[3]);
	w = defwin(display);
	XSelectExtensionEvent(display, w, class, 4);
	warppointer(display, w, 1, 1);
	devicebuttonpress(display, Devs.Button, Button1);
	axes[0]=1;
	SimulateDeviceMotionEvent(display, Devs.Button, False, 1, axes, 0);
	devicebuttonrel(display, Devs.Button, Button1);
	XSync(display, 0);
/* Verify that client1 received a DeviceButtonPress event for this window.*/
	if (!XCheckTypedWindowEvent(display, w, dbp, &event)) {
		report("Button event was not delivered to client.");
		FAIL;
	}
	else
		CHECK;
/* Verify that client1 received a DeviceMotion event for this window.*/
	if (!XCheckTypedWindowEvent(display, w, dmn, &event)) {
		report("Motion event was not delivered to client.");
		FAIL;
	}
	else
		CHECK;
/* Verify that client1 received a DeviceButtonRelease event for this window.*/
	if (!XCheckTypedWindowEvent(display, w, dbr, &event)) {
		report("Button event was not delivered to client.");
		FAIL;
	}
	else
		CHECK;

/* Now press the button and warp the pointer out of the window.*/
/* If no automatic grab is done, this client won't receive the other events.*/
	devicebuttonpress(display, Devs.Button, Button1);
	warppointer(display, w, 200, 200);
	SimulateDeviceMotionEvent(display, Devs.Button, False, 1, axes, 0);
	devicebuttonrel(display, Devs.Button, Button1);
	XSync(display, 0);
	if (!XCheckTypedWindowEvent(display, w, dbp, &event)) {
		report("Button event was not delivered to client.");
		FAIL;
	}
	else
		CHECK;
	if (!XCheckTypedWindowEvent(display, w, dmn, &event)) {
		report("Motion event was not delivered to client.");
		FAIL;
	}
	else
		CHECK;
	if (!XCheckTypedWindowEvent(display, w, dbr, &event)) {
		report("Button event was not delivered to client.");
		FAIL;
	}
	else
		CHECK;
	devicerelbuttons(Devs.Button);
	axes[0]=0;
	SimulateDeviceMotionEvent(display, Devs.Button, False, 1, axes, 0);
	XSync(display,1);
	CHECKPASS(6);

>>ASSERTION Good B 3
If the DeviceButtonPressGrab event class is specified in addition to the
DeviceButtonPress event class, automatic passive grabs activate with
owner_events equal to False.
>>STRATEGY
>>CODE
XID dbp, dmn, dbr, dbpg;
XEventClass class[4];
Window w;
int axes[2];
XEvent event;

	if (noext(1))
	    return;
	if (!Setup_Extension_DeviceInfo(BtnMask))
	    {
	    untested("%s: No input extension button device.\n", TestName);
	    return;
	    }
	DeviceButtonPress(Devs.Button, dbp, class[0]);
	DeviceMotionNotify(Devs.Button, dmn, class[1]);
	DeviceButtonRelease(Devs.Button, dbr, class[2]);
	DeviceButtonPressGrab(Devs.Button, dbpg, class[3]);
	w = defwin(display);
	XSelectExtensionEvent(display, w, class, 4);
	XSync(display,0);

/* Press the button and warp the pointer out of the window.*/
/* If no automatic grab is done, this client won't receive the other events.*/
	warppointer(display, w, 1, 1);
	devicebuttonpress(display, Devs.Button, Button1);
	XSync(display,0);
	sleep(5);
	warppointer(display, w, 200, 200);
	axes[0]=1;
	SimulateDeviceMotionEvent(display, Devs.Button, False, 1, axes, 0);
	devicebuttonrel(display, Devs.Button, Button1);
	XSync(display, 0);
	if (!XCheckTypedWindowEvent(display, w, dbp, &event)) {
		report("Button event was not delivered to client.");
		FAIL;
	}
	else
		{
		XDeviceButtonEvent *bev = (XDeviceButtonEvent *) &event;
		if (bev->window == w)
		    CHECK;
		else
		    {
		    report("Button event reported with wrong window.");
		    FAIL;
		    }
		}
	if (!XCheckTypedWindowEvent(display, w, dmn, &event)) {
		report("Motion event was not delivered to client.");
		FAIL;
	}
	else
		{
		XDeviceMotionEvent *mev = (XDeviceMotionEvent *) &event;
		if (mev->window == w)
		    CHECK;
		else
		    {
		    report("Motion event reported with wrong window.");
		    FAIL;
		    }
		}
	if (!XCheckTypedWindowEvent(display, w, dbr, &event)) {
		report("Button event was not delivered to client.");
		FAIL;
	}
	else
		{
		XDeviceButtonEvent *bev = (XDeviceButtonEvent *) &event;
		if (bev->window == w)
		    CHECK;
		else
		    {
		    report("Button event reported with wrong window.");
		    FAIL;
		    }
		}
	devicerelbuttons(Devs.Button);
	axes[0]=0;
	SimulateDeviceMotionEvent(display, Devs.Button, False, 1, axes, 0);
	XSync(display,1);
	CHECKPASS(3);

>>ASSERTION Good B 3
If the DeviceOwnerGrabButton event class is specified in addition to the
DeviceButtonPress and DeviceButtonPressGrab event classes, automatic passive 
grabs activate with owner_events equal to True.
requesting client.
>>STRATEGY
>>CODE
XID dbp, dmn, dbr, dbpg, dbpog;
XEventClass class[5];
Window w;
int axes[2];
XEvent event;

	if (noext(1))
	    return;
	if (!Setup_Extension_DeviceInfo(BtnMask))
	    {
	    untested("%s: No input extension button device.\n", TestName);
	    return;
	    }
	DeviceButtonPress(Devs.Button, dbp, class[0]);
	DeviceMotionNotify(Devs.Button, dmn, class[1]);
	DeviceButtonRelease(Devs.Button, dbr, class[2]);
	DeviceButtonPressGrab(Devs.Button, dbpg, class[3]);
	DeviceOwnerGrabButton(Devs.Button, dbpog, class[4]);
	w = defwin(display);
	XSelectExtensionEvent(display, w, class, 5);

/* Press the button and warp the pointer out of the window.*/
/* If owner_events is True, these events will still be reported to the 
   grab window, because they would not otherwise be reported. (No select
   has been done on the root window. */
	warppointer(display, w, 1, 1);
	devicebuttonpress(display, Devs.Button, Button1);
	warppointer(display, w, 200, 200);
	axes[0]=1;
	SimulateDeviceMotionEvent(display, Devs.Button, False, 1, axes, 0);
	devicebuttonrel(display, Devs.Button, Button1);
	XSync(display, 0);
	if (!XCheckTypedWindowEvent(display, w, dbp, &event)) {
		report("Button event was not delivered to client.");
		FAIL;
	}
	else
	    CHECK;
	if (!XCheckTypedWindowEvent(display, w, dmn, &event)) {
		report("Motion event was not delivered to client.");
		FAIL;
	}
	else
	    CHECK;
	if (!XCheckTypedWindowEvent(display, w, dbr, &event)) {
		report("Button event was not delivered to client.");
		FAIL;
	}
	else
	    CHECK;

/* Now select the events on the root window and repeat the process */
/* Events should now be reported with respect to the root window.  */
	XSelectExtensionEvent(display, RootWindow(display,0), class, 5);
	warppointer(display, w, 1, 1);
	devicebuttonpress(display, Devs.Button, Button1);
	warppointer(display, w, 200, 200);
	SimulateDeviceMotionEvent(display, Devs.Button, False, 1, axes, 0);
	devicebuttonrel(display, Devs.Button, Button1);
	XSync(display, 0);
	if (!XCheckTypedWindowEvent(display, w, dbp, &event)) {
		report("Button event was incorrectly delivered to client.");
		FAIL;
	}
	else
	    CHECK;
	if (XCheckTypedWindowEvent(display, w, dmn, &event)) {
		report("Motion event was incorrectly delivered to client.");
		FAIL;
	}
	else
	    CHECK;
	if (XCheckTypedWindowEvent(display, w, dbr, &event)) {
		report("Button event was incorrectly delivered to client.");
		FAIL;
	}
	else
	    CHECK;

	if (XCheckTypedWindowEvent(display, RootWindow(display,0), dbp, &event)) {
		report("Button event was incorrectly delivered to client.");
		FAIL;
	}
	else
	    CHECK;
	if (!XCheckTypedWindowEvent(display, RootWindow(display,0), dmn, &event)) {
		report("Motion event was not delivered to client.");
		FAIL;
	}
	else
	    CHECK;
	if (!XCheckTypedWindowEvent(display, RootWindow(display,0), dbr, &event)) {
		report("Button event was not delivered to client.");
		FAIL;
	}
	else
	    CHECK;

	devicerelbuttons(Devs.Button);
	axes[0]=0;
	SimulateDeviceMotionEvent(display, Devs.Button, False, 1, axes, 0);
	XSync(display,1);
	CHECKPASS(9);

>>ASSERTION Good B 3
If a call to XSelectExtensionEvent is made with event classes Button1Motion
through Button5Motion, motion events will be received from that device only
when the specified button is down.
>>STRATEGY
>>CODE
XEventClass dbmclasses[6];
XID dbm1, dbm2, dbm3, dbm4, dbm5;
int i, axes[2];
XEvent event;


	if (noext(1))
	    return;
	if (!Setup_Extension_DeviceInfo(BtnMask))
	    {
	    untested("%s: No input extension button device.\n", TestName);
	    return;
	    }
	DeviceMotionNotify(Devs.Button, dbm1, dbmclasses[0]);
	DeviceButton1Motion(Devs.Button, dbm1, dbmclasses[0]);
	DeviceButton2Motion(Devs.Button, dbm2, dbmclasses[1]);
	DeviceButton3Motion(Devs.Button, dbm3, dbmclasses[2]);
	DeviceButton4Motion(Devs.Button, dbm4, dbmclasses[3]);
	DeviceButton5Motion(Devs.Button, dbm5, dbmclasses[4]);
	DeviceButtonMotion(Devs.Button, dbm5, dbmclasses[5]);
	w = defwin(display);
	XSetDeviceFocus(display, Devs.Button, w, RevertToPointerRoot, 
	    CurrentTime);
	XSync(display,0);
	eclass = dbmclasses;
	ecount = 1;
	XCALL;
	axes[0]=1;
	SimulateDeviceMotionEvent(display, Devs.Button, False, 1, axes, 0);
	XSync(display,0);
	if (XCheckTypedWindowEvent(display, w, dbm1, &event)) {
		report("Event was incorrectly delivered to client.");
		FAIL;
	}
	else
	    CHECK;
	devicebuttonpress(display, Devs.Button, Button1);
	SimulateDeviceMotionEvent(display, Devs.Button, False, 1, axes, 0);
	XSync(display,0);
	if (!XCheckTypedWindowEvent(display, w, dbm1, &event)) {
		report("Event was not delivered to client.");
		FAIL;
	}
	else
	    CHECK;

	if (NumButtons > 1) {
	    devicebuttonpress(display, Devs.Button, Button2);
	    SimulateDeviceMotionEvent(display, Devs.Button, False, 1, axes, 0);
	    XSync(display,0);
	    if (!XCheckTypedWindowEvent(display, w, dbm1, &event)) {
		report("Event was not delivered to client.");
		FAIL;
	    }
	    else
	        CHECK;
	}
	else {
	    report("%s can't be completely tested because the device doesn't have enough buttons\n",TestName);
	    UNTESTED;
	    return;
	    }

	devicebuttonrel(display, Devs.Button, Button1);
	SimulateDeviceMotionEvent(display, Devs.Button, False, 1, axes, 0);
	XSync(display,0);
	if (XCheckTypedWindowEvent(display, w, dbm1, &event)) {
		report("Event was incorrectly delivered to client.");
		FAIL;
	}
	else
	    CHECK;

	eclass = &dbmclasses[1];
	XCALL;
	SimulateDeviceMotionEvent(display, Devs.Button, False, 1, axes, 0);
	XSync(display,0);
	if (!XCheckTypedWindowEvent(display, w, dbm1, &event)) {
		report("Event was not delivered to client.");
		FAIL;
	}
	else
	    CHECK;

	if (NumButtons > 2) {
	    devicebuttonpress(display, Devs.Button, Button3);
	    SimulateDeviceMotionEvent(display, Devs.Button, False, 1, axes, 0);
	    XSync(display,0);
	    if (!XCheckTypedWindowEvent(display, w, dbm1, &event)) {
		report("Event was not delivered to client.");
		FAIL;
	    }
	    else
	        CHECK;
	}
	else {
	    report("%s can't be completely tested because the device doesn't have enough buttons\n",TestName);
	    UNTESTED;
	    return;
	    }

	devicebuttonrel(display, Devs.Button, Button2);
	SimulateDeviceMotionEvent(display, Devs.Button, False, 1, axes, 0);
	XSync(display,0);
	if (XCheckTypedWindowEvent(display, w, dbm1, &event)) {
		report("Event was incorrectly delivered to client.");
		FAIL;
	}
	else
	    CHECK;

	eclass = &dbmclasses[2];
	XCALL;
	SimulateDeviceMotionEvent(display, Devs.Button, False, 1, axes, 0);
	XSync(display,0);
	if (!XCheckTypedWindowEvent(display, w, dbm1, &event)) {
		report("Event was not delivered to client.");
		FAIL;
	}
	else
	    CHECK;


	if (NumButtons > 3) {
	    devicebuttonpress(display, Devs.Button, Button4);
	    SimulateDeviceMotionEvent(display, Devs.Button, False, 1, axes, 0);
	    XSync(display,0);
	    if (!XCheckTypedWindowEvent(display, w, dbm1, &event)) {
		report("Event was not delivered to client.");
		FAIL;
	    }
	    else
	        CHECK;
	}
	else {
	    report("%s can't be completely tested because the device doesn't have enough buttons\n",TestName);
	    UNTESTED;
	    return;
	    }

	devicebuttonrel(display, Devs.Button, Button3);
	SimulateDeviceMotionEvent(display, Devs.Button, False, 1, axes, 0);
	XSync(display,0);
	if (XCheckTypedWindowEvent(display, w, dbm1, &event)) {
		report("Event was incorrectly delivered to client.");
		FAIL;
	}
	else
	    CHECK;

	eclass = &dbmclasses[3];
	XCALL;
	SimulateDeviceMotionEvent(display, Devs.Button, False, 1, axes, 0);
	XSync(display,0);
	if (!XCheckTypedWindowEvent(display, w, dbm1, &event)) {
		report("Event was not delivered to client.");
		FAIL;
	}
	else
	    CHECK;

	if (NumButtons > 4) {
	    devicebuttonpress(display, Devs.Button, Button5);
	    SimulateDeviceMotionEvent(display, Devs.Button, False, 1, axes, 0);
	    XSync(display,0);
	    if (!XCheckTypedWindowEvent(display, w, dbm1, &event)) {
		report("Event was not delivered to client.");
		FAIL;
	    }
	    else
	        CHECK;
	}
	else {
	    report("%s can't be completely tested because the device doesn't have enough buttons\n",TestName);
	    UNTESTED;
	    return;
	    }

	devicebuttonrel(display, Devs.Button, Button4);
	SimulateDeviceMotionEvent(display, Devs.Button, False, 1, axes, 0);
	XSync(display,0);
	if (XCheckTypedWindowEvent(display, w, dbm1, &event)) {
		report("Event was incorrectly delivered to client.");
		FAIL;
	}
	else
	    CHECK;

	eclass = &dbmclasses[4];
	XCALL;
	SimulateDeviceMotionEvent(display, Devs.Button, False, 1, axes, 0);
	XSync(display,0);
	if (!XCheckTypedWindowEvent(display, w, dbm1, &event)) {
		report("Event was not delivered to client.");
		FAIL;
	}
	else
	    CHECK;

	eclass = &dbmclasses[5];
	XCALL;
	SimulateDeviceMotionEvent(display, Devs.Button, False, 1, axes, 0);
	XSync(display,0);
	if (!XCheckTypedWindowEvent(display, w, dbm1, &event)) {
		report("Event was not delivered to client.");
		FAIL;
	}
	else
	    CHECK;
	devicerelbuttons(Devs.Button);
	axes[0]=0;
	SimulateDeviceMotionEvent(display, Devs.Button, False, 1, axes, 0);
	XSync(display,1);
	CHECKPASS(15);

>>ASSERTION Good B 3
A call to xname specifying devicemotionnotify and devicemotionnotifyhint
classes will result in only one devicemotionnotify event being sent,
until a button is pressed on that device.
>>STRATEGY
>>CODE
XEventClass classes[3];
XID dmn, dmh;
int i, axes[2];
XEvent event;

	if (noext(1))
	    return;
	if (!Setup_Extension_DeviceInfo(BtnMask))
	    {
	    untested("%s: No input extension Button device.\n", TestName);
	    return;
	    }
	DeviceMotionNotify(Devs.Button, dmn, classes[0]);
	DevicePointerMotionHint(Devs.Button,dmh,classes[1]);
	w = defwin(display);
	XSetDeviceFocus(display, Devs.Button, w, RevertToPointerRoot, 
	    CurrentTime);
	XSync(display,0);
	eclass = classes;
	ecount = 2;
	XCALL;
	axes[0]=1;
	for(i=0; i<5; i++)
	    SimulateDeviceMotionEvent(display, Devs.Button, False, 1, axes, 0);
	XSync(display,0);
	if (XPending(display) > 1)
	    {
	    report("Too many events sent to client.");
	    FAIL;
	    }
	else
	    CHECK;
	if (!XCheckTypedWindowEvent(display, w, dmn, &event)) {
		report("Initial event was not delivered to client.");
		FAIL;
	}
	else
	    CHECK;
	for(i=0; i<5; i++)
	    SimulateDeviceMotionEvent(display, Devs.Button, False, 1, axes, 0);
	XSync(display,0);
	if (XCheckTypedWindowEvent(display, w, dmn, &event)) {
		report("Event was incorrectly delivered to client.");
		FAIL;
	}
	else
	    CHECK;
	devicebuttonpress(display, Devs.Button, Button1);
	for(i=0; i<5; i++)
	    SimulateDeviceMotionEvent(display, Devs.Button, False, 1, axes, 0);
	devicebuttonrel(display, Devs.Button, Button1);
	XSync(display,0);
	if (XPending(display) > 1)
	    {
	    report("Too many events sent to client.");
	    FAIL;
	    }
	else
	    CHECK;
	if (!XCheckTypedWindowEvent(display, w, dmn, &event)) {
		report("Event was not delivered to client after button.");
		FAIL;
	}
	else
	    CHECK;
	devicerelbuttons(Devs.Button);
	axes[0]=0;
	SimulateDeviceMotionEvent(display, Devs.Button, False, 1, axes, 0);
	XSync(display,1);
	CHECKPASS(5);

>>ASSERTION Bad B 3
A call to xname specifying an invalid window id results in a  BadWindow
error.
>>CODE BadWindow
int dfi;
XEventClass dfic;

	if (!Setup_Extension_DeviceInfo(BtnMask))
	    {
	    untested("%s: No input extension Button device.\n", TestName);
	    return;
	    }
	DeviceFocusIn(Devs.Button, dfi, dfic);
	w = 0;
	eclass = &dfic;
	ecount = 1;
	XCALL;
	if (geterr()==BadWindow)
	    PASS;
	else
	    FAIL;

>>ASSERTION Bad B 3
If xname is called with an eventclass of DeviceButtonPressGrab specifying
a window for which another client has already selected DeviceButtonPressGrab,
a BadAccess error will result.
>>STRATEGY
Call xname with an eventclass of DeviceButtonPressGrab specifying a 
window for which another client has already specified DeviceButtonPressGrab.
>>CODE
Display *client1;
Display *client2;
int dbpgtype;
XEventClass dbpgclass;

	if (!Setup_Extension_DeviceInfo(BtnMask))
	    {
	    untested("%s: No input extension button device.\n", TestName);
	    return;
	    }
/* Create client1. */
	client1 = opendisplay();
	if (client1 == (Display *) NULL) {
		delete("Can not open display");
		return;
	}
	else
		CHECK;
/* Create window with client1. */
	w = mkwin(client1, (XVisualInfo *) NULL, (struct area *) NULL, False);
/* Select DeviceButtonPressGrab class with client1 on this window. */
	DeviceButtonPressGrab(Devs.Button, dbpgtype, dbpgclass);
	BASIC_STARTCALL(client1);
	eclass = &dbpgclass;
	ecount = 1;
	XSelectExtensionEvent(client1, w, &dbpgclass, 1);
	BASIC_ENDCALL(client1, Success);
/* Create client2. */
	client2 = opendisplay();
	if (client2 == (Display *) NULL) {
		delete("Can not open display");
		return;
	}
	else
		CHECK;
/* Select DeviceButtonPressGrab class with client2 on this window. */
	BASIC_STARTCALL(client2);
	XSelectExtensionEvent(client2, w, &dbpgclass, 1);
	BASIC_ENDCALL(client2, BadAccess);
/* Verify that a BadAccess error was generated. */
	if (geterr() != BadAccess) {
		report("A call to XSelectExtensionEvent did not generate BadAccess error");
		FAIL;
	}
	else
		CHECK;

	CHECKPASS(3);
>>ASSERTION Bad B 3
A call to xname specifying an invalid eventclass results in a BadClass
error.
>>CODE badclass
XID badclass;
XEventClass bogus;

	if (!Setup_Extension_DeviceInfo(BtnMask))
	    {
	    untested("%s: No input extension button device.\n", TestName);
	    return;
	    }
	BadClass(display, badclass);
	w = 0;
	bogus = -1;
	eclass = &bogus;
	ecount = 1;
	w = mkwin(display, (XVisualInfo *) NULL, (struct area *) NULL, False);
	XCALL;
	if (geterr()==badclass)
	    PASS;
	else
	    FAIL;


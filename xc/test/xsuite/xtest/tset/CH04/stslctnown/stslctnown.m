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
 * Copyright 1990, 1991 by UniSoft Group Limited.
 * 
 * Permission to use, copy, modify, distribute, and sell this software and
 * its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation, and that the name of UniSoft not be
 * used in advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.  UniSoft
 * makes no representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 *
 * $XConsortium: stslctnown.m,v 1.10 94/04/17 21:03:38 rws Exp $
 */
>>TITLE XSetSelectionOwner CH04
void

Display *display = Dsp;
Atom selection = XA_COPYRIGHT;
Window owner = defwin(display);
Time thetime = CurrentTime;
>>EXTERN
#include "Xatom.h"
>>ASSERTION Good A
>># The code must test for a window, and a genuine time
>># (not CurrentTime).				-stuart.
A call to xname changes the owner window to
.A owner
and
the last-change time to
.A time
for the specified
.A selection .
>>STRATEGY
Create a new client.
Obtain current server time.
Call xname to assign the owner and time for the selection.
Call xname to clear the selection.
Verify that the correct SelectionClear event occurred.
>>CODE
Display *client1, *client2;
Time calltime;
Window  callwindow;
int num_ev;
XEvent ev;

/* Create a new client. */
	client1 = display;
	client2 = opendisplay();
	if (client2 == (Display *)NULL) {
		delete("Could not open client2");
		return;
	} else
		CHECK;

/* Obtain current server time. */
	calltime = gettime(client1);
	callwindow = owner;

/* Call xname to assign the owner and time for the selection. */
	thetime = calltime;
	XCALL;

/* Call xname to clear the selection. */
	display = client2;
	thetime = CurrentTime;
	owner = None;
	XCALL;
	XSync(client1, False);

/* Verify that the correct SelectionClear event occurred. */
	num_ev = getevent(client1, &ev);
	if (num_ev != 1) {
		delete("Expecting a single SelectionClear event");
		report("Received %d events", num_ev);
		while(num_ev !=0) {
			trace("Event: %s", eventname(ev.type));
			num_ev = getevent(client1, &ev);
		}
		return;
	} else {
		XEvent good;
		good.type = SelectionClear;
		good.xselectionclear.type = SelectionClear;
		good.xselectionclear.selection = selection;
		good.xselectionclear.display = client1;
		good.xselectionclear.window = callwindow;
		good.xselectionclear.time = calltime;
		if (checkevent(&good, &ev)) {
			FAIL;
			report("%s did not set the selection as expected",
				TestName);
		} else
			CHECK;
	}

	CHECKPASS(2);

>>ASSERTION Good A
When the specified
.A time
is earlier than the current last-change time of the specified
.A selection
or is later than the current server time, then a call to xname has no effect.
>>STRATEGY
Create two new clients.
Obtain current server time.
Call xname to assign the owner and time for the selection.
Call xname to assign the selection to a new client and owner
	with time before last change time.
Verify that no SelectionClear event occurred.
Call xname to assign the selection to a new client and owner
	with time after current server time.
Verify that no SelectionClear event occurred.
>>CODE
Display *calldisplay, *client2, *client3;
Time calltime;
int num_ev;
XEvent ev;

/* Create two new clients. */
	client2 = opendisplay();
	if (client2 == (Display *)NULL) {
		delete("Could not open client2");
		return;
	} else
		CHECK;

	client3 = opendisplay();
	if (client3 == (Display *)NULL) {
		delete("Could not open client3");
		return;
	} else
		CHECK;

/* Obtain current server time. */
	calldisplay = display;
	calltime = gettime(display);

/* Call xname to assign the owner and time for the selection. */
	display = calldisplay;
	thetime = calltime;
	XCALL;

/* Call xname to assign the selection to a new client and owner */
/* 	with time before last change time. */
	display = client2;
	owner = defwin(client2);
	thetime = calltime-1;
	XCALL;

/* Verify that no SelectionClear event occurred. */
	num_ev = getevent(calldisplay, &ev);
	if (num_ev != 0) {
		FAIL;
		report("%s called with an earlier time changed the selection",
			TestName);
		trace("Expecting 0 events");
		trace("Received %d events", num_ev);
		do {
			trace("Event: %s", eventname(ev.type));
		} while(getevent(calldisplay, &ev));
	} else
			CHECK;

/* Call xname to assign the selection to a new client and owner */
/* 	with time after current server time. */
	display = client3;
	owner = defwin(client3);
	thetime = gettime(client3)+(Time)100000;
	XCALL;

/* Verify that no SelectionClear event occurred. */
	num_ev = getevent(calldisplay, &ev);
	if (num_ev != 0) {
		FAIL;
		report("%s called with an earlier time changed the selection",
			TestName);
		trace("Expecting 0 events");
		trace("Received %d events", num_ev);
		do {
			trace("Event: %s", eventname(ev.type));
		} while(getevent(calldisplay, &ev));
	} else
			CHECK;

	CHECKPASS(4);

>>ASSERTION Good A
When
.A time
is
.S CurrentTime ,
then a call to xname sets the last-change time of the specified
.A selection
to the current server time.
>>STRATEGY
Create a new client.
Obtain current server time.
Call xname to assign the owner and time for the selection.
Call xname to clear the selection, to obtain selection time.
Verify that the correct SelectionClear event occurred.
Verify the time returned was within acceptable limits.
>>CODE
Display *client1, *client2;
Time calltime, rettime, maxtime;
int num_ev;
XEvent ev;

/* Create a new client. */
	client1 = display;
	client2 = opendisplay();
	if (client2 == (Display *)NULL) {
		delete("Could not open client2");
		return;
	} else
		CHECK;

/* Obtain current server time. */
	calltime = gettime(client1);

/* Call xname to assign the owner and time for the selection. */
	thetime = CurrentTime;
	XCALL;

/* Call xname to clear the selection, to obtain selection time. */
	display = client2;
	thetime = CurrentTime;
	owner = None;
	XCALL;
	XSync(client1, False);

/* Verify that the correct SelectionClear event occurred. */
	num_ev = getevent(client1, &ev);
	if (num_ev != 1) {
		delete("Expecting a single SelectionClear event");
		report("Received %d events", num_ev);
		while(num_ev !=0) {
			trace("Event: %s", eventname(ev.type));
			num_ev = getevent(client1, &ev);
		}
		return;
	} else {
		if (ev.type != SelectionClear) {
			delete("Expecting a SelectionClear event");
			report("Returned a %s event", eventname(ev.type));
			return;
		} else
			CHECK;
	}

/* Verify the time returned was within acceptable limits. */
	rettime=ev.xselectionclear.time;
	maxtime=calltime+5000*config.speedfactor;
	trace("Obtained server time: %u", calltime);
	trace("Returned server time: %u", rettime);
	trace("Upper expected time : %u", maxtime);
	if((rettime<calltime) || (rettime>maxtime)) {
		FAIL;
		report("%s did not set last modified time of selection",
			TestName);
		report("to within reasonable bounds");
	} else
		CHECK;

	CHECKPASS(3);

>>ASSERTION Good A
When the
.A owner
window is
.S None ,
then a call to xname sets the specified
.A selection
to have no owner.
>>STRATEGY
Call xname to set the owner for the selection.
Verify that the selection was set.
Call xname to set no owner for the selection.
Verify that the selection has no owner.
>>CODE

/* Call xname to set the owner for the selection. */
	XCALL;

/* Verify that the selection was set. */
	if(XGetSelectionOwner(display, selection) != owner) {
		delete("A call to %s did not set the selection for the test",
			TestName);
		return;
	} else
		CHECK;

/* Call xname to set no owner for the selection. */
	thetime = CurrentTime;
	owner = None;
	XCALL;

/* Verify that the selection has no owner. */
	if (XGetSelectionOwner(display, selection) != None) {
		FAIL;
		report("A call to %s with owner of None failed to set the",
			TestName);
		report("selection owner to None.");
	} else
		CHECK;

	CHECKPASS(2);

>>ASSERTION Good A
>># This is ambiguous. In reality, the section owner must be a different
>># client. This could be a grey area.
When the specified
.A selection
has an owner, and the specified
.A owner
is not the same as the current owner,
then on a call to xname the current owner is sent a
.S SelectionClear 
event.
>>STRATEGY
Create one new client.
Call xname to assign the owner and time for the selection.
Call xname to assign the selection to a new owner.
Verify that the correct SelectionClear event occurred.
>>CODE
Display *client1, *client2;
Time calltime;
Window  callwindow;
int num_ev;
XEvent ev;

/* Create a new client. */
	client1 = display;
	client2 = opendisplay();
	if (client2 == (Display *)NULL) {
		delete("Could not open client2");
		return;
	} else
		CHECK;

	calltime = gettime(client1);

/* Call xname to assign the owner and time for the selection. */
	callwindow = owner;
	thetime = calltime;
	XCALL;

/* Call xname to assign the selection to a new owner. */
	display = client2;
	thetime = CurrentTime;
	owner = defwin(client2);
	XCALL;
	XSync(client1, False);

/* Verify that the correct SelectionClear event occurred. */
	num_ev = getevent(client1, &ev);
	if (num_ev != 1) {
		delete("Expecting a single SelectionClear event");
		report("Received %d events", num_ev);
		while(num_ev !=0) {
			trace("Event: %s", eventname(ev.type));
			num_ev = getevent(client1, &ev);
		}
		return;
	} else {
		XEvent good;
		good.type = SelectionClear;
		good.xselectionclear.type = SelectionClear;
		good.xselectionclear.selection = selection;
		good.xselectionclear.display = client1;
		good.xselectionclear.window = callwindow;
		good.xselectionclear.time = calltime;
		if (checkevent(&good, &ev)) {
			FAIL;
			report("%s did not set the selection as expected",
				TestName);
		} else
			CHECK;
	}

	CHECKPASS(2);

>>ASSERTION Good A
>># Need to split this assertion into two, one for the owner, and the
>># other for the last-change time, which is untestable -stuart.
When the client that is the owner of a selection is terminated, or the
owner window is destroyed, 
then the selection reverts to having no owner, and the last-change
time is unaffected.
>>STRATEGY
Create a new client.
Call xname to assign the owner and time for the selection.
Close the connection for client2.
Allow time for the connection to close.
Verify that the ownership of the selection reverted to None.
Assign the selection to another window.
Close the window of the selection.
Verify that the ownership of the selection reverted to None.
>>CODE
Display *client1, *client2;

/* Create a new client. */
	client1 = display;
	client2 = XOpenDisplay(config.display); 
	if (client2 == (Display *)NULL) {
		delete("Could not open client2");
		return;
	} else
		CHECK;

/* Call xname to assign the owner and time for the selection. */
	display = client2;
	XCALL;

/* Close the connection for client2. */
	XCloseDisplay(client2);

/* Allow time for the connection to close. */
	reset_delay();

/* Verify that the ownership of the selection reverted to None. */
	if (XGetSelectionOwner(client1, selection) != None) {
		FAIL;
		report("Closing the owner client did not set the selection");
		report("owner to None.");
		return;
	} else
		CHECK;

/* Assign the selection to another window. */
	display = client1;
	owner = crechild(client1, defwin(client1), (struct area *)0);
	XCALL;

/* Close the window of the selection. */
	XDestroyWindow(client1, owner);

/* Verify that the ownership of the selection reverted to None. */
	if (XGetSelectionOwner(client1, selection) != None) {
		FAIL;
		report("Closing the owner window did not set the selection");
		report("owner to None.");
	} else
		CHECK;

	CHECKPASS(3);

>>ASSERTION Good B 1
>># This is meant to mean that the selection atoms are global.
>># I hope this is a near enough approximation.		-stuart.
The status of a selection atom is accessible by any client
of the server.
>>ASSERTION Bad A
.ER BadAtom
>>ASSERTION Bad A
.ER BadWindow

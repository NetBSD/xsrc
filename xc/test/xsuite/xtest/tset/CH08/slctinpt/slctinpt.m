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
 * $XConsortium: slctinpt.m,v 1.13 94/04/17 21:08:11 rws Exp $
 */
>>TITLE XSelectInput CH08
void
XSelectInput(display, w, event_mask)
Display *display = Dsp;
Window w;
long event_mask;
>>ASSERTION Good A
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
Select MapNotify events with client2 on this window.
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
/* Select no events with client1 on this window. */
	BASIC_STARTCALL(client1);
	XSelectInput(client1, w, NoEventMask);
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
	XSelectInput(client2, w, StructureNotifyMask);
	BASIC_ENDCALL(client2, Success);
/* Map window. */
	XSync(client1, True);
	XSync(client2, True);
	XMapWindow(client1, w);
/* XSync(.., False) on both clients to ensure generated events have come in. */
	XSync(client1, False);
	XSync(client2, False);
	trace("Mapping window with StructureNotifyMask selected, expecting MapNotify.");
/* Verify that client1 received no events. */
	if ((n=XPending(client1)) > 0) {
		XNextEvent(client1, &event);
		report("%d unexpected event%s (first %s) %s delivered to client1.",
			n, (n==1)?"":"s", eventname(event.type), (n==1)?"was":"were");
		FAIL;
	}
	else
		CHECK;
/* Verify that client2 received a single MapNotify event for this window. */
	if (!XCheckTypedWindowEvent(client2, w, MapNotify, &event)) {
		report("Selected event (MapNotify) was not delivered.");
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

	CHECKPASS(5);
>>ASSERTION Good A
A call to xname
overrides the event mask attribute set during any previous call to
xname ,
.F XChangeWindowAttributes ,
or
.F XCreateWindow .
>>STRATEGY
Create window with no events selected.
Call XGetWindowAttributes to get event mask for this window.
Verify event mask is as expected.
Call XSelectInput to change event mask to StructureNotifyMask.
Call XGetWindowAttributes to get new event mask for this window.
Verify event mask changed as expected.
Call XChangeWindowAttributes to change event mask to NoEventMask.
Call XGetWindowAttributes to get event mask for this window.
Verify event mask is as expected.
Call XSelectInput to change event mask to ALLEVENTS.
Call XGetWindowAttributes to get new event mask for this window.
Verify event mask changed as expected.
Call XSelectInput to change event mask to StructureNotifyMask.
Call XGetWindowAttributes to get new event mask for this window.
Verify event mask changed as expected.
>>CODE
XSetWindowAttributes setattrs;
XWindowAttributes attrs;

/* Create window with no events selected. */
	w = mkwin(display, (XVisualInfo *) NULL, (struct area *) NULL, False);
/* Call XGetWindowAttributes to get event mask for this window. */
	if (!XGetWindowAttributes(display, w, &attrs)) {
		delete("A call to XGetWindowAttributes failed.");
		return;
	}
	else
		CHECK;
/* Verify event mask is as expected. */
	if (attrs.your_event_mask != NoEventMask) {
		delete("Unexpected event mask value.");
		return;
	}
	else
		CHECK;
/* Call XSelectInput to change event mask to StructureNotifyMask. */
	event_mask = StructureNotifyMask;
	XCALL;
/* Call XGetWindowAttributes to get new event mask for this window. */
	if (!XGetWindowAttributes(display, w, &attrs)) {
		delete("A call to XGetWindowAttributes failed.");
		return;
	}
	else
		CHECK;
/* Verify event mask changed as expected. */
	if (attrs.your_event_mask != StructureNotifyMask) {
		report("Event mask incorrect first call.");
		FAIL;
	}
	else
		CHECK;
/* Call XChangeWindowAttributes to change event mask to NoEventMask. */
	setattrs.event_mask = NoEventMask;
	XChangeWindowAttributes(display, w, CWEventMask, &setattrs);
/* Call XGetWindowAttributes to get event mask for this window. */
	if (!XGetWindowAttributes(display, w, &attrs)) {
		delete("A call to XGetWindowAttributes failed.");
		return;
	}
	else
		CHECK;
/* Verify event mask is as expected. */
	if (attrs.your_event_mask != NoEventMask) {
		delete("Unexpected (non-empty) event mask value after XChangeWindowAttributes.");
		return;
	}
	else
		CHECK;
/* Call XSelectInput to change event mask to ALLEVENTS. */
	event_mask = ALLEVENTS;
	XCALL;
/* Call XGetWindowAttributes to get new event mask for this window. */
	if (!XGetWindowAttributes(display, w, &attrs)) {
		delete("A call to XGetWindowAttributes failed.");
		return;
	}
	else
		CHECK;
/* Verify event mask changed as expected. */
	if (attrs.your_event_mask != ALLEVENTS) {
		report("Event mask incorrect after second call.");
		FAIL;
	}
	else
		CHECK;
/* Call XSelectInput to change event mask to StructureNotifyMask. */
	event_mask = StructureNotifyMask;
	XCALL;
/* Call XGetWindowAttributes to get new event mask for this window. */
	if (!XGetWindowAttributes(display, w, &attrs)) {
		delete("A call to XGetWindowAttributes failed.");
		return;
	}
	else
		CHECK;
/* Verify event mask changed as expected. */
	if (attrs.your_event_mask != StructureNotifyMask) {
		report("Event mask incorrect after third call.");
		FAIL;
	}
	else
		CHECK;

	CHECKPASS(10);
>>ASSERTION Good A
A call to xname
does not change the event mask attribute
for other clients.
>>STRATEGY
Create client1.
Create window with client1.
Select NoEventMask events with client1 on this window.
Call XGetWindowAttributes to get event mask for client1 for window.
Verify event mask is as expected.
Create client2.
Select ALLEVENTS events with client2 on this window.
Call XGetWindowAttributes to get event mask for client2 for window.
Verify event mask is as expected.
Call XGetWindowAttributes to get event mask for client1 for window.
Verify event mask has not changed.
Select KeyPressMask events with client1 on this window.
Call XGetWindowAttributes to get event mask for client1 for window.
Verify event mask is as expected.
Call XGetWindowAttributes to get event mask for client2 for window.
Verify event mask has not changed.
>>CODE
Display *client1;
Display *client2;
XWindowAttributes attrs;

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
	XSelectInput(client1, w, NoEventMask);
	BASIC_ENDCALL(client1, Success);
/* Call XGetWindowAttributes to get event mask for client1 for window. */
	if (!XGetWindowAttributes(client1, w, &attrs)) {
		delete("A call to XGetWindowAttributes failed.");
		return;
	}
	else
		CHECK;
/* Verify event mask is as expected. */
	if (attrs.your_event_mask != NoEventMask) {
		delete("Unexpected event mask value.");
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
/* Select ALLEVENTS events with client2 on this window. */
	BASIC_STARTCALL(client2);
	XSelectInput(client2, w, ALLEVENTS);
	BASIC_ENDCALL(client2, Success);
/* Call XGetWindowAttributes to get event mask for client2 for window. */
	if (!XGetWindowAttributes(client2, w, &attrs)) {
		delete("A call to XGetWindowAttributes failed.");
		return;
	}
	else
		CHECK;
/* Verify event mask is as expected. */
	if (attrs.your_event_mask != ALLEVENTS) {
		delete("Unexpected event mask value.");
		return;
	}
	else
		CHECK;
/* Call XGetWindowAttributes to get event mask for client1 for window. */
	if (!XGetWindowAttributes(client1, w, &attrs)) {
		delete("A call to XGetWindowAttributes failed.");
		return;
	}
	else
		CHECK;
/* Verify event mask has not changed. */
	if (attrs.your_event_mask != NoEventMask) {
		report("Event mask incorrect.");
		FAIL;
	}
	else
		CHECK;
/* Select KeyPressMask events with client1 on this window. */
	BASIC_STARTCALL(client1);
	XSelectInput(client1, w, KeyPressMask);
	BASIC_ENDCALL(client1, Success);
/* Call XGetWindowAttributes to get event mask for client1 for window. */
	if (!XGetWindowAttributes(client1, w, &attrs)) {
		delete("A call to XGetWindowAttributes failed.");
		return;
	}
	else
		CHECK;
/* Verify event mask is as expected. */
	if (attrs.your_event_mask != KeyPressMask) {
		delete("Unexpected event mask value.");
		return;
	}
	else
		CHECK;
/* Call XGetWindowAttributes to get event mask for client2 for window. */
	if (!XGetWindowAttributes(client2, w, &attrs)) {
		delete("A call to XGetWindowAttributes failed.");
		return;
	}
	else
		CHECK;
/* Verify event mask has not changed. */
	if (attrs.your_event_mask != ALLEVENTS) {
		report("Event mask incorrect.");
		FAIL;
	}
	else
		CHECK;

	CHECKPASS(12);
>>ASSERTION Good A
When multiple clients make a call to xname
requesting the same event on the same window
and
that window is the event window for the requested event,
then the event is reported to each client.
>>STRATEGY
Create client1.
Create window with client1.
Select MapNotify events with client1 on this window.
Create client2.
Select MapNotify events with client2 on this window.
Map window.
XSync(.., False) on both clients to ensure generated events have come in.
Verify that client1 received a single MapNotify event for this window.
Verify that client1 received no other events.
Verify that client2 received a single MapNotify event for this window.
Verify that client2 received no other events.
>>CODE
Display *client1;
Display *client2;
XEvent	event;
int	n;

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
/* Select MapNotify events with client1 on this window. */
	BASIC_STARTCALL(client1);
	XSelectInput(client1, w, StructureNotifyMask);
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
	XSelectInput(client2, w, StructureNotifyMask);
	BASIC_ENDCALL(client2, Success);
/* Map window. */
	XSync(client1, True);
	XSync(client2, True);
	XMapWindow(client1, w);
/* XSync(.., False) on both clients to ensure generated events have come in. */
	XSync(client1, False);
	XSync(client2, False);
	trace("Mapping window with StructureNotifyMask selected, expecting MapNotify.");
/* Verify that client1 received a single MapNotify event for this window. */
	if (!XCheckTypedWindowEvent(client1, w, MapNotify, &event)) {
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
/* Verify that client2 received a single MapNotify event for this window. */
	if (!XCheckTypedWindowEvent(client2, w, MapNotify, &event)) {
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

	CHECKPASS(6);
>>ASSERTION Bad A
When another client has selected with an event mask
.S SubstructureRedirectMask ,
then on a call to xname
with
.S SubstructureRedirectMask
bits set in
.A event_mask
a
.S BadAccess
error occurs.
>>STRATEGY
Create client1.
Create window with client1.
Select SubstructureRedirectMask event mask with client1 on this window.
Create client2.
Select SubstructureRedirectMask event mask with client2 on this window.
Verify that a BadAccess error was generated.
>>CODE BadAccess
Display *client1;
Display *client2;

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
/* Select SubstructureRedirectMask event mask with client1 on this window. */
	BASIC_STARTCALL(client1);
	XSelectInput(client1, w, SubstructureRedirectMask);
	BASIC_ENDCALL(client1, Success);
/* Create client2. */
	client2 = opendisplay();
	if (client2 == (Display *) NULL) {
		delete("Can not open display");
		return;
	}
	else
		CHECK;
/* Select SubstructureRedirectMask event mask with client2 on this window. */
	BASIC_STARTCALL(client2);
	XSelectInput(client2, w, SubstructureRedirectMask);
	BASIC_ENDCALL(client2, BadAccess);
/* Verify that a BadAccess error was generated. */
	if (geterr() != BadAccess) {
		report("A call to XSelectInput did not generate BadAccess error");
		FAIL;
	}
	else
		CHECK;

	CHECKPASS(3);
>>ASSERTION Bad A
When another client has selected with an event mask
.S ResizeRedirectMask ,
then on a call to xname
with
.S ResizeRedirectMask
bits set in
.A event_mask
a
.S BadAccess
error occurs.
>>STRATEGY
Create client1.
Create window with client1.
Select ResizeRedirectMask event mask with client1 on this window.
Create client2.
Select ResizeRedirectMask event mask with client2 on this window.
Verify that a BadAccess error was generated.
>>CODE
Display *client1;
Display *client2;

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
/* Select ResizeRedirectMask event mask with client1 on this window. */
	BASIC_STARTCALL(client1);
	XSelectInput(client1, w, ResizeRedirectMask);
	BASIC_ENDCALL(client1, Success);
/* Create client2. */
	client2 = opendisplay();
	if (client2 == (Display *) NULL) {
		delete("Can not open display");
		return;
	}
	else
		CHECK;
/* Select ResizeRedirectMask event mask with client2 on this window. */
	BASIC_STARTCALL(client2);
	XSelectInput(client2, w, ResizeRedirectMask);
	BASIC_ENDCALL(client2, BadAccess);
/* Verify that a BadAccess error was generated. */
	if (geterr() != BadAccess) {
		report("A call to XSelectInput did not generate BadAccess error");
		FAIL;
	}
	else
		CHECK;

	CHECKPASS(3);
>>ASSERTION Bad A
When another client has selected with an event mask
.S ButtonPressMask ,
then on a call to xname
with
.S ButtonPressMask
bits set in
.A event_mask
a
.S BadAccess
error occurs.
>>STRATEGY
Create client1.
Create window with client1.
Select ButtonPressMask event mask with client1 on this window.
Create client2.
Select ButtonPressMask event mask with client2 on this window.
Verify that a BadAccess error was generated.
>>CODE
Display *client1;
Display *client2;

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
/* Select ButtonPressMask event mask with client1 on this window. */
	BASIC_STARTCALL(client1);
	XSelectInput(client1, w, ButtonPressMask);
	BASIC_ENDCALL(client1, Success);
/* Create client2. */
	client2 = opendisplay();
	if (client2 == (Display *) NULL) {
		delete("Can not open display");
		return;
	}
	else
		CHECK;
/* Select ButtonPressMask event mask with client2 on this window. */
	BASIC_STARTCALL(client2);
	XSelectInput(client2, w, ButtonPressMask);
	BASIC_ENDCALL(client2, BadAccess);
/* Verify that a BadAccess error was generated. */
	if (geterr() != BadAccess) {
		report("A call to XSelectInput did not generate BadAccess error");
		FAIL;
	}
	else
		CHECK;

	CHECKPASS(3);
>>ASSERTION Bad A
.ER BadWindow

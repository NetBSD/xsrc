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
 * $XConsortium: mpntfy.m,v 1.10 94/04/17 21:07:51 rws Exp $
 */
>>TITLE MapNotify CH08
>>EXTERN
#define	EVENT		MapNotify
#define	MASK		StructureNotifyMask
#define	MASKP		SubstructureNotifyMask
>>ASSERTION Good A
When a window's state changes from unmapped to mapped
as a result of save-set processing,
then a xname event is generated.
>>STRATEGY
Create client2.
Create parent window with client2.
Create child window as inferior to parent.
Add child window to client2's save-set.
Set client2's shut down mode to DestroyAll.
Select for MapNotify events on child.
Call XCloseDisplay with client2.
Verify that a MapNotify event was delivered.
Verify that event member fields are correctly set.
>>CODE
Display	*display = Dsp;
Display	*client2;
Window	child;
Window	parent;
XEvent	event_return;
XMapEvent good;
struct area area;

/* Create client2. */
	if (config.display == (char *) NULL) {
		delete("config.display not set");
		return;
	}
	else
		CHECK;
	client2 = XOpenDisplay(config.display);
	if (client2 == (Display *) NULL) {
		delete("Couldn't create client2.");
		return;
	}
	else
		CHECK;
/* Create parent window with client2. */
	/* must avoid resource registration! */
	parent = XCreateSimpleWindow(display, DRW(display), 1, 1, W_STDWIDTH, W_STDHEIGHT, 1, 0L, 0L);
	XSync(client2, True);
/* Create child window as inferior to parent. */
	area.x = 1;
	area.y = 1;
	area.width = W_STDWIDTH/2;
	area.height = W_STDHEIGHT/2;
	child = mkwinchild(display, (XVisualInfo *) NULL, &area, False, parent, 1);
	XSync(display, True);
/* Add child window to client2's save-set. */
	XAddToSaveSet(client2, child);
/* Set client2's shut down mode to DestroyAll. */
	XSetCloseDownMode(client2, DestroyAll);
/* Select for MapNotify events on child. */
	XSelectInput(display, child, MASK);
/* Call XCloseDisplay with client2. */
	XSync(display, True);
	XSync(client2, True);
	XCloseDisplay(client2);
	sleep(config.speedfactor);
	XSync(display, False);
/* Verify that a MapNotify event was delivered. */
	if (!XCheckTypedWindowEvent(display, child, EVENT, &event_return)) {
		report("Expected %s event, got none", eventname(EVENT));
		FAIL;
	}
	else
		CHECK;
/* Verify that event member fields are correctly set. */
	good = event_return.xmap;
	good.type = EVENT;
	good.event = child;
	good.window = child;
	if (checkevent((XEvent *) &good, &event_return)) {
		report("Unexpected values in delivered event");
		FAIL;
	}
	else
		CHECK;

	CHECKPASS(4);
>>#NOTEm >>ASSERTION
>>#NOTEm When a window's state changes from unmapped to mapped
>>#NOTEm as a result of a call to
>>#NOTEm .F XMapWindow ,
>>#NOTEm .F XMapRaised ,
>>#NOTEm .F XMapSubwindows ,
>>#NOTEm or
>>#NOTEm .F XReparentWindow ,
>>#NOTEm then ARTICLE xname event is generated.
>>ASSERTION Good A
When a xname event is generated,
then all clients having set
.S StructureNotifyMask
event mask bits on the mapped window are delivered
a xname event.
>>STRATEGY
Create clients client2 and client3.
Create window.
Select for MapNotify events using StructureNotifyMask.
Select for MapNotify events using StructureNotifyMask with client2.
Select for no events with client3.
Generate MapNotify event.
Verify that a MapNotify event was delivered.
Verify that event member fields are correctly set.
Verify that a MapNotify event was delivered to client2.
Verify that event member fields are correctly set.
Verify that no events were delivered to client3.
>>CODE
Display	*display = Dsp;
Display	*client2;
Display	*client3;
Window	w;
int	count;
XEvent	event_return;
XMapEvent good;

/* Create clients client2 and client3. */
	if ((client2 = opendisplay()) == (Display *) NULL) {
		delete("Couldn't create client2.");
		return;
	}
	else
		CHECK;
	if ((client3 = opendisplay()) == (Display *) NULL) {
		delete("Couldn't create client3.");
		return;
	}
	else
		CHECK;
/* Create window. */
	w = mkwin(display, (XVisualInfo *) NULL, (struct area *) NULL, False);
/* Select for MapNotify events using StructureNotifyMask. */
	XSelectInput(display, w, MASK);
/* Select for MapNotify events using StructureNotifyMask with client2. */
	XSelectInput(client2, w, MASK);
/* Select for no events with client3. */
	XSelectInput(client3, w, NoEventMask);
/* Generate MapNotify event. */
	XSync(display, True);
	XSync(client2, True);
	XSync(client3, True);
	XMapWindow(display, w);
	XSync(display, False);
	XSync(client2, False);
	XSync(client3, False);
/* Verify that a MapNotify event was delivered. */
/* Verify that event member fields are correctly set. */
	if (!XCheckTypedWindowEvent(display, w, EVENT, &event_return)) {
		report("Expected %s event, got none", eventname(EVENT));
		FAIL;
	}
	else
		CHECK;
	good = event_return.xmap;
	good.type = EVENT;
	good.event = w;
	good.window = w;
	good.override_redirect = config.debug_override_redirect;
	if (checkevent((XEvent *) &good, &event_return)) {
		report("Unexpected values in delivered event");
		FAIL;
	}
	else
		CHECK;
/* Verify that a MapNotify event was delivered to client2. */
/* Verify that event member fields are correctly set. */
	if (!XCheckTypedWindowEvent(client2, w, EVENT, &event_return)) {
		report("Expected %s event, got none", eventname(EVENT));
		FAIL;
	}
	else
		CHECK;
	good = event_return.xmap;
	good.type = EVENT;
	good.event = w;
	good.window = w;
	good.override_redirect = config.debug_override_redirect;
	if (checkevent((XEvent *) &good, &event_return)) {
		report("Unexpected values in delivered event for client2");
		FAIL;
	}
	else
		CHECK;
/* Verify that no events were delivered to client3. */
	count = XPending(client3);
	if (count != 0) {
		report("Got %d events, expected %d for client3", count, 0);
		FAIL;
		return;
	}
	else
		CHECK;
	
	CHECKPASS(7);
>>ASSERTION Good A
When a xname event is generated,
then all clients having set
.S SubstructureNotifyMask
event mask bits on the parent of the mapped window are delivered
a xname event.
>>STRATEGY
Create clients client2 and client3.
Create window.
Select for MapNotify events using SubstructureNotifyMask.
Select for MapNotify events using SubstructureNotifyMask with client2.
Select for no events with client3.
Generate MapNotify event.
Verify that a MapNotify event was delivered.
Verify that event member fields are correctly set.
Verify that a MapNotify event was delivered to client2.
Verify that event member fields are correctly set.
Verify that no events were delivered to client3.
>>CODE
Display	*display = Dsp;
Display	*client2;
Display	*client3;
Window	w;
int	count;
XEvent	event_return;
XMapEvent good;

/* Create clients client2 and client3. */
	if ((client2 = opendisplay()) == (Display *) NULL) {
		delete("Couldn't create client2.");
		return;
	}
	else
		CHECK;
	if ((client3 = opendisplay()) == (Display *) NULL) {
		delete("Couldn't create client3.");
		return;
	}
	else
		CHECK;
/* Create window. */
	w = mkwin(display, (XVisualInfo *) NULL, (struct area *) NULL, False);
/* Select for MapNotify events using SubstructureNotifyMask. */
	XSelectInput(display, DRW(display), MASKP);
/* Select for MapNotify events using SubstructureNotifyMask with client2. */
	XSelectInput(client2, DRW(display), MASKP);
/* Select for no events with client3. */
	XSelectInput(client3, DRW(display), NoEventMask);
/* Generate MapNotify event. */
	XSync(display, True);
	XSync(client2, True);
	XSync(client3, True);
	XMapWindow(display, w);
	XSync(display, False);
	XSync(client2, False);
	XSync(client3, False);
/* Verify that a MapNotify event was delivered. */
/* Verify that event member fields are correctly set. */
	if (!XCheckTypedWindowEvent(display, DRW(display), EVENT, &event_return)) {
		report("Expected %s event, got none", eventname(EVENT));
		FAIL;
	}
	else
		CHECK;
	good = event_return.xmap;
	good.type = EVENT;
	good.event = DRW(display);
	good.window = w;
	good.override_redirect = config.debug_override_redirect;
	if (checkevent((XEvent *) &good, &event_return)) {
		report("Unexpected values in delivered event");
		FAIL;
	}
	else
		CHECK;
/* Verify that a MapNotify event was delivered to client2. */
/* Verify that event member fields are correctly set. */
	if (!XCheckTypedWindowEvent(client2, DRW(display), EVENT, &event_return)) {
		report("Expected %s event, got none", eventname(EVENT));
		FAIL;
	}
	else
		CHECK;
	good = event_return.xmap;
	good.type = EVENT;
	good.event = DRW(display);
	good.window = w;
	good.override_redirect = config.debug_override_redirect;
	if (checkevent((XEvent *) &good, &event_return)) {
		report("Unexpected values in delivered event for client2");
		FAIL;
	}
	else
		CHECK;
/* Verify that no events were delivered to client3. */
	count = XPending(client3);
	if (count != 0) {
		report("Got %d events, expected %d for client3", count, 0);
		FAIL;
		return;
	}
	else
		CHECK;
	
	CHECKPASS(7);
>>ASSERTION def
>>#NOTE	Tested for in previous two assertions.
When a xname event is generated,
then
clients not having set
.S StructureNotifyMask
event mask bits on the
mapped window
and also not having set
.S SubstructureNotifyMask
event mask bits on the
parent of the mapped window
are not delivered
a xname event.
>>#NOTEs >>ASSERTION
>>#NOTEs When ARTICLE xname event is delivered,
>>#NOTEs then
>>#NOTEs .M type
>>#NOTEs is set to
>>#NOTEs xname.
>>#NOTEs >>ASSERTION
>>#NOTEs >>#NOTE The method of expansion is not clear.
>>#NOTEs When ARTICLE xname event is delivered,
>>#NOTEs then
>>#NOTEs .M serial
>>#NOTEs is set
>>#NOTEs from the serial number reported in the protocol
>>#NOTEs but expanded from the 16-bit least-significant bits
>>#NOTEs to a full 32-bit value.
>>#NOTEm >>ASSERTION
>>#NOTEm When ARTICLE xname event is delivered
>>#NOTEm and the event came from a
>>#NOTEm .S SendEvent
>>#NOTEm protocol request,
>>#NOTEm then
>>#NOTEm .M send_event
>>#NOTEm is set to
>>#NOTEm .S True .
>>#NOTEs >>ASSERTION
>>#NOTEs When ARTICLE xname event is delivered
>>#NOTEs and the event was not generated by a
>>#NOTEs .S SendEvent
>>#NOTEs protocol request,
>>#NOTEs then
>>#NOTEs .M send_event
>>#NOTEs is set to
>>#NOTEs .S False .
>>#NOTEs >>ASSERTION
>>#NOTEs When ARTICLE xname event is delivered,
>>#NOTEs then
>>#NOTEs .M display
>>#NOTEs is set to
>>#NOTEs a pointer to the display on which the event was read.
>>#NOTEs >>ASSERTION
>>#NOTEs When ARTICLE xname event is delivered
>>#NOTEs and
>>#NOTEs .S StructureNotify
>>#NOTEs was selected,
>>#NOTEs then
>>#NOTEs .M event
>>#NOTEs is set to
>>#NOTEs the WINDOWTYPE window.
>>#NOTEs >>ASSERTION
>>#NOTEs When ARTICLE xname event is delivered
>>#NOTEs and
>>#NOTEs .S SubstructureNotify
>>#NOTEs was selected,
>>#NOTEs then
>>#NOTEs .M event
>>#NOTEs is set to
>>#NOTEs the WINDOWTYPE window's parent.
>>#NOTEs >>ASSERTION
>>#NOTEs >>#NOTE Global except for MappingNotify and KeymapNotify.
>>#NOTEs When ARTICLE xname event is delivered,
>>#NOTEs then
>>#NOTEs .M window
>>#NOTEs is set to
>>#NOTEs the
>>#NOTEs ifdef(`WINDOWTYPE', WINDOWTYPE, event)
>>#NOTEs window.
>>#NOTEs >>ASSERTION
>>#NOTEs When ARTICLE xname event is delivered,
>>#NOTEs then
>>#NOTEs .M override_redirect
>>#NOTEs is set to the override-redirect attribute of
>>#NOTEs .M window .

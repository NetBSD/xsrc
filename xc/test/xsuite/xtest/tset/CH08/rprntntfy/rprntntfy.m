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
 * $XConsortium: rprntntfy.m,v 1.10 94/04/17 21:08:09 rws Exp $
 */
>>TITLE ReparentNotify CH08
>>EXTERN
#define	EVENT		ReparentNotify
#define	MASK		StructureNotifyMask
#define	MASKP		SubstructureNotifyMask
>>ASSERTION Good A
When a xname event is generated,
then all clients having set
.S StructureNotifyMask
event mask bits on the reparented window are delivered
a xname event.
>>STRATEGY
Create clients client2 and client3.
Create parent window.
Create window.
Select for ReparentNotify events using StructureNotifyMask.
Select for ReparentNotify events using StructureNotifyMask with client2.
Select for no events with client3.
Generate ReparentNotify event.
Verify that a ReparentNotify event was delivered.
Verify that event member fields are correctly set.
Verify that a ReparentNotify event was delivered to client2.
Verify that event member fields are correctly set.
Verify that no events were delivered to client3.
>>CODE
Display	*display = Dsp;
Display	*client2;
Display	*client3;
Window	parent, w;
int	count;
XEvent	event_return;
XReparentEvent good;

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
	/*
	 * The order in which these windows are created is critical.
	 * If the order were reversed, a BadWindow error would result
	 * when the resource registration routines destroyed these windows.
	 */
/* Create parent window. */
	parent = mkwin(display, (XVisualInfo *) NULL, (struct area *) NULL, False);
/* Create window. */
	w = mkwin(display, (XVisualInfo *) NULL, (struct area *) NULL, False);
/* Select for ReparentNotify events using StructureNotifyMask. */
	XSelectInput(display, w, MASK);
/* Select for ReparentNotify events using StructureNotifyMask with client2. */
	XSelectInput(client2, w, MASK);
/* Select for no events with client3. */
	XSelectInput(client3, w, NoEventMask);
/* Generate ReparentNotify event. */
	XSync(display, True);
	XSync(client2, True);
	XSync(client3, True);
	XReparentWindow(display, w, parent, 1, 1);
	XSync(display, False);
	XSync(client2, False);
	XSync(client3, False);
/* Verify that a ReparentNotify event was delivered. */
/* Verify that event member fields are correctly set. */
	if (!XCheckTypedWindowEvent(display, w, EVENT, &event_return)) {
		report("Expected %s event, got none", eventname(EVENT));
		FAIL;
	}
	else
		CHECK;
	good = event_return.xreparent;
	good.type = EVENT;
	good.event = w;
	good.window = w;
	good.parent = parent;
	good.x = 1;
	good.y = 1;
	good.override_redirect = config.debug_override_redirect;
	if (checkevent((XEvent *) &good, &event_return)) {
		report("Unexpected values in delivered event");
		FAIL;
	}
	else
		CHECK;
/* Verify that a ReparentNotify event was delivered to client2. */
/* Verify that event member fields are correctly set. */
	if (!XCheckTypedWindowEvent(client2, w, EVENT, &event_return)) {
		report("Expected %s event, got none", eventname(EVENT));
		FAIL;
	}
	else
		CHECK;
	good = event_return.xreparent;
	good.type = EVENT;
	good.event = w;
	good.window = w;
	good.parent = parent;
	good.x = 1;
	good.y = 1;
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
event mask bits on the parent of the reparented window are delivered
a xname event.
>>STRATEGY
Create clients client2 and client3.
Create parent window.
Create window.
Select for ReparentNotify events on new parent using SubstructureNotifyMask.
Select for ReparentNotify events on old parent using SubstructureNotifyMask.
Select for ReparentNotify events on new parent using SubstructureNotifyMask with client2.
Select for ReparentNotify events on old parent using SubstructureNotifyMask with client2.
Select for no events on new parent with client3.
Select for no events on old parent with client3.
Generate ReparentNotify event.
Verify that a ReparentNotify event was delivered.
Verify that event member fields are correctly set.
Verify that a ReparentNotify event was delivered for oldparent.
Verify that event member fields are correctly set.
Verify that a ReparentNotify event was delivered to client2.
Verify that event member fields are correctly set.
Verify that a ReparentNotify event was delivered to client2 for old parent.
Verify that event member fields are correctly set.
Verify that no events were delivered to client3.
>>CODE
Display	*display = Dsp;
Display	*client2;
Display	*client3;
Window	oldparent, parent, w;
int	count;
XEvent	event_return;
XReparentEvent good;

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
	oldparent = DRW(display);
	/*
	 * The order in which these windows are created is critical.
	 * If the order were reversed, a BadWindow error would result
	 * when the resource registration routines destroyed these windows.
	 */
/* Create parent window. */
	parent = mkwin(display, (XVisualInfo *) NULL, (struct area *) NULL, False);
/* Create window. */
	w = mkwin(display, (XVisualInfo *) NULL, (struct area *) NULL, False);
/* Select for ReparentNotify events on new parent using SubstructureNotifyMask. */
	XSelectInput(display, parent, MASKP);
/* Select for ReparentNotify events on old parent using SubstructureNotifyMask. */
	XSelectInput(display, oldparent, MASKP);
/* Select for ReparentNotify events on new parent using SubstructureNotifyMask with client2. */
	XSelectInput(client2, parent, MASKP);
/* Select for ReparentNotify events on old parent using SubstructureNotifyMask with client2. */
	XSelectInput(client2, oldparent, MASKP);
/* Select for no events on new parent with client3. */
	XSelectInput(client3, parent, NoEventMask);
/* Select for no events on old parent with client3. */
	XSelectInput(client3, oldparent, NoEventMask);
/* Generate ReparentNotify event. */
	XSync(display, True);
	XSync(client2, True);
	XSync(client3, True);
	XReparentWindow(display, w, parent, 1, 1);
	XSync(display, False);
	XSync(client2, False);
	XSync(client3, False);
/* Verify that a ReparentNotify event was delivered. */
/* Verify that event member fields are correctly set. */
	if (!XCheckTypedWindowEvent(display, parent, EVENT, &event_return)) {
		report("Expected %s event, got none", eventname(EVENT));
		FAIL;
	}
	else
		CHECK;
	good = event_return.xreparent;
	good.type = EVENT;
	good.event = parent;
	good.window = w;
	good.parent = parent;
	good.x = 1;
	good.y = 1;
	good.override_redirect = config.debug_override_redirect;
	if (checkevent((XEvent *) &good, &event_return)) {
		report("Unexpected values in delivered event");
		FAIL;
	}
	else
		CHECK;
/* Verify that a ReparentNotify event was delivered for oldparent. */
/* Verify that event member fields are correctly set. */
	if (!XCheckTypedWindowEvent(display, oldparent, EVENT, &event_return)) {
		report("Expected %s event, got none", eventname(EVENT));
		FAIL;
	}
	else
		CHECK;
	good = event_return.xreparent;
	good.type = EVENT;
	good.event = oldparent;
	good.window = w;
	good.parent = parent;
	good.x = 1;
	good.y = 1;
	good.override_redirect = config.debug_override_redirect;
	if (checkevent((XEvent *) &good, &event_return)) {
		report("Unexpected values in delivered event");
		FAIL;
	}
	else
		CHECK;
/* Verify that a ReparentNotify event was delivered to client2. */
/* Verify that event member fields are correctly set. */
	if (!XCheckTypedWindowEvent(client2, parent, EVENT, &event_return)) {
		report("Expected %s event, got none", eventname(EVENT));
		FAIL;
	}
	else
		CHECK;
	good = event_return.xreparent;
	good.type = EVENT;
	good.event = parent;
	good.window = w;
	good.parent = parent;
	good.x = 1;
	good.y = 1;
	good.override_redirect = config.debug_override_redirect;
	if (checkevent((XEvent *) &good, &event_return)) {
		report("Unexpected values in delivered event for client2");
		FAIL;
	}
	else
		CHECK;
/* Verify that a ReparentNotify event was delivered to client2 for old parent. */
/* Verify that event member fields are correctly set. */
	if (!XCheckTypedWindowEvent(client2, oldparent, EVENT, &event_return)) {
		report("Expected %s event, got none", eventname(EVENT));
		FAIL;
	}
	else
		CHECK;
	good = event_return.xreparent;
	good.type = EVENT;
	good.event = oldparent;
	good.window = w;
	good.parent = parent;
	good.x = 1;
	good.y = 1;
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
	
	CHECKPASS(11);
>>ASSERTION def
>>#NOTE	Tested for in previous assertion.
When a xname event is generated,
then all clients having set
.S SubstructureNotifyMask
event mask bits on either the old or new parent of
the reparented window are delivered
a xname event.
>>ASSERTION def
>>#NOTE	Tested for in preceeding assertions.
When a xname event is generated,
then
clients not having set
.S StructureNotifyMask
event mask bits on the
reparented window
and also not having set
.S SubstructureNotifyMask
event mask bits on either the
old or new parent of the reparented window
are not delivered
a xname event.
>>#NOTEm >>ASSERTION
>>#NOTEm When a window's parent changes
>>#NOTEm as a result of a call to
>>#NOTEm .F XReparentWindow ,
>>#NOTEm then ARTICLE xname event is generated.
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
>>#NOTEs .M parent
>>#NOTEs is set to
>>#NOTEs the parent window of
>>#NOTEs .M window .
>>#NOTEs >>ASSERTION
>>#NOTEs When ARTICLE xname event is delivered,
>>#NOTEs then
>>#NOTEs .M x
>>#NOTEs and
>>#NOTEs .M y
>>#NOTEs are set to
>>#NOTEs the coordinates of
>>#NOTEs .M window
>>#NOTEs relative to parent window's origin
>>#NOTEs and indicate the position of the upper-left outside corner of
>>#NOTEs .M window .
>>#NOTEs >>ASSERTION
>>#NOTEs When ARTICLE xname event is delivered,
>>#NOTEs then
>>#NOTEs .M override_redirect
>>#NOTEs is set to the override-redirect attribute of
>>#NOTEs .M window .

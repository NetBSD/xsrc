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
 * $XConsortium: unmpntfy.m,v 1.9 94/04/17 21:08:21 rws Exp $
 */
>>TITLE UnmapNotify CH08
>>EXTERN
#define	EVENT		UnmapNotify
#define	MASK		StructureNotifyMask
#define	MASKP		SubstructureNotifyMask
>>ASSERTION Good A
When an xname event is generated,
then all clients having set
.S StructureNotifyMask
event mask bits on the unmapped window are delivered
an xname event.
>>STRATEGY
Create clients client2 and client3.
Create window.
Select for UnmapNotify events using StructureNotifyMask.
Select for UnmapNotify events using StructureNotifyMask with client2.
Select for no events with client3.
Generate UnmapNotify event.
Verify that a UnmapNotify event was delivered.
Verify that event member fields are correctly set.
Verify that a UnmapNotify event was delivered to client2.
Verify that event member fields are correctly set.
Verify that no events were delivered to client3.
>>CODE
Display	*display = Dsp;
Display	*client2;
Display	*client3;
Window	w;
int	count;
XEvent	event_return;
XUnmapEvent good;

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
	w = mkwin(display, (XVisualInfo *) NULL, (struct area *) NULL, True);
/* Select for UnmapNotify events using StructureNotifyMask. */
	XSelectInput(display, w, MASK);
/* Select for UnmapNotify events using StructureNotifyMask with client2. */
	XSelectInput(client2, w, MASK);
/* Select for no events with client3. */
	XSelectInput(client3, w, NoEventMask);
/* Generate UnmapNotify event. */
	XSync(display, True);
	XSync(client2, True);
	XSync(client3, True);
	XUnmapWindow(display, w);
	XSync(display, False);
	XSync(client2, False);
	XSync(client3, False);
/* Verify that a UnmapNotify event was delivered. */
/* Verify that event member fields are correctly set. */
	if (!XCheckTypedWindowEvent(display, w, EVENT, &event_return)) {
		report("Expected %s event, got none", eventname(EVENT));
		FAIL;
	}
	else
		CHECK;
	good = event_return.xunmap;
	good.type = EVENT;
	good.event = w;
	good.window = w;
	good.from_configure = False;
	if (checkevent((XEvent *) &good, &event_return)) {
		report("Unexpected values in delivered event");
		FAIL;
	}
	else
		CHECK;
/* Verify that a UnmapNotify event was delivered to client2. */
/* Verify that event member fields are correctly set. */
	if (!XCheckTypedWindowEvent(client2, w, EVENT, &event_return)) {
		report("Expected %s event, got none", eventname(EVENT));
		FAIL;
	}
	else
		CHECK;
	good = event_return.xunmap;
	good.type = EVENT;
	good.event = w;
	good.window = w;
	good.from_configure = False;
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
When an xname event is generated,
then all clients having set
.S SubstructureNotifyMask
event mask bits on the parent of the unmapped window are delivered
an xname event.
>>STRATEGY
Create clients client2 and client3.
Create window.
Select for UnmapNotify events using SubstructureNotifyMask.
Select for UnmapNotify events using SubstructureNotifyMask with client2.
Select for no events with client3.
Generate UnmapNotify event.
Verify that a UnmapNotify event was delivered.
Verify that event member fields are correctly set.
Verify that a UnmapNotify event was delivered to client2.
Verify that event member fields are correctly set.
Verify that no events were delivered to client3.
>>CODE
Display	*display = Dsp;
Display	*client2;
Display	*client3;
Window	parent, w;
int	count;
XEvent	event_return;
XUnmapEvent good;

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
	parent = DRW(display);
/* Create window. */
	w = mkwin(display, (XVisualInfo *) NULL, (struct area *) NULL, True);
/* Select for UnmapNotify events using SubstructureNotifyMask. */
	XSelectInput(display, parent, MASKP);
/* Select for UnmapNotify events using SubstructureNotifyMask with client2. */
	XSelectInput(client2, parent, MASKP);
/* Select for no events with client3. */
	XSelectInput(client3, parent, NoEventMask);
/* Generate UnmapNotify event. */
	XSync(display, True);
	XSync(client2, True);
	XSync(client3, True);
	XUnmapWindow(display, w);
	XSync(display, False);
	XSync(client2, False);
	XSync(client3, False);
/* Verify that a UnmapNotify event was delivered. */
/* Verify that event member fields are correctly set. */
	if (!XCheckTypedWindowEvent(display, parent, EVENT, &event_return)) {
		report("Expected %s event, got none", eventname(EVENT));
		FAIL;
	}
	else
		CHECK;
	good = event_return.xunmap;
	good.type = EVENT;
	good.event = parent;
	good.window = w;
	good.from_configure = False;
	if (checkevent((XEvent *) &good, &event_return)) {
		report("Unexpected values in delivered event");
		FAIL;
	}
	else
		CHECK;
/* Verify that a UnmapNotify event was delivered to client2. */
/* Verify that event member fields are correctly set. */
	if (!XCheckTypedWindowEvent(client2, parent, EVENT, &event_return)) {
		report("Expected %s event, got none", eventname(EVENT));
		FAIL;
	}
	else
		CHECK;
	good = event_return.xunmap;
	good.type = EVENT;
	good.event = parent;
	good.window = w;
	good.from_configure = False;
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
When an xname event is generated,
then
clients not having set
.S StructureNotifyMask
event mask bits on the
unmapped window
and also not having set
.S SubstructureNotifyMask
event mask bits on the
parent of the unmapped window
are not delivered
an xname event.
>>#NOTEm >>ASSERTION
>>#NOTEm When a window's state changes
>>#NOTEm from mapped to unmapped,
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
>>#NOTEm >>ASSERTION
>>#NOTEm When ARTICLE xname
>>#NOTEm event is delivered
>>#NOTEm and
>>#NOTEm .M window
>>#NOTEm was unmapped as a result of a resizing of the window's parent when
>>#NOTEm the window's
>>#NOTEm .M win_gravity
>>#NOTEm attribute is set to
>>#NOTEm .S UnmapGravity ,
>>#NOTEm then
>>#NOTEm .M from_configure
>>#NOTEm is set to
>>#NOTEm .S True .
>>#NOTEm >>ASSERTION
>>#NOTEm When ARTICLE xname
>>#NOTEm event is delivered
>>#NOTEm and
>>#NOTEm .M window
>>#NOTEm was not unmapped as a result of a resizing of the window's parent when
>>#NOTEm the window's
>>#NOTEm .M win_gravity
>>#NOTEm attribute is set to
>>#NOTEm .S UnmapGravity ,
>>#NOTEm then
>>#NOTEm .M from_configure
>>#NOTEm is set to
>>#NOTEm .S False .

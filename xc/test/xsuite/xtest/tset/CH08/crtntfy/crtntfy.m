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
 * $XConsortium: crtntfy.m,v 1.11 94/04/17 21:07:23 rws Exp $
 */
>>TITLE CreateNotify CH08
>>EXTERN
#define	EVENT		CreateNotify
#define	MASK		SubstructureNotifyMask
>>ASSERTION Good A
When a xname event is generated,
then all clients having set
.S SubstructureNotifyMask
event mask bits on the parent of the created window are delivered
a xname event.
>>STRATEGY
Create clients client2 and client3.
Create parent window.
Select for CreateNotify events using StructureNotifyMask.
Select for CreateNotify events using StructureNotifyMask with client2.
Select for no events with client3.
Generate CreateNotify event.
Verify that a CreateNotify event was delivered.
Verify that event member fields are correctly set.
Verify that a CreateNotify event was delivered to client2.
Verify that no events were delivered to client3.
>>CODE
Display	*display = Dsp;
Display	*client2;
Display	*client3;
Window	parent, child;
int	count;
XEvent	event_return;
XCreateWindowEvent good;

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
/* Create parent window. */
	parent = mkwin(display, (XVisualInfo *) NULL, (struct area *) NULL, False);
/* Select for CreateNotify events using StructureNotifyMask. */
	XSelectInput(display, parent, MASK);
/* Select for CreateNotify events using StructureNotifyMask with client2. */
	XSelectInput(client2, parent, MASK);
/* Select for no events with client3. */
	XSelectInput(client3, parent, NoEventMask);
/* Generate CreateNotify event. */
	XSync(display, True);
	XSync(client2, True);
	XSync(client3, True);
	child = mkwinchild(display, (XVisualInfo *) NULL, (struct area *) NULL, False, parent, 1);
	XSync(display, False);
	XSync(client2, False);
	XSync(client3, False);
/* Verify that a CreateNotify event was delivered. */
/* Verify that event member fields are correctly set. */
	count = XPending(display);
	if (count != 1) {
		report("Got %d events, expected %d", count, 1);
		FAIL;
		return;
	}
	else
		CHECK;
	XNextEvent(display, &event_return);
	good = event_return.xcreatewindow;
	good.type = EVENT;
	good.parent = parent;
	good.window = child;
	if (checkevent((XEvent *) &good, &event_return)) {
		report("Unexpected values in delivered event");
		FAIL;
	}
	else
		CHECK;
/* Verify that a CreateNotify event was delivered to client2. */
	count = XPending(client2);
	if (count != 1) {
		report("Got %d events, expected %d for client2", count, 1);
		FAIL;
		return;
	}
	else
		CHECK;
	XNextEvent(client2, &event_return);
	good = event_return.xcreatewindow;
	good.type = EVENT;
	good.parent = parent;
	good.window = child;
	if (checkevent((XEvent *) &good, &event_return)) {
		report("Unexpected values in delivered event to client2");
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
>>#NOTE Tested for in previous assertion.
When a xname event is generated,
then
clients not having set
.S SubstructureNotifyMask
event mask bits on the event window are not delivered
a xname event.
>>#NOTEm >>ASSERTION
>>#NOTEm When a window is created
>>#NOTEm as a result of a call to
>>#NOTEm .F XCreateWindow
>>#NOTEm or
>>#NOTEm .F XCreateSimpleWindow ,
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
>>#NOTEs When ARTICLE xname event is delivered,
>>#NOTEs then
>>#NOTEs .M parent
>>#NOTEs is set to
>>#NOTEs the parent window of
>>#NOTEs .M window .
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
>>#NOTEs .M width
>>#NOTEs and
>>#NOTEs .M height
>>#NOTEs are set to
>>#NOTEs the
>>#NOTEs ifdef(`REQUESTED', REQUESTED,)
>>#NOTEs inside size of
>>#NOTEs .M window ,
>>#NOTEs not including the border.
>>#NOTEs >>ASSERTION
>>#NOTEs When ARTICLE xname event is delivered,
>>#NOTEs then
>>#NOTEs then
>>#NOTEs .M width
>>#NOTEs and
>>#NOTEs .M height
>>#NOTEs are always non-zero.
>>#NOTEs >>ASSERTION
>>#NOTEs When ARTICLE xname event is delivered,
>>#NOTEs then
>>#NOTEs .M border_width
>>#NOTEs is set to
>>#NOTEs the width in pixels of
>>#NOTEs .M window 's
>>#NOTEs border.
>>#NOTEs >>ASSERTION
>>#NOTEs When ARTICLE xname event is delivered,
>>#NOTEs then
>>#NOTEs .M override_redirect
>>#NOTEs is set to the override-redirect attribute of
>>#NOTEs .M window .

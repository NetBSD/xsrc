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
 * $XConsortium: cnfgrntfy.m,v 1.10 94/04/17 21:07:18 rws Exp $
 */
>>TITLE ConfigureNotify CH08
>>EXTERN
#define	EVENT		ConfigureNotify
#define	MASK		StructureNotifyMask
#define	MASKP		SubstructureNotifyMask
>>ASSERTION Good A
When a xname event is generated,
then all clients having set
.S StructureNotifyMask
event mask bits on the reconfigured window are delivered
a xname event.
>>STRATEGY
Create clients client2 and client3.
Build and create window hierarchy.
Select for ConfigureNotify events using StructureNotifyMask.
Select for ConfigureNotify events using StructureNotifyMask with client2.
Select for no events with client3.
Raise lowest window to top.
Verify that a ConfigureNotify event is delivered.
Verify that a ConfigureNotify event is delivered to client2.
Verify that no events are delivered to client3.
Verify that event member fields are correctly set.
>>CODE
Display	*display = Dsp;
Display	*client2;
Display	*client3;
Winh	*parent, *eventw, *lastw;
Winhg	winhg;
int	i;
int	status;
int	numchildren = 4;
XEvent	event;

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
/* Build and create window hierarchy. */
	/* can't use winh() because the windows need to overlap */
	winhg.border_width = 1;
	winhg.area.x = 1;
	winhg.area.y = 1;
	winhg.area.width = 100;
	winhg.area.height = 100;
	parent = winh_adopt(display, (Winh *) NULL, 0L, (XSetWindowAttributes *) NULL, &winhg, WINH_NOMASK);
	if (parent == (Winh *) NULL) {
		report("Could not create parent");
		return;
	}
	else
		CHECK;
	winhg.area.x = 1;
	winhg.area.y = 1;
	winhg.area.width = 30;
	winhg.area.height = 30;
	for (i=0; i<numchildren; i++) {
		if (!i)
			CHECK;
		lastw = winh_adopt(display, parent, 0L, (XSetWindowAttributes *) NULL, &winhg, WINH_NOMASK);
		if (lastw == (Winh *) NULL) {
			report("Could not create child %d", i);
			return;
		}
		winhg.area.x += 10;
		winhg.area.y += 10;
	}
	if (winh_create(display, (Winh *) NULL, WINH_MAP))
		return;
	else
		CHECK;
/* Select for ConfigureNotify events using StructureNotifyMask. */
	if (winh_selectinput(display, (Winh *) NULL, MASK)) {
		report("Selection with first client failed.");
		return;
	}
	else
		CHECK;
/* Select for ConfigureNotify events using StructureNotifyMask with client2. */
	if (winh_selectinput(client2, (Winh *) NULL, MASK)) {
		report("Selection with client2 failed.");
		return;
	}
	else
		CHECK;
/* Select for no events with client3. */
	if (winh_selectinput(client3, (Winh *) NULL, NoEventMask)) {
		report("Selection with client3 failed.");
		return;
	}
	else
		CHECK;
/* Raise lowest window to top. */
	XSync(display, True);
	XSync(client2, True);
	XSync(client3, True);
	eventw = parent->firstchild;
	XRaiseWindow(display, eventw->window);
	event.xany.type = EVENT;
	event.xany.window = eventw->window;
	if (winh_plant(eventw, &event, MASK, WINH_NOMASK)) {
		report("Could not plant events for eventw");
		return;
	}
	else
		CHECK;
	XSync(display, False);
	XSync(client2, False);
	XSync(client3, False);
/* Verify that a ConfigureNotify event is delivered. */
/* Verify that a ConfigureNotify event is delivered to client2. */
/* Verify that no events are delivered to client3. */
	if (winh_harvest(display, (Winh *) NULL)) {
		report("Could not harvest events for display");
		return;
	}
	else
		CHECK;
	if (winh_harvest(client2, (Winh *) NULL)) {
		report("Could not harvest events for client2");
		return;
	}
	else
		CHECK;
	if (winh_harvest(client3, (Winh *) NULL)) {
		report("Could not harvest events for client3");
		return;
	}
	else
		CHECK;
	status = winh_weed((Winh *) NULL, -1, WINH_WEED_IDENTITY);
	if (status == -1)
		return;
	else if (status) {
		report("Event delivery not as expected");
		FAIL;
	}
	else {
		XConfigureEvent	good;

/* Verify that event member fields are correctly set. */
		good = winh_qdel->event->xconfigure;
		good.window = good.event;
		good.x = 1;
		good.y = 1;
		good.width = 30;
		good.height = 30;
		good.border_width = 1;
		good.above = lastw->window;
		if (checkevent((XEvent *) &good, winh_qdel->event)) {
			report("Unexpected values in delivered event");
			FAIL;
		}
		else
			CHECK;
	}

	CHECKPASS(13);
>>ASSERTION Good A
When a xname event is generated,
then all clients having set
.S SubstructureNotifyMask
event mask bits on the parent of the reconfigured window are delivered
a xname event.
>>STRATEGY
Create clients client2 and client3.
Build and create window hierarchy.
Select for ConfigureNotify events using SubstructureNotifyMask.
Select for ConfigureNotify events using SubstructureNotifyMask with client2.
Select for no events with client3.
Raise lowest window to top.
Verify that a ConfigureNotify event is delivered.
Verify that a ConfigureNotify event is delivered to client2.
Verify that no events are delivered to client3.
Verify that event member fields are correctly set.
>>CODE
Display	*display = Dsp;
Display	*client2;
Display	*client3;
Winh	*parent, *eventw, *lastw;
Winhg	winhg;
int	i;
int	status;
int	numchildren = 4;
XEvent	event;

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
/* Build and create window hierarchy. */
	/* can't use winh() because the windows need to overlap */
	winhg.border_width = 1;
	winhg.area.x = 1;
	winhg.area.y = 1;
	winhg.area.width = 100;
	winhg.area.height = 100;
	parent = winh_adopt(display, (Winh *) NULL, 0L, (XSetWindowAttributes *) NULL, &winhg, WINH_NOMASK);
	if (parent == (Winh *) NULL) {
		report("Could not create parent");
		return;
	}
	else
		CHECK;
	winhg.area.x = 1;
	winhg.area.y = 1;
	winhg.area.width = 30;
	winhg.area.height = 30;
	for (i=0; i<numchildren; i++) {
		if (!i)
			CHECK;
		lastw = winh_adopt(display, parent, 0L, (XSetWindowAttributes *) NULL, &winhg, WINH_NOMASK);
		if (lastw == (Winh *) NULL) {
			report("Could not create child %d", i);
			return;
		}
		winhg.area.x += 10;
		winhg.area.y += 10;
	}
	if (winh_create(display, (Winh *) NULL, WINH_MAP))
		return;
	else
		CHECK;
/* Select for ConfigureNotify events using SubstructureNotifyMask. */
	if (winh_selectinput(display, (Winh *) NULL, MASKP)) {
		report("Selection with first client failed.");
		return;
	}
	else
		CHECK;
/* Select for ConfigureNotify events using SubstructureNotifyMask with client2. */
	if (winh_selectinput(client2, (Winh *) NULL, MASKP)) {
		report("Selection with client2 failed.");
		return;
	}
	else
		CHECK;
/* Select for no events with client3. */
	if (winh_selectinput(client3, (Winh *) NULL, NoEventMask)) {
		report("Selection with client3 failed.");
		return;
	}
	else
		CHECK;
/* Raise lowest window to top. */
	XSync(display, True);
	XSync(client2, True);
	XSync(client3, True);
	eventw = parent->firstchild;
	XRaiseWindow(display, eventw->window);
	event.xany.type = EVENT;
	event.xany.window = parent->window;
	if (winh_plant(parent, &event, MASKP, WINH_NOMASK)) {
		report("Could not plant events for eventw");
		return;
	}
	else
		CHECK;
	XSync(display, False);
	XSync(client2, False);
	XSync(client3, False);
/* Verify that a ConfigureNotify event is delivered. */
/* Verify that a ConfigureNotify event is delivered to client2. */
/* Verify that no events are delivered to client3. */
	if (winh_harvest(display, (Winh *) NULL)) {
		report("Could not harvest events for display");
		return;
	}
	else
		CHECK;
	if (winh_harvest(client2, (Winh *) NULL)) {
		report("Could not harvest events for client2");
		return;
	}
	else
		CHECK;
	if (winh_harvest(client3, (Winh *) NULL)) {
		report("Could not harvest events for client3");
		return;
	}
	else
		CHECK;
	status = winh_weed((Winh *) NULL, -1, WINH_WEED_IDENTITY);
	if (status == -1)
		return;
	else if (status) {
		report("Event delivery not as expected");
		FAIL;
	}
	else {
		XConfigureEvent	good;

/* Verify that event member fields are correctly set. */
		good = winh_qdel->event->xconfigure;
		good.window = eventw->window;
		good.x = 1;
		good.y = 1;
		good.width = 30;
		good.height = 30;
		good.border_width = 1;
		good.above = lastw->window;
	}

	CHECKPASS(12);
>>ASSERTION def
>>#NOTE	Tested for in previous two assertions.
When a xname event is generated,
then
clients not having set
.S StructureNotifyMask
event mask bits on the
reconfigured window
and also not having set
.S SubstructureNotifyMask
event mask bits on the
parent of the reconfigured window
are not delivered
a xname event.
>>#NOTEm >>ASSERTION
>>#NOTEm When a window's size is changed
>>#NOTEm as a result of a call to
>>#NOTEm .F XConfigureWindow ,
>>#NOTEm then ARTICLE xname event is generated.
>>#NOTEm >>ASSERTION
>>#NOTEm When a window's position is changed
>>#NOTEm as a result of a call to
>>#NOTEm .F XConfigureWindow ,
>>#NOTEm then ARTICLE xname event is generated.
>>#NOTEm >>ASSERTION
>>#NOTEm When a window's border is changed
>>#NOTEm as a result of a call to
>>#NOTEm .F XConfigureWindow ,
>>#NOTEm then ARTICLE xname event is generated.
>>#NOTEm >>ASSERTION
>>#NOTEm When a window's stacking order is changed
>>#NOTEm as a result of a call to
>>#NOTEm .F XConfigureWindow ,
>>#NOTEm then ARTICLE xname event is generated.
>>#NOTEm >>ASSERTION
>>#NOTEm When a window's position in the stacking order is changed
>>#NOTEm as a result of a call to
>>#NOTEm .F XLowerWindow ,
>>#NOTEm .F XRaiseWindow ,
>>#NOTEm or
>>#NOTEm .F XRestackWindows,
>>#NOTEm then ARTICLE xname event is generated.
>>#NOTEm >>ASSERTION
>>#NOTEm When a window is moved
>>#NOTEm as a result of a call to
>>#NOTEm .F XMoveWindow ,
>>#NOTEm then ARTICLE xname event is generated.
>>#NOTEm >>ASSERTION
>>#NOTEm When a window's size is changed
>>#NOTEm as a result of a call to
>>#NOTEm .F XResizeWindow ,
>>#NOTEm then ARTICLE xname event is generated.
>>#NOTEm >>ASSERTION
>>#NOTEm When a window's size and location is changed
>>#NOTEm as a result of a call to
>>#NOTEm .F XMoveResizeWindow ,
>>#NOTEm then ARTICLE xname event is generated.
>>#NOTEm >>ASSERTION
>>#NOTEm When a window is mapped
>>#NOTEm and its position in the stacking order is changed
>>#NOTEm as a result of a call to
>>#NOTEm .F XMapRaised ,
>>#NOTEm then ARTICLE xname event is generated.
>>#NOTEm >>ASSERTION
>>#NOTEm When a window's border width is changed
>>#NOTEm as a result of a call to
>>#NOTEm .F XSetWindowBorderWidth ,
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
>>#NOTEs .M border_width
>>#NOTEs is set to
>>#NOTEs the width in pixels of
>>#NOTEs .M window 's
>>#NOTEs border.
>>#NOTEs >>ASSERTION
>>#NOTEs When ARTICLE xname event is delivered
>>#NOTEs and
>>#NOTEs .M window
>>#NOTEs is on the bottom of the stack with respect to sibilings,
>>#NOTEs then
>>#NOTEs .M above
>>#NOTEs is set to
>>#NOTEs .S None .
>>#NOTEs >>ASSERTION
>>#NOTEs When ARTICLE xname event is delivered
>>#NOTEs and
>>#NOTEs .M window
>>#NOTEs is not on the bottom of the stack with respect to sibilings,
>>#NOTEs then
>>#NOTEs .M above
>>#NOTEs is set to
>>#NOTEs the sibling immediately below
>>#NOTEs .M window .
>>#NOTEs >>ASSERTION
>>#NOTEs When ARTICLE xname event is delivered,
>>#NOTEs then
>>#NOTEs .M override_redirect
>>#NOTEs is set to the override-redirect attribute of
>>#NOTEs .M window .

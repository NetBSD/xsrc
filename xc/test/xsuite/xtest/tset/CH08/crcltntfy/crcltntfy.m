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
 * $XConsortium: crcltntfy.m,v 1.9 94/04/17 21:07:21 rws Exp $
 */
>>TITLE CirculateNotify CH08
>>EXTERN
#define	EVENT		CirculateNotify
#define	MASK		StructureNotifyMask
#define	MASKP		SubstructureNotifyMask
>>ASSERTION Good A
When a xname event is generated,
then all clients having set
.S StructureNotifyMask
event mask bits on the restacked window are delivered
a xname event.
>>STRATEGY
Create clients client2 and client3.
Build and create window hierarchy.
Select for CirculateNotify events using StructureNotifyMask.
Select for CirculateNotify events using StructureNotifyMask with client2.
Select for no events with client3.
Circulate lowest window to top.
Verify that a CirculateNotify event is delivered.
Verify that a CirculateNotify event is delivered to client2.
Verify that no events are delivered to client3.
Verify that, in the delivered event corresponding to the restacked
window which is now on top of all siblings, place is set to PlaceOnTop.
Circulate top window to bottom.
Verify that CirculateNotify events are delivered.
Verify that CirculateNotify events are delivered to client2.
Verify that no events are delivered to client3.
Verify that, in the delivered event corresponding to the restacked
window which is now on below all siblings, place is set to PlaceOnBottom.
>>CODE
Display	*display = Dsp;
Display	*client2;
Display	*client3;
Winh	*parent, *eventw, *winh;
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
		winh = winh_adopt(display, parent, 0L, (XSetWindowAttributes *) NULL, &winhg, WINH_NOMASK);
		if (winh == (Winh *) NULL) {
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
/* Select for CirculateNotify events using StructureNotifyMask. */
	if (winh_selectinput(display, (Winh *) NULL, MASK)) {
		report("Selection with first client failed.");
		return;
	}
	else
		CHECK;
/* Select for CirculateNotify events using StructureNotifyMask with client2. */
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
/* Circulate lowest window to top. */
	XSync(display, True);
	XSync(client2, True);
	XSync(client3, True);
	XCirculateSubwindows(display, parent->window, RaiseLowest);
	eventw = parent->firstchild;
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
/* Verify that a CirculateNotify event is delivered. */
/* Verify that a CirculateNotify event is delivered to client2. */
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
		XCirculateEvent	*e;

/* Verify that, in the delivered event corresponding to the restacked */
/* window which is now on top of all siblings, place is set to PlaceOnTop. */
		e = &(winh_qdel->event->xcirculate);
		if (e->place != PlaceOnTop) {
			report("Got %d value for place, expected PlaceOnTop (%d)",
				e->place, PlaceOnTop);
			FAIL;
		}
		else
			CHECK;
	}
/* Circulate top window to bottom. */
	XSync(display, True);
	XSync(client2, True);
	XSync(client3, True);
	XCirculateSubwindows(display, parent->window, LowerHighest);
	if (winh_plant(eventw, &event, MASK, WINH_NOMASK)) {
		report("Could not plant events for eventw");
		return;
	}
	else
		CHECK;
	XSync(display, False);
	XSync(client2, False);
	XSync(client3, False);
/* Verify that CirculateNotify events are delivered. */
/* Verify that CirculateNotify events are delivered to client2. */
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
		XCirculateEvent	*e;

/* Verify that, in the delivered event corresponding to the restacked */
/* window which is now on below all siblings, place is set to PlaceOnBottom. */
		e = &(winh_qdel->event->xcirculate);
		if (e->place != PlaceOnBottom) {
			report("Got %d value for place, expected PlaceOnBottom (%d)",
				e->place, PlaceOnBottom);
			FAIL;
		}
		else
			CHECK;
	}

	CHECKPASS(18);
>>ASSERTION Good A
When a xname event is generated,
then all clients having set
.S SubstructureNotifyMask
event mask bits on the parent of the restacked window are delivered
a xname event.
>>STRATEGY
Create clients client2 and client3.
Build and create window hierarchy.
Select for CirculateNotify events using SubstructureNotifyMask.
Select for CirculateNotify events using SubstructureNotifyMask with client2.
Select for no events with client3.
Circulate lowest window to top.
Verify that a CirculateNotify event is delivered.
Verify that a CirculateNotify event is delivered to client2.
Verify that no events are delivered to client3.
Verify that, in the delivered event corresponding to the restacked
window which is now on top of all siblings, place is set to PlaceOnTop.
Verify that window member is set to restacked window.
>>CODE
Display	*display = Dsp;
Display	*client2;
Display	*client3;
Winh	*parent, *eventw, *winh;
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
		winh = winh_adopt(display, parent, 0L, (XSetWindowAttributes *) NULL, &winhg, WINH_NOMASK);
		if (winh == (Winh *) NULL) {
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
/* Select for CirculateNotify events using SubstructureNotifyMask. */
	if (winh_selectinput(display, (Winh *) NULL, MASKP)) {
		report("Selection with first client failed.");
		return;
	}
	else
		CHECK;
/* Select for CirculateNotify events using SubstructureNotifyMask with client2. */
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
/* Circulate lowest window to top. */
	XSync(display, True);
	XSync(client2, True);
	XSync(client3, True);
	XCirculateSubwindows(display, parent->window, RaiseLowest);
	eventw = parent->firstchild;
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
/* Verify that a CirculateNotify event is delivered. */
/* Verify that a CirculateNotify event is delivered to client2. */
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
		XCirculateEvent	*e;

/* Verify that, in the delivered event corresponding to the restacked */
/* window which is now on top of all siblings, place is set to PlaceOnTop. */
		e = &(winh_qdel->event->xcirculate);
		if (e->place != PlaceOnTop) {
			report("Got %d value for place, expected PlaceOnTop (%d)",
				e->place, PlaceOnTop);
			FAIL;
		}
		else
			CHECK;
/* Verify that window member is set to restacked window. */
		if (e->window != eventw->window) {
			report("Got 0x%x value for window, expected 0x%x",
				e->window, eventw->window);
			FAIL;
		}
		else
			CHECK;
	}

	CHECKPASS(14);
>>ASSERTION def
>>#NOTE	Tested for in two previous assertions.
When a xname event is generated,
then
clients not having set
.S StructureNotifyMask
event mask bits on the
restacked window
and also not having set
.S SubstructureNotifyMask
event mask bits on the
parent of the restacked window
are not delivered
a xname event.
>>#NOTEm >>ASSERTION
>>#NOTEm When a window is restacked as a result of a call to
>>#NOTEm .F XCirculateSubwindows ,
>>#NOTEm .F XCirculateSubwindowsUp ,
>>#NOTEm or
>>#NOTEm .F XCirculateSubwindowsDown ,
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
>>#NOTEs When ARTICLE xname event is delivered
>>#NOTEs and the restacked window is now on top of all siblings,
>>#NOTEs then
>>#NOTEs .M place
>>#NOTEs is set to
>>#NOTEs .S PlaceOnTop .
>>#NOTEs >>ASSERTION
>>#NOTEs When ARTICLE xname event is delivered
>>#NOTEs and the restacked window is now below all siblings,
>>#NOTEs then
>>#NOTEs .M place
>>#NOTEs is set to
>>#NOTEs .S PlaceOnBottom .

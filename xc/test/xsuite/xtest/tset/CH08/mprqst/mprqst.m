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
 * $XConsortium: mprqst.m,v 1.10 94/04/17 21:07:53 rws Exp $
 */
>>TITLE MapRequest CH08
>>EXTERN
#define	EVENT		MapRequest
#define	MASK		SubstructureRedirectMask
>>ASSERTION Good A
When a xname event is generated,
then
all clients having set
.S SubstructureRedirectMask
event mask bits on the parent of the window
for which the map request was issued are delivered
a xname event.
>>STRATEGY
Create clients client2 and client3.
Build and create window hierarchy.
Create inferiors with override-redirect set to True.
Select for ConfigureRequest events using SubstructureRedirectMask.
Select for no events with client3.
Map window.
Verify that no events were delivered.
Verify that no events were delivered to client3.
Unmap window.
Set the override-redirect flag on inferiors to False.
Attempt to map window.
Initialize for expected events.
Verify that a ConfigureRequest event is delivered.
Verify that no events are delivered to client3.
Verify members of event structure.
Verify that no events were delivered to client3.
>>CODE
Display	*display = Dsp;
Display	*client2, *client3;
Winh	*parent, *child, *lastw, *winh;
Winhg	winhg;
XEvent	event;
int	i;
int	status;
int	numchildren = 2;
int	count;
XSetWindowAttributes attrs;
unsigned long valuemask;

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
/* Create inferiors with override-redirect set to True. */
	attrs.override_redirect = True;
	attrs.border_pixel = W_FG;
	attrs.background_pixel = W_BG;
	valuemask = CWOverrideRedirect | CWBorderPixel | CWBackPixel;
	for (i=0; i<numchildren; i++) {
		if (!i)
			CHECK;
		lastw = winh_adopt(display, parent, valuemask, &attrs, &winhg, WINH_NOMASK);
		if (lastw == (Winh *) NULL) {
			report("Could not create child %d", i);
			return;
		}
		winhg.area.x += 10;
		winhg.area.y += 10;
	}
	child = parent->firstchild;
	if (winh_create(display, (Winh *) NULL, WINH_MAP))
		return;
	else
		CHECK;
/* Select for ConfigureRequest events using SubstructureRedirectMask. */
	/*
	 * Selection with a single client because only one can select
	 * for this event at a time.
	 */
	if (winh_selectinput(display, parent, MASK)) {
		report("Selection failed.");
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
/* Map window. */
	XSync(display, True);
	XSync(client2, True);
	XSync(client3, True);
	XMapWindow(client2, child->window);
	XSync(client2, False);
	XSync(display, False);
	XSync(client3, False);
/* Verify that no events were delivered. */
	count = XPending(display);
	if (count != 0) {
		report("Got %d events, expected %d (with override-redirect set)", count, 0);
		FAIL;
	}
	else
		CHECK;
/* Verify that no events were delivered to client3. */
	count = XPending(client3);
	if (count != 0) {
		report("Got %d events, expected %d for client3 (with override-redirect set)", count, 0);
		FAIL;
	}
	else
		CHECK;
/* Unmap window. */
	/*
	 * this assumes that it was mapped in the first place
	 * which is not the case where the override-redirect flag is ignored
	 */
	XUnmapWindow(client2, child->window);
	XSync(client2, True);
/* Set the override-redirect flag on inferiors to False. */
	attrs.override_redirect = False;
	valuemask = CWOverrideRedirect;
	for (winh = parent->firstchild, i=0; i<numchildren; winh = winh->nextsibling, i++) {
		if (!i)
			CHECK;
		if (winh_changewindowattributes(display, winh, valuemask, &attrs)) {
			report("Failed to change attributes for subwindow %d", i);
			return;
		}
	}
/* Attempt to map window. */
	XSync(display, True);
	XSync(client2, True);
	XSync(client3, True);
	XMapWindow(client2, child->window);
	XSync(client2, False);
	XSync(display, False);
	XSync(client3, False);
/* Initialize for expected events. */
	event.xany.type = EVENT;
	event.xany.window = parent->window;
	if (winh_plant(parent, &event, MASK, WINH_NOMASK)) {
		report("Could not plant events for parent");
		return;
	}
	else
		CHECK;
/* Verify that a ConfigureRequest event is delivered. */
/* Verify that no events are delivered to client3. */
	if (winh_harvest(display, (Winh *) NULL)) {
		report("Could not harvest events");
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
		XEvent	*event_return;
		XMapRequestEvent	good;

/* Verify members of event structure. */
		event_return = winh_qdel->event;
		good = event_return->xmaprequest;
		good.type = EVENT;
		good.send_event = False;
		good.display = display;
		good.parent = parent->window;
		good.window = child->window;
		if (checkevent((XEvent *) &good, event_return)) {
			report("Unexpected values in delivered event");
			FAIL;
		}
		else
			CHECK;
	}
/* Verify that no events were delivered to client3. */
	count = XPending(client3);
	if (count != 0) {
		report("Got %d events, expected %d for client3", count, 0);
		FAIL;
		return;
	}
	else
		CHECK;

	CHECKPASS(15);
>>ASSERTION def
>>#NOTE	Tested for in previous assertion.
When a xname event is generated,
then
clients not having set
.S SubstructureRedirectMask
event mask bits on the
parent of the window for which the map request was issued
are not delivered
a xname event.
>>#NOTEm >>ASSERTION
>>#NOTEm When ARTICLE xname event is generated
>>#NOTEm and a client has selected xname events on the child's parent
>>#NOTEm and the override-redirect attribute of the child window is set to
>>#NOTEm .S False ,
>>#NOTEm then
>>#NOTEm .M window
>>#NOTEm is not WINDOWTYPE.
>>#NOTEm >>ASSERTION
>>#NOTEm When a
>>#NOTEm .S MapWindow
>>#NOTEm protocol request is issued on an unmapped child window by another client
>>#NOTEm and the unmapped child window's
>>#NOTEm .M override_redirect
>>#NOTEm member is set to
>>#NOTEm .S False ,
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
>>#NOTEs When ARTICLE xname event is delivered,
>>#NOTEs then
>>#NOTEs .M window
>>#NOTEs is set to
>>#NOTEs the child window to be WINDOWTYPE.

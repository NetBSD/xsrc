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
 * $XConsortium: kympntfy.m,v 1.10 94/04/17 21:07:45 rws Exp $
 */
>>TITLE KeymapNotify CH08
>>EXTERN
#define	EVENT		KeymapNotify
#define	MASK		KeymapStateMask
>>ASSERTION Good A
When a
.S EnterNotify
event is generated,
then a xname event is generated immediately afterwards.
>>STRATEGY
Move pointer to known location.
Build and create window hierarchy.
Select for EnterNotify and KeymapNotify events on window hierarchy.
Move cursor into hierarchy window.
Get events from event queue.
Verify that EnterNotify events are delivered.
Verify that a KeymapNotify event follows each of these EnterNotify events.
Repeat.
>>CODE
int	i;
Display	*display = Dsp;
Winh	*winhs[5];

#ifdef	OTHERMASK
#undef	OTHERMASK
#endif
#define	OTHERMASK	EnterWindowMask
#ifdef	OTHEREVENT
#undef	OTHEREVENT
#endif
#define	OTHEREVENT	EnterNotify
/* Move pointer to known location. */
	if (warppointer(display, DRW(display), 0, 0) == (PointerPlace *) NULL)
		return;
	else
		CHECK;
/* Build and create window hierarchy. */
	if (winh(display, 3, WINH_MAP)) {
		report("Could not build window hierarchy");
		return;
	}
	else
		CHECK;
	/*
	 * Initialize all members to null.  Then check for null members
	 * in main for-loop as a sign of inconsistency between array
	 * size and the initialization of array members below.
	 */
	for (i=0; i<NELEM(winhs); i++)
		winhs[i] = (Winh *) NULL;
	winhs[0] = guardian->firstchild;
	winhs[1] = winhs[0]->nextsibling;
	winhs[2] = winhs[1]->nextsibling;
	winhs[3] = winhs[0]->firstchild->firstchild;
	winhs[4] = winhs[1]->firstchild->firstchild;
/* Select for EnterNotify and KeymapNotify events on window hierarchy. */
	if (winh_selectinput(display, (Winh *) NULL, MASK|OTHERMASK))
		return;
	else
		CHECK;
	for (i=0; i<NELEM(winhs); i++) {
		int	once;
		Winhe	*ptr;

		if (winhs[i] == (Winh *) NULL) {
			delete("Unexpected NULL window hierarchy member");
			return;
		}
		else
			CHECK;
/* Move cursor into hierarchy window. */
		XSync(display, True);
		XWarpPointer(display, None, winhs[i]->window, 0, 0, 0, 0, 2, 2);
		XSync(display, False);
/* Get events from event queue. */
		if (winh_plant((Winh *) NULL, (XEvent *) NULL, NoEventMask, WINH_NOMASK)) {
			/* already deleted */
			report("Could not initialize event data structures");
			return;
		}
		else
			CHECK;
		if (winh_harvest(display, (Winh *) NULL)) {
			report("Could not harvest events");
			return;
		}
		else
			CHECK;
		ptr = winh_qdel;
		if (ptr == (Winhe *) NULL) {
			report("No events received.");
			FAIL;
			continue;
		}
		else
			CHECK;
/* Verify that EnterNotify events are delivered. */
/* Verify that a KeymapNotify event follows each of these EnterNotify events. */
		for (once=1; ptr != (Winhe *) NULL; ptr = ptr->next) {
			if (once) {
				CHECK;
				once = 0;
			}
			if (ptr->event->type != OTHEREVENT) {
				report("Got %s, expected %s",
					eventname(ptr->event->type),
					eventname(OTHEREVENT));
				FAIL;
				break;
			}
			ptr = ptr->next;
			if (ptr == (Winhe *) NULL) {
				report("Missing %s event", EVENT);
				FAIL;
				break;
			}
			if (ptr->event->type != EVENT) {
				report("Got %s, expected %s",
					eventname(ptr->event->type),
					eventname(EVENT));
				FAIL;
				break;
			}
		}
/* Repeat. */
	}
	CHECKPASS(3 + (NELEM(winhs)*5));
>>ASSERTION Good A
When a
.S FocusIn
event is generated,
then a xname event is generated immediately afterwards.
>>STRATEGY
Move pointer to known location.
Build and create window hierarchy.
Select for FocusIn and KeymapNotify events on window hierarchy.
Generate FocusIn event.
Get events from event queue.
Verify that FocusIn events are delivered.
Verify that a KeymapNotify event follows each of these FocusIn events.
Repeat.
>>CODE
int	i;
Display	*display = Dsp;
Winh	*winhs[5];

#ifdef	OTHERMASK
#undef	OTHERMASK
#endif
#define	OTHERMASK	FocusChangeMask
#ifdef	OTHEREVENT
#undef	OTHEREVENT
#endif
#define	OTHEREVENT	FocusIn
#ifdef	IGNOREEVENT
#undef	IGNOREEVENT
#endif
#define	IGNOREEVENT	FocusOut

/* Move pointer to known location. */
	if (warppointer(display, DRW(display), 0, 0) == (PointerPlace *) NULL)
		return;
	else
		CHECK;
/* Build and create window hierarchy. */
	if (winh(display, 3, WINH_MAP)) {
		report("Could not build window hierarchy");
		return;
	}
	else
		CHECK;
	/*
	 * Initialize all members to null.  Then check for null members
	 * in main for-loop as a sign of inconsistency between array
	 * size and the initialization of array members below.
	 */
	for (i=0; i<NELEM(winhs); i++)
		winhs[i] = (Winh *) NULL;
	winhs[0] = guardian->firstchild;
	winhs[1] = winhs[0]->nextsibling;
	winhs[2] = winhs[1]->nextsibling;
	winhs[3] = winhs[0]->firstchild->firstchild;
	winhs[4] = winhs[1]->firstchild->firstchild;
/* Select for FocusIn and KeymapNotify events on window hierarchy. */
	if (winh_selectinput(display, (Winh *) NULL, MASK|OTHERMASK))
		return;
	else
		CHECK;
	for (i=0; i<NELEM(winhs); i++) {
		int	once;
		Winhe	*ptr;

		if (winhs[i] == (Winh *) NULL) {
			delete("Unexpected NULL window hierarchy member");
			return;
		}
		else
			CHECK;
/* Generate FocusIn event. */
		XSync(display, True);
		XSetInputFocus(display, winhs[i]->window, RevertToNone, CurrentTime);
		XSync(display, False);
/* Get events from event queue. */
		if (winh_plant((Winh *) NULL, (XEvent *) NULL, NoEventMask, WINH_NOMASK)) {
			/* already deleted */
			report("Could not initialize event data structures");
			return;
		}
		else
			CHECK;
		if (winh_harvest(display, (Winh *) NULL)) {
			report("Could not harvest events");
			return;
		}
		else
			CHECK;
		ptr = winh_qdel;
		if (ptr == (Winhe *) NULL) {
			report("No events received.");
			FAIL;
			continue;
		}
		else
			CHECK;
/* Verify that FocusIn events are delivered. */
/* Verify that a KeymapNotify event follows each of these FocusIn events. */
		for (once=1; ptr != (Winhe *) NULL; ptr = ptr->next) {
			if (once) {
				CHECK;
				once = 0;
			}
			if (ptr->event->type == IGNOREEVENT)
				continue;
			if (ptr->event->type != OTHEREVENT) {
				report("Got %s, expected %s",
					eventname(ptr->event->type),
					eventname(OTHEREVENT));
				FAIL;
				break;
			}
			ptr = ptr->next;
			if (ptr == (Winhe *) NULL) {
				report("Missing %s event", EVENT);
				FAIL;
				break;
			}
			if (ptr->event->type != EVENT) {
				report("Got %s, expected %s",
					eventname(ptr->event->type),
					eventname(EVENT));
				FAIL;
				break;
			}
		}
/* Repeat. */
	}
	CHECKPASS(3 + (NELEM(winhs)*5));
>>ASSERTION Good A
When a xname event is generated,
then
all clients having set
.S KeymapStateMask
event mask bits on the event window are delivered
a xname event.
>>STRATEGY
Move pointer to known location.
Create clients client2 and client3.
Build and create window hierarchy.
Select for KeymapNotify and EnterNotify events on eventw.
Select for KeymapNotify and EnterNotify events on eventw with client2.
Select for no events on eventw with client3.
Generate KeymapNotify event.
Initialize for expected events.
Harvest events from each clients event queue.
Verify that KeymapNotify event was received.
Verify that KeymapNotify event was received by client2.
Verify that no event was received by client3.
>>CODE
Display	*display = Dsp;
Display	*client2, *client3;
Winh	*eventw;
int	status;
XEvent	good;

#ifdef	OTHERMASK
#undef	OTHERMASK
#endif
#define	OTHERMASK	EnterWindowMask
#ifdef	OTHEREVENT
#undef	OTHEREVENT
#endif
#define	OTHEREVENT	EnterNotify

/* Move pointer to known location. */
	if (warppointer(display, DRW(display), 0, 0) == (PointerPlace *) NULL)
		return;
	else
		CHECK;
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
	if (winh(display, 1, WINH_MAP)) {
		report("Could not build window hierarchy");
		return;
	}
	else
		CHECK;
	eventw = guardian->firstchild;
/* Select for KeymapNotify and EnterNotify events on eventw. */
	/*
	 * necessary so that winh routines know which window
	 * to associate the event with
	 */
	if (winh_selectinput(display, eventw, MASK|OTHERMASK))
		return;
	else
		CHECK;
/* Select for KeymapNotify and EnterNotify events on eventw with client2. */
	/*
	 * necessary so that winh routines know which window
	 * to associate the event with
	 */
	if (winh_selectinput(client2, eventw, MASK|OTHERMASK))
		return;
	else
		CHECK;
/* Select for no events on eventw with client3. */
	if (winh_selectinput(client3, eventw, NoEventMask))
		return;
	else
		CHECK;
/* Generate KeymapNotify event. */
	XSync(display, True);
	XSync(client2, True);
	XSync(client3, True);
	XWarpPointer(display, None, eventw->window, 0, 0, 0, 0, 2, 2);
	XSync(display, False);
	XSync(client2, False);
	XSync(client3, False);
/* Initialize for expected events. */
	good.xany.type = EVENT;
	good.xany.window = eventw->window;
	good.xany.display = display;
	if (winh_plant(eventw, &good, NoEventMask, WINH_NOMASK)) {
		/* already deleted */
		report("Could not initialize event data structures (1)");
		return;
	}
	else
		CHECK;
/* Harvest events from each clients event queue. */
	if (winh_harvest(display, (Winh *) NULL)) {
		report("Could not harvest events");
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
	if (winh_ignore_event((Winh *) NULL, OTHEREVENT, WINH_NOMASK)) {
		report("Could not ignore %s events", eventname(OTHEREVENT));
		return;
	}
	else
		CHECK;
/* Verify that KeymapNotify event was received. */
/* Verify that KeymapNotify event was received by client2. */
/* Verify that no event was received by client3. */
	status = winh_weed((Winh *) NULL, -1, WINH_WEED_IDENTITY);
	if (status < 0)
		return;
	else if (status > 0) {
		report("Event delivery was not as expected");
		report("display: 0x%x, client2: 0x%x, client2: 0x%x",
			display, client2, client3);
		FAIL;
	}
	else
		CHECK;
	
	CHECKPASS(13);
>>ASSERTION def
>>#NOTE True for most events (except MappingNotify and selection stuff).
>>#NOTE	Tested for in previous assertion (i.e. client3).
When a xname event is generated,
then
clients not having set
.S KeymapStateMask
event mask bits on the event window are not delivered
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
>>#NOTEd >>ASSERTION
>>#NOTEd When ARTICLE xname is delivered,
>>#NOTEd then
>>#NOTEd .M key_vector
>>#NOTEd is set such that each bit set to one indicates that
>>#NOTEd the corresponding key is currently pressed.

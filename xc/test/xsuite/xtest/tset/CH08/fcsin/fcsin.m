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
 * $XConsortium: fcsin.m,v 1.12 94/04/17 21:07:36 rws Exp $
 */
>>TITLE FocusIn CH08
>>SET startup focusstartup
>>SET cleanup focuscleanup
>>EXTERN
#define	EVENT		FocusIn
#define	OTHEREVENT	FocusOut
#define	MASK		FocusChangeMask

static	Display	*_display_;
static	int	_detail_;
static	long	_event_mask_;
static	XEvent	good;

static	int
selectinput(start, stop, current, previous)
Winh	*start, *stop, *current, *previous;
{
#ifdef	lint
	winh_free(start);
	winh_free(stop);
	winh_free(previous);
#endif
	return(winh_selectinput(_display_, current, _event_mask_));
}

static	int
plant(start, stop, current, previous)
Winh	*start, *stop, *current, *previous;
{
#ifdef	lint
	winh_free(start);
	winh_free(stop);
	winh_free(previous);
#endif
	good.xany.window = current->window;
	return(winh_plant(current, &good, NoEventMask, WINH_NOMASK));
}

static	Bool	increasing;	/* event sequence increases as we climb */

static	int
checksequence(start, stop, current, previous)
Winh	*start, *stop, *current, *previous;
{
	Winhe	*d;
	int	current_sequence;
	int	status;
	static	int	last_sequence;

#ifdef	lint
	winh_free(start);
	winh_free(stop);
#endif
	/* look for desired event type */
	for (d = current->delivered; d != (Winhe *) NULL; d = d->next) {
		if (d->event->type == good.type) {
			current_sequence = d->sequence;
			break;
		}
	}
	if (d == (Winhe *) NULL) {
		report("%s event not delivered", eventname(good.type));
		delete("Missing event");
		return(-1);
	}
	if (previous == (Winh *) NULL)
		status = 0;	/* first call, no previous sequence value */
	else {
		/* assume sequence numbers are not the same */
		status = (current_sequence < last_sequence);
		if (increasing)
			status = (status ? 0 : 1);
		if (status)
			report("Ordering problem between 0x%x (%d) and 0x%x (%d)",
				current->window, current_sequence,
				previous->window, last_sequence);
	}
	last_sequence = current_sequence;
	return(status);
}

static	int
checkdetail(start, stop, current, previous)
Winh	*start, *stop, *current, *previous;
{
	Winhe	*d;

#ifdef	lint
	winh_free(start);
	winh_free(stop);
	winh_free(previous);
#endif
	/* look for desired event type */
	for (d = current->delivered; d != (Winhe *) NULL; d = d->next)
		if (d->event->type == good.type)
			break;
	if (d == (Winhe *) NULL) {
		report("%s event not delivered to window 0x%x",
			eventname(good.type), current->window);
		delete("Missing event");
		return(-1);
	}
	/* check detail */
	if (_detail_ != d->event->xfocus.detail) {
		report("Expected detail of %d, got %d on window 0x%x",
			_detail_, d->event->xfocus.detail, current->window);
		return(1);
	}
	return(0);
}
>>ASSERTION Good A
When a xname event is generated,
then
all clients having set
.S FocusChangeMask
event mask bits on the event window are delivered
a xname event.
>>STRATEGY
Create client.
Create clients client2 and client3.
Create window.
Move pointer to known location.
Select for xname events on window.
Select for xname events on window with client2.
Select for no events on window with client3.
Generate xname event by changing focus to window.
Verify that xname event was delivered.
Verify members in delivered xname event structure.
Verify that xname event was delivered to client2.
Verify members in delivered xname event structure.
Verify that no events were delivered to client3.
>>CODE
int	i;
Display	*display;
Display	*client2, *client3;
Window	w;
XEvent	event;
XFocusChangeEvent	good;

/* Create client. */
	if ((display = opendisplay()) == (Display *) NULL) {
		delete("Couldn't create client.");
		return;
	}
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
/* Create window. */
	w = mkwin(display, (XVisualInfo *) NULL, (struct area *) NULL, True);
/* Move pointer to known location. */
	if (warppointer(display, DRW(display), 0, 0) == (PointerPlace *) NULL) {
		report("Couldn't move pointer");
		return;
	}
	else
		CHECK;
/* Select for xname events on window. */
	XSelectInput(display, w, MASK);
/* Select for xname events on window with client2. */
	XSelectInput(client2, w, MASK);
/* Select for no events on window with client3. */
	XSelectInput(client3, w, NoEventMask);
/* Generate xname event by changing focus to window. */
	XSync(display, True);
	XSync(client2, True);
	XSync(client3, True);
	XSetInputFocus(display, w, RevertToNone, CurrentTime);
	XSync(display, False);
	XSync(client2, False);
	XSync(client3, False);
/* Verify that xname event was delivered. */
	if (XPending(display) < 1) {
		report("Expected %s event not delivered.", eventname(EVENT));
		FAIL;
		return;
	}
	else
		CHECK;
/* Verify members in delivered xname event structure. */
	XNextEvent(display, &event);
	good = event.xfocus;
	good.type = EVENT;
	good.send_event = False;
	good.display = display;
	good.window = w;
	good.mode = NotifyNormal;
	good.detail = NotifyAncestor;
	if (checkevent((XEvent*)&good, &event)) {
		report("Unexpected event structure member value(s)");
		FAIL;
	}
	else
		CHECK;
	if ((i = XPending(display)) > 0) {
		report("Expected 1 event, got %d", i+1);
		FAIL;
	}
	else
		CHECK;
/* Verify that xname event was delivered to client2. */
	if (XPending(client2) < 1) {
		report("Expected %s event not delivered to client2.", eventname(EVENT));
		FAIL;
		return;
	}
	else
		CHECK;
/* Verify members in delivered xname event structure. */
	XNextEvent(client2, &event);
	good = event.xfocus;
	good.type = EVENT;
	good.send_event = False;
	good.display = client2;
	good.window = w;
	good.mode = NotifyNormal;
	good.detail = NotifyAncestor;
	if (checkevent((XEvent*)&good, &event)) {
		report("Unexpected event structure member value(s) for client2");
		FAIL;
	}
	else
		CHECK;
	if ((i = XPending(client2)) > 0) {
		report("Expected 1 event, got %d for client2", i+1);
		FAIL;
	}
	else
		CHECK;
/* Verify that no events were delivered to client3. */
	if ((i = XPending(client3)) > 0) {
		report("Expected 0 events, got %d for client3", i);
		FAIL;
	}
	else
		CHECK;
	CHECKPASS(11);
>>ASSERTION def
>>#NOTE Tested for in pervious assertion.
When a xname event is generated,
then
clients not having set
.S FocusChangeMask
event mask bits on the event window are not delivered
a xname event.
>>#NOTEd >>ASSERTION
>>#NOTEd When the input focus changes,
>>#NOTEd then ARTICLE xname event is generated.
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
>>#NOTEs When ARTICLE xname event is generated while the pointer is not grabbed,
>>#NOTEs then
>>#NOTEs .A mode
>>#NOTEs is set to
>>#NOTEs .S NotifyNormal .
>>#NOTEs >>ASSERTION
>>#NOTEs When ARTICLE xname event is generated while the pointer is grabbed,
>>#NOTEs then
>>#NOTEs .A mode
>>#NOTEs is set to
>>#NOTEs .S NotifyWhileGrabbed .
>>#NOTEm >>ASSERTION
>>#NOTEm When ARTICLE xname event is generated
>>#NOTEm and a keyboard grab activates,
>>#NOTEm then xname events are generated as if the focus were to change from
>>#NOTEm the old focus to the grab window with
>>#NOTEm .A mode
>>#NOTEm is set to
>>#NOTEm .S NotifyGrab .
>>#NOTEm >>ASSERTION
>>#NOTEm When ARTICLE xname event is generated
>>#NOTEm and a keyboard grab deactivates,
>>#NOTEm then xname events are generated as if the focus were to change from
>>#NOTEm the grab window to the new focus with
>>#NOTEm .A mode
>>#NOTEm is set to
>>#NOTEm .S NotifyUngrab .
>>ASSERTION def
>>#NOTE	Tested for in most assertions.
All xname events are delivered after
any related
.S FocusOut
are delivered.
>>ASSERTION Good A
>>#NOTE	Am getting a detail of NotifyAncestor instead of NotifyInferior
>>#NOTE	(i.e. the test fails).
When the input focus moves from window A to window B
and window A is an inferior of window B
and the pointer is in window P,
then a xname event is generated on window B,
with
.M detail
set to
.S NotifyInferior .
>>STRATEGY
Create client.
Build window hierarchy.
Move pointer to known location.
Set window B.
Set window A to inferior of window B.
Set input focus to window A.
Select for Focus events on windows.
Generate xname event by changing focus from A to B.
Verify that the expected events were delivered.
Verify that event delivered to window B with detail set to NotifyInferior.
Verify that all xname events are delivered after all
FocusOut events.
>>CODE
Display	*display;
int	depth = 3;
Winh	*A, *B;
int	status;

/* Create client. */
	if ((display = opendisplay()) == (Display *) NULL) {
		delete("Couldn't create client.");
		return;
	}
	else
		CHECK;
/* Build window hierarchy. */
	if (winh(display, depth, WINH_MAP)) {
		report("Could not build window hierarchy");
		return;
	}
	else
		CHECK;
/* Move pointer to known location. */
	if (warppointer(display, DRW(display), 0, 0) == (PointerPlace *) NULL)
		return;
	else
		CHECK;
/* Set window B. */
	B = guardian->firstchild;
/* Set window A to inferior of window B. */
	A = B->firstchild;
/* Set input focus to window A. */
	XSetInputFocus(display, A->window, RevertToNone, CurrentTime);
/* Select for Focus events on windows. */
	_event_mask_ = MASK;
	_display_ = display;
	if (winh_climb(A, B, selectinput)) {
		report("Could not select for events");
		return;
	}
	else
		CHECK;
	good.type = EVENT;
	good.xany.display = display;
	if (winh_climb(B, B, plant)) {
		report("Could not plant events");
		return;
	}
	else
		CHECK;
/* Generate xname event by changing focus from A to B. */
	XSync(display, True);
	XSetInputFocus(display, B->window, RevertToNone, CurrentTime);
	XSync(display, False);
	if (winh_harvest(display, (Winh *) NULL)) {
		report("Could not harvest events");
		return;
	}
	else
		CHECK;
/* Verify that the expected events were delivered. */
	if (winh_ignore_event(A, OTHEREVENT, WINH_NOMASK)) {
		delete("Could not ignore %s events", eventname(OTHEREVENT));
		return;
	}
	else
		CHECK;
	status = winh_weed((Winh *) NULL, -1, WINH_WEED_IDENTITY);
	if (status < 0)
		return;
	else if (status > 0) {
		report("Event delivery was not as expected");
		FAIL;
	}
	else {
/* Verify that event delivered to window B with detail set to NotifyInferior. */
		_detail_ = NotifyInferior;
		if (winh_climb(B, B, checkdetail))
			FAIL;
		else
			CHECK;
/* Verify that all xname events are delivered after all */
/* FocusOut events. */
		status = winh_ordercheck(OTHEREVENT, EVENT);
		if (status == -1)
			return;
		else if (status)
			FAIL;
		else
			CHECK;
	}
	CHECKPASS(9);
>>ASSERTION Good A
When the input focus moves from window A to window B
and window A is an inferior of window B
and the pointer is in window P
and window P is an inferior of window B
and window P is not the same window as window A
and window P is not an inferior of window A
and window P is not an ancestor of window A,
then, after a xname event is generated on window B
with
.M detail
set to
.S NotifyInferior ,
a xname event is generated on each window
below window B,
down to and including window P,
with
.M detail
set to
.S NotifyPointer .
>>STRATEGY
Create client.
Build window hierarchy.
Move pointer to known location.
Set window B.
Set window A to inferior of window B.
Set window P to inferior of sibling of window A.
Move pointer to window P.
Set input focus to window A.
Select for Focus events on windows.
Generate xname event by changing focus from A to B.
Verify that the expected events were delivered.
Verify that event delivered to window B with detail set to NotifyInferior.
Verify that event delivered below window B, down to and including
window P, with detail set to NotifyPointer.
Verify order of xname event delivery.
Verify that all xname events are delivered after all
FocusOut events.
>>CODE
Display	*display;
int	depth = 4;
Winh	*A, *B, *P, *Pancestor;
int	status;

/* Create client. */
	if ((display = opendisplay()) == (Display *) NULL) {
		delete("Couldn't create client.");
		return;
	}
	else
		CHECK;
/* Build window hierarchy. */
	if (winh(display, depth, WINH_MAP)) {
		report("Could not build window hierarchy");
		return;
	}
	else
		CHECK;
/* Move pointer to known location. */
	if (warppointer(display, DRW(display), 0, 0) == (PointerPlace *) NULL)
		return;
	else
		CHECK;
/* Set window B. */
	B = guardian->firstchild;
/* Set window A to inferior of window B. */
	A = B->firstchild;
/* Set window P to inferior of sibling of window A. */
	Pancestor = B->firstchild->nextsibling;
	P = Pancestor->firstchild->firstchild;
/* Move pointer to window P. */
	XWarpPointer(display, None, P->window, 0, 0, 0, 0, 0, 0);
/* Set input focus to window A. */
	XSetInputFocus(display, A->window, RevertToNone, CurrentTime);
/* Select for Focus events on windows. */
	_event_mask_ = MASK;
	_display_ = display;
	if (winh_climb(P, Pancestor, selectinput)) {
		report("Could not select for events");
		return;
	}
	else
		CHECK;
	if (winh_climb(A, B, selectinput)) {
		report("Could not select for events between A and B");
		return;
	}
	else
		CHECK;
	good.type = EVENT;
	good.xany.display = display;
	if (winh_climb(P, B, plant)) {
		report("Could not plant events");
		return;
	}
	else
		CHECK;
/* Generate xname event by changing focus from A to B. */
	XSync(display, True);
	XSetInputFocus(display, B->window, RevertToNone, CurrentTime);
	XSync(display, False);
	if (winh_harvest(display, (Winh *) NULL)) {
		report("Could not harvest events");
		return;
	}
	else
		CHECK;
/* Verify that the expected events were delivered. */
	if (winh_ignore_event((Winh *) NULL, OTHEREVENT, WINH_NOMASK)) {
		delete("Could not ignore %s events", eventname(OTHEREVENT));
		return;
	}
	else
		CHECK;
	status = winh_weed((Winh *) NULL, -1, WINH_WEED_IDENTITY);
	if (status < 0)
		return;
	else if (status > 0) {
		report("Event delivery was not as expected");
		FAIL;
	}
	else {
/* Verify that event delivered to window B with detail set to NotifyInferior. */
		_detail_ = NotifyInferior;
		if (winh_climb(B, B, checkdetail))
			FAIL;
		else
			CHECK;
/* Verify that event delivered below window B, down to and including */
/* window P, with detail set to NotifyPointer. */
		_detail_ = NotifyPointer;
		if (winh_climb(P, Pancestor, checkdetail))
			FAIL;
		else
			CHECK;
/* Verify order of xname event delivery. */
		increasing = True;
		if (winh_climb(P, B, checksequence))
			FAIL;
		else
			CHECK;
/* Verify that all xname events are delivered after all */
/* FocusOut events. */
		status = winh_ordercheck(OTHEREVENT, EVENT);
		if (status == -1)
			return;
		else if (status)
			FAIL;
		else
			CHECK;
	}
	CHECKPASS(12);
>>ASSERTION Good A
When the input focus moves from window A to window B
and window B is an inferior of window A
and the pointer is in window P,
then a xname event is generated on each window
between window A and window B,
exclusive,
with
.M detail
set to
.S NotifyVirtual
and then on window B
with
.M detail
set to
.S NotifyAncestor .
>>STRATEGY
Create client.
Build window hierarchy.
Move pointer to known location.
Set window A.
Set window B to inferior of window A.
Set input focus to window A.
Select for Focus events on windows.
Generate xname event by changing focus from A to B.
Verify that the expected events were delivered.
Verify that event delivered to windows between window A and window B, exclusive,
with detail set to NotifyVirtual.
Verify that event delivered to window B with detail set to NotifyAncestor.
Verify order of xname event delivery.
Verify that all xname events are delivered after all
FocusOut events.
>>CODE
Display	*display;
int	depth = 4;
Winh	*A, *B;
int	status;

/* Create client. */
	if ((display = opendisplay()) == (Display *) NULL) {
		delete("Couldn't create client.");
		return;
	}
	else
		CHECK;
/* Build window hierarchy. */
	if (winh(display, depth, WINH_MAP)) {
		report("Could not build window hierarchy");
		return;
	}
	else
		CHECK;
/* Move pointer to known location. */
	if (warppointer(display, DRW(display), 0, 0) == (PointerPlace *) NULL)
		return;
	else
		CHECK;
/* Set window A. */
	A = guardian->firstchild;
/* Set window B to inferior of window A. */
	B = A->firstchild->firstchild->firstchild;
/* Set input focus to window A. */
	XSetInputFocus(display, A->window, RevertToNone, CurrentTime);
/* Select for Focus events on windows. */
	_display_ = display;
#if 0
	_event_mask_ = MASK;
	if (winh_climb(B, A->firstchild, selectinput)) {
		report("Could not select for events");
		return;
	}
	else
		CHECK;
#endif
	if (winh_selectinput(display, (Winh *) NULL, MASK)) {
		report("Could not select for events");
		return;
	}
	else
		CHECK;
	good.type = EVENT;
	good.xany.display = display;
	if (winh_climb(B, A->firstchild, plant)) {
		report("Could not plant events");
		return;
	}
	else
		CHECK;
/* Generate xname event by changing focus from A to B. */
	XSync(display, True);
	XSetInputFocus(display, B->window, RevertToNone, CurrentTime);
	XSync(display, False);
	if (winh_harvest(display, (Winh *) NULL)) {
		report("Could not harvest events");
		return;
	}
	else
		CHECK;
/* Verify that the expected events were delivered. */
	if (winh_ignore_event((Winh *) NULL, OTHEREVENT, WINH_NOMASK)) {
		delete("Could not ignore %s events", eventname(OTHEREVENT));
		return;
	}
	else
		CHECK;
	status = winh_weed((Winh *) NULL, -1, WINH_WEED_IDENTITY);
	if (status < 0)
		return;
	else if (status > 0) {
		report("Event delivery was not as expected");
		FAIL;
	}
	else {
/* Verify that event delivered to windows between window A and window B, exclusive, */
/* with detail set to NotifyVirtual. */
		_detail_ = NotifyVirtual;
		if (winh_climb(B->parent, A->firstchild, checkdetail))
			FAIL;
		else
			CHECK;
/* Verify that event delivered to window B with detail set to NotifyAncestor. */
		_detail_ = NotifyAncestor;
		if (winh_climb(B, B, checkdetail))
			FAIL;
		else
			CHECK;
/* Verify order of xname event delivery. */
		increasing = True;
		if (winh_climb(B, A->firstchild, checksequence))
			FAIL;
		else
			CHECK;
/* Verify that all xname events are delivered after all */
/* FocusOut events. */
		status = winh_ordercheck(OTHEREVENT, EVENT);
		if (status == -1)
			return;
		else if (status)
			FAIL;
		else
			CHECK;
	}
	CHECKPASS(11);
>>ASSERTION Good A
When the input focus moves from window A to window B
and window C is their least common ancestor
and the pointer is in window P,
then a xname event is generated on
each window between C and B, exclusive,
with
.M detail
set to
.S NotifyNonlinearVirtual
and then on window B
with
.M detail
set to
.S NotifyNonlinear .
>>STRATEGY
Create client.
Build window hierarchy.
Move pointer to known location.
Set window C.
Set window B to inferior of window C.
Set window A to inferior of window C.
Set input focus to window A.
Select for Focus events on windows.
Generate xname event by changing focus from A to B.
Verify that the expected events were delivered.
Verify that event delivered to windows between window C and window B, exclusive,
with detail set to NotifyNonlinearVirtual.
Verify that event delivered to window B with detail set to NotifyNonlinear.
Verify order of xname event delivery.
Verify that all xname events are delivered after all
FocusOut events.
>>CODE
Display	*display;
int	depth = 4;
Winh	*A, *B, *C;
int	status;

/* Create client. */
	if ((display = opendisplay()) == (Display *) NULL) {
		delete("Couldn't create client.");
		return;
	}
	else
		CHECK;
/* Build window hierarchy. */
	if (winh(display, depth, WINH_MAP)) {
		report("Could not build window hierarchy");
		return;
	}
	else
		CHECK;
/* Move pointer to known location. */
	if (warppointer(display, DRW(display), 0, 0) == (PointerPlace *) NULL)
		return;
	else
		CHECK;
/* Set window C. */
	C = guardian->firstchild;
/* Set window B to inferior of window C. */
	B = C->firstchild->firstchild->firstchild;
/* Set window A to inferior of window C. */
	A = C->firstchild->nextsibling->firstchild;
/* Set input focus to window A. */
	XSetInputFocus(display, A->window, RevertToNone, CurrentTime);
/* Select for Focus events on windows. */
	_event_mask_ = MASK;
	_display_ = display;
	if (winh_climb(B, C->firstchild, selectinput)) {
		report("Could not select for events");
		return;
	}
	else
		CHECK;
	if (winh_climb(A, C, selectinput)) {
		report("Could not select for events between A and C");
		return;
	}
	else
		CHECK;
	good.type = EVENT;
	good.xany.display = display;
	if (winh_climb(B, C->firstchild, plant)) {
		report("Could not plant events");
		return;
	}
	else
		CHECK;
/* Generate xname event by changing focus from A to B. */
	XSync(display, True);
	XSetInputFocus(display, B->window, RevertToNone, CurrentTime);
	XSync(display, False);
	if (winh_harvest(display, (Winh *) NULL)) {
		report("Could not harvest events");
		return;
	}
	else
		CHECK;
/* Verify that the expected events were delivered. */
	if (winh_ignore_event((Winh *) NULL, OTHEREVENT, WINH_NOMASK)) {
		delete("Could not ignore %s events", eventname(OTHEREVENT));
		return;
	}
	else
		CHECK;
	status = winh_weed((Winh *) NULL, -1, WINH_WEED_IDENTITY);
	if (status < 0)
		return;
	else if (status > 0) {
		report("Event delivery was not as expected");
		FAIL;
	}
	else {
/* Verify that event delivered to windows between window C and window B, exclusive, */
/* with detail set to NotifyNonlinearVirtual. */
		_detail_ = NotifyNonlinearVirtual;
		if (winh_climb(B->parent, C->firstchild, checkdetail))
			FAIL;
		else
			CHECK;
/* Verify that event delivered to window B with detail set to NotifyNonlinear. */
		_detail_ = NotifyNonlinear;
		if (winh_climb(B, B, checkdetail))
			FAIL;
		else
			CHECK;
/* Verify order of xname event delivery. */
		increasing = True;
		if (winh_climb(B, C->firstchild, checksequence))
			FAIL;
		else
			CHECK;
/* Verify that all xname events are delivered after all */
/* FocusOut events. */
		status = winh_ordercheck(OTHEREVENT, EVENT);
		if (status == -1)
			return;
		else if (status)
			FAIL;
		else
			CHECK;
	}
	CHECKPASS(12);
>>ASSERTION Good A
When the input focus moves from window A to window B
and window C is their least common ancestor
and the pointer is in window P
and window P is an inferior of window B,
then, after the related xname events are generated
with
.M detail
set to
.S NotifyNonlinearVirtual
and
.S NotifyNonlinear ,
a xname event is generated on
each window below window B down to and including window P,
with
.M detail
set to
.S NotifyPointer .
>>STRATEGY
Create client.
Build window hierarchy.
Move pointer to known location.
Set window C.
Set window B to inferior of window C.
Set window P to inferior of window B.
Set window A to inferior of window C.
Move pointer to window P.
Set input focus to window A.
Select for Focus events on windows.
Generate xname event by changing focus from A to B.
Verify that the expected events were delivered.
Verify that event delivered to windows between window C and window B, exclusive,
with detail set to NotifyNonlinearVirtual.
Verify that event delivered to window B with detail set to NotifyNonlinear.
Verify that events were delivered to windows below window B down to and
including window P, exclusive,
with detail set to NotifyPointer.
Verify order of xname event delivery.
Verify that all xname events are delivered after all
FocusOut events.
>>CODE
Display	*display;
int	depth = 5;
Winh	*A, *B, *C, *P;
int	status;

/* Create client. */
	if ((display = opendisplay()) == (Display *) NULL) {
		delete("Couldn't create client.");
		return;
	}
	else
		CHECK;
/* Build window hierarchy. */
	if (winh(display, depth, WINH_MAP)) {
		report("Could not build window hierarchy");
		return;
	}
	else
		CHECK;
/* Move pointer to known location. */
	if (warppointer(display, DRW(display), 0, 0) == (PointerPlace *) NULL)
		return;
	else
		CHECK;
/* Set window C. */
	C = guardian->firstchild;
/* Set window B to inferior of window C. */
	B = C->firstchild->firstchild;
/* Set window P to inferior of window B. */
	P = B->firstchild->firstchild;
/* Set window A to inferior of window C. */
	A = C->firstchild->nextsibling->firstchild;
/* Move pointer to window P. */
	XWarpPointer(display, None, P->window, 0, 0, 0, 0, 0, 0);
/* Set input focus to window A. */
	XSetInputFocus(display, A->window, RevertToNone, CurrentTime);
/* Select for Focus events on windows. */
	_event_mask_ = MASK;
	_display_ = display;
	if (winh_climb(B, C->firstchild, selectinput)) {
		report("Could not select for events");
		return;
	}
	else
		CHECK;
	if (winh_climb(A, C, selectinput)) {
		report("Could not select for events between A and C");
		return;
	}
	else
		CHECK;
	if (winh_climb(P, B, selectinput)) {
		report("Could not select for events between P and B");
		return;
	}
	else
		CHECK;
	good.type = EVENT;
	good.xany.display = display;
	if (winh_climb(B, C->firstchild, plant)) {
		report("Could not plant events");
		return;
	}
	else
		CHECK;
	if (winh_climb(P, B->firstchild, plant)) {
		report("Could not plant events below B");
		return;
	}
	else
		CHECK;
/* Generate xname event by changing focus from A to B. */
	XSync(display, True);
	XSetInputFocus(display, B->window, RevertToNone, CurrentTime);
	XSync(display, False);
	if (winh_harvest(display, (Winh *) NULL)) {
		report("Could not harvest events");
		return;
	}
	else
		CHECK;
/* Verify that the expected events were delivered. */
	if (winh_ignore_event((Winh *) NULL, OTHEREVENT, WINH_NOMASK)) {
		delete("Could not ignore %s events", eventname(OTHEREVENT));
		return;
	}
	else
		CHECK;
	status = winh_weed((Winh *) NULL, -1, WINH_WEED_IDENTITY);
	if (status < 0)
		return;
	else if (status > 0) {
		report("Event delivery was not as expected");
		FAIL;
	}
	else {
/* Verify that event delivered to windows between window C and window B, exclusive, */
/* with detail set to NotifyNonlinearVirtual. */
		_detail_ = NotifyNonlinearVirtual;
		if (winh_climb(B->parent, C->firstchild, checkdetail))
			FAIL;
		else
			CHECK;
/* Verify that event delivered to window B with detail set to NotifyNonlinear. */
		_detail_ = NotifyNonlinear;
		if (winh_climb(B, B, checkdetail))
			FAIL;
		else
			CHECK;
/* Verify that events were delivered to windows below window B down to and */
/* including window P, exclusive, */
/* with detail set to NotifyPointer. */
		_detail_ = NotifyPointer;
		if (winh_climb(P, B->firstchild, checkdetail))
			FAIL;
		else
			CHECK;
/* Verify order of xname event delivery. */
		increasing = True;
		if (winh_climb(B, C->firstchild, checksequence))
			FAIL;
		else
			CHECK;
		increasing = True;
		if (winh_climb(P, B->firstchild, checksequence))
			FAIL;
		else
			CHECK;
/* Verify that all xname events are delivered after all */
/* FocusOut events. */
		status = winh_ordercheck(OTHEREVENT, EVENT);
		if (status == -1)
			return;
		else if (status)
			FAIL;
		else
			CHECK;
	}
	CHECKPASS(16);
>>ASSERTION Good C
If the implementation supports multiple screens:
When the input focus moves from window A to window B
and window A and window B are not on the same screens
and the pointer is in window P
and window B is not a root window,
then a xname event is generated on
each window from window B's root down to but not including window B,
with
.M detail
set to
.S NotifyNonlinearVirtual .
>>STRATEGY
Check to see if multiple screens are supported.
Create client.
Build window hierarchy on all supported screens.
Move pointer to known location.
Set window A.
Set window B to an inferior of the root window on a different screen than A.
Set input focus to window A.
Select for Focus events on windows.
Generate xname event by changing focus from A to B.
Verify that the expected events were delivered.
Verify that event delivered to each window from window B's root down
to but not including window B
with detail set to NotifyNonlinearVirtual.
Verify order of xname event delivery.
Verify that all xname events are delivered after all
FocusOut events.
>>CODE
Display	*display;
int	depth = 4;
Winh	*A, *B, *Broot;
int	status;

/* Check to see if multiple screens are supported. */
	if (config.alt_screen == -1) {
		unsupported("Multiple screens not supported.");
		return;
	}
	else
		CHECK;
/* Create client. */
	if ((display = opendisplay()) == (Display *) NULL) {
		delete("Couldn't create client.");
		return;
	}
	else
		CHECK;
/* Build window hierarchy on all supported screens. */
	if (winh(display, depth, WINH_MAP|WINH_BOTH_SCREENS)) {
		report("Could not build window hierarchy");
		return;
	}
	else
		CHECK;
/* Move pointer to known location. */
	if (warppointer(display, DRW(display), 0, 0) == (PointerPlace *) NULL)
		return;
	else
		CHECK;
/* Set window A. */
	A = guardian->firstchild;
/* Set window B to an inferior of the root window on a different screen than A. */
	Broot = guardian->nextsibling;
	B = Broot->firstchild->firstchild->firstchild;
/* Set input focus to window A. */
	XSetInputFocus(display, A->window, RevertToNone, CurrentTime);
/* Select for Focus events on windows. */
	_event_mask_ = MASK;
	_display_ = display;
	if (winh_climb(B->parent, Broot, selectinput)) {
		report("Could not select for events");
		return;
	}
	else
		CHECK;
	if (winh_climb(A, A, selectinput)) {
		report("Could not select for events on window A");
		return;
	}
	else
		CHECK;
	good.type = EVENT;
	good.xany.display = display;
	if (winh_climb(B->parent, Broot, plant)) {
		report("Could not plant events");
		return;
	}
	else
		CHECK;
/* Generate xname event by changing focus from A to B. */
	XSync(display, True);
	XSetInputFocus(display, B->window, RevertToNone, CurrentTime);
	XSync(display, False);
	if (winh_harvest(display, (Winh *) NULL)) {
		report("Could not harvest events");
		return;
	}
	else
		CHECK;
/* Verify that the expected events were delivered. */
	if (winh_ignore_event((Winh *) NULL, OTHEREVENT, WINH_NOMASK)) {
		delete("Could not ignore %s events", eventname(OTHEREVENT));
		return;
	}
	else
		CHECK;
	status = winh_weed((Winh *) NULL, -1, WINH_WEED_IDENTITY);
	if (status < 0)
		return;
	else if (status > 0) {
		report("Event delivery was not as expected");
		FAIL;
	}
	else {
/* Verify that event delivered to each window from window B's root down */
/* to but not including window B */
/* with detail set to NotifyNonlinearVirtual. */
		_detail_ = NotifyNonlinearVirtual;
		if (winh_climb(B->parent, Broot, checkdetail))
			FAIL;
		else
			CHECK;
/* Verify order of xname event delivery. */
		increasing = True;
		if (winh_climb(B->parent, Broot, checksequence))
			FAIL;
		else
			CHECK;
/* Verify that all xname events are delivered after all */
/* FocusOut events. */
		status = winh_ordercheck(OTHEREVENT, EVENT);
		if (status == -1)
			return;
		else if (status)
			FAIL;
		else
			CHECK;
	}
	CHECKPASS(12);
>>ASSERTION Good C
If the implementation supports multiple screens:
When the input focus moves from window A to window B
and window A and window B are not on the same screens
and the pointer is in window P,
then,
after any related xname events are generated with
.M detail
set to
.S NotifyNonlinearVirtual ,
a xname event is generated on window B
with
.M detail
set to
.S NotifyNonlinear .
>>STRATEGY
Check to see if multiple screens are supported.
Create client.
Build window hierarchy on all supported screens.
Move pointer to known location.
Set window A.
Set window B to an inferior of the root window on a different screen than A.
Set input focus to window A.
Select for Focus events on windows.
Generate xname event by changing focus from A to B.
Verify that the expected events were delivered.
Verify that event delivered to each window from window B's root down
to but not including window B
with detail set to NotifyNonlinearVirtual.
Verify that event delivered on window B with detail
set to NotifyNonlinear.
Verify order of xname event delivery.
Verify that all xname events are delivered after all
FocusOut events.
>>CODE
Display	*display;
int	depth = 4;
Winh	*A, *B, *Broot;
int	status;

/* Check to see if multiple screens are supported. */
	if (config.alt_screen == -1) {
		unsupported("Multiple screens not supported.");
		return;
	}
	else
		CHECK;
/* Create client. */
	if ((display = opendisplay()) == (Display *) NULL) {
		delete("Couldn't create client.");
		return;
	}
	else
		CHECK;
/* Build window hierarchy on all supported screens. */
	if (winh(display, depth, WINH_MAP|WINH_BOTH_SCREENS)) {
		report("Could not build window hierarchy");
		return;
	}
	else
		CHECK;
/* Move pointer to known location. */
	if (warppointer(display, DRW(display), 0, 0) == (PointerPlace *) NULL)
		return;
	else
		CHECK;
/* Set window A. */
	A = guardian->firstchild;
/* Set window B to an inferior of the root window on a different screen than A. */
	Broot = guardian->nextsibling;
	B = Broot->firstchild->firstchild->firstchild;
/* Set input focus to window A. */
	XSetInputFocus(display, A->window, RevertToNone, CurrentTime);
/* Select for Focus events on windows. */
	_event_mask_ = MASK;
	_display_ = display;
	if (winh_climb(B, Broot, selectinput)) {
		report("Could not select for events");
		return;
	}
	else
		CHECK;
	if (winh_climb(A, A, selectinput)) {
		report("Could not select for events on window A");
		return;
	}
	else
		CHECK;
	good.type = EVENT;
	good.xany.display = display;
	if (winh_climb(B, Broot, plant)) {
		report("Could not plant events");
		return;
	}
	else
		CHECK;
/* Generate xname event by changing focus from A to B. */
	XSync(display, True);
	XSetInputFocus(display, B->window, RevertToNone, CurrentTime);
	XSync(display, False);
	if (winh_harvest(display, (Winh *) NULL)) {
		report("Could not harvest events");
		return;
	}
	else
		CHECK;
/* Verify that the expected events were delivered. */
	if (winh_ignore_event((Winh *) NULL, OTHEREVENT, WINH_NOMASK)) {
		delete("Could not ignore %s events", eventname(OTHEREVENT));
		return;
	}
	else
		CHECK;
	status = winh_weed((Winh *) NULL, -1, WINH_WEED_IDENTITY);
	if (status < 0)
		return;
	else if (status > 0) {
		report("Event delivery was not as expected");
		FAIL;
	}
	else {
/* Verify that event delivered to each window from window B's root down */
/* to but not including window B */
/* with detail set to NotifyNonlinearVirtual. */
		_detail_ = NotifyNonlinearVirtual;
		if (winh_climb(B->parent, Broot, checkdetail))
			FAIL;
		else
			CHECK;
/* Verify that event delivered on window B with detail */
/* set to NotifyNonlinear. */
		_detail_ = NotifyNonlinear;
		if (winh_climb(B, B, checkdetail))
			FAIL;
		else
			CHECK;
/* Verify order of xname event delivery. */
		increasing = True;
		if (winh_climb(B, Broot, checksequence))
			FAIL;
		else
			CHECK;
/* Verify that all xname events are delivered after all */
/* FocusOut events. */
		status = winh_ordercheck(OTHEREVENT, EVENT);
		if (status == -1)
			return;
		else if (status)
			FAIL;
		else
			CHECK;
	}
	CHECKPASS(13);
>>ASSERTION Good C
If the implementation supports multiple screens:
When the input focus moves from window A to window B
and window A and window B are not on the same screens
and the pointer is in window P
and window P is an inferior of window B,
then a xname event is generated on window B
with
.M detail
set to
.S NotifyNonlinear .
>>STRATEGY
Check to see if multiple screens are supported.
Create client.
Build window hierarchy on all supported screens.
Move pointer to known location.
Set window A.
Set window B.
Set P to inferior of window B.
Set input focus to window A.
Move pointer to window P.
Select for Focus events on windows.
Generate xname event by changing focus from A to B.
Verify that the expected events were delivered.
Verify that event delivered on window B with detail
set to NotifyNonlinear.
Verify that all xname events are delivered after all
FocusOut events.
>>CODE
Display	*display;
int	depth = 2;
Winh	*A, *B, *P;
int	status;

/* Check to see if multiple screens are supported. */
	if (config.alt_screen == -1) {
		unsupported("Multiple screens not supported.");
		return;
	}
	else
		CHECK;
/* Create client. */
	if ((display = opendisplay()) == (Display *) NULL) {
		delete("Couldn't create client.");
		return;
	}
	else
		CHECK;
/* Build window hierarchy on all supported screens. */
	if (winh(display, depth, WINH_MAP|WINH_BOTH_SCREENS)) {
		report("Could not build window hierarchy");
		return;
	}
	else
		CHECK;
/* Move pointer to known location. */
	if (warppointer(display, DRW(display), 0, 0) == (PointerPlace *) NULL)
		return;
	else
		CHECK;
/* Set window A. */
	A = guardian->firstchild;
/* Set window B. */
	B = guardian->nextsibling->firstchild;
/* Set P to inferior of window B. */
	P = guardian->nextsibling->firstchild;
/* Set input focus to window A. */
	XSetInputFocus(display, A->window, RevertToNone, CurrentTime);
/* Move pointer to window P. */
	XWarpPointer(display, None, P->window, 0, 0, 0, 0, 0, 0);
/* Select for Focus events on windows. */
	_event_mask_ = MASK;
	_display_ = display;
	if (winh_climb(B, B, selectinput)) {
		report("Could not select for events");
		return;
	}
	else
		CHECK;
	if (winh_climb(A, A, selectinput)) {
		report("Could not select for events on window A");
		return;
	}
	else
		CHECK;
	good.type = EVENT;
	good.xany.display = display;
	if (winh_climb(B, B, plant)) {
		report("Could not plant events");
		return;
	}
	else
		CHECK;
/* Generate xname event by changing focus from A to B. */
	XSync(display, True);
	XSetInputFocus(display, B->window, RevertToNone, CurrentTime);
	XSync(display, False);
	if (winh_harvest(display, (Winh *) NULL)) {
		report("Could not harvest events");
		return;
	}
	else
		CHECK;
/* Verify that the expected events were delivered. */
	if (winh_ignore_event((Winh *) NULL, OTHEREVENT, WINH_NOMASK)) {
		delete("Could not ignore %s events", eventname(OTHEREVENT));
		return;
	}
	else
		CHECK;
	status = winh_weed((Winh *) NULL, -1, WINH_WEED_IDENTITY);
	if (status < 0)
		return;
	else if (status > 0) {
		report("Event delivery was not as expected");
		FAIL;
	}
	else {
/* Verify that event delivered on window B with detail */
/* set to NotifyNonlinear. */
		_detail_ = NotifyNonlinear;
		if (winh_climb(B, B, checkdetail))
			FAIL;
		else
			CHECK;
/* Verify that all xname events are delivered after all */
/* FocusOut events. */
		status = winh_ordercheck(OTHEREVENT, EVENT);
		if (status == -1)
			return;
		else if (status)
			FAIL;
		else
			CHECK;
	}
	CHECKPASS(11);
>>ASSERTION Good C
If the implementation supports multiple screens:
When the input focus moves from window A to window B
and window A and window B are not on the same screens
and the pointer is in window P
and window P is an inferior of window B,
then, after the related xname events are generated
with
.M detail
set to
.S NotifyNonlinearVirtual
and
.S NotifyNonlinear ,
a xname event is generated on
each window below window B down to and including window P
with
.M detail
set to
.S NotifyPointer .
>>STRATEGY
Check to see if multiple screens are supported.
Create client.
Build window hierarchy on all supported screens.
Move pointer to known location.
Set window A.
Set window B to an inferior of the root window on a different screen than A.
Set window P to an inferior of B.
Set input focus to window A.
Move pointer to window P.
Select for Focus events on windows.
Generate xname event by changing focus from A to B.
Verify that the expected events were delivered.
Verify that event delivered to each window from window B's root down
to but not including window B
with detail set to NotifyNonlinearVirtual.
Verify that event delivered on window B with detail
set to NotifyNonlinear.
Verify that event delivered to each window from window B down
to and including window P
with detail set to NotifyPointer.
Verify order of xname event delivery.
Verify that all xname events are delivered after all
FocusOut events.
>>CODE
Display	*display;
int	depth = 5;
Winh	*A, *B, *Broot, *P;
int	status;

/* Check to see if multiple screens are supported. */
	if (config.alt_screen == -1) {
		unsupported("Multiple screens not supported.");
		return;
	}
	else
		CHECK;
/* Create client. */
	if ((display = opendisplay()) == (Display *) NULL) {
		delete("Couldn't create client.");
		return;
	}
	else
		CHECK;
/* Build window hierarchy on all supported screens. */
	if (winh(display, depth, WINH_MAP|WINH_BOTH_SCREENS)) {
		report("Could not build window hierarchy");
		return;
	}
	else
		CHECK;
/* Move pointer to known location. */
	if (warppointer(display, DRW(display), 0, 0) == (PointerPlace *) NULL)
		return;
	else
		CHECK;
/* Set window A. */
	A = guardian->firstchild;
/* Set window B to an inferior of the root window on a different screen than A. */
	Broot = guardian->nextsibling;
	B = Broot->firstchild->firstchild;
/* Set window P to an inferior of B. */
	P = B->firstchild->firstchild;
/* Set input focus to window A. */
	XSetInputFocus(display, A->window, RevertToNone, CurrentTime);
/* Move pointer to window P. */
	XWarpPointer(display, None, P->window, 0, 0, 0, 0, 0, 0);
/* Select for Focus events on windows. */
	_event_mask_ = MASK;
	_display_ = display;
	if (winh_climb(P, Broot, selectinput)) {
		report("Could not select for events");
		return;
	}
	else
		CHECK;
	if (winh_climb(A, A, selectinput)) {
		report("Could not select for events on window A");
		return;
	}
	else
		CHECK;
	good.type = EVENT;
	good.xany.display = display;
	if (winh_climb(P, Broot, plant)) {
		report("Could not plant events");
		return;
	}
	else
		CHECK;
/* Generate xname event by changing focus from A to B. */
	XSync(display, True);
	XSetInputFocus(display, B->window, RevertToNone, CurrentTime);
	XSync(display, False);
	if (winh_harvest(display, (Winh *) NULL)) {
		report("Could not harvest events");
		return;
	}
	else
		CHECK;
/* Verify that the expected events were delivered. */
	if (winh_ignore_event((Winh *) NULL, OTHEREVENT, WINH_NOMASK)) {
		delete("Could not ignore %s events", eventname(OTHEREVENT));
		return;
	}
	else
		CHECK;
	status = winh_weed((Winh *) NULL, -1, WINH_WEED_IDENTITY);
	if (status < 0)
		return;
	else if (status > 0) {
		report("Event delivery was not as expected");
		FAIL;
	}
	else {
/* Verify that event delivered to each window from window B's root down */
/* to but not including window B */
/* with detail set to NotifyNonlinearVirtual. */
		_detail_ = NotifyNonlinearVirtual;
		if (winh_climb(B->parent, Broot, checkdetail))
			FAIL;
		else
			CHECK;
/* Verify that event delivered on window B with detail */
/* set to NotifyNonlinear. */
		_detail_ = NotifyNonlinear;
		if (winh_climb(B, B, checkdetail))
			FAIL;
		else
			CHECK;
/* Verify that event delivered to each window from window B down */
/* to and including window P */
/* with detail set to NotifyPointer. */
		_detail_ = NotifyPointer;
		if (winh_climb(P, B->firstchild, checkdetail))
			FAIL;
		else
			CHECK;
/* Verify order of xname event delivery. */
		increasing = True;
		if (winh_climb(P, Broot, checksequence))
			FAIL;
		else
			CHECK;
/* Verify that all xname events are delivered after all */
/* FocusOut events. */
		status = winh_ordercheck(OTHEREVENT, EVENT);
		if (status == -1)
			return;
		else if (status)
			FAIL;
		else
			CHECK;
	}
	CHECKPASS(14);
>>ASSERTION def
>>#NOTE	Tested for in next test.
When the focus moves from window A to
.S PointerRoot
(events sent to the window under the pointer)
and the pointer is in window P,
then a xname event is generated on
the root window of all screens,
with
.M detail
set to
.S NotifyPointerRoot .
>>ASSERTION Good A
When the focus moves from window A to
.S PointerRoot
(events sent to the window under the pointer)
and the pointer is in window P,
then, after the related xname events are generated
with
.M detail
set to
.S NotifyPointerRoot ,
a xname event is generated on
each window from window P's root down to and including window P,
with
.M detail
set to
.S NotifyPointer .
>>STRATEGY
Create client.
Build window hierarchy on all supported screens.
Set window A.
Set window P.
Move pointer to window P.
Set input focus to window A.
Select for Focus events on windows.
Generate xname event by changing focus from A to PointerRoot.
Verify that the expected events were delivered.
Verify that event delivered to the root window of all screens
with detail set to NotifyPointerRoot.
Verify that event generated on each window from window P's root down
to and including window P with detail set to NotifyPointer.
Verify that these events occurred in the correct order.
Verify that the NotifyPointerRoot events were delivered before
NotifyPointer events.
Verify that all xname events are delivered after all
FocusOut events.
>>CODE
Display	*display;
int	depth = 4;
Winh	*A, *P, *Proot, *ptr;
Winhe	*winhe;
int	status;
int	high;

/* Create client. */
	if ((display = opendisplay()) == (Display *) NULL) {
		delete("Couldn't create client.");
		return;
	}
	else
		CHECK;
/* Build window hierarchy on all supported screens. */
	if (winh(display, depth, WINH_MAP|WINH_BOTH_SCREENS)) {
		report("Could not build window hierarchy");
		return;
	}
	else
		CHECK;
/* Set window A. */
	A = guardian->firstchild;
/* Set window P. */
	Proot = guardian;
	P = Proot->firstchild->firstchild;
/* Move pointer to window P. */
	if (warppointer(display, P->window, 0, 0) == (PointerPlace *) NULL)
		return;
	else
		CHECK;
/* Set input focus to window A. */
	XSetInputFocus(display, A->window, RevertToNone, CurrentTime);
/* Select for Focus events on windows. */
	if (winh_selectinput(display, (Winh *) NULL, MASK)) {
		report("Could not select for events");
		return;
	}
	else
		CHECK;
	good.type = EVENT;
	good.xany.display = display;
	/* root window of all screens */
	for (ptr = guardian; ptr != (Winh *) NULL; ptr = ptr->nextsibling) {
		if (ptr == guardian)
			CHECK;
		good.xany.window = ptr->window;
		if (winh_plant(ptr, &good, MASK, WINH_NOMASK)) {
			report("Could not plant events");
			return;
		}
	}
	/* from P's root to P */
	if (winh_climb(P, Proot, plant)) {
		report("Could not plant events between P's root and P");
		return;
	}
	else
		CHECK;
/* Generate xname event by changing focus from A to PointerRoot. */
	XSync(display, True);
	XSetInputFocus(display, PointerRoot, RevertToNone, CurrentTime);
	XSync(display, False);
	if (winh_harvest(display, (Winh *) NULL)) {
		report("Could not harvest events");
		return;
	}
	else
		CHECK;
/* Verify that the expected events were delivered. */
	if (winh_ignore_event((Winh *) NULL, OTHEREVENT, WINH_NOMASK)) {
		delete("Could not ignore %s events", eventname(OTHEREVENT));
		return;
	}
	else
		CHECK;
	status = winh_weed((Winh *) NULL, -1, WINH_WEED_IDENTITY);
	if (status < 0)
		return;
	else if (status > 0) {
		report("Event delivery was not as expected");
		FAIL;
	}
	else {
/* Verify that event delivered to the root window of all screens */
/* with detail set to NotifyPointerRoot. */
		_detail_ = NotifyPointerRoot;
		/* used to keep track of last NotifyPointerRoot event */
		high = 0;
		for (ptr = guardian; ptr != (Winh *) NULL; ptr = ptr->nextsibling) {
			if (ptr == guardian)
				CHECK;
			/*
			 * skip to first FocusIn-type event
			 */
			for (winhe = ptr->delivered; winhe != (Winhe *) NULL; winhe = winhe->next) {
				if (winhe->event->type == EVENT)
					break;
			}
			if (winhe == (Winhe *) NULL) {
				delete("Lost %s event in delivered list",
					eventname(EVENT));
				return;
			}
			if (winhe->sequence > high)
				high = winhe->sequence;
			if (winhe->event->xfocus.detail != _detail_) {
				report("Got detail %d, expected %d on window 0x%x",
					winhe->event->xfocus.detail,
					_detail_, ptr->window);
				FAIL;
			}
			else {
				/*
				 * cause this event to be ignored during
				 * later checks for FocusIn events
				 */
				winhe->event->type = 0;
			}
		}
/* Verify that event generated on each window from window P's root down */
/* to and including window P with detail set to NotifyPointer. */
		_detail_ = NotifyPointer;
		if (winh_climb(P, Proot, checkdetail))
			FAIL;
		else
			CHECK;
/* Verify that these events occurred in the correct order. */
		increasing = True;
		if (winh_climb(P, Proot, checksequence))
			FAIL;
		else
			CHECK;
/* Verify that the NotifyPointerRoot events were delivered before */
/* NotifyPointer events. */
		for (winhe = Proot->delivered; winhe != (Winhe *) NULL; winhe = winhe->next) {
			if (winhe == Proot->delivered)
				CHECK;
			if (winhe->event->type == EVENT)
				break;
		}
		if (winhe == (Winhe *) NULL) {
			delete("Lost %s event in delivered list of Proot",
				eventname(EVENT));
			return;
		}
		else
			CHECK;
		if (high > winhe->sequence) {
			report("NotifyPointerRoot events not delivered before all NotifyPointer events");
			FAIL;
		}
		else
			CHECK;

/* Verify that all xname events are delivered after all */
/* FocusOut events. */
		status = winh_ordercheck(OTHEREVENT, EVENT);
		if (status == -1)
			return;
		else if (status) {
			int	in, out;

			in = winh_eventindex(EVENT);
			out = winh_eventindex(OTHEREVENT);
			report("%s: %d, %s: %d",
				eventname(EVENT), winh_event_stats[in].low,
				eventname(OTHEREVENT), winh_event_stats[out].high
				);
			FAIL;
		}
		else
			CHECK;
	}
	CHECKPASS(15);
>>ASSERTION Good A
When the focus moves from window A to
.S None
(discard)
and the pointer is in window P,
then a xname event is generated on
the root window of all screens,
with
.M detail
set to
.S NotifyDetailNone .
>>STRATEGY
Create client.
Build window hierarchy on all supported screens.
Set window A.
Set input focus to window A.
Select for Focus events on windows.
Generate xname event by changing focus from A to None.
Verify that the expected events were delivered.
Verify that event delivered to the root window of all screens
with detail set to NotifyDetailNone.
Verify that all xname events are delivered after all
FocusOut events.
>>CODE
Display	*display;
int	depth = 1;
Winh	*A, *ptr;
Winhe	*winhe;
int	status;

/* Create client. */
	if ((display = opendisplay()) == (Display *) NULL) {
		delete("Couldn't create client.");
		return;
	}
	else
		CHECK;
/* Build window hierarchy on all supported screens. */
	if (winh(display, depth, WINH_MAP|WINH_BOTH_SCREENS)) {
		report("Could not build window hierarchy");
		return;
	}
	else
		CHECK;
/* Set window A. */
	A = guardian->firstchild;
	/* Don't bother moving the pointer to known location. */
/* Set input focus to window A. */
	XSetInputFocus(display, A->window, RevertToNone, CurrentTime);
/* Select for Focus events on windows. */
	if (winh_selectinput(display, (Winh *) NULL, MASK)) {
		report("Could not select for events");
		return;
	}
	else
		CHECK;
	good.type = EVENT;
	good.xany.display = display;
	/* root window of all screens */
	for (ptr = guardian; ptr != (Winh *) NULL; ptr = ptr->nextsibling) {
		if (ptr == guardian)
			CHECK;
		good.xany.window = ptr->window;
		if (winh_plant(ptr, &good, MASK, WINH_NOMASK)) {
			report("Could not plant events");
			return;
		}
	}
/* Generate xname event by changing focus from A to None. */
	XSync(display, True);
	XSetInputFocus(display, None, RevertToNone, CurrentTime);
	XSync(display, False);
	if (winh_harvest(display, (Winh *) NULL)) {
		report("Could not harvest events");
		return;
	}
	else
		CHECK;
/* Verify that the expected events were delivered. */
	if (winh_ignore_event((Winh *) NULL, OTHEREVENT, WINH_NOMASK)) {
		delete("Could not ignore %s events", eventname(OTHEREVENT));
		return;
	}
	else
		CHECK;
	status = winh_weed((Winh *) NULL, -1, WINH_WEED_IDENTITY);
	if (status < 0)
		return;
	else if (status > 0) {
		report("Event delivery was not as expected");
		FAIL;
	}
	else {
/* Verify that event delivered to the root window of all screens */
/* with detail set to NotifyDetailNone. */
		_detail_ = NotifyDetailNone;
		for (ptr = guardian; ptr != (Winh *) NULL; ptr = ptr->nextsibling) {
			if (ptr == guardian)
				CHECK;
			/*
			 * skip to first FocusIn-type event
			 */
			for (winhe = ptr->delivered; winhe != (Winhe *) NULL; winhe = winhe->next) {
				if (winhe->event->type == EVENT)
					break;
			}
			if (winhe == (Winhe *) NULL) {
				delete("Lost %s event in delivered list",
					eventname(EVENT));
				return;
			}
			if (winhe->event->xfocus.detail != _detail_) {
				report("Got detail %d, expected %d on window 0x%x",
					winhe->event->xfocus.detail,
					_detail_, ptr->window);
				FAIL;
			}
		}
/* Verify that all xname events are delivered after all */
/* FocusOut events. */
		status = winh_ordercheck(OTHEREVENT, EVENT);
		if (status == -1)
			return;
		else if (status)
			FAIL;
		else
			CHECK;
	}
	CHECKPASS(8);
>>ASSERTION Good A
>>#NOTE	One could argue that this is tested sufficiently in the next assertion.
When the focus moves from
.S PointerRoot
(events sent to the window under the pointer)
or
.S None
(discard)
to window A
and the pointer is in window P
and window A is not a root window,
then a xname event is generated on
each window from window A's root down to but not including window A,
with
.M detail
set to
.S NotifyNonlinearVirtual .
>>STRATEGY
Create client.
Build window hierarchy.
Set window A.
Move pointer to known location.
Set input focus to PointerRoot.
Select for Focus events on windows.
Generate xname event by changing focus from PointerRoot to A.
Verify that the expected events were delivered.
Verify that event delivered on each window from window A's root down to
but not including window A
with detail set to NotifyNonlinearVirtual.
Verify that these events occurred in the correct order.
Verify that all xname events are delivered after all
FocusOut events.
Repeat with focus initially set to None.
>>CODE
Display	*display;
int	depth = 4;
Winh	*A, *Aroot;
int	status;
int	i;
static	Window	focuses[] = {
	(Window) PointerRoot,
	(Window) None
};

/* Create client. */
	if ((display = opendisplay()) == (Display *) NULL) {
		delete("Couldn't create client.");
		return;
	}
	else
		CHECK;
/* Build window hierarchy. */
	if (winh(display, depth, WINH_MAP)) {
		report("Could not build window hierarchy");
		return;
	}
	else
		CHECK;
/* Set window A. */
	Aroot = guardian;
	A = Aroot->firstchild->firstchild->firstchild;
/* Move pointer to known location. */
	if (warppointer(display, DRW(display), 0, 0) == (PointerPlace *) NULL)
		return;
	else
		CHECK;
	for (i = 0; i < NELEM(focuses); i++) {
/* Set input focus to PointerRoot. */
		XSetInputFocus(display, focuses[i], RevertToNone, CurrentTime);
/* Select for Focus events on windows. */
		if (winh_selectinput(display, (Winh *) NULL, MASK)) {
			report("Could not select for events");
			return;
		}
		else
			CHECK;
		good.type = EVENT;
		good.xany.display = display;
		/*
		 * Plant at A instead of A->parent because we will also be getting
		 * an xname event on A with detail set to NotifyNonlinear.
		 */
		if (winh_climb(A, Aroot, plant)) {
			report("Could not plant events");
			return;
		}
		else
			CHECK;
/* Generate xname event by changing focus from PointerRoot to A. */
		XSync(display, True);
		XSetInputFocus(display, A->window, RevertToNone, CurrentTime);
		XSync(display, False);
		if (winh_harvest(display, (Winh *) NULL)) {
			report("Could not harvest events");
			return;
		}
		else
			CHECK;
/* Verify that the expected events were delivered. */
		if (winh_ignore_event((Winh *) NULL, OTHEREVENT, WINH_NOMASK)) {
			delete("Could not ignore %s events", eventname(OTHEREVENT));
			return;
		}
		else
			CHECK;
		status = winh_weed((Winh *) NULL, -1, WINH_WEED_IDENTITY);
		if (status < 0)
			return;
		else if (status > 0) {
			report("Event delivery was not as expected");
			FAIL;
		}
		else {
/* Verify that event delivered on each window from window A's root down to */
/* but not including window A */
/* with detail set to NotifyNonlinearVirtual. */
			_detail_ = NotifyNonlinearVirtual;
			if (winh_climb(A->parent, Aroot, checkdetail))
				FAIL;
			else
				CHECK;
/* Verify that these events occurred in the correct order. */
			increasing = True;
			if (winh_climb(A->parent, Aroot, checksequence))
				FAIL;
			else
				CHECK;
/* Verify that all xname events are delivered after all */
/* FocusOut events. */
			status = winh_ordercheck(OTHEREVENT, EVENT);
			if (status == -1)
				return;
			else if (status)
				FAIL;
			else
				CHECK;
		}
/* Repeat with focus initially set to None. */
	}
	CHECKPASS(3 + (7*NELEM(focuses)));
>>ASSERTION Good A
>>#NOTE	One could argue that this is tested sufficiently in the next assertion.
>>#NOTE
>>#NOTE	The approved (and incorrect) form of this assertion used to
>>#NOTE	contain the following phrase:
>>#NOTE
>>#NOTE		then a xname event is generated on
>>#NOTE
>>#NOTE Additional text was inserted after the "then" to reflect
>>#NOTE ordering with respect to NotifyNonlinearVirtual FocusIn events.
When the focus moves from
.S PointerRoot
(events sent to the window under the pointer)
or
.S None
(discard)
to window A
and the pointer is in window P,
then, after any related xname events are generated
with
.M detail
set to
.S NotifyNonlinearVirtual ,
a xname event is generated
on window A
with
.M detail
set to
.S NotifyNonlinear .
>>STRATEGY
Create client.
Build window hierarchy.
Set window A.
Move pointer to known location.
Set input focus to PointerRoot.
Select for Focus events on windows.
Generate xname event by changing focus from PointerRoot to A.
Verify that the expected events were delivered.
Verify that event delivered on each window from window A's root down to
but not including window A
with detail set to NotifyNonlinearVirtual.
Verify that event delivered on to window A
with detail set to NotifyNonlinear.
Verify that these events occurred in the correct order.
Verify that all xname events are delivered after all
FocusOut events.
Repeat with focus initially set to None.
>>CODE
Display	*display;
int	depth = 4;
Winh	*A, *Aroot;
int	status;
int	i;
static	Window	focuses[] = {
	(Window) PointerRoot,
	(Window) None
};

/* Create client. */
	if ((display = opendisplay()) == (Display *) NULL) {
		delete("Couldn't create client.");
		return;
	}
	else
		CHECK;
/* Build window hierarchy. */
	if (winh(display, depth, WINH_MAP)) {
		report("Could not build window hierarchy");
		return;
	}
	else
		CHECK;
/* Set window A. */
	Aroot = guardian;
	A = Aroot->firstchild->firstchild->firstchild;
/* Move pointer to known location. */
	if (warppointer(display, DRW(display), 0, 0) == (PointerPlace *) NULL)
		return;
	else
		CHECK;
	for (i = 0; i < NELEM(focuses); i++) {
/* Set input focus to PointerRoot. */
		XSetInputFocus(display, focuses[i], RevertToNone, CurrentTime);
/* Select for Focus events on windows. */
		if (winh_selectinput(display, (Winh *) NULL, MASK)) {
			report("Could not select for events");
			return;
		}
		else
			CHECK;
		good.type = EVENT;
		good.xany.display = display;
		if (winh_climb(A, Aroot, plant)) {
			report("Could not plant events");
			return;
		}
		else
			CHECK;
/* Generate xname event by changing focus from PointerRoot to A. */
		XSync(display, True);
		XSetInputFocus(display, A->window, RevertToNone, CurrentTime);
		XSync(display, False);
		if (winh_harvest(display, (Winh *) NULL)) {
			report("Could not harvest events");
			return;
		}
		else
			CHECK;
/* Verify that the expected events were delivered. */
		if (winh_ignore_event((Winh *) NULL, OTHEREVENT, WINH_NOMASK)) {
			delete("Could not ignore %s events", eventname(OTHEREVENT));
			return;
		}
		else
			CHECK;
		status = winh_weed((Winh *) NULL, -1, WINH_WEED_IDENTITY);
		if (status < 0)
			return;
		else if (status > 0) {
			report("Event delivery was not as expected");
			FAIL;
		}
		else {
/* Verify that event delivered on each window from window A's root down to */
/* but not including window A */
/* with detail set to NotifyNonlinearVirtual. */
			_detail_ = NotifyNonlinearVirtual;
			if (winh_climb(A->parent, Aroot, checkdetail))
				FAIL;
			else
				CHECK;
/* Verify that event delivered on to window A */
/* with detail set to NotifyNonlinear. */
			_detail_ = NotifyNonlinear;
			if (winh_climb(A, A, checkdetail))
				FAIL;
			else
				CHECK;
/* Verify that these events occurred in the correct order. */
			increasing = True;
			if (winh_climb(A, Aroot, checksequence))
				FAIL;
			else
				CHECK;
/* Verify that all xname events are delivered after all */
/* FocusOut events. */
			status = winh_ordercheck(OTHEREVENT, EVENT);
			if (status == -1)
				return;
			else if (status)
				FAIL;
			else
				CHECK;
		}
/* Repeat with focus initially set to None. */
	}
	CHECKPASS(3 + (8*NELEM(focuses)));
>>ASSERTION Good A
When the focus moves from
.S PointerRoot
(events sent to the window under the pointer)
or
.S None
(discard)
to window A
and the pointer is in window P
and window P is an inferior of window A,
then, after the related xname events are generated
with
.M detail
set to
.S NotifyNonlinearVirtual
and
.S NotifyNonlinear ,
a xname event is generated on
each window below window A down to and including window P,
with
.M detail
set to
.S NotifyPointer .
>>STRATEGY
Create client.
Build window hierarchy.
Set window A.
Set window P.
Move pointer to window P.
Set input focus to PointerRoot.
Select for Focus events on windows.
Generate xname event by changing focus from PointerRoot to A.
Verify that the expected events were delivered.
Verify that event delivered on each window from window A's root down to
but not including window A
with detail set to NotifyNonlinearVirtual.
Verify that event delivered on to window A
with detail set to NotifyNonlinear.
Verify that event delivered on each window below window A down to
and including window P
with detail set to NotifyPointer.
Verify that these events occurred in the correct order.
Verify that all xname events are delivered after all
FocusOut events.
Repeat with focus initially set to None.
>>CODE
Display	*display;
int	depth = 5;
Winh	*A, *Aroot, *P;
int	status;
int	i;
static	Window	focuses[] = {
	(Window) PointerRoot,
	(Window) None
};

/* Create client. */
	if ((display = opendisplay()) == (Display *) NULL) {
		delete("Couldn't create client.");
		return;
	}
	else
		CHECK;
/* Build window hierarchy. */
	if (winh(display, depth, WINH_MAP)) {
		report("Could not build window hierarchy");
		return;
	}
	else
		CHECK;
/* Set window A. */
	Aroot = guardian;
	A = Aroot->firstchild->firstchild;
/* Set window P. */
	P = A->firstchild->firstchild;
/* Move pointer to window P. */
	if (warppointer(display, P->window, 0, 0) == (PointerPlace *) NULL)
		return;
	else
		CHECK;
	for (i = 0; i < NELEM(focuses); i++) {
/* Set input focus to PointerRoot. */
		XSetInputFocus(display, focuses[i], RevertToNone, CurrentTime);
/* Select for Focus events on windows. */
		if (winh_selectinput(display, (Winh *) NULL, MASK)) {
			report("Could not select for events");
			return;
		}
		else
			CHECK;
		good.type = EVENT;
		good.xany.display = display;
		if (winh_climb(P, Aroot, plant)) {
			report("Could not plant events");
			return;
		}
		else
			CHECK;
/* Generate xname event by changing focus from PointerRoot to A. */
		XSync(display, True);
		XSetInputFocus(display, A->window, RevertToNone, CurrentTime);
		XSync(display, False);
		if (winh_harvest(display, (Winh *) NULL)) {
			report("Could not harvest events");
			return;
		}
		else
			CHECK;
/* Verify that the expected events were delivered. */
		if (winh_ignore_event((Winh *) NULL, OTHEREVENT, WINH_NOMASK)) {
			delete("Could not ignore %s events", eventname(OTHEREVENT));
			return;
		}
		else
			CHECK;
		status = winh_weed((Winh *) NULL, -1, WINH_WEED_IDENTITY);
		if (status < 0)
			return;
		else if (status > 0) {
			report("Event delivery was not as expected");
			FAIL;
		}
		else {
/* Verify that event delivered on each window from window A's root down to */
/* but not including window A */
/* with detail set to NotifyNonlinearVirtual. */
			_detail_ = NotifyNonlinearVirtual;
			if (winh_climb(A->parent, Aroot, checkdetail)) {
				report("Bad detail on some window above A's parent");
				FAIL;
			}
			else
				CHECK;
/* Verify that event delivered on to window A */
/* with detail set to NotifyNonlinear. */
			_detail_ = NotifyNonlinear;
			if (winh_climb(A, A, checkdetail)) {
				report("Bad detail on window A");
				FAIL;
			}
			else
				CHECK;
/* Verify that event delivered on each window below window A down to */
/* and including window P */
/* with detail set to NotifyPointer. */
			_detail_ = NotifyPointer;
			if (winh_climb(P, A->firstchild, checkdetail)) {
				report("Bad detail on some window below window A down to window P");
				FAIL;
			}
			else
				CHECK;
/* Verify that these events occurred in the correct order. */
			increasing = True;
			if (winh_climb(P, Aroot, checksequence))
				FAIL;
			else
				CHECK;
/* Verify that all xname events are delivered after all */
/* FocusOut events. */
			status = winh_ordercheck(OTHEREVENT, EVENT);
			if (status == -1)
				return;
			else if (status)
				FAIL;
			else
				CHECK;
		}
/* Repeat with focus initially set to None. */
	}
	CHECKPASS(3 + (9*NELEM(focuses)));
>>ASSERTION Good A
>>#NOTE
>>#NOTE The approved wording for this assertion specified that detail
>>#NOTE should be set to NotifyPointerRoot.  This was incorrect and the
>>#NOTE wording has been changed to specify that detail should be set to
>>#NOTE NotifyDetailNone.
>>#NOTE
When the focus moves from
.S PointerRoot
(events sent to the window under the pointer)
to
.S None
(discard)
and the pointer is in window P,
then a xname event is generated on
the root window of all screens,
with
.M detail
set to
.S NotifyDetailNone .
>>STRATEGY
Create client.
Build window hierarchy on all supported screens.
Move pointer to known location.
Set input focus to PointerRoot.
Select for Focus events on windows.
Generate xname event by changing focus from PointerRoot to None.
Verify that the expected events were delivered.
Verify that event delivered to the root window of all screens
with detail set to NotifyDetailNone.
Verify that all xname events are delivered after all
FocusOut events.
>>CODE
Display	*display;
int	depth = 2;
Winh	*ptr;
Winhe	*winhe;
int	status;

/* Create client. */
	if ((display = opendisplay()) == (Display *) NULL) {
		delete("Couldn't create client.");
		return;
	}
	else
		CHECK;
/* Build window hierarchy on all supported screens. */
	if (winh(display, depth, WINH_MAP|WINH_BOTH_SCREENS)) {
		report("Could not build window hierarchy");
		return;
	}
	else
		CHECK;
/* Move pointer to known location. */
	if (warppointer(display, DRW(display), 0, 0) == (PointerPlace *) NULL)
		return;
	else
		CHECK;
/* Set input focus to PointerRoot. */
	XSetInputFocus(display, PointerRoot, RevertToNone, CurrentTime);
/* Select for Focus events on windows. */
	if (winh_selectinput(display, (Winh *) NULL, MASK)) {
		report("Could not select for events");
		return;
	}
	else
		CHECK;
	good.type = EVENT;
	good.xany.display = display;
	/* root window of all screens */
	for (ptr = guardian; ptr != (Winh *) NULL; ptr = ptr->nextsibling) {
		if (ptr == guardian)
			CHECK;
		good.xany.window = ptr->window;
		if (winh_plant(ptr, &good, MASK, WINH_NOMASK)) {
			report("Could not plant events");
			return;
		}
	}
/* Generate xname event by changing focus from PointerRoot to None. */
	XSync(display, True);
	XSetInputFocus(display, None, RevertToNone, CurrentTime);
	XSync(display, False);
	if (winh_harvest(display, (Winh *) NULL)) {
		report("Could not harvest events");
		return;
	}
	else
		CHECK;
/* Verify that the expected events were delivered. */
	if (winh_ignore_event((Winh *) NULL, OTHEREVENT, WINH_NOMASK)) {
		delete("Could not ignore %s events", eventname(OTHEREVENT));
		return;
	}
	else
		CHECK;
	status = winh_weed((Winh *) NULL, -1, WINH_WEED_IDENTITY);
	if (status < 0)
		return;
	else if (status > 0) {
		report("Event delivery was not as expected");
		FAIL;
	}
	else {
/* Verify that event delivered to the root window of all screens */
/* with detail set to NotifyDetailNone. */
		_detail_ = NotifyDetailNone;
		for (ptr = guardian; ptr != (Winh *) NULL; ptr = ptr->nextsibling) {
			if (ptr == guardian)
				CHECK;
			/*
			 * skip to first FocusIn-type event
			 */
			for (winhe = ptr->delivered; winhe != (Winhe *) NULL; winhe = winhe->next) {
				if (winhe->event->type == EVENT)
					break;
			}
			if (winhe == (Winhe *) NULL) {
				delete("Lost %s event in delivered list",
					eventname(EVENT));
				return;
			}
			if (winhe->event->xfocus.detail != _detail_) {
				report("Got detail %d, expected %d on window 0x%x",
					winhe->event->xfocus.detail,
					_detail_, ptr->window);
				FAIL;
			}
			else {
				/*
				 * cause this event to be ignored during
				 * later checks for FocusIn events
				 */
				winhe->event->type = 0;
			}
		}
/* Verify that all xname events are delivered after all */
/* FocusOut events. */
		status = winh_ordercheck(OTHEREVENT, EVENT);
		if (status == -1)
			return;
		else if (status)
			FAIL;
		else
			CHECK;
	}
	CHECKPASS(9);
>>ASSERTION Good A
>>#NOTE
>>#NOTE The approved wording for this assertion specified that detail
>>#NOTE should be set to NotifyDetailNone.  This was incorrect and the
>>#NOTE wording has been changed to specify that detail should be set to
>>#NOTE NotifyPointerRoot.
>>#NOTE
When the focus moves from
None to PointerRoot
and the pointer is in window P,
then a xname event is generated on
the root window of all screens,
with
.M detail
set to
.S NotifyPointerRoot .
>>STRATEGY
Create client.
Build window hierarchy on all supported screens.
Set window P.
Move pointer to known location.
Set input focus to None.
Select for Focus events on windows.
Generate xname event by changing focus from None to PointerRoot.
Verify that the expected events were delivered.
Verify that event delivered to the root window of all screens
with detail set to NotifyPointerRoot.
Verify that all xname events are delivered after all
FocusOut events.
>>CODE
Display	*display;
int	depth = 2;
Winh	*P, *ptr;
Winhe	*winhe;
int	status;

/* Create client. */
	if ((display = opendisplay()) == (Display *) NULL) {
		delete("Couldn't create client.");
		return;
	}
	else
		CHECK;
/* Build window hierarchy on all supported screens. */
	if (winh(display, depth, WINH_MAP|WINH_BOTH_SCREENS)) {
		report("Could not build window hierarchy");
		return;
	}
	else
		CHECK;
/* Set window P. */
	P = guardian;
/* Move pointer to known location. */
	if (warppointer(display, P->window, 0, 0) == (PointerPlace *) NULL)
		return;
	else
		CHECK;
/* Set input focus to None. */
	XSetInputFocus(display, None, RevertToNone, CurrentTime);
/* Select for Focus events on windows. */
	if (winh_selectinput(display, (Winh *) NULL, MASK)) {
		report("Could not select for events");
		return;
	}
	else
		CHECK;
	good.type = EVENT;
	good.xany.display = display;
	/* root window of all screens */
	for (ptr = guardian; ptr != (Winh *) NULL; ptr = ptr->nextsibling) {
		if (ptr == guardian)
			CHECK;
		good.xany.window = ptr->window;
		if (winh_plant(ptr, &good, MASK, WINH_NOMASK)) {
			report("Could not plant events");
			return;
		}
	}
	/* also expect a subsequent event on P */
	good.xany.window = P->window;
	if (winh_plant(P, &good, MASK, WINH_NOMASK)) {
		report("Could not plant events on P");
		return;
	}
	else
		CHECK;
/* Generate xname event by changing focus from None to PointerRoot. */
	XSync(display, True);
	XSetInputFocus(display, PointerRoot, RevertToNone, CurrentTime);
	XSync(display, False);
	if (winh_harvest(display, (Winh *) NULL)) {
		report("Could not harvest events");
		return;
	}
	else
		CHECK;
/* Verify that the expected events were delivered. */
	if (winh_ignore_event((Winh *) NULL, OTHEREVENT, WINH_NOMASK)) {
		delete("Could not ignore %s events", eventname(OTHEREVENT));
		return;
	}
	else
		CHECK;
	status = winh_weed((Winh *) NULL, -1, WINH_WEED_IDENTITY);
	if (status < 0)
		return;
	else if (status > 0) {
		report("Event delivery was not as expected");
		FAIL;
	}
	else {
/* Verify that event delivered to the root window of all screens */
/* with detail set to NotifyPointerRoot. */
		_detail_ = NotifyPointerRoot;
		for (ptr = guardian; ptr != (Winh *) NULL; ptr = ptr->nextsibling) {
			if (ptr == guardian)
				CHECK;
			/*
			 * skip to first FocusIn-type event
			 */
			for (winhe = ptr->delivered; winhe != (Winhe *) NULL; winhe = winhe->next) {
				if (winhe->event->type == EVENT)
					break;
			}
			if (winhe == (Winhe *) NULL) {
				delete("Lost %s event in delivered list",
					eventname(EVENT));
				return;
			}
			if (winhe->event->xfocus.detail != _detail_) {
				report("Got detail %d, expected %d on window 0x%x",
					winhe->event->xfocus.detail,
					_detail_, ptr->window);
				FAIL;
			}
			else {
				/*
				 * cause this event to be ignored during
				 * later checks for FocusIn events
				 */
				winhe->event->type = 0;
			}
		}
/* Verify that all xname events are delivered after all */
/* FocusOut events. */
		status = winh_ordercheck(OTHEREVENT, EVENT);
		if (status == -1)
			return;
		else if (status)
			FAIL;
		else
			CHECK;
	}
	CHECKPASS(10);
>>ASSERTION Good A
>>#NOTE
>>#NOTE The wording of this assertion was changed from that which was
>>#NOTE approved to eliminate extraneous wording:
>>#NOTE
>>#NOTE	.S PointerRoot
>>#NOTE	(events sent to the window under the pointer)
>>#NOTE	to
>>#NOTE	.S None
>>#NOTE	(discard)
>>#NOTE
>>#NOTE	.S NotifyDetailNone
>>#NOTE	and
>>#NOTE
When the focus moves from
None to PointerRoot
and the pointer is in window P,
then, after the related xname events are generated
with
.M detail
set to
.S NotifyPointerRoot ,
a xname event is generated on
each window from window P's root down to and including window P,
with
.M detail
set to
.S NotifyPointer .
>>STRATEGY
Create client.
Build window hierarchy on all supported screens.
Set window P.
Move pointer to known location.
Set input focus to None.
Select for Focus events on windows.
Generate xname event by changing focus from None to PointerRoot.
Verify that the expected events were delivered.
Verify that event delivered to the root window of all screens
with detail set to NotifyPointerRoot.
Verify that event delivered on each window from window P's root down to
and including window P
with detail set to NotifyPointer.
Verify that the NotifyPointerRoot events were delivered before
NotifyPointer events.
Verify that all xname events are delivered after all
FocusOut events.
>>CODE
Display	*display;
int	depth = 4;
Winh	*P, *Proot, *ptr;
Winhe	*winhe;
int	status;
int	high;

/* Create client. */
	if ((display = opendisplay()) == (Display *) NULL) {
		delete("Couldn't create client.");
		return;
	}
	else
		CHECK;
/* Build window hierarchy on all supported screens. */
	if (winh(display, depth, WINH_MAP|WINH_BOTH_SCREENS)) {
		report("Could not build window hierarchy");
		return;
	}
	else
		CHECK;
/* Set window P. */
	Proot = guardian;
	P = Proot->firstchild->firstchild->firstchild;
/* Move pointer to known location. */
	if (warppointer(display, P->window, 0, 0) == (PointerPlace *) NULL)
		return;
	else
		CHECK;
/* Set input focus to None. */
	XSetInputFocus(display, None, RevertToNone, CurrentTime);
/* Select for Focus events on windows. */
	if (winh_selectinput(display, (Winh *) NULL, MASK)) {
		report("Could not select for events");
		return;
	}
	else
		CHECK;
	good.type = EVENT;
	good.xany.display = display;
	/* root window of all screens */
	for (ptr = guardian; ptr != (Winh *) NULL; ptr = ptr->nextsibling) {
		if (ptr == guardian)
			CHECK;
		good.xany.window = ptr->window;
		if (winh_plant(ptr, &good, MASK, WINH_NOMASK)) {
			report("Could not plant events");
			return;
		}
	}
	if (winh_climb(P, Proot, plant)) {
		report("Could not plant events from P's root to P");
		return;
	}
	else
		CHECK;
/* Generate xname event by changing focus from None to PointerRoot. */
	XSync(display, True);
	XSetInputFocus(display, PointerRoot, RevertToNone, CurrentTime);
	XSync(display, False);
	if (winh_harvest(display, (Winh *) NULL)) {
		report("Could not harvest events");
		return;
	}
	else
		CHECK;
/* Verify that the expected events were delivered. */
	if (winh_ignore_event((Winh *) NULL, OTHEREVENT, WINH_NOMASK)) {
		delete("Could not ignore %s events", eventname(OTHEREVENT));
		return;
	}
	else
		CHECK;
	status = winh_weed((Winh *) NULL, -1, WINH_WEED_IDENTITY);
	if (status < 0)
		return;
	else if (status > 0) {
		report("Event delivery was not as expected");
		FAIL;
	}
	else {
/* Verify that event delivered to the root window of all screens */
/* with detail set to NotifyPointerRoot. */
		_detail_ = NotifyPointerRoot;
		high = 0;
		for (ptr = guardian; ptr != (Winh *) NULL; ptr = ptr->nextsibling) {
			if (ptr == guardian)
				CHECK;
			/*
			 * skip to first FocusIn-type event
			 */
			for (winhe = ptr->delivered; winhe != (Winhe *) NULL; winhe = winhe->next) {
				if (winhe->event->type == EVENT)
					break;
			}
			if (winhe == (Winhe *) NULL) {
				delete("Lost %s event in delivered list",
					eventname(EVENT));
				return;
			}
			if (winhe->sequence > high)
				high = winhe->sequence;
			if (winhe->event->xfocus.detail != _detail_) {
				report("Got detail %d, expected %d on window 0x%x",
					winhe->event->xfocus.detail,
					_detail_, ptr->window);
				FAIL;
			}
			else {
				/*
				 * cause this event to be ignored during
				 * later checks for FocusIn events
				 */
				winhe->event->type = 0;
			}
		}
/* Verify that event delivered on each window from window P's root down to */
/* and including window P */
/* with detail set to NotifyPointer. */
			_detail_ = NotifyPointer;
			if (winh_climb(P, Proot, checkdetail)) {
				report("Bad detail on some window between windows P's root and P");
				FAIL;
			}
			else
				CHECK;
/* Verify that the NotifyPointerRoot events were delivered before */
/* NotifyPointer events. */
		for (winhe = Proot->delivered; winhe != (Winhe *) NULL; winhe = winhe->next) {
			if (winhe == Proot->delivered)
				CHECK;
			if (winhe->event->type == EVENT)
				break;
		}
		if (winhe == (Winhe *) NULL) {
			delete("Lost %s event in delivered list of Proot",
				eventname(EVENT));
			return;
		}
		else
			CHECK;
		if (high > winhe->sequence) {
			report("NotifyPointerRoot events not delivered before all NotifyPointer events");
			FAIL;
		}
		else
			CHECK;
/* Verify that all xname events are delivered after all */
/* FocusOut events. */
		status = winh_ordercheck(OTHEREVENT, EVENT);
		if (status == -1)
			return;
		else if (status)
			FAIL;
		else
			CHECK;
	}
	CHECKPASS(14);

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
 * $XConsortium: prprtyntfy.m,v 1.9 94/04/17 21:08:07 rws Exp $
 */
>>TITLE PropertyNotify CH08
>>EXTERN
#include	"Xatom.h"
#define	EVENT		PropertyNotify
#define	MASK		PropertyChangeMask
>>ASSERTION Good A
When a xname event is generated,
then
all clients having set
.S PropertyChangeMask
event mask bits on the event window are delivered
a xname event.
>>STRATEGY
Create clients client2 and client3.
Create window.
Select for PropertyNotify events using PropertyChangeMask.
Select for PropertyNotify events using PropertyChangeMask with client2.
Select for no events with client3.
Create atom for property.
Generate PropertyNotify event.
Verify that a PropertyNotify event was delivered.
Verify that event member fields are correctly set.
Verify that a PropertyNotify event was delivered to client2.
Verify that event member fields are correctly set.
Verify that no events were delivered to client3.
>>CODE
Display	*display = Dsp;
Display	*client2;
Display	*client3;
Window	w;
int	count;
XEvent	event_return;
XPropertyEvent good;
Atom	atom;
int	data = 42;

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
/* Select for PropertyNotify events using PropertyChangeMask. */
	XSelectInput(display, w, MASK);
/* Select for PropertyNotify events using PropertyChangeMask with client2. */
	XSelectInput(client2, w, MASK);
/* Select for no events with client3. */
	XSelectInput(client3, w, NoEventMask);
/* Create atom for property. */
	atom = XInternAtom(display, XT_TIMESTAMP, False);
	if (atom == None) {
		delete("XInternAtom of '%s' failed.", XT_TIMESTAMP);
		return;
	}
	else
		CHECK;
/* Generate PropertyNotify event. */
	XSync(display, True);
	XSync(client2, True);
	XSync(client3, True);
	XChangeProperty(display, w, atom, XA_INTEGER, 32, PropModeReplace, (unsigned char *)&data, 1);
	XSync(display, False);
	XSync(client2, False);
	XSync(client3, False);
/* Verify that a PropertyNotify event was delivered. */
/* Verify that event member fields are correctly set. */
	if (!XCheckTypedWindowEvent(display, w, EVENT, &event_return)) {
		report("Expected %s event, got none", eventname(EVENT));
		FAIL;
	}
	else
		CHECK;
	good = event_return.xproperty;
	good.type = EVENT;
	good.send_event = False;
	good.display = display;
	good.window = w;
	good.atom = atom;
	good.state = PropertyNewValue;
	if (checkevent((XEvent *) &good, &event_return)) {
		report("Unexpected values in delivered event");
		FAIL;
	}
	else
		CHECK;
/* Verify that a PropertyNotify event was delivered to client2. */
/* Verify that event member fields are correctly set. */
	if (!XCheckTypedWindowEvent(client2, w, EVENT, &event_return)) {
		report("Expected %s event, got none", eventname(EVENT));
		FAIL;
	}
	else
		CHECK;
	good = event_return.xproperty;
	good.type = EVENT;
	good.send_event = False;
	good.display = client2;
	good.window = w;
	good.atom = atom;
	good.state = PropertyNewValue;
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
	
	CHECKPASS(8);
>>ASSERTION def
>>#NOTE	Tested for in previous assertion.
When a xname event is generated,
then
clients not having set
.S PropertyChangeMask
event mask bits on the event window are not delivered
a xname event.
>>#NOTEm >>ASSERTION
>>#NOTEm When a window's property changes,
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
>>#NOTEs .M atom
>>#NOTEs is set to the property's atom.
>>#NOTEm >>ASSERTION
>>#NOTEm >>#NOTE
>>#NOTEm >>#NOTE ``Changed'' includes adding zero-length data or replacing all or part of
>>#NOTEm >>#NOTE a property with identical data.
>>#NOTEm >>#NOTE
>>#NOTEm When ARTICLE xname event is delivered
>>#NOTEm and the property was changed by a call to
>>#NOTEm .F XChangeProperty ,
>>#NOTEm .F XRotateWindowProperties ,
>>#NOTEm .F XChangeProperty ,
>>#NOTEm or
>>#NOTEm .F XRotateWindowProperties ,
>>#NOTEm then
>>#NOTEm .M state
>>#NOTEm is set to
>>#NOTEm .S PropertyNewValue .
>>#NOTEm >>ASSERTION
>>#NOTEm When ARTICLE xname event is delivered
>>#NOTEm and the property was deleted by a call to
>>#NOTEm .F XDeleteProperty
>>#NOTEm or by a call to
>>#NOTEm .F XGetWindowProperty
>>#NOTEm with a delete argument of
>>#NOTEm .S True ,
>>#NOTEm then
>>#NOTEm .M state
>>#NOTEm is set to
>>#NOTEm .S PropertyDelete .
>>#NOTEs >>ASSERTION
>>#NOTEs >>#NOTE
>>#NOTEs >>#NOTE Untestable?
>>#NOTEs >>#NOTE
>>#NOTEs When ARTICLE xname event is delivered,
>>#NOTEs then
>>#NOTEs .M time
>>#NOTEs is set to
>>#NOTEs the server time when the property was changed.

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
 * $XConsortium: chckwdwevn.m,v 1.8 94/04/17 21:07:15 rws Exp $
 */
>>TITLE XCheckWindowEvent CH08
Bool
XCheckWindowEvent(display, w, event_mask, event_return)
Display *display = Dsp;
Window w;
long event_mask;
XEvent	*event_return = &_event;
>>EXTERN
/*
 * Can not use "xcall" because it empties the event queue.
 */
#define	_xcall_(rvalue)	\
		_startcall(display);\
		rvalue = XCheckWindowEvent(display, w, event_mask, event_return);\
		_endcall(display)
static XEvent _event;
>>ASSERTION Good A
A call to xname
returns in
.A event_return
the first event in the event queue that matches
window
.A w
and
.A event_mask .
>>STRATEGY
Create a window.
Discard all events on the event queue.
Call XPutBackEvent to put events on the event queue.
Call XCheckWindowEvent.
Verify that XCheckWindowEvent returned True.
Verify the correct event-type was returned.
Verify the event contained correct window.
Verify the first matching event in event queue was returned.
>>CODE
Window	w1;
Window	w2;
XEvent	event;
XAnyEvent *ep;
Bool	return_value;

/* Create a window. */
	w1 = mkwin(display, (XVisualInfo *) NULL, (struct area *) NULL, False);
	w2 = mkwin(display, (XVisualInfo *) NULL, (struct area *) NULL, False);
/* Discard all events on the event queue. */
	XSync(display, True);
/* Call XPutBackEvent to put events on the event queue. */
	ep = (XAnyEvent *) &event;
	ep->type = KeyPress;
	ep->window = w1;
	ep->send_event = False;
	XPutBackEvent(display, &event);
	ep->type = ButtonPress;
	ep->window = w2;
	ep->send_event = False;
	XPutBackEvent(display, &event);
	ep->type = ButtonPress;
	ep->window = w2;
	ep->send_event = True;	/* first occurrence has send_event True */
	XPutBackEvent(display, &event);
	ep->type = KeyPress;
	ep->window = w1;
	ep->send_event = False;
	XPutBackEvent(display, &event);
/* Call XCheckWindowEvent. */
	w = w2;
	event_mask = ButtonPressMask;
	_xcall_(return_value);
/* Verify that XCheckWindowEvent returned True. */
	if (return_value != True) {	
		report("Did not return True: returned %d", return_value);
		FAIL;
	}
	else
		CHECK;
/* Verify the correct event-type was returned. */
	ep = (XAnyEvent *) event_return;
	if (ep->type != ButtonPress) {
		report("Got %s, expected %s", eventname(ep->type), eventname(ButtonPress));
		FAIL;
	}
	else
		CHECK;
/* Verify the event contained correct window. */
	if (ep->window != w2) {
		report("Got %d, expected %d", ep->window, w2);
		FAIL;
	}
	else
		CHECK;
/* Verify the first matching event in event queue was returned. */
	if (ep->send_event != True) {
		report("First event in event queue was not returned.");
		FAIL;
	}
	else
		CHECK;
	CHECKPASS(4);
>>ASSERTION def
When a call to xname finds a matching event,
then
xname
returns
.S True .
>>ASSERTION Good A
A call to xname removes the returned event from the event queue.
>>STRATEGY
Create a window.
Discard all events on the event queue.
Call XPutBackEvent to put events on the event queue.
Call XPending to get the current event queue size.
Call XCheckWindowEvent.
Verify that XCheckWindowEvent returned True.
Call XPending to get the current event queue size.
Verify that size of event queue has decreased by one.
>>CODE
XEvent	event;
XAnyEvent *ep;
int	oldqsize;
int	newqsize;
Bool	return_value;

/* Create a window. */
	w = mkwin(display, (XVisualInfo *) NULL, (struct area *) NULL, False);
/* Discard all events on the event queue. */
	XSync(display, True);
/* Call XPutBackEvent to put events on the event queue. */
	ep = (XAnyEvent *) &event;
	ep->type = ButtonPressMask;
	ep->window = w;
	XPutBackEvent(display, &event);
/* Call XPending to get the current event queue size. */
	oldqsize = XPending(display);
/* Call XCheckWindowEvent. */
	event_mask = ButtonPressMask;
	_xcall_(return_value);
/* Verify that XCheckWindowEvent returned True. */
	if (return_value != True) {	
		report("Did not return True: returned %d", return_value);
		FAIL;
	}
	else
		CHECK;
/* Call XPending to get the current event queue size. */
	newqsize = XPending(display);
/* Verify that size of event queue has decreased by one. */
	if (newqsize != (oldqsize-1)) {
		report("Event queue size %d, expected %d", newqsize, oldqsize-1);
		FAIL;
	}
	else
		CHECK;
	CHECKPASS(2);
>>ASSERTION def
When a matching event is not in the event queue,
then a call to xname
returns in
.A event_return
the first matching event available on the X server connection.
>>ASSERTION Good A
When a matching event is not in the event queue and
is not available on the X server connection,
then a call to xname
returns
.S False .
>>STRATEGY
Create a window.
Discard all events on the event queue.
Call XCheckWindowEvent.
Verify that XCheckWindowEvent returned False.
>>CODE
Bool	return_value;

/* Create a window. */
	w = mkwin(display, (XVisualInfo *) NULL, (struct area *) NULL, False);
/* Discard all events on the event queue. */
	XSync(display, True);
/* Call XCheckWindowEvent. */
	event_mask = ButtonPressMask;
	_xcall_(return_value);
/* Verify that XCheckWindowEvent returned False. */
	if (return_value != False) {	
		report("Did not return False: returned %d", return_value);
		FAIL;
	}
	else
		CHECK;
	CHECKPASS(1);
>>ASSERTION Good A
When a matching event is not in the event queue and
is not available on the X server connection,
then a call to xname
flushes the output buffer.
>>STRATEGY
Create client2.
Discard all events on the event queue.
Create pixmap.
Create a window.
Call XCheckWindowEvent.
Verify that XCheckWindowEvent returned False.
Empty the buffer.
Ensure the server has dealt with anything flushed to it: do XSync()
Verify that the output buffer was flushed by effect on server.
>>CODE
Bool	return_value;
Pixmap	pm;
Display *client2;

/* Create client2. */
	client2 = opendisplay();
	if (client2 == (Display *) NULL) {
		delete("Can not open display");
		return;
	}
	else
		CHECK;
/* Discard all events on the event queue. */
	XSync(display, True);
/* Create pixmap. */
	/* avoid using makepixm() */
	pm = XCreatePixmap(display, DRW(display), 10, 10, 1);
/* Create a window. */
	w = mkwin(display, (XVisualInfo *) NULL, (struct area *) NULL, False);
/* Call XCheckWindowEvent. */
	event_mask = ButtonPressMask;
	_xcall_(return_value);
/* Verify that XCheckWindowEvent returned False. */
	if (return_value != False) {	
		report("Did not return False: returned %d", return_value);
		FAIL;
	}
	else
		CHECK;
/* Empty the buffer. */
	(void)XTestDiscard(display);
/* Ensure the server has dealt with anything flushed to it: do XSync() */
	XSync(display, False);
/* Verify that the output buffer was flushed by effect on server. */
	_startcall(client2);
	XFreePixmap(client2, pm);
	XSync(client2, True);
	_endcall(client2);
	if (geterr() != Success) {
		report("The output buffer was not flushed.");
		XFreePixmap(display, pm);
		FAIL;
	}
	else
		CHECK;
	CHECKPASS(3);

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
 * $XConsortium: mtnntfy.m,v 1.17 94/04/17 21:07:56 rws Exp $
 */
>>TITLE MotionNotify CH08
>>EXTERN
#define	MASK		PointerMotionMask
#define	HINTMASK	PointerMotionHintMask
#define EVENT		MotionNotify
#define EVENTMASK	MASK
>>ASSERTION Good A
When the pointer is moved
and the pointer motion begins and ends in the same window,
then a xname event is generated.
>>STRATEGY
Create client2.
Create window.
Move pointer to inside of window.
Set PointerMotionMask event mask bits on window.
Set PointerMotionMask event mask bits on window with client2.
Synthesize expected event.
Call XWarpPointer to move the pointer with motion beginning and
ending in window.
Verify that a MotionNotify event was received.
Verify event members.
Verify that only one MotionNotify event was received.
Verify that no other events were received.
Verify that a MotionNotify event was received by client2.
Verify event members for client2.
Verify that only one MotionNotify event was received.
Verify that no other events were received.
Call XWarpPointer to move the pointer multiple times with
motion beginning and ending in window.
Verify that multiple MotionNotify events were received.
Verify that no other events were received.
>>CODE
int	i;
int	nummoves;
Display	*display = Dsp;
Display	*client2;
Window	w;
int	x, y;
XEvent	event_return;
XMotionEvent	good;
PointerPlace	*warp;

/* Create client2. */
	client2 = opendisplay();
	if (client2 == (Display *) NULL) {
		delete("Can't open display");
		return;
	}
	else
		CHECK;
/* Create window. */
	w = mkwin(display, (XVisualInfo *) NULL, (struct area *) NULL, True);
/* Move pointer to inside of window. */
	warp = warppointer(display, w, 0, 0);
	if (warp == (PointerPlace *) NULL)
		return;
	else
		CHECK;
/* Set PointerMotionMask event mask bits on window. */
	XSelectInput(display, w, MASK);
	XSync(display, True);
/* Set PointerMotionMask event mask bits on window with client2. */
	XSelectInput(client2, w, MASK);
	XSync(client2, True);
/* Synthesize expected event. */
	x = 2;
	y = 2;
	good.type = MotionNotify;
	/* ignore serial */
	good.send_event = False;
	good.display = display;
	good.window = w;
	good.root = DRW(display);
	good.subwindow = None;
	/* ignore time */
	good.x = x;
	good.y = y;
	ROOTCOORDSET(display, &good);	/* x_root and y_root */
	good.state = 0;
	good.is_hint = NotifyNormal;
	good.same_screen = True;
/* Call XWarpPointer to move the pointer with motion beginning and */
/* ending in window. */
	XWarpPointer(display, None, w, 0, 0, 0, 0, x, y);
/* Verify that a MotionNotify event was received. */
	XSync(display, False);
	if (!XCheckTypedWindowEvent(display, w, MotionNotify, &event_return)) {
		report("No events delivered.");
		FAIL;
	}
	else
		CHECK;
/* Verify event members. */
	/* ignore serial */
	good.serial = ((XMotionEvent *) &event_return)->serial = 0;
	/* ignore time */
	good.time = ((XMotionEvent *) &event_return)->time = 0;
	CHECKEVENT((XEvent *) &good, &event_return);
/* Verify that only one MotionNotify event was received. */
	if (XCheckTypedWindowEvent(display, w, MotionNotify, &event_return)) {
		report("Excess events generated.");
		FAIL;
	}
	else
		CHECK;
/* Verify that no other events were received. */
	if (XPending(display) > 0) {
		delete("Unexpected events generated.");
		return;
	}
	else
		CHECK;
/* Verify that a MotionNotify event was received by client2. */
	XSync(client2, False);
	if (!XCheckTypedWindowEvent(client2, w, MotionNotify, &event_return)) {
		report("No events delivered to client2.");
		FAIL;
	}
	else
		CHECK;
/* Verify event members for client2. */
	good.display = client2;
	/* ignore serial */
	good.serial = ((XMotionEvent *) &event_return)->serial = 0;
	/* ignore time */
	good.time = ((XMotionEvent *) &event_return)->time = 0;
	CHECKEVENT((XEvent *) &good, &event_return);
/* Verify that only one MotionNotify event was received. */
	if (XCheckTypedWindowEvent(client2, w, MotionNotify, &event_return)) {
		report("Excess events generated for client2.");
		FAIL;
	}
	else
		CHECK;
/* Verify that no other events were received. */
	if (XPending(client2) > 0) {
		delete("Unexpected events generated for client2.");
		return;
	}
	else
		CHECK;
/* Call XWarpPointer to move the pointer multiple times with */
/* motion beginning and ending in window. */
	XSync(display, True);
	nummoves = 5;
	for (i=0; i<nummoves; i++) {
		if (!i)
			CHECK;
		XWarpPointer(display, None, w, 0, 0, 0, 0, ++x, ++y);
		/*
		 * Some servers may only generate the expected multiple
		 * MotionNotify events when this call to XWarpPointer is
		 * followed by an XSync (or probably any other protocol
		 * request).
		 */
	}
/* Verify that multiple MotionNotify events were received. */
	XSync(display, False);
	for (i=0; i<nummoves; i++) {
		if (!XCheckTypedWindowEvent(display, w, MotionNotify, &event_return)) {
			report("Missing %d events.", nummoves - i);
			FAIL;
			break;
		}
		else
			CHECK;
	}
/* Verify that no other events were received. */
	if (XPending(display) > 0) {
		delete("Unexpected events generated.");
		return;
	}
	else
		CHECK;

	CHECKPASS(12 + nummoves);
>>ASSERTION def
When a xname event is generated,
then all clients having set
.S PointerMotionMask
event mask bits on the event window are delivered
a xname event.
>>ASSERTION Good B 1
When a xname event is generated while pointer button 1 was pressed,
then all clients having set
.S Button1MotionMask
event mask bits on the event window are delivered
a xname event.
>>STRATEGY
If extended testing is required:
  Verify that at least one button is supported.
  Create a second client.
  Create a third client.
  Create a window.
  Select Button1MotionMask as the event mask for all clients.
  Simulate a Motion event with Button1 depressed on the window.
  Verify a xname event was generated for the first client.
  Verify that event member fields were correctly set.
  Verify a xname event was generated for the second client.
  Verify that event member fields were correctly set.
  Verify a xname event was generated for the third client.
  Verify that event member fields were correctly set.
>>CODE
Display			*client2;
Display			*client3;
Window			w;
XEvent			ev;
XPointerMovedEvent	good;
PointerPlace		*ptr;

	if(noext(1))
		return;

	/* If extended testing is required: */

		/* Verify that at least one button is supported. */
	if(nbuttons() < 1) {
		delete("No buttons are supported.");
		return;
	} else
		CHECK;

		/* Create a second client. */
	client2 = opendisplay();
		/* Create a third client. */
	client3 = opendisplay();

		/* Create a window. */
	w = defwin(Dsp);

		/* Select Button1MotionMask as the event mask for all clients. */
	XSelectInput(Dsp, w, Button1MotionMask);
	XSelectInput(client2, w, Button1MotionMask);
	XSelectInput(client3, w, Button1MotionMask);

	(void) warppointer(Dsp, w, 0,0);
	XSync(Dsp, True);
	XSync(client2, True);
	XSync(client3, True);

		/* Simulate a Motion event with Button1 depressed on the window. */
	buttonpress(Dsp, Button1);
	ptr = warppointer(Dsp, w, 10,10);
	XSync(Dsp, False);
	XSync(client2, False);
	XSync(client3, False);
	buttonrel(Dsp, Button1);

	good = ev.xmotion;
	good.type = EVENT;
	good.send_event = False;
	good.display = Dsp;
	good.window = w;
	good.root = DRW(Dsp);
	good.subwindow = None;

	good.x = 10;
	good.y = 10;
	good.x_root = ptr->nx;
	good.y_root = ptr->ny;
	good.state = Button1Mask;
	good.is_hint = NotifyNormal;
	good.same_screen = True;

		/* Verify a xname event was generated for the first client. */
	if(XCheckWindowEvent(Dsp, w, EVENTMASK, &ev) == False) {
		report("Expected %s event was not generated.", eventname(EVENT));
		FAIL;
	} else {
		CHECK;

		/* Verify that event member fields were correctly set. */
		if (checkevent((XEvent *) &good, &ev)) {
			report("Unexpected values in delivered event");
			FAIL;
		} else
			CHECK;
	}

		/* Verify a xname event was generated for the second client. */
	if(XCheckWindowEvent(client2, w, EVENTMASK, &ev) == False) {
		report("Expected %s event was not generated.", eventname(EVENT));
		FAIL;
	} else {
		CHECK;
		
		/* Verify that event member fields were correctly set. */
		good.display = client2;
		if (checkevent((XEvent *) &good, &ev)) {
			report("Unexpected values in delivered event");
			FAIL;
		} else
			CHECK;
	}

		/* Verify a xname event was generated for the third client. */
	if(XCheckWindowEvent(client3, w, EVENTMASK, &ev) == False) {
		report("Expected %s event was not generated.", eventname(EVENT));
		FAIL;
	} else {
		CHECK;
		
		/* Verify that event member fields were correctly set. */
		good.display = client3;
		if (checkevent((XEvent *) &good, &ev)) {
			report("Unexpected values in delivered event");
			FAIL;
		} else
			CHECK;
	}

	CHECKPASS(7);

>>ASSERTION Good D 1
When a xname event is generated while pointer button 2 was pressed,
then all clients having set
.S Button2MotionMask
event mask bits on the event window are delivered
a xname event.
>>STRATEGY
If extended testing is required:
  Verify that the server supports at least 2 buttons.
  Create a second client.
  Create a third client.
  Create a window.
  Select Button2MotionMask as the event mask for all clients.
  Simulate a Motion event with Button2 depressed on the window.
  Verify a xname event was generated for the first client.
  Verify that event member fields were correctly set.
  Verify a xname event was generated for the second client.
  Verify that event member fields were correctly set.
  Verify a xname event was generated for the third client.
  Verify that event member fields were correctly set.
>>CODE
Display			*client2;
Display			*client3;
Window			w;
int			b;
XEvent			ev;
XPointerMovedEvent	good;
PointerPlace		*ptr;

	/* If extended testing is required: */
	if(noext(1))
		return;

		/* Verify that the server supports at least 2 buttons. */
	if((b = nbuttons()) < 2) {
		unsupported("Server supports only %d buttons.", b);
		return;
	} else
		CHECK;

		/* Create a second client. */
	client2 = opendisplay();
		/* Create a third client. */
	client3 = opendisplay();

		/* Create a window. */
	w = defwin(Dsp);

		/* Select Button2MotionMask as the event mask for all clients. */
	XSelectInput(Dsp, w, Button2MotionMask);
	XSelectInput(client2, w, Button2MotionMask);
	XSelectInput(client3, w, Button2MotionMask);

	(void) warppointer(Dsp, w, 0,0);
	XSync(Dsp, True);
	XSync(client2, True);
	XSync(client3, True);

		/* Simulate a Motion event with Button2 depressed on the window. */
	buttonpress(Dsp, Button2);
	ptr = warppointer(Dsp, w, 10,10);
	XSync(Dsp, False);
	XSync(client2, False);
	XSync(client3, False);
	buttonrel(Dsp, Button2);

	good = ev.xmotion;
	good.type = EVENT;
	good.send_event = False;
	good.display = Dsp;
	good.window = w;
	good.root = DRW(Dsp);
	good.subwindow = None;

	good.x = 10;
	good.y = 10;
	good.x_root = ptr->nx;
	good.y_root = ptr->ny;
	good.state = Button2Mask;
	good.is_hint = NotifyNormal;
	good.same_screen = True;

		/* Verify a xname event was generated for the first client. */
	if(XCheckWindowEvent(Dsp, w, EVENTMASK, &ev) == False) {
		report("Expected %s event was not generated.", eventname(EVENT));
		FAIL;
	} else {
		CHECK;

		/* Verify that event member fields were correctly set. */
		if (checkevent((XEvent *) &good, &ev)) {
			report("Unexpected values in delivered event");
			FAIL;
		} else
			CHECK;
	}

		/* Verify a xname event was generated for the second client. */
	if(XCheckWindowEvent(client2, w, EVENTMASK, &ev) == False) {
		report("Expected %s event was not generated.", eventname(EVENT));
		FAIL;
	} else {
		CHECK;
		
		/* Verify that event member fields were correctly set. */
		good.display = client2;
		if (checkevent((XEvent *) &good, &ev)) {
			report("Unexpected values in delivered event");
			FAIL;
		} else
			CHECK;
	}

		/* Verify a xname event was generated for the third client. */
	if(XCheckWindowEvent(client3, w, EVENTMASK, &ev) == False) {
		report("Expected %s event was not generated.", eventname(EVENT));
		FAIL;
	} else {
		CHECK;
		
		/* Verify that event member fields were correctly set. */
		good.display = client3;
		if (checkevent((XEvent *) &good, &ev)) {
			report("Unexpected values in delivered event");
			FAIL;
		} else
			CHECK;
	}

	CHECKPASS(7);

>>ASSERTION Good D 1
When a xname event is generated while pointer button 3 was pressed,
then all clients having set
.S Button3MotionMask
event mask bits on the event window are delivered
a xname event.
>>STRATEGY
If extended testing is required:
  Verify that the server supports at least 3 buttons.
  Create a second client.
  Create a third client.
  Create a window.
  Select Button3MotionMask as the event mask for all clients.
  Simulate a Motion event with Button3 depressed on the window.
  Verify a xname event was generated for the first client.
  Verify that event member fields were correctly set.
  Verify a xname event was generated for the second client.
  Verify that event member fields were correctly set.
  Verify a xname event was generated for the third client.
  Verify that event member fields were correctly set.
>>CODE
Display			*client2;
Display			*client3;
Window			w;
int 			b;
XEvent			ev;
XPointerMovedEvent	good;
PointerPlace		*ptr;

	/* If extended testing is required: */
	if(noext(1))
		return;

		/* Verify that the server supports at least 3 buttons. */
	if((b = nbuttons()) < 3) {
		unsupported("Server supports only %d buttons.", b);
		return;
	} else
		CHECK;

		/* Create a second client. */
	client2 = opendisplay();
		/* Create a third client. */
	client3 = opendisplay();

		/* Create a window. */
	w = defwin(Dsp);

		/* Select Button3MotionMask as the event mask for all clients. */
	XSelectInput(Dsp, w, Button3MotionMask);
	XSelectInput(client2, w, Button3MotionMask);
	XSelectInput(client3, w, Button3MotionMask);

	(void) warppointer(Dsp, w, 0,0);
	XSync(Dsp, True);
	XSync(client2, True);
	XSync(client3, True);

		/* Simulate a Motion event with Button3 depressed on the window. */
	buttonpress(Dsp, Button3);
	ptr = warppointer(Dsp, w, 10,10);
	XSync(Dsp, False);
	XSync(client2, False);
	XSync(client3, False);
	buttonrel(Dsp, Button3);

	good = ev.xmotion;
	good.type = EVENT;
	good.send_event = False;
	good.display = Dsp;
	good.window = w;
	good.root = DRW(Dsp);
	good.subwindow = None;

	good.x = 10;
	good.y = 10;
	good.x_root = ptr->nx;
	good.y_root = ptr->ny;
	good.state = Button3Mask;
	good.is_hint = NotifyNormal;
	good.same_screen = True;

		/* Verify a xname event was generated for the first client. */
	if(XCheckWindowEvent(Dsp, w, EVENTMASK, &ev) == False) {
		report("Expected %s event was not generated.", eventname(EVENT));
		FAIL;
	} else {
		CHECK;

		/* Verify that event member fields were correctly set. */
		if (checkevent((XEvent *) &good, &ev)) {
			report("Unexpected values in delivered event");
			FAIL;
		} else
			CHECK;
	}

		/* Verify a xname event was generated for the second client. */
	if(XCheckWindowEvent(client2, w, EVENTMASK, &ev) == False) {
		report("Expected %s event was not generated.", eventname(EVENT));
		FAIL;
	} else {
		CHECK;
		
		/* Verify that event member fields were correctly set. */
		good.display = client2;
		if (checkevent((XEvent *) &good, &ev)) {
			report("Unexpected values in delivered event");
			FAIL;
		} else
			CHECK;
	}

		/* Verify a xname event was generated for the third client. */
	if(XCheckWindowEvent(client3, w, EVENTMASK, &ev) == False) {
		report("Expected %s event was not generated.", eventname(EVENT));
		FAIL;
	} else {
		CHECK;
		
		/* Verify that event member fields were correctly set. */
		good.display = client3;
		if (checkevent((XEvent *) &good, &ev)) {
			report("Unexpected values in delivered event");
			FAIL;
		} else
			CHECK;
	}

	CHECKPASS(7);

>>ASSERTION Good D 1
When a xname event is generated while pointer button 4 was pressed,
then all clients having set
.S Button4MotionMask
event mask bits on the event window are delivered
a xname event.
>>STRATEGY
If extended testing is required:
  Verify that the server supports at least 4 buttons.
  Create a second client.
  Create a third client.
  Create a window.
  Select Button4MotionMask as the event mask for all clients.
  Simulate a Motion event with Button4 depressed on the window.
  Verify a xname event was generated for the first client.
  Verify that event member fields were correctly set.
  Verify a xname event was generated for the second client.
  Verify that event member fields were correctly set.
  Verify a xname event was generated for the third client.
  Verify that event member fields were correctly set.
>>CODE
Display			*client2;
Display			*client3;
Window			w;
int			b;
XEvent			ev;
XPointerMovedEvent	good;
PointerPlace		*ptr;

	/* If extended testing is required: */
	if(noext(1))
		return;

		/* Verify that the server supports at least 4 buttons. */
	if((b = nbuttons()) < 4) {
		unsupported("Server supports only %d buttons.", b);
		return;
	} else
		CHECK;

		/* Create a second client. */
	client2 = opendisplay();
		/* Create a third client. */
	client3 = opendisplay();

		/* Create a window. */
	w = defwin(Dsp);

		/* Select Button4MotionMask as the event mask for all clients. */
	XSelectInput(Dsp, w, Button4MotionMask);
	XSelectInput(client2, w, Button4MotionMask);
	XSelectInput(client3, w, Button4MotionMask);

	(void) warppointer(Dsp, w, 0,0);
	XSync(Dsp, True);
	XSync(client2, True);
	XSync(client3, True);

		/* Simulate a Motion event with Button4 depressed on the window. */
	buttonpress(Dsp, Button4);
	ptr = warppointer(Dsp, w, 10,10);
	XSync(Dsp, False);
	XSync(client2, False);
	XSync(client3, False);
	buttonrel(Dsp, Button4);

	good = ev.xmotion;
	good.type = EVENT;
	good.send_event = False;
	good.display = Dsp;
	good.window = w;
	good.root = DRW(Dsp);
	good.subwindow = None;

	good.x = 10;
	good.y = 10;
	good.x_root = ptr->nx;
	good.y_root = ptr->ny;
	good.state = Button4Mask;
	good.is_hint = NotifyNormal;
	good.same_screen = True;

		/* Verify a xname event was generated for the first client. */
	if(XCheckWindowEvent(Dsp, w, EVENTMASK, &ev) == False) {
		report("Expected %s event was not generated.", eventname(EVENT));
		FAIL;
	} else {
		CHECK;

		/* Verify that event member fields were correctly set. */
		if (checkevent((XEvent *) &good, &ev)) {
			report("Unexpected values in delivered event");
			FAIL;
		} else
			CHECK;
	}

		/* Verify a xname event was generated for the second client. */
	if(XCheckWindowEvent(client2, w, EVENTMASK, &ev) == False) {
		report("Expected %s event was not generated.", eventname(EVENT));
		FAIL;
	} else {
		CHECK;
		
		/* Verify that event member fields were correctly set. */
		good.display = client2;
		if (checkevent((XEvent *) &good, &ev)) {
			report("Unexpected values in delivered event");
			FAIL;
		} else
			CHECK;
	}

		/* Verify a xname event was generated for the third client. */
	if(XCheckWindowEvent(client3, w, EVENTMASK, &ev) == False) {
		report("Expected %s event was not generated.", eventname(EVENT));
		FAIL;
	} else {
		CHECK;
		
		/* Verify that event member fields were correctly set. */
		good.display = client3;
		if (checkevent((XEvent *) &good, &ev)) {
			report("Unexpected values in delivered event");
			FAIL;
		} else
			CHECK;
	}

	CHECKPASS(7);

>>ASSERTION Good D 1
When a xname event is generated while pointer button 5 was pressed,
then all clients having set
.S Button5MotionMask
event mask bits on the event window are delivered
a xname event.
>>STRATEGY
If extended testing is required:
  Verify that the server supports 5 buttons.
  Create a second client.
  Create a third client.
  Create a window.
  Select Button5MotionMask as the event mask for all clients.
  Simulate a Motion event with Button5 depressed on the window.
  Verify a xname event was generated for the first client.
  Verify that event member fields were correctly set.
  Verify a xname event was generated for the second client.
  Verify that event member fields were correctly set.
  Verify a xname event was generated for the third client.
  Verify that event member fields were correctly set.
>>CODE
Display			*client2;
Display			*client3;
Window			w;
int			b;
XEvent			ev;
XPointerMovedEvent	good;
PointerPlace		*ptr;

	/* If extended testing is required: */
	if(noext(1))
		return;

		/* Verify that the server supports 5 buttons. */
	if((b = nbuttons()) < 5) {
		unsupported("Server supports only %d buttons.", b);
		return;
	} else
		CHECK;

		/* Create a second client. */
	client2 = opendisplay();
		/* Create a third client. */
	client3 = opendisplay();

		/* Create a window. */
	w = defwin(Dsp);

		/* Select Button5MotionMask as the event mask for all clients. */
	XSelectInput(Dsp, w, Button5MotionMask);
	XSelectInput(client2, w, Button5MotionMask);
	XSelectInput(client3, w, Button5MotionMask);

	(void) warppointer(Dsp, w, 0,0);
	XSync(Dsp, True);
	XSync(client2, True);
	XSync(client3, True);

		/* Simulate a Motion event with Button5 depressed on the window. */
	buttonpress(Dsp, Button5);
	ptr = warppointer(Dsp, w, 10,10);
	XSync(Dsp, False);
	XSync(client2, False);
	XSync(client3, False);
	buttonrel(Dsp, Button5);

	good = ev.xmotion;
	good.type = EVENT;
	good.send_event = False;
	good.display = Dsp;
	good.window = w;
	good.root = DRW(Dsp);
	good.subwindow = None;

	good.x = 10;
	good.y = 10;
	good.x_root = ptr->nx;
	good.y_root = ptr->ny;
	good.state = Button5Mask;
	good.is_hint = NotifyNormal;
	good.same_screen = True;

		/* Verify a xname event was generated for the first client. */
	if(XCheckWindowEvent(Dsp, w, EVENTMASK, &ev) == False) {
		report("Expected %s event was not generated.", eventname(EVENT));
		FAIL;
	} else {
		CHECK;

		/* Verify that event member fields were correctly set. */
		if (checkevent((XEvent *) &good, &ev)) {
			report("Unexpected values in delivered event");
			FAIL;
		} else
			CHECK;
	}

		/* Verify a xname event was generated for the second client. */
	if(XCheckWindowEvent(client2, w, EVENTMASK, &ev) == False) {
		report("Expected %s event was not generated.", eventname(EVENT));
		FAIL;
	} else {
		CHECK;
		
		/* Verify that event member fields were correctly set. */
		good.display = client2;
		if (checkevent((XEvent *) &good, &ev)) {
			report("Unexpected values in delivered event");
			FAIL;
		} else
			CHECK;
	}

		/* Verify a xname event was generated for the third client. */
	if(XCheckWindowEvent(client3, w, EVENTMASK, &ev) == False) {
		report("Expected %s event was not generated.", eventname(EVENT));
		FAIL;
	} else {
		CHECK;
		
		/* Verify that event member fields were correctly set. */
		good.display = client3;
		if (checkevent((XEvent *) &good, &ev)) {
			report("Unexpected values in delivered event");
			FAIL;
		} else
			CHECK;
	}

	CHECKPASS(7);

>>ASSERTION Good B 1
When a xname event is generated while at least one
pointer button was pressed,
then all clients having set
.S ButtonMotionMask
event mask bits on the event window are delivered
a xname event.
>>STRATEGY
If extended testing is required:
  Verify that at least one button is supported.
  Create a second client.
  Create a third client.
  Create a window.
  Select ButtonMotionMask as the event mask for all clients.
  Simulate a Motion event with Button1 depressed on the window.
  Verify a xname event was generated for the first client.
  Verify that event member fields were correctly set.
  Verify a xname event was generated for the second client.
  Verify that event member fields were correctly set.
  Verify a xname event was generated for the third client.
  Verify that event member fields were correctly set.
>>CODE
Display			*client2;
Display			*client3;
Window			w;
XEvent			ev;
XPointerMovedEvent	good;
PointerPlace		*ptr;

	/* If extended testing is required: */
	if(noext(1))
		return;

		/* Verify that at least one button is supported. */
	if(nbuttons() < 1) {
		delete("No buttons are supported.");
		return;
	} else
		CHECK;

		/* Create a second client. */
	client2 = opendisplay();
		/* Create a third client. */
	client3 = opendisplay();

		/* Create a window. */
	w = defwin(Dsp);

		/* Select ButtonMotionMask as the event mask for all clients. */
	XSelectInput(Dsp, w, ButtonMotionMask);
	XSelectInput(client2, w, ButtonMotionMask);
	XSelectInput(client3, w, ButtonMotionMask);

	(void) warppointer(Dsp, w, 0,0);
	XSync(Dsp, True);
	XSync(client2, True);
	XSync(client3, True);

		/* Simulate a Motion event with Button1 depressed on the window. */
	buttonpress(Dsp, Button1);
	ptr = warppointer(Dsp, w, 10,10);
	XSync(Dsp, False);
	XSync(client2, False);
	XSync(client3, False);
	buttonrel(Dsp, Button1);

	good = ev.xmotion;
	good.type = EVENT;
	good.send_event = False;
	good.display = Dsp;
	good.window = w;
	good.root = DRW(Dsp);
	good.subwindow = None;

	good.x = 10;
	good.y = 10;
	good.x_root = ptr->nx;
	good.y_root = ptr->ny;
	good.state = Button1Mask;
	good.is_hint = NotifyNormal;
	good.same_screen = True;

		/* Verify a xname event was generated for the first client. */
	if(XCheckWindowEvent(Dsp, w, EVENTMASK, &ev) == False) {
		report("Expected %s event was not generated.", eventname(EVENT));
		FAIL;
	} else {
		CHECK;

		/* Verify that event member fields were correctly set. */
		if (checkevent((XEvent *) &good, &ev)) {
			report("Unexpected values in delivered event");
			FAIL;
		} else
			CHECK;
	}

		/* Verify a xname event was generated for the second client. */
	if(XCheckWindowEvent(client2, w, EVENTMASK, &ev) == False) {
		report("Expected %s event was not generated.", eventname(EVENT));
		FAIL;
	} else {
		CHECK;
		
		/* Verify that event member fields were correctly set. */
		good.display = client2;
		if (checkevent((XEvent *) &good, &ev)) {
			report("Unexpected values in delivered event");
			FAIL;
		} else
			CHECK;
	}

		/* Verify a xname event was generated for the third client. */
	if(XCheckWindowEvent(client3, w, EVENTMASK, &ev) == False) {
		report("Expected %s event was not generated.", eventname(EVENT));
		FAIL;
	} else {
		CHECK;
		
		/* Verify that event member fields were correctly set. */
		good.display = client3;
		if (checkevent((XEvent *) &good, &ev)) {
			report("Unexpected values in delivered event");
			FAIL;
		} else
			CHECK;
	}

	CHECKPASS(7);

>>ASSERTION Good B 1
When a xname event is generated
and a client has selected one or more of
.S Button1MotionMask ,
.S Button2MotionMask ,
.S Button3MotionMask ,
.S Button4MotionMask ,
.S Button5MotionMask ,
.S ButtonMotionMask ,
or
.S PointerMotionMask
event mask bits on the event window,
then only one xname event is delivered to that client.
>>EXTERN
unsigned long		motionmasks[7] = { 	Button1MotionMask, Button2MotionMask,
						Button3MotionMask, Button4MotionMask, Button5MotionMask,
						ButtonMotionMask, PointerMotionMask };

#define MOTIONMASK 	Button1MotionMask | Button2MotionMask | \
			Button3MotionMask | Button4MotionMask | Button5MotionMask | \
			ButtonMotionMask | PointerMotionMask
>>STRATEGY
If extended testing is required:
  Verify that at least one button is supported.
  Create a window.
  Select all the motion masks on the window.
  Simulate a Motion event with Button1 depressed on the window.
  Check that only one of the possible events was generated.
  Simulate a Motion event with all buttons pressed.
  Check that only one of the possible events was generated.
>>CODE
int		i;
int		b;
int		nevents = 0;
Window		w;
XEvent		ev;

	/* If extended testing is required: */
	if(noext(1))
		return;

		/* Verify that at least one button is supported. */
	if((b = nbuttons()) < 1) {
		delete("No buttons are supported.");
		return;
	} else
		CHECK;

		/* Create a window. */
	w = defwin(Dsp);

		/* Select all the motion masks on the window. */
	XSelectInput(Dsp, w, MOTIONMASK);

		/* Simulate a Motion event with Button1 depressed on the window. */
	(void) warppointer(Dsp, w, 0,0);
	XSync(Dsp, True);
	buttonpress(Dsp, Button1);
	(void) warppointer(Dsp, w, 10, 10);
	XSync(Dsp, False);
	buttonrel(Dsp, Button1);

	for(i=0; i < NELEM(motionmasks); i++) {

		if(XCheckWindowEvent(Dsp, w, motionmasks[i], &ev) == False) {
			trace("%s event was not generated.", eventmaskname(motionmasks[i]));
		} else {
			nevents++;
			trace("%s event was generated.", eventmaskname(motionmasks[i]));
		}

	}

		/* Check that only one of the possible events was generated. */
	if(nevents != 1) {
		report("More than one of the selected motion events was generated.");
		FAIL;
	} else
		CHECK;

	nevents = 0;
		/* Simulate a Motion event with all buttons pressed. */
	buttonpress(Dsp, Button1);
	if(b>=2) 
		buttonpress(Dsp, Button2);

	if(b>=3) 
		buttonpress(Dsp, Button3);

	if(b>=4) 
		buttonpress(Dsp, Button4);

	if(b>=5) 
		buttonpress(Dsp, Button5);


	(void) warppointer(Dsp, w, 0,0);
	XSync(Dsp, True);
	(void) warppointer(Dsp, w, 10, 10);
	XSync(Dsp, False);

	for(i=0; i < NELEM(motionmasks); i++) {

		if(XCheckWindowEvent(Dsp, w, motionmasks[i], &ev) == False) {
			trace("Event selected by %s was not generated.", eventmaskname(motionmasks[i]));
		} else {
			nevents++;
			trace("Event selected by %s event was generated.", eventmaskname(motionmasks[i]));
		}

	}

	if(b>=2) 
		buttonrel(Dsp, Button2);

	if(b>=3) 
		buttonrel(Dsp, Button3);

	if(b>=4) 
		buttonrel(Dsp, Button4);

	if(b>=5) 
		buttonrel(Dsp, Button5);


		/* Check that only one of the possible events was generated. */
	if(nevents != 1) {
		report("More than one of the selected motion events was generated.");
		FAIL;
	} else
		CHECK;

	CHECKPASS(3);

>>EXTERN
unsigned long	buttonmask[7] = {	Button5MotionMask,Button4MotionMask,Button3MotionMask,
					Button2MotionMask,Button1MotionMask,ButtonMotionMask,PointerMotionMask};

unsigned int buttons[7] = { Button5, Button4, Button3, Button2, Button1, Button1, Button1 };	

>>ASSERTION Good B 1
When a xname event is generated,
then
clients not having set
.S Button1MotionMask ,
.S Button2MotionMask ,
.S Button3MotionMask ,
.S Button4MotionMask ,
.S Button5MotionMask ,
.S ButtonMotionMask ,
or
.S PointerMotionMask
event mask bits on the event window,
are not delivered
a xname event.
>>STRATEGY
If extended testing is required:
  Verify that at least one button is supported.
  Create a second client.
  Create a window.
  Select for something on the second client.
  For each supported type of motion event :
    Select the mask as the event mask for the first client.
    Simulate the motion event.
    Verify that the first client received the appropriate event.
    Verify that the second client did not receive that event.
>>CODE
Display			*client2;
Window			w;
XEvent			ev;
int			i;


	/* If extended testing is required: */
	if(noext(1))
		return;

		/* Verify that at least one button is supported. */
	if(nbuttons() < 1) {
		delete("No buttons are supported.");
		return;
	} else
		CHECK;

		/* Create a second client. */
	client2 = opendisplay();

		/* Create a window. */
	w = defwin(Dsp);

		/* Select for something on the second client. */
	XSelectInput(client2, w, EnterWindowMask);

		/* For each supported type of motion event : */
	for(i = 5-nbuttons(); i<NELEM(buttonmask); i++) {
		
		trace("Iteration : %d Event mask : %s Button : %s", i, eventmaskname(buttonmask[i]), buttonname((int)buttons[i]));

			/* Select the mask as the event mask for the first client. */
		XSelectInput(Dsp, w, buttonmask[i]);

			/* Simulate the motion event. */
		(void) warppointer(Dsp, w, 0,0);
		XSync(Dsp, False);
		XSync(client2, False);
		if(buttonmask[i] != PointerMotionMask)
			buttonpress(Dsp, buttons[i]);
		(void) warppointer(Dsp, w, 10,10);
		XSync(Dsp, False);
		XSync(client2, False);
			
			/* Verify that the first client received the appropriate event. */
		if(XCheckWindowEvent(Dsp, w, buttonmask[i], &ev) == False) {
			report("Expected %s event was not generated.", eventmaskname(buttonmask[i]));
			FAIL;
		} else {
			CHECK;

			/* Verify that the second client did not receive that event. */
		if(XCheckWindowEvent(client2, w, buttonmask[i], &ev) != False) {
			report("Unexpected %s event was generated.", eventmaskname(buttonmask[i]));
			FAIL;
		} else
			CHECK;
		}
		if(buttonmask[i] != PointerMotionMask)
			buttonrel(Dsp, buttons[i]);

	}

	CHECKPASS(1 + 2 * (nbuttons()+2));	

>>ASSERTION Good B 1
When a xname event is delivered with
.M is_hint
set to
.S NotifyHint ,
then clients which have set
.S PointerMotionHintMask
and one or more of
.S Button1MotionMask ,
.S Button2MotionMask ,
.S Button3MotionMask ,
.S Button4MotionMask ,
.S Button5MotionMask ,
.S ButtonMotionMask ,
or
.S PointerMotionMask
event mask bits on the event window may not be
delivered another xname event until either the key or button
state changes, the pointer leaves the event window or the client
calls
.S XQueryPointer
or
.S XGetMotionEvents .

>>ASSERTION def
When a xname event is generated, then all clients which have not set
.S PointerMotionHintMask 
and have set one or more of
.S Button1MotionMask ,
.S Button2MotionMask ,
.S Button3MotionMask ,
.S Button4MotionMask ,
.S Button5MotionMask ,
.S ButtonMotionMask ,
or
.S PointerMotionMask
event mask bits on the event window are delivered a
xname event with
.M is_hint
set to
.S NotifyNormal .

>>ASSERTION Good A
When a xname event is generated, then all clients which
have set 
.S PointerMotionHintMask 
and one or more of
.S Button1MotionMask ,
.S Button2MotionMask ,
.S Button3MotionMask ,
.S Button4MotionMask ,
.S Button5MotionMask ,
.S ButtonMotionMask ,
or
.S PointerMotionMask event mask bits on the event window are
delivered a xname event with 
.M is_hint 
set to 
.S NotifyHint
or 
.S NotifyNormal .
>>STRATEGY
Create client2.
Create window.
Move pointer to inside of window.
Set PointerMotionMask and PointerMotionHintMask event mask bits on window.
Set PointerMotionMask and PointerMotionHintMask event mask bits on window
with client2.
Synthesize expected event.
Call XWarpPointer to move the pointer with motion beginning and
ending in window.
Verify that a MotionNotify event was received.
Verify event members.
Verify that only one MotionNotify event was received.
Verify that no other events were received.
Verify that a MotionNotify event was received by client2.
Verify event members for client2.
Verify that only one MotionNotify event was received.
Verify that no other events were received.
>>CODE
Display	*display = Dsp;
Display	*client2;
Window	w;
int	x, y;
XEvent	event_return;
XMotionEvent	good;
PointerPlace	*warp;

/* Create client2. */
	client2 = opendisplay();
	if (client2 == (Display *) NULL) {
		delete("Can't open display");
		return;
	}
	else
		CHECK;
/* Create window. */
	w = mkwin(display, (XVisualInfo *) NULL, (struct area *) NULL, True);
/* Move pointer to inside of window. */
	warp = warppointer(display, w, 0, 0);
	if (warp == (PointerPlace *) NULL)
		return;
	else
		CHECK;
/* Set PointerMotionMask and PointerMotionHintMask event mask bits on window. */
	XSelectInput(display, w, HINTMASK|MASK);
	XSync(display, True);
/* Set PointerMotionMask and PointerMotionHintMask event mask bits on window */
/* with client2. */
	XSelectInput(client2, w, HINTMASK|MASK);
	XSync(client2, True);
/* Synthesize expected event. */
	x = 2;
	y = 2;
	good.type = MotionNotify;
	/* ignore serial */
	good.send_event = False;
	good.display = display;
	good.window = w;
	good.root = DRW(display);
	good.subwindow = None;
	/* ignore time */
	good.x = x;
	good.y = y;
	ROOTCOORDSET(display, &good);	/* x_root and y_root */
	good.state = 0;
	good.is_hint = NotifyNormal;
	good.same_screen = True;
/* Call XWarpPointer to move the pointer with motion beginning and */
/* ending in window. */
	XWarpPointer(display, None, w, 0, 0, 0, 0, x, y);
/* Verify that a MotionNotify event was received. */
	XSync(display, False);
	if (!XCheckTypedWindowEvent(display, w, MotionNotify, &event_return)) {
		report("No events delivered.");
		FAIL;
	}
	else
		CHECK;
/* Verify is_hint is set to NotifyNormal or Notify Hint */
        if(((XMotionEvent *) &event_return)->is_hint != NotifyNormal &&
	   ((XMotionEvent *) &event_return)->is_hint != NotifyHint) {
		report("is_hint was set to 0x%x, expected 0x%x or 0x%x", 
				((XMotionEvent *) &event_return)->is_hint, 
				NotifyNormal, NotifyHint);
		FAIL;
	}
	else
		CHECK;

        /* is_hint has been checked so don't bother reporting again */
        good.is_hint = event_return.xmotion.is_hint;

/* Verify event members. */
	/* ignore serial */
	good.serial = ((XMotionEvent *) &event_return)->serial = 0;
	/* ignore time */
	good.time = ((XMotionEvent *) &event_return)->time = 0;
	if (checkevent((XEvent *) &good, &event_return)) {
		report("Delivered event did not match expected event");
		FAIL;
	}
	else
		CHECK;
/* Verify that only one MotionNotify event was received. */
	if (XCheckTypedWindowEvent(display, w, MotionNotify, &event_return)) {
		report("Excess events generated.");
		FAIL;
	}
	else
		CHECK;
/* Verify that no other events were received. */
	if (XPending(display) > 0) {
		delete("Unexpected events generated.");
		return;
	}
	else
		CHECK;
/* Verify that a MotionNotify event was received by client2. */
	XSync(client2, False);
	if (!XCheckTypedWindowEvent(client2, w, MotionNotify, &event_return)) {
		report("No events delivered to client2.");
		FAIL;
	}
	else
		CHECK;
/* Verify is_hint is set to NotifyNormal or Notify Hint */
        if(((XMotionEvent *) &event_return)->is_hint != NotifyNormal &&
	   ((XMotionEvent *) &event_return)->is_hint != NotifyHint) {
		report("is_hint was set to 0x%x, expected 0x%x or 0x%x", 
				((XMotionEvent *) &event_return)->is_hint, 
				NotifyNormal, NotifyHint);
		FAIL;
	}
	else
		CHECK;

        /* is_hint has been checked so don't bother reporting again */
        good.is_hint = event_return.xmotion.is_hint;

/* Verify event members for client2. */
	good.display = client2;
	/* ignore serial */
	good.serial = ((XMotionEvent *) &event_return)->serial = 0;
	/* ignore time */
	good.time = ((XMotionEvent *) &event_return)->time = 0;
	if (checkevent((XEvent *) &good, &event_return)) {
		report("Delivered event did not match expected event");
		FAIL;
	}
	else
		CHECK;
/* Verify that only one MotionNotify event was received. */
	if (XCheckTypedWindowEvent(client2, w, MotionNotify, &event_return)) {
		report("Excess events generated for client2.");
		FAIL;
	}
	else
		CHECK;
/* Verify that no other events were received. */
	if (XPending(client2) > 0) {
		delete("Unexpected events generated for client2.");
		return;
	}
	else
		CHECK;

	CHECKPASS(12);

>>ASSERTION Good B 1
When a xname event is generated
and no client has selected
.S Button1MotionMask ,
.S Button2MotionMask ,
.S Button3MotionMask ,
.S Button4MotionMask ,
.S Button5MotionMask ,
.S ButtonMotionMask ,
or
.S PointerMotionMask
event mask bits
on the source window,
then the event propagates,
with propagation stopping at the root window of the screen or
at the first window with
.S Button1MotionMask ,
.S Button2MotionMask ,
.S Button3MotionMask ,
.S Button4MotionMask ,
.S Button5MotionMask ,
.S ButtonMotionMask ,
or
.S PointerMotionMask
event mask bits
in its do-not-propagate mask,
from the source window to
the first ancestor window for which
some client has selected for xname events.
>>STRATEGY
If extended is required:
  Verify that at least one button is supported.
  For each supported type of motion event :
    Create a window.
    Create a child of that window.
    Create a grandchild of the window.
    Select xname events on the root window of the screen.
    Simulate a xname event on the grandchild.
    Verify that a xname event was generated on the root.
    Select xname events on the grandparent window.
    Set the do_not_propagate mask on the child to xname events.
    Simulate a xname event on the grandchild.
    Verify no xname event was generated on the grandchild.
    Verify no xname event was generated on the child.
    Verify no xname event was generated on the parent.
    Select xname events on the child window.
    Set the do_not_propagate mask of the child to NoEventMask.
    Simulate a xname event on the grandchild.
    Verify that no xname event was generated on the grandchild.
    Verify that no xname event was generated on the parent.
    Verify that a xname event was generated on the child.
>>CODE
XEvent			ev;
Window			w;
Window			w1;
Window			w2;
XSetWindowAttributes	atts;
int			i;

/* If extended is required: */
	if(noext(0))
		return;

/* Verify that at least one button is supported. */
	if(nbuttons() < 1) {
		delete("No buttons are supported.");
		return;
	} else
		CHECK;

/* For each supported type of motion event : */
	for(i = 5-nbuttons(); i<NELEM(buttonmask); i++) {
		trace("Iteration : %d Event mask : %s Button : %s", i, eventmaskname(buttonmask[i]), buttonname((int)buttons[i]));

/* Create a window. */
		w = defwin(Dsp);
/* Create a child of that window. */
		w1 = crechild(Dsp, w, (struct area *) NULL);
/* Create a grandchild of the window. */
		w2 = crechild(Dsp, w1, (struct area *) NULL);

/* Select xname events on the root window of the screen. */
		XSelectInput(Dsp, DRW(Dsp), buttonmask[i]);	

/* Simulate a xname event on the grandchild. */
		(void) warppointer(Dsp, w2, 0,0);
		XSync(Dsp, False);
		if(buttonmask[i] != PointerMotionMask)
			buttonpress(Dsp, buttons[i]);
		(void) warppointer(Dsp, w2, 1,1);
		if(buttonmask[i] != PointerMotionMask)
			buttonrel(Dsp, buttons[i]);
		XSync(Dsp, False);
	
/* Verify that a xname event was generated on the root. */
		if (XCheckWindowEvent(Dsp, DRW(Dsp), buttonmask[i], &ev) == False) {
			report("Expected event (%s) not received.", eventmaskname(buttonmask[i]));
			FAIL;
		} else
			CHECK;

		XSelectInput(Dsp, DRW(Dsp), NoEventMask);	

/* Select xname events on the grandparent window. */
		XSelectInput(Dsp, w, buttonmask[i]);	

/* Set the do_not_propagate mask on the child to xname events. */
		atts.do_not_propagate_mask = buttonmask[i];
		XChangeWindowAttributes(Dsp, w1, CWDontPropagate, &atts);
	
/* Simulate a xname event on the grandchild. */
		(void) warppointer(Dsp, w2, 0,0);
		XSync(Dsp, False);
		if(buttonmask[i] != PointerMotionMask)
			buttonpress(Dsp, buttons[i]);
		(void) warppointer(Dsp, w2, 1,1);
		if(buttonmask[i] != PointerMotionMask)
			buttonrel(Dsp, buttons[i]);
		XSync(Dsp, False);

/* Verify no xname event was generated on the grandchild. */
		if (XCheckWindowEvent(Dsp, w2, buttonmask[i], &ev) != False) {
			report("Unexpected event (%s) received.", eventmaskname(buttonmask[i]));
			FAIL;
		} else
			CHECK;

/* Verify no xname event was generated on the child. */
		if (XCheckWindowEvent(Dsp, w1, buttonmask[i], &ev) != False) {
			report("Unexpected event (%s) received.", eventmaskname(buttonmask[i]));
			FAIL;
		} else
			CHECK;

/* Verify no xname event was generated on the parent. */
		if (XCheckWindowEvent(Dsp, w, buttonmask[i], &ev) != False) {
			report("Unexpected event (%s) received.", eventmaskname(buttonmask[i]));
			FAIL;
		} else
			CHECK;

/* Select xname events on the child window. */
		XSelectInput(Dsp, w1, buttonmask[i]);	

/* Set the do_not_propagate mask of the child to NoEventMask. */
		atts.do_not_propagate_mask = NoEventMask;
		XChangeWindowAttributes(Dsp, w1, CWDontPropagate, &atts);

/* Simulate a xname event on the grandchild. */
		(void) warppointer(Dsp, w2, 0,0);
		XSync(Dsp, False);
		if(buttonmask[i] != PointerMotionMask)
			buttonpress(Dsp, buttons[i]);
		(void) warppointer(Dsp, w2, 1,1);
		if(buttonmask[i] != PointerMotionMask)
			buttonrel(Dsp, buttons[i]);
		XSync(Dsp, False);

/* Verify that no xname event was generated on the grandchild. */
		if (XCheckWindowEvent(Dsp, w2, buttonmask[i], &ev) != False) {
			report("Unexpected event (%s) received.", eventmaskname(buttonmask[i]));
			FAIL;
		} else
			CHECK;

/* Verify that no xname event was generated on the parent. */
		if (XCheckWindowEvent(Dsp, w, buttonmask[i], &ev) != False) {
			report("Unexpected event (%s) received.", eventmaskname(buttonmask[i]));
			FAIL;
		} else
			CHECK;

/* Verify that a xname event was generated on the child. */
		if (XCheckWindowEvent(Dsp, w1, buttonmask[i], &ev) == False) {
			report("%s event was not delivered to selecting child window.", eventmaskname(buttonmask[i]));
			FAIL;
		} else
			CHECK;

	} /* for */


	CHECKPASS(1 + (nbuttons()+2)*7);	

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
>>#NOTEs .M root
>>#NOTEs is set to the source window's root window.
>>ASSERTION Good A
When a xname event is delivered
and the source window is an inferior of the event window
and the source window is a child of the event window,
then
.M subwindow
is set to
the source window.
>>STRATEGY
Create window hierarchy.
Create the hierarchy.
Move pointer to inside of window.
Select no events on the sourcew.
Set PointerMotionMask event mask bits on the eventw.
Call XWarpPointer to move the pointer with motion beginning and
ending in window.
Verify that a MotionNotify event was received.
Verify that subwindow is set to the source window.
>>CODE
int	status;
Display	*display = Dsp;
Winh	*eventw;
Winh	*sourcew;
XEvent	good;
Winhg	winhg;
PointerPlace	*warp;

/* Create window hierarchy. */
	winhg.area.x = 0;
	winhg.area.y = 0;
	winhg.area.width = W_STDWIDTH;
	winhg.area.height = W_STDHEIGHT;
	winhg.border_width = 1;
	eventw = winh_adopt(display, (Winh *) NULL, 0L, (XSetWindowAttributes *) NULL, &winhg, WINH_NOMASK);
	if (eventw == (Winh *) NULL) {
		delete("Could not create eventw");
		return;
	}
	else
		CHECK;
	winhg.area.width /= 2;
	winhg.area.height /= 2;
	sourcew = winh_adopt(display, eventw, 0L, (XSetWindowAttributes *) NULL, &winhg, WINH_NOMASK);
	if (sourcew == (Winh *) NULL) {
		delete("Could not create sourcew");
		return;
	}
	else
		CHECK;
/* Create the hierarchy. */
	if (winh_create(display, (Winh *) NULL, WINH_MAP))
		return;
	else
		CHECK;
/* Move pointer to inside of window. */
	warp = warppointer(display, sourcew->window, 0, 0);
	if (warp == (PointerPlace *) NULL)
		return;
	else
		CHECK;
/* Select no events on the sourcew. */
	if (winh_selectinput(display, sourcew, NoEventMask))
		return;
	else
		CHECK;
/* Set PointerMotionMask event mask bits on the eventw. */
	if (winh_selectinput(display, eventw, MASK))
		return;
	else
		CHECK;
	XSync(display, True);
	good.type = MotionNotify;
	good.xmotion.display = display;
	good.xmotion.window = eventw->window;
	good.xmotion.subwindow = sourcew->window;
/* Call XWarpPointer to move the pointer with motion beginning and */
/* ending in window. */
	XWarpPointer(display, None, sourcew->window, 0, 0, 0, 0, 2, 2);
/* Verify that a MotionNotify event was received. */
	XSync(display, False);
	if (winh_plant(sourcew, &good, NoEventMask, WINH_NOMASK))
		return;
	else
		CHECK;
	if (winh_harvest(display, (Winh *) NULL))
		return;
	else
		CHECK;
	status = winh_weed((Winh *) NULL, -1, WINH_WEED_IDENTITY);
	if (status < 0)
		return;
	else
		CHECK;
	if (status > 0) {
		report("Event delivery was not as expected");
		FAIL;
	}
	else {
/* Verify that subwindow is set to the source window. */
		/* since only one event was expected, it must be first in list */
		if (eventw->delivered->event->xmotion.subwindow != sourcew->window) {
			report("Subwindow set to 0x%x, expected 0x%x",
				eventw->delivered->event->xmotion.subwindow, sourcew->window);
			FAIL;
		}
		else
			CHECK;
	}

	CHECKPASS(10);
>>ASSERTION Good A
When a xname event is delivered
and the source window is an inferior of the event window
and the source window is not a child of the event window,
then
.M subwindow
is set to
the child of the event window that is
an ancestor of the source window.
>>STRATEGY
Create window hierarchy.
Create the hierarchy.
Move pointer to inside of window.
Select no events on the sourcew.
Set PointerMotionMask event mask bits on the eventw.
Call XWarpPointer to move the pointer with motion beginning and
ending in window.
Verify that a MotionNotify event was received.
Verify that subwindow is set to the source window.
>>CODE
int	status;
Display	*display = Dsp;
Winh	*eventw;
Winh	*ancestorw;
Winh	*sourcew;
XEvent	good;
Winhg	winhg;
PointerPlace	*warp;

/* Create window hierarchy. */
	winhg.area.x = 0;
	winhg.area.y = 0;
	winhg.area.width = W_STDWIDTH;
	winhg.area.height = W_STDHEIGHT;
	winhg.border_width = 1;
	eventw = winh_adopt(display, (Winh *) NULL, 0L, (XSetWindowAttributes *) NULL, &winhg, WINH_NOMASK);
	if (eventw == (Winh *) NULL) {
		delete("Could not create eventw");
		return;
	}
	else
		CHECK;
	winhg.area.width /= 2;
	winhg.area.height /= 2;
	ancestorw = winh_adopt(display, eventw, 0L, (XSetWindowAttributes *) NULL, &winhg, WINH_NOMASK);
	if (ancestorw == (Winh *) NULL) {
		delete("Could not create ancestorw");
		return;
	}
	else
		CHECK;
	winhg.area.width /= 2;
	winhg.area.height /= 2;
	sourcew = winh_adopt(display, ancestorw, 0L, (XSetWindowAttributes *) NULL, &winhg, WINH_NOMASK);
	if (sourcew == (Winh *) NULL) {
		delete("Could not create sourcew");
		return;
	}
	else
		CHECK;
/* Create the hierarchy. */
	if (winh_create(display, (Winh *) NULL, WINH_MAP))
		return;
	else
		CHECK;
/* Move pointer to inside of window. */
	warp = warppointer(display, sourcew->window, 0, 0);
	if (warp == (PointerPlace *) NULL)
		return;
	else
		CHECK;
/* Select no events on the sourcew. */
	if (winh_selectinput(display, sourcew, NoEventMask))
		return;
	else
		CHECK;
/* Set PointerMotionMask event mask bits on the eventw. */
	if (winh_selectinput(display, eventw, MASK))
		return;
	else
		CHECK;
	XSync(display, True);
	good.type = MotionNotify;
	good.xmotion.display = display;
	good.xmotion.window = eventw->window;
	good.xmotion.subwindow = ancestorw->window;
/* Call XWarpPointer to move the pointer with motion beginning and */
/* ending in window. */
	XWarpPointer(display, None, sourcew->window, 0, 0, 0, 0, 2, 2);
/* Verify that a MotionNotify event was received. */
	XSync(display, False);
	if (winh_plant(sourcew, &good, NoEventMask, WINH_NOMASK))
		return;
	else
		CHECK;
	if (winh_harvest(display, (Winh *) NULL))
		return;
	else
		CHECK;
	status = winh_weed((Winh *) NULL, -1, WINH_WEED_IDENTITY);
	if (status < 0)
		return;
	else
		CHECK;
	if (status > 0) {
		report("Event delivery was not as expected");
		FAIL;
	}
	else {
/* Verify that subwindow is set to the source window. */
		/* since only one event was expected, it must be first in list */
		if (eventw->delivered->event->xmotion.subwindow != ancestorw->window) {
			report("Subwindow set to 0x%x, expected 0x%x",
				eventw->delivered->event->xmotion.subwindow, ancestorw->window);
			FAIL;
		}
		else
			CHECK;
	}

	CHECKPASS(11);

>>ASSERTION def
When a xname event is delivered
and the source window is not an inferior of the event window,
then
.M subwindow
is set to
.S None .
>>#NOTEs >>ASSERTION
>>#NOTEs When ARTICLE xname event is delivered,
>>#NOTEs then
>>#NOTEs .M time
>>#NOTEs is set to
>>#NOTEs the time in milliseconds at which the event was generated.
>>#NOTEs >>ASSERTION
>>#NOTEs When ARTICLE xname event is delivered
>>#NOTEs and the event window is on the same screen as the root window,
>>#NOTEs then
>>#NOTEs .M x
>>#NOTEs and
>>#NOTEs .M y
>>#NOTEs are set to
>>#NOTEs the coordinates of
>>#NOTEs the final pointer position relative to the event window's origin.
>>ASSERTION Good D 1
If multiple screens are supported:
When a xname event is delivered
and the event and root windows are not on the same screen,
then
.M x
and
.M y
are set to
zero.
>>STRATEGY
If multiple screens are supported:
  If extended testing is required:
    Create a window on the default screen.
    Create a window on the alternate screen.
    Grab the pointer asynchronously  for the first window selecting PointerMotion and ButtonMotion events.
    Generate a PointerMotion event on the alternate window.
    Verify that a PointerMotion event was generated with respect to the grabbing window.
    Verify that the x and y event components were zero.
    Generate a ButtonMotion mask on the alternate window.
    Verify that a ButtonMotion event was generated with respect to the grabbing window. 
    Verify that the x and y event components were zero.
>>CODE
Window			w;
Window			w2;
XEvent			ev;

/* If multiple screens are supported: */
	if (config.alt_screen == -1) {
		unsupported("Multiple screens not supported.");
		return;
	} else
		CHECK;

/* If extended testing is required: */
	if(noext(1))
		return;

/* Create a window on the default screen. */
        w = defwin(Dsp);
/* Create a window on the alternate screen. */
	w2 = defdraw(Dsp, VI_ALT_WIN);
	
/* Grab the pointer asynchronously  for the first window selecting PointerMotion and ButtonMotion events. */
	if( XGrabPointer(Dsp, w, False, PointerMotionMask | ButtonMotionMask, GrabModeAsync, GrabModeAsync, None, None, CurrentTime) != GrabSuccess) {
		delete("XGrabPointer() did not return GrabSuccess.");
		return;
	} else
		CHECK;
	
/* Generate a PointerMotion event on the alternate window. */
	(void) warppointer(Dsp, w2, 0,0);
	XSync(Dsp, True);
	(void) warppointer(Dsp, w2, 10, 10);
	XSync(Dsp, False);

/* Verify that a PointerMotion event was generated with respect to the grabbing window. */
	if (XCheckWindowEvent(Dsp, w, PointerMotionMask, &ev) == False) {
		report("Expected event (%s) not received.", eventname(MotionNotify));
		FAIL;
	} else {
		CHECK;
/* Verify that the x and y event components were zero. */
		if(ev.xmotion.x != 0 || ev.xmotion.y != 0) {
			report("The x (value %d) and y (value %d) components of the %s event were not set to zero.",
				 ev.xmotion.x, ev.xmotion.y, eventname(EVENT));
			FAIL;
		} else 
			CHECK;
	}

/* Generate a ButtonMotion mask on the alternate window. */
	(void) warppointer(Dsp, w2, 0,0);
	XSync(Dsp, True);
	buttonpress(Dsp, Button1);
	(void) warppointer(Dsp, w2, 10, 10);
	XSync(Dsp, False);
	buttonrel(Dsp, Button1);

/* Verify that a ButtonMotion event was generated with respect to the grabbing window. */
	if (XCheckWindowEvent(Dsp, w, ButtonMotionMask, &ev) == False) {
		report("Expected event (%s) not received.", eventname(MotionNotify));
		FAIL;
	} else {
		CHECK;
/* Verify that the x and y event components were zero. */
		if(ev.xmotion.x != 0 || ev.xmotion.y != 0) {
			report("The x (value %d) and y (value %d) components of the %s event were not set to zero.",
				ev.xmotion.x, ev.xmotion.y, eventname(EVENT));
			FAIL;
		} else 
			CHECK;
	}
	
	XUngrabPointer(Dsp, CurrentTime);
	
	CHECKPASS(6);

>>#NOTEs >>ASSERTION
>>#NOTEs When ARTICLE xname event is delivered,
>>#NOTEs then
>>#NOTEs .M x_root
>>#NOTEs and
>>#NOTEs .M y_root
>>#NOTEs are set to coordinates of the pointer
>>#NOTEs when the event was generated
>>#NOTEs relative to the root window's origin.
>>#NOTEs >>ASSERTION
>>#NOTEs When ARTICLE xname event is delivered,
>>#NOTEs then
>>#NOTEs .M state
>>#NOTEs is set to
>>#NOTEs indicate the logical state
>>#NOTEs of the pointer buttons,
>>#NOTEs which is the bitwise OR of one or more of
>>#NOTEs the button or modifier key masks
>>#NOTEs .S Button1Mask ,
>>#NOTEs .S Button2Mask ,
>>#NOTEs .S Button3Mask ,
>>#NOTEs .S Button4Mask ,
>>#NOTEs .S Button5Mask ,
>>#NOTEs .S ShiftMask ,
>>#NOTEs .S LockMask ,
>>#NOTEs .S ControlMask ,
>>#NOTEs .S Mod1Mask ,
>>#NOTEs .S Mod2Mask ,
>>#NOTEs .S Mod3Mask ,
>>#NOTEs .S Mod4Mask ,
>>#NOTEs and
>>#NOTEs .S Mod5Mask .
>>#NOTEs >>ASSERTION
>>#NOTEs When ARTICLE xname event is delivered,
>>#NOTEs then
>>#NOTEs .M keycode
>>#NOTEs is set to
>>#NOTEs a number that represents the physical key on the keyboard.
>>#NOTEs >>ASSERTION
>>#NOTEs When ARTICLE xname event is delivered
>>#NOTEs and the event and root windows are on the same screen,
>>#NOTEs then
>>#NOTEs .M same_screen
>>#NOTEs is set to
>>#NOTEs .S True .
>>ASSERTION Good D 1
If multiple screens are supported:
When a xname event is delivered
and the event and root windows are not on the same screen,
then
.M same_screen
is set to
.S False .
>>STRATEGY
If multiple screens are supported:
  If extended testing is required:
    Create a window on the default screen.
    Create a window on the alternate screen.
    Grab the pointer asynchronously  for the first window selecting PointerMotion and ButtonMotion events.
    Generate a PointerMotion event on the alternate window.
    Verify that a PointerMotion event was generated with respect to the grabbing window.
    Verify that same_screen event component was False.
    Generate a ButtonMotion event on the alternate window.
    Verify that a ButtonMotion event was generated with respect to the grabbing window.
    Verify that same_screen event component was False.
>>CODE
Window			w;
Window			w2;
XEvent			ev;

/* If multiple screens are supported: */
	if (config.alt_screen == -1) {
		unsupported("Multiple screens not supported.");
		return;
	} else
		CHECK;

/* If extended testing is required: */
	if(noext(1))
		return;

/* Create a window on the default screen. */
        w = defwin(Dsp);
/* Create a window on the alternate screen. */
	w2 = defdraw(Dsp, VI_ALT_WIN);
	
/* Grab the pointer asynchronously  for the first window selecting PointerMotion and ButtonMotion events. */
	if( XGrabPointer(Dsp, w, False, PointerMotionMask | ButtonMotionMask, GrabModeAsync, GrabModeAsync, None, None, CurrentTime) != GrabSuccess) {
		delete("XGrabPointer() did not return GrabSuccess.");
		return;
	} else
		CHECK;
	
/* Generate a PointerMotion event on the alternate window. */
	(void) warppointer(Dsp, w2, 0,0);
	XSync(Dsp, True);
	(void) warppointer(Dsp, w2, 10, 10);
	XSync(Dsp, False);

/* Verify that a PointerMotion event was generated with respect to the grabbing window. */
	if (XCheckWindowEvent(Dsp, w, PointerMotionMask, &ev) == False) {
		report("Expected event (%s) not received.", eventname(MotionNotify));
		FAIL;
	} else {
		CHECK;
/* Verify that same_screen event component was False. */
		if(ev.xmotion.same_screen != False ) {
			report("The same_screen component of the %s event was not set correctly.", eventname(ButtonPress));
			FAIL;
		} else 
			CHECK;
	}

/* Generate a ButtonMotion event on the alternate window. */
	(void) warppointer(Dsp, w2, 0,0);
	XSync(Dsp, True);
	buttonpress(Dsp, Button1);
	(void) warppointer(Dsp, w2, 10, 10);
	XSync(Dsp, False);
	buttonrel(Dsp, Button1);

/* Verify that a ButtonMotion event was generated with respect to the grabbing window. */
	if (XCheckWindowEvent(Dsp, w, ButtonMotionMask, &ev) == False) {
		report("Expected event (%s) not received.", eventname(MotionNotify));
		FAIL;
	} else {
		CHECK;
/* Verify that same_screen event component was False. */
		if(ev.xmotion.same_screen != False ) {
			report("The same_screen component of the %s event was not set correctly.", eventname(ButtonPress));
			FAIL;
		} else 
			CHECK;
	}
	
	XUngrabPointer(Dsp, CurrentTime);
	
	CHECKPASS(6);

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
 * $XConsortium: bttnrls.m,v 1.11 94/04/17 21:07:10 rws Exp $
 */
>>TITLE ButtonRelease CH08
>>EXTERN
#include	<stdio.h>
#define EVENT ButtonRelease
#define EVENTMASK ButtonReleaseMask
#define PTRX 4
#define PTRY 3
>>SET end-function restoredevstate
>>ASSERTION Good B 1
When any pointer button is released,
then a xname event is generated.
>>STRATEGY
If extended testing is required:
  Create a window.
  Select for xname events.
  Simulate a xname event.
  Verify that event was delivered.
  Verify that event member fields are correctly set.
>>CODE
Display			*display = Dsp;
Window			w;
XEvent			event_return;
XButtonReleasedEvent	good;
PointerPlace		*ptr;

	/* If extended testing is required: */
	if(noext(1))
		return;

		/* Create a window. */
	w = defwin(display);

		/* Select for xname events. */
	XSelectInput(display, w, EVENTMASK);

		/* Simulate a xname event. */
	ptr = warppointer(display, w, PTRX, PTRY);
	buttonpress(display, Button1);
	XSync(display, True);
	buttonrel(display, Button1);

		/* Verify that event was delivered. */
		/* Verify that event member fields are correctly set. */
	if (!XCheckTypedWindowEvent(display, w, EVENT, &event_return)) {
		report("Expected %s event, got none", eventname(EVENT));
		FAIL;
	}
	else
		CHECK;

	good = event_return.xbutton;
	good.type = EVENT;
	good.send_event = False;
	good.display = display;
	good.window = w;
	good.root = DRW(display);
	good.subwindow = None;
	good.x = PTRX;
	good.y = PTRY;
	good.x_root = ptr->nx;
	good.y_root = ptr->ny;
	good.state = Button1Mask;
	good.button = Button1;
	good.same_screen = True;

	if (checkevent((XEvent *) &good, &event_return)) {
		report("Unexpected values in delivered event");
		FAIL;
	}
	else
		CHECK;

	
	CHECKPASS(2);

>>ASSERTION Good B 1
>>#
>>#COMMENT
>>#  Note that clients can have selected ButtonRelease events
>>#  and not get them since a buttonpress starts an implicit
>>#  pointer grab. We can avoid this by either:
>>#   (i) Not selecting on ButtonPressMask events on any client.
>>#  (ii) Not simulating a ButtonPress event.
>># (iii) Selecting after the button press has been simulated.
>>#  (iv) Explicitly performing an XUngrabPointer after the button press.
>>#  I choose (i).
>>#
>>#  Cal 18/02/92.
>>#
When a xname event is generated,
then
all clients having set
.S ButtonReleaseMask
event mask bits on the event window are delivered
a xname event.
>>STRATEGY
If extended testing is required:
  Create a second client.
  Create a third client.
  Create a window.
  Select for xname events on all the clients.
  Simulate a xname event on the window.
  Verify that a xname event was delivered to all clients.
  Verify that the event member fields are correctly set.
Otherwise:
  Create window.
  Set ButtonReleaseMask event mask bits on window.
  Verify that no error occurred.
>>CODE
Display			*display = Dsp;
Display			*client2;
Display			*client3;
Window			w;
XEvent			event_return;
XButtonReleasedEvent	good;
PointerPlace		*ptr;


	/* If extended testing is required: */
	if(config.extensions) {
	
		if(noext(1))
			return;

		/* Create a second client. */
		client2 = opendisplay();
		/* Create a third client. */
		client3 = opendisplay();

		/* Create a window. */
		w = defwin(display);

		/* Select for xname events on all the clients. */
		XSelectInput(client2, w, EVENTMASK);
		XSelectInput(client3, w, EVENTMASK);
		XSelectInput(display, w, EVENTMASK);
	
		/* Simulate a xname event on the window. */
		ptr = warppointer(display, w, PTRX, PTRY);
		buttonpress(display, Button1);

		XSync(display, True);
		XSync(client2, True);
		XSync(client3, True);

		buttonrel(display, Button1);

		XSync(client2, False);
		XSync(client3, False);
	
		good = event_return.xbutton;
		good.type = EVENT;
		good.send_event = False;
		good.display = display;
		good.window = w;
		good.root = DRW(display);
		good.subwindow = None;
		good.x = PTRX;
		good.y = PTRY;
		good.x_root = ptr->nx;
		good.y_root = ptr->ny;
		good.state = Button1Mask;
		good.button = Button1;
		good.same_screen = True;
	
		/* Verify that a xname event was delivered to all clients. */
		/* Verify that the event member fields are correctly set. */
		if (!XCheckTypedWindowEvent(display, w, EVENT, &event_return)) {
			report("Expected %s event, got none", eventname(EVENT));
			FAIL;
		} else {
			CHECK;
			if (checkevent((XEvent *) &good, &event_return)) {
				report("Unexpected values in delivered event");
				FAIL;
			} else
				CHECK;
		}	

		if (!XCheckTypedWindowEvent(client2, w, EVENT, &event_return)) {
			report("Expected %s event, got none", eventname(EVENT));
			FAIL;
		} else {
			CHECK;

			good.display = client2;
		
			if (checkevent((XEvent *) &good, &event_return)) {
				report("Unexpected values in delivered event");
				FAIL;
			} else
				CHECK;
		}


		if (!XCheckTypedWindowEvent(client3, w, EVENT, &event_return)) {
			report("Expected %s event, got none", eventname(EVENT));
			FAIL;
		} else {
			CHECK;

			good.display = client3;

			if (checkevent((XEvent *) &good, &event_return)) {
				report("Unexpected values in delivered event");
				FAIL;
			} else
				CHECK;
		}

		CHECKPASS(6);


	} else {
		
	/* Otherwise: */
		/* Create window. */
		w = mkwin(display, (XVisualInfo *) NULL, (struct area *) NULL, False);
		/* Set ButtonReleaseMask event mask bits on window. */
		XSelectInput(display, w, ButtonReleaseMask);
		XSync(display, True);
		/* Verify that no error occurred. */
		if (geterr() != Success) {
			delete("Error setting up for test.");
			return;
		}
		else
			CHECK;
		CHECKUNTESTED(1);
	}

>>ASSERTION Good B 1
>>#NOTE True for most events (except MappingNotify and selection stuff).
When a xname event is generated,
then
clients not having set
.S ButtonReleaseMask
event mask bits on the event window are not delivered
a xname event.
>>STRATEGY
If extended testing is enabled:
  Create window.
  Select for ButtonReleaseEvents. 
  Create a second client.
  Generate xname event.
  Verify that a xname event was delivered to the selecting client.
  Verify that no events were delivered to the other client.
>>CODE
Display			*display = Dsp;
Display			*client2;
XEvent			ev;
Window			w;
int			count;

	if(noext(1))
		return;

	client2 = opendisplay();

/* Create window. */
	w = defwin(display);

	(void) warppointer(display, w, PTRX, PTRY);

	buttonpress(display, Button1);

 	XSelectInput(display, w, ALLEVENTS & ~ButtonReleaseMask);
	XSelectInput(client2, w, EVENTMASK);

/* Generate xname event. */

	XSync(display, True);
	XSync(client2, True);

	buttonrel(display, Button1);

	XSync(client2, False);

/* Verify a ButtonRelease was generated. */
	if(XCheckWindowEvent(client2, w, EVENTMASK, &ev) == False) {
		report("No ButtonRelease event was generated.");
		FAIL;
	} else
		CHECK;

/* Verify no events were delivered. */
	count = XPending(display);
	if (count != 0) {
		report("Got %d unexpected events.", count);
		FAIL;
	} else
		CHECK;

	CHECKPASS(2);


>>ASSERTION Good B 1
When a xname event is generated
and no client has selected
.S ButtonReleaseMask
on the source window,
then the event propagates,
with propagation stopping at the root window of the screen or
at the first window with
.S ButtonReleaseMask
in its do-not-propagate mask,
from the source window to
the first ancestor window for which
some client has selected 
for xname events.
>>STRATEGY
If extended testing is required:
  Create a window.
  Create a child of that window.
  Create a child of the child.
  Select xname events on the root window.
  Simulate a xname event on the youngest child.
  Verify that a xname event was generated on the root.
  Select xname events on the oldest window.
  Set the do_not_propagate mask of its child to xname events.
  Simulate a xname event on the youngest child.
  Verify that no created window received a xname event.
  Clear the do_not_propagate mask on the oldest child.
  Set the do_not_propagate mask on the oldest window.
  Select for xname events on the oldest child.
  Simulate a xname on the youngest child.
  Verify that no xname event was sent to the oldest window.
  Verify that no xname event was sent to the youngest window.
  Verify that a xname event was sent to the oldest child.
>>CODE
XEvent			ev;
Window			w;
Window			w1;
Window			w2;
XSetWindowAttributes	atts;

	if(noext(1))
		return;

	w = defwin(Dsp);
	w1 = crechild(Dsp, w, (struct area *) NULL);
	w2 = crechild(Dsp, w1, (struct area *) NULL);
	XSelectInput(Dsp, DRW(Dsp), EVENTMASK);	
	(void) warppointer(Dsp, w2, 1,1);

	buttonpress(Dsp, Button1);
	XSync(Dsp, True);
	buttonrel(Dsp, Button1);
	
	if (XCheckWindowEvent(Dsp, DRW(Dsp), EVENTMASK, &ev) == False) {
		report("Expected event (%s) not received.", eventname(EVENT));
		FAIL;
	} else
		CHECK;

	XSelectInput(Dsp, DRW(Dsp), NoEventMask);	
	XSelectInput(Dsp, w, EVENTMASK);	

	atts.do_not_propagate_mask = EVENTMASK;
	XChangeWindowAttributes(Dsp, w1, CWDontPropagate, &atts);
	
	buttonpress(Dsp, Button1);	
	XSync(Dsp, True);
	buttonrel(Dsp, Button1);

	if (XCheckWindowEvent(Dsp, w2, EVENTMASK, &ev) != False) {
		report("Unexpected event (%s) received.", eventname(EVENT));
		FAIL;
	} else
		CHECK;

	if (XCheckWindowEvent(Dsp, w1, EVENTMASK, &ev) != False) {
		report("Unexpected event (%s) received.", eventname(EVENT));
		FAIL;
	} else
		CHECK;

	if (XCheckWindowEvent(Dsp, w, EVENTMASK, &ev) != False) {
		report("Unexpected event (%s) received.", eventname(EVENT));
		FAIL;
	} else
		CHECK;

	XSelectInput(Dsp, w1, EVENTMASK);	
	XChangeWindowAttributes(Dsp, w, CWDontPropagate, &atts);

	atts.do_not_propagate_mask = NoEventMask;
	XChangeWindowAttributes(Dsp, w1, CWDontPropagate, &atts);

	buttonpress(Dsp, Button1);
	XSync(Dsp, True);
	buttonrel(Dsp, Button1);	

	if (XCheckWindowEvent(Dsp, w2, EVENTMASK, &ev) != False) {
		report("Unexpected event (%s) received.", eventname(EVENT));
		FAIL;
	} else
		CHECK;

	if (XCheckWindowEvent(Dsp, w, EVENTMASK, &ev) != False) {
		report("Unexpected event (%s) received.", eventname(EVENT));
		FAIL;
	} else
		CHECK;

	if (XCheckWindowEvent(Dsp, w1, EVENTMASK, &ev) == False) {
		report("ButtonRelease event was not delivered to selecting child window.");
		FAIL;
	} else
		CHECK;

	CHECKPASS(7);

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
>>ASSERTION Good B 1
>>#
>>#COMMENT
>># This assertion contains a tautology; a child is an inferior.
>># - Cal 12/2/92.
>>#
When a xname event is delivered
and the source window is an inferior of the event window
and the source window is a child of the event window,
then
.M subwindow
is set to
the source window.
>>STRATEGY
If extended testing is required:
  Create a window.
  Select xname events on that window.
  Create a child of that window.
  Simulate a xname event on the child.
  Verify that a xname event was delivered to the parent. 
  Verify that the subwindow component was the child of the event window.
>>CODE
XEvent			ev;
Window			w;
Window			w1;

	/* If extended testing is required: */
	if(noext(1))
		return;

		/* Create a window. */
	w = defwin(Dsp);
		/* Create a child of the window. */
	w1 = crechild(Dsp, w, (struct area *) NULL);
		/* Select xname events on the child. */
	XSelectInput(Dsp, w, EVENTMASK);	
	(void) warppointer(Dsp, w1, 1,1);

		/* Simulate a xname event on the child. */
	buttonpress(Dsp, Button1);
	XSync(Dsp, True);
	buttonrel(Dsp, Button1);

		/* Verify that a xname event was generated on the parent. */
	if (XCheckWindowEvent(Dsp, w, EVENTMASK, &ev) == False) {
		report("Expected event (%s) not received.", eventname(EVENT));
		FAIL;
	} else {
		CHECK;

		/* Verify that the subwindow component was set to the child. */
		if(ev.xbutton.subwindow != w1) {
			report("The subwindow component of the %s event was not set correctly.", eventname(EVENT));
			FAIL;
		} else 
			CHECK;
	}

	CHECKPASS(2);


>>ASSERTION Good B 1
When a xname event is delivered
and the source window is an inferior of the event window
and the source window is not a child of the event window,
then
.M subwindow
is set to
the child of the event window that is
an ancestor of the source window.
>>STRATEGY
If extended testing is required:
  Create a window.
  Select xname events on that window.
  Create a child of that window.
  Create a child of the child.
  Simulate a xname event on the youngest child.
  Verify that a xname event was generated on the oldest parent.  
  Verify that the subwindow component was the child of the event window.
>>CODE
Window			w;
Window			w1;
Window			w2;
XEvent			ev;
struct area		area;

	if (noext(1))
		return;

        w = defwin(Dsp);
	setarea(&area, 10, 10, 30, 30);
	w1 = crechild(Dsp, w, &area);
	w2 = crechild(Dsp, w1, &area);
	(void) warppointer(Dsp, w2, 1,1);
	XSelectInput(Dsp, w, EVENTMASK);

	buttonpress(Dsp, Button1);
	XSync(Dsp, True);
	buttonrel(Dsp, Button1);
	
	if (XCheckWindowEvent(Dsp, w, EVENTMASK, &ev) == False) {
		report("Expected event (%s) not received.", eventname(EVENT));
		FAIL;
	} else {
		CHECK;
		if(ev.xbutton.subwindow != w1) {
			report("The subwindow component of the %s event was not set correctly.", eventname(EVENT));
			FAIL;
		} else 
			CHECK;
	}

	CHECKPASS(2);



>>ASSERTION Good B 1
When a xname event is delivered
and the source window is not an inferior of the event window,
then
.M subwindow
is set to
.S None .
>>STRATEGY
If extended testing is required:
  Create a window.
  Select xname events on that window.
  Simulate a xname event on the window.
  Verify that a xname event was generated.
  Verify that the subwindow component was None.
  Create a window.
  Grab the pointer with owner_events set to False.
  Simulate a xname event on the window.
  Verify that a xname event was generated on the grabbing window.
  Verify that the subwindow component was None.
>>CODE
Window			w;
Window			w2;
XEvent			ev;

	if (noext(1))
		return;

        w = defwin(Dsp);

	(void) warppointer(Dsp, w, 1,1);
	XSelectInput(Dsp, w, EVENTMASK);

	buttonpress(Dsp, Button1);
	XSync(Dsp, True);
	buttonrel(Dsp, Button1);
	
	if (XCheckWindowEvent(Dsp, w, EVENTMASK, &ev) == False) {
		report("Expected event (%s) not received.", eventname(EVENT));
		FAIL;
	} else {
		CHECK;
		if(ev.xbutton.subwindow != None) {
			report("The subwindow component of the %s event was not set correctly.", eventname(EVENT));
			FAIL;
		} else 
			CHECK;
	}

	w2 = defwin(Dsp);

	if( XGrabPointer(Dsp, w, False, EVENTMASK, GrabModeAsync, GrabModeAsync, None, None, CurrentTime) != GrabSuccess) {
		delete("XGrabPointer() did not return GrabSuccess.");
		return;
	} else
		CHECK;

	(void) warppointer(Dsp, w2, 1,1);
	buttonpress(Dsp, Button1);
	XSync(Dsp, True);
	buttonrel(Dsp, Button1);
	
	if (XCheckWindowEvent(Dsp, w, EVENTMASK, &ev) == False) {
		report("Expected event (%s) not received.", eventname(EVENT));
		FAIL;
	} else {
		CHECK;
		if(ev.xbutton.subwindow != None) {
			report("The subwindow component of the %s event was not set correctly.", eventname(EVENT));
			FAIL;
		} else 
			CHECK;
	}

	XUngrabPointer(Dsp, CurrentTime);

	CHECKPASS(5);

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
    Create a window.
    Select for xname events.
    Create a window on the alternate screen.
    Grab the pointer for the first window with owner_events set to False.
    Simulate a xname event on the alternate window.
    Verify that a xname event was generated with respect to the grabbing window.
    Verify that the x and y components were set to zero.
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

			/* Create a window. */
        w = defwin(Dsp);
	
	(void) warppointer(Dsp, w, 1,1);
			/* Select for xname events. */
	XSelectInput(Dsp, w, EVENTMASK);
	
			/* Create a window on the alternate screen. */
	w2 = defdraw(Dsp, VI_ALT_WIN);
	
			/* Grab the pointer for the first window with owner_events set to False. */
	if( XGrabPointer(Dsp, w, False, EVENTMASK, GrabModeAsync, GrabModeAsync, None, None, CurrentTime) != GrabSuccess) {
		delete("XGrabPointer() did not return GrabSuccess.");
		return;
	} else
		CHECK;
	
			/* Simulate a xname event on the alternate window. */
	(void) warppointer(Dsp, w2, 1,1);
	buttonpress(Dsp, Button1);
	XSync(Dsp, True);
	buttonrel(Dsp, Button1);
	
			/* Verify that a xname event was generated with respect to the grabbing window. */
	if (XCheckWindowEvent(Dsp, w, EVENTMASK, &ev) == False) {
		report("Expected event (%s) not received.", eventname(EVENT));
		FAIL;
	} else {
		CHECK;
			/* Verify that the x and y components were set to zero. */
		if(ev.xbutton.x != 0 || ev.xbutton.y != 0) {
			report("The x and y components of the %s event were not set correctly.", eventname(EVENT));
			FAIL;
		} else 
			CHECK;
	}
	
	XUngrabPointer(Dsp, CurrentTime);
	
	CHECKPASS(4);

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
>>#NOTEs .M button
>>#NOTEs represents the pointer button which changed state
>>#NOTEs and is set to either
>>#NOTEs .S Button1 ,
>>#NOTEs .S Button2 ,
>>#NOTEs .S Button3 ,
>>#NOTEs .S Button4 ,
>>#NOTEs or
>>#NOTEs .S Button5 .
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
    Select xname events on the window.
    Create a window on the alternative screen.
    Grab the keyboard for the first window.
    Simulate a xname event on the alternate window.
    Verify that a xname event was generated on the grabbing window.
    Verify that the same_screen component was False.
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
	
	(void) warppointer(Dsp, w, 1,1);
			/* Select xname events on the window. */
	XSelectInput(Dsp, w, EVENTMASK);
	
			/* Create a window on the alternative screen. */
	w2 = defdraw(Dsp, VI_ALT_WIN);
	
			/* Grab the keyboard for the first window. */
	if( XGrabPointer(Dsp, w, False, EVENTMASK, GrabModeAsync, GrabModeAsync, None, None, CurrentTime) != GrabSuccess) {
		delete("XGrabPointer() did not return GrabSuccess.");
		return;
	} else
		CHECK;
	
			/* Simulate a xname event on the alternate window. */
	(void) warppointer(Dsp, w2, 1,1);
	buttonpress(Dsp, Button1);
	XSync(Dsp, True);
	buttonrel(Dsp, Button1);
	
			/* Verify that a xname event was generated on the grabbing window. */
	if (XCheckWindowEvent(Dsp, w, EVENTMASK, &ev) == False) {
		report("Expected event (%s) not received.", eventname(EVENT));
		FAIL;
	} else {
		CHECK;
			/* Verify that the same_screen component was False. */
		if(ev.xbutton.same_screen != False ) {
			report("The same_screen component of the %s event was not set correctly.", eventname(EVENT));
			FAIL;
		} else 
			CHECK;
	}
	
	XUngrabPointer(Dsp, CurrentTime);
	
	CHECKPASS(4);

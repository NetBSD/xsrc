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
 * $XConsortium: bttnprss.m,v 1.13 94/04/17 21:07:09 rws Exp $
 */
>>TITLE ButtonPress CH08
>>EXTERN
#define EVENT ButtonPress
#define EVENTMASK ButtonPressMask
#define PTRX 4
#define PTRY 3
>>SET end-function restoredevstate
>>ASSERTION Good B 1
When any pointer button is pressed,
then a xname event is generated.
>>STRATEGY
If extended testing is enabled:
  Create window.
  Select for xname events.
  Generate xname event.
  Verify that a xname event was delivered.
  Verify that event member fields are correctly set.
>>CODE
Display			*display = Dsp;
Window			w;
XEvent			event_return;
XButtonPressedEvent	good;
PointerPlace		*ptr;

	if(noext(1))
		return;

/* Create window. */
	w = defwin(display);

/* Select for events. */
	XSelectInput(display, w, EVENTMASK);
	XSync(display, True);

/* Generate xname event. */
	XSync(display, True);
	ptr = warppointer(display, w, PTRX, PTRY);
	buttonpress(display, Button1);

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
	good.state = 0L;
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
When a xname event is generated
and no active pointer grab is in progress
and at least one ancestor of the source window has a matching passive grab
on the pressed pointer button,
then the X server activates a passive grab for
the first ancestor of the source window,
searching from the root window down.
>>STRATEGY
If extended testing is enabled :
  Create a window.
  Create a child window.
  Create a child of the child.
  Select input on EnterNotify events for all of the windows.
  Select passive grabs for all the windows on Button1.
  Simulate a ButtonPress event.
  Verify that the child windows did not receive EnterNotify events.
  Verify that the oldest ancestor window received an EnterNotify with mode NotifyGrab.
  Simulate a ButtonRelease event.
  Release the passive grabs.
>>CODE
Window			w;
Window			w1;
Window			w2;
XEvent			ev;
struct area		area;
char			*unexpstr = "Unexpected event on a child window.";

	if (noext(1))
		return;

        w = defwin(Dsp);
	setarea(&area, 10, 10, 30, 30);
	w1 = crechild(Dsp, w, &area);
	w2 = crechild(Dsp, w1, &area);
	(void) warppointer(Dsp, w2, 1,1);

	XSelectInput(Dsp, w, EnterWindowMask);
	XSelectInput(Dsp, w1, EnterWindowMask);
	XSelectInput(Dsp, w2, EnterWindowMask);

	XGrabButton(Dsp, Button1, AnyModifier, w, False,  0L, GrabModeAsync, GrabModeAsync, None, None);
	XGrabButton(Dsp, Button1, AnyModifier, w1, False, 0L, GrabModeAsync, GrabModeAsync, None, None);
	XGrabButton(Dsp, Button1, AnyModifier, w2, False, 0L, GrabModeAsync, GrabModeAsync, None, None);

	XSync(Dsp, True);
	buttonpress(Dsp, Button1);

	if(XCheckWindowEvent(Dsp, w1, EnterWindowMask, &ev) != False) {
		report(unexpstr);
		FAIL;
	} else
		CHECK;

	if(XCheckWindowEvent(Dsp, w2, EnterWindowMask, &ev) != False) {
		report(unexpstr);
		FAIL;
	} else
		CHECK;

	if(XCheckWindowEvent(Dsp, w, EnterWindowMask, &ev) == False) {
		report("No event was activated on the oldest ancestor window.");
		FAIL;
	} else {

		CHECK;
		if(ev.xcrossing.mode != NotifyGrab) {
			report("Passive grab was not activated on oldest ancestor window.");
			FAIL;
		} else
			CHECK;
	}

	buttonrel(Dsp, Button1);
	XUngrabButton(Dsp, Button1, AnyModifier, w);
	XUngrabButton(Dsp, Button1, AnyModifier, w1);
	XUngrabButton(Dsp, Button1, AnyModifier, w2);

	CHECKPASS(4);

>>ASSERTION Good B 1
When a xname event is generated
and no active pointer grab is in progress
and no ancestor of the source window has a matching passive grab
on the pressed pointer button,
then the X server automatically starts an active grab
for the client receiving the event and
sets the last-pointer-grab time to the current server time.
>>ASSERTION Good B 1
When a xname event is generated
and no active pointer grab is in progress
and no ancestor of the source window has a matching passive grab
on the pressed pointer button and
no client has selected for button presses on the event window,
then the event is discarded.
>>STRATEGY
If extended testing is enabled:
  Create window.
  Select for all events except ButtonPress.
  Generate xname event.
  Verify that no event was delivered.
>>CODE
Display			*display = Dsp;
Window			w;
int			count;

	if(noext(1))
		return;

/* Create window. */
	w = defwin(display);

	(void) warppointer(display, w, PTRX, PTRY);
	XSelectInput(display, w, ALLEVENTS & ~ButtonPressMask);
	XSync(display, True);
/* Generate xname event. */
	XSync(display, True);
	buttonpress(display, Button1);

/* Verify no events were delivered. */

	count = XPending(display);
	if (count != 0) {
		report("Got %d unexpected events.", count);
		FAIL;
	}
	else
		PASS;

>>ASSERTION Good B 1
When a xname event is generated,
then
all clients having set
.S ButtonPressMask
event mask bits on the event window are delivered
a xname event.
>>#COMMENT
>># This assertion is misleading, there can only ever be one client 
>># selecting ButtonPress events on any particular window.
>># For the time-being this test is a copy of the single client test.
>># Cal 12/2/92.
>>#
>>STRATEGY
If extended testing is enabled:
  Create window.
  Select for xname events.
  Generate xname event.
  Verify that a xname event was delivered.
  Verify that event member fields are correctly set.
Otherwise:
  Create window.
  Set ButtonPressMask event mask bits on window.
  Verify that no error occurred.
>>CODE
Display			*display = Dsp;
Window			w;
XEvent			event_return;
XButtonPressedEvent	good;
PointerPlace		*ptr;


	if(config.extensions) {

		if(noext(1))
			return;
	/* Create window. */
		w = defwin(display);

	/* Select for events. */
		XSelectInput(display, w, EVENTMASK);
		XSync(display, True);
	
	/* Generate xname event. */
		XSync(display, True);
		ptr = warppointer(display, w, PTRX, PTRY);
		buttonpress(display, Button1);
	
	/* Verify that event was delivered. */
	/* Verify that event member fields are correctly set. */
		if (!XCheckTypedWindowEvent(display, w, EVENT, &event_return)) {
			report("Expected %s event, got none", eventname(EVENT));
			FAIL;
		} else
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
		good.state = 0L;
		good.button = Button1;
		good.same_screen = True;
	
		if (checkevent((XEvent *) &good, &event_return)) {
			report("Unexpected values in delivered event");
			FAIL;
		}
		else
			CHECK;
		
		CHECKPASS(2);

	} else {

	/* Create window. */
		w = mkwin(display, (XVisualInfo *) NULL, (struct area *) NULL, False);
	/* Set ButtonPressMask event mask bits on window. */
		XSelectInput(display, w, ButtonPressMask);
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
.S ButtonPressMask
event mask bits on the event window are not delivered
a xname event.
>>STRATEGY
If extended testing is enabled:
  Create window.
  Select for all events except ButtonPress.
  Create a second client.
  Select for ButtonPress events on the window
  Generate xname event.
  Verify that a xname event was delivered.
  Verify that no event was delivered to the other client.
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
	XSelectInput(display, w, ALLEVENTS & ~ButtonPressMask);
	XSelectInput(client2, w, ButtonPressMask);
	XSync(display, True);
	XSync(client2, True);
/* Generate xname event. */
	buttonpress(display, Button1);

	XSync(client2, False);


/* Verify a ButtonPress was generated. */
	if(XCheckWindowEvent(client2, w, ButtonPressMask, &ev) == False) {
		report("No ButtonPress event was generated.");
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
.S ButtonPressMask
on the source window,
then the event propagates,
with propagation stopping at the root window of the screen or
at the first window with
.S ButtonPressMask
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
	XSelectInput(Dsp, DRW(Dsp), ButtonPressMask);	
	(void) warppointer(Dsp, w2, 1,1);

	XSync(Dsp, True);
	buttonpress(Dsp, Button1);
	
	if (XCheckWindowEvent(Dsp, DRW(Dsp), ButtonPressMask, &ev) == False) {
		report("Expected event (%s) not received.", eventname(ButtonPress));
		FAIL;
	} else
		CHECK;

	XSelectInput(Dsp, DRW(Dsp), NoEventMask);	
	XSelectInput(Dsp, w, ButtonPressMask);	

	atts.do_not_propagate_mask = ButtonPressMask;
	XChangeWindowAttributes(Dsp, w1, CWDontPropagate, &atts);
	
	buttonrel(Dsp, Button1);	
	buttonpress(Dsp, Button1);

	if (XCheckWindowEvent(Dsp, w2, ButtonPressMask, &ev) != False) {
		report("Unexpected event (%s) received.", eventname(ButtonPress));
		FAIL;
	} else
		CHECK;

	if (XCheckWindowEvent(Dsp, w1, ButtonPressMask, &ev) != False) {
		report("Unexpected event (%s) received.", eventname(ButtonPress));
		FAIL;
	} else
		CHECK;

	if (XCheckWindowEvent(Dsp, w, ButtonPressMask, &ev) != False) {
		report("Unexpected event (%s) received.", eventname(ButtonPress));
		FAIL;
	} else
		CHECK;

	XSelectInput(Dsp, w1, ButtonPressMask);	
	XChangeWindowAttributes(Dsp, w, CWDontPropagate, &atts);

	atts.do_not_propagate_mask = NoEventMask;
	XChangeWindowAttributes(Dsp, w1, CWDontPropagate, &atts);

	buttonrel(Dsp, Button1);	
	buttonpress(Dsp, Button1);

	if (XCheckWindowEvent(Dsp, w2, ButtonPressMask, &ev) != False) {
		report("Unexpected event (%s) received.", eventname(ButtonPress));
		FAIL;
	} else
		CHECK;

	if (XCheckWindowEvent(Dsp, w, ButtonPressMask, &ev) != False) {
		report("Unexpected event (%s) received.", eventname(ButtonPress));
		FAIL;
	} else
		CHECK;

	if (XCheckWindowEvent(Dsp, w1, ButtonPressMask, &ev) == False) {
		report("ButtonPress event was not delivered to selecting child window.");
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
>># Cal 12/2/92.
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

	if(noext(1))
		return;

	w = defwin(Dsp);
	w1 = crechild(Dsp, w, (struct area *) NULL);
	XSelectInput(Dsp, w, ButtonPressMask);	
	(void) warppointer(Dsp, w1, 1,1);

	XSync(Dsp, True);
	buttonpress(Dsp, Button1);
	
	if (XCheckWindowEvent(Dsp, w, ButtonPressMask, &ev) == False) {
		report("Expected event (%s) not received.", eventname(ButtonPress));
		FAIL;
	} else {
		CHECK;
		if(ev.xbutton.subwindow != w1) {
			report("The subwindow component of the %s event was not set correctly.", eventname(ButtonPress));
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
	XSelectInput(Dsp, w, ButtonPressMask);

	XSync(Dsp, True);
	buttonpress(Dsp, Button1);
	
	if (XCheckWindowEvent(Dsp, w, ButtonPressMask, &ev) == False) {
		report("Expected event (%s) not received.", eventname(ButtonPress));
		FAIL;
	} else {
		CHECK;
		if(ev.xbutton.subwindow != w1) {
			report("The subwindow component of the %s event was not set correctly.", eventname(ButtonPress));
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
	XSelectInput(Dsp, w, ButtonPressMask);

	XSync(Dsp, True);
	buttonpress(Dsp, Button1);
	
	if (XCheckWindowEvent(Dsp, w, ButtonPressMask, &ev) == False) {
		report("Expected event (%s) not received.", eventname(ButtonPress));
		FAIL;
	} else {
		CHECK;
		if(ev.xbutton.subwindow != None) {
			report("The subwindow component of the %s event was not set correctly.", eventname(ButtonPress));
			FAIL;
		} else 
			CHECK;
	}

	w2 = defwin(Dsp);

	if( XGrabPointer(Dsp, w, False, ButtonPressMask, GrabModeAsync, GrabModeAsync, None, None, CurrentTime) != GrabSuccess) {
		delete("XGrabPointer() did not return GrabSuccess.");
		return;
	} else
		CHECK;

	(void) warppointer(Dsp, w2, 1,1);
	XSync(Dsp, True);
	buttonpress(Dsp, Button1);
	
	if (XCheckWindowEvent(Dsp, w, ButtonPressMask, &ev) == False) {
		report("Expected event (%s) not received.", eventname(ButtonPress));
		FAIL;
	} else {
		CHECK;
		if(ev.xbutton.subwindow != None) {
			report("The subwindow component of the %s event was not set correctly.", eventname(ButtonPress));
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
    Select xname events on that window.
    Create a window on the alternate screen.
    Grab the pointer with owner_events set to False.
    Simulate a xname event on the window.
    Verify that a xname event was generated on the grabbing window.
    Verify that the x and y  components are set to zero.
>>CODE
Window			w;
Window			w2;
XEvent			ev;

/* Report unsupported if multiple screens are not supported. */
	if (config.alt_screen == -1) {
		unsupported("Multiple screens not supported.");
		return;
	} else
		CHECK;

	if(noext(1))
		return;

        w = defwin(Dsp);
	
	(void) warppointer(Dsp, w, 1,1);
	XSelectInput(Dsp, w, ButtonPressMask);
	
	w2 = defdraw(Dsp, VI_ALT_WIN);
	
	if( XGrabPointer(Dsp, w, False, ButtonPressMask, GrabModeAsync, GrabModeAsync, None, None, CurrentTime) != GrabSuccess) {
		delete("XGrabPointer() did not return GrabSuccess.");
		return;
	} else
		CHECK;
	
	(void) warppointer(Dsp, w2, 1,1);
	XSync(Dsp, True);
	buttonpress(Dsp, Button1);
	
	if (XCheckWindowEvent(Dsp, w, ButtonPressMask, &ev) == False) {
		report("Expected event (%s) not received.", eventname(ButtonPress));
		FAIL;
	} else {
		CHECK;
		if(ev.xbutton.x != 0 || ev.xbutton.y != 0) {
			report("The x and y components of the %s event were not set correctly.", eventname(ButtonPress));
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
    Create a window.
    Select xname events on that window.
    Create a window on the alternate screen.
    Grab the pointer with owner_events set to False.
    Simulate a xname event on the window.
    Verify that a xname event was generated on the grabbing window.
    Verify that the same_screen component is set to zero.
>>CODE
Window			w;
Window			w2;
XEvent			ev;

	if (config.alt_screen == -1) {
		unsupported("Multiple screens not supported.");
		return;
	} else
		CHECK;

	if(noext(1))
		return;

        w = defwin(Dsp);
	
	(void) warppointer(Dsp, w, 1,1);
	XSelectInput(Dsp, w, ButtonPressMask);
	
	w2 = defdraw(Dsp, VI_ALT_WIN);
	
	if( XGrabPointer(Dsp, w, False, ButtonPressMask, GrabModeAsync, GrabModeAsync, None, None, CurrentTime) != GrabSuccess) {
		delete("XGrabPointer() did not return GrabSuccess.");
		return;
	} else
		CHECK;
	
	(void) warppointer(Dsp, w2, 1,1);
	XSync(Dsp, True);
	buttonpress(Dsp, Button1);
	
	if (XCheckWindowEvent(Dsp, w, ButtonPressMask, &ev) == False) {
		report("Expected event (%s) not received.", eventname(ButtonPress));
		FAIL;
	} else {
		CHECK;
		if(ev.xbutton.same_screen != False ) {
			report("The same_screen component of the %s event was not set correctly.", eventname(ButtonPress));
			FAIL;
		} else 
			CHECK;
	}
	
	XUngrabPointer(Dsp, CurrentTime);
	
	CHECKPASS(4);

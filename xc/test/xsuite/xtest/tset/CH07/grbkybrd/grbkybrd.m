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

 * Copyright 1990, 1991 UniSoft Group Limited.
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
 * $XConsortium: grbkybrd.m,v 1.9 94/04/17 21:06:23 rws Exp $
 */
>>TITLE XGrabKeyboard CH07
int
xname
Display	*display = Dsp;
Window	grab_window = defwin(display);
Bool	owner_events = False;
int 	pointer_mode = GrabModeAsync;
int 	keyboard_mode = GrabModeAsync;
Time	thetime = CurrentTime;
>>#
>># Expect that GrabSuccess is returned unless otherwise mentioned.
>>SET return-value GrabSuccess
>># 
>># Set the startup and cleanup functions to set the focus to the root window
>># for the tests and reset it afterwards.
>>SET startup focusstartup
>>SET cleanup focuscleanup
>>#
>>ASSERTION Good A
A successful call to xname actively grabs control of the keyboard and
returns
.S GrabSuccess .
>>STRATEGY
Touch test for return value.
>>CODE

	XCALL;
	if (ValueReturn == GrabSuccess)
		PASS;
>>ASSERTION Good A
When the keyboard is grabbed, then
.S FocusIn
and
.S FocusOut
events are generated as though the focus had changed from the
previous focus window to
.A grab_window.
>>STRATEGY
Create window.
Set Focus to that window.
Enable events on grab and focus window.
Grab keyboard.
Verify grab-mode FocusOut from window.
Verify grab-mode FocusIn to grab window.
>>CODE
Window	win;
Window	ofocus;
XEvent	ev;
XFocusInEvent	figood;
XFocusOutEvent	fogood;
int 	orevert;

	/*
	 * Save current input focus to pose as little inconvenience as
	 * possible.
	 */
	XGetInputFocus(display, &ofocus, &orevert);

	win = defwin(display);
	XSetInputFocus(display, win, RevertToNone, CurrentTime);
	if (isdeleted()) {
		report("Could not set up focus");
		return;
	}

	XSelectInput(display, grab_window, FocusChangeMask);
	XSelectInput(display, win, FocusChangeMask);

	XCALL;

	/*
	 * Set up the expected good events.
	 */
	defsetevent(figood, display, FocusIn);
	figood.window = grab_window;
	figood.mode = NotifyGrab;
	figood.detail = NotifyNonlinear;

	defsetevent(fogood, display, FocusOut);
	fogood.window = win;
	fogood.mode = NotifyGrab;
	fogood.detail = NotifyNonlinear;

	if (getevent(display, &ev) == 0 || ev.type != FocusOut) {
		report("Did not get expected FocusOut event");
		FAIL;
	} else
		CHECK;

	if (checkevent((XEvent*)&fogood, &ev))
		FAIL;
	else
		CHECK;

	if (getevent(display, &ev) == 0 || ev.type != FocusIn) {
		report("Did not get expected FocusIn event");
		FAIL;
	} else
		CHECK;

	if (checkevent((XEvent*)&figood, &ev))
		FAIL;
	else
		CHECK;

	/* Reset old focus */
	XSetInputFocus(display, ofocus, orevert, CurrentTime);

	CHECKPASS(4);
>>ASSERTION Good B 3
When a successful call to xname is made by a client, then
subsequent keyboard events are reported only to that client.
>>STRATEGY
If extensions are available:
  Create a second client.
  Select key events for default client.
  Select key events for second client.
  Call xname with default client.

  Press key in window.
  Verify that key event is received only by default client.
>>CODE
>>SET end-function restoredevstate
Display	*client2;
XEvent	ev;

	if (noext(0))
		return;

	if ((client2 = opendisplay()) == NULL) {
		tccabort("Could not open display (%s)", config.display);
		return;
	} else
		CHECK;

	XSelectInput(display, grab_window, KeyPressMask|KeyReleaseMask);
	XSelectInput(client2, grab_window, KeyPressMask|KeyReleaseMask);

	XCALL;

	(void) warppointer(client2, grab_window, 1, 1);
	keypress(display, getkeycode(display));

	if (XCheckMaskEvent(display, KeyPressMask, &ev))
		CHECK;
	else {
		report("Keyboard event not received on the grabbing client");
		FAIL;
	}
	if (XCheckMaskEvent(client2, KeyPressMask, &ev)) {
		report("Keyboard event was received on a non-grabbing client");
		FAIL;
	} else
		CHECK;

	CHECKPASS(3);
>>ASSERTION Good A
A call to xname overrides any active keyboard grab by this client.
>>STRATEGY
Call xname with pointer_mode GrabModeAsync.
Check that pointer is not frozen.
Call xname with pointer_mode GrabModeSync.
Verify that pointer is frozen and so the last grab was overriden.
>>CODE

	pointer_mode = GrabModeAsync;
	XCALL;

	if (ispfrozen()) {
		delete("Could not setup grab");
		return;
	}

	pointer_mode = GrabModeSync;
	XCALL;

	if (ispfrozen())
		CHECK;
	else {
		report("A second grab did not override the first");
		FAIL;
	}

	CHECKPASS(1);
>>ASSERTION Good B 3
When
.A  owner_events
is
.S False ,
then all generated keyboard events are reported with
respect to the
.A grab_window .
>>STRATEGY
If extensions available:
  Create grab_window.
  Create window2.
  Select key events on both windows.
  Call xname with owner_events False.
  Move pointer to window2 (focus is the root).
  Press key.
  Verify that event is reported on grab_window.
>>CODE
Window	window2;
XEvent	ev;

	if (noext(0))
		return;

	window2 = defwin(display);
	XSelectInput(display, grab_window, KeyPressMask|KeyReleaseMask);
	XSelectInput(display, window2, KeyPressMask|KeyReleaseMask);

	owner_events = False;
	XCALL;

	(void) warppointer(display, window2, 1, 1);
	keypress(display, getkeycode(display));

	if (XCheckWindowEvent(display, grab_window, KeyPressMask, &ev))
		CHECK;
	else {
		report("Event was not reported on grab_window");
		FAIL;
	}
	if (XCheckWindowEvent(display, window2, KeyPressMask, &ev)) {
		report("Event was reported on the owner window");
		FAIL;
	} else
		CHECK;

	CHECKPASS(2);
>>ASSERTION Good B 3
When
.A owner_events
is
.S True
and a keyboard event is generated that would normally be reported to the client,
then it is reported on the window it would normally be reported on.
>>STRATEGY
If extensions available:
  Create grab_window.
  Create window2.
  Select key events on both windows.
  Call xname with owner_events True.
  Move pointer to window2 (focus is the root).
  Press key.
  Verify that event is reported on window2.
>>CODE
Window	window2;
XEvent	ev;

	if (noext(0))
		return;

	window2 = defwin(display);
	XSelectInput(display, grab_window, KeyPressMask|KeyReleaseMask);
	XSelectInput(display, window2, KeyPressMask|KeyReleaseMask);

	owner_events = True;
	XCALL;

	(void) warppointer(display, window2, 1, 1);
	keypress(display, getkeycode(display));

	if (XCheckWindowEvent(display, window2, KeyPressMask, &ev))
		CHECK;
	else {
		report("Event was not reported on the owner window");
		FAIL;
	}
	if (XCheckWindowEvent(display, grab_window, KeyPressMask, &ev)) {
		report("Event was reported on the grab_window");
		FAIL;
	} else
		CHECK;

	CHECKPASS(2);
>>ASSERTION Good B 3
When
.A owner_events
is
.S True ,
and a keyboard event is generated that would not normally be reported
to the client,
then it is reported on the
.A grab_window .
>>STRATEGY
If extensions available:
  Create grab_window.
  Create window2.
  Call xname with owner_events True.
  Move pointer to window2 (focus is the root).
  Press key.
  Verify that event is reported on grab_window.
>>CODE
Window	window2;
XEvent	ev;

	if (noext(0))
		return;

	window2 = defwin(display);

	owner_events = True;
	XCALL;

	(void) warppointer(display, window2, 1, 1);
	keypress(display, getkeycode(display));

	if (XCheckWindowEvent(display, grab_window, KeyPressMask, &ev))
		CHECK;
	else {
		report("Event was not reported on grab_window");
		FAIL;
	}
	if (XCheckWindowEvent(display, window2, KeyPressMask, &ev)) {
		report("Event was reported on the owner window");
		FAIL;
	} else
		CHECK;

	CHECKPASS(2);
>>ASSERTION Good B 3
When the keyboard is grabbed, then
.S KeyPress
and
.S KeyRelease
events are always reported,
independent of any event selection made by the client.
>>STRATEGY
If extensions available:
  Grab keyboard by calling xname.
  Press key.
  Verify that a KeyPress event is reported.
  Release key.
  Verify that a KeyRelease event is reported.
>>CODE
XEvent	ev;
int 	key;

	if (noext(0))
		return;

	XCALL;

	(void) warppointer(display, grab_window, 1, 1);
	key = getkeycode(display);
	keypress(display, key);

	if (XCheckMaskEvent(display, KeyPressMask, &ev))
		CHECK;
	else {
		report("KeyPress event was not reported during grab when not selected");
		FAIL;
	}
	keyrel(display, key);

	if (XCheckMaskEvent(display, KeyReleaseMask, &ev))
		CHECK;
	else {
		report("KeyRelease event was not reported during grab when not selected");
		FAIL;
	}

	CHECKPASS(2);
>>ASSERTION Good B 3
When
.A keyboard_mode
is
.S GrabModeAsync ,
then keyboard event processing continues normally.
>>STRATEGY
If extensions available:
  Call xname with keyboard_mode GrabModeAsync.
  Verify that keyboard is not frozen.
>>EXTERN

/*
 * Returns True if the keyboard is frozen.
 */
static int
iskfrozen()
{
XEvent	ev;
int 	res;
static int 	key;

	XSync(display, True); /* Flush previous events */
	key = getkeycode(display);

	/*
	 * Try to provoke a keypress on grab_window.
	 */
	(void) warppointer(display, grab_window, 1, 1);
	keypress(display, key);
	if (XCheckMaskEvent(display, (long)KeyPressMask, &ev))
		res = False;
	else
		res = True;

	return(res);
}

>>CODE

	if (noext(0))
		return;

	keyboard_mode = GrabModeAsync;
	XCALL;

	if (iskfrozen()) {
		report("Keyboard was frozen by GrabModeAsync");
		FAIL;
	} else
		CHECK;

	CHECKPASS(1);
>>ASSERTION Good B 3
When
.A keyboard_mode
is
.S GrabModeAsync ,
and the keyboard is currently frozen by this client,
then processing of keyboard events is resumed.
>>STRATEGY
If extensions available:
  Freeze keyboard using XGrabPointer.
  Call xname with keyboard_mode GrabModeAsync.
  Verify that keyboard is not frozen.
>>CODE

	if (noext(0))
		return;

	XGrabPointer(display, grab_window, False, 0, GrabModeAsync, GrabModeSync,
		None, None, CurrentTime);

	keyboard_mode = GrabModeAsync;
	XCALL;

	if (iskfrozen()) {
		report("Keyboard was not released by GrabModeAsync");
		FAIL;
	} else
		CHECK;

	CHECKPASS(1);
>>ASSERTION Good B 3
When
.A keyboard_mode
is
.S GrabModeSync ,
then the state of the keyboard,
as seen by client applications,
appears to freeze
and no further keyboard events are generated until the
grabbing client issues a releasing
.S XAllowEvents
call or until the keyboard grab is released.
>>STRATEGY
If extensions available:
  Call xname with keyboard_mode GrabModeSync.
  Verify that keyboard is frozen.
  Release grab.
  Verify that keyboard is not frozen.
>>CODE

	if (noext(0))
		return;

	XSelectInput(display, grab_window, KeyPressMask);

	keyboard_mode = GrabModeSync;
	XCALL;

	if (iskfrozen())
		CHECK;
	else {
		report("Keyboard was not frozen with keyboard_mode GrabModeSync");
		FAIL;
	}

	XUngrabKeyboard(display, CurrentTime);
	XSync(display, False);
	if (iskfrozen()) {
		report("Keyboard was not thawed after the grab was released");
		FAIL;
	} else
		CHECK;

	CHECKPASS(2);
>>ASSERTION Good B 3
When the keyboard is frozen, then the
actual keyboard changes are not lost while the keyboard is frozen
and are processed after the grab is released or the client calls
.F XAllowEvents .
>>STRATEGY
If extensions available:
  Enable key events on grab_window.
  Call xname with keyboard_mode GrabModeSync.
  Press and release key.
  Check no events arrived yet.
  Release grab.
  Verify that KeyPress and KeyRelease events are now received.
>>CODE
int 	key;
int 	n;
int 	first;
XEvent	ev;

	if (noext(0))
		return;

	key = getkeycode(display);
	XSelectInput(display, grab_window, KeyPressMask|KeyReleaseMask);

	keyboard_mode = GrabModeSync;
	XCALL;

	(void) warppointer(display, grab_window, 10, 10);
	keypress(display, key);
	keyrel(display, key);

	if (XCheckMaskEvent(display, KeyPressMask|KeyReleaseMask, &ev)) {
		delete("Events received while keyboard supposed to be frozen");
		return;
	} else
		CHECK;

	XUngrabKeyboard(display, thetime);
	if (isdeleted())
		return;

	n = getevent(display, &ev);
	if (n != 2) {
		report("Expecting two events to be released after grab");
		report("  got %d", n);
		FAIL;
	} else {
		first = ev.type;
		(void) getevent(display, &ev);
		/* (We have already checked that there is another event) */

		if (ev.type != KeyPress && first != KeyPress) {
			report("Did not get KeyPress event after releasing grab");
			FAIL;
		} else
			CHECK;
		if (ev.type != KeyRelease && first != KeyRelease) {
			report("Did not get KeyRelease event after releasing grab");
			FAIL;
		} else
			CHECK;
	}

	CHECKPASS(3);
>>ASSERTION Good A
When
.A pointer_mode
is
.S GrabModeAsync ,
then pointer event processing is unaffected
by activation of the grab.
>>STRATEGY
Grab keyboard with pointer_mode GrabModeAsync.
Verify that pointer events are still received.
>>EXTERN

/*
 * Returns True if the pointer is frozen.
 */
static Bool
ispfrozen()
{
XEvent	ev;
Window	win;

	win = defwin(Dsp);
	(void) warppointer(Dsp, win, 0, 0);
	XSync(Dsp, True);	/* discard events */

	XSelectInput(Dsp, win, PointerMotionMask);

	(void) warppointer(Dsp, win, 1, 4);
	XSync(Dsp, False);

	if (XCheckWindowEvent(Dsp, win, PointerMotionMask, &ev))
		return(False);
	else
		return(True);
}

>>CODE

	pointer_mode = GrabModeAsync;

	XCALL;

	if (ispfrozen() == False)
		CHECK;
	else {
		report("Pointer events were affected by GrabModeAsync");
		FAIL;
	}

	CHECKPASS(1);
>>ASSERTION Good A
When
.A pointer_mode
is
.S GrabModeSync ,
then state of the pointer as seen by client applications appears to freeze
and no further pointer events are generated
until the grabbing client issues a releasing
.S XAllowEvents
call or until the keyboard grab is released.
>>STRATEGY
Call xname with pointer_mode GrabModeSync.
Verify that pointer events are frozen.
>>CODE

	pointer_mode = GrabModeSync;

	XCALL;

	if (ispfrozen() == True)
		CHECK;
	else {
		report("Pointer events were not frozen by GrabModeSync");
		FAIL;
	}

	CHECKPASS(1);
>>ASSERTION Good A
When the pointer is frozen, then the actual pointer changes are
not lost and are processed after the grab is released or the client
calls
.F XAllowEvents .
>>STRATEGY
Call xname with pointer_mode GrabModeSync.
Warp pointer to create some pointer events.
Check that they are not received yet.
Release grab.
Verify that events are now received.
>>CODE
XEvent	ev;

	(void) warppointer(display, grab_window, 0, 0);
	XSelectInput(display, grab_window, PointerMotionMask);

	pointer_mode = GrabModeSync;
	XCALL;

	(void) warppointer(display, grab_window, 1, 1);
	XSync(display, False);	/* warppointer has effectivly done this */
	if (!XCheckWindowEvent(display, grab_window, PointerMotionMask, &ev))
		CHECK;
	else {
		report("Events were received while pointer was frozen");
		FAIL;
	}

	XUngrabKeyboard(display, CurrentTime);
	XSync(display, False);

	if (XCheckWindowEvent(display, grab_window, PointerMotionMask, &ev))
		CHECK;
	else {
		report("Events were not saved while pointer was frozen");
		FAIL;
	}

	CHECKPASS(2);
>>ASSERTION Good A
When the event window for an active grab becomes not viewable, then
the grab is released automatically.
>>STRATEGY
Call xname with pointer_mode GrabModeSync to freeze pointer.
Unmap the grab_window.
Verify that pointer is unfrozen, and that therefore the grab has
been released.
>>CODE

	pointer_mode = GrabModeSync;
	XCALL;

	XUnmapWindow(display, grab_window);
	if (isdeleted())
		return;

	if (ispfrozen()) {
		report("Grab was not released when grab_window was unmapped");
		FAIL;
	} else
		CHECK;

	CHECKPASS(1);
>>ASSERTION Good A
A successful call to xname sets the last-keyboard-grab time
to the specified time, with
.S CurrentTime
being replaced by the current X server time.
>>STRATEGY
Get a server time.
Use this time in the xname call with a pointer_mode of GrabModeSync.
Check that pointer is frozen.
Call XUngrabKeyboard with time-1.
Verify that pointer is still frozen.
Call XUngrabKeyboard with time.
Verify that pointer is released.
>>CODE

	thetime = gettime(display);
	pointer_mode = GrabModeSync;
	XCALL;

	if (ispfrozen())
		CHECK;
	else {
		delete("Could not freeze pointer");
		return;
	}

	XUngrabKeyboard(display, thetime-1);
	if (ispfrozen())
		CHECK;
	else {
		report("Last-keyboard-grab time not set correctly");
		FAIL;
	}
	XUngrabKeyboard(display, thetime);
	if (ispfrozen()) {
		report("Last-keyboard-grab time not set correctly");
		FAIL;
	} else
		CHECK;

	CHECKPASS(3);
>>ASSERTION Bad A
When the keyboard is actively grabbed by some other client,
then a call to xname fails and returns
.S AlreadyGrabbed .
>>STRATEGY
Create client2.
Call xname with default client.
Attempt to call xname with client2.
Verify that xname fails with AlreadyGrabbed.
>>CODE
int 	ret;
Display	*client2;

	client2 = opendisplay();

	XCALL;

	display = client2;
	ret = XCALL;

	if (ret == AlreadyGrabbed)
		CHECK;
	else {
		report("Return value was %s, expecting AlreadyGrabbed",
			grabreplyname(ret));
		FAIL;
	}

	CHECKPASS(1);
>>ASSERTION Bad A
When the
.A grab_window
is not viewable,
then a call to xname fails and returns
.S GrabNotViewable .
>>STRATEGY
Unmap grab_window.
Call xname.
Verify that xname fails with GrabNotViewable.
>>CODE
int 	ret;

	XUnmapWindow(display, grab_window);

	ret = XCALL;
	if (ret == GrabNotViewable)
		CHECK;
	else {
		report("Return value was %s, expecting GrabNotViewable",
			grabreplyname(ret));
		FAIL;
	}

	CHECKPASS(1);
>>ASSERTION Bad A
When the keyboard is frozen by an active grab of another client,
then a call to xname fails and returns
.S GrabFrozen .
>>STRATEGY
Grab and freeze keyboard with default client using XGrabPointer.
Create client2.
Call xname with client2.
Verify that xname returns GrabFrozen.
>>CODE
int 	ret;
Display	*client2;

	client2 = opendisplay();

	display = Dsp;
	XGrabPointer(Dsp, grab_window, False, 0, GrabModeAsync, GrabModeSync,
		None, None, CurrentTime);
	if (isdeleted())
		return;

	display = client2;
	ret = XCALL;
	if (ret == GrabFrozen)
		CHECK;
	else {
		report("Return value was %s, expecting GrabFrozen",
			grabreplyname(ret));
		FAIL;
	}

	CHECKPASS(1);
>>ASSERTION Bad A
When the specified time is earlier than the last-keyboard-grab time
or later than the current X server time,
then a call to xname fails and returns
.S GrabInvalidTime .
>>STRATEGY
Get current time.
Grab keyboard using this time to set last-keyboard-grab time.
Attempt to grab keyboard with an earlier time.
Verify that xname returns GrabInvalidTime.

Attempt to grab keyboard with a future time.
Verify that xname returns GrabInvalidTime.
>>CODE
int 	ret;

	thetime = gettime(display);

	/* Set the last-keyboard-grab time */
	XCALL;
	XUngrabKeyboard(display, thetime);

	thetime -= 43;
	ret = XCALL;
	if (ret == GrabInvalidTime)
		CHECK;
	else {
		report("Trying time earlier than last-keyboard-grab time");
		report("Return value was %s, expecting GrabInvalidTime",
			grabreplyname(ret));
		FAIL;
	}
	XUngrabKeyboard(display, thetime);

	thetime = gettime(display);
	thetime += ((config.speedfactor+1) * 1000000);
	ret = XCALL;
	if (ret == GrabInvalidTime)
		CHECK;
	else {
		report("Trying time later than current X server time");
		report("Return value was %s, expecting GrabInvalidTime",
			grabreplyname(ret));
		FAIL;
	}

	CHECKPASS(2);
>>ASSERTION Bad A
.ER BadValue owner_events True False
>>ASSERTION Bad A
.ER BadValue pointer_mode GrabModeSync GrabModeAsync
>>ASSERTION Bad A
.ER BadValue keyboard_mode GrabModeSync GrabModeAsync
>>ASSERTION Bad A
.ER BadWindow

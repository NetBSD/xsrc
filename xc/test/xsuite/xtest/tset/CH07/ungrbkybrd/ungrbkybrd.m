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
 * $XConsortium: ungrbkybrd.m,v 1.9 94/04/17 21:06:57 rws Exp $
 */
>>TITLE XUngrabKeyboard CH07
void

Display	*display = Dsp;
Time	thetime = CurrentTime;
>>ASSERTION Good B 3
When the client has actively grabbed the keyboard, then a call to xname
releases the keyboard and any queued events.
>>STRATEGY
Create grab window.
Grab and freeze keyboard.
If no extensions:
  Touch test xname.
else
  Press and release key.
  Call xname.
  Verify that events are released.
  Create new window.
  Verify that keyboard events can be received on it.
>>CODE
Window	win;
int 	key;
int 	n;
int 	first;
XEvent	ev;

	win = defwin(display);
	XSelectInput(display, win, KeyPressMask|KeyReleaseMask);

	XGrabKeyboard(display, win, False, GrabModeAsync, GrabModeSync, thetime);

	if (noext(0)) {
		XCALL;
		untested("There is no reliable test method, but a touch test was performed");
		return;
	} else
		CHECK;

	key = getkeycode(display);
	(void) warppointer(display, win, 10, 10);
	keypress(display, key);
	keyrel(display, key);

	if (XCheckMaskEvent(display, KeyPressMask|KeyReleaseMask, &ev)) {
		report("Got events while keyboard was meant to be frozen");
		FAIL;
	} else
		CHECK;

	XCALL;

	n = getevent(display, &ev);
	if (n != 2) {
		report("Expecting two events to be released after grab");
		report("  got %d", n);
		FAIL;
	} else {
		first = ev.type;
		(void) getevent(display, &ev);

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

	win = defwin(display);
	XSelectInput(display, win, KeyPressMask);
	(void) warppointer(display, win, 5, 5);
	keypress(display, key);
	if (XCheckWindowEvent(display, win, KeyPressMask, &ev))
		CHECK;
	else {
		report("Keyboard grab was not released");
		FAIL;
	}

	CHECKPASS(5);

	restoredevstate();

>>ASSERTION Good A
When the specified time is earlier than
the last-keyboard-grab time or is later than the current X server time,
then a call to xname does not release the keyboard and any queued events.
>>STRATEGY
Get current time.
Grab keyboard and freeze pointer with this time.
Call xname with earlier time.
Verify that pointer is still frozen and therefore keyboard grab is not released.

Get current time and add several seconds to get future time.
Call xname with this time.
Verify that pointer is still frozen and therefore keyboard grab is not released.
>>EXTERN

static Status
ispfrozen()
{
Window	win;
XEvent	ev;

	win = defwin(display);
	XSelectInput(display, win, PointerMotionMask);

	(void) warppointer(display, win, 0, 0);
	(void) warppointer(display, win, 1, 1);

	if (XCheckWindowEvent(display, win, PointerMotionMask, &ev))
		return(False);
	else
		return(True);
}

>>CODE
Window	win;

	win = defwin(display);
	thetime = gettime(display);

	XGrabKeyboard(display, win, False, GrabModeSync, GrabModeSync, thetime);

	thetime -= 1;
	XCALL;

	if (ispfrozen())
		CHECK;
	else {
		report("Grab released for time earlier than last-keyboard-grab time");
		FAIL;
	}

	thetime = gettime(display);
	thetime += (config.speedfactor+1) * 1000000;

	XCALL;

	if (ispfrozen())
		CHECK;
	else {
		report("Grab release for time later than current X server time");
		FAIL;
	}

	CHECKPASS(2);
>>ASSERTION Good A
A call to xname generates
.S FocusIn
and
.S FocusOut
events as though the focus were to change from the grab window to the
current focus window.
>>STRATEGY
Create grab window.
Create a focus window and set focus to that window.
Grab keyboard.
Enable events on windows.
Call xname to release keyboard.
Verify that FocusIn and FocusOut events are received.
>>CODE
Window	grabwin;
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

	grabwin = defwin(display);
	win = defwin(display);
	XSetInputFocus(display, win, RevertToNone, CurrentTime);
	if (isdeleted()) {
		report("Could not set up focus");
		return;
	}
	XGrabKeyboard(display, grabwin, False, GrabModeSync, GrabModeSync, thetime);

	XSelectInput(display, grabwin, FocusChangeMask);
	XSelectInput(display, win, FocusChangeMask);

	XCALL;

	/*
	 * Set up the expected good events.
	 */
	defsetevent(figood, display, FocusIn);
	figood.window = win;
	figood.mode = NotifyUngrab;
	figood.detail = NotifyNonlinear;

	defsetevent(fogood, display, FocusOut);
	fogood.window = grabwin;
	fogood.mode = NotifyUngrab;
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


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
 * $XConsortium: allwevnts.m,v 1.9 94/04/17 21:06:06 rws Exp $
 */
>>TITLE XAllowEvents CH07
void
xname(display, event_mode, thetime)
Display	*display = Dsp;
int 	event_mode = AsyncPointer;
Time	thetime = CurrentTime;
>>EXTERN

/*
 * The focus startup routines set the focus to the root window.
 */
>>SET startup focusstartup
>>SET cleanup focuscleanup

/*
 * A window for use as a grab window in the freeze and freezecheck
 * routines.
 */
static	Window	grabwin;

/*
 * Grab and freeze the pointer.
 */
static void
grabfreezepointer(disp, thetime)
Display	*disp;
Time	thetime;
{

	grabwin = defwin(disp);

	XGrabPointer(disp, grabwin, False, PointerMotionMask,
		GrabModeSync, GrabModeAsync, None, None, thetime);
}

/*
 * Return True if the pointer is frozen.  We move the pointer and check
 * the reported new position as returned by warppointer.  If this is the
 * same as the old position then the pointer is frozen.
 */
static	Bool
ispfrozen(disp)
Display	*disp;
{
PointerPlace	*pp;

	(void) warppointer(disp, grabwin, 0, 0);
	pp = warppointer(disp, grabwin, 1, 1);
	if (pp->ox == pp->nx)
		return True;
	else
		return False;
}

>>ASSERTION Good A
When the specified time is earlier than the last-grab
time of the most recent active grab for the client or
later than the current X server time, then a call to xname has no effect.
>>STRATEGY
Grab and freeze pointer with a given time.
Call xname with earlier time and AsyncPointer.
Verify that the pointer is still frozen.
Get current server time.
Call xname with a later time.
Verify that the pointer is still frozen.
>>CODE

	/* get time from the server */
	thetime = gettime(display);
	grabfreezepointer(display, thetime);

	thetime -= 100;
	XCALL;

	if (ispfrozen(display))
		CHECK;
	else {
		report("Events allowed when time was earlier than last-grab time");
		FAIL;
	}

	/*
	 * Get current time again and add several minutes to get a time in the
	 * future.
	 */
	thetime = gettime(display);
	thetime += ((config.speedfactor+1) * 1000000);
	XCALL;

	if (ispfrozen(display))
		CHECK;
	else {
		report("Events allowed when time was later than current server time");
		FAIL;
	}

	CHECKPASS(2);
>>ASSERTION Good A
When the
.A event_mode
argument is
.S AsyncPointer
and the pointer is frozen by the client,
then pointer event processing is resumed.
>>STRATEGY
Freeze pointer.
Call xname with event_mode AsyncPointer.
Verify that pointer is not frozen.
>>CODE

	grabfreezepointer(display, thetime);

	event_mode = AsyncPointer;
	XCALL;

	if (ispfrozen(display)) {
		report("Pointer was not released after AsyncPointer");
		FAIL;
	} else
		CHECK;

	CHECKPASS(1);
>>ASSERTION Good A
When the
.A event_mode
argument is
.S AsyncPointer
and the pointer is frozen twice by the client on behalf of two separate grabs,
then a call to xname thaws for both grabs.
>>STRATEGY
Freeze pointer with XGrabPointer.
Freeze pointer with XGrabKeyboard.
Call xname with event_mode AsyncPointer.
Verify that pointer is not frozen.
>>CODE

	grabfreezepointer(display, thetime);
	XGrabKeyboard(display, grabwin, False, GrabModeSync, GrabModeAsync,
		CurrentTime);

	if (isdeleted())
		return;

	event_mode = AsyncPointer;
	XCALL;

	if (ispfrozen(display)) {
		report("Pointer was not released from double grab after AsyncPointer");
		FAIL;
	} else
		CHECK;

	CHECKPASS(1);
>>ASSERTION Good A
When the
.A event_mode
argument is
.S AsyncPointer
and the pointer is not frozen by the client,
then a call to xname has no effect.
>>STRATEGY
Call xname with event_mode AsyncPointer.
Verify pointer is not frozen.
>>CODE

	event_mode = AsyncPointer;
	grabwin = defwin(display);

	XCALL;
	if (!ispfrozen(display))
		PASS;
	else {
		report("Pointer was frozen after AsyncPointer");
		FAIL;
	}
>>ASSERTION Good B 3
When the
.A event_mode
argument is
.S SyncPointer
and the pointer is frozen and actively grabbed by the client, then
pointer event processing is resumed normally until the next
.S ButtonPress
or
.S ButtonRelease
event is reported to the client, at which time
the pointer again appears to freeze, unless the reported event causes
the pointer grab to be released.
>>STRATEGY
Grab and freeze the pointer.
Call xname with event_mode SyncPointer.
Verify that pointer is not frozen.
If test extension available:
  Press a button.
  Verify that the pointer is frozen.
>>CODE
>>SET end-function restoredevstate

	grabfreezepointer(display, thetime);
	XChangeActivePointerGrab(display, ButtonPressMask | PointerMotionMask,
				 None, CurrentTime);
	if (ispfrozen(display))
		CHECK;
	else {
		delete("Could not freeze pointer");
		return;
	}

	event_mode = SyncPointer;
	XCALL;

	if (ispfrozen(display)) {
		report("Pointer was not released after SyncPointer");
		FAIL;
	} else
		CHECK;

	if (noext(1) || nbuttons() <= 1) {
		CHECKUNTESTED(2);
		return;
	}

	/* If extension we can go on */
	(void)warppointer(display, grabwin, 1, 2);
	buttonpress(display, Button1);

	if (ispfrozen(display))
		CHECK;
	else {
		report("Pointer was not re-frozen by a button press");
		FAIL;
	}

	CHECKPASS(3);

>>ASSERTION Good A
When the
.A event_mode
argument is
.S SyncPointer
and the pointer is not frozen by the client or the pointer is not grabbed by
the client,
then a call to xname has no effect.
>>STRATEGY
Call xname with event_mode SyncPointer.
Verify pointer is not frozen.
>>CODE

	event_mode = SyncPointer;
	grabwin = defwin(display);

	XCALL;
	if (!ispfrozen(display))
		PASS;
	else {
		report("Pointer was frozen after SyncPointer with no initial freeze");
		FAIL;
	}
>>ASSERTION Good A
When the
.A event_mode
argument is
.S SyncPointer
and the pointer is frozen twice by the client on behalf of two separate
grabs, then a call to xname thaws for both grabs.
>>STRATEGY
Freeze pointer with XGrabPointer.
Freeze pointer with XGrabKeyboard.
Call xname with event_mode SyncPointer.
Verify that pointer is not frozen.
>>CODE

	grabfreezepointer(display, thetime);
	XGrabKeyboard(display, grabwin, False, GrabModeSync, GrabModeAsync,
		CurrentTime);

	if (isdeleted())
		return;

	event_mode = SyncPointer;
	XCALL;

	if (ispfrozen(display)) {
		report("Pointer was not released after SyncPointer");
		FAIL;
	} else
		CHECK;

	CHECKPASS(1);
>>#NUM 008
>>ASSERTION Good B 3
When the
.A event_mode
argument is
.S ReplayPointer
and the pointer is actively grabbed by the client and is frozen as
the result of an event having been sent to the client, then
the pointer grab is released and the event is completely reprocessed
as though
any passive grabs at or above the grab window of the grab just released
were not present.
>>STRATEGY
Touch test for replay pointer.
If extensions are available:
  Create window.
  Create child of this window.
  Set passive grabs on both these windows.
  Warp pointer into child window.
  Press button to activate the grab.
  Check that parent window has the grab.
  Set event_mode to ReplayPointer.
  Call xname.
  Verify that the child window now has the grab.
>>CODE
XEvent  ev;
XButtonPressedEvent    *bpp;
Window  chwin;
struct  area    area;

	event_mode = ReplayPointer;
	if (noext(1)) {
		XCALL;

		untested("There is no reliable test method, but a touch test was performed");
		return;
	} else
		CHECK;

	/*
	 * Set up a pointer freeze as a result of a button press.
	 */
	grabwin = defwin(display);
	setarea(&area, 50, 50, 5, 5);
	chwin = crechild(display, grabwin, &area);

	/* XSelectInput(display, grabwin, ButtonPress); */
	XGrabButton(display, Button1, 0, grabwin,
		False, PointerMotionMask, GrabModeSync, GrabModeAsync,
		None, None);
	XGrabButton(display, Button1, 0, chwin,
		False, PointerMotionMask, GrabModeSync, GrabModeAsync,
		None, None);

	/*
	 * Activate the grab.
	 */
	XSync(display, True);	/* Discard any events */
	(void) warppointer(display, chwin, 1, 1);
	buttonpress(display, Button1);

	/*
	 * Check that the grab was activated and that it occurs on the parent
	 * window.
	 */
	if (XCheckMaskEvent(display, ButtonPressMask, &ev)) {
		bpp = (XButtonPressedEvent*)&ev;
		if (bpp->window == grabwin)
			CHECK;
		else if (bpp->window == chwin) {
			delete("Child window had the grab");
			return;
		} else {
			delete("Could not get grab on parent window");
			return;
		}
	} else {
		delete("Did not get a button event when trying to activate grab");
		return;
	}

	/* Do the ReplayPointer */
	XCALL;

	/*
	 * The effect should be as if the button were pressed again
	 * but without the passive grab on the parent window.  So this
	 * time the child should pick up the grab.
	 */
	if (XCheckMaskEvent(display, ButtonPressMask, &ev)) {
		bpp = (XButtonPressedEvent*)&ev;
		if (bpp->window == chwin)
			CHECK;
		else if (bpp->window == grabwin) {
			report("Parent window had the grab after a ReplayPointer");
			FAIL;
		} else {
			report("After ReplayPointer the grab on the child did not activate");
			FAIL;
		}
	} else {
		report("Did not get a button event when trying to activate grab");
		FAIL;
	}

	CHECKPASS(3);
>>ASSERTION Good A
When the
.A event_mode
argument is
.S ReplayPointer
and the pointer is not frozen as a result of an event
or the pointer is not grabbed by the client,
then a call to xname has no effect.
>>STRATEGY
Call xname with event_mode ReplayPointer.
Verify pointer is not frozen.
>>CODE

	event_mode = ReplayPointer;
	grabwin = defwin(display);

	XCALL;
	if (!ispfrozen(display))
		PASS;
	else {
		report("Pointer was frozen after ReplayPointer");
		FAIL;
	}
>>ASSERTION Good B 3
When the
.A event_mode
argument is
.S ReplayPointer
and the pointer is frozen twice by the client on behalf of two separate
grabs, then a call to xname thaws for both grabs.
>>STRATEGY
If extensions are available:
  Freeze pointer with XGrabKeyboard.
  Set up passive grab.
  Freeze pointer by activating grab with a button press.
  Call xname with event_mode of ReplayPointer.
  Verify that pointer was released.
>>CODE
int 	key;

	if (noext(1))
		return;

	grabwin = defwin(display);
	(void) warppointer(display, grabwin, 1, 1);

	key = getkeycode(display);
	XGrabKey(display, key, 0, grabwin, False, GrabModeSync, GrabModeAsync);
	XGrabButton(display, Button1, 0, grabwin,
		False, PointerMotionMask, GrabModeSync, GrabModeAsync,
		None, None);
	buttonpress(display, Button1);
	keypress(display, key);

	event_mode = ReplayPointer;
	XCALL;

	if (ispfrozen(display)) {
		report("Pointer was not released after ReplayPointer");
		report("  and the pointer was frozen by two grabs.");
		FAIL;
	} else
		CHECK;

	CHECKPASS(1);

>>ASSERTION Good B 3
When the
.A event_mode
argument is
.S AsyncKeyboard
and the keyboard is frozen by the client,
then keyboard event processing is resumed.
>>STRATEGY
If no extensions:
  Touch test for AsyncKeyboard.
else
  Grab and freeze Keyboard.
  Call xname with event_mode AsyncKeyboard.
  Verify that the keyboard is released.
>>EXTERN

/*
 * Returns True if the keyboard is frozen.
 */
static
iskfrozen(display)
Display	*display;
{
XEvent	ev;
Window	win;
int 	res;
int 	key;

	XSync(display, True); /* Flush previous events */
	key = getkeycode(display);

	/*
	 * Try to provoke a keypress on win.
	 */
	win = defwin(display);
	XSelectInput(display, win, KeyPressMask);
	(void) warppointer(display, win, 1, 1);
	keypress(display, key);
	if (XCheckMaskEvent(display, (long)KeyPressMask, &ev))
		res = False;
	else
		res = True;

	return(res);
}
>>CODE

	if (noext(0)) {
		event_mode = AsyncKeyboard;
		XCALL;

		untested("There is no reliable test method, but a touch test was performed");
		return;
	} else
		CHECK;

	grabwin = defwin(display);
	XGrabKeyboard(display, grabwin, False, GrabModeAsync, GrabModeSync,
		CurrentTime);
	if (iskfrozen(display))
		CHECK;
	else {
		delete("Could not freeze keyboard");
		return;
	}

	event_mode = AsyncKeyboard;
	XCALL;
	if (iskfrozen(display)) {
		report("Keyboard was not released by AsyncKeyboard");
		FAIL;
	} else
		CHECK;

	CHECKPASS(3);

>>ASSERTION Good B 3
When the
.A event_mode
argument is
.S AsyncKeyboard
and the keyboard is frozen twice by the client on behalf of two separate grabs,
then a call to xname thaws for both.
>>STRATEGY
If extensions available:
  Freeze keyboard with XGrabPointer.
  Freeze keyboard with XGrabKeyboard.
  Call xname with AsyncKeyboard.
  Verify that keyboard is released.
>>CODE

	if (noext(0))
		return;

	grabwin = defwin(display);
	XGrabPointer(display, grabwin, False, PointerMotionMask,
		GrabModeAsync, GrabModeSync, None, None, thetime);
	XGrabKeyboard(display, grabwin, False, GrabModeAsync, GrabModeSync,
		CurrentTime);

	if (iskfrozen(display))
		CHECK;
	else {
		delete("Could not not set up keyboard grab");
		return;
	}

	event_mode = AsyncKeyboard;
	XCALL;

	if (iskfrozen(display)) {
		report("Keyboard was not released by AsyncKeyboard");
		report("  when it was frozen on behalf of two separate grabs.");
		FAIL;
	} else
		CHECK;

	CHECKPASS(2);
>>ASSERTION Good B 3
When the
.A event_mode
argument is
.S AsyncKeyboard
and the keyboard is not frozen by the client,
then a call to xname has no effect.
>>STRATEGY
If extensions available:
  Call xname with AsyncKeyboard.
  Verify that keyboard is not frozen.
>>CODE

	if (noext(0))
		return;

	event_mode = AsyncKeyboard;
	XCALL;
	if (iskfrozen(display)) {
		report("Keyboard was frozen by AsyncKeyboard");
		FAIL;
	} else
		CHECK;

	CHECKPASS(1);
>>ASSERTION Good B 3
When the
.A event_mode
argument is
.S SyncKeyboard
and the keyboard is frozen and actively grabbed by the client,
then keyboard event processing is resumed normally until the next
.S KeyPress
or
.S KeyRelease
event is reported to the client, at which time
the keyboard again appears to freeze unless the reported event
causes the keyboard grab to be released.
>>STRATEGY
If no extensions:
  Touch test for SyncKeyboard.
else
  Freeze keyboard.
  Call xname with event_mode of SyncKeyboard.
  Verify that keyboard is not frozen.
  Press key.
  Verify that keyboard is frozen.
>>CODE
int 	key;

	event_mode = SyncKeyboard;
	if (noext(0)) {
		XCALL;
		untested("There is no reliable test method, but a touch test was performed");
		return;
	} else
		CHECK;

	grabwin = defwin(display);
	XSelectInput(display, grabwin, KeyPressMask);
	XGrabKeyboard(display, grabwin, False, GrabModeAsync, GrabModeSync,
		CurrentTime);

	XCALL;
	if (iskfrozen(display)) {
		report("Keyboard remained frozen after a SyncKeyboard");
		FAIL;
	} else
		CHECK;

	key = getkeycode(display);
	keypress(display, key);

	if (iskfrozen(display))
		CHECK;
	else {
		report("Keyboard was not frozen by a keypress after a SyncKeyboard");
		FAIL;
	}

	CHECKPASS(3);
>>ASSERTION Good B 3
When the
.A event_mode
argument is
.S SyncKeyboard
and the keyboard is not frozen by the client or the keyboard is not
grabbed by the client,
then a call to xname has no effect.
>>STRATEGY
If extensions available:
  Call xname with event_mode of SyncKeyboard.
  Verify that keyboard is not frozen.
  Press key.
  Verify that keyboard is not frozen.
>>CODE

	if (noext(0))
		return;

	event_mode = SyncKeyboard;
	XCALL;

	if (iskfrozen(display)) {
		report("Keyboard was frozen by SyncKeyboard though there was no freeze in effect");
		FAIL;
	} else
		CHECK;

	keypress(display, getkeycode(display));
	if (iskfrozen(display)) {
		report("Keyboard was frozen by SyncKeyboard and a key press");
		report("  though there was no freeze in effect");
		FAIL;
	} else
		CHECK;

	CHECKPASS(2);
>>ASSERTION Good B 3
When the
.A event_mode
argument is
.S SyncKeyboard
and the keyboard is frozen twice by the client on behalf of two separate grabs,
then a call to xname thaws for both grabs.
>>STRATEGY
If extensions available:
  Freeze keyboard with XGrabPointer.
  Freeze keyboard with XGrabKeyboard.
  Call xname with event_mode of SyncKeyboard.
  Verify that keyboard is not frozen.
>>CODE

	if (noext(0))
		return;

	grabwin = defwin(display);
	XGrabPointer(display, grabwin, False, PointerMotionMask,
		GrabModeAsync, GrabModeSync, None, None, thetime);
	XGrabKeyboard(display, grabwin, False, GrabModeAsync, GrabModeSync,
		CurrentTime);

	event_mode = SyncKeyboard;
	XCALL;

	if (iskfrozen(display)) {
		report("Keyboard remained frozen after a SyncKeyboard with");
		report("  the keyboard frozen on behalf of two separate grabs");
		FAIL;
	} else
		CHECK;

	CHECKPASS(1);
>>ASSERTION Good B 3
When the
.A event_mode
argument is
.S ReplayKeyboard
and the keyboard is actively grabbed by the client and is frozen
as the result of an event having been sent to the client, then
the keyboard grab is released and the event is completely reprocessed
as though any passive grabs at or above
the grab window of the grab just released were not present.
>>STRATEGY
If no extensions:
  Touch test for ReplayKeyboard.
else
  Create window.
  Create child of that window.
  Enable key events on windows.
  Place passive grab with GrabModeSync on each window.
  Warp pointer to child window.
  Press key.
  Check that the parent window has the grab.

  Call xname with ReplayKeyboard.
  Verify that child window has grab.
>>CODE
XEvent	ev;
XKeyPressedEvent	*kpp;
Window	win;
Window	chwin;
struct	area	area;
int 	key;

	event_mode = ReplayKeyboard;

	if (noext(0)) {
		XCALL;
		untested("There is no reliable test method, but a touch test was performed");
		return;
	} else
		CHECK;

	win = defwin(display);
	setarea(&area, 50, 50, 5, 5);
	chwin = crechild(display, win, &area);
	XSelectInput(display, win, KeyPressMask);
	XSelectInput(display, chwin, KeyPressMask);

	key = getkeycode(display);
	XGrabKey(display, key, 0, win, False, GrabModeAsync, GrabModeSync);
	XGrabKey(display, key, 0, chwin, False, GrabModeAsync, GrabModeSync);

	(void) warppointer(display, chwin, 1, 1);

	XSync(display, True);
	/* This should activate the grab */
	keypress(display, key);

	if (XCheckMaskEvent(display, KeyPressMask, &ev)) {
		kpp = (XKeyPressedEvent*)&ev;
		if (kpp->window == win)
			CHECK;
		else if (kpp->window == chwin) {
			/*
			 * This could also be because owner_events=False is being
			 * ignored.
			 */
			delete("Child window had the grab, rather than parent");
			return;
		} else {
			delete("Parent did not get the grab");
			return;
		}
	} else {
		delete("Did not get KeyPress event");
		return;
	}
	XSync(display, True);	/* Discard any other events */

	XCALL;

	/*
	 * When the event is replayed then we should now get the event on
	 * the child window.
	 */
	if (XCheckMaskEvent(display, KeyPressMask, &ev)) {
		kpp = (XKeyPressedEvent*)&ev;
		if (kpp->window == chwin)
			CHECK;
		else if (kpp->window == win) {
			report("Parent window had the grab after ReplayPointer");
			report("  expecting that child window would get grab");
			FAIL;
		} else {
			report("Child did not get the grab after ReplayPointer");
			FAIL;
		}
	} else {
		report("Did not get KeyPress event after ReplayPointer");
		FAIL;
	}

	CHECKPASS(3);
>>ASSERTION Good B 3
When the
.A event_mode
argument is
.S ReplayKeyboard
and the keyboard is not frozen as a result of an event or the keyboard
is not grabbed by the client,
then a call to xname has no effect.
>>STRATEGY
If extensions available:
  Create window.
  Create child of that window.
  Place passive grab with GrabModeAsync on each window.
  Warp pointer to child window.
  Press key.
  Check that the parent window has the grab.

  Call xname with ReplayKeyboard.
  Verify that no key event was received on the child.
>>CODE
XEvent	ev;
XKeyPressedEvent	*kpp;
Window	win;
Window	chwin;
struct	area	area;
int 	key;

	event_mode = ReplayKeyboard;

	if (noext(0)) {
		XCALL;
		untested("There is no reliable test method, but a touch test was performed");
		return;
	} else
		CHECK;

	win = defwin(display);
	setarea(&area, 50, 50, 5, 5);
	chwin = crechild(display, win, &area);

	key = getkeycode(display);
	XGrabKey(display, key, 0, win, False, GrabModeAsync, GrabModeAsync);
	XGrabKey(display, key, 0, chwin, False, GrabModeAsync, GrabModeAsync);

	(void) warppointer(display, chwin, 1, 1);

	XSync(display, True);
	/* This should activate the grab */
	keypress(display, key);

	if (XCheckMaskEvent(display, KeyPressMask, &ev)) {
		kpp = (XKeyPressedEvent*)&ev;
		if (kpp->window == win)
			CHECK;
		else if (kpp->window == chwin) {
			delete("Child window had the grab");
			return;
		} else {
			delete("Parent did not get the grab");
			return;
		}
	} else {
		delete("Did not get KeyPress event");
		return;
	}
	XSync(display, True);	/* Discard any other events */

	XCALL;

	/*
	 * Should be no event after the ReplayKeyboard.
	 */
	if (XCheckMaskEvent(display, KeyPressMask, &ev)) {
		report("ReplayKeyboard had an effect when the keyboard was not frozen.");
		FAIL;
	} else
		CHECK;

	CHECKPASS(3);
>>ASSERTION Good B 3
When the
.A event_mode
argument is
.S ReplayKeyboard
and the keyboard is frozen twice by the client on behalf of two separate grabs,
then a call to xname thaws for both grabs.
>>STRATEGY
If extensions available:
  Freeze keyboard with XGrabButton.
  Freeze keyboard with XGrabKey.
  Call xname with event_mode of ReplayKeyboard.
  Verify that keyboard is not frozen.
>>CODE
int	key;

	if (noext(1))
		return;

	grabwin = defwin(display);
	(void) warppointer(display, grabwin, 1, 1);

	XGrabButton(display, Button1, 0, grabwin,
		False, PointerMotionMask, GrabModeSync, GrabModeAsync,
		None,  None);

	key = getkeycode(display);
	XGrabKey(display, key, 0, grabwin, False, GrabModeSync, GrabModeAsync);

	keypress(display, key);
	buttonpress(display, Button1);

	event_mode = ReplayKeyboard;
	XCALL;

	if (iskfrozen(display)) {
		report("Keyboard was not unfrozen by ReplayKeyboard when frozen");
		report("  on behalf of two separate grabs.");
		FAIL;
	} else
		CHECK;

	CHECKPASS(1);

	restoredevstate();

>>ASSERTION Good B 3
When the
.A event_mode
argument is
.S SyncBoth
and both pointer and keyboard are frozen by the client,
then event processing for both devices continues normally until the next
.S ButtonPress ,
.S ButtonRelease ,
.S KeyPress ,
or
.S KeyRelease
event is reported to the client for a grabbed device
at which time both devices again appear to freeze,
unless the reported event causes the grab to be released.
>>STRATEGY
If no extensions:
  Touch test for SyncBoth.
else
  Create grabwindow.
  Select events on grab window.
  Freeze and grab pointer by calling XGrabPointer.
  Freeze and grab keyboard by calling XGrabKeyboard.
  Check that pointer is frozen.

  Call xname with event_mode of SyncBoth.
  Verify that pointer has been released.

  Press button.
  Verify that pointer is frozen.
  Verify that keyboard is frozen.

  Call xname with event_mode of SyncBoth.
  Check pointer released.
  Release button.
  Verify that pointer is frozen.
  Verify that keyboard is frozen.

  Call xname with event_mode of SyncBoth.
  Check pointer released.
  Press key.
  Verify that pointer is frozen.
  Verify that keyboard is frozen.

  Call xname with event_mode of SyncBoth.
  Check pointer released.
  Release key.
  Verify that pointer is frozen.
  Verify that keyboard is frozen.
>>EXTERN

/*
 * Set up for SyncBoth tests grab and freeze both pointer and keyboard.
 */
bothset()
{
	XUngrabPointer(display, CurrentTime);
	XUngrabKeyboard(display, CurrentTime);

	grabwin = defwin(display);
	XSelectInput(display, grabwin, ButtonPressMask|ButtonReleaseMask|KeyPressMask|KeyReleaseMask);
	(void) warppointer(display, grabwin, 5, 5);

	XGrabPointer(display, grabwin, False,
	 	ButtonPressMask|ButtonReleaseMask|PointerMotionMask,
		GrabModeSync, GrabModeAsync, None, None, CurrentTime);
	XGrabKeyboard(display, grabwin, False, GrabModeAsync, GrabModeSync,
		CurrentTime);


	if (!ispfrozen(display)) {
		delete("Could not freeze pointer");
		return;
	}
	/*
	 * Can't check for the keyboard being frozen here since that requires
	 * pressing a key - and that would release the grab.
	 */
}

>>CODE
int 	key;

	event_mode = SyncBoth;

	if (noext(1)) {
		XCALL;

		untested("There is no reliable test method, but a touch test was performed");
		return;
	} else
		CHECK;


	bothset();
	XCALL;

	if (ispfrozen(display)) {
		report("SyncBoth did not release pointer and keyboard");
		FAIL;
	} else
		CHECK;

	/* 1. Button press */
	buttonpress(display, Button1);
	if (ispfrozen(display))
		CHECK;
	else {
		report("Pointer was not re-frozen by a button press after SyncBoth");
		FAIL;
	}
	if (iskfrozen(display))
		CHECK;
	else {
		report("Keyboard was not re-frozen by a button press after SyncBoth");
		FAIL;
	}

	/* Allow events again for next part */
	bothset();
	XCALL;
	if (ispfrozen(display)) {
		report("SyncBoth did not release pointer and keyboard");
		FAIL;
	} else
		CHECK;

	/* 2. Button release */
	buttonrel(display, Button1);
	if (ispfrozen(display))
		CHECK;
	else {
		report("Pointer was not re-frozen by a button release after SyncBoth");
		FAIL;
	}
	if (iskfrozen(display))
		CHECK;
	else {
		report("Keyboard was not re-frozen by a button release after SyncBoth");
		FAIL;
	}

	/* Allow events again for next part */
	bothset();
	XCALL;
	if (ispfrozen(display)) {
		report("SyncBoth did not release pointer and keyboard");
		FAIL;
	} else
		CHECK;

	/* 3. Press key. */
	key = getkeycode(display);
	keypress(display, key);
	if (ispfrozen(display))
		CHECK;
	else {
		report("Pointer was not re-frozen by a key press after SyncBoth");
		FAIL;
	}
	if (iskfrozen(display))
		CHECK;
	else {
		report("Keyboard was not re-frozen by a key press after SyncBoth");
		FAIL;
	}

	/* Allow events again for next part */
	bothset();
	XCALL;
	if (ispfrozen(display)) {
		report("SyncBoth did not release pointer and keyboard");
		FAIL;
	} else
		CHECK;

	/* 4. Key release. */
	keyrel(display, key);
	if (ispfrozen(display))
		CHECK;
	else {
		report("Pointer was not re-frozen by a key release after SyncBoth");
		FAIL;
	}
	if (iskfrozen(display))
		CHECK;
	else {
		report("Keyboard was not re-frozen by a key release after SyncBoth");
		FAIL;
	}

	CHECKPASS(13);
>>ASSERTION Good B 3
When the
.A event_mode
argument is
.S SyncBoth
and an event has caused the grab on one device to be released
and a subsequent event is reported for the other device that does
not cause the grab to be released,
then both devices are again frozen.
>>STRATEGY
If extension available:
  Grab and freeze Keyboard with XGrabKeyboard.
  Set passive grab on button with pointer_mode GrabModeSync.
  Activate pointer grab.

  Call xname with event_mode SyncBoth.
  Release button to release pointer grab.
  Check pointer not frozen.

  Press key.
  Verify that pointer and keyboard are frozen.
>>CODE

	if (noext(1))
		return;

	grabwin = defwin(display);
	XGrabKeyboard(display, grabwin, False, GrabModeAsync, GrabModeSync,
		CurrentTime);
	XGrabButton(display, Button1, 0, grabwin,
		False, PointerMotionMask, GrabModeSync, GrabModeAsync,
		None, None);

	(void) warppointer(display, grabwin, 1, 1);
	buttonpress(display, Button1);

	if (ispfrozen(display))
		CHECK;
	else {
		delete("Could not freeze pointer and keyboard");
		return;
	}

	event_mode = SyncBoth;
	XCALL;

	/*
	 * Release pointer grab.
	 */
	buttonrel(display, Button1);
	if (ispfrozen(display)) {
		report("Pointer remained frozen after releasing button");
		FAIL;
	} else
		CHECK;

	keypress(display, getkeycode(display));
	if (ispfrozen(display))
		CHECK;
	else {
		report("Pointer was not re-frozen by an event from the keyboard after the pointer grab was released.");
		FAIL;
	}
	if (iskfrozen(display))
		CHECK;
	else {
		report("Keyboard was not re-frozen by an event from the keyboard after the pointer grab was released");
		FAIL;
	}

	CHECKPASS(4);
>>ASSERTION Good B 3
When the
.A event_mode
argument is
.S SyncBoth
and the pointer or keyboard is frozen twice
by the client on behalf of two separate grabs,
then a call to xname thaws for both grabs.
>>STRATEGY
If extensions are available:
  Grab and freeze pointer.
  Grab keyboard and freeze pointer.

  Call xname with event_mode of SyncBoth.
  Verify that pointer and keyboard are thawed.
>>CODE

	if (noext(0))
		return;

	grabwin = defwin(display);
	XGrabPointer(display, grabwin, False, PointerMotionMask,
		GrabModeSync, GrabModeSync, None, None, thetime);
	XGrabKeyboard(display, grabwin, False, GrabModeSync, GrabModeSync,
		CurrentTime);

	event_mode = SyncBoth;
	XCALL;

	if (ispfrozen(display)) {
		report("Pointer was not thawed by SyncBoth when pointer was frozen");
		report("  on behalf of two grabs");
		FAIL;
	} else
		CHECK;
	if (iskfrozen(display)) {
		report("Keyboard was not thawed by SyncBoth when pointer was frozen");
		report("  on behalf of two grabs");
		FAIL;
	} else
		CHECK;

	CHECKPASS(2);
>>ASSERTION Good B 3
When the
.A event_mode
argument is
.S SyncBoth
and either the keyboard or pointer is not frozen by the client
or neither are grabbed by the client,
then a call to xname has no effect.
>>STRATEGY
If extensions available:
  Call xname with SyncBoth.
  Press button.
  Verify that pointer and keyboard are not frozen.
>>CODE

	if (noext(0))
		return;

	grabwin = defwin(display);

	event_mode = SyncBoth;
	XCALL;

	buttonpress(display, Button1);
	if (ispfrozen(display)) {
		report("Pointer was frozen by button press after SyncBoth");
		report("  even though there were no grabs active");
		FAIL;
	} else
		CHECK;
	if (iskfrozen(display)) {
		report("Keyboard was frozen by button press after SyncBoth");
		report("  even though there were no grabs active");
		FAIL;
	} else
		CHECK;

	CHECKPASS(2);
>>ASSERTION Good B 3
When the
.A event_mode
argument is
.S AsyncBoth
and the pointer and the keyboard are frozen by the
client, then event processing for both devices is resumed normally.
>>STRATEGY
If no extensions:
  Touch test for AsyncBoth.
else
  Grab and freeze keyboard and pointer.
  Call xname with AsyncBoth.
  Verify that pointer is released.
  Verify that keyboard is released.
>>CODE

	event_mode = AsyncBoth;

	if (noext(0)) {
		XCALL;
		untested("There is no reliable test method, but a touch test was performed");
		return;
	} else
		CHECK;

	bothset();
	XCALL;

	if (ispfrozen(display)) {
		report("Pointer remained frozen after AsyncBoth");
		FAIL;
	} else
		CHECK;
	if (iskfrozen(display)) {
		report("Keyboard remained frozen after AsyncBoth");
		FAIL;
	} else
		CHECK;

	CHECKPASS(3);
>>ASSERTION Good B 3
When the
.A event_mode
argument is
.S AsyncBoth
and either the keyboard or the pointer is not frozen by the client,
then a call to xname has no effect.
>>STRATEGY
If extensions available:
  Grab and freeze pointer.
  Call xname with AsyncBoth.
  Verify that pointer is not released.
>>CODE

	if (noext(0))
		return;

	grabfreezepointer(display, CurrentTime);

	event_mode = AsyncBoth;
	XCALL;

	if (ispfrozen(display))
		CHECK;
	else {
		report("Pointer was released by AsyncBoth, although keyboard was not frozen");
		FAIL;
	}
	CHECKPASS(1);
>>ASSERTION Good B 3
When the
.A event_mode
argument is
.S AsyncBoth
and the pointer or keyboard
is frozen twice by the client on behalf of two separate grabs,
then a call to xname thaws for both grabs.
>>STRATEGY
If extensions available:
  Freeze pointer and keyboard by calling XGrabPointer.
  Freeze pointer and keyboard again by calling XGrabKeyboard.

  Call xname with AsyncBoth.
  Verify that pointer and keyboard are not frozen.
>>CODE

	if (noext(0))
		return;

	grabwin = defwin(display);
	XGrabPointer(display, grabwin, False, PointerMotionMask,
		GrabModeSync, GrabModeSync, None, None, thetime);
	XGrabKeyboard(display, grabwin, False, GrabModeSync, GrabModeSync,
		CurrentTime);

	event_mode = AsyncBoth;
	XCALL;

	if (ispfrozen(display)) {
		report("Pointer remained frozen after AsyncBoth");
		report("  when it was frozen twice");
		FAIL;
	} else
		CHECK;
	if (iskfrozen(display)) {
		report("Keyboard remained frozen after AsyncBoth");
		report("  when it was frozen twice");
		FAIL;
	} else
		CHECK;

	CHECKPASS(2);
>>ASSERTION Good B 3
When the
.A event_mode
argument is
.S AsyncPointer ,
.S SyncPointer ,
or
.S ReplayPointer ,
then a call to xname has no effect on the
processing of keyboard events.
>>STRATEGY
If extensions are available:
  Grab and freeze the keyboard.
  For each event_mode AsyncPointer SyncPointer ReplayPointer
    Call xname.
    Verify that keyboard is still frozen.
>>CODE
static	int 	modes[] = {
	AsyncPointer, SyncPointer, ReplayPointer};
int 	i;

	if (noext(0))
		return;

	grabwin = defwin(display);
	XGrabKeyboard(display, grabwin, False, GrabModeAsync, GrabModeSync,
		CurrentTime);

	for (i = 0; i < NELEM(modes); i++) {
		event_mode = modes[i];
		XCALL;
		if (iskfrozen(display))
			CHECK;
		else {
			report("Keyboard was released when event_mode was %s",
				alloweventmodename(modes[i]));
			FAIL;
		}
	}

	CHECKPASS(NELEM(modes));
>>ASSERTION Good A
When the
.A event_mode
argument is
.S AsyncKeyboard ,
.S SyncKeyboard ,
or
.S ReplayKeyboard ,
then a call to xname has no effect on the
processing of pointer events.
>>STRATEGY
Grab and freeze pointer.
For each event_mode AsyncKeyboard SyncKeyboard ReplayKeyboard
  Call xname.
  Verify that pointer is still frozen.
>>CODE
static	int 	modes[] = {
	AsyncKeyboard, SyncKeyboard, ReplayKeyboard};
int 	i;

	grabfreezepointer(display, thetime);

	for (i = 0; i < NELEM(modes); i++) {
		event_mode = modes[i];
		XCALL;
		if (ispfrozen(display))
			CHECK;
		else {
			report("Pointer was released when event_mode was %s",
				alloweventmodename(modes[i]));
			FAIL;
		}
	}

	CHECKPASS(NELEM(modes));
>>ASSERTION Bad A
.ER BadValue event_mode AsyncPointer SyncPointer AsyncKeyboard SyncKeyboard ReplayPointer ReplayKeyboard AsyncBoth SyncBoth

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
 * $XConsortium: grbbttn.m,v 1.14 94/04/17 21:06:21 rws Exp $
 */
>>TITLE XGrabButton CH07
void
xname
Display	*display = Dsp;
unsigned int	button = Button1;
unsigned int	modifiers = 0;
Window	grab_window = defwin(Dsp);
Bool	owner_events = False;
unsigned int	event_mask = PointerMotionMask;
int 	pointer_mode = GrabModeAsync;
int 	keyboard_mode = GrabModeAsync;
Window	confine_to = None;
Cursor	cursor = None;
>>SET end-function restoredevstate
>>EXTERN

/*
 * Returns True if the pointer is grabbed.  This is not a general purpose
 * routine since it knows about the Grab Button args.
 * When the pointer is grabbed then pointer events are not reported to
 * non grabbing clients.
 */
static
pgrabbed()
{
Window	win;
Display	*client2;
XEvent	ev;

	if (confine_to != None)
		win = confine_to;
	else
		win = grab_window;

	client2 = opendisplay();
	
	XSelectInput(client2, win, PointerMotionMask|EnterWindowMask);
	/* Flush events for client2 */
	XSync(client2, True);

	/*
	 * Ensure that pointer either enters or moves within win.
	 */
	(void) warppointer(display, win, 1, 1);
	(void) warppointer(display, win, 1, 2);

	XSync(client2, False);
	if (getevent(client2, &ev)) {
		/*
		 * An event was reported - pointer isn't grabbed, do a sanity
		 * check on the type of event.
		 */
		if (ev.type != MotionNotify && ev.type != EnterNotify) {
			delete("Unexpected event received in pgrabbed()");
			delete("  event type %s", eventname(ev.type));
		}
		return(False);
	} else {
		return(True);
	}
}

/*
 * Get the window that the pointer is currently in, if the pointer
 * is in a child of the given window. Otherwise it returns None.
 */
static Window
getpointerwin(disp, win)
Display	*disp;
Window	win;
{
Window	child;
Window	wtmp;
int 	itmp;
unsigned uitmp;
Bool 	s;

	s = XQueryPointer(disp, win, &wtmp, &child, &itmp, &itmp
		, &itmp, &itmp , &uitmp);

	if (!s)
		delete("Could not get pointer window");

	return(child);
}

#define	ACTPOSX	3
#define	ACTPOSY	6
/*
 * Warp to the grab_window and press the keys in modifiers and then press
 * the button in the 'button' arg.
 * (This activates the previously set up grab if the arg variables
 * have not been changed.)
 */
static
activate()
{
	(void) warppointer(display, grab_window, ACTPOSX, ACTPOSY);
	if (modifiers)
		modpress(display, modifiers);
	buttonpress(display, button);
}

/*
 * Returns True if the pointer is frozen.
 */
static
pfrozen()
{
XEvent	ev;
Window	win;

	if (confine_to != None) {
		report("ERROR");
		delete("pfrozen should not be used with a confine_to win");
		return False;
	}

	XSync(display, True);	/* Flush events */

	win = defwin(display);
	XSelectInput(display, win, PointerMotionMask);
	(void) warppointer(display, win, 1, 1);
	(void) warppointer(display, win, 2, 2);
	if (XCheckMaskEvent(display, (long)PointerMotionMask, &ev))
		return False;
	else
		return True;
}

/*
 * Returns True if the keyboard is frozen.
 */
static
kfrozen()
{
XEvent	ev;
Window	win;
int 	minkc, maxkc;
int 	res;

	XSync(display, True); /* Flush previous events */
	XDisplayKeycodes(display, &minkc, &maxkc);
	if (minkc < 8)
		minkc = 8;	/* For buggy servers */

	/*
	 * Try to provoke a keypress on win.
	 */
	win = defwin(display);
	XSelectInput(display, win, KeyPressMask);
	(void) warppointer(display, win, 1, 1);
	keypress(display, minkc);
	if (XCheckMaskEvent(display, (long)KeyPressMask, &ev))
		res = False;
	else
		res = True;

	keyrel(display, minkc);
	return(res);
}

>>ASSERTION Good B 3
A call to xname establishes a passive grab that is activated in the future
by the specified
.A button
being logically pressed,
the modifier keys given by
.A modifiers
being logically down,
no other buttons or modifier keys being logically down and
the pointer being contained in the
.A grab_window .
>>STRATEGY
Call xname as touch test.
If extensions available:
  Warp pointer into grab window.
  Simulate a button press of button.
  Verify that pointer is now grabbed.
  Release grab.

  Set up a grab with xname for a button.
  Warp pointer into grab window.
  Simulate press of another button.
  Simulate the button press.
  Verify that the pointer is not grabbed.
  Release grab.

  Set up a grab with xname for a button and modifier keys.
  Warp pointer to grab window.
  Simulate modifier key presses.
  Simulate the button press.
  Verify that the pointer is grabbed.
  Release grab.

  Set up a grab with xname for a button and modifier keys.
  Warp pointer to grab window.
  Simulate modifier key presses.
  Simulate extra modifier key presses.
  Simulate the button press.
  Verify that the pointer is not grabbed.
  Release grab.
>>CODE
unsigned int 	mask;
int 	onemod;

	if (pgrabbed()) { /* Sanity check */
		delete("Pointer seemed to be grabbed before doing test");
		return;
	}

	XCALL;

	if (noext(1)) {
		untested("There is no reliable test method, but a touch test was performed");
		return;
	}

	/*
	 * --- Simple case no modifiers.
	 */
	activate();

	if (pgrabbed()) {
		CHECK;
	} else {
		report("Pointer was not grabbed after button press");
		FAIL;
	}
	relalldev();

	/*
	 * --- Press another button first and then press the grabbed button.
	 * The pointer should not be grabbed.
	 */
	if (nbuttons() > 1) {
		button = Button1;
		XCALL;

		(void) warppointer(display, grab_window, 2, 2);
		buttonpress(display, button+1);
		buttonpress(display, button);
		if (pgrabbed()) {
			report("Pointer was grabbed although another button was pressed");
			FAIL;
		} else
			CHECK;
		relalldev();
	} else {
		trace("Only one button supported");
		CHECK;
	}

	/*
	 * --- Set up a grab with modifiers.
	 */
	modifiers = wantmods(display, 2);
	grab_window = defwin(display);
	trace("Grabbing %s with mods %s", buttonname((int)button),
		keymaskname((unsigned long)modifiers));
	XCALL;

	activate();
	if (pgrabbed()) {
		CHECK;
	} else {
		report("Pointer was not grabbed for %s and %s", buttonname((int)button),
			keymaskname((unsigned long)modifiers));
		FAIL;
	}
	relalldev();

	/*
	 * --- Set up a grab with modifiers, try to activate the grab with
	 * too many modifiers held down.  Grab should not become active.
	 */
	mask = wantmods(display, 2);
	for (onemod = 1; onemod; onemod <<= 1) {
		if (mask & onemod)
			break;
	}
	/* Only assumes one modifier */
	modifiers = mask & ~onemod;
	grab_window = defwin(display);
	XCALL;

	(void) warppointer(display, grab_window, 2, 2);
	/* Pressing an extra modifier */
	modpress(display, mask);
	buttonpress(display, button);

	if (mask && pgrabbed()) {
		report("Pointer was grabbed when there were extra modifier keys down");
		FAIL;
	} else {
		/*
		 * If mask was zero this means that there are no modifiers KeyCodes
		 * available.  This is unlikely and really means that we cannot test
		 * this part of the assertion.  However in this case this part of
		 * the assertion has no meaning so say it passes.
		 */
		CHECK;
	}
	
	CHECKPASS(4);
>>ASSERTION Good B 3
When the conditions for activating the grab are otherwise satisfied
and the pointer is already grabbed, then no active grab is established.
>>STRATEGY
If extensions are available:
  Create two windows that do not overlap.
  Call xname with one of the windows as the confine_to.
  Activate grab with a button press.
  Check that pointer is within that window.
  Call xname with confine_to as the other window.
  Press button in grab_window.
  Verify that pointer has not been warped to second confine_to window.
else
  Report untested.
>>CODE
Window	win1, win2;
Window	win;

	if (noext(1))
		return;

	win1 = defwin(display);
	win2 = defwin(display);

	confine_to = win1;
	XCALL;

	activate();

	if (getpointerwin(display, DRW(display)) == confine_to)
		CHECK;
	else {
		delete("Could not set up first grab");
		return;
	}

	confine_to = win2;
	if (nbuttons() > 1)
		button = Button2;
	grab_window = defwin(display);
	XCALL;

	activate();

	win = getpointerwin(display, DRW(display));
	if (win == win1)
		CHECK;
	else if (win == win2) {
		report("A second grab became active while another was active");
		FAIL;
	} else {
		/* Our test went unexpectedly wrong */
		delete("Pointer in unexpected window");
	}

	CHECKPASS(2);
>>ASSERTION Good B 3
When the conditions for activating the grab are otherwise satisfied
and the
.A confine_to
window is not viewable, then no active grab is established.
>>STRATEGY
If extensions are available:
  Create a confine_to window.
  Unmap the confine to window.
  Set up a passive grab.
  Move pointer to grab window.
  Attempt to activate grab by simulating a button press.
  Verify that grab is not activated.
else
  Report untested.
>>CODE

	if (noext(1))
		return;

	confine_to = defwin(display);
	XUnmapWindow(display, confine_to);

	XCALL;

	activate();

	/*
	 * Since confine_to is not viewable then the pointer should still
	 * be in grab_window.
	 */
	if (getpointerwin(display, DRW(display)) != grab_window) {
		report("Pointer was warped out of the grab_window");
		FAIL;
	}

	confine_to = None;	/* Unmapped so no use for pgrabbed() */
	if (!pgrabbed())
		CHECK;
	else {
		report("Pointer was grabbed when confine_to was not viewable");
		FAIL;
	}

	CHECKPASS(1);
>>ASSERTION Good B 3
When the conditions for activating the grab are otherwise satisfied
and a passive grab on the same button/key combination exists for an
ancestor of
.A grab_window ,
then no active grab is established.
>>STRATEGY
If extensions are available:
  Call xname to place a passive grab.
  Create a child of the grab_window.
  Place a passive grab for the same key/button combination on the child.
  Move pointer into the child.
  Attempt to activate grab by simulating button press.
  Verify that pointer is not grabbed by the child window.
else
  Report untested.
>>CODE
struct	area	area;
Window	parent;
XEvent	ev;

	if (noext(1))
		return;

	modifiers = wantmods(display, 2);
	event_mask = PointerMotionMask;
	XCALL;

	parent = grab_window;
	setarea(&area, 10, 10, 20, 24);
	grab_window = crechild(display, grab_window, &area);

	XCALL;

	activate();

	/*
	 * Provoke an event by moving the pointer on the child window,  since
	 * this child grab should not have become active then no events should
	 * be received on the child.
	 * However the assertion does not apply to the parent, so the grab will
	 * have become active on the parent -- therefore we expect the events
	 * to show up on the parent.
	 */
	(void) warppointer(display, grab_window, 1, 1);
	(void) warppointer(display, grab_window, 8, 8);

	/*
	 * Since no events are selected on the windows, any event must
	 * be the result of a grab.
	 */
	if (XCheckWindowEvent(display, grab_window, (long)event_mask, &ev)) {
		report("Grab was activated on a window which had an ancestor with the same grab");
		FAIL;
	} else
		CHECK;

	if (XCheckWindowEvent(display, parent, (long)event_mask, &ev))
		CHECK;
	else {
		report("Grab was not activated on the parent");
		FAIL;
	}

	CHECKPASS(2);
>>ASSERTION Good B 3
When the conditions for activating the grab are satisfied, then the
last-pointer-grab time is set to the time at which the button was pressed
and a
.S ButtonPress
event is generated.
>>STRATEGY
If extensions are available:
  Call xname to place passive grab.
  Enable events on grab window.
  Move pointer into grab window.
  Activate grab with simulated device events.
  Verify that a ButtonPress event is generated.
else
  Report untested.
>>CODE
XEvent	ev;
XButtonPressedEvent	good;
XWindowAttributes	atts;
int 	n;

	if (noext(1))
		return;

	XCALL;

	XSelectInput(display, grab_window, ALLEVENTS);

	(void) warppointer(display, grab_window, ACTPOSX, ACTPOSY);
	XSync(display, True);	/* Discard any events */
	buttonpress(display, button);

	XGetWindowAttributes(display, grab_window, &atts);
	n = getevent(display, &ev);
	if (n)
		CHECK;
	else {
		report("No events received");
		FAIL;
		return;
	}

	defsetevent(good, display, ButtonPress);
	good.window = grab_window;
	good.root = DRW(display);
	good.subwindow = None;
	good.time = ((XButtonPressedEvent*)&ev)->time;
	good.x = ACTPOSX;
	good.y = ACTPOSY;
	good.x_root = good.x + atts.x + atts.border_width;
	good.y_root = good.y + atts.y + atts.border_width;
	good.state = modifiers;
	good.button = button;
	good.same_screen = True;

	if (checkevent((XEvent*)&good, &ev) == 0)
		CHECK;
	else
		FAIL;

	CHECKPASS(2);
>>ASSERTION Good B 3
When the grab subsequently becomes active and later the logical state of the
pointer has all buttons released, then the active grab
is terminated automatically.
>># independent of the state of the logical modifier keys.
>>STRATEGY
If extensions are available:
  Place passive grab with xname.
  Activate grab with simulated device events.
  Simulate pressing some modifier keys.
  Release the button.
  Verify that the grab has been released.
else
  Report untested.
>>CODE
unsigned int 	mods;

	if (noext(1))
		return;

	XCALL;

	activate();

	mods = wantmods(display, 3);
	modpress(display, mods);

	buttonrel(display, button);

	if (pgrabbed()) {
		report("Grab was not released, when button was released and modifier");
		report("  keys were down");
		FAIL;
	} else
		PASS;

>>ASSERTION Good B 3
A call to xname overrides all previous passive grabs by the same
client on the same button/key combinations on the same window.
>>STRATEGY
If extensions are available:
  Place a passive grab with no confine_to window.
  Place a passive grab as before but with a confine_to window.
  Move pointer to grab_window and activate grab.
  Verify that pointer is warped to the confine_to window and thus the second
  grab overrode the first.
else
  Report untested.
>>CODE

	if (noext(1))
		return;

	XCALL;

	/* Try to override first grab */
	confine_to = defwin(display);
	XCALL;

	activate();

	if (getpointerwin(display, DRW(display)) == confine_to)
		PASS;
	else {
		report("A second passive grab did not override the first");
		FAIL;
	}

>>ASSERTION Good B 3
A
.A modifiers
argument of
.S AnyModifier
is equivalent to issuing the grab request for all
possible modifier combinations including no modifiers.
>>STRATEGY
If extensions are available:
  Place passive grab with a modifiers of AnyModifier.
  Press a bunch of modifier keys.
  Press button to activate grab.
  Verify that grab is activated.
  Release button and keys.

  Press button (no modifiers).
  Verify that grab is active.
else
  Perform touch test.
  Report untested.
>>CODE
unsigned int 	mods;

	modifiers = AnyModifier;
	XCALL;

	if (noext(1)) {
		untested("There is no reliable test method, but a touch test was performed");
		return;
	}

	mods = wantmods(display, 4);
	modpress(display, mods);

	/*
	 * modifiers was AnyModifier, several modifier keys are held down.
	 */
	activate();
	if (pgrabbed())
		CHECK;
	else {
		report("Grab not activated for AnyModifier");
		report("  Modifiers used %s", keymaskname((unsigned long)mods));
		FAIL;
	}

	/* Release all buttons and modifiers */
	relalldev();

	if (pgrabbed()) {
		delete("Could not release grab for second part of test");
		return;
	} else
		CHECK;

	buttonpress(display, button);
	if (pgrabbed())
		CHECK;
	else {
		report("Grab with AnyModifier was not activated by a button press with");
		report("  no modifiers");
		FAIL;
	}

	CHECKPASS(3);
>>ASSERTION Good B 3
It is not required that all modifiers specified have currently assigned
KeyCodes.
>>STRATEGY
If extensions are available:
  Get a modifier mask.
  Remove the keycode for the modifier from the map.
  Call xname to set up a passive grab with that modifier.
  Reset the keycode in the modifier map.
  Verify that the grab can be activated with the newly set modifier.
else
  Report untested.
>>CODE
XModifierKeymap	*mmap;
XModifierKeymap	*newmap;
int 	i;

	if (noext(1))
		return;

	modifiers = wantmods(display, 1);
	if (modifiers == 0) {
		untested("No available modifier keys");
		return;
	} else
		CHECK;

	mmap = XGetModifierMapping(display);
	if (mmap == NULL) {
		delete("Could not get modifier map");
		return;
	} else
		CHECK;

	/*
	 * Remove all the modifiers mappings.
	 */
	newmap = XNewModifiermap(mmap->max_keypermod);
	for (i = 0; i < newmap->max_keypermod*8; i++)
		newmap->modifiermap[i] = NoSymbol;

	if (XSetModifierMapping(display, newmap) == MappingSuccess)
		CHECK;
	else {
		delete("Could not remove modifier mapping");
		return;
	}

	/*
	 * Now we have a modifier that has no keycode - set up a passive grab.
	 */
	XCALL;

	/*
	 * Reset the modifier map, and try to activate the grab.
	 */
	if (XSetModifierMapping(display, mmap) == MappingSuccess)
		CHECK;
	else {
		delete("Could not reset modifier mapping");
		return;
	}

	activate();

	if (pgrabbed())
		CHECK;
	else {
		report("Passive grab not set when the modifier did not have a current keycode");
		FAIL;
	}
	CHECKPASS(5);
>>ASSERTION Good B 3
A
.A button
argument of
.S AnyButton
is equivalent to issuing the grab request for all possible buttons.
>>STRATEGY
If extensions are available:
  Set up a passive grab using AnyButton.
  Move pointer to grab window.
  Simulate a button press.
  Verify that pointer is grabbed.
  Repeat for other buttons.
else
  Touch test using AnyButton.
  Report untested.
>>CODE
extern	struct	valname	S_button[];
extern	int 	NS_button;
int 	i;

	button = AnyButton;
	XCALL;

	if (noext(1)) {
		untested("There is no reliable test method, but a touch test was performed");
		return;
	} else
		CHECK;

	(void) warppointer(display, grab_window, 10, 10);

	for (i = 0; i < nbuttons(); i++) {
		buttonpress(display, (unsigned int)S_button[i].val);
		if (pgrabbed())
			CHECK;
		else {
			report("Passive grab of AnyButton, not grabbed for %s",
				S_button[i].name);
			FAIL;
		}

		/*
		 * Release this grab and try next button.
		 */
		relbuttons();
		if (pgrabbed()) {
			delete("Could not release grab for next part of test");
			return;
		} else
			CHECK;
	}

	CHECKPASS(1+2*nbuttons());
>>ASSERTION Good B 3
It is not required that the specified button currently be assigned
to a physical button.
>>STRATEGY
If extensions are available:
  Remove a button from the button map.
  Set a passive grab with that button.
  Replace the button in the map.
  Simulate pressing button in window.
  Verify that grab is activated.
else
  Report untested.
>>CODE
unsigned char	pmap[100];
int 	savebutton;
int 	n;

	if (noext(1))
		return;

	n = XGetPointerMapping(display, pmap, (int)sizeof(pmap));
	savebutton = pmap[0];
	pmap[0] = 0;	/* Disable button1 */
	if (XSetPointerMapping(display, pmap, n) == MappingSuccess)
		CHECK;
	else {
		delete("Could not change pointer mapping");
		return;
	}

	button = Button1;
	XCALL;

	pmap[0] = savebutton;
	if (XSetPointerMapping(display, pmap, n) == MappingSuccess)
		CHECK;
	else {
		delete("Could not restore pointer mapping");
		return;
	}

	activate();
	if (pgrabbed())
		CHECK;
	else {
		report("Could not set grab when button was not assigned a physical button");
		FAIL;
	}

	CHECKPASS(3);
>>ASSERTION Good B 3
When
.A owner_events
is
.S False
and the grab is now active,
then all generated pointer events that are selected by the
.A event_mask
are reported with respect to the
.A grab_window .
>>STRATEGY
If extensions are available:
  Set owner_events to False.
  Set event_mask to select for PointerMotion.
  Set passive grab.
  Activate grab by simulated button press.
  Create a window.
  Select events on the window.
  Warp pointer into window.
  Verify MotionNotify reported to grab_window.
else
  Report untested.
>>CODE
Window	win;
XEvent	ev;
XMotionEvent	*mp;

	if (noext(1))
		return;

	owner_events = False;
	event_mask = PointerMotionMask;

	XCALL;

	activate();

	/* Flush any events so far */
	XSync(display, True);

	win = defwin(display);
	XSelectInput(display, win, PointerMotionMask);
	(void) warppointer(display, win, 1, 2);
	(void) warppointer(display, win, 2, 3);

	if (getevent(display, &ev) == 0) {
		report("No events received");
		FAIL;
	} else
		CHECK;

	if (ev.type != MotionNotify) {
		report("Got event %s, expecting MotionNotify", eventname(ev.type));
		FAIL;
	} else
		CHECK;

	mp = (XMotionEvent*)&ev;
	if (mp->window == grab_window)
		CHECK;
	else if (mp->window == win) {
		report("Event was reported on the window the pointer was in");
		FAIL;
	} else {
		report("Event was not reported on the grab_window");
		FAIL;
	}

	CHECKPASS(3);
>>ASSERTION Good B 3
When
.A owner_events
is
.S True ,
the grab is now active
and a pointer event is generated that would normally be reported to the client,
then it is reported on the window that it would normally be reported on.
>>STRATEGY
If extensions are available:
  Set owner_events to True.
  Set passive grab.
  Activate grab by simulated button press.
  Create a window.
  Select events on the window.
  Warp pointer into window.
  Verify events are reported on window.
else
  Report untested.
>>CODE
Window	win;
XEvent	ev;
XMotionEvent	*mp;

	if (noext(1))
		return;

	owner_events = True;
	event_mask = PointerMotionMask;

	XCALL;

	activate();

	/* Flush any events so far */
	XSync(display, True);

	win = defwin(display);
	XSelectInput(display, win, PointerMotionMask);
	(void) warppointer(display, win, 1, 2);
	(void) warppointer(display, win, 1, 6);

	if (getevent(display, &ev) == 0) {
		report("No events received");
		FAIL;
	} else
		CHECK;

	if (ev.type != MotionNotify) {
		report("Got event %s, expecting MotionNotify", eventname(ev.type));
		FAIL;
	} else
		CHECK;

	mp = (XMotionEvent*)&ev;
	if (mp->window == win)
		CHECK;
	else if (mp->window == grab_window) {
		report("Event was reported on the grab_window");
		FAIL;
	} else {
		report("Event was not reported on the normal window");
		FAIL;
	}

	CHECKPASS(3);
>>ASSERTION Good B 3
When
.A owner_events
is
.S True ,
the grab is now active,
a pointer event is generated that would not normally be reported to the client
and it is selected by
.A event_mask ,
then it is reported on the
.A grab_window .
>>STRATEGY
If extensions are available:
  Set owner_events to True.
  Set passive grab.
  Activate grab by simulated button press.
  Create a window.
  Warp pointer into window and simulate a button press/release.
  Verify events are reported to grab_window.
else
  Report untested.
>>CODE
Window	win;
XEvent	ev;
XMotionEvent	*mp;

	if (noext(1))
		return;

	owner_events = True;
	event_mask = PointerMotionMask;

	XCALL;

	activate();

	/* Flush any events so far */
	XSync(display, True);

	win = defwin(display);
	(void) warppointer(display, win, 1, 2);
	(void) warppointer(display, win, 4, 12);

	if (getevent(display, &ev) == 0) {
		report("No events received");
		FAIL;
	} else
		CHECK;

	if (!fail && ev.type != MotionNotify) {
		report("Got event %s, expecting MotionNotify", eventname(ev.type));
		FAIL;
	} else
		CHECK;

	mp = (XMotionEvent*)&ev;
	if (!fail) {
		if (mp->window == grab_window)
			CHECK;
		else if (mp->window == win) {
			report("Event was reported on the window the pointer was in");
			FAIL;
		} else {
			report("Event was not reported on the grab_window");
			FAIL;
		}
	}

	CHECKPASS(3);
>>ASSERTION Good B 3
When
.A pointer_mode
is
.S GrabModeAsync
and the grab is now active,
then pointer event processing continues normally.
>>STRATEGY
If extensions are available:
  Set pointer_mode to GrabModeAsync.
  Set passive grab.
  Press button in grab_window to activate grab.
  Verify that pointer is not frozen.
else
  Report untested.
>>CODE

	if (noext(1))
		return;

	pointer_mode = GrabModeAsync;
	XCALL;

	activate();

	if (!pfrozen())
		CHECK;
	else {
		report("Pointer was frozen with GrabModeAsync");
		FAIL;
	}

	CHECKPASS(1);
>>#REMOVED >>ASSERTION Good B 3
>>#COMMENT:
>># This assertion was removed April 1992 because MIT reviewed the assertion 
>># again, and decided it did not make sense (bug report 0236). We agree.
>># Dave Cater, 27/4/92
>>#REMOVED When
>>#REMOVED .A pointer_mode
>>#REMOVED is
>>#REMOVED .S GrabModeAsync ,
>>#REMOVED the grab is now active
>>#REMOVED and the pointer is currently frozen by this client, then
>>#REMOVED the processing of events for the pointer is resumed.
>>#REMOVED >>STRATEGY
>>#REMOVED If extensions are available:
>>#REMOVED   Freeze pointer by grabbing keyboard.
>>#REMOVED   Check pointer is frozen.
>>#REMOVED   Set pointer_mode to GrabModeAsync.
>>#REMOVED   Set and activate grab.
>>#REMOVED   Verify that pointer is not frozen.
>>#REMOVED else
>>#REMOVED   Report untested.
>>#REMOVED >>CODE
>>#REMOVED 
>>#REMOVED 	if (noext(1))
>>#REMOVED 		return;
>>#REMOVED 
>>#REMOVED 	/* Freeze pointer by grabbing keyboard */
>>#REMOVED 	XGrabKeyboard(display, grab_window, False, GrabModeSync, GrabModeAsync, CurrentTime);
>>#REMOVED 	if (pfrozen())
>>#REMOVED 		CHECK;
>>#REMOVED 	else {
>>#REMOVED 		delete("Could not freeze pointer prior to test");
>>#REMOVED 		return;
>>#REMOVED 	}
>>#REMOVED 
>>#REMOVED 	pointer_mode = GrabModeAsync;
>>#REMOVED 	XCALL;
>>#REMOVED 
>>#REMOVED 	activate();
>>#REMOVED 
>>#REMOVED 	if (pfrozen()) {
>>#REMOVED 		report("Pointer remained frozen after a grab with GrabModeAsync");
>>#REMOVED 		FAIL;
>>#REMOVED 	} else
>>#REMOVED 		CHECK;
>>#REMOVED 
>>#REMOVED 	CHECKPASS(2);
>>ASSERTION Good B 3
When
.A pointer_mode
is
.S GrabModeSync
and the grab is now active,
then the state of the pointer, as seen by client applications,
appears to freeze, and no further pointer events are generated
until the grabbing client calls
.F XAllowEvents
or until the grab is released.
>>STRATEGY
If extensions are available:
  Set pointer_mode to GrabModeSync.
  Set grab with xname and activate.
  Verify that pointer is frozen.
  Allow events with XAllowEvents().
  Verify that pointer is not frozen.
  Release the grab and restore the device state.
  Set grab with xname and activate.
  Verify that pointer is frozen.
  Release grab.
  Verify that pointer is not frozen.
else
  Report untested.
>>CODE

	if (noext(1))
		return;

	pointer_mode = GrabModeSync;
	XCALL;

	activate();

	if (pfrozen())
		CHECK;
	else {
		report("Pointer not frozen when pointer mode was GrabModeSync after first activate");
		FAIL;
	}

	XAllowEvents(display, AsyncPointer, CurrentTime);
	XSync(display,False);

	if (pfrozen()) {
		report("Pointer was still frozen after XAllowEvents()");
		FAIL;
	} else
		CHECK;

	relalldev();
	XUngrabPointer(display, CurrentTime);
	XUngrabButton(display, AnyButton, AnyModifier, grab_window);

	pointer_mode = GrabModeSync;
	XCALL;

	activate();

	if (pfrozen())
		CHECK;
	else {
		report("Pointer not frozen when pointer mode was GrabModeSync after second activate");
		FAIL;
	}

	XUngrabPointer(display, CurrentTime);
	XSync(display,False);

	if (pfrozen()) {
		report("Pointer was still frozen after grab released");
		FAIL;
	} else
		CHECK;

	XUngrabButton(display, AnyButton, AnyModifier, grab_window);

	CHECKPASS(4);
>>ASSERTION Good B 3
When the pointer is frozen, then the
actual pointer changes are not lost and are processed after the grab
is released or the client calls
.F XAllowEvents .
>>STRATEGY
If extensions are available:
  Set pointer_mode to GrabModeSync.
  Set grab with xname and activate.
  Check that no pointer events are generated.
  Release grab.
  Verify that events are now received.
  Release the grab and restore the device state.
  Set grab with xname and activate.
  Check that no pointer events are generated.
  Allow events with XAllowEvents().
  Verify that events are now received.
else
  Report untested.
>>CODE
XEvent	ev;

	if (noext(1))
		return;

	XSelectInput(display, grab_window, event_mask);
	pointer_mode = GrabModeSync;
	XCALL;

	activate();

	XSync(display, True);	/* Flush events */
	(void) warppointer(display, grab_window, ACTPOSX+3, ACTPOSY+4);
	if (XCheckMaskEvent(display, (long)event_mask, &ev)) {
		delete("Pointer event was received while pointer was frozen");
		trace("ev.type = %s", eventname(ev.type));
		return;
	} else
		CHECK;

	XUngrabPointer(display, CurrentTime);
	XSync(display, False);

	if (XCheckMaskEvent(display, (long)event_mask, &ev)) {
		if( ev.type == MotionNotify) {
			CHECK;
		} else {
			report("Unexpected event (%s)was received",
				eventname(ev.type));
			FAIL;
		}
	} else {
		if(pgrabbed())
			report("Grab was not released.");
		else
			report("Pointer event was not saved while pointer was frozen");
		FAIL;
	}

	relalldev();
	XUngrabButton(display, AnyButton, AnyModifier, grab_window);
	
	pointer_mode = GrabModeSync;
	XCALL;

	activate();

	XSync(display, True);
	(void) warppointer(display, grab_window, ACTPOSX+3, ACTPOSY+4);
        if (XCheckMaskEvent(display, (long)event_mask, &ev)) {
                delete("Pointer event was received while pointer was frozen");
		trace("ev.type = %s", eventname(ev.type));
                return;
        } else
                CHECK;

	XAllowEvents(display, AsyncPointer, CurrentTime);
	XSync(display, False);

	if (XCheckMaskEvent(display, (long)event_mask, &ev)) {
		if( ev.type == MotionNotify) {
			CHECK;
		} else {
			report("Unexpected event (%s) was received",
				eventname(ev.type));
			FAIL;
		}
	} else {
		report("Pointer event was not saved while pointer was frozen");
		report("  or grab was not released by a call to XAllowEvents()");
		FAIL;
	}

	XUngrabButton(display, AnyButton, AnyModifier, grab_window);

	CHECKPASS(4);
>>ASSERTION Good B 3
When
.A keyboard_mode
is
.S GrabModeAsync ,
then keyboard event processing is unaffected by activation of the grab.
>>STRATEGY
If extensions are available:
  Set keyboard_mode to GrabModeAsync.
  Set grab with xname and activate.
  Verify that keyboard is not frozen.
else
  Report untested.
>>CODE

	if (noext(1))
		return;

	keyboard_mode = GrabModeAsync;
	XCALL;
	activate();

	if (kfrozen()) {
		report("Keyboard was frozen when keyboard_mode was GrabModeAsync");
		FAIL;
	} else
		CHECK;

	CHECKPASS(1);
>>ASSERTION Good B 3
When
.A keyboard_mode
is
.S GrabModeSync
and the grab is now active,
then the state of the keyboard, as seen by
client applications,
appears to freeze, and no further keyboard events are generated
until the grabbing client calls
.S XAllowEvents
or until the grab is released.
>>STRATEGY
If extensions are available:
  Set keyboard_mode to GrabModeSync.
  Set grab with xname and activate.
  Verify that keyboard is frozen.
  Allow events with XAllowEvents().
  Verify that keyboard is not frozen.
  Release the grab and restore the device state.
  Set grab with xname and activate.
  Verify that keyboard is frozen.
  Release grab.
  Verify that keyboard is not frozen.
else
  Report untested.
>>CODE

	if (noext(1))
		return;

	keyboard_mode = GrabModeSync;
	XCALL;
	activate();

	if (kfrozen())
		CHECK;
	else {
		report("Keyboard was not frozen when keyboard_mode was GrabModeSync");
		FAIL;
	}

	XAllowEvents(display, AsyncKeyboard, CurrentTime);
	XSync(display, False);

	if(kfrozen())  {
		report("Keyboard still frozen after XAllowEvents()");
		FAIL;
	} else
		CHECK;

	relbuttons();
	XUngrabPointer(display, CurrentTime);
	XUngrabButton(display, AnyButton, AnyModifier, grab_window);

	keyboard_mode = GrabModeSync;
	XCALL;
	activate();

	if (kfrozen())
		CHECK;
	else {
		report("Keyboard was not frozen when keyboard_mode was GrabModeSync");
		FAIL;
	}
	
	buttonrel(display, button);

	if (kfrozen()) {
		report("Keyboard not thawed after a button grab released");
		FAIL;
	} else
		CHECK;

	XUngrabButton(display, AnyButton, AnyModifier, grab_window);

	CHECKPASS(4);
>>ASSERTION Good B 3
When the pointer is frozen, then the
>># Shouldn't this be 'When the keyboard is frozen,...'
actual keyboard changes are not lost and are processed after the grab
is released or the client calls
.F XAllowEvents .
>>STRATEGY
If extensions are available:
  Enable key press events.
  Set keyboard_mode to GrabModeSync.
  Set grab with xname and activate.
  Press key.
  Check that no pointer events are generated.
  Allow events with XAllowEvents().
  Verify that events are now received.
  Release the grab and restore the device state.
  Set grab with xname and activate.
  Press key.
  Check that no pointer events are generated.
  Release grab.
  Verify that events are now received.
else
  Report untested.
>>CODE
int 	minkc, maxkc;
XEvent	ev;

	if (noext(1))
		return;

	XSelectInput(display, grab_window, KeyPressMask);
	keyboard_mode = GrabModeSync;
	XCALL;

	activate();
	XSync(display, True);	/* Flush events */

	XDisplayKeycodes(display, &minkc, &maxkc);
	keypress(display, minkc);
	keyrel(display, minkc);

	if (XCheckMaskEvent(display, (long)KeyPressMask, &ev)) {
		delete("KeyPress event was received while pointer was frozen");
		return;
	} else
		CHECK;

	XUngrabPointer(display, CurrentTime);
	XSync(display,False);

	if (XCheckMaskEvent(display, (long)KeyPressMask, &ev))
		CHECK;
	else {
		if(pgrabbed()) {
			report("Grab was not released");
		} else {
			report("KeyPress event was not saved while keyboard was frozen");
		}
		FAIL;
	}

	relalldev();
	XUngrabButton(display, AnyButton, AnyModifier, grab_window);

	pointer_mode = GrabModeSync;
	XCALL;

	activate();

	XSync(display, True);
	keypress(display, minkc);
	keyrel(display, minkc);

	if (XCheckMaskEvent(display, (long)KeyPressMask, &ev)) {
		delete("KeyPress event was received while pointer was frozen");
		return;
	} else
		CHECK;

	XAllowEvents(display, AsyncKeyboard, CurrentTime);
	XSync(display, False);

	if (XCheckMaskEvent(display, (long)KeyPressMask, &ev))
		CHECK;
	else {
		if(pgrabbed()) {
			report("Grab was not released");
		} else {
			report("KeyPress event was not saved while keyboard was frozen");
		}
		FAIL;
	}

	CHECKPASS(4);
>>ASSERTION Good B 3
When
.A cursor
is a valid cursor
and the grab is now active,
then it is displayed regardless of which window the pointer is in.
>>STRATEGY
If extensions available:
  Make a tree of windows rooted at grab_window, all with default cursor.
  Make a non-overlapping sibling of grab_window, the cursor window.
  Set that window's cursor to be a good, non default, cursor.
  Call xname with cursor = the good cursor.
  Activate grab by simulating button press.
  Warp pointer to all windows in the tree, root and cursor window, and
    validate that current cursor is equal to that of cursor window
    using extension.
else:
  Perform touch test.
>>EXTERN
static char	*WindowTree[]= {
	".",
		"child1 . (10,10) 30x30",
		"child2 . (50,50) 30x30",
			"grandchild child2 (2,2) 20x20",
};

static int NWindowTree = NELEM(WindowTree);

>>CODE
Cursor	goodcur;
Window	parent,curwin;
Window	windows[7]; /* root + 5 above + None stopper */
Window	root;
Window	*wp;
struct buildtree *tree;
char	*wname;

	if (!noext(1)) {

		goodcur = makecur(display);
		wp = windows;
		*wp++ = root = DRW(display);
		*wp++ = parent = grab_window;
		tree = buildtree(display, parent, WindowTree, NWindowTree);
		*wp++ = btntow(tree, "child1");
		*wp++ = btntow(tree, "child2");
		*wp++ = btntow(tree, "grandchild");
		*wp++ = curwin = defwin(display);
		*wp = None;
		XDefineCursor(display, curwin, goodcur);
		(void)warppointer(display, curwin, 2, 2);
		if (spriteiswin(display, parent)) {
			delete("Require XT_FONTCURSOR_GOOD to be other than default.");
			return;
		} else
			CHECK;
	
		cursor = goodcur;
		XCALL;
		activate();
	
		for(wp=windows; *wp != None; wp++) { /* around 6 times */
			(void)warppointer(display, *wp, 0,0);
			/* use 0,0 as window making stuff keeps away from there on
			 * root. All of our tree windows are not at 0,0 either
			 */
			if (spriteiswin(display, curwin)) /* true at least once! */
				CHECK;
			else {
				report("Window %s did not have specified cursor as displayed cursor.",
					(wname=btwton(tree,*wp)) ? wname :
						( *wp == root ? "ROOT" : "<Unknown>" ));
				FAIL;
			}
		}
	
		CHECKPASS(1+6);
	
	} else {

		cursor = makecur(display);

		XCALL;
		CHECK;

		CHECKUNTESTED(1);
	}

>>ASSERTION Good B 3
When
.A cursor
is
.S None
and the grab is now active,
then the normal cursor is displayed for the
.A grab_window
and its subwindows and the
.A grab_window
cursor is displayed for all other windows.
>>STRATEGY
If extensions available:
  Make a tree of windows rooted at grab_window (parent), all with default cursor.
  Set grab_window to be one of its own children (child2), which has its own
    child (grandchild).
  Set grab_window's cursor to be a good, non default, cursor.
  Set grab_window's child's (grandchild) cursor to be yet another good,
    non default, cursor.
  Call xname with cursor = None.
  Activate grab by simulating button press.
  Warp pointer to all windows in the tree, plus root, and
    validate that current cursor is equal to that of grab_window & grandchild,
    respectively, when in the corresponding windows, but equal to that of
    grab_window otherwise, using extension.
else
  Report untested.
>>CODE
Cursor	goodcur1, goodcur2;
Window	parent,gchild;
Window	windows[6]; /* root + 4 above + None stopper */
Window	root;
Window	*wp;
struct buildtree *tree;
char	*wname;

	if (noext(0))
		return;

	goodcur1 = makecur(display);
	goodcur2 = makecur2(display);
	if (isdeleted()) {
		report("Could not make cursors. Check that XT_FONTCURSOR_GOOD");
		report("and XT_FONTCURSOR_GOOD+2 both are good glyphs in cursor font.");
		return;
	}
	wp = windows;
	*wp++ = root = DRW(display);
	*wp++ = parent = grab_window;
	tree = buildtree(display, parent, WindowTree, NWindowTree);
	*wp++ = btntow(tree, "child1");
	*wp++ = grab_window = btntow(tree, "child2");
	*wp++ = gchild = btntow(tree, "grandchild");
	*wp = None;
	XDefineCursor(display, grab_window, goodcur1);
	XDefineCursor(display, gchild, goodcur2);

	(void)warppointer(display, gchild, 2, 2);
	if (spriteiswin(display, grab_window)) {
		delete("Require glyphs XT_FONTCURSOR_GOOD and XT_FONTCURSOR_GOOD+2 to differ in cursor font.");
		return;
	} else
		CHECK;

	cursor = None;
	XCALL;
	activate();

	for(wp=windows; *wp != None; wp++) { /* around 5 times */
		(void)warppointer(display, *wp, 0,0);
		/* use 0,0 as window making stuff keeps away from there on
		 * root. All of our tree windows are not at 0,0 either
		 */
		if (!(wname = btwton(tree,*wp)))
			wname = (*wp == root) ? "ROOT" : "<Unknown>";
		if (*wp == grab_window || *wp == gchild) {
			if (spriteiswin(display, *wp))
				CHECK;
			else {
				report("Window %s did not have cursor as its normal cursor", wname);
				report("\twhen in grab_window or descendent.");
				FAIL;
			}
		} else {
			if (spriteiswin(display, grab_window))
				CHECK;
			else {
				report("Window %s did not have cursor as grab_window's cursor", wname);
				report("\twhen not in grab_window or descendent.");
				FAIL;
			}
		}
	}

	CHECKPASS(1+5);

>>ASSERTION Good B 3
When
.A confine_to
is specified
and the grab is now active, then the pointer is confined to that window.
>>STRATEGY
If extensions are available:
  Create a confine_to window.
  Set up grab with xname.
  Active grab.
  Verify that pointer is within confine_to window.
else
  Report untested.
>>CODE

	if (noext(1))
		return;

	confine_to = defwin(display);
	XCALL;

	activate();

	if (getpointerwin(display, DRW(display)) == confine_to)
		CHECK;
	else {
		report("Pointer was not within the confine_to window");
		FAIL;
	}

	CHECKPASS(1);
>>ASSERTION Good B 3
When the pointer is not initially contained in the
.A confine_to
window,
then it is warped automatically to the closest edge
just before the grab activates and enter and leave events
are generated.
>>STRATEGY
If extensions are available:
  Create confine_to window.
  Enable events on the grab_window and the confine_to windows.
  Set owner_events to True.
  Set and activate button grab.
  Verify that a leave event is generated on the window the pointer was in.
  Verify that an enter event is generated on the confine_to window.
else
  Report untested.
>>CODE
XEvent	ev;
XCrossingEvent	*cp;
XEnterWindowEvent	entergood;
XLeaveWindowEvent	leavegood;

	if (noext(1))
		return;

	confine_to = defwin(display);

	XSelectInput(display, confine_to, EnterWindowMask|LeaveWindowMask);
	XSelectInput(display, grab_window, EnterWindowMask|LeaveWindowMask);
	owner_events = True;
	XCALL;

	(void) warppointer(display, grab_window, ACTPOSX, ACTPOSY);
	XSync(display, True);	/* Discard events so far */
	buttonpress(display, button);

	defsetevent(entergood, display, EnterNotify);
	entergood.window = confine_to;
	entergood.root = DRW(display);
	entergood.subwindow = None;
	entergood.time = 0;
	entergood.mode = NotifyNormal;
	entergood.detail = NotifyNonlinear;
	entergood.same_screen = True;
	entergood.focus = False;
	entergood.state = Button1Mask;

	defsetevent(leavegood, display, LeaveNotify);
	leavegood.window = grab_window;
	leavegood.root = DRW(display);
	leavegood.subwindow = None;
	leavegood.time = 0;
	leavegood.mode = NotifyNormal;
	leavegood.detail = NotifyNonlinear;
	leavegood.same_screen = True;
	leavegood.focus = False;
	leavegood.state = Button1Mask;

	/* Event of the pointer leaving the grab_window */
	if (getevent(display, &ev) == 0 || ev.type != LeaveNotify) {
		report("No leave notify event for grab_window received");
		FAIL;
	} else
		CHECK;

	cp = (XCrossingEvent*)&ev;
	/* Set the fields that we can't conveniently check */
	leavegood.time = cp->time;
	leavegood.x = cp->x;
	leavegood.y = cp->y;
	leavegood.x_root = cp->x_root;
	leavegood.y_root = cp->y_root;
	leavegood.focus = cp->focus;
	if (checkevent((XEvent*)&leavegood, &ev))
		FAIL;
	else
		CHECK;

	/* Now the Enter into the confine_to window */
	if (getevent(display, &ev) == 0 || ev.type != EnterNotify) {
		report("No enter notify event for confine_to received");
		FAIL;
	} else
		CHECK;

	cp = (XCrossingEvent*)&ev;
	/* Set the fields that we can't conveniently check */
	entergood.time = cp->time;
	entergood.x = cp->x;
	entergood.y = cp->y;
	entergood.x_root = cp->x_root;
	entergood.y_root = cp->y_root;
	entergood.focus = cp->focus;
	if (checkevent((XEvent*)&entergood, &ev))
		FAIL;
	else
		CHECK;

	CHECKPASS(4);
>>ASSERTION Good B 3
When the
.A confine_to
window is subsequently reconfigured
and the grab is now active,
then the pointer is warped automatically to keep it within the window.
>>STRATEGY
If extensions are available:
  Create confine_to window.
  Set up and activate button grab,
  Move confine_to so that it does not overlap with it's previous position.
  Verify that the pointer has been warped to the new position.
else
  Report untested.
>>CODE
struct	area	area;

	if (noext(1))
		return;

	(void) warppointer(display, DRW(display), 0, 0);

	/*
	 * This time make confine_to a child of grab_window, also make it small
	 * so that moving it will force the pointer to move.
	 */
	setarea(&area, 1, 1, 2, 2);
	confine_to = crechild(display, grab_window, &area);

	XCALL;
	activate();

	XMoveWindow(display, confine_to, 20, 20);
	if (isdeleted())
		return;

	if (getpointerwin(display, grab_window) == confine_to)
		CHECK;
	else {
		report("Pointer was not kept within confine_to when confine_to was moved");
		FAIL;
	}

	CHECKPASS(1);
>>ASSERTION Good D 3
If multiple screens are supported:
It is valid for the
.A confine_to
window to be on a different screen to the
.A grab_window .
>>STRATEGY
If only one screen
  UNSUPPORTED.
If extensions are available:
  Create grab_window on default screen.
  Create confine_to window on alternate screen.
  Set up and activate grab.
  Verify that pointer is warped to other screen.
else
  Touch test with grab_window and confine_to on different screens.
>>CODE
Window  root;
Window  wtmp;
int     itmp;
unsigned uitmp;
Bool    s;

	if (config.alt_screen < 0) {
		unsupported("Only one screen supported");
		return;
	}

	confine_to = defdraw(display, VI_ALT_WIN);

	XCALL;

	if (noext(1)) {
		untested("There is no reliable test method, but a touch test was performed");
		return;
	}

	activate();

	/*
	 * Query Pointer returns false if the pointer is not the same screen as
	 * the given window.  Check with both windows, mainly as a sanity check
	 * against them being on the same screen.
	 */
	s = XQueryPointer(display, confine_to, &root, &wtmp, &itmp, &itmp,
		&itmp, &itmp , &uitmp);
	if (s)
		CHECK;
	else {
		report("Pointer was not warped to confine_to window on other screen");
		FAIL;
	}

	s = XQueryPointer(display, grab_window, &root, &wtmp, &itmp, &itmp,
		&itmp, &itmp , &uitmp);
	if (s) {
		report("Pointer was not warped to confine_to window on other screen");
		FAIL;
	} else
		CHECK;

	CHECKPASS(2);

>>ASSERTION Good B 3
When the button/key combination is pressed and the
grab becomes active, then the last-pointer-grab
time is set to the time the button was pressed.
>>STRATEGY
If extensions available:
  Call xname to set up passive grab.
  Press button.
  Get the ButtonPress event.
  Save the time field in the event.
  Release grab by releasing button.
  Attempt to grab pointer using the saved time - 1.
  Verify that pointer is not grabbed.
  Attempt to grab pointer using the saved time.
  Verify that pointer is grabbed.
>>CODE
XEvent	ev;
Time	savtime;

	if (noext(1))
		return;

	XCALL;
	activate();

	if (getevent(display, &ev) == 0) {
		delete("Could not get button event");
		return;
	} else
		CHECK;

	if (ev.type != ButtonPress) {
		delete("Did not get ButtonPressEvent");
		return;
	} else
		CHECK;

	savtime = ((XButtonPressedEvent*)&ev)->time;

	relbuttons();
	if (pgrabbed()) {
		delete("Could not release grab");
		return;
	} else
		CHECK;

	XGrabPointer(display, grab_window, False, 0, GrabModeAsync, GrabModeAsync,
		None, None, savtime-1);

	if (pgrabbed()) {
		report("Pointer was grabbed when time was earlier than the");
		report("  pointer-last-grab time");
		FAIL;
	} else
		CHECK;

	XGrabPointer(display, grab_window, False, 0, GrabModeAsync, GrabModeAsync,
		None, None, savtime);
	if (pgrabbed())
		CHECK;
	else {
		report("Pointer was not grabbed when time was equal to the");
		report("  pointer-last-grab time");
		FAIL;
	}

	CHECKPASS(5);

>>ASSERTION Good B 3
A call to xname has no effect on an active grab.
>>STRATEGY
If extensions available:
  Create window.
  Grab pointer on this window.
  Set up passive grab on another window.
  Verify that pointer is still grabbed on first window.
>>CODE
Window	win;
XMotionEvent	*mep;
XEvent	ev;

	if (noext(1))
		return;

	win = defwin(display);
	XGrabPointer(display, win, False, PointerMotionMask,
		GrabModeAsync, GrabModeAsync, None, None, CurrentTime);

	/* This sets the grab on grab_window */
	XCALL;

	(void) warppointer(display, grab_window, 1, 1);
	(void) warppointer(display, grab_window, 3, 3);

	if (getevent(display, &ev) < 1) {
		delete("Could not get motion event");
		return;
	} else
		CHECK;

	mep = (XMotionEvent*)&ev;
	if (mep->window == win)
		CHECK;
	else if (!fail) {
		report("A call to %s had an effect on an active grab", TestName);
		report("  A new grab window was set");
		FAIL;
	}

	CHECKPASS(2);
>>ASSERTION Good B 3
When the
.A grab_window
or
.A confine_to
window becomes not viewable during an active pointer grab,
then the grab is released.
>>STRATEGY
If extensions are available:
  Create grab and confine_to windows.
  Set up passive grab with xname.

  Activate grab with a button press.
  Unmap grab_window.
  Verify that pointer is not grabbed.

  Re-map grab_window.
  Activate grab with a button press.
  Unmap confine_to window.
  Verify that pointer is not grabbed.
else
  Report untested.
>>CODE
Display	*client2;
Window	win;
XEvent	ev;

	if (noext(1))
		return;

	(void) warppointer(display, DRW(display), 0, 0);

	client2 = opendisplay();
	win = defwin(display);
	XSelectInput(client2, win, PointerMotionMask|EnterWindowMask);
	XSync(client2, True);

	confine_to = defwin(display);
	XCALL;

	activate();

	XUnmapWindow(display, grab_window);

	/*
	 * Warp into win and force all events to be received.
	 * If the grab has been released then this will generate
	 * an event for client2.
	 */
	(void) warppointer(display, win, 0, 0);
	XSync(client2, False);

	if (XCheckWindowEvent(client2, win, (long)PointerMotionMask|EnterWindowMask, &ev))
		CHECK;
	else {
		report("Grab was not released when grab_window was unmapped");
		FAIL;
	}

	/* Clear any extra events */
	XSync(client2, True);

	/* Now repeat for confine_to window. */
	XMapWindow(display, grab_window);

	activate();

	XUnmapWindow(display, confine_to);

	/* Warp to win and check for events on client2 */
	(void) warppointer(display, win, 0, 0);
	XSync(client2, False);

	if (XCheckWindowEvent(client2, win, (long)PointerMotionMask|EnterWindowMask, &ev))
		CHECK;
	else {
		report("Grab was not released when confine_to window was unmapped");
		FAIL;
	}

	CHECKPASS(2);
>>ASSERTION Good B 3
When window reconfiguration causes the
.A confine_to
window to lie completely
outside the boundaries of the root window during an active pointer grab, then
the grab is released.
>>STRATEGY
If extensions are available:
  Create grab and confine_to windows.
  Set up and activate grab.
  Move confine_to window off the root window.
  Verify that grab is released.
else
  Report untested.
>>CODE
Display	*client2;
Window	win;
XEvent	ev;

	if (noext(1))
		return;

	(void) warppointer(display, DRW(display), 0, 0);

	client2 = opendisplay();

	confine_to = defwin(display);
	win = defwin(display);

	XSelectInput(client2, win, PointerMotionMask|EnterWindowMask);
	XSync(client2, True);

	XCALL;
	activate();

	XMoveWindow(display, confine_to, -9000, -9000);
	(void) warppointer(display, win, 0, 0);
	XSync(client2, False);

	if (isdeleted())
		return;

	if (XCheckWindowEvent(client2, win, (long)PointerMotionMask|EnterWindowMask, &ev))
		CHECK;
	else {
		report("Grab was not released when the confine_to window was placed beyond root window");
		FAIL;
	}

	CHECKPASS(1);
>>ASSERTION Bad A
.ER Access grab
>>STRATEGY
Grab a button.
Create new client, client1.
Attempt to grab same button with client1.
Verify that a BadAccess error occurs.
>>CODE BadAccess
Display	*client1;

	XGrabButton(display, button, modifiers, grab_window, owner_events, event_mask, pointer_mode, keyboard_mode, confine_to, cursor);

	if ((client1 = opendisplay()) == 0) {
		delete("Could not open display");
		return;
	}

	display = client1;
	XCALL;

	if (geterr() == BadAccess)
		PASS;
	else
		FAIL;
>>ASSERTION Bad A
.ER BadCursor
>>ASSERTION Bad A
.ER BadValue event_mask mask ButtonPressMask ButtonReleaseMask EnterWindowMask LeaveWindowMask PointerMotionMask PointerMotionHintMask Button1MotionMask Button2MotionMask Button3MotionMask Button4MotionMask Button5MotionMask ButtonMotionMask KeymapStateMask
>>ASSERTION Bad A
.ER BadValue modifiers mask ShiftMask LockMask ControlMask Mod1Mask Mod2Mask Mod3Mask Mod4Mask Mod5Mask AnyModifier
>>ASSERTION Bad A
.ER BadValue pointer_mode GrabModeSync GrabModeAsync
>>ASSERTION Bad A
.ER BadValue keyboard_mode GrabModeSync GrabModeAsync
>>ASSERTION Bad A
.ER BadValue owner_events True False
>>ASSERTION Bad A
>># It would be much preferable to use the automatic code.
>># .ER BadWindow
When the
.A grab_window
argument does not name a valid Window,
then a
.S BadWindow
error occurs.
>>STRATEGY
Set grab_window to invalid window.
Call xname.
Verify that a BadWindow error occurs.
>>CODE BadWindow

	seterrdef();

	grab_window = badwin(display);

	XCALL;

	if (geterr() == BadWindow)
		PASS;
	else
		FAIL;

>>ASSERTION Bad A
When the
.A confine_to
argument does not name a valid Window or 
.S None ,
then a
.S BadWindow
error occurs.
>>STRATEGY
Set confine_to to invalid window.
Call xname.
Verify that a BadWindow error occurs.
>>CODE BadWindow

	seterrdef();

	confine_to = badwin(display);

	XCALL;

	if (geterr() == BadWindow)
		PASS;
	else
		FAIL;


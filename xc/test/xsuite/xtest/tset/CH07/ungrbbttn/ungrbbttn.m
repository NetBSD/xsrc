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
 * $XConsortium: ungrbbttn.m,v 1.7 94/04/17 21:06:55 rws Exp $
 */
>>TITLE XUngrabButton CH07
void
xname()
Display	*display = Dsp;
unsigned int 	button = Button1;
unsigned int 	modifiers = 0;
Window	grab_window = defwin(display);
>>EXTERN
>>SET end-function restoredevstate

/*
 * Returns True if the pointer is grabbed.  This is not a general purpose
 * routine since it knows about the Grab Button args.
 * When the pointer is grabbed then pointer events are not reported to
 * none grabbing clients.
 */
static
pgrabbed()
{
Display	*client2;
XEvent	ev;

	client2 = opendisplay();
	
	XSelectInput(client2, grab_window, PointerMotionMask|EnterWindowMask);
	/* Flush events for client2 */
	XSync(client2, True);

	/*
	 * Ensure that pointer either enters or moves within grab_window.
	 */
	(void) warppointer(display, grab_window, 1, 1);
	(void) warppointer(display, grab_window, 1, 2);

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

>>ASSERTION Good B 3
When a passive grab for the specified button/key
combination on the specified window
exists and it was grabbed by this client, then
a call to xname releases the grab.
>>STRATEGY
If no test extension:
  Grab button.
  Call xname to release grab.
  UNTESTED touch test only.
Place a passive grab on a button.
Activate grab.
Check that grab is active.
Release grab.
Call xname to remove grab.
Attempt to activate grab.
Verify that pointer is not grabbed.
>>CODE

	XGrabButton(display, button, modifiers, grab_window,
		False, PointerMotionMask, GrabModeAsync, GrabModeAsync,
		None, None);

	if (isdeleted())
		return;


	if (noext(1)) {
		XCALL;
		untested("There is no reliable test method, but a touch test was performed");
		return;
	} else
		CHECK;

	activate();
	if (pgrabbed())
		CHECK;
	else {
		delete("Could not set up grab");
		return;
	}

	relbuttons();

	XCALL;

	activate();
	if (pgrabbed()) {
		report("Grab was not removed");
		FAIL;
	} else
		CHECK;

	CHECKPASS(3);
>>ASSERTION Good B 3
A
.A modifiers
argument of
.S AnyModifier
is equivalent to issuing
the ungrab request for all possible modifier combinations, including
no modifiers.
>>STRATEGY
If no test extension:
  Touch test using AnyModifier.
Set passive grabs for different modifier combination on grab_window.
Set modifier to AnyModifier.
Call xname.
Verify that all grabs have been released.
>>CODE
unsigned int 	mods;

	if (noext(1)) {
		modifiers = AnyModifier;
		XCALL;
		untested("There is no reliable test method, but a touch test was performed");
		return;
	} else
		CHECK;

	mods = wantmods(display, 3);

	/* Try a few modifiers */
	modifiers = mods;
	XGrabButton(display, button, modifiers, grab_window,
		False, PointerMotionMask, GrabModeAsync, GrabModeAsync,
		None, None);

	/* Try no modifiers */
	modifiers = 0;
	XGrabButton(display, button, modifiers, grab_window,
		False, PointerMotionMask, GrabModeAsync, GrabModeAsync,
		None, None);

	modifiers = AnyModifier;
	XCALL;

	/* Try to activate with modifier combination */
	modifiers = mods;
	activate();
	if (pgrabbed()) {
		report("Grab with modifiers %s was not released", keymaskname(mods));
		FAIL;
	} else
		CHECK;

	/* Try to activate with no modifiers */
	modifiers = 0;
	activate();
	if (pgrabbed()) {
		report("Grab with no modifiers was not released");
		FAIL;
	} else
		CHECK;

	CHECKPASS(3);
>>ASSERTION Good B 3
A
.A button
argument of
.S AnyButton
is equivalent to issuing the
request for all possible buttons.
>>STRATEGY
If no test extension:
  Touch test using AnyButton.
Set passive grabs for different buttons.
Set button to AnyButton.
Call xname.
Verify that all grabs have been released.
>>CODE

	if (noext(1)) {
		button = AnyButton;
		XCALL;
		report("There is no reliable test method, but a touch test was performed");
		return;
	} else
		CHECK;

	button = Button1;
	XGrabButton(display, button, modifiers, grab_window,
		False, PointerMotionMask, GrabModeAsync, GrabModeAsync,
		None, None);

	if (nbuttons() > 1)
		button = Button2;
	XGrabButton(display, button, modifiers, grab_window,
		False, PointerMotionMask, GrabModeAsync, GrabModeAsync,
		None, None);

	button = AnyButton;
	XCALL;

	/* Check the first grab */
	button = Button1;
	activate();

	if (pgrabbed()) {
		report("Grab on button1 was not released with AnyButton");
		FAIL;
	} else
		CHECK;

	if (nbuttons() > 1) {
		button = Button2;
		activate();
		if (pgrabbed()) {
			report("Grab on button2 was not released with AnyButton");
			FAIL;
		} else
			CHECK;
	} else
		CHECK;

	CHECKPASS(3);
>>ASSERTION Good B 3
A call to xname has no effect on an active grab.
>>STRATEGY
If test extension available:
  Set up passive grab.
  Activate grab.
  Call xname.
  Verify that pointer is still grabbed.
>>CODE

	if (noext(1))
		return;

	XGrabButton(display, button, modifiers, grab_window,
		False, PointerMotionMask, GrabModeAsync, GrabModeAsync,
		None, None);

	activate();

	XCALL;

	if (pgrabbed())
		CHECK;
	else {
		report("A call to %s released already active grab", TestName);
		FAIL;
	}

	CHECKPASS(1);
>>ASSERTION Bad A
.ER BadValue modifiers mask ShiftMask LockMask ControlMask Mod1Mask Mod2Mask Mod3Mask Mod4Mask Mod5Mask AnyModifier
>>ASSERTION Bad A
.ER BadWindow

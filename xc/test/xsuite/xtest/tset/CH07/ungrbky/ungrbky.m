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
 * $XConsortium: ungrbky.m,v 1.11 94/04/17 21:06:56 rws Exp $
 */
>>TITLE XUngrabKey CH07
void

Display	*display = Dsp;
int 	keycode = grab_key_code(display);
unsigned int 	modifiers = 0;
Window	grab_window = defwin(display);
>>SET startup focusstartup
>>SET cleanup focuscleanup
>>EXTERN

static int minkc = -1,maxkc = -1;

static void set_kcs(dpy)
	Display *dpy;
{
	int kmin, kmax;

	if (minkc >= 8)
		return;
	XDisplayKeycodes(dpy, &kmin, &kmax);
	if (kmin < 8)
		kmin = 8;	/* For buggy servers */
	if (kmin >= kmax) {
		delete("Could not find two distinct key codes\n");
		delete("\t(min=%d, max=%d)\n", kmin, kmax);
		return;
	}
	minkc = kmin;
	maxkc = kmax;
}

#define	NMODS	8	/* Number of modifiers */

static int grab_key_code(dpy)
	Display *dpy;
{
	XModifierKeymap	*curmap;
	int i,key;

	set_kcs(dpy);
	curmap = XGetModifierMapping(dpy);
	for (key=minkc;key<=maxkc;key++) {
	    for (i = NMODS*curmap->max_keypermod; --i >= 0; )
		if (curmap->modifiermap[i] == key)
		    break;
	    if (i < 0) /* not a modifier, return it*/
		return key;
	}
	return minkc;
}

static int nongrab_key_code(dpy)
	Display *dpy;
{
	set_kcs(dpy);
	return (keycode == maxkc) ? minkc : maxkc;
}

#define	ACTPOSX	3
#define	ACTPOSY	6
/*
 * Warp to the grab_window and press the keys in modifiers and then press
 * the keycode in the 'keycode' arg.
 * (This activates the previously set up grab if the arg variables
 * have not been changed.)
 */
static void
activate_press()
{
	XSetInputFocus(display, grab_window, RevertToPointerRoot, CurrentTime);
	if (modifiers)
		modpress(display, modifiers);
	keypress(display, keycode);
}

/*
 * As above, but release (all) keys.
 */
static void
activate_release()
{
	XSetInputFocus(display, grab_window, RevertToPointerRoot, CurrentTime);
	keyrel(display, keycode);
	if (modifiers)
		modrel(display, modifiers);
}

/*
 * Returns True if the keyboard is grabbed.  This is not a general purpose
 * routine since it knows about the Grab Key args.
 * When the keyboard is grabbed then keyboard events are not reported to
 * non-grabbing clients. The grab client gets all keyboard events with
 * event window depending on owner_events and clients event mask.
 */
static
kgrabbed_check(client2)
Display	*client2;
{
XEvent	ev;
int	saved_keyc = keycode;

	XSelectInput(client2, grab_window, KeyPressMask|KeyReleaseMask);
	/* Flush events for client2 */
	XSync(client2, True);

	/*
	 * Ensure another-key press release pair in grab_window.
	 */
	keycode = nongrab_key_code(display);
	activate_press();
	activate_release();
	keycode = saved_keyc;

	XSync(client2, False);
	if (getevent(client2, &ev)) {
		/*
		 * An event was reported - keyboard isn't grabbed, do a sanity
		 * check on the type of event.
		 */
		if (ev.type != KeyPress && ev.type != KeyRelease) {
			delete("Unexpected event received in kgrabbed()");
			delete("  event type %s", eventname(ev.type));
		}
		return(False);
	} else {
		return(True);
	}
}

/*
 * as for kgrabbed_check() but used where number of fd's consumed by
 * multiple calls doesn't outweigh convenience of avoiding extra arg.
 */
static
kgrabbed()
{
Display	*client2;

	client2 = opendisplay();
	return kgrabbed_check(client2);
}

>>ASSERTION Good B 3
When the specified key/modifier combination has been grabbed by this
client, then a call to xname releases the grab.
>>STRATEGY
If extension available:
  Setup up grab on specified key/modifier combination using XGrabKey.
  Activate grab.
  Check keyboard grabbed.
  Deactivate grab.
  Check keyboard not grabbed.
  Call xname to release grab.
  Attempt to reactivate Grab.
  Check keyboard still not grabbed.
  Release keys.
else
  Grab key.
  Touch test.
  Report untested.
>>CODE

	XGrabKey(display, keycode, modifiers, grab_window, False,
		GrabModeAsync, GrabModeAsync);

	if (noext(0)) {
		XCALL;
		report("There is no reliable test method, but a touch test was performed");
		return;
	} else
		CHECK;

	activate_press();
	if (!kgrabbed()) {
		delete("Failed to activate grab on keycode %d.", keycode);
		relalldev();
		return;
	} else
		CHECK;

	activate_release();
	if (kgrabbed()) {
		delete("Failed to deactivate grab on keycode %d.", keycode);
		relalldev();
		return;
	} else
		CHECK;

	XCALL;

	activate_press();
	if (kgrabbed()) {
		report("Failed to release initial grab on keycode %d.", keycode);
		FAIL;
	} else
		CHECK;
	relalldev();

	CHECKPASS(4);

>>ASSERTION Good B 3
A
.A modifiers
argument of
.S AnyModifier
releases all grabs by this client for the specified key and all possible
modifier combinations.
>>STRATEGY
If extensions are available:
  Place passive grab with a bunch of modifier keys.
  Activate grab.
  Check keyboard grabbed.
  Deactivate grab.
  Check keyboard not grabbed.
  Call xname with a modifiers of AnyModifier.
  Attempt to reactivate Grab.
  Check keyboard still not grabbed.
  Release keys.

  Place passive grab with just keycode (no modifiers).
  Activate grab.
  Check keyboard grabbed.
  Deactivate grab.
  Check keyboard not grabbed.
  Call xname with a modifiers of AnyModifier.
  Attempt to reactivate Grab.
  Check keyboard still not grabbed.
  Release keys.
else
  Report untested.
>>CODE
unsigned int 	mods;

	if (noext(0)) {
		return;
	} else
		CHECK;

	modifiers = mods = wantmods(display, 4);

	XGrabKey(display, keycode, mods, grab_window, False,
		GrabModeAsync, GrabModeAsync);

	activate_press();
	if (!kgrabbed()) {
		delete("Failed to activate grab on keycode %d.", keycode);
		delete("  Modifiers used %s", keymaskname((unsigned long)mods));
		relalldev();
		return;
	} else
		CHECK;

	activate_release();
	if (kgrabbed()) {
		delete("Failed to deactivate grab on keycode %d.", keycode);
		delete("  Modifiers used %s", keymaskname((unsigned long)mods));
		relalldev();
		return;
	} else
		CHECK;

	modifiers = AnyModifier;
	XCALL;

	modifiers = mods;
	activate_press();
	if (!kgrabbed())
		CHECK;
	else {
		report("Grab not released for AnyModifier on keycode %d", keycode);
		report("  Modifier keys used %s", keymaskname((unsigned long)mods));
		FAIL;
	}
	relalldev();

	if (kgrabbed()) { /* releasing keys may have done the job. */
		delete("Could not release grab for second part of test");
		return;
	} else
		CHECK;

	XGrabKey(display, keycode, 0, grab_window, False,
		GrabModeAsync, GrabModeAsync);

	modifiers = 0;
	activate_press();
	if (!kgrabbed()) {
		delete("Failed to activate grab on keycode %d.", keycode);
		relalldev();
		return;
	} else
		CHECK;

	activate_release();
	if (kgrabbed()) {
		delete("Failed to deactivate grab on keycode %d.", keycode);
		relalldev();
		return;
	} else
		CHECK;

	modifiers = AnyModifier;
	XCALL;

	modifiers = 0;
	activate_press();
	if (!kgrabbed())
		CHECK;
	else {
		report("Grab with no modifier and keycode %d was not released.", keycode);
		FAIL;
	}

	relalldev();

	CHECKPASS(8);
>>ASSERTION Good B 3
A
.A keycode
argument of
.S AnyKey
releases all grabs by this client for the specified modifiers and all keys.
>>STRATEGY
If extensions are available:
  Place passive grab with a bunch of modifier keys and given keycode.
  Activate grab.
  Check keyboard grabbed.
  Deactivate grab.
  Check keyboard not grabbed.
  Call xname with a keycode of AnyKey.
  Attempt to reactivate Grab.
  Check keyboard still not grabbed.
  Release keys.
  Repeat for all keycodes in the range min_keycode to max_keycode
	except for those in the modifier bunch.

else
  Report untested.
>>CODE
unsigned int 	mods;
int	i;
int 	minkc, maxkc;
Display	*client2;
int	keycount = 0;

	client2 = opendisplay();

	XDisplayKeycodes(display, &minkc, &maxkc);

	if (noext(0)) {
		return;
	} else
		CHECK;

	modifiers = mods = wantmods(display, 4);

	for(keycount=0, i = minkc; i <= maxkc; i++) {
		if (ismodkey(mods, i))
			continue;
		keycount++;
		XGrabKey(display, i, mods, grab_window, False,
			GrabModeAsync, GrabModeAsync);

		keycode = i;
		activate_press();
		if (!kgrabbed_check(client2)) {
			delete("Failed to activate grab on keycode %d.", keycode);
			delete("  Modifiers used %s", keymaskname((unsigned long)mods));
			relalldev();
			return;
		} else
			CHECK;

		activate_release();
		if (kgrabbed_check(client2)) {
			delete("Failed to deactivate grab on keycode %d.", keycode);
			delete("  Modifiers used %s", keymaskname((unsigned long)mods));
			relalldev();
			return;
		} else
			CHECK;

		keycode = AnyKey;
		XCALL;

		keycode = i;
		activate_press();
		if (!kgrabbed_check(client2))
			CHECK;
		else {
			report("Grab not released for AnyKey on keycode %d", i);
			report("  Modifier keys used %s", keymaskname((unsigned long)mods));
			FAIL;
		}
		relalldev();

		if (kgrabbed_check(client2)) { /* releasing keys may have done the job. */
			delete("Could not release grab for second part of test");
			return;
		} else
			CHECK;

		relalldev();
	}
	trace("Tested on %d non-modifier keys, with %d modifiers - all from %d keys (%d to %d).",
		keycount, bitcount((unsigned long)mods), 
		1+maxkc-minkc, minkc, maxkc);

	relalldev();

	CHECKPASS(1+4*keycount);
>>ASSERTION Good B 3
A call to xname has no effect on an active grab.
>>STRATEGY
If extension available:
  Set passive grab.
  Activate it.
  Check keyboard is grabbed.
  Call xname to attempt to release grab.
  Check keyboard still grabbed.
  Release keys to release grab.
  If still grabbed:
    Delete with message.
  else
    Call xname to attempt to release grab.
    Attempt to activate grab.
    Check keyboard still not grabbed.
  Release keys.
else
  Report untested.
>>CODE

	if (noext(0)) {
		return;
	} else
		CHECK;
	XGrabKey(display, keycode, modifiers, grab_window, False, GrabModeAsync, GrabModeAsync);

	activate_press();

	if (!kgrabbed()) {
		delete("Failed to set initial grab.");
		return;
	} else
		CHECK;

	XCALL;

	if (!kgrabbed()) {
		report("Released active grab.");
		FAIL;
	} else
		CHECK;

	relalldev(); /* should release grab, allowing a final test */

	if (kgrabbed()) {
		delete("grab still active after key release.");
		return;
	} else
		CHECK;

	XCALL;

	activate_press();
	if (kgrabbed()) {
		report("Couldn't release grab when inactive.");
		FAIL;
	} else
		CHECK;

	CHECKPASS(5);

>>ASSERTION Bad A
When the specified keycode is not in the range
specified by min_keycode and max_keycode in the connection setup or
.S AnyKey ,
then a
.S BadValue
error occurs.
>>STRATEGY
Get min and max keycodes.
Attempt to grab key less than the minimum.
Verify that a BadValue error occurs.
If the maximum is less than 255
  Attempt to grab key greater than the maximum
  Verify a BadValue error occurs.
>>CODE BadValue
int 	minkc, maxkc;

	XDisplayKeycodes(display, &minkc, &maxkc);

	keycode = minkc - 1;
	XCALL;

	if (geterr() == BadValue)
		CHECK;

	if (maxkc < 255) {
		keycode = maxkc + 1;

		XCALL;

		if (geterr() == BadValue)
			CHECK;
	} else
		CHECK;

	CHECKPASS(2);
>>ASSERTION Bad A
.ER BadValue modifiers mask ShiftMask LockMask ControlMask Mod1Mask Mod2Mask Mod3Mask Mod4Mask Mod5Mask AnyModifier
>>ASSERTION Bad A
.ER BadWindow

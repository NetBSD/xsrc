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
 * $XConsortium: grbky.m,v 1.11 94/04/17 21:06:22 rws Exp $
 */
>>TITLE XGrabKey CH07
void

Display	*display = Dsp;
int 	keycode = grab_key_code(display); /* always min. k.c. */
unsigned int 	modifiers = 0;
Window	grab_window = defwin(display);
Bool	owner_events = False;
int 	pointer_mode = GrabModeAsync;
int 	keyboard_mode = GrabModeAsync;
>>SET startup focusstartup
>>SET cleanup focuscleanup
>>EXTERN

/*
 * as unwarppointer but don't free ptr as this done at tpcleanup time.
 */
static void
my_unwarppointer(display, ptr)
Display *display;
PointerPlace *ptr;
{
        XWarpPointer(display, None, ptr->oroot, 0, 0, 0, 0, ptr->ox, ptr->oy);
}


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

static char	*WindowTree[]= {
	".",
		"child1 . (10,10) 30x30",
		"child2 . (50,50) 30x30",
			"grandchild child2 (2,2) 20x20",
};

static int NWindowTree = NELEM(WindowTree);

>>ASSERTION Good B 3
A call to xname establishes a passive grab on the keyboard that is activated
in the future by
the specified key being logically pressed,
the specified modifier keys being logically down,
no other modifier keys being logically down,
the
.A grab_window
being the focus window or an ancestor of the focus window
or being a descendant of the focus window that contains the pointer
and
a passive grab on the same key combination not existing on any
ancestor of
.A grab_window .
>>STRATEGY
Call xname as touch test.
If extensions available:
  Set focus to grab window.
  Simulate a key press of key.
  Verify that keyboard is now grabbed.
  Release grab & key.

  Set up a grab with xname for a key.
  Set focus to grab window.
  Simulate press of another key.
  Simulate the key press.
  Verify that the keyboard is not grabbed.
  Release grab & key.

  Set up a grab with xname for a key and modifier keys.
  Set focus to grab window.
  Simulate modifier key presses.
  Simulate the key press.
  Verify that the keyboard is grabbed.
  Release grab & key.

  Set up a grab with xname for a key and modifier keys.
  Set focus to grab window.
  Simulate modifier key presses.
  Simulate extra modifier key presses.
  Simulate the key press.
  Verify that the keyboard is not grabbed.
  Release grab & key.

  Make a child of current grab_window become focus window.
  Call xname for a passive grab with no modifiers on parent.
  Set focus window to child of grab window.
  Activate the grab.
  Check if grabbed.
  Release key & grab.

  Have grab_window a child of focus window and containing the pointer.
  Call xname with no modifiers.
  Activate the grab.
  Check if grabbed.
  Release key & grab.

  Have grab_window a child of focus window and not containing the pointer.
  Call xname with no modifiers.
  Activate the grab.
  Check not grabbed.
  Release key & grab.

  Set grab with no modifiers and grab_window a top-level window.
  Set another grab on child of top-level window.
  Discard event queue.
  Attempt to activate second grab (in child).
  Check event reported w.r.t. parent.
  Check that no event reported for child.
  Check no further events outstanding.
  Check that a grab is active.
  Release parent grab and check grab not active on child.
  Activate child and check key grabbed.
  Release any grabs and keys outstanding.

>>CODE
unsigned int    mask;
int     onemod;
int	keysave,otherkc;
Window	parent,child1,child2,gchild;
struct buildtree *tree;
PointerPlace *p;
XEvent	ev;
int	cantdoit;

	if (!(cantdoit=noext(0)) && kgrabbed()) { /* Sanity check */
		delete("Keyboard seemed to be grabbed before doing test");
		return;
	}

	XCALL;

	if (cantdoit) {
		untested("There is no reliable test method, but a touch test was performed");
		XUngrabKey(display, AnyKey, AnyModifier, grab_window);
		return;
	}

	/*
	 * --- Simple case no modifiers.
	 */
	activate_press();

	if (kgrabbed()) {
		CHECK;
	} else {
		report("Keyboard was not grabbed after key press");
		FAIL;
	}
	relalldev();
	XUngrabKey(display, AnyKey, AnyModifier, grab_window);

	/*
	 * --- Press another key instead of the grabbed key.
	 * The keyboard should not be grabbed.
	 */
	if ((otherkc=nongrab_key_code(display)) != keycode) {
		XCALL;

		keysave = keycode;
		keycode = otherkc;
		activate_press();
		keycode = keysave; /* restore to normality */
		if (kgrabbed()) {
			report("Keyboard was grabbed although another key was pressed");
			FAIL;
		} else
			CHECK;
		relalldev();
	} else {
		trace("Only one key supported");
		CHECK;
	}
	XUngrabKey(display, AnyKey, AnyModifier, grab_window);

	/*
	 * --- Set up a grab with modifiers.
	 */
	modifiers = wantmods(display, 2);
	grab_window = defwin(display);
	trace("Grabbing keycode %d with mods %s", keycode,
		keymaskname((unsigned long)modifiers));
	XCALL;

	activate_press();
	if (kgrabbed()) {
		CHECK;
	} else {
		report("Keyboard was not grabbed for keycode %d and %s", keycode,
			keymaskname((unsigned long)modifiers));
		FAIL;
	}
	relalldev();
	XUngrabKey(display, AnyKey, AnyModifier, grab_window);

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

	XSetInputFocus(display, grab_window, RevertToPointerRoot, CurrentTime);
	/* Pressing an extra modifier */
	modpress(display, mask);
	keypress(display, keycode);

	if (mask && kgrabbed()) {
		report("Keyboard was grabbed when there were extra modifier keys down");
		FAIL;
	} else {
		/*
		 * If mask was zero this means that there are no modifiers KeyCodes
		 * available.  This is unlikely and realy means that we cannot test
		 * this part of the assertion.  However in this case this part of
		 * the assertion has no meaning so say it passes.
		 */
		CHECK;
	}
	relalldev();
	XUngrabKey(display, AnyKey, AnyModifier, grab_window);

/* Make a child of current grab_window become focus window. */
	parent = grab_window;
	tree = buildtree(display, parent, WindowTree, NWindowTree);
	child1 = btntow(tree, "child1");
/* Call xname for a passive grab with no modifiers on parent. */
	modifiers = 0;
	XCALL;
/* Set focus window to child of grab window. */
	XSetInputFocus(display, parent, RevertToPointerRoot, CurrentTime);
/* Activate the grab. */
	keypress(display, keycode);
/* Check if grabbed. */
	if (kgrabbed()) {
		CHECK;
	} else {
		report("Key not grabbed when focus is child of grab_window.");
		FAIL;
	}
/* Release key & grab. */
	relalldev();
	XUngrabKey(display, AnyKey, AnyModifier, grab_window);


/* Have grab_window a child of focus window and containing the pointer. */
	child2 = btntow(tree, "child2");
	gchild = btntow(tree, "grandchild");
	grab_window = gchild;
	XSetInputFocus(display, child2, RevertToPointerRoot, CurrentTime);
	p = warppointer(display, grab_window, 2, 2);
/* Call xname with no modifiers. */
	modifiers = 0;
	XCALL;
/* Activate the grab. */
	keypress(display, keycode);
/* Check if grabbed. */
	if (kgrabbed()) {
		CHECK;
	} else {
		report("Key not grabbed with grab_window a descendent of focus");
		report(" and containing the pointer.");
		FAIL;
	}
	my_unwarppointer(display, p);
/* Release key & grab. */
	relalldev();
	XUngrabKey(display, AnyKey, AnyModifier, grab_window);

/* Have grab_window a child of focus window and not containing the pointer. */
	grab_window = gchild;
	XSetInputFocus(display, child2, RevertToPointerRoot, CurrentTime);
	p = warppointer(display, child1, 2, 2);
/* Call xname with no modifiers. */
	modifiers = 0;
	XCALL;
/* Activate the grab. */
	keypress(display, keycode);
/* Check not grabbed. */
	if (!kgrabbed()) {
		CHECK;
	} else {
		report("Key grabbed with grab_window a descendent of focus");
		report(" and not containing the pointer.");
		FAIL;
	}
	my_unwarppointer(display, p);
/* Release key & grab. */
	relalldev();
	XUngrabKey(display, AnyKey, AnyModifier, grab_window);

/* Set grab with no modifiers and grab_window a top-level window. */
	modifiers = 0;
	grab_window = parent;
	XCALL;
/* Set another grab on child of top-level window. */
	grab_window = child1;
	XCALL;
/* Discard event queue. */
	XSync(display, True);
/* Attempt to activate second grab (in child). */
	grab_window = child1;
	activate_press();
/* Check event reported w.r.t. parent. */
	if (XCheckWindowEvent(display, parent, KeyPressMask, &ev))
		CHECK;
	else {
		report("No KeyPress event for parent window.");
		FAIL;
	}
/* Check that no event reported for child. */
	if (!XCheckWindowEvent(display, child1, KeyPressMask, &ev))
		CHECK;
	else {
		report("Unexpected KeyPress event for child window.");
		FAIL;
	}
/* Check no further events outstanding. */
	if (getevent(display, &ev) != 0) {
		report("Unexpected %s event on %s window.",
			eventname(ev.type),
			ev.xany.window == parent ? "parent" : (
				ev.xany.window == child1 ? "child" : "unexpected"
			));
		FAIL;
	} else
		CHECK;
/* Check that a grab is active. */
	if (kgrabbed())
		CHECK;
	else {
		report("Key not grabbed after set in parent and child and");
		report(" activation attempt in child.");
		FAIL;
	}
/* Release parent grab and check grab not active on child. */
	relalldev();
	XUngrabKey(display, AnyKey, AnyModifier, parent);
	/* grab_window is child1, still, so kgrabbed() checking OK. */
	if (kgrabbed()) {
		report("Child grab became active after interfering parent released.");
		FAIL;
	} else
		CHECK;
/* Activate child and check key grabbed. */
	/* grab_window still child so activate_press etc. OK */
	activate_press();
	if (!kgrabbed()) {
		report("Child grab not active after interfering parent released.");
		FAIL;
	} else
		CHECK;
/* Release any grabs and keys outstanding. */
	relalldev();
	XUngrabKey(display, AnyKey, AnyModifier, grab_window);
	
	CHECKPASS(13);
>>ASSERTION Good B 3
When the conditions for activating the grab are otherwise satisfied
and the keyboard is already grabbed,
then no active grab is established.
>>STRATEGY
If extensions are available:
  Create two windows that do not overlap.
  Check that at least two keycodes are available.
  Set and activate grab on first window.
  Check grab activated.
  Set grab on second window on another keycode.
  Attempt to activate grab on second window, using other keycode.
  Check that only one KeyPress event received.
  Check it was reported from first grab window.
  Check grab still outstanding.
  Release keys & grabs.
else
  Report untested.
>>CODE
Window	w1,w2;
XEvent	ev;
int	n;
int	grabbed;
int	keysave,otherkc;

	if (noext(0))
		return;
	else
		CHECK;
	w1 = defwin(display);
	w2 = defwin(display);
	keysave = keycode;
	otherkc = nongrab_key_code(display);

	if (otherkc == keycode) {
		delete("Need at least two distinct keycodes.");
		return;
	} else
		CHECK;

	grab_window = w1;
	XCALL;
	activate_press();
	if (!kgrabbed()) {
		delete("Could not activate first grab.");
		relalldev();
		XUngrabKey(display, AnyKey, AnyModifier, grab_window);
		return;
	} else
		CHECK;

	grab_window = w2;
	keycode = otherkc;
	XCALL;
	XSync(display, True);
	activate_press();
	keycode = keysave;
	if ((n=getevent(display, &ev)) != 1) {
		report("Received %d events on key press when grab active, expected just %d.", n, 1);
		FAIL;
	} else
		CHECK;
	if (n > 0 && ev.type != KeyPress) {
		report("First event was type %s instead of KeyPress.",eventname(ev.type));
		FAIL;
	} else
		CHECK;
	grabbed = kgrabbed();
	if (n > 0 && ev.xany.window != w1) {
		report("Event appeared on %s window instead of first grab window.",
			ev.xany.window == w2 ? "second grab" : "unexpected");
		if (grabbed)
			report("Grab appears to have moved.");
		else
			report("Grab has been prematurely released.");
		FAIL;
	} else if (!grabbed) {
		report("Key grab erroneously released on trying to activate second.");
		FAIL;
	} else
		CHECK;
	relalldev();
	XUngrabKey(display, AnyKey, AnyModifier, w1);
	XUngrabKey(display, AnyKey, AnyModifier, w2);

	CHECKPASS(6);
>>ASSERTION Good B 3
When the conditions for activating the grab are satisfied
and the grab subsequently becomes active, then
the last-keyboard-grab time is set to the time at which the key was pressed.
>>STRATEGY
If extensions are available:
  Set and activate grab.
  Check activated.
  Check activating event received.
  Check event type and event window are KeyPress and grab_window.
  Attempt XUngrabKeyboard at time just before event time.
  Check still grabbed.
  Attempt XUngrabKeyboard at time equal to event time.
  Check no longer grabbed.
  Release grab & key.
else
  Report untested.
>>CODE
XEvent	ev;

	if (noext(0))
		return;
	else
		CHECK;
	XSync(display, True);
	XCALL;
	activate_press();
	if (!kgrabbed()) {
		delete("Failed to activate grab.");
		return;
	} else
		CHECK;
	if (!getevent(display, &ev)) {
		delete("No event reported for activating grab.");
		return;
	} else
		CHECK;
	/* sanity check on the event. */
	if (ev.type != KeyPress || ev.xany.window != grab_window) {
		delete("Reported event has type %s and event window 0x%x", eventname(ev.type), ev.xany.window);
		delete(" expected KeyPress and 0x%x.", grab_window);
		return;
	} else
		CHECK;
	/* now set up OK */
	trace("Grabbed at time 0x%lx.",(unsigned long)ev.xkey.time);
	XUngrabKeyboard(display, ev.xkey.time - 1);
	if (!kgrabbed()) {
		report("Last keyboard grab time set earlier than reported event time.");
		FAIL;
	} else
		CHECK;
	XUngrabKeyboard(display, ev.xkey.time);
	if (kgrabbed()) {
		report("Last keyboard grab time set later than reported event time.");
		FAIL;
	} else
		CHECK;
	relalldev();
	XUngrabKey(display, AnyKey, AnyModifier, grab_window);

	CHECKPASS(6);
	
>>ASSERTION Good B 3
When the grab subsequently becomes active and later
the logical state of the
keyboard has the specified key released,
then the active grab is terminated automatically.
>>#(independent of the logical state of the modifier keys).
>>STRATEGY
If extension available:
    Set grab with modifiers.
    Activate grab.
    Check grabbed.
    Simulate keycode only key release with testing extension.
    Check for grab release.
    Simulate modifiers only key release with testing extension.
    Check for grab release.
    Release grab & keys.
  Repeat but in opposite order with grab release expected on keycode up only.
  Do the same for keycode = AnyKey, no modifiers.
    (releasing the grab with XUngrabKey before expected !kgrabbed() tests.)
else
  Report untested.
>>CODE

	if (noext(0))
		return;
	else
		CHECK;
	modifiers = wantmods(display, 2);
	trace("Grabbing keycode %d with mods %s", keycode,
		keymaskname((unsigned long)modifiers));
	XCALL;

	activate_press();
	if (kgrabbed()) {
		CHECK;
	} else {
		delete("Keyboard was not grabbed for keycode %d and %s", keycode,
			keymaskname((unsigned long)modifiers));
		return;
	}
	trace("releasing keycode %d only.", keycode);
	keyrel(display, keycode); /* leaving modifiers down. */
	if (!kgrabbed()) {
		CHECK;
	} else {
		report("Keyboard still grabbed for keycode %d and %s after keycode release.", keycode,
			keymaskname((unsigned long)modifiers));
		FAIL;
	}
	modrel(display, modifiers); /* there go the modifiers. */
	if (!kgrabbed()) {
		CHECK;
	} else {
		report("Keyboard still grabbed for keycode %d and %s even after all keys released.", keycode,
			keymaskname((unsigned long)modifiers));
		FAIL;
	}
	relalldev();
	XUngrabKey(display, AnyKey, AnyModifier, grab_window);
	/* repeat but now release modifiers first and keycode last */
	trace("Grabbing keycode %d with mods %s", keycode,
		keymaskname((unsigned long)modifiers));
	XCALL;

	activate_press();
	if (kgrabbed()) {
		CHECK;
	} else {
		delete("Keyboard was not grabbed for keycode %d and %s", keycode,
			keymaskname((unsigned long)modifiers));
		return;
	}
	trace("releasing modifiers %s only.", keymaskname((unsigned long)modifiers));
	modrel(display, modifiers); /* leaving keycode down. */
	if (kgrabbed()) {
		CHECK;
	} else {
		report("Keyboard no longer grabbed for keycode %d and %s after %s release.", keycode,
			keymaskname((unsigned long)modifiers),
			keymaskname((unsigned long)modifiers));
		FAIL;
	}
	keyrel(display, keycode); /* there goes the keycode key. */
	if (!kgrabbed()) {
		CHECK;
	} else {
		report("Keyboard still grabbed for keycode %d and %s even after all keys released.", keycode,
			keymaskname((unsigned long)modifiers));
		FAIL;
	}
	relalldev();
	XUngrabKey(display, AnyKey, AnyModifier, grab_window);

	/*
	 * Finally test AnyKey grabs only release when activating key released.
	 */
	modifiers = 0;
	keycode = AnyKey;
	trace("Grabbing keycode AnyKey with mods %s",
		keymaskname((unsigned long)modifiers));
	XCALL;

	keycode = grab_key_code(display);
	activate_press();
	if (kgrabbed()) {
		CHECK;
	} else {
		delete("Keyboard was not grabbed for keycode AnyKey when keycode %d only pressed.", keycode);
		return;
	}
	trace("releasing non activating keycode only.");
	/* already released by previous kgrabbed test but can't check
	 * if grabbed with kgrabbed as that would just make it active if
	 * (erroneously) passive. So, try and release grab with XUngrabKey
	 * which will do nothing if its active, as it should be, but will
	 * allow subsequent kgrabbed to return False if it wasn't active.
	 */
	XUngrabKey(display, AnyKey, AnyModifier, grab_window);
	if (kgrabbed()) {
		CHECK;
	} else {
		delete("Keyboard was not grabbed for keycode AnyKey after press/release of non-activating key.");
		return;
	}
	trace("releasing activating keycode %d only.", keycode);
	keyrel(display, keycode);
	/*
	 * now the XUngrabKey should find it non-active so kgrabbed => False
	 */
	XUngrabKey(display, AnyKey, AnyModifier, grab_window);
	if (!kgrabbed()) {
		CHECK;
	} else {
		report("Keyboard still grabbed for keycode AnyKey after keycode %d release.", keycode);
		FAIL;
	}
	relalldev();
	XUngrabKey(display, AnyKey, AnyModifier, grab_window);

	CHECKPASS(10);
>>ASSERTION Good B 3
A call to xname overrides all previous passive grabs by the same client on the
same key combinations on the same window.
>>STRATEGY
If extensions available:
  Create a window for event reporting and set event mask to KeyPressMask.
  Set a grab with owner_events False on another window.
  Set a grab with owner_events True on this other window.
  Activate the grab.
  Check it activated.
  Simulate a KeyPress in the reporting window.
  Check that the reported event has event window equal to reporting
    window rather than grab_window showing that second overrode first.
  Release grabs & keys.
else
  Report untested.
>>CODE
Window	reportwin;
XEvent	ev;

	if (noext(0))
		return;
	else
		CHECK;
	reportwin = defwin(display);
	XCALL;
	owner_events = True;
	XCALL;
	activate_press();
	if (!kgrabbed()) {
		delete("Failed to activate grab.");
		return;
	} else
		CHECK;
	XSync(display, True); /* empty event-Q */
	XSelectInput(display, reportwin, KeyPressMask);
	/* now cause key-press in reportwin */
	XSetInputFocus(display, reportwin, RevertToPointerRoot, CurrentTime);
	keypress(display, nongrab_key_code(display));
	/* check it was reported w.r.t. reportwin, not grab_window */
	if (!getevent(display, &ev)) {
		delete("Event not reported to grabbing client.");
		return;
	} else
		CHECK;
	if (ev.type != KeyPress) {
		delete("Reported event of type %s rather than KeyPress.", eventname(ev.type));
		return;
	} else
		CHECK;
	if (ev.xany.window != reportwin) {
		if (ev.xany.window == grab_window) {
			report("First grab not overridden by second.");
			FAIL;
		} else {
			delete("Unexpected window 0x%lx in reported event (not grab_window or reportwin).",
				(unsigned long)ev.xany.window);
			return;
		}
	} else
		CHECK;
	
	relalldev();
	XUngrabKey(display, AnyKey, AnyModifier, grab_window);

	CHECKPASS(5);
>>ASSERTION Good B 3
When the
.A modifiers
argument is
.S AnyModifier ,
then this is equivalent to separate calls to xname for all
possible modifier combinations including no modifiers.
>>STRATEGY
If extensions are available:
  Place passive grab with a modifiers of AnyModifier.
  Press a bunch of modifier keys.
  Press keycode to activate grab.
  Verify that grab is activated.
  Release keys & grab.

  Press keycode (no modifiers).
  Verify that grab is active.
else
  Perform touch test.
  Report untested.
>>CODE
unsigned int 	mods;

	modifiers = AnyModifier;
	XCALL;

	if (noext(0)) {
		untested("There is no reliable test method, but a touch test was performed");
		return;
	} else
		CHECK;

	modifiers = mods = wantmods(display, 4);

	/*
	 * modifiers was AnyModifier, several modifier keys are held down.
	 */
	activate_press();
	if (kgrabbed())
		CHECK;
	else {
		report("Grab not activated for AnyModifier on keycode %d", keycode);
		report("  Modifiers used %s", keymaskname((unsigned long)mods));
		FAIL;
	}

	/* Release all grabs, keys and modifiers */
	relalldev();
	XUngrabKey(display, AnyKey, AnyModifier, grab_window);

	if (kgrabbed()) {
		delete("Could not release grab for second part of test");
		return;
	} else
		CHECK;

	modifiers = AnyModifier;
	XCALL;
	modifiers = 0; /* cause keycode only to be pressed */
	activate_press();
	if (kgrabbed())
		CHECK;
	else {
		report("Grab with AnyModifier was not activated by pressing keycode %d with", keycode);
		report("  no modifiers");
		FAIL;
	}

	/* Release all grabs, keys etc. */
	relalldev();
	XUngrabKey(display, AnyKey, AnyModifier, grab_window);

	CHECKPASS(4);
>>ASSERTION Good B 3
It is not required that all modifiers specified have
currently assigned KeyCodes.
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

	if (noext(0))
		return;
	else
		CHECK;

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

	activate_press();

	if (kgrabbed())
		CHECK;
	else {
		report("Passive grab not set when the modifier did not have a current keycode");
		FAIL;
	}
	/* Release all grabs, keys etc. */
	relalldev();
	XUngrabKey(display, AnyKey, AnyModifier, grab_window);

	CHECKPASS(6);
>>ASSERTION Good B 3
When the
.A keycode
argument is
.S AnyKey ,
then this is equivalent to separate calls to xname for
all possible KeyCodes.
>>STRATEGY
If extensions are available:
  Set up a passive grab using AnyKey.
    Activate grab with simulated key press.
    Verify that keyboard is grabbed.
    Release key & grab.
    Repeat for other keycodes in range min_keycode to max_keycode.
  Release all grabs & keys.
else
  Touch test using AnyKey.
  Report untested.
>>CODE
int 	i;
int 	minkc, maxkc;
Display	*client2;

	client2 = opendisplay();

	XDisplayKeycodes(display, &minkc, &maxkc);

	keycode = AnyKey;
	XCALL;

	if (noext(0)) {
		untested("There is no reliable test method, but a touch test was performed");
		return;
	} else
		CHECK;

	XSetInputFocus(display, grab_window, RevertToPointerRoot, CurrentTime);

	for (i = minkc; i <= maxkc; i++) {
		keycode = i; /* let nongrab_key_code know */
		keypress(display, i);
		if (kgrabbed_check(client2))
			CHECK;
		else {
			report("Passive grab of AnyKey, not grabbed for keycode %d", (int)i);
			FAIL;
		}

		/*
		 * Release this grab and try next key.
		 */
		relkeys();
		/*
		 * This XUngrabKey must be in or else the key release is not
		 * enough because the press/release in kgrabbed will
		 * trigger/release the grab again. To test properly we
		 * must try and remove the grab, which only works if not
		 * active.
		 */
		XUngrabKey(display, AnyKey, AnyModifier, grab_window);
		XSync(display, True);
		if (kgrabbed_check(client2)) {
			delete("Could not release grab for next part of test");
			return;
		} else
			CHECK;
		/*
		 * The XUngrabKey requires us to have another passive
		 * grab installed for next iteration.
		 */
		keycode = AnyKey;
		XCALL;
	}
	/* Release all grabs, keys etc. */
	relalldev();
	XUngrabKey(display, AnyKey, AnyModifier, grab_window);

	CHECKPASS(1+2*(1 + maxkc - minkc));
>>ASSERTION Good B 3
When the event window for an active grab becomes not viewable, then the
grab is released automatically.
>>STRATEGY
If extension is available:
  Set up grab on a child window (to avoid window manager interference).
  Activate grab.
  Check grabbed.
  Unmap grab_window.
  Map grab_window back again (to perform grab check).
  Check no longer grabbed.
  Release key & grab.
else
  Report untested.
>>CODE
Window	win;
struct area	a;

	if (noext(0))
		return;
	else
		CHECK;
	setarea(&a, 2, 2, 0, 0);
	win = crechild(display, grab_window, &a);
	grab_window = win;
	XCALL;
	activate_press();
	if (!kgrabbed()) {
		delete("Failed to activate grab.");
		return;
	} else
		CHECK;
	XUnmapWindow(display, grab_window);
	XSync(display, True);
	XMapWindow(display, grab_window);
	XSync(display, True);
	if (kgrabbed()) {
		report("Unmapping grab_window did not inactivate grab.");
		FAIL;
	} else
		CHECK;
	/* Release all grabs, keys etc. */
	relalldev();
	XUngrabKey(display, AnyKey, AnyModifier, grab_window);

	CHECKPASS(3);
>>ASSERTION Bad A
When the specified keycode is not in the range
specified by min_keycode and max_keycode in the connection setup or
.S AnyKey ,
then a
.S BadValue
error occurs.
>>STRATEGY
Call xname with keycode less than min_keycode.
Verify that a BadValue error occurs.
Call xname with keycode greater than max_keycode if it is less than 255.
Verify that a BadValue error occurs.
>>CODE BadValue
int 	minkc, maxkc;

	XDisplayKeycodes(display, &minkc, &maxkc);

	keycode = minkc - 2;
	XCALL;

	if (geterr() == BadValue)
		CHECK;

	/*
	 * Since the protocol only has one byte for the key then this
	 * assertion cannot be tested when max_keycode is 255.
	 */
	if (maxkc < 255) {

		keycode = maxkc+1;

		XCALL;

		if (geterr() == BadValue)
			CHECK;
	} else
		CHECK;

	CHECKPASS(2);
>>ASSERTION Bad A
.ER BadValue modifiers mask ShiftMask LockMask ControlMask Mod1Mask Mod2Mask Mod3Mask Mod4Mask Mod5Mask AnyModifier
>>ASSERTION Bad A
.ER BadValue owner_events True False
>>ASSERTION Bad A
.ER BadValue pointer_mode GrabModeSync GrabModeAsync
>>ASSERTION Bad A
.ER BadValue keyboard_mode GrabModeSync GrabModeAsync
>>ASSERTION Bad A
.ER Access grab
>>STRATEGY
Grab key/modifier.
Create client2.
Attempt to grab same key modifier for client2.
Verify BadAccess error.
>>CODE BadAccess
Display	*client2;

	XGrabKey(Dsp, keycode, modifiers, grab_window, owner_events, pointer_mode,
		keyboard_mode);
	if (isdeleted()) {
		delete("Could not set up initial grab");
		return;
	}

	if ((client2 = opendisplay()) == NULL)
		return;

	display = client2;
	XCALL;

	if (geterr() == BadAccess)
		CHECK;
	else
		FAIL;

	CHECKPASS(1);
>>ASSERTION Bad A
.ER BadWindow

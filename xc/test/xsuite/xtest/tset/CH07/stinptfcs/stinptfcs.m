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
 * $XConsortium: stinptfcs.m,v 1.10 94/04/17 21:06:51 rws Exp $
 */
>>TITLE XSetInputFocus CH07
void

Display	*display = Dsp;
Window	focus;
int 	revert_to = RevertToNone;
Time	thetime = CurrentTime;
>>EXTERN
/*
 * NOTE: This test should not be run with a window manager as some
 * window managers interfere with the test.
 */
>>SET startup focusstartup
>>SET cleanup focuscleanup
>>ASSERTION Good A
A call to xname changes the input focus window to that specified
in the
.A focus
argument.
>>STRATEGY
Create window.
Set input focus with that window.
Verify that input focus has been set with XGetInputFocus.
>>CODE
Window	newfocus;
int 	newrevert;

	focus = defwin(display);

	XCALL;

	XGetInputFocus(display, &newfocus, &newrevert);
	if (newfocus != focus) {
		report("Focus window was 0x%x, expecting 0x%x", newfocus, focus);
		FAIL;
	} else
		CHECK;

	CHECKPASS(1);
>>ASSERTION Good B 3
When the
.A focus
argument is
.S None ,
then all keyboard events are discarded until a new focus window is set.
>>STRATEGY
If extension available:
  Create a window and select KeyPress and KeyRelease events.
  Call xname with focus = None.
  Warp pointer into window and simulate keypress using extension.
  Ensure no event received.
  Call xname with focus = window.
  Simulate key release.
  Ensure event now received.
else
  report untested.
>>CODE
Window	win;
XEvent	ev;
int	n;
long	mask;

	if (noext(0))
		return;
	else
		CHECK;

	win = defwin(display);
	XSelectInput(display, win, mask = KeyPressMask|KeyReleaseMask);
	focus = None;
	XCALL;
	(void) warppointer(display, win, 2, 2);
	XSync(display, True); /* clear out event queue */
	keypress(display, getkeycode(display));
	if (n=getevent(display, &ev)) /* assignment intentional */ {
		report("Got %d events instead of 0, first was type %s.",
			n, eventname(ev.type));
		FAIL;
	} else
		CHECK;
	focus = win;
	XCALL;
	XSync(display, True); /* clear out event queue */
	relalldev();
	if (!XCheckWindowEvent(display, win, mask, &ev)) {
		report("Normal event processing not restored.");
		FAIL;
	} else
		CHECK;

	CHECKPASS(3);

>>ASSERTION Good B 3
When the
.A focus
argument is a window, then keyboard events that would normally be reported
to the focus window or one of its inferiors are reported as usual and all other
keyboard events are reported relative to the focus window.
>>STRATEGY
If extension available:
  Create a window tree and select KeyPress and KeyRelease events on all.
  Call xname with focus = window in tree with child (child2, with child
    grandchild).
  Warp pointer into all windows, and root, and simulate keypress/release using
    extension in each.
  Ensure event.xany.window is focus (child2) in all cases except grandchild,
    when it should be grandchild.
  Release any remaining keys.
else
  report untested.
>>EXTERN

static char	*WindowTree[]= {
	"toplevel",
		"child1 toplevel (10,10) 30x30",
		"child2 toplevel (50,50) 30x30",
			"grandchild child2 (2,2) 20x20",
};

static int NWindowTree = NELEM(WindowTree);

>>CODE
int	keycode;
XEvent	ev;
int	n;
long	mask;
Window	parent,child2,gchild;
Window	windows[6]; /* root + 4 + None stopper */
Window	root;
Window	*wp;
struct buildtree *tree;
char	*wname;
char	*evwname;

	if (noext(0))
		return;
	else
		CHECK;
	wp = windows;
	*wp++ = root = DRW(display);
	*wp++ = parent = defwin(display);
	tree = buildtree(display, parent, WindowTree, NWindowTree);
	*wp++ = btntow(tree, "child1");
	*wp++ = child2 = btntow(tree, "child2");
	*wp++ = gchild = btntow(tree, "grandchild");
	*wp = None;

	keycode=getkeycode(display);
	mask = KeyPressMask|KeyReleaseMask;
	for(wp=windows; *wp != None; wp++)
		XSelectInput(display, *wp, mask);

	focus = child2;
	XCALL;

	for(wp=windows; *wp != None; wp++) { /* around 5 times */
		(void)warppointer(display, *wp, 0,0);
		/* use 0,0 as window making stuff keeps away from there on
		 * root. All of our tree windows are not at 0,0 either
		 */
		XSync(display, True); /* clear out event queue */
		keypress(display, keycode);
		relalldev();
		if (!(wname = btwton(tree,*wp)))
			wname = (*wp == root) ? "ROOT" : "<Unknown>";
		if (!(n=XCheckMaskEvent(display, mask, &ev))) {
			report("No event received after keypress/release in window %s.", (*wp==focus)?"focus":wname);
			FAIL;
		} else
			CHECK;
		if (n && !(evwname = btwton(tree,ev.xany.window)))
			evwname = (ev.xany.window == root) ? "ROOT" :
				((ev.xany.window == None) ? "None" : "<Unknown>");
		if (*wp == focus || *wp == gchild) {
			if (n && ev.xany.window != *wp) {
				report("Event window was %s instead of %s for focus window or child.", evwname, wname);
				FAIL;
			} else
				CHECK;
		} else {
			if (n && ev.xany.window != focus) {
				report("Event window was %s instead of focus window.", evwname);
				FAIL;
			} else
				CHECK;
		}
	}

	CHECKPASS(1+2*5);

>>ASSERTION Good B 3
When the
.A focus
argument is
.S PointerRoot ,
then the focus window is taken to be the root window of the screen the pointer
is on at each keyboard event.
>>STRATEGY
If extension available:
  Create a toplevel window and select KeyPress and KeyRelease events.
  Select KeyPress and KeyRelease events in root window.
  Call xname with focus = PointerRoot.
  Warp pointer into window and simulate keypress using extension.
  Ensure event received and that event.xany.window = window.
  Warp pointer into root window.
  Simulate key release.
  Ensure event received and that event.xany.window = root (focus).
  If more than one screen:
    Select KeyPress and KeyRelease events in root window of alternate screen.
    Warp pointer into root window of alternate screen.
    Simulate KeyPress/KeyRelease.
    Ensure event received and that event.xany.window = altroot (focus) and
      event.xkey.same_screen is True and event.xkey.root is altroot.
  else
    Issue incomplete testing message and report untested.
else
  report untested.
>>CODE
Window	win, root, altroot;
int	keycode;
XEvent	ev;
long	mask;

	if (noext(0))
		return;
	else
		CHECK;

	win = defwin(display);
	XSelectInput(display, win, mask = KeyPressMask|KeyReleaseMask);
	XSelectInput(display, root = DRW(display), mask);
	trace("Test with toplevel window and PointerRoot.");
	focus = PointerRoot;
	XCALL;
	(void) warppointer(display, win, 2, 2);
	XSync(display, True); /* clear out event queue */
	keypress(display, keycode=getkeycode(display));
	if (!getevent(display, &ev)) {
		report("No event received.");
		FAIL;
	} else if (ev.type != KeyPress && ev.type != KeyRelease) {
		report("First event was of unexpected type: %s.", eventname(ev.type));
		FAIL;
	} else if (ev.xkey.window != win) {
		report("First event had unexpected window: 0x%x instead of 0x%x.",
			(unsigned)ev.xkey.window, (unsigned)win);
		FAIL;
	} else
		CHECK;
	trace("Test with root and PointerRoot.");
	(void) warppointer(display, root, 0,0);
	XSync(display, True); /* clear out event queue */
	relalldev();
	if (!getevent(display, &ev)) {
		report("No event received.");
		FAIL;
	} else if (ev.type != KeyPress && ev.type != KeyRelease) {
		report("First event was of unexpected type: %s.", eventname(ev.type));
		FAIL;
	} else if (ev.xkey.window != root) {
		report("First event had unexpected window: 0x%x instead of 0x%x.",
			(unsigned)ev.xkey.window, (unsigned)root);
		FAIL;
	} else
		CHECK;

	if (config.alt_screen != -1) {
		altroot = RootWindow(display, config.alt_screen);
		trace("Testing with root of alternate screen as source (0x%x) and PointerRoot.",
				(unsigned)altroot);
		XSelectInput(display, altroot, mask);
		(void) warppointer(display, altroot, 0,0);
		XSync(display, True); /* clear out event queue */
		keypress(display, keycode);
		relalldev();

		if (!getevent(display, &ev)) {
			report("No event received.");
			FAIL;
		} else if (ev.type != KeyPress && ev.type != KeyRelease) {
			report("First event was of unexpected type: %s.", eventname(ev.type));
			FAIL;
		} else if (ev.xkey.window != altroot) {
			report("First event had unexpected window: 0x%x instead of focus (altroot) 0x%x.",
				(unsigned)ev.xkey.window, (unsigned)altroot);
			FAIL;
		} else if (!ev.xkey.same_screen) {
			report("same_screen unexpectedly False.");
			FAIL;
		} else if (ev.xkey.root != altroot) {
			report("First event had unexpected root window: 0x%x instead of 0x%x.",
				(unsigned)ev.xkey.root, (unsigned)altroot);
			FAIL;
		} else
			CHECK;
		CHECKPASS(4);
	} else {
		report("Tested as far as possible with just one screen.");
		CHECKUNTESTED(3);
	}

>>ASSERTION Good A
When the focus window later becomes not viewable and
.A revert_to
is
.S RevertToParent ,
then the focus reverts to the closest viewable ancestor of the focus
window, the
.A revert_to
value is changed to
.S RevertToNone
and
.S FocusIn
and
.S FocusOut
events are generated.
>>STRATEGY
Create base window
Create child of this window and set focus argument to it.
Set revert_to argument to RevertToParent.
Call xname.
Enable events on windows.
Unmap focus window.
Verify that focus is the base window.
Verify that revert_to is RevertToNone.
Verify that Focus events are generated.
>>CODE
Window	base;
XFocusInEvent	figood;
XFocusOutEvent	fogood;
XEvent	ev;
Window	newfocus;
int 	newrevert;

	base = defwin(display);
	focus = crechild(display, base, (struct area *)0);
	revert_to = RevertToParent;

	if (isdeleted())
		return;

	XCALL;

	XSelectInput(display, base, FocusChangeMask);
	XSelectInput(display, focus, FocusChangeMask);

	XUnmapWindow(display, focus);
	XGetInputFocus(display, &newfocus, &newrevert);
	if (isdeleted())
		return;

	if (newfocus != base) {
		report("Focus window was 0x%x, expecting 0x%x", newfocus, base);
		FAIL;
	} else
		CHECK;

	if (newrevert != RevertToNone) {
		report("New revert_to value was %s, expecting RevertToNone",
			reverttoname(newrevert));
		FAIL;
	} else
		CHECK;

	/*
	 * Event testing. In this case there should be a FocusOut on the
	 * focus window, followed by a FocusIn on the base window.
	 */
	defsetevent(fogood, display, FocusOut);
	fogood.window = focus;
	fogood.mode = NotifyNormal;
	fogood.detail = NotifyAncestor;

	if (getevent(display, &ev) == 0 || ev.type != FocusOut) {
		report("Was expecting a FocusOut event");
		FAIL;
	} else
		CHECK;

	if (checkevent((XEvent*)&fogood, &ev))
		FAIL;
	else
		CHECK;

	defsetevent(figood, display, FocusIn);
	figood.window = base;
	figood.mode = NotifyNormal;
	figood.detail = NotifyInferior;

	if (getevent(display, &ev) == 0 || ev.type != FocusIn) {
		report("Was expecting a FocusIn event");
		FAIL;
	} else
		CHECK;

	if (checkevent((XEvent*)&figood, &ev))
		FAIL;
	else
		CHECK;

	CHECKPASS(6);
>>ASSERTION Good A
When the focus window later becomes not viewable and
.A revert_to
is
.S RevertToPointerRoot ,
then the focus window reverts to
.S PointerRoot
and
.S FocusIn
and
.S FocusOut
events are generated.
>>STRATEGY
Create base window
Create child of this window and set focus argument to it.
Set revert_to argument to RevertToPointerRoot.
Warp pointer to 0,0 (guaranteed none of our windows are here)
Call xname.
Unmap focus window.
Verify that focus is PointerRoot.
Verify that revert_to is RevertToPointerRoot.
Verify that Focus events are generated.
>>CODE
Window	base;
XFocusInEvent	figood;
XFocusOutEvent	fogood;
XEvent	ev;
Window	newfocus;
int 	newrevert;

	/*
	 * Use a non-default display, because we are setting the event mask
	 * on the root window, which would mess up things for the next tests.
	 */
	display = opendisplay();

	base = defwin(display);
	focus = crechild(display, base, (struct area *)0);
	revert_to = RevertToPointerRoot;

	(void) warppointer(display, DRW(display), 0, 0);

	if (isdeleted())
		return;

	XCALL;

	XSelectInput(display, base, FocusChangeMask);
	XSelectInput(display, focus, FocusChangeMask);
	XSelectInput(display, DRW(display), FocusChangeMask);

	XUnmapWindow(display, focus);
	XGetInputFocus(display, &newfocus, &newrevert);
	if (isdeleted())
		return;

	if (newfocus != PointerRoot) {
		report("Focus window was 0x%x, expecting 0x%x", newfocus, PointerRoot);
		FAIL;
	} else
		CHECK;

	if (newrevert != RevertToPointerRoot) {
		report("New revert_to value was %s, expecting RevertToPointerRoot",
			reverttoname(newrevert));
		FAIL;
	} else
		CHECK;

	/* FocusOut from focus window */
	defsetevent(fogood, display, FocusOut);
	fogood.window = focus;
	fogood.mode = NotifyNormal;
	fogood.detail = NotifyNonlinear;

	if (getevent(display, &ev) == 0 || ev.type != FocusOut) {
		report("Was expecting a FocusOut event");
		FAIL;
	} else
		CHECK;

	if (checkevent((XEvent*)&fogood, &ev))
		FAIL;
	else
		CHECK;

	/* FocusOut on the base window */
	fogood.window = base;
	fogood.detail = NotifyNonlinearVirtual;

	if (getevent(display, &ev) == 0 || ev.type != FocusOut) {
		report("Was expecting a FocusOut event");
		FAIL;
	} else
		CHECK;

	if (checkevent((XEvent*)&fogood, &ev))
		FAIL;
	else
		CHECK;
	
	/* FocusOut on the root window */
	fogood.window = DRW(display);

	if (getevent(display, &ev) == 0 || ev.type != FocusOut) {
		report("Was expecting a FocusOut event");
		FAIL;
	} else
		CHECK;

	if (checkevent((XEvent*)&fogood, &ev))
		FAIL;
	else
		CHECK;
	
	/* FocusIn on the root window */
	defsetevent(figood, display, FocusIn);
	figood.window = DRW(display);
	figood.mode = NotifyNormal;
	figood.detail = NotifyPointerRoot;

	if (getevent(display, &ev) == 0 || ev.type != FocusIn) {
		report("Was expecting a FocusIn event");
		FAIL;
	} else
		CHECK;

	if (checkevent((XEvent*)&figood, &ev))
		FAIL;
	else
		CHECK;

	figood.window = DRW(display);
	figood.detail = NotifyPointer;

	/* FocusIn for the pointer */
	if (getevent(display, &ev) == 0 || ev.type != FocusIn) {
		report("Was expecting a FocusIn event");
		FAIL;
	} else
		CHECK;

	if (checkevent((XEvent*)&figood, &ev))
		FAIL;
	else
		CHECK;
	
	CHECKPASS(12);

>>ASSERTION Good A
When the focus window later becomes not viewable and
.A revert_to
is
.S RevertToNone ,
then the focus window reverts to
.S None
and
.S FocusIn
and
.S FocusOut
events are generated.
>>STRATEGY
Create base window
Create child of this window and set focus argument to it.
Set revert_to argument to RevertToNone.
Call xname.
Unmap focus window.
Verify that focus is None
Verify that revert_to is RevertToNone.
Verify that Focus events are generated.
>>CODE
Window	base;
XFocusInEvent	figood;
XFocusOutEvent	fogood;
XEvent	ev;
Window	newfocus;
int 	newrevert;

	/*
	 * Use a non-default display, because we are setting the event mask
	 * on the root window, which would mess up things for the next tests.
	 */
	display = opendisplay();

	base = defwin(display);
	focus = crechild(display, base, (struct area *)0);
	revert_to = RevertToNone;

	(void) warppointer(display, DRW(display), 0, 0);

	if (isdeleted())
		return;

	XCALL;

	XSelectInput(display, base, FocusChangeMask);
	XSelectInput(display, focus, FocusChangeMask);
	XSelectInput(display, DRW(display), FocusChangeMask);

	XUnmapWindow(display, focus);
	XGetInputFocus(display, &newfocus, &newrevert);
	if (isdeleted())
		return;

	if (newfocus != None) {
		report("Focus window was 0x%x, expecting None", newfocus);
		FAIL;
	} else
		CHECK;

	if (newrevert != RevertToNone) {
		report("New revert_to value was %s, expecting RevertToNone",
			reverttoname(newrevert));
		FAIL;
	} else
		CHECK;

	/* FocusOut from focus window */
	defsetevent(fogood, display, FocusOut);
	fogood.window = focus;
	fogood.mode = NotifyNormal;
	fogood.detail = NotifyNonlinear;

	if (getevent(display, &ev) == 0 || ev.type != FocusOut) {
		report("Was expecting a FocusOut event");
		FAIL;
	} else
		CHECK;

	if (checkevent((XEvent*)&fogood, &ev))
		FAIL;
	else
		CHECK;

	/* FocusOut on the base window */
	fogood.window = base;
	fogood.detail = NotifyNonlinearVirtual;

	if (getevent(display, &ev) == 0 || ev.type != FocusOut) {
		report("Was expecting a FocusOut event");
		FAIL;
	} else
		CHECK;

	if (checkevent((XEvent*)&fogood, &ev))
		FAIL;
	else
		CHECK;
	
	/* FocusOut on the root window */
	fogood.window = DRW(display);

	if (getevent(display, &ev) == 0 || ev.type != FocusOut) {
		report("Was expecting a FocusOut event");
		FAIL;
	} else
		CHECK;

	if (checkevent((XEvent*)&fogood, &ev))
		FAIL;
	else
		CHECK;
	
	/* FocusIn on the root window */
	defsetevent(figood, display, FocusIn);
	figood.window = DRW(display);
	figood.mode = NotifyNormal;
	figood.detail = NotifyDetailNone;

	if (getevent(display, &ev) == 0 || ev.type != FocusIn) {
		report("Was expecting a FocusIn event");
		FAIL;
	} else
		CHECK;

	if (checkevent((XEvent*)&figood, &ev))
		FAIL;
	else
		CHECK;

	CHECKPASS(10);

>>ASSERTION Good A
When the specified time is earlier than the current
last-focus-change time or is later than the current X server time,
then a call to xname has no effect.
>>STRATEGY
Create window.
Get current X server time with gettime().
Set focus to None using this time.

Attempt to set the focus window with a time less than the previous time.
Verify that focus is still None.

Get current time again.
Add amount to get time in the future.
Attempt to set the focus window with this time.
Verify that focus is still None.
>>CODE
Window	win;
Window	newfocus;
int 	newrevert;

	win = defwin(display);

	thetime = gettime(display);
	focus = None;

	XCALL;

	thetime -= 12;
	focus = win;
	XCALL;

	XGetInputFocus(display, &newfocus, &newrevert);
	if (newfocus == None)
		CHECK;
	else {
		report("Focus was changed when time was earlier than last-focus-change time");
		FAIL;
	}

	thetime = gettime(display);
	thetime += ((config.speedfactor+1) * 1000000);

	XCALL;

	XGetInputFocus(display, &newfocus, &newrevert);
	if (newfocus == None)
		CHECK;
	else {
		report("Focus was changed when time was later than current X server time");
		FAIL;
	}

	CHECKPASS(2);
>>ASSERTION Good A
A successful call to xname sets the last-focus-change time to
the specified time with CurrentTime being
replaced by the current X server time.
>>STRATEGY
Create toplevel window.
Call xname with time = gettime(display) and focus = window.
Call XGetInputFocus and verify that focus_return is window.
Attempt xname at time just before time with focus = root.
Check focus is still window.
Attempt xname at time equal to time and focus = root.
Check focus is now root.

Get time before with gettime(display).
Call xname with time = CurrentTime and focus = window.
Call XGetInputFocus and verify that focus_return is window.
Attempt xname at time before with focus = root.
Check focus is still window.
Attempt xname at CurrentTime and focus = root.
Check focus is now root.
>>CODE
Time	t1,t2;
Window	win;
Window	focus_return;
int	junk;

	win = defwin(display);
	XSync(display, True);
	t1 = gettime(display);
	if (t1 == CurrentTime) {
		delete("Could not get server time.");
		return;
	} else
		CHECK;

	thetime = t1;
	focus = win;
	XCALL;

	XGetInputFocus(display, &focus_return, &junk);
	if (focus_return != win) {
		report("Failed to change focus with time = 0x%lx.", (unsigned long)thetime);
		FAIL;
	} else
		CHECK;
	trace("Focus set at time 0x%lx.",(unsigned long)thetime);
	thetime--;
	focus = DRW(display);
	XCALL;
	XGetInputFocus(display, &focus_return, &junk);
	if (focus_return != win) {
		report("Last focus change time set earlier than specified time.");
		FAIL;
	} else
		CHECK;
	thetime = t1;
	XCALL;
	XGetInputFocus(display, &focus_return, &junk);
	if (focus_return != focus) {
		report("Last focus change time set later than specified time.");
		FAIL;
	} else
		CHECK;

	/* last despairing attempt */
	thetime = CurrentTime;
	XCALL;
	XGetInputFocus(display, &focus_return, &junk);
	if (focus_return != focus) {
		delete("Cannot restore focus to perform CurrentTime tests.");
		return;
	} else
		CHECK;

	XSync(display, True);
	t1 = gettime(display);
	if (t1 == CurrentTime) {
		delete("Could not get earlier server time.");
		return;
	} else
		CHECK;

	thetime = CurrentTime;
	focus = win;
	XCALL;

	XGetInputFocus(display, &focus_return, &junk);
	if (focus_return != focus) {
		delete("Failed to change focus at CurrentTime.");
		return;
	} else
		CHECK;
	/* now set up OK */
	t2 = gettime(display);
	if (t2 == CurrentTime) {
		delete("Could not get later server time.");
		return;
	} else
		CHECK;

	trace("Focus changed at time between 0x%lx and 0x%lx (diff = %d).",t1,t2,t2-t1);
	thetime = t1;
	focus = DRW(display);
	XCALL;
	XGetInputFocus(display, &focus_return, &junk);
	if (focus_return != win) {
		report("Last focus change time set earlier than 0x%lx.",t1);
		FAIL;
	} else
		CHECK;
	thetime = t2;
	XCALL;
	XGetInputFocus(display, &focus_return, &junk);
	if (focus_return != focus) {
		report("Last focus change time set later than 0x%lx.", t2);
		FAIL;
	} else
		CHECK;

	CHECKPASS(10);
	
>>ASSERTION Good A
When the focus changes, then
.S FocusIn
and
.S FocusOut
events are generated.
>>STRATEGY
Create base window.
Create two subwindows on base.
Set focus to first subwindow.
Enable events on all three windows.
Call xname to change focus to second subwindow.
Verify that focus events are generated.
>>CODE
Window	base;
Window	ch1;
Window	ch2;
struct	area	area;
XFocusInEvent	figood;
XFocusOutEvent	fogood;
XEvent	ev;

	base = defwin(display);
	setarea(&area, 0, 0, 2, 2);
	ch1 = crechild(display, base, &area);
	setarea(&area, 20, 20, 2, 2);
	ch2 = crechild(display, base, &area);

	focus = ch1;
	XCALL;

	XSelectInput(display, ch1, FocusChangeMask);
	XSelectInput(display, ch2, FocusChangeMask);
	XSelectInput(display, base, FocusChangeMask);

	focus = ch2;
	XCALL;

	/* FocusOut from old focus window, ch1 */
	defsetevent(fogood, display, FocusOut);
	fogood.window = ch1;
	fogood.mode = NotifyNormal;
	fogood.detail = NotifyNonlinear;

	if (getevent(display, &ev) == 0 || ev.type != FocusOut) {
		report("Was expecting a FocusOut event");
		FAIL;
	} else
		CHECK;

	if (checkevent((XEvent*)&fogood, &ev))
		FAIL;
	else
		CHECK;

	/* FocusIn on the new focus window, ch2 */
	defsetevent(figood, display, FocusIn);
	figood.window = ch2;
	figood.mode = NotifyNormal;
	figood.detail = NotifyNonlinear;

	if (getevent(display, &ev) == 0 || ev.type != FocusIn) {
		report("Was expecting a FocusIn event");
		FAIL;
	} else
		CHECK;

	if (checkevent((XEvent*)&figood, &ev))
		FAIL;
	else
		CHECK;

	CHECKPASS(4);

>>ASSERTION Good A
When the specified focus window is not viewable, then a
.S BadMatch
error occurs.
>>STRATEGY
Create unmapped window.
Attempt to set focus to it.
Verify that a BadMatch error occurs.
>>CODE BadMatch
Window	base;

	base = defwin(display);
	focus = creunmapchild(display, base, (struct area *)0);

	XCALL;

	if (geterr() == BadMatch)
		PASS;
	else
		FAIL;	/* Already done */
>>ASSERTION Good A
.ER BadValue revert_to RevertToParent RevertToPointerRoot RevertToNone
>>ASSERTION Good A
.ER BadWindow PointerRoot None

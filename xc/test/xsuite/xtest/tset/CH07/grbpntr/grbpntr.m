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
 * $XConsortium: grbpntr.m,v 1.13 94/04/17 21:06:24 rws Exp $
 */
>>TITLE XGrabPointer CH07
int

Display	*display = Dsp;
Window	grab_window = defwin(display);
Bool	owner_events = True;
unsigned int 	event_mask = 0;
int 	pointer_mode = GrabModeAsync;
int 	keyboard_mode = GrabModeAsync;
Window	confine_to = None;
Cursor	cursor = None;
Time	thetime = CurrentTime;
>>EXTERN

/*
 * For all these tests note that the grab_window is automatically destroyed
 * at the end of the test, and therefore the grab is released.
 */

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

	s = XQueryPointer(disp, win, &wtmp, &child, &itmp, &itmp, &itmp, &itmp
		, &uitmp);

	if (!s)
		delete("Could not get pointer window");

	return(child);
}

>>ASSERTION Good A
A successful call to xname actively grabs control of the pointer and returns
.S GrabSuccess .
>>SET return-value GrabSuccess
>>STRATEGY
Call xname.
Verify that it returns GrabSuccess.
>>CODE
int 	ret;

	grab_window = defwin(display);
	ret = XCALL;

	if (ret != GrabSuccess) {
		report("GrabSuccess was not returned");
		FAIL;
	} else
		CHECK;

	CHECKPASS(1);
>>ASSERTION Good A
After a call to xname is made by a client,
pointer events are reported only to that client.
>>STRATEGY
Create second client.
Create grab window.
Select pointer events for both clients.
Grab the pointer.
Warp pointer.
Verify that the event is only reported to grabbing client.
>>CODE
Display	*client2;
XEvent	ev;

	client2 = opendisplay();

	grab_window = defwin(display);
	(void) warppointer(display, grab_window, 0, 0);

	XSelectInput(display, grab_window, PointerMotionMask);
	XSelectInput(client2, grab_window, PointerMotionMask);
	XSync(client2, False);

	XCALL;

	(void) warppointer(display, grab_window, 1, 1);

	if (getevent(display, &ev) == 0 || ev.type != MotionNotify) {
		report("No pointer event occurred on the grabbing client");
		FAIL;
	} else
		CHECK;
	if (getevent(client2, &ev) != 0) {
		report("Event was reported on the non-grabbing client");
		FAIL;
	} else
		CHECK;

	CHECKPASS(2);
>>ASSERTION Good A
A call to xname
overrides any active pointer grab by this client.
>>STRATEGY
Create two windows.
Call xname with confine_to as one of the windows.
Check that pointer is within that window.
Call xname with confine_to as the other window.
Verify that pointer is within the other window.
>>CODE
Window	w1, w2;

	/* Move pointer, mainly so that its position will be restored */
	(void) warppointer(display, DRW(display), 0, 0);

	w1 = defwin(display);
	w2 = defwin(display);

	grab_window = w1;
	confine_to = w1;

	XCALL;

	if (getpointerwin(display, DRW(display)) == confine_to)
		CHECK;
	else {
		delete("Could not set up first grab properly");
		return;
	}

	confine_to = w2;
	XCALL;

	if (getpointerwin(display, DRW(display)) == confine_to)
		CHECK;
	else {
		report("A second grab did not override the first");
		FAIL;
	}

	CHECKPASS(2);
>>ASSERTION Good A
When
.A owner_events
is
.S False ,
then all generated pointer events that are selected by the
.A event_mask
are reported with respect to the
.A grab_window .
>>STRATEGY
Set owner_events to False.
Create grab window.
Create other window.
Set event-mask to select pointer events.
Grab pointer.
Warp pointer to other window.
Verify that events are reported with respect to the grab window.
>>CODE
Window	win;
XEvent	ev;
XMotionEvent	*mp;

	owner_events = False;
	grab_window = defwin(display);
	event_mask = PointerMotionMask;

	win = defwin(display);
	(void) warppointer(display, DRW(display), 0, 0);

	XCALL;
	(void) warppointer(display, win, 0, 0);
	(void) warppointer(display, win, 1, 1);

	if (!XCheckMaskEvent(display, (long)event_mask, &ev)) {
		delete("No pointer event was received");
		return;
	} else
		CHECK;

	mp = (XMotionEvent*)&ev;
	if (mp->window == grab_window)
		CHECK;
	else if (mp->window == win) {
		report("Event was unexpectedly received on the pointer window");
		FAIL;
	} else {
		report("Event was not received on the grab_window");
		FAIL;
	}

	CHECKPASS(2);
>>ASSERTION Good A
When
.A owner_events
is
.S True ,
and a pointer event is generated that would normally be reported to the client,
then it is reported on the window that it would normally be reported on.
>>STRATEGY
Create window for grab window.
Create window2.
Set owner_events to True.
Select pointer events on window2.
Grab pointer.
Warp pointer in window2.
Verify that pointer events are generated on window2.
>>CODE
Window	window2;
XEvent	ev;
XMotionEvent	*mp;

	grab_window = defwin(display);
	window2 = defwin(display);

	owner_events = True;
	XSelectInput(display, window2, PointerMotionMask);
	(void) warppointer(display, window2, 0, 0);

	XCALL;

	(void) warppointer(display, window2, 1, 0);

	if (!XCheckMaskEvent(display, PointerMotionMask, &ev)) {
		delete("No pointer event was received");
		return;
	} else
		CHECK;

	mp = (XMotionEvent*)&ev;
	if (mp->window == window2)
		CHECK;
	else if (mp->window == grab_window) {
		report("Event was unexpectedly received on the grab window");
		FAIL;
	} else {
		report("Event was not received on the normal window");
		FAIL;
	}

	CHECKPASS(2);

>>ASSERTION Good A
When
.A owner_events
is
.S True ,
and a pointer event is generated that would not normally be reported
to the client,
and it is selected by
.A event_mask ,
then it is reported on the
.A grab_window .
>>STRATEGY
Create window for grab window.
Create window2.
Set owner_events to True.
Select pointer events in event_mask.
Do not select pointer events on window2.
Grab pointer.
Warp pointer in window2.
Verify that pointer events are generated on the grab window.
>>CODE
Window	window2;
XEvent	ev;
XMotionEvent	*mp;

	grab_window = defwin(display);
	window2 = defwin(display);

	owner_events = True;
	event_mask = PointerMotionMask;
	(void) warppointer(display, window2, 0, 0);

	XCALL;

	(void) warppointer(display, window2, 1, 0);

	if (!XCheckMaskEvent(display, (long)event_mask, &ev)) {
		delete("No pointer event was received");
		return;
	} else
		CHECK;

	mp = (XMotionEvent*)&ev;
	if (mp->window == grab_window)
		CHECK;
	else if (mp->window == window2) {
		report("Event was unexpectedly received on the source window");
		FAIL;
	} else {
		report("Event was not received on the grab window");
		FAIL;
	}

	CHECKPASS(2);

>>ASSERTION Good A
When
.A pointer_mode
is
.S GrabModeAsync ,
then pointer event processing continues normally.
>>STRATEGY
Grab pointer with pointer_mode GrabModeAsync.
Warp pointer.
Verify that pointer events are received.
>>CODE
XEvent	ev;

	owner_events = False;
	grab_window = defwin(display);
	event_mask = PointerMotionMask;
	pointer_mode = GrabModeAsync;
	(void) warppointer(display, grab_window, 0, 0);

	XCALL;

	(void) warppointer(display, grab_window, 1, 1);

	if (!XCheckMaskEvent(display, (long)event_mask, &ev)) {
		delete("No pointer event was received");
		return;
	} else
		CHECK;

	CHECKPASS(1);
>>ASSERTION Good A
When
.A pointer_mode
is
.S GrabModeAsync
and the pointer is currently frozen by this client, then
the processing of events for the pointer is resumed.
>>STRATEGY
Freeze pointer by grabbing keyboard with pointer_mode GrabModeSync.
Warp pointer.
Verify that no pointer events received yet.
Grab pointer with GrabModeAsync.
Verify that the pointer event is now released.
>>CODE
XEvent	ev;

	owner_events = False;
	grab_window = defwin(display);
	event_mask = PointerMotionMask;
	pointer_mode = GrabModeAsync;
	(void) warppointer(display, grab_window, 0, 0);

	/*
	 * Freeze the pointer by grabbing the keyboard.
	 */
	XGrabKeyboard(display, grab_window, False, GrabModeSync, GrabModeAsync, thetime);
	(void) warppointer(display, grab_window, 1, 1);
	if (XCheckMaskEvent(display, (long)event_mask, &ev)) {
		delete("Pointer event was received while frozen");
		return;
	} else
		CHECK;

	XCALL;

	if (!XCheckMaskEvent(display, (long)event_mask, &ev)) {
		report("Normal pointer processing was not restored");
		FAIL;
	} else
		CHECK;

	CHECKPASS(2);
>>ASSERTION Good A
When
.A pointer_mode
is
.S GrabModeSync ,
then the state of the pointer, as seen by client applications,
appears to freeze, and no further pointer events are generated
until the grabbing client calls
.F XAllowEvents
or until the pointer grab is released.
>>STRATEGY
Grab pointer with GrabModeSync.
Warp pointer.
Verify that no pointer events are received.
>>CODE
XEvent	ev;

	owner_events = False;
	grab_window = defwin(display);
	event_mask = PointerMotionMask;
	pointer_mode = GrabModeSync;
	(void) warppointer(display, grab_window, 0, 0);

	XCALL;

	(void) warppointer(display, grab_window, 1, 1);
	if (XCheckMaskEvent(display, (long)event_mask, &ev)) {
		delete("Pointer event was received while frozen");
		return;
	} else
		CHECK;

	CHECKPASS(1);
>>ASSERTION Good A
When the pointer is frozen, then the
actual pointer changes are not lost and are processed after the grab
is released or the client calls
.F XAllowEvents .
>>STRATEGY
Grab pointer with GrabModeSync.
Warp pointer.
Verify that no pointer events are received.
Release pointer.
Verify that pointer events are now received.
>>CODE
XEvent	ev;

	owner_events = False;
	grab_window = defwin(display);
	event_mask = PointerMotionMask;
	pointer_mode = GrabModeSync;
	(void) warppointer(display, grab_window, 0, 0);
	XSelectInput(display, grab_window, PointerMotionMask);

	XCALL;

	(void) warppointer(display, grab_window, 1, 1);
	if (XCheckMaskEvent(display, (long)event_mask, &ev)) {
		delete("Pointer event was received while frozen");
		return;
	} else
		CHECK;

	XUngrabPointer(display, CurrentTime);
	XSync(display, False);

	if (XCheckMaskEvent(display, (long)event_mask, &ev) && ev.type == MotionNotify)
		CHECK;
	else {
		report("Pointer event was not saved while pointer was frozen");
		FAIL;
	}

	CHECKPASS(2);
>>ASSERTION Good B 1
When
.A keyboard_mode
is
.S GrabModeAsync ,
then keyboard event processing is unaffected by activation of the grab.
>>STRATEGY
If extension available:
  Grab pointer with keyboard_mode = GrabModeAsync.
  Check keyboard not frozen.
else
  Report untested.

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

	XSelectInput(display, grab_window, KeyPressMask);
	XSync(display, True); /* Flush previous events */
	key = getkeycode(display);

	/*
	 * Try to provoke a keypress on grab_window.
	 */
	(void) warppointer(display, grab_window, 1, 1);
	keypress(display, key);
	if (XCheckMaskEvent(display, KeyPressMask, &ev))
		res = False;
	else
		res = True;
	keyrel(display, key);

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

	relalldev();
	CHECKPASS(1);
>>ASSERTION Good B 1
When
.A keyboard_mode
is
.S GrabModeSync ,
then the state of the keyboard, as seen by
client applications,
appears to freeze, and no further keyboard events are generated
until the grabbing client calls
.S XAllowEvents
or until the pointer grab is released.
>>STRATEGY
If extensions available:
  Call xname with keyboard_mode GrabModeSync.
  Verify that keyboard is frozen.
  Release grab.
  Verify that keyboard is not frozen.
>>CODE

	if (noext(0))
		return;

	keyboard_mode = GrabModeSync;
	XCALL;

	if (iskfrozen())
		CHECK;
	else {
		report("Keyboard was not frozen with keyboard_mode GrabModeSync");
		FAIL;
	}

	XUngrabKeyboard(display, thetime);
	XAllowEvents(display, AsyncKeyboard, thetime);
	if (!iskfrozen())
		CHECK;
	else {
		report("Could not unfreeze keyboard from keyboard_mode GrabModeSync");
		FAIL;
	}

	relalldev();
	CHECKPASS(2);
>>ASSERTION Good B 3
>># When the pointer is frozen, then the
When the keyboard is frozen, then the
actual keyboard changes are not lost and are processed after the grab
is released or the client calls
.F XAllowEvents .
>>STRATEGY
If extensions available:
  Enable key events on grab_window.
  Call xname with keyboard_mode GrabModeSync.
  Press and release key.
  Check no events arrived yet.
  Release grab.
  Verify that KeyPress and KeyRelease events are now received.
else
  Report untested.
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
	XAllowEvents(display, AsyncKeyboard, thetime);
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
>>ASSERTION Good B 3
When
.A cursor
is a valid cursor,
then it is displayed regardless of which window the pointer is in.
>>STRATEGY
If extensions available:
  Make a tree of windows rooted at grab_window, all with default cursor.
  Make a non-overlapping sibling of grab_window, the cursor window.
  Set that window's cursor to be a good, non default, cursor.
  Call xname with cursor = the good cursor.
  Warp pointer to all windows in the tree, root and cursor window, and
    validate that current cursor is equal to that of cursor window
    using extension.
else
  Report untested.
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
>># This assertion uses 'when' inappropriately. Could be split into two to
>># fix this.
When
.A cursor
is
.S None ,
then the normal cursor is displayed when the pointer position is
in the
.A grab_window
and its subwindows, and the
.A grab_window
cursor is displayed when the pointer is in any other window.
>>STRATEGY
If extensions available:
  Make a tree of windows rooted at grab_window (parent), all with default cursor.
  Set grab_window to be one of its own children (child2), which has its own
    child (grandchild).
  Set grab_window's cursor to be a good, non default, cursor.
  Set grab_window's child's (grandchild) cursor to be yet another good,
    non default, cursor.
  Call xname with cursor = None.
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
>>ASSERTION Good A
When
.A confine_to
is not
.S None ,
then the pointer is confined to that window.
>>STRATEGY
Create grab_window.
Create window as the confine_to window.
Grab pointer.
Verify that pointer is now in the confine_to window.
Warp pointer outside confine_to window.
Verify that pointer is still within the confine_to window.
>>CODE

	(void) warppointer(display, DRW(display), 0, 0);

	grab_window = defwin(display);
	confine_to = defwin(display);

	XCALL;

	if (getpointerwin(display, DRW(display)) != confine_to) {
		report("Pointer was not warped to confine_to window");
		FAIL;
	} else
		CHECK;

	/* It is known that no window gets created at (0,0) */
	(void) warppointer(display, DRW(display), 0, 0);

	if (getpointerwin(display, DRW(display)) != confine_to) {
		report("Pointer moved out of confine_to window after warp");
		FAIL;
	} else
		CHECK;

	CHECKPASS(2);
>>ASSERTION Good A
When the pointer is not initially in the
.A confine_to
window, then it is warped automatically to the closest edge
just before the grab activates and enter and leave events
are generated.
>>STRATEGY
Create confine_to window.
Create grab_window.
Create spare window 'win'.
Warp pointer to win.
Enable events on win and grab and confine_to windows.
Grab pointer.
Verify that a leave event is generated on win.
Verify that an enter event is generated on the confine_to window.
>>CODE
Window	win;
XEvent	ev;
XCrossingEvent	*cp;
XEnterWindowEvent	entergood;
XLeaveWindowEvent	leavegood;

	(void) warppointer(Dsp, DRW(Dsp), 0, 0);

	grab_window = defwin(display);
	confine_to = defwin(display);
	win = defwin(display);

	(void) warppointer(display, win, 0, 0);

	XSelectInput(display, win, EnterWindowMask|LeaveWindowMask);
	XSelectInput(display, grab_window, EnterWindowMask|LeaveWindowMask);
	XSelectInput(display, confine_to, EnterWindowMask|LeaveWindowMask);

	XCALL;

	defsetevent(entergood, display, EnterNotify);
	entergood.window = confine_to;
	entergood.root = DRW(display);
	entergood.subwindow = None;
	entergood.time = 0;
	entergood.mode = NotifyNormal;
	entergood.detail = NotifyNonlinear;
	entergood.same_screen = True;
	entergood.focus = False;
	entergood.state = 0;

	defsetevent(leavegood, display, LeaveNotify);
	leavegood.window = win;
	leavegood.root = DRW(display);
	leavegood.subwindow = None;
	leavegood.time = 0;
	leavegood.mode = NotifyNormal;
	leavegood.detail = NotifyNonlinear;
	leavegood.same_screen = True;
	leavegood.focus = False;
	leavegood.state = 0;

	/*
	 * The first two events are the pointer leaving win and warping to
	 * confine_to.  These are normal mode events.
	 */
	if (getevent(display, &ev) == 0 || ev.type != LeaveNotify) {
		report("No leave notify event for win received");
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

	/*
	 * There are then grab mode enter and leave events to the grab window.
	 */
	leavegood.window = win;
	leavegood.mode = NotifyGrab;
	entergood.window = grab_window;
	entergood.mode = NotifyGrab;

	if (getevent(display, &ev) == 0 || ev.type != LeaveNotify) {
		report("No grab-mode leave notify event for win received");
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

	if (getevent(display, &ev) == 0 || ev.type != EnterNotify) {
		report("No grab-mode enter notify event for grab_window received");
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
	
	CHECKPASS(8);
>>ASSERTION Good A
When the
.A confine_to
window is subsequently reconfigured,
then the pointer is warped automatically to keep it within the window.
>>STRATEGY
Create grab_window and confine_to window.
Grab pointer.
Move window so that it does not overlap it's previous position.
Verify that pointer has been warped to the new position.
>>CODE
struct	area	area;

	(void) warppointer(display, DRW(display), 0, 0);

	grab_window = defwin(display);

	/*
	 * This time make confine_to a child of grab_window, also make it small
	 * so that moving it will force the pointer to move.
	 */
	setarea(&area, 1, 1, 2, 2);
	confine_to = crechild(display, grab_window, &area);

	XCALL;

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
>>ASSERTION Good C
If multiple screens are supported:
When the
.A confine_to
window and the
.A grab_window
are not on the same screen, then the pointer is warped to the screen
containing the
.A confine_to
window.
>>STRATEGY
If only one screen
  UNSUPPORTED.
Create grab_window on default screen.
Create confine_to window on alternate screen.
Grab pointer.
Verify that pointer is warped to other screen.
>>CODE
Window	root;
Window	wtmp;
int 	itmp;
unsigned uitmp;
Bool 	s;

	if (config.alt_screen < 0) {
		unsupported("Only one screen supported");
		return;
	}

	(void) warppointer(display, DRW(display), 0, 0);

	grab_window = defwin(display);
	confine_to = defdraw(display, VI_ALT_WIN);

	XCALL;

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
When a successful call to xname is made, then the last-pointer-grab
time is set to the specified time with
.S CurrentTime
replaced by the current X server time.
>>EXTERN

/*
 * Returns True if the pointer is grabbed.  This is not a general purpose
 * routine since it knows about the Grab Pointer args.
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

>>STRATEGY
If extensions are available:
  Call xname with time = gettime(display).
  Check result and that pointer is grabbed.
  Attempt XUngrabPointer at time just before time.
  Check still grabbed.
  Attempt XUngrabPointer at time equal to time.
  Check no longer grabbed.

  Get time before with gettime(display).
  Call xname with time = CurrentTime.
  Check result and that pointer is grabbed.
  Attempt XUngrabPointer at time before.
  Check still grabbed.
  Attempt XUngrabPointer at CurrentTime.
  Check no longer grabbed.
else
  Report untested.
>>CODE
Time	t1,t2;
int	rc;

	if (noext(0))
		return;
	else
		CHECK;
	XSync(display, True);
	t1 = gettime(display);
	if (t1 == CurrentTime) {
		delete("Could not get server time.");
		return;
	} else
		CHECK;

	thetime = t1;
	rc = XCALL;

	if (!pgrabbed()) {
		delete("Failed to grab and result was %s.", grabreplyname(rc));
		return;
	} else if (rc != GrabSuccess) {
		report("Grab succeeded but result was %s.", grabreplyname(rc));
		FAIL;
	} else
		CHECK;
	/* now set up OK */
	trace("Grabbed at time 0x%lx.",(unsigned long)thetime);
	XUngrabPointer(display, thetime - 1);
	if (!pgrabbed()) {
		report("Last pointer grab time set earlier than specified time.");
		FAIL;
	} else
		CHECK;
	XUngrabPointer(display, thetime);
	if (pgrabbed()) {
		report("Last pointer grab time set later than specified time.");
		FAIL;
	} else
		CHECK;

	XUngrabPointer(display, CurrentTime); /* last despairing attempt */
	if (pgrabbed()) {
		delete("Cannot release pointer grab to perform CurrentTime tests.");
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
	rc = XCALL;

	if (!pgrabbed()) {
		delete("Failed to grab at CurrentTime and result was %s.", grabreplyname(rc));
		return;
	} else if (rc != GrabSuccess) {
		report("Grab succeeded at CurrentTime but result was %s.", grabreplyname(rc));
		FAIL;
	} else
		CHECK;
	/* now set up OK */
	t2 = gettime(display);
	if (t2 == CurrentTime) {
		delete("Could not get later server time.");
		return;
	} else
		CHECK;

	trace("Grabbed at time between 0x%lx and 0x%lx (diff = %d).",t1,t2,t2-t1);
	XUngrabPointer(display, t1);
	if (!pgrabbed()) {
		report("Last pointer grab time set earlier than 0x%lx.",t1);
		FAIL;
	} else
		CHECK;
	XUngrabPointer(display, t2);
	if (pgrabbed()) {
		report("Last pointer grab time set later than 0x%lx.", t2);
		FAIL;
	} else
		CHECK;

	CHECKPASS(11);
	
>>ASSERTION Good A
When the
.A grab_window
or
.A confine_to
window becomes not viewable during an active pointer grab,
then the grab is released.
>>STRATEGY
Create new client, client2.
Create grab and confine_to windows.
Create spare window 'win' that does not overlap with the other two.
Enable events on win for client2.
Grab pointer.
Unmap grab_window.
Verify that grab is released by provoking pointer events for client2.

Re-map grab_window.
Grab pointer.
Unmap confine_to window.
Verify that grab is released by provoking pointer events for client2.
>>CODE
Display	*client2;
Window	win;
XEvent	ev;

	(void) warppointer(display, DRW(display), 0, 0);

	client2 = opendisplay();

	grab_window = defwin(display);
	confine_to = defwin(display);
	win = defwin(display);

	XSelectInput(client2, win, PointerMotionMask|EnterWindowMask);
	XSync(client2, True);

	XCALL;

	XUnmapWindow(display, grab_window);

	/*
	 * Warp into win and force all events to be received.
	 * If the grab has been released then this will generate
	 * an event for client2.
	 */
	(void) warppointer(display, win, 0, 0);
	XSync(client2, False);

	if (XCheckWindowEvent(client2, win, PointerMotionMask|EnterWindowMask, &ev))
		CHECK;
	else {
		report("Grab was not released when grab_window was unmapped");
		FAIL;
	}

	/* Clear any extra events */
	XSync(client2, True);

	/* Now repeat for confine_to window. */
	XMapWindow(display, grab_window);

	XCALL;

	XUnmapWindow(display, confine_to);

	/* Warp to win and check for events on client2 */
	(void) warppointer(display, win, 0, 0);
	XSync(client2, False);

	if (XCheckWindowEvent(client2, win, PointerMotionMask|EnterWindowMask, &ev))
		CHECK;
	else {
		report("Grab was not released when confine_to window was unmapped");
		FAIL;
	}

	CHECKPASS(2);

>>ASSERTION Good A
When window reconfiguration causes the
.A confine_to
window to lie completely
outside the boundaries of the root window during an active pointer grab, then
the grab is released.
>>STRATEGY
Create second client.
Create grab and confine_to windows.
Create window, win.
Select events on win for second client.
Grab pointer.
Move confine_to window off the root window.
Verify that grab is released by provoking an event for second client.
>>CODE
Display	*client2;
Window	win;
XEvent	ev;

	(void) warppointer(display, DRW(display), 0, 0);

	client2 = opendisplay();

	grab_window = defwin(display);
	confine_to = defwin(display);
	win = defwin(display);

	XSelectInput(client2, win, PointerMotionMask|EnterWindowMask);
	XSync(client2, True);

	XCALL;

	XMoveWindow(display, confine_to, -9000, -9000);
	(void) warppointer(display, win, 0, 0);
	XSync(client2, False);

	if (isdeleted())
		return;

	if (XCheckWindowEvent(client2, win, PointerMotionMask|EnterWindowMask, &ev))
		CHECK;
	else {
		report("Grab was not released when the confine_to window was placed beyond root window");
		FAIL;
	}

	CHECKPASS(1);
>>ASSERTION Bad A
When the
.A grab_window
is not viewable, then a call to xname fails and returns
.S GrabNotViewable .
>>STRATEGY
Create unmapped grab window.
Attempt to grab pointer.
Verify that xname fails and returns GrabNotViewable.
>>CODE
int 	ret;

	grab_window = defwin(display);
	XUnmapWindow(display, grab_window);

	ret = XCALL;
	if (ret == GrabNotViewable)
		PASS;
	else {
		report("Return value was %s, expecting GrabNotViewable", grabreplyname(ret));
		FAIL;
	}
>>ASSERTION Bad A
When the
.A confine_to
window is not viewable, then a call to xname fails and returns
.S GrabNotViewable .
>>STRATEGY
Create unmapped confine_to window.
Attempt to grab pointer.
Verify that xname fails and returns GrabNotViewable.
>>CODE
int 	ret;

	(void) warppointer(Dsp, DRW(Dsp), 0, 0);

	grab_window = defwin(display);
	confine_to = defwin(display);
	XUnmapWindow(display, confine_to);

	ret = XCALL;
	if (ret == GrabNotViewable)
		PASS;
	else {
		report("Return value was %s, expecting GrabNotViewable", grabreplyname(ret));
		FAIL;
	}
>>ASSERTION Bad A
When the
.A confine_to
window lies completely outside the boundaries of the root
window, then a call to xname fails and returns
.S GrabNotViewable .
>>STRATEGY
Create confine_to window.
Move outside root window.
Attempt to grab pointer.
Verify that xname fails and returns GrabNotViewable.
>>CODE
int 	ret;

	(void) warppointer(Dsp, DRW(Dsp), 0, 0);

	grab_window = defwin(display);
	confine_to = defwin(display);
	XMoveWindow(display, confine_to, -9000, -9000);

	ret = XCALL;
	if (ret == GrabNotViewable)
		PASS;
	else {
		report("Return value was %s, expecting GrabNotViewable", grabreplyname(ret));
		FAIL;
	}
>>ASSERTION Bad A
When the pointer is actively grabbed by some other client, then a call to xname
fails and returns
.S AlreadyGrabbed .
>>STRATEGY
Create client2.
Grab pointer with client2.
Attempt to grab pointer with default client.
Verify that xname fails and returns AlreadyGrabbed.
>>CODE
Display	*client2;
int 	ret;

	if ((client2 = opendisplay()) == 0)
		return;

	grab_window = defwin(Dsp);
	if (isdeleted())
		return;

	display = client2;
	XCALL;

	display = Dsp;
	ret = XCALL;
	if (ret == AlreadyGrabbed)
		CHECK;
	else {
		report("Return value was %s, expecting AlreadyGrabbed", grabreplyname(ret));
		FAIL;
	}

	CHECKPASS(1);
>>ASSERTION Bad A
When the pointer is frozen by an active grab of another client, then
a call to xname fails and returns
.S GrabFrozen .
>>STRATEGY
Create client2.
Grab keyboard and freeze pointer with client2.
Attempt to grab pointer with default client.
Verify that xname fails and returns GrabFrozen.
>>CODE
Display	*client2;
int 	ret;

	if ((client2 = opendisplay()) == 0)
		return;

	grab_window = defwin(display);

	if (XGrabKeyboard(client2, grab_window, True, GrabModeSync, GrabModeAsync, CurrentTime) != GrabSuccess) {
		delete("Could not freeze pointer by grabbing keyboard");
		return;
	}
	
	ret = XCALL;
	if (ret == GrabFrozen)
		CHECK;
	else {
		report("Return value was %s, expecting GrabFrozen", grabreplyname(ret));
		FAIL;
	}

	CHECKPASS(1);
>>ASSERTION Bad A
When the specified time is earlier than the last-pointer-grab time or later
than the current X server time, then a call to xname fails and returns
.S GrabInvalidTime .
>>STRATEGY
Grab pointer with a given time.
Release grab.
Grab pointer with earlier time.
Verify that xname fails and returns GrabInvalidTime.
Get current server time.
Grab pointer with later time.
Verify that xname fails and returns GrabInvalidTime.
>>CODE
int 	ret;

	grab_window = defwin(display);

	/* get time from the server */
	thetime = gettime(display);

	/* This sets the last-pointer-grab time */
	XCALL;
	XUngrabPointer(display, thetime);

	thetime -= 100;
	ret = XCALL;
	if (ret == GrabInvalidTime)
		CHECK;
	else {
		report("Return value was %s, expecting GrabInvalidTime",
			grabreplyname(ret));
		FAIL;
	}
	XUngrabPointer(display, thetime);

	/*
	 * Get current time again and add several minutes to get a time in the
	 * future.
	 */
	thetime = gettime(display);
	thetime += ((config.speedfactor+1) * 1000000);

	ret = XCALL;
	if (ret == GrabInvalidTime)
		CHECK;
	else {
		 report("Returned valued was %s, expecting GrabInvalidTime", grabreplyname(ret));
		 FAIL;
	}

	CHECKPASS(2);
>>ASSERTION Bad A
.ER BadCursor
>>ASSERTION Bad A
.ER BadValue event_mask mask ButtonPressMask ButtonReleaseMask EnterWindowMask LeaveWindowMask PointerMotionMask PointerMotionHintMask Button1MotionMask Button2MotionMask Button3MotionMask Button4MotionMask Button5MotionMask ButtonMotionMask KeymapStateMask
>>ASSERTION Bad A
.ER BadValue pointer_mode GrabModeSync GrabModeAsync
>>ASSERTION Bad A
.ER BadValue keyboard_mode GrabModeSync GrabModeAsync
>>ASSERTION Bad A
.ER BadValue owner_events True False
>>ASSERTION Bad A
.ER BadWindow

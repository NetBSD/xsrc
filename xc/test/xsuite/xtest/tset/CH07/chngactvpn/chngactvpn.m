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
 * $XConsortium: chngactvpn.m,v 1.8 94/04/17 21:06:10 rws Exp $
 */
>>TITLE XChangeActivePointerGrab CH07
void
xname(display, event_mask, cursor, thetime)
Display	*display = Dsp;
unsigned int 	event_mask = PointerMotionMask;
Cursor	cursor = None;
Time	thetime = CurrentTime;
>>ASSERTION Good A
When the pointer is actively grabbed by the client and the specified time is
not earlier than the last-pointer-grab time nor later than
the current X server time, then a call to xname
changes the specified dynamic parameters.
>>STRATEGY
Create grab_window.
Grab pointer with XGrabPointer using an event_mask of EnterWindowMask.
Check that just enter events are reported.
Call xname to change event_mask to PointerMotionMask.
Verify that now only motion events are reported.
>>CODE
XEvent	ev;
Window	win;

	(void) warppointer(display, DRW(display), 0, 0);

	win = defwin(display);

	XGrabPointer(display, win, False, EnterWindowMask, GrabModeAsync,
		GrabModeAsync, None, None, CurrentTime);

	(void) warppointer(display, win, 0, 0);
	(void) warppointer(display, win, 2, 2);

	/*
	 * This is just a check to see that the grab is behaving as expected.
	 * delete and return if these fail.  We do the same checks
	 * after the parameters are changed.
	 */
	if (XCheckMaskEvent(display, EnterWindowMask, &ev))
		CHECK;
	else {
		delete("No enter event received when checking set up");
		return;
	}
	if (!XCheckMaskEvent(display, PointerMotionMask, &ev))
		CHECK;
	else {
		delete("A pointer motion event was received when checking set up");
		return;
	}

	(void) warppointer(display, DRW(display), 0, 0);

	XCALL;

	XSync(display, True);	/* Flush any remaining event */
	(void) warppointer(display, win, 0, 0);
	(void) warppointer(display, win, 2, 2);
	if (isdeleted())
		return;

	if (!XCheckMaskEvent(display, EnterWindowMask, &ev))
		CHECK;
	else {
		report("Event mask was not changed to disallow enter events");
		FAIL;
	}
	if (XCheckMaskEvent(display, PointerMotionMask, &ev))
		CHECK;
	else {
		report("Event mask was not changed - no pointer motion event received");
		FAIL;
	}

	CHECKPASS(4);
>>ASSERTION Good A
When the specified time is earlier than the last-pointer-grab time
or later than the current X server time, then a call to xname
does not change the parameters.
>>STRATEGY
Create grab_window.
Grab pointer with XGrabPointer using an event_mask of EnterWindowMask.
Use event_mask of PointerMotionMask.
Call xname with time earlier that last pointer grab time.
Verify that enter events are still being reported.

Call xname with time later than current X server time.
Verify that enter events are still being reported.
>>CODE
XEvent	ev;
Window	win;
Time	t;

	(void) warppointer(display, DRW(display), 0, 0);

	win = defwin(display);
	t = gettime(Dsp);

	XGrabPointer(display, win, False, EnterWindowMask, GrabModeAsync,
		GrabModeAsync, None, None, t);

	/* Earlier than last-pointer-grab time */
	thetime = t - 12;
	XCALL;

	XSync(display, True);	/* Flush any event */
	(void) warppointer(display, win, 0, 0);
	(void) warppointer(display, win, 2, 2);
	if (isdeleted())
		return;

	if (XCheckMaskEvent(display, EnterWindowMask, &ev))
		CHECK;
	else {
		report("Event mask was changed for time earlier than last-pointer-grab time");
		FAIL;
	}
	if (!XCheckMaskEvent(display, PointerMotionMask, &ev))
		CHECK;
	else {
		report("Event mask was changed for time earlier than last-pointer-grab time");
		FAIL;
	}

	(void) warppointer(display, DRW(display), 0, 0);

	/* Later than X server time */
	t = gettime(Dsp);
	thetime = t + (config.speedfactor+1)*1000000;
	XCALL;

	XSync(display, True);	/* Flush any event */
	(void) warppointer(display, win, 0, 0);
	(void) warppointer(display, win, 2, 2);
	if (isdeleted())
		return;

	if (XCheckMaskEvent(display, EnterWindowMask, &ev))
		CHECK;
	else {
		report("Event mask was changed for time later than X server time");
		FAIL;
	}
	if (!XCheckMaskEvent(display, PointerMotionMask, &ev))
		CHECK;
	else {
		report("Event mask was changed for time later than X server time");
		FAIL;
	}

	CHECKPASS(4);
>>ASSERTION Good B 3
A call to xname has no effect on the passive parameters of the function
.F XGrabButton .
>>STRATEGY
Create and map a window.
Create and map another window (curwin) and attach a non-default
  cursor (cur) to it.
Select for no events on the window.
Set a passive grab with XGrabButton on that window with owner_events = False,
  cursor = None and event_mask = 0.
Clear event queue.
Warp the pointer into the window and around inside it.
Check no events received.
Call xname with event_mask = PointerMotionMask and cursor = cur.
Clear event queue.
Warp the pointer into the window and around inside it.
Check no events received.
If extension available:
  Activate grab with simulated button press in window.
  Check grab activated.
  Warp pointer around in window.
  Check no events received.
  Check cursor is not equal to that of curwin.
  Call xname with event_mask = PointerMotionMask and cursor = cur.
  Warp pointer around in window.
  Check MotionNotify events received.
  Check cursor is now equal to that of curwin.
  Release all buttons, hence grab.
else
  Issue message and report untested.
>>EXTERN
/* check to see if pointer is grabbed. Use existing win and another client */
/* Depending on owner_events in xname or when setting grab may cause
 * events to appear on grabbing clients event queue. Before checking for
 * events in the queue the grabbing client should have flushed any junk
 * left by this function.
 */
static int pgrabbed(win)
Window win;
{
	PointerPlace *p;
	XEvent ev;
	Display *client2 = opendisplay();

	XSelectInput(client2, win, PointerMotionMask);
	XSync(client2, True);
	p = warppointer(client2, win, 1,1);
	(void) warppointer(client2, win, 10,10);
	(void) warppointer(client2, win, 1,1);
	(void) warppointer(client2, p->oroot, p->ox, p->oy);
	return (!getevent(client2, &ev));
}
>>CODE
Window	win;
Window	curwin;
XEvent	ev;
int	n;
Cursor	cur;

	win = defwin(display);
	curwin = defwin(display);
	cur = makecur2(display);
	XDefineCursor(display, curwin, cur);
	XSelectInput(display, win, 0L);
	XGrabButton(display, AnyButton, 0, win, False, 0, GrabModeAsync,
		GrabModeAsync, None, None);
	if (isdeleted()) {
		delete("Failed to set passive button grab.");
		return;
	} else
		CHECK;
	XSync(display, True);
	(void) warppointer(display, win, 1,1);
	(void) warppointer(display, win, 10,10);
	(void) warppointer(display, win, 1,1);
	if (n=getevent(display, &ev)) {
		delete("Unexpectedly received %d events, first was type %s.",
				n, eventname(ev.type));
		return;
	} else
		CHECK;
	event_mask = PointerMotionMask;
	cursor = cur;
	XCALL;
	XSync(display, True);
	(void) warppointer(display, win, 1,1);
	(void) warppointer(display, win, 10,10);
	(void) warppointer(display, win, 1,1);
	if (n=getevent(display, &ev)) {
		report("event_mask changed passive grab, received %d events, first was type %s.",
				n, eventname(ev.type));
		FAIL;
	} else
		CHECK;
	if (noext(1)) {
		report("Tested as far as possible without activating grab.");
		CHECKUNTESTED(3);
	} else {
		Window	oldf;
		int	oldr;

		XGetInputFocus(display, &oldf, &oldr);
		XSetInputFocus(display, win, RevertToPointerRoot, CurrentTime);
		buttonpress(display, Button1);
		if (!pgrabbed(win)) {
			report("Failed to activate grab.");
			relalldev();
			XSetInputFocus(display, oldf, oldr, CurrentTime);
			return;
		}
		XSync(display, True);
		(void) warppointer(display, win, 1,1);
		(void) warppointer(display, win, 10,10);
		(void) warppointer(display, win, 1,1);
		if (n=getevent(display, &ev)) {
			report("event_mask changed on grab activation, received %d events, first was type %s.",
					n, eventname(ev.type));
			FAIL;
		} else
			CHECK;
		if (spriteiswin(display, curwin)) {
			report("Cursor changed before grab activated.");
			FAIL;
		} else
			CHECK;
		event_mask = PointerMotionMask;
		cursor = cur;
		XCALL;
		XSync(display, True);
		(void) warppointer(display, win, 1,1);
		(void) warppointer(display, win, 10,10);
		(void) warppointer(display, win, 1,1);
		if (!(n=getevent(display, &ev)) || ev.type != MotionNotify) {
			report("event_mask not changed properly for active grab.");
			if (n)
				report("First event of %d was %s instead of MotionNotify.",
					n, eventname(ev.type));
			FAIL;
		} else
			CHECK;
		if (!spriteiswin(display, curwin)) {
			report("Cursor not changed although grab was active.");
			FAIL;
		} else
			CHECK;
		
		relalldev();
		XSetInputFocus(display, oldf, oldr, CurrentTime);
		CHECKPASS(7);
	}

>>ASSERTION Bad A
.ER BadCursor
>>ASSERTION Bad A
.ER BadValue event_mask mask ButtonPressMask ButtonReleaseMask EnterWindowMask LeaveWindowMask PointerMotionMask PointerMotionHintMask Button1MotionMask Button2MotionMask Button3MotionMask Button4MotionMask Button5MotionMask ButtonMotionMask KeymapStateMask

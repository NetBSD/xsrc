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
 * $XConsortium: ungrbpntr.m,v 1.6 94/04/17 21:07:03 rws Exp $
 */
>>TITLE XUngrabPointer CH07
void

Display	*display = Dsp;
Time	thetime = CurrentTime;
>>ASSERTION Good A
When
the client has actively grabbed the pointer with
a call to
.F XGrabPointer ,
.F XGrabButton
or from a normal button press, then a call to xname
releases the pointer and any queued events.
>>STRATEGY
Create a grab window.
Grab pointer with pointer_mode GrabModeSync.
Warp pointer within the grab window.
Verify that no pointer events are received.
Call xname to release pointer.
Verify that pointer events are now received.
>>CODE
XEvent	ev;
Window	win;
unsigned int 	mask = PointerMotionMask;

	win = defwin(display);
	XSelectInput(display, win, PointerMotionMask);
	(void) warppointer(display, win, 0, 0);

	XGrabPointer(display, win, False, mask, GrabModeSync, GrabModeAsync,
		None, None, CurrentTime);

	(void) warppointer(display, win, 1, 1);
	if (XCheckMaskEvent(display, (unsigned long)mask, &ev)) {
		delete("Pointer event was received while frozen");
		return;
	} else
		CHECK;

	XCALL;

	if (XCheckMaskEvent(display, (unsigned long)mask, &ev) && ev.type == MotionNotify)
		CHECK;
	else {
		report("Pointer event was not saved while pointer was frozen");
		FAIL;
	}

	CHECKPASS(2);
>>ASSERTION Good A
When the specified
time is earlier than the last-pointer-grab time or is later than the
current X server time, then a call to xname
does not release the pointer grab.
>>STRATEGY
Create client2.
Create window 'win'.
Enable events for client2 on win.
Create window for use as the grab window.
Grab pointer with a given time.
Call xname with earlier time.
Warp pointer within win.
Verify that no event is received for client2.

Get current server time.
Call xname with a later time than the current server time.
Warp pointer within win.
Verify that no event is received for client2.
>>CODE
Window	win;
Window	grabwin;
Display	*client2;
XEvent	ev;
unsigned int 	mask;

	mask = PointerMotionMask;

	client2 = opendisplay();

	win = defwin(display);
	(void) warppointer(display, win, 0, 0);
	XSelectInput(client2, win, (unsigned long)mask);

	grabwin = defwin(display);

	thetime = gettime(display);
	XGrabPointer(display, grabwin, False, mask, GrabModeSync, GrabModeAsync,
		None, None, thetime);

	XSync(client2, True);
	if (isdeleted())
		return;

	thetime -= 100;
	XCALL;

	(void) warppointer(display, win, 8, 8);
	if (XCheckMaskEvent(client2, (unsigned long)mask, &ev)) {
		report("Grab was released when time was earlier than last-pointer-grab time");
		FAIL;
	} else
		CHECK;

	XSync(client2, True);	/* Flush any remaining events */

	/*
	 * Get current time again and add several minutes to get a time in the
	 * future.
	 */
	thetime = gettime(display);
	thetime += ((config.speedfactor+1) * 1000000);

	XCALL;

	(void) warppointer(display, win, 12, 1);
	if (XCheckMaskEvent(client2, (unsigned long)mask, &ev)) {
		report("Grab was released when time was earlier than last-pointer-grab time");
		FAIL;
	} else
		CHECK;

	CHECKPASS(2);
>>ASSERTION Good A
A call to xname generates
.S EnterNotify
and
.S LeaveNotify
events.
>>STRATEGY
Create the grab window.
Create spare window 'win'.
Warp pointer to win.
Grab pointer with XGrabPointer.
Enable events on win and grab windows.
Call xname to release grab.
Verify that a leave event is generated on the grab window.
Verify that an enter event is generated on win.
>>CODE
Window	grabwin;
Window	win;
XEvent	ev;
XCrossingEvent	*cp;
XEnterWindowEvent	entergood;
XLeaveWindowEvent	leavegood;

	grabwin = defwin(display);
	win = defwin(display);

	(void) warppointer(display, win, 0, 0);

	XGrabPointer(display, grabwin, False, (unsigned int)PointerMotionMask,
		GrabModeSync, GrabModeAsync, None, None, CurrentTime);

	XSelectInput(display, win, EnterWindowMask|LeaveWindowMask);
	XSelectInput(display, grabwin, EnterWindowMask|LeaveWindowMask);

	XCALL;

	defsetevent(entergood, display, EnterNotify);
	entergood.root = DRW(display);
	entergood.subwindow = None;
	entergood.time = 0;
	entergood.mode = NotifyNormal;
	entergood.detail = NotifyNonlinear;
	entergood.same_screen = True;
	entergood.focus = False;
	entergood.state = 0;

	defsetevent(leavegood, display, LeaveNotify);
	leavegood.root = DRW(display);
	leavegood.subwindow = None;
	leavegood.time = 0;
	leavegood.mode = NotifyNormal;
	leavegood.detail = NotifyNonlinear;
	leavegood.same_screen = True;
	leavegood.focus = False;
	leavegood.state = 0;

	/*
	 * There is an Ungrab mode leave from the grab window followed by
	 * an enter to the original window.
	 */
	leavegood.window = grabwin;
	leavegood.mode = NotifyUngrab;
	entergood.window = win;
	entergood.mode = NotifyUngrab;

	if (getevent(display, &ev) == 0 || ev.type != LeaveNotify) {
		report("No ungrab-mode leave event from the grab window");
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
		report("No ungrab-mode enter notify event to window pointer is in");
		FAIL;
	} else
		CHECK;

	/* The cursor was at 0,0 */
	entergood.x = 0;
	entergood.y = 0;

	cp = (XCrossingEvent*)&ev;
	/* Set the fields that we can't conveniently check */
	entergood.time = cp->time;
	entergood.x_root = cp->x_root;
	entergood.y_root = cp->y_root;
	entergood.focus = cp->focus;
	if (checkevent((XEvent*)&entergood, &ev))
		FAIL;
	else
		CHECK;
	
	CHECKPASS(4);
>># Things about the grab being released --> grab{button,pointer}

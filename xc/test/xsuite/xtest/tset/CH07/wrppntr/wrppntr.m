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
 * $XConsortium: wrppntr.m,v 1.5 94/04/17 21:07:02 rws Exp $
 */
>>TITLE XWarpPointer CH07
void

Display	*display = Dsp;
Window	src_w;
Window	dest_w;
int 	src_x = SRC_X;
int 	src_y = SRC_Y;
unsigned int 	src_width = SRC_WIDTH;
unsigned int 	src_height = SRC_HEIGHT;
int 	dest_x = DEST_X;
int 	dest_y = DEST_Y;
>>EXTERN

#define	SRC_X	7
#define	SRC_Y	10
#define	SRC_WIDTH	20
#define	SRC_HEIGHT	31
#define	DEST_X	11
#define	DEST_Y	13

#define	POS_ABS	1
#define	POS_REL	2

struct	coord {
	int 	x;
	int 	y;
};

/*
 * Get the current position of the pointer.  If type is POS_REL this
 * is relative to the given window.  For a type POS_ABS the coordinates
 * are absolute.
 */
static void
getpos(w, co, type)
Window	w;
struct	coord	*co;
int 	type;
{
Window	root, child;
unsigned int 	mask;
int 	xroot, yroot;
int 	status;

	if (w == None)
		w = DRW(Dsp);

	status = XQueryPointer(Dsp, w, &root, &child, &xroot, &yroot, &co->x, &co->y, &mask);
	if (status == False) {
		delete("Could not get pointer position");
		return;
	}

	if (type == POS_ABS) {
		co->x = xroot;
		co->y = yroot;
	}
}

>>ASSERTION Good A
When
.A dest_w
is
.S None ,
then a call to xname
moves the pointer by the offsets
.A dest_x ,
.A dest_y
relative to the current position of the pointer.
>>STRATEGY
Get current position of pointer.
Set dest_w to None.
Call xname.
Verify that new position is offset from old by dest_x, dest_y.
>>CODE
struct	coord	old;
struct	coord	new;

	(void) warppointer(display, DRW(display), 100, 100);

	dest_w = None;
	src_w = None;

	getpos(None, &old, POS_ABS);

	XCALL;

	getpos(None, &new, POS_ABS);

	if (dest_x == 0 || dest_y == 0)
		delete("Setup error of destination coordinates");

	if (new.x == old.x + dest_x)
		CHECK;
	else {
		report("X position of pointer incorrect - was %d, expecting %d",
			new.x, old.x + dest_x);
		FAIL;
	}

	if (new.y == old.y + dest_y)
		CHECK;
	else {
		report("Y position of pointer incorrect - was %d, expecting %d",
			new.y, old.y + dest_y);
		FAIL;
	}

	CHECKPASS(2);
>>ASSERTION Good A
When
.A dest_w
is a window
and
.A src_w
is
.S None ,
then a call to xname
moves the pointer to the offsets
.A dest_x ,
.A dest_y
relative to the origin of
.A dest_w .
>>STRATEGY
Create window for dest_w.
Set src_w to None.
Call xname.
Verify that pointer is at (dest_x,dest_y) from origin of dest_w.
>>CODE
struct	coord	pos;

	(void) warppointer(display, DRW(display), 0, 0);

	dest_w = defwin(display);
	src_w = None;

	XCALL;

	getpos(dest_w, &pos, POS_REL);

	if (dest_x == pos.x && dest_y == pos.y)
		CHECK;
	else {
		report("Position relative to destination window was incorrect");
		report("  position was (%d, %d), expecting (%d, %d)",
			pos.x, pos.y, dest_x, dest_y);
		FAIL;
	}

	CHECKPASS(1);
>>ASSERTION Good A
When
.A dest_w
and
.A src_w
are windows
and the pointer is within the specified rectangle of
.A src_w ,
then a call to xname
moves the pointer to the offsets
.A dest_x ,
.A dest_y
relative to the origin of
.A dest_w .
>>STRATEGY
Create window dest_w.
Create window src_w.
Move pointer to within the specified rectange of src_w.
Call xname.
Verify that pointer moves to offset (dest_x, dest_y) from origin of dest_w.
>>CODE
struct	coord	pos;

	dest_w = defwin(display);
	src_w = defwin(display);

	(void) warppointer(display, src_w, src_x, src_y);

	XCALL;

	getpos(dest_w, &pos, POS_REL);

	if (pos.x == dest_x && pos.y == dest_y)
		CHECK;
	else {
		report("Pointer not positioned correctly when pointer in source rectangle");
		report("  position was (%d, %d), expecting (%d, %d)",
			pos.x, pos.y, dest_x, dest_y);
		FAIL;
	}

	CHECKPASS(1);
>>ASSERTION Good A
When
.A dest_w
and
.A src_w
are windows
and the pointer is not within the specified rectangle of
.A src_w ,
then a call to xname does not move the pointer.
>>STRATEGY
Create window dest_w.
Create window src_w.
Move pointer outside the specified rectange of src_w.
Call xname.
Verify that the pointer is not moved.
>>CODE
PointerPlace	*pplace;

	dest_w = defwin(display);
	src_w = defwin(display);

	pplace = warppointer(display, src_w, (int)(src_x+src_width+2), src_y);

	XCALL;

	if (pointermoved(display, pplace)) {
		report("Pointer was moved when outside the source rectangle");
		FAIL;
	} else
		CHECK;

	CHECKPASS(1);
>>ASSERTION Good A
When
.A src_height
is zero,
then it is replaced with the current height of
.A src_w
minus
.A src_y .
>>STRATEGY
Create window src_w.
Create window dest_w.
Set src_height to 0.
Position pointer in src_w such that it is inside source rectangle.
Call xname.
Verify that pointer is moved.
>>CODE
unsigned int 	width, height;
struct	coord	pos;

	src_w = defwin(display);
	dest_w = defwin(display);
	src_height = 0;

	getsize(display, src_w, &width, &height);

	(void) warppointer(display, src_w, src_x+3, (int)height-3);

	XCALL;

	getpos(dest_w, &pos, POS_REL);
	if (pos.x == dest_x && pos.y == dest_y)
		CHECK;
	else {
		report("Pointer not moved correctly when pointer in source rectangle");
		report("  and src_height 0 - position was (%d, %d), expecting (%d, %d)",
			pos.x, pos.y, dest_x, dest_y);
		FAIL;
	}

	CHECKPASS(1);

>>ASSERTION Good A
When
.A src_width
is zero,
then it is replaced with the current width of
.A src_w
minus
.A src_x .
>>STRATEGY
Create window src_w.
Create window dest_w.
Set src_width to 0.
Position pointer in src_w such that it is inside source rectangle.
Call xname.
Verify that pointer is moved.
>>CODE
unsigned int 	width, height;
struct	coord	pos;

	src_w = defwin(display);
	dest_w = defwin(display);
	src_width = 0;

	getsize(display, src_w, &width, &height);

	(void) warppointer(display, src_w, (int)width-3, src_y+3);

	XCALL;

	getpos(dest_w, &pos, POS_REL);
	if (pos.x == dest_x && pos.y == dest_y)
		CHECK;
	else {
		report("Pointer not moved correctly when pointer in source rectangle");
		report("  and src_width 0 - position was (%d, %d), expecting (%d, %d)",
			pos.x, pos.y, dest_x, dest_y);
		FAIL;
	}

	CHECKPASS(1);

>>ASSERTION Good A
A call to xname generates events as though
the pointer position had been instantaneously moved by the user.
>>STRATEGY
Create dest_w.
Position pointer at 0,0 in dest_w.
Enable events.
Call xname to move pointer within window.
Verify that only one motion event is generated.
>>CODE
XEvent	ev;
XMotionEvent	good;
struct	coord	pos;
int 	n;

	dest_w = defwin(display);
	src_w = None;

	(void) warppointer(display, dest_w, 0, 0);

	XSelectInput(display, dest_w, PointerMotionMask);

	XCALL;

	defsetevent(good, display, MotionNotify);
	good.window = dest_w;
	good.root = DRW(display);
	good.subwindow = None;
	good.time = 0;
	good.x = dest_x;
	good.y = dest_y;
	good.state = 0;
	good.is_hint = False;
	good.same_screen = True;

	getpos(None, &pos, POS_ABS);
	good.x_root = pos.x;
	good.y_root = pos.y;

	n = getevent(display, &ev);
	if (n > 1) {
		report("More than one event generated");
		FAIL;
	} else
		CHECK;

	if (checkevent((XEvent*)&good, &ev))
		FAIL;
	else
		CHECK;
	
	CHECKPASS(2);
>>ASSERTION Good A
When an active pointer grab is in progress and the pointer is within
the confine_to window, then a call to xname will only move the pointer
as far as the closest edge of the confine_to window.
>>STRATEGY
Create confine_to window.
Create dest_w window.
Call XGrabPointer to actively grab the pointer with confine_to set.
Attempt to move pointer into dest_w window with xname.
Verify that pointer is still within the confine_to window.
>>CODE
Window	confine;
Window	grabwin;
struct	coord	pos;
unsigned int 	width, height;

	confine = defwin(display);
	dest_w = defwin(display);
	grabwin = defwin(display);
	src_w = None;

	/* Set border to zero so that containment size is just width and height */
	XSetWindowBorderWidth(display, confine, 0);
	getsize(display, confine, &width, &height);

	/* May as well put the pointer there to begin with */
	(void) warppointer(display, confine, 1, 1);

	XGrabPointer(display, grabwin, False, 0, GrabModeAsync, GrabModeAsync,
		confine, None, CurrentTime);

	if (isdeleted())
		return;

	XCALL;

	XUngrabPointer(display, CurrentTime);
	getpos(confine, &pos, POS_REL);
	if (pos.x > 0 && pos.x < width && pos.y > 0 && pos.y < height)
		CHECK;
	else {
		report("Pointer did not remain within the confine_to window");
		FAIL;
	}

	/*
	 * We can also check that the pointer is on a edge.  (The windows
	 * do not overlap).
	 */
	if (pos.x == 0 || pos.y == 0 || pos.x == width-1 || pos.y == height-1)
		CHECK;
	else {
		report("Pointer was not on edge of confine_to window");
		FAIL;
	}

	CHECKPASS(2);
>>ASSERTION Bad A
.ER BadWindow None

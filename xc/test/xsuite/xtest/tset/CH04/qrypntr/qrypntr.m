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
 * $XConsortium: qrypntr.m,v 1.6 94/04/17 21:03:35 rws Exp $
 */
>>TITLE XQueryPointer CH04
Bool

Display *display = Dsp;
Window w;
Window *root_return = &root;
Window *child_return = &child;
int *root_x_return = &root_x;
int *root_y_return = &root_y;
int *win_x_return = &win_x;
int *win_y_return = &win_y;
unsigned int *mask_return = &mask;
>>EXTERN
Window	root;
Window	child;
int	root_x;
int	root_y;
int	win_x;
int	win_y;
unsigned int mask;

static void
set_variables()
{
	root = (Window)0;
	child= (Window)0;
	root_x= 0;
	root_y= 0;
	win_x = 0;
	win_y =0;
	mask = 0;
}

#define XQP_X 60
#define XQP_Y 50
>>ASSERTION Good A
A call to xname returns the root window the pointer is logically on in
.A root_return ,
and the pointer coordinates relative to the root window's origin in
.A root_x_return
and
.A root_y_return .
>>STRATEGY
Call XWarpPointer to move pointer to a known position
Call xname to obtain pointer position
Verify that the correct root window and pointer position were returned
>>CODE
Bool ret;

/* Call XWarpPointer to move pointer to a known position */
	(void) warppointer(display, DefaultRootWindow(display), XQP_X, XQP_Y);

/* Call xname to obtain pointer position */
	w = DefaultRootWindow(display);
	set_variables();
	ret = XCALL;
	if (ret != True) {
		delete("%s returned %s, expecting True", TestName,
			boolname(ret));
	} else
		CHECK;

/* Verify that the correct root window and pointer position were returned */
	if (root != DefaultRootWindow(display)) {
		FAIL;
		report("%s did not return expected root window", TestName);
		trace("Expected root window=%0x", DefaultRootWindow(display));
		trace("Returned root window=%0x", root);
	} else
		CHECK;

	if (root_x != XQP_X) {
		FAIL;
		report("%s did not return expected root_x_return", TestName);
		trace("Expected root_x_return=%d", XQP_X);
		trace("Returned root_x_return=%d", root_x);
	} else
		CHECK;

	if (root_y != XQP_Y) {
		FAIL;
		report("%s did not return expected root_y_return", TestName);
		trace("Expected root_y_return=%d", XQP_Y);
		trace("Returned root_y_return=%d", root_y);
	} else
		CHECK;

	CHECKPASS(4);
>>ASSERTION Good C
If multiple screens are supported:
When the pointer is not on the same screen as the specified window
.A w ,
then a call to xname returns 
.S False ,
.A child_return
is set to 
.S None ,
and
.A win_x_return
and
.A win_y_return
are set to zero.
>>STRATEGY
If multiple screens are supported:
	Ensure that pointer is not on the alternate screen
	Call xname to obtain pointer position on alternate screen
	Verify that win_x_return, win_y_return and child_return
		were set correctly
>>CODE
Bool ret;

/* If multiple screens are supported: */
	if (config.alt_screen == -1) {
		unsupported("No alternate screen supported");
		return;
	}

/* 	Ensure that pointer is not on the alternate screen */
	(void) warppointer(display, DefaultRootWindow(display), XQP_X, XQP_Y);

/* 	Call xname to obtain pointer position on alternate screen */
	set_variables();
	w = RootWindow(display, config.alt_screen);
	ret = XCALL;
	if (ret != False) {
		FAIL;
		report("%s returned %s, expecting False.", TestName,
			boolname(ret));
	} else
		CHECK;

/* 	Verify that win_x_return, win_y_return and child_return */
/* 		were set correctly */
	if (child != None) {
		FAIL;
		report("%s returned an unexpected window in child_return",
			TestName);
		trace("Expected (*child_return)=%0x (None)", None);
		trace("Returned (*child_return)=%0x", child);
	} else
		CHECK;

	if (win_x != 0) {
		FAIL;
		report("%s returned an unexpected value for win_x_return",
			TestName);
		trace("Expected (*win_x_return)=0");
		trace("Returned (*win_x_return)=%d", win_x);
	} else
		CHECK;

	if (win_y != 0) {
		FAIL;
		report("%s returned an unexpected value for win_y_return",
			TestName);
		trace("Expected (*win_y_return)=0");
		trace("Returned (*win_y_return)=%d", win_y);
	} else
		CHECK;

	CHECKPASS(4);
>>ASSERTION Good A
When the pointer is on the same screen as the specified window
.A w
and a child of that window contains the pointer, then a call to xname returns
.S True ,
.A win_x_return
and
.A win_y_return
are set to the pointer coordinates relative to the origin
of the specified window
.A w ,
and
.A child_return
is set to the child window.
>>STRATEGY
Create a child of the root window
Ensure the pointer is over the child by calling warppointer()
Call xname to obtain pointer position
Verify that win_x_return, win_y_return and child_return
	were set correctly
>>CODE
Window rc;
XVisualInfo *vp;
Bool ret;

/* Create a child of the root window */
	resetvinf(VI_WIN); (void)nextvinf(&vp);
	/* makewinpos border width had better not change... */
	rc = makewinpos(display, vp, XQP_X-1, XQP_Y-1);

/* Ensure the pointer is over the child by calling warppointer() */
	(void) warppointer(display, DefaultRootWindow(display),
		XQP_X+10, XQP_Y+5);

/* Call xname to obtain pointer position */
	set_variables();
	w = DefaultRootWindow(display);
	ret = XCALL;
	if (ret != True) {
		FAIL;
		report("%s returned %s, expecting True.", TestName,
			boolname(ret));
	} else
		CHECK;

/* Verify that win_x_return, win_y_return and child_return */
/* 	were set correctly */
	if (child != rc) {
		FAIL;
		report("%s returned an unexpected window in child_return",
			TestName);
		trace("Expected (*child_return)=%0x", rc);
		trace("Returned (*child_return)=%0x", child);
	} else
		CHECK;

	if (win_x != XQP_X+10) {
		FAIL;
		report("%s returned an unexpected value for win_x_return",
			TestName);
		trace("Expected (*win_x_return)=%d", XQP_X+10);
		trace("Returned (*win_x_return)=%d", win_x);
	} else
		CHECK;

	if (win_y != XQP_Y+5) {
		FAIL;
		report("%s returned an unexpected value for win_y_return",
			TestName);
		trace("Expected (*win_y_return)=%d", XQP_Y+5);
		trace("Returned (*win_y_return)=%d", win_y);
	} else
		CHECK;

	CHECKPASS(4);
>>ASSERTION Good A
When the pointer is on the same screen as the specified window
.A w
and no child of that window contains the pointer,
then a call to xname returns 
.S True ,
.A win_x_return
and
.A win_y_return
are set to the pointer coordinates relative to the origin
of the specified window
.A w ,
and
.A child_return
is set to
.S None .
>>STRATEGY
Create window
Ensure the pointer is over the window by calling warppointer()
Call xname to obtain pointer position
Verify that win_x_return, win_y_return and child_return
	were set correctly
>>CODE
Bool ret;

/* Create a window */
	w = defwin(display);

/* Ensure the pointer is over the window by calling warppointer() */
	(void) warppointer(display, w, 10, 5);

/* Call xname to obtain pointer position */
	set_variables();
	ret = XCALL;
	if (ret != True) {
		FAIL;
		report("%s returned %s, expecting True.", TestName,
			boolname(ret));
	} else
		CHECK;

/* Verify that win_x_return, win_y_return and child_return */
/* 	were set correctly */
	if (child != None) {
		FAIL;
		report("%s returned an unexpected window in child_return",
			TestName);
		trace("Expected child_return=%0x", None);
		trace("Returned child_return=%0x", child);
	} else
		CHECK;

	if (win_x != 10) {
		FAIL;
		report("%s returned an unexpected value for win_x_return",
			TestName);
		trace("Expected win_x_return=%d", 10);
		trace("Returned win_x_return=%d", win_x);
	} else
		CHECK;

	if (win_y != 5) {
		FAIL;
		report("%s returned an unexpected value for win_y_return",
			TestName);
		trace("Expected (*win_y_return)=%d", 5);
		trace("Returned (*win_y_return)=%d", win_y);
	} else
		CHECK;

	CHECKPASS(4);
>>ASSERTION Good B 1
A call to xname returns the current logical state of the keyboard buttons 
and the modifier keys as the bitwise inclusive OR of one or more
of the button or modifier key bitmasks in
.A mask_return .
>>ASSERTION Bad A
.ER BadWindow

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
 * $XConsortium: gtinptfcs.m,v 1.5 94/04/17 21:06:30 rws Exp $
 */
>>TITLE XGetInputFocus CH07
void

Display	*display = Dsp;
Window	*focus_return = &fretwin;
int 	*revert_to_return = &rtret;
>>EXTERN

static Window	fretwin;
static int 	rtret;

>>ASSERTION Good A
A call to xname
returns the focus window,
.S PointerRoot ,
or
.S None
to
.A focus_return
and the current focus revert state to
.A revert_to_return .
>>STRATEGY
Set focus state.
Call xname to get focus state.
Verify that it was the state that was set.
>>EXTERN
#define	REVERT_TO	RevertToPointerRoot

>>SET startup focusstartup
>>SET cleanup focuscleanup
>>CODE
Window	win;

	win = defwin(display);

	XSetInputFocus(display, win, REVERT_TO, CurrentTime);

	XCALL;

	if (*focus_return != win) {
		report("Incorrect window was returned");
		FAIL;
	} else
		CHECK;

	if (*revert_to_return != REVERT_TO) {
		report("The revert_to_return argument was incorrect");
		report("  was %d, expecting %d", *revert_to_return, REVERT_TO);
		FAIL;
	} else
		CHECK;

	CHECKPASS(2);

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
 * $XConsortium: gtslctnown.m,v 1.5 94/04/17 21:03:31 rws Exp $
 */
>>TITLE XGetSelectionOwner CH04
Window

Display *display = Dsp;
Atom selection = XA_COPYRIGHT;
>>EXTERN
#include "Xatom.h"
>>ASSERTION Good A
A call to xname returns the window ID associated with the window that
currently owns the specified
.A selection .
>>STRATEGY
Create a window with a selection.
Call xname to obtain the window ID of the selection owner.
Verify the window ID returned was that of the selection owner.
>>CODE
Window owner, ret;

/* Create a window with a selection. */
	owner = defwin(display);
	XSetSelectionOwner(display, selection, owner, CurrentTime);

/* Call xname to obtain the window ID of the selection owner. */
	ret = XCALL;

/* Verify the window ID returned was that of the selection owner. */
	if (ret != owner) {
		FAIL;
		report("%s did not return the window ID of the selection owner",
			TestName);
		trace("Expected window ID: %0x", owner);
		trace("Returned window ID: %0x", ret);
	} else
		CHECK;

	CHECKPASS(1);

>>ASSERTION Good A
When the
.A selection
is not owned by a window, then a call to xname returns
.S None . 
>>STRATEGY
Ensure the selection has no owner.
Call xname to obtain selection owner.
Verify that xname returned None.
>>CODE
Window ret;

/* Ensure the selection has no owner. */
	XSetSelectionOwner(display, selection, None, CurrentTime);

/* Call xname to obtain selection owner. */
	ret = XCALL;

/* Verify that xname returned None. */
	if(ret != None) {
		FAIL;
		report("%s returned %0x on a selection with no owner.",
			TestName);
		report("Expecting None.");
	} else
		CHECK;

	CHECKPASS(1);

>>ASSERTION Bad A
.ER BadAtom

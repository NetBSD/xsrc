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
 * $XConsortium: allcwmhnts.m,v 1.8 94/04/17 21:08:31 rws Exp $
 */
>>TITLE XAllocWMHints CH09
XWMHints *
XAllocWMHints()
>>ASSERTION Good A
A call to xname allocates and returns a pointer to a
.S XWMHints 
structure, which can be freed with XFree,
in which each component is set to zero.
>>STRATEGY
Allocate a XWMHints structure with XAllocWMHints.
Verify that the call did not return NULL.
Verify that each component of the allocated structure was set to 0.
Free the allocated structure using XFree.
>>CODE
XWMHints	*hints;

	hints = XCALL;

	if(hints == (XWMHints *) NULL) {
		delete("XAllocWMHints returned NULL.");
		return;
	} else
		CHECK;

	if( hints->flags != 0L ) {
		report("The flags component of the XWMHints structure was not 0.");
		FAIL;
	} else
		CHECK;

	if( hints->input != (Bool) 0 ) {
		report("The input component of the XWMHints structure was not 0.");
		FAIL;
	} else
		CHECK;

	if( hints->initial_state != (int) 0 ) {
		report("The initial_state component of the XWMHints structure was not 0.");
		FAIL;
	} else
		CHECK;

	if( hints->icon_pixmap != (Pixmap) 0 ) {
		report("The icon_pixmap component of the XWMHints structure was not 0.");
		FAIL;
	} else
		CHECK;

	if( hints->icon_window != (Window) 0 ) {
		report("The icon_window component of the XWMHints structure was not 0.");
		FAIL;
	} else
		CHECK;

	if( hints->icon_x != (int) 0 ) {
		report("The icon_x component of the XWMHints structure was not 0.");
		FAIL;
	} else
		CHECK;

	if( hints->icon_y != (int) 0 ) {
		report("The icon_y component of the XWMHints structure was not 0.");
		FAIL;
	} else
		CHECK;

	if( hints->icon_mask != (Pixmap) 0 ) {
		report("The icon_mask component of the XWMHints structure was not 0.");
		FAIL;
	} else
		CHECK;

	if( hints->window_group != (XID) 0 ) {
		report("The window_group component of the XWMHints structure was not 0.");
		FAIL;
	} else
		CHECK;

	XFree((char*)hints);

	CHECKPASS(10);

>>ASSERTION Bad B 1
When insufficient memory is available, then
a call to xname returns NULL.
>># Completed	Kieron	Review

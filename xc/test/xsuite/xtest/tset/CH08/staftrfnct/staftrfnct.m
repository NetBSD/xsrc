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
 * $XConsortium: staftrfnct.m,v 1.8 94/04/17 21:08:17 rws Exp $
 */
>>TITLE XSetAfterFunction CH08
int ((*)())()
XSetAfterFunction(display, procedure)
Display *display = Dsp;
int (*procedure)() = afterfunction;
>>EXTERN
static	int	counter = 0;

static int
afterfunction(display)
Display *display;
{
	trace("In afterfunction(), screen number %d", XDefaultScreen(display));
	return(counter++);
}
static int
_afterfunction(display)
Display *display;
{
	trace("In _afterfunction(), screen number %d", XDefaultScreen(display));
	return(--counter);
}
>>ASSERTION Good A
A call to xname
sets the after function to
.A procedure .
>>STRATEGY
Create window.
Verify that afterfunction was not called.
Call XSetAfterFunction to set the after function to afterfunction.
Create window.
Verify that the after function was called.
>>CODE
int	oldcounter;

	oldcounter = counter;
/* Create window. */
	(void) mkwin(display, (XVisualInfo *) NULL, (struct area *) NULL, False);
/* Verify that afterfunction was not called. */
	if (counter != oldcounter) {
		delete("After function already set to afterfunction.");
		return;
	}
	else
		CHECK;
/* Call XSetAfterFunction to set the after function to afterfunction. */
	(void) XCALL;
/* Create window. */
	(void) mkwin(display, (XVisualInfo *) NULL, (struct area *) NULL, False);
/* Verify that the after function was called. */
	if (counter == oldcounter) {
		report("After function not called.");
		FAIL;
	}
	else
		CHECK;
	CHECKPASS(2);
>>ASSERTION Good A
A call to xname
returns the previous after function.
>>STRATEGY
Call XSetAfterFunction to set after function to afterfunction.
Call XSetAfterFunction to set after function to _afterfunction.
Verify that XSetAfterFunction returned afterfunction.
Call XSetAfterFunction to set after function to afterfunction.
Verify that XSetAfterFunction returned _afterfunction.
>>CODE
int	(*proc)();

/* Call XSetAfterFunction to set after function to afterfunction. */
	procedure = afterfunction;
	(void) XCALL;
/* Call XSetAfterFunction to set after function to _afterfunction. */
	procedure = _afterfunction;
	proc = XCALL;
/* Verify that XSetAfterFunction returned afterfunction. */
	if (proc != afterfunction) {
		report("Returned 0x%x, expected 0x%x", proc, afterfunction);
		FAIL;
	}
	else
		CHECK;
/* Call XSetAfterFunction to set after function to afterfunction. */
	procedure = afterfunction;
	proc = XCALL;
/* Verify that XSetAfterFunction returned _afterfunction. */
	if (proc != _afterfunction) {
		report("Returned 0x%x, expected 0x%x", proc, _afterfunction);
		FAIL;
	}
	else
		CHECK;
	CHECKPASS(2);
>>ASSERTION Good B 6
>>#NOTE Category B, reason 6 (unreasonable amount of test development time).
After a call to xname,
all Xlib functions that generate protocol requests call
.A procedure
before returning.

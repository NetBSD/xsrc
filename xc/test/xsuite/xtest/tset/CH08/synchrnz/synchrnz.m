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
 * $XConsortium: synchrnz.m,v 1.7 94/04/17 21:08:20 rws Exp $
 */
>>TITLE XSynchronize CH08
int ((*)())()
XSynchronize(display, onoff)
Display *display = Dsp;
Bool onoff;
>>EXTERN
static int
afterfunction(display)
Display *display;
{
	return((int) display);
}
>>ASSERTION Good B 1
>>#NOTE Untestable.
A call to xname
with
.A onoff
set to
.S True
turns on synchronous behavior.
>>ASSERTION Good B 1
>>#NOTE Untestable.
A call to xname
with
.A onoff
set to
.S False
turns off synchronous behavior.
>>ASSERTION Good A
A call to xname
with
.A onoff
set to
.S True
sets the after function to a non-NULL value.
>>STRATEGY
Call XSynchronize with onoff set to True.
Call XSetAfterFunction to get value of old after function.
Verify that XSetAfterFunction returned non-NULL.
>>CODE
int	(*proc)();

/* Call XSynchronize with onoff set to True. */
	onoff = True;
	(void) XCALL;
/* Call XSetAfterFunction to get value of old after function. */
	proc = XSetAfterFunction(display, afterfunction);
/* Verify that XSetAfterFunction returned non-NULL. */
	if (proc == (int (*)()) NULL) {
		report("Returned NULL, expected non-NULL.");
		FAIL;
	}
	else
		CHECK;
	CHECKPASS(1);
>>ASSERTION Good A
A call to xname
with
.A onoff
set to
.S False
sets the after function to NULL.
>>STRATEGY
Call XSynchronize with onoff set to False.
Call XSetAfterFunction to get value of old after function.
Verify that XSetAfterFunction returned NULL.
>>CODE
int	(*proc)();

/* Call XSynchronize with onoff set to False. */
	onoff = False;
	(void) XCALL;
/* Call XSetAfterFunction to get value of old after function. */
	proc = XSetAfterFunction(display, afterfunction);
/* Verify that XSetAfterFunction returned NULL. */
	if (proc != (int (*)()) NULL) {
		report("Returned non-NULL, expected NULL.");
		FAIL;
	}
	else
		CHECK;
	CHECKPASS(1);
>>ASSERTION Good A
A call to xname
returns the previous after function.
>>STRATEGY
Call XSetAfterFunction to set after function to afterfunction.
Call XSynchronize with onoff set to False.
Verify that XSynchronize returned afterfunction.
Call XSetAfterFunction to set after function to afterfunction.
Call XSynchronize with onoff set to True.
Verify that XSynchronize returned afterfunction.
>>CODE
int	(*proc)();

/* Call XSetAfterFunction to set after function to afterfunction. */
	(void) XSetAfterFunction(display, afterfunction);
/* Call XSynchronize with onoff set to False. */
	onoff = False;
	proc = XCALL;
/* Verify that XSynchronize returned afterfunction. */
	if (proc != afterfunction) {
		report("Did not return previous after function.");
		FAIL;
	}
	else
		CHECK;
/* Call XSetAfterFunction to set after function to afterfunction. */
	(void) XSetAfterFunction(display, afterfunction);
/* Call XSynchronize with onoff set to True. */
	onoff = True;
	proc = XCALL;
/* Verify that XSynchronize returned afterfunction. */
	if (proc != afterfunction) {
		report("Did not return previous after function.");
		FAIL;
	}
	else
		CHECK;
	CHECKPASS(2);

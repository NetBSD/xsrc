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
 * $XConsortium: gterrrtxt.m,v 1.8 94/04/17 21:07:42 rws Exp $
 */
>>TITLE XGetErrorText CH08
void
XGetErrorText(display, code, buffer_return, length)
Display *display = Dsp;
int code = 1;
char *buffer_return = buffer;
int length = sizeof(buffer);
>>EXTERN
static char buffer[512];
>>ASSERTION Good A
A call to xname
returns in
.A buffer_return
a string terminated with ASCII NUL.
>>STRATEGY
Set each character in buffer_return to non-ASCII NUL.
Call XGetErrorText.
Verify that there exists at least one ASCII NUL
character in buffer_return.
>>CODE
int	i;

/* Set each character in buffer_return to non-ASCII NUL. */
	for (i=0; i < sizeof(buffer); i++) {
		if (!i)
			CHECK;
		buffer[i] = 'A';
	}
	buffer_return = buffer;
/* Call XGetErrorText. */
	XCALL;
/* Verify that there exists at least one ASCII NUL */
/* character in buffer_return. */
	for (i=0; i < sizeof(buffer); i++) {
		if (buffer[i] == '\0') {
			CHECK;
			break;
		}
	}
	if (i == sizeof(buffer)) {
		report("Returned string not terminated with ASCII NUL.");
		FAIL;
	}
	else
		CHECK;
	CHECKPASS(3);
>>ASSERTION Good B 1
>>#NOTE How to prove this??? Answer: tet result code FIP
After a call to xname
.A buffer_return
contains a textual description of
.A code .

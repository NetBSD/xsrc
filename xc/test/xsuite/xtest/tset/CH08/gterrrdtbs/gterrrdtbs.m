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
 * $XConsortium: gterrrdtbs.m,v 1.8 94/04/17 21:07:42 rws Exp $
 */
>>TITLE XGetErrorDatabaseText CH08
void
XGetErrorDatabaseText()
Display *display = Dsp;
char *name;
char *message;
char *default_string = "default_string";
char *buffer_return = buffer;
int length = sizeof(buffer);
>>EXTERN
#include "Xproto.h"

static char buffer[512];
>>ASSERTION Good A
>>#NOTE	As the precise format of a message is not defined,
>>#NOTE there is not too much which can be tested.
>>#NOTE Therefore, this assertion is deliberately vague.
A call to xname
returns a string in
.A buffer_return .
>>STRATEGY
Set each character in buffer_return to non-ASCII NUL.
Set name to "foo".
Set message to "bar".
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
/* Set name to "foo". */
	name = "foo";
/* Set message to "bar". */
	message = "bar";
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
		report("String not returned.");
		FAIL;
	}
	else
		CHECK;
	CHECKPASS(3);
>>ASSERTION Good A
A call to xname
with
.A name
set to the string
.S XProtoError
and
.A message
set to the string representation of a protocol error number
returns a string terminated with ASCII NUL in
.A buffer_return .
>>STRATEGY
Set each character in buffer_return to non-ASCII NUL.
Set name to "XProtoError".
Set message to the string representation of BadWindow.
Call XGetErrorText.
Verify that there exists at least one ASCII NUL
character in buffer_return.
>>CODE
int	i;
char	msgbuf[512];

/* Set each character in buffer_return to non-ASCII NUL. */
	for (i=0; i < sizeof(buffer); i++) {
		if (!i)
			CHECK;
		buffer[i] = 'A';
	}
	buffer_return = buffer;
/* Set name to "XProtoError". */
	name = "XProtoError";
/* Set message to the string representation of BadWindow. */
	(void) sprintf(msgbuf, "%d", BadWindow);
	message = msgbuf;
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
>>ASSERTION Good A
A call to xname
with
.A name
set to the string
.S XRequest
and
.A message
set to the string representation of a major protocol number
returns a string terminated with ASCII NUL in
.A buffer_return .
>>STRATEGY
Set each character in buffer_return to non-ASCII NUL.
Set name to "XRequest".
Set message to the string representation of X_DestroyWindow.
Call XGetErrorText.
Verify that there exists at least one ASCII NUL
character in buffer_return.
>>CODE
int	i;
char	msgbuf[512];

/* Set each character in buffer_return to non-ASCII NUL. */
	for (i=0; i < sizeof(buffer); i++) {
		if (!i)
			CHECK;
		buffer[i] = 'A';
	}
	buffer_return = buffer;
/* Set name to "XRequest". */
	name = "XRequest";
/* Set message to the string representation of X_DestroyWindow. */
	(void) sprintf(msgbuf, "%d", X_DestroyWindow);
	message = msgbuf;
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
>>ASSERTION Good D 1
If the implementation supports an error database:
The error message database used is
.S /usr/lib/X11/XErrorDB .
>>ASSERTION Good B 1
When a call to xname is made and
no string is found in the error database,
then
.A default_string
is returned in
.A buffer_return .

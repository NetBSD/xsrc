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
 * $XConsortium: opndsply.m,v 1.9 94/04/17 21:11:58 rws Exp $
 */
>>TITLE OpenDisplay XPROTO
>>SET startup protostartup
>>SET cleanup protocleanup
>>EXTERN
/* Touch test for OpenDisplay request */

/****************************************************************************
 * Copyright 1989 by Sequent Computer Systems, Inc., Portland, Oregon       *
 *                                                                          *
 *                                                                          *
 *                         All Rights Reserved                              *
 *                                                                          *
 * Permission to use, copy, modify, and distribute this software and its    *
 * documentation for any purpose and without fee is hereby granted,         *
 * provided that the above copyright notice appears in all copies and that  *
 * both that copyright notice and this permission notice appear in          *
 * supporting documentation, and that the name of Sequent not be used       *
 * in advertising or publicity pertaining to distribution or use of the     *
 * software without specific, written prior permission.                     *
 *                                                                          *
 * SEQUENT DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING *
 * ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS; IN NO EVENT SHALL *
 * SEQUENT BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR  *
 * ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,      *
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,   *
 * ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS      *
 * SOFTWARE.                                                                *
 ****************************************************************************/

#include "Xstlib.h"


#define CLIENT 0
static TestType test_type = SETUP;

/* 
   intent:	 send an OpenDisplay request to the server and check
                 that the server sent an OpenDisplay reply back
   input:	 
   output:	 none
   global input: 
   side effects: 
   methods:	 
*/

static
void
tester()
{
	int status = Create_Client_Tested(CLIENT, test_type);

	if(!status) {
		switch(test_type) {
		case OPEN_DISPLAY:
			Log_Err("Expected connection to fail, but it didn't");
			break;
		default:
			Log_Err("Expected connection not to fail, but it did");
			break;
		}
		Exit();
	}

	Exit_OK();
}
>>ASSERTION Good A
When a client sends a valid xname protocol request to the X server,
then the X server sends back a valid authorisation-accepted 
reply to the client.
>>STRATEGY
Call library function testfunc() to do the following:
Open a connection to the X server using native byte sex.
Send a valid xname protocol request to the X server.
Verify that the X server sends back a valid authorisation-accepted reply.
Open a connection to the X server using reversed byte sex.
Send a valid xname protocol request to the X server.
Verify that the X server sends back a valid authorisation-accepted reply.
>>CODE

	test_type = GOOD;

	/* Call a library function to exercise the test code */
	testfunc(tester);

>>ASSERTION Bad B 1
>># Assertion changed, 11th March 1992, to expect other than valid
>>#	auth.-accepted reply. It's wrong to expect a valid auth.-fail
>>#	reply as if byte order unknown then the 2-byte major & minor
>>#	protocol versions can't be sent properly: client byte-sex is
>>#	unknown to the server. We therefore will accept anything but
>>#	a valid reply: nothing; connection closed or reply with first
>>#	byte False (0). See bug report 0223.
>># This was not noticed at initial assertion review time.
When a client sends an invalid xname protocol request to the X server,
in which the 
.M byte_order
is not 0x42 or 0x6C,
then the X server does not
send back a valid authorisation-accepted reply to the client.
>>STRATEGY
Call library function testfunc() to do the following:
Open a connection to the X server.
Send an invalid xname protocol request to the X server with byte_order
  other than 0x42 (and not 0x6C, either).
Verify that the X server does not send back a valid authorisation-accepted reply.
Open a connection to the X server.
Send an invalid xname protocol request to the X server with byte_order
  other than 0x6C (and not 0x42, either).
Verify that the X server does not send back a valid authorisation-accepted reply.
>>CODE

	test_type = OPEN_DISPLAY;

	/* Call a library function to exercise the test code */
	testfunc(tester);

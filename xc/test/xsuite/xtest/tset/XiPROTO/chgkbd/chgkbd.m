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
 * Copyright 1993 by the Hewlett-Packard Company.
 * Copyright 1990, 1991 by UniSoft Group Limited.
 * 
 * Permission to use, copy, modify, distribute, and sell this software and
 * its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation, and that the names of HP, and UniSoft not be
 * used in advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.  HP, and UniSoft
 * make no representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 *
 * $XConsortium: chgkbd.m,v 1.5 94/04/17 21:13:25 rws Exp $
 */
>>TITLE ChangeKeyboardDevice XIPROTO
>>SET startup protostartup
>>SET cleanup protocleanup
>>EXTERN
/* Touch test for ChangeKeyboardDevice request */

/****************************************************************************
 * Copyright 1988 by Sequent Computer Systems, Inc., Portland, Oregon       *
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
extern int XInputMajorOpcode;
static TestType test_type = SETUP;
xReq *freq;
xGetInputFocusReply *frep;
xChangeKeyboardDeviceReq *req;
xChangeKeyboardDeviceReply *rep;
xEvent *ev;
xSelectExtensionEventReq *slctreq;
extern ExtDeviceInfo Devs;

static
void
tester()
{
extern int SavID;
int chdvnot;
XEventClass chdvnotcl;

	Create_Client(CLIENT);
	Create_Default_Window(CLIENT);
	if (!Setup_Extension_DeviceInfo(KeyMask))
	    {
	    Log_Err("Required extension devices are not present\n");
	    UNTESTED;
	    return;
	    }
	ChangeDeviceNotify (Devs.Key, chdvnot, chdvnotcl);

	slctreq = (xSelectExtensionEventReq *) Make_XInput_Req(CLIENT, X_SelectExtensionEvent);
	Send_XInput_Req(CLIENT, (xReq *) slctreq);
	Log_Trace("client %d sent default SelectExtensionEvent request\n", CLIENT);
	freq = (xReq *) Make_Req(CLIENT, X_GetInputFocus);
	Send_Req(CLIENT, freq);
	Log_Trace("client %d sent default GetInputFocus request\n", CLIENT);
	if ((frep = (xGetInputFocusReply *) Expect_Ext_Reply(CLIENT, X_GetInputFocus, XInputMajorOpcode)) == NULL) {
	    Log_Err("client %d failed to receive GetInputFocus reply\n", CLIENT);
	    Exit();
	    }  else  {
	    Log_Trace("client %d received GetInputFocus reply\n", CLIENT);
	    /* do any reply checking here */
	    Free_Reply(rep);
	    }

	Set_Test_Type(CLIENT, test_type);
	req = (xChangeKeyboardDeviceReq *) Make_XInput_Req(CLIENT, X_ChangeKeyboardDevice);
	Send_XInput_Req(CLIENT, (xReq *) req);
	Set_Test_Type(CLIENT, GOOD);
	switch(test_type) {
	case GOOD:
		Log_Trace("client %d sent default ChangeKeyboardDevice request\n", CLIENT);
		if ((ev = (xEvent *) Expect_Event(CLIENT, chdvnot)) == NULL) {
			Log_Err("client %d failed to receive ChangeDeviceNotify\n", CLIENT);
			Exit();
		}  else  {
			Log_Trace("client %d received ChangeDeviceNotify\n", CLIENT);
			Free_Event(ev);
		}
		if ((ev = (xEvent *) Expect_Event(CLIENT, MappingNotify)) == NULL) {
			Log_Err("client %d failed to receive MappingNotify\n", CLIENT);
			Exit();
		}  else  {
			Log_Trace("client %d received MappingNotify\n", CLIENT);
			Free_Event(ev);
		}
		if ((rep = (xChangeKeyboardDeviceReply *) Expect_Ext_Reply(CLIENT, X_ChangeKeyboardDevice, XInputMajorOpcode)) == NULL) {
			Log_Err("client %d failed to receive ChangeKeyboardDevice reply\n", CLIENT);
			Exit();
		}  else  {
			Log_Trace("client %d received ChangeKeyboardDevice reply\n", CLIENT);
			/* do any reply checking here */
			Free_Reply(rep);
		}
		/*(void) Expect_Nothing(CLIENT);*/
		
		/* Now restore the keyboard */
		/* Note we don't select changedevicenotify this time. */
		Set_Test_Type(CLIENT, test_type);

		req = (xChangeKeyboardDeviceReq *) Make_XInput_Req(CLIENT, X_ChangeKeyboardDevice);
		req->deviceid = SavID;
		Send_XInput_Req(CLIENT, (xReq *) req);
		Set_Test_Type(CLIENT, GOOD);
		Log_Trace("client %d sent default ChangeKeyboardDevice request\n", CLIENT);
		if ((ev = (xEvent *) Expect_Event(CLIENT, MappingNotify)) == NULL) {
			Log_Err("client %d failed to receive MappingNotify\n", CLIENT);
			Exit();
		}  else  {
			Log_Trace("client %d received MappingNotify\n", CLIENT);
			Free_Event(ev);
		}
		if ((rep = (xChangeKeyboardDeviceReply *) Expect_Ext_Reply(CLIENT, X_ChangeKeyboardDevice, XInputMajorOpcode)) == NULL) {
			Log_Err("client %d failed to receive ChangeKeyboardDevice reply\n", CLIENT);
			Exit();
		}  else  {
			Log_Trace("client %d received ChangeKeyboardDevice reply\n", CLIENT);
			/* do any reply checking here */
			Free_Reply(rep);
		}
		(void) Expect_Nothing(CLIENT);
		break;
	case BAD_LENGTH:
		Log_Trace("client %d sent ChangeKeyboardDevice request with bad length (%d)\n", CLIENT, req->length);
		(void) Expect_BadLength(CLIENT);
		(void) Expect_Nothing(CLIENT);
		break;
	case TOO_LONG:
	case JUST_TOO_LONG:
		Log_Trace("client %d sent overlong ChangeKeyboardDevice request (%d)\n", CLIENT, req->length);
		(void) Expect_BadLength(CLIENT);
		(void) Expect_Nothing(CLIENT);
		break;
	default:
		Log_Err("INTERNAL ERROR: test_type %d not one of GOOD(%d), BAD_LENGTH(%d), TOO_LONG(%d) or JUST_TOO_LONG(%d)\n",
			test_type, GOOD, BAD_LENGTH, TOO_LONG, JUST_TOO_LONG);
		Abort();
		/*NOTREACHED*/
		break;
	}
	Free_Req(req);
	Exit_OK();
}
>>ASSERTION Good A
When a client with native byte sex sends a valid xname protocol request to the 
X server, and the client has selected ChangeDeviceNotify events,
then the X server sends back a ChangeDeviceNotify event to the client,
followed by a MappingNotify event, followed by a reply
with the minimum required length.
>>STRATEGY
Call library function testfunc() to do the following:
Open a connection to the X server using native byte sex.
Send a valid xname protocol request to the X server.
Verify that the X server sends back an event.
>>CODE

	test_type = GOOD;
	Set_Required_Byte_Sex(SEX_NATIVE);

	/* Call a library function to exercise the test code */
	testfunc(tester);
	Set_Required_Byte_Sex(SEX_BOTH);

>>ASSERTION Good A
When a client with reversed byte sex sends a valid xname protocol request to 
the X server, and the client has selected ChangeDeviceNotify events,
then the X server sends back a ChangeDeviceNotify event to the client,
followed by a MappingNotify event, followed by a reply
with the minimum required length.
>>STRATEGY
Call library function testfunc() to do the following:
Open a connection to the X server using reversed byte sex.
Send a valid xname protocol request to the X server.
Verify that the X server sends back an event.
>>CODE

	Set_Required_Byte_Sex(SEX_REVERSE);
	test_type = GOOD;

	/* Call a library function to exercise the test code */
	testfunc(tester);
	Set_Required_Byte_Sex(SEX_BOTH);

>>ASSERTION Bad A
When a client sends an invalid xname protocol request to the X server,
in which the length field of the request is not the minimum length required to 
contain the request,
then the X server sends back a BadLength error to the client.
>>STRATEGY
Call library function testfunc() to do the following:
Open a connection to the X server using native byte sex.
Send an invalid xname protocol request to the X server with length 
  one less than the minimum length required to contain the request.
Verify that the X server sends back a BadLength error.
Open a connection to the X server using reversed byte sex.
Send an invalid xname protocol request to the X server with length 
  one less than the minimum length required to contain the request.
Verify that the X server sends back a BadLength error.

Open a connection to the X server using native byte sex.
Send an invalid xname protocol request to the X server with length 
  one greater than the minimum length required to contain the request.
Verify that the X server sends back a BadLength error.
Open a connection to the X server using reversed byte sex.
Send an invalid xname protocol request to the X server with length 
  one greater than the minimum length required to contain the request.
Verify that the X server sends back a BadLength error.
>>CODE

	test_type = BAD_LENGTH; /* < minimum */

	/* Call a library function to exercise the test code */
	testfunc(tester);

	test_type = JUST_TOO_LONG; /* > minimum */

	/* Call a library function to exercise the test code */
	testfunc(tester);

>>ASSERTION Bad A
When a client sends an invalid xname protocol request to the X server,
in which the length field of the request exceeds the maximum length accepted
by the X server,
then the X server sends back a BadLength error to the client.
>>STRATEGY
Call library function testfunc() to do the following:
Open a connection to the X server using native byte sex.
Send an invalid xname protocol request to the X server with length 
  one greater than the maximum length accepted by the server.
Verify that the X server sends back a BadLength error.
Open a connection to the X server using reversed byte sex.
Send an invalid xname protocol request to the X server with length 
  one greater than the maximum length accepted by the server.
Verify that the X server sends back a BadLength error.
>>CODE

	test_type = TOO_LONG;

	/* Call a library function to exercise the test code */
	testfunc(tester);

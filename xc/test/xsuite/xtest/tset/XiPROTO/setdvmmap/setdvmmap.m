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
 * $XConsortium: setdvmmap.m,v 1.9 94/04/17 21:13:31 rws Exp $
 */
>>TITLE SetDeviceModifierMapping XIPROTO
>>SET startup protostartup
>>SET cleanup protocleanup
>>EXTERN
/* Touch test for SetDeviceModifierMapping request */

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
xReq *gmmr;
xGetDeviceModifierMappingReply *gmmrep;
xSetDeviceModifierMappingReq *req;
xSetDeviceModifierMappingReply *rep;
extern ExtDeviceInfo Devs;
xSelectExtensionEventReq *slctreq;
xEvent *ev;

static
void
tester()
{
int dmn;
XEventClass dmnc;

	Create_Client(CLIENT);
	Create_Default_Window(CLIENT);
	if (!Setup_Extension_DeviceInfo(ModMask))
	    {
	    Log_Err("Required extension devices are not present\n");
	    UNTESTED;
	    return;
	    }

	DeviceMappingNotify(Devs.Mod, dmn, dmnc);
	slctreq = (xSelectExtensionEventReq *) Make_XInput_Req(CLIENT, X_SelectExtensionEvent);
	Send_XInput_Req(CLIENT, (xReq *) slctreq);
	Log_Trace("client %d sent default SelectExtensionEvent request\n", CLIENT);

	/* Get the existing mapping... */

	gmmr = (xReq *) Make_XInput_Req(CLIENT, X_GetDeviceModifierMapping);
	Send_XInput_Req(CLIENT, (xReq *) gmmr);
	Log_Trace("client %d sent default GetDeviceModifierMapping request\n", CLIENT);

	if ((gmmrep = (xGetDeviceModifierMappingReply *) Expect_Ext_Reply(CLIENT, X_GetDeviceModifierMapping, XInputMajorOpcode)) == NULL) {
		Log_Err("client %d failed to receive GetDeviceModifierMapping reply\n", CLIENT);
		Exit();
	}  else  {
		Log_Trace("client %d received GetDeviceModifierMapping reply\n", CLIENT);
		/* do any reply checking here */
	}
	(void) Expect_Nothing(CLIENT);
	Free_Req(gmmr);

	/* ... then make a request to duplicate that mapping. */

	Set_Test_Type(CLIENT, test_type);
	req = (xSetDeviceModifierMappingReq *) Make_XInput_Req(CLIENT, X_SetDeviceModifierMapping);
	if (test_type == GOOD)
	{
	req->numKeyPerModifier = gmmrep->numKeyPerModifier;
	req->length += 2*req->numKeyPerModifier;
	req = (xSetDeviceModifierMappingReq *) Xstrealloc((char *) req, req->length<<2);
	{
	  unsigned char *p, *q, *r;

	  p = (unsigned char *) (gmmrep + 1);
	  q = (unsigned char *) (req + 1);
	  r = ((unsigned char *)req) + (int)(req->length<<2);
	  while (q < r)
	    *q++ = *p++;
	}
	}

	Send_XInput_Req(CLIENT, (xReq *) req);
	Set_Test_Type(CLIENT, GOOD);
	switch(test_type) {
	case GOOD:
		Log_Trace("client %d sent default SetDeviceModifierMapping request\n", CLIENT);
		if ((ev = (xEvent *) Expect_Event(CLIENT, dmn)) == NULL) {
			Log_Err("client %d failed to receive DeviceMappingNotify\n", CLIENT);
			Exit();
		}  else  {
			Log_Trace("client %d received DeviceMappingNotify\n", CLIENT);
			/* do any event checking here */
			Free_Event(ev);
		}
		if ((rep = (xSetDeviceModifierMappingReply *) Expect_Ext_Reply(CLIENT, X_SetDeviceModifierMapping, XInputMajorOpcode)) == NULL) {
			Log_Err("client %d failed to receive SetDeviceModifierMapping reply\n", CLIENT);
			Exit();
		}  else  {
			Log_Trace("client %d received SetDeviceModifierMapping reply\n", CLIENT);
			/* do any reply checking here */
			Free_Reply(rep);
		}
		(void) Expect_Nothing(CLIENT);
		break;
	case BAD_LENGTH:
		Log_Trace("client %d sent SetDeviceModifierMapping request with bad length (%d)\n", CLIENT, req->length);
		(void) Expect_BadLength(CLIENT);
		(void) Expect_Nothing(CLIENT);
		break;
	case TOO_LONG:
	case JUST_TOO_LONG:
		Log_Trace("client %d sent overlong SetDeviceModifierMapping request (%d)\n", CLIENT, req->length);
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
When a client sends a valid xname protocol request to the X server,
then the X server sends back a reply and a 
.S DeviceMappingNotify 
event to the client.
>>STRATEGY
Call library function testfunc() to do the following:
Open a connection to the X server using native byte sex.
Send a valid xname protocol request to the X server.
Verify that the X server sends back a reply and a DeviceMappingNotify event.
Open a connection to the X server using reversed byte sex.
Send a valid xname protocol request to the X server.
Verify that the X server sends back a reply and a DeviceMappingNotify event.
>>CODE

	test_type = GOOD;

	/* Call a library function to exercise the test code */
	testfunc(tester);

>>ASSERTION Bad A
When a client sends an invalid xname protocol request to the X server,
in which the length field of the request is not the minimum length required to 
contain the request
plus zero or more keycode groups,
then the X server sends back a BadLength error to the client.
>>STRATEGY
Call library function testfunc() to do the following:
Open a connection to the X server using native byte sex.
Send an invalid xname protocol request to the X server with length 
  one less than the minimum length required to contain the request + zero keycode groups.
Verify that the X server sends back a BadLength error.
Open a connection to the X server using reversed byte sex.
Send an invalid xname protocol request to the X server with length 
  one less than the minimum length required to contain the request + zero keycode groups.
Verify that the X server sends back a BadLength error.

Open a connection to the X server using native byte sex.
Send an invalid xname protocol request to the X server with length 
  one greater than the minimum length required to contain the request + n keycode groups.
Verify that the X server sends back a BadLength error.
Open a connection to the X server using reversed byte sex.
Send an invalid xname protocol request to the X server with length 
  one greater than the minimum length required to contain the request + n keycode groups.
Verify that the X server sends back a BadLength error.
>>CODE

	test_type = BAD_LENGTH; /* < minimum */

	/* Call a library function to exercise the test code */
	testfunc(tester);

	test_type = JUST_TOO_LONG; /* > minimum + n keycode groups */

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

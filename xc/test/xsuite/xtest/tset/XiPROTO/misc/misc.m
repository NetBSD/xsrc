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
 * $XConsortium: misc.m,v 1.7 94/09/08 17:11:02 dpw Exp $
 */
>>TITLE Miscellaneous XIPROTO
>>SET startup protostartup
>>SET cleanup protocleanup
>>EXTERN

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
static TestType test_type = SETUP;
extern ExtDeviceInfo Devs;
extern int MinKeyCode;

static
void
tester()
{
Display *dpy2, *opendisplay();
Window w;
xSetDeviceFocusReq *freq;
xSelectExtensionEventReq *req;
XDevice *dev;

	Create_Client(CLIENT);
	(void) Create_Default_Window(CLIENT);

	dpy2 = opendisplay();
	if (dpy2 == (Display *) NULL) {
		delete("Can not open display");
		return;
	}
/* Create window with client1. */
	w = Get_Root_Id (CLIENT);

	Set_Test_Type(CLIENT, test_type);
	req = (xSelectExtensionEventReq *) Make_XInput_Req(CLIENT, X_SelectExtensionEvent);
	req->window = w;

	Send_XInput_Req(CLIENT, (xReq *) req);
	Set_Test_Type(CLIENT, GOOD);
	switch(test_type) {
	case GOOD:
		Log_Trace("client %d sent default SelectExtensionEvent request\n", CLIENT);
		(void) Expect_Nothing(CLIENT);
		if (Devs.Key){
		    dev = Devs.Key;
		    devicekeypress (dpy2, dev, MinKeyCode);
		    devicekeyrel (dpy2, dev, MinKeyCode);
		    }
		if (Devs.Button){
		    dev = Devs.Button;
		    devicebuttonpress (dpy2, dev, Button1);
		    devicebuttonrel (dpy2, dev, Button1);
		    }
		if (Devs.Valuator){
		    int axes = 0;
		    dev = Devs.Valuator;
		    SimulateDeviceMotionEvent (dpy2, dev, True, 1, &axes, 0);
		    }
		freq = (xSetDeviceFocusReq *) Make_XInput_Req(CLIENT, X_SetDeviceFocus);
		freq->device = dev->device_id;
		Send_XInput_Req(CLIENT, (xReq *) freq);
		XSync(dpy2,0);
		break;
	case BAD_LENGTH:
		Log_Trace("client %d sent SelectExtensionEvent request with bad length (%d)\n", CLIENT, req->length);
		(void) Expect_BadLength(CLIENT);
		(void) Expect_Nothing(CLIENT);
		break;
	case TOO_LONG:
	case JUST_TOO_LONG:
		Log_Trace("client %d sent overlong SelectExtensionEvent request (%d)\n", CLIENT, req->length);
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
Touch test for event byteswapping code.
>>STRATEGY
Select all events.
Generate them if possible.
>>CODE

	test_type = GOOD;
	if (noext(0))
	    return;
	if (!Setup_Extension_DeviceInfo(KeyMask))
	    untested("Can't fully execute %s because required key device not present",TestName);
	else {
	    /* Call a library function to exercise the test code */
	    testfunc(tester);
	}
	if (!Setup_Extension_DeviceInfo(BtnMask))
	    untested("Can't fully execute %s because required button device not present",TestName);
	else {
	    /* Call a library function to exercise the test code */
	    testfunc(tester);
	}
	if (!Setup_Extension_DeviceInfo(ValMask))
	    untested("Can't fully execute %s because required valuator device not present",TestName);
	else {
	    /* Call a library function to exercise the test code */
	    testfunc(tester);
	}

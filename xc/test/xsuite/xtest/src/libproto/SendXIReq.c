
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

 * Copyright 1990, 1991 by UniSoft Group Limited.
 * 
 * Copyright 1993 by the Hewlett-Packard Company.
 *
 * Permission to use, copy, modify, distribute, and sell this software and
 * its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation, and that the names of HP,  and UniSoft not be
 * used in advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.  HP,  and UniSoft
 * make no representations about the suitability of this software for any
 . purpose.  It is provided "as is" without express or implied warranty.
 *
 * $XConsortium: SendXIReq.c,v 1.10 94/04/17 21:01:24 rws Exp $
 */
/*
 * ***************************************************************************
 *  Copyright 1988 by Sequent Computer Systems, Inc., Portland, Oregon       *
 *                                                                           *
 *                                                                           *
 *                          All Rights Reserved                              *
 *                                                                           *
 *  Permission to use, copy, modify, and distribute this software and its    *
 *  documentation for any purpose and without fee is hereby granted,         *
 *  provided that the above copyright notice appears in all copies and that  *
 *  both that copyright notice and this permission notice appear in          *
 *  supporting documentation, and that the name of Sequent not be used       *
 *  in advertising or publicity pertaining to distribution or use of the     *
 *  software without specific, written prior permission.                     *
 *                                                                           *
 *  SEQUENT DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING *
 *  ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL *
 *  SEQUENT BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR  *
 *  ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,      *
 *  WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,   *
 *  ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS      *
 *  SOFTWARE.                                                                *
 * ***************************************************************************
 */

/*
 *	$Header: /cvsroot/xsrc/xc/test/xsuite/xtest/src/libproto/Attic/SendXIReq.c,v 1.1.1.1 1997/03/15 06:19:56 scottr Exp $
 */

#ifndef lint
static char rcsid[]="$Header: /cvsroot/xsrc/xc/test/xsuite/xtest/src/libproto/Attic/SendXIReq.c,v 1.1.1.1 1997/03/15 06:19:56 scottr Exp $";
#endif

#include "XstlibInt.h"
#include <X11/extensions/XI.h>
#include <X11/extensions/XIproto.h>
#include "DataMove.h"

void _Send_XInput_Req();

void
Send_XInput_Req(client,rp)
xReq *rp;
int client;
{
    _Send_XInput_Req(client,rp,0);	/* not polling thru this entry point */
}

void
_Send_XInput_Req(client,rp,pollreq)
int client;
xReq *rp;
int pollreq;
{
	XstDisplay *dpy = Get_Display(client);
	unsigned long oldlen = rp->length;
	long bytes_out = oldlen << 2;
	long n;

	switch (Get_Test_Type(client)) {
	case OPEN_DISPLAY:
	case SETUP:
	case GOOD:
	    /* nothing */
	    break;
	case TOO_LONG: /* munge length to be enormous... */
	    {
		unsigned long newlen = Get_Max_Request(client) + 1;

		rp->length = newlen;
		bytes_out = (newlen << 2);
	    }
	    break;
	}
	if (!pollreq) {
	    Log_Debug("Sending:");
	    Show_Req(rp);
	}

	if (!pollreq) {
	    Xst_clients[client].cl_reqtype = rp->reqType;/* major opcode */
	    Xst_clients[client].cl_minor = rp->data;	 /* minor opcode */
	}

	dpy->request++;

	switch (((xOpenDeviceReq *)rp)->ReqType) {
	case X_GetExtensionVersion:
		send1(client,(long) ((xGetExtensionVersionReq *)rp)->reqType);
		send1(client,(long) ((xGetExtensionVersionReq *)rp)->ReqType);
		send2(client,(short) ((xGetExtensionVersionReq *)rp)->length);
		send2(client,(short) ((xGetExtensionVersionReq *)rp)->nbytes);
		send1(client,(long) ((xGetExtensionVersionReq *)rp)->pad1);
		send1(client,(long) ((xGetExtensionVersionReq *)rp)->pad2);
		break;
	case X_ListInputDevices:
		send1(client,(long) ((xListInputDevicesReq *)rp)->reqType);
		send1(client,(long) ((xListInputDevicesReq *)rp)->ReqType);
		send2(client,(short) ((xListInputDevicesReq *)rp)->length);
		break;
	case X_OpenDevice:
		send1(client,(long) ((xOpenDeviceReq *)rp)->reqType);
		send1(client,(long) ((xOpenDeviceReq *)rp)->ReqType);
		send2(client,(short) ((xOpenDeviceReq *)rp)->length);
		send1(client,(long) ((xOpenDeviceReq *)rp)->deviceid);
		send1(client,(long) ((xOpenDeviceReq *)rp)->pad1);
		send1(client,(long) ((xOpenDeviceReq *)rp)->pad2);
		send1(client,(long) ((xOpenDeviceReq *)rp)->pad3);
		break;
	case X_CloseDevice:
		send1(client,(long) ((xCloseDeviceReq *)rp)->reqType);
		send1(client,(long) ((xCloseDeviceReq *)rp)->ReqType);
		send2(client,(short) ((xCloseDeviceReq *)rp)->length);
		send1(client,(long) ((xCloseDeviceReq *)rp)->deviceid);
		send1(client,(long) ((xCloseDeviceReq *)rp)->pad1);
		send1(client,(long) ((xCloseDeviceReq *)rp)->pad2);
		send1(client,(long) ((xCloseDeviceReq *)rp)->pad3);
		break;
	case X_SetDeviceMode:
		send1(client,(long) ((xSetDeviceModeReq *)rp)->reqType);
		send1(client,(long) ((xSetDeviceModeReq *)rp)->ReqType);
		send2(client,(short) ((xSetDeviceModeReq *)rp)->length);
		send1(client,(long) ((xSetDeviceModeReq *)rp)->deviceid);
		send1(client,(long) ((xSetDeviceModeReq *)rp)->mode);
		send1(client,(long) ((xSetDeviceModeReq *)rp)->pad1);
		send1(client,(long) ((xSetDeviceModeReq *)rp)->pad2);
		break;
	case X_SelectExtensionEvent:
		{
		int i, *cptr;
		send1(client,(long) ((xSelectExtensionEventReq *)rp)->reqType);
		send1(client,(long) ((xSelectExtensionEventReq *)rp)->ReqType);
		send2(client,(short) ((xSelectExtensionEventReq *)rp)->length);
		send4(client,(long) ((xSelectExtensionEventReq *)rp)->window);
		send2(client,(short) ((xSelectExtensionEventReq *)rp)->count);
		send2(client,(short) ((xSelectExtensionEventReq *)rp)->pad00);
		cptr = (int *)((xSelectExtensionEventReq *)rp+1);
		for (i=0; i<((xSelectExtensionEventReq *)rp)->count; i++)
		    send4(client,(long) *cptr++);
		}
		break;
	case X_GetSelectedExtensionEvents:
		send1(client,(long) 
		    ((xGetSelectedExtensionEventsReq *)rp)->reqType);
		send1(client,(long) 
		    ((xGetSelectedExtensionEventsReq *)rp)->ReqType);
		send2(client,(short) 
		    ((xGetSelectedExtensionEventsReq *)rp)->length);
		send4(client,(long) 
		    ((xGetSelectedExtensionEventsReq *)rp)->window);
		break;
	case X_ChangeDeviceDontPropagateList:
		send1(client,(long) 
		    ((xChangeDeviceDontPropagateListReq *)rp)->reqType);
		send1(client,(long) 
		    ((xChangeDeviceDontPropagateListReq *)rp)->ReqType);
		send2(client,(short) 
		    ((xChangeDeviceDontPropagateListReq *)rp)->length);
		send4(client,(long) 
		    ((xChangeDeviceDontPropagateListReq *)rp)->window);
		send2(client,(short) 
		    ((xChangeDeviceDontPropagateListReq *)rp)->count);
		send1(client,(long) 
		    ((xChangeDeviceDontPropagateListReq *)rp)->mode);
		send1(client,(long) 
		    ((xChangeDeviceDontPropagateListReq *)rp)->pad);
		send4(client,(long) 
		    *((int *)((xChangeDeviceDontPropagateListReq *)rp+1)));
		break;
	case X_GetDeviceDontPropagateList:
		send1(client,(long) 
		    ((xGetDeviceDontPropagateListReq *)rp)->reqType);
		send1(client,(long) 
		    ((xGetDeviceDontPropagateListReq *)rp)->ReqType);
		send2(client,(short) 
		    ((xGetDeviceDontPropagateListReq *)rp)->length);
		send4(client,(long) 
		    ((xGetDeviceDontPropagateListReq *)rp)->window);
		break;
	case X_GetDeviceMotionEvents:
		send1(client,(long) 
		    ((xGetDeviceMotionEventsReq *)rp)->reqType);
		send1(client,(long) 
		    ((xGetDeviceMotionEventsReq *)rp)->ReqType);
		send2(client,(short) 
		    ((xGetDeviceMotionEventsReq *)rp)->length);
		send4(client,(long) 
		    ((xGetDeviceMotionEventsReq *)rp)->start);
		send4(client,(long) 
		    ((xGetDeviceMotionEventsReq *)rp)->stop);
		send1(client,(long) 
		    ((xGetDeviceMotionEventsReq *)rp)->deviceid);
		send1(client,(long) 
		    ((xGetDeviceMotionEventsReq *)rp)->pad1);
		send1(client,(long) 
		    ((xGetDeviceMotionEventsReq *)rp)->pad2);
		send1(client,(long) 
		    ((xGetDeviceMotionEventsReq *)rp)->pad3);
		break;
	case X_ChangeKeyboardDevice:
		send1(client,(long) ((xChangeKeyboardDeviceReq *)rp)->reqType);
		send1(client,(long) ((xChangeKeyboardDeviceReq *)rp)->ReqType);
		send2(client,(short) ((xChangeKeyboardDeviceReq *)rp)->length);
		send1(client,(long) ((xChangeKeyboardDeviceReq *)rp)->deviceid);
		send1(client,(long) ((xChangeKeyboardDeviceReq *)rp)->pad1);
		send1(client,(long) ((xChangeKeyboardDeviceReq *)rp)->pad2);
		send1(client,(long) ((xChangeKeyboardDeviceReq *)rp)->pad3);
		break;
	case X_ChangePointerDevice:
		send1(client,(long) ((xChangePointerDeviceReq *)rp)->reqType);
		send1(client,(long) ((xChangePointerDeviceReq *)rp)->ReqType);
		send2(client,(short) ((xChangePointerDeviceReq *)rp)->length);
		send1(client,(long) ((xChangePointerDeviceReq *)rp)->xaxis);
		send1(client,(long) ((xChangePointerDeviceReq *)rp)->yaxis);
		send1(client,(long) ((xChangePointerDeviceReq *)rp)->deviceid);
		send1(client,(long) ((xChangePointerDeviceReq *)rp)->pad1);
		break;
	case X_GrabDevice:
		{
		int i, *cptr;
		send1(client,(long) ((xGrabDeviceReq *)rp)->reqType);
		send1(client,(long) ((xGrabDeviceReq *)rp)->ReqType);
		send2(client,(short) ((xGrabDeviceReq *)rp)->length);
		send4(client,(long) ((xGrabDeviceReq *)rp)->grabWindow);
		send4(client,(long) ((xGrabDeviceReq *)rp)->time);
		send2(client,(short) ((xGrabDeviceReq *)rp)->event_count);
		send1(client,(long) ((xGrabDeviceReq *)rp)->this_device_mode);
		send1(client,(long) ((xGrabDeviceReq *)rp)->other_devices_mode);
		send1(client,(long) ((xGrabDeviceReq *)rp)->ownerEvents);
		send1(client,(long) ((xGrabDeviceReq *)rp)->deviceid);
		send2(client,(short) ((xGrabDeviceReq *)rp)->pad01);
		cptr = (int *)((xGrabDeviceReq *)rp+1);
		for (i=0; i<((xGrabDeviceReq *)rp)->event_count; i++)
		    send4(client,(long) *cptr++);
		}
		break;
	case X_UngrabDevice:
		send1(client,(long) ((xUngrabDeviceReq *)rp)->reqType);
		send1(client,(long) ((xUngrabDeviceReq *)rp)->ReqType);
		send2(client,(short) ((xUngrabDeviceReq *)rp)->length);
		send4(client,(long) ((xUngrabDeviceReq *)rp)->time);
		send1(client,(long) ((xUngrabDeviceReq *)rp)->deviceid);
		send1(client,(long) ((xUngrabDeviceReq *)rp)->pad1);
		send1(client,(long) ((xUngrabDeviceReq *)rp)->pad2);
		send1(client,(long) ((xUngrabDeviceReq *)rp)->pad3);
		break;
	case X_GrabDeviceKey:
		send1(client,(long) ((xGrabDeviceKeyReq *)rp)->reqType);
		send1(client,(long) ((xGrabDeviceKeyReq *)rp)->ReqType);
		send2(client,(short) ((xGrabDeviceKeyReq *)rp)->length);
		send4(client,(long) ((xGrabDeviceKeyReq *)rp)->grabWindow);
		send2(client,(short) ((xGrabDeviceKeyReq *)rp)->event_count);
		send2(client,(short) ((xGrabDeviceKeyReq *)rp)->modifiers);
		send1(client,(long) 
		    ((xGrabDeviceKeyReq *)rp)->modifier_device);
		send1(client,(long) 
		    ((xGrabDeviceKeyReq *)rp)->grabbed_device);
		send1(client,(long) ((xGrabDeviceKeyReq *)rp)->key);
		send1(client,(long) 
		    ((xGrabDeviceKeyReq *)rp)->this_device_mode);
		send1(client,(long) 
		    ((xGrabDeviceKeyReq *)rp)->other_devices_mode);
		send1(client,(long) ((xGrabDeviceKeyReq *)rp)->ownerEvents);
		send1(client,(long) ((xGrabDeviceKeyReq *)rp)->pad1);
		send1(client,(long) ((xGrabDeviceKeyReq *)rp)->pad2);
		send4(client,(long) *((int *)((xGrabDeviceKeyReq *)rp + 1)));
		break;
	case X_UngrabDeviceKey:
		send1(client,(long) ((xUngrabDeviceKeyReq *)rp)->reqType);
		send1(client,(long) ((xUngrabDeviceKeyReq *)rp)->ReqType);
		send2(client,(short) ((xUngrabDeviceKeyReq *)rp)->length);
		send4(client,(long) ((xUngrabDeviceKeyReq *)rp)->grabWindow);
		send2(client,(short) ((xUngrabDeviceKeyReq *)rp)->modifiers);
		send1(client,(long) 
		    ((xUngrabDeviceKeyReq *)rp)->modifier_device);
		send1(client,(long) ((xUngrabDeviceKeyReq *)rp)->key);
		send1(client,(long) 
		    ((xUngrabDeviceKeyReq *)rp)->grabbed_device);
		send1(client,(long) ((xUngrabDeviceKeyReq *)rp)->pad1);
		send1(client,(long) ((xUngrabDeviceKeyReq *)rp)->pad2);
		send1(client,(long) ((xUngrabDeviceKeyReq *)rp)->pad3);
		break;
	case X_GrabDeviceButton:
		send1(client,(long) ((xGrabDeviceButtonReq *)rp)->reqType);
		send1(client,(long) ((xGrabDeviceButtonReq *)rp)->ReqType);
		send2(client,(short) ((xGrabDeviceButtonReq *)rp)->length);
		send4(client,(long) ((xGrabDeviceButtonReq *)rp)->grabWindow);
		send1(client,(long) 
		    ((xGrabDeviceButtonReq *)rp)->grabbed_device);
		send1(client,(long) 
		    ((xGrabDeviceButtonReq *)rp)->modifier_device);
		send2(client,(short) ((xGrabDeviceButtonReq *)rp)->event_count);
		send2(client,(short) ((xGrabDeviceButtonReq *)rp)->modifiers);
		send1(client,(long) 
		    ((xGrabDeviceButtonReq *)rp)->this_device_mode);
		send1(client,(long) 
		    ((xGrabDeviceButtonReq *)rp)->other_devices_mode);
		send1(client,(long) ((xGrabDeviceButtonReq *)rp)->button);
		send1(client,(long) ((xGrabDeviceButtonReq *)rp)->ownerEvents);
		send1(client,(long) ((xGrabDeviceButtonReq *)rp)->pad1);
		send1(client,(long) ((xGrabDeviceButtonReq *)rp)->pad2);
		send4(client,(long) *((int *)((xGrabDeviceButtonReq *)rp + 1)));
		break;
	case X_UngrabDeviceButton:
		send1(client,(long) ((xUngrabDeviceButtonReq *)rp)->reqType);
		send1(client,(long) ((xUngrabDeviceButtonReq *)rp)->ReqType);
		send2(client,(short) ((xUngrabDeviceButtonReq *)rp)->length);
		send4(client,(long) ((xUngrabDeviceButtonReq *)rp)->grabWindow);
		send2(client,(short) ((xUngrabDeviceButtonReq *)rp)->modifiers);
		send1(client,(long) 
		    ((xUngrabDeviceButtonReq *)rp)->modifier_device);
		send1(client,(long) ((xUngrabDeviceButtonReq *)rp)->button);
		send1(client,(long) 
		    ((xUngrabDeviceButtonReq *)rp)->grabbed_device);
		send1(client,(long) ((xUngrabDeviceButtonReq *)rp)->pad1);
		send1(client,(long) ((xUngrabDeviceButtonReq *)rp)->pad2);
		send1(client,(long) ((xUngrabDeviceButtonReq *)rp)->pad3);
		break;
	case X_AllowDeviceEvents:
		send1(client,(long) ((xAllowDeviceEventsReq *)rp)->reqType);
		send1(client,(long) ((xAllowDeviceEventsReq *)rp)->ReqType);
		send2(client,(short) ((xAllowDeviceEventsReq *)rp)->length);
		send4(client,(long) ((xAllowDeviceEventsReq *)rp)->time);
		send1(client,(short) ((xAllowDeviceEventsReq *)rp)->mode);
		send1(client,(short) ((xAllowDeviceEventsReq *)rp)->deviceid);
		send1(client,(short) ((xAllowDeviceEventsReq *)rp)->pad1);
		send1(client,(short) ((xAllowDeviceEventsReq *)rp)->pad2);
		break;
	case X_GetDeviceFocus:
		send1(client,(long) ((xGetDeviceFocusReq *)rp)->reqType);
		send1(client,(long) ((xGetDeviceFocusReq *)rp)->ReqType);
		send2(client,(short) ((xGetDeviceFocusReq *)rp)->length);
		send1(client,(long) ((xGetDeviceFocusReq *)rp)->deviceid);
		send1(client,(long) ((xGetDeviceFocusReq *)rp)->pad1);
		send1(client,(long) ((xGetDeviceFocusReq *)rp)->pad2);
		send1(client,(long) ((xGetDeviceFocusReq *)rp)->pad3);
		break;
	case X_SetDeviceFocus:
		send1(client,(long) ((xSetDeviceFocusReq *)rp)->reqType);
		send1(client,(long) ((xSetDeviceFocusReq *)rp)->ReqType);
		send2(client,(short) ((xSetDeviceFocusReq *)rp)->length);
		send4(client,(long) ((xSetDeviceFocusReq *)rp)->focus);
		send4(client,(long) ((xSetDeviceFocusReq *)rp)->time);
		send1(client,(long) ((xSetDeviceFocusReq *)rp)->revertTo);
		send1(client,(long) ((xSetDeviceFocusReq *)rp)->device);
		send2(client,(short) ((xSetDeviceFocusReq *)rp)->pad01);
		break;
	case X_GetFeedbackControl:
		send1(client,(long) ((xGetFeedbackControlReq *)rp)->reqType);
		send1(client,(long) ((xGetFeedbackControlReq *)rp)->ReqType);
		send2(client,(short) ((xGetFeedbackControlReq *)rp)->length);
		send1(client,(long) ((xGetFeedbackControlReq *)rp)->deviceid);
		send1(client,(long) ((xGetFeedbackControlReq *)rp)->pad1);
		send1(client,(long) ((xGetFeedbackControlReq *)rp)->pad2);
		send1(client,(long) ((xGetFeedbackControlReq *)rp)->pad3);
		break;
	case X_ChangeFeedbackControl:
		{
		XID class;
		extern char *FeedbackData;
		send1(client,(long) ((xChangeFeedbackControlReq *)rp)->reqType);
		send1(client,(long) ((xChangeFeedbackControlReq *)rp)->ReqType);
		send2(client,(short) ((xChangeFeedbackControlReq *)rp)->length);
		send4(client,(short) ((xChangeFeedbackControlReq *)rp)->mask);
		send1(client,(long) 
		    ((xChangeFeedbackControlReq *)rp)->deviceid);
		send1(client,(long) 
		    ((xChangeFeedbackControlReq *)rp)->feedbackid);
		send1(client,(long) ((xChangeFeedbackControlReq *)rp)->pad1);
		send1(client,(long) ((xChangeFeedbackControlReq *)rp)->pad2);
		class = *((char *)((xChangeFeedbackControlReq *)rp+1));
		Send_Feedback_Control (client, class, (xChangeFeedbackControlReq *)rp+1);
		/*
		Send_Feedback_Control (client, class, FeedbackData);
		*/
		}
		break;
	case X_GetDeviceKeyMapping:
		send1(client,(long) ((xGetDeviceKeyMappingReq *)rp)->reqType);
		send1(client,(long) ((xGetDeviceKeyMappingReq *)rp)->ReqType);
		send2(client,(short) ((xGetDeviceKeyMappingReq *)rp)->length);
		send1(client,(long) ((xGetDeviceKeyMappingReq *)rp)->deviceid);
		send1(client,(long) 
		    ((xGetDeviceKeyMappingReq *)rp)->firstKeyCode);
		send1(client,(long) ((xGetDeviceKeyMappingReq *)rp)->count);
		send1(client,(long) ((xGetDeviceKeyMappingReq *)rp)->pad1);
		break;
	case X_ChangeDeviceKeyMapping:
		send1(client,(long) 
		    ((xChangeDeviceKeyMappingReq *)rp)->reqType);
		send1(client,(long) 
		    ((xChangeDeviceKeyMappingReq *)rp)->ReqType);
		send2(client,(short) 
		    ((xChangeDeviceKeyMappingReq *)rp)->length);
		send1(client,(long) 
		    ((xChangeDeviceKeyMappingReq *)rp)->deviceid);
		send1(client,(long) 
		    ((xChangeDeviceKeyMappingReq *)rp)->firstKeyCode);
		send1(client,(long) 
		    ((xChangeDeviceKeyMappingReq *)rp)->keySymsPerKeyCode);
		send1(client,(long) 
		    ((xChangeDeviceKeyMappingReq *)rp)->keyCodes);
		Send_Value_List(client, rp, sizeof (xChangeDeviceKeyMappingReq), 32);
		break;
	case X_GetDeviceModifierMapping:
		send1(client,(long) 
		    ((xGetDeviceModifierMappingReq *)rp)->reqType);
		send1(client,(long) 
		    ((xGetDeviceModifierMappingReq *)rp)->ReqType);
		send2(client,(short) 
		    ((xGetDeviceModifierMappingReq *)rp)->length);
		send1(client,(long) 
		    ((xGetDeviceModifierMappingReq *)rp)->deviceid);
		send1(client,(long) 
		    ((xGetDeviceModifierMappingReq *)rp)->pad1);
		send1(client,(long) 
		    ((xGetDeviceModifierMappingReq *)rp)->pad2);
		send1(client,(long) 
		    ((xGetDeviceModifierMappingReq *)rp)->pad3);
		break;
	case X_SetDeviceModifierMapping:
		send1(client,(long) 
		    ((xSetDeviceModifierMappingReq *)rp)->reqType);
		send1(client,(long) 
		    ((xSetDeviceModifierMappingReq *)rp)->ReqType);
		send2(client,(short) 
		    ((xSetDeviceModifierMappingReq *)rp)->length);
		send1(client,(long) 
		    ((xSetDeviceModifierMappingReq *)rp)->deviceid);
		send1(client,(long) 
		    ((xSetDeviceModifierMappingReq *)rp)->numKeyPerModifier);
		send2(client,(short) 
		    ((xSetDeviceModifierMappingReq *)rp)->pad1);
		Send_Value_List(client, rp, sizeof (xSetDeviceModifierMappingReq), 8);
		break;
	case X_GetDeviceButtonMapping:
		send1(client,(long) 
		    ((xGetDeviceButtonMappingReq *)rp)->reqType);
		send1(client,(long) 
		    ((xGetDeviceButtonMappingReq *)rp)->ReqType);
		send2(client,(short) 
		    ((xGetDeviceButtonMappingReq *)rp)->length);
		send1(client,(long) 
		    ((xGetDeviceButtonMappingReq *)rp)->deviceid);
		send1(client,(long) 
		    ((xGetDeviceButtonMappingReq *)rp)->pad1);
		send1(client,(long) 
		    ((xGetDeviceButtonMappingReq *)rp)->pad2);
		send1(client,(long) 
		    ((xGetDeviceButtonMappingReq *)rp)->pad3);
		break;
	case X_SetDeviceButtonMapping:
		send1(client,(long) 
		    ((xSetDeviceButtonMappingReq *)rp)->reqType);
		send1(client,(long) 
		    ((xSetDeviceButtonMappingReq *)rp)->ReqType);
		send2(client,(short) 
		    ((xSetDeviceButtonMappingReq *)rp)->length);
		send1(client,(long) 
		    ((xSetDeviceButtonMappingReq *)rp)->deviceid);
		send1(client,(long) 
		    ((xSetDeviceButtonMappingReq *)rp)->map_length);
		send1(client,(long) 
		    ((xSetDeviceButtonMappingReq *)rp)->pad1);
		send1(client,(long) 
		    ((xSetDeviceButtonMappingReq *)rp)->pad2);
		Send_Value_List(client, rp, sizeof (xSetDeviceButtonMappingReq), 8);
		break;
	case X_QueryDeviceState:
		send1(client,(long) ((xQueryDeviceStateReq *)rp)->reqType);
		send1(client,(long) ((xQueryDeviceStateReq *)rp)->ReqType);
		send2(client,(short) ((xQueryDeviceStateReq *)rp)->length);
		send1(client,(long) ((xQueryDeviceStateReq *)rp)->deviceid);
		send1(client,(long) ((xQueryDeviceStateReq *)rp)->pad1);
		send1(client,(long) ((xQueryDeviceStateReq *)rp)->pad2);
		send1(client,(long) ((xQueryDeviceStateReq *)rp)->pad3);
		break;
	case X_SendExtensionEvent:
		send1(client,(long) ((xSendExtensionEventReq *)rp)->reqType);
		send1(client,(long) ((xSendExtensionEventReq *)rp)->ReqType);
		send2(client,(short) ((xSendExtensionEventReq *)rp)->length);
		send4(client,(long) 
		    ((xSendExtensionEventReq *)rp)->destination);
		send1(client,(long) ((xSendExtensionEventReq *)rp)->deviceid);
		send1(client,(long) ((xSendExtensionEventReq *)rp)->propagate);
		send2(client,(short)((xSendExtensionEventReq *)rp)->count);
		send1(client,(long) ((xSendExtensionEventReq *)rp)->num_events);
		send1(client,(long) ((xSendExtensionEventReq *)rp)->pad1);
		send1(client,(long) ((xSendExtensionEventReq *)rp)->pad2);
		send1(client,(long) ((xSendExtensionEventReq *)rp)->pad3);
		Send_Extension_Event (client, 
		((xSendExtensionEventReq *)rp)->num_events,
		((xSendExtensionEventReq *)rp)->count,
		(xSendExtensionEventReq *)rp+1);
		break;
	case X_DeviceBell:
		send1(client,(long) ((xDeviceBellReq *)rp)->reqType);
		send1(client,(long) ((xDeviceBellReq *)rp)->ReqType);
		send2(client,(short) ((xDeviceBellReq *)rp)->length);
		send1(client,(long) ((xDeviceBellReq *)rp)->deviceid);
		send1(client,(long) ((xDeviceBellReq *)rp)->feedbackid);
		send1(client,(long) ((xDeviceBellReq *)rp)->feedbackclass);
		send1(client,(long) ((xDeviceBellReq *)rp)->percent);
		break;
	case X_SetDeviceValuators:
		send1(client,(long) ((xSetDeviceValuatorsReq *)rp)->reqType);
		send1(client,(long) ((xSetDeviceValuatorsReq *)rp)->ReqType);
		send2(client,(short) ((xSetDeviceValuatorsReq *)rp)->length);
		send1(client,(long) ((xSetDeviceValuatorsReq *)rp)->deviceid);
		send1(client,(long) 
		    ((xSetDeviceValuatorsReq *)rp)->first_valuator);
		send1(client,(long) 
		    ((xSetDeviceValuatorsReq *)rp)->num_valuators);
		send1(client,(long) 
		    ((xSetDeviceValuatorsReq *)rp)->pad1);
		Send_Value_List(client, rp, sizeof (xSetDeviceValuatorsReq), 32);
		break;
	case X_GetDeviceControl:
		send1(client,(long) ((xGetDeviceControlReq *)rp)->reqType);
		send1(client,(long) ((xGetDeviceControlReq *)rp)->ReqType);
		send2(client,(short) ((xGetDeviceControlReq *)rp)->length);
		send2(client,(short) ((xGetDeviceControlReq *)rp)->control);
		send1(client,(long) ((xGetDeviceControlReq *)rp)->deviceid);
		send1(client,(long) ((xGetDeviceControlReq *)rp)->pad2);
		break;
	case X_ChangeDeviceControl:
		send1(client,(long) ((xChangeDeviceControlReq *)rp)->reqType);
		send1(client,(long) ((xChangeDeviceControlReq *)rp)->ReqType);
		send2(client,(short)((xChangeDeviceControlReq *)rp)->length);
		send2(client,(short)((xChangeDeviceControlReq *)rp)->control);
		send1(client,(long) ((xChangeDeviceControlReq *)rp)->deviceid);
		send1(client,(long) ((xChangeDeviceControlReq *)rp)->pad0);
		Send_Value_List(client, rp, sizeof (xChangeDeviceControlReq), 8);
		break;
	default:
		DEFAULT_ERROR;
		break;
	}
	SendIt(client,bytes_out,oldlen);

}

Send_Feedback_Control (client, class, ptr)
    int client;
    XID class;
    char *ptr;
    {
    int i;
    xFeedbackCtl *f = (xFeedbackCtl *) ptr;

    send1(client,(long) f->class);
    send1(client,(long) f->id);
    send2(client,(short)f->length);

    switch (class)
	{
	case KbdFeedbackClass:
	    {
	    xKbdFeedbackCtl *K = (xKbdFeedbackCtl *) ptr;
	    send1(client,(long) K->key);
	    send1(client,(long) K->auto_repeat_mode);
	    send1(client,(long) K->click);
	    send1(client,(long) K->percent);
	    send2(client,(short) K->pitch);
	    send2(client,(short) K->duration);
	    send4(client,(long) K->led_mask);
	    send4(client,(long) K->led_values);
	    }
	    break;
	case PtrFeedbackClass:
	    {
	    xPtrFeedbackCtl *P = (xPtrFeedbackCtl *) ptr;
	    send1(client,(long) P->pad1);
	    send1(client,(long) P->pad2);
	    send2(client,(short) P->num);
	    send2(client,(short) P->denom);
	    send2(client,(short) P->thresh);
	    }
	    break;
	case StringFeedbackClass:
	    {
	    xStringFeedbackCtl *S = (xStringFeedbackCtl *) ptr;
	    KeySym *key = (KeySym *) (S+1);
	    send1(client,(long) S->pad1);
	    send1(client,(long) S->pad2);
	    send2(client,(short) S->num_keysyms);
	    for (i=0; i<S->num_keysyms; i++)
		send4(client,(long)*key++);
	    }
	    break;
	case IntegerFeedbackClass:
	    {
	    xIntegerFeedbackCtl *I = (xIntegerFeedbackCtl *) ptr;
	    send4(client,(long) I->int_to_display);
	    }
	    break;
	case BellFeedbackClass:
	    {
	    xBellFeedbackCtl *B = (xBellFeedbackCtl *) ptr;
	    send1(client,(long) B->percent);
	    send1(client,(long) B->pad1);
	    send1(client,(long) B->pad2);
	    send1(client,(long) B->pad3);
	    send2(client,(short) B->pitch);
	    send2(client,(short) B->duration);
	    }
	    break;
	case LedFeedbackClass:
	    {
	    xLedFeedbackCtl *L = (xLedFeedbackCtl *) ptr;
	    send4(client,(long) L->led_mask);
	    send4(client,(long) L->led_values);
	    }
	    break;
	default:
	    break;
	}
    }

Send_Extension_Event (client, num_ev, num_classes, ptr)
    int client;
    int num_ev, num_classes;
    char *ptr;
    {
    int i;
    int *cptr = (int *) ((xEvent *)ptr + num_ev);

    for (i=0; i<num_ev; i++, ptr+=32)
	if (i == 0)
	    {
	    deviceKeyButtonPointer *kev = (deviceKeyButtonPointer *) ptr;

	    send1(client,(long) kev->type);
	    send1(client,(long) kev->detail);
	    send2(client,(short)kev->sequenceNumber);
	    send4(client,(long) kev->time);
	    send4(client,(long) kev->root);
	    send4(client,(long) kev->event);
	    send4(client,(long) kev->child);
	    send2(client,(short)kev->root_x);
	    send2(client,(short)kev->root_y);
	    send2(client,(short)kev->event_x);
	    send2(client,(short)kev->event_y);
	    send2(client,(short)kev->state);
	    send1(client,(long) kev->same_screen);
	    send1(client,(long) kev->deviceid);
	    }
	else
	    {
	    deviceValuator *vev = (deviceValuator *) ptr;
	    send1(client,(long) vev->type);
	    send1(client,(long) vev->deviceid);
	    send2(client,(short)vev->sequenceNumber);
	    send2(client,(short)vev->device_state);
	    send1(client,(long) vev->num_valuators);
	    send1(client,(long) vev->first_valuator);
	    send4(client,(long) vev->valuator0);
	    send4(client,(long) vev->valuator1);
	    send4(client,(long) vev->valuator2);
	    send4(client,(long) vev->valuator3);
	    send4(client,(long) vev->valuator4);
	    send4(client,(long) vev->valuator5);
	    }
    for (i=0; i<num_classes; i++)
	send4(client,(long) *cptr++);
    }

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

 * Copyright 1990, 1991 and UniSoft Group Limited.
 * 
 * Copyright 1993 by the Hewlett-Packard Company.
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
 * $XConsortium: ShowExtReq.c,v 1.4 94/04/17 21:01:28 rws Exp $
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
 *	$Header: /cvsroot/xsrc/xc/test/xsuite/xtest/src/libproto/Attic/ShowExtReq.c,v 1.1.1.1 1997/03/15 06:19:58 scottr Exp $
 */

#ifndef lint
static char rcsid[]="$Header: /cvsroot/xsrc/xc/test/xsuite/xtest/src/libproto/Attic/ShowExtReq.c,v 1.1.1.1 1997/03/15 06:19:58 scottr Exp $";
#endif

#include "XstlibInt.h"
extern int XInputMajorOpcode;

void
Show_Ext_Req(mp)
xReq *mp;
{
	if (mp->reqType == XInputMajorOpcode) {
		switch (mp->reqType) {
		case X_GetExtensionVersion:
			BPRINTF1("GetExtensionVersion:\n");
			BPRINTF2("\treqType = %ld\n",(long) ((xGetExtensionVersionReq *)mp)->reqType);
			BPRINTF2("\tReqType = %ld\n",(long) ((xGetExtensionVersionReq *)mp)->ReqType);
			BPRINTF2("\tlength = %ld\n",(long) ((xGetExtensionVersionReq *)mp)->length);
			BPRINTF2("\tnbytes = %ld\n",(long) ((xGetExtensionVersionReq *)mp)->nbytes);
			break;
		case X_ListInputDevices:
			BPRINTF1("ListInputDevices:\n");
			BPRINTF2("\treqType = %ld\n",(long) ((xListInputDevicesReq *)mp)->reqType);
			BPRINTF2("\tReqType = %ld\n",(long) ((xListInputDevicesReq *)mp)->ReqType);
			BPRINTF2("\tlength = %ld\n",(long) ((xListInputDevicesReq *)mp)->length);
			break;
		case X_OpenDevice:
			BPRINTF1("OpenDevice:\n");
			BPRINTF2("\treqType = %ld\n",(long) ((xOpenDeviceReq *)mp)->reqType);
			BPRINTF2("\tReqType = %ld\n",(long) ((xOpenDeviceReq *)mp)->ReqType);
			BPRINTF2("\tlength = %ld\n",(long) ((xOpenDeviceReq *)mp)->length);
			BPRINTF2("\tdeviceid = %ld\n",(long) ((xOpenDeviceReq *)mp)->deviceid);
			break;
		case X_CloseDevice:
			BPRINTF1("CloseDevice:\n");
			BPRINTF2("\treqType = %ld\n",(long) ((xCloseDeviceReq *)mp)->reqType);
			BPRINTF2("\tReqType = %ld\n",(long) ((xCloseDeviceReq *)mp)->ReqType);
			BPRINTF2("\tlength = %ld\n",(long) ((xCloseDeviceReq *)mp)->length);
			BPRINTF2("\tdeviceid = %ld\n",(long) ((xCloseDeviceReq *)mp)->deviceid);
			break;
		case X_SetDeviceMode:
			BPRINTF1("SetDeviceMode:\n");
			BPRINTF2("\treqType = %ld\n",(long) ((xSetDeviceModeReq *)mp)->reqType);
			BPRINTF2("\tReqType = %ld\n",(long) ((xSetDeviceModeReq *)mp)->ReqType);
			BPRINTF2("\tlength = %ld\n",(long) ((xSetDeviceModeReq *)mp)->length);
			BPRINTF2("\tdeviceid = %ld\n",(long) ((xSetDeviceModeReq *)mp)->deviceid);
			BPRINTF2("\tmode = %ld\n",(long) ((xSetDeviceModeReq *)mp)->mode);
			break;
		case X_SelectExtensionEvent:
			BPRINTF1("SelectExtensionEvent:\n");
			BPRINTF2("\treqType = %ld\n",(long) ((xSelectExtensionEventReq *)mp)->reqType);
			BPRINTF2("\tReqType = %ld\n",(long) ((xSelectExtensionEventReq *)mp)->ReqType);
			BPRINTF2("\tlength = %ld\n",(long) ((xSelectExtensionEventReq *)mp)->length);
			BPRINTF2("\tcount = %ld\n",(long) ((xSelectExtensionEventReq *)mp)->count);
			Show_Value_List_Req(mp,sizeof(xSelectExtensionEventReq),FORMAT32);
			break;
		case X_GetSelectedExtensionEvents:
			BPRINTF1("GetSelectedExtensionEvents:\n");
			BPRINTF2("\treqType = %ld\n",(long) ((xGetSelectedExtensionEventsReq *)mp)->reqType);
			BPRINTF2("\tReqType = %ld\n",(long) ((xGetSelectedExtensionEventsReq *)mp)->ReqType);
			BPRINTF2("\tlength = %ld\n",(long) ((xGetSelectedExtensionEventsReq *)mp)->length);
			BPRINTF2("\twindow = %ld\n",(long) ((xGetSelectedExtensionEventsReq *)mp)->window);
			break;
		case X_ChangeDeviceDontPropagateList:
			BPRINTF1("ChangeDeviceDontPropagateList:\n");
			BPRINTF2("\treqType = %ld\n",(long) ((xChangeDeviceDontPropagateListReq *)mp)->reqType);
			BPRINTF2("\tReqType = %ld\n",(long) ((xChangeDeviceDontPropagateListReq *)mp)->ReqType);
			BPRINTF2("\tlength = %ld\n",(long) ((xChangeDeviceDontPropagateListReq *)mp)->length);
			BPRINTF2("\twindow = %ld\n",(long) ((xChangeDeviceDontPropagateListReq *)mp)->window);
			BPRINTF2("\tcount = %ld\n",(long) ((xChangeDeviceDontPropagateListReq *)mp)->count);
			BPRINTF2("\tmode = %ld\n",(long) ((xChangeDeviceDontPropagateListReq *)mp)->mode);
			Show_Value_List_Req(mp,sizeof(xChangeDeviceDontPropagateListReq),FORMAT32);
			break;
		case X_GetDeviceDontPropagateList:
			BPRINTF1("GetDeviceDontPropagateList:\n");
			BPRINTF2("\treqType = %ld\n",(long) ((xGetDeviceDontPropagateListReq *)mp)->reqType);
			BPRINTF2("\tReqType = %ld\n",(long) ((xGetDeviceDontPropagateListReq *)mp)->ReqType);
			BPRINTF2("\tlength = %ld\n",(long) ((xGetDeviceDontPropagateListReq *)mp)->length);
			BPRINTF2("\twindow = %ld\n",(long) ((xGetDeviceDontPropagateListReq *)mp)->window);
			break;
		case X_GetDeviceMotionEvents:
			BPRINTF1("GetDeviceMotionEvents:\n");
			BPRINTF2("\treqType = %ld\n",(long) ((xGetDeviceMotionEventsReq *)mp)->reqType);
			BPRINTF2("\tReqType = %ld\n",(long) ((xGetDeviceMotionEventsReq *)mp)->ReqType);
			BPRINTF2("\tlength = %ld\n",(long) ((xGetDeviceMotionEventsReq *)mp)->length);
			BPRINTF2("\tstart = %ld\n",(long) ((xGetDeviceMotionEventsReq *)mp)->start);
			BPRINTF2("\tstop = %ld\n",(long) ((xGetDeviceMotionEventsReq *)mp)->stop);
			BPRINTF2("\tdeviceid = %ld\n",(long) ((xGetDeviceMotionEventsReq *)mp)->deviceid);
			break;
		case X_ChangeKeyboardDevice:
			BPRINTF1("ChangeKeyboardDevice:\n");
			BPRINTF2("\treqType = %ld\n",(long) ((xChangeKeyboardDeviceReq *)mp)->reqType);
			BPRINTF2("\tReqType = %ld\n",(long) ((xChangeKeyboardDeviceReq *)mp)->ReqType);
			BPRINTF2("\tlength = %ld\n",(long) ((xChangeKeyboardDeviceReq *)mp)->length);
			BPRINTF2("\tdeviceid = %ld\n",(long) ((xChangeKeyboardDeviceReq *)mp)->deviceid);
			break;
		case X_ChangePointerDevice:
			BPRINTF1("ChangePointerDevice:\n");
			BPRINTF2("\treqType = %ld\n",(long) ((xChangePointerDeviceReq *)mp)->reqType);
			BPRINTF2("\tReqType = %ld\n",(long) ((xChangePointerDeviceReq *)mp)->ReqType);
			BPRINTF2("\tlength = %ld\n",(long) ((xChangePointerDeviceReq *)mp)->length);
			BPRINTF2("\txaxis = %ld\n",(long) ((xChangePointerDeviceReq *)mp)->xaxis);
			BPRINTF2("\tyaxis = %ld\n",(long) ((xChangePointerDeviceReq *)mp)->yaxis);
			BPRINTF2("\tdeviceid = %ld\n",(long) ((xChangePointerDeviceReq *)mp)->deviceid);
			break;
		case X_GrabDevice:
			BPRINTF1("GrabDevice:\n");
			BPRINTF2("\treqType = %ld\n",(long) ((xGrabDeviceReq *)mp)->reqType);
			BPRINTF2("\tReqType = %ld\n",(long) ((xGrabDeviceReq *)mp)->ReqType);
			BPRINTF2("\tlength = %ld\n",(long) ((xGrabDeviceReq *)mp)->length);
			BPRINTF2("\tgrabWindow = %ld\n",(long) ((xGrabDeviceReq *)mp)->grabWindow);
			BPRINTF2("\ttime = %ld\n",(long) ((xGrabDeviceReq *)mp)->time);
			BPRINTF2("\tevent_count = %ld\n",(long) ((xGrabDeviceReq *)mp)->event_count);
			BPRINTF2("\tthis_device_mode = %ld\n",(long) ((xGrabDeviceReq *)mp)->this_device_mode);
			BPRINTF2("\tother_devices_mode = %ld\n",(long) ((xGrabDeviceReq *)mp)->other_devices_mode);
			BPRINTF2("\townerEvents = %ld\n",(long) ((xGrabDeviceReq *)mp)->ownerEvents);
			BPRINTF2("\tdeviceid = %ld\n",(long) ((xGrabDeviceReq *)mp)->deviceid);
			break;
		case X_UngrabDevice:
			BPRINTF1("UnUngrabDevice:\n");
			BPRINTF2("\treqType = %ld\n",(long) ((xUngrabDeviceReq *)mp)->reqType);
			BPRINTF2("\tReqType = %ld\n",(long) ((xUngrabDeviceReq *)mp)->ReqType);
			BPRINTF2("\tlength = %ld\n",(long) ((xUngrabDeviceReq *)mp)->length);
			BPRINTF2("\ttime = %ld\n",(long) ((xUngrabDeviceReq *)mp)->time);
			BPRINTF2("\tdeviceid = %ld\n",(long) ((xUngrabDeviceReq *)mp)->deviceid);
			break;
		case X_GrabDeviceKey:
			BPRINTF1("GrabDeviceKeyKey:\n");
			BPRINTF2("\treqType = %ld\n",(long) ((xGrabDeviceKeyReq *)mp)->reqType);
			BPRINTF2("\tReqType = %ld\n",(long) ((xGrabDeviceKeyReq *)mp)->ReqType);
			BPRINTF2("\tlength = %ld\n",(long) ((xGrabDeviceKeyReq *)mp)->length);
			BPRINTF2("\tgrabWindow = %ld\n",(long) ((xGrabDeviceKeyReq *)mp)->grabWindow);
			BPRINTF2("\tevent_count = %ld\n",(long) ((xGrabDeviceKeyReq *)mp)->event_count);
			BPRINTF2("\tmodifiers = %ld\n",(long) ((xGrabDeviceKeyReq *)mp)->modifiers);
			BPRINTF2("\tmodifier_device = %ld\n",(long) ((xGrabDeviceKeyReq *)mp)->modifier_device);
			BPRINTF2("\tgrabbed_device = %ld\n",(long) ((xGrabDeviceKeyReq *)mp)->grabbed_device);
			BPRINTF2("\tkey = %ld\n",(long) ((xGrabDeviceKeyReq *)mp)->key);
			BPRINTF2("\tthis_device_mode = %ld\n",(long) ((xGrabDeviceKeyReq *)mp)->this_device_mode);
			BPRINTF2("\tother_devices_mode = %ld\n",(long) ((xGrabDeviceKeyReq *)mp)->other_devices_mode);
			BPRINTF2("\townerEvents = %ld\n",(long) ((xGrabDeviceKeyReq *)mp)->ownerEvents);
			break;
		case X_UngrabDeviceKey:
			BPRINTF1("UnUngrabDeviceKey:\n");
			BPRINTF2("\treqType = %ld\n",(long) ((xUngrabDeviceKeyReq *)mp)->reqType);
			BPRINTF2("\tReqType = %ld\n",(long) ((xUngrabDeviceKeyReq *)mp)->ReqType);
			BPRINTF2("\tlength = %ld\n",(long) ((xUngrabDeviceKeyReq *)mp)->length);
			BPRINTF2("\tgrabWindow = %ld\n",(long) ((xUngrabDeviceKeyReq *)mp)->grabWindow);
			BPRINTF2("\tmodifiers = %ld\n",(long) ((xUngrabDeviceKeyReq *)mp)->modifiers);
			BPRINTF2("\tmodifier_device = %ld\n",(long) ((xUngrabDeviceKeyReq *)mp)->modifier_device);
			BPRINTF2("\tkey = %ld\n",(long) ((xUngrabDeviceKeyReq *)mp)->key);
			BPRINTF2("\tgrabbed_device = %ld\n",(long) ((xUngrabDeviceKeyReq *)mp)->grabbed_device);
			BPRINTF2("\tthis_device_mode = %ld\n",(long) ((xGrabDeviceKeyReq *)mp)->this_device_mode);
			BPRINTF2("\tother_device_mode = %ld\n",(long) ((xGrabDeviceKeyReq *)mp)->other_devices_mode);
			BPRINTF2("\townerEvents = %ld\n",(long) ((xGrabDeviceKeyReq *)mp)->ownerEvents);
			break;
		case X_GrabDeviceButton:
			BPRINTF1("GrabDeviceButton:\n");
			BPRINTF2("\treqType = %ld\n",(long) ((xGrabDeviceButtonReq *)mp)->reqType);
			BPRINTF2("\tReqType = %ld\n",(long) ((xGrabDeviceButtonReq *)mp)->ReqType);
			BPRINTF2("\tlength = %ld\n",(long) ((xGrabDeviceButtonReq *)mp)->length);
			BPRINTF2("\tgrabWindow = %ld\n",(long) ((xGrabDeviceButtonReq *)mp)->grabWindow);
			BPRINTF2("\tgrabbed_device = %ld\n",(long) ((xGrabDeviceButtonReq *)mp)->grabbed_device);
			BPRINTF2("\tmodifier_device = %ld\n",(long) ((xGrabDeviceButtonReq *)mp)->modifier_device);
			BPRINTF2("\tevent_count = %ld\n",(long) ((xGrabDeviceButtonReq *)mp)->event_count);
			BPRINTF2("\tmodifiers = %ld\n",(long) ((xGrabDeviceButtonReq *)mp)->modifiers);
			BPRINTF2("\tthis_device_mode = %ld\n",(long) ((xGrabDeviceButtonReq *)mp)->this_device_mode);
			BPRINTF2("\tother_devices_mode = %ld\n",(long) ((xGrabDeviceButtonReq *)mp)->other_devices_mode);
			BPRINTF2("\tbutton = %ld\n",(long) ((xGrabDeviceButtonReq *)mp)->button);
			BPRINTF2("\townerEvents = %ld\n",(long) ((xGrabDeviceButtonReq *)mp)->ownerEvents);
			break;
		case X_UngrabDeviceButton:
			BPRINTF1("UngrabDeviceButton:\n");
			BPRINTF2("\treqType = %ld\n",(long) ((xUngrabDeviceButtonReq *)mp)->reqType);
			BPRINTF2("\tReqType = %ld\n",(long) ((xUngrabDeviceButtonReq *)mp)->ReqType);
			BPRINTF2("\tlength = %ld\n",(long) ((xUngrabDeviceButtonReq *)mp)->length);
			BPRINTF2("\tgrabWindow = %ld\n",(long) ((xUngrabDeviceButtonReq *)mp)->grabWindow);
			BPRINTF2("\tmodifiers = %ld\n",(long) ((xUngrabDeviceButtonReq *)mp)->modifiers);
			BPRINTF2("\tmodifier_device = %ld\n",(long) ((xUngrabDeviceButtonReq *)mp)->modifier_device);
			BPRINTF2("\tbutton = %ld\n",(long) ((xUngrabDeviceButtonReq *)mp)->button);
			BPRINTF2("\tgrabbed_device = %ld\n",(long) ((xUngrabDeviceButtonReq *)mp)->grabbed_device);
			break;
		case X_AllowDeviceEvents:
			BPRINTF1("AllowDeviceEvents:\n");
			BPRINTF2("\treqType = %ld\n",(long) ((xAllowDeviceEventsReq *)mp)->reqType);
			BPRINTF2("\tReqType = %ld\n",(long) ((xAllowDeviceEventsReq *)mp)->ReqType);
			BPRINTF2("\tlength = %ld\n",(long) ((xAllowDeviceEventsReq *)mp)->length);
			BPRINTF2("\ttime = %ld\n",(long) ((xAllowDeviceEventsReq *)mp)->time);
			BPRINTF2("\tmode = %ld\n",(long) ((xAllowDeviceEventsReq *)mp)->mode);
			BPRINTF2("\tdeviceid = %ld\n",(long) ((xAllowDeviceEventsReq *)mp)->deviceid);
			break;
		case X_GetDeviceFocus:
			BPRINTF1("GetDeviceFocus:\n");
			BPRINTF2("\treqType = %ld\n",(long) ((xGetDeviceFocusReq *)mp)->reqType);
			BPRINTF2("\tReqType = %ld\n",(long) ((xGetDeviceFocusReq *)mp)->ReqType);
			BPRINTF2("\tlength = %ld\n",(long) ((xGetDeviceFocusReq *)mp)->length);
			BPRINTF2("\tdeviceid = %ld\n",(long) ((xGetDeviceFocusReq *)mp)->deviceid);
			break;
		case X_SetDeviceFocus:
			BPRINTF1("SetDeviceFocus:\n");
			BPRINTF2("\treqType = %ld\n",(long) ((xSetDeviceFocusReq *)mp)->reqType);
			BPRINTF2("\tReqType = %ld\n",(long) ((xSetDeviceFocusReq *)mp)->ReqType);
			BPRINTF2("\tlength = %ld\n",(long) ((xSetDeviceFocusReq *)mp)->length);
			BPRINTF2("\tfocus = %ld\n",(long) ((xSetDeviceFocusReq *)mp)->focus);
			BPRINTF2("\ttime = %ld\n",(long) ((xSetDeviceFocusReq *)mp)->time);
			BPRINTF2("\trevertTo = %ld\n",(long) ((xSetDeviceFocusReq *)mp)->revertTo);
			BPRINTF2("\tdevice = %ld\n",(long) ((xSetDeviceFocusReq *)mp)->device);
			break;
		case X_GetFeedbackControl:
			BPRINTF1("GetFeedbackControl:\n");
			BPRINTF2("\treqType = %ld\n",(long) ((xGetFeedbackControlReq *)mp)->reqType);
			BPRINTF2("\tReqType = %ld\n",(long) ((xGetFeedbackControlReq *)mp)->ReqType);
			BPRINTF2("\tlength = %ld\n",(long) ((xGetFeedbackControlReq *)mp)->length);
			BPRINTF2("\tdeviceid = %ld\n",(long) ((xGetFeedbackControlReq *)mp)->deviceid);
			break;
		case X_ChangeFeedbackControl:
			BPRINTF1("ChangeFeedbackControl:\n");
			BPRINTF2("\treqType = %ld\n",(long) ((xChangeFeedbackControlReq *)mp)->reqType);
			BPRINTF2("\tReqType = %ld\n",(long) ((xChangeFeedbackControlReq *)mp)->ReqType);
			BPRINTF2("\tlength = %ld\n",(long) ((xChangeFeedbackControlReq *)mp)->length);
			BPRINTF2("\tdeviceid = %ld\n",(long) ((xChangeFeedbackControlReq *)mp)->deviceid);
			break;
		case X_GetDeviceKeyMapping:
			BPRINTF1("GetDeviceKeyMapping:\n");
			BPRINTF2("\treqType = %ld\n",(long) ((xGetDeviceKeyMappingReq *)mp)->reqType);
			BPRINTF2("\tReqType = %ld\n",(long) ((xGetDeviceKeyMappingReq *)mp)->ReqType);
			BPRINTF2("\tlength = %ld\n",(long) ((xGetDeviceKeyMappingReq *)mp)->length);
			BPRINTF2("\tdeviceid = %ld\n",(long) ((xGetDeviceKeyMappingReq *)mp)->deviceid);
			BPRINTF2("\tfirstKeyCode = %ld\n",(long) ((xGetDeviceKeyMappingReq *)mp)->firstKeyCode);
			BPRINTF2("\tcount = %ld\n",(long) ((xGetDeviceKeyMappingReq *)mp)->count);
			break;
		case X_ChangeDeviceKeyMapping:
			BPRINTF1("ChangeDeviceKeyMapping:\n");
			BPRINTF2("\treqType = %ld\n",(long) ((xChangeDeviceKeyMappingReq *)mp)->reqType);
			BPRINTF2("\tReqType = %ld\n",(long) ((xChangeDeviceKeyMappingReq *)mp)->ReqType);
			BPRINTF2("\tlength = %ld\n",(long) ((xChangeDeviceKeyMappingReq *)mp)->length);
			BPRINTF2("\tdeviceid = %ld\n",(long) ((xChangeDeviceKeyMappingReq *)mp)->deviceid);
			BPRINTF2("\tfirstKeyCode = %ld\n",(long) ((xChangeDeviceKeyMappingReq *)mp)->firstKeyCode);
			BPRINTF2("\tkeySymsPerKeyCode = %ld\n",(long) ((xChangeDeviceKeyMappingReq *)mp)->keySymsPerKeyCode);
			BPRINTF2("\tkeyCodes = %ld\n",(long) ((xChangeDeviceKeyMappingReq *)mp)->keyCodes);
			break;
		case X_GetDeviceModifierMapping:
			BPRINTF1("GetDeviceModifierMapping:\n");
			BPRINTF2("\treqType = %ld\n",(long) ((xGetDeviceModifierMappingReq *)mp)->reqType);
			BPRINTF2("\tReqType = %ld\n",(long) ((xGetDeviceModifierMappingReq *)mp)->ReqType);
			BPRINTF2("\tlength = %ld\n",(long) ((xGetDeviceModifierMappingReq *)mp)->length);
			BPRINTF2("\tdeviceid = %ld\n",(long) ((xGetDeviceModifierMappingReq *)mp)->deviceid);
			break;
		case X_SetDeviceModifierMapping:
			BPRINTF1("SetDeviceModifierMapping:\n");
			BPRINTF2("\treqType = %ld\n",(long) ((xSetDeviceModifierMappingReq *)mp)->reqType);
			BPRINTF2("\tReqType = %ld\n",(long) ((xSetDeviceModifierMappingReq *)mp)->ReqType);
			BPRINTF2("\tlength = %ld\n",(long) ((xSetDeviceModifierMappingReq *)mp)->length);
			BPRINTF2("\tdeviceid = %ld\n",(long) ((xSetDeviceModifierMappingReq *)mp)->deviceid);
			BPRINTF2("\tnumKeyPerModifier = %ld\n",(long) ((xSetDeviceModifierMappingReq *)mp)->numKeyPerModifier);
			break;
		case X_GetDeviceButtonMapping:
			BPRINTF1("GetDeviceButtonMapping:\n");
			BPRINTF2("\treqType = %ld\n",(long) ((xGetDeviceButtonMappingReq *)mp)->reqType);
			BPRINTF2("\tReqType = %ld\n",(long) ((xGetDeviceButtonMappingReq *)mp)->ReqType);
			BPRINTF2("\tlength = %ld\n",(long) ((xGetDeviceButtonMappingReq *)mp)->length);
			BPRINTF2("\tdeviceid = %ld\n",(long) ((xGetDeviceButtonMappingReq *)mp)->deviceid);
			break;
		case X_SetDeviceButtonMapping:
			BPRINTF1("SetDeviceButtonMapping:\n");
			BPRINTF2("\treqType = %ld\n",(long) ((xSetDeviceButtonMappingReq *)mp)->reqType);
			BPRINTF2("\tReqType = %ld\n",(long) ((xSetDeviceButtonMappingReq *)mp)->ReqType);
			BPRINTF2("\tlength = %ld\n",(long) ((xSetDeviceButtonMappingReq *)mp)->length);
			BPRINTF2("\tdeviceid = %ld\n",(long) ((xSetDeviceButtonMappingReq *)mp)->deviceid);
			BPRINTF2("\tmap_length = %ld\n",(long) ((xSetDeviceButtonMappingReq *)mp)->map_length);
			break;
		case X_QueryDeviceState:
			BPRINTF1("QueryDeviceState:\n");
			BPRINTF2("\treqType = %ld\n",(long) ((xQueryDeviceStateReq *)mp)->reqType);
			BPRINTF2("\tReqType = %ld\n",(long) ((xQueryDeviceStateReq *)mp)->ReqType);
			BPRINTF2("\tlength = %ld\n",(long) ((xQueryDeviceStateReq *)mp)->length);
			BPRINTF2("\tdeviceid = %ld\n",(long) ((xQueryDeviceStateReq *)mp)->deviceid);
			break;
		case X_SendExtensionEvent:
			BPRINTF1("SendExtensionEvent:\n");
			BPRINTF2("\treqType = %ld\n",(long) ((xSendExtensionEventReq *)mp)->reqType);
			BPRINTF2("\tReqType = %ld\n",(long) ((xSendExtensionEventReq *)mp)->ReqType);
			BPRINTF2("\tlength = %ld\n",(long) ((xSendExtensionEventReq *)mp)->length);
			BPRINTF2("\tdestination = %ld\n",(long) ((xSendExtensionEventReq *)mp)->destination);
			BPRINTF2("\tdeviceid = %ld\n",(long) ((xSendExtensionEventReq *)mp)->deviceid);
			BPRINTF2("\tpropagate = %ld\n",(long) ((xSendExtensionEventReq *)mp)->propagate);
			BPRINTF2("\tcount = %ld\n",(long) ((xSendExtensionEventReq *)mp)->count);
			BPRINTF2("\tnum_events = %ld\n",(long) ((xSendExtensionEventReq *)mp)->num_events);
			break;
		case X_DeviceBell:
			BPRINTF1("DeviceBell:\n");
			BPRINTF2("\treqType = %ld\n",(long) ((xDeviceBellReq *)mp)->reqType);
			BPRINTF2("\tReqType = %ld\n",(long) ((xDeviceBellReq *)mp)->ReqType);
			BPRINTF2("\tlength = %ld\n",(long) ((xDeviceBellReq *)mp)->length);
			BPRINTF2("\tdeviceid = %ld\n",(long) ((xDeviceBellReq *)mp)->deviceid);
			BPRINTF2("\tfeedbackid = %ld\n",(long) ((xDeviceBellReq *)mp)->feedbackid);
			BPRINTF2("\tfeedbackclass = %ld\n",(long) ((xDeviceBellReq *)mp)->feedbackclass);
			BPRINTF2("\tpercent = %ld\n",(long) ((xDeviceBellReq *)mp)->percent);
			break;
		case X_SetDeviceValuators:
			BPRINTF1("SetDeviceValuators:\n");
			BPRINTF2("\treqType = %ld\n",(long) ((xSetDeviceValuatorsReq *)mp)->reqType);
			BPRINTF2("\tReqType = %ld\n",(long) ((xSetDeviceValuatorsReq *)mp)->ReqType);
			BPRINTF2("\tlength = %ld\n",(long) ((xSetDeviceValuatorsReq *)mp)->length);
			BPRINTF2("\tdeviceid = %ld\n",(long) ((xSetDeviceValuatorsReq *)mp)->deviceid);
			BPRINTF2("\tfirst_valuator = %ld\n",(long) ((xSetDeviceValuatorsReq *)mp)->first_valuator);
			BPRINTF2("\tnum_valuators = %ld\n",(long) ((xSetDeviceValuatorsReq *)mp)->num_valuators);
			break;
		case X_GetDeviceControl:
			BPRINTF1("GetDeviceControl:\n");
			BPRINTF2("\treqType = %ld\n",(long) ((xGetDeviceControlReq *)mp)->reqType);
			BPRINTF2("\tReqType = %ld\n",(long) ((xGetDeviceControlReq *)mp)->ReqType);
			BPRINTF2("\tlength = %ld\n",(long) ((xGetDeviceControlReq *)mp)->length);
			BPRINTF2("\tcontrol = %ld\n",(long) ((xGetDeviceControlReq *)mp)->control);
			BPRINTF2("\tdeviceid = %ld\n",(long) ((xGetDeviceControlReq *)mp)->deviceid);
			break;
		case X_ChangeDeviceControl:
			BPRINTF1("ChangeDeviceControl:\n");
			BPRINTF2("\treqType = %ld\n",(long) ((xChangeDeviceControlReq *)mp)->reqType);
			BPRINTF2("\tReqType = %ld\n",(long) ((xChangeDeviceControlReq *)mp)->ReqType);
			BPRINTF2("\tlength = %ld\n",(long) ((xChangeDeviceControlReq *)mp)->length);
			BPRINTF2("\tcontrol = %ld\n",(long) ((xChangeDeviceControlReq *)mp)->control);
			BPRINTF2("\tdeviceid = %ld\n",(long) ((xChangeDeviceControlReq *)mp)->deviceid);
			break;

		default:
			BPRINTF1("Impossible request:\n");
			BPRINTF2("\treqType = %ld\n",(long) ((xChangeDeviceControlReq *)mp)->reqType);
			break;
		}
	}
	else 	{
		BPRINTF1("Unsupported Extension request:\n");
		BPRINTF2("\treqType = %ld\n",(long) ((xChangeDeviceControlReq *)mp)->reqType);
	}
}

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
 * $XConsortium: MakeXIReq.c /main/4 1995/12/07 10:47:19 dpw $
 */
/*
 * ***************************************************************************
 *  Copyright 1988, 1989 by Sequent Computer Systems, Inc., Portland, Oregon *
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
 *  WHETHER IN AN ACTION OF CONTRACT, NEGLIGCE OR OTHER TORTIOUS ACTION,   *
 *  ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS      *
 *  SOFTWARE.                                                                *
 * ***************************************************************************
 */

/*
 *	$Header: /cvsroot/xsrc/xc/test/xsuite/xtest/src/libproto/Attic/MakeXIReq.c,v 1.1.1.1 1997/03/15 06:19:55 scottr Exp $
 */

#ifndef lint
static char rcsid[]="$Header: /cvsroot/xsrc/xc/test/xsuite/xtest/src/libproto/Attic/MakeXIReq.c,v 1.1.1.1 1997/03/15 06:19:55 scottr Exp $";
#endif

#include "DataMove.h"
#include "XstlibInt.h"
#include "extensions/XIproto.h"
#include "extensions/XInput.h"
#include "XItest.h"
#include <string.h>
#include <X11/Xatom.h>
#include <X11/X.h>

#define RED 65535       /* default red intensity, range is 0 to 65535 */
#define GREEN 65535     /* default green intensity, range is 0 to 65535 */
#define BLUE 65535      /* default blue intensity, range is 0 to 65535 */
#define NPLANES 0       /* default number of planes */
#define PLANEMASK 0     /* default plane mask */
#define TEST16_2 0x2129   /* ? character in test16 (16 bit) font */
#define TEST16_3 0x212a   /* ! character in test16 (16 bit) font */

extern int XInputMajorOpcode;
extern ExtDeviceInfo Devs;
extern int ButtonMapLength;
extern int MinKeyCode;
extern char *FeedbackData;
extern int NumKeys,
	   nclass,
	   Feedback_Class, 
	   FeedbackDevice, 
	   FeedbackSize, 
	   FeedbackMask;
extern XEventClass classes[32];
extern xEvent kev[2];
extern XEventClass devicebuttonpressclass,
	    devicekeypressclass, 
	    devicemotionnotifyclass;
static int bad_len,
	   this_client;
static int alloc_len; /* significantly different only in TOO_LONG tests */

/*
 * X Protocol packetizing macros.
 */

/* The following four macros - GetReq, GetReqExtra, GetEmptyReq, and
   GetResReq, all allocate memory for a request and initialize the
   length and reqType fields of the request.  Use GetEmptyReq for
   requests with no parameters, GetResReq for requests with one
   parameter, GetReq for fixed-length requests with more than one
   parameter, and GetReqExtra for variable-length requests with more
   than one parameter.  Look in <X11/Xproto.h> for structure
   definitions - this will help determine which one to use.
*/
/*
 * GetReq - Get an X request packet and return it. 
 *
 * "name" is the name of the request, e.g. CreatePixmap, OpenFont, etc.
 * "req" is the name of the request pointer.
 *
 *	- assumes 'dpy' available
 *	- packet is 'malloc'ed, must be 'free'd later
 *
 */


/*
 *	Macro: GetReq - allocates memory for a fixed-length request that 
 *      takes more than one parameter, and initialize the the length and 
 *      reqType fields of the request.
 *
 *	Input: name - request name
 *             req - pointer to request struct
 *
 *	Output: allocated, initialized request structure
 *
 *	Returns:
 *
 *	Globals used:	this_client (test_type) (IN), bad_len (OUT)
 *						 and alloc_len (OUT).
 *
 *	Side Effects: allocates memory, assigns to bad_len.
 *
 *	Methods:
 *
 */

#ifdef __STDC__
#define GetReq(name, req) \
	bad_len = (Get_Test_Type(this_client) == TOO_LONG) ? \
		((sizeof(x##name##Req))>>2) : \
		( \
			(Get_Test_Type(this_client) == JUST_TOO_LONG) ? \
			((sizeof(x##name##Req))>>2) + 1 : \
			((sizeof(x##name##Req))>>2) - 1 \
		);\
	if (Get_Test_Type(this_client) == TOO_LONG) \
		alloc_len = Get_Max_Request(client) + 1; \
	else \
		alloc_len = bad_len; \
	req = (xReq *)Xstmalloc(max(sizeof(x##name##Req), (alloc_len<<2))); \
	req->length = (sizeof(x##name##Req))>>2;\
	((x##name##Req *)req)->ReqType = X_##name;\
	req->reqType = XInputMajorOpcode
#else
#define GetReq(name, req) \
	bad_len = (Get_Test_Type(this_client) == TOO_LONG) ? \
		((sizeof(x/**/name/**/Req))>>2) : \
		( \
			(Get_Test_Type(this_client) == JUST_TOO_LONG) ? \
			((sizeof(x/**/name/**/Req))>>2) + 1 : \
			((sizeof(x/**/name/**/Req))>>2) - 1 \
		);\
	if (Get_Test_Type(this_client) == TOO_LONG) \
		alloc_len = Get_Max_Request(client) + 1; \
	else \
		alloc_len = bad_len; \
	req = (xReq *)Xstmalloc(max(sizeof(x/**/name/**/Req), (alloc_len<<2))); \
	req->length = (sizeof(x/**/name/**/Req))>>2;\
	((x/**/name/**/Req *)req)->ReqType = X_/**/name;\
	req->reqType = XInputMajorOpcode
#endif

/*
 *	Macro: GetReqExtra - allocates memory for a variable-length request 
 *      that takes more than one parameter, and initialize the length and
 *      reqType fields of the request.
 *
 *	Input: name - request name
 *             n - number of extra bytes to allocate
 *             req - pointer to request struct
 *
 *	Output: allocated, initialized request structure
 *
 *	Returns:
 *
 *	Globals used:	this_client (test_type) (IN), bad_len (OUT)
 *						 and alloc_len (OUT).
 *
 *	Side Effects: allocates memory, assigns to bad_len.
 *
 *	Methods:
 *
 */

#ifdef __STDC__
#define GetReqExtra(name, n, req) \
	bad_len = (Get_Test_Type(this_client) == TOO_LONG) ? \
		(sizeof(x##name##Req)+padup(n))>>2 : \
		( \
			(Get_Test_Type(this_client) == JUST_TOO_LONG) ? \
			((sizeof(x##name##Req)+padup(n))>>2) + 1 : \
			((sizeof(x##name##Req))>>2) - 1 \
		);\
	if (Get_Test_Type(this_client) == TOO_LONG) \
		alloc_len = Get_Max_Request(client) + 1; \
	else \
		alloc_len = bad_len; \
	req = (xReq *)Xstmalloc(max((sizeof(x##name##Req)+padup(n)), (alloc_len<<2))); \
	req->length = (sizeof(x##name##Req)+padup(n))>>2;\
	req->reqType = XInputMajorOpcode;\
	((x##name##Req *)req)->ReqType = X_##name;\
	valuePtr = (char *) Get_Value_Ptr(req,name)
#else
#define GetReqExtra(name, n, req) \
	bad_len = (Get_Test_Type(this_client) == TOO_LONG) ? \
		(sizeof(x/**/name/**/Req)+padup(n))>>2 : \
		( \
			(Get_Test_Type(this_client) == JUST_TOO_LONG) ? \
			((sizeof(x/**/name/**/Req)+padup(n))>>2) + 1 : \
			((sizeof(x/**/name/**/Req))>>2) - 1 \
		);\
	if (Get_Test_Type(this_client) == TOO_LONG) \
		alloc_len = Get_Max_Request(client) + 1; \
	else \
		alloc_len = bad_len; \
	req = (xReq *)Xstmalloc(max((sizeof(x/**/name/**/Req)+padup(n)), (alloc_len<<2))); \
	req->length = (sizeof(x/**/name/**/Req)+padup(n))>>2;\
	req->reqType = XInputMajorOpcode;\
	((x/**/name/**/Req *)req)->ReqType = X_/**/name;\
	valuePtr = (char *) Get_Value_Ptr(req,name)
#endif

/*
 *	Macro: GetEmptyReq - allocate memory for a request that takes no 
 *      parameters, and initialize the length, reqType fields of the request.
 *
 *	Input: name - request name
 *             req - pointer to request struct
 *
 *	Output: allocated, initialized request structure
 *
 *	Returns:
 *
 *	Globals used:	this_client (test_type) (IN), bad_len (OUT)
 *						 and alloc_len (OUT).
 *
 *	Side Effects: allocates memory, assigns to bad_len.
 *
 *	Methods:
 *
 */

#ifdef __STDC__
#define GetEmptyReq(name, req) \
	bad_len = (Get_Test_Type(this_client) == TOO_LONG) ? \
		1 : \
		( \
			(Get_Test_Type(this_client) == JUST_TOO_LONG) ? \
			2 : 0 \
		);\
	if (Get_Test_Type(this_client) == TOO_LONG) \
		alloc_len = Get_Max_Request(client) + 1; \
	else \
		alloc_len = bad_len; \
	req = (xReq *)Xstmalloc(max(sizeof(xReq), (alloc_len<<2))); \
	req->length = 1;\
	req->reqType = XInputMajorOpcode;\
	((x##name##Req *)req)->ReqType = X_##name
#else
#define GetEmptyReq(name, req) \
	bad_len = (Get_Test_Type(this_client) == TOO_LONG) ? \
		1 : \
		( \
			(Get_Test_Type(this_client) == JUST_TOO_LONG) ? \
			2 : 0 \
		);\
	if (Get_Test_Type(this_client) == TOO_LONG) \
		alloc_len = Get_Max_Request(client) + 1; \
	else \
		alloc_len = bad_len; \
	req = (xReq *)Xstmalloc(max(sizeof(xReq), (alloc_len<<2))); \
	req->length = 1;\
	req->reqType = XInputMajorOpcode;\
	((x/**/name/**/Req *)req)->ReqType = X_/**/name
#endif

/* 
   intent:	 allocate memory for a request that takes one parameter,
                 and initialize the length, reqType, id fields of the request.
   input:	 name - request name as defined in X11 protocol spec
                 rid - resource id (integer from ? to ?)
		 req - pointer to xReq
   output:	 
   globals used: this_client (test_type) (IN), bad_len (OUT)
 						 and alloc_len (OUT).
   side effects: allocates memory, assigns to bad_len.
   methods:	 
*/

#ifdef __STDC__
#define GetResReq(name,rid,req)\
	bad_len = (Get_Test_Type(this_client) == TOO_LONG) ? \
		2 : \
		( \
			(Get_Test_Type(this_client) == JUST_TOO_LONG) ? \
			3 : 1 \
		);\
	if (Get_Test_Type(this_client) == TOO_LONG) \
		alloc_len = Get_Max_Request(client) + 1; \
	else \
		alloc_len = bad_len; \
	req = (xReq *)Xstmalloc(max(sizeof(xResourceReq), (alloc_len<<2)));\
	req->length = 2;\
	req->reqType = XInputMajorOpcode;\
	((x##name##Req *)req)->ReqType = X_##name;\
	((xResourceReq *)req)->id = (rid)
#else
#define GetResReq(name,rid,req)\
	bad_len = (Get_Test_Type(this_client) == TOO_LONG) ? \
		2 : \
		( \
			(Get_Test_Type(this_client) == JUST_TOO_LONG) ? \
			3 : 1 \
		);\
	if (Get_Test_Type(this_client) == TOO_LONG) \
		alloc_len = Get_Max_Request(client) + 1; \
	else \
		alloc_len = bad_len; \
	req = (xReq *)Xstmalloc(max(sizeof(xResourceReq), (alloc_len<<2)));\
	req->length = 2;\
	req->reqType = XInputMajorOpcode;\
	((x##name##Req *)req)->ReqType = X_/**/name;\
	((xResourceReq *)req)->id = (rid)
#endif

#define UNKNOWN_LENGTH 0

/*
  intent:       return a pointer to a "filled in" protocol request
  input:        client - client created by invoking Create_Client
                type - protocol request type e.g. X_CopyArea
  output:       rp - pointer to allocated and initialized request
  global input: Get_Test_Type(client)
  side-effects: causes memory to be allocated for requests
  methods:
*/
xReq
*Make_XInput_Req(client,type)
int client;
int type;
{
	int i;
	xReq *rp;
	char *valuePtr = NULL;	/* set NULL to trap errors */
	XstDisplay *dpy = Get_Display(client);

	/* if max_request_size is 65535 then we can't exceed it in the 16-bit
	* length field of requests so no TOO_LONG testing can be done. In
	* this case abandon ship now, with a reason.
	*/
	if (dpy->max_request_size >= (unsigned)0x0000ffff &&
	    Get_Test_Type(client) == TOO_LONG) {
		unsigned n = dpy->max_request_size;

		Log_Msg("This server accepts the largest requests possible (%d words, %d bytes)\n", n, n<<2);
		Log_Msg("\tso this test cannot be performed.\n");
		Destroy_Client(client);
		Untested();
		/*NOTREACHED*/
	}

	this_client = client;

	switch (type) {
	case X_GetExtensionVersion:
		GetReq(GetExtensionVersion,rp);
		((xGetExtensionVersionReq *)rp)->nbytes = 0;
		break;
	case X_ListInputDevices:
		GetReq(ListInputDevices,rp);
		break;
	case X_OpenDevice:
		GetReq(OpenDevice,rp);
		((xOpenDeviceReq *)rp)->deviceid = Devs.Any->device_id;
		break;
	case X_CloseDevice:
		GetReq(CloseDevice,rp);
		((xCloseDeviceReq *)rp)->deviceid = Devs.Any->device_id;
		break;
	case X_SetDeviceMode:
		GetReq(SetDeviceMode,rp);
		((xSetDeviceModeReq *)rp)->deviceid = Devs.DvMod->device_id;
		((xSetDeviceModeReq *)rp)->mode = Absolute;
		break;
	case X_SelectExtensionEvent:
		{
		int i, *cptr;
		GetReqExtra(SelectExtensionEvent,4*nclass,rp);
		((xSelectExtensionEventReq *)rp)->window =
		    Get_Default_Window(client);
		((xSelectExtensionEventReq *)rp)->count = nclass;
		cptr = (int *)((xSelectExtensionEventReq *)rp+1);
		for (i=0; i<nclass; i++)
		    *cptr++ = classes[i];
		}
		break;
	case X_GetSelectedExtensionEvents:
		GetReq(GetSelectedExtensionEvents,rp);
		((xGetSelectedExtensionEventsReq *)rp)->window = 
		    Get_Default_Window(client);
		break;
	case X_ChangeDeviceDontPropagateList:
		GetReqExtra(ChangeDeviceDontPropagateList,4,rp);
		((xChangeDeviceDontPropagateListReq *)rp)->window =
		    Get_Default_Window(client);
		((xChangeDeviceDontPropagateListReq *)rp)->count = 1;
		((xChangeDeviceDontPropagateListReq *)rp)->mode = AddToList;
		*((int *)((xChangeDeviceDontPropagateListReq *)rp+1)) = 
		    devicemotionnotifyclass;
		break;
	case X_GetDeviceDontPropagateList:
		GetReq(GetDeviceDontPropagateList,rp);
		((xGetDeviceDontPropagateListReq *)rp)->window =
		    Get_Default_Window(client);
		break;
	case X_GetDeviceMotionEvents:
		GetReq(GetDeviceMotionEvents,rp);
		((xGetDeviceMotionEventsReq *)rp)->start = 0;
		((xGetDeviceMotionEventsReq *)rp)->stop = CurrentTime;
		((xGetDeviceMotionEventsReq *)rp)->deviceid = 
		    Devs.Valuator->device_id;
		break;
	case X_ChangeKeyboardDevice:
		GetReq(ChangeKeyboardDevice,rp);
		((xChangeKeyboardDeviceReq *)rp)->deviceid = Devs.Key->device_id;
		break;
	case X_ChangePointerDevice:
		GetReq(ChangePointerDevice,rp);
		((xChangePointerDeviceReq *)rp)->deviceid = 
		    Devs.Valuator->device_id;
		((xChangePointerDeviceReq *)rp)->xaxis = 1;
		((xChangePointerDeviceReq *)rp)->yaxis = 0;
		break;
	case X_GrabDevice:
		{
		int i, *cptr;
		GetReqExtra(GrabDevice,8,rp);
		((xGrabDeviceReq *)rp)->grabWindow = Get_Default_Window(client);
		((xGrabDeviceReq *)rp)->time = CurrentTime;
		((xGrabDeviceReq *)rp)->event_count = 2;
		((xGrabDeviceReq *)rp)->this_device_mode = GrabModeAsync;
		((xGrabDeviceReq *)rp)->other_devices_mode = GrabModeAsync;
		((xGrabDeviceReq *)rp)->ownerEvents = True;
		((xGrabDeviceReq *)rp)->deviceid = Devs.Any->device_id;
		cptr = (int *)((xGrabDeviceReq *)rp+1);
		for (i=0; i<2; i++)
		    *cptr++ = classes[i];
		}
		break;
	case X_UngrabDevice:
		GetReq(UngrabDevice,rp);
		((xUngrabDeviceReq *)rp)->deviceid = Devs.Any->device_id;
		((xUngrabDeviceReq *)rp)->time = CurrentTime;
		break;
	case X_GrabDeviceKey:
		GetReqExtra(GrabDeviceKey,4,rp);
		((xGrabDeviceKeyReq *)rp)->grabWindow = 
		    Get_Default_Window(client);
		((xGrabDeviceKeyReq *)rp)->event_count = 1;
		((xGrabDeviceKeyReq *)rp)->modifiers = AnyModifier;
		((xGrabDeviceKeyReq *)rp)->modifier_device = UseXKeyboard;
		((xGrabDeviceKeyReq *)rp)->grabbed_device = Devs.Key->device_id;
		((xGrabDeviceKeyReq *)rp)->key = AnyKey;
		((xGrabDeviceKeyReq *)rp)->this_device_mode = GrabModeAsync;
		((xGrabDeviceKeyReq *)rp)->other_devices_mode = GrabModeAsync;
		((xGrabDeviceKeyReq *)rp)->ownerEvents = True;
		*((int *)((xGrabDeviceKeyReq *)rp+1)) = devicekeypressclass;
		break;
	case X_UngrabDeviceKey:
		GetReq(UngrabDeviceKey,rp);
		((xUngrabDeviceKeyReq *)rp)->grabWindow =
		    Get_Default_Window(client);
		((xUngrabDeviceKeyReq *)rp)->modifiers = AnyModifier;
		((xUngrabDeviceKeyReq *)rp)->modifier_device = UseXKeyboard;
		((xUngrabDeviceKeyReq *)rp)->key = AnyKey;
		((xUngrabDeviceKeyReq *)rp)->grabbed_device = 
		    Devs.Key->device_id;
		break;
	case X_GrabDeviceButton:
		GetReqExtra(GrabDeviceButton,4,rp);
		((xGrabDeviceButtonReq *)rp)->grabWindow =
		    Get_Default_Window(client);
		((xGrabDeviceButtonReq *)rp)->event_count = 1;
		((xGrabDeviceButtonReq *)rp)->modifiers = AnyModifier;
		((xGrabDeviceButtonReq *)rp)->modifier_device = UseXKeyboard;
		((xGrabDeviceButtonReq *)rp)->grabbed_device = 
		    Devs.Button->device_id;
		((xGrabDeviceButtonReq *)rp)->button = AnyButton;
		((xGrabDeviceButtonReq *)rp)->this_device_mode = GrabModeAsync;
		((xGrabDeviceButtonReq *)rp)->other_devices_mode = 
		    GrabModeAsync;
		((xGrabDeviceButtonReq *)rp)->ownerEvents = True;
		*((int *)((xGrabDeviceButtonReq *)rp+1)) = 
		    devicebuttonpressclass;
		break;
	case X_UngrabDeviceButton:
		GetReq(UngrabDeviceButton,rp);
		((xUngrabDeviceButtonReq *)rp)->grabWindow =
		    Get_Default_Window(client);
		((xUngrabDeviceButtonReq *)rp)->modifiers = AnyModifier;
		((xUngrabDeviceButtonReq *)rp)->modifier_device = UseXKeyboard;
		((xUngrabDeviceButtonReq *)rp)->button = AnyButton;
		((xUngrabDeviceButtonReq *)rp)->grabbed_device = 
		    Devs.Button->device_id;
		break;
	case X_AllowDeviceEvents:
		GetReq(AllowDeviceEvents,rp);
		((xAllowDeviceEventsReq *)rp)->time = CurrentTime;
		((xAllowDeviceEventsReq *)rp)->mode = AsyncAll;
		((xAllowDeviceEventsReq *)rp)->deviceid = Devs.Any->device_id;
		break;
	case X_GetDeviceFocus:
		GetReq(GetDeviceFocus,rp);
		((xGetDeviceFocusReq *)rp)->deviceid = Devs.Focus->device_id;
		break;
	case X_SetDeviceFocus:
		GetReq(SetDeviceFocus,rp);
		((xSetDeviceFocusReq *)rp)->focus = PointerRoot;
		((xSetDeviceFocusReq *)rp)->time = CurrentTime;
		((xSetDeviceFocusReq *)rp)->revertTo = RevertToNone;
		((xSetDeviceFocusReq *)rp)->device = Devs.Focus->device_id;
		break;
	case X_GetFeedbackControl:
		GetReq(GetFeedbackControl,rp);
		((xGetFeedbackControlReq *)rp)->deviceid = FeedbackDevice;
		break;
	case X_ChangeFeedbackControl:
		GetReqExtra(ChangeFeedbackControl,FeedbackSize,rp);
		((xChangeFeedbackControlReq *)rp)->mask = FeedbackMask;
		((xChangeFeedbackControlReq *)rp)->deviceid = FeedbackDevice;
		((xChangeFeedbackControlReq *)rp)->feedbackid = Feedback_Class;
		memcpy ((char *) ((xChangeFeedbackControlReq *)rp+1), 
		    FeedbackData, FeedbackSize);
		break;
	case X_GetDeviceKeyMapping:
		GetReq(GetDeviceKeyMapping,rp);
		((xGetDeviceKeyMappingReq *)rp)->deviceid = Devs.Key->device_id;
		((xGetDeviceKeyMappingReq *)rp)->firstKeyCode = MinKeyCode;
		((xGetDeviceKeyMappingReq *)rp)->count = NumKeys;
		break;
	case X_ChangeDeviceKeyMapping:
		GetReq(ChangeDeviceKeyMapping,rp);
		((xChangeDeviceKeyMappingReq *)rp)->deviceid = 
		    Devs.Key->device_id;
		((xChangeDeviceKeyMappingReq *)rp)->firstKeyCode = MinKeyCode;
		((xChangeDeviceKeyMappingReq *)rp)->keySymsPerKeyCode = 1;
		((xChangeDeviceKeyMappingReq *)rp)->keyCodes = 0;
		/* Test must supply data */
		break;
	case X_GetDeviceModifierMapping:
		GetReq(GetDeviceModifierMapping,rp);
		((xGetDeviceModifierMappingReq *)rp)->deviceid = 
		    Devs.Mod->device_id;
		break;
	case X_SetDeviceModifierMapping:
		GetReq(SetDeviceModifierMapping,rp);
		((xSetDeviceModifierMappingReq *)rp)->deviceid = 
		    Devs.Mod->device_id;
		((xSetDeviceModifierMappingReq *)rp)->numKeyPerModifier = 1;
		break;
	case X_GetDeviceButtonMapping:
		GetReq(GetDeviceButtonMapping,rp);
		((xGetDeviceButtonMappingReq *)rp)->deviceid = 
		    Devs.Button->device_id;
		break;
	case X_SetDeviceButtonMapping:
		GetReq(SetDeviceButtonMapping,rp);
		((xSetDeviceButtonMappingReq *)rp)->deviceid = 
		    Devs.Button->device_id;
		((xSetDeviceButtonMappingReq *)rp)->map_length = 
		    ButtonMapLength;
		break;
	case X_QueryDeviceState:
		GetReq(QueryDeviceState,rp);
		((xQueryDeviceStateReq *)rp)->deviceid = 
		    Devs.Any->device_id;
		break;
	case X_SendExtensionEvent:
		{
		int *cptr;
		GetReqExtra(SendExtensionEvent,2 * sizeof(xEvent)+ 8,rp);
		((xSendExtensionEventReq *)rp)->destination =
		    Get_Default_Window(client);
		((xSendExtensionEventReq *)rp)->deviceid = Devs.Key->device_id;
		((xSendExtensionEventReq *)rp)->propagate = True;
		((xSendExtensionEventReq *)rp)->count = 2;
		((xSendExtensionEventReq *)rp)->num_events = 2;
		memcpy ((char *) ((xSendExtensionEventReq *)rp+1), 
		    kev, 2 * sizeof(xEvent));
		cptr = (int *)((xSendExtensionEventReq *)rp+1);
		cptr += 16;
		for (i=0; i<2; i++)
		    *cptr++ = classes[i];
		}
		break;
	case X_DeviceBell:
		GetReq(DeviceBell,rp);
		((xDeviceBellReq *)rp)->deviceid = Devs.KbdFeed->device_id;
		((xDeviceBellReq *)rp)->feedbackid = 0;
		((xDeviceBellReq *)rp)->feedbackclass = KbdFeedbackClass;
		((xDeviceBellReq *)rp)->percent = 100;
		break;
	case X_SetDeviceValuators:
		GetReqExtra(SetDeviceValuators,4,rp);
		((xSetDeviceValuatorsReq *)rp)->deviceid =Devs.DvVal->device_id;
		((xSetDeviceValuatorsReq *)rp)->first_valuator = 0;
		((xSetDeviceValuatorsReq *)rp)->num_valuators = 1;
		*((int *)((xSetDeviceValuatorsReq *)rp+1)) = 1;
		break;
	case X_GetDeviceControl:
		GetReq(GetDeviceControl,rp);
		((xGetDeviceControlReq *)rp)->deviceid = Devs.DvCtl->device_id;
		((xGetDeviceControlReq *)rp)->control = DEVICE_RESOLUTION;
		break;
	case X_ChangeDeviceControl:
		GetReqExtra(ChangeDeviceControl,8,rp);
		((xChangeDeviceControlReq *)rp)->deviceid =Devs.DvCtl->device_id;
		((xChangeDeviceControlReq *)rp)->control = DEVICE_RESOLUTION;
		((xChangeDeviceControlReq *)rp)->pad0 = 0;
		((xDeviceResolutionCtl *)((xChangeDeviceControlReq *)rp+1))->control = DEVICE_RESOLUTION;
		((xDeviceResolutionCtl *)((xChangeDeviceControlReq *)rp+1))->length = sizeof(xDeviceResolutionCtl);
		((xDeviceResolutionCtl *)((xChangeDeviceControlReq *)rp+1))->first_valuator = 0;
		((xDeviceResolutionCtl *)((xChangeDeviceControlReq *)rp+1))->num_valuators = 0;
		break;
	default:
		DEFAULT_ERROR;
		break;
	}
	switch (Get_Test_Type(client)) {
	case SETUP:
	case GOOD:
	case OPEN_DISPLAY:
        case BAD_IDCHOICE1:
	case BAD_IDCHOICE2:
	case BAD_VALUE:
		break;
	case BAD_LENGTH:
	case JUST_TOO_LONG:
	case TOO_LONG:
		rp->length = bad_len;
		break;
	default:
		Log_Msg ("INTERNAL ERROR: Make_XInput_Req - bad test type %d\n", Get_Test_Type(client));
		Delete ();
		/*NOTREACHED*/
		break;
	}
	return(rp);
}


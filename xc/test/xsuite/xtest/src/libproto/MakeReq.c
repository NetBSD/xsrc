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
 * $XConsortium: MakeReq.c /main/6 1995/11/26 22:33:11 dpw $
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

#include "DataMove.h"
#include "XstlibInt.h"
#include <X11/Xatom.h>
#include <X11/X.h>

#define RED 65535       /* default red intensity, range is 0 to 65535 */
#define GREEN 65535     /* default green intensity, range is 0 to 65535 */
#define BLUE 65535      /* default blue intensity, range is 0 to 65535 */
#define NPLANES 0       /* default number of planes */
#define PLANEMASK 0     /* default plane mask */
#define TEST16_2 0x2129   /* ? character in test16 (16 bit) font */
#define TEST16_3 0x212a   /* ! character in test16 (16 bit) font */

CARD32 reply_pixel;
Atom Test_Atom1;
Atom Test_Atom2;
char *Test_prop = "__Test_Atom";
static int bad_len,
	   alloc_len; /* significantly different only in TOO_LONG tests */
static int this_client;

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
	req->reqType = X_##name
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
	req->reqType = X_/**/name
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
	req->reqType = X_##name;\
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
	req->reqType = X_/**/name;\
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
	req->reqType = X_##name
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
	req->reqType = X_/**/name
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
	req->reqType = X_##name;\
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
	req->reqType = X_/**/name;\
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
*Make_Req(client,type)
int client;
int type;
{
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
	case X_CreateWindow:
		GetReq(CreateWindow,rp);
		((xCreateWindowReq *)rp)->depth = CopyFromParent;
		((xCreateWindowReq *)rp)->wid = (Window) Get_Resource_Id(client);
		((xCreateWindowReq *)rp)->parent = Get_Root_Id(client);
		((xCreateWindowReq *)rp)->x = 0;
		((xCreateWindowReq *)rp)->y = 0;
		((xCreateWindowReq *)rp)->width = Get_Screen_Width(client)/2;
		((xCreateWindowReq *)rp)->height = Get_Screen_Height(client)/2;
		((xCreateWindowReq *)rp)->borderWidth = 0;
		((xCreateWindowReq *)rp)->class = InputOutput;
		((xCreateWindowReq *)rp)->visual = CopyFromParent;
		((xCreateWindowReq *)rp)->mask = 0;	/* no values yet */
		rp = Add_Masked_Value(rp,CWBackPixel,(unsigned long) 1);
		rp = Add_Masked_Value(rp,CWEventMask,(unsigned long)ExposureMask);
		rp = Add_Masked_Value(rp,CWOverrideRedirect,(unsigned long)Xst_override);
		break;
	case X_ChangeWindowAttributes:
		GetReq(ChangeWindowAttributes,rp);
		((xChangeWindowAttributesReq *)rp)->window = Get_Default_Window(client);
		((xChangeWindowAttributesReq *)rp)->valueMask = 0;
		rp = Add_Masked_Value(rp,CWWinGravity,(unsigned long)SouthEastGravity);
		if (Get_Test_Type(this_client) == JUST_TOO_LONG)
		    bad_len++;
		break;
	case X_GetWindowAttributes:
		GetResReq(GetWindowAttributes,Get_Default_Window(client),rp);
		break;
	case X_DestroyWindow:
		GetResReq(DestroyWindow,Get_Default_Window(client),rp);
		break;
	case X_DestroySubwindows:
		GetResReq(DestroySubwindows,Get_Default_Window(client),rp);
		break;
	case X_ChangeSaveSet:
		GetReq(ChangeSaveSet,rp);
		((xChangeSaveSetReq *)rp)->mode = SetModeInsert;
		((xChangeSaveSetReq *)rp)->window = Get_Default_Window(client);
		break;
	case X_ReparentWindow:
		GetReq(ReparentWindow,rp);
		((xReparentWindowReq *)rp)->window = Get_Default_Window(client);
		((xReparentWindowReq *)rp)->parent = Get_Root_Id(client);
		((xReparentWindowReq *)rp)->x = 0;
		((xReparentWindowReq *)rp)->y = 0;
		break;
	case X_MapWindow:
		GetResReq(MapWindow,Get_Default_Window(client),rp);
		break;
	case X_MapSubwindows:
		GetResReq(MapSubwindows,Get_Default_Window(client),rp);
		break;
	case X_UnmapWindow:
		GetResReq(UnmapWindow,Get_Default_Window(client),rp);
		break;
	case X_UnmapSubwindows:
		GetResReq(UnmapSubwindows,Get_Default_Window(client),rp);
		break;
	case X_ConfigureWindow:
		GetReq(ConfigureWindow,rp);
		((xConfigureWindowReq *)rp)->window = Get_Default_Window(client);
		((xConfigureWindowReq *)rp)->mask = 0;
		rp = Add_Masked_Value(rp,CWY,(unsigned long)(Get_Screen_Height(client)/2 + 2));
		if (Get_Test_Type(this_client) == JUST_TOO_LONG)
		    bad_len++;
		break;
	case X_CirculateWindow:
		GetReq(CirculateWindow,rp);
		((xCirculateWindowReq *)rp)->direction = RaiseLowest;
		((xCirculateWindowReq *)rp)->window = Get_Default_Window(client);
		break;
	case X_GetGeometry:
		GetResReq(GetGeometry,Get_Default_Window(client),rp);
		break;
	case X_QueryTree:
		GetResReq(QueryTree,Get_Default_Window(client),rp);
		break;
	case X_InternAtom:
		GetReqExtra(InternAtom,((int)(3+strlen(Test_prop))>>2) << 2,rp);
		((xInternAtomReq *)rp)->onlyIfExists = False;
		((xInternAtomReq *)rp)->nbytes = strlen(Test_prop);
		Copy_Padded_String8(&valuePtr, Test_prop);
		break;
	case X_GetAtomName:
		GetResReq(GetAtomName,XA_NOTICE,rp);
		break;
	case X_ChangeProperty:
		GetReqExtra(ChangeProperty,15,rp);
		((xChangePropertyReq *)rp)->mode = PropModeReplace;
		((xChangePropertyReq *)rp)->window = Get_Default_Window(client);
		((xChangePropertyReq *)rp)->property = Get_Default_Atom(client);
		((xChangePropertyReq *)rp)->type = XA_STRING;
		((xChangePropertyReq *)rp)->format = 8;
		((xChangePropertyReq *)rp)->nUnits = 15;
		Copy_Padded_String8(&valuePtr, "Test prop data.");
		break;
	case X_DeleteProperty:
		GetReq(DeleteProperty,rp);
		((xDeletePropertyReq *)rp)->window = Get_Default_Window(client);
		((xDeletePropertyReq *)rp)->property = Get_Default_Atom(client);
		break;
	case X_GetProperty:
		GetReq(GetProperty,rp);
		((xGetPropertyReq *)rp)->delete = False;
		((xGetPropertyReq *)rp)->window = Get_Default_Window(client);
		((xGetPropertyReq *)rp)->property = Get_Default_Atom(client);
		((xGetPropertyReq *)rp)->type = AnyPropertyType;
		((xGetPropertyReq *)rp)->longOffset = 0;
		((xGetPropertyReq *)rp)->longLength = 100;
		break;
	case X_ListProperties:
		GetResReq(ListProperties,Get_Default_Window(client),rp);
		break;
	case X_SetSelectionOwner:
		GetReq(SetSelectionOwner,rp);
		((xSetSelectionOwnerReq *)rp)->window = Get_Default_Window(client);
		((xSetSelectionOwnerReq *)rp)->selection = Get_Default_Atom(client);
		((xSetSelectionOwnerReq *)rp)->time = CurrentTime;
		break;
	case X_GetSelectionOwner:
		GetResReq(GetSelectionOwner,Get_Default_Atom(client),rp);
		break;
	case X_ConvertSelection:
		GetReq(ConvertSelection,rp);
		((xConvertSelectionReq *)rp)->requestor = Get_Default_Window(client);
		((xConvertSelectionReq *)rp)->selection = Get_Default_Atom(client);
		((xConvertSelectionReq *)rp)->target = 31; /* STRING */
		((xConvertSelectionReq *)rp)->property = Get_Default_Atom(client);
		((xConvertSelectionReq *)rp)->time = Gen_Good_time(client);
		break;
	case X_SendEvent:
		GetReq(SendEvent,rp);
		((xSendEventReq *)rp)->propagate = False;
		((xSendEventReq *)rp)->destination = Get_Default_Window(client);
		((xSendEventReq *)rp)->eventMask = NoEventMask;
		((xSendEventReq *)rp)->event = Get_Default_Event(client);
		break;
	case X_GrabPointer:
		GetReq(GrabPointer,rp);
		((xGrabPointerReq *)rp)->ownerEvents = False;
		((xGrabPointerReq *)rp)->grabWindow = Get_Default_Window(client);
		((xGrabPointerReq *)rp)->eventMask = 0x7ffc;
		((xGrabPointerReq *)rp)->pointerMode = GrabModeAsync;
		((xGrabPointerReq *)rp)->keyboardMode = GrabModeAsync;
		((xGrabPointerReq *)rp)->confineTo = None;
		((xGrabPointerReq *)rp)->cursor = None;
		((xGrabPointerReq *)rp)->time = CurrentTime;
		break;
	case X_UngrabPointer:
		GetResReq(UngrabPointer,CurrentTime,rp);
		break;
	case X_GrabButton:
		GetReq(GrabButton,rp);
		((xGrabButtonReq *)rp)->ownerEvents = False;
		((xGrabButtonReq *)rp)->grabWindow = Get_Default_Window(client);
		((xGrabButtonReq *)rp)->eventMask = 0x7ffc;
		((xGrabButtonReq *)rp)->pointerMode = GrabModeAsync;
		((xGrabButtonReq *)rp)->keyboardMode = GrabModeAsync;
		((xGrabButtonReq *)rp)->confineTo = None;
		((xGrabButtonReq *)rp)->cursor = None;
		((xGrabButtonReq *)rp)->button = AnyButton;
		((xGrabButtonReq *)rp)->modifiers = AnyModifier;
		break;
	case X_UngrabButton:
		GetReq(UngrabButton,rp);
		((xUngrabButtonReq *)rp)->button = AnyButton;
		((xUngrabButtonReq *)rp)->grabWindow = Get_Default_Window(client);
		((xUngrabButtonReq *)rp)->modifiers = AnyModifier;
		break;
	case X_ChangeActivePointerGrab:
		GetReq(ChangeActivePointerGrab,rp);
		((xChangeActivePointerGrabReq *)rp)->cursor = None;
		((xChangeActivePointerGrabReq *)rp)->time = CurrentTime;
		((xChangeActivePointerGrabReq *)rp)->eventMask = NoEventMask;
		break;
	case X_GrabKeyboard:
		GetReq(GrabKeyboard,rp);
		((xGrabKeyboardReq *)rp)->ownerEvents = False;
		((xGrabKeyboardReq *)rp)->grabWindow = Get_Default_Window(client);
		((xGrabKeyboardReq *)rp)->time = CurrentTime;
		((xGrabKeyboardReq *)rp)->pointerMode = GrabModeAsync;
		((xGrabKeyboardReq *)rp)->keyboardMode = GrabModeAsync;
		break;
	case X_UngrabKeyboard:
		GetResReq(UngrabKeyboard,CurrentTime,rp);
		break;
	case X_GrabKey:
		GetReq(GrabKey,rp);
		((xGrabKeyReq *)rp)->ownerEvents = False;
		((xGrabKeyReq *)rp)->grabWindow = Get_Default_Window(client);
		((xGrabKeyReq *)rp)->modifiers = AnyModifier;
		((xGrabKeyReq *)rp)->key = AnyKey;
		((xGrabKeyReq *)rp)->pointerMode = GrabModeAsync;
		((xGrabKeyReq *)rp)->keyboardMode = GrabModeAsync;
		break;
	case X_UngrabKey:
		GetReq(UngrabKey,rp);
		((xUngrabKeyReq *)rp)->key = AnyKey;
		((xUngrabKeyReq *)rp)->grabWindow = Get_Default_Window(client);
		((xUngrabKeyReq *)rp)->modifiers = AnyModifier;
		break;
	case X_AllowEvents:
		GetReq(AllowEvents,rp);
		((xAllowEventsReq *)rp)->mode = AsyncPointer;
		((xAllowEventsReq *)rp)->time = CurrentTime;
		break;
	case X_GrabServer:
		GetEmptyReq(GrabServer,rp);
		break;
	case X_UngrabServer:
		GetEmptyReq(UngrabServer,rp);
		break;
	case X_QueryPointer:
		GetResReq(QueryPointer,Get_Default_Window(client),rp);
		break;
	case X_GetMotionEvents:
		GetReq(GetMotionEvents,rp);
		((xGetMotionEventsReq *)rp)->window = Get_Default_Window(client);
		((xGetMotionEventsReq *)rp)->start = CurrentTime;
		((xGetMotionEventsReq *)rp)->stop = CurrentTime;
		break;
	case X_TranslateCoords:
		GetReq(TranslateCoords,rp);
		((xTranslateCoordsReq *)rp)->srcWid = Get_Default_Window(client);
		((xTranslateCoordsReq *)rp)->dstWid = Get_Default_Window(client);
		((xTranslateCoordsReq *)rp)->srcX = Gen_Good_srcX(client);
		((xTranslateCoordsReq *)rp)->srcY = Gen_Good_srcY(client);
		break;
	case X_WarpPointer:
		GetReq(WarpPointer,rp);
		((xWarpPointerReq *)rp)->srcWid = None;
		((xWarpPointerReq *)rp)->dstWid = None;
		((xWarpPointerReq *)rp)->srcX = 0;
		((xWarpPointerReq *)rp)->srcY = 0;
		((xWarpPointerReq *)rp)->srcWidth = 0;
		((xWarpPointerReq *)rp)->srcHeight = 0;
		((xWarpPointerReq *)rp)->dstX = Get_Screen_Width(client) / 4;
		((xWarpPointerReq *)rp)->dstY = Get_Screen_Height(client) / 4;
		break;
	case X_SetInputFocus:
		GetReq(SetInputFocus,rp);
		((xSetInputFocusReq *)rp)->revertTo = RevertToPointerRoot;
		((xSetInputFocusReq *)rp)->focus = Get_Default_Window(client);
		((xSetInputFocusReq *)rp)->time = CurrentTime;
		break;
	case X_GetInputFocus:
		GetEmptyReq(GetInputFocus,rp);
		break;
	case X_QueryKeymap:
		GetEmptyReq(QueryKeymap,rp);
		break;
	case X_OpenFont:
		GetReqExtra(OpenFont,(int)strlen(Xst_def_font8),rp);
		((xOpenFontReq *)rp)->fid = (Font) Get_Resource_Id(client);
		((xOpenFontReq *)rp)->nbytes = strlen(Xst_def_font8);
		Copy_Padded_String8(&valuePtr, Xst_def_font8);
		break;
	case X_CloseFont:
		GetResReq(CloseFont,Get_Default_Font(client),rp);
		break;
	case X_QueryFont:
		GetResReq(QueryFont,Get_Default_Font(client),rp);
		break;
	case X_QueryTextExtents:
		GetReqExtra(QueryTextExtents,10,rp);
		((xQueryTextExtentsReq *)rp)->oddLength = False;
		((xQueryTextExtentsReq *)rp)->fid = Get_Default_Font(client);
		Copy_Padded_String16(&valuePtr, "[a-z]");
		break;
	case X_ListFonts:
		GetReqExtra(ListFonts,1,rp);
		((xListFontsReq *)rp)->maxNames = 200;
		((xListFontsReq *)rp)->nbytes = 1;
		Copy_Padded_String8(&valuePtr, "xtfont1");
		break;
	case X_ListFontsWithInfo:
		GetReqExtra(ListFontsWithInfo,1,rp);
		((xListFontsWithInfoReq *)rp)->maxNames = 200;
		((xListFontsWithInfoReq *)rp)->nbytes = 1;
		Copy_Padded_String8(&valuePtr, "xtfont1");
		break;
	case X_SetFontPath:
		GetReqExtra(SetFontPath,0,rp);
		((xSetFontPathReq *)rp)->nFonts = 0;
	    /*
	     * NOTE - This requires a value list if you want to send 
	     * anything besides 'nFonts=0'
	     */
		break;
	case X_GetFontPath:
		GetEmptyReq(GetFontPath,rp);
		break;
	case X_CreatePixmap:
		GetReq(CreatePixmap,rp);
		((xCreatePixmapReq *)rp)->depth = Gen_Good_depth(client);
		((xCreatePixmapReq *)rp)->pid = (Pixmap) Get_Resource_Id(client);
		((xCreatePixmapReq *)rp)->drawable = Get_Default_Window(client);
		((xCreatePixmapReq *)rp)->width = Get_Screen_Width(client)/2;
		((xCreatePixmapReq *)rp)->height = Get_Screen_Height(client)/2;
		break;
	case X_FreePixmap:
		GetResReq(FreePixmap,Get_Default_Pixmap(client),rp);
		break;
	case X_CreateGC:
		GetReq(CreateGC,rp);
		((xCreateGCReq *)rp)->gc = (GContext) Get_Resource_Id(client);
		((xCreateGCReq *)rp)->drawable = Get_Default_Window(client);
		((xCreateGCReq *)rp)->mask = 0;
		break;
	case X_ChangeGC:
		GetReq(ChangeGC,rp);
		((xChangeGCReq *)rp)->gc = Get_Default_GContext(client);
		((xChangeGCReq *)rp)->mask = 0;
		rp = Add_Masked_Value(rp,GCLineStyle,(unsigned long)LineOnOffDash);
		if (Get_Test_Type(this_client) == JUST_TOO_LONG)
		    bad_len++;
		break;
	case X_CopyGC:
		GetReq(CopyGC,rp);
		((xCopyGCReq *)rp)->srcGC = Get_Default_GContext(client);
		((xCopyGCReq *)rp)->dstGC = Get_Default_GContext(client);
		((xCopyGCReq *)rp)->mask = GCLineStyle;
		break;
	case X_SetDashes:
		GetReqExtra(SetDashes,4,rp);
		((xSetDashesReq *)rp)->gc = Get_Default_GContext(client);
		((xSetDashesReq *)rp)->dashOffset = 6;
		((xSetDashesReq *)rp)->nDashes = 4;
		Set_Value1(&valuePtr,2);
		Set_Value1(&valuePtr,2);
		Set_Value1(&valuePtr,2);
		Set_Value1(&valuePtr,6);
		break;
	case X_SetClipRectangles:
		GetReqExtra(SetClipRectangles,8,rp);
		((xSetClipRectanglesReq *)rp)->ordering = Unsorted;
		((xSetClipRectanglesReq *)rp)->gc = Get_Default_GContext(client);
		((xSetClipRectanglesReq *)rp)->xOrigin = 0;
		((xSetClipRectanglesReq *)rp)->yOrigin = 0;
		Set_Value2(&valuePtr,0);	/* x */
		Set_Value2(&valuePtr,0);	/* y */
		Set_Value2(&valuePtr,Get_Screen_Width(client)/4);	/* width */
		Set_Value2(&valuePtr,Get_Screen_Height(client)/4);	/* height */
		break;
	case X_FreeGC:
		GetResReq(FreeGC,Get_Default_GContext(client),rp);
		break;
	case X_ClearArea:
		GetReq(ClearArea,rp);
		((xClearAreaReq *)rp)->exposures = True;
		((xClearAreaReq *)rp)->window = Get_Default_Window(client);
		((xClearAreaReq *)rp)->x = 0;
		((xClearAreaReq *)rp)->y = 0;
		((xClearAreaReq *)rp)->width = 0;
		((xClearAreaReq *)rp)->height = 0;
		break;
	case X_CopyArea:
		GetReq(CopyArea,rp);
		((xCopyAreaReq *)rp)->srcDrawable = Get_Default_Window(client);
		((xCopyAreaReq *)rp)->dstDrawable = Get_Default_Window(client);
		((xCopyAreaReq *)rp)->gc = Get_Default_GContext(client);
		((xCopyAreaReq *)rp)->srcX = -Get_Screen_Width(client)/4;
		((xCopyAreaReq *)rp)->srcY = -Get_Screen_Height(client)/4;
		((xCopyAreaReq *)rp)->dstX = 0;
		((xCopyAreaReq *)rp)->dstY = 0;
		((xCopyAreaReq *)rp)->width = Get_Screen_Width(client)/4;
		((xCopyAreaReq *)rp)->height = Get_Screen_Height(client)/4;
		break;
	case X_CopyPlane:
		GetReq(CopyPlane,rp);
		((xCopyPlaneReq *)rp)->srcDrawable = Get_Default_Window(client);
		((xCopyPlaneReq *)rp)->dstDrawable = Get_Default_Window(client);
		((xCopyPlaneReq *)rp)->gc = Get_Default_GContext(client);
		((xCopyPlaneReq *)rp)->srcX = -Get_Screen_Width(client)/4;
		((xCopyPlaneReq *)rp)->srcY = -Get_Screen_Height(client)/4;
		((xCopyPlaneReq *)rp)->dstX = 0;
		((xCopyPlaneReq *)rp)->dstY = 0;
		((xCopyPlaneReq *)rp)->width = Get_Screen_Width(client)/4;
		((xCopyPlaneReq *)rp)->height = Get_Screen_Height(client)/4;
		((xCopyPlaneReq *)rp)->bitPlane = 1;
		break;
	case X_PolyPoint:
		GetReqExtra(PolyPoint,12,rp);
		((xPolyPointReq *)rp)->coordMode = CoordModeOrigin;
		((xPolyPointReq *)rp)->drawable = Get_Default_Window(client);
		((xPolyPointReq *)rp)->gc = Get_Default_GContext(client);
		Set_Value2(&valuePtr,1);	/* x[0] */
		Set_Value2(&valuePtr,1);	/* y[0] */
		Set_Value2(&valuePtr,Get_Screen_Width(client)/4);	/* x[1] */
		Set_Value2(&valuePtr,Get_Screen_Height(client)/4);	/* y[1] */
		Set_Value2(&valuePtr,(Get_Screen_Width(client)/2)-1);	/* x[2] */
		Set_Value2(&valuePtr,(Get_Screen_Height(client)/2)-1);	/* y[2] */

		break;
	case X_PolyLine:
		GetReqExtra(PolyLine,32,rp);
		((xPolyLineReq *)rp)->coordMode = CoordModeOrigin;
		((xPolyLineReq *)rp)->drawable = Get_Default_Window(client);
		((xPolyLineReq *)rp)->gc = Get_Default_GContext(client);
		Set_Value2(&valuePtr,1);
		Set_Value2(&valuePtr,1);
		Set_Value2(&valuePtr,Get_Screen_Width(client)/4);
		Set_Value2(&valuePtr,Get_Screen_Height(client)/4);
		Set_Value2(&valuePtr,Get_Screen_Width(client)/2-1);
		Set_Value2(&valuePtr,Get_Screen_Height(client)/2-1);
		Set_Value2(&valuePtr,Get_Screen_Width(client)/4);
		Set_Value2(&valuePtr,Get_Screen_Height(client)/4);
		Set_Value2(&valuePtr,1);
		Set_Value2(&valuePtr,Get_Screen_Height(client)/2-1);
		Set_Value2(&valuePtr,Get_Screen_Width(client)/4);
		Set_Value2(&valuePtr,Get_Screen_Height(client)/4);
		Set_Value2(&valuePtr,Get_Screen_Width(client)/2-1);
		Set_Value2(&valuePtr,1);
		Set_Value2(&valuePtr,Get_Screen_Width(client)/4);
		Set_Value2(&valuePtr,Get_Screen_Height(client)/4);
		break;
	case X_PolySegment:
		GetReqExtra(PolySegment,32,rp);
		((xPolySegmentReq *)rp)->drawable = Get_Default_Window(client);
		((xPolySegmentReq *)rp)->gc = Get_Default_GContext(client);
		Set_Value2(&valuePtr,1);
		Set_Value2(&valuePtr,1);
		Set_Value2(&valuePtr,Get_Screen_Width(client)/4);
		Set_Value2(&valuePtr,Get_Screen_Height(client)/4);
		Set_Value2(&valuePtr,Get_Screen_Width(client)/2-1);
		Set_Value2(&valuePtr,Get_Screen_Height(client)/2-1);
		Set_Value2(&valuePtr,Get_Screen_Width(client)/4);
		Set_Value2(&valuePtr,Get_Screen_Height(client)/4);
		Set_Value2(&valuePtr,1);
		Set_Value2(&valuePtr,Get_Screen_Height(client)/2-1);
		Set_Value2(&valuePtr,Get_Screen_Width(client)/4);
		Set_Value2(&valuePtr,Get_Screen_Height(client)/4);
		Set_Value2(&valuePtr,Get_Screen_Width(client)/2-1);
		Set_Value2(&valuePtr,1);
		Set_Value2(&valuePtr,Get_Screen_Width(client)/4);
		Set_Value2(&valuePtr,Get_Screen_Height(client)/4);
		break;
	case X_PolyRectangle:
		GetReqExtra(PolyRectangle,16,rp);
		((xPolyRectangleReq *)rp)->drawable = Get_Default_Window(client);
		((xPolyRectangleReq *)rp)->gc = Get_Default_GContext(client);
		Set_Value2(&valuePtr,1);
		Set_Value2(&valuePtr,1);
		Set_Value2(&valuePtr,Get_Screen_Width(client)/4);
		Set_Value2(&valuePtr,Get_Screen_Height(client)/4);
		Set_Value2(&valuePtr,Get_Screen_Width(client)/8);
		Set_Value2(&valuePtr,Get_Screen_Height(client)/8);
		Set_Value2(&valuePtr,Get_Screen_Width(client)/4);
		Set_Value2(&valuePtr,Get_Screen_Height(client)/4);
		break;
	case X_PolyArc:
		GetReqExtra(PolyArc,12,rp);
		((xPolyArcReq *)rp)->drawable = Get_Default_Window(client);
		((xPolyArcReq *)rp)->gc = Get_Default_GContext(client);
		Set_Value2(&valuePtr,Get_Screen_Width(client)/8);
		Set_Value2(&valuePtr,Get_Screen_Height(client)/8);
		Set_Value2(&valuePtr,Get_Screen_Width(client)/4);
		Set_Value2(&valuePtr,Get_Screen_Height(client)/8);
		Set_Value2(&valuePtr,90*64);
		Set_Value2(&valuePtr,360*64);
		break;
	case X_FillPoly:
		GetReqExtra(FillPoly,16,rp);
		((xFillPolyReq *)rp)->drawable = Get_Default_Window(client);
		((xFillPolyReq *)rp)->gc = Get_Default_GContext(client);
		((xFillPolyReq *)rp)->shape = Complex;
		((xFillPolyReq *)rp)->coordMode = CoordModeOrigin;
		Set_Value2(&valuePtr,1);
		Set_Value2(&valuePtr,1);
		Set_Value2(&valuePtr,1);
		Set_Value2(&valuePtr,Get_Screen_Height(client)/4);
		Set_Value2(&valuePtr,Get_Screen_Width(client)/4);
		Set_Value2(&valuePtr,Get_Screen_Height(client)/4);
		Set_Value2(&valuePtr,Get_Screen_Width(client)/4);
		Set_Value2(&valuePtr,1);
		break;
	case X_PolyFillRectangle:
		GetReqExtra(PolyFillRectangle,16,rp);
		((xPolyFillRectangleReq *)rp)->drawable = Get_Default_Window(client);
		((xPolyFillRectangleReq *)rp)->gc = Get_Default_GContext(client);
		Set_Value2(&valuePtr,1);
		Set_Value2(&valuePtr,1);
		Set_Value2(&valuePtr,Get_Screen_Width(client)/4);
		Set_Value2(&valuePtr,Get_Screen_Height(client)/4);
		Set_Value2(&valuePtr,Get_Screen_Width(client)/8);
		Set_Value2(&valuePtr,Get_Screen_Height(client)/8);
		Set_Value2(&valuePtr,Get_Screen_Width(client)/4);
		Set_Value2(&valuePtr,Get_Screen_Height(client)/4);
		break;
	case X_PolyFillArc:
		GetReqExtra(PolyFillArc,12,rp);
		((xPolyFillArcReq *)rp)->drawable = Get_Default_Window(client);
		((xPolyFillArcReq *)rp)->gc = Get_Default_GContext(client);
		Set_Value2(&valuePtr,Get_Screen_Width(client)/8);
		Set_Value2(&valuePtr,Get_Screen_Height(client)/8);
		Set_Value2(&valuePtr,Get_Screen_Width(client)/4);
		Set_Value2(&valuePtr,Get_Screen_Height(client)/8);
		Set_Value2(&valuePtr,90*64);
		Set_Value2(&valuePtr,360*64);
		break;
	case X_PutImage:
		GetReq(PutImage,rp);
		((xPutImageReq *)rp)->format = XYBitmap;
		((xPutImageReq *)rp)->drawable = Get_Default_Window(client);
		((xPutImageReq *)rp)->gc = Get_Default_GContext(client);
		((xPutImageReq *)rp)->width = Get_Screen_Width(client)/2;
		((xPutImageReq *)rp)->height = Get_Screen_Height(client)/2;
		((xPutImageReq *)rp)->dstX = 0;
		((xPutImageReq *)rp)->dstY = 0;
		((xPutImageReq *)rp)->leftPad = 0;
		((xPutImageReq *)rp)->depth = 1;
		/* Test must provide the data */
		break;
	case X_GetImage:
		GetReq(GetImage,rp);
		((xPutImageReq *)rp)->format = XYPixmap;
		((xGetImageReq *)rp)->drawable = Get_Default_Window(client);
		((xGetImageReq *)rp)->x = 0;
		((xGetImageReq *)rp)->y = 0;
		((xGetImageReq *)rp)->width = Get_Screen_Width(client)/2;
		((xGetImageReq *)rp)->height = Get_Screen_Height(client)/2;
		((xGetImageReq *)rp)->planeMask = 1;
		break;
	case X_PolyText8:
		GetReqExtra(PolyText8,31,rp);
		((xPolyText8Req *)rp)->drawable = Get_Default_Window(client);
		((xPolyText8Req *)rp)->gc = Get_Default_GContext(client);
		((xPolyText8Req *)rp)->x = Get_Screen_Width(client)/8;
		((xPolyText8Req *)rp)->y = Get_Screen_Height(client)/8;
		Set_Value1(&valuePtr, 12);
		Set_Value1(&valuePtr, 0);
		Copy_String8(&valuePtr, "Hello World!");
		Set_Value1(&valuePtr, 15);
		Set_Value1(&valuePtr, 30);
		Copy_String8(&valuePtr, "Parle vu Norsk?");
		if (Get_Test_Type(this_client) == JUST_TOO_LONG) {
			Set_Value1(&valuePtr, 4);
			Set_Value1(&valuePtr, 0);
			Copy_String8(&valuePtr, "ABC");
		} else {
			Set_Value1(&valuePtr, 0);
		}
		break;
	case X_PolyText16:
		GetReqExtra(PolyText16,8,rp);
		((xPolyText16Req *)rp)->drawable = Get_Default_Window(client);
		((xPolyText16Req *)rp)->gc = Get_Default_GContext(client);
		((xPolyText16Req *)rp)->x = Get_Screen_Width(client)/8;
		((xPolyText16Req *)rp)->y = Get_Screen_Height(client)/8;
		Set_Value1(&valuePtr, 2);
		Set_Value1(&valuePtr, 0);
		Set_Value2(&valuePtr,TEST16_2);
		Set_Value2(&valuePtr,TEST16_3);
		if (Get_Test_Type(this_client) == JUST_TOO_LONG) {
			Set_Value1(&valuePtr, 4);
			Set_Value1(&valuePtr, 0);
			Set_Value2(&valuePtr, TEST16_2);
			Set_Value2(&valuePtr, TEST16_3);
		} else {
			Set_Value2(&valuePtr,0);          /* pad up to longword */
		}
		break;
	case X_ImageText8:
		GetReqExtra(ImageText8,12,rp);
		((xImageText8Req *)rp)->nChars = 12;
		((xImageText8Req *)rp)->drawable = Get_Default_Window(client);
		((xImageText8Req *)rp)->gc = Get_Default_GContext(client);
		((xImageText8Req *)rp)->x = Get_Screen_Width(client)/8;
		((xImageText8Req *)rp)->y = Get_Screen_Height(client)/8;
		Copy_Padded_String8(&valuePtr, "Hello World!");
		break;
	case X_ImageText16:
		GetReqExtra(ImageText16,8,rp);
		((xImageText16Req *)rp)->nChars = 4;
		((xImageText16Req *)rp)->drawable = Get_Default_Window(client);
		((xImageText16Req *)rp)->gc = Get_Default_GContext(client);
		((xImageText16Req *)rp)->x = Get_Screen_Width(client)/8;
		((xImageText16Req *)rp)->y = Get_Screen_Height(client)/8;
		Set_Value2(&valuePtr,TEST16_2);
		Set_Value2(&valuePtr,TEST16_3);
		Set_Value2(&valuePtr,TEST16_3);
		Set_Value2(&valuePtr,TEST16_2);
		break;
	case X_CreateColormap:
		GetReq(CreateColormap,rp);
		((xCreateColormapReq *)rp)->alloc = AllocNone;
		((xCreateColormapReq *)rp)->mid = (Colormap) Get_Resource_Id(client);
		((xCreateColormapReq *)rp)->window = Get_Default_Window(client);
		((xCreateColormapReq *)rp)->visual = Gen_Good_Visual(client,XstDefaultScreen (Get_Display (client)));
		break;
	case X_FreeColormap:
		GetResReq(FreeColormap,Get_Default_Colormap(client),rp);
		break;
	case X_CopyColormapAndFree:
		GetReq(CopyColormapAndFree,rp);
		((xCopyColormapAndFreeReq *)rp)->mid = (Colormap) Get_Resource_Id(client);
		((xCopyColormapAndFreeReq *)rp)->srcCmap = Get_Default_Colormap(client);
		break;
	case X_InstallColormap:
		GetResReq(InstallColormap,Get_Default_Colormap(client),rp);
		break;
	case X_UninstallColormap:
		GetResReq(UninstallColormap,Get_Default_Colormap(client),rp);
		break;
	case X_ListInstalledColormaps:
		GetResReq(ListInstalledColormaps,Get_Default_Window(client),rp);
		break;
	case X_AllocColor:
		GetReq(AllocColor,rp);
		((xAllocColorReq *)rp)->cmap = Get_Default_Colormap(client);
		((xAllocColorReq *)rp)->red = RED;
		((xAllocColorReq *)rp)->green = GREEN;
		((xAllocColorReq *)rp)->blue = BLUE;
		break;
	case X_AllocNamedColor:
		GetReqExtra(AllocNamedColor,5,rp);
		((xAllocNamedColorReq *)rp)->cmap = Get_Default_Colormap(client);
		((xAllocNamedColorReq *)rp)->nbytes = 5;
		Copy_Padded_String8(&valuePtr, "black");
		break;
	case X_AllocColorCells:
		GetReq(AllocColorCells,rp);
		((xAllocColorCellsReq *)rp)->contiguous = False;
		((xAllocColorCellsReq *)rp)->cmap = Get_Default_Colormap(client);
		((xAllocColorCellsReq *)rp)->colors = Get_Maxsize(client);
		((xAllocColorCellsReq *)rp)->planes = NPLANES;
		break;
	case X_AllocColorPlanes:
		GetReq(AllocColorPlanes,rp);
		((xAllocColorPlanesReq *)rp)->contiguous = False;
		((xAllocColorPlanesReq *)rp)->cmap = Get_Default_Colormap(client);
		((xAllocColorPlanesReq *)rp)->colors = 1;
		((xAllocColorPlanesReq *)rp)->red = 0;
		((xAllocColorPlanesReq *)rp)->green = 0;
		((xAllocColorPlanesReq *)rp)->blue = 0;
		break;
	case X_FreeColors:
		GetReqExtra(FreeColors,4,rp);
		((xFreeColorsReq *)rp)->cmap = Get_Default_Colormap(client);
		((xFreeColorsReq *)rp)->planeMask = PLANEMASK;
		Set_Value4(&valuePtr,reply_pixel); /* pixel */
		break;
	case X_StoreColors:
		GetReqExtra(StoreColors,12,rp);
		((xStoreColorsReq *)rp)->cmap = Get_Default_Colormap(client);
		Set_Value4(&valuePtr,reply_pixel); /* pixel */
		Set_Value2(&valuePtr,RED); /* red */
		Set_Value2(&valuePtr,GREEN); /* green */
		Set_Value2(&valuePtr,BLUE); /* blue */
		Set_Value1(&valuePtr,DoRed | DoGreen | DoBlue); /* flags */
		Set_Value1(&valuePtr,0); /* pad */
		break;
	case X_StoreNamedColor:
		GetReqExtra(StoreNamedColor,5,rp);
		((xStoreNamedColorReq *)rp)->flags = DoRed | DoGreen | DoBlue;
		((xStoreNamedColorReq *)rp)->cmap = Get_Default_Colormap(client);
		((xStoreNamedColorReq *)rp)->pixel = reply_pixel;
		((xStoreNamedColorReq *)rp)->nbytes = 5;
		Copy_Padded_String8(&valuePtr,"black");
		break;
	case X_QueryColors:
		GetReqExtra(QueryColors,4,rp);
		((xQueryColorsReq *)rp)->cmap = Get_Default_Colormap(client);
		Set_Value4(&valuePtr,reply_pixel); /* pixel */
		break;
	case X_LookupColor:
		GetReqExtra(LookupColor,5,rp);
		((xLookupColorReq *)rp)->cmap = Get_Default_Colormap(client);
		((xLookupColorReq *)rp)->nbytes = 5;
		Copy_Padded_String8(&valuePtr, "black");
		break;
	case X_CreateCursor:
		GetReq(CreateCursor,rp);
		((xCreateCursorReq *)rp)->cid = (Cursor) Get_Resource_Id(client);
		((xCreateCursorReq *)rp)->source = Get_Default_Cursor_Pixmap(client);
		((xCreateCursorReq *)rp)->mask = Get_Default_Cursor_Pixmap(client);
		((xCreateCursorReq *)rp)->foreRed = 1;
		((xCreateCursorReq *)rp)->foreGreen = 1;
		((xCreateCursorReq *)rp)->foreBlue = 1;
		((xCreateCursorReq *)rp)->backRed = 0;
		((xCreateCursorReq *)rp)->backGreen = 0;
		((xCreateCursorReq *)rp)->backBlue = 0;
		((xCreateCursorReq *)rp)->x = 8;
		((xCreateCursorReq *)rp)->y = 8;
		break;
	case X_CreateGlyphCursor:
		GetReq(CreateGlyphCursor,rp);
		((xCreateGlyphCursorReq *)rp)->cid = (Cursor) Get_Resource_Id(client);
		((xCreateGlyphCursorReq *)rp)->source = Get_Default_Font(client);
		((xCreateGlyphCursorReq *)rp)->mask = Get_Default_Font(client);
		/* sourceChar and maskChar are valid for xtfont0. */
		((xCreateGlyphCursorReq *)rp)->sourceChar = 1;
		((xCreateGlyphCursorReq *)rp)->maskChar = 1;
		((xCreateGlyphCursorReq *)rp)->foreRed = 1;
		((xCreateGlyphCursorReq *)rp)->foreGreen = 1;
		((xCreateGlyphCursorReq *)rp)->foreBlue = 1;
		((xCreateGlyphCursorReq *)rp)->backRed = 0;
		((xCreateGlyphCursorReq *)rp)->backGreen = 0;
		((xCreateGlyphCursorReq *)rp)->backBlue = 0;
		break;
	case X_FreeCursor:
		GetResReq(FreeCursor,Get_Default_Cursor(client),rp);
		break;
	case X_RecolorCursor:
		GetReq(RecolorCursor,rp);
		((xRecolorCursorReq *)rp)->cursor = Get_Default_Cursor(client);
		((xRecolorCursorReq *)rp)->foreRed = 0;
		((xRecolorCursorReq *)rp)->foreGreen = 0;
		((xRecolorCursorReq *)rp)->foreBlue = 0;
		((xRecolorCursorReq *)rp)->backRed = 1;
		((xRecolorCursorReq *)rp)->backGreen = 1;
		((xRecolorCursorReq *)rp)->backBlue = 1;
		break;
	case X_QueryBestSize:
		GetReq(QueryBestSize,rp);
		((xQueryBestSizeReq *)rp)->class = TileShape;
		((xQueryBestSizeReq *)rp)->drawable = Get_Default_Window(client);
		((xQueryBestSizeReq *)rp)->width = 20;
		((xQueryBestSizeReq *)rp)->height = 20;
		break;
	case X_QueryExtension:
		GetReqExtra(QueryExtension,16,rp);
		((xQueryExtensionReq *)rp)->nbytes = 16;
		Copy_Padded_String8(&valuePtr, "Not-an-Extension");
		break;
	case X_ListExtensions:
		GetEmptyReq(ListExtensions,rp);
		break;
	case X_ChangeKeyboardMapping:
		GetReq(ChangeKeyboardMapping,rp);
		((xChangeKeyboardMappingReq *)rp)->keyCodes = 0;
		((xChangeKeyboardMappingReq *)rp)->firstKeyCode = 0;
		((xChangeKeyboardMappingReq *)rp)->keySymsPerKeyCode = 0;
		break;
	case X_GetKeyboardMapping:
		GetReq(GetKeyboardMapping,rp);
		((xGetKeyboardMappingReq *)rp)->firstKeyCode = Get_Display(client)->min_keycode;
		((xGetKeyboardMappingReq *)rp)->count = Get_Display(client)->max_keycode - Get_Display(client)->min_keycode + 1;
		break;
	case X_ChangeKeyboardControl:
		GetReq(ChangeKeyboardControl,rp);
		((xChangeKeyboardControlReq *)rp)->mask = 0;
		break;
	case X_GetKeyboardControl:
		GetEmptyReq(GetKeyboardControl,rp);
		break;
	case X_Bell:
		GetReq(Bell,rp);
		((xBellReq *)rp)->percent = 100;
		break;
	case X_ChangePointerControl:
		GetReq(ChangePointerControl,rp);
		((xChangePointerControlReq *)rp)->accelNum = 1;
		((xChangePointerControlReq *)rp)->accelDenum = 1;
		((xChangePointerControlReq *)rp)->threshold = 2000;
		((xChangePointerControlReq *)rp)->doAccel = 1;
		((xChangePointerControlReq *)rp)->doThresh = 1;
		break;
	case X_GetPointerControl:
		GetEmptyReq(GetPointerControl,rp);
		break;
	case X_SetScreenSaver:
		GetReq(SetScreenSaver,rp);
		((xSetScreenSaverReq *)rp)->timeout = 300;
		((xSetScreenSaverReq *)rp)->interval = 300;
		((xSetScreenSaverReq *)rp)->preferBlank = DefaultBlanking;
		((xSetScreenSaverReq *)rp)->allowExpose = DefaultExposures;
		break;
	case X_GetScreenSaver:
		GetEmptyReq(GetScreenSaver,rp);
		break;
	case X_ChangeHosts:
		GetReq(ChangeHosts,rp);
		((xChangeHostsReq *)rp)->mode = HostInsert;
		((xChangeHostsReq *)rp)->hostFamily = FamilyInternet;
		((xChangeHostsReq *)rp)->hostLength = 0;
		break;
	case X_ListHosts:
		GetEmptyReq(ListHosts,rp);
		break;
	case X_SetAccessControl:
		GetReq(SetAccessControl,rp);
		((xSetAccessControlReq *)rp)->mode = DisableAccess;
		break;
	case X_SetCloseDownMode:
		GetReq(SetCloseDownMode,rp);
		((xSetCloseDownModeReq *)rp)->mode = DestroyAll;
		break;
	case X_KillClient:
		GetResReq(KillClient,Gen_Good_id(client),rp);
		((xResourceReq *)rp)->id = 0;
		break;
	case X_RotateProperties:
		GetReqExtra(RotateProperties,8,rp);
		((xRotatePropertiesReq *)rp)->window = Get_Default_Window(client);
		((xRotatePropertiesReq *)rp)->nAtoms = 2;
		((xRotatePropertiesReq *)rp)->nPositions = 1;
		Set_Value4(&valuePtr, Test_Atom1);
		Set_Value4(&valuePtr, Test_Atom2);
		break;
	case X_ForceScreenSaver:
		GetReq(ForceScreenSaver,rp);
		((xForceScreenSaverReq *)rp)->mode = ScreenSaverActive;
		break;
	case X_SetPointerMapping:
		GetReq(SetPointerMapping,rp);
		((xSetPointerMappingReq *)rp)->nElts = 0;
		break;
	case X_GetPointerMapping:
		GetEmptyReq(GetPointerMapping,rp);
		break;
	case X_SetModifierMapping:
		GetReq(SetModifierMapping,rp);
		((xSetModifierMappingReq *)rp)->numKeyPerModifier = 0;
		break;
	case X_GetModifierMapping:
		GetEmptyReq(GetModifierMapping,rp);
		break;
	case X_NoOperation:
		GetEmptyReq(NoOperation,rp);
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
		Log_Msg ("INTERNAL ERROR: Make_Req - bad test type %d\n", Get_Test_Type(client));
		Delete ();
		/*NOTREACHED*/
		break;
	}
	return(rp);
}


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
 * $XConsortium: SendReq.c,v 1.12 94/04/17 21:01:23 rws Exp $
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

#include "XstlibInt.h"
#include "DataMove.h"

extern int Send_Evt();
void _Send_Req();

void
Send_Req(client,rp)
xReq *rp;
int client;
{
    _Send_Req(client,rp,0);		/* not polling thru this entry point */
}

void
_Send_Req(client,rp,pollreq)
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
	    Xst_clients[client].cl_reqtype = rp->reqType;	/* stash for Expect */
	    Xst_clients[client].cl_minor = 0;		/* stash for Expect */
	}

	dpy->request++;

	switch (rp->reqType) {
	case X_CreateWindow:
		send1(client,(long) ((xCreateWindowReq *)rp)->reqType);
		send1(client,(long) ((xCreateWindowReq *)rp)->depth);
		send2(client,(short) ((xCreateWindowReq *)rp)->length);
		send4(client,(long) ((xCreateWindowReq *)rp)->wid);
		send4(client,(long) ((xCreateWindowReq *)rp)->parent);
		send2(client,(short) ((xCreateWindowReq *)rp)->x);
		send2(client,(short) ((xCreateWindowReq *)rp)->y);
		send2(client,(short) ((xCreateWindowReq *)rp)->width);
		send2(client,(short) ((xCreateWindowReq *)rp)->height);
		send2(client,(short) ((xCreateWindowReq *)rp)->borderWidth);
		send2(client,(short) ((xCreateWindowReq *)rp)->class);
		send4(client,(long) ((xCreateWindowReq *)rp)->visual);
		send4(client,(long) ((xCreateWindowReq *)rp)->mask);
		Send_Value_List(client, rp, sizeof (xCreateWindowReq), 32);
		break;
	case X_ChangeWindowAttributes:
		send1(client,(long) ((xChangeWindowAttributesReq *)rp)->reqType);
		sendpad(client,1);
		send2(client,(short) ((xChangeWindowAttributesReq *)rp)->length);
		send4(client,(long) ((xChangeWindowAttributesReq *)rp)->window);
		send4(client,(long) ((xChangeWindowAttributesReq *)rp)->valueMask);
		Send_Value_List(client, rp, sizeof (xChangeWindowAttributesReq), 32);
		break;
	case X_GetWindowAttributes:
		send1(client,(long) ((xResourceReq *)rp)->reqType);
		sendpad(client,1);
		send2(client,(short) ((xResourceReq *)rp)->length);
		send4(client,(long) ((xResourceReq *)rp)->id);
		break;
	case X_DestroyWindow:
		send1(client,(long) ((xResourceReq *)rp)->reqType);
		sendpad(client,1);
		send2(client,(short) ((xResourceReq *)rp)->length);
		send4(client,(long) ((xResourceReq *)rp)->id);
		break;
	case X_DestroySubwindows:
		send1(client,(long) ((xResourceReq *)rp)->reqType);
		sendpad(client,1);
		send2(client,(short) ((xResourceReq *)rp)->length);
		send4(client,(long) ((xResourceReq *)rp)->id);
		break;
	case X_ChangeSaveSet:
		send1(client,(long) ((xChangeSaveSetReq *)rp)->reqType);
		send1(client,(long) ((xChangeSaveSetReq *)rp)->mode);
		send2(client,(short) ((xChangeSaveSetReq *)rp)->length);
		send4(client,(long) ((xChangeSaveSetReq *)rp)->window);
		break;
	case X_ReparentWindow:
		send1(client,(long) ((xReparentWindowReq *)rp)->reqType);
		sendpad(client,1);
		send2(client,(short) ((xReparentWindowReq *)rp)->length);
		send4(client,(long) ((xReparentWindowReq *)rp)->window);
		send4(client,(long) ((xReparentWindowReq *)rp)->parent);
		send2(client,(short) ((xReparentWindowReq *)rp)->x);
		send2(client,(short) ((xReparentWindowReq *)rp)->y);
		break;
	case X_MapWindow:
		send1(client,(long) ((xResourceReq *)rp)->reqType);
		sendpad(client,1);
		send2(client,(short) ((xResourceReq *)rp)->length);
		send4(client,(long) ((xResourceReq *)rp)->id);
		break;
	case X_MapSubwindows:
		send1(client,(long) ((xResourceReq *)rp)->reqType);
		sendpad(client,1);
		send2(client,(short) ((xResourceReq *)rp)->length);
		send4(client,(long) ((xResourceReq *)rp)->id);
		break;
	case X_UnmapWindow:
		send1(client,(long) ((xResourceReq *)rp)->reqType);
		sendpad(client,1);
		send2(client,(short) ((xResourceReq *)rp)->length);
		send4(client,(long) ((xResourceReq *)rp)->id);
		break;
	case X_UnmapSubwindows:
		send1(client,(long) ((xResourceReq *)rp)->reqType);
		sendpad(client,1);
		send2(client,(short) ((xResourceReq *)rp)->length);
		send4(client,(long) ((xResourceReq *)rp)->id);
		break;
	case X_ConfigureWindow:
		send1(client,(long) ((xConfigureWindowReq *)rp)->reqType);
		sendpad(client,1);
		send2(client,(short) ((xConfigureWindowReq *)rp)->length);
		send4(client,(long) ((xConfigureWindowReq *)rp)->window);
		send2(client,(short) ((xConfigureWindowReq *)rp)->mask);
		sendpad(client,2);
		Send_Value_List(client, rp, sizeof (xConfigureWindowReq), 32);
		break;
	case X_CirculateWindow:
		send1(client,(long) ((xCirculateWindowReq *)rp)->reqType);
		send1(client,(long) ((xCirculateWindowReq *)rp)->direction);
		send2(client,(short) ((xCirculateWindowReq *)rp)->length);
		send4(client,(long) ((xCirculateWindowReq *)rp)->window);
		break;
	case X_GetGeometry:
		send1(client,(long) ((xResourceReq *)rp)->reqType);
		sendpad(client,1);
		send2(client,(short) ((xResourceReq *)rp)->length);
		send4(client,(long) ((xResourceReq *)rp)->id);
		break;
	case X_QueryTree:
		send1(client,(long) ((xResourceReq *)rp)->reqType);
		sendpad(client,1);
		send2(client,(short) ((xResourceReq *)rp)->length);
		send4(client,(long) ((xResourceReq *)rp)->id);
		break;
	case X_InternAtom:
		send1(client,(long) ((xInternAtomReq *)rp)->reqType);
		send1(client,(long) ((xInternAtomReq *)rp)->onlyIfExists);
		send2(client,(short) ((xInternAtomReq *)rp)->length);
		send2(client,(short) ((xInternAtomReq *)rp)->nbytes);
		sendpad(client,2);
		Send_Value_List(client, rp, sizeof (xInternAtomReq),8);
		break;
	case X_GetAtomName:
		send1(client,(long) ((xResourceReq *)rp)->reqType);
		sendpad(client,1);
		send2(client,(short) ((xResourceReq *)rp)->length);
		send4(client,(long) ((xResourceReq *)rp)->id);
		break;
	case X_ChangeProperty:
		send1(client,(long) ((xChangePropertyReq *)rp)->reqType);
		send1(client,(long) ((xChangePropertyReq *)rp)->mode);
		send2(client,(short) ((xChangePropertyReq *)rp)->length);
		send4(client,(long) ((xChangePropertyReq *)rp)->window);
		send4(client,(long) ((xChangePropertyReq *)rp)->property);
		send4(client,(long) ((xChangePropertyReq *)rp)->type);
		send1(client,(long) ((xChangePropertyReq *)rp)->format);
		sendpad(client,3);
		send4(client,(long) ((xChangePropertyReq *)rp)->nUnits);
		Send_Value_List(client, rp, sizeof (xChangePropertyReq),
		    (int)((xChangePropertyReq *)rp)->format);
		break;
	case X_DeleteProperty:
		send1(client,(long) ((xDeletePropertyReq *)rp)->reqType);
		sendpad(client,1);
		send2(client,(short) ((xDeletePropertyReq *)rp)->length);
		send4(client,(long) ((xDeletePropertyReq *)rp)->window);
		send4(client,(long) ((xDeletePropertyReq *)rp)->property);
		break;
	case X_GetProperty:
		send1(client,(long) ((xGetPropertyReq *)rp)->reqType);
		send1(client,(long) ((xGetPropertyReq *)rp)->delete);
		send2(client,(short) ((xGetPropertyReq *)rp)->length);
		send4(client,(long) ((xGetPropertyReq *)rp)->window);
		send4(client,(long) ((xGetPropertyReq *)rp)->property);
		send4(client,(long) ((xGetPropertyReq *)rp)->type);
		send4(client,(long) ((xGetPropertyReq *)rp)->longOffset);
		send4(client,(long) ((xGetPropertyReq *)rp)->longLength);
		break;
	case X_ListProperties:
		send1(client,(long) ((xResourceReq *)rp)->reqType);
		sendpad(client,1);
		send2(client,(short) ((xResourceReq *)rp)->length);
		send4(client,(long) ((xResourceReq *)rp)->id);
		break;
	case X_SetSelectionOwner:
		send1(client,(long) ((xSetSelectionOwnerReq *)rp)->reqType);
		sendpad(client,1);
		send2(client,(short) ((xSetSelectionOwnerReq *)rp)->length);
		send4(client,(long) ((xSetSelectionOwnerReq *)rp)->window);
		send4(client,(long) ((xSetSelectionOwnerReq *)rp)->selection);
		send4(client,(long) ((xSetSelectionOwnerReq *)rp)->time);
		break;
	case X_GetSelectionOwner:
		send1(client,(long) ((xResourceReq *)rp)->reqType);
		sendpad(client,1);
		send2(client,(short) ((xResourceReq *)rp)->length);
		send4(client,(long) ((xResourceReq *)rp)->id);
		break;
	case X_ConvertSelection:
		send1(client,(long) ((xConvertSelectionReq *)rp)->reqType);
		sendpad(client,1);
		send2(client,(short) ((xConvertSelectionReq *)rp)->length);
		send4(client,(long) ((xConvertSelectionReq *)rp)->requestor);
		send4(client,(long) ((xConvertSelectionReq *)rp)->selection);
		send4(client,(long) ((xConvertSelectionReq *)rp)->target);
		send4(client,(long) ((xConvertSelectionReq *)rp)->property);
		send4(client,(long) ((xConvertSelectionReq *)rp)->time);
		break;
	case X_SendEvent: {
	        xEvent *event_ptr;
		send1(client,(long) ((xSendEventReq *)rp)->reqType);
		send1(client,(long) ((xSendEventReq *)rp)->propagate);
		send2(client,(short) ((xSendEventReq *)rp)->length);
		send4(client,(long) ((xSendEventReq *)rp)->destination);
		send4(client,(long) ((xSendEventReq *)rp)->eventMask);
		event_ptr = &(((xSendEventReq *)rp)->event);
		Send_Evt(client, event_ptr, event_ptr->u.u.type);
		break;
	    }
	case X_GrabPointer:
		send1(client,(long) ((xGrabPointerReq *)rp)->reqType);
		send1(client,(long) ((xGrabPointerReq *)rp)->ownerEvents);
		send2(client,(short) ((xGrabPointerReq *)rp)->length);
		send4(client,(long) ((xGrabPointerReq *)rp)->grabWindow);
		send2(client,(short) ((xGrabPointerReq *)rp)->eventMask);
		send1(client,(long) ((xGrabPointerReq *)rp)->pointerMode);
		send1(client,(long) ((xGrabPointerReq *)rp)->keyboardMode);
		send4(client,(long) ((xGrabPointerReq *)rp)->confineTo);
		send4(client,(long) ((xGrabPointerReq *)rp)->cursor);
		send4(client,(long) ((xGrabPointerReq *)rp)->time);
		break;
	case X_UngrabPointer:
		send1(client,(long) ((xResourceReq *)rp)->reqType);
		sendpad(client,1);
		send2(client,(short) ((xResourceReq *)rp)->length);
		send4(client,(long) ((xResourceReq *)rp)->id);
		break;
	case X_GrabButton:
		send1(client,(long) ((xGrabButtonReq *)rp)->reqType);
		send1(client,(long) ((xGrabButtonReq *)rp)->ownerEvents);
		send2(client,(short) ((xGrabButtonReq *)rp)->length);
		send4(client,(long) ((xGrabButtonReq *)rp)->grabWindow);
		send2(client,(short) ((xGrabButtonReq *)rp)->eventMask);
		send1(client,(long) ((xGrabButtonReq *)rp)->pointerMode);
		send1(client,(long) ((xGrabButtonReq *)rp)->keyboardMode);
		send4(client,(long) ((xGrabButtonReq *)rp)->confineTo);
		send4(client,(long) ((xGrabButtonReq *)rp)->cursor);
		send1(client,(long) ((xGrabButtonReq *)rp)->button);
		sendpad(client,1);
		send2(client,(short) ((xGrabButtonReq *)rp)->modifiers);
		break;
	case X_UngrabButton:
		send1(client,(long) ((xUngrabButtonReq *)rp)->reqType);
		send1(client,(long) ((xUngrabButtonReq *)rp)->button);
		send2(client,(short) ((xUngrabButtonReq *)rp)->length);
		send4(client,(long) ((xUngrabButtonReq *)rp)->grabWindow);
		send2(client,(short) ((xUngrabButtonReq *)rp)->modifiers);
		sendpad(client,2);
		break;
	case X_ChangeActivePointerGrab:
		send1(client,(long) ((xChangeActivePointerGrabReq *)rp)->reqType);
		sendpad(client,1);
		send2(client,(short) ((xChangeActivePointerGrabReq *)rp)->length);
		send4(client,(long) ((xChangeActivePointerGrabReq *)rp)->cursor);
		send4(client,(long) ((xChangeActivePointerGrabReq *)rp)->time);
		send2(client,(short) ((xChangeActivePointerGrabReq *)rp)->eventMask);
		sendpad(client,2);
		break;
	case X_GrabKeyboard:
		send1(client,(long) ((xGrabKeyboardReq *)rp)->reqType);
		send1(client,(long) ((xGrabKeyboardReq *)rp)->ownerEvents);
		send2(client,(short) ((xGrabKeyboardReq *)rp)->length);
		send4(client,(long) ((xGrabKeyboardReq *)rp)->grabWindow);
		send4(client,(long) ((xGrabKeyboardReq *)rp)->time);
		send1(client,(long) ((xGrabKeyboardReq *)rp)->pointerMode);
		send1(client,(long) ((xGrabKeyboardReq *)rp)->keyboardMode);
		sendpad(client,2);
		break;
	case X_UngrabKeyboard:
		send1(client,(long) ((xResourceReq *)rp)->reqType);
		sendpad(client,1);
		send2(client,(short) ((xResourceReq *)rp)->length);
		send4(client,(long) ((xResourceReq *)rp)->id);
		break;
	case X_GrabKey:
		send1(client,(long) ((xGrabKeyReq *)rp)->reqType);
		send1(client,(long) ((xGrabKeyReq *)rp)->ownerEvents);
		send2(client,(short) ((xGrabKeyReq *)rp)->length);
		send4(client,(long) ((xGrabKeyReq *)rp)->grabWindow);
		send2(client,(short) ((xGrabKeyReq *)rp)->modifiers);
		send1(client,(long) ((xGrabKeyReq *)rp)->key);
		send1(client,(long) ((xGrabKeyReq *)rp)->pointerMode);
		send1(client,(long) ((xGrabKeyReq *)rp)->keyboardMode);
		sendpad(client,3);
		break;
	case X_UngrabKey:
		send1(client,(long) ((xUngrabKeyReq *)rp)->reqType);
		send1(client,(long) ((xUngrabKeyReq *)rp)->key);
		send2(client,(short) ((xUngrabKeyReq *)rp)->length);
		send4(client,(long) ((xUngrabKeyReq *)rp)->grabWindow);
		send2(client,(short) ((xUngrabKeyReq *)rp)->modifiers);
		sendpad(client,2);
		break;
	case X_AllowEvents:
		send1(client,(long) ((xAllowEventsReq *)rp)->reqType);
		send1(client,(long) ((xAllowEventsReq *)rp)->mode);
		send2(client,(short) ((xAllowEventsReq *)rp)->length);
		send4(client,(long) ((xAllowEventsReq *)rp)->time);
		break;
	case X_GrabServer:
		send1(client,(long) ((xResourceReq *)rp)->reqType);
		sendpad(client,1);
		send2(client,(short) ((xResourceReq *)rp)->length);
		break;
	case X_UngrabServer:
		send1(client,(long) ((xResourceReq *)rp)->reqType);
		sendpad(client,1);
		send2(client,(short) ((xResourceReq *)rp)->length);
		break;
	case X_QueryPointer:
		send1(client,(long) ((xResourceReq *)rp)->reqType);
		sendpad(client,1);
		send2(client,(short) ((xResourceReq *)rp)->length);
		send4(client,(long) ((xResourceReq *)rp)->id);
		break;
	case X_GetMotionEvents:
		send1(client,(long) ((xGetMotionEventsReq *)rp)->reqType);
		sendpad(client,1);
		send2(client,(short) ((xGetMotionEventsReq *)rp)->length);
		send4(client,(long) ((xGetMotionEventsReq *)rp)->window);
		send4(client,(long) ((xGetMotionEventsReq *)rp)->start);
		send4(client,(long) ((xGetMotionEventsReq *)rp)->stop);
		break;
	case X_TranslateCoords:
		send1(client,(long) ((xTranslateCoordsReq *)rp)->reqType);
		sendpad(client,1);
		send2(client,(short) ((xTranslateCoordsReq *)rp)->length);
		send4(client,(long) ((xTranslateCoordsReq *)rp)->srcWid);
		send4(client,(long) ((xTranslateCoordsReq *)rp)->dstWid);
		send2(client,(short) ((xTranslateCoordsReq *)rp)->srcX);
		send2(client,(short) ((xTranslateCoordsReq *)rp)->srcY);
		break;
	case X_WarpPointer:
		send1(client,(long) ((xWarpPointerReq *)rp)->reqType);
		sendpad(client,1);
		send2(client,(short) ((xWarpPointerReq *)rp)->length);
		send4(client,(long) ((xWarpPointerReq *)rp)->srcWid);
		send4(client,(long) ((xWarpPointerReq *)rp)->dstWid);
		send2(client,(short) ((xWarpPointerReq *)rp)->srcX);
		send2(client,(short) ((xWarpPointerReq *)rp)->srcY);
		send2(client,(short) ((xWarpPointerReq *)rp)->srcWidth);
		send2(client,(short) ((xWarpPointerReq *)rp)->srcHeight);
		send2(client,(short) ((xWarpPointerReq *)rp)->dstX);
		send2(client,(short) ((xWarpPointerReq *)rp)->dstY);
		break;
	case X_SetInputFocus:
		send1(client,(long) ((xSetInputFocusReq *)rp)->reqType);
		send1(client,(long) ((xSetInputFocusReq *)rp)->revertTo);
		send2(client,(short) ((xSetInputFocusReq *)rp)->length);
		send4(client,(long) ((xSetInputFocusReq *)rp)->focus);
		send4(client,(long) ((xSetInputFocusReq *)rp)->time);
		break;
	case X_GetInputFocus:
		send1(client,(long) ((xResourceReq *)rp)->reqType);
		sendpad(client,1);
		send2(client,(short) ((xResourceReq *)rp)->length);
		break;
	case X_QueryKeymap:
		send1(client,(long) ((xResourceReq *)rp)->reqType);
		sendpad(client,1);
		send2(client,(short) ((xResourceReq *)rp)->length);
		break;
	case X_OpenFont:
		send1(client,(long) ((xOpenFontReq *)rp)->reqType);
		sendpad(client,1);
		send2(client,(short) ((xOpenFontReq *)rp)->length);
		send4(client,(long) ((xOpenFontReq *)rp)->fid);
		send2(client,(short) ((xOpenFontReq *)rp)->nbytes);
		sendpad(client,2);
		Send_Value_List(client, rp, sizeof (xOpenFontReq),8);
		sendpad(client,0);
		break;
	case X_CloseFont:
		send1(client,(long) ((xResourceReq *)rp)->reqType);
		sendpad(client,1);
		send2(client,(short) ((xResourceReq *)rp)->length);
		send4(client,(long) ((xResourceReq *)rp)->id);
		break;
	case X_QueryFont:
		send1(client,(long) ((xResourceReq *)rp)->reqType);
		sendpad(client,1);
		send2(client,(short) ((xResourceReq *)rp)->length);
		send4(client,(long) ((xResourceReq *)rp)->id);
		break;
	case X_QueryTextExtents:
		send1(client,(long) ((xQueryTextExtentsReq *)rp)->reqType);
		send1(client,(long) ((xQueryTextExtentsReq *)rp)->oddLength);
		send2(client,(short) ((xQueryTextExtentsReq *)rp)->length);
		send4(client,(long) ((xQueryTextExtentsReq *)rp)->fid);
		Send_String16(client, rp, sizeof (xQueryTextExtentsReq));
		break;
	case X_ListFonts:
		send1(client,(long) ((xListFontsReq *)rp)->reqType);
		sendpad(client,1);
		send2(client,(short) ((xListFontsReq *)rp)->length);
		send2(client,(short) ((xListFontsReq *)rp)->maxNames);
		send2(client,(short) ((xListFontsReq *)rp)->nbytes);
		Send_Value_List(client, rp, sizeof (xListFontsReq),8);
		break;
	case X_ListFontsWithInfo:
		send1(client,(long) ((xListFontsWithInfoReq *)rp)->reqType);
		sendpad(client,1);
		send2(client,(short) ((xListFontsWithInfoReq *)rp)->length);
		send2(client,(short) ((xListFontsWithInfoReq *)rp)->maxNames);
		send2(client,(short) ((xListFontsWithInfoReq *)rp)->nbytes);
		Send_Value_List(client, rp, sizeof (xListFontsWithInfoReq),8);
		break;
	case X_SetFontPath:
		send1(client,(long) ((xSetFontPathReq *)rp)->reqType);
		sendpad(client,1);
		send2(client,(short) ((xSetFontPathReq *)rp)->length);
		send2(client,(short) ((xSetFontPathReq *)rp)->nFonts);
		sendpad(client,2);
		Send_Value_List(client, rp, sizeof (xSetFontPathReq),8);
		break;
	case X_GetFontPath:
		send1(client,(long) rp->reqType);
		sendpad(client,1);
		send2(client,(short) rp->length);
		break;
	case X_CreatePixmap:
		send1(client,(long) ((xCreatePixmapReq *)rp)->reqType);
		send1(client,(long) ((xCreatePixmapReq *)rp)->depth);
		send2(client,(short) ((xCreatePixmapReq *)rp)->length);
		send4(client,(long) ((xCreatePixmapReq *)rp)->pid);
		send4(client,(long) ((xCreatePixmapReq *)rp)->drawable);
		send2(client,(short) ((xCreatePixmapReq *)rp)->width);
		send2(client,(short) ((xCreatePixmapReq *)rp)->height);
		break;
	case X_FreePixmap:
		send1(client,(long) ((xResourceReq *)rp)->reqType);
		sendpad(client,1);
		send2(client,(short) ((xResourceReq *)rp)->length);
		send4(client,(long) ((xResourceReq *)rp)->id);
		break;
	case X_CreateGC:
		send1(client,(long) ((xCreateGCReq *)rp)->reqType);
		sendpad(client,1);
		send2(client,(short) ((xCreateGCReq *)rp)->length);
		send4(client,(long) ((xCreateGCReq *)rp)->gc);
		send4(client,(long) ((xCreateGCReq *)rp)->drawable);
		send4(client,(long) ((xCreateGCReq *)rp)->mask);
		Send_Value_List(client, rp, sizeof (xCreateGCReq), 32);
		break;
	case X_ChangeGC:
		send1(client,(long) ((xChangeGCReq *)rp)->reqType);
		sendpad(client,1);
		send2(client,(short) ((xChangeGCReq *)rp)->length);
		send4(client,(long) ((xChangeGCReq *)rp)->gc);
		send4(client,(long) ((xChangeGCReq *)rp)->mask);
		Send_Value_List(client, rp, sizeof (xChangeGCReq),32);
		break;
	case X_CopyGC:
		send1(client,(long) ((xCopyGCReq *)rp)->reqType);
		sendpad(client,1);
		send2(client,(short) ((xCopyGCReq *)rp)->length);
		send4(client,(long) ((xCopyGCReq *)rp)->srcGC);
		send4(client,(long) ((xCopyGCReq *)rp)->dstGC);
		send4(client,(long) ((xCopyGCReq *)rp)->mask);
		break;
	case X_SetDashes:
		send1(client,(long) ((xSetDashesReq *)rp)->reqType);
		sendpad(client,1);
		send2(client,(short) ((xSetDashesReq *)rp)->length);
		send4(client,(long) ((xSetDashesReq *)rp)->gc);
		send2(client,(short) ((xSetDashesReq *)rp)->dashOffset);
		send2(client,(short) ((xSetDashesReq *)rp)->nDashes);
		Send_Value_List(client, rp, sizeof (xSetDashesReq),8);
		break;
	case X_SetClipRectangles:
		send1(client,(long) ((xSetClipRectanglesReq *)rp)->reqType);
		send1(client,(long) ((xSetClipRectanglesReq *)rp)->ordering);
		send2(client,(short) ((xSetClipRectanglesReq *)rp)->length);
		send4(client,(long) ((xSetClipRectanglesReq *)rp)->gc);
		send2(client,(short) ((xSetClipRectanglesReq *)rp)->xOrigin);
		send2(client,(short) ((xSetClipRectanglesReq *)rp)->yOrigin);
		Send_Value_List(client, rp, sizeof (xSetClipRectanglesReq),16);
		break;
	case X_FreeGC:
		send1(client,(long) ((xResourceReq *)rp)->reqType);
		sendpad(client,1);
		send2(client,(short) ((xResourceReq *)rp)->length);
		send4(client,(long) ((xResourceReq *)rp)->id);
		break;
	case X_ClearArea:
		send1(client,(long) ((xClearAreaReq *)rp)->reqType);
		send1(client,(long) ((xClearAreaReq *)rp)->exposures);
		send2(client,(short) ((xClearAreaReq *)rp)->length);
		send4(client,(long) ((xClearAreaReq *)rp)->window);
		send2(client,(short) ((xClearAreaReq *)rp)->x);
		send2(client,(short) ((xClearAreaReq *)rp)->y);
		send2(client,(short) ((xClearAreaReq *)rp)->width);
		send2(client,(short) ((xClearAreaReq *)rp)->height);
		break;
	case X_CopyArea:
		send1(client,(long) ((xCopyAreaReq *)rp)->reqType);
		sendpad(client,1);
		send2(client,(short) ((xCopyAreaReq *)rp)->length);
		send4(client,(long) ((xCopyAreaReq *)rp)->srcDrawable);
		send4(client,(long) ((xCopyAreaReq *)rp)->dstDrawable);
		send4(client,(long) ((xCopyAreaReq *)rp)->gc);
		send2(client,(short) ((xCopyAreaReq *)rp)->srcX);
		send2(client,(short) ((xCopyAreaReq *)rp)->srcY);
		send2(client,(short) ((xCopyAreaReq *)rp)->dstX);
		send2(client,(short) ((xCopyAreaReq *)rp)->dstY);
		send2(client,(short) ((xCopyAreaReq *)rp)->width);
		send2(client,(short) ((xCopyAreaReq *)rp)->height);
		break;
	case X_CopyPlane:
		send1(client,(long) ((xCopyPlaneReq *)rp)->reqType);
		sendpad(client,1);
		send2(client,(short) ((xCopyPlaneReq *)rp)->length);
		send4(client,(long) ((xCopyPlaneReq *)rp)->srcDrawable);
		send4(client,(long) ((xCopyPlaneReq *)rp)->dstDrawable);
		send4(client,(long) ((xCopyPlaneReq *)rp)->gc);
		send2(client,(short) ((xCopyPlaneReq *)rp)->srcX);
		send2(client,(short) ((xCopyPlaneReq *)rp)->srcY);
		send2(client,(short) ((xCopyPlaneReq *)rp)->dstX);
		send2(client,(short) ((xCopyPlaneReq *)rp)->dstY);
		send2(client,(short) ((xCopyPlaneReq *)rp)->width);
		send2(client,(short) ((xCopyPlaneReq *)rp)->height);
		send4(client,(long) ((xCopyPlaneReq *)rp)->bitPlane);
		break;
	case X_PolyPoint:
		send1(client,(long) ((xPolyPointReq *)rp)->reqType);
		send1(client,(long) ((xPolyPointReq *)rp)->coordMode);
		send2(client,(short) ((xPolyPointReq *)rp)->length);
		send4(client,(long) ((xPolyPointReq *)rp)->drawable);
		send4(client,(long) ((xPolyPointReq *)rp)->gc);
		Send_Value_List(client, rp, sizeof (xPolyPointReq),16);
		break;
	case X_PolyLine:
		send1(client,(long) ((xPolyLineReq *)rp)->reqType);
		send1(client,(long) ((xPolyLineReq *)rp)->coordMode);
		send2(client,(short) ((xPolyLineReq *)rp)->length);
		send4(client,(long) ((xPolyLineReq *)rp)->drawable);
		send4(client,(long) ((xPolyLineReq *)rp)->gc);
		Send_Value_List(client, rp, sizeof (xPolyLineReq),16);
		break;
	case X_PolySegment:
		send1(client,(long) ((xPolySegmentReq *)rp)->reqType);
		sendpad(client,1);
		send2(client,(short) ((xPolySegmentReq *)rp)->length);
		send4(client,(long) ((xPolySegmentReq *)rp)->drawable);
		send4(client,(long) ((xPolySegmentReq *)rp)->gc);
		Send_Value_List(client, rp, sizeof (xPolySegmentReq), 16);
		break;
	case X_PolyRectangle:
		send1(client,(long) ((xPolyRectangleReq *)rp)->reqType);
		sendpad(client,1);
		send2(client,(short) ((xPolyRectangleReq *)rp)->length);
		send4(client,(long) ((xPolyRectangleReq *)rp)->drawable);
		send4(client,(long) ((xPolyRectangleReq *)rp)->gc);
		Send_Value_List(client, rp, sizeof (xPolyRectangleReq),16);
		break;
	case X_PolyArc:
		send1(client,(long) ((xPolyArcReq *)rp)->reqType);
		sendpad(client,1);
		send2(client,(short) ((xPolyArcReq *)rp)->length);
		send4(client,(long) ((xPolyArcReq *)rp)->drawable);
		send4(client,(long) ((xPolyArcReq *)rp)->gc);
		Send_Value_List(client, rp, sizeof (xPolyArcReq),16);
		break;
	case X_FillPoly:
		send1(client,(long) ((xFillPolyReq *)rp)->reqType);
		sendpad(client,1);
		send2(client,(short) ((xFillPolyReq *)rp)->length);
		send4(client,(long) ((xFillPolyReq *)rp)->drawable);
		send4(client,(long) ((xFillPolyReq *)rp)->gc);
		send1(client,(long) ((xFillPolyReq *)rp)->shape);
		send1(client,(long) ((xFillPolyReq *)rp)->coordMode);
		sendpad(client,2);
		Send_Value_List(client, rp, sizeof (xFillPolyReq),16);
		break;
	case X_PolyFillRectangle:
		send1(client,(long) ((xPolyFillRectangleReq *)rp)->reqType);
		sendpad(client,1);
		send2(client,(short) ((xPolyFillRectangleReq *)rp)->length);
		send4(client,(long) ((xPolyFillRectangleReq *)rp)->drawable);
		send4(client,(long) ((xPolyFillRectangleReq *)rp)->gc);
		Send_Value_List(client, rp, sizeof (xPolyFillRectangleReq), 16);
		break;
	case X_PolyFillArc:
		send1(client,(long) ((xPolyFillArcReq *)rp)->reqType);
		sendpad(client,1);
		send2(client,(short) ((xPolyFillArcReq *)rp)->length);
		send4(client,(long) ((xPolyFillArcReq *)rp)->drawable);
		send4(client,(long) ((xPolyFillArcReq *)rp)->gc);
		Send_Value_List(client, rp, sizeof (xPolyFillArcReq), 16);
		break;
	case X_PutImage:
		{
/*
 *	Images are stored in the test programs in client byte order and
 *	unpadded.  This allows images to be independent of the server.
 *	However the server will expect images in server byte order and 
 *	padded.  This routine sends an altered xPutImageReq which the server
 *	will like.  Note that we're assuming client-normal form means that
 *	rows are padded to a byte boundary; otherwise the translation is
 *	more complex.  Similarly, left-pad must be zero.
 */

		int row, col = 1;
		unsigned char my_sex = *((unsigned char *) &col) ^ 1;
		unsigned char server_sex =
			(Xst_clients[client].cl_dpy) -> byte_order;
		long flip = my_sex ^ server_sex;  /* assume MSBFirst == 1 */
		int server_pad = (Xst_clients[client].cl_dpy) -> bitmap_pad;
		int server_unit = (Xst_clients[client].cl_dpy) -> bitmap_unit;
		int server_bitorder =
			(Xst_clients[client].cl_dpy) -> bitmap_bit_order;
		int src_width /*in bytes*/ =
			(int)(((xPutImageReq *)rp)->width + 7) >> 3;
		int dst_width /*in bytes*/ = src_width +
			((src_width % (server_pad>>3)) == 0 ? 0 :
			 (server_pad>>3) - src_width % (server_pad>>3));
		char *src = (char *)rp + sizeof(xPutImageReq);
		char **dst = (&(Get_Display(client)->bufptr));
		char *drop;

/*****
		if (server_bitorder != MSBFirst) {
			Log_Err("LSBFirst bit ordering not supported in Send_Req()\n");
			Abort();
		}
*****/
		if (((xPutImageReq *)rp)->leftPad != 0)  {
			Log_Err("leftPad != 0; not supported in Send_Req()\n");
			Abort();
		}

		send1(client,(long) ((xPutImageReq *)rp)->reqType);
		send1(client,(long) ((xPutImageReq *)rp)->format);
		send2(client,(short) (bytes_out >> 2));
		send4(client,(long) ((xPutImageReq *)rp)->drawable);
		send4(client,(long) ((xPutImageReq *)rp)->gc);
		send2(client,(short) ((xPutImageReq *)rp)->width);
		send2(client,(short) ((xPutImageReq *)rp)->height);
		send2(client,(short) ((xPutImageReq *)rp)->dstX);
		send2(client,(short) ((xPutImageReq *)rp)->dstY);
		send1(client,(long) ((xPutImageReq *)rp)->leftPad);
		send1(client,(long) ((xPutImageReq *)rp)->depth);
		sendpad(client,2);

		switch (Get_Test_Type(client)) {
		case TOO_LONG:
		case BAD_LENGTH:
			n = 0;
			break;
		default:
			n = bytes_out - sizeof(xPutImageReq);
			break;
		}
		squeeze_me_in(client, (long)n);
		for (row = 0; n > 0 && row < (int)((xPutImageReq *)rp)->height; row++)
			for(col = 0; col < dst_width; col++) {
				if (col < src_width)  {
					drop = (char *)((long)(*dst)++ ^ flip);
					*drop = *(src++);
				}  else  {
					(*dst)++;
				}
				n--;
			}
	        }
		break;
	case X_GetImage:
		send1(client,(long) ((xGetImageReq *)rp)->reqType);
		send1(client,(long) ((xGetImageReq *)rp)->format);
		send2(client,(short) ((xGetImageReq *)rp)->length);
		send4(client,(long) ((xGetImageReq *)rp)->drawable);
		send2(client,(short) ((xGetImageReq *)rp)->x);
		send2(client,(short) ((xGetImageReq *)rp)->y);
		send2(client,(short) ((xGetImageReq *)rp)->width);
		send2(client,(short) ((xGetImageReq *)rp)->height);
		send4(client,(long) ((xGetImageReq *)rp)->planeMask);
		Xst_clients[client].cl_imagewidth = ((xGetImageReq *)rp)->width;
		Xst_clients[client].cl_imageheight = ((xGetImageReq *)rp)->height;
		break;
	case X_PolyText8:
		send1(client,(long) ((xPolyText8Req *)rp)->reqType);
		sendpad(client,1);
		send2(client,(short) ((xPolyText8Req *)rp)->length);
		send4(client,(long) ((xPolyText8Req *)rp)->drawable);
		send4(client,(long) ((xPolyText8Req *)rp)->gc);
		send2(client,(short) ((xPolyText8Req *)rp)->x);
		send2(client,(short) ((xPolyText8Req *)rp)->y);
		Send_TextItem8(client, rp, sizeof (xPolyText8Req));
		break;
	case X_PolyText16:
		send1(client,(long) ((xPolyText16Req *)rp)->reqType);
		sendpad(client,1);
		send2(client,(short) ((xPolyText16Req *)rp)->length);
		send4(client,(long) ((xPolyText16Req *)rp)->drawable);
		send4(client,(long) ((xPolyText16Req *)rp)->gc);
		send2(client,(short) ((xPolyText16Req *)rp)->x);
		send2(client,(short) ((xPolyText16Req *)rp)->y);
		Send_TextItem16(client, rp, sizeof (xPolyText16Req));
		break;
	case X_ImageText8:
		send1(client,(long) ((xImageText8Req *)rp)->reqType);
		send1(client,(long) ((xImageText8Req *)rp)->nChars);
		send2(client,(short) ((xImageText8Req *)rp)->length);
		send4(client,(long) ((xImageText8Req *)rp)->drawable);
		send4(client,(long) ((xImageText8Req *)rp)->gc);
		send2(client,(short) ((xImageText8Req *)rp)->x);
		send2(client,(short) ((xImageText8Req *)rp)->y);
		Send_Value_List(client, rp, sizeof (xImageText8Req), 8);
		break;
	case X_ImageText16:
		send1(client,(long) ((xImageText16Req *)rp)->reqType);
		send1(client,(long) ((xImageText16Req *)rp)->nChars);
		send2(client,(short) ((xImageText16Req *)rp)->length);
		send4(client,(long) ((xImageText16Req *)rp)->drawable);
		send4(client,(long) ((xImageText16Req *)rp)->gc);
		send2(client,(short) ((xImageText16Req *)rp)->x);
		send2(client,(short) ((xImageText16Req *)rp)->y);
		Send_CHAR2B(client, rp, sizeof (xImageText16Req));
		break;
	case X_CreateColormap:
		send1(client,(long) ((xCreateColormapReq *)rp)->reqType);
		send1(client,(long) ((xCreateColormapReq *)rp)->alloc);
		send2(client,(short) ((xCreateColormapReq *)rp)->length);
		send4(client,(long) ((xCreateColormapReq *)rp)->mid);
		send4(client,(long) ((xCreateColormapReq *)rp)->window);
		send4(client,(long) ((xCreateColormapReq *)rp)->visual);
		break;
	case X_FreeColormap:
		send1(client,(long) ((xResourceReq *)rp)->reqType);
		sendpad(client,1);
		send2(client,(short) ((xResourceReq *)rp)->length);
		send4(client,(long) ((xResourceReq *)rp)->id);
		break;
	case X_CopyColormapAndFree:
		send1(client,(long) ((xCopyColormapAndFreeReq *)rp)->reqType);
		sendpad(client,1);
		send2(client,(short) ((xCopyColormapAndFreeReq *)rp)->length);
		send4(client,(long) ((xCopyColormapAndFreeReq *)rp)->mid);
		send4(client,(long) ((xCopyColormapAndFreeReq *)rp)->srcCmap);
		break;
	case X_InstallColormap:
		send1(client,(long) ((xResourceReq *)rp)->reqType);
		sendpad(client,1);
		send2(client,(short) ((xResourceReq *)rp)->length);
		send4(client,(long) ((xResourceReq *)rp)->id);
		break;
	case X_UninstallColormap:
		send1(client,(long) ((xResourceReq *)rp)->reqType);
		sendpad(client,1);
		send2(client,(short) ((xResourceReq *)rp)->length);
		send4(client,(long) ((xResourceReq *)rp)->id);
		break;
	case X_ListInstalledColormaps:
		send1(client,(long) ((xResourceReq *)rp)->reqType);
		sendpad(client,1);
		send2(client,(short) ((xResourceReq *)rp)->length);
		send4(client,(long) ((xResourceReq *)rp)->id);
		break;
	case X_AllocColor:
		send1(client,(long) ((xAllocColorReq *)rp)->reqType);
		sendpad(client,1);
		send2(client,(short) ((xAllocColorReq *)rp)->length);
		send4(client,(long) ((xAllocColorReq *)rp)->cmap);
		send2(client,(short) ((xAllocColorReq *)rp)->red);
		send2(client,(short) ((xAllocColorReq *)rp)->green);
		send2(client,(short) ((xAllocColorReq *)rp)->blue);
		sendpad(client,2);
		break;
	case X_AllocNamedColor:
		send1(client,(long) ((xAllocNamedColorReq *)rp)->reqType);
		sendpad(client,1);
		send2(client,(short) ((xAllocNamedColorReq *)rp)->length);
		send4(client,(long) ((xAllocNamedColorReq *)rp)->cmap);
		send2(client,(short) ((xAllocNamedColorReq *)rp)->nbytes);
		sendpad(client,2);
		Send_Value_List(client, rp, sizeof (xAllocNamedColorReq), 8);
		break;
	case X_AllocColorCells:
		send1(client,(long) ((xAllocColorCellsReq *)rp)->reqType);
		send1(client,(long) ((xAllocColorCellsReq *)rp)->contiguous);
		send2(client,(short) ((xAllocColorCellsReq *)rp)->length);
		send4(client,(long) ((xAllocColorCellsReq *)rp)->cmap);
		send2(client,(short) ((xAllocColorCellsReq *)rp)->colors);
		send2(client,(short) ((xAllocColorCellsReq *)rp)->planes);
		break;
	case X_AllocColorPlanes:
		send1(client,(long) ((xAllocColorPlanesReq *)rp)->reqType);
		send1(client,(long) ((xAllocColorPlanesReq *)rp)->contiguous);
		send2(client,(short) ((xAllocColorPlanesReq *)rp)->length);
		send4(client,(long) ((xAllocColorPlanesReq *)rp)->cmap);
		send2(client,(short) ((xAllocColorPlanesReq *)rp)->colors);
		send2(client,(short) ((xAllocColorPlanesReq *)rp)->red);
		send2(client,(short) ((xAllocColorPlanesReq *)rp)->green);
		send2(client,(short) ((xAllocColorPlanesReq *)rp)->blue);
		break;
	case X_FreeColors:
		send1(client,(long) ((xFreeColorsReq *)rp)->reqType);
		sendpad(client,1);
		send2(client,(short) ((xFreeColorsReq *)rp)->length);
		send4(client,(long) ((xFreeColorsReq *)rp)->cmap);
		send4(client,(long) ((xFreeColorsReq *)rp)->planeMask);
		Send_Value_List(client, rp, sizeof (xFreeColorsReq), 32);
		break;
	case X_StoreColors:
		send1(client,(long) ((xStoreColorsReq *)rp)->reqType);
		sendpad(client,1);
		send2(client,(short) ((xStoreColorsReq *)rp)->length);
		send4(client,(long) ((xStoreColorsReq *)rp)->cmap);
		Send_Value_List(client, rp, sizeof (xStoreColorsReq), 32);
		break;
	case X_StoreNamedColor:
		send1(client,(long) ((xStoreNamedColorReq *)rp)->reqType);
		send1(client,(long) ((xStoreNamedColorReq *)rp)->flags);
		send2(client,(short) ((xStoreNamedColorReq *)rp)->length);
		send4(client,(long) ((xStoreNamedColorReq *)rp)->cmap);
		send4(client,(long) ((xStoreNamedColorReq *)rp)->pixel);
		send2(client,(short) ((xStoreNamedColorReq *)rp)->nbytes);
		sendpad(client,2);
		Send_Value_List(client, rp, sizeof (xStoreNamedColorReq), 8);
		break;
	case X_QueryColors:
		send1(client,(long) ((xQueryColorsReq *)rp)->reqType);
		sendpad(client,1);
		send2(client,(short) ((xQueryColorsReq *)rp)->length);
		send4(client,(long) ((xQueryColorsReq *)rp)->cmap);
		Send_Value_List(client, rp, sizeof (xQueryColorsReq), 32);
		break;
	case X_LookupColor:
		send1(client,(long) ((xLookupColorReq *)rp)->reqType);
		sendpad(client,1);
		send2(client,(short) ((xLookupColorReq *)rp)->length);
		send4(client,(long) ((xLookupColorReq *)rp)->cmap);
		send2(client,(short) ((xLookupColorReq *)rp)->nbytes);
		sendpad(client,2);
		Send_Value_List(client, rp, sizeof (xLookupColorReq), 8);
		break;
	case X_CreateCursor:
		send1(client,(long) ((xCreateCursorReq *)rp)->reqType);
		sendpad(client,1);
		send2(client,(short) ((xCreateCursorReq *)rp)->length);
		send4(client,(long) ((xCreateCursorReq *)rp)->cid);
		send4(client,(long) ((xCreateCursorReq *)rp)->source);
		send4(client,(long) ((xCreateCursorReq *)rp)->mask);
		send2(client,(short) ((xCreateCursorReq *)rp)->foreRed);
		send2(client,(short) ((xCreateCursorReq *)rp)->foreGreen);
		send2(client,(short) ((xCreateCursorReq *)rp)->foreBlue);
		send2(client,(short) ((xCreateCursorReq *)rp)->backRed);
		send2(client,(short) ((xCreateCursorReq *)rp)->backGreen);
		send2(client,(short) ((xCreateCursorReq *)rp)->backBlue);
		send2(client,(short) ((xCreateCursorReq *)rp)->x);
		send2(client,(short) ((xCreateCursorReq *)rp)->y);
		break;
	case X_CreateGlyphCursor:
		send1(client,(long) ((xCreateGlyphCursorReq *)rp)->reqType);
		sendpad(client,1);
		send2(client,(short) ((xCreateGlyphCursorReq *)rp)->length);
		send4(client,(long) ((xCreateGlyphCursorReq *)rp)->cid);
		send4(client,(long) ((xCreateGlyphCursorReq *)rp)->source);
		send4(client,(long) ((xCreateGlyphCursorReq *)rp)->mask);
		send2(client,(short) ((xCreateGlyphCursorReq *)rp)->sourceChar);
		send2(client,(short) ((xCreateGlyphCursorReq *)rp)->maskChar);
		send2(client,(short) ((xCreateGlyphCursorReq *)rp)->foreRed);
		send2(client,(short) ((xCreateGlyphCursorReq *)rp)->foreGreen);
		send2(client,(short) ((xCreateGlyphCursorReq *)rp)->foreBlue);
		send2(client,(short) ((xCreateGlyphCursorReq *)rp)->backRed);
		send2(client,(short) ((xCreateGlyphCursorReq *)rp)->backGreen);
		send2(client,(short) ((xCreateGlyphCursorReq *)rp)->backBlue);
		break;
	case X_FreeCursor:
		send1(client,(long) ((xResourceReq *)rp)->reqType);
		sendpad(client,1);
		send2(client,(short) ((xResourceReq *)rp)->length);
		send4(client,(long) ((xResourceReq *)rp)->id);
		break;
	case X_RecolorCursor:
		send1(client,(long) ((xRecolorCursorReq *)rp)->reqType);
		sendpad(client,1);
		send2(client,(short) ((xRecolorCursorReq *)rp)->length);
		send4(client,(long) ((xRecolorCursorReq *)rp)->cursor);
		send2(client,(short) ((xRecolorCursorReq *)rp)->foreRed);
		send2(client,(short) ((xRecolorCursorReq *)rp)->foreGreen);
		send2(client,(short) ((xRecolorCursorReq *)rp)->foreBlue);
		send2(client,(short) ((xRecolorCursorReq *)rp)->backRed);
		send2(client,(short) ((xRecolorCursorReq *)rp)->backGreen);
		send2(client,(short) ((xRecolorCursorReq *)rp)->backBlue);
		break;
	case X_QueryBestSize:
		send1(client,(long) ((xQueryBestSizeReq *)rp)->reqType);
		send1(client,(long) ((xQueryBestSizeReq *)rp)->class);
		send2(client,(short) ((xQueryBestSizeReq *)rp)->length);
		send4(client,(long) ((xQueryBestSizeReq *)rp)->drawable);
		send2(client,(short) ((xQueryBestSizeReq *)rp)->width);
		send2(client,(short) ((xQueryBestSizeReq *)rp)->height);
		break;
	case X_QueryExtension:
		send1(client,(long) ((xQueryExtensionReq *)rp)->reqType);
		sendpad(client,1);
		send2(client,(short) ((xQueryExtensionReq *)rp)->length);
		send2(client,(short) ((xQueryExtensionReq *)rp)->nbytes);
		sendpad(client,2);
		Send_Value_List(client, rp, sizeof (xQueryExtensionReq), 8);
		break;
	case X_ListExtensions:
		send1(client,(long) ((xResourceReq *)rp)->reqType);
		sendpad(client,1);
		send2(client,(short) ((xResourceReq *)rp)->length);
		break;
	case X_ChangeKeyboardMapping:
		send1(client,(long) ((xChangeKeyboardMappingReq *)rp)->reqType);
		send1(client,(long) ((xChangeKeyboardMappingReq *)rp)->keyCodes);
		send2(client,(short) ((xChangeKeyboardMappingReq *)rp)->length);
		send1(client,(long) ((xChangeKeyboardMappingReq *)rp)->firstKeyCode);
		send1(client,(long) ((xChangeKeyboardMappingReq *)rp)->keySymsPerKeyCode);
		sendpad(client,2);
		Send_Value_List(client, rp, sizeof (xChangeKeyboardMappingReq), 32);
		break;
	case X_GetKeyboardMapping:
		send1(client,(long) ((xGetKeyboardMappingReq *)rp)->reqType);
		sendpad(client,1);
		send2(client,(short) ((xGetKeyboardMappingReq *)rp)->length);
		send1(client,(long) ((xGetKeyboardMappingReq *)rp)->firstKeyCode);
		send1(client,(long) ((xGetKeyboardMappingReq *)rp)->count);
		sendpad(client,2);
		break;
	case X_ChangeKeyboardControl:
		send1(client,(long) ((xChangeKeyboardControlReq *)rp)->reqType);
		sendpad(client,1);
		send2(client,(short) ((xChangeKeyboardControlReq *)rp)->length);
		send4(client,(long) ((xChangeKeyboardControlReq *)rp)->mask);
		Send_Value_List(client, rp, sizeof (xChangeKeyboardControlReq), 32);
		break;
	case X_GetKeyboardControl:
		send1(client,(long) ((xResourceReq *)rp)->reqType);
		sendpad(client,1);
		send2(client,(short) ((xResourceReq *)rp)->length);
		break;
	case X_Bell:
		send1(client,(long) ((xBellReq *)rp)->reqType);
		send1(client,(long) ((xBellReq *)rp)->percent);
		send2(client,(short) ((xBellReq *)rp)->length);
		break;
	case X_ChangePointerControl:
		send1(client,(long) ((xChangePointerControlReq *)rp)->reqType);
		sendpad(client,1);
		send2(client,(short) ((xChangePointerControlReq *)rp)->length);
		send2(client,(short) ((xChangePointerControlReq *)rp)->accelNum);
		send2(client,(short) ((xChangePointerControlReq *)rp)->accelDenum);
		send2(client,(short) ((xChangePointerControlReq *)rp)->threshold);
		send1(client,(long) ((xChangePointerControlReq *)rp)->doAccel);
		send1(client,(long) ((xChangePointerControlReq *)rp)->doThresh);
		break;
	case X_GetPointerControl:
		send1(client,(long) ((xResourceReq *)rp)->reqType);
		sendpad(client,1);
		send2(client,(short) ((xResourceReq *)rp)->length);
		break;
	case X_SetScreenSaver:
		send1(client,(long) ((xSetScreenSaverReq *)rp)->reqType);
		sendpad(client,1);
		send2(client,(short) ((xSetScreenSaverReq *)rp)->length);
		send2(client,(short) ((xSetScreenSaverReq *)rp)->timeout);
		send2(client,(short) ((xSetScreenSaverReq *)rp)->interval);
		send1(client,(long) ((xSetScreenSaverReq *)rp)->preferBlank);
		send1(client,(long) ((xSetScreenSaverReq *)rp)->allowExpose);
		sendpad(client,2);
		break;
	case X_GetScreenSaver:
		send1(client,(long) ((xResourceReq *)rp)->reqType);
		sendpad(client,1);
		send2(client,(short) ((xResourceReq *)rp)->length);
		break;
	case X_ChangeHosts:
		send1(client,(long) ((xChangeHostsReq *)rp)->reqType);
		send1(client,(long) ((xChangeHostsReq *)rp)->mode);
		send2(client,(short) ((xChangeHostsReq *)rp)->length);
		send1(client,(long) ((xChangeHostsReq *)rp)->hostFamily);
		sendpad(client,1);
		send2(client,(short) ((xChangeHostsReq *)rp)->hostLength);
debug(3,"before Send_Value_List length is %d, req at 0x%lx\n",rp->length,(unsigned long)rp);
		Send_Value_List(client, rp, sizeof (xChangeHostsReq), 8);
debug(3,"after Send_Value_List length is %d, req at 0x%lx\n",rp->length,(unsigned long)rp);
		break;
	case X_ListHosts:
		send1(client,(long) ((xListHostsReq *)rp)->reqType);
		sendpad(client,1);
		send2(client,(short) ((xListHostsReq *)rp)->length);
		break;
	case X_SetAccessControl:
		send1(client,(long) ((xSetAccessControlReq *)rp)->reqType);
		send1(client,(long) ((xSetAccessControlReq *)rp)->mode);
		send2(client,(short) ((xSetAccessControlReq *)rp)->length);
		break;
	case X_SetCloseDownMode:
		send1(client,(long) ((xSetCloseDownModeReq *)rp)->reqType);
		send1(client,(long) ((xSetCloseDownModeReq *)rp)->mode);
		send2(client,(short) ((xSetCloseDownModeReq *)rp)->length);
		break;
	case X_KillClient:
		send1(client,(long) ((xResourceReq *)rp)->reqType);
		sendpad(client,1);
		send2(client,(short) ((xResourceReq *)rp)->length);
		send4(client,(long) ((xResourceReq *)rp)->id);
		break;
	case X_RotateProperties:
		send1(client,(long) ((xRotatePropertiesReq *)rp)->reqType);
		sendpad(client,1);
		send2(client,(short) ((xRotatePropertiesReq *)rp)->length);
		send4(client,(long) ((xRotatePropertiesReq *)rp)->window);
		send2(client,(short) ((xRotatePropertiesReq *)rp)->nAtoms);
		send2(client,(short) ((xRotatePropertiesReq *)rp)->nPositions);
		Send_Value_List(client, rp, sizeof (xRotatePropertiesReq), 32);
		break;
	case X_ForceScreenSaver:
		send1(client,(long) ((xForceScreenSaverReq *)rp)->reqType);
		send1(client,(long) ((xForceScreenSaverReq *)rp)->mode);
		send2(client,(short) ((xForceScreenSaverReq *)rp)->length);
		break;
	case X_SetPointerMapping:
		send1(client,(long) ((xSetPointerMappingReq *)rp)->reqType);
		send1(client,(long) ((xSetPointerMappingReq *)rp)->nElts);
		send2(client,(short) ((xSetPointerMappingReq *)rp)->length);
		Send_Value_List(client, rp, sizeof (xSetPointerMappingReq), 8);
		break;
	case X_GetPointerMapping:
		send1(client,(long) ((xResourceReq *)rp)->reqType);
		sendpad(client,1);
		send2(client,(short) ((xResourceReq *)rp)->length);
		break;
	case X_SetModifierMapping:
		send1(client,(long) ((xSetModifierMappingReq *)rp)->reqType);
		send1(client,(long) ((xSetModifierMappingReq *)rp)->numKeyPerModifier);
		send2(client,(short) ((xSetModifierMappingReq *)rp)->length);
		Send_Value_List(client, rp, sizeof (xSetModifierMappingReq), 8);
		break;
	case X_GetModifierMapping:
		send1(client,(long) ((xResourceReq *)rp)->reqType);
		sendpad(client,1);
		send2(client,(short) ((xResourceReq *)rp)->length);
		break;
	case X_NoOperation:
		send1(client,(long) ((xResourceReq *)rp)->reqType);
		sendpad(client,1);
		send2(client,(short) ((xResourceReq *)rp)->length);
		break;
	case Xst_BadType:
		send1(client,(long) ((xResourceReq *)rp)->reqType);
		sendpad(client,1);
		send2(client,(short) ((xResourceReq *)rp)->length);
		break;
	case Xst_BadLength:
		send1(client,(long) X_CreateWindow);
		send1(client,(long) ((xCreateWindowReq *)rp)->depth);
		send2(client,(short) ((xCreateWindowReq *)rp)->length);
		send4(client,(long) ((xCreateWindowReq *)rp)->wid);
		send4(client,(long) ((xCreateWindowReq *)rp)->parent);
		send2(client,(short) ((xCreateWindowReq *)rp)->x);
		send2(client,(short) ((xCreateWindowReq *)rp)->y);
		send2(client,(short) ((xCreateWindowReq *)rp)->width);
		send2(client,(short) ((xCreateWindowReq *)rp)->height);
		send2(client,(short) ((xCreateWindowReq *)rp)->borderWidth);
		send2(client,(short) ((xCreateWindowReq *)rp)->class);
		send4(client,(long) ((xCreateWindowReq *)rp)->visual);
		send4(client,(long) ((xCreateWindowReq *)rp)->mask);
#ifdef notdef
		bytes_out =   /* must fake out SendIt to let us go! */
			dpy->bufptr - dpy->buffer;
#endif
		break;
	default:
		DEFAULT_ERROR;
		break;
	}
	SendIt(client,bytes_out,oldlen);

}

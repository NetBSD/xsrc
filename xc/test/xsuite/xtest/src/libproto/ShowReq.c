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
 * $XConsortium: ShowReq.c,v 1.10 94/04/17 21:01:29 rws Exp $
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

void
Show_Req(mp)
xReq *mp;
{
	/*
	 * This is really one switch, it is split up here to accomodate
	 * compilers that cannot cope with large switches properly.
	 */
	if (mp->reqType > X_NoOperation) {
	    Show_Ext_Req(mp);
	    return;
	}

	switch (mp->reqType) {
	case X_CreateWindow:
		BPRINTF1("CreateWindow:\n");
		BPRINTF2("\treqType = %ld\n",(long) ((xCreateWindowReq *)mp)->reqType);
		BPRINTF2("\tdepth = %d\n",((xCreateWindowReq *)mp)->depth);
		BPRINTF2("\tlength = %ld\n",(long) ((xCreateWindowReq *)mp)->length);
		BPRINTF2("\twid = %ld\n",((xCreateWindowReq *)mp)->wid);
		BPRINTF2("\tparent = %ld\n",((xCreateWindowReq *)mp)->parent);
		BPRINTF2("\tx = %d\n",((xCreateWindowReq *)mp)->x);
		BPRINTF2("\ty = %d\n",((xCreateWindowReq *)mp)->y);
		BPRINTF2("\twidth = %d\n",((xCreateWindowReq *)mp)->width);
		BPRINTF2("\theight = %d\n",((xCreateWindowReq *)mp)->height);
		BPRINTF2("\tborderWidth = %d\n",((xCreateWindowReq *)mp)->borderWidth);
		BPRINTF2("\tclass = %ld\n",(long) ((xCreateWindowReq *)mp)->class);
		BPRINTF2("\tvisual = %ld\n",((xCreateWindowReq *)mp)->visual);
		BPRINTF2("\tvalue-mask (has n 1-bits) = 0x%08x\n",((xCreateWindowReq *)mp)->mask);
		Show_Value_List_Req(mp,sizeof(xCreateWindowReq),FORMAT32);
		break;
	case X_ChangeWindowAttributes:
		BPRINTF1("ChangeWindowAttributes:\n");
		BPRINTF2("\treqType = %ld\n",(long) ((xChangeWindowAttributesReq *)mp)->reqType);
		BPRINTF2("\tlength = %ld\n",(long) ((xChangeWindowAttributesReq *)mp)->length);
		BPRINTF2("\twindow = %ld\n",((xChangeWindowAttributesReq *)mp)->window);
		BPRINTF2("\tvalue-mask (has n 1-bits) = 0x%08x\n",((xChangeWindowAttributesReq *)mp)->valueMask);
		Show_Value_List_Req(mp,sizeof(xChangeWindowAttributesReq),FORMAT32);
		break;
	case X_GetWindowAttributes:
		BPRINTF1("GetWindowAttributes:\n");
		BPRINTF2("\treqType = %ld\n",(long) ((xResourceReq *)mp)->reqType);
		BPRINTF2("\tlength = %ld\n",(long) ((xResourceReq *)mp)->length);
		BPRINTF2("\tid = %ld\n",((xResourceReq *)mp)->id);
		break;
	case X_DestroyWindow:
		BPRINTF1("DestroyWindow:\n");
		BPRINTF2("\treqType = %ld\n",(long) ((xResourceReq *)mp)->reqType);
		BPRINTF2("\tlength = %ld\n",(long) ((xResourceReq *)mp)->length);
		BPRINTF2("\tid = %ld\n",((xResourceReq *)mp)->id);
		break;
	case X_DestroySubwindows:
		BPRINTF1("DestroySubwindows:\n");
		BPRINTF2("\treqType = %ld\n",(long) ((xResourceReq *)mp)->reqType);
		BPRINTF2("\tlength = %ld\n",(long) ((xResourceReq *)mp)->length);
		BPRINTF2("\tid = %ld\n",((xResourceReq *)mp)->id);
		break;
	case X_ChangeSaveSet:
		BPRINTF1("ChangeSaveSet:\n");
		BPRINTF2("\treqType = %ld\n",(long) ((xChangeSaveSetReq *)mp)->reqType);
		BPRINTF2("\tmode = %ld\n",(long) ((xChangeSaveSetReq *)mp)->mode);
		BPRINTF2("\tlength = %ld\n",(long) ((xChangeSaveSetReq *)mp)->length);
		BPRINTF2("\twindow = %ld\n",((xChangeSaveSetReq *)mp)->window);
		break;
	case X_ReparentWindow:
		BPRINTF1("ReparentWindow:\n");
		BPRINTF2("\treqType = %ld\n",(long) ((xReparentWindowReq *)mp)->reqType);
		BPRINTF2("\tlength = %ld\n",(long) ((xReparentWindowReq *)mp)->length);
		BPRINTF2("\twindow = %ld\n",((xReparentWindowReq *)mp)->window);
		BPRINTF2("\tparent = %ld\n",((xReparentWindowReq *)mp)->parent);
		BPRINTF2("\tx = %d\n",((xReparentWindowReq *)mp)->x);
		BPRINTF2("\ty = %d\n",((xReparentWindowReq *)mp)->y);
		break;
	case X_MapWindow:
		BPRINTF1("MapWindow:\n");
		BPRINTF2("\treqType = %ld\n",(long) ((xResourceReq *)mp)->reqType);
		BPRINTF2("\tlength = %ld\n",(long) ((xResourceReq *)mp)->length);
		BPRINTF2("\tid = %ld\n",((xResourceReq *)mp)->id);
		break;
	case X_MapSubwindows:
		BPRINTF1("MapSubwindows:\n");
		BPRINTF2("\treqType = %ld\n",(long) ((xResourceReq *)mp)->reqType);
		BPRINTF2("\tlength = %ld\n",(long) ((xResourceReq *)mp)->length);
		BPRINTF2("\tid = %ld\n",((xResourceReq *)mp)->id);
		break;
	case X_UnmapWindow:
		BPRINTF1("UnmapWindow:\n");
		BPRINTF2("\treqType = %ld\n",(long) ((xResourceReq *)mp)->reqType);
		BPRINTF2("\tlength = %ld\n",(long) ((xResourceReq *)mp)->length);
		BPRINTF2("\tid = %ld\n",((xResourceReq *)mp)->id);
		break;
	case X_UnmapSubwindows:
		BPRINTF1("UnmapSubwindows:\n");
		BPRINTF2("\treqType = %ld\n",(long) ((xResourceReq *)mp)->reqType);
		BPRINTF2("\tlength = %ld\n",(long) ((xResourceReq *)mp)->length);
		BPRINTF2("\tid = %ld\n",((xResourceReq *)mp)->id);
		break;
	case X_ConfigureWindow:
		BPRINTF1("ConfigureWindow:\n");
		BPRINTF2("\treqType = %ld\n",(long) ((xConfigureWindowReq *)mp)->reqType);
		BPRINTF2("\tlength = %ld\n",(long) ((xConfigureWindowReq *)mp)->length);
		BPRINTF2("\twindow = %ld\n",((xConfigureWindowReq *)mp)->window);
		BPRINTF2("\tvalue-mask (has n 1-bits) = 0x%08x\n",((xConfigureWindowReq *)mp)->mask);
		Show_Value_List_Req(mp,sizeof(xConfigureWindowReq),FORMAT32);
		break;
	case X_CirculateWindow:
		BPRINTF1("CirculateWindow:\n");
		BPRINTF2("\treqType = %ld\n",(long) ((xCirculateWindowReq *)mp)->reqType);
		BPRINTF2("\tdirection = %ld\n",(long) ((xCirculateWindowReq *)mp)->direction);
		BPRINTF2("\tlength = %ld\n",(long) ((xCirculateWindowReq *)mp)->length);
		BPRINTF2("\twindow = %ld\n",((xCirculateWindowReq *)mp)->window);
		break;
	case X_GetGeometry:
		BPRINTF1("GetGeometry:\n");
		BPRINTF2("\treqType = %ld\n",(long) ((xResourceReq *)mp)->reqType);
		BPRINTF2("\tlength = %ld\n",(long) ((xResourceReq *)mp)->length);
		BPRINTF2("\tid = %ld\n",((xResourceReq *)mp)->id);
		break;
	case X_QueryTree:
		BPRINTF1("QueryTree:\n");
		BPRINTF2("\treqType = %ld\n",(long) ((xResourceReq *)mp)->reqType);
		BPRINTF2("\tlength = %ld\n",(long) ((xResourceReq *)mp)->length);
		BPRINTF2("\tid = %ld\n",((xResourceReq *)mp)->id);
		break;
	case X_InternAtom:
		BPRINTF1("InternAtom:\n");
		BPRINTF2("\treqType = %ld\n",(long) ((xInternAtomReq *)mp)->reqType);
		BPRINTF2("\tonlyIfExists = %d\n",((xInternAtomReq *)mp)->onlyIfExists);
		BPRINTF2("\tlength = %ld\n",(long) ((xInternAtomReq *)mp)->length);
		BPRINTF2("\tnbytes = %ld\n",(long) ((xInternAtomReq *)mp)->nbytes);
		Show_String8(mp,sizeof(xInternAtomReq),((xInternAtomReq *)mp)->nbytes);
		break;
	case X_GetAtomName:
		BPRINTF1("GetAtomName:\n");
		BPRINTF2("\treqType = %ld\n",(long) ((xResourceReq *)mp)->reqType);
		BPRINTF2("\tlength = %ld\n",(long) ((xResourceReq *)mp)->length);
		BPRINTF2("\tid = %ld\n",((xResourceReq *)mp)->id);
		break;
	case X_ChangeProperty:
		BPRINTF1("ChangeProperty:\n");
		BPRINTF2("\treqType = %ld\n",(long) ((xChangePropertyReq *)mp)->reqType);
		BPRINTF2("\tmode = %ld\n",(long) ((xChangePropertyReq *)mp)->mode);
		BPRINTF2("\tlength = %ld\n",(long) ((xChangePropertyReq *)mp)->length);
		BPRINTF2("\twindow = %ld\n",((xChangePropertyReq *)mp)->window);
		BPRINTF2("\tproperty = %ld\n",((xChangePropertyReq *)mp)->property);
		BPRINTF2("\ttype = %ld\n",((xChangePropertyReq *)mp)->type);
		BPRINTF2("\tformat = %d\n",((xChangePropertyReq *)mp)->format);
		BPRINTF2("\tnUnits = %ld\n",((xChangePropertyReq *)mp)->nUnits);
		Show_Value_List_Req(mp,sizeof(xChangePropertyReq),((xChangePropertyReq *)mp)->format);
		break;
	case X_DeleteProperty:
		BPRINTF1("DeleteProperty:\n");
		BPRINTF2("\treqType = %ld\n",(long) ((xDeletePropertyReq *)mp)->reqType);
		BPRINTF2("\tlength = %ld\n",(long) ((xDeletePropertyReq *)mp)->length);
		BPRINTF2("\twindow = %ld\n",((xDeletePropertyReq *)mp)->window);
		BPRINTF2("\tproperty = %ld\n",((xDeletePropertyReq *)mp)->property);
		break;
	case X_GetProperty:
		BPRINTF1("GetProperty:\n");
		BPRINTF2("\treqType = %ld\n",(long) ((xGetPropertyReq *)mp)->reqType);
		BPRINTF2("\tdelete = %d\n",((xGetPropertyReq *)mp)->delete);
		BPRINTF2("\tlength = %ld\n",(long) ((xGetPropertyReq *)mp)->length);
		BPRINTF2("\twindow = %ld\n",((xGetPropertyReq *)mp)->window);
		BPRINTF2("\tproperty = %ld\n",((xGetPropertyReq *)mp)->property);
		BPRINTF2("\ttype = %ld\n",((xGetPropertyReq *)mp)->type);
		BPRINTF2("\tlongOffset = %ld\n",((xGetPropertyReq *)mp)->longOffset);
		BPRINTF2("\tlongLength = %ld\n",((xGetPropertyReq *)mp)->longLength);
		break;
	case X_ListProperties:
		BPRINTF1("ListProperties:\n");
		BPRINTF2("\treqType = %ld\n",(long) ((xResourceReq *)mp)->reqType);
		BPRINTF2("\tlength = %ld\n",(long) ((xResourceReq *)mp)->length);
		BPRINTF2("\tid = %ld\n",((xResourceReq *)mp)->id);
		break;
	case X_SetSelectionOwner:
		BPRINTF1("SetSelectionOwner:\n");
		BPRINTF2("\treqType = %ld\n",(long) ((xSetSelectionOwnerReq *)mp)->reqType);
		BPRINTF2("\tlength = %ld\n",(long) ((xSetSelectionOwnerReq *)mp)->length);
		BPRINTF2("\twindow = %ld\n",((xSetSelectionOwnerReq *)mp)->window);
		BPRINTF2("\tselection = %ld\n",((xSetSelectionOwnerReq *)mp)->selection);
		BPRINTF2("\ttime = %ld\n",((xSetSelectionOwnerReq *)mp)->time);
		break;
	case X_GetSelectionOwner:
		BPRINTF1("GetSelectionOwner:\n");
		BPRINTF2("\treqType = %ld\n",(long) ((xResourceReq *)mp)->reqType);
		BPRINTF2("\tlength = %ld\n",(long) ((xResourceReq *)mp)->length);
		BPRINTF2("\tid = %ld\n",((xResourceReq *)mp)->id);
		break;
	case X_ConvertSelection:
		BPRINTF1("ConvertSelection:\n");
		BPRINTF2("\treqType = %ld\n",(long) ((xConvertSelectionReq *)mp)->reqType);
		BPRINTF2("\tlength = %ld\n",(long) ((xConvertSelectionReq *)mp)->length);
		BPRINTF2("\trequestor = %ld\n",((xConvertSelectionReq *)mp)->requestor);
		BPRINTF2("\tselection = %ld\n",((xConvertSelectionReq *)mp)->selection);
		BPRINTF2("\ttarget = %ld\n",((xConvertSelectionReq *)mp)->target);
		BPRINTF2("\tproperty = %ld\n",((xConvertSelectionReq *)mp)->property);
		BPRINTF2("\ttime = %ld\n",((xConvertSelectionReq *)mp)->time);
		break;
	case X_SendEvent:
		BPRINTF1("SendEvent:\n");
		BPRINTF2("\treqType = %ld\n",(long) ((xSendEventReq *)mp)->reqType);
		BPRINTF2("\tpropagate = %d\n",((xSendEventReq *)mp)->propagate);
		BPRINTF2("\tlength = %ld\n",(long) ((xSendEventReq *)mp)->length);
		BPRINTF2("\tdestination = %ld\n",((xSendEventReq *)mp)->destination);
		BPRINTF2("\teventMask = 0x%08x\n",((xSendEventReq *)mp)->eventMask);
		Show_Evt(&(((xSendEventReq *)mp)->event));
		break;
	}
	switch (mp->reqType) {
	case X_GrabPointer:
		BPRINTF1("GrabPointer:\n");
		BPRINTF2("\treqType = %ld\n",(long) ((xGrabPointerReq *)mp)->reqType);
		BPRINTF2("\townerEvents = %d\n",((xGrabPointerReq *)mp)->ownerEvents);
		BPRINTF2("\tlength = %ld\n",(long) ((xGrabPointerReq *)mp)->length);
		BPRINTF2("\tgrabWindow = %ld\n",((xGrabPointerReq *)mp)->grabWindow);
		BPRINTF2("\teventMask = 0x%04x\n",((xGrabPointerReq *)mp)->eventMask);
		BPRINTF2("\tpointerMode = %ld\n",(long) ((xGrabPointerReq *)mp)->pointerMode);
		BPRINTF2("\tkeyboardMode = %ld\n",(long) ((xGrabPointerReq *)mp)->keyboardMode);
		BPRINTF2("\tconfineTo = %ld\n",((xGrabPointerReq *)mp)->confineTo);
		BPRINTF2("\tcursor = %ld\n",((xGrabPointerReq *)mp)->cursor);
		BPRINTF2("\ttime = %ld\n",((xGrabPointerReq *)mp)->time);
		break;
	case X_UngrabPointer:
		BPRINTF1("UngrabPointer:\n");
		BPRINTF2("\treqType = %ld\n",(long) ((xResourceReq *)mp)->reqType);
		BPRINTF2("\tlength = %ld\n",(long) ((xResourceReq *)mp)->length);
		BPRINTF2("\tid = %ld\n",((xResourceReq *)mp)->id);
		break;
	case X_GrabButton:
		BPRINTF1("GrabButton:\n");
		BPRINTF2("\treqType = %ld\n",(long) ((xGrabButtonReq *)mp)->reqType);
		BPRINTF2("\townerEvents = %d\n",((xGrabButtonReq *)mp)->ownerEvents);
		BPRINTF2("\tlength = %ld\n",(long) ((xGrabButtonReq *)mp)->length);
		BPRINTF2("\tgrabWindow = %ld\n",((xGrabButtonReq *)mp)->grabWindow);
		BPRINTF2("\teventMask = 0x%04x\n",((xGrabButtonReq *)mp)->eventMask);
		BPRINTF2("\tpointerMode = %ld\n",(long) ((xGrabButtonReq *)mp)->pointerMode);
		BPRINTF2("\tkeyboardMode = %ld\n",(long) ((xGrabButtonReq *)mp)->keyboardMode);
		BPRINTF2("\tconfineTo = %ld\n",((xGrabButtonReq *)mp)->confineTo);
		BPRINTF2("\tcursor = %ld\n",((xGrabButtonReq *)mp)->cursor);
		BPRINTF2("\tbutton = %d\n",((xGrabButtonReq *)mp)->button);
		BPRINTF2("\tmodifiers = 0x%04x\n",((xGrabButtonReq *)mp)->modifiers);
		break;
	case X_UngrabButton:
		BPRINTF1("UngrabButton:\n");
		BPRINTF2("\treqType = %ld\n",(long) ((xUngrabButtonReq *)mp)->reqType);
		BPRINTF2("\tbutton = %d\n",((xUngrabButtonReq *)mp)->button);
		BPRINTF2("\tlength = %ld\n",(long) ((xUngrabButtonReq *)mp)->length);
		BPRINTF2("\tgrabWindow = %ld\n",((xUngrabButtonReq *)mp)->grabWindow);
		BPRINTF2("\tmodifiers = 0x%04x\n",((xUngrabButtonReq *)mp)->modifiers);
		break;
	case X_ChangeActivePointerGrab:
		BPRINTF1("ChangeActivePointerGrab:\n");
		BPRINTF2("\treqType = %ld\n",(long) ((xChangeActivePointerGrabReq *)mp)->reqType);
		BPRINTF2("\tlength = %ld\n",(long) ((xChangeActivePointerGrabReq *)mp)->length);
		BPRINTF2("\tcursor = %ld\n",((xChangeActivePointerGrabReq *)mp)->cursor);
		BPRINTF2("\ttime = %ld\n",((xChangeActivePointerGrabReq *)mp)->time);
		BPRINTF2("\teventMask = 0x%04x\n",((xChangeActivePointerGrabReq *)mp)->eventMask);
		break;
	case X_GrabKeyboard:
		BPRINTF1("GrabKeyboard:\n");
		BPRINTF2("\treqType = %ld\n",(long) ((xGrabKeyboardReq *)mp)->reqType);
		BPRINTF2("\townerEvents = %d\n",((xGrabKeyboardReq *)mp)->ownerEvents);
		BPRINTF2("\tlength = %ld\n",(long) ((xGrabKeyboardReq *)mp)->length);
		BPRINTF2("\tgrabWindow = %ld\n",((xGrabKeyboardReq *)mp)->grabWindow);
		BPRINTF2("\ttime = %ld\n",((xGrabKeyboardReq *)mp)->time);
		BPRINTF2("\tpointerMode = %ld\n",(long) ((xGrabKeyboardReq *)mp)->pointerMode);
		BPRINTF2("\tkeyboardMode = %ld\n",(long) ((xGrabKeyboardReq *)mp)->keyboardMode);
		break;
	case X_UngrabKeyboard:
		BPRINTF1("UngrabKeyboard:\n");
		BPRINTF2("\treqType = %ld\n",(long) ((xResourceReq *)mp)->reqType);
		BPRINTF2("\tlength = %ld\n",(long) ((xResourceReq *)mp)->length);
		BPRINTF2("\tid = %ld\n",((xResourceReq *)mp)->id);
		break;
	case X_GrabKey:
		BPRINTF1("GrabKey:\n");
		BPRINTF2("\treqType = %ld\n",(long) ((xGrabKeyReq *)mp)->reqType);
		BPRINTF2("\townerEvents = %d\n",((xGrabKeyReq *)mp)->ownerEvents);
		BPRINTF2("\tlength = %ld\n",(long) ((xGrabKeyReq *)mp)->length);
		BPRINTF2("\tgrabWindow = %ld\n",((xGrabKeyReq *)mp)->grabWindow);
		BPRINTF2("\tmodifiers = 0x%04x\n",((xGrabKeyReq *)mp)->modifiers);
		BPRINTF2("\tkey = %d\n",((xGrabKeyReq *)mp)->key);
		BPRINTF2("\tpointerMode = %ld\n",(long) ((xGrabKeyReq *)mp)->pointerMode);
		BPRINTF2("\tkeyboardMode = %ld\n",(long) ((xGrabKeyReq *)mp)->keyboardMode);
		break;
	case X_UngrabKey:
		BPRINTF1("UngrabKey:\n");
		BPRINTF2("\treqType = %ld\n",(long) ((xUngrabKeyReq *)mp)->reqType);
		BPRINTF2("\tkey = %d\n",((xUngrabKeyReq *)mp)->key);
		BPRINTF2("\tlength = %ld\n",(long) ((xUngrabKeyReq *)mp)->length);
		BPRINTF2("\tgrabWindow = %ld\n",((xUngrabKeyReq *)mp)->grabWindow);
		BPRINTF2("\tmodifiers = 0x%04x\n",((xUngrabKeyReq *)mp)->modifiers);
		break;
	case X_AllowEvents:
		BPRINTF1("AllowEvents:\n");
		BPRINTF2("\treqType = %ld\n",(long) ((xAllowEventsReq *)mp)->reqType);
		BPRINTF2("\tmode = %ld\n",(long) ((xAllowEventsReq *)mp)->mode);
		BPRINTF2("\tlength = %ld\n",(long) ((xAllowEventsReq *)mp)->length);
		BPRINTF2("\ttime = %ld\n",((xAllowEventsReq *)mp)->time);
		break;
	case X_GrabServer:
		BPRINTF1("GrabServer:\n");
		BPRINTF2("\treqType = %ld\n",(long) ((xResourceReq *)mp)->reqType);
		BPRINTF2("\tlength = %ld\n",(long) ((xResourceReq *)mp)->length);
		break;
	case X_UngrabServer:
		BPRINTF1("UngrabServer:\n");
		BPRINTF2("\treqType = %ld\n",(long) ((xResourceReq *)mp)->reqType);
		BPRINTF2("\tlength = %ld\n",(long) ((xResourceReq *)mp)->length);
		break;
	case X_QueryPointer:
		BPRINTF1("QueryPointer:\n");
		BPRINTF2("\treqType = %ld\n",(long) ((xResourceReq *)mp)->reqType);
		BPRINTF2("\tlength = %ld\n",(long) ((xResourceReq *)mp)->length);
		BPRINTF2("\tid = %ld\n",((xResourceReq *)mp)->id);
		break;
	case X_GetMotionEvents:
		BPRINTF1("GetMotionEvents:\n");
		BPRINTF2("\treqType = %ld\n",(long) ((xGetMotionEventsReq *)mp)->reqType);
		BPRINTF2("\tlength = %ld\n",(long) ((xGetMotionEventsReq *)mp)->length);
		BPRINTF2("\twindow = %ld\n",((xGetMotionEventsReq *)mp)->window);
		BPRINTF2("\tstart = %ld\n",((xGetMotionEventsReq *)mp)->start);
		BPRINTF2("\tstop = %ld\n",((xGetMotionEventsReq *)mp)->stop);
		break;
	case X_TranslateCoords:
		BPRINTF1("TranslateCoords:\n");
		BPRINTF2("\treqType = %ld\n",(long) ((xTranslateCoordsReq *)mp)->reqType);
		BPRINTF2("\tlength = %ld\n",(long) ((xTranslateCoordsReq *)mp)->length);
		BPRINTF2("\tsrcWid = %ld\n",((xTranslateCoordsReq *)mp)->srcWid);
		BPRINTF2("\tdstWid = %ld\n",((xTranslateCoordsReq *)mp)->dstWid);
		BPRINTF2("\tsrcX = %d\n",((xTranslateCoordsReq *)mp)->srcX);
		BPRINTF2("\tsrcY = %d\n",((xTranslateCoordsReq *)mp)->srcY);
		break;
	case X_WarpPointer:
		BPRINTF1("WarpPointer:\n");
		BPRINTF2("\treqType = %ld\n",(long) ((xWarpPointerReq *)mp)->reqType);
		BPRINTF2("\tlength = %ld\n",(long) ((xWarpPointerReq *)mp)->length);
		BPRINTF2("\tsrcWid = %ld\n",((xWarpPointerReq *)mp)->srcWid);
		BPRINTF2("\tdstWid = %ld\n",((xWarpPointerReq *)mp)->dstWid);
		BPRINTF2("\tsrcX = %d\n",((xWarpPointerReq *)mp)->srcX);
		BPRINTF2("\tsrcY = %d\n",((xWarpPointerReq *)mp)->srcY);
		BPRINTF2("\tsrcWidth = %d\n",((xWarpPointerReq *)mp)->srcWidth);
		BPRINTF2("\tsrcHeight = %d\n",((xWarpPointerReq *)mp)->srcHeight);
		BPRINTF2("\tdstX = %d\n",((xWarpPointerReq *)mp)->dstX);
		BPRINTF2("\tdstY = %d\n",((xWarpPointerReq *)mp)->dstY);
		break;
	case X_SetInputFocus:
		BPRINTF1("SetInputFocus:\n");
		BPRINTF2("\treqType = %ld\n",(long) ((xSetInputFocusReq *)mp)->reqType);
		BPRINTF2("\trevertTo = %ld\n",(long) ((xSetInputFocusReq *)mp)->revertTo);
		BPRINTF2("\tlength = %ld\n",(long) ((xSetInputFocusReq *)mp)->length);
		BPRINTF2("\tfocus = %ld\n",((xSetInputFocusReq *)mp)->focus);
		BPRINTF2("\ttime = %ld\n",((xSetInputFocusReq *)mp)->time);
		break;
	case X_GetInputFocus:
		BPRINTF1("GetInputFocus:\n");
		BPRINTF2("\treqType = %ld\n",(long) ((xResourceReq *)mp)->reqType);
		BPRINTF2("\tlength = %ld\n",(long) ((xResourceReq *)mp)->length);
		break;
	case X_QueryKeymap:
		BPRINTF1("QueryKeymap:\n");
		BPRINTF2("\treqType = %ld\n",(long) ((xResourceReq *)mp)->reqType);
		BPRINTF2("\tlength = %ld\n",(long) ((xResourceReq *)mp)->length);
		break;
	case X_OpenFont:
		BPRINTF1("OpenFont:\n");
		BPRINTF2("\treqType = %ld\n",(long) ((xOpenFontReq *)mp)->reqType);
		BPRINTF2("\tlength = %ld\n",(long) ((xOpenFontReq *)mp)->length);
		BPRINTF2("\tfid = %ld\n",((xOpenFontReq *)mp)->fid);
		BPRINTF2("\tnbytes = %ld\n",(long) ((xOpenFontReq *)mp)->nbytes);
		Show_String8(mp,sizeof(xOpenFontReq),((xOpenFontReq *)mp)->nbytes);
		break;
	case X_CloseFont:
		BPRINTF1("CloseFont:\n");
		BPRINTF2("\treqType = %ld\n",(long) ((xResourceReq *)mp)->reqType);
		BPRINTF2("\tlength = %ld\n",(long) ((xResourceReq *)mp)->length);
		BPRINTF2("\tid = %ld\n",((xResourceReq *)mp)->id);
		break;
	case X_QueryFont:
		BPRINTF1("QueryFont:\n");
		BPRINTF2("\treqType = %ld\n",(long) ((xResourceReq *)mp)->reqType);
		BPRINTF2("\tlength = %ld\n",(long) ((xResourceReq *)mp)->length);
		BPRINTF2("\tid = %ld\n",((xResourceReq *)mp)->id);
		break;
	case X_QueryTextExtents:
		BPRINTF1("QueryTextExtents:\n");
		BPRINTF2("\treqType = %ld\n",(long) ((xQueryTextExtentsReq *)mp)->reqType);
		BPRINTF2("\todd length, True if p = 2 = %d\n",((xQueryTextExtentsReq *)mp)->oddLength);
		BPRINTF2("\tlength = %ld\n",(long) ((xQueryTextExtentsReq *)mp)->length);
		BPRINTF2("\tfid = %ld\n",((xQueryTextExtentsReq *)mp)->fid);
		Show_Value_List_Req(mp,sizeof(xQueryTextExtentsReq),FORMAT16);
		break;
	case X_ListFonts:
		BPRINTF1("ListFonts:\n");
		BPRINTF2("\treqType = %ld\n",(long) ((xListFontsReq *)mp)->reqType);
		BPRINTF2("\tlength = %ld\n",(long) ((xListFontsReq *)mp)->length);
		BPRINTF2("\tmaxNames = %d\n",((xListFontsReq *)mp)->maxNames);
		BPRINTF2("\tnbytes = %ld\n",(long) ((xListFontsReq *)mp)->nbytes);
		Show_String8(mp,sizeof(xListFontsReq),((xListFontsReq *)mp)->nbytes);
		break;
	case X_ListFontsWithInfo:
		BPRINTF1("ListFontsWithInfo:\n");
		BPRINTF2("\treqType = %ld\n",(long) ((xListFontsWithInfoReq *)mp)->reqType);
		BPRINTF2("\tlength = %ld\n",(long) ((xListFontsWithInfoReq *)mp)->length);
		BPRINTF2("\tmaxNames = %d\n",((xListFontsWithInfoReq *)mp)->maxNames);
		BPRINTF2("\tnbytes = %ld\n",(long) ((xListFontsWithInfoReq *)mp)->nbytes);
		Show_String8(mp,sizeof(xListFontsWithInfoReq),((xListFontsWithInfoReq *)mp)->nbytes);
		break;
	}
	switch (mp->reqType) {
	case X_SetFontPath:
		BPRINTF1("SetFontPath:\n");
		BPRINTF2("\treqType = %ld\n",(long) ((xSetFontPathReq *)mp)->reqType);
		BPRINTF2("\tlength = %ld\n",(long) ((xSetFontPathReq *)mp)->length);
		BPRINTF2("\tnFonts = %d\n",((xSetFontPathReq *)mp)->nFonts);
		Show_Strs((unsigned char *)((unsigned char *)mp)+sizeof(xSetFontPathReq),((xSetFontPathReq *)mp)->nFonts,0/*unused*/,NULL/*unused*/);
		break;
	case X_GetFontPath:
		BPRINTF1("GetFontPath:\n");
		BPRINTF2("\treqType = %ld\n",(long) ((xResourceReq *)mp)->reqType);
		BPRINTF2("\tid = %ld\n",(long) ((xResourceReq *)mp)->id);
		break;
	case X_CreatePixmap:
		BPRINTF1("CreatePixmap:\n");
		BPRINTF2("\treqType = %ld\n",(long) ((xCreatePixmapReq *)mp)->reqType);
		BPRINTF2("\tdepth = %d\n",((xCreatePixmapReq *)mp)->depth);
		BPRINTF2("\tlength = %ld\n",(long) ((xCreatePixmapReq *)mp)->length);
		BPRINTF2("\tpid = %ld\n",((xCreatePixmapReq *)mp)->pid);
		BPRINTF2("\tdrawable = %ld\n",((xCreatePixmapReq *)mp)->drawable);
		BPRINTF2("\twidth = %d\n",((xCreatePixmapReq *)mp)->width);
		BPRINTF2("\theight = %d\n",((xCreatePixmapReq *)mp)->height);
		break;
	case X_FreePixmap:
		BPRINTF1("FreePixmap:\n");
		BPRINTF2("\treqType = %ld\n",(long) ((xResourceReq *)mp)->reqType);
		BPRINTF2("\tlength = %ld\n",(long) ((xResourceReq *)mp)->length);
		BPRINTF2("\tid = %ld\n",((xResourceReq *)mp)->id);
		break;
	case X_CreateGC:
		BPRINTF1("CreateGC:\n");
		BPRINTF2("\treqType = %ld\n",(long) ((xCreateGCReq *)mp)->reqType);
		BPRINTF2("\tlength = %ld\n",(long) ((xCreateGCReq *)mp)->length);
		BPRINTF2("\tgc = %ld\n",((xCreateGCReq *)mp)->gc);
		BPRINTF2("\tdrawable = %ld\n",((xCreateGCReq *)mp)->drawable);
		BPRINTF2("\tvalue-mask (has n 1-bits) = 0x%08x\n",((xCreateGCReq *)mp)->mask);
		Show_Value_List_Req(mp,sizeof(xCreateGCReq),FORMAT32);
		break;
	case X_ChangeGC:
		BPRINTF1("ChangeGC:\n");
		BPRINTF2("\treqType = %ld\n",(long) ((xChangeGCReq *)mp)->reqType);
		BPRINTF2("\tlength = %ld\n",(long) ((xChangeGCReq *)mp)->length);
		BPRINTF2("\tgc = %ld\n",((xChangeGCReq *)mp)->gc);
		BPRINTF2("\tvalue-mask (has n 1-bits) = 0x%08x\n",((xChangeGCReq *)mp)->mask);
		Show_Value_List_Req(mp,sizeof(xChangeGCReq),FORMAT32);
		break;
	case X_CopyGC:
		BPRINTF1("CopyGC:\n");
		BPRINTF2("\treqType = %ld\n",(long) ((xCopyGCReq *)mp)->reqType);
		BPRINTF2("\tlength = %ld\n",(long) ((xCopyGCReq *)mp)->length);
		BPRINTF2("\tsrcGC = %ld\n",((xCopyGCReq *)mp)->srcGC);
		BPRINTF2("\tdstGC = %ld\n",((xCopyGCReq *)mp)->dstGC);
		BPRINTF2("\tmask = 0x%08x\n",((xCopyGCReq *)mp)->mask);
		break;
	case X_SetDashes:
		BPRINTF1("SetDashes:\n");
		BPRINTF2("\treqType = %ld\n",(long) ((xSetDashesReq *)mp)->reqType);
		BPRINTF2("\tlength = %ld\n",(long) ((xSetDashesReq *)mp)->length);
		BPRINTF2("\tgc = %ld\n",((xSetDashesReq *)mp)->gc);
		BPRINTF2("\tdashOffset = %d\n",((xSetDashesReq *)mp)->dashOffset);
		BPRINTF2("\tnDashes = %ld\n",(long) ((xSetDashesReq *)mp)->nDashes);
		Show_Value_List_Req(mp,sizeof(xSetDashesReq),FORMAT8);
		break;
	case X_SetClipRectangles:
		BPRINTF1("SetClipRectangles:\n");
		BPRINTF2("\treqType = %ld\n",(long) ((xSetClipRectanglesReq *)mp)->reqType);
		BPRINTF2("\tordering = %ld\n",(long) ((xSetClipRectanglesReq *)mp)->ordering);
		BPRINTF2("\tlength = %ld\n",(long) ((xSetClipRectanglesReq *)mp)->length);
		BPRINTF2("\tgc = %ld\n",((xSetClipRectanglesReq *)mp)->gc);
		BPRINTF2("\txOrigin = %d\n",((xSetClipRectanglesReq *)mp)->xOrigin);
		BPRINTF2("\tyOrigin = %d\n",((xSetClipRectanglesReq *)mp)->yOrigin);
		Show_Value_List_Req(mp,sizeof(xSetClipRectanglesReq),FORMATrectangle);
		break;
	case X_FreeGC:
		BPRINTF1("FreeGC:\n");
		BPRINTF2("\treqType = %ld\n",(long) ((xResourceReq *)mp)->reqType);
		BPRINTF2("\tlength = %ld\n",(long) ((xResourceReq *)mp)->length);
		BPRINTF2("\tid = %ld\n",((xResourceReq *)mp)->id);
		break;
	case X_ClearArea:
		BPRINTF1("ClearArea:\n");
		BPRINTF2("\treqType = %ld\n",(long) ((xClearAreaReq *)mp)->reqType);
		BPRINTF2("\texposures = %d\n",((xClearAreaReq *)mp)->exposures);
		BPRINTF2("\tlength = %ld\n",(long) ((xClearAreaReq *)mp)->length);
		BPRINTF2("\twindow = %ld\n",((xClearAreaReq *)mp)->window);
		BPRINTF2("\tx = %d\n",((xClearAreaReq *)mp)->x);
		BPRINTF2("\ty = %d\n",((xClearAreaReq *)mp)->y);
		BPRINTF2("\twidth = %d\n",((xClearAreaReq *)mp)->width);
		BPRINTF2("\theight = %d\n",((xClearAreaReq *)mp)->height);
		break;
	case X_CopyArea:
		BPRINTF1("CopyArea:\n");
		BPRINTF2("\treqType = %ld\n",(long) ((xCopyAreaReq *)mp)->reqType);
		BPRINTF2("\tlength = %ld\n",(long) ((xCopyAreaReq *)mp)->length);
		BPRINTF2("\tsrcDrawable = %ld\n",((xCopyAreaReq *)mp)->srcDrawable);
		BPRINTF2("\tdstDrawable = %ld\n",((xCopyAreaReq *)mp)->dstDrawable);
		BPRINTF2("\tgc = %ld\n",((xCopyAreaReq *)mp)->gc);
		BPRINTF2("\tsrcX = %d\n",((xCopyAreaReq *)mp)->srcX);
		BPRINTF2("\tsrcY = %d\n",((xCopyAreaReq *)mp)->srcY);
		BPRINTF2("\tdstX = %d\n",((xCopyAreaReq *)mp)->dstX);
		BPRINTF2("\tdstY = %d\n",((xCopyAreaReq *)mp)->dstY);
		BPRINTF2("\twidth = %d\n",((xCopyAreaReq *)mp)->width);
		BPRINTF2("\theight = %d\n",((xCopyAreaReq *)mp)->height);
		break;
	case X_CopyPlane:
		BPRINTF1("CopyPlane:\n");
		BPRINTF2("\treqType = %ld\n",(long) ((xCopyPlaneReq *)mp)->reqType);
		BPRINTF2("\tlength = %ld\n",(long) ((xCopyPlaneReq *)mp)->length);
		BPRINTF2("\tsrcDrawable = %ld\n",((xCopyPlaneReq *)mp)->srcDrawable);
		BPRINTF2("\tdstDrawable = %ld\n",((xCopyPlaneReq *)mp)->dstDrawable);
		BPRINTF2("\tgc = %ld\n",((xCopyPlaneReq *)mp)->gc);
		BPRINTF2("\tsrcX = %d\n",((xCopyPlaneReq *)mp)->srcX);
		BPRINTF2("\tsrcY = %d\n",((xCopyPlaneReq *)mp)->srcY);
		BPRINTF2("\tdstX = %d\n",((xCopyPlaneReq *)mp)->dstX);
		BPRINTF2("\tdstY = %d\n",((xCopyPlaneReq *)mp)->dstY);
		BPRINTF2("\twidth = %d\n",((xCopyPlaneReq *)mp)->width);
		BPRINTF2("\theight = %d\n",((xCopyPlaneReq *)mp)->height);
		BPRINTF2("\tbitPlane = %ld\n",((xCopyPlaneReq *)mp)->bitPlane);
		break;
	case X_PolyPoint:
		BPRINTF1("PolyPoint:\n");
		BPRINTF2("\treqType = %ld\n",(long) ((xPolyPointReq *)mp)->reqType);
		BPRINTF2("\tcoordMode = %ld\n",(long) ((xPolyPointReq *)mp)->coordMode);
		BPRINTF2("\tlength = %ld\n",(long) ((xPolyPointReq *)mp)->length);
		BPRINTF2("\tdrawable = %ld\n",((xPolyPointReq *)mp)->drawable);
		BPRINTF2("\tgc = %ld\n",((xPolyPointReq *)mp)->gc);
		Show_Value_List_Req(mp,sizeof(xPolyPointReq),FORMATpoint);
		break;
	case X_PolyLine:
		BPRINTF1("PolyLine:\n");
		BPRINTF2("\treqType = %ld\n",(long) ((xPolyLineReq *)mp)->reqType);
		BPRINTF2("\tcoordMode = %ld\n",(long) ((xPolyLineReq *)mp)->coordMode);
		BPRINTF2("\tlength = %ld\n",(long) ((xPolyLineReq *)mp)->length);
		BPRINTF2("\tdrawable = %ld\n",((xPolyLineReq *)mp)->drawable);
		BPRINTF2("\tgc = %ld\n",((xPolyLineReq *)mp)->gc);
		Show_Value_List_Req(mp,sizeof(xPolyLineReq),FORMATpoint);
		break;
	case X_PolySegment:
		BPRINTF1("PolySegment:\n");
		BPRINTF2("\treqType = %ld\n",(long) ((xPolySegmentReq *)mp)->reqType);
		BPRINTF2("\tlength = %ld\n",(long) ((xPolySegmentReq *)mp)->length);
		BPRINTF2("\tdrawable = %ld\n",((xPolySegmentReq *)mp)->drawable);
		BPRINTF2("\tgc = %ld\n",((xPolySegmentReq *)mp)->gc);
		Show_Value_List_Req(mp,sizeof(xPolySegmentReq),FORMATpoint);
		break;
	case X_PolyRectangle:
		BPRINTF1("PolyRectangle:\n");
		BPRINTF2("\treqType = %ld\n",(long) ((xPolyRectangleReq *)mp)->reqType);
		BPRINTF2("\tlength = %ld\n",(long) ((xPolyRectangleReq *)mp)->length);
		BPRINTF2("\tdrawable = %ld\n",((xPolyRectangleReq *)mp)->drawable);
		BPRINTF2("\tgc = %ld\n",((xPolyRectangleReq *)mp)->gc);
		Show_Value_List_Req(mp,sizeof(xPolyRectangleReq),FORMATrectangle);
		break;
	case X_PolyArc:
		BPRINTF1("PolyArc:\n");
		BPRINTF2("\treqType = %ld\n",(long) ((xPolyArcReq *)mp)->reqType);
		BPRINTF2("\tlength = %ld\n",(long) ((xPolyArcReq *)mp)->length);
		BPRINTF2("\tdrawable = %ld\n",((xPolyArcReq *)mp)->drawable);
		BPRINTF2("\tgc = %ld\n",((xPolyArcReq *)mp)->gc);
		Show_Value_List_Req(mp,sizeof(xPolyArcReq),FORMATarc);
		break;
	case X_FillPoly:
		BPRINTF1("FillPoly:\n");
		BPRINTF2("\treqType = %ld\n",(long) ((xFillPolyReq *)mp)->reqType);
		BPRINTF2("\tlength = %ld\n",(long) ((xFillPolyReq *)mp)->length);
		BPRINTF2("\tdrawable = %ld\n",((xFillPolyReq *)mp)->drawable);
		BPRINTF2("\tgc = %ld\n",((xFillPolyReq *)mp)->gc);
		BPRINTF2("\tshape = %ld\n",(long) ((xFillPolyReq *)mp)->shape);
		BPRINTF2("\tcoordMode = %ld\n",(long) ((xFillPolyReq *)mp)->coordMode);
		Show_Value_List_Req(mp,sizeof(xFillPolyReq),FORMATpoint);
		break;
	case X_PolyFillRectangle:
		BPRINTF1("PolyFillRectangle:\n");
		BPRINTF2("\treqType = %ld\n",(long) ((xPolyFillRectangleReq *)mp)->reqType);
		BPRINTF2("\tlength = %ld\n",(long) ((xPolyFillRectangleReq *)mp)->length);
		BPRINTF2("\tdrawable = %ld\n",((xPolyFillRectangleReq *)mp)->drawable);
		BPRINTF2("\tgc = %ld\n",((xPolyFillRectangleReq *)mp)->gc);
		Show_Value_List_Req(mp,sizeof(xPolyFillRectangleReq),FORMATrectangle);
		break;
	case X_PolyFillArc:
		BPRINTF1("PolyFillArc:\n");
		BPRINTF2("\treqType = %ld\n",(long) ((xPolyFillArcReq *)mp)->reqType);
		BPRINTF2("\tlength = %ld\n",(long) ((xPolyFillArcReq *)mp)->length);
		BPRINTF2("\tdrawable = %ld\n",((xPolyFillArcReq *)mp)->drawable);
		BPRINTF2("\tgc = %ld\n",((xPolyFillArcReq *)mp)->gc);
		Show_Value_List_Req(mp,sizeof(xPolyFillArcReq),FORMATarc);
		break;
	case X_PutImage:
		BPRINTF1("PutImage:\n");
		BPRINTF2("\treqType = %ld\n",(long) ((xPutImageReq *)mp)->reqType);
		BPRINTF2("\tformat = %ld\n",(long) ((xPutImageReq *)mp)->format);
		BPRINTF2("\tlength = %ld\n",(long) ((xPutImageReq *)mp)->length);
		BPRINTF2("\tdrawable = %ld\n",((xPutImageReq *)mp)->drawable);
		BPRINTF2("\tgc = %ld\n",((xPutImageReq *)mp)->gc);
		BPRINTF2("\twidth = %d\n",((xPutImageReq *)mp)->width);
		BPRINTF2("\theight = %d\n",((xPutImageReq *)mp)->height);
		BPRINTF2("\tdstX = %d\n",((xPutImageReq *)mp)->dstX);
		BPRINTF2("\tdstY = %d\n",((xPutImageReq *)mp)->dstY);
		BPRINTF2("\tleftPad = %d\n",((xPutImageReq *)mp)->leftPad);
		BPRINTF2("\tdepth = %d\n",((xPutImageReq *)mp)->depth);
		Show_Value_List_Req(mp,sizeof(xPutImageReq),FORMAT8);
		break;
	}
	switch (mp->reqType) {
	case X_GetImage:
		BPRINTF1("GetImage:\n");
		BPRINTF2("\treqType = %ld\n",(long) ((xGetImageReq *)mp)->reqType);
		BPRINTF2("\tformat = %ld\n",(long) ((xGetImageReq *)mp)->format);
		BPRINTF2("\tlength = %ld\n",(long) ((xGetImageReq *)mp)->length);
		BPRINTF2("\tdrawable = %ld\n",((xGetImageReq *)mp)->drawable);
		BPRINTF2("\tx = %d\n",((xGetImageReq *)mp)->x);
		BPRINTF2("\ty = %d\n",((xGetImageReq *)mp)->y);
		BPRINTF2("\twidth = %d\n",((xGetImageReq *)mp)->width);
		BPRINTF2("\theight = %d\n",((xGetImageReq *)mp)->height);
		BPRINTF2("\tplaneMask = %ld\n",((xGetImageReq *)mp)->planeMask);
		break;
	case X_PolyText8:
		BPRINTF1("PolyText8:\n");
		BPRINTF2("\treqType = %ld\n",(long) ((xPolyText8Req *)mp)->reqType);
		BPRINTF2("\tlength = %ld\n",(long) ((xPolyText8Req *)mp)->length);
		BPRINTF2("\tdrawable = %ld\n",((xPolyText8Req *)mp)->drawable);
		BPRINTF2("\tgc = %ld\n",((xPolyText8Req *)mp)->gc);
		BPRINTF2("\tx = %d\n",((xPolyText8Req *)mp)->x);
		BPRINTF2("\ty = %d\n",((xPolyText8Req *)mp)->y);
		{ xTextElt *xte;

		    xte = (xTextElt *)(((char *)mp) + sizeof(xPolyText8Req));
		    if (xte->len == 255) {
			BPRINTF1("\tFont change....\n");
		    } else {
			BPRINTF2("\tfirst delta = %d\n", xte->delta);
			BPRINTF2("\t%d length string:\n\t", xte->len);
			Show_String8(mp,sizeof(xPolyText8Req)+sizeof(xTextElt),xte->len);
		    }
		}
		break;
	case X_PolyText16:
		BPRINTF1("PolyText16:\n");
		BPRINTF2("\treqType = %ld\n",(long) ((xPolyText16Req *)mp)->reqType);
		BPRINTF2("\tlength = %ld\n",(long) ((xPolyText16Req *)mp)->length);
		BPRINTF2("\tdrawable = %ld\n",((xPolyText16Req *)mp)->drawable);
		BPRINTF2("\tgc = %ld\n",((xPolyText16Req *)mp)->gc);
		BPRINTF2("\tx = %d\n",((xPolyText16Req *)mp)->x);
		BPRINTF2("\ty = %d\n",((xPolyText16Req *)mp)->y);
		{ xTextElt *xte;

		    xte = (xTextElt *)(((char *)mp) + sizeof(xPolyText8Req));
		    if (xte->len == 255) {
			BPRINTF1("\tFont change....\n");
		    } else {
			BPRINTF2("\tfirst delta = %d\n", xte->delta);
			BPRINTF2("\t%d length string:\n\t", xte->len);
			Show_String8(mp,sizeof(xPolyText8Req)+sizeof(xTextElt),xte->len);
		    }
		}
		break;
	case X_ImageText8:
		BPRINTF1("ImageText8:\n");
		BPRINTF2("\treqType = %ld\n",(long) ((xImageText8Req *)mp)->reqType);
		BPRINTF2("\tnChars = %ld\n",(long) ((xImageText8Req *)mp)->nChars);
		BPRINTF2("\tlength = %ld\n",(long) ((xImageText8Req *)mp)->length);
		BPRINTF2("\tdrawable = %ld\n",((xImageText8Req *)mp)->drawable);
		BPRINTF2("\tgc = %ld\n",((xImageText8Req *)mp)->gc);
		BPRINTF2("\tx = %d\n",((xImageText8Req *)mp)->x);
		BPRINTF2("\ty = %d\n",((xImageText8Req *)mp)->y);
		Show_String8(mp,sizeof(xImageText8Req),((xImageText8Req *)mp)->nChars);
		break;
	case X_ImageText16:
		BPRINTF1("ImageText16:\n");
		BPRINTF2("\treqType = %ld\n",(long) ((xImageText16Req *)mp)->reqType);
		BPRINTF2("\tnChars = %ld\n",(long) ((xImageText16Req *)mp)->nChars);
		BPRINTF2("\tlength = %ld\n",(long) ((xImageText16Req *)mp)->length);
		BPRINTF2("\tdrawable = %ld\n",((xImageText16Req *)mp)->drawable);
		BPRINTF2("\tgc = %ld\n",((xImageText16Req *)mp)->gc);
		BPRINTF2("\tx = %d\n",((xImageText16Req *)mp)->x);
		BPRINTF2("\ty = %d\n",((xImageText16Req *)mp)->y);
		Show_Value_List_Req(mp,sizeof(xImageText16Req),FORMAT16);
		break;
	case X_CreateColormap:
		BPRINTF1("CreateColormap:\n");
		BPRINTF2("\treqType = %ld\n",(long) ((xCreateColormapReq *)mp)->reqType);
		BPRINTF2("\talloc = %ld\n",(long) ((xCreateColormapReq *)mp)->alloc);
		BPRINTF2("\tlength = %ld\n",(long) ((xCreateColormapReq *)mp)->length);
		BPRINTF2("\tmid = %ld\n",((xCreateColormapReq *)mp)->mid);
		BPRINTF2("\twindow = %ld\n",((xCreateColormapReq *)mp)->window);
		BPRINTF2("\tvisual = %ld\n",((xCreateColormapReq *)mp)->visual);
		break;
	case X_FreeColormap:
		BPRINTF1("FreeColormap:\n");
		BPRINTF2("\treqType = %ld\n",(long) ((xResourceReq *)mp)->reqType);
		BPRINTF2("\tlength = %ld\n",(long) ((xResourceReq *)mp)->length);
		BPRINTF2("\tid = %ld\n",((xResourceReq *)mp)->id);
		break;
	case X_CopyColormapAndFree:
		BPRINTF1("CopyColormapAndFree:\n");
		BPRINTF2("\treqType = %ld\n",(long) ((xCopyColormapAndFreeReq *)mp)->reqType);
		BPRINTF2("\tlength = %ld\n",(long) ((xCopyColormapAndFreeReq *)mp)->length);
		BPRINTF2("\tmid = %ld\n",((xCopyColormapAndFreeReq *)mp)->mid);
		BPRINTF2("\tsrcCmap = %ld\n",((xCopyColormapAndFreeReq *)mp)->srcCmap);
		break;
	case X_InstallColormap:
		BPRINTF1("InstallColormap:\n");
		BPRINTF2("\treqType = %ld\n",(long) ((xResourceReq *)mp)->reqType);
		BPRINTF2("\tlength = %ld\n",(long) ((xResourceReq *)mp)->length);
		BPRINTF2("\tid = %ld\n",((xResourceReq *)mp)->id);
		break;
	case X_UninstallColormap:
		BPRINTF1("UninstallColormap:\n");
		BPRINTF2("\treqType = %ld\n",(long) ((xResourceReq *)mp)->reqType);
		BPRINTF2("\tlength = %ld\n",(long) ((xResourceReq *)mp)->length);
		BPRINTF2("\tid = %ld\n",((xResourceReq *)mp)->id);
		break;
	case X_ListInstalledColormaps:
		BPRINTF1("ListInstalledColormaps:\n");
		BPRINTF2("\treqType = %ld\n",(long) ((xResourceReq *)mp)->reqType);
		BPRINTF2("\tlength = %ld\n",(long) ((xResourceReq *)mp)->length);
		BPRINTF2("\tid = %ld\n",((xResourceReq *)mp)->id);
		break;
	case X_AllocColor:
		BPRINTF1("AllocColor:\n");
		BPRINTF2("\treqType = %ld\n",(long) ((xAllocColorReq *)mp)->reqType);
		BPRINTF2("\tlength = %ld\n",(long) ((xAllocColorReq *)mp)->length);
		BPRINTF2("\tcmap = %ld\n",((xAllocColorReq *)mp)->cmap);
		BPRINTF2("\tred = %d\n",((xAllocColorReq *)mp)->red);
		BPRINTF2("\tgreen = %d\n",((xAllocColorReq *)mp)->green);
		BPRINTF2("\tblue = %d\n",((xAllocColorReq *)mp)->blue);
		break;
	case X_AllocNamedColor:
		BPRINTF1("AllocNamedColor:\n");
		BPRINTF2("\treqType = %ld\n",(long) ((xAllocNamedColorReq *)mp)->reqType);
		BPRINTF2("\tlength = %ld\n",(long) ((xAllocNamedColorReq *)mp)->length);
		BPRINTF2("\tcmap = %ld\n",((xAllocNamedColorReq *)mp)->cmap);
		BPRINTF2("\tnbytes = %ld\n",(long) ((xAllocNamedColorReq *)mp)->nbytes);
		Show_String8(mp,sizeof(xAllocNamedColorReq),((xAllocNamedColorReq *)mp)->nbytes);
		break;
	case X_AllocColorCells:
		BPRINTF1("AllocColorCells:\n");
		BPRINTF2("\treqType = %ld\n",(long) ((xAllocColorCellsReq *)mp)->reqType);
		BPRINTF2("\tcontiguous = %d\n",((xAllocColorCellsReq *)mp)->contiguous);
		BPRINTF2("\tlength = %ld\n",(long) ((xAllocColorCellsReq *)mp)->length);
		BPRINTF2("\tcmap = %ld\n",((xAllocColorCellsReq *)mp)->cmap);
		BPRINTF2("\tcolors = %d\n",((xAllocColorCellsReq *)mp)->colors);
		BPRINTF2("\tplanes = %d\n",((xAllocColorCellsReq *)mp)->planes);
		break;
	case X_AllocColorPlanes:
		BPRINTF1("AllocColorPlanes:\n");
		BPRINTF2("\treqType = %ld\n",(long) ((xAllocColorPlanesReq *)mp)->reqType);
		BPRINTF2("\tcontiguous = %d\n",((xAllocColorPlanesReq *)mp)->contiguous);
		BPRINTF2("\tlength = %ld\n",(long) ((xAllocColorPlanesReq *)mp)->length);
		BPRINTF2("\tcmap = %ld\n",((xAllocColorPlanesReq *)mp)->cmap);
		BPRINTF2("\tcolors = %d\n",((xAllocColorPlanesReq *)mp)->colors);
		BPRINTF2("\tred = %d\n",((xAllocColorPlanesReq *)mp)->red);
		BPRINTF2("\tgreen = %d\n",((xAllocColorPlanesReq *)mp)->green);
		BPRINTF2("\tblue = %d\n",((xAllocColorPlanesReq *)mp)->blue);
		break;
	case X_FreeColors:
		BPRINTF1("FreeColors:\n");
		BPRINTF2("\treqType = %ld\n",(long) ((xFreeColorsReq *)mp)->reqType);
		BPRINTF2("\tlength = %ld\n",(long) ((xFreeColorsReq *)mp)->length);
		BPRINTF2("\tcmap = %ld\n",((xFreeColorsReq *)mp)->cmap);
		BPRINTF2("\tplaneMask = %ld\n",((xFreeColorsReq *)mp)->planeMask);
		Show_Value_List_Req(mp,sizeof(xFreeColorsReq),FORMAT32);
		break;
	case X_StoreColors:
		BPRINTF1("StoreColors:\n");
		BPRINTF2("\treqType = %ld\n",(long) ((xStoreColorsReq *)mp)->reqType);
		BPRINTF2("\tlength = %ld\n",(long) ((xStoreColorsReq *)mp)->length);
		BPRINTF2("\tcmap = %ld\n",((xStoreColorsReq *)mp)->cmap);
		Show_Value_List_Req(mp,sizeof(xStoreColorsReq),FORMATcoloritem);
		break;
	case X_StoreNamedColor:
		BPRINTF1("StoreNamedColor:\n");
		BPRINTF2("\treqType = %ld\n",(long) ((xStoreNamedColorReq *)mp)->reqType);
		BPRINTF2("\tdo-red, do-green, do-blue = %ld\n",(long) ((xStoreNamedColorReq *)mp)->flags);
		BPRINTF2("\tlength = %ld\n",(long) ((xStoreNamedColorReq *)mp)->length);
		BPRINTF2("\tcmap = %ld\n",((xStoreNamedColorReq *)mp)->cmap);
		BPRINTF2("\tpixel = %ld\n",((xStoreNamedColorReq *)mp)->pixel);
		BPRINTF2("\tnbytes = %ld\n",(long) ((xStoreNamedColorReq *)mp)->nbytes);
		Show_String8(mp,sizeof(xStoreNamedColorReq),((xStoreNamedColorReq *)mp)->nbytes);
		break;
	case X_QueryColors:
		BPRINTF1("QueryColors:\n");
		BPRINTF2("\treqType = %ld\n",(long) ((xQueryColorsReq *)mp)->reqType);
		BPRINTF2("\tlength = %ld\n",(long) ((xQueryColorsReq *)mp)->length);
		BPRINTF2("\tcmap = %ld\n",((xQueryColorsReq *)mp)->cmap);
		Show_Value_List_Req(mp,sizeof(xQueryColorsReq),FORMAT32);
		break;
	case X_LookupColor:
		BPRINTF1("LookupColor:\n");
		BPRINTF2("\treqType = %ld\n",(long) ((xLookupColorReq *)mp)->reqType);
		BPRINTF2("\tlength = %ld\n",(long) ((xLookupColorReq *)mp)->length);
		BPRINTF2("\tcmap = %ld\n",((xLookupColorReq *)mp)->cmap);
		BPRINTF2("\tnbytes = %ld\n",(long) ((xLookupColorReq *)mp)->nbytes);
		Show_String8(mp,sizeof(xLookupColorReq),((xLookupColorReq *)mp)->nbytes);
		break;
	}
	switch (mp->reqType) {
	case X_CreateCursor:
		BPRINTF1("CreateCursor:\n");
		BPRINTF2("\treqType = %ld\n",(long) ((xCreateCursorReq *)mp)->reqType);
		BPRINTF2("\tlength = %ld\n",(long) ((xCreateCursorReq *)mp)->length);
		BPRINTF2("\tcid = %ld\n",((xCreateCursorReq *)mp)->cid);
		BPRINTF2("\tsource = %ld\n",((xCreateCursorReq *)mp)->source);
		BPRINTF2("\tmask = %ld\n",((xCreateCursorReq *)mp)->mask);
		BPRINTF2("\tforeRed = %d\n",((xCreateCursorReq *)mp)->foreRed);
		BPRINTF2("\tforeGreen = %d\n",((xCreateCursorReq *)mp)->foreGreen);
		BPRINTF2("\tforeBlue = %d\n",((xCreateCursorReq *)mp)->foreBlue);
		BPRINTF2("\tbackRed = %d\n",((xCreateCursorReq *)mp)->backRed);
		BPRINTF2("\tbackGreen = %d\n",((xCreateCursorReq *)mp)->backGreen);
		BPRINTF2("\tbackBlue = %d\n",((xCreateCursorReq *)mp)->backBlue);
		BPRINTF2("\tx = %d\n",((xCreateCursorReq *)mp)->x);
		BPRINTF2("\ty = %d\n",((xCreateCursorReq *)mp)->y);
		break;
	case X_CreateGlyphCursor:
		BPRINTF1("CreateGlyphCursor:\n");
		BPRINTF2("\treqType = %ld\n",(long) ((xCreateGlyphCursorReq *)mp)->reqType);
		BPRINTF2("\tlength = %ld\n",(long) ((xCreateGlyphCursorReq *)mp)->length);
		BPRINTF2("\tcid = %ld\n",((xCreateGlyphCursorReq *)mp)->cid);
		BPRINTF2("\tsource = %ld\n",((xCreateGlyphCursorReq *)mp)->source);
		BPRINTF2("\tmask = %ld\n",((xCreateGlyphCursorReq *)mp)->mask);
		BPRINTF2("\tsourceChar = %d\n",((xCreateGlyphCursorReq *)mp)->sourceChar);
		BPRINTF2("\tmaskChar = %d\n",((xCreateGlyphCursorReq *)mp)->maskChar);
		BPRINTF2("\tforeRed = %d\n",((xCreateGlyphCursorReq *)mp)->foreRed);
		BPRINTF2("\tforeGreen = %d\n",((xCreateGlyphCursorReq *)mp)->foreGreen);
		BPRINTF2("\tforeBlue = %d\n",((xCreateGlyphCursorReq *)mp)->foreBlue);
		BPRINTF2("\tbackRed = %d\n",((xCreateGlyphCursorReq *)mp)->backRed);
		BPRINTF2("\tbackGreen = %d\n",((xCreateGlyphCursorReq *)mp)->backGreen);
		BPRINTF2("\tbackBlue = %d\n",((xCreateGlyphCursorReq *)mp)->backBlue);
		break;
	case X_FreeCursor:
		BPRINTF1("FreeCursor:\n");
		BPRINTF2("\treqType = %ld\n",(long) ((xResourceReq *)mp)->reqType);
		BPRINTF2("\tlength = %ld\n",(long) ((xResourceReq *)mp)->length);
		BPRINTF2("\tid = %ld\n",((xResourceReq *)mp)->id);
		break;
	case X_RecolorCursor:
		BPRINTF1("RecolorCursor:\n");
		BPRINTF2("\treqType = %ld\n",(long) ((xRecolorCursorReq *)mp)->reqType);
		BPRINTF2("\tlength = %ld\n",(long) ((xRecolorCursorReq *)mp)->length);
		BPRINTF2("\tcursor = %ld\n",((xRecolorCursorReq *)mp)->cursor);
		BPRINTF2("\tforeRed = %d\n",((xRecolorCursorReq *)mp)->foreRed);
		BPRINTF2("\tforeGreen = %d\n",((xRecolorCursorReq *)mp)->foreGreen);
		BPRINTF2("\tforeBlue = %d\n",((xRecolorCursorReq *)mp)->foreBlue);
		BPRINTF2("\tbackRed = %d\n",((xRecolorCursorReq *)mp)->backRed);
		BPRINTF2("\tbackGreen = %d\n",((xRecolorCursorReq *)mp)->backGreen);
		BPRINTF2("\tbackBlue = %d\n",((xRecolorCursorReq *)mp)->backBlue);
		break;
	case X_QueryBestSize:
		BPRINTF1("QueryBestSize:\n");
		BPRINTF2("\treqType = %ld\n",(long) ((xQueryBestSizeReq *)mp)->reqType);
		BPRINTF2("\tclass = %ld\n",(long) ((xQueryBestSizeReq *)mp)->class);
		BPRINTF2("\tlength = %ld\n",(long) ((xQueryBestSizeReq *)mp)->length);
		BPRINTF2("\tdrawable = %ld\n",((xQueryBestSizeReq *)mp)->drawable);
		BPRINTF2("\twidth = %d\n",((xQueryBestSizeReq *)mp)->width);
		BPRINTF2("\theight = %d\n",((xQueryBestSizeReq *)mp)->height);
		break;
	case X_QueryExtension:
		BPRINTF1("QueryExtension:\n");
		BPRINTF2("\treqType = %ld\n",(long) ((xQueryExtensionReq *)mp)->reqType);
		BPRINTF2("\tlength = %ld\n",(long) ((xQueryExtensionReq *)mp)->length);
		BPRINTF2("\tnbytes = %ld\n",(long) ((xQueryExtensionReq *)mp)->nbytes);
		Show_String8(mp,sizeof(xQueryExtensionReq),((xQueryExtensionReq *)mp)->nbytes);
		break;
	case X_ListExtensions:
		BPRINTF1("ListExtensions:\n");
		BPRINTF2("\treqType = %ld\n",(long) ((xResourceReq *)mp)->reqType);
		BPRINTF2("\tlength = %ld\n",(long) ((xResourceReq *)mp)->length);
		break;
	case X_ChangeKeyboardMapping:
		BPRINTF1("ChangeKeyboardMapping:\n");
		BPRINTF2("\treqType = %ld\n",(long) ((xChangeKeyboardMappingReq *)mp)->reqType);
		BPRINTF2("\tkeyCodes = %ld\n",(long) ((xChangeKeyboardMappingReq *)mp)->keyCodes);
		BPRINTF2("\tlength = %ld\n",(long) ((xChangeKeyboardMappingReq *)mp)->length);
		BPRINTF2("\tfirstKeyCode = %d\n",((xChangeKeyboardMappingReq *)mp)->firstKeyCode);
		BPRINTF2("\tkeySymsPerKeyCode = %ld\n",(long) ((xChangeKeyboardMappingReq *)mp)->keySymsPerKeyCode);
		Show_Value_List_Req(mp,sizeof(xChangeKeyboardMappingReq),FORMAT32);
		break;
	case X_GetKeyboardMapping:
		BPRINTF1("GetKeyboardMapping:\n");
		BPRINTF2("\treqType = %ld\n",(long) ((xGetKeyboardMappingReq *)mp)->reqType);
		BPRINTF2("\tlength = %ld\n",(long) ((xGetKeyboardMappingReq *)mp)->length);
		BPRINTF2("\tfirstKeyCode = %d\n",((xGetKeyboardMappingReq *)mp)->firstKeyCode);
		BPRINTF2("\tcount = %d\n",((xGetKeyboardMappingReq *)mp)->count);
		break;
	case X_ChangeKeyboardControl:
		BPRINTF1("ChangeKeyboardControl:\n");
		BPRINTF2("\treqType = %ld\n",(long) ((xChangeKeyboardControlReq *)mp)->reqType);
		BPRINTF2("\tlength = %ld\n",(long) ((xChangeKeyboardControlReq *)mp)->length);
		BPRINTF2("\tvalue-mask (has n 1-bits) = 0x%08x\n",((xChangeKeyboardControlReq *)mp)->mask);
		Show_Value_List_Req(mp,sizeof(xChangeKeyboardControlReq),FORMAT32);
		break;
	case X_GetKeyboardControl:
		BPRINTF1("GetKeyboardControl:\n");
		BPRINTF2("\treqType = %ld\n",(long) ((xResourceReq *)mp)->reqType);
		BPRINTF2("\tlength = %ld\n",(long) ((xResourceReq *)mp)->length);
		break;
	case X_Bell:
		BPRINTF1("Bell:\n");
		BPRINTF2("\treqType = %ld\n",(long) ((xBellReq *)mp)->reqType);
		BPRINTF2("\tpercent = %d\n",((xBellReq *)mp)->percent);
		BPRINTF2("\tlength = %ld\n",(long) ((xBellReq *)mp)->length);
		break;
	case X_ChangePointerControl:
		BPRINTF1("ChangePointerControl:\n");
		BPRINTF2("\treqType = %ld\n",(long) ((xChangePointerControlReq *)mp)->reqType);
		BPRINTF2("\tlength = %ld\n",(long) ((xChangePointerControlReq *)mp)->length);
		BPRINTF2("\taccelNum = %d\n",((xChangePointerControlReq *)mp)->accelNum);
		BPRINTF2("\taccelDenum = %d\n",((xChangePointerControlReq *)mp)->accelDenum);
		BPRINTF2("\tthreshold = %d\n",((xChangePointerControlReq *)mp)->threshold);
		BPRINTF2("\tdoAccel = %d\n",((xChangePointerControlReq *)mp)->doAccel);
		BPRINTF2("\tdoThresh = %d\n",((xChangePointerControlReq *)mp)->doThresh);
		break;
	case X_GetPointerControl:
		BPRINTF1("GetPointerControl:\n");
		BPRINTF2("\treqType = %ld\n",(long) ((xResourceReq *)mp)->reqType);
		BPRINTF2("\tlength = %ld\n",(long) ((xResourceReq *)mp)->length);
		break;
	case X_SetScreenSaver:
		BPRINTF1("SetScreenSaver:\n");
		BPRINTF2("\treqType = %ld\n",(long) ((xSetScreenSaverReq *)mp)->reqType);
		BPRINTF2("\tlength = %ld\n",(long) ((xSetScreenSaverReq *)mp)->length);
		BPRINTF2("\ttimeout = %d\n",((xSetScreenSaverReq *)mp)->timeout);
		BPRINTF2("\tinterval = %d\n",((xSetScreenSaverReq *)mp)->interval);
		BPRINTF2("\tpreferBlank = %ld\n",(long) ((xSetScreenSaverReq *)mp)->preferBlank);
		BPRINTF2("\tallowExpose = %ld\n",(long) ((xSetScreenSaverReq *)mp)->allowExpose);
		break;
	case X_GetScreenSaver:
		BPRINTF1("GetScreenSaver:\n");
		BPRINTF2("\treqType = %ld\n",(long) ((xResourceReq *)mp)->reqType);
		BPRINTF2("\tlength = %ld\n",(long) ((xResourceReq *)mp)->length);
		break;
	case X_ChangeHosts:
		BPRINTF1("ChangeHosts:\n");
		BPRINTF2("\treqType = %ld\n",(long) ((xChangeHostsReq *)mp)->reqType);
		BPRINTF2("\tmode = %ld\n",(long) ((xChangeHostsReq *)mp)->mode);
		BPRINTF2("\tlength = %ld\n",(long) ((xChangeHostsReq *)mp)->length);
		BPRINTF2("\thostFamily = %ld\n",(long) ((xChangeHostsReq *)mp)->hostFamily);
		BPRINTF2("\thostLength = %d\n",((xChangeHostsReq *)mp)->hostLength);
		Show_String8(mp,sizeof(xChangeHostsReq),((xChangeHostsReq *)mp)->hostLength);
		break;
	case X_ListHosts:
		BPRINTF1("ListHosts:\n");
		BPRINTF2("\treqType = %ld\n",(long) ((xListHostsReq *)mp)->reqType);
		BPRINTF2("\tlength = %ld\n",(long) ((xListHostsReq *)mp)->length);
		break;
	case X_SetAccessControl:
		BPRINTF1("SetAccessControl:\n");
		BPRINTF2("\treqType = %ld\n",(long) ((xSetAccessControlReq *)mp)->reqType);
		BPRINTF2("\tmode = %ld\n",(long) ((xSetAccessControlReq *)mp)->mode);
		BPRINTF2("\tlength = %ld\n",(long) ((xSetAccessControlReq *)mp)->length);
		break;
	case X_SetCloseDownMode:
		BPRINTF1("SetCloseDownMode:\n");
		BPRINTF2("\treqType = %ld\n",(long) ((xSetCloseDownModeReq *)mp)->reqType);
		BPRINTF2("\tmode = %ld\n",(long) ((xSetCloseDownModeReq *)mp)->mode);
		BPRINTF2("\tlength = %ld\n",(long) ((xSetCloseDownModeReq *)mp)->length);
		break;
	case X_KillClient:
		BPRINTF1("KillClient:\n");
		BPRINTF2("\treqType = %ld\n",(long) ((xResourceReq *)mp)->reqType);
		BPRINTF2("\tlength = %ld\n",(long) ((xResourceReq *)mp)->length);
		BPRINTF2("\tid = %ld\n",((xResourceReq *)mp)->id);
		break;
	case X_RotateProperties:
		BPRINTF1("RotateProperties:\n");
		BPRINTF2("\treqType = %ld\n",(long) ((xRotatePropertiesReq *)mp)->reqType);
		BPRINTF2("\tlength = %ld\n",(long) ((xRotatePropertiesReq *)mp)->length);
		BPRINTF2("\twindow = %ld\n",((xRotatePropertiesReq *)mp)->window);
		BPRINTF2("\tnAtoms = %ld\n",(long) ((xRotatePropertiesReq *)mp)->nAtoms);
		BPRINTF2("\tnPositions = %d\n",((xRotatePropertiesReq *)mp)->nPositions);
		Show_Value_List_Req(mp,sizeof(xRotatePropertiesReq),FORMAT32);
		break;
	case X_ForceScreenSaver:
		BPRINTF1("ForceScreenSaver:\n");
		BPRINTF2("\treqType = %ld\n",(long) ((xForceScreenSaverReq *)mp)->reqType);
		BPRINTF2("\tmode = %ld\n",(long) ((xForceScreenSaverReq *)mp)->mode);
		BPRINTF2("\tlength = %ld\n",(long) ((xForceScreenSaverReq *)mp)->length);
		break;
	case X_SetPointerMapping:
		BPRINTF1("SetPointerMapping:\n");
		BPRINTF2("\treqType = %ld\n",(long) ((xSetPointerMappingReq *)mp)->reqType);
		BPRINTF2("\tnElts = %ld\n",(long) ((xSetPointerMappingReq *)mp)->nElts);
		BPRINTF2("\tlength = %ld\n",(long) ((xSetPointerMappingReq *)mp)->length);
		Show_Value_List_Req(mp,sizeof(xSetPointerMappingReq),FORMAT8);
		break;
	case X_GetPointerMapping:
		BPRINTF1("GetPointerMapping:\n");
		BPRINTF2("\treqType = %ld\n",(long) ((xResourceReq *)mp)->reqType);
		BPRINTF2("\tlength = %ld\n",(long) ((xResourceReq *)mp)->length);
		break;
	case X_SetModifierMapping:
		BPRINTF1("SetModifierMapping:\n");
		BPRINTF2("\treqType = %ld\n",(long) ((xSetModifierMappingReq *)mp)->reqType);
		BPRINTF2("\tnumKeyPerModifier = %ld\n",(long) ((xSetModifierMappingReq *)mp)->numKeyPerModifier);
		BPRINTF2("\tlength = %ld\n",(long) ((xSetModifierMappingReq *)mp)->length);
		Show_Value_List_Req(mp,sizeof(xSetModifierMappingReq),FORMAT8);
		break;
	case X_GetModifierMapping:
		BPRINTF1("GetModifierMapping:\n");
		BPRINTF2("\treqType = %ld\n",(long) ((xResourceReq *)mp)->reqType);
		BPRINTF2("\tlength = %ld\n",(long) ((xResourceReq *)mp)->length);
		break;
	case X_NoOperation:
		BPRINTF1("NoOperation:\n");
		BPRINTF2("\treqType = %ld\n",(long) ((xResourceReq *)mp)->reqType);
		BPRINTF2("\tlength = %ld\n",(long) ((xResourceReq *)mp)->length);
		break;
	}
}

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
 * $XConsortium: SendEvt.c,v 1.3 94/04/17 21:01:22 rws Exp $
 */
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

#include "XstlibInt.h"
#include "DataMove.h"

/*
 *	Routine: Send_Evt
 *
 *	Input: client - integer from 0 to MAX_CLIENTS representing which 
 *             client this is
 *             rp - pointer to xlib style request structure
 *             size - size of request without event (assumes event is at end)
 *             evt - event type as defined in xlib (e.g. KeyPress)
 *
 *	Output: packs event into request output buffer
 *
 *	Returns:
 *
 *	Globals used: 
 *
 *	Side Effects: 
 *
 *	Methods:
 *
 */

/*
 *     NOTE: this code has only been tested down the ClientMessage leg
 */    

Send_Evt(client,event_ptr,event_type)
int client;
xEvent *event_ptr;
int event_type;
{
	switch (event_type) {
	case KeyPress:
		send1(client,(long) event_ptr->u.u.type);
 		send1(client,(long) event_ptr->u.u.detail);
		send2(client,(long) event_ptr->u.u.sequenceNumber);
		send4(client,(long) event_ptr->u.keyButtonPointer.time);
		send4(client,(long) event_ptr->u.keyButtonPointer.root);
		send4(client,(long) event_ptr->u.keyButtonPointer.event);
		send4(client,(long) event_ptr->u.keyButtonPointer.child);
		send2(client,(long) event_ptr->u.keyButtonPointer.rootX);
		send2(client,(long) event_ptr->u.keyButtonPointer.rootY);
		send2(client,(long) event_ptr->u.keyButtonPointer.eventX);
		send2(client,(long) event_ptr->u.keyButtonPointer.eventY);
		send2(client,(long) event_ptr->u.keyButtonPointer.state);
		send1(client,(long) event_ptr->u.keyButtonPointer.sameScreen);
		sendpad(client,1);
		break;
	case KeyRelease:
		send1(client,(long) event_ptr->u.u.type);
		send1(client,(long) event_ptr->u.u.detail);
		send2(client,(long) event_ptr->u.u.sequenceNumber);
		send4(client,(long) event_ptr->u.keyButtonPointer.time);
		send4(client,(long) event_ptr->u.keyButtonPointer.root);
		send4(client,(long) event_ptr->u.keyButtonPointer.event);
		send4(client,(long) event_ptr->u.keyButtonPointer.child);
		send2(client,(long) event_ptr->u.keyButtonPointer.rootX);
		send2(client,(long) event_ptr->u.keyButtonPointer.rootY);
		send2(client,(long) event_ptr->u.keyButtonPointer.eventX);
		send2(client,(long) event_ptr->u.keyButtonPointer.eventY);
		send2(client,(long) event_ptr->u.keyButtonPointer.state);
		send1(client,(long) event_ptr->u.keyButtonPointer.sameScreen);
		sendpad(client,1);
		break;
	case ButtonPress:
		send1(client,(long) event_ptr->u.u.type);
		send1(client,(long) event_ptr->u.u.detail);
		send2(client,(long) event_ptr->u.u.sequenceNumber);
		send4(client,(long) event_ptr->u.keyButtonPointer.time);
		send4(client,(long) event_ptr->u.keyButtonPointer.root);
		send4(client,(long) event_ptr->u.keyButtonPointer.event);
		send4(client,(long) event_ptr->u.keyButtonPointer.child);
		send2(client,(long) event_ptr->u.keyButtonPointer.rootX);
		send2(client,(long) event_ptr->u.keyButtonPointer.rootY);
		send2(client,(long) event_ptr->u.keyButtonPointer.eventX);
		send2(client,(long) event_ptr->u.keyButtonPointer.eventY);
		send2(client,(long) event_ptr->u.keyButtonPointer.state);
		send1(client,(long) event_ptr->u.keyButtonPointer.sameScreen);
		sendpad(client,1);
		break;
	case ButtonRelease:
		send1(client,(long) event_ptr->u.u.type);
		send1(client,(long) event_ptr->u.u.detail);
		send2(client,(long) event_ptr->u.u.sequenceNumber);
		send4(client,(long) event_ptr->u.keyButtonPointer.time);
		send4(client,(long) event_ptr->u.keyButtonPointer.root);
		send4(client,(long) event_ptr->u.keyButtonPointer.event);
		send4(client,(long) event_ptr->u.keyButtonPointer.child);
		send2(client,(long) event_ptr->u.keyButtonPointer.rootX);
		send2(client,(long) event_ptr->u.keyButtonPointer.rootY);
		send2(client,(long) event_ptr->u.keyButtonPointer.eventX);
		send2(client,(long) event_ptr->u.keyButtonPointer.eventY);
		send2(client,(long) event_ptr->u.keyButtonPointer.state);
		send1(client,(long) event_ptr->u.keyButtonPointer.sameScreen);
		sendpad(client,1);
		break;
	case MotionNotify:
		send1(client,(long) event_ptr->u.u.type);
		send1(client,(long) event_ptr->u.u.detail);
		send2(client,(long) event_ptr->u.u.sequenceNumber);
		send4(client,(long) event_ptr->u.keyButtonPointer.time);
		send4(client,(long) event_ptr->u.keyButtonPointer.root);
		send4(client,(long) event_ptr->u.keyButtonPointer.event);
		send4(client,(long) event_ptr->u.keyButtonPointer.child);
		send2(client,(long) event_ptr->u.keyButtonPointer.rootX);
		send2(client,(long) event_ptr->u.keyButtonPointer.rootY);
		send2(client,(long) event_ptr->u.keyButtonPointer.eventX);
		send2(client,(long) event_ptr->u.keyButtonPointer.eventY);
		send2(client,(long) event_ptr->u.keyButtonPointer.state);
		send1(client,(long) event_ptr->u.keyButtonPointer.sameScreen);
		sendpad(client,1);
		break;
	case EnterNotify:
		send1(client,(long) event_ptr->u.u.type);
		send1(client,(long) event_ptr->u.u.detail);
		send2(client,(long) event_ptr->u.u.sequenceNumber);
		send4(client,(long) event_ptr->u.enterLeave.time);
		send4(client,(long) event_ptr->u.enterLeave.root);
		send4(client,(long) event_ptr->u.enterLeave.event);
		send4(client,(long) event_ptr->u.enterLeave.child);
		send2(client,(long) event_ptr->u.enterLeave.rootX);
		send2(client,(long) event_ptr->u.enterLeave.rootY);
		send2(client,(long) event_ptr->u.enterLeave.eventX);
		send2(client,(long) event_ptr->u.enterLeave.eventY);
		send2(client,(long) event_ptr->u.enterLeave.state);
		send1(client,(long) event_ptr->u.enterLeave.mode);
		send1(client,(long) event_ptr->u.enterLeave.flags);
		break;
	case LeaveNotify:
		send1(client,(long) event_ptr->u.u.type);
		send1(client,(long) event_ptr->u.u.detail);
		send2(client,(long) event_ptr->u.u.sequenceNumber);
		send4(client,(long) event_ptr->u.enterLeave.time);
		send4(client,(long) event_ptr->u.enterLeave.root);
		send4(client,(long) event_ptr->u.enterLeave.event);
		send4(client,(long) event_ptr->u.enterLeave.child);
		send2(client,(long) event_ptr->u.enterLeave.rootX);
		send2(client,(long) event_ptr->u.enterLeave.rootY);
		send2(client,(long) event_ptr->u.enterLeave.eventX);
		send2(client,(long) event_ptr->u.enterLeave.eventY);
		send2(client,(long) event_ptr->u.enterLeave.state);
		send1(client,(long) event_ptr->u.enterLeave.mode);
		send1(client,(long) event_ptr->u.enterLeave.flags);
		break;
	case FocusIn:
		send1(client,(long) event_ptr->u.u.type);
		send1(client,(long) event_ptr->u.u.detail);
		send2(client,(long) event_ptr->u.u.sequenceNumber);
		send4(client,(long) event_ptr->u.focus.window);
		send1(client,(long) event_ptr->u.focus.mode);
		sendpad(client,23);
		break;
	case FocusOut:
		send1(client,(long) event_ptr->u.u.type);
		send1(client,(long) event_ptr->u.u.detail);
		send2(client,(long) event_ptr->u.u.sequenceNumber);
		send4(client,(long) event_ptr->u.focus.window);
		send1(client,(long) event_ptr->u.focus.mode);
		sendpad(client,23);
		break;
	case KeymapNotify:
		send1(client,(long)((xKeymapEvent *) event_ptr)->type);
		/* 
		   call to sendbytes is commented out to let this file compile;
		   sendbytes is not yet implemented.

		 sendbytes(client,((xKeymapEvent *)event_ptr)->map);
		 */
		break;
	case Expose:
		send1(client,(long) event_ptr->u.u.type);
		sendpad(client,1);
		send2(client,(long) event_ptr->u.u.sequenceNumber);
		send4(client,(long) event_ptr->u.expose.window);
		send2(client,(long) event_ptr->u.expose.x);
		send2(client,(long) event_ptr->u.expose.y);
		send2(client,(long) event_ptr->u.expose.width);
		send2(client,(long) event_ptr->u.expose.height);
		send2(client,(long) event_ptr->u.expose.count);
		sendpad(client,14);
		break;
	case GraphicsExpose:
		send1(client,(long) event_ptr->u.u.type);
		sendpad(client,1);
		send2(client,(long) event_ptr->u.u.sequenceNumber);
		send4(client,(long) event_ptr->u.graphicsExposure.drawable);
		send2(client,(long) event_ptr->u.graphicsExposure.x);
		send2(client,(long) event_ptr->u.graphicsExposure.y);
		send2(client,(long) event_ptr->u.graphicsExposure.width);
		send2(client,(long) event_ptr->u.graphicsExposure.height);
		send2(client,(long) event_ptr->u.graphicsExposure.minorEvent);
		send2(client,(long) event_ptr->u.graphicsExposure.count);
		send1(client,(long) event_ptr->u.graphicsExposure.majorEvent);
		sendpad(client,11);
		break;
	case NoExpose:
		send1(client,(long) event_ptr->u.u.type);
		sendpad(client,1);
		send2(client,(long) event_ptr->u.u.sequenceNumber);
		send4(client,(long) event_ptr->u.noExposure.drawable);
		send2(client,(long) event_ptr->u.noExposure.minorEvent);
		send1(client,(long) event_ptr->u.noExposure.majorEvent);
		sendpad(client,21);
		break;
	case VisibilityNotify:
		send1(client,(long) event_ptr->u.u.type);
		sendpad(client,1);
		send2(client,(long) event_ptr->u.u.sequenceNumber);
		send4(client,(long) event_ptr->u.visibility.window);
		send1(client,(long) event_ptr->u.visibility.state);
		sendpad(client,23);
		break;
	case CreateNotify:
		send1(client,(long) event_ptr->u.u.type);
		sendpad(client,1);
		send2(client,(long) event_ptr->u.u.sequenceNumber);
		send4(client,(long) event_ptr->u.createNotify.parent);
		send4(client,(long) event_ptr->u.createNotify.window);
		send2(client,(long) event_ptr->u.createNotify.x);
		send2(client,(long) event_ptr->u.createNotify.y);
		send2(client,(long) event_ptr->u.createNotify.width);
		send2(client,(long) event_ptr->u.createNotify.height);
		send2(client,(long) event_ptr->u.createNotify.borderWidth);
		send1(client,(long) event_ptr->u.createNotify.override);
		sendpad(client,9);
		break;
	case DestroyNotify:
		send1(client,(long) event_ptr->u.u.type);
		sendpad(client,1);
		send2(client,(long) event_ptr->u.u.sequenceNumber);
		send4(client,(long) event_ptr->u.destroyNotify.event);
		send4(client,(long) event_ptr->u.destroyNotify.window);
		sendpad(client,20);
		break;
	case UnmapNotify:
		send1(client,(long) event_ptr->u.u.type);
		sendpad(client,1);
		send2(client,(long) event_ptr->u.u.sequenceNumber);
		send4(client,(long) event_ptr->u.unmapNotify.event);
		send4(client,(long) event_ptr->u.unmapNotify.window);
		send1(client,(long) event_ptr->u.unmapNotify.fromConfigure);
		sendpad(client,19);
		break;
	case MapNotify:
		send1(client,(long) event_ptr->u.u.type);
		sendpad(client,1);
		send2(client,(long) event_ptr->u.u.sequenceNumber);
		send4(client,(long) event_ptr->u.mapNotify.event);
		send4(client,(long) event_ptr->u.mapNotify.window);
		send1(client,(long) event_ptr->u.mapNotify.override);
		sendpad(client,19);
		break;
	case MapRequest:
		send1(client,(long) event_ptr->u.u.type);
		sendpad(client,1);
		send2(client,(long) event_ptr->u.u.sequenceNumber);
		send4(client,(long) event_ptr->u.mapRequest.parent);
		send4(client,(long) event_ptr->u.mapRequest.window);
		sendpad(client,20);
		break;
	case ReparentNotify:
		send1(client,(long) event_ptr->u.u.type);
		sendpad(client,1);
		send2(client,(long) event_ptr->u.u.sequenceNumber);
		send4(client,(long) event_ptr->u.reparent.event);
		send4(client,(long) event_ptr->u.reparent.window);
		send4(client,(long) event_ptr->u.reparent.parent);
		send2(client,(long) event_ptr->u.reparent.x);
		send2(client,(long) event_ptr->u.reparent.y);
		send1(client,(long) event_ptr->u.reparent.override);
		sendpad(client,11);
		break;
	case ConfigureNotify:
		send1(client,(long) event_ptr->u.u.type);
		sendpad(client,1);
		send2(client,(long) event_ptr->u.u.sequenceNumber);
		send4(client,(long) event_ptr->u.configureNotify.event);
		send4(client,(long) event_ptr->u.configureNotify.window);
		send4(client,(long) event_ptr->u.configureNotify.aboveSibling);
		send2(client,(long) event_ptr->u.configureNotify.x);
		send2(client,(long) event_ptr->u.configureNotify.y);
		send2(client,(long) event_ptr->u.configureNotify.width);
		send2(client,(long) event_ptr->u.configureNotify.height);
		send2(client,(long) event_ptr->u.configureNotify.borderWidth);
		send1(client,(long) event_ptr->u.configureNotify.override);
		sendpad(client,5);
		break;
	case ConfigureRequest:
		send1(client,(long) event_ptr->u.u.type);
		send1(client,(long) event_ptr->u.u.detail);
		send2(client,(long) event_ptr->u.u.sequenceNumber);
		send4(client,(long) event_ptr->u.configureRequest.parent);
		send4(client,(long) event_ptr->u.configureRequest.window);
		send4(client,(long) event_ptr->u.configureRequest.sibling);
		send2(client,(long) event_ptr->u.configureRequest.x);
		send2(client,(long) event_ptr->u.configureRequest.y);
		send2(client,(long) event_ptr->u.configureRequest.width);
		send2(client,(long) event_ptr->u.configureRequest.height);
		send2(client,(long) event_ptr->u.configureRequest.borderWidth);
		send2(client,(long) event_ptr->u.configureRequest.valueMask);
		sendpad(client,4);
		break;
	case GravityNotify:
		send1(client,(long) event_ptr->u.u.type);
		sendpad(client,1);
		send2(client,(long) event_ptr->u.u.sequenceNumber);
		send4(client,(long) event_ptr->u.gravity.event);
		send4(client,(long) event_ptr->u.gravity.window);
		send2(client,(long) event_ptr->u.gravity.x);
		send2(client,(long) event_ptr->u.gravity.y);
		sendpad(client,16);
		break;
	case ResizeRequest:
		send1(client,(long) event_ptr->u.u.type);
		sendpad(client,1);
		send2(client,(long) event_ptr->u.u.sequenceNumber);
		send4(client,(long) event_ptr->u.resizeRequest.window);
		send2(client,(long) event_ptr->u.resizeRequest.width);
		send2(client,(long) event_ptr->u.resizeRequest.height);
		sendpad(client,20);
		break;
	case CirculateNotify:
		send1(client,(long) event_ptr->u.u.type);
		sendpad(client,1);
		send2(client,(long) event_ptr->u.u.sequenceNumber);
		send4(client,(long) event_ptr->u.circulate.event);
		send4(client,(long) event_ptr->u.circulate.window);
		send4(client,(long) event_ptr->u.circulate.parent);
		send1(client,(long) event_ptr->u.circulate.place);
		sendpad(client,15);
		break;
	case CirculateRequest:
		send1(client,(long) event_ptr->u.u.type);
		sendpad(client,1);
		send2(client,(long) event_ptr->u.u.sequenceNumber);
		send4(client,(long) event_ptr->u.circulate.parent);
		send4(client,(long) event_ptr->u.circulate.window);
		sendpad(client,4);
		send1(client,(long) event_ptr->u.circulate.place);
		sendpad(client,15);
		break;
	case PropertyNotify:
		send1(client,(long) event_ptr->u.u.type);
		sendpad(client,1);
		send2(client,(long) event_ptr->u.u.sequenceNumber);
		send4(client,(long) event_ptr->u.property.window);
		send4(client,(long) event_ptr->u.property.atom);
		send4(client,(long) event_ptr->u.property.time);
		send1(client,(long) event_ptr->u.property.state);
		sendpad(client,15);
		break;
	case SelectionClear:
		send1(client,(long) event_ptr->u.u.type);
		sendpad(client,1);
		send2(client,(long) event_ptr->u.u.sequenceNumber);
		send4(client,(long) event_ptr->u.selectionClear.time);
		send4(client,(long) event_ptr->u.selectionClear.window);
		send4(client,(long) event_ptr->u.selectionClear.atom);
		sendpad(client,16);
		break;
	case SelectionRequest:
		send1(client,(long) event_ptr->u.u.type);
		sendpad(client,1);
		send2(client,(long) event_ptr->u.u.sequenceNumber);
		send4(client,(long) event_ptr->u.selectionRequest.time);
		send4(client,(long) event_ptr->u.selectionRequest.owner);
		send4(client,(long) event_ptr->u.selectionRequest.requestor);
		send4(client,(long) event_ptr->u.selectionRequest.selection);
		send4(client,(long) event_ptr->u.selectionRequest.target);
		send4(client,(long) event_ptr->u.selectionRequest.property);
		sendpad(client,4);
		break;
	case SelectionNotify:
		send1(client,(long) event_ptr->u.u.type);
		sendpad(client,1);
		send2(client,(long) event_ptr->u.u.sequenceNumber);
		send4(client,(long) event_ptr->u.selectionNotify.time);
		send4(client,(long) event_ptr->u.selectionNotify.requestor);
		send4(client,(long) event_ptr->u.selectionNotify.selection);
		send4(client,(long) event_ptr->u.selectionNotify.target);
		send4(client,(long) event_ptr->u.selectionNotify.property);
		sendpad(client,8);
		break;
	case ColormapNotify:
		send1(client,(long) event_ptr->u.u.type);
		sendpad(client,1);
		send2(client,(long) event_ptr->u.u.sequenceNumber);
		send4(client,(long) event_ptr->u.colormap.window);
		send4(client,(long) event_ptr->u.colormap.colormap);
		send1(client,(long) event_ptr->u.colormap.new);
		send1(client,(long) event_ptr->u.colormap.state);
		sendpad(client,18);
		break;
	case ClientMessage:
		send1(client,(long) event_ptr->u.u.type);
		send1(client,(long) event_ptr->u.u.detail);
		send2(client,(long) event_ptr->u.u.sequenceNumber);
		send4(client,(long) event_ptr->u.clientMessage.window);
		send4(client,(long) event_ptr->u.clientMessage.u.l.type);
		if (event_ptr->u.u.detail == 8) {
		    int i;
		    for (i = 0; i < 20; i++) 
			send1(client, event_ptr->u.clientMessage.u.b.bytes[i]);
		}
		else if (event_ptr->u.u.detail == 16) {
		    send2(client,event_ptr->u.clientMessage.u.s.shorts0);
		    send2(client,event_ptr->u.clientMessage.u.s.shorts1);
		    send2(client,event_ptr->u.clientMessage.u.s.shorts2);
		    send2(client,event_ptr->u.clientMessage.u.s.shorts3);
		    send2(client,event_ptr->u.clientMessage.u.s.shorts4);
		    send2(client,event_ptr->u.clientMessage.u.s.shorts5);
		    send2(client,event_ptr->u.clientMessage.u.s.shorts6);
		    send2(client,event_ptr->u.clientMessage.u.s.shorts7);
		    send2(client,event_ptr->u.clientMessage.u.s.shorts8);
		    send2(client,event_ptr->u.clientMessage.u.s.shorts9);
		}
		else if (event_ptr->u.u.detail == 32) {
		    send4(client,event_ptr->u.clientMessage.u.l.longs0);
		    send4(client,event_ptr->u.clientMessage.u.l.longs1);
		    send4(client,event_ptr->u.clientMessage.u.l.longs2);
		    send4(client,event_ptr->u.clientMessage.u.l.longs3);
		    send4(client,event_ptr->u.clientMessage.u.l.longs4);
		}
		else
		    Log_Err ("Send_Evt: bad format for ClientMessage data\n");

/*		send_client_data(client, event_ptr);*/
		break;
	case MappingNotify:
		send1(client,(long) event_ptr->u.u.type);
		sendpad(client,1);
		send2(client,(long) event_ptr->u.u.sequenceNumber);
		send1(client,(long) event_ptr->u.mappingNotify.request);
		send1(client,(long) event_ptr->u.mappingNotify.firstKeyCode);
		send1(client,(long) event_ptr->u.mappingNotify.count);
		sendpad(client,25);
		break;
	default:
		DEFAULT_ERROR;
		break;
	}

}

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
 * $XConsortium: RcvEvt.c,v 1.7 94/04/17 21:01:17 rws Exp $
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
#ifdef Xpi
#include "xtestext1.h"
#endif
#include "DataMove.h"

#define FirstExtensionEvent	64
#define EVENT_HEADER	4	/* constant header */

#ifdef Xpi
int
Rcv_Evt(rp,rbuf,client,base)
xEvent *rp;
char rbuf[];
int client;
int base;
#else
int
Rcv_Evt(rp,rbuf,client)
xEvent *rp;
char rbuf[];
int client;
#endif
{
#ifdef Xpi
	int xtestType;
#endif
	int needswap = Xst_clients[client].cl_swap;
	char *rbp = rbuf;
	int valid = 1;		/* assume all is OK */

	rbp += EVENT_HEADER;

	if (real_type(rp->u.u.type) > FirstExtensionEvent) {
	    Rcv_Ext_Evt(rp,rbuf,client);
	    return(valid);
	}
	switch (real_type(rp->u.u.type)) {
	case KeyPress:
		rp->u.keyButtonPointer.time = unpack4(&rbp,needswap);
		rp->u.keyButtonPointer.root = unpack4(&rbp,needswap);
		rp->u.keyButtonPointer.event = unpack4(&rbp,needswap);
		rp->u.keyButtonPointer.child = unpack4(&rbp,needswap);
		rp->u.keyButtonPointer.rootX = unpack2(&rbp,needswap);
		rp->u.keyButtonPointer.rootY = unpack2(&rbp,needswap);
		rp->u.keyButtonPointer.eventX = unpack2(&rbp,needswap);
		rp->u.keyButtonPointer.eventY = unpack2(&rbp,needswap);
		rp->u.keyButtonPointer.state = unpack2(&rbp,needswap);
		rp->u.keyButtonPointer.sameScreen = unpack1(&rbp);
		break;
	case KeyRelease:
		rp->u.keyButtonPointer.time = unpack4(&rbp,needswap);
		rp->u.keyButtonPointer.root = unpack4(&rbp,needswap);
		rp->u.keyButtonPointer.event = unpack4(&rbp,needswap);
		rp->u.keyButtonPointer.child = unpack4(&rbp,needswap);
		rp->u.keyButtonPointer.rootX = unpack2(&rbp,needswap);
		rp->u.keyButtonPointer.rootY = unpack2(&rbp,needswap);
		rp->u.keyButtonPointer.eventX = unpack2(&rbp,needswap);
		rp->u.keyButtonPointer.eventY = unpack2(&rbp,needswap);
		rp->u.keyButtonPointer.state = unpack2(&rbp,needswap);
		rp->u.keyButtonPointer.sameScreen = unpack1(&rbp);
		break;
	case ButtonPress:
		rp->u.keyButtonPointer.time = unpack4(&rbp,needswap);
		rp->u.keyButtonPointer.root = unpack4(&rbp,needswap);
		rp->u.keyButtonPointer.event = unpack4(&rbp,needswap);
		rp->u.keyButtonPointer.child = unpack4(&rbp,needswap);
		rp->u.keyButtonPointer.rootX = unpack2(&rbp,needswap);
		rp->u.keyButtonPointer.rootY = unpack2(&rbp,needswap);
		rp->u.keyButtonPointer.eventX = unpack2(&rbp,needswap);
		rp->u.keyButtonPointer.eventY = unpack2(&rbp,needswap);
		rp->u.keyButtonPointer.state = unpack2(&rbp,needswap);
		rp->u.keyButtonPointer.sameScreen = unpack1(&rbp);
		break;
	case ButtonRelease:
		rp->u.keyButtonPointer.time = unpack4(&rbp,needswap);
		rp->u.keyButtonPointer.root = unpack4(&rbp,needswap);
		rp->u.keyButtonPointer.event = unpack4(&rbp,needswap);
		rp->u.keyButtonPointer.child = unpack4(&rbp,needswap);
		rp->u.keyButtonPointer.rootX = unpack2(&rbp,needswap);
		rp->u.keyButtonPointer.rootY = unpack2(&rbp,needswap);
		rp->u.keyButtonPointer.eventX = unpack2(&rbp,needswap);
		rp->u.keyButtonPointer.eventY = unpack2(&rbp,needswap);
		rp->u.keyButtonPointer.state = unpack2(&rbp,needswap);
		rp->u.keyButtonPointer.sameScreen = unpack1(&rbp);
		break;
	case MotionNotify:
		rp->u.keyButtonPointer.time = unpack4(&rbp,needswap);
		rp->u.keyButtonPointer.root = unpack4(&rbp,needswap);
		rp->u.keyButtonPointer.event = unpack4(&rbp,needswap);
		rp->u.keyButtonPointer.child = unpack4(&rbp,needswap);
		rp->u.keyButtonPointer.rootX = unpack2(&rbp,needswap);
		rp->u.keyButtonPointer.rootY = unpack2(&rbp,needswap);
		rp->u.keyButtonPointer.eventX = unpack2(&rbp,needswap);
		rp->u.keyButtonPointer.eventY = unpack2(&rbp,needswap);
		rp->u.keyButtonPointer.state = unpack2(&rbp,needswap);
		rp->u.keyButtonPointer.sameScreen = unpack1(&rbp);
		break;
	case EnterNotify:
		rp->u.enterLeave.time = unpack4(&rbp,needswap);
		rp->u.enterLeave.root = unpack4(&rbp,needswap);
		rp->u.enterLeave.event = unpack4(&rbp,needswap);
		rp->u.enterLeave.child = unpack4(&rbp,needswap);
		rp->u.enterLeave.rootX = unpack2(&rbp,needswap);
		rp->u.enterLeave.rootY = unpack2(&rbp,needswap);
		rp->u.enterLeave.eventX = unpack2(&rbp,needswap);
		rp->u.enterLeave.eventY = unpack2(&rbp,needswap);
		rp->u.enterLeave.state = unpack2(&rbp,needswap);
		rp->u.enterLeave.mode = unpack1(&rbp);
		rp->u.enterLeave.flags = unpack1(&rbp);
		break;
	case LeaveNotify:
		rp->u.enterLeave.time = unpack4(&rbp,needswap);
		rp->u.enterLeave.root = unpack4(&rbp,needswap);
		rp->u.enterLeave.event = unpack4(&rbp,needswap);
		rp->u.enterLeave.child = unpack4(&rbp,needswap);
		rp->u.enterLeave.rootX = unpack2(&rbp,needswap);
		rp->u.enterLeave.rootY = unpack2(&rbp,needswap);
		rp->u.enterLeave.eventX = unpack2(&rbp,needswap);
		rp->u.enterLeave.eventY = unpack2(&rbp,needswap);
		rp->u.enterLeave.state = unpack2(&rbp,needswap);
		rp->u.enterLeave.mode = unpack1(&rbp);
		rp->u.enterLeave.flags = unpack1(&rbp);
		break;
	case FocusIn:
		rp->u.focus.window = unpack4(&rbp,needswap);
		rp->u.focus.mode = unpack1(&rbp);
		break;
	case FocusOut:
		rp->u.focus.window = unpack4(&rbp,needswap);
		rp->u.focus.mode = unpack1(&rbp);
		break;
	case KeymapNotify:
		rbp = rbuf + 1;
		bcopy(rbp,(unsigned char *) (((xKeymapEvent *)rp)->map),31);
		break;
	case Expose:
		rp->u.expose.window = unpack4(&rbp,needswap);
		rp->u.expose.x = unpack2(&rbp,needswap);
		rp->u.expose.y = unpack2(&rbp,needswap);
		rp->u.expose.width = unpack2(&rbp,needswap);
		rp->u.expose.height = unpack2(&rbp,needswap);
		rp->u.expose.count = unpack2(&rbp,needswap);
		break;
	case GraphicsExpose:
		rp->u.graphicsExposure.drawable = unpack4(&rbp,needswap);
		rp->u.graphicsExposure.x = unpack2(&rbp,needswap);
		rp->u.graphicsExposure.y = unpack2(&rbp,needswap);
		rp->u.graphicsExposure.width = unpack2(&rbp,needswap);
		rp->u.graphicsExposure.height = unpack2(&rbp,needswap);
		rp->u.graphicsExposure.minorEvent = unpack2(&rbp,needswap);
		rp->u.graphicsExposure.count = unpack2(&rbp,needswap);
		rp->u.graphicsExposure.majorEvent = unpack1(&rbp);
		break;
	case NoExpose:
		rp->u.noExposure.drawable = unpack4(&rbp,needswap);
		rp->u.noExposure.minorEvent = unpack2(&rbp,needswap);
		rp->u.noExposure.majorEvent = unpack1(&rbp);
		break;
	case VisibilityNotify:
		rp->u.visibility.window = unpack4(&rbp,needswap);
		rp->u.visibility.state = unpack1(&rbp);
		break;
	case CreateNotify:
		rp->u.createNotify.parent = unpack4(&rbp,needswap);
		rp->u.createNotify.window = unpack4(&rbp,needswap);
		rp->u.createNotify.x = unpack2(&rbp,needswap);
		rp->u.createNotify.y = unpack2(&rbp,needswap);
		rp->u.createNotify.width = unpack2(&rbp,needswap);
		rp->u.createNotify.height = unpack2(&rbp,needswap);
		rp->u.createNotify.borderWidth = unpack2(&rbp,needswap);
		rp->u.createNotify.override = unpack1(&rbp);
		break;
	case DestroyNotify:
		rp->u.destroyNotify.event = unpack4(&rbp,needswap);
		rp->u.destroyNotify.window = unpack4(&rbp,needswap);
		break;
	case UnmapNotify:
		rp->u.unmapNotify.event = unpack4(&rbp,needswap);
		rp->u.unmapNotify.window = unpack4(&rbp,needswap);
		rp->u.unmapNotify.fromConfigure = unpack1(&rbp);
		break;
	case MapNotify:
		rp->u.mapNotify.event = unpack4(&rbp,needswap);
		rp->u.mapNotify.window = unpack4(&rbp,needswap);
		rp->u.mapNotify.override = unpack1(&rbp);
		break;
	case MapRequest:
		rp->u.mapRequest.parent = unpack4(&rbp,needswap);
		rp->u.mapRequest.window = unpack4(&rbp,needswap);
		break;
	case ReparentNotify:
		rp->u.reparent.event = unpack4(&rbp,needswap);
		rp->u.reparent.window = unpack4(&rbp,needswap);
		rp->u.reparent.parent = unpack4(&rbp,needswap);
		rp->u.reparent.x = unpack2(&rbp,needswap);
		rp->u.reparent.y = unpack2(&rbp,needswap);
		rp->u.reparent.override = unpack1(&rbp);
		break;
	case ConfigureNotify:
		rp->u.configureNotify.event = unpack4(&rbp,needswap);
		rp->u.configureNotify.window = unpack4(&rbp,needswap);
		rp->u.configureNotify.aboveSibling = unpack4(&rbp,needswap);
		rp->u.configureNotify.x = unpack2(&rbp,needswap);
		rp->u.configureNotify.y = unpack2(&rbp,needswap);
		rp->u.configureNotify.width = unpack2(&rbp,needswap);
		rp->u.configureNotify.height = unpack2(&rbp,needswap);
		rp->u.configureNotify.borderWidth = unpack2(&rbp,needswap);
		rp->u.configureNotify.override = unpack1(&rbp);
		break;
	case ConfigureRequest:
		rp->u.configureRequest.parent = unpack4(&rbp,needswap);
		rp->u.configureRequest.window = unpack4(&rbp,needswap);
		rp->u.configureRequest.sibling = unpack4(&rbp,needswap);
		rp->u.configureRequest.x = unpack2(&rbp,needswap);
		rp->u.configureRequest.y = unpack2(&rbp,needswap);
		rp->u.configureRequest.width = unpack2(&rbp,needswap);
		rp->u.configureRequest.height = unpack2(&rbp,needswap);
		rp->u.configureRequest.borderWidth = unpack2(&rbp,needswap);
		rp->u.configureRequest.valueMask = unpack2(&rbp,needswap);
		break;
	case GravityNotify:
		rp->u.gravity.event = unpack4(&rbp,needswap);
		rp->u.gravity.window = unpack4(&rbp,needswap);
		rp->u.gravity.x = unpack2(&rbp,needswap);
		rp->u.gravity.y = unpack2(&rbp,needswap);
		break;
	case ResizeRequest:
		rp->u.resizeRequest.window = unpack4(&rbp,needswap);
		rp->u.resizeRequest.width = unpack2(&rbp,needswap);
		rp->u.resizeRequest.height = unpack2(&rbp,needswap);
		break;
	case CirculateNotify:
		rp->u.circulate.event = unpack4(&rbp,needswap);
		rp->u.circulate.window = unpack4(&rbp,needswap);
		rp->u.circulate.parent = unpack4(&rbp,needswap);
		rp->u.circulate.place = unpack1(&rbp);
		break;
	case CirculateRequest:
		rp->u.circulate.parent = unpack4(&rbp,needswap);
		rp->u.circulate.window = unpack4(&rbp,needswap);
		rbp += 4;
		rp->u.circulate.place = unpack1(&rbp);
		break;
	case PropertyNotify:
		rp->u.property.window = unpack4(&rbp,needswap);
		rp->u.property.atom = unpack4(&rbp,needswap);
		rp->u.property.time = unpack4(&rbp,needswap);
		rp->u.property.state = unpack1(&rbp);
		break;
	case SelectionClear:
		rp->u.selectionClear.time = unpack4(&rbp,needswap);
		rp->u.selectionClear.window = unpack4(&rbp,needswap);
		rp->u.selectionClear.atom = unpack4(&rbp,needswap);
		break;
	case SelectionRequest:
		rp->u.selectionRequest.time = unpack4(&rbp,needswap);
		rp->u.selectionRequest.owner = unpack4(&rbp,needswap);
		rp->u.selectionRequest.requestor = unpack4(&rbp,needswap);
		rp->u.selectionRequest.selection = unpack4(&rbp,needswap);
		rp->u.selectionRequest.target = unpack4(&rbp,needswap);
		rp->u.selectionRequest.property = unpack4(&rbp,needswap);
		break;
	case SelectionNotify:
		rp->u.selectionNotify.time = unpack4(&rbp,needswap);
		rp->u.selectionNotify.requestor = unpack4(&rbp,needswap);
		rp->u.selectionNotify.selection = unpack4(&rbp,needswap);
		rp->u.selectionNotify.target = unpack4(&rbp,needswap);
		rp->u.selectionNotify.property = unpack4(&rbp,needswap);
		break;
	case ColormapNotify:
		rp->u.colormap.window = unpack4(&rbp,needswap);
		rp->u.colormap.colormap = unpack4(&rbp,needswap);
		rp->u.colormap.new = unpack1(&rbp);
		rp->u.colormap.state = unpack1(&rbp);
		break;
	case ClientMessage:  {
	        int i;

		rp->u.clientMessage.window = unpack4(&rbp,needswap);
		rp->u.clientMessage.u.l.type = unpack4(&rbp,needswap);

		if (rp->u.u.detail == 8)
		    for (i = 0; i < 20; i++) 
			rp->u.clientMessage.u.b.bytes[i] = unpack1(&rbp);
		else if (rp->u.u.detail == 16) {
		    rp->u.clientMessage.u.s.shorts0 = unpack2(&rbp,needswap);
		    rp->u.clientMessage.u.s.shorts1 = unpack2(&rbp,needswap);
		    rp->u.clientMessage.u.s.shorts2 = unpack2(&rbp,needswap);
		    rp->u.clientMessage.u.s.shorts3 = unpack2(&rbp,needswap);
		    rp->u.clientMessage.u.s.shorts4 = unpack2(&rbp,needswap);
		    rp->u.clientMessage.u.s.shorts5 = unpack2(&rbp,needswap);
		    rp->u.clientMessage.u.s.shorts6 = unpack2(&rbp,needswap);
		    rp->u.clientMessage.u.s.shorts7 = unpack2(&rbp,needswap);
		    rp->u.clientMessage.u.s.shorts8 = unpack2(&rbp,needswap);
		    rp->u.clientMessage.u.s.shorts9 = unpack2(&rbp,needswap);
		}
		else if (rp->u.u.detail == 32) {
		    rp->u.clientMessage.u.l.longs0 = unpack4(&rbp,needswap);
		    rp->u.clientMessage.u.l.longs1 = unpack4(&rbp,needswap);
		    rp->u.clientMessage.u.l.longs2 = unpack4(&rbp,needswap);
		    rp->u.clientMessage.u.l.longs3 = unpack4(&rbp,needswap);
		    rp->u.clientMessage.u.l.longs4 = unpack4(&rbp,needswap);
		}
		else
		    Log_Err ("Rcv_Evt: bad format for ClientMessage data\n");
		break;
	    }
	case MappingNotify:
		rp->u.mappingNotify.request = unpack1(&rbp);
		rp->u.mappingNotify.firstKeyCode = unpack1(&rbp);
		rp->u.mappingNotify.count = unpack1(&rbp);
		break;
	default:
#ifdef Xpi
	/*
	 * This is an Event for the input synthesis extension 
	 * or it is a Default Error. If it is an Event then Expect will
	 * pass the base code in the type field. We can check for
	 * the type of input synthesis Event by subtracting the
	 * base code from the received value.
	 *
	 */

		xtestType = (rp->u.u.type) - base;

		switch(xtestType){
		case XTest_InputAction_EventType:
			/*
			 * Pull out the input actions captured by the
			 * server.
			 */
			Log_Msg("InputActionEvent");
			break;
		case XTest_FakeAck_EventType:
			/*
			 * Nothing special to pull out.
			 */
			Log_Msg("FakeAckEvent");
			break;
		default:
			DEFAULT_ERROR;
			break;
		}
#else
		DEFAULT_ERROR;
		break;
#endif
	}
	return(valid);

}

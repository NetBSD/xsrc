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
 * $XConsortium: ShowEvt.c,v 1.7 94/04/17 21:01:25 rws Exp $
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

#define FirstExtensionEvent 64
#include "XstlibInt.h"

void
Show_Evt(mmp)
xEvent *mmp;
{
	xEvent *mp = (xEvent *)Xstmalloc((unsigned)sizeof(xEvent));

	bcopy((char *)mmp, (char *)mp, (unsigned)sizeof(xEvent));
	/* always ensure we've got enough room */

	if (real_type(mp->u.u.type) > FirstExtensionEvent) {
	    Show_Ext_Evt (mp);
	    Free_Event(mp);
	    return;
	}

	switch (real_type(mp->u.u.type)) {
	case KeyPress:
		BPRINTF1("KeyPress:\n");
 		BPRINTF2("\tdetail = %ld\n", (long) mp -> u.u.detail);
 		BPRINTF2("\tsequenceNumber = %ld\n", (long) mp -> u.u.sequenceNumber);
 		BPRINTF2("\ttime = %ld\n",(long) mp->u.keyButtonPointer.time);
		BPRINTF2("\troot = %ld\n",(long) mp->u.keyButtonPointer.root);
		BPRINTF2("\tevent = %ld\n",(long) mp->u.keyButtonPointer.event);
		BPRINTF2("\tchild = %ld\n",(long) mp->u.keyButtonPointer.child);
		BPRINTF2("\trootX = %ld\n",(long) mp->u.keyButtonPointer.rootX);
		BPRINTF2("\trootY = %ld\n",(long) mp->u.keyButtonPointer.rootY);
		BPRINTF2("\teventX = %ld\n",(long) mp->u.keyButtonPointer.eventX);
		BPRINTF2("\teventY = %ld\n",(long) mp->u.keyButtonPointer.eventY);
		BPRINTF2("\tstate = %ld\n",(long) mp->u.keyButtonPointer.state);
		BPRINTF2("\tsameScreen = %ld\n",(long) mp->u.keyButtonPointer.sameScreen);
		break;
	case KeyRelease:
		BPRINTF1("KeyRelease:\n");
 		BPRINTF2("\tdetail = %ld\n", (long) mp -> u.u.detail);
 		BPRINTF2("\tsequenceNumber = %ld\n", (long) mp -> u.u.sequenceNumber);
 		BPRINTF2("\ttime = %ld\n",(long) mp->u.keyButtonPointer.time);
		BPRINTF2("\troot = %ld\n",(long) mp->u.keyButtonPointer.root);
		BPRINTF2("\tevent = %ld\n",(long) mp->u.keyButtonPointer.event);
		BPRINTF2("\tchild = %ld\n",(long) mp->u.keyButtonPointer.child);
		BPRINTF2("\trootX = %ld\n",(long) mp->u.keyButtonPointer.rootX);
		BPRINTF2("\trootY = %ld\n",(long) mp->u.keyButtonPointer.rootY);
		BPRINTF2("\teventX = %ld\n",(long) mp->u.keyButtonPointer.eventX);
		BPRINTF2("\teventY = %ld\n",(long) mp->u.keyButtonPointer.eventY);
		BPRINTF2("\tstate = %ld\n",(long) mp->u.keyButtonPointer.state);
		BPRINTF2("\tsameScreen = %ld\n",(long) mp->u.keyButtonPointer.sameScreen);
		break;
	case ButtonPress:
		BPRINTF1("ButtonPress:\n");
 		BPRINTF2("\tdetail = %ld\n", (long) mp -> u.u.detail);
 		BPRINTF2("\tsequenceNumber = %ld\n", (long) mp -> u.u.sequenceNumber);
 		BPRINTF2("\ttime = %ld\n",(long) mp->u.keyButtonPointer.time);
		BPRINTF2("\troot = %ld\n",(long) mp->u.keyButtonPointer.root);
		BPRINTF2("\tevent = %ld\n",(long) mp->u.keyButtonPointer.event);
		BPRINTF2("\tchild = %ld\n",(long) mp->u.keyButtonPointer.child);
		BPRINTF2("\trootX = %ld\n",(long) mp->u.keyButtonPointer.rootX);
		BPRINTF2("\trootY = %ld\n",(long) mp->u.keyButtonPointer.rootY);
		BPRINTF2("\teventX = %ld\n",(long) mp->u.keyButtonPointer.eventX);
		BPRINTF2("\teventY = %ld\n",(long) mp->u.keyButtonPointer.eventY);
		BPRINTF2("\tstate = %ld\n",(long) mp->u.keyButtonPointer.state);
		BPRINTF2("\tsameScreen = %ld\n",(long) mp->u.keyButtonPointer.sameScreen);
		break;
	case ButtonRelease:
		BPRINTF1("ButtonRelease:\n");
 		BPRINTF2("\tdetail = %ld\n", (long) mp -> u.u.detail);
 		BPRINTF2("\tsequenceNumber = %ld\n", (long) mp -> u.u.sequenceNumber);
 		BPRINTF2("\ttime = %ld\n",(long) mp->u.keyButtonPointer.time);
		BPRINTF2("\troot = %ld\n",(long) mp->u.keyButtonPointer.root);
		BPRINTF2("\tevent = %ld\n",(long) mp->u.keyButtonPointer.event);
		BPRINTF2("\tchild = %ld\n",(long) mp->u.keyButtonPointer.child);
		BPRINTF2("\trootX = %ld\n",(long) mp->u.keyButtonPointer.rootX);
		BPRINTF2("\trootY = %ld\n",(long) mp->u.keyButtonPointer.rootY);
		BPRINTF2("\teventX = %ld\n",(long) mp->u.keyButtonPointer.eventX);
		BPRINTF2("\teventY = %ld\n",(long) mp->u.keyButtonPointer.eventY);
		BPRINTF2("\tstate = %ld\n",(long) mp->u.keyButtonPointer.state);
		BPRINTF2("\tsameScreen = %ld\n",(long) mp->u.keyButtonPointer.sameScreen);
		break;
	case MotionNotify:
		BPRINTF1("MotionNotify:\n");
 		BPRINTF2("\tdetail = %ld\n", (long) mp -> u.u.detail);
 		BPRINTF2("\tsequenceNumber = %ld\n", (long) mp -> u.u.sequenceNumber);
 		BPRINTF2("\ttime = %ld\n",(long) mp->u.keyButtonPointer.time);
		BPRINTF2("\troot = %ld\n",(long) mp->u.keyButtonPointer.root);
		BPRINTF2("\tevent = %ld\n",(long) mp->u.keyButtonPointer.event);
		BPRINTF2("\tchild = %ld\n",(long) mp->u.keyButtonPointer.child);
		BPRINTF2("\trootX = %ld\n",(long) mp->u.keyButtonPointer.rootX);
		BPRINTF2("\trootY = %ld\n",(long) mp->u.keyButtonPointer.rootY);
		BPRINTF2("\teventX = %ld\n",(long) mp->u.keyButtonPointer.eventX);
		BPRINTF2("\teventY = %ld\n",(long) mp->u.keyButtonPointer.eventY);
		BPRINTF2("\tstate = %ld\n",(long) mp->u.keyButtonPointer.state);
		BPRINTF2("\tsameScreen = %ld\n",(long) mp->u.keyButtonPointer.sameScreen);
		break;
	case EnterNotify:
		BPRINTF1("EnterNotify:\n");
 		BPRINTF2("\tdetail = %ld\n", (long) mp -> u.u.detail);
 		BPRINTF2("\tsequenceNumber = %ld\n", (long) mp -> u.u.sequenceNumber);
 		BPRINTF2("\ttime = %ld\n",(long) mp->u.enterLeave.time);
		BPRINTF2("\troot = %ld\n",(long) mp->u.enterLeave.root);
		BPRINTF2("\tevent = %ld\n",(long) mp->u.enterLeave.event);
		BPRINTF2("\tchild = %ld\n",(long) mp->u.enterLeave.child);
		BPRINTF2("\trootX = %ld\n",(long) mp->u.enterLeave.rootX);
		BPRINTF2("\trootY = %ld\n",(long) mp->u.enterLeave.rootY);
		BPRINTF2("\teventX = %ld\n",(long) mp->u.enterLeave.eventX);
		BPRINTF2("\teventY = %ld\n",(long) mp->u.enterLeave.eventY);
		BPRINTF2("\tstate = %ld\n",(long) mp->u.enterLeave.state);
		BPRINTF2("\tmode = %ld\n",(long) mp->u.enterLeave.mode);
		BPRINTF2("\tsame-screen, focus = %ld\n",(long) mp->u.enterLeave.flags);
		break;
	case LeaveNotify:
		BPRINTF1("LeaveNotify:\n");
 		BPRINTF2("\tdetail = %ld\n", (long) mp -> u.u.detail);
 		BPRINTF2("\tsequenceNumber = %ld\n", (long) mp -> u.u.sequenceNumber);
 		BPRINTF2("\ttime = %ld\n",(long) mp->u.enterLeave.time);
		BPRINTF2("\troot = %ld\n",(long) mp->u.enterLeave.root);
		BPRINTF2("\tevent = %ld\n",(long) mp->u.enterLeave.event);
		BPRINTF2("\tchild = %ld\n",(long) mp->u.enterLeave.child);
		BPRINTF2("\trootX = %ld\n",(long) mp->u.enterLeave.rootX);
		BPRINTF2("\trootY = %ld\n",(long) mp->u.enterLeave.rootY);
		BPRINTF2("\teventX = %ld\n",(long) mp->u.enterLeave.eventX);
		BPRINTF2("\teventY = %ld\n",(long) mp->u.enterLeave.eventY);
		BPRINTF2("\tstate = %ld\n",(long) mp->u.enterLeave.state);
		BPRINTF2("\tmode = %ld\n",(long) mp->u.enterLeave.mode);
		BPRINTF2("\tsame-screen, focus = %ld\n",(long) mp->u.enterLeave.flags);
		break;
	case FocusIn:
		BPRINTF1("FocusIn:\n");
 		BPRINTF2("\tdetail = %ld\n", (long) mp -> u.u.detail);
 		BPRINTF2("\tsequenceNumber = %ld\n", (long) mp -> u.u.sequenceNumber);
 		BPRINTF2("\twindow = %ld\n",(long) mp->u.focus.window);
		BPRINTF2("\tmode = %ld\n",(long) mp->u.focus.mode);
		break;
	case FocusOut:
		BPRINTF1("FocusOut:\n");
 		BPRINTF2("\tdetail = %ld\n", (long) mp -> u.u.detail);
 		BPRINTF2("\tsequenceNumber = %ld\n", (long) mp -> u.u.sequenceNumber);
 		BPRINTF2("\twindow = %ld\n",(long) mp->u.focus.window);
		BPRINTF2("\tmode = %ld\n",(long) mp->u.focus.mode);
		break;
	case KeymapNotify:
		BPRINTF1("KeymapNotify:\n");
 		CANT_SHOW("\tXXX","map");
		break;
	case Expose:
		BPRINTF1("Expose:\n");
 		BPRINTF2("\tsequenceNumber = %ld\n", (long) mp -> u.u.sequenceNumber);
 		BPRINTF2("\twindow = %ld\n",(long) mp->u.expose.window);
		BPRINTF2("\tx = %ld\n",(long) mp->u.expose.x);
		BPRINTF2("\ty = %ld\n",(long) mp->u.expose.y);
		BPRINTF2("\twidth = %ld\n",(long) mp->u.expose.width);
		BPRINTF2("\theight = %ld\n",(long) mp->u.expose.height);
		BPRINTF2("\tcount = %ld\n",(long) mp->u.expose.count);
		break;
	case GraphicsExpose:
		BPRINTF1("GraphicsExpose:\n");
 		BPRINTF2("\tsequenceNumber = %ld\n", (long) mp -> u.u.sequenceNumber);
 		BPRINTF2("\tdrawable = %ld\n",(long) mp->u.graphicsExposure.drawable);
		BPRINTF2("\tx = %ld\n",(long) mp->u.graphicsExposure.x);
		BPRINTF2("\ty = %ld\n",(long) mp->u.graphicsExposure.y);
		BPRINTF2("\twidth = %ld\n",(long) mp->u.graphicsExposure.width);
		BPRINTF2("\theight = %ld\n",(long) mp->u.graphicsExposure.height);
		BPRINTF2("\tminorEvent = %ld\n",(long) mp->u.graphicsExposure.minorEvent);
		BPRINTF2("\tcount = %ld\n",(long) mp->u.graphicsExposure.count);
		BPRINTF2("\tmajorEvent = %ld\n",(long) mp->u.graphicsExposure.majorEvent);
		break;
	case NoExpose:
		BPRINTF1("NoExpose:\n");
 		BPRINTF2("\tsequenceNumber = %ld\n", (long) mp -> u.u.sequenceNumber);
 		BPRINTF2("\tdrawable = %ld\n",(long) mp->u.noExposure.drawable);
		BPRINTF2("\tminorEvent = %ld\n",(long) mp->u.noExposure.minorEvent);
		BPRINTF2("\tmajorEvent = %ld\n",(long) mp->u.noExposure.majorEvent);
		break;
	case VisibilityNotify:
		BPRINTF1("VisibilityNotify:\n");
 		BPRINTF2("\tsequenceNumber = %ld\n", (long) mp -> u.u.sequenceNumber);
 		BPRINTF2("\twindow = %ld\n",(long) mp->u.visibility.window);
		BPRINTF2("\tstate = %ld\n",(long) mp->u.visibility.state);
		break;
	case CreateNotify:
		BPRINTF1("CreateNotify:\n");
 		BPRINTF2("\tsequenceNumber = %ld\n", (long) mp -> u.u.sequenceNumber);
 		BPRINTF2("\tparent = %ld\n",(long) mp->u.createNotify.parent);
		BPRINTF2("\twindow = %ld\n",(long) mp->u.createNotify.window);
		BPRINTF2("\tx = %ld\n",(long) mp->u.createNotify.x);
		BPRINTF2("\ty = %ld\n",(long) mp->u.createNotify.y);
		BPRINTF2("\twidth = %ld\n",(long) mp->u.createNotify.width);
		BPRINTF2("\theight = %ld\n",(long) mp->u.createNotify.height);
		BPRINTF2("\tborderWidth = %ld\n",(long) mp->u.createNotify.borderWidth);
		BPRINTF2("\toverride = %ld\n",(long) mp->u.createNotify.override);
		break;
	case DestroyNotify:
		BPRINTF1("DestroyNotify:\n");
 		BPRINTF2("\tsequenceNumber = %ld\n", (long) mp -> u.u.sequenceNumber);
 		BPRINTF2("\tevent = %ld\n",(long) mp->u.destroyNotify.event);
		BPRINTF2("\twindow = %ld\n",(long) mp->u.destroyNotify.window);
		break;
	case UnmapNotify:
		BPRINTF1("UnmapNotify:\n");
 		BPRINTF2("\tsequenceNumber = %ld\n", (long) mp -> u.u.sequenceNumber);
 		BPRINTF2("\tevent = %ld\n",(long) mp->u.unmapNotify.event);
		BPRINTF2("\twindow = %ld\n",(long) mp->u.unmapNotify.window);
		BPRINTF2("\tfromConfigure = %ld\n",(long) mp->u.unmapNotify.fromConfigure);
		break;
	case MapNotify:
		BPRINTF1("MapNotify:\n");
 		BPRINTF2("\tsequenceNumber = %ld\n", (long) mp -> u.u.sequenceNumber);
 		BPRINTF2("\tevent = %ld\n",(long) mp->u.mapNotify.event);
		BPRINTF2("\twindow = %ld\n",(long) mp->u.mapNotify.window);
		BPRINTF2("\toverride = %ld\n",(long) mp->u.mapNotify.override);
		break;
	case MapRequest:
		BPRINTF1("MapRequest:\n");
 		BPRINTF2("\tsequenceNumber = %ld\n", (long) mp -> u.u.sequenceNumber);
 		BPRINTF2("\tparent = %ld\n",(long) mp->u.mapRequest.parent);
		BPRINTF2("\twindow = %ld\n",(long) mp->u.mapRequest.window);
		break;
	case ReparentNotify:
		BPRINTF1("ReparentNotify:\n");
 		BPRINTF2("\tsequenceNumber = %ld\n", (long) mp -> u.u.sequenceNumber);
 		BPRINTF2("\tevent = %ld\n",(long) mp->u.reparent.event);
		BPRINTF2("\twindow = %ld\n",(long) mp->u.reparent.window);
		BPRINTF2("\tparent = %ld\n",(long) mp->u.reparent.parent);
		BPRINTF2("\tx = %ld\n",(long) mp->u.reparent.x);
		BPRINTF2("\ty = %ld\n",(long) mp->u.reparent.y);
		BPRINTF2("\toverride = %ld\n",(long) mp->u.reparent.override);
		break;
	case ConfigureNotify:
		BPRINTF1("ConfigureNotify:\n");
 		BPRINTF2("\tsequenceNumber = %ld\n", (long) mp -> u.u.sequenceNumber);
 		BPRINTF2("\tevent = %ld\n",(long) mp->u.configureNotify.event);
		BPRINTF2("\twindow = %ld\n",(long) mp->u.configureNotify.window);
		BPRINTF2("\taboveSibling = %ld\n",(long) mp->u.configureNotify.aboveSibling);
		BPRINTF2("\tx = %ld\n",(long) mp->u.configureNotify.x);
		BPRINTF2("\ty = %ld\n",(long) mp->u.configureNotify.y);
		BPRINTF2("\twidth = %ld\n",(long) mp->u.configureNotify.width);
		BPRINTF2("\theight = %ld\n",(long) mp->u.configureNotify.height);
		BPRINTF2("\tborderWidth = %ld\n",(long) mp->u.configureNotify.borderWidth);
		BPRINTF2("\toverride = %ld\n",(long) mp->u.configureNotify.override);
		break;
	case ConfigureRequest:
		BPRINTF1("ConfigureRequest:\n");
 		BPRINTF2("\tstack-mode = %ld\n", (long) mp -> u.u.detail);
 		BPRINTF2("\tsequenceNumber = %ld\n", (long) mp -> u.u.sequenceNumber);
 		BPRINTF2("\tparent = %ld\n",(long) mp->u.configureRequest.parent);
		BPRINTF2("\twindow = %ld\n",(long) mp->u.configureRequest.window);
		BPRINTF2("\tsibling = %ld\n",(long) mp->u.configureRequest.sibling);
		BPRINTF2("\tx = %ld\n",(long) mp->u.configureRequest.x);
		BPRINTF2("\ty = %ld\n",(long) mp->u.configureRequest.y);
		BPRINTF2("\twidth = %ld\n",(long) mp->u.configureRequest.width);
		BPRINTF2("\theight = %ld\n",(long) mp->u.configureRequest.height);
		BPRINTF2("\tborderWidth = %ld\n",(long) mp->u.configureRequest.borderWidth);
		BPRINTF2("\tvalueMask = %ld\n",(long) mp->u.configureRequest.valueMask);
		break;
	case GravityNotify:
		BPRINTF1("GravityNotify:\n");
 		BPRINTF2("\tsequenceNumber = %ld\n", (long) mp -> u.u.sequenceNumber);
 		BPRINTF2("\tevent = %ld\n",(long) mp->u.gravity.event);
		BPRINTF2("\twindow = %ld\n",(long) mp->u.gravity.window);
		BPRINTF2("\tx = %ld\n",(long) mp->u.gravity.x);
		BPRINTF2("\ty = %ld\n",(long) mp->u.gravity.y);
		break;
	case ResizeRequest:
		BPRINTF1("ResizeRequest:\n");
 		BPRINTF2("\tsequenceNumber = %ld\n", (long) mp -> u.u.sequenceNumber);
 		BPRINTF2("\twindow = %ld\n",(long) mp->u.resizeRequest.window);
		BPRINTF2("\twidth = %ld\n",(long) mp->u.resizeRequest.width);
		BPRINTF2("\theight = %ld\n",(long) mp->u.resizeRequest.height);
		break;
	case CirculateNotify:
		BPRINTF1("CirculateNotify:\n");
 		BPRINTF2("\tsequenceNumber = %ld\n", (long) mp -> u.u.sequenceNumber);
 		BPRINTF2("\tevent = %ld\n",(long) mp->u.circulate.event);
		BPRINTF2("\twindow = %ld\n",(long) mp->u.circulate.window);
		BPRINTF2("\tparent = %ld\n",(long) mp->u.circulate.parent);
		BPRINTF2("\tplace = %ld\n",(long) mp->u.circulate.place);
		break;
	case CirculateRequest:
		BPRINTF1("CirculateRequest:\n");
 		BPRINTF2("\tsequenceNumber = %ld\n", (long) mp -> u.u.sequenceNumber);
 		BPRINTF2("\tparent = %ld\n",(long) mp->u.circulate.parent);
		BPRINTF2("\twindow = %ld\n",(long) mp->u.circulate.window);
		BPRINTF2("\tplace = %ld\n",(long) mp->u.circulate.place);
		break;
	case PropertyNotify:
		BPRINTF1("PropertyNotify:\n");
 		BPRINTF2("\tsequenceNumber = %ld\n", (long) mp -> u.u.sequenceNumber);
 		BPRINTF2("\twindow = %ld\n",(long) mp->u.property.window);
		BPRINTF2("\tatom = %ld\n",(long) mp->u.property.atom);
		BPRINTF2("\ttime = %ld\n",(long) mp->u.property.time);
		BPRINTF2("\tstate = %ld\n",(long) mp->u.property.state);
		break;
	case SelectionClear:
		BPRINTF1("SelectionClear:\n");
 		BPRINTF2("\tsequenceNumber = %ld\n", (long) mp -> u.u.sequenceNumber);
 		BPRINTF2("\ttime = %ld\n",(long) mp->u.selectionClear.time);
		BPRINTF2("\twindow = %ld\n",(long) mp->u.selectionClear.window);
		BPRINTF2("\tatom = %ld\n",(long) mp->u.selectionClear.atom);
		break;
	case SelectionRequest:
		BPRINTF1("SelectionRequest:\n");
 		BPRINTF2("\tsequenceNumber = %ld\n", (long) mp -> u.u.sequenceNumber);
 		BPRINTF2("\ttime = %ld\n",(long) mp->u.selectionRequest.time);
		BPRINTF2("\towner = %ld\n",(long) mp->u.selectionRequest.owner);
		BPRINTF2("\trequestor = %ld\n",(long) mp->u.selectionRequest.requestor);
		BPRINTF2("\tselection = %ld\n",(long) mp->u.selectionRequest.selection);
		BPRINTF2("\ttarget = %ld\n",(long) mp->u.selectionRequest.target);
		BPRINTF2("\tproperty = %ld\n",(long) mp->u.selectionRequest.property);
		break;
	case SelectionNotify:
		BPRINTF1("SelectionNotify:\n");
 		BPRINTF2("\tsequenceNumber = %ld\n", (long) mp -> u.u.sequenceNumber);
 		BPRINTF2("\ttime = %ld\n",(long) mp->u.selectionNotify.time);
		BPRINTF2("\trequestor = %ld\n",(long) mp->u.selectionNotify.requestor);
		BPRINTF2("\tselection = %ld\n",(long) mp->u.selectionNotify.selection);
		BPRINTF2("\ttarget = %ld\n",(long) mp->u.selectionNotify.target);
		BPRINTF2("\tproperty = %ld\n",(long) mp->u.selectionNotify.property);
		break;
	case ColormapNotify:
		BPRINTF1("ColormapNotify:\n");
 		BPRINTF2("\tsequenceNumber = %ld\n", (long) mp -> u.u.sequenceNumber);
 		BPRINTF2("\twindow = %ld\n",(long) mp->u.colormap.window);
		BPRINTF2("\tcolormap = %ld\n",(long) mp->u.colormap.colormap);
		BPRINTF2("\tnew = %ld\n",(long) mp->u.colormap.new);
		BPRINTF2("\tstate = %ld\n",(long) mp->u.colormap.state);
		break;
	case ClientMessage: {
		int i;
		
		BPRINTF1("ClientMessage:\n");
 		BPRINTF2("\tformat = %ld\n", (long) mp -> u.u.detail);
 		BPRINTF2("\tsequenceNumber = %ld\n", (long) mp -> u.u.sequenceNumber);
 		BPRINTF2("\twindow = %ld\n",(long) mp->u.clientMessage.window);
 		BPRINTF2("\ttype = %ld\n",(long) mp->u.clientMessage.u.b.type);
		if (mp->u.u.detail == 8)
		    for (i = 0; i < 20; i++) {
			BPRINTF2("\tdata[%d] = ", i);
			BPRINTF2("%x\n",mp->u.clientMessage.u.b.bytes[i]);
		    }
		else if (mp->u.u.detail == 16) {
			BPRINTF1("\tdata[0] = ");
			BPRINTF2("%x\n",mp->u.clientMessage.u.s.shorts0);
			BPRINTF1("\tdata[1] = ");
			BPRINTF2("%x\n",mp->u.clientMessage.u.s.shorts1);
			BPRINTF1("\tdata[2] = ");
			BPRINTF2("%x\n",mp->u.clientMessage.u.s.shorts2);
			BPRINTF1("\tdata[3] = ");
			BPRINTF2("%x\n",mp->u.clientMessage.u.s.shorts3);
			BPRINTF1("\tdata[4] = ");
			BPRINTF2("%x\n",mp->u.clientMessage.u.s.shorts4);
			BPRINTF1("\tdata[5] = ");
			BPRINTF2("%x\n",mp->u.clientMessage.u.s.shorts5);
			BPRINTF1("\tdata[6] = ");
			BPRINTF2("%x\n",mp->u.clientMessage.u.s.shorts6);
			BPRINTF1("\tdata[7] = ");
			BPRINTF2("%x\n",mp->u.clientMessage.u.s.shorts7);
			BPRINTF1("\tdata[8] = ");
			BPRINTF2("%x\n",mp->u.clientMessage.u.s.shorts8);
			BPRINTF1("\tdata[9] = ");
			BPRINTF2("%x\n",mp->u.clientMessage.u.s.shorts9);
		    }
		else if (mp->u.u.detail == 32) {
			BPRINTF1("\tdata[0] = ");
			BPRINTF2("%x\n",mp->u.clientMessage.u.l.longs0);
			BPRINTF1("\tdata[1] = ");
			BPRINTF2("%x\n",mp->u.clientMessage.u.l.longs1);
			BPRINTF1("\tdata[2] = ");
			BPRINTF2("%x\n",mp->u.clientMessage.u.l.longs2);
			BPRINTF1("\tdata[3] = ");
			BPRINTF2("%x\n",mp->u.clientMessage.u.l.longs3);
			BPRINTF1("\tdata[4] = ");
			BPRINTF2("%x\n",mp->u.clientMessage.u.l.longs4);
		    }
		else {
		    BPRINTF2("\tData is unknown format %d; cannot show",mp->u.u.detail);
		    DEFAULT_ERROR;
		}
		BPRINTF1("\n");
		break;
	    }
	case MappingNotify:
		BPRINTF1("MappingNotify:\n");
 		BPRINTF2("\tsequenceNumber = %ld\n", (long) mp -> u.u.sequenceNumber);
 		BPRINTF2("\trequest = %ld\n",(long) mp->u.mappingNotify.request);
		BPRINTF2("\tfirstKeyCode = %ld\n",(long) mp->u.mappingNotify.firstKeyCode);
		BPRINTF2("\tcount = %ld\n",(long) mp->u.mappingNotify.count);
		break;
	default:
		BPRINTF1("UNKNOWN EVENT TYPE:\n");
		BPRINTF2("\ttype = %ld\n", (long) mp -> u.u.type);
 		BPRINTF2("\tsequenceNumber = %ld\n", (long) mp -> u.u.sequenceNumber);
 		BPRINTF2("\trequest = %ld\n",(long) mp->u.mappingNotify.request);
		break;
	}
	Free_Event(mp);
}

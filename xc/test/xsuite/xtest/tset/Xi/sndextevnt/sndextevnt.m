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
 * $XConsortium: sndextevnt.m,v 1.22 94/04/17 21:12:54 rws Exp $
 */
>>TITLE XSendExtensionEvent XINPUT
Status
XSendExtensionEvent(display, device, w, propagate, count, event_list, event_send)
Display *display = Dsp;
XDevice *device;
Window w = (Window) 0xffffffff;
Bool propagate = False;
int count=1;
XEventClass *event_list;
XEvent	*event_send = &_event;
>>SET startup focusstartup
>>SET cleanup focuscleanup
>>EXTERN
#define XInputNumEvents 15
extern ExtDeviceInfo Devs;
extern	int	nclass, nevent, event_types[];
extern XEventClass classes[];
extern XInputFirstEvent;
/*
 * Can not use "xcall" because it empties the event queue.
 */
#define	_xcall_(rvalue)	\
		_startcall(display);\
		((XAnyEvent *) event_send)->send_event = False;\
		rvalue = XSendExtensionEvent(display, device, w, propagate, count, event_list,event_send);\
		_endcall(display)
static XEvent _event;

>>ASSERTION Good B 3
A call to xname
sends
.A event_send
to window
.A w .
>>STRATEGY
Create window.
Discard all events in the event queue.
Call XSendExtensionEvent to send a KeyPress event to creator of window.
Verify that XSendExtensionEvent returned non-zero.
Verify that event was received.
Verify that send_event was not set to False.
Repeat for each event-type.
>>CODE
int	i,j;
Window	w;
XAnyEvent *event;
XEvent	event_return;
int	return_value;
XID noextensionevent;
XEventClass noextensioneventclass;

/* Create window. */
	if (!Setup_Extension_DeviceInfo(BtnMask))
	    {
	    untested("%s: No input extension button device.\n", TestName);
	    return;
	    }
	device = Devs.Button;
	NoExtensionEvent(device, noextensionevent, noextensioneventclass);
	w = mkwin(display, (XVisualInfo *) NULL, (struct area *) NULL, False);
	propagate = False;
	event_list = &noextensioneventclass;
	event = (XAnyEvent *) &event_return;
	for (i = 0; i < nevent; i++) {

/* Discard all events in the event queue. */
		XSync(display, True);

/* Call XSendExtensionEvent to send a KeyPress event to creator of window. */
		event_send->type = event_types[i];
		_xcall_(return_value);

/* Verify that XSendExtensionEvent returned non-zero. */
		if (return_value == 0) {
			report("Returned zero, expected non-zero.");
			FAIL;
		}
		else
			CHECK;

/* Verify that event was received. */
		XSync(display, False);
		if (XCheckTypedEvent(display, event_send->type, &event_return) == False) {
			report("Expected event (%s) not received.", eventname(event_send->type));
			FAIL;
			continue;
		}
		else
			CHECK;

/* Verify that send_event was not set to False. */
		if (event->send_event == False) {
			report("send_event not set to True");
			FAIL;
		}
		else
			CHECK;

/* Repeat for each event-type. */
	}
	CHECKPASS(3*nevent);
>>ASSERTION Good B 3
When
.A w
is
.S PointerWindow ,
then the destination window is the window that contains the pointer.
>>STRATEGY
Create window.
Select KeyPress-type events on window.
Grab server.
Enable synchronization.
Save initial pointer location.
Warp pointer to window.
Get new pointer location.
Discard all events in the event queue.
Call XSendExtensionEvent to send a KeyPress event to window containing pointer.
Get current pointer location.
Check to see if pointer moved.
Warp pointer back to where it started.
Disable synchronization.
Ungrab server.
Verify that XSendExtensionEvent returned non-zero.
Verify that event was received.
Verify that send_event was not set to False.
>>CODE
XVisualInfo *vp;
Window	ptrwin;
XAnyEvent *event;
XEvent	event_return;
int	return_value;
int root_x, root_y;	/* pointer location after XSendExtensionEvent */
int oroot_x, oroot_y;	/* pointer location before XSendExtensionEvent */
int sroot_x, sroot_y;	/* initial pointer location */
Window oldroot;		/* initial root window */
int itmp;		/* useless XQueryPointer return values */
unsigned int uitmp;	/* useless XQueryPointer return values */
Window wtmp;		/* useless XQueryPointer return values */
int dbp;
XEventClass dbpc;

/* Create window. */
	if (!Setup_Extension_DeviceInfo(BtnMask))
	    {
	    untested("%s: No input extension button device.\n", TestName);
	    return;
	    }
	device = Devs.Button;
	DeviceButtonPress(Devs.Button, dbp, dbpc);
	resetvinf(VI_WIN);
	nextvinf(&vp);
	ptrwin = makewin(display, vp);
/* Select KeyPress-type events on window. */
	XSelectExtensionEvent(display, ptrwin, &dbpc, 1);
/* Grab server. */
	XGrabServer(display);
/* Enable synchronization. */
	(void) XSynchronize(display, True);
/* Save initial pointer location. */
	(void) XQueryPointer(display, ptrwin, &oldroot, &wtmp, &sroot_x, &sroot_y, &itmp, &itmp, &uitmp);
/* Warp pointer to window. */
	XWarpPointer(display, None, ptrwin, 0, 0, 0, 0, 0, 0);
/* Get new pointer location. */
	if (XQueryPointer(display, ptrwin, &wtmp, &wtmp, &oroot_x, &oroot_y, &itmp, &itmp, &uitmp) == False) {
		delete("Pointer on wrong root.");
		(void) XSynchronize(display, False);
		XUngrabServer(display);
		return;
	} else
		CHECK;
/* Discard all events in the event queue. */
	XSync(display, True);
/* Call XSendExtensionEvent to send a KeyPress event to window containing pointer. */
	w = (Window) PointerWindow;
	propagate = False;
	event_list = &dbpc;
	event_send->type = dbp;
	_xcall_(return_value);
/* Get current pointer location. */
	if (XQueryPointer(display, ptrwin, &wtmp, &wtmp, &root_x, &root_y, &itmp, &itmp, &uitmp) == False) {
		delete("Pointer moved.");
		(void) XSynchronize(display, False);
		XUngrabServer(display);
		return;
	}
	else
		CHECK;
/* Check to see if pointer moved. */
	if (oroot_x != root_x || oroot_y != root_y) {
		delete("Pointer moved.");
		/*
		 * if this is ever changed to be a return, take care to
		 * also turn off synchronization and ungrab the server...
		 */
	}
	else
		CHECK;
/* Warp pointer back to where it started. */
	XWarpPointer(display, None, oldroot, 0, 0, 0, 0, sroot_x, sroot_y);
/* Disable synchronization. */
	(void) XSynchronize(display, False);
/* Ungrab server. */
	XUngrabServer(display);
/* Verify that XSendExtensionEvent returned non-zero. */
	if (return_value == 0) {
		report("Returned zero, expected non-zero.");
		FAIL;
	}
	else
		CHECK;
/* Verify that event was received. */
	XSync(display, False);
	if (XCheckTypedEvent(display, event_send->type, &event_return) == False) {
		report("Expected event (%s) not received.", eventname(event_send->type));
		FAIL;
	}
	else
		CHECK;
/* Verify that send_event was not set to False. */
	event = (XAnyEvent *) &event_return;
	if (event->send_event == False) {
		report("send_event not set to True");
		FAIL;
	}
	else
		CHECK;
	CHECKPASS(6);
>>ASSERTION Good B 3
When
.A w
is
.S InputFocus
and an inferior of the focus window contains the pointer,
then
the destination window is that inferior.
>>STRATEGY
Create client2.
Create parent window.
Create inferior window.
Select KeyPress-type events on parent window with client2.
Flush client2 requests.
Select KeyPress-type events on child window.
Grab server.
Enable synchronization.
Set input focus to parent window.
Save initial pointer location.
Warp pointer to inferior of focus window.
Get new pointer location.
Discard all events in the event queue.
Call XSendExtensionEvent to send a KeyPress event to inferior of the focus window.
Get current pointer location.
Check to see if pointer moved.
Warp pointer back to where it started.
Disable synchronization.
Ungrab server.
Verify that XSendExtensionEvent returned non-zero.
Verify that event was received for inferior of focus window.
Verify that send_event was not set to False.
Verify that event was not received for focus window.
>>CODE
Display	*client2;
XVisualInfo *vp;
Window	parent;
Window	child;
XAnyEvent *event;
XEvent	event_return;
int	return_value;
int root_x, root_y;	/* pointer location after XSendExtensionEvent */
int oroot_x, oroot_y;	/* pointer location before XSendExtensionEvent */
int sroot_x, sroot_y;	/* initial pointer location */
Window oldroot;		/* initial root window */
int itmp;		/* useless XQueryPointer return values */
unsigned int uitmp;	/* useless XQueryPointer return values */
Window wtmp;		/* useless XQueryPointer return values */
unsigned int width;
unsigned int height;
struct area a;
int dbp;
XEventClass dbpc;

/* Create client2. */
	if (!Setup_Extension_DeviceInfo(BtnMask))
	    {
	    untested("%s: No input extension button device.\n", TestName);
	    return;
	    }
	device = Devs.Button;
	DeviceButtonPress(Devs.Button, dbp, dbpc);
	client2 = opendisplay();
	if (client2 == (Display *) NULL) {
		delete("Can't create new client.");
		return;
	}
	else
		CHECK;
/* Create parent window. */
	resetvinf(VI_WIN);
	nextvinf(&vp);
	parent = makewin(display, vp);
/* Create inferior window. */
	getsize(display, (Drawable) parent, &width, &height);
	a.x = 0;
	a.y = 0;
	a.width = width/2;
	a.height = height/2;
	child = crechild(display, parent, &a);
/* Select KeyPress-type events on parent window with client2. */
	XSelectExtensionEvent(client2, parent, &dbpc, 1);
/* Flush client2 requests. */
	XFlush(client2);
/* Select KeyPress-type events on child window. */
	XSelectExtensionEvent(display, child, &dbpc, 1);
/* Grab server. */
	XGrabServer(display);
/* Enable synchronization. */
	(void) XSynchronize(display, True);
/* Set input focus to parent window. */
	XSetDeviceFocus(display, device, parent, RevertToPointerRoot, CurrentTime);
/* Save initial pointer location. */
	(void) XQueryPointer(display, child, &oldroot, &wtmp, &sroot_x, &sroot_y, &itmp, &itmp, &uitmp);
/* Warp pointer to inferior of focus window. */
	XWarpPointer(display, None, child, 0, 0, 0, 0, 0, 0);

/* Get new pointer location. */
	if (XQueryPointer(display, child, &wtmp, &wtmp, &oroot_x, &oroot_y, &itmp, &itmp, &uitmp) == False) {
		delete("Pointer on wrong root.");
		(void) XSynchronize(display, False);
		XUngrabServer(display);
		return;
	} else
		CHECK;
/* Discard all events in the event queue. */
	XSync(display, True);
/* Call XSendExtensionEvent to send a KeyPress event to inferior of the focus window. */
	w = (Window) InputFocus;
	propagate = False;
	event_list = &dbpc;
	event_send->type = dbp;
	((XAnyEvent *) event_send)->window = child;
	_xcall_(return_value);
/* Get current pointer location. */
	if (XQueryPointer(display, child, &wtmp, &wtmp, &root_x, &root_y, &itmp, &itmp, &uitmp) == False) {
		delete("Pointer moved.");
		(void) XSynchronize(display, False);
		XUngrabServer(display);
		return;
	}
	else
		CHECK;
/* Check to see if pointer moved. */
	if (oroot_x != root_x || oroot_y != root_y) {
		delete("Pointer moved.");
		/*
		 * if this is ever changed to be a return, take care to
		 * also turn off synchronization and ungrab the server...
		 */
	}
	else
		CHECK;
/* Warp pointer back to where it started. */
	XWarpPointer(display, None, oldroot, 0, 0, 0, 0, sroot_x, sroot_y);
/* Disable synchronization. */
	(void) XSynchronize(display, False);
/* Ungrab server. */
	XUngrabServer(display);
/* Verify that XSendExtensionEvent returned non-zero. */
	if (return_value == 0) {
		report("Returned zero, expected non-zero.");
		FAIL;
	}
	else
		CHECK;
/* Verify that event was received for inferior of focus window. */
	XSync(display, False);
	if (XCheckTypedWindowEvent(display, child, event_send->type, &event_return) == False) {
		report("Expected event (%s) on inferior not received.", eventname(event_send->type));
		FAIL;
	}
	else
		CHECK;
/* Verify that send_event was not set to False. */
	event = (XAnyEvent *) &event_return;
	if (event->send_event == False) {
		report("send_event not set to True");
		FAIL;
	}
	else
		CHECK;
/* Verify that event was not received for focus window. */
	XSync(client2, False);
	if (XCheckTypedWindowEvent(client2, child, event_send->type, &event_return) != False) {
		report("Focus window received event.", eventname(event_send->type));
		FAIL;
	}
	else
		CHECK;
	CHECKPASS(8);
>>ASSERTION Good B 3
When
.A w
is
.S InputFocus
and an inferior of the focus window does not contain the pointer,
then
the destination window is the focus window.
>>STRATEGY
Create window.
Select KeyPress-type events on new focus window.
Set input focus to new focus window.
Discard all events in the event queue.
Call XSendExtensionEvent to send a KeyPress event to inferior of the focus window.
Verify that XSendExtensionEvent returned non-zero.
Verify that event was received for inferior of focus window.
Verify that send_event was not set to False.
>>CODE
XVisualInfo *vp;
Window	nfocus;
XAnyEvent *event;
XEvent	event_return;
int	return_value;
int	dbp;
XEventClass dbpc;

/* Create window. */
	if (!Setup_Extension_DeviceInfo(BtnMask))
	    {
	    untested("%s: No input extension button device.\n", TestName);
	    return;
	    }
	device = Devs.Button;
	DeviceButtonPress(Devs.Button, dbp, dbpc);
	resetvinf(VI_WIN);
	nextvinf(&vp);
	nfocus = makewin(display, vp);
/* Select KeyPress-type events on new focus window. */
	XSelectExtensionEvent(display, nfocus, &dbpc, 1);
/* Set input focus to new focus window. */
	XSetDeviceFocus(display, device, nfocus, RevertToPointerRoot, CurrentTime);
/* Discard all events in the event queue. */
	XSync(display, True);
/* Call XSendExtensionEvent to send a KeyPress event to inferior of the focus window. */
	w = (Window) InputFocus;
	propagate = False;
	event_list = &dbpc;
	event_send->type = dbp;
	((XAnyEvent *) event_send)->window = nfocus;
	_xcall_(return_value);
/* Verify that XSendExtensionEvent returned non-zero. */
	if (return_value == 0) {
		report("Returned zero, expected non-zero.");
		FAIL;
	}
	else
		CHECK;
/* Verify that event was received for inferior of focus window. */
	XSync(display, False);
	if (XCheckTypedWindowEvent(display, nfocus, event_send->type, &event_return) == False) {
		report("Expected event (%s) not received.", eventname(event_send->type));
		FAIL;
	}
	else
		CHECK;
/* Verify that send_event was not set to False. */
	event = (XAnyEvent *) &event_return;
	if (event->send_event == False) {
		report("send_event not set to True");
		FAIL;
	}
	else
		CHECK;
	CHECKPASS(3);
>>ASSERTION def
When
.A event_list
is set to noextensioneventclass
then a call to xname results in
.A event_send
being sent to the client that created the destination window.
>>ASSERTION Good B 3
When
.A event_list
is set to noextensioneventclass
and the client that created the destination window
.A w
no longer exists,
then no event is sent.
>>STRATEGY
Create client2.
Call XSetCloseDownMode with RetainPermanent for client2.
Create window for client2.
Call XCloseDisplay for client2.
Select ALLEVENTS on window.
Call XSendExtensionEvent to send event to window.
Verify that XSendExtensionEvent returned non-zero.
Verify that no events were received.
>>CODE
Display	*client2;
int	return_value;
XID dbp, noextensionevent;
XEventClass dbpc, noextensioneventclass;

	if (!Setup_Extension_DeviceInfo(BtnMask))
	    {
	    untested("%s: No input extension button device.\n", TestName);
	    return;
	    }
	device = Devs.Button;
	NoExtensionEvent(device, noextensionevent, noextensioneventclass);
	DeviceButtonPress(Devs.Button, dbp, dbpc);
/* Create client2. */

	/* We must disable resource registration here to prevent attempted
	   deallocation of closed connections and deleted resources. */
	regdisable(); 

	client2 = opendisplay();
	if (client2 == (Display *) NULL) {
		delete("Can not open display");
		regenable();
		return;
	}
	else
		CHECK;
/* Call XSetCloseDownMode with RetainPermanent for client2. */
	XSetCloseDownMode(client2, RetainPermanent);
/* Create window for client2. */
	w = mkwin(client2, (XVisualInfo *) NULL, (struct area *) NULL, False);
/* Call XCloseDisplay for client2. */
	XCloseDisplay(client2);

	/* re-enable resource registration */
	regenable();

/* Select ALLEVENTS on window. */
	XSelectExtensionEvent(display, w, classes, nclass);
/* Call XSendExtensionEvent to send event to window. */
	propagate = False;
	event_list = &noextensioneventclass;
	event_send->type = dbp;
	_xcall_(return_value);
/* Verify that XSendExtensionEvent returned non-zero. */
	XSync(display, False);
	if (return_value == 0) {
		report("Returned zero, expected non-zero.");
		FAIL;
	}
	else
		CHECK;
/* Verify that no events were received. */
	if (XPending(display) > 0) {
		XEvent	event;

		XNextEvent(display, &event);
		report("Event(s) delivered unexpectedly (%s)", eventname(event.type));
		FAIL;
	}
	else
		CHECK;
	CHECKPASS(3);
>>ASSERTION Good B 3
When
.A event_list
is not set to noextensioneventclass
and no clients have selected on the destination window
and
.A propagate
is
.S False ,
then
a call to xname
results in no event being sent.
>>STRATEGY
Create window.
Select no events on window.
Set propagate to False.
Set event_list to something other than noextensioneventclass.
Discard all events in the event queue.
Call XSendExtensionEvent.
Verify that XSendExtensionEvent returned non-zero.
Verify that no events were received.
>>CODE
XVisualInfo *vp;
XEvent	event_return;
int	return_value;
XID dbp, noextensionevent;
XEventClass dbpc, noextensioneventclass;

/* Create window. */
	if (!Setup_Extension_DeviceInfo(BtnMask))
	    {
	    untested("%s: No input extension button device.\n", TestName);
	    return;
	    }
	device = Devs.Button;
	NoExtensionEvent(device, noextensionevent, noextensioneventclass);
	DeviceButtonPress(device, dbp, dbpc);
	resetvinf(VI_WIN);
	nextvinf(&vp);
	w = mkwin(display, (XVisualInfo *) NULL, (struct area *) NULL, False);
/* Select no events on window. */
	XSelectExtensionEvent(display, w, &noextensioneventclass, 1);
	XSync(display, True);
/* Set propagate to False. */
	propagate = False;
/* Set event_list to something other than NULL */
	count = 1;
	event_list = &dbpc;
/* Discard all events in the event queue. */
	XSync(display, True);
/* Call XSendExtensionEvent. */
	event_send->type = dbp;
	((XAnyEvent *) event_send)->window = w;
	_xcall_(return_value);
/* Verify that XSendExtensionEvent returned non-zero. */
	if (return_value == 0) {
		report("Returned zero, expected non-zero.");
		FAIL;
	}
	else
		CHECK;
/* Verify that no events were received. */
	XSync(display, False);
	if (XCheckTypedWindowEvent(display, w, event_send->type, &event_return) != False) {
		report("Unexpected event (%s) received.", eventname(event_send->type));
		FAIL;
	}
	else
		CHECK;
	CHECKPASS(2);
>>ASSERTION Good B 3
When
.A event_list
is not set to noextensioneventclass
and no clients have selected on the destination
.A w
and
.A propagate
is
.S True
and there is no matching ancestor of the destination
.A w
for which no intervening window has that type in its
do-not-propagate-mask,
then
a call to xname
results in no event being sent.
>>STRATEGY
Create a window hierarchy.
Create a hierarchy member with KeyPressMask set in do_not_propagate_mask.
Create a child of this member without setting do_not_propagate_mask.
Create the hierarchy.
Select no events on the destination w.
Select for KeyPress on the grandparent window of this member.
Set w to the window corresponding to child node.
Set propagate to True.
Set event_list to KeyPressMask.
Set event type to KeyPress.
Call XSendExtensionEvent.
Verify that XSendExtensionEvent returned non-zero.
Verify that no events were received.
Select for no events on the grandparent window of this member.
Call XSendExtensionEvent.
Verify that XSendExtensionEvent returned non-zero.
Verify that no events were received.
Change the do_not_propagate_mask from KeyPressMask to noextensioneventclass.
Call XSendExtensionEvent.
Verify that XSendExtensionEvent returned non-zero.
Verify that no events were received.
>>CODE
Winh	*grandparent;
Winh	*parent;
Winh	*child;
XSetWindowAttributes attrs;
int	return_value;
XID dbp, noextensionevent;
XEventClass dbpc, noextensioneventclass;

/* Create a window hierarchy. */
	if (!Setup_Extension_DeviceInfo(BtnMask))
	    {
	    untested("%s: No input extension button device.\n", TestName);
	    return;
	    }
	device = Devs.Button;
	DeviceButtonPress(Devs.Button, dbp, dbpc);
	NoExtensionEvent(device, noextensionevent, noextensioneventclass);
	grandparent = winh_adopt(display, (Winh *) NULL, 0L, (XSetWindowAttributes *) NULL, (Winhg *) NULL, WINH_NOMASK);
	if (grandparent == (Winh *) NULL) {
		delete("Could not create grandparent");
		return;
	}
	else
		CHECK;
/* Create a hierarchy member with KeyPressMask set in do_not_propagate_mask. */
	attrs.do_not_propagate_mask = KeyPressMask;
	parent = winh_adopt(display, grandparent, CWDontPropagate, &attrs, (Winhg *) NULL, WINH_NOMASK);
	if (parent == (Winh *) NULL) {
		delete("Could not create parent");
		return;
	}
	else
		CHECK;
/* Create a child of this member without setting do_not_propagate_mask. */
	child = winh_adopt(display, parent, 0L, (XSetWindowAttributes *) NULL, (Winhg *) NULL, WINH_NOMASK);
	if (child == (Winh *) NULL) {
		delete("Could not create child");
		return;
	}
	else
		CHECK;
/* Create the hierarchy. */
	if (winh_create(display, (Winh *) NULL, WINH_NOMASK)) {
		return;
	}
	else
		CHECK;
	XSync(display, True);
/* Select no events on the destination w. */
	XChangeDeviceDontPropagateList (display, parent->window, 1, &dbpc, AddToList);
	if (XSelectExtensionEvent(display, child->window, &noextensioneventclass, 1))
		return;
	else
		CHECK;
/* Select no events on the parent of w. */
	if (XSelectExtensionEvent(display, parent->window, &noextensioneventclass, 1))
		return;
	else
		CHECK;
/* Select for devicebuttonPress on the grandparent window of this member. */
	if (XSelectExtensionEvent(display, grandparent->window, &dbpc, 1))
		return;
	else
		CHECK;
	XSync(display, True);
/* Set w to the window corresponding to child node. */
	w = child->window;
/* Set propagate to True. */
	propagate = True;
/* Set event_list to KeyPressMask. */
	event_list = &dbpc;
	count = 1;
/* Set event type to KeyPress. */
	event_send->type = dbp;
/* Call XSendExtensionEvent. */
	_xcall_(return_value);
	XSync(display, False);
/* Verify that XSendExtensionEvent returned non-zero. */
	if (return_value == 0) {
		report("Returned zero, expected non-zero.");
		FAIL;
	}
	else
		CHECK;
/* Verify that no events were received. */
	if (XPending(display) > 0) {
		XEvent	event;

		XNextEvent(display, &event);
		report("Event(s) delivered unexpectedly (%s) through do_not_propagate_mask", eventname(event.type));
		FAIL;
	}
	else
		CHECK;
/* Select for no events on the grandparent window of this member. */
	if (XSelectExtensionEvent(display, grandparent->window, &noextensioneventclass, 1))
		return;
	else
		CHECK;
	XSync(display, True);
/* Call XSendExtensionEvent. */
	_xcall_(return_value);
	XSync(display, False);
/* Verify that XSendExtensionEvent returned non-zero. */
	if (return_value == 0) {
		report("Returned zero, expected non-zero.");
		FAIL;
	}
	else
		CHECK;
/* Verify that no events were received. */
	if (XPending(display) > 0) {
		XEvent	event;

		XNextEvent(display, &event);
		report("Event(s) delivered unexpectedly (%s) while none selected and do_not_propagate_mask on", eventname(event.type));
		FAIL;
	}
	else
		CHECK;
/* Change the do_not_propagate_mask from KeyPressMask to noextensioneventclass.*/
	if (XChangeDeviceDontPropagateList (display, parent->window, 1, &dbpc, DeleteFromList))
		return;
	else
		CHECK;
	XSync(display, True);
/* Call XSendExtensionEvent. */
	_xcall_(return_value);
	XSync(display, False);
/* Verify that XSendExtensionEvent returned non-zero. */
	if (return_value == 0) {
		report("Returned zero, expected non-zero.");
		FAIL;
	}
	else
		CHECK;
/* Verify that no events were received. */
	if (XPending(display) > 0) {
		XEvent	event;

		XNextEvent(display, &event);
		report("Event(s) delivered unexpectedly (%s) with none selected", eventname(event.type));
		FAIL;
	}
	else
		CHECK;
	CHECKPASS(15);
>>ASSERTION Good B 3
When
.A event_list
is not set to noextensioneventclass
and
.A w
is set to
.S InputFocus
and an inferior of the focus window contains the pointer
and no clients have selected on that inferior,
then
a call to xname
results in no event being sent.
>>STRATEGY
Create client2.
Create parent window.
Create inferior window.
Select KeyPress-type events on parent window with client2.
Flush client2 requests.
Select no events on child window.
Grab server.
Enable synchronization.
Set input focus to parent window.
Save initial pointer location.
Warp pointer to inferior of focus window.
Get new pointer location.
Discard all events in the event queue.
Call XSendExtensionEvent to send a KeyPress event to inferior of the focus window.
Get current pointer location.
Check to see if pointer moved.
Warp pointer back to where it started.
Disable synchronization.
Ungrab server.
Verify that XSendExtensionEvent returned non-zero.
Verify that no event was received for focus window.
Verify that event was not received for client2.
>>CODE
Display	*client2;
XVisualInfo *vp;
Window	parent;
Window	child;
int	return_value;
int root_x, root_y;	/* pointer location after XSendExtensionEvent */
int oroot_x, oroot_y;	/* pointer location before XSendExtensionEvent */
int sroot_x, sroot_y;	/* initial pointer location */
Window oldroot;		/* initial root window */
int itmp;		/* useless XQueryPointer return values */
unsigned int uitmp;	/* useless XQueryPointer return values */
Window wtmp;		/* useless XQueryPointer return values */
unsigned int width;
unsigned int height;
struct area a;
XID noextensionevent, dbp;
XEventClass noextensioneventclass, dbpc;

/* Create client2. */
	if (!Setup_Extension_DeviceInfo(BtnMask))
	    {
	    untested("%s: No input extension button device.\n", TestName);
	    return;
	    }
	device = Devs.Button;
	DeviceButtonPress(device, dbp, dbpc);
	NoExtensionEvent(device, noextensionevent, noextensioneventclass);
	client2 = opendisplay();
	if (client2 == (Display *) NULL) {
		delete("Can't create new client.");
		return;
	}
	else
		CHECK;
/* Create parent window. */
	resetvinf(VI_WIN);
	nextvinf(&vp);
	parent = makewin(display, vp);
/* Create inferior window. */
	getsize(display, (Drawable) parent, &width, &height);
	a.x = 0;
	a.y = 0;
	a.width = width/2;
	a.height = height/2;
	child = crechild(display, parent, &a);
/* Select KeyPress-type events on parent window with client2. */
	XSelectExtensionEvent(client2, parent, &noextensioneventclass, 1);
/* Flush client2 requests. */
	XFlush(client2);
/* Select no events on child window. */
	XSelectExtensionEvent(display, child, &noextensioneventclass, 1);
/* Grab server. */
	XGrabServer(display);
/* Enable synchronization. */
	(void) XSynchronize(display, True);
/* Set input focus to parent window. */
	XSetDeviceFocus(display, Devs.Button, parent, RevertToPointerRoot, CurrentTime);
/* Save initial pointer location. */
	(void) XQueryPointer(display, child, &oldroot, &wtmp, &sroot_x, &sroot_y, &itmp, &itmp, &uitmp);
/* Warp pointer to inferior of focus window. */
	XWarpPointer(display, None, child, 0, 0, 0, 0, 0, 0);

/* Get new pointer location. */
	if (XQueryPointer(display, child, &wtmp, &wtmp, &oroot_x, &oroot_y, &itmp, &itmp, &uitmp) == False) {
		delete("Pointer on wrong root.");
		(void) XSynchronize(display, False);
		XUngrabServer(display);
		return;
	} else
		CHECK;
/* Discard all events in the event queue. */
	XSync(display, True);
/* Call XSendExtensionEvent to send a KeyPress event to inferior of the focus window. */
	w = (Window) InputFocus;
	propagate = False;
	count = 1;
	event_list = &dbpc;
	event_send->type = dbp;
	((XAnyEvent *) event_send)->window = child;
	_xcall_(return_value);
/* Get current pointer location. */
	if (XQueryPointer(display, child, &wtmp, &wtmp, &root_x, &root_y, &itmp, &itmp, &uitmp) == False) {
		delete("Pointer moved.");
		(void) XSynchronize(display, False);
		XUngrabServer(display);
		return;
	}
	else
		CHECK;
/* Check to see if pointer moved. */
	if (oroot_x != root_x || oroot_y != root_y) {
		delete("Pointer moved.");
		/*
		 * if this is ever changed to be a return, take care to
		 * also turn off synchronization and ungrab the server...
		 */
	}
	else
		CHECK;
/* Warp pointer back to where it started. */
	XWarpPointer(display, None, oldroot, 0, 0, 0, 0, sroot_x, sroot_y);
/* Disable synchronization. */
	(void) XSynchronize(display, False);
/* Ungrab server. */
	XUngrabServer(display);
/* Verify that XSendExtensionEvent returned non-zero. */
	if (return_value == 0) {
		report("Returned zero, expected non-zero.");
		FAIL;
	}
	else
		CHECK;
	XSync(display, False);
	XSync(client2, False);
/* Verify that no event was received for focus window. */
	if (XPending(display) > 0) {
		XEvent	event;

		XNextEvent(display, &event);
		report("Event(s) delivered unexpectedly (%s) to focus window", eventname(event.type));
		FAIL;
	}
	else
		CHECK;
/* Verify that event was not received for client2. */
	if (XPending(client2) > 0) {
		XEvent	event;

		XNextEvent(client2, &event);
		report("Event(s) delivered unexpectedly (%s)", eventname(event.type));
		FAIL;
	}
	else
		CHECK;
	CHECKPASS(7);
>>ASSERTION Good B 3
When
.A event_list
is not set to noextensioneventclass
and
.A w
is set to
.S InputFocus
and an inferior of the focus window does not contain the pointer
and no clients have selected on the focus window,
then
a call to xname
results in no event being sent.
>>STRATEGY
Create window.
Grab server.
Enable synchronization.
Set input focus to new focus window.
Save initial pointer location.
Warp pointer away from inferior of focus window; to root.
Get new pointer location.
Select no events on new focus window.
Discard all events in the event queue.
Call xname with propagate == False, w == InputFocus & event_list == 
devicebuttonpressclass.
Get current pointer location.
Check to see if pointer moved.
Warp pointer back to where it started.
Disable synchronization.
Ungrab server.
Verify that XSendExtensionEvent returned non-zero.
Verify that no event was received.
Grab server.
Enable synchronization.
Set input focus to new focus window.
Save initial pointer location.
Warp pointer away from inferior of focus window; to root.
Get new pointer location.
Call XSendExtensionEvent again with propagate set to True.
Get current pointer location.
Check to see if pointer moved.
Warp pointer back to where it started.
Disable synchronization.
Ungrab server.
Verify that XSendExtensionEvent returned non-zero.
Verify that no event was received.
>>CODE
XVisualInfo *vp;
Window	nfocus;
XEvent	event_return;
int	return_value;
int root_x, root_y;	/* pointer location after XSendExtensionEvent */
int oroot_x, oroot_y;	/* pointer location before XSendExtensionEvent */
int sroot_x, sroot_y;	/* initial pointer location */
Window oldroot;		/* initial root window */
int itmp;		/* useless XQueryPointer return values */
unsigned int uitmp;	/* useless XQueryPointer return values */
Window wtmp;		/* useless XQueryPointer return values */
XID noextensionevent, dbp;
XEventClass noextensioneventclass, dbpc;

/* Create window. */
	if (!Setup_Extension_DeviceInfo(BtnMask))
	    {
	    untested("%s: No input extension button device.\n", TestName);
	    return;
	    }
	device = Devs.Button;
	DeviceButtonPress(device, dbp, dbpc);
	NoExtensionEvent(device, noextensionevent, noextensioneventclass);
	resetvinf(VI_WIN);
	nextvinf(&vp);
	nfocus = makewin(display, vp);
/* Grab server. */
	XGrabServer(display);
/* Enable synchronization. */
	(void) XSynchronize(display, True);
/* Set input focus to new focus window. */
	XSetInputFocus(display, nfocus, RevertToPointerRoot, CurrentTime);
/* Save initial pointer location. */
	(void) XQueryPointer(display, nfocus, &oldroot, &wtmp, &sroot_x, &sroot_y, &itmp, &itmp, &uitmp);
/* Warp pointer away from inferior of focus window; to root. */
	XWarpPointer(display, None, DRW(display), 0, 0, 0, 0, 0, 0);

/* Get new pointer location. */
	if (XQueryPointer(display, nfocus, &wtmp, &wtmp, &oroot_x, &oroot_y, &itmp, &itmp, &uitmp) == False) {
		delete("Pointer on wrong root.");
		(void) XSynchronize(display, False);
		XUngrabServer(display);
		return;
	} else
		CHECK;
/* Select no events on new focus window. */
	XSelectExtensionEvent(display, nfocus, &noextensioneventclass, 1);
/* Discard all events in the event queue. */
	XSync(display, True);
	w = (Window) InputFocus;
	event_list = &dbpc;
	event_send->type = dbp;
	((XAnyEvent *) event_send)->window = nfocus;
	propagate = False;
/* Call xname with propagate == False, w == InputFocus & event_list == 
   devicebuttonpressclass. */
	_xcall_(return_value);
/* Get current pointer location. */
	if (XQueryPointer(display, nfocus, &wtmp, &wtmp, &root_x, &root_y, &itmp, &itmp, &uitmp) == False) {
		delete("Pointer moved.");
		(void) XSynchronize(display, False);
		XUngrabServer(display);
		return;
	}
	else
		CHECK;
/* Check to see if pointer moved. */
	if (oroot_x != root_x || oroot_y != root_y) {
		delete("Pointer moved.");
		/*
		 * if this is ever changed to be a return, take care to
		 * also turn off synchronization and ungrab the server...
		 */
	}
	else
		CHECK;
/* Warp pointer back to where it started. */
	XWarpPointer(display, None, oldroot, 0, 0, 0, 0, sroot_x, sroot_y);
/* Disable synchronization. */
	(void) XSynchronize(display, False);
/* Ungrab server. */
	XUngrabServer(display);
/* Verify that XSendExtensionEvent returned non-zero. */
	if (return_value == 0) {
		report("Returned zero, expected non-zero.");
		FAIL;
	}
	else
		CHECK;
	XSync(display, False);
/* Verify that no event was received. */
	XSync(display, False);
	if (XPending(display) > 0) {
		XEvent	event;

		XNextEvent(display, &event);
		report("Event(s) delivered unexpectedly (%s)", eventname(event.type));
		FAIL;
	}
	else
		CHECK;
/* Grab server. */
	XGrabServer(display);
/* Enable synchronization. */
	(void) XSynchronize(display, True);
/* Set input focus to new focus window. */
	XSetInputFocus(display, nfocus, RevertToPointerRoot, CurrentTime);
/* Save initial pointer location. */
	(void) XQueryPointer(display, nfocus, &oldroot, &wtmp, &sroot_x, &sroot_y, &itmp, &itmp, &uitmp);
/* Warp pointer away from inferior of focus window; to root. */
	XWarpPointer(display, None, DRW(display), 0, 0, 0, 0, 0, 0);

/* Get new pointer location. */
	if (XQueryPointer(display, nfocus, &wtmp, &wtmp, &oroot_x, &oroot_y, &itmp, &itmp, &uitmp) == False) {
		delete("Pointer on wrong root.");
		(void) XSynchronize(display, False);
		XUngrabServer(display);
		return;
	} else
		CHECK;
/* Call XSendExtensionEvent again with propagate set to True. */
	propagate = True;
	_xcall_(return_value);
/* Get current pointer location. */
	if (XQueryPointer(display, nfocus, &wtmp, &wtmp, &root_x, &root_y, &itmp, &itmp, &uitmp) == False) {
		delete("Pointer moved.");
		(void) XSynchronize(display, False);
		XUngrabServer(display);
		return;
	}
	else
		CHECK;
/* Check to see if pointer moved. */
	if (oroot_x != root_x || oroot_y != root_y) {
		delete("Pointer moved.");
		/*
		 * if this is ever changed to be a return, take care to
		 * also turn off synchronization and ungrab the server...
		 */
	}
	else
		CHECK;
/* Warp pointer back to where it started. */
	XWarpPointer(display, None, oldroot, 0, 0, 0, 0, sroot_x, sroot_y);
/* Disable synchronization. */
	(void) XSynchronize(display, False);
/* Ungrab server. */
	XUngrabServer(display);
/* Verify that XSendExtensionEvent returned non-zero. */
	if (return_value == 0) {
		report("Returned zero, expected non-zero.");
		FAIL;
	}
	else
		CHECK;
	XSync(display, False);
/* Verify that no event was received. */
	XSync(display, False);
	if (XPending(display) > 0) {
		XEvent	event;

		XNextEvent(display, &event);
		report("Event(s) delivered unexpectedly (%s)", eventname(event.type));
		FAIL;
	}
	else
		CHECK;
	CHECKPASS(10);
>>ASSERTION Good B 3
When
.A event_list
is not set to noextensioneventclass
and no clients have selected on the destination
.A w
and
.A propagate
is
.S True
and there is a matching ancestor of the destination
.A w
for which no intervening window has that type in its
do-not-propagate-mask
and that ancestor is an ancestor of the focus window
and
.S InputFocus
was not specified as the destination,
then
a call to xname
results in the event being sent to all clients selecting
a type in
.A event_list
on the first matching ancestor.
>>STRATEGY
Set event_list to devicebuttonpressclass;
Set propagate to True.
Create window hierarchy with depth of three and with all windows
have no bits set in their do-not-propagate-mask.
Set focus window to window mid-level in window hierarchy.
Set w to window at bottom of window hierarchy.
Create client2.
Select no events on w.
Select no events on w with client2.
Select KeyPress events on top-level window in window hierarchy.
Select KeyPress events on top-level window in window hierarchy
with client2.
Call XSendExtensionEvent to send a KeyPress event to w.
Verify that XSendExtensionEvent returned non-zero.
Verify that event was received for top-level window in window hierarchy.
Verify that send_event was not set to False.
Verify that event was received for top-level window in window hierarchy
for client2.
Verify that send_event was not set to False for client2.
>>CODE
Display	*client2;
Winh	*grandparent;
Winh	*parent;
Winh	*child;
int	return_value;
XEvent	event_return;
XAnyEvent *event;
XID noextensionevent, dbp;
XEventClass noextensioneventclass, dbpc;

/* Set event_list to devicebuttonpressclass. */
	if (!Setup_Extension_DeviceInfo(BtnMask))
	    {
	    untested("%s: No input extension button device.\n", TestName);
	    return;
	    }
	device = Devs.Button;
	DeviceButtonPress(device, dbp, dbpc);
	NoExtensionEvent(device, noextensionevent, noextensioneventclass);
	event_list = &dbpc;
/* Set propagate to True. */
	propagate = True;
/* Create window hierarchy with depth of three and with all windows */
/* have no bits set in their do-not-propagate-mask. */
	grandparent = winh_adopt(display, (Winh *) NULL, 0L, (XSetWindowAttributes *) NULL, (Winhg *) NULL, WINH_NOMASK);
	if (grandparent == (Winh *) NULL) {
		delete("Could not create grandparent");
		return;
	}
	else
		CHECK;
	parent = winh_adopt(display, grandparent, 0L, (XSetWindowAttributes *) NULL, (Winhg *) NULL, WINH_NOMASK);
	if (parent == (Winh *) NULL) {
		delete("Could not create parent");
		return;
	}
	else
		CHECK;
	child = winh_adopt(display, parent, 0L, (XSetWindowAttributes *) NULL, (Winhg *) NULL, WINH_NOMASK);
	if (child == (Winh *) NULL) {
		delete("Could not create child");
		return;
	}
	else
		CHECK;
	if (winh_create(display, (Winh *) NULL, WINH_MAP)) {
		return;
	}
	else
		CHECK;
/* Set focus window to window mid-level in window hierarchy. */
	XSetInputFocus(display, parent->window, RevertToPointerRoot, CurrentTime);
/* Set w to window at bottom of window hierarchy. */
	w = child->window;
/* Create client2. */
	client2 = opendisplay();
	if (client2 == (Display *) NULL) {
		delete("Can't create new client.");
		return;
	}
	else
		CHECK;
/* Select no events on w. */
	if(XSelectExtensionEvent(display, child->window, &noextensioneventclass, 1)){
		return;
	}
	else
		CHECK;
/* Select no events on w with client2. */
	if(XSelectExtensionEvent(client2, child->window, &noextensioneventclass, 1)){
		return;
	}
	else
		CHECK;
/* Select KeyPress events on top-level window in window hierarchy. */
	if(XSelectExtensionEvent(display, grandparent->window, &dbpc, 1)){
		return;
	}
	else
		CHECK;
/* Select KeyPress events on top-level window in window hierarchy */
/* with client2. */
	if(XSelectExtensionEvent(client2, grandparent->window, &dbpc, 1)){
		return;
	}
	else
		CHECK;
	XSync(display, True);
	XSync(client2, True);
/* Call XSendExtensionEvent to send a KeyPress event to w. */
	count = 1;
	event_list = &dbpc;
	event_send->type = dbp;
	((XAnyEvent *) event_send)->window = grandparent->window;
	_xcall_(return_value);
/* Verify that XSendExtensionEvent returned non-zero. */
	if (return_value == 0) {
		report("Returned zero, expected non-zero.");
		FAIL;
	}
	else
		CHECK;
/* Verify that event was received for top-level window in window hierarchy. */
	XSync(display, False);
	if (XCheckTypedWindowEvent(display, grandparent->window, event_send->type, &event_return) != True) {
		report("Expected event (%s) not received.", eventname(event_send->type));
		FAIL;
	}
	else
		CHECK;
/* Verify that send_event was not set to False. */
	event = (XAnyEvent *) &event_return;
	if (event->send_event == False) {
		report("send_event not set to True");
		FAIL;
	}
	else
		CHECK;
/* Verify that event was received for top-level window in window hierarchy */
/* for client2. */
	XSync(client2, False);
	if (XCheckTypedWindowEvent(client2, grandparent->window, event_send->type, &event_return) != True) {
		report("Expected event (%s) not received.", eventname(event_send->type));
		FAIL;
	}
	else
		CHECK;
/* Verify that send_event was not set to False for client2. */
	event = (XAnyEvent *) &event_return;
	if (event->send_event == False) {
		report("send_event not set to True");
		FAIL;
	}
	else
		CHECK;
	CHECKPASS(14);
>>ASSERTION Good B 3
When
.A event_list
is not set to noextensioneventclass
and no clients have selected on the destination
.A w
and
.A propagate
is
.S True
and there is a matching ancestor of the destination
.A w
for which no intervening window has that type in its
do-not-propagate-mask
and this ancestor is not an ancestor of the focus window,
then
a call to xname
results in the event being sent to all clients selecting
a type in
.A event_list
on the first matching ancestor.
>>STRATEGY
Set event_list to devicebuttonpressclass.
Set propagate to True.
Create window hierarchy with depth of three and with all windows
have no bits set in their do-not-propagate-mask.
Set focus window to top window in window hierarchy.
Set w to window at bottom of window hierarchy.
Create client2.
Select no events on w.
Select no events on w with client2.
Select KeyPress events on mid-level window in window hierarchy.
Select KeyPress events on mid-level window in window hierarchy
with client2.
Call XSendExtensionEvent to send a KeyPress event to w.
Verify that XSendExtensionEvent returned non-zero.
Verify that event was received for mid-level window in window hierarchy.
Verify that send_event was not set to False.
Verify that event was received for mid-level window in window hierarchy
for client2.
Verify that send_event was not set to False for client2.
>>CODE
Display	*client2;
Winh	*grandparent;
Winh	*parent;
Winh	*child;
int	return_value;
XEvent	event_return;
XAnyEvent *event;
XID noextensionevent, dbp;
XEventClass noextensioneventclass, dbpc;

/* Set event_list to devicebuttonpressclass.*/
	if (!Setup_Extension_DeviceInfo(BtnMask))
	    {
	    untested("%s: No input extension button device.\n", TestName);
	    return;
	    }
	device = Devs.Button;
	DeviceButtonPress(device, dbp, dbpc);
	NoExtensionEvent(device, noextensionevent, noextensioneventclass);
	count = 1;
	event_list = &dbpc;
/* Set propagate to True. */
	propagate = True;
/* Create window hierarchy with depth of three and with all windows */
/* have no bits set in their do-not-propagate-mask. */
	grandparent = winh_adopt(display, (Winh *) NULL, 0L, (XSetWindowAttributes *) NULL, (Winhg *) NULL, WINH_NOMASK);
	if (grandparent == (Winh *) NULL) {
		delete("Could not create grandparent");
		return;
	}
	else
		CHECK;
	parent = winh_adopt(display, grandparent, 0L, (XSetWindowAttributes *) NULL, (Winhg *) NULL, WINH_NOMASK);
	if (parent == (Winh *) NULL) {
		delete("Could not create parent");
		return;
	}
	else
		CHECK;
	child = winh_adopt(display, parent, 0L, (XSetWindowAttributes *) NULL, (Winhg *) NULL, WINH_NOMASK);
	if (child == (Winh *) NULL) {
		delete("Could not create child");
		return;
	}
	else
		CHECK;
	if (winh_create(display, (Winh *) NULL, WINH_MAP)) {
		return;
	}
	else
		CHECK;
/* Set focus window to top window in window hierarchy. */
	XSetDeviceFocus(display, Devs.Button, grandparent->window, RevertToPointerRoot, CurrentTime);
/* Set w to window at bottom of window hierarchy. */
	w = child->window;
/* Create client2. */
	client2 = opendisplay();
	if (client2 == (Display *) NULL) {
		delete("Can't create new client.");
		return;
	}
	else
		CHECK;
/* Select no events on w. */
	if(XSelectExtensionEvent(display, child->window, &noextensioneventclass, 1)){
		return;
	}
	else
		CHECK;
/* Select no events on w with client2. */
	if(XSelectExtensionEvent(client2, child->window, &noextensioneventclass, 1)){
		return;
	}
	else
		CHECK;
/* Select KeyPress events on mid-level window in window hierarchy. */
	if(XSelectExtensionEvent(display, parent->window, &dbpc, 1)){
		return;
	}
	else
		CHECK;
/* Select KeyPress events on mid-level window in window hierarchy */
/* with client2. */
	if(XSelectExtensionEvent(client2, parent->window, &dbpc, 1)){
		return;
	}
	else
		CHECK;
	XSync(display, True);
	XSync(client2, True);
/* Call XSendExtensionEvent to send a KeyPress event to w. */
	count = 1;
	event_list = &dbpc;
	event_send->type = dbp;
	((XAnyEvent *) event_send)->window = parent->window;
	_xcall_(return_value);
/* Verify that XSendExtensionEvent returned non-zero. */
	if (return_value == 0) {
		report("Returned zero, expected non-zero.");
		FAIL;
	}
	else
		CHECK;
/* Verify that event was received for mid-level window in window hierarchy. */
	XSync(display, False);
	if (XCheckTypedWindowEvent(display, parent->window, event_send->type, &event_return) != True) {
		report("Expected event (%s) not received.", eventname(event_send->type));
		FAIL;
	}
	else
		CHECK;
/* Verify that send_event was not set to False. */
	event = (XAnyEvent *) &event_return;
	if (event->send_event == False) {
		report("send_event not set to True");
		FAIL;
	}
	else
		CHECK;
/* Verify that event was received for mid-level window in window hierarchy */
/* for client2. */
	XSync(client2, False);
	if (XCheckTypedWindowEvent(client2, parent->window, event_send->type, &event_return) != True) {
		report("Expected event (%s) not received.", eventname(event_send->type));
		FAIL;
	}
	else
		CHECK;
/* Verify that send_event was not set to False for client2. */
	event = (XAnyEvent *) &event_return;
	if (event->send_event == False) {
		report("send_event not set to True");
		FAIL;
	}
	else
		CHECK;
	CHECKPASS(14);
>>ASSERTION Good B 3
A call to xname
ignores active grabs.
>>STRATEGY
Create window.
Select ALLEVENTS on this window.
Set propagate to False.
Create client2.
Create window with client2.
Grab the pointer with client2 using client2's window as the grab window,
owner_events set to False, event_list set to PointerMotionMask,
pointer_mode set to GrabModeSync, and keyboard_mode set to GrabModeSync.
Verify that XGrabPointer returned GrabSuccess.
Grab AnyButton with client2 using client2's window as the grab window,
owner_events set to False, event_list set to ButtonPressMask,
pointer_mode set to GrabModeSync, and keyboard_mode set to GrabModeSync.
Grab the keyboard with client2 using client2's window as the grab window,
owner_events set to False, pointer_mode set to GrabModeSync,
and keyboard_mode set to GrabModeSync.
Verify that XGrabKeyboard returned GrabSuccess.
Set event_list to PointerMotionMask.
Flush display, discarding events.
Flush client2, discarding events.
Call XSendExtensionEvent to send a MotionNotify event.
Verify that XSendExtensionEvent returned non-zero.
Flush display, not discarding events.
Flush client2, not discarding events.
Verify that event was received.
Verify that send_event was not set to False.
Verify that client2 received no events.
Set event_list to ButtonPressMask.
Flush display, discarding events.
Flush client2, discarding events.
Call XSendExtensionEvent to send a ButtonPress event.
Verify that XSendExtensionEvent returned non-zero.
Flush display, not discarding events.
Flush client2, not discarding events.
Verify that event was received.
Verify that send_event was not set to False.
Verify that client2 received no events.
Set event_list to KeyPressMask.
Flush display, discarding events.
Flush client2, discarding events.
Call XSendExtensionEvent to send a KeyPress event.
Flush display, not discarding events.
Flush client2, not discarding events.
Verify that XSendExtensionEvent returned non-zero.
Verify that event was received.
Verify that send_event was not set to False.
Verify that client2 received no events.
Ungrab the keyboard with client2.
Ungrab buttons with client2.
Ungrab the pointer with client2.
>>CODE
Display	*client2;
Window	w2;
int	status;
XAnyEvent *event;
XEvent	event_return;
int	return_value;
XID devicemotionnotify, dbp;
XEventClass devicemotionnotifyclass, dbpc;
XEventClass classes[2];

/* Create window. */
	if (!Setup_Extension_DeviceInfo(BtnMask))
	    {
	    untested("%s: No input extension button device.\n", TestName);
	    return;
	    }
	device = Devs.Button;
	DeviceButtonPress(device, dbp, dbpc);
	w = mkwin(display, (XVisualInfo *) NULL, (struct area *) NULL, False);
	((XAnyEvent *) event_send)->window = w;
/* Select ALLEVENTS on this window. */
	DeviceMotionNotify(device,devicemotionnotify, devicemotionnotifyclass);
	classes[0] = devicemotionnotifyclass;
	classes[1] = dbpc;
	XSelectExtensionEvent(display, w, classes, 2);
/* Set propagate to False. */
	propagate = False;
/* Create client2. */
	client2 = opendisplay();
	if (client2 == (Display *) NULL) {
		delete("Can't create new client.");
		return;
	}
	else
		CHECK;
/* Create window with client2. */
	w2 = mkwin(client2, (XVisualInfo *) NULL, (struct area *) NULL, True);
/* Grab the pointer with client2 using */
/* client2's window as the grab window, */
/* owner_events set to False, */
/* event_list set to PointerMotionMask, */
/* pointer_mode set to GrabModeSync, */
/* and keyboard_mode set to GrabModeSync. */
	status = XGrabPointer(client2, w2, False, PointerMotionMask, GrabModeSync, GrabModeSync, None, None, CurrentTime);
/* Verify that XGrabPointer returned GrabSuccess. */
	if (status != GrabSuccess) {
		delete("Can't grab pointer (%d).", status);
		return;
	}
	else
		CHECK;
/* Grab AnyButton with client2 using */
/* client2's window as the grab window, */
/* owner_events set to False, */
/* event_list set to ButtonPressMask, */
/* pointer_mode set to GrabModeSync, */
/* and keyboard_mode set to GrabModeSync. */
	XGrabButton(client2, AnyButton, AnyModifier, w2, False, ButtonPressMask, GrabModeSync, GrabModeSync, None, None);
/* Grab the keyboard with client2 using */
/* client2's window as the grab window, */
/* owner_events set to False, */
/* pointer_mode set to GrabModeSync, */
/* and keyboard_mode set to GrabModeSync. */
	status = XGrabKeyboard(client2, w2, False, GrabModeSync, GrabModeSync, CurrentTime);
/* Verify that XGrabKeyboard returned GrabSuccess. */
	if (status != GrabSuccess) {
		delete("Can't grab keyboard (%d).", status);
		return;
	}
	else
		CHECK;
/* Set event_mask to PointerMotionMask. */
	event_list = &devicemotionnotifyclass;
	event_send->type = devicemotionnotify;
/* Flush display, discarding events. */
	XSync(display, True);
/* Flush client2, discarding events. */
	XSync(client2, True);
/* Call XSendExtensionEvent to send a MotionNotify event. */
	_xcall_(return_value);
/* Verify that XSendExtensionEvent returned non-zero. */
	if (return_value == 0) {
		report("Returned zero, expected non-zero.");
		FAIL;
	}
	else
		CHECK;
/* Flush display, not discarding events. */
	XSync(display, False);
/* Flush client2, not discarding events. */
	XSync(client2, False);
/* Verify that event was received. */
	if (XCheckTypedWindowEvent(display, w, event_send->type, &event_return) == False) {
		report("Expected event (%s) not received.", eventname(event_send->type));
		FAIL;
	}
	else
		CHECK;
/* Verify that send_event was not set to False. */
	event = (XAnyEvent *) &event_return;
	if (event->send_event == False) {
		report("send_event not set to True");
		FAIL;
	}
	else
		CHECK;
/* Verify that client2 received no events. */
	status = XPending(client2);
	if (status != 0) {
		report("Client2 received %d events, expected %d", status, 0);
		FAIL;
	}
	else
		CHECK;
/* Set event_mask to ButtonPressMask. */
	event_list = &dbpc;
	event_send->type = dbp;
/* Flush display, discarding events. */
	XSync(display, True);
/* Flush client2, discarding events. */
	XSync(client2, True);
/* Call XSendExtensionEvent to send a ButtonPress event. */
	_xcall_(return_value);
/* Verify that XSendExtensionEvent returned non-zero. */
	if (return_value == 0) {
		report("Returned zero, expected non-zero.");
		FAIL;
	}
	else
		CHECK;
/* Flush display, not discarding events. */
	XSync(display, False);
/* Flush client2, not discarding events. */
	XSync(client2, False);
/* Verify that event was received. */
	if (XCheckTypedWindowEvent(display, w, event_send->type, &event_return) == False) {
		report("Expected event (%s) not received.", eventname(event_send->type));
		FAIL;
	}
	else
		CHECK;
/* Verify that send_event was not set to False. */
	event = (XAnyEvent *) &event_return;
	if (event->send_event == False) {
		report("send_event not set to True");
		FAIL;
	}
	else
		CHECK;
/* Verify that client2 received no events. */
	status = XPending(client2);
	if (status != 0) {
		report("Client2 received %d events, expected %d", status, 0);
		FAIL;
	}
	else
		CHECK;
/* Set event_mask to KeyPressMask. */
	event_list = &dbpc;
	event_send->type = dbp;
/* Flush display, discarding events. */
	XSync(display, True);
/* Flush client2, discarding events. */
	XSync(client2, True);
/* Call XSendExtensionEvent to send a KeyPress event. */
	_xcall_(return_value);
/* Flush display, not discarding events. */
	XSync(display, False);
/* Flush client2, not discarding events. */
	XSync(client2, False);
/* Verify that XSendExtensionEvent returned non-zero. */
	if (return_value == 0) {
		report("Returned zero, expected non-zero.");
		FAIL;
	}
	else
		CHECK;
/* Verify that event was received. */
	if (XCheckTypedWindowEvent(display, w, event_send->type, &event_return) == False) {
		report("Expected event (%s) not received.", eventname(event_send->type));
		FAIL;
	}
	else
		CHECK;
/* Verify that send_event was not set to False. */
	event = (XAnyEvent *) &event_return;
	if (event->send_event == False) {
		report("send_event not set to True");
		FAIL;
	}
	else
		CHECK;
/* Verify that client2 received no events. */
	status = XPending(client2);
	if (status != 0) {
		report("Client2 received %d events, expected %d", status, 0);
		FAIL;
	}
	else
		CHECK;
/* Ungrab the keyboard with client2. */
	XUngrabKeyboard(client2, CurrentTime);
/* Ungrab buttons with client2. */
	XUngrabButton(client2, AnyButton, AnyModifier, w2);
/* Ungrab the pointer with client2. */
	XUngrabPointer(client2, CurrentTime);
	CHECKPASS(15);
>>ASSERTION Good B 3
On a call to xname
the only fields in the forwarded event which are changed
are the
.M send_event
and
.M serial
fields.
>>STRATEGY
Create window.
Set type member of event to KeyPress.
Set serial member of event to 0.
Set send_event member of event to False.
Ignore display member of event, as it's filled in by Xlib.
Set window member of event to 0xffffffff.
Discard all events in the event queue.
Call XSendExtensionEvent to send event to creator of window.
Verify that XSendExtensionEvent returned non-zero.
Verify that event was received.
Verify that type was set to KeyPress.
Verify that serial was not set to 0.
Verify that send_event was not set to False.
Check display member of event was filled in by Xlib properly.
Verify that window was set to 0.
Repeat for each event-type.
>>CODE
int	i;
XAnyEvent *event;
XEvent	event_return;
int	return_value;
XID noextensionevent, cdn, dmn, dsn;
XEventClass noextensioneventclass, cdnc, dmnc, dsnc;

/* Create window. */
	if (!Setup_Extension_DeviceInfo(BtnMask))
	    {
	    untested("%s: No input extension button device.\n", TestName);
	    return;
	    }
	device = Devs.Button;
	ChangeDeviceNotify(Devs.Button, cdn, cdnc);
	DeviceMappingNotify(Devs.Button, dmn, dmnc);
	DeviceStateNotify(Devs.Button, dsn, dsnc);
	NoExtensionEvent(device, noextensionevent, noextensioneventclass);
	w = mkwin(display, (XVisualInfo *) NULL, (struct area *) NULL, False);
	propagate = False;
	count=1;
	event_list = &noextensioneventclass;
	event = (XAnyEvent *) &event_return;
	for (i = 0; i < nevent; i++) {
		char *en = eventname(event_types[i]);

/* Set type member of event to KeyPress. */
		event_send->type = event_types[i];

/* Set serial member of event to 0. */
		event_send->xany.serial = 0;

/* Set send_event member of event to False. */
		event_send->xany.send_event = False;

/* Ignore display member of event, as it's filled in by Xlib. */
		event_send->xany.display = (Display *) NULL;

/* Set window member of event to 0xffffffff. */
		event_send->xany.window = (Window) 0xffffffff;

/* Discard all events in the event queue. */
		XSync(display, True);

/* Call XSendExtensionEvent to send event to creator of window. */
		_xcall_(return_value);

/* Verify that XSendExtensionEvent returned non-zero. */
		if (return_value == 0) {
			report("%s: Returned zero, expected non-zero.",en);
			FAIL;
		}
		else
			CHECK;

/* Verify that event was received. */
		XSync(display, False);
		if (XPending(display) == 0) {
			report("%s: No events received.",en);
			FAIL;
			continue;
		}
		else
			CHECK;
		if (XCheckTypedEvent(display, event_send->type, &event_return) == False) {
			report("Expected event (%s) not received.", en);
			FAIL;
			continue;
		}
		else
			CHECK;

/* Verify that type was set to KeyPress. */
		if (event->type != event_types[i]) {
			report("type set to %s, expected %s", eventname(event->type), en);
			FAIL;
		}
		else
			CHECK;

/* Verify that serial was not set to 0. */
		if (event->serial == 0) {
			report("%s: serial set to zero, expected non-zero",en);
			FAIL;
		}
		else
			CHECK;

/* Verify that send_event was not set to False. */
		if (event->send_event == False) {
			report("%s: send_event not set to True", en);
			FAIL;
		}
		else
			CHECK;

/* Check display member of event was filled in by Xlib properly. */
		if (event->display != display) {
			report("%s: display not set to my display", en);
			FAIL;
		}
		else
			CHECK;

/* Verify that window was set to 0. */
		if (event->window != (Window) 0xffffffff &&
			 event->type != dmn &&
			 event->type != cdn &&
			 event->type != dsn) {
			report("%s: window set to 0x%x, expected 0x%x", en, event->window, (Window) 0xffffffff);
			FAIL;
		}
		else
			CHECK;

/* Repeat for each event-type. */
	}
	CHECKPASS(8*nevent);
>>ASSERTION def
On a call to xname
the
.M send_event
field in the forwarded event is set to
.S True .
>>ASSERTION Good B 3
On a call to xname
the
.M serial
field in the forwarded event is set correctly.
>>STRATEGY
Create window.
Set serial to zero.
Call NextRequest to get correct serial.
Call XSendExtensionEvent.
Verify that XSendExtensionEvent returned non-zero.
Verify that event was received.
Verify that serial in delivered event is set correctly.
>>CODE
int	correctserial;
XAnyEvent *event;
XEvent	event_return;
int	return_value;
XID noextensionevent, dbp;
XEventClass noextensioneventclass, dbpc;

/* Create window. */
	if (!Setup_Extension_DeviceInfo(BtnMask))
	    {
	    untested("%s: No input extension button device.\n", TestName);
	    return;
	    }
	/* Force the extension version to be initialized.  Otherwise, the call
	   to XSendExtensionEvent will result in two calls, one to 
	   XGetExtensionVersion and another to XSendExtensionEvent. */

	XGetExtensionVersion(display,"XInputExtension");
	device = Devs.Button;
	DeviceButtonPress(device, dbp, dbpc);
	NoExtensionEvent(device, noextensionevent, noextensioneventclass);
	w = mkwin(display, (XVisualInfo *) NULL, (struct area *) NULL, False);
	propagate = False;
	count = 1;
	event_list = &noextensioneventclass;
/* Set serial to zero. */
	event = (XAnyEvent *) event_send;
	event->serial = 0;
	event_send->type = dbp;
/* Call NextRequest to get correct serial. */
	XSync(display,0);
	correctserial = NextRequest(display);
/* Call XSendExtensionEvent. */
	_xcall_(return_value);
/* Verify that XSendExtensionEvent returned non-zero. */
	if (return_value == 0) {
		report("Returned zero, expected non-zero.");
		FAIL;
	}
	else
		CHECK;
/* Verify that event was received. */
	XFlush(display);
	XSync(display, False);
	if (XPending(display) == 0) {
		report("No events received.");
		FAIL;
		return;
	}
	else
		CHECK;
	if (XCheckTypedEvent(display, event_send->type, &event_return) == False) {
		report("Expected event (%s) not received.", eventname(event_send->type));
		FAIL;
		return;
	}
	else
		CHECK;
/* Verify that serial in delivered event is set correctly. */
	event = (XAnyEvent *) &event_return;
	if (event->serial != correctserial) {
		report("Serial set to %d, expected %d", event->serial, correctserial);
		FAIL;
	}
	else
		CHECK;
	CHECKPASS(4);
>>ASSERTION Bad B 3
>>#NOTE How does one determine events associated with an extension?
>>#NOTE Responses to this question include:
>>#NOTE 	Use XListExtensions?
>>#NOTE 	Use testing extension?
>>#NOTE 	Use values which don't correspond to known event types?
>>#NOTE At present, the strategy chooses the last option.
When the event in
.A event_send
is not one of the core events or one of the events defined by an extension,
>># Next line altered at the request of MIT (bug report 60).
>># then a
then either xname returns zero or a
.S BadValue
error occurs.
>>STRATEGY
Create window.
Use notmember() to get some bad event types.
Make sure this list also includes 0 and 1 which are specifically for errors and replies.
Call XSendExtensionEvent to send bad event type.
Verify that XSendExtensionEvent either returned zero and no error occurs,
	or returned non-zero and a BadValue error occurs.
>>CODE BadValue
int	i;
int	n;
int	return_value;
int	good_event_types[XInputNumEvents];
long	bad_event_types[NM_LEN+2];
XID noextensionevent;
XEventClass noextensioneventclass;

	if (!Setup_Extension_DeviceInfo(BtnMask))
	    {
	    untested("%s: No input extension button device.\n", TestName);
	    return;
	    }
	device = Devs.Button;
	NoExtensionEvent(device, noextensionevent, noextensioneventclass);
	propagate = False;
	event_list = &noextensioneventclass;

/* Create window. */
	w = mkwin(display, (XVisualInfo *) NULL, (struct area *) NULL, False);

/* Use notmember() to get some bad event types. */
	for(i=0; i<XInputNumEvents; i++)
	    good_event_types[i]=XInputFirstEvent+i;
	n = notmember(good_event_types, XInputNumEvents, &bad_event_types[2]);

/* Make sure this list also includes 0 and 1 which are specifically for errors and replies. */
	n += 2;
	bad_event_types[0] = 0;
	bad_event_types[1] = 1;
	for (i = 0; i < n; i++) {

/* Call XSendExtensionEvent to send bad event type. */
		event_send->type = bad_event_types[i];
		trace("Using known bad event type %d.", event_send->type);
>>SET no-error-status-check
		return_value = XCALL;

/* Verify that XSendExtensionEvent either returned zero and no error occurs, */
/* 	or returned non-zero and a BadValue error occurs. */
		if (return_value == 0) {
			if(geterr() != Success) {
				FAIL;
				report("%s returned zero, but got %s when expecting no error.",
					TestName, errorname(geterr()));
			} else {
				trace("%s returned zero and no error", TestName);
				CHECK;
			}
		} else {
			if (geterr() != BadValue) {
				FAIL;
				report("%s returned non-zero, but got %s when expecting %s",
					TestName, errorname(geterr()), errorname(BadValue));
			} else {
				trace("%s returned non-zero and BadValue", TestName);
				CHECK;
			}
		}
	}

	CHECKPASS(n);
>>ASSERTION Bad B 3
When a window argument does not name a valid window, then a 
.S BadWindow
error occurs.
>>STRATEGY
Create a bad window by creating and destroying a window.
Initialise the event structure for the call.
Verify that a BadWindow error occurs.
>>CODE BadWindow
int	dbp, return_value;
XEventClass dbpc;

	if (!Setup_Extension_DeviceInfo(BtnMask))
	    {
	    untested("%s: No input extension button device.\n", TestName);
	    return;
	    }
	device = Devs.Button;
	DeviceButtonPress(device, dbp, dbpc);
	event_list = &dbpc;
	seterrdef();

/* Create a bad window by creating and destroying a window. */
	w = badwin(display);

/* Initialise the event structure for the call. */
	propagate = False;
	event_send->type = dbp;

/* Call xname using bad window as the window argument.*/
	return_value = XCALL;
	if (return_value == 0) {
		FAIL;
		report("%s returned zero, expecting a non-zero result.",
			TestName);
	} else
		CHECK;

/* Verify that a BadWindow error occurs. */
	if (geterr() == BadWindow)
		CHECK;
	else
		FAIL;

	CHECKPASS(2);
>>ASSERTION Bad B 3
When a device argument does not name a valid device, then a 
baddevice error occurs.
>>STRATEGY
Specify a bad device.
Initialise the event structure for the call.
Verify that a baddevice error occurs.
>>CODE baddevice
XID baddevice, dbp;
XEventClass dbpc;
XDevice bogus;
int	return_value;

	if (!Setup_Extension_DeviceInfo(BtnMask))
	    {
	    untested("%s: No input extension button device.\n", TestName);
	    return;
	    }
	DeviceButtonPress(Devs.Button, dbp, dbpc);
	event_list = &dbpc;
/* Create window. */
	w = mkwin(display, (XVisualInfo *) NULL, (struct area *) NULL, False);
	BadDevice(display,baddevice);
	bogus.device_id = -1;
	device = &bogus;
	seterrdef();

/* Initialise the event structure for the call. */
	propagate = False;
	event_send->type = dbp;

	return_value = XCALL;
	if (return_value == 0) {
		FAIL;
		report("%s returned zero, expecting a non-zero result.",
			TestName);
	} else
		CHECK;

/* Verify that a baddevice error occurs. */
	if (geterr() == baddevice)
		CHECK;
	else
		FAIL;
	CHECKPASS(2);

>>ASSERTION Bad B 3
When an eventclass argument does not name a valid device, then a 
badclass error occurs.
>>STRATEGY
Specify a bad event class;
Initialise the event structure for the call.
Verify that a badclass error occurs.
>>CODE badclass
XID badclass, dbp;
XEventClass bogus=-1, dbpc;
int	return_value;

	if (!Setup_Extension_DeviceInfo(BtnMask))
	    {
	    untested("%s: No input extension button device.\n", TestName);
	    return;
	    }
	BadClass(display,badclass);
	device = Devs.Button;
	DeviceButtonPress(device, dbp, dbpc);
	seterrdef();
	event_list = &bogus;

/* Initialise the event structure for the call. */
	propagate = False;
	event_send->type = dbp;

/* Call xname using bad window as the window argument.*/
	return_value = XCALL;
	if (return_value == 0) {
		FAIL;
		report("%s returned zero, expecting a non-zero result.",
			TestName);
	} else
		CHECK;

/* Verify that a badclass error occurs. */
	if (geterr() == badclass)
		CHECK;
	else
		FAIL;

	CHECKPASS(2);

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
 * $XConsortium: kyrls.m,v 1.14 94/04/17 21:07:48 rws Exp $
 */
>>TITLE KeyRelease CH08
>>EXTERN
#define EVENT KeyRelease
#define EVENTMASK KeyReleaseMask

static void
presskey(Dpy, w, kc)
Display		*Dpy;
Window		w;
int		kc;
{
	/*
	 * Note that we do not call XSetInputFocus() since 2 tests require
	 * the event window to be an ancestor of the source window.
	 */
	keypress(Dpy, kc);
	XSync(Dsp, True);
	keyrel(Dsp, kc);
}

static	int	keyc = 0;

static unsigned int
keycode()
{
int	tmp;

	if(keyc == 0) {
		XDisplayKeycodes(Dsp, &keyc, &tmp);
        	if (keyc < 8)
	                keyc = 8;
	}
	return(keyc);
}    

static unsigned int
keystate(keyc)
int keyc;
{
XModifierKeymap *map;
unsigned int state;
int i, j;

	map = XGetModifierMapping(Dsp);
	if (!map)
		return(0);
	state = 0;
	for (i = 0; i < 8; i++) {
		for (j = 0; j < map->max_keypermod; j++) {
			if (map->modifiermap[(i * map->max_keypermod) + j] ==
			    keyc)
				state |= (ShiftMask << i);
		}
	}
	XFreeModifiermap(map);
	return(state);
}
>>SET startup focusstartup
>>SET cleanup focuscleanup
>>ASSERTION Good B 1
>>#NOTE The documentation not explicitly state the following anywhere,
>>#NOTE it merely states something relative to logical changes in
>>#NOTE key state.  In addition, I don't expect for very many of these
>>#NOTE assertions to be testable.
When any key is released,
then a xname event is generated.
>>STRATEGY
If extended testing is enabled:
  Create a window.
  Select for xname events.
  Simulate a xname event on the window.
  Verify that a xname event was delivered.
  Verify that event member fields are correctly set.
>>CODE
Display		*display = Dsp;
Window		w;
XEvent		event_return;
XKeyEvent	good;
PointerPlace	*ptr;
int		kc;

	

	/* If extended testing is enabled: */
	if(noext(0))
		return;

		/* Create a window. */
	w = defwin(display);

		/* Select for xname events. */
	XSelectInput(display, w, EVENTMASK);

		/* Simulate a xname event on the window. */
	ptr = warppointer(display, w, 0,0);
	kc = keycode();
	presskey(display, w, kc);

		/* Verify that a xname event was delivered. */
	if (!XCheckTypedWindowEvent(display, w, EVENT, &event_return)) {
		report("Expected %s event, got none", eventname(EVENT));
		FAIL;
	} else {

		CHECK;

		good = event_return.xkey;
		good.type = EVENT;
		good.send_event = False;
		good.display = display;
		good.window = w;
		good.root = DRW(display);
		good.subwindow = None;
		good.x = 0;
		good.y = 0;
		good.x_root = ptr->nx;
		good.y_root = ptr->ny;
		good.state = keystate(kc);
		good.keycode = kc;
		good.same_screen = True;

		/* Verify that event member fields are correctly set. */
		if (checkevent((XEvent *) &good, &event_return)) {
			report("Unexpected values in delivered event");
			FAIL;
		} else
			CHECK;
	}

	CHECKPASS(2);

>>ASSERTION Good B 1
When a xname event is generated,
then
all clients having set
.S KeyReleaseMask
event mask bits on the event window are delivered
a xname event.
>>STRATEGY
If extended testing is required:
  Create a second client.
  Create a third client.
  Create a window.
  Select for xname events on all clients.
  Simulate a xname event on the window.
  Verify that the event was delivered to all clients.
  Verify that event member fields are correctly set.
Otherwise :
  Create a window.
  Select for xname events.
  Verify no error was generated.
>>CODE
Display			*display = Dsp;
Display			*client2;
Display			*client3;
Window			w;
XEvent			event_return;
XKeyReleasedEvent	good;
PointerPlace		*ptr;
int	 		kc;

		/* If extended testing is required: */
	if(config.extensions) {
	
		if(noext(0))
			return;

			/* Create a second client. */
		client2 = opendisplay();
			/* Create a third client. */
		client3 = opendisplay();

			/* Create a window. */
		w = defwin(display);

			/* Select for xname events on all clients. */
		XSelectInput(display, w, EVENTMASK);
		XSelectInput(client2, w, EVENTMASK);
		XSelectInput(client3, w, EVENTMASK);
	
			/* Simulate a xname event on the window. */
		ptr = warppointer(display, w, 0,0);
		kc = keycode();

		XSync(client2, True);
		XSync(client3, True);

		presskey(display, w, kc);

		XSync(client2, False);
		XSync(client3, False);
	
		good = event_return.xkey;
		good.type = EVENT;
		good.send_event = False;
		good.display = display;
		good.window = w;
		good.root = DRW(display);
		good.subwindow = None;
		good.x = 0;
		good.y = 0;
		good.x_root = ptr->nx;
		good.y_root = ptr->ny;
		good.state = keystate(kc);
		good.keycode = kc;
		good.same_screen = True;
	
			/* Verify that the event was delivered to all clients. */
			/* Verify that event member fields are correctly set. */
		if (!XCheckTypedWindowEvent(display, w, EVENT, &event_return)) {
			report("Expected %s event, got none", eventname(EVENT));
			FAIL;
		} else {
			CHECK;
			if (checkevent((XEvent *) &good, &event_return)) {
				report("Unexpected values in delivered event");
				FAIL;
			} else
				CHECK;
		}	
/* client 2 */
		if (!XCheckTypedWindowEvent(client2, w, EVENT, &event_return)) {
			report("Expected %s event, got none", eventname(EVENT));
			FAIL;
		} else {
			CHECK;

			good.display = client2;
		
			if (checkevent((XEvent *) &good, &event_return)) {
				report("Unexpected values in delivered event");
				FAIL;
			} else
				CHECK;
		}
/* client 3 */

		if (!XCheckTypedWindowEvent(client3, w, EVENT, &event_return)) {
			report("Expected %s event, got none", eventname(EVENT));
			FAIL;
		} else {
			CHECK;

			good.display = client3;

			if (checkevent((XEvent *) &good, &event_return)) {
				report("Unexpected values in delivered event");
				FAIL;
			} else
				CHECK;
		}

		CHECKPASS(6);

	} else {

		/* Otherwise : */
			/* Create a window. */
		w = mkwin(display, (XVisualInfo *) NULL, (struct area *) NULL, False);
			/* Select for xname events. */
		XSelectInput(display, w, EVENTMASK);
		XSync(display, True);
			/* Verify no error was generated. */
		if (geterr() != Success) {
			delete("Error setting up for test.");
			return;
		}
		else
			CHECK;
	
		CHECKUNTESTED(1);
	}

>>ASSERTION Good B 1
>>#NOTE True for most events (except MappingNotify and selection stuff).
When a xname event is generated,
then
clients not having set
.S KeyReleaseMask
event mask bits on the event window are not delivered
a xname event.
>>STRATEGY
If extended testing is required:
  Create a window.
  Create a second client.
  Select for various events other that xname for the first client on the window.
  Select for xname events for the second client on the window.
  Simulate a xname event on the window.
  Verify that a xname event was generated for the second client.
  Verify no events were generated for the first client.
>>CODE
Display			*client2;
XEvent			ev;
Window			w;
int			count;
int			kc;

	/* If extended testing is required: */
	if(noext(0))
		return;

		/* Create a window. */
	w = defwin(Dsp);

		/* Create a second client. */
	client2 = opendisplay();

		/* Select for various events other that xname for the first client on the window. */
	kc = keycode();
 	XSelectInput(Dsp, w, KeyPressMask|ButtonPressMask|ButtonReleaseMask);

		/* Select for xname events for the second client on the window. */
	XSelectInput(client2, w, EVENTMASK);

		/* Simulate a xname event on the window. */
	(void) warppointer(Dsp, w, 0, 0);
	XSync(Dsp, True);
	XSync(client2, True);

	presskey(Dsp, w, kc);

	XSync(client2, False);

		/* Verify that a xname event was generated for the second client. */
	if(XCheckWindowEvent(client2, w, EVENTMASK, &ev) == False) {
		report("No %s event was generated.", eventname(EVENT));
		FAIL;
	} else
		CHECK;

		/* Verify no events were generated for the first client. */
	count = XPending(Dsp);
	if (count != 0) {
		report("Got %d unexpected events.", count);
		FAIL;
	} else
		CHECK;

	CHECKPASS(2);

>>ASSERTION Good B 1
When a xname event is generated
and no client has selected
.S KeyReleaseMask
on the source window,
then the event propagates,
with propagation stopping at the root window of the screen or
at the first window with
.S KeyReleaseMask
in its do-not-propagate mask,
from the source window to
the first ancestor window for which
some client has selected 
for xname events.
>>STRATEGY
If extended testing is required:
  Create a window.
  Create a child of that window.
  Create a grandchild of the window.
  Select xname events on the root window of the screen.
  Simulate a xname event on the grandchild.
  Verify that a xname event was generated on the root.
  Select xname events on the grandparent window.
  Set the do_not_propagate mask on the child to xname events.
  Simulate a xname event on the grandchild.
  Verify no xname event was generated on the grandchild.
  Verify no xname event was generated on the child.
  Verify no xname event was generated on the parent.
  Select xname events on the child window.
  Set the do_not_propagate mask of the grandparent to xname events.
  Set the do_not_propagate mask of the child to NoEventMask.
  Simulate a xname event on the grandchild.
  Verify that no xname event was generated on the grandchild.
  Verify that no xname event was generated on the parent.
  Verify that a xname event was generated on the child.
>>CODE
XEvent			ev;
Window			w;
Window			w1;
Window			w2;
XSetWindowAttributes	atts;
int			kc;


	/* If extended testing is required: */
	if(noext(0))
		return;

		/* Create a window. */
	w = defwin(Dsp);
		/* Create a child of that window. */
	w1 = crechild(Dsp, w, (struct area *) NULL);
		/* Create a grandchild of the window. */
	w2 = crechild(Dsp, w1, (struct area *) NULL);

		/* Select xname events on the root window of the screen. */
	XSelectInput(Dsp, DRW(Dsp), EVENTMASK);	

		/* Simulate a xname event on the grandchild. */
	(void) warppointer(Dsp, w2, 1,1);
	kc = keycode();
	presskey(Dsp, w2, kc);
	
		/* Verify that a xname event was generated on the root. */
	if (XCheckWindowEvent(Dsp, DRW(Dsp), EVENTMASK, &ev) == False) {
		report("Expected event (%s) not received.", eventname(EVENT));
		FAIL;
	} else
		CHECK;

	XSelectInput(Dsp, DRW(Dsp), NoEventMask);	
		/* Select xname events on the grandparent window. */
	XSelectInput(Dsp, w, EVENTMASK);	

		/* Set the do_not_propagate mask on the child to xname events. */
	atts.do_not_propagate_mask = EVENTMASK;
	XChangeWindowAttributes(Dsp, w1, CWDontPropagate, &atts);
	
		/* Simulate a xname event on the grandchild. */
	presskey(Dsp, w2, kc);	

		/* Verify no xname event was generated on the grandchild. */
	if (XCheckWindowEvent(Dsp, w2, EVENTMASK, &ev) != False) {
		report("Unexpected event (%s) received.", eventname(EVENT));
		FAIL;
	} else
		CHECK;

		/* Verify no xname event was generated on the child. */
	if (XCheckWindowEvent(Dsp, w1, EVENTMASK, &ev) != False) {
		report("Unexpected event (%s) received.", eventname(EVENT));
		FAIL;
	} else
		CHECK;

		/* Verify no xname event was generated on the parent. */
	if (XCheckWindowEvent(Dsp, w, EVENTMASK, &ev) != False) {
		report("Unexpected event (%s) received.", eventname(EVENT));
		FAIL;
	} else
		CHECK;


		/* Select xname events on the child window. */
	XSelectInput(Dsp, w1, EVENTMASK);	
		/* Set the do_not_propagate mask of the grandparent to xname events. */
	XChangeWindowAttributes(Dsp, w, CWDontPropagate, &atts);

		/* Set the do_not_propagate mask of the child to NoEventMask. */
	atts.do_not_propagate_mask = NoEventMask;
	XChangeWindowAttributes(Dsp, w1, CWDontPropagate, &atts);

		/* Simulate a xname event on the grandchild. */
	presskey(Dsp, w2, kc);

		/* Verify that no xname event was generated on the grandchild. */
	if (XCheckWindowEvent(Dsp, w2, EVENTMASK, &ev) != False) {
		report("Unexpected event (%s) received.", eventname(EVENT));
		FAIL;
	} else
		CHECK;

		/* Verify that no xname event was generated on the parent. */
	if (XCheckWindowEvent(Dsp, w, EVENTMASK, &ev) != False) {
		report("Unexpected event (%s) received.", eventname(EVENT));
		FAIL;
	} else
		CHECK;

		/* Verify that a xname event was generated on the child. */
	if (XCheckWindowEvent(Dsp, w1, EVENTMASK, &ev) == False) {
		report("KeyRelease event was not delivered to selecting child window.");
		FAIL;
	} else
		CHECK;

	CHECKPASS(7);

>>#NOTEs >>ASSERTION
>>#NOTEs When ARTICLE xname event is delivered,
>>#NOTEs then
>>#NOTEs .M type
>>#NOTEs is set to
>>#NOTEs xname.
>>#NOTEs >>ASSERTION
>>#NOTEs >>#NOTE The method of expansion is not clear.
>>#NOTEs When ARTICLE xname event is delivered,
>>#NOTEs then
>>#NOTEs .M serial
>>#NOTEs is set
>>#NOTEs from the serial number reported in the protocol
>>#NOTEs but expanded from the 16-bit least-significant bits
>>#NOTEs to a full 32-bit value.
>>#NOTEm >>ASSERTION
>>#NOTEm When ARTICLE xname event is delivered
>>#NOTEm and the event came from a
>>#NOTEm .S SendEvent
>>#NOTEm protocol request,
>>#NOTEm then
>>#NOTEm .M send_event
>>#NOTEm is set to
>>#NOTEm .S True .
>>#NOTEs >>ASSERTION
>>#NOTEs When ARTICLE xname event is delivered
>>#NOTEs and the event was not generated by a
>>#NOTEs .S SendEvent
>>#NOTEs protocol request,
>>#NOTEs then
>>#NOTEs .M send_event
>>#NOTEs is set to
>>#NOTEs .S False .
>>#NOTEs >>ASSERTION
>>#NOTEs When ARTICLE xname event is delivered,
>>#NOTEs then
>>#NOTEs .M display
>>#NOTEs is set to
>>#NOTEs a pointer to the display on which the event was read.
>>#NOTEs >>ASSERTION
>>#NOTEs >>#NOTE Global except for MappingNotify and KeymapNotify.
>>#NOTEs When ARTICLE xname event is delivered,
>>#NOTEs then
>>#NOTEs .M window
>>#NOTEs is set to
>>#NOTEs the
>>#NOTEs ifdef(`WINDOWTYPE', WINDOWTYPE, event)
>>#NOTEs window.
>>#NOTEs >>ASSERTION
>>#NOTEs When ARTICLE xname event is delivered,
>>#NOTEs then
>>#NOTEs .M root
>>#NOTEs is set to the source window's root window.
>>ASSERTION Good B 1
>>#
>>#COMMENT
>># This assertion contains a tautology; a child is an inferior.
>># Cal 12/2/92.
>>#
When a xname event is delivered
and the source window is an inferior of the event window
and the source window is a child of the event window,
then
.M subwindow
is set to
the source window.
>>STRATEGY
If extended testing is required:
  Create a window.
  Create a child of the window
  Select xname events on the parent.
  Simulate an xname event on the child.
  Verify that a xname event was generated on the parent.
  Verify that the subwindow component was set to the child.
>>CODE
XEvent		ev;
Window		w;
Window		w1;
int		kc;


	/* If extended testing is required: */
	if(noext(0))
		return;

		/* Create a window. */
	w = defwin(Dsp);
		/* Create a child of the window */
	w1 = crechild(Dsp, w, (struct area *) NULL);
		/* Select xname events on the parent. */
	XSelectInput(Dsp, w, EVENTMASK);	
	(void) warppointer(Dsp, w1, 1,1);
	
		/* Simulate a xname event on the child. */
	kc=keycode();
	presskey(Dsp, w1, kc);

		/* Verify that a xname event was generated on the parent. */
	if (XCheckWindowEvent(Dsp, w, EVENTMASK, &ev) == False) {
		report("Expected event (%s) not received.", eventname(EVENT));
		FAIL;
	} else {
		CHECK;
		/* Verify that the subwindow component was set to the child. */
		if(ev.xkey.subwindow != w1) {
			report("The subwindow component of the %s name was not set correctly.", eventname(EVENT));
			FAIL;
		} else 
			CHECK;
	}
	
	CHECKPASS(2);

>>ASSERTION Good B 1
When a xname event is delivered
and the source window is an inferior of the event window
and the source window is not a child of the event window,
then
.M subwindow
is set to
the child of the event window that is
an ancestor of the source window.
>>STRATEGY
If extended testing is required :
  Create a window.  
  Create a child.
  Create a grandchild.
  Create a great-grandchild.
  Select xname events on the great-grandparent.
  Simulate a xname event on the great-grandchild.
  Verify that a xname event was generated on the great-grandparent.
  Verify that subwindow component of the event was the child.
>>CODE
Window		w;
Window		w1;
Window		w2;
Window		w3;
XEvent		ev;
struct area	area;
int		kc;

	/* If extended testing is required : */
	if (noext(0))
		return;
		
		/* Create a window. */	
	w = defwin(Dsp);
	setarea(&area, 10, 10, 30, 30);
		/* Create a child. */
	w1 = crechild(Dsp, w, &area);
		/* Create a grandchild. */
	w2 = crechild(Dsp, w1, &area);
		/* Create a great-grandchild. */
	w3 = crechild(Dsp, w2, &area);
	(void) warppointer(Dsp, w3, 1,1);

		/* Select xname events on the great-grandparent. */
	XSelectInput(Dsp, w, EVENTMASK);

		/* Simulate a xname event on the great-grandchild. */
	kc = keycode();
	presskey(Dsp, w3, kc);
	
		/* Verify that a xname event was generated on the great-grandparent. */
	if (XCheckWindowEvent(Dsp, w, EVENTMASK, &ev) == False) {
		report("Expected event (%s) not received.", eventname(EVENT));
		FAIL;
	} else {
		CHECK;
		/* Verify that subwindow component of the event was the child. */
		if(ev.xkey.subwindow != w1) {
			report("The subwindow component of the %s event was not set correctly.", eventname(EVENT));
			FAIL;
		} else 
			CHECK;
	}

	CHECKPASS(2);

>>ASSERTION Good B 1
When a xname event is delivered
and the source window is not an inferior of the event window,
then
.M subwindow
is set to
.S None .
>>STRATEGY
If extended testing is required:
  Create a window.
  Select xname events on the window.
  Simulate a xname event on the window.
  Verify that a xname event was generated on the window.
  Verify that the subwindow component was set to None.
  Create a second window.
  Grab the keyboard for the first window with owner_events set to False.
  Simulate a xname event on the second window.
  Verify that a xname event was generated on the grabbing window.
  Verify that the subwindow component was set to None.
>>CODE
Window		w;
Window		w2;
XEvent		ev;
int		gr;
int		kc;

	/* If extended testing is required: */
	if (noext(0))
		return;

		/* Create a window. */
        w = defwin(Dsp);
	(void) warppointer(Dsp, w, 1,1);

		/* Select xname events on the window. */
	XSelectInput(Dsp, w, EVENTMASK);

		/* Simulate a xname event on the window. */
	kc=keycode();
	presskey(Dsp, w, kc);
	
		/* Verify that a xname event was generated on the window. */
	if (XCheckWindowEvent(Dsp, w, EVENTMASK, &ev) == False) {
		report("Expected event (%s) not received.", eventname(EVENT));
		FAIL;
	} else {
		CHECK;
		/* Verify that the subwindow component was set to None. */
		if(ev.xkey.subwindow != None) {
			report("The subwindow component of the %s event was not set correctly.", eventname(EVENT));
			FAIL;
		} else 
			CHECK;
	}

		/* Create a second window. */
	w2 = defwin(Dsp);

		/* Grab the keyboard for the first window with owner_events set to False. */
	if( (gr=XGrabKeyboard(Dsp, w, False, GrabModeAsync, GrabModeAsync, CurrentTime)) != GrabSuccess) {
		delete("XGrabKeyboard() returned %s instead of GrabSuccess.", grabreplyname(gr));
		return;
	} else
		CHECK;

		/* Simulate a xname event on the second window. */
	(void) warppointer(Dsp, w2, 1,1);
	presskey(Dsp, w2, kc);
	
		/* Verify that a xname event was generated on the grabbing window. */
	if (XCheckWindowEvent(Dsp, w, EVENTMASK, &ev) == False) {
		report("Expected event (%s) not received.", eventname(EVENT));
		FAIL;
	} else {
		CHECK;
		/* Verify that the subwindow component was set to None. */
		if(ev.xkey.subwindow != None) {
			report("The subwindow component of the %s event was not set correctly.", eventname(EVENT));
			FAIL;
		} else 
			CHECK;
	}

	XUngrabKeyboard(Dsp, CurrentTime);

	CHECKPASS(5);

>>#NOTEs >>ASSERTION
>>#NOTEs When ARTICLE xname event is delivered,
>>#NOTEs then
>>#NOTEs .M time
>>#NOTEs is set to
>>#NOTEs the time in milliseconds at which the event was generated.
>>#NOTEs >>ASSERTION
>>#NOTEs When ARTICLE xname event is delivered
>>#NOTEs and the event window is on the same screen as the root window,
>>#NOTEs then
>>#NOTEs .M x
>>#NOTEs and
>>#NOTEs .M y
>>#NOTEs are set to
>>#NOTEs the coordinates of
>>#NOTEs the final pointer position relative to the event window's origin.
>>ASSERTION Good D 1
If multiple screens are supported:
When a xname event is delivered
and the event and root windows are not on the same screen,
then
.M x
and
.M y
are set to
zero.
>>STRATEGY
If multiple screens are supported:
  If extended testing is required:
    Create a window on the default screen.
    Select xname events on the window.
    Create a window on the alternative screen.
    Grab the keyboard for the first window.
    Simulate a xname event on the alternate window.
    Verify that a xname event was generated on the grabbing window.
    Verify that the x and y components were set to zero.
>>CODE
Window		w;
Window		w2;
XEvent		ev;
int		gr;
int		kc;


	/* If multiple screens are supported: */
	if (config.alt_screen == -1) {
		unsupported("Multiple screens not supported.");
		return;
	} else
		CHECK;

		/* If extended testing is required: */
	if(noext(0))
		return;
		
			/* Create a window on the default screen. */
        w = defwin(Dsp);
	
			/* Select xname events on the window. */
	(void) warppointer(Dsp, w, 1,1);
	XSelectInput(Dsp, w, EVENTMASK);
	
			/* Create a window on the alternative screen. */
	w2 = defdraw(Dsp, VI_ALT_WIN);
	
			/* Grab the keyboard for the first window. */
	if( (gr=XGrabKeyboard(Dsp, w, False, GrabModeAsync, GrabModeAsync, CurrentTime)) != GrabSuccess) {
		delete("XGrabKeyboard() returned %s instead of GrabSuccess.", grabreplyname(gr));
		return;
	} else
		CHECK;
	
	kc=keycode();
			/* Simulate a xname event on the alternate window. */
	(void) warppointer(Dsp, w2, 1,1);
	presskey(Dsp, w2, kc);
	
			/* Verify that a xname event was generated on the grabbing window. */
	if (XCheckWindowEvent(Dsp, w, EVENTMASK, &ev) == False) {
		report("Expected event (%s) not received.", eventname(EVENT));
		FAIL;
	} else {
		CHECK;
			/* Verify that the x and y components were set to zero. */
		if(ev.xkey.x != 0 || ev.xkey.y != 0) {
			report("The x (value %d) and y (value %d) components of the %s event were not set to zero.", ev.xkey.x, ev.xkey.y, eventname(EVENT));
			FAIL;
		} else 
			CHECK;
	}
	
	XUngrabKeyboard(Dsp, CurrentTime);
	
	CHECKPASS(4);

>>#NOTEs >>ASSERTION
>>#NOTEs When ARTICLE xname event is delivered,
>>#NOTEs then
>>#NOTEs .M x_root
>>#NOTEs and
>>#NOTEs .M y_root
>>#NOTEs are set to coordinates of the pointer
>>#NOTEs when the event was generated
>>#NOTEs relative to the root window's origin.
>>#NOTEs >>ASSERTION
>>#NOTEs When ARTICLE xname event is delivered,
>>#NOTEs then
>>#NOTEs .M state
>>#NOTEs is set to
>>#NOTEs indicate the logical state
>>#NOTEs of the pointer buttons,
>>#NOTEs which is the bitwise OR of one or more of
>>#NOTEs the button or modifier key masks
>>#NOTEs .S Button1Mask ,
>>#NOTEs .S Button2Mask ,
>>#NOTEs .S Button3Mask ,
>>#NOTEs .S Button4Mask ,
>>#NOTEs .S Button5Mask ,
>>#NOTEs .S ShiftMask ,
>>#NOTEs .S LockMask ,
>>#NOTEs .S ControlMask ,
>>#NOTEs .S Mod1Mask ,
>>#NOTEs .S Mod2Mask ,
>>#NOTEs .S Mod3Mask ,
>>#NOTEs .S Mod4Mask ,
>>#NOTEs and
>>#NOTEs .S Mod5Mask .
>>#NOTEs >>ASSERTION
>>#NOTEs When ARTICLE xname event is delivered,
>>#NOTEs then
>>#NOTEs .M keycode
>>#NOTEs is set to
>>#NOTEs a number that represents the physical key on the keyboard.
>>#NOTEs >>ASSERTION
>>#NOTEs When ARTICLE xname event is delivered
>>#NOTEs and the event and root windows are on the same screen,
>>#NOTEs then
>>#NOTEs .M same_screen
>>#NOTEs is set to
>>#NOTEs .S True .
>>ASSERTION Good D 1
If multiple screens are supported:
When a xname event is delivered
and the event and root windows are not on the same screen,
then
.M same_screen
is set to
.S False .
>>STRATEGY
If multiple screens are supported:
  If extended testing is required:
    Create a window on the default screen.
    Select xname events on the window.
    Create a window on the alternative screen.
    Grab the keyboard for the first window.
    Simulate a xname event on the alternate window.
    Verify that a xname event was generated on the grabbing window.
    Verify that the same_screen component was False.
>>CODE
Window		w;
Window		w2;
XEvent		ev;
int		gr;
int		kc;

	/* If multiple screens are supported: */
	if (config.alt_screen == -1) {
		unsupported("Multiple screens not supported.");
		return;
	} else
		CHECK;

		/* If extended testing is required: */
	if(noext(0))
		return;

			/* Create a window on the default screen. */
        w = defwin(Dsp);
	
	(void) warppointer(Dsp, w, 1,1);
			/* Select xname events on the window. */
	XSelectInput(Dsp, w, EVENTMASK);
	
			/* Create a window on the alternative screen. */
	w2 = defdraw(Dsp, VI_ALT_WIN);
	
			/* Grab the keyboard for the first window. */
	if( (gr=XGrabKeyboard(Dsp, w, False, GrabModeAsync, GrabModeAsync, CurrentTime)) != GrabSuccess) {
		delete("XGrabKeyboard() returned %s instead of GrabSuccess.", grabreplyname(gr));
		return;
	} else
		CHECK;
	
	kc=keycode();
			/* Simulate a xname event on the alternate window. */
	(void) warppointer(Dsp, w2, 1,1);
	presskey(Dsp, w2, kc);
	
			/* Verify that a xname event was generated on the grabbing window. */
	if (XCheckWindowEvent(Dsp, w, EVENTMASK, &ev) == False) {
		report("Expected event (%s) not received.", eventname(EVENT));
		FAIL;
	} else {
		CHECK;
			/* Verify that the same_screen component was False. */
		if(ev.xkey.same_screen != False) {
			report("The same_screen component of the %s event was not set to False.", eventname(EVENT));
			FAIL;
		} else 
			CHECK;
	}
	
	XUngrabKeyboard(Dsp, CurrentTime);
	
	CHECKPASS(4);

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
 * $XConsortium: stwdwbrdrw.m,v 1.6 94/04/17 21:03:21 rws Exp $
 */
>>TITLE XSetWindowBorderWidth CH03
void

Display	*display = Dsp;
Window	w;
unsigned int width = 4;
>>EXTERN

struct	area	area;

void
tpXSWBWstart()
{
	tpstartup();

	area.x = 15;
	area.y = 15;
	area.width = 30;
	area.height= 30;

}
>>SET tpstartup tpXSWBWstart
>>ASSERTION Good A
A call to xname sets the border width of the specified
window to 
.A width .
>>STRATEGY
Create parent window with a child.
Call xname to set the window border width to 4.
Verify that window border width was set using Pixchecking
Call xname to set the window border width to 10.
Verify that window border width was set using Pixchecking
>>CODE
Window	parent, child;

	trace("Starting");
/* Create parent window with a child. */
	parent = defwin(display);
	trace("Parent made");
	child = crechild(display, parent, &area);
	trace("Child made");

/* Call xname to set the window border width to 4. */
	w = child;
	width = 4;
	XCALL;
	trace("Called %s first time", TestName);

/* Verify that window border width was set using Pixchecking */
	PIXCHECK(display, parent);
	trace("Pixchecked");

/*  Call xname to set the window border width to 10. */
	w = child;
	width = 10;
	XCALL;
	trace("Called %s second time", TestName);

/* Verify that window border width was set using Pixchecking */
	PIXCHECK(display, parent);
	trace("Pixchecked");
	CHECKPASS(2);

>>ASSERTION Good B 1
When the window
is a root window, then a call to xname has no affect.
>>ASSERTION Good A
When the override-redirect attribute of the window is
.S False
and some
other client has selected
.S SubstructureRedirectMask
on the parent window, then a
.S ConfigureRequest
event is generated, and the window configuration is not changed.
>>STRATEGY
Create client1 and client2.
Create a window and child window one
Save parent window image as reference image.
Set override-redirect on window one to False.
Select SubstructureRedirectMask events on the parent window for client2.
Call xname on window one for client1.
Verify that no events were delivered to client1.
Verify that a correct ConfigureRequest event was delivered to client2.
Verify that no further processing occurred by comparing the window
	to our reference window.
>>CODE
Display	*client1, *client2;
Window	parent, one; 
XImage	*image;
XSetWindowAttributes	attr;
XEvent	ev;
int	numevent;

/* Create client1 and client2. */
	client1 = opendisplay();
	if (client1 == NULL) {
		delete("could not create client1");
		return;
	}
	else
		CHECK;
	client2 = opendisplay();
	if (client2 == NULL) {
		delete("could not create client2");
		return;
	}
	else
		CHECK;

/* Create a window and child window one */
	parent = defwin(client1);
	one = crechild(client1, parent, &area);

/* Save parent window image as reference image. */
	image = savimage(client1, parent);

/* Set override-redirect on window one to False. */
	attr.override_redirect = False;
	XChangeWindowAttributes(client1, one, CWOverrideRedirect, &attr);

/* Select SubstructureRedirectMask events on the parent window for client2. */
	XSelectInput(client2, parent, SubstructureRedirectMask);
	XSync(client2, True);

/* Call xname on window one for client1. */
	display = client1;
	w = one;
	XCALL;
	XSync(client2, False);

/* Verify that no events were delivered to client1. */
	numevent = getevent(client1, &ev);
	if (numevent != 0) {
		FAIL;
		report("%d unexpected %s delivered to client1",
			numevent, (numevent==1)?"event was":"events were");
		report("%sevent was %s", (numevent!=1)?"first ":"", eventname(ev.type));
		while(getevent(client1, &ev) != 0)
			report("next event was %s", eventname(ev.type));
	} else
		CHECK;

/* Verify that a correct ConfigureRequest event was delivered to client2. */
	numevent = getevent(client2, &ev);
	if (numevent != 1) {
		FAIL;
		report("Expecting a single ConfigureRequest event");
		report("Received %d events", numevent);
		if (numevent != 0) {
			report("First event was %s", eventname(ev.type));
			while(getevent(client2, &ev) != 0)
				report("next event was %s", eventname(ev.type));
		}
	} else	{
		XConfigureRequestEvent	good;

		good.type = ConfigureRequest;
		good.serial = 0;
		good.send_event = False;
		good.display = client2;
		good.parent = parent;
		good.window = one;
		good.x	= area.x;
		good.y  = area.y;
		good.width = area.width;
		good.height = area.height;
		good.border_width = width;
		good.above = None;
		good.detail= None;
		good.value_mask = CWBorderWidth;

		if ( checkevent((XEvent *)&good, &ev) )
			FAIL;
		else
			CHECK;
	}

/* Verify that no further processing occurred by comparing the window */
/* 	to our reference window. */
	if (!compsavimage(client1, parent, image)) {
		FAIL;
	} else
		CHECK;
	
	CHECKPASS(5);

>>ASSERTION Good A
When the border width actually changes, then a
.S ConfigureNotify
event is generated.
>>STRATEGY
Create client1 and client2.
Create a window and child window one
Select StructureNotifyMask events on the window one for client1. 
Select SubstructureNotifyMask events on the parent window for client2.
Call xname on window one for client1.
Verify that a single ConfigureNotify event was delivered to client1.
Verify that a single ConfigureNotify event was delivered to client2.
>>CODE
Display	*client1, *client2;
Window	parent, one; 
XEvent	ev;
int	numevent;

/* Create client1 and client2. */
	client1 = opendisplay();
	if (client1 == NULL) {
		delete("could not create client1");
		return;
	}
	else
		CHECK;
	client2 = opendisplay();
	if (client2 == NULL) {
		delete("could not create client2");
		return;
	}
	else
		CHECK;

/* Create a window and child window one */
	parent = defwin(client1);
	one = crechild(client1, parent, &area);

/* Select StructureNotifyMask events on the window one for client1.  */
	XSelectInput(client1, one, StructureNotifyMask);
	XSync(client1, True);

/* Select SubstructureNotifyMask events on the parent window for client2. */
	XSelectInput(client2, parent, SubstructureNotifyMask);
	XSync(client2, True);

/* Call xname on window one for client1. */
	display = client1;
	w = one;
	XCALL;
	XSync(client2, False);

/* Verify that a single ConfigureNotify event was delivered to client1. */
	numevent = getevent(client1, &ev);
	if (numevent != 1) {
		FAIL;
		report("Expecting a single ConfigureNotify event on client1");
		report("Received %d events", numevent);
		if (numevent != 0) {
			report("First event was %s", eventname(ev.type));
			while(getevent(client1, &ev) != 0)
				report("next event was %s", eventname(ev.type));
		}
	} else	{
		XConfigureEvent	good;

		good.type = ConfigureNotify;
		good.serial = 0;
		good.send_event = False;
		good.display = client1;
		good.event = one;
		good.window = one;
		good.x	= area.x;
		good.y  = area.y;
		good.width = area.width;
		good.height = area.height;
		good.border_width = width;
		good.above = None;
		good.override_redirect = ev.xconfigure.override_redirect;

		if ( checkevent((XEvent *)&good, &ev) )
			FAIL;
		else
			CHECK;
	}

/* Verify that a single ConfigureNotify event was delivered to client2. */
	numevent = getevent(client2, &ev);
	if (numevent != 1) {
		FAIL;
		report("Expecting a single ConfigureNotify event on client2");
		report("Received %d events", numevent);
		if (numevent != 0) {
			report("First event was %s", eventname(ev.type));
			while(getevent(client2, &ev) != 0)
				report("next event was %s", eventname(ev.type));
		}
	} else	{
		XConfigureEvent good;

		good.type = ConfigureNotify;
		good.serial = 0;
		good.send_event = False;
		good.display = client2;
		good.event = parent;
		good.window = one;
		good.x	= area.x;
		good.y  = area.y;
		good.width = area.width;
		good.height = area.height;
		good.border_width = width;
		good.above = None;
		good.override_redirect = ev.xconfigure.override_redirect;

		if ( checkevent((XEvent *)&good, &ev) )
			FAIL;
		else
			CHECK;
	}
	CHECKPASS(4);

>>ASSERTION Good A
>># I didn't see this anywhere, but it seems like it should happen.
>>#	I agree. well spotted steve.	kieron
When a call to xname
uncovers part of any window that was formerly obscured, then
either
.S Expose
events are generated or the contents are restored from backing store.
>>STRATEGY
Create a parent window, and the test windows win1 and win2. 
Select ExposureMask events on win1
Set window border on win2 to cover some of win1
Set window border on win2 to expose some of win1
Verify that Expose events were generated or that win2 was
	recovered from Backing Store.
>>CODE
Window	parent, win1, win2;
struct	area	area2;

/* Create a parent window, and the test windows win1 and win2.  */
	parent = defwin(display);
	win1 = crechild(display, parent, &area);
	setforexpose(display, win1);

	area2.x = area.x + area.width + 5;
	area2.y = area.y + 5;
	area2.width = 10;
	area2.height= 10;
	win2 = crechild(display, parent, &area2);	

/* Select ExposureMask events on win1 */
	XSelectInput(display, win1, ExposureMask);

/* Set window border on win2 to cover some of win1 */
	w = win2;
	width = 10;
	XCALL;
	
/* Set window border on win2 to expose some of win1 */
	w = win2;
	width = 2;
	XCALL;

/* Verify that Expose events were genereated or that win2 was */
/* 	recovered from Backing Store. */
	if (!exposecheck(display, win1)) {
		report("Neither Expose events or backing store procesing");
		report("could correctly restore the window contents.");
		FAIL;
	} else
		CHECK;
	CHECKPASS(1);

>>ASSERTION Bad A
>># This did read: 
>>#	When an attempt is made to set the border-width attribute of an
>>#	.S InputOnly
>>#	window to a non-zero value, then a
>>#	.S BadMatch
>>#	error occurs.
>>#					stuart.
.ER BadMatch wininputonly
>>ASSERTION Bad A
.ER BadWindow 

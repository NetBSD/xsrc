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
 * $XConsortium: lwrwdw.m,v 1.8 94/04/17 21:03:04 rws Exp $
 */
>>TITLE XLowerWindow CH03
void

Display	*display = Dsp;
Window	w;
>>EXTERN

static char	*SimpleTemplate[] = {
	".",
        "zero . (20,10) 40x40",
        "one . (30,20) 40x40",
        "two . (10,30) 40x40",
        "other1 . (75,10) 15x70",
};
static int	NSimpleTemplate = NELEM(SimpleTemplate);

static char	*Expose1Template[] = {
	".",
	"zero . (20,10) 40x40",
	"other1 . (75,10) 15x70",
};
static int	NExpose1Template = NELEM(Expose1Template);

static char	*Expose2Template[] = {
	".",
	"one . (30,30) 40x40",
	"two . (50,60) 40x40",
};
static int	NExpose2Template = NELEM(Expose2Template);

>>ASSERTION Good A
A call to xname lowers the specified window to the bottom of the stack
so that it does not obscure any sibling windows.
>>STRATEGY
Create a window hierarchy using buildtree.
Call xname on window 'one' to lower it
Verify that window 'one' became the lowest sibling window.
>>CODE
struct	buildtree	*tree;
Window	parent;

/* Create a window hierarchy using buildtree. */
	parent = defwin(display);
	tree   = buildtree(display, parent, SimpleTemplate, NSimpleTemplate);
/* Call xname on window 'one' to lower it */
	w = btntow(tree, "one");
	XCALL;
/* Verify that window 'one' became the lowest sibling window. */
	PIXCHECK(display, parent);
	CHECKPASS(1);
>>ASSERTION Good A
When a call to xname
uncovers part of any window that was formerly obscured, then
either
.S Expose
events are generated or the contents are restored from backing store.
>>STRATEGY
Create a window hierarchy using buildtree.
Call setforexpose on window 'zero' to allow Expose event checking.
Select Expose events on window 'zero'.
Call xname on windows 'one' and 'two' in order to expose window 'zero'.
Use exposecheck to ensure that the window 'zero' was restored correctly.
>>CODE
struct	buildtree	*tree1, *tree2;
Window	parent, zero, one;

/* Create a window hierarchy using buildtree. */
	parent =  defwin(display);
	tree1= buildtree(display, parent, Expose1Template, NExpose1Template);
	zero = btntow(tree1, "zero");

/* Call setforexpose on window 'zero' to allow Expose event checking. */
	setforexpose(display, zero);	
	tree2= buildtree(display, parent, Expose2Template, NExpose2Template);
	one = btntow(tree2, "one");

/* Select Expose events on window 'zero'. */
	XSelectInput(display, zero, ExposureMask);

/* Call xname on windows 'one' and 'two' in order to expose window 'zero'. */
	w = one;
	XCALL;

/* Use exposecheck to ensure that the window 'zero' was restored correctly. */
	if (!exposecheck(display, zero)) {
		report("Neither Expose events or backing store processing");
		report("could correctly restore the window contents.");
		FAIL;
	} else
		CHECK;
	CHECKPASS(1);

>>ASSERTION Good A
When the override-redirect attribute of the window is 
.S False 
and some other client has selected 
.S SubstructureRedirectMask 
on the parent window, then a
.S ConfigureRequest 
event is generated, and no further processing is performed.
>>STRATEGY
Create client1 and client2.
Create a window hierarchy for client1.
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
struct	buildtree	*c1tree;
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

/* Create a window hierarchy for client1. */
	parent = defwin(client1);
	c1tree = buildtree(client1, parent, SimpleTemplate, NSimpleTemplate);
	one = btntow(c1tree, "one");

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
		report("%sevent was %s", (numevent!=1)?"first ":"",
			eventname(ev.type));
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
		good.x	= ev.xconfigure.x;
		good.y  = ev.xconfigure.y;
		good.width = ev.xconfigure.width;
		good.height = ev.xconfigure.height;
		good.border_width = ev.xconfigure.border_width;
		good.above = Above;
		good.detail= Below;
		good.value_mask = CWStackMode;

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

>>ASSERTION Bad A
.ER BadWindow 

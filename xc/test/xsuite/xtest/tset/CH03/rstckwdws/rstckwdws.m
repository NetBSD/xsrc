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
 * $XConsortium: rstckwdws.m,v 1.8 94/04/17 21:03:15 rws Exp $
 */
>>TITLE XRestackWindows CH03
void

Display	*display = Dsp;
Window	*windows = array;
int	nwindows;
>>EXTERN

static Window	array[8];

static char	*SimpleTemplate[] = {
	".",
	"zero . (10,10) 30x30",
	"one . (35,35) 30x30",
	"two . (10,35) 30x30",
	"three . (35,10) 30x30",
	"other1 . (75,10) 15x70",
	"other2 other1 (5,5) 5x10",
	"other3 . (80,40) 5x20",
};
static int	NSimpleTemplate = NELEM(SimpleTemplate);

>>ASSERTION Good A
A call to xname restacks the windows in the
.A windows
array in the order specified, from top to bottom.
>>STRATEGY
Create a window hierarchy.
Assign members of the window array
Call xname to restack the windows
Verify that the windows were restacked in the order two, one, zero, three using pixchecking
>>CODE
struct	buildtree	*tree;
Window	parent, zero, one, two, three ;

/* Create a window hierarchy. */
	parent = defwin(display);
	tree = buildtree(display, parent, SimpleTemplate, NSimpleTemplate);
	zero = btntow(tree, "zero");
	one = btntow(tree, "one");
	two = btntow(tree, "two");
	three = btntow(tree, "three");

/* Assign members of the window array */
	array[0] = two;	/* top window */
	array[1] = one;
	array[2] = zero;
	array[3] = three;

/* Call xname to restack the windows */
	nwindows = 4;
	XCALL;

/* Verify that the windows were restacked in the order two, one, zero, three using pixchecking */
	PIXCHECK(display, parent);
	CHECKPASS(1);

>>ASSERTION Good A
The stacking order of the first window in the
.A windows
array is unaffected
with respect to other windows not in the array.
>>STRATEGY
Create a window hierarchy.
Assign members of the window array
Call xname to restack the windows
Verify that the windows were restacked such that the order became two, one, three, zero by pixchecking
>>CODE
struct	buildtree	*tree;
Window	parent, one, three ;

/* Create a window hierarchy. */
	parent = defwin(display);
	tree = buildtree(display, parent, SimpleTemplate, NSimpleTemplate);
	one = btntow(tree, "one");
	three = btntow(tree, "three");

/* Assign members of the window array */
	array[0] = one;
	array[1] = three;

/* Call xname to restack the windows */
	nwindows = 2;
	XCALL;

/* Verify that the windows were restacked such that the order became two, one, three, zero by pixchecking */
	PIXCHECK(display, parent);
	CHECKPASS(1);

>>ASSERTION Good A
The stacking order of any windows not in the
.A windows
array is not affected.
>>STRATEGY
Create a window hierarchy.
Assign members of the window array
Call xname to restack the windows
Verify that the windows were restacked such that the windows zero, one, two and three were unaffected
>>CODE
struct	buildtree	*tree;
Window	parent, other1, other3;

/* Create a window hierarchy. */
	parent = defwin(display);
	tree = buildtree(display, parent, SimpleTemplate, NSimpleTemplate);
	other1= btntow(tree, "other1");
	other3= btntow(tree, "other3");

/* Assign members of the window array */
	array[0] = other3;
	array[1] = other1;

/* Call xname to restack the windows */
	nwindows = 2;
	XCALL;

/* Verify that the windows were restacked such that the windows zero, one, two and three were unaffected */
	PIXCHECK(display, parent);
	CHECKPASS(1);

>>ASSERTION Good A
When the override-redirect attribute of a window in the 
.A windows 
array is 
.S False 
and some other client has selected 
.S SubstructureRedirectMask 
on the parent window, then a
.S ConfigureRequest 
event is generated for each such window
and no further processing is performed on that window.
>>STRATEGY
Create client1 and client2.
Create a window hierarchy for client1.
Save parent window image as reference image.
Ensure override-redirect on window one is False.
Select SubstructureRedirectMask events on the parent window for client2.
Call xname to restack window one.
Verify that no events were delivered to client1.
Verify that a correct ConfigureRequest event was delivered to client2.
Verify that no further processing occurred by comparing the window to our reference window.
>>CODE
Display	*client1, *client2;
Window	parent, zero, one, two, three; 
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
	parent = defwin(client1); trace("window parent %d", parent);
	c1tree = buildtree(client1, parent, SimpleTemplate, NSimpleTemplate);
	zero= btntow(c1tree, "zero"); trace("window zero %d", zero);
	one = btntow(c1tree, "one"); trace("window one  %d", one);
	two = btntow(c1tree, "two"); trace("window two  %d", two);
	three=btntow(c1tree, "three"); trace("window three %d", three);

/* Save parent window image as reference image. */
	image = savimage(client1, parent);

/* Ensure override-redirect on window one is False. */
	attr.override_redirect = False;
	XChangeWindowAttributes(client1, one, CWOverrideRedirect, &attr);

/* Select SubstructureRedirectMask events on the parent window for client2. */
	XSelectInput(client2, parent, SubstructureRedirectMask);
	XSync(client2, True);

/* Call xname to restack window one. */
	array[0] = zero;
	array[1] = one;
	nwindows = 2;
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
		trace("Window field was %d", ev.xany.window);
		while(getevent(client1, &ev) != 0) {
			report("next event was %s", eventname(ev.type));
			trace("Window field was %d", ev.xany.window);
		}
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
			trace("Window field was %d", ev.xany.window);
			while(getevent(client2, &ev) != 0) {
				report("next event was %s", eventname(ev.type));
				trace("Window field was %d", ev.xany.window);
			}
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
		good.above = zero;
		good.detail= Below;
		good.value_mask = CWStackMode | CWSibling;

		if ( checkevent((XEvent *)&good, &ev) )
			FAIL;
		else
			CHECK;
	}

/* Verify that no further processing occurred by comparing the window to our reference window. */
	if (!compsavimage(client1, parent, image)) {
		FAIL;
	} else
		CHECK;
	
	CHECKPASS(5);
>>ASSERTION Bad A
When a window in the
.A windows
array is not
a sibling of the previous window in the array, then for each such pair
of windows a
.S BadMatch 
error occurs.
>>STRATEGY
Create two window hierarchies
Assign members of the windows array, with one element not a sibling
Verify that a BadMatch error occurred.
Assign members of the windows array, with an element as the parent window
Verify that a BadMatch error occurred.
Assign members of the windows array, with an element as an unrelated window
Verify that a BadMatch error occurred.
>>CODE BadMatch
struct	buildtree	*tree;
Window	parent, one, two, three, other1, other2, parent2;

/* Create two window hierarchies */
	parent = defwin(display);
	tree = buildtree(display, parent, SimpleTemplate, NSimpleTemplate);
	one = btntow(tree, "one");
	two = btntow(tree, "two");
	three = btntow(tree, "three");
	other1 = btntow(tree, "other1");
	other2 = btntow(tree, "other2");

	parent2 = defwin(display);

/* Assign members of the windows array, with one element not a sibling */
	array[0] = one;
	array[1] = other1;
	array[2] = other2;	/* This is a child of other1, not a sibling */
	array[3] = two;
	array[4] = three;

/* Verify that a BadMatch error occurred. */
	nwindows = 5;
	XCALL;
	if (geterr() != BadMatch)
	{
		FAIL;
		report("child window of previous member window did not cause a BadMatch");
	} else
		CHECK;

/* Assign members of the windows array, with an element as the parent window */
	array[0] = one;
	array[1] = other1;
	array[2] = parent;	/* this is the parent window */
	array[3] = two;
	array[4] = three;

/* Verify that a BadMatch error occurred. */
	nwindows = 5;
	XCALL;
	if (geterr() != BadMatch)
	{
		FAIL;
		report("parent window of a previous window did not cause a BadMatch");
	} else
		CHECK;

/* Assign members of the windows array, with an element as an unrelated window */
	array[0] = one;
	array[1] = other1;
	array[2] = parent2;	/* this is an unrelated window */
	array[3] = two;
	array[4] = three;

/* Verify that a BadMatch error occurred. */
	nwindows = 5;
	XCALL;
	if (geterr() != BadMatch)
	{
		FAIL;
		report("unrelated window of a previous window did not cause a BadMatch");
	}
	else
		CHECK;

	CHECKPASS(3);

>>ASSERTION Bad A
When one or more entries in the 
.A windows 
array do not name valid windows,
then a 
>># This was .S BadMatch, but I think it ought to be BadWindow...  stuart
.S BadWindow
error occurs for each such window.
>>STRATEGY
Create a window hierarchy.
Assign members of the windows array, with one element as a bad window id.
Verify that a BadWindow error occurred.
>>CODE BadWindow
struct	buildtree	*tree;
Window	parent, zero, one, two, three ;

/* Create a window hierarchy. */
	parent = defwin(display);
	tree = buildtree(display, parent, SimpleTemplate, NSimpleTemplate);
	zero = btntow(tree, "zero");
	one = btntow(tree, "one");
	two = btntow(tree, "two");
	three = btntow(tree, "three");

/* Assign members of the windows array, with one element as a bad window id. */
	array[0] = zero;
	array[1] = three;
	array[2] = badwin(display);
	array[3] = one;
	array[4] = two;

/* Verify that a BadWindow error occurred. */
	nwindows = 5;
	XCALL;
	if (geterr() != BadWindow)
		FAIL;
	else
		CHECK;
	CHECKPASS(1);

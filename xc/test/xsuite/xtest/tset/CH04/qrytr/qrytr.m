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
 * $XConsortium: qrytr.m,v 1.10 94/04/17 21:03:36 rws Exp $
 */
>>TITLE XQueryTree CH04
Status

Display *display = Dsp;
Window w;
Window *root_return = &root;
Window *parent_return = &parent;
Window **children_return = &children;
unsigned int *nchildren_return = &nchildren;
>>EXTERN
/* These are the store locations for the returned data */
static	Window	root;
static	Window	parent;
static	Window	*children;
static	unsigned int	nchildren;

/* Window structure template */
/* A window with three child windows, one of which has */
/* 3 overlapping children */
static	char *QTtemplate[] = {
	".",
	"one . (10,10) 80x70",
	"onec1 one (5,5) 30x30",
	"onec2 one (15,10) 15x15",
	"onec3 one (10,15) 15x15",
	"two . (20,50) 40x20",
	"three . (55,20) 20x20",
};
static	int NQTtemplate = NELEM(QTtemplate);

static	char *QT2template[] = {
	".",
	"one . (10,10) 80x70",
	"o1 one (5,5) 40x40",
	"two . (25,25) 10x10",
	"o2 one (30,30) 40x35",
	"o3 one (20,50) 45x10",
	"o4 one (60,10) 10x10",
	"o5 one (15,35) 30x20",
};
static int NQT2template = NELEM(QT2template);

>>ASSERTION Good A
A successful call to xname returns non-zero, the root window ID in
.A root_return ,
the parent window of the specified window
.A w
in
.A parent_return , 
a pointer to the list of children windows of specified window
.A w
in
.A children_return ,
and the number of children in the list for the specified window
.A w
in
.A nchildren_return .
>># ALTERNATIVE WORDING:
>># A call to xname returns the root window ID of the specified window
>># .A w
>># in
>># .A root_return ,
>># the parent window in
>># .A parent_return ,
>># a pointer to the list of child windows in
>># .A children_return
>># which can be freed with XFree,
>># and the number of child windows in the list in
>># .A nchildren_return .
>>STRATEGY
Create a window hierarchy.
Call xname to query the window tree.
Verify that the root window, parent window, number of children and
	the children array were returned as expected.
>>CODE
Window realparent;
struct buildtree *tree;
Window one, two, three;
Window childarray[3];
int loop;

/* Create a window hierarchy. */
	realparent = defwin(display);
	tree = buildtree(display, realparent, QTtemplate, NQTtemplate);
	one = btntow(tree, "one");
	two = btntow(tree, "two");
	three = btntow(tree, "three");
	childarray[0] = btntow(tree, "onec1"); 
	childarray[1] = btntow(tree, "onec2");
	childarray[2] = btntow(tree, "onec3");
	trace("root window is %0x", DefaultRootWindow(Dsp));
	trace("parent is %0x", realparent);
	trace("one is %0x", one);
	trace("two is %0x", two);
	trace("three is %0x", three);
	for(loop=0; loop<3; loop++)
		trace("onec%d is %0x", loop, childarray[loop]);

/* Call xname to query the window tree. */
#ifdef TESTING
	w = realparent;
#else
	w = one;
#endif
	parent = None;
	root = None;
	children = (Window *)NULL;
	nchildren = -1;
	XCALL;

/* Verify that the root window, parent window, number of children and */
/* 	the children array were returned as expected. */


	if (root != DefaultRootWindow(Dsp)) {	
		FAIL;
		report("%s returned an unexpected value for the root window", TestName);
		report("Expected: %0x", DefaultRootWindow(Dsp)); 
		report("Returned: %0x", root);
	} else
		CHECK;

	if (parent != realparent) {
		FAIL;
		report("%s returned an unexpected value for the parent window", TestName);
		report("Expected: %0x", realparent);
		report("Returned: %0x", parent);
	} else
		CHECK;

	if (nchildren != 3) {
		FAIL;
		report("%s returned an unexpected number of child windows", TestName);
		report("Expected: 3");
		report("Returned: %d", nchildren);
		for(loop=0; loop<nchildren; loop++)
			report("children[%d] is  %0x", loop, children[loop]);
	} else {
		for(loop=0; loop<nchildren; loop++)
		{
			if(children[loop] != childarray[loop]) {
				FAIL;
				report("children array [%d] returned unexpected window", loop);
				report("Expected: %0x", childarray[loop]);
				report("Returned: %0x", children[loop]);
			} else
				CHECK;
		}
		XFree((char*)children);
	}

	CHECKPASS(5);

>>ASSERTION Good A
A call to xname returns the child windows
of the specified window
.A w
in
.A children_return
in current stacking order from bottommost first to topmost last. 
>>STRATEGY
Create a window hierarchy.
Call xname to query the window tree.
Verify that the children array was returned as expected.
>>CODE
Window realparent;
struct buildtree *tree;
Window one, two;
Window childarray[5];
int loop;

/* Create a window hierarchy. */
	realparent = defwin(display);
	tree = buildtree(display, realparent, QT2template, NQT2template);
	one = btntow(tree, "one");
	two = btntow(tree, "two");
	childarray[0] = btntow(tree, "o1");
	childarray[1] = btntow(tree, "o2");
	childarray[2] = btntow(tree, "o3"); 
	childarray[3] = btntow(tree, "o4");
	childarray[4] = btntow(tree, "o5");
	trace("root window is %0x", DefaultRootWindow(Dsp));
	trace("parent is %0x", realparent);
	trace("one is %0x", one);
	trace("two is %0x", two);
	for(loop=0; loop<5; loop++)
		trace("one child %d is %0x", loop, childarray[loop]);

/* Call xname to query the window tree. */
#ifdef TESTING
	w = realparent;
#else
	w = one;
#endif
	parent = None;
	root = None;
	children = (Window *)NULL;
	nchildren = -1;
	XCALL;

/* Verify that the children array was returned as expected. */
	if (nchildren != 5) {
		FAIL;
		report("%s returned an unexpected number of child windows", TestName);
		report("Expected: 5");
		report("Returned: %d", nchildren);
		for(loop=0; loop<nchildren; loop++)
			report("children[%d] is  %0x", loop, children[loop]);
	} else {
		for(loop=0; loop<nchildren; loop++)
		{
			if(children[loop] != childarray[loop]) {
				FAIL;
				report("children array [%d] returned unexpected window", loop);
				report("Expected: %0x", childarray[loop]);
				report("Returned: %0x", children[loop]);
			} else
				CHECK;
		}
		XFree((char*)children);
	}

	CHECKPASS(5);

>>ASSERTION Bad A
.ER BadWindow

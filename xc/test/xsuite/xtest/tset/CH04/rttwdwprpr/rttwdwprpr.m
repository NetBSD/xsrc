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
 * $XConsortium: rttwdwprpr.m,v 1.12 94/04/17 21:03:37 rws Exp $
 */
>>TITLE XRotateWindowProperties CH04
void

Display *display = Dsp;
Window w = defwin(display);
Atom *properties = (Atom *)&(xrwp_list[0]);
int num_prop = 4;
int npositions = 1;
>>EXTERN
#include "Xatom.h"

static Atom xrwp_list[5] = {
	0, 0, 0, 0, (unsigned)-1 };

static void
xrwp_add_property(disp, win, prop, data)
Display *disp;
Window win;
Atom prop;
unsigned long data;
{
        XChangeProperty(disp, win, prop, XA_INTEGER, 32,
                PropModeReplace,(unsigned char *)&data, 1);
        XSync(disp, True);
}

>>ASSERTION Good A
>># This assertion is rather redundant. Perhaps we should delete it.
>># If we don't delete it, we really ought to make the following changes.
A call to xname rotates
>># INSERT: the property values associated with
.A properties
>># DELETE next line
on the window
>># INSERT: of the window
.A w
and generates
>># INSERT: a
.S PropertyNotify 
>># DELETE next line
events.
>># INSERT: for each property.
>>STRATEGY
Create a window with properties, with PropertyChangeMask events selected.
Call xname to rotate the window properties.
Verify that PropertyNotify events were genereated.
Verify that the property values were rotated as expected.
>>CODE
int num_ev;
int loop;
XEvent ev, good;

/* Create a window with properties, with PropertyChangeMask events selected. */
	xrwp_list[0] = XA_INTEGER;
	xrwp_list[1] = XA_COPYRIGHT;
	for(loop=0; loop<2; loop++)
		xrwp_add_property(display, w, xrwp_list[loop],
				(unsigned long)loop);

	XSelectInput(display, w, PropertyChangeMask);

/* Call xname to rotate the window properties. */
	num_prop = 2;
	XCALL;

/* Verify that PropertyNotify events were genereated. */
	num_ev = getevent(display, &ev);
	if(num_ev != 2) {
		FAIL;
		report("%s did not cause the expected events",
			TestName);
		trace("Expected: 2 PropertyNotify events");
		trace("Returned: %d events", num_ev);
		for(; num_ev>0; ) {
			trace("Event: %s", eventname(ev.type));
			if(ev.type==PropertyNotify)
				trace("Property: %s",
					atomname(ev.xproperty.atom));
			num_ev = getevent(display, &ev);
		}
		return;
	} else {
		loop=0;
		do {
			good.type = PropertyNotify;
			good.xproperty.type = PropertyNotify;
			good.xproperty.atom = xrwp_list[loop++];
			good.xproperty.display = display;
			good.xproperty.window = w;
			good.xproperty.state = PropertyNewValue;
			if (checkevent(&good, &ev)) {
				FAIL;
			} else
				CHECK;
		} while(getevent(display, &ev));
	}

/* Verify that the property values were rotated as expected. */
	for(loop=0; loop<2; loop++) {
		Atom type;
		int format;
		unsigned long nitems;
		unsigned long after;
		unsigned char *value;

		XGetWindowProperty(display, w, xrwp_list[loop], 0, 1, False,
				XA_INTEGER, &type, &format, &nitems, &after,
					&value);

		if (type != XA_INTEGER || format != 32 ||
			nitems != 1 || after != 0) {
			delete("XGetWindowProperty returned unexpected values");
			trace("loop is %d", loop);
			trace("type is %s (expected XA_INTEGER)",
				atomname(type));
			trace("format is %d (expected 32)", format);
			trace("nitems is %u (expected 1)", nitems);
			trace("after is %u (expected 0)", after);
			return;
		} else {
			unsigned long tmp;

			tmp = *(unsigned long *)value;
			if (tmp != ((loop+1) % 2) ) {
				FAIL;
				report("%s did not rotate values as expected",
						TestName);
				trace("expected value %u", (loop+1) %2);
				trace("returned value %u", tmp);
			} else {
				CHECK;
				trace("expected value %u returned", tmp);
			}
		}
	}
			
	CHECKPASS(4);

>>ASSERTION Good A
When
.A npositions
mod
.A num_prop
is non-zero, then a call to xname reorders the values associated with the
.A properties
such that the value associated with a
.A properties
[I] becomes the value associated with the
.A properties
[ I + 
.A npositions ]
mod
.A num_prop ,
and a
.S PropertyNotify
event is generated for each member of the
.A properties
array in the array order.
>>STRATEGY
Create a window with properties, with PropertyChangeMask events selected.
Call xname to rotate the window properties.
Verify that PropertyNotify events were genereated.
Verify that the property values were rotated as expected.
>>CODE
int num_ev;
int loop;
XEvent ev, good;

/* Create a window with properties, with PropertyChangeMask events selected. */
	xrwp_list[0] = XA_INTEGER;
	xrwp_list[1] = XA_COPYRIGHT;
	xrwp_list[2] = XA_WM_HINTS;
	xrwp_list[3] = XA_NOTICE;
	for(loop=0; loop<num_prop; loop++)
		xrwp_add_property(display, w, xrwp_list[loop],
				(unsigned long)loop);

	XSelectInput(display, w, PropertyChangeMask);

/* Call xname to rotate the window properties. */
	XCALL;

/* Verify that PropertyNotify events were genereated. */
	num_ev = getevent(display, &ev);
	if(num_ev != num_prop) {
		FAIL;
		report("%s did not cause the expected events",
			TestName);
		trace("Expected: %d PropertyNotify events", num_prop);
		trace("Returned: %d events", num_ev);
		for(; num_ev>0; ) {
			trace("Event: %s", eventname(ev.type));
			if(ev.type==PropertyNotify)
				trace("Property: %s",
					atomname(ev.xproperty.atom));
			num_ev = getevent(display, &ev);
		}
		return;
	} else {
		loop=0;
		do {
			good.type = PropertyNotify;
			good.xproperty.type = PropertyNotify;
			good.xproperty.atom = xrwp_list[loop++];
			good.xproperty.display = display;
			good.xproperty.window = w;
			good.xproperty.state = PropertyNewValue;
			if (checkevent(&good, &ev)) {
				FAIL;
			} else
				CHECK;
		} while(getevent(display, &ev));
	}

/* Verify that the property values were rotated as expected. */
	for(loop=0; loop<num_prop; loop++) {
		Atom type;
		int format;
		unsigned long nitems;
		unsigned long after;
		unsigned char *value;
		XGetWindowProperty(display, w, xrwp_list[loop], 0, 1, False,
				XA_INTEGER, &type, &format, &nitems, &after,
					&value);

		if (type != XA_INTEGER || format != 32 ||
			nitems != 1 || after != 0) {
			delete("XGetWindowProperty returned unexpected values");
			trace("loop is %d", loop);
			trace("type is %s (expected XA_INTEGER)",
				atomname(type));
			trace("format is %d (expected 32)", format);
			trace("nitems is %u (expected 1)", nitems);
			trace("after is %u (expected 0)", after);
			return;
		} else {
			unsigned long tmp;

			/*
			 * The new value of prop[loop] is the value of the old
			 * prop[loop-npositions mod num_prop]
			 */
			tmp = *(unsigned long *)value;
			if (tmp != ((loop-npositions + num_prop) % num_prop) ) {
				FAIL;
				report("%s did not rotate values as expected",
					TestName);
				trace("expected value %u",
					(loop-npositions + num_prop) % num_prop);
				trace("returned value %u", tmp);
			} else {
				CHECK;
				trace("expected value %u returned", tmp);
			}
		}
	}

	CHECKPASS(2*num_prop);
			
>>ASSERTION Bad A
When an atom occurs more than once in the
.A properties
list, then a call to xname changes no window
>># DELETE next line
.A properties, and a
>># INSERT: property values, and a 
.S BadMatch 
error occurs.
>>STRATEGY
Create a window with properties, and PropertyChangeMask events selected.
Ensure that the properties array contains a duplicate member.
Call xname to rotate the window properties.
Verify that a BadMatch error occurred.
Verify that no property notify events were raised.
Verify that the property values were not modified.
>>CODE BadMatch
XEvent ev;
int loop;
int num_ev;

/* Create a window with properties, and PropertyChangeMask events selected. */
	xrwp_list[0] = XA_INTEGER;
	xrwp_list[1] = XA_COPYRIGHT;
	xrwp_list[2] = XA_WM_HINTS;
	for(loop=0; loop<3; loop++)
		xrwp_add_property(display, w, xrwp_list[loop],
				(unsigned long)loop);

	XSelectInput(display, w, PropertyChangeMask);

/* Ensure that the properties array contains a duplicate member. */
	xrwp_list[3] = XA_COPYRIGHT;

/* Call xname to rotate the window properties. */
	XCALL;

/* Verify that a BadMatch error occurred. */
	if (geterr() != BadMatch) {
		FAIL;
		report("%s did not generate a BadMatch", TestName);
		report("when the property list contained a duplicate");
	} else
		CHECK;

/* Verify that no property notify events were raised. */
	num_ev =getevent(display, &ev);
	if(num_ev != 0) {
		XEvent ev;
		FAIL;
		report("%s caused unexpected events",
			TestName);
		trace("Expected: no events");
		trace("Returned: %d events", num_ev);
		for(; num_ev>0; ) {
			trace("Event: %s", eventname(ev.type));
			if(ev.type==PropertyNotify)
				trace("Property: %s",
					atomname(ev.xproperty.atom));
			num_ev = getevent(display, &ev);
		}
		return;
	} else 
		CHECK;

/* Verify that the property values were not modified. */
	for(loop=0; loop<3; loop++) {
		Atom type;
		int format;
		unsigned long nitems;
		unsigned long after;
		unsigned char *value;
		XGetWindowProperty(display, w, xrwp_list[loop], 0, 1, False,
				XA_INTEGER, &type, &format, &nitems, &after,
					&value);

		if (type != XA_INTEGER || format != 32 ||
			nitems != 1 || after != 0) {
			delete("XGetWindowProperty returned unexpected values");
			trace("loop is %d", loop);
			trace("type is %s (expected XA_INTEGER)",
				atomname(type));
			trace("format is %d (expected 32)", format);
			trace("nitems is %u (expected 1)", nitems);
			trace("after is %u (expected 0)", after);
			return;
		} else {
			unsigned long tmp;

			tmp = *(unsigned long *)value;
			if (tmp != loop ) {
				FAIL;
				report("%s changed the property values",
					TestName);
				report("when no change was expected");
				trace("expected value %u", loop);
				trace("returned value %u", tmp);
			} else {
				CHECK;
				trace("expected property value %u", tmp);
			}
		}
	}

	CHECKPASS(5);
			
>>ASSERTION Bad A
When an atom in the 
.A properties
list is not a property of the specified window
.A w ,
then a call to xname
changes no window
>># DELETE next line
.A propreties ,
>># INSERT: property values,
and a 
.S BadMatch
error occurs.
>>STRATEGY
Create a window with properties, and PropertyChangeMask events selected.
Ensure the properties array has a member that is not a window property.
Call xname to rotate the window properties.
Verify that a BadMatch error occurred.
Verify that no property notify events were raised.
Verify that the property values were not modified.
>>CODE BadMatch
int loop;
XEvent ev;
int num_ev;

/* Create a window with properties, and PropertyChangeMask events selected. */
	xrwp_list[0] = XA_INTEGER;
	xrwp_list[1] = XA_COPYRIGHT;
	xrwp_list[2] = XA_WM_HINTS;
	for(loop=0; loop<3; loop++)
		xrwp_add_property(display, w, xrwp_list[loop],
				(unsigned long)loop);

	XSelectInput(display, w, PropertyChangeMask);

/* Ensure the properties array has a member that is not a window property. */
	xrwp_list[3] = XA_NOTICE;

/* Call xname to rotate the window properties. */
	XCALL;

/* Verify that a BadMatch error occurred. */
	if (geterr() != BadMatch) {
		FAIL;
		report("%s did not generate a BadMatch", TestName);
		report("when the property list contained a duplicate");
	} else
		CHECK;

/* Verify that no property notify events were raised. */
	num_ev =getevent(display, &ev);
	if(num_ev != 0) {
		XEvent ev;
		FAIL;
		report("%s caused unexpected events",
			TestName);
		trace("Expected: no events");
		trace("Returned: %d events", num_ev);
		for(; num_ev>0; ) {
			trace("Event: %s", eventname(ev.type));
			if(ev.type==PropertyNotify)
				trace("Property: %s",
					atomname(ev.xproperty.atom));
			num_ev = getevent(display, &ev);
		}
		return;
	} else 
		CHECK;

/* Verify that the property values were not modified. */
	for(loop=0; loop<3; loop++) {
		Atom type;
		int format;
		unsigned long nitems;
		unsigned long after;
		unsigned char *value;
		XGetWindowProperty(display, w, xrwp_list[loop], 0, 1, False,
				XA_INTEGER, &type, &format, &nitems, &after,
					&value);

		if (type != XA_INTEGER || format != 32 ||
			nitems != 1 || after != 0) {
			delete("XGetWindowProperty returned unexpected values");
			trace("loop is %d", loop);
			trace("type is %s (expected XA_INTEGER)",
				atomname(type));
			trace("format is %d (expected 32)", format);
			trace("nitems is %u (expected 1)", nitems);
			trace("after is %u (expected 0)", after);
			return;
		} else {
			unsigned long tmp;

			tmp = *(unsigned long *)value;
			if (tmp != loop ) {
				FAIL;
				report("%s changed the property values",
					TestName);
				report("when no change was expected");
				trace("expected value %u", loop);
				trace("returned value %u", tmp);
			} else {
				CHECK;
				trace("expected property value %u", tmp);
			}
		}
	}

	CHECKPASS(5);

>>ASSERTION Bad A
>># THIS IS NEW!!! NOT originally submitted to mit
When an atom in the 
.A properties
list is a bad atom,
then a call to xname
changes no window
property values, and a 
.S BadAtom
error occurs.
>>STRATEGY
Create a window with properties, and PropertyChangeMask events selected.
Ensure the properties array has a member that is a bad atom (-1).
Call xname to rotate the window properties.
Verify that a BadMatch error occurred.
Verify that no property notify events were raised.
Verify that the property values were not modified.
>>CODE BadAtom
int loop;
XEvent ev;
int num_ev;

/* Create a window with properties, and PropertyChangeMask events selected. */
	xrwp_list[0] = XA_INTEGER;
	xrwp_list[1] = XA_COPYRIGHT;
	xrwp_list[2] = XA_WM_HINTS;
	for(loop=0; loop<3; loop++)
		xrwp_add_property(display, w, xrwp_list[loop],
				(unsigned long)loop);

	XSelectInput(display, w, PropertyChangeMask);

/* Ensure the properties array has a member that is a bad atom (-1). */
	xrwp_list[3] = -1L;

/* Call xname to rotate the window properties. */
	XCALL;

/* Verify that a BadMatch error occurred. */
	if (geterr() != BadAtom) {
		FAIL;
		report("%s did not generate a BadAtom", TestName);
		report("when the property list contained -1");
	} else
		CHECK;

/* Verify that no property notify events were raised. */
	num_ev =getevent(display, &ev);
	if(num_ev != 0) {
		XEvent ev;
		FAIL;
		report("%s caused unexpected events",
			TestName);
		trace("Expected: no events");
		trace("Returned: %d events", num_ev);
		for(; num_ev>0; ) {
			trace("Event: %s", eventname(ev.type));
			if(ev.type==PropertyNotify)
				trace("Property: %s",
					atomname(ev.xproperty.atom));
			num_ev = getevent(display, &ev);
		}
		return;
	} else 
		CHECK;

/* Verify that the property values were not modified. */
	for(loop=0; loop<3; loop++) {
		Atom type;
		int format;
		unsigned long nitems;
		unsigned long after;
		unsigned char *value;
		XGetWindowProperty(display, w, xrwp_list[loop], 0, 1, False,
				XA_INTEGER, &type, &format, &nitems, &after,
					&value);

		if (type != XA_INTEGER || format != 32 ||
			nitems != 1 || after != 0) {
			delete("XGetWindowProperty returned unexpected values");
			trace("loop is %d", loop);
			trace("type is %s (expected XA_INTEGER)",
				atomname(type));
			trace("format is %d (expected 32)", format);
			trace("nitems is %u (expected 1)", nitems);
			trace("after is %u (expected 0)", after);
			return;
		} else {
			unsigned long tmp;

			tmp = *(unsigned long *)value;
			if (tmp != loop ) {
				FAIL;
				report("%s changed the property values",
					TestName);
				report("when no change was expected");
				trace("expected value %u", loop);
				trace("returned value %u", tmp);
			} else {
				CHECK;
				trace("expected property value %u", tmp);
			}
		}
	}

	CHECKPASS(5);

>>ASSERTION Bad A
.ER BadWindow

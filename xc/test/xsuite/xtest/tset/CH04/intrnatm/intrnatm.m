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
 * $XConsortium: intrnatm.m,v 1.8 94/04/17 21:03:34 rws Exp $
 */
>>TITLE XInternAtom CH04
Atom

Display *display = Dsp;
char *atom_name;
Bool only_if_exists;
>>EXTERN
/* Need to pick up the XA_RECTANGLE declaration */
#include "Xatom.h"

#define XIA_ATOM_NAME_1 "XTEST_ATOM_ONE"
#define XIA_ATOM_NAME_2 "XTEST_ATOM_TWO"
#define XIA_ATOM_NAME_3a "XTEST_ATOM_THREE"
#define XIA_ATOM_NAME_3b "Xtest_Atom_Three"
#define XIA_ATOM_NAME_3c "xTeSt_aToM_tHrEe"
#define XIA_ATOM_NAME_4 "XTEST_ATOM_FOUR"
#define XIA_ATOM_NAME_5 "XTEST_ATOM_FIVE"
>>ASSERTION Good A
A call to xname
returns the atom identifier associated with the specified
.A atom_name .
>>STRATEGY
Call xname to obtain the atom representation associated with "RECTANGLE".
Verify that the atom returned was as expected.
>>CODE
Atom atom_return;

/* Call xname to obtain the atom representation associated with "RECTANGLE". */
	atom_name = "RECTANGLE";
	only_if_exists = True;
	atom_return = XCALL;

/* Verify that the atom returned was as expected. */
	if (atom_return != XA_RECTANGLE) {
		FAIL;
		report("%s did not return the expected value",
			TestName);
		report("Expected value: %u (%s)", (unsigned long)XA_RECTANGLE,
			atomname(XA_RECTANGLE));
		report("Returned value: %u (%s)", (unsigned long)atom_return,
			atomname(atom_return));
	} else
		CHECK;

	CHECKPASS(1);

>>ASSERTION Good A
When xname is called with
.A only_if_exists
set to
.S True
and the specified
.A atom_name
is not associated with an atom, then
.S None
is returned.
>>STRATEGY
Call xname with an atom_name not associated with an atom,
	with only_if_exists True.
Verify that None was returned.
>>CODE
Atom atom_return;

/* Call xname with an atom_name not associated with an atom, */
/* 	with only_if_exists True. */
	atom_name=XIA_ATOM_NAME_1;
	only_if_exists = True;
	atom_return = XCALL;

/* Verify that None was returned. */
	if (atom_return != (Atom)None) {
		FAIL;
		report("%s did not return the expected value", TestName);
		report("Expected value: %u (None)", None);
		report("Returned value: %u (%s)", (unsigned long)atom_return,
			atomname(atom_return));
	} else
		CHECK;
	CHECKPASS(1);

>>ASSERTION Good A
When xname is called with
.A only_if_exists
set to
.S False
and the specified
.A atom_name
is not associated with an atom, then
an atom is created and its atom identifer is returned.
>>STRATEGY
Call xname to ensure that atom_name is not associated with an atom.
Call xname with an atom_name not associated with an atom,
	with only_if_exists False.
Verify that None was not returned.
Verify that the returned atom id corresponds to atom_name.
>>CODE
Atom atom_return;
char *check_name;

/* Call xname to ensure that atom_name is not associated with an atom. */
	atom_name=XIA_ATOM_NAME_2;
	only_if_exists = True;
	atom_return = XCALL;

	if(atom_return != None) {
		delete("Atom name %s has an identifier associated with it (%u)",
			XIA_ATOM_NAME_2, atom_return);
		report("Possible reason: Server was not reset");
		report("after a previous test run");
		return;
	} else
		CHECK;

/* Call xname with an atom_name not associated with an atom, */
/* 	with only_if_exists False. */
	atom_name=XIA_ATOM_NAME_2;
	only_if_exists = False;
	atom_return = XCALL;

/* Verify that None was not returned. */
	trace("Atom id returned: %u (%s)", (unsigned long)atom_return,
		atomname(atom_return));

	if (atom_return == (Atom)None) {
		FAIL;
		report("%s did not return the expected value", TestName);
		report("Expected value: an atom identifier");
		report("Returned value: %u (None)", None);
		return;
	} else
		CHECK;

/* Verify that the returned atom id corresponds to atom_name. */
	check_name = XGetAtomName(display, atom_return);
	if (check_name == NULL) {
		delete("XGetAtomName returned NULL");
		return;
	} else
		CHECK;

	if(strcmp(check_name, XIA_ATOM_NAME_2) != 0) {
		FAIL;
		report("%s did not return an atom id associated with atom_name",
			TestName);
		report("Atom returned was %u", (unsigned long)atom_return);
		report("Expected associated name: %s", XIA_ATOM_NAME_2);
		report("Returned associated name: %s", check_name);
	} else
		CHECK;

	XFree(check_name);
	CHECKPASS(4);

>>ASSERTION Good A
On a call to xname, the case of the
.A atom_name
string is significant for designating or identifying atoms.
>>STRATEGY
Call xname to ensure that atom XIA_ATOM_NAME_3a does not exist.
Call xname to create an atom XIA_ATOM_NAME_3a.
Call xname to create an atom XIA_ATOM_NAME_3b.
Verify the atom identifiers are distinct.
Call xname to obtain the atom id associated with XIA_ATOM_NAME_3c.
Verify the atom identifier is None.
>>CODE
Atom atom_a, atom_b, atom_return;

/* Call xname to ensure that atom XIA_ATOM_NAME_3a does not exist. */
	atom_name=XIA_ATOM_NAME_3a;
	only_if_exists = True;
	atom_return = XCALL;

	if(atom_return != None) {
		delete("Atom name %s has an identifier associated with it (%u)",
			XIA_ATOM_NAME_3a, atom_return);
		report("Possible reason: Server was not reset");
		report("after a previous test run");
		return;
	} else
		CHECK;

/* Call xname to create an atom XIA_ATOM_NAME_3a. */
	atom_name=XIA_ATOM_NAME_3a;
	only_if_exists = False;
	atom_a = XCALL;

	if (atom_a == (Atom)None) {
		delete("%s did not return the expected value", TestName);
		report("Expected value: an atom identifier for %s",
			XIA_ATOM_NAME_3a);
		report("Returned value: %u (None)", None);
		return;
	} else
		CHECK;
	trace("XIA_ATOM_NAME_3a (%s) is %u", XIA_ATOM_NAME_3a,
		(unsigned long)atom_a);

/* Call xname to create an atom XIA_ATOM_NAME_3b. */
	atom_name=XIA_ATOM_NAME_3b;
	only_if_exists = False;
	atom_b = XCALL;

	if (atom_b == (Atom)None) {
		delete("%s did not return the expected value", TestName);
		report("Expected value: an atom identifier for %s",
			XIA_ATOM_NAME_3b);
		report("Returned value: %u (None)", None);
		return;
	} else
		CHECK;
	trace("XIA_ATOM_NAME_3b (%s) is %u", XIA_ATOM_NAME_3b,
		(unsigned long)atom_b);

/* Verify the atom identifiers are distinct. */
	if(atom_a == atom_b) {
		FAIL;
		report("%s did not return distinct atom identifiers for",
			TestName);
		report("atom %s and atom %s.",
			XIA_ATOM_NAME_3a, XIA_ATOM_NAME_3b);
	} else
		CHECK;

/* Call xname to obtain the atom id associated with XIA_ATOM_NAME_3c. */
	atom_name=XIA_ATOM_NAME_3c;
	only_if_exists = True;
	atom_return = XCALL;

/* Verify the atom id associated with XIA_ATOM_NAME_3c is None. */
	if (atom_return != (Atom)None) {
		char *check_name;
		check_name=XGetAtomName(display, atom_return);
		FAIL;
		report("%s did not return the expected value", TestName);
		report("Expected value: %u (None)", None);
		report("Returned value: %u (%s)", (unsigned long)atom_return,
			(check_name==NULL?"NO_NAME":check_name));
		XFree(check_name);
	} else
		CHECK;

	CHECKPASS(5);

>>ASSERTION Good A
Atoms created by a call to xname will remain defined after the client's
connection closes.
>>STRATEGY
Create a new client.
Call xname to ensure that XIA_ATOM_NAME_4 is not associated with an atom.
Call xname to create atom XIA_ATOM_NAME_4 on display2.
Close display2.
Allow sufficient time for the server to register the close display
Verify that atom XIA_ATOM_NAME_4 remains defined.
>>CODE
Atom atom_return, atom_return2;
Display *display2;

/* Create a new client. */
	if (config.display == (char *) NULL) {
		delete("config.display not set");
		return;
	}
	else
		CHECK;

	display2 = XOpenDisplay(config.display);
	if (display2 == (Display *)NULL) {
		delete("Could not open display for display2");
		return;
	} else
		CHECK;

/* Call xname to ensure that atom_name is not associated with an atom. */
	display = display2;
	atom_name=XIA_ATOM_NAME_4;
	only_if_exists = True;
	atom_return = XCALL;

	if(atom_return != None) {
		delete("Atom name %s has an identifier associated with it (%u)",
			XIA_ATOM_NAME_4, atom_return);
		report("Possible reason: Server was not reset");
		report("after a previous test run");
		return;
	} else
		CHECK;

/* Call xname to create a new atom on display2. */
	atom_name=XIA_ATOM_NAME_4;
	only_if_exists = False;
	atom_return = XCALL;

	trace("Atom id %u returned for %s", (unsigned long)atom_return,
		XIA_ATOM_NAME_4);

	if (atom_return == (Atom)None) {
		delete("%s did not return the expected value", TestName);
		report("Expected value: an atom identifier");
		report("Returned value: %u (None)", None);
		return;
	} else
		CHECK;

/* Close display2. */
	(void) XCloseDisplay(display2);

/* Allow sufficient time for the server to register the close display */
	reset_delay();

/* Verify that atom XIA_ATOM_NAME_4 remains defined. */
	display = Dsp;
	atom_name=XIA_ATOM_NAME_4;
	only_if_exists = True;
	atom_return2 = XCALL;

	trace("Atom id %u returned for %s", (unsigned long)atom_return,
		XIA_ATOM_NAME_4);

	if (atom_return2 != atom_return) {
		FAIL;
		report("%s did not return the expected value", TestName);
		report("Expected value: %u (%s)", atom_return,
			XIA_ATOM_NAME_4);
		report("Returned value: %u", (unsigned long)atom_return2);
	} else
		CHECK;

	CHECKPASS(5);

>>ASSERTION Good A
When the last connection to the server closes, then atoms created by
a call to xname will become undefined.
>>STRATEGY
Ensure we can open a new display.
Create an atom by calling xname.
Close the last connection to the server.
Allow sufficient time for the server to register the close display
Open a new connection to the server.
Verify that the atom is undefined.
>>CODE
Atom atom_return;

/* Ensure we can open a new display. */
	if (config.display == (char *) NULL) {
		delete("config.display not set");
		return;
	}
	else
		CHECK;

/* Create an atom by calling xname. */
	atom_name=XIA_ATOM_NAME_5;
	only_if_exists = False;
	atom_return = XCALL;

	trace("Atom id %u returned for %s", (unsigned long)atom_return,
		XIA_ATOM_NAME_4);

#ifdef TESTING
	atom_return = None;
#endif
	if (atom_return == (Atom)None) {
		delete("%s did not return the expected value", TestName);
		report("Expected value: an atom identifier");
		report("Returned value: %u (None)", None);
		return;
	} else
		CHECK;

/* Close the last connection to the server. */
	(void) XCloseDisplay(display);

/* Allow sufficient time for the server to register the close display */
	reset_delay();

/* Open a new connection to the server. */
	Dsp = XOpenDisplay(config.display);
	if (Dsp == (Display *)NULL) {
		int i;
		delete("Could not open display");
		cancelrest("Could not open display");
		return;
	} else
		CHECK;

/* Verify that the atom is undefined. */
	display = Dsp;
	atom_name=XIA_ATOM_NAME_5;
	only_if_exists = True;
	atom_return = XCALL;

	trace("Atom id %u returned for %s", (unsigned long)atom_return,
		XIA_ATOM_NAME_5);

	if (atom_return != (Atom)None) {
		char *check_name;
		FAIL;
		report("Closing last connection to server did not");
		report("clear the defined atom %s", XIA_ATOM_NAME_5);
		report("Expected value: %u (None)", (unsigned long)None);
		check_name=XGetAtomName(display, atom_return);
		report("Returned value: %u (%s)", (unsigned long)atom_return,
			(check_name==NULL?"NO_NAME":check_name));
		report("Possible reason: Other connections open to server");
		report("during test run");
		XFree(check_name);
	} else
		CHECK;

	CHECKPASS(4);
		
>>ASSERTION Bad A
.ER BadAlloc
>>ASSERTION Bad A
.ER BadValue only_if_exists True False

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
 * $XConsortium: gtatmnm.m,v 1.6 94/04/17 21:03:29 rws Exp $
 */
>>TITLE XGetAtomName CH04
char *

Display *display = Dsp;
Atom atom;
>>EXTERN
#include "Xatom.h"

static struct xga_struct {
	char *name;
	Atom atom;
} xga_list[] = {
	 "PRIMARY", XA_PRIMARY, 
	 "CUT_BUFFER0", XA_CUT_BUFFER0, 
	 "RECTANGLE", XA_RECTANGLE,
	 "COPYRIGHT", XA_COPYRIGHT,
};
static int xga_nlist = NELEM(xga_list);

>>ASSERTION Good A
A call to xname returns the name, which can be freeed with XFree,
associated with the specified
.A atom .
>>STRATEGY
For some predefined atoms:
	Call xname to obtain the name associated with the atom.
	Verify the strings returned were as expected.
>>CODE
char *ret_str;
int l;

/* For some predefined atoms: */
	for(l=0; l<xga_nlist; l++) {

/* 	Call xname to obtain the name associated with the atom. */
		atom = xga_list[l].atom;
		trace("checking atom %d (%s)", atom, atomname(atom));
		ret_str = XCALL;

		if (ret_str == NULL) {
			FAIL;
			report("%s returned a null string with atom name %s",
				TestName, xga_list[l].name);
			continue;
		} else
			CHECK;

/* 	Verify the strings returned were as expected. */
		if (strcmp(xga_list[l].name, ret_str) != 0 ) {
			FAIL;
			report("%s returned an unexpected string");
			report("Expected: '%s'", xga_list[l].name);
			report("Returned: '%s'", ret_str);
		} else
			CHECK;
		XFree(ret_str);
	}

	CHECKPASS(2*xga_nlist);
>>ASSERTION Bad A
.ER BadAtom

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
 * $XConsortium: gtwmprtcls.m,v 1.7 94/04/17 21:08:49 rws Exp $
 */
>>TITLE XGetWMProtocols CH09
Status
XGetWMProtocols(display, w, protocols_return, count_return)
Display	*display = Dsp;
Window	w = DRW(Dsp);
Atom	**protocols_return = &protret;
int	*count_return = &countret;
>>EXTERN
#include	"Xatom.h"
Atom	*protret;
int	countret;
>>ASSERTION Good A
When the WM_PROTOCOLS property is set on the window
.A w ,
is of
.M type
ATOM, and is of
.M format
32, then a call to xname returns the list of atoms stored in the property,
which can be freed with
.S XFree ,
in the
.A protocols_return
argument, the number of atoms in the
.A count_return
argument and returns non-zero.
>>STRATEGY
Create a window using XCreateWindow.
Set the WM_PROTOCOLS property using XSetWMProtocols.
Obtain the value of the WM_PROTOCOLS property using XGetWMProtocols.
Verify that the call did not return False.
Verify that the value was correct.
Free the allocated memory using XFree.
>>CODE
Status		status;
XVisualInfo	*vp;
int		nats = 5;
Atom		prots[5];
Atom		*retprots = (Atom *) NULL;
Atom		at, *atp;
int 		i, cntret;


	for(i=0, at = XA_LAST_PREDEFINED; i<nats; i++)
		prots[i] = (int) --at;

	resetvinf(VI_WIN);
	nextvinf(&vp);
	w = makewin(display, vp);

	XSetWMProtocols(display, w, prots, nats);

	protocols_return = &retprots;
	count_return = &cntret;
	status = XCALL;

	if(status == False) {
		delete("%s() returned False.", TestName);
		return;
	} else
		CHECK;

	if(cntret != nats) {
		report("The number of elements set for the WM_PROTOCOLS property was %d instead of %d", cntret, nats);
		FAIL;
	} else
		CHECK;

	if(retprots == (Atom *) NULL) {
		report("The WM_PROTOCOLS property had no associated value.");
		FAIL;		
	} else {

		CHECK;
		for(i = 0, atp = retprots; i<nats; i++, atp++)
			if( *atp != prots[i]) {
				report("Element %d of the WM_PROTOCOLS value was %lu instead of %lu", i+1, (long) *atp, (long) prots[i]);
				FAIL;
			} else
				CHECK;
		
		XFree((char*)retprots);
	}

	CHECKPASS(nats + 3);


>>ASSERTION Bad B 1
When the atom name \(lqWM_PROTOCOLS\(rq cannot be interned,
then a call to xname does not set the
.A protocols_return
or
.A count_return
arguments and returns zero.
>>ASSERTION Good A
When the WM_PROTOCOLS property is not set on the window
.A w ,
or is not of
.M type
ATOM, or is not of
.M format
32, then a call to xname does not set the
.A protocols_return
or
.A count_return
arguments and returns zero.
>>STRATEGY
Create a window with XCreateWindow.
Insure that the name "WM_PROTOCOLS" is interned using XSetWMProtocols.
Obtain the WM_PROTOCOLS atom using XInternAtom.

Create a window with XCreateWindow.
Initialise the protocols_return and count_return arguments.
Obtain the value of the WM_PROTOCOLS property with XGetWMProtocols.
Verify that the call returned False.
Verify that protocols_return and count_return arguments were unchanged.

Create a window with XCreateWindow.
Set the WM_PROTOCOLS property with format 16 and type ATOM using XChangeProperty.
Initialise the protocols_return and count_return arguments.
Obtain the value of the WM_PROTOCOLS property with XGetWMProtocols.
Verify that the call returned False.
Verify that protocols_return and count_return arguments were unchanged.

Create a window with XCreateWindow.
Initialise the protocols_return and count_return arguments.
Set the WM_PROTOCOLS property with format 32 type STRING using XChangeProperty.
Obtain the value of the WM_PROTOCOLS property with XGetWMProtocols.
Verify that the call returned False.
Verify that protocols_return and count_return arguments were unchanged.

>>CODE
Status		status;
Atom		at, xa_wm_protocols;
Atom		*rat;
int		rcnt;
XVisualInfo	*vp;

	resetvinf(VI_WIN);
	nextvinf(&vp);

	protocols_return = &rat;
	count_return = &rcnt;
	w = makewin(display, vp);
	
	if( (at = XInternAtom(display, XT_TIMESTAMP, False)) == None) {
		delete("The \"%s\" string was not interned.", XT_TIMESTAMP);
		return;
	} else
		CHECK;

	if(XSetWMProtocols(display, w, &at, 1) == False) {
		delete("XSetWMProtocols() returned False.");
		return;
	} else
		CHECK;

	if( (xa_wm_protocols = XInternAtom(display, "WM_PROTOCOLS", True)) == None) {
		delete("The \"WM_PROTOCOLS\" string was not interned.");
		return;
	} else
		CHECK;

	w = makewin(display, vp);

/* Property unset */

	rat = (Atom *) -1;
	rcnt = -1;
	status = XCALL;

	if(status != False) {
		report("%s() did not return False when the WM_PROTOCOLS property was not set.", TestName);
		FAIL;
	} else
		CHECK;

	if( rat != (Atom *) -1) {
		report("Atom list pointer variable was updated when the WM_PROTOCOLS property was not set.");
		FAIL;
	} else
		CHECK;

	if( rcnt != -1) {
		report("Atom count variable was updated when the WM_PROTOCOLS property was not set.");
		FAIL;
	} else
		CHECK;



	w = makewin(display, vp);


/* format 16 */
 	XChangeProperty(display, w, xa_wm_protocols, XA_ATOM, 16, PropModeReplace, (unsigned char *) &at, 1);

	rat = (Atom *) -1;
	rcnt = -1;
	status = XCALL;

	if(status != False) {
		report("%s() did not return False when the WM_PROTOCOLS property had format 16.", TestName);
		FAIL;
	} else
		CHECK;

	if( rat != (Atom *) -1) {
		report("Atom list pointer variable was updated when the WM_PROTOCOLS property had format 16.");
		FAIL;
	} else
		CHECK;

	if( rcnt != -1) {
		report("Atom count variable was updated when the WM_PROTOCOLS property had format 16.");
		FAIL;
	} else
		CHECK;

	w = makewin(display, vp);

/* Type STRING */
 	XChangeProperty(display, w, xa_wm_protocols, XA_STRING, 32, PropModeReplace, (unsigned char *) &at, 1);

	rat = (Atom *) -1;
	rcnt = -1;
	status = XCALL;

	if(status != False) {
		report("%s() did not return False when the WM_PROTOCOLS property had type STRING.", TestName);
		FAIL;
	} else
		CHECK;

	if( rat != (Atom *) -1) {
		report("Atom list pointer variable was updated when the WM_PROTOCOLS property had type STRING.");
		FAIL;
	} else
		CHECK;

	if( rcnt != -1) {
		report("Atom count variable was updated when the WM_PROTOCOLS property had type STRING.");
		FAIL;
	} else
		CHECK;

	CHECKPASS(12);

>>ASSERTION Bad A
.ER BadWindow 
>># Kieron	Action	Review

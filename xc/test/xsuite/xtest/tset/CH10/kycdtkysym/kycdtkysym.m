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
 * $XConsortium: kycdtkysym.m,v 1.7 94/04/17 21:09:48 rws Exp $
 */
>>TITLE XKeycodeToKeysym CH10
KeySym
XKeyCodeToKeysym(display, keycode, index)
Display	*display = Dsp;
KeyCode	keycode;
int	index;
>>EXTERN
#define XK_LATIN1
#include	"keysymdef.h"
#undef XK_LATIN1
>>ASSERTION Good A
A call to xname returns the
.S KeySym
specified by element
.A index
of the
.S KeyCode
vector specified by the
.A keycode
argument.
>>STRATEGY
Obtain the KeyCode corresponding to KeySym XK_a using XKeysymToKeycode.
Obtain the KeySym corresponding to the KeyCode using xname with index = 0.
Verify that the KeySym returned is XK_a.
Obtain the KeySym corresponding to the KeyCode using xname with index = 1.
Verify that the KeySym returned is XK_A.
>>CODE
KeyCode	kc;
KeySym	ks;

	kc = XKeysymToKeycode(display, XK_a);

	if(kc == 0) {
		delete("XKeysymToKeycode() returned 0 for KeySym XK_a");
		return;
	} else
		CHECK;

	keycode = kc;
	index = 0;
	ks = XCALL;

	if(ks != XK_a) {
		report("%s() returned KeySym %lu instead of %lu for KeyCode %lu and index %d.",
			TestName, (long) ks, (long) XK_a, (long) kc, index);
		FAIL;
	} else
		CHECK;


	index = 1;
	ks = XCALL;

	if(ks != XK_A) {
		report("%s() returned KeySym %lu instead of %lu for KeyCode %lu and index %d.",
			TestName, (long) ks, (long) XK_A, (long) kc, index);
		FAIL;
	} else
		CHECK;

	CHECKPASS(3);

>>ASSERTION Good A
When the symbol specified by element
.A index
of the
.S KeyCode
vector specified by the
.A keycode
argument is not defined, then a call to
xname returns
.S NoSymbol . 
>>STRATEGY
Obtain the maximum keycode using XDisplayKeycodes.
Obtain the KeySym corresponding to the maximum KeyCode+1 using xname with index 0.
Verify that the call returned NoSymbol.
Obtain the KeySym corresponding to the minimum KeyCode using xname with index 9.
Verify that the call returned NoSymbol.
>>CODE
int	maxi,mini;
KeyCode	maxkc, minkc;
KeySym	ks;
int	kspkc;

	XDisplayKeycodes(display, &mini, &maxi);
	minkc = (KeyCode)mini;
	maxkc = (KeyCode)maxi;

	index = 0;		
	keycode = maxkc + 1;
	ks = XCALL;

	if(ks != NoSymbol) {
		report("%s() returned KeySym value %ul instead of NoSymbol (%lu) for KeyCode %lu and index %d.",
			TestName, (long) ks, (long) NoSymbol, (long) keycode, index);
		FAIL;
	} else
		CHECK;

	keycode = minkc;
	index = 9; /* only 0-8 are valid. */
	ks = XCALL;

	if(ks != NoSymbol) {
		report("%s() returned KeySym value %ul instead of NoSymbol (%lu) for KeyCode %lu and index %d.",
			TestName, (long) ks, (long) NoSymbol, (long) keycode, index);
		FAIL;
	} else
		CHECK;

	CHECKPASS(2);

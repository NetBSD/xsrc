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
 * $XConsortium: nwmdfrmp.m,v 1.5 94/04/17 21:06:41 rws Exp $
 */
>>TITLE XNewModifiermap CH07
XModifierKeymap *

int 	max_keys_per_mod;
>>ASSERTION Good A
A call to xname returns a pointer to a
.S XModifierKeymap
structure,
that can be freed with
.F XFreeModifiermap ,
with
.A max_keys_per_mod
entries preallocated for each of the 8 modifiers.
>>STRATEGY
Create a XModifierKeymap structure with xname.
Verify that the max_keypermod member is set correctly.
Verify that the whole of the modifiermap array can be accessed.
Free with XFreeModifiermap.

Call xname with max_keys_per_mod of 0.
Free with XFreeModifiermap.
>>CODE
XModifierKeymap	*mkmp;
int 	i;

	max_keys_per_mod = 12;

	mkmp = XCALL;

	if (mkmp->max_keypermod == max_keys_per_mod)
		CHECK;
	else {
		report("max_keypermod was %d, expecting %d", mkmp->max_keypermod,
			max_keys_per_mod);
		FAIL;
	}

	/*
	 * Access each member of the array.  If this fails (with a signal)
	 * then the TET will delete the test.  This is not exactly right
	 * because it should really fail. Anyway the problem still has to
	 * be fixed to get a PASS so there is no real problem.
	 */
	for (i = 0; i < 8*mkmp->max_keypermod; i++) {
		mkmp->modifiermap[i] = NoSymbol;
	}

	XFreeModifiermap(mkmp);

	max_keys_per_mod = 0;

	mkmp = XCALL;

	if (mkmp->max_keypermod == 0)
		CHECK;
	else {
		report("max_keypermod was %d, not zero", mkmp->max_keypermod);
		FAIL;
	}
	XFreeModifiermap(mkmp);

	CHECKPASS(2);

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
 * $XConsortium: iskypdky.m,v 1.5 94/04/17 21:09:44 rws Exp $
 */
>>TITLE IsKeypadKey CH10

IsKeypadKey(keysym)
KeySym	keysym;
>>EXTERN
#define XK_LATIN1
#define XK_MISCELLANY
#include	"keysymdef.h"
#undef	XK_MISCELLANY
#undef XK_LATIN1
>>ASSERTION Good A
When the
.A keysym
argument is a keypad key, then
invocation of the xname macro
returns
.S True .
>>STRATEGY
For each keypad key KeySym:
   Verify that xname returns True.
>>CODE
static KeySym ks[] = {
			XK_KP_Space,
			XK_KP_Tab,
			XK_KP_Enter,
			XK_KP_F1,
			XK_KP_F2,
			XK_KP_F3,
			XK_KP_F4,
			XK_KP_Equal,
			XK_KP_Multiply,
			XK_KP_Add,
			XK_KP_Separator,
			XK_KP_Subtract,
			XK_KP_Decimal,
			XK_KP_Divide,
			XK_KP_0,
			XK_KP_1,
			XK_KP_2,
			XK_KP_3,
			XK_KP_4,
			XK_KP_5,
			XK_KP_6,
			XK_KP_7,
			XK_KP_8,
			XK_KP_9,
			0 };
KeySym	*ksp;
Bool	res;

	for(ksp = ks; *ksp; ksp++) {
		keysym = *ksp;
		res = XCALL;
		if(res != True) {
			char	*kstr = XKeysymToString(*ksp);

			report("%s() did not return True for KeySym XK_%s (value %lu).",
				TestName, kstr != (char *) NULL ? kstr : "<KeySym Undefined>", *ksp);
			FAIL;
		} else
			CHECK;

	}

	CHECKPASS(NELEM(ks) - 1);

>>ASSERTION Good A
When the
.A keysym
argument is not a keypad key, then
invocation of the xname macro returns
.S False .
>>STRATEGY
Verify that xname returns False for the KeySym XK_A.
>>CODE
Bool	res;

	keysym = XK_A;
	res = XCALL;
	if(res != False) {
		char	*kstr = XKeysymToString(keysym);

		report("%s() did not return False for KeySym XK_%s (value %lu).",
			TestName, kstr != (char *) NULL ? kstr : "<KeySym Undefined>", keysym);
		FAIL;
	} else
		CHECK;

	CHECKPASS(1);

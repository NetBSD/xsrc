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
 * $XConsortium: qrykymp.m,v 1.8 94/04/17 21:06:42 rws Exp $
 */
>>TITLE XQueryKeymap CH07
void

Display	*display = Dsp;
char	*keys_return = Keys;
>>EXTERN

static	char	Keys[32];

>>ASSERTION Good B 3
A call to xname returns a bit vector
with bits set to 1 where the corresponding key is currently
in the logical down state and all other bits set to 0.
>>STRATEGY
If extension available:
  Check no keys pressed down at start.
  Press some keys down.
  Call xname and check they, and only they, are down.
  Release them all.
  Call xname to check all released.
else
  Call xname.
  UNTESTED touch test only.
>>CODE
int i;
char *p,*q;
int fcount;
char pressed[32];
int minkc, maxkc;

	if (noext(0)) {
		XCALL;

		report("There is no reliable test method, but a touch test was performed");

		UNTESTED;
		return;
	} else
		CHECK;

	/* expect all zero map. If not can't start. Should we fail? */
	XCALL;

	for(fcount=i=0,p=keys_return; i<32; i++,p++)
		if (*p)
			fcount++;

	if (fcount) {
		delete("%d keys appear to be pressed down.", fcount);
		return;
	} else
		CHECK;

	XDisplayKeycodes(display, &minkc, &maxkc);

	for(p=pressed,i=0; i<32; i++,p++)
		*p = (char) 0;

	for(i=minkc; i < minkc+5 && i <= maxkc; i++) {
		keypress(display, i);
		pressed[i/8] |= (char) (0x1 << (i % 8));
	}

	/* check map now has only those set */
	XCALL;

	for(p=pressed,q=keys_return,fcount=i=0; i<32; i++,p++,q++)
		if (*p != *q) {
			char diffs = (unsigned)*p ^ (unsigned)*q;
			int di;

			fcount++;
			for(di=0; di<8; di++) {
				char mask = (char) (0x1 << di);

				if (diffs & mask)
					report("Key %d%s pressed.", 8*i + di,
						(*p & mask) ? "" : " not");
			}
		}

	if (fcount)
		FAIL;
	else
		CHECK;

	relalldev(); /* now all released */

	/* check map now all zero. */
	XCALL;

	for(fcount=i=0,p=keys_return; i<32; i++,p++)
		if (*p)
			fcount++;

	if (fcount) {
		report("%d keys appear to be pressed down.", fcount);
		FAIL;
	} else
		CHECK;

	CHECKPASS(4);


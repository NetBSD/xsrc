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
 * $XConsortium: chngkybrdm.m,v 1.7 94/04/17 21:06:12 rws Exp $
 */
>>TITLE XChangeKeyboardMapping CH07
void

Display	*display = Dsp;
int 	first_keycode = First;
int 	keysyms_per_keycode = 1;
KeySym	*keysyms = Keys;
int 	num_codes = 2;
>>EXTERN

#define	MAXKPK	5	/* Maximum keysyms_per_keyocde we will use */
#define	MAXCODES 255	/* Max number of keycodes we will use */

static	int 	First;
static	int 	ncodes;
static	int 	kpk;	/* Keysyms per keycode */
static	KeySym	*oldkeym;

/*
 * Can't really assume that there are any particular keysym names defined so
 * we use our own arbitrary values.
 */
#define	XT_KSYM1	0x5678
#define	XT_KSYM2	0x9328

static	KeySym	Keys[MAXKPK*MAXCODES] = {
	XT_KSYM1, XT_KSYM2};

/*
 * Set startup and cleanup functions to save and restore
 * the original keyboard map.
 */
>>SET startup savekeymap
static void
savekeymap()
{
int 	last;

	startup();

	if(Dsp) {
		XDisplayKeycodes(Dsp, &First, &last);
		ncodes = last-First;

		oldkeym = XGetKeyboardMapping(Dsp, First, ncodes, &kpk);
	}
}

>>SET cleanup cleankeymap
static void
cleankeymap()
{
	if(Dsp) {
		XChangeKeyboardMapping(Dsp, First, kpk, oldkeym, ncodes);
		XFree((char*) oldkeym);
	}

	cleanup();
}


>>ASSERTION Good A
A call to xname associates
.A keysyms_per_keycode
KeySyms for each of the
.A num_codes
KeyCodes starting with
.A first_keycode ,
with the KeySyms being taken from the array
.A keysyms .
>>STRATEGY
Set up keysym array.
Call xname.
Verify that keyboard mapping has been changed.
>>CODE
int 	i, j;
int 	newkpk;
KeySym	*newmap;

	first_keycode = First+3;
	keysyms_per_keycode = MAXKPK;
	num_codes = 6;

	for (i = 0; i < keysyms_per_keycode*num_codes; i++)
		keysyms[i] = XT_KSYM1;

	XCALL;

	newmap = XGetKeyboardMapping(display, first_keycode, num_codes, &newkpk);

	for (i = 0; i < num_codes; i++) {
		for (j = 0; j < keysyms_per_keycode; j++) {
			if (keysyms[i*keysyms_per_keycode+j] == newmap[i*newkpk+j])
				CHECK;
			else {
				report("Keysym for keycode %d was 0x%x, expecting 0x%x",
					first_keycode+i,
					newmap[i*newkpk+j], keysyms[i*keysyms_per_keycode+j]);
				FAIL;
				break;	/* probably pointless to continue */
			}
		}
	}

	CHECKPASS(keysyms_per_keycode*num_codes);

	XFree((char*)newmap);
>>ASSERTION Good A
The KeySyms for KeyCodes outside the specified range
remain unchanged.
>>STRATEGY
Get current keymap.
Call xname to change part of the map.
Verify that there is no change outside the specified range.
>>CODE
KeySym	*oldmap;
int 	oldkpk;
KeySym	*newmap;
int 	newkpk;
int 	i, j;
int 	oldind, newind;

	oldmap = XGetKeyboardMapping(display, First, ncodes, &oldkpk);

	first_keycode = First+2;
	XCALL;

	newmap = XGetKeyboardMapping(display, First, ncodes, &newkpk);

	for (i = 0; i < ncodes; i++) {

		/* Skip the ones that were changed. */
		if (i + First >= first_keycode && i + First < first_keycode+num_codes)
			continue;

		/*
		 * The call may have altered the number of keysyms per keycode,
		 * so must only check that the previously set position have
		 * not changed.  Expansion should have been filled with NoSymbol.
		 */
		for (j = 0; j < oldkpk; j++) {
			oldind = i*oldkpk + j;
			newind = i*newkpk + j;
			if (oldmap[oldind] == newmap[newind])
				CHECK;
			else {
				report("Keysym outside of range altered");
				report("  keycode %d was 0x%x, expecting 0x%x",
					First+i, newmap[newind], oldmap[oldind]);
				FAIL;
			}
		}
	}

	/* Number of codes that should not have been changed is ncodes-num_codes */
	CHECKPASS((ncodes - num_codes)*oldkpk);
>>ASSERTION Good A
A call to xname generates a
.S MappingNotify
event.
>>STRATEGY
Call xname.
Verify that a MappingNotify event is generated.
>>CODE
XEvent	ev;
XMappingEvent	good;
int 	n;

	first_keycode = First + 2;
	num_codes = 3;
	XCALL;

	defsetevent(good, display, MappingNotify);
	good.window = None;	/* unused */
	good.request = MappingKeyboard;
	good.first_keycode = first_keycode;
	good.count = num_codes;

	n = getevent(display, &ev);
	if (n == 0) {
		report("Expecting one MappingEvent");
		FAIL;
		return;
	} else
		CHECK;

	if (checkevent((XEvent*)&good, &ev))
		FAIL;
	else
		CHECK;

	CHECKPASS(2);
>>ASSERTION Good A
It is legal for the KeySym
.S NoSymbol
to appear anywhere in the KeySym list for a particular KeyCode.
>>STRATEGY
Call xname with the KeySym NoSymbol in the list.
Verify that no error occurs.
>>CODE

	Keys[0] = NoSymbol;
	XCALL;

	if (geterr() == Success)
		PASS;

>>ASSERTION Bad A
When
.A first_keycode
is less than the value of min_keycode returned by
.F XDisplayKeycodes ,
then a
.S BadValue
error occurs.
>>STRATEGY
Set first_keycode to a value less than min_keycode.
Call xname.
Verify that a BadValue error occurs.
>>CODE BadValue

	first_keycode = First-1;
	XCALL;

	if (geterr() == BadValue)
		PASS;
>>ASSERTION Bad A
When
.A first_keycode + num_codes \- 1
is greater than the value of max_keycode returned by
.F XDisplayKeycodes ,
then a
.S BadValue
error occurs.
>>STRATEGY
Set end of range to beyond max_keycode.
Call xname.
Verify that a BadValue error occurs.
>>CODE BadValue

	num_codes = ncodes;
	first_keycode = First+10;

	XCALL;

	if (geterr() == BadValue)
		PASS;
>>ASSERTION Bad B 3
.ER BadAlloc

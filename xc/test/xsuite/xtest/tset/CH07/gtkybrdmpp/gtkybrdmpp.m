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
 * $XConsortium: gtkybrdmpp.m,v 1.11 94/04/17 21:06:32 rws Exp $
 */
>>TITLE XGetKeyboardMapping CH07
KeySym *
xname
Display	*display = Dsp;
KeyCode	first_keycode = First;
int 	keycode_count = ncodes;
int 	*keysyms_per_keycode_return = &kpk;
>>EXTERN

#define	MAXKPK	5	/* Maximum keysyms_per_keyocde we will use */
#define	MAXCODES 255	/* Max number of keycodes we will use */

static	KeyCode	First;
static	int 	ncodes;
static	int 	kpk;	/* Keysyms per keycode */
static	int 	oldkpk;	/* old Keysyms per keycode */
static	KeySym	*oldkeym;

/*
 * Can't really assume that there are any particular keysym names defined so
 * we use our own arbitrary values.
 */
#define	XT_KSYM1	0x5678
#define	XT_KSYM2	0x9228
#define	XT_KSYM3	0x4425
#define	XT_KSYM4	0x5326

static	KeySym	Keys[MAXKPK*MAXCODES];

/*
 * Set startup and cleanup functions to save and restore
 * the original keyboard map.
 */
>>SET startup savekeymap
static void
savekeymap()
{
int	tmp;
int 	last;
int 	i;

	startup();

	if (Dsp==(Display *)NULL)
		return;

	XDisplayKeycodes(Dsp, &tmp, &last);
	ncodes = last-tmp;
	First = (KeyCode)tmp;

	debug(2, "First keycode  :%d", First);
	debug(2, "Last keycode   :%d", last);
	debug(2, "Number of codes:%d", ncodes);

	oldkeym = XGetKeyboardMapping(Dsp, First, ncodes, &oldkpk);

	/*
	 * Initialise the keysym table.
	 */
	for (i = 0; i < MAXKPK*MAXCODES-4; i += 4) {
		Keys[i] = XT_KSYM1;
		Keys[i+1] = XT_KSYM2;
		Keys[i+2] = XT_KSYM3;
		Keys[i+3] = XT_KSYM4;
	}
}

>>SET cleanup cleankeymap
static void
cleankeymap()
{
	if (Dsp) {
		XChangeKeyboardMapping(Dsp, First, oldkpk, oldkeym, ncodes);
		XFree((char*) oldkeym);
	}

	cleanup();
}


>>ASSERTION Good A
A call to xname returns an array, that can be freed with
.F XFree ,
of KeySyms associated with
the specified number,
.A keycode_count ,
of KeyCodes starting with
.A first_keycode .
>>STRATEGY
Set some KeySyms with XChangeKeyboardMapping.
Call xname to get KeySyms.
Verify they are as set.
Free returned array with XFree.
>>CODE
int 	i, j;
int 	syms_per_code;
KeySym	*newmap;

	first_keycode = First+3;
	syms_per_code = 3;
	keycode_count = 9;

	XChangeKeyboardMapping(display, first_keycode, syms_per_code, Keys, keycode_count);
	if (isdeleted())
		return;

	newmap = XCALL;

	for (i = 0; i < keycode_count; i++) {
		for (j = 0; j < syms_per_code; j++) {
			if (Keys[i*syms_per_code+j] ==
				  newmap[i*keysyms_per_keycode_return[0]+j])
				CHECK;
			else {
				report("Keysym for keycode %d was 0x%x, expecting 0x%x",
					first_keycode+i,
					newmap[i*keysyms_per_keycode_return[0]+j],
					Keys[i*syms_per_code+j]
					);
				FAIL;
				break;	/* probably pointless to continue */
			}
		}
	}

	XFree((char*)newmap);

	CHECKPASS(syms_per_code*keycode_count);
>>ASSERTION def
>># A silly assertion.  It is tested as much as possible above.
On a call to xname the returned KeySyms list contains
.br
.A keycode_count * keysyms_per_keycode_return
.br
elements.
>>ASSERTION Good A
On a call to xname
.A keysyms_per_keycode_return
is set to a value that is large enough to report all of the KeySyms
for any of the requested KeyCodes.
>>STRATEGY
Set KeySyms with XChangeKeyboardMapping.
Call xname to get new value of this parameter.
Verify that it is at least as large as set.
>>CODE
int 	syms_per_code = 6;

	/*
	 * I don't know a really good test for this.
	 */
	XChangeKeyboardMapping(display, first_keycode, syms_per_code, Keys, keycode_count);
	if (isdeleted())
		return;

	XCALL;

	if (keysyms_per_keycode_return[0] >= syms_per_code)
		CHECK;
	else {
		report("keysyms_per_keycode_return was unexpected");
		FAIL;
	}

	CHECKPASS(1);
>>ASSERTION def
>># I'm not sure what you could do here, its just saying that there is
>># a reserved value that means 'not used'.
When an element for a particular KeyCode is unused,
then a KeySym value of
.S NoSymbol
is used in the returned array.
>>ASSERTION Bad A
When the value specified in
.A first_keycode
is less than the minimum keycode as returned by
.F XDisplayKeycodes ,
then a
.S BadValue
error occurs.
>>STRATEGY
Set first_keycode to less than the minimum keycode.
Call xname.
Verify that a BadValue error occurs.
>>CODE BadValue

	first_keycode = First-1;

	XCALL;

	if (geterr() == BadValue)
		PASS;
	else
		FAIL;
>>ASSERTION Bad A
When the expression
.A first_keycode + keycode_count "\- 1"
is greater than the maximum keycode as returned by
.F XDisplayKeycodes ,
then a
.S BadValue
error occurs.
>>STRATEGY
Set first keycode to greater than the maximum keycode.
Call xname.
Verify that a BadValue error occurs.
>>CODE BadValue

	first_keycode = First+ncodes;

	XCALL;

	if (geterr() == BadValue)
		PASS;
	else
		FAIL;

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

 * Copyright 1993 by the Hewlett-Packard Company.
 *
 * Copyright 1990, 1991 UniSoft Group Limited.
 *
 * Permission to use, copy, modify, distribute, and sell this software and
 * its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation, and that the names of HP, and UniSoft not be
 * used in advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.  HP, and UniSoft
 * make no representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 *
 * $XConsortium: gtdvkmap.m,v 1.7 94/09/06 21:07:52 dpw Exp $
 */
>>TITLE XGetDeviceKeyMapping XINPUT
void

Display	*display = Dsp;
XDevice *device = Devs.Key;
KeyCode first = Min_KeyCode;
int keycount = 1;
int *syms_per_code = &ksym_cnt;
>>EXTERN
static int Min_KeyCode, Max_KeyCode;
extern ExtDeviceInfo Devs;
static int ksym_cnt;

#define	MAXKPK	5	/* Maximum keysyms_per_keyocde we will use */
#define	MAXCODES 255	/* Max number of keycodes we will use */

static	int 	ncodes;
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
int 	numkeys;

	startup();

	if (Dsp==(Display *)NULL)
		return;

	if (!Setup_Extension_DeviceInfo(KeyMask))
	    return;
	MinMaxKeys(Dsp, Devs.Key, &Min_KeyCode, &Max_KeyCode, &numkeys);
	device = Devs.Key;
	ncodes = Max_KeyCode-Min_KeyCode;
	oldkeym = XGetDeviceKeyMapping(Dsp, device, Min_KeyCode, ncodes, &oldkpk);

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
	if (Dsp && Devs.Key) {
		XChangeDeviceKeyMapping(Dsp, Devs.Key, Min_KeyCode, oldkpk, oldkeym, ncodes);
		XSync(Dsp,0);
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
Set some KeySyms with XChangeDeviceKeyMapping.
Call xname to get KeySyms.
Verify they are as set.
Free returned array with XFree.
>>CODE
int 	i, j;
int 	SymsPerCode;
KeySym	*newmap;

	if (!Devs.Key) {
	    untested("%s: Required input devices not present\n",TestName);
	    return;
	    }
	first = Min_KeyCode+3;
	SymsPerCode = 3;
	keycount = 9;

	XChangeDeviceKeyMapping(display, device, first, SymsPerCode, Keys, keycount);
	if (isdeleted())
		return;

	newmap = XCALL;

	for (i = 0; i < keycount; i++) {
		for (j = 0; j < SymsPerCode; j++) {
			if (Keys[i*SymsPerCode+j] ==
				  newmap[i*syms_per_code[0]+j])
				CHECK;
			else {
				report("Keysym for keycode %d was 0x%x, expecting 0x%x",
					first+i,
					newmap[i*syms_per_code[0]+j],
					Keys[i*SymsPerCode+j]
					);
				FAIL;
				break;	/* probably pointless to continue */
			}
		}
	}

	XFree((char*)newmap);

	CHECKPASS(SymsPerCode*keycount);

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
Set KeySyms with XChangeDeviceKeyMapping.
Call xname to get new value of this parameter.
Verify that it is at least as large as set.
>>CODE
int 	SymsPerCode = 6;

	if (!Devs.Key) {
	    untested("%s: Required input devices not present\n",TestName);
	    return;
	    }
	first = Min_KeyCode;
	/*
	 * I don't know a really good test for this.
	 */
	XChangeDeviceKeyMapping(display, device, first, SymsPerCode, Keys, keycount);
	if (isdeleted())
		return;

	XCALL;

	if (syms_per_code[0] >= SymsPerCode)
		CHECK;
	else {
		report("syms_per_code was unexpected");
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
.F XListInputDevices,
then a
.S BadValue
error occurs.
>>STRATEGY
Set first_keycode to less than the minimum keycode.
Call xname.
Verify that a BadValue error occurs.
>>CODE BadValue

	if (!Devs.Key) {
	    untested("%s: Required input devices not present\n",TestName);
	    return;
	    }
	device = Devs.Key;
	first = Min_KeyCode-1;

	XCALL;

	if (geterr() == BadValue)
		PASS;
	else
		FAIL;

>>ASSERTION Bad A
When the expression
.A first_keycode + keycode_count "\- 1"
is greater than the maximum keycode as returned by
.F XListInputDevices,
then a
.S BadValue
error occurs.
>>STRATEGY
Set first keycode to greater than the maximum keycode.
Call xname.
Verify that a BadValue error occurs.
>>CODE BadValue
KeySym *ret;

	if (!Devs.Key) {
	    untested("%s: Required input devices not present\n",TestName);
	    return;
	    }
	first = Max_KeyCode+1;

	ret = XCALL;

	if (geterr() == BadValue)
		PASS;
	else
	    FAIL;

>>ASSERTION Bad B 3
A call to xname will fail with a BadValue error if a firstkeycode
value that is out of the range of valid values is specified.
>>STRATEGY
Make the call with a firstkeycode value that is out of range.
>>CODE BadValue
int count=0;

	if (!Devs.Key) {
	    untested("%s: Required input devices not present\n",TestName);
	    return;
	    }
	device = Devs.Key;
	first = Min_KeyCode;
	if (Max_KeyCode < 255)
	    {
	    first = Max_KeyCode + 1;
	    XCALL;
	    if (geterr() == BadValue)
		CHECK;
	    else
		FAIL;
	    count++;
	    }

	if (Min_KeyCode > 0)
	    {
	    first = Min_KeyCode - 1;
	    XCALL;
	    if (geterr() == BadValue)
		CHECK;
	    else
		FAIL;
	    count++;
	    }
	CHECKPASS(count);
>>ASSERTION Bad B 3
A call to xname will fail with a BadValue error if the expression 
"firstkeycode + count" is > Max_KeyCode + 1;
>>STRATEGY
Make the call with a too many keycodes.
>>CODE BadValue

	if (!Devs.Key) {
	    untested("%s: Required input devices not present\n",TestName);
	    return;
	    }
	first = Max_KeyCode;
	keycount = 2;
	XCALL;
	if (geterr() == BadValue)
	    PASS;
	else
	    FAIL;
>>ASSERTION Bad B 3
A call to xname will fail with a BadMatch error if a valid device
with no keys is specified.
>>STRATEGY
Make the call with a valid device that has no keys.
>>CODE BadMatch

	if (!Setup_Extension_DeviceInfo(NKeysMask))
	    {
	    untested("%s: Required extension device not present\n",TestName);
	    return;
	    }
	device = Devs.NoKeys;
	first = Min_KeyCode;

	XCALL;

	if (geterr() == BadMatch)
		PASS;
	else
		FAIL;
>>ASSERTION Bad B 3
A call to xname will fail with a BadDevice error if an invalid device
is specified.
>>STRATEGY
Make the call with an invalid device.
>>CODE baddevice
XDevice nodevice;
XID baddevice;
int ximajor, first, err;

	if (!XQueryExtension (display, INAME, &ximajor, &first, &err))
	    {
	    untested("%s: Input extension not supported.\n", TestName);
	    return;
	    }

	BadDevice (display, baddevice);
	nodevice.device_id = -1;
	device = &nodevice;
	first = Min_KeyCode;

	XCALL;

	if (geterr() == baddevice)
		PASS;
	else
		FAIL;
	device = Devs.Key;

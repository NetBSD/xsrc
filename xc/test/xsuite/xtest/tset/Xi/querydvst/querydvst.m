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
 * $XConsortium: querydvst.m,v 1.7 94/09/06 21:08:29 dpw Exp $
 */
>>TITLE XQueryDeviceState XINPUT
XDeviceState *

Display	*display = Dsp;
XDevice *device;
>>EXTERN
#define KEYMAPLEN 32
#define VALMAPLEN 6
extern ExtDeviceInfo Devs;
static NumValuators;

>>ASSERTION Good B 3
A successful call to QueryDeviceState resets the DeviceMotionHint state so
that the next motion event generated will be sent to interested clients.
>>STRATEGY
Select DeviceMotionNotify and DevicePointerMotionHint classes.
Generate some motion events, verify only one is sent.
Now call xname.
Generate some motion events, verify another one is sent.
>>CODE
XID dmn, dmnh;
XEventClass classes[2];
Window w;
int axes=1, n, i;
XEvent ev;
XDeviceMotionEvent *d;

	if (!Setup_Extension_DeviceInfo(ValMask))
	    {
	    untested("%s: Required input extension device not present.\n", 
		TestName);
	    return;
	    }
	if(noext(0))
	    return;
	device = Devs.Valuator;
	DeviceMotionNotify(device, dmn, classes[0]);
	DevicePointerMotionHint(device, dmnh, classes[1]);
	w = defwin(display);
	XSelectExtensionEvent(display, w, classes, 2);
	warppointer (display, w, 1, 1);

	XSync(display,1);
	for (i=0; i<10; i++)
	    SimulateDeviceMotionEvent(display, device, False, 1, &axes, 0);
	XSync(display,0);

	n = getevent(display, &ev);
	if (n != 1)
	    {
	    report("Expecting one event with DeviceMotionHint, got %d",n);
	    FAIL;
	    }
	else
	    CHECK;
	if (ev.type != dmn)
	    {
	    report("Expecting DeviceMotionNotify event, got event type %d",
		ev.type);
	    FAIL;
	    }
	else
	    CHECK;
	d = (XDeviceMotionEvent *) &ev;
	if (d->is_hint != True)
	    {
	    report("Expecting is_hint = True, was False");
	    FAIL;
	    }
	else
	    CHECK;

	XCALL;
	if (geterr() == Success)
		CHECK;
	else
	    {
	    report("Expecting Success, got error\n");
	    FAIL;
	    return;
	    }

	XSync(display,1);
	for (i=0; i<10; i++)
	    SimulateDeviceMotionEvent(display,  device, False,  1, &axes, 0);
	XSync(display,0);

	n = getevent(display, &ev);
	if (n != 1)
	    {
	    report("Expecting one event with DeviceMotionHint, got %d",n);
	    FAIL;
	    }
	else
	    CHECK;
	if (ev.type != dmn)
	    {
	    report("Expecting DeviceMotionNotify event, got event type %d",
		ev.type);
	    FAIL;
	    }
	else
	    CHECK;
	d = (XDeviceMotionEvent *) &ev;
	if (d->is_hint != True)
	    {
	    report("Expecting is_hint = True, was False");
	    FAIL;
	    }
	else
	    CHECK;

	axes=0;
	SimulateDeviceMotionEvent(display,  device, False,  1, &axes, 0);
	XSync(display,1);
	CHECKPASS(7);

>>ASSERTION Good B 3
A successful call to QueryDeviceState returns the state of the keys,
buttons, and valuators on the extension device.
>>STRATEGY
Press some keys.
Press some buttons, if the device has any.
Move some valuators, if the device has any.
Call QueryDeviceState.
Check to see if those keys, buttons, and valuators are reported as being down.
>>EXTERN
devtype(dpy, dev, haskeys, hasbuttons, hasvaluators)
Display *dpy;
XDevice *dev;
int *haskeys, *hasbuttons, *hasvaluators;
{
int i, j, ndevices;
XDeviceInfo *list;
XAnyClassPtr any;

    list = XListInputDevices (dpy, &ndevices);
    for (i=0; i<ndevices; i++,list++)
	{
	if (list->id != dev->device_id)
	    continue;
	*haskeys = False;
	*hasbuttons = False;
	*hasvaluators = False;
	any = (XAnyClassPtr) (list->inputclassinfo);
	for (j=0; j<list->num_classes; j++)
	    {
	    switch (any->class)
		{
		case KeyClass:
		    {
		    if (((XKeyInfo *)any)->num_keys)
			*haskeys=True;
		    break;
		    }
		case ButtonClass:
		    {
		    if (((XButtonInfo *)any)->num_buttons)
			*hasbuttons=True;
		    break;
		    }
		case ValuatorClass:
		    {
		    if (((XValuatorInfo *)any)->num_axes)
			{
			*hasvaluators=True;
			NumValuators = (((XValuatorInfo *)any)->num_axes);
			}
		    break;
		    }
		default:
		    break;
		}
	    any = (XAnyClassPtr) ((char *) any + any->length);
	    }
	}
}

>>EXTERN
pokedev(display, device)
Display *display;
XDevice *device;
{
int i, j, key1;
XDeviceState *state;
XKeyState *ks;
XButtonState *bs;
XValuatorState *vs;
char *keys;
int *vals;
int axes[256];
int haskeys, hasbuttons, hasvaluators, pass=0, fail=0, count=1;

	devtype(display, device, &haskeys, &hasbuttons, &hasvaluators);
	if (!(haskeys & hasbuttons & hasvaluators))
	    {
	    untested("Extension device has no keys, buttons, or valuators");
	    return;
	    }
	key1 = getkeycode(display);
	if (haskeys)
	    {
	    devicekeyrel (display, device, key1);
	    devicerelkeys (device);
	    }
	if (hasbuttons)
	    {
	    devicebuttonrel (display, device, Button1);
	    devicerelbuttons(device);
	    }
	if (hasvaluators)
	    {
	    int count;
	    int nvals = NumValuators < 6 ? NumValuators : 6;
	    for (i=0; i<nvals; i++)
		axes[i] = 0;
	    for (i=0; i<NumValuators; i+=6) {
	        count = NumValuators-i < 6 ? NumValuators-i : 6;
		SimulateDeviceMotionEvent (display, device, False, count, axes,
		    i);
		XSync(display,0);
		}
	    XSync(display,0);
	    }
	state = XCALL;
	if (state)
		CHECK;
	else
	    {
	    report("Expecting Success, got error (NULL state returned).");
	    FAIL;
	    return;
	    }
	ks = (XKeyState *) state->data;
	for (i=0; i<state->num_classes; i++)
	    {
	    if (ks->class == KeyClass)
		{
		for (j=0,keys=ks->keys; j<KEYMAPLEN; j++,keys++)
		    if (*keys != 0)
			{
			report("%s: byte %d of keys was %x, should be 0\n",
			    TestName,j,*keys);
			FAIL;
			}
		    else
			CHECK;
		count += KEYMAPLEN;
		}
	    else if (ks->class == ButtonClass)
		{
		bs = (XButtonState *) ks;
		for (j=0,keys=bs->buttons; j<KEYMAPLEN; j++,keys++)
		    if (*keys != 0)
			{
			report("%s: byte %d of buttons was %x, should be 0\n",
			    TestName,j,*keys);
			FAIL;
			}
		    else
			CHECK;
		count += KEYMAPLEN;
		}
	    else if (ks->class == ValuatorClass)
		{
		vs = (XValuatorState *) ks;
		for (j=0,vals=vs->valuators; j<vs->num_valuators; j++,vals++)
		    if (*vals != 0)
			{
			report("%s: valuator %d was %x, should be 0\n",
			    TestName,j,*vals);
			FAIL;
			}
		    else
			CHECK;
		count += vs->num_valuators;
		}
	    ks = (XKeyState *) ((char *) ks + ks->length);
	    }

	if (haskeys)
	    devicekeypress (display, device, key1);
	if (hasbuttons)
	    devicebuttonpress (display, device, Button1);
	if (hasvaluators)
	    {
	    int nvals = vs->num_valuators < 6 ? vs->num_valuators : 6;
	    for (i=0; i<vs->num_valuators; i++)
		axes[i] = i;
	    SimulateDeviceMotionEvent(display,  device, False,  nvals, axes, 0);
	    XSync(display,0);
	    }
	state = XCALL;
	ks = (XKeyState *) state->data;
	for (i=0; i<state->num_classes; i++)
	    {
	    if (ks->class == KeyClass)
		{
		for (j=0,keys=ks->keys; j<KEYMAPLEN; j++,keys++)
		    if (j != key1 >> 3 && *keys != 0)
			{
			report("%s: byte %d of keys was %x, should be 0\n",
			    TestName,j,*keys);
			FAIL;
			}
		    else if (j== key1 >> 3 && *keys != 1 << (key1 & 7))
			{
			report("%s: byte %d of keys was %x, should be %x\n",
			    TestName,j,*keys, 1 << (key1 & 7));
			FAIL;
			}
		    else
			CHECK;
		count += KEYMAPLEN;
		}
	    else if (ks->class == ButtonClass)
		{
		bs = (XButtonState *) ks;
		for (j=0,keys=bs->buttons; j<KEYMAPLEN; j++,keys++)
		    if (j != Button1 >> 3 && *keys != 0)
			{
			report("%s: byte %d of buttons was %x, should be 0\n",
			    TestName,j,*keys);
			FAIL;
			}
		    else if (j== Button1 >> 3 && *keys != 1 << (Button1 & 7))
			{
			report("%s: byte %d of buttons was %x, should be %x\n",
			    TestName,j,*keys, 1 << (key1 & 7));
			FAIL;
			}
		    else
			CHECK;
		count += KEYMAPLEN;
		}
	    else if (ks->class == ValuatorClass)
		{
		int nval = vs->num_valuators < 6 ? vs->num_valuators : 6;
		vs = (XValuatorState *) ks;
		for (j=0,vals=vs->valuators; j<nval; j++,vals++)
		    if (*vals != j)
			{
			report("%s: valuator %d was %x, should be %d\n",
			    TestName,j,*vals,j);
			FAIL;
			}
		    else
			CHECK;
		for (j=nval; j<vs->num_valuators; j++,vals++)
		    if (*vals != 0)
			{
			report("%s: valuator %d was %x, should be 0\n",
			    TestName,j,*vals);
			FAIL;
			}
		    else
			CHECK;
		count += vs->num_valuators;
		}
	    ks = (XKeyState *) ((char *) ks + ks->length);
	    }
	if (haskeys)
	    {
	    devicekeyrel (display, device, key1);
	    devicerelkeys (device);
	    }
	if (hasbuttons)
	    {
	    devicebuttonrel (display, device, Button1);
	    devicerelbuttons(device);
	    }
	if (hasvaluators)
	    {
	    int nvals = vs->num_valuators < 6 ? vs->num_valuators : 6;
	    for (i=0; i<nvals; i++)
		axes[i] = 0;
	    for (i=0; i<vs->num_valuators; i+=6) {
		if (i >= vs->num_valuators - 6)
		    nvals = vs->num_valuators - i;
		SimulateDeviceMotionEvent (display, device, False, nvals, axes,
		    i);
		XSync(display,0);
		}
	    }
	CHECKPASS(count);
}
>>CODE

	if (!Setup_Extension_DeviceInfo(KeyMask))
	    {
	    untested("%s: Required input extension device not present.\n", TestName);
	    return;
	    }
	if(noext(0))
	    return;
	device = Devs.Key;
	pokedev (display, device);

>>ASSERTION Good B 3
A successful call to QueryDeviceState returns the state of the keys,
buttons, and valuators on the extension device.
>>STRATEGY
Press some keys.
Press some buttons, if the device has any.
Move some valuators, if the device has any.
Call QueryDeviceState.
Check to see if those keys, buttons, and valuators are reported as being down.
>>CODE

	if (!Setup_Extension_DeviceInfo(BtnMask))
	    {
	    untested("%s: Required input extension device not present.\n", TestName);
	    return;
	    }
	if(noext(0))
	    return;
	device = Devs.Button;
	pokedev (display, device);

>>ASSERTION Good B 3
A successful call to QueryDeviceState returns the state of the keys,
buttons, and valuators on the extension device.
>>STRATEGY
Press some keys.
Press some buttons, if the device has any.
Move some valuators, if the device has any.
Call QueryDeviceState.
Check to see if those keys, buttons, and valuators are reported as being down.
>>CODE

	if (!Setup_Extension_DeviceInfo(ValMask))
	    {
	    untested("%s: Required input extension device not present.\n", TestName);
	    return;
	    }
	if(noext(0))
	    return;
	device = Devs.Valuator;
	pokedev (display, device);

>>ASSERTION Good B 3
A successful call to QueryDeviceState returns the state of the keys,
buttons, and valuators on the extension device.
>>STRATEGY
Call QueryDeviceState for a device with no keys.
>>CODE

	if (!Setup_Extension_DeviceInfo(NKeysMask))
	    {
	    untested("%s: Required input extension device not present.\n", TestName);
	    return;
	    }
	if(noext(0))
	    return;
	device = Devs.NoKeys;
	pokedev (display, device);

>>ASSERTION Good B 3
A successful call to QueryDeviceState returns the state of the keys,
buttons, and valuators on the extension device.
>>STRATEGY
Call QueryDeviceState for a device with no buttons.
>>CODE

	if (!Setup_Extension_DeviceInfo(NBtnsMask))
	    {
	    untested("%s: Required input extension device not present.\n", TestName);
	    return;
	    }
	if(noext(0))
	    return;
	device = Devs.NoButtons;
	pokedev (display, device);

>>ASSERTION Good B 3
A successful call to QueryDeviceState returns the state of the keys,
buttons, and valuators on the extension device.
>>STRATEGY
Call QueryDeviceState for a device with no valuators.
>>CODE

	if (!Setup_Extension_DeviceInfo(NValsMask))
	    {
	    untested("%s: Required input extension device not present.\n", TestName);
	    return;
	    }
	if(noext(0))
	    return;
	device = Devs.NoValuators;
	pokedev (display, device);

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

	XCALL;

	if (geterr() == baddevice)
		PASS;
	else
		FAIL;

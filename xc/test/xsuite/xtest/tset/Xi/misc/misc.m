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
 * $XConsortium: misc.m,v 1.7 94/04/17 21:13:15 rws Exp $
 */
>>TITLE Miscellaneous XINPUT
void

Display	*display = Dsp;
>>EXTERN
extern ExtDeviceInfo Devs;
static Window w;

>>ASSERTION Good B 3
Touch test to execute miscellaneous code that would not otherwise get executed.
>>STRATEGY
Touch test.
>>EXTERN
static devtype(dpy, dev, keys, buttons, valuators)
Display *dpy;
XDevice *dev;
XKeyInfoPtr *keys;
XButtonInfoPtr *buttons;
XValuatorInfoPtr *valuators;
{
int i, j, ndevices, ret=0;
XDeviceInfo *list;
XAnyClassPtr any;

    list = XListInputDevices (dpy, &ndevices);
    for (i=0; i<ndevices; i++,list++)
	{
	if (list->id != dev->device_id)
	    continue;
	*keys = NULL;
	*buttons = NULL;
	*valuators = NULL;
	any = (XAnyClassPtr) (list->inputclassinfo);
	for (j=0; j<list->num_classes; j++)
	    {
	    switch (any->class)
		{
		case KeyClass:
		    *keys=(XKeyInfoPtr) any;
		    ret++;
		    break;
		case ButtonClass:
		    *buttons=(XButtonInfoPtr) any;
		    ret++;
		    break;
		case ValuatorClass:
		    if (((XValuatorInfoPtr) any)->mode != Absolute)
			break;
		    if (!(*valuators)) /* in case  multiple valuator classes */
			{
			*valuators = (XValuatorInfoPtr) any;
			ret++;
			}
		    break;
		default:
		    break;
		}
	    any = (XAnyClassPtr) ((char *) any + any->length);
	    }
	break;
	}
return(ret);
}

/*
 * Set up to get DeviceStateNotify events from the pathological input device:
 * one with 255 keys, 255 buttons, and 255 valuators.
 */

getds (dpy, dev, input)
Display *dpy;
XDevice *dev;
Bool input;
{
int i, j, n, numc, count=0, pass=0, fail=0;
char *data;
XID ds;
XEvent ev;
XDeviceStateNotifyEvent dsgood;
XEventClass dsc;
XKeyInfoPtr keys;
XButtonInfoPtr buttons;
XValuatorInfoPtr valuators;
XValuatorStatus *v;
XKeyStatus *k;
XButtonStatus *b;

	numc = devtype(dpy, dev, &keys, &buttons, &valuators);
	DeviceStateNotify (dev, ds, dsc);
	XSelectExtensionEvent (dpy, w, &dsc, 1);
	warppointer(dpy, w, 1, 1);
	XSync(dpy,1);
	XSetDeviceFocus (dpy, dev, w, RevertToPointerRoot, CurrentTime);
	XSync(dpy,0);

	defsetevent(dsgood, dpy, ds);
	dsgood.window = None;
	dsgood.deviceid = dev->device_id;
	dsgood.num_classes = ((keys && valuators) || (buttons && valuators) || 
	    (keys && buttons)) ? 2 : 1;
	data = dsgood.data;
	if (buttons)
	    {
	    b = (XButtonStatus *) dsgood.data;

	    b->class = ButtonClass;
	    b->length = sizeof (XButtonStatus);
	    b->num_buttons = buttons->num_buttons > 32 ? 256 : buttons->num_buttons;
	    for (i=0; i<32; i++)
	        b->buttons[i]=0;
	    if (input)
		for (i=1; i<buttons->num_buttons; i++)
		    b->buttons[i>>3] |= (1 << (i&7));

	    data += b->length;
	    }
	else if (keys)
	    {
	    k = (XKeyStatus *) data;
	    k->class = KeyClass;
	    k->length = sizeof (XKeyStatus);
	    k->num_keys = keys->max_keycode - keys->min_keycode + 1;
	    k->num_keys = k->num_keys > 32 ? 256 : k->num_keys;
	    for (i=0; i<32; i++)
		k->keys[i]=0;
	    if (input)
		for (i=keys->min_keycode; i<keys->max_keycode; i++)
		    k->keys[i>>3] |= (1 << (i&7));
	    data += k->length;
	    }

	if (valuators)
	    {
	    v = (XValuatorStatus *) data;
	    v->class = ValuatorClass;
	    v->length = sizeof (XValuatorStatus);
	    v->num_valuators = valuators->num_axes<6 ? valuators->num_axes : 6;
	    for (i=0; i<6; i++)
		v->valuators[i]=0;
	    if (input)
		for (i=0; i<v->num_valuators; i++)
		    v->valuators[i]=i;
	    data += v->length;
	    }

	if ((n=getevent(dpy, &ev) == 0) || ev.type != ds) {
		if (n)
		    report("Got %d events, first was type %s.",
			n, eventname(ev.type));
		report("Was expecting a DeviceStateNotify event");
		FAIL;
	} else
		{
		CHECK;
		count++;
		}
	if (checkevent((XEvent*)&dsgood, &ev))
		FAIL;
	else
		{
		CHECK;
		count++;
		}

	if (!(keys && buttons && valuators) && valuators->num_axes <=6)
	    {
	    CHECKPASS(count);
	    return;
	    }

	dsgood.num_classes = numc > 2 ? 2 : 1;
	data = dsgood.data;
	if (keys)
	    {
	    k = (XKeyStatus *) data;
	    k->class = KeyClass;
	    k->length = sizeof (XKeyStatus);
	    k->num_keys = keys->max_keycode - keys->min_keycode;
	    for (i=0; i<32; i++)
		k->keys[i]=0;
	    if (input)
		for (i=0; i<keys->num_keys; i++)
		    k->keys[i>>3] |= (1 << (i&7));
	    data += k->length;
	    }

	if (valuators->num_axes > 6)
	    {
	    v = (XValuatorStatus *) data;
	    v->class = ValuatorClass;
	    v->length = sizeof (XValuatorStatus);
	    v->num_valuators = valuators->num_axes<6 ? valuators->num_axes : 6;
	    for (i=0; i<6; i++)
		v->valuators[i]=0;
	    if (input)
		for (i=0; i<v->num_valuators; i++)
		    v->valuators[i]=i+6;
	    data += v->length;
	    }

	if ((n=getevent(dpy, &ev)) == 0 || ev.type != ds) {
		if (n)
		    report("Got %d events, first was type %s.",
			n, eventname(ev.type));
		report("Was expecting a DeviceStateNotify event");
		FAIL;
	} else
		{
		CHECK;
		count++;
		}

	for (i=12; i < valuators->num_axes; i+=6)
	    {
	    dsgood.num_classes = 1;
	    v = (XValuatorStatus *) dsgood.data;
	    v->class = ValuatorClass;
	    v->length = sizeof (XValuatorStatus);
	    v->num_valuators = valuators->num_axes-i<6 ? valuators->num_axes-i : 6;
	    for (j=0; j<6; j++)
		v->valuators[j]=0;
	    if (input)
		for (j=0; j<v->num_valuators; j++)
		    v->valuators[j]=i+j;
	    if ((n=getevent(dpy, &ev)) == 0 || ev.type != ds) {
		if (n)
		    report("Got %d events, first was type %s.",
			n, eventname(ev.type));
		report("Was expecting a DeviceStateNotify event");
		FAIL;
	    } else
		{
		CHECK;
		count++;
		}
	    if (checkevent((XEvent*)&dsgood, &ev))
		FAIL;
	    else
		{
		CHECK;
		count++;
		}
	    }
    CHECKPASS(count);
}

>>CODE
XDevice *device;

	if (!Setup_Extension_DeviceInfo(KeyMask))
	    {
	    untested("%s: No input extension key device.\n", TestName);
	    return;
	    }
	if (noext(0))
	    return;

	device = Devs.Key;
	w = defwin(display);
	getds (display, device, False);

>>ASSERTION Good B 3
Touch test to execute miscellaneous code that would not otherwise get executed.
>>STRATEGY
Touch test.
>>CODE
int i, j, axes[6], numc, count;
XDevice *device;
XKeyInfoPtr keys;
XButtonInfoPtr buttons;
XValuatorInfoPtr valuators;

	if (!Setup_Extension_DeviceInfo(KeyMask))
	    {
	    untested("%s: No input extension key device.\n", TestName);
	    return;
	    }
	if (noext(0))
	    return;

	device = Devs.Key;
	w = defwin(display);
	numc = devtype(display, device, &keys, &buttons, &valuators);
	warppointer(display, w, 1, 1);
	for (i=keys->min_keycode; i<keys->max_keycode; i++)
	    devicekeypress (display, device, i);
	if (buttons)
	    for (i=1; i<buttons->num_buttons; i++)
		devicebuttonpress (display, device, i);
	if (valuators)
	    {
	    for (i=0; i<6; i++)
		axes[i]=0;
	    for (i=0; i<valuators->num_axes; i+=6)
		{
		for (j=0; j<6; j++)
		    axes[j]=i+j;
		count = valuators->num_axes-i<6 ? valuators->num_axes-i : 6;
		SimulateDeviceMotionEvent (display, device, False, count, axes,
		    i);
		}
	    }
	getds (display, device, True);
	for (i=keys->min_keycode; i<keys->max_keycode; i++)
	    devicekeyrel (display, device, i);
	devicerelkeys (device);
	if (buttons)
	    {
	    for (i=1; i<buttons->num_buttons; i++)
		devicebuttonrel (display, device, i);
	    devicerelbuttons (device);
	    }
	if (valuators)
	    {
	    for (i=0; i<6; i++)
		axes[i]=0;
	    for (i=0; i<valuators->num_axes; i+=6)
		{
		count = valuators->num_axes-i<6 ? valuators->num_axes-i : 6;
		SimulateDeviceMotionEvent (display, device, False, count, axes, 
		    i);
		}
	    }

>>ASSERTION Good B 3
Touch test to execute miscellaneous code that would not otherwise get executed.
>>STRATEGY
Touch test.
>>CODE
XDevice *device;

	if (!Setup_Extension_DeviceInfo(BtnMask))
	    {
	    untested("%s: Required extension device not present.\n", TestName);
	    return;
	    }
	if (noext(0))
	    return;

	device = Devs.Button;
	w = defwin(display);
	getds (display, device, False);

>>ASSERTION Good B 3
Touch test to execute miscellaneous code that would not otherwise get executed.
>>STRATEGY
Touch test.
>>CODE
int i, j, axes[6], numc, count;
XDevice *device;
XKeyInfoPtr keys;
XButtonInfoPtr buttons;
XValuatorInfoPtr valuators;

	if (!Setup_Extension_DeviceInfo(BtnMask))
	    {
	    untested("%s: Required extension device not present.\n", TestName);
	    return;
	    }
	if (noext(0))
	    return;

	device = Devs.Button;
	w = defwin(display);
	numc = devtype(display, device, &keys, &buttons, &valuators);
	warppointer(display, w, 1, 1);
	for (i=1; i<buttons->num_buttons; i++)
	    devicebuttonpress (display, device, i);
	if (keys)
	    for (i=keys->min_keycode; i<keys->max_keycode; i++)
		devicekeypress (display, device, i);
	if (valuators)
	    {
	    for (i=0; i<6; i++)
		axes[i]=0;
	    for (i=0; i<valuators->num_axes; i+=6)
		{
		for (j=0; j<6; j++)
		    axes[j]=i+j;
		count = valuators->num_axes-i<6 ? valuators->num_axes-i : 6;
		SimulateDeviceMotionEvent (display, device, False, count, axes,
		    i);
		}
	    }
	getds (display, device, True);
	for (i=1; i<buttons->num_buttons; i++)
	    devicebuttonrel (display, device, i);
	devicerelbuttons (device);
	if (keys)
	    {
	    for (i=keys->min_keycode; i<keys->max_keycode; i++)
		devicekeyrel (display, device, i);
	    devicerelkeys (device);
	    }
	if (valuators)
	    {
	    for (i=0; i<6; i++)
		axes[i]=0;
	    for (i=0; i<valuators->num_axes; i+=6)
		{
		count = valuators->num_axes-i<6 ? valuators->num_axes-i : 6;
		SimulateDeviceMotionEvent (display, device, False, count, axes,
		    i);
		}
	    }

>>ASSERTION Good B 3
Touch test to execute miscellaneous code that would not otherwise get executed.
>>STRATEGY
Touch test.
>>CODE
XDevice *device;

	if (!Setup_Extension_DeviceInfo(DModMask))
	    {
	    untested("%s: Required extension device not present.\n", TestName);
	    return;
	    }
	if (noext(0))
	    return;

	w = defwin(display);
	device = Devs.DvMod;
	XSetDeviceMode (display, device, Absolute);
	getds (display, device, False);
>>ASSERTION Good B 3
Touch test to execute miscellaneous code that would not otherwise get executed.
>>STRATEGY
Touch test.
>>CODE
int i, j, axes[6], numc, count;
XDevice *device;
XKeyInfoPtr keys;
XButtonInfoPtr buttons;
XValuatorInfoPtr valuators;

	if (!Setup_Extension_DeviceInfo(DModMask))
	    {
	    untested("%s: Required extension device not present.\n", TestName);
	    return;
	    }
	if (noext(0))
	    return;

	device = Devs.DvMod;
	XSetDeviceMode (display, device, Absolute);
	w = defwin(display);
	numc = devtype(display, device, &keys, &buttons, &valuators);
	warppointer(display, w, 1, 1);
	for (i=0; i<6; i++)
	    axes[i]=0;
	for (i=0; i<valuators->num_axes; i+=6)
	    {
	    for (j=0; j<6; j++)
		axes[j]=i+j;
	    count = valuators->num_axes-i<6 ? valuators->num_axes-i : 6;
	    SimulateDeviceMotionEvent (display, device, False, count, axes, i);
	    }
	if (buttons)
	    for (i=1; i<buttons->num_buttons; i++)
		devicebuttonpress (display, device, i);
	if (keys)
	    for (i=keys->min_keycode; i<keys->max_keycode; i++)
		devicekeypress (display, device, i);
	getds (display, device, True);
	if (buttons)
	    {
	    for (i=1; i<buttons->num_buttons; i++)
		devicebuttonrel (display, device, i);
	    devicerelbuttons (device);
	    }
	if (keys)
	    {
	    for (i=keys->min_keycode; i<keys->max_keycode; i++)
		devicekeyrel (display, device, i);
	    devicerelkeys (device);
	    }
	for (i=0; i<6; i++)
	    axes[i]=0;
	for (i=0; i<valuators->num_axes; i+=6)
	    {
	    count = valuators->num_axes-i<6 ? valuators->num_axes-i : 6;
	    SimulateDeviceMotionEvent (display, device, False, count, axes, i);
	    }

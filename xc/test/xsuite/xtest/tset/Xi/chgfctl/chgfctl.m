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
 * Copyright 1993 by the Hewlett-Packard Company.
 * Copyright 1990, 1991 by UniSoft Group Limited.
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
 * $XConsortium: chgfctl.m,v 1.7 94/09/06 20:55:39 dpw Exp $
 */
>>TITLE XChangeFeedbackControl XINPUT
int
xname()
Display	*display = Dsp;
XDevice	*device;
unsigned long mask;
XFeedbackControl *f;
>>EXTERN
extern ExtDeviceInfo Devs;

>>ASSERTION Good B 3
If the device has feedbacks, they can be set to their largest valid
values.
>>STRATEGY
For all devices that have feedbacks, do a GetFeedbackControl, 
then a ChangeFeecbackControl using the largest valid values.
>>CODE
int i,l,Nfeed2;
XFeedbackState *state2;

if (SetFeedbackInfo (KFeedMask, 0))
    {
    XKbdFeedbackState *K2;
    XKbdFeedbackControl kbdf;
    device = Devs.KbdFeed;
    mask = DvPercent | DvPitch | DvDuration | DvLed | DvKeyClickPercent | DvKey 
	| DvAutoRepeatMode;
    kbdf.class = KbdFeedbackClass;
    kbdf.id = 0;
    kbdf.length = sizeof (XKbdFeedbackControl);
    kbdf.pitch =  0;
    kbdf.percent = 0;
    kbdf.duration = 0;
    kbdf.click = 0;
    kbdf.led_mask = 0xffffffff;
    kbdf.led_value = 0;
    kbdf.key = 0xff;
    kbdf.auto_repeat_mode = AutoRepeatModeOn;
    f = (XFeedbackControl *) &kbdf;
    XCALL;
    state2 = XGetFeedbackControl(display, device, &Nfeed2);
    for(l=0; l<Nfeed2; l++)
        {
       	if (state2->class==KbdFeedbackClass && state2->id==0)
	    {
    	    K2 = (XKbdFeedbackState *) state2;
    	    if (K2->pitch==0 && K2->duration==0 && K2->led_mask==0 &&
	        K2->percent==0 && K2->click==0 &&
		K2->global_auto_repeat == AutoRepeatModeOn)
		{
		trace("%s changed a keyboard feedback\n", TestName);
		PASS;
		}
	    else
		{
		report("%s returned %d %d %d %d %d %d instead of 0 0 0 0 0 1 for a keyboard feedback\n",TestName, 
		    K2->pitch, K2->duration, K2->led_mask, K2->percent, 
		    K2->click, K2->global_auto_repeat);
		FAIL;
		}
	    }
   	state2 = (XFeedbackState *) ((char *) state2 + state2->length);
	}
    }
else
    {
    report("%s could not find any keyboard feedbacks to test.\n",TestName);
    UNTESTED;
    }

if (SetFeedbackInfo (PFeedMask, 0))
    {
    XPtrFeedbackState *P2;
    XPtrFeedbackControl ptrf;
    device = Devs.PtrFeed;
    mask = DvAccelNum | DvAccelDenom | DvThreshold;
    ptrf.class = PtrFeedbackClass;
    ptrf.length = sizeof (XPtrFeedbackControl);
    ptrf.id = 0;
    ptrf.accelNum = 0;
    ptrf.accelDenom = 1;
    ptrf.threshold = 0;
    f = (XFeedbackControl *) &ptrf;
    XCALL;
    state2 = XGetFeedbackControl(display, device, &Nfeed2);
    for(l=0; l<Nfeed2; l++)
	{
	if (state2->class==PtrFeedbackClass && state2->id==0)
	    {
	    P2 = (XPtrFeedbackState *) state2;
	    if (P2->accelNum==0 && P2->accelDenom==1 && P2->threshold==0)
		{
		trace("%s changed a pointer feedback\n", TestName);
		PASS;
		}
	    else
		{
		report("%s returned %d %d %d instead of 0 1 0 for a pointer feedback\n",TestName, P2->accelNum, P2->accelDenom, P2->threshold);
		FAIL;
		}
	    }
	 state2 = (XFeedbackState *) ((char *) state2 + state2->length);
	 }
    }
else
    {
    report("%s could not find any pointer feedbacks to test.\n",TestName);
    UNTESTED;
    }

if (SetFeedbackInfo (SFeedMask, 0))
    {
    XStringFeedbackControl strf;
    XStringFeedbackState *S2;
    device = Devs.StrFeed;
    state2 = XGetFeedbackControl(display, device, &Nfeed2);
    for(i=0; i<Nfeed2; i++)
        {
       	if (state2->class==StringFeedbackClass && state2->id==0)
	    {
	    S2 = (XStringFeedbackState *) state2;
	    break;
	    }
   	state2 = (XFeedbackState *) ((char *) state2 + state2->length);
	}
    strf.class = StringFeedbackClass;
    strf.length = sizeof (XStringFeedbackControl);
    strf.id = 0;
    strf.num_keysyms = S2->max_symbols;
    strf.syms_to_display = S2->syms_supported;
    f = (XFeedbackControl *) &strf;
    mask = DvString;
    XCALL;
    trace("%s changed a string feedback\n",TestName);
    }
else
    {
    report("%s could not find any string feedbacks to test.\n",TestName);
    UNTESTED;
    }

if (SetFeedbackInfo (BFeedMask, 0))
    {
    XBellFeedbackControl belf;
    XBellFeedbackState *B2;
    device = Devs.BelFeed;
    belf.class = BellFeedbackClass;
    belf.length = sizeof (XBellFeedbackControl);
    belf.pitch = 0;
    belf.id = 0;
    belf.percent = 0;
    belf.pitch = 0;
    belf.duration = 100;
    f = (XFeedbackControl *) &belf;
    mask = DvPercent;
    XCALL;
    mask = DvPitch;
    XCALL;
    mask = DvDuration;
    XCALL;
    mask = DvPercent | DvPitch | DvDuration;
    XCALL;
    state2 = XGetFeedbackControl(display, device, &Nfeed2);
    for(l=0; l<Nfeed2; l++)
	{
	if (state2->class==BellFeedbackClass && state2->id==0)
	    {
	    B2 = (XBellFeedbackState *) state2;
	    if (B2->pitch==0 && B2->duration==100 && B2->percent==0)
		{
		trace("%s changed a bell feedback\n", TestName);
		PASS;
		}
	    else
		{
		report("%s returned %d %d %d instead of 0 100 0 for a bell feedback\n",TestName, 
		    B2->pitch, B2->duration, B2->percent);
		FAIL;
		}
	    }
   	state2 = (XFeedbackState *) ((char *) state2 + state2->length);
        }
    }
else
    {
    report("%s could not find any bell feedbacks to test.\n",TestName);
    UNTESTED;
    }

if (SetFeedbackInfo (IFeedMask, 0))
    {
    XIntegerFeedbackControl intf;
    XIntegerFeedbackState *I2;
    device = Devs.IntFeed;
    state2 = XGetFeedbackControl(display, device, &Nfeed2);
    for(i=0; i<Nfeed2; i++)
        {
       	if (state2->class==IntegerFeedbackClass && state2->id==0)
	    {
	    I2 = (XIntegerFeedbackState *) state2;
	    break;
	    }
   	state2 = (XFeedbackState *) ((char *) state2 + state2->length);
	}
    intf.class = IntegerFeedbackClass;
    intf.length = sizeof (XIntegerFeedbackControl);
    intf.int_to_display = I2->minVal;
    intf.id = 0;
    f = (XFeedbackControl *) &intf;
    mask = DvInteger;
    XCALL;
    trace("%s changed an integer feedback\n",TestName);
    }
else
    {
    report("%s could not find any integer feedbacks to test.\n",TestName);
    UNTESTED;
    }

if (SetFeedbackInfo (LFeedMask, 0))
    {
    XLedFeedbackControl ledf;
    XLedFeedbackState *L2;
    device = Devs.LedFeed;
    ledf.class = LedFeedbackClass;
    ledf.length = sizeof (XLedFeedbackControl);
    ledf.id = 0;
    ledf.led_mask = 0xffffffff;
    ledf.led_values = 0xffffffff;
    f = (XFeedbackControl *) &ledf;
    mask = 0;
    XCALL;
    mask = DvLed;
    XCALL;
    state2 = XGetFeedbackControl(display, device, &Nfeed2);
    for(l=0; l<Nfeed2; l++)
	{
   	if (state2->class==LedFeedbackClass && state2->id==0)
	    {
	    L2 = (XLedFeedbackState *) state2;
	    if (L2->led_values==L2->led_mask)
	        {
	        trace("%s changed a led feedback\n",TestName);
	        PASS;
	        }
	    else
	        {
	        report("%s returned %x instead of %x for a led feedback\n",
		    TestName, L2->led_values, L2->led_mask);
		FAIL;
		}
	    }
	state2 = (XFeedbackState *) ((char *) state2 + state2->length);
	}
    }
else
    {
    report("%s could not find any led feedbacks to test.\n",TestName);
    UNTESTED;
    }

>>ASSERTION Good B 3
If the device has feedbacks, they can be set to their default values.
>>STRATEGY
Do a ChangeFeedbackControl, then a SetFeecbackControl using the default values 
for those classes that have defaults.
>>CODE
int i,j,k,ndevices,Nfeed,count = 0;
XKbdFeedbackControl kbdf;
XPtrFeedbackControl ptrf;
XBellFeedbackControl belf;
XKbdFeedbackState *K;
XPtrFeedbackState *P;
XBellFeedbackState *B;
XDeviceInfoPtr list;
XInputClassInfo *ip;
XFeedbackState *state;
int ximajor, first, err;

if (!XQueryExtension (display, INAME, &ximajor, &first, &err)) {
    untested("%s: Input extension not supported.\n", TestName);
    return;
    }
list = XListInputDevices (display, &ndevices);
for (i=0; i<ndevices; i++,list++)
    {
    if (list->use != IsXExtensionDevice)
       continue;
    device = XOpenDevice (display, list->id);
    for (j=0, ip=device->classes; j<device->num_classes; j++,ip++)
	{
	if (ip->input_class != FeedbackClass)
	    continue;
	state = XGetFeedbackControl(display, device, &Nfeed);
	for(k=0; k<Nfeed; k++)
	    {
	    if (state->class==KbdFeedbackClass)
		{
		K = (XKbdFeedbackState *) state;
		mask = DvPercent | DvPitch | DvDuration | DvLed |
		       DvKeyClickPercent | DvKey | DvAutoRepeatMode;
		kbdf.class = KbdFeedbackClass;
		kbdf.id = K->id;
		kbdf.length = sizeof (XKbdFeedbackControl);
		kbdf.pitch =  -1;
		kbdf.percent = -1;
		kbdf.duration = -1;
		kbdf.click = -1;
		kbdf.led_mask = K->led_mask;
		kbdf.led_value = 0xffff;
		kbdf.key = 0x10;
		kbdf.auto_repeat_mode = AutoRepeatModeOn;
		f = (XFeedbackControl *) &kbdf;
		XCALL;
		if (geterr() != Success)
		    FAIL;
		else
		    {
		    CHECK;
		    count++;
		    }

		mask = DvKey | DvAutoRepeatMode;
		kbdf.key = -1;
		XCALL;
		if (geterr() != Success)
		    FAIL;
		else
		    {
		    CHECK;
		    count++;
		    }

		mask = DvAutoRepeatMode;
		XCALL;
		if (geterr() != Success)
		    FAIL;
		else
		    {
		    CHECK;
		    count++;
		    }

		mask = DvKey | DvAutoRepeatMode;
		kbdf.auto_repeat_mode = AutoRepeatModeOff;
		kbdf.key = 8;
		XCALL;
		if (geterr() != Success)
		    FAIL;
		else
		    {
		    CHECK;
		    count++;
		    }

		kbdf.auto_repeat_mode = AutoRepeatModeDefault;
		XCALL;
		if (geterr() != Success)
		    FAIL;
		else
		    {
		    CHECK;
		    count++;
		    }

		mask = DvAutoRepeatMode;
		kbdf.auto_repeat_mode = AutoRepeatModeOff;
		XCALL;
		if (geterr() != Success)
		    FAIL;
		else
		    {
		    CHECK;
		    count++;
		    }

		kbdf.auto_repeat_mode = AutoRepeatModeDefault;
		XCALL;
		if (geterr() != Success)
		    FAIL;
		else
		    {
		    CHECK;
		    count++;
		    }

		mask = DvPercent;
		XCALL;
		if (geterr() != Success)
		    FAIL;
		else
		    {
		    CHECK;
		    count++;
		    }
		}
	    else if (state->class==PtrFeedbackClass)
		{
		P = (XPtrFeedbackState *) state;
		ptrf.class = PtrFeedbackClass;
		ptrf.length = sizeof (XPtrFeedbackControl);
		ptrf.id = P->id;
		ptrf.accelNum = -1;
		ptrf.accelDenom = -1;
		ptrf.threshold = -1;
		f = (XFeedbackControl *) &ptrf;
    		mask = DvAccelNum;
		XCALL;
		if (geterr() != Success)
		    FAIL;
		else
		    {
		    CHECK;
		    count++;
		    }
    		mask = DvAccelDenom;
		XCALL;
		if (geterr() != Success)
		    FAIL;
		else
		    {
		    CHECK;
		    count++;
		    }
    		mask = DvThreshold;
		XCALL;
		if (geterr() != Success)
		    FAIL;
		else
		    {
		    CHECK;
		    count++;
		    }
    		mask = DvAccelNum | DvAccelDenom | DvThreshold;
		XCALL;
		if (geterr() != Success)
		    FAIL;
		else
		    {
		    CHECK;
		    count++;
		    }
		}
	    else if (state->class==BellFeedbackClass)
		{
		B = (XBellFeedbackState *) state;
		belf.class = BellFeedbackClass;
		belf.length = sizeof (XBellFeedbackControl);
		belf.pitch = B->pitch;
		belf.id = B->id;
		belf.percent = -1;
		belf.pitch = -1;
		belf.duration = -1;
		f = (XFeedbackControl *) &belf;
    		mask = DvPercent | DvPitch | DvDuration;
		XCALL;
		if (geterr() != Success)
		    FAIL;
		else
		    {
		    CHECK;
		    count++;
		    }
		}
	    state = (XFeedbackState *) ((char *) state + state->length);
	    }
	}
    }
    if (count)
	CHECKPASS(count);
    else
	UNTESTED;
>>ASSERTION Bad B 3
If the device has no feedbacks, we will get a BadMatch error.
>>STRATEGY
Do a ChangeFeedbackControl, specifying a device that has no feedbacks.
>>CODE BadMatch
int i,j,k,ndevices,Nfeed;

if (Setup_Extension_DeviceInfo(NFeedMask))
    {
    device = Devs.NoFeedback;
    XCALL;
    if (geterr() == BadMatch)
	CHECK;
    else
	FAIL;

    CHECKPASS(1);
    }
else
    untested("%s can't be tested completely because there are no input devices without feedbacks\n",TestName);

>>ASSERTION Bad B 3
If an invalid device is specified, a BadDevice error will result.
>>STRATEGY
Do a ChangeFeedbackControl, specifying an invalid device.
>>CODE baddevice
XDevice bogus;
XID baddevice;
int ximajor, first, err;

    if (!XQueryExtension (display, INAME, &ximajor, &first, &err)) {
	    untested("%s: Input extension not supported.\n", TestName);
	    return;
	    }

    BadDevice (display, baddevice);
    bogus.device_id = -1;
    device = &bogus;
    XCALL;
    if (geterr() == baddevice)
	CHECK;
    else
	FAIL;

    CHECKPASS(1);

>>ASSERTION Bad B 3
If an valid device with an invalid feedback class is specified, 
a BadMatch error will result.
>>STRATEGY
Do a ChangeFeedbackControl, specifying an valid device with an invalid
feedback class.
>>CODE BadMatch

if (SetFeedbackInfo (KFeedMask, 0))
    {
    XKbdFeedbackControl kbdf;
    mask = DvKey;
    kbdf.class = 255;
    kbdf.id = 0;
    kbdf.length = sizeof (XKbdFeedbackControl);
    kbdf.pitch =  0;
    kbdf.percent = -2;
    kbdf.duration = 0;
    kbdf.click = 0;
    kbdf.led_mask = 0;
    kbdf.led_value = 0;
    kbdf.key = 0xff;
    kbdf.auto_repeat_mode = AutoRepeatModeOn;
    f = (XFeedbackControl *) &kbdf;
    device = Devs.KbdFeed;
    XCALL;
    if (geterr() == BadMatch)
	CHECK;
    else
	FAIL;

    CHECKPASS(1);
    }
else
    untested("%s can't be tested completely because there are no input devices with keyboard feedbacks\n",TestName);
>>ASSERTION Bad B 3
If an valid device with a keyboard feedback is specified with a 
a mask of DvKey but not DvAutoRepeatMode, a BadMatch error will
result.
>>STRATEGY
Do a ChangeFeedbackControl, specifying an valid key device with a mask of
DvKey but not DvAutoRepeatMode.
>>CODE BadMatch
XFeedbackState *state2;
KeySym save;
int i, Nfeed2, n=1;

if (SetFeedbackInfo (KFeedMask, 0))
    {
    XKbdFeedbackControl kbdf;
    mask = DvKey;
    kbdf.class = KbdFeedbackClass;
    kbdf.id = 0;
    kbdf.length = sizeof (XKbdFeedbackControl);
    kbdf.pitch =  0;
    kbdf.percent = -2;
    kbdf.duration = 0;
    kbdf.click = 0;
    kbdf.led_mask = 0;
    kbdf.led_value = 0;
    kbdf.key = 0xff;
    kbdf.auto_repeat_mode = AutoRepeatModeOn;
    f = (XFeedbackControl *) &kbdf;
    device = Devs.KbdFeed;
    XCALL;
    if (geterr() == BadMatch)
	CHECK;
    else
	FAIL;

    CHECKPASS(n);
    n++;
    }
else
    untested("%s can't be tested completely because there are no input devices with keyboard feedbacks\n",TestName);

if (SetFeedbackInfo (SFeedMask, 0))
    {
    XStringFeedbackControl strf;
    XStringFeedbackState *S2;
    device = Devs.StrFeed;
    state2 = XGetFeedbackControl(display, device, &Nfeed2);
    for(i=0; i<Nfeed2; i++)
        {
       	if (state2->class==StringFeedbackClass && state2->id==0)
	    {
	    S2 = (XStringFeedbackState *) state2;
	    break;
	    }
   	state2 = (XFeedbackState *) ((char *) state2 + state2->length);
	}
    strf.class = StringFeedbackClass;
    strf.length = sizeof (XStringFeedbackControl);
    strf.id = 0;
    strf.num_keysyms = S2->max_symbols;
    save = S2->syms_supported[0];
    S2->syms_supported[0] = -1;
    strf.syms_to_display = S2->syms_supported;
    f = (XFeedbackControl *) &strf;
    mask = DvString;
    XCALL;
    if (geterr() == BadMatch)
	CHECK;
    else
	FAIL;

    CHECKPASS(n);
    }
else
    untested("%s can't be tested completely because there are no input devices with string feedbacks\n",TestName);

>>ASSERTION Bad B 3
If an valid device with an invalid feedback id is specified,
a BadMatch error will result.
>>STRATEGY
Do a ChangeFeedbackControl, specifying an valid device with an id of 255.
>>CODE BadMatch
int n=1, ret;

if (SetFeedbackInfo (KFeedMask, 0))
    {
    XKbdFeedbackControl kbdf;
    mask = DvKeyClickPercent;
    kbdf.class = KbdFeedbackClass;
    kbdf.id = 255;
    kbdf.length = sizeof (XKbdFeedbackControl);
    f = (XFeedbackControl *) &kbdf;
    device = Devs.KbdFeed;
    ret = XCALL;
    if (geterr() == BadMatch)
	CHECK;
    else
	FAIL;

    CHECKPASS(n);
    n++;
    }
else
    untested("%s can't be tested completely because there are no input devices with keyboard feedbacks\n",TestName);

if (SetFeedbackInfo (SFeedMask, 0))
    {
    XStringFeedbackControl strf;
    strf.class = StringFeedbackClass;
    strf.length = sizeof (XStringFeedbackControl);
    strf.id = 255;
    strf.num_keysyms = 0;
    f = (XFeedbackControl *) &strf;
    device = Devs.StrFeed;
    mask = DvString;
    ret = XCALL;
    if (geterr() == BadMatch)
	CHECK;
    else
	FAIL;

    CHECKPASS(n);
    n++;
    }
else
    untested("%s can't be tested completely because there are no input devices with string feedbacks\n",TestName);

if (SetFeedbackInfo (PFeedMask, 0))
    {
    XPtrFeedbackState *P2;
    XPtrFeedbackControl ptrf;
    mask = DvAccelNum | DvAccelDenom | DvThreshold;
    ptrf.class = PtrFeedbackClass;
    ptrf.length = sizeof (XPtrFeedbackControl);
    ptrf.id = 255;
    f = (XFeedbackControl *) &ptrf;
    device = Devs.PtrFeed;
    ret = XCALL;
    if (geterr() == BadMatch)
	CHECK;
    else
	FAIL;

    CHECKPASS(n);
    n++;
    }
else
    untested("%s can't be tested completely because there are no input devices with pointer feedbacks\n",TestName);

if (SetFeedbackInfo (BFeedMask, 0))
    {
    XBellFeedbackControl belf;
    XBellFeedbackState *B2;
    belf.class = BellFeedbackClass;
    belf.length = sizeof (XBellFeedbackControl);
    belf.id = 255;
    f = (XFeedbackControl *) &belf;
    device = Devs.BelFeed;
    mask = DvPercent | DvPitch | DvDuration;
    ret = XCALL;
    if (geterr() == BadMatch)
	CHECK;
    else
	FAIL;

    CHECKPASS(n);
    n++;
    }
else
    untested("%s can't be tested completely because there are no input devices with bell feedbacks\n",TestName);

if (SetFeedbackInfo (IFeedMask, 0))
    {
    XIntegerFeedbackControl intf;
    intf.class = IntegerFeedbackClass;
    intf.length = sizeof (XIntegerFeedbackControl);
    intf.id = 255;
    f = (XFeedbackControl *) &intf;
    device = Devs.IntFeed;
    mask = DvInteger;
    ret = XCALL;
    if (geterr() == BadMatch)
	CHECK;
    else
	FAIL;

    CHECKPASS(n);
    n++;
    }
else
    untested("%s can't be tested completely because there are no input devices with integer feedbacks\n",TestName);

if (SetFeedbackInfo (LFeedMask, 0))
    {
    XLedFeedbackControl ledf;
    XLedFeedbackState *L2;
    ledf.class = LedFeedbackClass;
    ledf.length = sizeof (XLedFeedbackControl);
    ledf.id = 255;
    f = (XFeedbackControl *) &ledf;
    device = Devs.LedFeed;
    mask = DvLed;
    ret = XCALL;
    if (geterr() == BadMatch)
	CHECK;
    else
	FAIL;

    CHECKPASS(n);
    n++;
    }
else
    untested("%s can't be tested completely because there are no input devices with led feedbacks\n",TestName);

>>ASSERTION Bad B 3
If an valid device is specified with a value that is out of range,
a BadValue error will result.
>>STRATEGY
Do a ChangeFeedbackControl, specifying an valid device with a value
out of range.
>>CODE BadValue
XFeedbackState *state2;
int i, Nfeed2, n=1;

if (SetFeedbackInfo (KFeedMask, 0))
    {
    XKbdFeedbackControl kbdf;
    device = Devs.KbdFeed;
    mask = DvPercent;
    kbdf.class = KbdFeedbackClass;
    kbdf.id = 0;
    kbdf.length = sizeof (XKbdFeedbackControl);
    kbdf.pitch =  0;
    kbdf.percent = -2;
    kbdf.duration = 0;
    kbdf.click = 0;
    kbdf.led_mask = 0;
    kbdf.led_value = 0;
    kbdf.key = 0xff;
    kbdf.auto_repeat_mode = AutoRepeatModeOn;
    f = (XFeedbackControl *) &kbdf;
    XCALL;
    if (geterr() == BadValue)
	CHECK;
    else
	FAIL;

    CHECKPASS(n);
    n++;

    mask =  DvPitch;
    kbdf.pitch =  -2;
    kbdf.percent = 0;
    XCALL;
    if (geterr() == BadValue)
	CHECK;
    else
	FAIL;

    CHECKPASS(n);
    n++;

    mask =  DvDuration;
    kbdf.pitch =  0;
    kbdf.duration = -2;
    XCALL;
    if (geterr() == BadValue)
	CHECK;
    else
	FAIL;

    CHECKPASS(n);
    n++;

    mask =  DvKeyClickPercent;
    kbdf.click =  -2;
    kbdf.duration = 0;
    XCALL;
    if (geterr() == BadValue)
	CHECK;
    else
	FAIL;

    CHECKPASS(n);
    n++;

    mask =  DvKey;
    kbdf.click =  0;
    kbdf.key = 0;
    XCALL;
    if (geterr() == BadValue)
	CHECK;
    else
	FAIL;
    CHECKPASS(n);
    n++;

    mask =  DvAutoRepeatMode;
    kbdf.auto_repeat_mode = -2;
    kbdf.key = 10;
    XCALL;
    if (geterr() == BadValue)
	CHECK;
    else
	FAIL;
    CHECKPASS(n);
    n++;
    }
else
    untested("%s can't be tested completely because there are no input devices with keyboard feedbacks\n",TestName);

if (SetFeedbackInfo (PFeedMask, 0))
    {
    XPtrFeedbackControl ptrf;
    device = Devs.PtrFeed;
    mask = DvAccelNum | DvAccelDenom | DvThreshold;
    ptrf.class = PtrFeedbackClass;
    ptrf.length = sizeof (XPtrFeedbackControl);
    ptrf.id = 0;
    ptrf.accelNum = -2;
    ptrf.accelDenom = 1;
    ptrf.threshold = 0;
    f = (XFeedbackControl *) &ptrf;
    XCALL;
    if (geterr() == BadValue)
	CHECK;
    else
	FAIL;

    CHECKPASS(n);
    n++;

    mask = DvAccelDenom;
    ptrf.accelNum = 0;
    ptrf.accelDenom = 0;
    XCALL;
    if (geterr() == BadValue)
	CHECK;
    else
	FAIL;

    CHECKPASS(n);
    n++;

    mask = DvThreshold;
    ptrf.accelDenom = 1;
    ptrf.threshold = -2;
    XCALL;
    if (geterr() == BadValue)
	CHECK;
    else
	FAIL;

    CHECKPASS(n);
    n++;
    }
else
    untested("%s can't be tested completely because there are no input devices with pointer feedbacks\n",TestName);

if (SetFeedbackInfo (BFeedMask, 0))
    {
    XBellFeedbackControl belf;
    belf.class = BellFeedbackClass;
    belf.length = sizeof (XBellFeedbackControl);
    belf.pitch = 0;
    belf.id = 0;
    belf.percent = -2;
    belf.pitch = 0;
    belf.duration = 100;
    f = (XFeedbackControl *) &belf;
    device = Devs.BelFeed;

    mask = DvPercent;
    XCALL;
    if (geterr() == BadValue)
	CHECK;
    else
	FAIL;

    CHECKPASS(n);
    n++;

    mask = DvPitch;
    belf.percent = 0;
    belf.pitch = -2;
    XCALL;
    if (geterr() == BadValue)
	CHECK;
    else
	FAIL;

    CHECKPASS(n);
    n++;

    mask = DvDuration;
    belf.duration = -2;
    belf.pitch = 0;
    XCALL;
    if (geterr() == BadValue)
	CHECK;
    else
	FAIL;

    CHECKPASS(n);
    n++;
    }
else
    untested("%s can't be tested completely because there are no input devices with bell feedbacks\n",TestName);

if (SetFeedbackInfo (SFeedMask, 0))
    {
    XStringFeedbackControl strf;
    XStringFeedbackState *S2;
    state2 = XGetFeedbackControl(display, device, &Nfeed2);
    for(i=0; i<Nfeed2; i++)
        {
       	if (state2->class==StringFeedbackClass && state2->id==0)
	    {
	    S2 = (XStringFeedbackState *) state2;
	    break;
	    }
   	state2 = (XFeedbackState *) ((char *) state2 + state2->length);
	}
    strf.class = StringFeedbackClass;
    strf.length = sizeof (XStringFeedbackControl);
    strf.id = 0;
    strf.num_keysyms = S2->max_symbols+1;
    strf.syms_to_display = S2->syms_supported;
    f = (XFeedbackControl *) &strf;
    device = Devs.StrFeed;
    XCALL;
    if (geterr() == BadValue)
	CHECK;
    else
	FAIL;

    CHECKPASS(n);
    }
else
    untested("%s can't be tested completely because there are no input devices with string feedbacks\n",TestName);

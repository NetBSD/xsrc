/*
 * Copyright 1993 by the Hewlett-Packard Company.
 *
 * Permission to use, copy, modify, distribute, and sell this software and
 * its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation, and that the names of HP, and UniSoft not be
 * used in advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.  HP and UniSoft
 * make no representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 *
 * $XConsortium: XInput.c,v 1.5 94/09/06 19:40:10 dpw Exp $
 */
#include <stdio.h>
#include "extensions/XIproto.h"
#include "extensions/XInput.h"
#include "XItest.h"
#include "tet_api.h"

static XEventClass propclasses[32];
static XDevice *baddev, *testdev;
static Display *dpy2;
static xKbdFeedbackCtl xKbd;
static xPtrFeedbackCtl xPtr;
static struct _strdata
    {
    xStringFeedbackCtl xStr;
    KeySym strsyms[4];
    }strdata;
static xIntegerFeedbackCtl xInt;
static xLedFeedbackCtl xLed;
static xBellFeedbackCtl xBel;
static ExtDeviceInfo zdev;
static int pclass;

ExtDeviceInfo Devs;
XEventClass devicekeypressclass, devicebuttonpressclass, 
	    devicemotionnotifyclass;
XEventClass classes[32];
char *FeedbackData;
xEvent kev[2];
int NumKeys,
    NumButtons,
    NumValuators,
    SavID,
    SavPID,
    ButtonMapLength,
    nclass,
    nevent,
    event_types[32],
    MinKeyCode, 
    MaxKeyCode,
    Feedback_Class, 
    FeedbackDevice, 
    FeedbackSize, 
    FeedbackMask;

extern int XInputMajorOpcode;
extern int unexp_err();

Close_Extension_Display()
    {
    XCloseDisplay(dpy2);
    dpy2 = NULL;
    }

Setup_Extension_DeviceInfo(dmask)
    unsigned int dmask;
    {
    char *disp;
    int handle_x_errors();
    int i, j, k, num_feedbacks,ndevices,numvaluators;
    XModifierKeymap *mmap;
    XDevice *dev;
    Bool nokeys, nobuttons, novaluators;
    unsigned int mask=0;
    extern Display *Dsp;
    XDeviceInfoPtr list;
    XAnyClassPtr	any;
    XInputClassInfo *ip;
    XFeedbackState *state;
    Bool init_xinput();
    int devicekeypress=0,
	   devicekeyrelease,
	   devicebuttonpress=0,
	   devicebuttonrelease,
	   devicemotionnotify=0,
	   devicefocusin=0,
	   devicefocusout,
	   proximityin=0,
	   proximityout,
	   devicestatenotify=0,
	   devicemappingnotify,
	   devicebmapnotify,
	   changedevicenotify;

    XEventClass bchangedevicenotifyclass,
	    devicekeyreleaseclass,
            devicebuttonreleaseclass, 
	    devicefocusinclass, 
	    devicefocusoutclass, 
	    proximityinclass,
	    proximityoutclass, 
	    devicestatenotifyclass, 
	    devicebmapclass, 
	    devicemappingnotifyclass,
	    kchangedevicenotifyclass;

    /*
     * Get the display to use and open it.
     */
    disp = tet_getvar("XT_DISPLAY");

    if (disp == (char*)0) {
	report("XT_DISPLAY was not set.  Can not continue.");
	return;
    }

    dpy2 = XOpenDisplay(disp);
    if (!init_xinput(dpy2))
	return False;
    XSetErrorHandler(handle_x_errors);
    nevent = 0;
    nclass = 0;
    pclass = 0;
    Devs = zdev; 

    list = XListInputDevices (dpy2, &ndevices);
    for (i=0; i<ndevices; i++,list++)
	{
	if (list->use == IsXKeyboard)
	    SavID = list->id;
	if (list->use == IsXPointer)
	    SavPID = list->id;
	if (list->use != IsXExtensionDevice)
	    continue;
	dev = XOpenDevice (dpy2, list->id);
	nokeys = True;
	nobuttons = True;
	novaluators = True;
	any = (XAnyClassPtr) (list->inputclassinfo);
	for (j=0,numvaluators=0; j<list->num_classes; j++)
	    {
	    switch (any->class)
		{
		case KeyClass:
		    {
		    XKeyInfo *K = (XKeyInfo *) any;

		    nokeys = False;
		    testdev = dev;
		    baddev = 0;
		    mmap = XGetDeviceModifierMapping (dpy2, dev);
		    if (baddev!=testdev && mmap->max_keypermod>0)
			{
			Devs.Key = Devs.Mod = dev;
			mask |= KeyMask;
			mask |= ModMask;
		        DeviceMappingNotify (dev,  devicemappingnotify, 
			    devicemappingnotifyclass);
		        DeviceKeyPress (dev, devicekeypress, 
			    devicekeypressclass);
		        DeviceKeyRelease (dev, devicekeyrelease, 
			    devicekeyreleaseclass);
		        ChangeDeviceNotify (dev, changedevicenotify, 
			    kchangedevicenotifyclass);
		        MinKeyCode = K->min_keycode;
		        MaxKeyCode = K->max_keycode;
		        NumKeys = K->num_keys;
			}
		    if (!Devs.Mod)
			{
		        Devs.Key = dev;
			mask |= KeyMask;
		        DeviceMappingNotify (dev,  devicemappingnotify, 
			    devicemappingnotifyclass);
		        DeviceKeyPress (dev, devicekeypress, 
			    devicekeypressclass);
		        DeviceKeyRelease (dev, devicekeyrelease, 
			    devicekeyreleaseclass);
		        ChangeDeviceNotify (dev, changedevicenotify, 
			    kchangedevicenotifyclass);
		        MinKeyCode = K->min_keycode;
		        MaxKeyCode = K->max_keycode;
		        NumKeys = K->num_keys;
			}
		    kev[0].u.u.type = devicekeypress;
		    kev[1].u.u.type = devicekeypress - 1; 
		    break;
		    }
		case ButtonClass:
		    {
		    XButtonInfo *B = (XButtonInfo *) any;
		    unsigned char map[256];

		    if (B->num_buttons==0)
			continue;
		    Devs.Button = dev;
		    mask |= BtnMask;
		    DeviceButtonPress (dev, devicebuttonpress, 
		        devicebuttonpressclass);
		    DeviceButtonRelease (dev, devicebuttonrelease, 
		        devicebuttonreleaseclass);
		    DeviceMappingNotify (dev,  devicebmapnotify, devicebmapclass);
		    ButtonMapLength = XGetDeviceButtonMapping (dpy2, dev, map,
			256);
		    NumButtons = B->num_buttons;
		    nobuttons = False;
		    break;
		    }
		case ValuatorClass:
		    {
		    XValuatorInfo *V = (XValuatorInfo *) any;
		    numvaluators = V->num_axes;
		    Devs.Valuator = dev;
		    mask |= ValMask;
		    DeviceMotionNotify (dev, devicemotionnotify,
		        devicemotionnotifyclass);
		    ChangeDeviceNotify (dev, changedevicenotify, 
			bchangedevicenotifyclass);
		    novaluators = False;
		    break;
		    }
		default:
		    break;
		}
	    any = (XAnyClassPtr) ((char *) any + any->length);
	    }

	if (nokeys)
	    {
	    Devs.NoKeys = dev;
	    mask |= NKeysMask;
	    }
	if (nobuttons)
	    {
	    Devs.NoButtons = dev;
	    mask |= NBtnsMask;
	    }
	if (novaluators)
	    {
	    Devs.NoValuators = dev;
	    mask |= NValsMask;
	    }
	mask |= AnyMask;
	if (Devs.Key)
	    Devs.Any = Devs.Key;
	else if (Devs.Button)
	    Devs.Any = Devs.Button;
	else if (Devs.Valuator)
	    Devs.Any = Devs.Valuator;
	else
	    mask &= ~AnyMask;
	for (ip= dev->classes, j=0; j<dev->num_classes; j++, ip++)
	    if (ip->input_class == FocusClass)
		{
		DeviceFocusIn (dev, devicefocusin, devicefocusinclass);
		DeviceFocusOut (dev, devicefocusout, devicefocusoutclass);
		Devs.Focus = dev;
		mask |= FocusMask;
		}
	    else if (ip->input_class == ProximityClass)
		{
		ProximityIn (dev, proximityin, proximityinclass);
		ProximityOut (dev, proximityout, proximityoutclass);
		}
	    else if (ip->input_class == OtherClass)
		{
		DeviceStateNotify (dev, devicestatenotify, devicestatenotifyclass);
		}

	if ((dmask & DCtlMask) && !novaluators)
	    {
	    int res = 1;
	    XDeviceResolutionControl dctl;

	    testdev = dev;
	    baddev = 0;
	    dctl.first_valuator=0;
	    dctl.num_valuators=1;
	    dctl.resolutions = &res;
	    XChangeDeviceControl (dpy2, dev, DEVICE_RESOLUTION, &dctl);
	    XSync(dpy2,0);
	    if (baddev!=testdev) {
		Devs.DvCtl = dev;
		mask |= DCtlMask;
	        NumValuators=numvaluators;
		}
	    else
		{
		Devs.NDvCtl = dev;
		mask |= NDvCtlMask;
		}
	    }
	else if ((dmask & DValMask) && !novaluators)
	    {
	    int val = 1;

	    testdev = dev;
	    baddev = 0;
	    XSetDeviceValuators (dpy2, dev, &val, 0, 1);
	    XSync(dpy2,0);
	    if (baddev!=testdev) {
		Devs.DvVal = dev;
		mask |= DValMask;
	        NumValuators=numvaluators;
		}
	    }
	else if ((dmask & DModMask) && !novaluators)
	    {
	    testdev = dev;
	    baddev = 0;
	    XSetDeviceMode (dpy2, dev, Absolute);
	    XSync(dpy2,0);
	    if (baddev!=testdev) {
		Devs.DvMod = dev;
		mask |= DModMask;
		}
	    }
	else if (!novaluators)
	    NumValuators=numvaluators;
	baddev = dev;
	Devs.NoFeedback=0;
	state = NULL;
	state = XGetFeedbackControl (dpy2, dev, &num_feedbacks);
	if (state==NULL || Devs.NoFeedback==baddev)
	    continue;
	for (k=0; k<num_feedbacks; k++)
	    {
	    if (state->class == KbdFeedbackClass)
		{
		Devs.KbdFeed = dev;
		mask |= KFeedMask;
		xKbd.class = KbdFeedbackClass;
		xKbd.id = 0;
		xKbd.length = sizeof(xKbdFeedbackCtl);
		xKbd.key = 10;
		xKbd.auto_repeat_mode = AutoRepeatModeOn;
		xKbd.click = 100;
		xKbd.percent = 100;
		xKbd.pitch = 100;
		xKbd.duration = 100;
		xKbd.led_mask = 0xffffffff;
		xKbd.led_values = 0xffffffff;
		}
	    if (state->class == PtrFeedbackClass)
		{
		Devs.PtrFeed = dev;
		mask |= PFeedMask;
		xPtr.class = PtrFeedbackClass;
		xPtr.id = 0;
		xPtr.length = sizeof(xPtrFeedbackCtl);
		xPtr.num = 1;
		xPtr.denom = 1;
		xPtr.thresh = 1;
		}
	    if (state->class == IntegerFeedbackClass)
		{
		Devs.IntFeed = dev;
		mask |= IFeedMask;
		xInt.class = IntegerFeedbackClass;
		xInt.id = 0;
		xInt.length = sizeof(xIntegerFeedbackCtl);
		xInt.int_to_display = 0x7fffffff;
		}
	    if (state->class == StringFeedbackClass)
		{
		XStringFeedbackState *S = (XStringFeedbackState *) state;
		Devs.StrFeed = dev;
		mask |= SFeedMask;
		strdata.xStr.class = StringFeedbackClass;
		strdata.xStr.id = 0;
		strdata.xStr.length = sizeof(xStringFeedbackCtl);
		strdata.xStr.num_keysyms = 4;
		for (j=0; j<4; j++)
		    strdata.strsyms[j] = S->syms_supported[j];

		}
	    if (state->class == BellFeedbackClass)
		{
		Devs.BelFeed = dev;
		mask |= BFeedMask;
		xBel.class = BellFeedbackClass;
		xBel.id = 0;
		xBel.length = sizeof(xBellFeedbackCtl);
		xBel.percent = 100;
		xBel.pitch = 100;
		xBel.duration = 100;
		}
	    if (state->class == LedFeedbackClass)
		{
		Devs.LedFeed = dev;
		mask |= LFeedMask;
		xLed.class = LedFeedbackClass;
		xLed.id = 0;
		xLed.length = sizeof(xLedFeedbackCtl);
		xLed.led_mask = 0xffffffff;
		xLed.led_values = 0xffffffff;
		}
	    state = (XFeedbackState *) ((char *) state + state->length);
	    }
	}
    if (devicekeypress)
	{
	propclasses[pclass++] = devicekeypressclass;
	propclasses[pclass++] = devicekeyreleaseclass;
	classes[nclass++] = devicekeypressclass;
	classes[nclass++] = devicekeyreleaseclass;
	classes[nclass++]=kchangedevicenotifyclass;
	classes[nclass++]=devicemappingnotifyclass;
	event_types[nevent++] = devicekeypress;
	event_types[nevent++] = devicekeyrelease;
	event_types[nevent++] = devicemappingnotify;
	}
    if (devicebuttonpress)
	{
	propclasses[pclass++]=devicebuttonpressclass;
	propclasses[pclass++]=devicebuttonreleaseclass;
	classes[nclass++]=devicebuttonpressclass;
	classes[nclass++]=devicebuttonreleaseclass;
	classes[nclass++]=devicebmapclass;
	event_types[nevent++] = devicebuttonpress;
	event_types[nevent++] = devicebuttonrelease;
	event_types[nevent++] = devicebmapnotify;
	}
    if (devicemotionnotify)
	{
	propclasses[pclass++]=devicemotionnotifyclass;
	classes[nclass++]=devicemotionnotifyclass;
	classes[nclass++]=bchangedevicenotifyclass;
	event_types[nevent++] = devicemotionnotify;
	}
    if (devicefocusin)
	{
	classes[nclass++]=devicefocusinclass;
	classes[nclass++]=devicefocusoutclass;
	event_types[nevent++] = devicefocusin;
	event_types[nevent++] = devicefocusout;
	}
    if (proximityin)
	{
	propclasses[pclass++]=proximityinclass;
	propclasses[pclass++]=proximityoutclass;
	classes[nclass++]=proximityinclass;
	classes[nclass++]=proximityoutclass;
	event_types[nevent++] = proximityin;
	event_types[nevent++] = proximityout;
	}
    if (devicestatenotify)
	{
	classes[nclass++]=devicestatenotifyclass;
	event_types[nevent++] = devicestatenotify;
	event_types[nevent++] = changedevicenotify;
	}
    XSetErrorHandler(unexp_err);
    if ((mask & dmask) == dmask)
	return (True);
    else
	return (False);
    }

SuppressExtensionEvents(dpy, win)
    Display *dpy;
    Window win;
    {
    XChangeDeviceDontPropagateList(dpy, win, pclass, propclasses, AddToList);
    }

SelectExtensionEvents(dpy, win)
    Display *dpy;
    Window win;
    {
    XSelectExtensionEvent(dpy, win, classes, nclass);
    }

MinMaxKeys(dpy, dev, min, max, num)
    Display *dpy;
    XDevice *dev;
    int *min, *max, *num;
    {
    int i, j, ndevices;
    XDeviceInfoPtr list;
    XAnyClassPtr any;

    list = XListInputDevices (dpy, &ndevices);
    for (i=0; i<ndevices; i++,list++)
	{
	if (list->id != dev->device_id)
	    continue;
	any = (XAnyClassPtr) (list->inputclassinfo);
	for (j=0; j<list->num_classes; j++)
	    {
	    if (any->class == KeyClass)
		{
		XKeyInfo *K = (XKeyInfo *) any;
		*min = K->min_keycode;
		*max = K->max_keycode;
		*num = K->num_keys;
		break;
		}
	    any = (XAnyClassPtr) ((char *) any + any->length);
	    }
	}
    }

SetFeedbackInfo (mask, id)
    unsigned int mask;
    XID id;
    {
    if (!Setup_Extension_DeviceInfo(mask))
	return(False);
    if (mask==KFeedMask && Devs.KbdFeed!=NULL)
	{
	Feedback_Class = KbdFeedbackClass;
	FeedbackDevice = Devs.KbdFeed->device_id;
	FeedbackSize = sizeof(xKbdFeedbackCtl);
	xKbd.id = id;
	FeedbackData = (char *) &xKbd;
	FeedbackMask = DvLed | DvLedMode | DvKey | DvAutoRepeatMode |
	    DvPercent | DvPitch | DvDuration | DvKeyClickPercent;
	return(True);
	}
    else if (mask==PFeedMask && Devs.PtrFeed!=NULL)
	{
	Feedback_Class = PtrFeedbackClass;
	FeedbackDevice = Devs.PtrFeed->device_id;
	FeedbackSize = sizeof(xPtrFeedbackCtl);
	xPtr.id = id;
	FeedbackData = (char *) &xPtr;
	FeedbackMask = DvAccelNum | DvAccelDenom | DvThreshold;
	return(True);
	}
    else if (mask==IFeedMask && Devs.IntFeed!=NULL)
	{
	Feedback_Class = IntegerFeedbackClass;
	FeedbackDevice = Devs.IntFeed->device_id;
	FeedbackSize = sizeof(xIntegerFeedbackCtl);
	xInt.id = id;
	FeedbackData = (char *) &xInt;
	FeedbackMask = DvInteger;
	return(True);
	}
    else if (mask==SFeedMask && Devs.StrFeed!=NULL)
	{
	Feedback_Class = StringFeedbackClass;
	FeedbackDevice = Devs.StrFeed->device_id;
	FeedbackSize = sizeof(struct _strdata);
	strdata.xStr.id = id;
	FeedbackData = (char *) &strdata;
	FeedbackMask = DvString;
	return(True);
	}
    else if (mask==BFeedMask && Devs.BelFeed!=NULL)
	{
	Feedback_Class = BellFeedbackClass;
	FeedbackDevice = Devs.BelFeed->device_id;
	FeedbackSize = sizeof(xBellFeedbackCtl);
	xBel.id = id;
	FeedbackData = (char *) &xBel;
	FeedbackMask = DvPercent | DvPitch | DvDuration;
	return(True);
	}
    else if (mask==LFeedMask && Devs.LedFeed!=NULL)
	{
	Feedback_Class = LedFeedbackClass;
	FeedbackDevice = Devs.LedFeed->device_id;
	FeedbackSize = sizeof(xLedFeedbackCtl);
	xLed.id = id;
	FeedbackData = (char *) &xLed;
	FeedbackMask = DvLed;
	return(True);
	}
    else
	return(False);
    }
/***********************************************************************
 *
 * This function handles X errors.
 *
 */

handle_x_errors (disp, err)
    Display     *disp;
    XErrorEvent *err;
    {
    if (err->request_code == XInputMajorOpcode)
	if (err->minor_code == X_GetFeedbackControl &&
	    err->error_code == BadMatch)
	    {
	    Devs.NoFeedback = baddev;
	    }
	else if (err->minor_code == X_ChangeDeviceControl &&
	    err->error_code == BadMatch)
	    {
	    baddev = testdev;
	    }
	else if (err->minor_code == X_SetDeviceValuators &&
	    err->error_code == BadMatch)
	    {
	    baddev = testdev;
	    }
	else if (err->minor_code == X_SetDeviceMode &&
	    err->error_code == BadMatch)
	    {
	    baddev = testdev;
	    }
	else if (err->minor_code == X_GetDeviceModifierMapping &&
	    err->error_code == BadMatch)
	    {
	    baddev = testdev;
	    }
    }

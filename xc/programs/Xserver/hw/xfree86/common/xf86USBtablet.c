/*
 * Copyright (c) 1999 Lennart Augustsson <augustss@netbsd.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/* $XFree86:  $ */

/*
 * Driver for USB HID tablet devices.
 * Works for:
 *   Wacom PenPartner
 */

#include <stdio.h>
#include <dev/usb/usb.h>
#include <dev/usb/usbhid.h>

#ifdef USB_GET_REPORT_ID
#define USB_NEW_HID
#endif

#ifdef USB_NEW_HID
#include <usbhid.h>
#else
#include <usb.h>
#endif

#include "Xos.h"

#define NEED_EVENTS
#include "X.h"
#include "Xproto.h"
#include "misc.h"
#include "inputstr.h"
#include "scrnintstr.h"
#include "XI.h"
#include "XIproto.h"
#include "keysym.h"

#include "compiler.h"

#include "xf86.h"
#include "xf86Procs.h"
#include "xf86_OSlib.h"
#include "xf86_Config.h"
#include "xf86Xinput.h"
#include "atKeynames.h"
#include "xf86Version.h"

#include "osdep.h"
#include "exevents.h"

#include "extnsionst.h"
#include "extinit.h"

#define SYSCALL(call) while(((call) == -1) && (errno == EINTR))
#define ABS(x) ((x) > 0 ? (x) : -(x))
#define mils(res) (res * 1000 / 2.54) /* resolution */

#undef DEBUG
#define DEBUG 1

static int debug_level = 0;
#if DEBUG
#define DBG(lvl, f) {if ((lvl) <= debug_level) f;}
#else
#define DBG(lvl, f)
#endif

#define STYLUS_SEC	"usbstylus" /* config section name */
#define STYLUS_XI	"Stylus"    /* X device name for the stylus */
#define STYLUS_ID	1	    /* local id */

#define ERASER_SEC	"usberaser" /* config section name */
#define ERASER_XI	"Eraser"    /* X device name for the stylus */
#define ERASER_ID	2	    /* local id */

#define ABSOLUTE_FLAG	0x10000

#define NBUTTONS 4
#define NAXES 5	/* X, Y, Pressure, Tilt-X, Tilt-Y */

typedef struct USBTDevice USBTDevice, *USBTDevicePtr;

typedef struct {
	char		*devName;
	int		nDevs;
	USBTDevicePtr	first;
	double		factorX;
	double		factorY;
	hid_item_t	hidX;
	hid_item_t	hidY;
	hid_item_t	hidTiltX;
	hid_item_t	hidTiltY;
	hid_item_t	hidIn_Range;
	hid_item_t	hidTip_Pressure;
	hid_item_t	hidBarrel_Switch;
	hid_item_t	hidInvert;
	int		reportSize;
	int		reportId;
	USBTDevicePtr	currentProxDev;
} USBTCommon, *USBTCommonPtr;

typedef struct {
	int	x, y, pressure, buttons, xTilt, yTilt, proximity;
} USBTState;

struct USBTDevice {
	USBTCommonPtr	comm;
	USBTDevicePtr	next;
	LocalDevicePtr	local;
	USBTState	state;
	int		threshold;
	int		thresCent;
	int		suppress;
	int		flags;
};
	

#if NeedFunctionPrototypes
static LocalDevicePtr xf86USBTAllocateStylus(void);
static LocalDevicePtr xf86USBTAllocateEraser(void);
static LocalDevicePtr xf86USBTAllocate(char *, int);
static Bool xf86USBTConfig(LocalDevicePtr *, int, int, LexPtr);
static int xf86USBTProc(DeviceIntPtr, int);
static void xf86USBTReadInput(LocalDevicePtr);
static int xf86USBTChangeControl(LocalDevicePtr, xDeviceCtl *);
static int xf86USBTSwitchMode(ClientPtr, DeviceIntPtr, int);
static void xf86USBTClose(LocalDevicePtr);
static Bool xf86USBTConvert(LocalDevicePtr, int, int, int, int, int,
			   int, int, int, int *, int *);
static Bool xf86USBTReverseConvert(LocalDevicePtr, int, int, int *);
static void xf86USBTControlProc(DeviceIntPtr, PtrCtrl *);
static int xf86USBTOpenDevice(DeviceIntPtr);
static void xf86USBTSendEvents(LocalDevicePtr, int, USBTState *);
static void xf86USBTSendButtons(LocalDevicePtr, int, int, int, int, int, int);
static void xf86USBTOutOfProx(USBTDevicePtr prx);
static void xf86USBTIntoProx(USBTDevicePtr prx, USBTState *ds);
#endif

#define PORT		1
#define DEVICENAME	2
#define DEBUG_LEVEL     3
#define HISTORY_SIZE	4
#define ALWAYS_CORE	5
#define THRESHOLD	6
#define SUPPRESS	7

static SymTabRec USBTTab[] = {
	{ ENDSUBSECTION,	"endsubsection" },
	{ PORT,			"port" },
	{ DEVICENAME,		"devicename" },
	{ DEBUG_LEVEL,		"debuglevel" },
	{ HISTORY_SIZE,		"historysize" },
	{ ALWAYS_CORE,		"alwayscore" },
	{ THRESHOLD,		"threshold" },
	{ SUPPRESS,		"suppress" },
	{ -1,			"" }
};

static Bool
xf86USBTConfig(LocalDevicePtr    *array,
              int               inx,
              int               max,
	      LexPtr            val)
{
	LocalDevicePtr	dev = array[inx];
	USBTDevicePtr	priv = (USBTDevicePtr)dev->private, pr;
	USBTCommonPtr	comm = priv->comm;
	int		token;
	int		i;
    
	DBG(1, ErrorF("xf86USBTConfig\n"));

	if (xf86GetToken(USBTTab) != PORT) {
		xf86ConfigError("PORT option must be the first option of a USBT SubSection");
	}
    
	if (xf86GetToken(NULL) != STRING)
		xf86ConfigError("Option string expected");
		
	/* try to find another usbtablet device which share the same port */
	for (i = 0; i < max; i++) {
		if (i == inx)
			continue;
		pr = (USBTDevicePtr)array[i]->private;
		if ((array[i]->device_config == xf86USBTConfig) &&
		    (strcmp(pr->comm->devName, val->str) == 0)) {
			DBG(2, ErrorF("xf86USBTConfig usbtablet port share "
				      " between %s and %s\n",
				      dev->name, array[i]->name));
			xfree(comm);
			comm = priv->comm = pr->comm;
			comm->nDevs++;
			priv->next = comm->first;
			comm->first = priv;
			break;
		}
	}
	if (i >= max) {
		comm->devName = strdup(val->str);
		if (xf86Verbose)
			ErrorF("%s USBTtablet port is %s\n", XCONFIG_GIVEN,
			       comm->devName);
	}

	while ((token = xf86GetToken(USBTTab)) != ENDSUBSECTION) {
		switch(token) {
		case DEVICENAME:
			if (xf86GetToken(NULL) != STRING)
				xf86ConfigError("Option string expected");
			dev->name = strdup(val->str);
			if (xf86Verbose)
				ErrorF("%s USBTtablet X device name is %s\n", 
				       XCONFIG_GIVEN, dev->name);
			break;	    
	    
		case DEBUG_LEVEL:
			if (xf86GetToken(NULL) != NUMBER)
				xf86ConfigError("Option number expected");
			debug_level = val->num;
			if (xf86Verbose) {
#if DEBUG
				ErrorF("%s USBTtablet debug level sets to %d\n",
				       XCONFIG_GIVEN, debug_level);      
#else
				ErrorF("%s USBTtablet debug level not sets to %"
				       "d because debugging is not compiled\n",
				       XCONFIG_GIVEN, debug_level);      
#endif
			}
			break;

		case HISTORY_SIZE:
			if (xf86GetToken(NULL) != NUMBER)
				xf86ConfigError("Option number expected");
			dev->history_size = val->num;
			if (xf86Verbose)
				ErrorF("%s USBTtablet Motion history size is %d\n", 
				       XCONFIG_GIVEN, dev->history_size);      
			break;

		case ALWAYS_CORE:
			xf86AlwaysCore(dev, TRUE);
			if (xf86Verbose)
				ErrorF("%s USBTtablet device always stays core pointer\n",
				       XCONFIG_GIVEN);
			break;

		case THRESHOLD:
			if (xf86GetToken(NULL) != NUMBER)
				xf86ConfigError("Option number expected");
			priv->thresCent = val->num;
			if (xf86Verbose)
				ErrorF("%s USBTtablet threshold=%d\n",
				       XCONFIG_GIVEN, val->num);
			break;

		case SUPPRESS:
			if (xf86GetToken(NULL) != NUMBER)
				xf86ConfigError("Option number expected");
			priv->suppress = val->num;
			if (xf86Verbose)
				ErrorF("%s USBTtablet suppress=%d\n",
				       XCONFIG_GIVEN, val->num);
			break;

		case EOF:
			FatalError("Unexpected EOF (missing EndSubSection)");
			break;
	    
		default:
			xf86ConfigError("USBTtablet subsection keyword expected");
			break;
		}
	}
    
	DBG(1, ErrorF("xf86USBTConfig name=%s\n", comm->devName));
    
	return Success;
}

static int
xf86USBTProc(DeviceIntPtr       pUSBT,
	    int                what)
{
	LocalDevicePtr local = (LocalDevicePtr)pUSBT->public.devicePrivate;
	USBTDevicePtr   priv = (USBTDevicePtr)PRIVATE(pUSBT);
	CARD8          map[NBUTTONS+1];
	int            i;
  
	DBG(2, ErrorF("BEGIN xf86USBTProc pUSBT=0x%x what=%d\n", pUSBT, what));
  
	switch (what) {
	case DEVICE_INIT: 
		DBG(1, ErrorF("xf86USBTProc pUSBT=0x%x what=INIT\n", pUSBT));
      
		for(i = 1; i <= NBUTTONS; i++) 
			map[i] = i;

		if (InitButtonClassDeviceStruct(pUSBT,
						NBUTTONS,
						map) == FALSE) {
			ErrorF("unable to allocate Button class device\n");
			return !Success;
		}
      
		if (InitFocusClassDeviceStruct(pUSBT) == FALSE) {
			ErrorF("unable to init Focus class device\n");
			return !Success;
		}
          
		if (InitPtrFeedbackClassDeviceStruct(pUSBT,
					xf86USBTControlProc) == FALSE) {
			ErrorF("unable to init ptr feedback\n");
			return !Success;
		}
	    
		if (InitProximityClassDeviceStruct(pUSBT) == FALSE) {
			ErrorF("unable to init proximity class device\n");
			return !Success;
		}

		if (InitValuatorClassDeviceStruct(
			pUSBT, NAXES, xf86GetMotionEvents, 
			local->history_size,
			((priv->flags & ABSOLUTE_FLAG) ? Absolute : Relative) |
			 OutOfProximity) == FALSE) {
			ErrorF("unable to allocate Valuator class device\n"); 
			return !Success;
		} else {
			/* allocate the motion history buffer if needed */
			xf86MotionHistoryAllocate(local);

			AssignTypeAndName(pUSBT, local->atom, local->name);
		}

		/* open the device to gather informations */
		xf86USBTOpenDevice(pUSBT);
		break; 
      
	case DEVICE_ON:
		DBG(1, ErrorF("xf86USBTProc pUSBT=0x%x what=ON\n", pUSBT));

		if ((local->fd < 0) && (!xf86USBTOpenDevice(pUSBT))) {
			return !Success;
		}
		AddEnabledDevice(local->fd);
		pUSBT->public.on = TRUE;
		break;
      
	case DEVICE_OFF:
		DBG(1, ErrorF("xf86USBTProc  pUSBT=0x%x OFF\n", pUSBT));
		if (local->fd >= 0)
			RemoveEnabledDevice(local->fd);
		pUSBT->public.on = FALSE;
		break;
      
	case DEVICE_CLOSE:
		DBG(1, ErrorF("xf86USBTProc  pUSBT=0x%x CLOSE\n", pUSBT));
		xf86USBTClose(local);
		break;

	default:
		ErrorF("unsupported mode=%d\n", what);
		return !Success;
		break;
	}
	DBG(2, ErrorF("END   xf86USBTProc Success what=%d\n", what));
	return Success;
}

static void
xf86USBTReadInput(LocalDevicePtr local)
{
	USBTDevicePtr	priv = (USBTDevicePtr)local->private, pr;
	USBTCommonPtr	comm = priv->comm;
	int		is_button, is_proximity;
	int		invert, len;
	unsigned char	buffer[200], *p;
	USBTState	ds;
  
	DBG(7, ErrorF("xf86USBTReadInput BEGIN device=%s fd=%d\n",
		      comm->devName, local->fd));

	for(;;) {
		p = buffer;
		DBG(10, ErrorF("xf86USBTReadInput reading fd=%d len=%d\n",
			       local->fd, comm->reportSize));

		/*SYSCALL(len = read(local->fd, p, comm->reportSize));*/
		len = read(local->fd, p, comm->reportSize);

		DBG(8, ErrorF("xf86USBTReadInput len=%d\n", len));
	    
		if (len <= 0) {
			if (errno != EWOULDBLOCK)
				Error("error reading USBT device");
			break;
		}
	    
#ifndef USB_NEW_HID
		if (comm->reportId)
			p++;
#endif
		ds.x = hid_get_data(p, &comm->hidX);
		ds.y = hid_get_data(p, &comm->hidY);
		ds.buttons = hid_get_data(p, &comm->hidBarrel_Switch) << 2;
		invert = hid_get_data(p, &comm->hidInvert);
		ds.pressure = hid_get_data(p, &comm->hidTip_Pressure);
		if (ds.pressure > priv->threshold)
			ds.buttons |= 1;
		ds.proximity = hid_get_data(p, &comm->hidIn_Range);
		ds.xTilt = hid_get_data(p, &comm->hidTiltX);
		ds.yTilt = hid_get_data(p, &comm->hidTiltY);

		if (!ds.proximity)
			xf86USBTOutOfProx(comm->currentProxDev);

		for(pr = comm->first; pr; pr = pr->next) {
			DBG(7, ErrorF("xf86USBTReadInput sending to %s\n",
				      pr->local->name));

			xf86USBTSendEvents(pr->local, invert, &ds);
		}
	}
	DBG(7, ErrorF("xf86USBTReadInput END   local=0x%x priv=0x%x\n",
		      local, priv));
}

static void
xf86USBTOutOfProx(USBTDevicePtr prx)
{
	USBTState *ods;

	if (!prx)
		return;

	DBG(1, ErrorF("Out of proximity %s\n", prx->local->name));

	ods = &prx->state;
	prx->comm->currentProxDev = 0;

	if (prx->state.buttons) {
		/* Report buttons up when the device goes out of proximity. */
		DBG(9, ErrorF("xf86USBTOutOfProx: reset buttons\n"));
		xf86USBTSendButtons(prx->local, 0, 
				   ods->x, ods->y, ods->pressure, 
				   ods->xTilt, ods->yTilt);
		prx->state.buttons = 0;
	}
	if (!xf86IsCorePointer(prx->local->dev)) {
		DBG(1, ErrorF("xf86USBTSendEvents: out proximity\n"));
		xf86PostProximityEvent(prx->local->dev, 0, 0, 5, 
				       ods->x, ods->y, ods->pressure,
				       ods->xTilt, ods->yTilt);
	}
}

static void
xf86USBTIntoProx(USBTDevicePtr prx, USBTState *ds)
{
	if (prx->comm->currentProxDev == prx)
		return;
	xf86USBTOutOfProx(prx->comm->currentProxDev);
	prx->comm->currentProxDev = prx;

	DBG(1, ErrorF("Into proximity %s\n", prx->local->name));

	if (!xf86IsCorePointer(prx->local->dev)) {
		DBG(1, ErrorF("xf86USBTSendEvents: in proximity\n"));
		xf86PostProximityEvent(prx->local->dev, 1, 0, 5, 
				       ds->x, ds->y, ds->pressure,
				       ds->xTilt, ds->yTilt);
	}
}

static void
xf86USBTSendButtons(LocalDevicePtr	local,
		   int                  buttons,
		   int                  rx,
		   int                  ry,
		   int                  rz,
		   int                  rtx,
		   int                  rty)
		   
{
	USBTDevicePtr  priv = (USBTDevicePtr) local->private;
	int           button, mask;

	for (button = 1; button < NBUTTONS; button++) {
		mask = 1 << (button-1);
	
		if ((mask & priv->state.buttons) != (mask & buttons)) {
			DBG(4, ErrorF("xf86USBTSendButtons button=%d is %d\n", 
				      button, (buttons & mask) != 0));
			xf86PostButtonEvent(local->dev,
					    (priv->flags & ABSOLUTE_FLAG),
					    button, (buttons & mask) != 0,
					    0, 5, rx, ry, rz, rtx, rty);
		}
	}
}

static void
xf86USBTSendEvents(LocalDevicePtr local, int invert, USBTState *ds)
{
	USBTDevicePtr	priv = (USBTDevicePtr)local->private;
	USBTState	*ods = &priv->state;
	int		is_abs;
	int		rx, ry, rz, rtx, rty;

	DBG(9, ErrorF("xf86USBTSendEvents %s x=%d y=%d pressure=%d buttons=%x "
		      "xTilt=%d yTilt=%d proximity=%d, invert=%d, eraser=%d\n",
		      local->name,
		      ds->x, ds->y, ds->pressure, ds->buttons,
		      ds->xTilt, ds->yTilt, ds->proximity,
		      invert, (priv->flags & ERASER_ID) != 0));

	if (!ds->proximity)
		return;
	if (((priv->flags & ERASER_ID) != 0) != invert)
		return;

	xf86USBTIntoProx(priv, ds);

	if (ds->buttons == ods->buttons && ds->proximity == ods->proximity &&
	    ABS(ds->x - ods->x) < priv->suppress &&
	    ABS(ds->y - ods->y) < priv->suppress &&
	    ds->pressure == ods->pressure &&
	    ds->xTilt == ods->xTilt &&
	    ds->yTilt == ods->yTilt) {
		DBG(9, ErrorF("xf86USBTSendEvents no change\n"));
		return;
	}
	is_abs = 1;
	rx = ds->x; ry = ds->y; rz = ds->pressure; 
	rtx = ds->xTilt; rty = ds->yTilt;

	if (rx != ods->x || ry != ods->y || rz != ods->pressure ||
	    rtx != ods->xTilt || rty != ods->yTilt) {
		DBG(9, ErrorF("xf86USBTSendEvents: motion\n"));
		xf86PostMotionEvent(local->dev, is_abs, 0, 5, 
				    rx, ry, rz, rtx, rty); 
		
	}
	if (ds->buttons != ods->buttons)
		xf86USBTSendButtons(local, ds->buttons, 
				   rx, ry, rz, rtx, rty);

	*ods = *ds;
}

static int
xf86USBTChangeControl(LocalDevicePtr local,
		     xDeviceCtl	    *control)
{
	return (BadMatch);
}

static int
xf86USBTSwitchMode(ClientPtr	client,
		  DeviceIntPtr	dev,
		  int		mode)
{
	return BadMatch;
}

static void
xf86USBTClose(LocalDevicePtr	local)
{
	USBTDevicePtr	priv = (USBTDevicePtr)local->private;
	USBTCommonPtr	comm = priv->comm;
	int		num;
    
	for (num = 0, priv = comm->first; priv; priv = priv->next)
		if (priv->local->fd >= 0)
			num++;
	DBG(4, ErrorF("USBTtablet number of open devices = %d\n", num));
	
	if (num == 1) {		    
		SYSCALL(close(local->fd));
	}
	
	local->fd = -1;
}

static Bool
xf86USBTConvert(LocalDevicePtr	local,
	       int		first,
	       int		num,
	       int		v0,
	       int		v1,
	       int		v2,
	       int		v3,
	       int		v4,
	       int		v5,
	       int*		x,
	       int*		y)
{
	USBTCommonPtr comm = ((USBTDevicePtr)local->private)->comm;

	DBG(6, ErrorF("xf86USBTConvert\n"));

	if (first != 0 || num == 1)
		return FALSE;

	*x = v0 * comm->factorX;
	*y = v1 * comm->factorY;

	DBG(6, ErrorF("USBTtablet converted v0=%d v1=%d to x=%d y=%d\n",
		      v0, v1, *x, *y));

	return TRUE;
}

static Bool
xf86USBTReverseConvert(LocalDevicePtr	local,
		      int		x,
		      int		y,
		      int		*valuators)
{
	USBTCommonPtr comm = ((USBTDevicePtr)local->private)->comm;

	valuators[0] = x / comm->factorX;
	valuators[1] = y / comm->factorY;

	DBG(6, ErrorF("USBTtablet converted x=%d y=%d to v0=%d v1=%d\n", x, y,
		      valuators[0], valuators[1]));

	return TRUE;
}
 
static void
xf86USBTControlProc(DeviceIntPtr	device,
		   PtrCtrl	*ctrl)
{
	DBG(2, ErrorF("xf86USBTControlProc\n"));
}

static Bool
xf86USBTOpen(LocalDevicePtr local)
{
	USBTDevicePtr	priv = (USBTDevicePtr)local->private;
	USBTCommonPtr	comm = priv->comm;
	struct usb_ctl_report_desc rep;
	hid_data_t     d;
	hid_item_t     h;
	report_desc_t rd;
#ifdef USB_NEW_HID
	int            r;
#endif
    
	DBG(1, ErrorF("opening %s\n", comm->devName));

	SYSCALL(local->fd = open(comm->devName, O_RDWR|O_NDELAY, 0));
	if (local->fd == -1) {
		ErrorF("Error opening %s : %s\n", 
		       comm->devName, strerror(errno));
		return !Success;
	}

#ifdef USB_NEW_HID
	SYSCALL(r = ioctl(local->fd, USB_GET_REPORT_ID, &comm->reportId));
	if (r == -1) {
		ErrorF("Error ioctl USB_GET_REPORT_ID on %s : %s\n", 
		       comm->devName, strerror(errno));
		return !Success;
	}
#endif

	DBG(1, ErrorF("initializing tablet\n"));
    
	rd = hid_get_report_desc(local->fd);
	if (rd == 0) {
		Error(comm->devName);
		SYSCALL(close(local->fd));
		return !Success;
	}
    
	memset(&comm->hidX, 0, sizeof (hid_item_t));
	memset(&comm->hidY, 0, sizeof (hid_item_t));
	memset(&comm->hidTiltX, 0, sizeof (hid_item_t));
	memset(&comm->hidTiltY, 0, sizeof (hid_item_t));
	memset(&comm->hidIn_Range, 0, sizeof (hid_item_t));
	memset(&comm->hidInvert, 0, sizeof (hid_item_t));
	memset(&comm->hidTip_Pressure, 0, sizeof (hid_item_t));
	memset(&comm->hidBarrel_Switch, 0, sizeof (hid_item_t));
#ifdef USB_NEW_HID
	for (d = hid_start_parse(rd, 1<<hid_input, comm->reportId); 
#else
	for (d = hid_start_parse(rd, 1<<hid_input); 
#endif
	     hid_get_item(d, &h); ) {
		if (h.kind != hid_input || (h.flags & HIO_CONST))
			continue;
		if (h.usage == HID_USAGE2(HUP_GENERIC_DESKTOP, HUG_X))
			comm->hidX = h;
		if (h.usage == HID_USAGE2(HUP_GENERIC_DESKTOP, HUG_Y))
			comm->hidY = h;
		if (h.usage == HID_USAGE2(HUP_GENERIC_DESKTOP, HUD_X_TILT))
			comm->hidTiltX = h;
		if (h.usage == HID_USAGE2(HUP_GENERIC_DESKTOP, HUD_Y_TILT))
			comm->hidTiltY = h;
		if (h.usage == HID_USAGE2(HUP_DIGITIZERS, HUD_INVERT))
			comm->hidInvert = h;
		if (h.usage == HID_USAGE2(HUP_DIGITIZERS, HUD_IN_RANGE))
			comm->hidIn_Range = h;
		if (h.usage == HID_USAGE2(HUP_DIGITIZERS, HUD_TIP_PRESSURE))
			comm->hidTip_Pressure = h;
		if (h.usage == HID_USAGE2(HUP_DIGITIZERS, HUD_BARREL_SWITCH))
			comm->hidBarrel_Switch = h;
	}
	hid_end_parse(d);
#ifdef USB_NEW_HID
	comm->reportSize = hid_report_size(rd, hid_input, comm->reportId);
#else
	comm->reportSize = hid_report_size(rd, hid_input, &comm->reportId);
#endif
	hid_dispose_report_desc(rd);
	if (comm->hidX.report_size == 0 ||
	    comm->hidY.report_size == 0 ||
	    comm->hidIn_Range.report_size == 0) {
		ErrorF("%s has no X, Y, or In_Range report\n", 
		       comm->devName);
		return !Success;
	}
	DBG(2, ErrorF("Found X at %d, size=%d\n", 
		      comm->hidX.pos, comm->hidX.report_size));
	DBG(2, ErrorF("Found Y at %d, size=%d\n", 
		      comm->hidY.pos, comm->hidY.report_size));
	DBG(2, ErrorF("Found Invert at %d, size=%d\n", 
		      comm->hidInvert.pos, comm->hidInvert.report_size));
	DBG(2, ErrorF("Found In_Range at %d, size=%d\n", 
		      comm->hidIn_Range.pos, comm->hidIn_Range.report_size));
	DBG(2, ErrorF("Found Tip_Pressure at %d, size=%d\n", 
		      comm->hidTip_Pressure.pos, 
		      comm->hidTip_Pressure.report_size));
	DBG(2, ErrorF("Found Barrel_Switch at %d, size=%d\n", 
		      comm->hidBarrel_Switch.pos, 
		      comm->hidBarrel_Switch.report_size));
	DBG(2, ErrorF("Report size=%d, report id=%d\n",
		      comm->reportSize, comm->reportId));

	comm->factorX = ((double) screenInfo.screens[0]->width)
		/ (comm->hidX.logical_maximum - comm->hidX.logical_minimum);
	comm->factorY = ((double) screenInfo.screens[0]->height)
		/ (comm->hidY.logical_maximum - comm->hidY.logical_minimum);
    
	if (xf86Verbose) {
		ErrorF("%s USBT tablet X=%d..%d, Y=%d..%d", XCONFIG_PROBED,
		       comm->hidX.logical_minimum, comm->hidX.logical_maximum, comm->hidY.logical_minimum, comm->hidY.logical_maximum);
		if (comm->hidTip_Pressure.report_size != 0)
			ErrorF(", pressure=%d..%d",
			       comm->hidTip_Pressure.logical_minimum, comm->hidTip_Pressure.logical_maximum);
		ErrorF("\n");
	}
  
	return Success;
}

static int
xf86USBTOpenDevice(DeviceIntPtr pUSBT)
{
	LocalDevicePtr	local = (LocalDevicePtr)pUSBT->public.devicePrivate;
	USBTDevicePtr	priv = (USBTDevicePtr)PRIVATE(pUSBT), pr;
	USBTCommonPtr	comm = priv->comm;
	hid_item_t	*h = &comm->hidTip_Pressure;
    
	DBG(1, ErrorF("xf86USBTOpenDevice start\n"));
	if (local->fd < 0) {
		DBG(2, ErrorF("xf86USBTOpenDevice really open\n"));
		if (xf86USBTOpen(local) != Success) {
			if (local->fd >= 0) {
				SYSCALL(close(local->fd));
			}
			local->fd = -1;
			return 0;
		}
		/* report the file descriptor to all devices */
		for (pr = comm->first; pr; pr = pr->next)
			pr->local->fd = local->fd;
	}

	priv->threshold = 
	    h->logical_minimum + 
	    (h->logical_maximum - h->logical_minimum) * priv->thresCent / 100;
	if (xf86Verbose) {
		if (h->report_size != 0)
			ErrorF("%s USBT %s pressure threshold=%d, suppress=%d\n",
			       XCONFIG_PROBED, local->name, priv->threshold,
			       priv->suppress);
	}

	/* Set the real values */
	InitValuatorAxisStruct(pUSBT,
			       0,
			       comm->hidX.logical_minimum, /* min val */
			       comm->hidX.logical_maximum, /* max val */
			       mils(1000), /* resolution */
			       0, /* min_res */
			       mils(1000)); /* max_res */
	InitValuatorAxisStruct(pUSBT,
			       1,
			       comm->hidY.logical_minimum, /* min val */
			       comm->hidY.logical_maximum, /* max val */
			       mils(1000), /* resolution */
			       0, /* min_res */
			       mils(1000)); /* max_res */
	InitValuatorAxisStruct(pUSBT,
			       2,
			       h->logical_minimum, /* min val */
			       h->logical_maximum, /* max val */
			       mils(1000), /* resolution */
			       0, /* min_res */
			       mils(1000)); /* max_res */
	InitValuatorAxisStruct(pUSBT,
			       3,
			       comm->hidTiltX.logical_minimum, /* min val */
			       comm->hidTiltX.logical_maximum, /* max val */
			       mils(1000), /* resolution */
			       0, /* min_res */
			       mils(1000)); /* max_res */
	InitValuatorAxisStruct(pUSBT,
			       4,
			       comm->hidTiltY.logical_minimum, /* min val */
			       comm->hidTiltY.logical_maximum, /* max val */
			       mils(1000), /* resolution */
			       0, /* min_res */
			       mils(1000)); /* max_res */
	return (1);
}

static LocalDevicePtr
xf86USBTAllocate(char *name, int flag)
{
	LocalDevicePtr local = (LocalDevicePtr)xalloc(sizeof(LocalDeviceRec));
	USBTDevicePtr priv = (USBTDevicePtr)xalloc(sizeof(USBTDevice));
	USBTCommonPtr comm = (USBTCommonPtr)xalloc(sizeof(USBTCommon));

	DBG(1, ErrorF("xf86USBTAllocate start name=%s flag=%d\n",
		      name, flag));

	memset(local, 0, sizeof *local);
	memset(priv, 0, sizeof *priv);
	memset(comm, 0, sizeof *comm);

	local->name = name;
	local->device_config = xf86USBTConfig;
	local->device_control = xf86USBTProc;
	local->read_input = xf86USBTReadInput;
	local->control_proc = xf86USBTChangeControl;
	local->close_proc = xf86USBTClose;
	local->switch_mode = xf86USBTSwitchMode;
	local->conversion_proc = xf86USBTConvert;
	local->reverse_conversion_proc = xf86USBTReverseConvert;
	local->fd = -1;
	local->private = priv;
	local->old_x = -1;
	local->old_y = -1;

	priv->local = local;
	priv->comm = comm;
	priv->flags = ABSOLUTE_FLAG | flag;
	priv->suppress = 2;
	priv->thresCent = 5;

	comm->nDevs = 1;
	comm->first = priv;

	return (local);
}

static LocalDevicePtr
xf86USBTAllocateStylus()
{
	LocalDevicePtr local = xf86USBTAllocate(STYLUS_XI, STYLUS_ID);

	local->type_name = "USBT Stylus";
	return (local);
}

static LocalDevicePtr
xf86USBTAllocateEraser()
{
	LocalDevicePtr local = xf86USBTAllocate(ERASER_XI, ERASER_ID);

	local->type_name = "USBT Eraser";
	return (local);
}

DeviceAssocRec usb_stylus_assoc =
{
	STYLUS_SEC,		/* config_section_name */
	xf86USBTAllocateStylus	/* device_allocate */
};

DeviceAssocRec usb_eraser_assoc =
{
	ERASER_SEC,		/* config_section_name */
	xf86USBTAllocateEraser	/* device_allocate */
};

#ifdef DYNAMIC_MODULE
/*
 * Entry point of dynamic loading.
 */
int
init_module(unsigned long server_version)
{
	DBG(2, ErrorF("init_module USBT\n"));

	if (server_version != XF86_VERSION_CURRENT) {
		ErrorF("Warning: USBT module compiled for version%s\n", 
		       XF86_VERSION);
		return 0;
	}

	xf86AddDeviceAssoc(&usb_stylus_assoc);
	xf86AddDeviceAssoc(&usb_eraser_assoc);
	return (1);
}
#endif

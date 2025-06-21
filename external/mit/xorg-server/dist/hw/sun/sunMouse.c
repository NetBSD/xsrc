/* $Xorg: sunMouse.c,v 1.3 2000/08/17 19:48:32 cpqbld Exp $ */
/*-
 * Copyright 1987 by the Regents of the University of California
 *
 * Permission to use, copy, modify, and distribute this
 * software and its documentation for any purpose and without
 * fee is hereby granted, provided that the above copyright
 * notice appear in all copies.  The University of California
 * makes no representations about the suitability of this
 * software for any purpose.  It is provided "as is" without
 * express or implied warranty.
 */

/************************************************************
Copyright 1987 by Sun Microsystems, Inc. Mountain View, CA.

                    All Rights Reserved

Permission  to  use,  copy,  modify,  and  distribute   this
software  and  its documentation for any purpose and without
fee is hereby granted, provided that the above copyright no-
tice  appear  in all copies and that both that copyright no-
tice and this permission notice appear in  supporting  docu-
mentation,  and  that the names of Sun or The Open Group
not be used in advertising or publicity pertaining to
distribution  of  the software  without specific prior
written permission. Sun and The Open Group make no
representations about the suitability of this software for
any purpose. It is provided "as is" without any express or
implied warranty.

SUN DISCLAIMS ALL WARRANTIES WITH REGARD TO  THIS  SOFTWARE,
INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FIT-
NESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL SUN BE  LI-
ABLE  FOR  ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,  DATA  OR
PROFITS,  WHETHER  IN  AN  ACTION OF CONTRACT, NEGLIGENCE OR
OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION  WITH
THE USE OR PERFORMANCE OF THIS SOFTWARE.

********************************************************/
/*
 * Copyright 1991, 1992, 1993 Kaleb S. Keithley
 *
 * Permission to use, copy, modify, and distribute this
 * software and its documentation for any purpose and without
 * fee is hereby granted, provided that the above copyright
 * notice appear in all copies.  Kaleb S. Keithley makes no
 * representations about the suitability of this software for
 * any purpose.  It is provided "as is" without express or
 * implied warranty.
 */
/* $XFree86: xc/programs/Xserver/hw/sun/sunMouse.c,v 1.4 2003/11/17 22:20:36 dawes Exp $ */

#define NEED_EVENTS
#include    "sun.h"
#include    "mi.h"
#include    "cursor.h"
#include    "input.h"
#include    "inpututils.h"
#include    "exevents.h"
#include    "xserver-properties.h"

/*
 * Data private to any sun pointer device.
 */
typedef struct {
    int		fd;
    int		bmask;		/* last known button state */
    int		oformat;	/* saved value of VUIDGFORMAT */
    Firm_event	evbuf[SUN_MAXEVENTS];	/* Buffer for Firm_events */
} sunPtrPrivRec, *sunPtrPrivPtr;

Bool sunActiveZaphod = TRUE;
DeviceIntPtr sunPointerDevice = NULL;

static void sunMouseEvents(int, int, void *);
static int sunMouseGetEvents(DeviceIntPtr);
static void sunMouseEnqueueEvent(DeviceIntPtr, Firm_event *);
static Bool sunCursorOffScreen(ScreenPtr *, int *, int *);
static void sunCrossScreen(ScreenPtr, int);
static void sunWarpCursor(DeviceIntPtr, ScreenPtr, int, int);

miPointerScreenFuncRec sunPointerScreenFuncs = {
    sunCursorOffScreen,
    sunCrossScreen,
    sunWarpCursor,
};

static void
sunMouseEvents(int fd, int ready, void *data)
{
    int i, numEvents;
    DeviceIntPtr device = (DeviceIntPtr)data;
    DevicePtr pMouse = &device->public;
    sunPtrPrivPtr pPriv = pMouse->devicePrivate;

    input_lock();

    do {
	numEvents = sunMouseGetEvents(device);
	for (i = 0; i < numEvents; i++) {
	    sunMouseEnqueueEvent(device, &pPriv->evbuf[i]);
	}
    } while (numEvents == SUN_MAXEVENTS);

    input_unlock();
}

/*-
 *-----------------------------------------------------------------------
 * sunMouseCtrl --
 *	Alter the control parameters for the mouse. Since acceleration
 *	etc. is done from the PtrCtrl record in the mouse's device record,
 *	there's nothing to do here.
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	None.
 *
 *-----------------------------------------------------------------------
 */
/*ARGSUSED*/
static void
sunMouseCtrl(DeviceIntPtr device, PtrCtrl *ctrl)
{
}

/*-
 *-----------------------------------------------------------------------
 * sunMouseProc --
 *	Handle the initialization, etc. of a mouse
 *
 * Results:
 *	none.
 *
 * Side Effects:
 *
 * Note:
 *	When using sunwindows, all input comes off a single fd, stored in the
 *	global windowFd.  Therefore, only one device should be enabled and
 *	disabled, even though the application still sees both mouse and
 *	keyboard.  We have arbitrarily chosen to enable and disable windowFd
 *	in the keyboard routine sunKbdProc rather than in sunMouseProc.
 *
 *-----------------------------------------------------------------------
 */
int
sunMouseProc(DeviceIntPtr device, int what)
{
    DevicePtr	  pMouse = &device->public;
    sunPtrPrivPtr pPriv;
    int	    	  format;
    BYTE    	  map[4];
    Atom btn_labels[3] = {0};
    Atom axes_labels[2] = { 0, 0 };

    switch (what) {
	case DEVICE_INIT:
	    pPriv = malloc(sizeof(*pPriv));
	    if (pPriv == NULL) {
		LogMessage(X_ERROR, "Cannot allocate private data for mouse\n");
		return !Success;
	    }
	    pPriv->fd = open("/dev/mouse", O_RDWR | O_NONBLOCK, 0);
	    if (pPriv->fd < 0) {
		LogMessage(X_ERROR, "Cannot open /dev/mouse, error %d\n",
		    errno);
		free(pPriv);
		return !Success;
	    }
	    pPriv->bmask = 0;
	    pPriv->oformat = 0;
	    pMouse->devicePrivate = pPriv;
	    pMouse->on = FALSE;
	    
	    map[1] = 1;
	    map[2] = 2;
	    map[3] = 3;
	    btn_labels[0] = XIGetKnownProperty(BTN_LABEL_PROP_BTN_LEFT);
	    btn_labels[1] = XIGetKnownProperty(BTN_LABEL_PROP_BTN_MIDDLE);
	    btn_labels[2] = XIGetKnownProperty(BTN_LABEL_PROP_BTN_RIGHT);
	    axes_labels[0] = XIGetKnownProperty(AXIS_LABEL_PROP_REL_X);
	    axes_labels[1] = XIGetKnownProperty(AXIS_LABEL_PROP_REL_Y);

	    InitPointerDeviceStruct(pMouse, map, 3, btn_labels,
		sunMouseCtrl, GetMotionHistorySize(),
		2, axes_labels);

	    /* X valuator */
	    InitValuatorAxisStruct(device, 0, axes_labels[0],
		NO_AXIS_LIMITS, NO_AXIS_LIMITS, 1, 0, 1, Relative);
            device->valuator->axisVal[0] = screenInfo.screens[0]->width / 2;
            device->last.valuators[0] = device->valuator->axisVal[0];

	    /* Y valuator */
	    InitValuatorAxisStruct(device, 1, axes_labels[1],
		NO_AXIS_LIMITS, NO_AXIS_LIMITS, 1, 0, 1, Relative);
            device->valuator->axisVal[1] = screenInfo.screens[0]->height / 2;
            device->last.valuators[1] = device->valuator->axisVal[1];

	    break;

	case DEVICE_ON:
	    pPriv = (sunPtrPrivPtr)pMouse->devicePrivate;
	    if (ioctl(pPriv->fd, VUIDGFORMAT, &pPriv->oformat) == -1) {
		LogMessage(X_ERROR, "sunMouseProc ioctl VUIDGFORMAT\n");
		return !Success;
	    }
	    format = VUID_FIRM_EVENT;
	    if (ioctl(pPriv->fd, VUIDSFORMAT, &format) == -1) {
		LogMessage(X_ERROR, "sunMouseProc ioctl VUIDSFORMAT\n");
		return !Success;
	    }

	    SetNotifyFd(pPriv->fd, sunMouseEvents, X_NOTIFY_READ, device);

	    pPriv->bmask = 0;
	    pMouse->on = TRUE;
	    break;

	case DEVICE_OFF:
	    pPriv = (sunPtrPrivPtr)pMouse->devicePrivate;
	    RemoveNotifyFd(pPriv->fd);
	    if (ioctl(pPriv->fd, VUIDSFORMAT, &pPriv->oformat) == -1)
		LogMessage(X_ERROR, "sunMouseProc ioctl VUIDSFORMAT\n");
	    pMouse->on = FALSE;
	    break;

	case DEVICE_CLOSE:
	    pPriv = (sunPtrPrivPtr)pMouse->devicePrivate;
	    close(pPriv->fd);
	    free(pPriv);
	    pMouse->devicePrivate = NULL;
	    break;

	case DEVICE_ABORT:
	    break;
    }
    return Success;
}

/*-
 *-----------------------------------------------------------------------
 * sunMouseGetEvents --
 *	Return the events waiting in the wings for the given mouse.
 *
 * Results:
 *      Update Firm_event buffer in DeviceIntPtr if events are received.
 *      Return the number of received Firm_events in the buffer.
 *
 * Side Effects:
 *	None.
 *-----------------------------------------------------------------------
 */

static int
sunMouseGetEvents(DeviceIntPtr device)
{
    DevicePtr pMouse = &device->public;
    sunPtrPrivPtr pPriv = pMouse->devicePrivate;
    int nBytes;		    /* number of bytes of events available. */
    int NumEvents = 0;

    nBytes = read(pPriv->fd, pPriv->evbuf, sizeof(pPriv->evbuf));
    if (nBytes == -1) {
	if (errno != EWOULDBLOCK) {
	    LogMessage(X_ERROR, "Unexpected error on reading mouse\n");
	    FatalError("Could not read from mouse");
	}
    } else {
	if (pMouse->on) {
	    NumEvents = nBytes / sizeof(pPriv->evbuf[0]);
	}
    }
    return NumEvents;
}


/*-
 *-----------------------------------------------------------------------
 * sunMouseEnqueueEvent --
 *	Given a Firm_event for a mouse, pass it off the the dix layer
 *	properly converted...
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	The cursor may be redrawn...? devPrivate/x/y will be altered.
 *
 *-----------------------------------------------------------------------
 */

static void
sunMouseEnqueueEvent(DeviceIntPtr device, Firm_event *fe)
{
    sunPtrPrivPtr	pPriv;	/* Private data for pointer */
    int			bmask;	/* Temporary button mask */
    int			x, y;
    double		tmpx, tmpy;
    int			type, buttons, flag;
    int			valuators[2];
    ValuatorMask	mask;

    pPriv = (sunPtrPrivPtr)device->public.devicePrivate;

    switch (fe->id) {
    case MS_LEFT:
    case MS_MIDDLE:
    case MS_RIGHT:
	/*
	 * A button changed state. Sometimes we will get two events
	 * for a single state change. Should we get a button event which
	 * reflects the current state of affairs, that event is discarded.
	 *
	 * Mouse buttons start at 1.
	 */
	buttons = (fe->id - MS_LEFT) + 1;
	bmask = 1 << buttons;
	if (fe->value == VKEY_UP) {
	    if (pPriv->bmask & bmask) {
		type = ButtonRelease;
		pPriv->bmask &= ~bmask;
	    } else {
		return;
	    }
	} else {
	    if ((pPriv->bmask & bmask) == 0) {
		type = ButtonPress;
		pPriv->bmask |= bmask;
	    } else {
		return;
	    }
	}
	flag = POINTER_RELATIVE;
	valuator_mask_zero(&mask);
	QueuePointerEvents(device, type, buttons, flag, &mask);
	break;
    case LOC_X_DELTA:
	valuators[0] = fe->value;
	valuators[1] = 0;
	valuator_mask_set_range(&mask, 0, 2, valuators);
	flag = POINTER_RELATIVE | POINTER_ACCELERATE;
	QueuePointerEvents(device, MotionNotify, 0, flag, &mask);
	break;
    case LOC_Y_DELTA:
	/*
	 * For some reason, motion up generates a positive y delta
	 * and motion down a negative delta, so we must subtract
	 * here instead of add...
	 */
	valuators[0] = 0;
	valuators[1] = -fe->value;
	valuator_mask_set_range(&mask, 0, 2, valuators);
	flag = POINTER_RELATIVE | POINTER_ACCELERATE;
	QueuePointerEvents(device, MotionNotify, 0, flag, &mask);
	break;
    case LOC_X_ABSOLUTE:
	miPointerGetPosition(device, &x, &y);
	tmpx = fe->value;
	tmpy = y;
	miPointerSetPosition(device, Absolute, &tmpx, &tmpy, NULL, NULL);
	break;
    case LOC_Y_ABSOLUTE:
	miPointerGetPosition(device, &x, &y);
	tmpx = x;
	tmpy = fe->value;
	miPointerSetPosition(device, Absolute, &tmpx, &tmpy, NULL, NULL);
	break;
    default:
	FatalError ("sunMouseEnqueueEvent: unrecognized id\n");
	break;
    }
}

/*ARGSUSED*/
static Bool
sunCursorOffScreen(ScreenPtr *pScreen, int *x, int *y)
{
    int	    index, ret = FALSE;
    DeviceIntPtr device = sunPointerDevice;	/* XXX */

    if (device && PointerConfinedToScreen(device))
	return TRUE;
    /*
     * Active Zaphod implementation:
     *    increment or decrement the current screen
     *    if the x is to the right or the left of
     *    the current screen.
     */
    if (sunActiveZaphod &&
	screenInfo.numScreens > 1 && (*x >= (*pScreen)->width || *x < 0)) {
	index = (*pScreen)->myNum;
	if (*x < 0) {
	    index = (index ? index : screenInfo.numScreens) - 1;
	    *pScreen = screenInfo.screens[index];
	    *x += (*pScreen)->width;
	} else {
	    *x -= (*pScreen)->width;
	    index = (index + 1) % screenInfo.numScreens;
	    *pScreen = screenInfo.screens[index];
	}
	ret = TRUE;
    }
    return ret;
}

static void
sunCrossScreen(ScreenPtr pScreen, int entering)
{
    if (sunFbs[pScreen->myNum].EnterLeave)
	(*sunFbs[pScreen->myNum].EnterLeave) (pScreen, entering ? 0 : 1);
}

static void
sunWarpCursor(DeviceIntPtr pDev, ScreenPtr pScreen, int x, int y)
{
    input_lock();
    miPointerWarpCursor (pDev, pScreen, x, y);
    input_unlock();
}

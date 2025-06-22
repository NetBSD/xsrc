/* $NetBSD: x68kMouse.c,v 1.18 2025/06/22 22:33:03 tsutsui Exp $ */
/*-------------------------------------------------------------------------
 * Copyright (c) 1996 Yasushi Yamasaki
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
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *-----------------------------------------------------------------------*/

/*-
 * Copyright (c) 1987 by the Regents of the University of California
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
mentation,  and  that the names of Sun or X Consortium
not be used in advertising or publicity pertaining to
distribution  of  the software  without specific prior
written permission. Sun and X Consortium make no
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

#include "x68k.h"
#include "mi.h"
#include "input.h"
#include "inpututils.h"

#include "exevents.h"
#include "events.h"
#include "eventstr.h"
#include <X11/Xatom.h>
#include "xserver-properties.h"

static void x68kMouseEvents(int, int, void *);
static int x68kMouseGetEvents(DeviceIntPtr);
static void x68kMouseEnqueueEvent(DeviceIntPtr, Firm_event *);
static Bool x68kCursorOffScreen(ScreenPtr *, int *, int *);
static void x68kCrossScreen(ScreenPtr, int);
static void x68kWarpCursor(DeviceIntPtr, ScreenPtr, int, int);
static void x68kMouseCtrl(DeviceIntPtr, PtrCtrl*);

typedef struct _X68kMousePriv {
    int fd;
    int bmask;
    int oformat;
    MouseEmu3btn emu3btn;
    Firm_event evbuf[X68K_MAXEVENTS];
} X68kMousePriv, *X68kMousePrivPtr;

miPointerScreenFuncRec x68kPointerScreenFuncs = {
    x68kCursorOffScreen,
    x68kCrossScreen,
    x68kWarpCursor,
};

DeviceIntPtr x68kPointerDevice = NULL;

/*------------------------------------------------------------------------
 * x68kMouseEvents --
 *	When registered polled mouse input event handler is invoked,
 *	read device events and enqueue them using the mi event queue.
 * Results:
 *	None.
 *
 *----------------------------------------------------------------------*/
static void
x68kMouseEvents(int fd, int ready, void *data)
{
    int i, numEvents;
    DeviceIntPtr device = (DeviceIntPtr)data;
    DevicePtr pMouse = &device->public;
    X68kMousePrivPtr pPriv = pMouse->devicePrivate;

    input_lock();

    do {
	numEvents = x68kMouseGetEvents(device);
	for (i = 0; i < numEvents; i++) {
	    x68kMouseEnqueueEvent(device, &pPriv->evbuf[i]);
	}
    } while (numEvents == X68K_MAXEVENTS);

    input_unlock();
}

/*-
 *-----------------------------------------------------------------------
 * x68kMouseProc --
 *	Handle the initialization, etc. of a mouse
 *
 * Results:
 *	none.
 *
 * Side Effects:
 *-----------------------------------------------------------------------
 */
int
x68kMouseProc(DeviceIntPtr device, int what)
{
    DevicePtr   pMouse = &device->public;
    X68kMousePrivPtr pPriv;
    int		format;
    BYTE	map[4];
    Atom btn_labels[3] = {0};
    Atom axes_labels[2] = { 0, 0 };
    MouseEmu3btnPtr pEmu3btn;
    Bool emu3enable;
    int emu3timeout;

    switch (what) {
	case DEVICE_INIT:
            pPriv = malloc(sizeof(*pPriv));
            if (pPriv == NULL) {
                LogMessage(X_ERROR, "Cannot allocate private data for mouse\n");
                return !Success;
            }
            pPriv->fd = open("/dev/mouse", O_RDONLY | O_NONBLOCK);
            if (pPriv->fd == -1) {
                LogMessage(X_ERROR, "Can't open mouse device\n");
                return !Success;
            }
            pPriv->bmask = 0;
            pPriv->oformat = 0;
            memset(&pPriv->emu3btn, 0, sizeof(pPriv->emu3btn));
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
		x68kMouseCtrl, GetMotionHistorySize(),
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

	    /* Initialize emulation 3 buttons settings */
	    emu3enable = TRUE;			/* XXX should be configurable */
	    emu3timeout = EMU3B_DEF_TIMEOUT;	/* XXX should be configurable */
	    if (emu3enable) {
		pEmu3btn = &pPriv->emu3btn;
		Emulate3ButtonsEnable(pEmu3btn, device, emu3timeout);
	    }

	    break;

	case DEVICE_ON:
	    pPriv = (X68kMousePrivPtr)pMouse->devicePrivate;
	    if (ioctl(pPriv->fd, VUIDGFORMAT, &pPriv->oformat) == -1) {
		LogMessage(X_ERROR, "x68kMouseProc ioctl VUIDGFORMAT\n");
		return !Success;
	    }
	    format = VUID_FIRM_EVENT;
	    if (ioctl(pPriv->fd, VUIDSFORMAT, &format) == -1) {
		LogMessage(X_ERROR, "x68kMouseProc ioctl VUIDSFORMAT\n");
		return !Success;
	    }

	    SetNotifyFd(pPriv->fd, x68kMouseEvents, X_NOTIFY_READ, device);

	    pPriv->bmask = 0;
	    pMouse->on = TRUE;
	    break;

	case DEVICE_OFF:
	    pPriv = (X68kMousePrivPtr)pMouse->devicePrivate;
	    RemoveNotifyFd(pPriv->fd);
	    if (ioctl(pPriv->fd, VUIDSFORMAT, &pPriv->oformat) == -1)
		LogMessage(X_ERROR, "x68kMouseProc ioctl VUIDSFORMAT\n");
	    pMouse->on = FALSE;
	    break;

	case DEVICE_CLOSE:
	    pPriv = (X68kMousePrivPtr)pMouse->devicePrivate;
	    close(pPriv->fd);
	    free(pPriv);
	    break;

	case DEVICE_ABORT:
	    break;
    }
    return Success;
}

/*-
 *-----------------------------------------------------------------------
 * x68kMouseCtrl --
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
x68kMouseCtrl(DeviceIntPtr device, PtrCtrl* ctrl)
{
}

/*-
 *-----------------------------------------------------------------------
 * x68kMouseGetEvents --
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
x68kMouseGetEvents(DeviceIntPtr device)
{
    DevicePtr pMouse = &device->public;
    X68kMousePrivPtr pPriv = pMouse->devicePrivate;
    int nBytes;               /* number of bytes of events available. */
    int NumEvents = 0;

    nBytes = read(pPriv->fd, (char *)pPriv->evbuf, sizeof(pPriv->evbuf));
    if (nBytes == -1) {
	if (errno != EWOULDBLOCK) {
	    LogMessage(X_ERROR, "Unexpected error on reading mouse\n");
	    FatalError("Could not read from mouse");
	}
    } else {
	NumEvents = nBytes / sizeof(pPriv->evbuf[0]);
    }
    return NumEvents;
}

/*-
 *-----------------------------------------------------------------------
 * x68kMouseEnqueueEvent --
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
x68kMouseEnqueueEvent(DeviceIntPtr device, Firm_event *fe)
{
    X68kMousePrivPtr	pPriv;	/* Private data for pointer */
    int			bmask;	/* Temporary button mask */
    int			type, buttons, flag;
    int			valuators[2];
    ValuatorMask	mask;

    pPriv = (X68kMousePrivPtr)device->public.devicePrivate;

    switch (fe->id) {
    case MS_LEFT:
    case MS_MIDDLE:
    case MS_RIGHT:
	/*
	 * A button changed state. Sometimes we will get two events
	 * for a single state change. Should we get a button event which
	 * reflects the current state of affairs, that event is discarded.
	 *
	 * Mouse buttons start at 1 as defined in <X11/X.h>.
	 *
	 * The bmask stores which buttons are currently pressed.
	 * This bmask is also used for Emulate3Buttons functions that
	 * assume the left button is LSB as defined in mouseEmu3btn.c.
	 */
	buttons = (fe->id - MS_LEFT) + 1;
	bmask = 1 << (buttons - 1);
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
	if (buttons == Button1 || buttons == Button3) {
	    /* Handle middle button emulation */
	    Emulate3ButtonsQueueEvent(&pPriv->emu3btn, type, buttons, pPriv->bmask);
	} else {
	    flag = POINTER_RELATIVE;
	    valuator_mask_zero(&mask);
	    QueuePointerEvents(device, type, buttons, flag, &mask);
	}
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
    case LOC_Y_ABSOLUTE:
	/* XXX not sure how to get current X and Y position */
    default:
	FatalError ("%s: unrecognized id\n", __func__);
	break;
    }
}

/*ARGSUSED*/
static Bool
x68kCursorOffScreen(ScreenPtr *pScreen, int *x, int *y)
{
    return FALSE;
}

static void
x68kCrossScreen(ScreenPtr pScreen, int entering)
{
}

static void
x68kWarpCursor(DeviceIntPtr device, ScreenPtr pScreen, int x, int y)
{
    input_lock();
    miPointerWarpCursor(device, pScreen, x, y);
    input_unlock();
}

/* EOF x68kMouse.c */

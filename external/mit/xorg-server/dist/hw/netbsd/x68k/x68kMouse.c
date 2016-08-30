/* $NetBSD: x68kMouse.c,v 1.2 2016/08/30 07:50:55 mrg Exp $ */
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
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *      This product includes software developed by Yasushi Yamasaki
 * 4. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission
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

static Bool x68kCursorOffScreen(ScreenPtr *, int *, int *);
static void x68kCrossScreen(ScreenPtr, int);
static void x68kWarpCursor(DeviceIntPtr, ScreenPtr, int, int);
static void x68kMouseCtrl(DeviceIntPtr, PtrCtrl*);

miPointerScreenFuncRec x68kPointerScreenFuncs = {
    x68kCursorOffScreen,
    x68kCrossScreen,
    x68kWarpCursor,
};

DeviceIntPtr x68kPointerDevice = NULL;

static X68kMousePriv x68kMousePriv;

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
    int	    	format;
    static int	oformat;
    BYTE    	map[4];
    Atom btn_labels[3] = {0};
    Atom axes_labels[2] = { 0, 0 };

    switch (what) {
	case DEVICE_INIT:
            pMouse->devicePrivate = (pointer) &x68kMousePriv;
            if( (x68kMousePriv.fd = open("/dev/mouse", O_RDONLY)) == -1 ) {
                ErrorF("Can't open mouse device");
                return !Success;
            }
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
	    break;

	case DEVICE_ON:
	    if (ioctl (x68kMousePriv.fd, VUIDGFORMAT, &oformat) == -1) {
		ErrorF("x68kMouseProc ioctl VUIDGFORMAT");
		return !Success;
	    }
	    format = VUID_FIRM_EVENT;
	    if (ioctl (x68kMousePriv.fd, VUIDSFORMAT, &format) == -1) {
		ErrorF("x68kMouseProc ioctl VUIDSFORMAT");
		return !Success;
	    }
            if ( fcntl(x68kMousePriv.fd, F_SETOWN, getpid()) == -1 ||
                 fcntl(x68kMousePriv.fd, F_SETFL, O_NONBLOCK | O_ASYNC) == -1
                 ) {
                ErrorF("Async mouse I/O failed");
                return !Success;
            }
	    x68kMousePriv.bmask = 0;
	    AddEnabledDevice(x68kMousePriv.fd);
	    pMouse->on = TRUE;
	    break;

	case DEVICE_OFF:
	    pMouse->on = FALSE;
	    RemoveEnabledDevice(x68kMousePriv.fd);
	    break;

	case DEVICE_CLOSE:
	    if (ioctl (x68kMousePriv.fd, VUIDSFORMAT, &oformat) == -1)
		ErrorF("x68kMouseProc ioctl VUIDSFORMAT");
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
 *	A pointer to an array of Firm_events or (Firm_event *)0 if no events
 *	The number of events contained in the array.
 *	A boolean as to whether more events might be available.
 *
 * Side Effects:
 *	None.
 *-----------------------------------------------------------------------
 */

Firm_event *
x68kMouseGetEvents(int fd, int *pNumEvents, Bool *pAgain)
{
    int nBytes;               /* number of bytes of events available. */
    static Firm_event evBuf[X68K_MAXEVENTS];     /* Buffer for Firm_events */

    if ((nBytes = read (fd, (char *)evBuf, sizeof(evBuf))) == -1) {
	if (errno == EWOULDBLOCK) {
	    *pNumEvents = 0;
	    *pAgain = FALSE;
	} else {
	    ErrorF("x68kMouseGetEvents read");
	    FatalError ("Could not read from mouse");
	}
    } else {
	*pNumEvents = nBytes / sizeof (Firm_event);
	*pAgain = (nBytes == sizeof (evBuf));
    }
    return evBuf;
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

void
x68kMouseEnqueueEvent(DeviceIntPtr device, Firm_event *fe)
{
    X68kMousePrivPtr	pPriv;	/* Private data for pointer */
    int			bmask;	/* Temporary button mask */
    int			type, buttons, flag;
    int			i, nevents, valuators[2];
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
	valuator_mask_set_range(&mask, 0, 0, NULL);
	nevents = GetPointerEvents(x68kEvents, device,
	    type, buttons, flag, &mask);
	for (i = 0; i < nevents; i++)
	    mieqEnqueue(device, &x68kEvents[i]);
	break;
    case LOC_X_DELTA:
	valuators[0] = fe->value;
	valuators[1] = 0;
	valuator_mask_set_range(&mask, 0, 2, valuators);
        flag = POINTER_RELATIVE | POINTER_ACCELERATE;
	nevents = GetPointerEvents(x68kEvents, device,
	    MotionNotify, 0, flag, &mask);
	for (i = 0; i < nevents; i++)
	    mieqEnqueue(device, &x68kEvents[i]);
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
	nevents = GetPointerEvents(x68kEvents, device,
	    MotionNotify, 0, flag, &mask);
	for (i = 0; i < nevents; i++)
	    mieqEnqueue(device, &x68kEvents[i]);
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
x68kWarpCursor(DeviceIntPtr pDev, ScreenPtr pScreen, int x, int y)
{
    sigset_t newsigmask;

    (void) sigemptyset (&newsigmask);
    (void) sigaddset (&newsigmask, SIGIO);
    (void) sigprocmask (SIG_BLOCK, &newsigmask, (sigset_t *)NULL);
    miPointerWarpCursor (pDev, pScreen, x, y);
    (void) sigprocmask (SIG_UNBLOCK, &newsigmask, (sigset_t *)NULL);
}

/* EOF x68kMouse.c */

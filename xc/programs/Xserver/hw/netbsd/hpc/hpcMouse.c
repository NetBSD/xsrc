/* $NetBSD: hpcMouse.c,v 1.1 2000/05/06 06:01:49 takemura Exp $	*/
/* $XConsortium: sunMouse.c,v 5.21 94/04/17 20:29:47 kaleb Exp $ */
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

#define NEED_EVENTS
#include "hpc.h"
#include <stdio.h>

#if 0 /* XXX */
Bool hpcActiveZaphod = TRUE;
#endif /* 0 XXX */

static Bool hpcCursorOffScreen();
static void hpcCrossScreen();
static void hpcWarpCursor();

miPointerScreenFuncRec hpcPointerScreenFuncs = {
    hpcCursorOffScreen,
    hpcCrossScreen,
    hpcWarpCursor,
};

/*
 * hpcMouseCtrl --
 *	Alter the control parameters for the mouse. Since acceleration
 *	etc. is done from the PtrCtrl record in the mouse's device record,
 *	there's nothing to do here.
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	None.
 */
void
hpcMouseCtrl(device, ctrl)
    DeviceIntPtr    device;
    PtrCtrl*	    ctrl;
{
}

/*
 * hpcMouseProc --
 *	Handle the initialization, etc. of a mouse
 *
 * Results:
 *	none.
 *
 * Side Effects:
 */
int
hpcMouseProc(device, what)
    DeviceIntPtr  device;   	/* Mouse to play with */
    int	    	  what;	    	/* What to do with it */
{
    DevicePtr	  pMouse = (DevicePtr) device;
    int	    	  format;
    static int	  oformat;
    BYTE    	  map[4];
    char	  *dev;

    switch (what) {
	case DEVICE_INIT:
	    if (pMouse != LookupPointerDevice()) {
		ErrorF ("Cannot open non-system mouse");
		return !Success;
	    }
	    if (hpcPtrPriv.fd == -1)
		return !Success;
	    pMouse->devicePrivate = (pointer) &hpcPtrPriv;
	    pMouse->on = FALSE;
	    map[1] = 1;
	    map[2] = 2;
	    map[3] = 3;
	    InitPointerDeviceStruct(
		pMouse, map, 3, miPointerGetMotionEvents,
 		hpcMouseCtrl, miPointerGetMotionBufferSize());
	    break;

	case DEVICE_ON:
#if 0
	    if (ioctl (hpcPtrPriv.fd, VUIDGFORMAT, &oformat) == -1) {
		Error ("hpcMouseProc ioctl VUIDGFORMAT");
		return !Success;
	    }
	    format = VUID_FIRM_EVENT;
	    if (ioctl (hpcPtrPriv.fd, VUIDSFORMAT, &format) == -1) {
		Error ("hpcMouseProc ioctl VUIDSFORMAT");
		return !Success;
	    }
#endif
	    hpcPtrPriv.bmask = 0;
	    AddEnabledDevice (hpcPtrPriv.fd);
	    pMouse->on = TRUE;
	    break;

	case DEVICE_CLOSE:
#if 0
	    if (ioctl (hpcPtrPriv.fd, VUIDSFORMAT, &oformat) == -1)
		Error ("hpcMouseProc ioctl VUIDSFORMAT");
#endif
	    break;

	case DEVICE_OFF:
	    pMouse->on = FALSE;
	    RemoveEnabledDevice (hpcPtrPriv.fd);
	    break;
    }
    return Success;
}

/*
 * hpcMouseGetEvents --
 *	Return the events waiting in the wings for the given mouse.
 *
 * Results:
 *	A pointer to an array of hpcEvents or (hpcEvent *)0 if no events
 *	The number of events contained in the array.
 *	A boolean as to whether more events might be available.
 *
 * Side Effects:
 *	None.
 */
struct wscons_event*
hpcMouseGetEvents (pPriv, pNumEvents, pAgain)
    hpcPtrPrivPtr pPriv;
    int*	pNumEvents;
    Bool*	pAgain;
{
    int fd;
    int	nBytes;	    /* number of bytes of events available. */
    static hpcEvent evBuf[MAXEVENTS];   /* Buffer for hpcEvents */

    fd = pPriv->fd;
    if ((nBytes = read (fd, (char *)evBuf, sizeof(evBuf))) == -1) {
	if (errno == EWOULDBLOCK) {
	    *pNumEvents = 0;
	    *pAgain = FALSE;
	} else {
	    Error ("hpcMouseGetEvents read");
	    FatalError ("Could not read from mouse");
	}
    } else {
	*pNumEvents = nBytes / sizeof (hpcEvent);
	*pAgain = (nBytes == sizeof (evBuf));
    }
    return evBuf;
}


/*
 * MouseAccelerate --
 *	Given a delta and a mouse, return the acceleration of the delta.
 *
 * Results:
 *	The corrected delta
 *
 * Side Effects:
 *	None.
 */
static short
MouseAccelerate (device, delta)
    DeviceIntPtr  device;
    int	    	  delta;
{
    int  sgn = sign(delta);
    PtrCtrl *pCtrl;
    short ret;

    delta = abs(delta);
    pCtrl = &device->ptrfeed->ctrl;
    if (delta > pCtrl->threshold) {
	ret =
	    (short) sgn *
		(pCtrl->threshold + ((delta - pCtrl->threshold) * pCtrl->num) /
		    pCtrl->den);
    } else {
	ret = (short) sgn * delta;
    }
    return ret;
}

/*
 * hpcMouseEnqueueEvent --
 *	Given a hpcEvent for a mouse, pass it off the the dix layer
 *	properly converted...
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	The cursor may be redrawn...? devPrivate/x/y will be altered.
 */
void
hpcMouseEnqueueEvent (device, fe)
    DeviceIntPtr  device;   	/* Mouse from which the event came */
    hpcEvent	  *fe;	    	/* Event to process */
{
    xEvent		xE;
    hpcPtrPrivPtr	pPriv;	/* Private data for pointer */
    int			bmask;	/* Temporary button mask */
    unsigned long	time;
    int			x, y;

    pPriv = (hpcPtrPrivPtr)device->public.devicePrivate;

    time = xE.u.keyButtonPointer.time = TSTOMILLI(fe->time);

    switch (fe->type) {
    case WSCONS_EVENT_MOUSE_UP:
    case WSCONS_EVENT_MOUSE_DOWN:
	/*
	 * A button changed state. Sometimes we will get two events
	 * for a single state change. Should we get a button event which
	 * reflects the current state of affairs, that event is discarded.
	 *
	 * Mouse buttons start at 1.
	 */
	xE.u.u.detail = fe->value + 1;
	bmask = 1 << xE.u.u.detail;
	if (fe->type == WSCONS_EVENT_MOUSE_UP) {
	    if (pPriv->bmask & bmask) {
		xE.u.u.type = ButtonRelease;
		pPriv->bmask &= ~bmask;
	    } else {
		return;
	    }
	} else {
	    if ((pPriv->bmask & bmask) == 0) {
		xE.u.u.type = ButtonPress;
		pPriv->bmask |= bmask;
	    } else {
		return;
	    }
	}
	mieqEnqueue (&xE);
	break;
    case WSCONS_EVENT_MOUSE_DELTA_X:
	miPointerDeltaCursor (MouseAccelerate(device,fe->value),0,time);
	break;
    case WSCONS_EVENT_MOUSE_DELTA_Y:
	miPointerDeltaCursor (0,-MouseAccelerate(device,fe->value),time);
	break;
    case WSCONS_EVENT_MOUSE_DELTA_Z:
	/* Ignore for now. */
	break;
    case WSCONS_EVENT_MOUSE_ABSOLUTE_X:
	miPointerPosition (&x, &y);
	miPointerAbsoluteCursor (fe->value, y, time);
	break;
    case WSCONS_EVENT_MOUSE_ABSOLUTE_Y:
	miPointerPosition (&x, &y);
	miPointerAbsoluteCursor (x, fe->value, time);
	break;
    case WSCONS_EVENT_MOUSE_ABSOLUTE_Z:
	break;
    default:
	FatalError ("hpcMouseEnqueueEvent: unrecognized id\n");
	break;
    }
}

static Bool
hpcCursorOffScreen (pScreen, x, y)
    ScreenPtr	*pScreen;
    int		*x, *y;
{
    int	    index, ret = FALSE;
    extern Bool PointerConfinedToScreen();

    if (PointerConfinedToScreen()) return TRUE;
#if 0 /* XXX */
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
#endif /* 0 XXX */
    return ret;
}

static void
hpcCrossScreen(pScreen, entering)
    ScreenPtr	pScreen;
    Bool	entering;
{
    hpcFbPtr pFb = hpcGetScreenFb(pScreen);

    if (pFb->EnterLeave)
	(*pFb->EnterLeave) (pScreen, entering ? 0 : 1);
}

static void
hpcWarpCursor(pScreen, x, y)
    ScreenPtr	pScreen;
    int		x, y;
{
    sigset_t newsigmask;

    (void) sigemptyset (&newsigmask);
    (void) sigaddset (&newsigmask, SIGIO);
    (void) sigprocmask (SIG_BLOCK, &newsigmask, (sigset_t *)NULL);
    miPointerWarpCursor (pScreen, x, y);
    (void) sigprocmask (SIG_UNBLOCK, &newsigmask, (sigset_t *)NULL);
}

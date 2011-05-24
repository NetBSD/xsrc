/*	$NetBSD: decMouse.c,v 1.4 2011/05/24 21:23:52 jakllsch Exp $	*/

/* XConsortium: sunMouse.c,v 5.21 94/04/17 20:29:47 kaleb Exp */
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
#include    "mi.h"
#include    "dec.h"
#include <stdio.h>

/*-
 *-----------------------------------------------------------------------
 * decMouseCtrl --
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
static 
void decMouseCtrl (device, ctrl)
    DeviceIntPtr    device;
    PtrCtrl*	    ctrl;
{
}

/*-
 *-----------------------------------------------------------------------
 * decMouseProc --
 *	Handle the initialization, etc. of a mouse
 *
 * Results:
 *	none.
 *
 * Side Effects:
 *
 *-----------------------------------------------------------------------
 */
int decMouseProc (device, what)
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
	    if (decPtrPriv.fd == -1)
		return !Success;
	    pMouse->devicePrivate = (pointer) &decPtrPriv;
	    pMouse->on = FALSE;
	    map[1] = 1;
	    map[2] = 2;
	    map[3] = 3;
	    InitPointerDeviceStruct(
		pMouse, map, 3, miPointerGetMotionEvents,
 		decMouseCtrl, miPointerGetMotionBufferSize());
	    break;

	case DEVICE_ON:
#if 0		/* XXX -- deal with this rcd */
	    if (ioctl (decPtrPriv.fd, VUIDGFORMAT, &oformat) == -1) {
		Error ("decMouseProc ioctl VUIDGFORMAT");
		return !Success;
	    }
	    format = VUID_FIRM_EVENT;
	    if (ioctl (decPtrPriv.fd, VUIDSFORMAT, &format) == -1) {
		Error ("decMouseProc ioctl VUIDSFORMAT");
		return !Success;
	    }
#endif

#ifdef WSMOUSEIO_SETVERSION
	    {
		int version = WSMOUSE_EVENT_VERSION;
		if (ioctl(decPtrPriv.fd, WSMOUSEIO_SETVERSION, &version) == -1) {
		    Error ("decMouseProc ioctl WSMOUSEIO_SETVERSION");
		    return !Success;
		}
	    }
#endif

#if 0
	    decPtrPriv.bmask = 0;
#endif
	    AddEnabledDevice (decPtrPriv.fd);
	    pMouse->on = TRUE;
	    break;

	case DEVICE_CLOSE:
#if 0		/* XXX -- deal with this rcd */
	    if (ioctl (decPtrPriv.fd, VUIDSFORMAT, &oformat) == -1)
		Error ("decMouseProc ioctl VUIDSFORMAT");
#endif
	    break;

	case DEVICE_OFF:
	    pMouse->on = FALSE;
	    RemoveEnabledDevice (decPtrPriv.fd);
	    break;
    }
    return Success;
}
    
/*-
 *-----------------------------------------------------------------------
 * decMouseGetEvents --
 *	Return the events waiting in the wings for the given mouse.
 *
 * Results:
 *	A pointer to an array of wscons_events or NULL if no events
 *	The number of events contained in the array.
 *	A boolean as to whether more events might be available.
 *
 * Side Effects:
 *	None.
 *-----------------------------------------------------------------------
 */

struct wscons_event* decMouseGetEvents (fd, pNumEvents, pAgain)
    int		fd;
    int*	pNumEvents;
    Bool*	pAgain;
{
    int	    	  nBytes;	    /* number of bytes of events available. */
    static struct wscons_event	evBuf[MAXEVENTS];   /* Buffer for wscons_events */

    if ((nBytes = read (fd, (char *)evBuf, sizeof(evBuf))) == -1) {
	if (errno == EWOULDBLOCK) {
	    *pNumEvents = 0;
	    *pAgain = FALSE;
	} else {
	    Error ("decMouseGetEvents read");
	    FatalError ("Could not read from mouse");
	}
    } else {
	*pNumEvents = nBytes / sizeof (struct wscons_event);
	*pAgain = (nBytes == sizeof (evBuf));
    }
    return evBuf;
}


/*-
 *-----------------------------------------------------------------------
 * decMouseAccelerate --
 *	Given a delta and a mouse, return the acceleration of the delta.
 *
 * Results:
 *	The corrected delta
 *
 * Side Effects:
 *	None.
 *
 *-----------------------------------------------------------------------
 */
static short
decMouseAccelerate (device, delta)
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

/*-
 *-----------------------------------------------------------------------
 * decMouseEnqueueEvent --
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

void decMouseEnqueueEvent (device, fe)
    DeviceIntPtr  device;   	/* Mouse from which the event came */
    struct wscons_event *fe;
{
    xEvent		xE;
    decPtrPrivPtr	pPriv;	/* Private data for pointer */
    int			bmask;	/* Temporary button mask */
    unsigned long	time;
    int			x, y;

    pPriv = (decPtrPrivPtr)device->public.devicePrivate;

    time = xE.u.keyButtonPointer.time = TSTOMILLI(fe->time);

    switch (fe->type) {
    case WSCONS_EVENT_MOUSE_UP:
    case WSCONS_EVENT_MOUSE_DOWN:
	/*
	 * A button changed state. Sometimes we will get two events
	 * for a single state change. Should we get a button event which
	 * reflects the current state of affairs, that event is discarded.
	 *
	 * Mouse buttons start with 0.
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
	miPointerDeltaCursor (decMouseAccelerate(device,fe->value),0,time);
	break;
    case WSCONS_EVENT_MOUSE_DELTA_Y:
	/*
	 * For some reason, motion up generates a positive y delta
	 * and motion down a negative delta, so we must subtract
	 * here instead of add...
	 */
	miPointerDeltaCursor (0,-decMouseAccelerate(device,fe->value),time);
	break;
    case WSCONS_EVENT_MOUSE_ABSOLUTE_X:
	miPointerPosition (&x, &y);
	miPointerAbsoluteCursor (fe->value, y, time);
	break;
    case WSCONS_EVENT_MOUSE_ABSOLUTE_Y:
	miPointerPosition (&x, &y);
	miPointerAbsoluteCursor (x, fe->value, time);
	break;
    default:
	FatalError ("decMouseEnqueueEvent: unrecognized id\n");
	break;
    }
}

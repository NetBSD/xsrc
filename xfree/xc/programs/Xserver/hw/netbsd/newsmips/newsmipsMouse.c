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

#define	NEED_EVENTS
#include "newsmips.h"
#include "mi.h"
#include <stdio.h>

static Bool newsmipsCursorOffScreen();
static void newsmipsCrossScreen();
static void newsmipsWarpCursor();

miPointerScreenFuncRec newsmipsPointerScreenFuncs = {
	newsmipsCursorOffScreen,
	newsmipsCrossScreen,
	newsmipsWarpCursor,
};

/*
 * newsmipsMouseCtrl --
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
newsmipsMouseCtrl(DeviceIntPtr device, PtrCtrl *ctrl)
{

}

/*
 * newsmipsMouseProc --
 *	Handle the initialization, etc. of a mouse
 *
 * Results:
 *	none.
 *
 * Side Effects:
 */
int
newsmipsMouseProc(DeviceIntPtr device, int what)
{
	DevicePtr pMouse = (DevicePtr) device;
	int format;
	static int oformat;
	BYTE map[7];
	char *dev;

	switch (what) {
	case DEVICE_INIT:
		if (pMouse != LookupPointerDevice()) {
			newsmipsErrorF (("Cannot open non-system mouse"));
			return !Success;
		}
		if (newsmipsPtrPriv.fd == -1)
			return !Success;
		pMouse->devicePrivate = (pointer) &newsmipsPtrPriv;
		pMouse->on = FALSE;
		map[1] = 1;
		map[2] = 2;
		map[3] = 3;
		map[4] = 2;
		map[5] = 4;
		map[6] = 5;
		InitPointerDeviceStruct(pMouse, map, 6,
		    miPointerGetMotionEvents, newsmipsMouseCtrl,
		    miPointerGetMotionBufferSize());
		break;

	case DEVICE_ON:
#ifdef WSMOUSEIO_SETVERSION
		{
			int version = WSMOUSE_EVENT_VERSION;
			if (ioctl(newsmipsPtrPriv.fd, WSMOUSEIO_SETVERSION, &version) == -1) {
				Error ("newsmipsMouseProc ioctl WSMOUSEIO_SETVERSION");
				return !Success;
			}
		}
#endif

		newsmipsPtrPriv.bmask = 0;

		AddEnabledDevice(newsmipsPtrPriv.fd);
		pMouse->on = TRUE;
		break;

	case DEVICE_CLOSE:
		break;

	case DEVICE_OFF:
		pMouse->on = FALSE;
		RemoveEnabledDevice(newsmipsPtrPriv.fd);
		break;
	}
	return Success;
}

/*
 * newsmipsMouseGetEvents --
 *	Return the events waiting in the wings for the given mouse.
 *
 * Results:
 *	A pointer to an array of newsmipsEvents or
 *	(newsmipsEvent *)0 if no events The number of events
 *	contained in the array.
 *
 *	A boolean as to whether more events might be available.
 *
 * Side Effects:
 *	None.
 */
struct wscons_event*
newsmipsMouseGetEvents(newsmipsPtrPrivPtr pPriv, int *pNumEvents,
    Bool *pAgain)
{
	static newsmipsEvent evBuf[MAXEVENTS];
	int fd, n;

	fd = pPriv->fd;
	if ((n = read(fd, (char *)evBuf, sizeof(evBuf))) == -1) {
		if (errno == EWOULDBLOCK) {
			*pNumEvents = 0;
			*pAgain = FALSE;
		} else {
			newsmipsError("newsmipsMouseGetEvents read");
			newsmipsFatalError(("Could not read from mouse"));
		}
	} else {
		*pNumEvents = n / sizeof(newsmipsEvent);
		*pAgain = n == sizeof(evBuf);
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
MouseAccelerate(DeviceIntPtr device, int delta)
{
	int  sgn = sign(delta);
	PtrCtrl *pCtrl;
	short ret;

	delta = abs(delta);
	pCtrl = &device->ptrfeed->ctrl;
	if (delta > pCtrl->threshold) {
		ret = (short) sgn * (pCtrl->threshold + ((delta - pCtrl->threshold) *
		    pCtrl->num) / pCtrl->den);
	} else {
		ret = (short) sgn * delta;
	}

	return ret;
}

/*
 * newsmipsMouseEnqueueEvent --
 *	Given a newsmipsEvent for a mouse, pass it off the the dix layer
 *	properly converted...
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	The cursor may be redrawn...? devPrivate/x/y will be altered.
 */
void
newsmipsMouseEnqueueEvent(DeviceIntPtr  device, newsmipsEvent *fe)	    	/* Event to process */
{
	xEvent		xE;
	newsmipsPtrPrivPtr	pPriv;	/* Private data for pointer */
	newsmipsKbdPrivPtr	keyPriv;/* Private keyboard data for
                                           button emul */
	unsigned long	time;
	int			x, y, bmask;

	pPriv = (newsmipsPtrPrivPtr)device->public.devicePrivate;

	time = xE.u.keyButtonPointer.time = TSTOMILLI(fe->time);

	/*
	 * Mouse buttons start at 1.
	 *
	 * Sometimes we will get two events for a single button state change.
	 * Should we get a button even which reflects the current state of
	 * affairs, that event is discarded.  In the button emulation case, we
	 * may need to generate several events from one real event.
	 *
	 * The button emulation allows WIN-1 through WIN-5 to be used as
	 * buttons one to five when the first real button is down.  If other
	 * real buttons are present, they are accounted for separately so that
	 * lifting an emulated button will not cause a button up event if the
	 * real button is down.  If the WIN button is not down, a button down
	 * event will be sent for the first button.  If the first button is
	 * pressed when just the WIN key is down, the button events will not
	 * be sent.  The allows you to move just the cursor on a touch screen.
	 * Emulated buttons are only released when the keys are released.
	 */

	switch (fe->type) {
	case WSCONS_EVENT_MOUSE_UP:
	case WSCONS_EVENT_MOUSE_DOWN:
		/*
		 * A button changed state. Sometimes we will get two
		 * events for a single state change. Should we get a
		 * button event which reflects the current state of
		 * affairs, that event is discarded.
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
		miPointerDeltaCursor(MouseAccelerate(device,fe->value),0,time);
		break;
	case WSCONS_EVENT_MOUSE_DELTA_Y:
		miPointerDeltaCursor(0,-MouseAccelerate(device,fe->value),time);
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
		newsmipsFatalError(("newsmipsMouseEnqueueEvent: "
		    "unrecognized id\n"));
		break;
	}
}

static Bool
newsmipsCursorOffScreen(ScreenPtr *pScreen, int *x, int *y)
{
	extern Bool PointerConfinedToScreen();

	if (PointerConfinedToScreen())
		return TRUE;

	return FALSE;
}

static void
newsmipsCrossScreen(ScreenPtr pScreen, Bool entering)
{
	newsmipsFbPtr pFb = newsmipsGetScreenFb(pScreen);

	if (pFb->EnterLeave)
		(*pFb->EnterLeave)(pScreen, entering ? 0 : 1);
}

static void
newsmipsWarpCursor(ScreenPtr pScreen, int x, int y)
{
	sigset_t newsigmask;

	(void) sigemptyset (&newsigmask);
	(void) sigaddset (&newsigmask, SIGIO);
	(void) sigprocmask (SIG_BLOCK, &newsigmask, (sigset_t *)NULL);
	miPointerWarpCursor (pScreen, x, y);
	(void) sigprocmask (SIG_UNBLOCK, &newsigmask, (sigset_t *)NULL);
}

/* $XConsortium: amigaMouse.c,v 5.21 94/04/17 20:29:47 kaleb Exp $ */
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
NESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL AMIGA BE  LI-
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

#include    <termios.h>

#define NEED_EVENTS
#include    "amiga.h"
#include    "mi.h"

Bool amigaActiveZaphod = TRUE;

static Bool amigaCursorOffScreen();
static void amigaCrossScreen();

miPointerScreenFuncRec amigaPointerScreenFuncs = {
    amigaCursorOffScreen,
    amigaCrossScreen,
    miPointerWarpCursor,
};

/*-
 *-----------------------------------------------------------------------
 * amigaMouseCtrl --
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
#if NeedFunctionPrototypes
void amigaMouseCtrl (
    DeviceIntPtr    device,
    PtrCtrl*	    ctrl)
#else
void amigaMouseCtrl (device, ctrl)
    DeviceIntPtr    device;
    PtrCtrl*	    ctrl;
#endif
{
}


 extern Bool amigaEmulateMiddleButton;
 extern Bool amigaEmulateRightButton;


/*-
 *-----------------------------------------------------------------------
 * amigaMouseProc --
 *	Handle the initialization, etc. of a mouse
 *
 * Results:
 *	none.
 *
 * Side Effects:
 *
 *-----------------------------------------------------------------------
 */
#if NeedFunctionPrototypes
int amigaMouseProc (
    DeviceIntPtr  device,
    int	    	  what)
#else
int amigaMouseProc (device, what)
    DeviceIntPtr  device;   	/* Mouse to play with */
    int	    	  what;	    	/* What to do with it */
#endif
{
    DevicePtr	  pMouse = (DevicePtr) device;
    int	    	  format;
    static int	  oformat;
    BYTE    	  map[4];
    char	  *dev;
    struct termios tio;

    switch (what) {
	case DEVICE_INIT:
	    if (pMouse != LookupPointerDevice()) {
		ErrorF ("Cannot open non-system mouse");	
		return !Success;
	    }
	    if (amigaPtrPriv.fd == -1)
		return !Success;
	    pMouse->devicePrivate = (pointer) &amigaPtrPriv;
	    pMouse->on = FALSE;
	    map[1] = 1;
	    map[2] = 2;
	    map[3] = 3;
	    InitPointerDeviceStruct(
		pMouse, map, 3, miPointerGetMotionEvents,
 		amigaMouseCtrl, miPointerGetMotionBufferSize());
	    break;

	case DEVICE_ON:
	    if (ioctl (amigaPtrPriv.fd, VUIDGFORMAT, &oformat) == -1) {

		if (tcgetattr(amigaPtrPriv.fd, &tio) == -1) {
			Error("amigaMouseProc: serial mouse getting termios");
			return !Success;
		}

		cfsetspeed(&tio, 1200);
		cfmakeraw(&tio);
		tio.c_cflag = CREAD|CS7|CLOCAL;

		if (tcsetattr(amigaPtrPriv.fd, TCSANOW, &tio) == -1) {
			Error("amigaMouseProc: serial mouse setting termios"); 
			return !Success;
		}

		/*(void)write(amigaPtrPriv.fd, "*X", 2);*/

		amigaPtrPriv.mousetype = 0;
	        AddEnabledDevice (amigaPtrPriv.fd);
	        pMouse->on = TRUE;
		return Success;
	    }

	    format = VUID_FIRM_EVENT;
	    if (ioctl (amigaPtrPriv.fd, VUIDSFORMAT, &format) == -1) {
		Error ("amigaMouseProc ioctl VUIDSFORMAT");
		return !Success;
	    }
	    amigaPtrPriv.bmask = 0;
	    amigaPtrPriv.mousetype = -1;
	    AddEnabledDevice (amigaPtrPriv.fd);
	    pMouse->on = TRUE;
	    break;

	case DEVICE_CLOSE:
	    pMouse->on = FALSE;
	    if (amigaPtrPriv.mousetype == -1)
	    	if (ioctl (amigaPtrPriv.fd, VUIDSFORMAT, &oformat) == -1)
			Error ("amigaMouseProc ioctl VUIDSFORMAT");
	    break;

	case DEVICE_OFF:
	    pMouse->on = FALSE;
	    RemoveEnabledDevice (amigaPtrPriv.fd);
	    break;
    }
    return Success;
}
    
/*-
 *-----------------------------------------------------------------------
 * amigaMouseGetEvents --
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

#if NeedFunctionPrototypes
Firm_event* amigaMouseGetEvents (
    int		fd,
    Bool	on,
    int*	pNumEvents,
    Bool*	pAgain)
#else
Firm_event* amigaMouseGetEvents (fd, on, pNumEvents, pAgain)
    int		fd;
    Bool	on;
    int*	pNumEvents;
    Bool*	pAgain;
#endif
{
    int	    	  nBytes;	    /* number of bytes of events available. */
    static Firm_event	evBuf[MAXEVENTS];   /* Buffer for Firm_events */

    if ((nBytes = read (fd, (char *)evBuf, sizeof(evBuf))) == -1) {
	if (errno == EWOULDBLOCK) {
	    *pNumEvents = 0;
	    *pAgain = FALSE;
	} else {
	    Error ("amigaMouseGetEvents read");
	    FatalError ("Could not read from mouse");
	}
    } else {
	if (on) {
	    *pNumEvents = nBytes / sizeof (Firm_event);
	    *pAgain = (nBytes == sizeof (evBuf));
	} else {
	   *pNumEvents = 0;
	   *pAgain = FALSE;
	}
    }
    return evBuf;
}

/*-
 *-----------------------------------------------------------------------
 * amigaSerGetEvents --
 *	Return the events waiting in the wings for the given mouse.
 *
 * Results:
 *	A pointer to an array of Firm_events or (Firm_event *)0 if no events
 *	The number of events contained in the array.
 *	A boolean as to whether more events might be available.
 *
 * Side Effects:
 *	Static State Updated XXXXXX should be in ptrPriv.
 *-----------------------------------------------------------------------
 */

#if NeedFunctionPrototypes
Firm_event* amigaSerGetEvents (
    int		fd,
    Bool	on,
    int*	pNumEvents,
    Bool*	pAgain)
#else
Firm_event* amigaSerGetEvents (fd, on, pNumEvents, pAgain)
    int		fd;
    Bool	on;
    int*	pNumEvents;
    Bool*	pAgain;
#endif
{
    static Firm_event	evBuf[MAXEVENTS]; /* Buffer for Firm_events */
    int	    	  	nBytes;	/* number of bytes of raw events available. */
    char	rawbuf[3*MAXEVENTS/4];	/* 3 raw bytes can generate 4 events */
    static int		byteno = 0;

    int 		i, evnum = 0;
    struct timeval	now;

    if ((nBytes = read (fd, rawbuf, sizeof(rawbuf))) == -1) {
	if (errno == EWOULDBLOCK) {
	    *pNumEvents = 0;
	    *pAgain = FALSE;
	} else {
	    Error ("amigaSerGetEvents read");
	    FatalError ("Could not read from mouse");
	}
    } else {
	if (on) {
	    *pAgain = (nBytes == sizeof(rawbuf));
    	    (void)gettimeofday(&now, (void *)0);
	    for (i=0; i < nBytes; i++) {
		if ((rawbuf[i] & 0x40) == 0x40) {
			byteno = 0;
		}
		if ((byteno == 0) && ((rawbuf[i] & 0x40) != 0x40)) {
			if ((rawbuf[i] & ~0x23) != 0)
				continue;

			evBuf[evnum].id = MS_MIDDLE;
			evBuf[evnum].value = rawbuf[i] & 0x20 ?
			    VKEY_DOWN : VKEY_UP;
			/* firm_event still uses 32 bit time */
			evBuf[evnum].time.tv_sec = now.tv_sec;
			evBuf[evnum].time.tv_usec = now.tv_usec;
			evnum++;
			continue;
		}
		switch(byteno) {
		case 0:
			amigaPtrPriv.fb = rawbuf[i];
			byteno = 1;
			break;
		case 1:
			amigaPtrPriv.dx = (char)
			    (((amigaPtrPriv.fb & 0x3) << 6) | rawbuf[i]);
			byteno = 2;
			break;
		case 2:
			amigaPtrPriv.dy = (char)
			    (((amigaPtrPriv.fb & 0xc) << 4) | rawbuf[i]);

			if (amigaPtrPriv.dx) {
				evBuf[evnum].id = LOC_X_DELTA;
				evBuf[evnum].value = amigaPtrPriv.dx;
				/* firm_event still uses 32 bit time */
				evBuf[evnum].time.tv_sec = now.tv_sec;
				evBuf[evnum].time.tv_usec = now.tv_usec;
				evnum++;
			}

			if (amigaPtrPriv.dy) {
				evBuf[evnum].id = LOC_Y_DELTA;
				evBuf[evnum].value = amigaPtrPriv.dy;
				/* firm_event still uses 32 bit time */
				evBuf[evnum].time.tv_sec = now.tv_sec;
				evBuf[evnum].time.tv_usec = now.tv_usec;
				evnum++;
			}

			amigaPtrPriv.fb ^= amigaPtrPriv.bmask;

			if (amigaPtrPriv.fb & 0x10) {
				amigaPtrPriv.bmask ^= 0x10;
				evBuf[evnum].id = MS_RIGHT;
				evBuf[evnum].value = amigaPtrPriv.bmask & 0x10
				    ? VKEY_DOWN: VKEY_UP;
				/* firm_event still uses 32 bit time */
				evBuf[evnum].time.tv_sec = now.tv_sec;
				evBuf[evnum].time.tv_usec = now.tv_usec;
				evnum++;
			}

			if (amigaPtrPriv.fb & 0x20) {
				amigaPtrPriv.bmask ^= 0x20;
				evBuf[evnum].id = MS_LEFT;
				evBuf[evnum].value = amigaPtrPriv.bmask & 0x20
				    ? VKEY_DOWN: VKEY_UP;
				/* firm_event still uses 32 bit time */
				evBuf[evnum].time.tv_sec = now.tv_sec;
				evBuf[evnum].time.tv_usec = now.tv_usec;
				evnum++;
			}
			byteno = 0;

		} /* switch */
	    } /* for */
	    *pNumEvents = evnum;
	} else {
	   *pNumEvents = 0;
	   *pAgain = FALSE;
	}
    }
    return evBuf;
}


/*-
 *-----------------------------------------------------------------------
 * MouseAccelerate --
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

/*-
 *-----------------------------------------------------------------------
 * amigaMouseEnqueueEvent --
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

#if NeedFunctionPrototypes
void amigaMouseEnqueueEvent (
    DeviceIntPtr  device,
    Firm_event	  *fe,
    Firm_event    *fe_next)
#else
void amigaMouseEnqueueEvent (device, fe, fe_next)
    DeviceIntPtr  device;   	/* Mouse from which the event came */
    Firm_event	  *fe;	    	/* Event to process */
    Firm_event    *fe_next;     /* Next Event for emulating middle or right */
#endif
{
    xEvent		xE;
    amigaPtrPrivPtr	pPriv;	/* Private data for pointer */
    int			bmask;	/* Temporary button mask */
    unsigned long	time;
    int			x, y;
    time_t		time64;	/* firm_event still uses 32 bit time */

    pPriv = (amigaPtrPrivPtr)device->public.devicePrivate;

    time = xE.u.keyButtonPointer.time = GetTimeInMillis();

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
#define MS_MIDDLE_BUTTON_MASK 0x4
#define MS_RIGHT_BUTTON_MASK 0x8
#define MS_ALL_BUTTONS_MASK 0xe

	xE.u.u.detail = (fe->id - MS_LEFT) + 1;
	/* If we're emulating the right, the real right button becomes middle */
	if (amigaEmulateRightButton && fe->id == MS_RIGHT)
		 xE.u.u.detail = 2;

	bmask = 1 << xE.u.u.detail;
	if (fe->value == VKEY_UP) {
		xE.u.u.type = ButtonRelease;

		if (amigaEmulateMiddleButton &&
			(pPriv->bmask & MS_MIDDLE_BUTTON_MASK)) {
	                /* Any button release when emulating the middle mouse button
        	         * causes a fake middle mouse button release if the middle
                	 * button was "pressed".
                 	*/
			xE.u.u.detail = 2;
			pPriv->bmask &= ~MS_MIDDLE_BUTTON_MASK;
		} else if (amigaEmulateRightButton &&
			   (pPriv->bmask & MS_RIGHT_BUTTON_MASK)) {
                	xE.u.u.detail = 3;
			pPriv->bmask &= ~MS_RIGHT_BUTTON_MASK;
		} else  if (pPriv->bmask & bmask) {
			pPriv->bmask &= ~bmask;
	    	} else {
			return;
	    }
	} else {
		xE.u.u.type = ButtonPress;
            	/* Ignore next event if not a mouse button press event */
            	if (fe_next && fe_next->id != MS_LEFT &&
                	fe_next->id != MS_MIDDLE && fe_next->id != MS_RIGHT)
                	fe_next = NULL;
                
            	if (amigaEmulateMiddleButton && fe_next &&
                	fe_next->value == VKEY_DOWN && fe_next->id != fe->id &&
                	(pPriv->bmask & MS_ALL_BUTTONS_MASK) == 0) {
                	/* A mouse button down with another mouse button down to
                 	 * follow causes a fake button 2 down event but only if no
                 	 * other mouse buttons were already pressed i.e.
                 	 * the user pressed the left button followed by the right
                 	 * rather than both together.
                 	 */
                	pPriv->bmask |= MS_MIDDLE_BUTTON_MASK;
                	xE.u.u.detail = 2;
                } else if (amigaEmulateRightButton && fe_next &&
                		fe_next->value == VKEY_DOWN && fe_next->id != fe->id &&
                		(pPriv->bmask & MS_ALL_BUTTONS_MASK) == 0) {
                	pPriv->bmask |= MS_RIGHT_BUTTON_MASK;
                	xE.u.u.detail = 3;
            	} else if ((pPriv->bmask & bmask) == 0 &&
                		!(amigaEmulateMiddleButton &&
                  		(pPriv->bmask & MS_MIDDLE_BUTTON_MASK)) &&
                		!(amigaEmulateRightButton &&
                  		(pPriv->bmask & MS_RIGHT_BUTTON_MASK))) {
			pPriv->bmask |= bmask;
	    	} else {
			return;
	    	}
	}
	mieqEnqueue (&xE);
	break;
    case LOC_X_DELTA:
	miPointerDeltaCursor (MouseAccelerate(device,fe->value),0,time);
	break;
    case LOC_Y_DELTA:
	miPointerDeltaCursor (0,MouseAccelerate(device,fe->value),time);
	break;
#ifdef LOC_X_ABSOLUTE
    case LOC_X_ABSOLUTE:
	miPointerPosition (&x, &y);
	miPointerAbsoluteCursor (fe->value, y, time);
	break;
    case LOC_Y_ABSOLUTE:
	miPointerPosition (&x, &y);
	miPointerAbsoluteCursor (x, fe->value, time);
	break;
#endif
    default:
        time64 = fe->time.tv_sec;	/* firm_event still uses 32 bit time */
	FatalError ("amigaMouseEnqueueEvent: unrecognized id 0x%x val 0x%x time %s\n",
		fe->id, fe->value, ctime(&time64));
	break;
    }
}

/*ARGSUSED*/
static Bool
amigaCursorOffScreen (pScreen, x, y)
    ScreenPtr	*pScreen;
    int		*x, *y;
{
    int	    index, ret = FALSE;
    extern Bool PointerConfinedToScreen();

    if (PointerConfinedToScreen()) return TRUE;
    /*
     * Active Zaphod implementation:
     *    increment or decrement the current screen
     *    if the x is to the right or the left of
     *    the current screen.
     */
    if (amigaActiveZaphod &&
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
amigaCrossScreen (pScreen, entering)
    ScreenPtr	pScreen;
    Bool	entering;
{
    if (amigaFbs[pScreen->myNum].EnterLeave)
	(*amigaFbs[pScreen->myNum].EnterLeave) (pScreen, entering ? 0 : 1);
}

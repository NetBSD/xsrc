/*
 * $XFree86: xc/programs/Xserver/hw/xfree86/os-support/hurd/hurd_io.c,v 1.1.2.2 1998/12/22 12:27:34 hohndel Exp $
 *
 * Copyright 1997,1998 by UCHIYAMA Yasushi
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of UCHIYAMA Yasushi not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  UCHIYAMA Yasushi makes no representations
 * about the suitability of this software for any purpose.  It is provided
 * "as is" without express or implied warranty.
 *
 * UCHIYAMA YASUSHI DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL UCHIYAMA YASUSHI BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 *
 */
#define NEED_EVENTS
#include "X.h"
#include "Xproto.h"
#include "inputstr.h"
#include "scrnintstr.h"

#include "compiler.h"

#include "xf86.h"
#include "xf86Procs.h"
#include "xf86_OSlib.h"

#include "xf86_Config.h"

#include <stdio.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/file.h>
#include <assert.h>
#include <mach.h>
#include <sys/ioctl.h>

typedef unsigned short kev_type;		/* kd event type */
typedef unsigned char Scancode;

struct mouse_motion {		
    short mm_deltaX;		/* units? */
    short mm_deltaY;
};

typedef struct {
    kev_type type;			/* see below */
    struct timeval time;		/* timestamp */
    union {				/* value associated with event */
	boolean_t up;		/* MOUSE_LEFT .. MOUSE_RIGHT */
	Scancode sc;		/* KEYBD_EVENT */
	struct mouse_motion mmotion;	/* MOUSE_MOTION */
    } value;
} kd_event;

/* 
 * kd_event ID's.
 */
#define MOUSE_LEFT	1		/* mouse left button up/down */
#define MOUSE_MIDDLE	2
#define MOUSE_RIGHT	3
#define MOUSE_MOTION	4		/* mouse motion */
#define KEYBD_EVENT	5		/* key up/down */

/*
 *	Handle any XF86Config options for "OsMouse", How you treat errors
 *	is up to you, they may or may not be Fatal
 */
void
xf86OsMouseOption(
    int		lt,	/* type returned by gettoken */
    pointer	lp	/* The lexical return symbol */
    )
{
}
/*
 * xf86OsMouseProc --
 *      Handle the initialization, etc. of a mouse
 */
int
xf86OsMouseProc( DeviceIntPtr pPointer , int what )
{
    extern int miPointerGetMotionEvents(DeviceIntPtr pPtr, xTimecoord *coords,
					unsigned long start, unsigned long stop,
					ScreenPtr pScreen);
    unsigned char map[MSE_MAXBUTTONS + 1];
    int nbuttons;
    int mousefd;

    switch( what )
    {
    case DEVICE_INIT: 
	pPointer->public.on = FALSE;

	for (nbuttons = 0; nbuttons < MSE_MAXBUTTONS; ++nbuttons)
	    map[nbuttons + 1] = nbuttons + 1;

	InitPointerDeviceStruct((DevicePtr)pPointer, 
				map, 
				min(xf86Info.mouseDev->buttons, MSE_MAXBUTTONS),
				miPointerGetMotionEvents, 
				(PtrCtrlProcPtr)xf86MseCtrl, 
				0);
#ifdef XINPUT
	InitValuatorAxisStruct(pPointer,
			       0,
			       0, /* min val */
			       screenInfo.screens[0]->width, /* max val */
			       1, /* resolution */
			       0, /* min_res */
			       1); /* max_res */
	InitValuatorAxisStruct(pPointer,
			       1,
			       0, /* min val */
			       screenInfo.screens[0]->height, /* max val */
			       1, /* resolution */
			       0, /* min_res */
			       1); /* max_res */
	/* Initialize valuator values in synch
	 * with dix/event.c DefineInitialRootWindow
	 */
	*pPointer->valuator->axisVal = screenInfo.screens[0]->width / 2;
	*(pPointer->valuator->axisVal+1) = screenInfo.screens[0]->height / 2;
#endif
	break;
    case DEVICE_ON:
	if ( (xf86Info.mouseDev->mseFd = open(xf86Info.mouseDev->mseDevice,O_RDONLY|O_NONBLOCK) ) == -1 )
	    return !Success;
	AddEnabledDevice( xf86Info.mouseDev->mseFd );
	xf86Info.mouseDev->lastButtons = 0;
	xf86Info.mouseDev->emulateState = 0;
	pPointer->public.on = TRUE;
	break;
    case DEVICE_OFF:
    case DEVICE_CLOSE:
	if( close( xf86Info.mouseDev->mseFd ) != -1 )
	    RemoveEnabledDevice(mousefd);
	pPointer->public.on = FALSE;
	usleep(300000);
	break;
    }
    return Success;
}
/*
 * xf86OsMouseEvents --
 *      Get some events from our queue.  Process all outstanding events now.
 */
void
xf86OsMouseEvents()
{
    static kd_event eventList[64];
    int n; 
    kd_event *event = eventList;

    if( (n =  read( xf86Info.mouseDev->mseFd , eventList, sizeof eventList )) <= 0 )
	return;
    n /= sizeof( kd_event );
    while( n-- )
    {
	int buttons = xf86Info.mouseDev->lastButtons;
	int dx =0, dy = 0;
	switch( event->type )
	{
	case MOUSE_RIGHT:
	    buttons  = buttons & 6 |(event->value.up ? 0 : 1);
	    break;
	case MOUSE_MIDDLE:
	    buttons  = buttons & 5 |(event->value.up ? 0 : 2);
	    break;
	case MOUSE_LEFT:
	    buttons  = buttons & 3 |(event->value.up ? 0 : 4) ;
	    break;
	case MOUSE_MOTION:
	    dx = event->value.mmotion.mm_deltaX;
	    dy = - event->value.mmotion.mm_deltaY;
	    break;
	default:
	    ErrorF("Bad mouse event (%d)\n",event->type);
	    continue;
	}
	xf86PostMseEvent(xf86Info.pMouse,buttons, dx, dy );
	++event;
    }
    return;
}

void 
xf86MouseInit( MouseDevPtr mouse )
{
}
int 
xf86MouseOn( MouseDevPtr mouse)
{
    if ((mouse->mseFd = open(mouse->mseDevice, O_RDWR | O_NONBLOCK)) < 0)
    {
	if (xf86AllowMouseOpenFail) {
	    ErrorF("Cannot open mouse (%s) - Continuing...\n",
		   strerror(errno));
	    return(-2);
	}
	FatalError("Cannot open mouse (%s)\n", strerror(errno));
    }

    xf86SetupMouse(mouse);

    /* Flush any pending input */
    tcflush(mouse->mseFd, TCIFLUSH);

    return mouse->mseFd;
}
int 
xf86MouseOff( MouseDevPtr mouse,Bool doclose )
{
    int oldfd;

    if ((oldfd = mouse->mseFd) >= 0)
    {
	if (mouse->mseType == P_LOGI)
	{
	    write(mouse->mseFd, "U", 1);
	}
	if (mouse->oldBaudRate > 0) {
	    xf86SetMouseSpeed(mouse,
			      mouse->baudRate,
			      mouse->oldBaudRate,
			      xf86MouseCflags[mouse->mseType]);
	}
	close(mouse->mseFd);
	oldfd = mouse->mseFd;
	mouse->mseFd = -1;
    }
    return oldfd;
}

/***********************************************************************
 * Keyboard
 **********************************************************************/
void 
xf86SoundKbdBell(int loudness,int pitch,int duration)
{
    return;
}

void 
xf86SetKbdLeds(int leds)
{
    return;
}

int 
xf86GetKbdLeds()
{
    return 0;
}

void 
xf86SetKbdRepeat(char rad)
{
    return;
}

void 
xf86KbdInit()
{
    return;
}
int
xf86KbdOn()
{
    int data = 1;
    if( ioctl( xf86Info.consoleFd, _IOW('k', 1, int),&data) < 0)
	FatalError("Cannot set event mode on keyboard (%s)\n",strerror(errno));
    return xf86Info.consoleFd;
}
int
xf86KbdOff()
{
    int data = 2;
    if( ioctl( xf86Info.consoleFd, _IOW('k', 1, int),&data) < 0)
	FatalError("can't reset keyboard mode (%s)\n",strerror(errno));
}

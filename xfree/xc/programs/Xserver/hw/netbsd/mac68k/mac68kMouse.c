/*-
 * Copyright (C) 1994 Bradley A. Grantham and Lawrence A. Kesteloot
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
 * 3. The names of the Alice Group or any of its members may not be used
 *    to endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE ALICE GROUP ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE ALICE GROUP BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include "mac68k.h"


#define LEFTB(b)	(b & 0x1)	/* Mouse buttons are stored reverse. */
#define MIDDLEB(b)	(b & 0x2)
#define RIGHTB(b)	(b & 0x4)


static Bool mac68k_msoffscreen(
	ScreenPtr	*screen,
	int *x,
	int *y)
{
	int index;

	if((screenInfo.numScreens > 1) &&
		((*x >= (*screen)->width) || (*x < 0)))
	{
		index = (*screen)->myNum;

		if(*x < 0){
			index = (index - 1 + screenInfo.numScreens) %
				screenInfo.numScreens;
			*screen = screenInfo.screens[index];
			*x += (*screen)->width;
		}else{
			*x -= (*screen)->width;
			index = (index + 1 ) % screenInfo.numScreens;
			*screen = screenInfo.screens[index];
		}

		return(TRUE);
	}
	return(FALSE);
}


static void mac68k_mscrossscreen(
	ScreenPtr	screen,
	Bool	entering)
{
	/* nothing. (stolen from A/UX X server) */
}


static void mac68k_mswarp(
	ScreenPtr	screen,
	int x,
	int y)
{
	/* Why even have a function here? */
	miPointerWarpCursor(screen, x, y);
}


miPointerScreenFuncRec mac_mousefuncs = {
	mac68k_msoffscreen,
	mac68k_mscrossscreen,
	mac68k_mswarp
};


void mac68k_mousectrl()
{
	return;
}


int mac68k_mouseproc(
	DevicePtr mouse,
	int what)
{
	BYTE map[4];

	switch(what){
		case DEVICE_INIT:
			if(mouse != LookupPointerDevice()){
				ErrorF("Mouse routines can only handle DESKTOP"
					" mice.\n");
				return(!Success);
			}

			map[0] = 0;
			map[1] = 1;
			map[2] = 2;
			map[3] = 3;

			InitPointerDeviceStruct(mouse, map, 3,
				miPointerGetMotionEvents, mac68k_mousectrl,
				miPointerGetMotionBufferSize());

			mouse->on = FALSE;
			break;

		case DEVICE_ON:
			mouse->on = TRUE;
			break;

		case DEVICE_OFF:
			mouse->on = FALSE;
			break;

		case DEVICE_CLOSE:
			/* nothing! */
			break;
	}

	return(Success);
}


static int accel_mouse (
	DeviceIntPtr mouse,
	int delta)
{
    register int sgn = sign(delta);
    register PtrCtrl *ctrlptr;

    delta = abs(delta);
    ctrlptr = &mouse->ptrfeed->ctrl;

    if (delta > ctrlptr->threshold)
	return (sgn * (ctrlptr->threshold + ((delta - ctrlptr->threshold) *
		ctrlptr->num) / ctrlptr->den));
    else
	return (sgn * delta);
}


void mac68k_processmouse(
	DeviceIntPtr	mouse,
	adb_event_t	*event)
{
	xEvent	xev;
	int	dx, dy;
	static int	buttons = 0;

	mac_lasteventtime = xev.u.keyButtonPointer.time =
		TVTOMILLI(event->timestamp);

	if(buttons != event->u.m.buttons){
		if(LEFTB(buttons) != LEFTB(event->u.m.buttons)){
			xev.u.u.detail = 1;	/* leftmost */
			xev.u.u.type = LEFTB(event->u.m.buttons) ?
				ButtonPress : ButtonRelease;
			mieqEnqueue(&xev);
		}
		if(MIDDLEB(buttons) != MIDDLEB(event->u.m.buttons)){
			xev.u.u.detail = 2;	/* middle */
			xev.u.u.type = MIDDLEB(event->u.m.buttons) ?
				ButtonPress : ButtonRelease;
			mieqEnqueue(&xev);
		}
		if(RIGHTB(buttons) != RIGHTB(event->u.m.buttons)){
			xev.u.u.detail = 3;	/* right */
			xev.u.u.type = RIGHTB(event->u.m.buttons) ?
				ButtonPress : ButtonRelease;
			mieqEnqueue(&xev);
		}
		buttons = event->u.m.buttons;
	}

	dx = accel_mouse(mouse, event->u.m.dx);
	dy = accel_mouse(mouse, event->u.m.dy);

	miPointerDeltaCursor(dx, dy, mac_lasteventtime);
}


void mac68k_getmouse(
	void)
{
	int index;

	/* make sure there is a mouse */

	/* FatalError("Cannot run X server without a mouse.\n"); */
}

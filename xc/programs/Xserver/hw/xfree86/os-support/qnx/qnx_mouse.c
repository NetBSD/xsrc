/*
 * (c) Copyright 1998 by Sebastien Marineau
 *			<sebastien@qnx.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a 
 * copy of this software and associated documentation files (the "Software"), 
 * to deal in the Software without restriction, including without limitation 
 * the rights to use, copy, modify, merge, publish, distribute, sublicense, 
 * and/or sell copies of the Software, and to permit persons to whom the 
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL 
 * SEBASTIEN MARINEAU BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, 
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF 
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE 
 * SOFTWARE.
 * 
 * Except as contained in this notice, the name of Sebastien Marineau shall not be
 * used in advertising or otherwise to promote the sale, use or other dealings
 * in this Software without prior written authorization from Sebastien Marineau.
 *
 * $XFree86: xc/programs/Xserver/hw/xfree86/os-support/qnx/qnx_mouse.c,v 1.1.2.2 1999/07/23 13:42:36 hohndel Exp $
 */

/* This module contains the qnx-specific functions to access the keyboard
 * and the console.
 */

/* [jcm] Fix mouse problem, check XINPUT for XFree86 3.3.3.1 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <i86.h>
#include <sys/mman.h>
#include <sys/dev.h>
#include <sys/mouse.h>
#include <sys/proxy.h>
#include <errno.h>

#include <X.h>
#include "xf86.h"
#include "xf86Priv.h"
#include "xf86_OSlib.h"
#include "inputstr.h"

extern int miPointerGetMotionEvents(DeviceIntPtr pPtr, xTimecoord *coords, 
				unsigned long start, unsigned long stop, 
				ScreenPtr pScreen);


struct _mouse_ctrl *QNX_mouse = NULL;
pid_t QNX_mouse_proxy = -1;
Bool QNX_mouse_event = FALSE;

int xf86MouseOff(mouse, doclose)
MouseDevPtr mouse;
Bool doclose;
{
ErrorF("Called mouseOff\n");
	if(QNX_mouse) {
		mouse_close (QNX_mouse);
		QNX_mouse = NULL;
		}
	return(-1);
}

void xf86OsMouseEvents()
{
	struct mouse_event events[16];
	int i, nEvents;
	int buttons, col, row;
	int armed = 0;

	while ((nEvents = mouse_read(QNX_mouse, &events, 
		16, QNX_mouse_proxy, &armed) ) > 0) {
		/* ErrorF("Got mouse event, #%d!\n", nEvents);*/

		for (i = 0; i < nEvents; i ++){	
			col = events[i].dx;
			row = -events[i].dy;
			buttons = events[i].buttons; 
			xf86PostMseEvent(xf86Info.pMouse, buttons, col, row);
			}
		xf86Info.inputPending = TRUE;
		}
	if (!armed) ErrorF("Drained mouse queue, armed = 0??\n");
	QNX_mouse_event = FALSE;
}

void
xf86MouseInit(mouse)
MouseDevPtr mouse;
{
	ErrorF("Called MouseInit\n");
	return;
}

int
xf86MouseOn(mouse)
MouseDevPtr mouse;
{
	struct mouse_event mevent;
	int armed, ret;

	if((QNX_mouse_proxy = qnx_proxy_attach(0, 0, 0, -1)) == -1){
		FatalError("xf86MouseOn: Could not create mouse proxy; %s\n", 
			strerror(errno));
		}
        if ((QNX_mouse = mouse_open(0, NULL, xf86Info.consoleFd)) == NULL) {
        
                if (xf86AllowMouseOpenFail) {
                        ErrorF("Cannot open mouse (%s) - Continuing...\n",
                                strerror(errno));
                        return(-2);
                	}
                FatalError("Cannot open mouse (%s)\n", strerror(errno));
        	}
	ErrorF("Opened mouse: handle %d fd %d\n", QNX_mouse->handle, 
	QNX_mouse->fd);	
        /* Flush any pending input */
        mouse_flush(QNX_mouse);
	while (!armed) {
		ret = mouse_read(QNX_mouse, &mevent, 1, 
			QNX_mouse_proxy, &armed);
		if (ret < 0) { 
			FatalError("xf86MouseOn: could not arm proxy; %s\n",
				strerror(errno));
			}
		} 
        return(-1);
}

void
xf86OsMouseOption(token, lex_ptr)
int token;
pointer lex_ptr;
{
	/* No options are supported for now - could be later */
	ErrorF("xf86OsMouseOption: no supported options at this time\n");
}

/* The main mouse setup proc */
int
xf86OsMouseProc(pPointer, what)
DeviceIntPtr pPointer;
int what;
{
	int i, ret, armed;
	int nbuttons;
	unsigned char *map;
	struct mouse_event mevent;

	switch (what) {
	case DEVICE_INIT:
		pPointer->public.on = FALSE;
		if (QNX_mouse_proxy == -1) {
			if((QNX_mouse_proxy = 
				qnx_proxy_attach(0, 0, 0, -1)) == -1){
				FatalError("xf86MouseOn: Could not create mouse proxy; %s\n", 
				strerror(errno));
				}
			}
		if (QNX_mouse == NULL) 	QNX_mouse = 
			mouse_open(0, NULL, xf86Info.consoleFd);
		if (QNX_mouse == NULL) {
                	if (xf86AllowMouseOpenFail) {
                        	ErrorF("Cannot open mouse (%s) - Continuing...\n",
                                	strerror(errno));
                        	return(-1);
                        	}
                	FatalError("Cannot open mouse (%s)\n", strerror(errno));
                	}
		/* Ok, so we have opened the channel to the mouse driver */
	ErrorF("Opened mouse: handle %d buttons\n", QNX_mouse->handle, 
	QNX_mouse->buttons);	
		xf86Info.mouseDev->mseFd = QNX_mouse->fd;
        	mouse_flush(QNX_mouse);
		QNX_mouse_event = FALSE;
		/* How de we determine how many buttons we have?? */
		nbuttons = 3;
		map = (unsigned char *) xalloc(nbuttons + 1);
		if (map == (unsigned char *) NULL)
			FatalError("Failed to allocate memory for mouse structures\n");
		for(i=0;i <= nbuttons; i++)
			map[i] = i;			
		InitPointerDeviceStruct ((DevicePtr) pPointer, map, nbuttons, 
			miPointerGetMotionEvents, (PtrCtrlProcPtr) xf86MseCtrl,
			miPointerGetMotionBufferSize());

#ifdef XINPUT
      InitValuatorAxisStruct( (DevicePtr) pPointer,
			     0,
			     0, /* min val */
			     screenInfo.screens[0]->width, /* max val */
			     1, /* resolution */
			     0, /* min_res */
			     1); /* max_res */
      InitValuatorAxisStruct( (DevicePtr) pPointer,
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

		xfree(map);
		break;

	case DEVICE_ON:
		if(QNX_mouse == NULL) return(-1);
		xf86Info.mouseDev->lastButtons = 0;
		xf86Info.mouseDev->emulateState = 0;
		pPointer->public.on = TRUE;
        	mouse_flush(QNX_mouse);
		/* AddEnabledDevice(xf86Info.mouseDev->mseFd); */
		ret = mouse_read(QNX_mouse, &mevent, 0, 
			QNX_mouse_proxy, NULL);
		ErrorF("MouseOn: armed proxy, %d, proxy pid %d\n", ret, 
			QNX_mouse_proxy);
		if (ret < 0) { 
			FatalError("xf86MouseOn: could not arm proxy; %s\n",
				strerror(errno));
			}
		break;

	case DEVICE_CLOSE:
	case DEVICE_OFF:
		if(QNX_mouse == NULL) return(-1);
		pPointer->public.on = FALSE;
		if (what == DEVICE_CLOSE){
			mouse_close (QNX_mouse);
			QNX_mouse = NULL;
			}
		break;
	}
	return (Success);
}				



/* This table is somewhat useless, since we only
 * support the mouse type OsMouse. This is because we use 
 * the Input/Mouse driver.
 */ 

Bool xf86SupportedMouseTypes[] =
{
	FALSE,	/* Microsoft */
	FALSE,	/* MouseSystems */
	FALSE,	/* MMSeries */
	FALSE,	/* Logitech */
	FALSE,	/* BusMouse */
	FALSE,	/* MouseMan */
	FALSE,	/* PS/2 */
	FALSE,	/* Hitachi Tablet */
};

int xf86NumMouseTypes = sizeof(xf86SupportedMouseTypes) /
			sizeof(xf86SupportedMouseTypes[0]);



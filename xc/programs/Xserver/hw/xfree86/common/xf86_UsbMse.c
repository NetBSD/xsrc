/* $XFree86: xc/programs/Xserver/hw/xfree86/common/xf86_UsbMse.c,v 1.1.2.1 1999/12/01 12:49:24 hohndel Exp $ */
/* $NetBSD: xf86_UsbMse.c,v 1.1.1.1 2000/01/10 22:01:56 veego Exp $	*/

/*
 * Copyright (c) 1998 The NetBSD Foundation, Inc.
 * Copyright (c) 1999 The XFree86 Project, Inc. 
 * All rights reserved.
 *
 * This code is derived from software contributed to The NetBSD Foundation
 * by Lennart Augustsson (augustss@carlstedt.se) at
 * Carlstedt Research & Technology.
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
 *        This product includes software developed by the NetBSD
 *        Foundation, Inc. and its contributors.
 * 4. Neither the name of The NetBSD Foundation nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE NETBSD FOUNDATION, INC. AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "xf86Procs.h"
#include "xf86_OSlib.h"
#include "xf86_Config.h"

#include "xf86_UsbMse.h"

#define HUP_GENERIC_DESKTOP     0x0001
#define HUP_BUTTON              0x0009

#define HUG_X                   0x0030
#define HUG_Y                   0x0031
#define HUG_Z                   0x0032
#define HUG_WHEEL               0x0038

#define HID_USAGE2(p,u) (((p) << 16) | u)

#define UMS_BUT(i) ((i) == 0 ? 2 : (i) == 1 ? 0 : (i) == 2 ? 1 : (i))

/*
 * Allocate a UsbMouseRec and probe for locators 
 */
struct UsbMouseRec *
usbMouseInit(MouseDevPtr mouse)
{
    struct UsbMouseRec *ptr;
    int i, id;

    ptr = (struct UsbMouseRec *)xalloc(sizeof(struct UsbMouseRec));

    ptr->dev = hid_get_report_desc(mouse->mseFd);
    if (hid_locate(ptr->dev, HID_USAGE2(HUP_GENERIC_DESKTOP, HUG_X),
		   hid_input, &ptr->loc_x) < 0) {
	ErrorF("usbMouseInit: no x locator\n");
    }
    if (hid_locate(ptr->dev, HID_USAGE2(HUP_GENERIC_DESKTOP, HUG_Y),
		   hid_input, &ptr->loc_y) < 0) {
	ErrorF("usbMouseInit: no y locator\n");
    }
    if (hid_locate(ptr->dev, HID_USAGE2(HUP_GENERIC_DESKTOP, HUG_WHEEL),
		   hid_input, &ptr->loc_z) < 0) {
	ErrorF("usbMouseInit: no wheel locator\n");
    }
    for (i = 1; i <= MSE_MAXBUTTONS; i++) {
	if (!hid_locate(ptr->dev, HID_USAGE2(HUP_BUTTON, i),
			hid_input, &ptr->loc_btn[i-1])) 
	    break;
    }
    mouse->buttons = i - 1;
    mouse->protoPara[4] = hid_report_size(ptr->dev, hid_input, &id);
    
    return ptr;
}

/*
 * Parse one event
 */
void 
usbMouseProtocol(MouseDevPtr mouse, int *dx, int *dy, int *dz, int *buttons)
{
    int i;

    *dx = hid_get_data(mouse->pBuf, &mouse->usb->loc_x);
    *dy = hid_get_data(mouse->pBuf, &mouse->usb->loc_y);
    *dz = hid_get_data(mouse->pBuf, &mouse->usb->loc_z);

    *buttons = 0;
    for (i = 0; i < mouse->buttons; i++) {
	if (hid_get_data(mouse->pBuf, &mouse->usb->loc_btn[i])) 
	    *buttons |= (1 << UMS_BUT(i));
    }
}



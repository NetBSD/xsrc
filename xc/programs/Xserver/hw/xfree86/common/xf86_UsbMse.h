/*
 * Copyright (c) 1999, The XFree86 Project, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the names of The XFree86 Project, Inc. not be
 * used in advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.  The XFree86
 * Project Inc. makes no representations about the suitability of this
 * software for any purpose.  It is provided "as is" without express or
 * implied warranty.
 *
 * THE XFREE86 PROJECT INC. DISCLAIM ALL WARRANTIES WITH REGARD TO THIS
 * SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS, IN NO EVENT SHALL THE XFREE86 PROJECT INC. BE LIABLE FOR ANY
 * SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER
 * RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF
 * CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * $XFree86: xc/programs/Xserver/hw/xfree86/common/xf86_UsbMse.h,v 1.1.2.2 1999/12/03 10:12:40 hohndel Exp $
 */
#ifndef _XF86_USBMSE_H
#define _XF86_USBMSE_H

#include <usb.h>

struct UsbMouseRec {
    report_desc_t dev;
    hid_item_t loc_x;
    hid_item_t loc_y;
    hid_item_t loc_z;
    hid_item_t loc_btn[MSE_MAXBUTTONS];
};

#if NeedFunctionPrototypes
struct UsbMouseRec *usbMouseInit(MouseDevPtr mouse);
void usbMouseProtocol(MouseDevPtr mouse, int *dx, int *dy, int *dz, 
		      int *buttons);
#endif

#endif

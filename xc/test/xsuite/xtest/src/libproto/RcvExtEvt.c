/*
Copyright (c) 1990, 1991  X Consortium

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of the X Consortium shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from the X Consortium.

 * Copyright 1990, 1991 by UniSoft Group Limited.
 * 
 * Copyright 1993 by the Hewlett-Packard Company.
 *
 * Permission to use, copy, modify, distribute, and sell this software and
 * its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation, and that the names of HP,  and UniSoft not be
 * used in advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.  HP,  and UniSoft
 * make no representations about the suitability of this software for any
 . purpose.  It is provided "as is" without express or implied warranty.
 *
 * $XConsortium: RcvExtEvt.c,v 1.5 94/04/17 21:01:18 rws Exp $
 */
/*
 * ***************************************************************************
 *  Copyright 1988 by Sequent Computer Systems, Inc., Portland, Oregon       *
 *                                                                           *
 *                                                                           *
 *                          All Rights Reserved                              *
 *                                                                           *
 *  Permission to use, copy, modify, and distribute this software and its    *
 *  documentation for any purpose and without fee is hereby granted,         *
 *  provided that the above copyright notice appears in all copies and that  *
 *  both that copyright notice and this permission notice appear in          *
 *  supporting documentation, and that the name of Sequent not be used       *
 *  in advertising or publicity pertaining to distribution or use of the     *
 *  software without specific, written prior permission.                     *
 *                                                                           *
 *  SEQUENT DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING *
 *  ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL *
 *  SEQUENT BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR  *
 *  ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,      *
 *  WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,   *
 *  ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS      *
 *  SOFTWARE.                                                                *
 * ***************************************************************************
 */

/*
 *	$Header: /cvsroot/xsrc/xc/test/xsuite/xtest/src/libproto/Attic/RcvExtEvt.c,v 1.1.1.1 1997/03/15 06:19:55 scottr Exp $
 */

#ifndef lint
static char rcsid[]="$Header: /cvsroot/xsrc/xc/test/xsuite/xtest/src/libproto/Attic/RcvExtEvt.c,v 1.1.1.1 1997/03/15 06:19:55 scottr Exp $";
#endif

#include "XstlibInt.h"
#include "extensions/XIproto.h"
#ifdef Xpi
#include "xtestext1.h"
#endif
#include "DataMove.h"

#define EVENT_HEADER	4	/* constant header */
#define XInputNumEvents	15
extern int XInputFirstEvent;

int
Rcv_Ext_Evt(rp,rbuf,client,base)
xEvent *rp;
char rbuf[];
int client;
int base;
{
#ifdef Xpi
	int xtestType;
#endif
	int needswap = Xst_clients[client].cl_swap;
	char *rbp = rbuf;
	int valid = 1;		/* assume all is OK */
	int rtype;

	rbp += EVENT_HEADER;

	rtype = real_type(rp->u.u.type) - XInputFirstEvent; 
	if (rtype >= 0 && rtype < XInputNumEvents) {
		switch (rtype) {
		case XI_DeviceKeyPress:
		case XI_DeviceKeyRelease:
		case XI_DeviceButtonPress:
		case XI_DeviceButtonRelease:
		case XI_DeviceMotionNotify:
		case XI_ProximityIn:
		case XI_ProximityOut:
			{
			deviceKeyButtonPointer *rpi = (deviceKeyButtonPointer *) rp;
	
			rpi->time = unpack4(&rbp,needswap);
			rpi->root = unpack4(&rbp,needswap);
			rpi->event = unpack4(&rbp,needswap);
			rpi->child = unpack4(&rbp,needswap);
			rpi->root_x = unpack2(&rbp,needswap);
			rpi->root_y = unpack2(&rbp,needswap);
			rpi->event_x = unpack2(&rbp,needswap);
			rpi->event_y = unpack2(&rbp,needswap);
			rpi->state = unpack2(&rbp,needswap);
			rpi->same_screen = unpack1(&rbp);
			rpi->deviceid = unpack1(&rbp);
			break;
			}
		case XI_DeviceFocusIn:
		case XI_DeviceFocusOut:
			{
			deviceFocus *rpi = (deviceFocus *) rp;
	
			rpi->time = unpack4(&rbp,needswap);
			rpi->window = unpack4(&rbp,needswap);
			rpi->mode = unpack1(&rbp);
			rpi->deviceid = unpack1(&rbp);
			break;
			}
		case XI_ChangeDeviceNotify:
			{
			changeDeviceNotify *rpi = (changeDeviceNotify *) rp;
	
			rpi->time = unpack4(&rbp,needswap);
			rpi->request = unpack1(&rbp);
			break;
			}
		case XI_DeviceStateNotify:
			{
			deviceStateNotify *rpi = (deviceStateNotify *) rp;
	
			rpi->time = unpack4(&rbp,needswap);
			rpi->num_keys = unpack1(&rbp);
			rpi->num_buttons = unpack1(&rbp);
			rpi->num_valuators = unpack1(&rbp);
			rpi->classes_reported = unpack1(&rbp);
			rpi->buttons[0] = unpack1(&rbp);
			rpi->buttons[1] = unpack1(&rbp);
			rpi->buttons[2] = unpack1(&rbp);
			rpi->buttons[3] = unpack1(&rbp);
			rpi->keys[0] = unpack1(&rbp);
			rpi->keys[1] = unpack1(&rbp);
			rpi->keys[2] = unpack1(&rbp);
			rpi->keys[3] = unpack1(&rbp);
			rpi->valuator0 = unpack4(&rbp,needswap);
			rpi->valuator1 = unpack4(&rbp,needswap);
			rpi->valuator2 = unpack4(&rbp,needswap);
			break;
			}
		case XI_DeviceMappingNotify:
			{
			deviceMappingNotify *rpi = (deviceMappingNotify *) rp;
	
			rpi->request = unpack1(&rbp);
			rpi->firstKeyCode = unpack1(&rbp);
			rpi->count = unpack1(&rbp);
			rpi->pad1 = unpack1(&rbp);
			rpi->time = unpack4(&rbp,needswap);
			break;
			}
		case XI_DeviceValuator:
			{
			deviceValuator *rpi = (deviceValuator *) rp;
	
			rpi->device_state = unpack2(&rbp);
			rpi->num_valuators = unpack1(&rbp);
			rpi->first_valuator = unpack1(&rbp);
			rpi->valuator0 = unpack4(&rbp,needswap);
			rpi->valuator1 = unpack4(&rbp,needswap);
			rpi->valuator2 = unpack4(&rbp,needswap);
			rpi->valuator3 = unpack4(&rbp,needswap);
			rpi->valuator4 = unpack4(&rbp,needswap);
			rpi->valuator5 = unpack4(&rbp,needswap);
			break;
			}
		default:
			report("Unknown event of type %d received",
			    real_type(rp->u.u.type));
			DEFAULT_ERROR;
		}
	}
	else
		{
		report("Unknown event of type %d received",
		    real_type(rp->u.u.type));
		DEFAULT_ERROR;
		}
	return(valid);

}

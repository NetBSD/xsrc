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

 * Copyright 1990, 1991 and UniSoft Group Limited.
 * 
 * Copyright 1993 by the Hewlett-Packard Company.
 *
 * Permission to use, copy, modify, distribute, and sell this software and
 * its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation, and that the names of MIT, HP, and UniSoft not be
 * used in advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.  MIT, HP, and UniSoft
 * make no representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 *
 * $XConsortium: ShowExtRep.c,v 1.4 94/04/17 21:01:27 rws Exp $
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
 *	$Header: /cvsroot/xsrc/xc/test/xsuite/xtest/src/libproto/Attic/ShowExtRep.c,v 1.1.1.1 1997/03/15 06:19:58 scottr Exp $
 */

#ifndef lint
static char rcsid[]="$Header: /cvsroot/xsrc/xc/test/xsuite/xtest/src/libproto/Attic/ShowExtRep.c,v 1.1.1.1 1997/03/15 06:19:58 scottr Exp $";
#endif

#include "XstlibInt.h"
extern int XInputMajorOpcode;

void
Show_Ext_Rep(mp,type,bytes_given)
xReply *mp;
int type;
long bytes_given;
{
	if (type & 0x0ff == XInputMajorOpcode) {
		switch (type >> 8) {
		case X_GetExtensionVersion:
			BPRINTF1("GetExtensionVersion:\n");
			break;
		case X_ListInputDevices:
			BPRINTF1("ListInputDevices:\n");
			break;
		case X_OpenDevice:
			BPRINTF1("OpenDevice:\n");
			break;
		case X_SetDeviceMode:
			BPRINTF1("SetDeviceMode:\n");
			break;
		case X_GetSelectedExtensionEvents:
			BPRINTF1("GetSelectedExtensionEvents:\n");
			break;
		case X_GetDeviceDontPropagateList:
			BPRINTF1("GetDeviceDontPropagateList:\n");
			break;
		case X_GetDeviceMotionEvents:
			BPRINTF1("GetDeviceMotionEvents:\n");
			break;
		case X_ChangeKeyboardDevice:
			BPRINTF1("ChangeKeyboardDevice:\n");
			break;
		case X_ChangePointerDevice:
			BPRINTF1("ChangePointerDevice:\n");
			break;
		case X_GrabDevice:
			BPRINTF1("GrabDevice:\n");
			break;
		case X_GetDeviceFocus:
			BPRINTF1("GetDeviceFocus:\n");
			break;
		case X_GetFeedbackControl:
			BPRINTF1("GetFeedbackControl:\n");
			break;
		case X_GetDeviceKeyMapping:
			BPRINTF1("GetDeviceKeyMapping:\n");
			break;
		case X_GetDeviceModifierMapping:
			BPRINTF1("GetDeviceModifierMapping:\n");
			break;
		case X_SetDeviceModifierMapping:
			BPRINTF1("SetDeviceModifierMapping:\n");
			break;
		case X_GetDeviceButtonMapping:
			BPRINTF1("GetDeviceButtonMapping:\n");
			break;
		case X_SetDeviceButtonMapping:
			BPRINTF1("SetDeviceButtonMapping:\n");
			break;
		case X_QueryDeviceState:
			BPRINTF1("QueryDeviceState:\n");
			break;
		case X_SetDeviceValuators:
			BPRINTF1("SetDeviceValuators:\n");
			break;
		case X_GetDeviceControl:
			BPRINTF1("GetDeviceControl:\n");
			break;
		case X_ChangeDeviceControl:
			BPRINTF1("GetDeviceControl:\n");
			break;
		default:
			BPRINTF1("Impossible request:\n");
			BPRINTF2("\trepType = %ld\n",(long) ((xGetDeviceControlReq *)mp)->reqType);
			break;
		}
	}
	else 	{
		BPRINTF1("Unsupported Extension request:\n");
		BPRINTF2("\treqType = %ld\n",(long) ((xGetDeviceControlReq *)mp)->reqType);
	}
}

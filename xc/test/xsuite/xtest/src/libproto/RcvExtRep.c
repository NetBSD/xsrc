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
 * $XConsortium: RcvExtRep.c,v 1.8 94/04/17 21:01:19 rws Exp $
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
 *	$Header: /cvsroot/xsrc/xc/test/xsuite/xtest/src/libproto/Attic/RcvExtRep.c,v 1.1.1.1 1997/03/15 06:19:55 scottr Exp $
 */

#ifndef lint
static char rcsid[]="$Header: /cvsroot/xsrc/xc/test/xsuite/xtest/src/libproto/Attic/RcvExtRep.c,v 1.1.1.1 1997/03/15 06:19:55 scottr Exp $";
#endif

#include "stdio.h"
#include "extensions/XI.h"
#include "extensions/XIproto.h"
#include "XstlibInt.h"
#ifdef Xpi
#include "xtestext1.h"
#endif
#include "DataMove.h"

#define REPLY_HEADER	8	/* number of bytes */

extern int XInputMajorOpcode;
static void Length_Error();

int
Rcv_Ext_Rep(rp,rbuf,type,client)
xReply *rp;   /* pointer to XLIB-format reply structure */
char rbuf[];  /* receive buffer for reply data */
int type;     /* */
int client;   /* */
{       /*
	needswap           
	rbp                pointer to first byte of receive buffer after header
	valuePtr           pointer to first byte of rp after fixed-size part
	                   of reply
	i                  
	nlen               
	valid              
	nitems             
	calculated_length  
	*/

	int needswap = Xst_clients[client].cl_swap;
	char *nptr;
	char *rbp = (char *) ((char *)rbuf + REPLY_HEADER);
	unsigned char *valuePtr = (unsigned char *) ((unsigned char *)rp +
	    sizeof(xReply));
	int i, j;
	int nlen;
	int valid = 1;		/* assume all is OK */
	int nitems;
	int calculated_length = 0;
	int extension;
	unsigned long bytes_there = (long)(rp->generic.length<<2) + sizeof(xReply);

	Log_Debug2("Rcv_Rep(): type = %d, length = %d\n", type, rp->generic.length);
	extension = type & 0x0ff;
	if (extension == XInputMajorOpcode) {
	type >>= 8;
	switch (type) {
	case X_GetExtensionVersion:
		if (rp->generic.length != 0) {
		    Length_Error(max(bytes_there,calculated_length<<2),client,rp,type,"GetExtensionVersion",0);
		    break;
		}
		break;
	case X_ListInputDevices:
		{
	        /* rp->generic.data1 is the number of extension strings in
		   the returned list of strings. */
	        int value_len = 0;       /* total bytes in returned value */
		xDeviceInfo *list =(xDeviceInfoPtr)(rbuf + sizeof (xReply));
		xAnyClassPtr any;
		int ndevices;

		ndevices = unpack1(&rbp);
		((xListInputDevicesReply *)rp)->ndevices = ndevices;
		calculated_length = ndevices * sizeof (xDeviceInfo);
		any =(xAnyClassPtr)((char *) list + calculated_length);
		for (i = 0; i < ndevices; i++, list++) 
		    {
		    for (j = 0; j < list->num_classes; j++) 
			{
		        calculated_length += any->length;
		        any = (xAnyClassPtr)((char *) any + any->length);
			}
		    }
		for (i=0, nptr=(char *) any; i < ndevices; i++) {
		    calculated_length += *nptr + 1;
		    nptr += (*nptr + 1);
		}
		calculated_length++; /* add 1 for name buffer */

		calculated_length = (calculated_length + 3) / 4;
		if (rp->generic.length != calculated_length) {
		    Length_Error(max(bytes_there,calculated_length<<2),client,rp,type,"ListInputDevices",calculated_length);
		    break;
		}
		bcopy(rbuf + 32, valuePtr, calculated_length << 2);
		break;
		}
	case X_OpenDevice:
		((xOpenDeviceReply *)rp)->num_classes = unpack1(&rbp);
		calculated_length = (((xOpenDeviceReply *)rp)->num_classes + 1)/2;
		if (rp->generic.length != calculated_length) {
		    Length_Error(max(bytes_there,calculated_length<<2),client,rp,type,"OpenDevice",calculated_length);
		    break;
		}
		rbp += 23;
		Unpack_Longs((long *) valuePtr, &rbp,
		    ((xOpenDeviceReply *)rp)->length,needswap);
		break;
	case X_SetDeviceMode:
		((xSetDeviceModeReply *)rp)->status = unpack1(&rbp);
		if (rp->generic.length != 0) {
		    Length_Error(max(bytes_there,calculated_length<<2),client,rp,type,"SetDeviceMode",0);
		    break;
		}
		break;
	case X_GetSelectedExtensionEvents:
		((xGetSelectedExtensionEventsReply *)rp)->this_client_count = unpack2(&rbp, needswap);
		((xGetSelectedExtensionEventsReply *)rp)->all_clients_count = unpack2(&rbp, needswap);
		calculated_length = 
		   ((xGetSelectedExtensionEventsReply *)rp)->all_clients_count +
		   ((xGetSelectedExtensionEventsReply *)rp)->this_client_count;
		if (rp->generic.length != calculated_length) {
		    Length_Error(max(bytes_there,calculated_length<<2),client,rp,type,"GetSelectedExtensionEvents",calculated_length);
		    break;
		}
		rbp += 20;
		Unpack_Longs((long *) valuePtr, &rbp,
		    ((xGetSelectedExtensionEventsReply *)rp)->length,needswap);
		break;
	case X_GetDeviceDontPropagateList:
		((xGetDeviceDontPropagateListReply *)rp)->count = 
			unpack2(&rbp, needswap);
		calculated_length = 
		   ((xGetDeviceDontPropagateListReply *)rp)->count;
		if (rp->generic.length != calculated_length) {
		    Length_Error(max(bytes_there,calculated_length<<2),client,rp,type,"GetDeviceDontPropagateList",calculated_length);
		    break;
		}
		rbp += 22;
		Unpack_Longs((long *) valuePtr, &rbp,
		    ((xGetDeviceDontPropagateListReply *)rp)->length,needswap);
		break;
	case X_GetDeviceMotionEvents:
		((xGetDeviceMotionEventsReply *)rp)->nEvents = 
			unpack4(&rbp, needswap);
		((xGetDeviceMotionEventsReply *)rp)->axes = unpack1(&rbp);
		((xGetDeviceMotionEventsReply *)rp)->mode = unpack1(&rbp);
		calculated_length = 
		   ((xGetDeviceMotionEventsReply *)rp)->nEvents *
		(((xGetDeviceMotionEventsReply *)rp)->axes + 1);
		if (rp->generic.length != calculated_length) {
		    Length_Error(max(bytes_there,calculated_length<<2),client,rp,type,"GetDeviceMotionEvents",calculated_length);
		    break;
		}
		rbp += 18;
		Unpack_Longs((long *) valuePtr, &rbp,
		    ((xGetDeviceMotionEventsReply *)rp)->length,needswap);
		break;
	case X_ChangeKeyboardDevice:
		((xChangeKeyboardDeviceReply *)rp)->status = unpack1(&rbp);
		if (rp->generic.length != 0) {
		    Length_Error(max(bytes_there,calculated_length<<2),client,rp,type,"ChangeKeyboardDevice",0);
		    break;
		}
		break;
	case X_ChangePointerDevice:
		((xChangePointerDeviceReply *)rp)->status = unpack1(&rbp);
		if (rp->generic.length != 0) {
		    Length_Error(max(bytes_there,calculated_length<<2),client,rp,type,"ChangePointerDevice",0);
		    break;
		}
		break;
	case X_GrabDevice:
		((xGrabDeviceReply *)rp)->status = unpack1(&rbp);
		if (rp->generic.length != 0) {
		    Length_Error(max(bytes_there,calculated_length<<2),client,rp,type,"GrabDevice",0);
		    break;
		}
		break;
	case X_GetDeviceFocus:
		((xGetDeviceFocusReply *)rp)->focus = unpack4(&rbp,needswap);
		((xGetDeviceFocusReply *)rp)->time = unpack4(&rbp,needswap);
		((xGetDeviceFocusReply *)rp)->revertTo = unpack1(&rbp);
		/*
		if (rp->generic.length != 0) {
		    Length_Error(max(bytes_there,calculated_length<<2),client,rp,type,"GetDeviceFocus",0);
		    break;
		}
		*/
		break;
	case X_GetFeedbackControl:
		((xGetFeedbackControlReply *)rp)->num_feedbacks = unpack2(&rbp,needswap);
		rbp += 22;
		Unpack_Longs((long *) valuePtr, &rbp,
		    ((xGetFeedbackControlReply *)rp)->length,needswap);
		break;
	case X_GetDeviceKeyMapping:
/*
 *	Can't validate length - depends on value in original request
 */
		((xGetDeviceKeyMappingReply *)rp)->keySymsPerKeyCode = unpack1(&rbp);
		rbp += 23;
		Unpack_Longs((long *) valuePtr, &rbp,
		    ((xGetDeviceModifierMappingReply *)rp)->length,needswap);
		break;
	case X_GetDeviceModifierMapping:
		((xGetDeviceModifierMappingReply *)rp)->numKeyPerModifier = unpack1(&rbp);
		calculated_length = ((xGetDeviceModifierMappingReply *)rp)->numKeyPerModifier * 2;
		if (rp->generic.length != calculated_length) {
		    Length_Error(max(bytes_there,calculated_length<<2),client,rp,type,"GetDeviceModifierMapping",calculated_length);
		    break;
		}
		rbp += 23;
		Unpack_Longs((long *) valuePtr, &rbp,
		    ((xGetDeviceModifierMappingReply *)rp)->length,needswap);
		break;
	case X_SetDeviceModifierMapping:
		if (rp->generic.length != 0) {
		    Length_Error(max(bytes_there,calculated_length<<2),client,rp,type,"SetDeviceModifierMapping",0);
		    break;
		}
		break;
	case X_GetDeviceButtonMapping:
		((xGetDeviceButtonMappingReply *)rp)->nElts = unpack1(&rbp);
		calculated_length =
		(((xGetDeviceButtonMappingReply *)rp)->nElts + 3) /4;
		if (rp->generic.length != calculated_length) {
		    Length_Error(max(bytes_there,calculated_length<<2),client,rp,type,"GetDeviceButtonMapping",calculated_length);
		    break;
		}
		rbp += 23;
		bcopy(rbp,valuePtr,((xGetDeviceButtonMappingReply *)rp)->nElts);
		break;
	case X_SetDeviceButtonMapping:
		((xSetDeviceButtonMappingReply *)rp)->status = unpack1(&rbp);
		if (rp->generic.length != 0) {
		    Length_Error(max(bytes_there,calculated_length<<2),client,rp,type,"SetDeviceButtonMapping",calculated_length);
		    break;
		}
		break;
	case X_QueryDeviceState:
		((xQueryDeviceStateReply *)rp)->num_classes = unpack1(&rbp);
		/*
		if (rp->generic.length != 0) {
		    Length_Error(max(bytes_there,calculated_length<<2),client,rp,type,"SetDeviceButtonMapping",calculated_length);
		    break;
		}
		*/
		rbp += 23;
		bcopy(rbp,valuePtr,((xGetDeviceButtonMappingReply *)rp)->nElts);
		break;
	case X_SetDeviceValuators:
		((xSetDeviceValuatorsReply *)rp)->status = unpack1(&rbp);
		if (rp->generic.length != 0) {
		    Length_Error(max(bytes_there,calculated_length<<2),client,rp,type,"SetDeviceValuators",calculated_length);
		    break;
		}
		break;
	case X_GetDeviceControl:
		((xGetDeviceControlReply *)rp)->status = unpack1(&rbp);
		/*
		((xDeviceResolutionState *)((xGetDeviceControl *)rp+1))->length=
			unpack1(
			*/
		break;
	case X_ChangeDeviceControl:
		((xChangeDeviceControlReply *)rp)->status = unpack1(&rbp);
		if (rp->generic.length != 0) {
		    Length_Error(max(bytes_there,calculated_length<<2),client,rp,type,"ChangeDeviceControl",calculated_length);
		    break;
		}
		break;
	default:
		/* we got a reply to a request that didn't expect one. Assume
		   sequence numbers will be enough to sort out that this is an
		   error so return 1.
		*/
		Log_Trace("Reply unexpected for request type %d\n", type);
		break;
	}
	}
	else	{
		Log_Trace("Reply From unsupported extension %d\n", extension);
	}
	return(valid);
}

static void
Length_Error(bytes_needed,client,rp,type,label,calc)
unsigned long bytes_needed;
int client;
xReply *rp;
int type;
char *label;
int calc;
{
    Log_Msg("Rcv_Rep: BAD LENGTH ERROR!!!\n");
    Log_Msg("\treply = %s\n",label);
    Log_Msg("\tlength is %d, should be %d\n",rp->generic.length,calc);
    Show_Ext_Rep(rp,type, bytes_needed);
    Finish(client);
}

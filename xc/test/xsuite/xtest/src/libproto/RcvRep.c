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

 *
 * Copyright 1990, 1991 by UniSoft Group Limited.
 * 
 * Permission to use, copy, modify, distribute, and sell this software and
 * its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation, and that the name of UniSoft not be
 * used in advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.  UniSoft
 * makes no representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 *
 * $XConsortium: RcvRep.c,v 1.12 94/04/17 21:01:20 rws Exp $
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

#include "XstlibInt.h"
#ifdef Xpi
#include "xtestext1.h"
#endif
#include "DataMove.h"

#define REPLY_HEADER	8	/* number of bytes */

static void Length_Error();

int
Rcv_Rep(rp,rbuf,type,client)
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
	char *rbp = (char *) ((char *)rbuf + REPLY_HEADER);
	unsigned char *valuePtr = (unsigned char *) ((unsigned char *)rp +
	    sizeof(xReply));
	int i;
	int nlen;
	int valid = 1;		/* assume all is OK */
	int nitems;
	int calculated_length = 0;
	unsigned long bytes_there = (long)(rp->generic.length<<2) + sizeof(xReply);

	Log_Debug2("Rcv_Rep(): type = %d, length = %d\n", type, rp->generic.length);
	if (type > X_NoOperation) {
	    Rcv_Ext_Rep(rp,rbuf,((rp->generic.data1<<8) | type),client);
	    return(valid);
	}
	switch (type) {
	case X_GetWindowAttributes:
		if (rp->generic.length != 3) {
		    Length_Error(max(bytes_there,calculated_length<<2),client,rp,type,"GetWindowAttributes",3);
		    break;
		}
		((xGetWindowAttributesReply *)rp)->visualID = unpack4(&rbp,needswap);
		((xGetWindowAttributesReply *)rp)->class = unpack2(&rbp,needswap);
		((xGetWindowAttributesReply *)rp)->bitGravity = unpack1(&rbp);
		((xGetWindowAttributesReply *)rp)->winGravity = unpack1(&rbp);
		((xGetWindowAttributesReply *)rp)->backingBitPlanes = unpack4(&rbp,needswap);
		((xGetWindowAttributesReply *)rp)->backingPixel = unpack4(&rbp,needswap);
		((xGetWindowAttributesReply *)rp)->saveUnder = unpack1(&rbp);
		((xGetWindowAttributesReply *)rp)->mapInstalled = unpack1(&rbp);
		((xGetWindowAttributesReply *)rp)->mapState = unpack1(&rbp);
		((xGetWindowAttributesReply *)rp)->override = unpack1(&rbp);
		((xGetWindowAttributesReply *)rp)->colormap = unpack4(&rbp,needswap);
		((xGetWindowAttributesReply *)rp)->allEventMasks = unpack4(&rbp,needswap);
		((xGetWindowAttributesReply *)rp)->yourEventMask = unpack4(&rbp,needswap);
		((xGetWindowAttributesReply *)rp)->doNotPropagateMask = unpack2(&rbp,needswap);
		break;
	case X_GetGeometry:
		if (rp->generic.length != 0) {
		    Length_Error(max(bytes_there,calculated_length<<2),client,rp,type,"GetGeometry",0);
		    break;
		}
		((xGetGeometryReply *)rp)->root = unpack4(&rbp,needswap);
		((xGetGeometryReply *)rp)->x = unpack2(&rbp,needswap);
		((xGetGeometryReply *)rp)->y = unpack2(&rbp,needswap);
		((xGetGeometryReply *)rp)->width = unpack2(&rbp,needswap);
		((xGetGeometryReply *)rp)->height = unpack2(&rbp,needswap);
		((xGetGeometryReply *)rp)->borderWidth = unpack2(&rbp,needswap);
		break;
	case X_QueryTree:
		((xQueryTreeReply *)rp)->root = unpack4(&rbp,needswap);
		((xQueryTreeReply *)rp)->parent = unpack4(&rbp,needswap);
		((xQueryTreeReply *)rp)->nChildren = unpack2(&rbp,needswap);
		nitems = ((xQueryTreeReply *)rp)->nChildren;
		calculated_length = nitems;
		if (rp->generic.length != calculated_length) {
		    Length_Error(max(bytes_there,calculated_length<<2),client,rp,type,"QueryTree",calculated_length);
		    break;
		}
		rbp += 14;
		Unpack_Longs((long *) valuePtr, &rbp,
		    ((xQueryTreeReply *)rp)->nChildren,needswap);
		break;
	case X_InternAtom:
		if (rp->generic.length != 0) {
		    Length_Error(max(bytes_there,calculated_length<<2),client,rp,type,"InternAtom",0);
		    break;
		}
		((xInternAtomReply *)rp)->atom = unpack4(&rbp,needswap);
		break;
	case X_GetAtomName:
		((xGetAtomNameReply *)rp)->nameLength = unpack2(&rbp,needswap);
		nitems = ((xGetAtomNameReply *)rp)->nameLength;
		calculated_length = (nitems + 3) / 4;
		if (rp->generic.length != calculated_length) {
		    Length_Error(max(bytes_there,calculated_length<<2),client,rp,type,"GetAtomName",calculated_length);
		    break;
		}
		rbp += 22;
		bcopy(rbp,valuePtr,((xGetAtomNameReply *)rp)->nameLength);
		break;
	case X_GetProperty:
		((xGetPropertyReply *)rp)->type = unpack4(&rbp,needswap);
		((xGetPropertyReply *)rp)->bytesAfter = unpack4(&rbp,needswap);
		((xGetPropertyReply *)rp)->nItems = unpack4(&rbp,needswap);
		nitems = ((xGetPropertyReply *)rp)->nItems;
		rbp += 12;
		switch (((xGetPropertyReply *)rp)->format) {
		case 0:
		    if (rp->generic.length != 0) {
			Length_Error(max(bytes_there,calculated_length<<2),client,rp,type,"GetProperty",0);
			break;
		    }
		    break;
		case FORMAT8:
		    calculated_length = (nitems + 3) / 4;
		    if (rp->generic.length != calculated_length) {
			Length_Error(max(bytes_there,calculated_length<<2),client,rp,type,"GetProperty",calculated_length);
			break;
		    }
		    bcopy(rbp,valuePtr,nitems);
		    break;
		case FORMAT16:
		    calculated_length = (nitems + 1) >> 1;
		    if (rp->generic.length != calculated_length) {
			Length_Error(max(bytes_there,calculated_length<<2),client,rp,type,"GetProperty",calculated_length);
			break;
		    }
		    Unpack_Shorts((unsigned short *) valuePtr, &rbp,nitems,needswap);
		    break;
		case FORMAT32:
		    calculated_length = nitems;
		    if (rp->generic.length != calculated_length) {
			Length_Error(max(bytes_there,calculated_length<<2),client,rp,type,"GetProperty",calculated_length);
			break;
		    }
		    Unpack_Longs((unsigned long *) valuePtr, &rbp,nitems,needswap);
		    break;
		default:
		    Log_Err("Rcv_Rep: bad format field in GetPropertyReply\n");
		    valid = 0;
		    break;
		}
		break;
	case X_ListProperties:
		((xListPropertiesReply *)rp)->nProperties = unpack2(&rbp,needswap);
		nitems = ((xListPropertiesReply *)rp)->nProperties;
		calculated_length = nitems;
		if (rp->generic.length != calculated_length) {
		    Length_Error(max(bytes_there,calculated_length<<2),client,rp,type,"ListProperties",calculated_length);
		    break;
		}
		rbp += 22;
		Unpack_Longs((unsigned long *) valuePtr, &rbp,
		    ((xListPropertiesReply *)rp)->nProperties,needswap);
		break;
	case X_GetSelectionOwner:
		if (rp->generic.length != 0) {
		    Length_Error(max(bytes_there,calculated_length<<2),client,rp,type,"GetSelectionOwner",0);
		    break;
		}
		((xGetSelectionOwnerReply *)rp)->owner = unpack4(&rbp,needswap);
		break;
	case X_GrabPointer:
		if (rp->generic.length != 0) {
		    Length_Error(max(bytes_there,calculated_length<<2),client,rp,type,"GrabPointer",0);
		    break;
		}
		break;
	case X_GrabKeyboard:
		if (rp->generic.length != 0) {
		    Length_Error(max(bytes_there,calculated_length<<2),client,rp,type,"GrabKeyboard",0);
		    break;
		}
		break;
	case X_QueryPointer:
		if (rp->generic.length != 0) {
		    Length_Error(max(bytes_there,calculated_length<<2),client,rp,type,"QueryPointer",0);
		    break;
		}
		((xQueryPointerReply *)rp)->root = unpack4(&rbp,needswap);
		((xQueryPointerReply *)rp)->child = unpack4(&rbp,needswap);
		((xQueryPointerReply *)rp)->rootX = unpack2(&rbp,needswap);
		((xQueryPointerReply *)rp)->rootY = unpack2(&rbp,needswap);
		((xQueryPointerReply *)rp)->winX = unpack2(&rbp,needswap);
		((xQueryPointerReply *)rp)->winY = unpack2(&rbp,needswap);
		((xQueryPointerReply *)rp)->mask = unpack2(&rbp,needswap);
		break;
	case X_GetMotionEvents:
		((xGetMotionEventsReply *)rp)->nEvents = unpack4(&rbp,needswap);
		nitems = ((xGetMotionEventsReply *)rp)->nEvents;
		calculated_length = nitems * 2;
		if (rp->generic.length != calculated_length) {
		    Length_Error(max(bytes_there,calculated_length<<2),client,rp,type,"GetProperty",calculated_length);
		    break;
		}
		rbp += 20;
		/*
		 *	Extract timecoords from reply -
		 *		CARD32 time;
		 *		CARD16 x, y;
		 */
		for(i=0;i<((xGetMotionEventsReply *)rp)->nEvents;i++) {
		    *((unsigned long *) valuePtr) = unpack4(&rbp,needswap);
		    valuePtr += 4;
		    *((unsigned short *) valuePtr) = unpack2(&rbp,needswap);
		    valuePtr += 2;
		    *((unsigned short *) valuePtr) = unpack2(&rbp,needswap);
		    valuePtr += 2;
		}
		break;
	case X_TranslateCoords:
		if (rp->generic.length != 0) {
		    Length_Error(max(bytes_there,calculated_length<<2),client,rp,type,"TranslateCoords",0);
		    break;
		}
		((xTranslateCoordsReply *)rp)->child = unpack4(&rbp,needswap);
		((xTranslateCoordsReply *)rp)->dstX = unpack2(&rbp,needswap);
		((xTranslateCoordsReply *)rp)->dstY = unpack2(&rbp,needswap);
		break;
	case X_GetInputFocus:
		if (rp->generic.length != 0) {
		    Length_Error(max(bytes_there,calculated_length<<2),client,rp,type,"GetInputFocus",0);
		    break;
		}
		((xGetInputFocusReply *)rp)->focus = unpack4(&rbp,needswap);
		break;
	case X_QueryKeymap:
		if (rp->generic.length != 2) {
		    Length_Error(max(bytes_there,calculated_length<<2),client,rp,type,"QueryKeymap",2);
		    break;
		}
		valuePtr = (unsigned char *) ((unsigned char *) rp +
		    (rbp - rbuf));
		bcopy(rbp,valuePtr, 32);
		break;
	case X_QueryFont:
		valuePtr =  (unsigned char *) &(((xQueryFontReply *)rp)->minBounds);
		Unpack_Shorts((unsigned short *) valuePtr, &rbp, 6, needswap);
		rbp += 4;
		valuePtr =  (unsigned char *) &(((xQueryFontReply *)rp)->maxBounds);
		Unpack_Shorts((unsigned short *) valuePtr, &rbp, 6, needswap);
		rbp += 4;
		((xQueryFontReply *)rp)->minCharOrByte2 = unpack2(&rbp,needswap);
		((xQueryFontReply *)rp)->maxCharOrByte2 = unpack2(&rbp,needswap);
		((xQueryFontReply *)rp)->defaultChar = unpack2(&rbp,needswap);
		((xQueryFontReply *)rp)->nFontProps = unpack2(&rbp,needswap);
		((xQueryFontReply *)rp)->drawDirection = unpack1(&rbp);
		((xQueryFontReply *)rp)->minByte1 = unpack1(&rbp);
		((xQueryFontReply *)rp)->maxByte1 = unpack1(&rbp);
		((xQueryFontReply *)rp)->allCharsExist = unpack1(&rbp);
		((xQueryFontReply *)rp)->fontAscent = unpack2(&rbp,needswap);
		((xQueryFontReply *)rp)->fontDescent = unpack2(&rbp,needswap);
		((xQueryFontReply *)rp)->nCharInfos = unpack4(&rbp,needswap);
		calculated_length = 7 +
		    (2 * ((xQueryFontReply *)rp)->nFontProps) +
		    (3 * ((xQueryFontReply *)rp)->nCharInfos);
		if (rp->generic.length != calculated_length) {
		    Length_Error(max(bytes_there,calculated_length<<2),client,rp,type,"QueryFont",calculated_length);
		    break;
		}
		valuePtr = (unsigned char *) ((unsigned char *) rp + sizeof(xQueryFontReply));
		Unpack_Longs((unsigned long *) valuePtr, &rbp, 
		    ((xQueryFontReply *)rp)->nFontProps * 2,needswap);
		valuePtr += (((xQueryFontReply *)rp)->nFontProps * 2 * 4);
		Unpack_Shorts((unsigned long *) valuePtr, &rbp, 
		    (((xQueryFontReply *)rp)->nCharInfos) * 6,needswap);
		break;
	case X_QueryTextExtents:
		if (rp->generic.length != 0) {
		    Length_Error(max(bytes_there,calculated_length<<2),client,rp,type,"QueryFontExtents",0);
		    break;
		}
		((xQueryTextExtentsReply *)rp)->fontAscent = unpack2(&rbp,needswap);
		((xQueryTextExtentsReply *)rp)->fontDescent = unpack2(&rbp,needswap);
		((xQueryTextExtentsReply *)rp)->overallAscent = unpack2(&rbp,needswap);
		((xQueryTextExtentsReply *)rp)->overallDescent = unpack2(&rbp,needswap);
		((xQueryTextExtentsReply *)rp)->overallWidth = unpack4(&rbp,needswap);
		((xQueryTextExtentsReply *)rp)->overallLeft = unpack4(&rbp,needswap);
		((xQueryTextExtentsReply *)rp)->overallRight = unpack4(&rbp,needswap);
		break;
	case X_ListFonts:
		((xListFontsReply *)rp)->nFonts = unpack2(&rbp,needswap);
		nitems = ((xListFontsReply *)rp)->nFonts;
		rbp += 22;
		calculated_length = 0;
		for(i=0;i<nitems;i++) {
		    nlen = *rbp;
		    calculated_length += (nlen + 1);	/* # chars + 1 for len */
		    if (calculated_length > (rp->generic.length * 4)) {
			Log_Msg("Rcv_Rep: BAD LENGTH ERROR!!!\n");
			Log_Msg("\treply = %s\n","ListFonts");
			Log_Msg("\tlength is %d, cumulative for %d out of %d names = %d\n",
			    rp->generic.length,i+1,nitems,(calculated_length+3)/4);
			Show_Rep(rp,X_ListFonts,calculated_length<<2);
			Finish(client);
		    }
		    *valuePtr++ = *rbp++;
		    bcopy(rbp,valuePtr,nlen);
		    rbp += nlen;
		    valuePtr += nlen;
		}
		calculated_length = (calculated_length + 3) / 4;
		if (rp->generic.length != calculated_length) {
		    Length_Error(max(bytes_there,calculated_length<<2),client,rp,type,"ListFonts",calculated_length);
		    break;
		}
		break;
	case X_ListFontsWithInfo:
		valuePtr =  (unsigned char *) &(((xListFontsWithInfoReply *)rp)->minBounds);
		Unpack_Shorts((unsigned short *) valuePtr, &rbp, 6, needswap);
		rbp += 4;
		valuePtr =  (unsigned char *) &(((xListFontsWithInfoReply *)rp)->maxBounds);
		Unpack_Shorts((unsigned short *) valuePtr, &rbp, 6, needswap);
		rbp += 4;
		((xListFontsWithInfoReply *)rp)->minCharOrByte2 = unpack2(&rbp,needswap);
		((xListFontsWithInfoReply *)rp)->maxCharOrByte2 = unpack2(&rbp,needswap);
		((xListFontsWithInfoReply *)rp)->defaultChar = unpack2(&rbp,needswap);
		((xListFontsWithInfoReply *)rp)->nFontProps = unpack2(&rbp,needswap);
		((xListFontsWithInfoReply *)rp)->drawDirection = unpack1(&rbp);
		((xListFontsWithInfoReply *)rp)->minByte1 = unpack1(&rbp);
		((xListFontsWithInfoReply *)rp)->maxByte1 = unpack1(&rbp);
		((xListFontsWithInfoReply *)rp)->allCharsExist = unpack1(&rbp);
		((xListFontsWithInfoReply *)rp)->fontAscent = unpack2(&rbp,needswap);
		((xListFontsWithInfoReply *)rp)->fontDescent = unpack2(&rbp,needswap);
		((xListFontsWithInfoReply *)rp)->nReplies = unpack4(&rbp,needswap);
		calculated_length = 7 +
		    (2 * ((xQueryFontReply *)rp)->nFontProps) +
		    ((int)(rp->generic.data1 + 3) / 4);
		if (rp->generic.length != calculated_length) {
		    Length_Error(max(bytes_there,calculated_length<<2),client,rp,type,"ListFontsWithInfo",calculated_length);
		    break;
		}
		valuePtr = (unsigned char *) (((unsigned char *) rp) + sizeof(xListFontsWithInfoReply));
		Unpack_Longs((unsigned long *) valuePtr, &rbp, 
		    (((xListFontsWithInfoReply *)rp)->nFontProps) * 2,needswap);
		valuePtr += (((xListFontsWithInfoReply *)rp)->nFontProps * 2 * 4);
		bcopy(rbp,valuePtr,((xListFontsWithInfoReply *)rp)->nameLength);
		Show_Rep(rp,X_ListFontsWithInfo,bytes_there);
		break;
	case X_GetFontPath:
		((xGetFontPathReply *)rp)->nPaths = unpack2(&rbp,needswap);
		nitems = ((xGetFontPathReply *)rp)->nPaths;
		rbp += 22;
		calculated_length = 0;
		for(i=0;i<nitems;i++) {
		    nlen = *rbp;
		    calculated_length += nlen + 1;
		    if (calculated_length > (rp->generic.length * 4)) {
			Log_Msg("Rcv_Rep: BAD LENGTH ERROR!!!\n");
			Log_Msg("\treply = %s\n","GetFontPath");
			Log_Msg("\tlength is %d, cumulative for %d out of %d names = %d\n",
			    rp->generic.length,i+1,nitems,(calculated_length+3)/4);
			Show_Rep(rp,X_GetFontPath,calculated_length<<2);
			Finish(client);
		    }
		    *valuePtr++ = *rbp++;
		    bcopy(rbp,valuePtr,nlen);
		    rbp += nlen;
		    valuePtr += nlen;
		}
		calculated_length = (calculated_length + 3) / 4;
		if (rp->generic.length != calculated_length) {
		    Length_Error(max(bytes_there,calculated_length<<2),client,rp,type,"GetFontPath",calculated_length);
		    break;
		}
		break;
	case X_GetImage:
		{
/*
 *	Images are stored in the test programs in client byte order and
 *	unpadded.  This allows images to be independent of the server.
 *	However the server will send images in server byte order and 
 *	padded.  This routine unpacks from server form into client-normal
 *	form.  Note that we're assuming client-normal images are padded to
 *	byte boundary; otherwise the translation is more complicated.
 *	Similarly, left-pad must be zero.
 */

		int row, col = 1;
		unsigned char my_sex = *((unsigned char *) &col) ^ 1;
		unsigned char server_sex =
			(Xst_clients[client].cl_dpy) -> byte_order;
		long flip = my_sex ^ server_sex;  /* assume MSBFirst == 1 */
		int server_pad = (Xst_clients[client].cl_dpy) -> bitmap_pad;
		int server_unit = (Xst_clients[client].cl_dpy) -> bitmap_unit;
		int server_bitorder =
			(Xst_clients[client].cl_dpy) -> bitmap_bit_order;
		int dst_width /*in bytes*/ =
			(Xst_clients[client].cl_imagewidth + 7) >> 3;
		int src_width /*in bytes*/ = dst_width +
			((dst_width % (server_pad>>3)) == 0 ? 0 :
			 (server_pad>>3) - dst_width % (server_pad>>3));
			
		char *dst = (char *)rp + sizeof(xReply);

		((xGetImageReply *)rp)->visual = unpack4(&rbp,needswap);
		rbp += 20;

                calculated_length =
			(src_width * Xst_clients[client].cl_imageheight) >> 2;
                if (rp->generic.length != calculated_length) {
                    Length_Error(max(bytes_there,calculated_length<<2),client,rp,type,"GetImage",calculated_length);
	                    break;
                }
/*****
		if (server_bitorder != MSBFirst) {
			Log_Err("LSBFirst bit ordering not supported in Rcv_Rep()\n");
			Finish(client);
		}
*****/

		rp->generic.length =
			(dst_width * Xst_clients[client].cl_imageheight) >> 2;

		for (row = 0; row < Xst_clients[client].cl_imageheight; row++)
			for(col = 0; col < src_width; col++)

				if (col < dst_width)  {
				    *(dst++) = *((char *)((long)rbp++ ^ flip));
				}  else  {
					rbp++;
				}
	        }
		break;
	case X_ListInstalledColormaps:
		((xListInstalledColormapsReply *)rp)->nColormaps = unpack2(&rbp,needswap);
		calculated_length = ((xListInstalledColormapsReply *)rp)->nColormaps;
		if (rp->generic.length != calculated_length) {
		    Length_Error(max(bytes_there,calculated_length<<2),client,rp,type,"ListInstalledColormaps",calculated_length);
		    break;
		}
		rbp += 22;
		Unpack_Longs((long *) valuePtr, &rbp,
			     ((xListInstalledColormapsReply *)rp)->nColormaps,
			     needswap);
		break;
	case X_AllocColor:
		if (rp->generic.length != 0) {
		    Length_Error(max(bytes_there,calculated_length<<2),client,rp,type,"AllocColor",0);
		    break;
		}
		((xAllocColorReply *)rp)->red = unpack2(&rbp,needswap);
		((xAllocColorReply *)rp)->green = unpack2(&rbp,needswap);
		((xAllocColorReply *)rp)->blue = unpack2(&rbp,needswap);
		rbp += 2;
		((xAllocColorReply *)rp)->pixel = unpack4(&rbp,needswap);
		break;
	case X_AllocNamedColor:
		if (rp->generic.length != 0) {
		    Length_Error(max(bytes_there,calculated_length<<2),client,rp,type,"AllocNamedColor",0);
		    break;
		}
		((xAllocNamedColorReply *)rp)->pixel = unpack4(&rbp,needswap);
		((xAllocNamedColorReply *)rp)->exactRed = unpack2(&rbp,needswap);
		((xAllocNamedColorReply *)rp)->exactGreen = unpack2(&rbp,needswap);
		((xAllocNamedColorReply *)rp)->exactBlue = unpack2(&rbp,needswap);
		((xAllocNamedColorReply *)rp)->screenRed = unpack2(&rbp,needswap);
		((xAllocNamedColorReply *)rp)->screenGreen = unpack2(&rbp,needswap);
		((xAllocNamedColorReply *)rp)->screenBlue = unpack2(&rbp,needswap);
		break;
	case X_AllocColorCells:
		((xAllocColorCellsReply *)rp)->nPixels = unpack2(&rbp,needswap);
		((xAllocColorCellsReply *)rp)->nMasks = unpack2(&rbp,needswap);
		calculated_length =  ((xAllocColorCellsReply *)rp)->nPixels +
		    ((xAllocColorCellsReply *)rp)->nMasks;
		if (rp->generic.length != calculated_length) {
		    Length_Error(max(bytes_there,calculated_length<<2),client,rp,type,"AllocColorCells",calculated_length);
		    break;
		}
		rbp += 20;
		Unpack_Longs((long *) valuePtr, &rbp,
			     ((xAllocColorCellsReply *)rp)->nPixels,
			     needswap);
		Unpack_Longs((long *) valuePtr, &rbp,
			     ((xAllocColorCellsReply *)rp)->nMasks,
			     needswap);
		break;
	case X_AllocColorPlanes:
		((xAllocColorPlanesReply *)rp)->nPixels = unpack2(&rbp,needswap);
		calculated_length = ((xAllocColorPlanesReply *)rp)->nPixels;
		if (rp->generic.length != calculated_length) {
		    Length_Error(max(bytes_there,calculated_length<<2),client,rp,type,"AllocColorPlanes",calculated_length);
		    break;
		}
		rbp += 2;
		((xAllocColorPlanesReply *)rp)->redMask = unpack4(&rbp,needswap);
		((xAllocColorPlanesReply *)rp)->greenMask = unpack4(&rbp,needswap);
		((xAllocColorPlanesReply *)rp)->blueMask = unpack4(&rbp,needswap);
		rbp += 8;
		Unpack_Longs((long *) valuePtr, &rbp,
			     ((xAllocColorPlanesReply *)rp)->nPixels,
			     needswap);
		break;
	case X_QueryColors:
		((xQueryColorsReply *)rp)->nColors = unpack2(&rbp,needswap);
		calculated_length = ((xQueryColorsReply *)rp)->nColors * 2;
		if (rp->generic.length != calculated_length) {
		    Length_Error(max(bytes_there,calculated_length<<2),client,rp,type,"QueryColors",calculated_length);
		    break;
		}
		rbp += 22;
		Unpack_Shorts((long *) valuePtr, &rbp, ((xQueryColorsReply *)rp)->nColors * 4, needswap); 
		break;
	case X_LookupColor:
		if (rp->generic.length != 0) {
		    Length_Error(max(bytes_there,calculated_length<<2),client,rp,type,"LookupColor",0);
		    break;
		}
		((xLookupColorReply *)rp)->exactRed = unpack2(&rbp,needswap);
		((xLookupColorReply *)rp)->exactGreen = unpack2(&rbp,needswap);
		((xLookupColorReply *)rp)->exactBlue = unpack2(&rbp,needswap);
		((xLookupColorReply *)rp)->screenRed = unpack2(&rbp,needswap);
		((xLookupColorReply *)rp)->screenGreen = unpack2(&rbp,needswap);
		((xLookupColorReply *)rp)->screenBlue = unpack2(&rbp,needswap);
		break;
	case X_QueryBestSize:
		if (rp->generic.length != 0) {
		    Length_Error(max(bytes_there,calculated_length<<2),client,rp,type,"QueryBestSize",0);
		    break;
		}
		((xQueryBestSizeReply *)rp)->width = unpack2(&rbp,needswap);
		((xQueryBestSizeReply *)rp)->height = unpack2(&rbp,needswap);
		break;
	case X_QueryExtension:
		if (rp->generic.length != 0) {
		    Length_Error(max(bytes_there,calculated_length<<2),client,rp,type,"QueryExtension",0);
		    break;
		}
		((xQueryExtensionReply *)rp)->present = unpack1(&rbp);
		((xQueryExtensionReply *)rp)->major_opcode = unpack1(&rbp);
		((xQueryExtensionReply *)rp)->first_event = unpack1(&rbp);
		((xQueryExtensionReply *)rp)->first_error = unpack1(&rbp);
		break;
	case X_ListExtensions: {
	        /* rp->generic.data1 is the number of extension strings in
		   the returned list of strings. */
	        int value_len = 0;       /* total bytes in returned value */
		char *buf_ptr = rbuf + sizeof(xReply);
		int nchars;

		for (i = 0; i < (int)rp->generic.data1; i++) {
		    nchars = *buf_ptr++;
		    value_len += nchars + 1; /* characters plus count */
		    buf_ptr += nchars;
		}

		calculated_length = (value_len + 3) / 4;
		if (rp->generic.length != calculated_length) {
		    Length_Error(max(bytes_there,calculated_length<<2),client,rp,type,"ListExtensions",calculated_length);
		    break;
		}
		bcopy(rbuf + 32, valuePtr, calculated_length << 2);
		break;
	    }
	case X_GetKeyboardMapping:
/*
 *	Can't validate length - depends on value in original request
 */
		rbp += 24;
		Unpack_Longs((long *) valuePtr, &rbp,
		    ((xGetKeyboardMappingReply *)rp)->length,needswap);
		break;
	case X_GetKeyboardControl:
		if (rp->generic.length != 5) {
		    Length_Error(max(bytes_there,calculated_length<<2),client,rp,type,"GetKeyboardControl",5);
		    break;
		}
		((xGetKeyboardControlReply *)rp)->ledMask = unpack4(&rbp,needswap);
		((xGetKeyboardControlReply *)rp)->keyClickPercent = unpack1(&rbp);
		((xGetKeyboardControlReply *)rp)->bellPercent = unpack1(&rbp);
		((xGetKeyboardControlReply *)rp)->bellPitch = unpack2(&rbp,needswap);
		((xGetKeyboardControlReply *)rp)->bellDuration = unpack2(&rbp,needswap);
		rbp += 2;
		valuePtr = (unsigned char *) ((unsigned char *) rp +
		    (rbp - rbuf));
		bcopy(rbp,valuePtr,32);
		break;
	case X_GetPointerControl:
		if (rp->generic.length != 0) {
		    Length_Error(max(bytes_there,calculated_length<<2),client,rp,type,"GetPointerControl",0);
		    break;
		}
		((xGetPointerControlReply *)rp)->accelNumerator = unpack2(&rbp,needswap);
		((xGetPointerControlReply *)rp)->accelDenominator = unpack2(&rbp,needswap);
		((xGetPointerControlReply *)rp)->threshold = unpack2(&rbp,needswap);
		break;
	case X_GetScreenSaver:
		if (rp->generic.length != 0) {
		    Length_Error(max(bytes_there,calculated_length<<2),client,rp,type,"GetScreenSaver",0);
		    break;
		}
		((xGetScreenSaverReply *)rp)->timeout = unpack2(&rbp,needswap);
		((xGetScreenSaverReply *)rp)->interval = unpack2(&rbp,needswap);
		((xGetScreenSaverReply *)rp)->preferBlanking = unpack1(&rbp);
		((xGetScreenSaverReply *)rp)->allowExposures = unpack1(&rbp);
		break;
	case X_ListHosts:
		{
		    unsigned short nhosts;
		    unsigned short hlen;
		    unsigned char *endrpPtr = ((unsigned char *)rp) + bytes_there;
		    char *endrbPtr = ((char *)rbuf) + bytes_there;

		    nhosts = unpack2(&rbp,needswap);
		    ((xListHostsReply *)rp)->nHosts = nhosts;
		    calculated_length = (int)nhosts * 4 / 4; /* min poss. */
		    if (rp->generic.length < calculated_length) {
			Length_Error(max(bytes_there,calculated_length<<2),client,rp,type,"ListHosts",calculated_length);
			break;
		    }
		    rbp += 22;
		    calculated_length = 0;
		    for(i=0;i<(int)nhosts;i++) {
			calculated_length++;
			if ((rbp+4) > endrbPtr || (valuePtr+4) > endrpPtr) {
				Length_Error(bytes_there,
					client,rp,type,"ListHosts",
					calculated_length);
				break;
			}
			*valuePtr++ = *rbp++;	/* family */

			valuePtr++;
			rbp++;			/* pad */

			hlen = unpack2(&rbp,needswap);
			*((unsigned short *) valuePtr) = hlen;
			valuePtr += 2;
			calculated_length += padup((int)hlen)/4;
			if ((rbp+hlen) > endrbPtr || (valuePtr+hlen) > endrpPtr) {
				Length_Error(bytes_there,
					client,rp,type,"ListHosts",
					calculated_length);
				break;
			}
			bcopy(rbp,valuePtr,hlen);
			rbp += padup((int)hlen);
			valuePtr += padup((int)hlen);
		    }
		    if (i < (int)nhosts) {
			Log_Msg("Incomplete ListHosts reply: %d hosts found instead of %d\n", i, nhosts);
			Finish(client);
			/*NOTREACHED*/
		    }
		}
		break;
	case X_SetPointerMapping:
		if (rp->generic.length != 0) {
		    Length_Error(max(bytes_there,calculated_length<<2),client,rp,type,"SetPointerMapping",0);
		    break;
		}
		break;
	case X_GetPointerMapping:
		calculated_length = (int)(rp->generic.data1 + 3) / 4;
		if (rp->generic.length != calculated_length) {
		    Length_Error(max(bytes_there,calculated_length<<2),client,rp,type,"GetPointerMapping",calculated_length);
		    break;
		}
		rbp += 24;
		bcopy(rbp,valuePtr,rp->generic.data1);
		break;
	case X_SetModifierMapping:
		if (rp->generic.length != 0) {
		    Length_Error(max(bytes_there,calculated_length<<2),client,rp,type,"SetModifierMapping",0);
		    break;
		}
		break;
	case X_GetModifierMapping:
		calculated_length = rp->generic.data1 * 2;
		if (rp->generic.length != calculated_length) {
		    Length_Error(max(bytes_there,calculated_length<<2),client,rp,type,"GetModifierMapping",calculated_length);
		    break;
		}
		rbp += 24;
		Unpack_Longs((long *) valuePtr, &rbp,
		    ((xGetModifierMappingReply *)rp)->length,needswap);
		break;
	default:
		/* we got a reply to a request that didn't expect one. Assume
		   sequence numbers will be enough to sort out that this is an
		   error so return 1.
		*/
		Log_Trace("Reply unexpected for request type %d\n", type);
		break;
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
    Show_Rep(rp,type, bytes_needed);
    Finish(client);
}

/* $XFree86: xc/programs/Xserver/hw/xfree86/accel/et4000w32/w32/w32stip.h,v 3.8 1996/12/23 06:35:27 dawes Exp $ */ 
/*******************************************************************************
                        Copyright 1994 by Glenn G. Lai

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its
documentation for any purpose and without fee is hereby granted,
provided that the above copyr notice appear in all copies and that
both that copyr notice and this permission notice appear in
supporting documentation, and that the name of Glenn G. Lai not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.

Glenn G. Lai DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

Glenn G. Lai
P.O. Box 4314
Austin, Tx 78765
glenn@cs.utexas.edu)
10/1/94
*******************************************************************************/
/* $XConsortium: w32stip.h /main/8 1996/10/25 10:21:53 kaleb $ */

#ifndef W32_STIP_H
#define W32_STIP_H

#include "w32.h"


#define W32_INIT_OPAQUE_STIPPLE(OP, FOREGROUND, BACKGROUND, DST_OFFSET) \
{ \
    *ACL_FOREGROUND_RASTER_OPERATION 	= W32OpTable[OP]; \
    *ACL_BACKGROUND_RASTER_OPERATION	= W32PatternOpTable[OP]; \
    *ACL_ROUTING_CONTROL		= 0x2; \
    *ACL_XY_DIRECTION			= 0; \
    *ACL_DESTINATION_Y_OFFSET		= byteswap16(DST_OFFSET); \
    *ACL_VIRTUAL_BUS_SIZE		= 0x0; \
    *ACL_SOURCE_WRAP			= 0x12; \
    *ACL_SOURCE_Y_OFFSET		= BYTESWAP16(0x3); \
    *ACL_SOURCE_ADDRESS			= byteswap32(W32Foreground); \
    *MBP0 				= byteswap32(W32Foreground); \
    *(LongP)W32Buffer	 		= byteswap32(FOREGROUND); \
    *(LongP)(W32Buffer + 4) 		= byteswap32(FOREGROUND); \
    *ACL_PATTERN_WRAP			= 0x12; \
    *ACL_PATTERN_Y_OFFSET		= BYTESWAP16(0x3); \
    *ACL_PATTERN_ADDRESS		= byteswap32(W32Background); \
    *MBP0 				= byteswap32(W32Background); \
    *(LongP)W32Buffer	 		= byteswap32(BACKGROUND); \
    *(LongP)(W32Buffer + 4) 		= byteswap32(BACKGROUND); \
}


#define W32_INIT_TR_STIPPLE(OP, MASK, FOREGROUND, BACKGROUND, DST_OFFSET) \
{ \
    if (MASK == 0xffffffff) \
	*ACL_FOREGROUND_RASTER_OPERATION	= W32OpTable[OP]; \
    else \
    { \
	*ACL_FOREGROUND_RASTER_OPERATION= \
	    (0xf0 & W32OpTable[OP]) | 0x0a; \
	*ACL_PATTERN_WRAP		= 0x12; \
	*ACL_PATTERN_Y_OFFSET		= BYTESWAP16(0x3); \
	*ACL_PATTERN_ADDRESS		= byteswap32(W32Pattern); \
	*MBP0 				= byteswap32(W32Pattern); \
	*(LongP)W32Buffer 		= byteswap32(MASK); \
	*(LongP)(W32Buffer + 4) 	= byteswap32(MASK); \
    } \
    *ACL_BACKGROUND_RASTER_OPERATION	= 0xaa; \
    *ACL_ROUTING_CONTROL		= 0x2; \
    *ACL_XY_DIRECTION			= 0; \
    *ACL_DESTINATION_Y_OFFSET		= byteswap16(DST_OFFSET); \
    *ACL_VIRTUAL_BUS_SIZE		= 0x0; \
    *ACL_SOURCE_WRAP			= 0x12; \
    *ACL_SOURCE_Y_OFFSET		= BYTESWAP16(0x3); \
    *ACL_SOURCE_ADDRESS			= byteswap32(W32Foreground); \
    *MBP0 				= byteswap32(W32Foreground); \
    *(LongP)W32Buffer	 		= byteswap32(FOREGROUND); \
    *(LongP)(W32Buffer + 4) 		= byteswap32(FOREGROUND); \
}


#define W32P_INIT_OPAQUE_STIPPLE(OP, FOREGROUND, BACKGROUND, DST_OFFSET) \
{ \
    *ACL_FOREGROUND_RASTER_OPERATION 	= W32OpTable[OP]; \
    *ACL_BACKGROUND_RASTER_OPERATION	= W32PatternOpTable[OP]; \
    if (W32et6000) \
      *ACL_MIX_CONTROL			= 0x32; \
    else \
      *ACL_ROUTING_CONTROL		= 0x02; \
    *ACL_XY_DIRECTION			= 0; \
    *ACL_DESTINATION_Y_OFFSET		= byteswap16(DST_OFFSET); \
    *ACL_SOURCE_WRAP			= 0x02; \
    *ACL_SOURCE_Y_OFFSET		= BYTESWAP16(0x3); \
    *ACL_SOURCE_ADDRESS			= byteswap32(W32Foreground); \
    *MBP0 				= byteswap32(W32Foreground); \
    *(LongP)W32Buffer	 		= byteswap32(FOREGROUND); \
    *ACL_PATTERN_WRAP			= 0x02; \
    *ACL_PATTERN_Y_OFFSET		= BYTESWAP16(0x3); \
    *ACL_PATTERN_ADDRESS		= byteswap32(W32Pattern); \
    *MBP0 				= byteswap32(W32Pattern); \
    *(LongP)W32Buffer	 		= byteswap32(BACKGROUND); \
    *ACL_MIX_ADDRESS			= 0; \
    *ACL_MIX_Y_OFFSET 			= BYTESWAP16(31); \
}


#define W32P_INIT_TR_STIPPLE(OP, MASK, FOREGROUND, BACKGROUND, DST_OFFSET) \
{ \
    if (MASK == 0xffffffff) \
	*ACL_FOREGROUND_RASTER_OPERATION	= W32OpTable[OP]; \
    else \
    { \
	*ACL_FOREGROUND_RASTER_OPERATION= \
	    (0xf0 & W32OpTable[OP]) | 0x0a; \
	*ACL_PATTERN_WRAP		= 0x02; \
	*ACL_PATTERN_Y_OFFSET		= BYTESWAP16(0x3); \
	*ACL_PATTERN_ADDRESS		= byteswap32(W32Pattern); \
	*MBP0 				= byteswap32(W32Pattern); \
	*(LongP)W32Buffer 		= byteswap32(MASK); \
    } \
    *ACL_BACKGROUND_RASTER_OPERATION	= 0xaa; \
    if (W32et6000) \
      *ACL_MIX_CONTROL			= 0x32; \
    else \
      *ACL_ROUTING_CONTROL		= 0x02; \
    *ACL_XY_DIRECTION			= 0; \
    *ACL_DESTINATION_Y_OFFSET		= byteswap16(DST_OFFSET); \
    *ACL_SOURCE_WRAP			= 0x02; \
    *ACL_SOURCE_Y_OFFSET		= BYTESWAP16(0x3); \
    *ACL_SOURCE_ADDRESS			= byteswap32(W32Foreground); \
    *MBP0 				= byteswap32(W32Foreground); \
    *(LongP)W32Buffer	 		= byteswap32(FOREGROUND); \
    *ACL_MIX_ADDRESS			= 0; \
    *ACL_MIX_Y_OFFSET 			= BYTESWAP16(31); \
}

#define W32_STIPPLE \
	    while (h--) \
	    { \
	    	bits = src[y]; \
	    	if (++y == stippleHeight) \
		    y = 0; \
	    	if (rot) \
		    RotBitsLeft(bits,rot); \
		w = w32_chunks; \
		while (w--) \
		{ \
		    *ACL = LowByte(bits); \
		    *ACL = HighByteLowWord(bits); \
		    *ACL = LowByteHighWord(bits); \
		    *ACL = HighByte(bits); \
	    	} \
		switch (w32_misc) \
		{ \
		    case 3: \
			*ACL = LowByte(bits); \
			*ACL = HighByteLowWord(bits); \
			*ACL = LowByteHighWord(bits); \
			break; \
		    case 2: \
			*ACL = LowByte(bits); \
			*ACL = HighByteLowWord(bits); \
			break; \
		    case 1: \
			*ACL = LowByte(bits); \
			break; \
		} \
	    }


#define W32P_STIPPLE \
	    while (h--) \
	    { \
	    	bits = src[y]; \
	    	if (++y == stippleHeight) \
		    y = 0; \
	    	if (rot) \
		    RotBitsLeft(bits,rot); \
		w = w32_chunks; \
		while (w--) \
		{ \
		    *ACL = LowByte(bits); \
		    *(ACL + 1) = HighByteLowWord(bits); \
		    *(ACL + 2) = LowByteHighWord(bits); \
		    *(ACL + 3) = HighByte(bits); \
	    	} \
		switch (w32_misc) \
		{ \
		    case 3: \
			*ACL = LowByte(bits); \
			*(ACL + 1) = HighByteLowWord(bits); \
			*(ACL + 2) = LowByteHighWord(bits); \
			break; \
		    case 2: \
			*ACL = LowByte(bits); \
			*(ACL + 1) = HighByteLowWord(bits); \
			break; \
		    case 1: \
			*ACL = LowByte(bits); \
			break; \
		} \
	    }

#define ET6K_STIPPLE \
	    while (h--) \
	    { \
                if (h&1) /* ping-pong between stipple buffers */ \
                { \
                    *ACL_MIX_ADDRESS = byteswap32(MixDstPong); \
                    *MBP0 = byteswap32(W32MixPong); \
                } \
                else \
                { \
                    *ACL_MIX_ADDRESS = byteswap32(MixDstPing); \
                    *MBP0 = byteswap32(W32Mix); /* Ping */ \
                } \
	    	bits = src[y]; \
	    	if (++y == stippleHeight) \
		    y = 0; \
	    	if (rot) \
		    RotBitsLeft(bits,rot); \
		w = w32_chunks+1; \
		while (w--) \
		    *((LongP)W32Buffer+w) = byteswap32(bits); \
		*(ACL_DESTINATION_ADDRESS)=byteswap32(ACLDst); \
	        ACLDst += nlwDst; \
	    }


#endif /* W32_STIP_H */

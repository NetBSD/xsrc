/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/neo/neo.h,v 1.1.2.3 1998/10/01 12:19:43 hohndel Exp $ */
/**********************************************************************
Copyright 1998 by Precision Insight, Inc., Cedar Park, Texas.

                        All Rights Reserved

Permission to use, copy, modify, distribute, and sell this software and
its documentation for any purpose is hereby granted without fee,
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in
supporting documentation, and that the name of Precision Insight not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  Precision Insight
and its suppliers make no representations about the suitability of this
software for any purpose.  It is provided "as is" without express or 
implied warranty.

PRECISION INSIGHT DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
EVENT SHALL PRECISION INSIGHT AND/OR ITS SUPPLIERS BE LIABLE FOR ANY
SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER
RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF
CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
**********************************************************************/

/*
 * This Precision Insight driver has been sponsored by Red Hat.
 *
 * Authors:
 *   Jens Owen (jens@precisioninsight.com)
 *   Kevin E. Martin (kevin@precisioninsight.com)
 */

/* function prototypes */

/* in neo_2070.c */
extern void Neo2070AccelInit();
extern void Neo2070Sync();

/* in neo_2090.c */
extern void Neo2090AccelInit();
extern void Neo2090Sync();

/* in neo_2097.c */
extern void Neo2097AccelInit();
extern void Neo2097Sync();

/* in neo_2200.c */
extern void Neo2200AccelInit();
extern void Neo2200Sync();

/* in neo_cursor.c */
extern Bool NeoCursorInit();
extern void NeoRestoreCursor();
extern void NeoWarpCursor();
extern void NeoQueryBestSize();
extern void NeoHideCursor();
extern void NeoTempSWCursor();

/* globals */
extern unsigned char* NeoMMIOBase;
extern int NeoFifoCount;
extern int NeoChipset;
extern int NeoPanelWidth;
extern int NeoPanelHeight;
extern int NeoMMIOAddr;
extern int NeoLinearAddr;
extern int NeoCursorMemSegment;
extern int NeoCursorOffset;

/* I/O register offsets */
#define GRAX	0x3CE

/* memory mapped register access macros */
extern unsigned char *NeoMMIOBase;
#define INREG8(addr) *(volatile CARD8 *)(NeoMMIOBase + (addr))
#define INREG16(addr) *(volatile CARD16 *)(NeoMMIOBase + (addr))
#define INREG(addr) *(volatile CARD32 *)(NeoMMIOBase + (addr))
#define OUTREG8(addr, val) *(volatile CARD8 *)(NeoMMIOBase + (addr)) = (val)
#define OUTREG16(addr, val) *(volatile CARD16 *)(NeoMMIOBase + (addr)) = (val)
#define OUTREG(addr, val) *(volatile CARD32 *)(NeoMMIOBase + (addr)) = (val)

/* This swizzle macro is to support the manipulation of cursor masks when
 * the sprite moves off the left edge of the display.  This code is
 * platform specific, and is known to work with 32bit little endian machines
 */
#define SWIZZLE32(__b) { \
  ((unsigned char *)&__b)[0] = byte_reversed[((unsigned char *)&__b)[0]]; \
  ((unsigned char *)&__b)[1] = byte_reversed[((unsigned char *)&__b)[1]]; \
  ((unsigned char *)&__b)[2] = byte_reversed[((unsigned char *)&__b)[2]]; \
  ((unsigned char *)&__b)[3] = byte_reversed[((unsigned char *)&__b)[3]]; \
}

#define PROBED_NM2070	0x01
#define PROBED_NM2090	0x42
#define PROBED_NM2093	0x43
#define PROBED_NM2097	0x83
#define PROBED_NM2160	0x44
#define PROBED_NM2200	0x45

/* $XConsortium: tgui_ger.h /main/2 1996/01/10 10:21:38 kaleb $ */
/*
 * Copyright 1995 by Alan Hourihane, Wigan, England.
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Alan Hourihane not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  Alan Hourihane makes no representations
 * about the suitability of this software for any purpose.  It is provided
 * "as is" without express or implied warranty.
 *
 * ALAN HOURIHANE DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL ALAN HOURIHANE BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 *
 * Author:  Alan Hourihane, alanh@fairlite.demon.co.uk
 */
/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/tvga8900/tgui_ger.h,v 3.2 1996/02/04 09:14:23 dawes Exp $ */

/* Graphics Engine for 9420/9430 */

#define GER_INDEX	0x210A
#define GER_BYTE0	0x210C
#define GER_BYTE1	0x210D
#define GER_BYTE2	0x210E
#define GER_BYTE3	0x210F

/* Graphics Engine for 9440/9660/9680 */

#define GER_STATUS	0x2120		
#define		GE_BUSY	0x80
#define GER_OPERMODE	0x2122		/* Byte for 9440, Word for 9660/9680 */
#define GER_COMMAND	0x2124		
#define		GE_NOP		0x00	/* No Operation */
#define		GE_BLT		0x01	/* BitBLT ROP3 only */
#define		GE_BLT_ROP4	0x02	/* BitBLT ROP4 (9660/9680 only) */
#define		GE_SCANLINE	0x03	/* Scan Line */
#define		GE_BRESLINE	0x04	/* Bresenham Line */
#define		GE_SHVECTOR	0x05	/* Short Vector */
#define		GE_FASTLINE	0x06	/* Fast Line (9660/9680 only) */
#define		GE_TRAPEZ	0x07	/* Trapezoidal fill (9660/9680 only) */
#define		GE_ELLIPSE	0x08	/* Ellipse (9660/9680 only) (RES) */
#define		GE_ELLIP_FILL	0x09	/* Ellipse Fill (9660/9680 only) (RES)*/
#define	GER_FMIX	0x2127
#define GER_DRAWFLAG	0x2128		/* long */
#define GER_FCOLOUR	0x212C		/* Word for 9440, long for 9660/9680 */
#define GER_BCOLOUR	0x2130		/* Word for 9440, long for 9660/9680 */
#define GER_PATLOC	0x2134		/* Word */
#define GER_DEST_X	0x2138		/* Word */
#define GER_DEST_Y	0x213A		/* Word */
#define GER_SRC_X	0x213C		/* Word */
#define GER_SRC_Y	0x213D		/* Word */
#define GER_DIM_X	0x2140		/* Word */
#define GER_DIM_Y	0x2142		/* Word */
#define GER_PATTERN	0x2180		/* from 0x2180 to 0x21FF */

/* Graphics Engine for 9660/9680 */

/* Routines */

#define WaitIdle()	while (inb(GER_STATUS) & GE_BUSY);

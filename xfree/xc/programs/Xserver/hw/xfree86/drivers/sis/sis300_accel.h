/*
 * Copyright 1998,1999 by Alan Hourihane, Wigan, England.
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
 * Authors:  Alan Hourihane, alanh@fairlite.demon.co.uk
 *           Mike Chapman <mike@paranoia.com>, 
 *           Juanjo Santamarta <santamarta@ctv.es>, 
 *           Mitani Hiroshi <hmitani@drl.mei.co.jp> 
 *           David Thomas <davtom@dream.org.uk>. 
 *           Xavier Ducoin <x.ducoin@lectra.com>
 */
/* $XFree86: xc/programs/Xserver/hw/xfree86/drivers/sis/sis300_accel.h,v 1.3 2001/04/19 12:40:33 alanh Exp $ */


/* Definitions for the SIS engine communication. */

#define PATREGSIZE      384
#define BR(x)   (0x8200 | (x) << 2)
#define PBR(x)  (0x8300 | (x) << 2)

/* Definitions for the SiS300 engine command */
#define BITBLT                  0x00000000
#define COLOREXP                0x00000001
#define ENCOLOREXP              0x00000002
#define MULTIPLE_SCANLINE       0x00000003
#define LINE                    0x00000004
#define TRAPAZOID_FILL          0x00000005
#define TRANSPARENT_BITBLT      0x00000006

#define SRCVIDEO                0x00000000
#define SRCSYSTEM               0x00000010
#define SRCAGP                  0x00000020

#define PATFG                   0x00000000
#define PATPATREG               0x00000040
#define PATMONO                 0x00000080

#define X_INC                   0x00010000
#define X_DEC                   0x00000000
#define Y_INC                   0x00020000
#define Y_DEC                   0x00000000

#define NOCLIP                  0x00000000
#define NOMERGECLIP             0x04000000
#define CLIPENABLE              0x00040000
#define CLIPWITHOUTMERGE        0x04040000

#define OPAQUE                  0x00000000
#define TRANSPARENT             0x00100000

#define DSTAGP                  0x02000000
#define DSTVIDEO                0x02000000

/* Line */
#define LINE_STYLE              0x00800000
#define NO_RESET_COUNTER        0x00400000
#define NO_LAST_PIXEL           0x00200000

/* Macros to do useful things with the SIS BitBLT engine */

/* 
   bit 31 2D engine: 1 is idle,
   bit 30 3D engine: 1 is idle,
   bit 29 Command queue: 1 is empty
*/

int     CmdQueLen;

#define SiSIdle \
  while( (MMIO_IN16(pSiS->IOBase, BR(16)+2) & 0xE000) != 0xE000){}; \
  while( (MMIO_IN16(pSiS->IOBase, BR(16)+2) & 0xE000) != 0xE000){}; \
  CmdQueLen=MMIO_IN16(pSiS->IOBase, 0x8240);
        

#define SiSSetupSRCBase(base) \
                if (CmdQueLen <= 0)  SiSIdle;\
                MMIO_OUT32(pSiS->IOBase, BR(0), base);\
                CmdQueLen --;
        

#define SiSSetupSRCPitch(pitch) \
                if (CmdQueLen <= 0)  SiSIdle;\
                MMIO_OUT16(pSiS->IOBase, BR(1), pitch);\
                CmdQueLen --;

#define SiSSetupSRCXY(x,y) \
                if (CmdQueLen <= 0)  SiSIdle;\
                MMIO_OUT32(pSiS->IOBase, BR(2), (x)<<16 | (y) );\
                CmdQueLen --;

#define SiSSetupDSTBase(base) \
                if (CmdQueLen <= 0)  SiSIdle;\
                MMIO_OUT32(pSiS->IOBase, BR(4), base);\
                CmdQueLen --;

#define SiSSetupDSTXY(x,y) \
                if (CmdQueLen <= 0)  SiSIdle;\
                MMIO_OUT32(pSiS->IOBase, BR(3), (x)<<16 | (y) );\
                CmdQueLen --;

#define SiSSetupDSTRect(x,y) \
                if (CmdQueLen <= 0)  SiSIdle;\
                MMIO_OUT32(pSiS->IOBase, BR(5), (y)<<16 | (x) );\
                CmdQueLen --;

#define SiSSetupDSTColorDepth(bpp) \
                if (CmdQueLen <= 0)  SiSIdle;\
                MMIO_OUT16(pSiS->IOBase, BR(1)+2, bpp);\
                CmdQueLen --;

#define SiSSetupRect(w,h) \
                if (CmdQueLen <= 0)  SiSIdle;\
                MMIO_OUT32(pSiS->IOBase, BR(6), (h)<<16 | (w) );\
                CmdQueLen --;

#define SiSSetupPATFG(color) \
                if (CmdQueLen <= 0)  SiSIdle;\
                MMIO_OUT32(pSiS->IOBase, BR(7), color);\
                CmdQueLen --;

#define SiSSetupPATBG(color) \
                if (CmdQueLen <= 0)  SiSIdle;\
                MMIO_OUT32(pSiS->IOBase, BR(8), color);\
                CmdQueLen --;

#define SiSSetupSRCFG(color) \
                if (CmdQueLen <= 0)  SiSIdle;\
                MMIO_OUT32(pSiS->IOBase, BR(9), color);\
                CmdQueLen --;

#define SiSSetupSRCBG(color) \
                if (CmdQueLen <= 0)  SiSIdle;\
                MMIO_OUT32(pSiS->IOBase, BR(10), color);\
                CmdQueLen --;

#define SiSSetupMONOPAT(p0,p1) \
                if (CmdQueLen <= 1)  SiSIdle;\
                MMIO_OUT32(pSiS->IOBase, BR(11), p0);\
                MMIO_OUT32(pSiS->IOBase, BR(12), p1);\
                CmdQueLen =CmdQueLen-2;

#define SiSSetupClipLT(left,top) \
                if (CmdQueLen <= 0)  SiSIdle;\
                MMIO_OUT32(pSiS->IOBase, BR(13), ((left) & 0xFFFF) | (top)<<16 );\
                CmdQueLen--;

#define SiSSetupClipRB(right,bottom) \
                if (CmdQueLen <= 0)  SiSIdle;\
                MMIO_OUT32(pSiS->IOBase, BR(14), ((right) & 0xFFFF) | (bottom)<<16 );\
                CmdQueLen --;

#define SiSSetupROP(rop) \
        pSiS->CommandReg = (rop) << 8;

#define SiSSetupCMDFlag(flags) \
        pSiS->CommandReg |= (flags);

#define SiSDoCMD \
                if (CmdQueLen <= 1)  SiSIdle;\
                MMIO_OUT32(pSiS->IOBase, BR(15), pSiS->CommandReg); \
                MMIO_OUT32(pSiS->IOBase, BR(16), 0);\
                CmdQueLen =CmdQueLen-2;

#define SiSSetupX0Y0(x,y) \
                if (CmdQueLen <= 0)  SiSIdle;\
                MMIO_OUT32(pSiS->IOBase, BR(2), (y)<<16 | (x) );\
                CmdQueLen --;

#define SiSSetupX1Y1(x,y) \
                if (CmdQueLen <= 0)  SiSIdle;\
                MMIO_OUT32(pSiS->IOBase, BR(3), (y)<<16 | (x) );\
                CmdQueLen --;

#define SiSSetupLineCount(c) \
                if (CmdQueLen <= 0)  SiSIdle;\
                MMIO_OUT16(pSiS->IOBase, BR(6), c);\
                CmdQueLen --;

#define SiSSetupStylePeriod(p) \
                if (CmdQueLen <= 0)  SiSIdle;\
                MMIO_OUT16(pSiS->IOBase, BR(6)+2, p);\
                CmdQueLen --;

#define SiSSetupStyleLow(ls) \
                if (CmdQueLen <= 0)  SiSIdle;\
                MMIO_OUT32(pSiS->IOBase, BR(11), ls);\
                CmdQueLen --;

#define SiSSetupStyleHigh(ls) \
                if (CmdQueLen <= 0)  SiSIdle;\
                MMIO_OUT32(pSiS->IOBase, BR(12), ls);\
                CmdQueLen --;

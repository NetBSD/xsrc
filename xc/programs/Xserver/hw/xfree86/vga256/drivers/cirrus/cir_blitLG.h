#ifndef __CIR_BLITLG_H
#define __CIR_BLITLG_H


/*
 *
 * Copyright 1996 by Corin Anderson, Bellevue, Washington, USA
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Corin Anderson not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  Corin Anderson makes no
 * representations about the suitability of this software for any purpose.  It
 * is provided "as is" without express or implied warranty.
 *
 * CORIN ANDERSON DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL CORIN ANDERSON BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 *
 * Author:  Corin Anderson, <corina@bdc.cirrus.com>
 *
 * cir_blitLG.h
 */

/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/cirrus/cir_blitLG.h,v 3.1 1996/09/29 13:39:40 dawes Exp $ */


/* This header file defines the necessary structures, contstants, and 
   variables for using the bitBLT engine on a Laguna family graphics 
   accelerator.  */


void CirrusLgDoBitbltCopy();
void CirrusLgFillRectSolidCopy ();
void CirrusLgFillBoxSolid ();
void CirrusLgPolyFillRect();
void CirrusLgFillRectSolid(
    DrawablePtr	    pDrawable,
    GCPtr	    pGC,
    int		    nBox,
    BoxPtr	    pBox);
extern RegionPtr CirrusLgCopyArea16();
extern RegionPtr CirrusLgCopyArea24();
extern RegionPtr CirrusLgCopyArea32();
extern void CirrusLgCopyWindow();


enum {                            /* Offsets into MMIO space for bitBLT regs */
  STATUS       = 0x0400,
  OP0_opRDRAM  = 0x0520,
  OP1_opRDRAM  = 0x0540,
  OP0_opMRDRAM = 0x0524,
  OP1_opMRDRAM = 0x0544,
  DRAWDEF      = 0x0584,
  BLTDEF       = 0x0586,
  BLTEXT_EX    = 0x0700,
  QFREE        = 0x0404,
  PATOFF       = 0x052A,
  HOSTDATA     = 0x0800,
  OP_opBGCOLOR = 0x05E4,
  OP_opFGCOLOR = 0x05E0
};

enum {                            /* OR these together to form a bitBLT mode */
  HOST2SCR  = 0x1120,
  SCR2HOST  = 0x2010,
  SCR2SCR   = 0x1110,
  COLORSRC  = 0x0000,
  MONOSRC   = 0x0040,
  PATTERN   = 0x0080,
  COLORFILL = 0x0070,
  BLITUP    = 0x8000
};

extern int lgCirrusRop[16];  /* Defined in cir_blitLG.c */


/* Use the function call as opposed to the macro.  The reason is that
   the function call uses a volatile variable, which is necessary for 
   forcing the STATUS register to be read each time.  Using just the macro,
   only one read will be performed, and the machine will hang. */
int LgReady(void);
/*
#define LgREADY() \
  ((*(unsigned char *)(cirrusMMIOBase + STATUS) & 0x07) == 0x00)
*/

#define LgSETROP(rop) \
  *(unsigned short *)(cirrusMMIOBase + DRAWDEF) = (rop);

#define LgSETMODE(mode) \
  *(unsigned short *)(cirrusMMIOBase + BLTDEF) = (mode);

#define LgSETDSTXY(X, Y) \
  *(unsigned long *)(cirrusMMIOBase + OP0_opRDRAM) = (((Y) << 16) | (X));

#define LgSETSRCXY(X, Y) \
  *(unsigned long *)(cirrusMMIOBase + OP1_opRDRAM) = (((Y) << 16) | (X));
    
#define LgSETPHASE0(phase) \
  *(unsigned long *)(cirrusMMIOBase + OP0_opRDRAM) = (phase);

#define LgSETPHASE1(phase) \
  *(unsigned long *)(cirrusMMIOBase + OP1_opRDRAM) = (phase);

#define LgSETMPHASE0(phase) \
  *(unsigned long *)(cirrusMMIOBase + OP0_opMRDRAM) = (phase);

#define LgSETMPHASE1(phase) \
  *(unsigned long *)(cirrusMMIOBase + OP1_opMRDRAM) = (phase);

#define LgSETEXTENTS(width, height)  \
  *(unsigned long *)(cirrusMMIOBase + BLTEXT_EX) = (((height) << 16)|(width));

#define LgHOSTDATAWRITE(data)  \
  *(unsigned long *)(cirrusMMIOBase + HOSTDATA) = (data);

#define LgHOSTDATAREAD()  \
  (*(unsigned long *)(cirrusMMIOBase + HOSTDATA))

#define LgSETBACKGROUND(color) \
  *(unsigned long *)(cirrusMMIOBase + OP_opBGCOLOR) = (color);

#define LgSETFOREGROUND(color) \
  *(unsigned long *)(cirrusMMIOBase + OP_opFGCOLOR) = (color);


#endif  /* __CIR_BLITLG_H */

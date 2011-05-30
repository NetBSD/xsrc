/* $XConsortium: s3bcach.c,v 1.2 94/10/12 20:07:37 kaleb Exp $ */
/* $XFree86: xc/programs/Xserver/hw/xfree86/accel/s3/s3bcach.c,v 3.7 1995/01/28 17:01:46 dawes Exp $ */
/*
 * Copyright 1993 by Jon Tombs. Oxford University
 * 
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Jon Tombs or Oxford University shall
 * not be used in advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission. The authors  make no
 * representations about the suitability of this software for any purpose. It
 * is provided "as is" without express or implied warranty.
 * 
 * JON TOMBS DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
 * ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
 * THE AUTHORS BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES
 * OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
 * ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 * 
 */

/*
 * Id: s3bcach.c,v 2.3 1993/07/24 13:16:56 jon Exp
 */

/*
 * Modified for the CyberVision 64 by Michael Teske
 */

#include "amiga.h" 
#include "gcstruct.h"
#include "cfb.h"
#include        "amigaCV.h"  
#include "xf86bcache.h" 

void
s3CacheMoveBlock(srcx, srcy, dstx, dsty, h, w, id)
int srcx, srcy, dstx, dsty, h, w;
unsigned int id;
{
   fbFd *inf = amigaInfo(amigaCVsavepScreen);
   volatile caddr_t vgaBase =  (inf->regs);   

   __dolog ("Entering CacheMoveBlock()\n");

   BLOCK_CURSOR;
   WaitQueue(7);
   PCI_HACK();
   S3_OUTW(MULTIFUNC_CNTL, SCISSORS_T | 0);
   S3_OUTW(MULTIFUNC_CNTL, SCISSORS_L | 0);
   S3_OUTW(MULTIFUNC_CNTL, SCISSORS_R | (amigaVirtualWidth - 1));
   S3_OUTW(MULTIFUNC_CNTL, SCISSORS_B | 0xfff);
   S3_OUTW(MULTIFUNC_CNTL, PIX_CNTL | MIXSEL_FRGDMIX);  
   S3_OUTW(FRGD_MIX, FSS_BITBLT | MIX_SRC);
   S3_OUTW(BKGD_MIX, BSS_BITBLT | MIX_SRC);

   WaitQueue16_32(6,8);		/* now shift the cache */
   PCI_HACK();

   S3_OUTW32(WRT_MASK, id);
   S3_OUTW32(RD_MASK, id);
   S3_OUTW(CUR_Y, srcy);
   S3_OUTW(CUR_X, srcx);
   S3_OUTW(DESTX_DIASTP, dstx);
   S3_OUTW(DESTY_AXSTP, dsty);

   WaitQueue(3);
   PCI_HACK();

   S3_OUTW(MAJ_AXIS_PCNT, w - 1);
   S3_OUTW(MULTIFUNC_CNTL, MIN_AXIS_PCNT | (h - 1));
   S3_OUTW(CMD, CMD_BITBLT | INC_X | INC_Y | DRAW | PLANAR | WRTDATA);

   /* sanity returns */
   WaitQueue16_32(4,5);
   WaitIdle();

   S3_OUTW32(RD_MASK, ~0);
   S3_OUTW(MULTIFUNC_CNTL, PIX_CNTL | MIXSEL_FRGDMIX | COLCMPOP_F);
   S3_OUTW(FRGD_MIX, FSS_FRGDCOL | MIX_SRC);
   S3_OUTW(BKGD_MIX, BSS_BKGDCOL | MIX_SRC);
   UNBLOCK_CURSOR;

   __dolog ("Leaving s3CacheMoveBlock()\n");

}

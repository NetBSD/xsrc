/* $XConsortium: s3fcach.c,v 1.4 95/01/23 15:33:59 kaleb Exp $ */
/* $XFree86: xc/programs/Xserver/hw/xfree86/accel/s3/s3fcach.c,v 3.16 1995/07/12 15:36:43 dawes Exp $ */
/*
 * Copyright 1992 by Kevin E. Martin, Chapel Hill, North Carolina.
 * 
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Kevin E. Martin not be used in
 * advertising or publicity pertaining to distribution of the software
 * without specific, written prior permission.  Kevin E. Martin makes no
 * representations about the suitability of this software for any purpose. It
 * is provided "as is" without express or implied warranty.
 * 
 * KEVIN E. MARTIN DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL KEVIN E. MARTIN BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF
 * USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 * 
 */

/*
 * Modified by Amancio Hasty and Jon Tombs
 * 
 * Id: s3fcach.c,v 2.5 1993/08/09 06:17:57 jon Exp jon
 */

/*
 * Modified for the CyberVision 64 by Michael Teske
 */

#include	"amiga.h"
#include	"gcstruct.h"
#include	"cfb.h"
#include 	"amigaCV.h"
#include        "xf86bcache.h"
#include	"xf86fcache.h"
#include	"xf86text.h"

#include        "gcstruct.h"
#include        "fontstruct.h"
#include        "dixfontstr.h"

static unsigned long s3FontAge;
#define NEXT_FONT_AGE  ++s3FontAge

extern short s3alu[];

static __inline__ void
Dos3CPolyText8(x, y, count, chars, fentry, pGC, pBox)
     int   x, y, count;
     unsigned char *chars;
     CacheFont8Ptr fentry;
     GCPtr pGC;
     BoxPtr pBox;
{
   int   gHeight;
   int   w = fentry->w;
   int blocki = 255;
   unsigned short height = 0;
   unsigned short width = 0;
   Pixel pmsk = 0;
   fbFd *inf = amigaInfo(amigaCVsavepScreen);
   volatile caddr_t vgaBase =  (inf->regs);

   __dolog ("Entering Dos3CPolyText8(), count = %d c=%d\n", count,(int)*chars);


   BLOCK_CURSOR;
   for (;count > 0; count--, chars++) {
      CharInfoPtr pci;
      short xoff;

      pci = fentry->pci[(int)*chars];

      if (pci != NULL) {

	 gHeight = GLYPHHEIGHTPIXELS(pci);
	 if (gHeight) {

	    if ((int) (*chars / 32) != blocki) {
	       bitMapBlockPtr block;
	       
	       blocki = (int) (*chars / 32);
	       block = fentry->fblock[blocki];
	       if (block == NULL) {
		  /*
		   * Reset the GE context to a known state before calling
		   * the xf86loadfontblock function.
		   */

		  WaitQueue16_32(5,6);PCI_HACK();
		  S3_OUTW(MULTIFUNC_CNTL, SCISSORS_T | 0);
		  S3_OUTW(MULTIFUNC_CNTL, SCISSORS_L | 0);
		  S3_OUTW(MULTIFUNC_CNTL, SCISSORS_R | (amigaVirtualWidth-1));
		  S3_OUTW(MULTIFUNC_CNTL, SCISSORS_B | 0xfff);
		  S3_OUTW32(RD_MASK, ~0);

		  xf86loadFontBlock(fentry, blocki);
		  block = fentry->fblock[blocki];		  

		  /*
		   * Restore the GE context.
		   */
		  WaitQueue(4);PCI_HACK();
		  S3_OUTW(MULTIFUNC_CNTL, SCISSORS_L | (short)pBox->x1);
		  S3_OUTW(MULTIFUNC_CNTL, SCISSORS_T | (short)pBox->y1);
		  S3_OUTW(MULTIFUNC_CNTL, SCISSORS_R | (short)(pBox->x2 - 1));
		  S3_OUTW(MULTIFUNC_CNTL, SCISSORS_B | (short)(pBox->y2 - 1));
		  WaitQueue16_32(5,7);PCI_HACK();
		  S3_OUTW32(FRGD_COLOR, pGC->fgPixel);
		  S3_OUTW(MULTIFUNC_CNTL, 
			PIX_CNTL | MIXSEL_EXPBLT | COLCMPOP_F);
		  S3_OUTW(FRGD_MIX, FSS_FRGDCOL | s3alu[pGC->alu]);
		  S3_OUTW(BKGD_MIX, BSS_BKGDCOL | MIX_DST);
		  S3_OUTW32(WRT_MASK, pGC->planemask);
		  height = width = pmsk = 0;
	       }
	       WaitQueue16_32(2,3);PCI_HACK();
	       S3_OUTW(CUR_Y, block->y);	       

	       /*
		* Is the readmask altered
		*/
	       if (!pmsk || pmsk != block->id) {
		  pmsk = block->id;
	       	  S3_OUTW32(RD_MASK, pmsk);
	       }
	       xoff = block->x;
	       block->lru = NEXT_FONT_AGE;
   	    }

	    WaitQueue(6);PCI_HACK();

	    S3_OUTW(CUR_X, (short) (xoff + (*chars & 0x1f) * w));
	    S3_OUTW(DESTX_DIASTP,
		  (short)(x + pci->metrics.leftSideBearing));
	    S3_OUTW(DESTY_AXSTP, (short)(y - pci->metrics.ascent));
	    if (!width || (short)(GLYPHWIDTHPIXELS(pci) - 1) != width) {
	       width = (short)(GLYPHWIDTHPIXELS(pci) - 1);
	       S3_OUTW(MAJ_AXIS_PCNT, width);
	    }
	    if (!height || (short)(gHeight - 1) != height) {
	       height = (short)(gHeight - 1);
	       S3_OUTW(MULTIFUNC_CNTL, MIN_AXIS_PCNT | height);
	    }
	    S3_OUTW(CMD, CMD_BITBLT | INC_X | INC_Y | DRAW | PLANAR | WRTDATA);
	    /*WaitIdle();*/
	 }
	 x += pci->metrics.characterWidth;
      }
   }
   UNBLOCK_CURSOR;
   __dolog ("Leaving Dos3CPolyText8()\n");

   return;
}

/*
 * Set the hardware scissors to match the clipping rectables and 
 * call the glyph output routine.
 */
void 
s3GlyphWrite(
     int x, int y, int count,
     unsigned char *chars,
     CacheFont8Ptr fentry,
     GCPtr pGC,
     BoxPtr pBox,
     int numRects)
{

   fbFd *inf = amigaInfo(amigaCVsavepScreen);
   volatile caddr_t vgaBase =  (inf->regs); 
   BLOCK_CURSOR;
   __dolog ("Entering s3GlyphWrite\n");

   WaitQueue16_32(5,7);PCI_HACK();
   S3_OUTW32(FRGD_COLOR, pGC->fgPixel);
   S3_OUTW(MULTIFUNC_CNTL, PIX_CNTL | MIXSEL_EXPBLT | COLCMPOP_F);
   S3_OUTW(FRGD_MIX, FSS_FRGDCOL | s3alu[pGC->alu]);
   S3_OUTW(BKGD_MIX, BSS_BKGDCOL | MIX_DST);
   S3_OUTW32(WRT_MASK, pGC->planemask);

   for (; --numRects >= 0; ++pBox) {
      WaitQueue(4);PCI_HACK();
      S3_OUTW(MULTIFUNC_CNTL, SCISSORS_L | (short)pBox->x1);
      S3_OUTW(MULTIFUNC_CNTL, SCISSORS_T | (short)pBox->y1);
      S3_OUTW(MULTIFUNC_CNTL, SCISSORS_R | (short)(pBox->x2 - 1));
      S3_OUTW(MULTIFUNC_CNTL, SCISSORS_B | (short)(pBox->y2 - 1));
      Dos3CPolyText8(x, y, count, chars, fentry, pGC, pBox);
   }

   WaitQueue(4);PCI_HACK();
   S3_OUTW(MULTIFUNC_CNTL, SCISSORS_T | 0);
   S3_OUTW(MULTIFUNC_CNTL, SCISSORS_L | 0);
   S3_OUTW(MULTIFUNC_CNTL, SCISSORS_R | amigaVirtualWidth);
   S3_OUTW(MULTIFUNC_CNTL, SCISSORS_B | 0xfff);
   WaitQueue16_32(4,5);PCI_HACK();
   S3_OUTW32(RD_MASK, ~0);
   S3_OUTW(MULTIFUNC_CNTL, PIX_CNTL | MIXSEL_FRGDMIX | COLCMPOP_F);
   S3_OUTW(FRGD_MIX, FSS_FRGDCOL | MIX_SRC);
   S3_OUTW(BKGD_MIX, BSS_BKGDCOL | MIX_SRC);
   UNBLOCK_CURSOR;
     __dolog ("Leaving s3GlyphWrite\n");

   return;
}

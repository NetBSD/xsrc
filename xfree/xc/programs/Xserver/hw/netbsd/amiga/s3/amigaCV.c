#ifndef lint
static char *rid="$XConsortium: amigaCV.c,v 0.1 96/01/17 20:29:38 teske Exp $";
#endif /* lint */
/*
Copyright (c) 1991  X Consortium

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
 * Author:  Keith Packard, MIT X Consortium
 */

/*
 * Modified for the CyberVision 64 by Michael Teske
 */


#include	"amiga.h"

#include	"Xmd.h"
#include	"gcstruct.h"
#include	"scrnintstr.h"
#include	"pixmapstr.h"
#include	"regionstr.h"
#include	"mistruct.h"
#include	"mifillarc.h"
#include	"fontstruct.h"
#include	"dixfontstr.h"
#include	"cfb.h"
#include	"cfbmskbits.h"
#include	"cfb8bit.h"
#include	"fastblt.h"
#include	"mergerop.h"
#include	"amigaCV.h"
#include	"migc.h"


extern int cfb16ScreenPrivateIndex, cfb32ScreenPrivateIndex;

short s3alu[16] =
{
   MIX_0,
   MIX_AND,
   MIX_SRC_AND_NOT_DST,
   MIX_SRC,
   MIX_NOT_SRC_AND_DST,
   MIX_DST,
   MIX_XOR,
   MIX_OR,
   MIX_NOR,
   MIX_XNOR,
   MIX_NOT_DST,
   MIX_SRC_OR_NOT_DST,
   MIX_NOT_SRC,
   MIX_NOT_SRC_OR_DST,
   MIX_NAND,
   MIX_1
};           



static unsigned long	copyPlaneFG, copyPlaneBG;



int	amigaCVScreenPrivateIndex;
int	amigaCVGCPrivateIndex;
int	amigaCVWindowPrivateIndex;
int	amigaCVGeneration;


extern GCFuncs	amiga8CVGCFuncs;

extern GCFuncs	amiga16CVGCFuncs;

extern GCFuncs  amiga32CVGCFuncs;


/* externs from amiga8CVgc.c */
extern GCOps	amiga8CVNonTEOps,amiga8CVTEOps,
                amiga8CVNonTEOps1Rect,amiga8CVTEOps1Rect  ;





/* externs from amiga16CVgc.c */
extern GCOps	amiga16CVNonTEOps,  amiga16CVTEOps,
                /*amiga16CVNonTEOps1Rect,*/ amiga16CVTEOps1Rect  ;



/* externs from amiga32CVgc.c */
extern GCOps	amiga32CVNonTEOps,  amiga32CVTEOps,
                /*amiga32CVNonTEOps1Rect,*/ amiga32CVTEOps1Rect  ;


#define FONTWIDTH(font)	(FONTMAXBOUNDS(font,rightSideBearing) - \
			 FONTMINBOUNDS(font,leftSideBearing))

GCOps *
amigaCVMatchCommon(GCPtr pGC, cfbPrivGCPtr devPriv, int bpp)
{
    if (pGC->lineWidth != 0)
	return 0;
    if (pGC->lineStyle != LineSolid)
	return 0;
    if (pGC->fillStyle != FillSolid)
	return 0;
    if (devPriv->rop != GXcopy)
	return 0;
    if (pGC->font &&
        FONTWIDTH(pGC->font) <= 32 &&
	FONTMINBOUNDS(pGC->font,characterWidth) >= 0)
    {
        if (bpp == 8)
	  {
	    if (TERMINALFONT(pGC->font))
	      if (devPriv->oneRect)
		return &amiga8CVTEOps1Rect;
	      else
	        return &amiga8CVTEOps;
	    else
	      if (devPriv->oneRect)
		return &amiga8CVNonTEOps1Rect;
	      else 
	        return &amiga8CVNonTEOps;
	  }
	else if (bpp == 16)
	  {
#if 0
	    if (TERMINALFONT(pGC->font))
	      return &amiga16CVTEOps;
	    else
	      return &amiga16CVNonTEOps;
#endif
	 if (devPriv->oneRect)
	    return &amiga16CVTEOps1Rect;
	 else
	    return &amiga16CVTEOps;

	  }
	else
	  {
#if 0
	    if (TERMINALFONT(pGC->font))
	      return &amiga24CVTEOps;
	    else
	      return &amiga24CVNonTEOps;
#endif
	 if (devPriv->oneRect)
	    return &amiga32CVTEOps1Rect;
	 else
	    return &amiga32CVTEOps;


	  }

    }
    return 0;
}

void
amigaCVDestroyGC(GCPtr   pGC)
{

    amigaCVPrivGCPtr	    gxPriv = amigaCVGetGCPrivate(pGC);

#if 0
    if (gxPriv->stipple)
	xfree(gxPriv->stipple);
#endif
    miDestroyGC(pGC);
}


Bool
amigaCVCreateGC(GCPtr   pGC)
{
    amigaCVPrivGCPtr  gxPriv;
    if (pGC->depth == 1)
	return mfbCreateGC(pGC);
#if AMIGAMAXDEPTH == 32
    if (!amigaCfbCreateGC(pGC))
#else
    if (!cfbCreateGC(pGC))
#endif
	return FALSE;
    if (pGC->depth == 8)
      {
	pGC->ops = &amiga8CVNonTEOps;
	pGC->funcs = &amiga8CVGCFuncs;
      }
    else if (pGC->depth == 16 || pGC->depth == 15)
      {
	pGC->ops = &amiga16CVNonTEOps;
	pGC->funcs = &amiga16CVGCFuncs;
      }
    else
      {
	pGC->ops = &amiga32CVNonTEOps;
	pGC->funcs = &amiga32CVGCFuncs;
      }

    gxPriv = amigaCVGetGCPrivate(pGC);
    gxPriv->type = DRAWABLE_WINDOW;
    gxPriv->stipple = 0;

    return TRUE;
}



void
amigaCVCopyWindow(WindowPtr pWin, DDXPointRec ptOldOrg, RegionPtr prgnSrc)
{
   RegionPtr prgnDst;
   register BoxPtr pbox, pboxOrig;
   register int dx, dy;
   register int i, nbox;
   short direction = 0;
   unsigned int *ordering;
   GC    dummyGC;
   fbFd *inf = amigaInfo(pWin->drawable.pScreen);
   volatile caddr_t vgaBase = (volatile caddr_t)(inf->regs);

   dummyGC.subWindowMode = ClipByChildren;	/* ~IncludeInferiors */

   prgnDst = REGION_CREATE(pWin->drawable.pScreen, NULL, 1);

   if ((dx = ptOldOrg.x - pWin->drawable.x) > 0)
      direction |= INC_X;

   if ((dy = ptOldOrg.y - pWin->drawable.y) > 0)
      direction |= INC_Y;

   REGION_TRANSLATE(pWin->drawable.pScreen, prgnSrc, -dx, -dy);
   REGION_INTERSECT(pWin->drawable.pScreen, prgnDst, &pWin->borderClip, prgnSrc);

   pboxOrig = REGION_RECTS(prgnDst);
   nbox = REGION_NUM_RECTS(prgnDst);

   ordering = (unsigned int *)ALLOCATE_LOCAL(nbox * sizeof(unsigned int));

   if (!ordering) {
      REGION_DESTROY(pWin->drawable.pScreen, prgnDst);
      return;
   }
   amigaCVFindOrdering((DrawablePtr)pWin, (DrawablePtr)pWin, &dummyGC, nbox,
			pboxOrig, ptOldOrg.x, ptOldOrg.y,
			pWin->drawable.x, pWin->drawable.y, ordering);

   BLOCK_CURSOR;
   WaitQueue16_32(3,4);
   S3_OUTW(FRGD_MIX, FSS_BITBLT | MIX_SRC);
   S3_OUTW(BKGD_MIX, BSS_BKGDCOL | MIX_SRC);
   S3_OUTW32(WRT_MASK, ~0);

   if (direction == (INC_X | INC_Y)) {
      for (i = 0; i < nbox; i++) {
	 pbox = &pboxOrig[ordering[i]];

	 WaitQueue(7); PCI_HACK();
	 S3_OUTW(CUR_X, (short)(pbox->x1 + dx));
	 S3_OUTW(CUR_Y, (short)(pbox->y1 + dy));
	 S3_OUTW(DESTX_DIASTP, (short)(pbox->x1));
	 S3_OUTW(DESTY_AXSTP, (short)(pbox->y1));
	 S3_OUTW(MAJ_AXIS_PCNT, (short)(pbox->x2 - pbox->x1 - 1));
	 S3_OUTW(MULTIFUNC_CNTL, MIN_AXIS_PCNT | (short)(pbox->y2 - pbox->y1 - 1));
	 S3_OUTW(CMD, CMD_BITBLT | direction | DRAW | PLANAR | WRTDATA);
      }
   } else if (direction == INC_X) {
      for (i = 0; i < nbox; i++) {
	 pbox = &pboxOrig[ordering[i]];

	 WaitQueue(7); PCI_HACK();
	 S3_OUTW(CUR_X, (short)(pbox->x1 + dx));
	 S3_OUTW(CUR_Y, (short)(pbox->y2 + dy - 1));
	 S3_OUTW(DESTX_DIASTP, (short)(pbox->x1));
	 S3_OUTW(DESTY_AXSTP, (short)(pbox->y2 - 1));
	 S3_OUTW(MAJ_AXIS_PCNT, (short)(pbox->x2 - pbox->x1 - 1));
	 S3_OUTW(MULTIFUNC_CNTL, MIN_AXIS_PCNT | (short)(pbox->y2 - pbox->y1 - 1));
	 S3_OUTW(CMD, CMD_BITBLT | direction | DRAW | PLANAR | WRTDATA);
      }
   } else if (direction == INC_Y) {
      for (i = 0; i < nbox; i++) {
	 pbox = &pboxOrig[ordering[i]];

	 WaitQueue(7); PCI_HACK();
	 S3_OUTW(CUR_X, (short)(pbox->x2 + dx - 1));
	 S3_OUTW(CUR_Y, (short)(pbox->y1 + dy));
	 S3_OUTW(DESTX_DIASTP, (short)(pbox->x2 - 1));
	 S3_OUTW(DESTY_AXSTP, (short)(pbox->y1));
	 S3_OUTW(MAJ_AXIS_PCNT, (short)(pbox->x2 - pbox->x1 - 1));
	 S3_OUTW(MULTIFUNC_CNTL, MIN_AXIS_PCNT | (short)(pbox->y2 - pbox->y1 - 1));
	 S3_OUTW(CMD, CMD_BITBLT | direction | DRAW | PLANAR | WRTDATA);
      }
   } else {
      for (i = 0; i < nbox; i++) {
	 pbox = &pboxOrig[ordering[i]];

	 WaitQueue(7); PCI_HACK();
	 S3_OUTW(CUR_X, (short)(pbox->x2 + dx - 1));
	 S3_OUTW(CUR_Y, (short)(pbox->y2 + dy - 1));
	 S3_OUTW(DESTX_DIASTP, (short)(pbox->x2 - 1));
	 S3_OUTW(DESTY_AXSTP, (short)(pbox->y2 - 1));
	 S3_OUTW(MAJ_AXIS_PCNT, (short)(pbox->x2 - pbox->x1 - 1));
	 S3_OUTW(MULTIFUNC_CNTL, MIN_AXIS_PCNT | (short)(pbox->y2 - pbox->y1 - 1));
	 S3_OUTW(CMD, CMD_BITBLT | direction | DRAW | PLANAR | WRTDATA);
      }
   }

   WaitQueue(2); 
   WaitIdle();

   S3_OUTW(FRGD_MIX, FSS_FRGDCOL | MIX_SRC);
   S3_OUTW(BKGD_MIX, BSS_BKGDCOL | MIX_SRC);
   UNBLOCK_CURSOR;

   REGION_DESTROY(pWin->drawable.pScreen, prgnDst);
   DEALLOCATE_LOCAL(ordering);
}


void
amigaCVadjustVirtual(volatile caddr_t ba)
{
        unsigned char cr50, test;
	fbFd *inf = amigaInfo(amigaCVsavepScreen);
	int depth = inf->info.gd_planes; 
	int HDE;
	

        cr50 = RCrt(ba, CRT_ID_EXT_SYS_CNTL_1);
        cr50 &= ~0xc1;
        /* Set up graphics engine */
        switch (amigaVirtualWidth) {
           case 1024:
                cr50 |= 0x00;
                break;   
           case 640:
                cr50 |= 0x40;
                break;
           case 800:
                cr50 |= 0x80;
                break;
           case 1280:
                cr50 |= 0xc0;
                break;
           case 1152:
                cr50 |= 0x01;
                break;
           case 1600:
                cr50 |= 0x81;
                break;
           default: /* XXX*/
                break;
        }





        WCrt (ba, CRT_ID_EXT_SYS_CNTL_1, cr50);

	switch (depth) {
		case 8:
			HDE = amigaVirtualWidth / 8;
			break;
		case 15: case 16:
                        HDE = amigaVirtualWidth / 4;
                        break;
		case 24: case 32:
                        HDE = amigaVirtualWidth / 2;
                        break;
		default: /* ??? */
			__dolog("wrong depth %d\n", depth);
	}

	WCrt(ba, CRT_ID_SCREEN_OFFSET, (HDE & 0xff));

	__dolog("VW %d, HDE %x\n", (int)amigaVirtualWidth, (int)HDE);

	test = RCrt(ba, CRT_ID_EXT_SYS_CNTL_2);

	test &= ~0x30;
	test |= (HDE >> 4) & 0x30;
	/* HDE Overflow in bits 4-5 */ 
	WCrt(ba, CRT_ID_EXT_SYS_CNTL_2, test); 
}





Bool
amigaCVGXInit(ScreenPtr	pScreen, fbFd	*fb)
{
    unsigned int	    mode;
    register long   r;
    fbFd *inf = amigaInfo(pScreen);
    volatile caddr_t ba = (volatile caddr_t) (inf->regs);


    if (serverGeneration != amigaCVGeneration)
    {
	amigaCVScreenPrivateIndex = AllocateScreenPrivateIndex();
	if (amigaCVScreenPrivateIndex == -1)
	    return FALSE;
	amigaCVGCPrivateIndex = AllocateGCPrivateIndex();
	amigaCVWindowPrivateIndex = AllocateWindowPrivateIndex();
	amigaCVGeneration = serverGeneration;
    }
    if (!AllocateGCPrivate(pScreen, amigaCVGCPrivateIndex, sizeof(amigaCVPrivGCRec)))
	return FALSE;
    if (!AllocateWindowPrivate(pScreen, amigaCVWindowPrivateIndex, 0))
	return FALSE;
    /*
     * Replace various screen functions
     */

    if (fb->info.gd_planes == 8)
      {
	pScreen->CreateGC = amigaCVCreateGC;
	pScreen->CopyWindow = amigaCVCopyWindow;
      }
    else if (fb->info.gd_planes == 16 || fb->info.gd_planes == 15)
      {
	pScreen->CreateGC = amigaCVCreateGC;
	pScreen->CopyWindow = amigaCVCopyWindow;

      }
    else
      {
	pScreen->CreateGC = amigaCVCreateGC;
	pScreen->CopyWindow = amigaCVCopyWindow;

      }

      if  (amigaVirtualWidth) {
	amigaCVadjustVirtual(ba);
    }

    /* Black border if < 80 MHz */
#if 0
     if (amigaFlipPixels) 
	WAttr(ba, ACT_ID_OVERSCAN_COLOR, 0);
     else 
        WAttr(ba, ACT_ID_OVERSCAN_COLOR, 1);
#endif

    /* Init Image variables and Pixmap cache */
    amigaCVImageInit(fb);

    return TRUE;
}

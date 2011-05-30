#ifndef lint
static char *rid="$XConsortium: amigaGX.c,v 1.26 94/04/17 20:29:38 kaleb Exp $";
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
#include	"cfb16.h"
#include	"cfb32.h"
#include	"cfbmskbits.h"
#include	"cfb8bit.h"
#include	"fastblt.h"
#include	"mergerop.h"
#include	"amigaGX.h"
#include	"migc.h"
#include	"mi.h"
#include	"mispans.h"


/* make cfb somewhat more consistent.. */
#define cfb8CopyArea			cfbCopyArea
#define cfb8BitBlt			cfbBitBlt
#define cfb8CopyPlane			cfbCopyPlane
#define cfb8CopyPlaneReduce		cfbCopyPlaneReduce
#define cfb8CopyPlane8to1 		cfbCopyPlane8to1
#define cfb8PolyGlyphBlt8		cfbPolyGlyphBlt8
#define cfb8NonTEOps			cfbNonTEOps
#define cfb8ValidateGC			cfbValidateGC
#define cfb8CopyRotatePixmap		cfbCopyRotatePixmap
#define cfb8DestroyPixmap		cfbDestroyPixmap
#define cfb8ReduceRasterOp 		cfbReduceRasterOp
#define cfb8ZeroPolyArcSS8Xor 		cfbZeroPolyArcSS8Xor
#define cfb8ZeroPolyArcSS8Copy		cfbZeroPolyArcSS8Copy
#define cfb8ZeroPolyArcSS8General	cfbZeroPolyArcSS8General
#define cfb8LineSS			cfbLineSS
#define cfb8LineSD			cfbLineSD
#define cfb8SegmentSS			cfbSegmentSS
#define cfb8SegmentSD			cfbSegmentSD
#define cfb8Tile32FSCopy		cfbTile32FSCopy
#define cfb8Tile32FSGeneral		cfbTile32FSGeneral
#define cfb8UnnaturalTileFS		cfbUnnaturalTileFS
#define cfb8UnnaturalStippleFS		cfbUnnaturalStippleFS
#define cfb8PolyFillRect 		cfbPolyFillRect
#define cfb8PushPixels8			cfbPushPixels8
#define cfb8CreateWindow		cfbCreateWindow
#define cfb8DestroyWindow		cfbDestroyWindow
#define cfb8XRotatePixmap		cfbXRotatePixmap
#define cfb8YRotatePixmap		cfbYRotatePixmap
#define cfb8CopyRotatePixmap		cfbCopyRotatePixmap
#define cfb8FillBoxTile32		cfbFillBoxTile32
#define cfb8FillBoxTileOdd		cfbFillBoxTileOdd
#define cfb8SetSpans			cfbSetSpans
#define cfb8PutImage			cfbPutImage
#define cfb8PolyPoint			cfbPolyPoint


#define cfb8GetTypedWidthAndPointer(pDrawable, width, pointer, wtype, ptype) {\
    PixmapPtr   _pPix; \
    if ((pDrawable)->type != DRAWABLE_PIXMAP) \
	_pPix = cfbGetScreenPixmap(pDrawable->pScreen); \
    else \
	_pPix = (PixmapPtr) (pDrawable); \
    (pointer) = (ptype *) _pPix->devPrivate.ptr; \
    (width) = ((int) _pPix->devKind) / sizeof (wtype); \
}

#define cfb8GetByteWidthAndPointer(pDrawable, width, pointer) \
    cfb8GetTypedWidthAndPointer(pDrawable, width, pointer, unsigned char, unsigned char)

#define cfb8GetLongWidthAndPointer(pDrawable, width, pointer) \
    cfb8GetTypedWidthAndPointer(pDrawable, width, pointer, unsigned long, unsigned long)

#define cfb16GetScreenPixmap(s)	((PixmapPtr) (s)->devPrivates[cfb16ScreenPrivateIndex].ptr)
#define cfb16GetTypedWidthAndPointer(pDrawable, width, pointer, wtype, ptype) {\
    PixmapPtr   _pPix; \
    if ((pDrawable)->type != DRAWABLE_PIXMAP) \
	_pPix = cfb16GetScreenPixmap(pDrawable->pScreen); \
    else \
	_pPix = (PixmapPtr) (pDrawable); \
    (pointer) = (ptype *) _pPix->devPrivate.ptr; \
    (width) = ((int) _pPix->devKind) / sizeof (wtype); \
}

#define cfb16GetByteWidthAndPointer(pDrawable, width, pointer) \
    cfb16GetTypedWidthAndPointer(pDrawable, width, pointer, unsigned char, unsigned char)

#define cfb16GetLongWidthAndPointer(pDrawable, width, pointer) \
    cfb16GetTypedWidthAndPointer(pDrawable, width, pointer, unsigned long, unsigned long)

#define cfb32GetScreenPixmap(s)	((PixmapPtr) (s)->devPrivates[cfb32ScreenPrivateIndex].ptr)
#define cfb32GetTypedWidthAndPointer(pDrawable, width, pointer, wtype, ptype) {\
    PixmapPtr   _pPix; \
    if ((pDrawable)->type != DRAWABLE_PIXMAP) \
	_pPix = cfb32GetScreenPixmap(pDrawable->pScreen); \
    else \
	_pPix = (PixmapPtr) (pDrawable); \
    (pointer) = (ptype *) _pPix->devPrivate.ptr; \
    (width) = ((int) _pPix->devKind) / sizeof (wtype); \
}

#define cfb32GetByteWidthAndPointer(pDrawable, width, pointer) \
    cfb32GetTypedWidthAndPointer(pDrawable, width, pointer, unsigned char, unsigned char)

#define cfb32GetLongWidthAndPointer(pDrawable, width, pointer) \
    cfb32GetTypedWidthAndPointer(pDrawable, width, pointer, unsigned long, unsigned long)

extern int cfb16ScreenPrivateIndex, cfb32ScreenPrivateIndex;


/* blitter ops works like this:
 * - there are three input sources to logically combine and form a 
 *   result
 * - these are: source-bitmap (S), pattern-bitmap (P), destination-bitmap (D)
 * - there are 8 possible combinations of the above 3 and their negated sources
 * - bit values are ("s" means negated "S"):
 *   7: PSD  6: PSd  5: PsD  4: Psd  3: pSD  2: pSd  1: psD  0: psd
 *   or as a tree (sort of):
 *             P                 p
 *       S         s        S         s
 *    D    d    D    d    D   d     D   d
 * - that's how the following tables are built.
 *******************************************************************/


/* blitting rops: there are two tables each, one ignoring a pattern, 
 * second using pattern as a plane mask */

static Uchar blit_rop_table[16] = {
    0x00,	/* GXclear */
    0x88,	/* GXand */
    0x44,	/* GXandReverse */
    0xcc,	/* GXcopy */
    0x22,	/* GXandInverted */
    0xaa,	/* GXnoop */
    0x66,	/* GXxor */
    0xee,	/* GXor */
    0x11,	/* GXnor */
    0x99,	/* GXequiv */
    0x55,	/* GXinvert */
    0xdd,	/* GXorReverse */
    0x33,	/* GXcopyInverted */
    0xbb,	/* GXorInverted */
    0x77,	/* GXnand */
    0xff,	/* GXset */
};

static Uchar blit_rop_table_masked[16] = {
    0x0A,	/* GXclear */
    0x8A,	/* GXand */
    0x4A,	/* GXandReverse */
    0xcA,	/* GXcopy */
    0x2A,	/* GXandInverted */
    0xaA,	/* GXnoop */
    0x6A,	/* GXxor */
    0xeA,	/* GXor */
    0x1A,	/* GXnor */
    0x9A,	/* GXequiv */
    0x5A,	/* GXinvert */
    0xdA,	/* GXorReverse */
    0x3A,	/* GXcopyInverted */
    0xbA,	/* GXorInverted */
    0x7A,	/* GXnand */
    0xfA,	/* GXset */
};


/* rops for drawing stipples. these can only be used with a planemask of all
 * 1 since the pattern is now used for drawing. We're again using
 * color-expansion here, use of the transparency-bit decides whether the
 * operation is "stipple" or "opaque stipple". */

static Uchar stipple_rop_table[16] = {
    0x00,	/* GXclear */
    0xa0,	/* GXand */
    0x50, 	/* GXandReverse */
    0xf0,	/* GXcopy */
    0x0a,	/* GXandInverted */
    0xaa,	/* GXnoop */
    0x5a,	/* GXxor */
    0xfa,	/* GXor */
    0x05,	/* GXnor */
    0xa5,	/* GXequiv */
    0x55,	/* GXinvert */
    0xf5,	/* GXorReverse */
    0x0f,	/* GXcopyInverted */
    0xaf,	/* GXorInverted */
    0x5f,	/* GXnand */
    0xff,	/* GXset */
};

/* those != 0 need the src-register to be setup */
static char optabs[] = {
	   0,   -1,   -1,   -1,   -1,    0,   -1,   -1,
	  -1,   -1,    0,   -1,   -1,   -1,   -1,    0
};

int	amigaGXScreenPrivateIndex;
int	amigaGXGCPrivateIndex;
int	amigaGXWindowPrivateIndex;
int	amigaGXGeneration;

#if 0
#define amigaGXFillSpan(acm,inf,y,x1,x2,bpp) \
{ VUint *feed = (VUint *) inf->fb;							\
  Uint ltmp;									\
  ltmp = *feed;									\
  RZ3Blit1toFB(acm,0,0,x1,y,(((x2)-(x1))+1),1,inf->info.gd_fbwidth,bpp,0); 	\
  while ((acm->status & 1) == 0) *feed = ~0;					\
  *feed = ltmp; }
#else
static void __inline
amigaGXFillSpan(acm, inf, y, x1, x2, bpp)
     struct ACM *acm;
     fbFd *inf;
     int y, x1, x2, bpp;
{ VUint *feed = (VUint *) inf->fb;
  Uint ltmp;
  ltmp = *feed;
  RZ3Blit1toFB(acm,0,0,x1,y,(((x2)-(x1))+1),1,inf->info.gd_fbwidth,bpp,0);
  while ((acm->status & 1) == 0) *feed = ~0;
  *feed = ltmp; 
}
#endif



/*
  amigaGXDoBitBlt
  =============
  Bit Blit for all window to window blits.
*/
static void
amigaGXDoBitblt(pSrc, pDst, alu, prgnDst, pptSrc, planemask)
    DrawablePtr	    pSrc, pDst;
    int		    alu;
    RegionPtr	    prgnDst;
    DDXPointPtr	    pptSrc;
    unsigned long   planemask;
{
    register long r;
    register BoxPtr pboxTmp;
    register DDXPointPtr pptTmp;
    register int nbox;
    BoxPtr pboxNext,pboxBase,pbox;
    fbFd *inf = amigaInfo(pSrc->pScreen);
    struct ACM *acm = (struct ACM *) (inf->regs + ACM_OFFSET);
    short bpp = inf->info.gd_planes >> 3;
    Uint mask = (1<<bpp) - 1;

    if ((mask & planemask) == mask)
      {
	/* no masking, simple rop table */
	RZ3BlitInit(acm, blit_rop_table, alu);
      }
    else
      {
	/* use pattern as mask, and replicate the planemask as many 
	 * times as necessary. */
	RZ3BlitInit(acm, blit_rop_table_masked, alu);
	RZ3MaskInit(acm, inf->fb, planemask, bpp);
      }

    pbox = REGION_RECTS(prgnDst);
    nbox = REGION_NUM_RECTS(prgnDst);

    /* need to blit rectangles in different orders, depending on the 
       direction of copy so that an area isnt overwritten before it is 
       blitted */
    if( (pptSrc->y < pbox->y1) && (nbox > 1) ){

	if( (pptSrc->x < pbox->x1) && (nbox > 1) ){

	    /* reverse order of bands and rects in each band */
	    pboxTmp=pbox+nbox;
	    pptTmp=pptSrc+nbox;
	    
	    while (nbox--){
		pboxTmp--;
		pptTmp--;	
		RZ3BlitFB2FB(acm, alu,
			     pptTmp->x, pptTmp->y,
			     pboxTmp->x1, pboxTmp->y1,
			     (pboxTmp->x2 - pboxTmp->x1),
			     (pboxTmp->y2 - pboxTmp->y1),
			     inf->info.gd_fbwidth, bpp);
		RZ3WaitDone(acm);
	    }
	}
	else{

	    /* keep ordering in each band, reverse order of bands */
	    pboxBase = pboxNext = pbox+nbox-1;

	    while (pboxBase >= pbox){ /* for each band */

		/* find first box in band */
		while ((pboxNext >= pbox) &&
		       (pboxBase->y1 == pboxNext->y1))
		    pboxNext--;
		
		pboxTmp = pboxNext+1;			/* first box in band */
		pptTmp = pptSrc + (pboxTmp - pbox);	/* first point in band */
		
		while (pboxTmp <= pboxBase){ /* for each box in band */
		    RZ3BlitFB2FB(acm, alu,
				 pptTmp->x, pptTmp->y,
				 pboxTmp->x1, pboxTmp->y1,
				 (pboxTmp->x2 - pboxTmp->x1),
				 (pboxTmp->y2 - pboxTmp->y1),
				 inf->info.gd_fbwidth, bpp);
		    ++pboxTmp;
		    ++pptTmp;	
		    RZ3WaitDone(acm);
		}
		pboxBase = pboxNext;
	    }
	}
    }
    else{

	if( (pptSrc->x < pbox->x1) && (nbox > 1) ){
	
	    /* reverse order of rects in each band */
	    pboxBase = pboxNext = pbox;

	    while (pboxBase < pbox+nbox){ /* for each band */

		/* find last box in band */
		while ((pboxNext < pbox+nbox) &&
		       (pboxNext->y1 == pboxBase->y1))
		    pboxNext++;
		
		pboxTmp = pboxNext;			/* last box in band */
		pptTmp = pptSrc + (pboxTmp - pbox);	/* last point in band */
		
		while (pboxTmp != pboxBase){ /* for each box in band */
		    --pboxTmp;
		    --pptTmp;	
		    RZ3BlitFB2FB(acm, alu,
				 pptTmp->x, pptTmp->y,
				 pboxTmp->x1, pboxTmp->y1,
				 (pboxTmp->x2 - pboxTmp->x1),
				 (pboxTmp->y2 - pboxTmp->y1),
				 inf->info.gd_fbwidth, bpp);
		    RZ3WaitDone(acm);
		}
		pboxBase = pboxNext;
	    }
	}
	else{

	    /* dont need to change order of anything */
	    pptTmp=pptSrc;
	    pboxTmp=pbox;
	    
	    while(nbox--){
		RZ3BlitFB2FB(acm, alu,
			     pptTmp->x, pptTmp->y,
			     pboxTmp->x1, pboxTmp->y1,
			     (pboxTmp->x2 - pboxTmp->x1),
			     (pboxTmp->y2 - pboxTmp->y1),
			     inf->info.gd_fbwidth, bpp);
		++pboxTmp;
		++pptTmp;	
		RZ3WaitDone(acm);
	    }
	}
    }
}

extern RegionPtr cfb8CopyArea(), cfb16CopyArea(), cfb32CopyArea();
extern RegionPtr cfb8CopyPlane(), cfb16CopyPlane(), cfb32CopyPlane();

extern RegionPtr cfb8BitBlt(), cfb16BitBlt(), cfb32BitBlt();

RegionPtr
amiga8GXCopyArea(pSrcDrawable, pDstDrawable,
		 pGC, srcx, srcy, width, height, dstx, dsty)
     register DrawablePtr pSrcDrawable;
     register DrawablePtr pDstDrawable;
     GC *pGC;
     int srcx, srcy;
     int width, height;
     int dstx, dsty;
{
__dolog("amiga8GXCopyArea -> %s\n",
	pSrcDrawable->type != DRAWABLE_WINDOW ? "cfbCopyArea" : "cfbBitBlt");

    if (pSrcDrawable->type != DRAWABLE_WINDOW 
	|| pDstDrawable->type != DRAWABLE_WINDOW)
      return cfb8CopyArea (pSrcDrawable, pDstDrawable,
			   pGC, srcx, srcy, width, height, dstx, dsty);
    return cfb8BitBlt (pSrcDrawable, pDstDrawable,
		       pGC, srcx, srcy, width, height, dstx, dsty, 
		       amigaGXDoBitblt, 0);
}

RegionPtr
amiga16GXCopyArea(pSrcDrawable, pDstDrawable,
		  pGC, srcx, srcy, width, height, dstx, dsty)
     register DrawablePtr pSrcDrawable;
     register DrawablePtr pDstDrawable;
     GC *pGC;
     int srcx, srcy;
     int width, height;
     int dstx, dsty;
{
__dolog("amiga16GXCopyArea -> %s\n",
	pSrcDrawable->type != DRAWABLE_WINDOW ? "cfbCopyArea" : "cfbBitBlt");

    if (pSrcDrawable->type != DRAWABLE_WINDOW 
	|| pDstDrawable->type != DRAWABLE_WINDOW)
      return cfb16CopyArea (pSrcDrawable, pDstDrawable,
			    pGC, srcx, srcy, width, height, dstx, dsty);
    return cfb16BitBlt (pSrcDrawable, pDstDrawable,
			pGC, srcx, srcy, width, height, dstx, dsty, 
			amigaGXDoBitblt, 0);
}

RegionPtr
amiga24GXCopyArea(pSrcDrawable, pDstDrawable,
		  pGC, srcx, srcy, width, height, dstx, dsty)
     register DrawablePtr pSrcDrawable;
     register DrawablePtr pDstDrawable;
     GC *pGC;
     int srcx, srcy;
     int width, height;
     int dstx, dsty;
{
__dolog("amiga24GXCopyArea -> %s\n",
	pSrcDrawable->type != DRAWABLE_WINDOW ? "cfbCopyArea" : "cfbBitBlt");

    if (pSrcDrawable->type != DRAWABLE_WINDOW 
	&& pDstDrawable->type != DRAWABLE_WINDOW)
      return cfb32CopyArea (pSrcDrawable, pDstDrawable,
			    pGC, srcx, srcy, width, height, dstx, dsty);
    else if (pSrcDrawable->type != DRAWABLE_WINDOW 
	|| pDstDrawable->type != DRAWABLE_WINDOW)
      return miCopyArea (pSrcDrawable, pDstDrawable,
			 pGC, srcx, srcy, width, height, dstx, dsty);
    else
      return cfb32BitBlt (pSrcDrawable, pDstDrawable,
			  pGC, srcx, srcy, width, height, dstx, dsty, 
			  amigaGXDoBitblt, 0);
}


static unsigned long	copyPlaneFG, copyPlaneBG;

static void
amigaGXCopyPlane1to8 (pSrcDrawable, pDstDrawable, rop, prgnDst, pptSrc, planemask, bitPlane)
    DrawablePtr pSrcDrawable;
    DrawablePtr pDstDrawable;
    int	rop;
    RegionPtr prgnDst;
    DDXPointPtr pptSrc;
    unsigned long planemask;
    unsigned long   bitPlane;
{
    int			srcx, srcy, dstx, dsty, width, height;
    int			dstLastx, dstRightx;
    int			xoffSrc, widthSrc, widthRest;
    int			widthLast;
    unsigned long	*psrcBase, *psrcLine, *psrc;
    unsigned char	bits, tmp, lastTmp;
    register int	leftShift, rightShift;
    register int	nl, nlMiddle;
    int			nbox;
    BoxPtr		pbox;
    register int	r;
    fbFd *inf = amigaInfo(pSrcDrawable->pScreen);
    struct ACM *acm = (struct ACM *) (inf->regs + ACM_OFFSET);
    short bpp = inf->info.gd_planes >> 3;
    VUint *feed = (VUint *) inf->fb;
    Uint ltmp;
    Uint mask = (1<<bpp) - 1;

    if (bpp == 1)
      {
	cfb8GetLongWidthAndPointer (pSrcDrawable, widthSrc, psrcBase);
      }
    else if (bpp == 2)
      {
	cfb16GetLongWidthAndPointer (pSrcDrawable, widthSrc, psrcBase);
      }
    else
      {
	cfb32GetLongWidthAndPointer (pSrcDrawable, widthSrc, psrcBase);
      }

    if ((mask & planemask) == mask)
      {
	/* no masking, simple rop table */
	RZ3BlitInit(acm, blit_rop_table, rop);
      }
    else
      {
	/* use pattern as mask, and replicate the planemask as many 
	 * times as necessary. */
	RZ3BlitInit(acm, blit_rop_table_masked, rop);
	RZ3MaskInit(acm, inf->fb, planemask, bpp);
      }

    ltmp = copyPlaneFG;
    M2I(ltmp);
    acm->fg = ltmp;
    ltmp = copyPlaneBG;
    M2I(ltmp);
    acm->bg = ltmp;
      
    nbox = REGION_NUM_RECTS(prgnDst);
    pbox = REGION_RECTS(prgnDst);
    while (nbox--)
    {
	dstx = pbox->x1;
	dsty = pbox->y1;
	srcx = pptSrc->x;
	srcy = pptSrc->y;
	dstLastx = pbox->x2;
	width = dstLastx - dstx;
	height = pbox->y2 - dsty;
	pbox++;
	pptSrc++;
	if (!width)
	    continue;
	psrc = psrcBase + srcy * widthSrc + (srcx >> 5);
__dolog("psrcBase=0x%08x, srcy=%d, srcx=%d, psrc=0x%08x\n",
	psrcBase, srcy, srcx, psrc);
	dstLastx--;
	dstRightx = dstx + 31;
	nlMiddle = (width + 31) >> 5;
	widthLast = width & 31;
	xoffSrc = srcx & 0x1f;
	leftShift = xoffSrc;
	rightShift = 32 - leftShift;
	widthRest = widthSrc - nlMiddle;
	if (widthLast)
	    nlMiddle--;

	RZ3Blit1toFB(acm,rop,xoffSrc,dstx,dsty,width,height,
		     inf->info.gd_fbwidth, bpp, 0);

__dolog("widthSrc = %d, nlMiddle = %d, widthLast = %d\n",
	widthSrc, nlMiddle, widthLast);
	while (height--)
	  {
	    nl = nlMiddle;
	    while (nl--)
	      {
		 if (acm->status & 0x10)
		   while (!(acm->status & 0x04)) ;
__dolog("%08x ", *psrc);
	         *feed = *psrc++;
	      }
	    if (widthLast) 
	      {
		 if (acm->status & 0x10)
		   while (!(acm->status & 0x04)) ;
__dolog("{%08x} ", *psrc);
	         *feed = *psrc++;
	      }
__dolog("\n");
	    psrc += widthRest;
	  }

	RZ3WaitDone(acm);
    }
}

RegionPtr amiga8GXCopyPlane(pSrcDrawable, pDstDrawable,
			    pGC, srcx, srcy, width, height, dstx, dsty, 
			    bitPlane)
    DrawablePtr 	pSrcDrawable;
    DrawablePtr		pDstDrawable;
    GCPtr		pGC;
    int 		srcx, srcy;
    int 		width, height;
    int 		dstx, dsty;
    unsigned long	bitPlane;
{
    RegionPtr		ret;
    int			(*doBitBlt)();

__dolog("amigaGXCopyPlane: s(%d) -> d(%d), plane = %d\n",
	pSrcDrawable->bitsPerPixel, pDstDrawable->bitsPerPixel, bitPlane);

    if (pDstDrawable->type != DRAWABLE_WINDOW)
      return cfb8CopyPlane(pSrcDrawable, pDstDrawable,
			  pGC, srcx, srcy, width, height,
			  dstx, dsty, bitPlane);

    if (pSrcDrawable->bitsPerPixel == 1 && pDstDrawable->bitsPerPixel == 8)
    {
    	if (bitPlane == 1)
	{
	    copyPlaneFG = pGC->fgPixel;
	    copyPlaneBG = pGC->bgPixel;
    	    ret = cfb8BitBlt (pSrcDrawable, pDstDrawable,
	    	    pGC, srcx, srcy, width, height, dstx, dsty, amigaGXCopyPlane1to8, bitPlane);
	}
	else
	    ret = miHandleExposures (pSrcDrawable, pDstDrawable,
	    	pGC, srcx, srcy, width, height, dstx, dsty, bitPlane);
    }
    else if (pSrcDrawable->bitsPerPixel == 8 && pDstDrawable->bitsPerPixel == 1)
    {
	extern	int InverseAlu[16];
	int oldalu;

	oldalu = pGC->alu;
    	if ((pGC->fgPixel & 1) == 0 && (pGC->bgPixel&1) == 1)
	    pGC->alu = InverseAlu[pGC->alu];
    	else if ((pGC->fgPixel & 1) == (pGC->bgPixel & 1))
	    pGC->alu = mfbReduceRop(pGC->alu, pGC->fgPixel);
	ret = cfb8CopyPlaneReduce (pSrcDrawable, pDstDrawable,
		    pGC, srcx, srcy, width, height, dstx, dsty, cfb8CopyPlane8to1, bitPlane);
	pGC->alu = oldalu;
    }
    else
    {
	PixmapPtr	pBitmap;
	ScreenPtr	pScreen = pSrcDrawable->pScreen;
	GCPtr		pGC1;
	unsigned long	fg, bg;

	pBitmap = (*pScreen->CreatePixmap) (pScreen, width, height, 1);
	if (!pBitmap)
	    return NULL;
	pGC1 = GetScratchGC (1, pScreen);
	if (!pGC1)
	{
	    (*pScreen->DestroyPixmap) (pBitmap);
	    return NULL;
	}
	/*
	 * don't need to set pGC->fgPixel,bgPixel as copyPlane8to1
	 * ignores pixel values, expecting the rop to "do the
	 * right thing", which GXcopy will.
	 */
	ValidateGC ((DrawablePtr) pBitmap, pGC1);
	/* no exposures here, scratch GC's don't get graphics expose */
	(void) cfb8CopyPlaneReduce (pSrcDrawable, (DrawablePtr) pBitmap,
			  pGC1, srcx, srcy, width, height, 0, 0, 
			  cfb8CopyPlane8to1, bitPlane);
	copyPlaneFG = pGC->fgPixel;
	copyPlaneBG = pGC->bgPixel;
	(void) cfb8BitBlt ((DrawablePtr) pBitmap, pDstDrawable, pGC,
			  0, 0, width, height, dstx, dsty, 
			  amigaGXCopyPlane1to8, 1);
	FreeScratchGC (pGC1);
	(*pScreen->DestroyPixmap) (pBitmap);
	/* compute resultant exposures */
	ret = miHandleExposures (pSrcDrawable, pDstDrawable, pGC,
				 srcx, srcy, width, height,
				 dstx, dsty, bitPlane);
    }
    return ret;
}

RegionPtr amiga16GXCopyPlane(pSrcDrawable, pDstDrawable,
	    pGC, srcx, srcy, width, height, dstx, dsty, bitPlane)
    DrawablePtr 	pSrcDrawable;
    DrawablePtr		pDstDrawable;
    GCPtr		pGC;
    int 		srcx, srcy;
    int 		width, height;
    int 		dstx, dsty;
    unsigned long	bitPlane;
{
    RegionPtr		ret;
    int			(*doBitBlt)();

__dolog("amigaGXCopyPlane: s(%d) -> d(%d), plane = %d\n",
	pSrcDrawable->bitsPerPixel, pDstDrawable->bitsPerPixel, bitPlane);

    if (pDstDrawable->type != DRAWABLE_WINDOW)
      return cfb16CopyPlane(pSrcDrawable, pDstDrawable,
			  pGC, srcx, srcy, width, height,
			  dstx, dsty, bitPlane);

    if (pSrcDrawable->bitsPerPixel == 1 && pDstDrawable->bitsPerPixel == 16)
    {
    	if (bitPlane == 1)
	{
	    copyPlaneFG = pGC->fgPixel;
	    copyPlaneBG = pGC->bgPixel;
    	    ret = cfb16BitBlt (pSrcDrawable, pDstDrawable,
	    	    pGC, srcx, srcy, width, height, dstx, dsty, amigaGXCopyPlane1to8, bitPlane);
	}
	else
	    ret = miHandleExposures (pSrcDrawable, pDstDrawable,
	    	pGC, srcx, srcy, width, height, dstx, dsty, bitPlane);
    }
    else {
      ret = cfb16CopyPlane (pSrcDrawable, pDstDrawable, pGC, srcx, srcy,
			    width, height, dstx, dsty, bitPlane);
    }
    return ret;
}

RegionPtr amiga24GXCopyPlane(pSrcDrawable, pDstDrawable,
	    pGC, srcx, srcy, width, height, dstx, dsty, bitPlane)
    DrawablePtr 	pSrcDrawable;
    DrawablePtr		pDstDrawable;
    GCPtr		pGC;
    int 		srcx, srcy;
    int 		width, height;
    int 		dstx, dsty;
    unsigned long	bitPlane;
{
    RegionPtr		ret;
    int			(*doBitBlt)();

__dolog("amigaGXCopyPlane: s(%d) -> d(%d), plane = %d\n",
	pSrcDrawable->bitsPerPixel, pDstDrawable->bitsPerPixel, bitPlane);

    if (pSrcDrawable->type != DRAWABLE_WINDOW 
	&& pDstDrawable->type != DRAWABLE_WINDOW)
      return cfb32CopyPlane(pSrcDrawable, pDstDrawable,
			  pGC, srcx, srcy, width, height,
			  dstx, dsty, bitPlane);
    else if (pDstDrawable->type != DRAWABLE_WINDOW)
      return miCopyPlane(pSrcDrawable, pDstDrawable,
			  pGC, srcx, srcy, width, height,
			  dstx, dsty, bitPlane);

    if (pSrcDrawable->bitsPerPixel == 1)
    {
    	if (bitPlane == 1)
	{
	    copyPlaneFG = pGC->fgPixel;
	    copyPlaneBG = pGC->bgPixel;
    	    ret = cfb32BitBlt (pSrcDrawable, pDstDrawable,
	    	    pGC, srcx, srcy, width, height, dstx, dsty, amigaGXCopyPlane1to8, bitPlane);
	}
	else
	    ret = miHandleExposures (pSrcDrawable, pDstDrawable,
	    	pGC, srcx, srcy, width, height, dstx, dsty, bitPlane);
    }
    else {
      ret = miCopyPlane(pSrcDrawable, pDstDrawable,
			  pGC, srcx, srcy, width, height,
			  dstx, dsty, bitPlane);
    }
    return ret;
}

void
amigaGXFillRectAll (pDrawable, pGC, nBox, pBox)
    DrawablePtr	    pDrawable;
    GCPtr	    pGC;
    int		    nBox;
    BoxPtr	    pBox;
{
    register int	r;
    fbFd *inf = amigaInfo(pDrawable->pScreen);
    struct ACM *acm = (struct ACM *) (inf->regs + ACM_OFFSET);
    short bpp = inf->info.gd_planes >> 3;
    VUint *feed = (VUint *) inf->fb;
    Uint ltmp;

    /* can't deal with stipple-operations in here since we use the pattern
       channel to implement the planemasking feature. */
    
    Uint mask = (1<<bpp) - 1;

    __dolog("GXFillRectAll: nbox=%d, alu=0x%02x, planemask=0x%08x, fg=%d, bg=%d=n", nBox, pGC->alu, pGC->planemask, pGC->fgPixel, pGC->bgPixel);

    if ((mask & pGC->planemask) == mask)
      {
	/* no masking, simple rop table */
	RZ3BlitInit(acm, blit_rop_table, pGC->alu);
      }
    else
      {
	/* use pattern as mask, and replicate the planemask as many 
	 * times as necessary. */
	RZ3BlitInit(acm, blit_rop_table_masked, pGC->alu);
	RZ3MaskInit(acm, inf->fb, pGC->planemask, bpp);
      }

    ltmp = pGC->fgPixel;
    M2I(ltmp);
    acm->fg = ltmp;
    ltmp = pGC->bgPixel;
    M2I(ltmp);
    acm->bg = ltmp;

    /* just to be sure.. */
    ltmp = *feed;
    while (nBox--) {
	RZ3Blit1toFB(acm, 0, 0,
		     pBox->x1, pBox->y1,
		     (pBox->x2 - pBox->x1),
		     (pBox->y2 - pBox->y1),
		     inf->info.gd_fbwidth, bpp, 0);
	while ((acm->status & 1) == 0) *feed = ~0;
	pBox++;
    }
    *feed = ltmp;
}

#define NUM_STACK_RECTS	1024

void
amigaGXPolyFillRect(pDrawable, pGC, nrectFill, prectInit)
    DrawablePtr pDrawable;
    register GCPtr pGC;
    int		nrectFill; 	/* number of rectangles to fill */
    xRectangle	*prectInit;  	/* Pointer to first rectangle to fill */
{
    xRectangle	    *prect;
    RegionPtr	    prgnClip;
    register BoxPtr pbox;
    register BoxPtr pboxClipped;
    BoxPtr	    pboxClippedBase;
    BoxPtr	    pextent;
    BoxRec	    stackRects[NUM_STACK_RECTS];
    cfbPrivGC	    *priv;
    int		    numRects;
    int		    n;
    int		    xorg, yorg;

    __dolog("GXPolyFillRect: nrect=%d, alu=0x%02x, planemask=0x%08x, fg=%d, bg=%d=n", nrectFill, pGC->alu, pGC->planemask, pGC->fgPixel, pGC->bgPixel);

    priv = (cfbPrivGC *) pGC->devPrivates[cfbGCPrivateIndex].ptr;
    prgnClip = cfbGetCompositeClip(pGC);
    prect = prectInit;
    xorg = pDrawable->x;
    yorg = pDrawable->y;
    if (xorg || yorg)
    {
	prect = prectInit;
	n = nrectFill;
	while(n--)
	{
	    prect->x += xorg;
	    prect->y += yorg;
	    prect++;
	}
    }

    prect = prectInit;

    numRects = REGION_NUM_RECTS(prgnClip) * nrectFill;
    if (numRects > NUM_STACK_RECTS)
    {
	pboxClippedBase = (BoxPtr)ALLOCATE_LOCAL(numRects * sizeof(BoxRec));
	if (!pboxClippedBase)
	    return;
    }
    else
	pboxClippedBase = stackRects;

    pboxClipped = pboxClippedBase;
	
    if (REGION_NUM_RECTS(prgnClip) == 1)
    {
	int x1, y1, x2, y2, bx2, by2;

	pextent = REGION_RECTS(prgnClip);
	x1 = pextent->x1;
	y1 = pextent->y1;
	x2 = pextent->x2;
	y2 = pextent->y2;
    	while (nrectFill--)
    	{
	    if ((pboxClipped->x1 = prect->x) < x1)
		pboxClipped->x1 = x1;
    
	    if ((pboxClipped->y1 = prect->y) < y1)
		pboxClipped->y1 = y1;
    
	    bx2 = (int) prect->x + (int) prect->width;
	    if (bx2 > x2)
		bx2 = x2;
	    pboxClipped->x2 = bx2;
    
	    by2 = (int) prect->y + (int) prect->height;
	    if (by2 > y2)
		by2 = y2;
	    pboxClipped->y2 = by2;

	    prect++;
	    if ((pboxClipped->x1 < pboxClipped->x2) &&
		(pboxClipped->y1 < pboxClipped->y2))
	    {
		pboxClipped++;
	    }
    	}
    }
    else
    {
	int x1, y1, x2, y2, bx2, by2;

	pextent = REGION_EXTENTS(pGC->pScreen, prgnClip);
	x1 = pextent->x1;
	y1 = pextent->y1;
	x2 = pextent->x2;
	y2 = pextent->y2;
    	while (nrectFill--)
    	{
	    BoxRec box;
    
	    if ((box.x1 = prect->x) < x1)
		box.x1 = x1;
    
	    if ((box.y1 = prect->y) < y1)
		box.y1 = y1;
    
	    bx2 = (int) prect->x + (int) prect->width;
	    if (bx2 > x2)
		bx2 = x2;
	    box.x2 = bx2;
    
	    by2 = (int) prect->y + (int) prect->height;
	    if (by2 > y2)
		by2 = y2;
	    box.y2 = by2;
    
	    prect++;
    
	    if ((box.x1 >= box.x2) || (box.y1 >= box.y2))
	    	continue;
    
	    n = REGION_NUM_RECTS (prgnClip);
	    pbox = REGION_RECTS(prgnClip);
    
	    /* clip the rectangle to each box in the clip region
	       this is logically equivalent to calling Intersect()
	    */
	    while(n--)
	    {
		pboxClipped->x1 = max(box.x1, pbox->x1);
		pboxClipped->y1 = max(box.y1, pbox->y1);
		pboxClipped->x2 = min(box.x2, pbox->x2);
		pboxClipped->y2 = min(box.y2, pbox->y2);
		pbox++;

		/* see if clipping left anything */
		if(pboxClipped->x1 < pboxClipped->x2 && 
		   pboxClipped->y1 < pboxClipped->y2)
		{
		    pboxClipped++;
		}
	    }
    	}
    }
    if (pboxClipped != pboxClippedBase)
	amigaGXFillRectAll(pDrawable, pGC,
		    pboxClipped-pboxClippedBase, pboxClippedBase);
    if (pboxClippedBase != stackRects)
    	DEALLOCATE_LOCAL(pboxClippedBase);
}

void
amigaGXFillSpans (pDrawable, pGC, n, ppt, pwidth, fSorted)
    DrawablePtr pDrawable;
    GCPtr	pGC;
    int		n;			/* number of spans to fill */
    DDXPointPtr ppt;			/* pointer to list of start points */
    int		*pwidth;		/* pointer to list of n widths */
    int 	fSorted;
{
    int		    x, y;
    int		    width;
				/* next three parameters are post-clip */
    int		    nTmp;
    int		    *pwidthFree;/* copies of the pointers to free */
    DDXPointPtr	    pptFree;
    cfbPrivGCPtr    devPriv = cfbGetGCPrivate(pGC);
    register int    r;
    BoxPtr	    extents;
    fbFd *inf = amigaInfo(pDrawable->pScreen);
    struct ACM *acm = (struct ACM *) (inf->regs + ACM_OFFSET);
    short bpp = inf->info.gd_planes >> 3;
    VUint *feed = (VUint *) inf->fb;
    Uint ltmp;
    Uint mask = (1<<bpp) - 1;

    __dolog("GXFillSpans: n=%d, alu=0x%02x, planemask=0x%08x, fg=%d, bg=%d=n", n, pGC->alu, pGC->planemask, pGC->fgPixel, pGC->bgPixel);

    /* can't deal with stipple-operations in here since we use the pattern
       channel to implement the planemasking feature. */
    
    if ((mask & pGC->planemask) == mask)
      {
	/* no masking, simple rop table */
	RZ3BlitInit(acm, blit_rop_table, pGC->alu);
      }
    else
      {
	/* use pattern as mask, and replicate the planemask as many 
	 * times as necessary. */
	RZ3BlitInit(acm, blit_rop_table_masked, pGC->alu);
	RZ3MaskInit(acm, inf->fb, pGC->planemask, bpp);
      }

    ltmp = pGC->fgPixel;
    M2I(ltmp);
    acm->fg = ltmp;
    ltmp = pGC->bgPixel;
    M2I(ltmp);
    acm->bg = ltmp;


    {
    	nTmp = n * miFindMaxBand(cfbGetCompositeClip(pGC));
    	pwidthFree = (int *)ALLOCATE_LOCAL(nTmp * sizeof(int));
    	pptFree = (DDXPointRec *)ALLOCATE_LOCAL(nTmp * sizeof(DDXPointRec));
    	if(!pptFree || !pwidthFree)
    	{
	    if (pptFree) DEALLOCATE_LOCAL(pptFree);
	    if (pwidthFree) DEALLOCATE_LOCAL(pwidthFree);
	    return;
    	}
    	n = miClipSpans(cfbGetCompositeClip(pGC),
		     	 ppt, pwidth, n,
		     	 pptFree, pwidthFree, fSorted);
    	pwidth = pwidthFree;
    	ppt = pptFree;
    }
    while (n--)
    {
	x = ppt->x;
	y = ppt->y;
	ppt++;
	width = *pwidth++;
	if (width)
	{
	  amigaGXFillSpan(acm,inf,y,x,x + width - 1,bpp);
	}
    }

    {
	DEALLOCATE_LOCAL(pptFree);
	DEALLOCATE_LOCAL(pwidthFree);
    }
}


#define FILLSPAN(acm,inf,y,x1,x2,bpp,x_off,y_off) {\
    if (x2 >= x1) {\
	amigaGXFillSpan(acm,inf,(y)+y_off,(x1)+x_off,(x2)+x_off,bpp); \
    } \
}

#define FILLSLICESPANS(flip,y) \
    if (!flip) \
    { \
	FILLSPAN(acm,inf,y,xl,xr,bpp,x_off,y_off) \
    } \
    else \
    { \
	xc = xorg - x; \
	FILLSPAN(acm,inf, y, xc, xr, bpp,x_off,y_off) \
	xc += slw - 1; \
	FILLSPAN(acm,inf, y, xl, xc, bpp,x_off,y_off) \
    }

#ifdef __STDC__
static void amigaGXFillEllipse (DrawablePtr, struct ACM *,
				fbFd *, int, xArc *, int, int);
#endif

static void
amigaGXFillEllipse (pDraw, acm, inf, bpp, arc, x_off, y_off)
    DrawablePtr	pDraw;
    struct ACM *acm;
    fbFd *inf;
    int bpp;
    xArc	*arc;
    int x_off, y_off;
{
    int x, y, e;
    int yk, xk, ym, xm, dx, dy, xorg, yorg;
    int	y_top, y_bot;
    miFillArcRec info;
    register int xpos;
    int	r;
    int	slw;

    miFillArcSetup(arc, &info);
    MIFILLARCSETUP();
    y_top = yorg - y;
    y_bot = yorg + y + dy;
    while (y)
    {
	y_top++;
	y_bot--;
	MIFILLARCSTEP(slw);
	if (!slw)
	    continue;
	xpos = xorg - x;
	amigaGXFillSpan (acm,inf,y_top+y_off,xpos+x_off,xpos+x_off+slw - 1,bpp);
	if (miFillArcLower(slw))
	    amigaGXFillSpan (acm,inf,y_bot+y_off,xpos+x_off,xpos+x_off+slw - 1,bpp);
    }
}

#ifdef __STDC__
static void amigaGXFillArcSlice (DrawablePtr, GCPtr, struct ACM *,
				 fbFd *, int, xArc *, int, int);
#endif

static void
amigaGXFillArcSlice (pDraw, pGC, acm, inf, bpp, arc, x_off, y_off)
    DrawablePtr pDraw;
    GCPtr	pGC;
    struct ACM *acm;
    fbFd *inf;
    int bpp;
    xArc	*arc;
    int x_off, y_off;
{
    int yk, xk, ym, xm, dx, dy, xorg, yorg, slw;
    register int x, y, e;
    miFillArcRec info;
    miArcSliceRec slice;
    int xl, xr, xc;
    int	y_top, y_bot;
    int	r;

    miFillArcSetup(arc, &info);
    miFillArcSliceSetup(arc, &slice, pGC);
    MIFILLARCSETUP();
    y_top = yorg - y;
    y_bot = yorg + y + dy;
    while (y > 0)
    {
	y_top++;
	y_bot--;
	MIFILLARCSTEP(slw);
	MIARCSLICESTEP(slice.edge1);
	MIARCSLICESTEP(slice.edge2);
	if (miFillSliceUpper(slice))
	{
	    MIARCSLICEUPPER(xl, xr, slice, slw);
	    FILLSLICESPANS(slice.flip_top, y_top);
	}
	if (miFillSliceLower(slice))
	{
	    MIARCSLICELOWER(xl, xr, slice, slw);
	    FILLSLICESPANS(slice.flip_bot, y_bot);
	}
    }
}

#define UNSET_CIRCLE


void
amigaGXPolyFillArc (pDraw, pGC, narcs, parcs)
    DrawablePtr	pDraw;
    GCPtr	pGC;
    int		narcs;
    xArc	*parcs;
{
    register xArc *arc;
    register int i;
    int		x, y;
    BoxRec box;
    BoxPtr	extents;
    RegionPtr cclip;
    cfbPrivGCPtr    devPriv;
    register int	r;
    fbFd *inf = amigaInfo(pDraw->pScreen);
    struct ACM *acm = (struct ACM *) (inf->regs + ACM_OFFSET);
    short bpp = inf->info.gd_planes >> 3;
    VUint *feed = (VUint *) inf->fb;
    Uint ltmp;
    Uint mask = (1<<bpp) - 1;
    int x_off, y_off;

    /* can't deal with stipple-operations in here since we use the pattern
       channel to implement the planemasking feature. */
    
    if ((mask & pGC->planemask) == mask)
      {
	/* no masking, simple rop table */
	RZ3BlitInit(acm, blit_rop_table, pGC->alu);
      }
    else
      {
	/* use pattern as mask, and replicate the planemask as many 
	 * times as necessary. */
	RZ3BlitInit(acm, blit_rop_table_masked, pGC->alu);
	RZ3MaskInit(acm, inf->fb, pGC->planemask, bpp);
      }

    ltmp = pGC->fgPixel;
    M2I(ltmp);
    acm->fg = ltmp;
    ltmp = pGC->bgPixel;
    M2I(ltmp);
    acm->bg = ltmp;

    devPriv = (cfbPrivGC *)(pGC->devPrivates[cfbGCPrivateIndex].ptr); 
    cclip = cfbGetCompositeClip(pGC);
    x_off = pDraw->x;
    y_off = pDraw->y;
    for (arc = parcs, i = narcs; --i >= 0; arc++)
    {
	if (miFillArcEmpty(arc))
	    continue;
	if (miCanFillArc(arc))
	{
	    x = arc->x;
	    y = arc->y;
	    {
	    	box.x1 = x + pDraw->x;
	    	box.y1 = y + pDraw->y;
	    	box.x2 = box.x1 + (int)arc->width + 1;
	    	box.y2 = box.y1 + (int)arc->height + 1;
	    }
	    if (RECT_IN_REGION(pDraw->pScreen, cclip, &box) == rgnIN)
	    {
		if ((arc->angle2 >= FULLCIRCLE) ||
		    (arc->angle2 <= -FULLCIRCLE))
		{
		    {
			amigaGXFillEllipse (pDraw, acm, inf, bpp, arc,
					    x_off, y_off);
		    }
		}
		else
		{
		    amigaGXFillArcSlice (pDraw, pGC, acm, inf, bpp, arc,
					 x_off, y_off);
		}
		continue;
	    }
	}
	miPolyFillArc(pDraw, pGC, 1, arc);
    }
}


static void
amigaGXPolyGlyphBlt (pDrawable, pGC, x, y, nglyph, ppci, pglyphBase)
    DrawablePtr	    pDrawable;
    GCPtr	    pGC;
    int		    x, y;
    unsigned int    nglyph;
    CharInfoPtr	    *ppci;		/* array of character info */
    pointer         pglyphBase;
{
    int		    h;
    int		    w;
    CharInfoPtr	    pci;
    unsigned long   *bits;
    register int    r;
    RegionPtr	    clip;
    BoxRec	    box;
    fbFd *inf = amigaInfo(pDrawable->pScreen);
    struct ACM *acm = (struct ACM *) (inf->regs + ACM_OFFSET);
    short bpp = inf->info.gd_planes >> 3;
    VUint *feed = (VUint *) inf->fb;
    Uint ltmp;
    int width, height;
    Uint mask = (1<<bpp) - 1;
    int x_off, y_off;

    __dolog("GXPolyGlyphBlt: nglyph=%d, x=%d,y=%d, alu=0x%02x, planemask=0x%08x, fg=%d, bg=%d=n", nglyph, x, y, pGC->alu, pGC->planemask, pGC->fgPixel, pGC->bgPixel);

    /* can't deal with stipple-operations in here since we use the pattern
       channel to implement the planemasking feature. */
    
    if ((mask & pGC->planemask) == mask)
      {
	/* no masking, simple rop table */
	RZ3BlitInit(acm, blit_rop_table, pGC->alu);
      }
    else
      {
	/* use pattern as mask, and replicate the planemask as many 
	 * times as necessary. */
	RZ3BlitInit(acm, blit_rop_table_masked, pGC->alu);
	RZ3MaskInit(acm, inf->fb, pGC->planemask, bpp);
      }

    ltmp = pGC->fgPixel;
    M2I(ltmp);
    acm->fg = ltmp;
    ltmp = pGC->bgPixel;
    M2I(ltmp);
    acm->bg = ltmp;

    clip = cfbGetCompositeClip(pGC);

    {
    	/* compute an approximate (but covering) bounding box */
    	box.x1 = 0;
    	if ((ppci[0]->metrics.leftSideBearing < 0))
	    box.x1 = ppci[0]->metrics.leftSideBearing;
    	h = nglyph - 1;
    	w = ppci[h]->metrics.rightSideBearing;
    	while (--h >= 0)
	    w += ppci[h]->metrics.characterWidth;
    	box.x2 = w;
    	box.y1 = -FONTMAXBOUNDS(pGC->font,ascent);
    	box.y2 = FONTMAXBOUNDS(pGC->font,descent);
    
    	box.x1 += pDrawable->x + x;
    	box.x2 += pDrawable->x + x;
    	box.y1 += pDrawable->y + y;
    	box.y2 += pDrawable->y + y;
    
    	switch (RECT_IN_REGION(pGC->pScreen, clip, &box))
	{
	case rgnPART:
	    if (bpp == 1)
	      cfb8PolyGlyphBlt8 (pDrawable, pGC, x, y, nglyph, ppci, 
				 pglyphBase);
	    else if (bpp == 2)
	      cfb16PolyGlyphBlt8 (pDrawable, pGC, x, y, nglyph, ppci, 
				  pglyphBase);
	    else
	      miPolyGlyphBlt (pDrawable, pGC, x, y, nglyph, ppci, 
			      pglyphBase);

	case rgnOUT:
	    return;
	}
    }


    x += pDrawable->x;
    y += pDrawable->y;

    while (nglyph--)
    {
	pci = *ppci++;
	width = pci->metrics.rightSideBearing - pci->metrics.leftSideBearing;
	height = pci->metrics.ascent + pci->metrics.descent;

	if (width && height && pci->bits)
	  {
	    RZ3Blit1toFB(acm, pGC->alu,
			 0,
			 (x + pci->metrics.leftSideBearing),
			 (y - pci->metrics.ascent),
			 width, height,
			 inf->info.gd_fbwidth, bpp, 1);
	    
	    h = height;
	    bits = (unsigned long *) pci->bits;
	    while (h--) {
	      *feed = *bits++;
	    }
	    RZ3WaitDone(acm);
	  }
	x += pci->metrics.characterWidth;
    }
}

static void
amigaGXTEGlyphBlt (pDrawable, pGC, x, y, nglyph, ppci, pglyphBase)
    DrawablePtr	pDrawable;
    GCPtr	pGC;
    int 	x, y;
    unsigned int nglyph;
    CharInfoPtr *ppci;		/* array of character info */
    pointer pglyphBase;		/* start of array of glyphs */
{
    int		    h, hTmp;
    int		    w;
    FontPtr	    pfont = pGC->font;
    register int    r;
    unsigned long   *char1, *char2, *char3, *char4;
    int		    widthGlyphs, widthGlyph;
    RegionPtr	    clip;
    BoxRec	    bbox;
    unsigned short  rop;
    fbFd *inf = amigaInfo(pDrawable->pScreen);
    struct ACM *acm = (struct ACM *) (inf->regs + ACM_OFFSET);
    short bpp = inf->info.gd_planes >> 3;
    VUint *feed = (VUint *) inf->fb;
    Uint ltmp;
    Uint mask = (1<<bpp) - 1;
    Uint dst, dstTmp, pat, dimen;

    __dolog("GXTEGlyphBlt: nglyph=%d, x=%d,y=%d, alu=0x%02x, planemask=0x%08x, fg=%d, bg=%d=n", nglyph, x, y, pGC->alu, pGC->planemask, pGC->fgPixel, pGC->bgPixel);

    widthGlyph = FONTMAXBOUNDS(pfont,characterWidth);
    h = FONTASCENT(pfont) + FONTDESCENT(pfont);

    clip = cfbGetCompositeClip(pGC);

    {
    	bbox.x1 = x + pDrawable->x;
    	bbox.x2 = bbox.x1 + (widthGlyph * nglyph);
    	bbox.y1 = y + pDrawable->y - FONTASCENT(pfont);
    	bbox.y2 = bbox.y1 + h;
    
    	switch (RECT_IN_REGION(pGC->pScreen, clip, &bbox))
    	{
	case rgnPART:
	    if (pglyphBase)
	        if (bpp == 1)
		  cfb8PolyGlyphBlt8(pDrawable, pGC, x, y, nglyph, ppci, NULL);
		else if (bpp == 2)
		  cfb16PolyGlyphBlt8(pDrawable, pGC, x, y, nglyph, ppci, NULL);
		else
		  miPolyGlyphBlt(pDrawable, pGC, x, y, nglyph, ppci, NULL);
	    else
		miImageGlyphBlt(pDrawable, pGC, x, y, nglyph, ppci, pglyphBase);
	case rgnOUT:
	    return;
    	}
    }

    rop = pglyphBase ? pGC->alu : GXcopy;

    if ((mask & pGC->planemask) == mask)
      {
	/* no masking, simple rop table */
	RZ3BlitInit(acm, blit_rop_table, rop);
      }
    else
      {
	/* use pattern as mask, and replicate the planemask as many 
	 * times as necessary. */
	RZ3BlitInit(acm, blit_rop_table_masked, rop);
	RZ3MaskInit(acm, inf->fb, pGC->planemask, bpp);
      }

    ltmp = pGC->fgPixel;
    M2I(ltmp);
    acm->fg = ltmp;
    ltmp = pGC->bgPixel;
    M2I(ltmp);
    acm->bg = ltmp;

    y = y + pDrawable->y - FONTASCENT(pfont);
    x += pDrawable->x;

    /* we start at (x, y), with font data which is (h) high and (widthGlyphs)
     * wide. */
    acm->start = 2;
    dst = 8 * bpp * (x + y * inf->info.gd_fbwidth);
    pat = 8 * PAT_MEM_OFF;
    M2I(pat);
    acm->pattern = pat;
    acm->control = 0xc06a; /* RIGHT,DOWN,SRC=M,DST=fb,ColExp,static-pattern */
    /* font data is enough aligned */
    acm->src = 0;

#define LoopIt(count, w, loadup, fetch) \
        dimen = (w) | (h << 16);	\
        M2I(dimen);			\
        acm->dimension = dimen;		\
    	while (nglyph >= count) 	\
    	{ 				\
            dstTmp = dst; M2I(dstTmp);	\
	    acm->dst = dstTmp;		\
            acm->start = 1;		\
	    nglyph -= count; 		\
	    loadup 			\
	    hTmp = h; 			\
	    while (hTmp--) 		\
	    	*feed = fetch; 		\
            dst += 8 * w * bpp;		\
	    RZ3WaitDone(acm);		\
    	}

    if (widthGlyph <= 8)
    {
	widthGlyphs = widthGlyph << 2;
	LoopIt(4, widthGlyphs,
	    char1 = (unsigned long *) (*ppci++)->bits;
	    char2 = (unsigned long *) (*ppci++)->bits;
	    char3 = (unsigned long *) (*ppci++)->bits;
	    char4 = (unsigned long *) (*ppci++)->bits;,
	    (*char1++ | ((*char2++ | ((*char3++ | (*char4++
		    >> widthGlyph))
		    >> widthGlyph))
		    >> widthGlyph)))
    }
    else if (widthGlyph <= 10)
    {
	widthGlyphs = (widthGlyph << 1) + widthGlyph;
	LoopIt(3, widthGlyphs,
	    char1 = (unsigned long *) (*ppci++)->bits;
	    char2 = (unsigned long *) (*ppci++)->bits;
	    char3 = (unsigned long *) (*ppci++)->bits;,
	    (*char1++ | ((*char2++ | (*char3++ >> widthGlyph)) >> widthGlyph)))
    }
    else if (widthGlyph <= 16)
    {
	widthGlyphs = widthGlyph << 1;
	LoopIt(2, widthGlyphs,
	    char1 = (unsigned long *) (*ppci++)->bits;
	    char2 = (unsigned long *) (*ppci++)->bits;,
	    (*char1++ | (*char2++ >> widthGlyph)))
    }
    dimen = widthGlyph | (h << 16);
    M2I(dimen);
    acm->dimension = dimen;
    while (nglyph--) {
        dstTmp = dst; M2I(dstTmp);
	acm->dst = dstTmp;
        acm->start = 1;
	char1 = (unsigned long *) (*ppci++)->bits;
	hTmp = h;
	while (hTmp--)
	    *feed = *char1++;
	dst += 8 * widthGlyph * bpp;
	RZ3WaitDone(acm);
    }
}

static void
amigaGXPolyTEGlyphBlt (pDrawable, pGC, x, y, nglyph, ppci, pglyphBase)
    DrawablePtr	pDrawable;
    GCPtr	pGC;
    int 	x, y;
    unsigned int nglyph;
    CharInfoPtr *ppci;		/* array of character info */
    pointer pglyphBase;		/* start of array of glyphs */
{
    amigaGXTEGlyphBlt (pDrawable, pGC, x, y, nglyph, ppci, (char *) 1);
}

static void
amigaGXFillBoxSolid (pDrawable, nBox, pBox, pixel)
    DrawablePtr	    pDrawable;
    int		    nBox;
    BoxPtr	    pBox;
    unsigned long   pixel;
{
    register int	r;
    fbFd *inf = amigaInfo(pDrawable->pScreen);
    struct ACM *acm = (struct ACM *) (inf->regs + ACM_OFFSET);
    short bpp = inf->info.gd_planes >> 3;
    VUint *feed = (VUint *) inf->fb;
    Uint ltmp;

    __dolog("GXFillBoxSolid: nBox=%d, (%d,%d) - (%d,%d)\n",
	    nBox, pBox->x1, pBox->y1, pBox->x2, pBox->y2);

    /* no masking, simple rop table */
    RZ3BlitInit(acm, blit_rop_table, GXcopy);

    ltmp = pixel;
    M2I(ltmp);
    acm->fg = ltmp;

    ltmp = *feed;
    while (nBox--) {
	RZ3Blit1toFB(acm, 0, 0,
		     pBox->x1, pBox->y1,
		     (pBox->x2 - pBox->x1),
		     (pBox->y2 - pBox->y1),
		     inf->info.gd_fbwidth, bpp, 0);
	while ((acm->status & 1) == 0) *feed = ~0;
	pBox++;
    }
    *feed = ltmp;
}

#if 0
void
amigaGXFillBoxStipple (pDrawable, nBox, pBox, stipple)
    DrawablePtr	    pDrawable;
    int		    nBox;
    BoxPtr	    pBox;
    amigaGXStipplePtr stipple;
{
    register amigaGXPtr	gx = amigaGXGetScreenPrivate (pDrawable->pScreen);
    register int	r;
    int			patx, paty;

    patx = 16 - (pDrawable->x & 0xf);
    paty = 16 - (pDrawable->y & 0xf);
    stipple->patalign = (patx <<  16) | paty;
    GXDrawInit(gx,0,gx_solid_rop_table[GXcopy]|POLY_N,~0);
    GXStippleInit(gx, stipple);
    while (nBox--) {
	gx->arecty = pBox->y1;
	gx->arectx = pBox->x1;
	gx->arecty = pBox->y2;
	gx->arectx = pBox->x2;
	pBox++;
	GXDrawDone(gx,r);
    }
    GXWait(gx,r);
}

amigaGXCheckTile (pPixmap, stipple)
    PixmapPtr	    pPixmap;
    amigaGXStipplePtr stipple;
{
    unsigned short  *sbits;
    unsigned int    fg = (unsigned int)~0, bg = (unsigned int)~0;
    unsigned char   *tilebitsLine, *tilebits, tilebit;
    unsigned short  sbit, mask;
    int		    nbwidth;
    int		    h, w;
    int		    x, y;
    int		    s_y, s_x;

    h = pPixmap->drawable.height;
    if (h > 16 || (h & (h - 1)))
	return FALSE;
    w = pPixmap->drawable.width;
    if (w > 16 || (w & (w - 1)))
	return FALSE;
    sbits = (unsigned short *) stipple->bits;
    tilebitsLine = (unsigned char *) pPixmap->devPrivate.ptr;
    nbwidth = pPixmap->devKind;
    for (y = 0; y < h; y++) {
	tilebits = tilebitsLine;
	tilebitsLine += nbwidth;
	sbit = 0;
	mask = 1 << 15;
	for (x = 0; x < w; x++)
	{
	    tilebit = *tilebits++;
	    if (tilebit == fg)
		sbit |=  mask;
	    else if (tilebit != bg)
	    {
		if (fg == ~0)
		{
		    fg = tilebit;
		    sbit |= mask;
		}
		else if (bg == ~0)
		{
		    bg = tilebit;
		}
		else
		{
		    return FALSE;
		}
	    }
	    mask >>= 1;
	}
	for (s_x = w; s_x < 16; s_x <<= 1)
	    sbit = sbit | (sbit >> s_x);
	for (s_y = y; s_y < 16; s_y += h)
	    sbits[s_y] = sbit;
    }
    stipple->fore = fg;
    stipple->back = bg;
    return TRUE;
}

amigaGXCheckStipple (pPixmap, stipple)
    PixmapPtr	    pPixmap;
    amigaGXStipplePtr stipple;
{
    unsigned short  *sbits;
    unsigned long   *stippleBits;
    unsigned long   sbit, mask;
    int		    h, w;
    int		    y;
    int		    s_y, s_x;

    h = pPixmap->drawable.height;
    if (h > 16 || (h & (h - 1)))
	return FALSE;
    w = pPixmap->drawable.width;
    if (w > 16 || (w & (w - 1)))
	return FALSE;
    sbits = (unsigned short *) stipple->bits;
    stippleBits = (unsigned long *) pPixmap->devPrivate.ptr;
    mask = ((1 << w) - 1) << (16 - w);
    for (y = 0; y < h; y++) {
	sbit = (*stippleBits++ >> 16) & mask;
	for (s_x = w; s_x < 16; s_x <<= 1)
	    sbit = sbit | (sbit >> s_x);
	for (s_y = y; s_y < 16; s_y += h)
	    sbits[s_y] = sbit;
    }
    return TRUE;
}

/* cache one stipple; figuring out if we can use the stipple is as hard as
 * computing it, so we just use this one and leave it here if it
 * can't be used this time
 */

static  amigaGXStipplePtr tmpStipple;

amigaGXCheckFill (pGC, pDrawable)
    GCPtr	pGC;
    DrawablePtr	pDrawable;
{
    amigaGXPrivGCPtr	    gxPriv = amigaGXGetGCPrivate (pGC);
    amigaGXStipplePtr	    stipple;
    Uint		    alu;
    int			    xrot, yrot;

    if (pGC->fillStyle == FillSolid)
    {
	if (gxPriv->stipple)
	{
	    xfree (gxPriv->stipple);
	    gxPriv->stipple = 0;
	}
	return TRUE;
    }
    if (!(stipple = gxPriv->stipple))
    {
	if (!tmpStipple)
	{
	    tmpStipple = (amigaGXStipplePtr) xalloc (sizeof *tmpStipple);
	    if (!tmpStipple)
		return FALSE;
	}
	stipple = tmpStipple;
    }
    alu =  gx_opaque_stipple_rop_table[pGC->alu]|GX_PATTERN_MASK;
    switch (pGC->fillStyle) {
    case FillTiled:
	if (!amigaGXCheckTile (pGC->tile.pixmap, stipple))
	{
	    if (gxPriv->stipple)
	    {
		xfree (gxPriv->stipple);
		gxPriv->stipple = 0;
	    }
	    return FALSE;
	}
	break;
    case FillStippled:
	alu = gx_stipple_rop_table[pGC->alu]|GX_PATTERN_MASK;
    case FillOpaqueStippled:
	if (!amigaGXCheckStipple (pGC->stipple, stipple))
	{
	    if (gxPriv->stipple)
	    {
	    	xfree (gxPriv->stipple);
	    	gxPriv->stipple = 0;
	    }
	    return FALSE;
	}
	stipple->fore = pGC->fgPixel;
	stipple->back = pGC->bgPixel;
	break;
    }
    xrot = (pGC->patOrg.x + pDrawable->x) & 0xf;
    yrot = (pGC->patOrg.y + pDrawable->y) & 0xf;
/*
    stipple->patalign = ((16 - (xrot & 0xf)) << 16) | (16 - (yrot & 0xf));
*/
    xrot = 16 - xrot;
    yrot = 16 - yrot;
    stipple->patalign = (xrot << 16) | yrot;
    stipple->alu = alu;
    gxPriv->stipple = stipple;
    if (stipple == tmpStipple)
	tmpStipple = 0;
    return TRUE;
}
#endif

void	amigaGXValidateGC ();
void	amigaGXDestroyGC ();
void	amiga8GXValidateGC (), amiga16GXValidateGC (), amiga24GXValidateGC ();

GCFuncs	amiga8GXGCFuncs = {
    amiga8GXValidateGC,
    miChangeGC,
    miCopyGC,
    amigaGXDestroyGC,
    miChangeClip,
    miDestroyClip,
    miCopyClip
};

GCFuncs	amiga16GXGCFuncs = {
    amiga16GXValidateGC,
    miChangeGC,
    miCopyGC,
    amigaGXDestroyGC,
    miChangeClip,
    miDestroyClip,
    miCopyClip
};

GCFuncs	amiga24GXGCFuncs = {
    amiga24GXValidateGC,
    miChangeGC,
    miCopyGC,
    amigaGXDestroyGC,
    miChangeClip,
    miDestroyClip,
    miCopyClip
};

extern void cfb8SetSpans(), cfb16SetSpans(), cfb32SetSpans();
extern void cfb8PutImage(), cfb16PutImage(), cfb32PutImage();
extern void cfb8PolyPoint(), cfb16PolyPoint(), cfb32PolyPoint();
extern void cfb8LineSS(), cfb16LineSS(), cfb32LineSS();
extern void cfb8SegmentSS(), cfb16SegmentSS(), cfb32SegmentSS();
extern void cfb8LineSD(), cfb16LineSD(), cfb32LineSD();
extern void cfb8SegmentSD(), cfb16SegmentSD(), cfb32SegmentSD();
extern void cfb8ZeroPolyArcSS8Copy(), cfb16ZeroPolyArcSSCopy();
extern void cfb8PushPixels8();
extern void cfb8ZeroPolyArcSS8Xor(), cfb16ZeroPolyArcSSXor();
extern void cfb8ZeroPolyArcSS8General(),cfb16ZeroPolyArcSSGeneral();
extern void cfb8Tile32FSCopy(),cfb16Tile32FSCopy(),cfb32Tile32FSCopy();
extern void cfb8Tile32FSGeneral(),cfb16Tile32FSGeneral(),cfb32Tile32FSGeneral();
extern void cfb8UnnaturalTileFS(), cfb16UnnaturalTileFS(), cfb32UnnaturalTileFS();
extern void cfb8Stipple32FS();
extern void cfb8UnnaturalStippleFS(),cfb16UnnaturalStippleFS(),cfb32UnnaturalStippleFS();
extern void cfb8OpaqueStipple32FS();
extern void cfb8UnnaturalStippleFS(),cfb16UnnaturalStippleFS(),cfb32UnnaturalStippleFS();
extern void cfb8PolyFillRect(),cfb16PolyFillRect();

void amiga24GXSetSpans(), amiga24GXGetSpans(), amiga24GXGetImage();
void amiga24GXResolveColor();
Bool amiga24GXInitializeColormap();


GCOps	amiga8GXTEOps = {
    amigaGXFillSpans,
    cfb8SetSpans,
    cfb8PutImage,
    amiga8GXCopyArea,
    amiga8GXCopyPlane,
    cfb8PolyPoint,
    cfb8LineSS,
    cfb8SegmentSS,
    miPolyRectangle,
    cfb8ZeroPolyArcSS8Copy,
    miFillPolygon,
    amigaGXPolyFillRect,
    amigaGXPolyFillArc,
    miPolyText8,
    miPolyText16,
    miImageText8,
    miImageText16,
    amigaGXTEGlyphBlt,
    amigaGXPolyTEGlyphBlt,
    cfb8PushPixels8
#ifdef NEED_LINEHELPER
    ,NULL
#endif
};


GCOps	amiga8GXNonTEOps = {
    amigaGXFillSpans,
    cfb8SetSpans,
    cfb8PutImage,
    amiga8GXCopyArea,
    amiga8GXCopyPlane,
    cfb8PolyPoint,
    cfb8LineSS,
    cfb8SegmentSS,
    miPolyRectangle,
    cfb8ZeroPolyArcSS8Copy,
    miFillPolygon,
    amigaGXPolyFillRect,
    amigaGXPolyFillArc,
    miPolyText8,
    miPolyText16,
    miImageText8,
    miImageText16,
    miImageGlyphBlt,
    amigaGXPolyGlyphBlt,
    cfb8PushPixels8
#ifdef NEED_LINEHELPER
    ,NULL
#endif
};

GCOps	amiga16GXTEOps = {
    amigaGXFillSpans,
    cfb16SetSpans,
    cfb16PutImage,
    amiga16GXCopyArea,
    amiga16GXCopyPlane,
    cfb16PolyPoint,
    cfb16LineSS,
    cfb16SegmentSS,
    miPolyRectangle,
    cfb16ZeroPolyArcSSCopy,
    miFillPolygon,
    amigaGXPolyFillRect,
    amigaGXPolyFillArc,
    miPolyText8,
    miPolyText16,
    miImageText8,
    miImageText16,
    amigaGXTEGlyphBlt,
    amigaGXPolyTEGlyphBlt,
    mfbPushPixels
#ifdef NEED_LINEHELPER
    ,NULL
#endif
};


GCOps	amiga16GXNonTEOps = {
    amigaGXFillSpans,
    cfb16SetSpans,
    cfb16PutImage,
    amiga16GXCopyArea,
    amiga16GXCopyPlane,
    cfb16PolyPoint,
    cfb16LineSS,
    cfb16SegmentSS,
    miPolyRectangle,
    cfb16ZeroPolyArcSSCopy,
    miFillPolygon,
    amigaGXPolyFillRect,
    amigaGXPolyFillArc,
    miPolyText8,
    miPolyText16,
    miImageText8,
    miImageText16,
    miImageGlyphBlt,
    amigaGXPolyGlyphBlt,
    mfbPushPixels
#ifdef NEED_LINEHELPER
    ,NULL
#endif
};

GCOps	amiga24GXTEOps = {
    amigaGXFillSpans,
    amiga24GXSetSpans,
    miPutImage,
    amiga24GXCopyArea,
    amiga24GXCopyPlane,
    miPolyPoint,
    miZeroLine,
    miPolySegment,
    miPolyRectangle,
    miZeroPolyArc,
    miFillPolygon,
    amigaGXPolyFillRect,
    amigaGXPolyFillArc,
    miPolyText8,
    miPolyText16,
    miImageText8,
    miImageText16,
    amigaGXTEGlyphBlt,
    amigaGXPolyTEGlyphBlt,
    mfbPushPixels
#ifdef NEED_LINEHELPER
    ,NULL
#endif
};


GCOps	amiga24GXNonTEOps = {
    amigaGXFillSpans,
    amiga24GXSetSpans,
    miPutImage,
    amiga24GXCopyArea,
    amiga24GXCopyPlane,
    miPolyPoint,
    miZeroLine,
    miPolySegment,
    miPolyRectangle,
    miZeroPolyArc,
    miFillPolygon,
    amigaGXPolyFillRect,
    amigaGXPolyFillArc,
    miPolyText8,
    miPolyText16,
    miImageText8,
    miImageText16,
    miImageGlyphBlt,
    amigaGXPolyGlyphBlt,
    mfbPushPixels
#ifdef NEED_LINEHELPER
    ,NULL
#endif
};

#define FONTWIDTH(font)	(FONTMAXBOUNDS(font,rightSideBearing) - \
			 FONTMINBOUNDS(font,leftSideBearing))

GCOps *
amigaGXMatchCommon (pGC, devPriv, bpp)
    GCPtr	    pGC;
    cfbPrivGCPtr    devPriv;
    int		    bpp;
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
        FONTWIDTH (pGC->font) <= 32 &&
	FONTMINBOUNDS(pGC->font,characterWidth) >= 0)
    {
        if (bpp == 8)
	  {
	    if (TERMINALFONT(pGC->font))
	      return &amiga8GXTEOps;
	    else
	      return &amiga8GXNonTEOps;
	  }
	else if (bpp == 16)
	  {
	    if (TERMINALFONT(pGC->font))
	      return &amiga16GXTEOps;
	    else
	      return &amiga16GXNonTEOps;
	  }
	else
	  {
	    if (TERMINALFONT(pGC->font))
	      return &amiga24GXTEOps;
	    else
	      return &amiga24GXNonTEOps;
	  }

    }
    return 0;
}

void
amiga8GXValidateGC(
    GCPtr	pGC,
    Mask	changes,
    DrawablePtr	pDrawable,
    int		bpp)
{
    int         mask;		/* stateChanges */
    int         index;		/* used for stepping through bitfields */
    int		new_rrop;
    int         new_line, new_text, new_fillspans, new_fillarea;
    int		new_rotate;
    int		xrot, yrot;
    /* flags for changing the proc vector */
    cfbPrivGCPtr devPriv;
    amigaGXPrivGCPtr  gxPriv;
    int		oneRect;
    int		canGX;

    gxPriv = amigaGXGetGCPrivate (pGC);
    if (pDrawable->type != DRAWABLE_WINDOW)
    {
	if (gxPriv->type == DRAWABLE_WINDOW)
	{
	    extern GCOps    cfb8NonTEOps;

	    miDestroyGCOps (pGC->ops);
	    pGC->ops = &cfb8NonTEOps;
	    changes = (1 << GCLastBit+1) - 1;
	    pGC->stateChanges = changes;
	    gxPriv->type = pDrawable->type;
	}
	cfb8ValidateGC (pGC, changes, pDrawable);
	return;
    }
    if (gxPriv->type != DRAWABLE_WINDOW)
    {
	changes = (1 << GCLastBit+1) - 1;
	gxPriv->type = DRAWABLE_WINDOW;
    }

    new_rotate = pGC->lastWinOrg.x != pDrawable->x ||
		 pGC->lastWinOrg.y != pDrawable->y;

    pGC->lastWinOrg.x = pDrawable->x;
    pGC->lastWinOrg.y = pDrawable->y;

    devPriv = ((cfbPrivGCPtr) (pGC->devPrivates[cfbGCPrivateIndex].ptr));

    new_rrop = FALSE;
    new_line = FALSE;
    new_text = FALSE;
    new_fillspans = FALSE;
    new_fillarea = FALSE;

    /*
     * if the client clip is different or moved OR the subwindowMode has
     * changed OR the window's clip has changed since the last validation
     * we need to recompute the composite clip 
     */

    if ((changes & (GCClipXOrigin|GCClipYOrigin|GCClipMask|GCSubwindowMode)) ||
	(pDrawable->serialNumber != (pGC->serialNumber & DRAWABLE_SERIAL_BITS))
	)
    {
	miComputeCompositeClip(pGC, pDrawable);
#if 0
	oneRect = REGION_NUM_RECTS(cfbGetCompositeClip(pGC)) == 1;
	if (oneRect != devPriv->oneRect)
	{
	    new_line = TRUE;
	    new_fillarea = TRUE;
	    devPriv->oneRect = oneRect;
	}
#endif
    }

    mask = changes;
    while (mask) {
	index = lowbit (mask);
	mask &= ~index;

	/*
	 * this switch acculmulates a list of which procedures might have
	 * to change due to changes in the GC.  in some cases (e.g.
	 * changing one 16 bit tile for another) we might not really need
	 * a change, but the code is being paranoid. this sort of batching
	 * wins if, for example, the alu and the font have been changed,
	 * or any other pair of items that both change the same thing. 
	 */
	switch (index) {
	case GCFunction:
	case GCForeground:
	    new_rrop = TRUE;
	    break;
	case GCPlaneMask:
	    new_rrop = TRUE;
	    new_text = TRUE;
	    break;
	case GCBackground:
	    break;
	case GCLineStyle:
	case GCLineWidth:
	    new_line = TRUE;
	    break;
	case GCCapStyle:
	    break;
	case GCJoinStyle:
	    break;
	case GCFillStyle:
	    new_text = TRUE;
	    new_fillspans = TRUE;
	    new_line = TRUE;
	    new_fillarea = TRUE;
	    break;
	case GCFillRule:
	    break;
	case GCTile:
	    new_fillspans = TRUE;
	    new_fillarea = TRUE;
	    break;

	case GCStipple:
	    new_fillspans = TRUE;
	    new_fillarea = TRUE;
	    break;

	case GCTileStipXOrigin:
	    new_rotate = TRUE;
	    break;

	case GCTileStipYOrigin:
	    new_rotate = TRUE;
	    break;

	case GCFont:
	    new_text = TRUE;
	    break;
	case GCSubwindowMode:
	    break;
	case GCGraphicsExposures:
	    break;
	case GCClipXOrigin:
	    break;
	case GCClipYOrigin:
	    break;
	case GCClipMask:
	    break;
	case GCDashOffset:
	    break;
	case GCDashList:
	    break;
	case GCArcMode:
	    break;
	default:
	    break;
	}
    }

    /*
     * If the drawable has changed,  check its depth & ensure suitable
     * entries are in the proc vector. 
     */
    if (pDrawable->serialNumber != (pGC->serialNumber & (DRAWABLE_SERIAL_BITS))) {
	new_fillspans = TRUE;	/* deal with FillSpans later */
    }

    if ((new_rotate || new_fillspans))
    {
	Bool new_pix = FALSE;
	xrot = pGC->patOrg.x + pDrawable->x;
	yrot = pGC->patOrg.y + pDrawable->y;

#if 0
	if (!amigaGXCheckFill (pGC, pDrawable))
#endif

	{
	    switch (pGC->fillStyle)
	    {
	    case FillTiled:
	    	if (!pGC->tileIsPixel)
	    	{
		    int width = pGC->tile.pixmap->drawable.width * PSZ;
    
		    if ((width <= 32) && !(width & (width - 1)))
		    {
		    	cfb8CopyRotatePixmap(pGC->tile.pixmap,
					    &pGC->pRotatedPixmap,
					    xrot, yrot);
		    	new_pix = TRUE;
		    }
	    	}
	    	break;
	    case FillStippled:
	    case FillOpaqueStippled:
	    	{
		    int width = pGC->stipple->drawable.width;
    
		    if ((width <= 32) && !(width & (width - 1)))
		    {
		    	mfbCopyRotatePixmap(pGC->stipple,
					    &pGC->pRotatedPixmap, xrot, yrot);
		    	new_pix = TRUE;
		    }
	    	}
	    	break;
	    }
	}
	if (!new_pix && pGC->pRotatedPixmap)
	{
	    cfb8DestroyPixmap(pGC->pRotatedPixmap);
	    pGC->pRotatedPixmap = (PixmapPtr) NULL;
	}
    }

    if (new_rrop)
    {
	int old_rrop;

#if 0
	if (gxPriv->stipple)
	{
	    if (pGC->fillStyle == FillStippled)
		gxPriv->stipple->alu = gx_stipple_rop_table[pGC->alu]|GX_PATTERN_MASK;
	    else
		gxPriv->stipple->alu = gx_opaque_stipple_rop_table[pGC->alu]|GX_PATTERN_MASK;
	    if (pGC->fillStyle != FillTiled)
	    {
		gxPriv->stipple->fore = pGC->fgPixel;
		gxPriv->stipple->back = pGC->bgPixel;
	    }
	}
#endif
	old_rrop = devPriv->rop;
	devPriv->rop = cfb8ReduceRasterOp (pGC->alu, pGC->fgPixel,
					   pGC->planemask,
					   &devPriv->and, &devPriv->xor);
	if (old_rrop == devPriv->rop)
	    new_rrop = FALSE;
	else
	{
	    new_line = TRUE;
	    new_text = TRUE;
	    new_fillspans = TRUE;
	    new_fillarea = TRUE;
	}
    }

    if (new_rrop || new_fillspans || new_text || new_fillarea || new_line)
    {
	GCOps	*newops;

	if (newops = amigaGXMatchCommon (pGC, devPriv, 8))
 	{
	    if (pGC->ops->devPrivate.val)
		miDestroyGCOps (pGC->ops);
	    pGC->ops = newops;
	    new_rrop = new_line = new_fillspans = new_text = new_fillarea = 0;
	}
 	else
 	{
	    if (!pGC->ops->devPrivate.val)
	    {
		pGC->ops = miCreateGCOps (pGC->ops);
		pGC->ops->devPrivate.val = 1;
	    }
	}
    }

    canGX = pGC->fillStyle == FillSolid /*|| gxPriv->stipple*/;

    /* deal with the changes we've collected */
    if (new_line)
    {
	pGC->ops->FillPolygon = miFillPolygon;
	if (pGC->lineWidth == 0)
	{
	    if ((pGC->lineStyle == LineSolid) && (pGC->fillStyle == FillSolid))
	    {
		switch (devPriv->rop)
		{
		case GXxor:
		    pGC->ops->PolyArc = cfb8ZeroPolyArcSS8Xor;
		    break;
		case GXcopy:
		    pGC->ops->PolyArc = cfb8ZeroPolyArcSS8Copy;
		    break;
		default:
		    pGC->ops->PolyArc = cfb8ZeroPolyArcSS8General;
		    break;
		}
	    }
	    else
		pGC->ops->PolyArc = miZeroPolyArc;
	}
	else
	    pGC->ops->PolyArc = miPolyArc;
	pGC->ops->PolySegment = miPolySegment;
	switch (pGC->lineStyle)
	{
	case LineSolid:
	    if(pGC->lineWidth == 0)
	    {
	        if (pGC->fillStyle == FillSolid)
		{
		  pGC->ops->Polylines = cfb8LineSS;
		  pGC->ops->PolySegment = cfb8SegmentSS;
		}
		else
		    pGC->ops->Polylines = miZeroLine;
	    }
	    else
		pGC->ops->Polylines = miWideLine;
	    break;
	case LineOnOffDash:
	case LineDoubleDash:
	    if (pGC->lineWidth == 0 && pGC->fillStyle == FillSolid)
	    {
		pGC->ops->Polylines = cfb8LineSD;
		pGC->ops->PolySegment = cfb8SegmentSD;
	    } else
		pGC->ops->Polylines = miWideDash;
	    break;
	}
    }

    if (new_text && (pGC->font))
    {
        if (FONTWIDTH(pGC->font) > 32 ||
	    FONTMINBOUNDS(pGC->font,characterWidth) < 0)
        {
            pGC->ops->PolyGlyphBlt = miPolyGlyphBlt;
            pGC->ops->ImageGlyphBlt = miImageGlyphBlt;
        }
        else
        {
	    if (pGC->fillStyle == FillSolid) 
	    {
		if (TERMINALFONT (pGC->font))
		    pGC->ops->PolyGlyphBlt = amigaGXPolyTEGlyphBlt;
		else
		    pGC->ops->PolyGlyphBlt = amigaGXPolyGlyphBlt;
	    }
	    else
		pGC->ops->PolyGlyphBlt = miPolyGlyphBlt;
            /* special case ImageGlyphBlt for terminal emulator fonts */
            if (TERMINALFONT(pGC->font))
		pGC->ops->ImageGlyphBlt = amigaGXTEGlyphBlt;
            else
                pGC->ops->ImageGlyphBlt = miImageGlyphBlt;
        }
    }    


    if (new_fillspans) {
	if (canGX)
	    pGC->ops->FillSpans = amigaGXFillSpans;
	else switch (pGC->fillStyle) {
	case FillTiled:
	    if (pGC->pRotatedPixmap)
	    {
		if (pGC->alu == GXcopy && (pGC->planemask & PMSK) == PMSK)
		    pGC->ops->FillSpans = cfb8Tile32FSCopy;
		else
		    pGC->ops->FillSpans = cfb8Tile32FSGeneral;
	    }
	    else
		pGC->ops->FillSpans = cfb8UnnaturalTileFS;
	    break;
	case FillStippled:
	    if (pGC->pRotatedPixmap)
		pGC->ops->FillSpans = cfb8Stipple32FS;
	    else
		pGC->ops->FillSpans = cfb8UnnaturalStippleFS;
	    break;
	case FillOpaqueStippled:
	    if (pGC->pRotatedPixmap)
		pGC->ops->FillSpans = cfb8OpaqueStipple32FS;
	    else
		pGC->ops->FillSpans = cfb8UnnaturalStippleFS;
	    break;
	default:
	    FatalError("cfb8ValidateGC: illegal fillStyle\n");
	}
    } /* end of new_fillspans */

    if (new_fillarea) {
	pGC->ops->PolyFillRect = cfb8PolyFillRect;
	pGC->ops->PolyFillArc = miPolyFillArc;
	if (canGX)
	{
	    pGC->ops->PolyFillArc = amigaGXPolyFillArc;
	    pGC->ops->PolyFillRect = amigaGXPolyFillRect;
	}
	pGC->ops->PushPixels = mfbPushPixels;
	if (pGC->fillStyle == FillSolid && devPriv->rop == GXcopy)
	    pGC->ops->PushPixels = cfb8PushPixels8;
    }
}

void
amiga16GXValidateGC(
    GCPtr	pGC,
    Mask	changes,
    DrawablePtr	pDrawable,
    int		bpp)
{
    int         mask;		/* stateChanges */
    int         index;		/* used for stepping through bitfields */
    int		new_rrop;
    int         new_line, new_text, new_fillspans, new_fillarea;
    int		new_rotate;
    int		xrot, yrot;
    /* flags for changing the proc vector */
    cfbPrivGCPtr devPriv;
    amigaGXPrivGCPtr  gxPriv;
    int		oneRect;
    int		canGX;

    gxPriv = amigaGXGetGCPrivate (pGC);
    if (pDrawable->type != DRAWABLE_WINDOW)
    {
	if (gxPriv->type == DRAWABLE_WINDOW)
	{
	    extern GCOps    cfb16NonTEOps;

	    miDestroyGCOps (pGC->ops);
	    pGC->ops = &cfb16NonTEOps;
	    changes = (1 << GCLastBit+1) - 1;
	    pGC->stateChanges = changes;
	    gxPriv->type = pDrawable->type;
	}
	cfb16ValidateGC (pGC, changes, pDrawable);
	return;
    }
    if (gxPriv->type != DRAWABLE_WINDOW)
    {
	changes = (1 << GCLastBit+1) - 1;
	gxPriv->type = DRAWABLE_WINDOW;
    }

    new_rotate = pGC->lastWinOrg.x != pDrawable->x ||
		 pGC->lastWinOrg.y != pDrawable->y;

    pGC->lastWinOrg.x = pDrawable->x;
    pGC->lastWinOrg.y = pDrawable->y;

    devPriv = ((cfbPrivGCPtr) (pGC->devPrivates[cfbGCPrivateIndex].ptr));

    new_rrop = FALSE;
    new_line = FALSE;
    new_text = FALSE;
    new_fillspans = FALSE;
    new_fillarea = FALSE;

    /*
     * if the client clip is different or moved OR the subwindowMode has
     * changed OR the window's clip has changed since the last validation
     * we need to recompute the composite clip 
     */

    if ((changes & (GCClipXOrigin|GCClipYOrigin|GCClipMask|GCSubwindowMode)) ||
	(pDrawable->serialNumber != (pGC->serialNumber & DRAWABLE_SERIAL_BITS))
	)
    {
	miComputeCompositeClip(pGC, pDrawable);
#if 0
	oneRect = REGION_NUM_RECTS(cfbGetCompositeClip(pGC)) == 1;
	if (oneRect != devPriv->oneRect)
	{
	    new_line = TRUE;
	    new_fillarea = TRUE;
	    devPriv->oneRect = oneRect;
	}
#endif
    }

    mask = changes;
    while (mask) {
	index = lowbit (mask);
	mask &= ~index;

	/*
	 * this switch acculmulates a list of which procedures might have
	 * to change due to changes in the GC.  in some cases (e.g.
	 * changing one 16 bit tile for another) we might not really need
	 * a change, but the code is being paranoid. this sort of batching
	 * wins if, for example, the alu and the font have been changed,
	 * or any other pair of items that both change the same thing. 
	 */
	switch (index) {
	case GCFunction:
	case GCForeground:
	    new_rrop = TRUE;
	    break;
	case GCPlaneMask:
	    new_rrop = TRUE;
	    new_text = TRUE;
	    break;
	case GCBackground:
	    break;
	case GCLineStyle:
	case GCLineWidth:
	    new_line = TRUE;
	    break;
	case GCCapStyle:
	    break;
	case GCJoinStyle:
	    break;
	case GCFillStyle:
	    new_text = TRUE;
	    new_fillspans = TRUE;
	    new_line = TRUE;
	    new_fillarea = TRUE;
	    break;
	case GCFillRule:
	    break;
	case GCTile:
	    new_fillspans = TRUE;
	    new_fillarea = TRUE;
	    break;

	case GCStipple:
	    new_fillspans = TRUE;
	    new_fillarea = TRUE;
	    break;

	case GCTileStipXOrigin:
	    new_rotate = TRUE;
	    break;

	case GCTileStipYOrigin:
	    new_rotate = TRUE;
	    break;

	case GCFont:
	    new_text = TRUE;
	    break;
	case GCSubwindowMode:
	    break;
	case GCGraphicsExposures:
	    break;
	case GCClipXOrigin:
	    break;
	case GCClipYOrigin:
	    break;
	case GCClipMask:
	    break;
	case GCDashOffset:
	    break;
	case GCDashList:
	    break;
	case GCArcMode:
	    break;
	default:
	    break;
	}
    }

    /*
     * If the drawable has changed,  check its depth & ensure suitable
     * entries are in the proc vector. 
     */
    if (pDrawable->serialNumber != (pGC->serialNumber & (DRAWABLE_SERIAL_BITS))) {
	new_fillspans = TRUE;	/* deal with FillSpans later */
    }

    if ((new_rotate || new_fillspans))
    {
	Bool new_pix = FALSE;
	xrot = pGC->patOrg.x + pDrawable->x;
	yrot = pGC->patOrg.y + pDrawable->y;

#if 0
	if (!amigaGXCheckFill (pGC, pDrawable))
#endif

	{
	    switch (pGC->fillStyle)
	    {
	    case FillTiled:
	    	if (!pGC->tileIsPixel)
	    	{
		    int width = pGC->tile.pixmap->drawable.width * PSZ;
    
		    if ((width <= 32) && !(width & (width - 1)))
		    {
		    	cfb16CopyRotatePixmap(pGC->tile.pixmap,
					      &pGC->pRotatedPixmap,
					      /* &cfbGetCompositeClip(pGC), */
					      xrot, yrot);
		    	new_pix = TRUE;
		    }
	    	}
	    	break;
	    case FillStippled:
	    case FillOpaqueStippled:
	    	{
		    int width = pGC->stipple->drawable.width;
    
		    if ((width <= 32) && !(width & (width - 1)))
		    {
		    	mfbCopyRotatePixmap(pGC->stipple,
					    &pGC->pRotatedPixmap, xrot, yrot);
		    	new_pix = TRUE;
		    }
	    	}
	    	break;
	    }
	}
	if (!new_pix && pGC->pRotatedPixmap)
	{
	    cfb16DestroyPixmap(pGC->pRotatedPixmap);
	    pGC->pRotatedPixmap = (PixmapPtr) NULL;
	}
    }

    if (new_rrop)
    {
	int old_rrop;

#if 0
	if (gxPriv->stipple)
	{
	    if (pGC->fillStyle == FillStippled)
		gxPriv->stipple->alu = gx_stipple_rop_table[pGC->alu]|GX_PATTERN_MASK;
	    else
		gxPriv->stipple->alu = gx_opaque_stipple_rop_table[pGC->alu]|GX_PATTERN_MASK;
	    if (pGC->fillStyle != FillTiled)
	    {
		gxPriv->stipple->fore = pGC->fgPixel;
		gxPriv->stipple->back = pGC->bgPixel;
	    }
	}
#endif
	old_rrop = devPriv->rop;
	devPriv->rop = cfb16ReduceRasterOp (pGC->alu, pGC->fgPixel,
					   pGC->planemask,
					   &devPriv->and, &devPriv->xor);
	if (old_rrop == devPriv->rop)
	    new_rrop = FALSE;
	else
	{
	    new_line = TRUE;
	    new_text = TRUE;
	    new_fillspans = TRUE;
	    new_fillarea = TRUE;
	}
    }

    if (new_rrop || new_fillspans || new_text || new_fillarea || new_line)
    {
	GCOps	*newops;

	if (newops = amigaGXMatchCommon (pGC, devPriv, 16))
 	{
	    if (pGC->ops->devPrivate.val)
		miDestroyGCOps (pGC->ops);
	    pGC->ops = newops;
	    new_rrop = new_line = new_fillspans = new_text = new_fillarea = 0;
	}
 	else
 	{
	    if (!pGC->ops->devPrivate.val)
	    {
		pGC->ops = miCreateGCOps (pGC->ops);
		pGC->ops->devPrivate.val = 1;
	    }
	}
    }

    canGX = pGC->fillStyle == FillSolid /*|| gxPriv->stipple*/;

    /* deal with the changes we've collected */
    if (new_line)
    {
	pGC->ops->FillPolygon = miFillPolygon;
	if (pGC->lineWidth == 0)
	{
	    if ((pGC->lineStyle == LineSolid) && (pGC->fillStyle == FillSolid))
	    {
		switch (devPriv->rop)
		{
		case GXxor:
		    pGC->ops->PolyArc = cfb16ZeroPolyArcSSXor;
		    break;
		case GXcopy:
		    pGC->ops->PolyArc = cfb16ZeroPolyArcSSCopy;
		    break;
		default:
		    pGC->ops->PolyArc = cfb16ZeroPolyArcSSGeneral;
		    break;
		}
	    }
	    else
		pGC->ops->PolyArc = miZeroPolyArc;
	}
	else
	    pGC->ops->PolyArc = miPolyArc;
	pGC->ops->PolySegment = miPolySegment;
	switch (pGC->lineStyle)
	{
	case LineSolid:
	    if(pGC->lineWidth == 0)
	    {
	        if (pGC->fillStyle == FillSolid)
		{
		  pGC->ops->Polylines = cfb16LineSS;
		  pGC->ops->PolySegment = cfb16SegmentSS;
		}
		else
		    pGC->ops->Polylines = miZeroLine;
	    }
	    else
		pGC->ops->Polylines = miWideLine;
	    break;
	case LineOnOffDash:
	case LineDoubleDash:
	    if (pGC->lineWidth == 0 && pGC->fillStyle == FillSolid)
	    {
		pGC->ops->Polylines = cfb16LineSD;
		pGC->ops->PolySegment = cfb16SegmentSD;
	    } else
		pGC->ops->Polylines = miWideDash;
	    break;
	}
    }

    if (new_text && (pGC->font))
    {
        if (FONTWIDTH(pGC->font) > 32 ||
	    FONTMINBOUNDS(pGC->font,characterWidth) < 0)
        {
            pGC->ops->PolyGlyphBlt = miPolyGlyphBlt;
            pGC->ops->ImageGlyphBlt = miImageGlyphBlt;
        }
        else
        {
	    if (pGC->fillStyle == FillSolid) 
	    {
		if (TERMINALFONT (pGC->font))
		    pGC->ops->PolyGlyphBlt = amigaGXPolyTEGlyphBlt;
		else
		    pGC->ops->PolyGlyphBlt = amigaGXPolyGlyphBlt;
	    }
	    else
		pGC->ops->PolyGlyphBlt = miPolyGlyphBlt;
            /* special case ImageGlyphBlt for terminal emulator fonts */
            if (TERMINALFONT(pGC->font))
		pGC->ops->ImageGlyphBlt = amigaGXTEGlyphBlt;
            else
                pGC->ops->ImageGlyphBlt = miImageGlyphBlt;
        }
    }    


    if (new_fillspans) {
	if (canGX)
	    pGC->ops->FillSpans = amigaGXFillSpans;
	else switch (pGC->fillStyle) {
	case FillTiled:
	    if (pGC->pRotatedPixmap)
	    {
		if (pGC->alu == GXcopy && (pGC->planemask & PMSK) == PMSK)
		    pGC->ops->FillSpans = cfb16Tile32FSCopy;
		else
		    pGC->ops->FillSpans = cfb16Tile32FSGeneral;
	    }
	    else
		pGC->ops->FillSpans = cfb16UnnaturalTileFS;
	    break;
	case FillStippled:
	    pGC->ops->FillSpans = cfb16UnnaturalStippleFS;
	    break;
	case FillOpaqueStippled:
	    pGC->ops->FillSpans = cfb16UnnaturalStippleFS;
	    break;
	default:
	    FatalError("cfb16ValidateGC: illegal fillStyle\n");
	}
    } /* end of new_fillspans */

    if (new_fillarea) {
	pGC->ops->PolyFillRect = cfb16PolyFillRect;
	pGC->ops->PolyFillArc = miPolyFillArc;
	if (canGX)
	{
	    pGC->ops->PolyFillArc = amigaGXPolyFillArc;
	    pGC->ops->PolyFillRect = amigaGXPolyFillRect;
	}
	pGC->ops->PushPixels = mfbPushPixels;
	if (pGC->fillStyle == FillSolid && devPriv->rop == GXcopy)
	    pGC->ops->PushPixels = mfbPushPixels;
    }
}

void
amiga24GXValidateGC(
    GCPtr	pGC,
    Mask	changes,
    DrawablePtr	pDrawable,
    int		bpp)
{
    int         mask;		/* stateChanges */
    int         index;		/* used for stepping through bitfields */
    int		new_rrop;
    int         new_line, new_text, new_fillspans, new_fillarea;
    int		new_rotate;
    int		xrot, yrot;
    /* flags for changing the proc vector */
    cfbPrivGCPtr devPriv;
    amigaGXPrivGCPtr  gxPriv;
    int		oneRect;
    int		canGX;

    gxPriv = amigaGXGetGCPrivate (pGC);
    if (pDrawable->type != DRAWABLE_WINDOW)
    {
	if (gxPriv->type == DRAWABLE_WINDOW)
	{
	    extern GCOps    cfb32NonTEOps;

	    miDestroyGCOps (pGC->ops);
	    pGC->ops = &cfb32NonTEOps;
	    changes = (1 << GCLastBit+1) - 1;
	    pGC->stateChanges = changes;
	    gxPriv->type = pDrawable->type;
	}
	cfb32ValidateGC (pGC, changes, pDrawable);
	return;
    }

    /* starting here, we're ONLY dealing with FRAMEBUFFER manipulation,
     * so *no* cfb32 calls on the drawable are permitted!
     ******************************************************************/
    if (gxPriv->type != DRAWABLE_WINDOW)
    {
	changes = (1 << GCLastBit+1) - 1;
	gxPriv->type = DRAWABLE_WINDOW;
    }

    new_rotate = pGC->lastWinOrg.x != pDrawable->x ||
		 pGC->lastWinOrg.y != pDrawable->y;

    pGC->lastWinOrg.x = pDrawable->x;
    pGC->lastWinOrg.y = pDrawable->y;

    devPriv = ((cfbPrivGCPtr) (pGC->devPrivates[cfbGCPrivateIndex].ptr));

    new_rrop = FALSE;
    new_line = FALSE;
    new_text = FALSE;
    new_fillspans = FALSE;
    new_fillarea = FALSE;

    /*
     * if the client clip is different or moved OR the subwindowMode has
     * changed OR the window's clip has changed since the last validation
     * we need to recompute the composite clip 
     */

    if ((changes & (GCClipXOrigin|GCClipYOrigin|GCClipMask|GCSubwindowMode)) ||
	(pDrawable->serialNumber != (pGC->serialNumber & DRAWABLE_SERIAL_BITS))
	)
    {
	miComputeCompositeClip(pGC, pDrawable);
#if 0
	oneRect = REGION_NUM_RECTS(cfbGetCompositeClip(pGC)) == 1;
	if (oneRect != devPriv->oneRect)
	{
	    new_line = TRUE;
	    new_fillarea = TRUE;
	    devPriv->oneRect = oneRect;
	}
#endif
    }

    mask = changes;
    while (mask) {
	index = lowbit (mask);
	mask &= ~index;

	/*
	 * this switch acculmulates a list of which procedures might have
	 * to change due to changes in the GC.  in some cases (e.g.
	 * changing one 16 bit tile for another) we might not really need
	 * a change, but the code is being paranoid. this sort of batching
	 * wins if, for example, the alu and the font have been changed,
	 * or any other pair of items that both change the same thing. 
	 */
	switch (index) {
	case GCFunction:
	case GCForeground:
	    new_rrop = TRUE;
	    break;
	case GCPlaneMask:
	    new_rrop = TRUE;
	    new_text = TRUE;
	    break;
	case GCBackground:
	    break;
	case GCLineStyle:
	case GCLineWidth:
	    new_line = TRUE;
	    break;
	case GCCapStyle:
	    break;
	case GCJoinStyle:
	    break;
	case GCFillStyle:
	    new_text = TRUE;
	    new_fillspans = TRUE;
	    new_line = TRUE;
	    new_fillarea = TRUE;
	    break;
	case GCFillRule:
	    break;
	case GCTile:
	    new_fillspans = TRUE;
	    new_fillarea = TRUE;
	    break;

	case GCStipple:
	    new_fillspans = TRUE;
	    new_fillarea = TRUE;
	    break;

	case GCTileStipXOrigin:
	    new_rotate = TRUE;
	    break;

	case GCTileStipYOrigin:
	    new_rotate = TRUE;
	    break;

	case GCFont:
	    new_text = TRUE;
	    break;
	case GCSubwindowMode:
	    break;
	case GCGraphicsExposures:
	    break;
	case GCClipXOrigin:
	    break;
	case GCClipYOrigin:
	    break;
	case GCClipMask:
	    break;
	case GCDashOffset:
	    break;
	case GCDashList:
	    break;
	case GCArcMode:
	    break;
	default:
	    break;
	}
    }

    /*
     * If the drawable has changed,  check its depth & ensure suitable
     * entries are in the proc vector. 
     */
    if (pDrawable->serialNumber != (pGC->serialNumber & (DRAWABLE_SERIAL_BITS))) {
	new_fillspans = TRUE;	/* deal with FillSpans later */
    }

    if ((new_rotate || new_fillspans))
    {
	Bool new_pix = FALSE;
	xrot = pGC->patOrg.x + pDrawable->x;
	yrot = pGC->patOrg.y + pDrawable->y;

#if 0
	if (!amigaGXCheckFill (pGC, pDrawable))
#endif

	{
	    switch (pGC->fillStyle)
	    {
	    case FillTiled:
	    	if (!pGC->tileIsPixel)
	    	{
		    int width = pGC->tile.pixmap->drawable.width * PSZ;
    
		    if ((width <= 32) && !(width & (width - 1)))
		    {
		        /* cfb32 on tile, not drawable */
		    	cfb32CopyRotatePixmap(pGC->tile.pixmap,
					    &pGC->pRotatedPixmap,
					    xrot, yrot);
		    	new_pix = TRUE;
		    }
	    	}
	    	break;
	    case FillStippled:
	    case FillOpaqueStippled:
	    	{
		    int width = pGC->stipple->drawable.width;
    
		    if ((width <= 32) && !(width & (width - 1)))
		    {
		    	mfbCopyRotatePixmap(pGC->stipple,
					    &pGC->pRotatedPixmap, xrot, yrot);
		    	new_pix = TRUE;
		    }
	    	}
	    	break;
	    }
	}
	if (!new_pix && pGC->pRotatedPixmap)
	{
	    cfb32DestroyPixmap(pGC->pRotatedPixmap);
	    pGC->pRotatedPixmap = (PixmapPtr) NULL;
	}
    }

    if (new_rrop)
    {
	int old_rrop;

#if 0
	if (gxPriv->stipple)
	{
	    if (pGC->fillStyle == FillStippled)
		gxPriv->stipple->alu = gx_stipple_rop_table[pGC->alu]|GX_PATTERN_MASK;
	    else
		gxPriv->stipple->alu = gx_opaque_stipple_rop_table[pGC->alu]|GX_PATTERN_MASK;
	    if (pGC->fillStyle != FillTiled)
	    {
		gxPriv->stipple->fore = pGC->fgPixel;
		gxPriv->stipple->back = pGC->bgPixel;
	    }
	}
#endif
	old_rrop = devPriv->rop;
	devPriv->rop = cfb32ReduceRasterOp (pGC->alu, pGC->fgPixel,
					   pGC->planemask,
					   &devPriv->and, &devPriv->xor);
	if (old_rrop == devPriv->rop)
	    new_rrop = FALSE;
	else
	{
	    new_line = TRUE;
	    new_text = TRUE;
	    new_fillspans = TRUE;
	    new_fillarea = TRUE;
	}
    }

    if (new_rrop || new_fillspans || new_text || new_fillarea || new_line)
    {
	GCOps	*newops;

	if (newops = amigaGXMatchCommon (pGC, devPriv, 24))
 	{
	    if (pGC->ops->devPrivate.val)
		miDestroyGCOps (pGC->ops);
	    pGC->ops = newops;
	    new_rrop = new_line = new_fillspans = new_text = new_fillarea = 0;
	}
 	else
 	{
	    if (!pGC->ops->devPrivate.val)
	    {
		pGC->ops = miCreateGCOps (pGC->ops);
		pGC->ops->devPrivate.val = 1;
	    }
	}
    }

    canGX = pGC->fillStyle == FillSolid /*|| gxPriv->stipple*/;

    /* deal with the changes we've collected */
    if (new_line)
    {
	pGC->ops->FillPolygon = miFillPolygon;
	if (pGC->lineWidth == 0)
	  pGC->ops->PolyArc = miZeroPolyArc;
	else
	  pGC->ops->PolyArc = miPolyArc;
	pGC->ops->PolySegment = miPolySegment;
	switch (pGC->lineStyle)
	{
	case LineSolid:
#if 0
	    if(pGC->lineWidth == 0)
	    {
	        if (pGC->fillStyle == FillSolid)
		{
		  pGC->ops->Polylines = cfb32LineSS;
		  pGC->ops->PolySegment = cfb32SegmentSS;
		}
		else
		    pGC->ops->Polylines = miZeroLine;
	    }
	    else
		pGC->ops->Polylines = miWideLine;
#else
	    if (pGC->lineWidth == 0 && pGC->fillStyle != FillSolid)
	      pGC->ops->Polylines = miZeroLine;
	    else
	      pGC->ops->Polylines = miWideLine;
#endif
	    break;
	case LineOnOffDash:
	case LineDoubleDash:
#if 0
	    if (pGC->lineWidth == 0 && pGC->fillStyle == FillSolid)
	    {
		pGC->ops->Polylines = cfb32LineSD;
		pGC->ops->PolySegment = cfb32SegmentSD;
	    } else
#endif
		pGC->ops->Polylines = miWideDash;
	    break;
	}
    }

    if (new_text && (pGC->font))
    {
        if (FONTWIDTH(pGC->font) > 32 ||
	    FONTMINBOUNDS(pGC->font,characterWidth) < 0)
        {
            pGC->ops->PolyGlyphBlt = miPolyGlyphBlt;
            pGC->ops->ImageGlyphBlt = miImageGlyphBlt;
        }
        else
        {
	    if (pGC->fillStyle == FillSolid) 
	    {
		if (TERMINALFONT (pGC->font))
		    pGC->ops->PolyGlyphBlt = amigaGXPolyTEGlyphBlt;
		else
		    pGC->ops->PolyGlyphBlt = amigaGXPolyGlyphBlt;
	    }
	    else
		pGC->ops->PolyGlyphBlt = miPolyGlyphBlt;
            /* special case ImageGlyphBlt for terminal emulator fonts */
            if (TERMINALFONT(pGC->font))
		pGC->ops->ImageGlyphBlt = amigaGXTEGlyphBlt;
            else
                pGC->ops->ImageGlyphBlt = miImageGlyphBlt;
        }
    }    


    if (new_fillspans) {
#if 0
	if (canGX)
#endif
	    pGC->ops->FillSpans = amigaGXFillSpans;
#if 0
	else switch (pGC->fillStyle) {
	case FillTiled:
	    if (pGC->pRotatedPixmap)
	    {
		if (pGC->alu == GXcopy && (pGC->planemask & PMSK) == PMSK)
		    pGC->ops->FillSpans = cfb32Tile32FSCopy;
		else
		    pGC->ops->FillSpans = cfb32Tile32FSGeneral;
	    }
	    else
		pGC->ops->FillSpans = cfb32UnnaturalTileFS;
	    break;
	case FillStippled:
	    pGC->ops->FillSpans = cfb32UnnaturalStippleFS;
	    break;
	case FillOpaqueStippled:
	    pGC->ops->FillSpans = cfb32UnnaturalStippleFS;
	    break;
	default:
	    FatalError("cfb32ValidateGC: illegal fillStyle\n");
	}
#endif
    } /* end of new_fillspans */

    if (new_fillarea) {
	pGC->ops->PolyFillRect = miPolyFillRect;
	pGC->ops->PolyFillArc = miPolyFillArc;
	if (canGX)
	{
	    pGC->ops->PolyFillArc = amigaGXPolyFillArc;
	    pGC->ops->PolyFillRect = amigaGXPolyFillRect;
	}
	pGC->ops->PushPixels = mfbPushPixels;
	if (pGC->fillStyle == FillSolid && devPriv->rop == GXcopy)
	    pGC->ops->PushPixels = mfbPushPixels;
    }
}


void
amigaGXDestroyGC (pGC)
    GCPtr   pGC;
{
    amigaGXPrivGCPtr	    gxPriv = amigaGXGetGCPrivate (pGC);

#if 0
    if (gxPriv->stipple)
	xfree (gxPriv->stipple);
#endif
    miDestroyGC (pGC);
}

Bool
amigaGXCreateGC(GCPtr   pGC)
{
    amigaGXPrivGCPtr  gxPriv;
    if (pGC->depth == 1)
	return mfbCreateGC (pGC);
#if AMIGAMAXDEPTH == 32
    if (!amigaCfbCreateGC(pGC))
#else
    if (!cfbCreateGC (pGC))
#endif
	return FALSE;
    if (pGC->depth == 8)
      {
	pGC->ops = &amiga8GXNonTEOps;
	pGC->funcs = &amiga8GXGCFuncs;
      }
    else if (pGC->depth == 16)
      {
	pGC->ops = &amiga16GXNonTEOps;
	pGC->funcs = &amiga16GXGCFuncs;
      }
    else
      {
	pGC->ops = &amiga24GXNonTEOps;
	pGC->funcs = &amiga24GXGCFuncs;
      }
    gxPriv = amigaGXGetGCPrivate(pGC);
    gxPriv->type = DRAWABLE_WINDOW;
    gxPriv->stipple = 0;
    return TRUE;
}

Bool
amiga8GXCreateWindow (pWin)
    WindowPtr	pWin;
{
    if (!cfb8CreateWindow (pWin))
	return FALSE;
    pWin->devPrivates[amigaGXWindowPrivateIndex].ptr = 0;
    return TRUE;
}

Bool
amiga16GXCreateWindow (pWin)
    WindowPtr	pWin;
{
    if (!cfb16CreateWindow (pWin))
	return FALSE;
    pWin->devPrivates[amigaGXWindowPrivateIndex].ptr = 0;
    return TRUE;
}

Bool
amiga24GXCreateWindow (pWin)
    WindowPtr	pWin;
{
    if (!cfb32CreateWindow (pWin))
	return FALSE;
    pWin->devPrivates[amigaGXWindowPrivateIndex].ptr = 0;
    return TRUE;
}

Bool
amiga8GXDestroyWindow (pWin)
    WindowPtr	pWin;
{
#if 0
    amigaGXStipplePtr stipple;
    if (stipple = amigaGXGetWindowPrivate(pWin))
	xfree (stipple);
#endif
    return cfb8DestroyWindow (pWin);
}

Bool
amiga16GXDestroyWindow (pWin)
    WindowPtr	pWin;
{
#if 0
    amigaGXStipplePtr stipple;
    if (stipple = amigaGXGetWindowPrivate(pWin))
	xfree (stipple);
#endif
    return cfb16DestroyWindow (pWin);
}

Bool
amiga24GXDestroyWindow (pWin)
    WindowPtr	pWin;
{
#if 0
    amigaGXStipplePtr stipple;
    if (stipple = amigaGXGetWindowPrivate(pWin))
	xfree (stipple);
#endif
    return cfb32DestroyWindow (pWin);
}

Bool
amiga8GXChangeWindowAttributes(
    WindowPtr	pWin,
    Mask	mask)
{
#if 0
    amigaGXStipplePtr stipple;
#endif
    Mask	    index;
    WindowPtr	pBgWin;
    register cfbPrivWin *pPrivWin;
    int		    width;

    pPrivWin = (cfbPrivWin *)(pWin->devPrivates[cfbWindowPrivateIndex].ptr);
    /*
     * When background state changes from ParentRelative and
     * we had previously rotated the fast border pixmap to match
     * the parent relative origin, rerotate to match window
     */
    if (mask & (CWBackPixmap | CWBackPixel) &&
	pWin->backgroundState != ParentRelative &&
	pPrivWin->fastBorder &&
	(pPrivWin->oldRotate.x != pWin->drawable.x ||
	 pPrivWin->oldRotate.y != pWin->drawable.y))
    {
	cfb8XRotatePixmap(pPrivWin->pRotatedBorder,
		      pWin->drawable.x - pPrivWin->oldRotate.x);
	cfb8YRotatePixmap(pPrivWin->pRotatedBorder,
		      pWin->drawable.y - pPrivWin->oldRotate.y);
	pPrivWin->oldRotate.x = pWin->drawable.x;
	pPrivWin->oldRotate.y = pWin->drawable.y;
    }
    while (mask)
    {
	index = lowbit(mask);
	mask &= ~index;
	switch (index)
	{
	case CWBackPixmap:
#if 0
	    stipple = amigaGXGetWindowPrivate(pWin);
#endif
	    if (pWin->backgroundState == None ||
		pWin->backgroundState == ParentRelative)
	    {
		pPrivWin->fastBackground = FALSE;
#if 0
		if (stipple)
		{
		    xfree (stipple);
		    amigaGXSetWindowPrivate(pWin,0);
		}
#endif
		/* Rotate border to match parent origin */
		if (pWin->backgroundState == ParentRelative &&
		    pPrivWin->pRotatedBorder) 
		{
		    for (pBgWin = pWin->parent;
			 pBgWin->backgroundState == ParentRelative;
			 pBgWin = pBgWin->parent);
		    cfb8XRotatePixmap(pPrivWin->pRotatedBorder,
				  pBgWin->drawable.x - pPrivWin->oldRotate.x);
		    cfb8YRotatePixmap(pPrivWin->pRotatedBorder,
				  pBgWin->drawable.y - pPrivWin->oldRotate.y);
		}
		
		break;
	    }
#if 0
	    if (!stipple)
	    {
		if (!tmpStipple)
		    tmpStipple = (amigaGXStipplePtr) xalloc (sizeof *tmpStipple);
		stipple = tmpStipple;
	    }
 	    if (stipple && amigaGXCheckTile (pWin->background.pixmap, stipple))
	    {
		stipple->alu = gx_opaque_stipple_rop_table[GXcopy]|GX_PATTERN_MASK;
		pPrivWin->fastBackground = FALSE;
		if (stipple == tmpStipple)
		{
		    amigaGXSetWindowPrivate(pWin, stipple);
		    tmpStipple = 0;
		}
		break;
	    }
	    if (stipple = amigaGXGetWindowPrivate(pWin))
	    {
		xfree (stipple);
		amigaGXSetWindowPrivate(pWin,0);
	    }
#endif
 	    if (((width = (pWin->background.pixmap->drawable.width * PSZ)) <= 32) &&
		       !(width & (width - 1)))
	    {
		cfb8CopyRotatePixmap(pWin->background.pixmap,
				  &pPrivWin->pRotatedBackground,
				  pWin->drawable.x,
				  pWin->drawable.y);
		if (pPrivWin->pRotatedBackground)
		{
    	    	    pPrivWin->fastBackground = TRUE;
    	    	    pPrivWin->oldRotate.x = pWin->drawable.x;
    	    	    pPrivWin->oldRotate.y = pWin->drawable.y;
		}
		else
		{
		    pPrivWin->fastBackground = FALSE;
		}
		break;
	    }
	    pPrivWin->fastBackground = FALSE;
	    break;

	case CWBackPixel:
	    pPrivWin->fastBackground = FALSE;
	    break;

	case CWBorderPixmap:
	    /* don't bother with accelerator for border tiles (just lazy) */
	    if (((width = (pWin->border.pixmap->drawable.width * PSZ)) <= 32) &&
		!(width & (width - 1)))
	    {
		for (pBgWin = pWin;
		     pBgWin->backgroundState == ParentRelative;
		     pBgWin = pBgWin->parent);
		cfb8CopyRotatePixmap(pWin->border.pixmap,
				    &pPrivWin->pRotatedBorder,
				    pBgWin->drawable.x,
				    pBgWin->drawable.y);
		if (pPrivWin->pRotatedBorder)
		{
		    pPrivWin->fastBorder = TRUE;
		    pPrivWin->oldRotate.x = pBgWin->drawable.x;
		    pPrivWin->oldRotate.y = pBgWin->drawable.y;
		}
		else
		{
		    pPrivWin->fastBorder = TRUE;
		}
	    }
	    else
	    {
		pPrivWin->fastBorder = FALSE;
	    }
	    break;
	case CWBorderPixel:
	    pPrivWin->fastBorder = FALSE;
	    break;
	}
    }
    return (TRUE);
}

Bool
amiga16GXChangeWindowAttributes(
    WindowPtr	pWin,
    Mask	mask)
{
#if 0
    amigaGXStipplePtr stipple;
#endif
    Mask	    index;
    WindowPtr	pBgWin;
    register cfbPrivWin *pPrivWin;
    int		    width;

    pPrivWin = (cfbPrivWin *)(pWin->devPrivates[cfbWindowPrivateIndex].ptr);
    /*
     * When background state changes from ParentRelative and
     * we had previously rotated the fast border pixmap to match
     * the parent relative origin, rerotate to match window
     */
    if (mask & (CWBackPixmap | CWBackPixel) &&
	pWin->backgroundState != ParentRelative &&
	pPrivWin->fastBorder &&
	(pPrivWin->oldRotate.x != pWin->drawable.x ||
	 pPrivWin->oldRotate.y != pWin->drawable.y))
    {
	cfb16XRotatePixmap(pPrivWin->pRotatedBorder,
		      pWin->drawable.x - pPrivWin->oldRotate.x);
	cfb16YRotatePixmap(pPrivWin->pRotatedBorder,
		      pWin->drawable.y - pPrivWin->oldRotate.y);
	pPrivWin->oldRotate.x = pWin->drawable.x;
	pPrivWin->oldRotate.y = pWin->drawable.y;
    }
    while (mask)
    {
	index = lowbit(mask);
	mask &= ~index;
	switch (index)
	{
	case CWBackPixmap:
#if 0
	    stipple = amigaGXGetWindowPrivate(pWin);
#endif
	    if (pWin->backgroundState == None ||
		pWin->backgroundState == ParentRelative)
	    {
		pPrivWin->fastBackground = FALSE;
#if 0
		if (stipple)
		{
		    xfree (stipple);
		    amigaGXSetWindowPrivate(pWin,0);
		}
#endif
		/* Rotate border to match parent origin */
		if (pWin->backgroundState == ParentRelative &&
		    pPrivWin->pRotatedBorder) 
		{
		    for (pBgWin = pWin->parent;
			 pBgWin->backgroundState == ParentRelative;
			 pBgWin = pBgWin->parent);
		    cfb16XRotatePixmap(pPrivWin->pRotatedBorder,
				  pBgWin->drawable.x - pPrivWin->oldRotate.x);
		    cfb16YRotatePixmap(pPrivWin->pRotatedBorder,
				  pBgWin->drawable.y - pPrivWin->oldRotate.y);
		}
		
		break;
	    }
#if 0
	    if (!stipple)
	    {
		if (!tmpStipple)
		    tmpStipple = (amigaGXStipplePtr) xalloc (sizeof *tmpStipple);
		stipple = tmpStipple;
	    }
 	    if (stipple && amigaGXCheckTile (pWin->background.pixmap, stipple))
	    {
		stipple->alu = gx_opaque_stipple_rop_table[GXcopy]|GX_PATTERN_MASK;
		pPrivWin->fastBackground = FALSE;
		if (stipple == tmpStipple)
		{
		    amigaGXSetWindowPrivate(pWin, stipple);
		    tmpStipple = 0;
		}
		break;
	    }
	    if (stipple = amigaGXGetWindowPrivate(pWin))
	    {
		xfree (stipple);
		amigaGXSetWindowPrivate(pWin,0);
	    }
#endif
 	    if (((width = (pWin->background.pixmap->drawable.width * PSZ)) <= 32) &&
		       !(width & (width - 1)))
	    {
		cfb16CopyRotatePixmap(pWin->background.pixmap,
				  &pPrivWin->pRotatedBackground,
				  pWin->drawable.x,
				  pWin->drawable.y);
		if (pPrivWin->pRotatedBackground)
		{
    	    	    pPrivWin->fastBackground = TRUE;
    	    	    pPrivWin->oldRotate.x = pWin->drawable.x;
    	    	    pPrivWin->oldRotate.y = pWin->drawable.y;
		}
		else
		{
		    pPrivWin->fastBackground = FALSE;
		}
		break;
	    }
	    pPrivWin->fastBackground = FALSE;
	    break;

	case CWBackPixel:
	    pPrivWin->fastBackground = FALSE;
	    break;

	case CWBorderPixmap:
	    /* don't bother with accelerator for border tiles (just lazy) */
	    if (((width = (pWin->border.pixmap->drawable.width * PSZ)) <= 32) &&
		!(width & (width - 1)))
	    {
		for (pBgWin = pWin;
		     pBgWin->backgroundState == ParentRelative;
		     pBgWin = pBgWin->parent);
		cfb16CopyRotatePixmap(pWin->border.pixmap,
				    &pPrivWin->pRotatedBorder,
				    pBgWin->drawable.x,
				    pBgWin->drawable.y);
		if (pPrivWin->pRotatedBorder)
		{
		    pPrivWin->fastBorder = TRUE;
		    pPrivWin->oldRotate.x = pBgWin->drawable.x;
		    pPrivWin->oldRotate.y = pBgWin->drawable.y;
		}
		else
		{
		    pPrivWin->fastBorder = TRUE;
		}
	    }
	    else
	    {
		pPrivWin->fastBorder = FALSE;
	    }
	    break;
	case CWBorderPixel:
	    pPrivWin->fastBorder = FALSE;
	    break;
	}
    }
    return (TRUE);
}

Bool
amiga24GXChangeWindowAttributes(
    WindowPtr	pWin,
    Mask	mask)
{
#if 0
    amigaGXStipplePtr stipple;
#endif
    Mask	    index;
    WindowPtr	pBgWin;
    register cfbPrivWin *pPrivWin;
    int		    width;

    pPrivWin = (cfbPrivWin *)(pWin->devPrivates[cfbWindowPrivateIndex].ptr);
    /*
     * When background state changes from ParentRelative and
     * we had previously rotated the fast border pixmap to match
     * the parent relative origin, rerotate to match window
     */
    if (mask & (CWBackPixmap | CWBackPixel) &&
	pWin->backgroundState != ParentRelative &&
	pPrivWin->fastBorder &&
	(pPrivWin->oldRotate.x != pWin->drawable.x ||
	 pPrivWin->oldRotate.y != pWin->drawable.y))
    {
	cfb32XRotatePixmap(pPrivWin->pRotatedBorder,
		      pWin->drawable.x - pPrivWin->oldRotate.x);
	cfb32YRotatePixmap(pPrivWin->pRotatedBorder,
		      pWin->drawable.y - pPrivWin->oldRotate.y);
	pPrivWin->oldRotate.x = pWin->drawable.x;
	pPrivWin->oldRotate.y = pWin->drawable.y;
    }
    while (mask)
    {
	index = lowbit(mask);
	mask &= ~index;
	switch (index)
	{
	case CWBackPixmap:
#if 0
	    stipple = amigaGXGetWindowPrivate(pWin);
#endif
	    if (pWin->backgroundState == None ||
		pWin->backgroundState == ParentRelative)
	    {
		pPrivWin->fastBackground = FALSE;
#if 0
		if (stipple)
		{
		    xfree (stipple);
		    amigaGXSetWindowPrivate(pWin,0);
		}
#endif
		/* Rotate border to match parent origin */
		if (pWin->backgroundState == ParentRelative &&
		    pPrivWin->pRotatedBorder) 
		{
		    for (pBgWin = pWin->parent;
			 pBgWin->backgroundState == ParentRelative;
			 pBgWin = pBgWin->parent);
		    cfb32XRotatePixmap(pPrivWin->pRotatedBorder,
				  pBgWin->drawable.x - pPrivWin->oldRotate.x);
		    cfb32YRotatePixmap(pPrivWin->pRotatedBorder,
				  pBgWin->drawable.y - pPrivWin->oldRotate.y);
		}
		
		break;
	    }
#if 0
	    if (!stipple)
	    {
		if (!tmpStipple)
		    tmpStipple = (amigaGXStipplePtr) xalloc (sizeof *tmpStipple);
		stipple = tmpStipple;
	    }
 	    if (stipple && amigaGXCheckTile (pWin->background.pixmap, stipple))
	    {
		stipple->alu = gx_opaque_stipple_rop_table[GXcopy]|GX_PATTERN_MASK;
		pPrivWin->fastBackground = FALSE;
		if (stipple == tmpStipple)
		{
		    amigaGXSetWindowPrivate(pWin, stipple);
		    tmpStipple = 0;
		}
		break;
	    }
	    if (stipple = amigaGXGetWindowPrivate(pWin))
	    {
		xfree (stipple);
		amigaGXSetWindowPrivate(pWin,0);
	    }
#endif
 	    if (((width = (pWin->background.pixmap->drawable.width * PSZ)) <= 32) &&
		       !(width & (width - 1)))
	    {
		cfb32CopyRotatePixmap(pWin->background.pixmap,
				  &pPrivWin->pRotatedBackground,
				  pWin->drawable.x,
				  pWin->drawable.y);
		if (pPrivWin->pRotatedBackground)
		{
    	    	    pPrivWin->fastBackground = TRUE;
    	    	    pPrivWin->oldRotate.x = pWin->drawable.x;
    	    	    pPrivWin->oldRotate.y = pWin->drawable.y;
		}
		else
		{
		    pPrivWin->fastBackground = FALSE;
		}
		break;
	    }
	    pPrivWin->fastBackground = FALSE;
	    break;

	case CWBackPixel:
	    pPrivWin->fastBackground = FALSE;
	    break;

	case CWBorderPixmap:
	    /* don't bother with accelerator for border tiles (just lazy) */
	    if (((width = (pWin->border.pixmap->drawable.width * PSZ)) <= 32) &&
		!(width & (width - 1)))
	    {
		for (pBgWin = pWin;
		     pBgWin->backgroundState == ParentRelative;
		     pBgWin = pBgWin->parent);
		cfb32CopyRotatePixmap(pWin->border.pixmap,
				    &pPrivWin->pRotatedBorder,
				    pBgWin->drawable.x,
				    pBgWin->drawable.y);
		if (pPrivWin->pRotatedBorder)
		{
		    pPrivWin->fastBorder = TRUE;
		    pPrivWin->oldRotate.x = pBgWin->drawable.x;
		    pPrivWin->oldRotate.y = pBgWin->drawable.y;
		}
		else
		{
		    pPrivWin->fastBorder = TRUE;
		}
	    }
	    else
	    {
		pPrivWin->fastBorder = FALSE;
	    }
	    break;
	case CWBorderPixel:
	    pPrivWin->fastBorder = FALSE;
	    break;
	}
    }
    return (TRUE);
}

void
amiga8GXPaintWindow(pWin, pRegion, what)
    WindowPtr	pWin;
    RegionPtr	pRegion;
    int		what;
{
    register cfbPrivWin	*pPrivWin;
#if 0
    amigaGXStipplePtr stipple;
#endif
    WindowPtr	pBgWin;
    pPrivWin = (cfbPrivWin *)(pWin->devPrivates[cfbWindowPrivateIndex].ptr);

    switch (what) {
    case PW_BACKGROUND:
#if 0
	stipple = amigaGXGetWindowPrivate(pWin);
#endif
	switch (pWin->backgroundState) {
	case None:
	    return;
	case ParentRelative:
	    do {
		pWin = pWin->parent;
	    } while (pWin->backgroundState == ParentRelative);
	    (*pWin->drawable.pScreen->PaintWindowBackground)(pWin, pRegion,
							     what);
	    return;
	case BackgroundPixmap:
#if 0
	    if (stipple)
	    {
		amigaGXFillBoxStipple ((DrawablePtr)pWin,
				  (int)REGION_NUM_RECTS(pRegion),
				  REGION_RECTS(pRegion),
				  stipple);
	    }
	    else
#endif
	      if (pPrivWin->fastBackground)
	    {
		cfb8FillBoxTile32 ((DrawablePtr)pWin,
				  (int)REGION_NUM_RECTS(pRegion),
				  REGION_RECTS(pRegion),
				  pPrivWin->pRotatedBackground);
	    }
	    else
	    {
		cfb8FillBoxTileOdd ((DrawablePtr)pWin,
				   (int)REGION_NUM_RECTS(pRegion),
				   REGION_RECTS(pRegion),
				   pWin->background.pixmap,
				   (int) pWin->drawable.x, (int) pWin->drawable.y);
	    }
	    return;
	case BackgroundPixel:
	    amigaGXFillBoxSolid((DrawablePtr)pWin,
			     (int)REGION_NUM_RECTS(pRegion),
			     REGION_RECTS(pRegion),
			     pWin->background.pixel);
	    return;
    	}
    	break;
    case PW_BORDER:
	if (pWin->borderIsPixel)
	{
	    amigaGXFillBoxSolid((DrawablePtr)pWin,
			     (int)REGION_NUM_RECTS(pRegion),
			     REGION_RECTS(pRegion),
			     pWin->border.pixel);
	    return;
	}
	else if (pPrivWin->fastBorder)
	{
	    cfb8FillBoxTile32 ((DrawablePtr)pWin,
			      (int)REGION_NUM_RECTS(pRegion),
			      REGION_RECTS(pRegion),
			      pPrivWin->pRotatedBorder);
	    return;
	}
	else
	{
	    for (pBgWin = pWin;
		 pBgWin->backgroundState == ParentRelative;
		 pBgWin = pBgWin->parent);

	    cfb8FillBoxTileOdd ((DrawablePtr)pWin,
			       (int)REGION_NUM_RECTS(pRegion),
			       REGION_RECTS(pRegion),
			       pWin->border.pixmap,
			       (int) pBgWin->drawable.x,
 			       (int) pBgWin->drawable.y);
	    return;
	}
	break;
    }
}

void
amiga16GXPaintWindow(pWin, pRegion, what)
    WindowPtr	pWin;
    RegionPtr	pRegion;
    int		what;
{
    register cfbPrivWin	*pPrivWin;
#if 0
    amigaGXStipplePtr stipple;
#endif
    WindowPtr	pBgWin;
    pPrivWin = (cfbPrivWin *)(pWin->devPrivates[cfbWindowPrivateIndex].ptr);

    switch (what) {
    case PW_BACKGROUND:
#if 0
	stipple = amigaGXGetWindowPrivate(pWin);
#endif
	switch (pWin->backgroundState) {
	case None:
	    return;
	case ParentRelative:
	    do {
		pWin = pWin->parent;
	    } while (pWin->backgroundState == ParentRelative);
	    (*pWin->drawable.pScreen->PaintWindowBackground)(pWin, pRegion,
							     what);
	    return;
	case BackgroundPixmap:
#if 0
	    if (stipple)
	    {
		amigaGXFillBoxStipple ((DrawablePtr)pWin,
				  (int)REGION_NUM_RECTS(pRegion),
				  REGION_RECTS(pRegion),
				  stipple);
	    }
	    else
#endif
#if 1
	      if (pPrivWin->fastBackground)
	    {
		cfb16FillBoxTile32 ((DrawablePtr)pWin,
				  (int)REGION_NUM_RECTS(pRegion),
				  REGION_RECTS(pRegion),
				  pPrivWin->pRotatedBackground);
	    }
	    else
#endif
	    {
		cfb16FillBoxTileOdd ((DrawablePtr)pWin,
				   (int)REGION_NUM_RECTS(pRegion),
				   REGION_RECTS(pRegion),
				   pWin->background.pixmap,
				   (int) pWin->drawable.x, (int) pWin->drawable.y);
	    }
	    return;
	case BackgroundPixel:
	    amigaGXFillBoxSolid((DrawablePtr)pWin,
			     (int)REGION_NUM_RECTS(pRegion),
			     REGION_RECTS(pRegion),
			     pWin->background.pixel);
	    return;
    	}
    	break;
    case PW_BORDER:
	if (pWin->borderIsPixel)
	{
	    amigaGXFillBoxSolid((DrawablePtr)pWin,
			     (int)REGION_NUM_RECTS(pRegion),
			     REGION_RECTS(pRegion),
			     pWin->border.pixel);
	    return;
	}
	else
#if 1
 if (pPrivWin->fastBorder)
	{
	    cfb16FillBoxTile32 ((DrawablePtr)pWin,
			      (int)REGION_NUM_RECTS(pRegion),
			      REGION_RECTS(pRegion),
			      pPrivWin->pRotatedBorder);
	    return;
	}
	else
#endif
	{
	    for (pBgWin = pWin;
		 pBgWin->backgroundState == ParentRelative;
		 pBgWin = pBgWin->parent);

	    cfb16FillBoxTileOdd ((DrawablePtr)pWin,
			       (int)REGION_NUM_RECTS(pRegion),
			       REGION_RECTS(pRegion),
			       pWin->border.pixmap,
			       (int) pBgWin->drawable.x,
 			       (int) pBgWin->drawable.y);
	    return;
	}
	break;
    }
}

void
amiga24GXPaintWindow(pWin, pRegion, what)
    WindowPtr	pWin;
    RegionPtr	pRegion;
    int		what;
{
    register cfbPrivWin	*pPrivWin;
#if 0
    amigaGXStipplePtr stipple;
#endif
    WindowPtr	pBgWin;
    pPrivWin = (cfbPrivWin *)(pWin->devPrivates[cfbWindowPrivateIndex].ptr);

    switch (what) {
    case PW_BACKGROUND:
#if 0
	stipple = amigaGXGetWindowPrivate(pWin);
#endif
	switch (pWin->backgroundState) {
	case None:
	    return;
	case ParentRelative:
	    do {
		pWin = pWin->parent;
	    } while (pWin->backgroundState == ParentRelative);
	    (*pWin->drawable.pScreen->PaintWindowBackground)(pWin, pRegion,
							     what);
	    return;
	case BackgroundPixmap:
#if 0
#if 0
	    if (stipple)
	    {
		amigaGXFillBoxStipple ((DrawablePtr)pWin,
				  (int)REGION_NUM_RECTS(pRegion),
				  REGION_RECTS(pRegion),
				  stipple);
	    }
	    else
#endif
	      if (pPrivWin->fastBackground)
	    {
		cfb32FillBoxTile32 ((DrawablePtr)pWin,
				  (int)REGION_NUM_RECTS(pRegion),
				  REGION_RECTS(pRegion),
				  pPrivWin->pRotatedBackground);
	    }
	    else
	    {
		cfb32FillBoxTileOdd ((DrawablePtr)pWin,
				   (int)REGION_NUM_RECTS(pRegion),
				   REGION_RECTS(pRegion),
				   pWin->background.pixmap,
				   (int) pWin->drawable.x, (int) pWin->drawable.y);
	    }
	    return;
#endif
	case BackgroundPixel:
	    amigaGXFillBoxSolid((DrawablePtr)pWin,
			     (int)REGION_NUM_RECTS(pRegion),
			     REGION_RECTS(pRegion),
			     pWin->background.pixel);
	    return;
    	}
    	break;
    case PW_BORDER:
#if 0
	if (pWin->borderIsPixel)
	{
#endif
	    amigaGXFillBoxSolid((DrawablePtr)pWin,
			     (int)REGION_NUM_RECTS(pRegion),
			     REGION_RECTS(pRegion),
			     pWin->border.pixel);
	    return;
#if 0
	}
	else
#if 1
 if (pPrivWin->fastBorder)
	{
	    cfb32FillBoxTile32 ((DrawablePtr)pWin,
			      (int)REGION_NUM_RECTS(pRegion),
			      REGION_RECTS(pRegion),
			      pPrivWin->pRotatedBorder);
	    return;
	}
	else
#endif
	{
	    for (pBgWin = pWin;
		 pBgWin->backgroundState == ParentRelative;
		 pBgWin = pBgWin->parent);

	    cfb32FillBoxTileOdd ((DrawablePtr)pWin,
			       (int)REGION_NUM_RECTS(pRegion),
			       REGION_RECTS(pRegion),
			       pWin->border.pixmap,
			       (int) pBgWin->drawable.x,
 			       (int) pBgWin->drawable.y);
	    return;
	}
#endif
	break;
    }
}

void 
amigaGXCopyWindow(pWin, ptOldOrg, prgnSrc)
    WindowPtr pWin;
    DDXPointRec ptOldOrg;
    RegionPtr prgnSrc;
{
    DDXPointPtr pptSrc;
    register DDXPointPtr ppt;
    RegionPtr prgnDst;
    register BoxPtr pbox;
    register int dx, dy;
    register int i, nbox;
    WindowPtr pwinRoot;
    extern WindowPtr *WindowTable;

    pwinRoot = WindowTable[pWin->drawable.pScreen->myNum];

    prgnDst = REGION_CREATE(pWin->drawable.pScreen, NULL, 1);

    dx = ptOldOrg.x - pWin->drawable.x;
    dy = ptOldOrg.y - pWin->drawable.y;
    REGION_TRANSLATE(pWin->drawable.pScreen, prgnSrc, -dx, -dy);
    REGION_INTERSECT(pWin->drawable.pScreen, prgnDst, &pWin->borderClip, prgnSrc);

    pbox = REGION_RECTS(prgnDst);
    nbox = REGION_NUM_RECTS(prgnDst);
    if(!(pptSrc = (DDXPointPtr )ALLOCATE_LOCAL(nbox * sizeof(DDXPointRec))))
	return;
    ppt = pptSrc;

    for (i = nbox; --i >= 0; ppt++, pbox++)
    {
	ppt->x = pbox->x1 + dx;
	ppt->y = pbox->y1 + dy;
    }

    amigaGXDoBitblt ((DrawablePtr)pwinRoot, (DrawablePtr)pwinRoot,
		    GXcopy, prgnDst, pptSrc, ~0L);
    DEALLOCATE_LOCAL(pptSrc);
    REGION_DESTROY(pWin->drawable.pScreen, prgnDst);
}

Bool
amigaGXInit (
    ScreenPtr	pScreen,
    fbFd	*fb)
{
    Uint	    mode;
    register long   r;

    if (serverGeneration != amigaGXGeneration)
    {
	amigaGXScreenPrivateIndex = AllocateScreenPrivateIndex();
	if (amigaGXScreenPrivateIndex == -1)
	    return FALSE;
	amigaGXGCPrivateIndex = AllocateGCPrivateIndex ();
	amigaGXWindowPrivateIndex = AllocateWindowPrivateIndex ();
	amigaGXGeneration = serverGeneration;
    }
    if (!AllocateGCPrivate(pScreen, amigaGXGCPrivateIndex, sizeof (amigaGXPrivGCRec)))
	return FALSE;
    if (!AllocateWindowPrivate(pScreen, amigaGXWindowPrivateIndex, 0))
	return FALSE;
    /*
     * Replace various screen functions
     */

    if (fb->info.gd_planes == 8)
      {
	pScreen->CreateGC = amigaGXCreateGC;
	pScreen->CreateWindow = amiga8GXCreateWindow;
	pScreen->ChangeWindowAttributes = amiga8GXChangeWindowAttributes;
	pScreen->DestroyWindow = amiga8GXDestroyWindow;
	pScreen->PaintWindowBackground = amiga8GXPaintWindow;
	pScreen->PaintWindowBorder = amiga8GXPaintWindow;
	pScreen->CopyWindow = amigaGXCopyWindow;
      }
    else if (fb->info.gd_planes == 16)
      {
	pScreen->CreateGC = amigaGXCreateGC;
	pScreen->CreateWindow = amiga16GXCreateWindow;
	pScreen->ChangeWindowAttributes = amiga16GXChangeWindowAttributes;
	pScreen->DestroyWindow = amiga16GXDestroyWindow;
	pScreen->PaintWindowBackground = amiga16GXPaintWindow;
	pScreen->PaintWindowBorder = amiga16GXPaintWindow;
	pScreen->CopyWindow = amigaGXCopyWindow;
      }
    else
      {
	pScreen->CreateGC = amigaGXCreateGC;
	pScreen->CreateWindow = amiga24GXCreateWindow;
	pScreen->ChangeWindowAttributes = amiga24GXChangeWindowAttributes;
	pScreen->DestroyWindow = amiga24GXDestroyWindow;
	pScreen->PaintWindowBackground = amiga24GXPaintWindow;
	pScreen->PaintWindowBorder = amiga24GXPaintWindow;
	pScreen->CopyWindow = amigaGXCopyWindow;

	pScreen->GetSpans = amiga24GXGetSpans;
	pScreen->GetImage = amiga24GXGetImage;
	pScreen->ResolveColor = amiga24GXResolveColor;
	pScreen->CreateColormap = amiga24GXInitializeColormap;
      }
    return TRUE;
}


/* GetSpans -- for each span, gets bits from drawable starting at ppt[i]
 * and continuing for pwidth[i] bits
 * Each scanline returned will be server scanline padded, i.e., it will come
 * out to an integral number of words.
 */
void
amiga24GXGetSpans(pDrawable, wMax, ppt, pwidth, nspans, pchardstStart)
    DrawablePtr		pDrawable;	/* drawable from which to get bits */
    int			wMax;		/* largest value of all *pwidths */
    register DDXPointPtr ppt;		/* points to start copying from */
    int			*pwidth;	/* list of number of bits to copy */
    int			nspans;		/* number of scanlines to copy */
    char		*pchardstStart; /* where to put the bits */
{
    unsigned long	*pdstStart = (unsigned long *)pchardstStart;
    register unsigned long	*pdst;		/* where to put the bits */
    register unsigned char	*psrc;		/* where to get the bits */
    register unsigned long	tmpSrc;		/* scratch buffer for bits */
    unsigned char		*psrcBase;	/* start of src bitmap */
    int			widthSrc;	/* width of pixmap in bytes */
    register DDXPointPtr pptLast;	/* one past last point to get */
    int         	xEnd;		/* last pixel to copy from */
    register int	nstart; 
    int	 		nend; 
    unsigned char	startmask, endmask;
    int			nlMiddle, nl, srcBit;
    int			w;
    unsigned long		*pdstNext;

    switch (pDrawable->bitsPerPixel) {
    case 1:
	mfbGetSpans(pDrawable, wMax, ppt, pwidth, nspans, pchardstStart);
	return;
    case 8:
	cfbGetSpans(pDrawable, wMax, ppt, pwidth, nspans, pchardstStart);
	return;
    case 16:
	cfb16GetSpans(pDrawable, wMax, ppt, pwidth, nspans, pchardstStart);
	return;
    case 32:
	break;
    default:
	FatalError("amiga24GXGetSpans: invalid depth\n");
    }

    if (pDrawable->type != DRAWABLE_WINDOW)
      {
	cfb32GetSpans (pDrawable, wMax, ppt, pwidth, nspans, pchardstStart);
	return;
      }

    cfb32GetByteWidthAndPointer (pDrawable, widthSrc, psrcBase)

    pdst = pdstStart;
    pptLast = ppt + nspans;
    while(ppt < pptLast)
    {
	xEnd = min(ppt->x + *pwidth, widthSrc);
	psrc = psrcBase + (ppt->y * widthSrc + ppt->x) * 3;
	w = xEnd - ppt->x;

	while (w--)
	  {
	    /* breaks convention of only accessing fb's long aligned */
	    *pdst++ = *(unsigned long *)psrc >> 8;
	    psrc += 3;
	  }

        ppt++;
	pwidth++;
    }
}


/* cfbSetScanline -- copies the bits from psrc to the drawable starting at
 * (xStart, y) and continuing to (xEnd, y).  xOrigin tells us where psrc 
 * starts on the scanline. (I.e., if this scanline passes through multiple
 * boxes, we may not want to start grabbing bits at psrc but at some offset
 * further on.) 
 */
void
amiga24GXSetScanline(
    fbFd		*inf,
    struct ACM		*acm,
    int			y,
    int			xOrigin,	/* where this scanline starts */
    int			xStart,		/* first bit to use from scanline */
    int			xEnd,		/* last bit to use from scanline + 1 */
    unsigned int	*psrc,
    int			alu,		/* raster op */
    int			*pdstBase,	/* start of the drawable */
    int			widthDst,	/* width of drawable in pixels */
    unsigned long	planemask)
{
    int			w;		/* width of scanline in bits */
    Uint		dst, dim;
    VUint		*feed = (VUint *) inf->fb;
    Uint		mask = 0x00ffffff;
    Uint 		l1, l2, l3, l4;

    if ((mask & planemask) == mask)
      {
	/* no masking, simple rop table */
	RZ3BlitInit(acm, blit_rop_table, alu);
      }
    else
      {
	/* use pattern as mask, and replicate the planemask as many 
	 * times as necessary. */
	RZ3BlitInit(acm, blit_rop_table_masked, alu);
	RZ3MaskInit(acm, inf->fb, planemask, 3);
	dst = 8 * PAT_MEM_OFF;
	M2I(dst);
	acm->pattern = dst;
      }

    psrc += xStart - xOrigin;
    w = xEnd - xStart;

    dst = 24 * (y * widthDst + xStart);
    M2I(dst);
    acm->dst = dst;
    acm->src = 0;
    acm->control = 0xc062;
    dim = w | (1 << 16);
    M2I(dim);
    acm->dimension = dim;
    acm->start = 0;
    acm->start = 1;

    while (w >= 4)
      {
	l1 = *psrc++ << 8;
	l2 = *psrc++ << 8;
	l3 = *psrc++ << 8;
	l4 = *psrc++ << 8;
	*feed = l1 | (l2 >> 24);
	*feed = (l2 << 8) | (l3 >> 16);
	*feed = (l3 << 16) | (l4 >> 8);
	w -= 4;
      }
    
    l1 = l2 = l3 = 0;
    switch (w)
      {
      case 3:
	l3 = psrc[2] << 8;
	/* fall into */

      case 2:
	l2 = psrc[1] << 8;
	/* fall into */

      case 1:
	l1 = psrc[0] << 8;
	/* fall into */

      case 0:
	psrc += w;
	break;
      }

    if (w-- > 0)
      {
	*feed = l1 | (l2 >> 24);

	if (w-- > 0)
	  {
	    *feed = (l2 << 8) | (l3 >> 16);

	    if (w-- > 0)
	      *feed = l3 << 16;
	  }

      }

    RZ3WaitDone(acm);
}



/* SetSpans -- for each span copy pwidth[i] bits from psrc to pDrawable at
 * ppt[i] using the raster op from the GC.  If fSorted is TRUE, the scanlines
 * are in increasing Y order.
 * Source bit lines are server scanline padded so that they always begin
 * on a word boundary.
 */ 
void
amiga24GXSetSpans(pDrawable, pGC, pcharsrc, ppt, pwidth, nspans, fSorted)
    DrawablePtr		pDrawable;
    GCPtr		pGC;
    char		*pcharsrc;
    register DDXPointPtr ppt;
    int			*pwidth;
    int			nspans;
    int			fSorted;
{
    unsigned int	*psrc = (unsigned int *)pcharsrc;
    unsigned long	*pdstBase;	/* start of dst bitmap */
    int 		widthDst;	/* width of bitmap in pixels */
    register BoxPtr 	pbox, pboxLast, pboxTest;
    register DDXPointPtr pptLast;
    int 		alu;
    RegionPtr 		prgnDst;
    int			xStart, xEnd;
    int			yMax;
    fbFd *inf = amigaInfo(pDrawable->pScreen);
    struct ACM *acm = (struct ACM *) (inf->regs + ACM_OFFSET);

    if (pDrawable->type != DRAWABLE_WINDOW)
      {
	cfb32SetSpans (pDrawable, pGC, pcharsrc, ppt, pwidth, nspans,
		       fSorted);
	return;
      }

    alu = pGC->alu;
    prgnDst = cfbGetCompositeClip(pGC);
    pptLast = ppt + nspans;

    cfb32GetLongWidthAndPointer (pDrawable, widthDst, pdstBase)

    yMax = (int) pDrawable->y + (int) pDrawable->height;

    pbox = REGION_RECTS(prgnDst);
    pboxLast = pbox + REGION_NUM_RECTS(prgnDst);

    if(fSorted)
    {
    /* scan lines sorted in ascending order. Because they are sorted, we
     * don't have to check each scanline against each clip box.  We can be
     * sure that this scanline only has to be clipped to boxes at or after the
     * beginning of this y-band 
     */
	pboxTest = pbox;
	while(ppt < pptLast)
	{
	    pbox = pboxTest;
	    if(ppt->y >= yMax)
		break;
	    while(pbox < pboxLast)
	    {
		if(pbox->y1 > ppt->y)
		{
		    /* scanline is before clip box */
		    break;
		}
		else if(pbox->y2 <= ppt->y)
		{
		    /* clip box is before scanline */
		    pboxTest = ++pbox;
		    continue;
		}
		else if(pbox->x1 > ppt->x + *pwidth) 
		{
		    /* clip box is to right of scanline */
		    break;
		}
		else if(pbox->x2 <= ppt->x)
		{
		    /* scanline is to right of clip box */
		    pbox++;
		    continue;
		}

		/* at least some of the scanline is in the current clip box */
		xStart = max(pbox->x1, ppt->x);
		xEnd = min(ppt->x + *pwidth, pbox->x2);
		amiga24GXSetScanline(inf, acm,
				     ppt->y, ppt->x, xStart, xEnd, psrc, alu,
				     (int *)pdstBase, widthDst, pGC->planemask);
		if(ppt->x + *pwidth <= pbox->x2)
		{
		    /* End of the line, as it were */
		    break;
		}
		else
		    pbox++;
	    }
	    /* We've tried this line against every box; it must be outside them
	     * all.  move on to the next point */
	    ppt++;
	    psrc += PixmapWidthInPadUnits(*pwidth, pDrawable->depth);
	    pwidth++;
	}
    }
    else
    {
    /* scan lines not sorted. We must clip each line against all the boxes */
	while(ppt < pptLast)
	{
	    if(ppt->y >= 0 && ppt->y < yMax)
	    {
		
		for(pbox = REGION_RECTS(prgnDst); pbox< pboxLast; pbox++)
		{
		    if(pbox->y1 > ppt->y)
		    {
			/* rest of clip region is above this scanline,
			 * skip it */
			break;
		    }
		    if(pbox->y2 <= ppt->y)
		    {
			/* clip box is below scanline */
			pbox++;
			break;
		    }
		    if(pbox->x1 <= ppt->x + *pwidth &&
		       pbox->x2 > ppt->x)
		    {
			xStart = max(pbox->x1, ppt->x);
			xEnd = min(pbox->x2, ppt->x + *pwidth);
			amiga24GXSetScanline(inf, acm, ppt->y, ppt->x, xStart, xEnd, psrc, alu,
			    (int *)pdstBase, widthDst, pGC->planemask);
		    }

		}
	    }
	psrc += PixmapWidthInPadUnits(*pwidth, pDrawable->depth);
	ppt++;
	pwidth++;
	}
    }
}

void
amiga24GXGetImage(pDrawable, sx, sy, w, h, format, planeMask, pdstLine)
    DrawablePtr pDrawable;
    int		sx, sy, w, h;
    unsigned int format;
    unsigned long planeMask;
    char	*pdstLine;
{
    switch (pDrawable->bitsPerPixel)
    {
    case 1:
	mfbGetImage(pDrawable, sx, sy, w, h, format, planeMask, pdstLine);
	break;
    case 8:
	cfbGetImage(pDrawable, sx, sy, w, h, format, planeMask, pdstLine);
	break;
    case 16:
	cfb16GetImage(pDrawable, sx, sy, w, h, format, planeMask, pdstLine);
	break;
    case 32:
	if (pDrawable->type != DRAWABLE_WINDOW)
	  cfb32GetImage(pDrawable, sx, sy, w, h, format, planeMask, pdstLine);
	else
	  miGetImage(pDrawable, sx, sy, w, h, format, planeMask, pdstLine);
	break;
    }
}

/* these two in cfb just don't work with 24bit deep color data ... */
void
amiga24GXResolveColor(pred, pgreen, pblue, pVisual)
    unsigned short	*pred, *pgreen, *pblue;
    register VisualPtr	pVisual;
{
  /* do nothing.. */
}

Bool
amiga24GXInitializeColormap(pmap)
    register ColormapPtr	pmap;
{
    register unsigned i;
    register VisualPtr pVisual;
    unsigned maxent;

    pVisual = pmap->pVisual;
    maxent = pVisual->ColormapEntries - 1;
    
    for(i = 0; i <= maxent; i++)
      {
	pmap->red[i].co.local.red = (i << 8) | i;
	pmap->blue[i].co.local.blue = (i << 8) | i;
	pmap->green[i].co.local.green = (i << 8) | i;

      }

    return TRUE;
}


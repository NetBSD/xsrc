/*
 * $XFree86: xc/programs/Xserver/hw/xfree86/xaa/xaaPict.c,v 1.5 2000/10/22 20:54:30 mvojkovi Exp $
 *
 * Copyright © 2000 Keith Packard, member of The XFree86 Project, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Keith Packard not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  Keith Packard makes no
 * representations about the suitability of this software for any purpose.  It
 * is provided "as is" without express or implied warranty.
 *
 * KEITH PACKARD DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL KEITH PACKARD BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

#include "misc.h"
#include "xf86.h"
#include "xf86_ansic.h"
#include "xf86_OSproc.h"

#include "X.h"
#include "scrnintstr.h"
#include "pixmapstr.h"
#include "windowstr.h"
#include "xf86str.h"
#include "mi.h"
#include "picturestr.h"
#include "glyphstr.h"
#include "picture.h"
#include "mipict.h"
#include "xaa.h"
#include "xaalocal.h"
#include "xaawrap.h"
#include "xf86fbman.h"
#include "servermd.h"

static Bool
XAAGetRGBAFromPixel(
    CARD32 pixel,
    CARD16 *red,
    CARD16 *green,
    CARD16 *blue,
    CARD16 *alpha,
    CARD32 format
){

    *alpha = 0xffff;

    switch(PICT_FORMAT_BPP(format)) {
    case 32:
	switch(format) {
	case PICT_a8r8g8b8:
	    *alpha = (pixel >> 24) & 0x000000ff;
	    *alpha |= *alpha << 8;
	case PICT_x8r8g8b8:
	    *blue = pixel & 0x000000ff;
	    *blue |= *blue << 8;
	    *green = pixel & 0x0000ff00;
	    *green |= *green >> 8;
	    *red = (pixel >> 16) & 0x000000ff;
	    *red |= *red << 8;
	    return TRUE;
	case PICT_a8b8g8r8:
	    *alpha = (pixel >> 24) & 0x000000ff;
	    *alpha |= *alpha << 8;
	case PICT_x8b8g8r8:
	    *red = pixel & 0x000000ff;
	    *red |= *red << 8;
	    *green = pixel & 0x0000ff00;
	    *green |= *green >> 8;
	    *blue = (pixel >> 16) & 0x000000ff;
	    *blue |= *blue << 8;
	    return TRUE;
	default:
	    break;
	}
	break;
    case 24:
	switch(format) {
	case PICT_r8g8b8:
	    *blue = pixel & 0x000000ff;
	    *blue |= *blue << 8;
	    *green = pixel & 0x0000ff00;
	    *green |= *green >> 8;
	    *red = (pixel >> 16) & 0x000000ff;
	    *red |= *red << 8;
	    return TRUE;
	case PICT_b8g8r8:
	    *red = pixel & 0x000000ff;
	    *red |= *red << 8;
	    *green = pixel & 0x0000ff00;
	    *green |= *green >> 8;
	    *blue = (pixel >> 16) & 0x000000ff;
	    *blue |= *blue << 8;
	    return TRUE;
	default:
	     break;
	}
	break;
    case 16:
    case 8:
    case 4:
    default:
	return FALSE;
    }

    return FALSE;
}

/* 8:8:8 + PICT_a8 -> 8:8:8:8 texture */

void
XAA_888_plus_PICT_a8_to_8888 (
    CARD32 color,
    CARD8  *alphaPtr,   /* in bytes */
    int    alphaPitch,
    CARD32  *dstPtr,
    int    dstPitch,	/* in dwords */
    int    width,
    int    height
){
    int x;

    color &= 0x00ffffff;

    while(height--) {
	for(x = 0; x < width; x++)
	   dstPtr[x] = color | (alphaPtr[x] << 24);
	dstPtr += dstPitch;
	alphaPtr += alphaPitch;
    } 
}


Bool
XAADoComposite (
    CARD8      op,
    PicturePtr pSrc,
    PicturePtr pMask,
    PicturePtr pDst,
    INT16      xSrc,
    INT16      ySrc,
    INT16      xMask,
    INT16      yMask,
    INT16      xDst,
    INT16      yDst,
    CARD16     width,
    CARD16     height
){
    ScreenPtr pScreen = pDst->pDrawable->pScreen;
    XAAInfoRecPtr infoRec = GET_XAAINFORECPTR_FROM_SCREEN(pScreen);
    RegionRec region;
    CARD32 *formats;
    BoxPtr pbox;
    int nbox;

    if(!infoRec->pScrn->vtSema || 
      ((pDst->pDrawable->type != DRAWABLE_WINDOW) &&
	!IS_OFFSCREEN_PIXMAP(pDst->pDrawable)))
	return FALSE;
  
    xDst += pDst->pDrawable->x;
    yDst += pDst->pDrawable->y;
    xSrc += pSrc->pDrawable->x;
    ySrc += pSrc->pDrawable->y;

    if(pMask) {
	/* for now we only do it if there is a 1x1 (solid) source */

	if((pSrc->pDrawable->width == 1) && (pSrc->pDrawable->height == 1)) {
	   CARD16 red, green, blue, alpha;
           CARD32 pixel =
                *((CARD32*)(((PixmapPtr)(pSrc->pDrawable))->devPrivate.ptr));

	   if(!XAAGetRGBAFromPixel(pixel,&red,&green,&blue,&alpha,pSrc->format))
		return FALSE;

	   if((alpha != 0xffff) &&
              (infoRec->CPUToScreenAlphaTextureFlags & XAA_RENDER_NO_SRC_ALPHA))
		return FALSE;

	   formats = infoRec->CPUToScreenAlphaTextureFormats;

	   while(*formats != pMask->format) {
		if(!(*formats)) return FALSE;
		formats++;
           }

	   xMask += pMask->pDrawable->x;
	   yMask += pMask->pDrawable->y;

	   if (!miComputeCompositeRegion (&region, pSrc, pMask, pDst,
                                   xSrc, ySrc, xMask, yMask, xDst, yDst,
                                   width, height))
		return TRUE;

	  nbox = REGION_NUM_RECTS(&region);
	  pbox = REGION_RECTS(&region);   
	     
	  if(!nbox)
		return TRUE;

	  if(!(infoRec->SetupForCPUToScreenAlphaTexture)(infoRec->pScrn,
			op, red, green, blue, alpha, pMask->format, 
			((PixmapPtr)(pMask->pDrawable))->devPrivate.ptr,
			((PixmapPtr)(pMask->pDrawable))->devKind, 
			pMask->pDrawable->width, pMask->pDrawable->height, 0))
		return FALSE;

	   xMask -= xDst;
	   yMask -= yDst;
	
	   while(nbox--) {
	      (*infoRec->SubsequentCPUToScreenAlphaTexture)(infoRec->pScrn,
			pbox->x1, pbox->y1, 
			pbox->x1 + xMask, pbox->y1 + yMask,
			pbox->x2 - pbox->x1, pbox->y2 - pbox->y1);
	      pbox++;
	   }

	   SET_SYNC_FLAG(infoRec);
	   REGION_UNINIT(pScreen, &region);
	   return TRUE;
	}
    } else {	
	formats = infoRec->CPUToScreenTextureFormats;

	while(*formats != pSrc->format) {
	    if(!(*formats)) return FALSE;
	    formats++;
	}

	if (!miComputeCompositeRegion (&region, pSrc, pMask, pDst,
                                   xSrc, ySrc, xMask, yMask, xDst, yDst,
                                   width, height))
		return TRUE;

	nbox = REGION_NUM_RECTS(&region);
	pbox = REGION_RECTS(&region);   
	     
	if(!nbox)
	    return TRUE;

	if(!(infoRec->SetupForCPUToScreenTexture)(infoRec->pScrn,
			op, pSrc->format, 
			((PixmapPtr)(pSrc->pDrawable))->devPrivate.ptr,
			((PixmapPtr)(pSrc->pDrawable))->devKind, 
			pSrc->pDrawable->width, pSrc->pDrawable->height, 0))
		return FALSE;

	xSrc -= xDst;
	ySrc -= yDst;
	
	while(nbox--) {
	    (*infoRec->SubsequentCPUToScreenTexture)(infoRec->pScrn,
			pbox->x1, pbox->y1, 
			pbox->x1 + xSrc, pbox->y1 + ySrc,
			pbox->x2 - pbox->x1, pbox->y2 - pbox->y1);
	    pbox++;
	 }

	SET_SYNC_FLAG(infoRec);
	REGION_UNINIT(pScreen, &region);
	return TRUE;
    }


    return FALSE;
}


void
XAAComposite (CARD8      op,
	      PicturePtr pSrc,
	      PicturePtr pMask,
	      PicturePtr pDst,
	      INT16      xSrc,
	      INT16      ySrc,
	      INT16      xMask,
	      INT16      yMask,
	      INT16      xDst,
	      INT16      yDst,
	      CARD16     width,
	      CARD16     height)
{
    ScreenPtr	pScreen = pDst->pDrawable->pScreen;
    XAAInfoRecPtr infoRec = GET_XAAINFORECPTR_FROM_SCREEN(pScreen);
    XAA_RENDER_PROLOGUE(pScreen, Composite);

    if(!infoRec->Composite ||
       !(*infoRec->Composite)(op, pSrc, pMask, pDst,
                       xSrc, ySrc, xMask, yMask, xDst, yDst,
                       width, height))
    { 
        SYNC_CHECK(pDst->pDrawable);
        (*GetPictureScreen(pScreen)->Composite) (op,
		       pSrc,
		       pMask,
		       pDst,
		       xSrc,
		       ySrc,
		       xMask,
		       yMask,
		       xDst,
		       yDst,
		       width,
		       height);    
    }

    if(pDst->pDrawable->type == DRAWABLE_PIXMAP)
	(XAA_GET_PIXMAP_PRIVATE((PixmapPtr)(pDst->pDrawable)))->flags |= DIRTY;

    XAA_RENDER_EPILOGUE(pScreen, Composite, XAAComposite);
}

void
XAAGlyphs (CARD8         op,
	   PicturePtr    pSrc,
	   PicturePtr    pDst,
	   PictFormatPtr maskFormat,
	   INT16         xSrc,
	   INT16         ySrc,
	   int           nlist,
	   GlyphListPtr  list,
	   GlyphPtr      *glyphs)
{
    ScreenPtr	pScreen = pDst->pDrawable->pScreen;
    XAAInfoRecPtr infoRec = GET_XAAINFORECPTR_FROM_SCREEN(pScreen);
    XAA_RENDER_PROLOGUE(pScreen, Glyphs);

    if(!infoRec->Glyphs ||
       !(*infoRec->Glyphs)(op, pSrc, pDst, maskFormat,
                                          xSrc, ySrc, nlist, list, glyphs))
    {
       SYNC_CHECK(pDst->pDrawable);
       (*GetPictureScreen(pScreen)->Glyphs) (op, pSrc, pDst, maskFormat,
					  xSrc, ySrc, nlist, list, glyphs);
    }

    if(pDst->pDrawable->type == DRAWABLE_PIXMAP)
	(XAA_GET_PIXMAP_PRIVATE((PixmapPtr)(pDst->pDrawable)))->flags |= DIRTY;

    XAA_RENDER_EPILOGUE(pScreen, Glyphs, XAAGlyphs);
}

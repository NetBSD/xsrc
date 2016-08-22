/* $NetBSD: ffb_exa.c,v 1.4 2016/08/22 08:28:32 mrg Exp $ */
/*
 * Copyright (c) 2015 Michael Lorenz
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *    - Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *    - Redistributions in binary form must reproduce the above
 *      copyright notice, this list of conditions and the following
 *      disclaimer in the documentation and/or other materials provided
 *      with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */
 
#include <sys/types.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "ffb_fifo.h"
#include "ffb_rcache.h"
#include "ffb.h"
#include "ffb_regs.h"

/* all driver need this */
#include "xf86.h"
#include "xf86_OSproc.h"
#include "compiler.h"
#include "exa.h"

extern void VISmoveImageRL(unsigned char *, unsigned char *, long, long, long, long);
extern void VISmoveImageLR(unsigned char *, unsigned char *, long, long, long, long);

/*#define FFB_DEBUG*/

#ifdef FFB_DEBUG
#define ENTER xf86Msg(X_ERROR, "%s>\n", __func__);
#define DPRINTF xf86Msg
#else
#define ENTER
#define DPRINTF while (0) xf86Msg
#endif

#define arraysize(ary)        (sizeof(ary) / sizeof(ary[0]))

int src_formats[] = {PICT_a8r8g8b8, PICT_x8r8g8b8,
		     PICT_a8b8g8r8, PICT_x8b8g8r8, PICT_a8};
int tex_formats[] = {PICT_a8r8g8b8, PICT_a8b8g8r8, PICT_a8};

static void
FFBWaitMarker(ScreenPtr pScreen, int Marker)
{
	ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
	FFBPtr pFfb = GET_FFB_FROM_SCRN(pScrn);
	ffb_fbcPtr ffb = pFfb->regs;

	FFBWait(pFfb, ffb);
}

static Bool
FFBPrepareCopy(PixmapPtr pSrcPixmap, PixmapPtr pDstPixmap,
		int xdir, int ydir, int alu, Pixel planemask)
{
	ScrnInfoPtr pScrn = xf86Screens[pDstPixmap->drawable.pScreen->myNum];
	FFBPtr pFfb = GET_FFB_FROM_SCRN(pScrn);
	ffb_fbcPtr ffb = pFfb->regs;

	ENTER;
	pFfb->srcpitch = exaGetPixmapPitch(pSrcPixmap);
	pFfb->srcoff = exaGetPixmapOffset(pSrcPixmap);
	pFfb->xdir = xdir;
	pFfb->ydir = ydir;
	pFfb->rop = alu;
	pFfb->planemask = planemask;
	return TRUE;
}

static void
FFBCopy(PixmapPtr pDstPixmap,
         int srcX, int srcY, int dstX, int dstY, int w, int h)
{
	ScrnInfoPtr pScrn = xf86Screens[pDstPixmap->drawable.pScreen->myNum];
	FFBPtr pFfb = GET_FFB_FROM_SCRN(pScrn);
	ffb_fbcPtr ffb = pFfb->regs;
	unsigned char *src, *dst, *sfb32;
	int psz_shift = 2;
	int sdkind;

	ENTER;
	if ((srcX == dstX) && (srcY != dstY) && (pFfb->rop == GXcopy)) {
		/* we can use the vscroll command */
		FFB_ATTR_VSCROLL_XAA(pFfb, pFfb->planemask);
		FFBFifo(pFfb, 7);
		ffb->drawop = FFB_DRAWOP_VSCROLL;
		FFB_WRITE64(&ffb->by, srcY, srcX);
                       FFB_WRITE64_2(&ffb->dy, dstY, dstX);
                       FFB_WRITE64_3(&ffb->bh, h, w);
		exaMarkSync(pDstPixmap->drawable.pScreen);
		return;
	}
	FFB_ATTR_SFB_VAR_XAA(pFfb, pFfb->planemask, pFfb->rop);
	if (pFfb->use_blkread_prefetch) {
		FFBFifo(pFfb, 1);
		if (pFfb->xdir < 0)
			ffb->mer = FFB_MER_EDRA;
		else
			ffb->mer = FFB_MER_EIRA;
	}
	FFBWait(pFfb, ffb);
	sfb32 = (unsigned char *) pFfb->sfb32;
	src = sfb32 + (srcY * (2048 << psz_shift)) + (srcX << psz_shift);
	dst = sfb32 + (dstY * (2048 << psz_shift)) + (dstX << psz_shift);
	sdkind = (2048 << psz_shift);

	if (pFfb->ydir < 0) {
		src += ((h - 1) * (2048 << psz_shift));
		dst += ((h - 1) * (2048 << psz_shift));
		sdkind = -sdkind;
	}
	w <<= psz_shift;
	if (pFfb->xdir < 0)
		VISmoveImageRL(src, dst, w, h, sdkind, sdkind);
	else
		VISmoveImageLR(src, dst, w, h, sdkind, sdkind);
	if (pFfb->use_blkread_prefetch) {
		FFBFifo(pFfb, 1);
		ffb->mer = FFB_MER_DRA;
	}
}

static void
FFBDoneCopy(PixmapPtr pDstPixmap)
{
}

static Bool
FFBPrepareSolid(PixmapPtr pPixmap, int alu, Pixel planemask, Pixel fg)
{
	ScrnInfoPtr pScrn = xf86Screens[pPixmap->drawable.pScreen->myNum];
	FFBPtr pFfb = GET_FFB_FROM_SCRN(pScrn);
	ffb_fbcPtr ffb = pFfb->regs;
	unsigned int ppc, ppc_mask, fbc;

	ENTER;
	FFBWait(pFfb, ffb);
	pFfb->planemask = planemask;
	pFfb->rop = alu;

	fbc = pFfb->fbc;
	if (pFfb->ffb_res == ffb_res_high)
		fbc |= FFB_FBC_WB_B;
	ppc = FFB_PPC_ABE_DISABLE | FFB_PPC_APE_DISABLE | FFB_PPC_CS_CONST | FFB_PPC_XS_WID;
	ppc_mask = FFB_PPC_ABE_MASK | FFB_PPC_APE_MASK | FFB_PPC_CS_MASK | FFB_PPC_XS_MASK;

	FFB_ATTR_RAW(pFfb, ppc, ppc_mask, planemask,
                    (FFB_ROP_EDIT_BIT | alu) | (FFB_ROP_NEW << 8),
                    FFB_DRAWOP_RECTANGLE, fg, fbc, pFfb->wid);

	return TRUE;
}

static void
FFBSolid(PixmapPtr pPixmap, int x1, int y1, int x2, int y2)
{
	ScrnInfoPtr pScrn = xf86Screens[pPixmap->drawable.pScreen->myNum];
	FFBPtr pFfb = GET_FFB_FROM_SCRN(pScrn);
	ffb_fbcPtr ffb = pFfb->regs;

	ENTER;
	FFBFifo(pFfb, 4);
	FFB_WRITE64(&ffb->by, y1, x1);
	FFB_WRITE64_2(&ffb->bh, y2 - y1, x2 - x1);
	exaMarkSync(pPixmap->drawable.pScreen);
}

static Bool
FFBUploadToScreen(PixmapPtr pDst, int x, int y, int w, int h,
    char *src, int src_pitch)
{
	ScrnInfoPtr pScrn = xf86Screens[pDst->drawable.pScreen->myNum];
	FFBPtr pFfb = GET_FFB_FROM_SCRN(pScrn);
	unsigned char *dst, *sfb32;
	int psz_shift = 2;
	ffb_fbcPtr ffb = pFfb->regs;

	ENTER;
	FFB_ATTR_SFB_VAR_XAA(pFfb, 0xffffffff, GXcopy);
	FFBWait(pFfb, ffb);

	sfb32 = (unsigned char *) pFfb->sfb32;
	dst = sfb32 + (y * (2048 << psz_shift)) + (x << psz_shift);
	VISmoveImageLR(src, dst, w << psz_shift, h,
		src_pitch, (2048 << psz_shift));
	return TRUE;
}

static Bool
FFBDownloadFromScreen(PixmapPtr pSrc, int x, int y, int w, int h,
    char *dst, int dst_pitch)
{
	ScrnInfoPtr pScrn = xf86Screens[pSrc->drawable.pScreen->myNum];
	FFBPtr pFfb = GET_FFB_FROM_SCRN(pScrn);
	unsigned char *src, *sfb32;
	int psz_shift = 2;
	ffb_fbcPtr ffb = pFfb->regs;

	ENTER;
	FFB_ATTR_SFB_VAR_XAA(pFfb, 0xffffffff, GXcopy);
	if (pFfb->use_blkread_prefetch) {
		FFBFifo(pFfb, 1);
		ffb->mer = FFB_MER_EIRA;
	}
	FFBWait(pFfb, ffb);

	sfb32 = (unsigned char *) pFfb->sfb32;
	src = sfb32 + (y * (2048 << psz_shift)) + (x << psz_shift);
	VISmoveImageLR(src, dst, w << psz_shift, h,
		(2048 << psz_shift), dst_pitch);
	if (pFfb->use_blkread_prefetch) {
		FFBFifo(pFfb, 1);
		ffb->mer = FFB_MER_DRA;
	}
	return TRUE;
}

static Bool
FFBCheckComposite(int op, PicturePtr pSrcPicture,
                           PicturePtr pMaskPicture,
                           PicturePtr pDstPicture)
{
	int i, ok = FALSE;

	ENTER;

	i = 0;
	while ((i < arraysize(src_formats)) && (!ok)) {
		ok =  (pSrcPicture->format == src_formats[i]);
		i++;
	}

	if (!ok) {
		xf86Msg(X_ERROR, "%s: unsupported src format %x\n",
		    __func__, pSrcPicture->format);
		return FALSE;
	}

	DPRINTF(X_ERROR, "src is %x, %d: %d %d\n", pSrcPicture->format, op,
	    pSrcPicture->pDrawable->width, pSrcPicture->pDrawable->height);

	if (pMaskPicture != NULL) {
		DPRINTF(X_ERROR, "mask is %x %d %d\n", pMaskPicture->format,
		    pMaskPicture->pDrawable->width,
		    pMaskPicture->pDrawable->height);
	}
	return TRUE;
}

static Bool
FFBPrepareComposite(int op, PicturePtr pSrcPicture,
                             PicturePtr pMaskPicture,
                             PicturePtr pDstPicture,
                             PixmapPtr  pSrc,
                             PixmapPtr  pMask,
                             PixmapPtr  pDst)
{
	ScrnInfoPtr pScrn = xf86Screens[pDst->drawable.pScreen->myNum];
	FFBPtr pFfb = GET_FFB_FROM_SCRN(pScrn);

	ENTER;

	pFfb->no_source_pixmap = FALSE;
	pFfb->source_is_solid = FALSE;

	if (pSrcPicture->format == PICT_a1) {
		xf86Msg(X_ERROR, "src mono, dst %x, op %d\n",
		    pDstPicture->format, op);
		if (pMaskPicture != NULL) {
			xf86Msg(X_ERROR, "msk %x\n", pMaskPicture->format);
		}
	}
	if (pSrcPicture->pSourcePict != NULL) {
		if (pSrcPicture->pSourcePict->type == SourcePictTypeSolidFill) {
			pFfb->fillcolour =
			    pSrcPicture->pSourcePict->solidFill.color;
			DPRINTF(X_ERROR, "%s: solid src %08x\n",
			    __func__, pFfb->fillcolour);
			pFfb->no_source_pixmap = TRUE;
			pFfb->source_is_solid = TRUE;
		}
	}
	if ((pMaskPicture != NULL) && (pMaskPicture->pSourcePict != NULL)) {
		if (pMaskPicture->pSourcePict->type ==
		    SourcePictTypeSolidFill) {
			pFfb->fillcolour = 
			   pMaskPicture->pSourcePict->solidFill.color;
			xf86Msg(X_ERROR, "%s: solid mask %08x\n",
			    __func__, pFfb->fillcolour);
		}
	}
	if (pMaskPicture != NULL) {
		pFfb->mskoff = exaGetPixmapOffset(pMask);
		pFfb->mskpitch = exaGetPixmapPitch(pMask);
		pFfb->mskformat = pMaskPicture->format;
	} else {
		pFfb->mskoff = 0;
		pFfb->mskpitch = 0;
		pFfb->mskformat = 0;
	}
	if (pSrc != NULL) {
		pFfb->source_is_solid = 
		   ((pSrc->drawable.width == 1) && (pSrc->drawable.height == 1));
		pFfb->srcoff = exaGetPixmapOffset(pSrc);
		pFfb->srcpitch = exaGetPixmapPitch(pSrc);
		if (pFfb->source_is_solid) {
			pFfb->fillcolour = *(uint32_t *)(pFfb->fb + pFfb->srcoff);
		}
	}
	pFfb->srcformat = pSrcPicture->format;
	pFfb->dstformat = pDstPicture->format;
	
	if (pFfb->source_is_solid) {
		uint32_t temp;

		/* swap solid source as needed */
		switch (pFfb->srcformat) {
			case PICT_a8r8g8b8:
			case PICT_x8r8g8b8:
				temp = (pFfb->fillcolour & 0x000000ff) << 16;
				temp |= (pFfb->fillcolour & 0x00ff0000) >> 16;
				temp |= (pFfb->fillcolour & 0xff00ff00);
				pFfb->fillcolour = temp;
				break;
		}
	}
	pFfb->op = op;
	return TRUE;
}

static void
FFBComposite(PixmapPtr pDst, int srcX, int srcY,
                              int maskX, int maskY,
                              int dstX, int dstY,
                              int width, int height)
{
	ScrnInfoPtr pScrn = xf86Screens[pDst->drawable.pScreen->myNum];
	FFBPtr pFfb = GET_FFB_FROM_SCRN(pScrn);
	exaMarkSync(pDst->drawable.pScreen);
}

static Bool
FFBPrepareAccess(PixmapPtr pPix, int index)
{
	ScrnInfoPtr pScrn = xf86Screens[pPix->drawable.pScreen->myNum];
	FFBPtr pFfb = GET_FFB_FROM_SCRN(pScrn);
	ffb_fbcPtr ffb = pFfb->regs;

	ENTER;
	FFB_ATTR_SFB_VAR_XAA(pFfb, 0xffffffff, GXcopy);
	FFBWait(pFfb, ffb);

	return TRUE;	
}

static void
FFBFinishAccess(PixmapPtr pPix, int index)
{
}

Bool
FFBInitEXA(ScreenPtr pScreen)
{
	ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
	FFBPtr pFfb = GET_FFB_FROM_SCRN(pScrn);
	ffb_fbcPtr ffb = pFfb->regs;
	ExaDriverPtr pExa;

	pFfb->fbc = (FFB_FBC_WB_A | FFB_FBC_WM_COMBINED | FFB_FBC_RB_A |
			 FFB_FBC_WE_FORCEON |
			 FFB_FBC_SB_BOTH |
			 FFB_FBC_ZE_OFF | FFB_FBC_YE_OFF |
			 FFB_FBC_RGBE_MASK |
			 FFB_FBC_XE_ON);
	pFfb->wid = FFBWidAlloc(pFfb, TrueColor, 0, TRUE);
	if (pFfb->wid == (unsigned int) -1)
		return FALSE;

	pFfb->ppc_cache = (FFB_PPC_FW_DISABLE |
			   FFB_PPC_VCE_DISABLE | FFB_PPC_APE_DISABLE | FFB_PPC_CS_CONST |
			   FFB_PPC_XS_WID | FFB_PPC_YS_CONST | FFB_PPC_ZS_CONST |
			   FFB_PPC_DCE_DISABLE | FFB_PPC_ABE_DISABLE | FFB_PPC_TBE_OPAQUE);
	pFfb->wid_cache = pFfb->wid;
	pFfb->pmask_cache = ~0;
	pFfb->rop_cache = (FFB_ROP_NEW | (FFB_ROP_NEW << 8));
	pFfb->drawop_cache = FFB_DRAWOP_RECTANGLE;
	pFfb->fg_cache = pFfb->bg_cache = 0;
	pFfb->fontw_cache = 32;
	pFfb->fontinc_cache = (1 << 16) | 0;
	pFfb->fbc_cache = (FFB_FBC_WB_A | FFB_FBC_WM_COMBINED | FFB_FBC_RB_A |
			   FFB_FBC_WE_FORCEON |
			   FFB_FBC_SB_BOTH |
			   FFB_FBC_ZE_OFF | FFB_FBC_YE_OFF |
			   FFB_FBC_RGBE_OFF |
			   FFB_FBC_XE_ON);

	/* We will now clear the screen: we'll draw a rectangle covering all the
	 * viewscreen, using a 'blackness' ROP.
	 */
	FFBFifo(pFfb, 22);
	ffb->fbc = pFfb->fbc_cache;
	ffb->ppc = pFfb->ppc_cache;
	ffb->wid = pFfb->wid_cache;
	ffb->xpmask = 0xff;
	ffb->pmask = pFfb->pmask_cache;
	ffb->rop = pFfb->rop_cache;
	ffb->drawop = pFfb->drawop_cache;
	ffb->fg = pFfb->fg_cache;
	ffb->bg = pFfb->bg_cache;
	ffb->fontw = pFfb->fontw_cache;
	ffb->fontinc = pFfb->fontinc_cache;
	ffb->xclip = FFB_XCLIP_TEST_ALWAYS;
	ffb->cmp = 0x80808080;
	ffb->matchab = 0x80808080;
	ffb->magnab = 0x80808080;
	ffb->blendc = (FFB_BLENDC_FORCE_ONE |
		       FFB_BLENDC_DF_ONE_M_A |
		       FFB_BLENDC_SF_A);
	ffb->blendc1 = 0;
	ffb->blendc2 = 0;
	FFB_WRITE64(&ffb->by, 0, 0);
	FFB_WRITE64_2(&ffb->bh, pFfb->psdp->height, pFfb->psdp->width);
	FFBWait(pFfb, ffb);
	
	FFB_ATTR_SFB_VAR_XAA(pFfb, 0xffffffff, GXcopy);
	FFBWait(pFfb, ffb);

	FFB_HardwareSetup(pFfb);
	pExa = exaDriverAlloc();
	if (!pExa)
		return FALSE;

	pFfb->pExa = pExa;

	pExa->exa_major = EXA_VERSION_MAJOR;
	pExa->exa_minor = EXA_VERSION_MINOR;


	pExa->memoryBase = (char *)pFfb->sfb32;
	/*
	 * we don't have usable off-screen memory but EXA craps out if we don't
	 * pretend that we do, so register a ridiculously small amount and
	 * cross fingers
	 */
	pExa->memorySize = 8192 * pFfb->psdp->height + 4;
	pExa->offScreenBase = pExa->memorySize - 4;

	/* we want to use 64bit aligned accesses */	
	pExa->pixmapOffsetAlign = 8;
	pExa->pixmapPitchAlign = 8;

	pExa->flags = EXA_OFFSCREEN_PIXMAPS |
		      /*EXA_SUPPORTS_OFFSCREEN_OVERLAPS |*/
		      EXA_MIXED_PIXMAPS;

	pExa->maxX = 2048;
	pExa->maxY = 2048;

	pExa->WaitMarker = FFBWaitMarker;

	pExa->PrepareSolid = FFBPrepareSolid;
	pExa->Solid = FFBSolid;
	pExa->DoneSolid = FFBDoneCopy;

	pExa->PrepareCopy = FFBPrepareCopy;
	pExa->Copy = FFBCopy;
	pExa->DoneCopy = FFBDoneCopy;

	pExa->UploadToScreen = FFBUploadToScreen;
	pExa->DownloadFromScreen = FFBDownloadFromScreen;

	pExa->PrepareAccess = FFBPrepareAccess;
	pExa->FinishAccess = FFBFinishAccess;

if(0) {
	pExa->CheckComposite = FFBCheckComposite;
	pExa->PrepareComposite = FFBPrepareComposite;
	pExa->Composite = FFBComposite;
	pExa->DoneComposite = FFBDoneCopy;
}
	return exaDriverInit(pScreen, pExa);
}

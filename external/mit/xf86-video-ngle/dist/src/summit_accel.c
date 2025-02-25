/*
 * hardware acceleration for Visualize FX 4
 *
 * Copyright (C) 2024 Michael Lorenz
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * MICHAEL LORENZ BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/* $NetBSD: summit_accel.c,v 1.9 2025/02/25 13:21:24 macallan Exp $ */

#include <sys/types.h>
#include <dev/ic/summitreg.h>


#include "ngle.h"
#include "mipict.h"

//#define DEBUG

void
 exaPrepareAccess(DrawablePtr pDrawable, int index);

void
 exaFinishAccess(DrawablePtr pDrawable, int index);

#ifdef DEBUG
#define ENTER xf86Msg(X_ERROR, "%s\n", __func__)
#define LEAVE xf86Msg(X_ERROR, "%s done\n", __func__)
#define DBGMSG xf86Msg
#else
#define ENTER
#define DBGMSG if (0) xf86Msg
#define LEAVE
#endif

#define SUMMIT_READ_MODE(m) \
	if ((m) != fPtr->read_mode) { \
		SummitWait(fPtr); \
		NGLEWrite4(fPtr, VISFX_VRAM_READ_MODE, (m)); \
		fPtr->read_mode = (m); \
	}

#define SUMMIT_WRITE_MODE(m) \
	if ((m) != fPtr->write_mode) { \
		SummitWait(fPtr); \
		NGLEWrite4(fPtr, VISFX_VRAM_WRITE_MODE, (m)); \
		fPtr->write_mode = (m); \
	}

static inline void
SummitWait(NGLEPtr fPtr)
{
	int reg, count = 0;;

	ENTER;
	do {
		reg = NGLERead4(fPtr, VISFX_STATUS);
		count++;
	} while ((reg & 0x01000000) != 0);
	if (reg != 0) {
		xf86Msg(X_ERROR, "%s status %08x\n", __func__, reg);
		xf86Msg(X_ERROR, "fault %08x\n", NGLERead4(fPtr, 0x641040));
	}
	DBGMSG(X_ERROR, "%s: %d\n", __func__, count);
}
	
static void
SummitWaitMarker(ScreenPtr pScreen, int Marker)
{
	ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
	NGLEPtr fPtr = NGLEPTR(pScrn);
	int reg, count = 0;

	ENTER;
	do {
		reg = NGLERead4(fPtr, VISFX_STATUS);
		count++;
	} while ((reg & 0x01000000) != 0);
	if (reg != 0) {
		xf86Msg(X_ERROR, "%s status %08x\n", __func__, reg);
		xf86Msg(X_ERROR, "fault %08x\n", NGLERead4(fPtr, 0x641040));
	}
	DBGMSG(X_ERROR, "%s: %d\n", __func__, count);
}

static void
SummitWaitFifo(NGLEPtr fPtr, int count)
{
	int reg;
	do {
		reg = NGLERead4(fPtr, VISFX_FIFO);
	} while (reg < count);
#ifdef DEBUG
	if (reg != 0x800) xf86Msg(X_ERROR, "%s %x\n", __func__, reg);
#endif
}

static Bool
SummitPrepareCopy
(
    PixmapPtr pSrcPixmap,
    PixmapPtr pDstPixmap,
    int       xdir,
    int       ydir,
    int       alu,
    Pixel     planemask
)
{
	ScrnInfoPtr pScrn = xf86Screens[pDstPixmap->drawable.pScreen->myNum];
	NGLEPtr fPtr = NGLEPTR(pScrn);
	int dstoff = exaGetPixmapOffset(pDstPixmap);
	int srcoff = exaGetPixmapOffset(pSrcPixmap);
	uint32_t sm, dm;
	int y;

	ENTER;

	sm = dm = OTC01 | BIN8F | BUFFL;
	DBGMSG(X_ERROR, "%s %d %d\n", __func__, srcoff, dstoff);

	y = (srcoff >> 13);	/* pitch is 8192 bytes in 24 bit */
	if (y >= fPtr->fbi.fbi_height) {
		sm = OTC01 | BIN8F | BUFBL;
		y -= fPtr->fbi.fbi_height;
	}
	fPtr->offset = y;
	SUMMIT_READ_MODE(sm);

	y = (dstoff >> 13);	/* pitch is 8192 bytes in 24 bit */
	if (y >= fPtr->fbi.fbi_height) {
		dm = OTC01 | BIN8F | BUFBL;
		y -= fPtr->fbi.fbi_height;
	}
	fPtr->offsetd = y;
	SUMMIT_WRITE_MODE(dm);

	SummitWaitFifo(fPtr, 8);
	if (alu == GXcopy) {
		NGLEWrite4(fPtr, VISFX_FOE, 0);
	} else {
		NGLEWrite4(fPtr, VISFX_FOE, FOE_BLEND_ROP);
		NGLEWrite4(fPtr, VISFX_IBO, alu);
	}
	NGLEWrite4(fPtr, VISFX_PLANE_MASK, planemask);
	LEAVE;
	return TRUE;
}

static void
SummitCopy
(
    PixmapPtr pDstPixmap,
    int       xs,
    int       ys,
    int       xd,
    int       yd,
    int       wi,
    int       he
)
{
	ScrnInfoPtr pScrn = xf86Screens[pDstPixmap->drawable.pScreen->myNum];
	NGLEPtr fPtr = NGLEPTR(pScrn);

	ENTER;
	SummitWaitFifo(fPtr, 8);
	NGLEWrite4(fPtr, VISFX_COPY_SRC, (xs << 16) | (ys + fPtr->offset));
	NGLEWrite4(fPtr, VISFX_COPY_WH, (wi << 16) | he);
	NGLEWrite4(fPtr, VISFX_COPY_DST, (xd << 16) | (yd + fPtr->offsetd));

	LEAVE;
}

static void
SummitDoneCopy(PixmapPtr pDstPixmap)
{
    ENTER;
    LEAVE;
}

static Bool
SummitPrepareSolid(
    PixmapPtr pPixmap,
    int alu,
    Pixel planemask,
    Pixel fg)
{
	ScrnInfoPtr pScrn = xf86Screens[pPixmap->drawable.pScreen->myNum];
	NGLEPtr fPtr = NGLEPTR(pScrn);
	int	ofs =  exaGetPixmapOffset(pPixmap);
	int	y;
	uint32_t wm = OTC32 | BIN8F | BUFFL | 0x8c0, rm = OTC01 | BIN8F | BUFFL;

	ENTER;
	y = (ofs >> 13);	/* pitch is 8192 bytes in 24 bit */
	if (y >= fPtr->fbi.fbi_height) {
		wm = OTC32 | BIN8F | BUFBL | 0x8c0;
		rm = OTC01 | BIN8F | BUFBL;
		y -= fPtr->fbi.fbi_height;
	}
	SUMMIT_READ_MODE(rm);
	SUMMIT_WRITE_MODE(wm);
	fPtr->offset = y;
	SummitWaitFifo(fPtr, 10);		
	if (alu == GXcopy) {
		NGLEWrite4(fPtr, VISFX_FOE, 0);
	} else {
		NGLEWrite4(fPtr, VISFX_FOE, FOE_BLEND_ROP);
		NGLEWrite4(fPtr, VISFX_IBO, alu);
	}
	NGLEWrite4(fPtr, VISFX_FG_COLOUR, fg);
	NGLEWrite4(fPtr, VISFX_PLANE_MASK, planemask);
	LEAVE;
	return TRUE;
}

static void
SummitSolid(
    PixmapPtr pPixmap,
    int x1,
    int y1,
    int x2,
    int y2)
{
	ScrnInfoPtr pScrn = xf86Screens[pPixmap->drawable.pScreen->myNum];
	NGLEPtr fPtr = NGLEPTR(pScrn);
	int wi = x2 - x1, he = y2 - y1;

	ENTER;
	
	y1 += fPtr->offset;
	
	SummitWaitFifo(fPtr, 6);		
	NGLEWrite4(fPtr, VISFX_START, (x1 << 16) | y1);
	NGLEWrite4(fPtr, VISFX_SIZE, (wi << 16) | he);

	LEAVE;
}

static Bool
SummitUploadToScreen(PixmapPtr pDst, int x, int y, int w, int h,
    char *src, int src_pitch)
{
	ScrnInfoPtr pScrn = xf86Screens[pDst->drawable.pScreen->myNum];
	NGLEPtr fPtr = NGLEPTR(pScrn);
	int	ofs =  exaGetPixmapOffset(pDst);
	int i;
	uint32_t *line, mode = OTC01 | BIN8F | BUFFL;
	uint8_t *dst;

	ENTER;
	y += (ofs >> 13);	/* pitch is 8192 bytes in 24 bit */
	if (y >= fPtr->fbi.fbi_height) {
		mode = OTC01 | BIN8F | BUFBL;
		y -= fPtr->fbi.fbi_height;
	}

	dst = fPtr->fbmem;
	dst += (y << 13) + (x << 2);

	SUMMIT_WRITE_MODE(mode);
	NGLEWrite4(fPtr, VISFX_PLANE_MASK, 0xffffffff);
	NGLEWrite4(fPtr, VISFX_FOE, 0);
	
	while (h--) {
		/*
		 * it *should* be impossible to overrun the FIFO using BINC
		 * writes, but overruns are annoying if they do happen so be
		 * overly cautious and make sure there is at least some room
		 */
		SummitWaitFifo(fPtr, w + 1);
		NGLEWrite4(fPtr, VISFX_VRAM_WRITE_DEST, (y << 16) | x);
		line = (uint32_t *)src;

		for (i = 0; i < w; i++)
			NGLEWrite4(fPtr, VISFX_VRAM_WRITE_DATA_INCRX, line[i]);
		//memcpy(dst, src, w << 2);
		src += src_pitch;
		dst += 8192;
		y++;
	}

	LEAVE;

	return TRUE;
}

static Bool
SummitDownloadFromScreen(PixmapPtr pSrc, int x, int y, int w, int h,
    char *dst, int dst_pitch)
{
	ScrnInfoPtr pScrn = xf86Screens[pSrc->drawable.pScreen->myNum];
	NGLEPtr fPtr = NGLEPTR(pScrn);
	uint8_t *src;
	int	ofs =  exaGetPixmapOffset(pSrc);
	uint32_t mode = OTC01 | BIN8F | BUFFL;

	ENTER;

	y += (ofs >> 13);
	if (y >= fPtr->fbi.fbi_height) {
		mode = OTC01 | BIN8F | BUFBL;
		y -= fPtr->fbi.fbi_height;
	}
	SUMMIT_READ_MODE(mode);
	SummitWait(fPtr);
	NGLEWrite4(fPtr, VISFX_RPH, VISFX_RPH_LTR);
	SummitWait(fPtr);

	src = fPtr->fbmem;
	src += (y << 13) + (x << 2);

	while (h--) {
		memcpy(dst, src, w << 2);
		src += 8192;
		dst += dst_pitch;
	}

	LEAVE;

	return TRUE;
}

Bool
SummitPrepareAccess(PixmapPtr pPixmap, int index)
{
	ScrnInfoPtr pScrn = xf86Screens[pPixmap->drawable.pScreen->myNum];
	NGLEPtr fPtr = NGLEPTR(pScrn);
	int	ofs =  exaGetPixmapOffset(pPixmap);
	int	y;

	ENTER;
	//xf86Msg(X_ERROR, "%s %d\n", __func__, ofs);
	if (ofs == 0) {
		/* accessing the visible framebuffer */
		SUMMIT_READ_MODE(OTC01 | BIN8F | BUFFL);
		SUMMIT_WRITE_MODE(OTC01 | BIN8F | BUFFL);
	} else {
		SUMMIT_READ_MODE(OTC01 | BIN8F | BUFBL);
		SUMMIT_WRITE_MODE(OTC01 | BIN8F | BUFBL);
		y = ofs >> 13;
		y -= fPtr->fbi.fbi_height;
		pPixmap->devPrivate.ptr = fPtr->fbmem + (y << 13);
	}
	NGLEWrite4(fPtr, VISFX_FOE, 0);
	NGLEWrite4(fPtr, VISFX_RPH, VISFX_RPH_LTR);
	//NGLEWrite4(fPtr, VISFX_CONTROL, 0x200);
	SummitWait(fPtr);
	LEAVE;
	return TRUE;
}

void
SummitFinishAccess(PixmapPtr pPixmap, int index)
{
	ScrnInfoPtr pScrn = xf86Screens[pPixmap->drawable.pScreen->myNum];
	NGLEPtr fPtr = NGLEPTR(pScrn);

	ENTER;
	//NGLEWrite4(fPtr, VISFX_CONTROL, 0);
	//SummitWait(fPtr);
	LEAVE;
}

PixmapPtr
SummitGetDrawablePixmap(DrawablePtr pDrawable)
{
    if (pDrawable->type == DRAWABLE_WINDOW)
        return pDrawable->pScreen->GetWindowPixmap((WindowPtr) pDrawable);
    else
        return (PixmapPtr) pDrawable;
}

static void
SummitDrawGlyph8(NGLEPtr fPtr, int32_t fg, PixmapPtr mask, int p,
    int xm, int ym, int xd, int yd, int w, int h)
{
	uint8_t *gdata = mask->devPrivate.ptr;
	uint32_t msk;
	int i, j, needs_coords;

	gdata += xm;
	gdata += (p * ym);
	for (i = 0; i < h; i++) {
		SummitWaitFifo(fPtr, w * 2);
		needs_coords = 1;
		for (j = 0; j < w; j++) {
			msk = gdata[j];
			if (msk == 0) {
				needs_coords = 1;
				continue;
			}
			if (needs_coords) {
				NGLEWrite4(fPtr, VISFX_VRAM_WRITE_DEST, 
				    ((yd + i) << 16) | (xd + j));
				needs_coords = 0;
			}
			msk = (msk << 24) | fg;
			NGLEWrite4(fPtr,
			    VISFX_VRAM_WRITE_DATA_INCRX, msk);	
		}
		gdata += p;
	}
}

static void
SummitDrawGlyph32(NGLEPtr fPtr, uint32_t fg, PixmapPtr mask, int p,
    int xm, int ym, int xd, int yd, int w, int h)
{
	uint32_t *gdata = mask->devPrivate.ptr;
	uint32_t msk;
	int i, j, needs_coords;

	gdata += xm;
	gdata += (p * ym);

	for (i = 0; i < h; i++) {
		SummitWaitFifo(fPtr, w * 2);
		needs_coords = 1;
		for (j = 0; j < w; j++) {
			msk = gdata[j] & 0xff000000;
			if (msk == 0) {
				needs_coords = 1;
				continue;
			}
			if (needs_coords) {
				NGLEWrite4(fPtr, VISFX_VRAM_WRITE_DEST, 
				    ((yd + i) << 16) | (xd + j));
				needs_coords = 0;
			}
			msk |= fg;
			NGLEWrite4(fPtr,
			    VISFX_VRAM_WRITE_DATA_INCRX, msk);	
		}
		gdata += p >> 2;
	}
}

static void
SummitGlyphs (CARD8	op,
	  PicturePtr	pSrc,
	  PicturePtr	pDst,
	  PictFormatPtr	maskFormat,
	  INT16		xSrc,
	  INT16		ySrc,
	  int		nlist,
	  GlyphListPtr	list,
	  GlyphPtr	*glyphs)
{
	ScreenPtr	pScreen = pDst->pDrawable->pScreen;
	ScrnInfoPtr 	pScrn = xf86Screens[pScreen->myNum];
	NGLEPtr 	fPtr = NGLEPTR(pScrn);
	PicturePtr	pPicture;
	PixmapPtr	mask, dst;
	GlyphPtr	glyph;
	int		xDst = list->xOff, yDst = list->yOff;
	int		x = 0, y = 0, i, n, ofs, p, j, wi, he, xs, ys;
	int		dw, dh;
	uint32_t fg = 0xffffffff, msk;

	if (op != PictOpOver) goto fallback;

	if (!exaDrawableIsOffscreen(pDst->pDrawable)) goto fallback;

	dst = SummitGetDrawablePixmap(pDst->pDrawable);
	ofs = exaGetPixmapOffset(dst);
	ofs = ofs >> 13;
	dw = pDst->pDrawable->width;
	dh = pDst->pDrawable->height;

	if (pDst->pDrawable->type == DRAWABLE_WINDOW) {
		x += pDst->pDrawable->x;
		y += pDst->pDrawable->y;
	}

	if (pSrc->pSourcePict != NULL) {
		if (pSrc->pSourcePict->type == SourcePictTypeSolidFill) {
			fg = pSrc->pSourcePict->solidFill.color;
		}
	}
	fg &= 0x00ffffff;

	if (ofs == 0) {
		/* accessing the visible framebuffer */
		SUMMIT_WRITE_MODE(OTC01 | BIN8F | BUFFL);
	} else {
		SUMMIT_WRITE_MODE(OTC01 | BIN8F | BUFBL);
	}

	SummitWaitFifo(fPtr, 4);
	NGLEWrite4(fPtr, VISFX_FOE, FOE_BLEND_ROP);
	NGLEWrite4(fPtr, VISFX_IBO,
	    IBO_ADD | SRC(IBO_SRC) | DST(IBO_ONE_MINUS_SRC));

	while (nlist--)     {
		x += list->xOff;
		y += list->yOff;
		n = list->len;
		while (n--) {
			glyph = *glyphs++;
			pPicture = GlyphPicture (glyph)[pScreen->myNum];
			if (pPicture) {
				int xd = x - glyph->info.x;
				int yd = y - glyph->info.y;
				RegionRec region;
				BoxPtr pbox;
				int nbox;

				if (ofs == 0) {
					/*
					 * we're drawing to the visible screen,
					 * so we must take care not to scribble
					 * over other windows
					 */
					if (!miComputeCompositeRegion(&region,
					  pSrc, pPicture, pDst,
                        	          0, 0, 0, 0, xd, yd,
                        	          glyph->info.width,
                        	          glyph->info.height))
						goto skip;

					fbGetDrawablePixmap(pPicture->pDrawable,
					    mask, wi, he);
					exaPrepareAccess(pPicture->pDrawable,
					    EXA_PREPARE_SRC);
					p = exaGetPixmapPitch(mask);

					nbox = RegionNumRects(&region);
					pbox = RegionRects(&region);
					while (nbox--) {
						if (pPicture->format == PICT_a8) {
							SummitDrawGlyph8(fPtr,
							    fg, mask, p,
							    pbox->x1 - xd,
							    pbox->y1 - yd,
							    pbox->x1, pbox->y1,
							    pbox->x2 - pbox->x1,
							    pbox->y2 - pbox->y1);
						} else {
							SummitDrawGlyph32(fPtr,
							    fg, mask, p,
							    pbox->x1 - xd,
							    pbox->y1 - yd,
							    pbox->x1, pbox->y1,
							    pbox->x2 - pbox->x1,
							    pbox->y2 - pbox->y1);
						}
        					pbox++;
    					}
					RegionUninit(&region);
					exaFinishAccess(pPicture->pDrawable,
					    EXA_PREPARE_SRC);
				} else {
					/*
					 * drawing into off-screen memory, we
					 * only need to clip to the destination
					 * pixmap's boundaries
					 */

					fbGetDrawablePixmap(pPicture->pDrawable,
					    mask, wi, he);

					xs = 0;
					ys = 0;
					wi = glyph->info.width;
					he = glyph->info.height;

					if (xd < 0) {
						xs -= xd;
						wi += xd;
						xd = 0;
					}

					if (yd < 0) {
						ys -= yd;
						he += yd;
						yd = 0;
					}

					if ((xd + wi) > dw) {
						wi -= (xd + wi - dw);
					}

					if ((yd + he) > dh) {
						he -= (yd + he - dh);
					}

					if ((he <= 0) || (wi <= 0))
						goto skip;

					yd += (ofs - fPtr->fbi.fbi_height);

					exaPrepareAccess(pPicture->pDrawable,
					    EXA_PREPARE_SRC);
					p = exaGetPixmapPitch(mask);

					if (pPicture->format == PICT_a8) {
						SummitDrawGlyph8(fPtr,
						    fg, mask, p,
						    xs, ys,
						    xd, yd, 
						    wi, he);
					} else {
						SummitDrawGlyph32(fPtr,
						    fg, mask, p,
						    xs, ys,
						    xd, yd, 
						    wi, he);
					}
					exaFinishAccess(pPicture->pDrawable,
					    EXA_PREPARE_SRC);
				}
			}
skip:
			x += glyph->info.xOff;
			y += glyph->info.yOff;
		}
		list++;
	}
	return;	
fallback:
	fPtr->glyphs(op, pSrc, pDst, maskFormat, xSrc, ySrc, nlist, list, glyphs);
}

Bool
SummitInitAccel(ScreenPtr pScreen)
{
	ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
	PictureScreenPtr ps = GetPictureScreen(pScreen);
	NGLEPtr fPtr = NGLEPTR(pScrn);
	ExaDriverPtr pExa;
	int bpp = pScrn->bitsPerPixel >> 3, ret;

	pExa = exaDriverAlloc();
	if (!pExa)
		return FALSE;

	fPtr->pExa = pExa;

	pExa->exa_major = EXA_VERSION_MAJOR;
	pExa->exa_minor = EXA_VERSION_MINOR;

	pExa->memoryBase = fPtr->fbmem;
	pExa->memorySize = fPtr->fbi.fbi_stride * (fPtr->fbi.fbi_height * 2);
	pExa->offScreenBase = fPtr->fbi.fbi_stride * fPtr->fbi.fbi_height;
	pExa->pixmapOffsetAlign = fPtr->fbi.fbi_stride;
	pExa->pixmapPitchAlign = fPtr->fbi.fbi_stride;

	pExa->flags = EXA_OFFSCREEN_PIXMAPS | EXA_MIXED_PIXMAPS;

	pExa->maxX = 2048;
	pExa->maxY = 2048;	

	pExa->WaitMarker = SummitWaitMarker;
	pExa->Solid = SummitSolid;
	pExa->DoneSolid = SummitDoneCopy;
	pExa->Copy = SummitCopy;
	pExa->DoneCopy = SummitDoneCopy;
	pExa->PrepareCopy = SummitPrepareCopy;
	pExa->PrepareSolid = SummitPrepareSolid;
	pExa->UploadToScreen = SummitUploadToScreen;
	pExa->DownloadFromScreen = SummitDownloadFromScreen;
	pExa->PrepareAccess = SummitPrepareAccess;
	pExa->FinishAccess = SummitFinishAccess;

	fPtr->read_mode = -1;
	fPtr->write_mode = -1;
	SUMMIT_READ_MODE(OTC01 | BIN8F | BUFFL);
	SUMMIT_WRITE_MODE(OTC01 | BIN8F | BUFFL);
	NGLEWrite4(fPtr, VISFX_FOE, FOE_BLEND_ROP);
	NGLEWrite4(fPtr, VISFX_IBO, GXcopy);
	NGLEWrite4(fPtr, VISFX_CONTROL, 0);

	ret = exaDriverInit(pScreen, pExa);

	fPtr->glyphs = ps->Glyphs;
	ps->Glyphs = SummitGlyphs;
	return ret;
}

/*
 * crude EXA support for geforce chips
 *
 * Copyright (C) 2018 Michael Lorenz
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

/* $NetBSD: nv_exa.c,v 1.3 2018/07/16 16:30:04 macallan Exp $ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "nv_include.h"
#include "miline.h"
#include "nv_dma.h"
#include "exa.h"

//#define DEBUG

#ifdef DEBUG
#define ENTER xf86Msg(X_ERROR, "%s\n", __func__)
#define LEAVE xf86Msg(X_ERROR, "%s done\n", __func__)
#else
#define ENTER
#define LEAVE
#endif

static void
NvWaitMarker(ScreenPtr pScreen, int Marker)
{
	ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];

	ENTER;
	NVSync(pScrn);
	LEAVE;
}

static Bool
NvPrepareCopy
(
    PixmapPtr pSrcPixmap,
    PixmapPtr pDstPixmap,
    int       xdir,
    int       ydir,
    int       rop,
    Pixel     planemask
)
{
	ScrnInfoPtr pScrn = xf86Screens[pDstPixmap->drawable.pScreen->myNum];
	NVPtr pNv = NVPTR(pScrn);
	int srcpitch, srcoff, dstpitch, dstoff;

	ENTER;
	if (pSrcPixmap->drawable.bitsPerPixel != 32)
		xf86Msg(X_ERROR, "%s %d bpp\n", __func__, pSrcPixmap->drawable.bitsPerPixel);
	planemask |= ~0 << pNv->CurrentLayout.depth;
	NVSetRopSolid(pScrn, rop, planemask);
	pNv->DMAKickoffCallback = NVDMAKickoffCallback;
	dstpitch = exaGetPixmapPitch(pDstPixmap);
	dstoff = exaGetPixmapOffset(pDstPixmap);
	srcpitch = exaGetPixmapPitch(pSrcPixmap);
	srcoff = exaGetPixmapOffset(pSrcPixmap);

	NVDmaStart(pNv, SURFACE_PITCH, 3);
	NVDmaNext (pNv, srcpitch | (dstpitch << 16));
	NVDmaNext (pNv, srcoff);
	NVDmaNext (pNv, dstoff);

	LEAVE;
	return TRUE;
}

static void
NvCopy
(
    PixmapPtr pDstPixmap,
    int       srcX,
    int       srcY,
    int       dstX,
    int       dstY,
    int       w,
    int       h
)
{
	ScrnInfoPtr pScrn = xf86Screens[pDstPixmap->drawable.pScreen->myNum];
	NVPtr pNv = NVPTR(pScrn);

	NVDmaStart(pNv, BLIT_POINT_SRC, 3);
	NVDmaNext (pNv, (srcY << 16) | srcX);
	NVDmaNext (pNv, (dstY << 16) | dstX);
	NVDmaNext (pNv, (h  << 16) | w);

	if((w * h) >= 512)
		NVDmaKickoff(pNv); 

	LEAVE;
}

static void
NvDoneCopy(PixmapPtr pDstPixmap)
{
    ENTER;
    LEAVE;
}

static Bool
NvPrepareSolid(
    PixmapPtr pPixmap,
    int rop,
    Pixel planemask,
    Pixel color)
{
	ScrnInfoPtr pScrn = xf86Screens[pPixmap->drawable.pScreen->myNum];
	NVPtr pNv = NVPTR(pScrn);
	int pitch, off;

	ENTER;
	if (pPixmap->drawable.bitsPerPixel != 32)
		xf86Msg(X_ERROR, "%s %d bpp\n", __func__, pPixmap->drawable.bitsPerPixel);
	planemask |= ~0 << pNv->CurrentLayout.depth;


	pitch = exaGetPixmapPitch(pPixmap);
	off = exaGetPixmapOffset(pPixmap);

	NVDmaStart(pNv, SURFACE_PITCH, 3);
	NVDmaNext (pNv, pitch | (pitch << 16));
	NVDmaNext (pNv, off);
	NVDmaNext (pNv, off);

	NVSetRopSolid(pScrn, rop, planemask);
	pNv->DMAKickoffCallback = NVDMAKickoffCallback;
	NVDmaStart(pNv, RECT_SOLID_COLOR, 1);
	NVDmaNext (pNv, color);

	LEAVE;
	return TRUE;
}

static void
NvSolid(
    PixmapPtr pPixmap,
    int x1,
    int y1,
    int x2,
    int y2)
{
	ScrnInfoPtr pScrn = xf86Screens[pPixmap->drawable.pScreen->myNum];
	NVPtr pNv = NVPTR(pScrn);
	int w = x2 - x1, h = y2 - y1;

	ENTER;
	NVDmaStart(pNv, RECT_SOLID_RECTS(0), 2);
	NVDmaNext (pNv, (x1 << 16) | y1);
	NVDmaNext (pNv, (w << 16) | h);

	if((w * h) >= 512)
		NVDmaKickoff(pNv);

	LEAVE;
}

/*
 * Memcpy-based UTS.
 */
static Bool
NvUploadToScreen(PixmapPtr pDst, int x, int y, int w, int h,
    char *src, int src_pitch)
{
	ScrnInfoPtr pScrn = xf86Screens[pDst->drawable.pScreen->myNum];
	NVPtr pNv = NVPTR(pScrn);
	unsigned char *dst = pNv->FbStart + exaGetPixmapOffset(pDst);
	int dst_pitch = exaGetPixmapPitch(pDst);

	int bpp    = pDst->drawable.bitsPerPixel;
	int cpp    = (bpp + 7) >> 3;
	int wBytes = w * cpp;

	ENTER;
	dst += (x * cpp) + (y * dst_pitch);

	NVSync(pScrn);

	while (h--) {
		memcpy(dst, src, wBytes);
		src += src_pitch;
		dst += dst_pitch;
	}
	_NV_FENCE()
	LEAVE;
	return TRUE;
}

/*
 * Memcpy-based DFS.
 */
static Bool
NvDownloadFromScreen(PixmapPtr pSrc, int x, int y, int w, int h,
    char *dst, int dst_pitch)
{
	ScrnInfoPtr pScrn = xf86Screens[pSrc->drawable.pScreen->myNum];
	NVPtr pNv = NVPTR(pScrn);
	unsigned char *src = pNv->FbStart + exaGetPixmapOffset(pSrc);
	int src_pitch = exaGetPixmapPitch(pSrc);

	int bpp    = pSrc->drawable.bitsPerPixel;
	int cpp    = (bpp + 7) >> 3;
	int wBytes = w * cpp;

	ENTER;
	src += (x * cpp) + (y * src_pitch);

	NVSync(pScrn);

	while (h--) {
		memcpy(dst, src, wBytes);
		src += src_pitch;
		dst += dst_pitch;
	}
	LEAVE;
	return TRUE;
}

static Bool
NvPrepareAccess(PixmapPtr pPix, int index)
{
	ScrnInfoPtr pScrn = xf86Screens[pPix->drawable.pScreen->myNum];

	NVSync(pScrn);
	return TRUE;	
}

static void
NvFinishAccess(PixmapPtr pPix, int index)
{
}

Bool
NvInitExa(ScreenPtr pScreen)
{
	ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
	NVPtr pNv = NVPTR(pScrn);
	ExaDriverPtr pExa;
	int surfaceFormat, rectFormat;

	pExa = exaDriverAlloc();
	if (!pExa)
		return FALSE;

	pNv->pExa = pExa;

	NVResetGraphics(pScrn);

	pExa->exa_major = EXA_VERSION_MAJOR;
	pExa->exa_minor = EXA_VERSION_MINOR;

	pExa->memoryBase = pNv->FbStart;
	pExa->memorySize = pNv->ScratchBufferStart & (~63);
	pExa->offScreenBase = pScrn->virtualY * 
				pScrn->displayWidth * pScrn->bitsPerPixel >> 3;
	pExa->pixmapOffsetAlign = 64;
	pExa->pixmapPitchAlign = 64;

	pExa->flags = EXA_OFFSCREEN_PIXMAPS |
		      EXA_SUPPORTS_OFFSCREEN_OVERLAPS |
		      EXA_MIXED_PIXMAPS;

	pExa->maxX = 8192;
	pExa->maxY = 8192;	

	pExa->WaitMarker = NvWaitMarker;
	pExa->PrepareSolid = NvPrepareSolid;
	pExa->Solid = NvSolid;
	pExa->DoneSolid = NvDoneCopy;
	pExa->PrepareCopy = NvPrepareCopy;
	pExa->Copy = NvCopy;
	pExa->DoneCopy = NvDoneCopy;

	switch(pNv->CurrentLayout.depth) {
	case 24:
		surfaceFormat = SURFACE_FORMAT_DEPTH24;
		rectFormat = RECT_FORMAT_DEPTH24;
		break;
	case 16:
	case 15:
		surfaceFormat = SURFACE_FORMAT_DEPTH16;
		rectFormat = RECT_FORMAT_DEPTH16;
		break;
	default:
		surfaceFormat = SURFACE_FORMAT_DEPTH8;
		rectFormat = RECT_FORMAT_DEPTH8;
		break;
	}
	NVDmaStart(pNv, SURFACE_FORMAT, 1);
	NVDmaNext (pNv, surfaceFormat);
	NVDmaStart(pNv, RECT_FORMAT, 1);
	NVDmaNext (pNv, rectFormat);

	pNv->currentRop = ~0;  /* set to something invalid */
	NVSetRopSolid(pScrn, GXcopy, ~0);

	NVDmaKickoff(pNv);

	/* EXA hits more optimized paths when it does not have to fallback 
	 * because of missing UTS/DFS, hook memcpy-based UTS/DFS.
	 */
	pExa->UploadToScreen = NvUploadToScreen;
	pExa->DownloadFromScreen = NvDownloadFromScreen;
	pExa->PrepareAccess = NvPrepareAccess;
	pExa->FinishAccess = NvFinishAccess;

	return exaDriverInit(pScreen, pExa);
}

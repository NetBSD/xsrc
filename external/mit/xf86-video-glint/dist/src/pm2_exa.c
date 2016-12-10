/*
 * Copyright 1997-2001 by Alan Hourihane, Wigan, England.
 * Copyright 2016 by Michael Lorenz 
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
 * Authors:  Alan Hourihane, <alanh@fairlite.demon.co.uk>
 *           Dirk Hohndel, <hohndel@suse.de>
 *           Stefan Dirsch, <sndirsch@suse.de>
 *           Mark Vojkovich, <mvojkovi@ucsd.edu>
 *           Michel Dänzer, <michdaen@iiic.ethz.ch>
 *
 * this work is sponsored by S.u.S.E. GmbH, Fuerth, Elsa GmbH, Aachen and
 * Siemens Nixdorf Informationssysteme
 * 
 * Permedia 2 accelerated options, EXA style.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <X11/Xarch.h>
#include "xf86.h"
#include "xf86_OSproc.h"

#include "xf86Pci.h"

#include "glint_regs.h"
#include "glint.h"

//#define PM2_DEBUG

#ifdef PM2_DEBUG
#define STATIC static
#define ENTER xf86Msg(X_ERROR, "%s>\n", __func__);
#define DPRINTF xf86Msg
#else
#define ENTER
#define STATIC 
#define DPRINTF while (0) xf86Msg
#endif

STATIC void
Pm2WaitMarker(ScreenPtr pScreen, int Marker)
{
	ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];

	Permedia2Sync(pScrn);
}

STATIC Bool
Pm2PrepareCopy(PixmapPtr pSrcPixmap, PixmapPtr pDstPixmap,
		int xdir, int ydir, int rop, Pixel planemask)
{
	ScrnInfoPtr pScrn = xf86Screens[pDstPixmap->drawable.pScreen->myNum];
	GLINTPtr pGlint = GLINTPTR(pScrn);

	ENTER;

	pGlint->BltScanDirection = 0;
	if (xdir == 1) pGlint->BltScanDirection |= XPositive;
	if (ydir == 1) pGlint->BltScanDirection |= YPositive;
  
	GLINT_WAIT(4);
	DO_PLANEMASK(planemask);

	GLINT_WRITE_REG(UNIT_DISABLE, ColorDDAMode);
	if ((rop == GXset) || (rop == GXclear)) {
		pGlint->FrameBufferReadMode = pGlint->pprod;
	} else
	if ((rop == GXcopy) || (rop == GXcopyInverted)) {
		pGlint->FrameBufferReadMode = pGlint->pprod |FBRM_SrcEnable;
	} else {
		pGlint->FrameBufferReadMode = pGlint->pprod | FBRM_SrcEnable |
							FBRM_DstEnable;
	}
	LOADROP(rop);
	pGlint->srcoff = exaGetPixmapOffset(pSrcPixmap);
	return TRUE;
}

STATIC void
Pm2Copy(PixmapPtr pDstPixmap,
         int srcX, int srcY, int dstX, int dstY, int w, int h)
{
	ScrnInfoPtr pScrn = xf86Screens[pDstPixmap->drawable.pScreen->myNum];
	GLINTPtr pGlint = GLINTPTR(pScrn);
	int align;
	int dstoff = exaGetPixmapOffset(pDstPixmap);
	int pitch = exaGetPixmapPitch(pDstPixmap);
	
	ENTER;

	/* assuming constant pitch for now */	
	srcY += pGlint->srcoff / pitch;
	dstY += dstoff / pitch;

	/* We can only use GXcopy for Packed modes */
	if ((pGlint->ROP != GXcopy) || (pScrn->bitsPerPixel != 8)) { 
		GLINT_WAIT(5);
		GLINT_WRITE_REG(pGlint->FrameBufferReadMode, FBReadMode);
        	Permedia2LoadCoord(pScrn, dstX, dstY, w, h);
        	GLINT_WRITE_REG(((srcY - dstY) & 0x0FFF) << 16 |
        			((srcX - dstX) & 0x0FFF), FBSourceDelta);
	} else {
  		align = (dstX & pGlint->bppalign) - (srcX & pGlint->bppalign);
		GLINT_WAIT(6);
		GLINT_WRITE_REG(pGlint->FrameBufferReadMode|FBRM_Packed, FBReadMode);
        	Permedia2LoadCoord(pScrn, dstX >> pGlint->BppShift, dstY, 
					(w + 7) >> pGlint->BppShift, h);
  		GLINT_WRITE_REG(align << 29 | dstX << 16 | (dstX + w), PackedDataLimits);
        	GLINT_WRITE_REG(((srcX - dstX) & 0x0FFF) << 16 |
        		       (((srcX & ~pGlint->bppalign) - (dstX & ~pGlint->bppalign)) & 0x0FFF),
        		       FBSourceDelta);
	}
	GLINT_WRITE_REG(PrimitiveRectangle | pGlint->BltScanDirection, Render);

	exaMarkSync(pDstPixmap->drawable.pScreen);
}

STATIC void
Pm2DoneCopy(PixmapPtr pPixmap)
{
	ScrnInfoPtr pScrn = xf86Screens[pPixmap->drawable.pScreen->myNum];
	GLINTPtr pGlint = GLINTPTR(pScrn);

}

STATIC Bool
Pm2PrepareSolid(PixmapPtr pPixmap, int rop, Pixel planemask, Pixel color)
{
	ScrnInfoPtr pScrn = xf86Screens[pPixmap->drawable.pScreen->myNum];
	GLINTPtr pGlint = GLINTPTR(pScrn);

	ENTER;

	REPLICATE(color);

	GLINT_WAIT(6);
	DO_PLANEMASK(planemask);
	if (rop == GXcopy) {
		GLINT_WRITE_REG(UNIT_DISABLE, ColorDDAMode);
		GLINT_WRITE_REG(pGlint->pprod, FBReadMode);
		GLINT_WRITE_REG(color, FBBlockColor);
	} else {
		GLINT_WRITE_REG(UNIT_ENABLE, ColorDDAMode);
      		GLINT_WRITE_REG(color, ConstantColor);
		/* We can use Packed mode for filling solid non-GXcopy rasters */
		GLINT_WRITE_REG(pGlint->pprod|FBRM_DstEnable|FBRM_Packed, FBReadMode);
	}
	LOADROP(rop);

	return TRUE;
}

STATIC void
Pm2Solid(PixmapPtr pPixmap, int x1, int y1, int x2, int y2)
{
	ScrnInfoPtr pScrn = xf86Screens[pPixmap->drawable.pScreen->myNum];
	GLINTPtr pGlint = GLINTPTR(pScrn);
	int offset = exaGetPixmapOffset(pPixmap);
	int pitch = exaGetPixmapPitch(pPixmap);
	int w = x2 - x1, h = y2 - y1;
	int speed = 0;

	ENTER;

	y1 += offset / pitch;
	if (pGlint->ROP == GXcopy) {
		GLINT_WAIT(3);
        	Permedia2LoadCoord(pScrn, x1, y1, w, h);
		speed = FastFillEnable;
	} else {
		GLINT_WAIT(4);
		Permedia2LoadCoord(pScrn, x1 >> pGlint->BppShift, y1, 
					(w + 7) >> pGlint->BppShift, h);
  		GLINT_WRITE_REG(x1 << 16 | (x1 + w), PackedDataLimits);
  		speed = 0;
	}
	GLINT_WRITE_REG(PrimitiveRectangle | XPositive | YPositive | speed, Render);

	exaMarkSync(pPixmap->drawable.pScreen);
}

/*
 * Memcpy-based UTS.
 */
STATIC Bool
Pm2UploadToScreen(PixmapPtr pDst, int x, int y, int w, int h,
    char *src, int src_pitch)
{
	ScrnInfoPtr pScrn = xf86Screens[pDst->drawable.pScreen->myNum];
	GLINTPtr pGlint = GLINTPTR(pScrn);
	char  *dst        = pGlint->FbBase + exaGetPixmapOffset(pDst);
	int    dst_pitch  = exaGetPixmapPitch(pDst);

	int bpp    = pDst->drawable.bitsPerPixel;
	int cpp    = (bpp + 7) >> 3;
	int wBytes = w * cpp;

	ENTER;
	dst += (x * cpp) + (y * dst_pitch);

	Permedia2Sync(pScrn);

	while (h--) {
		memcpy(dst, src, wBytes);
		src += src_pitch;
		dst += dst_pitch;
	}
	return TRUE;
}

/*
 * Memcpy-based DFS.
 */
STATIC Bool
Pm2DownloadFromScreen(PixmapPtr pSrc, int x, int y, int w, int h,
    char *dst, int dst_pitch)
{
 	ScrnInfoPtr pScrn = xf86Screens[pSrc->drawable.pScreen->myNum];
	GLINTPtr pGlint = GLINTPTR(pScrn);
	char  *src        = pGlint->FbBase + exaGetPixmapOffset(pSrc);
	int    src_pitch  = exaGetPixmapPitch(pSrc);

	ENTER;
	int bpp    = pSrc->drawable.bitsPerPixel;
	int cpp    = (bpp + 7) >> 3;
	int wBytes = w * cpp;

	src += (x * cpp) + (y * src_pitch);

	Permedia2Sync(pScrn);

	while (h--) {
		memcpy(dst, src, wBytes);
		src += src_pitch;
		dst += dst_pitch;
	}

	return TRUE;
}

Bool
Pm2InitEXA(ScreenPtr pScreen)
{
	ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
	GLINTPtr pGlint = GLINTPTR(pScrn);
	ExaDriverPtr pExa;
	int stride;

	ENTER;

	pExa = exaDriverAlloc();
	if (!pExa)
		return FALSE;

	pGlint->pExa = pExa;

	pExa->exa_major = EXA_VERSION_MAJOR;
	pExa->exa_minor = EXA_VERSION_MINOR;

	pExa->memoryBase = pGlint->FbBase;
	stride = pScrn->displayWidth * (pScrn->bitsPerPixel >> 3);
	pExa->memorySize = min(pGlint->FbMapSize, 2047 * stride);
	DPRINTF(X_ERROR, "stride: %d\n", stride);
	DPRINTF(X_ERROR, "pprod: %08x\n", pGlint->pprod);
	pExa->offScreenBase = stride * pScrn->virtualY;

	/* for now, until I figure out how to do variable stride */
	pExa->pixmapOffsetAlign = stride;
	pExa->pixmapPitchAlign = stride;

	pExa->flags = EXA_OFFSCREEN_PIXMAPS
		      /* | EXA_SUPPORTS_OFFSCREEN_OVERLAPS |*/
		      | EXA_MIXED_PIXMAPS;

	pExa->maxX = 2048;
	pExa->maxY = 2048;

	pExa->WaitMarker = Pm2WaitMarker;

	pExa->PrepareSolid = Pm2PrepareSolid;
	pExa->Solid = Pm2Solid;
	pExa->DoneSolid = Pm2DoneCopy;
	pExa->PrepareCopy = Pm2PrepareCopy;
	pExa->Copy = Pm2Copy;
	pExa->DoneCopy = Pm2DoneCopy;

	/* EXA hits more optimized paths when it does not have to fallback 
	 * because of missing UTS/DFS, hook memcpy-based UTS/DFS.
	 */
	pExa->UploadToScreen = Pm2UploadToScreen;
	pExa->DownloadFromScreen = Pm2DownloadFromScreen;

	Permedia2InitializeEngine(pScrn);

	return exaDriverInit(pScreen, pExa);
}

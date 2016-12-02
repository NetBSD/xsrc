/* $NetBSD: pm3_exa.c,v 1.1 2016/12/02 22:58:26 macallan Exp $ */
/*
 * Copyright (c) 2016 Michael Lorenz
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

/* all driver need this */
#include "xf86.h"
#include "xf86_OSproc.h"
#include "compiler.h"
#include "exa.h"

#include "glint_regs.h"
#include "pm3_regs.h"
#include "glint.h"

/*#define PM3_DEBUG*/

#ifdef PM3_DEBUG
#define ENTER xf86Msg(X_ERROR, "%s>\n", __func__);
#define DPRINTF xf86Msg
#else
#define ENTER
#define DPRINTF while (0) xf86Msg
#endif

static void
Pm3WaitMarker(ScreenPtr pScreen, int Marker)
{
	ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];

	Permedia3Sync(pScrn);
}

static Bool
Pm3PrepareCopy(PixmapPtr pSrcPixmap, PixmapPtr pDstPixmap,
		int xdir, int ydir, int rop, Pixel planemask)
{
	ScrnInfoPtr pScrn = xf86Screens[pDstPixmap->drawable.pScreen->myNum];
	GLINTPtr pGlint = GLINTPTR(pScrn);

	ENTER;

	pGlint->PM3_Render2D =
		PM3Render2D_SpanOperation |
		PM3Render2D_Operation_Normal;

	pGlint->PM3_Config2D =
		PM3Config2D_UserScissorEnable |
		PM3Config2D_ForegroundROPEnable |
		PM3Config2D_ForegroundROP(rop) |
		PM3Config2D_FBWriteEnable;

	if (xdir == 1) pGlint->PM3_Render2D |= PM3Render2D_XPositive;
	if (ydir == 1) pGlint->PM3_Render2D |= PM3Render2D_YPositive;

	if ((rop!=GXclear)&&(rop!=GXset)&&(rop!=GXnoop)&&(rop!=GXinvert)) {
		pGlint->PM3_Render2D |= PM3Render2D_FBSourceReadEnable;
		pGlint->PM3_Config2D |= PM3Config2D_Blocking;
	}

	if ((rop!=GXclear)&&(rop!=GXset)&&(rop!=GXcopy)&&(rop!=GXcopyInverted))
		pGlint->PM3_Config2D |= PM3Config2D_FBDestReadEnable;

	pGlint->srcoff = exaGetPixmapOffset(pSrcPixmap);

	GLINT_WAIT(2);
	PM3_PLANEMASK(planemask);
	GLINT_WRITE_REG(pGlint->PM3_Config2D, PM3Config2D);
	return TRUE;
}

static void
Pm3Copy(PixmapPtr pDstPixmap,
         int srcX, int srcY, int dstX, int dstY, int w, int h)
{
	ScrnInfoPtr pScrn = xf86Screens[pDstPixmap->drawable.pScreen->myNum];
	GLINTPtr pGlint = GLINTPTR(pScrn);
	/* Spans needs to be 32 bit aligned. */
	int x_align = srcX & 0x1f;
	int dstoff = exaGetPixmapOffset(pDstPixmap);
	int pitch = exaGetPixmapPitch(pDstPixmap);
	
	ENTER;

	/* assuming constant pitch for now */	
	srcY += pGlint->srcoff / pitch;
	dstY += dstoff / pitch;

	GLINT_WAIT(5);
	GLINT_WRITE_REG(((dstY & 0x0fff) << 16) | (dstX & 0x0fff), ScissorMinXY);
	GLINT_WRITE_REG((((dstY + h) & 0x0fff) << 16) | ((dstX + w) & 0x0fff), ScissorMaxXY);
	GLINT_WRITE_REG(
		PM3RectanglePosition_XOffset(dstX - x_align) |
		PM3RectanglePosition_YOffset(dstY),
		PM3RectanglePosition);
	GLINT_WRITE_REG(
		PM3FBSourceReadBufferOffset_XOffset(srcX - dstX)|
		PM3FBSourceReadBufferOffset_YOffset(srcY - dstY),
		PM3FBSourceReadBufferOffset);
	GLINT_WRITE_REG(pGlint->PM3_Render2D |
		PM3Render2D_Width(w + x_align)|
		PM3Render2D_Height(h),
		PM3Render2D);
	exaMarkSync(pDstPixmap->drawable.pScreen);
}

static void
Pm3DoneCopy(PixmapPtr pPixmap)
{
	ScrnInfoPtr pScrn = xf86Screens[pPixmap->drawable.pScreen->myNum];
	GLINTPtr pGlint = GLINTPTR(pScrn);

	GLINT_WRITE_REG(0, ScissorMinXY);
	GLINT_WRITE_REG(0x0fff0fff, ScissorMaxXY);
}

static Bool
Pm3PrepareSolid(PixmapPtr pPixmap, int rop, Pixel planemask, Pixel color)
{
	ScrnInfoPtr pScrn = xf86Screens[pPixmap->drawable.pScreen->myNum];
	GLINTPtr pGlint = GLINTPTR(pScrn);

	ENTER;

	/* Prepare Common Render2D & Config2D data */
	pGlint->PM3_Render2D =
		PM3Render2D_XPositive |
		PM3Render2D_YPositive |
		PM3Render2D_Operation_Normal;
	pGlint->PM3_Config2D =
		PM3Config2D_UseConstantSource |
		PM3Config2D_ForegroundROPEnable |
		PM3Config2D_ForegroundROP(rop) |
		PM3Config2D_FBWriteEnable;
	GLINT_WAIT(3);
	REPLICATE(color);

	/* We can't do block fills properly at 32bpp, so we can stick the chip
	 * into 16bpp and double the width and xcoord, but it seems that at
	 * extremely high resolutions (above 1600) it doesn't fill.
	 * so, we fall back to the slower span filling method.
	 */
#if 0
	if ((rop == GXcopy) && (pScrn->bitsPerPixel == 32) && 
	    (pScrn->displayWidth <= 1600)) {
		pGlint->AccelInfoRec->SubsequentSolidFillRect = 
		    Permedia3SubsequentFillRectSolid32bpp;

		if (pGlint->PM3_UsingSGRAM) {
			GLINT_WRITE_REG(color, PM3FBBlockColor);
		} else {
			pGlint->PM3_Render2D |= PM3Render2D_SpanOperation;
			GLINT_WRITE_REG(color, PM3ForegroundColor);
		}
	} else {
	    	pGlint->AccelInfoRec->SubsequentSolidFillRect = 
			Permedia3SubsequentFillRectSolid;
#endif
	    	/* Can't do block fills at 8bpp either */
	    	if ((rop == GXcopy) && (pScrn->bitsPerPixel == 16)) {
			if (pGlint->PM3_UsingSGRAM) {
				GLINT_WRITE_REG(color, PM3FBBlockColor);
			} else {
	        		pGlint->PM3_Render2D |= PM3Render2D_SpanOperation;
				GLINT_WRITE_REG(color, PM3ForegroundColor);
			}
        	} else {
			pGlint->PM3_Render2D |= PM3Render2D_SpanOperation;
			GLINT_WRITE_REG(color, PM3ForegroundColor);
    		}
#if 0
	}
#endif
	PM3_PLANEMASK(planemask);
	if (((rop != GXclear) && 
	     (rop != GXset) &&
	     (rop != GXcopy) &&
	     (rop != GXcopyInverted)) ||
	    ((planemask != 0xffffffff) && !(pGlint->PM3_UsingSGRAM)))
		pGlint->PM3_Config2D |= PM3Config2D_FBDestReadEnable;
	GLINT_WRITE_REG(pGlint->PM3_Config2D, PM3Config2D);
	return TRUE;
}

static void
Pm3Solid(PixmapPtr pPixmap, int x1, int y1, int x2, int y2)
{
	ScrnInfoPtr pScrn = xf86Screens[pPixmap->drawable.pScreen->myNum];
	GLINTPtr pGlint = GLINTPTR(pScrn);
	int offset = exaGetPixmapOffset(pPixmap);
	int pitch = exaGetPixmapPitch(pPixmap);
	int w = x2 - x1, h = y2 - y1;

	ENTER;

	y1 += offset / pitch;

	GLINT_WAIT(2);
	GLINT_WRITE_REG(
		PM3RectanglePosition_XOffset(x1) |
		PM3RectanglePosition_YOffset(y1),
		PM3RectanglePosition);
	GLINT_WRITE_REG(pGlint->PM3_Render2D |
		PM3Render2D_Width(w) | PM3Render2D_Height(h),
		PM3Render2D);

	exaMarkSync(pPixmap->drawable.pScreen);
}

/*
 * Memcpy-based UTS.
 */
static Bool
Pm3UploadToScreen(PixmapPtr pDst, int x, int y, int w, int h,
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

	Permedia3Sync(pScrn);

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
static Bool
Pm3DownloadFromScreen(PixmapPtr pSrc, int x, int y, int w, int h,
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

	Permedia3Sync(pScrn);

	while (h--) {
		memcpy(dst, src, wBytes);
		src += src_pitch;
		dst += dst_pitch;
	}

	return TRUE;
}

Bool
Pm3InitEXA(ScreenPtr pScreen)
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
	pExa->memorySize = min(pGlint->FbMapSize, 4095 * stride);
	xf86Msg(X_ERROR, "stride: %d\n", stride);
	pExa->offScreenBase = stride * pScrn->virtualY;

	/* for now, until I figure out how to do variable stride */
	pExa->pixmapOffsetAlign = stride;
	pExa->pixmapPitchAlign = stride;

	pExa->flags = EXA_OFFSCREEN_PIXMAPS
		      /* | EXA_SUPPORTS_OFFSCREEN_OVERLAPS |*/
		      | EXA_MIXED_PIXMAPS;

	pExa->maxX = 4095;
	pExa->maxY = 4095;

	pExa->WaitMarker = Pm3WaitMarker;

	pExa->PrepareSolid = Pm3PrepareSolid;
	pExa->Solid = Pm3Solid;
	pExa->DoneSolid = Pm3DoneCopy;
	pExa->PrepareCopy = Pm3PrepareCopy;
	pExa->Copy = Pm3Copy;
	pExa->DoneCopy = Pm3DoneCopy;

	/* EXA hits more optimized paths when it does not have to fallback 
	 * because of missing UTS/DFS, hook memcpy-based UTS/DFS.
	 */
	pExa->UploadToScreen = Pm3UploadToScreen;
	pExa->DownloadFromScreen = Pm3DownloadFromScreen;

	Permedia3InitializeEngine(pScrn);

	return exaDriverInit(pScreen, pExa);
}

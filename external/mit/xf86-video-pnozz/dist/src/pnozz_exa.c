/*
 * SBus Weitek P9100 EXA support
 */
/*-
 * Copyright (c) 2021 The NetBSD Foundation, Inc.
 * All rights reserved.
 *
 * This code is derived from software contributed to The NetBSD Foundation
 * by Julian Coleman.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE NETBSD FOUNDATION, INC. AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "pnozz.h"
#include "pnozz_regs.h"


static CARD32 PnozzCopyROP[] = {
	/*GXclear*/		0,
	/*GXand*/		ROP_SRC & ROP_DST,
	/*GXandReverse*/	ROP_SRC & (~ROP_DST),
	/*GXcopy*/		ROP_SRC,
	/*GXandInverted*/	(~ROP_SRC) & ROP_DST,
	/*GXnoop*/		ROP_DST,
	/*GXxor*/		ROP_SRC ^ ROP_DST,
	/*GXor*/		ROP_SRC | ROP_DST,
	/*GXnor*/		(~ROP_SRC) & (~ROP_DST),
	/*GXequiv*/		(~ROP_SRC) ^ ROP_DST,
	/*GXinvert*/		(~ROP_DST),
	/*GXorReverse*/		ROP_SRC | (~ROP_DST),
	/*GXcopyInverted*/	(~ROP_SRC),
	/*GXorInverted*/	(~ROP_SRC) | ROP_DST,
	/*GXnand*/		(~ROP_SRC) | (~ROP_DST),
	/*GXset*/		ROP_SET
};

static CARD32 PnozzDrawROP[] = {
	/*GXclear*/		0,
	/*GXand*/		ROP_PAT & ROP_DST,
	/*GXandReverse*/	ROP_PAT & (~ROP_DST),
	/*GXcopy*/		ROP_PAT,
	/*GXandInverted*/	(~ROP_PAT) & ROP_DST,
	/*GXnoop*/		ROP_DST,
	/*GXxor*/		ROP_PAT ^ ROP_DST,
	/*GXor*/		ROP_PAT | ROP_DST,
	/*GXnor*/		(~ROP_PAT) & (~ROP_DST),
	/*GXequiv*/		(~ROP_PAT) ^ ROP_DST,
	/*GXinvert*/		(~ROP_DST),
	/*GXorReverse*/		ROP_PAT | (~ROP_DST),
	/*GXcopyInverted*/	(~ROP_PAT),
	/*GXorInverted*/	(~ROP_PAT) | ROP_DST,
	/*GXnand*/		(~ROP_PAT) | (~ROP_DST),
	/*GXset*/		ROP_PAT
};

#define waitReady(pPnozz) while((pnozz_read_4(pPnozz, ENGINE_STATUS) & \
				(ENGINE_BUSY | BLITTER_BUSY)) !=0 )

void PnozzInitEngine(PnozzPtr);
void pnozz_write_colour(PnozzPtr pPnozz, int reg, CARD32 colour);

extern CARD32 MaxClip, junk;

static void
PnozzWaitMarker(ScreenPtr pScreen, int Marker)
{
    ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
    PnozzPtr p = GET_PNOZZ_FROM_SCRN(pScrn);

    waitReady(p);
}

static Bool
PnozzPrepareCopy
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
    PnozzPtr pPnozz = GET_PNOZZ_FROM_SCRN(pScrn);
    
    waitReady(pPnozz);
    pnozz_write_4(pPnozz, RASTER_OP, (PnozzCopyROP[alu] & 0xff));
    pnozz_write_4(pPnozz, PLANE_MASK, planemask);
    pPnozz->srcoff = exaGetPixmapOffset(pSrcPixmap) / pPnozz->width;

    return TRUE;
}

static void
PnozzCopy
(
    PixmapPtr pDstPixmap,
    int       xSrc,
    int       ySrc,
    int       xDst,
    int       yDst,
    int       w,
    int       h
)
{
    ScrnInfoPtr pScrn = xf86Screens[pDstPixmap->drawable.pScreen->myNum];
    PnozzPtr pPnozz = GET_PNOZZ_FROM_SCRN(pScrn);
    CARD32 src, dst, srcw, dstw;
    int doff = exaGetPixmapOffset(pDstPixmap) / pPnozz->width;
    
    src = (((xSrc << pPnozz->depthshift) & 0x1fff) << 16) |
	((ySrc + pPnozz->srcoff) & 0x1fff);
    dst = (((xDst << pPnozz->depthshift) & 0x1fff) << 16) |
	((yDst + doff) & 0x1fff);
    srcw = ((((xSrc + w) << pPnozz->depthshift) - 1) << 16) | 
        ((ySrc + pPnozz->srcoff + h - 1) & 0x1fff);
    dstw = ((((xDst + w) << pPnozz->depthshift) - 1) << 16) |
        ((yDst + doff + h - 1) & 0x1fff);

    waitReady(pPnozz);

    pnozz_write_4(pPnozz, ABS_XY0, src);
    pnozz_write_4(pPnozz, ABS_XY1, srcw);
    pnozz_write_4(pPnozz, ABS_XY2, dst);
    pnozz_write_4(pPnozz, ABS_XY3, dstw);
    junk = pnozz_read_4(pPnozz, COMMAND_BLIT);
    exaMarkSync(pDstPixmap->drawable.pScreen);
}

static void
PnozzDoneCopy(PixmapPtr pDstPixmap)
{
    ScrnInfoPtr pScrn = xf86Screens[pDstPixmap->drawable.pScreen->myNum];
    PnozzPtr pPnozz = GET_PNOZZ_FROM_SCRN(pScrn);

    waitReady(pPnozz);
}

static Bool
PnozzPrepareSolid(
    PixmapPtr pPixmap,
    int alu,
    Pixel planemask,
    Pixel fg)
{
    ScrnInfoPtr pScrn = xf86Screens[pPixmap->drawable.pScreen->myNum];
    PnozzPtr pPnozz = GET_PNOZZ_FROM_SCRN(pScrn);
    
    waitReady(pPnozz);
    pnozz_write_colour(pPnozz, FOREGROUND_COLOR, fg);
    pnozz_write_4(pPnozz, RASTER_OP, PnozzDrawROP[alu] & 0xff);
    pnozz_write_4(pPnozz, PLANE_MASK, planemask);
    pnozz_write_4(pPnozz, COORD_INDEX, 0);

    return TRUE;
}

static void
PnozzSolid(
    PixmapPtr pPixmap,
    int x,
    int y,
    int x2,
    int y2)
{
    ScrnInfoPtr pScrn = xf86Screens[pPixmap->drawable.pScreen->myNum];
    PnozzPtr pPnozz = GET_PNOZZ_FROM_SCRN(pScrn);
    int w = x2 - x - 1;
    int h = y2 - y - 1;

    waitReady(pPnozz);
    pnozz_write_4(pPnozz, RECT_RTW_XY, ((x & 0x1fff) << 16) | 
        (y & 0x1fff));
    pnozz_write_4(pPnozz, RECT_RTP_XY, (((w & 0x1fff) << 16) | 
        (h & 0x1fff)));
    junk = pnozz_read_4(pPnozz, COMMAND_QUAD);
    exaMarkSync(pPixmap->drawable.pScreen);
}

int
PnozzEXAInit(ScreenPtr pScreen)
{
    ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
    PnozzPtr pPnozz = GET_PNOZZ_FROM_SCRN(pScrn);
    ExaDriverPtr pExa;
    
    PnozzInitEngine(pPnozz);

    pExa = exaDriverAlloc();
    if (!pExa)
	return FALSE;

    pPnozz->pExa = pExa;

    pExa->exa_major = EXA_VERSION_MAJOR;
    pExa->exa_minor = EXA_VERSION_MINOR;

    pExa->memoryBase = pPnozz->fb;

    /* round to multiple of pixmap pitch */
    pExa->memorySize = (pPnozz->vidmem / pPnozz->width) * pPnozz->width;
    pExa->offScreenBase = pPnozz->width * pPnozz->height;

    /*
     * our blitter can't deal with variable pitches
     */
    pExa->pixmapOffsetAlign = pPnozz->width;
    pExa->pixmapPitchAlign = pPnozz->width;

    pExa->flags = EXA_MIXED_PIXMAPS;

    pExa->maxX = 4096;
    pExa->maxY = 4096;

    pExa->WaitMarker = PnozzWaitMarker;

    pExa->PrepareSolid = PnozzPrepareSolid;
    pExa->Solid = PnozzSolid;
    pExa->DoneSolid = PnozzDoneCopy;

    pExa->PrepareCopy = PnozzPrepareCopy;
    pExa->Copy = PnozzCopy;
    pExa->DoneCopy = PnozzDoneCopy;

    return exaDriverInit(pScreen, pExa);;
}

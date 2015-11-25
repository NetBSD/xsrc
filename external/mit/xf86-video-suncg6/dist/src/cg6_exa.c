/*
 * Sun GX and Turbo GX EXA support
 *
 * Copyright (C) 2015 Michael Lorenz
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
/* $XFree86: xc/programs/Xserver/hw/xfree86/drivers/suncg6/cg6_accel.c $ */

#include "cg6.h"
#include "cg6_regs.h"


static CARD32 Cg6BlitROP[] = {
    ROP_BLIT(GX_ROP_CLEAR,  GX_ROP_CLEAR),	/* GXclear */
    ROP_BLIT(GX_ROP_CLEAR,  GX_ROP_NOOP),	/* GXand */
    ROP_BLIT(GX_ROP_CLEAR,  GX_ROP_INVERT),	/* GXandReverse */
    ROP_BLIT(GX_ROP_CLEAR,  GX_ROP_SET),	/* GXcopy */
    ROP_BLIT(GX_ROP_NOOP,   GX_ROP_CLEAR),	/* GXandInverted */
    ROP_BLIT(GX_ROP_NOOP,   GX_ROP_NOOP),	/* GXnoop */
    ROP_BLIT(GX_ROP_NOOP,   GX_ROP_INVERT),	/* GXxor */
    ROP_BLIT(GX_ROP_NOOP,   GX_ROP_SET),	/* GXor */
    ROP_BLIT(GX_ROP_INVERT, GX_ROP_CLEAR),	/* GXnor */
    ROP_BLIT(GX_ROP_INVERT, GX_ROP_NOOP),	/* GXequiv */
    ROP_BLIT(GX_ROP_INVERT, GX_ROP_INVERT),	/* GXinvert */
    ROP_BLIT(GX_ROP_INVERT, GX_ROP_SET),	/* GXorReverse */
    ROP_BLIT(GX_ROP_SET,    GX_ROP_CLEAR),	/* GXcopyInverted */
    ROP_BLIT(GX_ROP_SET,    GX_ROP_NOOP),	/* GXorInverted */
    ROP_BLIT(GX_ROP_SET,    GX_ROP_INVERT),	/* GXnand */
    ROP_BLIT(GX_ROP_SET,    GX_ROP_SET),	/* GXset */
};

static CARD32 Cg6DrawROP[] = {
    ROP_FILL(GX_ROP_CLEAR,  GX_ROP_CLEAR),	/* GXclear */
    ROP_FILL(GX_ROP_CLEAR,  GX_ROP_NOOP),	/* GXand */
    ROP_FILL(GX_ROP_CLEAR,  GX_ROP_INVERT),	/* GXandReverse */
    ROP_FILL(GX_ROP_CLEAR,  GX_ROP_SET),	/* GXcopy */
    ROP_FILL(GX_ROP_NOOP,   GX_ROP_CLEAR),	/* GXandInverted */
    ROP_FILL(GX_ROP_NOOP,   GX_ROP_NOOP),	/* GXnoop */
    ROP_FILL(GX_ROP_NOOP,   GX_ROP_INVERT),	/* GXxor */
    ROP_FILL(GX_ROP_NOOP,   GX_ROP_SET),	/* GXor */
    ROP_FILL(GX_ROP_INVERT, GX_ROP_CLEAR),	/* GXnor */
    ROP_FILL(GX_ROP_INVERT, GX_ROP_NOOP),	/* GXequiv */
    ROP_FILL(GX_ROP_INVERT, GX_ROP_INVERT),	/* GXinvert */
    ROP_FILL(GX_ROP_INVERT, GX_ROP_SET),	/* GXorReverse */
    ROP_FILL(GX_ROP_SET,    GX_ROP_CLEAR),	/* GXcopyInverted */
    ROP_FILL(GX_ROP_SET,    GX_ROP_NOOP),	/* GXorInverted */
    ROP_FILL(GX_ROP_SET,    GX_ROP_INVERT),	/* GXnand */
    ROP_FILL(GX_ROP_SET,    GX_ROP_SET),	/* GXset */
};

#define runDraw(pCg6) { volatile CARD32 rubbish = pCg6->fbc->draw; }
#define runBlit(pCg6) { volatile CARD32 rubbish = pCg6->fbc->blit; }

/*
 * XXX
 * was GX_FULL, which apparently isn't enough on some (slower) CG6 like
 * the one found on the SPARCstation LX mainboard
 */
#define waitReady(pCg6) while(pCg6->fbc->s & GX_INPROGRESS)

void Cg6InitEngine(Cg6Ptr);

static void
Cg6WaitMarker(ScreenPtr pScreen, int Marker)
{
	ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
	Cg6Ptr p = GET_CG6_FROM_SCRN(pScrn);

	waitReady(p);
}

static Bool
Cg6PrepareCopy
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
    Cg6Ptr pCg6 = GET_CG6_FROM_SCRN(pScrn);
    
    waitReady(pCg6);
    
    pCg6->fbc->mode = GX_BLIT_SRC |
		GX_MODE_COLOR8 |
		GX_DRAW_RENDER |
		GX_BWRITE0_ENABLE |
		GX_BWRITE1_DISABLE |
		GX_BREAD_0 |
		GX_BDISP_0;
                
    /* we probably don't need the following three */
    pCg6->fbc->fg = 0xff;
    pCg6->fbc->bg = 0x00;
    pCg6->fbc->s = 0;
    
    pCg6->srcoff = exaGetPixmapOffset(pSrcPixmap) / pCg6->width;
    pCg6->fbc->alu = Cg6BlitROP[alu];
    pCg6->fbc->pm = planemask;
    return TRUE;
}

static void
Cg6Copy
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
    Cg6Ptr pCg6 = GET_CG6_FROM_SCRN(pScrn);
    int doff;

    doff = exaGetPixmapOffset(pDstPixmap) / pCg6->width;
    waitReady(pCg6);
    pCg6->fbc->x0 = xSrc;
    pCg6->fbc->y0 = ySrc + pCg6->srcoff;
    pCg6->fbc->x1 = xSrc + w - 1;
    pCg6->fbc->y1 = ySrc + pCg6->srcoff + h - 1;
    pCg6->fbc->x2 = xDst;
    pCg6->fbc->y2 = yDst + doff;
    pCg6->fbc->x3 = xDst + w - 1;
    pCg6->fbc->y3 = yDst + doff + h - 1;
    runBlit(pCg6);
    exaMarkSync(pDstPixmap->drawable.pScreen);
}

static void
Cg6DoneCopy(PixmapPtr pDstPixmap)
{
    ScrnInfoPtr pScrn = xf86Screens[pDstPixmap->drawable.pScreen->myNum];
    Cg6Ptr pCg6 = GET_CG6_FROM_SCRN(pScrn);

    waitReady(pCg6);
}

static Bool
Cg6PrepareSolid(
    PixmapPtr pPixmap,
    int alu,
    Pixel planemask,
    Pixel fg)
{
    ScrnInfoPtr pScrn = xf86Screens[pPixmap->drawable.pScreen->myNum];
    Cg6Ptr pCg6 = GET_CG6_FROM_SCRN(pScrn);
    CARD32 c2;

    pCg6->srcoff = exaGetPixmapOffset(pPixmap) / pCg6->width;

    waitReady(pCg6);

    pCg6->fbc->mode = GX_BLIT_SRC |
		GX_MODE_COLOR8 |
		GX_DRAW_RENDER |
		GX_BWRITE0_ENABLE |
		GX_BWRITE1_DISABLE |
		GX_BREAD_0 |
		GX_BDISP_0;
    pCg6->fbc->fg = fg;
    pCg6->fbc->s = 0;
    pCg6->fbc->alu = Cg6DrawROP[alu];
    pCg6->fbc->pm = planemask;
    return TRUE;
}

static void
Cg6Solid(
    PixmapPtr pPixmap,
    int x,
    int y,
    int x2,
    int y2)
{
    ScrnInfoPtr pScrn = xf86Screens[pPixmap->drawable.pScreen->myNum];
    Cg6Ptr pCg6 = GET_CG6_FROM_SCRN(pScrn);

    waitReady(pCg6);
    pCg6->fbc->arecty = y + pCg6->srcoff;
    pCg6->fbc->arectx = x;
    pCg6->fbc->rrecty = y2 - y - 1;
    pCg6->fbc->rrectx = x2 - x - 1;
    runDraw(pCg6);
    exaMarkSync(pPixmap->drawable.pScreen);
}

/*
 * Memcpy-based UTS.
 * TODO: use host blit
 */
static Bool
Cg6UploadToScreen(PixmapPtr pDst, int x, int y, int w, int h,
    char *src, int src_pitch)
{
    ScrnInfoPtr pScrn = xf86Screens[pDst->drawable.pScreen->myNum];
    Cg6Ptr pCg6       = GET_CG6_FROM_SCRN(pScrn);
    char  *dst        = pCg6->fb + exaGetPixmapOffset(pDst);
    int    dst_pitch  = exaGetPixmapPitch(pDst);


    dst += x + (y * dst_pitch);

    while (h--) {
        memcpy(dst, src, w);
        src += src_pitch;
        dst += dst_pitch;
    }
    return TRUE;
}

/*
 * Memcpy-based DFS.
 */
static Bool
Cg6DownloadFromScreen(PixmapPtr pSrc, int x, int y, int w, int h,
    char *dst, int dst_pitch)
{
    ScrnInfoPtr pScrn = xf86Screens[pSrc->drawable.pScreen->myNum];
    Cg6Ptr pCg6       = GET_CG6_FROM_SCRN(pScrn);
    char  *src        = pCg6->fb + exaGetPixmapOffset(pSrc);
    int    src_pitch  = exaGetPixmapPitch(pSrc);

    src += x + (y * src_pitch);

    while (h--) {
        memcpy(dst, src, w);
        src += src_pitch;
        dst += dst_pitch;
    }
    return TRUE;
}

int
CG6EXAInit(ScreenPtr pScreen)
{
    ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
    Cg6Ptr pCg6 = GET_CG6_FROM_SCRN(pScrn);
    ExaDriverPtr pExa;
    
    Cg6InitEngine(pCg6);

    pExa = exaDriverAlloc();
    if (!pExa)
	return FALSE;

    pCg6->pExa = pExa;

    pExa->exa_major = EXA_VERSION_MAJOR;
    pExa->exa_minor = EXA_VERSION_MINOR;

    pExa->memoryBase = pCg6->fb;
    pExa->memorySize = pCg6->vidmem;
    pExa->offScreenBase = pCg6->width * pCg6->height;

    /*
     * our blitter can't deal with variable pitches
     */
    pExa->pixmapOffsetAlign = pCg6->width;
    pExa->pixmapPitchAlign = pCg6->width;

    pExa->flags = EXA_OFFSCREEN_PIXMAPS |
		  EXA_MIXED_PIXMAPS;

    pExa->maxX = 4096;
    pExa->maxY = 4096;

    pExa->WaitMarker = Cg6WaitMarker;

    pExa->PrepareSolid = Cg6PrepareSolid;
    pExa->Solid = Cg6Solid;
    pExa->DoneSolid = Cg6DoneCopy;

    pExa->PrepareCopy = Cg6PrepareCopy;
    pExa->Copy = Cg6Copy;
    pExa->DoneCopy = Cg6DoneCopy;

    pExa->UploadToScreen = Cg6UploadToScreen;
    pExa->DownloadFromScreen = Cg6DownloadFromScreen;

    return exaDriverInit(pScreen, pExa);;
}

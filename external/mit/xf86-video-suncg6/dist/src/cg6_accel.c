/*
 * Sun GX and Turbo GX acceleration support
 *
 * Copyright (C) 2005 Michael Lorenz
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "cg6.h"
#include "cg6_regs.h"
#include "dgaproc.h"


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

static CARD32 Cg6StippleROP[16]={
    ROP_STIP(GX_ROP_CLEAR,  GX_ROP_CLEAR),	/* GXclear */
    ROP_STIP(GX_ROP_CLEAR,  GX_ROP_NOOP),	/* GXand */
    ROP_STIP(GX_ROP_CLEAR,  GX_ROP_INVERT),	/* GXandReverse */
    ROP_STIP(GX_ROP_CLEAR,  GX_ROP_SET),	/* GXcopy */
    ROP_STIP(GX_ROP_NOOP,   GX_ROP_CLEAR),	/* GXandInverted */
    ROP_STIP(GX_ROP_NOOP,   GX_ROP_NOOP),	/* GXnoop */
    ROP_STIP(GX_ROP_NOOP,   GX_ROP_INVERT),	/* GXxor */
    ROP_STIP(GX_ROP_NOOP,   GX_ROP_SET),	/* GXor */
    ROP_STIP(GX_ROP_INVERT, GX_ROP_CLEAR),	/* GXnor */
    ROP_STIP(GX_ROP_INVERT, GX_ROP_NOOP),	/* GXequiv */
    ROP_STIP(GX_ROP_INVERT, GX_ROP_INVERT),	/* GXinvert */
    ROP_STIP(GX_ROP_INVERT, GX_ROP_SET),	/* GXorReverse */
    ROP_STIP(GX_ROP_SET,    GX_ROP_CLEAR),	/* GXcopyInverted */
    ROP_STIP(GX_ROP_SET,    GX_ROP_NOOP),	/* GXorInverted */
    ROP_STIP(GX_ROP_SET,    GX_ROP_INVERT),	/* GXnand */
    ROP_STIP(GX_ROP_SET,    GX_ROP_SET),	/* GXset */
};

static CARD32 Cg6OpaqueStippleROP[16]={
    ROP_OSTP(GX_ROP_CLEAR,  GX_ROP_CLEAR),	/* GXclear */
    ROP_OSTP(GX_ROP_CLEAR,  GX_ROP_NOOP),	/* GXand */
    ROP_OSTP(GX_ROP_CLEAR,  GX_ROP_INVERT),	/* GXandReverse */
    ROP_OSTP(GX_ROP_CLEAR,  GX_ROP_SET),	/* GXcopy */
    ROP_OSTP(GX_ROP_NOOP,   GX_ROP_CLEAR),	/* GXandInverted */
    ROP_OSTP(GX_ROP_NOOP,   GX_ROP_NOOP),	/* GXnoop */
    ROP_OSTP(GX_ROP_NOOP,   GX_ROP_INVERT),	/* GXxor */
    ROP_OSTP(GX_ROP_NOOP,   GX_ROP_SET),	/* GXor */
    ROP_OSTP(GX_ROP_INVERT, GX_ROP_CLEAR),	/* GXnor */
    ROP_OSTP(GX_ROP_INVERT, GX_ROP_NOOP),	/* GXequiv */
    ROP_OSTP(GX_ROP_INVERT, GX_ROP_INVERT),	/* GXinvert */
    ROP_OSTP(GX_ROP_INVERT, GX_ROP_SET),	/* GXorReverse */
    ROP_OSTP(GX_ROP_SET,    GX_ROP_CLEAR),	/* GXcopyInverted */
    ROP_OSTP(GX_ROP_SET,    GX_ROP_NOOP),	/* GXorInverted */
    ROP_OSTP(GX_ROP_SET,    GX_ROP_INVERT),	/* GXnand */
    ROP_OSTP(GX_ROP_SET,    GX_ROP_SET),	/* GXset */
};

/* DGA stuff */

static Bool Cg6_OpenFramebuffer(ScrnInfoPtr pScrn, char **, unsigned char **mem,
    int *, int *, int *);
static Bool Cg6_SetMode(ScrnInfoPtr, DGAModePtr);
static void Cg6_SetViewport(ScrnInfoPtr, int, int, int);
static int Cg6_GetViewport(ScrnInfoPtr);
static void Cg6_FillRect(ScrnInfoPtr, int, int, int, int, unsigned long);
static void Cg6_BlitRect(ScrnInfoPtr, int, int, int, int, int, int);

static void Cg6Sync(ScrnInfoPtr);

static DGAFunctionRec Cg6_DGAFuncs = {
	Cg6_OpenFramebuffer,
	NULL,
	Cg6_SetMode,
	Cg6_SetViewport,
	Cg6_GetViewport,
	Cg6Sync,
	Cg6_FillRect,
	Cg6_BlitRect,
	NULL
};


/* 
 * wait until the engine is idle
 * unclip since clipping also influences framebuffer accesses 
 */
static void 
Cg6Sync(ScrnInfoPtr pScrn)
{
    Cg6Ptr pCg6 = GET_CG6_FROM_SCRN(pScrn);
    
    while (pCg6->fbc->s & GX_INPROGRESS);
    pCg6->fbc->clipminx = 0;
    pCg6->fbc->clipmaxx = pCg6->width;
    pCg6->fbc->clipminy = 0;
    pCg6->fbc->clipmaxy = pCg6->maxheight;
}

#define runDraw(pCg6) { volatile CARD32 rubbish = pCg6->fbc->draw; }
#define runBlit(pCg6) { volatile CARD32 rubbish = pCg6->fbc->blit; }

/*
 * XXX
 * was GX_FULL, which apparently isn't enough on some (slower) CG6 like
 * the one found on the SPARCstation LX mainboard
 */
#define waitReady(pCg6) while(pCg6->fbc->s & GX_INPROGRESS)

/*
 * restore clipping values set by the Xserver since we're messing with them in 
 * CPU-to-screen colour expansion
 */
 
static void 
unClip(Cg6Ptr pCg6)
{
    pCg6->fbc->clipminx = pCg6->clipxa;
    pCg6->fbc->clipmaxx = pCg6->clipxe;
}

void
Cg6InitEngine(Cg6Ptr pCg6)
{
    pCg6->clipxa = 0;
    pCg6->clipxe = pCg6->width;
    pCg6->fbc->clipminx = 0;
    pCg6->fbc->clipmaxx = pCg6->width;
    pCg6->fbc->clipminy = 0;
    pCg6->fbc->clipmaxy = pCg6->maxheight;

    pCg6->fbc->mode = GX_BLIT_SRC |
		GX_MODE_COLOR8 |
		GX_DRAW_RENDER |
		GX_BWRITE0_ENABLE |
		GX_BWRITE1_DISABLE |
		GX_BREAD_0 |
		GX_BDISP_0;
	
    pCg6->fbc->fg = 0xff;
    pCg6->fbc->bg = 0x00;
    
    /* we ignore the pixel mask anyway but for completeness... */
    pCg6->fbc->pixelm = ~0;
    
    pCg6->fbc->s = 0;
    pCg6->fbc->clip = 0;
    pCg6->fbc->offx = 0;
    pCg6->fbc->offy = 0;
    pCg6->fbc->incx = 0;
    pCg6->fbc->incy = 0;
}

static void
Cg6SetupForScreenToScreenCopy(
    ScrnInfoPtr  pScrn,
    int          xdir,
    int          ydir,
    int          rop,
    unsigned int planemask,
    int          TransparencyColour
)
{
    Cg6Ptr pCg6 = GET_CG6_FROM_SCRN(pScrn);
    
    waitReady(pCg6);
    unClip(pCg6);
    
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
    
    pCg6->fbc->alu = Cg6BlitROP[rop];
    pCg6->fbc->pm = planemask;
}

static void
Cg6SubsequentScreenToScreenCopy
(
    ScrnInfoPtr pScrn,
    int         xSrc,
    int         ySrc,
    int         xDst,
    int         yDst,
    int         w,
    int         h
)
{
    Cg6Ptr pCg6 = GET_CG6_FROM_SCRN(pScrn);

    waitReady(pCg6);
    pCg6->fbc->x0 = xSrc;
    pCg6->fbc->y0 = ySrc;
    pCg6->fbc->x1 = xSrc + w - 1;
    pCg6->fbc->y1 = ySrc + h - 1;
    pCg6->fbc->x2 = xDst;
    pCg6->fbc->y2 = yDst;
    pCg6->fbc->x3 = xDst + w - 1;
    pCg6->fbc->y3 = yDst + h - 1;
    runBlit(pCg6);
}

static void
Cg6SetupForSolidFill
(
    ScrnInfoPtr  pScrn,
    int          colour,
    int          rop,
    unsigned int planemask
)
{
    Cg6Ptr pCg6 = GET_CG6_FROM_SCRN(pScrn);
    CARD32 c2;

    waitReady(pCg6);
    unClip(pCg6);

    pCg6->fbc->mode = GX_BLIT_SRC |
		GX_MODE_COLOR8 |
		GX_DRAW_RENDER |
		GX_BWRITE0_ENABLE |
		GX_BWRITE1_DISABLE |
		GX_BREAD_0 |
		GX_BDISP_0;
    pCg6->fbc->fg = colour;
    pCg6->fbc->s = 0;
    pCg6->fbc->alu = Cg6DrawROP[rop];
    pCg6->fbc->pm = planemask;
}

static void
Cg6SubsequentSolidFillRect
(
    ScrnInfoPtr pScrn,
    int         x,
    int         y,
    int         w,
    int         h
)
{
    Cg6Ptr pCg6 = GET_CG6_FROM_SCRN(pScrn);

    waitReady(pCg6);
    pCg6->fbc->arecty = y;
    pCg6->fbc->arectx = x;
    /* use the relative coordinate registers - saves two additions */
    pCg6->fbc->rrecty = h - 1;
    pCg6->fbc->rrectx = w - 1;
    runDraw(pCg6);
}

#ifdef HAVE_XAA_H

static void 
Cg6SetupForCPUToScreenColorExpandFill(ScrnInfoPtr pScrn,
        		int fg, int bg,
			int rop,
			unsigned int planemask)
{
    Cg6Ptr pCg6 = GET_CG6_FROM_SCRN(pScrn);

    waitReady(pCg6);
    pCg6->fbc->mode = GX_BLIT_NOSRC | GX_MODE_COLOR1;
    
    if(bg == -1) {
	/* transparent */
	pCg6->fbc->alu = Cg6StippleROP[rop] | GX_PATTERN_ONES;
	pCg6->fbc->bg = 0xff;
    } else {
	/* draw background */
	pCg6->fbc->alu = Cg6OpaqueStippleROP[rop] | GX_PATTERN_ONES;
	pCg6->fbc->bg = bg;
    }	
    pCg6->fbc->fg = fg;
    pCg6->fbc->incx = 32;
    pCg6->fbc->incy = 0;
    pCg6->fbc->s = 0;
    pCg6->fbc->pm = planemask;
}

static void 
Cg6SubsequentScanlineCPUToScreenColorExpandFill(ScrnInfoPtr pScrn,
			int x, int y, int w, int h,
			int skipleft )
{
    Cg6Ptr pCg6 = GET_CG6_FROM_SCRN(pScrn);
    
    pCg6->scan_x = x;
    pCg6->scan_xe = x + w - 1;
    pCg6->scan_y = y;
    
    /* 
     * we need to clip the left and right margins of what we're going to draw or
     * we'll end up with garbage left or right
     */
    pCg6->fbc->clipminx = x + skipleft;
    pCg6->fbc->clipmaxx = x + w - 1;
    pCg6->words_in_scanline = ((w + 31) >> 5);
}

static void
Cg6SubsequentColorExpandScanline(ScrnInfoPtr pScrn, int bufno)
{
    Cg6Ptr pCg6 = GET_CG6_FROM_SCRN(pScrn);
    int i;
    
    /* 
     * the GX is WEIRD. if we tell it to draw n pixels it will fill the entire 
     * line with whatever we feed into the font register. When we write the next
     * word it draws the entire line AGAIN. So we turn on clipping and pretend 
     * to write only 32 pixels... 
     */
    pCg6->fbc->x0 = pCg6->scan_x;
    pCg6->fbc->x1 = pCg6->scan_x + 31;
    pCg6->fbc->y0 = pCg6->scan_y;	
    for (i = 0; i < pCg6->words_in_scanline; i++) {
	pCg6->fbc->font = pCg6->scanline[i];
    }
    pCg6->scan_y++;	
}

static void 
Cg6SetupForSolidLine(ScrnInfoPtr pScrn, int color, int rop, 
    unsigned int planemask)
{
    Cg6Ptr pCg6 = GET_CG6_FROM_SCRN(pScrn);

    waitReady(pCg6);
    unClip(pCg6);

    pCg6->fbc->fg = color;
    pCg6->fbc->mode = GX_BLIT_NOSRC;
    pCg6->fbc->s = 0;
    pCg6->fbc->alu = Cg6DrawROP[rop];
    pCg6->fbc->pm = planemask;
}

static void
Cg6SubsequentSolidTwoPointLine(ScrnInfoPtr pScrn, int x1, int y1, int x2,
    int y2, int flags)
{
    Cg6Ptr pCg6 = GET_CG6_FROM_SCRN(pScrn);

    /* 
     * XXX we're blatantly ignoring the flags parameter which could tell us not 
     * to draw the last point. Xsun simply reads it from the framebuffer and 
     * puts it back after drawing the line but that would mean we have to wait 
     * until the line is actually drawn. On the other hand - line drawing is 
     * pretty fast so we won't lose too much speed
     */
    waitReady(pCg6);
    pCg6->fbc->aliney = y1;
    pCg6->fbc->alinex = x1;
    pCg6->fbc->aliney = y2;
    pCg6->fbc->alinex = x2;
    runDraw(pCg6);
}      
     
static void
Cg6SetClippingRectangle(ScrnInfoPtr pScrn, int left, int top, int right, 
			 int bottom)
{
    Cg6Ptr pCg6 = GET_CG6_FROM_SCRN(pScrn);

    pCg6->fbc->clipminx = pCg6->clipxa = left;
    pCg6->fbc->clipminy = top;
    pCg6->fbc->clipmaxx = pCg6->clipxe = right;
    pCg6->fbc->clipmaxy = bottom;
}

static void
Cg6DisableClipping(ScrnInfoPtr pScrn)
{
    Cg6Ptr pCg6 = GET_CG6_FROM_SCRN(pScrn);

    pCg6->fbc->clipminx = pCg6->clipxa = 0;
    pCg6->fbc->clipminy = 0;
    pCg6->fbc->clipmaxx = pCg6->clipxe = pCg6->width;
    pCg6->fbc->clipmaxy = pCg6->maxheight;    
}

int
CG6AccelInit(ScrnInfoPtr pScrn)
{
    Cg6Ptr pCg6 = GET_CG6_FROM_SCRN(pScrn);
    XAAInfoRecPtr pXAAInfo = pCg6->pXAA;

    pXAAInfo->Flags = LINEAR_FRAMEBUFFER | PIXMAP_CACHE | OFFSCREEN_PIXMAPS;
    pXAAInfo->maxOffPixWidth = pCg6->width;
    pXAAInfo->maxOffPixHeight = pCg6->maxheight;
    
    Cg6InitEngine(pCg6);
    
    /* wait until the engine is idle and remove clipping */
    pXAAInfo->Sync = Cg6Sync;
    
    /* clipping */
    pXAAInfo->SetClippingRectangle = Cg6SetClippingRectangle;
    pXAAInfo->DisableClipping = Cg6DisableClipping;
    pXAAInfo->ClippingFlags = HARDWARE_CLIP_SCREEN_TO_SCREEN_COPY |
        HARDWARE_CLIP_SOLID_FILL |
        /*HARDWARE_CLIP_MONO_8x8_FILL |
        HARDWARE_CLIP_COLOR_8x8_FILL |*/
        HARDWARE_CLIP_SOLID_LINE;

    /* Screen-to-screen copy */
    pXAAInfo->ScreenToScreenCopyFlags = NO_TRANSPARENCY;
    pXAAInfo->SetupForScreenToScreenCopy = Cg6SetupForScreenToScreenCopy;
    pXAAInfo->SubsequentScreenToScreenCopy =
        Cg6SubsequentScreenToScreenCopy;

    /* Solid fills */
    pXAAInfo->SetupForSolidFill = Cg6SetupForSolidFill;
    pXAAInfo->SubsequentSolidFillRect = Cg6SubsequentSolidFillRect;

    /* TODO: add pattern fills */

    /* colour expansion */
    pXAAInfo->ScanlineCPUToScreenColorExpandFillFlags = 
	LEFT_EDGE_CLIPPING|SCANLINE_PAD_DWORD;
    pXAAInfo->NumScanlineColorExpandBuffers = 1;
    pCg6->buffers[0] = (unsigned char *)pCg6->scanline;
    pXAAInfo->ScanlineColorExpandBuffers = pCg6->buffers;
    pXAAInfo->SetupForScanlineCPUToScreenColorExpandFill = 
	Cg6SetupForCPUToScreenColorExpandFill;
    pXAAInfo->SubsequentScanlineCPUToScreenColorExpandFill =
	Cg6SubsequentScanlineCPUToScreenColorExpandFill;
    pXAAInfo->SubsequentColorExpandScanline =
	Cg6SubsequentColorExpandScanline;
    
    /* line drawing */
    pXAAInfo->SetupForSolidLine = Cg6SetupForSolidLine;
    pXAAInfo->SubsequentSolidTwoPointLine = Cg6SubsequentSolidTwoPointLine;
    pXAAInfo->SolidLineFlags = BIT_ORDER_IN_BYTE_MSBFIRST;
    /* 
     * apparently the hardware can't do dashed lines, only lines with patterns
     * which isn't useful
     */
    
    /* TODO: add host-to-vram colour blits */
     
    return 0;
}

#endif /* HAVE_XAA_H */

Bool
Cg6DGAInit(ScreenPtr pScreen)
{
    ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
    Cg6Ptr pCg6 = GET_CG6_FROM_SCRN(pScrn);
    DGAModePtr mode;
    int result;
    
    mode = xnfcalloc(sizeof(DGAModeRec), 1);
    if (mode == NULL) {
        xf86Msg(X_WARNING, "%s: DGA setup failed, cannot allocate memory\n",
            pCg6->psdp->device);
        return FALSE;
    }
    
    mode->mode = pScrn->modes;
    mode->flags = DGA_PIXMAP_AVAILABLE | DGA_CONCURRENT_ACCESS;
    if(!pCg6->NoAccel) {
        mode->flags |= DGA_FILL_RECT | DGA_BLIT_RECT;
    }
    
    mode->imageWidth = mode->pixmapWidth = mode->viewportWidth =
	pScrn->virtualX;
    mode->imageHeight = mode->pixmapHeight = mode->viewportHeight =
	pScrn->virtualY;

    mode->bytesPerScanline = mode->imageWidth;

    mode->byteOrder = pScrn->imageByteOrder;
    mode->depth = 8;
    mode->bitsPerPixel = 8;
    mode->red_mask = pScrn->mask.red;
    mode->green_mask = pScrn->mask.green;
    mode->blue_mask = pScrn->mask.blue;
    
    mode->visualClass = PseudoColor;
    mode->address = pCg6->fb;

    result = DGAInit(pScreen, &Cg6_DGAFuncs, mode, 1);

    if (result) {
    	xf86Msg(X_INFO, "%s: DGA initialized\n",
            pCg6->psdp->device);
	return TRUE;
    } else {
     	xf86Msg(X_WARNING, "%s: DGA setup failed\n",
            pCg6->psdp->device);
	return FALSE;
    }
}

static Bool 
Cg6_OpenFramebuffer(ScrnInfoPtr pScrn, char **name,
				unsigned char **mem,
				int *size, int *offset,
				int *extra)
{
    Cg6Ptr pCg6 = GET_CG6_FROM_SCRN(pScrn);

    *name = pCg6->psdp->device;

    *mem = (unsigned char*)CG6_RAM_VOFF;
    *size = pCg6->vidmem;
    *offset = 0;
    *extra = 0;

    return TRUE;
}

static Bool
Cg6_SetMode(ScrnInfoPtr pScrn, DGAModePtr pMode)
{
    /*
     * Nothing to do, we currently only support one mode
     * and we are always in it.
     */
    return TRUE;
}

static void
Cg6_SetViewport(ScrnInfoPtr pScrn, int x, int y, int flags)
{
     /* We don't support viewports, so... */
}

static int
Cg6_GetViewport(ScrnInfoPtr pScrn)
{
    /* No viewports, none pending... */
    return 0;
}

static void
Cg6_FillRect(ScrnInfoPtr pScrn, int x, int y, int w, int h, unsigned long color)
{

    Cg6SetupForSolidFill(pScrn, color, GXset, 8);
    Cg6SubsequentSolidFillRect(pScrn, x, y, w, h);
}

static void
Cg6_BlitRect(ScrnInfoPtr pScrn, int srcx, int srcy,
			 int w, int h, int dstx, int dsty)
{

    Cg6SetupForScreenToScreenCopy(pScrn, 0, 0, GXcopy, 8, 0);
    Cg6SubsequentScreenToScreenCopy(pScrn, srcx, srcy, dstx, dsty, w, h);
}

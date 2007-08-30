/*
 * Fujitsu AG-10e acceleration support
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

#include <fcntl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <dev/sun/fbio.h>
#include <dev/wscons/wsconsio.h>

#include "ag10e.h"
#include "dgaproc.h"
#include "miline.h"
#include "xaalocal.h"	/* For replacements */

static Bool AG10E_OpenFramebuffer(ScrnInfoPtr pScrn, char **, unsigned int *mem,
    unsigned int *, unsigned int *, unsigned int *);
static Bool AG10E_SetMode(ScrnInfoPtr, DGAModePtr);
static void AG10E_SetViewport(ScrnInfoPtr, int, int, int);
static int AG10E_GetViewport(ScrnInfoPtr);

#if 0
static void AG10E_FillRect(ScrnInfoPtr, int, int, int, int, unsigned long);
static void AG10E_BlitRect(ScrnInfoPtr, int, int, int, int, int, int);
#endif

static DGAFunctionRec AG10E_DGAFuncs = {
	AG10E_OpenFramebuffer,
	NULL,
	AG10E_SetMode,
	AG10E_SetViewport,
	AG10E_GetViewport,
#if 0
	AG10ESync,
	AG10E_FillRect,
	AG10E_BlitRect,
#else
	NULL,
	NULL,
	NULL,
#endif
	NULL
};

static void SXSync(ScrnInfoPtr pScrn);
static void SXSetupForFillRectSolid(ScrnInfoPtr pScrn, int color, int rop,
						unsigned int planemask);
static void SXSubsequentFillRectSolid(ScrnInfoPtr pScrn, int x, int y,
						int w, int h);
static void SXSetupForMono8x8PatternFill(ScrnInfoPtr pScrn, int patternx, 
						int patterny, 
					   	int fg, int bg, int rop,
					   	unsigned int planemask);
static void SXSubsequentMono8x8PatternFillRect(ScrnInfoPtr pScrn, int patternx,
						int patterny, int x, int y,
				   		int w, int h);
static void SXSetupForScreenToScreenCopy(ScrnInfoPtr pScrn, int xdir, int ydir,
						int rop, unsigned int planemask,
				    		int transparency_color);
static void SXSubsequentScreenToScreenCopy(ScrnInfoPtr pScrn, int x1, int y1,
						int x2, int y2, int w, int h);
static void SXSetClippingRectangle(ScrnInfoPtr pScrn, int x1, int y1, 
						int x2,int y2);
static void SXDisableClipping(ScrnInfoPtr pScrn);
static void SXLoadCoord(ScrnInfoPtr pScrn, int x, int y, int w, int h,
				int a, int d);
static void SXSetupForSolidLine(ScrnInfoPtr pScrn, int color, int rop,
				unsigned int planemask);
static void SXSubsequentHorVertLine(ScrnInfoPtr pScrn, int x1, int y1,
				int len, int dir);
static void SXSubsequentSolidBresenhamLine(ScrnInfoPtr pScrn,
        			int x, int y, int dmaj, int dmin, int e, 
				int len, int octant);
static void SXPolylinesThinSolidWrapper(DrawablePtr pDraw, GCPtr pGC,
   				int mode, int npt, DDXPointPtr pPts);
static void SXPolySegmentThinSolidWrapper(DrawablePtr pDraw, GCPtr pGC,
 				int nseg, xSegment *pSeg);

#define MAX_FIFO_ENTRIES		15

#define PARTPROD(a,b,c) (((a)<<6) | ((b)<<3) | (c))

static int partprod500TX[] = {
	-1,
	PARTPROD(0,0,1), PARTPROD(0,0,2), PARTPROD(0,1,2), PARTPROD(0,0,3),
	PARTPROD(0,1,3), PARTPROD(0,2,3), PARTPROD(1,2,3), PARTPROD(0,0,4),
	PARTPROD(0,1,4), PARTPROD(0,2,4), PARTPROD(1,2,4), PARTPROD(0,3,4),
	PARTPROD(1,3,4), PARTPROD(2,3,4),              -1, PARTPROD(0,0,5), 
	PARTPROD(0,1,5), PARTPROD(0,2,5), PARTPROD(1,2,5), PARTPROD(0,3,5), 
	PARTPROD(1,3,5), PARTPROD(2,3,5),              -1, PARTPROD(0,4,5), 
	PARTPROD(1,4,5), PARTPROD(2,4,5), PARTPROD(3,4,5),              -1,
	             -1,              -1,              -1, PARTPROD(0,0,6), 
	PARTPROD(0,1,6), PARTPROD(0,2,6), PARTPROD(1,2,6), PARTPROD(0,3,6), 
	PARTPROD(1,3,6), PARTPROD(2,3,6),              -1, PARTPROD(0,4,6), 
	PARTPROD(1,4,6), PARTPROD(2,4,6),              -1, PARTPROD(3,4,6),
	             -1,              -1,              -1, PARTPROD(0,5,6), 
	PARTPROD(1,5,6), PARTPROD(2,5,6),              -1, PARTPROD(3,5,6), 
	             -1,              -1,              -1, PARTPROD(4,5,6), 
	             -1,              -1,              -1,              -1,
		     -1,              -1,              -1, PARTPROD(0,0,7), 
	             -1, PARTPROD(0,2,7), PARTPROD(1,2,7), PARTPROD(0,3,7), 
	PARTPROD(1,3,7), PARTPROD(2,3,7),              -1, PARTPROD(0,4,7),
	PARTPROD(1,4,7), PARTPROD(2,4,7),              -1, PARTPROD(3,4,7), 
	             -1,              -1,              -1, PARTPROD(0,5,7),
	PARTPROD(1,5,7), PARTPROD(2,5,7),              -1, PARTPROD(3,5,7), 
	             -1,              -1,              -1, PARTPROD(4,5,7),
	             -1,              -1,              -1,              -1,
		     -1,              -1,              -1, PARTPROD(0,6,7), 
	PARTPROD(1,6,7), PARTPROD(2,6,7),              -1, PARTPROD(3,6,7),
	             -1,              -1,              -1, PARTPROD(4,6,7), 
	             -1,              -1,              -1,              -1,
		     -1,              -1,              -1, PARTPROD(5,6,7), 
	             -1,              -1,              -1,              -1,
		     -1,              -1,              -1,              -1,
		     -1,              -1,              -1,              -1,
		     -1,              -1,              -1, PARTPROD(0,7,7),
		      0};


void
SXInitializeEngine(ScrnInfoPtr pScrn)
{
    AG10EPtr pAG10E = GLINTPTR(pScrn);
    /* Initialize the Accelerator Engine to defaults */

    GLINT_SLOW_WRITE_REG(UNIT_DISABLE,	DitherMode);
    GLINT_SLOW_WRITE_REG(0x400,		FilterMode);
    GLINT_SLOW_WRITE_REG(UNIT_DISABLE,  ScissorMode);
    GLINT_SLOW_WRITE_REG(pAG10E->pprod,	LBReadMode);
    GLINT_SLOW_WRITE_REG(pAG10E->pprod,	FBReadMode);
    GLINT_SLOW_WRITE_REG(UNIT_DISABLE,	LBWriteMode);
    GLINT_SLOW_WRITE_REG(UNIT_ENABLE,	FBWriteMode);
    GLINT_SLOW_WRITE_REG(UNIT_DISABLE,	AlphaBlendMode);
    GLINT_SLOW_WRITE_REG(UNIT_DISABLE,	ColorDDAMode);
    GLINT_SLOW_WRITE_REG(UNIT_DISABLE,	TextureColorMode);
    GLINT_SLOW_WRITE_REG(UNIT_DISABLE,	TextureAddressMode);
    GLINT_SLOW_WRITE_REG(UNIT_DISABLE,  TextureReadMode);
    GLINT_SLOW_WRITE_REG(UNIT_DISABLE,  GLINTWindow);
    GLINT_SLOW_WRITE_REG(UNIT_DISABLE,  AlphaBlendMode);
    GLINT_SLOW_WRITE_REG(UNIT_DISABLE,  DepthMode);
    GLINT_SLOW_WRITE_REG(UNIT_DISABLE,  RouterMode);
    GLINT_SLOW_WRITE_REG(UNIT_DISABLE,	FogMode);
    GLINT_SLOW_WRITE_REG(UNIT_DISABLE,	AntialiasMode);
    GLINT_SLOW_WRITE_REG(UNIT_DISABLE,	AlphaTestMode);
    GLINT_SLOW_WRITE_REG(UNIT_DISABLE,	StencilMode);
    GLINT_SLOW_WRITE_REG(UNIT_DISABLE,	AreaStippleMode);
    GLINT_SLOW_WRITE_REG(UNIT_DISABLE,	LineStippleMode);
    GLINT_SLOW_WRITE_REG(0,		UpdateLineStippleCounters);
    GLINT_SLOW_WRITE_REG(UNIT_DISABLE,	LogicalOpMode);
    GLINT_SLOW_WRITE_REG(UNIT_DISABLE,	DepthMode);
    GLINT_SLOW_WRITE_REG(UNIT_DISABLE,	StatisticMode);
    GLINT_SLOW_WRITE_REG(0xffffffff,	FBHardwareWriteMask);
    GLINT_SLOW_WRITE_REG(0xffffffff,	FBSoftwareWriteMask);
    GLINT_SLOW_WRITE_REG(UNIT_DISABLE,	RasterizerMode);
    GLINT_SLOW_WRITE_REG(UNIT_DISABLE,	GLINTDepth);
    GLINT_SLOW_WRITE_REG(UNIT_DISABLE,	FBSourceOffset);
    GLINT_SLOW_WRITE_REG(UNIT_DISABLE,	FBPixelOffset);
    GLINT_SLOW_WRITE_REG(UNIT_DISABLE,	LBSourceOffset);
    GLINT_SLOW_WRITE_REG(UNIT_DISABLE,	WindowOrigin);
    GLINT_SLOW_WRITE_REG(UNIT_DISABLE,	FBWindowBase);
    GLINT_SLOW_WRITE_REG(UNIT_DISABLE,	LBWindowBase);
    GLINT_SLOW_WRITE_REG(UNIT_DISABLE,	TextureAddressMode);
    GLINT_SLOW_WRITE_REG(UNIT_DISABLE,	RouterMode);

    pAG10E->ROP = 0xFF;
    pAG10E->ClippingOn = FALSE;
    pAG10E->startxsub = 0;
    pAG10E->startxdom = 0;
    pAG10E->starty = 0;
    pAG10E->count = 0;
    pAG10E->dxdom = 0;
    pAG10E->dy = 1;
    pAG10E->planemask = 0;
    GLINT_SLOW_WRITE_REG(0, StartXSub);
    GLINT_SLOW_WRITE_REG(0, StartXDom);
    GLINT_SLOW_WRITE_REG(0, StartY);
    GLINT_SLOW_WRITE_REG(0, GLINTCount);
    GLINT_SLOW_WRITE_REG(0, dXDom);
    GLINT_SLOW_WRITE_REG(0, dXSub);
    GLINT_SLOW_WRITE_REG(1<<16, dY);
}

int
AG10EAccelInit(ScreenPtr pScreen)
{
    XAAInfoRecPtr infoPtr;
    ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
    AG10EPtr pAG10E = GLINTPTR(pScrn);
    long memory = pAG10E->FbMapSize;
    BoxRec AvailFBArea;

    pAG10E->pprod = partprod500TX[pScrn->displayWidth >> 5];

    pAG10E->AccelInfoRec = infoPtr = XAACreateInfoRec();
    if (!infoPtr) return FALSE;

    SXInitializeEngine(pScrn);

    infoPtr->Flags = PIXMAP_CACHE |
		     LINEAR_FRAMEBUFFER |
		     OFFSCREEN_PIXMAPS;
 
    infoPtr->Sync = SXSync;

    infoPtr->SetClippingRectangle = SXSetClippingRectangle;
    infoPtr->DisableClipping = SXDisableClipping;
    infoPtr->ClippingFlags = HARDWARE_CLIP_MONO_8x8_FILL |
			     HARDWARE_CLIP_SCREEN_TO_SCREEN_COPY |
			     HARDWARE_CLIP_SOLID_FILL;

    infoPtr->SolidFillFlags = 0;
    infoPtr->SetupForSolidFill = SXSetupForFillRectSolid;
    infoPtr->SubsequentSolidFillRect = SXSubsequentFillRectSolid;

    infoPtr->SolidLineFlags = 0;
    infoPtr->PolySegmentThinSolidFlags = 0;
    infoPtr->PolylinesThinSolidFlags = 0;
    infoPtr->SetupForSolidLine = SXSetupForSolidLine;
    infoPtr->SubsequentSolidHorVertLine = SXSubsequentHorVertLine;
    infoPtr->SubsequentSolidBresenhamLine = 
					SXSubsequentSolidBresenhamLine;
    infoPtr->PolySegmentThinSolid = SXPolySegmentThinSolidWrapper;
    infoPtr->PolylinesThinSolid = SXPolylinesThinSolidWrapper;

    infoPtr->ScreenToScreenCopyFlags = NO_TRANSPARENCY |
				       ONLY_LEFT_TO_RIGHT_BITBLT;
    infoPtr->SetupForScreenToScreenCopy = SXSetupForScreenToScreenCopy;
    infoPtr->SubsequentScreenToScreenCopy = SXSubsequentScreenToScreenCopy;

    infoPtr->Mono8x8PatternFillFlags = HARDWARE_PATTERN_PROGRAMMED_ORIGIN |
				       HARDWARE_PATTERN_SCREEN_ORIGIN |
				       HARDWARE_PATTERN_PROGRAMMED_BITS;
    infoPtr->SetupForMono8x8PatternFill = SXSetupForMono8x8PatternFill;
    infoPtr->SubsequentMono8x8PatternFillRect = 
					SXSubsequentMono8x8PatternFillRect;

    AvailFBArea.x1 = 0;
    AvailFBArea.y1 = 0;
    AvailFBArea.x2 = pScrn->displayWidth;
    AvailFBArea.y2 = pAG10E->vidmem / (pScrn->displayWidth * 
					  pScrn->bitsPerPixel / 8);

    if (AvailFBArea.y2 > 2047) AvailFBArea.y2 = 2047;

    xf86InitFBManager(pScreen, &AvailFBArea);

    return (XAAInit(pScreen, infoPtr));
}

static void SXLoadCoord(
	ScrnInfoPtr pScrn,
	int x, int y,
	int w, int h,
	int a, int d
){
    AG10EPtr pAG10E = GLINTPTR(pScrn);
    
    if (w != pAG10E->startxsub) {
    	GLINT_WRITE_REG(w<<16, StartXSub);
	pAG10E->startxsub = w;
    }
    if (x != pAG10E->startxdom) {
    	GLINT_WRITE_REG(x<<16,StartXDom);
	pAG10E->startxdom = x;
    }
    if (y != pAG10E->starty) {
    	GLINT_WRITE_REG(y<<16,StartY);
	pAG10E->starty = y;
    }
    if (h != pAG10E->count) {
    	GLINT_WRITE_REG(h,GLINTCount);
	pAG10E->count = h;
    }
    if (a != pAG10E->dxdom) {
    	GLINT_WRITE_REG(a<<16,dXDom);
	pAG10E->dxdom = a;
    }
    if (d != pAG10E->dy) {
    	GLINT_WRITE_REG(d<<16,dY);
	pAG10E->dy = d;
    }
}

static void
SXSync(
	ScrnInfoPtr pScrn
){
    AG10EPtr pAG10E = GLINTPTR(pScrn);
    CARD32 readValue;

    CHECKCLIPPING;

    while (GLINT_READ_REG(DMACount) != 0);
    GLINT_WAIT(3);
    GLINT_WRITE_REG(0x400, FilterMode);
    GLINT_WRITE_REG(0, GlintSync);
    do {
   	while(GLINT_READ_REG(OutFIFOWords) == 0);
	readValue = GLINT_READ_REG(OutputFIFO);
    } while (readValue != Sync_tag);
}

static void
SXSetupForFillRectSolid(
	ScrnInfoPtr pScrn, 
	int color, int rop, 
	unsigned int planemask
){
    AG10EPtr pAG10E = GLINTPTR(pScrn);
    pAG10E->ForeGroundColor = color;
	
    GLINT_WAIT(6);
    REPLICATE(color);
    DO_PLANEMASK(planemask);
    if (pScrn->bitsPerPixel >= 24) {
	GLINT_WRITE_REG(pAG10E->pprod | FBRM_DstEnable, FBReadMode);
	GLINT_WRITE_REG(UNIT_ENABLE, ColorDDAMode);
	GLINT_WRITE_REG(color, ConstantColor);
	pAG10E->FrameBufferReadMode = 0;
    } else
    if (rop == GXcopy) {
	GLINT_WRITE_REG(pAG10E->pprod, FBReadMode);
	GLINT_WRITE_REG(UNIT_DISABLE, ColorDDAMode);
	GLINT_WRITE_REG(color, FBBlockColor);
	pAG10E->FrameBufferReadMode = FastFillEnable;
    } else {
	GLINT_WRITE_REG(pAG10E->pprod | FBRM_DstEnable, FBReadMode);
	GLINT_WRITE_REG(UNIT_ENABLE, ColorDDAMode);
	GLINT_WRITE_REG(color, ConstantColor);
	pAG10E->FrameBufferReadMode = 0;
    }
    LOADROP(rop);
}

static void
SXSubsequentFillRectSolid(
	ScrnInfoPtr pScrn, 
	int x, int y, 
	int w, int h
){
    AG10EPtr pAG10E = GLINTPTR(pScrn);

    GLINT_WAIT(8);
    SXLoadCoord(pScrn, x, y, x+w, h, 0, 1);
    GLINT_WRITE_REG(PrimitiveTrapezoid | pAG10E->FrameBufferReadMode,Render);
}

static void
SXSetClippingRectangle(
	ScrnInfoPtr pScrn, 	
	int x1, int y1, 
	int x2, int y2
){
    AG10EPtr pAG10E = GLINTPTR(pScrn);

    GLINT_WAIT(5);
    GLINT_WRITE_REG((y1&0xFFFF)<<16|(x1&0xFFFF), ScissorMinXY);
    GLINT_WRITE_REG((y2&0xFFFF)<<16|(x2&0xFFFF), ScissorMaxXY);
    GLINT_WRITE_REG(1, ScissorMode); /* Enable Scissor Mode */
    pAG10E->ClippingOn = TRUE;
}

static void
SXDisableClipping(
	ScrnInfoPtr pScrn
){
    AG10EPtr pAG10E = GLINTPTR(pScrn);
    CHECKCLIPPING;
}

static void
SXSetupForScreenToScreenCopy(
	ScrnInfoPtr pScrn,
	int xdir, int  ydir, 	
	int rop,
	unsigned int planemask, 
	int transparency_color
){
    AG10EPtr pAG10E = GLINTPTR(pScrn);
    
    pAG10E->BltScanDirection = ydir;

    GLINT_WAIT(6);
    DO_PLANEMASK(planemask);

    if (rop == GXcopy) {
	GLINT_WRITE_REG(pAG10E->pprod | FBRM_SrcEnable, FBReadMode);
    } else {
	GLINT_WRITE_REG(pAG10E->pprod | FBRM_SrcEnable | FBRM_DstEnable, FBReadMode);
    }
    LOADROP(rop);
}

static void
SXSubsequentScreenToScreenCopy(
	ScrnInfoPtr pScrn, 
	int x1, int y1, 
	int x2, int y2,
	int w, int h
){
    AG10EPtr pAG10E = GLINTPTR(pScrn);
    int srcaddr, dstaddr;

    GLINT_WAIT(10);

    srcaddr = y1 * pScrn->displayWidth + x1;
    dstaddr = y2 * pScrn->displayWidth + x2;
    GLINT_WRITE_REG(srcaddr - dstaddr, FBSourceOffset);

    if (pAG10E->BltScanDirection != 1) {
	y1 += h - 1;
	y2 += h - 1;
        SXLoadCoord(pScrn, x2, y2, x2+w, h, 0, -1);
    } else {
        SXLoadCoord(pScrn, x2, y2, x2+w, h, 0, 1);
    }	

    GLINT_WRITE_REG(PrimitiveTrapezoid, Render);
}

void SXSetupForMono8x8PatternFill(
	ScrnInfoPtr pScrn,
	int patternx, int patterny, 
	int fg, int bg, int rop,
	unsigned int planemask
){
    AG10EPtr pAG10E = GLINTPTR(pScrn);

    if (bg == -1) pAG10E->FrameBufferReadMode = -1;
	else    pAG10E->FrameBufferReadMode = 0;
    pAG10E->ForeGroundColor = fg;
    pAG10E->BackGroundColor = bg;
    REPLICATE(pAG10E->ForeGroundColor);
    REPLICATE(pAG10E->BackGroundColor);

    GLINT_WAIT(13);
    DO_PLANEMASK(planemask);
    GLINT_WRITE_REG((patternx & 0x000000FF),       AreaStipplePattern0);
    GLINT_WRITE_REG((patternx & 0x0000FF00) >> 8,  AreaStipplePattern1);
    GLINT_WRITE_REG((patternx & 0x00FF0000) >> 16, AreaStipplePattern2);
    GLINT_WRITE_REG((patternx & 0xFF000000) >> 24, AreaStipplePattern3);
    GLINT_WRITE_REG((patterny & 0x000000FF),       AreaStipplePattern4);
    GLINT_WRITE_REG((patterny & 0x0000FF00) >> 8,  AreaStipplePattern5);
    GLINT_WRITE_REG((patterny & 0x00FF0000) >> 16, AreaStipplePattern6);
    GLINT_WRITE_REG((patterny & 0xFF000000) >> 24, AreaStipplePattern7);
  
    GLINT_WRITE_REG(UNIT_ENABLE, ColorDDAMode);
    if (rop == GXcopy) 
    	GLINT_WRITE_REG(pAG10E->pprod, FBReadMode);
    else
    	GLINT_WRITE_REG(pAG10E->pprod | FBRM_DstEnable, FBReadMode);
    LOADROP(rop);
}

static void 
SXSubsequentMono8x8PatternFillRect(
	ScrnInfoPtr pScrn, 
	int patternx, int patterny, 
	int x, int y,
	int w, int h
){
    AG10EPtr pAG10E = GLINTPTR(pScrn);
  
    GLINT_WAIT(12);
    SXLoadCoord(pScrn, x, y, x+w, h, 0, 1);

    if (pAG10E->FrameBufferReadMode != -1) {
  	GLINT_WRITE_REG(pAG10E->BackGroundColor, ConstantColor);
	GLINT_WRITE_REG(2<<1|2<<4|patternx<<7|patterny<<12|ASM_InvertPattern |
					UNIT_ENABLE, AreaStippleMode);
	GLINT_WRITE_REG(AreaStippleEnable | PrimitiveTrapezoid, Render);
    }

    GLINT_WRITE_REG(pAG10E->ForeGroundColor, ConstantColor);
    GLINT_WRITE_REG(2<<1|2<<4|patternx<<7|patterny<<12|
  						UNIT_ENABLE, AreaStippleMode);
    GLINT_WRITE_REG(AreaStippleEnable | PrimitiveTrapezoid, Render);
}

#if 0
static void 
SXWriteBitmap(ScrnInfoPtr pScrn,
    int x, int y, int w, int h,
    unsigned char *src,
    int srcwidth,
    int skipleft,
    int fg, int bg,
    int rop,
    unsigned int planemask
){
    XAAInfoRecPtr infoRec = GET_XAAINFORECPTR_FROM_SCRNINFOPTR(pScrn);
    AG10EPtr pAG10E = GLINTPTR(pScrn);
    unsigned char *srcpntr;
    int dwords, height, mode;
    Bool SecondPass = FALSE;
    register int count;
    register CARD32* pattern;

    w += skipleft;
    x -= skipleft;
    dwords = (w + 31) >> 5;

    SXSetClippingRectangle(pScrn,x+skipleft, y, x+w, y+h);

    GLINT_WAIT(11);
    DO_PLANEMASK(planemask);
    GLINT_WRITE_REG(0, RasterizerMode);
    LOADROP(rop);
    if (rop == GXcopy) {
	mode = FastFillEnable;
	GLINT_WRITE_REG(UNIT_DISABLE, ColorDDAMode);
	GLINT_WRITE_REG(pAG10E->pprod, FBReadMode);
    } else {
	mode = 0;
	GLINT_WRITE_REG(UNIT_ENABLE, ColorDDAMode);
	GLINT_WRITE_REG(pAG10E->pprod | FBRM_DstEnable, FBReadMode);
    }
    SXLoadCoord(pScrn, x, y, x+w, h, 0, 1);

    if(bg == -1) {
	REPLICATE(fg);
	GLINT_WAIT(3);
	if (rop == GXcopy) {
	    GLINT_WRITE_REG(fg, FBBlockColor);
	} else {
	    GLINT_WRITE_REG(fg, PatternRamData0);
	}
    } else if(rop == GXcopy) {
	REPLICATE(bg);
	GLINT_WAIT(5);
	if (rop == GXcopy) {
	    GLINT_WRITE_REG(bg, FBBlockColor);
	} else {
	    GLINT_WRITE_REG(bg, PatternRamData0);
	}
	GLINT_WRITE_REG(PrimitiveTrapezoid |mode|FastFillEnable,Render);
	REPLICATE(fg);
	if (rop == GXcopy) {
	    GLINT_WRITE_REG(fg, FBBlockColor);
	} else {
	    GLINT_WRITE_REG(fg, PatternRamData0);
	}
    } else {
	SecondPass = TRUE;
	REPLICATE(fg);
	GLINT_WAIT(3);
	if (rop == GXcopy) {
	    GLINT_WRITE_REG(fg, FBBlockColor);
	} else {
	    GLINT_WRITE_REG(fg, PatternRamData0);
	}
    }

SECOND_PASS:
    GLINT_WRITE_REG(PrimitiveTrapezoid | FastFillEnable | mode | SyncOnBitMask, Render);
    
    height = h;
    srcpntr = src;
    while(height--) {
	count = dwords >> 3;
	pattern = (CARD32*)srcpntr;
	while(count--) {
		GLINT_WAIT(8);
		GLINT_WRITE_REG(*(pattern), BitMaskPattern);
		GLINT_WRITE_REG(*(pattern+1), BitMaskPattern);
		GLINT_WRITE_REG(*(pattern+2), BitMaskPattern);
		GLINT_WRITE_REG(*(pattern+3), BitMaskPattern);
		GLINT_WRITE_REG(*(pattern+4), BitMaskPattern);
		GLINT_WRITE_REG(*(pattern+5), BitMaskPattern);
		GLINT_WRITE_REG(*(pattern+6), BitMaskPattern);
		GLINT_WRITE_REG(*(pattern+7), BitMaskPattern);
		pattern+=8;
	}
	count = dwords & 0x07;
	GLINT_WAIT(count);
	while (count--)
		GLINT_WRITE_REG(*(pattern++), BitMaskPattern);
	srcpntr += srcwidth;
    }    

    if(SecondPass) {
	SecondPass = FALSE;
	REPLICATE(bg);
	GLINT_WAIT(4);
	GLINT_WRITE_REG(InvertBitMask, RasterizerMode);
	if (rop == GXcopy) {
	    GLINT_WRITE_REG(bg, FBBlockColor);
	} else {
	    GLINT_WRITE_REG(bg, PatternRamData0);
	}
	goto SECOND_PASS;
    }

    GLINT_WAIT(2);
    GLINT_WRITE_REG(0, RasterizerMode);
    CHECKCLIPPING;
    SET_SYNC_FLAG(infoRec);
}

static void 
SXWritePixmap(
   ScrnInfoPtr pScrn,
   int x, int y, int w, int h,
   unsigned char *src,	
   int srcwidth,	/* bytes */
   int rop,
   unsigned int planemask,
   int trans,
   int bpp, int depth
){
    XAAInfoRecPtr infoRec = GET_XAAINFORECPTR_FROM_SCRNINFOPTR(pScrn);
    AG10EPtr pAG10E = GLINTPTR(pScrn);
    CARD32 *srcp;
    int count,dwords, skipleft, Bpp = bpp >> 3; 

    if((skipleft = (long)src & 0x03L)) {
	skipleft /= Bpp;

	x -= skipleft;	     
	w += skipleft;
	
	src = (unsigned char*)((long)src & ~0x03L);     
    }

    switch(Bpp) {
    case 1:	dwords = (w + 3) >> 2;
		break;
    case 2:	dwords = (w + 1) >> 1;
		break;
    case 4:	dwords = w;
		break;
    default: return; 
    }

    SXSetClippingRectangle(pScrn,x+skipleft, y, x+w, y+h);

    GLINT_WAIT(12);
    DO_PLANEMASK(planemask);
    GLINT_WRITE_REG(UNIT_DISABLE, ColorDDAMode);
    if (rop == GXcopy) {
        GLINT_WRITE_REG(pAG10E->pprod, FBReadMode);
    } else {
        GLINT_WRITE_REG(pAG10E->pprod | FBRM_DstEnable, FBReadMode);
    }
    LOADROP(rop);
    SXLoadCoord(pScrn, x, y, x+w, h, 0, 1);
    GLINT_WRITE_REG(PrimitiveTrapezoid | SyncOnHostData, Render);

    while(h--) {
      count = dwords;
      srcp = (CARD32*)src;
      while(count >= infoRec->ColorExpandRange) {
	GLINT_WAIT(infoRec->ColorExpandRange);
	/* (0x0f << 4) | 0x0e is the TAG for GLINTColor */
       	GLINT_WRITE_REG(((infoRec->ColorExpandRange - 2) << 16) | (0x0F << 4) | 
				0x0E, OutputFIFO);
	GLINT_MoveDWORDS(
		(CARD32*)((char*)pAG10E->IOBase + OutputFIFO + 4),
 		(CARD32*)srcp, infoRec->ColorExpandRange - 1);
	count -= infoRec->ColorExpandRange - 1;
	srcp += infoRec->ColorExpandRange - 1;
      }
      if(count) {
	GLINT_WAIT(count);
	/* (0x0F << 4) | 0x0E is the TAG for GLINTColor */
       	GLINT_WRITE_REG(((count - 1) << 16) | (0x0f << 4) | 
				0x0e, OutputFIFO);
	GLINT_MoveDWORDS(
		(CARD32*)((char*)pAG10E->IOBase + OutputFIFO + 4),
 		(CARD32*)srcp, count);
      }
      src += srcwidth;
    }  
    CHECKCLIPPING;
    SET_SYNC_FLAG(infoRec);
}
#endif

static void 
SXPolylinesThinSolidWrapper(
   DrawablePtr     pDraw,
   GCPtr           pGC,
   int             mode,
   int             npt,
   DDXPointPtr     pPts
){
    XAAInfoRecPtr infoRec = GET_XAAINFORECPTR_FROM_GC(pGC);
    AG10EPtr pAG10E = GLINTPTR(infoRec->pScrn);
    pAG10E->CurrentGC = pGC;
    pAG10E->CurrentDrawable = pDraw;
    if(infoRec->NeedToSync) (*infoRec->Sync)(infoRec->pScrn);
    XAAPolyLines(pDraw, pGC, mode, npt, pPts);
}

static void 
SXPolySegmentThinSolidWrapper(
   DrawablePtr     pDraw,
   GCPtr           pGC,
   int             nseg,
   xSegment        *pSeg
){
    XAAInfoRecPtr infoRec = GET_XAAINFORECPTR_FROM_GC(pGC);
    AG10EPtr pAG10E = GLINTPTR(infoRec->pScrn);
    pAG10E->CurrentGC = pGC;
    pAG10E->CurrentDrawable = pDraw;
    if(infoRec->NeedToSync) (*infoRec->Sync)(infoRec->pScrn);
    XAAPolySegment(pDraw, pGC, nseg, pSeg);
}

static void
SXSetupForSolidLine(ScrnInfoPtr pScrn, int color,
					 int rop, unsigned int planemask)
{
    AG10EPtr pAG10E = GLINTPTR(pScrn);

    GLINT_WAIT(7);
    DO_PLANEMASK(planemask);
    GLINT_WRITE_REG(color, GLINTColor);
    GLINT_WRITE_REG(UNIT_DISABLE, ColorDDAMode);
    if (rop == GXcopy) {
  	GLINT_WRITE_REG(pAG10E->pprod, FBReadMode);
    } else {
  	GLINT_WRITE_REG(pAG10E->pprod | FBRM_DstEnable, FBReadMode);
    }
    LOADROP(rop);
}

static void
SXSubsequentHorVertLine(ScrnInfoPtr pScrn,int x,int y,int len,int dir)
{
    AG10EPtr pAG10E = GLINTPTR(pScrn);
  
    GLINT_WAIT(9);
    if (dir == DEGREES_0) {
        SXLoadCoord(pScrn, x, y, 0, len, 1, 0);
    } else {
        SXLoadCoord(pScrn, x, y, 0, len, 0, 1);
    }

    GLINT_WRITE_REG(PrimitiveLine, Render);
}

static void 
SXSubsequentSolidBresenhamLine( ScrnInfoPtr pScrn,
        int x, int y, int dmaj, int dmin, int e, int len, int octant)
{
    AG10EPtr pAG10E = GLINTPTR(pScrn);
    int dxdom, dy;

    if(dmaj == dmin) {
	GLINT_WAIT(9);
	if(octant & YDECREASING) {
	    dy = -1;
	} else {
	    dy = 1;
	}

	if(octant & XDECREASING) {
	    dxdom = -1;
	} else {
	    dxdom = 1;
	}

        SXLoadCoord(pScrn, x, y, 0, len, dxdom, dy);
	GLINT_WRITE_REG(PrimitiveLine, Render);
	return;
    }

    fbBres(pAG10E->CurrentDrawable, pAG10E->CurrentGC, 0,
                (octant & XDECREASING) ? -1 : 1, 
                (octant & YDECREASING) ? -1 : 1, 
                (octant & YMAJOR) ? Y_AXIS : X_AXIS,
                x, y,  e, dmin, -dmaj, len);
}

Bool
AG10EDGAInit(ScreenPtr pScreen)
{
    ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
    AG10EPtr pAG10E = GET_AG10E_FROM_SCRN(pScrn);
    DGAModePtr mode;
    int result;
    
    mode = xnfcalloc(sizeof(DGAModeRec), 1);
    if (mode == NULL) {
        xf86Msg(X_WARNING, "%s: DGA setup failed, cannot allocate memory\n",
            pAG10E->psdp->device);
        return FALSE;
    }
    
    mode->mode = pScrn->modes;
    mode->flags = 0 /*DGA_PIXMAP_AVAILABLE | DGA_CONCURRENT_ACCESS*/;
#if 0
    if(!pAG10E->NoAccel) {
        mode->flags |= DGA_FILL_RECT | DGA_BLIT_RECT;
    }
#endif

    mode->imageWidth = mode->pixmapWidth = mode->viewportWidth =
	pScrn->virtualX;
    mode->imageHeight = mode->pixmapHeight = mode->viewportHeight =
	pScrn->virtualY;

    mode->bytesPerScanline = mode->imageWidth;

    mode->byteOrder = pScrn->imageByteOrder;
    mode->depth = 24;
    mode->bitsPerPixel = 32;
    mode->red_mask = pScrn->mask.red;
    mode->green_mask = pScrn->mask.green;
    mode->blue_mask = pScrn->mask.blue;
    
    mode->visualClass = TrueColor;
    mode->address = pAG10E->fb;

    result = DGAInit(pScreen, &AG10E_DGAFuncs, mode, 1);

    if (result) {
    	xf86Msg(X_INFO, "%s: DGA initialized\n",
            pAG10E->psdp->device);
    } else {
     	xf86Msg(X_WARNING, "%s: DGA setup failed\n",
            pAG10E->psdp->device);
    }       
}

static Bool 
AG10E_OpenFramebuffer(ScrnInfoPtr pScrn, char **name,
				unsigned int *mem,
				unsigned int *size, unsigned int *offset,
				unsigned int *extra)
{
    AG10EPtr pAG10E = GET_AG10E_FROM_SCRN(pScrn);

    *name = pAG10E->psdp->device;

    *mem = 0;
    *size = pAG10E->vidmem;
    *offset = 0;
    *extra = 0;

    return TRUE;
}

static Bool
AG10E_SetMode(ScrnInfoPtr pScrn, DGAModePtr pMode)
{
    /*
     * Nothing to do, we currently only support one mode
     * and we are always in it.
     */
    return TRUE;
}

static void
AG10E_SetViewport(ScrnInfoPtr pScrn, int x, int y, int flags)
{
     /* We don't support viewports, so... */
}

static int
AG10E_GetViewport(ScrnInfoPtr pScrn)
{
    /* No viewports, none pending... */
    return 0;
}

static void
AG10E_FillRect(ScrnInfoPtr pScrn, int x, int y, int w, int h, unsigned long color)
{
#if 0
    Cg6SetupForSolidFill(pScrn, color, GXset, 8);
    Cg6SubsequentSolidFillRect(pScrn, x, y, w, h);
#endif
}

static void
AG10E_BlitRect(ScrnInfoPtr pScrn, int srcx, int srcy,
			 int w, int h, int dstx, int dsty)
{
#if 0
    Cg6SetupForScreenToScreenCopy(pScrn, 0, 0, GXcopy, 8, 0);
    Cg6SubsequentScreenToScreenCopy(pScrn, srcx, srcy, dstx, dsty, w, h);
#endif
}

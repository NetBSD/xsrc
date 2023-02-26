/*
 * Copyright 1992-2003 by Alan Hourihane, North Wales, UK.
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without
 * fee, provided that the above copyright notice appear in all copies
 * and that both that copyright notice and this permission notice
 * appear in supporting documentation, and that the name of Alan
 * Hourihane not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior
 * permission.  Alan Hourihane makes no representations about the
 * suitability of this software for any purpose.  It is provided
 * "as is" without express or implied warranty.
 *
 * ALAN HOURIHANE DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS
 * SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS, IN NO EVENT SHALL ALAN HOURIHANE BE LIABLE FOR ANY
 * SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN
 * AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING
 * OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 *
 * Authors:  Alan Hourihane, <alanh@fairlite.demon.co.uk>
 *
 * CyberBladeXP4 accelerated options.
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "xf86.h"
#include "xf86_OSproc.h"

#include "xf86Pci.h"

#include "miline.h"

#include "trident.h"
#include "trident_regs.h"

#ifdef HAVE_XAA_H
#include "xaarop.h"

static void XP4Sync(ScrnInfoPtr pScrn);
#if 0
static void XP4SetupForDashedLine(ScrnInfoPtr pScrn,
                                    int fg, int bg,
                                    int rop,
                                    unsigned int planemask,
                                    int length,
                                    unsigned char *pattern);
static void XP4SubsequentDashedBresenhamLine(ScrnInfoPtr pScrn,
                                                int x, int y,
                                                int dmaj, int dmin,
                                                int e, int len,
                                                int octant,
                                                int phase);
static void XP4SetupForSolidLine(ScrnInfoPtr pScrn,
                                    int color,
                                    int rop,
                                    unsigned int planemask);
static void XP4SubsequentSolidBresenhamLine(ScrnInfoPtr pScrn,
                                            int x, int y,
                                            int dmaj, int dmin,
                                            int e, int len,
                                            int octant);
static void XP4SubsequentSolidHorVertLine(ScrnInfoPtr pScrn,
                                            int x, int y,
                                            int len, int dir);
#endif
static void XP4SetupForFillRectSolid(ScrnInfoPtr pScrn,
                                        int color,
                                        int rop,
                                        unsigned int planemask);
static void XP4SubsequentFillRectSolid(ScrnInfoPtr pScrn,
                                        int x, int y,
                                        int w, int h);
static void XP4SubsequentScreenToScreenCopy(ScrnInfoPtr pScrn,
                                            int x1, int y1,
                                            int x2, int y2,
                                            int w, int h);
static void XP4SetupForScreenToScreenCopy(ScrnInfoPtr pScrn,
                                            int xdir, int ydir,
                                            int rop,
                                            unsigned int planemask,
                                            int transparency_color);
static void XP4SetupForMono8x8PatternFill(ScrnInfoPtr pScrn,
                                            int patternx,
                                            int patterny,
                                            int fg, int bg,
                                            int rop,
                                            unsigned int planemask);
static void XP4SubsequentMono8x8PatternFillRect(ScrnInfoPtr pScrn,
                                                int patternx,
                                                int patterny,
                                                int x, int y,
                                                int w, int h);
#if 0
static void XP4SetupForCPUToScreenColorExpandFill(ScrnInfoPtr pScrn,
                                            int fg, int bg,
                                            int rop,
                                            unsigned int planemask);
static void XP4SubsequentCPUToScreenColorExpandFill(ScrnInfoPtr pScrn,
                                                    int x, int y,
                                                    int w, int h,
                                                    int skipleft);
#endif

static int bpp;
static int ropcode;

static void
XP4InitializeAccelerator(ScrnInfoPtr pScrn)
{
    TRIDENTPtr pTrident = TRIDENTPTR(pScrn);
    int shift;

    /* This forces updating the clipper */
    pTrident->Clipping = TRUE;

    CHECKCLIPPING;

    switch (pScrn->bitsPerPixel) {
    case 8:
    default: /* Muffle compiler */
        shift = 18;
        break;
    case 16:
        shift = 19;
        break;
    case 32:
        shift = 20;
        break;
    }

    switch (pScrn->bitsPerPixel) {
    case 8:
        bpp = 0x40;
        break;
    case 16:
        bpp = 0x41;
        break;
    case 32:
        bpp = 0x42;
        break;
    }

    MMIO_OUT32(pTrident->IOBase, 0x2154,
                                    (pScrn->displayWidth) << shift);
    MMIO_OUT32(pTrident->IOBase, 0x2150,
                                    (pScrn->displayWidth) << shift);
}
#endif

Bool XP4XaaInit(ScreenPtr pScreen)
{
#ifdef HAVE_XAA_H
    XAAInfoRecPtr infoPtr;
    ScrnInfoPtr pScrn = xf86ScreenToScrn(pScreen);
    TRIDENTPtr pTrident = TRIDENTPTR(pScrn);

    if (pTrident->NoAccel)
    return FALSE;

    pTrident->AccelInfoRec = infoPtr = XAACreateInfoRec();
    if (!infoPtr) return FALSE;

    infoPtr->Flags = PIXMAP_CACHE | OFFSCREEN_PIXMAPS |
                                    LINEAR_FRAMEBUFFER;

    pTrident->InitializeAccelerator = XP4InitializeAccelerator;
    XP4InitializeAccelerator(pScrn);

    infoPtr->Sync = XP4Sync;

#if 0 /* TO DO for the XP */
    infoPtr->SolidLineFlags = NO_PLANEMASK;
    infoPtr->SetupForSolidLine = XP4SetupForSolidLine;
    infoPtr->SolidBresenhamLineErrorTermBits = 12;
    infoPtr->SubsequentSolidBresenhamLine =
                                    XP4SubsequentSolidBresenhamLine;

    infoPtr->DashedLineFlags = LINE_PATTERN_MSBFIRST_LSBJUSTIFIED |
                                NO_PLANEMASK |
                                LINE_PATTERN_POWER_OF_2_ONLY;
    infoPtr->SetupForDashedLine = XP4SetupForDashedLine;
    infoPtr->DashedBresenhamLineErrorTermBits = 12;
    infoPtr->SubsequentDashedBresenhamLine =
                                    XP4SubsequentDashedBresenhamLine;
    infoPtr->DashPatternMaxLength = 16;
#endif

    infoPtr->SolidFillFlags = NO_PLANEMASK;
    infoPtr->SetupForSolidFill = XP4SetupForFillRectSolid;
    infoPtr->SubsequentSolidFillRect = XP4SubsequentFillRectSolid;
#if 0
    infoPtr->SubsequentSolidHorVertLine = XP4SubsequentSolidHorVertLine;
#endif

    infoPtr->ScreenToScreenCopyFlags = NO_PLANEMASK | NO_TRANSPARENCY;

    infoPtr->SetupForScreenToScreenCopy =
                                        XP4SetupForScreenToScreenCopy;
    infoPtr->SubsequentScreenToScreenCopy =
                                    XP4SubsequentScreenToScreenCopy;

    infoPtr->Mono8x8PatternFillFlags = NO_PLANEMASK |
                                    HARDWARE_PATTERN_PROGRAMMED_BITS |
                                    HARDWARE_PATTERN_SCREEN_ORIGIN |
                                    BIT_ORDER_IN_BYTE_MSBFIRST;

    infoPtr->SetupForMono8x8PatternFill =
                                        XP4SetupForMono8x8PatternFill;
    infoPtr->SubsequentMono8x8PatternFillRect =
                                XP4SubsequentMono8x8PatternFillRect;

#if 0
    infoPtr->CPUToScreenColorExpandFillFlags = NO_PLANEMASK |
                                        BIT_ORDER_IN_BYTE_MSBFIRST;
    infoPtr->ColorExpandBase = pTrident->D3Base;
    infoPtr->ColorExpandRange = pScrn->displayWidth;

    infoPtr->SetupForCPUToScreenColorExpandFill =
                                XP4SetupForCPUToScreenColorExpandFill;
    infoPtr->SubsequentCPUToScreenColorExpandFill =
                            XP4SubsequentCPUToScreenColorExpandFill;
#endif

    return(XAAInit(pScreen, infoPtr));
#else
    return FALSE;
#endif
}

#ifdef HAVE_XAA_H
static void
XP4Sync(ScrnInfoPtr pScrn)
{
    TRIDENTPtr pTrident = TRIDENTPTR(pScrn);
    int count = 0, timeout = 0;
    int busy;

    for (;;) {
        BLTBUSY(busy);
        if (busy != GE_BUSY) {
            return;
        }

        count++;
        if (count == 10000000) {
            ErrorF("XP: BitBLT engine time-out.\n");
            count = 9990000;
            timeout++;
            if (timeout == 4) {
                /* Reset BitBLT Engine */
                TGUI_STATUS(0x00);
                return;
            }
        }
    }
}

static void
XP4SetupForScreenToScreenCopy(ScrnInfoPtr pScrn,
                                int xdir, int ydir,
                                int rop,
                                unsigned int planemask,
                                int transparency_color)
{
    TRIDENTPtr pTrident = TRIDENTPTR(pScrn);
    int dst = 0;

    pTrident->BltScanDirection = 0;
    if (xdir < 0) pTrident->BltScanDirection |= XNEG;
    if (ydir < 0) pTrident->BltScanDirection |= YNEG;

    REPLICATE(transparency_color);
    if (transparency_color != -1) {
        dst |= 3<<16;
        MMIO_OUT32(pTrident->IOBase, 0x2134, transparency_color);
    }

    ropcode = rop;

    MMIO_OUT32(pTrident->IOBase, 0x2128,
                                pTrident->BltScanDirection | SCR2SCR);
}

static void
XP4SubsequentScreenToScreenCopy(ScrnInfoPtr pScrn,
                                int x1, int y1,
                                int x2, int y2,
                                int w, int h)
{
    TRIDENTPtr pTrident = TRIDENTPTR(pScrn);

    if (pTrident->BltScanDirection & YNEG) {
        y1 = y1 + h - 1;
        y2 = y2 + h - 1;
    }
    if (pTrident->BltScanDirection & XNEG) {
        x1 = x1 + w - 1;
        x2 = x2 + w - 1;
    }
    MMIO_OUT32(pTrident->IOBase, 0x2138, (x2 << 16) | y2);
    MMIO_OUT32(pTrident->IOBase, 0x213C, (x1 << 16) | y1);
    MMIO_OUT32(pTrident->IOBase, 0x2140, (w << 16) | h);
    XP4Sync(pScrn);
    MMIO_OUT32(pTrident->IOBase, 0x2124,
                                    (XAAGetCopyROP(ropcode) << 24) |
                                    (bpp << 8) |
                                    1);
}

#if 0
static void
XP4SetupForSolidLine(ScrnInfoPtr pScrn,
                        int color,
                        int rop,
                        unsigned int planemask)
{
    TRIDENTPtr pTrident = TRIDENTPTR(pScrn);

    pTrident->BltScanDirection = 0;
    REPLICATE(color);
    TGUI_FMIX(XAAPatternROP[rop]);
    if (pTrident->Chipset >= PROVIDIA9685) {
        TGUI_FPATCOL(color);
    } else {
        TGUI_FCOLOUR(color);
    }
}

static void
XP4SubsequentSolidBresenhamLine(ScrnInfoPtr pScrn,
                                int x, int y,
                                int dmaj, int dmin,
                                int e, int len, int octant)
{
    TRIDENTPtr pTrident = TRIDENTPTR(pScrn);
    int tmp = pTrident->BltScanDirection;

    if (octant & YMAJOR) tmp |= YMAJ;
    if (octant & XDECREASING) tmp |= XNEG;
    if (octant & YDECREASING) tmp |= YNEG;
    TGUI_DRAWFLAG(SOLIDFILL | STENCIL | tmp);
    XP_SRC_XY(dmin - dmaj, dmin);
    XP_DEST_XY(x, y);
    XP_DIM_XY(dmin + e, len);
    TGUI_COMMAND(GE_BRESLINE);
    XP4Sync(pScrn);
}

static void
XP4SubsequentSolidHorVertLine(ScrnInfoPtr pScrn,
                                int x, int y,
                                int len, int dir)
{
    TRIDENTPtr pTrident = TRIDENTPTR(pScrn);

    TGUI_DRAWFLAG(SOLIDFILL);
    if (dir == DEGREES_0) {
        XP_DIM_XY(len, 1);
        XP_DEST_XY(x, y);
    } else {
        XP_DIM_XY(1, len);
        XP_DEST_XY(x, y);
    }

    TGUI_COMMAND(GE_BLT);
    XP4Sync(pScrn);
}

void
XP4SetupForDashedLine(ScrnInfoPtr pScrn,
                        int fg, int bg,
                        int rop,
                        unsigned int planemask,
                        int length,
                        unsigned char *pattern)
{
    TRIDENTPtr pTrident = TRIDENTPTR(pScrn);
    CARD32 *DashPattern = (CARD32*)pattern;
    CARD32 NiceDashPattern = DashPattern[0];

    NiceDashPattern = *((CARD16 *)pattern) & ((1 << length) - 1);
    switch(length) {
        case 2: NiceDashPattern |= NiceDashPattern << 2;
        case 4: NiceDashPattern |= NiceDashPattern << 4;
        case 8: NiceDashPattern |= NiceDashPattern << 8;
    }
    pTrident->BltScanDirection = 0;
    REPLICATE(fg);
    if (pTrident->Chipset >= PROVIDIA9685) {
        TGUI_FPATCOL(fg);
        if (bg == -1) {
            pTrident->BltScanDirection |= (1 << 12);
            TGUI_BPATCOL(~fg);
        } else {
            REPLICATE(bg);
            TGUI_BPATCOL(bg);
        }
    } else {
        TGUI_FCOLOUR(fg);
        if (bg == -1) {
            pTrident->BltScanDirection |= (1 << 12);
            TGUI_BCOLOUR(~fg);
        } else {
            REPLICATE(bg);
            TGUI_BCOLOUR(bg);
        }
    }

    TGUI_FMIX(XAAPatternROP[rop]);
    pTrident->LinePattern = NiceDashPattern;
}

void
XP4SubsequentDashedBresenhamLine(ScrnInfoPtr pScrn,
                                    int x, int y,
                                    int dmaj, int dmin,
                                    int e, int len,
                                    int octant, int phase)
{
    TRIDENTPtr pTrident = TRIDENTPTR(pScrn);
    int tmp = pTrident->BltScanDirection;

    if (octant & YMAJOR) tmp |= YMAJ;
    if (octant & XDECREASING) tmp |= XNEG;
    if (octant & YDECREASING) tmp |= YNEG;

    TGUI_STYLE(((pTrident->LinePattern >> phase) |
                (pTrident->LinePattern << (16 - phase))) & 0x0000FFFF);
    TGUI_DRAWFLAG(STENCIL | tmp);
    XP_SRC_XY(dmin - dmaj, dmin);
    XP_DEST_XY(x, y);
    XP_DIM_XY(e + dmin, len);
    TGUI_COMMAND(GE_BRESLINE);
    XP4Sync(pScrn);
}
#endif

static void
XP4SetupForFillRectSolid(ScrnInfoPtr pScrn,
                            int color,
                            int rop,
                            unsigned int planemask)
{
    TRIDENTPtr pTrident = TRIDENTPTR(pScrn);

    ropcode = rop;

    REPLICATE(color);
    MMIO_OUT32(pTrident->IOBase, 0x2158, color);
    MMIO_OUT32(pTrident->IOBase, 0x2128, 1 << 14);
}

static void
XP4SubsequentFillRectSolid(ScrnInfoPtr pScrn,
                            int x, int y,
                            int w, int h)
{
    TRIDENTPtr pTrident = TRIDENTPTR(pScrn);

    MMIO_OUT32(pTrident->IOBase, 0x2138, (x << 16) | y);
    MMIO_OUT32(pTrident->IOBase, 0x2140, (w << 16) | h);
    XP4Sync(pScrn);
    MMIO_OUT32(pTrident->IOBase, 0x2124,
                            (XAAGetPatternROP(ropcode) << 24) |
                            (bpp << 8) |
                            2);
}

#if 0
static void
MoveDWORDS(register CARD32* dest,
            register CARD32* src,
            register int dwords)
{
    while(dwords & ~0x03) {
        *dest = *src;
        *(dest + 1) = *(src + 1);
        *(dest + 2) = *(src + 2);
        *(dest + 3) = *(src + 3);
        src += 4;
        dest += 4;
        dwords -= 4;
    }

    if (!dwords) return;
    *dest = *src;
    dest += 1;
    src += 1;
    if (dwords == 1) return;
    *dest = *src;
    dest += 1;
    src += 1;
    if (dwords == 2) return;
    *dest = *src;
    dest += 1;
    src += 1;
}
#endif

#if 0
static void
MoveDWORDS_FixedBase(register CARD32* dest,
                        register CARD32* src,
                        register int dwords)
{
    while(dwords & ~0x03) {
        *dest = *src;
        *dest = *(src + 1);
        *dest = *(src + 2);
        *dest = *(src + 3);
        dwords -= 4;
        src += 4;
    }

    if(!dwords) return;
    *dest = *src;
    if(dwords == 1) return;
    *dest = *(src + 1);
    if(dwords == 2) return;
    *dest = *(src + 2);
}
#endif

static void
XP4SetupForMono8x8PatternFill(ScrnInfoPtr pScrn,
                                int patternx, int patterny,
                                int fg, int bg,
                                int rop,
                                unsigned int planemask)
{
    TRIDENTPtr pTrident = TRIDENTPTR(pScrn);
    int drawflag = 0;

    REPLICATE(fg);
    MMIO_OUT32(pTrident->IOBase, 0x2158, fg);

    if (bg == -1) {
        drawflag |= (1 << 12);
        MMIO_OUT32(pTrident->IOBase, 0x215C, ~fg);
    } else {
        REPLICATE(bg);
        MMIO_OUT32(pTrident->IOBase, 0x215C, bg);
    }

    ropcode = rop;

    drawflag |= (7 << 18);
    TGUI_DRAWFLAG(PATMONO | drawflag);
}

static void
XP4SubsequentMono8x8PatternFillRect(ScrnInfoPtr pScrn,
                                    int patternx, int patterny,
                                    int x, int y,
                                    int w, int h)
{
    TRIDENTPtr pTrident = TRIDENTPTR(pScrn);

    MMIO_OUT32(pTrident->IOBase, 0x2180, patternx);
    MMIO_OUT32(pTrident->IOBase, 0x2184, patterny);
    MMIO_OUT32(pTrident->IOBase, 0x2138, (x << 16) | y);
    MMIO_OUT32(pTrident->IOBase, 0x2140, (w << 16) | h);
    XP4Sync(pScrn);
    MMIO_OUT32(pTrident->IOBase, 0x2124,
                                (XAAGetPatternROP(ropcode) << 24) |
                                (bpp << 8) |
                                2);
}

#if 0
static void
XP4SetupForCPUToScreenColorExpandFill(ScrnInfoPtr pScrn,
                                        int fg, int bg,
                                        int rop,
                                        unsigned int planemask)
{
    TRIDENTPtr pTrident = TRIDENTPTR(pScrn);

    ropcode = XAAGetCopyROP(rop);
#if 0
    TGUI_FMIX(XAACopyROP[rop]);
#endif
    if (bg == -1) {
        TGUI_DRAWFLAG(SRCMONO | (1 << 12));
        REPLICATE(fg);
        TGUI_FCOLOUR(fg);
    } else {
        TGUI_DRAWFLAG(SRCMONO);
        REPLICATE(fg);
        REPLICATE(bg);
        TGUI_FCOLOUR(fg);
        TGUI_BCOLOUR(bg);
    }
}

static void
XP4SubsequentCPUToScreenColorExpandFill(ScrnInfoPtr pScrn,
                                        int x, int y,
                                        int w, int h,
                                        int skipleft)
{
    TRIDENTPtr pTrident = TRIDENTPTR(pScrn);

    MMIO_OUT32(pTrident->IOBase, 0x2138, (x << 16) | y);
    MMIO_OUT32(pTrident->IOBase, 0x2140, (w << 16) | h);
    XP4Sync(pScrn);
    MMIO_OUT32(pTrident->IOBase, 0x2124, (ropcode << 24) |
                                            (bpp << 8) |
                                            2);
}
#endif
#endif

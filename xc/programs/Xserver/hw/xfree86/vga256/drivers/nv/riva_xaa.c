 /***************************************************************************\
|*                                                                           *|
|*       Copyright 1993-1999 NVIDIA, Corporation.  All rights reserved.      *|
|*                                                                           *|
|*     NOTICE TO USER:   The source code  is copyrighted under  U.S. and     *|
|*     international laws.  Users and possessors of this source code are     *|
|*     hereby granted a nonexclusive,  royalty-free copyright license to     *|
|*     use this code in individual and commercial software.                  *|
|*                                                                           *|
|*     Any use of this source code must include,  in the user documenta-     *|
|*     tion and  internal comments to the code,  notices to the end user     *|
|*     as follows:                                                           *|
|*                                                                           *|
|*       Copyright 1993-1999 NVIDIA, Corporation.  All rights reserved.      *|
|*                                                                           *|
|*     NVIDIA, CORPORATION MAKES NO REPRESENTATION ABOUT THE SUITABILITY     *|
|*     OF  THIS SOURCE  CODE  FOR ANY PURPOSE.  IT IS  PROVIDED  "AS IS"     *|
|*     WITHOUT EXPRESS OR IMPLIED WARRANTY OF ANY KIND.  NVIDIA, CORPOR-     *|
|*     ATION DISCLAIMS ALL WARRANTIES  WITH REGARD  TO THIS SOURCE CODE,     *|
|*     INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY, NONINFRINGE-     *|
|*     MENT,  AND FITNESS  FOR A PARTICULAR PURPOSE.   IN NO EVENT SHALL     *|
|*     NVIDIA, CORPORATION  BE LIABLE FOR ANY SPECIAL,  INDIRECT,  INCI-     *|
|*     DENTAL, OR CONSEQUENTIAL DAMAGES,  OR ANY DAMAGES  WHATSOEVER RE-     *|
|*     SULTING FROM LOSS OF USE,  DATA OR PROFITS,  WHETHER IN AN ACTION     *|
|*     OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,  ARISING OUT OF     *|
|*     OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOURCE CODE.     *|
|*                                                                           *|
|*     U.S. Government  End  Users.   This source code  is a "commercial     *|
|*     item,"  as that  term is  defined at  48 C.F.R. 2.101 (OCT 1995),     *|
|*     consisting  of "commercial  computer  software"  and  "commercial     *|
|*     computer  software  documentation,"  as such  terms  are  used in     *|
|*     48 C.F.R. 12.212 (SEPT 1995)  and is provided to the U.S. Govern-     *|
|*     ment only as  a commercial end item.   Consistent with  48 C.F.R.     *|
|*     12.212 and  48 C.F.R. 227.7202-1 through  227.7202-4 (JUNE 1995),     *|
|*     all U.S. Government End Users  acquire the source code  with only     *|
|*     those rights set forth herein.                                        *|
|*                                                                           *|
 \***************************************************************************/
/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/nv/riva_xaa.c,v 1.1.2.9 2000/01/08 03:25:41 robin Exp $ */
/*
 * Based initially on the NV1, NV3 code by Dave McKay.
 *
 * Copyright 1996-1997  David J. McKay
 */
#include <math.h>
#include <stdlib.h>
#include "X.h"
#include "Xproto.h"
#include "misc.h"
#include "input.h"
#include "screenint.h"
#include "cursorstr.h"
#include "regionstr.h"
#include "scrnintstr.h"
#include "servermd.h"
#include "windowstr.h"
#include "compiler.h"
#include "vga256.h"
#include "mipointer.h"
#include "xf86.h"
#include "xf86Priv.h"
#include "xf86_Option.h"
#include "xf86_OSlib.h"
#include "xf86_HWlib.h"
#include "xf86xaa.h"
#include "vga.h"
#include "vgaPCI.h"
#define XCONFIG_FLAGS_ONLY
#include "xf86_Config.h"
#ifdef XFreeXDGA
#include "X.h"
#include "Xproto.h"
#include "scrnintstr.h"
#include "servermd.h"
#define _XF86DGA_SERVER_
#include "extensions/xf86dgastr.h"
#endif
/*
 * RIVA and NVIDIA specific includes.
 */
#include "riva_hw.h"
#include "nvreg.h"
#include "nvvga.h"
/*
 * Very useful macros that allows you to set overflow bits
 */
#define SetBitField(value,from,to) SetBF(to,GetBF(value,from))
#define SetBit(n) (1<<(n))
#define Set8Bits(value) ((value)&0xff)
/*
 * Macro to define valid rectangle.
 */
#define RIVA_RECT_VALID(rr)  (((rr).x1 < (rr).x2) && ((rr).y1 < (rr).y2))
/*
 * RIVA hardware instance structure.
 */
RIVA_HW_INST riva;
/*
 * Buffers.
 */
#define RIVA_FRONT_BUFFER       0
#define RIVA_BACK_BUFFER        1
#define RIVA_DEPTH_BUFFER       2
#define RIVA_TEXTURE_BUFFER     3
#define RIVA_CACHE_BUFFER       4
unsigned rivaBufferOffset[5] = {0, 0, 0, 0, 0};

/****************************************************************************\
*                                                                            *
*                        XAA 2D Accelation Entrypoints                       *
*                                                                            *
\****************************************************************************/

/*
 * 2D render flag for 3D code.
 */
int rivaRendered2D = 0;
/*
 * Opaque monochrome bits.
 */
unsigned int rivaOpaqueMonochrome;
/*
 * Set scissor clip rect.  Internal routine.
 */
static void RivaSetClippingRectangle(int x1, int y1, int x2, int y2)
{
    int height = y2-y1 + 1;
    int width  = x2-x1 + 1;

    rivaRendered2D = 1;
    RIVA_FIFO_FREE(riva, Clip, 2);
    riva.Clip->TopLeft     = (y1     << 16) | x1;
    riva.Clip->WidthHeight = (height << 16) | width;
}
/*
 * Set pattern. Internal routine. The upper bits of the colors
 * are the ALPHA bits.  0 == transparency.
 */
static void RivaSetPattern(int clr0, int clr1, int pat0, int pat1)
{
    rivaRendered2D = 1;
    RIVA_FIFO_FREE(riva, Patt, 5);
    riva.Patt->Shape         = 0; /* 0 = 8X8, 1 = 64X1, 2 = 1X64 */
    riva.Patt->Color0        = clr0;
    riva.Patt->Color1        = clr1;
    riva.Patt->Monochrome[0] = pat0;
    riva.Patt->Monochrome[1] = pat1;
}
/*
 * Set ROP.  Translate X rop into ROP3.  Internal routine.
 */
static int currentRop = -1;
static void RivaSetRopSolid(int rop)
{
    static int ropTrans[16] = 
    {
        0x00, /* GXclear        */
        0x88, /* Gxand          */
        0x44, /* GXandReverse   */
        0xCC, /* GXcopy         */
        0x22, /* GXandInverted  */
        0xAA, /* GXnoop         */
        0x66, /* GXxor          */
        0xEE, /* GXor           */
        0x11, /* GXnor          */
        0x99, /* GXequiv        */
        0x55, /* GXinvert       */
        0xDD, /* GXorReverse    */
        0x33, /* GXcopyInverted */
        0xBB, /* GXorInverted   */
        0x77, /* GXnand         */
        0xFF  /* GXset          */
    };
    
    if (currentRop != rop)
    {
        if (currentRop > 16)
            RivaSetPattern(0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF);
        currentRop     = rop;
        rivaRendered2D = 1;
        RIVA_FIFO_FREE(riva, Rop, 1);
        riva.Rop->Rop3 = ropTrans[rop];
    }
}
static void RivaSetRopPattern(int rop)
{
    static int ropTrans[16] = 
    {
        0x00, /* GXclear        */
        0xA0, /* Gxand          */
        0x0A, /* GXandReverse   */
        0xF0, /* GXcopy         */
        0x30, /* GXandInverted  */
        0xAA, /* GXnoop         */
        0x3A, /* GXxor          */
        0xFA, /* GXor           */
        0x03, /* GXnor          */
        0xA0, /* Gxequiv        */
        0x0F, /* GXinvert       */
        0xAF, /* GXorReverse    */
        0x33, /* GXcopyInverted */
        0xBB, /* GXorInverted   */
        0xF3, /* GXnand         */
        0xFF  /* GXset          */
    };
    
    if (currentRop != rop + 16)
    {
        currentRop     = rop + 16; /* +16 is important */
        rivaRendered2D = 1;
        RIVA_FIFO_FREE(riva, Rop, 1);
        riva.Rop->Rop3 = ropTrans[rop];
    }
}
/*
 * Fill solid rectangles.
 */                                           
static void RivaSetupForFillRectSolid(int color, int rop, unsigned planemask)
{
    RivaSetRopSolid(rop);
    rivaRendered2D = 1;
    RIVA_FIFO_FREE(riva, Bitmap, 1);
    riva.Bitmap->Color1A = color;
}
static void RivaSubsequentFillRectSolid(int x, int y, int w, int h)
{
    RIVA_FIFO_FREE(riva, Bitmap, 2);
    riva.Bitmap->UnclippedRectangle[0].TopLeft     = (x << 16) | y;
    riva.Bitmap->UnclippedRectangle[0].WidthHeight = (w << 16) | h;
}
/*
 * Fill 8x8 monochrome pattern rectangles.  patternx and patterny are
 * the overloaded pattern bits themselves. The pattern colors don't
 * support 565, only 555. Hack around it.
 */                                           
static void RivaSetupFor8x8PatternColorExpand(int patternx, int patterny, int bg, int fg, int rop, unsigned planemask)
{
    RivaSetRopPattern(rop);
    rivaRendered2D = 1;
    fg |= rivaOpaqueMonochrome;
    bg  = (bg == -1) ? 0 : bg | rivaOpaqueMonochrome;
    RivaSetPattern(bg, fg, patternx, patterny);
    RIVA_FIFO_FREE(riva, Bitmap, 1);
    riva.Bitmap->Color1A = fg;
}
static void RivaSubsequent8x8PatternColorExpand(int patternx, int patterny, int x, int y, int w, int h)
{
    RIVA_FIFO_FREE(riva, Bitmap, 2);
    riva.Bitmap->UnclippedRectangle[0].TopLeft     = (x << 16) | y;
    riva.Bitmap->UnclippedRectangle[0].WidthHeight = (w << 16) | h;
}
/*
 * Screen to screen BLTs.
 */
static void RivaSetupForScreenToScreenCopy(int xdir, int ydir, int rop, unsigned planemask, int transparency_color)
{
    rivaRendered2D = 1;
    RivaSetRopSolid(rop);
}
static void RivaSubsequentScreenToScreenCopy(int x1, int y1, int x2, int y2, int w, int h)
{
    RIVA_FIFO_FREE(riva, Blt, 3);
    riva.Blt->TopLeftSrc  = (y1 << 16) | x1;
    riva.Blt->TopLeftDst  = (y2 << 16) | x2;
    riva.Blt->WidthHeight = (h  << 16) | w;
}
/*
 * Individual glyph routines.
 */
static void RivaTransparentGlyph(int x, int y, int w, int h, unsigned long *pbits)
{
    int i, padHeight, padWidth;

    while (riva.Busy(&riva));
    RIVA_FIFO_FREE(riva, Bitmap, 3);
    if (w <= 8)
    {
        padHeight = (h + 3) >> 2;
        riva.Bitmap->WidthHeightInD  = (padHeight << (16 + 2)) | 8;
        riva.Bitmap->WidthHeightOutD = (h         <<  16)      | w;
        riva.Bitmap->PointD          = (y         <<  16)      | (x & 0xFFFF);
        while (padHeight--)
        {
            RIVA_FIFO_FREE(riva, Bitmap, 1);
            riva.Bitmap->MonochromeData1D = (pbits[0])
                                          | (pbits[1] << 8)
                                          | (pbits[2] << 16)
                                          | (pbits[3] << 24);
            pbits += 4;
        }
    }
    else if (w <= 16)
    {
        padHeight = (h + 1) >> 1;
        riva.Bitmap->WidthHeightInD  = (padHeight << (16 + 1)) | 16;
        riva.Bitmap->WidthHeightOutD = (h         <<  16)      | w;
        riva.Bitmap->PointD          = (y         <<  16)      | (x & 0xFFFF);
        while (padHeight--)
        {
            RIVA_FIFO_FREE(riva, Bitmap, 1);
            riva.Bitmap->MonochromeData1D = (pbits[0])
                                          | (pbits[1] << 16);
            pbits += 2;
        }
    }
    else
    {
        padWidth = (w + 31) >> 5;
        riva.Bitmap->WidthHeightInD  = (h << 16) | (padWidth << 5);
        riva.Bitmap->WidthHeightOutD = (h << 16) | w;
        riva.Bitmap->PointD          = (y << 16) | (x & 0xFFFF);
        while (h--)
            for (i = 0; i < padWidth; i++)
            {
                RIVA_FIFO_FREE(riva, Bitmap, 1);
                riva.Bitmap->MonochromeData1D = *pbits++;
            }
    }
}
static void RivaOpaqueGlyph(int x, int y, int w, int h, unsigned long *pbits)
{
    int i, padHeight, padWidth;

    while (riva.Busy(&riva));
    RIVA_FIFO_FREE(riva, Bitmap, 3);
    if (w <= 8)
    {
        padHeight = (h + 3) >> 2;
        riva.Bitmap->WidthHeightInE  = (padHeight << (16 + 2)) | 8;
        riva.Bitmap->WidthHeightOutE = (h         <<  16)      | w;
        riva.Bitmap->PointE          = (y         <<  16)      | (x & 0xFFFF);
        while (padHeight--)
        {
            RIVA_FIFO_FREE(riva, Bitmap, 1);
            riva.Bitmap->MonochromeData01E = (pbits[0])
                                           | (pbits[1] << 8)
                                           | (pbits[2] << 16)
                                           | (pbits[3] << 24);
            pbits += 4;
        }
    }
    else if (w <= 16)
    {
        padHeight = (h + 1) >> 1;
        riva.Bitmap->WidthHeightInE  = (padHeight << (16 + 1)) | 16;
        riva.Bitmap->WidthHeightOutE = (h         <<  16)      | w;
        riva.Bitmap->PointE          = (y         <<  16)      | (x & 0xFFFF);
        while (padHeight--)
        {
            RIVA_FIFO_FREE(riva, Bitmap, 1);
            riva.Bitmap->MonochromeData01E = (pbits[0])
                                           | (pbits[1] << 16);
            pbits += 2;
        }
    }
    else
    {
        padWidth = (w + 31) >> 5;
        riva.Bitmap->WidthHeightInE  = (h << 16) | (padWidth << 5);
        riva.Bitmap->WidthHeightOutE = (h << 16) | w;
        riva.Bitmap->PointE          = (y << 16) | (x & 0xFFFF);
        while (h--)
            for (i = 0; i < padWidth; i++)
            {
                RIVA_FIFO_FREE(riva, Bitmap, 1);
                riva.Bitmap->MonochromeData01E = *pbits++;
            }
    }
}
/*
 * Terminal Emulator font routines.
 */
static void RivaPolyGlyphBltTE(DrawablePtr pDrawable, GCPtr pGC, int xInit, int yInit, int nglyph, CharInfoPtr *ppci, unsigned char *pglyphBase)
{
    FontPtr   pfont = pGC->font;
    RegionPtr cclip;
    BoxPtr    pclipRect;
    BoxRec    glyphRect, intersectRect;
    int       x, y, i;
    int       glyphWidth, glyphHeight;
    int       nclip;

    /*
     * Sanity checks.
     */
    if (!xf86VTSema) return;
    if (pGC->fillStyle != FillSolid)
    {
        miPolyGlyphBlt(pDrawable, pGC, xInit, yInit, nglyph, ppci,pglyphBase);
        return;
    }
    /*
     * Get glyph extents.
     */
    glyphWidth  = FONTMAXBOUNDS(pfont, characterWidth);
    glyphHeight = FONTASCENT(pfont) + FONTDESCENT(pfont);
    if ((glyphHeight | glyphWidth) == 0) return;
    x = xInit + FONTMAXBOUNDS(pfont, leftSideBearing) + pDrawable->x;
    y = yInit - FONTASCENT(pfont)                     + pDrawable->y;
    glyphRect.x1 = x; glyphRect.x2 = x + (glyphWidth * nglyph) + 1;
    glyphRect.y1 = y; glyphRect.y2 = y + glyphHeight           + 1;
    /*
     * Get region clip rects.
     */
    cclip     = cfbGetCompositeClip(pGC);
    nclip     = REGION_NUM_RECTS(cclip);
    pclipRect = REGION_RECTS(cclip);
    /*
     * Render glyphs through clipping regions.
     */
    RivaSetRopSolid(pGC->alu);
    rivaRendered2D = 1;
    while (nclip--)
    {
        /*
         * Intersect glyph bounds with clip rect.
         */
        intersectRect.x1 = max(pclipRect->x1, glyphRect.x1);
        intersectRect.y1 = max(pclipRect->y1, glyphRect.y1);
        intersectRect.x2 = min(pclipRect->x2, glyphRect.x2);
        intersectRect.y2 = min(pclipRect->y2, glyphRect.y2);
        pclipRect++;
        /*
         * Only draw glyph string if visible through this region.
         */
        if (RIVA_RECT_VALID(intersectRect))
        {
            /*
             * Set clip rect.
             */
            RIVA_FIFO_FREE(riva, Bitmap, 3);
            riva.Bitmap->ClipD.TopLeft     = (intersectRect.y1 << 16) | intersectRect.x1;
            riva.Bitmap->ClipD.BottomRight = (intersectRect.y2 << 16) | intersectRect.x2;
            riva.Bitmap->Color1D           = pGC->fgPixel;
            /*
             * Render each glyph.
             */
            for (i = 0; i < nglyph; i++)
                RivaTransparentGlyph(x + i * glyphWidth, 
                                     y,
                                     glyphWidth,
                                     glyphHeight,
                                     (unsigned long *)(FONTGLYPHBITS(pglyphBase, ppci[i])));
        }
    }
}
static void RivaImageGlyphBltTE(DrawablePtr pDrawable, GCPtr pGC, int xInit, int yInit, int nglyph, CharInfoPtr *ppci, unsigned char *pglyphBase)
{
    FontPtr   pfont = pGC->font;
    RegionPtr cclip;
    BoxPtr    pclipRect;
    BoxRec    glyphRect, intersectRect;
    int       x, y, i, bg;
    int       glyphWidth, glyphHeight;
    int       nclip;

    /*
     * Sanity checks.
     */
    if (!xf86VTSema) return;
    /*
     * Get glyph extents.
     */
    glyphWidth  = FONTMAXBOUNDS(pfont, characterWidth);
    glyphHeight = FONTASCENT(pfont) + FONTDESCENT(pfont);
    if ((glyphHeight | glyphWidth) == 0) return;
    x = xInit + FONTMAXBOUNDS(pfont, leftSideBearing) + pDrawable->x;
    y = yInit - FONTASCENT(pfont)                     + pDrawable->y;
    glyphRect.x1 = x; glyphRect.x2 = x + (glyphWidth * nglyph);
    glyphRect.y1 = y; glyphRect.y2 = y + glyphHeight;
    /*
     * Get region clip rects.
     */
    cclip     = cfbGetCompositeClip(pGC);
    nclip     = REGION_NUM_RECTS(cclip);
    pclipRect = REGION_RECTS(cclip);
    /*
     * Render glyphs through clipping regions.
     */
    RivaSetRopSolid(GXcopy);
    rivaRendered2D = 1;
    bg = pGC->bgPixel | rivaOpaqueMonochrome;
    while (nclip--)
    {
        /*
         * Intersect glyph bounds with clip rect.
         */
        intersectRect.x1 = max(pclipRect->x1, glyphRect.x1);
        intersectRect.y1 = max(pclipRect->y1, glyphRect.y1);
        intersectRect.x2 = min(pclipRect->x2, glyphRect.x2);
        intersectRect.y2 = min(pclipRect->y2, glyphRect.y2);
        pclipRect++;
        /*
         * Only draw glyph string if visible through this region.
         */
        if (RIVA_RECT_VALID(intersectRect))
        {
            /*
             * Set clip rect.
             */
            RIVA_FIFO_FREE(riva, Bitmap, 4);
            riva.Bitmap->ClipE.TopLeft     = (intersectRect.y1 << 16) | intersectRect.x1;
            riva.Bitmap->ClipE.BottomRight = (intersectRect.y2 << 16) | intersectRect.x2;
            riva.Bitmap->Color0E           = bg;
            riva.Bitmap->Color1E           = pGC->fgPixel;
            /*
             * Render each glyph.
             */
            for (i = 0; i < nglyph; i++)
                RivaOpaqueGlyph(x + i * glyphWidth, 
                                y,
                                glyphWidth,
                                glyphHeight,
                                (unsigned long *)FONTGLYPHBITS(pglyphBase, ppci[i]));
        }
    }
}
/*
 * Normal Font routines.
 */
static void RivaPolyGlyphBltNonTE(DrawablePtr pDrawable, GCPtr pGC, int xInit, int yInit, int nglyph, CharInfoPtr *ppci, unsigned char *pglyphBase)
{
    FontPtr       pfont = pGC->font;
    RegionPtr     cclip;
    BoxPtr        pclipRect;
    BoxRec        glyphRect, intersectRect;
    int           x, y, i, dx;
    int           nclip;

    /*
     * Sanity checks.
     */
    if (!xf86VTSema) return;
    if (pGC->fillStyle != FillSolid)
    {
        miPolyGlyphBlt(pDrawable, pGC, xInit, yInit, nglyph, ppci,pglyphBase);
        return;
    }
    y = yInit + pDrawable->y;
    x = xInit + pDrawable->x;
    /*
     * Get glyph extents.
     */
    for (i = dx = 0; i < nglyph; i++)
    	dx += ppci[i]->metrics.characterWidth;
    if (dx >= 0)
    {
        glyphRect.x1 = x; glyphRect.x2 = x + dx + 1;
    }
    else
    {
        glyphRect.x1 = x + dx; glyphRect.x2 = x + 1;
    }
    glyphRect.y1 = y - FONTASCENT(pfont); glyphRect.y2 = y + FONTDESCENT(pfont) + 1;
    /*
     * Get region clip rects.
     */
    cclip     = cfbGetCompositeClip(pGC);
    nclip     = REGION_NUM_RECTS(cclip);
    pclipRect = REGION_RECTS(cclip);
    /*
     * Render glyphs through clipping regions.
     */
    RivaSetRopSolid(pGC->alu);
    rivaRendered2D = 1;
    while (nclip--)
    {
        /*
         * Intersect glyph bounds with clip rect.
         */
        intersectRect.x1 = max(pclipRect->x1, glyphRect.x1);
        intersectRect.y1 = max(pclipRect->y1, glyphRect.y1);
        intersectRect.x2 = min(pclipRect->x2, glyphRect.x2);
        intersectRect.y2 = min(pclipRect->y2, glyphRect.y2);
        pclipRect++;
        /*
         * Only draw glyph string if visible through this region.
         */
        if (RIVA_RECT_VALID(intersectRect))
        {
            /*
             * Set clip rect.
             */
            RIVA_FIFO_FREE(riva, Bitmap, 3);
            riva.Bitmap->ClipD.TopLeft     = (intersectRect.y1 << 16) | intersectRect.x1;
            riva.Bitmap->ClipD.BottomRight = (intersectRect.y2 << 16) | intersectRect.x2;
            riva.Bitmap->Color1D           = pGC->fgPixel;
            /*
             * Render each glyph.
             */
            for (i = dx = 0; i < nglyph; i++)
            {
                RivaTransparentGlyph(x + dx + ppci[i]->metrics.leftSideBearing,
                                     y      - ppci[i]->metrics.ascent,
                                     GLYPHWIDTHPIXELS(ppci[i]),
                                     GLYPHHEIGHTPIXELS(ppci[i]),
                                     (unsigned long *)FONTGLYPHBITS(pglyphBase, ppci[i]));
                dx += ppci[i]->metrics.characterWidth;
            }
        }
    }
}
static void RivaImageGlyphBltNonTE(DrawablePtr pDrawable, GCPtr pGC, int xInit, int yInit, int nglyph, CharInfoPtr *ppci, unsigned char *pglyphBase)
{
    FontPtr       pfont = pGC->font;
    RegionPtr     cclip;
    BoxPtr        pclipRect;
    BoxRec        glyphRect, intersectRect;
    int           x, y, i, dx;
    int           nclip;

    /*
     * Sanity checks.
     */
    if (!xf86VTSema) return;
    y = yInit + pDrawable->y;
    x = xInit + pDrawable->x;
    /*
     * Get glyph extents.
     */
    for (i = dx = 0; i < nglyph; i++)
    	dx += ppci[i]->metrics.characterWidth;
    if (dx >= 0)
    {
        glyphRect.x1 = x; glyphRect.x2 = x + dx;
    }
    else
    {
        glyphRect.x1 = x + dx; glyphRect.x2 = x;
    }
    glyphRect.y1 = y - FONTASCENT(pfont); glyphRect.y2 = y + FONTDESCENT(pfont);
    /*
     * Get region clip rects.
     */
    cclip     = cfbGetCompositeClip(pGC);
    nclip     = REGION_NUM_RECTS(cclip);
    pclipRect = REGION_RECTS(cclip);
    /*
     * Render glyphs through clipping regions.
     */
    RivaSetRopSolid(GXcopy);
    rivaRendered2D = 1;
    while (nclip--)
    {
        /*
         * Intersect glyph bounds with clip rect.
         */
        intersectRect.x1 = max(pclipRect->x1, glyphRect.x1);
        intersectRect.y1 = max(pclipRect->y1, glyphRect.y1);
        intersectRect.x2 = min(pclipRect->x2, glyphRect.x2);
        intersectRect.y2 = min(pclipRect->y2, glyphRect.y2);
        pclipRect++;
        /*
         * Only draw glyph string if visible through this region.
         */
        if (RIVA_RECT_VALID(intersectRect))
        {
            /*
             * Draw background rect.
             */
            RIVA_FIFO_FREE(riva, Bitmap, 3);
            riva.Bitmap->Color1A                           = pGC->bgPixel;
            riva.Bitmap->UnclippedRectangle[0].TopLeft     = (intersectRect.x1 << 16) | intersectRect.y1;
            riva.Bitmap->UnclippedRectangle[0].WidthHeight = ((intersectRect.x2 - intersectRect.x1) << 16)
                                                           |  (intersectRect.y2 - intersectRect.y1);
            /*
             * Set clip rect.
             */
            RIVA_FIFO_FREE(riva, Bitmap, 3);
            riva.Bitmap->ClipD.TopLeft     = (intersectRect.y1 << 16) | intersectRect.x1;
            riva.Bitmap->ClipD.BottomRight = (intersectRect.y2 << 16) | intersectRect.x2;
            riva.Bitmap->Color1D           = pGC->fgPixel;
            /*
             * Render each glyph.
             */
            for (i = dx = 0; i < nglyph; i++)
            {
                RivaTransparentGlyph(x + dx + ppci[i]->metrics.leftSideBearing,
                                     y      - ppci[i]->metrics.ascent,
                                     GLYPHWIDTHPIXELS(ppci[i]),
                                     GLYPHHEIGHTPIXELS(ppci[i]),
                                     (unsigned long *)FONTGLYPHBITS(pglyphBase, ppci[i]));
                dx += ppci[i]->metrics.characterWidth;
            }
        }
    }
}
/*
 * Synchronise with graphics engine.  Make sure it is idle before returning.
 * Should attempt to yield CPU if busy for awhile.
 */
static void RivaSync(void)
{
    while (riva.Busy(&riva));
}

/****************************************************************************\
*                                                                            *
*                        XAA HW Cursor Entrypoints                           *
*                                                                            *
\****************************************************************************/
                              
/*
 * RIVA supports full colour cursors as X1R5G5B5.  Upper bit is the XOR
 * bit.  All 0's equals transparency.
 */
#define MAX_CURS            32
#define TRANSPARENT_PIXEL   0
#define ConvertToRGB555(red,green,blue) \
    ((((red)>>11)&0x1F)<<10)|((((green)>>11)&0x1F)<<5)|(((blue)>>11)&0x1F)|0x8000
/*
 * Externals.
 */
extern vgaHWCursorRec         vgaHWCursor;
extern miPointerScreenFuncRec xf86PointerScreenFuncs;
extern xf86InfoRec            xf86Info;
/*
 * Internal realized cursor structure.
 */
typedef struct
{
    unsigned short foreColour, backColour;   /* Colour for this cursor in RGB555 */
    unsigned short image[MAX_CURS*MAX_CURS]; /* Image */
}RivaCursor;
/*
 * This is the set variables that defines the cursor state within the
 * driver.
 */
static int       RivaCursorHotX, RivaCursorHotY;
static CursorPtr RivaCursorpCurs;
/*
 * This function is called when a new cursor image is requested by
 * the server. The main thing to do is convert the bitwise image
 * provided by the server into a format that the graphics card
 * can conveniently handle, and store that in system memory.
 * Adapted from accel/s3/s3Cursor.c.
 */
static Bool RivaRealizeCursor(ScreenPtr pScr, CursorPtr pCurs)
{
    RivaCursor    *cursor;
    int            x, y, i;
    int            byteIndex, bitIndex, maskBit, sourceBit;
    int            height, width;
    int            pad, lineOffset;
    unsigned char *source, *mask;

    if (pCurs->bits->refcnt > 1) return (TRUE);
    if (!(cursor = (RivaCursor*)xalloc(sizeof(RivaCursor)))) return (FALSE);
    cursor->foreColour = ConvertToRGB555(pCurs->foreRed, pCurs->foreGreen, pCurs->foreBlue);
    cursor->backColour = ConvertToRGB555(pCurs->backRed, pCurs->backGreen, pCurs->backBlue);
    height = pCurs->bits->height;
    width  = pCurs->bits->width;
    source = (unsigned char *)pCurs->bits->source;
    mask   = (unsigned char *)pCurs->bits->mask;
    pad    = PixmapBytePad(width, 1); /* Bytes per line. */
    for (y = 0, i = 0, lineOffset = 0; y < MAX_CURS; y++, lineOffset += pad)
    {
        for (x = 0; x < MAX_CURS; x++, i++)
        {
            cursor->image[i] = TRANSPARENT_PIXEL;
            if (x < width && y < height)
            {
                byteIndex = lineOffset + (x >> 3);
                bitIndex  = x & 0x07;
                maskBit   = mask[byteIndex]   & (1 << bitIndex);
                sourceBit = source[byteIndex] & (1 << bitIndex);
                if (maskBit)
                    cursor->image[i] = (sourceBit) ? cursor->foreColour : cursor->backColour;
            }
        }
    }
    pCurs->bits->devPriv[pScr->myNum] = (pointer)cursor;
    return (TRUE);
}
/*
 * This is called when a cursor is no longer used. The intermediate
 * cursor image storage that we created needs to be deallocated.
 */
static Bool RivaUnrealizeCursor(ScreenPtr pScr, CursorPtr pCurs)
{
    if (pCurs->bits->refcnt <= 1 && pCurs->bits->devPriv[pScr->myNum])
    {
        xfree(pCurs->bits->devPriv[pScr->myNum]);
        pCurs->bits->devPriv[pScr->myNum] = 0;
    }
    return (TRUE);
}
/*
 * This function is called when the current cursor is moved. It makes
 * the graphic chip display the cursor at the new position.
 */
static void RivaMoveCursor(ScreenPtr pScr, int x, int y)
{
    if ( ! xf86VTSema)
        return;

    x -= vga256InfoRec.frameX0 + RivaCursorHotX;
    y -= vga256InfoRec.frameY0 + RivaCursorHotY;
    if (XF86SCRNINFO(pScr)->modes->Flags & V_DBLSCAN)
        y *= 2;

    *(riva.CURSORPOS) = (x & 0xFFFF) | (y << 16);
}
/*
 * This function uploads a cursor image to the video memory of the
 * graphics card. The source image has already been converted by the
 * Realize function to a format that can be quickly transferred to
 * the card.
 * This is a local function that is not called from outside of this
 * module.
 */
static void RivaLoadCursorToCard(ScreenPtr pScr, CursorPtr pCurs)
{
    int        *image, i, numInts, save;
    RivaCursor *cursor = (RivaCursor*) pCurs->bits->devPriv[pScr->myNum];

    numInts = sizeof(cursor->image) / sizeof(int);
    image   = (int *)cursor->image;
    save    = riva.ShowHideCursor(&riva, 0); /* Hide cursor, saving its current display state */
    for (i = 0; i < numInts; i++)
        riva.CURSOR[i] = image[i];
    riva.ShowHideCursor(&riva, save); /* Restore cursor display state */
}
/*
 * This function should make the graphics chip display new cursor that
 * has already been "realized". We need to upload it to video memory,
 * make the graphics chip display it.
 * This is a local function that is not called from outside of this
 * module (although it largely corresponds to what the SetCursor
 * function in the Pointer record needs to do).
 */
static void RivaLoadCursor(ScreenPtr pScr, CursorPtr pCurs, int x, int y)
{
    RivaCursorpCurs = pCurs;
    RivaCursorHotX  = pCurs->bits->xhot;
    RivaCursorHotY  = pCurs->bits->yhot;
    riva.ShowHideCursor(&riva, 0);
    RivaLoadCursorToCard(pScr, pCurs);
    RivaMoveCursor(pScr, x, y);
    riva.ShowHideCursor(&riva, 1);
}
/*
 * This function should display a new cursor at a new position.
 */
static void RivaSetCursor(ScreenPtr pScr, CursorPtr pCurs, int x, int y, Bool generateEvent)
{
    if (pCurs && xf86VTSema)
            RivaLoadCursor(pScr, pCurs, x, y);
}
/*
 * This is a local function that programs the colors of the cursor
 * on the graphics chip.
 * Adapted from accel/s3/s3Cursor.c.
 */
static void RivaRecolorCursor(ScreenPtr pScr, CursorPtr pCurs, Bool displayed)
{
    unsigned short fore, back;
    RivaCursor    *cursor = (RivaCursor *)pCurs->bits->devPriv[pScr->myNum];

    if (xf86VTSema && displayed)
    {
        fore = ConvertToRGB555(pCurs->foreRed, pCurs->foreGreen, pCurs->foreBlue);
        back = ConvertToRGB555(pCurs->backRed, pCurs->backGreen, pCurs->backBlue);
        if (cursor->foreColour != fore || cursor->backColour != back)
            RivaLoadCursorToCard(pScr, pCurs);
    }
}
/*
 * This function should redisplay a cursor that has been
 * displayed earlier. It is called by the SVGA server.
 */
static void RivaRestoreCursor(ScreenPtr pScr)
{
    int x, y;

    miPointerPosition(&x, &y);
    RivaLoadCursor(pScr, RivaCursorpCurs, x, y);
}
/*
 * This doesn't do very much. It just calls the mi routine. It is called
 * by the SVGA server.
 */
static void RivaWarpCursor(ScreenPtr pScr, int x, int y)
{
    miPointerWarpCursor(pScr, x, y);
    xf86Info.currentScreen = pScr;
}
/*
 * This function is called by the SVGA server. It returns the
 * size of the hardware cursor that we support when asked for.
 * It is called by the SVGA server.
 */
static void RivaQueryBestSize(int class, unsigned short *pwidth, unsigned short *pheight, ScreenPtr pScreen)
{
    if (*pwidth > 0)
    {
        if (class == CursorShape)
            *pwidth = *pheight = MAX_CURS;
        else
            (void)mfbQueryBestSize(class, pwidth, pheight, pScreen);
    }
}
/*
 * This is a high-level init function, called once; it passes a local
 * miPointerSpriteFuncRec with additional functions that we need to provide.
 * It is called by the SVGA server.
 */
static Bool RivaCursorInit(char *pm,ScreenPtr pScr)
{
    static int RivaCursorGeneration = -1;
    static miPointerSpriteFuncRec RivaPointerSpriteFuncs =
    {
        (Bool (*)())RivaRealizeCursor, (Bool (*)())RivaUnrealizeCursor,
        (void (*)())RivaSetCursor,     (void (*)())RivaMoveCursor,
    };

    RivaCursorHotX = RivaCursorHotY = 0;
    if (RivaCursorGeneration != serverGeneration)
    {
        if (!(miPointerInitialize(pScr, &RivaPointerSpriteFuncs, &xf86PointerScreenFuncs, FALSE)))
            return (FALSE);
        pScr->RecolorCursor  = RivaRecolorCursor;
        RivaCursorGeneration = serverGeneration;
    }
    return (TRUE);
}

/****************************************************************************\
*                                                                            *
*                        XAA Chip Setup Entrypoints                          *
*                                                                            *
\****************************************************************************/

/*
 * Main initialization entrypoint.
 */
static void RivaFbInit(void)
{
    unsigned surfSize, cacheSize;
    /*
     * Hardware Cursor initialization.
     */
    if (!OFLG_ISSET(OPTION_SW_CURSOR, &vga256InfoRec.options))
    {
        vgaHWCursor.Initialized   = TRUE;
        vgaHWCursor.Init          = RivaCursorInit;
        vgaHWCursor.Restore       = RivaRestoreCursor;
        vgaHWCursor.Warp          = RivaWarpCursor;
        vgaHWCursor.QueryBestSize = RivaQueryBestSize;
        if (xf86Verbose)
            ErrorF("%s %s: %s: Using hardware cursor\n",
                   XCONFIG_PROBED,
                   vga256InfoRec.name,
                   vga256InfoRec.chipset);
    }
    /*
     * 2D acceleration initialization.
     */
    if (!OFLG_ISSET(OPTION_NOACCEL, &vga256InfoRec.options))
    {
        /*
         * There are still some problems with delayed syncing.
         */
        xf86AccelInfoRec.Flags = BACKGROUND_OPERATIONS/*| DELAYED_SYNC*/
                               | HARDWARE_PATTERN_PROGRAMMED_BITS/*| HARDWARE_PATTERN_BIT_ORDER_MSBFIRST*/
                               | HARDWARE_PATTERN_MONO_TRANSPARENCY
                               | HARDWARE_PATTERN_SCREEN_ORIGIN;
        xf86AccelInfoRec.Sync  = RivaSync;
        /*
         * Hook filled rectangles.
         */
        xf86GCInfoRec.PolyFillRectSolidFlags             = NO_PLANEMASK;
        xf86AccelInfoRec.SetupForFillRectSolid           = RivaSetupForFillRectSolid;
        xf86AccelInfoRec.SubsequentFillRectSolid         = RivaSubsequentFillRectSolid;
        if (!(vga256InfoRec.bitsPerPixel == 16 && xf86weight.green == 6))
        {
            xf86AccelInfoRec.SetupFor8x8PatternColorExpand   = RivaSetupFor8x8PatternColorExpand;
            xf86AccelInfoRec.Subsequent8x8PatternColorExpand = RivaSubsequent8x8PatternColorExpand;
        }
        /*
         * Set pattern opaque bits based on pixel format.
         */
        switch (vga256InfoRec.bitsPerPixel)
        {
            case 8:
                rivaOpaqueMonochrome = 0xFFFFFF00;
                break;
            case 15:
            case 16:
                rivaOpaqueMonochrome = (xf86weight.green == 5) ? 0xFFFF8000 : 0xFFFF0000;
                break;
            default:
                rivaOpaqueMonochrome = 0xFF000000;
        }
        /*
         * Hook screen-to-screen BLTs.
         */
        xf86GCInfoRec.CopyAreaFlags                   = NO_PLANEMASK | NO_TRANSPARENCY;
        xf86AccelInfoRec.SetupForScreenToScreenCopy   = RivaSetupForScreenToScreenCopy;
        xf86AccelInfoRec.SubsequentScreenToScreenCopy = RivaSubsequentScreenToScreenCopy;
        /*
         * Hook text routines.
         */
#if GLYPHPADBYTES == 4
        xf86GCInfoRec.PolyGlyphBltNonTEFlags  |= NO_PLANEMASK;
        xf86GCInfoRec.ImageGlyphBltNonTEFlags |= NO_PLANEMASK;
        xf86GCInfoRec.PolyGlyphBltTEFlags     |= NO_PLANEMASK;
        xf86GCInfoRec.ImageGlyphBltTEFlags    |= NO_PLANEMASK;
        xf86GCInfoRec.PolyGlyphBltNonTE        = RivaPolyGlyphBltNonTE;
        xf86GCInfoRec.PolyGlyphBltTE           = RivaPolyGlyphBltTE;
        xf86GCInfoRec.ImageGlyphBltNonTE       = RivaImageGlyphBltNonTE;
        if (vga256InfoRec.bitsPerPixel == 16 && xf86weight.green == 6)
            xf86GCInfoRec.ImageGlyphBltNonTE       = RivaImageGlyphBltNonTE;
        else
            xf86GCInfoRec.ImageGlyphBltTE          = RivaImageGlyphBltTE;
#endif
        /*
         * Calc surface size and round up to nearest 256 bytes.
         */
        surfSize = vga256InfoRec.virtualY 
                 * vga256InfoRec.displayWidth
                 * vga256InfoRec.bitsPerPixel / 8;
        surfSize = (surfSize + 255) & 0xFFFFFF00;
        /*
         * Get a reasonable value for pixmap cache size.
         */
        switch (vga256InfoRec.bitsPerPixel)
        {
            case 8:
                cacheSize = vga256InfoRec.videoRam * 1024 - surfSize;
            break;
            case 15:
            case 16:
                cacheSize = 128 * vga256InfoRec.displayWidth;
                if ((vga256InfoRec.videoRam < 7*1024) && (vga256InfoRec.displayWidth < 1024))
                    cacheSize = 0;
                break;
            case 24:
            case 32:
                cacheSize = 256 * vga256InfoRec.displayWidth;
                if (vga256InfoRec.videoRam < 15*1024)
                    cacheSize = vga256InfoRec.videoRam * 1024 - surfSize;
                break;
        }
        if (cacheSize + surfSize > vga256InfoRec.videoRam * 1024)
            cacheSize = vga256InfoRec.videoRam * 1024 - surfSize;
        /*
         * Calc surface offsets.
         */
        rivaBufferOffset[RIVA_FRONT_BUFFER]   = 0;
        rivaBufferOffset[RIVA_CACHE_BUFFER]   = rivaBufferOffset[RIVA_FRONT_BUFFER] + surfSize;
        rivaBufferOffset[RIVA_BACK_BUFFER]    = rivaBufferOffset[RIVA_CACHE_BUFFER] + cacheSize;
        rivaBufferOffset[RIVA_DEPTH_BUFFER]   = vga256InfoRec.videoRam * 1024       - surfSize;
        rivaBufferOffset[RIVA_TEXTURE_BUFFER] = rivaBufferOffset[RIVA_BACK_BUFFER]  + surfSize;
        /*
         * Make sure there are no overlapping surfaces.
         */
        if (rivaBufferOffset[RIVA_TEXTURE_BUFFER] > rivaBufferOffset[RIVA_DEPTH_BUFFER])
        {
            /*
             * Overlap back and depth buffer.  Accelerated apps can't have both.
             */
            rivaBufferOffset[RIVA_TEXTURE_BUFFER] = rivaBufferOffset[RIVA_BACK_BUFFER];
            rivaBufferOffset[RIVA_BACK_BUFFER]    = rivaBufferOffset[RIVA_DEPTH_BUFFER];
            if (rivaBufferOffset[RIVA_TEXTURE_BUFFER] > rivaBufferOffset[RIVA_DEPTH_BUFFER])
            {
                /*
                 * Nope, no room.
                 */
                rivaBufferOffset[RIVA_BACK_BUFFER]  = 0;
                rivaBufferOffset[RIVA_DEPTH_BUFFER] = 0;
            }
        }
        /*
         * Check for any room at all.
         */
        if (rivaBufferOffset[RIVA_BACK_BUFFER] > rivaBufferOffset[RIVA_DEPTH_BUFFER])
        {
            rivaBufferOffset[RIVA_BACK_BUFFER]    = 0;
            rivaBufferOffset[RIVA_DEPTH_BUFFER]   = 0;
            rivaBufferOffset[RIVA_TEXTURE_BUFFER] = 0;
        }
        /*
         * Set pixmap cache if enough room.
         */
        if (cacheSize > 1024)
        {
            xf86AccelInfoRec.Flags |= PIXMAP_CACHE;
            xf86InitPixmapCache(&vga256InfoRec,
                                rivaBufferOffset[RIVA_CACHE_BUFFER],
                                rivaBufferOffset[RIVA_CACHE_BUFFER] + cacheSize);
        }
    }
}
/*
 * Lock and unlock VGA and SVGA registers.
 */
static void RivaEnterLeave(Bool enter)
{
    unsigned char tmp;

#ifdef XFreeXDGA
    if (vga256InfoRec.directMode&XF86DGADirectGraphics && !enter)
    {
        if (vgaHWCursor.Initialized)
            riva.ShowHideCursor(&riva, 0);
        return;
    }
#endif 

    if (enter)
    {
        xf86EnableIOPorts(vga256InfoRec.scrnIndex);
        outb(vgaIOBase + 4, 0x11);
        tmp = inb(vgaIOBase + 5);
        outb(vgaIOBase + 5, tmp & 0x7F);
        outb(riva.LockUnlockIO, riva.LockUnlockIndex);
        outb(riva.LockUnlockIO + 1, 0x57);
    }
    else
    {
        outb(vgaIOBase + 4, 0x11);
        tmp = inb(vgaIOBase + 5);
        outb(vgaIOBase + 5, (tmp & 0x7F) | 0x80);
        outb(riva.LockUnlockIO, riva.LockUnlockIndex);
        outb(riva.LockUnlockIO + 1, 0x99);
        xf86DisableIOPorts(vga256InfoRec.scrnIndex);
    }
}
/*
 * Initialize mode info.
 */
static Bool RivaInit(DisplayModePtr mode)
{
    int i;
    int horizDisplay = (mode->CrtcHDisplay/8)   - 1;
    int horizStart   = (mode->CrtcHSyncStart/8) - 1;
    int horizEnd     = (mode->CrtcHSyncEnd/8)   - 1;
    int horizTotal   = (mode->CrtcHTotal/8)     - 1;
    int vertDisplay  =  mode->CrtcVDisplay      - 1;
    int vertStart    =  mode->CrtcVSyncStart    - 1;
    int vertEnd      =  mode->CrtcVSyncEnd      - 1;
    int vertTotal    =  mode->CrtcVTotal        - 2;

    /* 
     * Calculate standard VGA settings.
     */
    if (!vgaHWInit(mode, sizeof(vgaNVRec)))
        return (FALSE);
    /*
     * VGA is always valid for RIVA.
     */
    ((vgaNVPtr)vgaNewVideoState)->vgaValid = 1;
    /*
     * Set all CRTC values.
     */
    ((vgaNVPtr)vgaNewVideoState)->std.CRTC[0x0]  = Set8Bits(horizTotal - 4);
    ((vgaNVPtr)vgaNewVideoState)->std.CRTC[0x1]  = Set8Bits(horizDisplay);
    ((vgaNVPtr)vgaNewVideoState)->std.CRTC[0x2]  = Set8Bits(horizDisplay);
    ((vgaNVPtr)vgaNewVideoState)->std.CRTC[0x3]  = SetBitField(horizTotal,4:0,4:0) 
                                                 | SetBit(7);
    ((vgaNVPtr)vgaNewVideoState)->std.CRTC[0x4]  = Set8Bits(horizStart);
    ((vgaNVPtr)vgaNewVideoState)->std.CRTC[0x5]  = SetBitField(horizTotal,5:5,7:7)
                                                 | SetBitField(horizEnd,4:0,4:0);
    ((vgaNVPtr)vgaNewVideoState)->std.CRTC[0x6]  = SetBitField(vertTotal,7:0,7:0);
    ((vgaNVPtr)vgaNewVideoState)->std.CRTC[0x7]  = SetBitField(vertTotal,8:8,0:0)
                                                 | SetBitField(vertDisplay,8:8,1:1)
                                                 | SetBitField(vertStart,8:8,2:2)
                                                 | SetBitField(vertDisplay,8:8,3:3)
                                                 | SetBit(4)
                                                 | SetBitField(vertTotal,9:9,5:5)
                                                 | SetBitField(vertDisplay,9:9,6:6)
                                                 | SetBitField(vertStart,9:9,7:7);
    ((vgaNVPtr)vgaNewVideoState)->std.CRTC[0x9]  = SetBitField(vertDisplay,9:9,5:5)
                                                 | SetBit(6)
                                                 | ((mode->Flags & V_DBLSCAN) ? 0x80 : 0x00);
    ((vgaNVPtr)vgaNewVideoState)->std.CRTC[0x10] = Set8Bits(vertStart);
    ((vgaNVPtr)vgaNewVideoState)->std.CRTC[0x11] = SetBitField(vertEnd,3:0,3:0) | SetBit(5);
    ((vgaNVPtr)vgaNewVideoState)->std.CRTC[0x12] = Set8Bits(vertDisplay);
    ((vgaNVPtr)vgaNewVideoState)->std.CRTC[0x13] = ((vga256InfoRec.displayWidth/8)*(vgaBitsPerPixel/8)) & 0xFF;
    ((vgaNVPtr)vgaNewVideoState)->std.CRTC[0x15] = Set8Bits(vertDisplay);
    ((vgaNVPtr)vgaNewVideoState)->std.CRTC[0x16] = Set8Bits(vertTotal + 1);
    /*
     * Initialize DAC palette.
     */
    if(vgaBitsPerPixel != 8 )
    {
        if (riva.Architecture == NV_ARCH_03)
            for (i = 0; i < 256; i++)
            {
                ((vgaNVPtr)vgaNewVideoState)->std.DAC[i*3]     = i >> 2;
                ((vgaNVPtr)vgaNewVideoState)->std.DAC[(i*3)+1] = i >> 2;
                ((vgaNVPtr)vgaNewVideoState)->std.DAC[(i*3)+2] = i >> 2;
            }
        else
            for (i = 0; i < 256; i++)
            {
                ((vgaNVPtr)vgaNewVideoState)->std.DAC[i*3]     = i;
                ((vgaNVPtr)vgaNewVideoState)->std.DAC[(i*3)+1] = i;
                ((vgaNVPtr)vgaNewVideoState)->std.DAC[(i*3)+2] = i;
            }
    }
    /*
     * Calculate the extended registers.
     */
    switch (vgaBitsPerPixel)
    {
        case 8:
            i = 8;
            break;
        case 15:
        case 16:
            i = (xf86weight.green == 6) ? 16 : 15;
            break;
        default:
            i = 32;
    }
    riva.CalcStateExt(&riva, 
                      (RIVA_HW_STATE *)&(((vgaNVPtr)vgaNewVideoState)->regs.RivaState),
                      i, /* BPP */
                      vga256InfoRec.displayWidth,
                      mode->CrtcHDisplay,
                      horizDisplay,
                      horizStart,
                      horizEnd,
                      horizTotal,
                      vga256InfoRec.virtualY,
                      vertDisplay,
                      vertStart,
                      vertEnd,
                      vertTotal,
                      vga256InfoRec.clock[mode->Clock]);
    return (TRUE);
}
/*
 * Load new mode state.
 */
static void RivaRestore(void *data)
{
    RIVA_HW_STATE *state = &(((vgaNVPtr)data)->regs.RivaState);
    vgaProtect(TRUE);
    /*
     * Make sure to restore state in this order.  Required for proper 8 bit DAC.
     * restoration.
     */
    riva.LoadStateExt(&riva, state);
    vgaHWRestore((vgaHWPtr)data);
    /*
     * Reset accelerator state. This MUST be done after a mode change to make
     * the HW update complete.
     */
    RivaSetRopSolid(GXcopy);
    RivaSetClippingRectangle(0, 0, 0x3FFF, 0x3FFF);
    RivaSetPattern(~0, ~0, 0, 0);
    vgaProtect(FALSE);
}
/*
 * Save current mode state.
 */
static void *RivaSave(void *data)
{
    data = vgaHWSave((vgaHWPtr)data, sizeof(vgaNVRec));
    riva.UnloadStateExt(&riva, (RIVA_HW_STATE *)&(((vgaNVPtr)data)->regs.RivaState));
    return (data);
}
/*
 * Update scanout address.
 */
static void RivaAdjust(int x, int y)
{
    int startAddr = (((y*vga256InfoRec.virtualX)+x)*(vgaBitsPerPixel/8));
    riva.SetStartAddress(&riva, startAddr);
}
static int RivaValidMode(DisplayModePtr mode, Bool verbose, int flag)
{
    float refresh_rate;

    refresh_rate = mode->Clock * 1000 / (mode->CrtcHTotal * mode->CrtcVTotal);
    if (mode->Flags & V_DBLSCAN)
        refresh_rate /= 2;
    return (MODE_OK);
}
/*
 * These are all NO-OPed for now.
 */
static void RivaDisplayPowerManagementSet(int mode)
{
}
static Bool RivaScreenInit(ScreenPtr pScreen, pointer pbits, int xsize, int ysize, int dpix, int dpiy, int width)
{
    return (TRUE);
}
static void RivaSaveScreen(int on)
{
    vgaHWSaveScreen(on);
}
static void RivaGetMode(DisplayModePtr display)
{
}

/****************************************************************************\
*                                                                            *
*                        XAA Chip Probe Entrypoints                          *
*                                                                            *
\****************************************************************************/

/*
 * OS specific code needs to map the registers so the config sense code can get
 * to the registers.  This keeps the riva_hw.c file OS agnostic.
 */
static int RivaProbe(vgaVideoChipRec *nv, void *regBase, void *frameBase)
{
    OFLG_SET(OPTION_NOACCEL,   &(nv->ChipOptionFlags));
    OFLG_SET(OPTION_SW_CURSOR, &(nv->ChipOptionFlags));
    OFLG_SET(OPTION_HW_CURSOR, &(nv->ChipOptionFlags));
    /*
     * No IRQ in use.
     */
    riva.EnableIRQ = 0;
    /*
     * Map and enable I/O registers.
     */
    xf86ClearIOPortList(vga256InfoRec.scrnIndex);
    xf86AddIOPorts(vga256InfoRec.scrnIndex, Num_VGA_IOPorts, VGA_IOPorts);
    xf86EnableIOPorts(vga256InfoRec.scrnIndex);
    /*
     * Map remaining registers. This MUST be done in the OS specific driver code.
     */
    riva.IO      = vgaIOBase = (inb(0x3CC) & 0x01) ? 0x3D0 : 0x3B0;
    riva.PRAMDAC = (unsigned *)xf86MapVidMem(vga256InfoRec.scrnIndex, MMIO_REGION,((char *)regBase+0x00680000), 0x00001000);
    riva.PFB     = (unsigned *)xf86MapVidMem(vga256InfoRec.scrnIndex, MMIO_REGION,((char *)regBase+0x00100000), 0x00001000);
    riva.PFIFO   = (unsigned *)xf86MapVidMem(vga256InfoRec.scrnIndex, MMIO_REGION,((char *)regBase+0x00002000), 0x00002000);
    riva.PGRAPH  = (unsigned *)xf86MapVidMem(vga256InfoRec.scrnIndex, MMIO_REGION,((char *)regBase+0x00400000), 0x00002000);
    riva.PEXTDEV = (unsigned *)xf86MapVidMem(vga256InfoRec.scrnIndex, MMIO_REGION,((char *)regBase+0x00101000), 0x00001000);
    riva.PTIMER  = (unsigned *)xf86MapVidMem(vga256InfoRec.scrnIndex, MMIO_REGION,((char *)regBase+0x00009000), 0x00001000);
    riva.PMC     = (unsigned *)xf86MapVidMem(vga256InfoRec.scrnIndex, MMIO_REGION,((char *)regBase+0x00000000), 0x00001000);
    riva.FIFO    = (unsigned *)xf86MapVidMem(vga256InfoRec.scrnIndex, MMIO_REGION,((char *)regBase+0x00800000), 0x00010000);
    /*
     * Low level probe.
     */
    RivaGetConfig(&riva);
    /*
     * Enable extended IO ports.
     */
    RivaEnterLeave(ENTER);
    /*
     * Fill in the results of the probe.
     */
    vga256InfoRec.maxClock = riva.MaxVClockFreqKHz;
    vga256InfoRec.videoRam = riva.RamAmountKBytes;
    /*
     * Set framebuffer information. Linear size must be power of 2 for 8bpp to
     * work correctly (not the same as videoRam). Round up to next MByte.
     */
    nv->ChipLinearSize = ((vga256InfoRec.videoRam + 0x3FF) & ~0x3FF) * 1024;
    nv->ChipLinearBase = (int)frameBase;
    nv->ChipHas32bpp   = TRUE;
    /*
     * Changes the entries in the NV struct to point at the correct function.
     */
    nv->ChipEnterLeave = RivaEnterLeave;
    nv->ChipInit       = RivaInit;
    nv->ChipValidMode  = RivaValidMode;
    nv->ChipSave       = RivaSave;
    nv->ChipRestore    = RivaRestore;
    nv->ChipAdjust     = RivaAdjust;
    nv->ChipSaveScreen = RivaSaveScreen;
    nv->ChipGetMode    = (void (*)())NoopDDA;
    nv->ChipFbInit     = RivaFbInit;
    return (1);
}
/*
 * Need a probe for each architecture because the memory maps are
 * slightly different for each.
 */
int NV3Probe(vgaVideoChipRec *nv, void *regBase, void *frameBase)
{
    OFLG_SET(OPTION_NOACCEL,   &(nv->ChipOptionFlags));
    OFLG_SET(OPTION_SW_CURSOR, &(nv->ChipOptionFlags));
    OFLG_SET(OPTION_HW_CURSOR, &(nv->ChipOptionFlags));
    /*
     * Unfortunately, RIVA 128 only wants to be accelerated at 15 BPP, not 16 BPP.
     */
    if (vgaBitsPerPixel == 16)
        xf86weight.red = xf86weight.green = xf86weight.blue = 5;
    /*
     * Record chip architecture based in PCI probe.
     */
    riva.Architecture = NV_ARCH_03;
    /*
     * Map chip-specific memory-mapped registers. This MUST be done in the OS specific driver code.
     */
    riva.PRAMIN = (unsigned *)xf86MapVidMem(vga256InfoRec.scrnIndex, MMIO_REGION,((char *)frameBase+0x00C00000), 0x00008000);
    /*
     * Call the common chip probe.
     */
    return (RivaProbe(nv, regBase, frameBase));
}
int NV4Probe(vgaVideoChipRec *nv, void *regBase, void *frameBase)
{
    OFLG_SET(OPTION_DAC_8_BIT, &(nv->ChipOptionFlags));
    OFLG_SET(OPTION_DAC_8_BIT, &vga256InfoRec.options);
    /*
     * Record chip architecture based in PCI probe.
     */
    riva.Architecture = NV_ARCH_04;
    /*
     * Map chip-specific memory-mapped registers. This MUST be done in the OS specific driver code.
     */
    riva.PRAMIN = (unsigned *)xf86MapVidMem(vga256InfoRec.scrnIndex, MMIO_REGION,((char *)regBase+0x00710000), 0x00010000);
    riva.PCRTC  = (unsigned *)xf86MapVidMem(vga256InfoRec.scrnIndex, MMIO_REGION,((char *)regBase+0x00600000), 0x00001000);
    /*
     * Call the common chip probe.
     */
    return (RivaProbe(nv, regBase, frameBase));
}
int NV10Probe(vgaVideoChipRec *nv, void *regBase, void *frameBase)
{
    OFLG_SET(OPTION_DAC_8_BIT, &(nv->ChipOptionFlags));
    OFLG_SET(OPTION_DAC_8_BIT, &vga256InfoRec.options);
    /*
     * Record chip architecture based in PCI probe.
     */
    riva.Architecture = NV_ARCH_10;
    /*
     * Map chip-specific memory-mapped registers. This MUST be done in the OS specific driver code.
     */
    riva.PRAMIN = (unsigned *)xf86MapVidMem(vga256InfoRec.scrnIndex, MMIO_REGION,((char *)regBase+0x00710000), 0x00010000);
    riva.PCRTC  = (unsigned *)xf86MapVidMem(vga256InfoRec.scrnIndex, MMIO_REGION,((char *)regBase+0x00600000), 0x00001000);
    /*
     * Call the common chip probe.
     */
    return (RivaProbe(nv, regBase, frameBase));
}


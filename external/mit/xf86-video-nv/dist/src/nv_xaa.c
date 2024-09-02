/*
 * Copyright (c) 2003 NVIDIA, Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "nv_include.h"
#ifdef HAVE_XAA_H
#include "xaalocal.h"
#endif
#include "miline.h"
#include "nv_dma.h"

static const int NVCopyROP[16] =
{
   0x00,            /* GXclear */
   0x88,            /* GXand */
   0x44,            /* GXandReverse */
   0xCC,            /* GXcopy */
   0x22,            /* GXandInverted */
   0xAA,            /* GXnoop */
   0x66,            /* GXxor */
   0xEE,            /* GXor */
   0x11,            /* GXnor */
   0x99,            /* GXequiv */
   0x55,            /* GXinvert*/
   0xDD,            /* GXorReverse */
   0x33,            /* GXcopyInverted */
   0xBB,            /* GXorInverted */
   0x77,            /* GXnand */
   0xFF             /* GXset */
};

static const int NVCopyROP_PM[16] =
{
   0x0A,            /* GXclear */
   0x8A,            /* GXand */
   0x4A,            /* GXandReverse */
   0xCA,            /* GXcopy */
   0x2A,            /* GXandInverted */
   0xAA,            /* GXnoop */
   0x6A,            /* GXxor */
   0xEA,            /* GXor */
   0x1A,            /* GXnor */
   0x9A,            /* GXequiv */
   0x5A,            /* GXinvert*/
   0xDA,            /* GXorReverse */
   0x3A,            /* GXcopyInverted */
   0xBA,            /* GXorInverted */
   0x7A,            /* GXnand */
   0xFA             /* GXset */
};

#ifdef HAVE_XAA_H
static const int NVPatternROP[16] =
{
   0x00,
   0xA0,
   0x50,
   0xF0,
   0x0A,
   0xAA,
   0x5A,
   0xFA,
   0x05,
   0xA5,
   0x55,
   0xF5,
   0x0F,
   0xAF,
   0x5F,
   0xFF
};
#endif

void
NVDmaKickoff(NVPtr pNv)
{
    if(pNv->dmaCurrent != pNv->dmaPut) {
        pNv->dmaPut = pNv->dmaCurrent;
        WRITE_PUT(pNv,  pNv->dmaPut);
    }
}


/* There is a HW race condition with videoram command buffers.
   You can't jump to the location of your put offset.  We write put
   at the jump offset + SKIPS dwords with noop padding in between
   to solve this problem */
#define SKIPS  8

void 
NVDmaWait (
   NVPtr pNv,
   int size
){
    int dmaGet;

    size++;

    while(pNv->dmaFree < size) {
       dmaGet = READ_GET(pNv);

       if(pNv->dmaPut >= dmaGet) {
           pNv->dmaFree = pNv->dmaMax - pNv->dmaCurrent;
           if(pNv->dmaFree < size) {
               NVDmaNext(pNv, 0x20000000);
               if(dmaGet <= SKIPS) {
                   if(pNv->dmaPut <= SKIPS) /* corner case - will be idle */
                      WRITE_PUT(pNv, SKIPS + 1);
                   do { dmaGet = READ_GET(pNv); }
                   while(dmaGet <= SKIPS);
               }
               WRITE_PUT(pNv, SKIPS);
               pNv->dmaCurrent = pNv->dmaPut = SKIPS;
               pNv->dmaFree = dmaGet - (SKIPS + 1);
           }
       } else 
           pNv->dmaFree = dmaGet - pNv->dmaCurrent - 1;
    }
}

void
NVWaitVSync(NVPtr pNv)
{
    NVDmaStart(pNv, 0x0000A12C, 1);
    NVDmaNext (pNv, 0);
    NVDmaStart(pNv, 0x0000A134, 1);
    NVDmaNext (pNv, pNv->CRTCnumber);
    NVDmaStart(pNv, 0x0000A100, 1);
    NVDmaNext (pNv, 0);
    NVDmaStart(pNv, 0x0000A130, 1);
    NVDmaNext (pNv, 0);
}

/* 
  currentRop =  0-15  solid fill
               16-31  8x8 pattern fill
               32-47  solid fill with planemask 
*/

static void 
NVSetPattern(
   ScrnInfoPtr pScrn,
   CARD32 clr0,
   CARD32 clr1,
   CARD32 pat0,
   CARD32 pat1
)
{
    NVPtr pNv = NVPTR(pScrn);

    NVDmaStart(pNv, PATTERN_COLOR_0, 4);
    NVDmaNext (pNv, clr0);
    NVDmaNext (pNv, clr1);
    NVDmaNext (pNv, pat0);
    NVDmaNext (pNv, pat1);
}

void 
NVSetRopSolid(ScrnInfoPtr pScrn, CARD32 rop, CARD32 planemask)
{
    NVPtr pNv = NVPTR(pScrn);

    if(planemask != ~0) {
        NVSetPattern(pScrn, 0, planemask, ~0, ~0);
        if(pNv->currentRop != (rop + 32)) {
           NVDmaStart(pNv, ROP_SET, 1);
           NVDmaNext (pNv, NVCopyROP_PM[rop]);
           pNv->currentRop = rop + 32;
        }
    } else 
    if (pNv->currentRop != rop) {
        if(pNv->currentRop >= 16)
             NVSetPattern(pScrn, ~0, ~0, ~0, ~0);
        NVDmaStart(pNv, ROP_SET, 1);
        NVDmaNext (pNv, NVCopyROP[rop]);
        pNv->currentRop = rop;
    }
}

void NVResetGraphics(ScrnInfoPtr pScrn)
{
    NVPtr pNv = NVPTR(pScrn);
    CARD32 surfaceFormat, patternFormat, rectFormat, lineFormat;
    int pitch, i;

    if(pNv->NoAccel) return;

    pitch = pNv->CurrentLayout.displayWidth * 
            (pNv->CurrentLayout.bitsPerPixel >> 3);

    pNv->dmaBase = (CARD32*)(&pNv->FbStart[pNv->FbUsableSize]);

    for(i = 0; i < SKIPS; i++)
      pNv->dmaBase[i] = 0x00000000;

    pNv->dmaBase[0x0 + SKIPS] = 0x00040000;
    pNv->dmaBase[0x1 + SKIPS] = 0x80000010;
    pNv->dmaBase[0x2 + SKIPS] = 0x00042000;
    pNv->dmaBase[0x3 + SKIPS] = 0x80000011;
    pNv->dmaBase[0x4 + SKIPS] = 0x00044000;
    pNv->dmaBase[0x5 + SKIPS] = 0x80000012;
    pNv->dmaBase[0x6 + SKIPS] = 0x00046000;
    pNv->dmaBase[0x7 + SKIPS] = 0x80000013;
    pNv->dmaBase[0x8 + SKIPS] = 0x00048000;
    pNv->dmaBase[0x9 + SKIPS] = 0x80000014;
    pNv->dmaBase[0xA + SKIPS] = 0x0004A000;
    pNv->dmaBase[0xB + SKIPS] = 0x80000015;
    pNv->dmaBase[0xC + SKIPS] = 0x0004C000;
    pNv->dmaBase[0xD + SKIPS] = 0x80000016;
    pNv->dmaBase[0xE + SKIPS] = 0x0004E000;
    pNv->dmaBase[0xF + SKIPS] = 0x80000017;

    pNv->dmaPut = 0;
    pNv->dmaCurrent = 16 + SKIPS;
    pNv->dmaMax = 8191;
    pNv->dmaFree = pNv->dmaMax - pNv->dmaCurrent;

    switch(pNv->CurrentLayout.depth) {
    case 24:
       surfaceFormat = SURFACE_FORMAT_DEPTH24;
       patternFormat = PATTERN_FORMAT_DEPTH24;
       rectFormat    = RECT_FORMAT_DEPTH24;
       lineFormat    = LINE_FORMAT_DEPTH24;
       break;
    case 16:
    case 15:
       surfaceFormat = SURFACE_FORMAT_DEPTH16;
       patternFormat = PATTERN_FORMAT_DEPTH16;
       rectFormat    = RECT_FORMAT_DEPTH16;
       lineFormat    = LINE_FORMAT_DEPTH16;
       break;
    default:
       surfaceFormat = SURFACE_FORMAT_DEPTH8;
       patternFormat = PATTERN_FORMAT_DEPTH8;
       rectFormat    = RECT_FORMAT_DEPTH8;
       lineFormat    = LINE_FORMAT_DEPTH8;
       break;
    }

    NVDmaStart(pNv, SURFACE_FORMAT, 4);
    NVDmaNext (pNv, surfaceFormat);
    NVDmaNext (pNv, pitch | (pitch << 16));
    NVDmaNext (pNv, 0);
    NVDmaNext (pNv, 0);

    NVDmaStart(pNv, PATTERN_FORMAT, 1);
    NVDmaNext (pNv, patternFormat);

    NVDmaStart(pNv, RECT_FORMAT, 1);
    NVDmaNext (pNv, rectFormat);

    NVDmaStart(pNv, LINE_FORMAT, 1);
    NVDmaNext (pNv, lineFormat);

    pNv->currentRop = ~0;  /* set to something invalid */
    NVSetRopSolid(pScrn, GXcopy, ~0);

    NVDmaKickoff(pNv);
}

void NVSync(ScrnInfoPtr pScrn)
{
    NVPtr pNv = NVPTR(pScrn);
    int bail = 10000000;
    
    if(pNv->DMAKickoffCallback)
       (*pNv->DMAKickoffCallback)(pScrn);

    while((READ_GET(pNv) != pNv->dmaPut) && (bail > 0)) bail--;
    if (bail == 0) {
    	xf86Msg(X_ERROR, "DMA drain timeout\n");
    	goto crap;
    }
    
    bail = 10000000;
    while((pNv->PGRAPH[0x0700/4]) && (bail > 0)) bail--;
    if (bail == 0) {
    	xf86Msg(X_ERROR, "engine stalled\n");
    	goto crap;
    }
    return;
    
crap:
    NVResetGraphics(pScrn);
}

void
NVDMAKickoffCallback (ScrnInfoPtr pScrn)
{
   NVPtr pNv = NVPTR(pScrn);

   NVDmaKickoff(pNv);
   pNv->DMAKickoffCallback = NULL;
}

#ifdef HAVE_XAA_H
static void
NVSetupForScreenToScreenCopy(
   ScrnInfoPtr pScrn, 
   int xdir, int ydir, 
   int rop,
   unsigned planemask, 
   int transparency_color
)
{
    NVPtr pNv = NVPTR(pScrn);

    planemask |= ~0 << pNv->CurrentLayout.depth;

    NVSetRopSolid(pScrn, rop, planemask);

    pNv->DMAKickoffCallback = NVDMAKickoffCallback;
}

static void
NVSubsequentScreenToScreenCopy(
   ScrnInfoPtr pScrn, 
   int x1, int y1,
   int x2, int y2, 
   int w, int h
)
{
    NVPtr pNv = NVPTR(pScrn);

    NVDmaStart(pNv, BLIT_POINT_SRC, 3);
    NVDmaNext (pNv, (y1 << 16) | x1);
    NVDmaNext (pNv, (y2 << 16) | x2);
    NVDmaNext (pNv, (h  << 16) | w);

    if((w * h) >= 512)
       NVDmaKickoff(pNv); 
}

static void
NVSetupForSolidFill(
   ScrnInfoPtr pScrn, 
   int color, 
   int rop,
   unsigned planemask
)
{
   NVPtr pNv = NVPTR(pScrn);

   planemask |= ~0 << pNv->CurrentLayout.depth;

   NVSetRopSolid(pScrn, rop, planemask);
   NVDmaStart(pNv, RECT_SOLID_COLOR, 1);
   NVDmaNext (pNv, color);

   pNv->DMAKickoffCallback = NVDMAKickoffCallback;
}

static void
NVSubsequentSolidFillRect(ScrnInfoPtr pScrn, int x, int y, int w, int h)
{
   NVPtr pNv = NVPTR(pScrn);

   NVDmaStart(pNv, RECT_SOLID_RECTS(0), 2);
   NVDmaNext (pNv, (x << 16) | y);
   NVDmaNext (pNv, (w << 16) | h);

   if((w * h) >= 512)
      NVDmaKickoff(pNv);
}

static void
NVSetupForMono8x8PatternFill (
   ScrnInfoPtr pScrn, 
   int patternx, int patterny,
   int fg, int bg, 
   int rop, 
   unsigned planemask
)
{
   NVPtr pNv = NVPTR(pScrn);

   planemask = ~0 << pNv->CurrentLayout.depth;

   fg |= planemask;
   if(bg == -1) bg = 0;
   else bg |= planemask;

   if (pNv->currentRop != (rop + 16)) {
       NVDmaStart(pNv, ROP_SET, 1);
       NVDmaNext (pNv, NVPatternROP[rop]);
       pNv->currentRop = rop + 16;
   }

   NVSetPattern(pScrn, bg, fg, patternx, patterny);
   NVDmaStart(pNv, RECT_SOLID_COLOR, 1);
   NVDmaNext (pNv, fg);

   pNv->DMAKickoffCallback = NVDMAKickoffCallback;
}

static void
NVSubsequentMono8x8PatternFillRect(
   ScrnInfoPtr pScrn,
   int patternx, int patterny,
   int x, int y, 
   int w, int h
)
{
   NVPtr pNv = NVPTR(pScrn);

   NVDmaStart(pNv, RECT_SOLID_RECTS(0), 2);
   NVDmaNext (pNv, (x << 16) | y);
   NVDmaNext (pNv, (w << 16) | h);

   if((w * h) >= 512)
      NVDmaKickoff(pNv);
}

static CARD32 _bg_pixel;
static CARD32 _fg_pixel;
static Bool _transparent;
static CARD32 _color_expand_dwords;
static CARD32 _color_expand_offset;
static int _remaining;
static unsigned char *_storage_buffer[1];

static void
NVSetupForScanlineCPUToScreenColorExpandFill (
   ScrnInfoPtr pScrn,
   int fg, int bg,
   int rop,
   unsigned int planemask
)
{
   NVPtr pNv = NVPTR(pScrn);

   CARD32 mask = ~0 << pNv->CurrentLayout.depth;

   planemask |= mask;
   _fg_pixel = fg | mask;

   if(bg == -1) {
      _transparent = TRUE;
   } else {
      _transparent = FALSE;
      _bg_pixel = bg | mask;
   }

   NVSetRopSolid (pScrn, rop, planemask);
}

static void
NVSubsequentScanlineCPUToScreenColorExpandFill (
    ScrnInfoPtr pScrn, 
    int x, int y,
    int w, int h,
    int skipleft
)
{
   NVPtr pNv = NVPTR(pScrn);
   int bw = (w + 31) & ~31;

   _color_expand_dwords = bw >> 5;
   _remaining = h;

   if(_transparent) {
      NVDmaStart(pNv, RECT_EXPAND_ONE_COLOR_CLIP, 5);
      NVDmaNext (pNv, (y << 16) | ((x + skipleft) & 0xFFFF));
      NVDmaNext (pNv, ((y + h) << 16) | ((x + w) & 0xFFFF));
      NVDmaNext (pNv, _fg_pixel);
      NVDmaNext (pNv, (h << 16) | bw);
      NVDmaNext (pNv, (y << 16) | (x & 0xFFFF));
      _color_expand_offset = RECT_EXPAND_ONE_COLOR_DATA(0);
   } else {
      NVDmaStart(pNv, RECT_EXPAND_TWO_COLOR_CLIP, 7);
      NVDmaNext (pNv, (y << 16) | ((x + skipleft) & 0xFFFF));
      NVDmaNext (pNv, ((y + h) << 16) | ((x + w) & 0xFFFF));
      NVDmaNext (pNv, _bg_pixel);
      NVDmaNext (pNv, _fg_pixel);
      NVDmaNext (pNv, (h << 16) | bw);
      NVDmaNext (pNv, (h << 16) | bw);
      NVDmaNext (pNv, (y << 16) | (x & 0xFFFF));
      _color_expand_offset = RECT_EXPAND_TWO_COLOR_DATA(0); 
   }

   NVDmaStart(pNv, _color_expand_offset, _color_expand_dwords);
   _storage_buffer[0] = (unsigned char*)&pNv->dmaBase[pNv->dmaCurrent];
}

static void
NVSubsequentColorExpandScanline(ScrnInfoPtr pScrn, int bufno)
{
   NVPtr pNv = NVPTR(pScrn);

   pNv->dmaCurrent += _color_expand_dwords;

   if(--_remaining) {
       NVDmaStart(pNv, _color_expand_offset, _color_expand_dwords);
       _storage_buffer[0] = (unsigned char*)&pNv->dmaBase[pNv->dmaCurrent];
   } else {
       /* hardware bug workaround */
       NVDmaStart(pNv, BLIT_POINT_SRC, 1);
       NVDmaNext (pNv, 0);
       NVDmaKickoff(pNv);
   }
}

static void 
NVSetupForScanlineImageWrite(
   ScrnInfoPtr pScrn, int rop, 
   unsigned int planemask, 
   int trans_color, 
   int bpp, int depth
)
{
   NVPtr pNv = NVPTR(pScrn);

   planemask |= ~0 << pNv->CurrentLayout.depth;

   NVSetRopSolid (pScrn, rop, planemask);
}

static CARD32 _image_size;
static CARD32 _image_srcpoint;
static CARD32 _image_dstpoint;
static CARD32 _image_dstpitch;

static void 
NVSubsequentScanlineImageWriteRect(
   ScrnInfoPtr pScrn, 
   int x, int y, 
   int w, int h, 
   int skipleft
)
{
   NVPtr pNv = NVPTR(pScrn);
   int Bpp = pNv->CurrentLayout.bitsPerPixel >> 3;
   int image_srcpitch;

   _image_size = (1 << 16) | (w - skipleft);
   _image_srcpoint = skipleft;
   _image_dstpoint = (y << 16) | (x + skipleft);
   _remaining = h;
   _image_dstpitch = pNv->CurrentLayout.displayWidth * Bpp;
   image_srcpitch =  ((w * Bpp) + 63) & ~63;
   _storage_buffer[0] = pNv->FbStart + pNv->ScratchBufferStart;

   NVSync(pScrn);

   NVDmaStart(pNv, SURFACE_PITCH, 2);
   NVDmaNext (pNv, (_image_dstpitch << 16) | image_srcpitch);
   NVDmaNext (pNv, pNv->ScratchBufferStart);
}

static void NVSubsequentImageWriteScanline(ScrnInfoPtr pScrn, int bufno)
{
   NVPtr pNv = NVPTR(pScrn);

   NVDmaStart(pNv, BLIT_POINT_SRC, 3);
   NVDmaNext (pNv, _image_srcpoint);
   NVDmaNext (pNv, _image_dstpoint);
   NVDmaNext (pNv, _image_size);
   NVDmaKickoff(pNv);

   if(--_remaining) {
      _image_dstpoint += (1 << 16);
      NVSync(pScrn);
   } else {
      NVDmaStart(pNv, SURFACE_PITCH, 2);
      NVDmaNext (pNv, _image_dstpitch | (_image_dstpitch << 16));
      NVDmaNext (pNv, 0);
   }
}

static void
NVSetupForSolidLine(ScrnInfoPtr pScrn, int color, int rop, unsigned planemask)
{
    NVPtr pNv = NVPTR(pScrn);

    planemask |= ~0 << pNv->CurrentLayout.depth;

    NVSetRopSolid(pScrn, rop, planemask);

    _fg_pixel = color;

    pNv->DMAKickoffCallback = NVDMAKickoffCallback;
}

static void 
NVSubsequentSolidHorVertLine(ScrnInfoPtr pScrn, int x, int y, int len, int dir)
{
    NVPtr pNv = NVPTR(pScrn);

    NVDmaStart(pNv, LINE_COLOR, 1);
    NVDmaNext (pNv, _fg_pixel);
    NVDmaStart(pNv, LINE_LINES(0), 2);
    NVDmaNext (pNv, (y << 16) | ( x & 0xffff));
    if(dir == DEGREES_0) {
       NVDmaNext (pNv, (y << 16) | ((x + len) & 0xffff));
    } else {
       NVDmaNext (pNv, ((y + len) << 16) | (x & 0xffff));
    }
}

static void 
NVSubsequentSolidTwoPointLine(
   ScrnInfoPtr pScrn, 
   int x1, int y1,
   int x2, int y2, 
   int flags
)
{
    NVPtr pNv = NVPTR(pScrn);
    Bool drawLast = !(flags & OMIT_LAST);

    NVDmaStart(pNv, LINE_COLOR, 1);
    NVDmaNext (pNv, _fg_pixel);
    NVDmaStart(pNv, LINE_LINES(0), drawLast ? 4 : 2);
    NVDmaNext (pNv, (y1 << 16) | (x1 & 0xffff));
    NVDmaNext (pNv, (y2 << 16) | (x2 & 0xffff));
    if(drawLast) {
        NVDmaNext (pNv, (y2 << 16) | (x2 & 0xffff));
        NVDmaNext (pNv, ((y2 + 1) << 16) | (x2 & 0xffff));
    }
}

static void
NVSetClippingRectangle(ScrnInfoPtr pScrn, int x1, int y1, int x2, int y2)
{
    NVPtr pNv = NVPTR(pScrn);
    int h = y2 - y1 + 1;
    int w = x2 - x1 + 1;

    NVDmaStart(pNv, CLIP_POINT, 2);
    NVDmaNext (pNv, (y1 << 16) | x1); 
    NVDmaNext (pNv, (h << 16) | w);
}

static void
NVDisableClipping(ScrnInfoPtr pScrn)
{
    NVPtr pNv = NVPTR(pScrn);

    NVDmaStart(pNv, CLIP_POINT, 2);
    NVDmaNext (pNv, 0);              
    NVDmaNext (pNv, 0x7FFF7FFF);
}

#endif

/* Initialize XAA acceleration info */
Bool
NVAccelInit(ScreenPtr pScreen) 
{
#ifdef HAVE_XAA_H
   ScrnInfoPtr pScrn = xf86ScreenToScrn(pScreen);
   NVPtr pNv = NVPTR(pScrn);
   XAAInfoRecPtr accel;

   accel = pNv->AccelInfoRec = XAACreateInfoRec();
   if(!accel) return FALSE;

   accel->Flags = LINEAR_FRAMEBUFFER | PIXMAP_CACHE | OFFSCREEN_PIXMAPS;
   accel->Sync = NVSync;

   accel->ScreenToScreenCopyFlags = NO_TRANSPARENCY;
   accel->SetupForScreenToScreenCopy = NVSetupForScreenToScreenCopy;
   accel->SubsequentScreenToScreenCopy = NVSubsequentScreenToScreenCopy;

   accel->SolidFillFlags = 0;
   accel->SetupForSolidFill = NVSetupForSolidFill;
   accel->SubsequentSolidFillRect = NVSubsequentSolidFillRect;

   accel->Mono8x8PatternFillFlags = HARDWARE_PATTERN_SCREEN_ORIGIN |
                                    HARDWARE_PATTERN_PROGRAMMED_BITS |
                                    NO_PLANEMASK;
   accel->SetupForMono8x8PatternFill = NVSetupForMono8x8PatternFill;
   accel->SubsequentMono8x8PatternFillRect = NVSubsequentMono8x8PatternFillRect;

   accel->ScanlineCPUToScreenColorExpandFillFlags = 
                                    BIT_ORDER_IN_BYTE_LSBFIRST |
                                    CPU_TRANSFER_PAD_DWORD |
                                    LEFT_EDGE_CLIPPING |
                                    LEFT_EDGE_CLIPPING_NEGATIVE_X;
   accel->NumScanlineColorExpandBuffers = 1;
   accel->SetupForScanlineCPUToScreenColorExpandFill = 
            NVSetupForScanlineCPUToScreenColorExpandFill;
   accel->SubsequentScanlineCPUToScreenColorExpandFill = 
            NVSubsequentScanlineCPUToScreenColorExpandFill;
   accel->SubsequentColorExpandScanline = 
            NVSubsequentColorExpandScanline;
   accel->ScanlineColorExpandBuffers = _storage_buffer;

   accel->ScanlineImageWriteFlags = NO_GXCOPY |
                                    NO_TRANSPARENCY |
                                    LEFT_EDGE_CLIPPING |
                                    LEFT_EDGE_CLIPPING_NEGATIVE_X;
   accel->NumScanlineImageWriteBuffers = 1;
   accel->SetupForScanlineImageWrite = NVSetupForScanlineImageWrite;
   accel->SubsequentScanlineImageWriteRect = NVSubsequentScanlineImageWriteRect;
   accel->SubsequentImageWriteScanline = NVSubsequentImageWriteScanline;
   accel->ScanlineImageWriteBuffers = _storage_buffer;

   accel->SolidLineFlags = 0;
   accel->SetupForSolidLine = NVSetupForSolidLine;
   accel->SubsequentSolidHorVertLine = NVSubsequentSolidHorVertLine;
   accel->SubsequentSolidTwoPointLine = NVSubsequentSolidTwoPointLine;
   accel->SetClippingRectangle = NVSetClippingRectangle;
   accel->DisableClipping = NVDisableClipping;
   accel->ClippingFlags = HARDWARE_CLIP_SOLID_LINE;
   
   miSetZeroLineBias(pScreen, OCTANT1 | OCTANT3 | OCTANT4 | OCTANT6);

   return (XAAInit(pScreen, accel));
#else
   return FALSE;
#endif
}

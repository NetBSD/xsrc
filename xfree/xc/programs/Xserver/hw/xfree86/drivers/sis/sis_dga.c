/*
 * Copyright 2000 by Alan Hourihane, Sychdyn, North Wales, UK.
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
 */
/* $XFree86: xc/programs/Xserver/hw/xfree86/drivers/sis/sis_dga.c,v 1.2 2001/04/19 12:40:33 alanh Exp $ */

#include "xf86.h"
#include "xf86_OSproc.h"
#include "xf86_ansic.h"
#include "xf86Pci.h"
#include "xf86PciInfo.h"
#include "xaa.h"
#include "xaalocal.h"
#include "sis.h"
#include "sis_regs.h"
#include "dgaproc.h"

Bool SISDGAInit(ScreenPtr pScreen);
static Bool SIS_OpenFramebuffer(ScrnInfoPtr, char **, unsigned char **, 
                    int *, int *, int *);
static Bool SIS_SetMode(ScrnInfoPtr, DGAModePtr);
static void SIS_Sync(ScrnInfoPtr);
static int  SIS_GetViewport(ScrnInfoPtr);
static void SIS_SetViewport(ScrnInfoPtr, int, int, int);
static void SIS_FillRect(ScrnInfoPtr, int, int, int, int, unsigned long);
static void SIS_BlitRect(ScrnInfoPtr, int, int, int, int, int, int);
static void SIS_BlitTransRect(ScrnInfoPtr, int, int, int, int, int, int, 
                    unsigned long);

static
DGAFunctionRec SISDGAFuncs = {
   SIS_OpenFramebuffer,
   NULL,
   SIS_SetMode,
   SIS_SetViewport,
   SIS_GetViewport,
   SIS_Sync,
   SIS_FillRect,
   SIS_BlitRect,
#if 0
   SIS_BlitTransRect,
#else
   NULL
#endif
};

Bool
SISDGAInit(ScreenPtr pScreen)
{   
   ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
   SISPtr pSIS = SISPTR(pScrn);
   DGAModePtr modes = NULL, newmodes = NULL, currentMode;
   DisplayModePtr pMode, firstMode;
   int Bpp = pScrn->bitsPerPixel >> 3;
   int num = 0;
   Bool oneMore;

   pMode = firstMode = pScrn->modes;

   while(pMode) {

#if 0
    if(pScrn->displayWidth != pMode->HDisplay) {
        newmodes = xrealloc(modes, (num + 2) * sizeof(DGAModeRec));
        oneMore = TRUE;
    } else {
#endif
        newmodes = xrealloc(modes, (num + 1) * sizeof(DGAModeRec));
        oneMore = FALSE;
    /* } */

    if(!newmodes) {
       xfree(modes);
       return FALSE;
    }
    modes = newmodes;

SECOND_PASS:

    currentMode = modes + num;
    num++;

    currentMode->mode = pMode;
    currentMode->flags = DGA_CONCURRENT_ACCESS | DGA_PIXMAP_AVAILABLE;
    currentMode->flags |= DGA_FILL_RECT | DGA_BLIT_RECT;
    if(pMode->Flags & V_DBLSCAN)
       currentMode->flags |= DGA_DOUBLESCAN;
    if(pMode->Flags & V_INTERLACE)
       currentMode->flags |= DGA_INTERLACED;
    currentMode->byteOrder = pScrn->imageByteOrder;
    currentMode->depth = pScrn->depth;
    currentMode->bitsPerPixel = pScrn->bitsPerPixel;
    currentMode->red_mask = pScrn->mask.red;
    currentMode->green_mask = pScrn->mask.green;
    currentMode->blue_mask = pScrn->mask.blue;
    currentMode->visualClass = (Bpp == 1) ? PseudoColor : TrueColor;
    currentMode->viewportWidth = pMode->HDisplay;
    currentMode->viewportHeight = pMode->VDisplay;
    currentMode->xViewportStep = 1;
    currentMode->yViewportStep = 1;
    currentMode->viewportFlags = DGA_FLIP_RETRACE;
    currentMode->offset = 0;
    currentMode->address = pSIS->FbBase;

    if(oneMore) { /* first one is narrow width */
        currentMode->bytesPerScanline = ((pMode->HDisplay * Bpp) + 3) & ~3L;
        currentMode->imageWidth = pMode->HDisplay;
        currentMode->imageHeight =  pMode->VDisplay;
        currentMode->pixmapWidth = currentMode->imageWidth;
        currentMode->pixmapHeight = currentMode->imageHeight;
        currentMode->maxViewportX = currentMode->imageWidth - 
                    currentMode->viewportWidth;
        /* this might need to get clamped to some maximum */
        currentMode->maxViewportY = currentMode->imageHeight -
                    currentMode->viewportHeight;
        oneMore = FALSE;
        goto SECOND_PASS;
    } else {
        currentMode->bytesPerScanline = 
            ((pScrn->displayWidth * Bpp) + 3) & ~3L;
        currentMode->imageWidth = pScrn->displayWidth;
        currentMode->imageHeight =  pMode->VDisplay;
        currentMode->pixmapWidth = currentMode->imageWidth;
        currentMode->pixmapHeight = currentMode->imageHeight;
        currentMode->maxViewportX = currentMode->imageWidth - 
                    currentMode->viewportWidth;
        /* this might need to get clamped to some maximum */
        currentMode->maxViewportY = currentMode->imageHeight -
                    currentMode->viewportHeight;
    }       

    pMode = pMode->next;
    if(pMode == firstMode)
       break;
   }

   pSIS->numDGAModes = num;
   pSIS->DGAModes = modes;

   return DGAInit(pScreen, &SISDGAFuncs, modes, num);  
}


static Bool
SIS_SetMode(
   ScrnInfoPtr pScrn,
   DGAModePtr pMode
){
   static int OldDisplayWidth[MAXSCREENS];
   int index = pScrn->pScreen->myNum;
   SISPtr pSIS = SISPTR(pScrn);

    if(!pMode) { /* restore the original mode */
        /* put the ScreenParameters back */
        pScrn->displayWidth = OldDisplayWidth[index];

        (*pScrn->SwitchMode)(index, pScrn->currentMode, 0);
        pSIS->DGAactive = FALSE;
    } else {
        if(!pSIS->DGAactive) {  /* save the old parameters */
            OldDisplayWidth[index] = pScrn->displayWidth;

            pSIS->DGAactive = TRUE;
    }

    pScrn->displayWidth = pMode->bytesPerScanline / 
      (pMode->bitsPerPixel >> 3);

    (*pScrn->SwitchMode)(index, pMode->mode, 0);
    }
    return TRUE;
}

static int  
SIS_GetViewport(
  ScrnInfoPtr pScrn
){
    SISPtr pSIS = SISPTR(pScrn);

    return pSIS->DGAViewportStatus;
}

static void 
SIS_SetViewport(
   ScrnInfoPtr pScrn, 
   int x, int y, 
   int flags
){
   SISPtr pSIS = SISPTR(pScrn);

   (*pScrn->AdjustFrame)(pScrn->pScreen->myNum, x, y, flags);
   pSIS->DGAViewportStatus = 0;  /* SISAdjustFrame loops until finished */
}

static void 
SIS_FillRect (
   ScrnInfoPtr pScrn, 
   int x, int y, int w, int h, 
   unsigned long color
){
    SISPtr pSIS = SISPTR(pScrn);

    if(pSIS->AccelInfoPtr) {
    (*pSIS->AccelInfoPtr->SetupForSolidFill)(pScrn, color, GXcopy, ~0);
    (*pSIS->AccelInfoPtr->SubsequentSolidFillRect)(pScrn, x, y, w, h);
    SET_SYNC_FLAG(pSIS->AccelInfoPtr);
    }
}

static void 
SIS_Sync(
   ScrnInfoPtr pScrn
){
    SISPtr pSIS = SISPTR(pScrn);

    if(pSIS->AccelInfoPtr) {
    (*pSIS->AccelInfoPtr->Sync)(pScrn);
    }
}

static void 
SIS_BlitRect(
   ScrnInfoPtr pScrn, 
   int srcx, int srcy, 
   int w, int h, 
   int dstx, int dsty
){
    SISPtr pSIS = SISPTR(pScrn);

    if(pSIS->AccelInfoPtr) {
    int xdir = ((srcx < dstx) && (srcy == dsty)) ? -1 : 1;
    int ydir = (srcy < dsty) ? -1 : 1;

    (*pSIS->AccelInfoPtr->SetupForScreenToScreenCopy)(
        pScrn, xdir, ydir, GXcopy, ~0, -1);
    (*pSIS->AccelInfoPtr->SubsequentScreenToScreenCopy)(
        pScrn, srcx, srcy, dstx, dsty, w, h);
    SET_SYNC_FLAG(pSIS->AccelInfoPtr);
    }
}


static void 
SIS_BlitTransRect(
   ScrnInfoPtr pScrn, 
   int srcx, int srcy, 
   int w, int h, 
   int dstx, int dsty,
   unsigned long color
){
  /* this one should be separate since the XAA function would
     prohibit usage of ~0 as the key */
}


static Bool 
SIS_OpenFramebuffer(
   ScrnInfoPtr pScrn, 
   char **name,
   unsigned char **mem,
   int *size,
   int *offset,
   int *flags
){
    SISPtr pSIS = SISPTR(pScrn);

    *name = NULL;       /* no special device */
    *mem = (unsigned char*)pSIS->FbAddress;
    *size = pSIS->FbMapSize;
    *offset = 0;
    *flags = DGA_NEED_ROOT;

    return TRUE;
}

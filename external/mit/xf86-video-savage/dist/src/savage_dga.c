
/*
Copyright (C) 1994-2000 The XFree86 Project, Inc.  All Rights Reserved.

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FIT-
NESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
XFREE86 PROJECT BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of the XFree86 Project shall not
be used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from the XFree86 Project.
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

/*
 * file: savage_dga.c
 * ported from s3v, which was ported from mga
 *
 */

#ifdef HAVE_XAA_H
#include "xaalocal.h"
#endif
#include "savage_driver.h"
#include "dgaproc.h"


Bool SavageDGAInit(ScreenPtr pScreen);
static Bool Savage_OpenFramebuffer(ScrnInfoPtr, char **, unsigned char **, 
		int *, int *, int *);
static Bool Savage_SetMode(ScrnInfoPtr, DGAModePtr);
static int  Savage_GetViewport(ScrnInfoPtr);
static void Savage_SetViewport(ScrnInfoPtr, int, int, int);
#ifdef HAVE_XAA_H
static void Savage_FillRect(ScrnInfoPtr, int, int, int, int, unsigned long);
static void Savage_BlitRect(ScrnInfoPtr, int, int, int, int, int, int);
#endif

static
DGAFunctionRec Savage_DGAFuncs = {
    Savage_OpenFramebuffer,
    NULL,	/* CloseFrameBuffer */
    Savage_SetMode,
    Savage_SetViewport,
    Savage_GetViewport,
    SavageAccelSync,
#ifdef HAVE_XAA_H
    Savage_FillRect,
    Savage_BlitRect,
#else
    NULL, NULL,
#endif
    NULL			 /* BlitTransRect */
};

#define DGATRACE	4

/*
 * I don't understand the thinking here.  As near as I can tell, we are
 * never asked to change into a depth other than the frame buffer depth.
 * So why create modes to do so?
 */

static DGAModePtr
SavageSetupDGAMode(
    ScrnInfoPtr pScrn,
    DGAModePtr modes,
    int *num,
    int bitsPerPixel,
    int depth,
    Bool pixmap,
    int secondPitch,
    unsigned long red,
    unsigned long green,
    unsigned long blue,
    short visualClass
)
{
    SavagePtr psav = SAVPTR(pScrn);
    DGAModePtr mode, newmodes = NULL;
    DisplayModePtr pMode, firstMode;
    int otherPitch, Bpp = bitsPerPixel >> 3;
    Bool oneMore;

    xf86ErrorFVerb(DGATRACE, "		SavageSetupDGAMode\n");

    pMode = firstMode = pScrn->modes;

    /*
     * DGA 1.0 would only provide modes where the depth and stride
     * matched the current desktop.  Some DGA apps might still expect
     * this, so we provide them, too.
     */

    while(pMode) {

	otherPitch = secondPitch ? secondPitch : pMode->HDisplay;

	if(pMode->HDisplay != otherPitch) {
	    newmodes = realloc(modes, (*num + 2) * sizeof(DGAModeRec));
	    oneMore = TRUE;
	} else {
	    newmodes = realloc(modes, (*num + 1) * sizeof(DGAModeRec));
	    oneMore = FALSE;
	}

	if(!newmodes) {
	   free(modes);
	   return NULL;
	}
	modes = newmodes;

SECOND_PASS:

	mode = modes + *num;
	(*num)++;

	mode->mode = pMode;
	mode->flags = DGA_CONCURRENT_ACCESS | DGA_PIXMAP_AVAILABLE;
#ifdef HAVE_XAA_H
	if(!psav->NoAccel)
	    mode->flags |= DGA_FILL_RECT | DGA_BLIT_RECT;
#endif
	if(pMode->Flags & V_DBLSCAN)
	    mode->flags |= DGA_DOUBLESCAN;
	if(pMode->Flags & V_INTERLACE)
	    mode->flags |= DGA_INTERLACED;
	mode->byteOrder = pScrn->imageByteOrder;
	mode->depth = depth;
	mode->bitsPerPixel = bitsPerPixel;
	mode->red_mask = red;
	mode->green_mask = green;
	mode->blue_mask = blue;
	mode->visualClass = visualClass;
	mode->viewportWidth = pMode->HDisplay;
	mode->viewportHeight = pMode->VDisplay;
	mode->xViewportStep = 2;
	mode->yViewportStep = 1;
	mode->viewportFlags = DGA_FLIP_RETRACE;
	mode->offset = 0;
	mode->address = psav->FBBase;

	xf86ErrorFVerb(DGATRACE,
	    "SavageDGAInit vpWid=%d, vpHgt=%d, Bpp=%d, mdbitsPP=%d\n",
	    mode->viewportWidth,
	    mode->viewportHeight,
	    Bpp,
	    mode->bitsPerPixel
	);

	if(oneMore) { /* first one is narrow width */
	    /* Force stride to multiple of 16 pixels. */
	    mode->bytesPerScanline = ((pMode->HDisplay + 15) & ~15) * Bpp;
	    mode->imageWidth = pMode->HDisplay;
	    mode->imageHeight =  pMode->VDisplay;
	    mode->pixmapWidth = mode->imageWidth;
	    mode->pixmapHeight = mode->imageHeight;
	    mode->maxViewportX = mode->imageWidth - mode->viewportWidth;
	    /* this might need to get clamped to some maximum */
	    mode->maxViewportY = mode->imageHeight - mode->viewportHeight;
	    oneMore = FALSE;

	    xf86ErrorFVerb(DGATRACE,
		"SavageDGAInit 1 imgHgt=%d, stride=%d\n",
		mode->imageHeight,
		mode->bytesPerScanline );
 
	    goto SECOND_PASS;
	} else {
	    mode->bytesPerScanline = ((pScrn->displayWidth + 15) & ~15) * Bpp;
	    mode->imageWidth = pScrn->displayWidth;
	    mode->imageHeight = psav->videoRambytes / mode->bytesPerScanline;
	    mode->pixmapWidth = mode->imageWidth;
	    mode->pixmapHeight = mode->imageHeight;
	    mode->maxViewportX = mode->imageWidth - mode->viewportWidth;
	    /* this might need to get clamped to some maximum */
	    mode->maxViewportY = mode->imageHeight - mode->viewportHeight;

	    xf86ErrorFVerb(DGATRACE,
		"SavageDGAInit 2 imgHgt=%d, stride=%d\n",
		mode->imageHeight,
		mode->bytesPerScanline );
	}		

	pMode = pMode->next;
	if(pMode == firstMode)
	    break;
    }

    return modes;
}


Bool
SavageDGAInit(ScreenPtr pScreen)
{   
    ScrnInfoPtr pScrn = xf86ScreenToScrn(pScreen);
    SavagePtr psav = SAVPTR(pScrn);
    DGAModePtr modes = NULL;
    int num = 0;

    xf86ErrorFVerb(DGATRACE, "		SavageDGAInit\n");

    /* 8 */
    modes = SavageSetupDGAMode (pScrn, modes, &num, 8, 8, 
		(pScrn->bitsPerPixel == 8),
		(pScrn->bitsPerPixel != 8) ? 0 : pScrn->displayWidth,
		0, 0, 0, PseudoColor);

    /* 15 */
    modes = SavageSetupDGAMode (pScrn, modes, &num, 16, 15, 
		(pScrn->bitsPerPixel == 16),
		(pScrn->depth != 15) ? 0 : pScrn->displayWidth,
		0x7c00, 0x03e0, 0x001f, TrueColor);

    modes = SavageSetupDGAMode (pScrn, modes, &num, 16, 15, 
		(pScrn->bitsPerPixel == 16),
		(pScrn->depth != 15) ? 0 : pScrn->displayWidth,
		0x7c00, 0x03e0, 0x001f, DirectColor);

    /* 16 */
    modes = SavageSetupDGAMode (pScrn, modes, &num, 16, 16, 
		(pScrn->bitsPerPixel == 16),
		(pScrn->depth != 16) ? 0 : pScrn->displayWidth,
		0xf800, 0x07e0, 0x001f, TrueColor);

    modes = SavageSetupDGAMode (pScrn, modes, &num, 16, 16, 
		(pScrn->bitsPerPixel == 16),
		(pScrn->depth != 16) ? 0 : pScrn->displayWidth,
		0xf800, 0x07e0, 0x001f, DirectColor);

    /* 24-in-32 */
    modes = SavageSetupDGAMode (pScrn, modes, &num, 32, 24, 
		(pScrn->bitsPerPixel == 32),
		(pScrn->bitsPerPixel != 32) ? 0 : pScrn->displayWidth,
		0xff0000, 0x00ff00, 0x0000ff, TrueColor);

    modes = SavageSetupDGAMode (pScrn, modes, &num, 32, 24, 
		(pScrn->bitsPerPixel == 32),
		(pScrn->bitsPerPixel != 32) ? 0 : pScrn->displayWidth,
		0xff0000, 0x00ff00, 0x0000ff, DirectColor);

    psav->numDGAModes = num;
    psav->DGAModes = modes;

    return DGAInit(pScreen, &Savage_DGAFuncs, modes, num);  
}


static Bool
Savage_SetMode(
    ScrnInfoPtr pScrn,
    DGAModePtr pMode
){
    static int OldDisplayWidth[MAXSCREENS];
    static int OldBitsPerPixel[MAXSCREENS];
    static int OldDepth[MAXSCREENS];
    static DisplayModePtr OldMode[MAXSCREENS];
    int index = pScrn->pScreen->myNum;
    SavagePtr psav = SAVPTR(pScrn);

    if(!pMode) { /* restore the original mode */
	/* put the ScreenParameters back */

	pScrn->displayWidth = OldDisplayWidth[index];
	pScrn->bitsPerPixel = OldBitsPerPixel[index];
	pScrn->depth = OldDepth[index];
	pScrn->currentMode = OldMode[index];

	psav->DGAactive = FALSE;
	SavageSwitchMode(SWITCH_MODE_ARGS(pScrn, pScrn->currentMode));
	if( psav->hwcursor && psav->hwc_on )
	    SavageShowCursor(pScrn);
    } else {
	Bool holdBIOS = psav->UseBIOS;

#if 0
	ErrorF( 
	    "pScrn->bitsPerPixel %d, pScrn->depth %d\n",
	    pScrn->bitsPerPixel, pScrn->depth);
	ErrorF(
	    " want  bitsPerPixel %d,  want  depth %d\n",
	    pMode->bitsPerPixel, pMode->depth);
#endif

	if( psav->hwcursor && psav->hwc_on) {
	    SavageHideCursor(pScrn);
	    psav->hwc_on = TRUE;    /* save for later restoration */
	}
	

	if(!psav->DGAactive) {  /* save the old parameters */
	    OldDisplayWidth[index] = pScrn->displayWidth;
	    OldBitsPerPixel[index] = pScrn->bitsPerPixel;
	    OldDepth[index] = pScrn->depth;
	    OldMode[index] = pScrn->currentMode;

	    psav->DGAactive = TRUE;
	}

	pScrn->bitsPerPixel = pMode->bitsPerPixel;
	pScrn->depth = pMode->depth;
	pScrn->displayWidth = pMode->bytesPerScanline / 
	    (pMode->bitsPerPixel >> 3);

/*	psav->UseBIOS = FALSE; */
	SavageSwitchMode(SWITCH_MODE_ARGS(pScrn, pMode->mode));
	psav->UseBIOS = holdBIOS;
    }

    return TRUE;
}


static int	
Savage_GetViewport(
    ScrnInfoPtr pScrn
){
    SavagePtr psav = SAVPTR(pScrn);
    return psav->DGAViewportStatus;
}


static void 
Savage_SetViewport(
   ScrnInfoPtr pScrn, 
   int x, int y, 
   int flags
){
    SavagePtr psav = SAVPTR(pScrn);

    SavageAdjustFrame(ADJUST_FRAME_ARGS(pScrn, x, y));
    psav->DGAViewportStatus = 0;  /* MGAAdjustFrame loops until finished */
}

#ifdef HAVE_XAA_H
static void 
Savage_FillRect (
    ScrnInfoPtr pScrn, 
    int x, int y, int w, int h, 
    unsigned long color
){
    SavagePtr psav = SAVPTR(pScrn);

    if(psav->AccelInfoRec) {
	(*psav->AccelInfoRec->SetupForSolidFill)(pScrn, color, GXcopy, ~0);
	(*psav->AccelInfoRec->SubsequentSolidFillRect)(pScrn, x, y, w, h);
	SET_SYNC_FLAG(psav->AccelInfoRec);
    }
}

static void 
Savage_BlitRect(
    ScrnInfoPtr pScrn, 
    int srcx, int srcy, 
    int w, int h, 
    int dstx, int dsty
){
    SavagePtr psav = SAVPTR(pScrn);

    if(psav->AccelInfoRec) {
    int xdir = ((srcx < dstx) && (srcy == dsty)) ? -1 : 1;
    int ydir = (srcy < dsty) ? -1 : 1;

    (*psav->AccelInfoRec->SetupForScreenToScreenCopy)(
	pScrn, xdir, ydir, GXcopy, ~0, -1);
    (*psav->AccelInfoRec->SubsequentScreenToScreenCopy)(
	pScrn, srcx, srcy, dstx, dsty, w, h);
    SET_SYNC_FLAG(psav->AccelInfoRec);
    }
}
#endif

static Bool 
Savage_OpenFramebuffer(
    ScrnInfoPtr pScrn, 
    char **name,
    unsigned char **mem,
    int *size,
    int *offset,
    int *flags
){
    SavagePtr psav = SAVPTR(pScrn);

    *name = NULL;	 /* no special device */
    *mem = (unsigned char*)(uintptr_t) psav->FbRegion.base;
    *size = psav->videoRambytes;
    *offset = 0;
    *flags = DGA_NEED_ROOT;

    return TRUE;
}

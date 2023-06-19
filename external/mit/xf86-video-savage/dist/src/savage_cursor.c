/*
 * Copyright (C) 1994-2000 The XFree86 Project, Inc.  All Rights Reserved.
 * Copyright (c) 2003-2006, X.Org Foundation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER(S) OR AUTHOR(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Except as contained in this notice, the name of the copyright holder(s)
 * and author(s) shall not be used in advertising or otherwise to promote
 * the sale, use or other dealings in this Software without prior written
 * authorization from the copyright holder(s) and author(s).
 */

/**
 * \file savage_cursor.c
 * Hardware cursor support for S3 Savage driver. Taken with very few changes
 * from the s3virge cursor file.
 *
 * \author S. Marineau (19/04/97)
 * \author Amancio Hasty
 * \author Jon Tombs
 * \author Tim Roberts
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "savage_driver.h"

static void SavageLoadCursorImage(ScrnInfoPtr pScrn, unsigned char *src);
static void SavageSetCursorPosition(ScrnInfoPtr pScrn, int x, int y);
static void SavageSetCursorColors(ScrnInfoPtr pScrn, int bg, int fg);


/*
 * Read/write to the DAC via MMIO 
 */

#define inCRReg(reg) (VGAHWPTR(pScrn))->readCrtc( VGAHWPTR(pScrn), reg )
#define outCRReg(reg, val) (VGAHWPTR(pScrn))->writeCrtc( VGAHWPTR(pScrn), reg, val )
#define inSRReg(reg) (VGAHWPTR(pScrn))->readSeq( VGAHWPTR(pScrn), reg )
#define outSRReg(reg, val) (VGAHWPTR(pScrn))->writeSeq( VGAHWPTR(pScrn), reg, val )
#if 0
#define inStatus1() (VGAHWPTR(pScrn))->readST01( VGAHWPTR(pScrn) )
#endif

/* 
 * certain HW cursor operations seem 
 * to require a delay to prevent lockups.
 */
#define waitHSync(n) { \
                       int num = n; \
                       while (num--) { \
			 while (inStatus1() & 0x01){};\
                         while (!(inStatus1() & 0x01)){};\
                        } \
                      } 
#define MAX_CURS 64

/*
 * Disable HW Cursor on stretched LCDs. We don't know how to
 * detect if display is stretched. Therefore we cannot rescale
 * the HW cursor position.
 */

#if 0
static Bool
SavageUseHWCursor(ScreenPtr pScr, CursorPtr pCurs)
{
    ScrnInfoPtr pScrn = xf86ScreenToScrn(pScr);
    SavagePtr psav = SAVPTR(pScrn);

    if (psav->PanelX != pScrn->currentMode->HDisplay 
	|| psav->PanelY != pScrn->currentMode->VDisplay) {
	/* BIT 1 : CRT is active, BIT 2 : LCD is active */
	unsigned char cr6d = inCRReg( 0x6d );
	if (cr6d & 0x02)
	    return FALSE;
    }
    return TRUE;
}
#endif

Bool 
SavageHWCursorInit(ScreenPtr pScreen)
{
    ScrnInfoPtr pScrn = xf86ScreenToScrn(pScreen);
    SavagePtr psav = SAVPTR(pScrn);
    xf86CursorInfoPtr infoPtr;

    infoPtr = xf86CreateCursorInfoRec();
    if(!infoPtr) 
        return FALSE;
    
    psav->CursorInfoRec = infoPtr;

    infoPtr->MaxWidth = MAX_CURS;
    infoPtr->MaxHeight = MAX_CURS;
    infoPtr->Flags = HARDWARE_CURSOR_SOURCE_MASK_INTERLEAVE_16 |
		     HARDWARE_CURSOR_SWAP_SOURCE_AND_MASK |
		     HARDWARE_CURSOR_AND_SOURCE_WITH_MASK |
		     HARDWARE_CURSOR_BIT_ORDER_MSBFIRST |
	             HARDWARE_CURSOR_INVERT_MASK;
#if 0
    /*
     * The /MX family is apparently unique among the Savages, in that
     * the cursor color is always straight RGB.  The rest of the Savages
     * use palettized values at 8-bit when not clock doubled.
     */

    if (((psav->Chipset != S3_SAVAGE4) 
	 && (inSRReg(0x18) & 0x80) && (inSRReg(0x15) & 0x50))
	|| S3_SAVAGE_MOBILE_SERIES(psav->Chipset))
	infoPtr->Flags |= HARDWARE_CURSOR_TRUECOLOR_AT_8BPP; 
#endif
    /*
     * With streams engine the Cursor seems to be ALWAYS TrueColor 
     *except at least the Savage4
     */
    if (psav->Chipset != S3_SAVAGE4)
	infoPtr->Flags |= HARDWARE_CURSOR_TRUECOLOR_AT_8BPP; 

    infoPtr->SetCursorColors = SavageSetCursorColors;
    infoPtr->SetCursorPosition = SavageSetCursorPosition;
    infoPtr->LoadCursorImage = SavageLoadCursorImage;
    infoPtr->HideCursor = SavageHideCursor;
    infoPtr->ShowCursor = SavageShowCursor;
    infoPtr->UseHWCursor = NULL;
#if 0 /*AGD:  HW cursor seems to work fine even with expansion... */
    if ((S3_SAVAGE_MOBILE_SERIES(psav->Chipset)
	 || (S3_MOBILE_TWISTER_SERIES(psav->Chipset))) && !psav->CrtOnly)
	infoPtr->UseHWCursor = SavageUseHWCursor;
    else
	infoPtr->UseHWCursor = NULL;
#endif
    if( !psav->CursorKByte )
	psav->CursorKByte = pScrn->videoRam - 4;

    return xf86InitCursor(pScreen, infoPtr);
}

void
SavageShowCursor(ScrnInfoPtr pScrn)
{
    SavagePtr psav = SAVPTR(pScrn);

   /* Turn cursor on. */
   if (psav->IsSecondary) {
       SelectIGA2();
       outCRReg( 0x45, inCRReg(0x45) | 0x01 );
       SelectIGA1();
   } else {
       outCRReg( 0x45, inCRReg(0x45) | 0x01 );
   }
   SAVPTR(pScrn)->hwc_on = TRUE;
}


void
SavageHideCursor(ScrnInfoPtr pScrn)
{
    SavagePtr psav = SAVPTR(pScrn);

    /* Turn cursor off. */

    if( S3_SAVAGE4_SERIES( SAVPTR(pScrn)->Chipset ) )
    {
       waitHSync(5);
    }
    if (psav->IsSecondary) {
        SelectIGA2();
	outCRReg( 0x45, inCRReg(0x45) & 0xfe ); /* cursor2 */
	SelectIGA1();
    } else {
        outCRReg( 0x45, inCRReg(0x45) & 0xfe );
    }
    SAVPTR(pScrn)->hwc_on = FALSE;
}

static void
SavageLoadCursorImage(
    ScrnInfoPtr pScrn,
    unsigned char* src)
{
    SavagePtr psav = SAVPTR(pScrn);

    /* Set cursor location in frame buffer.  */
    if (psav->IsSecondary) {
	SelectIGA2();
    	/* Set cursor location in frame buffer.  */
    	outCRReg( 0x4d, (0xff & psav->CursorKByte));
    	outCRReg( 0x4c, (0xff00 & psav->CursorKByte) >> 8);
	SelectIGA1();
    } else {
        outCRReg( 0x4d, (0xff & (CARD32)psav->CursorKByte));
        outCRReg( 0x4c, (0xff00 & (CARD32)psav->CursorKByte) >> 8);
    }

    /* Upload the cursor image to the frame buffer. */
    memcpy(psav->FBBase + psav->CursorKByte * 1024, src, 1024);

    if( S3_SAVAGE4_SERIES( psav->Chipset ) ) {
	/*
	 * Bug in Savage4 Rev B requires us to do an MMIO read after
	 * loading the cursor.
	 */
	volatile unsigned int i = ALT_STATUS_WORD0;
	(void)i++;	/* Not to be optimised out */
    }
}

static void
SavageSetCursorPosition(
     ScrnInfoPtr pScrn,
     int x, 
     int y)
{
    SavagePtr psav = SAVPTR(pScrn);
    unsigned char xoff, yoff, byte;

    if( S3_SAVAGE4_SERIES( SAVPTR(pScrn)->Chipset ) )
    {
	waitHSync(5);
    }
    /* adjust for frame buffer base address granularity */
    if (pScrn->bitsPerPixel == 8)
	x += ((pScrn->frameX0) & 3);
    else if (pScrn->bitsPerPixel == 16)
	x += ((pScrn->frameX0) & 1);
    else if (pScrn->bitsPerPixel == 32)
	x += ((pScrn->frameX0+2) & 3) - 2;

    /*
    * Make these even when used.  There is a bug/feature on at least
    * some chipsets that causes a "shadow" of the cursor in interlaced
    * mode.  Making this even seems to have no visible effect, so just
    * do it for the generic case.
    */

    if (x < 0) {
	xoff = ((-x) & 0xFE);
	x = 0;
    } else {
	xoff = 0;
    }

    if (y < 0) {
	yoff = ((-y) & 0xFE);
	y = 0;
    } else {
	yoff = 0;
    }

    /* This is the recommended order to move the cursor */
        if (psav->IsSecondary) {
	SelectIGA2();
    	outCRReg( 0x46, (x & 0xff00)>>8 );
    	outCRReg( 0x47, (x & 0xff) );
    	outCRReg( 0x49, (y & 0xff) );
    	outCRReg( 0x4e, xoff );
    	outCRReg( 0x4f, yoff );
    	outCRReg( 0x48, (y & 0xff00)>>8 );
	SelectIGA1();
    } else {
        outCRReg( 0x46, (x & 0xff00)>>8 );
        outCRReg( 0x47, (x & 0xff) );
        outCRReg( 0x49, (y & 0xff) );
        outCRReg( 0x4e, xoff );
        outCRReg( 0x4f, yoff );
        outCRReg( 0x48, (y & 0xff00)>>8 );
    }

    /* fix for HW cursor on crtc2 */
    byte = inCRReg( 0x46 );
    outCRReg( 0x46, byte );

}


static void 
SavageSetCursorColors(
    ScrnInfoPtr pScrn,
    int bg,
    int fg)
{
    SavagePtr psav = SAVPTR(pScrn);
#if 0
    Bool bNeedExtra = FALSE;
#endif

    /* Clock doubled modes need an extra cursor stack write. */

#if 0
    bNeedExtra =
        (psav->CursorInfoRec->Flags & HARDWARE_CURSOR_TRUECOLOR_AT_8BPP);
#endif

    /* With the streams engine on HW Cursor seems to be 24bpp ALWAYS */
    if( 1 
#if 0
	|| S3_SAVAGE_MOBILE_SERIES(psav->Chipset) ||
 	(pScrn->depth == 24) ||
 	((pScrn->depth == 8) && bNeedExtra)
#endif
	) 
    {
	/* Do it straight, full 24 bit color. */
       if (psav->IsSecondary) {
            /* cursor 2 */
	    /* Reset the cursor color stack pointer */
	    SelectIGA2();
	    inCRReg(0x45);
	    /* Write low, mid, high bytes - foreground */
	    outCRReg(0x4a, fg);
	    outCRReg(0x4a, fg >> 8);
	    outCRReg(0x4a, fg >> 16);
	    /* Reset the cursor color stack pointer */
	    inCRReg(0x45);
	    /* Write low, mid, high bytes - background */
	    outCRReg(0x4b, bg);
	    outCRReg(0x4b, bg >> 8);
	    outCRReg(0x4b, bg >> 16);
	    SelectIGA1();
	} else {      
	    /* Reset the cursor color stack pointer */
	    inCRReg(0x45);
	    /* Write low, mid, high bytes - foreground */
	    outCRReg(0x4a, fg);
	    outCRReg(0x4a, fg >> 8);
	    outCRReg(0x4a, fg >> 16);
	    /* Reset the cursor color stack pointer */
	    inCRReg(0x45);
	    /* Write low, mid, high bytes - background */
	    outCRReg(0x4b, bg);
	    outCRReg(0x4b, bg >> 8);
	    outCRReg(0x4b, bg >> 16);
	}
	return;
    }
#if 0
    else if( (pScrn->depth == 15) || (pScrn->depth == 16) )
    {
	if (pScrn->depth == 15) {
	    fg = ((fg & 0xf80000) >> 9) |
		((fg & 0xf800) >> 6) |
		((fg & 0xf8) >> 3);
	    bg = ((bg & 0xf80000) >> 9) |
		((bg & 0xf800) >> 6) |
		((bg & 0xf8) >> 3);
	} else {
	    fg = ((fg & 0xf80000) >> 8) |
		((fg & 0xfc00) >> 5) |
		((fg & 0xf8) >> 3);
	    bg = ((bg & 0xf80000) >> 8) |
		((bg & 0xfc00) >> 5) |
		((bg & 0xf8) >> 3);
	}
	/* Reset the cursor color stack pointer */
        inCRReg( 0x45 );
        outCRReg( 0x4a, fg );
        outCRReg( 0x4a, fg>>8 );
	if( bNeedExtra )
	{
	    outCRReg( 0x4a, fg );
	    outCRReg( 0x4a, fg>>8 );
	}
	/* Reset the cursor color stack pointer */
        inCRReg( 0x45 );
        outCRReg( 0x4b, bg );
        outCRReg( 0x4b, bg>>8 );
	if( bNeedExtra )
	{
	    outCRReg( 0x4b, bg );
	    outCRReg( 0x4b, bg>>8 );
	}
    }
    else if( pScrn->depth == 8 )
    {
	/* Reset the cursor color stack pointer */
	inCRReg(0x45);
	/* Write foreground */
	outCRReg(0x4a, fg);
	outCRReg(0x4a, fg);
	/* Reset the cursor color stack pointer */
	inCRReg(0x45);
	/* Write background */
	outCRReg(0x4b, bg);
	outCRReg(0x4b, bg);
    }
#endif
}

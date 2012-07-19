/*
 * XGI hardware cursor handling
 *
 * Copyright (C) 2001-2004 by Thomas Winischhofer, Vienna, Austria.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1) Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2) Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3) The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESSED OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Author:   Thomas Winischhofer <thomas@winischhofer.net>
 *
 * Idea based on code by Can-Ru Yeou, XGI Inc.
 *
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "xf86.h"
#include "xf86PciInfo.h"
#include "cursorstr.h"
#include "misc.h"
#include "vgaHW.h"
#include "xgi.h"
#include "xgi_cursor.h"

static int currX = 0 , currY = 0 ;

static void XGIG1_SetCursorPosition(ScrnInfoPtr pScrn, int x, int y) ;

#if X_BYTE_ORDER == X_BIG_ENDIAN
# define BE_SWAP32(v) (lswapl(v))
#else
# define BE_SWAP32(v) (v)
#endif


/* Helper function for Xabre to convert mono image to ARGB */
/* The Xabre's cursor engine for CRT2 is buggy and can't
 * handle mono cursors. We therefore convert the mono image
 * to ARGB
 */

static void
Volari_ShowCursor(ScrnInfoPtr pScrn)
{
    XGIPtr    pXGI = XGIPTR(pScrn);
 /*   unsigned long cursor_addr = pXGI->CursorOffset ;  */
    unsigned long cursor_base = pXGI->CursorOffset/1024 ;

    /* Jong 09/19/2007; bug fixing for ??? */
    if( pXGI->HWARGBCursor )
    {
        xgiG2CRT1_EnableARGBHWCursor(cursor_base, 0);
        if (pXGI->VBFlags & CRT2_ENABLE)  
        {
    		xgiG2CRT2_EnableARGBHWCursor(cursor_base, 0);
        }
    }
    else
    {
        xgiG2CRT1_EnableHWCursor(cursor_base, 0);
        if (pXGI->VBFlags & CRT2_ENABLE)  
        {
    	    xgiG2CRT2_EnableHWCursor(cursor_base, 0);
        }
    }

    XGIG1_SetCursorPosition(pScrn, currX, currY) ;

	/* Jong 02/05/2009; improve performance of WinBench 99 */
	/* XGI_WaitEndRetrace(pXGI->RelIO); */
}

static void
Volari_HideCursor(ScrnInfoPtr pScrn)
{
    XGIPtr    pXGI = XGIPTR(pScrn);

    PDEBUG4(ErrorF("Volari_HideCursor(pScrn)\n"));
    xgiG1CRT1_DisableHWCursor() ;
    if (pXGI->VBFlags & CRT2_ENABLE)  {
        xgiG1CRT2_DisableHWCursor() ;
    }

    XGIG1_SetCursorPosition(pScrn, currX, currY) ;

	/* Jong 02/05/2009; improve performance of WinBench 99 */
    /* XGI_WaitEndRetrace(pXGI->RelIO); */
}

static void
XGIG1_SetCursorPosition(ScrnInfoPtr pScrn, int x, int y)
{
    XGIPtr    pXGI = XGIPTR(pScrn);

    unsigned char    x_preset = 0;
    unsigned char    y_preset = 0;

    currX = x ;
    currY = y ;
    
/* ErrorF("\nHWC Set pos x:%d y:%d",x,y);*/
    if (x < 0) {
        x_preset = (-x);
        x = 0;
    }
    if (y < 0) {
        y_preset = (-y);
        y = 0;
    }
   
    xgiG1CRT1_SetCursorPositionX(x , x_preset) ;
    xgiG1CRT1_SetCursorPositionY(y , y_preset) ;
    if (pXGI->VBFlags & CRT2_ENABLE)  {
        xgiG1CRT2_SetCursorPositionX(x+13, x_preset) ;
        xgiG1CRT2_SetCursorPositionY(y, y_preset) ;
    }
}

static void
Volari_SetCursorColors(ScrnInfoPtr pScrn, int bg, int fg)
{
    XGIPtr pXGI = XGIPTR(pScrn);

    xgiG1CRT1_SetCursorBGColor(bg) ;
    xgiG1CRT1_SetCursorFGColor(fg) ;
    if (pXGI->VBFlags & CRT2_ENABLE)  {
        xgiG1CRT2_SetCursorBGColor(bg) ;
        xgiG1CRT2_SetCursorFGColor(fg) ;
    }
    XGIG1_SetCursorPosition(pScrn, currX, currY) ;
}

static void
Volari_LoadCursorImage(ScrnInfoPtr pScrn, unsigned char *src)
{
    XGIPtr pXGI = XGIPTR(pScrn);
    const unsigned long cursor_base = pXGI->CursorOffset / 1024;
    unsigned char *const pCursorShape =
        pXGI->FbBase + pXGI->CursorOffset;


    memcpy(pCursorShape, src, 1024);

    xgiG2CRT1_SetCursorAddressPattern(cursor_base,0) ;
    if (pXGI->VBFlags & CRT2_ENABLE)  {
        xgiG2CRT2_SetCursorAddressPattern(cursor_base,0) ;
    }
    XGIG1_SetCursorPosition(pScrn, currX, currY) ;
}

static Bool
Volari_UseHWCursor(ScreenPtr pScreen, CursorPtr pCurs)
{
    ScrnInfoPtr       pScrn = xf86Screens[pScreen->myNum];
    DisplayModePtr     mode = pScrn->currentMode;

    if (mode->Flags & V_INTERLACE)
    {
        return FALSE;
    }
    return TRUE;
}

/* Jong 09/19/2007; Is this required? */
Bool
Volari_UseHWCursorARGB(ScreenPtr pScreen, CursorPtr pCurs)
{
    ScrnInfoPtr       pScrn = xf86Screens[pScreen->myNum];
    DisplayModePtr     mode = pScrn->currentMode;
    XGIPtr pXGI = XGIPTR(pScrn);

    if (mode->Flags & V_INTERLACE)
    {
        return FALSE;
    }

	/* DumpDDIName("Volari_UserHWCursorARGB()\n") ; */

	return TRUE ;
}

/* Jong 09/19/2007; Is this required? */
static void
Volari_LoadCursorARGB(ScrnInfoPtr pScrn, CursorPtr pCursor)
{
    XGIPtr pXGI = XGIPTR(pScrn);
    unsigned long cursor_addr = pXGI->CursorOffset ;
    unsigned long cursor_base = pXGI->CursorOffset/1024 ;
    unsigned char *pCursorShape ;
	int i , j ; CARD32 *pDest,*pSrc ;
	CursorBitsPtr pCursorBits = pCursor->bits ;


	/* DumpDDIName("Volari_LoadCursorARGB()\n") ; */
	pXGI->HWARGBCursor = TRUE ;
    pCursorShape = pXGI->FbBase + cursor_addr ;

	pSrc = pCursorBits->argb ;

	pXGI->CurXPreset = 64-pCursorBits->width ;
	pXGI->CurYPreset = 64-pCursorBits->height ;

	for( i = 64 - pCursorBits->height ; i< 64 ; i++ )
	{
		pDest = (CARD32 *)(pCursorShape + i*64*4 ) ;
		for( j = 64-pCursorBits->width ; j < 64 ; j++, pSrc++ )
		{
			pDest[j] = *pSrc ;
		}
	}

    xgiG2CRT1_SetCursorAddressPattern(cursor_base,0) ;

    if (pXGI->VBFlags & CRT2_ENABLE)  {
        xgiG2CRT2_SetCursorAddressPattern(cursor_base,0) ;
        /* xgiG1CRT2_SetCursorAddress(cursor_base) ; */
        /* xgiG1CRT2_SetCursorPatternSelect(0) ; */
    }
    XGIG1_SetCursorPosition(pScrn, currX, currY) ;
	PDEBUG4(vWaitCRT1VerticalRetrace(pScrn)) ;
	PDEBUG4(XGIDumpMMIO(pScrn));
}

Bool
XGIHWCursorInit(ScreenPtr pScreen)
{
    ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
    XGIPtr pXGI = XGIPTR(pScrn);
    xf86CursorInfoPtr infoPtr;

    infoPtr = xf86CreateCursorInfoRec();
    if(!infoPtr) return FALSE;

    pXGI->CursorInfoPtr = infoPtr;

    switch (pXGI->Chipset)  {

      case PCI_CHIP_XGIXG40:
      case PCI_CHIP_XGIXG20:
      case PCI_CHIP_XGIXG21:
    case PCI_CHIP_XGIXG27:
        default:
    PDEBUG(ErrorF("--- HWCursorInit() \n"));
        infoPtr->MaxWidth  = 64;
        infoPtr->MaxHeight = 64;
/* infoPtr->ShowCursor = Volari_ShowCursorColor; // */
        infoPtr->ShowCursor = Volari_ShowCursor;
        infoPtr->HideCursor = Volari_HideCursor;
        infoPtr->SetCursorPosition = XGIG1_SetCursorPosition;
        infoPtr->SetCursorColors = Volari_SetCursorColors;
/* infoPtr->LoadCursorImage = Volari_LoadCursorImageColors; // */
        infoPtr->LoadCursorImage = Volari_LoadCursorImage;
        infoPtr->UseHWCursor = Volari_UseHWCursor;
/* infoPtr->RealizeCursor = XGIRealizeCursorColor ; // */

        /* Jong 09/19/2007; Is this required */
	#ifdef XGI_ARGB_CURSOR
		infoPtr->UseHWCursorARGB = Volari_UseHWCursorARGB ;
		infoPtr->LoadCursorARGB = Volari_LoadCursorARGB ;
	#endif

        infoPtr->Flags =
            HARDWARE_CURSOR_TRUECOLOR_AT_8BPP |
            HARDWARE_CURSOR_INVERT_MASK |
            HARDWARE_CURSOR_BIT_ORDER_MSBFIRST  |
            HARDWARE_CURSOR_AND_SOURCE_WITH_MASK |
            HARDWARE_CURSOR_SWAP_SOURCE_AND_MASK |
            HARDWARE_CURSOR_SOURCE_MASK_INTERLEAVE_64  ;
        break ;
    }

    return(xf86InitCursor(pScreen, infoPtr));
}

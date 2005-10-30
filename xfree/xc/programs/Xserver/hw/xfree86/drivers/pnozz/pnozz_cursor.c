/*
 * SBus Weitek P9100 hardware cursor support
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
/* $NetBSD: pnozz_cursor.c,v 1.2 2005/10/30 15:57:58 macallan Exp $ */

#include "pnozz.h"

static void PnozzLoadCursorImage(ScrnInfoPtr pScrn, unsigned char *src);
static void PnozzSetCursorPosition(ScrnInfoPtr pScrn, int x, int y);
static void PnozzSetCursorColors(ScrnInfoPtr pScrn, int bg, int fg);

static void
PnozzLoadCursorImage(ScrnInfoPtr pScrn, unsigned char *src)
{
    PnozzPtr pPnozz = GET_PNOZZ_FROM_SCRN(pScrn);
    int i;

    /*
     * this DAC is funky. Sometimes it messes up the index and we end up 
     * stomping all over the video timing registers, the result looks funny.
     * so we're paranoid and write the index for every single byte.
     */
     
    pnozz_write_dac_ctl_reg(pPnozz, DAC_CURSOR_CTL, 
        DAC_CURSOR_X11| DAC_CURSOR_64);

    /* we use a 64x64, 2bit cursor image -> 0x400 bytes */
    for (i = 0; i < 0x400; i++)
	pnozz_write_dac_ctl_reg(pPnozz, i + 0x100, src[i]);
    
    /* write the hotspot coordinates, sometimes they seem to get lost */
    pnozz_write_dac_ctl_reg(pPnozz, DAC_CURSOR_HOT_X, 63);
    pnozz_write_dac_ctl_reg(pPnozz, DAC_CURSOR_HOT_Y, 63);

    pnozz_read_4(pPnozz, VID_DACSYNC);
}

void 
PnozzShowCursor(ScrnInfoPtr pScrn)
{
    PnozzPtr pPnozz = GET_PNOZZ_FROM_SCRN(pScrn);

    /* we use 64x64 */
    pnozz_write_dac_ctl_reg(pPnozz, DAC_CURSOR_CTL, 
        DAC_CURSOR_X11 | DAC_CURSOR_64);
    pnozz_read_4(pPnozz, VID_DACSYNC);
    
    pPnozz->CursorEnabled = TRUE;
}

void
PnozzHideCursor(ScrnInfoPtr pScrn)
{
    PnozzPtr pPnozz = GET_PNOZZ_FROM_SCRN(pScrn);

    pnozz_write_dac_ctl_reg(pPnozz, DAC_CURSOR_CTL, 
        DAC_CURSOR_OFF);
    
    pPnozz->CursorEnabled = FALSE;
}

static void
PnozzSetCursorPosition(ScrnInfoPtr pScrn, int x, int y)
{
    PnozzPtr pPnozz = GET_PNOZZ_FROM_SCRN(pScrn);

    pPnozz->CursorX = x;
    pPnozz->CursorY = y;
    
    pnozz_write_dac(pPnozz, DAC_INDX_CTL, DAC_INDX_AUTOINCR);
    pnozz_write_dac_ctl_reg_2(pPnozz, DAC_CURSOR_X, min(0x3ff, max(0, x + 63)));
    pnozz_write_dac_ctl_reg_2(pPnozz, DAC_CURSOR_Y, min(0x3ff, max(0, y + 63)));
    pnozz_read_4(pPnozz, VID_DACSYNC);
}

static void
PnozzSetCursorColors(ScrnInfoPtr pScrn, int bg, int fg)
{
    PnozzPtr pPnozz = GET_PNOZZ_FROM_SCRN(pScrn);

    if (bg != pPnozz->CursorBg || fg != pPnozz->CursorFg) {
	pnozz_write_dac(pPnozz, DAC_INDX_HI, 0);
	pnozz_write_dac(pPnozz, DAC_INDX_LO, 0x40);
	pnozz_write_dac(pPnozz, DAC_INDX_CTL, DAC_INDX_AUTOINCR);
	pnozz_write_dac(pPnozz, DAC_INDX_DATA, (bg >> 16) & 0xff);
	pnozz_write_dac(pPnozz, DAC_INDX_DATA, (bg >> 8) & 0xff);
	pnozz_write_dac(pPnozz, DAC_INDX_DATA, (bg) & 0xff);
	pnozz_write_dac(pPnozz, DAC_INDX_DATA, (fg >> 16) & 0xff);
	pnozz_write_dac(pPnozz, DAC_INDX_DATA, (fg >> 8) & 0xff);
	pnozz_write_dac(pPnozz, DAC_INDX_DATA, (fg) & 0xff);
	pPnozz->CursorBg = bg;
	pPnozz->CursorFg = fg;
	pnozz_read_4(pPnozz, VID_DACSYNC);
    }
}

Bool 
PnozzHWCursorInit(ScreenPtr pScreen)
{
    ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
    PnozzPtr pPnozz;
    xf86CursorInfoPtr infoPtr;

    pPnozz = GET_PNOZZ_FROM_SCRN(pScrn);
    pPnozz->CursorX = pPnozz->CursorY = 0;
    pPnozz->CursorBg = pPnozz->CursorFg = 0;
    pPnozz->CursorEnabled = FALSE;

    infoPtr = xf86CreateCursorInfoRec();
    if(!infoPtr) return FALSE;
    
    pPnozz->CursorInfoRec = infoPtr;

    infoPtr->MaxWidth = 64;
    infoPtr->MaxHeight = 64;
    infoPtr->Flags = HARDWARE_CURSOR_AND_SOURCE_WITH_MASK |
	HARDWARE_CURSOR_TRUECOLOR_AT_8BPP | 
	HARDWARE_CURSOR_SOURCE_MASK_INTERLEAVE_1 |
	HARDWARE_CURSOR_BIT_ORDER_MSBFIRST;

    infoPtr->SetCursorColors = PnozzSetCursorColors;
    infoPtr->SetCursorPosition = PnozzSetCursorPosition;
    infoPtr->LoadCursorImage = PnozzLoadCursorImage;
    infoPtr->HideCursor = PnozzHideCursor;
    infoPtr->ShowCursor = PnozzShowCursor;
    infoPtr->UseHWCursor = NULL;

    pnozz_write_dac_ctl_reg(pPnozz, DAC_CURSOR_HOT_X, 63);
    pnozz_write_dac_ctl_reg(pPnozz, DAC_CURSOR_HOT_Y, 63);
    pnozz_read_4(pPnozz, VID_DACSYNC);
    
    return xf86InitCursor(pScreen, infoPtr);
}

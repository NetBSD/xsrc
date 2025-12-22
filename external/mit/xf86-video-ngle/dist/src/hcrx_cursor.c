/* $NetBSD: hcrx_cursor.c,v 1.1 2025/12/22 13:07:11 macallan Exp $ */
/*
 * Copyright (c) 2025 Michael Lorenz
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *    - Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *    - Redistributions in binary form must reproduce the above
 *      copyright notice, this list of conditions and the following
 *      disclaimer in the documentation and/or other materials provided
 *      with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include <sys/types.h>
#include <dev/ic/nglereg.h>
#include "ngle.h"

static void HCRXLoadCursorImage(ScrnInfoPtr pScrn, unsigned char *src);
static void HCRXSetCursorPosition(ScrnInfoPtr pScrn, int x, int y);
static void HCRXSetCursorColors(ScrnInfoPtr pScrn, int bg, int fg);

static void
HCRXLoadCursorImage(ScrnInfoPtr pScrn, unsigned char *src)
{
	NGLEPtr pNGLE = NGLEPTR(pScrn);
	uint32_t latch;
	int i;

	/* image first */
	NGLEWrite4(pNGLE, NGLE_HCRX_CURSOR_ADDR, 0x80);
	for (i = 0; i < 128; i++) {
		memcpy(&latch, src, 4);
		NGLEWrite4(pNGLE, NGLE_HCRX_CURSOR_DATA, latch);
		src += 4;
	}

	/* and now the mask */
	NGLEWrite4(pNGLE, NGLE_HCRX_CURSOR_ADDR, 0);
	for (i = 0; i < 128; i++) {
		memcpy(&latch, src, 4);
		NGLEWrite4(pNGLE, NGLE_HCRX_CURSOR_DATA, latch);
		src += 4;
	}
}

void 
HCRXShowCursor(ScrnInfoPtr pScrn)
{
	NGLEPtr pNGLE = NGLEPTR(pScrn);

	pNGLE->cursor.enable = 1;
	NGLEWrite4(pNGLE, NGLE_HCRX_CURSOR, pNGLE->creg | HCRX_ENABLE_CURSOR);
}

void
HCRXHideCursor(ScrnInfoPtr pScrn)
{
	NGLEPtr pNGLE = NGLEPTR(pScrn);

	pNGLE->cursor.enable = 0;
	NGLEWrite4(pNGLE, NGLE_HCRX_CURSOR, pNGLE->creg);
}

static void
HCRXSetCursorPosition(ScrnInfoPtr pScrn, int x, int y)
{
	NGLEPtr pNGLE = NGLEPTR(pScrn);

	if (x < 0)
		x = 0x1000 - x;
	if (y < 0)
		y = 0x1000 - y;
	pNGLE->creg = (x << 16) | y;
	NGLEWrite4(pNGLE, NGLE_HCRX_CURSOR, pNGLE->creg |
	    (pNGLE->cursor.enable ? HCRX_ENABLE_CURSOR : 0));
}

static void
HCRXSetCursorColors(ScrnInfoPtr pScrn, int bg, int fg)
{
	NGLEPtr pNGLE = NGLEPTR(pScrn);
	uint8_t stat;

	do {
		stat = NGLERead1(pNGLE, NGLE_BUSY);
		if (stat == 0)
			stat = NGLERead1(pNGLE, NGLE_BUSY);
	} while (stat != 0);

	NGLEWrite4(pNGLE, NGLE_BAboth,
	  BA(FractDcd, Otc01, Ots08, Addr24, 0, BINcmap, 0));
	NGLEWrite4(pNGLE, NGLE_IBO,
	  IBOvals(RopSrc, 0, BitmapExtent08, 0, DataDynamic, MaskOtc, 0, 0));
	NGLEWrite4(pNGLE, NGLE_PLANEMASK, 0xffffffff);
	NGLEWrite4(pNGLE, NGLE_BINC_DST, 0);
	NGLEWrite4(pNGLE, NGLE_BINC_DATA_R, bg);
	NGLEWrite4(pNGLE, NGLE_BINC_DATA_R, fg);
	NGLEWrite4(pNGLE, NGLE_BINC_SRC, 0);
	NGLEWrite4(pNGLE, NGLE_HCRX_LUTBLT, LBC_ENABLE | LBC_TYPE_CURSOR | 4);
	pNGLE->hwmode = -1;
}

Bool 
HCRXSetupCursor(ScreenPtr pScreen)
{
	ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
	NGLEPtr pNGLE = NGLEPTR(pScrn);
	xf86CursorInfoPtr infoPtr;
	
	pNGLE->cursor.enable = 0;
	pNGLE->creg = 0;

	infoPtr = xf86CreateCursorInfoRec();
	if(!infoPtr) return FALSE;
    
	pNGLE->CursorInfoRec = infoPtr;
		
	xf86Msg(X_INFO, "HCRX HW cursor enabled\n");

	infoPtr->MaxWidth = 64;
	infoPtr->MaxHeight = 64;
	pNGLE->maskoffset = 8 * 64;
	
	infoPtr->Flags = HARDWARE_CURSOR_AND_SOURCE_WITH_MASK |
	    HARDWARE_CURSOR_TRUECOLOR_AT_8BPP;

	infoPtr->SetCursorColors = HCRXSetCursorColors;
	infoPtr->SetCursorPosition = HCRXSetCursorPosition;
	infoPtr->LoadCursorImage = HCRXLoadCursorImage;
	infoPtr->HideCursor = HCRXHideCursor;
	infoPtr->ShowCursor = HCRXShowCursor;
	infoPtr->UseHWCursor = NULL;

	return xf86InitCursor(pScreen, infoPtr);
}

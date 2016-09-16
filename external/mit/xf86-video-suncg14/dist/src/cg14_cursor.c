/* $NetBSD: cg14_cursor.c,v 1.3 2016/09/16 21:16:37 macallan Exp $ */
/*
 * Copyright (c) 2005 Michael Lorenz
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <sys/types.h>

/* all driver need this */
#include "xf86.h"
#include "xf86_OSproc.h"

#include "cg14.h"

static void CG14LoadCursorImage(ScrnInfoPtr pScrn, unsigned char *src);
static void CG14SetCursorPosition(ScrnInfoPtr pScrn, int x, int y);
static void CG14SetCursorColors(ScrnInfoPtr pScrn, int bg, int fg);

static void
CG14LoadCursorImage(ScrnInfoPtr pScrn, unsigned char *src)
{
	Cg14Ptr pCG14 = GET_CG14_FROM_SCRN(pScrn);
	uint32_t buf[64];
	uint32_t *img = buf;
	int i;

	memcpy(buf, src, 256); /* to make sure it's aligned */
	for (i = 0; i < 32; i++) {
		pCG14->curs->curs_plane0[i] = *img;
		img++;
	}
	for (i = 0; i < 32; i++) {
		pCG14->curs->curs_plane1[i] = *img;
		img++;
	}
}

void 
CG14ShowCursor(ScrnInfoPtr pScrn)
{
	Cg14Ptr pCG14 = GET_CG14_FROM_SCRN(pScrn);

	pCG14->curs->curs_ctl = CG14_CURS_ENABLE;
}

void
CG14HideCursor(ScrnInfoPtr pScrn)
{
	Cg14Ptr pCG14 = GET_CG14_FROM_SCRN(pScrn);

	pCG14->curs->curs_ctl = 0;
}

static void
CG14SetCursorPosition(ScrnInfoPtr pScrn, int x, int y)
{
	Cg14Ptr pCG14 = GET_CG14_FROM_SCRN(pScrn);

	pCG14->curs->curs_x = x;
	pCG14->curs->curs_y = y;
}

static void
CG14SetCursorColors(ScrnInfoPtr pScrn, int bg, int fg)
{
	Cg14Ptr pCG14 = GET_CG14_FROM_SCRN(pScrn);
#define RGB2BGR(x) (((x & 0xff0000) >> 16) | (x & 0xff00) | ((x & 0xff) << 16))
	pCG14->curs->curs_color1 = RGB2BGR(bg);
	pCG14->curs->curs_color2 = RGB2BGR(fg);
}

Bool 
CG14SetupCursor(ScreenPtr pScreen)
{
	ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
	Cg14Ptr pCG14 = GET_CG14_FROM_SCRN(pScrn);
	xf86CursorInfoPtr infoPtr;
	
	pCG14->curs->curs_ctl = 0;

	infoPtr = xf86CreateCursorInfoRec();
	if(!infoPtr) return FALSE;
    
	pCG14->CursorInfoRec = infoPtr;
		
	xf86Msg(X_INFO, "HW cursor enabled\n");

	infoPtr->MaxWidth = 32;
	infoPtr->MaxHeight = 32;
	
	infoPtr->Flags = HARDWARE_CURSOR_AND_SOURCE_WITH_MASK |
	    HARDWARE_CURSOR_TRUECOLOR_AT_8BPP |
	    HARDWARE_CURSOR_SWAP_SOURCE_AND_MASK;
	infoPtr->SetCursorColors = CG14SetCursorColors;
	infoPtr->SetCursorPosition = CG14SetCursorPosition;
	infoPtr->LoadCursorImage = CG14LoadCursorImage;
	infoPtr->HideCursor = CG14HideCursor;
	infoPtr->ShowCursor = CG14ShowCursor;
	infoPtr->UseHWCursor = NULL;

	return xf86InitCursor(pScreen, infoPtr);
}

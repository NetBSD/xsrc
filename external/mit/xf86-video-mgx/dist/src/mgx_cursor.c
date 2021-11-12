/* $NetBSD: mgx_cursor.c,v 1.1 2021/11/12 18:58:14 macallan Exp $ */
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

/*
 * fugly copypasta for now
 * the mgx at sbus driver lets us run wsdisplay ioctl()s on the fb device
 * will replace with direct hw access eventually
 */
 
#include <fcntl.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/endian.h>
#include <dev/wscons/wsconsio.h>
#include <sys/ioctl.h>
#include <errno.h>

#include "mgx.h"

static void MgxLoadCursorImage(ScrnInfoPtr pScrn, unsigned char *src);
static void MgxSetCursorPosition(ScrnInfoPtr pScrn, int x, int y);
static void MgxSetCursorColors(ScrnInfoPtr pScrn, int bg, int fg);

static void
MgxLoadCursorImage(ScrnInfoPtr pScrn, unsigned char *src)
{
	MgxPtr pMgx = GET_MGX_FROM_SCRN(pScrn);
	int err, i;
	
	pMgx->cursor.which = WSDISPLAY_CURSOR_DOALL;
	pMgx->cursor.image = src;
	pMgx->cursor.mask = src + pMgx->maskoffset;
	if(ioctl(pMgx->psdp->fd, WSDISPLAYIO_SCURSOR, &pMgx->cursor) == -1)
		xf86Msg(X_ERROR, "MgxLoadCursorImage: %d\n", errno);
}

void 
MgxShowCursor(ScrnInfoPtr pScrn)
{
	MgxPtr pMgx = GET_MGX_FROM_SCRN(pScrn);

	pMgx->cursor.which = WSDISPLAY_CURSOR_DOCUR;
	pMgx->cursor.enable = 1;
	if(ioctl(pMgx->psdp->fd, WSDISPLAYIO_SCURSOR, &pMgx->cursor) == -1)
		xf86Msg(X_ERROR, "MgxShowCursor: %d\n", errno);
}

void
MgxHideCursor(ScrnInfoPtr pScrn)
{
	MgxPtr pMgx = GET_MGX_FROM_SCRN(pScrn);

	pMgx->cursor.which = WSDISPLAY_CURSOR_DOCUR;
	pMgx->cursor.enable = 0;
	if(ioctl(pMgx->psdp->fd, WSDISPLAYIO_SCURSOR, &pMgx->cursor) == -1)
		xf86Msg(X_ERROR, "MgxHideCursor: %d\n", errno);
}

static void
MgxSetCursorPosition(ScrnInfoPtr pScrn, int x, int y)
{
	MgxPtr pMgx = GET_MGX_FROM_SCRN(pScrn);
	int xoff = 0, yoff = 0;
	
	pMgx->cursor.which = WSDISPLAY_CURSOR_DOPOS | WSDISPLAY_CURSOR_DOHOT;
	
	if (x < 0) {
		xoff = -x;
		x = 0;
	}
	if (y < 0) {
		yoff = -y;
		y = 0;
	}
	
	pMgx->cursor.pos.x = x;
	pMgx->cursor.hot.x = xoff;
	pMgx->cursor.pos.y = y;
	pMgx->cursor.hot.y = yoff;
	
	if(ioctl(pMgx->psdp->fd, WSDISPLAYIO_SCURSOR, &pMgx->cursor) == -1)
		xf86Msg(X_ERROR, "MgxSetCursorPosition: %d\n", errno);
}

static void
MgxSetCursorColors(ScrnInfoPtr pScrn, int bg, int fg)
{
	MgxPtr pMgx = GET_MGX_FROM_SCRN(pScrn);
	u_char r[4], g[4], b[4];
	
	pMgx->cursor.which = WSDISPLAY_CURSOR_DOCMAP;
	pMgx->cursor.cmap.red = r;
	pMgx->cursor.cmap.green = g;
	pMgx->cursor.cmap.blue = b;
	r[1] = fg & 0xff;
	g[1] = (fg & 0xff00) >> 8;
	b[1] = (fg & 0xff0000) >> 16;
	r[0] = bg & 0xff;
	g[0] = (bg & 0xff00) >> 8;
	b[0] = (bg & 0xff0000) >> 16;
	pMgx->cursor.cmap.index = 0;
	pMgx->cursor.cmap.count = 2;
	if(ioctl(pMgx->psdp->fd, WSDISPLAYIO_SCURSOR, &pMgx->cursor) == -1)
		xf86Msg(X_ERROR, "MgxSetCursorColors: %d\n", errno);
}

Bool 
MgxSetupCursor(ScreenPtr pScreen)
{
	ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
	MgxPtr pMgx = GET_MGX_FROM_SCRN(pScrn);
	xf86CursorInfoPtr infoPtr;
	
	pMgx->cursor.pos.x = 0;
	pMgx->cursor.pos.y = 0;
	pMgx->cursor.enable = 0;

	infoPtr = xf86CreateCursorInfoRec();
	if(!infoPtr) return FALSE;
    
	pMgx->CursorInfoRec = infoPtr;
	if(ioctl(pMgx->psdp->fd, WSDISPLAYIO_GCURMAX, &pMgx->cursor.size) == -1) {
		xf86Msg(X_WARNING, "No HW cursor support found\n");
		return FALSE;
	}
		
	xf86Msg(X_INFO, "HW cursor enabled\n");

	infoPtr->MaxWidth = pMgx->cursor.size.x;
	infoPtr->MaxHeight = pMgx->cursor.size.y;
	pMgx->maskoffset = ( pMgx->cursor.size.x >> 3) * pMgx->cursor.size.y;
	
	pMgx->cursor.hot.x = 0;
	pMgx->cursor.hot.y = 0;
	pMgx->cursor.which = WSDISPLAY_CURSOR_DOHOT | WSDISPLAY_CURSOR_DOCUR |
	    WSDISPLAY_CURSOR_DOPOS;
	if(ioctl(pMgx->psdp->fd, WSDISPLAYIO_SCURSOR, &pMgx->cursor) == -1)
		xf86Msg(X_ERROR, "WSDISPLAYIO_SCURSOR: %d\n", errno);
	
	infoPtr->Flags = HARDWARE_CURSOR_AND_SOURCE_WITH_MASK |
	    HARDWARE_CURSOR_TRUECOLOR_AT_8BPP
/* XXX not sure why exactly we need this */
#if BYTE_ORDER == BIG_ENDIAN
	    | HARDWARE_CURSOR_BIT_ORDER_MSBFIRST
#endif
	;
	infoPtr->SetCursorColors = MgxSetCursorColors;
	infoPtr->SetCursorPosition = MgxSetCursorPosition;
	infoPtr->LoadCursorImage = MgxLoadCursorImage;
	infoPtr->HideCursor = MgxHideCursor;
	infoPtr->ShowCursor = MgxShowCursor;
	infoPtr->UseHWCursor = NULL;

	return xf86InitCursor(pScreen, infoPtr);
}

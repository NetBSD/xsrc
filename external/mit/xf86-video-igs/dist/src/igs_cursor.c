/* $NetBSD: igs_cursor.c,v 1.4 2016/08/18 09:32:26 mrg Exp $ */
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

#include <fcntl.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/endian.h>
#include <dev/wscons/wsconsio.h>
#include <sys/ioctl.h>
#include <errno.h>

#include "igs.h"

static void IgsLoadCursorImage(ScrnInfoPtr pScrn, unsigned char *src);
static void IgsSetCursorPosition(ScrnInfoPtr pScrn, int x, int y);
static void IgsSetCursorColors(ScrnInfoPtr pScrn, int bg, int fg);

static void
IgsLoadCursorImage(ScrnInfoPtr pScrn, unsigned char *src)
{
	IgsPtr pIgs = IGSPTR(pScrn);
	int err, i;
	
	pIgs->cursor.which = WSDISPLAY_CURSOR_DOALL;
	pIgs->cursor.image = src;
	pIgs->cursor.mask = src + pIgs->maskoffset;
	if(ioctl(pIgs->fd, WSDISPLAYIO_SCURSOR, &pIgs->cursor) == -1)
		xf86Msg(X_ERROR, "IgsLoadCursorImage: %d\n", errno);
}

void 
IgsShowCursor(ScrnInfoPtr pScrn)
{
	IgsPtr pIgs = IGSPTR(pScrn);

	pIgs->cursor.which = WSDISPLAY_CURSOR_DOCUR;
	pIgs->cursor.enable = 1;
	if(ioctl(pIgs->fd, WSDISPLAYIO_SCURSOR, &pIgs->cursor) == -1)
		xf86Msg(X_ERROR, "IgsShowCursor: %d\n", errno);
}

void
IgsHideCursor(ScrnInfoPtr pScrn)
{
	IgsPtr pIgs = IGSPTR(pScrn);

	pIgs->cursor.which = WSDISPLAY_CURSOR_DOCUR;
	pIgs->cursor.enable = 0;
	if(ioctl(pIgs->fd, WSDISPLAYIO_SCURSOR, &pIgs->cursor) == -1)
		xf86Msg(X_ERROR, "IgsHideCursor: %d\n", errno);
}

static void
IgsSetCursorPosition(ScrnInfoPtr pScrn, int x, int y)
{
	IgsPtr pIgs = IGSPTR(pScrn);
	int xoff = 0, yoff = 0;
	
	pIgs->cursor.which = WSDISPLAY_CURSOR_DOPOS | WSDISPLAY_CURSOR_DOHOT;
	
	if (x < 0) {
		xoff = -x;
		x = 0;
	}
	if (y < 0) {
		yoff = -y;
		y = 0;
	}
	
	pIgs->cursor.pos.x = x;
	pIgs->cursor.hot.x = xoff;
	pIgs->cursor.pos.y = y;
	pIgs->cursor.hot.y = yoff;
	
	if(ioctl(pIgs->fd, WSDISPLAYIO_SCURSOR, &pIgs->cursor) == -1)
		xf86Msg(X_ERROR, "IgsSetCursorPosition: %d\n", errno);
}

static void
IgsSetCursorColors(ScrnInfoPtr pScrn, int bg, int fg)
{
	IgsPtr pIgs = IGSPTR(pScrn);
	u_char r[4], g[4], b[4];
	
	pIgs->cursor.which = WSDISPLAY_CURSOR_DOCMAP;
	pIgs->cursor.cmap.red = r;
	pIgs->cursor.cmap.green = g;
	pIgs->cursor.cmap.blue = b;
	r[1] = fg & 0xff;
	g[1] = (fg & 0xff00) >> 8;
	b[1] = (fg & 0xff0000) >> 16;
	r[0] = bg & 0xff;
	g[0] = (bg & 0xff00) >> 8;
	b[0] = (bg & 0xff0000) >> 16;
	pIgs->cursor.cmap.index = 0;
	pIgs->cursor.cmap.count = 2;
	if(ioctl(pIgs->fd, WSDISPLAYIO_SCURSOR, &pIgs->cursor) == -1)
		xf86Msg(X_ERROR, "IgsSetCursorColors: %d\n", errno);
}

Bool 
IgsSetupCursor(ScreenPtr pScreen)
{
	ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
	IgsPtr pIgs = IGSPTR(pScrn);
	xf86CursorInfoPtr infoPtr;
	
	pIgs->cursor.pos.x = 0;
	pIgs->cursor.pos.y = 0;
	pIgs->cursor.enable = 0;

	infoPtr = xf86CreateCursorInfoRec();
	if(!infoPtr) return FALSE;
    
	pIgs->CursorInfoRec = infoPtr;
	if(ioctl(pIgs->fd, WSDISPLAYIO_GCURMAX, &pIgs->cursor.size) == -1) {
		xf86Msg(X_WARNING, "No HW cursor support found\n");
		return FALSE;
	}
		
	xf86Msg(X_INFO, "HW cursor enabled\n");

	infoPtr->MaxWidth = pIgs->cursor.size.x;
	infoPtr->MaxHeight = pIgs->cursor.size.y;
	pIgs->maskoffset = ( pIgs->cursor.size.x >> 3) * pIgs->cursor.size.y;
	
	pIgs->cursor.hot.x = 0;
	pIgs->cursor.hot.y = 0;
	pIgs->cursor.which = WSDISPLAY_CURSOR_DOHOT | WSDISPLAY_CURSOR_DOCUR |
	    WSDISPLAY_CURSOR_DOPOS;
	if(ioctl(pIgs->fd, WSDISPLAYIO_SCURSOR, &pIgs->cursor) == -1)
		xf86Msg(X_ERROR, "WSDISPLAYIO_SCURSOR: %d\n", errno);
	
	infoPtr->Flags = HARDWARE_CURSOR_AND_SOURCE_WITH_MASK |
	    HARDWARE_CURSOR_TRUECOLOR_AT_8BPP
/* XXX not sure why exactly we need this */
#if BYTE_ORDER == BIG_ENDIAN
	    | HARDWARE_CURSOR_BIT_ORDER_MSBFIRST
#endif
	;
	infoPtr->SetCursorColors = IgsSetCursorColors;
	infoPtr->SetCursorPosition = IgsSetCursorPosition;
	infoPtr->LoadCursorImage = IgsLoadCursorImage;
	infoPtr->HideCursor = IgsHideCursor;
	infoPtr->ShowCursor = IgsShowCursor;
	infoPtr->UseHWCursor = NULL;

	return xf86InitCursor(pScreen, infoPtr);
}

/* $NetBSD: ngle_cursor.c,v 1.1 2024/10/16 11:00:36 macallan Exp $ */
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
 * Based on fbdev.c written by:
 *
 * Authors:  Alan Hourihane, <alanh@fairlite.demon.co.uk>
 *	     Michel Dänzer, <michdaen@iiic.ethz.ch>
 */

#include <fcntl.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/endian.h>
#include <dev/wscons/wsconsio.h>
#include <errno.h>

/* all driver need this */
#include "xf86.h"
#include "xf86_OSproc.h"
#include "xf86_OSlib.h"

#include "ngle.h"

static void NGLELoadCursorImage(ScrnInfoPtr pScrn, unsigned char *src);
static void NGLESetCursorPosition(ScrnInfoPtr pScrn, int x, int y);
static void NGLESetCursorColors(ScrnInfoPtr pScrn, int bg, int fg);

static void
NGLELoadCursorImage(ScrnInfoPtr pScrn, unsigned char *src)
{
	NGLEPtr pNGLE = NGLEPTR(pScrn);
	int err, i;
	
	pNGLE->cursor.which = WSDISPLAY_CURSOR_DOALL;
	pNGLE->cursor.image = src;
	pNGLE->cursor.mask = src + pNGLE->maskoffset;
	if(ioctl(pNGLE->fd, WSDISPLAYIO_SCURSOR, &pNGLE->cursor) == -1)
		xf86Msg(X_ERROR, "NGLELoadCursorImage: %d\n", errno);
}

void 
NGLEShowCursor(ScrnInfoPtr pScrn)
{
	NGLEPtr pNGLE = NGLEPTR(pScrn);

	pNGLE->cursor.which = WSDISPLAY_CURSOR_DOCUR;
	pNGLE->cursor.enable = 1;
	if(ioctl(pNGLE->fd, WSDISPLAYIO_SCURSOR, &pNGLE->cursor) == -1)
		xf86Msg(X_ERROR, "NGLEShowCursor: %d\n", errno);
}

void
NGLEHideCursor(ScrnInfoPtr pScrn)
{
	NGLEPtr pNGLE = NGLEPTR(pScrn);

	pNGLE->cursor.which = WSDISPLAY_CURSOR_DOCUR;
	pNGLE->cursor.enable = 0;
	if(ioctl(pNGLE->fd, WSDISPLAYIO_SCURSOR, &pNGLE->cursor) == -1)
		xf86Msg(X_ERROR, "NGLEHideCursor: %d\n", errno);
}

static void
NGLESetCursorPosition(ScrnInfoPtr pScrn, int x, int y)
{
	NGLEPtr pNGLE = NGLEPTR(pScrn);
	int xoff = 0, yoff = 0;
	
	pNGLE->cursor.which = WSDISPLAY_CURSOR_DOPOS | WSDISPLAY_CURSOR_DOHOT;
	
	if (x < 0) {
		xoff = -x;
		x = 0;
	}
	if (y < 0) {
		yoff = -y;
		y = 0;
	}
	
	pNGLE->cursor.pos.x = x;
	pNGLE->cursor.hot.x = xoff;
	pNGLE->cursor.pos.y = y;
	pNGLE->cursor.hot.y = yoff;
	
	if(ioctl(pNGLE->fd, WSDISPLAYIO_SCURSOR, &pNGLE->cursor) == -1)
		xf86Msg(X_ERROR, "NGLESetCursorPosition: %d\n", errno);
}

static void
NGLESetCursorColors(ScrnInfoPtr pScrn, int bg, int fg)
{
	NGLEPtr pNGLE = NGLEPTR(pScrn);
	u_char r[4], g[4], b[4];
	
	pNGLE->cursor.which = WSDISPLAY_CURSOR_DOCMAP;
	pNGLE->cursor.cmap.red = r;
	pNGLE->cursor.cmap.green = g;
	pNGLE->cursor.cmap.blue = b;
	r[1] = fg & 0xff;
	g[1] = (fg & 0xff00) >> 8;
	b[1] = (fg & 0xff0000) >> 16;
	r[0] = bg & 0xff;
	g[0] = (bg & 0xff00) >> 8;
	b[0] = (bg & 0xff0000) >> 16;
	pNGLE->cursor.cmap.index = 0;
	pNGLE->cursor.cmap.count = 2;
	if(ioctl(pNGLE->fd, WSDISPLAYIO_SCURSOR, &pNGLE->cursor) == -1)
		xf86Msg(X_ERROR, "NGLESetCursorColors: %d\n", errno);
}

Bool 
NGLESetupCursor(ScreenPtr pScreen)
{
	ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
	NGLEPtr pNGLE = NGLEPTR(pScrn);
	xf86CursorInfoPtr infoPtr;
	
	pNGLE->cursor.pos.x = 0;
	pNGLE->cursor.pos.y = 0;
	pNGLE->cursor.enable = 0;

	infoPtr = xf86CreateCursorInfoRec();
	if(!infoPtr) return FALSE;
    
	pNGLE->CursorInfoRec = infoPtr;
	if(ioctl(pNGLE->fd, WSDISPLAYIO_GCURMAX, &pNGLE->cursor.size) == -1) {
		xf86Msg(X_WARNING, "No HW cursor support found\n");
		return FALSE;
	}
		
	xf86Msg(X_INFO, "HW cursor enabled\n");

	infoPtr->MaxWidth = pNGLE->cursor.size.x;
	infoPtr->MaxHeight = pNGLE->cursor.size.y;
	pNGLE->maskoffset = ( pNGLE->cursor.size.x >> 3) * pNGLE->cursor.size.y;
	
	pNGLE->cursor.hot.x = 0;
	pNGLE->cursor.hot.y = 0;
	pNGLE->cursor.which = WSDISPLAY_CURSOR_DOHOT | WSDISPLAY_CURSOR_DOCUR |
	    WSDISPLAY_CURSOR_DOPOS;
	if(ioctl(pNGLE->fd, WSDISPLAYIO_SCURSOR, &pNGLE->cursor) == -1)
		xf86Msg(X_ERROR, "WSDISPLAYIO_SCURSOR: %d\n", errno);
	
	infoPtr->Flags = HARDWARE_CURSOR_AND_SOURCE_WITH_MASK |
	    HARDWARE_CURSOR_TRUECOLOR_AT_8BPP
/* XXX not sure why exactly we need this */
#if BYTE_ORDER == BIG_ENDIAN
	    | HARDWARE_CURSOR_BIT_ORDER_MSBFIRST
#endif
	;
	infoPtr->SetCursorColors = NGLESetCursorColors;
	infoPtr->SetCursorPosition = NGLESetCursorPosition;
	infoPtr->LoadCursorImage = NGLELoadCursorImage;
	infoPtr->HideCursor = NGLEHideCursor;
	infoPtr->ShowCursor = NGLEShowCursor;
	infoPtr->UseHWCursor = NULL;

	return xf86InitCursor(pScreen, infoPtr);
}

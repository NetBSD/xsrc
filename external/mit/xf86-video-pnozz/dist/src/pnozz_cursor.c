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
/* $NetBSD: pnozz_cursor.c,v 1.3 2021/05/27 04:48:10 jdc Exp $ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <fcntl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <dev/wscons/wsconsio.h>

#include "pnozz.h"

static void PnozzLoadCursorImage(ScrnInfoPtr pScrn, unsigned char *src);
static void PnozzSetCursorPosition(ScrnInfoPtr pScrn, int x, int y);
static void PnozzSetCursorColors(ScrnInfoPtr pScrn, int bg, int fg);

static void
PnozzLoadCursorImage(ScrnInfoPtr pScrn, unsigned char *src)
{
    PnozzPtr pPnozz = GET_PNOZZ_FROM_SCRN(pScrn);
    
    pPnozz->Cursor.set = FB_CUR_SETSHAPE;
    pPnozz->Cursor.image = src;
    pPnozz->Cursor.mask = src + 0x200;
    
    if (ioctl(pPnozz->psdp->fd, FBIOSCURSOR, &pPnozz->Cursor) == -1) 
	xf86Msg(X_ERROR, "FB_CUR_SETSHAPE failed\n");
}

void 
PnozzShowCursor(ScrnInfoPtr pScrn)
{
    PnozzPtr pPnozz = GET_PNOZZ_FROM_SCRN(pScrn);

    if (pPnozz->Cursor.enable == 0) {
    	pPnozz->Cursor.enable = 1;
	pPnozz->Cursor.set = FB_CUR_SETCUR;
	if (ioctl(pPnozz->psdp->fd, FBIOSCURSOR, &pPnozz->Cursor) == -1) 
	    xf86Msg(X_ERROR, "FB_CUR_SETCUR failed\n");
    }
}

void
PnozzHideCursor(ScrnInfoPtr pScrn)
{
    PnozzPtr pPnozz = GET_PNOZZ_FROM_SCRN(pScrn);

    if (pPnozz->Cursor.enable == 1) {
    	pPnozz->Cursor.enable = 0;
	pPnozz->Cursor.set = FB_CUR_SETCUR;
	if (ioctl(pPnozz->psdp->fd, FBIOSCURSOR, &pPnozz->Cursor) == -1) 
	    xf86Msg(X_ERROR, "FB_CUR_SETCUR failed\n");
    }
}

static void
PnozzSetCursorPosition(ScrnInfoPtr pScrn, int x, int y)
{
    PnozzPtr pPnozz = GET_PNOZZ_FROM_SCRN(pScrn);

    pPnozz->Cursor.pos.x = x + 63;
    pPnozz->Cursor.pos.y = y + 63;
    pPnozz->Cursor.set = FB_CUR_SETPOS;
    if (ioctl(pPnozz->psdp->fd, FBIOSCURSOR, &pPnozz->Cursor) == -1) 
	xf86Msg(X_ERROR, "FB_CUR_SETPOS failed\n");
}

static void
PnozzSetCursorColors(ScrnInfoPtr pScrn, int bg, int fg)
{
    PnozzPtr pPnozz = GET_PNOZZ_FROM_SCRN(pScrn);

    pPnozz->Cursor.set = FB_CUR_SETCMAP;
    pPnozz->Cursor.cmap.red[0] = (bg & 0xff0000) >> 16;
    pPnozz->Cursor.cmap.green[0] = (bg & 0xff00) >> 8;
    pPnozz->Cursor.cmap.blue[0] = bg & 0xff;
    pPnozz->Cursor.cmap.red[1] = (fg & 0xff0000) >> 16;
    pPnozz->Cursor.cmap.green[1] = (fg & 0xff00) >> 8;
    pPnozz->Cursor.cmap.blue[1] = fg & 0xff;
    if (ioctl(pPnozz->psdp->fd, FBIOSCURSOR, &pPnozz->Cursor) == -1) 
	xf86Msg(X_ERROR, "FB_CUR_SETCMAP failed\n");
}

Bool 
PnozzHWCursorInit(ScreenPtr pScreen)
{
    ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
    PnozzPtr pPnozz;
    xf86CursorInfoPtr infoPtr;

    pPnozz = GET_PNOZZ_FROM_SCRN(pScrn);
    
    pPnozz->Cursor.mask = NULL;
    pPnozz->Cursor.image = NULL;
    if (ioctl(pPnozz->psdp->fd, FBIOGCURSOR, &pPnozz->Cursor) == -1) {
    	xf86Msg(X_ERROR, "Hardware cursor isn't available\n");
	return FALSE;
    }

    infoPtr = xf86CreateCursorInfoRec();
    if(!infoPtr) return FALSE;
    
    pPnozz->CursorInfoRec = infoPtr;

    infoPtr->MaxWidth = 64;
    infoPtr->MaxHeight = 64;
    
    pPnozz->Cursor.hot.x = 63;
    pPnozz->Cursor.hot.y = 63;
    pPnozz->Cursor.set = FB_CUR_SETHOT;
    ioctl(pPnozz->psdp->fd, FBIOSCURSOR, &pPnozz->Cursor);
    
    pPnozz->Cursor.cmap.red = pPnozz->pal;
    pPnozz->Cursor.cmap.green = pPnozz->pal + 3;
    pPnozz->Cursor.cmap.blue = pPnozz->pal + 6;
    
    infoPtr->Flags = HARDWARE_CURSOR_AND_SOURCE_WITH_MASK |
	HARDWARE_CURSOR_TRUECOLOR_AT_8BPP /*| 
	HARDWARE_CURSOR_BIT_ORDER_MSBFIRST | HARDWARE_CURSOR_NIBBLE_SWAPPED*/;

    infoPtr->SetCursorColors = PnozzSetCursorColors;
    infoPtr->SetCursorPosition = PnozzSetCursorPosition;
    infoPtr->LoadCursorImage = PnozzLoadCursorImage;
    infoPtr->HideCursor = PnozzHideCursor;
    infoPtr->ShowCursor = PnozzShowCursor;
    infoPtr->UseHWCursor = NULL;

    return xf86InitCursor(pScreen, infoPtr);
}

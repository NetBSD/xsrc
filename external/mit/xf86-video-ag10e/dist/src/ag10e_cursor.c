/*
 * Hardware cursor support for Fujitsu AG-10e
 *
 * Copyright 2007 by Michael Lorenz
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without
 * fee, provided that the above copyright notice appear in all copies
 * and that both that copyright notice and this permission notice
 * appear in supporting documentation, and that the name of Jakub
 * Jelinek not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior
 * permission.  Jakub Jelinek makes no representations about the
 * suitability of this software for any purpose.  It is provided "as
 * is" without express or implied warranty.
 *
 * JAKUB JELINEK DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS
 * SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS, IN NO EVENT SHALL JAKUB JELINEK BE LIABLE FOR ANY
 * SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN
 * AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING
 * OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 */
/* $XFree86: xc/programs/Xserver/hw/xfree86/drivers/suncg6/cg6_cursor.c,v 1.1 2000/06/30 19:30:46 dawes Exp $ */

#include <fcntl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <dev/sun/fbio.h>
#include <dev/wscons/wsconsio.h>

#include "ag10e.h"

static void AG10ELoadCursorImage(ScrnInfoPtr pScrn, unsigned char *src);
static void AG10EShowCursor(ScrnInfoPtr pScrn);
static void AG10EHideCursor(ScrnInfoPtr pScrn);
static void AG10ESetCursorPosition(ScrnInfoPtr pScrn, int x, int y);
static void AG10ESetCursorColors(ScrnInfoPtr pScrn, int bg, int fg);

static void
AG10ELoadCursorImage(ScrnInfoPtr pScrn, unsigned char *src)
{
    AG10EPtr pAG10E = GET_AG10E_FROM_SCRN(pScrn);
    
    pAG10E->Cursor.set = FB_CUR_SETSHAPE;
    pAG10E->Cursor.image = src;
    pAG10E->Cursor.mask = src + 0x200;
    
    if (ioctl(pAG10E->psdp->fd, FBIOSCURSOR, &pAG10E->Cursor) == -1) 
	xf86Msg(X_ERROR, "FB_CUR_SETSHAPE failed\n");
}

void 
AG10EShowCursor(ScrnInfoPtr pScrn)
{
    AG10EPtr pAG10E = GET_AG10E_FROM_SCRN(pScrn);

    if (pAG10E->Cursor.enable == 0) {
    	pAG10E->Cursor.enable = 1;
	pAG10E->Cursor.set = FB_CUR_SETCUR;
	if (ioctl(pAG10E->psdp->fd, FBIOSCURSOR, &pAG10E->Cursor) == -1) 
	    xf86Msg(X_ERROR, "FB_CUR_SETCUR failed\n");
    }
}

void
AG10EHideCursor(ScrnInfoPtr pScrn)
{
    AG10EPtr pAG10E = GET_AG10E_FROM_SCRN(pScrn);

    if (pAG10E->Cursor.enable == 1) {
    	pAG10E->Cursor.enable = 0;
	pAG10E->Cursor.set = FB_CUR_SETCUR;
	if (ioctl(pAG10E->psdp->fd, FBIOSCURSOR, &pAG10E->Cursor) == -1) 
	    xf86Msg(X_ERROR, "FB_CUR_SETCUR failed\n");
    }
}

static void
AG10ESetCursorPosition(ScrnInfoPtr pScrn, int x, int y)
{
    AG10EPtr pAG10E = GET_AG10E_FROM_SCRN(pScrn);

    pAG10E->Cursor.pos.x = x;
    pAG10E->Cursor.pos.y = y;
    pAG10E->Cursor.set = FB_CUR_SETPOS;
    if (ioctl(pAG10E->psdp->fd, FBIOSCURSOR, &pAG10E->Cursor) == -1) 
	xf86Msg(X_ERROR, "FB_CUR_SETPOS failed\n");
}

static void
AG10ESetCursorColors(ScrnInfoPtr pScrn, int bg, int fg)
{
    AG10EPtr pAG10E = GET_AG10E_FROM_SCRN(pScrn);

    pAG10E->Cursor.set = FB_CUR_SETCMAP;
    pAG10E->Cursor.cmap.index = 0;
    pAG10E->Cursor.cmap.count = 2;
    pAG10E->Cursor.cmap.red[0] = (bg & 0xff0000) >> 16;
    pAG10E->Cursor.cmap.green[0] = (bg & 0xff00) >> 8;
    pAG10E->Cursor.cmap.blue[0] = bg & 0xff;
    pAG10E->Cursor.cmap.red[1] = (fg & 0xff0000) >> 16;
    pAG10E->Cursor.cmap.green[1] = (fg & 0xff00) >> 8;
    pAG10E->Cursor.cmap.blue[1] = fg & 0xff;
    if (ioctl(pAG10E->psdp->fd, FBIOSCURSOR, &pAG10E->Cursor) == -1) 
	xf86Msg(X_ERROR, "FB_CUR_SETCMAP failed\n");
}

Bool
AG10EHWCursorInit(ScreenPtr pScreen)
{
    ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
    AG10EPtr pAG10E;
    xf86CursorInfoPtr infoPtr;

    pAG10E = GET_AG10E_FROM_SCRN(pScrn);
    
    pAG10E->Cursor.mask = NULL;
    pAG10E->Cursor.image = NULL;
    if (ioctl(pAG10E->psdp->fd, FBIOGCURSOR, &pAG10E->Cursor) == -1) {
    	xf86Msg(X_ERROR, "Hardware cursor isn't available\n");
	return FALSE;
    }

    infoPtr = xf86CreateCursorInfoRec();
    if(!infoPtr) return FALSE;
    
    pAG10E->CursorInfoRec = infoPtr;

    infoPtr->MaxWidth = 64;
    infoPtr->MaxHeight = 64;
    
    pAG10E->Cursor.hot.x = 0;
    pAG10E->Cursor.hot.y = 0;
    pAG10E->Cursor.set = FB_CUR_SETHOT;
    ioctl(pAG10E->psdp->fd, FBIOSCURSOR, &pAG10E->Cursor);
    
    pAG10E->Cursor.cmap.red = pAG10E->pal;
    pAG10E->Cursor.cmap.green = pAG10E->pal + 3;
    pAG10E->Cursor.cmap.blue = pAG10E->pal + 6;
    
    infoPtr->Flags = HARDWARE_CURSOR_AND_SOURCE_WITH_MASK |
	HARDWARE_CURSOR_TRUECOLOR_AT_8BPP;

    infoPtr->SetCursorColors = AG10ESetCursorColors;
    infoPtr->SetCursorPosition = AG10ESetCursorPosition;
    infoPtr->LoadCursorImage = AG10ELoadCursorImage;
    infoPtr->HideCursor = AG10EHideCursor;
    infoPtr->ShowCursor = AG10EShowCursor;
    infoPtr->UseHWCursor = NULL;

    return xf86InitCursor(pScreen, infoPtr);
}

/* $XFree86: xc/programs/Xserver/hw/xfree86/drivers/ati/radeon_cursor.c,v 1.5 2001/03/03 22:26:10 tsi Exp $ */
/*
 * Copyright 2000 ATI Technologies Inc., Markham, Ontario, and
 *                VA Linux Systems Inc., Fremont, California.
 *
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation on the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial
 * portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NON-INFRINGEMENT.  IN NO EVENT SHALL ATI, VA LINUX SYSTEMS AND/OR
 * THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

/*
 * Authors:
 *   Kevin E. Martin <martin@valinux.com>
 *   Rickard E. Faith <faith@valinux.com>
 *
 * References:
 *
 * !!!! FIXME !!!!
 *   RAGE 128 VR/ RAGE 128 GL Register Reference Manual (Technical
 *   Reference Manual P/N RRG-G04100-C Rev. 0.04), ATI Technologies: April
 *   1999.
 *
 *   RAGE 128 Software Development Manual (Technical Reference Manual P/N
 *   SDK-G04000 Rev. 0.01), ATI Technologies: June 1999.
 *
 */

				/* Driver data structures */
#include "radeon.h"
#include "radeon_reg.h"

				/* X and server generic header files */
#include "xf86.h"

#if X_BYTE_ORDER == X_BIG_ENDIAN
#define P_SWAP32( a , b )                 \
       ((char *)a)[0] = ((char *)b)[3];   \
       ((char *)a)[1] = ((char *)b)[2];   \
       ((char *)a)[2] = ((char *)b)[1];   \
       ((char *)a)[3] = ((char *)b)[0]

#define P_SWAP16( a , b )                 \
       ((char *)a)[0] = ((char *)b)[1];   \
       ((char *)a)[1] = ((char *)b)[0];   \
       ((char *)a)[2] = ((char *)b)[3];   \
       ((char *)a)[3] = ((char *)b)[2]
#endif


/* Set cursor foreground and background colors. */
static void RADEONSetCursorColors(ScrnInfoPtr pScrn, int bg, int fg)
{
    RADEONInfoPtr info        = RADEONPTR(pScrn);
    unsigned char *RADEONMMIO = info->MMIO;

    OUTREG(RADEON_CUR_CLR0, bg);
    OUTREG(RADEON_CUR_CLR1, fg);
}

/* Set cursor position to (x,y) with offset into cursor bitmap at
   (xorigin,yorigin). */
static void RADEONSetCursorPosition(ScrnInfoPtr pScrn, int x, int y)
{
    RADEONInfoPtr         info        = RADEONPTR(pScrn);
    unsigned char         *RADEONMMIO = info->MMIO;
    xf86CursorInfoPtr     cursor      = info->cursor;
    int                   xorigin     = 0;
    int                   yorigin     = 0;
    int                   total_y     = pScrn->frameY1 - pScrn->frameY0;

    if (x < 0)                        xorigin = -x;
    if (y < 0)                        yorigin = -y;
    if (y > total_y)                  y       = total_y;
    if (info->Flags & V_DBLSCAN)      y       *= 2;
    if (xorigin >= cursor->MaxWidth)  xorigin = cursor->MaxWidth - 1;
    if (yorigin >= cursor->MaxHeight) yorigin = cursor->MaxHeight - 1;

    OUTREG(RADEON_CUR_HORZ_VERT_OFF,  (RADEON_CUR_LOCK
				       | (xorigin << 16)
				       | yorigin));
    OUTREG(RADEON_CUR_HORZ_VERT_POSN, (RADEON_CUR_LOCK
				       | ((xorigin ? 0 : x) << 16)
				       | (yorigin ? 0 : y)));
    OUTREG(RADEON_CUR_OFFSET,         info->cursor_start + yorigin * 16);
}

/* Copy cursor image from `image' to video memory.  RADEONSetCursorPosition
   will be called after this, so we can ignore xorigin and yorigin. */
static void RADEONLoadCursorImage(ScrnInfoPtr pScrn, unsigned char *image)
{
    RADEONInfoPtr info        = RADEONPTR(pScrn);
    unsigned char *RADEONMMIO = info->MMIO;
    CARD32        *s          = (pointer)image;
    CARD32        *d          = (pointer)(info->FB + info->cursor_start);
    int           y;
    CARD32        save;

    save = INREG(RADEON_CRTC_GEN_CNTL);
    OUTREG(RADEON_CRTC_GEN_CNTL, save & (CARD32)~RADEON_CRTC_CUR_EN);

#if X_BYTE_ORDER == X_BIG_ENDIAN
    switch(info->CurrentLayout.pixel_bytes) {
    case 4:
    case 3:
	for (y = 0; y < 64; y++) {
	    P_SWAP32(d,s);
	    d++; s++;
	    P_SWAP32(d,s);
	    d++; s++;
	    P_SWAP32(d,s);
	    d++; s++;
	    P_SWAP32(d,s);
	    d++; s++;
	}
	break;
    case 2:
	for (y = 0; y < 64; y++) {
	    P_SWAP16(d,s);
	    d++; s++;
	    P_SWAP16(d,s);
	    d++; s++;
	    P_SWAP16(d,s);
	    d++; s++;
	    P_SWAP16(d,s);
	    d++; s++;
	}
	break;
    default:
	for (y = 0; y < 64; y++) {
	    *d++ = *s++;
	    *d++ = *s++;
	    *d++ = *s++;
	    *d++ = *s++;
	}
    }
#else
    for (y = 0; y < 64; y++) {
	*d++ = *s++;
	*d++ = *s++;
	*d++ = *s++;
	*d++ = *s++;
    }
#endif

    /* Set the area after the cursor to be all transparent so that we
       won't display corrupted cursors on the screen */
    for (y = 0; y < 64; y++) {
	*d++ = 0xffffffff; /* The AND bits */
	*d++ = 0xffffffff;
	*d++ = 0x00000000; /* The XOR bits */
	*d++ = 0x00000000;
    }

    OUTREG(RADEON_CRTC_GEN_CNTL, save);
}

/* Hide hardware cursor. */
static void RADEONHideCursor(ScrnInfoPtr pScrn)
{
    RADEONInfoPtr info        = RADEONPTR(pScrn);
    unsigned char *RADEONMMIO = info->MMIO;

    OUTREGP(RADEON_CRTC_GEN_CNTL, 0, ~RADEON_CRTC_CUR_EN);
}

/* Show hardware cursor. */
static void RADEONShowCursor(ScrnInfoPtr pScrn)
{
    RADEONInfoPtr info        = RADEONPTR(pScrn);
    unsigned char *RADEONMMIO = info->MMIO;

    OUTREGP(RADEON_CRTC_GEN_CNTL, RADEON_CRTC_CUR_EN, ~RADEON_CRTC_CUR_EN);
}

/* Determine if hardware cursor is in use. */
static Bool RADEONUseHWCursor(ScreenPtr pScreen, CursorPtr pCurs)
{
    ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
    RADEONInfoPtr info  = RADEONPTR(pScrn);

    return info->cursor_start ? TRUE : FALSE;
}

/* Initialize hardware cursor support. */
Bool RADEONCursorInit(ScreenPtr pScreen)
{
    ScrnInfoPtr           pScrn   = xf86Screens[pScreen->myNum];
    RADEONInfoPtr           info    = RADEONPTR(pScrn);
    xf86CursorInfoPtr     cursor;
    FBAreaPtr             fbarea;
    int                   width;
    int                   height;
    int                   size;


    if (!(cursor = info->cursor = xf86CreateCursorInfoRec())) return FALSE;

    cursor->MaxWidth          = 64;
    cursor->MaxHeight         = 64;
    cursor->Flags             = (HARDWARE_CURSOR_TRUECOLOR_AT_8BPP

#if X_BYTE_ORDER == X_LITTLE_ENDIAN
				 | HARDWARE_CURSOR_BIT_ORDER_MSBFIRST
#endif
				 | HARDWARE_CURSOR_INVERT_MASK
				 | HARDWARE_CURSOR_AND_SOURCE_WITH_MASK
				 | HARDWARE_CURSOR_SOURCE_MASK_INTERLEAVE_64
				 | HARDWARE_CURSOR_SWAP_SOURCE_AND_MASK);

    cursor->SetCursorColors   = RADEONSetCursorColors;
    cursor->SetCursorPosition = RADEONSetCursorPosition;
    cursor->LoadCursorImage   = RADEONLoadCursorImage;
    cursor->HideCursor        = RADEONHideCursor;
    cursor->ShowCursor        = RADEONShowCursor;
    cursor->UseHWCursor       = RADEONUseHWCursor;

    size                      = (cursor->MaxWidth/4) * cursor->MaxHeight;
    width                     = pScrn->displayWidth;
    height                    = (size*2 + 1023) / pScrn->displayWidth;
    fbarea                    = xf86AllocateOffscreenArea(pScreen,
							  width,
							  height,
							  16,
							  NULL,
							  NULL,
							  NULL);

    if (!fbarea) {
	info->cursor_start    = 0;
	xf86DrvMsg(pScrn->scrnIndex, X_WARNING,
		   "Hardware cursor disabled"
		   " due to insufficient offscreen memory\n");
    } else {
	info->cursor_start    = RADEON_ALIGN((fbarea->box.x1
					      + width * fbarea->box.y1)
					     * info->CurrentLayout.pixel_bytes,
					     16);
	info->cursor_end      = info->cursor_start + size;
    }

    RADEONTRACE(("RADEONCursorInit (0x%08x-0x%08x)\n",
		 info->cursor_start, info->cursor_end));

    return xf86InitCursor(pScreen, cursor);
}

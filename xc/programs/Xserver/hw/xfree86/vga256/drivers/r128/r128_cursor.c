/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/r128/r128_cursor.c,v 1.1.2.1 1999/10/11 21:13:51 hohndel Exp $ */
/**************************************************************************

Copyright 1999 ATI Technologies Inc. and Precision Insight, Inc.,
                                         Cedar Park, Texas. 
All Rights Reserved.

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
on the rights to use, copy, modify, merge, publish, distribute, sub
license, and/or sell copies of the Software, and to permit persons to whom
the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice (including the next
paragraph) shall be included in all copies or substantial portions of the
Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
ATI, PRECISION INSIGHT AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM,
DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
USE OR OTHER DEALINGS IN THE SOFTWARE.

**************************************************************************/

/*
 * Authors:
 *   Rickard E. Faith <faith@precisioninsight.com>
 *   Kevin E. Martin <kevin@precisioninsight.com>
 *
 * References:
 *
 *   RAGE 128 VR/ RAGE 128 GL Register Reference Manual (Technical
 *   Reference Manual P/N RRG-G04100-C Rev. 0.04), ATI Technologies: April
 *   1999.
 *
 *   RAGE 128 Software Development Manual (Technical Reference Manual P/N
 *   SDK-G04000 Rev. 0.01), ATI Technologies: June 1999.
 *
 * $PI: xc/programs/Xserver/hw/xfree86/vga256/drivers/r128/r128_cursor.c,v 1.11 1999/10/08 08:28:46 faith Exp $
 */

#include "compiler.h"
#include "cursorstr.h"
#include "vga.h"
#include "vga256.h"
#include "xf86cursor.h"

#include "xf86Priv.h"
#include "xf86_Config.h"
#include "xf86_PCI.h"

#include "r128.h"
#include "r128_reg.h"

/* Set cursor foreground and background colors. */
static void R128SetCursorColors(int bg, int fg)
{
    unsigned char *R128MMIO      = R128PTR()->MMIO;

    OUTREG(R128_CUR_CLR0, bg);
    OUTREG(R128_CUR_CLR1, fg);
}

/* Set cursor position to (x,y) with offset into cursor bitmap at
   (xorigin,yorigin). */
static void R128SetCursorPosition(int x, int y, int xorigin, int yorigin)
{
    R128InfoPtr   info      = R128PTR();
    unsigned char *R128MMIO = info->MMIO;
    int           total_y   = vga256InfoRec.frameY1 - vga256InfoRec.frameY0;
    int           total_x   = vga256InfoRec.frameX1 - vga256InfoRec.frameX0;
    int           fudge     = 2;

    if (info->Flags & V_DBLSCAN) {
	y       *= 2;
	total_y *= 2;
	fudge   *= 2;
    }
    
				/* Don't draw cursor off edge of screen.
                                   Instead of `fudge', we should store the
                                   cursor hot point and use that. */
    if (y > total_y) y = total_y - fudge;
    if (x > total_x) x = total_x - fudge;
    if (y < 0)       y = fudge;
    if (x < 0)       x = fudge;

    OUTREG(R128_CUR_HORZ_VERT_OFF,  R128_CUR_LOCK | (xorigin << 16) | yorigin);
    OUTREG(R128_CUR_HORZ_VERT_POSN, (R128_CUR_LOCK
				     | ((xorigin ? 0 : x) << 16)
				     | (yorigin ? 0 : y)));
    OUTREG(R128_CUR_OFFSET,         info->cursor_start + yorigin * 16);
}

/* Copy cursor image from `image' to video memory.  R128SetCursorPosition
   will be called after this, so we can ignore xorigin and yorigin. */
static void R128LoadCursorImage(void *image, int xorigin, int yorigin)
{
    R128InfoPtr   info      = R128PTR();
    CARD32        *s        = image;
    CARD32        *d        = (CARD32 *)((unsigned long)vgaLinearBase
					 + info->cursor_start);
    int           y;

				/* Waiting for vertical sync eliminates the
                                   problem that the whole 64x64 cursor
                                   block will "flash" during this routine.
                                   However, this causes the cursor to
                                   flicker when being updated in the last
                                   few scanlines on the screen.  Flickering
                                   is extremely back in double scan modes,
                                   so don't bother waiting in those
                                   modes. */
    if (!(info->Flags & V_DBLSCAN)) R128WaitForVerticalSync();
    
    for (y = 0; y < 64; y++, s += 4) {
				/* Interleave by 8-byte line.  In XFree86
                                   4.0, be sure to try 64-bit interleaving
                                   to make this even simpler
                                   (HARDWARE_CURSOR_INT64_BIT_FORMAT
                                   doesn't seem to work in XFree86
                                   3.3.5). */
	*d++ = s[0];
	*d++ = s[2];
	*d++ = s[1];
	*d++ = s[3];
    }

    if (info->Flags & V_DBLSCAN) {
				/* This is a kludge to get the whole cursor
                                   to appear after the load, but before
                                   anything else is rendered on the screen.
                                   In XFree86 4.0, we can just disable the
                                   hardware cursor for double scan modes,
                                   and we won't need this any more. */
	CARD32 save;
	int    i;

	for (i = 0; i < info->virtual_x; i += 64) {
	    save = *(CARD32 *)((char *)vgaLinearBase + i);
	    *(CARD32 *)((char *)vgaLinearBase + i) = 0;
	    *(CARD32 *)((char *)vgaLinearBase + i) = save;
	}
    }
}

/* Hide hardware cursor. */
static void R128HideCursor(void)
{
    unsigned char *R128MMIO      = R128PTR()->MMIO;
    
    OUTREGP(R128_CRTC_GEN_CNTL, 0, ~R128_CRTC_CUR_EN);
}

/* Show hardware cursor. */
static void R128ShowCursor(void)
{
    unsigned char *R128MMIO      = R128PTR()->MMIO;
    
    OUTREGP(R128_CRTC_GEN_CNTL, R128_CRTC_CUR_EN, ~R128_CRTC_CUR_EN);
}

/* Initialize hardware cursor support. */
void R128CursorInit(void)
{
    R128InfoPtr           info           = R128PTR();
    extern vgaHWCursorRec vgaHWCursor;
    
				/* These are exported, but aren't declared
                                   in any header files... */
    extern Bool           XAACursorInit(char *pm, ScreenPtr pScr);
    extern void           XAARestoreCursor(ScreenPtr pScr);
    extern void           XAAWarpCursor(ScreenPtr pScr, int x, int y);
    extern void           XAAQueryBestSize(int class, unsigned short *pwidth,
					   unsigned short *pheight,
					   ScreenPtr pScreen);
    
    info->cursor_start = R128_ALIGN(info->virtual_y
				    * info->virtual_x
				    * info->pixel_bytes,
				    16);
    info->cursor_end   = info->cursor_start + 1024; /* 1k cursor area */

    R128DEBUG(("R128CursorInit (0x%08x-0x%08x)\n",
	       info->cursor_start, info->cursor_end));
    
    XAACursorInfoRec.MaxWidth  = 64;
    XAACursorInfoRec.MaxHeight = 64;

    XAACursorInfoRec.Flags     = (USE_HARDWARE_CURSOR
				  | HARDWARE_CURSOR_TRUECOLOR_AT_8BPP
				  | HARDWARE_CURSOR_PROGRAMMED_BITS
				  | HARDWARE_CURSOR_BIT_ORDER_MSBFIRST
				  | HARDWARE_CURSOR_PROGRAMMED_ORIGIN
				  | HARDWARE_CURSOR_AND_SOURCE_WITH_MASK
				  | HARDWARE_CURSOR_LONG_BIT_FORMAT
				  | HARDWARE_CURSOR_INVERT_MASK);

    XAACursorInfoRec.SetCursorColors   = R128SetCursorColors;
    XAACursorInfoRec.SetCursorPosition = R128SetCursorPosition;
    XAACursorInfoRec.LoadCursorImage   = R128LoadCursorImage;
    XAACursorInfoRec.HideCursor        = R128HideCursor;
    XAACursorInfoRec.ShowCursor        = R128ShowCursor;

    vgaHWCursor.Init                   = XAACursorInit;
    vgaHWCursor.Initialized            = TRUE;
    vgaHWCursor.Restore                = XAARestoreCursor;
    vgaHWCursor.Warp                   = XAAWarpCursor;
    vgaHWCursor.QueryBestSize          = XAAQueryBestSize;
}

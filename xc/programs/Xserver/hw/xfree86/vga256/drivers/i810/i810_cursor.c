/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/i810/i810_cursor.c,v 1.1.2.2 1999/11/18 19:06:17 hohndel Exp $ */
/**************************************************************************

Copyright 1998-1999 Precision Insight, Inc., Cedar Park, Texas.
All Rights Reserved.

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sub license, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice (including the
next paragraph) shall be included in all copies or substantial portions
of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
IN NO EVENT SHALL PRECISION INSIGHT AND/OR ITS SUPPLIERS BE LIABLE FOR
ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

**************************************************************************/

/*
 * Authors:
 *   Kevin E. Martin <kevin@precisioninsight.com>
 *   Ported to i810 by Keith Whitwell <keithw@precisioninsight.com>
 *
 * $PI$
 */
#include <stdio.h>

#include "cursorstr.h"
#include "vga.h"
#include "vga256.h"
#include "xf86cursor.h"

#include "i810.h"
#include "i810_reg.h"

extern void I810SetCursorColors();
extern void I810SetCursorPosition();
extern void I810LoadCursorImage();
extern void I810HideCursor();
extern void I810ShowCursor();

extern vgaHWCursorRec vgaHWCursor;

extern Bool XAACursorInit();
extern void XAARestoreCursor();
extern void XAAWarpCursor();
extern void XAAQueryBestSize();

void
I810CursorInit()
{
    XAACursorInfoRec.MaxWidth = 64;
    XAACursorInfoRec.MaxHeight = 64;

    XAACursorInfoRec.Flags = (USE_HARDWARE_CURSOR |
			      HARDWARE_CURSOR_TRUECOLOR_AT_8BPP |
			      HARDWARE_CURSOR_PROGRAMMED_ORIGIN |
			      HARDWARE_CURSOR_CHAR_BIT_FORMAT |
			      HARDWARE_CURSOR_BIT_ORDER_MSBFIRST |
			      HARDWARE_CURSOR_INVERT_MASK |
			      HARDWARE_CURSOR_AND_SOURCE_WITH_MASK |
			      HARDWARE_CURSOR_PROGRAMMED_BITS);

    XAACursorInfoRec.SetCursorColors = I810SetCursorColors;
    XAACursorInfoRec.SetCursorPosition = I810SetCursorPosition;
    XAACursorInfoRec.LoadCursorImage = I810LoadCursorImage;
    XAACursorInfoRec.HideCursor = I810HideCursor;
    XAACursorInfoRec.ShowCursor = I810ShowCursor;

    XAACursorInfoRec.CursorDataX =
	(I810Cursor.Start % (vga256InfoRec.displayWidth * vgaBytesPerPixel))
	/ vgaBytesPerPixel;
    XAACursorInfoRec.CursorDataY =
	I810Cursor.Start / (vga256InfoRec.displayWidth * vgaBytesPerPixel);

    vgaHWCursor.Init = XAACursorInit;
    vgaHWCursor.Initialized = TRUE;
    vgaHWCursor.Restore = XAARestoreCursor;
    vgaHWCursor.Warp = XAAWarpCursor;
    vgaHWCursor.QueryBestSize = XAAQueryBestSize;
}

void
I810SetCursorColors(bg, fg)
    unsigned int bg, fg;
{
    int tmp;

    tmp = INREG8(PIXPIPE_CONFIG_0); 
    tmp |= EXTENDED_PALETTE;
    OUTREG8( PIXPIPE_CONFIG_0, tmp );

    outb(DACMASK, 0xFF);
    outb(DACWX,   0x04);

    outb(DACDATA, (bg & 0x00FF0000) >> 16);
    outb(DACDATA, (bg & 0x0000FF00) >> 8);
    outb(DACDATA, (bg & 0x000000FF));

    outb(DACDATA, (fg & 0x00FF0000) >> 16);
    outb(DACDATA, (fg & 0x0000FF00) >> 8);
    outb(DACDATA, (fg & 0x000000FF));

    tmp = INREG8(PIXPIPE_CONFIG_0);
    tmp &= ~EXTENDED_PALETTE;
    OUTREG8( PIXPIPE_CONFIG_0, tmp );
}

void
I810SetCursorPosition(x, y, xoff, yoff)
    int x, y, xoff, yoff;
{
    x -= xoff;
    y -= yoff;

    x += I810CursorOffset;

    if (x >= 0) {
       OUTREG8( CURSOR_X_LO,    x       & 0xFF );
       OUTREG8( CURSOR_X_HI, (((x >> 8 ) & 0x07) |
				    CURSOR_X_POS));
    } else {
       OUTREG8( CURSOR_X_LO,    -x       & 0xFF );
       OUTREG8( CURSOR_X_HI, (((-x >> 8 ) & 0x07) |
					     CURSOR_X_NEG));
    }

    /*
     * This is a hack.
     *
     * This is not the entirely correct way to handle double scan
     * modes.  One more correct way would be to add a flag to XAA that
     * correctly doubles the Y values and the CursorHotY value.
     * Another correct way would be to add an XAA flag to switch to a
     * software cursor when in a double scan mode.
     */
    if (vga256InfoRec.modes->Flags & V_DBLSCAN)
	y *= 2; 

    if (y >= 0) {
       OUTREG8( CURSOR_Y_LO,    y       & 0xFF );
       OUTREG8( CURSOR_Y_HI, (((y >> 8 ) & 0x07) |
					     CURSOR_Y_POS));
    } else {
       OUTREG8( CURSOR_Y_LO,    -y       & 0xFF );
       OUTREG8( CURSOR_Y_HI, (((-y >> 8 ) & 0x07) |
					     CURSOR_Y_NEG));
    }
}

void
I810LoadCursorImage(image, xoff, yoff)
    unsigned char *image;
    int xoff, yoff;
{
    int x, y;
    unsigned int pcurs = (unsigned int)vgaLinearBase + I810Cursor.Start;

    static int conv[] = { 0, 2, 4, 6, 8, 10, 12, 14, 1, 3, 5, 7, 9, 11, 13, 15 };

    if (0)
       fprintf(stderr, "Load cursor %x %x\n", vgaLinearBase, I810Cursor.Start);

    for (y = 0; y < XAACursorInfoRec.MaxHeight; y++) {
	for (x = 0; x < XAACursorInfoRec.MaxWidth / 4; x++) {
	    *(volatile CARD8 *)pcurs++ = 
		image[conv[x] + y * XAACursorInfoRec.MaxWidth/4];
	}
    }
}

void
I810HideCursor()
{
    unsigned char tmp;

    tmp = INREG8(PIXPIPE_CONFIG_0);
    tmp &= ~HW_CURSOR_ENABLE;
    OUTREG8( PIXPIPE_CONFIG_0, tmp );
}

void
I810ShowCursor()
{
    unsigned char tmp;

    OUTREG( CURSOR_BASEADDR, (I810CursorPhysical & CURSOR_BASEADDR_MASK) );
    OUTREG8( CURSOR_CONTROL, CURSOR_ORIGIN_DISPLAY | CURSOR_MODE_64_3C );

    tmp = INREG8(PIXPIPE_CONFIG_0);
    tmp |= HW_CURSOR_ENABLE;
    OUTREG8( PIXPIPE_CONFIG_0, tmp );
}

/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/i740/i740_cursor.c,v 1.1.2.2 1999/05/14 09:00:20 dawes Exp $ */
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
 *
 * $PI: i740_cursor.c,v 1.5 1999/02/18 20:50:59 martin Exp martin $
 */

#include "compiler.h"
#include "cursorstr.h"
#include "vga.h"
#include "vga256.h"
#include "xf86cursor.h"

#include "i740.h"
#include "i740_reg.h"
#include "i740_macros.h"

extern void I740SetCursorColors();
extern void I740SetCursorPosition();
extern void I740LoadCursorImage();
extern void I740HideCursor();
extern void I740ShowCursor();

extern vgaHWCursorRec vgaHWCursor;

extern Bool XAACursorInit();
extern void XAARestoreCursor();
extern void XAAWarpCursor();
extern void XAAQueryBestSize();

void
I740CursorInit()
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

    XAACursorInfoRec.SetCursorColors = I740SetCursorColors;
    XAACursorInfoRec.SetCursorPosition = I740SetCursorPosition;
    XAACursorInfoRec.LoadCursorImage = I740LoadCursorImage;
    XAACursorInfoRec.HideCursor = I740HideCursor;
    XAACursorInfoRec.ShowCursor = I740ShowCursor;

    XAACursorInfoRec.CursorDataX =
	(I740CursorStart % (vga256InfoRec.displayWidth * vgaBytesPerPixel))
	/ vgaBytesPerPixel;
    XAACursorInfoRec.CursorDataY =
	I740CursorStart / (vga256InfoRec.displayWidth * vgaBytesPerPixel);

    vgaHWCursor.Init = XAACursorInit;
    vgaHWCursor.Initialized = TRUE;
    vgaHWCursor.Restore = XAARestoreCursor;
    vgaHWCursor.Warp = XAAWarpCursor;
    vgaHWCursor.QueryBestSize = XAAQueryBestSize;
}

void
I740SetCursorColors(bg, fg)
    unsigned int bg, fg;
{
    int tmp;

    outb(XRX, PIXPIPE_CONFIG_0); tmp = inb(XRX+1);
    tmp |= EXTENDED_PALETTE;
    outb(XRX, PIXPIPE_CONFIG_0); outb(XRX+1, tmp);

    outb(DACMASK, 0xFF);
    outb(DACWX,   0x04);

    outb(DACDATA, (bg & 0x00FF0000) >> 16);
    outb(DACDATA, (bg & 0x0000FF00) >> 8);
    outb(DACDATA, (bg & 0x000000FF));

    outb(DACDATA, (fg & 0x00FF0000) >> 16);
    outb(DACDATA, (fg & 0x0000FF00) >> 8);
    outb(DACDATA, (fg & 0x000000FF));

    outb(XRX, PIXPIPE_CONFIG_0); tmp = inb(XRX+1);
    tmp &= ~EXTENDED_PALETTE;
    outb(XRX, PIXPIPE_CONFIG_0); outb(XRX+1, tmp);
}

void
I740SetCursorPosition(x, y, xoff, yoff)
    int x, y, xoff, yoff;
{
    x -= xoff;
    y -= yoff;

    if (x >= 0) {
	outb(XRX, CURSOR_X_LO); outb(XRX+1,    x       & 0xFF);
	outb(XRX, CURSOR_X_HI); outb(XRX+1, (((x >> 8) & 0x07) |
					     CURSOR_X_POS));
    } else {
	outb(XRX, CURSOR_X_LO); outb(XRX+1,    -x       & 0xFF);
	outb(XRX, CURSOR_X_HI); outb(XRX+1, (((-x >> 8) & 0x07) |
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
	outb(XRX, CURSOR_Y_LO); outb(XRX+1,    y       & 0xFF);
	outb(XRX, CURSOR_Y_HI); outb(XRX+1, (((y >> 8) & 0x07) |
					     CURSOR_Y_POS));
    } else {
	outb(XRX, CURSOR_Y_LO); outb(XRX+1,    -y       & 0xFF);
	outb(XRX, CURSOR_Y_HI); outb(XRX+1, (((-y >> 8) & 0x07) |
					     CURSOR_Y_NEG));
    }
}

void
I740LoadCursorImage(image, xoff, yoff)
    unsigned char *image;
    int xoff, yoff;
{
    int x, y;
    unsigned int pcurs = (unsigned int)vgaLinearBase + I740CursorStart;

    static int conv[] = { 0, 2, 4, 6, 8, 10, 12, 14, 1, 3, 5, 7, 9, 11, 13, 15 };

    for (y = 0; y < XAACursorInfoRec.MaxHeight; y++) {
	for (x = 0; x < XAACursorInfoRec.MaxWidth / 4; x++) {
	    *(volatile CARD8 *)pcurs++ = 
		image[conv[x] + y * XAACursorInfoRec.MaxWidth/4];
	}
    }
}

void
I740HideCursor()
{
    unsigned char tmp;

    outb(XRX, PIXPIPE_CONFIG_0); tmp = inb(XRX+1);
    tmp &= ~HW_CURSOR_ENABLE;
    outb(XRX, PIXPIPE_CONFIG_0); outb(XRX+1, tmp);
}

void
I740ShowCursor()
{
    unsigned char tmp;

    outb(XRX, CURSOR_BASEADDR_LO);
    outb(XRX+1, (I740CursorStart & 0x0000F000) >> 8);
    outb(XRX, CURSOR_BASEADDR_HI);
    outb(XRX+1, (I740CursorStart & 0x003F0000) >> 16);
    outb(XRX, CURSOR_CONTROL);
    outb(XRX+1, CURSOR_ORIGIN_DISPLAY | CURSOR_MODE_64_3C);

    outb(XRX, PIXPIPE_CONFIG_0); tmp = inb(XRX+1);
    tmp |= HW_CURSOR_ENABLE;
    outb(XRX, PIXPIPE_CONFIG_0); outb(XRX+1, tmp);
}

/* $XFree86: xc/programs/Xserver/hw/xfree86/accel/glint/glintIBMCurs.c,v 1.8.2.1 1998/07/30 06:23:40 hohndel Exp $ */
/*
 * Copyright 1996 by Alan Hourihane <alanh@fairlite.demon.co.uk>
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Alan Hourihane not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  Alan Hourihane makes no representations
 * about the suitability of this software for any purpose.  It is provided
 * "as is" without express or implied warranty.
 *
 * ALAN HOURIHANE DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL ALAN HOURIHANE BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 *
 * Written by Alan Hourihane <alanh@fairlite.demon.co.uk>
 * for the 3Dlabs GLINT chipset
 */
#include "glint_regs.h"
#include "glint.h"
#define GLINT_SERVER
#include "IBMRGB.h"

extern Bool UsePCIRetry;

void 
glintIBMShowCursor()
{
   unsigned char tmp;

   /* Enable cursor - X11 mode */
   tmp = GLINT_READ_REG(IBMRGB_INDEX_LOW);
   GLINT_SLOW_WRITE_REG(IBMRGB_curs, IBMRGB_INDEX_LOW);
   GLINT_SLOW_WRITE_REG(0x27, IBMRGB_INDEX_DATA);

   GLINT_SLOW_WRITE_REG(tmp, IBMRGB_INDEX_LOW);
}

void
glintIBMHideCursor()
{
   unsigned char tmp, tmp1;

   tmp = GLINT_READ_REG(IBMRGB_INDEX_LOW);
   GLINT_SLOW_WRITE_REG(IBMRGB_curs, IBMRGB_INDEX_LOW);
   tmp1 = GLINT_READ_REG(IBMRGB_INDEX_DATA) & ~3;
   GLINT_SLOW_WRITE_REG(tmp1, IBMRGB_INDEX_DATA);

   GLINT_SLOW_WRITE_REG(tmp, IBMRGB_INDEX_LOW);
}

void
glintIBMSetCursorPosition(x, y, xorigin, yorigin)
{
   unsigned char tmp;

   tmp = GLINT_READ_REG(IBMRGB_INDEX_LOW);
   
   GLINT_SLOW_WRITE_REG(IBMRGB_curs_hot_x, IBMRGB_INDEX_LOW);
   GLINT_SLOW_WRITE_REG(xorigin & 0xFF, IBMRGB_INDEX_DATA);
   GLINT_SLOW_WRITE_REG(IBMRGB_curs_hot_y, IBMRGB_INDEX_LOW);
   GLINT_SLOW_WRITE_REG(yorigin & 0xFF, IBMRGB_INDEX_DATA);
   GLINT_SLOW_WRITE_REG(IBMRGB_curs_xl, IBMRGB_INDEX_LOW);
   GLINT_SLOW_WRITE_REG(x & 0xFF, IBMRGB_INDEX_DATA);
   GLINT_SLOW_WRITE_REG(IBMRGB_curs_xh, IBMRGB_INDEX_LOW);
   GLINT_SLOW_WRITE_REG((x >> 8) & 0x0F, IBMRGB_INDEX_DATA);
   GLINT_SLOW_WRITE_REG(IBMRGB_curs_yl, IBMRGB_INDEX_LOW);
   GLINT_SLOW_WRITE_REG(y & 0xFF, IBMRGB_INDEX_DATA);
   GLINT_SLOW_WRITE_REG(IBMRGB_curs_yh, IBMRGB_INDEX_LOW);
   GLINT_SLOW_WRITE_REG((y >> 8) & 0x0F, IBMRGB_INDEX_DATA);

   GLINT_SLOW_WRITE_REG(tmp, IBMRGB_INDEX_LOW);
}

void
glintIBMSetCursorColors(bg, fg)
	int bg, fg;
{
   unsigned char tmp;

   tmp = GLINT_READ_REG(IBMRGB_INDEX_LOW);

   /* Background color */
   GLINT_SLOW_WRITE_REG(IBMRGB_curs_col1_r , IBMRGB_INDEX_LOW);
   GLINT_SLOW_WRITE_REG((bg & 0x00FF0000) >> 16, IBMRGB_INDEX_DATA);
   GLINT_SLOW_WRITE_REG(IBMRGB_curs_col1_g, IBMRGB_INDEX_LOW);
   GLINT_SLOW_WRITE_REG((bg & 0x0000FF00) >> 8, IBMRGB_INDEX_DATA);
   GLINT_SLOW_WRITE_REG(IBMRGB_curs_col1_b, IBMRGB_INDEX_LOW);
   GLINT_SLOW_WRITE_REG((bg & 0x000000FF), IBMRGB_INDEX_DATA);

   /* Foreground color */
   GLINT_SLOW_WRITE_REG(IBMRGB_curs_col2_r, IBMRGB_INDEX_LOW);
   GLINT_SLOW_WRITE_REG((fg & 0x00FF0000) >> 16, IBMRGB_INDEX_DATA);
   GLINT_SLOW_WRITE_REG(IBMRGB_curs_col2_g, IBMRGB_INDEX_LOW);
   GLINT_SLOW_WRITE_REG((fg & 0x0000FF00) >> 8, IBMRGB_INDEX_DATA);
   GLINT_SLOW_WRITE_REG(IBMRGB_curs_col2_b, IBMRGB_INDEX_LOW);
   GLINT_SLOW_WRITE_REG((fg & 0x000000FF), IBMRGB_INDEX_DATA);

   GLINT_SLOW_WRITE_REG(tmp, IBMRGB_INDEX_LOW);
}

void 
glintIBMLoadCursorImage(bits, xorigin, yorigin)
	unsigned char *bits;
	int xorigin, yorigin;
{
   unsigned char tmp, tmp1, tmpcurs;
   int i;

   tmp = GLINT_READ_REG(IBMRGB_INDEX_LOW);

   /* turn the cursor off */
   GLINT_SLOW_WRITE_REG(IBMRGB_curs, IBMRGB_INDEX_LOW);
   if ((tmpcurs = GLINT_READ_REG(IBMRGB_INDEX_DATA) & 0x03))
      glintIBMHideCursor();

   GLINT_SLOW_WRITE_REG(IBMRGB_curs_hot_x, IBMRGB_INDEX_LOW);
   GLINT_SLOW_WRITE_REG(xorigin & 0xFF, IBMRGB_INDEX_DATA);
   GLINT_SLOW_WRITE_REG(IBMRGB_curs_hot_y, IBMRGB_INDEX_LOW);
   GLINT_SLOW_WRITE_REG(yorigin & 0xFF, IBMRGB_INDEX_DATA);

   tmp1 = GLINT_READ_REG(IBMRGB_INDEX_CONTROL) & 0xFE;
   GLINT_SLOW_WRITE_REG(tmp1 | 1, IBMRGB_INDEX_CONTROL); /* enable auto-inc */

   GLINT_SLOW_WRITE_REG((IBMRGB_curs_array >> 8) & 0xFF, IBMRGB_INDEX_HIGH);
   GLINT_SLOW_WRITE_REG(IBMRGB_curs_array & 0xFF, IBMRGB_INDEX_LOW);

   /* 
    * Output the cursor data.  The realize function has put the planes into
    * their correct order, so we can just blast this out.
    */
   for (i = 0; i < 1024; i++) {
      GLINT_SLOW_WRITE_REG(*bits++, IBMRGB_INDEX_DATA);
   }

   GLINT_SLOW_WRITE_REG(0, IBMRGB_INDEX_HIGH);
   GLINT_SLOW_WRITE_REG(tmp1, IBMRGB_INDEX_CONTROL);
   GLINT_SLOW_WRITE_REG(tmp, IBMRGB_INDEX_LOW);

   /* turn the cursor on */
   if (tmpcurs & 0x03)
      glintIBMShowCursor();
}

/* $XConsortium: i128TiCurs.c /main/3 1996/03/09 11:17:04 kaleb $ */
/*
 * Copyright 1995 by Robin Cutshaw <robin@XFree86.Org>
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Robin Cutshaw not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  Robin Cutshaw makes no representations
 * about the suitability of this software for any purpose.  It is provided
 * "as is" without express or implied warranty.
 *
 * ROBIN CUTSHAW DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL ROBIN CUTSHAW BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 *
 */

/* $XFree86: xc/programs/Xserver/hw/xfree86/accel/i128/i128TiCurs.c,v 3.4.2.1 1998/01/12 03:02:10 robin Exp $ */

#include "servermd.h"

#include "i128.h"
#include "i128reg.h"
#include "Ti302X.h"

#define MAX_CURS_HEIGHT 64   /* 64 scan lines */
#define MAX_CURS_WIDTH  64   /* 64 pixels     */

extern struct i128mem i128mem;
extern Bool i128BlockCursor;


/*
 * Convert the cursor from server-format to hardware-format.  The Ti3020
 * has two planes, plane 0 selects cursor color 0 or 1 and plane 1
 * selects transparent or display cursor.  The bits of these planes
 * are packed together so that one byte has 4 pixels. The organization
 * looks like:
 *             Byte 0x000 - 0x00F    top scan line, left to right
 *                  0x010 - 0x01F
 *                    .       .
 *                  0x3F0 - 0x3FF    bottom scan line
 *
 *             Byte/bit map - D7D6,D5D4,D3D2,D1D0  four pixels, two planes each
 *             Pixel/bit map - P1P0  (plane 1) == 1 maps to cursor color
 *                                   (plane 1) == 0 maps to transparent
 *                                   (plane 0) maps to cursor colors 0 and 1
 */

Bool
i128TiRealizeCursor(pScr, pCurs)
     ScreenPtr pScr;
     CursorPtr pCurs;
{
   register int i, j;
   unsigned char *pServMsk;
   unsigned char *pServSrc;
   pointer *pPriv;
   int   wsrc, h;
   unsigned char *ram;

   if (pCurs->bits->refcnt > 1)
      return TRUE;

   ram = (unsigned char *)xalloc(1024);  /* 64x64x2 bits */
   pPriv = &pCurs->bits->devPriv[pScr->myNum];
   *pPriv = (pointer) ram;

   if (!ram)
      return FALSE;

   pServSrc = (unsigned char *)pCurs->bits->source;
   pServMsk = (unsigned char *)pCurs->bits->mask;

   h = pCurs->bits->height;
   if (h > MAX_CURS_HEIGHT)
      h = MAX_CURS_HEIGHT;

   wsrc = PixmapBytePad(pCurs->bits->width, 1);	/* bytes per line */

   for (i = 0; i < MAX_CURS_HEIGHT; i++,ram+=16) {
      for (j = 0; j < MAX_CURS_WIDTH / 8; j++) {
	 register unsigned char mask, source;

	 if (i < h && j < wsrc) {
	    /*
	     * mask byte ABCDEFGH and source byte 12345678 map to two byte
	     * cursor data H8G7F6E5 D4C3B2A1
	     */
	    mask = *pServMsk++;
	    source = *pServSrc++ & mask;

	    /* map 1 byte source and mask into two byte cursor data */
	    ram[j*2] =     ((mask&0x01) << 7) | ((source&0x01) << 6) |
		           ((mask&0x02) << 4) | ((source&0x02) << 3) |
		           ((mask&0x04) << 1) | (source&0x04)        |
		           ((mask&0x08) >> 2) | ((source&0x08) >> 3) ;
	    ram[(j*2)+1] = ((mask&0x10) << 3) | ((source&0x10) << 2) |
		           (mask&0x20)        | ((source&0x20) >> 1) |
		           ((mask&0x40) >> 3) | ((source&0x40) >> 4) |
		           ((mask&0x80) >> 6) | ((source&0x80) >> 7) ;
	 } else {
	    ram[j*2]     = 0x00;
	    ram[(j*2)+1] = 0x00;
	 }
      }
      /*
       * if we still have more bytes on this line (j < wsrc),
       * we have to ignore the rest of the line.
       */
       while (j++ < wsrc) pServMsk++,pServSrc++;
   }
   return TRUE;
}

void 
i128TiCursorOn()
{
   CARD32 tmp, tmp1;

   /* Enable cursor - sprite enable, X11 mode */
   tmp = i128mem.rbase_g[INDEX_TI] & 0xFF;
   i128mem.rbase_g[INDEX_TI] = TI_CURS_CONTROL;				MB;
   tmp1 = (i128mem.rbase_g[DATA_TI] & ~TI_CURS_CTRL_MASK) & 0xFF;
   i128mem.rbase_g[DATA_TI] = tmp1 |
                              (TI_CURS_SPRITE_ENABLE | TI_CURS_X_WINDOW_MODE);
									MB;

   i128mem.rbase_g[INDEX_TI] = tmp;					MB;

   return;
}

void
i128TiCursorOff()
{
   CARD32 tmp, tmp1;

   /* Enable cursor - sprite enable, X11 mode */
   tmp = i128mem.rbase_g[INDEX_TI] & 0xFF;
   i128mem.rbase_g[INDEX_TI] = TI_CURS_CONTROL;				MB;
   tmp1 = (i128mem.rbase_g[DATA_TI] & ~TI_CURS_CTRL_MASK) & 0xFF;
   i128mem.rbase_g[DATA_TI] = tmp1;					MB;

   i128mem.rbase_g[INDEX_TI] = tmp;					MB;

   return;
}

void
i128TiMoveCursor(pScr, x, y)
     ScreenPtr pScr;
     int   x, y;
{
   CARD32 tmp;
   extern int i128AdjustCursorXPos;

   if (i128BlockCursor)
      return;
   
   x -= i128InfoRec.frameX0 - i128AdjustCursorXPos;
   if (x < 0)
      return;

   y -= i128InfoRec.frameY0;
   if (y < 0)
      return;

   /* Output position - "only" 12 bits of location documented */

   tmp = i128mem.rbase_g[INDEX_TI] & 0xFF;
   i128mem.rbase_g[INDEX_TI] = TI_CURS_X_LOW;				MB;
   i128mem.rbase_g[DATA_TI] = x & 0xFF;					MB;
   i128mem.rbase_g[INDEX_TI] = TI_CURS_X_HIGH;				MB;
   i128mem.rbase_g[DATA_TI] = (x >> 8) & 0x0F;				MB;
   i128mem.rbase_g[INDEX_TI] = TI_CURS_Y_LOW;				MB;
   i128mem.rbase_g[DATA_TI] = y & 0xFF;					MB;
   i128mem.rbase_g[INDEX_TI] = TI_CURS_Y_HIGH;				MB;
   i128mem.rbase_g[DATA_TI] = (y >> 8) & 0x0F;				MB;

   i128mem.rbase_g[INDEX_TI] = tmp;					MB;
   return;
}

void
i128TiRecolorCursor(pScr, pCurs, displayed)
     ScreenPtr pScr;
     CursorPtr pCurs;
     Bool displayed;
{
   CARD32 tmp;

   if (!xf86VTSema) {
      miRecolorCursor(pScr, pCurs, displayed);
      return;
   }

   if (!displayed)
      return;

   tmp = i128mem.rbase_g[INDEX_TI] & 0xFF;

   /* The TI 3020 cursor is always 8 bits so shift 8, not 10 */

   /* Background color */
   i128mem.rbase_g[INDEX_TI] = TI_CURSOR_COLOR_0_RED;			MB;
   i128mem.rbase_g[DATA_TI] = (pCurs->backRed >> 8) & 0xFF;		MB;
   i128mem.rbase_g[INDEX_TI] = TI_CURSOR_COLOR_0_GREEN;			MB;
   i128mem.rbase_g[DATA_TI] = (pCurs->backGreen >> 8) & 0xFF;		MB;
   i128mem.rbase_g[INDEX_TI] = TI_CURSOR_COLOR_0_BLUE;			MB;
   i128mem.rbase_g[DATA_TI] = (pCurs->backBlue >> 8) & 0xFF;		MB;

   /* Foreground color */
   i128mem.rbase_g[INDEX_TI] = TI_CURSOR_COLOR_1_RED;			MB;
   i128mem.rbase_g[DATA_TI] = (pCurs->foreRed >> 8) & 0xFF;		MB;
   i128mem.rbase_g[INDEX_TI] = TI_CURSOR_COLOR_1_GREEN;			MB;
   i128mem.rbase_g[DATA_TI] = (pCurs->foreGreen >> 8) & 0xFF;		MB;
   i128mem.rbase_g[INDEX_TI] = TI_CURSOR_COLOR_1_BLUE;			MB;
   i128mem.rbase_g[DATA_TI] = (pCurs->foreBlue >> 8) & 0xFF;		MB;

   return;
}

void 
i128TiLoadCursor(pScr, pCurs, x, y)
     ScreenPtr pScr;
     CursorPtr pCurs;
     int x, y;
{
   extern int i128hotX, i128hotY;
   int   index = pScr->myNum;
   register int   i;
   unsigned char *ram, *p;
   CARD32 tmp, tmp1, tmpcurs;
   extern int i128InitCursorFlag;

   if (!xf86VTSema)
      return;

   if (!pCurs)
      return;

   tmp = i128mem.rbase_g[INDEX_TI] & 0xFF;

   /* turn the cursor off */
   i128mem.rbase_g[INDEX_TI] = TI_CURS_CONTROL;				MB;
   if ((tmpcurs = i128mem.rbase_g[DATA_TI]) & TI_CURS_SPRITE_ENABLE)
      i128TiCursorOff();

   /* load colormap */
   i128TiRecolorCursor(pScr, pCurs, TRUE);

   ram = (unsigned char *)pCurs->bits->devPriv[index];

   i128BlockCursor = TRUE;

   i128mem.rbase_g[INDEX_TI] = TI_CURS_RAM_ADDR_LOW;			MB;
   i128mem.rbase_g[DATA_TI] = 0x00;					MB;
   i128mem.rbase_g[INDEX_TI] = TI_CURS_RAM_ADDR_HIGH;			MB;
   i128mem.rbase_g[DATA_TI] = 0x00;					MB;
   i128mem.rbase_g[INDEX_TI] = TI_CURS_RAM_DATA;			MB;

   /* 
    * Output the cursor data.  The realize function has put the planes into
    * their correct order, so we can just blast this out.
    */
   p = ram;
   for (i = 0; i < 1024; i++,p++) {
      i128mem.rbase_g[DATA_TI] = (CARD32 )*p;				MB;
   }

   if (i128hotX >= MAX_CURS_WIDTH)
      i128hotX = MAX_CURS_WIDTH - 1;
   else if (i128hotX < 0)
      i128hotX = 0;
   if (i128hotY >= MAX_CURS_HEIGHT)
      i128hotY = MAX_CURS_HEIGHT - 1;
   else if (i128hotY < 0)
      i128hotY = 0;

   i128mem.rbase_g[INDEX_TI] = TI_SPRITE_ORIGIN_X;			MB;
   i128mem.rbase_g[DATA_TI] = i128hotX;					MB;
   i128mem.rbase_g[INDEX_TI] = TI_SPRITE_ORIGIN_Y;			MB;
   i128mem.rbase_g[DATA_TI] = i128hotY;					MB;
   i128mem.rbase_g[INDEX_TI] = tmp;					MB;

   i128BlockCursor = FALSE;

   /* position cursor */
   i128TiMoveCursor(0, x, y);

   /* turn the cursor on */
   if ((tmpcurs & TI_CURS_SPRITE_ENABLE) || i128InitCursorFlag)
      i128TiCursorOn();

   if (i128InitCursorFlag)
      i128InitCursorFlag = FALSE;

   return;
}

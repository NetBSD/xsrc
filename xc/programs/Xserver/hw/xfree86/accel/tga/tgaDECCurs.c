/* $XConsortium: tgaBtCurs.c /main/4 1996/10/27 11:47:33 kaleb $ */
/*
 * Copyright 1993 by David Wexelblat <dwex@goblin.org>
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of David Wexelblat not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  David Wexelblat makes no representations
 * about the suitability of this software for any purpose.  It is provided
 * "as is" without express or implied warranty.
 *
 * DAVID WEXELBLAT DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL DAVID WEXELBLAT BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 *
 * Modified by Erik Nygren (nygren@mit.edu) for use with P9000
 *
 * Modified by Alan Hourihane (alanh@fairlite.demon.co.uk) for use with TGA
 *
 * Modified by Tim Rowley (tor@cs.brown.edu) for use with TGA/Bt463
 */

/* $XFree86: xc/programs/Xserver/hw/xfree86/accel/tga/tgaDECCurs.c,v 1.1.2.1 1998/10/19 20:33:32 hohndel Exp $ */

#define NEED_EVENTS
#include <X.h>
#include "Xproto.h"
#include <misc.h>
#include <input.h>
#include <cursorstr.h>
#include <regionstr.h>
#include <scrnintstr.h>
#include <servermd.h>
#include <windowstr.h>
#include "xf86.h"
#include "inputstr.h"
#include "xf86Priv.h"
#include "xf86_OSlib.h"
#include "tga.h"
#include "tga_regs.h"
#include "tga_presets.h"
#include "tgacurs.h"

/*
 * Convert the cursor from server-format to hardware-format.  The Bt485
 * has two planes, output sequentially.
 */
Bool
tgaDECRealizeCursor(pScr, pCurs)
     ScreenPtr pScr;
     CursorPtr pCurs;
{
   unsigned char tgaSpread[] = {0x00, 0x02, 0x08, 0x0a,
 			        0x20, 0x22, 0x28, 0x2a,
			        0x80, 0x82, 0x88, 0x8a,
			        0xa0, 0xa2, 0xa8, 0xaa};
   extern unsigned char tgaSwapBits[256];
   register int i, j;
   unsigned char *pServMsk;
   unsigned char *pServSrc;
   int   index = pScr->myNum;
   pointer *pPriv = &pCurs->bits->devPriv[index];
   int   wsrc, h;
   unsigned char *ram, *plane0, *plane1;
   CursorBitsPtr bits = pCurs->bits;

   if (pCurs->bits->refcnt > 1)
      return TRUE;

   ram = (unsigned char *)xalloc(1024);
   *pPriv = (pointer) ram;
   plane0 = ram;
   plane1 = ram+512;

   if (!ram)
      return FALSE;

   pServSrc = (unsigned char *)bits->source;
   pServMsk = (unsigned char *)bits->mask;

#define MAX_CURS 64

   h = bits->height;
   if (h > MAX_CURS)
      h = MAX_CURS;

   wsrc = PixmapBytePad(bits->width, 1);	/* bytes per line */

   for (i = 0; i < MAX_CURS; i++) {
      for (j = 0; j < MAX_CURS / 8; j++) {
	 unsigned char mask, source;

	 if (i < h && j < wsrc) {
	    source = *pServSrc++;
	    mask = *pServMsk++;
/*
            source = tgaSwapBits[source];
            mask = tgaSwapBits[mask];
*/
	    if (j < MAX_CURS / 8) {
	       *plane0++ = tgaSpread[source&0xf] | (tgaSpread[mask&0xf]>>1);
	       *plane0++ = tgaSpread[source>>4] | (tgaSpread[mask>>4]>>1);
	    }
	 } else {
	    *plane0++ = 0x00;
	    *plane0++ = 0x00;
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
tgaDECCursorOn()
{
   /* Enable cursor mode 3 - X11 mode */
  if (xf86VTSema)
    TGA_WRITE_REG(0x05, TGA_VALID_REG);
  fprintf(stderr, "on\n");
  return;
}

void
tgaDECCursorOff()
{
   /* Disable cursor */
  if (xf86VTSema)
    TGA_WRITE_REG(0x01, TGA_VALID_REG);
  fprintf(stderr, "off\n");
  return;
}

void
tgaDECMoveCursor(pScr, x, y)
     ScreenPtr pScr;
     int   x, y;
{
  extern int tgahotX, tgahotY;
  extern tgaCRTCRegRec tgaCRTCRegs;
  fprintf(stderr, "move in %d %d\n", x, y);

  if (!xf86VTSema) return;

   if (tgaBlockCursor)
      return;

/*   x += (tgaCRTCRegs.h_sync+tgaCRTCRegs.h_bporch)*4; */
/*   x += (tgaCRTCRegs.h_fporch+tgaCRTCRegs.h_sync)*4; */
/*   x -= tgaInfoRec.frameX0; */
/*   x += 64; */
/*   x -= tgahotX; */
   if (x < 0)
      return;

/*   x += (tgaCRTCRegs.v_sync+tgaCRTCRegs.v_bporch)*4; */
/*   y -= tgaInfoRec.frameY0; */
/*   y += 64; */
/*   y -= tgahotY; */
   if (y < 0)
      return;

   fprintf(stderr, "move at %d %d\n", x, y);
   TGA_WRITE_REG(x | (y << 12), TGA_CURSOR_XY_REG);

   return;
}

void
tgaDECRecolorCursor(pScr, pCurs, displayed)
     ScreenPtr pScr;
     CursorPtr pCurs;
{
   extern Bool tgaDAC8Bit;

   if (!xf86VTSema) {
	miRecolorCursor(pScr, pCurs, displayed);
	return;
   }

   if (!displayed)
	return;

   BT463_LOAD_ADDR(BT463_CUR_CLR_0);
   TGA_WRITE_REG(((pCurs->backRed>>8)&0xff)|(BT463_REG_ACC<<10), TGA_RAMDAC_REG);
   TGA_WRITE_REG(((pCurs->backGreen>>8)&0xff)|(BT463_REG_ACC<<10), TGA_RAMDAC_REG);
   TGA_WRITE_REG(((pCurs->backBlue>>8)&0xff)|(BT463_REG_ACC<<10), TGA_RAMDAC_REG);
   TGA_WRITE_REG(((pCurs->foreRed>>8)&0xff)|(BT463_REG_ACC<<10), TGA_RAMDAC_REG);
   TGA_WRITE_REG(((pCurs->foreGreen>>8)&0xff)|(BT463_REG_ACC<<10), TGA_RAMDAC_REG);
   TGA_WRITE_REG(((pCurs->foreBlue>>8)&0xff)|(BT463_REG_ACC<<10), TGA_RAMDAC_REG);
   TGA_WRITE_REG(0x00|(BT463_REG_ACC<<10), TGA_RAMDAC_REG);
   TGA_WRITE_REG(0x00|(BT463_REG_ACC<<10), TGA_RAMDAC_REG);
   TGA_WRITE_REG(0x00|(BT463_REG_ACC<<10), TGA_RAMDAC_REG);
   TGA_WRITE_REG(0x00|(BT463_REG_ACC<<10), TGA_RAMDAC_REG);
   TGA_WRITE_REG(0x00|(BT463_REG_ACC<<10), TGA_RAMDAC_REG);
   TGA_WRITE_REG(0x00|(BT463_REG_ACC<<10), TGA_RAMDAC_REG);

   return;
}

void 
tgaDECLoadCursor(pScr, pCurs, x, y)
     ScreenPtr pScr;
     CursorPtr pCurs;
     int x, y;
{
   int   index = pScr->myNum;
   register int   i, j;
   unsigned char *ram, tmpcurs;
   unsigned long *p;
   extern int tgaInitCursorFlag;

   if (!xf86VTSema)
      return;

   if (!pCurs)
      return;

   /* turn the cursor off */
   tgaDECCursorOff();

   /* load colormap */
   tgaBtRecolorCursor(pScr, pCurs, TRUE);

   ram = (unsigned char *)pCurs->bits->devPriv[index];

   BLOCK_CURSOR;

#if 0
      /* Start data output */
      BT485_WRITE(0x00, BT485_ADDR_PAL_WRITE);

      /* 
       * Output the cursor data.  The realize function has put the planes into
       * their correct order, so we can just blast this out.
       */
      p = ram;
      for (i = 0; i < 1024; i++,p++)
         BT485_WRITE(*p, BT485_CUR_RAM);

      tgaBtYPosMask = 0xFF;
#endif
      /* 
       * Output the cursor data.  The realize function has put the planes into
       * their correct order, so we can just blast this out.
       */
      p = (unsigned long *)ram;
      for (i = 0; i < 256; i++,p++)
        ((unsigned long *)tgaCursorMem)[i] = *p;
      TGA_WRITE_REG(63<<10, TGA_CURSOR_BASE_REG);

   UNBLOCK_CURSOR;

   /* position cursor */
   tgaDECMoveCursor(0, x, y);

   tgaDECCursorOn();

   if (tgaInitCursorFlag)
      tgaInitCursorFlag = FALSE;

   return;
}

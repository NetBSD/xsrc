
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
/* $XFree86: xc/programs/Xserver/hw/xfree86/drivers/i810/i810_cursor.c,v 1.2 2000/02/23 04:47:15 martin Exp $ */

/*
 * Authors:
 *   Keith Whitwell <keithw@precisioninsight.com>
 *
 */

#include "xf86.h"
#include "xf86_OSproc.h"
#include "xf86_ansic.h"
#include "compiler.h"

#include "xf86fbman.h"

#include "i810.h"

static void I810LoadCursorImage(ScrnInfoPtr pScrn, unsigned char *src);
static void I810ShowCursor(ScrnInfoPtr pScrn);
static void I810HideCursor(ScrnInfoPtr pScrn);
static void I810SetCursorColors(ScrnInfoPtr pScrn, int bg, int fb);
static Bool I810UseHWCursor(ScreenPtr pScrn, CursorPtr pCurs);



Bool
I810CursorInit(ScreenPtr pScreen)
{
  ScrnInfoPtr pScrn;
  I810Ptr pI810;
  xf86CursorInfoPtr infoPtr;

  pScrn = xf86Screens[pScreen->myNum];
  pI810 = I810PTR(pScrn);
  pI810->CursorInfoRec = infoPtr = xf86CreateCursorInfoRec();
  if (!infoPtr) return FALSE;

  infoPtr->MaxWidth = 64;
  infoPtr->MaxHeight = 64;
  infoPtr->Flags =  (HARDWARE_CURSOR_TRUECOLOR_AT_8BPP |
		     HARDWARE_CURSOR_BIT_ORDER_MSBFIRST |
		     HARDWARE_CURSOR_INVERT_MASK |
		     HARDWARE_CURSOR_SWAP_SOURCE_AND_MASK |
		     HARDWARE_CURSOR_AND_SOURCE_WITH_MASK |
		     HARDWARE_CURSOR_SOURCE_MASK_INTERLEAVE_64);

  infoPtr->SetCursorColors = I810SetCursorColors;
  infoPtr->SetCursorPosition = I810SetCursorPosition;
  infoPtr->LoadCursorImage = I810LoadCursorImage;
  infoPtr->HideCursor = I810HideCursor;
  infoPtr->ShowCursor = I810ShowCursor;
  infoPtr->UseHWCursor = I810UseHWCursor;

  if (!pI810->CursorPhysical)
     return FALSE;

  return xf86InitCursor(pScreen, infoPtr);
}

static Bool
I810UseHWCursor(ScreenPtr pScreen, CursorPtr pCurs) {
   ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
   I810Ptr pI810 = I810PTR(pScrn);

/*    if (pScrn->currentMode->Flags&V_DBLSCAN) */
/*      return FALSE; */

   if (!pI810->CursorPhysical) return FALSE;
   return TRUE;
}

static void
I810LoadCursorImage(ScrnInfoPtr pScrn, unsigned char *src) {
  I810Ptr pI810 = I810PTR(pScrn);
  CARD8 *pcurs = (CARD8 *)(pI810->FbBase + pI810->CursorStart);
  int x, y;
  
  for (y = 0; y < 64; y++) {
    for (x = 0; x < 64 / 4; x++) {
      *pcurs++ = *src++;
    }
  }
}

void
I810SetCursorPosition(ScrnInfoPtr pScrn, int x, int y) {
  I810Ptr pI810 = I810PTR(pScrn);
  int flag;

  if (I810_DEBUG & DEBUG_VERBOSE_CURSOR)
     ErrorF( "I810SetCursorPosition %d %d\n", x, y);

  x += pI810->CursorOffset;

  if (x >= 0) flag = CURSOR_X_POS;
  else {
    flag = CURSOR_X_NEG;
    x=-x;
  }

  OUTREG8( CURSOR_X_LO, x&0xFF);
  OUTREG8( CURSOR_X_HI, (((x >> 8) & 0x07) | flag));

  if (y >= 0) flag = CURSOR_Y_POS;
  else {
    flag = CURSOR_Y_NEG;
    y=-y;
  }
  OUTREG8( CURSOR_Y_LO, y&0xFF);
  OUTREG8( CURSOR_Y_HI, (((y >> 8) & 0x07) | flag));
}

static void
I810ShowCursor(ScrnInfoPtr pScrn) {
  I810Ptr pI810 = I810PTR(pScrn);
  unsigned char tmp;

  OUTREG( CURSOR_BASEADDR, pI810->CursorPhysical);
  OUTREG8( CURSOR_CONTROL, CURSOR_ORIGIN_DISPLAY | CURSOR_MODE_64_3C);

  tmp = INREG8( PIXPIPE_CONFIG_0 );
  tmp |= HW_CURSOR_ENABLE;
  OUTREG8( PIXPIPE_CONFIG_0, tmp);
}

static void
I810HideCursor(ScrnInfoPtr pScrn) {
  unsigned char tmp;
  I810Ptr pI810 = I810PTR(pScrn);

  tmp=INREG8( PIXPIPE_CONFIG_0 );
  tmp &= ~HW_CURSOR_ENABLE;
  OUTREG8( PIXPIPE_CONFIG_0, tmp);
}

static void
I810SetCursorColors(ScrnInfoPtr pScrn, int bg, int fg) {
  int tmp;
  I810Ptr pI810 = I810PTR(pScrn);

  tmp=INREG8(PIXPIPE_CONFIG_0);
  tmp |= EXTENDED_PALETTE;
  OUTREG8( PIXPIPE_CONFIG_0, tmp);

  pI810->writeStandard(pI810, DACMASK, 0xFF);
  pI810->writeStandard(pI810, DACWX, 0x04);

  pI810->writeStandard(pI810, DACDATA, (bg & 0x00FF0000) >> 16);
  pI810->writeStandard(pI810, DACDATA, (bg & 0x0000FF00) >> 8);
  pI810->writeStandard(pI810, DACDATA, (bg & 0x000000FF));

  pI810->writeStandard(pI810, DACDATA, (fg & 0x00FF0000) >> 16);
  pI810->writeStandard(pI810, DACDATA, (fg & 0x0000FF00) >> 8);
  pI810->writeStandard(pI810, DACDATA, (fg & 0x000000FF));

  tmp=INREG8( PIXPIPE_CONFIG_0 );
  tmp &= ~EXTENDED_PALETTE;
  OUTREG8( PIXPIPE_CONFIG_0, tmp );
}



/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/cyrix/cyrix_cursor.c,v 1.1.2.3 1998/11/06 09:47:07 hohndel Exp $ */
/*
 * Copyright 1998 by Annius V. Groenink (A.V.Groenink@zfc.nl, avg@cwi.nl).
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Annius Groenink not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.	Annius Groenink makes no representations
 * about the suitability of this software for any purpose.  It is provided
 * "as is" without express or implied warranty.
 *
 * ANNIUS GROENINK DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL DAVID WEXELBLAT BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */
/* $XConsortium: $ */

/*
 * Hardware cursor support for CYRIX MediaGX SVGA driver.
 */


#include "X.h"
#include "Xproto.h"
#include "misc.h"
#include "input.h"
#include "cursorstr.h"
#include "regionstr.h"
#include "scrnintstr.h"
#include "servermd.h"
#include "windowstr.h"

#include "compiler.h"
#include "vga256.h"
#include "xf86.h"
#include "mipointer.h"
#include "inputstr.h"
#include "xf86Priv.h"
#include "xf86_Option.h"
#include "xf86_OSlib.h"
#include "vga.h"
#include "xf86xaa.h"
#include "cyrix.h"

static Bool CYRIXRealizeCursor();
static Bool CYRIXUnrealizeCursor();
static void CYRIXSetCursor();
static void CYRIXMoveCursor();
static void CYRIXRecolorCursor();

static miPointerSpriteFuncRec cyrixPointerSpriteFuncs =
{
   CYRIXRealizeCursor,
   CYRIXUnrealizeCursor,
   CYRIXSetCursor,
   CYRIXMoveCursor,
};

extern miPointerScreenFuncRec xf86PointerScreenFuncs;
extern xf86InfoRec xf86Info;

/* For byte swapping, use the XAA array */
extern unsigned char byte_reversed[256];

static int cyrixCursGeneration = -1;
static int cyrixHotX;
static int cyrixHotY;
static int cyrixCursorVRAMMemSegment;
static CursorPtr cyrixCursorpCurs;

#define MAX_CURS 32

Bool
CYRIXCursorInit(pm, pScr)
     char *pm;
     ScreenPtr pScr;
{


   if (cyrixCursGeneration != serverGeneration) {
      if (!(miPointerInitialize(pScr, &cyrixPointerSpriteFuncs,
	   &xf86PointerScreenFuncs, FALSE)))
               return FALSE;

      cyrixHotX = 0;
      cyrixHotY = 0;
      pScr->RecolorCursor = CYRIXRecolorCursor;
      cyrixCursGeneration = serverGeneration;
   }
   cyrixCursorVRAMMemSegment = vga256InfoRec.videoRam - 1;
   return TRUE;
}

/* This allows the cursor to be displayed */

void
CYRIXShowCursor()
{
	/* turn cursor on */
	GX_REG(DC_UNLOCK) = DC_UNLOCK_VALUE;
	GX_REG(DC_GENERAL_CFG) |= DC_GCFG_CURE;
	GX_REG(DC_UNLOCK) = 0;
}

void
CYRIXHideCursor()
{
	/* turn cursor off */
	GX_REG(DC_UNLOCK) = DC_UNLOCK_VALUE;
	GX_REG(DC_GENERAL_CFG) &= (~DC_GCFG_CURE);
	GX_REG(DC_UNLOCK) = 0;
}

static Bool
CYRIXRealizeCursor(pScr, pCurs)
     ScreenPtr pScr;
     CursorPtr pCurs;

{
   register int i, j;
   unsigned short *pServMsk;
   unsigned short *pServSrc;
   int   index = pScr->myNum;
   pointer *pPriv = &pCurs->bits->devPriv[index];
   int   wsrc, h;
   unsigned short *ram;
   CursorBitsPtr bits = pCurs->bits;

   if (pCurs->bits->refcnt > 1)
      return TRUE;

   ram = (unsigned short *)xalloc((MAX_CURS * MAX_CURS) / 4);
   *pPriv = (pointer) ram;

   if (!ram)
      return FALSE;

   pServSrc = (unsigned short *)bits->source;
   pServMsk = (unsigned short *)bits->mask;

   h = bits->height;
   if (h > MAX_CURS)
      h = MAX_CURS;

   wsrc = PixmapBytePad(bits->width, 1);	/* bytes per line */

   for (i = 0; i < MAX_CURS; i++) {
      for (j = 0; j < MAX_CURS / 16; j++) {
	 unsigned short smask, ssource, mask, source;

	 if (i < h && j < wsrc / 2) {
	    smask = *pServMsk++;
	    ssource = *pServSrc++;

	    ((char *)&mask)[0] = byte_reversed[((unsigned char *)&smask)[1]];
	    ((char *)&mask)[1] = byte_reversed[((unsigned char *)&smask)[0]];

	    ((char *)&source)[0] = byte_reversed[((unsigned char *)&ssource)[1]];
	    ((char *)&source)[1] = byte_reversed[((unsigned char *)&ssource)[0]];

	       *ram++ = (source & mask);
	       *ram++ = ~mask;
	 } else {
	    *ram++ = 0x0;
	    *ram++ = 0xffff;
	 }
      }
      if (j < wsrc / 2) {
	 pServMsk += (wsrc/2 - j);
	 pServSrc += (wsrc/2 - j);
      }
   }

   return TRUE;
}

static Bool
CYRIXUnrealizeCursor(pScr, pCurs)
     ScreenPtr pScr;
     CursorPtr pCurs;
{
   pointer priv;


   if (pCurs->bits->refcnt <= 1 &&
     (priv = pCurs->bits->devPriv[pScr->myNum])) {
         xfree(priv);
         pCurs->bits->devPriv[pScr->myNum] = 0x0;
   }
   return TRUE;
}

static void
CYRIXLoadCursor(pScr, pCurs, x, y)
     ScreenPtr pScr;
     CursorPtr pCurs;
     int x, y;
{
   int   index = pScr->myNum;

   if (!xf86VTSema)
      return;

   if (!pCurs)
      return;
 
   /* Remember which cursor is loaded */
   cyrixCursorpCurs = pCurs;

   /* Wait for vertical retrace */
   /* VerticalRetraceWait(); */

   /* turn cursor off */
   CYRIXHideCursor();

	memcpy((unsigned char *)vgaLinearBase + CYRIXcursorAddress,
		pCurs->bits->devPriv[index], 256);

   /* Wait for vertical retrace */
   /* VerticalRetraceWait(); */

   /* position cursor */
   CYRIXMoveCursor(0, x, y);
   CYRIXRecolorCursor(pScr, pCurs, TRUE);

   /* turn cursor on */
   CYRIXShowCursor();
}

static void
CYRIXSetCursor(pScr, pCurs, x, y, generateEvent)
     ScreenPtr pScr;
     CursorPtr pCurs;
     int   x, y;
     Bool  generateEvent;

{
   int index = pScr->myNum;

   if (!pCurs)
      return;

   cyrixHotX = pCurs->bits->xhot;
   cyrixHotY = pCurs->bits->yhot;
   CYRIXLoadCursor(pScr, pCurs, x, y);

}

void
CYRIXRestoreCursor(pScr)
     ScreenPtr pScr;
{
   int index = pScr->myNum;
   int x, y;

   miPointerPosition(&x, &y);
   CYRIXLoadCursor(pScr, cyrixCursorpCurs, x, y);
}

void
CYRIXRepositionCursor(pScr)
     ScreenPtr pScr;
{
   int x, y;


   miPointerPosition(&x, &y);
   /* Wait for vertical retrace */
   /* VerticalRetraceWait(); */
   CYRIXMoveCursor(pScr, x, y);
}

static void
CYRIXMoveCursor(pScr, x, y)
     ScreenPtr pScr;
     int   x, y;
{
	int xorigin, yorigin;

	if (!xf86VTSema)
		return;

	x -= vga256InfoRec.frameX0 + cyrixHotX;
	y -= vga256InfoRec.frameY0 + cyrixHotY;

	/*
	 * If the cursor is partly out of screen at the left or top,
	 * we need set the origin.
	 */
	xorigin = 0;
	yorigin = 0;
	if (x < 0) {
		xorigin = -x;
		x = 0;
	}
	if (y < 0) {
		yorigin = -y;
		y = 0;
	}

#if 0
	if (XF86SCRNINFO(pScr)->modes->Flags & V_DBLSCAN)
		y *= 2;
#endif

	GX_REG(DC_UNLOCK) = DC_UNLOCK_VALUE;
	GX_REG(DC_CURSOR_X) = (xorigin << 11) | x;
	GX_REG(DC_CURSOR_Y) = (yorigin << 11) | y;
	GX_REG(DC_CURS_ST_OFFSET) = CYRIXcursorAddress + 8 * yorigin;
	GX_REG(DC_UNLOCK) = 0;
}


static void
CYRIXRecolorCursor(pScr, pCurs, displayed)
     ScreenPtr pScr;
     CursorPtr pCurs;
     Bool displayed;
{
   ColormapPtr pmap;
   unsigned short packedcolfg, packedcolbg;
   xColorItem sourceColor, maskColor;

   if (!xf86VTSema)
      return;

   if (!displayed)
      return;

	if (vgaBitsPerPixel == 8 && CYRIXisOldChipRevision)
	{	/* this code has effect only if there is an external RAMDAC.
		   It generates a fatal signal on newer chipsets
		   (that have a 5520 or 5530) */
		vgaGetInstalledColormaps(pScr, &pmap);
		sourceColor.red = pCurs->foreRed;
		sourceColor.green = pCurs->foreGreen;
		sourceColor.blue = pCurs->foreBlue;
		FakeAllocColor(pmap, &sourceColor);
		maskColor.red = pCurs->backRed;
		maskColor.green = pCurs->backGreen;
		maskColor.blue = pCurs->backBlue;
		FakeAllocColor(pmap, &maskColor);
		FakeFreeColor(pmap, sourceColor.pixel);
		FakeFreeColor(pmap, maskColor.pixel);

		GX_REG(DC_UNLOCK) = DC_UNLOCK_VALUE;
		GX_REG(DC_CURSOR_COLOR) = maskColor.pixel
		                        | (sourceColor.pixel << 8);
		GX_REG(DC_UNLOCK) = 0;
	}
	
	/* write directly into the reserved color indices 100h, 101h -
	   this code has no effect in 8bpp with external RAMDAC, but
	   is harmless in that case.  We also set the overscan color
	   to black here, which is needed on some older systems to
	   get 16bpp right */
	GX_REG(DC_UNLOCK) = DC_UNLOCK_VALUE;
	GX_REG(DC_PAL_ADDRESS) = 0x100; /* cursor color */
	GX_REG(DC_PAL_DATA) = ((pCurs->backRed << 2) & 0x0003F000)
	                    | ((pCurs->backGreen >> 4) & 0x00000FC0)
	                    | ((pCurs->backBlue >> 10) & 0x0000003F);
	GX_REG(DC_PAL_DATA) = ((pCurs->foreRed << 2) & 0x0003F000)
	                    | ((pCurs->foreGreen >> 4) & 0x00000FC0)
	                    | ((pCurs->foreBlue >> 10) & 0x0000003F);
	GX_REG(DC_PAL_ADDRESS) = 0x104; /* overscan color */
	GX_REG(DC_PAL_DATA) = 0;
	GX_REG(DC_UNLOCK) = 0;
}

void
CYRIXWarpCursor(pScr, x, y)
     ScreenPtr pScr;
     int   x, y;
{
   miPointerWarpCursor(pScr, x, y);
   xf86Info.currentScreen = pScr;
}

void
CYRIXQueryBestSize(class, pwidth, pheight, pScreen)
     int class;
     unsigned short *pwidth;
     unsigned short *pheight;
     ScreenPtr pScreen;
{
   if (*pwidth > 0) {
      switch (class) {
         case CursorShape:
	    if (*pwidth > MAX_CURS)
	       *pwidth = MAX_CURS;
	    if (*pheight > MAX_CURS)
	       *pheight = MAX_CURS;
	    break;
         default:
	    mfbQueryBestSize(class, pwidth, pheight, pScreen);
	    break;
      }
   }
}

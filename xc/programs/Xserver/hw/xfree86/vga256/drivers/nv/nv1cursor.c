/*
 * Copyright 1996-1997  David J. McKay
 *
 * Permission is hereby granted, free of charge, to any person obtaining a 
 * copy of this software and associated documentation files (the "Software"), 
 * to deal in the Software without restriction, including without limitation 
 * the rights to use, copy, modify, merge, publish, distribute, sublicense, 
 * and/or sell copies of the Software, and to permit persons to whom the 
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL 
 * DAVID J. MCKAY BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, 
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF 
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE 
 * SOFTWARE.
 */

/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/nv/nv1cursor.c,v 1.1.2.4 1998/12/22 07:37:45 hohndel Exp $ */

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
#include "xf86Priv.h"
#include "xf86_Option.h"
#include "xf86_OSlib.h"
#include "vga.h"

#include "miline.h"
#include "nv1ref.h"
#include "nvreg.h"
#include "nvcursor.h"


static Bool NVRealizeCursor();
static Bool NVUnrealizeCursor();
static void NVSetCursor();
static void NVMoveCursor();
static void NVRecolorCursor();

static miPointerSpriteFuncRec NVPointerSpriteFuncs =
{
  NVRealizeCursor,
  NVUnrealizeCursor,
  NVSetCursor,
  NVMoveCursor,
};

/* vga256 interface defines Init, Restore, Warp, QueryBestSize. */


extern miPointerScreenFuncRec xf86PointerScreenFuncs;
extern xf86InfoRec xf86Info;

static int NVCursorGeneration = -1;

/*
 * This is the set variables that defines the cursor state within the
 * driver.
 */

static int NVCursorHotX;
static int NVCursorHotY;
static CursorPtr NVCursorpCurs;


/*
 * This is a high-level init function, called once; it passes a local
 * miPointerSpriteFuncRec with additional functions that we need to provide.
 * It is called by the SVGA server.
 */

Bool NVCursorInit(char *pm,ScreenPtr pScr)
{
  unsigned char power;

  NVCursorHotX = 0;
  NVCursorHotY = 0;


  if(NVCursorGeneration != serverGeneration) {
    if(!(miPointerInitialize(pScr, &NVPointerSpriteFuncs,
			     &xf86PointerScreenFuncs, FALSE))) {
      return FALSE;
    }
    pScr->RecolorCursor = NVRecolorCursor;
    NVCursorGeneration = serverGeneration;
  }
  return TRUE;
}

/*
 * This enables displaying of the cursor by the NV graphics chip.
 * It's a local function, it's not called from outside of the module.
 */

static void NVShowCursor(void)
{
  PDAC_WriteExt(CURSOR_CTRL_A,NV_PDAC_CURSOR_CTRL_A_XWIN);
}

/*
 * This disables displaying of the cursor by the NV graphics chip.
 * This is also a local function, it's not called from outside.
 */
void NVHideCursor(void)
{
  PDAC_WriteExt(CURSOR_CTRL_A,NV_PDAC_CURSOR_CTRL_A_OFF);
}

/*
 * This function is called when a new cursor image is requested by
 * the server. The main thing to do is convert the bitwise image
 * provided by the server into a format that the graphics card
 * can conveniently handle, and store that in system memory.
 * Adapted from accel/s3/s3Cursor.c.
 */

static Bool NVRealizeCursor(ScreenPtr pScr,CursorPtr pCurs)
{
  register int i, j;
  unsigned char *pServMsk;
  unsigned char *pServSrc;
  int index = pScr->myNum;
  pointer *pPriv = &pCurs->bits->devPriv[index];
  int wsrc, h;
  unsigned char *ram;
  CursorBits *bits = pCurs->bits;

  /* Presumably this checks to see if this is already the cursor in there */
  if(pCurs->bits->refcnt > 1) return TRUE;

  /* What do we need ram for ???? */
  ram = (unsigned char *)xalloc(1024);
  *pPriv = (pointer) ram;
  if(!ram) return FALSE;

  pServSrc = (unsigned char *)bits->source;
  pServMsk = (unsigned char *)bits->mask;

#define MAX_CURS 32

  h = bits->height;
  if(h > MAX_CURS) h = MAX_CURS;

  wsrc = PixmapBytePad(bits->width, 1);		/* Bytes per line. */

  for (i = 0; i < MAX_CURS; i++) {
    for (j = 0; j < MAX_CURS / 8; j++) {
      unsigned char mask, source;

      if(i < h && j < wsrc) {
	mask = *pServMsk++;
	source = *pServSrc++;
#if 0
        /* We don't need to do this on the NV1, as it supports bitmaps
         * in the "right" order 
         */
	mask = byte_reversed[mask];
	source = byte_reversed[source];
#endif
	if(j < MAX_CURS / 8) {	/* ??? */
	  *ram++ = mask;
	  *ram++ = source;
	}
      } else {
	*ram++ = 0x00;
	*ram++ = 0xFF;
      }
    }
    /*
     * if we still have more bytes on this line (j < wsrc),
     * we have to ignore the rest of the line.
     */
    while (j++ < wsrc)
      pServMsk++, pServSrc++;
  }
  return TRUE;
}

/*
 * This is called when a cursor is no longer used. The intermediate
 * cursor image storage that we created needs to be deallocated.
 */

static Bool NVUnrealizeCursor(ScreenPtr pScr,CursorPtr pCurs)
{
  pointer priv;

  if(pCurs->bits->refcnt <= 1 &&
      (priv = pCurs->bits->devPriv[pScr->myNum])) {
    xfree(priv);
    pCurs->bits->devPriv[pScr->myNum] = 0x0;
  }
  return TRUE;
}

/*
 * This function uploads a cursor image to the video memory of the
 * graphics card. The source image has already been converted by the
 * Realize function to a format that can be quickly transferred to
 * the card.
 * This is a local function that is not called from outside of this
 * module.
 */

extern void NVSetWrite();

static void NVLoadCursorToCard(ScreenPtr pScr,CursorPtr pCurs,int x,int y)
{
  unsigned char *cursor_image,*p;
  int index = pScr->myNum;
  int i;

  if(!xf86VTSema)
    return;

  p=cursor_image = pCurs->bits->devPriv[index];
  /* Upload the cursor to the card */
  for(i=0;i<MAX_CURS*4;i++) {
    PDAC_WriteExt(CURSOR_PLANE_1+i,*p++);
    PDAC_WriteExt(CURSOR_PLANE_0+i,*p++);
  }

}

/*
 * This function should make the graphics chip display new cursor that
 * has already been "realized". We need to upload it to video memory,
 * make the graphics chip display it.
 * This is a local function that is not called from outside of this
 * module (although it largely corresponds to what the SetCursor
 * function in the Pointer record needs to do).
 */

static void NVLoadCursor(ScreenPtr pScr,CursorPtr pCurs,int x,int y)
{
  if(!xf86VTSema)
    return;

  if(!pCurs)
    return;

  /* Remember the cursor currently loaded into this cursor slot. */
  NVCursorpCurs = pCurs;

  NVHideCursor();

  NVLoadCursorToCard(pScr, pCurs, x, y);

  NVRecolorCursor(pScr, pCurs, 1);

  /* Position cursor */
  NVMoveCursor(pScr, x, y);

  /* Turn it on. */
  NVShowCursor();
}

/*
 * This function should display a new cursor at a new position.
 */

static void NVSetCursor(ScreenPtr pScr,CursorPtr pCurs,int x,int y,
                        Bool generateEvent)
{
  if(!pCurs)
    return;

  NVCursorHotX = pCurs->bits->xhot;
  NVCursorHotY = pCurs->bits->yhot;

  NVLoadCursor(pScr, pCurs, x, y);
}

/*
 * This function should redisplay a cursor that has been
 * displayed earlier. It is called by the SVGA server.
 */

void NVRestoreCursor(ScreenPtr pScr)
{
  int x, y;

  miPointerPosition(&x, &y);

  NVLoadCursor(pScr, NVCursorpCurs, x, y);
}

/*
 * This function is called when the current cursor is moved. It makes
 * the graphic chip display the cursor at the new position.
 */

int nvMiLineZeroBias=OCTANT4 | OCTANT3 | OCTANT1 | OCTANT6;

static void NVMoveCursor(ScreenPtr pScr,int x,int y)
{
  int xorigin, yorigin;

  if(!xf86VTSema) return;


  x -= vga256InfoRec.frameX0 + NVCursorHotX;
  y -= vga256InfoRec.frameY0 + NVCursorHotY;

  PDAC_WriteExt(CURSOR_X_POS_LO,x & 0xff);
  PDAC_WriteExt(CURSOR_X_POS_HI,(x >>8) & 0xff);

  PDAC_WriteExt(CURSOR_Y_POS_LO,y & 0xff);
  PDAC_WriteExt(CURSOR_Y_POS_HI,(y >>8) & 0xff);
  
}

/*
 * This is a local function that programs the colors of the cursor
 * on the graphics chip.
 * Adapted from accel/s3/s3Cursor.c.
 */

static void NVRecolorCursor(ScreenPtr pScr,CursorPtr pCurs,Bool displayed)
{
  ColormapPtr pmap;
  unsigned short packedcolfg, packedcolbg;
  xColorItem sourceColor, maskColor;

  if(!xf86VTSema) return;

  if(!displayed) return;

  PDAC_WriteExt(CURSOR_COLOUR_3_RED,pCurs->foreRed >> 8);
  PDAC_WriteExt(CURSOR_COLOUR_3_GREEN,pCurs->foreGreen >> 8);
  PDAC_WriteExt(CURSOR_COLOUR_3_BLUE,pCurs->foreBlue >> 8); 

  PDAC_WriteExt(CURSOR_COLOUR_2_RED,pCurs->backRed >> 8);
  PDAC_WriteExt(CURSOR_COLOUR_2_GREEN,pCurs->backGreen >> 8);
  PDAC_WriteExt(CURSOR_COLOUR_2_BLUE,pCurs->backBlue >> 8); 

}

/*
 * This doesn't do very much. It just calls the mi routine. It is called
 * by the SVGA server.
 */

void NVWarpCursor(ScreenPtr pScr,int x,int y)
{
  miPointerWarpCursor(pScr, x, y);
  xf86Info.currentScreen = pScr;
}

/*
 * This function is called by the SVGA server. It returns the
 * size of the hardware cursor that we support when asked for.
 * It is called by the SVGA server.
 */



void NVQueryBestSize(int class,unsigned short *pwidth, 
                     unsigned short *pheight,ScreenPtr pScreen)
{
#if 0

  /*  miSetZeroLineBias(pScreen,OCTANT4 | OCTANT3 | OCTANT1 | OCTANT6);*/
  miSetZeroLineBias(pScreen,nvMiLineZeroBias);
  ErrorF("Zero Bias is 0x%02x\n",nvMiLineZeroBias);
#endif
  
  if(*pwidth > 0) {
    if(class == CursorShape) {
      *pwidth = MAX_CURS;
      *pheight = MAX_CURS;
    } else
      (void)mfbQueryBestSize(class, pwidth, pheight, pScreen);
  }
}

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

/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/nv/nv3cursor.c,v 1.1.2.3 1998/02/08 01:12:44 dawes Exp $ */

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

#include "nv3ref.h"
#include "nvreg.h"
#include "nvcursor.h"


static Bool NV3RealizeCursor();
static Bool NV3UnrealizeCursor();
static void NV3SetCursor();
static void NV3MoveCursor();
static void NV3RecolorCursor();

static miPointerSpriteFuncRec NV3PointerSpriteFuncs =
{
  NV3RealizeCursor,
  NV3UnrealizeCursor,
  NV3SetCursor,
  NV3MoveCursor,
};

/* vga256 interface defines Init, Restore, Warp, QueryBestSize. */


extern miPointerScreenFuncRec xf86PointerScreenFuncs;
extern xf86InfoRec xf86Info;

static int NV3CursorGeneration = -1;

/*
 * This is the set variables that defines the cursor state within the
 * driver.
 */

static int NV3CursorHotX;
static int NV3CursorHotY;
static CursorPtr NV3CursorpCurs;


/*
 * This is a high-level init function, called once; it passes a local
 * miPointerSpriteFuncRec with additional functions that we need to provide.
 * It is called by the SVGA server.
 */

Bool NV3CursorInit(char *pm,ScreenPtr pScr)
{
  unsigned char power;

  NV3CursorHotX = 0;
  NV3CursorHotY = 0;


  if(NV3CursorGeneration != serverGeneration) {
    if(!(miPointerInitialize(pScr, &NV3PointerSpriteFuncs,
			     &xf86PointerScreenFuncs, FALSE))) {
      return FALSE;
    }
    pScr->RecolorCursor = NV3RecolorCursor;
    NV3CursorGeneration = serverGeneration;
  }
  return TRUE;
}

/*
 * This enables displaying of the cursor by the NV3 graphics chip.
 * It's a local function, it's not called from outside of the module.
 */

static void NV3ShowCursor(void)
{
  unsigned char tmp;
  tmp = PCRTC_Read(GRCURSOR1)|1|PCRTC_Def(GRCURSOR1_CURSOR,ENABLE);
  PCRTC_Write(GRCURSOR1,tmp);
}

/*
 * This disables displaying of the cursor by the NV3 graphics chip.
 * This is also a local function, it's not called from outside.
 */
void NV3HideCursor(void)
{
  unsigned char tmp;
  tmp = PCRTC_Read(GRCURSOR1)&(~PCRTC_Def(GRCURSOR1_CURSOR,ENABLE));
  PCRTC_Write(GRCURSOR1,tmp);
}

/*
 * This function is called when a new cursor image is requested by
 * the server. The main thing to do is convert the bitwise image
 * provided by the server into a format that the graphics card
 * can conveniently handle, and store that in system memory.
 * Adapted from accel/s3/s3Cursor.c.
 */


/* The NV3 supports true colour cursors, useless for X but essential
 * for those animated cursors under Win95!
 */
#define MAX_CURS 32
#define TRANSPARENT_PIXEL 0
#define SHOW
typedef struct {
  unsigned short foreColour, /* Colour for this cursor in RGB555 */
                 backColour;
  unsigned short image[MAX_CURS*MAX_CURS]; /* Image */
  int address; /* Pramin where image loaded */
}NV3Cursor;
  
  
static unsigned short ConvertToRGB555(int red,int green,int blue)
{
  unsigned short colour;

  colour=((red>>11)&0x1f)<<10;
  colour|=((green>>11)&0x1f)<<5;
  colour|=((blue>>11)&0x1f);
  colour|=1<<15; /* We must set the top bit, else it appears transparent */

  return colour;
}

static void RenderCursor(CursorBits *bits,NV3Cursor *cursor)
{
  int x,y,i;
  int byteIndex,bitIndex,maskBit,sourceBit;
  int height,width;
  int pad,lineOffset;
  unsigned char *source,*mask;

  height = bits->height;
  width=bits->width;
  source=(unsigned char*)bits->source;
  pad=PixmapBytePad(bits->width, 1);/* Bytes per line. */
  mask=(unsigned char*)bits->mask;
  for(y=0,i=0,lineOffset=0;y<MAX_CURS;y++,lineOffset+=pad) {
    for(x=0;x<MAX_CURS;x++,i++) {
      if(x<width && y<height) {
        byteIndex=lineOffset+(x/8);bitIndex=x%8;
        maskBit=mask[byteIndex]&(1<<bitIndex);
        sourceBit=source[byteIndex]&(1<<bitIndex);
        if(maskBit) {
	  cursor->image[i]=(sourceBit) ? cursor->foreColour :
                                         cursor->backColour;
	}else {
          cursor->image[i]=TRANSPARENT_PIXEL;
	}        
      }else {
        cursor->image[i]=TRANSPARENT_PIXEL;
      }
    }
  }
}

static Bool NV3RealizeCursor(ScreenPtr pScr,CursorPtr pCurs)
{
  NV3Cursor *ram;
  int index = pScr->myNum;
  pointer *pPriv = &pCurs->bits->devPriv[index];
  CursorBits *bits = pCurs->bits;

  /* Presumably this checks to see if this is already the cursor in there */
  if(pCurs->bits->refcnt > 1) return TRUE;

  ram = (NV3Cursor*)xalloc(sizeof(NV3Cursor));
  *pPriv = (pointer) ram;
  if(!ram) return FALSE;

  ram->foreColour=
     ConvertToRGB555(pCurs->foreRed,pCurs->foreGreen,pCurs->foreBlue);
  ram->backColour=
     ConvertToRGB555(pCurs->backRed,pCurs->backGreen,pCurs->backBlue);
  
  RenderCursor(bits,ram);

  return TRUE;
}

/*
 * This is called when a cursor is no longer used. The intermediate
 * cursor image storage that we created needs to be deallocated.
 */

static Bool NV3UnrealizeCursor(ScreenPtr pScr,CursorPtr pCurs)
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

#define PRAMINRead nvPRAMINPort(addr) nvPRAMINPort[addr]
#define PRAMINWrite(addr,val) nvPRAMINPort[addr]=(val)

#define CURSOR_ADDRESS ((8192-2048)/4)

#define SetBitField(value,from,to) SetBF(to,GetBF(value,from))
#define SetBit(n) (1<<(n))
#define Set8Bits(value) ((value)&0xff)

static void NV3LoadCursorToCard(ScreenPtr pScr,CursorPtr pCurs)
{
  NV3Cursor *cursor;
  int index = pScr->myNum;
  int i;
  int numInts;
  int *image;
  int save;

  if(!xf86VTSema)
    return;

  cursor=(NV3Cursor*) pCurs->bits->devPriv[index];
  numInts=sizeof(cursor->image)/sizeof(int);
  image=(int*)cursor->image;

  save=PCRTC_Read(GRCURSOR1);

  PCRTC_Write(GRCURSOR1,0);

  /* Upload the cursor to the card */
  for(i=0;i<numInts;i++) {
    PRAMINWrite(CURSOR_ADDRESS+i,image[i]);
  }
  /* Tell the ramdac where we are */
  PCRTC_Write(GRCURSOR0,SetBitField(CURSOR_ADDRESS*4,21:16,5:0));
  save&=0x7;
  PCRTC_Write(GRCURSOR1,save|SetBitField(CURSOR_ADDRESS*4,15:11,7:3));
  

}

/*
 * This function should make the graphics chip display new cursor that
 * has already been "realized". We need to upload it to video memory,
 * make the graphics chip display it.
 * This is a local function that is not called from outside of this
 * module (although it largely corresponds to what the SetCursor
 * function in the Pointer record needs to do).
 */

static void NV3LoadCursor(ScreenPtr pScr,CursorPtr pCurs,int x,int y)
{
 
  if(!xf86VTSema)
    return;

  if(!pCurs)
    return;

  /* Remember the cursor currently loaded into this cursor slot. */
  NV3CursorpCurs = pCurs;

  NV3HideCursor();

  NV3LoadCursorToCard(pScr, pCurs);

  /* Position cursor */
  NV3MoveCursor(pScr, x, y);

  /* Turn it on. */
  NV3ShowCursor();
}

/*
 * This function should display a new cursor at a new position.
 */

static void NV3SetCursor(ScreenPtr pScr,CursorPtr pCurs,int x,int y,
                        Bool generateEvent)
{
  if(!pCurs)
    return;

  NV3CursorHotX = pCurs->bits->xhot;
  NV3CursorHotY = pCurs->bits->yhot;

  NV3LoadCursor(pScr, pCurs, x, y);
}

/*
 * This function should redisplay a cursor that has been
 * displayed earlier. It is called by the SVGA server.
 */

void NV3RestoreCursor(ScreenPtr pScr)
{
  int x, y;

  miPointerPosition(&x, &y);

  NV3LoadCursor(pScr, NV3CursorpCurs, x, y);
}

/*
 * This function is called when the current cursor is moved. It makes
 * the graphic chip display the cursor at the new position.
 */

static void NV3MoveCursor(ScreenPtr pScr,int x,int y)
{
  int xorigin, yorigin;

  if(!xf86VTSema) return;

  x -= vga256InfoRec.frameX0 + NV3CursorHotX;
  y -= vga256InfoRec.frameY0 + NV3CursorHotY;
  x&=PRAMDAC_Mask(GRCURSOR_START_POS_X);
  PRAMDAC_Write(GRCURSOR_START_POS,PRAMDAC_Val(GRCURSOR_START_POS_X,x)|
                                   PRAMDAC_Val(GRCURSOR_START_POS_Y,y));
}

/*
 * This is a local function that programs the colors of the cursor
 * on the graphics chip.
 * Adapted from accel/s3/s3Cursor.c.
 */

static void NV3RecolorCursor(ScreenPtr pScr,CursorPtr pCurs,Bool displayed)
{
  CursorBits *bits = pCurs->bits;
  int index = pScr->myNum;
  NV3Cursor *cursor=(NV3Cursor*) pCurs->bits->devPriv[index];
  unsigned short fore,back;

  if(!xf86VTSema) return;

  if(!displayed) return;

  fore=ConvertToRGB555(pCurs->foreRed,pCurs->foreGreen,pCurs->foreBlue);
  back=ConvertToRGB555(pCurs->backRed,pCurs->backGreen,pCurs->backBlue);

  if(cursor->foreColour==fore && cursor->backColour==back) return;
  
  NV3LoadCursorToCard(pScr,pCurs);

}

/*
 * This doesn't do very much. It just calls the mi routine. It is called
 * by the SVGA server.
 */

void NV3WarpCursor(ScreenPtr pScr,int x,int y)
{
  miPointerWarpCursor(pScr, x, y);
  xf86Info.currentScreen = pScr;
}

/*
 * This function is called by the SVGA server. It returns the
 * size of the hardware cursor that we support when asked for.
 * It is called by the SVGA server.
 */

void NV3QueryBestSize(int class,unsigned short *pwidth, 
                     unsigned short *pheight,ScreenPtr pScreen)
{
  if(*pwidth > 0) {
    if(class == CursorShape) {
      *pwidth = MAX_CURS;
      *pheight = MAX_CURS;
    } else
      (void)mfbQueryBestSize(class, pwidth, pheight, pScreen);
  }
}


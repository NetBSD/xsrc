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

/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/nv/nv3cursor.c,v 1.1.2.5 1998/11/18 16:38:44 hohndel Exp $ */

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
#include "xf86.h"
#include "mipointer.h"
#include "xf86Priv.h"
#include "xf86_Option.h"
#include "xf86_OSlib.h"
#include "vga256.h"
#include "vga.h"

#include "miline.h"

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

 


extern miPointerScreenFuncRec xf86PointerScreenFuncs;
extern xf86InfoRec xf86Info;

static int NV3CursorGeneration = -1;

 




static int NV3CursorHotX;
static int NV3CursorHotY;
static CursorPtr NV3CursorpCurs;


 





Bool NV3CursorInit(char *pm,ScreenPtr pScr)
{
  unsigned char power;

  NV3CursorHotX = 0;
  NV3CursorHotY = 0;


  if(NV3CursorGeneration != serverGeneration) {
    if(!(miPointerInitialize(pScr, &NV3PointerSpriteFuncs,
			     &xf86PointerScreenFuncs, 0 ))) {
      return 0 ;
    }
    pScr->RecolorCursor = NV3RecolorCursor;
    NV3CursorGeneration = serverGeneration;
  }
  return 1 ;
}

 




static void NV3ShowCursor(void)
{
  unsigned char tmp;
  tmp = (outb(980, 49   ),inb(981))  |1| (( 1    ) << (0))   ;
  outb(980,( 49   ));outb(981,  tmp  )  ;
}

 



void NV3HideCursor(void)
{
  unsigned char tmp;
  tmp = (outb(980, 49   ),inb(981))  &(~(( 1    ) << (0))   );
  outb(980,( 49   ));outb(981,  tmp  )  ;
}

 








 





typedef struct {
  unsigned short foreColour,  
                 backColour;
  unsigned short image[32 * 32 ];  
  int address;  
}NV3Cursor;
  
  
static unsigned short ConvertToRGB555(int red,int green,int blue)
{
  unsigned short colour;

  colour=((red>>11)&31)<<10;
  colour|=((green>>11)&31)<<5;
  colour|=((blue>>11)&31);
  colour|=1<<15;  

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
  pad= ((PixmapWidthPaddingInfo[    1  ].notPower2 ? (((int)(  bits->width  ) * PixmapWidthPaddingInfo[    1  ].bytesPerPixel + PixmapWidthPaddingInfo[    1  ].bytesPerPixel) >> PixmapWidthPaddingInfo[    1  ].padBytesLog2) : ((int)((  bits->width  ) + PixmapWidthPaddingInfo[    1  ].padRoundUp) >> PixmapWidthPaddingInfo[    1  ].padPixelsLog2))  << PixmapWidthPaddingInfo[  1 ].padBytesLog2) ; 
  mask=(unsigned char*)bits->mask;
  for(y=0,i=0,lineOffset=0;y< 32 ;y++,lineOffset+=pad) {
    for(x=0;x< 32 ;x++,i++) {
      if(x<width && y<height) {
        byteIndex=lineOffset+(x/8);bitIndex=x%8;
        maskBit=mask[byteIndex]&(1<<bitIndex);
        sourceBit=source[byteIndex]&(1<<bitIndex);
        if(maskBit) {
	  cursor->image[i]=(sourceBit) ? cursor->foreColour :
                                         cursor->backColour;
	}else {
          cursor->image[i]= 0 ;
	}        
      }else {
        cursor->image[i]= 0 ;
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

   
  if(pCurs->bits->refcnt > 1) return 1 ;

  ram = (NV3Cursor*)Xalloc((unsigned long)( sizeof(NV3Cursor) )) ;
  *pPriv = (pointer) ram;
  if(!ram) return 0 ;

  ram->foreColour=
     ConvertToRGB555(pCurs->foreRed,pCurs->foreGreen,pCurs->foreBlue);
  ram->backColour=
     ConvertToRGB555(pCurs->backRed,pCurs->backGreen,pCurs->backBlue);
  
  RenderCursor(bits,ram);

  return 1 ;
}

 




static Bool NV3UnrealizeCursor(ScreenPtr pScr,CursorPtr pCurs)
{
  pointer priv;

  if(pCurs->bits->refcnt <= 1 &&
      (priv = pCurs->bits->devPriv[pScr->myNum])) {
    Xfree((pointer)( priv )) ;
    pCurs->bits->devPriv[pScr->myNum] = 0;
  }
  return 1 ;
}

 

















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

  save= (outb(980, 49   ),inb(981))  ;

  outb(980,( 49   ));outb(981,  0  )  ;

   
  for(i=0;i<numInts;i++) {
    nvPRAMINPort[ ((8192-2048)/4) +i ]=( image[i] ) ;
  }
   
  outb(980,( 48   ));outb(981,  (( (((unsigned)((  ((8192-2048)/4) *4  ) & (((unsigned)(1U << ((( 21)-( 16)+1)))-1)  << ( 16))  )) >> (16) )  ) << (0))    )  ;
  save&=7;
  outb(980,( 49   ));outb(981,  save| (( (((unsigned)((  ((8192-2048)/4) *4  ) & (((unsigned)(1U << ((( 15)-( 11)+1)))-1)  << ( 11))  )) >> (11) )  ) << (3))    )  ;
  

}

 








static void NV3LoadCursor(ScreenPtr pScr,CursorPtr pCurs,int x,int y)
{
 
  if(!xf86VTSema)
    return;

  if(!pCurs)
    return;

   
  NV3CursorpCurs = pCurs;

  NV3HideCursor();

  NV3LoadCursorToCard(pScr, pCurs);

   
  NV3MoveCursor(pScr, x, y);

   
  NV3ShowCursor();
}

 



static void NV3SetCursor(ScreenPtr pScr,CursorPtr pCurs,int x,int y,
                        Bool generateEvent)
{
  if(!pCurs)
    return;

  NV3CursorHotX = pCurs->bits->xhot;
  NV3CursorHotY = pCurs->bits->yhot;

  NV3LoadCursor(pScr, pCurs, x, y);
}

 




void NV3RestoreCursor(ScreenPtr pScr)
{
  int x, y;

  miPointerPosition(&x, &y);

  NV3LoadCursor(pScr, NV3CursorpCurs, x, y);
}

 




static void NV3MoveCursor(ScreenPtr pScr,int x,int y)
{
  int xorigin, yorigin;

  if(!xf86VTSema) return;

  x -= vga256InfoRec.frameX0 + NV3CursorHotX;
  y -= vga256InfoRec.frameY0 + NV3CursorHotY;
  x&= (((unsigned)(1U << ((( 11)-( 0)+1)))-1)  << ( 0))    ;
  nvPRAMDACPort[((6816512    )- (6815744) )/4] =(  ((   x   ) << (0))   |
                                   ((   y   ) << (16))     )  ;
}

 





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

 




void NV3WarpCursor(ScreenPtr pScr,int x,int y)
{
  miPointerWarpCursor(pScr, x, y);
  xf86Info.currentScreen = pScr;
}

 





void NV3QueryBestSize(int class,unsigned short *pwidth, 
                     unsigned short *pheight,ScreenPtr pScreen)
{
  if(*pwidth > 0) {
    if(class == 0 ) {
      *pwidth = 32 ;
      *pheight = 32 ;
    } else
      (void)mfbQueryBestSize(class, pwidth, pheight, pScreen);
  }
}


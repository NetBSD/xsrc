 /***************************************************************************\
|*                                                                           *|
|*       Copyright 1993-1998 NVIDIA, Corporation.  All rights reserved.      *|
|*                                                                           *|
|*     NOTICE TO USER:   The source code  is copyrighted under  U.S. and     *|
|*     international laws.  Users and possessors of this source code are     *|
|*     hereby granted a nonexclusive,  royalty-free copyright license to     *|
|*     use this code in individual and commercial software.                  *|
|*                                                                           *|
|*     Any use of this source code must include,  in the user documenta-     *|
|*     tion and  internal comments to the code,  notices to the end user     *|
|*     as follows:                                                           *|
|*                                                                           *|
|*       Copyright 1993-1998 NVIDIA, Corporation.  All rights reserved.      *|
|*                                                                           *|
|*     NVIDIA, CORPORATION MAKES NO REPRESENTATION ABOUT THE SUITABILITY     *|
|*     OF  THIS SOURCE  CODE  FOR ANY PURPOSE.  IT IS  PROVIDED  "AS IS"     *|
|*     WITHOUT EXPRESS OR IMPLIED WARRANTY OF ANY KIND.  NVIDIA, CORPOR-     *|
|*     ATION DISCLAIMS ALL WARRANTIES  WITH REGARD  TO THIS SOURCE CODE,     *|
|*     INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY, NONINFRINGE-     *|
|*     MENT,  AND FITNESS  FOR A PARTICULAR PURPOSE.   IN NO EVENT SHALL     *|
|*     NVIDIA, CORPORATION  BE LIABLE FOR ANY SPECIAL,  INDIRECT,  INCI-     *|
|*     DENTAL, OR CONSEQUENTIAL DAMAGES,  OR ANY DAMAGES  WHATSOEVER RE-     *|
|*     SULTING FROM LOSS OF USE,  DATA OR PROFITS,  WHETHER IN AN ACTION     *|
|*     OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,  ARISING OUT OF     *|
|*     OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOURCE CODE.     *|
|*                                                                           *|
|*     U.S. Government  End  Users.   This source code  is a "commercial     *|
|*     item,"  as that  term is  defined at  48 C.F.R. 2.101 (OCT 1995),     *|
|*     consisting  of "commercial  computer  software"  and  "commercial     *|
|*     computer  software  documentation,"  as such  terms  are  used in     *|
|*     48 C.F.R. 12.212 (SEPT 1995)  and is provided to the U.S. Govern-     *|
|*     ment only as  a commercial end item.   Consistent with  48 C.F.R.     *|
|*     12.212 and  48 C.F.R. 227.7202-1 through  227.7202-4 (JUNE 1995),     *|
|*     all U.S. Government End Users  acquire the source code  with only     *|
|*     those rights set forth herein.                                        *|
|*                                                                           *|
 \***************************************************************************/
/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/nv/nv4cursor.c,v 1.1.2.4 1998/11/18 16:38:47 hohndel Exp $ */
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

/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/nv/nv4cursor.c,v 1.1.2.4 1998/11/18 16:38:47 hohndel Exp $ */

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


static Bool NV4RealizeCursor();
static Bool NV4UnrealizeCursor();
static void NV4SetCursor();
static void NV4MoveCursor();
static void NV4RecolorCursor();

static miPointerSpriteFuncRec NV4PointerSpriteFuncs =
{
    NV4RealizeCursor,
    NV4UnrealizeCursor,
    NV4SetCursor,
    NV4MoveCursor,
};

extern miPointerScreenFuncRec xf86PointerScreenFuncs;
extern xf86InfoRec xf86Info;
static int NV4CursorGeneration = -1;
static int NV4CursorHotX;
static int NV4CursorHotY;
static CursorPtr NV4CursorpCurs;

Bool NV4CursorInit(char *pm,ScreenPtr pScr)
{
    unsigned char power;

    NV4CursorHotX = 0;
    NV4CursorHotY = 0;

    if (NV4CursorGeneration != serverGeneration)
    {
        if (!(miPointerInitialize(pScr, &NV4PointerSpriteFuncs,
                                  &xf86PointerScreenFuncs, 0 )))
        {
            return 0 ;
        }
        pScr->RecolorCursor = NV4RecolorCursor;
        NV4CursorGeneration = serverGeneration;
    }
    return 1 ;
}

static void NV4ShowCursor(void)
{
    unsigned char tmp;
    tmp = (outb(980, 49  ),inb(981)) |1| (( 1    ) << (0))   ;
    outb(980,( 49  ));outb(981, tmp ) ;
}

void NV4HideCursor(void)
{
    unsigned char tmp;
    tmp = (outb(980, 49  ),inb(981)) &(~(( 1    ) << (0))   );
    outb(980,( 49  ));outb(981, tmp ) ;
}

typedef struct
{
    unsigned short foreColour,  
    backColour;
    unsigned short image[32 * 32 ];  
    int address;  
}NV4Cursor;

static unsigned short ConvertToRGB555(int red,int green,int blue)
{
    unsigned short colour;

    colour=((red>>11)&31)<<10;
    colour|=((green>>11)&31)<<5;
    colour|=((blue>>11)&31);
    colour|=1<<15;  
    return colour;
}

static void RenderCursor(CursorBits *bits,NV4Cursor *cursor)
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
    for (y=0,i=0,lineOffset=0;y< 32 ;y++,lineOffset+=pad)
    {
        for (x=0;x< 32 ;x++,i++)
        {
            if (x<width && y<height)
            {
                byteIndex=lineOffset+(x/8);bitIndex=x%8;
                maskBit=mask[byteIndex]&(1<<bitIndex);
                sourceBit=source[byteIndex]&(1<<bitIndex);
                if (maskBit)
                {
                    cursor->image[i]=(sourceBit) ? cursor->foreColour :
                                     cursor->backColour;
                }
                else
                {
                    cursor->image[i]= 0 ;
                }
            }
            else
            {
                cursor->image[i]= 0 ;
            }
        }
    }
}

static Bool NV4RealizeCursor(ScreenPtr pScr,CursorPtr pCurs)
{
    NV4Cursor *ram;
    int index = pScr->myNum;
    pointer *pPriv = &pCurs->bits->devPriv[index];
    CursorBits *bits = pCurs->bits;

     
    if (pCurs->bits->refcnt > 1) return 1 ;

    ram = (NV4Cursor*)Xalloc((unsigned long)( sizeof(NV4Cursor) )) ;
    *pPriv = (pointer) ram;
    if (!ram) return 0 ;
    ram->foreColour=
    ConvertToRGB555(pCurs->foreRed,pCurs->foreGreen,pCurs->foreBlue);
    ram->backColour=
    ConvertToRGB555(pCurs->backRed,pCurs->backGreen,pCurs->backBlue);
    RenderCursor(bits,ram);
    return 1 ;
}

static Bool NV4UnrealizeCursor(ScreenPtr pScr,CursorPtr pCurs)
{
    pointer priv;

    if (pCurs->bits->refcnt <= 1 &&
        (priv = pCurs->bits->devPriv[pScr->myNum]))
    {
        Xfree((pointer)( priv )) ;
        pCurs->bits->devPriv[pScr->myNum] = 0;
    }
    return 1 ;
}

static void NV4LoadCursorToCard(ScreenPtr pScr,CursorPtr pCurs)
{
    NV4Cursor *cursor;
    int index = pScr->myNum;
    int i;
    int numInts;
    int *image;
    int save;

    if (!xf86VTSema)
        return;
    cursor=(NV4Cursor*) pCurs->bits->devPriv[index];
    numInts=sizeof(cursor->image)/sizeof(int);
    image=(int*)cursor->image;
    save= (outb(980, 49  ),inb(981)) ;
    outb(980,( 49  ));outb(981, 0 ) ;
    for (i=0;i<numInts;i++)
    {
        nvPRAMINPort[ ((65536)/4) +i ]=( image[i] ) ;
    }
    outb(980,( 48  ));outb(981, (( (((unsigned)((  ((65536)/4) *4+65536  ) & (((unsigned)(1U << ((( 23)-( 17)+1)))-1)  << ( 17))  )) >> (17) )  ) << (0))   ) ;
    save&=3;
    outb(980,( 49  ));outb(981, save| (( (((unsigned)((  ((65536)/4) *4+65536  ) & (((unsigned)(1U << ((( 16)-( 11)+1)))-1)  << ( 11))  )) >> (11) )  ) << (2))   ) ;
}

static void NV4LoadCursor(ScreenPtr pScr,CursorPtr pCurs,int x,int y)
{

    if (!xf86VTSema)
        return;
    if (!pCurs)
        return;
    NV4CursorpCurs = pCurs;
    NV4HideCursor();
    NV4LoadCursorToCard(pScr, pCurs);
    NV4MoveCursor(pScr, x, y);
    NV4ShowCursor();
}

static void NV4SetCursor(ScreenPtr pScr,CursorPtr pCurs,int x,int y,
                         Bool generateEvent)
{
    if (!pCurs)
        return;
    NV4CursorHotX = pCurs->bits->xhot;
    NV4CursorHotY = pCurs->bits->yhot;
    NV4LoadCursor(pScr, pCurs, x, y);
}

void NV4RestoreCursor(ScreenPtr pScr)
{
    int x, y;

    miPointerPosition(&x, &y);
    NV4LoadCursor(pScr, NV4CursorpCurs, x, y);
}

static void NV4MoveCursor(ScreenPtr pScr,int x,int y)
{
    int xorigin, yorigin;

    if (!xf86VTSema) return;
    x -= vga256InfoRec.frameX0 + NV4CursorHotX;
    y -= vga256InfoRec.frameY0 + NV4CursorHotY;
    x &= (((unsigned)(1U << ((( 11)-( 0)+1)))-1)  << ( 0))    ;
    nvPRAMDACPort[((6816512    )- (6815744) )/4] =(  
                  ((   x   ) << (0))   | ((   y   ) << (16))     )  ;
}

static void NV4RecolorCursor(ScreenPtr pScr,CursorPtr pCurs,Bool displayed)
{
    CursorBits *bits = pCurs->bits;
    int index = pScr->myNum;
    NV4Cursor *cursor=(NV4Cursor*) pCurs->bits->devPriv[index];
    unsigned short fore,back;

    if (!xf86VTSema) return;
    if (!displayed) return;
    fore=ConvertToRGB555(pCurs->foreRed,pCurs->foreGreen,pCurs->foreBlue);
    back=ConvertToRGB555(pCurs->backRed,pCurs->backGreen,pCurs->backBlue);
    if (cursor->foreColour==fore && cursor->backColour==back) return;
    NV4LoadCursorToCard(pScr,pCurs);
}

void NV4WarpCursor(ScreenPtr pScr,int x,int y)
{
    miPointerWarpCursor(pScr, x, y);
    xf86Info.currentScreen = pScr;
}

void NV4QueryBestSize(int class,unsigned short *pwidth,
                      unsigned short *pheight,ScreenPtr pScreen)
{
    if (*pwidth > 0)
    {
        if (class == 0 )
        {
            *pwidth = 32 ;
            *pheight = 32 ;
        }
        else
            (void)mfbQueryBestSize(class, pwidth, pheight, pScreen);
    }
}


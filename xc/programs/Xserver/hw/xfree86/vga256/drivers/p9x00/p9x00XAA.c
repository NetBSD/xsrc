/*
 * Copyright 1996-1997  Joerg Knura (knura@imst.de)
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
 * JOERG KNURA BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, 
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF 
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE 
 * SOFTWARE.
 */
/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/p9x00/p9x00XAA.c,v 1.1.2.4 1998/10/21 13:37:59 dawes Exp $ */

#define P9X00XAA_C
#include "xf86.h"
#include "xf86_OSlib.h"
#include "vga256.h"
#include "vga.h"

#include "xf86xaa.h"

#include "p9x00Regs.h"
#include "p9x00Access.h"
#include "p9x00Probe.h"

#define P91SRC 0xCCL
#define P91PAT 0xF0L
#define P91DST 0xAAL

#define P90SRC 0xCCCCL
#define P90PAT 0xFF00L
#define P90DST 0xAAAAL

CARD32 p9x00DefaultByteClipping;
CARD32 p9x00DefaultPixelClipping;
int p9x00BpPix;

static CARD32 p9x00CopyROP[16];
static CARD32 p9x00DrawROP[16];

int p9x00has_synced=0;

#define P9X_SYNC if (!p9x00has_synced) {\
                   while (P9X_R_STATUS&(P9X_RV_QUAD_BUSY|P9X_RV_DRAW_BUSY)){}\
                   p9x00has_synced=1;\
                 }

CARD32 p9x00DefaultByteClipping;
CARD32 p9x00DefaultPixelClipping;
int p9x00clipping_is_set=0;

#define P9X_UNCLIP if (p9x00clipping_is_set) {\
                     P9X_R_WMIN=0x00L;\
                     P9X_R_WMAX=p9x00DefaultPixelClipping;\
                     P9X_R_WMINB=0x00L;\
                     P9X_R_WMAXB=p9x00DefaultByteClipping;\
                     P9X_R_WOFFSET=0x00L;\
                     p9x00clipping_is_set=0;\
                   }
                 
static void p9x00SetColor(volatile CARD32 *reg,int color)
{
  CARD32 temp=(CARD32)color;
  switch(p9x00BpPix) {
    case 1:
      temp|=temp<<8;
      temp|=temp<<16;
      *reg=temp;
      return;
    case 2:
      temp=((temp&0x000000FFL)<<8)|
           ((temp&0x0000FF00L)>>8);
      temp|=temp<<16;
      *reg=temp;
      return;
    case 4:
      temp=((temp&0x000000FFL)<<24)|
           ((temp&0x00FF0000L)>>8)|
           ((temp&0x0000FF00L)<<8);
      *reg=temp;
      return;
    default:
      temp=((temp&0x000000FFL)<<24)|
           ((temp&0x00FF0000L)>>8)|
           ((temp&0x0000FF00L)<<8)|
           (temp&0x000000FFL);
      *reg=temp;
      return;
  }
}
  
void p9x00SetUpROP(void)
{
  CARD32 src;
  CARD32 dst;
  CARD32 pat;
  CARD32 set;

  int counter;
    
  if (P9X_CFG_CHIP==P9X_V_P9100) {
    src=P91SRC;
    dst=P91DST;
    pat=P91PAT;
    set=0xFFL;
  }
  else {
    src=P90SRC;
    dst=P90DST;
    pat=P90PAT;
    set=0xFFFFL;
  }
     
  p9x00CopyROP[GXclear]=       0;
  p9x00CopyROP[GXand]=         src&dst;
  p9x00CopyROP[GXandReverse]=  src&(~dst);
  p9x00CopyROP[GXcopy]=        src;
  p9x00CopyROP[GXandInverted]= (~src)&dst;
  p9x00CopyROP[GXnoop]=        dst;
  p9x00CopyROP[GXxor]=         src^dst;
  p9x00CopyROP[GXor]=          src|dst;
  p9x00CopyROP[GXnor]=         (~src)&(~dst);
  p9x00CopyROP[GXequiv]=       (~src)^dst;
  p9x00CopyROP[GXinvert]=      (~dst);
  p9x00CopyROP[GXorReverse]=   src|(~dst);
  p9x00CopyROP[GXcopyInverted]=(~src);
  p9x00CopyROP[GXorInverted]=  (~src)|dst;
  p9x00CopyROP[GXnand]=        (~src)|(~dst);
  p9x00CopyROP[GXset]=         set;

  p9x00DrawROP[GXclear]=       0;
  p9x00DrawROP[GXand]=         pat&dst;
  p9x00DrawROP[GXandReverse]=  pat&(~dst);
  p9x00DrawROP[GXcopy]=        pat;
  p9x00DrawROP[GXandInverted]= (~pat)&dst;
  p9x00DrawROP[GXnoop]=        dst;
  p9x00DrawROP[GXxor]=         pat^dst;
  p9x00DrawROP[GXor]=          pat|dst;
  p9x00DrawROP[GXnor]=         (~pat)&(~dst);
  p9x00DrawROP[GXequiv]=       (~pat)^dst;
  p9x00DrawROP[GXinvert]=      (~dst);
  p9x00DrawROP[GXorReverse]=   pat|(~dst);
  p9x00DrawROP[GXcopyInverted]=(~pat);
  p9x00DrawROP[GXorInverted]=  (~pat)|dst;
  p9x00DrawROP[GXnand]=        (~pat)|(~dst);
  p9x00DrawROP[GXset]=         pat;

  for (counter=0;counter<16;counter++) {
    p9x00DrawROP[counter]&=set;
    p9x00CopyROP[counter]&=set;
  }
}


void p9x00Sync();
void p9x00SetupForFillRectSolid();
void p9x00SubsequentFillRectSolid();
void p9x00SetupForScreenToScreenCopy();
void p9x00SubsequentScreenToScreenCopy();
void p9x00SubsequentTwoPointLine();
void p9x00SetClippingRectangle();
void p9x00SetupFor8x8PatternColorExpand();
void p9x00Subsequent8x8PatternColorExpand();
void p9x00SetupForCPUToScreenColorExpand();
void p9x00SubsequentCPUToScreenColorExpand();


void p9x00AccelInit() {

  xf86AccelInfoRec.Flags = 
    BACKGROUND_OPERATIONS | 
    COP_FRAMEBUFFER_CONCURRENCY |
    PIXMAP_CACHE |
    NO_SYNC_AFTER_CPU_COLOR_EXPAND |
    HARDWARE_CLIP_LINE|
    HARDWARE_PATTERN_PROGRAMMED_BITS|
    HARDWARE_PATTERN_PROGRAMMED_ORIGIN|
    HARDWARE_PATTERN_MONO_TRANSPARENCY|
    HARDWARE_PATTERN_BIT_ORDER_MSBFIRST;

  xf86AccelInfoRec.Sync = p9x00Sync;

  xf86GCInfoRec.PolyFillRectSolidFlags = 0;

  xf86AccelInfoRec.SetupForFillRectSolid = 
    p9x00SetupForFillRectSolid;
  xf86AccelInfoRec.SubsequentFillRectSolid = 
    p9x00SubsequentFillRectSolid;

  xf86GCInfoRec.CopyAreaFlags = NO_TRANSPARENCY;
 
  xf86AccelInfoRec.SetupForScreenToScreenCopy =
    p9x00SetupForScreenToScreenCopy;
  xf86AccelInfoRec.SubsequentScreenToScreenCopy =
    p9x00SubsequentScreenToScreenCopy;

  xf86AccelInfoRec.SubsequentTwoPointLine =
    p9x00SubsequentTwoPointLine;
  xf86AccelInfoRec.SetClippingRectangle =
    p9x00SetClippingRectangle;

/*  xf86AccelInfoRec.PatternFlags =
    HARDWARE_PATTERN_PROGRAMMED_BITS|
    HARDWARE_PATTERN_PROGRAMMED_ORIGIN|
    HARDWARE_PATTERN_MONO_TRANSPARENCY|
    HARDWARE_PATTERN_BIT_ORDER_MSBFIRST;
*/
  xf86AccelInfoRec.SetupFor8x8PatternColorExpand =
    p9x00SetupFor8x8PatternColorExpand;
  xf86AccelInfoRec.Subsequent8x8PatternColorExpand=
    p9x00Subsequent8x8PatternColorExpand;

  xf86AccelInfoRec.ColorExpandFlags =
    ONLY_TRANSPARENCY_SUPPORTED|
    SCANLINE_PAD_DWORD|
    CPU_TRANSFER_PAD_DWORD|
    CPU_TRANSFER_BASE_FIXED;

  xf86AccelInfoRec.CPUToScreenColorExpandBase = 
    (pointer)((char *)(&(P9X_R_PIXEL1((CARD32)31L)))+0x70000L);
 
  xf86AccelInfoRec.SetupForCPUToScreenColorExpand =
    p9x00SetupForCPUToScreenColorExpand;
  xf86AccelInfoRec.SubsequentCPUToScreenColorExpand = 
    p9x00SubsequentCPUToScreenColorExpand;

    /*
     * Finally, we set up the video memory space available to the pixmap
     * cache. In this case, all memory from the end of the virtual screen
     * to the end of video memory minus 1K, can be used. If you haven't
     * enabled the PIXMAP_CACHE flag, then these lines can be omitted.
     */
  xf86AccelInfoRec.PixmapCacheMemoryStart =
    vga256InfoRec.virtualY * vga256InfoRec.virtualX
    * (vga256InfoRec.bitsPerPixel+1) / 8;
  xf86AccelInfoRec.PixmapCacheMemoryEnd =
    vga256InfoRec.videoRam * 1024 - 1024;

  p9x00SetUpROP();    
}

void p9x00Sync() {
  P9X_SYNC;
}

void p9x00SetupForFillRectSolid(int color, int rop, unsigned planemask)
{
  P9X_SYNC;
  P9X_UNCLIP;
  p9x00SetColor(&P9X_R_COLOR0,color);
  P9X_R_RASTER=p9x00DrawROP[rop];
  P9X_R_PMASK=planemask;
  P9X_R_CINDEX=0x0L;
}

void p9x00SubsequentFillRectSolid(int x, int y, int w, int h)
{
  CARD32 temp;
  
  P9X_SYNC;
  P9X_R_RECT_WIN_XY=((CARD32)x<<16)|((CARD32)y&0xFFFFL);
  P9X_R_RECT_WIN_XY=((CARD32)(x+w)<<16)|((CARD32)(y+h)&0xFFFFL);
  temp=P9X_R_QUAD;
  p9x00has_synced=0;
}


void p9x00SetupForScreenToScreenCopy(int xdir, int ydir, int rop, 
                                    unsigned planemask, int transparency_color)
{
   P9X_SYNC;
   P9X_UNCLIP;
   P9X_R_RASTER=p9x00CopyROP[rop];
   P9X_R_PMASK=planemask;
}

void p9x00SubsequentScreenToScreenCopy(int x1, int y1, int x2, int y2, 
                                       int w, int h)
{
  CARD32 bpp=(CARD32)p9x00BpPix;
  CARD32 temp;
    
  P9X_SYNC;

  P9X_R_EDGE0_ABS_SCR_XY=((CARD32)(x1*bpp)<<16)|((CARD32)y1&0xFFFFL);
  P9X_R_EDGE2_ABS_SCR_XY=((CARD32)(x2*bpp)<<16)|((CARD32)y2&0xFFFFL);
  P9X_R_EDGE1_ABS_SCR_XY=((CARD32)((x1+w)*bpp-1)<<16)|((CARD32)(y1+h-1)&0xFFFFL);
  P9X_R_EDGE3_ABS_SCR_XY=((CARD32)((x2+w)*bpp-1)<<16)|((CARD32)(y2+h-1)&0xFFFFL);
  temp=P9X_R_BLIT;
  p9x00has_synced=0;
}


void p9x00SetClippingRectangle(int x1,int y1,int x2,int y2)
{
  P9X_SYNC;
  P9X_R_WMIN=((CARD32)x1<<16)|(CARD32)(y1&0xFFFFL);
  P9X_R_WMAX=((CARD32)x2<<16)|(CARD32)(y2&0xFFFFL);
  P9X_R_WMINB=((CARD32)(x1*p9x00BpPix)<<16)|(CARD32)(y1&0xFFFFL);
  P9X_R_WMAXB=((CARD32)(x2*p9x00BpPix)<<16)|(CARD32)(y2&0xFFFFL);
  P9X_R_WOFFSET=((CARD32)x1<<16)|(CARD32)(y1&0xFFFFL);
  p9x00clipping_is_set=1;
}


void p9x00SubsequentTwoPointLine(int x1,int y1,int x2,int y2,int bias)
{
  CARD32 temp;
  P9X_SYNC;
  P9X_R_LINE_WIN_XY=((CARD32)x1<<16)|((CARD32)y1&0xFFFFL);
  P9X_R_LINE_WIN_XY=((CARD32)x2<<16)|((CARD32)y2&0xFFFFL);
  P9X_R_RASTER|=P9X_RV_OVERSIZE;
  temp=P9X_R_QUAD;
  p9x00has_synced=0;
}


void p9x00SetupFor8x8PatternColorExpand(int patx, int paty,
                                        int bg, int fg,
                                        int rop, int planemask)
{
  CARD32 pat0=((CARD32)(patx&0xFFFFL));
  CARD32 pat1=((CARD32)(patx&0xFFFF0000L)>>16);
  CARD32 pat2=((CARD32)(paty&0xFFFFL));
  CARD32 pat3=((CARD32)(paty&0xFFFF0000L)>>16);
  
  pat0=((pat0&0xFF00L)<<8)|(pat0&0xFFL);
  pat1=((pat1&0xFF00L)<<8)|(pat1&0xFFL);
  pat2=((pat2&0xFF00L)<<8)|(pat2&0xFFL);
  pat3=((pat3&0xFF00L)<<8)|(pat3&0xFFL);
  
  P9X_SYNC;
  P9X_UNCLIP;
  p9x00SetColor(&P9X_R_COLOR1,fg);
  if (bg!=(-1)) {
    p9x00SetColor(&P9X_R_COLOR0,bg);
    P9X_R_RASTER=p9x00DrawROP[rop]|P9X_RV_NO_SOLID;
  }
  else {
    P9X_R_RASTER=p9x00DrawROP[rop]|P9X_RV_NO_SOLID|P9X_RV_TRANS;
  }

  P9X_R_PATTERN0=pat0;
  P9X_R_PATTERN1=pat1;
  P9X_R_PATTERN2=pat2;
  P9X_R_PATTERN3=pat3;
  
  P9X_R_PMASK=planemask;
}

void p9x00Subsequent8x8PatternColorExpand(int patx, int paty,
                                          int x, int y, int w, int h)
{
  CARD32 temp;
  
  P9X_SYNC;
  P9X_R_PORIGX=patx;
  P9X_R_PORIGY=paty;
  P9X_R_RECT_WIN_XY=((CARD32)x<<16)|((CARD32)y&0xFFFFL);
  P9X_R_RECT_WIN_XY=((CARD32)(x+w)<<16)|((CARD32)(y+h)&0xFFFFL);
  temp=P9X_R_QUAD;
  p9x00has_synced=0;
}


void p9x00SetupForCPUToScreenColorExpand(int bg, int fg,
                                         int rop, int planemask)
{
  P9X_SYNC;
  p9x00SetColor(&P9X_R_COLOR0,fg);
  P9X_R_RASTER=p9x00DrawROP[rop]|P9X_RV_PIX1_TRANS;
  P9X_R_PMASK=planemask;
}


void p9x00SubsequentCPUToScreenColorExpand(int x, int y, int w, int h, int  skip)
{
  CARD32 temp=(w>>5)<<5;
  
  if (temp<w) temp+=32;
  P9X_SYNC;
  P9X_R_EDGE0_ABS_SCR_X=(CARD32)(x);
  P9X_R_EDGE1_ABS_SCR_XY=((CARD32)(x)<<16)|((CARD32)y&0xFFFFL);
  P9X_R_EDGE2_ABS_SCR_X=(CARD32)(x+temp);
  P9X_R_EDGE3_ABS_SCR_Y=(CARD32)1;
}


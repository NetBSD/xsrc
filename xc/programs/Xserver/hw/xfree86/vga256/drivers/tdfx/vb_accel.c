/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/tdfx/vb_accel.c,v 1.1.2.7 1999/10/21 12:08:08 hohndel Exp $ */
/*
   Voodoo Banshee driver version 1.0.2

   Authors: Daryll Strauss, Scott J. Bertin

   Copyright: 1998,1999
*/

#include "compiler.h"
#include "vga256.h"
#include "xf86.h"
#include "vga.h"

/* Drivers that need to access the PCI config space directly need this */
#include "vgaPCI.h"

/* Drivers that use XAA need this */
#include "xf86xaa.h"
#include "xf86local.h"

#include "vb.h"

#if (defined(__STDC__) && !defined(UNIXCPP)) || defined(ANSICPP)
#define CONCATSTR(prefix,suffix) prefix##suffix
#else
#define CONCATSTR(prefix,suffix) prefix/**/suffix
#endif

#if PSZ == 8
#define VBNAME(subname) CONCATSTR(VB8, subname)
#elif PSZ == 16
#define VBNAME(subname) CONCATSTR(VB16, subname)
#elif PSZ == 24
#define VBNAME(subname) CONCATSTR(VB24, subname)
#elif PSZ == 32
#define VBNAME(subname) CONCATSTR(VB32, subname)
#endif

extern void VBIdle();
extern void VBSetupForFillRectSolid(int, int, unsigned int);
extern void VBSubsequentFillRectSolid(int, int, int, int);
extern void VBSetupForScreenToScreenCopy(int, int, int, unsigned int, int);
extern void VBSubsequentScreenToScreenCopy(int, int, int, int, int, int);
extern void VBSetupForCPUToScreenColorExpand(int, int, int, unsigned int);
extern void VBSubsequentCPUToScreenColorExpand(int, int, int, int, int);
extern void VBSetupForScreenToScreenColorExpand(int, int, int, unsigned int);
extern void VBSubsequentScreenToScreenColorExpand(int, int, int, int, int, int);
extern void VBSetClippingRectangle(int, int, int, int);
extern void VBSubsequentTwoPointLine(int, int, int, int, int);
extern void VBSetupForDashedLine(int, int, int, unsigned int, int);
extern void VBSubsequentDashedTwoPointLine(int, int, int, int, int, int);
extern void VBSetupFor8x8PatternColorExpand(int, int, int, int, int,
                                            unsigned int);
extern void VBSubsequent8x8PatternColorExpand(int, int, int, int, int, int);
extern void VBSubsequentFillTrapezoidSolid(int, int, int, int, int, int, int,
                                           int, int, int);
extern void VBImageWrite(int, int, int, int, void *, int, int, unsigned int);
extern void VBFillRect(DrawablePtr, GCPtr, int, BoxPtr);
extern void VBPolyFillRect(DrawablePtr, GCPtr, int, xRectangle *);
extern void VBFillPolygonSolid(DrawablePtr, GCPtr, int, int, int, DDXPointPtr);
extern void VBPolyLine(DrawablePtr, GCPtr, int, int, DDXPointPtr);
extern void VBPolySegment(DrawablePtr, GCPtr, int, xSegment *);

extern unsigned char byte_reversed[256];

#define REF32(addr) *((volatile int*)&pVB->IOMap[addr])

#ifdef TRACEREG
#define VBLaunch VBLaunchDebug
#else
#define VBLaunch(a, v) REF32(a)=v
#endif


#if PSZ == 8

/* 
   The ROP codes are based on windows "Ternary Restart Operations" which is
   this odd bit format. 
*/
static int VBROPCvt[] = {0x00, 0x88, 0x44, 0xCC, 0x22, 0xAA, 0x66, 0xEE,
			 0x11, 0x99, 0x55, 0xDD, 0x33, 0xBB, 0x77, 0xFF,
                         0x00, 0xA0, 0x50, 0xF0, 0x0A, 0xAA, 0x5A, 0xFA,
			 0x05, 0xA5, 0x55, 0xF5, 0x0F, 0xAF, 0x5F, 0xFF};

#define ROP_PATTERN_OFFSET 16

void
VBMakeRoom(int size) {
  VBPtr pVB;
  int stat;

  pVB = VBPTR();
  pVB->PciCnt-=size;
  if (pVB->PciCnt<1) {
    do {
      stat=REF32(0);
      pVB->PciCnt=stat&0x1F;
    } while (pVB->PciCnt<size);
  }
}

void
VBLaunchDebug(int addr, int v) {
  VBPtr pVB;
  int stat;

  pVB = VBPTR();
  REF32(addr)=v;
  VBTRACEREG("Send offset %x = %x\n", addr, v);
}

void VBIdle()
{
  VBPtr pVB;
  int i;
  int stat;

  VBTRACEACCEL("VBIdle start\n");
  pVB=VBPTR();
  VBMakeRoom(1);
  VBLaunch(SST_3D_OFFSET+SST_3D_COMMAND, SST_3D_NOP);
  pVB->BltPrevY=-1;
  i=0;
  while (1) {
    stat=REF32(0);
    if (stat&SST_BUSY) {
      i=0;
    } else {
      i++;
    }
    if (i==3) break;
  }
  pVB->PciCnt=stat&0x1F;
}

void
VBSetupForFillRectSolid(int color, int rop, unsigned int planemask) {
  /* This is used for rectangles, lines and trapezoids.
   * Can't set the command register here.
   */
  VBPtr pVB;
  int fmt;


  VBTRACEACCEL("SetupForFillRectSolid color=%x rop=%d\n", color, rop);
  pVB = VBPTR();
  pVB->CurrentROP = rop;
  pVB->CurrentCmd = SST_2D_NOP;

  if (pVB->cpp==1) fmt=1; else fmt=pVB->cpp+1;
  VBMakeRoom(2);
  VBLaunch(SST_2D_OFFSET+SST_2D_DSTFORMAT, pVB->stride | (fmt<<16));
  VBLaunch(SST_2D_OFFSET+SST_2D_COLORFORE, color);
}

void
VBSubsequentFillRectSolid(int x, int y, int w, int h)
{
  VBPtr pVB;

  VBTRACEACCEL("SubsequentFillRectSolid at (%x,%x) size (%x,%x)\n", x, y, w, h);
  pVB = VBPTR();

  if(pVB->CurrentCmd != SST_2D_RECTANGLEFILL) {
    pVB->CurrentCmd = SST_2D_RECTANGLEFILL;
    VBMakeRoom(1);
    VBLaunch(SST_2D_OFFSET+SST_2D_COMMAND,
             SST_2D_RECTANGLEFILL|(VBROPCvt[pVB->CurrentROP]<<24));
  }
  
  VBMakeRoom(2);
  VBLaunch(SST_2D_OFFSET+SST_2D_DSTSIZE, (w&0x1FFF) | ((h&0x1FFF)<<16));
  VBLaunch(SST_2D_OFFSET+SST_2D_LAUNCH, (x&0x1FFF) | ((y&0x1FFF)<<16));
  pVB->BltPrevY=y;
}

void
VBSetupForScreenToScreenCopy(int xdir, int ydir, int rop, 
			     unsigned int planemask, int trans_color) {
  VBPtr pVB;
  int cmd, fmt;

  VBTRACEACCEL("SetupForScreenToScreenCopy xdir=%d ydir=%d rop=%d trans_color=%d\n",
	 xdir, ydir, rop, trans_color);
  pVB = VBPTR();

  pVB->BlitDir=0;
  cmd=SST_2D_SCRNTOSCRNBLIT|(VBROPCvt[rop]<<24);
  if (xdir==-1) {
    pVB->BlitDir=BLIT_LEFT;
    cmd|=BIT(14);
  }
  if (ydir==-1) {
    pVB->BlitDir|=BLIT_UP;
    cmd|=BIT(15);
  }

  if (pVB->cpp==1) fmt=1; else fmt=pVB->cpp+1;
  
  if(trans_color != -1) {
    VBMakeRoom(3);
    VBLaunch(SST_2D_OFFSET+SST_2D_SRCCOLORKEYMIN, trans_color);
    VBLaunch(SST_2D_OFFSET+SST_2D_SRCCOLORKEYMAX, trans_color);
    VBLaunch(SST_2D_OFFSET+SST_2D_ROP, VBROPCvt[GXnoop]<<8);
    pVB->Transparent = TRUE;
  } else {
    pVB->Transparent = FALSE;
  }
  
  VBMakeRoom(3);
  VBLaunch(SST_2D_OFFSET+SST_2D_SRCFORMAT, pVB->stride | (fmt<<16));
  VBLaunch(SST_2D_OFFSET+SST_2D_DSTFORMAT, pVB->stride | (fmt<<16));
  VBLaunch(SST_2D_OFFSET+SST_2D_COMMAND, cmd);
}

void
VBSubsequentScreenToScreenCopy(int srcX, int srcY, int dstX, int dstY,
			       int w, int h) {
  VBPtr pVB;
  VBTRACEACCEL("SubsequentScreenToScreenCopy srcx=%x srcy=%x dstx=%x dsty=%x w=%x h=%x\n",
         srcX, srcY, dstX, dstY, w, h);
  pVB = VBPTR();

  if (pVB->BlitDir&BLIT_LEFT) {
    srcX+=w-1;
    dstX+=w-1;
  }
  if (pVB->BlitDir&BLIT_UP) {
    srcY+=h-1;
    dstY+=h-1;
  }
  if (srcY>=pVB->BltPrevY-8 && srcY<=pVB->BltPrevY) {
    VBMakeRoom(1);
    VBLaunch(SST_3D_OFFSET+SST_3D_COMMAND, SST_3D_NOP);
  }
  if(pVB->Transparent) {
    /* Turn on source color keying for transpaerncy. */
    VBMakeRoom(1);
    VBLaunch(SST_2D_OFFSET+SST_2D_COMMANDEXTRA, 1);
  }
  VBMakeRoom(3);
  VBLaunch(SST_2D_OFFSET+SST_2D_DSTSIZE, (w&0x1FFF) | ((h&0x1FFF)<<16));
  VBLaunch(SST_2D_OFFSET+SST_2D_DSTXY, (dstX&0x1FFF) | ((dstY&0x1FFF)<<16));
  VBLaunch(SST_2D_OFFSET+SST_2D_LAUNCH, (srcX&0x1FFF) | ((srcY&0x1FFF)<<16));
  if(pVB->Transparent) {
    /* Turn off source color keying to avoid affecting other operations. */
    VBMakeRoom(1);
    VBLaunch(SST_2D_OFFSET+SST_2D_COMMANDEXTRA, 0);
  }
  pVB->BltPrevY=dstY;
}

void
VBSetupForCPUToScreenColorExpand(int bg, int fg, int rop,
                                 unsigned int planemask)
{
  VBPtr pVB;
  int cmd;
  
  VBTRACEACCEL("SetupForCPUToScreenColorExpand bg=%x fg=%x rop=%d\n", bg, fg, rop);
  pVB = VBPTR();
  
  cmd=SST_2D_HOSTTOSCRNBLIT|(VBROPCvt[rop]<<24);
  
  if(bg == -1) {
    cmd |= SST_2D_TRANSPARENT_MONOCHROME;
    VBMakeRoom(2);
  } else {
    VBMakeRoom(3);
    VBLaunch(SST_2D_OFFSET+SST_2D_COLORBACK, bg);
  }
  VBLaunch(SST_2D_OFFSET+SST_2D_COLORFORE, fg);
  VBLaunch(SST_2D_OFFSET+SST_2D_COMMAND, cmd);
}

void
VBSubsequentCPUToScreenColorExpand(int x, int y, int w, int h, int skipleft)
{
  VBPtr pVB;
  int fmt;
  
  VBTRACEACCEL("SubsequentCPUToScreenColorExpand x=%d y=%d w=%d h=%d skipleft=%d\n",
         x, y, w, h, skipleft);
  pVB = VBPTR();
  
  if (pVB->cpp==1) fmt=1; else fmt=pVB->cpp+1;

  VBMakeRoom(5);
  VBLaunch(SST_2D_OFFSET+SST_2D_SRCFORMAT, (((w+31)/32)*4) & 0x3FFF);
  VBLaunch(SST_2D_OFFSET+SST_2D_DSTFORMAT, pVB->stride | (fmt<<16));
  VBLaunch(SST_2D_OFFSET+SST_2D_DSTSIZE, (w-skipleft&0x1FFF)|((h&0x1FFF)<<16));
  VBLaunch(SST_2D_OFFSET+SST_2D_SRCXY, skipleft&0x1F);
  VBLaunch(SST_2D_OFFSET+SST_2D_DSTXY, (x+skipleft&0x1FFF) | ((y&0x1FFF)<<16));
  pVB->BltPrevY=y;
}

void
VBSetupForScreenToScreenColorExpand(int bg, int fg, int rop,
                                    unsigned int planemask)
{
  VBPtr pVB;
  int cmd;
  
  VBTRACEACCEL("SetupForScreenToScreenColorExpand bg=%x fg=%x rop=%d\n", bg, fg, rop);
  pVB = VBPTR();
  
  cmd=SST_2D_SCRNTOSCRNBLIT|(VBROPCvt[rop]<<24);
  
  if(bg == -1) {
    cmd |= SST_2D_TRANSPARENT_MONOCHROME;
    VBMakeRoom(2);
  } else {
    VBMakeRoom(3);
    VBLaunch(SST_2D_OFFSET+SST_2D_COLORBACK, bg);
  }
  VBLaunch(SST_2D_OFFSET+SST_2D_COLORFORE, fg);
  VBLaunch(SST_2D_OFFSET+SST_2D_COMMAND, cmd);
}

void
VBSubsequentScreenToScreenColorExpand(int srcx, int srcy, int x, int y, int w,
                                      int h)
{
  VBPtr pVB;
  int fmt;
  
  VBTRACEACCEL("SubsequentScreenToScreenColorExpand srcx=%d srcy=%d x=%d y=%d w=%d h=%d\n",
         srcx, srcy, x, y, w, h);
  
  pVB = VBPTR();
  
  if (pVB->cpp==1) fmt=1; else fmt=pVB->cpp+1;

  VBMakeRoom(5);
  VBLaunch(SST_2D_OFFSET+SST_2D_SRCFORMAT, SST_2D_SOURCE_PACKING_DWORD);
  VBLaunch(SST_2D_OFFSET+SST_2D_DSTFORMAT, pVB->stride | (fmt<<16));
  VBLaunch(SST_2D_OFFSET+SST_2D_DSTSIZE, (w&0x1FFF) | ((h&0x1FFF)<<16));
  VBLaunch(SST_2D_OFFSET+SST_2D_DSTXY, (x&0x1FFF) | ((y&0x1FFF)<<16));
  VBLaunch(SST_2D_OFFSET+SST_2D_LAUNCH, (srcx&0x1FFF) | ((srcy&0x1FFF)<<16));
  pVB->BltPrevY=y;
}

void
VBSetClippingRectangle(int x1, int y1, int x2, int y2)
{
  VBPtr pVB;
  
  VBTRACEACCEL("SetClippingRectangle x1=%d y1=%d, x2=%d, y2=%d\n", x1, y1, x2, y2);
  pVB = VBPTR();
  
  VBMakeRoom(2);
  VBLaunch(SST_2D_OFFSET+SST_2D_CLIP1MIN, (x1&0xFFF) | ((y1&0xFFF)<<16));
  VBLaunch(SST_2D_OFFSET+SST_2D_CLIP1MAX, ((x2+1)&0xFFF)|(((y2+1)&0xFFF)<<16));
  
  pVB->ClipSelect = 1;
}

void
VBSubsequentTwoPointLine(int x1, int y1, int x2, int y2, int bias)
{
  VBPtr pVB;
  int cmd;
  
  VBTRACEACCEL("SubsequentTwoPointLine x1=%d y1=%d, x2=%d, y2=%d, bias=%x\n",
         x1, y1, x2, y2, bias);
  pVB = VBPTR();
  
  if(bias&0x100) {
    cmd=SST_2D_POLYLINE;
  } else {
    cmd=SST_2D_LINE;
  }
  if(pVB->CurrentCmd != cmd) {
    pVB->CurrentCmd = cmd;
    cmd |= (pVB->ClipSelect<<23) | (VBROPCvt[pVB->CurrentROP]<<24);
    VBMakeRoom(1);
    VBLaunch(SST_2D_OFFSET+SST_2D_COMMAND, cmd);
  }
  
  VBMakeRoom(2);
  VBLaunch(SST_2D_OFFSET+SST_2D_SRCXY, (x1&0x1FFF) | ((y1&0x1FFF)<<16));
  if (pVB->ErrorSet) {
    VBMakeRoom(1);
    VBLaunch(SST_2D_OFFSET+SST_2D_BRESERROR0, 0);
    pVB->ErrorSet=FALSE;
  }
  VBLaunch(SST_2D_OFFSET+SST_2D_LAUNCH, (x2&0x1FFF) | ((y2&0x1FFF)<<16));

  pVB->BltPrevY=y2;
  pVB->ClipSelect = 0;
}

void
VBSetupForDashedLine(int fg, int bg, int rop, unsigned int planemask, int size)
{
  VBPtr pVB;
  int cmd;
  int i, linestipple;
  
  pVB = VBPTR();
  
  VBTRACEACCEL("SetupForDashedLine fg=%x bg=%x, rop=%d, size=%d, pattern=%x\n",
         fg, bg, rop, size, pVB->LinePatternBuffer);
  pVB->CurrentROP = rop;
  pVB->DashedLineSize = ((size-1)&0xFF)+1;

  /* X Starts the pattern with the MSB, 3dfx starts with the LSB.
   * Swap the bit order.
   */
  linestipple=0;
  for(i=0; i<pVB->DashedLineSize; ++i) {
    if(pVB->LinePatternBuffer & BIT(i)) {
    	linestipple |= BIT(pVB->DashedLineSize-i-1);
    }
  }
  
  if(bg == -1) {
    pVB->Transparent = TRUE;
    VBMakeRoom(2);
  } else {
    pVB->Transparent = FALSE;
    VBMakeRoom(3);
    VBLaunch(SST_2D_OFFSET+SST_2D_COLORBACK, bg);
  }
  VBLaunch(SST_2D_OFFSET+SST_2D_COLORFORE, fg);
  VBLaunch(SST_2D_OFFSET+SST_2D_LINESTIPPLE, linestipple);
}

void
VBSubsequentDashedTwoPointLine(int x1, int y1, int x2, int y2, int bias,
                               int offset)
{
  VBPtr pVB;
  int cmd, linestyle;
  
  VBTRACEACCEL("SubsequentDashedTwoPointLine x1=%d y1=%d x2=%d y2=%d bias=%x offset=%d\n",
         x1, y1, x2, y2, bias, offset);
  pVB = VBPTR();
  
  if(bias&0x100) {
    cmd=SST_2D_POLYLINE;
  } else {
    cmd=SST_2D_LINE;
  }
  if(pVB->Transparent) {
    cmd |= SST_2D_TRANSPARENT_MONOCHROME;
  }
  cmd |= (pVB->ClipSelect<<23) |
         SST_2D_STIPPLE_LINE |
         (VBROPCvt[pVB->CurrentROP]<<24);
  
  linestyle = ((pVB->DashedLineSize-1)<<8) |
              (((offset%pVB->DashedLineSize)&0x1F)<<24);
  
  VBMakeRoom(4);
  VBLaunch(SST_2D_OFFSET+SST_2D_LINESTYLE, linestyle);
  VBLaunch(SST_2D_OFFSET+SST_2D_SRCXY, (x1&0x1FFF) | ((y1&0x1FFF)<<16));
  VBLaunch(SST_2D_OFFSET+SST_2D_COMMAND, cmd);
  if (pVB->ErrorSet) {
    VBMakeRoom(1);
    VBLaunch(SST_2D_OFFSET+SST_2D_BRESERROR0, 0);
    pVB->ErrorSet=FALSE;
  }
  VBLaunch(SST_2D_OFFSET+SST_2D_LAUNCH, (x2&0x1FFF) | ((y2&0x1FFF)<<16));

  pVB->BltPrevY=y2;
  pVB->ClipSelect = 0;
}

void
VBSetupFor8x8PatternColorExpand(int pattern0, int pattern1, int bg, int fg,
                                int rop, unsigned int planemask)
{
  VBPtr pVB;
  
  VBTRACEACCEL("SetupFor8x8PatternColorExpand pattern0=%x pattern1=%x bg=%x fg=%x rop=%d\n",
         pattern0, pattern1, bg, fg, rop);
  pVB = VBPTR();
  
  pVB->CurrentROP = rop;
  
  if(bg == -1) {
    pVB->Transparent = TRUE;
    VBMakeRoom(3);
  } else {
    pVB->Transparent = FALSE;
    VBMakeRoom(4);
    VBLaunch(SST_2D_OFFSET+SST_2D_COLORBACK, bg);
  }
  VBLaunch(SST_2D_OFFSET+SST_2D_COLORFORE, fg);
  VBLaunch(SST_2D_OFFSET+SST_2D_PATTERN0, pattern0);
  VBLaunch(SST_2D_OFFSET+SST_2D_PATTERN1, pattern1);
}

void
VBSubsequent8x8PatternColorExpand(int patternx, int patterny, int x, int y,
                                  int w, int h)
{
  VBPtr pVB;
  int cmd, fmt;
  
  VBTRACEACCEL("Subsequent8x8PatternColorExpand patternx=%d patterny=%d x=%d y=%d w=%d h=%d\n",
         patternx, patterny, x, y, w, h);
  pVB = VBPTR();
  
  cmd = SST_2D_RECTANGLEFILL |
        SST_2D_MONOCHROME_PATTERN |
        (VBROPCvt[pVB->CurrentROP+ROP_PATTERN_OFFSET] << 24) |
        ((patternx&0x7) << 17) |
        ((patterny&0x7) << 20);
  
  if(pVB->Transparent) {
        cmd |= SST_2D_TRANSPARENT_MONOCHROME;
  }
  
  if (pVB->cpp==1) fmt=1; else fmt=pVB->cpp+1;
  
  VBMakeRoom(4);
  VBLaunch(SST_2D_OFFSET+SST_2D_DSTFORMAT, pVB->stride | (fmt<<16));
  VBLaunch(SST_2D_OFFSET+SST_2D_DSTSIZE, (w&0x1FFF) | ((h&0x1FFF)<<16));
  VBLaunch(SST_2D_OFFSET+SST_2D_COMMAND, cmd);
  VBLaunch(SST_2D_OFFSET+SST_2D_LAUNCH, (x&0x1FFF) | ((y&0x1FFF)<<16));
  pVB->BltPrevY=y;
}

void
VBSubsequentFillTrapezoidSolid(int ytop, int height, int left, int dxL,
                               int dyL, int eL, int right, int dxR, int dyR,
                               int eR)
{
  /* IMPORTANT: Do not use the error terms, the card will lock up! */
  VBPtr pVB;
  int x3, x4, dmL, dmR;
  int ybot;
  int cmd;

  VBTRACEACCEL("SubsequentFillTrapezoidSolid ytop=%d height=%d left=%d dxL=%d dyL=%d eL=%d right=%d dxR=%d dyR=%d eR=%d\n",
         ytop, height, left, dxL, dyL, eL, right, dxR, dyR, eR);
  pVB = VBPTR();

  if(dyL == 0 || dyR == 0 || height == 0) {
    return;
  }

  x3 = (left+height*dxL/dyL);
  x4 = (++right+height*dxR/dyR);
  if (dxL<0) dxL=-dxL;
  if (dxR<0) dxR=-dxR;
  if (dyL<0) dyL=-dyL;
  if (dyR<0) dyR=-dyR;
  if(x3>x4) {
    if(dxL/dyL < dxR/dyR) {
      x4 = x3;
    } else {
      x3 = x4;
    }
  }

  dmL = dxL-eL;
  dmR = dxR-eR;

  if (dxL) dmL--;
  if (dxR) dmL--;

  ybot = ytop+height;
  
  pVB->CurrentCmd = SST_2D_POLYGONFILL;
  cmd = SST_2D_POLYGONFILL | (VBROPCvt[pVB->CurrentROP] << 24);
  
  VBMakeRoom(7);
  VBLaunch(SST_2D_OFFSET+SST_2D_SRCXY, (left&0x1FFF) | ((ytop&0x1FFF)<<16));
  VBLaunch(SST_2D_OFFSET+SST_2D_DSTXY, (right&0x1FFF) | ((ytop&0x1FFF)<<16));
  VBLaunch(SST_2D_OFFSET+SST_2D_BRESERROR0, (dmL&0xFFFF) | BIT(31));
  VBLaunch(SST_2D_OFFSET+SST_2D_BRESERROR1, (dmR&0xFFFF) | BIT(31));
  pVB->ErrorSet=TRUE;
  VBLaunch(SST_2D_OFFSET+SST_2D_COMMAND, cmd);
  VBLaunch(SST_2D_OFFSET+SST_2D_LAUNCH, (x3&0x1FFF) | ((ybot&0x1FFF)<<16));
  VBLaunch(SST_2D_OFFSET+SST_2D_LAUNCH, (x4&0x1FFF) | ((ybot&0x1FFF)<<16));
  pVB->BltPrevY=ytop;
}

void VBImageWrite(int x, int y, int w, int h, void *src, int srcwidth,
                  int rop, unsigned int planemask)
{
  VBPtr pVB;
  int cmd, fmt;
  int i, srcmax;
  
  VBTRACEACCEL("ImageWrite x=%d y=%d w=%d h=%d srcwidth=%d rop=%d\n",
         x, y, w, h, srcwidth, rop);
  pVB = VBPTR();
  
  cmd=SST_2D_HOSTTOSCRNBLIT|(VBROPCvt[rop]<<24);
  
  if (pVB->cpp==1) fmt=1; else fmt=pVB->cpp+1;

  srcmax = h*srcwidth/4;
  
  VBMakeRoom(6);
  VBLaunch(SST_2D_OFFSET+SST_2D_SRCFORMAT, srcwidth & 0x3FFF | (fmt<<16));
  VBLaunch(SST_2D_OFFSET+SST_2D_DSTFORMAT, pVB->stride | (fmt<<16));
  VBLaunch(SST_2D_OFFSET+SST_2D_SRCXY, 0);
  VBLaunch(SST_2D_OFFSET+SST_2D_DSTXY, (x&0x1FFF) | ((y&0x1FFF)<<16));
  VBLaunch(SST_2D_OFFSET+SST_2D_DSTSIZE, (w&0x1FFF)|((h&0x1FFF)<<16));
  VBLaunch(SST_2D_OFFSET+SST_2D_COMMAND, cmd);
  for(i=0; i<srcmax; i++) {
    VBMakeRoom(1);
    VBLaunch(SST_2D_OFFSET+SST_2D_LAUNCH, *((int *)src+i));
  }
  pVB->BltPrevY=y;
}

static Bool
LoadTilePattern(PixmapPtr pPixmap)
{
  /* Make the tile 8x8 and load it into the pattern registers if possible.
   * Adapted from ReduceTileToSize8 and Write8x8Pattern in
   * xc/programs/Xserver/hw/xfree86/xaa/xf86pcache.c
   */
  VBPtr pVB;
  int w, h;
  unsigned char *srcp, *bufp;
  unsigned char buf[256];
  int y, nh, bytespp, bytesPerRow, nReg;
  w = pPixmap->drawable.width;
  h = pPixmap->drawable.height;
  bytespp = pPixmap->drawable.bitsPerPixel / 8;

  VBTRACEACCEL("LoadTilePattern w=%d h=%d bytespp=%d\n", w, h, bytespp);
  
  /* w and h must be power of 2. */
  if ((w & (w - 1)) || (h & (h - 1)))
    return FALSE;
  /* w and h must be reducible to 8. */
  if ((w > 8 && w != 16 && w != 32) || (h > 8 && h != 16 && h != 32))
    return FALSE;
  srcp = pPixmap->devPrivate.ptr;
  /* Make sure that the tile is horizontally repeated. */
  if (w > 8) {
    int i, j, bytewidth8, bytewidth8_times_i;
    bytewidth8 = bytespp * 8;
    /* gcc seems too stupid to eliminate mul in inner loop. */
    bytewidth8_times_i = bytewidth8;
    /* For each extra horizontal repetition. */
    for (i = 1; i < w / 8; i++) {
      /* For each word in the horizontal repetion. */
      for (j = 0; j < bytewidth8; j += 4)
        /* Check that the i-th repetion matches the first. */
        if (*(unsigned int *)(srcp + j) !=
            *(unsigned int *)(srcp + bytewidth8_times_i + j))
          return FALSE;
      bytewidth8_times_i += bytewidth8;
    }
  }
  srcp += pPixmap->devKind;
  /*
   * When the height is 16 or 32, check whether the whole tile is
   * repeated vertically.
   */
  for (y = 8; y < h; y++) {
    int i;
    unsigned char *srcp2;
    /* Calculate pointer to top part of tile to compare to. */
    srcp2 = (unsigned char *)(pPixmap->devPrivate.ptr) +
            pPixmap->devKind * (y & 7);
    switch (w * bytespp) {
      case 1 :
        if (*(unsigned char *)(srcp) != *(unsigned char *)srcp2)
          return FALSE;
        break;
      case 2 :
        if (*(unsigned short *)(srcp) != *(unsigned short *)srcp2)
          return FALSE;
        break;
      default : /* width 4, 8, 16 or 32 at 8bpp */
        for (i = 0; i < w * bytespp; i += 4)
          if (*(unsigned int *)(srcp + i) != *(unsigned int *)(srcp2 + i))
            return FALSE;
        break;
    }
    srcp += pPixmap->devKind;
  }
  w = min(w , 8);
  h = min(h , 8);
  bufp = buf;
  srcp = pPixmap->devPrivate.ptr;
  bytesPerRow = w * bytespp;
  /* Expand horizontally and swap bits. */
  for (y = 0; y < h; y++) {
    int i, nw;
    for(i=0; i<bytesPerRow; i++) {
      *(bufp+i) = *(srcp+i);
    }
    nw = w;
    while (nw != 8) {
      memcpy(bufp + nw * bytespp, bufp, nw * bytespp);
      nw *= 2;
    }
    srcp += pPixmap->devKind;
    bufp += 8 * bytespp;
  }
  nh = h;
  /* Expand vertically. */
  while (nh != 8) {
    memcpy(buf + nh * 8 * bytespp, buf, nh * 8 * bytespp);
    nh *= 2;
  }
  /* Now write the pattern registers */
  pVB = VBPTR();
  nReg = 16*bytespp;
  for(y=0; y<nReg*4; y+=4) {
    if(!((y/4)&0xF)) {
      VBMakeRoom(16);
    }
    VBLaunch(SST_2D_OFFSET+SST_2D_PATTERN0+y, *(int *)&buf[y]);
  }
  return TRUE;
}

static Bool
LoadStipplePattern(PixmapPtr pPixmap)
{
  /* Make the stipple 8x8 and load it into the pattern registers if possible.
   * Adapted from ReduceStippleToSize8 and ExpandStippleTo8x8MonoPattern in
   * xc/programs/Xserver/hw/xfree86/xaa/xf86pcache.c
   */
  VBPtr pVB;
  int w, h;
  int y, nh;
  unsigned char *srcp;
  unsigned int bits[8];
  unsigned char pattern[8];
  w = pPixmap->drawable.width;
  h = pPixmap->drawable.height;

  VBTRACEACCEL("LoadStipplePattern w=%d h=%d\n", w, h);

  /* w and h must be power of 2. */
  if ((w & (w - 1)) || (h & (h - 1)))
    return FALSE;
  /* w and h must be reducible to 8. */
  if ((w > 8 && w != 16 && w != 32) || (h > 8 && h != 16 && h != 32))
    return FALSE;
  if(w > 8 || h > 8) {
    srcp = pPixmap->devPrivate.ptr;
    for (y = 0; y < min(8, h); y++) {
      switch ((w + 7) / 8) {
        case 1 : /* width 1, 2, 4 or 8 */
          bits[y] = *(unsigned char *)srcp;
          break;
        case 2 : /* width 16 */
          bits[y] = *(unsigned short *)srcp;
          if ((bits[y] & 0x00FF) != ((bits[y] & 0xFF00) >> 8))
            return FALSE;
          break;
        case 4 : /* width 32 */
          bits[y] = *(unsigned int *)srcp;
          if ((bits[y] & 0x000000FF) != ((bits[y] & 0x0000FF00) >> 8))
            return FALSE;
          if ((bits[y] & 0x000000FF) != ((bits[y] & 0x00FF0000) >> 16))
            return FALSE;
          if ((bits[y] & 0x000000FF) != ((bits[y] & 0xFF000000) >> 24))
            return FALSE;
      }
      srcp += pPixmap->devKind;
    }
  }
  for (y = 8; y < h; y++) {
    switch ((w + 7) / 8) {
      case 1 : /* width 1, 2, 4 or 8 */
        if (*(unsigned char *)srcp != bits[y & 7])
          return FALSE;
        break;
      case 2 : /* width 16 */
        if (*(unsigned short *)srcp != bits[y & 7])
          return FALSE;
        break;
      case 4 : /* width 32 */
        if (*(unsigned int *)srcp != bits[y & 7])
          return FALSE;
        break;
    }
    srcp += pPixmap->devKind;
  }
  w = min(w , 8);
  h = min(h , 8);
  /* Expand horizontally and swap bits. */
  for (y = 0; y < h; y++) {
    if (w < 8) {
       int j;
       pattern[y] = bits[y] & ((1 << w) - 1);
       for (j = w; j < 8; j += w)
         pattern[y] |= pattern[y] << j;
    } else {
        pattern[y] = bits[y] & 0xFF;
    }
    pattern[y] = byte_reversed[pattern[y]];
  }
  nh = h;
  /* Expand vertically. */
  while (nh != 8) {
    for (y = 0; y < nh; y++)
      pattern[nh + y] = pattern[y];
    nh *= 2;
  }
  pVB = VBPTR();
  VBMakeRoom(2);
  VBLaunch(SST_2D_OFFSET+SST_2D_PATTERN0, *(int *)pattern);
  VBLaunch(SST_2D_OFFSET+SST_2D_PATTERN1, *(int *)&pattern[4]);
  return TRUE;
}

void
VBPolyFillRect(DrawablePtr pDrawable, GCPtr pGC, int nrectFill,
               xRectangle *prectInit)
{
  xRectangle *prect;
  RegionPtr prgnClip;
  cfbPrivGC *priv;
  BoxPtr pextent;
  int xorg, yorg;
  VBPtr pVB;
  int cmd, fmt;

  pVB = VBPTR();
  
  VBTRACEACCEL("VBPolyFillRect nrectFill=%d fillstyle=%d fg=%x bg=%x rop=%x",
         nrectFill, pGC->fillStyle, pGC->fgPixel, pGC->bgPixel, pGC->alu);

  if (nrectFill <= 0) {
    VBTRACEACCEL("\n");
    return;
  }
  
  priv = cfbGetGCPrivate(pGC);
  prgnClip = priv->pCompositeClip;

  VBTRACEACCEL(" nregions=%d\n", REGION_NUM_RECTS(prgnClip));

  if (REGION_NUM_RECTS(prgnClip) > 1) {
    /* Can't handle multiple clipping regions in hardware,
       fall back to software clipping. */
    VBTRACEACCEL("VBPolyFillRect fall back to software clipping\n");
    xf86PolyFillRect(pDrawable, pGC, nrectFill, prectInit);
    return;
  }

  cmd=0;
  switch (pGC->fillStyle) {
    case FillSolid:
      cmd |= (VBROPCvt[pGC->alu]<<24);
      break;
      
    case FillTiled:
      if(!LoadTilePattern(pGC->tile.pixmap)) {
        VBTRACEACCEL("VBPolyFillRect fall back to xf86PolyFillRect for FillTiled\n");
        xf86PolyFillRect(pDrawable, pGC, nrectFill, prectInit);
        return;
      }
      
      cmd |= (VBROPCvt[pGC->alu+ROP_PATTERN_OFFSET]<<24) |
             ((pGC->patOrg.x + pDrawable->x) & 0x07)<<17 |
             ((pGC->patOrg.y + pDrawable->y) & 0x07)<<20;
      break;
      
    case FillStippled:
      cmd |= SST_2D_TRANSPARENT_MONOCHROME;
      /* fall through */
      
    case FillOpaqueStippled:
      if(!LoadStipplePattern(pGC->stipple)) {
        VBTRACEACCEL("VBPolyFillRect fall back to xf86PolyFillRect for Fill(Opaque)Stippled\n");
        xf86PolyFillRect(pDrawable, pGC, nrectFill, prectInit);
        return;
      }
      cmd |= (VBROPCvt[pGC->alu+ROP_PATTERN_OFFSET]<<24) |
             SST_2D_MONOCHROME_PATTERN |
             ((pGC->patOrg.x + pDrawable->x) & 0x07)<<17 |
             ((pGC->patOrg.y + pDrawable->y) & 0x07)<<20;
      if(pGC->fillStyle == FillOpaqueStippled) {
        VBMakeRoom(1);
        VBLaunch(SST_2D_OFFSET+SST_2D_COLORBACK, pGC->bgPixel);
      }
      break;
  }
  
  prect = prectInit;
  xorg = pDrawable->x;
  yorg = pDrawable->y;
  if (xorg || yorg) {
    int n;
    n = nrectFill;
    prect = prectInit;
    while(n--) {
      prect->x += xorg;
      prect->y += yorg;
      ++prect;
    }
  }

  if (REGION_NUM_RECTS(prgnClip) == 1) {
    pextent = REGION_RECTS(prgnClip);
  } else {
    pextent = REGION_EXTENTS(pGC->pScreen, prgnClip);
  }
  VBSetClippingRectangle(pextent->x1,   pextent->y1,
                         pextent->x2-1, pextent->y2-1);
  
  cmd |= SST_2D_RECTANGLEFILL | (pVB->ClipSelect<<23);
  
  if (pVB->cpp==1) fmt=1; else fmt=pVB->cpp+1;
  
  VBMakeRoom(3);
  VBLaunch(SST_2D_OFFSET+SST_2D_DSTFORMAT, pVB->stride | (fmt<<16));
  VBLaunch(SST_2D_OFFSET+SST_2D_COLORFORE, pGC->fgPixel);
  VBLaunch(SST_2D_OFFSET+SST_2D_COMMAND, cmd);

  prect = prectInit;
  while (nrectFill--) {
    if (prect->width > 0 && prect->height > 0) {
      VBMakeRoom(2);
      VBLaunch(SST_2D_OFFSET+SST_2D_DSTSIZE,
               (prect->width&0x1FFF) | ((prect->height&0x1FFF)<<16));
      VBLaunch(SST_2D_OFFSET+SST_2D_LAUNCH,
               (prect->x&0x1FFF) | ((prect->y&0x1FFF)<<16));
    }
    ++prect;
  }
  pVB->ClipSelect=0;
  VBIdle();
}

/* For performance: if only one box, see if it needs to be cached. */
/* Don't cache if area being filled is smaller than the pattern.   */

#define DO_CACHE_PATTERN(nBox, pBox, pPixmap)                     \
    (!(((nBox) == 1) &&                                           \
       ((pBox)->x2 - (pBox)->x1 <= (pPixmap)->drawable.width) &&  \
       ((pBox)->y2 - (pBox)->y1 <= (pPixmap)->drawable.height)))

void
VBFillRect(DrawablePtr pDrawable, GCPtr pGC, int nBox, BoxPtr pBoxInit)
{
  BoxPtr pBox;
  VBPtr pVB;
  int cmd, fmt;

  VBTRACEACCEL("VBFillRect fillstyle=%d color=%x rop=%x nBox=%d\n",
         pGC->fillStyle, pGC->fgPixel, pGC->alu, nBox);
  
  pVB = VBPTR();
  
  if (pVB->cpp==1) fmt=1; else fmt=pVB->cpp+1;
  
  cmd = SST_2D_RECTANGLEFILL;

  switch (pGC->fillStyle) {
    case FillSolid:
      cmd |= (VBROPCvt[pGC->alu]<<24);
      break;
      
    case FillTiled:
      if(!LoadTilePattern(pGC->tile.pixmap)) {
        if((xf86AccelInfoRec.Flags & PIXMAP_CACHE) &&
           DO_CACHE_PATTERN(nBox, pBoxInit, pGC->tile.pixmap) &&
           xf86CacheTile(pGC->tile.pixmap)) {
          VBTRACEACCEL("VBFillRect fall back to cache for FillTiled\n");
            xf86FillRectTileCached(pDrawable, pGC, nBox, pBoxInit);
            return;
        } else {
          /* Darn! Can't do this in hardware at all, fall back to software. */
          VBTRACEACCEL("VBFillRect fall back to software FillTiled\n");
          (*xf86AccelInfoRec.FillRectTiledFallBack)
            (pDrawable, pGC, nBox, pBoxInit);
          return;
        }
      }
      
      cmd |= (VBROPCvt[pGC->alu+ROP_PATTERN_OFFSET]<<24) |
             ((pGC->patOrg.x + pDrawable->x) & 0x07)<<17 |
             ((pGC->patOrg.y + pDrawable->y) & 0x07)<<20;
      break;
      
    case FillStippled:
      cmd |= SST_2D_TRANSPARENT_MONOCHROME;
      /* fall through */
      
    case FillOpaqueStippled:
      if(!LoadStipplePattern(pGC->stipple)) {
        if((xf86AccelInfoRec.Flags & PIXMAP_CACHE) &&
           DO_CACHE_PATTERN(nBox, pBoxInit, pGC->stipple) &&
           xf86CacheStipple(pDrawable, pGC)) {
          VBTRACEACCEL("VBFillRect fall back to cache for Fill(Opaque)Stippled\n");
          xf86FillRectTileCached(pDrawable, pGC, nBox, pBoxInit);
          return;
        } else {
          VBTRACEACCEL("VBFillRect fall back to color expand for (Opaque)Stippled\n");
          xf86FillRectStippledCPUToScreenColorExpand(pDrawable, pGC, nBox, pBoxInit);
          return;
        }
      }
      cmd |= (VBROPCvt[pGC->alu+ROP_PATTERN_OFFSET]<<24) |
             SST_2D_MONOCHROME_PATTERN |
             ((pGC->patOrg.x + pDrawable->x) & 0x07)<<17 |
             ((pGC->patOrg.y + pDrawable->y) & 0x07)<<20;
      if(pGC->fillStyle == FillOpaqueStippled) {
        VBMakeRoom(1);
        VBLaunch(SST_2D_OFFSET+SST_2D_COLORBACK, pGC->bgPixel);
      }
      break;
  }
  VBMakeRoom(3);
  VBLaunch(SST_2D_OFFSET+SST_2D_DSTFORMAT, pVB->stride | (fmt<<16));
  VBLaunch(SST_2D_OFFSET+SST_2D_COLORFORE, pGC->fgPixel);
  VBLaunch(SST_2D_OFFSET+SST_2D_COMMAND, cmd);
  
  pBox = pBoxInit;
  while (nBox > 0) {
    int w, h;
    w = pBox->x2 - pBox->x1;
    h = pBox->y2 - pBox->y1;
    if (w > 0 && h > 0) {
      VBMakeRoom(2);
      VBLaunch(SST_2D_OFFSET+SST_2D_DSTSIZE, (w&0x1FFF) | ((h&0x1FFF)<<16));
      VBLaunch(SST_2D_OFFSET+SST_2D_LAUNCH,
               (pBox->x1&0x1FFF) | ((pBox->y1&0x1FFF)<<16));
    }
    pBox++;
    nBox--;
  }
  VBIdle();
}

void VBFillPolygonSolid(DrawablePtr pDrawable, GCPtr pGC, int shape, int mode,
                        int count, DDXPointPtr ptsIn)
{
  cfbPrivGCPtr devPriv;
  int i;
  Bool ccw;
  int cmd, fmt;
  DDXPointPtr pt, pt2;
  DDXPointPtr lpt, rpt;
  BoxPtr clip;
  VBPtr pVB;

  VBTRACEACCEL("VBFillPolygonSolid shape=%d mode=%d count=%d\n", shape, mode, count);
  devPriv = cfbGetGCPrivate(pGC);
  if (shape != Convex  || REGION_NUM_RECTS(devPriv->pCompositeClip) != 1) {
    VBTRACEACCEL("VBFillPolygonSolid fallback\n");
    xf86FillPolygonSolid1Rect(pDrawable, pGC, shape, mode, count, ptsIn);
    return;
  }
  
  pVB = VBPTR();
  
  clip = &devPriv->pCompositeClip->extents;
  VBSetClippingRectangle(clip->x1, clip->y1, clip->x2-1, clip->y2-1);
  
  pt = ptsIn;
  if (mode == CoordModeOrigin) {
    for (i=0; i<count; i++, pt++) {
      pt->x += pDrawable->x;
      pt->y += pDrawable->y;
    }
  } else {
    pt->x += pDrawable->x;
    pt++->y += pDrawable->y;
    for (i = 1; i<count; i++, pt++) {
      pt->x += (pt-1)->x;
      pt->y += (pt-1)->y;
    }
  }

  /* Find the leftmost point at the minimum y value. */
  lpt = pt = ptsIn;
  for(i=1, pt++; i<count; i++, pt++) {
    if(pt->y < lpt->y || (pt->y == lpt->y && pt->x == lpt->x)) {
      lpt = pt;
    }
  }
  pt2 = lpt+1;
  if(pt2 == ptsIn+count) {
    pt2 = ptsIn;
  }
  pt = lpt-1;
  if(pt < ptsIn) {
    pt = ptsIn+count-1;
  }
  ccw = pt->x > pt2->x;
  /* Find the rightmost point at the minimum y value. */
  pt = lpt;
  do {
    rpt = pt;
    if(ccw) {
      if(--pt < ptsIn) {
        pt = ptsIn+count-1;
      }
    } else {
      if(++pt == ptsIn+count) {
        pt = ptsIn;
      }
    }
  } while(pt->y == lpt->y && pt->x > lpt->x);
  
  if (pVB->cpp==1) fmt=1; else fmt=pVB->cpp+1;
  
  cmd = SST_2D_POLYGONFILL |
        (pVB->ClipSelect<<23) |
        (VBROPCvt[pGC->alu]<<24);
  
  VBMakeRoom(4);
  VBLaunch(SST_2D_OFFSET+SST_2D_DSTFORMAT, pVB->stride | (fmt<<16));
  VBLaunch(SST_2D_OFFSET+SST_2D_COLORFORE, pGC->fgPixel);
  VBLaunch(SST_2D_OFFSET+SST_2D_SRCXY, (lpt->x&0x1FFF) | ((lpt->y&0x1FFF)<<16));
  VBLaunch(SST_2D_OFFSET+SST_2D_DSTXY, (rpt->x&0x1FFF) | ((rpt->y&0x1FFF)<<16));
  VBLaunch(SST_2D_OFFSET+SST_2D_COMMAND, cmd);
  for(;;) {
    if(ccw) {
      pt = lpt+1;
      if(pt == ptsIn+count) {
        pt = ptsIn;
      }
      pt2 = rpt-1;
      if(pt2 < ptsIn) {
        pt2 = ptsIn+count-1;
      }
    } else {
      pt = lpt-1;
      if(pt < ptsIn) {
        pt = ptsIn+count-1;
      }
      pt2 = rpt+1;
      if(pt2 == ptsIn+count) {
        pt2 = ptsIn;
      }
    }
    if(pt == rpt) break;
    
    VBMakeRoom(1);
    if(lpt->y <= rpt->y) {
      lpt = pt;
      VBLaunch(SST_2D_OFFSET+SST_2D_LAUNCH,
               (lpt->x&0x1FFF) | ((lpt->y&0x1FFF)<<16));
    } else {
      rpt = pt2;
      VBLaunch(SST_2D_OFFSET+SST_2D_LAUNCH,
               (rpt->x&0x1FFF) | ((rpt->y&0x1FFF)<<16));
    }
  }
  /* Make sure the left & right sides meet */
  VBMakeRoom(1);
  VBLaunch(SST_2D_OFFSET+SST_2D_LAUNCH,
           (lpt->x&0x1FFF) | ((lpt->y&0x1FFF)<<16));

  pVB->ClipSelect = 0;
  VBIdle();
}

void
VBPolyLine(DrawablePtr pDrawable, GCPtr pGC, int mode, int npt, DDXPointPtr ppt)
{
  VBPtr pVB;
  BoxPtr pbox;
  int xorg, yorg;
  RegionPtr cclip;
  cfbPrivGCPtr devPriv;
  int linestipple, i;
  int cmd, fmt;
  int x1, y1, x2, y2;
  int PatternLength;
  
  pVB = VBPTR();
  
  VBTRACEACCEL("VBPolyLine mode=%d npt=%d, linestyle=%d capstyle=%d\n",
	       mode, npt, pGC->lineStyle, pGC->capStyle);
  devPriv = cfbGetGCPrivate(pGC);
  cclip = devPriv->pCompositeClip;
  
  if(REGION_NUM_RECTS(cclip) != 1) {
    VBTRACEACCEL("VBPolyLine multiple clip regions, use software clipping\n");
    /* Fall back to software clipping for multiple clip regions */
    if(pGC->lineStyle == LineSolid) {
      xf86PolyLine(pDrawable, pGC, mode, npt, ppt);
    } else {
      xf86PolyDashedLine(pDrawable, pGC, mode, npt, ppt);
    }
    return;
  }
  
  if(pGC->lineStyle != LineSolid) {
    if(!(PatternLength = xf86PackDashPattern(pGC))) {
      VBTRACEACCEL("VBPolyLine can't pack pattern, fallback to software\n");
      switch (pVB->cpp) {
        case 8:
          cfbLineSD(pDrawable, pGC, mode, npt, ppt);
          break;
        case 16:
          cfb16LineSD(pDrawable, pGC, mode, npt, ppt);
          break;
        case 24:
          cfb24LineSD(pDrawable, pGC, mode, npt, ppt);
          break;
        case 32:
          cfb32LineSD(pDrawable, pGC, mode, npt, ppt);
          break;
      }
      return;
    }
  }
  
  pbox = REGION_RECTS(cclip);
  VBSetClippingRectangle(pbox->x1, pbox->y1, pbox->x2-1, pbox->y2-1);
  
  xorg = pDrawable->x;
  yorg = pDrawable->y;
  
  cmd = SST_2D_POLYLINE | (pVB->ClipSelect<<23) | (VBROPCvt[pGC->alu]<<24);
  
  if(pGC->lineStyle == LineOnOffDash) {
    cmd |= SST_2D_TRANSPARENT_MONOCHROME | SST_2D_STIPPLE_LINE;
  } else if(pGC->lineStyle == LineDoubleDash) {
    cmd |= SST_2D_STIPPLE_LINE;
    VBMakeRoom(1);
    VBLaunch(SST_2D_OFFSET+SST_2D_COLORBACK, pGC->bgPixel);
  }
  
  if (pGC->lineStyle != LineSolid) {
    /* X Starts the pattern with the MSB, 3dfx starts with the LSB.
     * Swap the bit order.
     */
    linestipple=0;
    for(i=0; i<PatternLength; ++i) {
      if(pVB->LinePatternBuffer & BIT(i)) {
    	  linestipple |= BIT(PatternLength-i-1);
      }
    }

    VBMakeRoom(2);
    VBLaunch(SST_2D_OFFSET+SST_2D_LINESTIPPLE, linestipple);
    VBLaunch(SST_2D_OFFSET+SST_2D_LINESTYLE, ((PatternLength-1)<<8) |
             (((pGC->dashOffset%PatternLength)&0x1F)<<24));
  }
  
  if (pVB->cpp==1) fmt=1; else fmt=pVB->cpp+1;
  
  x2 = ppt->x+xorg;
  y2 = ppt->y+yorg;
  
  VBMakeRoom(4);
  VBLaunch(SST_2D_OFFSET+SST_2D_DSTFORMAT, pVB->stride | (fmt<<16));
  VBLaunch(SST_2D_OFFSET+SST_2D_COLORFORE, pGC->fgPixel);
  VBLaunch(SST_2D_OFFSET+SST_2D_SRCXY, (x2&0x1FFF) | ((y2&0x1FFF)<<16));
  if (pVB->ErrorSet) {
    VBMakeRoom(1);
    VBLaunch(SST_2D_OFFSET+SST_2D_BRESERROR0, 0);
    pVB->ErrorSet=FALSE;
  }
  VBLaunch(SST_2D_OFFSET+SST_2D_COMMAND, cmd);
  
  while(--npt>1) {
    ++ppt;
    x1 = x2;
    y1 = y2;
    if(mode == CoordModePrevious) {
      x2 += ppt->x;
      y2 += ppt->y;
    } else {
      x2 = ppt->x+xorg;
      y2 = ppt->y+yorg;
    }
    VBMakeRoom(1);
    VBLaunch(SST_2D_OFFSET+SST_2D_LAUNCH, (x2&0x1FFF) | ((y2&0x1FFF)<<16));
  }
  
  if (pGC->capStyle != CapNotLast) {
    cmd = SST_2D_LINE | (pVB->ClipSelect<<23) | (VBROPCvt[pGC->alu]<<24);
    if (pGC->lineStyle == LineOnOffDash) {
      cmd |= SST_2D_TRANSPARENT_MONOCHROME | SST_2D_STIPPLE_LINE;
    } else if (pGC->lineStyle == LineDoubleDash) {
      cmd |= SST_2D_STIPPLE_LINE;
    }
    VBIdle();
    if (pGC->lineStyle != LineSolid) {
      VBMakeRoom(2);
      VBLaunch(SST_2D_OFFSET+SST_2D_LINESTIPPLE, linestipple);
      VBLaunch(SST_2D_OFFSET+SST_2D_LINESTYLE, ((PatternLength-1)<<8) |
	       (((pGC->dashOffset%PatternLength)&0x1F)<<24));
    }
    VBMakeRoom(4);
    VBLaunch(SST_2D_OFFSET+SST_2D_DSTFORMAT, pVB->stride | (fmt<<16));
    VBLaunch(SST_2D_OFFSET+SST_2D_COLORFORE, pGC->fgPixel);
    VBLaunch(SST_2D_OFFSET+SST_2D_SRCXY, (x2&0x1FFF) | ((y2&0x1FFF)<<16));
    VBLaunch(SST_2D_OFFSET+SST_2D_COMMAND, cmd);
  }
  
  ++ppt;
  x1 = x2;
  y1 = y2;
  if(mode == CoordModePrevious) {
    x2 += ppt->x;
    y2 += ppt->y;
  } else {
    x2 = ppt->x+xorg;
    y2 = ppt->y+yorg;
  }
  VBMakeRoom(1);
  VBLaunch(SST_2D_OFFSET+SST_2D_LAUNCH, (x2&0x1FFF) | ((y2&0x1FFF)<<16));
  
  pVB->ClipSelect=0;
  VBIdle();
}

void
VBPolySegment(DrawablePtr pDrawable, GCPtr pGC, int nseg, xSegment *pSeg)
{
  VBPtr pVB;
  BoxPtr pbox;
  int xorg, yorg;
  RegionPtr cclip;
  cfbPrivGCPtr devPriv;
  int cmd, fmt;
  int linestyle, PatternLength;
  
  pVB = VBPTR();
  
  VBTRACEACCEL("VBPolySegment nseg=%d, linestyle=%d\n", nseg, pGC->lineStyle);
  devPriv = cfbGetGCPrivate(pGC);
  cclip = devPriv->pCompositeClip;
  
  if(REGION_NUM_RECTS(cclip) != 1) {
    VBTRACEACCEL("VBPolySegment multiple clip regions, use software clipping\n");
    /* Fall back to software clipping for multiple clip regions */
    if(pGC->lineStyle == LineSolid) {
      xf86PolySegment(pDrawable, pGC, nseg, pSeg);
    } else {
      xf86PolyDashedSegment(pDrawable, pGC, nseg, pSeg);
    }
    return;
  }
  
  if(pGC->lineStyle != LineSolid) {
    if(!(PatternLength = xf86PackDashPattern(pGC))) {
      VBTRACEACCEL("VBPolySegment can't pack pattern, fallback to software\n");
      switch (pVB->cpp) {
        case 8:
          cfbSegmentSD(pDrawable, pGC, nseg, pSeg);
          break;
        case 16:
          cfb16SegmentSD(pDrawable, pGC, nseg, pSeg);
          break;
        case 24:
          cfb24SegmentSD(pDrawable, pGC, nseg, pSeg);
          break;
        case 32:
          cfb32SegmentSD(pDrawable, pGC, nseg, pSeg);
          break;
      }
      return;
    }
  }
  
  pbox = REGION_RECTS(cclip);
  VBSetClippingRectangle(pbox->x1, pbox->y1, pbox->x2-1, pbox->y2-1);
  
  xorg = pDrawable->x;
  yorg = pDrawable->y;
  
  if(pGC->capStyle == CapNotLast) {
    cmd = SST_2D_POLYLINE | (pVB->ClipSelect<<23) | (VBROPCvt[pGC->alu]<<24);
  } else {
    cmd = SST_2D_LINE | (pVB->ClipSelect<<23) | (VBROPCvt[pGC->alu]<<24);
  }
  
  if(pGC->lineStyle == LineOnOffDash) {
    cmd |= SST_2D_TRANSPARENT_MONOCHROME | SST_2D_STIPPLE_LINE;
  } else if(pGC->lineStyle == LineDoubleDash) {
    cmd |= SST_2D_STIPPLE_LINE;
    VBMakeRoom(1);
    VBLaunch(SST_2D_OFFSET+SST_2D_COLORBACK, pGC->bgPixel);
  }
  
  if(pGC->lineStyle != LineSolid) {
    int linestipple, i;
    /* X Starts the pattern with the MSB, 3dfx starts with the LSB.
     * Swap the bit order.
     */
    linestipple=0;
    for(i=0; i<PatternLength; ++i) {
      if(pVB->LinePatternBuffer & BIT(i)) {
    	  linestipple |= BIT(PatternLength-i-1);
      }
    }

    linestyle = ((PatternLength-1)<<8) |
                (((pGC->dashOffset%PatternLength)&0x1F)<<24);
    VBMakeRoom(1);
    VBLaunch(SST_2D_OFFSET+SST_2D_LINESTIPPLE, linestipple);
  }
  
  if (pVB->cpp==1) fmt=1; else fmt=pVB->cpp+1;
  
  VBMakeRoom(2);
  VBLaunch(SST_2D_OFFSET+SST_2D_DSTFORMAT, pVB->stride | (fmt<<16));
  VBLaunch(SST_2D_OFFSET+SST_2D_COLORFORE, pGC->fgPixel);
  if (pVB->ErrorSet) {
    VBMakeRoom(1);
    VBLaunch(SST_2D_OFFSET+SST_2D_BRESERROR0, 0);
    pVB->ErrorSet=FALSE;
  }
  
  VBMakeRoom(1);
  VBLaunch(SST_2D_OFFSET+SST_2D_COMMAND, cmd);
  while(nseg--) {
    if(pGC->lineStyle != LineSolid) {
      /* The pattern offset needs to be set for each segment */
      VBMakeRoom(1);
      VBLaunch(SST_2D_OFFSET+SST_2D_LINESTYLE, linestyle);
    }
    VBMakeRoom(2);
    VBLaunch(SST_2D_OFFSET+SST_2D_SRCXY,
             ((pSeg->x1+xorg)&0x1FFF) | (((pSeg->y1+yorg)&0x1FFF)<<16));
    VBLaunch(SST_2D_OFFSET+SST_2D_LAUNCH,
             ((pSeg->x2+xorg)&0x1FFF) | (((pSeg->y2+yorg)&0x1FFF)<<16));
    ++pSeg;
  }
  
  pVB->ClipSelect=0;
  VBIdle();
}

#endif

Bool
VBNAME(AccelInit)()
{
  VBPtr pVB;
  int lines;

  pVB = VBPTR();
  xf86AccelInfoRec.Flags = 
    BACKGROUND_OPERATIONS |
    PIXMAP_CACHE |
    /* XAA will cache stipples before using the pattern registers if
     * DO_NOT_CACHE_STIPPLES is not used.  Do the caching in VBPolyFillRect
     * and VBFillRect if needed. */
    DO_NOT_CACHE_STIPPLES |
    HARDWARE_CLIP_LINE |
    NO_SYNC_AFTER_CPU_COLOR_EXPAND |
    HARDWARE_PATTERN_MONO_TRANSPARENCY |
    HARDWARE_PATTERN_BIT_ORDER_MSBFIRST |
    HARDWARE_PATTERN_SCREEN_ORIGIN |
    HARDWARE_PATTERN_PROGRAMMED_ORIGIN |
    HARDWARE_PATTERN_PROGRAMMED_BITS |
    HARDWARE_PATTERN_TRANSPARENCY;
  xf86AccelInfoRec.ColorExpandFlags = 
    SCANLINE_PAD_DWORD |
    CPU_TRANSFER_PAD_DWORD |
    VIDEO_SOURCE_GRANULARITY_PIXEL |
    BIT_ORDER_IN_BYTE_MSBFIRST |
    LEFT_EDGE_CLIPPING |
    LEFT_EDGE_CLIPPING_NEGATIVE_X |
    NO_PLANEMASK;
  xf86GCInfoRec.PolyFillRectSolidFlags = NO_PLANEMASK;
  xf86GCInfoRec.PolyFillRectTiledFlags = NO_PLANEMASK;
  xf86GCInfoRec.PolyFillRectStippledFlags = NO_PLANEMASK;
  xf86GCInfoRec.PolyFillRectOpaqueStippledFlags = NO_PLANEMASK;
  xf86GCInfoRec.CopyAreaFlags = NO_PLANEMASK;
  xf86GCInfoRec.FillPolygonSolidFlags = NO_PLANEMASK | ONE_RECT_CLIPPING;
  xf86GCInfoRec.PolyLineSolidZeroWidthFlags = NO_PLANEMASK | ONE_RECT_CLIPPING;
  xf86GCInfoRec.PolyLineDashedZeroWidthFlags = NO_PLANEMASK | ONE_RECT_CLIPPING;
  xf86GCInfoRec.PolySegmentSolidZeroWidthFlags =
    NO_PLANEMASK | ONE_RECT_CLIPPING;
  xf86GCInfoRec.PolySegmentDashedZeroWidthFlags =
    NO_PLANEMASK | ONE_RECT_CLIPPING;

  xf86AccelInfoRec.CPUToScreenColorExpandBase =
    (unsigned int *)&REF32(SST_2D_OFFSET+SST_2D_LAUNCH);
  xf86AccelInfoRec.CPUToScreenColorExpandRange = 128;
  xf86AccelInfoRec.LinePatternBuffer = &pVB->LinePatternBuffer;
  xf86AccelInfoRec.LinePatternMaxLength = 32;
  xf86AccelInfoRec.ErrorTermBits = 16;

  /* General Functions */
  xf86AccelInfoRec.Sync = VBIdle;
  xf86AccelInfoRec.SetClippingRectangle = VBSetClippingRectangle;
  xf86AccelInfoRec.ImageWrite = VBImageWrite;

  /* Blits */
  xf86AccelInfoRec.SetupForScreenToScreenCopy = VBSetupForScreenToScreenCopy;
  xf86AccelInfoRec.SubsequentScreenToScreenCopy =
    VBSubsequentScreenToScreenCopy;
  xf86AccelInfoRec.SetupForCPUToScreenColorExpand =
    VBSetupForCPUToScreenColorExpand;
  xf86AccelInfoRec.SubsequentCPUToScreenColorExpand =
    VBSubsequentCPUToScreenColorExpand;
  xf86AccelInfoRec.SetupForScreenToScreenColorExpand =
    VBSetupForScreenToScreenColorExpand;
  xf86AccelInfoRec.SubsequentScreenToScreenColorExpand =
    VBSubsequentScreenToScreenColorExpand;
  xf86AccelInfoRec.SetupFor8x8PatternColorExpand =
    VBSetupFor8x8PatternColorExpand;
  xf86AccelInfoRec.Subsequent8x8PatternColorExpand =
    VBSubsequent8x8PatternColorExpand;

  /* Filled Shapes */
  xf86AccelInfoRec.SetupForFillRectSolid = VBSetupForFillRectSolid;
  xf86AccelInfoRec.SubsequentFillRectSolid = VBSubsequentFillRectSolid;
#if 0
  /* We can't use this until we figure out error terms */
  xf86AccelInfoRec.SubsequentFillTrapezoidSolid =
    VBSubsequentFillTrapezoidSolid;
#endif
  xf86AccelInfoRec.FillRectSolid = VBFillRect;
  xf86AccelInfoRec.FillRectTiled = VBFillRect;
  xf86AccelInfoRec.FillRectStippled = VBFillRect;
  xf86AccelInfoRec.FillRectOpaqueStippled = VBFillRect;
  xf86GCInfoRec.PolyFillRectSolid = VBPolyFillRect;
  xf86GCInfoRec.PolyFillRectTiled = VBPolyFillRect;
  xf86GCInfoRec.PolyFillRectStippled = VBPolyFillRect;
  xf86GCInfoRec.PolyFillRectOpaqueStippled = VBPolyFillRect;
#if 0
  /* Due to a hardware bug we can't support accelerated polygons correctly */
  xf86GCInfoRec.FillPolygonSolid = VBFillPolygonSolid;
#endif

  /* Lines */
  xf86AccelInfoRec.SubsequentDashedTwoPointLine =
    VBSubsequentDashedTwoPointLine;
  xf86GCInfoRec.PolyLineDashedZeroWidth = VBPolyLine;
  xf86GCInfoRec.PolySegmentDashedZeroWidth = VBPolySegment;
  xf86AccelInfoRec.SubsequentTwoPointLine = VBSubsequentTwoPointLine;
  xf86GCInfoRec.PolyLineSolidZeroWidth = VBPolyLine;
  xf86GCInfoRec.PolySegmentSolidZeroWidth = VBPolySegment;
  
  xf86AccelInfoRec.PixmapCacheMemoryStart = vga256InfoRec.virtualY*pVB->stride;
  lines=min(vga256InfoRec.virtualY*2.5, 4095);
  if (pVB->CursorData/pVB->stride>lines) {
    xf86AccelInfoRec.PixmapCacheMemoryEnd = lines*pVB->stride;
  } else {
    xf86AccelInfoRec.PixmapCacheMemoryEnd = pVB->CursorData;
  }
  
  /* This must be set before any call to VBMakeRoom */
  pVB->PciCnt=REF32(0)&0x1F;

  pVB->ErrorSet=FALSE;

  VBMakeRoom(7);
  /* Define clip0 to be the entire memory region. Use clip1 to clip */
  VBLaunch(SST_2D_OFFSET+SST_2D_CLIP0MIN, 0);
  VBLaunch(SST_2D_OFFSET+SST_2D_CLIP1MIN, 0);
  pVB->maxClip=pVB->CursorData/pVB->stride;
  if (pVB->maxClip>4095) pVB->maxClip=4095;
  pVB->maxClip=(vga256InfoRec.virtualX&0xFFF) | (pVB->maxClip&0xFFF)<<16;
  VBLaunch(SST_2D_OFFSET+SST_2D_CLIP0MAX, pVB->maxClip);
  VBLaunch(SST_2D_OFFSET+SST_2D_CLIP1MAX, pVB->maxClip);
  VBLaunch(SST_2D_OFFSET+SST_2D_COMMANDEXTRA, 0);
  VBLaunch(SST_2D_OFFSET+SST_2D_SRCBASEADDR, 0);
  VBLaunch(SST_2D_OFFSET+SST_2D_DSTBASEADDR, 0);

  return TRUE;
}

#if PSZ == 8
/* We only want this function once in the driver */
Bool
VBAccelInit()
{
  switch (vgaBitsPerPixel) {
  case 8:
    return VB8AccelInit();
  case 16:
    return VB16AccelInit();
  case 24:
    return VB24AccelInit();
  case 32:
    return VB32AccelInit();
  }
  return FALSE;
}

#endif

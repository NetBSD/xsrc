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
/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/p9x00/p9x00DAC.c,v 1.1.2.1 1998/08/25 10:54:14 hohndel Exp $ */

#include "vga.h"
#include "p9x00DAC.h"
#include "p9x00Regs.h"
#include "p9x00Access.h"
#include "p9x00ICD2061A.h"

extern vgaHWCursorRec vgaHWCursor;

extern Bool XAACursorInit();
extern void XAARestoreCursor();
extern void XAAWarpCursor();
extern void XAAQueryBestSize();

void ibm525DisableCursor(void);

void p9x00write_LUT_regs(int start, int n, CARD8 *lut)
{
   register int counter, i;
   
   P9X00_WRITEDAC(DAC_WRITE_ADDR, (CARD8)start);
   for (i=counter=0;counter<n;counter++) {
      P9X00_WRITEDAC(DAC_RAMDAC_DATA, lut[i++]);
      P9X00_WRITEDAC(DAC_RAMDAC_DATA, lut[i++]);
      P9X00_WRITEDAC(DAC_RAMDAC_DATA, lut[i++]);
   }
}

void p9x00read_LUT_regs(int start, int n, CARD8 *lut)
{
   register int counter, i;
   
   P9X00_WRITEDAC(DAC_READ_ADDR,(CARD8)start);
   for (i=counter=0;counter<n;counter++) {
      lut[i++]=P9X00_READDAC(DAC_RAMDAC_DATA);
      lut[i++]=P9X00_READDAC(DAC_RAMDAC_DATA);
      lut[i++]=P9X00_READDAC(DAC_RAMDAC_DATA);
   }
}

Bool allways_use_cursor(void *pScreen)
{
  return TRUE;
}

static void ibm525WriteIndex(unsigned int reg, CARD8 data)
{
   CARD8 temp;
   
   P9X00_WRITEDAC(IBMRGB_INDEX_CONTROL,0x00);
   P9X00_WRITEDAC(IBMRGB_INDEX_LOW,(CARD8)reg);
   P9X00_WRITEDAC(IBMRGB_INDEX_HIGH,(CARD8)(reg>>8));
   P9X00_WRITEDAC(IBMRGB_INDEX_DATA,data);
}

static CARD8 ibm525ReadIndex(unsigned int reg)
{
   P9X00_WRITEDAC(IBMRGB_INDEX_CONTROL,0x00);
   P9X00_WRITEDAC(IBMRGB_INDEX_LOW,(CARD8)reg);
   P9X00_WRITEDAC(IBMRGB_INDEX_HIGH,(CARD8)(reg>>8));
   return P9X00_READDAC(IBMRGB_INDEX_DATA);
}

/* This table converts from bytes per pixel to pixels per DWORD for a IBM 525 
*/
CARD8 ibm525PixelsPerDWORD[5] = {0x02,0x03,0x04,0x05,0x06};

/* IBM 525 Commmand Register Shadows 
*/
CARD8 ibm525MiscReg2;
CARD8 ibm525PixFReg;
CARD8 ibm525Pix8CtlReg;
CARD8 ibm525Pix16CtlReg;
CARD8 ibm525Pix24CtlReg;
CARD8 ibm525Pix32CtlReg;


void ibm525SetPort(vgap9x00Ptr state, Bool SetPort)
{
  /* Select 8-bits per gun in palette, VRAM pixel port, PLL output */

  if (SetPort == P9X_SET_VGAPORT) {
    ibm525DisableCursor();
    ibm525MiscReg2 = 0x00;
  }
  else if (1) /* 6Bit Per LUT */
    ibm525MiscReg2 = 0x41;
  else /* Normal 256 color Hires modes */
    ibm525MiscReg2 = 0x45;


  /* Select Color Depth */
  ibm525PixFReg = ibm525PixelsPerDWORD[state->BpPix];

  if ((state->BpPix==1) && (0)) 
    ibm525Pix8CtlReg = 0x01; /* 8 bit non-palettized */
  else
    ibm525Pix8CtlReg = 0x0; /* 8 bit palettized */

  if ((state->BpPix==2) && (0))
    ibm525Pix16CtlReg = 0xc4; /* 555 direct linear */
  else 
    ibm525Pix16CtlReg = 0xc6; /* 565 direct linear */

  ibm525Pix24CtlReg = 0x1; /* direct color for 24-bpp */
  ibm525Pix32CtlReg = 0x3; /* direct color for 32-bpp */

  ibm525WriteIndex (IBMRGB_misc2,ibm525MiscReg2);
  ibm525WriteIndex (IBMRGB_pix_fmt,ibm525PixFReg);
  ibm525WriteIndex (IBMRGB_8bpp,ibm525Pix8CtlReg);
  ibm525WriteIndex (IBMRGB_16bpp,ibm525Pix16CtlReg);
  ibm525WriteIndex (IBMRGB_24bpp,ibm525Pix24CtlReg );
  ibm525WriteIndex (IBMRGB_32bpp,ibm525Pix32CtlReg );

  /* Allow all bits to go through */
  P9X00_WRITEDAC(DAC_PIXEL_MASK, 0xFF);
}

CARD8 ibm525CalcClock(CARD32 freq) 
{
  CARD32 df;
  CARD32 m;
  
  if (freq<16250000L)
    freq=16250000L;
  if (freq<32250000L) {
    df=0L;
    m=((freq<<2)-65000000L)/1000000L;
  }
  else if (freq<64500000L) {
    df=1L;
    m=((freq<<1)-65000000L)/1000000L;
  }
  else if (freq<129000000L) {
    df=2L;
    m=((freq-65000000L)/1000000L);
  }
  else {
    df=3L;
    m=((freq>>1)-65000000L)/1000000L;
  }
  return ((CARD8)((df<<6)|m));
}


CARD8 divTable[5] = { 0, 6, 4, 6, 2 };


void ibm525SetClock(vgap9x00Ptr state, Bool SetPixClk)
{
  CARD8 DotFreq;
  CARD32 ICDFreq;
  CARD32 MemFreq;

  if (SetPixClk == P9X_SET_MEMCLK) {
    if (P9X_CFG_CLK==P9X_V_ICDCLOCK) {
      MemFreq = icd2061CalcClock(state->memspeed);
      icd2061WriteClock(MemFreq | ICD_MEMCLOCK);
      return;
    }
  }
  else { 
    DotFreq = ibm525CalcClock(state->dotfreq);
    if (P9X_CFG_CLK==P9X_V_ICDCLOCK) {
      ICDFreq = icd2061CalcClock(50000000);  /* DAC ref. frequency */
      icd2061WriteClock(ICDFreq | ICD_CLOCK2);
    }
    ibm525WriteIndex(IBMRGB_pll_ref_div_fix,0x19); /* 2 MHz = 50 MHz / 25 */
  }
  ibm525WriteIndex(IBMRGB_f0,DotFreq);
  ibm525WriteIndex(IBMRGB_pll_ctrl2,0x00); /* use pll-freq 0 */
  ibm525WriteIndex(IBMRGB_pll_ctrl1,0x02); /* use REFCLK */
  ibm525WriteIndex(IBMRGB_dac_op,0x02); /* use fast slew rate */
  ibm525WriteIndex(IBMRGB_misc1,0x01); /* 64Bit VRAM, no VRAM mask */
  if((state->dacdivides) || (state->BpPix==3) )
    ibm525WriteIndex(IBMRGB_misc_clock,0x21 | divTable[state->BpPix]);
  else
    ibm525WriteIndex(IBMRGB_misc_clock,0x21); 
}


void ibm525EnableCursor(void)
{
  int tmp;
  
  /* Enable cursor - X11 mode */
  ibm525WriteIndex(IBMRGB_curs,0x27);
}

void ibm525DisableCursor(void)
{
  ibm525WriteIndex(IBMRGB_curs,0x00);
}

void ibm525LoadCursorImage(void *image, int orgx, int orgy)
{
  register int i,j,b;
  CARD8 *data=(CARD8 *)image;

  ibm525WriteIndex(IBMRGB_curs_array,0x00);
  P9X00_WRITEDAC(IBMRGB_INDEX_CONTROL, 0x01); /* enable auto-inc */

  for (i = 0; i < 1024; i++, data++) {
    P9X00_WRITEDAC(IBMRGB_INDEX_DATA,*data);
  }
  P9X00_WRITEDAC(IBMRGB_INDEX_CONTROL, 0x00); /* disable auto-inc */
}

void ibm525SetCursorPos(int x, int y, int orgx, int orgy)
{
  ibm525WriteIndex(IBMRGB_curs_hot_x,orgx);
  ibm525WriteIndex(IBMRGB_curs_hot_y,orgy);
  ibm525WriteIndex(IBMRGB_curs_xl,(CARD8)x);
  ibm525WriteIndex(IBMRGB_curs_xh,(CARD8)(x>>8));
  ibm525WriteIndex(IBMRGB_curs_yl,(CARD8)y);
  ibm525WriteIndex(IBMRGB_curs_yh,(CARD8)(y>>8));
}

void ibm525SetCursorColor(int fg, int bg)
{
   /* Background color */
   ibm525WriteIndex(IBMRGB_curs_col1_b,fg&0xFF);
   ibm525WriteIndex(IBMRGB_curs_col1_g,(fg&0xFF00)>>8);
   ibm525WriteIndex(IBMRGB_curs_col1_r,(fg&0xFF0000)>>16);

   /* Foreground color */
   ibm525WriteIndex(IBMRGB_curs_col2_b,bg&0xFF);
   ibm525WriteIndex(IBMRGB_curs_col2_g,(bg&0xFF00)>>8);
   ibm525WriteIndex(IBMRGB_curs_col2_r,(bg&0xFF0000)>>16);
}	


void ibm525InitCursor(void)
{
  XAACursorInfoRec.Flags=HARDWARE_CURSOR_TRUECOLOR_AT_8BPP|
                         HARDWARE_CURSOR_SOURCE_MASK_INTERLEAVE|
                         HARDWARE_CURSOR_AND_SOURCE_WITH_MASK|
                         HARDWARE_CURSOR_CHAR_BIT_FORMAT|
                         HARDWARE_CURSOR_PROGRAMMED_BITS|
                         HARDWARE_CURSOR_PROGRAMMED_ORIGIN|
                         USE_HARDWARE_CURSOR;
  XAACursorInfoRec.MaxWidth=64;
  XAACursorInfoRec.MaxHeight=64;
  XAACursorInfoRec.SetCursorColors=ibm525SetCursorColor;
  XAACursorInfoRec.SetCursorPosition=ibm525SetCursorPos;
  XAACursorInfoRec.LoadCursorImage=ibm525LoadCursorImage;
  XAACursorInfoRec.HideCursor=ibm525DisableCursor;
  XAACursorInfoRec.ShowCursor=ibm525EnableCursor;
/*  XAACursorInfoRec.UseHWCursor=allways_use_cursor;
*/
  vgaHWCursor.Init = XAACursorInit;
  vgaHWCursor.Initialized = TRUE;
  vgaHWCursor.Restore = XAARestoreCursor;
  vgaHWCursor.Warp = XAAWarpCursor;
  vgaHWCursor.QueryBestSize = XAAQueryBestSize;
}

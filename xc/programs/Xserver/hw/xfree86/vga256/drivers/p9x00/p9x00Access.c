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
/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/p9x00/p9x00Access.c,v 1.1.2.1 1998/08/25 10:54:13 hohndel Exp $ */

#define P9X00ACCESS_C

#include "p9x00Regs.h"
#include "p9x00Access.h"
#include "p9x00VGARegs.h"
#include "p9x00VGA.h"
#include "p9x00Probe.h"
#include "xf86_PCI.h"

static CARD8 savedsequencer;

static CARD16 p9000dacregs[16];


static void p9000Init_DACRegs(void)
{
  if (P9X_CFG_BUS==P9X_V_VLBUS) {
    p9000dacregs[0x00]=0x03C8;
    p9000dacregs[0x01]=0x03C9;
    p9000dacregs[0x02]=0x03C6;
    p9000dacregs[0x03]=0x03C7;
    p9000dacregs[0x04]=0x43C8;
    p9000dacregs[0x05]=0x43C9;
    p9000dacregs[0x06]=0x43C6;
    p9000dacregs[0x07]=0x43C7;
    p9000dacregs[0x08]=0x83C8;
    p9000dacregs[0x09]=0x83C9;
    p9000dacregs[0x0A]=0x83C6;
    p9000dacregs[0x0B]=0x83C6;
    p9000dacregs[0x0C]=0xC3C8;
    p9000dacregs[0x0D]=0xC3C9;
    p9000dacregs[0x0E]=0xC3C6;
    p9000dacregs[0x0F]=0xC3C7;
  }
  else {
    p9000dacregs[0x00]=0x03C8;
    p9000dacregs[0x01]=0x03C9;
    p9000dacregs[0x02]=0x03C6;
    p9000dacregs[0x03]=0x03C7;
    p9000dacregs[0x04]=P9X_CFG_IO+0x4C8;
    p9000dacregs[0x05]=P9X_CFG_IO+0x4C9;
    p9000dacregs[0x06]=P9X_CFG_IO+0x4C6;
    p9000dacregs[0x07]=P9X_CFG_IO+0x4C7;
    p9000dacregs[0x08]=P9X_CFG_IO+0x8C8;
    p9000dacregs[0x09]=P9X_CFG_IO+0x8C9;
    p9000dacregs[0x0A]=P9X_CFG_IO+0x8C6;
    p9000dacregs[0x0B]=P9X_CFG_IO+0x8C7;
    p9000dacregs[0x0C]=P9X_CFG_IO+0xCC8;
    p9000dacregs[0x0D]=P9X_CFG_IO+0xCC9;
    p9000dacregs[0x0E]=P9X_CFG_IO+0xCC6;
    p9000dacregs[0x0F]=P9X_CFG_IO+0xCC7;
  }
}


CARD8 p9100readvl(CARD8 index)
{
  CARD8 result;
   
  P9X00_ENABLEPORTS;
  outb(P9X_CFG_IO,(unsigned char)index);
  result=(CARD8)inb(P9X_CFG_IO+4);
  P9X00_DISABLEPORTS;
  return result;
}


void p9100writevl(CARD8 index,CARD8 mask,CARD8 value)
{
  unsigned char tmp;

  P9X00_ENABLEPORTS;
  outb(P9X_CFG_IO,(unsigned char)index);
  tmp = inb(P9X_CFG_IO+4);
  outb(P9X_CFG_IO+4,(tmp&mask)|value);
  P9X00_DISABLEPORTS;
}

static void p9100writepci(CARD8 index,CARD8 mask,CARD8 value)
{
  xf86writepci(vga256InfoRec.scrnIndex,
               P9X_CFG_PBUS,
               P9X_CFG_PDEV,
               P9X_CFG_PFUN,
               (int)(index&0xFC),
               (((CARD32)~mask)&0x000000FFL)<<((index&0x03)<<3),
               ((CARD32)value)<<((index&0x03)<<3));
}


static void p9000enable(void)
{
  P9X00_ENABLEPORTS;
  outb(SEQ_INDEX_REG,SEQ_OUTPUT_CTL_INDEX);
  savedsequencer=inb(SEQ_PORT);
  outb(SEQ_PORT,(savedsequencer&P9X_CFG_SEQ_MASK)|P9X_CFG_SEQ_SET);
  P9X00_DISABLEPORTS;
}


static void p9000disable(void)
{
  P9X00_ENABLEPORTS;
  outb(SEQ_INDEX_REG,SEQ_OUTPUT_CTL_INDEX);
  outb(SEQ_PORT,savedsequencer);
  P9X00_DISABLEPORTS;
}


static void p9100enable(void)
{
  P9X00_WRITE(65,0xF0,0x00);
}

   
static void p9100disable(void)
{
  P9X00_WRITE(65,0xF0,0x02);
}


static CARD8 p9000read_dac(CARD8 index)
{
  return (CARD8)inb(p9000dacregs[index]);
}


static void p9000write_dac(CARD8 index,CARD8 value)
{
  outb(p9000dacregs[index],value);
}


static CARD8 p9100read_dac(CARD8 index)
{
  return (CARD8)(P9X_R_DAC(index)>>16);
}


static void p9100write_dac(CARD8 index,CARD8 value)
{
  if (P9X_R_PUCFG) /* DUMMY is always TRUE, but solves a timing problem */
    P9X_R_DAC(index)=((CARD32)value)<<16;
}


static void p9000write_icd(CARD8 value)
{
  unsigned char tmp;

  tmp = inb(0x3CC);
  outb(0x3C2,(tmp&0xF3)|value);
}


static void p9100write_icd(CARD8 value)
{
  P9X00_WRITE(0x42,0xF3,value);
}


void p9x00Init_Access(void)
{
  if (P9X_CFG_BUS==P9X_V_PCIBUS) {
    P9X00_WRITE=p9100writepci;
  }
  else {
    P9X00_WRITE=p9100writevl;
  }
  if (P9X_CFG_CHIP==P9X_V_P9100) {
    P9X00_ENABLE=p9100enable;
    P9X00_DISABLE=p9100disable;
    P9X00_WRITEDAC=p9100write_dac;
    P9X00_READDAC=p9100read_dac;
    P9X00_WRITEICD=p9100write_icd;
    p9x00regs.system_ctrl=
      (type_system_ctrl *)((unsigned char *)p9x00LinearBase+0x0004L);
    p9x00regs.video_ctrl=
      (type_video_ctrl *)((unsigned char *)p9x00LinearBase+0x0104L);
    p9x00regs.hardware_ctrl=
      (type_hardware_ctrl *)((unsigned char *)p9x00LinearBase+0x0184L);
    p9x00regs.dac=
      (CARD32 *)((unsigned char *)p9x00LinearBase+0x0200L);
    p9x00regs.copro=
      (CARD32 *)((unsigned char *)p9x00LinearBase+0x0400L);
    p9x00regs.command=
      (type_command *)((unsigned char *)p9x00LinearBase+0x2000L);
    p9x00regs.parameter_ctrl=
      (type_parameter_ctrl *)((unsigned char *)p9x00LinearBase+0x2184L);
    p9x00regs.drawing_ctrl=
      (type_drawing_ctrl *)((unsigned char *)p9x00LinearBase+0x2200L);
    p9x00regs.device_coord=
      (type_device_coord *)((unsigned char *)p9x00LinearBase+0x3000L);
    p9x00regs.meta_coord=
      (type_meta_coord *)((unsigned char *)p9x00LinearBase+0x3200L);
    p9x00regs.vram=
      (CARD32 *)((unsigned char *)p9x00LinearBase+0x800000L);
  }
  else {
    p9000Init_DACRegs();
    P9X00_ENABLE=p9000enable;
    P9X00_DISABLE=p9000disable;
    P9X00_WRITEDAC=p9000write_dac;
    P9X00_READDAC=p9000read_dac;
    P9X00_WRITEICD=p9000write_icd;
    p9x00regs.system_ctrl=
      (type_system_ctrl *)((unsigned char *)p9x00LinearBase+0x100004L);
    p9x00regs.video_ctrl=
      (type_video_ctrl *)((unsigned char *)p9x00LinearBase+0x100104L);
    p9x00regs.hardware_ctrl=
      (type_hardware_ctrl *)((unsigned char *)p9x00LinearBase+0x100184L);
    p9x00regs.command=
      (type_command *)((unsigned char *)p9x00LinearBase+0x180000L);
    p9x00regs.parameter_ctrl=
      (type_parameter_ctrl *)((unsigned char *)p9x00LinearBase+0x180184L);
    p9x00regs.drawing_ctrl=
      (type_drawing_ctrl *)((unsigned char *)p9x00LinearBase+0x180200L);
    p9x00regs.device_coord=
      (type_device_coord *)((unsigned char *)p9x00LinearBase+0x181000L);
    p9x00regs.meta_coord=
      (type_meta_coord *)((unsigned char *)p9x00LinearBase+0x181200L);
    p9x00regs.vram=
      (CARD32 *)((unsigned char *)p9x00LinearBase+0x200000L);
  }
}

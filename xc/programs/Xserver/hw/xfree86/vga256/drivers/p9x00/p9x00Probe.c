/*
 * Copyright 1996-1998  Joerg Knura (knura@imst.de)
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
/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/p9x00/p9x00Probe.c,v 1.1.2.5 1999/04/23 17:41:58 hohndel Exp $ */

#define P9X00PROBE_C

#include "p9x00Regs.h"
#include "p9x00Access.h"
#include "p9x00Probe.h"
#include "p9x00DAC.h"
#include "p9x00VGARegs.h"
#include "p9x00VGA.h"

#include "vgaPCI.h"

/*
 * it seems that the DPMS code breaks the server on several cards
 * so let's simply disable it here (990324 hohndel@XFree86.org)
 */
#undef DPMSExtension

#ifdef DPMSExtension
#include "extensions/dpms.h"
#endif

#define XCONFIG_FLAGS_ONLY
#include "xf86_Config.h"

extern Bool p9x00ScreenInit(ScreenPtr pScreen, pointer pbits,
              int xsize, int ysize, int dpix, int dpiy, int width);
  
static CARD32 p9000regions[]= {
  /*Orchid-style*/
  0xC0000000,
  0xD0000000,
  0xE0000000,
  /*Viper*/
  0x20000000,
  0x80000000,
  0xA0000000,
};
   
Bool p9000vl()
{
  Bool result = FALSE;
  CARD8 sequencer;
  int counter;
  CARD32 first_read_sysconfig;
  CARD32 x;
  CARD32 y;
  CARD32 xy;

  w5x86Save();
   
  /* First we enable the VGA-registers */     
  P9X00_ENABLEPORTS;
   
  /* unlock the Weitek's special registers */
  w5x86Unlock();
   
  /* and save the sequencer register for later restoring */
  outb(SEQ_INDEX_REG,SEQ_OUTPUT_CTL_INDEX); 
  sequencer=(CARD8)inb(SEQ_PORT);
   
  /* Now we read in P9000's System-Configuration-Register from
     that memory-locations that are possible - only those locations, 
     where values change after enabling the P9000, will be tested */

  for (counter=0;counter<6;counter++) {
    P9X_CFG_BASE=p9000regions[counter];
    P9X00_MAPALL;
    p9x00Init_Access;
    first_read_sysconfig=P9X_R_SYSCFG;                              

    /* Carefully enable P9000: */

    switch(counter) {
      /* The first three cases are only reached, when no
         Orchid-Style P9000 is found, that's why we can
         reset Bit 7 from the SEQ_PORT */
      case 3:   
        outb(SEQ_PORT,(sequencer&0x6C)|0x12);
        P9X_CFG_SEQ_MASK=0x6C;
        P9X_CFG_SEQ_SET=0x12;
        break;
      case 4:   
        outb(SEQ_PORT,(sequencer&0x6C)|0x13);
        P9X_CFG_SEQ_MASK=0x6C;
        P9X_CFG_SEQ_SET=0x13;
        break;
      case 5:   
        outb(SEQ_PORT,(sequencer&0x6C)|0x11);
        P9X_CFG_SEQ_MASK=0x6C;
        P9X_CFG_SEQ_SET=0x11;
        break;
      default: 
        /* This is the way we use for the more restricted 
           Orchid-Style P9000 (counter is 0..2) */   
        outb(SEQ_PORT,sequencer&0xEF|0x10);
        P9X_CFG_SEQ_MASK=0xEF;
        P9X_CFG_SEQ_SET=0x10;
        break;
    }
       
    if (first_read_sysconfig!=P9X_R_SYSCFG) { 
      /* We test whether writing to device coord xy register
         effects x and y registers */
       
      x=P9X_R_EDGE0_ABS_SCR_X;
      P9X_R_EDGE0_ABS_SCR_X=0L;
      y=P9X_R_EDGE0_ABS_SCR_Y;
      P9X_R_EDGE0_ABS_SCR_Y=0L;
      xy=P9X_R_EDGE0_ABS_SCR_XY;
      P9X_R_EDGE0_ABS_SCR_XY=0x12345678L;
      if ((P9X_R_EDGE0_ABS_SCR_X==0x1234L)&&
          (P9X_R_EDGE0_ABS_SCR_Y==0x5678L)) {
        result=TRUE;
      } 
      P9X_R_EDGE0_ABS_SCR_X=x;
      P9X_R_EDGE0_ABS_SCR_Y=y;
      P9X_R_EDGE0_ABS_SCR_XY=xy;
    }
          
    /* Restore sequencer - Disable P9000 */
    outb(SEQ_INDEX_REG,SEQ_OUTPUT_CTL_INDEX);
    outb(SEQ_PORT,sequencer);

    P9X00_UNMAPALL;       
  }   
          
  /* Lock Weitek's special registers */
  w5x86Lock();
   
  /* Disable Ports */
  P9X00_DISABLEPORTS;
   
  /* Save things back to VGA-Textmode */
  w5x86Restore();          

  return result;
}


Bool p9100vl(void)
{
  CARD8 temp1,temp2;
  Bool result = FALSE;
   
  for (P9X_CFG_IO=0x9100;P9X_CFG_IO<0x9140;P9X_CFG_IO+=0x08) {
    temp1=p9100readvl(4);
    temp2=(temp1&0xDF)|0x20;
    p9100writevl(4,0x00,temp2);
    if (temp2==p9100readvl(4)) {
      temp2&=0xDF;
      p9100writevl(4,0x00,temp2);
      if (temp2==p9100readvl(4)) {
        result=TRUE;
        break;
      }
    }
  }
  p9100writevl(4,0x00,temp1);
  return result;   
}


static Bool p9x00IsMemCorrect(int count)
{
  int counter;
  P9X_R_VRAM[0x8000L]=0x5A5A5A5AL;

  for (counter=0;counter<count;counter++) {
    P9X_R_VRAM[counter]=~counter;
  }
  for (counter=0;counter<count;counter++) {
    if (P9X_R_VRAM[counter]!=~counter)
      return FALSE;
  }
  return TRUE;          
}


static void p9x00TestMem()
{
  switch(P9X_CFG_CHIP) {
    case P9X_V_P9100:
      if ((P9X_R_PUCFG&P9X_RV_VRAMTYPE)==P9X_RV_VRAM256)
        P9X_CFG_MEM=P9X_V_VRAM256;
      else
        P9X_CFG_MEM=P9X_V_VRAM128;

      if ((P9X_R_PUCFG&P9X_RV_SAMSIZE)==P9X_RV_HALFSIZE)
        P9X_CFG_SAM=P9X_V_HALFSAM;
      else
        P9X_CFG_SAM=P9X_V_FULLSAM;

      if (P9X_CFG_MEM==P9X_V_VRAM256) {
        P9X_CFG_SIZE=4;
        P9X_CFG_BANKS=4;
        P9X_R_MEMCFG=0x37L;
        if (!p9x00IsMemCorrect(32)) {
          P9X_CFG_SIZE=2;
          P9X_CFG_BANKS=2;
          P9X_R_MEMCFG=0x35L;
          if (!p9x00IsMemCorrect(32)) {
            P9X_CFG_MEM=P9X_V_NONE;
            P9X_CFG_SIZE=0;
            P9X_CFG_BANKS=0;
          }
        }
      }
      else {
        P9X_CFG_SIZE=2;
        P9X_CFG_BANKS=4;
        P9X_R_MEMCFG=0x33L;
        if (!p9x00IsMemCorrect(32)) {
          P9X_CFG_MEM=P9X_V_NONE;
          P9X_CFG_SIZE=0;
          P9X_CFG_BANKS=0;
        }
      }        
      break;
    case P9X_V_P9000:
      P9X_CFG_MEM=P9X_V_VRAM256;
      P9X_CFG_SIZE=2;
      P9X_CFG_BANKS=2;
      P9X_R_MEMCFG=0x02L;
      if (!p9x00IsMemCorrect(32)) {
        P9X_CFG_SIZE=1;
        P9X_CFG_BANKS=1;
        P9X_R_MEMCFG=0x00L;
        if (!p9x00IsMemCorrect(32)) {
          P9X_CFG_MEM=P9X_V_VRAM128;
          P9X_CFG_SIZE=1;
          P9X_CFG_BANKS=2;
          P9X_R_MEMCFG=0x01L;
          if (!p9x00IsMemCorrect(1024)) {
            P9X_CFG_MEM=P9X_V_NONE;
            P9X_CFG_SIZE=0;
            P9X_CFG_BANKS=0;
          }
        }
      }
      break;   
  }
}   


static void p9x00TestDAC()
{
  CARD32 daccfg;
   
  if (P9X_CFG_CHIP==P9X_V_P9100) {
    P9X_CFG_DW=64;
    daccfg=(CARD8)((P9X_R_PUCFG&P9X_RV_DACTYPE)>>12)&0xFF;  
    switch(daccfg) {
      default:
      case 0:
        P9X_CFG_DAC=P9X_V_BT485;
        P9X_CFG_DW=32;
        break;
      case 1:
        P9X_CFG_DAC=P9X_V_BT489;
        break;
      case 2:
        P9X_CFG_DAC=P9X_V_ATT511;
        break;
      case 8:
        P9X_CFG_DAC=P9X_V_IBM525;
        break;
    }
  }
  if ((P9X_CFG_CHIP==P9X_V_P9000)||(P9X_CFG_DAC==P9X_V_BT485)) {
    P9X_CFG_DW=32;
    P9X00_WRITEDAC(BT_COMMAND_REG_0,P9X00_READDAC(BT_COMMAND_REG_0)&0x7F);      
    daccfg=P9X00_READDAC(BT_STATUS_REG);
    switch(daccfg) {
      case 0:
        P9X_CFG_DAC=P9X_V_PIXEL;
        break;
      case 2:
        P9X_CFG_DAC=P9X_V_BT485A;
        break;
      case 4:
        P9X_CFG_DAC=P9X_V_BT484;
        break;
      case 0xC:
        P9X_CFG_DAC=P9X_V_ATT504;
        break;
      case 0xD:
        P9X_CFG_DAC=P9X_V_ATT505;
        break;
      default:
        P9X_CFG_DAC=P9X_V_BT485;
        break;
    } 
  }
}


static void p9x00TestClk()
{
  if (P9X_CFG_CHIP==P9X_V_P9100) {
    if ((P9X_R_PUCFG&P9X_RV_CLOCKTYPE)==P9X_RV_ICD2061A)
      P9X_CFG_CLK=P9X_V_ICDCLOCK;
    else
      P9X_CFG_CLK=P9X_V_OSCCLOCK;
  }
  else
     P9X_CFG_CLK=P9X_V_ICDCLOCK;  
}


static void p9x00TestVCP(void)
{
  P9X_CFG_VCP=P9X_V_NONE;
  
  if (P9X_CFG_CHIP==P9X_V_P9000)
    return;

  P9X00_WRITE(0x42,0xFE,0x01);

  if ((P9X_R_COPRO(0)&0xFFFF0000L) == 0x56500000L)
    P9X_CFG_VCP=TRUE;
  else
    P9X_CFG_VCP=FALSE;
  
  P9X00_WRITE(0x42,0xFE,0x00);
}


static void p9x00GetConfig()
{
  w5x86Save();

  P9X00_MAPALL;
  p9x00Init_Access();
  P9X00_ENABLE();

  P9X_CFG_REV=P9X_R_SYSCFG&P9X_RV_REV_MASK;
  p9x00TestMem();
  p9x00TestDAC();
  p9x00TestClk();
  p9x00TestVCP();

  P9X00_DISABLE();
  P9X00_UNMAPALL;   

  w5x86Restore();
}
   
#ifdef DPMSExtension
static int LastMode=-1;
static CARD32 SyncBits=0L;

void p9x00DPMSSet(int Mode) 
{
  switch(Mode) {
    case DPMSModeOn:
      if (LastMode==-1)
        SyncBits=P9X_R_SYNC;
      else if (LastMode!=DPMSModeOn)
        P9X_R_SYNC=SyncBits;
      LastMode=DPMSModeOn;
      break;
    case DPMSModeStandby:
      if (LastMode==DPMSModeOn)
        SyncBits=P9X_R_SYNC;
      P9X_R_SYNC=P9X_RV_PM_STANDBY;
      LastMode=DPMSModeStandby;
      break;
    case DPMSModeSuspend:
      if (LastMode==DPMSModeOn)
        SyncBits=P9X_R_SYNC;
      P9X_R_SYNC=P9X_RV_PM_SUSPEND;
      LastMode=DPMSModeSuspend;
      break;
    case DPMSModeOff:
      if (LastMode==DPMSModeOn)
        SyncBits=P9X_R_SYNC;
      P9X_R_SYNC=P9X_RV_PM_OFF;
      LastMode=DPMSModeOff;
      break;
  } 
}
#endif

Bool p9x00Probe()
{
  if (vga256InfoRec.chipset) {
    if ((StrCaseCmp(vga256InfoRec.chipset, p9x00Ident(0)))
#if INCLUDE_p9000_CODE
        && (StrCaseCmp(vga256InfoRec.chipset, p9x00Ident(1)))
#endif
	)
      return (FALSE);
  }
  
  P9X_CFG_BUS=P9X_V_NONE;
  
  if (!xf86LinearVidMem()) {
    return(FALSE);
  }
  if (vgaPCIInfo) {
    if (vgaPCIInfo->Vendor!=PCI_VENDOR_WEITEK)
      return(FALSE);
    if (vgaPCIInfo->ChipType==PCI_CHIP_P9100) {
      P9X_CFG_CHIP=P9X_V_P9100;
      P9X_CFG_PBUS=vgaPCIInfo->Bus;
      P9X_CFG_PDEV=vgaPCIInfo->Card;
      P9X_CFG_PFUN=vgaPCIInfo->Func;
      P9X_CFG_BASE=vgaPCIInfo->MemBase;
      P9X_CFG_BUS=P9X_V_PCIBUS;
      P9X_CFG_ALL=0x1000000L;
    }
#if INCLUDE_p9000_CODE
    else if (vgaPCIInfo->ChipType==PCI_CHIP_P9000) {
      P9X_CFG_CHIP=P9X_V_P9000;
      P9X_CFG_PBUS=vgaPCIInfo->Bus;
      P9X_CFG_PDEV=vgaPCIInfo->Card;
      P9X_CFG_PFUN=vgaPCIInfo->Func;
      P9X_CFG_BASE=vgaPCIInfo->MemBase;
      P9X_CFG_IO=vgaPCIInfo->IOBase;
      P9X_CFG_BUS=P9X_V_PCIBUS;
      P9X_CFG_ALL=0x400000L;
    }
#endif
  }
  if (P9X_CFG_BUS==P9X_V_NONE) {
    P9X_CFG_ALL=0x1000000L;
    if (!p9100vl()) {
#if INCLUDE_p9000_CODE
      P9X_CFG_ALL=0x400000L;
      if (!p9000vl()) 
#endif
        return FALSE;
    }
  }
  p9x00GetConfig();
  if (P9X_CFG_CHIP==P9X_V_P9100) {
    ErrorF("%s %s: P9X00: Power9100",
         XCONFIG_PROBED, vga256InfoRec.name);
    if (P9X_CFG_VCP)
      ErrorF(" A%i and VideoPower9130",P9X_CFG_REV);
    else
      ErrorF(" A%i",P9X_CFG_REV);
  }
#if INCLUDE_p9000_CODE
  else
    ErrorF("%s %s: P9X00: Power9000 R%i",
         XCONFIG_PROBED, vga256InfoRec.name,P9X_CFG_REV);
#endif
  if (P9X_CFG_BUS==P9X_V_PCIBUS) 
    ErrorF(" at PCI BUS\n");
  else
    ErrorF(" at VL BUS\n");
    
  ErrorF("%s %s: P9X00: Linear Region at base: 0x%08lX\n",
         XCONFIG_PROBED, vga256InfoRec.name,P9X_CFG_BASE);
  if (P9X_CFG_MEM!=P9X_V_NONE)
    ErrorF("%s %s: P9X00: %i MBytes VRAM in %i Banks of %s VRAM-Chips\n",
           XCONFIG_PROBED, vga256InfoRec.name,P9X_CFG_SIZE,P9X_CFG_BANKS,
           (P9X_CFG_MEM==P9X_V_VRAM128)? "128Kx?":"256Kx?");
  else {
    ErrorF("%s %s: P9X00: Problems to determine memory-configuration\n");
    return FALSE;
  }
  
  if (P9X_CFG_CHIP==P9X_V_P9100)
    ErrorF("%s %s: P9X00: %s-size VRAM-Shift-Registers\n",
           XCONFIG_PROBED, vga256InfoRec.name,
           (P9X_CFG_SAM==P9X_V_HALFSAM)? "Half":"Full");
  
  ErrorF("%s %s: P9X00: ",XCONFIG_PROBED, vga256InfoRec.name);
  switch (P9X_CFG_DAC) {
    case P9X_V_IBM525:
      ErrorF("64 Bit IBM RGB525 ");
      break;
    case P9X_V_BT484:
      ErrorF("32 Bit Brooktree BT484 ");
      break;
    case P9X_V_BT485:
      ErrorF("32 Bit Brooktree BT485 ");
      break;
    case P9X_V_BT485A:
      ErrorF("32 Bit Brooktree BT485A ");
      break;
    case P9X_V_BT489:
      ErrorF("64 Bit Brooktree BT489 ");
      break;
    case P9X_V_ATT504:
      ErrorF("32 Bit AT&T ATT504 ");
      break;
    case P9X_V_ATT505:
      ErrorF("32 Bit AT&T ATT505 ");
      break;
    case P9X_V_ATT511:
      ErrorF("64 Bit AT&T ATT511 ");
      break;
    case P9X_V_PIXEL:
      ErrorF("32 Bit Pixel Semi PX2080 Media");
      break;
  }         
  ErrorF("DAC found\n");

  ErrorF("%s %s: P9X00: %s found\n",
         XCONFIG_PROBED, vga256InfoRec.name, 
         (P9X_CFG_CLK==P9X_V_OSCCLOCK)? "50 MHz Tin Can":"ICD 2061A clockchip");
  
  vga256InfoRec.MemBase=P9X_CFG_BASE; 
  vga256InfoRec.videoRam=P9X_CFG_SIZE*1024;

  if (P9X_CFG_CHIP==P9X_V_P9100) {
    vga256InfoRec.chipset= p9x00Ident(0);
    vga256InfoRec.maxClock = 170000;
  }
#if INCLUDE_p9000_CODE
  else {
    vga256InfoRec.chipset= p9x00Ident(1);
    vga256InfoRec.maxClock = 135000;
  }
#endif
  vgaSetScreenInitHook(p9x00ScreenInit);
 
  vga256InfoRec.bankedMono = FALSE;

  OFLG_SET(CLOCK_OPTION_PROGRAMABLE, &vga256InfoRec.clockOptions);

#ifdef DPMSExtension
  vga256InfoRec.DPMSSet=p9x00DPMSSet;
#endif

  if (P9X_CFG_DAC==P9X_V_IBM525)
    return TRUE;
  ErrorF("%s %s: P9X00: Sorry, this hardware is actually not supported\n",
         XCONFIG_PROBED, vga256InfoRec.name);
  return FALSE;
}
      

char *p9x00Ident(int n)
{
  static char *chipsets[] = {"p9100"
#if INCLUDE_p9000_CODE
  ,"p9000"
#endif
	};

  if (n + 1 > sizeof(chipsets) / sizeof(char *))
    return(NULL);
  else
    return(chipsets[n]);
}



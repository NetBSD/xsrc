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

/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/nv/nv3driver.c,v 1.1.2.3 1998/01/24 11:55:09 dawes Exp $ */

#include <math.h>
#include <stdlib.h>


#include "X.h"
#include "input.h"
#include "screenint.h"

#include "compiler.h"
#include "xf86.h"
#include "xf86Priv.h"
#include "xf86_OSlib.h"
#include "xf86_HWlib.h"
#include "vga.h"

#include "vgaPCI.h"

#define XCONFIG_FLAGS_ONLY
#include "xf86_Config.h"


#ifdef XFreeXDGA
#include "X.h"
#include "Xproto.h"
#include "scrnintstr.h"
#include "servermd.h"
#define _XF86DGA_SERVER_
#include "extensions/xf86dgastr.h"
#endif

#include "nv3ref.h"
#include "nvcursor.h"
#include "nvreg.h"


#include "nvvga.h"

void NV3EnterLeave(Bool enter)
{
   unsigned char temp;

  if(enter) {
    xf86EnableIOPorts(vga256InfoRec.scrnIndex);
    outb(vgaIOBase + 4, 0x11); temp = inb(vgaIOBase + 5);
    outb(vgaIOBase + 5, temp & 0x7F); 
    SR_Write(LOCK_EXT_INDEX,UNLOCK_EXT_MAGIC);
  }else {
    outb(vgaIOBase + 4, 0x11); temp = inb(vgaIOBase + 5);
    outb(vgaIOBase + 5, (temp & 0x7F) | 0x80);
    SR_Write(LOCK_EXT_INDEX,LOCK_EXT_MAGIC);
    xf86DisableIOPorts(vga256InfoRec.scrnIndex);
  }
}

#define MapDevice(device,base) \
  nv##device##Port=(unsigned*)xf86MapVidMem(vga256InfoRec.scrnIndex,\
                                 MMIO_REGION,\
                                 ((char*)(base))+DEVICE_BASE(device),\
                                 DEVICE_SIZE(device))


static void MapNV3Regs(void *regBase,void *frameBase)
{
  MapDevice(PRAMDAC,regBase);
  MapDevice(PFB,regBase);
  MapDevice(PFIFO,regBase);
  MapDevice(PGRAPH,regBase);
  MapDevice(PMC,regBase);
  MapDevice(CHAN0,regBase);
  MapDevice(PRAMIN,frameBase);
}

#define NV3_MAX_CLOCK_IN_KHZ 230000

static void NV3FlipFunctions(vgaVideoChipRec *nv);

int NV3Probe(vgaVideoChipRec *nv,void *base0,void *base1)
{
  int noaccel= OFLG_ISSET(OPTION_NOACCEL,&vga256InfoRec.options);

  /* By the time we have got here, we know it is an NV3 */
  vga256InfoRec.maxClock = NV3_MAX_CLOCK_IN_KHZ;

  MapNV3Regs(base0,base1);

  if(!vga256InfoRec.videoRam) {
    vga256InfoRec.videoRam = (1024 << 
     (PFB_Read(BOOT_0) & PFB_Mask(BOOT_0_RAM_AMOUNT)));
  }
  nv->ChipLinearSize=vga256InfoRec.videoRam*1024;
  /* Doesn't feel right that this should be an int! */  
  nv->ChipLinearBase=(int)base1;  
  nv->ChipHas32bpp=TRUE;
  /* I/O ports are needed for things like pallete selection etc */
  xf86ClearIOPortList (vga256InfoRec.scrnIndex);
  xf86AddIOPorts(vga256InfoRec.scrnIndex,Num_VGA_IOPorts,VGA_IOPorts);  
  xf86EnableIOPorts(vga256InfoRec.scrnIndex);
  vgaIOBase = (inb(0x3CC) & 0x01) ? 0x3D0 : 0x3B0;
  NV3EnterLeave(ENTER);

  /* The NV1 only supports 555 weighting, so force it here */
  if(vgaBitsPerPixel==16 && !noaccel) {
    ErrorF("%s %s: %s: Setting RGB weight to 555\n",XCONFIG_PROBED, 
                                                    vga256InfoRec.name,
                                                    vga256InfoRec.chipset);
    xf86weight.green=xf86weight.blue=xf86weight.red=5;
  }

  OFLG_SET(OPTION_NOACCEL, &(nv->ChipOptionFlags));
  OFLG_SET(OPTION_SW_CURSOR, &(nv->ChipOptionFlags));

  NV3FlipFunctions(nv);

  return 1;    
}

/*
 * Fout=(Fin *(N/M)) / (1<<P)
 *
 * Constraints:
 *   1Mhz <= Fin/M <= 2Mhz
 * 128Hhz <= Fin * (N/M) <= 256Mhz
 *
 */

/* NTSC cards have approx 14.3Mhz. Need to detect, but leave for now*/
#define PLL_INPUT_FREQ 13500 
#define M_MIN 7
#define M_MAX 13

#define P_MIN 0
#define P_MAX 7 /* Not sure about this. Could be 4 */

static int NV3ClockSelect(float clockIn,float *clockOut,int *mOut,
                                        int *nOut,int *pOut)
{ 
  int m,n,p;
  float bestDiff=1e10;
  float target=0.0;
  float best=0.0;
  float diff;
  int nMax,nMin;
  
  *clockOut=0.0;
  for(p=P_MIN;p<=P_MAX;p++) {
    for(m=M_MIN;m<=M_MAX;m++) {
      float fm=(float)m;
      /* Now calculate maximum and minimum values for n */
      nMax=(int) (((256000/PLL_INPUT_FREQ)*fm)-0.5);
      nMin=(int) (((128000/PLL_INPUT_FREQ)*fm)+0.5);
      n=(int)(((clockIn*((float)(1<<p)))/PLL_INPUT_FREQ)*fm);
      if(n>=nMin && n<=nMax) {  
        float fn=(float)n;
        target=(PLL_INPUT_FREQ*(fn/fm))/((float)(1<<p));
        diff=fabs(target-clockIn);
        if(diff<bestDiff) {
          bestDiff=diff;
          best=target;
          *mOut=m;*nOut=n;*pOut=p;
          *clockOut=best;
	}
      }
    }
  }
  return (best!=0.0);    
}

#define new ((vgaNVPtr)vgaNewVideoState)

/* Very useful macro that allows you to set overflow bits */
#define SetBitField(value,from,to) SetBF(to,GetBF(value,from))
#define SetBit(n) (1<<(n))
#define Set8Bits(value) ((value)&0xff)

static int CalculateCRTC(DisplayModePtr mode)
{
  int bpp=vgaBitsPerPixel/8,
      horizDisplay = (mode->CrtcHDisplay/8) - 1,
      horizStart = (mode->CrtcHSyncStart/8) - 1,
      horizEnd = (mode->CrtcHSyncEnd/8) - 1,
      horizTotal = (mode->CrtcHTotal/8)	- 1,
      vertDisplay = mode->CrtcVDisplay - 1,
      vertStart = mode->CrtcVSyncStart	- 1,
      vertEnd = mode->CrtcVSyncEnd - 1,
      vertTotal = mode->CrtcVTotal - 2;
        
  /* Calculate correct value for offset register */
  new->std.CRTC[0x13]=((vga256InfoRec.displayWidth/8)*bpp)&0xff;
  /* Extra bits for CRTC offset register */
  new->regs.nv3.repaint0=
    SetBitField((vga256InfoRec.displayWidth/8)*bpp,10:8,7:5);

  /* The NV3 manuals states that for native modes, there should be no 
   * borders. This code should also be tidied up to use symbolic names
   */     
  new->std.CRTC[0x0]=Set8Bits(horizTotal - 4);
  new->std.CRTC[0x1]=Set8Bits(horizDisplay);
  new->std.CRTC[0x2]=Set8Bits(horizDisplay);
  new->std.CRTC[0x3]=SetBitField(horizTotal,4:0,4:0) | SetBit(7);
  new->std.CRTC[0x4]=Set8Bits(horizStart);
  new->std.CRTC[0x5]=SetBitField(horizTotal,5:5,7:7)|
                     SetBitField(horizEnd,4:0,4:0);
  new->std.CRTC[0x6]=SetBitField(vertTotal,7:0,7:0);

  new->std.CRTC[0x7]=SetBitField(vertTotal,8:8,0:0)|
		     SetBitField(vertDisplay,8:8,1:1)|
		     SetBitField(vertStart,8:8,2:2)|
		     SetBitField(vertDisplay,8:8,3:3)|
		     SetBit(4)|
		     SetBitField(vertTotal,9:9,5:5)|
		     SetBitField(vertDisplay,9:9,6:6)|
		     SetBitField(vertStart,9:9,7:7);

  new->std.CRTC[0x9]= SetBitField(vertDisplay,9:9,5:5) | SetBit(6);
  new->std.CRTC[0x10]= Set8Bits(vertStart);
  new->std.CRTC[0x11]= SetBitField(vertEnd,3:0,3:0) | SetBit(5);
  new->std.CRTC[0x12]= Set8Bits(vertDisplay);
  new->std.CRTC[0x15]= Set8Bits(vertDisplay);
  new->std.CRTC[0x16]= Set8Bits(vertTotal + 1);

  new->regs.nv3.screenExtra= SetBitField(horizTotal,6:6,4:4) |
                             SetBitField(vertDisplay,10:10,3:3) |
                             SetBitField(vertStart,10:10,2:2) |
                             SetBitField(vertDisplay,10:10,1:1) |
                             SetBitField(vertTotal,10:10,0:0);

  if(mode->Flags & V_DBLSCAN) new->std.CRTC[0x9]|=0x80;
 
  /* I think this should be SetBitField(horizTotal,8:8,0:0), but this
   * doesn't work apparently. Why 260 ? 256 would make sense.
   */
  new->regs.nv3.horizExtra= (horizTotal < 260 ? 0 : 1);
  return 1;
}

/* Gamma seems possible only for 15bpp and 32bpp. Need to fiddle 
 * around to confirm this 
 */
#define GammaMode() (!(vgaBitsPerPixel==8 || xf86weight.green==6))

static void InitPalette(DisplayModePtr mode)
{
  int bpp=vgaBitsPerPixel/8;
  int i;

  if(!GammaMode()) return;
  /* Put simple ramp in for now !! */
  /* Do any other drivers implement gamma ?? */
  for(i=0;i<256;i++) {
    new->std.DAC[i*3]=i>>2;
    new->std.DAC[(i*3)+1]=i>>2;
    new->std.DAC[(i*3)+2]=i>>2;
  }
}


static Bool NV3Init(DisplayModePtr mode)
{
  int m,n,p;
  float clockIn=(float)vga256InfoRec.clock[mode->Clock];
  float clockOut;
  int time,data; 
  int i;
  int pixelDepth;
 
  /* Calculate standard VGA settings */
  if(!vgaHWInit (mode, sizeof (vgaNVRec))) {
    return 0;
  }

  /* Calculate Vclock frequency */
  if(!NV3ClockSelect(clockIn,&clockOut,&m,&n,&p)) {
    ErrorF("%s %s: %s: Unable to set desired video clock\n",
           XCONFIG_PROBED, vga256InfoRec.name,vga256InfoRec.chipset);
    return FALSE;  
  }
  new->regs.nv3.vpllCoeff=PRAMDAC_Val(VPLL_COEFF_NDIV,n) | 
                          PRAMDAC_Val(VPLL_COEFF_MDIV,m) |
                          PRAMDAC_Val(VPLL_COEFF_PDIV,p);

  /* VGA is always valid for the NV3 */
  new->vgaValid=1;

  CalculateCRTC(mode);
  InitPalette(mode);

  /* For now I don't support 8 bit pallettes but it is easy to add */  
  new->regs.nv3.repaint1=
    PCRTC_Val(REPAINT1_LARGE_SCREEN,mode->CrtcHDisplay<1280) |
    PCRTC_Def(REPAINT1_PALETTE_WIDTH,6BITS);
 
  /* Need to figure out what the algorithm to set these are */
  new->regs.nv3.fifoControl=0x82;/*PCRTC_Def(FIFO_CONTROL_BURST_LENGTH,64);*/
  new->regs.nv3.fifo=0x22;/*PCRTC_Val(FIFO_WATERMARK,256>>3)|
                     PCRTC_Val(FIFO_RESET,1);*/


  /* PixelFormat controls how many bits per pixel. 
   * There is another register in the 
   * DAC which controls if mode is 5:5:5 or 5:6:5
   */
  pixelDepth=(vgaBitsPerPixel+1)/8;
  if(pixelDepth>3) pixelDepth=3;
  new->regs.nv3.pixelFormat=pixelDepth;

  new->regs.nv3.generalControl=
     PRAMDAC_Def(GENERAL_CONTROL_IDC_MODE,GAMMA)|
     PRAMDAC_Val(GENERAL_CONTROL_565_MODE,xf86weight.green==6)|
     PRAMDAC_Def(GENERAL_CONTROL_TERMINATION,37OHM)|
     PRAMDAC_Def(GENERAL_CONTROL_BPC,6BITS)|  
     PRAMDAC_Def(GENERAL_CONTROL_VGA_STATE,SEL); /* Not sure about this */
     
  /* This makes sure that the Mclock and Vclock are are actually selected 
   * via the PLL. It also sets the Vclock/Pclock ratio to be divide by 2
   * or not. Not sure when this should be divide by 1, presumably for 
   * very high Vclock????
   */
  new->regs.nv3.coeffSelect=PRAMDAC_Def(PLL_COEFF_SELECT_MPLL_SOURCE,PROG)|
                            PRAMDAC_Def(PLL_COEFF_SELECT_VPLL_SOURCE,PROG)|
                            PRAMDAC_Def(PLL_COEFF_SELECT_VCLK_RATIO,DB2);

  /* Disable Tetris tiling for now. This looks completely mad but could 
   * give some significant performance gains. Will perhaps experiment 
   * later on with this stuff!  
   */
  new->regs.nv3.config0=
      PFB_Val(CONFIG_0_RESOLUTION,((vga256InfoRec.displayWidth+31)/32))|
      PFB_Val(CONFIG_0_PIXEL_DEPTH,pixelDepth)|
      PFB_Def(CONFIG_0_TILING,DISABLED); 

  return TRUE;
}  



static void NV3Restore(void *data)
{
  vgaNVPtr restore=data;
  NV3Registers *nv3=&(restore->regs.nv3);

  /* I do not know what this does */
  vgaProtect(TRUE); 

  PCRTC_Write(REPAINT0,nv3->repaint0);
  PCRTC_Write(REPAINT1,nv3->repaint1);
  PCRTC_Write(EXTRA,nv3->screenExtra);
  PCRTC_Write(PIXEL,nv3->pixelFormat);
  PCRTC_Write(HORIZ_EXTRA,nv3->horizExtra); 
  PCRTC_Write(FIFO_CONTROL,nv3->fifoControl); 
 
  PFB_Write(CONFIG_0,nv3->config0);

  PRAMDAC_Write(VPLL_COEFF,nv3->vpllCoeff);
  PRAMDAC_Write(PLL_COEFF_SELECT,nv3->coeffSelect);
  PRAMDAC_Write(GENERAL_CONTROL,nv3->generalControl);

  vgaHWRestore((vgaHWPtr)restore);  
  
  vgaProtect(FALSE);
}

static void *NV3Save(void *data)
{
  vgaNVPtr save=NULL;

  save=(vgaNVPtr)vgaHWSave((vgaHWPtr)data,sizeof(vgaNVRec));  
  save->regs.nv3.repaint0=PCRTC_Read(REPAINT0);
  save->regs.nv3.repaint1=PCRTC_Read(REPAINT1);
  save->regs.nv3.screenExtra=PCRTC_Read(EXTRA);
  save->regs.nv3.pixelFormat=PCRTC_Read(PIXEL);
  save->regs.nv3.horizExtra=PCRTC_Read(HORIZ_EXTRA);
  save->regs.nv3.fifoControl=PCRTC_Read(FIFO_CONTROL); 
  save->regs.nv3.fifo=PCRTC_Read(FIFO);
  save->regs.nv3.config0=PFB_Read(CONFIG_0);

  save->regs.nv3.vpllCoeff=PRAMDAC_Read(VPLL_COEFF);
  save->regs.nv3.coeffSelect=PRAMDAC_Read(PLL_COEFF_SELECT);
  save->regs.nv3.generalControl=PRAMDAC_Read(GENERAL_CONTROL);

  return (void*)save;
}


static void NV3Adjust(int x,int y) 
{
  int bpp=vgaBitsPerPixel/8;
  int startAddr=(((y*vga256InfoRec.virtualX)+x)*bpp);
  int offset=startAddr>>2;
  int pan=(startAddr&3)*2;
  unsigned char byte;
  
  /* Now shift start address. Word aligned */
  CRTC_Write(0x0d,Set8Bits(offset));
  CRTC_Write(0x0c,SetBitField(offset,15:8,7:0));
  byte=PCRTC_Read(REPAINT0) & ~PCRTC_Mask(REPAINT0_START_ADDR_20_16);
  PCRTC_Write(REPAINT0,SetBitField(offset,20:16,4:0)|byte);
  /* Attribute register 0x13 is used to provide up to 4 pixel shift */
  byte=inb(vgaIOBase+0x0a);
  outb(0x3c0,0x13);
  outb(0x3c0,pan);
}
 

static int NV3ValidMode(DisplayModePtr mode,Bool verbose,int flag)
{
  return MODE_OK;
}


extern vgaHWCursorRec vgaHWCursor;

static void NV3FbInit(void)
{
  /* Need check in here for wierd resolutions !! */

  if(!OFLG_ISSET(OPTION_SW_CURSOR, &vga256InfoRec.options)) {
    /* Initialise the hardware cursor */
    vgaHWCursor.Initialized = TRUE;
    vgaHWCursor.Init = NV3CursorInit;
    vgaHWCursor.Restore = NV3RestoreCursor;
    vgaHWCursor.Warp = NV3WarpCursor;
    vgaHWCursor.QueryBestSize = NV3QueryBestSize;
    if(xf86Verbose) {
      ErrorF("%s %s: %s: Using hardware cursor\n",XCONFIG_PROBED, 
             vga256InfoRec.name,vga256InfoRec.chipset);
    }
  }

  if(!OFLG_ISSET(OPTION_NOACCEL, &vga256InfoRec.options)) {
    NVAccelInit();
  }
  
}

static void NV3DisplayPowerManagementSet(int mode)
{
}

static Bool NV3ScreenInit(ScreenPtr pScreen,pointer pbits, 
                          int xsize,int ysize,int dpix,int dpiy,int width)
{
  return TRUE;
}



static void NV3SaveScreen(int on)
{
  vgaHWSaveScreen(on);
}

static void NV3GetMode(DisplayModePtr display)
{
}

/* Changes the entries in the NV struct to point at the correct function 
 * pointers. Called from the Probe() function
 */
static void NV3FlipFunctions(vgaVideoChipRec *nv)
{
  nv->ChipEnterLeave=NV3EnterLeave;
  nv->ChipInit=NV3Init;
  nv->ChipValidMode=NV3ValidMode;
  nv->ChipSave=NV3Save;
  nv->ChipRestore=NV3Restore;
  nv->ChipAdjust=NV3Adjust;
  nv->ChipSaveScreen=NV3SaveScreen;
  nv->ChipGetMode=(void (*)())NoopDDA;
  nv->ChipFbInit=NV3FbInit;
}

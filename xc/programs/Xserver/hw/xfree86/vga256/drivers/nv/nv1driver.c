
/* $XConsortium: nv_driver.c /main/3 1996/10/28 05:13:37 kaleb $ */
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

/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/nv/nv1driver.c,v 1.1.2.3 1998/01/24 11:55:08 dawes Exp $ */

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
/*
 * If the driver makes use of XF86Config 'Option' flags, the following will be
 * required
 */
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

#define NV_1764_MAX_CLOCK_IN_KHZ 170000
#define NV_1732_MAX_CLOCK_IN_KHZ 135000

#define NV_MAX_VCLK_PIN_CLOCK_IN_KHZ  50000

#include "nv1ref.h"
#include "nvreg.h"
#include "nvcursor.h"
#include "nvvga.h"


/* Function to read extended register */

#define MapDevice(device,base) \
  nv##device##Port=(unsigned*)xf86MapVidMem(vga256InfoRec.scrnIndex,\
                                 MMIO_REGION,\
                                 ((char*)(base))+DEVICE_BASE(device),\
                                 DEVICE_SIZE(device))

#define NV_CHAN0 0x0080ffff:0x00800000

#define NV_FRAME_BUFFER 0x01000000

static void MapNvRegs(void *base)
{
  MapDevice(PDAC,base);
  MapDevice(PFB,base);
  MapDevice(PRM,base);
  MapDevice(PGRAPH,base);
  MapDevice(PDMA,base);
  MapDevice(PFIFO,base);
  MapDevice(PRAM,base);
  MapDevice(PRAMFC,base);
  MapDevice(PRAMHT,base);
  MapDevice(PMC,base);
  MapDevice(CHAN0,base);
}


#define PALETTE_SIZE 256

#define VPLL_INPUT_FREQ 12096.0
#define PLL_LOWER_BOUND 64000
#define PLL_UPPER_BOUND vga256InfoRec.maxClock


/* 
 *  The following equation defines the output frequency of the PLL
 *  fout = ( N/(M*P) ) * fin;
 *
 *  It must also obey the following restraints!
 *  1Mhz <= fin/M <= 2Mhz (This means M in the range 12 -> 7)
 *
 *  64Mhz <= ((N*O)/M)*fin <=135 (or whatever the max ramdac speed is)
 *  
 *  The following function simple does a brute force search I'm afraid.
 *  I know that it is possible to write a much faster function, but 
 *  there are more important things to work on, and at least I know 
 *  this one will always get the right answer!
 */

static int NVClockSelect(float clockIn,float *clockOut,
                         int *Mparam,int *Nparam,int *Oparam,int *Pparam)
{
  int m,n,o,p;
  float bestDiff=1e10; /* Set to silly value for first range */
  float target=0.0;
  float best=0.0;

  *clockOut=0;
  for(p=1;p<=8;p*=2) {
    float fp=(float)p;
    for(n=8;n<=255;n++) {
      float fn=(float)n;
      for(m=12;m>=7;m--) {
        float fm=(float)m;
        for(o=1;o<=15;o++) {
          float fo=(float) o;
          float check= ((fn*fo)/fm)*VPLL_INPUT_FREQ;
          if(check>PLL_LOWER_BOUND && check <= PLL_UPPER_BOUND) break;
	}
        if(o!=16) {
          float diff;                    
          target=(fn/(fm*fp))*VPLL_INPUT_FREQ;
          diff=fabs(target - clockIn);
          if(diff < bestDiff) {
            bestDiff=diff;
            best=target; 
            *Mparam=m;*Nparam=n;*Oparam=o;*Pparam=p;
            *clockOut=best;
	  }
	}
      }
    }
  }
  return (best!=0.0);
}

static int pixelPortWidth=0; /* Holds how big it is,needed in NVInit() */


static int ProbeRamdac(void)
{
  int id=PDAC_ReadExt(COMPANY_ID);

  if(id!=NV_PDAC_SGS_ID) {
    ErrorF("%s %s: %s: Unsupported RAMDAC (vendor number 0x%02x)\n",
           XCONFIG_PROBED,vga256InfoRec.name,vga256InfoRec.chipset,id); 
    return 0;
  }
  /* Figure out what sort of RAMDAC we have. I don't know if the RAMDACS
   * have different PCI device numbers, but this test will always work
   */  
  switch(id=PDAC_ReadExt(DEVICE_ID)) {
    case NV_PDAC_1764_ID:
      /* I believe all Diamond Edge3D boards use the 1764 */
      vga256InfoRec.maxClock = NV_1764_MAX_CLOCK_IN_KHZ;
      break;
    case NV_PDAC_1732_ID:      
      vga256InfoRec.maxClock = NV_1732_MAX_CLOCK_IN_KHZ;
      break;
    default: 
      ErrorF("%s %s: %s: Unsupported SGS RAMDAC (id number 0x%02x)\n",
          XCONFIG_PROBED,vga256InfoRec.name,vga256InfoRec.chipset,id); 
      return 0;
      break;
  }
  /* We need to figure out what the pixel port width is. This depends 
   * on memory configuration etc, so we read it here so the NVInit() 
   * function can put it back in.
   */
   pixelPortWidth=GetBF(PDAC_ReadExt(CONF_0),NV_PDAC_CONF_0_PORT_WIDTH);

   return 1;
}

/* Forward declarations for Probe() */
static void NV1FlipFunctions(vgaVideoChipRec *nv);
static void NV1DisplayPowerManagementSet(int mode);
static Bool NV1BlankScreen(ScreenPtr pScreen,Bool on);
static Bool NV1ScreenInit(ScreenPtr pScreen,pointer pbits, 
                          int xsize,int ysize,int dpix,int dpiy,int width);

int NV1Probe(vgaVideoChipRec *nv,void *base0,void *base1)
{
  int ramdacId;

  ErrorF("NV1Probe() %p %p\n",base0,base1);

  /* I/o ports are needed for things like pallete selection etc */
  xf86ClearIOPortList (vga256InfoRec.scrnIndex);
  xf86AddIOPorts(vga256InfoRec.scrnIndex,Num_VGA_IOPorts,VGA_IOPorts);

  nv->ChipLinearBase=(int)base0+NV_FRAME_BUFFER;
  /* Now memory map the registers */
  MapNvRegs((pointer)base0); 

  /* Calculate how much RAM, unless user supplies */

  if(!vga256InfoRec.videoRam) {
    vga256InfoRec.videoRam = (1024 << 
      GetBF(PFB_Read(BOOT_0),NV_PFB_BOOT_0_RAM_AMOUNT));
  }
  nv->ChipLinearSize=vga256InfoRec.videoRam*1024;
  nv->ChipHas32bpp=FALSE;

  if(getenv("NV_NORAMDAC_PROBE")) {
    ErrorF("%s %s: %s: Skipping RAMDAC test (Hi Kent!)\n",XCONFIG_PROBED, 
                                                    vga256InfoRec.name,
                                                    vga256InfoRec.chipset);
    vga256InfoRec.maxClock = NV_1764_MAX_CLOCK_IN_KHZ;
    pixelPortWidth=GetBF(PDAC_ReadExt(CONF_0),NV_PDAC_CONF_0_PORT_WIDTH);
  }else {
    if(!ProbeRamdac()) return FALSE;
  }
#ifdef XFreeXDGA
  vga256InfoRec.directMode = XF86DGADirectPresent;
#endif
  vga256InfoRec.bankedMono = FALSE;

#ifdef DPMSExtension
  vga256InfoRec.DPMSSet = NV1DisplayPowerManagementSet;
#endif

  /* Hook blank screen function. Standard VGA one doesn't work */
  vgaBlankScreenFunc=NV1BlankScreen;
  /* Hook install palette */
  vgaSetScreenInitHook(NV1ScreenInit);

  /* The NV1 only supports 555 weighting, so force it here */
  if(vgaBitsPerPixel==16) {
    ErrorF("%s %s: %s: Setting RGB weight to 555\n",XCONFIG_PROBED, 
                                                    vga256InfoRec.name,
                                                    vga256InfoRec.chipset);
    xf86weight.green=xf86weight.blue=xf86weight.red=5;
  }

  OFLG_SET(OPTION_DAC_8_BIT,&(nv->ChipOptionFlags));
  OFLG_SET(OPTION_NOACCEL, &(nv->ChipOptionFlags));
  OFLG_SET(OPTION_SW_CURSOR, &(nv->ChipOptionFlags));

  NV1FlipFunctions(nv);

  return (TRUE);
}

/* This function does nothing for the NV1 as it is not driven 
 * though the VGA
 */
static void NV1EnterLeave(Bool enter)
{
}

/*
 * NV1Restore --
 *
 * This function restores a video mode.  It basically writes out all of
 * the registers that have previously been saved in the vgaNVRec data 
 * structure.
 *
 * Note that "Restore" is a little bit incorrect.  This function is also
/ * used when the server enters/changes video modes.  The mode definitions 
 * have previously been initialized by the Init() function, below.
 */
static void NV1Restore(void *data)
{
  int i;
  vgaNVPtr restore = data;
  NV1Registers *nv1=&(restore->regs.nv1);
  int toTextMode;

  /* Only restore generic vga IF WE ARE GOING TO TEXT MODE */
  if(restore->vgaValid) {
    xf86EnableIOPorts(vga256InfoRec.scrnIndex);
    vgaHWRestore ((vgaHWPtr) restore);
    xf86DisableIOPorts(vga256InfoRec.scrnIndex);
  }

  /* Set the clock registers */

  PRM_Write(TRACE,nv1->memoryTrace);
  PRM_Write(CONFIG_0,nv1->prmConfig0);

  PDAC_WriteExt(VPLL_M_PARAM,nv1->Mparam);
  PDAC_WriteExt(VPLL_N_PARAM,nv1->Nparam);
  PDAC_WriteExt(VPLL_O_PARAM,nv1->Oparam);
  PDAC_WriteExt(VPLL_P_PARAM,nv1->Pparam);

  PDAC_WriteExt(MPLL_M_PARAM,nv1->MparamMPLL);
  PDAC_WriteExt(MPLL_N_PARAM,nv1->NparamMPLL);
  PDAC_WriteExt(MPLL_O_PARAM,nv1->OparamMPLL);
  PDAC_WriteExt(MPLL_P_PARAM,nv1->PparamMPLL);

  /* Set the dac conf reg that sets the pixel depth */
  PDAC_WriteExt(CONF_0,nv1->dacConfReg0);
  PDAC_WriteExt(CONF_1,nv1->dacConfReg1);
  PDAC_WriteExt(RGB_PAL_CTRL,nv1->dacRgbPalCtrl);

  /* Write out the registers that control the dumb framebuffer */ 
  PFB_Write(CONFIG_0,nv1->confReg0);
  PFB_Write(GREEN_0,nv1->green0);
  PFB_Write(START,nv1->startAddr);

  PFB_Write(HOR_FRONT_PORCH,nv1->horFrontPorch);
  PFB_Write(HOR_SYNC_WIDTH,nv1->horSyncWidth);
  PFB_Write(HOR_BACK_PORCH,nv1->horBackPorch);
  PFB_Write(HOR_DISP_WIDTH,nv1->horDispWidth);
  PFB_Write(VER_FRONT_PORCH,nv1->verFrontPorch);
  PFB_Write(VER_SYNC_WIDTH,nv1->verSyncWidth);
  PFB_Write(VER_BACK_PORCH,nv1->verBackPorch);
  PFB_Write(VER_DISP_WIDTH,nv1->verDispWidth);

  /* Now restore cursor registers. I don't know if this is strictly 
   * needed or not, but it isn't hard to do
   */
  PDAC_WriteExt(CURSOR_CTRL_A,nv1->cursorCtrl);
  PDAC_WriteExt(CURSOR_X_POS_LO,nv1->xLo);
  PDAC_WriteExt(CURSOR_X_POS_HI,nv1->xHi);  
  PDAC_WriteExt(CURSOR_Y_POS_LO,nv1->yLo);
  PDAC_WriteExt(CURSOR_Y_POS_HI,nv1->yHi);
 
  for(i=0;i<3;i++) {
    PDAC_WriteExt(CURSOR_COLOUR_1_RGB+i,nv1->colour1[i]);
    PDAC_WriteExt(CURSOR_COLOUR_2_RGB+i,nv1->colour2[i]);
    PDAC_WriteExt(CURSOR_COLOUR_3_RGB+i,nv1->colour3[i]);
  }
  for(i=0;i<NV_PDAC_CURSOR_PLANE_SIZE;i++) {
    PDAC_WriteExt(CURSOR_PLANE_0+i,nv1->plane0[i]);
    PDAC_WriteExt(CURSOR_PLANE_1+i,nv1->plane1[i]);
  }

  /* Restore pallette */
  for(i=0;i<PALETTE_SIZE;i++) {
    PDAC_Write(WRITE_PALETTE,(unsigned char)i);
    PDAC_Write(COLOUR_DATA,nv1->palette[i][0]);
    PDAC_Write(COLOUR_DATA,nv1->palette[i][1]);
    PDAC_Write(COLOUR_DATA,nv1->palette[i][2]);
  }
}

/*
 * NV1Save --
 *
 * This function saves the video state.  It reads all of the SVGA registers
 * into the vgaNVRec data structure.  There is in general no need to
 * mask out bits here - just read the registers.
 */
static void *NV1Save(void *data)
{
  vgaNVPtr save=NULL;
  int i;
  int inTextMode;
  NV1Registers *nv1;
  inTextMode=(PRM_Read(CONFIG_0) & PRM_Def(CONFIG_0_TEXT,ENABLED));
  if(inTextMode) {
    xf86EnableIOPorts(vga256InfoRec.scrnIndex);
    save = (vgaNVPtr) vgaHWSave ((vgaHWPtr) data, sizeof (vgaNVRec));
    xf86DisableIOPorts(vga256InfoRec.scrnIndex);
    save->vgaValid=1;
  }else {
    save=(data) ? data : xcalloc(1,sizeof(vgaNVRec));
    save->vgaValid=0;      
  }
  nv1=&(save->regs.nv1);
  nv1->prmConfig0=PRM_Read(CONFIG_0);
  nv1->memoryTrace=PRM_Read(TRACE);

  nv1->dacConfReg0=PDAC_ReadExt(CONF_0);
  nv1->dacConfReg1=PDAC_ReadExt(CONF_1);
  nv1->dacRgbPalCtrl=PDAC_ReadExt(RGB_PAL_CTRL);

  nv1->Mparam=PDAC_ReadExt(VPLL_M_PARAM);
  nv1->Nparam=PDAC_ReadExt(VPLL_N_PARAM);
  nv1->Oparam=PDAC_ReadExt(VPLL_O_PARAM);
  nv1->Pparam=PDAC_ReadExt(VPLL_P_PARAM);

  nv1->MparamMPLL=PDAC_ReadExt(MPLL_M_PARAM);
  nv1->NparamMPLL=PDAC_ReadExt(MPLL_N_PARAM);
  nv1->OparamMPLL=PDAC_ReadExt(MPLL_O_PARAM);
  nv1->PparamMPLL=PDAC_ReadExt(MPLL_P_PARAM);

  nv1->confReg0=PFB_Read(CONFIG_0);
  nv1->green0=PFB_Read(GREEN_0);
  nv1->startAddr=PFB_Read(START);

  nv1->horFrontPorch=PFB_Read(HOR_FRONT_PORCH);
  nv1->horSyncWidth=PFB_Read(HOR_SYNC_WIDTH);
  nv1->horBackPorch=PFB_Read(HOR_BACK_PORCH);
  nv1->horDispWidth=PFB_Read(HOR_DISP_WIDTH);
  nv1->verFrontPorch=PFB_Read(VER_FRONT_PORCH);
  nv1->verSyncWidth=PFB_Read(VER_SYNC_WIDTH);
  nv1->verBackPorch=PFB_Read(VER_BACK_PORCH);
  nv1->verDispWidth=PFB_Read(VER_DISP_WIDTH);

  /* Restore the HW cursor registers */
  nv1->cursorCtrl=PDAC_ReadExt(CURSOR_CTRL_A);
  nv1->xLo=PDAC_ReadExt(CURSOR_X_POS_LO);
  nv1->xHi=PDAC_ReadExt(CURSOR_X_POS_HI);
  nv1->yLo=PDAC_ReadExt(CURSOR_Y_POS_LO);
  nv1->yHi=PDAC_ReadExt(CURSOR_Y_POS_HI);
 
  for(i=0;i<3;i++) {
    nv1->colour1[i]=PDAC_ReadExt(CURSOR_COLOUR_1_RGB+i);
    nv1->colour2[i]=PDAC_ReadExt(CURSOR_COLOUR_2_RGB+i);
    nv1->colour3[i]=PDAC_ReadExt(CURSOR_COLOUR_3_RGB+i);
  }
  for(i=0;i<NV_PDAC_CURSOR_PLANE_SIZE;i++) {
    nv1->plane0[i]=PDAC_ReadExt(CURSOR_PLANE_0+i);
    nv1->plane1[i]=PDAC_ReadExt(CURSOR_PLANE_1+i);
  }

  /* Save pallette */
  for(i=0;i<PALETTE_SIZE;i++) {      
    PDAC_Write(READ_PALETTE,(unsigned char)i);
    nv1->palette[i][0]=PDAC_Read(COLOUR_DATA);
    nv1->palette[i][1]=PDAC_Read(COLOUR_DATA);
    nv1->palette[i][2]=PDAC_Read(COLOUR_DATA);
  }

  return ((void*)save);
}



static int VirtualScreenOk(DisplayModePtr mode)
{
 if(mode->CrtcHDisplay!=vga256InfoRec.virtualX) return 0;
 
 if(mode->Flags & V_DBLSCAN) {
   return (2*vga256InfoRec.virtualY==mode->CrtcVDisplay);
 }
  
 return (vga256InfoRec.virtualY==mode->CrtcVDisplay);
}


#define RESOLUTION_NOT_SUPPORTED (-1)

static int GetResolutionValue(int xwidth)
{
  int resolution;

  /* We need to do more here with respect to rejecting modes !! */
  switch(xwidth) {
    case 576:
      resolution=0;break;
    case 640:
      resolution=1;break;      
    case 800:
      resolution=2;break;      
    case 1024:
      resolution=3;break;      
    case 1152:
      resolution=4;break;      
    case 1280:
      resolution=5;break;      
    case 1600:
      resolution=6;break;      
    default: 
      /* Oops. Will have to switch off accel */
      resolution=RESOLUTION_NOT_SUPPORTED; 
      break;
  }
  return resolution;
}


static unsigned long currentGreen0=0;

static unsigned long SetSync(DisplayModePtr mode)
{
  unsigned long green0=0;

  if((mode->Flags & (V_PHSYNC | V_NHSYNC)) && 
     (mode->Flags & (V_PVSYNC | V_NVSYNC)) ) {
    if(mode->Flags & V_PHSYNC) {
      green0|=PFB_Def(GREEN_0_POLAR_HSYNC,POSITIVE);
    }else {
      green0|=PFB_Def(GREEN_0_POLAR_HSYNC,NEGATIVE);
    } 
    if(mode->Flags & V_PVSYNC) {
      green0|=PFB_Def(GREEN_0_POLAR_VSYNC,POSITIVE);
    }else {
      green0|=PFB_Def(GREEN_0_POLAR_VSYNC,NEGATIVE);
    } 
  }else {
    int v = (mode->Flags & V_DBLSCAN) ? mode->VDisplay*2 : mode->VDisplay;
    if(v < 400)
      green0|=PFB_Def(GREEN_0_POLAR_HSYNC,POSITIVE) | 
              PFB_Def(GREEN_0_POLAR_VSYNC,NEGATIVE);
    else if(v < 480) { 
      green0|=PFB_Def(GREEN_0_POLAR_HSYNC,NEGATIVE) | 
              PFB_Def(GREEN_0_POLAR_VSYNC,POSITIVE);
    }else if(v < 768) {
      green0|=PFB_Def(GREEN_0_POLAR_HSYNC,NEGATIVE) | 
              PFB_Def(GREEN_0_POLAR_VSYNC,NEGATIVE);
    }else {
      green0|=PFB_Def(GREEN_0_POLAR_HSYNC,POSITIVE) | 
              PFB_Def(GREEN_0_POLAR_VSYNC,POSITIVE);
    }       
  }
  currentGreen0=green0;
  /* We should also check for composite sync here as well */
  return green0;

}


#define DROP_MCLOCK_THRESHOLD 28300.0
#define MCLOCK_VCLOCK_RATIO 3.5
#define DEFAULT_MCLOCK 100669.0

static int SetMclock(float vclock,float *clockOut,int *m,int *n,int *o,int *p)
{

  /* Drive it to the max if we can */
  if(vclock > DROP_MCLOCK_THRESHOLD) {
    *m=11;*n=91;*o=1;*p=1;
    *clockOut=DEFAULT_MCLOCK;
    return 1;
  }
  /* Hmm. Will have to drop MCLOCK to allow the DAC to function 
   * correctly.
   */
  return NVClockSelect(vclock*MCLOCK_VCLOCK_RATIO,clockOut,m,n,o,p);

}

#define new ((vgaNVPtr)vgaNewVideoState)

static Bool NV1Init(DisplayModePtr mode)     
{
  int bppShift=(vgaBitsPerPixel==8)  ? 1 : 2;
  int m,n,o,p;
  int mm,nm,om,pm;
  float clockIn=(float)vga256InfoRec.clock[mode->Clock];
  float clockOut,mclockOut;
  int pclkVclkRatio;
  int i;
  int resolution;
  int dac8bit= OFLG_ISSET(OPTION_DAC_8_BIT,&vga256InfoRec.options);
  NV1Registers *nv1;
  int vertSyncStart,vertDisplay,vertTotal,vertSyncEnd;

  /* Check to see that we are not trying to do a virtual screen */
  if(!VirtualScreenOk(mode)) {
    ErrorF("%s %s: %s: Can't select mode %s : virtual desktop not supported\n",
        XCONFIG_PROBED, vga256InfoRec.name,vga256InfoRec.chipset,mode->name);
    return FALSE;
  }

  if(!NVClockSelect(clockIn,&clockOut,&m,&n,&o,&p)) {
    ErrorF("%s %s: %s: Unable to set desired video clock\n",
           XCONFIG_PROBED, vga256InfoRec.name,vga256InfoRec.chipset);
    return FALSE;
  
  }
  /* Figure out divide down for clock. The Vclk pin is rated for
   * 25-50Mhz but can actually be driven higher than this on most 
   * silicon
   */
  for(i=1,pclkVclkRatio=0;i<=16;i*=2,pclkVclkRatio++) {
    if((clockOut/(double)i) <= NV_MAX_VCLK_PIN_CLOCK_IN_KHZ) break;
  }
  ErrorF("%s %s: %s: Using %.3fMhz video clock\n",XCONFIG_PROBED, 
          vga256InfoRec.name,vga256InfoRec.chipset,clockOut/1000);

  if(!SetMclock(clockOut,&mclockOut,&mm,&nm,&om,&pm)) {
    ErrorF("%s %s: %s: Unable to set desired master clock\n",
           XCONFIG_PROBED, vga256InfoRec.name,vga256InfoRec.chipset);
    return FALSE;
  }
  ErrorF("%s %s: %s: Using %.3fMhz master clock\n",XCONFIG_PROBED,
          vga256InfoRec.name,vga256InfoRec.chipset,mclockOut/1000);

  /*
   * This will allocate the datastructure and initialize all of the
   * generic VGA registers. It doesn`t actually write to any VGA registers.
   */
  if (!vgaHWInit (mode, sizeof (vgaNVRec))) {
    return (FALSE);
  }
  nv1=&((new)->regs.nv1);  
  new->vgaValid=0;
  nv1->Mparam=m;nv1->Nparam=n;nv1->Oparam=o;nv1->Pparam=p;
  nv1->MparamMPLL=mm;nv1->NparamMPLL=nm;nv1->OparamMPLL=om;nv1->PparamMPLL=pm;
  
  nv1->dacConfReg0= PDAC_Val(CONF_0_VGA_STATE,1) | 
                    PDAC_Val(CONF_0_PORT_WIDTH,pixelPortWidth) | 
                    PDAC_Val(CONF_0_VISUAL_DEPTH,bppShift) |
                    PDAC_Val(CONF_0_IDC_MODE,vgaBitsPerPixel==8);

  nv1->dacConfReg1=PDAC_Val(CONF_1_VCLK_IMPEDANCE,1) | 
                   PDAC_Val(CONF_1_PCLK_VCLK_RATIO,pclkVclkRatio);


  nv1->dacRgbPalCtrl=(dac8bit) ? PDAC_Def(EXT_RGB_PAL_CTRL_DAC_WIDTH,BITS_8) :
                                 PDAC_Def(EXT_RGB_PAL_CTRL_DAC_WIDTH,BITS_6) ;

  nv1->prmConfig0=(dac8bit) ? PRM_Def(CONFIG_0_DAC_WIDTH,BITS_8) :
                              PRM_Def(CONFIG_0_DAC_WIDTH,BITS_6); 
  nv1->memoryTrace=0;

  resolution=GetResolutionValue(vga256InfoRec.virtualX);
  /* If we have an "error" here , just set it to zero. If we are 
   * not using any acceleration, the value doesn't matter
   */
  if(resolution==RESOLUTION_NOT_SUPPORTED) resolution=0;

  nv1->confReg0=PFB_Val(CONFIG_0_PIXEL_DEPTH,bppShift) | 
                PFB_Val(CONFIG_0_PCLK_VCLK_RATIO,pclkVclkRatio) | 
                PFB_Val(CONFIG_0_RESOLUTION,resolution) | 
                PFB_Val(CONFIG_0_SCANLINE,
                        (mode->Flags & V_DBLSCAN)==V_DBLSCAN);

  nv1->green0=SetSync(mode);
  nv1->startAddr=0;
  /* Calculate the monitor timings */
  nv1->horFrontPorch=mode->CrtcHSyncStart - mode->CrtcHDisplay+1;
  nv1->horBackPorch=mode->CrtcHTotal - mode->CrtcHSyncEnd+1;
  nv1->horSyncWidth=mode->CrtcHSyncEnd - mode->CrtcHSyncStart+1;

  nv1->horDispWidth=mode->CrtcHDisplay;

  nv1->verFrontPorch=mode->CrtcVSyncStart - mode->CrtcVDisplay+1;
  nv1->verBackPorch=mode->CrtcVTotal - mode->CrtcVSyncEnd+1;
  nv1->verSyncWidth=mode->CrtcVSyncEnd - mode->CrtcVSyncStart+1;
  nv1->verDispWidth=mode->CrtcVDisplay;

  /* The server will initialise the cursor correctly, so we just set it 
   * all to zero here
   */
  nv1->cursorCtrl=NV_PDAC_CURSOR_CTRL_A_OFF;
  nv1->xLo=0;nv1->xHi=0;nv1->yLo=0;nv1->yHi=0;
 
  for(i=0;i<3;i++) {
    nv1->colour1[i]=0;nv1->colour2[i]=0;nv1->colour3[i]=0;
  }
  for(i=0;i<NV_PDAC_CURSOR_PLANE_SIZE;i++) {
    nv1->plane0[i]=0;nv1->plane1[i]=0;
  }
  
  for(i=0;i<PALETTE_SIZE;i++) {
    nv1->palette[i][0]=0;
    nv1->palette[i][1]=0;
    nv1->palette[i][2]=0;
  }

  return (TRUE);
}

static void NV1Adjust(int x, int y)
{
  int bppShift=(vgaBitsPerPixel==8)  ? 1 : 2;

  /* Wait for vertical blank */
  while(GetBF(PFB_Read(CONFIG_0),NV_PFB_CONFIG_0_VERTICAL)==0);

  PFB_Write(START,(y * vga256InfoRec.displayWidth + x)*bppShift);
 
}

static int NV1ValidMode(DisplayModePtr mode,Bool verbose,int flag)
{
  return (MODE_OK);
}



extern vgaHWCursorRec vgaHWCursor;

static void NV1FbInit(void)
{

  if(GetResolutionValue(vga256InfoRec.virtualX)==RESOLUTION_NOT_SUPPORTED) {
    OFLG_SET(OPTION_NOACCEL,&vga256InfoRec.options);
    ErrorF("%s %s: %s: Display width must be "
           "576,640,800,1024,1152,1280 or 1600 pixels\n",
            XCONFIG_PROBED,vga256InfoRec.name,vga256InfoRec.chipset);
    ErrorF("%s %s: %s: Acceleration switched off\n",
             XCONFIG_PROBED,vga256InfoRec.name,vga256InfoRec.chipset);
  }
  /* Is this the correct thing to do ?? */
  if(!OFLG_ISSET(OPTION_SW_CURSOR, &vga256InfoRec.options)) {
    /* Initialise the hardware cursor */
    vgaHWCursor.Initialized = TRUE;
    vgaHWCursor.Init = NVCursorInit;
    vgaHWCursor.Restore = NVRestoreCursor;
    vgaHWCursor.Warp = NVWarpCursor;
    vgaHWCursor.QueryBestSize = NVQueryBestSize;
    if(xf86Verbose) {
      ErrorF("%s %s: %s: Using hardware cursor\n",XCONFIG_PROBED, 
             vga256InfoRec.name,vga256InfoRec.chipset);
    }
  }
    
  if(!OFLG_ISSET(OPTION_NOACCEL, &vga256InfoRec.options)) {
    NVAccelInit();
  }
}

static Bool NV1BlankScreen(ScreenPtr pScreen,Bool on)
{
  unsigned long green;

  green=PFB_Read(GREEN_0) & ~(PFB_Mask(GREEN_0_LEVEL));
  if(on) {
    green|=PFB_Def(GREEN_0_LEVEL,VIDEO_ENABLED);
  }else {
    green|=PFB_Def(GREEN_0_LEVEL,VIDEO_DISABLED);
  }
  PFB_Write(GREEN_0,green);
  return TRUE;
  
}


static int DGADirectMode(void)
{
#ifdef XFreeXDGA
  return ( ((vga256InfoRec.directMode & XF86DGADirectGraphics) && 
           !(vga256InfoRec.directMode & XF86DGADirectColormap))
	   || (vga256InfoRec.directMode & XF86DGAHasColormap));
#else
  return 0;
#endif
}


#ifdef DEBUG
/* If the Vclock is too low for the Mclock, the first sympton is 
 * that reads from the palette start failing. Set this to 
 * check that you can read back the palette value that you wrote
 */
#define PARANOID_PALETTE_CHECK
#endif

static void NVStoreColors(ColormapPtr pmap,int ndef,xColorItem *pdefs)
{
  int           i;
  xColorItem    directDefs[256];
  int shift=(vgaDAC8BitComponents) ? 8 : 10;

  if(vgaCheckColorMap(pmap)) return;
  if((pmap->pVisual->class | DynamicClass) == DirectColor) {
    ndef = cfbExpandDirectColors (pmap, ndef, pdefs, directDefs);
    pdefs = directDefs;
  }

 if(!(xf86VTSema || DGADirectMode())) return;

  for(i=0;i<ndef;i++) {
    int r,g,b;
    PDAC_Write(WRITE_PALETTE,pdefs[i].pixel);
    r=(pdefs[i].red >> shift);g=(pdefs[i].green >> shift);
    b=(pdefs[i].blue >> shift);
    PDAC_Write(COLOUR_DATA,r);
    PDAC_Write(COLOUR_DATA,g);
    PDAC_Write(COLOUR_DATA,b);
#ifdef PARANOID_PALETTE_CHECK
    /* Checks that what you write can be read ! */
    {
      int rout,bout,gout;
      PDAC_Write(READ_PALETTE,pdefs[i].pixel);
      rout=PDAC_Read(COLOUR_DATA);gout=PDAC_Read(COLOUR_DATA);
      bout=PDAC_Read(COLOUR_DATA);
      if(rout!=r || gout!=g || bout!=b) 
        ErrorF("Entry %d In -> [%02x:%02x:%02x] Out -> [%02x:%02x:%02x]\n",
                 pdefs[i].pixel,r,g,b,
                 rout,gout,bout);
    }
#endif
  }

}

static Bool NV1ScreenInit(ScreenPtr pScreen,pointer pbits, 
                         int xsize,int ysize,int dpix,int dpiy,int width)
{
  /* Hook the palette function. The standard VGA one sets overscan which 
   * results in a corrupt pixel on the screen as the NV1 will write it 
   * through to the framebuffer
   */
  pScreen->StoreColors = NVStoreColors;
}


#ifdef DPMSExtension
static void NV1DisplayPowerManagementSet(int mode)
{
  int level;
  int hsyncOn=0,vsyncOn=0;
  unsigned long green=0;

  if (!xf86VTSema) return;

  switch(mode) {
    case DPMSModeOn:
      /* Screen: On; HSync: On, VSync: On */
      level=0;hsyncOn=1;vsyncOn=1;
      break;
    case DPMSModeStandby:
      /* Screen: Off; HSync: Off, VSync: On */
      level=1;hsyncOn=0;vsyncOn=1;
      break;
    case DPMSModeSuspend:
      /* Screen: Off; HSync: On, VSync: Off */
      level=1;hsyncOn=1;vsyncOn=0;
      break;
    case DPMSModeOff:
      /* Screen: Off; HSync: Off, VSync: Off */
      level=2;hsyncOn=0;vsyncOn=0;
      break;
  }

  if(hsyncOn) {
    green|= (currentGreen0 & PFB_Mask(GREEN_0_POLAR_HSYNC));
  }else { 
    green|=PFB_Def(GREEN_0_POLAR_HSYNC,LOW);
  }
  if(vsyncOn) {
    green|= (currentGreen0 & PFB_Mask(GREEN_0_POLAR_VSYNC));
  }else { 
    green|=PFB_Def(GREEN_0_POLAR_VSYNC,LOW);
  }
  green|=PFB_Val(GREEN_0_LEVEL,level);
  PFB_Write(GREEN_0,green);

}
#endif

static void NV1SaveScreen(int on)
{
}

static void NV1GetMode(DisplayModePtr display)
{
}

/* Changes the entries in the NV struct to point at the correct function 
 * pointers. Called from the Probe() function
 */
static void NV1FlipFunctions(vgaVideoChipRec *nv)
{
  nv->ChipEnterLeave=NV1EnterLeave;
  nv->ChipInit=NV1Init;
  nv->ChipValidMode=NV1ValidMode;
  nv->ChipSave=NV1Save;
  nv->ChipRestore=NV1Restore;
  nv->ChipAdjust=NV1Adjust;
  nv->ChipSaveScreen=NV1SaveScreen;
  nv->ChipGetMode=NV1GetMode;
  nv->ChipFbInit=NV1FbInit;

}

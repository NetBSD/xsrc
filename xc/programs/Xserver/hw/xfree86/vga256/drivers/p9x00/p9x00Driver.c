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
/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/p9x00/p9x00Driver.c,v 1.1.2.2 1998/09/13 12:29:12 hohndel Exp $ */

#define P9X00DRIVER_C
/*
 * These are X and server generic header files.
 */
#include "X.h"
#include "input.h"
#include "screenint.h"

/*
 * These are XFree86-specific header files
 */
#include "compiler.h"
#include "xf86.h"
/*#include "xf86Version.h"
*/
#include "xf86Priv.h"
#include "xf86_OSlib.h"
#include "xf86_HWlib.h"
#include "vga.h"

/*
 * If the driver makes use of XF86Config 'Option' flags, the following will be
 * required
 */
#define XCONFIG_FLAGS_ONLY
#include "xf86_Config.h"

/* DGA includes */
#ifdef XFreeXDGA
#include "X.h"
#include "Xproto.h"
#include "scrnintstr.h"
#include "servermd.h"
#define _XF86DGA_SERVER_
#include "extensions/xf86dgastr.h"
#endif

/*
#ifdef DPMSExtension
#include "extensions/dpms.h"
#endif
*/

extern Bool xf86Exiting, xf86Resetting;
extern char *xf86VisualNames[];

static ScreenPtr savepScreen = NULL;

/*
 * This header is required for drivers that implement FbInit().
 */
#include "vga256.h"

#include "p9x00Regs.h"
#include "p9x00Access.h"
#include "p9x00Probe.h"
#include "p9x00VGA.h"
#include "p9x00ICD2061A.h"
#include "p9x00DAC.h"
#include "p9x00XAA.h"
#include "p9x00Driver.h"

ScreenPtr p9x00savepScreen;
Bool      p9x00Initialized=FALSE;
Bool      p9x00SeverGeneration=TRUE;

static CARD8 p9x00saved_palette[768];

/*
 * Forward definitions for the functions that make up the driver.  See
 * the definitions of these functions for the real scoop.
 */
static void     p9x00EnterLeave();
static Bool     p9x00Init();
static int	p9x00ValidMode();
static void *   p9x00Save();
static void     p9x00Restore();
static void     p9x00Adjust();
static void     p9x00SaveScreen();
static void     p9x00GetMode();
static void	p9x00FbInit();

/*
 * This data structure defines the driver itself.  The data structure is
 * initialized with the functions that make up the driver and some data 
 * that defines how the driver operates.
 */
vgaVideoChipRec P9X00 = {
	p9x00Probe,
	p9x00Ident,
	p9x00EnterLeave,
	p9x00Init,
	p9x00ValidMode,
	p9x00Save,
	p9x00Restore,
	p9x00Adjust,
	p9x00SaveScreen,
	p9x00GetMode,
	p9x00FbInit,
	(void (*)())NoopDDA,
	(void (*)())NoopDDA,
	(void (*)())NoopDDA,
	0x10000,		
	0x10000,
	16,
	0xFFFF,
	0x00000, 0x10000,
	0x00000, 0x10000,
	FALSE,
	VGA_NO_DIVIDE_VERT,
	{0,},
	8,
	FALSE,
	0, /* Framebuffer Addresse */
	0, /* Framebuffer Size */
	TRUE,  /* 16bpp */
	TRUE,  /* 24bpp */
	TRUE,  /* 32bpp */
	NULL, /* builtin driver modes */
	1,
	1
};

#ifdef XFree86LOADER
XF86ModuleVersionInfo p9x00VersRec =
{
	"p9x00_drv.o",
	MODULEVENDORSTRING,
	MODINFOSTRING1,
	MODINFOSTRING2,
	XF86_VERSION_CURRENT,
	0x00010001,
	{0,0,0,0}
};
/*
 * this function returns the vgaVideoChipPtr for this driver
 *
 * its name has to be ModuleInit()
 */
void ModuleInit(pointer *data,INT32 *magic)
{
    static int cnt = 0;

    switch(cnt++)
    {
    /* MAGIC_VERSION must be first in ModuleInit */
    case 0:
    	* data = (pointer) &p9x00VersRec;
	* magic= MAGIC_VERSION;
	break;
    case 1:
        * data = (pointer) &P9X00;
        * magic= MAGIC_ADD_VIDEO_CHIP_REC;
        break;
    default:
        xf86issvgatype = TRUE; /* later load the correct libvgaxx.a */
        * magic= MAGIC_DONE;
        break;
    }

    return;
}
#endif /* XFree86LOADER */


static void p9x00CalcClipMax(DisplayModePtr mode,vgap9x00Ptr state)
{
  CARD32 memsize=P9X_CFG_SIZE*1024*1024;
  CARD32 maxy=memsize/(mode->CrtcHDisplay*state->BpPix);
  
  state->byteclipmax=((CARD32)(mode->CrtcHDisplay)*(CARD32)(state->BpPix)-1)<<16;
  state->byteclipmax|=maxy&0xFFFFL;
  state->clipmax=(CARD32)(mode->CrtcHDisplay-1)<<16;
  state->clipmax|=maxy&0xFFFFL;
  p9x00DefaultByteClipping=state->byteclipmax;
  p9x00DefaultPixelClipping=state->clipmax;
}


static Bool p9x00CalcSysCfg(DisplayModePtr mode,vgap9x00Ptr state)
{
  int remhres=mode->CrtcHDisplay*state->BpPix;
  int actualhrespart;
  CARD32 actualshiftvalue;
  int hrespart=2048;
  CARD32 shiftvalue=7; 
  int shift;
  int partcount;
    
  state->syscfg=P9X_RV_MEM_SWAP_BYTES|P9X_RV_MEM_SWAP_WORDS;
  if (P9X_CFG_CHIP==P9X_V_P9100) {
    switch (state->BpPix) {
      case 1:
        state->syscfg|=P9X_RV_COLOR_MODE(2);
        break;
      case 2:
        state->syscfg|=P9X_RV_COLOR_MODE(3);
        break;
      case 3:
        state->syscfg|=P9X_RV_COLOR_MODE(7);
        break;
      case 4:
        state->syscfg|=P9X_RV_COLOR_MODE(5);
        break;
    }
    if (remhres>=4096) {
      state->syscfg|=P9X_RV_SHIFT_3(3);
      remhres-=4096;
    }
    else if (remhres>=2048) {
      state->syscfg|=P9X_RV_SHIFT_3(2);
      remhres-=2048;
    }
    else if (remhres>=1024) {
      state->syscfg|=P9X_RV_SHIFT_3(1);
      remhres-=1024;
    }
  }
  for (shift=20;shift>=14;shift-=3) {
    actualhrespart=hrespart;
    actualshiftvalue=shiftvalue;
    for (partcount=0;partcount<5;partcount++) {
      if (remhres>=actualhrespart) {
        state->syscfg|=P9X_RV_SHIFT(shift,actualshiftvalue);
        remhres-=actualhrespart;
        break;
      }
      actualhrespart>>=1;
      actualshiftvalue--;
    }
    hrespart>>=1;
    shiftvalue--;
  }  
  if (remhres)
    return(FALSE);
  return(TRUE);
}


/* Thanks to Mr. David Wyatt (P9X00-Windows95-driver-developer),
   for his important informations and code-samples. Without his 
   help the following code would NOT exist   
*/
   

static CARD32 logbase2(CARD32 n)
{
  CARD32 result = 0;

  while (n != 1) {
    result++;
    n >>= 1;
  }
  return result;
}

/*
  Tables for handling blnkdly when the P9100 divides the pixel clock
  Index these tables with:
       vram type       0 - 128k, 1 - 256k
       ramdac width    0 - 32 bits, 1 - 64 bits
       # of banks      0 - 1 bank, 1 - 2 banks, 3 - 4 banks
*/
static pos_blnkdly_tbl[2][2][3] =
  /*   1b    2b    4b */
  {{{0xff, 0x00, 0x00},   /* 128k VRAM, 32-bit RAMDAC */
    {0xff, 0x00, 0x00}},  /* 128k VRAM, 64-bit RAMDAC */
   {{0x01, 0x02, 0x00},   /* 256k VRAM, 32-bit RAMDAC */
    {0xff, 0x01, 0x02}}}; /* 256k VRAM, 64-bit RAMDAC */
static neg_blnkdly_tbl[2][2][3] =
  /*   1b    2b    4b */
  {{{0xff, 0x02, 0x03},   /* 128k VRAM, 32-bit RAMDAC */
    {0xff, 0x00, 0x00}},  /* 128k VRAM, 64-bit RAMDAC */
   {{0x01, 0x02, 0x03},   /* 256k VRAM, 32-bit RAMDAC */
    {0xff, 0x01, 0x02}}}; /* 256k VRAM, 64-bit RAMDAC */


static Bool p9x00CalcMemCfgScrTiming(vgap9x00Ptr state)
{
  long LoadClock;

  /* The Bits of the Memory-Configuration-Register */
  CARD32 config;
  CARD32 vram_miss_adj = 0;
  CARD32 vram_read_adj = 1;
  CARD32 vram_write_adj = 1;
  CARD32 priority_select = 0;
  CARD32 dac_access_adjust = 0;
  CARD32 dac_mode = 0;
  CARD32 hold_reset = 0;
  CARD32 shiftclk_freq;
  CARD32 crtc_freq;
  CARD32 blank_edge;
  CARD32 video_clk_sel;
  CARD32 vad_sht;
  CARD32 shiftclk_mode;
  CARD32 soe_mode;
  CARD32 blnkdly;
  CARD32 slow_host_intf = 1;
  CARD32 vram_read_sample = 1;
  CARD32 division_factor;

  /* The Bits of the Screen-Timing-Control-Register */
  CARD32 qsfselect;
  CARD32 display_buffer = 0;
  CARD32 hblnk_reload = 0;
  CARD32 enable_video = 1;
  CARD32 internal_hsync = 1;
  CARD32 internal_vsync = 1;
  CARD32 src_incs;
  
  /* Miscellaneous intermediates */
  CARD32 effective_backend_banks;
  CARD32 depth_of_one_sam;
  CARD32 effective_sam_depth;
  CARD32 effective_row_depth;

  state->dacdivides=TRUE;
  state->memspeed=45000000L;
  if (P9X_CFG_CHIP==P9X_V_P9100) {
    /* Compute the intermediate values... */
    division_factor = P9X_CFG_DW/((CARD32)8 * (CARD32)state->BpPix);

    LoadClock = (state->dotfreq / P9X_CFG_DW) * (CARD32)state->BpPix * (CARD32)8;

    if ((P9X_CFG_DAC==P9X_V_BT489) && (state->BpPix==3)) {
      /* Bt489 True Color modes require blank_delay = 2 ie. blank_edge = 1 */
      blank_edge = 1;
    }
    else {
      /* Use positive edge if Load clock if greater than 33 Mhz.. */
      blank_edge = (LoadClock > 33000000L) ? 0 : 1;
    }

    effective_backend_banks = ((CARD32)32*P9X_CFG_BANKS)/P9X_CFG_DW;
    depth_of_one_sam = (P9X_CFG_MEM==P9X_V_VRAM256) ? 512 : 256;
    effective_sam_depth = depth_of_one_sam * effective_backend_banks;
    effective_row_depth = depth_of_one_sam * P9X_CFG_BANKS;

    /* (1) Compute mem_config.config[2..0] */
    config = P9X_CFG_BANKS - 1;

    /* Add mod's for VRAM type */
    if (P9X_CFG_MEM==P9X_V_VRAM256) 
      config |= 0x04;

    /* (2) Make sure that we don't try to have the P9100 divide
       the pixel clock when it shouldn't
    
       (i) If we are running in 24bpp packed mode, then we must
       let the RAMDAC divide the pixel clock to generate the
       load clock.

       (ii) If (LCLK == pixclock) then it is best to use the SCLK from
       the RAMDAC because of clock doubler complications.

       (iii) If we're using A3 silicon we don't need to have the
       P9100 divide the clock. Finally!!!
    */

    if ((P9X_CFG_DAC == P9X_V_ATT511) && (state->BpPix==3)) {
      shiftclk_freq = 001;
      crtc_freq = 001;
    }
    else {
      shiftclk_freq = 000;
      crtc_freq = 000;
    }


    if (P9X_CFG_DAC==P9X_V_BT489) {
      if (LoadClock > 48500000) {
        /* invert the sclk polarity to avoid the forbidden region */
        state->InvertSCLK = TRUE;
      }
      else {
        state->InvertSCLK = FALSE;
      }
    }

/*
   WTK CTI - SPLIT SHIFT TRANSFER work around.
  
   This is the main logic for the split shift 
   transfer bug software work around. The current 
   assumption is that the RAMDAC will always be 
   doing the dividing of the clock.
  
   Special attention is required for A1/A2 silicon 
   for low resolution modes such as 640x480x8, 
   640x480x15, 640x480x16 and 800x600x8.  Furthormore,
   the problem only occurs on boards with 2 megabytes 
   of VRAM, a 64-bit RAMDAC and when the following 
   condition is met.
  
      (SCLK * 7) < MEMCLK
  
   Note: The value calculated for LCLK can also be 
         used in place of SCLK
         in the computations.
*/

    if ((P9X_CFG_REV==2) && 
        (state->BpPix <= 2) &&
	( ((P9X_CFG_BANKS==2) &&
	   (P9X_CFG_DW==64) && 
	   (LoadClock < 12000000)
          ) || 
          (LoadClock < 8000000) 
        ) 
       ) {
      if (state->dotfreq <= 67500000 ) {                                                                                          
        video_clk_sel = 0;
        shiftclk_freq = 2 - state->BpPix + (P9X_CFG_DW/32);
        crtc_freq     = shiftclk_freq;
        state->dacdivides = FALSE;
      }
      else {
        video_clk_sel = 0;
        shiftclk_freq = 1 - state->BpPix + (P9X_CFG_DW/32);
	crtc_freq     = shiftclk_freq;
	state->dacdivides = FALSE;
      }
    }
    else
      video_clk_sel = 1;

    if ((P9X_CFG_REV < 3) &&
        (state->BpPix != 3) &&
        (P9X_CFG_DW==64) &&
        (P9X_CFG_BANKS==2) &&
        (P9X_CFG_DW==64) &&
        (P9X_CFG_CLK != P9X_V_OSCCLOCK) &&
        ((LoadClock * 7) < state->memspeed)
       ) {
      /*
         All the conditions are right for the split shift 
         transfer bug to occur. The software fix for this 
         bug requires that the memory clock is adjusted
         so that the (SCLK * 7) < MEMCLK equation is no 
         longer satisfied.  This is done easily enough by 
         setting MEMCLK = SCLK * 7.  By doing this, MEMCLK 
         is not reduced any more than neccessary.
      */
      state->memspeed = LoadClock * 7;
    }

    /* (4) Compute shiftclk_mode and soe_mode */
    shiftclk_mode = soe_mode = logbase2(effective_backend_banks);

    /* (5) Compute qsfselect, src_incs, and vad_sht */
    qsfselect = logbase2(effective_sam_depth) - 5;
    src_incs = logbase2(effective_row_depth) - 9;
    vad_sht = 0;

    /* (6) Adjust values from step (5) for half-sam vram */
    if (P9X_CFG_SAM==P9X_V_HALFSAM) {
      qsfselect--;
      if (src_incs != 0)
        src_incs--;
      else
        vad_sht = 1;
    }

    /* (8) Compute blnkdly */
    if (P9X_CFG_REV >= 3) {
      blnkdly = 0x01;
      blank_edge = 1;	/* Use negative edge all the time */
      /* Handle a special case */
      if ((P9X_CFG_BANKS == 4) && (P9X_CFG_DW==32))
        blnkdly++;
    }
    else if (video_clk_sel == 1) {
      /* RAMDAC divides pixel clock */
      if (blank_edge == 0)
        /* positive edge of vidoutclk generates blank */
        blnkdly = 0x01;
      else
        /* negative edge of vidoutclk generates blank */
        blnkdly = 0x02;
      /* Handle a special case */
      if ((P9X_CFG_BANKS == 4) && (P9X_CFG_DW==32))
        blnkdly++;
    }
    else {
      /* P9100 divides the pixel clock */
      if (blank_edge == 0)
	/* positive edge of vidoutclk generates blank */
        blnkdly = pos_blnkdly_tbl[(P9X_CFG_MEM==P9X_V_VRAM256)? 1:0]
                                 [(P9X_CFG_DW==64)? 1:0]
                                 [P9X_CFG_BANKS>>1];
      else
	/* negative edge of vidoutclk generates blank */
	blnkdly = neg_blnkdly_tbl[(P9X_CFG_MEM==P9X_V_VRAM256)? 1:0]
	                         [(P9X_CFG_DW==64)? 1:0]
	                         [P9X_CFG_BANKS>>1];

      /* Handle some special cases */
      if ((state->BpPix==4) && 
          (P9X_CFG_BANKS==1) &&
          (P9X_CFG_MEM==P9X_V_VRAM256) && 
          (P9X_CFG_DW==32)
         )
        blnkdly = 0x02;
      if ((state->BpPix==4) &&
          (P9X_CFG_BANKS==2) &&
          (P9X_CFG_MEM==P9X_V_VRAM256) &&
          (P9X_CFG_DW==32)
         )
        blnkdly = 0x01;
    }


    /* (9) Combine all of these fields to generate 
       Memory-Configuration and Screen-Repaint-Timing-Control
    */
    state->memctrl = config
      | (vram_miss_adj << 3)
      | (vram_read_adj << 4)
      | (vram_write_adj << 5)
      | (priority_select << 6)
      | (dac_access_adjust << 7)
      | (dac_mode << 8)
      | (hold_reset << 9)
      | (shiftclk_freq << 10)
      | (crtc_freq << 13)
      | (blank_edge << 19)
      | (video_clk_sel << 20)
      | (vad_sht << 21)
      | (shiftclk_mode << 22)
      | (soe_mode << 24)
      | (blnkdly << 27)
      | (slow_host_intf << 30)
      | (vram_read_sample << 31);

    state->timingctrl= qsfselect
      | (display_buffer << 3)
      | (hblnk_reload << 4)
      | (enable_video << 5)
      | (1 << 6)
      | (internal_hsync << 7)
      | (internal_vsync << 8)
      | (src_incs << 9);
  }
  else {
    /* Screen-Repaint-Timing-Control */
    if (P9X_CFG_BUS==P9X_V_PCIBUS) {
      if (P9X_CFG_SEQ_MASK==0x6C) {
        /* Viper-Masking */
        if (P9X_CFG_SIZE==2)
          state->timingctrl = 0x000001E5L;  
        else
          state->timingctrl = 0x000001E4L;  
      }
      else {
        if (P9X_CFG_SIZE==2)
          state->timingctrl = 0x000001E4L;  
        else	
          state->timingctrl = 0x000001E3L;  
      }
    }
    else {
      if (P9X_CFG_SIZE==2)
        state->timingctrl = 0x000001E5L;  
      else
        state->timingctrl = 0x000001E4L;  
    }

    /* Memory-Configuration */
    if (P9X_CFG_SIZE==2)
      state->memctrl = 0x02;
    else {
      if (P9X_CFG_MEM==P9X_V_VRAM256)
        state->memctrl = 0x00;
      else
        state->memctrl = 0x01;
    }
  }
}


void p9x00CalcCRTC(DisplayModePtr mode,vgap9x00Ptr state)
{
  CARD32 num,den;
  CARD32 BlnkDly;

  /* First we have to figure out the adjustment factors
     for the timing equations.
  */
  if (state->BpPix==3) {
    /* 24-bit color */
    num=3;
    den=P9X_CFG_DW/8;
  }
  else {
    num=1;
    den=P9X_CFG_DW/(state->BpPix*8); /* must reconvert from av. */
  }
  
  /* Blank delay is affected, in part, by shift clock */
  BlnkDly=(state->memctrl>>27)&0x3L;

  /* This part calculates the video-control values */
  
  state->hrzt  =(CARD32)
    ((mode->CrtcHTotal/(den*2))*(num*2) - 1);
  state->hrzsr =(CARD32)
    ((((mode->CrtcHSyncEnd-mode->CrtcHSyncStart)/den)*num)-1);
  state->hrzbr =(CARD32)
    ((((mode->CrtcHTotal-mode->CrtcHSyncStart)/den)*num)-BlnkDly-4);
  state->hrzbf =(CARD32)
    ((((mode->CrtcHDisplay+(mode->CrtcHTotal-mode->CrtcHSyncStart))/den)*num)-BlnkDly-4);

  if (P9X_CFG_DAC==P9X_V_BT485 ||
      P9X_CFG_DAC==P9X_V_BT485A ||
      P9X_CFG_DAC==P9X_V_BT489) {
    if (state->BpPix==1) {
      state->hrzbr-=12*num/den;
      state->hrzbf-=12*num/den;
    }
    else {
      state->hrzbr-=9*num/den;
      state->hrzbf-=9*num/den; 
    }
  }
	
  state->vrtt  =(CARD32)(mode->CrtcVTotal);
  state->vrtsr =(CARD32)(mode->CrtcVSyncEnd-mode->CrtcVSyncStart);
  state->vrtbr =(CARD32)(mode->CrtcVTotal-mode->CrtcVSyncStart);
  state->vrtbf =state->vrtbr+(CARD32)(mode->CrtcVDisplay);

  if (P9X_CFG_DAC==P9X_V_BT485 ||
      P9X_CFG_DAC==P9X_V_BT485A ||
      P9X_CFG_DAC==P9X_V_BT489) {
    while (state->hrzsr>=state->hrzbr) {
      if (state->hrzsr>(state->hrzt-state->hrzbf))
        state->hrzsr--;
      else {
        state->hrzbr++;
        state->hrzbf++;
      }
    }
  }

  if (mode->Flags & V_NHSYNC) 
    state->hpol=-1;
  else
    state->hpol=1;
  if (mode->Flags & V_NVSYNC) 
    state->vpol=-1;
  else
    state->vpol=1;
}


void p9x00EnterNativeMode(vgap9x00Ptr state)
{
  CARD32 temp;
  
  P9X00_ENABLE();
  /* !!!! System-Configuration must come first 
     since we set Endianess here !!!! */
  P9X_R_SYSCFG=state->syscfg;
    
  p9x00read_LUT_regs(0,256,p9x00saved_palette);
  
  if (P9X_CFG_CHIP==P9X_V_P9100)
    /* Initial Default = 2 Banks of 256Kx16 VRAMs, full SAM */
    P9X_R_MEMCFG=0x114A48BDL;

  P9X_R_INTEN=P9X_RV_DISABLE_INT;
  P9X_R_REFPER=16L*(state->memspeed/1000000L);
  P9X_R_RASLOW=0x000000FAL;
  P9X_R_MODE=0x0000000AL;
  P9X_R_WOFFSET=0x00000000L;
  P9X_R_PORIGX=0x00000000L;
  P9X_R_PORIGY=0x00000000L;
  
  if (P9X_CFG_CHIP==P9X_V_P9100)
    P9X_R_PMASK=0xFFFFFFFFL; /* Write to all 32 planes */
  else
    P9X_R_PMASK=0x000000FFL; /* Write to all 8 planes */

  P9X_R_WMINB=0x00000000L;
  P9X_R_WMAXB=state->byteclipmax;
  P9X_R_WMIN=0x000000000L;
  P9X_R_WMAX=state->clipmax;

  switch(P9X_CFG_DAC) {
    case P9X_V_IBM525 :
      ibm525SetPort(state,P9X_SET_VRAMPORT);
      ibm525SetClock(state,P9X_SET_PIXCLK);
      break;
#if 0
    case P9X_V_ATT511 :
      att511SetClock(state,P9X_SET_PIXCLK);
      att511SetPort(state,P9X_SET_VRAMPORT);
      break;
    case P9X_V_BT489 :
    default :
      bt485SetClock(state,P9X_SET_PIXCLK);
      bt485SetPort(state,P9X_SET_VRAMPORT);
      break;
#endif
  }
  /* These MUST come after setting up the dac ! */
  P9X_R_MEMCFG=state->memctrl;
  P9X_R_SCRTIME=state->timingctrl;
  P9X_R_HCP=0x00000000L;
  P9X_R_VCP=0x00000000L;

  /* must come after setting up the MEMCONF ! */
  /* Setup CRTC timing & Sync Polarity */
  if (P9X_CFG_CHIP==P9X_V_P9100)
    P9X_R_SYNC=(state->hpol<0)? 0x00000001L:0x00000000L|
               (state->vpol<0)? 0x00000004L:0x00000000L;
  else {
  }

  P9X_R_HBF=state->hrzbf;
  P9X_R_HBR=state->hrzbr;
  P9X_R_HSR=state->hrzsr;
  P9X_R_HT=state->hrzt;
  P9X_R_VSR=state->vrtsr;
  P9X_R_VBR=state->vrtbr;
  P9X_R_VBF=state->vrtbf;
  P9X_R_VT=state->vrtt;

  P9X_R_HBR=state->hrzbr;
  P9X_R_HBF=state->hrzbf;

  switch(P9X_CFG_DAC) {
    case P9X_V_IBM525 :
      ibm525SetClock(state,P9X_SET_MEMCLK);
      break;
#if 0
    case P9X_V_ATT511 :
      att511SetClock(state,P9X_SET_MEMCLK);
      break;
    case P9X_V_BT489 :
    default :
      bt485SetClock(state,P9X_SET_MEMCLK);
      break;
#endif
  }

  usleep(4000);
}


static void p9x00LeaveNativeMode(vgap9x00Ptr state)
 {
  state->dotfreq=28333333;

  switch(P9X_CFG_DAC) {
    case P9X_V_IBM525 :
      ibm525SetPort(state,P9X_SET_VGAPORT);
      ibm525SetClock(state,P9X_SET_PIXCLK);
      break;
#if 0
    case P9X_V_ATT511 :
      att511SetClock(state,P9X_SET_PIXCLK);
      att511SetPort(state,P9X_SET_VGAPORT);
      break;
    case P9X_V_BT489 :
    default :
      bt485SetClock(state,P9X_SET_PIXCLK);
      bt485SetPort(state,P9X_SET_VGAPORT);
      break;
#endif
  }

/* Restore the DAC regs before leaving Native mode
   Need to put some test here to check if it is primary VGA
	(*npfnRestoreExtDacRegs)(lpP9XEPS);
*/
  p9x00write_LUT_regs(0,256,p9x00saved_palette);

  P9X_R_MEMCFG=0x200000L;
  P9X_R_SYSCFG=0x0L;
  P9X_R_SCRTIME=0x0L;
  P9X_R_SYNC=0x0L;

  P9X00_DISABLE();
}


/*
 * p9x00Restore --
 *
 */
static void p9x00Restore(vgap9x00Ptr restore)
{
  if (restore->vga_mode) {
    p9x00LeaveNativeMode(restore);
    ErrorF("POWER OFF\n");
    w5x86Restore();
    p9x00Initialized = FALSE;
  }
  else {
    ErrorF("POWER ON\n");
    p9x00EnterNativeMode(restore);
    P9X00_ENABLEPORTS;
    p9x00Initialized = TRUE;
  } 
}

/*
 * p9x00Save --
 *
 */
static void *p9x00Save(vgap9x00Ptr save)
{
  save=(vgap9x00Ptr)xcalloc(1,sizeof(vgap9x00Rec));
  if (!p9x00Initialized) {
    w5x86Save();
    save->vga_mode=TRUE;
    save->dotfreq=28333333;
  }
  else {
    save->vga_mode=FALSE;
  }    
  return ((void *) save);
}

#define p9x00state ((vgap9x00Ptr)vgaNewVideoState)

/*
 * p9x00Init --
 *
 */
static Bool p9x00Init(DisplayModePtr mode)
{
  
  if (!vgaHWInit(mode,sizeof(vgap9x00Rec)))
    return FALSE;

  p9x00state->BpPix=(vga256InfoRec.bitsPerPixel+1)/8;
  p9x00BpPix=p9x00state->BpPix;
  mode->SynthClock=vga256InfoRec.clock[mode->Clock];
  p9x00state->dotfreq = (CARD32)mode->SynthClock * (CARD32)1000L;

  p9x00CalcClipMax(mode,p9x00state);
  
  if (!p9x00CalcMemCfgScrTiming(p9x00state))
    return FALSE;
  if (!p9x00CalcSysCfg(mode,p9x00state))
    return FALSE;
  p9x00CalcCRTC(mode,p9x00state);

  p9x00state->vga_mode=FALSE;
  return TRUE;
}

/*
 * p9x00EnterLeave --
 *
 */
static void p9x00EnterLeave(Bool enter)
{
  if (enter) {
  } 
  else {
  }
}

/*
 * p9x00Adjust --
 *
 * This function is used to initialize the SVGA Start Address - the first
 * displayed location in the video memory.  This is used to implement the
 * virtual window.
 */
static void p9x00Adjust(int x, int y)
{
}

/*
 * p9x00SaveScreen --
 *
 */
static void p9x00SaveScreen(int mode)
{
  if (mode == SS_START) {
  }
  else {
  }
}

/*
 * p9x00GetMode --
 *
 * This function will read the current SVGA register settings and produce
 * a filled-in DisplayModeRec containing the current mode.
 *
 * Note that the is function is NOT used in XFree86 1.3, hence in a real
 * driver you should put 'NoopDDA' in the vgaVideoChipRec structure.  At
 * some point in the future, this function will be used to implement
 * interactive mode setting, and drivers will be required to supply it.
 */
static void p9x00GetMode(DisplayModePtr mode)
{
}


/*
 * p9x00FbInit --
 *
 */
static void p9x00FbInit()
{
  if (P9X_CFG_CHIP==P9X_V_P9100)
    P9X00.ChipLinearBase = vga256InfoRec.MemBase+0x800000;
  else
    P9X00.ChipLinearBase = vga256InfoRec.MemBase+0x200000;
  P9X00.ChipLinearSize = vga256InfoRec.videoRam * 1024;
  P9X00.ChipUseLinearAddressing=TRUE;
  /* Map the registers */
  p9x00Init_Access();
  			
  if(!OFLG_ISSET(OPTION_NOACCEL, &vga256InfoRec.options)) 
	{}
  p9x00AccelInit();

  switch (P9X_CFG_DAC) {
    case P9X_V_IBM525:
      ibm525InitCursor();
  }
}

static int p9x00ValidMode(DisplayModePtr mode, Bool verbose, int flag)
{
  return(MODE_OK);
}

void p9x00StoreColors(ColormapPtr pmap, int ndef, xColorItem *pdefs)
{
  int i;
  xColorItem directDefs[256];
  int shift=(vgaDAC8BitComponents)? 8:10; 
  if (vgaCheckColorMap(pmap)) return;
  if ((pmap->pVisual->class|DynamicClass) == DirectColor) {
    ndef=cfbExpandDirectColors (pmap,ndef,pdefs,directDefs);
    pdefs=directDefs;
  }
  if (!(xf86VTSema
#ifdef XFreeXDGA
                  ||( ((vga256InfoRec.directMode & XF86DGADirectGraphics) &&
                      !(vga256InfoRec.directMode & XF86DGADirectColormap))
                      ||(vga256InfoRec.directMode & XF86DGAHasColormap))
#endif
     )) return;

  for (i=0;i<ndef;i++) {
    P9X00_WRITEDAC(DAC_WRITE_ADDR, (CARD8)pdefs[i].pixel);
    P9X00_WRITEDAC(DAC_RAMDAC_DATA, (CARD8)(pdefs[i].red>>shift));
    P9X00_WRITEDAC(DAC_RAMDAC_DATA, (CARD8)(pdefs[i].green>>shift));
    P9X00_WRITEDAC(DAC_RAMDAC_DATA, (CARD8)(pdefs[i].blue>>shift));
  }
}


Bool p9x00ScreenInit(ScreenPtr pScreen, pointer pbits, 
                    int xsize, int ysize, int dpix, int dpiy, int width)
{
  pScreen->StoreColors=p9x00StoreColors;
}

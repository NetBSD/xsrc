/* $XFree86: xc/programs/Xserver/hw/xfree86/accel/glint/glint_init.c,v 1.20.2.8 1999/12/10 12:38:19 hohndel Exp $ */
/*
 * Copyright 1997 by Alan Hourihane <alanh@fairlite.demon.co.uk>
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Alan Hourihane not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  Alan Hourihane makes no representations
 * about the suitability of this software for any purpose.  It is provided
 * "as is" without express or implied warranty.
 *
 * ALAN HOURIHANE DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL ALAN HOURIHANE BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 *
 * Authors:  Alan Hourihane, <alanh@fairlite.demon.co.uk>
 *           Dirk Hohndel,   <hohndel@suse.de>
 *	     Stefan Dirsch,  <sndirsch@suse.de>
 *	     Helmut Fahrion, <hf@suse.de>
 *
 * this work is sponsored by S.u.S.E. GmbH, Fuerth, Elsa GmbH, Aachen and
 * Siemens Nixdorf Informationssysteme
 */

#include "X.h"
#include "Xmd.h"
#include "input.h"
#include "servermd.h"
#include "scrnintstr.h"
#include "site.h"

#include "xf86Procs.h"
#include "xf86_OSlib.h"
#include "xf86_HWlib.h"
#include "vga.h"
#include "glint_regs.h"
#define GLINT_SERVER
#include "IBMRGB.h"
#include "glint.h"
#include "xf86_Config.h"

#define vga256InfoRec glintInfoRec

typedef struct {
	unsigned long glintRegs[0x200];
	unsigned long DacRegs[0x100];
} glintRegisters;
static glintRegisters SR;

static Bool LUTInited = FALSE;
static LUTENTRY oldlut[256];
int VBlank;
int glintHDisplay;
extern struct glintmem glintmem;
extern Bool UsePCIRetry;
extern int glintWeight;
extern int glintDisplayWidth;
extern Bool glintDoubleBufferMode;
extern Bool VGAcore;
void glintDumpRegs(void);
unsigned int glintSetLUT(int , unsigned int );
int pprod = -2;
int Bppshift = 0;
extern int coprotype;
extern glintLBvideoRam;

#define PARTPROD(a,b,c) (((a)<<6) | ((b)<<3) | (c))

int partprod500TX[] = {
	-1,
	PARTPROD(0,0,1), PARTPROD(0,0,2), PARTPROD(0,1,2), PARTPROD(0,0,3),
	PARTPROD(0,1,3), PARTPROD(0,2,3), PARTPROD(1,2,3), PARTPROD(0,0,4),
	PARTPROD(0,1,4), PARTPROD(0,2,4), PARTPROD(1,2,4), PARTPROD(0,3,4),
	PARTPROD(1,3,4), PARTPROD(2,3,4),              -1, PARTPROD(0,0,5), 
	PARTPROD(0,1,5), PARTPROD(0,2,5), PARTPROD(1,2,5), PARTPROD(0,3,5), 
	PARTPROD(1,3,5), PARTPROD(2,3,5),              -1, PARTPROD(0,4,5), 
	PARTPROD(1,4,5), PARTPROD(2,4,5), PARTPROD(3,4,5),              -1,
	             -1,              -1,              -1, PARTPROD(0,0,6), 
	PARTPROD(0,1,6), PARTPROD(0,2,6), PARTPROD(1,2,6), PARTPROD(0,3,6), 
	PARTPROD(1,3,6), PARTPROD(2,3,6),              -1, PARTPROD(0,4,6), 
	PARTPROD(1,4,6), PARTPROD(2,4,6),              -1, PARTPROD(3,4,6),
	             -1,              -1,              -1, PARTPROD(0,5,6), 
	PARTPROD(1,5,6), PARTPROD(2,5,6),              -1, PARTPROD(3,5,6), 
	             -1,              -1,              -1, PARTPROD(4,5,6), 
	             -1,              -1,              -1,              -1,
		     -1,              -1,              -1, PARTPROD(0,0,7), 
	             -1, PARTPROD(0,2,7), PARTPROD(1,2,7), PARTPROD(0,3,7), 
	PARTPROD(1,3,7), PARTPROD(2,3,7),              -1, PARTPROD(0,4,7),
	PARTPROD(1,4,7), PARTPROD(2,4,7),              -1, PARTPROD(3,4,7), 
	             -1,              -1,              -1, PARTPROD(0,5,7),
	PARTPROD(1,5,7), PARTPROD(2,5,7),              -1, PARTPROD(3,5,7), 
	             -1,              -1,              -1, PARTPROD(4,5,7),
	             -1,              -1,              -1,              -1,
		     -1,              -1,              -1, PARTPROD(0,6,7), 
	PARTPROD(1,6,7), PARTPROD(2,6,7),              -1, PARTPROD(3,6,7),
	             -1,              -1,              -1, PARTPROD(4,6,7), 
	             -1,              -1,              -1,              -1,
		     -1,              -1,              -1, PARTPROD(5,6,7), 
	             -1,              -1,              -1,              -1,
		     -1,              -1,              -1,              -1,
		     -1,              -1,              -1,              -1,
		     -1,              -1,              -1, PARTPROD(0,7,7)};
int partprodPermedia[] = {
	-1,
	PARTPROD(0,0,1), PARTPROD(0,1,1), PARTPROD(1,1,1), PARTPROD(1,1,2),
	PARTPROD(1,2,2), PARTPROD(1,2,2), PARTPROD(1,2,3), PARTPROD(2,2,3),
	PARTPROD(1,3,3), PARTPROD(2,3,3),              -1, PARTPROD(3,3,3),
	PARTPROD(1,3,4), PARTPROD(2,3,4),              -1, PARTPROD(3,3,4), 
	PARTPROD(1,4,4), PARTPROD(2,4,4),              -1, PARTPROD(3,4,4), 
	             -1,              -1,              -1, PARTPROD(4,4,4), 
	PARTPROD(1,4,5), PARTPROD(2,4,5), PARTPROD(3,4,5),              -1,
	             -1,              -1,              -1, PARTPROD(4,4,5), 
	PARTPROD(1,5,5), PARTPROD(2,5,5),              -1, PARTPROD(3,5,5), 
	             -1,              -1,              -1, PARTPROD(4,5,5), 
	             -1,              -1,              -1,              -1,
	             -1,              -1,              -1, PARTPROD(5,5,5), 
	PARTPROD(1,5,6), PARTPROD(2,5,6),              -1, PARTPROD(3,5,6),
	             -1,              -1,              -1, PARTPROD(4,5,6),
	             -1,              -1,              -1,              -1,
	             -1,              -1,              -1, PARTPROD(5,5,6),
	             -1,              -1,              -1,              -1,
	             -1,              -1,              -1,              -1,
	             -1,              -1,              -1,              -1,
	             -1,              -1,              -1,              -1,
	             -1,              -1,              -1,              -1,
	             -1,              -1,              -1,              -1,
	             -1,              -1,              -1,              -1,
	             -1,              -1,              -1,              -1,
	             -1,              -1,              -1,              -1,
	             -1,              -1,              -1,              -1,
	             -1,              -1,              -1,              -1,
	             -1,              -1,              -1,              -1,
	             -1,              -1,              -1,              -1,
	             -1,              -1,              -1,              -1,
	             -1,              -1,              -1,              -1,
	             -1,              -1,              -1,              -1};

typedef struct {
   vgaHWRec std;
} SaveVGAState;

pointer        vgaBase  = NULL;
pointer        vgaNewVideoState = NULL;
#define save   ((SaveVGAState *)vgaNewVideoState)
int            vgaIOBase;
int            vgaInterlaceType = VGA_DIVIDE_VERT;
/* Keep vgaHW.c happy */
void (*vgaSaveScreenFunc)() = vgaHWSaveScreen;
extern int ibm_id;

void PermediaSaveVGAInfo()
{
   vgaNewVideoState = vgaHWSave(vgaNewVideoState, sizeof(SaveVGAState));
}

void PermediaRestoreVGAInfo()
{
   /*
    * Restore the generic vga registers
    */
   vgaHWRestore((vgaHWPtr)save);
}

int
Shiftbpp(int value)
{
    /* shift horizontal timings for 64bit VRAM's or 32bit SGRAMs */
    int logbytesperaccess = 0;
	
    if (IS_3DLABS_TX_MX_CLASS(coprotype)) {
	if (ibm_id == 0x121C /* IBM640 */) 
	    logbytesperaccess = 4;
	else
	    logbytesperaccess = 3;
    } else if (IS_3DLABS_PM_FAMILY(coprotype))
	logbytesperaccess = 2;

    switch (glintInfoRec.bitsPerPixel) {
    case 8:
	value >>= logbytesperaccess;
	Bppshift = logbytesperaccess;
	break;
    case 16:
	if (glintDoubleBufferMode) {
	    value >>= (logbytesperaccess-2);
	    Bppshift = logbytesperaccess-2;
	} else {
	    value >>= (logbytesperaccess-1);
	    Bppshift = logbytesperaccess-1;
	}
	break;
    case 24:
	value *= 3;
	value >>= logbytesperaccess;
	Bppshift = logbytesperaccess;
	break;
    case 32:
	value >>= (logbytesperaccess-2);
	Bppshift = logbytesperaccess-2;
	break;
    }
    return (value);
}

void
glintCalcCRTCRegs(glintCRTCRegPtr crtcRegs, DisplayModePtr mode)
{
    int h_front_porch, v_front_porch;
    int h_sync_width, v_sync_width;

    h_front_porch = mode->CrtcHSyncStart - mode->CrtcHDisplay;
    v_front_porch = mode->CrtcVSyncStart - mode->CrtcVDisplay;
    h_sync_width  = mode->CrtcHSyncEnd - mode->CrtcHSyncStart;
    v_sync_width  = mode->CrtcVSyncEnd - mode->CrtcVSyncStart;

    crtcRegs->h_limit		= Shiftbpp(mode->CrtcHTotal);
    crtcRegs->h_sync_end	= Shiftbpp(h_front_porch + h_sync_width);
    crtcRegs->h_sync_start	= Shiftbpp(h_front_porch);
    crtcRegs->h_blank_end	= Shiftbpp(mode->CrtcHTotal-mode->CrtcHDisplay);
    crtcRegs->screenstride	= Shiftbpp(glintInfoRec.displayWidth>>1);

    crtcRegs->v_limit		= mode->CrtcVTotal;
    crtcRegs->v_sync_end	= v_front_porch + v_sync_width + 1;
    crtcRegs->v_sync_start	= v_front_porch + 1;
    crtcRegs->v_blank_end	= mode->CrtcVTotal - mode->CrtcVDisplay;
    VBlank 			= crtcRegs->v_blank_end;

    if (IS_3DLABS_TX_MX_CLASS(coprotype)) {
	crtcRegs->vtgpolarity = 
	    (((mode->Flags & V_PHSYNC) ? 0 : 2) << 2) | 
	    ((mode->Flags & V_PVSYNC) ? 0 : 2) | (0xb0);
    }

    else if (IS_3DLABS_PM_FAMILY(coprotype)) {
        if (OFLG_ISSET(OPTION_SYNC_ON_GREEN, &glintInfoRec.options)) 
             /* default is positive sync polarity, but many SOG monitors 
                want negative sync polarity */
            crtcRegs->vtgpolarity = 
                 (((mode->Flags & V_NHSYNC) ? 0x3 : 0x1) << 3) |  
                 (((mode->Flags & V_NVSYNC) ? 0x3 : 0x1) << 5) |
                 ((mode->Flags & V_DBLSCAN) ? (1 << 2) : 0) | 1;
 	else
 	    crtcRegs->vtgpolarity = 
		(((mode->Flags & V_PHSYNC) ? 0x1 : 0x3) << 3) |  
		(((mode->Flags & V_PVSYNC) ? 0x1 : 0x3) << 5) |
		((mode->Flags & V_DBLSCAN) ? (1 << 2) : 0) | 1;
	if (IS_3DLABS_PM2_CLASS(coprotype)) {
	 if (coprotype == PCI_CHIP_3DLABS_PERMEDIA2V) {
		/* We stick the RAMDAC into 64bit mode */
		/* And reduce the horizontal timings by half */
		crtcRegs->vtgpolarity |= 1<<16;
    		crtcRegs->h_limit >>= 1;
		crtcRegs->h_sync_end >>= 1;
		crtcRegs->h_sync_start >>= 1;
		crtcRegs->h_blank_end >>= 1;
	 } else {
	   if (glintInfoRec.bitsPerPixel > 8) {
		/* When != 8bpp then we stick the RAMDAC into 64bit mode */
		/* And reduce the horizontal timings by half */
		crtcRegs->vtgpolarity |= 1<<16;
    		crtcRegs->h_limit >>= 1;
		crtcRegs->h_sync_end >>= 1;
		crtcRegs->h_sync_start >>= 1;
		crtcRegs->h_blank_end >>= 1;
	   }
	 }
	}
    }

    crtcRegs->clock_sel = glintInfoRec.clock[mode->Clock];

    if (IS_3DLABS_PERMEDIA_CLASS(coprotype)) {
      /* crtcRegs->vclkctl = 0x03 | 
	 (((33*10000*6 + (crtcRegs->clock_sel-1)) / crtcRegs->clock_sel) << 2); */
      /* GL 1000 none Recovery time */
      crtcRegs->vclkctl = 0x03;
    }
    else if (IS_3DLABS_PM2_CLASS(coprotype)) {
        /* crtcRegs->vclkctl = (GLINT_READ_REG(VClkCtl) & 0xFFFFFFFC); */
	unsigned long PixelClock;
	unsigned int highWater, ulValue, newChipConfig;

	PixelClock = crtcRegs->clock_sel * 10;
	crtcRegs->vclkctl = (((66*10000*6 + (PixelClock-1)) / PixelClock) << 2);

#define videoFIFOSize           32
#define videoFIFOLoWater         8
#define videoFIFOLatency        25

        highWater = (((videoFIFOLatency * crtcRegs->clock_sel * 
        glintInfoRec.bitsPerPixel) / (64 *  14318 /*SystemClock*/ )) + 1);
                
        if (highWater > videoFIFOSize)
                highWater = videoFIFOSize;

        highWater = videoFIFOSize - highWater;

        if (highWater <= videoFIFOLoWater)
                highWater = videoFIFOLoWater + 1;
                
        ulValue = (highWater << 8) | videoFIFOLoWater;

        GLINT_WRITE_REG(ulValue, PM2FifoControl);

        /* Teste neues Setup für AGP */
#define SCLK_SEL_PCI      (0x0 << 10)   /* Delta Clk == PCI Clk */
#define SCLK_SEL_PCIHALF  (0x1 << 10)  /* Delta Clk == 1/2 PCI Clk */
#define SCLK_SEL_MCLK     (0x2 << 10)   /* Delta Clk == MClk */
#define SCLK_SEL_MCLKHALF (0x3 << 10)   /* Delta Clk == 1/2 MClk */

        newChipConfig = GLINT_READ_REG(ChipConfig);
#if DEBUG
        ErrorF("orig ChipConfig = 0x%08x\n", newChipConfig);
#endif
        newChipConfig &= ~(0x3 << 10);
        newChipConfig |= SCLK_SEL_MCLKHALF;
        GLINT_WRITE_REG(newChipConfig, ChipConfig);
#if DEBUG
        ErrorF("new ChipConfig = 0x%08x\n", newChipConfig);
#endif
    }
    else {
	/*
	 * tell DAC to use the ICD chip clock 0 as ref clock 
	 * and set up some more video timining generator registers
	 */
	crtcRegs->vclkctl = 0x00;
	crtcRegs->vtgserialclk = 0x05;
	/*
	 * Different settings for Fire GL 3000 and Elsa Gloria cards
	 */
	if (OFLG_ISSET(OPTION_FIREGL3000, &glintInfoRec.options)) {
	    crtcRegs->fbmodesel = 0x907; /* ?way interleave */
	    crtcRegs->vtgmodectl = 0x00;
	} else {
	    crtcRegs->fbmodesel = 0xa07; /* 4way interleave */
	    if (ibm_id == 0x121C /* IBM640 */)
		crtcRegs->vtgmodectl = 0x04;
	    else
	        crtcRegs->vtgmodectl = 0x44;
	}
    }
}

void
glintSetCRTCRegs(glintCRTCRegPtr crtcRegs)
{
 	int i;
    GLINT_SLOW_WRITE_REG(0, ScissorMode);
    GLINT_SLOW_WRITE_REG(1, FBWriteMode);
    GLINT_SLOW_WRITE_REG(0, dXDom);
    GLINT_SLOW_WRITE_REG(0, dXSub);

    if (IS_3DLABS_TX_MX_CLASS(coprotype)) {

	/* Initialize the Accelerator Engine to defaults */

	GLINT_SLOW_WRITE_REG(pprod,		LBReadMode);
	GLINT_SLOW_WRITE_REG(0x01,		LBWriteMode);
#if 1	/* For the GLINT MX - Added by Dirk */
	GLINT_SLOW_WRITE_REG(0x0,		0x6008);
	GLINT_SLOW_WRITE_REG(0x0,		0x6000);
#endif
	GLINT_SLOW_WRITE_REG(UNIT_DISABLE,	DitherMode);
	GLINT_SLOW_WRITE_REG(UNIT_DISABLE,	AlphaBlendMode);
	GLINT_SLOW_WRITE_REG(UNIT_DISABLE,	ColorDDAMode);
	GLINT_SLOW_WRITE_REG(UNIT_DISABLE,	TextureColorMode);
	GLINT_SLOW_WRITE_REG(UNIT_DISABLE,	TextureAddressMode);
	GLINT_SLOW_WRITE_REG(UNIT_DISABLE,   TextureReadMode);
	GLINT_SLOW_WRITE_REG(UNIT_DISABLE,   GLINTWindow);
	GLINT_SLOW_WRITE_REG(UNIT_DISABLE,   AlphaBlendMode);
	GLINT_SLOW_WRITE_REG(UNIT_DISABLE,   LogicalOpMode);
	GLINT_SLOW_WRITE_REG(UNIT_DISABLE,   DepthMode);
	GLINT_SLOW_WRITE_REG(UNIT_DISABLE,   RouterMode);
	GLINT_SLOW_WRITE_REG(UNIT_DISABLE,	FogMode);
	GLINT_SLOW_WRITE_REG(UNIT_DISABLE,	AntialiasMode);
	GLINT_SLOW_WRITE_REG(UNIT_DISABLE,	AlphaTestMode);
	GLINT_SLOW_WRITE_REG(UNIT_DISABLE,	StencilMode);
	GLINT_SLOW_WRITE_REG(UNIT_DISABLE,	AreaStippleMode);
	GLINT_SLOW_WRITE_REG(UNIT_DISABLE,	LineStippleMode);
	GLINT_SLOW_WRITE_REG(0,		UpdateLineStippleCounters);
	GLINT_SLOW_WRITE_REG(UNIT_DISABLE,	LogicalOpMode);
	GLINT_SLOW_WRITE_REG(UNIT_DISABLE,	DepthMode);
	GLINT_SLOW_WRITE_REG(UNIT_DISABLE,	StatisticMode);
	GLINT_SLOW_WRITE_REG(0xc00,		FilterMode);
	GLINT_SLOW_WRITE_REG(0xffffffff,	FBHardwareWriteMask);
	GLINT_SLOW_WRITE_REG(0xffffffff,	FBSoftwareWriteMask);
	GLINT_SLOW_WRITE_REG(UNIT_DISABLE,	RasterizerMode);
	GLINT_SLOW_WRITE_REG(UNIT_DISABLE,	GLINTDepth);
	GLINT_SLOW_WRITE_REG(UNIT_DISABLE,	FBSourceOffset);
	GLINT_SLOW_WRITE_REG(UNIT_DISABLE,	FBPixelOffset);
	GLINT_SLOW_WRITE_REG(UNIT_DISABLE,	LBSourceOffset);
	GLINT_SLOW_WRITE_REG(UNIT_DISABLE,	WindowOrigin);
	GLINT_SLOW_WRITE_REG(UNIT_DISABLE,	FBWindowBase);
	GLINT_SLOW_WRITE_REG(UNIT_DISABLE,	LBWindowBase);
	GLINT_SLOW_WRITE_REG(UNIT_DISABLE,	TextureAddressMode);
	GLINT_SLOW_WRITE_REG(UNIT_DISABLE,	RouterMode);
	GLINT_SLOW_WRITE_REG(UNIT_DISABLE,	PatternRamMode);

	switch (glintInfoRec.bitsPerPixel) {
	case 8:
	    GLINT_SLOW_WRITE_REG(0x2,	PixelSize);
	    break;
	case 16:
	    GLINT_SLOW_WRITE_REG(0x1,	PixelSize);
	    break;
	case 32:
	    GLINT_SLOW_WRITE_REG(0x0,	PixelSize);
	    break;
	}

	/* Now write the CRTC registers */

	GLINT_SLOW_WRITE_REG(crtcRegs->vtgpolarity,	VTGPolarity);
	GLINT_SLOW_WRITE_REG(crtcRegs->vclkctl,	VClkCtl);
	GLINT_SLOW_WRITE_REG(0x01,			VTGSerialClk);
	GLINT_SLOW_WRITE_REG(0x00,			VTGModeCtl);
	GLINT_SLOW_WRITE_REG(crtcRegs->vtgserialclk,	VTGSerialClk);
	GLINT_SLOW_WRITE_REG(crtcRegs->vtgmodectl,	VTGModeCtl);
	GLINT_SLOW_WRITE_REG(crtcRegs->h_limit,      VTGHLimit);
	GLINT_SLOW_WRITE_REG(crtcRegs->h_sync_start, VTGHSyncStart);
	GLINT_SLOW_WRITE_REG(crtcRegs->h_sync_end,	VTGHSyncEnd);
	GLINT_SLOW_WRITE_REG(crtcRegs->h_blank_end,	VTGHBlankEnd);
	GLINT_SLOW_WRITE_REG(crtcRegs->v_limit,	VTGVLimit);
	GLINT_SLOW_WRITE_REG(crtcRegs->v_sync_start, VTGVSyncStart);
	GLINT_SLOW_WRITE_REG(crtcRegs->v_sync_end,	VTGVSyncEnd);
	GLINT_SLOW_WRITE_REG(crtcRegs->v_blank_end,	VTGVBlankEnd);
	GLINT_SLOW_WRITE_REG(crtcRegs->v_blank_end-1,VTGVGateStart);
	GLINT_SLOW_WRITE_REG(crtcRegs->v_blank_end,	VTGVGateEnd);
	GLINT_SLOW_WRITE_REG(crtcRegs->fbmodesel, FBModeSel);

	if (OFLG_ISSET(OPTION_FIREGL3000, &glintInfoRec.options)) {
    		GLINT_SLOW_WRITE_REG(crtcRegs->h_blank_end-1,VTGHGateStart);
    		GLINT_SLOW_WRITE_REG(crtcRegs->h_limit-1,	VTGHGateEnd);
  	} else {
		GLINT_SLOW_WRITE_REG(crtcRegs->h_blank_end-2,VTGHGateStart);
		GLINT_SLOW_WRITE_REG(crtcRegs->h_limit-2,	VTGHGateEnd);
  	}
    }
    else if (IS_3DLABS_PM_FAMILY(coprotype)) {

	/* Initialize the Accelerator Engine to defaults */

	GLINT_SLOW_WRITE_REG(GWIN_DisableLBUpdate,   GLINTWindow);
	GLINT_SLOW_WRITE_REG(UNIT_DISABLE,	DitherMode);
	GLINT_SLOW_WRITE_REG(UNIT_DISABLE,	AlphaBlendMode);
	GLINT_SLOW_WRITE_REG(UNIT_DISABLE,	ColorDDAMode);
	GLINT_SLOW_WRITE_REG(UNIT_DISABLE,	TextureColorMode);
	GLINT_SLOW_WRITE_REG(UNIT_DISABLE,	TextureAddressMode);
	GLINT_SLOW_WRITE_REG(UNIT_DISABLE,   TextureReadMode);
	GLINT_SLOW_WRITE_REG(UNIT_DISABLE,	PMTextureReadMode);
	GLINT_SLOW_WRITE_REG(pprod,		LBReadMode);
	GLINT_SLOW_WRITE_REG(UNIT_DISABLE,   AlphaBlendMode);
	GLINT_SLOW_WRITE_REG(UNIT_DISABLE,	TexelLUTMode);
	GLINT_SLOW_WRITE_REG(UNIT_DISABLE,	YUVMode);
	GLINT_SLOW_WRITE_REG(UNIT_DISABLE,   DepthMode);
	GLINT_SLOW_WRITE_REG(UNIT_DISABLE,   RouterMode);
	GLINT_SLOW_WRITE_REG(UNIT_DISABLE,	FogMode);
	GLINT_SLOW_WRITE_REG(UNIT_DISABLE,	AntialiasMode);
	GLINT_SLOW_WRITE_REG(UNIT_DISABLE,	AlphaTestMode);
	GLINT_SLOW_WRITE_REG(UNIT_DISABLE,	StencilMode);
	GLINT_SLOW_WRITE_REG(UNIT_DISABLE,	AreaStippleMode);
	GLINT_SLOW_WRITE_REG(UNIT_DISABLE,	LogicalOpMode);
	GLINT_SLOW_WRITE_REG(UNIT_DISABLE,	DepthMode);
	GLINT_SLOW_WRITE_REG(UNIT_DISABLE,	StatisticMode);
	GLINT_SLOW_WRITE_REG(0xc00,		FilterMode);
	GLINT_SLOW_WRITE_REG(0xffffffff,	FBHardwareWriteMask);
	GLINT_SLOW_WRITE_REG(0xffffffff,	FBSoftwareWriteMask);
	GLINT_SLOW_WRITE_REG(UNIT_DISABLE,	RasterizerMode);
	GLINT_SLOW_WRITE_REG(UNIT_DISABLE,	GLINTDepth);
	GLINT_SLOW_WRITE_REG(UNIT_DISABLE,	FBSourceOffset);
	GLINT_SLOW_WRITE_REG(UNIT_DISABLE,	FBPixelOffset);
	GLINT_SLOW_WRITE_REG(UNIT_DISABLE,	LBSourceOffset);
	GLINT_SLOW_WRITE_REG(UNIT_DISABLE,	WindowOrigin);
	GLINT_SLOW_WRITE_REG(UNIT_DISABLE,	FBWindowBase);
	GLINT_SLOW_WRITE_REG(UNIT_DISABLE,	LBWindowBase);

	switch (glintInfoRec.bitsPerPixel) {
	case 8:
	  GLINT_SLOW_WRITE_REG(0x0, FBReadPixel); /* 8 Bits */
	  GLINT_SLOW_WRITE_REG(pprod,		PMTextureMapFormat);
	  break;
	case 16:
	  GLINT_SLOW_WRITE_REG(0x1, FBReadPixel); /* 16 Bits */
	  GLINT_SLOW_WRITE_REG(pprod | 1<<19,	PMTextureMapFormat);
	  break;
	case 24: /* For PM2 */
	  GLINT_SLOW_WRITE_REG(0x4, FBReadPixel); /* 24 Bits */
	  GLINT_SLOW_WRITE_REG(pprod | 2<<19,	PMTextureMapFormat);
	  break;
	case 32:
	  GLINT_SLOW_WRITE_REG(0x2, FBReadPixel); /* 32 Bits */
	  GLINT_SLOW_WRITE_REG(pprod | 2<<19,	PMTextureMapFormat);
	  break;
	}

	/* Now write the CRTC registers */

	GLINT_SLOW_WRITE_REG(crtcRegs->vtgpolarity,	PMVideoControl);
	GLINT_SLOW_WRITE_REG(crtcRegs->h_blank_end,	PMHgEnd);
	GLINT_SLOW_WRITE_REG(0,			PMScreenBase);
	GLINT_SLOW_WRITE_REG(crtcRegs->vclkctl,	VClkCtl);
	GLINT_SLOW_WRITE_REG(crtcRegs->screenstride, PMScreenStride);
	GLINT_SLOW_WRITE_REG(crtcRegs->h_limit-1,    PMHTotal);
	GLINT_SLOW_WRITE_REG(crtcRegs->h_blank_end,	PMHbEnd);
	GLINT_SLOW_WRITE_REG(crtcRegs->h_sync_start-1,   PMHsStart);
	GLINT_SLOW_WRITE_REG(crtcRegs->h_sync_end-1,	PMHsEnd);
	GLINT_SLOW_WRITE_REG(crtcRegs->v_limit-1,	PMVTotal);
	GLINT_SLOW_WRITE_REG(crtcRegs->v_blank_end,	PMVbEnd);
	GLINT_SLOW_WRITE_REG(crtcRegs->v_sync_start-1,	PMVsStart);
	GLINT_SLOW_WRITE_REG(crtcRegs->v_sync_end-1,	PMVsEnd);
    }

    /* Now all the CRTC & Accelerator engines are set, program the clock */

    if (IS_3DLABS_TX_MX_CLASS(coprotype)|| 
        IS_3DLABS_PERMEDIA_CLASS(coprotype)) {
	IBMRGB52x_Init_Stdmode(crtcRegs->clock_sel);
    }
    else if (IS_3DLABS_PM2_CLASS(coprotype) ) {
	if (coprotype == PCI_CHIP_3DLABS_PERMEDIA2V)
		PM2VDACInit(crtcRegs->clock_sel);
	else
    		PM2DACInit(crtcRegs->clock_sel);
    }
}

void
saveGLINTstate()
{
    int i;

    if (IS_3DLABS_TX_MX_CLASS(coprotype)) {
	SR.glintRegs[0] = GLINT_SLOW_READ_REG(VTGHLimit);
	SR.glintRegs[1] = GLINT_SLOW_READ_REG(VTGHSyncStart);
	SR.glintRegs[2] = GLINT_SLOW_READ_REG(VTGHSyncEnd);
	SR.glintRegs[3] = GLINT_SLOW_READ_REG(VTGHBlankEnd);
	SR.glintRegs[4] = GLINT_SLOW_READ_REG(VTGVLimit);
	SR.glintRegs[5] = GLINT_SLOW_READ_REG(VTGVSyncStart);
	SR.glintRegs[6] = GLINT_SLOW_READ_REG(VTGVSyncEnd);
	SR.glintRegs[7] = GLINT_SLOW_READ_REG(VTGVBlankEnd);
	SR.glintRegs[8] = GLINT_SLOW_READ_REG(VTGPolarity);

        for (i=0; i<0x100; i++)
		SR.DacRegs[i] = glintInIBMRGBIndReg(i);

    } else if (IS_3DLABS_PM_FAMILY(coprotype)) {
	SR.glintRegs[0]  = GLINT_SLOW_READ_REG(Aperture0);
	SR.glintRegs[1]  = GLINT_SLOW_READ_REG(Aperture1);
	SR.glintRegs[2]  = GLINT_SLOW_READ_REG(PMFramebufferWriteMask);
	SR.glintRegs[3]  = GLINT_SLOW_READ_REG(PMBypassWriteMask);
	SR.glintRegs[4]  = GLINT_SLOW_READ_REG(FIFODis);
	/* We only muck about with PMMemConfig, if user wants to */
  	if (OFLG_ISSET(OPTION_BLOCK_WRITE, &glintInfoRec.options))
		SR.glintRegs[5]= GLINT_SLOW_READ_REG(PMMemConfig);
	SR.glintRegs[6] = GLINT_SLOW_READ_REG(PMHTotal);
	SR.glintRegs[7] = GLINT_SLOW_READ_REG(PMHbEnd);
	SR.glintRegs[8] = GLINT_SLOW_READ_REG(PMHgEnd);
	SR.glintRegs[9] = GLINT_SLOW_READ_REG(PMScreenStride);
	SR.glintRegs[10] = GLINT_SLOW_READ_REG(PMHsStart);
	SR.glintRegs[11] = GLINT_SLOW_READ_REG(PMHsEnd);
	SR.glintRegs[12] = GLINT_SLOW_READ_REG(PMVTotal);
	SR.glintRegs[13] = GLINT_SLOW_READ_REG(PMVbEnd);
	SR.glintRegs[14] = GLINT_SLOW_READ_REG(PMVsStart);
	SR.glintRegs[15]= GLINT_SLOW_READ_REG(PMVsEnd);
	SR.glintRegs[16]= GLINT_SLOW_READ_REG(PMScreenBase);
	SR.glintRegs[17]= GLINT_SLOW_READ_REG(PMVideoControl);
	SR.glintRegs[18]= GLINT_SLOW_READ_REG(VClkCtl);
	SR.glintRegs[19]= GLINT_SLOW_READ_REG(ChipConfig);

	if (IS_3DLABS_PERMEDIA_CLASS(coprotype)) {
 	       for (i=0; i<0x100; i++)
			SR.DacRegs[i] = glintInIBMRGBIndReg(i);
	}

        if (IS_3DLABS_PM2_CLASS(coprotype)) {
	   if (coprotype == PCI_CHIP_3DLABS_PERMEDIA2V) {
		GLINT_SLOW_WRITE_REG(0, PM2VDACIndexRegHigh);
		GLINT_SLOW_WRITE_REG(PM2VDACRDPixelSize, PM2VDACIndexRegLow);
		SR.glintRegs[101] = GLINT_SLOW_READ_REG(PM2VDACIndexData);
		GLINT_SLOW_WRITE_REG(PM2VDACRDColorFormat, PM2VDACIndexRegLow);
		SR.glintRegs[102] = GLINT_SLOW_READ_REG(PM2VDACIndexData);
		GLINT_SLOW_WRITE_REG(PM2VDACRDMiscControl, PM2VDACIndexRegLow);
		SR.glintRegs[103] = GLINT_SLOW_READ_REG(PM2VDACIndexData);

    		GLINT_SLOW_WRITE_REG(PM2VDACRDDClk0PreScale >> 8, PM2VDACIndexRegHigh);
		GLINT_SLOW_WRITE_REG(PM2VDACRDDClk0PreScale, PM2VDACIndexRegLow);
		SR.glintRegs[104] = GLINT_SLOW_READ_REG(PM2VDACIndexData);
		GLINT_SLOW_WRITE_REG(PM2VDACRDDClk0FeedbackScale, PM2VDACIndexRegLow);
		SR.glintRegs[105] = GLINT_SLOW_READ_REG(PM2VDACIndexData);
		GLINT_SLOW_WRITE_REG(PM2VDACRDDClk0PostScale, PM2VDACIndexRegLow);
		SR.glintRegs[106] = GLINT_SLOW_READ_REG(PM2VDACIndexData);
    		GLINT_SLOW_WRITE_REG(0, PM2VDACIndexRegHigh);
	   } else {
		GLINT_SLOW_WRITE_REG(PM2DACIndexMCR, PM2DACIndexReg);
		SR.glintRegs[101] = GLINT_SLOW_READ_REG(PM2DACIndexData);
		GLINT_SLOW_WRITE_REG(PM2DACIndexCMR, PM2DACIndexReg);
		SR.glintRegs[100] = GLINT_SLOW_READ_REG(PM2DACIndexData);
		GLINT_SLOW_WRITE_REG(PM2DACIndexClockAM, PM2DACIndexReg);
		SR.glintRegs[104] = GLINT_SLOW_READ_REG(PM2DACIndexData);
		GLINT_SLOW_WRITE_REG(PM2DACIndexClockAN, PM2DACIndexReg);
		SR.glintRegs[103] = GLINT_SLOW_READ_REG(PM2DACIndexData);
		GLINT_SLOW_WRITE_REG(PM2DACIndexClockAP, PM2DACIndexReg);
		SR.glintRegs[102] = GLINT_SLOW_READ_REG(PM2DACIndexData);
	   }
	}
    }
}

void
restoreGLINTstate(void)
{
    int i;
    unsigned short usData;

#if DEBUG
    if( xf86Verbose > 3)
      glintDumpRegs();
#endif

    if (IS_3DLABS_TX_MX_CLASS(coprotype)) {
	GLINT_SLOW_WRITE_REG(SR.glintRegs[0], VTGHLimit);
	GLINT_SLOW_WRITE_REG(SR.glintRegs[1], VTGHSyncStart);
	GLINT_SLOW_WRITE_REG(SR.glintRegs[2], VTGHSyncEnd);
	GLINT_SLOW_WRITE_REG(SR.glintRegs[3], VTGHBlankEnd);
	GLINT_SLOW_WRITE_REG(SR.glintRegs[4], VTGVLimit);
	GLINT_SLOW_WRITE_REG(SR.glintRegs[5], VTGVSyncStart);
	GLINT_SLOW_WRITE_REG(SR.glintRegs[6], VTGVSyncEnd);
	GLINT_SLOW_WRITE_REG(SR.glintRegs[7], VTGVBlankEnd);
	GLINT_SLOW_WRITE_REG(SR.glintRegs[8], VTGPolarity);
    
	for (i=0; i<0x100; i++) 
	    glintOutIBMRGBIndReg(i, 0, SR.DacRegs[i]);

    } else if (IS_3DLABS_PM_FAMILY(coprotype)) {

	    GLINT_SLOW_WRITE_REG(SR.glintRegs[0], Aperture0);
	    GLINT_SLOW_WRITE_REG(SR.glintRegs[1], Aperture1);
	    GLINT_SLOW_WRITE_REG(SR.glintRegs[2], PMFramebufferWriteMask);
	    GLINT_SLOW_WRITE_REG(SR.glintRegs[3], PMBypassWriteMask);
	    GLINT_SLOW_WRITE_REG(SR.glintRegs[4], FIFODis);
	    /* We only muck about with PMMemConfig, if user wants to */
  	    if (OFLG_ISSET(OPTION_BLOCK_WRITE, &glintInfoRec.options))
	    	GLINT_SLOW_WRITE_REG(SR.glintRegs[5], PMMemConfig);
	    GLINT_SLOW_WRITE_REG(SR.glintRegs[6], PMHTotal);
	    GLINT_SLOW_WRITE_REG(SR.glintRegs[7], PMHbEnd);
	    GLINT_SLOW_WRITE_REG(SR.glintRegs[8], PMHgEnd);
	    GLINT_SLOW_WRITE_REG(SR.glintRegs[9], PMScreenStride);
	    GLINT_SLOW_WRITE_REG(SR.glintRegs[10], PMHsStart);
	    GLINT_SLOW_WRITE_REG(SR.glintRegs[11], PMHsEnd);
	    GLINT_SLOW_WRITE_REG(SR.glintRegs[12], PMVTotal);
	    GLINT_SLOW_WRITE_REG(SR.glintRegs[13], PMVbEnd);
	    GLINT_SLOW_WRITE_REG(SR.glintRegs[14], PMVsStart);
	    GLINT_SLOW_WRITE_REG(SR.glintRegs[15], PMVsEnd);
	    GLINT_SLOW_WRITE_REG(SR.glintRegs[16], PMScreenBase);
	    GLINT_SLOW_WRITE_REG(SR.glintRegs[17], PMVideoControl);
	    GLINT_SLOW_WRITE_REG(SR.glintRegs[18], VClkCtl);
	    GLINT_SLOW_WRITE_REG(SR.glintRegs[19], ChipConfig);

            if (IS_3DLABS_PM2_CLASS(coprotype)) {
	     if (coprotype == PCI_CHIP_3DLABS_PERMEDIA2V) {
		GLINT_SLOW_WRITE_REG(0, PM2VDACIndexRegHigh);
		GLINT_SLOW_WRITE_REG(PM2VDACRDPixelSize, PM2VDACIndexRegLow);
		GLINT_SLOW_WRITE_REG(SR.glintRegs[101], PM2VDACIndexData);
		GLINT_SLOW_WRITE_REG(PM2VDACRDColorFormat, PM2VDACIndexRegLow);
		GLINT_SLOW_WRITE_REG(SR.glintRegs[102], PM2VDACIndexData);
		GLINT_SLOW_WRITE_REG(PM2VDACRDMiscControl, PM2VDACIndexRegLow);
		GLINT_SLOW_WRITE_REG(SR.glintRegs[103], PM2VDACIndexData);
		/* restore colors */
		GLINT_SLOW_WRITE_REG(0x00, PM2DACWriteAddress);
		for (i=0; i<256; i++) {
		    GLINT_SLOW_WRITE_REG(oldlut[i].r,PM2DACData);
		    GLINT_SLOW_WRITE_REG(oldlut[i].g,PM2DACData);
		    GLINT_SLOW_WRITE_REG(oldlut[i].b,PM2DACData);
		}
		GLINT_SLOW_WRITE_REG(PM2VDACRDDClk0PreScale>>8, PM2VDACIndexRegHigh);
		GLINT_SLOW_WRITE_REG(PM2VDACRDDClk0PreScale, PM2VDACIndexRegLow);
		GLINT_SLOW_WRITE_REG(SR.glintRegs[104],PM2VDACIndexData);
		GLINT_SLOW_WRITE_REG(PM2VDACRDDClk0FeedbackScale, PM2VDACIndexRegLow);
		GLINT_SLOW_WRITE_REG(SR.glintRegs[105],PM2VDACIndexData);
		GLINT_SLOW_WRITE_REG(PM2VDACRDDClk0PostScale, PM2VDACIndexRegLow);
		GLINT_SLOW_WRITE_REG(SR.glintRegs[106],PM2VDACIndexData);
		GLINT_SLOW_WRITE_REG(0, PM2VDACIndexRegHigh);
	     } else {
		GLINT_SLOW_WRITE_REG(PM2DACIndexMCR, PM2DACIndexReg);
		GLINT_SLOW_WRITE_REG(SR.glintRegs[101],PM2DACIndexData);
		GLINT_SLOW_WRITE_REG(PM2DACIndexCMR, PM2DACIndexReg);
		GLINT_SLOW_WRITE_REG(SR.glintRegs[100],PM2DACIndexData);
		/* restore colors */
		GLINT_SLOW_WRITE_REG(0x00, PM2DACWriteAddress);
		for (i=0; i<256; i++) {
		    GLINT_SLOW_WRITE_REG(oldlut[i].r,PM2DACData);
		    GLINT_SLOW_WRITE_REG(oldlut[i].g,PM2DACData);
		    GLINT_SLOW_WRITE_REG(oldlut[i].b,PM2DACData);
		}
		GLINT_SLOW_WRITE_REG(PM2DACIndexClockAM, PM2DACIndexReg);
		GLINT_SLOW_WRITE_REG(SR.glintRegs[104],PM2DACIndexData);
		GLINT_SLOW_WRITE_REG(PM2DACIndexClockAN, PM2DACIndexReg);
		GLINT_SLOW_WRITE_REG(SR.glintRegs[103],PM2DACIndexData);
		GLINT_SLOW_WRITE_REG(PM2DACIndexClockAP, PM2DACIndexReg);
		GLINT_SLOW_WRITE_REG(SR.glintRegs[102],PM2DACIndexData);
 	      }
	    }

	    if (IS_3DLABS_PERMEDIA_CLASS(coprotype)) {

		/* restore colors */
		GLINT_SLOW_WRITE_REG(0x00, IBMRGB_WRITE_ADDR);
		for (i=0; i<256; i++) {
		    GLINT_SLOW_WRITE_REG(oldlut[i].r,IBMRGB_RAMDAC_DATA);
		    GLINT_SLOW_WRITE_REG(oldlut[i].g,IBMRGB_RAMDAC_DATA);
		    GLINT_SLOW_WRITE_REG(oldlut[i].b,IBMRGB_RAMDAC_DATA);
		}

		/* switch to VGA */
		GLINT_SLOW_WRITE_REG((unsigned char)PERMEDIA_VGA_CTRL_INDEX, 
		                PERMEDIA_MMVGA_INDEX_REG);

		usData = GLINT_READ_REG(PERMEDIA_MMVGA_DATA_REG);
		usData |= 
		    ((PERMEDIA_VGA_ENABLE | PERMEDIA_VGA_MEMORYACCESS) << 8) | 
		    PERMEDIA_VGA_CTRL_INDEX;
		GLINT_SLOW_WRITE_REG(usData, PERMEDIA_MMVGA_INDEX_REG);

		/* restore ramdac */
		for (i=0; i<0x100; i++) 
		    glintOutIBMRGBIndReg(i, 0, SR.DacRegs[i]);
	    }

	}
}

#define PM2_NORMAL_MCLK		 82000	/* just guessing here */
#define PM2_OVERCLOCK_MCLK	100000	/* should get HOT!!! */

void
glintCleanUp(void)
{
    restoreGLINTstate();

    if (IS_3DLABS_PM_FAMILY(coprotype)) {
	if (VGAcore)
		PermediaRestoreVGAInfo();
    }
    if (IS_3DLABS_PM2_CLASS(coprotype)) {
      if (OFLG_ISSET(OPTION_OVERCLOCK_MEM, &glintInfoRec.options)) {
	PM2DACGlintSetMClock(PM2_NORMAL_MCLK);
      }
    }

}

void permediapreinit(void)
{
  int temp;

  if (OFLG_ISSET(OPTION_BLOCK_WRITE, &glintInfoRec.options)) {
 	/* Enable single cycle block writes */
	temp = GLINT_READ_REG(PMMemConfig);
	GLINT_SLOW_WRITE_REG(temp | 1<<21, PMMemConfig);
  }
  GLINT_SLOW_WRITE_REG(0x00,       Aperture0);
  GLINT_SLOW_WRITE_REG(0x00,       Aperture1);
  GLINT_SLOW_WRITE_REG(0xffffffff, PMFramebufferWriteMask);
  GLINT_SLOW_WRITE_REG(0xffffffff, PMBypassWriteMask);
  if (IS_3DLABS_PM2_CLASS(coprotype)) {
    if (OFLG_ISSET(OPTION_OVERCLOCK_MEM, &glintInfoRec.options)) {
      PM2DACGlintSetMClock(PM2_OVERCLOCK_MCLK);
    }
  }
}

Bool
glintInit(DisplayModePtr mode)
{
    unsigned short usData;

    if (UsePCIRetry) {
	GLINT_SLOW_WRITE_REG(1,			DFIFODis);
	GLINT_SLOW_WRITE_REG(3,			FIFODis);
    } else {
	GLINT_SLOW_WRITE_REG(0,			DFIFODis);
	GLINT_SLOW_WRITE_REG(1,			FIFODis);
    }

    if (IS_3DLABS_PM_FAMILY(coprotype)) {
	  permediapreinit();
    }

    /* this sets LB bank size and page detectors required for 3D operations */
    if (IS_3DLABS_TX_MX_CLASS(coprotype)) {
	int lb_mb = glintLBvideoRam / 1024;

	/* 0x11 = enables EDO functionality and two page detectors */

	if(lb_mb == 8)
	    GLINT_SLOW_WRITE_REG((2<<1) | 0x11,	LBMemoryEDO)	/* 512K pix */
	else if(lb_mb == 16)
	    GLINT_SLOW_WRITE_REG((3<<1) | 0x11,	LBMemoryEDO)	/* 1M pix */
	else if(lb_mb == 32)
	    GLINT_SLOW_WRITE_REG((4<<1) | 0x11,	LBMemoryEDO)	/* 2M pix */
	else if(lb_mb == 64)
	    GLINT_SLOW_WRITE_REG((5<<1) | 0x11,	LBMemoryEDO)	/* 4M pix */
    }

    if (IS_3DLABS_PERMEDIA_CLASS(coprotype)) {
	  /* switch to graphics Mode */
	  GLINT_SLOW_WRITE_REG((unsigned char)PERMEDIA_VGA_CTRL_INDEX, PERMEDIA_MMVGA_INDEX_REG);
	  usData = (unsigned short)GLINT_READ_REG(PERMEDIA_MMVGA_DATA_REG);
	  usData &= ~PERMEDIA_VGA_ENABLE;
	  usData = (usData << 8) | PERMEDIA_VGA_CTRL_INDEX;
	  GLINT_SLOW_WRITE_REG((unsigned short)usData, PERMEDIA_MMVGA_INDEX_REG);
    }

    memset(glintVideoMem, 0x00, glintInfoRec.videoRam * 1024);

    return(TRUE);
}

static void
InitLUT(void)
{
    int i;

    if (IS_3DLABS_PM2_CLASS(coprotype)) {
	GLINT_SLOW_WRITE_REG(0xFF, PM2DACReadMask);
	GLINT_SLOW_WRITE_REG(0x00, PM2DACReadAddress);
    } else {
     	GLINT_SLOW_WRITE_REG(0xFF, IBMRGB_PIXEL_MASK);
    	GLINT_SLOW_WRITE_REG(0x00, IBMRGB_READ_ADDR);
    }

    /*
     * we should make sure that we don't overrun the RAMDAC, so let's
     * pause for a moment every time after we've written to it
     */
    for (i=0; i<256; i++) {
	if (IS_3DLABS_PM2_CLASS(coprotype)) {
	oldlut[i].r = GLINT_SLOW_READ_REG(PM2DACData);
	oldlut[i].g = GLINT_SLOW_READ_REG(PM2DACData);
	oldlut[i].b = GLINT_SLOW_READ_REG(PM2DACData);
	} else {
	oldlut[i].r = GLINT_SLOW_READ_REG(IBMRGB_RAMDAC_DATA);
	oldlut[i].g = GLINT_SLOW_READ_REG(IBMRGB_RAMDAC_DATA);
	oldlut[i].b = GLINT_SLOW_READ_REG(IBMRGB_RAMDAC_DATA);
	}
    }

    if (IS_3DLABS_PM2_CLASS(coprotype)) {
	GLINT_SLOW_WRITE_REG(0x00, PM2DACWriteAddress);
    } else {
    	GLINT_SLOW_WRITE_REG(0x00, IBMRGB_WRITE_ADDR);
    }

    for (i=0; i<256; i++) {
	if (IS_3DLABS_PM2_CLASS(coprotype)) {
	GLINT_SLOW_WRITE_REG(0x00,PM2DACData);
	GLINT_SLOW_WRITE_REG(0x00,PM2DACData);
	GLINT_SLOW_WRITE_REG(0x00,PM2DACData);
	} else {
	GLINT_SLOW_WRITE_REG(0x00,IBMRGB_RAMDAC_DATA);
	GLINT_SLOW_WRITE_REG(0x00,IBMRGB_RAMDAC_DATA);
	GLINT_SLOW_WRITE_REG(0x00,IBMRGB_RAMDAC_DATA);
	}
    }

    if (glintInfoRec.bitsPerPixel > 8) {
	int r,g,b;
	int mr,mg,mb;
	int nr = xf86weight.red;
	int ng = xf86weight.green;
	int nb = xf86weight.blue;
	extern unsigned char xf86rGammaMap[], xf86gGammaMap[], xf86bGammaMap[];
	extern LUTENTRY currentglintdac[];
	
	if (!LUTInited) {
	    if (glintInfoRec.bitsPerPixel == 32 || (ibm_id == 0x121C)) {
		for (i=0;i<256;i++) {
		    currentglintdac[i].r = xf86rGammaMap[i];
		    currentglintdac[i].g = xf86gGammaMap[i];
		    currentglintdac[i].b = xf86bGammaMap[i];
		}
	    } else {
		if (IS_3DLABS_PM2_CLASS(coprotype)) {
		    for (i=0;i<256;i++) {
		    	currentglintdac[i].r = xf86rGammaMap[i];
		    	currentglintdac[i].g = xf86gGammaMap[i];
		    	currentglintdac[i].b = xf86bGammaMap[i];
		    }
		} else {
		    mr = (1<<nr)-1;
		    mg = (1<<ng)-1;
		    mb = (1<<nb)-1;
		
		    for (i=0;i<256;i++) {
		    	r = (i >> (6-nr)) & mr;
		    	g = (i >> (6-ng)) & mg;
		    	b = (i >> (6-nb)) & mb; 
		    	currentglintdac[i].r = xf86rGammaMap[(r*255+mr/2)/mr];
		    	currentglintdac[i].g = xf86gGammaMap[(g*255+mg/2)/mg];
		    	currentglintdac[i].b = xf86bGammaMap[(b*255+mb/2)/mb];
		    }
	        }
	    }
	}

    	if (IS_3DLABS_PM2_CLASS(coprotype)) {
		GLINT_SLOW_WRITE_REG(0x00, PM2DACWriteAddress);
    	} else {
    		GLINT_SLOW_WRITE_REG(0x00, IBMRGB_WRITE_ADDR);
    	}
    	for (i=0; i<256; i++) {
	    if (IS_3DLABS_PM2_CLASS(coprotype)) {
		GLINT_SLOW_WRITE_REG(currentglintdac[i].r,PM2DACData);
		GLINT_SLOW_WRITE_REG(currentglintdac[i].g,PM2DACData);
		GLINT_SLOW_WRITE_REG(currentglintdac[i].b,PM2DACData);
	    } else {
	    	GLINT_SLOW_WRITE_REG(currentglintdac[i].r,IBMRGB_RAMDAC_DATA);
	    	GLINT_SLOW_WRITE_REG(currentglintdac[i].g,IBMRGB_RAMDAC_DATA);
	    	GLINT_SLOW_WRITE_REG(currentglintdac[i].b,IBMRGB_RAMDAC_DATA);
   	    }
	}
    }

    LUTInited = TRUE;
}


void
glintInitEnvironment(void)
{
    InitLUT();
}


void
glintInitAperture(int screen_idx)
{
    if ((VGAcore) && (!vgaBase))
    	vgaBase = xf86MapVidMem(screen_idx,VGA_REGION,(pointer)0xA0000,65536);

    if (!glintVideoMem) {
    	glintVideoMem = xf86MapVidMem(screen_idx, LINEAR_REGION,
					  (pointer)(glintInfoRec.MemBase),
					  glintInfoRec.videoRam * 1024);
    }

#ifdef XFreeXDGA
    glintInfoRec.physBase = glintInfoRec.MemBase;
    glintInfoRec.physSize = glintInfoRec.videoRam * 1024;
#endif
}	


void
glintDumpRegs()
{
    int i;
    unsigned int v;

    /*
     * simply walk the registers and dump them
     */
    ErrorF("GLINT Register Dump\n");
    for( i=0; i<0x70; i+=8 )
	{
	    if( i % 32 == 0 )
		ErrorF("\n 0x%04x\t",i);
	    v = GLINT_SLOW_READ_REG(i);
	    if( v < 0x10000 )
		/*
		 * let's do hex and dec
		 */
		ErrorF("0x%08x %5d ",v,v);
	    else
		ErrorF("0x%08x ----- ",v);
	}
    ErrorF("\n");
    for( i=0x1000; i<0x1838; i+=8 )
	{
	    switch(i)
		{
		case 0x1010:
		    i = 0x1800;
		    ErrorF("\n 0x%04x\t",i);
		    break;
		case 0x1810:
		    i = 0x1820;
		case 0x1000:
		    ErrorF("\n 0x%04x\t",i);
		    break;
		}
	    v = GLINT_SLOW_READ_REG(i);
	    if( v < 0x10000 )
		/*
		 * let's do hex and dec
		 */
		ErrorF("0x%08x %5d ",v,v);
	    else
		ErrorF("0x%08x ----- ",v);
	}
    ErrorF("\n");
    for( i=0x3000; i<0x3088; i+=8 )
	{
	    if( i % 32 == 0 )
		ErrorF("\n 0x%04x\t",i);
	    v = GLINT_SLOW_READ_REG(i);
	    if( v < 0x10000 )
		/*
		 * let's do hex and dec
		 */
		ErrorF("0x%08x %5d ",v,v);
	    else
		ErrorF("0x%08x       ",v);
	}
    ErrorF("\n 0x4800\t");
    v = GLINT_SLOW_READ_REG(0x4800);
    if( v < 0x10000 )
	/*
	 * let's do hex and dec
	 */
	ErrorF("0x%08x %5d ",v,v);
    else
	ErrorF("0x%08x       ",v);
    ErrorF("\n 0x6000\t");
    v = GLINT_SLOW_READ_REG(0x6000);
    if( v < 0x10000 )
	/*
	 * let's do hex and dec
	 */
	ErrorF("0x%08x %5d ",v,v);
    else
	ErrorF("0x%08x       ",v);
    ErrorF("\n\n");
    if (IS_3DLABS_TX_MX_CLASS(coprotype) ||
	IS_3DLABS_PERMEDIA_CLASS(coprotype)) {
	ErrorF("IBM RAMDAC  Register Dump\n");
	for (i=0; i<0x100; i++)
	{
	    if( i % 8 == 0 )
		ErrorF("\n 0x%04x\t",i);
	    ErrorF("0x%02x ",glintInIBMRGBIndReg(i));
	}
	ErrorF("\n\n");
    }
}

void
glintDumpCoreDrawRegs()
{
    int i;
    unsigned int v;

    /*
     * simply walk the registers and dump them
     */
    for( i=0x8000; i<0x8038; i+=8 )
	{
	    if( i % 32 == 0 )
		ErrorF("\n 0x%04x\t",i);
	    v = GLINT_SLOW_READ_REG(i);
	    if( v < 0x10000 )
		/*
		 * let's do hex and dec
		 */
		ErrorF("0x%08x %5d ",v,v);
	    else
		ErrorF("0x%08x ----- ",v);
	}
    ErrorF("\n");
}




/*
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
 * ATI Rage128 driver for 3.3.x
 *
 * Author:  Alan Hourihane, alanh@fairlite.demon..co.uk
 *
 * $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/rage128/rage128.c,v 1.1.2.3 2000/01/18 16:30:38 tsi Exp $
 */

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

#include "rage128.h"

CARD8 BIOS[0xFFFF];
#define BIOSByte(_n)     (*((CARD8  *)(BIOS + (_n))))
#define BIOSWord(_n)     (*((CARD16 *)(BIOS + (_n))))
#define BIOSDWord(_n)     (*((CARD32 *)(BIOS + (_n))))

#ifdef XFreeXDGA 
#include "X.h"
#include "Xproto.h"
#include "scrnintstr.h"
#include "servermd.h"
#define _XF86DGA_SERVER_
#include "extensions/xf86dgastr.h"
#endif

typedef struct {
   vgaHWRec std;               /* good old IBM VGA */
   CARD32 ATIReg[0x1000];
   CARD32 ATIClock[5];
} vgaRage128Rec, *vgaRage128Ptr;

CARD16 HorizontalAdjust[5] = { 0, 18, 9, 6, 5 };

Bool RAGE128ScreenInit(ScreenPtr pScreen, pointer pbits, 
                   int xsize, int ysize, int dpix, int dpiy, int width);
static Bool     RAGE128Probe();
static char *   RAGE128Ident();
static void     RAGE128EnterLeave();
static Bool     RAGE128Init();
static int      RAGE128ValidMode();
static void *   RAGE128Save();
static void     RAGE128Restore();
static void     RAGE128Adjust();
static void     RAGE128FbInit();

static int FoundRage128 = 0;
CARD32 IOBase;

vgaVideoChipRec RAGE128 = {
	/* 
	 * Function pointers
	 */
	RAGE128Probe,
	RAGE128Ident,
	RAGE128EnterLeave,
	RAGE128Init,
	RAGE128ValidMode,
	RAGE128Save,
	RAGE128Restore,
	RAGE128Adjust,
	vgaHWSaveScreen,
	(void (*)())NoopDDA,
	RAGE128FbInit,
	(void (*)())NoopDDA,
	(void (*)())NoopDDA,	/* No banking. */
	(void (*)())NoopDDA,
	0x10000,	/* 64K VGA window. */
	0x10000,
	16,
	0xFFFF,
	0x00000, 0x10000,
	0x00000, 0x10000,
	FALSE,		/* We have seperate read and write banks in a */
			/* funny sort of way (just one bank). */
	VGA_NO_DIVIDE_VERT,
	{0,},
	8,
	TRUE,
	0,
	0,
	TRUE,
	FALSE,
	TRUE,
	NULL,
	1,                         /* ChipClockMulFactor */
	1                          /* ChipClockDivFactor */
};

#define new ((vgaRage128Ptr)vgaNewVideoState)


typedef struct {
	unsigned char r, g, b;
} LUTENTRY;

static void
RAGE128InitLUT(void)
{
    int i;

    if (vga256InfoRec.bitsPerPixel > 8) {
	extern unsigned char xf86rGammaMap[], xf86gGammaMap[], xf86bGammaMap[];
	LUTENTRY dac[256];
	
	for (i=0;i<256;i++) {
	    	dac[i].r = xf86rGammaMap[i];
	    	dac[i].g = xf86gGammaMap[i];
	    	dac[i].b = xf86bGammaMap[i];
	}

    	outl(IOBase + 0xB0, 0);
    	for (i=0; i<256; i++) {
    	    outl(IOBase + 0xB4, (dac[i].r << 16) | (dac[i].g<<8) | dac[i].b);
	}
    }
}

static char *
RAGE128Ident(n)
int n;
{
    static char *chipsets[] = {"rage128"};

    if (n + 1 > sizeof(chipsets) / sizeof(char *))
	return(NULL);
    else
	return(chipsets[n]);
}

static Bool
RAGE128Probe()
{
    pciConfigPtr PCIDevice;

    if (vga256InfoRec.chipset)
    {
	if (StrCaseCmp(vga256InfoRec.chipset, RAGE128Ident(0)))
		return (FALSE);
    }
	
    {
	if (vgaPCIInfo && vgaPCIInfo->AllCards)
	{
	    int Index = 0;
	    while (PCIDevice = vgaPCIInfo->AllCards[Index++])
	    {
		if (PCIDevice->_vendor != PCI_VENDOR_ATI)
		    continue;
		if (!vga256InfoRec.chipset) {
		    if ((PCIDevice->_device != PCI_CHIP_RAGE128RE) &&
			(PCIDevice->_device != PCI_CHIP_RAGE128RF) &&
			(PCIDevice->_device != PCI_CHIP_RAGE128RK) &&
			(PCIDevice->_device != PCI_CHIP_RAGE128RL) &&
			( (PCIDevice->_device > PCI_CHIP_RAGE128SE) || /* consecutive device ids! */
			  (PCIDevice->_device < PCI_CHIP_RAGE128PX) ))
			continue;
		}
		if ((PCIDevice->_command &
			(PCI_CMD_IO_ENABLE | PCI_CMD_MEM_ENABLE)) !=
			(PCI_CMD_IO_ENABLE | PCI_CMD_MEM_ENABLE))
			continue;
		FoundRage128 = PCIDevice->_device;
		IOBase = PCIDevice->_base1 & 0xFFFFFF00;
		RAGE128.ChipLinearBase = PCIDevice->_base0 & 0xFFFF0000;
	    }
	}
    }

    switch (FoundRage128) {
	    case PCI_CHIP_RAGE128RE:
		ErrorF("%s %s: Found ATI Rage 128 RE chip\n", 
			XCONFIG_PROBED, vga256InfoRec.name);
		break;
	    case PCI_CHIP_RAGE128RF:
		ErrorF("%s %s: Found ATI Rage 128 RF chip\n", 
			XCONFIG_PROBED, vga256InfoRec.name);
		break;
	    case PCI_CHIP_RAGE128RK:
		ErrorF("%s %s: Found ATI Rage 128 RK chip\n", 
			XCONFIG_PROBED, vga256InfoRec.name);
		break;
	    case PCI_CHIP_RAGE128RL:
		ErrorF("%s %s: Found ATI Rage 128 RL chip\n", 
			XCONFIG_PROBED, vga256InfoRec.name);
	    break;
	default:
	    if (vga256InfoRec.chipset)
		ErrorF("%s %s: Chipset override, continuing\n", 
			XCONFIG_GIVEN,vga256InfoRec.name);
	    else
		return (FALSE);
	    break;
    }

    RAGE128EnterLeave(ENTER);

    if (!vga256InfoRec.videoRam)
    {
	vga256InfoRec.videoRam = inl(IOBase + 0xF8) / 1024;
    }
    RAGE128.ChipLinearSize = vga256InfoRec.videoRam *1024;

    vga256InfoRec.maxClock = 250000;
    OFLG_SET(OPTION_NOACCEL, &RAGE128.ChipOptionFlags);
    OFLG_SET(OPTION_DAC_8_BIT, &RAGE128.ChipOptionFlags);

    OFLG_SET(OPTION_DAC_8_BIT, &vga256InfoRec.options);
    OFLG_SET(CLOCK_OPTION_PROGRAMABLE, &vga256InfoRec.clockOptions);

    vga256InfoRec.chipset = RAGE128Ident(0);
    vga256InfoRec.bankedMono = FALSE;
#ifndef MONOVGA
#ifdef XFreeXDGA 
    vga256InfoRec.directMode = XF86DGADirectPresent;
#endif
#endif
    return(TRUE);
}


static void 
RAGE128EnterLeave(enter)
Bool enter;
{
    unsigned char temp;

#ifndef MONOVGA
#ifdef XFreeXDGA 
    if (vga256InfoRec.directMode&XF86DGADirectGraphics && !enter)
	return;
#endif
#endif

    if (enter)
    {
	xf86EnableIOPorts(vga256InfoRec.scrnIndex);

	vgaIOBase = (inb(0x3CC) & 0x01) ? 0x3D0 : 0x3B0;

	/* Unprotect CRTC[0-7] */
	outb(vgaIOBase + 4, 0x11); temp = inb(vgaIOBase + 5);
	outb(vgaIOBase + 5, temp & 0x7F);
    }
    else
    {
	/* Protect CRTC[0-7] */
	outb(vgaIOBase + 4, 0x11); temp = inb(vgaIOBase + 5);
	outb(vgaIOBase + 5, (temp & 0x7F) | 0x80);

	xf86DisableIOPorts(vga256InfoRec.scrnIndex);
    }
}

static void
RAGE128Restore (restore)
vgaRage128Ptr restore;
{
    int i;
    CARD32 temp;
    vgaProtect(TRUE);

    ATI_WRITE(OVR_CLR);
    ATI_WRITE(OVR_WID_LEFT_RIGHT);
    ATI_WRITE(OVR_WID_TOP_BOTTOM);
    ATI_WRITE(OV0_SCALE_CNTL);
    ATI_WRITE(MPP_TB_CONFIG);
    ATI_WRITE(MPP_GP_CONFIG);
    ATI_WRITE(SUBPIC_CNTL);
    ATI_WRITE(VIPH_CONTROL);
    ATI_WRITE(I2C_CNTL_1);
    ATI_WRITE(GEN_INT_CNTL);
    ATI_WRITE(CAP0_TRIG_CNTL);
    ATI_WRITE(CAP1_TRIG_CNTL);
    ATI_WRITE(CRTC_GEN_CNTL);
    ATI_WRITE(CRTC_EXT_CNTL);
    vgaHWRestore((vgaHWPtr)restore);
    ATI_WRITE(DAC_CNTL);
    RAGE128InitLUT();
    ATI_WRITE(CRTC_H_TOTAL_DISP);
    ATI_WRITE(CRTC_H_SYNC_STRT_WID);
    ATI_WRITE(CRTC_V_TOTAL_DISP);
    ATI_WRITE(CRTC_V_SYNC_STRT_WID);
    ATI_WRITE(CRTC_OFFSET_CNTL);
    ATI_WRITE(CRTC_PITCH);
    ATI_WRITE(DDA_CONFIG);
    ATI_WRITE(DDA_ON_OFF);
    outl(IOBase + CLOCK_CNTL_INDEX, 1 | 7<<7);
    outl(IOBase + CLOCK_CNTL_DATA, restore->ATIClock[5]);

    outl(IOBase + CLOCK_CNTL_INDEX, 3 | 3<<8);
    while( (inl(IOBase + CLOCK_CNTL_DATA) & 0xFFFF7FFF) == 0x8000);

    outl(IOBase + CLOCK_CNTL_INDEX, 2 | 7<<7);
    outl(IOBase + CLOCK_CNTL_DATA, restore->ATIClock[1]);

    outl(IOBase + CLOCK_CNTL_INDEX, 3 | 3<<8);
    while( (inl(IOBase + CLOCK_CNTL_DATA) & 0xFFFF7FFF) == 0x8000);

    outl(IOBase + CLOCK_CNTL_INDEX, 3 | 7<<7);
    outl(IOBase + CLOCK_CNTL_DATA, restore->ATIClock[2]);

    outl(IOBase + CLOCK_CNTL_INDEX, 3 | 3<<8);
    while( (inl(IOBase + CLOCK_CNTL_DATA) & 0xFFFF7FFF) == 0x8000);

    outl(IOBase + CLOCK_CNTL_INDEX, 7 | 7<<7);
    outl(IOBase + CLOCK_CNTL_DATA, restore->ATIClock[3]);

    outl(IOBase + CLOCK_CNTL_INDEX, 3 | 3<<8);
    while( (inl(IOBase + CLOCK_CNTL_DATA) & 0xFFFF7FFF) == 0x8000);

    outl(IOBase + CLOCK_CNTL_INDEX, 9 | 7<<7);
    outl(IOBase + CLOCK_CNTL_DATA, restore->ATIClock[4]);

    outl(IOBase + CLOCK_CNTL_INDEX, 3 | 3<<8);
    while( (inl(IOBase + CLOCK_CNTL_DATA) & 0xFFFF7FFF) == 0x8000);

    outl(IOBase + CLOCK_CNTL_INDEX, 2 | 3<<8);
    temp = inl(IOBase + CLOCK_CNTL_DATA);
    outl(IOBase + CLOCK_CNTL_INDEX, 2 | 7<<7);
    outl(IOBase + CLOCK_CNTL_DATA, temp & 0xFFFCFFFE);

    vgaProtect(FALSE);
}

static void *
RAGE128Save(save)
vgaRage128Ptr save;
{
    save = (vgaRage128Ptr)vgaHWSave((vgaHWPtr)save, sizeof(vgaRage128Rec));

    ATI_READ(OVR_CLR);
    ATI_READ(OVR_WID_LEFT_RIGHT);
    ATI_READ(OVR_WID_TOP_BOTTOM);
    ATI_READ(OV0_SCALE_CNTL);
    ATI_READ(MPP_TB_CONFIG);
    ATI_READ(MPP_GP_CONFIG);
    ATI_READ(SUBPIC_CNTL);
    ATI_READ(VIPH_CONTROL);
    ATI_READ(I2C_CNTL_1);
    ATI_READ(GEN_INT_CNTL);
    ATI_READ(CAP0_TRIG_CNTL);
    ATI_READ(CAP1_TRIG_CNTL);
    ATI_READ(CRTC_GEN_CNTL);
    ATI_READ(CRTC_EXT_CNTL);
    ATI_READ(DAC_CNTL);
    ATI_READ(CRTC_H_TOTAL_DISP);
    ATI_READ(CRTC_H_SYNC_STRT_WID);
    ATI_READ(CRTC_V_TOTAL_DISP);
    ATI_READ(CRTC_V_SYNC_STRT_WID);
    ATI_READ(CRTC_OFFSET_CNTL);
    ATI_READ(CRTC_PITCH);
    ATI_READ(DDA_CONFIG);
    ATI_READ(DDA_ON_OFF);
    outl(IOBase + CLOCK_CNTL_INDEX, 1 | 1<<7);
    save->ATIClock[5] = inl(IOBase + CLOCK_CNTL_DATA);
    outl(IOBase + CLOCK_CNTL_INDEX, 2 | 1<<7);
    save->ATIClock[1] = inl(IOBase + CLOCK_CNTL_DATA);
    outl(IOBase + CLOCK_CNTL_INDEX, 3 | 1<<7);
    save->ATIClock[2] = inl(IOBase + CLOCK_CNTL_DATA);
    outl(IOBase + CLOCK_CNTL_INDEX, 7 | 1<<7);
    save->ATIClock[3] = inl(IOBase + CLOCK_CNTL_DATA);
    outl(IOBase + CLOCK_CNTL_INDEX, 9 | 1<<7);
    save->ATIClock[4] = inl(IOBase + CLOCK_CNTL_DATA);

    return ((void *) save);
}

static int
RAGE128Max(int a, int b)
{
    if (a>=b)
	return a;
    else
	return b;
}

static int
RAGE128MinBits(int temp)
{
    int required = 0;

    if (temp == 0) required = 1;
    while (temp) {
	temp >>= 1;
	required++;
    }
    return (required);
}

static Bool
RAGE128Init(mode)
DisplayModePtr mode;
{
    CARD32 RomTable;
    int maxclock, minclock, ref_divider, ref_freq, output, feedback;
    int frequency;
    unsigned int divider = -1;
    int pll, temp, temp2, Index, xclk, mclk, minimum;
    int i;
    CARD32 ML, MB, Trcd, Trp, Twr, CL, Tr2w;
    CARD32 Ron, Roff, XclksPerXfer, Rloop, Temp;

    if(!vgaHWInit (mode, sizeof(vgaRage128Rec)))
      return FALSE;

    Index = xf86ReadBIOS(vga256InfoRec.BIOSbase, 0, BIOS, sizeof(BIOS));

    RomTable = BIOSDWord(0x48);
    pll = BIOSWord(RomTable + 0x30); 
    ref_divider = BIOSWord(pll+16);
    ref_freq = BIOSWord(pll+14);
    maxclock = BIOSDWord(pll+22);
    minclock = BIOSDWord(pll+18);
    xclk = BIOSWord(pll+8);
    mclk = BIOSWord(pll+6);

    frequency = vga256InfoRec.clock[mode->Clock] / 10;

    if (frequency > maxclock)
	return FALSE;
    if ((frequency * 12) < minclock)
	return FALSE;

    output = frequency;
    if (output >= minclock && output <= maxclock) {
	divider = 1;
	goto clockdone;
    }
    output = frequency * 2;
    if (output >= minclock && output <= maxclock) {
	divider = 2;
	goto clockdone;
    }
    output = frequency * 3;
    if (output >= minclock && output <= maxclock) {
	divider = 3;
	goto clockdone;
    }
    output = frequency * 4;
    if (output >= minclock && output <= maxclock) {
	divider = 4;
	goto clockdone;
    }
    output = frequency * 6;
    if (output >= minclock && output <= maxclock) {
	divider = 6;
	goto clockdone;
    }
    output = frequency * 8;
    if (output >= minclock && output <= maxclock) {
	divider = 8;
	goto clockdone;
    }
    output = frequency * 12;
    divider = 12;
clockdone:
    feedback = ((ref_divider * output) + (ref_freq/2)) / ref_freq;

    switch (divider) {
	case 1:
		divider = 0;
		break;
	case 2:
		divider = 1;
		break;
	case 3:
		divider = 4;
		break;
	case 4:
		divider = 2;
		break;
	case 6:
		divider = 6;
		break;
	case 8:
		divider = 3;
		break;
	case 12:
		divider = 7;
		break;
	default:
		divider = 2;
		break;
    }
	
    outl(IOBase + CLOCK_CNTL_INDEX, 2);
    new->ATIClock[1] = 
      (inl(IOBase + CLOCK_CNTL_DATA) & 0xFFFCFFFE) | 0x00030001;

    outl(IOBase + CLOCK_CNTL_INDEX, 3);
    new->ATIClock[2] = 
      (inl(IOBase + CLOCK_CNTL_DATA) & 0xFFFF7C00) | 1<<15 | ref_divider;

    outl(IOBase + CLOCK_CNTL_INDEX, 7);
    new->ATIClock[3] = divider<<16 |
      (inl(IOBase + CLOCK_CNTL_DATA) & 0xFFF87800) | 1<<15 | feedback;

    new->ATIClock[4] = 0;

    outl(IOBase + CLOCK_CNTL_INDEX, 1);
    new->ATIClock[5] = 
      (inl(IOBase + CLOCK_CNTL_DATA) & 0xFFFFFFFD);

    new->ATIReg[OVR_CLR] = 0;
    new->ATIReg[OVR_WID_LEFT_RIGHT] = 0;
    new->ATIReg[OVR_WID_TOP_BOTTOM] = 0;
    new->ATIReg[OV0_SCALE_CNTL] = 0;
    new->ATIReg[MPP_TB_CONFIG] = 0;
    new->ATIReg[MPP_GP_CONFIG] = 0;
    new->ATIReg[SUBPIC_CNTL] = 0;
    new->ATIReg[VIPH_CONTROL] = 0;
    new->ATIReg[I2C_CNTL_1] = 0;
    new->ATIReg[GEN_INT_CNTL] = 0;
    new->ATIReg[CAP0_TRIG_CNTL] = 0;
    new->ATIReg[CAP1_TRIG_CNTL] = 0;

    switch (vga256InfoRec.bitsPerPixel) {
	case 4:
	    new->ATIReg[CRTC_GEN_CNTL] = 1<<8;
	    break;
	case 8:
	    new->ATIReg[CRTC_GEN_CNTL] = 2<<8;
	    break;
	case 16:
	    if (vga256InfoRec.depth == 15) 
		new->ATIReg[CRTC_GEN_CNTL] = 3<<8;
	    else
		new->ATIReg[CRTC_GEN_CNTL] = 4<<8;
	    break;
	case 24:
	    new->ATIReg[CRTC_GEN_CNTL] = 5<<8;
	    break;
	case 32:
	    new->ATIReg[CRTC_GEN_CNTL] = 6<<8;
	    break;
    }

    new->ATIReg[CRTC_GEN_CNTL] |= (mode->Flags & V_DBLSCAN ? (1<<0) : 0) |
				(mode->Flags & V_INTERLACE ? (1<<1) : 0) |
				(mode->Flags & V_CSYNC ? (1<<4) : 0) |
				1<<24 | /* Enable Extended VGA */
				1<<25; /* Enable CRT */

    outl(IOBase, CRTC_EXT_CNTL);
    new->ATIReg[CRTC_EXT_CNTL] = (inl(IOBase + 4) & 0xFFFFFCFF) | 0x48;

    outl(IOBase, DAC_CNTL);
    new->ATIReg[DAC_CNTL] = (inl(IOBase + 4) & 0x7) | 0xFF002100; 

    new->ATIReg[CRTC_H_TOTAL_DISP] = (mode->CrtcHTotal/8-1) |
					      (mode->CrtcHDisplay/8-1)<<16;

    temp = ((mode->CrtcHSyncEnd - mode->CrtcHSyncStart)/8);

    if (temp == 0)
	temp = 1;
    if (temp > 0x3f)
	temp = 0x3f;

    if (mode->Flags & V_NHSYNC)
	temp |= 0x80;

    new->ATIReg[CRTC_H_SYNC_STRT_WID] = (temp<<16) |
		(((mode->CrtcHSyncStart - 8) + 
		HorizontalAdjust[vga256InfoRec.bitsPerPixel/8]) & 0xFFFF);
	     
    temp = mode->CrtcVDisplay;
    if (mode->Flags & V_DBLSCAN) temp *= 2;
    new->ATIReg[CRTC_V_TOTAL_DISP] = ((temp-1)<<16) | 
					((mode->CrtcVTotal-1) & 0xFFFF);

    temp = mode->CrtcVSyncEnd - mode->CrtcVSyncStart;

    if (temp == 0)
	temp = 1;
    if (temp > 0x1f)
	temp = 0x1f;

    if (mode->Flags & V_NVSYNC)
	temp |= 0x80;

    new->ATIReg[CRTC_V_SYNC_STRT_WID] = (temp<<16) |
		(((mode->CrtcVSyncStart - 1) & 0xFFFF));

    new->ATIReg[CRTC_OFFSET] = 0x00;
    new->ATIReg[CRTC_OFFSET_CNTL] = 0x00;
    new->ATIReg[CRTC_PITCH] = vga256InfoRec.displayWidth >> 3;

    outl(IOBase, MEM_CNTL);
    temp = inl(IOBase + 4);

    switch (temp & 0x03) {
	case 0:
	    switch (FoundRage128) {
		case PCI_CHIP_RAGE128RE:
		case PCI_CHIP_RAGE128RF:
                    /*
                    These are 128-bit cards.
                    */
                    ML          = 4;
                    MB          = 4;
                    Trcd        = 3;
                    Trp         = 3;
                    Twr         = 1;
                    CL          = 3;
                    Tr2w        = 1;
                    Rloop       = 12 + ML;
                    break;
               	case PCI_CHIP_RAGE128RK:
               	case PCI_CHIP_RAGE128RL:
                    /*
                    These are 64-bit cards.
                    */
                    ML          = 4;
                    MB          = 8;
                    Trcd        = 3;
                    Trp         = 3;
                    Twr         = 1;
                    CL          = 3;
                    Tr2w        = 1;
                    Rloop       = 12 + ML + 1;
                    break;
               	default:
                    /*
                    Default to 64-bit values.
                    */
                    ML          = 4;
                    MB          = 8;
                    Trcd        = 3;
                    Trp         = 3;
                    Twr         = 1;
                    CL          = 3;
                    Tr2w        = 1;
                    Rloop       = 12 + ML;
                    break;
            }
            break;
        case 1:  /* SDR SGRAM (2:1) */
            ML          = 4;
            MB          = 4;
            Trcd        = 1;
            Trp         = 2;
            Twr         = 1;
            CL          = 2;
            Tr2w        = 1;
            Rloop       = 12 + ML;
            break;
        case 2:  /* DDR SGRAM */
            ML          = 4;
            MB          = 4;
            Trcd        = 3;
            Trp         = 3;
            Twr         = 2;
            CL          = 3;
            Tr2w        = 1;
            Rloop       = 12 + ML;
            break;
        default:
            /*
            Default to 64-bit, SDR-SGRAM values.
            */
            ML          = 4;
            MB          = 8;
            Trcd        = 3;
            Trp         = 3;
            Twr         = 1;
            CL          = 3;
            Tr2w        = 1;
            Rloop       = 12 + ML + 1;
    }

    xclk = xclk/100;
    temp = ( ( (xclk * 128) + (( (vga256InfoRec.clock[mode->Clock]/1000) * 
	vga256InfoRec.bitsPerPixel)/2))) / 
	(((vga256InfoRec.clock[mode->Clock]/1000)*vga256InfoRec.bitsPerPixel));

    minimum = RAGE128MinBits(temp);

    temp2 = ( ( ( (xclk * 128) << (11 - (minimum+1)) )+ (( (vga256InfoRec.clock[mode->Clock]/1000) * 
	vga256InfoRec.bitsPerPixel)/2))) / 
	(((vga256InfoRec.clock[mode->Clock]/1000)*vga256InfoRec.bitsPerPixel));

    Ron = 4 * MB + 3 * RAGE128Max(Trcd - 2, 0) + 2 * Trp + Twr + CL + Tr2w+temp;

    Ron = Ron << (11 - (minimum+1));

    Roff = temp2 * (32 - 4);

    if ((Ron + Rloop) >= Roff) {
	return (FALSE);	/* Can't do it */
    }

    new->ATIReg[DDA_CONFIG] = (minimum+1) << 16 | Rloop<<20 | temp2;
    new->ATIReg[DDA_ON_OFF] = Ron << 16 | Roff;

    return TRUE;
}

static void 
RAGE128Adjust(x, y)
int x, y;
{
	int Base = (y * vga256InfoRec.displayWidth + x);

	if (vga256InfoRec.bitsPerPixel == 16) Base <<= 1;
	if (vga256InfoRec.bitsPerPixel == 32) Base <<= 2;

	outl(IOBase, CRTC_OFFSET);
	outl(IOBase + 4, (Base >> 3) << 3);

#ifdef XFreeXDGA
	if (vga256InfoRec.directMode & XF86DGADirectGraphics) {
		/* Wait until vertical retrace is in progress. */
		while (inb(vgaIOBase + 0xA) & 0x08);
		while (!(inb(vgaIOBase + 0xA) & 0x08));
	}
#endif
}

/*
 * RAGE128ValidMode --
 *
 */
static int
RAGE128ValidMode(mode, verbose,flag)
DisplayModePtr mode;
Bool verbose;
int flag;
{
return MODE_OK;
}

void 
RAGE128FbInit()
{
   if(!OFLG_ISSET(OPTION_NOACCEL, &vga256InfoRec.options))
      RAGE128AccelInit();
}

/* $XFree86: xc/programs/Xserver/hw/xfree86/accel/mach64/mach64init.c,v 3.24.2.13 2000/05/14 02:02:11 tsi Exp $ */
/*
 * Written by Jake Richter
 * Copyright (c) 1989, 1990 Panacea Inc., Londonderry, NH - All Rights Reserved
 * Copyright 1993,1994,1995,1996,1997 by Kevin E. Martin, Chapel Hill, North Carolina.
 *
 * This code may be freely incorporated in any program without royalty, as
 * long as the copyright notice stays intact.
 *
 * Additions by Kevin E. Martin (martin@cs.unc.edu)
 *
 * KEVIN E. MARTIN AND RICKARD E. FAITH DISCLAIM ALL WARRANTIES WITH REGARD
 * TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS, IN NO EVENT SHALL KEVIN E. MARTIN BE LIABLE FOR ANY
 * SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER
 * RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF
 * CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Modified for the Mach-8 by Rickard E. Faith (faith@cs.unc.edu)
 * Rewritten for the Mach32 by Kevin E. Martin (martin@cs.unc.edu)
 * Rewritten for the Mach64 by Kevin E. Martin (martin@cs.unc.edu)
 *
 */
/* $XConsortium: mach64init.c /main/18 1996/10/27 18:06:34 kaleb $ */

#include <math.h>

#include "X.h"
#include "input.h"

#include "xf86.h"
#include "xf86_OSlib.h"
#include "mach64.h"
#include "ativga.h"
#include "mach64fifo.h"
#define XCONFIG_FLAGS_ONLY
#include "xf86_Config.h"
#ifdef linux
#include <asm/page.h>
#endif
#ifdef CSRG_BASED
#include <machine/param.h>
#endif

/* Defaults for i386 */
#ifndef PAGE_SIZE
#define PAGE_SIZE 0x1000
#endif
#ifndef PAGE_MASK
#define PAGE_MASK (~(PAGE_SIZE-1))
#endif

/* Linux and *BSD define PAGE_MASK differently */
#ifdef PAGE_MASK
#if ((PAGE_MASK) & 1)
#define XF_PAGE_MASK (~(PAGE_MASK))
#else
#define XF_PAGE_MASK (PAGE_MASK)
#endif
#endif

static LUTENTRY oldlut[256];
static Bool LUTInited = FALSE;

static int old_DAC_CNTL;
static int old_DAC_MASK;
static int old_DAC_in_clock;
static int old_DAC_out_clock;
static int old_DAC_mux_clock;
static int old_clock_sel;

static Bool mach64Inited = FALSE;

static Bool isMuxMode = FALSE;

pointer mach64VideoMem = NULL;
pointer mach64MemRegMap = NULL;
pointer mach64MemReg = NULL;
unsigned int mach64MemRegOffset;
Bool mach64InDoubleScanMode = FALSE;

static char old_ATI2E, old_ATI32, old_ATI36;
static char old_GRA06, old_SEQ02, old_SEQ04;

static unsigned long old_BUS_CNTL;
static unsigned long old_CONFIG_CNTL;
static unsigned long old_MEM_CNTL;
static unsigned long old_CRTC_GEN_CNTL;

static unsigned long old_HW_DEBUG;

static unsigned long old_CRTC_OFF_PITCH;
static unsigned long old_DST_OFF_PITCH;
static unsigned long old_SRC_OFF_PITCH;

/* Two each of these: non-shadow and shadow */
static unsigned long old_CRTC_H_SYNC_STRT_WID[2];
static unsigned long old_CRTC_H_TOTAL_DISP[2];
static unsigned long old_CRTC_V_SYNC_STRT_WID[2];
static unsigned long old_CRTC_V_TOTAL_DISP[2];

static unsigned long old_DSP_CONFIG;
static unsigned long old_DSP_ON_OFF;

static unsigned long old_LCD_INDEX;
static unsigned long old_CONFIG_PANEL;
static unsigned long old_LCD_GEN_CTRL;
static unsigned long old_HORZ_STRETCHING;
static unsigned long old_VERT_STRETCHING;
static unsigned long old_EXT_VERT_STRETCH;

static int oldClockFreq;
static unsigned char old_PLL[16];

static unsigned char old_ATI68860[4];
static unsigned char old_ATI68875[3];
static unsigned char old_CH8398;
static unsigned char old_STG170X[4];
static unsigned char old_ATT20C408;
static unsigned char old_IBMRGB514[0x100];

static double current_dot_clock = -1; /* Only used by SetDSPRegs */
static int current_hdisplay;

/*
 * mach64CalcCRTCRegs --
 *	Initializes the Mach64 for the currently selected CRTC parameters.
 */
void mach64CalcCRTCRegs(crtcRegs, mode)
     mach64CRTCRegPtr crtcRegs;
     DisplayModePtr mode;
{
    int i;
    int pixel_delay;

    current_hdisplay = mode->HDisplay;

    if ((mode->CrtcHTotal > 2048) && (mach64RamdacSubType != DAC_INTERNAL)) {
	isMuxMode = TRUE;

	crtcRegs->h_total_disp =
	    (((mode->CrtcHDisplay >> 4) - 1) << 16) |
		((mode->CrtcHTotal >> 4) - 1);
	crtcRegs->h_sync_strt_wid =
	    (((mode->CrtcHSyncEnd - mode->CrtcHSyncStart) >> 4) << 16) |
		((mode->CrtcHSyncStart >> 4) - 1);

	if ((crtcRegs->h_sync_strt_wid >> 16) > 0x1f) {
	    ErrorF("%s %s: Horizontal Sync width (%d) in mode \"%s\"\n",
		   XCONFIG_PROBED, mach64InfoRec.name,
		   crtcRegs->h_sync_strt_wid >> 16,
		   mode->name);
	    ErrorF("\tshortened to 496 pixels\n");
	    crtcRegs->h_sync_strt_wid &= 0x001fffff;
	    crtcRegs->h_sync_strt_wid |= 0x001f0000;
	}
    } else {
	isMuxMode = FALSE;

	crtcRegs->h_total_disp =
	    (((mode->CrtcHDisplay >> 3) - 1) << 16) |
		((mode->CrtcHTotal >> 3) - 1);
	crtcRegs->h_sync_strt_wid =
	    (((mode->CrtcHSyncEnd - mode->CrtcHSyncStart) >> 3) << 16) |
		((mode->CrtcHSyncStart >> 3) - 1);

	if ((crtcRegs->h_sync_strt_wid >> 16) > 0x1f) {
	    ErrorF("%s %s: Horizontal Sync width (%d) in mode \"%s\"\n",
		   XCONFIG_PROBED, mach64InfoRec.name,
		   crtcRegs->h_sync_strt_wid >> 16,
		   mode->name);
	    ErrorF("\tshortened to 248 pixels\n");
	    crtcRegs->h_sync_strt_wid &= 0x001fffff;
	    crtcRegs->h_sync_strt_wid |= 0x001f0000;
	}
    }

    if (mode->Flags & V_NHSYNC)
	crtcRegs->h_sync_strt_wid |= CRTC_H_SYNC_NEG;

    crtcRegs->v_total_disp =
	((mode->CrtcVDisplay - 1) << 16) |
	    (mode->CrtcVTotal - 1);
    crtcRegs->v_sync_strt_wid =
	((mode->CrtcVSyncEnd - mode->CrtcVSyncStart) << 16) |
	    (mode->CrtcVSyncStart - 1);

    if ((crtcRegs->v_sync_strt_wid >> 16) > 0x1f) {
	ErrorF("%s %s: Vertical Sync width (%d) in mode \"%s\"\n",
	       XCONFIG_PROBED, mach64InfoRec.name,
	       crtcRegs->v_sync_strt_wid >> 16,
	       mode->name);
	ErrorF("\tshortened to 31 lines\n");
	crtcRegs->v_sync_strt_wid &= 0x001fffff;
	crtcRegs->v_sync_strt_wid |= 0x001f0000;
    }

    if (mode->Flags & V_NVSYNC)
	crtcRegs->v_sync_strt_wid |= CRTC_V_SYNC_NEG;

    switch(mach64InfoRec.bitsPerPixel)
    {
	case 8:
    	    crtcRegs->color_depth = CRTC_PIX_WIDTH_8BPP;
	    break;
	case 16:
	    if (mach64WeightMask == RGB16_555)
		crtcRegs->color_depth = CRTC_PIX_WIDTH_15BPP;
	    else
		crtcRegs->color_depth = CRTC_PIX_WIDTH_16BPP;
	    break;
	case 32:
    	    crtcRegs->color_depth = CRTC_PIX_WIDTH_32BPP;
	    break;
    }

    crtcRegs->crtc_gen_cntl = 0;
    if (mode->Flags & V_INTERLACE)
	crtcRegs->crtc_gen_cntl |= CRTC_INTERLACE_EN;
    if (mode->Flags & V_CSYNC)
	crtcRegs->crtc_gen_cntl |= CRTC_CSYNC_EN;
    if (mode->Flags & V_DBLSCAN) {
	crtcRegs->crtc_gen_cntl |= CRTC_DBL_SCAN_EN;
	mach64InDoubleScanMode = TRUE;
    } else
	mach64InDoubleScanMode = FALSE;
    if (OFLG_ISSET(OPTION_CSYNC, &mach64InfoRec.options))
	crtcRegs->crtc_gen_cntl |= CRTC_CSYNC_EN;

    if (!OFLG_ISSET(OPTION_NO_PROGRAM_CLOCKS, &mach64InfoRec.options) &&
	OFLG_ISSET(CLOCK_OPTION_PROGRAMABLE, &mach64InfoRec.clockOptions)) {
	crtcRegs->clock_cntl = mach64CXClk;
	crtcRegs->dot_clock = mode->SynthClock / 10;

	switch (mach64RamdacSubType) {
	case DAC_STG1702:
	case DAC_STG1703:
	    switch (mach64InfoRec.bitsPerPixel) {
	    case 8:
		if (crtcRegs->dot_clock > 11000)
		    crtcRegs->clock_cntl |= CLOCK_DIV2;
		break;
	    case 32:
		crtcRegs->dot_clock += crtcRegs->dot_clock >> 1;
		break;
	    default:
		break;
	    }
	    break;
	case DAC_ATT20C408:
	case DAC_CH8398:
	    switch (mach64InfoRec.bitsPerPixel) {
	    case 8:
		if (crtcRegs->dot_clock > 8000)
		    crtcRegs->clock_cntl |= CLOCK_DIV2;
		break;
	    case 32:
		crtcRegs->dot_clock += crtcRegs->dot_clock >> 1;
		break;
	    default:
		break;
	    }
	    break;
	default:
	    break;
	}
    } else {
	crtcRegs->clock_cntl = mode->Clock;
	crtcRegs->dot_clock = mach64InfoRec.clock[mode->Clock] / 10;

	switch (mach64RamdacSubType) {
	case DAC_STG1702:
	case DAC_STG1703:
	    switch (mach64InfoRec.bitsPerPixel) {
	    case 8:
		if (crtcRegs->dot_clock > 11000)
		    crtcRegs->clock_cntl |= CLOCK_DIV2;
		break;
	    case 32:
		crtcRegs->dot_clock += crtcRegs->dot_clock >> 1;
		i = xf86GetNearestClock(&mach64InfoRec,
					crtcRegs->dot_clock*10);
		if (abs(crtcRegs->dot_clock*10 - mach64InfoRec.clock[i])
		    <= 2000)
		    crtcRegs->clock_cntl = i;
		else
		    crtcRegs->clock_cntl = mach64CXClk;
		break;
	    default:
		break;
	    }
	    break;
	case DAC_ATT20C408:
	case DAC_CH8398:
	    switch (mach64InfoRec.bitsPerPixel) {
	    case 8:
		if (crtcRegs->dot_clock > 8000)
		    crtcRegs->clock_cntl |= CLOCK_DIV2;
		break;
	    case 32:
		crtcRegs->dot_clock += crtcRegs->dot_clock >> 1;
		i = xf86GetNearestClock(&mach64InfoRec,
					crtcRegs->dot_clock*10);
		if (abs(crtcRegs->dot_clock*10 - mach64InfoRec.clock[i])
		    <= 2000)
		    crtcRegs->clock_cntl = i;
		else
		    crtcRegs->clock_cntl = mach64CXClk;
		break;
	    default:
		break;
	    }
	    break;
	default:
	    break;
	}
    }

    /* Set the pixel delay */
    pixel_delay = 0;

    switch (mach64RamdacSubType) {
    case DAC_IBMRGB514:
	switch (crtcRegs->color_depth) {
	case CRTC_PIX_WIDTH_8BPP:
	    pixel_delay = 4;
	    break;
	case CRTC_PIX_WIDTH_15BPP:
	case CRTC_PIX_WIDTH_16BPP:
	    pixel_delay = 2;
	    break;
	case CRTC_PIX_WIDTH_32BPP:
	    pixel_delay = 1;
	    break;
	}
	break;
    default:
	break;
    }

    crtcRegs->h_sync_strt_wid = 
	((crtcRegs->h_sync_strt_wid & 0xffffff00) |
	 ((crtcRegs->h_sync_strt_wid & 0xff) + pixel_delay));

    crtcRegs->fifo_v1 =	mach64FIFOdepth((int)crtcRegs->color_depth,
					(int)crtcRegs->dot_clock, mode->HDisplay);

    if (mach64LCDPanelID < 0)
	return;

    /* Panel setup */
    if (mach64ChipType == MACH64_LG_ID) {
	crtcRegs->horz_stretching = regr(HORZ_STRETCHING);
    } else {
	outb(ioLCD_INDEX, LCD_HORZ_STRETCHING);
	crtcRegs->horz_stretching = inl(ioLCD_DATA);
	outb(ioLCD_INDEX, LCD_EXT_VERT_STRETCH);
	crtcRegs->ext_vert_stretch = inl(ioLCD_DATA) &
	    ~(AUTO_VERT_RATIO | VERT_STRETCH_MODE);

	/*
	 * FIXME:  On a 1024x768 panel, vertical blending does not work when
	 *         HDIsplay is greater than 896.
	 */
	if ((mode->HDisplay < mach64LCDHorizontal) &&
	    (mode->VDisplay < mach64LCDVertical))
	    crtcRegs->ext_vert_stretch |= VERT_STRETCH_MODE;
    }

    crtcRegs->horz_stretching &= ~(HORZ_STRETCH_RATIO | HORZ_STRETCH_LOOP |
	AUTO_HORZ_RATIO | HORZ_STRETCH_MODE | HORZ_STRETCH_EN);
    if (mode->HDisplay < mach64LCDHorizontal)
	crtcRegs->horz_stretching |= HORZ_STRETCH_MODE | HORZ_STRETCH_EN |
	    (((mode->HDisplay & ~7) << 12) / mach64LCDHorizontal);

    if (mode->VDisplay >= mach64LCDVertical)
	crtcRegs->vert_stretching = 0;
    else
	crtcRegs->vert_stretching = VERT_STRETCH_USE0 | VERT_STRETCH_EN |
	    ((mode->VDisplay << 10) / mach64LCDVertical);
}

/*
 * mach64FIFOdepth --
 *	Calculates the correct FIFO depth for the Mach64 depending on the
 *	color depth and clock selected.
 */
int mach64FIFOdepth(cdepth, clock, width)
    int cdepth;
    int clock;
    int width;
{
    int fifo_depth;

    if (mach64ChipType == MACH64_VT_ID) {
	if (mach64ChipRev == 0x48) { /* VTA4 */
	    fifo_depth = mach64FIFOdepthVTA4(cdepth, clock, width);
	} else { /* VTA3 */
	    fifo_depth = mach64FIFOdepthVTA3(cdepth, clock, width);
	}
    } else if (mach64ChipType == MACH64_GT_ID) {
	fifo_depth = mach64FIFOdepthGT(cdepth, clock, width);
    } else if (mach64ChipType == MACH64_CT_ID && mach64ChipRev == 0x0a) {
	/* CT-D has a larger FIFO and thus requires special code */
	fifo_depth = mach64FIFOdepthCTD(cdepth, clock, width);
    } else if (mach64ChipType == MACH64_CT_ID ||
	       mach64ChipType == MACH64_ET_ID) {
	fifo_depth = mach64FIFOdepthCT(cdepth, clock, width);
    } else {
	fifo_depth = mach64FIFOdepthDefault(cdepth, clock, width);
    }

#ifdef DEBUG
    ErrorF("CRTC FIFO set to %d\n", fifo_depth);
#endif
    return(fifo_depth);
}


void mach64DACRead4()
{
    (void)inb(ioDAC_REGS);

    (void)inb(ioDAC_REGS+2);
    (void)inb(ioDAC_REGS+2);
    (void)inb(ioDAC_REGS+2);
    (void)inb(ioDAC_REGS+2);
}

/*
 * mach64StrobeClock --
 *
 */
void mach64StrobeClock()
{
    char tmp;

#ifdef __alpha__
    usleep(26);
#else
    /* Delay for 26 us */
    for (tmp = 0; tmp < 26; tmp++)
	GlennsIODelay();
#endif

    tmp = inb(ioCLOCK_CNTL);
    outb(ioCLOCK_CNTL, tmp | CLOCK_STROBE);
}

/*
 * mach64ICS2595_1bit --
 *
 */
void mach64ICS2595_1bit(data)
    char data;
{
    char tmp;

    tmp = inb(ioCLOCK_CNTL);
    outb(ioCLOCK_CNTL, (tmp & ~0x04) | (data << 2));

    tmp = inb(ioCLOCK_CNTL);
    outb(ioCLOCK_CNTL, (tmp & ~0x08) | (0 << 3));

    mach64StrobeClock();

    tmp = inb(ioCLOCK_CNTL);
    outb(ioCLOCK_CNTL, (tmp & ~0x08) | (1 << 3));

    mach64StrobeClock();
}

/*
 * mach64ProgramICS2595 --
 *
 */
void mach64ProgramICS2595(clkCntl, MHz100)
    int clkCntl;
    int MHz100;
{
    char old_clock_cntl;
    char old_crtc_ext_disp;
    unsigned int program_word;
    unsigned int divider;
    int i;

#define MAX_FREQ_2595 15938
#define ABS_MIN_FREQ_2595 1000
#define MIN_FREQ_2595 8000
#define N_ADJ_2595 257
#define REF_DIV_2595 46
#define REF_FREQ_2595 1432
#define STOP_BITS_2595 0x1800

    old_clock_cntl = inb(ioCLOCK_CNTL);
    outb(ioCLOCK_CNTL, 0);

    old_crtc_ext_disp = inb(ioCRTC_GEN_CNTL+3);
    outb(ioCRTC_GEN_CNTL+3, old_crtc_ext_disp | (CRTC_EXT_DISP_EN >> 24));

    usleep(15000); /* delay for 15 ms */

    /* Calculate the programming word */
    program_word = -1;
    divider = 1;

    if (MHz100 > MAX_FREQ_2595)
	MHz100 = MAX_FREQ_2595;
    else if (MHz100 < ABS_MIN_FREQ_2595)
	program_word = 0;
    else
	while (MHz100 < MIN_FREQ_2595) {
	    MHz100 *= 2;
	    divider *= 2;
	}

    MHz100 *= 1000;
    MHz100 = (REF_DIV_2595*MHz100)/REF_FREQ_2595;

    MHz100 += 500;
    MHz100 /= 1000;

    if (program_word == -1) {
	program_word = MHz100 - N_ADJ_2595;
	switch (divider) {
	case 1:
	    program_word |= 0x0600;
	    break;
	case 2:
	    program_word |= 0x0400;
	    break;
	case 4:
	    program_word |= 0x0200;
	    break;
	case 8:
	default:
	    break;
	}
    }

    program_word |= STOP_BITS_2595;

    /* Turn off interrupts */
    (void)xf86DisableInterrupts(); 

    /* Program the clock chip */
    outb(ioCLOCK_CNTL, 0);
    mach64StrobeClock();
    outb(ioCLOCK_CNTL, 1);
    mach64StrobeClock();

    mach64ICS2595_1bit(1); /* Send start bits */
    mach64ICS2595_1bit(0);
    mach64ICS2595_1bit(0);

    for (i = 0; i < 5; i++) {
	mach64ICS2595_1bit(clkCntl & 1);
	clkCntl >>= 1;
    }

    for (i = 0; i < 8 + 1 + 2 + 2; i++) {
	mach64ICS2595_1bit(program_word & 1);
	program_word >>= 1;
    }

    /* Enable interrupts */
    (void)xf86EnableInterrupts();

    usleep(1000); /* delay for 1 ms */

    (void)inb(ioDAC_REGS); /* Clear DAC Counter */
    outb(ioCRTC_GEN_CNTL+3, old_crtc_ext_disp);
    outb(ioCLOCK_CNTL, old_clock_cntl | CLOCK_STROBE);
}

/*
 * mach64ProgramClk1703 --
 *
 */
void mach64ProgramClk1703(clkCntl, MHz100)
    int clkCntl;
    int MHz100;
{
    char old_crtc_ext_disp;
    unsigned int program_word;
    unsigned int temp, tempB;
    unsigned short mhz100 = MHz100;
    unsigned short tempA, remainder, preRemainder, divider;

    old_crtc_ext_disp = inb(ioCRTC_GEN_CNTL+3);
    outb(ioCRTC_GEN_CNTL+3, old_crtc_ext_disp | (CRTC_EXT_DISP_EN >> 24));

#define MIN_N_1703		6

    /* Calculate program word */
    if (MHz100 == 0) {
	program_word = 0xe0;
    } else {
	if (mhz100 < mach64MinFreq) mhz100 = mach64MinFreq;
	if (mhz100 > mach64MaxFreq) mhz100 = mach64MaxFreq;

	divider = 0;
	while (mhz100 < (mach64MinFreq << 3)) {
	    mhz100 <<= 1;
	    divider += 0x20;
	}

	temp = (unsigned int)(mhz100);
	temp = (unsigned int)(temp * (MIN_N_1703 + 2));
	temp -= (short)(mach64RefFreq << 1);

	tempA = MIN_N_1703;
	preRemainder = 0xffff;

	do {
	    tempB = temp;
	    remainder = tempB % mach64RefFreq;
	    tempB = tempB / mach64RefFreq;

	    if ((tempB & 0xffff) <= 127 && (remainder <= preRemainder)) {
		preRemainder = remainder;
		divider &= ~0x1f;
		divider |= tempA;
		divider = (divider & 0x00ff) + ((tempB & 0xff) << 8);
	    }

	    temp += mhz100;
	    tempA++;
	} while (tempA <= (MIN_N_1703 << 1));

	program_word = divider;
    }

    /* Program clock */
    mach64DACRead4();

    (void)inb(ioDAC_REGS+2);
    outb(ioDAC_REGS+2, (clkCntl << 1) + 0x20);
    outb(ioDAC_REGS+2, 0);
    outb(ioDAC_REGS+2, (program_word & 0xff00) >> 8);
    outb(ioDAC_REGS+2, (program_word & 0xff));

    (void)inb(ioDAC_REGS); /* Clear DAC Counter */
    outb(ioCRTC_GEN_CNTL+3, old_crtc_ext_disp);
}

/*
 * mach64ProgramClk8398 --
 *
 */
void mach64ProgramClk8398(clkCntl, MHz100)
    int clkCntl;
    int MHz100;
{
    char old_crtc_ext_disp;
    float tempA, tempB, fOut, longMHz100, diff, preDiff;
    unsigned int temp;
    unsigned short program_word;
    unsigned short m, n, k=0, save_m, save_n, twoToKth;
    unsigned short mhz100 = MHz100;

    old_crtc_ext_disp = inb(ioCRTC_GEN_CNTL+3);
    outb(ioCRTC_GEN_CNTL+3, old_crtc_ext_disp | (CRTC_EXT_DISP_EN >> 24));

#define MIN_M		2
#define MAX_M		30
#define MIN_N		35
#define MAX_N		255-8

    /* Calculate program word */
    if (mhz100 == 0) {
	program_word = 0xe0;
    } else {
	if (mhz100 < mach64MinFreq) mhz100 = mach64MinFreq;
	if (mhz100 > mach64MaxFreq) mhz100 = mach64MaxFreq;

	longMHz100 = (float) mhz100/100;

	while (mhz100 < (mach64MinFreq << 3)) {
	    mhz100 <<= 1;
	    k++;
	}

	twoToKth = 1 << k;
	diff = 0.0;
	preDiff = 0xffffffff;

	for (m = MIN_M; m <= MAX_M; m++) {
	    for (n = MIN_N; n <= MAX_N; n++) {
		tempA = 14.31818;
		tempA *= (float)(n+8);
		tempB = (float)twoToKth;
		tempB *= (m+2);
		fOut = tempA/tempB;

		if (longMHz100 > fOut)
		    diff = longMHz100 - fOut;
		else
		    diff = fOut - longMHz100;

		if (diff < preDiff) {
		    save_m = m;
		    save_n = n;
		    preDiff = diff;
		}
	    }
	}

	program_word = (k << 6) + (save_m) + (save_n << 8);
    }

    /* Program clock */
    temp = inb(ioDAC_CNTL);
    outb(ioDAC_CNTL, temp | DAC_EXT_SEL_RS2 | DAC_EXT_SEL_RS3);

    outb(ioDAC_REGS, clkCntl);
    outb(ioDAC_REGS+1, (program_word & 0xff00) >> 8);
    outb(ioDAC_REGS+1, (program_word & 0xff));

    temp = inb(ioDAC_CNTL);
    outb(ioDAC_CNTL, (temp & ~DAC_EXT_SEL_RS2) | DAC_EXT_SEL_RS3);

    (void)inb(ioDAC_REGS); /* Clear DAC Counter */
    outb(ioCRTC_GEN_CNTL+3, old_crtc_ext_disp);
}

/*
 * mach64ProgramClk408 --
 *
 */
void mach64ProgramClk408(clkCntl, MHz100)
    int clkCntl;
    int MHz100;
{
    char old_crtc_ext_disp;
    unsigned char tmpA, tmpB, tmpC;
    unsigned int temp, tempB;
    unsigned short program_word;
    unsigned short remainder, preRemainder;
    unsigned short mhz100 = MHz100;
    short divider = 0, tempA;

    old_crtc_ext_disp = inb(ioCRTC_GEN_CNTL+3);
    outb(ioCRTC_GEN_CNTL+3, old_crtc_ext_disp | (CRTC_EXT_DISP_EN >> 24));

#define MIN_N_408		2

    /* Calculate program word */
    if (mhz100 == 0) {
	program_word = 0xff;
    } else {
	if (mhz100 < mach64MinFreq) mhz100 = mach64MinFreq;
	if (mhz100 > mach64MaxFreq) mhz100 = mach64MaxFreq;

	while (mhz100 < (mach64MinFreq << 3)) {
	    mhz100 <<= 1;
	    divider += 0x40;
	}

	temp = (unsigned int)mhz100;
	temp = (unsigned int)(temp * (MIN_N_408 + 2));
	temp -= ((short)(mach64RefFreq << 1));

	tempA = MIN_N_408;
	preRemainder = 0xffff;

	do {
	    tempB = temp;
	    remainder = tempB % mach64RefFreq;
	    tempB = tempB / mach64RefFreq;
	    if (((tempB & 0xffff) <= 255) && (remainder <= preRemainder)) {
		preRemainder = remainder;
		divider &= ~0x3f;
		divider |= tempA;
		divider = (divider & 0x00ff) + ((tempB & 0xff) << 8);
	    }
	    temp += mhz100;
	    tempA++;
	} while (tempA <= 32);

	program_word = divider;
    }

    /* Program clock */
    mach64DACRead4();
    tmpB = inb(ioDAC_REGS+2) | 1;
    mach64DACRead4();
    outb(ioDAC_REGS+2, tmpB);

    tmpA = tmpB;
    tmpC = tmpA;
    tmpA |= 8;
    tmpB = 1;

    outb(ioDAC_REGS, tmpB);
    outb(ioDAC_REGS+2, tmpA);

    usleep(400); /* delay for 400 us */
    clkCntl = (clkCntl << 2) + 0x40;
    tmpB = clkCntl;
    tmpA = program_word >> 8;

    outb(ioDAC_REGS, tmpB);
    outb(ioDAC_REGS+2, tmpA);

    tmpB = clkCntl+1;
    tmpA = (unsigned char)program_word;

    outb(ioDAC_REGS, tmpB);
    outb(ioDAC_REGS+2, tmpA);

    tmpB = clkCntl+2;
    tmpA = 0x77;

    outb(ioDAC_REGS, tmpB);
    outb(ioDAC_REGS+2, tmpA);

    usleep(400); /* delay for 400 us */
    tmpA = tmpC & (~(1 | 8));
    tmpB = 1;

    outb(ioDAC_REGS, tmpB);
    outb(ioDAC_REGS+2, tmpA);

    (void)inb(ioDAC_REGS); /* Clear DAC Counter */
    outb(ioCRTC_GEN_CNTL+3, old_crtc_ext_disp);
}

/*
 * mach64ProgramClkMach64CT --
 *
 */
void mach64ProgramClkMach64CT(clkCntl, MHz100)
    int clkCntl;
    int MHz100;
{
    char old_crtc_ext_disp;
#ifdef DEBUG
    extern void mach64PrintCTPLL();
#endif
    int M, N, P, R;
    float Q;
    int postDiv;
    int mhz100 = MHz100;
    unsigned char tmp1, tmp2;
    int ext_div = 0;

    old_crtc_ext_disp = inb(ioCRTC_GEN_CNTL+3);
    outb(ioCRTC_GEN_CNTL+3, old_crtc_ext_disp | (CRTC_EXT_DISP_EN >> 24));

    M = mach64RefDivider;
    R = mach64RefFreq;

    if (clkCntl > 3) clkCntl = 3;

    if (mhz100 < mach64MinFreq) mhz100 = mach64MinFreq;
    if (mhz100 > mach64MaxFreq) mhz100 = mach64MaxFreq;

    Q = (mhz100 * M)/(2.0 * R);

    if (mach64HasDSP) {
	if (Q > 255) {
	    ErrorF("mach64ProgramClkMach64CT: Warning: Q > 255\n");
	    Q = 255;
	    P = 0;
	    postDiv = 1;
	} else if (Q > 127.5) {
	    P = 0;
	    postDiv = 1;
	} else if (Q > 85) {
	    P = 1;
	    postDiv = 2;
	} else if (Q > 63.75) {
	    P = 0;
	    postDiv = 3;
	    ext_div = 1;
	} else if (Q > 42.5) {
	    P = 2;
	    postDiv = 4;
	} else if (Q > 31.875) {
	    P = 2;
	    postDiv = 6;
	    ext_div = 1;
	} else if (Q > 21.25) {
	    P = 3;
	    postDiv = 8;
	} else if (Q >= 10.6666666667) {
	    P = 3;
	    postDiv = 12;
	    ext_div = 1;
	} else {
	    ErrorF("mach64ProgramClkMach64CT: Warning: Q < 10.66666667\n");
	    P = 3;
	    postDiv = 12;
	    ext_div = 1;
	}
    } else {
	if (Q > 255) {
	    ErrorF("mach64ProgramClkMach64CT: Warning: Q > 255\n");
	    Q = 255;
	    P = 0;
	}
	else if (Q > 127.5)
	    P = 0;
	else if (Q > 63.75)
	    P = 1;
	else if (Q > 31.875)
	    P = 2;
	else if (Q >= 16)
	    P = 3;
	else {
	    ErrorF("mach64ProgramClkMach64CT: Warning: Q < 16\n");
	    P = 3;
	}
	postDiv = 1 << P;
    }
    N = (int)(Q * postDiv + 0.5);

    current_dot_clock = (2.0 * R * N)/(M * postDiv);

#ifdef DEBUG
    ErrorF("Q = %f N = %d P = %d, postDiv = %d R = %d M = %d\n", Q, N, P, postDiv, R, M);
    ErrorF("New freq: %.2f\n", (double)((2 * R * N)/(M * postDiv)) / 100.0);
#endif

    outb(ioCLOCK_CNTL + 1, PLL_VCLK_CNTL << 2);
    tmp1 = inb(ioCLOCK_CNTL + 2) | 0x03;
    outb(ioCLOCK_CNTL + 1, (PLL_VCLK_CNTL  << 2) | PLL_WR_EN);
    outb(ioCLOCK_CNTL + 2, tmp1 | 0x04);
    outb(ioCLOCK_CNTL + 1, VCLK_POST_DIV << 2);
    tmp2 = inb(ioCLOCK_CNTL + 2);
    outb(ioCLOCK_CNTL + 1, ((VCLK0_FB_DIV + clkCntl) << 2) | PLL_WR_EN);
    outb(ioCLOCK_CNTL + 2, N);
    outb(ioCLOCK_CNTL + 1, (VCLK_POST_DIV << 2) | PLL_WR_EN);
    outb(ioCLOCK_CNTL + 2,
	 (tmp2 & ~(0x03 << (2 * clkCntl))) | (P << (2 * clkCntl)));
    outb(ioCLOCK_CNTL + 1, (PLL_VCLK_CNTL << 2) | PLL_WR_EN);
    outb(ioCLOCK_CNTL + 2, tmp1 & ~0x04);

    if (mach64HasDSP) {
	outb(ioCLOCK_CNTL + 1, PLL_XCLK_CNTL << 2);
	tmp1 = inb(ioCLOCK_CNTL + 2);
	outb(ioCLOCK_CNTL + 1, (PLL_XCLK_CNTL << 2) | PLL_WR_EN);
	if (ext_div)
	    outb(ioCLOCK_CNTL + 2, tmp1 | (1 << (clkCntl + 4)));
	else
	    outb(ioCLOCK_CNTL + 2, tmp1 & ~(1 << (clkCntl + 4)));
    }

    usleep(5000);

    (void)inb(ioDAC_REGS); /* Clear DAC Counter */
    outb(ioCRTC_GEN_CNTL+3, old_crtc_ext_disp);

    return;
}

/* 
 * mach64P_RGB514Index --
 * 
 */
void mach64P_RGB514Index(index, data)
    int index;
    int data;
{
    int temp;

    temp = inb(ioDAC_CNTL);
    outb(ioDAC_CNTL, (temp & ~DAC_EXT_SEL_RS3) | DAC_EXT_SEL_RS2);

    outb(ioDAC_REGS, index & 0xff);
    outb(ioDAC_REGS+1, index >> 8);
    outb(ioDAC_REGS+2, data & 0xff);

    temp = inb(ioDAC_CNTL);
    outb(ioDAC_CNTL, (temp & ~(DAC_EXT_SEL_RS3 | DAC_EXT_SEL_RS2)));
}

/* 
 * mach64R_RGB514Index --
 * 
 */
unsigned char mach64R_RGB514Index(index)
    int index;
{
    int temp;
    unsigned char retval;

    temp = inb(ioDAC_CNTL);
    outb(ioDAC_CNTL, (temp & ~DAC_EXT_SEL_RS3) | DAC_EXT_SEL_RS2);

    outb(ioDAC_REGS, index & 0xff);
    outb(ioDAC_REGS+1, index >> 8);
    retval = inb(ioDAC_REGS+2);

    temp = inb(ioDAC_CNTL);
    outb(ioDAC_CNTL, (temp & ~(DAC_EXT_SEL_RS3 | DAC_EXT_SEL_RS2)));

    return retval;
}

/*
 * mach64ProgramClkRGB514 --
 *
 */
void mach64ProgramClkRGB514(clkCntl, MHz100)
    int clkCntl;
    int MHz100;
{
    char old_crtc_ext_disp;
    unsigned int program_word;
    unsigned short mhz100 = MHz100;
    float target, ref_freq;
    unsigned char m, n, p;
    float actual, save_freq, error, temp = 0xffff;

    old_crtc_ext_disp = inb(ioCRTC_GEN_CNTL+3);
    outb(ioCRTC_GEN_CNTL+3, old_crtc_ext_disp | (CRTC_EXT_DISP_EN >> 24));

#define RGB514_MIN_FREQ 1600
#define RGB514_MAX_FREQ 22000
#define RGB514_MAX_N    0x1f
#define RGB514_MAX_M    0x3f

    /* Calculate program word */
    if (mhz100 < RGB514_MIN_FREQ) mhz100 = RGB514_MIN_FREQ;
    if (mhz100 > RGB514_MAX_FREQ) mhz100 = RGB514_MAX_FREQ;

    target = (float)mhz100 / 100;
    ref_freq = 14.318;

    if (target < 32) p = 0;
    else if (target < 64) p = 1;
    else if (target < 128) p = 2;
    else p = 3;

    for (m = 0; m <= RGB514_MAX_M; m++) {
	for (n = 2; n <= RGB514_MAX_N; n++) {
	    actual = (float)(ref_freq * (m+65)) / (n * (1 << (3-p)));
	    error = target - actual;

	    if (error < 0) error = -error;
	    if (error < temp) {
		save_freq = actual;
		temp = error;
		program_word = ((((m & 0x3f) | ((p & 3) << 6)) << 8) |
				(n & 0x1f));
	    }
	}
    }

    /* Program clock */
    clkCntl = (clkCntl << 1) + 0x20;
    mach64P_RGB514Index(clkCntl, program_word >> 8);

    clkCntl++;
    mach64P_RGB514Index(clkCntl, program_word & 0xff);

    (void)inb(ioDAC_REGS); /* Clear DAC Counter */
    outb(ioCRTC_GEN_CNTL+3, old_crtc_ext_disp);
}

/*
 * mach64ProgramClk --
 *	Program the clock chip for the use with RAMDAC.
 */
void mach64ProgramClk(clkCntl, MHz100)
    int clkCntl;
    int MHz100;
{
    switch (mach64ClockType) {
    case CLK_ATI18818_1:
	mach64ProgramICS2595(clkCntl, MHz100);
	break;
    case CLK_STG1703:
	mach64ProgramClk1703(clkCntl, MHz100);
	break;
    case CLK_CH8398:
	mach64ProgramClk8398(clkCntl, MHz100);
	break;
    case CLK_INTERNAL:
	mach64ProgramClkMach64CT(clkCntl, MHz100);
	break;
    case CLK_ATT20C408:
	mach64ProgramClk408(clkCntl, MHz100);
	break;
    case CLK_IBMRGB514:
	mach64ProgramClkRGB514(clkCntl, MHz100);
	break;
    default:
	ErrorF("mach64ProgramClk: ClockType %d not currently supported.\n",
	       mach64ClockType);
	break;
    }
}

/*
 * mach64SetDSPRegs --
 *	Initializes the DSP registers.
 */
void mach64SetDSPRegs(depth)
     int depth;
{
    int bpp, bx, bt, p, roff, rloop, n, tmp, page_size;
    int trp, trcd, tcrd, tras;
    int pfc, rcc, ron;
    int fifo_depth;
    double x, t, f;
    unsigned short dsp_on, dsp_off, dsp_xclks_per_qw;

    switch (depth) {
    case CRTC_PIX_WIDTH_8BPP:  bpp =  8; break;
    case CRTC_PIX_WIDTH_15BPP: bpp = 16; break;
    case CRTC_PIX_WIDTH_16BPP: bpp = 16; break;
    case CRTC_PIX_WIDTH_24BPP: bpp = 24; break;
    case CRTC_PIX_WIDTH_32BPP: bpp = 32; break;
    default:                   bpp =  4; break;
    }

    x = ((double)mach64XCLK * 64.0) / (current_dot_clock * (double)bpp);
    if (mach64LCDPanelID >= 0)	/* Compensate for horizontal stretching */
	x *= (double)mach64LCDHorizontal / (double)current_hdisplay;
    bx = ceil(log(floor(x))/log(2));

    if (mach64ChipType == MACH64_GT_ID ||
	mach64ChipType == MACH64_GU_ID ||
	mach64ChipType == MACH64_VT_ID ||
	mach64ChipType == MACH64_VU_ID ||
	mach64ChipType == MACH64_GV_ID ||
	mach64ChipType == MACH64_GW_ID ||
	mach64ChipType == MACH64_GZ_ID ||
	OFLG_ISSET(OPTION_FIFO_CONSERV, &mach64InfoRec.options)) {
	rloop = 0;
	fifo_depth = 24;
    } else {
	rloop = 2;
	fifo_depth = 32;
    }

    t = x * fifo_depth;
    bt = ceil(log(floor(t))/log(2));

    p = (bt-5 > bx-3) ? bt-5 : bx-3;

    f = floor((1 << (5 + p))/x);
    if (f > fifo_depth) f = fifo_depth;

    roff = ceil(x*(f-1));

    switch (mach64MemType) {
    case DRAM:
	if (mach64MemorySize > MEM_SIZE_1M) {
	    rloop += 6;
	    n = 1;
	} else {
	    rloop += 8;
	    n = 3;
	}
	break;
    case EDO_DRAM:
    case PSEUDO_EDO:
	if (mach64MemorySize > MEM_SIZE_1M) {
	    rloop += 6;
	    n = 1;
	} else {
	    rloop += 7;
	    n = 2;
	}
	break;
    case SDRAM:
    case SGRAM:
	if (mach64MemorySize > MEM_SIZE_1M) {
	    rloop += 8;
	    n = 1;
	} else {
	    rloop += 9;
	    n = 2;
	}
	break;
    default:
	/* Max values from tables */
	rloop += 9; /* For SDRAM */
	n = 4;      /* For WRAM  */
	break;
    }

    tmp = regr(MEM_CNTL);
    trp  = ((tmp & MEM_TRP)  >>  8) + 1;
    trcd = ((tmp & MEM_TRCD) >> 10) + 1;
    tcrd = ((tmp & MEM_TCRD) >> 12);
    tras = ((tmp & MEM_TRAS) >> 16) + 1;

    pfc = trp + trcd + tcrd;

    rcc = (trp+tras > pfc+n) ? trp+tras : pfc+n;

    ron = (rcc > floor(x)) ? rcc : floor(x);
    ron += (3 * rcc) - 1 + pfc + n;

#if 1
    /*
     * The calculation for max random access cycles (rcc) does not
     * seem to be large enough.  The sample code uses a much larger
     * number, so I am going to use the greater of the two values
     * since that seems to work for all of the cards I have access to.
     */
    switch (mach64MemType) {
    case DRAM:
    case EDO_DRAM:
    case PSEUDO_EDO:
	if (mach64MemorySize > MEM_SIZE_1M)
	    page_size = 9;
	else
	    page_size = 10;
	break;
    case SDRAM:
    case SGRAM:
	if (mach64MemorySize > MEM_SIZE_1M)
	    page_size = 8;
	else
	    page_size = 10;
	break;
    default:
	/* Max values from tables */
	page_size = 10;
	break;
    }

    if (x >= page_size) tmp = 2*page_size + 1 + floor(x);
    else                tmp = 3*page_size;

    if (tmp > ron) ron = tmp;

    switch (mach64MemType) {
    case DRAM:
	break;
    case EDO_DRAM:
	if (mach64MemorySize > MEM_SIZE_1M)
	    if (depth == CRTC_PIX_WIDTH_32BPP)
		if (current_dot_clock > 4200)
		    ron += 1;
	break;
    case PSEUDO_EDO:
	if (mach64MemorySize > MEM_SIZE_1M)
	    roff -= 2;
	break;
    case SDRAM:
    case SGRAM:
	if (mach64ChipType == MACH64_GT_ID ||
	    mach64ChipType == MACH64_GU_ID ||
	    mach64ChipType == MACH64_VT_ID ||
	    mach64ChipType == MACH64_VU_ID ||
	    OFLG_ISSET(OPTION_FIFO_CONSERV, &mach64InfoRec.options)) {
	    if (depth == CRTC_PIX_WIDTH_15BPP || depth == CRTC_PIX_WIDTH_16BPP)
		ron += 1;
	}
	break;
    default:
	break;
    }
#endif

    dsp_on  = ron  << (6 - p);
    dsp_off = roff << (6 - p);
    dsp_xclks_per_qw = x * (1 << (11 - p));

    if (ron+rloop >= roff)
	ErrorF("Warning: Ron = %d, Rloop = %d, Roff = %d\n", ron, rloop, roff);

#ifdef DEBUG
    ErrorF("t = %f, bt = %d, x = %lf, bx = %d, p = %d\n", t, bt, x, bx, p);
    ErrorF("dsp_on  = %d, ron  = %d, rloop = %d\n", dsp_on, ron, rloop);
    ErrorF("dsp_off = %d, roff = %d\n", dsp_off, roff);
    ErrorF("dsp_xclks_per_qw = %d\n", dsp_xclks_per_qw);
    ErrorF("mach64XCLK = %d, ", mach64XCLK);
    ErrorF("dot_clock = %.3lf, ", current_dot_clock);
    ErrorF("bpp = %d\n", bpp);
    ErrorF("trp = %d, ", trp);
    ErrorF("trcd = %d, ", trcd);
    ErrorF("tcrd = %d, ", tcrd);
    ErrorF("tras = %d\n", tras);
    ErrorF("pfc = %d, ", pfc);
    ErrorF("rcc = %d\n", rcc);
#endif

    regw(DSP_ON_OFF,
	 ((dsp_on << 16) & DSP_ON) |
	 (dsp_off & DSP_OFF));
    regw(DSP_CONFIG,
	 ((p << 20) & DSP_PRECISION) |
	 ((rloop << 16) & DSP_LOOP_LATENCY) |
	 (dsp_xclks_per_qw & DSP_XCLKS_PER_QW));
}

/*
 * mach64SetCRTCRegs --
 * Initializes the Mach64 for the currently selected CRTC parameters.  */
void mach64SetCRTCRegs(crtcRegs)
     mach64CRTCRegPtr crtcRegs;
{
    int crtcGenCntl;
    unsigned long depth = crtcRegs->color_depth;
    unsigned long lcd_gen_ctrl;
    unsigned char CTD_sharedCntl;

    /* Now initialize the display controller part of the Mach64.
     * The CRTC registers are passed in from the calling routine.
     */

    WaitIdleEmpty();
    crtcGenCntl = regr(CRTC_GEN_CNTL);
    regw(CRTC_GEN_CNTL, crtcGenCntl & ~(CRTC_EXT_EN | CRTC_LOCK_REGS));

    /* Check to see if we need to program the clock chip */
    if (mach64ClockType != 0 && mach64Ramdac != DAC_IBMRGB514 &&
	(((mach64Ramdac == DAC_ATI68860_B || mach64Ramdac == DAC_ATI68860_C) &&
	 crtcRegs->clock_cntl & 0x30) ||
	 (crtcRegs->clock_cntl == mach64CXClk) ||
	 (OFLG_ISSET(CLOCK_OPTION_PROGRAMABLE, &mach64InfoRec.clockOptions) &&
	  !OFLG_ISSET(OPTION_NO_PROGRAM_CLOCKS, &mach64InfoRec.options)))) {
	if (mach64LCDPanelID >= 0)
	    mach64ProgramClk(mach64CXClk, mach64LCDClock);
	else if (crtcRegs->clock_cntl & CLOCK_DIV2)
	    mach64ProgramClk(mach64CXClk, crtcRegs->dot_clock >> 1);
	else
	    mach64ProgramClk(mach64CXClk, crtcRegs->dot_clock);
	crtcRegs->clock_cntl = mach64CXClk;
    }

    if (mach64RamdacSubType == DAC_IBMRGB514) {
#if 0
	if (mach64VRAMMemClk > 5017 &&
	    crtcRegs->color_depth == CRTC_PIX_WIDTH_8BPP &&
	    crtcRegs->dot_clock <= 2800)
	    mach64ProgramICS2595(mach64MemClk, 4700);
	else
	    mach64ProgramICS2595(mach64MemClk, mach64VRAMMemClk);
#endif

	mach64ProgramClkRGB514(mach64CXClk, crtcRegs->dot_clock);
	crtcRegs->clock_cntl = mach64CXClk;
    }

    /* Set the DSP registers on the VT-B and GT-B */
    if (mach64HasDSP)
	mach64SetDSPRegs(crtcRegs->color_depth);

    if (mach64LCDPanelID >= 0) {
	if (mach64ChipType == MACH64_LG_ID) {
	    /* Update non-shadow registers first */
	    lcd_gen_ctrl = regr(LCD_GEN_CTRL);
	    regw(LCD_GEN_CTRL, lcd_gen_ctrl &
		~(DISABLE_PCLK_RESET | CRTC_RW_SELECT | SHADOW_EN |
		  SHADOW_RW_EN));

	    /* Temporarily disable stretching */
	    regw(HORZ_STRETCHING, crtcRegs->horz_stretching &
		~(HORZ_STRETCH_MODE | HORZ_STRETCH_EN));
	    regw(VERT_STRETCHING, crtcRegs->vert_stretching &
		~(VERT_STRETCH_RATIO1 | VERT_STRETCH_RATIO2 |
		  VERT_STRETCH_USE0 | VERT_STRETCH_EN));
	} else {
	    /* Update non-shadow registers first */
	    outb(ioLCD_INDEX, LCD_GEN_CNTL);
	    lcd_gen_ctrl = inl(ioLCD_DATA);
	    outl(ioLCD_DATA, lcd_gen_ctrl &
		~(DISABLE_PCLK_RESET | CRTC_RW_SELECT | SHADOW_EN |
		  SHADOW_RW_EN));

	    /* Temporarily disable stretching */
	    outb(ioLCD_INDEX, LCD_HORZ_STRETCHING);
	    outl(ioLCD_DATA, crtcRegs->horz_stretching &
		~(HORZ_STRETCH_MODE | HORZ_STRETCH_EN));
	    outb(ioLCD_INDEX, LCD_VERT_STRETCHING);
	    outl(ioLCD_DATA, crtcRegs->vert_stretching &
		~(VERT_STRETCH_RATIO1 | VERT_STRETCH_RATIO2 |
		  VERT_STRETCH_USE0 | VERT_STRETCH_EN));
	}
    }

    /* Horizontal CRTC registers */
    regw(CRTC_H_TOTAL_DISP,    crtcRegs->h_total_disp);
    regw(CRTC_H_SYNC_STRT_WID, crtcRegs->h_sync_strt_wid);

    /* Vertical CRTC registers */
    regw(CRTC_V_TOTAL_DISP,    crtcRegs->v_total_disp);
    regw(CRTC_V_SYNC_STRT_WID, crtcRegs->v_sync_strt_wid);

    if (mach64LCDPanelID >= 0) {
	/* Switch to shadow registers */
	if (mach64ChipType == MACH64_LG_ID) {
	    regw(LCD_GEN_CTRL, (lcd_gen_ctrl &
		~(DISABLE_PCLK_RESET | CRTC_RW_SELECT)) |
		(SHADOW_EN | SHADOW_RW_EN));
	} else {
	    outb(ioLCD_INDEX, LCD_GEN_CNTL);
	    outl(ioLCD_DATA, (lcd_gen_ctrl &
		~(DISABLE_PCLK_RESET | CRTC_RW_SELECT)) |
		(SHADOW_EN | SHADOW_RW_EN));
	}

	/* Set shadow registers */
	regw(CRTC_H_TOTAL_DISP,    crtcRegs->h_total_disp);
	regw(CRTC_H_SYNC_STRT_WID, crtcRegs->h_sync_strt_wid);
	regw(CRTC_V_TOTAL_DISP,    crtcRegs->v_total_disp);
	regw(CRTC_V_SYNC_STRT_WID, crtcRegs->v_sync_strt_wid);

	/* Restore CRTC selection, shadow state and stretching */
	if (mach64ChipType == MACH64_LG_ID) {
	    regw(LCD_GEN_CTRL, lcd_gen_ctrl);
	    regw(HORZ_STRETCHING, crtcRegs->horz_stretching);
	    regw(VERT_STRETCHING, crtcRegs->vert_stretching);
	} else {
	    outb(ioLCD_INDEX, LCD_GEN_CNTL);
	    outl(ioLCD_DATA, lcd_gen_ctrl);
	    outb(ioLCD_INDEX, LCD_HORZ_STRETCHING);
	    outl(ioLCD_DATA, crtcRegs->horz_stretching);
	    outb(ioLCD_INDEX, LCD_VERT_STRETCHING);
	    outl(ioLCD_DATA, crtcRegs->vert_stretching);
	    outb(ioLCD_INDEX, LCD_EXT_VERT_STRETCH);
	    outl(ioLCD_DATA, crtcRegs->ext_vert_stretch);
	}
    }

    /* Clock select register */
    regw(CLOCK_CNTL, crtcRegs->clock_cntl | CLOCK_STROBE);

    /* Zero overscan register to insure proper color */
    regw(OVR_CLR, 0);
    regw(OVR_WID_LEFT_RIGHT, 0);
    regw(OVR_WID_TOP_BOTTOM, 0);

    /* Set the width of the display */
    if (isMuxMode)
	regw(CRTC_OFF_PITCH, (mach64VirtX >> 4) << 22);
    else
	regw(CRTC_OFF_PITCH, (mach64VirtX >> 3) << 22);
    regw(DST_OFF_PITCH, (mach64VirtX >> 3) << 22);
    regw(SRC_OFF_PITCH, (mach64VirtX >> 3) << 22);

    if (isMuxMode) {
	if (depth == CRTC_PIX_WIDTH_8BPP)
	    depth = CRTC_PIX_WIDTH_16BPP;
	else
	    depth = CRTC_PIX_WIDTH_32BPP;
    }

    if (mach64ChipType == MACH64_CT_ID && mach64ChipRev == 0x0a) { /* CT-D only */
	CTD_sharedCntl = regrb(SHARED_CNTL+3) & ~(CTD_FIFO5 >> 24);
	regwb(SHARED_CNTL+3,
	      CTD_sharedCntl | ((crtcRegs->fifo_v1 & 0x10) >> 4));
    }

    /* Display control register -- this one turns on the display */
    regw(CRTC_GEN_CNTL,
	 (crtcGenCntl & 0xff0000ff &
	  ~(CRTC_PIX_BY_2_EN | CRTC_DBL_SCAN_EN | CRTC_INTERLACE_EN |
            CRTC_HSYNC_DIS | CRTC_VSYNC_DIS)) |
	 depth |
	 (crtcRegs->crtc_gen_cntl & ~CRTC_PIX_BY_2_EN) |
	 ((crtcRegs->fifo_v1 & 0x0f) << 16) |
	 CRTC_EXT_DISP_EN | CRTC_EXT_EN);

    /* Set the DAC for the currect mode */
    mach64SetRamdac(crtcRegs->color_depth, TRUE, crtcRegs->dot_clock);

    WaitIdleEmpty();
}

/*
 * mach64SaveLUT --
 *	 Saves the LUT in lut.
 */
void mach64SaveLUT(lut)
     LUTENTRY *lut;
{
    int i;

    /* set DAC read index */
    outb(ioDAC_REGS+3, 0);

    for (i = 0; i < 256; i++) {
        lut[i].r = inb(ioDAC_REGS+1);
        lut[i].g = inb(ioDAC_REGS+1);
        lut[i].b = inb(ioDAC_REGS+1);
    }
}

/*
 * mach64RestoreLUT -- 
 *	Restores the LUT in lut.
 */
void mach64RestoreLUT(lut)
     LUTENTRY *lut;
{
    int i;

    /* set DAC write index */
    outb(ioDAC_REGS, 0);

    for (i = 0; i < 256; i++) {
        outb(ioDAC_REGS+1, lut[i].r);
        outb(ioDAC_REGS+1, lut[i].g);
        outb(ioDAC_REGS+1, lut[i].b);
    }
}

/*
 * mach64InitLUT --
 *	Loads the Look-Up Table with all black.
 *	Assumes 8-bit board is in use.
 */
void mach64InitLUT()
{
    int i;

    mach64SaveLUT(oldlut);
    LUTInited = TRUE;

    /* set DAC write index */
    outb(ioDAC_REGS, 0);

    /* Load the LUT entries */
    for (i = 0; i < 256; i++) {
        outb(ioDAC_REGS+1, 0);
        outb(ioDAC_REGS+1, 0);
        outb(ioDAC_REGS+1, 0);
    }
}

int
mach64GetCTClock(i)
     int i;
{
    int M = mach64RefDivider;
    int R = mach64RefFreq;
    int N, P, postDiv;

    outb(ioCLOCK_CNTL + 1, (VCLK0_FB_DIV + i) << 2);
    N = inb(ioCLOCK_CNTL + 2);
    outb(ioCLOCK_CNTL + 1, VCLK_POST_DIV << 2);
    postDiv = (inb(ioCLOCK_CNTL + 2) >> (2 * i)) & 0x03;
    if (mach64HasDSP) {
	outb(ioCLOCK_CNTL + 1, PLL_XCLK_CNTL << 2);
	if ((inb(ioCLOCK_CNTL + 2) >> (4 + i)) & 0x01) {
	    switch (postDiv) {
	    case 0: P = 3;  break;
	    case 1: P = 2;  break; /* Unknown */
	    case 2: P = 6;  break;
	    case 3: P = 12; break;
	    }
	} else {
	    P = 1 << postDiv;
	}
    } else {
	P = 1 << postDiv;
    }
    return (2 * R * N)/(M * P);
}
    
/*
 * mach64ResetEngine --
 *	Resets the GUI engine and clears any FIFO errors.
 */
void mach64ResetEngine()
{
    int temp;

    /* Ensure engine is not locked up by clearing any FIFO errors */
    regw(BUS_CNTL, regr(BUS_CNTL) | BUS_HOST_ERR_ACK | BUS_FIFO_ERR_ACK);

    /* Reset engine */
    temp = regr(GEN_TEST_CNTL);
    regw(GEN_TEST_CNTL, temp & ~GUI_ENGINE_ENABLE);

    if (!mach64IntegratedController) {
	/* Block write mode _should_ be correctly initialized by the BIOS
	 * at boot time, but this may not be the case.  If it is not, then
	 * allow the user to explicitly turn on or turn off block write
	 * mode via the "block_write" and "no_block_write" option in their
	 * XF86Config file.
	 */
	if (OFLG_ISSET(OPTION_BLOCK_WRITE, &mach64InfoRec.options)) {
	    temp |= BLOCK_WRITE_ENABLE;
	} else if (OFLG_ISSET(OPTION_NO_BLOCK_WRITE, &mach64InfoRec.options)) {
	    temp &= ~BLOCK_WRITE_ENABLE;
	}
    }

    regw(GEN_TEST_CNTL, temp | GUI_ENGINE_ENABLE);

    /* On RagePro chips we can enable auto block write and auto fast fill
     * modes if block write mode is not initialized by the BIOS.
     */
    if (mach64IntegratedController && mach64HasBlockWrite) {
	temp = regr(HW_DEBUG);
	if (OFLG_ISSET(OPTION_BLOCK_WRITE, &mach64InfoRec.options)) {
	    temp &= ~(AUTO_FF_DIS | AUTO_BLKWRT_DIS);
	} else if (OFLG_ISSET(OPTION_NO_BLOCK_WRITE, &mach64InfoRec.options)) {
	    temp |= AUTO_FF_DIS | AUTO_BLKWRT_DIS;
	}
	regw(HW_DEBUG, temp);
    }

    WaitIdleEmpty();
}

/*
 * mach64InitEnvironment --
 *	Initializes the Mach64's drawing environment and clears the display.
 */
void mach64InitEnvironment()
{
    mach64ResetEngine();

    WaitIdleEmpty();

    /* Set current color depth (8bpp) */
    switch (mach64InfoRec.bitsPerPixel) {
    case 8:
	regw(DP_PIX_WIDTH, HOST_8BPP | SRC_8BPP | DST_8BPP);
	regw(DP_CHAIN_MASK, DP_CHAIN_8BPP);
	break;
    case 16:
	if (mach64WeightMask == RGB16_555) {
	    regw(DP_PIX_WIDTH, HOST_16BPP | SRC_15BPP | DST_15BPP);
	    regw(DP_CHAIN_MASK, DP_CHAIN_15BPP);
	} else {
	    regw(DP_PIX_WIDTH, HOST_16BPP | SRC_16BPP | DST_16BPP);
	    regw(DP_CHAIN_MASK, DP_CHAIN_16BPP);
	}
	break;
    case 32:
	regw(DP_PIX_WIDTH, HOST_32BPP | SRC_32BPP | DST_32BPP);
	regw(DP_CHAIN_MASK, DP_CHAIN_32BPP);
	break;
    }

    regw(CONTEXT_MASK, 0xffffffff);

    regw(DST_Y_X, 0);
    regw(DST_HEIGHT, 0);
    regw(DST_BRES_ERR, 0);
    regw(DST_BRES_INC, 0);
    regw(DST_BRES_DEC, 0);
    regw(DST_CNTL, (DST_X_LEFT_TO_RIGHT | DST_Y_TOP_TO_BOTTOM));

    regw(SRC_Y_X, 0);
    regw(SRC_HEIGHT1_WIDTH1, 0);
    regw(SRC_Y_X_START, 0);
    regw(SRC_HEIGHT2_WIDTH2, 0);
    regw(SRC_CNTL, 0);

    WaitQueue(7);
    regw(HOST_CNTL, regr(HOST_CNTL) & ~HOST_BYTE_ALIGN);
    regw(PAT_REG0, 0);
    regw(PAT_REG1, 0);
    regw(PAT_CNTL, 0);

    regw(SC_LEFT_RIGHT, ((mach64MaxX << 16) | 0 ));
    regw(SC_TOP_BOTTOM, ((mach64MaxY << 16) | 0 ));

    WaitQueue(9);
    regw(DP_BKGD_CLR, 0);
    regw(DP_FRGD_CLR, 1);
    regw(DP_WRITE_MASK, 0xffffffff);
    regw(DP_MIX, (MIX_SRC << 16) | MIX_DST);
    regw(DP_SRC, FRGD_SRC_FRGD_CLR);

    regw(CLR_CMP_CLR, 0);
    regw(CLR_CMP_MASK, 0xffffffff);
    regw(CLR_CMP_CNTL, 0);

    regw(GUI_TRAJ_CNTL, DST_X_LEFT_TO_RIGHT | DST_Y_TOP_TO_BOTTOM);
    WaitIdleEmpty();
}

/*
 * mach64InitAperture --
 *	Initialize the aperture for the Mach64.
 */
void mach64InitAperture(screen_idx)
    int screen_idx;
{
    int i;
    unsigned long apaddr;
    unsigned long regpage, regoffset;
    long memsize, regsize;

    if (!mach64VideoMem) {
	old_CONFIG_CNTL = inw(ioCONFIG_CNTL);
    }

    apaddr = mach64ApertureAddr;

    if (mach64RegisterAddr) { /* Use Auxilliary Register Aperture */
	mach64MemRegOffset = 0x400;

	regpage = mach64RegisterAddr;
	regoffset = mach64MemRegOffset;
	regsize = 0x1000;
    } else {
	if ((mach64BusType == PCI) && (mach64IntegratedController)) {
	    mach64MemRegOffset = 0x7ffc00;
	} else if (mach64ApertureSize == MEM_SIZE_4M) {
	    mach64MemRegOffset = 0x3ffc00;
	    outw(ioCONFIG_CNTL, ((apaddr/(4*1024*1024)) << 4) | 1);
	} else {
	    mach64MemRegOffset = 0x7ffc00;
	    outw(ioCONFIG_CNTL, ((apaddr/(4*1024*1024)) << 4) | 2);
	}

	regpage = mach64MemRegOffset & XF_PAGE_MASK;
	regoffset = mach64MemRegOffset - regpage;
	regpage += apaddr;
	regsize = PAGE_SIZE;
    }

    switch(mach64MemorySize) {
    case MEM_SIZE_512K:
	memsize = 512 * 1024;
	break;
    case MEM_SIZE_1M:
	memsize = 1024 * 1024;
	break;
    case MEM_SIZE_2M:
	memsize = 2 * 1024 * 1024;
	break;
    case MEM_SIZE_4M:
	memsize = 4 * 1024 * 1024;
	break;
    case MEM_SIZE_6M:
	memsize = 6 * 1024 * 1024;
	break;
    case MEM_SIZE_8M:
	memsize = 8 * 1024 * 1024;
	break;
    case MEM_SIZE_16M:
	memsize = 16 * 1024 * 1024;
	break;
    }

    if (!mach64MemRegMap) {
	mach64MemRegMap = xf86MapVidMem(screen_idx, EXTENDED_REGION,
					(pointer)(regpage), regsize);
	mach64MemReg = (pointer)((unsigned long)mach64MemRegMap + regoffset);
    }

    if (!mach64VideoMem) {
	mach64VideoMem = xf86MapVidMem(screen_idx, LINEAR_REGION, 
				       (pointer)apaddr, memsize);
#ifdef XFreeXDGA
	mach64InfoRec.physBase = apaddr;
	mach64InfoRec.physSize = mach64InfoRec.videoRam * 1024;
#endif
    }
}

/* 
 * mach64ProgramATI68860 --
 * 
 */
int mach64ProgramATI68860(colorDepth, AccelMode)
    int colorDepth;
    int AccelMode;
{
    int gmr, dsra, temp, mask;

    switch (colorDepth) {
    case CRTC_PIX_WIDTH_8BPP:
	gmr = 0x83;
	dsra = 0x60 | (mach64DAC8Bit ? 0x00 : 0x01);
	break;
    case CRTC_PIX_WIDTH_15BPP:
	gmr = 0xA0;
	dsra = 0x60;
	break;
    case CRTC_PIX_WIDTH_16BPP:
	gmr = 0xA1;
	dsra = 0x60;
	break;
    case CRTC_PIX_WIDTH_24BPP:
	gmr = 0xC0;
	dsra = 0x60;
	break;
    case CRTC_PIX_WIDTH_32BPP:
	gmr = 0xE3;
	dsra = 0x60;
	break;
    }

    if (!AccelMode) {
	gmr = 0x80;
	dsra = 0x61;
    }

    temp = inb(ioDAC_CNTL);
    outb(ioDAC_CNTL, (temp & ~DAC_EXT_SEL_RS2) | DAC_EXT_SEL_RS3);

    outb(ioDAC_REGS+2, 0x1d);
    outb(ioDAC_REGS+3, gmr);
    outb(ioDAC_REGS, 0x02);

    temp = inb(ioDAC_CNTL);
    outb(ioDAC_CNTL, temp | DAC_EXT_SEL_RS2 | DAC_EXT_SEL_RS3);

    if (mach64MemorySize < MEM_SIZE_1M)
	mask = 0x04;
    else if (mach64MemorySize == MEM_SIZE_1M)
	mask = 0x08;
    else
	mask = 0x0c;

    /* The following assumes that the BIOS has correctly set R7 of the
     * Device Setup Register A at boot time.
     */
#define A860_DELAY_L	0x80

    temp = inb(ioDAC_REGS);
    outb(ioDAC_REGS, (dsra | mask) | (temp & A860_DELAY_L));
    temp = inb(ioDAC_CNTL);
    outb(ioDAC_CNTL, (temp & ~(DAC_EXT_SEL_RS2 | DAC_EXT_SEL_RS3)));

    return FALSE;
}

/* 
 * mach64ProgramATI68875 --
 * 
 */
int mach64ProgramATI68875(colorDepth, dotClock)
    int colorDepth;
    int dotClock;
{
    int ocsr, mcr, icsr, crtcPixWidth, clockCntl, muxMode, temp;

    muxMode = FALSE;

    switch (colorDepth) {
    case CRTC_PIX_WIDTH_8BPP:
	if (dotClock > 8000) {
	    ocsr = 0x09;
	    mcr = 0x1d;
	    icsr = 0x01;
	    muxMode = TRUE;
	} else {
	    ocsr = 0x30;
	    mcr = 0x2d;
	    icsr = 0x00;
	}
	break;
    case CRTC_PIX_WIDTH_15BPP:
	ocsr = 0x00;
	mcr = 0x0d;
	icsr = 0x01;
	break;
    case CRTC_PIX_WIDTH_16BPP:
	ocsr = 0x00;
	mcr = 0x0d;
	icsr = 0x01;
	break;
    case CRTC_PIX_WIDTH_24BPP:
	ocsr = 0x00;
	mcr = 0x0d;
	icsr = 0x01;
	break;
    case CRTC_PIX_WIDTH_32BPP:
	ocsr = 0x00;
	mcr = 0x0d;
	icsr = 0x01;
	break;
    }

    crtcPixWidth = regrb(CRTC_GEN_CNTL+1);
    regwb(CRTC_GEN_CNTL+1, (CRTC_PIX_WIDTH_8BPP >> 8));

    clockCntl = regrb(CLOCK_CNTL);
    regwb(CLOCK_CNTL, (clockCntl & ~CLOCK_DIV) | CLOCK_DIV4 | CLOCK_STROBE);

    temp = inb(ioDAC_CNTL);
    outb(ioDAC_CNTL, (temp & ~DAC_EXT_SEL_RS2) | DAC_EXT_SEL_RS3);

    outb(ioDAC_REGS+2, ocsr);
    outb(ioDAC_REGS+3, mcr);
    outb(ioDAC_REGS+1, icsr);
    
    temp = inb(ioDAC_CNTL);
    outb(ioDAC_CNTL, (temp & ~(DAC_EXT_SEL_RS2 | DAC_EXT_SEL_RS3)));

    if (colorDepth == CRTC_PIX_WIDTH_8BPP)
	outb(ioDAC_REGS+2, 0xff);
    else
	outb(ioDAC_REGS+2, 0x00);

    temp = (inb(ioDAC_CNTL+1) &
	    ~((DAC_8BIT_EN | DAC_PIX_DLY_MASK | DAC_BLANK_ADJ_MASK) >> 8) |
	    DAC_PIX_DLY_0NS | DAC_BLANK_ADJ_2);
    if (mach64DAC8Bit || (colorDepth > CRTC_PIX_WIDTH_8BPP))
	temp |= DAC_8BIT_EN;
    outb(ioDAC_CNTL+1, temp);

    regwb(CLOCK_CNTL, clockCntl | CLOCK_STROBE);
    regwb(CRTC_GEN_CNTL+1, crtcPixWidth);

    if (colorDepth < CRTC_PIX_WIDTH_15BPP ||
	colorDepth > CRTC_PIX_WIDTH_16BPP ||
	dotClock <= 7600)
	outb(ioDAC_CNTL, 0);
    else {
	temp = (inb(ioDAC_CNTL+1) &
		~((DAC_PIX_DLY_MASK | DAC_BLANK_ADJ_MASK) >> 8));
	outb(ioDAC_CNTL+1, temp | DAC_PIX_DLY_4NS | DAC_BLANK_ADJ_2);
    }

    return muxMode;
}

/* 
 * mach64ProgramBT481 --
 * 
 */
int mach64ProgramBT481(colorDepth)
    int colorDepth;
{
    int progByte, temp;

    switch (colorDepth) {
    case CRTC_PIX_WIDTH_8BPP:
	if (mach64DAC8Bit)
	    progByte = 0x02;
	else
	    progByte = 0x00;
	break;
    case CRTC_PIX_WIDTH_15BPP:
	progByte = 0xa0;
	break;
    case CRTC_PIX_WIDTH_16BPP:
	progByte = 0xe0;
	break;
    case CRTC_PIX_WIDTH_24BPP:
	progByte = 0xf0;
	break;
    case CRTC_PIX_WIDTH_32BPP:
	progByte = 0x00;
	break;
    }

    temp = inb(ioDAC_CNTL);
    outb(ioDAC_CNTL, (temp & ~DAC_EXT_SEL_RS2) | DAC_EXT_SEL_RS3);

    outb(ioDAC_REGS+2, progByte);

    temp = inb(ioDAC_CNTL);
    outb(ioDAC_CNTL, (temp & ~(DAC_EXT_SEL_RS2 | DAC_EXT_SEL_RS3)));

    return FALSE;
}

/* 
 * mach64ProgramSTG170X --
 * 
 */
int mach64ProgramSTG170X(pixMode, vgaMode, dotClock)
    int pixMode;
    int vgaMode;
    int dotClock;
{
    int muxMode = FALSE;
    int pcr;

    mach64DACRead4();
    pcr = (inb(ioDAC_REGS+2) & 0x06) | 0x10 | vgaMode;
    (void)inb(ioDAC_REGS);

    if (pixMode == 0xff) {
	mach64DACRead4();
	outb(ioDAC_REGS+2, pcr | 0x01);
	(void)inb(ioDAC_REGS);
	return muxMode;
    }

    if (mach64DAC8Bit)
	pcr |= 0x02;

    mach64DACRead4();
    outb(ioDAC_REGS+2, pcr);
    (void)inb(ioDAC_REGS);

    mach64DACRead4();

    (void)inb(ioDAC_REGS+2);
    outb(ioDAC_REGS+2, 0x03);
    outb(ioDAC_REGS+2, 0x00);
    outb(ioDAC_REGS+2, pixMode);
    outb(ioDAC_REGS+2, pixMode);

    if (pixMode == 5) {
	outb(ioDAC_REGS+2, 0x02);
    } else if (pixMode == 9) {
	if (dotClock < 1600)
	    outb(ioDAC_REGS+2, 0x00);
	else if (dotClock < 3200)
	    outb(ioDAC_REGS+2, 0x01);
	else if (dotClock < 6400)
	    outb(ioDAC_REGS+2, 0x02);
	else
	    outb(ioDAC_REGS+2, 0x03);
    }
    usleep(500);

    (void)inb(ioDAC_REGS);

    return muxMode;
}

/* 
 * mach64ProgramSTG1700 --
 * 
 */
int mach64ProgramSTG1700(colorDepth, dotClock)
    int colorDepth;
    int dotClock;
{
    int muxMode = FALSE;
    int pixMode, vgaMode, temp;

    switch (colorDepth) {
    case CRTC_PIX_WIDTH_8BPP:
	if (dotClock > 11000) {
	    pixMode = 0x05;
	    vgaMode = 0x08;
	    muxMode = TRUE;
	} else {
	    pixMode = 0x00;
	    vgaMode = 0x00;
	}
	break;
    case CRTC_PIX_WIDTH_15BPP:
	pixMode = 0x02;
	vgaMode = 0xa8;
	break;
    case CRTC_PIX_WIDTH_16BPP:
	pixMode = 0x03;
	vgaMode = 0xc8;
	break;
    case CRTC_PIX_WIDTH_24BPP:
	/* Can the STG1700 handle this video mode? */
	pixMode = 0x09;
	vgaMode = 0xe8;
	break;
    case CRTC_PIX_WIDTH_32BPP:
	/* Can the STG1700 handle this video mode? */
	pixMode = 0x09;
	vgaMode = 0xe8;
	break;
    }

    mach64ProgramSTG170X(pixMode, vgaMode, dotClock);

    return muxMode;
}

/* 
 * mach64ProgramSTG1702 --
 * 
 */
int mach64ProgramSTG1702(colorDepth, dotClock)
    int colorDepth;
    int dotClock;
{
    int muxMode = FALSE;
    int pixMode, vgaMode, temp;

    switch (colorDepth) {
    case CRTC_PIX_WIDTH_8BPP:
	if (dotClock > 11000) {
	    pixMode = 0x05;
	    vgaMode = 0x08;
	    muxMode = TRUE;
	} else {
	    pixMode = 0x00;
	    vgaMode = 0x00;
	}
	break;
    case CRTC_PIX_WIDTH_15BPP:
	pixMode = 0x02;
	vgaMode = 0xa8;
	break;
    case CRTC_PIX_WIDTH_16BPP:
	pixMode = 0x03;
	vgaMode = 0xc8;
	break;
    case CRTC_PIX_WIDTH_24BPP:
	pixMode = 0x09;
	vgaMode = 0xe8;
	break;
    case CRTC_PIX_WIDTH_32BPP:
	pixMode = 0x09;
	vgaMode = 0xe8;
	break;
    }

    mach64ProgramSTG170X(pixMode, vgaMode, dotClock);

    return muxMode;
}

/* 
 * mach64ProgramATT21C498 --
 * 
 */
int mach64ProgramATT21C498(colorDepth, dotClock)
    int colorDepth;
    int dotClock;
{
    int muxMode = FALSE;
    int DACMask;

    switch (colorDepth) {
    case CRTC_PIX_WIDTH_8BPP:
	if (dotClock > 8000) {
	    DACMask = 0x24;
	    muxMode = TRUE;
	} else {
	    DACMask = 0x04;
	}
	break;
    case CRTC_PIX_WIDTH_15BPP:
	DACMask = 0x16;
	break;
    case CRTC_PIX_WIDTH_16BPP:
	DACMask = 0x36;
	break;
    case CRTC_PIX_WIDTH_24BPP:
	DACMask = 0xe6;
	break;
    case CRTC_PIX_WIDTH_32BPP:
	DACMask = 0xe6;
	break;
    }

    if (mach64DAC8Bit)
	DACMask |= 0x02;

    mach64DACRead4();
    outb(ioDAC_REGS+2, DACMask);

    return muxMode;
}

/* 
 * mach64ProgramCH8398 --
 * 
 */
int mach64ProgramCH8398(colorDepth, dotClock)
    int colorDepth;
    int dotClock;
{
    int muxMode = FALSE;
    int controlReg, temp;

    switch (colorDepth) {
    case CRTC_PIX_WIDTH_8BPP:
	if (dotClock > 8000) {
	    controlReg = 0x24;
	    muxMode = TRUE;
	} else {
	    controlReg = 0x04;
	}
	break;
    case CRTC_PIX_WIDTH_15BPP:
	controlReg = 0x14;
	break;
    case CRTC_PIX_WIDTH_16BPP:
	controlReg = 0x34;
	break;
    case CRTC_PIX_WIDTH_24BPP:
	controlReg = 0xb4;
	break;
    case CRTC_PIX_WIDTH_32BPP:
	controlReg = 0xb4;
	break;
    }

    temp = inb(ioDAC_CNTL);
    outb(ioDAC_CNTL, temp | DAC_EXT_SEL_RS2 | DAC_EXT_SEL_RS3);

    outb(ioDAC_REGS+2, controlReg);

    temp = inb(ioDAC_CNTL);
    outb(ioDAC_CNTL, (temp & ~DAC_EXT_SEL_RS2) | DAC_EXT_SEL_RS3);

    return muxMode;
}

/* 
 * mach64ProgramInternal --
 * 
 */
int mach64ProgramInternal(colorDepth, dotClock)
    int colorDepth;
    int dotClock;
{
    int muxMode = FALSE;
    int temp;
    extern unsigned char xf86rGammaMap[], xf86gGammaMap[], xf86bGammaMap[];

    switch (colorDepth) {
    case CRTC_PIX_WIDTH_8BPP:
	temp = inb(ioDAC_CNTL + 1);
	if (mach64DAC8Bit)
	    outb(ioDAC_CNTL + 1, temp | 0x01);
	else
	    outb(ioDAC_CNTL + 1, temp & ~0x01);
	break;
    case CRTC_PIX_WIDTH_15BPP:
    case CRTC_PIX_WIDTH_16BPP:
    case CRTC_PIX_WIDTH_24BPP:
    case CRTC_PIX_WIDTH_32BPP:
	temp = inb(ioDAC_CNTL + 1);
	outb(ioDAC_CNTL + 1, temp | 0x01);
	outb(ioDAC_REGS + 2, 0xFF);
        outb(ioDAC_REGS, 0);
	for (temp = 0; temp < 256; temp++) {
	    outb(ioDAC_REGS + 1, xf86rGammaMap[temp]);
	    outb(ioDAC_REGS + 1, xf86gGammaMap[temp]);
	    outb(ioDAC_REGS + 1, xf86bGammaMap[temp]);
	}
	break;
    }

    return muxMode;
}

/* 
 * mach64ProgramIBMRGB514 --
 * 
 */
int mach64ProgramIBMRGB514(colorDepth, AccelMode)
    int colorDepth;
    int AccelMode;
{
    char old_crtc_ext_disp;
    int muxMode = FALSE;
    int temp;

    temp = inb(ioGEN_TEST_CNTL);
    outb(ioGEN_TEST_CNTL, temp | GEN_OVR_OUTPUT_EN);

    mach64P_RGB514Index(0x90, 0x00);

    if (colorDepth == CRTC_PIX_WIDTH_24BPP ||
	colorDepth == CRTC_PIX_WIDTH_32BPP)
	mach64P_RGB514Index(0x04, 0x02);
    else
	mach64P_RGB514Index(0x04, 0x00);

    mach64P_RGB514Index(0x05, 0x00);

    if (!AccelMode) {
	mach64P_RGB514Index(0x90, 0x03);
	temp = mach64R_RGB514Index(0x30);
	temp &= ~0x03;
	mach64P_RGB514Index(0x30, temp);
	mach64P_RGB514Index(0x71, 0x40);
	mach64P_RGB514Index(0x02, 0x00);
	mach64P_RGB514Index(0x71, 0x00);
	mach64P_RGB514Index(0x0a, 0x03);
	return FALSE;
    }

    mach64P_RGB514Index(0x02, 0x01);

    switch (colorDepth) {
    case CRTC_PIX_WIDTH_8BPP:
	if (mach64DAC8Bit)
	    mach64P_RGB514Index(0x71, 0x45);
	else
	    mach64P_RGB514Index(0x71, 0x41);
	mach64P_RGB514Index(0x0a, 0x03);
	break;
    case CRTC_PIX_WIDTH_15BPP:
	mach64P_RGB514Index(0x71, 0x45);
	mach64P_RGB514Index(0x0a, 0x04);
	mach64P_RGB514Index(0x0c, 0xc0);
	break;
    case CRTC_PIX_WIDTH_16BPP:
	mach64P_RGB514Index(0x71, 0x45);
	mach64P_RGB514Index(0x0a, 0x04);
	mach64P_RGB514Index(0x0c, 0xc2);
	break;
    case CRTC_PIX_WIDTH_24BPP:
    case CRTC_PIX_WIDTH_32BPP:
	mach64P_RGB514Index(0x71, 0x45);
	mach64P_RGB514Index(0x0a, 0x06);
	mach64P_RGB514Index(0x0e, 0x03);
	break;
    }

    temp = inb(ioCRTC_GEN_CNTL);
    if (temp & CRTC_INTERLACE_EN) {
	temp = mach64R_RGB514Index(0x71);
	temp |= 0x20;
	mach64P_RGB514Index(0x71, temp);
    }

    return muxMode;
}

/* 
 * mach64SetRamdac --
 * 
 */
void mach64SetRamdac(colorDepth, AccelMode, dotClock)
    int colorDepth;
    int AccelMode;
    int dotClock;
{
    int temp, muxMode;

    muxMode = FALSE;

    WaitIdleEmpty();
    if (AccelMode)
	regwb(CRTC_GEN_CNTL+3, ((CRTC_EXT_DISP_EN | CRTC_EXT_EN) >> 24));
    else
	regwb(CRTC_GEN_CNTL+3, 0);

    temp = inb(ioCRTC_GEN_CNTL+3);
    outb(ioCRTC_GEN_CNTL+3, temp | (CRTC_EXT_DISP_EN >> 24));

    switch(mach64RamdacSubType) {
    case DAC_ATI68860_B:
    case DAC_ATI68860_C:
	muxMode = mach64ProgramATI68860(colorDepth, AccelMode);
	break;
    case DAC_ATI68875:
	muxMode = mach64ProgramATI68875(colorDepth, dotClock);
	break;
    case DAC_CH8398:
	muxMode = mach64ProgramCH8398(colorDepth, dotClock);
	break;
    case DAC_STG1702:
    case DAC_STG1703:
	muxMode = mach64ProgramSTG1702(colorDepth, dotClock);
	break;
    case DAC_ATT20C408:
	muxMode = mach64ProgramATT21C498(colorDepth, dotClock);
	break;
    case DAC_INTERNAL:
	muxMode = mach64ProgramInternal(colorDepth, dotClock);
	break;
    case DAC_IBMRGB514:
	muxMode = mach64ProgramIBMRGB514(colorDepth, AccelMode);
	break;
#ifdef NOT_YET_SUPPORTED
    case DAC_STG1700:
	muxMode = mach64ProgramSTG1700(colorDepth, dotClock);
	break;
    case DAC_BT481:
	muxMode = mach64ProgramBT481(colorDepth);
	break;
    case DAC_ATT21C498:
	muxMode = mach64ProgramATT21C498(colorDepth, dotClock);
	break;
    case DAC_ATT20C491:
    case DAC_ATT498:
    case DAC_BT476:
    case DAC_IMSG174:
    case DAC_MU9C1880:
    case DAC_SC15021:
    case DAC_SC15026:
#endif
    default:
	break;
    }

    (void)inb(ioDAC_REGS);
    outb(ioCRTC_GEN_CNTL+3, temp);

    if (AccelMode && mach64Ramdac != DAC_INTERNAL) {
	temp = regrb(CRTC_GEN_CNTL) & ~CRTC_PIX_BY_2_EN;
	if (muxMode || isMuxMode)
	    temp |= CRTC_PIX_BY_2_EN;
	regwb(CRTC_GEN_CNTL, temp);
    }
}

/*
 * mach64InitDisplay --
 *	Initializes the display for the Mach64.
 */
void mach64InitDisplay(screen_idx)
     int screen_idx;
{
    int temp, i;
    char old_crtc_ext_disp;

    if (mach64Inited)
	return;

    xf86EnableIOPorts(mach64InfoRec.scrnIndex);
    mach64InitAperture(screen_idx);
    mach64ResetEngine();

    mach64SaveVGAInfo(screen_idx);

    WaitIdleEmpty();

    if (!mach64IntegratedController) {
	outb(ATIExtReg, ATI2E); old_ATI2E = inb(ATIExtReg+1);
	outb(ATIExtReg, ATI32); old_ATI32 = inb(ATIExtReg+1);
	outb(ATIExtReg, ATI36); old_ATI36 = inb(ATIExtReg+1);
	outb(VGAGRA, GRA06);    old_GRA06 = inb(VGAGRA+1);
	outb(VGASEQ, SEQ02);    old_SEQ02 = inb(VGASEQ+1);
	outb(VGASEQ, SEQ04);    old_SEQ04 = inb(VGASEQ+1);
    } else {
	if (mach64LCDPanelID >= 0) {
	    if (mach64ChipType == MACH64_LG_ID) {
		/* 3D Rage LT (not Pro) */
		old_LCD_GEN_CTRL = regr(LCD_GEN_CTRL);
		/* Use primary non-shadowed CTRC, disable CRT */
		temp = (old_LCD_GEN_CTRL | (DONT_SHADOW_VPAR | LOCK_8DOT)) &
		    ~(CRT_ON | HORZ_DIVBY2_EN | DISABLE_PCLK_RESET |
		      DIS_HOR_CRT_DIVBY2 | VCLK_DAC_PM_EN | XTALIN_PM_EN |
                      CRTC_RW_SELECT | USE_SHADOWED_VEND |
		      USE_SHADOWED_ROWCUR | SHADOW_EN | SHADOW_RW_EN);
		regw(LCD_GEN_CTRL, temp);
		old_HORZ_STRETCHING = regr(HORZ_STRETCHING);
		old_VERT_STRETCHING = regr(VERT_STRETCHING);
	    } else {
		/* All others */
		old_LCD_INDEX = inl(ioLCD_INDEX);
		/* Use primary non-shadowed CTRC, disable CRT */
		outl(ioLCD_INDEX, (old_LCD_INDEX &
		    ~(LCD_REG_INDEX | LCD_DISPLAY_DIS | LCD_SRC_SEL)) |
		      (LCD_SRC_SEL_CRTC1 | LCD_CRTC2_DISPLAY_DIS |
		       LCD_CONFIG_PANEL));
		old_CONFIG_PANEL = inl(ioLCD_DATA);
		outl(ioLCD_DATA, old_CONFIG_PANEL | DONT_SHADOW_HEND);
		outb(ioLCD_INDEX, LCD_GEN_CNTL);
		old_LCD_GEN_CTRL = inl(ioLCD_DATA);
		temp = (old_LCD_GEN_CTRL | (LOCK_8DOT | DONT_SHADOW_VPAR)) &
		    ~(CRT_ON | HORZ_DIVBY2_EN | DISABLE_PCLK_RESET |
		      DIS_HOR_CRT_DIVBY2 | VCLK_DAC_PM_EN | XTALIN_PM_EN |
                      CRTC_RW_SELECT | USE_SHADOWED_VEND |
		      USE_SHADOWED_ROWCUR | SHADOW_EN | SHADOW_RW_EN);
		outl(ioLCD_DATA, temp);
		outb(ioLCD_INDEX, LCD_HORZ_STRETCHING);
		old_HORZ_STRETCHING = inl(ioLCD_DATA);
		outb(ioLCD_INDEX, LCD_VERT_STRETCHING);
		old_VERT_STRETCHING = inl(ioLCD_DATA);
		outb(ioLCD_INDEX, LCD_EXT_VERT_STRETCH);
		old_EXT_VERT_STRETCH = inl(ioLCD_DATA);
	    }
	}

	old_CRTC_H_SYNC_STRT_WID[0] = regr(CRTC_H_SYNC_STRT_WID);
	old_CRTC_H_TOTAL_DISP[0] = regr(CRTC_H_TOTAL_DISP);
	old_CRTC_V_SYNC_STRT_WID[0] = regr(CRTC_V_SYNC_STRT_WID);
	old_CRTC_V_TOTAL_DISP[0] = regr(CRTC_V_TOTAL_DISP);

	if (mach64LCDPanelID >= 0) {
	    /* Set to save shadow registers */
	    if (mach64ChipType == MACH64_LG_ID) {
		regw(LCD_GEN_CTRL, (temp &
		    ~(DISABLE_PCLK_RESET | CRTC_RW_SELECT)) |
		    (SHADOW_EN | SHADOW_RW_EN));
	    } else {
		outb(ioLCD_INDEX, LCD_GEN_CNTL);
		outl(ioLCD_DATA, (temp &
		    ~(DISABLE_PCLK_RESET | CRTC_RW_SELECT)) |
		    (SHADOW_EN | SHADOW_RW_EN));
	    }

	    old_CRTC_H_SYNC_STRT_WID[1] = regr(CRTC_H_SYNC_STRT_WID);
	    old_CRTC_H_TOTAL_DISP[1] = regr(CRTC_H_TOTAL_DISP);
	    old_CRTC_V_SYNC_STRT_WID[1] = regr(CRTC_V_SYNC_STRT_WID);
	    old_CRTC_V_TOTAL_DISP[1] = regr(CRTC_V_TOTAL_DISP);

	    /* Set to non-shadow */
	    if (mach64ChipType == MACH64_LG_ID) {
		regw(LCD_GEN_CTRL, temp);
	    } else {
		outb(ioLCD_INDEX, LCD_GEN_CNTL);
		outl(ioLCD_DATA, temp);
	    }
	}
    }

    WaitIdleEmpty();

    old_DAC_MASK = inb(ioDAC_REGS+2);

    switch (mach64RamdacSubType) {
    case DAC_ATI68860_B:
    case DAC_ATI68860_C:
	temp = inb(ioDAC_CNTL);
	outb(ioDAC_CNTL, (temp & ~DAC_EXT_SEL_RS2) | DAC_EXT_SEL_RS3);

	old_ATI68860[0] = inb(ioDAC_REGS+2);
	old_ATI68860[1] = inb(ioDAC_REGS+3);
	old_ATI68860[2] = inb(ioDAC_REGS);

	temp = inb(ioDAC_CNTL);
	outb(ioDAC_CNTL, temp | DAC_EXT_SEL_RS2 | DAC_EXT_SEL_RS3);

	old_ATI68860[3] = inb(ioDAC_REGS);

	temp = inb(ioDAC_CNTL);
	outb(ioDAC_CNTL, (temp & ~(DAC_EXT_SEL_RS2 | DAC_EXT_SEL_RS3)));
	break;
    case DAC_ATI68875:
	temp = inb(ioDAC_CNTL);
	outb(ioDAC_CNTL, (temp & ~DAC_EXT_SEL_RS2) | DAC_EXT_SEL_RS3);

	old_ATI68875[0] = inb(ioDAC_REGS+2);
	old_ATI68875[1] = inb(ioDAC_REGS+3);
	old_ATI68875[2] = inb(ioDAC_REGS+1);
    
	temp = inb(ioDAC_CNTL);
	outb(ioDAC_CNTL, (temp & ~(DAC_EXT_SEL_RS2 | DAC_EXT_SEL_RS3)));
	break;
    case DAC_CH8398:
	temp = inb(ioDAC_CNTL);
	outb(ioDAC_CNTL, temp | DAC_EXT_SEL_RS2 | DAC_EXT_SEL_RS3);

	old_CH8398 = inb(ioDAC_REGS+2);

	temp = inb(ioDAC_CNTL);
	outb(ioDAC_CNTL, (temp & ~DAC_EXT_SEL_RS2) | DAC_EXT_SEL_RS3);
	break;
    case DAC_STG1702:
    case DAC_STG1703:
	mach64DACRead4();
	old_STG170X[0] = inb(ioDAC_REGS+2);
	(void)inb(ioDAC_REGS);

	mach64DACRead4();
	outb(ioDAC_REGS+2, old_STG170X[0] | 0x10);
	(void)inb(ioDAC_REGS);

	mach64DACRead4();
	(void)inb(ioDAC_REGS+2);

	outb(ioDAC_REGS+2, 0x03);
	outb(ioDAC_REGS+2, 0x00);

	old_STG170X[1] = inb(ioDAC_REGS+2);
	old_STG170X[2] = inb(ioDAC_REGS+2);
	old_STG170X[3] = inb(ioDAC_REGS+2);
	(void)inb(ioDAC_REGS);
	break;
    case DAC_ATT20C408:
	mach64DACRead4();
	old_ATT20C408 = inb(ioDAC_REGS+2);
	break;
    case DAC_IBMRGB514:
	old_crtc_ext_disp = inb(ioCRTC_GEN_CNTL+3);
	outb(ioCRTC_GEN_CNTL+3, old_crtc_ext_disp | (CRTC_EXT_DISP_EN >> 24));

	for (temp = 0; temp < 0x100; temp++) {
	    old_IBMRGB514[temp] = mach64R_RGB514Index(temp);
#ifdef DEBUG
	    ErrorF("IBM-RGB514[0x%02x] = 0x%02x\n", temp, old_IBMRGB514[temp]);
#endif
	}

	(void)inb(ioDAC_REGS); /* Clear DAC Counter */
	outb(ioCRTC_GEN_CNTL+3, old_crtc_ext_disp);
	break;
    default:
	break;
    }

    if (mach64IntegratedController) {
	oldClockFreq = mach64GetCTClock(mach64CXClk);

	for (i = 0; i < 16; i++) {
	    outb(ioCLOCK_CNTL+1, i << 2);
	    old_PLL[i] = inb(ioCLOCK_CNTL+2);
	}

	if (mach64HasDSP) {
	    old_DSP_CONFIG = regr(DSP_CONFIG);
	    old_DSP_ON_OFF = regr(DSP_ON_OFF);
	}

#ifdef DEBUG
	ErrorF("oldClockFreq = %d\n", oldClockFreq);
#endif
    }

    WaitQueue(7);
    old_BUS_CNTL = regr(BUS_CNTL);
    old_MEM_CNTL = regr(MEM_CNTL);
    old_CRTC_GEN_CNTL = regr(CRTC_GEN_CNTL);

#ifdef DEBUG
    ErrorF("BUS_CNTL was 0x%08x, MEM_CNTL was 0x%08x\n", old_BUS_CNTL,
	   old_MEM_CNTL);
    ErrorF("CRTC_GEN_CNTL was 0x%08x\n", old_CRTC_GEN_CNTL);
#endif

    old_CRTC_OFF_PITCH = regr(CRTC_OFF_PITCH);
    old_DST_OFF_PITCH = regr(DST_OFF_PITCH);
    old_SRC_OFF_PITCH = regr(SRC_OFF_PITCH);

    /* Turn off the VGA memory boundary */
    if (!mach64IntegratedController)
	regw(MEM_CNTL, old_MEM_CNTL & ~(MEM_BNDRY | MEM_BNDRY_EN));

    /* Turn off the memory mapped registers in the frame buffer */
    /* (only when there is an auxilliary register aperture) */
    if (mach64RegisterAddr)
	regw(BUS_CNTL, old_BUS_CNTL | BUS_APER_REG_DIS);

    /* Save the HW_DEBUG register if on a RagePro */
    if (mach64IntegratedController && mach64HasBlockWrite)
	old_HW_DEBUG = regr(HW_DEBUG);

#ifdef DEBUG
    ErrorF("MEM_CNTL is 0x%08x\n", regr(MEM_CNTL));
    ErrorF("BUS_CNTL is 0x%08x\n", regr(BUS_CNTL));
#endif

    WaitQueue(4);

    /* Disable all interrupts. */
    regw(CRTC_INT_CNTL, 0);

    /* Initialize the drawing and display offsets */
    regw(CRTC_OFF_PITCH, (mach64VirtX >> 3) << 22);
    regw(DST_OFF_PITCH, (mach64VirtX >> 3) << 22);
    regw(SRC_OFF_PITCH, (mach64VirtX >> 3) << 22);

    mach64AdjustFrame(mach64InfoRec.frameX0, mach64InfoRec.frameY0);

    /* Save the colormap */
    if (mach64InfoRec.bitsPerPixel == 8)
        mach64InitLUT();

    old_DAC_CNTL = inl(ioDAC_CNTL);
    if (mach64DAC8Bit)
	outl(ioDAC_CNTL, old_DAC_CNTL | DAC_8BIT_EN);

    WaitIdleEmpty(); /* Make sure that all commands have finished */

    mach64Inited = TRUE;
}

/*
 * mach64Cleanup -- 
 * 	Resets the state of the video display for text.
 */
void mach64CleanUp()
{
    int temp, i;
    char old_crtc_ext_disp;

    if (!mach64Inited)
	return;

    WaitIdleEmpty();

    if (mach64IntegratedController) {
    	mach64ProgramClk(mach64CXClk, oldClockFreq);

	for (i = 0; i < 16; i++) {
	    outb(ioCLOCK_CNTL+1, (i << 2) | PLL_WR_EN);
	    outb(ioCLOCK_CNTL+2, old_PLL[i]);
	}

	if (mach64HasDSP) {
	    regw(DSP_CONFIG, old_DSP_CONFIG);
	    regw(DSP_ON_OFF, old_DSP_ON_OFF);
	}
    }

    mach64SetRamdac(CRTC_PIX_WIDTH_8BPP, FALSE, 5035);

    WaitIdleEmpty();
    switch (mach64RamdacSubType) {
    case DAC_ATI68860_B:
    case DAC_ATI68860_C:
	temp = inb(ioDAC_CNTL);
	outb(ioDAC_CNTL, (temp & ~DAC_EXT_SEL_RS2) | DAC_EXT_SEL_RS3);

	outb(ioDAC_REGS+2, old_ATI68860[0]);
	outb(ioDAC_REGS+3, old_ATI68860[1]);
	outb(ioDAC_REGS,   old_ATI68860[2]);

	temp = inb(ioDAC_CNTL);
	outb(ioDAC_CNTL, temp | DAC_EXT_SEL_RS2 | DAC_EXT_SEL_RS3);

	outb(ioDAC_REGS,   old_ATI68860[3]);

	temp = inb(ioDAC_CNTL);
	outb(ioDAC_CNTL, (temp & ~(DAC_EXT_SEL_RS2 | DAC_EXT_SEL_RS3)));
	break;
    case DAC_ATI68875:
	temp = inb(ioDAC_CNTL);
	outb(ioDAC_CNTL, (temp & ~DAC_EXT_SEL_RS2) | DAC_EXT_SEL_RS3);

	outb(ioDAC_REGS+2, old_ATI68875[0]);
	outb(ioDAC_REGS+3, old_ATI68875[1]);
	outb(ioDAC_REGS+1, old_ATI68875[2]);
    
	temp = inb(ioDAC_CNTL);
	outb(ioDAC_CNTL, (temp & ~(DAC_EXT_SEL_RS2 | DAC_EXT_SEL_RS3)));
	break;
    case DAC_CH8398:
	temp = inb(ioDAC_CNTL);
	outb(ioDAC_CNTL, temp | DAC_EXT_SEL_RS2 | DAC_EXT_SEL_RS3);

	outb(ioDAC_REGS+2, old_CH8398);

	temp = inb(ioDAC_CNTL);
	outb(ioDAC_CNTL, (temp & ~DAC_EXT_SEL_RS2) | DAC_EXT_SEL_RS3);
	break;
    case DAC_STG1702:
    case DAC_STG1703:
	mach64DACRead4();
	temp = inb(ioDAC_REGS+2);
	(void)inb(ioDAC_REGS);

	mach64DACRead4();
	outb(ioDAC_REGS+2, temp | 0x10);
	(void)inb(ioDAC_REGS);

	mach64DACRead4();
	(void)inb(ioDAC_REGS+2);

	outb(ioDAC_REGS+2, 0x03);
	outb(ioDAC_REGS+2, 0x00);

	outb(ioDAC_REGS+2, old_STG170X[1]);
	outb(ioDAC_REGS+2, old_STG170X[2]);
	outb(ioDAC_REGS+2, old_STG170X[3]);
	usleep(500);
	(void)inb(ioDAC_REGS);

	mach64DACRead4();
	outb(ioDAC_REGS+2, old_STG170X[0]);
	(void)inb(ioDAC_REGS);
	break;
    case DAC_ATT20C408:
	mach64DACRead4();
	outb(ioDAC_REGS+2, old_ATT20C408);
	break;
    case DAC_IBMRGB514:
	old_crtc_ext_disp = inb(ioCRTC_GEN_CNTL+3);
	outb(ioCRTC_GEN_CNTL+3, old_crtc_ext_disp | (CRTC_EXT_DISP_EN >> 24));

	for (temp = 0; temp < 0x100; temp++)
	    mach64P_RGB514Index(temp, old_IBMRGB514[temp]);

	(void)inb(ioDAC_REGS); /* Clear DAC Counter */
	outb(ioCRTC_GEN_CNTL+3, old_crtc_ext_disp);
	break;
    default:
	break;
    }

    if (LUTInited) {
	mach64RestoreLUT(oldlut);
	LUTInited = FALSE;
    }

    WaitIdleEmpty();

    outb(ioDAC_REGS+2, old_DAC_MASK);
    outl(ioDAC_CNTL, old_DAC_CNTL);

    regw(BUS_CNTL, old_BUS_CNTL);
    regw(MEM_CNTL, old_MEM_CNTL);

    regw(CRTC_OFF_PITCH, old_CRTC_OFF_PITCH);
    regw(DST_OFF_PITCH, old_DST_OFF_PITCH);
    regw(SRC_OFF_PITCH, old_SRC_OFF_PITCH);

    WaitIdleEmpty();
    /* Restore the HW_DEBUG register if on a RagePro */
    if (mach64IntegratedController && mach64HasBlockWrite)
	regw(HW_DEBUG, old_HW_DEBUG);

    mach64CursorOff();

    WaitIdleEmpty();
    if (!mach64IntegratedController) {
	/* Reset the VGA registers */
	outw(ATIExtReg, ATI2E | old_ATI2E << 8);
	outw(ATIExtReg, ATI32 | old_ATI32 << 8);
	outw(ATIExtReg, ATI36 | old_ATI36 << 8);
	outw(VGAGRA, GRA06 | old_GRA06 << 8);
	outw(VGASEQ, SEQ02 | old_SEQ02 << 8);
	outw(VGASEQ, SEQ04 | old_SEQ04 << 8);
    } else {
	if (mach64LCDPanelID >= 0) {
	    /* Stop CRTC */
	    regw(CRTC_GEN_CNTL, old_CRTC_GEN_CNTL &
		 ~(CRTC_EXT_EN | CRTC_EXT_DISP_EN));

	    if (mach64ChipType == MACH64_LG_ID) {
		/* Update non-shadow registers first */
		regw(LCD_GEN_CTRL, old_LCD_GEN_CTRL &
		    ~(DISABLE_PCLK_RESET | CRTC_RW_SELECT | SHADOW_EN |
		      SHADOW_RW_EN));

		/* Temporarily disable stretching */
		regw(HORZ_STRETCHING, old_HORZ_STRETCHING &
		    ~(HORZ_STRETCH_EN | HORZ_STRETCH_MODE));
		regw(VERT_STRETCHING, old_VERT_STRETCHING &
		    ~(VERT_STRETCH_RATIO1 | VERT_STRETCH_RATIO2 |
		      VERT_STRETCH_USE0 | VERT_STRETCH_EN));
	    } else {
		outb(ioLCD_INDEX, LCD_CONFIG_PANEL);
		outl(ioLCD_DATA, old_CONFIG_PANEL);

		/* Update non-shadow registers first */
		outb(ioLCD_INDEX, LCD_GEN_CNTL);
		outl(ioLCD_DATA, old_LCD_GEN_CTRL &
		    ~(DISABLE_PCLK_RESET | CRTC_RW_SELECT | SHADOW_EN |
		      SHADOW_RW_EN));

		/* Temporarily disable stretching */
		outb(ioLCD_INDEX, LCD_HORZ_STRETCHING);
		outl(ioLCD_DATA, old_HORZ_STRETCHING &
		    ~(HORZ_STRETCH_EN | HORZ_STRETCH_MODE));
		outb(ioLCD_INDEX, LCD_VERT_STRETCHING);
		outl(ioLCD_DATA, old_VERT_STRETCHING &
		    ~(VERT_STRETCH_RATIO1 | VERT_STRETCH_RATIO2 |
		      VERT_STRETCH_USE0 | VERT_STRETCH_EN));
	    }
	}

	regw(CRTC_H_SYNC_STRT_WID, old_CRTC_H_SYNC_STRT_WID[0]);
	regw(CRTC_H_TOTAL_DISP, old_CRTC_H_TOTAL_DISP[0]);
	regw(CRTC_V_SYNC_STRT_WID, old_CRTC_V_SYNC_STRT_WID[0]);
	regw(CRTC_V_TOTAL_DISP, old_CRTC_V_TOTAL_DISP[0]);

	if (mach64LCDPanelID >= 0) {
	    /* Switch to shadow registers */
	    if (mach64ChipType == MACH64_LG_ID) {
		regw(LCD_GEN_CTRL, (old_LCD_GEN_CTRL &
		   ~(DISABLE_PCLK_RESET | CRTC_RW_SELECT)) |
		    (SHADOW_EN | SHADOW_RW_EN));
	    } else {
		outb(ioLCD_INDEX, LCD_GEN_CNTL);
		outl(ioLCD_DATA, (old_LCD_GEN_CTRL &
		    ~(DISABLE_PCLK_RESET | CRTC_RW_SELECT)) |
		    (SHADOW_EN | SHADOW_RW_EN));
	    }

	    regw(CRTC_H_SYNC_STRT_WID, old_CRTC_H_SYNC_STRT_WID[1]);
	    regw(CRTC_H_TOTAL_DISP, old_CRTC_H_TOTAL_DISP[1]);
	    regw(CRTC_V_SYNC_STRT_WID, old_CRTC_V_SYNC_STRT_WID[1]);
	    regw(CRTC_V_TOTAL_DISP, old_CRTC_V_TOTAL_DISP[1]);

	    /* Restore CRTC selection and shadow state & enable stretching */
	    if (mach64ChipType == MACH64_LG_ID) {
		regw(LCD_GEN_CTRL, old_LCD_GEN_CTRL);
		regw(HORZ_STRETCHING, old_HORZ_STRETCHING);
		regw(VERT_STRETCHING, old_VERT_STRETCHING);
	    } else {
		outb(ioLCD_INDEX, LCD_GEN_CNTL);
		outl(ioLCD_DATA, old_LCD_GEN_CTRL);
		outb(ioLCD_INDEX, LCD_HORZ_STRETCHING);
		outl(ioLCD_DATA, old_HORZ_STRETCHING);
		outb(ioLCD_INDEX, LCD_VERT_STRETCHING);
		outl(ioLCD_DATA, old_VERT_STRETCHING);
		outb(ioLCD_INDEX, LCD_EXT_VERT_STRETCH);
		outl(ioLCD_DATA, old_EXT_VERT_STRETCH);
		outl(ioLCD_INDEX, old_LCD_INDEX);
	    }
	}
    }

    regw(CRTC_GEN_CNTL, old_CRTC_GEN_CNTL);

    mach64RestoreVGAInfo();

    outw(ioCONFIG_CNTL, old_CONFIG_CNTL);

    xf86DisableIOPorts(mach64InfoRec.scrnIndex);

    mach64Inited = FALSE;
}

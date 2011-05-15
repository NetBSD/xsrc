/*
 * Copyright (c) 2009 KIYOHARA Takashi
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 * 
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "xf86.h"
#include "xf86_OSproc.h"
#include <X11/Xos.h>

#include "compiler.h"

#include "s3.h"

#define GENDAC_INDEX		0x3C8
#define GENDAC_DATA		0x3C9


static void S3GENDACSetClock(ScrnInfoPtr, long, int, int, int, int, int, int,
			     int, long, long);
static void S3GENDACCalcClock(long, int, int, int, int, int, long, long,
			      unsigned char *, unsigned char *);
static void S3GENDACSetPLL(ScrnInfoPtr, int, unsigned char, unsigned char);


static void xf86dactopel(void);

static void
xf86dactopel()
{
	outb(0x3C8,0);
	return;
}


Bool S3GENDACProbe(ScrnInfoPtr pScrn)
{
	/* probe for S3 GENDAC/SDAC */
	/*
	 * S3 GENDAC and SDAC have two fixed read only PLL clocks
	 *     CLK0 f0: 25.255MHz   M-byte 0x28  N-byte 0x61
	 *     CLK0 f1: 28.311MHz   M-byte 0x3d  N-byte 0x62
	 * which can be used to detect GENDAC and SDAC since there is no chip-id
	 * for the GENDAC.
	 *
	 * NOTE: for the GENDAC on a MIRO 10SD (805+GENDAC) reading PLL values
	 * for CLK0 f0 and f1 always returns 0x7f
	 * (but is documented "read only")
	 */

	S3Ptr pS3 = S3PTR(pScrn);
	int vgaCRIndex = pS3->vgaCRIndex, vgaCRReg = pS3->vgaCRReg;
	unsigned char saveCR55, saveCR45, saveCR43, savelut[6];
	unsigned int i;		/* don't use signed int, UW2.0 compiler bug */
	long clock01, clock23;
	int found = 0;

	if (!S3_864_SERIES())	/* need? */
		return FALSE;

	outb(vgaCRIndex, 0x43);
	saveCR43 = inb(vgaCRReg);
	outb(vgaCRReg, saveCR43 & ~0x02);

	outb(vgaCRIndex, 0x45);
	saveCR45 = inb(vgaCRReg);
	outb(vgaCRReg, saveCR45 & ~0x20);

	outb(vgaCRIndex, 0x55);
	saveCR55 = inb(vgaCRReg);
	outb(vgaCRReg, saveCR55 & ~1);

	outb(0x3c7,0);
	for(i = 0; i < 2 * 3; i++)	/* save first two LUT entries */
		savelut[i] = inb(0x3c9);
	outb(0x3c8,0);
	for(i = 0; i < 2 * 3; i++)	/* set first two LUT entries to zero */
		outb(0x3c9, 0);

	outb(vgaCRIndex, 0x55);
	outb(vgaCRReg, saveCR55 | 1);

	outb(0x3c7,0);
	for(i = clock01 = 0; i < 4; i++)
		clock01 = (clock01 << 8) | (inb(0x3c9) & 0xff);
	for(i = clock23 = 0; i < 4; i++)
		clock23 = (clock23 << 8) | (inb(0x3c9) & 0xff);

	outb(vgaCRIndex, 0x55);
	outb(vgaCRReg, saveCR55 & ~1);

	outb(0x3c8,0);
	for(i = 0; i < 2 * 3; i++)	/* restore first two LUT entries */
		outb(0x3c9, savelut[i]);

	outb(vgaCRIndex, 0x55);
	outb(vgaCRReg, saveCR55);

	if (clock01 == 0x28613d62 ||
	    (clock01 == 0x7f7f7f7f && clock23 != 0x7f7f7f7f)) {
		xf86dactopel();
		inb(0x3c6);
		inb(0x3c6);
		inb(0x3c6);

		/* the fourth read will show the SDAC chip ID and revision */
		if (((i = inb(0x3c6)) & 0xf0) == 0x70)
			found = SDAC_RAMDAC;
		else
			found = GENDAC_RAMDAC;
		saveCR43 &= ~0x02;
		saveCR45 &= ~0x20;
		xf86dactopel();
	}

	outb(vgaCRIndex, 0x45);
	outb(vgaCRReg, saveCR45);

	outb(vgaCRIndex, 0x43);
	outb(vgaCRReg, saveCR43);

	if (found) {
		RamDacInit(pScrn, pS3->RamDacRec);
		pS3->RamDac = RamDacHelperCreateInfoRec();
		pS3->RamDac->RamDacType = found;
		return TRUE;
	}
	return FALSE;
}

void S3GENDAC_PreInit(ScrnInfoPtr pScrn)
{
	S3Ptr pS3 = S3PTR(pScrn);
	int vgaCRIndex = pS3->vgaCRIndex, vgaCRReg = pS3->vgaCRReg;
	unsigned char saveCR55;
	int m, n, n1, n2, mclk;

	outb(vgaCRIndex, 0x55);
	saveCR55 = inb(vgaCRReg);
	outb(vgaCRReg, saveCR55 | 1);

	outb(0x3C7, 10); /* read MCLK */
	m = inb(0x3C9);
	n = inb(0x3C9);

	outb(vgaCRIndex, 0x55);
	outb(vgaCRReg, saveCR55);

	m &= 0x7f;
	n1 = n & 0x1f;
	n2 = (n >> 5) & 0x03;
	mclk = ((1431818 * (m + 2)) / (n1 + 2) / (1 << n2) + 50) / 100;

	pS3->mclk = mclk;
	xf86DrvMsg(pScrn->scrnIndex, X_PROBED, "MCLK %1.3f MHz\n",
	    mclk / 1000.0);
}

void S3GENDAC_Init(ScrnInfoPtr pScrn, DisplayModePtr mode)
{
	S3Ptr pS3 = S3PTR(pScrn);
	int vgaCRIndex = pS3->vgaCRIndex, vgaCRReg = pS3->vgaCRReg;
	int daccomm = 0;	/* GENDAC command */
	unsigned char blank, tmp;

	S3GENDACSetClock(pScrn, mode->Clock * (pScrn->depth >> 3), 2, 1, 1, 31,
	    0, 3, 1, 100000, 250000);

	outb(0x3C4, 1);
	blank = inb(0x3C5);
	outb(0x3C5, blank | 0x20); /* blank the screen */

	switch (pScrn->depth)
	{
	case 8:		/* 8-bit color, 1VCLK/pixel */
		break;

	case 15:	/* 15-bit color, 2VCLK/pixel */
		daccomm = 0x20;
		break;

	case 16:	/* 16-bit color, 2VCLK/pixel */
		daccomm = 0x60;
		break;

	case 32:	/* 32-bit color, 3VCLK/pixel */
		daccomm = 0x40;
	}

	outb(vgaCRIndex, 0x55);
	tmp = inb(vgaCRReg) | 1;
	outb(vgaCRReg, tmp);

	outb(0x3c6, daccomm);		/* set GENDAC mux mode */

	outb(vgaCRIndex, 0x55);
	tmp = inb(vgaCRReg) & ~1;
	outb(vgaCRReg, tmp);

	outb(0x3C4, 1);
	outb(0x3C5, blank);		/* unblank the screen */
}

void S3SDAC_Init(ScrnInfoPtr pScrn, DisplayModePtr mode)
{
	S3Ptr pS3 = S3PTR(pScrn);
	int vgaCRIndex = pS3->vgaCRIndex, vgaCRReg = pS3->vgaCRReg;
	int pixmux = 0;		/* SDAC command and CR67 */
	int blank_delay = 0;	/* CR6D */
	int invert_vclk = 0;	/* CR66 bit 0 */
	unsigned char blank, tmp;

#if 0
	S3GENDACSetClock(pScrn,
	    (pScrn->depth == 32) ? mode->Clock * 2 : mode->Clock,
	    2, 1, 1, 31, 0, 3, 1, 100000, 250000);
#else
	/* XXXX: for prep */
	long freq;

	switch (pScrn->depth) {
	case 32:
		freq = mode->Clock * 2;	/* XXXX: frem xfree86 3.x */
		break;
	case 16:
		freq = mode->Clock / 2;
		break;
	default:
		freq = mode->Clock;
		break;
	}
	S3GENDACSetClock(pScrn, freq,
	    2, 1, 1, 31, 0, 3, 1, 100000, 250000);
#endif

	outb(vgaCRIndex, 0x42);/* select the clock */
	tmp = inb(vgaCRReg) & 0xf0;
	outb(vgaCRReg, tmp | 0x02);
	usleep(150000);

	outb(0x3C4, 1);
	blank = inb(0x3C5);
	outb(0x3C5, blank | 0x20); /* blank the screen */

	switch (pScrn->depth)
	{
	case 8:		/* 8-bit color, 1 VCLK/pixel */
		pixmux = 0x10;
		blank_delay = 2;
		invert_vclk = 1;
		break;

	case 15:	/* 15-bit color, 1VCLK/pixel */
		pixmux = 0x30;
		blank_delay = 2;
		break;

	case 16:	/* 16-bit color, 1VCLK/pixel */
		pixmux = 0x50;
		blank_delay = 2;
		break;

	case 32:	/* 32-bit color, 2VCLK/pixel */
		pixmux = 0x70;
		blank_delay = 2;
	}

	outb(vgaCRIndex, 0x55);
	tmp = inb(vgaCRReg) | 1;
	outb(vgaCRReg, tmp);

	outb(vgaCRIndex, 0x67);
	outb(vgaCRReg, pixmux | invert_vclk);	/* set S3 mux mode */
	outb(0x3c6, pixmux);			/* set SDAC mux mode */

	outb(vgaCRIndex, 0x6D);
	outb(vgaCRReg, blank_delay);		/* set blank delay */

	outb(vgaCRIndex, 0x55);
	tmp = inb(vgaCRReg) & ~1;
	outb(vgaCRReg, tmp);

	outb(0x3C4, 1);
	outb(0x3C5, blank);			/* unblank the screen */
}

void S3GENDAC_Save(ScrnInfoPtr pScrn)
{
	S3Ptr pS3 = S3PTR(pScrn);
	int vgaCRIndex = pS3->vgaCRIndex, vgaCRReg = pS3->vgaCRReg;
	S3RegPtr save = &pS3->SavedRegs;
	unsigned char tmp;

	outb(vgaCRIndex, 0x55);
	tmp = inb(vgaCRReg);
	outb(vgaCRReg, tmp | 1);

	save->dacregs[0] = inb(0x3c6);	/* Enhanced command register */
	save->dacregs[2] = inb(0x3c8);	/* PLL write index */
	save->dacregs[1] = inb(0x3c7);	/* PLL read index */
	outb(0x3c7, 2);		/* index to f2 reg */
	save->dacregs[3] = inb(0x3c9);	/* f2 PLL M divider */
	save->dacregs[4] = inb(0x3c9);	/* f2 PLL N1/N2 divider */
	outb(0x3c7, 0x0e);	/* index to PLL control */
	save->dacregs[5] = inb(0x3c9);	/* PLL control */

	outb(vgaCRReg, tmp & ~1);
}

void S3GENDAC_Restore(ScrnInfoPtr pScrn)
{
	S3Ptr pS3 = S3PTR(pScrn);
	int vgaCRIndex = pS3->vgaCRIndex, vgaCRReg = pS3->vgaCRReg;
	S3RegPtr restore = &pS3->SavedRegs;
	unsigned char tmp;

	outb(vgaCRIndex, 0x55);
	tmp = inb(vgaCRReg);
	outb(vgaCRReg, tmp | 1);

	outb(0x3c6, restore->dacregs[0]);	/* Enhanced command register */
	outb(0x3c8, 2);			/* index to f2 reg */
	outb(0x3c9, restore->dacregs[3]);	/* f2 PLL M divider */
	outb(0x3c9, restore->dacregs[4]);	/* f2 PLL N1/N2 divider */
	outb(0x3c8, 0x0e);		/* index to PLL control */
	outb(0x3c9, restore->dacregs[5]);	/* PLL control */
	outb(0x3c8, restore->dacregs[2]);	/* PLL write index */
	outb(0x3c7, restore->dacregs[1]);	/* PLL read index */

	outb(vgaCRReg, tmp & ~1);
}

static void
S3GENDACSetClock(ScrnInfoPtr pScrn, long freq, int clk, int min_m, int min_n1,
	       int max_n1, int min_n2, int max_n2, int pll_type, long freq_min,
	       long freq_max)
{
	unsigned char m, n;

	S3GENDACCalcClock(freq, min_m, min_n1, max_n1, min_n2, max_n2,
	    freq_min, freq_max, &m, &n);

	/* XXX for pll_type == GENDAC */
	S3GENDACSetPLL(pScrn, clk, m, n);
}

/* This function is copy from S3GENDACCalcClock() */
static void
S3GENDACCalcClock(long freq, int min_m, int min_n1, int max_n1, int min_n2,
		  int max_n2, long freq_min, long freq_max,
		  unsigned char *mdiv, unsigned char *ndiv)
{
	double ffreq, ffreq_min, ffreq_max;
	double div, diff, best_diff;
	unsigned int m;
	unsigned char n1, n2, best_n1=18, best_n2=2, best_m=127;

#define BASE_FREQ	14.31818
	ffreq = freq / 1000.0 / BASE_FREQ;
	ffreq_min = freq_min / 1000.0 / BASE_FREQ;
	ffreq_max = freq_max / 1000.0 / BASE_FREQ;

	if (ffreq < ffreq_min / (1 << max_n2)) {
		ErrorF("invalid frequency %1.3f Mhz [freq >= %1.3f Mhz]\n",
		    ffreq*BASE_FREQ, ffreq_min*BASE_FREQ / (1 << max_n2));
		ffreq = ffreq_min / (1 << max_n2);
	}
	if (ffreq > ffreq_max / (1 << min_n2)) {
		ErrorF("invalid frequency %1.3f Mhz [freq <= %1.3f Mhz]\n",
		    ffreq*BASE_FREQ, ffreq_max*BASE_FREQ / (1 << min_n2));
		ffreq = ffreq_max / (1<<min_n2);
	}

	/* work out suitable timings */

	best_diff = ffreq;

	for (n2 = min_n2; n2 <= max_n2; n2++) {
		for (n1 = min_n1 + 2; n1 <= max_n1 + 2; n1++) {
			m = (int)(ffreq * n1 * (1 << n2) + 0.5);
			if (m < min_m + 2 || m > 127 + 2)
				continue;
			div = (double)(m) / (double)(n1);
			if ((div >= ffreq_min) && (div <= ffreq_max)) {
				diff = ffreq - div / (1 << n2);
				if (diff < 0.0)
					diff = -diff;
				if (diff < best_diff) {
					best_diff = diff;
					best_m = m;
					best_n1 = n1;
					best_n2 = n2;
				}
			}
		}
	}

	if (max_n1 == 63)
		*ndiv = (best_n1 - 2) | (best_n2 << 6);
	else
		*ndiv = (best_n1 - 2) | (best_n2 << 5);
	*mdiv = best_m - 2;
}

static void
S3GENDACSetPLL(ScrnInfoPtr pScrn, int clk, unsigned char m, unsigned char n)
{
	S3Ptr pS3 = S3PTR(pScrn);
	unsigned char tmp, tmp1;
	int vgaCRIndex = pS3->vgaCRIndex, vgaCRReg = pS3->vgaCRReg;

	/* set RS2 via CR55, yuck */
	outb(vgaCRIndex, 0x55);
	tmp = inb(vgaCRReg) & 0xFC;
	outb(vgaCRReg, tmp | 0x01);
	tmp1 = inb(GENDAC_INDEX);

	outb(GENDAC_INDEX, clk);
	outb(GENDAC_DATA, m);
	outb(GENDAC_DATA, n);

	/* Now clean up our mess */
	outb(GENDAC_INDEX, tmp1);
	outb(vgaCRReg, tmp);
}

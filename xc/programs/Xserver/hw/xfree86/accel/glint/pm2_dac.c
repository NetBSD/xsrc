/* $XFree86: xc/programs/Xserver/hw/xfree86/accel/glint/pm2_dac.c,v 1.6.2.5 1999/08/17 07:39:27 hohndel Exp $ */
/*
 * Copyright 1997 The XFree86 Project, Inc
 *
 * programming the onboard RAMDAC of the Permedia 2 chip
 *
 * Dirk Hohndel <hohndel@XFree86.Org>
 */

#include "Xfuncproto.h"
#include "compiler.h"
#define NO_OSLIB_PROTOTYPES
#include "xf86.h"
#include "xf86_Config.h"

#include "glint.h"
#include "glint_regs.h"

extern int coprotype;
extern Bool UsePCIRetry;

/*
 * glintOutPM2DACIndReg() and glintInPM2DACIndReg() are used to access 
 * the indirect DAC registers of the Permedia 2.
 */
void 
glintOutPM2IndReg(unsigned char reg, unsigned char mask, unsigned char data)
{
    unsigned char tmp, tmp2 = 0x00;

    if (coprotype == PCI_CHIP_3DLABS_PERMEDIA2V) {
    	GLINT_SLOW_WRITE_REG(reg, PM2VDACIndexRegLow);

    	if (mask != 0x00)
		tmp2 = GLINT_READ_REG(PM2VDACIndexData) & mask;
        GLINT_SLOW_WRITE_REG(tmp2 | data, PM2VDACIndexData);
    } else {
        GLINT_SLOW_WRITE_REG(reg, PM2DACIndexReg);

    	if (mask != 0x00)
		tmp2 = GLINT_READ_REG(PM2DACIndexData) & mask;
        GLINT_SLOW_WRITE_REG(tmp2 | data, PM2DACIndexData);
    }
}

unsigned char 
glintInPM2DACIndReg(unsigned char reg)
{
    unsigned char tmp, ret;

    if (coprotype == PCI_CHIP_3DLABS_PERMEDIA2V) {
    	GLINT_SLOW_WRITE_REG(reg, PM2VDACIndexRegLow);
    	ret = GLINT_READ_REG(PM2VDACIndexData);
    } else {
    	GLINT_SLOW_WRITE_REG(reg, PM2DACIndexReg);
    	ret = GLINT_READ_REG(PM2DACIndexData);
    }

    return(ret);
}

#define INITIALFREQERR 100000
#define MINCLK 150000		/* VCO frequency range */
#define MAXCLK 300000

unsigned long
PM2DAC_CalculateMNPCForClock
(
 unsigned long reqclock,		/* In kHz units */
 unsigned long refclock,		/* In kHz units */
 unsigned char *rm, 			/* M Out */
 unsigned char *rn, 			/* N Out */
 unsigned char *rp			/* P Out */
 )
{
    unsigned char	m, n, p;
    unsigned long	f;
    long		freqerr, lowestfreqerr = INITIALFREQERR;
    unsigned long  	clock,actualclock = 0;

    for (n = 2; n <= 14; n++) {
        for (m = 2; m != 0; m++) { /* this is a char, so this counts to 255 */
	    f = refclock * m / n;
	    if ( (f < MINCLK) || (f > MAXCLK) )
	    	continue;
	    for (p = 0; p <= 4; p++) {
	    	clock = f >> p;
		freqerr = reqclock - clock;
		if (freqerr < 0)
		    freqerr = -freqerr;
		if (freqerr < lowestfreqerr) {
		    *rn = n;
		    *rm = m;
		    *rp = p;
		    lowestfreqerr = freqerr;
		    actualclock = clock;
#ifdef DEBUG
	ErrorF("Best %ld diff %ld\n",actualclock,freqerr);
#endif
		}
	    }
	}
    }

    return(actualclock);
}

void 
PM2DACGlintSetClock(long freq)
{
    unsigned char m,n,p;
    unsigned long clockused;

    unsigned long fref = 14318;

    int timeout = 100;
    int stable;

    clockused = PM2DAC_CalculateMNPCForClock(freq,fref,&m,&n,&p);

#ifdef DEBUG
    ErrorF("requested %ld, setting to %ld, m %d, n %d, p %d\n",
           freq,clockused,m,n,p);
#endif

    /*
     * now set the clock
     */
    glintOutPM2IndReg(PM2DACIndexClockAP,0x00,0x00);
    glintOutPM2IndReg(PM2DACIndexClockAM,0x00,m);
    glintOutPM2IndReg(PM2DACIndexClockAN,0x00,n);
    glintOutPM2IndReg(PM2DACIndexClockAP,0x00,p|0x08);
    do
    {
      stable = glintInPM2DACIndReg(PM2DACIndexClockStatus);
    } while ((!(stable & (1 << 4))) && (--timeout));
}

void 
PM2DACGlintSetMClock(long freq)
{
    unsigned char m,n,p;
    unsigned long clockused;

    unsigned long fref = 14318;

    int timeout = 100;
    int stable;

    clockused = PM2DAC_CalculateMNPCForClock(freq,fref,&m,&n,&p);

#ifdef DEBUG
    ErrorF("requested %ld, setting to %ld, m %d, n %d, p %d\n",
           freq,clockused,m,n,p);
#endif

    /*
     * now set the clock
     */
    glintOutPM2IndReg(PM2DACIndexMemClockP,0x00,0x06);
    glintOutPM2IndReg(PM2DACIndexMemClockM,0x00,m);
    glintOutPM2IndReg(PM2DACIndexMemClockN,0x00,n);
    glintOutPM2IndReg(PM2DACIndexMemClockP,0x00,p|0x08);
    do
    {
      stable = glintInPM2DACIndReg(PM2DACIndexMemClockStatus);
    } while ((!(stable & (1 << 4))) && (--timeout));
}

int
PM2DACInit(int clock)
{
    int CC = GLINT_READ_REG(ChipConfig);
    /*
     * set up the RAMDAC in the right mode
     */
    GLINT_WRITE_REG(CC & 0xFFFFFFFD, ChipConfig); /* Disable VGA */

    /* - PaletteWidth 8bits
     *
     * Set some additional magic bits, only needed for SOG:
     * - Inverted HSyncPolarity 
     * - Inverted VSyncPolarity
     * - Enable BlankPedestal (whatever this does, really magic!)
     * - Enable SyncOnGreen (of course)
     */
    glintOutPM2IndReg(PM2DACIndexMCR, 0x00,  
                      OFLG_ISSET (OPTION_SYNC_ON_GREEN, 
                                  &glintInfoRec.options) ? 0x3e : 0x02);
    glintOutPM2IndReg(PM2DACIndexMDCR, 0x00, 0x00); /* Disable Overlay */
  
    switch (glintInfoRec.bitsPerPixel)
    {
    case 8:
    	glintOutPM2IndReg(PM2DACIndexCMR,0x00,
			  PM2DAC_RGB|PM2DAC_GRAPHICS|PM2DAC_CI8);
    	break;
    case 15:
    	glintOutPM2IndReg(PM2DACIndexCMR,0x00,
			  PM2DAC_RGB|PM2DAC_TRUECOLOR|PM2DAC_GRAPHICS|
			  PM2DAC_5551);
    	break;
    case 16:
    	glintOutPM2IndReg(PM2DACIndexCMR,0x00,
			  PM2DAC_RGB|PM2DAC_TRUECOLOR|PM2DAC_GRAPHICS|
			  PM2DAC_565);
    	break;
    case 24:
    	glintOutPM2IndReg(PM2DACIndexCMR,0x00,
			  PM2DAC_RGB|PM2DAC_TRUECOLOR|PM2DAC_GRAPHICS|
			  PM2DAC_PACKED);
    	break;
    case 32:
    	glintOutPM2IndReg(PM2DACIndexCMR,0x00,
			  PM2DAC_RGB|PM2DAC_TRUECOLOR|PM2DAC_GRAPHICS|
			  PM2DAC_8888);
    	break;
    }
    PM2DACGlintSetClock(clock);
}

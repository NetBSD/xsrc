/* $XFree86: xc/programs/Xserver/hw/xfree86/accel/glint/pm2v_dac.c,v 1.1.2.3 1999/04/15 14:28:58 hohndel Exp $ */
/*
 * Copyright 1998 The XFree86 Project, Inc
 *
 * programming the onboard RAMDAC of the Permedia 2v chip
 *
 * Alan Hourihane <alanh@fairlite.demon.co.uk>
 */

#include "Xfuncproto.h"
#include "compiler.h"
#define NO_OSLIB_PROTOTYPES
#include "xf86.h"
#include "xf86_Config.h"

#include "glint.h"
#include "glint_regs.h"

extern Bool UsePCIRetry;

static unsigned long
PM2VDAC_CalculateClock
(
 unsigned long reqclock,		/* In kHz units */
 unsigned long refclock,		/* In kHz units */
 unsigned char *prescale,		/* ClkPreScale */
 unsigned char *feedback, 		/* ClkFeedBackScale */
 unsigned char *postscale		/* ClkPostScale */
 )
{
    int			f, pre, post;
    unsigned long	freq;
    long		freqerr = 1000;
    unsigned long  	actualclock = 0;
    unsigned char	divide[5] = { 1, 2, 4, 8, 16 };

    for (f=1;f<256;f++) {
	for (pre=1;pre<256;pre++) {
	    for (post=0;post<2;post++) { 
	    	freq = ((refclock * f) / (pre * (1 << divide[post])));
		if ((reqclock > freq - freqerr)&&(reqclock < freq + freqerr)){
		    freqerr = (reqclock > freq) ? 
					reqclock - freq : freq - reqclock;
		    *feedback = f;
		    *prescale = pre;
		    *postscale = post;
		    actualclock = freq;
		}
	    }
	}
    }

    return(actualclock);
}

void 
PM2VDACGlintSetClock(long freq)
{
    unsigned char m,n,p;
    unsigned long clockused;

    unsigned long fref = 14318;
    clockused = PM2VDAC_CalculateClock(freq/2,fref,&m,&n,&p);

#ifdef DEBUG
    ErrorF("requested %ld, setting to %ld, m %d, n %d, p %d\n",
           freq,clockused,m,n,p);
#endif

    /*
     * now set the clock
     */
    GLINT_SLOW_WRITE_REG(PM2VDACRDDClk0PreScale >> 8, PM2VDACIndexRegHigh);
    glintOutPM2IndReg(PM2VDACRDDClk0PreScale,0x00,m);
    glintOutPM2IndReg(PM2VDACRDDClk0FeedbackScale,0x00,n);
    glintOutPM2IndReg(PM2VDACRDDClk0PostScale,0x00,p);
    GLINT_SLOW_WRITE_REG(0, PM2VDACIndexRegHigh);
}

int
PM2VDACInit(int clock)
{
    int CC = GLINT_READ_REG(ChipConfig);
    /*
     * set up the RAMDAC in the right mode
     */
    GLINT_WRITE_REG(CC & 0xFFFFFFFD, ChipConfig); /* Disable VGA */
    glintOutPM2IndReg(PM2VDACRDMiscControl, 0x00, 0x01); /* 8bit DAC */
  
    switch (glintInfoRec.bitsPerPixel)
    {
    case 8:
    	glintOutPM2IndReg(PM2VDACRDPixelSize,0x00,0x00);
    	glintOutPM2IndReg(PM2VDACRDColorFormat,0x00,0x0E);
    	break;
    case 15:
    	glintOutPM2IndReg(PM2VDACRDPixelSize,0x00,0x01);
    	glintOutPM2IndReg(PM2VDACRDColorFormat,0x00,0x61);
    	break;
    case 16:
    	glintOutPM2IndReg(PM2VDACRDPixelSize,0x00,0x01);
    	glintOutPM2IndReg(PM2VDACRDColorFormat,0x00,0x70);
    	break;
    case 24:
    	glintOutPM2IndReg(PM2VDACRDPixelSize,0x00,0x04);
    	glintOutPM2IndReg(PM2VDACRDColorFormat,0x00,0x20);
    	break;
    case 32:
    	glintOutPM2IndReg(PM2VDACRDPixelSize,0x00,0x02);
    	glintOutPM2IndReg(PM2VDACRDColorFormat,0x00,0x20);
    	break;
    }
    PM2VDACGlintSetClock(clock);
}

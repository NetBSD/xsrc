/*
 * Copyright 2000 by Sven Luther <luther@dpt-info.u-strasbg.fr>.
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Sven Luther not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission. Sven Luther makes no representations
 * about the suitability of this software for any purpose.  It is provided
 * "as is" without express or implied warranty.
 *
 * SVEN LUTHER DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL SVEN LUTHER BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 *
 * Authors: Sven Luther, <luther@dpt-info.u-strasbg.fr>
 *          Thomas Witzel, <twitzel@nmr.mgh.harvard.edu>
 *
 * this work is sponsored by Appian Graphics.
 * 
 */
/* $XFree86: xc/programs/Xserver/hw/xfree86/drivers/glint/pm3_dac.c,v 1.8 2000/10/26 13:41:32 alanh Exp $ */

#include "xf86.h"
#include "xf86_OSproc.h"
#include "xf86_ansic.h"

#include "xf86PciInfo.h"
#include "xf86Pci.h"

#include "glint_regs.h"
#include "pm3_regs.h"
#include "glint.h"

#define DEBUG 0

#if DEBUG
# define TRACE_ENTER(str)       ErrorF("glint: " str " %d\n",pScrn->scrnIndex)
# define TRACE_EXIT(str)        ErrorF("glint: " str " done\n")
# define TRACE(str)             ErrorF("glint trace: " str "\n")
#else
# define TRACE_ENTER(str)
# define TRACE_EXIT(str)
# define TRACE(str)
#endif

int PM3QuickFillMemory(ScrnInfoPtr pScrn,int size);

int
PM3QuickFillMemory(ScrnInfoPtr pScrn,int size)
{
    GLINTPtr pGlint = GLINTPTR (pScrn);
    unsigned int * p;
    unsigned int p_content;
    unsigned int i, j;
    long savemapsize;

    savemapsize = pGlint->FbMapSize;
    pGlint->FbMapSize = size*1024*1024;

    pGlint->FbBase = xf86MapPciMem(pScrn->scrnIndex, VIDMEM_FRAMEBUFFER,
	pGlint->PciTag, pGlint->FbAddress, pGlint->FbMapSize);
    if (pGlint->FbBase == NULL) {
	pGlint->FbMapSize = savemapsize;
	return FALSE;
    }

    /* Set pointer to Aperture1 */
    p = (unsigned int *) pGlint->FbBase;
    /* Fill in megs number of megabytes */
    for(i=0;i<size;i++)
	for(j=0;j<1024*256;j+=1024)
	    p[j+(i*1024*256)] = j + (i * 1024*256);

    /* Set pointer to Aperture1 */
    p = (unsigned int *) pGlint->FbBase;

    /* If original ptr is 0x0 then no rollover occured */
    p_content = p[0];
    xf86UnMapVidMem(pScrn->scrnIndex, (pointer)pGlint->FbBase,
	pGlint->FbMapSize);
    pGlint->FbBase = NULL;
    pGlint->FbMapSize = savemapsize;
    if (p_content == 0x0)
	return TRUE;
    else return FALSE;
}
int
Permedia3MemorySizeDetect(ScrnInfoPtr pScrn)
{
    GLINTPtr pGlint;
    int size = 1;
    pGlint = GLINTPTR (pScrn);
    /* Fill memory until get a rollover of dram to 0
     * fill in powers of 2, 1,2,4,8,16,32
     */
    while(PM3QuickFillMemory(pScrn,size))
    {
	size = size*2;
	if(size == 64) break;
    }
    /* Correct memory amount since fail */
    if (size != 1)
	size = size / 2;
    else
	return 1*1024;
    /* Just to make sure */
    if (PM3QuickFillMemory(pScrn,size))
	return size*1024;
    return 16*1024;
}

static int
Shiftbpp(ScrnInfoPtr pScrn, int value)
{
    GLINTPtr pGlint = GLINTPTR(pScrn);
    /* shift horizontal timings for 128bit SGRAMs or SDRAMs */
    int logbytesperaccess = 4;
	
    switch (pScrn->bitsPerPixel) {
    case 8:
	value >>= logbytesperaccess;
	pGlint->BppShift = logbytesperaccess;
	break;
    case 16:
	if (pGlint->DoubleBuffer) {
	    value >>= (logbytesperaccess-2);
	    pGlint->BppShift = logbytesperaccess-2;
	} else {
	    value >>= (logbytesperaccess-1);
	    pGlint->BppShift = logbytesperaccess-1;
	}
	break;
    case 24:
	value *= 3;
	value >>= logbytesperaccess;
	pGlint->BppShift = logbytesperaccess;
	break;
    case 32:
	value >>= (logbytesperaccess-2);
	pGlint->BppShift = logbytesperaccess-2;
	break;
    }
    return (value);
}

static unsigned long
PM3DAC_CalculateClock
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

    for (f=1;f<256;f++) {
	for (pre=1;pre<256;pre++) {
	    for (post=0;post<5;post++) { 
	    	freq = ((2* refclock * f) / (pre * (1 << post)));
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
Permedia3PreInit(ScrnInfoPtr pScrn)
{
    GLINTPtr pGlint = GLINTPTR(pScrn);
    TRACE_ENTER("Permedia3PreInit");
    if ((pGlint->PciInfo->subsysVendor == 0x1097) &&
	(pGlint->PciInfo->subsysCard == 0x3d32)) {
    	unsigned char m,n,p;
    	unsigned long clockused;
        xf86DrvMsg(pScrn->scrnIndex, X_INFO,
	    "Appian Jeronimo 2000 board detected and initialized.\n\t"
	    "subsysVendor = 0x%04x, subsysCard = 0x%04x.\n",
	    pGlint->PciInfo->subsysVendor, pGlint->PciInfo->subsysCard);

	/* Memory timings for the Appian J2000 board.
	 * This is needed for the second head which is left unitilialized
	 * by the bios, thus freezing the machine. */
	GLINT_SLOW_WRITE_REG(0x02e311B8, PM3LocalMemCaps);
	GLINT_SLOW_WRITE_REG(0x07424905, PM3LocalMemTimings);
	GLINT_SLOW_WRITE_REG(0x0c000003, PM3LocalMemControl);
	GLINT_SLOW_WRITE_REG(0x00000061, PM3LocalMemRefresh);
	GLINT_SLOW_WRITE_REG(0x00000000, PM3LocalMemPowerDown);
	
	/* Let's program the K, M and S Clocks to the same values as the bios
	 * does for first head :
	 *   - KClk and MClk are 105Mhz.
	 *   - S Clock is set to PClk.
	 * Note 1 : pGlint->RefClock is not set yet, so use 14318 instead.
	 * Note 2 : KClk gets internally halved, so we need to double it.
	 */
	clockused = PM3DAC_CalculateClock(2*105000, 14318, &m,&n,&p);
        Permedia2vOutIndReg(pScrn, PM3RD_KClkPreScale, 0x00, m);
        Permedia2vOutIndReg(pScrn, PM3RD_KClkFeedbackScale, 0x00, n);
        Permedia2vOutIndReg(pScrn, PM3RD_KClkPostScale, 0x00, p);
        Permedia2vOutIndReg(pScrn, PM3RD_KClkControl, 0x00,
	    PM3RD_KClkControl_STATE_RUN |
	    PM3RD_KClkControl_SOURCE_PLL |
	    PM3RD_KClkControl_ENABLE);
        Permedia2vOutIndReg(pScrn, PM3RD_MClkControl, 0x00,
	    PM3RD_MClkControl_STATE_RUN |
	    PM3RD_MClkControl_SOURCE_KCLK |
	    PM3RD_MClkControl_ENABLE);
        Permedia2vOutIndReg(pScrn, PM3RD_SClkControl, 0x00,
	    PM3RD_SClkControl_STATE_RUN |
	    PM3RD_SClkControl_SOURCE_PCLK |
	    PM3RD_SClkControl_ENABLE);
    }
    else xf86DrvMsg(pScrn->scrnIndex, X_INFO,
	"Unknown Glint Permedia3 board detected.\n\t"
	"subsysVendor = 0x%04x, subsysCard = 0x%04x.\n\t"
	"Let's hope that it is correctly initialized by the bios.\n",
	pGlint->PciInfo->subsysVendor, pGlint->PciInfo->subsysCard);
    TRACE_EXIT("Permedia3PreInit");
}

Bool
Permedia3Init(ScrnInfoPtr pScrn, DisplayModePtr mode)
{
    GLINTPtr pGlint = GLINTPTR(pScrn);
    GLINTRegPtr pReg = &pGlint->ModeReg;
    CARD32 temp1, temp2, temp3, temp4;

    pReg->glintRegs[PM3MemBypassWriteMask >> 3] = 0xffffffff;
    pReg->glintRegs[PM3ByAperture1Mode >> 3] = 0x00000000;
    pReg->glintRegs[PM3ByAperture2Mode >> 3] = 0x00000000;

    pReg->glintRegs[Aperture0 >> 3] = 0;
    pReg->glintRegs[Aperture1 >> 3] = 0;

    if (pGlint->UsePCIRetry) {
	pReg->glintRegs[DFIFODis >> 3] = 1;
	pReg->glintRegs[FIFODis >> 3] = 3;
    } else {
	pReg->glintRegs[DFIFODis >> 3] = 0;
	pReg->glintRegs[FIFODis >> 3] = 1;
    }

    temp1 = mode->CrtcHSyncStart - mode->CrtcHDisplay;
    temp2 = mode->CrtcVSyncStart - mode->CrtcVDisplay;
    temp3 = mode->CrtcHSyncEnd - mode->CrtcHSyncStart;
    temp4 = mode->CrtcVSyncEnd - mode->CrtcVSyncStart;

    pReg->glintRegs[PMHTotal >> 3] = Shiftbpp(pScrn,mode->CrtcHTotal);
    pReg->glintRegs[PMHsEnd >> 3] = Shiftbpp(pScrn, temp1 + temp3);
    pReg->glintRegs[PMHsStart >> 3] = Shiftbpp(pScrn, temp1);
    pReg->glintRegs[PMHbEnd >> 3] = 
		Shiftbpp(pScrn,mode->CrtcHTotal-mode->CrtcHDisplay);
    pReg->glintRegs[PMHgEnd >> 3] = pReg->glintRegs[PMHbEnd >> 3];
    pReg->glintRegs[PMScreenStride >> 3] = 
		Shiftbpp(pScrn,pScrn->displayWidth);

    pReg->glintRegs[PMVTotal >> 3] = mode->CrtcVTotal;
    pReg->glintRegs[PMVsEnd >> 3] = temp2 + temp4;
    pReg->glintRegs[PMVsStart >> 3] = temp2;
    pReg->glintRegs[PMVbEnd >> 3] = mode->CrtcVTotal - mode->CrtcVDisplay;

    pReg->glintRegs[PMHTotal >> 3] -= 1; 
    pReg->glintRegs[PMVTotal >> 3] -= 1;
    pReg->glintRegs[PMVsStart >> 3] -= 1;
    pReg->glintRegs[PMVsEnd >> 3] -= 1;

    /* The hw cursor needs /VSYNC to recognize vert retrace. We'll stick
       both sync lines to active high here and if needed invert them
       using the RAMDAC's RDSyncControl below. */
    /* We need to set the pixelsize (bit 19 & 20) also ... */
    switch (pScrn->bitsPerPixel)
    {
	case 8:
	    pReg->glintRegs[PM3ByAperture1Mode >> 3] =
		PM3ByApertureMode_PIXELSIZE_8BIT;
	    pReg->glintRegs[PM3ByAperture2Mode >> 3] =
		PM3ByApertureMode_PIXELSIZE_8BIT;
    	    pReg->glintRegs[PMVideoControl >> 3] =
		1 | (1 << 3) | (1 << 5) | (0 << 19);
	    break;
	case 16:
	    pReg->glintRegs[PM3ByAperture1Mode >> 3] =
		PM3ByApertureMode_PIXELSIZE_16BIT;
	    pReg->glintRegs[PM3ByAperture2Mode >> 3] =
		PM3ByApertureMode_PIXELSIZE_16BIT;
    	    pReg->glintRegs[PMVideoControl >> 3] =
		1 | (1 << 3) | (1 << 5) | (1 << 19);
	    break;
	case 32:
	    pReg->glintRegs[PM3ByAperture1Mode >> 3] =
		PM3ByApertureMode_PIXELSIZE_32BIT;
	    pReg->glintRegs[PM3ByAperture2Mode >> 3] =
		PM3ByApertureMode_PIXELSIZE_32BIT;
    	    pReg->glintRegs[PMVideoControl >> 3] =
		1 | (1 << 3) | (1 << 5) | (2 << 19);
	    break;
    }

    pReg->glintRegs[VClkCtl >> 3] = (GLINT_READ_REG(VClkCtl) & 0xFFFFFFFC);
    pReg->glintRegs[PMScreenBase >> 3] = 0; 

    pReg->glintRegs[ChipConfig >> 3] = GLINT_READ_REG(ChipConfig) & 0xFFFFFFDD;
    pReg->DacRegs[PM2VDACRDDACControl] = 0x00;
  
    {
	/* Get the programmable clock values */
    	unsigned char m,n,p;
    	unsigned long clockused;
	
	/* Let's program the dot clock */
    	clockused = PM3DAC_CalculateClock(mode->Clock,
	    pGlint->RefClock, &m,&n,&p);
	pReg->DacRegs[PM3RD_DClk0PreScale] = m;
	pReg->DacRegs[PM3RD_DClk0FeedbackScale] = n;
	pReg->DacRegs[PM3RD_DClk0PostScale] = p;
    }

    pReg->DacRegs[PM2VDACRDIndexControl] = 0x00;

    if (pScrn->rgbBits == 8)
        pReg->DacRegs[PM2VDACRDMiscControl] = 0x01; /* 8bit DAC */
    else
	pReg->DacRegs[PM2VDACRDMiscControl] = 0x00; /* 6bit DAC */

    pReg->DacRegs[PM2VDACRDSyncControl] = 0x00;
    if (!(mode->Flags & V_PHSYNC))
        pReg->DacRegs[PM2VDACRDSyncControl] |= 0x01; /* invert hsync */
    if (!(mode->Flags & V_PVSYNC))
        pReg->DacRegs[PM2VDACRDSyncControl] |= 0x08; /* invert vsync */

#if 0 /* Currently commented out while testing Flat Panel support */
    pReg->DacRegs[PM2VDACRDDACControl] = 0x01;
    pReg->DacRegs[PM2VDACRDSyncControl] |= 0x40;
    pReg->glintRegs[VSConfiguration >> 3] = (GLINT_READ_REG(VSConfiguration) & 0xFFFFFFF8) | 0x06;
    pReg->glintRegs[VSBBase >> 3] = 1<<14;
#endif

    switch (pScrn->bitsPerPixel)
    {
    case 8:
	pReg->DacRegs[PM2VDACRDPixelSize] = 0x00;
	pReg->DacRegs[PM2VDACRDColorFormat] = 0x2E;
    	break;
    case 16:
	pReg->DacRegs[PM2VDACRDPixelSize] = 0x01;
	if (pScrn->depth == 15) 
	    pReg->DacRegs[PM2VDACRDColorFormat] = 0x61;
	else
	    pReg->DacRegs[PM2VDACRDColorFormat] = 0x70;
    	break;
    case 24:
	pReg->DacRegs[PM2VDACRDPixelSize] = 0x04;
	pReg->DacRegs[PM2VDACRDColorFormat] = 0x60;
    	break;
    case 32:
	pReg->DacRegs[PM2VDACRDPixelSize] = 0x02;
	pReg->DacRegs[PM2VDACRDColorFormat] = 0x20;
	if (pScrn->overlayFlags & OVERLAY_8_32_PLANAR) {
	    pReg->DacRegs[PM2VDACRDMiscControl] |= 0x10;
	    pReg->DacRegs[PM2VDACRDOverlayKey] = pScrn->colorKey;
	}
    	break;
    }

    return(TRUE);
}

void
Permedia3Save(ScrnInfoPtr pScrn, GLINTRegPtr glintReg)
{
    GLINTPtr pGlint = GLINTPTR(pScrn);
    int i;

    /* Permedia 3 memory Timings */
    glintReg->glintRegs[PM3MemBypassWriteMask >> 3] =
	    				GLINT_READ_REG(PM3MemBypassWriteMask);
    glintReg->glintRegs[PM3ByAperture1Mode >> 3] = 
	    				GLINT_READ_REG(PM3ByAperture1Mode);
    glintReg->glintRegs[PM3ByAperture2Mode >> 3] = 
	    				GLINT_READ_REG(PM3ByAperture2Mode);

    glintReg->glintRegs[ChipConfig >> 3] = GLINT_READ_REG(ChipConfig);
    glintReg->glintRegs[Aperture0 >> 3]  = GLINT_READ_REG(Aperture0);
    glintReg->glintRegs[Aperture1 >> 3]  = GLINT_READ_REG(Aperture1);

    glintReg->glintRegs[DFIFODis >> 3]  = GLINT_READ_REG(DFIFODis);
    glintReg->glintRegs[FIFODis >> 3]  = GLINT_READ_REG(FIFODis);
    glintReg->glintRegs[PMHTotal >> 3] = GLINT_READ_REG(PMHTotal);
    glintReg->glintRegs[PMHbEnd >> 3] = GLINT_READ_REG(PMHbEnd);
    glintReg->glintRegs[PMHbEnd >> 3] = GLINT_READ_REG(PMHgEnd);
    glintReg->glintRegs[PMScreenStride >> 3] = GLINT_READ_REG(PMScreenStride);
    glintReg->glintRegs[PMHsStart >> 3] = GLINT_READ_REG(PMHsStart);
    glintReg->glintRegs[PMHsEnd >> 3] = GLINT_READ_REG(PMHsEnd);
    glintReg->glintRegs[PMVTotal >> 3] = GLINT_READ_REG(PMVTotal);
    glintReg->glintRegs[PMVbEnd >> 3] = GLINT_READ_REG(PMVbEnd);
    glintReg->glintRegs[PMVsStart >> 3] = GLINT_READ_REG(PMVsStart);
    glintReg->glintRegs[PMVsEnd >> 3] = GLINT_READ_REG(PMVsEnd);
    glintReg->glintRegs[PMScreenBase >> 3] = GLINT_READ_REG(PMScreenBase);
    glintReg->glintRegs[PMVideoControl >> 3] = GLINT_READ_REG(PMVideoControl);
    glintReg->glintRegs[VClkCtl >> 3] = GLINT_READ_REG(VClkCtl);
#if 0 /* Currently commented out while testing Flat Panel support */
    glintReg->glintRegs[VSConfiguration >> 3] = GLINT_READ_REG(VSConfiguration);
    glintReg->glintRegs[VSBBase >> 3] = GLINT_READ_REG(VSBBase);
#endif

    for (i=0;i<768;i++) {
    	Permedia2ReadAddress(pScrn, i);
	glintReg->cmap[i] = Permedia2ReadData(pScrn);
    }

    glintReg->DacRegs[PM2VDACRDIndexControl] = 
				Permedia2vInIndReg(pScrn, PM2VDACRDIndexControl);
    glintReg->DacRegs[PM2VDACRDOverlayKey] = 
				Permedia2vInIndReg(pScrn, PM2VDACRDOverlayKey);
    glintReg->DacRegs[PM2VDACRDSyncControl] = 
				Permedia2vInIndReg(pScrn, PM2VDACRDSyncControl);
    glintReg->DacRegs[PM2VDACRDMiscControl] = 
				Permedia2vInIndReg(pScrn, PM2VDACRDMiscControl);
    glintReg->DacRegs[PM2VDACRDDACControl] = 
				Permedia2vInIndReg(pScrn, PM2VDACRDDACControl);
    glintReg->DacRegs[PM2VDACRDPixelSize] = 
				Permedia2vInIndReg(pScrn, PM2VDACRDPixelSize);
    glintReg->DacRegs[PM2VDACRDColorFormat] = 
				Permedia2vInIndReg(pScrn, PM2VDACRDColorFormat);

    glintReg->DacRegs[PM2VDACRDDClk0PreScale] = Permedia2vInIndReg(pScrn, PM2VDACRDDClk0PreScale);
    glintReg->DacRegs[PM2VDACRDDClk0FeedbackScale] = Permedia2vInIndReg(pScrn, PM2VDACRDDClk0FeedbackScale);
    glintReg->DacRegs[PM2VDACRDDClk0PostScale] = Permedia2vInIndReg(pScrn, PM2VDACRDDClk0PostScale);
}

void
Permedia3Restore(ScrnInfoPtr pScrn, GLINTRegPtr glintReg)
{
    GLINTPtr pGlint = GLINTPTR(pScrn);
    CARD32 temp;
    int i;

#if 0
    GLINT_SLOW_WRITE_REG(0, ResetStatus);
    while(GLINT_READ_REG(ResetStatus) != 0) {
	xf86MsgVerb(X_INFO, 2, "Resetting Engine - Please Wait.\n");
    };
#endif

    /* Permedia 3 memory Timings */
    GLINT_SLOW_WRITE_REG(glintReg->glintRegs[PM3MemBypassWriteMask >> 3],
		    					PM3MemBypassWriteMask);
    GLINT_SLOW_WRITE_REG(glintReg->glintRegs[PM3ByAperture1Mode >> 3],
	    						PM3ByAperture1Mode);
    GLINT_SLOW_WRITE_REG(glintReg->glintRegs[PM3ByAperture2Mode >> 3],
	    						PM3ByAperture2Mode);

    GLINT_SLOW_WRITE_REG(glintReg->glintRegs[ChipConfig >> 3], ChipConfig);
    GLINT_SLOW_WRITE_REG(glintReg->glintRegs[Aperture0 >> 3], Aperture0);
    GLINT_SLOW_WRITE_REG(glintReg->glintRegs[Aperture1 >> 3], Aperture1);
    GLINT_SLOW_WRITE_REG(glintReg->glintRegs[DFIFODis >> 3], DFIFODis);
    GLINT_SLOW_WRITE_REG(glintReg->glintRegs[FIFODis >> 3], FIFODis);
    GLINT_SLOW_WRITE_REG(glintReg->glintRegs[PMVideoControl >> 3], 
								PMVideoControl);
    GLINT_SLOW_WRITE_REG(glintReg->glintRegs[PMHbEnd >> 3], PMHgEnd);
    GLINT_SLOW_WRITE_REG(glintReg->glintRegs[PMScreenBase >> 3], PMScreenBase);
    GLINT_SLOW_WRITE_REG(glintReg->glintRegs[VClkCtl >> 3], VClkCtl);
    GLINT_SLOW_WRITE_REG(glintReg->glintRegs[PMScreenStride >> 3], 
								PMScreenStride);
    GLINT_SLOW_WRITE_REG(glintReg->glintRegs[PMHTotal >> 3], PMHTotal);
    GLINT_SLOW_WRITE_REG(glintReg->glintRegs[PMHbEnd >> 3], PMHbEnd);
    GLINT_SLOW_WRITE_REG(glintReg->glintRegs[PMHsStart >> 3], PMHsStart);
    GLINT_SLOW_WRITE_REG(glintReg->glintRegs[PMHsEnd >> 3], PMHsEnd);
    GLINT_SLOW_WRITE_REG(glintReg->glintRegs[PMVTotal >> 3], PMVTotal);
    GLINT_SLOW_WRITE_REG(glintReg->glintRegs[PMVbEnd >> 3], PMVbEnd);
    GLINT_SLOW_WRITE_REG(glintReg->glintRegs[PMVsStart >> 3], PMVsStart);
    GLINT_SLOW_WRITE_REG(glintReg->glintRegs[PMVsEnd >> 3], PMVsEnd);
#if 0 /* Currently commented out while testing Flat Panel support */
    GLINT_SLOW_WRITE_REG(glintReg->glintRegs[VSConfiguration >> 3], VSConfiguration);
    GLINT_SLOW_WRITE_REG(glintReg->glintRegs[VSBBase >> 3], VSBBase);
#endif

    Permedia2vOutIndReg(pScrn, PM2VDACRDIndexControl, 0x00, 
				glintReg->DacRegs[PM2VDACRDIndexControl]);
    Permedia2vOutIndReg(pScrn, PM2VDACRDOverlayKey, 0x00, 
				glintReg->DacRegs[PM2VDACRDOverlayKey]);
    Permedia2vOutIndReg(pScrn, PM2VDACRDSyncControl, 0x00, 
				glintReg->DacRegs[PM2VDACRDSyncControl]);
    Permedia2vOutIndReg(pScrn, PM2VDACRDMiscControl, 0x00, 
				glintReg->DacRegs[PM2VDACRDMiscControl]);
    Permedia2vOutIndReg(pScrn, PM2VDACRDDACControl, 0x00, 
				glintReg->DacRegs[PM2VDACRDDACControl]);
    Permedia2vOutIndReg(pScrn, PM2VDACRDPixelSize, 0x00, 
				glintReg->DacRegs[PM2VDACRDPixelSize]);
    Permedia2vOutIndReg(pScrn, PM2VDACRDColorFormat, 0x00, 
				glintReg->DacRegs[PM2VDACRDColorFormat]);

    for (i=0;i<768;i++) {
    	Permedia2WriteAddress(pScrn, i);
	Permedia2WriteData(pScrn, glintReg->cmap[i]);
    }

    temp = Permedia2vInIndReg(pScrn, PM2VDACIndexClockControl) & 0xFC;
    Permedia2vOutIndReg(pScrn, PM2VDACRDDClk0PreScale, 0x00, 
	glintReg->DacRegs[PM2VDACRDDClk0PreScale]);
    Permedia2vOutIndReg(pScrn, PM2VDACRDDClk0FeedbackScale, 0x00, 
	glintReg->DacRegs[PM2VDACRDDClk0FeedbackScale]);
    Permedia2vOutIndReg(pScrn, PM2VDACRDDClk0PostScale, 0x00, 
	glintReg->DacRegs[PM2VDACRDDClk0PostScale]);
    Permedia2vOutIndReg(pScrn, PM2VDACIndexClockControl, 0x00, temp|0x03);
}

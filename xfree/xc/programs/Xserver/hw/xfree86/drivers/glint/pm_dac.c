/*
 * Copyright 1997,1998 by Alan Hourihane <alanh@fairlite.demon.co.uk>
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
/* $XFree86: xc/programs/Xserver/hw/xfree86/drivers/glint/pm_dac.c,v 1.7 1999/03/28 15:32:39 dawes Exp $ */

#include "xf86.h"
#include "xf86_OSproc.h"
#include "xf86_ansic.h"

#include "xf86PciInfo.h"
#include "xf86Pci.h"

#include "IBM.h"
#include "glint_regs.h"
#include "glint.h"

static int
Shiftbpp(ScrnInfoPtr pScrn, int value)
{
    GLINTPtr pGlint = GLINTPTR(pScrn);
    /* shift horizontal timings for 64bit VRAM's or 32bit SGRAMs */
    int logbytesperaccess = 2;
	
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

Bool
PermediaInit(ScrnInfoPtr pScrn, DisplayModePtr mode)
{
    GLINTPtr pGlint = GLINTPTR(pScrn);
    GLINTRegPtr pReg = &pGlint->ModeReg;
    RamDacHWRecPtr pIBM = RAMDACHWPTR(pScrn);
    RamDacRegRecPtr ramdacReg = &pIBM->ModeReg;
    CARD32 temp1, temp2, temp3, temp4;

    pReg->glintRegs[Aperture0 >> 3] = 0;
    pReg->glintRegs[Aperture1 >> 3] = 0;
    pReg->glintRegs[PMFramebufferWriteMask >> 3] = 0xFFFFFFFF;
    pReg->glintRegs[PMBypassWriteMask >> 3] = 0xFFFFFFFF;

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
    pReg->glintRegs[PMHgEnd >> 3] = Shiftbpp(pScrn, mode->CrtcHTotal - 
							mode->CrtcHDisplay);
    pReg->glintRegs[PMScreenStride >> 3] = 
					Shiftbpp(pScrn,pScrn->displayWidth>>1);

    pReg->glintRegs[PMVTotal >> 3] = mode->CrtcVTotal;
    pReg->glintRegs[PMVsEnd >> 3] = temp2 + temp4;
    pReg->glintRegs[PMVsStart >> 3] = temp2;
    pReg->glintRegs[PMVbEnd >> 3] = mode->CrtcVTotal - mode->CrtcVDisplay;

    pReg->glintRegs[PMVideoControl >> 3] = 
 	    (((mode->Flags & V_PHSYNC) ? 0x1 : 0x3) << 3) |  
 	    (((mode->Flags & V_PVSYNC) ? 0x1 : 0x3) << 5) | 1; 

    pReg->glintRegs[VClkCtl >> 3] = 3;
    pReg->glintRegs[PMScreenBase >> 3] = 0;
    pReg->glintRegs[PMHTotal >> 3] -= 1; /* PMHTotal */
    pReg->glintRegs[PMHsStart >> 3] -= 1; /* PMHsStart */
    pReg->glintRegs[PMVTotal >> 3] -= 1; /* PMVTotal */
    pReg->glintRegs[ChipConfig >> 3] = GLINT_READ_REG(ChipConfig) & 0xFFFFFFFD;

    {
	/* Get the programmable clock values */
    	unsigned long m=0,n=0,p=0,c=0;
    	unsigned long clock;
	
    	clock = IBMramdac526CalculateMNPCForClock(pGlint->RefClock, mode->Clock,
			1, pGlint->MinClock, pGlint->MaxClock, &m, &n, &p, &c);
			
	ramdacReg->DacRegs[IBMRGB_m0] = m;
	ramdacReg->DacRegs[IBMRGB_n0] = n;
	ramdacReg->DacRegs[IBMRGB_p0] = p;
	ramdacReg->DacRegs[IBMRGB_c0] = c;

	ramdacReg->DacRegs[IBMRGB_pll_ctrl1] = 0x05;
	ramdacReg->DacRegs[IBMRGB_pll_ctrl2] = 0x00;

	p = 1;
    	clock = IBMramdac526CalculateMNPCForClock(pGlint->RefClock, mode->Clock,
			0, pGlint->MinClock, pGlint->MaxClock, &m, &n, &p, &c);

	ramdacReg->DacRegs[IBMRGB_sysclk] = 0x05;
	ramdacReg->DacRegs[IBMRGB_sysclk_m] = m;
	ramdacReg->DacRegs[IBMRGB_sysclk_n] = n;
	ramdacReg->DacRegs[IBMRGB_sysclk_p] = p;
	ramdacReg->DacRegs[IBMRGB_sysclk_c] = c;
    }

    ramdacReg->DacRegs[IBMRGB_misc1] = SENS_DSAB_DISABLE | VRAM_SIZE_32;
    ramdacReg->DacRegs[IBMRGB_misc2] = COL_RES_8BIT | PORT_SEL_VRAM;
    if (pScrn->depth >= 24)
	ramdacReg->DacRegs[IBMRGB_misc2] |= PCLK_SEL_LCLK;
    else 
	ramdacReg->DacRegs[IBMRGB_misc2] |= PCLK_SEL_PLL;
    ramdacReg->DacRegs[IBMRGB_misc_clock] = 1;
    ramdacReg->DacRegs[IBMRGB_sync] = 0;
    ramdacReg->DacRegs[IBMRGB_hsync_pos] = 0;
    ramdacReg->DacRegs[IBMRGB_pwr_mgmt] = 0;
    ramdacReg->DacRegs[IBMRGB_dac_op] = 0;
    ramdacReg->DacRegs[IBMRGB_pal_ctrl] = 0;

    (*pGlint->RamDac->SetBpp)(pScrn, ramdacReg);

    return(TRUE);
}

void
PermediaSave(ScrnInfoPtr pScrn, GLINTRegPtr glintReg)
{
    GLINTPtr pGlint = GLINTPTR(pScrn);

    glintReg->glintRegs[Aperture0 >> 3]  = GLINT_READ_REG(Aperture0);
    glintReg->glintRegs[Aperture1 >> 3]  = GLINT_READ_REG(Aperture1);
    glintReg->glintRegs[PMFramebufferWriteMask] = 
					GLINT_READ_REG(PMFramebufferWriteMask);
    glintReg->glintRegs[PMBypassWriteMask >> 3] = 
					GLINT_READ_REG(PMBypassWriteMask);
    glintReg->glintRegs[DFIFODis >> 3]  = GLINT_READ_REG(DFIFODis);
    glintReg->glintRegs[FIFODis >> 3]  = GLINT_READ_REG(FIFODis);

    glintReg->glintRegs[PMHTotal >> 3] = GLINT_READ_REG(PMHTotal);
    glintReg->glintRegs[PMHgEnd >> 3] = GLINT_READ_REG(PMHbEnd);
    glintReg->glintRegs[PMHgEnd >> 3] = GLINT_READ_REG(PMHgEnd);
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
    glintReg->glintRegs[ChipConfig >> 3] = GLINT_READ_REG(ChipConfig);
}

void
PermediaRestore(ScrnInfoPtr pScrn, GLINTRegPtr glintReg)
{
    GLINTPtr pGlint = GLINTPTR(pScrn);

#if 0
    GLINT_SLOW_WRITE_REG(0, ResetStatus);
    while(GLINT_READ_REG(ResetStatus) != 0) {
	xf86MsgVerb(X_INFO, 2, "Resetting Engine - Please Wait.\n");
    };
#endif

    GLINT_SLOW_WRITE_REG(glintReg->glintRegs[ChipConfig >> 3], ChipConfig);
    GLINT_SLOW_WRITE_REG(glintReg->glintRegs[DFIFODis >> 3], DFIFODis);
    GLINT_SLOW_WRITE_REG(glintReg->glintRegs[FIFODis >> 3], FIFODis);
    GLINT_SLOW_WRITE_REG(glintReg->glintRegs[Aperture0 >> 3], Aperture0);
    GLINT_SLOW_WRITE_REG(glintReg->glintRegs[Aperture1 >> 3], Aperture1);
    GLINT_SLOW_WRITE_REG(glintReg->glintRegs[PMFramebufferWriteMask >> 3], 
							PMFramebufferWriteMask);
    GLINT_SLOW_WRITE_REG(glintReg->glintRegs[PMBypassWriteMask >> 3], 
							PMBypassWriteMask);

    GLINT_SLOW_WRITE_REG(glintReg->glintRegs[PMVideoControl >> 3], 
								PMVideoControl);
    GLINT_SLOW_WRITE_REG(glintReg->glintRegs[PMHgEnd >> 3], PMHgEnd);
    GLINT_SLOW_WRITE_REG(glintReg->glintRegs[VClkCtl >> 3], VClkCtl);
    GLINT_SLOW_WRITE_REG(glintReg->glintRegs[PMHTotal >> 3], PMHTotal);
    GLINT_SLOW_WRITE_REG(glintReg->glintRegs[PMHgEnd >> 3], PMHbEnd);
    GLINT_SLOW_WRITE_REG(glintReg->glintRegs[PMHsStart >> 3], PMHsStart);
    GLINT_SLOW_WRITE_REG(glintReg->glintRegs[PMHsEnd >> 3], PMHsEnd);
    GLINT_SLOW_WRITE_REG(glintReg->glintRegs[PMVTotal >> 3], PMVTotal);
    GLINT_SLOW_WRITE_REG(glintReg->glintRegs[PMVbEnd >> 3], PMVbEnd);
    GLINT_SLOW_WRITE_REG(glintReg->glintRegs[PMVsStart >> 3], PMVsStart);
    GLINT_SLOW_WRITE_REG(glintReg->glintRegs[PMVsEnd >> 3], PMVsEnd);
    GLINT_SLOW_WRITE_REG(glintReg->glintRegs[PMScreenBase >> 3], PMScreenBase);
    GLINT_SLOW_WRITE_REG(glintReg->glintRegs[PMScreenStride >> 3], 
								PMScreenStride);
}

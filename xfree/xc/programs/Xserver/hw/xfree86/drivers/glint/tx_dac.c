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
/* $XFree86: xc/programs/Xserver/hw/xfree86/drivers/glint/tx_dac.c,v 1.10 2000/05/10 18:55:30 alanh Exp $ */

#include "xf86.h"
#include "xf86_OSproc.h"
#include "xf86_ansic.h"

#include "xf86PciInfo.h"
#include "xf86Pci.h"

#include "IBM.h"
#include "TI.h"
#include "glint_regs.h"
#include "glint.h"

static int
Shiftbpp(ScrnInfoPtr pScrn, int value)
{
    GLINTPtr pGlint = GLINTPTR(pScrn);
    int logbytesperaccess;

    if ( (pGlint->RamDac->RamDacType == (IBM640_RAMDAC)) ||
         (pGlint->RamDac->RamDacType == (TI3030_RAMDAC)) )
    	logbytesperaccess = 4;
    else
    	logbytesperaccess = 3;
	
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
TXInit(ScrnInfoPtr pScrn, DisplayModePtr mode)
{
    GLINTPtr pGlint = GLINTPTR(pScrn);
    GLINTRegPtr pReg = &pGlint->ModeReg;
    RamDacHWRecPtr pIBM = RAMDACHWPTR(pScrn);
    RamDacRegRecPtr ramdacReg = &pIBM->ModeReg;
    CARD32 temp1, temp2, temp3, temp4;

    pReg->glintRegs[Aperture0 >> 3] = 0;
    pReg->glintRegs[Aperture1 >> 3] = 0;

    if (pGlint->UsePCIRetry) {
	pReg->glintRegs[DFIFODis >> 3] = GLINT_READ_REG(DFIFODis) | 0x01;
    	if (pGlint->Chipset == PCI_VENDOR_3DLABS_CHIP_GAMMA)
	    pReg->glintRegs[FIFODis >> 3] = GLINT_READ_REG(FIFODis) | 0x01;
	else
	    pReg->glintRegs[FIFODis >> 3] = GLINT_READ_REG(FIFODis) | 0x03;
    } else {
	pReg->glintRegs[DFIFODis >> 3] = GLINT_READ_REG(DFIFODis) & 0xFFFFFFFE;
	pReg->glintRegs[FIFODis >> 3] = GLINT_READ_REG(FIFODis) | 0x01;
    }

    temp1 = mode->CrtcHSyncStart - mode->CrtcHDisplay;
    temp2 = mode->CrtcVSyncStart - mode->CrtcVDisplay;
    temp3 = mode->CrtcHSyncEnd - mode->CrtcHSyncStart;
    temp4 = mode->CrtcVSyncEnd - mode->CrtcVSyncStart;

    pReg->glintRegs[VTGHLimit >> 3] = Shiftbpp(pScrn,mode->CrtcHTotal);
    pReg->glintRegs[VTGHSyncEnd >> 3] = Shiftbpp(pScrn, temp1 + temp3);
    pReg->glintRegs[VTGHSyncStart >> 3] = Shiftbpp(pScrn, temp1);
    pReg->glintRegs[VTGHBlankEnd >> 3] = Shiftbpp(pScrn, mode->CrtcHTotal -
							mode->CrtcHDisplay);

    pReg->glintRegs[VTGVLimit >> 3] = mode->CrtcVTotal;
    pReg->glintRegs[VTGVSyncEnd >> 3] = temp2 + temp4;
    pReg->glintRegs[VTGVSyncStart >> 3] = temp2;
    pReg->glintRegs[VTGVBlankEnd >> 3] = mode->CrtcVTotal - mode->CrtcVDisplay;

    pReg->glintRegs[VTGPolarity >> 3] = (((mode->Flags & V_PHSYNC) ? 0:2)<<2) |
			     ((mode->Flags & V_PVSYNC) ? 0 : 2) | (0xb0);

    pReg->glintRegs[VClkCtl >> 3] = 0; 
    pReg->glintRegs[VTGVGateStart >> 3] = pReg->glintRegs[VTGVBlankEnd>>3] - 1; 
    pReg->glintRegs[VTGVGateEnd >> 3] = pReg->glintRegs[VTGVBlankEnd>>3];
    /*
     * tell DAC to use the ICD chip clock 0 as ref clock 
     * and set up some more video timining generator registers
     */
    pReg->glintRegs[VTGSerialClk >> 3] = 0x05;

    /* This is ugly */
    if (pGlint->UseFireGL3000) {
	pReg->glintRegs[VTGHGateStart >> 3] = 
					pReg->glintRegs[VTGHBlankEnd>>3] - 1;
	pReg->glintRegs[VTGHGateEnd >> 3] = pReg->glintRegs[VTGHLimit>>3] - 1;
	pReg->glintRegs[FBModeSel >> 3] = 0x907;
	pReg->glintRegs[VTGModeCtl >> 3] = 0x00;
    } else {
	pReg->glintRegs[VTGHGateStart >> 3] = 
					pReg->glintRegs[VTGHBlankEnd>>3] - 2;
	pReg->glintRegs[VTGHGateEnd >> 3] = pReg->glintRegs[VTGHLimit>>3] - 2;
	pReg->glintRegs[FBModeSel >> 3] = 0x0A07;
	pReg->glintRegs[VTGModeCtl >> 3] = 0x44;
    }

    /* Override FBModeSel for 300SX chip */
    if (pGlint->Chipset == PCI_VENDOR_3DLABS_CHIP_300SX) {
	switch (pScrn->bitsPerPixel) {
	    case 8:
		pReg->glintRegs[FBModeSel >> 3] = 0x905;
		break;
	    case 16:
		pReg->glintRegs[FBModeSel >> 3] = 0x903;
		break;
	    case 32:
		pReg->glintRegs[FBModeSel >> 3] = 0x901;
		break;
	}
    }

    switch (pGlint->RamDac->RamDacType) {
    case IBM526DB_RAMDAC:
    case IBM526_RAMDAC:
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
    ramdacReg->DacRegs[IBMRGB_misc1] = SENS_DSAB_DISABLE | VRAM_SIZE_64;
    ramdacReg->DacRegs[IBMRGB_misc2] = COL_RES_8BIT|PORT_SEL_VRAM|PCLK_SEL_PLL;
    ramdacReg->DacRegs[IBMRGB_misc3] = 0;
    ramdacReg->DacRegs[IBMRGB_misc_clock] = 1;
    ramdacReg->DacRegs[IBMRGB_sync] = 0;
    ramdacReg->DacRegs[IBMRGB_hsync_pos] = 0;
    ramdacReg->DacRegs[IBMRGB_pwr_mgmt] = 0;
    ramdacReg->DacRegs[IBMRGB_dac_op] = 0;
    ramdacReg->DacRegs[IBMRGB_pal_ctrl] = 0;

    break;
    case IBM640_RAMDAC:
    {
	/* Get the programmable clock values */
    	unsigned long m=0,n=0,p=0,c=0;
    	unsigned long clock;

    	clock = IBMramdac640CalculateMNPCForClock(pGlint->RefClock, mode->Clock,
			1, pGlint->MinClock, pGlint->MaxClock, &m, &n, &p, &c);

	ramdacReg->DacRegs[RGB640_PLL_N] = n;
	ramdacReg->DacRegs[RGB640_PLL_M] = m;
	ramdacReg->DacRegs[RGB640_PLL_P] = p<<1;
	ramdacReg->DacRegs[RGB640_PLL_CTL] = c | IBM640_PLL_EN;
	ramdacReg->DacRegs[RGB640_AUX_PLL_CTL] = 0; /* Disable AUX PLL */
    }
    ramdacReg->DacRegs[RGB640_PIXEL_INTERLEAVE] = 0x00;
    ramdacReg->DacRegs[RGB640_VGA_CONTROL] = IBM640_RDBK | IBM640_VRAM;
    if (pScrn->rgbBits == 8) 
    	ramdacReg->DacRegs[RGB640_VGA_CONTROL] |= IBM640_PSIZE8;
    ramdacReg->DacRegs[RGB640_DAC_CONTROL] = IBM640_DACENBL | IBM640_SHUNT;
    ramdacReg->DacRegs[RGB640_OUTPUT_CONTROL] = IBM640_WATCTL;
    ramdacReg->DacRegs[RGB640_SYNC_CONTROL] = 0x00;
    ramdacReg->DacRegs[RGB640_VRAM_MASK0] = 0xFF;
    ramdacReg->DacRegs[RGB640_VRAM_MASK1] = 0xFF; 
    ramdacReg->DacRegs[RGB640_VRAM_MASK2] = 0x0F; 
    
    pReg->glintRegs[VTGModeCtl >> 3] = 0x04;
    break;

    case TI3026_RAMDAC:
    case TI3030_RAMDAC:
    {
	/* Get the programmable clock values */
	unsigned long m=0,n=0,p=0;
	unsigned long clock;
	int count;
	unsigned long q, status, VCO;

	clock = TIramdacCalculateMNPForClock(pGlint->RefClock, 
		mode->Clock, 1, pGlint->MinClock, pGlint->MaxClock, &m, &n, &p);

	ramdacReg->DacRegs[TIDAC_PIXEL_N] = ((n & 0x3f) | 0xC0);
	ramdacReg->DacRegs[TIDAC_PIXEL_M] =  (m & 0x3f)        ;
	ramdacReg->DacRegs[TIDAC_PIXEL_P] = ((p & 0x03) | 0xbc);
	ramdacReg->DacRegs[TIDAC_PIXEL_VALID] = TRUE;

    	if (pGlint->RamDac->RamDacType == (TI3026_RAMDAC))
            n = 65 - ((64 << 2) / pScrn->bitsPerPixel);
	else
            n = 65 - ((128 << 2) / pScrn->bitsPerPixel);
	m = 61;
	p = 0;
	for (q = 0; q < 8; q++) {
	    if (q > 0) p = 3;
	    for ( ; p < 4; p++) {
		VCO = ((clock * (q + 1) * (65 - m)) / (65 - n)) << (p + 1);
		if (VCO >= 110000) { break; }
	    }
	    if (VCO >= 110000) { break; }
	}
	ramdacReg->DacRegs[TIDAC_clock_ctrl] = (q | 0x38);

	ramdacReg->DacRegs[TIDAC_LOOP_N] = ((n & 0x3f) | 0xC0);
	ramdacReg->DacRegs[TIDAC_LOOP_M] =  (m & 0x3f)        ;
	ramdacReg->DacRegs[TIDAC_LOOP_P] = ((p & 0x03) | 0xF0);
	ramdacReg->DacRegs[TIDAC_LOOP_VALID] = TRUE;
    }
    if (pGlint->RamDac->RamDacType == (TI3030_RAMDAC))
    	pReg->glintRegs[VTGModeCtl >> 3] = 0x04;
    break;
    }

    /* Now use helper routines to setup bpp for this driver */
    (*pGlint->RamDac->SetBpp)(pScrn, ramdacReg);

    return(TRUE);
}

void
TXSave(ScrnInfoPtr pScrn, GLINTRegPtr glintReg)
{
    GLINTPtr pGlint = GLINTPTR(pScrn);

    glintReg->glintRegs[Aperture0 >> 3]  = GLINT_READ_REG(Aperture0);
    glintReg->glintRegs[Aperture1 >> 3]  = GLINT_READ_REG(Aperture1);

    if ((pGlint->Chipset == PCI_VENDOR_3DLABS_CHIP_DELTA) ||
    	(pGlint->Chipset == PCI_VENDOR_3DLABS_CHIP_GAMMA))
    	glintReg->glintRegs[DFIFODis >> 3]  = GLINT_READ_REG(DFIFODis);

    if (pGlint->Chipset != PCI_VENDOR_3DLABS_CHIP_300SX) {
    	glintReg->glintRegs[FIFODis >> 3]  = GLINT_READ_REG(FIFODis);
    	glintReg->glintRegs[VTGModeCtl >> 3] = GLINT_READ_REG(VTGModeCtl);
    }

    glintReg->glintRegs[VClkCtl >> 3] = GLINT_READ_REG(VClkCtl);
    glintReg->glintRegs[VTGPolarity >> 3] = GLINT_READ_REG(VTGPolarity);
    glintReg->glintRegs[VTGHLimit >> 3] = GLINT_READ_REG(VTGHLimit);
    glintReg->glintRegs[VTGHBlankEnd >> 3] = GLINT_READ_REG(VTGHBlankEnd);
    glintReg->glintRegs[VTGHSyncStart >> 3] = GLINT_READ_REG(VTGHSyncStart);
    glintReg->glintRegs[VTGHSyncEnd >> 3] = GLINT_READ_REG(VTGHSyncEnd);
    glintReg->glintRegs[VTGVLimit >> 3] = GLINT_READ_REG(VTGVLimit);
    glintReg->glintRegs[VTGVBlankEnd >> 3] = GLINT_READ_REG(VTGVBlankEnd);
    glintReg->glintRegs[VTGVSyncStart >> 3] = GLINT_READ_REG(VTGVSyncStart);
    glintReg->glintRegs[VTGVSyncEnd >> 3] = GLINT_READ_REG(VTGVSyncEnd);
    glintReg->glintRegs[VTGVGateStart >> 3] = GLINT_READ_REG(VTGVGateStart);
    glintReg->glintRegs[VTGVGateEnd >> 3] = GLINT_READ_REG(VTGVGateEnd);
    glintReg->glintRegs[VTGSerialClk >> 3] = GLINT_READ_REG(VTGSerialClk);
    glintReg->glintRegs[FBModeSel >> 3] = GLINT_READ_REG(FBModeSel);
    glintReg->glintRegs[VTGHGateStart >> 3] = GLINT_READ_REG(VTGHGateStart);
    glintReg->glintRegs[VTGHGateEnd >> 3] = GLINT_READ_REG(VTGHGateEnd);
}

void
TXRestore(ScrnInfoPtr pScrn, GLINTRegPtr glintReg)
{
    GLINTPtr pGlint = GLINTPTR(pScrn);

    GLINT_SLOW_WRITE_REG(glintReg->glintRegs[Aperture0 >> 3], Aperture0);
    GLINT_SLOW_WRITE_REG(glintReg->glintRegs[Aperture1 >> 3], Aperture1);

    if ((pGlint->Chipset == PCI_VENDOR_3DLABS_CHIP_DELTA) ||
    	(pGlint->Chipset == PCI_VENDOR_3DLABS_CHIP_GAMMA))
    	GLINT_SLOW_WRITE_REG(glintReg->glintRegs[DFIFODis >> 3], DFIFODis);

    if (pGlint->Chipset != PCI_VENDOR_3DLABS_CHIP_300SX) {
    	GLINT_SLOW_WRITE_REG(glintReg->glintRegs[FIFODis >> 3], FIFODis);
    	GLINT_SLOW_WRITE_REG(glintReg->glintRegs[VTGModeCtl >> 3], VTGModeCtl);
    }

    GLINT_SLOW_WRITE_REG(glintReg->glintRegs[VTGPolarity >> 3], VTGPolarity);
    GLINT_SLOW_WRITE_REG(glintReg->glintRegs[VClkCtl >> 3], VClkCtl);
    GLINT_SLOW_WRITE_REG(glintReg->glintRegs[VTGSerialClk >> 3], VTGSerialClk);
    GLINT_SLOW_WRITE_REG(glintReg->glintRegs[VTGHLimit >> 3], VTGHLimit);
    GLINT_SLOW_WRITE_REG(glintReg->glintRegs[VTGHSyncStart >> 3],VTGHSyncStart);
    GLINT_SLOW_WRITE_REG(glintReg->glintRegs[VTGHSyncEnd >> 3], VTGHSyncEnd);
    GLINT_SLOW_WRITE_REG(glintReg->glintRegs[VTGHBlankEnd >> 3], VTGHBlankEnd);
    GLINT_SLOW_WRITE_REG(glintReg->glintRegs[VTGVLimit >> 3], VTGVLimit);
    GLINT_SLOW_WRITE_REG(glintReg->glintRegs[VTGVSyncStart >> 3],VTGVSyncStart);
    GLINT_SLOW_WRITE_REG(glintReg->glintRegs[VTGVSyncEnd >> 3], VTGVSyncEnd);
    GLINT_SLOW_WRITE_REG(glintReg->glintRegs[VTGVBlankEnd >> 3], VTGVBlankEnd);
    GLINT_SLOW_WRITE_REG(glintReg->glintRegs[VTGVGateStart >> 3],VTGVGateStart);
    GLINT_SLOW_WRITE_REG(glintReg->glintRegs[VTGVGateEnd >> 3], VTGVGateEnd);
    GLINT_SLOW_WRITE_REG(glintReg->glintRegs[FBModeSel >> 3], FBModeSel);
    GLINT_SLOW_WRITE_REG(glintReg->glintRegs[VTGHGateStart >> 3],VTGHGateStart);
    GLINT_SLOW_WRITE_REG(glintReg->glintRegs[VTGHGateEnd >> 3], VTGHGateEnd);
}

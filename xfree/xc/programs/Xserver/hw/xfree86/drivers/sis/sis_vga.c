/*
 * Copyright 1998,1999 by Alan Hourihane, Wigan, England.
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
 * Authors:  Alan Hourihane, alanh@fairlite.demon.co.uk
 *           Mike Chapman <mike@paranoia.com>, 
 *           Juanjo Santamarta <santamarta@ctv.es>, 
 *           Mitani Hiroshi <hmitani@drl.mei.co.jp> 
 *           David Thomas <davtom@dream.org.uk>. 
 */
/* $XFree86: xc/programs/Xserver/hw/xfree86/drivers/sis/sis_vga.c,v 1.12 2002/01/17 09:57:30 eich Exp $ */

#include "xf86.h"
#include "xf86_OSproc.h"
#include "xf86_ansic.h"
#include "xf86Version.h"
#include "xf86PciInfo.h"
#include "xf86Pci.h"

#include "sis.h"
#include "sis_regs.h"
#include "sis_dac.h"

#define Midx    0
#define Nidx    1
#define VLDidx  2
#define Pidx    3
#define PSNidx  4
#define Fref 14318180
/* stability constraints for internal VCO -- MAX_VCO also determines
 * the maximum Video pixel clock */
#define MIN_VCO Fref
#define MAX_VCO 135000000
#define MAX_VCO_5597 353000000
#define MAX_PSN 0 /* no pre scaler for this chip */
#define TOLERANCE 0.01  /* search smallest M and N in this tolerance */


void SISVGAPreInit(ScrnInfoPtr pScrn);
static  Bool    SISInit(ScrnInfoPtr pScrn, DisplayModePtr mode);
static  Bool    SIS300Init(ScrnInfoPtr pScrn, DisplayModePtr mode);

static Bool
SISInit(ScrnInfoPtr pScrn, DisplayModePtr mode)
{
    SISPtr pSiS = SISPTR(pScrn);
    SISRegPtr pReg = &pSiS->ModeReg;
    vgaRegPtr   vgaReg = &VGAHWPTR(pScrn)->ModeReg;
    int gap, safetymargin, MemBand;
    int vgaIOBase;
    unsigned char temp;
    int Base,mclk;
    int offset;
    int clock = mode->Clock;
    unsigned int vclk[5];
    unsigned short CRT_CPUthresholdLow ;
    unsigned short CRT_CPUthresholdHigh ;
    unsigned short CRT_ENGthreshold ;

    int num, denum, div, sbit, scale;

    PDEBUG(xf86DrvMsgVerb(pScrn->scrnIndex, X_INFO, 3, "SISInit()\n"));
    vgaHWGetIOBase(VGAHWPTR(pScrn));
    vgaIOBase = VGAHWPTR(pScrn)->IOBase;

    (*pSiS->SiSSave)(pScrn, pReg);

    pSiS->scrnOffset = pScrn->displayWidth * pScrn->bitsPerPixel / 8;

    outw(VGA_SEQ_INDEX, 0x8605);

    pReg->sisRegs3C4[0x06] &= 0x01;
    
    if ((mode->Flags & V_INTERLACE)==0)  {
        offset = pScrn->displayWidth >> 3;
        pReg->sisRegs3C4[0x06] &= 0xDF;
    } else  {
        offset = pScrn->displayWidth >> 2;
        pReg->sisRegs3C4[0x06] |= 0x20;
    }

    /* Enable Linear */
    switch (pSiS->Chipset)  {
    case PCI_CHIP_SIS5597:
    case PCI_CHIP_SIS6326:
    case PCI_CHIP_SIS530:
        pReg->sisRegs3C4[BankReg] |= 0x82;
        pReg->sisRegs3C4[0x0C] |= 0xA0;
        pReg->sisRegs3C4[0x0B] |= 0x60;
        break;
    default:
        pReg->sisRegs3C4[BankReg] |= 0x82;
    }

    switch (pScrn->bitsPerPixel) {
        case 8:
            break;
        case 15:
            offset <<= 1;
            pReg->sisRegs3C4[BankReg] |= 0x04;
            break;
        case 16:
            offset <<= 1;
            pReg->sisRegs3C4[BankReg] |= 0x08;
            break;
        case 24:
            offset += (offset << 1);
            pReg->sisRegs3C4[BankReg] |= 0x10;
            pReg->sisRegs3C4[MMIOEnable] |= 0x90;
            break;
        case 32:
            offset <<= 2;
            if (pSiS->Chipset == PCI_CHIP_SIS530)  {
                    pReg->sisRegs3C4[BankReg] |= 0x10;
                    pReg->sisRegs3C4[MMIOEnable] |= 0x90;
                    pReg->sisRegs3C4[0x09] |= 0x80;
            } else  {
                    return FALSE;
            }
            break;
    }
    switch (pScrn->videoRam)  {
        case 512:
            temp = 0x00;
            break;
        case 1024:
            temp = 0x20;
            break;
        case 2048:
            temp = 0x40;
            break;
        case 4096:
            temp = 0x60;
            break;
        case 8192:
            temp = 0x80;
            break;
        default:
            temp = 0x20;
    }
    switch (pSiS->Chipset)  {
        case PCI_CHIP_SG86C225:
        case PCI_CHIP_SIS5597:
        case PCI_CHIP_SIS6326:
            pReg->sisRegs3C4[LinearAdd0] = (pSiS->FbAddress & 0x07F80000) >> 19;
            pReg->sisRegs3C4[LinearAdd1] =((pSiS->FbAddress & 0xF8000000) >> 27)
                                        | temp; /* Enable linear with max 4M */
            break;
        case PCI_CHIP_SIS530:
            pReg->sisRegs3C4[LinearAdd0] = (pSiS->FbAddress & 0x07F80000) >> 19;
            pReg->sisRegs3C4[LinearAdd1] =((pSiS->FbAddress & 0xF8000000) >> 27)
                                        | temp; /* Enable linear with max 8M */
            break;
    }

    /* Screen Offset */
    vgaReg->CRTC[0x13] = offset & 0xFF;
    pReg->sisRegs3C4[CRTCOff] = ((offset & 0xF00) >> 4) | 
                (((mode->CrtcVTotal-2)     & 0x400) >> 10 ) |
                (((mode->CrtcVDisplay-1)   & 0x400) >> 9 ) |
                (((mode->CrtcVSyncStart-1) & 0x400) >> 8 ) |
                (((mode->CrtcVSyncStart)   & 0x400) >> 7 ) ;

    /* Extended Horizontal Overflow Register */
    pReg->sisRegs3C4[0x12] &= 0xE0;
    pReg->sisRegs3C4[0x12] |= (
        (((mode->CrtcHTotal >> 3) - 5)     & 0x100) >> 8 |
        (((mode->CrtcHDisplay >> 3) - 1)   & 0x100) >> 7 |
        (((mode->CrtcHSyncStart >> 3) - 1) & 0x100) >> 6 |
        ((mode->CrtcHSyncStart >> 3)       & 0x100) >> 5 |
        (((mode->CrtcHBlankEnd >> 3) -1)   & 0x40)  >> 2);
/*      ((mode->CrtcHSyncEnd >> 3)         & 0x40)  >> 2); */

    if (mode->CrtcVDisplay > 1024)
        /* disable line compare */
        pReg->sisRegs3C4[0x38] |= 0x04;
    else
        pReg->sisRegs3C4[0x38] &= 0xFB;

    if (( pScrn->depth == 24) || (pScrn->depth == 32) ||
                    (mode->CrtcHDisplay >= 1280))
        /* Enable high speed DCLK */
        pReg->sisRegs3C4[0x3E] |= 1;
    else
        pReg->sisRegs3C4[0x3E] &= 0xFE;


    /* Set vclk */  
    if (SiScompute_vclk(clock, &num, &denum, &div, &sbit, &scale)) {
        switch  (pSiS->Chipset)  {
            case PCI_CHIP_SIS5597:
            case PCI_CHIP_SIS6326:
            case PCI_CHIP_SIS530:
                pReg->sisRegs3C4[XR2A] = (num - 1) & 0x7f ;
                pReg->sisRegs3C4[XR2A] |= (div == 2) ? 0x80 : 0;
                pReg->sisRegs3C4[XR2B] = ((denum -1) & 0x1f);
                pReg->sisRegs3C4[XR2B] |= (((scale -1)&3) << 5);
                /* When set VCLK, you should set SR13 first */
                if (sbit) 
                    pReg->sisRegs3C4[ClockBase] |= 0x40;
                else 
                    pReg->sisRegs3C4[ClockBase] &= 0xBF;
                    
                break;
        }
    }
    else  {
    /* if SiScompute_vclk cannot handle the request clock try sisCalcClock! */
        SiSCalcClock(pScrn, clock, 2, vclk);
        switch (pSiS->Chipset)  {
            case PCI_CHIP_SIS5597:
            case PCI_CHIP_SIS6326:
            case PCI_CHIP_SIS530:
                pReg->sisRegs3C4[XR2A] = (vclk[Midx] - 1) & 0x7f ;
                pReg->sisRegs3C4[XR2A] |= ((vclk[VLDidx] == 2 ) ? 1 : 0 ) << 7 ;

                /* bits [4:0] contain denumerator -MC */
                pReg->sisRegs3C4[XR2B] = (vclk[Nidx] -1) & 0x1f ;

                if (vclk[Pidx] <= 4){
                /* postscale 1,2,3,4 */
                    pReg->sisRegs3C4[XR2B] |= (vclk[Pidx] -1 ) << 5 ;
                    pReg->sisRegs3C4[ClockBase] &= 0xBF;
                } else {
                /* postscale 6,8 */
                    pReg->sisRegs3C4[XR2B] |= ((vclk[Pidx] / 2) -1 ) << 5 ;
                    pReg->sisRegs3C4[ClockBase] |= 0x40;
                }
                pReg->sisRegs3C4[XR2B] |= 0x80 ;   /* gain for high frequency */
                break;
        }
    } /* end of set vclk */

    if (clock > 135000)
        pReg->sisRegs3C4[ClockReg] |= 0x02;

    /* pReg->sisRegs3C2 = inb(0x3CC) | 0x0C;*/ /* Programmable Clock */
    pReg->sisRegs3C2 = inb(pSiS->RelIO+0x4c) | 0x0C; /* Programmable Clock */

    if (pSiS->FastVram && ((pSiS->Chipset == PCI_CHIP_SIS530) ||
        (pSiS->Chipset == PCI_CHIP_SIS6326) ||
        (pSiS->Chipset == PCI_CHIP_SIS5597)))
        pReg->sisRegs3C4[ExtMiscCont5]|= 0xC0;
    else 
        pReg->sisRegs3C4[ExtMiscCont5]&= ~0xC0;

    pSiS->ValidWidth = TRUE;
    if ((pSiS->Chipset == PCI_CHIP_SIS5597) ||
        (pSiS->Chipset == PCI_CHIP_SIS6326) ||
        (pSiS->Chipset == PCI_CHIP_SIS530))
    {
        pReg->sisRegs3C4[GraphEng] &= 0xCF; /* Clear logical width bits */
        if (pScrn->bitsPerPixel == 24)  {
            pReg->sisRegs3C4[GraphEng] |= 0x30; /* Invalid logical width */
            pSiS->ValidWidth = FALSE;
        }
        else  {
            switch ( pScrn->virtualX * (pScrn->bitsPerPixel >> 3) ) {
                case 1024:
                    pReg->sisRegs3C4[GraphEng] |= 0x00; /* | 00 = No change */
                    break;
                case 2048:
                    pReg->sisRegs3C4[GraphEng] |= 0x10; 
                    break;
                case 4096:
                    pReg->sisRegs3C4[GraphEng] |= 0x20; 
                    break;
                default:
                    /* Invalid logical width */
                    pReg->sisRegs3C4[GraphEng] =  0x30;
                    pSiS->ValidWidth = FALSE;
                    break;
            }
        }
    }
    PDEBUG(xf86DrvMsgVerb(pScrn->scrnIndex, X_INFO, 3,
                "virtualX = %d depth = %d Logical width = %d\n",
                pScrn->virtualX, pScrn->bitsPerPixel,
                pScrn->virtualX * pScrn->bitsPerPixel/8));

    if (!pSiS->NoAccel) {
        switch  (pSiS->Chipset)  {
            case PCI_CHIP_SIS5597:
            case PCI_CHIP_SIS6326:
            case PCI_CHIP_SIS530:
                pReg->sisRegs3C4[GraphEng] |= 0x40;
                if (pSiS->TurboQueue) {
                    pReg->sisRegs3C4[GraphEng] |= 0x80;
                                /* All Queue for 2D */
                    pReg->sisRegs3C4[ExtMiscCont9] &= 0xFC;
                    if (pSiS->HWCursor)
                        pReg->sisRegs3C4[TurboQueueBase] = (pScrn->videoRam/32) - 2;
                    else
                        pReg->sisRegs3C4[TurboQueueBase] = (pScrn->videoRam/32) - 1;
                }
                pReg->sisRegs3C4[MMIOEnable] |= 0x60; /* At PCI base */
                pReg->sisRegs3C4[Mode64] |= 0x80;
                break;
        }
    }

   /* Set memclock */
    if ((pSiS->Chipset == PCI_CHIP_SIS5597) || (pSiS->Chipset == PCI_CHIP_SIS6326)) {
      if (pSiS->MemClock > 66000) {
          SiSCalcClock(pScrn, pSiS->MemClock, 1, vclk);
  
          pReg->sisRegs3C4[MemClock0] = (vclk[Midx] - 1) & 0x7f ;
          pReg->sisRegs3C4[MemClock0] |= ((vclk[VLDidx] == 2 ) ? 1 : 0 ) << 7 ;
          pReg->sisRegs3C4[MemClock1]  = (vclk[Nidx] -1) & 0x1f ;   /* bits [4:0] contain denumerator -MC */
          if (vclk[Pidx] <= 4){
            pReg->sisRegs3C4[MemClock1] |= (vclk[Pidx] -1 ) << 5 ; /* postscale 1,2,3,4 */
            pReg->sisRegs3C4[ClockBase] &= 0x7F;
          } else {
            pReg->sisRegs3C4[MemClock1] |= ((vclk[Pidx] / 2) -1 ) << 5 ;  /* postscale 6,8 */
            pReg->sisRegs3C4[ClockBase] |= 0x80;
          }

#if 1  /* Check programmed memory clock. Enable only to check the above code */
          mclk=14318*((pReg->sisRegs3C4[MemClock0] & 0x7f)+1);
          mclk=mclk/((pReg->sisRegs3C4[MemClock1] & 0x0f)+1);
          Base = pReg->sisRegs3C4[ClockBase];
          if ( (Base & 0x80)==0 ) {
            mclk = mclk / (((pReg->sisRegs3C4[MemClock1] & 0x60) >> 5)+1);
          }  else {
            if ((pReg->sisRegs3C4[MemClock1] & 0x60) == 0x40) { mclk=mclk/6;}
            if ((pReg->sisRegs3C4[MemClock1] & 0x60) == 0x60) { mclk=mclk/8;}
          }
          xf86DrvMsgVerb(pScrn->scrnIndex, X_INFO,2,
           "Setting memory clock to %.3f MHz\n",
          mclk/1000.0);
#endif
      }
    }

    /* set threshold value */
    switch  (pSiS->Chipset)  {
        case PCI_CHIP_SIS5597:
        case PCI_CHIP_SIS6326:
            MemBand = SiSMemBandWidth(pScrn) / 10 ;
            safetymargin = 1; 
            gap          = 4;

            CRT_ENGthreshold = 0x0F;
            CRT_CPUthresholdLow = ((pScrn->depth*clock) / 
                                    MemBand)+safetymargin;
            CRT_CPUthresholdHigh =((pScrn->depth*clock) / 
                                    MemBand)+gap+safetymargin;

            if ( CRT_CPUthresholdLow > (pScrn->depth < 24 ? 0xe:0x0d) ) { 
                CRT_CPUthresholdLow = (pScrn->depth < 24 ? 0xe:0x0d); 
            }

            if ( CRT_CPUthresholdHigh > (pScrn->depth < 24 ? 0x10:0x0f) ) {
                CRT_CPUthresholdHigh = (pScrn->depth < 24 ? 0x10:0x0f);
            }

            pReg->sisRegs3C4[CPUThreshold] =  (CRT_ENGthreshold & 0x0F) | 
                            (CRT_CPUthresholdLow & 0x0F)<<4 ;
            pReg->sisRegs3C4[CRTThreshold] = CRT_CPUthresholdHigh & 0x0F;

            break;
        case PCI_CHIP_SIS530:
            (*pSiS->SetThreshold)(pScrn, mode, &CRT_CPUthresholdLow,
                            &CRT_CPUthresholdHigh);
            pReg->sisRegs3C4[8] = (CRT_CPUthresholdLow & 0xf) << 4 | 0xF;
            pReg->sisRegs3C4[9] &= 0xF0;
            pReg->sisRegs3C4[9] |= (CRT_CPUthresholdHigh & 0xF);
            pReg->sisRegs3C4[0x3F] &= 0xE3;
            pReg->sisRegs3C4[0x3F] |= (CRT_CPUthresholdHigh & 0x10) |
                            (CRT_CPUthresholdLow & 0x10) >> 2 |
                            0x08;
            break;
    }

    return(TRUE);
}

/* TW: Initialize various regs for mode. This is done to
 *     structure, not hardware. (SiSRestore would write
 *     structure to hardware registers.)
 *     This function is not used on SiS300, 540, 630 (unless
 *     VESA is used for mode switching); on these chips,
 *     the BIOS emulation (sis_bioc.s) does the job.
 */
Bool
SIS300Init(ScrnInfoPtr pScrn, DisplayModePtr mode)
{
    SISPtr  pSiS = SISPTR(pScrn);
    SISRegPtr   pReg = &pSiS->ModeReg;
    vgaRegPtr   vgaReg = &VGAHWPTR(pScrn)->ModeReg;
    int vgaIOBase;
    unsigned short  temp;
    int offset=0;
    int clock = mode->Clock;
    unsigned int    vclk[5];

    int num, denum, div, sbit, scale;
    unsigned short  Threshold_Low, Threshold_High;

    xf86DrvMsgVerb(pScrn->scrnIndex, X_INFO, 4, "SIS300Init()\n");
    xf86DrvMsgVerb(pScrn->scrnIndex, X_INFO, 4,
        "virtualX = %d depth = %d Logical width = %d\n",
        pScrn->virtualX, pScrn->bitsPerPixel,
        pScrn->virtualX * pScrn->bitsPerPixel/8);

    vgaHWGetIOBase(VGAHWPTR(pScrn));
    vgaIOBase = VGAHWPTR(pScrn)->IOBase;

    (*pSiS->SiSSave)(pScrn, pReg);

    pSiS->scrnOffset = pScrn->displayWidth * ((pScrn->bitsPerPixel+7)/8);

    outw(VGA_SEQ_INDEX, 0x8605);

    /* TW: The following MUST be done even with VESA */
    pReg->sisRegs3C4[6] &= ~GENMASK(4:2);

    switch (pScrn->bitsPerPixel) {
        case 8:
            pSiS->DstColor = 0x0000;
            pReg->sisRegs3C4[6] |= 0x03;
            break;
        case 16:
            if (pScrn->depth==15)  {
                pSiS->DstColor = 0x4000;
                pReg->sisRegs3C4[6] |= ((1 << 2) | 0x03);
            } else {
                pSiS->DstColor = (short) 0x8000;
                pReg->sisRegs3C4[6] |= ((2 << 2) | 0x03);
            }
            break;
        case 24:
            pReg->sisRegs3C4[6] |= ((3 << 2) | 0x03);
            break;
        case 32:
            pSiS->DstColor = (short) 0xC000;
            pReg->sisRegs3C4[6] |= ((4 << 2) | 0x03);
            break;
    }

    if (!pSiS->UseVESA) {	/* TW: Don't do the following when using VESA (NEW) */
    	pReg->sisRegs3D4[0x19] = 0;
    	pReg->sisRegs3D4[0x1A] &= 0xFC;

	if (mode->Flags & V_INTERLACE)  {
	        offset = pSiS->scrnOffset >> 2;
	        pReg->sisRegs3C4[0x06] |= 0x20;
	        if (pSiS->Chipset != PCI_CHIP_SIS300)  {
            	temp = (mode->CrtcHSyncStart >> 3) -
	                (mode->CrtcHTotal >> 3)/2;
            	pReg->sisRegs3D4[0x19] = GETVAR8(temp);
            	pReg->sisRegs3D4[0x1A] |= GETBITS(temp, 9:8);
        	}
    	} else  {
	        offset = pSiS->scrnOffset >> 3;
	        pReg->sisRegs3C4[0x06] &= ~0x20;
    	}

    	pReg->sisRegs3C4[0x07] |= 0x10;     	/* enable High Speed DAC */
    	pReg->sisRegs3C4[0x07] &= 0xFC;
    	if (clock < 100000)
        	pReg->sisRegs3C4[0x07] |= 0x03;
    	else if (clock < 200000)
        	pReg->sisRegs3C4[0x07] |= 0x02;
    	else if (clock < 250000)
        	pReg->sisRegs3C4[0x07] |= 0x01;

	pReg->sisRegs3C4[0x0A] = 			/* Extended Vertical Overflow */
            GETBITSTR(mode->CrtcVTotal     -2, 10:10, 0:0) |
            GETBITSTR(mode->CrtcVDisplay   -1, 10:10, 1:1) |
            GETBITSTR(mode->CrtcVBlankStart  , 10:10, 2:2) |
            GETBITSTR(mode->CrtcVSyncStart   , 10:10, 3:3) |
            GETBITSTR(mode->CrtcVBlankEnd    ,   8:8, 4:4) |
            GETBITSTR(mode->CrtcVSyncEnd     ,   4:4, 5:5) ;

    	pReg->sisRegs3C4[0x0B] = 			/* Extended Horizontal Overflow */
            GETBITSTR((mode->CrtcHTotal      >> 3) - 5, 9:8, 1:0) |
            GETBITSTR((mode->CrtcHDisplay    >> 3) - 1, 9:8, 3:2) |
            GETBITSTR((mode->CrtcHBlankStart >> 3)    , 9:8, 5:4) |
            GETBITSTR((mode->CrtcHSyncStart  >> 3)    , 9:8, 7:6) ;

    	pReg->sisRegs3C4[0x0C] &= 0xF8;
    	pReg->sisRegs3C4[0x0C] |=
            GETBITSTR(mode->CrtcHBlankEnd >> 3, 7:6, 1:0) |
            GETBITSTR(mode->CrtcHSyncEnd  >> 3, 5:5, 2:2) ;


    	vgaReg->CRTC[0x13] = GETVAR8(offset);	/* Screen Offset */
    	pReg->sisRegs3C4[0x0E] &= 0xF0;
    	pReg->sisRegs3C4[0x0E] |= GETBITS(offset, 11:8);

    	if (mode->CrtcHDisplay > 0)		/* line compare */
        	pReg->sisRegs3C4[0x0F] |= 0x08;
    	else
        	pReg->sisRegs3C4[0x0F] &= 0xF7;

    	pReg->sisRegs3C4[0x10] =
        ((mode->CrtcHDisplay *((pScrn->bitsPerPixel+7)/8) + 63) >> 6)+1;
    }	/* VESA */

/* TW: Enable PCI adressing (0x80) & MMIO enable (0x1) & ? (0x20) */
    pReg->sisRegs3C4[0x20] = 0xA1;
/* TW: Enable 3D accelerator & ? */
/* TW: 0x42 enables 2D accellerator (done below), 0x18 enables 3D engine */
/*  pReg->sisRegs3C4[0x1E] = 0x18; */
/* TW: !!! now done according to NoAccel setting !!! */

    if (!pSiS->UseVESA) {	/* TW: clocks have surely been set by VESA, so don't touch them now */
   	if (SiScompute_vclk(clock, &num, &denum, &div, &sbit, &scale)) {  /* Set vclk */
        	pReg->sisRegs3C4[0x2B] = (num -1) & 0x7f;
        	if (div == 2)
            		pReg->sisRegs3C4[0x2B] |= 0x80;
        	pReg->sisRegs3C4[0x2C] = ((denum -1) & 0x1f);
        	pReg->sisRegs3C4[0x2C] |= (((scale-1)&3) << 5);
        	if (sbit)
            		pReg->sisRegs3C4[0x2C] |= 0x80;
        	pReg->sisRegs3C4[0x2D] = 0x80;
    	}
    	else  {
    		/* if SiScompute_vclk cannot handle the request clock try sisCalcClock! */
        	SiSCalcClock(pScrn, clock, 2, vclk);
        	pReg->sisRegs3C4[0x2B] = (vclk[Midx] - 1) & 0x7f ;
        	pReg->sisRegs3C4[0x2B] |= ((vclk[VLDidx] == 2 ) ? 1 : 0 ) << 7 ;

        	/* bits [4:0] contain denumerator -MC */
        	pReg->sisRegs3C4[0x2C] = (vclk[Nidx] -1) & 0x1f ;

        	if (vclk[Pidx] <= 4)  {
        	/* postscale 1,2,3,4 */
            		pReg->sisRegs3C4[0x2C] |= (vclk[Pidx] -1 ) << 5 ;
            		pReg->sisRegs3C4[0x2C] &= 0x7F;
        	} else  {
        	/* postscale 6,8 */
            		pReg->sisRegs3C4[0x2C] |= ((vclk[Pidx] / 2) -1 ) << 5 ;
            		pReg->sisRegs3C4[0x2C] |= 0x80;
        	}
        	pReg->sisRegs3C4[0x2D] = 0x80;
    	} /* end of set vclk */

    	if (clock > 150000)  {  			/* enable two-pixel mode */
        	pReg->sisRegs3C4[0x07] |= 0x80;
        	pReg->sisRegs3C4[0x32] |= 0x08;
    	} else  {
        	pReg->sisRegs3C4[0x07] &= 0x7F;
        	pReg->sisRegs3C4[0x32] &= 0xF7;
    	}

    	pReg->sisRegs3C2 = inb(0x3CC) | 0x0C; 	/* Programmable Clock */
    }  /* VESA */

/* TW: Now initialize TurboQueue. TB is always located at the very top of
       the videoRAM (notably NOT the x framebuffer memory, which can/should
       be limited when using DRI)
*/
    if (!pSiS->NoAccel) {
        pReg->sisRegs3C4[0x1E] |= 0x42;  /* TW: Enable 2D accellerator */
	pReg->sisRegs3C4[0x1E] |= 0x18;  /* TW: Enable 3D accellerator */
        if (pSiS->TurboQueue)  {    		/* set Turbo Queue as 512k */
	    temp = ((pScrn->videoRam/64)-8);    /* TW: 8=512k, 4=256k, 2=128k, 1=64k */
            pReg->sisRegs3C4[0x26] = temp & 0xFF;
	    pReg->sisRegs3C4[0x27] =
		(pReg->sisRegs3C4[0x27] & 0xfc) | (((temp >> 8) & 3) | 0xF0);
        }	/* TW: line above new for saving D2&3 of state register */
    }

    if (!pSiS->UseVESA) {
	/* set threshold value */
    	(*pSiS->SetThreshold)(pScrn, mode, &Threshold_Low, &Threshold_High);
    	pReg->sisRegs3C4[0x08] = GETBITSTR(Threshold_Low, 3:0, 7:4) | 0xF;
    	pReg->sisRegs3C4[0x0F] &= ~GENMASK(5:5);
    	pReg->sisRegs3C4[0x0F] |= GETBITSTR(Threshold_Low, 4:4, 5:5);
    	pReg->sisRegs3C4[0x09] &= ~GENMASK(3:0);
    	pReg->sisRegs3C4[0x09] |= GETBITS(Threshold_High, 3:0);
    }

    return(TRUE);
}

/* TW: Detect video bridge and set VBFlags accordingly */
void SISVGAPreInit(ScrnInfoPtr pScrn)
{
    SISPtr  pSiS = SISPTR(pScrn);
    int     temp;
    char    BIOSversion[]="x.xx.xx\0";
    unsigned short usOffsetHigh, usOffsetLow, vBiosRevision;
    unsigned long   ROMAddr  = (unsigned long) SISPTR(pScrn)->BIOS;

    for (temp = 0; temp < 7; temp++) {
        BIOSversion[temp] = *((unsigned char *)(ROMAddr+temp+0x06));
    }

    xf86DrvMsg(pScrn->scrnIndex, X_PROBED, "Video BIOS version %s detected\n", BIOSversion);

    usOffsetHigh = *((unsigned char *)(ROMAddr+0x08)) - 0x30;
    usOffsetLow  = *((unsigned char *)(ROMAddr+0x09)) - 0x30;
    vBiosRevision = usOffsetHigh << 4 | usOffsetLow;
#if 0	/* TW: What's this good for? Check the BIOS revision???? That can't be correct! */
    if (vBiosRevision < 0x02) {
        outSISIDXREG(pSiS->RelIO+CROFFSET, 0x37, 0);
        inSISIDXREG(pSiS->RelIO+CROFFSET, 0x36, temp);
        temp &= 0x07;
        outSISIDXREG(pSiS->RelIO+CROFFSET, 0x36, temp);
    }
#endif
    outb(SISPART4, 0x00);
    temp = inb(SISPART4+1) & 0x0F;
    pSiS->VBFlags = 0; /*reset*/
    if (temp == 1) {
	outb(SISPART4, 0x01);	/* TW: new for 301b; support is yet incomplete */
	temp = inb(SISPART4+1) & 0xff;
	if (temp >= 0xB0) {
	        pSiS->VBFlags|=VB_301B;  /* TW: 301b */
    		xf86DrvMsg(pScrn->scrnIndex, X_PROBED, "Detected SiS301B video bridge\n");
	} else {
	        pSiS->VBFlags|=VB_301;   /*301*/
		xf86DrvMsg(pScrn->scrnIndex, X_PROBED, "Detected SiS301 video bridge\n");
	}
	outb(SISPART4, 0x23);	/* TW: new */
	temp = inb(SISPART4+1) & 0xff;
	if (!(temp & 0x02))  {
	        pSiS->VBFlags|=VB_NOLCD; /* TW: flag yet unused */
		xf86DrvMsg(pScrn->scrnIndex, X_PROBED, "[SiS301: NoLCD flag detected]\n");
	}
    }
    else if (temp == 2) {
        pSiS->VBFlags|=VB_302;  /*302*/
    	xf86DrvMsg(pScrn->scrnIndex, X_PROBED, "Detected SiS302(B) video bridge\n");
	outb(SISPART4, 0x38);	/* TW: new; LCDA (?) support - yet incomplete */
	temp = inb(SISPART4+1) & 0xff;
	if (temp == 0x03) {
		pSiS->VBFlags|=VB_LCDA; /* TW: flag yet unused */
		xf86DrvMsg(pScrn->scrnIndex, X_PROBED,
		         "[SiS302: LCDA flag detected]\n");
	}
    }
    else if (temp == 3) {
        pSiS->VBFlags|=VB_303;  /*303*/
    	xf86DrvMsg(pScrn->scrnIndex, X_PROBED, "Detected SiS303 video bridge\n");
    }
    else {
        outb(SISCR, 0x37);
        temp = ((inb(SISCR+1))>>1) & 0x07;
	if ((temp == 2) || (temp == 3) || (temp == 4)) {
            pSiS->VBFlags |= VB_LVDS;
    	    xf86DrvMsg(pScrn->scrnIndex, X_PROBED,
	             "Detected LVDS video bridge (Type %d)\n", temp);
	}
        if ((temp == 4) || (temp == 5))  {
            pSiS->VBFlags |= VB_CHRONTEL;
    	    xf86DrvMsg(pScrn->scrnIndex, X_PROBED,
	             "Detected CHRONTEL 7500 VGA->TV converter (Type %d)\n", temp);
	}
	if (temp == 3) {
            xf86DrvMsg(pScrn->scrnIndex, X_PROBED,
	             "Detected TRUMPION TV converter. This device is not supported yet.\n");
	}
	if ((temp < 2) || (temp > 5)) {
	    xf86DrvMsg(pScrn->scrnIndex, X_PROBED,
	             "Detected unknown bridge type (%d)\n", temp);
	}
    }

    switch (pSiS->Chipset) {
        case PCI_CHIP_SIS300:
        case PCI_CHIP_SIS630:
        case PCI_CHIP_SIS540:
            pSiS->ModeInit = SIS300Init;
            break;
        default:
            pSiS->ModeInit = SISInit;
    }
}

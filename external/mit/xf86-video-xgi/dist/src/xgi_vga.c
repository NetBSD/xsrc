/*
 * Mode setup and basic video bridge detection
 *
 * Copyright (C) 2001-2004 by Thomas Winischhofer, Vienna, Austria.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1) Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2) Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3) The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESSED OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Author: 	Thomas Winischhofer <thomas@winischhofer.net>
 *
 * Init() function for old series (except for TV and FIFO calculation)
 * previously based on code which is Copyright (C) 1998,1999 by Alan
 * Hourihane, Wigan, England
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "xf86.h"
#include "xf86_OSproc.h"
#include "xorgVersion.h"
#include "xf86PciInfo.h"
#include "xf86Pci.h"

#include "xgi.h"
#include "xgi_regs.h"
#include "xgi_dac.h"

#include "vb_def.h"

Bool    XG40Init(ScrnInfoPtr pScrn, DisplayModePtr mode);

/* Jong 01/07/2008; force to disable 2D */
extern Bool ForceToDisable2DEngine(ScrnInfoPtr pScrn);

#define Midx    0
#define Nidx    1
#define VLDidx  2
#define Pidx    3

Bool
XG40Init(ScrnInfoPtr pScrn, DisplayModePtr mode)
{
    XGIPtr     pXGI = XGIPTR(pScrn);
    XGIRegPtr    pReg = &pXGI->ModeReg;
    vgaRegPtr    vgaReg = &VGAHWPTR(pScrn)->ModeReg;
    int       vgaIOBase;
    unsigned short temp;
    int       offset;
    int       clock = mode->Clock;
    unsigned int  vclk[5];

    int       num, denum, div, sbit, scale;
    unsigned short Threshold_Low, Threshold_High;

PDEBUG(ErrorF("XG40Init()\n"));

    xf86DrvMsgVerb(pScrn->scrnIndex, X_INFO, 4, "XG40Init()\n");
    xf86DrvMsgVerb(pScrn->scrnIndex, X_INFO, 4,
        "virtualX = %d depth = %d Logical width = %d\n",
        pScrn->virtualX, pScrn->bitsPerPixel,
        pScrn->virtualX * pScrn->bitsPerPixel/8);

    vgaHWGetIOBase(VGAHWPTR(pScrn));
    vgaIOBase = VGAHWPTR(pScrn)->IOBase;

    (*pXGI->XGISave)(pScrn, pReg);

#if !defined(__arm__) 
    outw(VGA_SEQ_INDEX, 0x8605);
#else
    moutl(XGISR, 0x8605);
#endif

    pReg->xgiRegs3C4[6] &= ~GENMASK(4:2);

    switch (pScrn->bitsPerPixel) {
    case 8:
        pXGI->DstColor = 0 ;
        pReg->xgiRegs3C4[6] |= 0x03;
	PDEBUG(ErrorF("8: pXGI->DstColor = %08lX\n",pXGI->DstColor)) ;
        break;
    case 16:
        pXGI->DstColor = 1 << 16 ;
	PDEBUG(ErrorF("16: pXGI->DstColor = %08lX\n",pXGI->DstColor)) ;
        if (pScrn->depth==15) {
            pReg->xgiRegs3C4[6] |= ((1 << 2) | 0x03);
        } else {
            pReg->xgiRegs3C4[6] |= ((2 << 2) | 0x03);
        }
        break;
    case 24:
        pReg->xgiRegs3C4[6] |= ((3 << 2) | 0x03);
        break;
    case 32:
	PDEBUG(ErrorF("32: pXGI->DstColor = %08lX\n",pXGI->DstColor)) ;
        pXGI->DstColor = 2 << 16 ;
        pReg->xgiRegs3C4[6] |= ((4 << 2) | 0x03);
        break;
    }

    pXGI->scrnOffset = pScrn->displayWidth * ((pScrn->bitsPerPixel+7)/8);
    pXGI->scrnOffset += 15 ;
    pXGI->scrnOffset >>= 4 ;
    pXGI->scrnOffset <<= 4 ;

    PDEBUG(ErrorF("XG40Init: pScrn->displayWidth = %ld\n",pScrn->displayWidth )) ;
    PDEBUG(ErrorF("XG40Init: pScrn->bitsPerPixel = %ld\n",pScrn->bitsPerPixel )) ;
    PDEBUG(ErrorF("XG40Init: pXGI->scrnOffset = %ld\n",pXGI->scrnOffset )) ;

    pReg->xgiRegs3D4[0x19] = 0;
    pReg->xgiRegs3D4[0x1A] &= 0xFC;

    if (mode->Flags & V_INTERLACE) {
        offset = pXGI->scrnOffset >> 2;
        pReg->xgiRegs3C4[0x06] |= 0x20;

        temp = (mode->CrtcHSyncStart >> 3) -
            (mode->CrtcHTotal >> 3)/2;
        pReg->xgiRegs3D4[0x19] = GETVAR8(temp);
        pReg->xgiRegs3D4[0x1A] |= GETBITS(temp, 9:8);
    } else {
        offset = pXGI->scrnOffset >> 3;
        pReg->xgiRegs3C4[0x06] &= ~0x20;
    }

    pReg->xgiRegs3C4[0x07] |= 0x10; /* enable High Speed DAC */
    pReg->xgiRegs3C4[0x07] &= 0xFC;
    if (clock < 100000)
        pReg->xgiRegs3C4[0x07] |= 0x03;
    else if (clock < 200000)
        pReg->xgiRegs3C4[0x07] |= 0x02;
    else if (clock < 250000)
        pReg->xgiRegs3C4[0x07] |= 0x01;

    /* Extended Vertical Overflow */
    pReg->xgiRegs3C4[0x0A] =
            GETBITSTR(mode->CrtcVTotal   -2, 10:10, 0:0) |
            GETBITSTR(mode->CrtcVDisplay  -1, 10:10, 1:1) |
            GETBITSTR(mode->CrtcVBlankStart , 10:10, 2:2) |
            GETBITSTR(mode->CrtcVSyncStart  , 10:10, 3:3) |
            GETBITSTR(mode->CrtcVBlankEnd  ,  8:8, 4:4) |
            GETBITSTR(mode->CrtcVSyncEnd   ,  4:4, 5:5) ;

    /* Extended Horizontal Overflow */
    pReg->xgiRegs3C4[0x0B] =
            GETBITSTR((mode->CrtcHTotal   >> 3) - 5, 9:8, 1:0) |
            GETBITSTR((mode->CrtcHDisplay  >> 3) - 1, 9:8, 3:2) |
            GETBITSTR((mode->CrtcHBlankStart >> 3)  , 9:8, 5:4) |
            GETBITSTR((mode->CrtcHSyncStart >> 3)  , 9:8, 7:6) ;

    pReg->xgiRegs3C4[0x0C] &= 0xF8;
    pReg->xgiRegs3C4[0x0C] |=
            GETBITSTR(mode->CrtcHBlankEnd >> 3, 7:6, 1:0) |
            GETBITSTR(mode->CrtcHSyncEnd >> 3, 5:5, 2:2) ;

    /* Screen Offset */
    vgaReg->CRTC[0x13] = GETVAR8(offset);
    pReg->xgiRegs3C4[0x0E] &= 0xF0;
    pReg->xgiRegs3C4[0x0E] |= GETBITS(offset, 11:8);

    /* line compare */
    if (mode->CrtcHDisplay > 0)
        pReg->xgiRegs3C4[0x0F] |= 0x08;
    else
        pReg->xgiRegs3C4[0x0F] &= 0xF7;

    pReg->xgiRegs3C4[0x10] =
        ((mode->CrtcHDisplay *((pScrn->bitsPerPixel+7)/8) + 63) >> 6)+1;

    /* Enable Linear */
    pReg->xgiRegs3C4[0x20] |= 0x81;


  /* Set vclk */
  if (compute_vclk(clock, &num, &denum, &div, &sbit, &scale)) {
        pReg->xgiRegs3C4[0x2B] = (num -1) & 0x7f;
        if (div == 2)
          pReg->xgiRegs3C4[0x2B] |= 0x80;
        pReg->xgiRegs3C4[0x2C] = ((denum -1) & 0x1f);
        pReg->xgiRegs3C4[0x2C] |= (((scale-1)&3) << 5);
        if (sbit)
          pReg->xgiRegs3C4[0x2C] |= 0x80;
        pReg->xgiRegs3C4[0x2D] = 0x80;
  }
  else {
  /* if compute_vclk cannot handle the request clock try XGICalcClock! */
    XGICalcClock(pScrn, clock, 2, vclk);
        pReg->xgiRegs3C4[0x2B] = (vclk[Midx] - 1) & 0x7f ;
        pReg->xgiRegs3C4[0x2B] |= ((vclk[VLDidx] == 2 ) ? 1 : 0 ) << 7 ;

        /* bits [4:0] contain denumerator -MC */
        pReg->xgiRegs3C4[0x2C] = (vclk[Nidx] -1) & 0x1f ;

        if (vclk[Pidx] <= 4) {
        /* postscale 1,2,3,4 */
          pReg->xgiRegs3C4[0x2C] |= (vclk[Pidx] -1 ) << 5 ;
          pReg->xgiRegs3C4[0x2C] &= 0x7F;
        } else {
        /* postscale 6,8 */
          pReg->xgiRegs3C4[0x2C] |= ((vclk[Pidx] / 2) -1 ) << 5 ;
          pReg->xgiRegs3C4[0x2C] |= 0x80;
        }
        pReg->xgiRegs3C4[0x2D] = 0x80;
  } /* end of set vclk */

    if ( (pXGI->Chipset == PCI_CHIP_XGIXG40) && (clock > 150000) ) { /* enable two-pixel mode */
        pReg->xgiRegs3C4[0x07] |= 0x80;
        pReg->xgiRegs3C4[0x32] |= 0x08;
    } else {
        pReg->xgiRegs3C4[0x07] &= 0x7F;
        pReg->xgiRegs3C4[0x32] &= 0xF7;
    }

    /*pReg->xgiRegs3C2 = inb(0x3CC) | 0x0C;*/ /* Programmable Clock */
    pReg->xgiRegs3C2 = inb(pXGI->RelIO+0x4c) | 0x0C; /*Programmable Clock*/

    if (!pXGI->NoAccel) {
	/* Enable 2D accelerator. 
	 */
		/* Jong 01/07/2008; disable 2D engine depend on  SR3A[6]:1-> force to siable 2D */
		if(pXGI->Chipset != PCI_CHIP_XGIXG21)
			pReg->xgiRegs3C4[0x1E] |= 0x42;
		else
		{
			if(ForceToDisable2DEngine(pScrn))
				pReg->xgiRegs3C4[0x1E] |= 0x02;
		}

    }

    /* set threshold value */
    (*pXGI->SetThreshold)(pScrn, mode, &Threshold_Low, &Threshold_High);
    pReg->xgiRegs3C4[0x08] = GETBITSTR(Threshold_Low, 3:0, 7:4) | 0xF;
    pReg->xgiRegs3C4[0x0F] &= ~GENMASK(5:5);
    pReg->xgiRegs3C4[0x0F] |= GETBITSTR(Threshold_Low, 4:4, 5:5);
    pReg->xgiRegs3C4[0x09] &= ~GENMASK(3:0);
    pReg->xgiRegs3C4[0x09] |= GETBITS(Threshold_High, 3:0);

    return(TRUE);
}

/* Detect video bridge and set VBFlags accordingly */
void XGIVGAPreInit(ScrnInfoPtr pScrn)
{
    XGIPtr  pXGI = XGIPTR(pScrn);
    
    switch (pXGI->Chipset) {
        case PCI_CHIP_XGIXG40:
        case PCI_CHIP_XGIXG20:
        case PCI_CHIP_XGIXG21:
        case PCI_CHIP_XGIXG27:
          default:
            pXGI->ModeInit = XG40Init;
            break;
    }

 
}


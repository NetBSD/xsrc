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
/* $XFree86: xc/programs/Xserver/hw/xfree86/drivers/sis/sis_dac.c,v 1.18 2000/12/02 15:30:50 tsi Exp $ */

#include "xf86.h"
#include "xf86_OSproc.h"
#include "xf86_ansic.h"
#include "xf86Version.h"
#include "xf86PciInfo.h"
#include "xf86Pci.h"

#include "vgaHW.h"

#include "sis.h"
#include "sis_regs.h"
#include "sis_vb.h"

static  void    SiSSave(ScrnInfoPtr pScrn, SISRegPtr sisReg);
static  void    SiSRestore(ScrnInfoPtr pScrn, SISRegPtr sisReg);

static  void    SiS300Save(ScrnInfoPtr pScrn, SISRegPtr sisReg);
static  void    SiS301Save(ScrnInfoPtr pScrn, SISRegPtr sisReg);
static  void    SiSLVDSSave(ScrnInfoPtr pScrn, SISRegPtr sisReg);
static  void    SiS300Restore(ScrnInfoPtr pScrn, SISRegPtr sisReg);
static  void    SiS301Restore(ScrnInfoPtr pScrn, SISRegPtr sisReg);
static  void    SiSLVDSRestore(ScrnInfoPtr pScrn, SISRegPtr sisReg);
static  void	SiSChrontelSave(ScrnInfoPtr pScrn, SISRegPtr sisReg);

static  void    SiS301LoadPalette(ScrnInfoPtr pScrn, int numColors,
                        int *indicies, LOCO *colors, VisualPtr pVisual);

static  void    SiS300Threshold(ScrnInfoPtr pScrn, DisplayModePtr mode,
                                unsigned short *Low, unsigned short *High);
static  void    SiS630Threshold(ScrnInfoPtr pScrn, DisplayModePtr mode,
                                unsigned short *Low, unsigned short *High);
static  void    SiS530Threshold(ScrnInfoPtr pScrn, DisplayModePtr mode,
                                unsigned short *Low, unsigned short *High);
static  void    SiSThreshold(ScrnInfoPtr pScrn, DisplayModePtr mode,
                                unsigned short *Low, unsigned short *High);

unsigned short ch7005idx[0x11]={0x00,0x07,0x08,0x0a,0x0b,0x04,0x09,0x20,0x21,0x18,0x19,0x1a,0x1b,0x1c,0x1d,0x1e,0x1f};


int
compute_vclk(
        int Clock,
        int *out_n,
        int *out_dn,
        int *out_div,
        int *out_sbit,
        int *out_scale)
{
        float f,x,y,t, error, min_error;
        int n, dn, best_n=0, best_dn=0;

        /*
         * Rules
         *
         * VCLK = 14.318 * (Divider/Post Scalar) * (Numerator/DeNumerator)
         * Factor = (Divider/Post Scalar)
         * Divider is 1 or 2
         * Post Scalar is 1, 2, 3, 4, 6 or 8
         * Numberator ranged from 1 to 128
         * DeNumerator ranged from 1 to 32
         * a. VCO = VCLK/Factor, suggest range is 150 to 250 Mhz
         * b. Post Scalar selected from 1, 2, 4 or 8 first.
         * c. DeNumerator selected from 2.
         *
         * According to rule a and b, the VCO ranges that can be scaled by
         * rule b are:
         *      150    - 250    (Factor = 1)
         *       75    - 125    (Factor = 2)
         *       37.5  -  62.5  (Factor = 4)
         *       18.75 -  31.25 (Factor = 8)
         *
         * The following ranges use Post Scalar 3 or 6:
         *      125    - 150    (Factor = 1.5)
         *       62.5  -  75    (Factor = 3)
         *       31.25 -  37.5  (Factor = 6)
         *
         * Steps:
         * 1. divide the Clock by 2 until the Clock is less or equal to 31.25.
         * 2. if the divided Clock is range from 18.25 to 31.25, than
         *    the Factor is 1, 2, 4 or 8.
         * 3. if the divided Clock is range from 15.625 to 18.25, than
         *    the Factor is 1.5, 3 or 6.
         * 4. select the Numberator and DeNumberator with minimum deviation.
         *
         * ** this function can select VCLK ranged from 18.75 to 250 Mhz
         */
        f = (float) Clock;
        f /= 1000.0;
        if ((f > 250.0) || (f < 18.75))
                return 0;

        min_error = f;
        y = 1.0;
        x = f;
        while (x > 31.25) {
                y *= 2.0;
                x /= 2.0;
        }
        if (x >= 18.25) {
                x *= 8.0;
                y = 8.0 / y;
        } else if (x >= 15.625) {
                x *= 12.0;
                y = 12.0 / y;
        }

        t = y;
        if (t == (float) 1.5) {
                *out_div = 2;
                t *= 2.0;
        } else {
                *out_div = 1;
        }
        if (t > (float) 4.0) {
                *out_sbit = 1;
                t /= 2.0;
        } else {
                *out_sbit = 0;
        }

        *out_scale = (int) t;

        for (dn=2;dn<=32;dn++) {
                for (n=1;n<=128;n++) {
                        error = x;
                        error -= ((float) 14.318 * (float) n / (float) dn);
                        if (error < (float) 0)
                                error = -error;
                        if (error < min_error) {
                                min_error = error;
                                best_n = n;
                                best_dn = dn;
                        }
                }
        }
        *out_n = best_n;
        *out_dn = best_dn;
        PDEBUG(ErrorF("compute_vclk: Clock=%d, n=%d, dn=%d, div=%d, sbit=%d,"
                        " scale=%d\n", Clock, best_n, best_dn, *out_div,
                        *out_sbit, *out_scale));
        return 1;
}


void
SiSCalcClock(ScrnInfoPtr pScrn, int clock, int max_VLD, unsigned int *vclk)
{
    SISPtr pSiS = SISPTR(pScrn);
    int M, N, P , PSN, VLD , PSNx ;
    int bestM=0, bestN=0, bestP=0, bestPSN=0, bestVLD=0;
    double bestError, abest = 42.0, bestFout;
    double target;
    double Fvco, Fout;
    double error, aerror;

    /*
     *  fd = fref*(Numerator/Denumerator)*(Divider/PostScaler)
     *
     *  M       = Numerator [1:128] 
     *  N       = DeNumerator [1:32]
     *  VLD     = Divider (Vco Loop Divider) : divide by 1, 2
     *  P       = Post Scaler : divide by 1, 2, 3, 4
     *  PSN     = Pre Scaler (Reference Divisor Select) 
     * 
     * result in vclk[]
     */
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
  
  int M_min = 2; 
  int M_max = 128;
  
/*  abest=10000.0; */
 
  target = clock * 1000;
 

  if (pSiS->Chipset == PCI_CHIP_SIS5597 || pSiS->Chipset == PCI_CHIP_SIS6326){
        int low_N = 2;
        int high_N = 5;
        int PSN = 1;
 
        P = 1;
        if (target < MAX_VCO_5597 / 2)
            P = 2;
        if (target < MAX_VCO_5597 / 3)
            P = 3;
        if (target < MAX_VCO_5597 / 4)
            P = 4;
        if (target < MAX_VCO_5597 / 6)
            P = 6;
        if (target < MAX_VCO_5597 / 8)
            P = 8;
 
        Fvco = P * target;
 
        for (N = low_N; N <= high_N; N++){
            double M_desired = Fvco / Fref * N;
            if (M_desired > M_max * max_VLD)
                continue;
 
            if ( M_desired > M_max ) {
                M = M_desired / 2 + 0.5;
                VLD = 2;
            } else {
                M = Fvco / Fref * N + 0.5;
                VLD = 1;
            }
 
            Fout = (double)Fref * (M * VLD)/(N * P);
 
            error = (target - Fout) / target;
            aerror = (error < 0) ? -error : error;
/*          if (aerror < abest && abest > TOLERANCE) {*/
            if (aerror < abest) {
                abest = aerror;
                bestError = error;
                bestM = M;
                bestN = N;
                bestP = P;
                bestPSN = PSN;
                bestVLD = VLD;
                bestFout = Fout;
            }
        }
     }
     else {
         for (PSNx = 0; PSNx <= MAX_PSN ; PSNx++) {
            int low_N, high_N;
            double FrefVLDPSN;
 
            PSN = !PSNx ? 1 : 4;
 
            low_N = 2;
            high_N = 32;
 
            for ( VLD = 1 ; VLD <= max_VLD ; VLD++ ) {
 
                FrefVLDPSN = (double)Fref * VLD / PSN;
                for (N = low_N; N <= high_N; N++) {
                    double tmp = FrefVLDPSN / N;
 
                    for (P = 1; P <= 4; P++) {  
                        double Fvco_desired = target * ( P );
                        double M_desired = Fvco_desired / tmp;
 
                        /* Which way will M_desired be rounded?  
                         *  Do all three just to be safe.  
                         */
                        int M_low = M_desired - 1;
                        int M_hi = M_desired + 1;
 
                        if (M_hi < M_min || M_low > M_max)
                            continue;
 
                        if (M_low < M_min)
                            M_low = M_min;
                        if (M_hi > M_max)
                            M_hi = M_max;
 
                        for (M = M_low; M <= M_hi; M++) {
                            Fvco = tmp * M;
                            if (Fvco <= MIN_VCO)
                                continue;
                            if (Fvco > MAX_VCO)
                                break;
 
                            Fout = Fvco / ( P );
 
                            error = (target - Fout) / target;
                            aerror = (error < 0) ? -error : error;
                            if (aerror < abest) {
                                abest = aerror;
                                bestError = error;
                                bestM = M;
                                bestN = N;
                                bestP = P;
                                bestPSN = PSN;
                                bestVLD = VLD;
                                bestFout = Fout;
                            }
                        xf86DrvMsgVerb(pScrn->scrnIndex, X_INFO,3,"Freq. selected: %.2f MHz, M=%d, N=%d, VLD=%d,"
                               " P=%d, PSN=%d\n",
                               (float)(clock / 1000.), M, N, P, VLD, PSN);
                        xf86DrvMsgVerb(pScrn->scrnIndex, X_INFO,3,"Freq. set: %.2f MHz\n", Fout / 1.0e6);
                        }
                    }
                }
            }
         }
  }

  vclk[Midx]    = bestM;
  vclk[Nidx]    = bestN;
  vclk[VLDidx]  = bestVLD;
  vclk[Pidx]    = bestP;
  vclk[PSNidx]  = bestPSN;

        PDEBUG(xf86DrvMsgVerb(pScrn->scrnIndex, X_INFO, 3,
                "Freq. selected: %.2f MHz, M=%d, N=%d, VLD=%d, P=%d, PSN=%d\n",
                (float)(clock / 1000.), vclk[Midx], vclk[Nidx], vclk[VLDidx], 
                vclk[Pidx], vclk[PSNidx]));
        PDEBUG(xf86DrvMsgVerb(pScrn->scrnIndex, X_INFO, 3,
                "Freq. set: %.2f MHz\n", bestFout / 1.0e6));
        PDEBUG(xf86DrvMsgVerb(pScrn->scrnIndex, X_INFO, 3,
                "VCO Freq.: %.2f MHz\n", bestFout*bestP / 1.0e6));
}


static void
SiSSave(ScrnInfoPtr pScrn, SISRegPtr sisReg)
{
    SISPtr pSiS = SISPTR(pScrn);
    int vgaIOBase;
    int i,max;

        PDEBUG(xf86DrvMsgVerb(pScrn->scrnIndex, X_INFO, 3,
                        "SiSSave(ScrnInfoPtr pScrn, SISRegPtr sisReg)\n"));

    vgaHWGetIOBase(VGAHWPTR(pScrn));
    vgaIOBase = VGAHWPTR(pScrn)->IOBase;

    outw(VGA_SEQ_INDEX, 0x8605);

    switch (pSiS->Chipset) {
        case PCI_CHIP_SIS5597:
           max=0x39;
           break;
        case PCI_CHIP_SIS6326:
        case PCI_CHIP_SIS530:
           max=0x3F;
           break;
        default:
           max=0x37;
           break;
        }

    for (i = 0x06; i <= max; i++) {
        outb(VGA_SEQ_INDEX, i);
        xf86DrvMsgVerb(pScrn->scrnIndex, X_INFO, 4,
                    "XR%02X Contents - %02X \n", i, inb(VGA_SEQ_DATA));
        sisReg->sisRegs3C4[i] = inb(VGA_SEQ_DATA);
    }

    /*sisReg->sisRegs3C2 = inb(0x3CC);*/
    sisReg->sisRegs3C2 = inb(pSiS->RelIO+0x4c);
}

static void
SiSRestore(ScrnInfoPtr pScrn, SISRegPtr sisReg)
{
    SISPtr pSiS = SISPTR(pScrn);
    int vgaIOBase;
    int i,max;

    xf86DrvMsgVerb(pScrn->scrnIndex, X_INFO, 4,
                "SiSRestore(ScrnInfoPtr pScrn, SISRegPtr sisReg)\n");

    vgaHWGetIOBase(VGAHWPTR(pScrn));
    vgaIOBase = VGAHWPTR(pScrn)->IOBase;

    outw(VGA_SEQ_INDEX, 0x8605);

    switch (pSiS->Chipset) {
        case PCI_CHIP_SIS5597:
           max=0x39;
           break;
        case PCI_CHIP_SIS6326:
           max=0x3F;
           break;
        case PCI_CHIP_SIS530:
           max=0x3F;
           break;
        default:
           max=0x37;
           break;
    }

    for (i = 0x06; i <= max; i++) {
        outb(VGA_SEQ_INDEX,i);
        xf86DrvMsgVerb(pScrn->scrnIndex, X_INFO,4,
                    "XR%X Contents - %02X ", i, inb(VGA_SEQ_DATA));

        outb(VGA_SEQ_DATA,sisReg->sisRegs3C4[i]);

        xf86DrvMsgVerb(pScrn->scrnIndex, X_INFO,4,
                        "Restore to - %02X Read after - %02X\n",
                        sisReg->sisRegs3C4[i], inb(VGA_SEQ_DATA));
    }

    /*outb(0x3C2, sisReg->sisRegs3C2);*/
    outb(pSiS->RelIO+0x42, sisReg->sisRegs3C2);

    /* MemClock needs this to take effect */

    outw(VGA_SEQ_INDEX, 0x0100);        /* Synchronous Reset */
    outw(VGA_SEQ_INDEX, 0x0300);        /* End Reset */

}

static void
SiS300Save(ScrnInfoPtr pScrn, SISRegPtr sisReg)
{
    SISPtr pSiS = SISPTR(pScrn);
    int vgaIOBase;
    int i,max;

        PDEBUG(xf86DrvMsgVerb(pScrn->scrnIndex, X_INFO, 3,
                        "SiS300Save(ScrnInfoPtr pScrn, SISRegPtr sisReg)\n"));

    vgaHWGetIOBase(VGAHWPTR(pScrn));
    vgaIOBase = VGAHWPTR(pScrn)->IOBase;

    outw(VGA_SEQ_INDEX, 0x8605);

    max=0x3D;

    for (i = 0x06; i <= max; i++) {
        outb(VGA_SEQ_INDEX, i);
        xf86DrvMsgVerb(pScrn->scrnIndex, X_INFO, 4,
                    "XR%02X Contents - %02X \n", i, inb(VGA_SEQ_DATA));
        sisReg->sisRegs3C4[i] = inb(VGA_SEQ_DATA);
    }

    for (i=0x19; i<0x40; i++)  {
        inSISIDXREG(pSiS->RelIO+CROFFSET, i, sisReg->sisRegs3D4[i]);
    }

    /*sisReg->sisRegs3C2 = inb(0x3CC);*/
    sisReg->sisRegs3C2 = inb(pSiS->RelIO+0x4c);

    if ((pSiS->VBFlags & (VB_LVDS | CRT2_LCD))==(VB_LVDS|CRT2_LCD)) 
        (*pSiS->SiSSaveLVDS)(pScrn, sisReg);
    if ((pSiS->VBFlags & (VB_CHRONTEL | CRT2_TV))==(VB_CHRONTEL|CRT2_TV))
	(*pSiS->SiSSaveChrontel)(pScrn,sisReg);		
    if ((pSiS->VBFlags & (VB_301|VB_302|VB_303)) && (pSiS->VBFlags & (CRT2_LCD|CRT2_TV|CRT2_VGA))) 
        (*pSiS->SiSSave2)(pScrn, sisReg);
}

static void
SiS300Restore(ScrnInfoPtr pScrn, SISRegPtr sisReg)
{
    SISPtr pSiS = SISPTR(pScrn);
    int vgaIOBase;
    int i,max, temp;

    xf86DrvMsgVerb(pScrn->scrnIndex, X_INFO, 4,
                "SiS300Restore(ScrnInfoPtr pScrn, SISRegPtr sisReg)\n");

    vgaHWGetIOBase(VGAHWPTR(pScrn));
    vgaIOBase = VGAHWPTR(pScrn)->IOBase;

    outw(VGA_SEQ_INDEX, 0x8605);
    inSISIDXREG(VGA_SEQ_INDEX, 0x1E, temp);
    if (temp & 0x42)  {
        while ( (MMIO_IN16(pSiS->IOBase, 0x8242) & 0xE000) != 0xE000){};
    }

    max=0x3D;
    for (i = 0x19; i < 0x40; i++)  {
        outSISIDXREG(pSiS->RelIO+CROFFSET, i, sisReg->sisRegs3D4[i]);
    }
    if (pSiS->Chipset != PCI_CHIP_SIS300)  {
           outSISIDXREG(pSiS->RelIO+CROFFSET, 0x1A, sisReg->sisRegs3D4[0x19]);
           outSISIDXREG(pSiS->RelIO+CROFFSET, 0x19, sisReg->sisRegs3D4[0x1A]);
    }

    if ((pSiS->Chipset == PCI_CHIP_SIS630) && (sisReg->sisRegs3C4[0x1e] & 0x40))
        outw(VGA_SEQ_INDEX, sisReg->sisRegs3C4[0x20] << 8 | 0x20);

    for (i = 0x06; i <= max; i++) {
        outb(VGA_SEQ_INDEX,i);
        xf86DrvMsgVerb(pScrn->scrnIndex, X_INFO,4,
                    "XR%X Contents - %02X ", i, inb(VGA_SEQ_DATA));

        outb(VGA_SEQ_DATA,sisReg->sisRegs3C4[i]);

        xf86DrvMsgVerb(pScrn->scrnIndex, X_INFO,4,
                        "Restore to - %02X Read after - %02X\n",
                        sisReg->sisRegs3C4[i], inb(VGA_SEQ_DATA));
    }

    if ((pSiS->VBFlags & (VB_LVDS | CRT2_LCD))==(VB_LVDS|CRT2_LCD)) 
        (*pSiS->SiSRestoreLVDS)(pScrn, sisReg);
    if ((pSiS->VBFlags & (VB_CHRONTEL | CRT2_TV))==(VB_CHRONTEL|CRT2_TV))
	(*pSiS->SiSRestoreChrontel)(pScrn,sisReg);		
    if ((pSiS->VBFlags & (VB_301|VB_302|VB_303)) && (pSiS->VBFlags & (CRT2_LCD|CRT2_TV|CRT2_VGA))) 
        (*pSiS->SiSRestore2)(pScrn, sisReg);
    

    /*outb(0x3C2, sisReg->sisRegs3C2);*/
    outb(pSiS->RelIO+0x42, sisReg->sisRegs3C2);

    /* MemClock needs this to take effect */
     
    outw(VGA_SEQ_INDEX, 0x0100);        /* Synchronous Reset */
    outw(VGA_SEQ_INDEX, 0x0300);        /* End Reset */
}

static void
SiS301Save(ScrnInfoPtr pScrn, SISRegPtr sisReg)
{
        SISPtr  pSiS = SISPTR(pScrn);
        int     i;

        /* for SiS301 only */
        for (i=0; i<0x29; i++)  {
                inSISIDXREG(pSiS->RelIO+4, i, sisReg->VBPart1[i]);
        }
        for (i=0; i<0x46; i++)  {
                inSISIDXREG(pSiS->RelIO+0x10, i, sisReg->VBPart2[i]);
        }
        for (i=0; i<0x3F; i++)  {
                inSISIDXREG(pSiS->RelIO+0x12, i, sisReg->VBPart3[i]);
        }
        for (i=0; i<0x1C; i++)  {
                inSISIDXREG(pSiS->RelIO+0x14, i, sisReg->VBPart4[i]);
        }
        sisReg->VBPart2[0] &= ~0x20;    /* Disable VB Processor */
        sisReg->sisRegs3C4[0x32] &= ~0x20;      /* Disable Lock Mode */
}

static void
SiSLVDSSave(ScrnInfoPtr pScrn, SISRegPtr sisReg)
{
        SISPtr  pSiS = SISPTR(pScrn);
        int     i;

        /* for SiS LVDS only */
        for (i=0; i<0x29; i++)  {
                inSISIDXREG(pSiS->RelIO+4, i, sisReg->VBPart1[i]);
        }
        sisReg->sisRegs3C4[0x32] &= ~0x20;      /* Disable Lock Mode */
}

static void
SiSChrontelSave(ScrnInfoPtr pScrn, SISRegPtr sisReg)
{
        SISPtr  pSiS = SISPTR(pScrn);
        int     i;

        /* for SiS Chrontel TV */
        for (i=0; i<0x29; i++)  {
                inSISIDXREG(pSiS->RelIO+4, i, sisReg->VBPart1[i]);
        }
	for (i=0; i<0x11; i++)
		sisReg->ch7005[i]=GetCH7005(ch7005idx[i]);

        sisReg->sisRegs3C4[0x32] &= ~0x20;      /* Disable Lock Mode */
}


static void
SiS301Restore(ScrnInfoPtr pScrn, SISRegPtr sisReg)
{
        SISPtr  pSiS = SISPTR(pScrn);
        unsigned char   temp, temp1;

        DisableBridge(pSiS->RelIO+0x30); 
        UnLockCRT2(pSiS->RelIO+0x30);  

        /* SetCRT2ModeRegs() */
        outSISIDXREG(pSiS->RelIO+0x04, 4, 0);
        outSISIDXREG(pSiS->RelIO+0x04, 5, 0);
        outSISIDXREG(pSiS->RelIO+0x04, 6, 0);
        outSISIDXREG(pSiS->RelIO+0x04, 0, sisReg->VBPart1[0]);
        outSISIDXREG(pSiS->RelIO+0x04, 1, sisReg->VBPart1[1]);
        outSISIDXREG(pSiS->RelIO+0x14, 0x0D, sisReg->VBPart4[0x0D]);
        outSISIDXREG(pSiS->RelIO+0x14, 0x0C, sisReg->VBPart4[0x0C]); 

        if (!(sisReg->sisRegs3D4[0x30] & 0x03) &&
             (sisReg->sisRegs3D4[0x31] & 0x20))  {      /* disable CRT2 */
                LockCRT2(pSiS->RelIO+0x30);  
                return;
        }
        SetBlock(pSiS->RelIO+0x04, 0x02, 0x23, &(sisReg->VBPart1[0x02]));
        SetBlock(pSiS->RelIO+0x10, 0x00, 0x45, &(sisReg->VBPart2[0x00]));
        SetBlock(pSiS->RelIO+0x12, 0x00, 0x3E, &(sisReg->VBPart3[0x00]));
        SetBlock(pSiS->RelIO+0x14, 0x0E, 0x11, &(sisReg->VBPart4[0x0E]));
        SetBlock(pSiS->RelIO+0x14, 0x13, 0x1B, &(sisReg->VBPart4[0x13])); 

        outSISIDXREG(pSiS->RelIO+0x14, 0x0A, 1);
        outSISIDXREG(pSiS->RelIO+0x14, 0x0B, sisReg->VBPart4[0x0B]);
        outSISIDXREG(pSiS->RelIO+0x14, 0x0A, sisReg->VBPart4[0x0A]);
        outSISIDXREG(pSiS->RelIO+0x14, 0x12, 0);
        outSISIDXREG(pSiS->RelIO+0x14, 0x12, sisReg->VBPart4[0x12]); 

        
        temp1 = 0;
        if(!(pSiS->VBFlags & CRT2_VGA)) {
          inSISIDXREG(pSiS->RelIO+CROFFSET, 0x31, temp);
          if (temp & (SET_IN_SLAVE_MODE >> 8)) { 
             inSISIDXREG(pSiS->RelIO+CROFFSET, 0x30, temp);
             if (!(temp & (SET_CRT2_TO_RAMDAC >> 8))) { 
                temp1 = 0x20;
             }
          }
        }  
        setSISIDXREG(pSiS->RelIO+SROFFSET, 0x32, ~0x20, temp1); 
        orSISIDXREG(pSiS->RelIO+SROFFSET, 0x1E, 0x20);
        andSISIDXREG(pSiS->RelIO+SROFFSET, 1, ~0x20);   /* DisplayOn */

        EnableBridge(pSiS->RelIO+0x30);  
        LockCRT2(pSiS->RelIO+0x30);  
}

static void
SiSLVDSRestore(ScrnInfoPtr pScrn, SISRegPtr sisReg)
{
        SISPtr  pSiS = SISPTR(pScrn);

        DisableBridge(pSiS->RelIO+0x30); 
        UnLockCRT2(pSiS->RelIO+0x30);  

        /* SetCRT2ModeRegs() */
        outSISIDXREG(pSiS->RelIO+0x04, 4, 0);
        outSISIDXREG(pSiS->RelIO+0x04, 5, 0);
        outSISIDXREG(pSiS->RelIO+0x04, 6, 0);
        outSISIDXREG(pSiS->RelIO+0x04, 0, sisReg->VBPart1[0]);
        outSISIDXREG(pSiS->RelIO+0x04, 1, sisReg->VBPart1[1]);

        if (!(sisReg->sisRegs3D4[0x30] & 0x03) &&
             (sisReg->sisRegs3D4[0x31] & 0x20))  {      /* disable CRT2 */
                LockCRT2(pSiS->RelIO+0x30);  
                return;
        }
        SetBlock(pSiS->RelIO+0x04, 0x02, 0x23, &(sisReg->VBPart1[0x02]));

        orSISIDXREG(pSiS->RelIO+SROFFSET, 0x1E, 0x20);
        andSISIDXREG(pSiS->RelIO+SROFFSET, 1, ~0x20);   /* DisplayOn */

        EnableBridge(pSiS->RelIO+0x30);  
        LockCRT2(pSiS->RelIO+0x30);  
}

static void
SiSChrontelRestore(ScrnInfoPtr pScrn, SISRegPtr sisReg)
{
        SISPtr  pSiS = SISPTR(pScrn);
	int i;
	unsigned short wtemp;

        DisableBridge(pSiS->RelIO+0x30); 
        UnLockCRT2(pSiS->RelIO+0x30);  

	for (i=0; i<0x11; i++)
	{	wtemp = ((sisReg->ch7005[i]) << 8) + (ch7005idx[i] & 0x00FF);
		SetCH7005(wtemp);
	}

        /* SetCRT2ModeRegs() */
        outSISIDXREG(pSiS->RelIO+0x04, 4, 0);
        outSISIDXREG(pSiS->RelIO+0x04, 5, 0);
        outSISIDXREG(pSiS->RelIO+0x04, 6, 0);
        outSISIDXREG(pSiS->RelIO+0x04, 0, sisReg->VBPart1[0]);
        outSISIDXREG(pSiS->RelIO+0x04, 1, sisReg->VBPart1[1]);

        if (!(sisReg->sisRegs3D4[0x30] & 0x03) &&
             (sisReg->sisRegs3D4[0x31] & 0x20))  {      /* disable CRT2 */
                LockCRT2(pSiS->RelIO+0x30);  
                return;
        }
        SetBlock(pSiS->RelIO+0x04, 0x02, 0x23, &(sisReg->VBPart1[0x02]));

        orSISIDXREG(pSiS->RelIO+SROFFSET, 0x1E, 0x20);
        andSISIDXREG(pSiS->RelIO+SROFFSET, 1, ~0x20);   /* DisplayOn */

        EnableBridge(pSiS->RelIO+0x30);  
        LockCRT2(pSiS->RelIO+0x30);  
}

unsigned int
SiSddc1Read(ScrnInfoPtr pScrn)
{
#if 0
    SISPtr pSiS = SISPTR(pScrn);
#endif
    int vgaIOBase = VGAHWPTR(pScrn)->IOBase;
    unsigned char temp, temp2;

    outb(VGA_SEQ_INDEX, 0x05); /* Unlock Registers */
    temp2 = inb(VGA_SEQ_DATA);
    outw(VGA_SEQ_INDEX, 0x8605);

    /* Wait until vertical retrace is in progress. */
    while (inb(vgaIOBase + 0xA) & 0x08);
    while (!(inb(vgaIOBase + 0xA) & 0x08));

    /* Get the result */
    outb(VGA_SEQ_INDEX, 0x11); temp = inb(VGA_SEQ_DATA);

    outw(VGA_SEQ_INDEX, (temp2 << 8) | 0x05);

    return ((temp & 0x02)>>1);
}

/* Auxiliary function to find real memory clock (in Khz) */
int
SiSMclk(SISPtr pSiS)
{ int mclk;
  unsigned char Num, Denum, Base;

    /* Numerator */
    switch (pSiS->Chipset)  {
    case PCI_CHIP_SG86C201:
    case PCI_CHIP_SG86C202:
    case PCI_CHIP_SG86C205:
    case PCI_CHIP_SG86C215:
    case PCI_CHIP_SG86C225:
    case PCI_CHIP_SIS5597:
    case PCI_CHIP_SIS6326:
    case PCI_CHIP_SIS530:
        read_xr(MemClock0,Num);
        mclk=14318*((Num & 0x7f)+1);

        /* Denumerator */
        read_xr(MemClock1,Denum);
        mclk=mclk/((Denum & 0x1f)+1);

        /* Divider. Don't seems to work for mclk */
        if ( (Num & 0x80)!=0 ) { 
            mclk = mclk*2;
        }

        /* Post-scaler. Values depends on SR13 bit 7  */
        outb(VGA_SEQ_INDEX, ClockBase); 
        Base = inb(VGA_SEQ_DATA);

        if ( (Base & 0x80)==0 ) {
            mclk = mclk / (((Denum & 0x60) >> 5)+1);
        }
        else {
            /* Values 00 and 01 are reserved */
            if ((Denum & 0x60) == 0x40) { mclk=mclk/6; }
            if ((Denum & 0x60) == 0x60) { mclk=mclk/8; }
        }
        break;
    case PCI_CHIP_SIS300:
    case PCI_CHIP_SIS540:
    case PCI_CHIP_SIS630:
        /* Numerator */
        read_xr(0x28, Num);
        mclk = 14318*((Num &0x7f)+1);

        /* Denumerator */
        read_xr(0x29, Denum);
        mclk = mclk/((Denum & 0x1f)+1);

        /* Divider */
        if ((Num & 0x80)!=0)  {
            mclk = mclk * 2;
        }

        /* Post-Scaler */
        if ((Denum & 0x80)==0)  {
            mclk = mclk / (((Denum & 0x60) >> 5) + 1);
        }  
        else  {
            mclk = mclk / ((((Denum & 0x60) >> 5) + 1) * 2);
        }
        break;
    default:
        mclk = 0;
    }

    return(mclk);
}

/***** Only for SiS 5597 / 6326 *****/
/* Returns estimated memory bandwidth in Kbits/sec (for dotclock defaults)        */
/* Currently, a very rough estimate (4 cycles / read ; 2 for fast_vram) */
int sisMemBandWidth(ScrnInfoPtr pScrn)
{ int band;
  SISPtr pSiS = SISPTR(pScrn);
  SISRegPtr pReg = &pSiS->ModeReg;

     band=pSiS->MemClock;

   if (((pReg->sisRegs3C4[Mode64] >> 1) & 3) == 0) /* Only 1 bank Vram */
     band = (band * 8);
   else
     band = (band * 16);

   if ((pReg->sisRegs3C4[ExtMiscCont5] & 0xC0) == 0xC0) band=band*2;
     
   return(band);
}

const float     magic300[4] = { 1.2, 1.368421, 2.263158, 1.2};
const float     magic630[4] = { 1.441177, 1.441177, 2.588235, 1.441177 };
int sis300MemBandWidth(ScrnInfoPtr pScrn)
{
        SISPtr          pSiS = SISPTR(pScrn);
        int             bus = pSiS->BusWidth;
        int             mclk = pSiS->MemClock;
        int             bpp = pScrn->bitsPerPixel;
        float           magic, total;

        if (pSiS->Chipset==PCI_CHIP_SIS300)
                magic = magic300[bus/64];
        else
                magic = magic630[bus/64];

        PDEBUG(ErrorF("mclk: %d, bus: %d, magic: %g, bpp: %d\n",
                        mclk, bus, magic, bpp));

        total = mclk*bus/bpp;
        ErrorF("Total Adapter Bandwidth is %gM\n", total/1000);
        if (pSiS->VBFlags & CRT2_ENABLE)  {
                if (total/2 > 540000)
                        total = total - 540000;
                else
                        total = total/2;
                ErrorF("CRT1 Used Bandwidth is %gM\n", total/1000);
        }

        return  (int)(total/magic);
}

void
SISLoadPalette(ScrnInfoPtr pScrn, int numColors, int *indicies, LOCO *colors,
                VisualPtr pVisual)
{
        SISPtr  pSiS = SISPTR(pScrn);
        int     i, index;

        PDEBUG(ErrorF("SISLoadPalette(%d)\n", numColors));
        for (i=0; i<numColors; i++)  {
                index = indicies[i];
                /*outSISREG(0x3c8, index);
                outSISREG(0x3c9, colors[index].red >> 2);
                outSISREG(0x3c9, colors[index].green >> 2);
                outSISREG(0x3c9, colors[index].blue >> 2);*/
                outSISREG(pSiS->RelIO+0x48, index);
                outSISREG(pSiS->RelIO+0x49, colors[index].red >> 2);
                outSISREG(pSiS->RelIO+0x49, colors[index].green >> 2);
                outSISREG(pSiS->RelIO+0x49, colors[index].blue >> 2);
        }
        if (pSiS->VBFlags & CRT2_ENABLE)  {
                (*pSiS->LoadCRT2Palette)(pScrn, numColors, indicies,
                                                colors, pVisual);
        }
}
static  void
SiS301LoadPalette(ScrnInfoPtr pScrn, int numColors, int *indicies,
                                        LOCO *colors, VisualPtr pVisual)
{
        SISPtr  pSiS = SISPTR(pScrn);
        int     i, index;

        PDEBUG(ErrorF("SiS301LoadPalette(%d)\n", numColors));
        for (i=0; i<numColors; i++)  {
                index = indicies[i];
                outSISREG(pSiS->RelIO+0x16, index);
                outSISREG(pSiS->RelIO+0x17, colors[index].red);
                outSISREG(pSiS->RelIO+0x17, colors[index].green);
                outSISREG(pSiS->RelIO+0x17, colors[index].blue);
        }
}


struct QConfig  {
        int     GT;
        int     QC;
};

static  struct  QConfig qconfig[20] = {
        {1, 0x0}, {1, 0x2}, {1, 0x4}, {1, 0x6}, {1, 0x8},
        {1, 0x3}, {1, 0x5}, {1, 0x7}, {1, 0x9}, {1, 0xb},
        {0, 0x0}, {0, 0x2}, {0, 0x4}, {0, 0x6}, {0, 0x8},
        {0, 0x3}, {0, 0x5}, {0, 0x7}, {0, 0x9}, {0, 0xb}};

static  int cycleA[20][2] = {
        {88,88}, {80,80}, {78,78}, {72,72}, {70,70},
        {79,72}, {77,70}, {71,64}, {69,62}, {49,44},
        {73,78}, {65,70}, {63,68}, {57,62}, {55,60},
        {64,62}, {62,60}, {56,54}, {54,52}, {34,34}};

static  void
SiS630Threshold(ScrnInfoPtr pScrn, DisplayModePtr mode,
                                unsigned short *Low, unsigned short *High)
{
        SISPtr          pSiS = SISPTR(pScrn);
        int             mclk = pSiS->MemClock;
        int             vclk = mode->Clock;
        int             bpp = pScrn->bitsPerPixel/8;
        int             buswidth = pSiS->BusWidth;
        CARD32          temp;
        PCITAG          NBridge;
        int             cyclea;
        int             low, lowa;

        int             i, j;


        if (!bpp)       bpp = 1;

        i = 0;
        j = buswidth/128;

        while (1)  {
#ifdef  DEBUG
                ErrorF("Config %d GT = %d, QC = %x, CycleA = %d\n",
                        i, qconfig[i].GT, qconfig[i].QC, cycleA[i][j]);
#endif
                cyclea = cycleA[i][j];
                lowa = cyclea * vclk * bpp;
                lowa = (lowa + (mclk-1)) / mclk;
                lowa = (lowa + 15) / 16;
                low = lowa + 1;
                if (low <= 0x13)
                        break;
                else
                        if (i < 19)
                                i++;
                        else  {
                                low = 0x13;
                                PDEBUG(ErrorF("This mode may has threshold "
                                        "problem and had better removed\n"));
                                break;
                        }
        }
        PDEBUG(ErrorF("Using Config %d with CycleA = %d\n", i, cyclea));
        *Low = low;
        if (lowa+4 > 15)
                *High = 0x0F;
        else
                *High = lowa+4;

        /* write PCI configuration space */
        NBridge = pciTag(0, 0, 0);
        temp = pciReadLong(NBridge, 0x50);
        temp &= 0xF0FFFFFF;
        temp |= qconfig[i].QC << 24;
        pciWriteLong(NBridge, 0x50, temp);

        temp = pciReadLong(NBridge, 0xA0);
        temp &= 0xF0FFFFFF;
        temp |= qconfig[i].GT << 24;
        pciWriteLong(NBridge, 0xA0, temp);


}

struct funcargc {
        char    base;
        char    inc;
};

static  struct funcargc funca[12] = {
        {61, 3}, {52, 5}, {68, 7}, {100, 11},
        {43, 3}, {42, 5}, {54, 7}, {78, 11},
        {34, 3}, {37, 5}, {47, 7}, {67, 11}};
static  struct funcargc funcb[12] = {
        {81, 4}, {72, 6}, {88, 8}, {120, 12},
        {55, 4}, {54, 6}, {66, 8}, {90, 12},
        {42, 4}, {45, 6}, {55, 8}, {75, 12}};
static  char timing[8] = {1, 2, 2, 3, 0, 1, 1, 2};

static  void
SiS300Threshold(ScrnInfoPtr pScrn, DisplayModePtr mode,
                                unsigned short *Low, unsigned short *High)
{
        SISPtr          pSiS = SISPTR(pScrn);
        SISRegPtr       pReg = &pSiS->ModeReg;
        int             mclk = pSiS->MemClock;
        int             vclk = mode->Clock;
        int             bpp = pScrn->bitsPerPixel/8;
        int             lowa, lowb, low;
        struct funcargc *p;
        unsigned int    i, j;

        pReg->sisRegs3C4[0x16] = pSiS->SavedReg.sisRegs3C4[0x16];

        if (!bpp)       bpp = 1;

        do {
                i = GETBITSTR(pReg->sisRegs3C4[0x18], 6:5, 2:1) |
                      GETBITS(pReg->sisRegs3C4[0x18], 1:1);
                j = GETBITSTR(pReg->sisRegs3C4[0x14], 7:6, 3:2) |
                      GETBITS(pReg->sisRegs3C4[0x16], 7:6);
                p = &funca[j];

                lowa = (p->base + p->inc*timing[i])*vclk*bpp;
                lowa = (lowa + (mclk-1)) / mclk;
                lowa = (lowa + 15)/16;

                p = &funcb[j];
                lowb = (p->base + p->inc*timing[i])*vclk*bpp;
                lowb = (lowb + (mclk-1)) / mclk;
                lowb = (lowb + 15)/16;

                if (lowb < 4)
                        lowb = 0;
                else
                        lowb -= 4;

                low = (lowa > lowb)? lowa: lowb;

                low++;

                if (low <= 0x13) {
                        break;
                } else {
                        i = GETBITS(pReg->sisRegs3C4[0x16], 7:6);
                        if (!i) {
                                low = 0x13;
                                break;
                        } else {
                                i--;
                                pReg->sisRegs3C4[0x16] &= 0x3C;
                                pReg->sisRegs3C4[0x16] |= (i << 6);
                        }
                }
        } while (1);

        *Low = low;
        if (low+3 > 15)
                *High = 0x0F;
        else
                *High = low+3;
}

static  void
SiS530Threshold(ScrnInfoPtr pScrn, DisplayModePtr mode,
                                unsigned short *Low, unsigned short *High)
{
        SISPtr  pSiS = SISPTR(pScrn);
        unsigned int    factor, z;
        unsigned int    vclk = mode->Clock,
                        bpp = pScrn->bitsPerPixel,
                        mclk = pSiS->MemClock,
                        buswidth = pSiS->BusWidth;
        
        if (pSiS->Flags & UMA)
                factor = 0x60;
        else
                factor = 0x30;
        z = factor * vclk * bpp;
        z = z / mclk / buswidth;
        *Low = (z+1)/2 + 4;
        if (*Low > 0x1F)
                *Low = 0x1F;

        *High = 0x1F;
}

static  void
SiSThreshold(ScrnInfoPtr pScrn, DisplayModePtr mode,
                                unsigned short *Low, unsigned short *High)
{

}

void SiSIODump(ScrnInfoPtr pScrn)
{       SISPtr          pSiS = SISPTR(pScrn);
        int     i, max3c4, min3d4, max3d4;
        int     SR5State;
        unsigned char   temp;

    switch (pSiS->Chipset)  {
        case PCI_CHIP_SIS6326:
            max3c4 = 0x3F;
            max3d4 = 0x19;
            min3d4 = 0x26;
            break;
        case PCI_CHIP_SIS530:
            max3c4 = 0x3F;
            max3d4 = 0x19;
            min3d4 = 0x26;
            break;
        case PCI_CHIP_SIS300:
        case PCI_CHIP_SIS630:
        case PCI_CHIP_SIS540:
            max3c4 = 0x3D;
            max3d4 = 0x37;
            min3d4 = 0x30;
            break;
        default:
            max3c4 = 0x38;
            max3d4 = 0x19;
            min3d4 = 0x26;
    }
    /* dump Misc Registers */
    /*temp = inb(0x3CC);*/
    temp = inb(pSiS->RelIO+0x4c);
    xf86DrvMsg(pScrn->scrnIndex, X_INFO, "Misc Output 3CC=%x\n", temp);
    /*temp = inb(0x3CA);*/
    temp = inb(pSiS->RelIO+0x4a);
    xf86DrvMsg(pScrn->scrnIndex, X_INFO, "Feature Control 3CA=%x\n", temp);

    /* Dump GR */
    xf86DrvMsg(pScrn->scrnIndex, X_INFO, "-------------\n");
    xf86DrvMsg(pScrn->scrnIndex, X_INFO, "Registers 3CE\n");
    xf86DrvMsg(pScrn->scrnIndex, X_INFO, "-------------\n");
    for (i=0; i<=8; i++)  {
        /*outb(0x3ce, i);
        temp = inb(0x3cf);*/
        outb(pSiS->RelIO+0x4e, i);
        temp = inb(pSiS->RelIO+0x4f);
        xf86DrvMsg(pScrn->scrnIndex, X_INFO, "[%2x]=%2x\n", i, temp);
    }

    /* dump SR0 ~ SR4 */
    xf86DrvMsg(pScrn->scrnIndex, X_INFO, "-------------\n");
    xf86DrvMsg(pScrn->scrnIndex, X_INFO, "Registers 3C4\n");
    xf86DrvMsg(pScrn->scrnIndex, X_INFO, "-------------\n");
    for (i=0; i<=4; i++)  {
        /*outb(0x3c4, i);
        temp = inb(0x3c5);*/
        outb(pSiS->RelIO+0x44, i);
        temp = inb(pSiS->RelIO+0x45);
        xf86DrvMsg(pScrn->scrnIndex, X_INFO, "[%2x]=%2x\n", i, temp);
    }

    /* dump extended SR */
    /*outb(0x3c4, 5);
    SR5State = inb(0x3c5);*/
    outb(pSiS->RelIO+0x44, 5);
    SR5State = inb(pSiS->RelIO+0x45);
    xf86DrvMsg(pScrn->scrnIndex, X_INFO, "[05]=%2x\n", SR5State);
    /*outw(0x3c4, 0x8605);*/
    outw(pSiS->RelIO+0x44, 0x8605);
    for (i=6; i<=max3c4; i++)  {
        /*outb(0x3c4, i);
        temp = inb(0x3c5);*/
        outb(pSiS->RelIO+0x44, i);
        temp = inb(pSiS->RelIO+0x45);
        xf86DrvMsg(pScrn->scrnIndex, X_INFO, "[%2x]=%2x\n", i, temp);
    }

    /* dump CR0 ~ CR18 */
    xf86DrvMsg(pScrn->scrnIndex, X_INFO, "-------------\n");
    xf86DrvMsg(pScrn->scrnIndex, X_INFO, "Registers 3D4\n");
    xf86DrvMsg(pScrn->scrnIndex, X_INFO, "-------------\n");
    for (i=0; i<=0x18; i++)  {
        outb(0x3d4, i);
        temp = inb(0x3d5);
        xf86DrvMsg(pScrn->scrnIndex, X_INFO, "[%2x]=%2x\n", i, temp);
    }
    for (i=min3d4; i<=max3d4; i++)  {   /* dump extended CR */
        outb(0x3d4, i);
        temp = inb(0x3d5);
        xf86DrvMsg(pScrn->scrnIndex, X_INFO, "[%2x]=%2x\n", i, temp);
    }
    /*outw(0x3c4, SR5State << 8 | 0x05);*/
    outw(pSiS->RelIO+0x44, SR5State << 8 | 0x05);
}

void
SISDACPreInit(ScrnInfoPtr pScrn)
{
        SISPtr  pSiS = SISPTR(pScrn);

        switch (pSiS->Chipset)  {
        case PCI_CHIP_SIS630:
        case PCI_CHIP_SIS540:
                pSiS->MaxClock = sis300MemBandWidth(pScrn);
                pSiS->SiSSave           = SiS300Save;
                pSiS->SiSSave2          = SiS301Save;
                pSiS->SiSSaveLVDS       = SiSLVDSSave;
                pSiS->SiSSaveChrontel   = SiSChrontelSave;
                pSiS->SiSRestore        = SiS300Restore;
                pSiS->SiSRestore2       = SiS301Restore;
                pSiS->SiSRestoreLVDS    = SiSLVDSRestore;
                pSiS->SiSRestoreChrontel= SiSChrontelRestore;
                pSiS->LoadCRT2Palette   = SiS301LoadPalette;
                pSiS->SetThreshold      = SiS630Threshold;
                break;
        case PCI_CHIP_SIS300:
                pSiS->MaxClock = sis300MemBandWidth(pScrn);
                pSiS->SiSSave           = SiS300Save;
                pSiS->SiSSave2          = SiS301Save;
                pSiS->SiSSaveLVDS       = SiSLVDSSave;
                pSiS->SiSSaveChrontel   = SiSChrontelSave;
                pSiS->SiSRestore        = SiS300Restore;
                pSiS->SiSRestore2       = SiS301Restore;
                pSiS->SiSRestoreLVDS    = SiSLVDSRestore;
                pSiS->SiSRestoreChrontel= SiSChrontelRestore;
                pSiS->LoadCRT2Palette   = SiS301LoadPalette;
                pSiS->SetThreshold      = SiS300Threshold;
                break;
        case PCI_CHIP_SIS530:
                pSiS->MaxClock = 230000;        /* Guest */
                pSiS->SiSRestore        = SiSRestore;
                pSiS->SiSSave           = SiSSave;
                pSiS->SetThreshold      = SiS530Threshold;
                break;
        case PCI_CHIP_SIS6326:
                pSiS->MaxClock = 175000;        /* Guest */
                pSiS->SiSRestore        = SiSRestore;
                pSiS->SiSSave           = SiSSave;
                pSiS->SetThreshold      = SiSThreshold;
                break;
        default:
                pSiS->MaxClock = 135000;        /* Guest */
                pSiS->SiSRestore        = SiSRestore;
                pSiS->SiSSave           = SiSSave;
                pSiS->SetThreshold      = SiSThreshold;
        }
}

void
SetBlock(CARD16 port, CARD8 from, CARD8 to, CARD8 *DataPtr)
{
        CARD8   index;

        for (index=from; index <= to; index++, DataPtr++)  {
                outSISIDXREG(port, index, *DataPtr);
        }
}

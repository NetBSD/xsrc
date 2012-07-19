/*
 * DAC helper functions (Save/Restore, MemClk, etc)
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
 * Author:  	Thomas Winischhofer <thomas@winischhofer.net>
 *
 * XGI_compute_vclk(), XGICalcClock() and parts of XGIMclk():
 * Copyright (C) 1998, 1999 by Alan Hourihane, Wigan, England
 * Written by:
 *	 Alan Hourihane <alanh@fairlite.demon.co.uk>,
 *       Mike Chapman <mike@paranoia.com>,
 *       Juanjo Santamarta <santamarta@ctv.es>,
 *       Mitani Hiroshi <hmitani@drl.mei.co.jp>,
 *       David Thomas <davtom@dream.org.uk>,
 *	 Thomas Winischhofer <thomas@winischhofer.net>.
 * Licensed under the terms of the XFree86 license
 * (http://www.xfree86.org/current/LICENSE1.html)
 *
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "xf86.h"
#include "xf86_OSproc.h"
#include "xorgVersion.h"
#include "xf86PciInfo.h"
#include "xf86Pci.h"
#include "xf86DDC.h"

#include "xgi.h"
#include "xgi_dac.h"
#include "xgi_regs.h"
#include "xgi_vb.h"

static void Volari_Save(ScrnInfoPtr pScrn, XGIRegPtr xgiReg) ;
static void Volari_Restore(ScrnInfoPtr pScrn, XGIRegPtr xgiReg) ;
static void Volari_Threshold(ScrnInfoPtr pScrn, DisplayModePtr mode,
                                unsigned short *Low, unsigned short *High);
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
XGICalcClock(ScrnInfoPtr pScrn, int clock, int max_VLD, unsigned int *vclk)
{
/*    XGIPtr pXGI = XGIPTR(pScrn); */
    int M, N, P , PSN, VLD , PSNx ;
    int bestM=0, bestN=0, bestP=0, bestPSN=0, bestVLD=0;
    double abest = 42.0;
    double target;
    double Fvco, Fout;
    double error, aerror;
#ifdef DEBUG
    double bestFout;
#endif

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
#define MIN_VCO      Fref
#define MAX_VCO      135000000
#define MAX_VCO_5597 353000000
#define MAX_PSN      0          /* no pre scaler for this chip */
#define TOLERANCE    0.01       /* search smallest M and N in this tolerance */

  int M_min = 2;
  int M_max = 128;

  target = clock * 1000;

     for(PSNx = 0; PSNx <= MAX_PSN ; PSNx++) {

        int low_N, high_N;
        double FrefVLDPSN;

        PSN = !PSNx ? 1 : 4;

        low_N = 2;
        high_N = 32;

        for(VLD = 1 ; VLD <= max_VLD ; VLD++) {

           FrefVLDPSN = (double)Fref * VLD / PSN;

	   for(N = low_N; N <= high_N; N++) {
              double tmp = FrefVLDPSN / N;

              for(P = 1; P <= 4; P++) {
                 double Fvco_desired = target * ( P );
                 double M_desired = Fvco_desired / tmp;

                 /* Which way will M_desired be rounded?
                  *  Do all three just to be safe.
                  */
                 int M_low = M_desired - 1;
                 int M_hi = M_desired + 1;

                 if(M_hi < M_min || M_low > M_max) continue;

		 if(M_low < M_min)  M_low = M_min;

		 if(M_hi > M_max)   M_hi = M_max;

                 for(M = M_low; M <= M_hi; M++) {
                    Fvco = tmp * M;
                    if(Fvco <= MIN_VCO) continue;
                    if(Fvco > MAX_VCO)  break;

                    Fout = Fvco / ( P );

                    error = (target - Fout) / target;
                    aerror = (error < 0) ? -error : error;
                    if(aerror < abest) {
                       abest = aerror;
                       bestM = M;
                       bestN = N;
                       bestP = P;
                       bestPSN = PSN;
                       bestVLD = VLD;
#ifdef DEBUG
                       bestFout = Fout;
#endif
                    }
#ifdef TWDEBUG
                    xf86DrvMsgVerb(pScrn->scrnIndex, X_INFO,3,
			       "Freq. selected: %.2f MHz, M=%d, N=%d, VLD=%d, P=%d, PSN=%d\n",
                               (float)(clock / 1000.), M, N, P, VLD, PSN);
                    xf86DrvMsgVerb(pScrn->scrnIndex, X_INFO,3,
			       "Freq. set: %.2f MHz\n", Fout / 1.0e6);
#endif
                 }
              }
           }
        }
     }

  vclk[Midx]   = bestM;
  vclk[Nidx]   = bestN;
  vclk[VLDidx] = bestVLD;
  vclk[Pidx]   = bestP;
  vclk[PSNidx] = bestPSN;
/*
  PDEBUG(xf86DrvMsgVerb(pScrn->scrnIndex, X_INFO, 3,
                "Freq. selected: %.2f MHz, M=%d, N=%d, VLD=%d, P=%d, PSN=%d\n",
                (float)(clock / 1000.), vclk[Midx], vclk[Nidx], vclk[VLDidx],
                vclk[Pidx], vclk[PSNidx]));
  PDEBUG(xf86DrvMsgVerb(pScrn->scrnIndex, X_INFO, 3,
                "Freq. set: %.2f MHz\n", bestFout / 1.0e6));
  PDEBUG(xf86DrvMsgVerb(pScrn->scrnIndex, X_INFO, 3,
                "VCO Freq.: %.2f MHz\n", bestFout*bestP / 1.0e6));
*/
  
}

static void
Volari_Save(ScrnInfoPtr pScrn, XGIRegPtr xgiReg)
{
    XGIPtr pXGI = XGIPTR(pScrn);
    int vgaIOBase;
    int i; 

    PDEBUG(xf86DrvMsgVerb(pScrn->scrnIndex, X_INFO, 3,
           "Volari_Save(ScrnInfoPtr pScrn, XGIRegPtr xgiReg)\n"));

    vgaHWGetIOBase(VGAHWPTR(pScrn));
    vgaIOBase = VGAHWPTR(pScrn)->IOBase;

#if !defined(__arm__) 
    outw(VGA_SEQ_INDEX, 0x8605);
#else
    moutl(XGISR, 0x8605);
#endif

    for (i = 0x06; i <= 0x3F; i++) {
        /* outb(VGA_SEQ_INDEX, i); */
        outb(XGISR, i);

        /* xf86DrvMsgVerb(pScrn->scrnIndex, X_INFO, 4,
                    "XR%02X Contents - %02X \n", i, inb(VGA_SEQ_DATA));
        xgiReg->xgiRegs3C4[i] = inb(VGA_SEQ_DATA); */
        xf86DrvMsgVerb(pScrn->scrnIndex, X_INFO, 4,
                    "XR%02X Contents - %02X \n", i, inb(XGISR+1));
        xgiReg->xgiRegs3C4[i] = inb(XGISR+1);
    }

    for (i=0x19; i<0x5C; i++)  {
        inXGIIDXREG(XGICR, i, xgiReg->xgiRegs3D4[i]);
    }

    /*xgiReg->xgiRegs3C2 = inb(0x3CC);*/

    xgiReg->xgiRegs3C2 = inb(pXGI->RelIO+0x4c);

    for (i=0x19; i<0x5C; i++)  {
        inXGIIDXREG(XGICR, i, xgiReg->xgiRegs3D4[i]);
    }

// yilin save the VB register
    outXGIIDXREG(XGIPART1, 0x2f, 0x01);
    
    for (i=0; i<0x50; i++)  
    {
        inXGIIDXREG(XGIPART1, i, xgiReg->VBPart1[i]);
    }
    for (i=0; i<0x50; i++)  
    {
        inXGIIDXREG(XGIPART2, i, xgiReg->VBPart2[i]);
    }
    for (i=0; i<0x50; i++)  
    {
        inXGIIDXREG(XGIPART3, i, xgiReg->VBPart3[i]);
    }
    for (i=0; i<0x50; i++)  
    {
        inXGIIDXREG(XGIPART4, i, xgiReg->VBPart4[i]);
    }

    PDEBUG(xf86DrvMsgVerb(pScrn->scrnIndex, X_INFO, 3,
           "Volari_Save(ScrnInfoPtr pScrn, XGIRegPtr xgiReg) Done\n"));

}

static void
Volari_Restore(ScrnInfoPtr pScrn, XGIRegPtr xgiReg)
{
    XGIPtr pXGI = XGIPTR(pScrn);
    int vgaIOBase;
    int i;

	PDEBUG(ErrorF("--- Volari_Restore(). \n")) ;
    xf86DrvMsgVerb(pScrn->scrnIndex, X_INFO, 4,
                "Volari_Restore(ScrnInfoPtr pScrn, XGIRegPtr xgiReg)\n");

    vgaHWGetIOBase(VGAHWPTR(pScrn));
    vgaIOBase = VGAHWPTR(pScrn)->IOBase;

    outXGIIDXREG(XGISR, 0x05, 0x86);


#if 1
	/* Jong@08112009; recover this line */
	/* Volari_DisableAccelerator(pScrn) ; */
#else
    inXGIIDXREG(XGISR, 0x1E, temp);

    if (temp & (SR1E_ENABLE_2D | SR1E_ENABLE_3D)) {
        Volari_Idle(pXGI);
    }

    PDEBUG(XGIDumpRegs(pScrn));

    outXGIIDXREG(XGICR, 0x55, 0);
    andXGIIDXREG(XGISR, 0x1E,
                 ~(SR1E_ENABLE_3D_TRANSFORM_ENGINE
                   | SR1E_ENABLE_2D
                   | SR1E_ENABLE_3D));
    PDEBUG(XGIDumpRegs(pScrn));
#endif

	PDEBUG(XGIDumpRegs(pScrn)) ; //yilin

    for (i = 0x19; i < 0x5C; i++)  {
     /* Jong 09/19/2007; added for ??? */
     if((i!=0x48 && i!=0x4a)|| ((pXGI->Chipset != PCI_CHIP_XGIXG20)&&(pXGI->Chipset != PCI_CHIP_XGIXG21)&&(pXGI->Chipset != PCI_CHIP_XGIXG27)))
        outXGIIDXREG(XGICR, i, xgiReg->xgiRegs3D4[i]);
    }

    for (i = 0x06; i <= 0x3F; i++) {
 /* if( !(i==0x16 || i==0x18 || i==0x19 || i==0x28 || i==0x29 || i==0x2E || i==0x2F) ) { */
        if( !(i==0x16 ) ) {
           /* outb(VGA_SEQ_INDEX,i); */
           outb(XGISR,i);

            /* xf86DrvMsgVerb(pScrn->scrnIndex, X_INFO,4,
                    "XR%X Contents - %02X ", i, inb(VGA_SEQ_DATA));

            outb(VGA_SEQ_DATA,xgiReg->xgiRegs3C4[i]);

            xf86DrvMsgVerb(pScrn->scrnIndex, X_INFO,4,
                        "Restore to - %02X Read after - %02X\n",
                        xgiReg->xgiRegs3C4[i], inb(VGA_SEQ_DATA)); */

            xf86DrvMsgVerb(pScrn->scrnIndex, X_INFO,4,
                    "XR%X Contents - %02X ", i, inb(XGISR+1));

            outb(XGISR+1,xgiReg->xgiRegs3C4[i]);

            xf86DrvMsgVerb(pScrn->scrnIndex, X_INFO,4,
                        "Restore to - %02X Read after - %02X\n",
                        xgiReg->xgiRegs3C4[i], inb(XGISR+1));
        }
    }


#if 0
	// yilin restore the VB register
    outXGIIDXREG(XGIPART1, 0x2f, 0x01);
    for (i=0; i<0x50; i++)  
    {
        outXGIIDXREG(XGIPART1, i, xgiReg->VBPart1[i]);
    }
    for (i=0; i<0x50; i++)  
    {
        outXGIIDXREG(XGIPART2, i, xgiReg->VBPart2[i]);
    }
    for (i=0; i<0x50; i++)  
    {
        outXGIIDXREG(XGIPART3, i, xgiReg->VBPart3[i]);
    }
    for (i=0; i<0x50; i++)  
    {
        outXGIIDXREG(XGIPART4, i, xgiReg->VBPart4[i]);
    }
#endif

    outb(pXGI->RelIO+0x42, xgiReg->xgiRegs3C2);

    /* MemClock needs this to take effect */

#if !defined(__arm__) 
    outw(VGA_SEQ_INDEX, 0x0100); /* Synchronous Reset */
#else
    moutl(XGISR, 0x0100);        /* Synchronous Reset */
#endif

	PDEBUG(XGIDumpRegs(pScrn)) ; //yilin
    xf86DrvMsgVerb(pScrn->scrnIndex, X_INFO, 4,
                "Volari_Restore(ScrnInfoPtr pScrn, XGIRegPtr xgiReg) Done\n");


}

static void
Volari_Threshold(ScrnInfoPtr pScrn, DisplayModePtr mode,
		 unsigned short *Low, unsigned short *High)
{
        XGIPtr pXGI = XGIPTR(pScrn);

        orXGIIDXREG(XGISR, 0x3D, 0x01);
}


/**
 * Calculate available memory bandwidth for an XG40 series chip.
 *
 * \sa XG40_MemBandWidth
 */
static int XG40_MemBandWidth(ScrnInfoPtr pScrn)
{
    static const float magic315[4] = { 1.2, 1.368421, 2.263158, 1.2 };
    XGIPtr pXGI = XGIPTR(pScrn);
    const int bus = (pXGI->BusWidth > 128) ? 128 : pXGI->BusWidth;
    const int mclk = pXGI->MemClock;
    const int bpp = pScrn->bitsPerPixel;
    const float magic = magic315[bus / 64];
    float  total  = (mclk * bus) / bpp;

    PDEBUG5(ErrorF("mclk: %d, bus: %d, magic: %f, bpp: %d\n",
                   mclk, bus, magic, bpp));
    PDEBUG5(ErrorF("Total Adapter Bandwidth is %fM\n", total/1000));

    if (pXGI->VBFlags & CRT2_ENABLE)  {
	total = ((total / 2) > 540000)
	    ? (total - 540000) : (total / 2);
    }

    return  (int)(total / magic);
}


/**
 * Calculate available memory bandwidth for an XG20 series chip.
 *
 * \sa XG20_MemBandWidth
 */
static int XG20_MemBandWidth(ScrnInfoPtr pScrn)
{
    XGIPtr pXGI = XGIPTR(pScrn);
    const int bus = (pXGI->BusWidth > 128) ? 128 : pXGI->BusWidth;
    const int mclk = pXGI->MemClock;
    const int bpp = pScrn->bitsPerPixel;
    const float magic = 1.44;
    float  total  = (mclk * bus) / bpp;

    /* Jong 09/19/2007; support DDRII and double pixel clock */
    unsigned long SR39, CR97 ;

    PDEBUG5(ErrorF("mclk: %d, bus: %d, magic: %f, bpp: %d\n",
                   mclk, bus, magic, bpp));

    total = mclk*bus/bpp;

    /* Jong 04/26/2007; support DDRII and double pixel clock */
    /*-------------------------------------------------------*/
    inXGIIDXREG(XGISR, 0x39, SR39);
    inXGIIDXREG(XGICR, 0x97, CR97);

	/* Jong@09082009; modify for XG27 */
    if(pXGI->Chipset == PCI_CHIP_XGIXG27)
	{
		if (CR97 & 0xC1)
   			total *= 2;
	}
	else /* XG20/21 */
	{
		if (CR97 & 0x10)
		{
   			if (CR97 & 0x01)
			{
    			total *= 2;
			}
		}
		else
		{
			if (SR39 & 0x2)
 			{
  				total *= 2;
			}
		}
	}
    /*-------------------------------------------------------*/

    PDEBUG5(ErrorF("Total Adapter Bandwidth is %fM\n", total/1000));

    return (int)(total / magic);
}

extern unsigned int g_GammaRed;
extern unsigned int g_GammaGreen;
extern unsigned int g_GammaBlue;

void XGIAdjustGamma(ScrnInfoPtr pScrn, unsigned int gammaRed, unsigned int gammaGreen, unsigned int gammaBlue)
{
    XGIPtr  pXGI = XGIPTR(pScrn);
    int num = 255, i;
	double red = 1.0 / (double)((double)gammaRed / 1000);
    double green = 1.0 / (double)((double)gammaGreen / 1000);
    double blue = 1.0 / (double)((double)gammaBlue / 1000);
	CARD8  GammaRampRed[256], GammaRampGreen[256], GammaRampBlue[256];

    for(i = 0; i <= num; i++) {
        GammaRampRed[i] =
	    (red == 1.0) ? i : (CARD8)(pow((double)i / (double)num, red) * (double)num + 0.5);

		GammaRampGreen[i] =
	    (green == 1.0) ? i : (CARD8)(pow((double)i / (double)num, green) * (double)num + 0.5);

		GammaRampBlue[i] =
	    (blue == 1.0) ? i : (CARD8)(pow((double)i / (double)num, blue) * (double)num + 0.5);
    }

	/* set gamma ramp to HW */
    for(i = 0; i <= 255; i++) {
       MMIO_OUT32(pXGI->IOBase, 0x8570,
       		(i << 24)					|
			(GammaRampBlue[i] << 16)	|
			(GammaRampGreen[i] << 8)	|
			GammaRampRed[i]);
    }
}

void
XGILoadPalette(ScrnInfoPtr pScrn, int numColors, int *indices, LOCO *colors,
               VisualPtr pVisual)
{
     XGIPtr  pXGI = XGIPTR(pScrn);
     int     i, j, index;
/*     unsigned char backup = 0; */
     unsigned char SR7;
     Bool    dogamma1 = pXGI->CRT1gamma;
/*     Bool    resetxvgamma = FALSE; */

    if (IS_DUAL_HEAD(pXGI)) {
        XGIEntPtr pXGIEnt = ENTITY_PRIVATE(pXGI);
        dogamma1 = pXGIEnt->CRT1gamma;
    }

     PDEBUG(ErrorF("xgiLoadPalette(%d)\n", numColors));

     if (!IS_DUAL_HEAD(pXGI) || IS_SECOND_HEAD(pXGI)) {
        switch(pXGI->CurrentLayout.depth) {
#ifdef XGIGAMMA
          case 15:
	     if(dogamma1) {
	        orXGIIDXREG(XGISR, 0x07, 0x04);
	        for(i=0; i<numColors; i++) {
                   index = indices[i];
		   if(index < 32) {   /* Paranoia */
		      for(j=0; j<8; j++) {
		         outXGIREG(XGICOLIDX, (index * 8) + j);
                         outXGIREG(XGICOLDATA, colors[index].red << (8- pScrn->rgbBits));
                         outXGIREG(XGICOLDATA, colors[index].green << (8 - pScrn->rgbBits));
                         outXGIREG(XGICOLDATA, colors[index].blue << (8 - pScrn->rgbBits));
		      }
		   }
                }
	     } else {
	        andXGIIDXREG(XGISR, 0x07, ~0x04);
	     }
	     break;
	  case 16:
	     if(dogamma1) {
		orXGIIDXREG(XGISR, 0x07, 0x04);
                inXGIIDXREG(XGISR, 0x07, SR7);
PDEBUG(ErrorF("\ndogamma1 SR7=%x ", SR7));
		for(i=0; i<numColors; i++) {
                   index = indices[i];
		   if(index < 64) {  /* Paranoia */
		      for(j=0; j<4; j++) {
		         outXGIREG(XGICOLIDX, (index * 4) + j);
                         outXGIREG(XGICOLDATA, colors[index/2].red << (8 - pScrn->rgbBits));
                         outXGIREG(XGICOLDATA, colors[index].green << (8 - pScrn->rgbBits));
                         outXGIREG(XGICOLDATA, colors[index/2].blue << (8 - pScrn->rgbBits));
		      }
		   }
                }
	     } else {
	        andXGIIDXREG(XGISR, 0x07, ~0x04);
	     }
	     break;
          case 24:
	     if(dogamma1) {
	        orXGIIDXREG(XGISR, 0x07, 0x04);
                for(i=0; i<numColors; i++)  {
                   index = indices[i];
		   if(index < 256) {   /* Paranoia */
                      outXGIREG(XGICOLIDX, index);
                      outXGIREG(XGICOLDATA, colors[index].red);
                      outXGIREG(XGICOLDATA, colors[index].green);
                      outXGIREG(XGICOLDATA, colors[index].blue);
		   }
                }
	     } else {
	        andXGIIDXREG(XGISR, 0x07, ~0x04);
	     }
	     break;
#endif
	  default:
	     if((pScrn->rgbBits == 8) && (dogamma1))
	        orXGIIDXREG(XGISR, 0x07, 0x04);
	     else
	        andXGIIDXREG(XGISR, 0x07, ~0x04);
             for(i=0; i<numColors; i++)  {
                index = indices[i];
                outXGIREG(XGICOLIDX, index);
                outXGIREG(XGICOLDATA, colors[index].red >> (8 - pScrn->rgbBits));
                outXGIREG(XGICOLDATA, colors[index].green >> (8 - pScrn->rgbBits));
                outXGIREG(XGICOLDATA, colors[index].blue >> (8 - pScrn->rgbBits));
             }
	}

    }

    if (!IS_DUAL_HEAD(pXGI) || !IS_SECOND_HEAD(pXGI)) {

    }

	if(pXGI->CurrentLayout.depth != 8)
		XGIAdjustGamma(pScrn, g_GammaRed, g_GammaGreen, g_GammaBlue);
}

void
XGIDACPreInit(ScrnInfoPtr pScrn)
{
    XGIPtr  pXGI = XGIPTR(pScrn);

PDEBUG(ErrorF("XGIDACPreInit()\n"));

    pXGI->XGISave = Volari_Save;
    pXGI->XGIRestore = Volari_Restore;
    pXGI->SetThreshold = Volari_Threshold;

    pXGI->MaxClock = ((pXGI->Chipset == PCI_CHIP_XGIXG20) || (pXGI->Chipset == PCI_CHIP_XGIXG21) || (pXGI->Chipset == PCI_CHIP_XGIXG27))
	? XG20_MemBandWidth(pScrn) : XG40_MemBandWidth(pScrn);
}


int
XG40Mclk(XGIPtr pXGI)
{
    int mclk;
    unsigned char Num, Denum;

    /* Numerator */
    /* inXGIIDXREG(0x3c4, 0x28, Num); */
    inXGIIDXREG(XGISR, 0x28, Num);
    mclk = 14318 * ((Num & 0x7f) + 1);

    /* Denumerator */
    /* inXGIIDXREG(0x3c4, 0x29, Denum); */
    inXGIIDXREG(XGISR, 0x29, Denum);
    mclk = mclk / ((Denum & 0x1f) + 1);

    /* Divider */
    if ((Num & 0x80)!=0)  {
	mclk = mclk * 2;
    }

    /* Post-Scaler */
    mclk /= ((Denum & 0x80) == 0) 
	?  (((Denum & 0x60) >> 5) + 1)
	: ((((Denum & 0x60) >> 5) + 1) * 2);

    return mclk;
}


static int
retrace_signals_active(XGIIOADDRESS RelIO)
{
    unsigned char temp;

    /* Make sure the vertical retrace signal is enabled.
     */
    inXGIIDXREG(RelIO + CROFFSET, 0x17, temp);
    if (!(temp & 0x80)) 
	return 0;

    /* FIXME: Magic offset, what are you?
     */
    inXGIIDXREG(RelIO + SROFFSET, 0x1f, temp);
    if(temp & 0xc0) 
	return 0;

    return 1;
}


/**
 * Wait for beginning of next vertical retrace.
 * 
 * \bugs
 * The functions \c XGI_WaitBeginRetrace and \c XGI_WaitEndRetrace are
 * nearly identical.  Are both \b really necessary?
 */
void
XGI_WaitBeginRetrace(XGIIOADDRESS RelIO)
{
    int           watchdog;

    if (retrace_signals_active(RelIO)) {
	/* Wait for the CRTC to leave then re-enter the vertical retrace
	 * period.
	 */
	watchdog = 65536;
	while ((inXGIREG(RelIO + INPUTSTATOFFSET) & IS_BIT_VERT_ACTIVE) && --watchdog)
	    /* empty */ ;

	watchdog = 65536;
	while ((!(inXGIREG(RelIO + INPUTSTATOFFSET) & IS_BIT_VERT_ACTIVE)) && --watchdog)
	    /* empty */ ;
    }
}


/**
 * Wait for end of next vertical retrace.
 * 
 * \bugs
 * The functions \c XGI_WaitBeginRetrace and \c XGI_WaitEndRetrace are
 * nearly identical.  Are both \b really necessary?
 */
void
XGI_WaitEndRetrace(XGIIOADDRESS RelIO)
{
    int           watchdog;

    if (retrace_signals_active(RelIO)) {
	/* Wait for the CRTC to enter then leave the vertical retrace
	 * period.
	 */
	watchdog = 65536;
	while ((!(inXGIREG(RelIO + INPUTSTATOFFSET) & IS_BIT_VERT_ACTIVE)) && --watchdog)
	    /* empty */ ;

	watchdog = 65536;
	while ((inXGIREG(RelIO + INPUTSTATOFFSET) & IS_BIT_VERT_ACTIVE) && --watchdog)
	    /* empty */ ;
    }
}

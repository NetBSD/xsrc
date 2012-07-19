/*
 * Video bridge detection and configuration for 300, 315 and 330 series
 *
 * Copyright (C) 2001-2004 by Thomas Winischhofer, Vienna, Austria
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
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "xf86.h"
#include "compiler.h"
#include "xf86PciInfo.h"

#include "xgi.h"
#include "xgi_regs.h"
#include "xgi_vb.h"
#include "xgi_dac.h"
#include "vb_def.h"

extern unsigned char XGI_GetSetBIOSScratch(ScrnInfoPtr pScrn, USHORT offset, unsigned char value);

static Bool
TestDDC1(ScrnInfoPtr pScrn)
{
/*    XGIPtr  pXGI = XGIPTR(pScrn); 
    unsigned short old;  */
    int count = 48;
/*
    old = XGI_ReadDDC1Bit(pXGI->XGI_Pr);
    do {
       if(old != XGI_ReadDDC1Bit(pXGI->XGI_Pr)) break;
    } while(count--);
*/
    return (count == -1) ? FALSE : TRUE;
}

static int
XGI_XGIDetectCRT1(ScrnInfoPtr pScrn)
{
    XGIPtr  pXGI = XGIPTR(pScrn);
    unsigned short temp = 0xffff;
    unsigned char SR1F, CR17;
    int i, ret = 0;
    Bool mustwait = FALSE;

    inXGIIDXREG(XGISR,0x1F,SR1F);
    orXGIIDXREG(XGISR,0x1F,0x04);
    andXGIIDXREG(XGISR,0x1F,0x3F);
    if(SR1F & 0xc0) mustwait = TRUE;

    inXGIIDXREG(XGICR,0x17,CR17);
    CR17 &= 0x80;
    if(!CR17) {
       orXGIIDXREG(XGICR,0x17,0x80);
       mustwait = TRUE;
       outXGIIDXREG(XGISR, 0x00, 0x01);
       outXGIIDXREG(XGISR, 0x00, 0x03);
    }

    if(mustwait) {
       for(i=0; i < 10; i++) XGI_WaitBeginRetrace(pXGI->RelIO);
    }

    if((temp == 0) || (temp == 0xffff)) {
       if(TestDDC1(pScrn)) temp = 1;
    }

    if((temp) && (temp != 0xffff)) {
       orXGIIDXREG(XGICR,0x32,0x20);
       ret = 1;
    }

    setXGIIDXREG(XGICR,0x17,0x7F,CR17);

    outXGIIDXREG(XGISR,0x1F,SR1F);

    return ret;
}

/* Detect CRT1 */
void XGICRT1PreInit(ScrnInfoPtr pScrn)
{
    XGIPtr  pXGI = XGIPTR(pScrn);
    unsigned char CR32;
    unsigned char CRT1Detected;

    if (!(pXGI->XGI_Pr->VBType & VB_XGIVB) || IS_DUAL_HEAD(pXGI)
#ifdef XGIMERGED
       || ((pXGI->MergedFB) && (!(pXGI->MergedFBAuto)))
#endif
       ) {
       pXGI->CRT1off = 0;
       return;
    }

    inXGIIDXREG(XGICR, 0x32, CR32);

    CRT1Detected = (CR32 & 0x20) ? 1 : XGI_XGIDetectCRT1(pScrn);

    if (pXGI->CRT1off == -1) {
	const int OtherDevices = ((CR32 & 0x5F) != 0);

	/* If no CRT1 detected and other devices exist, switch CRT1 off.
	 * Otherwise, switch it on.
	 */
	pXGI->CRT1off = (!CRT1Detected && OtherDevices) ? 1 : 0;
    }

    xf86DrvMsg(pScrn->scrnIndex, X_PROBED,
    		"%sCRT1 (VGA) connection detected\n",
		CRT1Detected ? "" : "No ");
}

/* Detect CRT2-LCD and LCD size */
void XGILCDPreInit(ScrnInfoPtr pScrn)
{
    XGIPtr  pXGI = XGIPTR(pScrn);
    unsigned char CR32;

    pXGI->LCDwidth = 0;

    /* if(!(pXGI->XGI_Pr->VBType & VB_XGIVB)) return; */

    inXGIIDXREG(XGICR, 0x32, CR32);

    if(CR32 & 0x08) pXGI->VBFlags |= CRT2_LCD;
	if( pXGI->xgi_HwDevExt.jChipType == XG21)
	{
    	pXGI->VBFlags |= CRT2_LCD;
		CR32 |= 0x08 ;
    	outXGIIDXREG(XGICR, 0x32, CR32);
	}

    /* If no panel has been detected by the BIOS during booting,
     * we try to detect it ourselves at this point. We do that
     * if forcecrt2redetection was given, too.
     * This is useful on machines with DVI connectors where the
     * panel was connected after booting. This is only supported
     * on the 315/330 series and the 301/30xB/C bridge (because the
     * 30xLV don't seem to have a DDC port and operate only LVDS
     * panels which mostly don't support DDC). We only do this if
     * there was no secondary VGA detected by the BIOS, because LCD
     * and VGA2 share the same DDC channel and might be misdetected
     * as the wrong type (especially if the LCD panel only supports
     * EDID Version 1).
     *
     * By default, CRT2 redetection is forced since 12/09/2003, as
     * I encountered numerous panels which deliver more or less
     * bogus DDC data confusing the BIOS. Since our DDC detection
     * is waaaay better, we prefer it instead of the primitive
     * and buggy BIOS method.
     */
    if (!IS_DUAL_HEAD(pXGI) || !IS_SECOND_HEAD(pXGI)) {

    }
}

/* Detect CRT2-TV connector type and PAL/NTSC flag */
void XGITVPreInit(ScrnInfoPtr pScrn)
{
    XGIPtr  pXGI = XGIPTR(pScrn);
    unsigned char SR16, SR38, CR32, CR35=0, CR38=0, CR39;
    int temp = 0;

    if(!(pXGI->XGI_Pr->VBType & VB_XGIVB)) return;

    inXGIIDXREG(XGICR, 0x32, CR32);
    inXGIIDXREG(XGICR, 0x35, CR35);
    inXGIIDXREG(XGISR, 0x16, SR16);
    inXGIIDXREG(XGISR, 0x38, SR38);

    if(temp) {
       inXGIIDXREG(XGICR, temp, CR38);
    }

#ifdef TWDEBUG
    xf86DrvMsg(pScrn->scrnIndex, X_PROBED,
    	"(vb.c: CR32=%02x SR16=%02x SR38=%02x)\n",
	CR32, SR16, SR38);
#endif

    if(CR32 & 0x47) pXGI->VBFlags |= CRT2_TV;

    if(pXGI->XGI_SD_Flags & XGI_SD_SUPPORTYPBPR) {
       if(CR32 & 0x80) pXGI->VBFlags |= CRT2_TV;
    } else {
       CR32 &= 0x7f;
    }

    if(CR32 & 0x01)
       pXGI->VBFlags |= TV_AVIDEO;
    else if(CR32 & 0x02)
       pXGI->VBFlags |= TV_SVIDEO;
    else if(CR32 & 0x04)
       pXGI->VBFlags |= TV_SCART;
    else if((CR32 & 0x40) && (pXGI->XGI_SD_Flags & XGI_SD_SUPPORTHIVISION))
       pXGI->VBFlags |= (TV_HIVISION | TV_PAL);
    else if((CR32 & 0x80) && (pXGI->XGI_SD_Flags & XGI_SD_SUPPORTYPBPR)) {
       pXGI->VBFlags |= TV_YPBPR;
       if(pXGI->XGI_SD_Flags & XGI_SD_SUPPORTYPBPR) {
          if(CR38 & 0x08) {
	     switch(CR38 & 0x30) {
	     case 0x10: pXGI->VBFlags |= TV_YPBPR525P; break;
	     case 0x20: pXGI->VBFlags |= TV_YPBPR750P; break;
	     case 0x30: pXGI->VBFlags |= TV_YPBPR1080I; break;
	     default:   pXGI->VBFlags |= TV_YPBPR525I;
	     }
	  } else        pXGI->VBFlags |= TV_YPBPR525I;
	  if(pXGI->XGI_SD_Flags & XGI_SD_SUPPORTYPBPRAR) {
             inXGIIDXREG(XGICR,0x3B,CR39);
	     CR39 &= 0x03;
	     if(CR39 == 0x00)      pXGI->VBFlags |= TV_YPBPR43LB;
	     else if(CR39 == 0x01) pXGI->VBFlags |= TV_YPBPR169;
	     else if(CR39 == 0x03) pXGI->VBFlags |= TV_YPBPR43;
	  }
       }
    } 


    if(pXGI->VBFlags & (TV_SCART|TV_SVIDEO|TV_AVIDEO)) {
       xf86DrvMsg(pScrn->scrnIndex, X_PROBED, "Detected default TV standard %s\n",
          (pXGI->VBFlags & TV_NTSC) ?
	     ((pXGI->VBFlags & TV_NTSCJ) ? "NTSCJ" : "NTSC") :
	         ((pXGI->VBFlags & TV_PALM) ? "PALM" :
		     ((pXGI->VBFlags & TV_PALN) ? "PALN" : "PAL")));
    }

    if(pXGI->VBFlags & TV_HIVISION) {
       xf86DrvMsg(pScrn->scrnIndex, X_PROBED, "BIOS reports HiVision TV\n");
    }


    if(pXGI->VBFlags & TV_YPBPR) {
       xf86DrvMsg(pScrn->scrnIndex, X_PROBED, "Detected YPbPr TV (by default %s)\n",
         (pXGI->VBFlags & TV_YPBPR525I) ? "480i" :
	     ((pXGI->VBFlags & TV_YPBPR525P) ? "480p" :
	        ((pXGI->VBFlags & TV_YPBPR750P) ? "720p" : "1080i")));
    }
}



/* Detect CRT2 */
void XGICRT2PreInit(ScrnInfoPtr pScrn)
{
    XGIPtr  pXGI = XGIPTR(pScrn);
    unsigned char CR32;
    unsigned char CRT2Detected = 0;

    inXGIIDXREG(XGICR, 0x32, CR32);

    if(CR32 & 0x10)    
    {
	pXGI->VBFlags =pXGI->VBFlags | CRT2_VGA;
	CRT2Detected = 1;
    }
    
    xf86DrvMsg(pScrn->scrnIndex, X_PROBED,"%sCRT2 (VGA) connection detected\n",	    CRT2Detected ? "" : "No ");
}



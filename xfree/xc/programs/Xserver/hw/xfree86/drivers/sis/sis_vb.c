/* $XFree86: xc/programs/Xserver/hw/xfree86/drivers/sis/sis_vb.c,v 1.48 2004/07/07 21:20:41 twini Exp $ */
/* $XdotOrg$ */
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

#include "xf86.h"
#include "xf86_ansic.h"
#include "compiler.h"
#include "xf86PciInfo.h"

#include "sis.h"
#include "sis_regs.h"
#include "sis_vb.h"
#include "sis_dac.h"

extern BOOLEAN SiS_GetPanelID(SiS_Private *SiS_Pr, PSIS_HW_INFO HwDeviceExtension);
extern USHORT  SiS_SenseLCDDDC(SiS_Private *SiS_Pr, SISPtr pSiS);
extern USHORT  SiS_SenseVGA2DDC(SiS_Private *SiS_Pr, SISPtr pSiS);

extern void    SISDetermineLCDACap(ScrnInfoPtr pScrn);
extern void    SISSaveDetectedDevices(ScrnInfoPtr pScrn);

extern void    	     SISWaitRetraceCRT1(ScrnInfoPtr pScrn);
extern unsigned char SiS_GetSetBIOSScratch(ScrnInfoPtr pScrn, USHORT offset, unsigned char value);

static const SiS_LCD_StStruct SiS300_LCD_Type[]=
{
        { VB_LCD_1024x768, 1024,  768 },  /* 0 - invalid */
	{ VB_LCD_800x600,   800,  600 },  /* 1 */
	{ VB_LCD_1024x768, 1024,  768 },  /* 2 */
	{ VB_LCD_1280x1024,1280, 1024 },  /* 3 */
	{ VB_LCD_1280x960, 1280,  960 },  /* 4 */
	{ VB_LCD_640x480,   640,  480 },  /* 5 */
	{ VB_LCD_1024x600, 1024,  600 },  /* 6 */
	{ VB_LCD_1152x768, 1152,  768 },  /* 7 */
	{ VB_LCD_1024x768, 1024,  768 },  /* 8 */
	{ VB_LCD_1024x768, 1024,  768 },  /* 9 */
	{ VB_LCD_1280x768, 1280,  768 },  /* a */
	{ VB_LCD_1024x768, 1024,  768 },  /* b */
	{ VB_LCD_1024x768, 1024,  768 },  /* c */
	{ VB_LCD_1024x768, 1024,  768 },  /* d */
	{ VB_LCD_320x480,   320,  480 },  /* e */
	{ VB_LCD_CUSTOM,      0,    0 }   /* f */
};

static const SiS_LCD_StStruct SiS315_LCD_Type[]=
{
        { VB_LCD_1024x768, 1024,  768 },  /* 0 - invalid */
	{ VB_LCD_800x600,   800,  600 },  /* 1 */
	{ VB_LCD_1024x768, 1024,  768 },  /* 2 */
	{ VB_LCD_1280x1024,1280, 1024 },  /* 3 */
	{ VB_LCD_640x480,   640,  480 },  /* 4 */
	{ VB_LCD_1024x600, 1024,  600 },  /* 5 */
	{ VB_LCD_1152x864, 1152,  864 },  /* 6 */
	{ VB_LCD_1280x960, 1280,  960 },  /* 7 */
	{ VB_LCD_1152x768, 1152,  768 },  /* 8 */
	{ VB_LCD_1400x1050,1400, 1050 },  /* 9 */
	{ VB_LCD_1280x768, 1280,  768 },  /* a */
	{ VB_LCD_1600x1200,1600, 1200 },  /* b */
	{ VB_LCD_640x480_2, 640,  480 },  /* c DSTN/FSTN */
	{ VB_LCD_640x480_3, 640,  480 },  /* d DSTN/FSTN */
	{ VB_LCD_320x480,   320,  480 },  /* e */
	{ VB_LCD_CUSTOM,      0,    0 }   /* f */
};

static const SiS_LCD_StStruct SiS661_LCD_Type[]=
{
        { VB_LCD_1024x768, 1024,  768 },  /* 0 - invalid */
	{ VB_LCD_800x600,   800,  600 },  /* 1 */
	{ VB_LCD_1024x768, 1024,  768 },  /* 2 */
	{ VB_LCD_1280x1024,1280, 1024 },  /* 3 */
	{ VB_LCD_640x480,   640,  480 },  /* 4 */
	{ VB_LCD_1024x600, 1024,  600 },  /* 5 */
	{ VB_LCD_1152x864, 1152,  864 },  /* 6 */
	{ VB_LCD_1280x960, 1280,  960 },  /* 7 */
	{ VB_LCD_1152x768, 1152,  768 },  /* 8 */
	{ VB_LCD_1400x1050,1400, 1050 },  /* 9 */
	{ VB_LCD_1280x768, 1280,  768 },  /* a */
	{ VB_LCD_1600x1200,1600, 1200 },  /* b */
	{ VB_LCD_1280x800, 1280,  800 },  /* c */
	{ VB_LCD_1680x1050,1680, 1050 },  /* d */
	{ VB_LCD_1280x720, 1280,  720 },  /* e */
	{ VB_LCD_CUSTOM,      0,    0 }   /* f */
};

static Bool
TestDDC1(ScrnInfoPtr pScrn)
{
    SISPtr  pSiS = SISPTR(pScrn);
    unsigned short old;
    int count = 48;

    old = SiS_ReadDDC1Bit(pSiS->SiS_Pr);
    do {
       if(old != SiS_ReadDDC1Bit(pSiS->SiS_Pr)) break;
    } while(count--);
    return (count == -1) ? FALSE : TRUE;
}

static int
SiS_SISDetectCRT1(ScrnInfoPtr pScrn)
{
    SISPtr  pSiS = SISPTR(pScrn);
    unsigned short temp = 0xffff;
    unsigned char SR1F, CR63=0, CR17;
    int i, ret = 0;
    Bool mustwait = FALSE;

    inSISIDXREG(SISSR,0x1F,SR1F);
    setSISIDXREG(SISSR,0x1F,0x3f,0x04);
    if(SR1F & 0xc0) mustwait = TRUE;

    if(pSiS->VGAEngine == SIS_315_VGA) {
       inSISIDXREG(SISCR,pSiS->myCR63,CR63);
       CR63 &= 0x40;
       andSISIDXREG(SISCR,pSiS->myCR63,0xbf);
    }

    inSISIDXREG(SISCR,0x17,CR17);
    CR17 &= 0x80;
    if(!CR17) {
       orSISIDXREG(SISCR,0x17,0x80);
       mustwait = TRUE;
       outSISIDXREG(SISSR, 0x00, 0x01);
       outSISIDXREG(SISSR, 0x00, 0x03);
    }

    if(mustwait) {
       for(i=0; i < 10; i++) SISWaitRetraceCRT1(pScrn);
    }
    
    if(pSiS->sishw_ext.jChipType >= SIS_330) {
       if(pSiS->sishw_ext.jChipType >= SIS_340) {
          outSISIDXREG(SISCR, 0x57, 0x4a);
       } else {
          outSISIDXREG(SISCR, 0x57, 0x5f);
       }
       orSISIDXREG(SISCR, 0x53, 0x02);
       while((inSISREG(SISINPSTAT)) & 0x01)    break;
       while(!((inSISREG(SISINPSTAT)) & 0x01)) break;
       if((inSISREG(SISMISCW)) & 0x10) temp = 1;
       andSISIDXREG(SISCR, 0x53, 0xfd);
       andSISIDXREG(SISCR, 0x57, 0x00);
#ifdef TWDEBUG
       xf86DrvMsg(0, X_INFO, "330: Found CRT1: %s\n", (temp == 1) ? "yes" : "no");
#endif       
    }

    if(temp == 0xffff) {
       i = 3;
       do {
          temp = SiS_HandleDDC(pSiS->SiS_Pr, pSiS->VBFlags, pSiS->VGAEngine, 0, 0, NULL);
       } while(((temp == 0) || (temp == 0xffff)) && i--);

       if((temp == 0) || (temp == 0xffff)) {
          if(TestDDC1(pScrn)) temp = 1;
       }
    }
    
    if((temp) && (temp != 0xffff)) {
       orSISIDXREG(SISCR,0x32,0x20);
       ret = 1;
    } else if(pSiS->sishw_ext.jChipType >= SIS_330) {
       andSISIDXREG(SISCR,0x32,~0x20);
       ret = 0;
    }

    if(pSiS->VGAEngine == SIS_315_VGA) {
       setSISIDXREG(SISCR,pSiS->myCR63,0xBF,CR63);
    }

    setSISIDXREG(SISCR,0x17,0x7F,CR17);

    outSISIDXREG(SISSR,0x1F,SR1F);

    return ret;
}

/* Detect CRT1 */
void SISCRT1PreInit(ScrnInfoPtr pScrn)
{
    SISPtr  pSiS = SISPTR(pScrn);
    unsigned char CR32;
    unsigned char CRT1Detected = 0;
    unsigned char OtherDevices = 0;

    if(!(pSiS->VBFlags & VB_VIDEOBRIDGE)) {
       pSiS->CRT1off = 0;
       return;
    }

#ifdef SISDUALHEAD
    if(pSiS->DualHeadMode) {
       pSiS->CRT1off = 0;
       return;
    }
#endif

#ifdef SISMERGED
    if((pSiS->MergedFB) && (!(pSiS->MergedFBAuto))) {
       pSiS->CRT1off = 0;
       return;
    }
#endif

    inSISIDXREG(SISCR, 0x32, CR32);

    if(pSiS->sishw_ext.jChipType >= SIS_330) {
       /* Works reliably on 330 and later */
       CRT1Detected = SiS_SISDetectCRT1(pScrn);
    } else {
       if(CR32 & 0x20)  CRT1Detected = 1;
       else CRT1Detected = SiS_SISDetectCRT1(pScrn);
    }

    if(CR32 & 0x5F)  OtherDevices = 1;

    if(pSiS->CRT1off == -1) {
       if(!CRT1Detected) {

          /* No CRT1 detected. */
	  /* If other devices exist, switch it off */
	  if(OtherDevices) pSiS->CRT1off = 1;
	  else             pSiS->CRT1off = 0;

       } else {

          /* CRT1 detected, leave/switch it on */
	  pSiS->CRT1off = 0;

       }
    }

    xf86DrvMsg(pScrn->scrnIndex, X_PROBED,
    		"%sCRT1/VGA detected\n",
		CRT1Detected ? "" : "No ");
}

/* Detect CRT2-LCD and LCD size */
void SISLCDPreInit(ScrnInfoPtr pScrn, Bool quiet)
{
    SISPtr  pSiS = SISPTR(pScrn);
    unsigned char CR32, CR36, CR37, CR7D=0, tmp;
    
    pSiS->VBFlags &= ~(CRT2_LCD);
    pSiS->VBLCDFlags = 0;
    pSiS->LCDwidth   = 0;
    pSiS->LCDheight  = 0;

    if(!(pSiS->VBFlags & VB_VIDEOBRIDGE)) return;

    inSISIDXREG(SISCR, 0x32, CR32);
   
    if(CR32 & 0x08) pSiS->VBFlags |= CRT2_LCD;
    
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
#ifdef SISDUALHEAD
    if((!pSiS->DualHeadMode) || (!pSiS->SecondHead)) {
#endif 
       if((pSiS->VGAEngine == SIS_315_VGA) &&
          (pSiS->VBFlags & VB_SISTMDSBRIDGE) &&
          (!(pSiS->VBFlags & VB_30xBDH)) &&
	  (pSiS->VESA != 1)) {

          if(pSiS->forcecrt2redetection) {
             pSiS->VBFlags &= ~CRT2_LCD;
          }

          if(!(pSiS->nocrt2ddcdetection)) {
             if((!(pSiS->VBFlags & CRT2_LCD)) && (!(CR32 & 0x10))) {
	        if(!quiet) {
	           xf86DrvMsg(pScrn->scrnIndex, X_INFO,
	              "%s LCD/plasma panel, sensing via DDC\n",
		      pSiS->forcecrt2redetection ?
		         "Forced re-detection of" : "BIOS detected no");
		}
                if(SiS_SenseLCDDDC(pSiS->SiS_Pr, pSiS)) {
    	           xf86DrvMsg(pScrn->scrnIndex, X_INFO,
	              "DDC error during LCD panel detection\n");
	        } else {
	           inSISIDXREG(SISCR, 0x32, CR32);
	           if(CR32 & 0x08) {
	              pSiS->VBFlags |= CRT2_LCD;
		      pSiS->postVBCR32 |= 0x08;
	           } else {
	              xf86DrvMsg(pScrn->scrnIndex, X_INFO,
	        	   "No LCD/plasma panel detected\n");
	           }
	        }
             }
          }

       }
#ifdef SISDUALHEAD
    }
#endif

    if(pSiS->VBFlags & CRT2_LCD) {
       inSISIDXREG(SISCR, 0x36, CR36);
       inSISIDXREG(SISCR, 0x37, CR37);
       inSISIDXREG(SISCR, 0x7D, CR7D);
       if(pSiS->SiS_Pr->SiS_CustomT == CUT_BARCO1366) {
          pSiS->VBLCDFlags |= VB_LCD_BARCO1366;
	  pSiS->LCDwidth = 1360;
	  pSiS->LCDheight = 1024;
	  if(CR37 & 0x10) pSiS->VBLCDFlags |= VB_LCD_EXPANDING;
	  xf86DrvMsg(pScrn->scrnIndex, X_PROBED,
		"Detected LCD panel (%dx%d, type %d, %sexpanding, RGB%d)\n",
		pSiS->LCDwidth, pSiS->LCDheight,
		((CR36 & 0xf0) >> 4),
		(CR37 & 0x10) ? "" : "non-",
		(CR37 & 0x01) ? 18 : 24);
       } else if(pSiS->SiS_Pr->SiS_CustomT == CUT_PANEL848) {
          pSiS->VBLCDFlags |= VB_LCD_848x480;
	  pSiS->LCDwidth = pSiS->SiS_Pr->CP_MaxX = 848;
	  pSiS->LCDheight = pSiS->SiS_Pr->CP_MaxY = 480;
	  pSiS->VBLCDFlags |= VB_LCD_EXPANDING;
	  xf86DrvMsg(pScrn->scrnIndex, X_CONFIG,
	  	"Assuming LCD/plasma panel (848x480, expanding, RGB24)\n");
       } else {
	  if(CR36 == 0) {
	     /* Old 650/301LV and ECS A907 BIOS versions "forget" to set CR36, CR37 */
	     if(pSiS->VGAEngine == SIS_315_VGA) {
	        if(pSiS->sishw_ext.jChipType < SIS_661) {
	           xf86DrvMsg(pScrn->scrnIndex, X_WARNING,
	              "BIOS provided invalid panel size, probing...\n");
	           if(pSiS->VBFlags & VB_LVDS) pSiS->SiS_Pr->SiS_IF_DEF_LVDS = 1;
	           else pSiS->SiS_Pr->SiS_IF_DEF_LVDS = 0;
	           SiS_GetPanelID(pSiS->SiS_Pr, &pSiS->sishw_ext);
	           inSISIDXREG(SISCR, 0x36, CR36);
	           inSISIDXREG(SISCR, 0x37, CR37);
		} else {
		   xf86DrvMsg(pScrn->scrnIndex, X_WARNING,
	              "Broken BIOS, unable to determine panel size, disabling LCD\n");
		   pSiS->VBFlags &= ~CRT2_LCD;
		   return;
		}
	     } else if(pSiS->VGAEngine == SIS_300_VGA) {
	        xf86DrvMsg(pScrn->scrnIndex, X_WARNING,
	           "BIOS provided invalid panel size, assuming 1024x768, RGB18\n");
	        setSISIDXREG(SISCR,0x36,0xf0,0x02);
		setSISIDXREG(SISCR,0x37,0xee,0x01);
		CR36 = 0x02;
		inSISIDXREG(SISCR,0x37,CR37);
	     }
	  }
	  if((CR36 & 0x0f) == 0x0f) {
	     pSiS->VBLCDFlags |= VB_LCD_CUSTOM;
             pSiS->LCDheight = pSiS->SiS_Pr->CP_MaxY;
	     pSiS->LCDwidth = pSiS->SiS_Pr->CP_MaxX;
	     if(CR37 & 0x10) pSiS->VBLCDFlags |= VB_LCD_EXPANDING;
	     xf86DrvMsg(pScrn->scrnIndex, X_PROBED,
		"Detected LCD/Plasma panel (max. X %d Y %d, pref. %dx%d, RGB%d)\n",
 		pSiS->SiS_Pr->CP_MaxX, pSiS->SiS_Pr->CP_MaxY,
		pSiS->SiS_Pr->CP_PreferredX, pSiS->SiS_Pr->CP_PreferredY,
		(CR37 & 0x01) ? 18 : 24);
	  } else {
	     if(pSiS->VGAEngine == SIS_300_VGA) {
	        pSiS->VBLCDFlags |= SiS300_LCD_Type[(CR36 & 0x0f)].VBLCD_lcdflag;
                pSiS->LCDheight = SiS300_LCD_Type[(CR36 & 0x0f)].LCDheight;
	        pSiS->LCDwidth = SiS300_LCD_Type[(CR36 & 0x0f)].LCDwidth;
	        if(CR37 & 0x10) pSiS->VBLCDFlags |= VB_LCD_EXPANDING;
	     } else if((pSiS->sishw_ext.jChipType >= SIS_661) || (pSiS->ROM661New)) {
	        pSiS->VBLCDFlags |= SiS661_LCD_Type[(CR36 & 0x0f)].VBLCD_lcdflag;
                pSiS->LCDheight = SiS661_LCD_Type[(CR36 & 0x0f)].LCDheight;
	        pSiS->LCDwidth = SiS661_LCD_Type[(CR36 & 0x0f)].LCDwidth;
	        if(CR37 & 0x10) pSiS->VBLCDFlags |= VB_LCD_EXPANDING;
		if(pSiS->sishw_ext.jChipType < SIS_661) {
		   if(!(pSiS->SiS_Pr->PanelSelfDetected)) {
		      inSISIDXREG(SISCR,0x35,tmp);
		      CR37 &= 0xfc;
		      CR37 |= (tmp & 0x01);
		   }
		}
 	     } else {
	        pSiS->VBLCDFlags |= SiS315_LCD_Type[(CR36 & 0x0f)].VBLCD_lcdflag;
                pSiS->LCDheight = SiS315_LCD_Type[(CR36 & 0x0f)].LCDheight;
	        pSiS->LCDwidth = SiS315_LCD_Type[(CR36 & 0x0f)].LCDwidth;
	        if(CR37 & 0x10) pSiS->VBLCDFlags |= VB_LCD_EXPANDING;
	     }
	     xf86DrvMsg(pScrn->scrnIndex, X_PROBED,
			"Detected LCD/plasma panel (%dx%d, %d, %sexp., RGB%d [%02x%02x%02x])\n",
			pSiS->LCDwidth, pSiS->LCDheight,
			((pSiS->VGAEngine == SIS_315_VGA) &&
			 (!pSiS->NewCRLayout)) ?
			 	((CR36 & 0x0f) - 1) : ((CR36 & 0xf0) >> 4),
			(CR37 & 0x10) ? "" : "non-",
			(CR37 & 0x01) ? 18 : 24,
			CR36, CR37, CR7D);
	  }
       }
    }

}

/* Detect CRT2-TV connector type and PAL/NTSC flag */
void SISTVPreInit(ScrnInfoPtr pScrn, Bool quiet)
{
    SISPtr  pSiS = SISPTR(pScrn);
    unsigned char SR16, SR38, CR32, CR35=0, CR38=0, CR79, CR39;
    int temp = 0;
    
    if(!(pSiS->VBFlags & VB_VIDEOBRIDGE)) return;

    inSISIDXREG(SISCR, 0x32, CR32);
    inSISIDXREG(SISCR, 0x35, CR35);
    inSISIDXREG(SISSR, 0x16, SR16);
    inSISIDXREG(SISSR, 0x38, SR38);
    switch(pSiS->VGAEngine) {
    case SIS_300_VGA: 
       if(pSiS->Chipset == PCI_CHIP_SIS630) temp = 0x35;
       break;
    case SIS_315_VGA:
       temp = 0x38;
       break;
    }
    if(temp) {
       inSISIDXREG(SISCR, temp, CR38);
    }

#ifdef TWDEBUG
    xf86DrvMsg(pScrn->scrnIndex, X_PROBED, 
    	"(vb.c: CR32=%02x SR16=%02x SR38=%02x)\n", 
	CR32, SR16, SR38);
#endif

    if(CR32 & 0x47) pSiS->VBFlags |= CRT2_TV;

    if(pSiS->SiS_SD_Flags & SiS_SD_SUPPORTYPBPR) {
       if(CR32 & 0x80) pSiS->VBFlags |= CRT2_TV;
    } else {
       CR32 &= 0x7f;
    }

    if(CR32 & 0x01)
       pSiS->VBFlags |= TV_AVIDEO;
    else if(CR32 & 0x02)
       pSiS->VBFlags |= TV_SVIDEO;
    else if(CR32 & 0x04)
       pSiS->VBFlags |= TV_SCART;
    else if((CR32 & 0x40) && (pSiS->SiS_SD_Flags & SiS_SD_SUPPORTHIVISION))
       pSiS->VBFlags |= (TV_HIVISION | TV_PAL);
    else if((CR32 & 0x80) && (pSiS->SiS_SD_Flags & SiS_SD_SUPPORTYPBPR)) {
       pSiS->VBFlags |= TV_YPBPR;
       if(pSiS->NewCRLayout) {
          if(CR38 & 0x04) {
             switch(CR35 & 0xE0) {
             case 0x20: pSiS->VBFlags |= TV_YPBPR525P; break;
	     case 0x40: pSiS->VBFlags |= TV_YPBPR750P; break;
	     case 0x60: pSiS->VBFlags |= TV_YPBPR1080I; break;
	     default:   pSiS->VBFlags |= TV_YPBPR525I;
	     }
          } else        pSiS->VBFlags |= TV_YPBPR525I;
          inSISIDXREG(SISCR,0x39,CR39);
	  CR39 &= 0x03;
	  if(CR39 == 0x00)      pSiS->VBFlags |= TV_YPBPR43LB;
	  else if(CR39 == 0x01) pSiS->VBFlags |= TV_YPBPR43;
	  else if(CR39 == 0x02) pSiS->VBFlags |= TV_YPBPR169;
	  else			pSiS->VBFlags |= TV_YPBPR43;
       } else if(pSiS->SiS_SD_Flags & SiS_SD_SUPPORTYPBPR) {
          if(CR38 & 0x08) {
	     switch(CR38 & 0x30) {
	     case 0x10: pSiS->VBFlags |= TV_YPBPR525P; break;
	     case 0x20: pSiS->VBFlags |= TV_YPBPR750P; break;
	     case 0x30: pSiS->VBFlags |= TV_YPBPR1080I; break;
	     default:   pSiS->VBFlags |= TV_YPBPR525I;
	     }
	  } else        pSiS->VBFlags |= TV_YPBPR525I;
	  if(pSiS->SiS_SD_Flags & SiS_SD_SUPPORTYPBPRAR) {
             inSISIDXREG(SISCR,0x3B,CR39);
	     CR39 &= 0x03;
	     if(CR39 == 0x00)      pSiS->VBFlags |= TV_YPBPR43LB;
	     else if(CR39 == 0x01) pSiS->VBFlags |= TV_YPBPR169;
	     else if(CR39 == 0x03) pSiS->VBFlags |= TV_YPBPR43;
	  }
       }
    } else if((CR38 & 0x04) && (pSiS->VBFlags & VB_CHRONTEL))
       pSiS->VBFlags |= (TV_CHSCART | TV_PAL);
    else if((CR38 & 0x08) && (pSiS->VBFlags & VB_CHRONTEL))
       pSiS->VBFlags |= (TV_CHYPBPR525I | TV_NTSC);

    if(pSiS->VBFlags & (TV_SCART | TV_SVIDEO | TV_AVIDEO)) {
       if(pSiS->VGAEngine == SIS_300_VGA) {
	  /* Should be SR38, but this does not work. */
	  if(SR16 & 0x20)
	     pSiS->VBFlags |= TV_PAL;
          else
	     pSiS->VBFlags |= TV_NTSC;
       } else if(pSiS->Chipset == PCI_CHIP_SIS550) {
          inSISIDXREG(SISCR, 0x7a, CR79);
	  if(CR79 & 0x08) {
             inSISIDXREG(SISCR, 0x79, CR79);
	     CR79 >>= 5;
	  }
	  if(CR79 & 0x01) {
             pSiS->VBFlags |= TV_PAL;
	     if(CR38 & 0x40)      pSiS->VBFlags |= TV_PALM;
	     else if(CR38 & 0x80) pSiS->VBFlags |= TV_PALN;
 	  } else
	     pSiS->VBFlags |= TV_NTSC;
       } else if(pSiS->Chipset == PCI_CHIP_SIS650) {
	  inSISIDXREG(SISCR, 0x79, CR79);
	  if(CR79 & 0x20) {
             pSiS->VBFlags |= TV_PAL;
	     if(CR38 & 0x40)      pSiS->VBFlags |= TV_PALM;
	     else if(CR38 & 0x80) pSiS->VBFlags |= TV_PALN;
 	  } else
	     pSiS->VBFlags |= TV_NTSC;
       } else if(pSiS->NewCRLayout) {
          if(SR38 & 0x01) {
	     pSiS->VBFlags |= TV_PAL;
	     if(CR35 & 0x04)      pSiS->VBFlags |= TV_PALM;
	     else if(CR35 & 0x08) pSiS->VBFlags |= TV_PALN;
	  } else {
	     pSiS->VBFlags |= TV_NTSC;
	     if(CR35 & 0x02)      pSiS->VBFlags |= TV_NTSCJ;
	  }
       } else {	/* 315, 330 */
	  if(SR38 & 0x01) {
             pSiS->VBFlags |= TV_PAL;
	     if(CR38 & 0x40)      pSiS->VBFlags |= TV_PALM;
	     else if(CR38 & 0x80) pSiS->VBFlags |= TV_PALN;
 	  } else
	     pSiS->VBFlags |= TV_NTSC;
       }
    }

    if((pSiS->VBFlags & (TV_SCART|TV_SVIDEO|TV_AVIDEO)) && !quiet) {
       xf86DrvMsg(pScrn->scrnIndex, X_PROBED, "Detected default TV standard %s\n",
          (pSiS->VBFlags & TV_NTSC) ?
	     ((pSiS->VBFlags & TV_NTSCJ) ? "NTSCJ" : "NTSC") :
	         ((pSiS->VBFlags & TV_PALM) ? "PALM" :
		     ((pSiS->VBFlags & TV_PALN) ? "PALN" : "PAL")));
    }

    if((pSiS->VBFlags & TV_HIVISION) && !quiet) {
       xf86DrvMsg(pScrn->scrnIndex, X_PROBED, "BIOS reports HiVision TV\n");
    }

    if((pSiS->VBFlags & VB_CHRONTEL) && (pSiS->VBFlags & (TV_CHSCART|TV_CHYPBPR525I)) && !quiet) {
       xf86DrvMsg(pScrn->scrnIndex, X_PROBED, "Chrontel: %s forced\n",
       	(pSiS->VBFlags & TV_CHSCART) ? "SCART (PAL)" : "YPbPr (480i)");
    }

    if((pSiS->VBFlags & TV_YPBPR) && !quiet) {
       xf86DrvMsg(pScrn->scrnIndex, X_PROBED, "Detected YPbPr TV (by default %s)\n",
         (pSiS->VBFlags & TV_YPBPR525I) ? "480i" :
	     ((pSiS->VBFlags & TV_YPBPR525P) ? "480p" :
	        ((pSiS->VBFlags & TV_YPBPR750P) ? "720p" : "1080i")));
    }
}

/* Detect CRT2-VGA */
void SISCRT2PreInit(ScrnInfoPtr pScrn, Bool quiet)
{
    SISPtr  pSiS = SISPTR(pScrn);
    unsigned char CR32; 

    /* CRT2-VGA only supported on these bridges */
    if(!(pSiS->VBFlags & VB_SISVGA2BRIDGE))
       return;

    inSISIDXREG(SISCR, 0x32, CR32);
    
    if(CR32 & 0x10)  pSiS->VBFlags |= CRT2_VGA;

#ifdef SISDUALHEAD
    if((!pSiS->DualHeadMode) || (!pSiS->SecondHead)) {
#endif

       if(pSiS->forcecrt2redetection) {
          pSiS->VBFlags &= ~CRT2_VGA;
       }

       /* We don't trust the normal sensing method for VGA2 since
        * it is performed by the BIOS during POST, and it is
        * impossible to sense VGA2 if the bridge is disabled.
        * Therefore, we try sensing VGA2 by DDC as well (if not
        * detected otherwise and only if there is no LCD panel
        * which is prone to be misdetected as a secondary VGA)
        */
       if(!(pSiS->nocrt2ddcdetection)) {
          if(!(pSiS->VBFlags & (CRT2_VGA | CRT2_LCD))) {
	     xf86DrvMsg(pScrn->scrnIndex, X_INFO,
	         "%s secondary VGA, sensing via DDC\n",
	         pSiS->forcecrt2redetection ?
		      "Forced re-detection of" : "BIOS detected no");
             if(SiS_SenseVGA2DDC(pSiS->SiS_Pr, pSiS)) {
    	        xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
	              "DDC error during secondary VGA detection\n");
	     } else {
	        inSISIDXREG(SISCR, 0x32, CR32);
	        if(CR32 & 0x10) {
	           pSiS->VBFlags |= CRT2_VGA;
	           pSiS->postVBCR32 |= 0x10;
		   xf86DrvMsg(pScrn->scrnIndex, X_PROBED,
		         "Detected secondary VGA connection\n");
	        } else {
	           xf86DrvMsg(pScrn->scrnIndex, X_PROBED,
		         "No secondary VGA connection detected\n");
	        }
	     }
          }
       }
#ifdef SISDUALHEAD
    }
#endif    
}

static int
SISDoSense(ScrnInfoPtr pScrn, unsigned short type, unsigned short test)
{
    SISPtr pSiS = SISPTR(pScrn);
    int    temp, mytest, result, i, j;

#ifdef TWDEBUG
    xf86DrvMsg(0, X_INFO, "Sense: %x %x\n", type, test);
#endif

    for(j = 0; j < 10; j++) {
       result = 0;
       for(i = 0; i < 3; i++) {
          mytest = test;
          outSISIDXREG(SISPART4,0x11,(type & 0x00ff));
          temp = (type >> 8) | (mytest & 0x00ff);
          setSISIDXREG(SISPART4,0x10,0xe0,temp);
          SiS_DDC2Delay(pSiS->SiS_Pr, 0x1500);
          mytest >>= 8;
          mytest &= 0x7f;
          inSISIDXREG(SISPART4,0x03,temp);
          temp ^= 0x0e;
          temp &= mytest;
          if(temp == mytest) result++;
#if 1
	  outSISIDXREG(SISPART4,0x11,0x00);
	  andSISIDXREG(SISPART4,0x10,0xe0);
	  SiS_DDC2Delay(pSiS->SiS_Pr, 0x1000);
#endif
       }
       if((result == 0) || (result >= 2)) break;
    }
    return(result);
}

#define GETROMWORD(w) (pSiS->BIOS[w] | (pSiS->BIOS[w+1] << 8))

/* Sense connected devices on 30x */
void
SISSense30x(ScrnInfoPtr pScrn, Bool quiet)
{
    SISPtr  pSiS = SISPTR(pScrn);
    unsigned char backupP4_0d,backupP2_00,backupP2_4d,backupSR_1e,biosflag=0;
    unsigned short svhs=0, svhs_c=0;
    unsigned short cvbs=0, cvbs_c=0;
    unsigned short vga2=0, vga2_c=0;
    int myflag, result; /* , i; */
    
    if(!(pSiS->VBFlags & VB_SISBRIDGE)) return;
    
    if(pSiS->VBFlags & VB_301) {
       svhs = 0x00b9; cvbs = 0x00b3; vga2 = 0x00d1;
       inSISIDXREG(SISPART4,0x01,myflag);
       if(myflag & 0x04) {
	  svhs = 0x00dd; cvbs = 0x00ee; vga2 = 0x00fd;
       }
    } else if(pSiS->VBFlags & (VB_301B | VB_302B)) {
       svhs = 0x016b; cvbs = 0x0174; vga2 = 0x0190;
    } else if(pSiS->VBFlags & (VB_301LV | VB_302LV)) {
       svhs = 0x0200; cvbs = 0x0100;
    } else if(pSiS->VBFlags & (VB_301C | VB_302ELV)) {
       svhs = 0x016b; cvbs = 0x0110; vga2 = 0x0190;
    } else return;

    vga2_c = 0x0e08; svhs_c = 0x0404; cvbs_c = 0x0804;
    if(pSiS->VBFlags & (VB_301LV|VB_302LV|VB_302ELV)) {
       svhs_c = 0x0408; cvbs_c = 0x0808;
    }
    biosflag = 2;

    if(pSiS->Chipset == PCI_CHIP_SIS300) {
       inSISIDXREG(SISSR,0x3b,myflag);
       if(!(myflag & 0x01)) vga2 = vga2_c = 0;
    }

    if(pSiS->sishw_ext.UseROM) {
       if(pSiS->VGAEngine == SIS_300_VGA) {
          if(pSiS->VBFlags & VB_301) {
	     inSISIDXREG(SISPART4,0x01,myflag);
             if(!(myflag & 0x04)) {
                vga2 = GETROMWORD(0xf8); svhs = GETROMWORD(0xfa); cvbs = GETROMWORD(0xfc);
	     }
	  }
	  biosflag = pSiS->BIOS[0xfe];
       } else if((pSiS->Chipset == PCI_CHIP_SIS660) ||
                 (pSiS->Chipset == PCI_CHIP_SIS340)) {
          if(pSiS->ROM661New) {
	     biosflag = 2;
	     vga2 = GETROMWORD(0x63); svhs = cvbs = GETROMWORD(0x65);
	     if(pSiS->BIOS[0x5d] & 0x04) biosflag |= 0x01;
	  }
       } else if(!pSiS->ROM661New) {
#if 0	  /* eg. 1.15.23 has wrong values here */
          myflag = 0;
          if(pSiS->VBFlags & VB_301) {
	     if(pSiS->Chipset == PCI_CHIP_SIS330) {
	        myflag = 0xe5; i = 0x11b;
	     } else {
	        myflag = 0xbd; i = 0xf3
	     }
	  } else if(pSiS->VBFlags & (VB_301B|VB_302B|VB_301LV|VB_302LV)) {
	     if(pSiS->Chipset == PCI_CHIP_SIS330) {
	        myflag = 0xeb; i = 0x11b;
	     } else {
	        myflag = 0xc3; i = 0xf3
	     }
	  }
	  if(myflag) {
	     biosflag = pSiS->BIOS[i];    vga2 = GETROMWORD(myflag);
	     svhs = GETROMWORD(myflag+2); cvbs = GETROMWORD(myflag+4);
	  }
#endif
       }
    }

    if(!(pSiS->VBFlags & VB_SISVGA2BRIDGE)) {
       vga2 = vga2_c = 0;
    }
    
    inSISIDXREG(SISSR,0x1e,backupSR_1e);
    orSISIDXREG(SISSR,0x1e,0x20);
    
    inSISIDXREG(SISPART4,0x0d,backupP4_0d);
    if(pSiS->VBFlags & VB_301C) {
       setSISIDXREG(SISPART4,0x0d,~0x07,0x01);
    } else {
       orSISIDXREG(SISPART4,0x0d,0x04);
    }
    SiS_DDC2Delay(pSiS->SiS_Pr, 0x2000);

    inSISIDXREG(SISPART2,0x00,backupP2_00);
    outSISIDXREG(SISPART2,0x00,((backupP2_00 | 0x1c) & 0xfc));

    inSISIDXREG(SISPART2,0x4d,backupP2_4d);
    if(pSiS->VBFlags & VB_SISYPBPRBRIDGE) {
       outSISIDXREG(SISPART2,0x4d,(backupP2_4d & ~0x10));
    }
    
    if(!(pSiS->VBFlags & VB_301C)) {
       SISDoSense(pScrn, 0, 0);
    }

    andSISIDXREG(SISCR, 0x32, ~0x14);
    pSiS->postVBCR32 &= ~0x14;
    
    if(vga2_c || vga2) {
       if(SISDoSense(pScrn, vga2, vga2_c)) {
          if(biosflag & 0x01) {
	     if(!quiet) {
	        xf86DrvMsg(pScrn->scrnIndex, X_PROBED,
	            "SiS30x: Detected TV connected to SCART output\n");
	     }
	     pSiS->VBFlags |= TV_SCART;
	     orSISIDXREG(SISCR, 0x32, 0x04);
	     pSiS->postVBCR32 |= 0x04;
	  } else {
	     if(!quiet) {
	        xf86DrvMsg(pScrn->scrnIndex, X_PROBED,
	            "SiS30x: Detected secondary VGA connection\n");
	     }
	     pSiS->VBFlags |= VGA2_CONNECTED;
	     orSISIDXREG(SISCR, 0x32, 0x10);
	     pSiS->postVBCR32 |= 0x10;
	  }
       }
       if(biosflag & 0x01) pSiS->SiS_SD_Flags |= SiS_SD_VBHASSCART;
    }

    andSISIDXREG(SISCR, 0x32, 0x3f);
    pSiS->postVBCR32 &= 0x3f;
    
    if(pSiS->VBFlags & VB_301C) {
       orSISIDXREG(SISPART4,0x0d,0x04);
    }

    if((pSiS->VGAEngine == SIS_315_VGA) && (pSiS->VBFlags & VB_SISYPBPRBRIDGE)) {
       if(pSiS->SenseYPbPr) {
          outSISIDXREG(SISPART2,0x4d,(backupP2_4d | 0x10));
          SiS_DDC2Delay(pSiS->SiS_Pr, 0x2000);
          if((result = SISDoSense(pScrn, svhs, 0x0604))) {
             if((result = SISDoSense(pScrn, cvbs, 0x0804))) {
	        if(!quiet) {
	           xf86DrvMsg(pScrn->scrnIndex, X_PROBED,
     			"SiS30x: Detected TV connected to YPbPr component output\n");
		}
	        orSISIDXREG(SISCR,0x32,0x80);
	        pSiS->VBFlags |= TV_YPBPR;
	        pSiS->postVBCR32 |= 0x80;
	     }
          }
          outSISIDXREG(SISPART2,0x4d,backupP2_4d);
       }
    }

    andSISIDXREG(SISCR, 0x32, ~0x03);
    pSiS->postVBCR32 &= ~0x03;

    if(!(pSiS->VBFlags & TV_YPBPR)) {

       if((result = SISDoSense(pScrn, svhs, svhs_c))) {
          if(!quiet) {
             xf86DrvMsg(pScrn->scrnIndex, X_PROBED,
     		  "SiS30x: Detected TV connected to SVIDEO output\n");
  	  }
          pSiS->VBFlags |= TV_SVIDEO;
          orSISIDXREG(SISCR, 0x32, 0x02);
          pSiS->postVBCR32 |= 0x02;
       }

       if((biosflag & 0x02) || (!result)) {
          if(SISDoSense(pScrn, cvbs, cvbs_c)) {
	     if(!quiet) {
	        xf86DrvMsg(pScrn->scrnIndex, X_PROBED,
	             "SiS30x: Detected TV connected to COMPOSITE output\n");
	     }
	     pSiS->VBFlags |= TV_AVIDEO;
	     orSISIDXREG(SISCR, 0x32, 0x01);
	     pSiS->postVBCR32 |= 0x01;
          }
       }

    }

    SISDoSense(pScrn, 0, 0);

    outSISIDXREG(SISPART2,0x00,backupP2_00);
    outSISIDXREG(SISPART4,0x0d,backupP4_0d);
    outSISIDXREG(SISSR,0x1e,backupSR_1e);
    
    if(pSiS->VBFlags & VB_301C) {
       inSISIDXREG(SISPART2,0x00,biosflag);
       if(biosflag & 0x20) {
          for(myflag = 2; myflag > 0; myflag--) {
	     biosflag ^= 0x20;
	     outSISIDXREG(SISPART2,0x00,biosflag);
	  }
       }
    }
    
    outSISIDXREG(SISPART2,0x00,backupP2_00);
}

void
SISSenseChrontel(ScrnInfoPtr pScrn, Bool quiet)
{
    SISPtr  pSiS = SISPTR(pScrn);
    int     temp1=0, temp2, i;
    unsigned char test[3];

    if(pSiS->SiS_Pr->SiS_IF_DEF_CH70xx == 1) {

       /* Chrontel 700x */

       /* Read power status */
       temp1 = SiS_GetCH700x(pSiS->SiS_Pr, 0x0e);  /* Power status */
       if((temp1 & 0x03) != 0x03) {
	  /* Power all outputs */
	  SiS_SetCH700x(pSiS->SiS_Pr, 0x0B0E);
	  SiS_DDC2Delay(pSiS->SiS_Pr, 0x96);
       }
       /* Sense connected TV devices */
       for(i = 0; i < 3; i++) {
	  SiS_SetCH700x(pSiS->SiS_Pr, 0x0110);
	  SiS_DDC2Delay(pSiS->SiS_Pr, 0x96);
	  SiS_SetCH700x(pSiS->SiS_Pr, 0x0010);
	  SiS_DDC2Delay(pSiS->SiS_Pr, 0x96);
	  temp1 = SiS_GetCH700x(pSiS->SiS_Pr, 0x10);
	  if(!(temp1 & 0x08))       test[i] = 0x02;
	  else if(!(temp1 & 0x02))  test[i] = 0x01;
	  else                      test[i] = 0;
	  SiS_DDC2Delay(pSiS->SiS_Pr, 0x96);
       }

       if(test[0] == test[1])      temp1 = test[0];
       else if(test[0] == test[2]) temp1 = test[0];
       else if(test[1] == test[2]) temp1 = test[1];
       else {
	  xf86DrvMsg(pScrn->scrnIndex, X_PROBED,
	        "Chrontel: TV detection unreliable - test results varied\n");
	  temp1 = test[2];
       }

    } else if(pSiS->SiS_Pr->SiS_IF_DEF_CH70xx == 2) {

       /* Chrontel 701x */

       /* Backup Power register */
       temp1 = SiS_GetCH701x(pSiS->SiS_Pr, 0x49);

       /* Enable TV path */
       SiS_SetCH701x(pSiS->SiS_Pr, 0x2049);

       SiS_DDC2Delay(pSiS->SiS_Pr, 0x96);

       /* Sense connected TV devices */
       temp2 = SiS_GetCH701x(pSiS->SiS_Pr, 0x20);
       temp2 |= 0x01;
       SiS_SetCH701x(pSiS->SiS_Pr, (temp2 << 8) | 0x20);

       SiS_DDC2Delay(pSiS->SiS_Pr, 0x96);

       temp2 ^= 0x01;
       SiS_SetCH701x(pSiS->SiS_Pr, (temp2 << 8) | 0x20);

       SiS_DDC2Delay(pSiS->SiS_Pr, 0x96);
		   
       temp2 = SiS_GetCH701x(pSiS->SiS_Pr, 0x20); 

       /* Restore Power register */
       SiS_SetCH701x(pSiS->SiS_Pr, (temp1 << 8) | 0x49);

       temp1 = 0;
       if(temp2 & 0x02) temp1 |= 0x01;
       if(temp2 & 0x10) temp1 |= 0x01;
       if(temp2 & 0x04) temp1 |= 0x02;

       if( (temp1 & 0x01) && (temp1 & 0x02) ) temp1 = 0x04;

    }

    switch(temp1) {
       case 0x01:
          xf86DrvMsg(pScrn->scrnIndex, X_PROBED,
	       "Chrontel: Detected TV connected to COMPOSITE output\n");
	  pSiS->VBFlags |= TV_AVIDEO;
	  orSISIDXREG(SISCR, 0x32, 0x01);
   	  andSISIDXREG(SISCR, 0x32, ~0x06);
	  pSiS->postVBCR32 |= 0x01;
	  pSiS->postVBCR32 &= ~0x06;
          break;
       case 0x02:
	  xf86DrvMsg(pScrn->scrnIndex, X_PROBED,
	       "Chrontel: Detected TV connected to SVIDEO output\n");
	  pSiS->VBFlags |= TV_SVIDEO;
	  orSISIDXREG(SISCR, 0x32, 0x02);
	  andSISIDXREG(SISCR, 0x32, ~0x05);
	  pSiS->postVBCR32 |= 0x02;
	  pSiS->postVBCR32 &= ~0x05;
          break;
       case 0x04:
	  xf86DrvMsg(pScrn->scrnIndex, X_PROBED,
	       "Chrontel: Detected TV connected to SCART or YPBPR output\n");
  	  if(pSiS->chtvtype == -1) {
	     if(!quiet) {
	        xf86DrvMsg(pScrn->scrnIndex, X_INFO,
	            "Chrontel: Use CHTVType option to select either SCART or YPBPR525I\n");
	        xf86DrvMsg(pScrn->scrnIndex, X_INFO,
	            "Chrontel: Using SCART by default\n");
	     }
	     pSiS->chtvtype = 1;
	  }
	  if(pSiS->chtvtype)
	     pSiS->VBFlags |= TV_CHSCART;
	  else
	     pSiS->VBFlags |= TV_CHYPBPR525I;
          break;
       default:
	  xf86DrvMsg(pScrn->scrnIndex, X_PROBED,
	       "Chrontel: No TV detected.\n");
	  andSISIDXREG(SISCR, 0x32, ~0x07);
	  pSiS->postVBCR32 &= ~0x07;
       }
}		

/* Redetect CRT2 devices. Calling this requires a reset
 * of the current display mode if TRUE is returned.
 */
Bool SISRedetectCRT2Type(ScrnInfoPtr pScrn)
{
    SISPtr  pSiS = SISPTR(pScrn);
    unsigned long VBFlagsBackup = pSiS->VBFlags;
    Bool backup1 = pSiS->forcecrt2redetection;
    Bool backup2 = pSiS->nocrt2ddcdetection;
    
#ifdef SISDUALHEAD
    if(pSiS->DualHeadMode) return FALSE;
#endif
    
    pSiS->VBFlags &= (VB_VIDEOBRIDGE | DISPLAY_MODE);
    
    /* At first, re-do the sensing for TV and VGA2 */
    if(pSiS->VBFlags & VB_SISBRIDGE) {
       SISSense30x(pScrn, FALSE);
    } else if(pSiS->VBFlags & VB_CHRONTEL) {
       SiS_SetChrontelGPIO(pSiS->SiS_Pr, 0x9c);
       SISSenseChrontel(pScrn, TRUE);
       SiS_SetChrontelGPIO(pSiS->SiS_Pr, 0x00);
    }
    
    SISTVPreInit(pScrn, TRUE);
    
    pSiS->forcecrt2redetection = TRUE;
    pSiS->nocrt2ddcdetection = FALSE;
    
    /* We only re-detect LCD for the TMDS-SiS-bridges. LVDS
     * is practically never being hot-plugged (and even if,
     * there is no way of detecting this). 
     */
    if((pSiS->VGAEngine == SIS_315_VGA) &&
       (pSiS->VBFlags & VB_SISTMDSBRIDGE) &&
       (!(pSiS->VBFlags & VB_30xBDH)) &&
       (pSiS->VESA != 1)) {
       SISLCDPreInit(pScrn, TRUE);
    } else {
       pSiS->VBFlags |= (pSiS->detectedCRT2Devices & CRT2_LCD);
    }
    
    /* Secondary VGA is only supported on these bridges: */
    if(pSiS->VBFlags & VB_SISVGA2BRIDGE) {
       SISCRT2PreInit(pScrn, TRUE);
    }
    
    pSiS->forcecrt2redetection = backup1;
    pSiS->nocrt2ddcdetection = backup2;
    
    SISDetermineLCDACap(pScrn);
    SISSaveDetectedDevices(pScrn);

    pSiS->VBFlags = VBFlagsBackup;
    
    /* If LCD disappeared, don't use it and don't advertise LCDA support. Duh! */
    if(!(pSiS->detectedCRT2Devices & CRT2_LCD)) {
       pSiS->SiS_SD_Flags &= ~(SiS_SD_SUPPORTLCDA);
       if(pSiS->VBFlags & CRT2_LCD) {
          /* If CRT2 was LCD, disable CRT2 and adapt display mode flags */
          pSiS->VBFlags &= ~(CRT2_LCD | DISPLAY_MODE);
	  /* Switch on CRT1 as an emergency measure */
	  pSiS->VBFlags |= (SINGLE_MODE | DISPTYPE_CRT1);
	  pSiS->CRT1off = 0;
       }
       /* If CRT1 was LCD, switch to CRT1-VGA. No need to adapt display mode flags. */
       pSiS->VBFlags &= ~(CRT1_LCDA);       
       pSiS->VBFlags_backup = pSiS->VBFlags;
    }
    
    pSiS->VBFlagsInit = pSiS->VBFlags;
    
    /* Save new detection result registers to write them back in EnterVT() */
    inSISIDXREG(SISCR,0x32,pSiS->myCR32);
    inSISIDXREG(SISCR,0x36,pSiS->myCR36);
    inSISIDXREG(SISCR,0x37,pSiS->myCR37);
    
    return TRUE;
}



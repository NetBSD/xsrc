/* $XFree86: xc/programs/Xserver/hw/xfree86/drivers/sis/sis_vb.c,v 1.6 2002/01/17 09:57:30 eich Exp $ */

#include "xf86.h"
#include "xf86_ansic.h"
#include "compiler.h"
#include "xf86PciInfo.h"

#include "sis.h"
#include "sis_regs.h"
#include "sis_vb.h"

/* TW: Detect CRT2-LCD and LCD size */
void SISLCDPreInit(ScrnInfoPtr pScrn)
{
    SISPtr  pSiS = SISPTR(pScrn);
    int CR32, SR17, CR36;
  
    if (!(pSiS->VBFlags & VB_VIDEOBRIDGE))
	return;
  
    inSISIDXREG(pSiS->RelIO+CROFFSET, 0x32, CR32);
    inSISIDXREG(pSiS->RelIO+SROFFSET, 0x17, SR17);
 
    if ( (SR17 & 0x0F) && (pSiS->Chipset != PCI_CHIP_SIS300) ) {
	if ( (SR17 & 0x01) && (!pSiS->CRT1off) )
	    pSiS->CRT1off = 0;
	else {
	    if (SR17 & 0x0E)
		pSiS->CRT1off = 1;
	    else
		pSiS->CRT1off = 0;
	}
 	if (SR17 & 0x02)
	    pSiS->VBFlags |= CRT2_LCD;
    } else {
	if ( (CR32 & 0x20) && (!pSiS->CRT1off) )
	    pSiS->CRT1off = 0;
 	else {
	    if (CR32 & 0x5F)
		pSiS->CRT1off = 1;
	    else
		pSiS->CRT1off = 0;
 	}
     	if (CR32 & 0x08)
            pSiS->VBFlags |= CRT2_LCD;
    }
 
    if (pSiS->VBFlags & CRT2_LCD) {
	inSISIDXREG(pSiS->RelIO+CROFFSET, 0x36, CR36);
	switch (CR36) {
	case 1:
	    pSiS->VBFlags |= LCD_800x600;
 	    pSiS->LCDheight = 600;
 	    break;
	case 2:
	    pSiS->VBFlags |= LCD_1024x768;
 	    pSiS->LCDheight = 768;
 	    break;
	case 3:
	    pSiS->VBFlags |= LCD_1280x1024;
 	    pSiS->LCDheight = 1024;
 	    break;
	case 4:
	    pSiS->VBFlags |= LCD_1280x960;  /* TW */
 	    pSiS->LCDheight = 960;
 	    break;
	case 5:
	    pSiS->VBFlags |= LCD_640x480;   /* TW */
 	    pSiS->LCDheight = 480;
 	    break;
	default:
 	    pSiS->VBFlags |= LCD_1024x768;  /* TW */
 	    pSiS->LCDheight = 768;
 	    break;
	}
    }
}
 
/* TW: Detect CRT2-TV connector type and PAL/NTSC flag */
void SISTVPreInit(ScrnInfoPtr pScrn)
{
    SISPtr  pSiS = SISPTR(pScrn);
    int CR32, CR38, SR16, SR17;
 
    if (!(pSiS->VBFlags & VB_VIDEOBRIDGE))
	return;
 
    inSISIDXREG(pSiS->RelIO+CROFFSET, 0x32, CR32);
    inSISIDXREG(pSiS->RelIO+SROFFSET, 0x17, SR17);
 
    if ( (SR17 & 0x0F) && (pSiS->Chipset != PCI_CHIP_SIS300) ) {
	if (SR17 & 0x04) /* { */ /* TW: Determine TV type even if not using TV output */
 	    pSiS->VBFlags |= CRT2_TV;
 
	if (SR17 & 0x20)
	    pSiS->VBFlags |= TV_SVIDEO;
	else if (SR17 & 0x10)
	    pSiS->VBFlags |= TV_AVIDEO;
	inSISIDXREG(pSiS->RelIO+SROFFSET, 0x16, SR16);
	if (SR16 & 0x20)
	    pSiS->VBFlags |= TV_PAL;
	else
	    pSiS->VBFlags |= TV_NTSC;
	/* } */
    } else {
	if (CR32 & 0x47) /* { */
 	    pSiS->VBFlags |= CRT2_TV;
	if (CR32 & 0x04)
	    pSiS->VBFlags |= TV_SCART;
	else if (CR32 & 0x02)
	    pSiS->VBFlags |= TV_SVIDEO;
	else if (CR32 & 0x01)
	    pSiS->VBFlags |= TV_AVIDEO;
	else if (CR32 & 0x40)
	    pSiS->VBFlags |= (TV_SVIDEO | TV_HIVISION);
	inSISIDXREG(pSiS->RelIO+SROFFSET, 0x38, CR38);
	if (CR38 & 0x01)
	    pSiS->VBFlags |= TV_PAL;
	else
	    pSiS->VBFlags |= TV_NTSC;
	/* } */
    }
 
    /* TW: This is old code: */
  
    /* TW: Reading PAL/NTSC flag from 0x31 is not a good idea. We'd
     *     better read this from POWER_ON_TRAP (0x38) some day. */
#if 0
    inSISIDXREG(pSiS->RelIO+CROFFSET, 0x31, temp);
    if (temp & 0x01)
	pSiS->VBFlags |= TV_PAL;
    else
	pSiS->VBFlags |= TV_NTSC;
#endif
}
  
 /* TW: Detect CRT2-VGA */
  void SISCRT2PreInit(ScrnInfoPtr pScrn)
{
    SISPtr  pSiS = SISPTR(pScrn);
    int SR17, CR32;
  
    if (!(pSiS->VBFlags & VB_VIDEOBRIDGE))
	return;
  
    inSISIDXREG(pSiS->RelIO+CROFFSET, 0x32, CR32);
    inSISIDXREG(pSiS->RelIO+SROFFSET, 0x17, SR17);
 
    if ( (SR17 & 0x0F) && (pSiS->Chipset != PCI_CHIP_SIS300) ) {
	if (SR17 & 0x08)
	    pSiS->VBFlags |= CRT2_VGA;
    } else {
	if (CR32 & 0x10)
	    pSiS->VBFlags |= CRT2_VGA;
    }
}

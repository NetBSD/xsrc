/* $XFree86: xc/programs/Xserver/hw/xfree86/drivers/sis/sis_vb.c,v 1.2 2000/12/02 01:16:18 dawes Exp $ */

#include "xf86.h"
#include "xf86_ansic.h"
#include "compiler.h"
#include "xf86PciInfo.h"

#include "sis.h"
#include "sis_regs.h"
#include "sis_vb.h"

void SISLCDPreInit(ScrnInfoPtr pScrn)
{
	SISPtr	pSiS = SISPTR(pScrn);
	int	temp;

	
	if (!(pSiS->VBFlags & (VB_301|VB_302|VB_303|VB_LVDS|VB_CHRONTEL))) 
		return;

	inSISIDXREG(pSiS->RelIO+CROFFSET, 0x32, temp);
	if (temp & 0x08)
           pSiS->VBFlags |= CRT2_LCD;

	inSISIDXREG(pSiS->RelIO+CROFFSET, 0x36, temp);
	if (temp == 1)
		pSiS->VBFlags |= LCD_800x600;
	if (temp == 2)
		pSiS->VBFlags |= LCD_1024x768;
	if (temp == 3)
		pSiS->VBFlags |= LCD_1280x1024;
}

void SISTVPreInit(ScrnInfoPtr pScrn)
{
	SISPtr	pSiS = SISPTR(pScrn);
	int	temp;

	if (!(pSiS->VBFlags & (VB_301|VB_302|VB_303|VB_LVDS|VB_CHRONTEL))) 
		return;

	inSISIDXREG(pSiS->RelIO+CROFFSET, 0x32, temp);
	if (temp & 0x04)
		pSiS->VBFlags |= (CRT2_TV | TV_SCART);
	else if (temp & 0x02)
		pSiS->VBFlags |= (CRT2_TV | TV_SVIDEO);
	else if (temp & 0x01)
		pSiS->VBFlags |= (CRT2_TV | TV_AVIDEO);
	else if (temp & 0x40)
		pSiS->VBFlags |= (CRT2_TV | TV_HIVISION);
		return;

	inSISIDXREG(pSiS->RelIO+CROFFSET, 0x31, temp);
	if (temp & 0x01)
		pSiS->VBFlags |= TV_PAL;
	else
		pSiS->VBFlags |= TV_NTSC;
}

void SISCRT2PreInit(ScrnInfoPtr pScrn)
{
	SISPtr	pSiS = SISPTR(pScrn);
	int	temp;

	if (!(pSiS->VBFlags & (VB_301|VB_302|VB_303|VB_LVDS|VB_CHRONTEL))) 
		return;

	inSISIDXREG(pSiS->RelIO+CROFFSET, 0x32, temp);
	if (temp & 0x10)
           pSiS->VBFlags |= CRT2_VGA;
          
        return;

}

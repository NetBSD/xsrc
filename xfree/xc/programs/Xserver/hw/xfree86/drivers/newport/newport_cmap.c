/*
 * Id: newport_cmap.c,v 1.1 2000/11/29 20:58:10 agx Exp $
 */
/* $XFree86: xc/programs/Xserver/hw/xfree86/drivers/newport/newport_cmap.c,v 1.1 2000/12/01 19:47:59 dawes Exp $ */

#include "newport.h"

static void NewportCmapSetRGB( NewportRegsPtr pNewportRegs, unsigned short addr, LOCO color);
static void NewportCmapGetRGB( NewportRegsPtr pNewportRegs, unsigned short addr, LOCO *color);

/* Load a colormap into the hardware */
void NewportLoadPalette(ScrnInfoPtr pScrn, int numColors, int *indices, \
			LOCO* colors, VisualPtr pVisual)
{
	int i,index;
	NewportRegsPtr pNewportRegs = NEWPORTPTR(pScrn)->pNewportRegs;
	
	for(i = 0; i < numColors; i++) {
		index=indices[i];
 		NewportBfwait(pNewportRegs);
		NewportCmapSetRGB(pNewportRegs, index, colors[index]);
        }
}

void NewportBackupPalette(ScrnInfoPtr pScrn)
{
	int i;
	NewportPtr pNewport = NEWPORTPTR(pScrn);

	NewportWait(pNewport->pNewportRegs);
	for(i = 0; i < 256; i++) {
		NewportCmapGetRGB(pNewport->pNewportRegs, i, &(pNewport->txt_colormap[i]));
	}
}

/* restore the default colormap */
void NewportRestorePalette(ScrnInfoPtr pScrn)
{
	int i;
	NewportPtr pNewport = NEWPORTPTR(pScrn);
	
	for(i = 0; i < 256; i++) {
		NewportCmapSetRGB(pNewport->pNewportRegs, i, pNewport->txt_colormap[i]);
	}
}

/* set the colormap entry at addr to color */
static void NewportCmapSetRGB( NewportRegsPtr pNewportRegs, unsigned short addr, LOCO color)
{
	NewportWait(pNewportRegs);	/* this one should not be necessary */
	NewportBfwait(pNewportRegs);
	pNewportRegs->set.dcbmode = (NPORT_DMODE_ACMALL | NCMAP_PROTOCOL |
				NPORT_DMODE_SENDIAN | NPORT_DMODE_ECINC |
				NCMAP_REGADDR_AREG | NPORT_DMODE_W2);
	pNewportRegs->set.dcbdata0.hwords.s1 = addr;
	pNewportRegs->set.dcbmode = (NPORT_DMODE_ACMALL | NCMAP_PROTOCOL |
				 NCMAP_REGADDR_PBUF | NPORT_DMODE_W3);
	pNewportRegs->set.dcbdata0.all = (color.red << 24) |
						(color.green << 16) |
						(color.blue << 8);
}

/* get the colormap entry at addr */
static void NewportCmapGetRGB( NewportRegsPtr pNewportRegs, unsigned short addr, LOCO* color)
{
	npireg_t tmp;

	NewportBfwait(pNewportRegs);
	pNewportRegs->set.dcbmode = (NPORT_DMODE_ACMALL | NCMAP_PROTOCOL |
				NPORT_DMODE_SENDIAN | NPORT_DMODE_ECINC |
				NCMAP_REGADDR_AREG | NPORT_DMODE_W2);
	pNewportRegs->set.dcbdata0.hwords.s1 = addr;
	pNewportRegs->set.dcbmode = (NPORT_DMODE_ACMALL | NCMAP_PROTOCOL |
				 NCMAP_REGADDR_PBUF | NPORT_DMODE_W3);
	tmp = pNewportRegs->set.dcbdata0.all;
	color->red = (tmp >> 24) & 0xff;
	color->green = (tmp >> 16) & 0xff;
	color->blue = (tmp >> 8) & 0xff;
}


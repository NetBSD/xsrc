/* $XFree86: xc/programs/Xserver/hw/xfree86/drivers/cirrus/alp_dga.c,v 1.2 2000/02/08 13:13:14 eich Exp $ */

/* (c) Itai Nahshon */

#include "xf86.h"
#include "xf86_OSproc.h"
#include "xf86_ansic.h"
#include "compiler.h"

#include "xf86Pci.h"
#include "xf86PciInfo.h"

#include "vgaHW.h"

#include "cir.h"

static Bool
CirDGAGetParams(int scrnIndex, unsigned long *offset, int *banksize,
				int *memsize)
{
	ScrnInfoPtr pScrn = xf86Screens[scrnIndex];
	CIRPtr pCir = &CIRPTR(pScrn)->CirRec;

	*offset = pCir->FbAddress;
	*banksize = pScrn->videoRam * 1024;
	*memsize = pScrn->videoRam * 1024;

#ifdef CIR_DEBUG
	ErrorF("CirDGAGetParams %d = 0x%08x, %d, %d\n",
		scrnIndex, *banksize, *memsize);
#endif
	return TRUE;
}

static Bool
CirDGASetDirect(int scrnIndex, Bool enable)
{
	return TRUE;
}

static Bool
CirDGASetBank(int scrnIndex, int bank, int flags)
{
	return TRUE;
}

static Bool
CirDGASetViewport(int scrnIndex, int x, int y, int flags)
{
	ScrnInfoPtr pScrn = xf86Screens[scrnIndex];
	(*pScrn->AdjustFrame)(scrnIndex, x, y, 0);
	return TRUE;
}

static  Bool
CirDGAViewportChanged(int scrnIndex, int n, int flags)
{
	return TRUE;
}

Bool
CirDGAInit(ScreenPtr pScreen)
{
	ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
	CIRPtr pCir = &CIRPTR(pScrn)->CirRec;
	DGAInfoPtr pDGAInfo;

	pDGAInfo = DGACreateInfoRec();
	if(pDGAInfo == NULL)
		return FALSE;

	pCir->DGAInfo = pDGAInfo;

	pDGAInfo->GetParams = CirDGAGetParams;
	pDGAInfo->SetDirectMode = CirDGASetDirect;
	pDGAInfo->SetBank = CirDGASetBank;
	pDGAInfo->SetViewport = CirDGASetViewport;
	pDGAInfo->ViewportChanged = CirDGAViewportChanged;;

	return DGAInit(pScreen, pDGAInfo, 0);
}

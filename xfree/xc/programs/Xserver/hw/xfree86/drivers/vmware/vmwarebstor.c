/* **********************************************************
 * Copyright (C) 1998-2001 VMware, Inc.
 * All Rights Reserved
 * **********************************************************/
#ifdef VMX86_DEVEL
char rcsId_vmwarebstor[] =

    "Id: vmwarebstor.c,v 1.2 2001/01/26 23:32:16 yoel Exp $";
#endif
/* $XFree86: xc/programs/Xserver/hw/xfree86/drivers/vmware/vmwarebstor.c,v 1.1 2001/04/05 19:29:44 dawes Exp $ */

#include "vmware.h"

void
vmwareSaveDoomedAreas(WindowPtr pWin,
    RegionPtr prgnSave, int xorg, int yorg)
{
    VMWAREPtr pVMWARE = VMWAREPTR(infoFromScreen(pWin->drawable.pScreen));

    TRACEPOINT

    if (pVMWARE->vmwareBBLevel == 0) {
	BoxRec BB;
	
	BB = *REGION_EXTENTS(pWin->drawable.pScreen, prgnSave);
	BB.x1 += xorg;
	BB.x2 += xorg;
	BB.y1 += yorg;
	BB.y2 += yorg;
	HIDE_CURSOR(pVMWARE, BB);
	vmwareWaitForFB(pVMWARE);
	pVMWARE->vmwareBBLevel++;
	pVMWARE->ScrnFuncs.SaveDoomedAreas(pWin, prgnSave, xorg, yorg);
	pVMWARE->vmwareBBLevel--;
	SHOW_CURSOR(pVMWARE, BB);
    } else {
	pVMWARE->ScrnFuncs.SaveDoomedAreas(pWin, prgnSave, xorg, yorg);
    }
}

RegionPtr
vmwareRestoreAreas(WindowPtr pWin,
    RegionPtr prgnRestore)
{
    RegionPtr res;
    VMWAREPtr pVMWARE = VMWAREPTR(infoFromScreen(pWin->drawable.pScreen));

    TRACEPOINT

    if (pVMWARE->vmwareBBLevel == 0) {
	BoxPtr pBB;

	pBB = REGION_EXTENTS(pWin->drawable.pScreen, prgnRestore);
	HIDE_CURSOR(pVMWARE, *pBB);
	vmwareWaitForFB(pVMWARE);
	pVMWARE->vmwareBBLevel++;
	res = pVMWARE->ScrnFuncs.RestoreAreas(pWin, prgnRestore);
	pVMWARE->vmwareBBLevel--;
	vmwareSendSVGACmdUpdate(pVMWARE, pBB);
	SHOW_CURSOR(pVMWARE, *pBB);
    } else {
	res = pVMWARE->ScrnFuncs.RestoreAreas(pWin, prgnRestore);
    }
    return res;
}

/* $XFree86: xc/programs/Xserver/hw/xfree86/drivers/ati/radeon_probe.c,v 1.5 2000/12/13 02:45:00 tsi Exp $ */
/*
 * Copyright 2000 ATI Technologies Inc., Markham, Ontario, and
 *                VA Linux Systems Inc., Fremont, California.
 *
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation on the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial
 * portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NON-INFRINGEMENT.  IN NO EVENT SHALL ATI, VA LINUX SYSTEMS AND/OR
 * THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

/*
 * Authors:
 *   Kevin E. Martin <martin@valinux.com>
 *   Rickard E. Faith <faith@valinux.com>
 *
 * Modified by Marc Aurele La France <tsi@xfree86.org> for ATI driver merge.
 */

#include "atimodule.h"
#include "ativersion.h"

#include "radeon_probe.h"
#include "radeon_version.h"

#include "xf86PciInfo.h"

#include "xf86.h"
#include "xf86_ansic.h"
#include "xf86Resources.h"

SymTabRec RADEONChipsets[] = {
    { PCI_CHIP_RADEON_QD, "ATI Radeon QD (AGP)" },
    { PCI_CHIP_RADEON_QE, "ATI Radeon QE (AGP)" },
    { PCI_CHIP_RADEON_QF, "ATI Radeon QF (AGP)" },
    { PCI_CHIP_RADEON_QG, "ATI Radeon QG (AGP)" },
    { -1,                 NULL }
};

PciChipsets RADEONPciChipsets[] = {
    { PCI_CHIP_RADEON_QD, PCI_CHIP_RADEON_QD, RES_SHARED_VGA },
    { PCI_CHIP_RADEON_QE, PCI_CHIP_RADEON_QE, RES_SHARED_VGA },
    { PCI_CHIP_RADEON_QF, PCI_CHIP_RADEON_QF, RES_SHARED_VGA },
    { PCI_CHIP_RADEON_QG, PCI_CHIP_RADEON_QG, RES_SHARED_VGA },
    { -1,                 -1,                 RES_UNDEFINED }
};

/* Return the options for supported chipset 'n'; NULL otherwise */
OptionInfoPtr
RADEONAvailableOptions(int chipid, int busid)
{
    int i;

    /*
     * Return options defined in the radeon submodule which will have been
     * loaded by this point.
     */
    for (i = 0; RADEONPciChipsets[i].PCIid > 0; i++) {
	if (chipid == RADEONPciChipsets[i].PCIid)
	    return RADEONOptions;
    }
    return NULL;
}

/* Return the string name for supported chipset 'n'; NULL otherwise. */
void
RADEONIdentify(int flags)
{
    xf86PrintChipsets(RADEON_NAME,
		      "Driver for ATI Radeon chipsets",
		      RADEONChipsets);
}

/* Return TRUE if chipset is present; FALSE otherwise. */
Bool
RADEONProbe(DriverPtr drv, int flags)
{
    int           numUsed;
    int           numDevSections, nATIGDev, nRadeonGDev;
    int           *usedChips;
    GDevPtr       *devSections, *ATIGDevs, *RadeonGDevs;
    EntityInfoPtr pEnt;
    Bool          foundScreen = FALSE;
    int           i;

    if (!xf86GetPciVideoInfo()) return FALSE;

    /* Collect unclaimed device sections for both driver names */
    nATIGDev = xf86MatchDevice(ATI_NAME, &ATIGDevs);
    nRadeonGDev = xf86MatchDevice(RADEON_NAME, &RadeonGDevs);

    if (!(numDevSections = nATIGDev + nRadeonGDev)) return FALSE;

    if (!ATIGDevs) {
	if (!(devSections = RadeonGDevs))
	    numDevSections = 1;
	else
	    numDevSections = nRadeonGDev;
    } if (!RadeonGDevs) {
	devSections = ATIGDevs;
	numDevSections = nATIGDev;
    } else {
	/* Combine into one list */
	devSections = xnfalloc((numDevSections + 1) * sizeof(GDevPtr));
	(void)memcpy(devSections,
		     ATIGDevs, nATIGDev * sizeof(GDevPtr));
	(void)memcpy(devSections + nATIGDev,
		     RadeonGDevs, nRadeonGDev * sizeof(GDevPtr));
	devSections[numDevSections] = NULL;
	xfree(ATIGDevs);
	xfree(RadeonGDevs);
    }

    numUsed = xf86MatchPciInstances(RADEON_NAME,
				    PCI_VENDOR_ATI,
				    RADEONChipsets,
				    RADEONPciChipsets,
				    devSections,
				    numDevSections,
				    drv,
				    &usedChips);

    if (numUsed<=0) return FALSE;

    if (flags & PROBE_DETECT)
	foundScreen = TRUE;
    else for (i = 0; i < numUsed; i++) {
	pEnt = xf86GetEntityInfo(usedChips[i]);

	if (pEnt->active) {
	    ScrnInfoPtr pScrn    = xf86AllocateScreen(drv, 0);

#ifdef XFree86LOADER
	    if (!xf86LoadSubModule(pScrn, "radeon")) {
		xf86Msg(X_ERROR,
		    RADEON_NAME ":  Failed to load \"radeon\" module.\n");
		xf86DeleteScreen(pScrn->scrnIndex, 0);
		continue;
	    }

	    xf86LoaderReqSymLists(RADEONSymbols, NULL);

	    /* Workaround for possible loader bug */
#	    define RADEONPreInit     \
		(xf86PreInitProc*)    LoaderSymbol("RADEONPreInit")
#	    define RADEONScreenInit  \
		(xf86ScreenInitProc*) LoaderSymbol("RADEONScreenInit")
#	    define RADEONSwitchMode  \
		(xf86SwitchModeProc*) LoaderSymbol("RADEONSwitchMode")
#	    define RADEONAdjustFrame \
		(xf86AdjustFrameProc*)LoaderSymbol("RADEONAdjustFrame")
#	    define RADEONEnterVT     \
		(xf86EnterVTProc*)    LoaderSymbol("RADEONEnterVT")
#	    define RADEONLeaveVT     \
		(xf86LeaveVTProc*)    LoaderSymbol("RADEONLeaveVT")
#	    define RADEONFreeScreen  \
		(xf86FreeScreenProc*) LoaderSymbol("RADEONFreeScreen")
#	    define RADEONValidMode   \
		(xf86ValidModeProc*)  LoaderSymbol("RADEONValidMode")

#endif

	    pScrn->driverVersion = RADEON_VERSION_CURRENT;
	    pScrn->driverName    = RADEON_DRIVER_NAME;
	    pScrn->name          = RADEON_NAME;
	    pScrn->Probe         = RADEONProbe;
	    pScrn->PreInit       = RADEONPreInit;
	    pScrn->ScreenInit    = RADEONScreenInit;
	    pScrn->SwitchMode    = RADEONSwitchMode;
	    pScrn->AdjustFrame   = RADEONAdjustFrame;
	    pScrn->EnterVT       = RADEONEnterVT;
	    pScrn->LeaveVT       = RADEONLeaveVT;
	    pScrn->FreeScreen    = RADEONFreeScreen;
	    pScrn->ValidMode     = RADEONValidMode;

	    foundScreen          = TRUE;

	    xf86ConfigActivePciEntity(pScrn, usedChips[i], RADEONPciChipsets,
				      0, 0, 0, 0, 0);
	}
	xfree(pEnt);
    }

    xfree(usedChips);
    xfree(devSections);

    return foundScreen;
}

/* $XFree86: xc/programs/Xserver/hw/xfree86/scanpci/xf86PciData.c,v 1.5 2000/04/05 18:13:58 dawes Exp $ */
/*
 * the PCI data structures
 *
 * this module only includes the data that is relevant for video boards
 * the non-video data is included in the scanpci module
 *
 * Copyright 1995-2000 by The XFree86 Project, Inc.
 *
 */

/* XXX This is including a lot of stuff that modules should not include! */

#include "X.h"
#include "os.h"
#include "xf86.h"
#include "xf86Priv.h"
#include "xf86Pci.h"

#ifndef IN_MODULE
#include <ctype.h>
#include <stdlib.h>
#else
#include <xf86_ansic.h>
#endif

#define INIT_PCI_CARD_INFO		TRUE
#define DECLARE_CARD_DATASTRUCTURES	TRUE
#define INIT_PCI_VENDOR_INFO		TRUE
#define INIT_PCI_VENDOR_NAME_INFO	TRUE
#include "xf86PciInfo.h"
#include "xf86PciData.h"

#ifdef XFree86LOADER

#include "xf86Module.h"

static XF86ModuleVersionInfo pciDataVersRec = {
	"pcidata",
	MODULEVENDORSTRING,
	MODINFOSTRING1,
	MODINFOSTRING2,
	XF86_VERSION_CURRENT,
	0, 1, 0,
	ABI_CLASS_VIDEODRV,
	ABI_VIDEODRV_VERSION,
	NULL,
	{0, 0, 0, 0}
};

XF86ModuleData pcidataModuleData = { &pciDataVersRec, NULL, NULL };

#endif /* XFree86LOADER */

void
xf86SetupPciData(SymTabPtr *NameInfo,
		 pciVendorDeviceInfo **DeviceInfo,
		 pciVendorCardInfo **CardInfo)
{
    *CardInfo = xf86PCICardInfoData;
    *DeviceInfo = xf86PCIVendorInfoData;
    *NameInfo = xf86PCIVendorNameInfoData;
}


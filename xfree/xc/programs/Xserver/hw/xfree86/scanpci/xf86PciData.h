/* $XFree86: xc/programs/Xserver/hw/xfree86/scanpci/xf86PciData.h,v 1.1 2000/02/08 13:13:33 eich Exp $ */

#ifndef PCI_DATA_H_
#define PCI_DATA_H_

#ifndef DECLARE_CARD_DATASTRUCTURES
#define DECLARE_CARD_DATASTRUCTURES
#endif
#include "xf86PciInfo.h"

void
xf86SetupPciData(SymTabPtr *NameInfo,
		 pciVendorDeviceInfo **DeviceInfo,
		 pciVendorCardInfo **CardInfo);
#endif

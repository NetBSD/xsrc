/* $XFree86: Exp $ */

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

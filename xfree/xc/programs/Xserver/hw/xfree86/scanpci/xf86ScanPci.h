/* $XFree86: xc/programs/Xserver/hw/xfree86/scanpci/xf86ScanPci.h,v 1.1 2000/02/08 13:13:33 eich Exp $ */

#ifndef SCANPCI_H_
#define SCANPCI_H_

#ifndef DECLARE_CARD_DATASTRUCTURES
#define DECLARE_CARD_DATASTRUCTURES
#endif
#include "xf86PciInfo.h"

void
xf86SetupScanPci(SymTabPtr *NameInfo,
		 pciVendorDeviceInfo **DeviceInfo,
		 pciVendorCardInfo **CardInfo);
void xf86DisplayPCICardInfo(int);
CARD32 xf86FindPCIClassInCardList(
    unsigned short vendorID, unsigned short subsystemID);
CARD32 xf86FindPCIClassInDeviceList(
    unsigned short vendorID, unsigned short deviceID);

#endif

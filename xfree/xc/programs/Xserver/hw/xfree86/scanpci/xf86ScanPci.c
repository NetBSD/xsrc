/* $XFree86: xc/programs/Xserver/hw/xfree86/scanpci/xf86ScanPci.c,v 1.11 2000/04/05 18:13:58 dawes Exp $ */
/*
 * Display the Subsystem Vendor Id and Subsystem Id in order to identify
 * the cards installed in this computer
 *
 * Copyright 1995-2000 by The XFree86 Project, Inc.
 *
 * A lot of this comes from Robin Cutshaw's scanpci
 *
 */

/* XXX This is including a lot of stuff that modules should not include! */

#include "X.h"
#include "os.h"
#include "xf86.h"
#include "xf86Priv.h"
#include "xf86Pci.h"
#include "xf86_OSproc.h"

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
#define VENDOR_INCLUDE_NONVIDEO		TRUE
#include "xf86PciInfo.h"
#include "xf86ScanPci.h"

/*
 * PCI classes that have messages printed always.  The others are only
 * have a message printed when the vendor/dev IDs are recognised.
 */
#define PCIALWAYSPRINTCLASSES(b,s)					      \
    (((b) == PCI_CLASS_PREHISTORIC && (s) == PCI_SUBCLASS_PREHISTORIC_VGA) || \
     ((b) == PCI_CLASS_DISPLAY) ||					      \
     ((b) == PCI_CLASS_MULTIMEDIA && (s) == PCI_SUBCLASS_MULTIMEDIA_VIDEO))


#ifdef XFree86LOADER

#include "xf86Module.h"

static XF86ModuleVersionInfo scanPciVersRec = {
	"scanpci",
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

XF86ModuleData scanpciModuleData = { &scanPciVersRec, NULL, NULL };

#else

#endif /* XFree86LOADER */

void
xf86SetupScanPci(SymTabPtr *NameInfo,
		 pciVendorDeviceInfo **DeviceInfo,
		 pciVendorCardInfo **CardInfo)
{
    *CardInfo = xf86PCICardInfoData;
    *DeviceInfo = xf86PCIVendorInfoData;
    *NameInfo = xf86PCIVendorNameInfoData;
}


void
xf86DisplayPCICardInfo(int verbosity)
{
    pciConfigPtr pcrp, *pcrpp;
    int i = 0, j, k;

    xf86EnableIO();
    pcrpp = xf86scanpci(0);
   
    if (pcrpp == NULL) {
        xf86MsgVerb(X_NONE,0,"No PCI info available\n");
	return;
    }
    xf86MsgVerb(X_NONE,0,"Probing for PCI devices (Bus:Device:Function)\n\n");
    while ((pcrp = pcrpp[i])) {
	char *vendorname = NULL, *cardname = NULL;
	char *chipvendorname = NULL, *chipname = NULL;
	Bool noCard = FALSE;

	xf86MsgVerb(X_NONE,-verbosity,
		    "(%d:%d:%d) ", pcrp->busnum, pcrp->devnum,
		pcrp->funcnum);

	/* first let's look for the card itself, but only if information
	 * is available
	 */
	if ( pcrp->pci_subsys_vendor || pcrp->pci_subsys_card ) {
	    k = 0;
	    while (xf86PCIVendorNameInfoData[k].token) {
	      if (xf86PCIVendorNameInfoData[k].token == pcrp->pci_subsys_vendor) 
		vendorname = (char*)xf86PCIVendorNameInfoData[k].name;
	      k++;
	    }
	    k = 0; j = -1;
	    while(xf86PCICardInfoData[k].VendorID) {
		if (xf86PCICardInfoData[k].VendorID == pcrp->pci_subsys_vendor) {
		    j = 0;
		    while (xf86PCICardInfoData[k].Device[j].CardName) {
			if (xf86PCICardInfoData[k].Device[j].SubsystemID ==
			    pcrp->pci_subsys_card) {
			    cardname =
			      xf86PCICardInfoData[k].Device[j].CardName;
			    break;
			}
			j++;
		    }
		    break;
		}
		k++;
	    }
	    if (vendorname)
		xf86MsgVerb(X_NONE,-verbosity,"%s ", vendorname);
	    if (cardname)
		xf86MsgVerb(X_NONE,-verbosity,"%s ", cardname);
	    if (vendorname && !cardname) {
	        if (pcrp->pci_subsys_card && (j >= 0))
		    xf86MsgVerb(X_NONE,-verbosity,"unknown card (0x%04x) ",
				pcrp->pci_subsys_card);
		else
		    xf86MsgVerb(X_NONE,-verbosity,"card ",
				pcrp->pci_subsys_card);
	    }
	}
	if (!(cardname || vendorname)) {
	    /*
	     * we didn't find text representation of the information 
	     * about the card
	     */
	    if ( pcrp->pci_subsys_vendor || pcrp->pci_subsys_card ) {
		/*
		 * if there was information and we just couldn't interpret
		 * it, print it out as unknown, anyway
		 */
		xf86MsgVerb(X_NONE,-verbosity,
			    "unknown card (0x%04x/0x%04x) ",
			    pcrp->pci_subsys_vendor, pcrp->pci_subsys_card);
	    }
	    else {
		/*
		 * if there was no info to begin with, only print in
		 * verbose mode
		 */
		if (verbosity > 1)
		    xf86MsgVerb(X_NONE,-verbosity,
				"unknown card (0x%04x/0x%04x) ",
				pcrp->pci_subsys_vendor, pcrp->pci_subsys_card);
		else
		    noCard = TRUE;
	    }
	}
	/* now check for the chipset used */
	k = 0;
	while (xf86PCIVendorNameInfoData[k].token) {
	  if (xf86PCIVendorNameInfoData[k].token == pcrp->pci_vendor) 
	    chipvendorname = (char *)xf86PCIVendorNameInfoData[k].name;
	  k++;
	}
	k = 0;
	while(xf86PCIVendorInfoData[k].VendorID) {
	    if (xf86PCIVendorInfoData[k].VendorID == pcrp->pci_vendor) {
		j = 0;
		while (xf86PCIVendorInfoData[k].Device[j].DeviceName) {
		    if (xf86PCIVendorInfoData[k].Device[j].DeviceID ==
			pcrp->pci_device) {
			chipname =
			  xf86PCIVendorInfoData[k].Device[j].DeviceName;
			break;
		    }
		    j++;
		}
		break;
	    }
	    k++;
	}
	if (noCard) {
	  if (chipvendorname && chipname)
	    xf86MsgVerb(X_NONE,-verbosity,"%s %s",
			chipvendorname,chipname);
	  else if (chipvendorname)
	    xf86MsgVerb(X_NONE,-verbosity,
			"unknown chip (DeviceId 0x%04x) from %s",
			pcrp->pci_device,chipvendorname);
	  else
	    xf86MsgVerb(X_NONE,-verbosity,
			"unknown chipset(0x%04x/0x%04x)",
			pcrp->pci_vendor,pcrp->pci_device);
	  xf86MsgVerb(X_NONE,-verbosity,"\n");
	}
	else
	{
	  if (chipvendorname && chipname)
	    xf86MsgVerb(X_NONE,-verbosity,"using a %s %s",
			chipvendorname,chipname);
	  else if (chipvendorname)
	    xf86MsgVerb(X_NONE,-verbosity,
			"using an unknown chip (DeviceId 0x%04x) from %s",
			pcrp->pci_device,chipvendorname);
	  else
	    xf86MsgVerb(X_NONE,-verbosity,
			"using an unknown chipset(0x%04x/0x%04x)",
			pcrp->pci_vendor,pcrp->pci_device);
	  xf86MsgVerb(X_NONE,-verbosity,"\n");
	}
	i++;
    }
}

CARD32
xf86FindPCIClassInCardList(unsigned short vendorID, unsigned short subsystemID)
{
    pciVendorCardInfo   *cardInfo = xf86PCICardInfoData;
    int i,j;
    
    for(i = 0; cardInfo[i].VendorID != 0;i++) {
	if (cardInfo[i].VendorID == vendorID) {
	    for (j = 0; cardInfo[i].Device[j].SubsystemID != 0; j++) {
		if (cardInfo[i].Device[j].SubsystemID == subsystemID) 
		    return cardInfo[i].Device[j].class;
	    }
	    break;
	}
    }
    return 0;
}

CARD32
xf86FindPCIClassInDeviceList(unsigned short vendorID, unsigned short deviceID)
{
    pciVendorDeviceInfo *vendorInfo = xf86PCIVendorInfoData;
     int i,j;

    for(i = 0; vendorInfo[i].VendorID != 0;i++) {
	if (vendorInfo[i].VendorID == vendorID) {
	    for (j = 0; vendorInfo[i].Device[j].DeviceID != 0; j++) {
		if (vendorInfo[i].Device[j].DeviceID == deviceID) 
		    return vendorInfo[i].Device[j].class;
	    }
	    break;
	}
    }
    return 0;
}


/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/vga/vgaPCI.c,v 3.10 1996/09/29 13:41:34 dawes Exp $ */
/*
 * PCI Probe
 *
 * Copyright 1995  The XFree86 Project, Inc.
 *
 * A lot of this comes from Robin Cutshaw's scanpci
 *
 */
/* $XConsortium: vgaPCI.c /main/2 1996/01/13 13:15:11 kaleb $ */

#include "xf86.h"
#include "xf86Priv.h"
#include "xf86_OSlib.h"
#include "xf86_HWlib.h"
#include "xf86_Config.h"
#include "vga.h"

#define INIT_PCI_VENDOR_INFO
#include "vgaPCI.h"

vgaPCIInformation *
vgaGetPCIInfo()
{
    vgaPCIInformation *info = NULL;
    pciConfigPtr pcrp, *pcrpp;
    Bool found = FALSE;
    int i = 0;

    pcrpp = xf86scanpci(vga256InfoRec.scrnIndex);

    if (!pcrpp)
	return NULL;

    while (pcrp = pcrpp[i]) {
	if ((pcrp->_base_class == PCI_CLASS_PREHISTORIC &&
	     pcrp->_sub_class == PCI_SUBCLASS_PREHISTORIC_VGA) ||
	    (pcrp->_base_class == PCI_CLASS_DISPLAY &&
	     pcrp->_sub_class == PCI_SUBCLASS_DISPLAY_VGA)) {
	    found = TRUE;
	    if ((info = (vgaPCIInformation *)
		 xalloc(sizeof(vgaPCIInformation))) == NULL)
		return NULL;
	    info->Vendor = pcrp->_vendor;
	    info->ChipType = pcrp->_device;
	    info->ChipRev = pcrp->_rev_id;
	    info->Bus = pcrp->_bus;
	    info->Card = pcrp->_cardnum;
	    info->Func = pcrp->_func;
	    info->AllCards = pcrpp;
	    info->ThisCard = pcrp;
	    info->MemBase = 0;
	    info->IOBase = 0;

	    /*
	     * It should be possible to move this out into the Cirrus
	     * driver now.
	     */
	    if (info->Vendor == PCI_VENDOR_CIRRUS &&
		(info->ChipType == PCI_CHIP_GD5462) ||
		(info->ChipType == PCI_CHIP_GD5464) ||
		(info->ChipType == PCI_CHIP_GD7548)) {
	      info->IOBase = pcrp->_base0;
	      info->MemBase = pcrp->_base1;

	      xf86writepci(vga256InfoRec.scrnIndex, pcrp->_bus,
			   pcrp->_cardnum, pcrp->_func,
			   PCI_CMD_STAT_REG,
			   PCI_CMD_IO_ENABLE | PCI_CMD_MEM_ENABLE,
			   PCI_CMD_IO_ENABLE | PCI_CMD_MEM_ENABLE);
	      break;
	    }


	    if (pcrp->_base0) {
		if (pcrp->_base0 & 1)
		    info->IOBase = pcrp->_base0 & 0xFFFFFFFC;
		else
		    info->MemBase = pcrp->_base0 & 0xFFFFFFF0;
	    }
	    if (pcrp->_base1) {
		if (pcrp->_base1 & 1) {
		    if (!info->IOBase)
			info->IOBase = pcrp->_base1 & 0xFFFFFFFC;
		} else
		    if (!info->MemBase)
			info->MemBase = pcrp->_base1 & 0xFFFFFFF0;
	    }
	    if (pcrp->_base2) {
		if (pcrp->_base2 & 1) {
		    if (!info->IOBase)
			info->IOBase = pcrp->_base2 & 0xFFFFFFFC;
		} else
		    if (!info->MemBase)
			info->MemBase = pcrp->_base2 & 0xFFFFFFF0;
	    }
	    if (pcrp->_base3) {
		if (pcrp->_base3 & 1) {
		    if (!info->IOBase)
			info->IOBase = pcrp->_base3 & 0xFFFFFFFC;
		} else
		    if (!info->MemBase)
			info->MemBase = pcrp->_base3 & 0xFFFFFFF0;
	    }
	    if (pcrp->_base4) {
		if (pcrp->_base4 & 1) {
		    if (!info->IOBase)
			info->IOBase = pcrp->_base4 & 0xFFFFFFFC;
		} else
		    if (!info->MemBase)
			info->MemBase = pcrp->_base4 & 0xFFFFFFF0;
	    }
	    if (pcrp->_base5) {
		if (pcrp->_base5 & 1) {
		    if (!info->IOBase)
			info->IOBase = pcrp->_base5 & 0xFFFFFFFC;
		} else
		    if (!info->MemBase)
			info->MemBase = pcrp->_base5 & 0xFFFFFFF0;
	    }
	    break;
	}
	i++;
    }
    if (found && xf86Verbose) {
	int i = 0, j;
	char *vendorname = NULL, *chipname = NULL;

	while (xf86PCIVendorInfo[i].VendorName) {
	    if (xf86PCIVendorInfo[i].VendorID == info->Vendor) {
		j = 0;
		vendorname = xf86PCIVendorInfo[i].VendorName;
		while (xf86PCIVendorInfo[i].Device[j].DeviceName) {
		    if (xf86PCIVendorInfo[i].Device[j].DeviceID ==
			info->ChipType) {
			chipname = xf86PCIVendorInfo[i].Device[j].DeviceName;
			break;
		    }
		    j++;
		}
		break;
	    }
	    i++;
	}

	ErrorF("%s %s: PCI: ", XCONFIG_PROBED, vga256InfoRec.name);
	if (vendorname)
	    ErrorF("%s ", vendorname);
	else
	    ErrorF("Unknown vendor (0x%04x) ", info->Vendor);
	if (chipname)
	    ErrorF("%s ", chipname);
	else
	    ErrorF("Unknown chipset (0x%04x) ", info->ChipType);
	ErrorF("rev %d", info->ChipRev);

	if (info->MemBase)
	    ErrorF(", Memory @ 0x%08x", info->MemBase);
	if (info->IOBase)
	    ErrorF(", I/O @ 0x%04x", info->IOBase);
	ErrorF("\n");
    }
    return info;
}


/*
 * Copyright (C) 1994-2003 The XFree86 Project, Inc.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT.  IN NO EVENT SHALL THE XFREE86 PROJECT BE LIABLE
 * FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Except as contained in this notice, the name of the XFree86 Project
 * shall not be used in advertising or otherwise to promote the sale,
 * use or other dealings in this Software without prior written
 * authorization from the XFree86 Project.
 */

#ifdef HAVE_XORG_CONFIG_H
#include <xorg-config.h>
#endif

#include <sys/types.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <dev/pci/pciio.h>

#include "xf86.h"
#include "xf86Priv.h"
#include "xf86OSpriv.h"

#include "Pci.h"

static CARD32 netbsdPciConfRead(PCITAG, int);
static void netbsdPciConfWrite(PCITAG, int, CARD32);
static void netbsdPciSetBits(PCITAG, int, CARD32, CARD32);

static int devpci = -1;

static pciBusFuncs_t netbsdFuncs0 = {
/* pciReadLong      */	netbsdPciConfRead,
/* pciWriteLong     */	netbsdPciConfWrite,
/* pciSetBitsLong   */	netbsdPciSetBits,
/* pciAddrHostToBus */	pciAddrNOOP,
/* pciAddrBusToHost */	pciAddrNOOP
};

static pciBusInfo_t netbsdPci0 = {
/* configMech  */	PCI_CFG_MECH_OTHER,
/* numDevices  */	32,
/* secondary   */	FALSE,
/* primary_bus */	0,
/* funcs       */	&netbsdFuncs0,
/* pciBusPriv  */	NULL,
/* bridge      */	NULL
};

void
netbsdPciInit()
{
	struct pciio_businfo pci_businfo;

#ifdef PCI_VIA_TTYE0
	/* XXX we should already have this open somewhere */
	devpci = open("/dev/ttyE0", O_RDWR);
	if (devpci == -1) {
		ErrorF("netbsdPciInit: can't open /dev/ttyE0\n");
		return;
	}
#else
	devpci = open("/dev/pci0", O_RDWR);
	if (devpci == -1) {
		ErrorF("netbsdPciInit: can't open /dev/pci0\n");
		return;
	}
#endif

	pciNumBuses    = 1;
	pciBusInfo[0]  = &netbsdPci0;
	pciFindFirstFP = pciGenFindFirst;
	pciFindNextFP  = pciGenFindNext;
#ifdef PCI_VIA_TTYE0
	netbsdPci0.numDevices = 1;
#else
	/* use businfo to get the number of devs */
	if (ioctl(devpci, PCI_IOC_BUSINFO, &pci_businfo) != 0)
		ErrorF("netbsdPciInit: not a PCI bus device");
	netbsdPci0.numDevices = pci_businfo.maxdevs;
#endif
}

static CARD32
netbsdPciConfRead(PCITAG tag, int reg)
{
#ifdef PCI_VIA_TTYE0
	struct pciio_cfgreg cfgr;

	cfgr.reg = reg;
	if (ioctl(devpci, PCI_IOC_CFGREAD, &cfgr) == -1) {
		xf86Msg(X_ERROR, "netbsdPciConfRead: failed on ttyE0\n");
		return 0;
	}

	return (cfgr.val);
#else
	struct pciio_bdf_cfgreg bdfr;

	bdfr.bus      = PCI_BUS_FROM_TAG(tag);
	bdfr.device   = PCI_DEV_FROM_TAG(tag);
	bdfr.function = PCI_FUNC_FROM_TAG(tag);
	bdfr.cfgreg.reg = reg;

	if (ioctl(devpci, PCI_IOC_BDF_CFGREAD, &bdfr) == -1)
		FatalError("netbsdPciConfRead: failed on %d/%d/%d\n",
		    bdfr.bus, bdfr.device, bdfr.function);

	return (bdfr.cfgreg.val);
#endif
}

static void
netbsdPciConfWrite(PCITAG tag, int reg, CARD32 val)
{
#ifdef PCI_VIA_TTYE0
	struct pciio_cfgreg cfgr;

	cfgr.reg = reg;
	cfgr.val = val;
	if (ioctl(devpci, PCI_IOC_CFGWRITE, &cfgr) == -1)
		FatalError("netbsdPciConfWrite: failed on ttyE0\n");
#else
	struct pciio_bdf_cfgreg bdfr;

	bdfr.bus      = PCI_BUS_FROM_TAG(tag);
	bdfr.device   = PCI_DEV_FROM_TAG(tag);
	bdfr.function = PCI_FUNC_FROM_TAG(tag);
	bdfr.cfgreg.reg = reg;
	bdfr.cfgreg.val = val;

	if (ioctl(devpci, PCI_IOC_BDF_CFGWRITE, &bdfr) == -1)
		FatalError("netbsdPciConfWrite: failed on %d/%d/%d\n",
		    bdfr.bus, bdfr.device, bdfr.function);
#endif
}

static void
netbsdPciSetBits(PCITAG tag, int reg, CARD32 mask, CARD32 bits)
{
	CARD32 val;

	val = netbsdPciConfRead(tag, reg);
	val = (val & ~mask) | (bits & mask);
	netbsdPciConfWrite(tag, reg, val);
}

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

CARD32 netbsdPciConfRead(PCITAG, int);
void netbsdPciConfWrite(PCITAG, int, CARD32);
void netbsdPciSetBits(PCITAG, int, CARD32, CARD32);

static int devpci = -1;

pciBusInfo_t netbsdPci0 = {
/* configMech  */	PCI_CFG_MECH_OTHER,
/* numDevices  */	32,
/* secondary   */	FALSE,
/* primary_bus */	0,
/* io_base */		0,	/* XXX */
/* io_size */		0,	/* XXX */
/* funcs       */	{
				netbsdPciConfRead,
				netbsdPciConfWrite,
				netbsdPciSetBits,
				pciAddrNOOP,
				pciAddrNOOP
			},
/* pciBusPriv  */	NULL
};

void
netbsdPciInit()
{
    	struct pciio_businfo pci_businfo;

	devpci = open("/dev/pci0", O_RDWR);
	if (devpci == -1)
		FatalError("netbsdPciInit: can't open /dev/pci0\n");

	pciNumBuses    = 1;
	pciBusInfo[0]  = &netbsdPci0;
	pciFindFirstFP = pciGenFindFirst;
	pciFindNextFP  = pciGenFindNext;
	/* use businfo to get the number of devs */
	if (ioctl(devpci, PCI_IOC_BUSINFO, &pci_businfo) != 0)
	    FatalError("netbsdPciInit: not a PCI bus device");
	netbsdPci0.numDevices = pci_businfo.maxdevs;
}

CARD32
netbsdPciConfRead(PCITAG tag, int reg)
{
	struct pciio_bdf_cfgreg bdfr;

	bdfr.bus      = PCI_BUS_FROM_TAG(tag);
	bdfr.device   = PCI_DEV_FROM_TAG(tag);
	bdfr.function = PCI_FUNC_FROM_TAG(tag);
	bdfr.cfgreg.reg = reg;

	if (ioctl(devpci, PCI_IOC_BDF_CFGREAD, &bdfr) == -1)
		FatalError("netbsdPciConfRead: failed on %d/%d/%d\n",
		    bdfr.bus, bdfr.device, bdfr.function);

	return (bdfr.cfgreg.val);
}

void
netbsdPciConfWrite(PCITAG tag, int reg, CARD32 val)
{
	struct pciio_bdf_cfgreg bdfr;

	bdfr.bus      = PCI_BUS_FROM_TAG(tag);
	bdfr.device   = PCI_DEV_FROM_TAG(tag);
	bdfr.function = PCI_FUNC_FROM_TAG(tag);
	bdfr.cfgreg.reg = reg;
	bdfr.cfgreg.val = val;

	if (ioctl(devpci, PCI_IOC_BDF_CFGWRITE, &bdfr) == -1)
		FatalError("netbsdPciConfWrite: failed on %d/%d/%d\n",
		    bdfr.bus, bdfr.device, bdfr.function);
}

void
netbsdPciSetBits(PCITAG tag, int reg, CARD32 mask, CARD32 bits)
{
	CARD32 val;

	val = netbsdPciConfRead(tag, reg);
	val = (val & ~mask) | (bits & mask);
	netbsdPciConfWrite(tag, reg, val);
}

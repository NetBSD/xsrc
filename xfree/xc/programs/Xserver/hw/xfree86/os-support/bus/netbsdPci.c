/* $XFree86: xc/programs/Xserver/hw/xfree86/os-support/bus/freebsdPci.c,v 1.1 2000/02/12 20:45:42 dawes Exp $ */

/* xxx need copyright notice here */
/* totally different to other files, old copyright removed.  is this ok?? */

#include "Pci.h"

#include <machine/pio.h>
#include <machine/sysarch.h>

/*
 * NetBSD/alpha platform specific PCI access functions.
 * Needs NetBSD kernel version 1.5 or later.
 */
CARD32 netbsdPciCfgRead(PCITAG tag, int off);
void netbsdPciCfgWrite(PCITAG, int off, CARD32 val);
void netbsdPciCfgSetBits(PCITAG tag, int off, CARD32 mask, CARD32 bits);

pciBusInfo_t netbsdPci0 = {
	PCI_CFG_MECH_OTHER, 32, FALSE, 0, 0, 0,
	{ netbsdPciCfgRead, netbsdPciCfgWrite, netbsdPciCfgSetBits,
	  pciAddrNOOP, pciAddrNOOP},
	NULL
};

#define BUS(tag)	(((tag)>>16)&0xff)
#define DFN(tag)	(((tag)>>8)&0xff)
#define DEV(dfn)	(((DFN(tag))>>3)&0x1f)
#define FNC(dfn)	((DFN(tag))&0x7)

void  
netbsdPciInit()
{
	if (alpha_pci_io_enable(1) != 0)
		fprintf(stderr, "alpha_pci_io_enable failed!\n");

	pciNumBuses    = 1;
	pciBusInfo[0]  = &netbsdPci0;
	pciFindFirstFP = pciGenFindFirst;
	pciFindNextFP  = pciGenFindNext;
}

CARD32
netbsdPciCfgRead(PCITAG tag, int off)
{
	return(alpha_pci_conf_read(BUS(tag), DEV(tag), FNC(tag), off));
		return ~0;
}

void
netbsdPciCfgWrite(PCITAG tag, int off, CARD32 val)
{
	alpha_pci_conf_write(BUS(tag), DEV(tag), FNC(tag), off, val);
}

void
netbsdPciCfgSetBits(PCITAG tag, int off, CARD32 mask, CARD32 bits)
{
	CARD32	val = netbsdPciCfgRead(tag, off);
	val = (val & ~mask) | (bits & mask);
	netbsdPciCfgWrite(tag, off, val);
}

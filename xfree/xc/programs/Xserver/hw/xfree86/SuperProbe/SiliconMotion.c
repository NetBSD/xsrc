/* $XFree86: xc/programs/Xserver/hw/xfree86/SuperProbe/SiliconMotion.c,v 1.1 2000/11/28 20:59:16 dawes Exp $ */
/*
 * (c) Copyright 1993,1994 by David Wexelblat <dwex@xfree86.org>
 * (c) Copyright 2000 by Silicon Motion <frido@siliconmotion.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a 
 * copy of this software and associated documentation files (the "Software"), 
 * to deal in the Software without restriction, including without limitation 
 * the rights to use, copy, modify, merge, publish, distribute, sublicense, 
 * and/or sell copies of the Software, and to permit persons to whom the 
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL 
 * DAVID WEXELBLAT BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, 
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF 
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE 
 * SOFTWARE.
 * 
 * Except as contained in this notice, the name of David Wexelblat shall not be
 * used in advertising or otherwise to promote the sale, use or other dealings
 * in this Software without prior written authorization from David Wexelblat.
 *
 */

#include "Probe.h"

static Word Ports[] = {0x000, 0x000};
#define NUMPORTS (sizeof(Ports)/sizeof(Word))

static int MemProbe_SiliconMotion __STDCARGS((int));

Chip_Descriptor SiliconMotion_Descriptor = {
	"Silicon Motion",
	Probe_SiliconMotion,
	Ports,
	NUMPORTS,
	FALSE,
	FALSE,
	FALSE,
	MemProbe_SiliconMotion,
};

Bool Probe_SiliconMotion(Chipset)
int *Chipset;
{
	int i = 0;

	if (!NoPCI)
	{
		while ((pcrp = pci_devp[i]) != (struct pci_config_reg *)NULL)
		{
			if (pcrp->_vendor == PCI_VENDOR_SMI && pcrp->_status_command & 7)
			{
				switch (pcrp->_device)
				{
					case PCI_CHIP_SMI_910:
						*Chipset = CHIP_SMI_910;
						PCIProbed = TRUE;
						break;

					case PCI_CHIP_SMI_810:
						*Chipset = CHIP_SMI_810;
						PCIProbed = TRUE;
						break;

					case PCI_CHIP_SMI_820:
						*Chipset = CHIP_SMI_820;
						PCIProbed = TRUE;
						break;

					case PCI_CHIP_SMI_710:
						*Chipset = CHIP_SMI_710;
						PCIProbed = TRUE;
						break;

					case PCI_CHIP_SMI_712:
						*Chipset = CHIP_SMI_712;
						PCIProbed = TRUE;
						break;

					case PCI_CHIP_SMI_720:
						*Chipset = CHIP_SMI_720;
						PCIProbed = TRUE;
						break;
				}

				if (PCIProbed)
				{
					return(TRUE);
				}
			}
			i++;
		}
	}

	return(FALSE);
}

static int MemProbe_SiliconMotion(Chipset)
int Chipset;
{
	Byte config;
	int Mem = 0;

	EnableIOPorts(NUMPORTS, Ports);

	config = rdinx(SEQ_IDX, 0x71);
	switch (Chipset)
	{
		case CHIP_SMI_910:
		case CHIP_SMI_810:
		case CHIP_SMI_710:
		case CHIP_SMI_712:
		{
			int memsize[] = { 1, 2, 4, 0 };
			Mem = memsize[config >> 6] * 1024;
			break;
		}

		case CHIP_SMI_820:
		{
			int memsize[] = { 0, 2, 4, 6 };
			Mem = memsize[config >> 6] * 1024 + 512;
			break;
		}

		case CHIP_SMI_720:
		{
			int memsize[] = { 16, 2, 4, 8 };
			Mem = memsize[config >> 6] * 1024;
			break;
		}
	}

	DisableIOPorts(NUMPORTS, Ports);
	return(Mem);
}

/* $XFree86: xc/programs/Xserver/hw/xfree86/SuperProbe/Rendition.c,v 3.1.6.2 1999/11/26 15:22:51 hohndel Exp $ */
/*
 * (c) Copyright 1997 by Dirk Hohndel <hohndel@xfree86.org>
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
 * Except as contained in this notice, the name of Dirk Hohndel shall not be
 * used in advertising or otherwise to promote the sale, use or other dealings
 * in this Software without prior written authorization from Dirk Hohndel.
 *
 */

#include "Probe.h"

#define PCI_EN 0x80000000

#ifdef PC98
static Word Ports[] = {0xCF8, 0xCF9, 0xCFC, 0x000 };
#else
static Word Ports[] = {0xCF8, 0xCFA, 0xCFC, 0x000 };
#endif

#define NUMPORTS (sizeof(Ports)/sizeof(Word))

static int MemProbe_Rendition __STDCARGS((int));

Chip_Descriptor Rendition_Descriptor = {
	"Rendition",
	Probe_Rendition,
	Ports,
	NUMPORTS,
	FALSE,
	FALSE,
	FALSE,
	MemProbe_Rendition,
};

Bool Probe_Rendition(Chipset)
int *Chipset;
{
	int	  chipset_passed;
	int	  i = -1;
	Bool	  result = FALSE;
	struct pci_config_reg * pcip;

	/*
	 * to be able to detect multiple chips we need to set
	 * *Chipset to 0 first. We'll set it back to the value passed to
	 * us if nothing was found
	 */
	chipset_passed = *Chipset;
	*Chipset = 0;
	/*
	 * we only check for the Rendition in the PCI config data that we have
	 */
	while (pci_devp[++i] != NULL)
	{
	    pcip = pci_devp[i];
	    if (pcip->_vendor == PCI_VENDOR_RENDITION && pcrp->_status_command & 7)
	    {
	    	switch (pcip->_device)
		{
		case PCI_CHIP_V1000:
			*Chipset = CHIP_REND_V1000;
			result = TRUE;
			break;
		case PCI_CHIP_V2000:
			*Chipset = CHIP_REND_V2000;
			result = TRUE;
			break;
		}
	    }
	}
	if (!result)
	{
	    *Chipset = chipset_passed;
	}

	return(result);
}

static int MemProbe_Rendition(Chipset)
int Chipset;
{
	/*
	 * I don't know how to do that, yet
	 */
	return(0);
}

/*
 * (c) Copyright 1996 Alan Hourihane <alanh@fairlite.demon.co.uk>
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
 * ALAN HOURIHANE BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, 
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF 
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE 
 * SOFTWARE.
 * 
 * Except as contained in this notice, the name of Alan Hourihane shall not be
 * used in advertising or otherwise to promote the sale, use or other dealings
 * in this Software without prior written authorization from Alan Hourihane.
 *
 */

/* $XFree86: xc/programs/Xserver/hw/xfree86/SuperProbe/SiS.c,v 3.8 2000/02/12 20:45:15 dawes Exp $ */

#include "Probe.h"

static Word Ports[] = {0x000, 0x000, SEQ_IDX, SEQ_REG};
#define NUMPORTS (sizeof(Ports)/sizeof(Word))

static int MemProbe_SiS __STDCARGS((int));

Chip_Descriptor SiS_Descriptor = {
	"SiS",
	Probe_SiS,
	Ports,
	NUMPORTS,
	FALSE,
	FALSE,
	TRUE,
	MemProbe_SiS,
};

Bool Probe_SiS(Chipset)
int *Chipset;
{
	int i = 0;

	if (!NoPCI)
	{
	    while ((pcrp = pci_devp[i]) != (struct pci_config_reg *)NULL) {
		if (pcrp->_vendor == PCI_VENDOR_SIS)
		{
			switch (pcrp->_device)
			{
			case PCI_CHIP_SG86C201:
				*Chipset = CHIP_SIS86C201;
				break;
			case PCI_CHIP_SG86C202:
				*Chipset = CHIP_SIS86C202;
				break;
			case PCI_CHIP_SG86C205:
				*Chipset = CHIP_SIS86C205;
				break;
			case PCI_CHIP_SG86C215:		/* 86C215 */
				*Chipset = CHIP_SIS86C215;
				break;
			case PCI_CHIP_SIS5597:		/* 5597/5598 */
				*Chipset = CHIP_SIS5597;
				break;
			case PCI_CHIP_SIS6326:		/* 6326 */
				*Chipset = CHIP_SIS6326;
				break;
			case PCI_CHIP_SG86C225:
				*Chipset = CHIP_SIS86C225;
				break;
			case PCI_CHIP_SIS530:		/* 530/620 */
				*Chipset = CHIP_SIS530;
				break;
			case PCI_CHIP_SIS300:		/* 300 */
				*Chipset = CHIP_SIS300;
				break;
			case PCI_CHIP_SIS630:		/* 630 */
				*Chipset = CHIP_SIS630;
				break;
			case PCI_CHIP_SIS540:		/* 540 */
				*Chipset = CHIP_SIS540;
				break;
			default:
				*Chipset = CHIP_SIS_UNK;
				break;
			}
			PCIProbed = TRUE;
			return(TRUE);
		}
		i++;
	    }
	}

        return(FALSE);
}

static int MemProbe_SiS(Chipset)
int Chipset;
{
	int Mem = 0;
	unsigned char temp;
	unsigned char bsiz;
	unsigned char save;

        EnableIOPorts(NUMPORTS, Ports);

	/* unlock extended registers */
	save = rdinx(SEQ_IDX,0x05);
	wrinx(SEQ_IDX,0x05,0x86);

	switch (Chipset)
	{
	case CHIP_SIS86C201:
	case CHIP_SIS86C202:
	case CHIP_SIS86C205:
	case CHIP_SIS86C215:
	case CHIP_SIS86C225:
		switch (rdinx(SEQ_IDX, 0xF) & 0x03)
		{
		case 0x00:
			Mem = 1024;
			break;
		case 0x01:
			Mem = 2048;
			break;
		case 0x02:
			Mem = 4096;
			break;
		}
		break;
	case CHIP_SIS5597:
		bsiz = rdinx(SEQ_IDX,0x0C);
		bsiz = (bsiz >> 1) & 3;

		temp = rdinx(CRTC_IDX,0x2F);
		temp &= 7;
		temp++;
		if (bsiz > 0) temp = temp << 1;

		Mem = 256 * temp;
		break;
	case CHIP_SIS6326:
	case CHIP_SIS530:
		temp = rdinx(SEQ_IDX, 0xC);
		temp >>= 1;
		switch (temp & 0x0B)
		{
		case 0x00: 
    			Mem = 1024;
			break;
		case 0x01:
			Mem = 2048;
			break;
		case 0x02: 
			Mem = 4096;
			break;
		case 0x03: 
			if(Chipset == CHIP_SIS6326)
			    Mem = 1024;
			else
			    Mem = 0;
			break;
		case 0x08: 
    			Mem = 0;
			break;
		case 0x09:
			Mem = 2048;
			break;
		case 0x0A: 
			Mem = 4096;
			break;
		case 0x0B: 
			Mem = 8192;
			break;
		}
	case CHIP_SIS300:
	case CHIP_SIS630:
	case CHIP_SIS540:
		Mem = ((rdinx(SEQ_IDX, 0x14) & 0x3F) + 1) * 1024;
		break;
	}
	/* lock registers again */
	wrinx(SEQ_IDX,0x05,save);
        DisableIOPorts(NUMPORTS, Ports);
	return(Mem);
    }

/* $XFree86: xc/programs/Xserver/hw/xfree86/SuperProbe/Trident.c,v 3.11.2.5 1998/10/25 09:49:19 hohndel Exp $ */
/*
 * (c) Copyright 1993,1994 by David Wexelblat <dwex@xfree86.org>
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

/* $XConsortium: Trident.c /main/7 1996/10/24 07:10:11 kaleb $ */

#include "Probe.h"

static Word Ports[] = {0x000, 0x000, SEQ_IDX, SEQ_REG};
#define NUMPORTS (sizeof(Ports)/sizeof(Word))

static int MemProbe_Trident __STDCARGS((int));

Chip_Descriptor Trident_Descriptor = {
	"Trident",
	Probe_Trident,
	Ports,
	NUMPORTS,
	FALSE,
	FALSE,
	TRUE,
	MemProbe_Trident,
};

Bool Probe_Trident(Chipset)
int *Chipset;
{
        Bool result = FALSE;
	Byte chip, old, old1, val;
	unsigned char temp;
	int i = 0;

	if (!NoPCI)
	{
	    while ((pcrp = pci_devp[i]) != (struct pci_config_reg *)NULL) {
		if (pcrp->_vendor == PCI_VENDOR_TRIDENT)
		{
			switch (pcrp->_device)
			{
			case PCI_CHIP_9320:
				*Chipset = CHIP_TVGA9320;
				break;
			case PCI_CHIP_9420:
				*Chipset = CHIP_TVGA9420;
				break;
			case PCI_CHIP_9440:
				*Chipset = CHIP_TVGA9440;
				break;
			case PCI_CHIP_9660:
				outp(0x3C4, 0x09);
				temp = inp(0x3C5);
				switch (temp) {
					case 0x00:
						*Chipset = CHIP_TVGA9660;
						break;
					case 0x01:
						*Chipset = CHIP_TVGA9680;
						break;
					case 0x10:
						*Chipset = CHIP_TVGA9682;
						break;
					case 0x21:
						*Chipset = CHIP_TVGA9685;
						break;
					case 0x22:
					case 0x23:
						*Chipset = CHIP_TVGA9397;
						break;
					case 0x30:
					case 0x33:
					case 0x34:
					case 0x35:
					case 0xB3:
						*Chipset = CHIP_TVGA9385;
						break;
					case 0x38:
					case 0x3A:
						*Chipset = CHIP_TVGA9385_1;
						break;
					case 0x4A:
						*Chipset = CHIP_TVGA9388_1;
						break;
					case 0x40:
					case 0x41: /* Guessing */
					case 0x42: /* Guessing */
					case 0x43:
						*Chipset = CHIP_TVGA9382;
						break;
					case 0x50:
						*Chipset = CHIP_TVGA9692;
						break;
					default:
						Chip_data = temp;
						*Chipset = CHIP_TVGA_UNK;
						break;
				}
				break;
			case PCI_CHIP_9680:
				*Chipset = CHIP_TVGA9680;
				break;
			case PCI_CHIP_9682:
				*Chipset = CHIP_TVGA9682;
				break;
			case PCI_CHIP_9388:
				*Chipset = CHIP_TVGA9388;
				break;
			case PCI_CHIP_9397:
				*Chipset = CHIP_TVGA9397;
				break;
			case PCI_CHIP_9520:
				*Chipset = CHIP_TVGA9520;
				break;
			case PCI_CHIP_9750:
				*Chipset = CHIP_TVGA9750;
				break;
			case PCI_CHIP_9850:
				*Chipset = CHIP_TVGA9850;
				break;
			default:
				Chip_data = pcrp->_device_vendor;
				*Chipset = CHIP_TVGA_UNK;
				break;
			}
			PCIProbed = TRUE;
			return(TRUE);
		}
		i++;
	    }
	}

	Ports[0] = CRTC_IDX;
	Ports[1] = CRTC_REG;
        EnableIOPorts(NUMPORTS, Ports);

	old = rdinx(SEQ_IDX, 0x0B);
	wrinx(SEQ_IDX, 0x0B, 0x00);
	chip = inp(SEQ_REG);
	old1 = rdinx(SEQ_IDX, 0x0E);
	outp(SEQ_REG, 0);
	val = inp(SEQ_REG);
	outp(SEQ_REG, (old1 ^ 0x02));
	wrinx(SEQ_IDX, 0x0B, old);
	wrinx(SEQ_IDX, 0x0E, old1);
	if ((val & 0x0F) == 2)
	{
		result = TRUE;
		switch (chip)
		{
		case 0x01:		/* Can't happen - no "new mode" */
			*Chipset = CHIP_TVGA8800BR;
			break;
		case 0x02:
			*Chipset = CHIP_TVGA8800CS;
			break;
		case 0x03:
			*Chipset = CHIP_TVGA8900B;
			break;
		case 0x04:
		case 0x13:
			*Chipset = CHIP_TVGA8900C;
			break;
		case 0x23:
			*Chipset = CHIP_TVGA9000;
			break;
		case 0x33:
			*Chipset = CHIP_TVGA8900CL;
			break;
		case 0x43:
			*Chipset = CHIP_TVGA9000I;
			break;
		case 0x53:
			*Chipset = CHIP_TVGA9200CX;
			break;
		case 0x63:
			*Chipset = CHIP_TVGA9100B;
			break;
		case 0x73:
		case 0xF3:
			*Chipset = CHIP_TVGA9420;
			break;
		case 0x83:
			*Chipset = CHIP_TVGA8200;
			break;
		case 0x93:
			*Chipset = CHIP_TVGA9400CX;
			break;
		case 0xA3:
			*Chipset = CHIP_TVGA9320;
			break;
		case 0xC3:
			*Chipset = CHIP_TVGA9420D;
			break;
		case 0xD3:
			outp(0x3C4, 0x09);
			temp = inp(0x3C5);
			switch (temp) {
				case 0x00:
					*Chipset = CHIP_TVGA9660;
					break;
				case 0x01:
					*Chipset = CHIP_TVGA9680;
					break;
				case 0x10:
					*Chipset = CHIP_TVGA9682;
					break;
				case 0x21:
					*Chipset = CHIP_TVGA9685;
					break;
				case 0x22:
				case 0x23:
					*Chipset = CHIP_TVGA9397;
					break;
				case 0x30:
				case 0x33:
				case 0x34:
				case 0x35:
				case 0xB3:
					*Chipset = CHIP_TVGA9385;
					break;
				case 0x38:
				case 0x3A:
					*Chipset = CHIP_TVGA9385_1;
					break;
				case 0x4A:
					*Chipset = CHIP_TVGA9388_1;
					break;
				case 0x40:
				case 0x41: /* Guessing */
				case 0x42: /* Guessing */
				case 0x43:
					*Chipset = CHIP_TVGA9382;
					break;
				case 0x50:
					*Chipset = CHIP_TVGA9692;
					break;
				default:
					Chip_data = chip*256+temp;
					*Chipset = CHIP_TVGA_UNK;
					break;
			}
			break;
		case 0xE3:
			*Chipset = CHIP_TVGA9440;
			break;
		default:
			Chip_data = chip;
			*Chipset = CHIP_TVGA_UNK;
			break;
		}
	}
	else if ((chip == 1) && (testinx2(SEQ_IDX, 0x0E, 0x06)))
	{
		result = TRUE;
		*Chipset = CHIP_TVGA8800BR;
	}

        DisableIOPorts(NUMPORTS, Ports);
        return(result);
}

static int MemProbe_Trident(Chipset)
int Chipset;
{
	int Mem = 0;
	unsigned char temp;

        EnableIOPorts(NUMPORTS, Ports);
	temp = rdinx(CRTC_IDX, 0x1F);

	switch (temp & 0x07)
	{
	case 0:
	case 4:
		Mem = 256;
		break;
	case 1:
	case 5: /* New TGUI's don't support less than 1MB */
		if (Chipset >= CHIP_TVGA9660)
			Mem = 4096;
		else
			Mem = 512;
		break;
	case 2:
	case 6:
		Mem = 768;
		break;
	case 3:
		Mem = 1024;
		break;
	case 7:

		if (((temp & 0x0F)==0x0F) && (Chipset >= CHIP_TVGA9685))
			/* This is for the 9685 */
			Mem = 4096;
		else
			Mem = 2048;
		break;
	}

        DisableIOPorts(NUMPORTS, Ports);
	return(Mem);
}

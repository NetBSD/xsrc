/* $XFree86: xc/programs/Xserver/hw/xfree86/SuperProbe/ChipsTech.c,v 3.10.2.4 1998/10/11 12:35:26 hohndel Exp $ */
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

/* $XConsortium: ChipsTech.c /main/8 1996/10/28 04:23:17 kaleb $ */

#include "Probe.h"

static Word Ports[] = {0x3D6, 0x3D7};
#define NUMPORTS (sizeof(Ports)/sizeof(Word))

static int MemProbe_CT __STDCARGS((int));

Chip_Descriptor CT_Descriptor = {
	"CT",
	Probe_CT,
	Ports,
	NUMPORTS,
	FALSE,
	FALSE,
	TRUE,
	MemProbe_CT,
};

#ifdef __STDC__
Bool Probe_CT(int *Chipset)
#else
Bool Probe_CT(Chipset)
int *Chipset;
#endif
{
	Bool result = FALSE;
	Byte vers;

	EnableIOPorts(NUMPORTS, Ports);
	if ((testinx(0x3D6, 0x18) && (testinx2(0x3D6, 0x7E, 0x3F))))
	{
		/*
		 * It's a Chips & Tech.  Now figure out which one.
		 */
		result = TRUE;
		vers = rdinx(0x3D6, 0x00);
		switch (vers)
		{
		case 0x00:
			*Chipset = CHIP_CT451;
			break;
		case 0x10:
			*Chipset = CHIP_CT452;
			break;
		case 0x20:
			*Chipset = CHIP_CT455;
			break;
		case 0x30:
			*Chipset = CHIP_CT453;
			break;
		case 0x40:
			*Chipset = CHIP_CT450;
			break;
		case 0x50:
			*Chipset = CHIP_CT456;
			break;
		case 0x60:
			*Chipset = CHIP_CT457;
			break;
		case 0x70:
			*Chipset = CHIP_CTF65520;
			break;
		case 0x80:
		case 0xc0:				/* guess */
			*Chipset = CHIP_CTF65530;
			break;
		case 0x90:
			*Chipset = CHIP_CTF65510;
			break;
		case 0xa0:
			*Chipset = CHIP_CTF64200;
			break;
		case 0xb0:
			*Chipset = CHIP_CTF64300;
			break;
		case 0xd0:
			*Chipset = CHIP_CTF65540;
			break;
		case 0xd8:
		case 0xd9:
		case 0xda:
			*Chipset = CHIP_CTF65545;
			break;
		case 0xdb:
			*Chipset = CHIP_CTF65546;
			break;
		case 0xdc:
			*Chipset = CHIP_CTF65548;
			break;
		default:
			Chip_data = vers;
			*Chipset = CHIP_CT_UNKNOWN;
			break;
		}
	}
	/* 
	 * We could still have a HiQV style chipset. C&T have the PCI
	 * vendor ID stored in XR00 and XR01 for HiQV chips, regardless
	 * of whether the chip is actually connected to a PCI BUS. So
	 * probe for C&T vendor ID.
	 */
	if ((rdinx(0x3D6, 0x00) == 0x2C) && (rdinx(0x3D6, 0x01) == 0x10))
	{
		/*
		 * It's a HiQV PCI Chips & Tech.
		 * Now figure out which one on PCI device ID low
		 * stored in XR02.
		 */
		result = TRUE;
		*Chipset = CHIP_CT_UNKNOWN;
		vers = rdinx(0x3D6, 0x02);
		switch (vers)
		{
		case 0xe0:
			*Chipset = CHIP_CTF65550;
			break;
		case 0xe4:
			*Chipset = CHIP_CTF65554;
			break;
		case 0xe5:
			*Chipset = CHIP_CTF65555;
			break;
		case 0xf4:
			*Chipset = CHIP_CTF68554;
			break;
		case 0xc0:
			*Chipset = CHIP_CTF69000;
			break;
		default:
			Chip_data = vers;
			*Chipset = CHIP_CT_UNKNOWN;
			break;
		}
	}
	DisableIOPorts(NUMPORTS, Ports);
	return(result);
}

static int MemProbe_CT(Chipset)
int Chipset;
{
	int Mem = 0;

	EnableIOPorts(NUMPORTS, Ports);

	switch (Chipset)
	{
	case CHIP_CT450:
	case CHIP_CT456:
		if (rdinx(0x3D6, 0x04) & 0x01)
		{
			Mem = 512;
		}
		else
		{
			Mem = 256;
		}
		break;
	case CHIP_CT451:
	case CHIP_CT455:
	case CHIP_CT457:
		Mem = 256;
		break;
	case CHIP_CT452:
		switch (rdinx(0x3D6, 0x04) & 0x03)
		{
		case 0x00:
			Mem = 256;
			break;
		case 0x01:
			Mem = 512;
			break;
		case 0x02:
			Mem = 1024;
			break;
		}
		break;
	case CHIP_CT453:
		switch ((rdinx(0x3D6, 0x24) >> 1) & 0x03)
		{
		case 0x01:
			Mem = 512;
			break;
		case 0x02:
			Mem = 1024;
			break;
		case 0x03:
			Mem = 256;
			break;
		}
		break;
	case CHIP_CTF65510:
		Mem = 512;
		break;
	case CHIP_CTF65520:
	case CHIP_CTF65530:
	case CHIP_CTF65540:
	case CHIP_CTF65545:
	case CHIP_CTF65546:
	case CHIP_CTF65548:
		switch (rdinx(0x3D6, 0x0F) & 0x03)
		{
		case 0x00:
			Mem = 256;
			break;
		case 0x01:
			Mem = 512;
			break;
		case 0x02:	
		case 0x03:
			Mem = 1024;
			break;
		}
		break;
	case CHIP_CTF64200:
	case CHIP_CTF64300:
		switch (rdinx(0x3D6, 0x0F) & 0x03)
		{
		case 0x00:
			Mem = 256;
			break;
		case 0x01:
			Mem = 512;
			break;
		case 0x02:	
			Mem = 1024;
			break;
		case 0x03:
			Mem = 2048;
			break;
		}
		break;
	case CHIP_CTF65550:
		switch ((rdinx(0x3D6, 0x43) & 0x06) >> 1)
		{
		case 0x00:
			Mem = 1024;
			break;
		case 0x01:
			Mem = 2048;
			break;
		case 0x03:
		case 0x04:
			Mem = 4096;
			break;
		}
		break;	
	case CHIP_CTF65554:
	case CHIP_CTF65555:
	case CHIP_CTF68554:
		switch (rdinx(0x3D6, 0xE0) & 0x0F)
		{
		case 0x00:
			Mem = 512;
			break;
		case 0x01:
			Mem = 1024;
			break;
		case 0x02:
			Mem = 1536;
			break;
		case 0x03:
			Mem = 2048;
			break;
		case 0x07:
			Mem = 4096;
			break;
		}
		break;	
	case CHIP_CTF69000:
		Mem = 2048;
		break;
	}

	DisableIOPorts(NUMPORTS, Ports);
	return(Mem);
}

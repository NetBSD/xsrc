/*
 * Copyright 1998,1999 by Alan Hourihane, Wigan, England.
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Alan Hourihane not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  Alan Hourihane makes no representations
 * about the suitability of this software for any purpose.  It is provided
 * "as is" without express or implied warranty.
 *
 * ALAN HOURIHANE DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL ALAN HOURIHANE BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 *
 * Authors:  Alan Hourihane, alanh@fairlite.demon.co.uk
 *           Mike Chapman <mike@paranoia.com>, 
 *           Juanjo Santamarta <santamarta@ctv.es>, 
 *           Mitani Hiroshi <hmitani@drl.mei.co.jp> 
 *           David Thomas <davtom@dream.org.uk>. 
 */
/* $XFree86: xc/programs/Xserver/hw/xfree86/drivers/sis/sis_setup.c,v 1.3 2000/11/09 11:32:22 alanh Exp $ */


#include "fb.h"
#include "xf1bpp.h"
#include "xf4bpp.h"
#include "mibank.h"
#include "micmap.h"
#include "xf86.h"
#include "xf86_OSproc.h"
#include "xf86Resources.h"
#include "xf86_ansic.h"
#include "xf86Version.h"
#include "xf86PciInfo.h"
#include "xf86Pci.h"
#include "xf86cmap.h"
#include "vgaHW.h"
#include "xf86RAC.h"

#include "mipointer.h"

#include "mibstore.h"

#include "sis_regs.h"
#include "sis.h"

#ifdef XFreeXDGA
#define _XF86DGA_SERVER_
#include "extensions/xf86dgastr.h"
#endif

#ifdef DPMSExtension
#include "globals.h"
#define DPMS_SERVER
#include "extensions/dpms.h"
#endif

static	char	*dramTypeStr[] = {
		"Fast Page DRAM",
		"2 cycle EDO RAM",
		"1 cycle EDO RAM",
		"SDRAM/SGRAM",
		"SDRAM",
		"SGRAM",
		"ESDRAM"
		"" };

static	int	clockTable[4] = { 66, 75, 83, 100 };

static	void
sisOldChipSetup(ScrnInfoPtr pScrn)
{
	int	ramsize[4] = {1024, 2048, 4096, 1024};

	SISPTR(pScrn)->TurboQueue = FALSE;

	outb(VGA_SEQ_INDEX, RAMSize);
	pScrn->videoRam = ramsize[inb(VGA_SEQ_DATA) & 3];
}

static	void
sis530Setup(ScrnInfoPtr pScrn)
{
	SISPtr		pSiS = SISPTR(pScrn);
	int		ramsize[8] = { 1, 2, 4, 0, 0, 2, 4, 8};
	int		buswidth[8] = { 0, 64, 64, 0, 0, 32, 32, 64 };
	int		config;
	int		temp;

	if (pSiS->Chipset == PCI_CHIP_SIS5597)  {
		outb(VGA_SEQ_INDEX, FBSize);
		pScrn->videoRam = ((inb(VGA_SEQ_DATA) & 7) + 1)*256;
		outb(VGA_SEQ_INDEX, Mode64);
		if (inb(VGA_SEQ_DATA) & 6)
			pScrn->videoRam *= 2;
	}
	else  {
		outb(VGA_SEQ_INDEX, RAMSize);
		temp = inb(VGA_SEQ_DATA);
		config = ((temp & 0x10) >> 2 ) | ((temp & 6) >> 1);
		pScrn->videoRam = ramsize[config] * 1024;
		pSiS->BusWidth = buswidth[config];
	}

	if (pSiS->Chipset == PCI_CHIP_SIS530)  {
		outb(VGA_SEQ_INDEX, 0x10);
		pSiS->MemClock = clockTable[inb(VGA_SEQ_DATA) & 0x03] * 1000;
		outb(VGA_SEQ_INDEX, 0x0d);
		if (inb(VGA_SEQ_DATA) & 0x01)
			pSiS->Flags |= UMA;
	} else
		pSiS->MemClock = SiSMclk(pSiS);

	xf86DrvMsg(pScrn->scrnIndex, X_PROBED,
			"Detected memory clock : %3.3fMHz\n",
			pSiS->MemClock/1000.0);
}

static	void
sis300Setup(ScrnInfoPtr pScrn)
{
	SISPtr		pSiS = SISPTR(pScrn);
	int		bus[4] = {32, 64, 128, 32};
	unsigned int	config;

	pSiS->MemClock = SiSMclk(pSiS);

	outb(VGA_SEQ_INDEX, 0x14);
	config = inb(VGA_SEQ_DATA);
	pScrn->videoRam = ((config & 0x3F) + 1) * 1024;
	pSiS->BusWidth =bus[config >> 6];

	outb(VGA_SEQ_INDEX, 0x3A);
	config = inb(VGA_SEQ_DATA) & 3;

	xf86DrvMsg(pScrn->scrnIndex, X_PROBED,
			"Detected DRAM type : %s\n", dramTypeStr[config+4]);
	xf86DrvMsg(pScrn->scrnIndex, X_PROBED,
			"Detected memory clock : %3.3fMHz\n",
			pSiS->MemClock/1000.0);
	xf86DrvMsg(pScrn->scrnIndex, X_PROBED,
                        "Detected VRAM bus width is %d\n", pSiS->BusWidth);
}

void
SiSSetup(ScrnInfoPtr pScrn)
{
	SISPTR(pScrn)->Flags = 0;

	SISPTR(pScrn)->VBFlags = 0;
	switch	(SISPTR(pScrn)->Chipset)  {
	case	PCI_CHIP_SIS5597:
	case	PCI_CHIP_SIS6326:
	case	PCI_CHIP_SIS530:
		sis530Setup(pScrn);
		break;
	case	PCI_CHIP_SIS300:
	case	PCI_CHIP_SIS630:
	case	PCI_CHIP_SIS540:
		sis300Setup(pScrn);
		break;
	default:
		sisOldChipSetup(pScrn);
		break;
	}
}

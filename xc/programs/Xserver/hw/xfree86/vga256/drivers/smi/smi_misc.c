/******************************************************************************\

				   Copyright (c) 1999 by Silicon Motion, Inc.
							   All Rights Reserved

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that the
above copyright notice appear in all copies and that both that copyright notice
and this permission notice appear in supporting documentation, and that the name
of Silicon Motion, Inc. not be used in advertising or publicity pertaining to
distribution of the software without specific, written prior permission.
Silicon Motion, Inc. and its suppliers make no representations about the
suitability of this software for any purpose.  It is provided "as is" without
express or implied warranty.

SILICON MOTION, INC. DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT
SHALL SILICON MOTION, INC. AND/OR ITS SUPPLIERS BE LIABLE FOR ANY SPECIAL,
INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF
THIS SOFTWARE.
\******************************************************************************/
/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/smi/smi_misc.c,v 1.1.2.2 1999/12/11 17:43:21 hohndel Exp $ */

/*
 * Various functions used in the SMI driver. Right now, this only contains the
 * PCI probing function.
 *
 */

#include "X.h"
#include "input.h"
#include "screenint.h"

#include "compiler.h"
#include "xf86.h"
#include "xf86Priv.h"
#include "xf86_OSlib.h"
#include "xf86_HWlib.h"
#include "xf86_PCI.h"
#include "vga.h"
#include "vgaPCI.h"

#ifdef XFreeXDGA
#include "X.h"
#include "Xproto.h"
#include "scrnintstr.h"
#include "servermd.h"
#define _XF86DGA_SERVER_
#include "extensions/xf86dgastr.h"
#endif

#define XCONFIG_FLAGS_ONLY
#include "xf86_Config.h"

#include "regsmi.h"
#include "smi_driver.h"

extern vgaPCIInformation *vgaPCIInfo;
extern SymTabRec smiChipTable[];
extern SMIPRIV smiPriv;


/*
 * smiGetPCIInfo -- probe for PCI information
 */

SMIPCIInformation *
smiGetPCIInfo()
{
	static SMIPCIInformation info = {0, };
	pciConfigPtr pcrp = NULL;
	Bool found = FALSE;
	int i = 0;

	if (vgaPCIInfo && vgaPCIInfo->AllCards)
	{
		while (pcrp = vgaPCIInfo->AllCards[i])
		{
			if (pcrp->_vendor == PCI_SMI_VENDOR_ID && pcrp->_command != 0)
			{
				int ChipId = pcrp->_device;
				if (vga256InfoRec.chipID)
				{
					ErrorF("%s %s: SMI chipset override, using chip_id = 0x%04x"
						   " instead of 0x%04x\n", XCONFIG_GIVEN,
						   vga256InfoRec.name, vga256InfoRec.chipID, ChipId);
					ChipId = vga256InfoRec.chipID;
				}
				found = TRUE;

				switch (ChipId)
				{
					case PCI_910:
						info.ChipType = SMI_910;
						break;
					
					case PCI_810:
						info.ChipType = SMI_810;
						break;

					case PCI_820:
						info.ChipType = SMI_820;
						break;

					case PCI_710:
						info.ChipType = SMI_710;
						break;

					case PCI_712:
						info.ChipType = SMI_712;
						break;

					case PCI_720:
						info.ChipType = SMI_720;
						break;

					default:
						info.ChipType = SMI_UNKNOWN;
						info.DevID = pcrp->_device;
						break;
				}
				info.ChipRev = pcrp->_rev_id;
				info.MemBase = pcrp->_base0 & 0xFF800000;
				break;
			}
			i++;
		}
	}
	else
		found = FALSE;

	if (found && xf86Verbose)
	{
		if (info.ChipType != SMI_UNKNOWN)
		{
			ErrorF("%s %s: SMI: %s rev %x, Linear FB @ 0x%08lx\n",
					XCONFIG_PROBED, vga256InfoRec.name,
					xf86TokenToString(smiChipTable, info.ChipType),
					info.ChipRev, info.MemBase);
		}
	}

	if (found)
		return(&info);
	else
		return(NULL);
}

unsigned char ReadSeqc(unsigned char index)
{
	outb(0x3C4, index);
	return(inb(0x3C5));
}

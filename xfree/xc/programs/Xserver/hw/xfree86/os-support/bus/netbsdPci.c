/* $XFree86: xc/programs/Xserver/hw/xfree86/os-support/bus/freebsdPci.c,v 1.1 2000/02/12 20:45:42 dawes Exp $ */

/*-
 * Copyright (c) 2001 The NetBSD Foundation, Inc.
 * All rights reserved.
 *
 * This code is derived from software contributed to The NetBSD Foundation
 * by Simon Burge.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *        This product includes software developed by the NetBSD
 *        Foundation, Inc. and its contributors.
 * 4. Neither the name of The NetBSD Foundation nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE NETBSD FOUNDATION, INC. AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

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

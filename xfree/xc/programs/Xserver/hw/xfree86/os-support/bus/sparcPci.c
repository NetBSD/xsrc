/* $XFree86: xc/programs/Xserver/hw/xfree86/os-support/bus/sparcPci.c,v 1.5 2001/05/28 02:42:30 tsi Exp $ */
/*
 * Copyright 1998 by Concurrent Computer Corporation
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation, and that the name of Concurrent Computer
 * Corporation not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior
 * permission.  Concurrent Computer Corporation makes no representations
 * about the suitability of this software for any purpose.  It is
 * provided "as is" without express or implied warranty.
 *
 * CONCURRENT COMPUTER CORPORATION DISCLAIMS ALL WARRANTIES WITH REGARD
 * TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS, IN NO EVENT SHALL CONCURRENT COMPUTER CORPORATION BE
 * LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY
 * DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
 * ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 *
 * Copyright 1998 by Metro Link Incorporated
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation, and that the name of Metro Link
 * Incorporated not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior
 * permission.  Metro Link Incorporated makes no representations
 * about the suitability of this software for any purpose.  It is
 * provided "as is" without express or implied warranty.
 *
 * METRO LINK INCORPORATED DISCLAIMS ALL WARRANTIES WITH REGARD
 * TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS, IN NO EVENT SHALL METRO LINK INCORPORATED BE
 * LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY
 * DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
 * ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 */

#include "xf86.h"
#include "xf86Priv.h"
#include "xf86_OSlib.h"
#include "Pci.h"

#if defined(linux)
#include <asm/unistd.h>
#ifndef __NR_pciconfig_read
#define __NR_pciconfig_read  148
#define __NR_pciconfig_write 149
#endif
#endif

/*
 * UltraSPARC platform specific PCI access functions
 */
static CARD32 sparcPciCfgRead(PCITAG tag, int off);
static void sparcPciCfgWrite(PCITAG, int off, CARD32 val);
static void sparcPciCfgSetBits(PCITAG tag, int off, CARD32 mask, CARD32 bits);

pciBusInfo_t sparcPci0 = {
/* configMech  */	  PCI_CFG_MECH_OTHER,
/* numDevices  */	  32,
/* secondary   */	  FALSE,
/* primary_bus */	  0,
/* ppc_io_base */	  0,
/* ppc_io_size */	  0,		  
/* funcs       */	  {
	                    sparcPciCfgRead,
			    sparcPciCfgWrite,
			    sparcPciCfgSetBits,
			    pciAddrNOOP,
			    pciAddrNOOP
		          },
/* pciBusPriv  */	  NULL
};

void  
sparcPciInit()
{
  pciNumBuses    = 1;
  pciBusInfo[0]  = &sparcPci0;
  pciFindFirstFP = pciGenFindFirst;
  pciFindNextFP  = pciGenFindNext;
}


#if defined(linux)

/*
 * These funtions will work for Linux, but other OS's
 * are likely have a different mechanism for getting at
 * PCI configuration space
 */
static CARD32
sparcPciCfgRead(PCITAG tag, int off)
{
	int bus, dfn;
	CARD32 val = 0xffffffff;

	bus = PCI_BUS_FROM_TAG(tag);
	dfn = PCI_DFN_FROM_TAG(tag);
	
	syscall(__NR_pciconfig_read, bus, dfn, off, 4, &val);
	return(val);
}

static void
sparcPciCfgWrite(PCITAG tag, int off, CARD32 val)
{
	int bus, dfn;

	bus = PCI_BUS_FROM_TAG(tag);
	dfn = PCI_DFN_FROM_TAG(tag);
	
	syscall(__NR_pciconfig_write, bus, dfn, off, 4, &val);
}

static void
sparcPciCfgSetBits(PCITAG tag, int off, CARD32 mask, CARD32 bits)
{
    int bus, dfn;
    CARD32 val = 0xffffffff;

    bus = PCI_BUS_FROM_TAG(tag);
    dfn = PCI_DFN_FROM_TAG(tag);
	
    syscall(__NR_pciconfig_read, bus, dfn, off, 4, &val);
    val = (val & ~mask) | (bits & mask);
    syscall(__NR_pciconfig_write, bus, dfn, off, 4, &val);
}

#elif defined(sun)

/*
 * Obviously, these need to be fleshed out, probably using openpromio.
 */
static CARD32
sparcPciCfgRead(PCITAG tag, int off)
{
	FatalError("sparcPciCfgRead() called\n");

	return 0;	/* Gd'ole gcc... */
}

static void
sparcPciCfgWrite(PCITAG tag, int off, CARD32 val)
{
	FatalError("sparcPciCfbWrite() called\n");
}

static void
sparcPciCfgSetBits(PCITAG tag, int off, CARD32 mask, CARD32 bits)
{
	FatalError("sparcPciCfgSetBits() called\n");
}

#endif

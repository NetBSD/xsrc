/* $XFree86: xc/programs/Xserver/hw/xfree86/os-support/bus/linuxPci.c,v 1.3.6.1 2001/05/29 16:38:00 tsi Exp $ */
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

#include <stdio.h>
#include "compiler.h"
#include "xf86.h"
#include "xf86Priv.h"
#include "xf86_OSlib.h"
#include "Pci.h"

#include <asm/unistd.h>

/*
 * linux platform specific PCI access functions -- using /proc/bus/pci
 * needs kernel version 2.2.x
 */
CARD32 linuxPciCfgRead(PCITAG tag, int off);
void linuxPciCfgWrite(PCITAG, int off, CARD32 val);
void linuxPciCfgSetBits(PCITAG tag, int off, CARD32 mask, CARD32 bits);

pciBusInfo_t linuxPci0 = {
/* configMech  */	  PCI_CFG_MECH_OTHER,
/* numDevices  */	  32,
/* secondary   */	  FALSE,
/* primary_bus */	  0,
/* ppc_io_base */	  0,
/* ppc_io_size */	  0,		  
/* funcs       */	  {
	                    linuxPciCfgRead,
			    linuxPciCfgWrite,
			    linuxPciCfgSetBits,
			    pciAddrNOOP,
			    pciAddrNOOP
		          },
/* pciBusPriv  */	  NULL
};

#if X_BYTE_ORDER == X_BIG_ENDIAN
#define PCI_CPU(val)	(((val >> 24) & 0x000000ff) |	\
			 ((val >>  8) & 0x0000ff00) |	\
			 ((val <<  8) & 0x00ff0000) |	\
			 ((val << 24) & 0xff000000))
#else
#define PCI_CPU(val)	(val)
#endif

void  
linuxPciInit()
{
	struct stat st;
	if (-1 == stat("/proc/bus/pci",&st)) {
		/* when using this as default for all linux architectures,
                   we'll need a fallback for 2.0 kernels here */
		return;
	}
	pciNumBuses    = 1;
	pciBusInfo[0]  = &linuxPci0;
	pciFindFirstFP = pciGenFindFirst;
	pciFindNextFP  = pciGenFindNext;
}

static int
linuxPciOpenFile(PCITAG tag)
{
	static int	lbus,ldev,lfunc,fd = -1;
	int		bus, dev, func;
	char		file[32];
	
	bus  = PCI_BUS_FROM_TAG(tag);
	dev  = PCI_DEV_FROM_TAG(tag);
	func = PCI_FUNC_FROM_TAG(tag);
	if (fd == -1 || bus != lbus || dev != ldev || func != lfunc) {
		if (fd != -1)
			close(fd);
		sprintf(file,"/proc/bus/pci/%02x/%02x.%1x",bus,dev,func);
		fd = open(file,O_RDWR);
		lbus  = bus;
		ldev  = dev;
		lfunc = func;
	}
	return fd;
}

CARD32
linuxPciCfgRead(PCITAG tag, int off)
{
	int	fd;
	CARD32	val = 0xffffffff;

	if (-1 != (fd = linuxPciOpenFile(tag))) {
		lseek(fd,off,SEEK_SET);
		read(fd,&val,4);
	}
	return PCI_CPU(val);
}

void
linuxPciCfgWrite(PCITAG tag, int off, CARD32 val)
{
	int	fd;

	if (-1 != (fd = linuxPciOpenFile(tag))) {
		lseek(fd,off,SEEK_SET);
		val = PCI_CPU(val);
		write(fd,&val,4);
	}
}

void
linuxPciCfgSetBits(PCITAG tag, int off, CARD32 mask, CARD32 bits)
{
	int	fd;
	CARD32	val = 0xffffffff;

	return; 
	if (-1 != (fd = linuxPciOpenFile(tag))) {
		lseek(fd,off,SEEK_SET);
		read(fd,&val,4);
		val = PCI_CPU(val);
		val = (val & ~mask) | (bits & mask);
		val = PCI_CPU(val);
		lseek(fd,off,SEEK_SET);
		write(fd,&val,4);
	}
}

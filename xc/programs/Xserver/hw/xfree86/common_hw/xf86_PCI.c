/* $XFree86: xc/programs/Xserver/hw/xfree86/common_hw/xf86_PCI.c,v 3.14 1996/10/17 15:18:36 dawes Exp $ */
/*
 * Copyright 1995 by Robin Cutshaw <robin@XFree86.Org>
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the names of the above listed copyright holder(s)
 * not be used in advertising or publicity pertaining to distribution of 
 * the software without specific, written prior permission.  The above listed
 * copyright holder(s) make(s) no representations about the suitability of this 
 * software for any purpose.  It is provided "as is" without express or 
 * implied warranty.
 *
 * THE ABOVE LISTED COPYRIGHT HOLDER(S) DISCLAIM(S) ALL WARRANTIES WITH REGARD 
 * TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY 
 * AND FITNESS, IN NO EVENT SHALL THE ABOVE LISTED COPYRIGHT HOLDER(S) BE 
 * LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY 
 * DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER 
 * IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING 
 * OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 */
/* $XConsortium: xf86_PCI.c /main/5 1996/01/10 10:21:13 kaleb $ */

/*#define DEBUGPCI  1 */

#include <stdio.h>
#include "os.h"
#include "compiler.h"
#include "input.h"
#include "xf86Procs.h"
#include "xf86_OSlib.h"
#include "xf86_PCI.h"

#ifdef PC98
#define outb(port,data) _outb(port,data)
#define outl(port,data) _outl(port,data)
#define inb(port) _inb(port)
#define inl(port) _inl(port)
#endif

static pciConfigPtr pci_devp[MAX_PCI_DEVICES + 1] = {NULL, };

#ifdef USE_OLD_PCI_CODE
pciConfigPtr *
xf86scanpci(scrnIndex)
int scrnIndex;
{
    unsigned long tmplong1, tmplong2, config_cmd;
    unsigned char tmp1, tmp2;
    unsigned int i, j, idx = 0;
    pciConfigRec pcr;
    unsigned PCI_CtrlIOPorts[] = { PCI_MODE1_ADDRESS_REG,
				   PCI_MODE2_FORWARD_REG,
				   PCI_MODE1_DATA_REG };
    int Num_PCI_CtrlIOPorts = 3;
    unsigned PCI_DevIOAddrPorts[16*16];
    int Num_PCI_DevIOAddrPorts = 16*16;

    if (pci_devp[0])
	return (pciConfigPtr *)NULL;

    for (i=0; i<16; i++)
        for (j=0; j<16; j++)
            PCI_DevIOAddrPorts[(i*16)+j] = 0xC000 + (i*0x0100) + (j*4);

    xf86ClearIOPortList(scrnIndex);
    xf86AddIOPorts(scrnIndex, Num_PCI_CtrlIOPorts, PCI_CtrlIOPorts);
    xf86AddIOPorts(scrnIndex, Num_PCI_DevIOAddrPorts, PCI_DevIOAddrPorts);

    /* Enable I/O access */
    xf86EnableIOPorts(scrnIndex);

    outb(PCI_MODE2_ENABLE_REG, 0x00);
    outb(PCI_MODE2_FORWARD_REG, 0x00);
    tmp1 = inb(PCI_MODE2_ENABLE_REG);
    tmp2 = inb(PCI_MODE2_FORWARD_REG);

    /* 
     *
     * Check the configuration type 
     *
     */
    if ((tmp1 == 0x00) && (tmp2 == 0x00)) {
	pcr._configtype = 2;
#ifdef DEBUGPCI
        printf("PCI says configuration type 2\n");
#endif
    } else {
        tmplong1 = inl(PCI_MODE1_ADDRESS_REG);	/* save previous data */
        outl(PCI_MODE1_ADDRESS_REG, PCI_EN);
        tmplong2 = inl(PCI_MODE1_ADDRESS_REG);
        outl(PCI_MODE1_ADDRESS_REG, tmplong1);  
#if 0
        if (tmplong2 == PCI_EN) {
#else
        if (tmplong2 & PCI_EN) {
#endif
	    pcr._configtype = 1;
#ifdef DEBUGPCI
            printf("PCI says configuration type 1\n");
#endif
	} else {
	    pcr._configtype = 0;
#ifdef DEBUGPCI
            printf("No PCI !\n");
#endif
            xf86DisableIOPorts(scrnIndex);
            xf86ClearIOPortList(scrnIndex);
	    return (pciConfigPtr *)NULL;
	}
    }

    /* Try pci config 1 probe first */

#ifdef DEBUGPCI
    printf("\nPCI probing configuration type 1\n");
#endif

    pcr._pcibuses[0] = 0;
    pcr._pcinumbus = 1;
    pcr._pcibusidx = 0;

#ifndef DEBUGPCI
    if (pcr._configtype == 1)
#endif
    do {
        for (pcr._cardnum = 0x0; pcr._cardnum < PCI_CONFIG1_MAXDEV; 
			pcr._cardnum++) {
	  int func, maxfunc=0;
	  unsigned long config_cmd1;
	  CARD32	device_vendor1, base1;

	  pcr._bus = pcr._pcibuses[pcr._pcibusidx];
	  for (func = 0; func <= maxfunc; func++) {
	    config_cmd = PCI_EN | (pcr._pcibuses[pcr._pcibusidx] << 16ul) 
				| (pcr._cardnum << 11ul) 
				| (func << 8ul);

            outl(PCI_MODE1_ADDRESS_REG, config_cmd | PCI_ID_REG); /* ioreg 0 */
            pcr._device_vendor = inl(PCI_MODE1_DATA_REG);

            if (pcr._vendor == 0xFFFF)   /* nothing there */
                continue;

#ifdef DEBUGPCI
	    printf("\npci bus 0x%x cardnum 0x%02x, vendor 0x%04x device 0x%04x\n",
	        pcr._pcibuses[pcr._pcibusidx], pcr._cardnum, pcr._vendor,
                pcr._device);
#endif

            outl(PCI_MODE1_ADDRESS_REG, config_cmd | PCI_CMD_STAT_REG);
	    pcr._status_command  = inl(PCI_MODE1_DATA_REG);
            outl(PCI_MODE1_ADDRESS_REG, config_cmd | PCI_CLASS_REG);
	    pcr._class_revision  = inl(PCI_MODE1_DATA_REG);
            outl(PCI_MODE1_ADDRESS_REG, config_cmd | PCI_HEADER_MISC);
	    pcr._bist_header_latency_cache = inl(PCI_MODE1_DATA_REG);
            outl(PCI_MODE1_ADDRESS_REG, config_cmd | PCI_MAP_REG_START);
	    pcr._base0  = inl(PCI_MODE1_DATA_REG);
            outl(PCI_MODE1_ADDRESS_REG, config_cmd | (PCI_MAP_REG_START+0x04));
	    pcr._base1  = inl(PCI_MODE1_DATA_REG);
            outl(PCI_MODE1_ADDRESS_REG, config_cmd | (PCI_MAP_REG_START+0x08));
	    pcr._base2  = inl(PCI_MODE1_DATA_REG);
            outl(PCI_MODE1_ADDRESS_REG, config_cmd | (PCI_MAP_REG_START+0x0C));
	    pcr._base3  = inl(PCI_MODE1_DATA_REG);
            outl(PCI_MODE1_ADDRESS_REG, config_cmd | (PCI_MAP_REG_START+0x10));
	    pcr._base4  = inl(PCI_MODE1_DATA_REG);
            outl(PCI_MODE1_ADDRESS_REG, config_cmd | (PCI_MAP_REG_START+0x14));
	    pcr._base5  = inl(PCI_MODE1_DATA_REG);
            outl(PCI_MODE1_ADDRESS_REG, config_cmd | 0x30);
	    pcr._baserom = inl(PCI_MODE1_DATA_REG);
            outl(PCI_MODE1_ADDRESS_REG, config_cmd | PCI_INTERRUPT_REG);
	    pcr._max_min_ipin_iline = inl(PCI_MODE1_DATA_REG);
            outl(PCI_MODE1_ADDRESS_REG, config_cmd | PCI_REG_USERCONFIG);
	    pcr._user_config = inl(PCI_MODE1_DATA_REG);
	    pcr._func = func;

	    /* Check Multi-function device */
	    if (pcr._bist_header_latency_cache & PCI_HEADER_MULTIFUNCTION)
		maxfunc = 7;
	    if (func == 0) {
               config_cmd1 = PCI_EN | (pcr._pcibuses[pcr._pcibusidx] << 16ul)
                                | (pcr._cardnum << 11ul)
                                | (1<< 8ul);

               outl(PCI_MODE1_ADDRESS_REG, config_cmd1|PCI_ID_REG); 
               device_vendor1 = inl(PCI_MODE1_DATA_REG);

               if (device_vendor1 != 0xFFFFFFFF) {  /* nothing there */

	          if (pcr._device_vendor != device_vendor1)
		     maxfunc = 7;

                  outl(PCI_MODE1_ADDRESS_REG, config_cmd1|PCI_MAP_REG_START); 
                  base1 = inl(PCI_MODE1_DATA_REG);
	          if (pcr._base0 != base1)
		     maxfunc = 7;
	       }
            }

            /* check for pci-pci bridges */
            switch(pcr._class_revision & (PCI_CLASS_MASK|PCI_SUBCLASS_MASK)) {
                case PCI_CLASS_BRIDGE|PCI_SUBCLASS_BRIDGE_PCI:
                    if (pcr._secondary_bus_number > 0) {
                        pcr._pcibuses[pcr._pcinumbus++] = pcr._secondary_bus_number;
                    }
                        break;
                default:
                        break;
            }

	    if (idx >= MAX_PCI_DEVICES)
	        continue;

	    if ((pci_devp[idx] = (pciConfigPtr)xalloc(sizeof(pciConfigRec)))
		== (pciConfigPtr)NULL) {
                outl(PCI_MODE1_ADDRESS_REG, 0x00);
                xf86DisableIOPorts(scrnIndex);
                xf86ClearIOPortList(scrnIndex);
		return (pciConfigPtr *)NULL;
	    }

	    memcpy(pci_devp[idx++], &pcr, sizeof(pciConfigRec));
	    pci_devp[idx] = NULL;
          }
        }
    } while (++pcr._pcibusidx < pcr._pcinumbus);

#ifndef DEBUGPCI
    if (pcr._configtype == 1) {
        outl(PCI_MODE1_ADDRESS_REG, 0x00);
	xf86DisableIOPorts(scrnIndex);
	xf86ClearIOPortList(scrnIndex);
	return pci_devp;
    }
#endif
    /* Now try pci config 2 probe (deprecated) */

    /* The code here doesn't handle multifunction devices. */
    /* It doesn't seem to handle multiple busses fully either */
    outb(PCI_MODE2_ENABLE_REG, 0xF1);
    outb(PCI_MODE2_FORWARD_REG, 0x00); /* bus 0 for now */

#ifdef DEBUGPCI
    printf("\nPCI probing configuration type 2\n");
#endif

    pcr._pcibuses[0] = 0;
    pcr._pcinumbus = 1;
    pcr._pcibusidx = 0;

    do {
	pcr._bus = pcr._pcibuses[pcr._pcibusidx];
        for (pcr._ioaddr = 0xC000; pcr._ioaddr < 0xD000; pcr._ioaddr += 0x0100){
	    outb(PCI_MODE2_FORWARD_REG, pcr._pcibuses[pcr._pcibusidx]); /* bus 0 for now */
            pcr._device_vendor = inl(pcr._ioaddr);
	    outb(PCI_MODE2_FORWARD_REG, 0x00); /* bus 0 for now */
	    pcr._cardnum = (pcr._ioaddr >> 8) & 0x0f;

            if (pcr._vendor == 0xFFFF)   /* nothing there */
                continue;
	    /* opti chipsets that use config type 1 look like this on type 2 */
            if ((pcr._vendor == 0xFF00) && (pcr._device == 0xFFFF))
                continue;

#ifdef DEBUGPCI
	    printf("\npci bus 0x%x slot at 0x%04x, vendor 0x%04x device 0x%04x\n",
	        pcr._pcibuses[pcr._pcibusidx], pcr._ioaddr, pcr._vendor,
                pcr._device);
#endif

	    outb(PCI_MODE2_FORWARD_REG, pcr._pcibuses[pcr._pcibusidx]); /* bus 0 for now */
            pcr._status_command = inl(pcr._ioaddr + 0x04);
            pcr._class_revision = inl(pcr._ioaddr + 0x08);
            pcr._bist_header_latency_cache = inl(pcr._ioaddr + 0x0C);
            pcr._base0 = inl(pcr._ioaddr + 0x10);
            pcr._base1 = inl(pcr._ioaddr + 0x14);
            pcr._base2 = inl(pcr._ioaddr + 0x18);
            pcr._base3 = inl(pcr._ioaddr + 0x1C);
            pcr._base4 = inl(pcr._ioaddr + 0x20);
            pcr._base5 = inl(pcr._ioaddr + 0x24);
            pcr._baserom = inl(pcr._ioaddr + 0x30);
            pcr._max_min_ipin_iline = inl(pcr._ioaddr + 0x3C);
            pcr._user_config = inl(pcr._ioaddr + 0x40);
	    outb(PCI_MODE2_FORWARD_REG, 0x00); /* bus 0 for now */

            /* check for pci-pci bridges */
            switch(pcr._class_revision & (PCI_CLASS_MASK|PCI_SUBCLASS_MASK)) {
                case PCI_CLASS_BRIDGE|PCI_SUBCLASS_BRIDGE_PCI:
                    if (pcr._secondary_bus_number > 0) {
                        pcr._pcibuses[pcr._pcinumbus++] = pcr._secondary_bus_number;            
                    }       
                        break;
                default:
                        break;
            }


	    if (idx >= MAX_PCI_DEVICES)
	        continue;

	    if ((pci_devp[idx] = (pciConfigPtr)xalloc(sizeof(pciConfigRec)))
		== (struct pci_config_reg *)NULL) {
                outb(PCI_MODE2_ENABLE_REG, 0x00);
                outb(PCI_MODE2_FORWARD_REG, 0x00);
                xf86DisableIOPorts(scrnIndex);
                xf86ClearIOPortList(scrnIndex);
		return (pciConfigPtr *)NULL;
	    }

	    memcpy(pci_devp[idx++], &pcr, sizeof(pciConfigRec));
	    pci_devp[idx] = NULL;
	}
    } while (++pcr._pcibusidx < pcr._pcinumbus);

    outb(PCI_MODE2_ENABLE_REG, 0x00);
    outb(PCI_MODE2_FORWARD_REG, 0x00);

    xf86DisableIOPorts(scrnIndex);
    xf86ClearIOPortList(scrnIndex);
    return pci_devp;
}

void
xf86writepci(scrnIndex, bus, cardnum, func, reg, mask, value)
    int scrnIndex;
    int bus;
    int cardnum;
    int func;
    int reg;
    unsigned long mask;
    unsigned long value;
{
    unsigned char tmp1, tmp2;
    unsigned long tmplong1, tmplong2, tmp, config_cmd;
    unsigned int i, j;
    unsigned PCI_CtrlIOPorts[] = { PCI_MODE1_ADDRESS_REG,
				   PCI_MODE2_FORWARD_REG,
				   PCI_MODE1_DATA_REG };
    int configtype;
    int Num_PCI_CtrlIOPorts = 3;
    unsigned PCI_DevIOAddrPorts[16*16];
    int Num_PCI_DevIOAddrPorts = 16*16;

    for (i=0; i<16; i++)
        for (j=0; j<16; j++)
            PCI_DevIOAddrPorts[(i*16)+j] = 0xC000 + (i*0x0100) + (j*4);

    xf86ClearIOPortList(scrnIndex);
    xf86AddIOPorts(scrnIndex, Num_PCI_CtrlIOPorts, PCI_CtrlIOPorts);
    xf86AddIOPorts(scrnIndex, Num_PCI_DevIOAddrPorts, PCI_DevIOAddrPorts);

    /* Enable I/O access */
    xf86EnableIOPorts(scrnIndex);

    outb(PCI_MODE2_ENABLE_REG, 0x00);
    outb(PCI_MODE2_FORWARD_REG, 0x00);
    tmp1 = inb(PCI_MODE2_ENABLE_REG);
    tmp2 = inb(PCI_MODE2_FORWARD_REG);
    if ((tmp1 == 0x00) && (tmp2 == 0x00)) {
	configtype = 2;
#ifdef DEBUGPCI
        printf("PCI says configuration type 2\n");
#endif
    } else {
        tmplong1 = inl(PCI_MODE1_ADDRESS_REG);
        outl(PCI_MODE1_ADDRESS_REG, PCI_EN);
        tmplong2 = inl(PCI_MODE1_ADDRESS_REG);
        outl(PCI_MODE1_ADDRESS_REG, tmplong1);
        if (tmplong2 & PCI_EN) {
	    configtype = 1;
#ifdef DEBUGPCI
            printf("PCI says configuration type 1\n");
#endif
	} else {
	    configtype = 0;
#ifdef DEBUGPCI
            printf("No PCI !\n");
#endif
            xf86DisableIOPorts(scrnIndex);
            xf86ClearIOPortList(scrnIndex);
	    return;
	}
    }

    if (configtype == 1)
    {
	config_cmd = PCI_EN | (bus << 16) | (cardnum << 11) | (func << 8);

        outl(PCI_MODE1_ADDRESS_REG, config_cmd | reg);
	tmp = inl(PCI_MODE1_DATA_REG) & ~mask;
	outl(PCI_MODE1_DATA_REG, tmp | (value & mask));
        outl(PCI_MODE1_ADDRESS_REG, 0x00);
	xf86DisableIOPorts(scrnIndex);
	xf86ClearIOPortList(scrnIndex);
	return;
    } else {
	/* Now try pci config 2 probe (deprecated) */
	/* The code here doesn't handle multifunction devices. */
    	/* it doesn't seem to handle multiple busses fully either */

	int ioaddr = 0xC000 + (cardnum * 0x100);

	outb(PCI_MODE2_ENABLE_REG, 0xF1);
	outb(PCI_MODE2_FORWARD_REG, 0x00); /* bus 0 for now */

	outb(PCI_MODE2_FORWARD_REG, 0x00); /* bus 0 for now */
        tmp = inl(ioaddr + reg) & ~mask;
	outb(PCI_MODE2_FORWARD_REG, 0x00); /* bus 0 for now */
        outl(ioaddr + reg, tmp | (value & mask));
	outb(PCI_MODE2_FORWARD_REG, 0x00); /* bus 0 for now */

	outb(PCI_MODE2_ENABLE_REG, 0x00);
	outb(PCI_MODE2_FORWARD_REG, 0x00);

	xf86DisableIOPorts(scrnIndex);
	xf86ClearIOPortList(scrnIndex);
	return;
    }
}


void
xf86cleanpci()
{
    int idx = 0;

    while (pci_devp[idx])
	xfree((Pointer)pci_devp[idx++]);

    pci_devp[0] = (pciConfigPtr)NULL;
}

#else

/* New PCI code */

/*
 * This is based heavily on the code in FreeBSD-current, which was written
 * by Wolfgang Stanglmeier, and contains the following copyright:
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */


static int pciConfigType = 0;
static int pciMaxDevice = 0;

static Bool
pcibusCheck()
{
    CARD8 device;

    for (device = 0; device < pciMaxDevice; device++) {
	CARD32 id;
	id = pcibusRead(pcibusTag(0, device, 0), 0);
	if (id && id != 0xffffffff) {
	    return TRUE;
	}
    }
    return 0;
}

static void
pcibusSetup()
{
    static Bool setupDone = FALSE;
    CARD32 mode1Res, oldVal1;
    CARD8  mode2Res, oldVal2;

    if (setupDone)
	return;

    setupDone = TRUE;

    oldVal1 = inl(PCI_MODE1_ADDRESS_REG);

    /* Assuming config type 1 to start with */
    if ((oldVal1 & 0x7ff00000) == 0) {
	pciConfigType = 1;
	pciMaxDevice = PCI_CONFIG1_MAXDEV;

	outl(PCI_MODE1_ADDRESS_REG, PCI_EN);
	outb(PCI_MODE1_ADDRESS_REG + 3, 0);
	mode1Res = inl(PCI_MODE1_ADDRESS_REG);
	outl(PCI_MODE1_ADDRESS_REG, oldVal1);

	if (mode1Res) {
	    if (pcibusCheck()) {
		if (xf86Verbose > 1) {
		    ErrorF("PCI: Config type is 1\n");
		}
		return;
	    }
	}

	outl(PCI_MODE1_ADDRESS_REG, 0xff000001);
	mode1Res = inl(PCI_MODE1_ADDRESS_REG);
	outl(PCI_MODE1_ADDRESS_REG, oldVal1);

	if ((mode1Res & 0x80000001) == 0x80000000) {
	    if (pcibusCheck()) {
		if (xf86Verbose > 1) {
		    ErrorF("PCI: Config type is 1\n");
		}
		return;
	    }
	}
    }

    /* Try config type 2 */
    oldVal2 = inb(PCI_MODE2_ENABLE_REG);
    if ((oldVal2 & 0xf0) == 0) {
	pciConfigType = 2;
	pciMaxDevice = PCI_CONFIG2_MAXDEV;

	outb(PCI_MODE2_ENABLE_REG, 0x0e);
	mode2Res = inb(PCI_MODE2_ENABLE_REG);
	outb(PCI_MODE2_ENABLE_REG, oldVal2);

	if (mode2Res == 0x0e) {
	    if (pcibusCheck()) {
		if (xf86Verbose > 1) {
		    ErrorF("PCI: Config type is 2\n");
		}
		return;
	    }
	}
    }

    /* No PCI found */

    pciConfigType = 0;
    pciMaxDevice = 0;
    if (xf86Verbose > 1) {
	ErrorF("PCI: No PCI bus found\n");
    }
}

pciTagRec
pcibusTag(CARD8 bus, CARD8 cardnum, CARD8 func)
{
    pciTagRec tag;

    tag.cfg1 = 0;

    if (func > 7)
	return tag;

    switch (pciConfigType) {
    case 1:
	if (cardnum < PCI_CONFIG1_MAXDEV) {
	    tag.cfg1 = PCI_EN
			| ((CARD32)bus << 16)
			| ((CARD32)cardnum << 11)
			| ((CARD32)func << 8);
	}
	break;
    case 2:
	if (cardnum < PCI_CONFIG2_MAXDEV) {
	    tag.cfg2.port    = 0xc000 | ((CARD16)cardnum << 8);
	    tag.cfg2.enable  = 0xf0 | (func << 1);
	    tag.cfg2.forward = bus;
	}
	break;
    }
    return tag;
}

static pciTagRec
pcibusFTag(pciTagRec tag, CARD8 func)
{
    if (func > 7) {
	return tag;
    }

    switch (pciConfigType) {
    case 1:
	tag.cfg1 &= ~0x700UL;
	tag.cfg1 |= ((CARD32)func << 8);
	break;
    case 2:
	tag.cfg2.enable = 0xf0 | (func << 1);
	break;
    }
    return tag;
}

CARD32
pcibusRead(pciTagRec tag, CARD32 reg)
{
    CARD32 addr, data = 0;

    if (!tag.cfg1) {
	return 0xffffffff;
    }

    switch (pciConfigType) {
    case 1:
	addr = tag.cfg1 | (reg & 0xfc);
	outl(PCI_MODE1_ADDRESS_REG, addr);
	data = inl(PCI_MODE1_DATA_REG);
	outl(PCI_MODE1_ADDRESS_REG, 0);
	break;
    case 2:
	addr = tag.cfg2.port | (reg & 0xfc);
	outb(PCI_MODE2_ENABLE_REG, tag.cfg2.enable);
	outb(PCI_MODE2_FORWARD_REG, tag.cfg2.forward);
	data = inl((CARD16)addr);
	outb(PCI_MODE2_ENABLE_REG, 0);
	outb(PCI_MODE2_FORWARD_REG, 0);
	break;
    }
    return data;
}

CARD16
pciReadWord(pciTagRec tag, CARD32 reg)
{
    CARD32 addr;
    CARD16 data = 0;

    if (!tag.cfg1) {
	return 0xff;
    }

    switch (pciConfigType) {
    case 1:
	addr = tag.cfg1 | (reg & 0xfc);
	outl(PCI_MODE1_ADDRESS_REG, addr);
	data = inw(PCI_MODE1_DATA_REG);
	outl(PCI_MODE1_ADDRESS_REG, 0);
	break;
    case 2:
	addr = tag.cfg2.port | (reg & 0xfc);
	outb(PCI_MODE2_ENABLE_REG, tag.cfg2.enable);
	outb(PCI_MODE2_FORWARD_REG, tag.cfg2.forward);
	data = inw((CARD16)addr);
	outb(PCI_MODE2_ENABLE_REG, 0);
	outb(PCI_MODE2_FORWARD_REG, 0);
	break;
    }
    return data;
}

CARD8
pciReadByte(pciTagRec tag, CARD32 reg)
{
    CARD32 addr;
    CARD8 data = 0;

    if (!tag.cfg1) {
	return 0xff;
    }

    switch (pciConfigType) {
    case 1:
	addr = tag.cfg1 | (reg & 0xfc);
	outl(PCI_MODE1_ADDRESS_REG, addr);
	data = inb(PCI_MODE1_DATA_REG);
	outl(PCI_MODE1_ADDRESS_REG, 0);
	break;
    case 2:
	addr = tag.cfg2.port | (reg & 0xfc);
	outb(PCI_MODE2_ENABLE_REG, tag.cfg2.enable);
	outb(PCI_MODE2_FORWARD_REG, tag.cfg2.forward);
	data = inb((CARD16)addr);
	outb(PCI_MODE2_ENABLE_REG, 0);
	outb(PCI_MODE2_FORWARD_REG, 0);
	break;
    }
    return data;
}

void
pcibusWrite(pciTagRec tag, CARD32 reg, CARD32 data)
{
    CARD32 addr;

    if (!tag.cfg1) {
	return;
    }

    switch (pciConfigType) {
    case 1:
	addr = tag.cfg1 | (reg & 0xfc);
	outl(PCI_MODE1_ADDRESS_REG, addr);
	outl(PCI_MODE1_DATA_REG, data);
	outl(PCI_MODE1_ADDRESS_REG, 0);
	break;
    case 2:
	addr = tag.cfg2.port | (reg & 0xfc);
	outb(PCI_MODE2_ENABLE_REG, tag.cfg2.enable);
	outb(PCI_MODE2_FORWARD_REG, tag.cfg2.forward);
	outl((CARD16)addr, data);
	outb(PCI_MODE2_ENABLE_REG, 0);
	outb(PCI_MODE2_FORWARD_REG, 0);
	break;
    }
}

void
pciWriteWord(pciTagRec tag, CARD32 reg, CARD16 data)
{
    CARD32 addr;

    if (!tag.cfg1) {
	return;
    }

    switch (pciConfigType) {
    case 1:
	addr = tag.cfg1 | (reg & 0xfc);
	outl(PCI_MODE1_ADDRESS_REG, addr);
	outw(PCI_MODE1_DATA_REG, data);
	outl(PCI_MODE1_ADDRESS_REG, 0);
	break;
    case 2:
	addr = tag.cfg2.port | (reg & 0xfc);
	outb(PCI_MODE2_ENABLE_REG, tag.cfg2.enable);
	outb(PCI_MODE2_FORWARD_REG, tag.cfg2.forward);
	outw((CARD16)addr, data);
	outb(PCI_MODE2_ENABLE_REG, 0);
	outb(PCI_MODE2_FORWARD_REG, 0);
	break;
    }
}

void
pciWriteByte(pciTagRec tag, CARD32 reg, CARD8 data)
{
    CARD32 addr;

    if (!tag.cfg1) {
	return;
    }

    switch (pciConfigType) {
    case 1:
	addr = tag.cfg1 | (reg & 0xfc);
	outl(PCI_MODE1_ADDRESS_REG, addr);
	outb(PCI_MODE1_DATA_REG, data);
	outl(PCI_MODE1_ADDRESS_REG, 0);
	break;
    case 2:
	addr = tag.cfg2.port | (reg & 0xfc);
	outb(PCI_MODE2_ENABLE_REG, tag.cfg2.enable);
	outb(PCI_MODE2_FORWARD_REG, tag.cfg2.forward);
	outb((CARD16)addr, data);
	outb(PCI_MODE2_ENABLE_REG, 0);
	outb(PCI_MODE2_FORWARD_REG, 0);
	break;
    }
}

static void
pciEnableIO(int scrnIndex)
{
    /* This is enough to ensure that full I/O is enabled */
    unsigned pciIOPorts[] = { PCI_MODE1_ADDRESS_REG };
    int numPciIOPorts = sizeof(pciIOPorts) / sizeof(pciIOPorts[0]);

    xf86ClearIOPortList(scrnIndex);
    xf86AddIOPorts(scrnIndex, numPciIOPorts, pciIOPorts);
    xf86EnableIOPorts(scrnIndex);
}

static void
pciDisableIO(int scrnIndex)
{
    xf86DisableIOPorts(scrnIndex);
    xf86ClearIOPortList(scrnIndex);
}

static Bool
pciMfDev(CARD8 bus, CARD8 cardnum)
{
    pciTagRec tag0, tag1;
    CARD32 id0, id1;

    /* Detect a multi-function device that complies to the PCI 2.0 spec */

    tag0 = pcibusTag(bus, cardnum, 0);
    if (pcibusRead(tag0, PCI_HEADER_MISC) & PCI_HEADER_MULTIFUNCTION)
	return TRUE;

    /*
     * Now, to find non-compliant devices...
     * If there is a valid ID for function 1 and the ID for func 0 and 1
     * are different, or the base0 values of func 0 and 1 are differend,
     * then assume there is a multi-function device.
     */
    tag1 = pcibusTag(bus, cardnum, 1);
    id1 = pcibusRead(tag1, PCI_ID_REG);
    if (id1 != 0xffffffff) {
	id0 = pcibusRead(tag0, PCI_ID_REG);
	if (id0 != id1)
	    return TRUE;
	if (pcibusRead(tag0, PCI_MAP_REG_START)
	    != pcibusRead(tag1, PCI_MAP_REG_START))
	    return TRUE;
    }
    return FALSE;
}

pciConfigPtr *
xf86scanpci(int scrnIndex)
{
    pciConfigRec pcr;
    int pcibusidx, pcinumbus, pcibuses[16];
    int idx = 0;

    if (pci_devp[0])
	return pci_devp;

    pciEnableIO(scrnIndex);

    /* Check for a PCI bus, and find the config type */
    pcibusSetup();

    if (pciConfigType == 0)
	return (pciConfigPtr *)NULL;

    pcibusidx = 0;
    pcibuses[0] = 0;
    pcinumbus = 1;

    do {
	for (pcr._cardnum = 0; pcr._cardnum < pciMaxDevice; pcr._cardnum++) {
	    int maxfunc = 0;
	    pciTagRec tag;

	    pcr._bus = pcibuses[pcibusidx];

	    if (pciMfDev(pcr._bus, pcr._cardnum))
		maxfunc = 7;

	    tag = pcibusTag(pcr._bus, pcr._cardnum, 0);
	    for (pcr._func = 0; pcr._func <= maxfunc; pcr._func++) {
		tag = pcibusFTag(tag, pcr._func);
		pcr._device_vendor = pcibusRead(tag, PCI_ID_REG);
		if (pcr._device_vendor == 0xffffffff)	/* nothing there */
		    continue;

		pcr._status_command = pcibusRead(tag, PCI_CMD_STAT_REG);
		pcr._class_revision = pcibusRead(tag, PCI_CLASS_REG);
		pcr._bist_header_latency_cache =
			pcibusRead(tag, PCI_HEADER_MISC);
		pcr._base0 = pcibusRead(tag, PCI_MAP_REG_START);
		pcr._base1 = pcibusRead(tag, PCI_MAP_REG_START + 0x04);
		pcr._base2 = pcibusRead(tag, PCI_MAP_REG_START + 0x08);
		pcr._base3 = pcibusRead(tag, PCI_MAP_REG_START + 0x0c);
		pcr._base4 = pcibusRead(tag, PCI_MAP_REG_START + 0x10);
		pcr._base5 = pcibusRead(tag, PCI_MAP_REG_START + 0x14);
		pcr._baserom = pcibusRead(tag, PCI_MAP_ROM_REG);
		pcr._max_min_ipin_iline = pcibusRead(tag, PCI_INTERRUPT_REG);
		pcr._user_config = pcibusRead(tag, PCI_REG_USERCONFIG);

		/* Check for PCI-PCI bridges */
		if (pcr._base_class == PCI_CLASS_BRIDGE &&
		    pcr._sub_class == PCI_SUBCLASS_BRIDGE_PCI) {
		    if (pcr._secondary_bus_number > 0) {
			pcibuses[pcinumbus++] = pcr._secondary_bus_number;
		    }
		}

		if (idx >= MAX_PCI_DEVICES)
		    continue;

		if ((pci_devp[idx] = (pciConfigPtr)xalloc(sizeof(pciConfigRec)))
		    == (pciConfigPtr)NULL) {
		    pciDisableIO(scrnIndex);
		    return (pciConfigPtr *)NULL;
		}

		memcpy(pci_devp[idx++], &pcr, sizeof(pciConfigRec));
		pci_devp[idx] = (pciConfigPtr)NULL;
		if (xf86Verbose > 1) {
		    ErrorF("PCI: Bus 0x%x Card 0x%02x Func 0x%x ID 0x%04x,"
			   "0x%04x Rev 0x%02x Class 0x%02x,0x%02x\n",
			   pcr._bus, pcr._cardnum, pcr._func, pcr._vendor,
			   pcr._device, pcr._rev_id, pcr._base_class,
			   pcr._sub_class);
		}
	    }
	}
    } while (++pcibusidx < pcinumbus);
    pciDisableIO(scrnIndex);
    return pci_devp;
}

void
xf86writepci(int scrnIndex, int bus, int cardnum, int func, int reg,
	     CARD32 mask, CARD32 value)
{
    pciTagRec tag;
    CARD32 data;

    pciEnableIO(scrnIndex);

    /* Check for a PCI bus, and find the config type */
    pcibusSetup();

    if (pciConfigType == 0)
	return;

    tag = pcibusTag(bus, cardnum, func);
    data = pcibusRead(tag, reg) & ~mask | (value & mask);
    pcibusWrite(tag, reg, data);

    if (xf86Verbose > 2) {
	ErrorF("PCI: xf86writepci: Bus=0x%x Card=0x%x Func=0x%x Reg=0x%02x "
	       "Mask=0x%08x Val=0x%08x\n",
	       bus, cardnum, func, reg, mask, value);
    }

    pciDisableIO(scrnIndex);
}

void
xf86cleanpci()
{
    int idx = 0;

    while (pci_devp[idx])
	xfree((pointer)pci_devp[idx++]);

    pci_devp[0] = (pciConfigPtr)NULL;
}

#endif

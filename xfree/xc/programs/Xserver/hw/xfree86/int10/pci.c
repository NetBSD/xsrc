/* $XFree86: xc/programs/Xserver/hw/xfree86/int10/pci.c,v 1.8.2.2 2001/05/25 18:15:46 eich Exp $ */

/*
 *                   XFree86 int10 module
 *   execute BIOS int 10h calls in x86 real mode environment
 *                 Copyright 1999 Egbert Eich
 */
#include "xf86Pci.h"
#include "xf86.h"
#include "xf86_ansic.h"
#define _INT10_PRIVATE
#include "xf86int10.h"

int
mapPciRom(int pciEntity, unsigned char * address)
{
    PCITAG tag;
    unsigned char *mem, *ptr;
    int length;
    
    pciVideoPtr pvp = xf86GetPciInfoForEntity(pciEntity);

    if (pvp == NULL) {
#ifdef DEBUG
	ErrorF("mapPciRom: no PCI info\n");
#endif
	return 0;
    }

    tag = pciTag(pvp->bus,pvp->device,pvp->func);
    length = 1 << pvp->biosSize;

    /* Read in entire PCI ROM */
    mem = ptr = xnfcalloc(length, 1);
    if (xf86ReadPciBIOS(0, tag, -1, ptr, length) == 0) {
	xfree(mem);
#ifdef DEBUG
	ErrorF("mapPciRom: cannot read BIOS\n");
#endif
	return 0;
    }

    length = 0;
    while ((ptr[0] == 0x55) && (ptr[1] == 0xAA)) {
	unsigned short data_off = ptr[0x18] | (ptr[0x19] << 8);
	unsigned char *data = ptr + data_off;
	unsigned char type;

	if ((data[0] != 'P') ||
	    (data[1] != 'C') ||
	    (data[2] != 'I') ||
	    (data[3] != 'R'))
	    break;
	type = data[0x14];
#ifdef PRINT_PCI
	ErrorF("data segment in BIOS: 0x%x, type: 0x%x\n", data_off, type);
#endif
	if (type) {	/* not PC-AT image: find next one */
	    unsigned int image_length;
	    unsigned char indicator = data[0x15];
	    if (indicator & 0x80)	/* last image */
		break;
	    image_length = (data[0x10] | (data[0x11] << 8)) << 9;
#ifdef PRINT_PCI
	    ErrorF("data image length: 0x%x, ind: 0x%x\n",
		image_length, indicator);
#endif
	    ptr += image_length;
	    continue;
	 }
	 /* OK, we have a PC Image */
	 length = ptr[2] << 9;
#ifdef PRINT_PCI
	 ErrorF("BIOS length: 0x%x\n", length);
#endif
	 break;
     }

    if (length > 0)
	memcpy(address, ptr, length);
    /* unmap/close/disable PCI bios mem */
    xfree(mem);

#ifdef DEBUG
    if (!length)
	ErrorF("mapPciRom: no BIOS found\n");
#ifdef PRINT_PCI
    else
	dprint(address,0x20);
#endif
#endif

    return length;
}

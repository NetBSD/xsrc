/* $XFree86: xc/programs/Xserver/hw/xfree86/drivers/ati/atibios.c,v 1.7 2000/08/22 21:54:29 tsi Exp $ */
/*
 * Copyright 1999 through 2000 by Marc Aurele La France (TSI @ UQV), tsi@ualberta.ca
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that copyright
 * notice and this permission notice appear in supporting documentation, and
 * that the name of Marc Aurele La France not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  Marc Aurele La France makes no representations
 * about the suitability of this software for any purpose.  It is provided
 * "as-is" without express or implied warranty.
 *
 * MARC AURELE LA FRANCE DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.  IN NO
 * EVENT SHALL MARC AURELE LA FRANCE BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

#include "ati.h"
#include "atibios.h"
#include "atistruct.h"

#ifndef AVOID_CPIO

/*
 * ATIReadBIOS --
 *
 * This function is called to read in an adapter's BIOS, or parts thereof.
 */
int
ATIReadBIOS
(
    ATIPtr        pATI,
    pointer       Buffer,
    unsigned long Offset,
    int           Length
)
{
    pciVideoPtr  pVideo;
    pciConfigPtr pPCI;

    /*
     * Read PCI adapter expansion ROM, but only for adapters found to have been
     * disabled on server entry.
     */
    if ((pVideo = pATI->PCIInfo) &&
        (((pPCI = pVideo->thisCard)->pci_command &
          (PCI_CMD_IO_ENABLE | PCI_CMD_MEM_ENABLE)) !=
         (PCI_CMD_IO_ENABLE | PCI_CMD_MEM_ENABLE)))
    {
        pATI->BIOSBase = PCIGETROM(pciReadLong(pPCI->tag, PCI_MAP_ROM_REG));
        return xf86ReadPciBIOS(Offset, pPCI->tag, 0, Buffer, Length);
    }

    /* Read legacy ROM */
    if (!pATI->BIOSBase)
        pATI->BIOSBase = 0x000C0000U;
    return xf86ReadBIOS(pATI->BIOSBase, Offset, Buffer, Length);
}

#endif /* AVOID_CPIO */

/* $XFree86: xc/programs/Xserver/hw/xfree86/drivers/ati/atividmem.c,v 1.10 2000/08/22 21:54:32 tsi Exp $ */
/*
 * Copyright 1997 through 2000 by Marc Aurele La France (TSI @ UQV), tsi@ualberta.ca
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
#include "atiadapter.h"
#include "atistruct.h"
#include "atividmem.h"

/* Memory types for 68800's and 88800GX's */
const char *ATIMemoryTypeNames_Mach[] =
{
    "DRAM (256Kx4)",
    "VRAM (256Kx4, x8, x16)",
    "VRAM (256Kx16 with short shift register)",
    "DRAM (256Kx16)",
    "Graphics DRAM (256Kx16)",
    "Enhanced VRAM (256Kx4, x8, x16)",
    "Enhanced VRAM (256Kx16 with short shift register)",
    "Unknown video memory type"
};

/* Memory types for 88800CX's */
const char *ATIMemoryTypeNames_88800CX[] =
{
    "DRAM (256Kx4, x8, x16)",
    "EDO DRAM (256Kx4, x8, x16)",
    "Unknown video memory type",
    "DRAM (256Kx16 with assymetric RAS/CAS)",
    "Unknown video memory type",
    "Unknown video memory type",
    "Unknown video memory type",
    "Unknown video memory type"
};

/* Memory types for 264xT's */
const char *ATIMemoryTypeNames_264xT[] =
{
    "Disabled video memory",
    "DRAM",
    "EDO DRAM",
    "Pseudo-EDO DRAM",
    "SDRAM (1:1)",
    "SGRAM (1:1)",
    "SGRAM (2:1) 32-bit",
    "Unknown video memory type"
};

#ifndef AVOID_CPIO

/*
 * ATIUnmapVGA --
 *
 * Unmap VGA aperture.
 */
static void
ATIUnmapVGA
(
    int    iScreen,
    ATIPtr pATI
)
{
    if (!pATI->pBank)
        return;

    xf86UnMapVidMem(iScreen, pATI->pBank, 0x00010000U);

    pATI->pBank = pATI->BankInfo.pBankA = pATI->BankInfo.pBankB = NULL;
}

#endif /* AVOID_CPIO */

/*
 * ATIUnmapLinear --
 *
 * Unmap linear aperture.
 */
static void
ATIUnmapLinear
(
    int    iScreen,
    ATIPtr pATI
)
{
    unsigned long PageSize;
    int           LinearSize;

#ifdef AVOID_CPIO

    if (!pATI->pMemory)
        return;

#else /* AVOID_CPIO */

    if (pATI->pMemory != pATI->pBank)

#endif /* AVOID_CPIO */

    {
        PageSize = getpagesize();
        LinearSize = pATI->LinearSize;
        if (((pATI->Block0Base | (PageSize - 1)) + 1) ==
            (pATI->LinearBase + LinearSize))
            LinearSize -= PageSize;

        xf86UnMapVidMem(iScreen, pATI->pMemory, LinearSize);
    }

    pATI->pMemory = NULL;
}

/*
 * ATIUnmapMMIO --
 *
 * Unmap MMIO registers.
 */
static void
ATIUnmapMMIO
(
    int    iScreen,
    ATIPtr pATI
)
{
    if (pATI->pMMIO)
        xf86UnMapVidMem(iScreen, pATI->pMMIO, getpagesize());

    pATI->pMMIO = pATI->pBlock[0] = pATI->pBlock[1] = NULL;
}

/*
 * ATIMapApertures --
 *
 * This function maps all apertures used by the driver.
 */
Bool
ATIMapApertures
(
    int    iScreen,
    ATIPtr pATI
)
{
    pciVideoPtr   pVideo;
    PCITAG        Tag;
    unsigned long PageSize, MMIOBase;
    int           LinearSize;

    if (pATI->Mapped)
        return TRUE;

#ifndef AVOID_CPIO

    if (pATI->VGAAdapter == ATI_ADAPTER_NONE)

#endif /* AVOID_CPIO */

    {
        if (!pATI->LinearBase && !pATI->Block0Base)
            return FALSE;
    }

    PageSize = getpagesize();
    MMIOBase = pATI->Block0Base & ~(PageSize - 1);
    LinearSize = pATI->LinearSize;
    if ((MMIOBase + PageSize) == (pATI->LinearBase + LinearSize))
        LinearSize -= PageSize;

    if ((pVideo = pATI->PCIInfo))
        Tag = ((pciConfigPtr)(pVideo->thisCard))->tag;
    else
        Tag = 0;

#ifndef AVOID_CPIO

    /* Map VGA aperture */
    if (pATI->VGAAdapter != ATI_ADAPTER_NONE)
    {
        /*
         * No relocation, resizing, caching or write-combining of this
         * aperture is supported.  Hence, the hard-coded values here...
         */
        if (pVideo)
            pATI->pBank = xf86MapPciMem(iScreen, VIDMEM_MMIO,
                Tag, 0x000A0000U, 0x00010000U);
        else
            pATI->pBank = xf86MapVidMem(iScreen, VIDMEM_MMIO,
                0x000A0000U, 0x00010000U);

        if (!pATI->pBank)
            return FALSE;

        pATI->pMemory =
            pATI->BankInfo.pBankA =
            pATI->BankInfo.pBankB = pATI->pBank;

        pATI->Mapped = TRUE;
    }

#endif /* AVOID_CPIO */

    /* Map linear aperture */
    if (pATI->LinearBase)
    {
        if (pVideo)
            pATI->pMemory = xf86MapPciMem(iScreen, VIDMEM_FRAMEBUFFER,
                Tag, pATI->LinearBase, LinearSize);
        else
            pATI->pMemory = xf86MapVidMem(iScreen, VIDMEM_FRAMEBUFFER,
                pATI->LinearBase, LinearSize);

        if (!pATI->pMemory)
        {

#ifndef AVOID_CPIO

            ATIUnmapVGA(iScreen, pATI);

#endif /* AVOID_CPIO */

            pATI->Mapped = FALSE;
            return FALSE;
        }

        pATI->Mapped = TRUE;
    }

    /* Map MMIO aperture */
    if (pATI->Block0Base)
    {
        if (pVideo)
            pATI->pMMIO = xf86MapPciMem(iScreen, VIDMEM_MMIO,
                Tag, MMIOBase, PageSize);
        else
            pATI->pMMIO = xf86MapVidMem(iScreen, VIDMEM_MMIO,
                MMIOBase, PageSize);

        if (!pATI->pMMIO)
        {
            ATIUnmapLinear(iScreen, pATI);

#ifndef AVOID_CPIO

            ATIUnmapVGA(iScreen, pATI);

#endif /* AVOID_CPIO */

            pATI->Mapped = FALSE;
            return FALSE;
        }

        pATI->Mapped = TRUE;

        pATI->pBlock[0] = (char *)pATI->pMMIO +
            (pATI->Block0Base - MMIOBase);

        if (pATI->Block1Base)
            pATI->pBlock[1] = (char *)pATI->pBlock[0] - 0x00000400U;
    }

    return TRUE;
}

/*
 * ATIUnmapApertures --
 *
 * This function unmaps all apertures used by the driver.
 */
void
ATIUnmapApertures
(
    int    iScreen,
    ATIPtr pATI
)
{
    if (!pATI->Mapped)
        return;
    pATI->Mapped = FALSE;

    /* Unmap MMIO area */
    ATIUnmapMMIO(iScreen, pATI);

    /* Unmap linear aperture */
    ATIUnmapLinear(iScreen, pATI);

#ifndef AVOID_CPIO

    /* Unmap VGA aperture */
    ATIUnmapVGA(iScreen, pATI);

#endif /* AVOID_CPIO */

}

/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/ati/atividmem.c,v 1.1.2.2 1999/10/12 17:18:57 hohndel Exp $ */
/*
 * Copyright 1997 through 1999 by Marc Aurele La France (TSI @ UQV), tsi@ualberta.ca
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

#include "atividmem.h"
#include "misc.h"

/*
 * The number of banks and planes the driver needs to deal with when saving or
 * setting a video mode.
 */
unsigned int ATICurrentBanks, ATIMaximumBanks, ATICurrentPlanes;

/*
 * The amount of video memory that is on the adapter, as opposed to the amount
 * to be made available to the server.
 */
int ATIvideoRam;

CARD8 ATIUsingSmallApertures = FALSE;

CARD8 ATIMemoryType = 0;

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

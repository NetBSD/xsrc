/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/ati/atividmem.h,v 1.1.2.2 1999/10/12 17:18:58 hohndel Exp $ */
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

#ifndef ___ATIVIDMEM_H___
#define ___ATIVIDMEM_H___ 1

#include "Xmd.h"

/*
 * The number of banks and planes the driver needs to deal with when saving or
 * setting a video mode.
 */
extern unsigned int ATICurrentBanks, ATIMaximumBanks, ATICurrentPlanes;

/*
 * The amount of video memory that is on the adapter, as opposed to the amount
 * to be made available to the server.
 */
extern int ATIvideoRam;

extern CARD8 ATIUsingSmallApertures;

extern CARD8 ATIMemoryType;

/* Memory types for 68800's and 88800GX's */
#define MEM_MACH_DRAMx4         0
#define MEM_MACH_VRAM           1
#define MEM_MACH_VRAMssr        2
#define MEM_MACH_DRAMx16        3
#define MEM_MACH_GDRAM          4
#define MEM_MACH_EVRAM          5
#define MEM_MACH_EVRAMssr       6
#define MEM_MACH_TYPE_7         7
extern const char *ATIMemoryTypeNames_Mach[];

/* Memory types for 88800CX's */
#define MEM_CX_DRAM             0
#define MEM_CX_EDO              1
#define MEM_CX_TYPE_2           2
#define MEM_CX_DRAM_A           3
#define MEM_CX_TYPE_4           4
#define MEM_CX_TYPE_5           5
#define MEM_CX_TYPE_6           6
#define MEM_CX_TYPE_7           7
extern const char *ATIMemoryTypeNames_88800CX[];

/* Memory types for 264xT's */
#define MEM_264_NONE            0
#define MEM_264_DRAM            1
#define MEM_264_EDO             2
#define MEM_264_PSEUDO_EDO      3
#define MEM_264_SDRAM           4
#define MEM_264_SGRAM           5
#define MEM_264_SGRAM32         6
#define MEM_264_TYPE_7          7
extern const char *ATIMemoryTypeNames_264xT[];

#endif /* ___ATIVIDMEM_H___ */

/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/ati/atichip.h,v 1.1.2.4 1999/10/12 17:18:52 hohndel Exp $ */
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

#ifndef ___ATICHIP_H___
#define ___ATICHIP_H___ 1

#include "atiproto.h"
#include "Xmd.h"

/*
 * Chip-related definitions.
 */
#define ATI_CHIP_NONE      0
#define ATI_CHIP_VGA       1    /* Generic VGA */
#define ATI_CHIP_18800     2
#define ATI_CHIP_18800_1   3
#define ATI_CHIP_28800_2   4
#define ATI_CHIP_28800_4   5
#define ATI_CHIP_28800_5   6
#define ATI_CHIP_28800_6   7
#define ATI_CHIP_8514A     8    /* 8514/A */
#define ATI_CHIP_CT480     9    /* 8514/A clone */
#define ATI_CHIP_38800_1  10    /* Mach8 */
#define ATI_CHIP_68800    11    /* Mach32 */
#define ATI_CHIP_68800_3  12    /* Mach32 */
#define ATI_CHIP_68800_6  13    /* Mach32 */
#define ATI_CHIP_68800LX  14    /* Mach32 */
#define ATI_CHIP_68800AX  15    /* Mach32 */
#define ATI_CHIP_88800GXC 16    /* Mach64 */
#define ATI_CHIP_88800GXD 17    /* Mach64 */
#define ATI_CHIP_88800GXE 18    /* Mach64 */
#define ATI_CHIP_88800GXF 19    /* Mach64 */
#define ATI_CHIP_88800GX  20    /* Mach64 */
#define ATI_CHIP_88800CX  21    /* Mach64 */
#define ATI_CHIP_264CT    22    /* Mach64 */
#define ATI_CHIP_264ET    23    /* Mach64 */
#define ATI_CHIP_264VT    24    /* Mach64 */
#define ATI_CHIP_264GT    25    /* Mach64 */
#define ATI_CHIP_264VTB   26    /* Mach64 */
#define ATI_CHIP_264GTB   27    /* Mach64 */
#define ATI_CHIP_264VT3   28    /* Mach64 */
#define ATI_CHIP_264GTDVD 29    /* Mach64 */
#define ATI_CHIP_264LT    30    /* Mach64 */
#define ATI_CHIP_264VT4   31    /* Mach64 */
#define ATI_CHIP_264GT2C  32    /* Mach64 */
#define ATI_CHIP_264GTPRO 33    /* Mach64 */
#define ATI_CHIP_264LTPRO 34    /* Mach64 */
#define ATI_CHIP_264XL    35    /* Mach64 */
#define ATI_CHIP_MOBILITY 36    /* Mach64 */
#define ATI_CHIP_Mach64   37    /* Mach64 */
extern CARD8 ATIChip;
extern const char *ATIChipNames[];

/*
 * Foundry codes for 264xT's.
 */
#define ATI_FOUNDRY_SGS  0      /* SGS-Thompson */
#define ATI_FOUNDRY_NEC  1      /* NEC */
#define ATI_FOUNDRY_KSC  2      /* KSC (?) */
#define ATI_FOUNDRY_UMC  3      /* United Microelectronics Corporation */
#define ATI_FOUNDRY_TSMC 4      /* TSMC (?) */
#define ATI_FOUNDRY_5    5
#define ATI_FOUNDRY_6    6
#define ATI_FOUNDRY_UMCA 7      /* UMC alternate */
extern CARD16 ATIChipType, ATIChipClass, ATIChipRevision;
extern CARD16 ATIChipVersion, ATIChipFoundry;
extern CARD8 ATIChipHasSUBSYS_CNTL;
extern CARD8 ATIChipHasVGAWonder;
extern const char *ATIFoundryNames[];

extern int ATILCDPanelID, ATILCDClock, ATILCDHorizontal, ATILCDVertical;

extern void ATIMach32ChipID FunctionPrototype((void));
extern void ATIMach64ChipID FunctionPrototype((const CARD16));

#endif /* ___ATICHIP_H___ */

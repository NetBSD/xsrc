/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/vga/vgafuncs.c,v 3.2 1996/02/04 09:15:09 dawes Exp $ */

/*
 * vgafuncs.c
 *
 * Initialise low level vga256 functions
 */

/* $XConsortium: vgafuncs.c /main/4 1995/11/13 09:26:34 kaleb $ */

#include "vga256.h"

CfbfuncRec vga256LowlevFuncs = {
    vgaBitBlt,		/* default to the 2-bank version */
    vga256DoBitbltCopy,
    vga256FillRectSolidCopy,
    vga2568FillRectTransparentStippled32,
    vga2568FillRectOpaqueStippled32,
    vga256SegmentSS,
    vga256LineSS,
    vga256FillBoxSolid,
    vga256TEGlyphBlt8,
    vga256CopyPlane1to8,
    vga256SolidSpansGeneral,
};


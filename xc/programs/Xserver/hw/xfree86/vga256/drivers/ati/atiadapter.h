/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/ati/atiadapter.h,v 1.1.2.1 1998/02/01 16:41:39 robin Exp $ */
/*
 * Copyright 1997,1998 by Marc Aurele La France (TSI @ UQV), tsi@ualberta.ca
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

#ifndef ___ATIADAPTER_H___
#define ___ATIADAPTER_H___ 1

#include "Xmd.h"

/*
 * Adapter-related definitions.
 */
#define ATI_ADAPTER_NONE        0
#define ATI_ADAPTER_EGA         1
#define ATI_ADAPTER_EGA_PLUS    2
#define ATI_ADAPTER_VGA         3
#define ATI_ADAPTER_BASIC       4
#define ATI_ADAPTER_V3          5
#define ATI_ADAPTER_V4          6
#define ATI_ADAPTER_V5          7
#define ATI_ADAPTER_PLUS        8
#define ATI_ADAPTER_XL          9
#define ATI_ADAPTER_NONISA     10
#define ATI_ADAPTER_8514A      11
#define ATI_ADAPTER_MACH8      12
#define ATI_ADAPTER_MACH32     13
#define ATI_ADAPTER_MACH64     14
extern CARD8 ATIAdapter;
extern CARD8 ATIVGAAdapter;
extern const char *ATIAdapterNames[];

#endif /* ___ATIADAPTER_H___ */

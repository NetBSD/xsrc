/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/ati/atiident.h,v 1.1.2.2 1999/10/12 17:18:54 hohndel Exp $ */
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

#ifndef ___ATIIDENT_H___
#define ___ATIIDENT_H___ 1

#include "atiproto.h"
#include "misc.h"

#define ATI_CHIPSET_ATI     0
#define ATI_CHIPSET_ATIVGA  1
#define ATI_CHIPSET_IBMVGA  2
#define ATI_CHIPSET_IBM8514 3            /* ? */
extern CARD8 ATIChipSet;
extern char * ATIChipSetNames[];

extern char * ATIIdent      FunctionPrototype((int));
extern Bool   ATIIdentProbe FunctionPrototype((void));

#endif /* ___ATIIDENT_H___ */

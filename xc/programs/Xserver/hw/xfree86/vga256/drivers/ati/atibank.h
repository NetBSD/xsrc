/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/ati/atibank.h,v 1.1.2.1 1998/02/01 16:41:41 robin Exp $ */
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

#ifndef ___ATIBANK_H___
#define ___ATIBANK_H___ 1

#include "atiproto.h"

/*
 * This is the type of all banking functions.  After all, this isn't COBOL...
 */
typedef void BankFunction FunctionPrototype((int));

/*
 * These are the bank select functions.  There are several sets of them,
 * starting with generic ones.
 */
extern BankFunction ATISetRead,
                    ATISetWrite,
                    ATISetReadWrite;
/*
 * These are the bank selection functions for V3 adapters.
 */
extern BankFunction ATIV3SetRead,
                    ATIV3SetWrite,
                    ATIV3SetReadWrite;
/*
 * These are the bank selection functions for V4 and V5 adapters.
 */
extern BankFunction ATIV4V5SetRead,
                    ATIV4V5SetWrite,
                    ATIV4V5SetReadWrite;
/*
 * These are the bank selection functions for a Mach64's small dual paged
 * apertures.
 */
extern BankFunction ATIMach64SetReadPacked,
                    ATIMach64SetWritePacked,
                    ATIMach64SetReadWritePacked;
extern BankFunction ATIMach64SetReadPlanar,
                    ATIMach64SetWritePlanar,
                    ATIMach64SetReadWritePlanar;
/*
 * Unfortunately, the above banking functions cannot be called directly from
 * this module.
 */
extern BankFunction ATISelectBank;
extern BankFunction *ATISelectBankFunction;

#endif /* ___ATIBANK_H___ */

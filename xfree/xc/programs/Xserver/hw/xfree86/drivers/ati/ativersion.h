/* $XFree86: xc/programs/Xserver/hw/xfree86/drivers/ati/ativersion.h,v 1.28 2000/12/13 00:45:21 tsi Exp $ */
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

#ifndef ___ATIVERSION_H___
#define ___ATIVERSION_H___ 1

#define ATI_NAME          "ATI"
#define ATI_DRIVER_NAME   "ati"

#define ATI_VERSION_NAME  "6.2.3"

#define ATI_VERSION_MAJOR 6
#define ATI_VERSION_MINOR 2
#define ATI_VERSION_PATCH 3

#define ATI_VERSION_CURRENT \
    ((ATI_VERSION_MAJOR << 20) | (ATI_VERSION_MINOR << 10) | ATI_VERSION_PATCH)

#endif /* ___ATIVERSION_H___ */

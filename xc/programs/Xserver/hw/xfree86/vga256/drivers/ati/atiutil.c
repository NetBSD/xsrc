/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/ati/atiutil.c,v 1.1.2.1 1998/02/01 16:42:04 robin Exp $ */
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

#include "atiutil.h"

/*
 * ATIDivide --
 *
 * Using integer arithmetic and avoiding overflows, this function finds the
 * rounded integer that best approximates
 *
 *         Numerator      Shift
 *        ----------- * 2
 *        Denominator
 *
 * using the specified rounding (floor, nearest or ceiling).
 */
int
ATIDivide(int Numerator, int Denominator, int Shift, const int RoundingKind)
{
    int Multiplier, Divider;
    int Rounding = 0;                           /* Default to floor */

    /* Deal with right shifts */
    if (Shift < 0)
    {
        Divider = (Numerator - 1) ^ Numerator;
        Multiplier = 1 << (-Shift);
        if (Divider > Multiplier)
            Divider = Multiplier;
        Numerator /= Divider;
        Denominator *= Multiplier / Divider;
        Shift = 0;
    }

    if (!RoundingKind)                          /* Nearest */
        Rounding = Denominator >> 1;
    else if (RoundingKind > 0)                  /* Ceiling */
        Rounding = Denominator - 1;

    return ((Numerator / Denominator) << Shift) +
            ((((Numerator % Denominator) << Shift) + Rounding) / Denominator);
}

/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/ati/atireset.c,v 1.1.2.1 1998/02/01 16:42:02 robin Exp $ */
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

#include "atiio.h"
#include "atireset.h"
#include "vga.h"

/*
 * ATISaveScreen --
 *
 * This is called "SaveScreen" for historical reasons.  It actually only
 * performs a VGA sequencer reset.
 */
void
ATISaveScreen(const Bool start)
{
    static Bool started = SS_FINISH;

    if (start == started)
        return;
    started = start;

    if (start == SS_START)
        PutReg(SEQX, 0x00U, 0x02U);     /* Start sequencer reset */
    else
        PutReg(SEQX, 0x00U, 0x03U);     /* End sequencer reset */
}

/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/ati/atiident.c,v 1.1.2.1 1998/02/01 16:41:54 robin Exp $ */
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

#include "atiident.h"
#include "atiutil.h"
#include "vga.h"
#include "xf86_OSproc.h"
#include "xf86Priv.h"

#define XCONFIG_FLAGS_ONLY
#include "xf86_Config.h"

CARD8 ATIChipSet = ATI_CHIPSET_ATI;

char *ATIChipSetNames[] =
{
    "ati",              /* "Full-blown" ATI support */
    "ativga",           /* Don't use ATI accelerator */
    "ibmvga",           /* Generic VGA */
#if 0
    "ibm8514",          /* IBM 8514/A */
#endif
};

/*
 * ATIIdent --
 *
 * Returns a string name for this driver or NULL.
 */
char *
ATIIdent(int n)
{
#if 1
    /* For now, don't advertise non-default chipset names */
    if (n != ATI_CHIPSET_ATI)
#else
    if ((n < 0) || (n >= NumberOf(ATIChipSetNames)))
#endif
        return NULL;
    else
        return ATIChipSetNames[n];
}

/*
 * ATIIdentProbe --
 *
 * This function determines if the user specified a chipset name acceptable to
 * this driver, and, if so, sets ATIChipSet accordingly.
 */
Bool
ATIIdentProbe(void)
{
    int Index;
    static const char *LegacyNames[] =
        {"vgawonder", "mach8", "mach32", "mach64"};

    /* Let ATIProbe continue if no chipset is specified */
    if (!vga256InfoRec.chipset)
        return TRUE;

    for (;  ATIChipSet < NumberOf(ATIChipSetNames);  ATIChipSet++)
        if (!StrCaseCmp(vga256InfoRec.chipset, ATIChipSetNames[ATIChipSet]))
            return TRUE;

    /* Reset to default */
    ATIChipSet = ATI_CHIPSET_ATI;

    /* Check for some other chipset names that need changing */
    for (Index = 0;  StrCaseCmp(vga256InfoRec.chipset, LegacyNames[Index]);  )
        if (++Index >= NumberOf(LegacyNames))
            return FALSE;

    if (xf86Verbose)
        ErrorF("XF86Config ChipSet specification changed from \"%s\" to"
               " \"%s\".\n", LegacyNames[Index], ATIChipSetNames[ATIChipSet]);
    OFLG_CLR(XCONFIG_CHIPSET, &vga256InfoRec.xconfigFlag);
    return TRUE;
}

/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/ati/atidac.c,v 1.1.2.1 1998/02/01 16:41:49 robin Exp $ */
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

#include "atidac.h"
#include "atidepth.h"
#include "atiio.h"
#include "atimono.h"

/*
 * RAMDAC-related definitions.
 */
CARD16 ATIDac = ATI_DAC_GENERIC;
const DACRec ATIDACDescriptors[] =
{   /* Keep this table in ascending DACType order */
    {ATI_DAC_ATI68830,      "ATI 68830 or similar"},
    {ATI_DAC_SC11483,       "Sierra 11483 or similar"},
    {ATI_DAC_ATI68875,      "ATI 68875 or similar"},
    {ATI_DAC_TVP3026_A,     "TI ViewPoint3026 or similar"},
    {ATI_DAC_GENERIC,       "Brooktree 476 or similar"},
    {ATI_DAC_BT481,         "Brooktree 481 or similar"},
    {ATI_DAC_ATT20C491,     "AT&T 20C491 or similar"},
    {ATI_DAC_SC15026,       "Sierra 15026 or similar"},
    {ATI_DAC_MU9C1880,      "Music 9C1880 or similar"},
    {ATI_DAC_IMSG174,       "Inmos G174 or similar"},
    {ATI_DAC_ATI68860_B,    "ATI 68860 (Revision B) or similar"},
    {ATI_DAC_ATI68860_C,    "ATI 68860 (Revision C) or similar"},
    {ATI_DAC_TVP3026_B,     "TI ViewPoint3026 or similar"},
    {ATI_DAC_STG1700,       "SGS-Thompson 1700 or similar"},
    {ATI_DAC_ATT20C498,     "AT&T 20C498 or similar"},
    {ATI_DAC_STG1702,       "SGS-Thompson 1702 or similar"},
    {ATI_DAC_SC15021,       "Sierra 15021 or similar"},
    {ATI_DAC_ATT21C498,     "AT&T 21C498 or similar"},
    {ATI_DAC_STG1703,       "SGS-Thompson 1703 or similar"},
    {ATI_DAC_CH8398,        "Chrontel 8398 or similar"},
    {ATI_DAC_ATT20C408,     "AT&T 20C408 or similar"},
    {ATI_DAC_INTERNAL,      "Internal"},
    {ATI_DAC_IBMRGB514,     "IBM RGB 514 or similar"},
    {ATI_DAC_UNKNOWN,       "Unknown"}          /* Must be last */
};

/*
 * ATISetDACIOPorts --
 *
 * This function sets up DAC access I/O port numbers.
 */
void
ATISetDACIOPorts(CARD8 crtc)
{
    switch (crtc)
    {
        case ATI_CRTC_VGA:
            ATIIOPortDAC_DATA = VGA_DAC_DATA;
            ATIIOPortDAC_MASK = VGA_DAC_MASK;
            ATIIOPortDAC_READ = VGA_DAC_READ;
            ATIIOPortDAC_WRITE = VGA_DAC_WRITE;
            break;

        case ATI_CRTC_8514:
            ATIIOPortDAC_DATA = DAC_DATA;
            ATIIOPortDAC_MASK = DAC_MASK;
            ATIIOPortDAC_READ = DAC_R_INDEX;
            ATIIOPortDAC_WRITE = DAC_W_INDEX;
            break;

        case ATI_CRTC_MACH64:
            ATIIOPortDAC_DATA = ATIIOPortDAC_REGS + 1;
            ATIIOPortDAC_MASK = ATIIOPortDAC_REGS + 2;
            ATIIOPortDAC_READ = ATIIOPortDAC_REGS + 3;
            ATIIOPortDAC_WRITE = ATIIOPortDAC_REGS + 0;
            break;

        default:
            break;
    }
}

/*
 * ATIGetMach64DACCmdReg --
 *
 * Setup to access a RAMDAC's command register.
 */
CARD8
ATIGetMach64DACCmdReg(void)
{
    (void) inb(ATIIOPortDAC_WRITE);     /* Reset to PEL mode */
    (void) inb(ATIIOPortDAC_MASK);      /* Get command register */
    (void) inb(ATIIOPortDAC_MASK);
    (void) inb(ATIIOPortDAC_MASK);
    return inb(ATIIOPortDAC_MASK);
}

/*
 * ATIDACSave --
 *
 * This function is called by ATISave to save the current RAMDAC state into an
 * ATIHWRec structure occurrence.
 */
void
ATIDACSave(ATIHWPtr save)
{
    int Index;

    ATISetDACIOPorts(save->crtc);

    save->dac_read = inb(ATIIOPortDAC_READ);
    save->dac_write = inb(ATIIOPortDAC_WRITE);
    save->dac_mask = inb(ATIIOPortDAC_MASK);

    /* Save DAC's colour lookup table */
    outb(ATIIOPortDAC_MASK, 0xFFU);
    outb(ATIIOPortDAC_READ, 0x00U);
    for (Index = 0;  Index < NumberOf(save->std.DAC);  Index++)
    {
        save->std.DAC[Index] = inb(ATIIOPortDAC_DATA);
        DACDelay;
    }
}

/*
 * ATIDACInit --
 *
 * This function is called by ATIInit to initialize RAMDAC data in an ATIHWRec
 * structure occurrence.
 */
void
ATIDACInit(DisplayModePtr mode)
{
    int Index, Index2;

    if (!mode)
    {
        ATINewHWPtr->dac_read = ATINewHWPtr->dac_write = 0x00U;
        ATINewHWPtr->dac_mask = 0xFFU;

        /*
         * Set colour lookup table.  The first entry has already been zeroed
         * out.
         */
        if (vga256InfoRec.depth > 8)
            for (Index = 1;
                 Index < (NumberOf(ATINewHWPtr->std.DAC) / 3);
                 Index++)
            {
                Index2 = Index * 3;
                ATINewHWPtr->std.DAC[Index2 + 0] =
                    ATINewHWPtr->std.DAC[Index2 + 1] =
                    ATINewHWPtr->std.DAC[Index2 + 2] = Index;
            }
        else
        {
            /*
             * Initialize hardware colour map so that use of uninitialized
             * software colour map entries can easily be seen.
             */
            ATINewHWPtr->std.DAC[3] =
                ATINewHWPtr->std.DAC[4] =
                ATINewHWPtr->std.DAC[5] = 0xFFU;
            for (Index = 2;
                Index < NumberOf(ATINewHWPtr->std.DAC) / 3;
                Index++)
            {
                Index2 = Index * 3;
                ATINewHWPtr->std.DAC[Index2 + 0] = 0xFFU;
                ATINewHWPtr->std.DAC[Index2 + 1] = 0x00U;
                ATINewHWPtr->std.DAC[Index2 + 2] = 0xFFU;
            }
            if (ATIUsing1bppModes)
            {
                ATINewHWPtr->std.DAC[(MONO_BLACK * 3) + 0] =
                    vga256InfoRec.blackColour.red;
                ATINewHWPtr->std.DAC[(MONO_BLACK * 3) + 1] =
                    vga256InfoRec.blackColour.green;
                ATINewHWPtr->std.DAC[(MONO_BLACK * 3) + 2] =
                    vga256InfoRec.blackColour.blue;
                ATINewHWPtr->std.DAC[(MONO_WHITE * 3) + 0] =
                    vga256InfoRec.whiteColour.red;
                ATINewHWPtr->std.DAC[(MONO_WHITE * 3) + 1] =
                    vga256InfoRec.whiteColour.green;
                ATINewHWPtr->std.DAC[(MONO_WHITE * 3) + 2] =
                    vga256InfoRec.whiteColour.blue;
            }

            if (ATICRTC == ATI_CRTC_VGA)
            {
                /* Initialize overscan to black */
                Index = ATINewHWPtr->std.Attribute[17] * 3;
                ATINewHWPtr->std.DAC[Index + 0] =
                    ATINewHWPtr->std.DAC[Index + 1] =
                    ATINewHWPtr->std.DAC[Index + 2] = 0x00U;
            }
        }
    }
}

/*
 * ATIDACRestore --
 *
 * This function is called by ATIRestore to load RAMDAC data from an ATIHWRec
 * structure occurrence.
 */
void
ATIDACRestore(ATIHWPtr restore)
{
    int Index;

    ATISetDACIOPorts(restore->crtc);

    /* Load colour lookup table */
    outb(ATIIOPortDAC_MASK, 0xFFU);
    outb(ATIIOPortDAC_WRITE, 0x00U);
    for (Index = 0;  Index < NumberOf(restore->std.DAC);  Index++)
    {
        outb(ATIIOPortDAC_DATA, restore->std.DAC[Index]);
        DACDelay;
    }

    outb(ATIIOPortDAC_READ, restore->dac_read);
    outb(ATIIOPortDAC_WRITE, restore->dac_write);
}

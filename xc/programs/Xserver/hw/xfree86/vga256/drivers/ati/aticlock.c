/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/ati/aticlock.c,v 1.1.2.6 2000/05/14 02:02:13 tsi Exp $ */
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

/*
 * Adapters prior to V5 use 4 crystals.  Adapters V5 and later use a clock
 * generator chip.  V3 and V4 adapters differ when it comes to choosing clock
 * frequencies.
 *
 * VGA Wonder V3/V4 Adapter Clock Frequencies
 * R E G I S T E R S
 * 1CE(*)    3C2     3C2    Frequency
 * B2h/BEh
 * Bit 6/4  Bit 3   Bit 2   (MHz)
 * ------- ------- -------  -------
 *    0       0       0     50.000
 *    0       0       1     56.644
 *    0       1       0     Spare 1
 *    0       1       1     44.900
 *    1       0       0     44.900
 *    1       0       1     50.000
 *    1       1       0     Spare 2
 *    1       1       1     36.000
 *
 * (*):  V3 uses index B2h, bit 6;  V4 uses index BEh, bit 4
 *
 * V5, PLUS, XL and XL24 usually have an ATI 18810 clock generator chip, but
 * some have an ATI 18811-0, and it's quite conceivable that some exist with
 * ATI 18811-1's or ATI 18811-2's.  Mach32 adapters are known to use any one of
 * these clock generators.  Mach32 adapters also use a different dot clock
 * ordering.  ATI says there is no reliable way for the driver to determine
 * which clock generator is on the adapter, but this driver will do its best to
 * do so anyway.
 *
 * VGA Wonder V5/PLUS/XL/XL24 Clock Frequencies
 * R E G I S T E R S
 *   1CE     1CE     3C2     3C2    Frequency
 *   B9h     BEh                     (MHz)   18811-0  18811-1
 *  Bit 1   Bit 4   Bit 3   Bit 2    18810   18812-0  18811-2  (*5)
 * ------- ------- ------- -------  -------  -------  -------  -------
 *    0       0       0       0      30.240   30.240  135.000   75.000
 *    0       0       0       1      32.000   32.000   32.000   77.500
 *    0       0       1       0      37.500  110.000  110.000   80.000
 *    0       0       1       1      39.000   80.000   80.000   90.000
 *    0       1       0       0      42.954   42.954  100.000   25.175
 *    0       1       0       1      48.771   48.771  126.000   28.322
 *    0       1       1       0        (*1)   92.400   92.400   31.500
 *    0       1       1       1      36.000   36.000   36.000   36.000
 *    1       0       0       0      40.000   39.910   39.910  100.000
 *    1       0       0       1        (*4)   44.900   44.900  110.000
 *    1       0       1       0      75.000   75.000   75.000  126.000
 *    1       0       1       1      65.000   65.000   65.000  135.000
 *    1       1       0       0      50.350   50.350   50.350   40.000
 *    1       1       0       1      56.640   56.640   56.640   44.900
 *    1       1       1       0        (*2)     (*3)     (*3)   50.000
 *    1       1       1       1      44.900   44.900   44.900   65.000
 *
 * (*1) External 0 (supposedly 16.657 Mhz)
 * (*2) External 1 (supposedly 28.322 MHz)
 * (*3) This setting doesn't seem to generate anything
 * (*4) This setting is documented to be 56.644 MHz, but something close to 82
 *      MHz has also been encountered.
 * (*5) This setting is for Dell OmniPlex 590 systems, with a 68800AX on the
 *      motherboard, along with an AT&T21C498 DAC (which is reported as an
 *      STG1700) and ICS2494AM clock generator (a.k.a. ATI 18811-?).
 *
 * Mach32 Clock Frequencies
 * R E G I S T E R S
 *   1CE     1CE     3C2     3C2    Frequency
 *   B9h     BEh                     (MHz)   18811-0  18811-1
 *  Bit 1   Bit 4   Bit 3   Bit 2    18810   18812-0  18811-2  (*5)
 * ------- ------- ------- -------  -------  -------  -------  -------
 *    0       0       0       0      42.954   42.954  100.000   25.175
 *    0       0       0       1      48.771   48.771  126.000   28.322
 *    0       0       1       0        (*1)   92.400   92.400   31.500
 *    0       0       1       1      36.000   36.000   36.000   36.000
 *    0       1       0       0      30.240   30.240  135.000   75.000
 *    0       1       0       1      32.000   32.000   32.000   77.500
 *    0       1       1       0      37.500  110.000  110.000   80.000
 *    0       1       1       1      39.000   80.000   80.000   90.000
 *    1       0       0       0      50.350   50.350   50.350   40.000
 *    1       0       0       1      56.640   56.640   56.640   44.900
 *    1       0       1       0        (*2)     (*3)     (*3)   50.000
 *    1       0       1       1      44.900   44.900   44.900   65.000
 *    1       1       0       0      40.000   39.910   39.910  100.000
 *    1       1       0       1        (*4)   44.900   44.900  110.000
 *    1       1       1       0      75.000   75.000   75.000  126.000
 *    1       1       1       1      65.000   65.000   65.000  135.000
 *
 * (*1) External 0 (supposedly 16.657 Mhz)
 * (*2) External 1 (supposedly 28.322 MHz)
 * (*3) This setting doesn't seem to generate anything
 * (*4) This setting is documented to be 56.644 MHz, but something close to 82
 *      MHz has also been encountered.
 * (*5) This setting is for Dell OmniPlex 590 systems, with a 68800AX on the
 *      motherboard, along with an AT&T21C498 DAC (which is reported as an
 *      STG1700) and ICS2494AM clock generator (a.k.a. ATI 18811-?).
 *
 * Note that, to reduce confusion, this driver masks out the different clock
 * ordering.
 *
 * For all adapters, these frequencies can be divided by 1 or 2.  For all
 * adapters, except Mach32's and Mach64's, frequencies can also be divided by 3
 * or 4.
 *
 *      Register 1CE, index B8h
 *       Bit 7    Bit 6
 *      -------  -------
 *         0        0           Divide by 1
 *         0        1           Divide by 2
 *         1        0           Divide by 3
 *         1        1           Divide by 4
 *
 * With respect to clocks, Mach64's are entirely different animals.
 *
 * The oldest Mach64's use one of the non-programmable clock generators
 * described above.  In this case, the driver will handle clocks in much the
 * same way as it would for a Mach32.
 *
 * All other Mach64 adapters use a programmable clock generator.  BIOS
 * initialization programmes an initial set of frequencies.  Two of these are
 * reserved to allow for the setting of modes that do not use a frequency from
 * this initial set.  One of these reserved slots is used by the BIOS mode set
 * routine, the other by the particular accelerated driver used (MS-Windows,
 * AutoCAD, etc.).  The slots reserved in this way are dependent on the
 * particular clock generator used by the adapter.
 *
 * If the driver does not support the adapter's clock generator, it will try to
 * match the (probed or specified) clocks to one of the following sets.
 *
 * Mach64 Clock Frequencies for unsupported programmable clock generators
 * R E G I S T E R S
 *   1CE     1CE     3C2     3C2    Frequency
 *   B9h     BEh                     (MHz)
 *  Bit 1   Bit 4   Bit 3   Bit 2    Set 1    Set 2    Set 3
 * ------- ------- ------- -------  -------  -------  -------
 *    0       0       0       0      50.350   25.180   25.180
 *    0       0       0       1      56.640   28.320   28.320
 *    0       0       1       0      63.000   31.500    0.000
 *    0       0       1       1      72.000   36.000    0.000
 *    0       1       0       0       0.000    0.000    0.000
 *    0       1       0       1     110.000  110.000    0.000
 *    0       1       1       0     126.000  126.000    0.000
 *    0       1       1       1     135.000  135.000    0.000
 *    1       0       0       0      40.000   40.000    0.000
 *    1       0       0       1      44.900   44.900    0.000
 *    1       0       1       0      49.500   49.500    0.000
 *    1       0       1       1      50.000   50.000    0.000
 *    1       1       0       0       0.000    0.000    0.000
 *    1       1       0       1      80.000   80.000    0.000
 *    1       1       1       0      75.000   75.000    0.000
 *    1       1       1       1      65.000   65.000    0.000
 *
 * The driver will never select a setting of 0.000 MHz.  The above comments on
 * clock ordering and clock divider apply here also.
 *
 * For all supported programmable clock generators, the driver will ignore any
 * XF86Config clock line and programme, as needed, the clock number reserved by
 * the BIOS for accelerated drivers.  The driver's mode initialization routine
 * finds integers N, M and D such that
 *
 *             N
 *      R * -------  MHz
 *           M * D
 *
 * best approximates the mode's clock frequency, where R is the crystal-
 * generated reference frequency (usually 14.318 MHz).  D is a power of 2
 * except for those integrated controllers that also offer odd dividers.
 * Different clock generators have different restrictions on the value N, M and
 * D can assume.  The driver contains an internal table to record these
 * restrictions (among other things).  The resulting values of N, M and D are
 * then encoded in a generator-specific way and used to programme the clock.
 * The Mach64's clock divider is not used in this case.
 */

#include "ati.h"
#include "atiadapter.h"
#include "atichip.h"
#include "aticlock.h"
#include "atidac.h"
#include "atidsp.h"
#include "atiio.h"
#include "xf86_Config.h"
#include "xf86_HWlib.h"
#include "xf86Priv.h"

/*
 * Definitions related to non-programmable clock generators.
 */
CARD8 ATIClock = ATI_CLOCK_NONE;
const char *ATIClockNames[] =
{
    "unknown",
    "IBM VGA compatible",
    "crystals",
    "ATI 18810 or similar",
    "ATI 18811-0 or similar",
    "ATI 18811-1 or similar",
    "ICS 2494-AM or similar",
    "Programmable (BIOS setting 1)",
    "Programmable (BIOS setting 2)",
    "Programmable (BIOS setting 3)"
};

/*
 * Definitions related to programmable clock generators.
 */
CARD8 ATIProgrammableClock = ATI_CLOCK_FIXED;
static CARD16 ATIPostDividers[] = {1, 2, 4, 8, 16, 32, 64, 128},
              ATI264xTPostDividers[] = {1, 2, 4, 8, 3, 0, 6, 12};
ClockRec ATIClockDescriptors[] =
{
    {
          0,   0,   0, 1, 1,
          1,   1,   0,
          0, NULL,
        "Non-programmable"
    },
    {
        257, 512, 257, 1, 1,
         46,  46,   0,
          4, ATIPostDividers,
        "ATI 18818 or ICS 2595 or similar"
    },
    {
          2, 129,   2, 1, 1,
          8,  14,   2,
          8, ATIPostDividers,
        "SGS-Thompson 1703 or similar"
    },
    {
          8, 263,   8, 8, 9,
          4,  12,   2,
          4, ATIPostDividers,
        "Chrontel 8398 or similar"
    },
    {
          2, 255,   0, 1, 1,
         45,  45,   0,
          4, ATI264xTPostDividers,
        "Internal"
    },
    {
          2, 257,   2, 1, 1,
          2,  32,   2,
          4, ATIPostDividers,
        "AT&T 20C408 or similar"
    },
    {
         65, 128,  65, 1, 1,
          2,  31,   0,
          4, ATIPostDividers,
        "IBM RGB 514 or similar"
    }
};
int ATIClockNumberToProgramme = -1;
int ATIReferenceNumerator = 157500,
    ATIReferenceDenominator = 11;
ClockPtr ATIClockDescriptor = ATIClockDescriptors;
CARD16 ATIBIOSClocks[16] =
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

/*
 * XF86Config clocks line that start with the following will either be rejected
 * for ATI boards, or accepted for non-ATI boards.
 */
static const int
ATIVGAClocks[] =
{
     25175,  28322,
        -1
};

/*
 * The driver will attempt to match fixed clocks to one of the following
 * specifications.
 */
static const int
ATICrystalFrequencies[] =
{
     50000,  56644,      0,  44900,  44900,  50000,      0,  36000,
        -1
},
ATI18810Frequencies[] =
{
     30240,  32000,  37500,  39000,  42954,  48771,      0,  36000,
     40000,      0,  75000,  65000,  50350,  56640,      0,  44900
},
ATI188110Frequencies[] =
{
     30240,  32000, 110000,  80000,  42954,  48771,  92400,  36000,
     39910,  44900,  75000,  65000,  50350,  56640,      0,  44900
},
ATI188111Frequencies[] =
{
    135000,  32000, 110000,  80000, 100000, 126000,  92400,  36000,
     39910,  44900,  75000,  65000,  50350,  56640,      0,  44900
},
ATI2494AMFrequencies[] =
{
     75000,  77500,  80000,  90000,  25175,  28322,  31500,  36000,
    100000, 110000, 126000, 135000,  40000,  44900,  50000,  65000
},
ATIMach64AFrequencies[] =
{
         0, 110000, 126000, 135000,  50350,  56640,  63000,  72000,
         0,  80000,  75000,  65000,  40000,  44900,  49500,  50000
},
ATIMach64BFrequencies[] =
{
         0, 110000, 126000, 135000,  25180,  28320,  31500,  36000,
         0,  80000,  75000,  65000,  40000,  44900,  49500,  50000
},
ATIMach64CFrequencies[] =
{
         0,      0,      0,      0,  25180,  28320,      0,      0,
         0,      0,      0,      0,      0,      0,      0,      0
},
*ClockLine[] =
{
    NULL,
    ATIVGAClocks,
    ATICrystalFrequencies,
    ATI18810Frequencies,
    ATI188110Frequencies,
    ATI188111Frequencies,
    ATI2494AMFrequencies,
    ATIMach64AFrequencies,
    ATIMach64BFrequencies,
    ATIMach64CFrequencies,
    NULL
};

/*
 * The driver will reject XF86Config clocks lines that start with, or are an
 * initial subset of, one of the following.
 */
static const int
ATIPre_2_1_1_Clocks_A[] =       /* Based on 18810 */
{
     18000,  22450,  25175,  28320,  36000,  44900,  50350,  56640,
     30240,  32000,  37500,  39000,  40000,      0,  75000,  65000,
        -1
},
ATIPre_2_1_1_Clocks_B[] =       /* Based on 18811-0 */
{
     18000,  22450,  25175,  28320,  36000,  44900,  50350,  56640,
     30240,  32000, 110000,  80000,  39910,  44900,  75000,  65000,
        -1
},
ATIPre_2_1_1_Clocks_C[] =       /* Based on 18811-1 (or -2) */
{
     18000,  22450,  25175,  28320,  36000,  44900,  50350,  56640,
    135000,  32000, 110000,  80000,  39910,  44900,  75000,  65000,
        -1
},
ATIPre_2_1_1_Clocks_D[] =       /* Based on ICS 2494-AM */
{
     18000,  32500,  20000,  22450,  36000,  65000,  40000,  44900,
     75000,  77500,  80000,  90000, 100000, 110000, 126000, 135000,
        -1
},
ATIPre_2_1_1_Clocks_E[] =       /* Based on programmable setting 1 */
{
     36000,  25000,  20000,  22450,  72000,  50000,  40000,  44900,
         0, 110000, 126000, 135000,      0,  80000,  75000,  65000,
        -1
},
ATIPre_2_1_1_Clocks_F[] =       /* Based on programmable setting 2 */
{
     18000,  25000,  20000,  22450,  36000,  50000,  40000,  44900,
         0, 110000, 126000, 135000,      0,  80000,  75000,  65000,
        -1
},
*InvalidClockLine[] =
{
    NULL,
    ATIVGAClocks,
    ATIPre_2_1_1_Clocks_A,
    ATIPre_2_1_1_Clocks_B,
    ATIPre_2_1_1_Clocks_C,
    ATIPre_2_1_1_Clocks_D,
    ATIPre_2_1_1_Clocks_E,
    ATIPre_2_1_1_Clocks_F,
    NULL
};

/*
 * Clock maps.
 */
static const CARD8 Clock_Maps[][16] =
{
    /* Null map */
    {  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15},
    /* VGA Wonder map <-> Mach{8,32,64} */
    {  4,  5,  6,  7,  0,  1,  2,  3, 12, 13, 14, 15,  8,  9, 10, 11},
    /* VGA Wonder map <-> Accelerator */
    {  0,  1,  2,  3,  8,  9, 10, 11,  4,  5,  6,  7, 12, 13, 14, 15},
    /* VGA -> Accelerator map */
    {  8,  9, 10, 11,  0,  1,  2,  3, 12, 13, 14, 15,  4,  5,  6,  7},
    /* Accelerator -> VGA map */
    {  4,  5,  6,  7, 12, 13, 14, 15,  0,  1,  2,  3,  8,  9, 10, 11}
};
#define ATIVGAWonderClockMap         Clock_Maps[0]
#define ATIVGAWonderClockUnMap       ATIVGAWonderClockMap
#define ATIMachVGAClockMap           Clock_Maps[1]
#define ATIMachVGAClockUnMap         ATIMachVGAClockMap
#define ATIVGAProgrammableClockMap   Clock_Maps[2]
#define ATIVGAProgrammableClockUnMap ATIVGAProgrammableClockMap
#define ATIAcceleratorClockMap       Clock_Maps[3]
#define ATIAcceleratorClockUnMap     Clock_Maps[4]
#define ATIProgrammableClockMap      Clock_Maps[0]
#define ATIProgrammableClockUnMap    ATIProgrammableClockMap
const CARD8 *ATIClockMap   = ATIVGAWonderClockMap;
const CARD8 *ATIClockUnMap = ATIVGAWonderClockUnMap;

/*
 * ATIClockSelect --
 *
 * This function selects the dot-clock with index 'Index'.  This is done by
 * setting bits in various registers (generic VGA uses two bits in the
 * miscellaneous output register to select from 4 clocks).  Care is taken to
 * protect any other bits in these registers by fetching their values and
 * masking off the other bits.
 */
static Bool
ATIClockSelect(int Index)
{
    CARD8 misc;

    switch(Index)
    {
        case CLK_REG_SAVE:
            if ((ATICurrentHWPtr->crtc == ATI_CRTC_VGA) && ATIChipHasVGAWonder)
                ATIModifyExtReg(0xB5U, ATICurrentHWPtr->b5, 0x7FU, 0x00U);
            break;

        case CLK_REG_RESTORE:
            break;

        default:
            /* Remap clock number */
            Index = ATICurrentHWPtr->ClockMap[Index & 0x0FU] |
                (Index & ~0x0FU);

            switch (ATICurrentHWPtr->crtc)
            {
                case ATI_CRTC_VGA:
                    /* Set the generic two low-order bits */
                    misc = (inb(R_GENMO) & 0xF3U) | ((Index << 2) & 0x0CU);

                    if (ATIChipHasVGAWonder)
                    {
                        /*
                         * On adapters with crystals, switching to one of the
                         * spare assignments doesn't do anything (i.e. the
                         * previous setting remains in effect).  So, disable
                         * their selection.
                         */
                        if (((Index & 0x03U) == 0x02U) &&
                           ((ATIChip <= ATI_CHIP_18800) ||
                            (ATIAdapter == ATI_ADAPTER_V4)))
                            return FALSE;

                        /* Start sequencer reset */
                        PutReg(SEQX, 0x00, 0x00);

                        /* Set the high order bits */
                        if (ATIChip <= ATI_CHIP_18800)
                            ATIModifyExtReg(0xB2U, -1, 0xBFU, Index << 4);
                        else
                        {
                            ATIModifyExtReg(0xBEU, -1, 0xEFU, Index << 2);
                            if (ATIAdapter != ATI_ADAPTER_V4)
                            {
                                Index >>= 1;
                                ATIModifyExtReg(0xB9U, -1, 0xFDU, Index >> 1);
                            }
                        }

                        /* Set clock divider bits */
                        ATIModifyExtReg(0xB8U, -1, 0x00U,
                            (Index << 3) & 0xC0U);
                    }
                    /*
                     * Reject clocks that cannot be selected.
                     */
                    else
                    {
                        if (Index & 0xFCU)
                            return FALSE;

                        /* Start sequencer reset */
                        PutReg(SEQX, 0x00, 0x00);
                    }

                    /* Must set miscellaneous output register last */
                    outb(GENMO, misc);

                    PutReg(SEQX, 0x00U, 0x03U); /* End sequencer reset */
                    break;

                case ATI_CRTC_MACH64:
                    outl(ATIIOPortCLOCK_CNTL, CLOCK_STROBE |
                        SetBits(Index, CLOCK_SELECT | CLOCK_DIVIDER));
                    break;

                default:
                    return FALSE;
            }
            break;
    }

    return TRUE;
}

/*
 * ATIMatchClockLine --
 *
 * This function tries to match the XF86Config clocks to one of an array of
 * clock lines.  It returns a clock line number (or 0).
 */
static const int
ATIMatchClockLine(const int **Clock_Line,
                  const unsigned short int Number_Of_Clocks,
                  const int Calibration_Clock_Number, const int Clock_Map)
{
    int Clock_Chip = 0, Clock_Chip_Index = 0;
    int Number_Of_Matching_Clocks = 0;
    int Minimum_Gap = CLOCK_TOLERANCE + 1;

    /* For ATI boards, reject generic VGA clocks */
    if ((ATIAdapter != ATI_ADAPTER_VGA) && (Clock_Line == ClockLine))
        Clock_Chip_Index++;
    /* If checking for XF86Config clock order, skip crystals */
    if (Clock_Map)
        Clock_Chip_Index++;

    for (;  Clock_Line[++Clock_Chip_Index];  )
    {
        int Maximum_Gap = 0, Clock_Count = 0, Clock_Index = 0;

        /* Only Mach64's and Rage128's can have programmable clocks */
        if ((Clock_Chip_Index >= ATI_CLOCK_MACH64A) &&
            (ATIAdapter < ATI_ADAPTER_MACH64))
            break;

        for (;  Clock_Index < Number_Of_Clocks;  Clock_Index++)
        {
            int Gap, XF86Config_Clock, Specification_Clock;

            Specification_Clock = Clock_Line[Clock_Chip_Index]
                  [Clock_Maps[Clock_Map][Clock_Index]];
            if (Specification_Clock < 0)
                break;
            if (!Specification_Clock ||
               (Specification_Clock > vga256InfoRec.maxClock))
                continue;

            XF86Config_Clock = vga256InfoRec.clock[Clock_Index];
            if (!XF86Config_Clock ||
                (XF86Config_Clock > vga256InfoRec.maxClock))
                continue;

            Gap = abs(XF86Config_Clock - Specification_Clock);
            if (Gap >= Minimum_Gap)
                goto skip_this_clock_generator;
            if (!Gap)
            {
                if (Clock_Index == Calibration_Clock_Number)
                    continue;
            }
            else if (Gap > Maximum_Gap)
                Maximum_Gap = Gap;
            Clock_Count++;
        }

        if (Clock_Count <= Number_Of_Matching_Clocks)
            continue;
        Number_Of_Matching_Clocks = Clock_Count;
        Clock_Chip = Clock_Chip_Index;
        if (!(Minimum_Gap = Maximum_Gap))
            break;
    skip_this_clock_generator:
        /* For non-ATI adapters, only normalize standard VGA clocks */
        if (ATIAdapter == ATI_ADAPTER_VGA)
            break;
    }

    return Clock_Chip;
}

/*
 * ATIClockProbe --
 *
 * This function is called by ATIProbe and handles the XF86Config clocks line
 * (or lack thereof).
 */
void
ATIClockProbe(void)
{
    unsigned short int Number_Of_Undivided_Clocks;
    unsigned short int Number_Of_Dividers, Number_Of_Clocks;
    int Calibration_Clock_Number, Calibration_Clock_Value;
    int Clock_Index, Specification_Clock, Clock_Map = 0;
    pointer saved_vgaBase = NULL;
    CARD16 VSyncRegister = GENS1(vgaIOBase);
    CARD8 VSyncBit = 0x08U;

    /*
     * Decide what to do about the XF86Config clocks for programmable clock
     * generators.
     */
    if (ATIProgrammableClock != ATI_CLOCK_FIXED)
    {
        /* Check for those that are not (yet) handled */
        if (ATIProgrammableClock >= NumberOf(ATIClockDescriptors))
            ErrorF("Unknown programmable clock generator type (0x%02X)"
                   " detected.\n", ATIProgrammableClock);
        else if (ATIClockDescriptor->MaxN <= 0)
            ErrorF("Unsupported programmable clock generator detected:  %s.",
               ATIClockDescriptor->ClockName);
        else
        {
            /*
             * Recognize supported clock generators.  This involves telling the
             * rest of the server about it and (re-)initializing the XF86Config
             * clocks line.
             */
            OFLG_SET(CLOCK_OPTION_PROGRAMABLE, &vga256InfoRec.clockOptions);

            /* Set internal clock ordering */
            if (ATICRTC == ATI_CRTC_VGA)
            {
                ATIClockMap = ATIVGAProgrammableClockMap;
                ATIClockUnMap = ATIVGAProgrammableClockUnMap;
            }
            else
            {
                ATIClockMap = ATIProgrammableClockMap;
                ATIClockUnMap = ATIProgrammableClockUnMap;
            }

            if (xf86Verbose)
            {
                ErrorF("%s programmable clock generator detected.\n",
                    ATIClockDescriptor->ClockName);
                if (ATIReferenceDenominator == 1)
                    ErrorF("Reference clock %.3f MHz.\n",
                        ((double)ATIReferenceNumerator) / 1000.0);
                else
                    ErrorF("Reference clock %.6g/%d (%.3f) MHz.\n",
                        ((double)ATIReferenceNumerator) / 1000.0,
                        ATIReferenceDenominator,
                        ((double)ATIReferenceNumerator) /
                            ((double)ATIReferenceDenominator * 1000.0));
            }

            /* Clobber XF86Config clock line */
            if (OFLG_ISSET(XCONFIG_CLOCKS, &vga256InfoRec.xconfigFlag))
            {
                OFLG_CLR(XCONFIG_CLOCKS, &vga256InfoRec.xconfigFlag);
                ErrorF("XF86Config clocks specification ignored.\n");
            }
            if (ATIProgrammableClock == ATI_CLOCK_CH8398)
            {   /* The first two are fixed */
                vga256InfoRec.clocks = 2;
                vga256InfoRec.clock[0] = 25175;
                vga256InfoRec.clock[1] = 28322;
            }
            else
            {
                vga256InfoRec.clocks = 0;

                if (ATIProgrammableClock == ATI_CLOCK_INTERNAL)
                    /*
                     * The integrated PLL generates clocks as if the reference
                     * frequency were doubled.
                     */
                    ATIReferenceNumerator <<= 1;
            }

            return;     /* to ATIProbe */
        }
    }

    /*
     * Determine the number of clock values the adapter should be able to
     * generate and the dot clock to use for probe calibration.
     */
probe_clocks:
    if (ATIAdapter == ATI_ADAPTER_VGA)
    {
        Number_Of_Dividers = 1;
        Number_Of_Undivided_Clocks = 4;
        Calibration_Clock_Number = 1;
        Calibration_Clock_Value = 28322;
    }
    else
    {
        Number_Of_Dividers = 4;
        if ((ATIChip <= ATI_CHIP_18800) || (ATIAdapter == ATI_ADAPTER_V4))
        {
            Number_Of_Undivided_Clocks = 8;
            /* Actually, any undivided clock will do */
            Calibration_Clock_Number = 1;
            Calibration_Clock_Value = 56644;
        }
        else
        {
            Number_Of_Undivided_Clocks = 16;
            Calibration_Clock_Number = 7;
            Calibration_Clock_Value = 36000;
            if (ATIChip >= ATI_CHIP_68800)
            {
                Number_Of_Dividers = 2;
                if (ATIChip >= ATI_CHIP_264CT)
                {
                    Number_Of_Dividers = 1;
                    Number_Of_Undivided_Clocks = 4;
                    Calibration_Clock_Number = 1;
                    Calibration_Clock_Value = 28322;
                }
                else if (ATIAdapter >= ATI_ADAPTER_MACH64)
                {
                    Calibration_Clock_Number = 10 /* or 11 */;
                    Calibration_Clock_Value = 75000 /* or 65000 */;
                }

                /*
                 * When selecting clocks, all ATI accelerators use a different
                 * clock ordering.
                 */
                if (ATICRTC == ATI_CRTC_VGA)
                {
                    ATIClockMap = ATIMachVGAClockMap;
                    ATIClockUnMap = ATIMachVGAClockUnMap;
                }
                else
                {
                    ATIClockMap = ATIAcceleratorClockMap;
                    ATIClockUnMap = ATIAcceleratorClockUnMap;
                }
            }
        }
    }
    Number_Of_Clocks = Number_Of_Undivided_Clocks * Number_Of_Dividers;

    /*
     * Respect any XF86Config clocks line.  Well, that's the theory, anyway.
     * In practice, however, the regular use of probed values is widespread, at
     * times causing otherwise inexplicable results.  So, attempt to normalize
     * the clocks to known (specification) values.
     */
    if ((!vga256InfoRec.clocks) || xf86ProbeOnly ||
        (OFLG_ISSET(OPTION_PROBE_CLKS, &vga256InfoRec.options)))
    {
        if (ATIProgrammableClock != ATI_CLOCK_FIXED)
        {
            /*
             * For unsupported programmable clock generators, pick the highest
             * frequency set by BIOS initialization for clock calibration.
             */
            Calibration_Clock_Number = Calibration_Clock_Value = 0;
            for (Clock_Index = 0;
                 Clock_Index < Number_Of_Undivided_Clocks;
                 Clock_Index++)
                if (Calibration_Clock_Value < ATIBIOSClocks[Clock_Index])
                {
                    Calibration_Clock_Number = Clock_Index;
                    Calibration_Clock_Value = ATIBIOSClocks[Clock_Index];
                }
            Calibration_Clock_Number = ATIClockUnMap[Calibration_Clock_Number];
            Calibration_Clock_Value *= 10;
        }

        if (ATIVGAAdapter != ATI_ADAPTER_NONE)
        {
            /*
             * The current video state needs to be saved before the clock
             * probe, and restored after.  On some older adapters, the
             * sequencer resets that occur during the clock probe cause memory
             * corruption.
             */
            saved_vgaBase = vgaBase;
            vgaBase = xf86MapVidMem(vga256InfoRec.scrnIndex, VGA_REGION,
                (pointer)vga256InfoRec.VGAbase, ATI.ChipMapSize);
            ATICurrentHWPtr = ATISave(NULL);
        }

        switch (ATICurrentHWPtr->crtc)
        {
            case ATI_CRTC_VGA:
                /* Already set */
                break;

            case ATI_CRTC_MACH64:
                VSyncRegister = ATIIOPortCRTC_INT_CNTL + 0;
                VSyncBit = GetByte(CRTC_VBLANK, 0);
                break;

            default:
                break;
        }

        /* Probe the adapter for clock values */
        xf86GetClocks(Number_Of_Clocks, ATIClockSelect, (void (*)())NoopDDA,
            (SaveScreenProcPtr)NoopDDA, VSyncRegister, VSyncBit,
            Calibration_Clock_Number, Calibration_Clock_Value, &vga256InfoRec);

        if (ATIVGAAdapter != ATI_ADAPTER_NONE)
        {
            /* Restore video state */
            ATIRestore(ATICurrentHWPtr);
            xfree(ATICurrentHWPtr);
            xf86UnMapVidMem(vga256InfoRec.scrnIndex, VGA_REGION, vgaBase,
                ATI.ChipMapSize);
            vgaBase = saved_vgaBase;
        }

        /* Tell user clocks were probed, instead of supplied */
        OFLG_CLR(XCONFIG_CLOCKS, &vga256InfoRec.xconfigFlag);

        /* Attempt to match probed clocks to a known specification */
        ATIClock = ATIMatchClockLine(ClockLine, Number_Of_Undivided_Clocks,
            Calibration_Clock_Number, 0);

        if ((ATIChip <= ATI_CHIP_18800) || (ATIAdapter == ATI_ADAPTER_V4))
        {
            /* V3 and V4 adapters don't have clock chips */
            if (ATIClock > ATI_CLOCK_CRYSTALS)
                ATIClock = ATI_CLOCK_NONE;
        }
        else
        {
            /* All others don't have crystals */
            if (ATIClock == ATI_CLOCK_CRYSTALS)
                ATIClock = ATI_CLOCK_NONE;
        }
    }
    else
    {
        /*
         * Allow for an initial subset of specification clocks.  Can't allow
         * for any more than that though...
         */
        if (Number_Of_Clocks > vga256InfoRec.clocks)
        {
            Number_Of_Clocks = vga256InfoRec.clocks;
            if (Number_Of_Undivided_Clocks > Number_Of_Clocks)
                Number_Of_Undivided_Clocks = Number_Of_Clocks;
        }
        else
            vga256InfoRec.clocks = Number_Of_Clocks;

        /* Attempt to match clocks to a known specification */
        ATIClock =
            ATIMatchClockLine(ClockLine, Number_Of_Undivided_Clocks, -1, 0);

        if (ATIAdapter != ATI_ADAPTER_VGA)
        {
            if (ATIClock == ATI_CLOCK_NONE)
            {
                /*
                 * Reject certain clock lines that are obviously wrong.  This
                 * includes the standard VGA clocks, and clock lines that could
                 * have been used with the pre-2.1.1 driver.
                 */
                if (ATIMatchClockLine(InvalidClockLine, Number_Of_Clocks, -1, 0))
                    vga256InfoRec.clocks = 0;
                else if ((ATIChip > ATI_CHIP_18800) &&
                         (ATIAdapter != ATI_ADAPTER_V4))
                /*
                 * Check for clocks that are specified in the wrong order.
                 * This is meant to catch those who are trying to use the clock
                 * order intended for the accelerated servers.
                 */
                while((++Clock_Map, Clock_Map %= NumberOf(Clock_Maps)))
                    if ((ATIClock = ATIMatchClockLine(ClockLine,
                        Number_Of_Undivided_Clocks, -1, Clock_Map)))
                    {
                        ErrorF("XF86Config clocks ordering incorrect.  Clocks"
                               " will be reordered.\n See README.ati for more"
                               " information.\n");
                        break;
                    }
            }
            else
            /* Ensure crystals are not matched to clock chips, and vice versa */
            if ((ATIChip <= ATI_CHIP_18800) || (ATIAdapter == ATI_ADAPTER_V4))
            {
                if (ATIClock > ATI_CLOCK_CRYSTALS)
                    vga256InfoRec.clocks = 0;
            }
            else
            {
                if (ATIClock == ATI_CLOCK_CRYSTALS)
                    vga256InfoRec.clocks = 0;
            }

            if (!vga256InfoRec.clocks)
            {
                ErrorF("Invalid or obsolete XF86Config clocks line rejected.\n"
                       " Clocks will be probed.  See README.ati for more"
                       " information.\n");
                goto probe_clocks;
            }
        }
    }

    if (ATIProgrammableClock != ATI_CLOCK_FIXED)
        ATIProgrammableClock = ATI_CLOCK_FIXED;
    else if (ATIClock == ATI_CLOCK_NONE)
        ErrorF("Unknown clock generator detected.\n");
    else if (xf86Verbose)
    {
        if (ATIClock == ATI_CLOCK_CRYSTALS)
            ErrorF("This adapter uses crystals to generate clock"
                   " frequencies.\n");
        else if (ATIClock != ATI_CLOCK_VGA)
            ErrorF("%s clock chip detected.\n", ATIClockNames[ATIClock]);
    }

    if (ATIClock == ATI_CLOCK_NONE)
        return;                 /* Don't touch the clocks */

    /* Replace the undivided clocks with specification values */
    for (Clock_Index = 0;
         Clock_Index < Number_Of_Undivided_Clocks;
         Clock_Index++)
    {
        /*
         * Don't replace clocks that are probed, documented, or set by the user
         * to zero.  One exception is that we need to override the user's value
         * for the spare settings on a crystal-based adapter.  Another
         * exception is when the user specifies the clock ordering intended for
         * the accelerated servers.
         */
        Specification_Clock = ClockLine[ATIClock][Clock_Index];
        if (Specification_Clock < 0)
            break;
        if (!Clock_Map)
        {
            if (!vga256InfoRec.clock[Clock_Index])
                continue;
            if ((!Specification_Clock) && (ATIClock != ATI_CLOCK_CRYSTALS))
                continue;
        }
        vga256InfoRec.clock[Clock_Index] = Specification_Clock;
    }

    /* Adjust the divided clocks */
    for (Clock_Index = Number_Of_Undivided_Clocks;
         Clock_Index < Number_Of_Clocks;
         Clock_Index++)
        vga256InfoRec.clock[Clock_Index] = ATIDivide(
            vga256InfoRec.clock[Clock_Index % Number_Of_Undivided_Clocks],
                (Clock_Index / Number_Of_Undivided_Clocks) + 1, 0, 0);
}

/*
 * ATIClockSave --
 *
 * This function saves that part of an ATIHWRec that relates to clocks.
 */
void
ATIClockSave(ATIHWPtr save)
{
    if (!xf86ProbeFailed && (ATIProgrammableClock != ATI_CLOCK_FIXED))
    {
        if (save->crtc == ATI_CRTC_VGA)
        {
            save->ClockMap = ATIVGAProgrammableClockMap;
            save->ClockUnMap = ATIVGAProgrammableClockUnMap;
        }
        else
        {
            save->ClockMap = ATIProgrammableClockMap;
            save->ClockUnMap = ATIProgrammableClockUnMap;
        }
    }
    else
    {
        if (save->crtc != ATI_CRTC_VGA)
        {
            save->ClockMap = ATIAcceleratorClockMap;
            save->ClockUnMap = ATIAcceleratorClockUnMap;
        }
        else if (ATIChip < ATI_CHIP_68800)
        {
            save->ClockMap = ATIVGAWonderClockMap;
            save->ClockUnMap = ATIVGAWonderClockUnMap;
        }
        else
        {
            save->ClockMap = ATIMachVGAClockMap;
            save->ClockUnMap = ATIMachVGAClockUnMap;
        }
    }
}

/*
 * ATIClockInit --
 *
 * This function is called by ATIInit to generate, if necessary, the data
 * needed for clock programming, and set the clock select bits in various
 * register values.
 */
Bool
ATIClockInit(DisplayModePtr mode)
{
    int N, M, D;
    int ClockSelect, N1, TargetClock, MinimumGap;
    int Frequency, Multiple;            /* Used as temporaries */

    /* Set default values */
    ATINewHWPtr->FeedbackDivider = ATINewHWPtr->ReferenceDivider =
        ATINewHWPtr->PostDivider = 0;

    if ((ATIProgrammableClock == ATI_CLOCK_FIXED) ||
        ((ATIProgrammableClock == ATI_CLOCK_CH8398) && (mode->Clock < 2)))
    {
        /* Use a fixed clock */
        ClockSelect = mode->Clock;
    }
    else
    {
        /* Generate clock programme word, using units of kHz */
        if (ATILCDPanelID >= 0)
            TargetClock = ATILCDClock;
        else if (mode == ATI.ChipBuiltinModes)
            TargetClock = mode->SynthClock;
        else
            TargetClock = vga256InfoRec.clock[mode->Clock];

        MinimumGap = ((unsigned int)(-1)) >> 1;

        /* Loop through reference dividers */
        for (M = ATIClockDescriptor->MinM;  M <= ATIClockDescriptor->MaxM;  M++)
            /* Loop through post-dividers */
            for (D = 0;  D < ATIClockDescriptor->NumD;  D++)
            {
                if (!ATIClockDescriptor->PostDividers[D])
                    continue;

                /*
                 * Calculate closest feedback divider and apply its
                 * restrictions.
                 */
                Multiple = M * ATIReferenceDenominator *
                    ATIClockDescriptor->PostDividers[D];
                N = ATIDivide(TargetClock * Multiple, ATIReferenceNumerator, 0,
                    0);
                if (N < ATIClockDescriptor->MinN)
                    N = ATIClockDescriptor->MinN;
                else if (N > ATIClockDescriptor->MaxN)
                    N = ATIClockDescriptor->MaxN;
                N -= ATIClockDescriptor->NAdjust;
                N1 = (N / ATIClockDescriptor->N1) * ATIClockDescriptor->N2;
                if (N > N1)
                    N = ATIDivide(N1 + 1, ATIClockDescriptor->N1, 0, 1);
                N += ATIClockDescriptor->NAdjust;
                N1 += ATIClockDescriptor->NAdjust;

                for (;  ;  N = N1)
                {
                    /* Pick the closest setting */
                    Frequency = abs(ATIDivide(N * ATIReferenceNumerator,
                        Multiple, 0, 0) - TargetClock);
                    if ((Frequency < MinimumGap) ||
                       ((Frequency == MinimumGap) &&
                        (ATINewHWPtr->FeedbackDivider < N)))
                    {
                        /* Save settings */
                        ATINewHWPtr->FeedbackDivider = N;
                        ATINewHWPtr->ReferenceDivider = M;
                        ATINewHWPtr->PostDivider = D;
                        MinimumGap = Frequency;
                    }

                    if (N <= N1)
                        break;
                }
            }

        Multiple = ATINewHWPtr->ReferenceDivider * ATIReferenceDenominator *
            ATIClockDescriptor->PostDividers[ATINewHWPtr->PostDivider];
        Frequency = ATINewHWPtr->FeedbackDivider * ATIReferenceNumerator;
        Frequency = ATIDivide(Frequency, Multiple, 0, 0);
        if (abs(Frequency - TargetClock) > CLOCK_TOLERANCE)
        {
            ErrorF("Unable to programme clock %.3fMHz for mode %s.\n",
                (double)TargetClock / 1000.0, mode->name);
            return FALSE;
        }
        vga256InfoRec.clock[mode->Clock] = mode->SynthClock = Frequency;
        ClockSelect = ATIClockNumberToProgramme;

        if (xf86Verbose > 1)
        {
            ErrorF("\nProgramming clock %d to %.3fMHz for mode %s.\n",
                mode->Clock, (double)Frequency / 1000.0, mode->name);
            ErrorF(" N=%d, M=%d, D=%d, L=%d.\n",
                ATINewHWPtr->FeedbackDivider, ATINewHWPtr->ReferenceDivider,
                ATINewHWPtr->PostDivider, ClockSelect);
        }

        if ((ATIChip >= ATI_CHIP_264VTB) && (ATIIODecoding == BLOCK_IO))
            ATIDSPInit(mode);           /* Setup DSP registers */

    }

    /* Set clock select bits, after remapping them */
    ATINewHWPtr->std.NoClock = ClockSelect;     /* Save pre-map clock number */
    ClockSelect = ATINewHWPtr->ClockMap[ClockSelect & 0x0FU] |
        (ClockSelect & ~0x0FU);

    switch (ATICRTC)
    {
        case ATI_CRTC_VGA:
            /* Set generic VGA clock select bits */
            ATINewHWPtr->std.MiscOutReg =
                (ATINewHWPtr->std.MiscOutReg & 0xF3U) |
                    ((ClockSelect << 2) & 0x0CU);

            if (ATIChipHasVGAWonder)
            {
                /* Set ATI clock select bits */
                if (ATIChip <= ATI_CHIP_18800)
                    ATINewHWPtr->b2 = (ATINewHWPtr->b2 & 0xBFU) |
                        ((ClockSelect << 4) & 0x40U);
                else
                {
                    ATINewHWPtr->be = (ATINewHWPtr->be & 0xEFU) |
                        ((ClockSelect << 2) & 0x10U);
                    if (ATIAdapter != ATI_ADAPTER_V4)
                    {
                        ClockSelect >>= 1;
                        ATINewHWPtr->b9 = (ATINewHWPtr->b9 & 0xFDU) |
                            ((ClockSelect >> 1) & 0x02U);
                    }
                }

                /* Set clock divider bits */
                ATINewHWPtr->b8 = (ATINewHWPtr->b8 & 0x3FU) |
                    ((ClockSelect << 3) & 0xC0U);
            }
            break;

        case ATI_CRTC_MACH64:
            ATINewHWPtr->clock_cntl = CLOCK_STROBE |
                SetBits(ClockSelect, CLOCK_SELECT | CLOCK_DIVIDER);
            break;

        default:
            break;
    }

    return TRUE;
}

/*
 * ATIClockRestore --
 *
 * This function is called by ATIRestore to programme a clock for the mode
 * being set.
 */
void
ATIClockRestore(ATIHWPtr restore)
{
    CARD8 saved_clock_cntl0, saved_crtc_gen_cntl3;
    CARD8 tmp, tmp2;
    unsigned int Programme;
    int N = restore->FeedbackDivider - ATIClockDescriptor->NAdjust;
    int M = restore->ReferenceDivider - ATIClockDescriptor->MAdjust;
    int D = restore->PostDivider;

    /* Temporarily switch to accelerator mode */
    saved_crtc_gen_cntl3 = inb(ATIIOPortCRTC_GEN_CNTL + 3);
    outb(ATIIOPortCRTC_GEN_CNTL + 3, saved_crtc_gen_cntl3 |
        GetByte(CRTC_EXT_DISP_EN, 3));

    ATISetDACIOPorts(ATI_CRTC_MACH64);

    switch (ATIProgrammableClock)
    {
        case ATI_CLOCK_ICS2595:
            saved_clock_cntl0 = inb(ATIIOPortCLOCK_CNTL);

            Programme = (SetBits(restore->std.NoClock, ICS2595_CLOCK) |
                SetBits(N, ICS2595_FB_DIV) | SetBits(D, ICS2595_POST_DIV)) ^
                ICS2595_TOGGLE;

            ATIDelay(50000);            /* 50 milliseconds */

            (void) xf86DisableInterrupts();

            /* Send all 20 bits of programme word */
            while (Programme >= CLOCK_BIT)
            {
                tmp = (Programme & CLOCK_BIT) | CLOCK_STROBE;
                outb(ATIIOPortCLOCK_CNTL, tmp);
                ATIDelay(26);           /* 26 microseconds */
                outb(ATIIOPortCLOCK_CNTL, tmp | CLOCK_PULSE);
                ATIDelay(26);           /* 26 microseconds */
                Programme >>= 1;
            }

            xf86EnableInterrupts();

            /* Restore register */
            outb(ATIIOPortCLOCK_CNTL, saved_clock_cntl0 | CLOCK_STROBE);
            break;

        case ATI_CLOCK_STG1703:
            (void) ATIGetMach64DACCmdReg();
            (void) inb(ATIIOPortDAC_MASK);
            outb(ATIIOPortDAC_MASK, (restore->std.NoClock << 1) + 0x20U);
            outb(ATIIOPortDAC_MASK, 0);
            outb(ATIIOPortDAC_MASK, SetBits(N, 0xFFU));
            outb(ATIIOPortDAC_MASK, SetBits(M, 0x1FU) | SetBits(D, 0xE0U));
            break;

        case ATI_CLOCK_CH8398:
            tmp = inb(ATIIOPortDAC_CNTL);
            outb(ATIIOPortDAC_CNTL, tmp | (DAC_EXT_SEL_RS2 | DAC_EXT_SEL_RS3));
            outb(ATIIOPortDAC_WRITE, restore->std.NoClock);
            outb(ATIIOPortDAC_DATA, SetBits(N, 0xFFU));
            outb(ATIIOPortDAC_DATA, SetBits(M, 0x3FU) | SetBits(D, 0xC0U));
            outb(ATIIOPortDAC_CNTL, (tmp & ~DAC_EXT_SEL_RS2) | DAC_EXT_SEL_RS3);
            break;

        case ATI_CLOCK_INTERNAL:
            /* Reset VCLK generator */
            ATIPutMach64PLLReg(PLL_VCLK_CNTL, restore->pll_vclk_cntl);

            /* Set post-divider */
            tmp2 = restore->std.NoClock << 1;
            tmp = ATIGetMach64PLLReg(PLL_VCLK_POST_DIV);
            tmp &= ~(0x03U << tmp2);
            tmp |= SetBits(D, 0x03U) << tmp2;
            ATIPutMach64PLLReg(PLL_VCLK_POST_DIV, tmp);

            /* Set extended post-divider */
            tmp = ATIGetMach64PLLReg(PLL_XCLK_CNTL);
            tmp &= ~(SetBits(1, PLL_VCLK0_XDIV) << restore->std.NoClock);
            tmp |= SetBits(D >> 2, PLL_VCLK0_XDIV) << restore->std.NoClock;
            ATIPutMach64PLLReg(PLL_XCLK_CNTL, tmp);

            /* Set feedback divider */
            tmp = PLL_VCLK0_FB_DIV + restore->std.NoClock;
            ATIPutMach64PLLReg(tmp, SetBits(N, 0xFFU));

            /* End VCLK generator reset */
            ATIPutMach64PLLReg(PLL_VCLK_CNTL,
                restore->pll_vclk_cntl & ~PLL_VCLK_RESET);

            /* Reset write bit */
            ATIAccessMach64PLLReg(0, FALSE);
            break;

        case ATI_CLOCK_ATT20C408:
            (void) ATIGetMach64DACCmdReg();
            tmp = inb(ATIIOPortDAC_MASK);
            (void) ATIGetMach64DACCmdReg();
            outb(ATIIOPortDAC_MASK, tmp | 1);
            outb(ATIIOPortDAC_WRITE, 1);
            outb(ATIIOPortDAC_MASK, tmp | 9);
            ATIDelay(400);              /* 400 microseconds */
            tmp2 = (restore->std.NoClock << 2) + 0x40U;
            outb(ATIIOPortDAC_WRITE, tmp2);
            outb(ATIIOPortDAC_MASK, SetBits(N, 0xFFU));
            outb(ATIIOPortDAC_WRITE, ++tmp2);
            outb(ATIIOPortDAC_MASK, SetBits(M, 0x3FU) | SetBits(D, 0xC0U));
            outb(ATIIOPortDAC_WRITE, ++tmp2);
            outb(ATIIOPortDAC_MASK, 0x77U);
            ATIDelay(400);              /* 400 microseconds */
            outb(ATIIOPortDAC_WRITE, 1);
            outb(ATIIOPortDAC_MASK, tmp);
            break;

        case ATI_CLOCK_IBMRGB514:
            tmp = inb(ATIIOPortDAC_CNTL);
            outb(ATIIOPortDAC_CNTL, (tmp & ~DAC_EXT_SEL_RS3) | DAC_EXT_SEL_RS2);
            tmp = (restore->std.NoClock << 1) + 0x20U;
            outb(ATIIOPortDAC_WRITE, tmp);
            outb(ATIIOPortDAC_DATA, 0);
            outb(ATIIOPortDAC_MASK,
                (SetBits(N, 0x3FU) | SetBits(D, 0xC0U)) ^ 0xC0U);
            outb(ATIIOPortDAC_WRITE, tmp + 1);
            outb(ATIIOPortDAC_DATA, 0);
            outb(ATIIOPortDAC_MASK, SetBits(M, 0x3FU));
            break;

        default:
            break;
    }

    (void) inb(ATIIOPortDAC_WRITE);     /* Clear DAC counter */

    /* Restore register */
    outb(ATIIOPortCRTC_GEN_CNTL + 3, saved_crtc_gen_cntl3);
}

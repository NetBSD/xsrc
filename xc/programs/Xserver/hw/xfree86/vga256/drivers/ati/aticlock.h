/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/ati/aticlock.h,v 1.1.2.2 2000/02/28 04:51:28 tsi Exp $ */
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

#ifndef ___ATICLOCK_H___
#define ___ATICLOCK_H___ 1

#include "aticrtc.h"

/*
 * Definitions related to non-programmable clock generators.
 */
#define ATI_CLOCK_NONE      0    /* Must be zero */
#define ATI_CLOCK_VGA       1    /* Must be one */
#define ATI_CLOCK_CRYSTALS  2    /* Must be two */
#define ATI_CLOCK_18810     3
#define ATI_CLOCK_18811_0   4
#define ATI_CLOCK_18811_1   5
#define ATI_CLOCK_2494AM    6
#define ATI_CLOCK_MACH64A   7
#define ATI_CLOCK_MACH64B   8
#define ATI_CLOCK_MACH64C   9
extern CARD8 ATIClock;
extern const char *ATIClockNames[];

/*
 * Definitions related to programmable clock generators.
 */
#define ATI_CLOCK_FIXED     0   /* Further described above */
#define ATI_CLOCK_ICS2595   1
#define ATI_CLOCK_STG1703   2
#define ATI_CLOCK_CH8398    3
#define ATI_CLOCK_INTERNAL  4
#define ATI_CLOCK_ATT20C408 5
#define ATI_CLOCK_IBMRGB514 6
#define ATI_CLOCK_MAX       7   /* Must be last */
extern CARD8 ATIProgrammableClock;
typedef struct
{
        CARD16 MinN, MaxN;              /* Feedback divider and ... */
        CARD16 NAdjust;                 /* ... its adjustment and ... */
        CARD16 N1, N2;                  /* ... its restrictions */
        CARD16 MinM, MaxM;              /* Reference divider and ... */
        CARD16 MAdjust;                 /* ... its adjustment */
        CARD16 NumD, *PostDividers;     /* Post-dividers */
        const char *ClockName;
} ClockRec, *ClockPtr;
extern ClockRec ATIClockDescriptors[];
extern int ATIClockNumberToProgramme;
extern int ATIReferenceNumerator,
           ATIReferenceDenominator;
extern ClockPtr ATIClockDescriptor;
extern CARD16 ATIBIOSClocks[16];

/*
 * Clock maps.
 */
extern const CARD8 *ATIClockMap;
extern const CARD8 *ATIClockUnMap;

extern void ATIClockProbe   FunctionPrototype((void));
extern void ATIClockSave    FunctionPrototype((ATIHWPtr));
extern Bool ATIClockInit    FunctionPrototype((DisplayModePtr));
extern void ATIClockRestore FunctionPrototype((ATIHWPtr));

#endif /* ___ATICLOCK_H___ */

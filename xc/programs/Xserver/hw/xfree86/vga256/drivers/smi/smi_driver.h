/******************************************************************************\

				   Copyright (c) 1999 by Silicon Motion, Inc.
							   All Rights Reserved

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that the
above copyright notice appear in all copies and that both that copyright notice
and this permission notice appear in supporting documentation, and that the name
of Silicon Motion, Inc. not be used in advertising or publicity pertaining to
distribution of the software without specific, written prior permission.
Silicon Motion, Inc. and its suppliers make no representations about the
suitability of this software for any purpose.  It is provided "as is" without
express or implied warranty.

SILICON MOTION, INC. DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT
SHALL SILICON MOTION, INC. AND/OR ITS SUPPLIERS BE LIABLE FOR ANY SPECIAL,
INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF
THIS SOFTWARE.
\******************************************************************************/
/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/smi/smi_driver.h,v 1.1.2.2 1999/12/11 17:43:21 hohndel Exp $ */

/* Header file for SMI server */

#include "mmio.h"

/* Declared in smi_driver.c */
extern vgaVideoChipRec SMI;
extern vgaCRIndex, vgaCRReg;

/* Driver variables */

/* Driver data structure; this should contain all needed info for a mode */
typedef struct {
   vgaHWRec std;
   unsigned char SR17, SR18, SR21, SR31, SR32, SR6A, SR6B, SR81, SRA0;
   unsigned char CR33, CR3A;
   unsigned char CR40[14];
   unsigned char CR90[16];
   unsigned char CRA0[14];
   unsigned char smiDacRegs[0x100];
   unsigned int  DPR10, DPR1C, DPR20, DPR24, DPR28, DPR3C, DPR40, DPR44;
   unsigned int  VPR00, VPR10;
} vgaSMIRec, *vgaSMIPtr;

/*
 * This is a convenience macro, so that entries in the driver structure
 * can simply be dereferenced with 'new->xxx'.
 */

#define new ((vgaSMIPtr) vgaNewVideoState)


/* PCI info structure */

typedef struct SMIPCIInformation {
   int DevID;
   int ChipType;
   int ChipRev;
   unsigned long MemBase;
} SMIPCIInformation;

/* Private data structure used for storing all variables needed in driver */
/* This is not exported outside of here, so no need to worry about packing */

typedef struct {
   int chip;
   pointer MmioMem;
   volatile DPR* ptrDPR;
   volatile VPR* ptrVPR;
   int MemOffScreen;
   unsigned PlaneMask;
   int MCLK;
   Bool NoPCIRetry;
   int panelWidth;
   int panelHeight;
   int MemReserved;
} SMIPRIV;


/* Function prototypes */

SMIPCIInformation * smiGetPCIInfo();
extern Bool SMICursorInit();
extern void SMIRestoreCursor(ScreenPtr);
extern void SMIWarpCursor(ScreenPtr, int, int);
extern void SMIQueryBestSize(int, unsigned short *, unsigned short *, ScreenPtr);

/* Various defines which are used to pass flags between the Setup and
 * Subsequent functions.
 */

#define NO_MONO_FILL      0x00
#define NEED_MONO_FILL    0x01
#define MONO_TRANSPARENCY 0x02


extern SMIPRIV smiPriv;

/* This next function determines if the Source operand is present in the
 * given ROP. The rule is that both the lower and upper nibble of the rop
 * have to be neither 0x00, 0x05, 0x0a or 0x0f. If a CPU-Screen blit is done
 * with a ROP which does not contain the source, the engine will hang when
 * data is written to the image transfer area.
 */

static __inline__ Bool SMIROPHasSrc(int rop)
{
    if ((((rop & 0x0f) == 0x0a) | ((rop & 0x0f) == 0x0f)
        | ((rop & 0x0f) == 0x05) | ((rop & 0x0f) == 0x00)) &
       (((rop & 0xf0) == 0xa0) | ((rop & 0xf0) == 0xf0)
        | ((rop & 0xf0) == 0x50) | ((rop & 0xf0) == 0x00)))
            return FALSE;
    else
            return TRUE;
}

/* This next function determines if the Destination operand is present in the
 * given ROP. The rule is that both the lower and upper nibble of the rop
 * have to be neither 0x00, 0x03, 0x0c or 0x0f.
 */

static __inline__ Bool SMIROPHasDst(int rop)
{
    if ((((rop & 0x0f) == 0x0c) | ((rop & 0x0f) == 0x0f)
        | ((rop & 0x0f) == 0x03) | ((rop & 0x0f) == 0x00)) &
       (((rop & 0xf0) == 0xc0) | ((rop & 0xf0) == 0xf0)
        | ((rop & 0xf0) == 0x30) | ((rop & 0xf0) == 0x00)))
            return FALSE;
    else
            return TRUE;
}

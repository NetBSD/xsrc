/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/cyrix/cyrix_asm.s,v 1.1.2.5 1998/11/06 09:47:05 hohndel Exp $
 *
 * Copyright 1998 by Annius V. Groenink (A.V.Groenink@zfc.nl, avg@cwi.nl)
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of David Wexelblat not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.	David Wexelblat makes no representations
 * about the suitability of this software for any purpose.  It is provided
 * "as is" without express or implied warranty.
 *
 * ANNIUS GROENINK DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL DAVID WEXELBLAT BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
*/

#include "assyntax.h"

/* Assembly code for setting the blit buffer offsets in the L1 cache */
#define CPU_WRITE       D_WORD 0x3C0F
#define CPU_READ        D_WORD 0x3D0F
#define OLD_CPU_WRITE   D_WORD 0x740F
#define OLD_CPU_READ    D_WORD 0x750F
#define BB0_RESET       D_WORD 0x720F
#define BB1_RESET       D_WORD 0x730F

#define L1_BB0_BASE     CONST(0xFFFFFF0C)
#define L1_BB1_BASE     CONST(0xFFFFFF1C)

        FILE("cyrix_asm.s")

        AS_BEGIN

        SEG_TEXT

        ALIGNTEXT4
        GLOBL   GLNAME(CYRIXsetBlitBuffers)
GLNAME(CYRIXsetBlitBuffers):
        PUSH_L  (EBP)
        MOV_L   (ESP, EBP)
        PUSH_L  (EBX)
        MOV_L   (CONTENT(GLNAME(CYRIXbltBuf0Address)), EAX)
        MOV_L   (L1_BB0_BASE, EBX)
        CPU_WRITE
        MOV_L   (CONTENT(GLNAME(CYRIXbltBuf1Address)), EAX)
        MOV_L   (L1_BB1_BASE, EBX)
        CPU_WRITE
        POP_L   (EBX)
        LEAVE
        RET

        ALIGNTEXT4
        GLOBL   GLNAME(CYRIXsetBlitBuffersOnOldChip)
GLNAME(CYRIXsetBlitBuffersOnOldChip):
        PUSH_L  (EBP)
        MOV_L   (ESP, EBP)
        PUSH_L  (EBX)
        MOV_L   (CONTENT(GLNAME(CYRIXbltBuf0Address)), EAX)
        MOV_L   (L1_BB0_BASE, EBX)
        OLD_CPU_WRITE
        MOV_L   (CONTENT(GLNAME(CYRIXbltBuf1Address)), EAX)
        MOV_L   (L1_BB1_BASE, EBX)
        OLD_CPU_WRITE
        POP_L   (EBX)
        LEAVE
        RET


/*
 * Copyright 1997,1998 by Thomas Mueller
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Thomas Mueller not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  Thomas Mueller makes no representations
 * about the suitability of this software for any purpose.  It is provided
 * "as is" without express or implied warranty.
 *
 * THOMAS MUELLER DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL THOMAS MUELLER BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 *
 */
/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/spc8110/spc_bank.s,v 1.1.2.2 1998/10/20 20:51:25 hohndel Exp $ */

/*
 * These are here the very lowlevel VGA bankswitching routines.
 * The segment to switch to is passed via %eax. Only %eax and %edx my be used
 * without saving the original contents.
 */


/*
 * first we have here a mirror for the segment register. That's because a
 * I/O read costs so much more time, that is better to keep the value of it
 * in memory.
 */

#include "assyntax.h"

 	FILE("spc8110bank.s")

	AS_BEGIN

	SEG_DATA
Segment:		/* shadow of 0x3CD */
	D_BYTE 0

	SEG_TEXT

	ALIGNTEXT4
	GLOBL	GLNAME(SPC8110SetRead)
GLNAME(SPC8110SetRead):
	MOV_B	(CONTENT(Segment),AH)
	AND_B	(CONST(0x0f),AH)
	SHL_B	(CONST(4),AL)
	OR_B	(AH,AL)
	MOV_B	(AL,CONTENT(Segment))
	MOV_L	(CONST(0x3CD),EDX)
	OUT_B
	RET

        ALIGNTEXT4
	GLOBL	GLNAME(SPC8110SetWrite)
GLNAME(SPC8110SetWrite):
	MOV_B	(CONTENT(Segment),AH)
	AND_B	(CONST(0xf0),AH)
	OR_B	(AH,AL)
	MOV_B	(AL,CONTENT(Segment))
	MOV_L	(CONST(0x3CD),EDX)
	OUT_B
	RET
	
	ALIGNTEXT4
	GLOBL	GLNAME(SPC8110SetReadWrite)
GLNAME(SPC8110SetReadWrite):
	MOV_B	(AL,AH)
	SHL_B	(CONST(4),AH)
	OR_B	(AH,AL)
	MOV_B	(AL,CONTENT(Segment))
	MOV_L	(CONST(0x3CD),EDX)
        OUT_B
        RET


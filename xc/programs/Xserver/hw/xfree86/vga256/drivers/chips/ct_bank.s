/* $XConsortium: ct_bank.s /main/3 1995/09/04 19:44:07 kaleb $ */
/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/chips/ct_bank.s,v 3.5 1996/08/11 13:02:41 dawes Exp $ */
/*
 * Copyright 1994 by Régis Cridlig <cridlig@dmi.ens.fr>
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Jon Block not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  Jon Block makes no representations
 * about the suitability of this software for any purpose.  It is provided
 * "as is" without express or implied warranty.
 *
 * JON BLOCK DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL JON BLOCK BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */


#include "assyntax.h"

	FILE("chips_bank.s")

	AS_BEGIN

	SEG_TEXT

	ALIGNTEXT4		
	GLOBL	GLNAME(CHIPSSetReadWrite)
	GLOBL	GLNAME(CHIPSSetWrite)
	GLOBL	GLNAME(CHIPSSetRead)
	GLOBL	GLNAME(CHIPSHiQVSetReadWrite)
	GLOBL	GLNAME(CHIPSHiQVSetWrite)
	GLOBL	GLNAME(CHIPSHiQVSetRead)

GLNAME(CHIPSSetReadWrite):
	MOV_B	(AL, AH)		/* Move bank to high half */
	SHL_B	(CONST(3), AH)
	MOV_B	(CONST(0x10),AL)	/* Put read index in low byte */
	MOV_L	(CONST(0x3D6),EDX)	/* Store 0x3D6 register */
	OUT_W				/* Output read bank */
	MOV_B	(CONST(0x11),AL)	/* Put write index in low byte */
	MOV_L	(CONST(0x3D6),EDX)	/* Store 0x3D6 register */
	OUT_W				/* Output read bank */
	RET

GLNAME(CHIPSSetWrite):
	MOV_B	(AL, AH)		/* Move bank to high half */
	SHL_B	(CONST(3), AH)
	MOV_B	(CONST(0x11),AL)	/* Put write index in low byte */
	MOV_L	(CONST(0x3D6),EDX)	/* Store 0x3D6 register */
	OUT_W				/* Output read bank */
	RET

GLNAME(CHIPSSetRead):
	MOV_B	(AL, AH)		/* Move bank to high half */
	SHL_B	(CONST(3), AH)
	MOV_B	(CONST(0x10),AL)	/* Put read index in low byte */
	MOV_L	(CONST(0x3D6),EDX)	/* Store 0x3D6 register */
	OUT_W				/* Output read bank */
	RET

GLNAME(CHIPSHiQVSetReadWrite):
GLNAME(CHIPSHiQVSetWrite):
GLNAME(CHIPSHiQVSetRead):
	MOV_B	(AL, AH)		/* Move bank to high half */
	AND_B	(CONST(0x7F), AH)       /* Mask out bits 6-0 */
	MOV_B	(CONST(0x0E),AL)	/* Put read/write index in low byte */
	MOV_L	(CONST(0x3D6),EDX)	/* Store 0x3D6 register */
	OUT_W				/* Output read/write bank */
	RET

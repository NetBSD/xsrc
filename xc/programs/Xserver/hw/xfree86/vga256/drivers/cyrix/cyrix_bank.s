/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/cyrix/cyrix_bank.s,v 1.1.2.3 1998/11/06 09:47:06 hohndel Exp $ */
/*
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
/* $XConsortium: CYRIX_bank.s /main/4 1996/02/21 17:13:50 kaleb $ */

#include "assyntax.h"

	FILE("CYRIX_bank.s")	/* Define the file name for the .o file */

	AS_BEGIN		/* This macro does all generic setup */

	SEG_TEXT		/* Switch to the text segment */

/* 
 * The SetReadWrite function sets both read and write aperture
 */
	ALIGNTEXT4		
	GLOBL	GLNAME(CYRIXSetReadWrite)
GLNAME(CYRIXSetReadWrite):
	MOV_B	(AL, AH)		/* Move bank to high half */
	MOV_L	(CONST(0x3D4),EDX)	/* Store base register */
	MOV_B	(CONST(0x47),AL)	/* Put write index in low byte */
	OUT_W				/* Output read bank */
	MOV_B	(CONST(0x48),AL)	/* Put read index in low byte */
	OUT_W				/* Output write bank */
	RET

/* 
 * The SetWrite function sets the write aperture only
 */
	ALIGNTEXT4
	GLOBL	GLNAME(CYRIXSetWrite)
GLNAME(CYRIXSetWrite):
	MOV_B	(AL, AH)		/* Move bank to high half */
	MOV_L	(CONST(0x3D4),EDX)	/* Store base register */
	MOV_B	(CONST(0x47),AL)	/* Put write index in low byte */
	OUT_W				/* Output write bank */
	RET

/* 
 * The SetRead function sets the read aperture only
 */
	ALIGNTEXT4
	GLOBL	GLNAME(CYRIXSetRead)
GLNAME(CYRIXSetRead):
	MOV_B	(AL, AH)		/* Move bank to high half */
	MOV_L	(CONST(0x3D4),EDX)	/* Store base register */
	MOV_B	(CONST(0x48),AL)	/* Put read index in low byte */
	OUT_W				/* Output read bank */
	RET

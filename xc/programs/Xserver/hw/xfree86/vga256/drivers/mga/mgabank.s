/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/mga/mgabank.s,v 3.0 1996/09/26 14:00:33 dawes Exp $ */
/*
 * Copyright 1993 by David Wexelblat <dwex@XFree86.org>
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of David Wexelblat not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  David Wexelblat makes no representations
 * about the suitability of this software for any purpose.  It is provided
 * "as is" without express or implied warranty.
 *
 * DAVID WEXELBLAT DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL DAVID WEXELBLAT BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

/*
 * These are here the very lowlevel VGA bankswitching routines.
 * The segment to switch to is passed via %eax. Only %eax and %edx my be used
 * without saving the original contents.
 *
 * WHY ASSEMBLY LANGUAGE ???
 *
 * These routines must be callable by other assembly routines. But I don't
 * want to have the overhead of pushing and poping the normal stack-frame.
 */

/*
 * All XFree86 assembler-language files are written in a macroized assembler
 * format.  This is done to allow a single .s file to be assembled under
 * the USL-syntax assemblers with SVR3/4, the GAS-syntax used by 386BSD,
 * Linux, and Mach, and the Intel-syntax used by Amoeba and Minix.
 *
 * All of the 386 instructions, addressing modes, and appropriate assembler
 * pseudo-ops are definined in the "assyntax.h" header file.
 */
#include "assyntax.h"

/*
 * Three entry points must be defined in this file:
 *
 *	MGASetRead      - Set the read-bank pointer
 *	MGASetWrite     - Set the write-bank pointer
 *	MGASetReadWrite - Set both bank pointers to the same bank
 *
 * Most SVGA chipsets have two bank pointers - a read bank and a write bank.
 * In general, the server sets the read and write banks to the same bank,
 * via the SetReadWrite function.  For BitBlt operations, it is much more
 * efficient to use two banks independently.
 *
 * If the new chipset only has one bank pointer, then all three entry
 * points must still be defined, but they will be all the same function.
 * In this case, make sure the ChipUse2Banks field of the MGAChipRec is
 * set to FALSE in the mgadriver.c file.
 */

	FILE("mgabank.s")	/* Define the file name for the .o file */

	AS_BEGIN		/* This macro does all generic setup */

/*
 * Some chipsets maintain both bank pointers in a single register.  To
 * avoid having to do a read/modify/write cycle on the register, it is
 * best to maintain a copy of the register in memory, modify the 
 * appropriate part of it, and then do a single 'OUT_B' to set it.
 *
 *	SEG_DAT
 *Segment:		
 *	D_BYTE 0
 */

	SEG_TEXT		/* Switch to the text segment */

/* 
 * The SetReadWrite function sets both bank pointers.  The bank will be
 * passed in AL.  As an example, this MGA assumes that the read bank 
 * register is register 'base', index 'idx_r', and the write bank register
 * is index 'idx_w'.
 */
	ALIGNTEXT4
	GLOBL	GLNAME(MGASetRead)
	GLOBL	GLNAME(MGASetWrite)
	GLOBL	GLNAME(MGASetReadWrite)
GLNAME(MGASetRead):
GLNAME(MGASetWrite):
GLNAME(MGASetReadWrite):
	MOV_B	(AL, AH)		/* Move bank to high half */
	MOV_L	(CONST(0x3DE),EDX)	/* Store base register */
	MOV_B	(CONST(0x04),AL)	/* Put write index in low byte */
	OUT_W				/* Output write bank */
	RET

/*
 * The WaitForBlitter function doesn't work correctly
 * in C because of -O2 optimizations (I think...)
 */
#define max_time CONST(10000000)

	GLOBL	GLNAME(MGAMMIOBase)

	ALIGNTEXT4
	GLOBL	GLNAME(MGAWaitForBlitter)
GLNAME(MGAWaitForBlitter):

	PUSH_L	(EBX)
	MOV_L	(CONTENT(GLNAME(MGAMMIOBase)),EBX)
	MOV_L	(max_time,EAX)
.loop:
	DEC_L	(EAX)
	JZ	(.exit)
	TEST_B	(CONST(0x01),REGOFF(0x1E16,EBX))	/* 0x1e16<0> : 1 = dwgengine busy ? 0 = not busy */
	JNZ	(.loop)
.exit:
	POP_L	(EBX)
	RET

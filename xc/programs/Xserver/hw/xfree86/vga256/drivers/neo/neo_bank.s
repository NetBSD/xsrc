/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/neo/neo_bank.s,v 1.1.2.2 1998/09/27 13:25:18 hohndel Exp $ */
/**********************************************************************
Copyright 1998 by Precision Insight, Inc., Cedar Park, Texas.

                        All Rights Reserved

Permission to use, copy, modify, distribute, and sell this software and
its documentation for any purpose is hereby granted without fee,
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in
supporting documentation, and that the name of Precision Insight not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  Precision Insight
and its suppliers make no representations about the suitability of this
software for any purpose.  It is provided "as is" without express or 
implied warranty.

PRECISION INSIGHT DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
EVENT SHALL PRECISION INSIGHT AND/OR ITS SUPPLIERS BE LIABLE FOR ANY
SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER
RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF
CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
**********************************************************************/

/*
 * This Precision Insight driver has been sponsored by Red Hat.
 *
 * Authors:
 *   Jens Owen (jens@precisioninsight.com)
 *   Kevin E. Martin (kevin@precisioninsight.com)
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
 * Three entry points are defined in this file:
 *
 *	NeoSetRead      - Set the read-bank pointer
 *	NeoSetWrite     - Set the write-bank pointer
 *	NeoSetReadWrite - Set both bank pointers to the same bank
 *
 */

	FILE("neo_bank.s")	/* Define the file name for the .o file */

	AS_BEGIN		/* This macro does all generic setup */

	SEG_TEXT		/* Switch to the text segment */

	ALIGNTEXT4		
	GLOBL	GLNAME(NeoSetReadWrite)
GLNAME(NeoSetReadWrite):
	PUSH_L	(EAX)
	MOV_L	(CONST(0x3CE),EDX)
	MOV_B	(CONST(0x11),AL)
	OUT_B
	INC_L	(EDX)
	IN_B			
	MOV_L	(CONST(0x3CE),EDX)
	AND_B	(CONST(0xFC),AL)
	MOV_B	(AL,AH)	
	MOV_B	(CONST(0x11),AL)
	OUT_W		
	POP_L	(EAX)
	MOV_L	(CONST(0x3CE),EDX)
	SHL_L	(CONST(10),EAX)	
	MOV_B	(CONST(0x15),AL)
	OUT_W				/* Write read/write bank       */
	RET

	ALIGNTEXT4		
	GLOBL	GLNAME(NeoSetWrite)
GLNAME(NeoSetWrite):
	PUSH_L	(EAX)
	MOV_L	(CONST(0x3CE),EDX)
	MOV_B	(CONST(0x11),AL)
	OUT_B
	INC_L	(EDX)
	IN_B			
	MOV_L	(CONST(0x3CE),EDX)
	AND_B	(CONST(0xFC),AL)	
	OR_B	(CONST(0x01),AL)
	MOV_B	(AL,AH)			
	MOV_B	(CONST(0x11),AL)
	OUT_W			
	POP_L	(EAX)
	MOV_L	(CONST(0x3CE),EDX)
	SHL_L	(CONST(10),EAX)	
	MOV_B	(CONST(0x16),AL)
	OUT_W				/* Write write bank            */
	RET

	ALIGNTEXT4		
	GLOBL	GLNAME(NeoSetRead)
GLNAME(NeoSetRead):
	PUSH_L	(EAX)
	MOV_L	(CONST(0x3CE),EDX)
	MOV_B	(CONST(0x11),AL)
	OUT_B
	INC_L	(EDX)
	IN_B				
	MOV_L	(CONST(0x3CE),EDX)
	AND_B	(CONST(0xFC),AL)
	OR_B	(CONST(0x01),AL)
	MOV_B	(AL,AH)		
	MOV_B	(CONST(0x11),AL)
	OUT_W			
	POP_L	(EAX)
	MOV_L	(CONST(0x3CE),EDX)
	SHL_L	(CONST(10),EAX)		
	MOV_B	(CONST(0x15),AL)
	OUT_W				/* Write read bank             */
	RET


/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/s3_svga/s3bank.s,v 1.1.2.1 1998/02/07 10:05:40 hohndel Exp $ */
/*
 * Copyright 1990,91 by Thomas Roell, Dinkelscherben, Germany.
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Thomas Roell not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  Thomas Roell makes no representations
 * about the suitability of this software for any purpose.  It is provided
 * "as is" without express or implied warranty.
 *
 * THOMAS ROELL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL THOMAS ROELL BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 *
 * Author:  Thomas Roell, roell@informatik.tu-muenchen.de
 *
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
 * first we have here a mirror for the segment register. That's because a
 * I/O read costs so much more time, that is better to keep the value of it
 * in memory.
 */

#include "assyntax.h"

 	FILE("s3bank.s")

	AS_BEGIN

	SEG_DATA
Segment:
	D_BYTE 0
 

	SEG_TEXT

	ALIGNTEXT4
	GLOBL	GLNAME(S3SetRead)
GLNAME(S3SetRead):
	MOV_B	(AL, CONTENT(Segment))
/*	Segment b0..3 -> CRTC[0x35] b0..3 */
	MOV_B	(CONST(0x35), AL)
	MOV_L	(CONST(0x3D4), EDX)
	OUT_B	
	MOV_L	(CONST(0x3D5), EDX)
	IN_B
	AND_B	(CONST(0xF0), AL)
	MOV_B	(CONTENT(Segment), AH)
	AND_B	(CONST(0x0F), AH)
	OR_B	(AH, AL)
	OUT_B	
/*	Segment b4.5 -> CRTC[0x51] b2.3 */
	MOV_B	(CONST(0x51), AL)
	MOV_L	(CONST(0x3D4), EDX)
	OUT_B	
	MOV_L	(CONST(0x3D5), EDX)
	IN_B
	AND_B	(CONST(0xF3), AL)
	MOV_B	(CONTENT(Segment), AH)
	AND_B	(CONST(0x30), AH)
	ROR_B	(CONST(2), AH)
	OR_B	(AH, AL)
	OUT_B	
		
	RET

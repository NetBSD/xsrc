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
 */

#include "assyntax.h"

	FILE("bank.s")
	AS_BEGIN

	SEG_DATA

	SEG_TEXT

	ALIGNTEXT4
	GLOBL GLNAME(NeomagicSetReadWrite)
	GLOBL GLNAME(NeomagicSetRead)
	GLOBL GLNAME(NeomagicSetWrite)
GLNAME(NeomagicSetReadWrite):
GLNAME(NeomagicSetRead):
GLNAME(NeomagicSetWrite):
	MOV_L	(CONST(0x3CE),EDX)
	SHL_L	(CONST(10),EAX)
	MOV_B	(CONST(0x15),AL)
	OUT_W
	RET

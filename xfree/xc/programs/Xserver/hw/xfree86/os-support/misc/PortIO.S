/* $XFree86: xc/programs/Xserver/hw/xfree86/os-support/misc/PortIO.S,v 1.3 2004/04/05 02:33:15 dawes Exp $ */

/*
 * Copyright (c) 2004 by The XFree86 Project, Inc.
 * All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject
 * to the following conditions:
 *
 *   1.  Redistributions of source code must retain the above copyright
 *       notice, this list of conditions, and the following disclaimer.
 *
 *   2.  Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer
 *       in the documentation and/or other materials provided with the
 *       distribution, and in the same place and form as other copyright,
 *       license and disclaimer information.
 *
 *   3.  The end-user documentation included with the redistribution,
 *       if any, must include the following acknowledgment: "This product
 *       includes software developed by The XFree86 Project, Inc
 *       (http://www.xfree86.org/) and its contributors", in the same
 *       place and form as other third-party acknowledgments.  Alternately,
 *       this acknowledgment may appear in the software itself, in the
 *       same form and location as other such third-party acknowledgments.
 *
 *   4.  Except as contained in this notice, the name of The XFree86
 *       Project, Inc shall not be used in advertising or otherwise to
 *       promote the sale, use or other dealings in this Software without
 *       prior written authorization from The XFree86 Project, Inc.
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESSED OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE XFREE86 PROJECT, INC OR ITS CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
 * OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/* Port I/O functions for platforms with no inlining. */

#if defined(i386)

#include "assyntax.h"

	FILE("PortIO.s")

	AS_BEGIN

	GLOBL	GLNAME(outb)
	GLOBL	GLNAME(outw)
	GLOBL	GLNAME(outl)
	GLOBL	GLNAME(inb)
	GLOBL	GLNAME(inw)
	GLOBL	GLNAME(inl)

	SEG_TEXT
	ALIGNTEXT4
GLNAME(outb):
	MOV_L	(REGOFF(4,ESP), EDX)
	MOV_L	(REGOFF(8,ESP), EAX)
	OUT_B
	RET

	ALIGNTEXT4
GLNAME(outw):
	MOV_L	(REGOFF(4,ESP), EDX)
	MOV_L	(REGOFF(8,ESP), EAX)
	OUT_W
	RET

	ALIGNTEXT4
GLNAME(outl):
	MOV_L	(REGOFF(4,ESP), EDX)
	MOV_L	(REGOFF(8,ESP), EAX)
	OUT_L
	RET

	ALIGNTEXT4
GLNAME(inb):
	MOV_L	(REGOFF(4,ESP), EDX)
	IN_B
	RET

	ALIGNTEXT4
GLNAME(inw):
	MOV_L	(REGOFF(4,ESP), EDX)
	IN_L
	RET

	ALIGNTEXT4
GLNAME(inl):
	MOV_L	(REGOFF(4,ESP), EDX)
	IN_L
	RET

#elif defined(sparc)

	/*
	 * Don't know if these are actually used/useful for Solaris/SPARC,
	 * but they match the gcc inline versions in compiler.h.
	 */

	.file "PortIO.s"

	.global outb
	.global outw
	.global outl
	.global inb
	.global inw
	.global inl

	.section	".text"

	.align 4
outb:
	stba %o1, [%o0] #ASI_P_L
	membar #StoreStore | #StoreLoad
	retl
	nop

	.align 4
outw:
	stha %o1, [%o0] #ASI_P_L
	membar #StoreStore | #StoreLoad
	retl
	nop

	.align 4
outl:
	sta %o1, [%o0] #ASI_P_L
	membar #StoreStore | #StoreLoad
	retl
	nop

	.align 4
inb:
	lduba [%o0] #ASI_P_L, %o0
	retl
	nop

	.align 4
inw:
	lduha [%o0] #ASI_P_L, %o0
	retl
	nop

	.align 4
inl:
	lda [%o0] #ASI_P_L, %o0
	retl
	nop

#endif


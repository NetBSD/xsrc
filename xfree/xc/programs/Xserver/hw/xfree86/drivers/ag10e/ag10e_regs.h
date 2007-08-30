/*
 * Fujitsu AG-10e framebuffer - hardware registers.
 *
 * Copyright (C) 2007 Michael Lorenz
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * JAKUB JELINEK BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
/* $XFree86: xc/programs/Xserver/hw/xfree86/drivers/suncg6/cg6_regs.h,v 1.1 2000/05/23 04:47:43 dawes Exp $ */

#ifndef AG10E_REGS_H
#define AG10E_REGS_H

#include "compiler.h"

#define pGlint pAG10E
#define GLINTPTR(p)    ((AG10EPtr)((p)->driverPrivate))
#define IOBase regs
#define FbBase fb
#define FbMapSize vidmem

#undef MMIO_OUT32
#define MMIO_OUT32(base, offset, val) \
	xf86WriteMmio32Be(base, offset, (CARD32)(val))

#undef MMIO_IN32
#define MMIO_IN32(base, offset) \
	xf86ReadMmio32Be(base, offset)

#include "../glint/glint_regs.h"

#endif /* AG10E_REGS_H */

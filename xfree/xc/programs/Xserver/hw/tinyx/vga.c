/*
 * Copyright © 1999 Keith Packard
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Keith Packard not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  Keith Packard makes no
 * representations about the suitability of this software for any purpose.  It
 * is provided "as is" without express or implied warranty.
 *
 * KEITH PACKARD DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL KEITH PACKARD BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */
/* $XFree86: xc/programs/Xserver/hw/tinyx/vga.c,v 1.3 2004/10/26 22:14:57 tsi Exp $ */
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

#ifdef __GNUC__
#define inline __inline__
#endif
#include "vga.h"
#include <stdio.h>

#ifdef linux
#ifdef __i386__
#define extern static
#include <asm/io.h>
#undef extern

#define _VgaInb(r)	inb(r)
#define _VgaOutb(v,r)	outb(v,r)
#else
#define _VgaInb(r)	0
#define _VgaOutb(v,r)	
#endif

#define _VgaByteAddr(a)	((VGAVOL8 *) (a))
#define _VgaBytePort(a)	(a)
#endif

#ifdef VXWORKS
#define _VgaInb(r)  0
#define _VgaOutb(v,r)   0

#define _VgaByteAddr(a)     ((VGAVOL8 *) ((VGA32) (a) ^ 3))
#define _VgaBytePort(a)	    0

#undef stderr
#define stderr stdout

#endif

#undef VGA_DEBUG_REGISTERS
#ifdef VGA_DEBUG_REGISTERS
#define VGA_DEBUG(a)	fprintf a
#else
#define VGA_DEBUG(a)
#endif

VGA8
VgaInb (VGA16 r)
{
    return _VgaInb(r);
}

void
VgaOutb (VGA8 v, VGA16 r)
{
    _VgaOutb (v,r);
}
	
VGA8
VgaReadMemb (VGA32 addr)
{
    return *_VgaByteAddr(addr);
}

void
VgaWriteMemb (VGA8 v, VGA32 addr)
{
    *_VgaByteAddr(addr) = v;
}

VGA8
VgaFetch (VgaCard *card, VGA16 reg)
{
    VgaMap	map;
    VGA8	value;

    (*card->map) (card, reg, &map, VGAFALSE);
    switch (map.access) {
    case VgaAccessMem:
	value = VgaReadMemb (map.port);
	VGA_DEBUG ((stderr, "%08x -> %2x\n", map.port, value));
	break;
    case VgaAccessIo:
	value = _VgaInb (map.port);
	VGA_DEBUG ((stderr, "%4x    -> %2x\n", map.port, value));
	break;
    case VgaAccessIndMem:
	VgaWriteMemb (map.index, map.port + map.addr);
	value = VgaReadMemb (map.port + map.value);
	VGA_DEBUG ((stderr, "%4x/%2x -> %2x\n", map.port, map.index, value));
	break;
    case VgaAccessIndIo:
	_VgaOutb (map.index, map.port + map.addr);
	value = _VgaInb (map.port + map.value);
	VGA_DEBUG ((stderr, "%4x/%2x -> %2x\n", map.port, map.index, value));
	break;
    case VgaAccessDone:
	value = map.value;
	VGA_DEBUG ((stderr, "direct %4x -> %2x\n", reg, value));
	break;
    default:
	value = 0;
	break;
    }
    return value;
}

void
VgaStore (VgaCard *card, VGA16 reg, VGA8 value)
{
    VgaMap  map;

    map.value = value;
    (*card->map) (card, reg, &map, VGATRUE);
    switch (map.access) {
    case VgaAccessMem:
	VGA_DEBUG ((stderr, "%8x <- %2x\n", map.port, value));
	VgaWriteMemb (map.value, map.port);
	break;
    case VgaAccessIo:
	VGA_DEBUG ((stderr, "%4x    <- %2x\n", map.port, value));
	_VgaOutb (value, map.port);
	break;
    case VgaAccessIndMem:
	VgaWriteMemb (map.index, map.port + map.addr);
	VgaWriteMemb (value, map.port + map.value);
	VGA_DEBUG ((stderr, "%4x/%2x <- %2x\n", map.port, map.index, value));
	break;
    case VgaAccessIndIo:
	VGA_DEBUG ((stderr, "%4x/%2x <- %2x\n", map.port, map.index, value));
	_VgaOutb (map.index, map.port + map.addr);
	_VgaOutb (value, map.port + map.value);
	break;
    case VgaAccessDone:
	VGA_DEBUG ((stderr, "direct %4x <- %2x\n", reg, value));
	break;
    }
}

void
VgaPreserve (VgaCard *card)
{
    VgaSave *s;
    VGA16   id;

    for (s = card->saves; s->first != VGA_REG_NONE; s++)
    {
	for (id = s->first; id <= s->last; id++)
	{
	    card->values[id].cur = VgaFetch (card, id);
	    card->values[id].save = card->values[id].cur;
	    card->values[id].flags = VGA_VALUE_VALID | VGA_VALUE_SAVED;
	}
    }
}

void
VgaRestore (VgaCard *card)
{
    VgaSave *s;
    VGA16   id;

    for (s = card->saves; s->first != VGA_REG_NONE; s++)
    {
	for (id = s->first; id <= s->last; id++)
	{
	    if (card->values[id].flags & VGA_VALUE_SAVED)
	    {
		VgaStore (card, id, card->values[id].save);
		card->values[id].cur = card->values[id].save;
	    }
	}
    }
}

void
VgaFinish (VgaCard *card)
{
    VGA16   id;

    for (id = 0; id < card->max; id++)
	card->values[id].flags = 0;
}

void
VgaInvalidate (VgaCard *card)
{
    VGA16   id;

    for (id = 0; id < card->max; id++)
	card->values[id].flags &= ~VGA_VALUE_VALID;
}


void
_VgaSync (VgaCard *card, VGA16 id)
{
    if (!(card->values[id].flags & VGA_VALUE_VALID))
    {
	card->values[id].cur = VgaFetch (card, id);
	card->values[id].flags |= VGA_VALUE_VALID;
    }
}

void
VgaSet (VgaCard *card, VgaReg *reg, VGA32 value)
{
    VGA8    v, mask, new;
    
    while (reg->len)
    {
	if (reg->id != VGA_REG_NONE)
	{
	    _VgaSync (card, reg->id);
	    mask = ((1 << reg->len) - 1);
	    new = value & mask;
	    mask <<= reg->base;
	    new <<= reg->base;
	    v = card->values[reg->id].cur;
	    v = (v & ~mask) | new;
	    card->values[reg->id].cur = v;
	    card->values[reg->id].flags |= VGA_VALUE_MODIFIED|VGA_VALUE_DIRTY;
	}
	value >>= reg->len;
	reg++;
    }
}

void
VgaFlushReg (VgaCard *card, VgaReg *reg)
{
    while (reg->len)
    {
	if (reg->id != VGA_REG_NONE)
	{
	    if (card->values[reg->id].flags & VGA_VALUE_DIRTY)
	    {
		VgaStore (card, reg->id, card->values[reg->id].cur);
		card->values[reg->id].flags &= ~VGA_VALUE_DIRTY;
	    }
	}
	reg++;
    }
    
}

void
VgaSetImm (VgaCard *card, VgaReg *reg, VGA32 value)
{
    VgaSet (card, reg, value);
    VgaFlushReg (card, reg);
}

VGA32
VgaGet (VgaCard *card, VgaReg *reg)
{
    VGA32   value, offset, v;
    VGA8    mask;

    value = 0;
    offset = 0;
    while (reg->len)
    {
	if (reg->id != VGA_REG_NONE)
	{
	    _VgaSync (card, reg->id);
	    mask = ((1 << reg->len) - 1);
	    v = (card->values[reg->id].cur >> reg->base) & mask;
	    value |= (v << offset);
	}
	offset += reg->len;
	reg++;
    }
    return value;
}

VGA32
VgaGetImm (VgaCard *card, VgaReg *reg)
{
    VgaReg  *r = reg;
    
    while (r->len)
    {
	if (r->id != VGA_REG_NONE)
	    card->values[r->id].flags &= ~VGA_VALUE_VALID;
	r++;
    }
    return VgaGet (card, reg);
}

void
VgaFlush (VgaCard *card)
{
    VGA16   id;

    for (id = 0; id < card->max; id++)
    {
	if (card->values[id].flags & VGA_VALUE_DIRTY)
	{
	    VgaStore (card, id, card->values[id].cur);
	    card->values[id].flags &= ~VGA_VALUE_DIRTY;
	}
    }
}

void
VgaFill (VgaCard *card, VGA16 low, VGA16 high)
{
    VGA16   id;

    for (id = low; id < high; id++)
	_VgaSync (card, id);
}

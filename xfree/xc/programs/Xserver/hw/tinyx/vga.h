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
/* $XFree86: xc/programs/Xserver/hw/tinyx/vga.h,v 1.1 2004/06/02 22:43:01 dawes Exp $ */
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

#ifndef _VGA_H_
#define _VGA_H_

typedef unsigned long	VGA32;
typedef unsigned short	VGA16;
typedef unsigned char	VGA8;
typedef int		VGABOOL;
typedef volatile VGA8	VGAVOL8;

#define VGATRUE	    1
#define VGAFALSE    0

typedef struct _vgaReg {
    VGA16	id;
    VGA8	base;
    VGA8	len;
} VgaReg;

#define VGA_REG_NONE	    0xffff
#define VGA_REG_END	    {VGA_REG_NONE, 0, 0}

typedef struct _vgaValue {
    VGA8	save;
    VGA8	cur;
    VGA16	flags;
} VgaValue;

#define VGA_VALUE_VALID	    1	/* value ever fetched */
#define VGA_VALUE_MODIFIED  2	/* value ever changed */
#define VGA_VALUE_DIRTY	    4	/* value needs syncing */
#define VGA_VALUE_SAVED	    8	/* value preserved */

typedef enum _vgaAccess {
    VgaAccessMem, VgaAccessIo, VgaAccessIndMem, VgaAccessIndIo,
    VgaAccessDone
} VgaAccess;

typedef struct _vgaMap {
    VgaAccess	access;
    VGA32	port;
    VGA8	addr;	    /* for Ind access; addr offset from port */
    VGA8	value;	    /* for Ind access; value offset from port */
    VGA8	index;	    /* for Ind access; index value */
} VgaMap;

#define VGA_UNLOCK_FIXED    1	/* dont save current value */
#define VGA_UNLOCK_LOCK	    2	/* execute only on relock */
#define VGA_UNLOCK_UNLOCK   4	/* execute only on unlock */

typedef struct _vgaSave {
    VGA16	first;
    VGA16	last;
} VgaSave;

#define VGA_SAVE_END	{VGA_REG_NONE, VGA_REG_NONE}

typedef struct _vgaCard {
    void	(*map) (struct _vgaCard *card, VGA16 reg, VgaMap *map, VGABOOL write);
    void	*closure;
    int		max;
    VgaValue	*values;
    VgaSave	*saves;
} VgaCard;

VGA8
VgaInb (VGA16 r);

void
VgaOutb (VGA8 v, VGA16 r);
    
VGA8
VgaReadMemb (VGA32 addr);

void
VgaWriteMemb (VGA8 v, VGA32 addr);

void
VgaSetImm (VgaCard *card, VgaReg *reg, VGA32 value);

VGA32
VgaGetImm (VgaCard *card, VgaReg *reg);

void
VgaSet (VgaCard *card, VgaReg *reg, VGA32 value);

VGA32
VgaGet (VgaCard *card, VgaReg *reg);

void
VgaFlush (VgaCard *card);

void
VgaFinish (VgaCard *card);

void
VgaFill (VgaCard *card, VGA16 low, VGA16 high);

void
VgaPreserve (VgaCard *card);

void
VgaInvalidate (VgaCard *card);

void
VgaRestore (VgaCard *card);

VGA8
VgaFetch (VgaCard *card, VGA16 id);

void
VgaStore (VgaCard *card, VGA16 id, VGA8 value);

VGA8
_VgaFetchInd (VGA16 port, VGA8 reg);

void
_VgaStoreInd (VGA16 port, VGA8 reg, VGA8 value);

#endif /* _VGA_H_ */

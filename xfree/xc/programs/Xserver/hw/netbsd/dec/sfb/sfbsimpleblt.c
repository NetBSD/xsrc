/* $NetBSD: sfbsimpleblt.c,v 1.3 2011/05/24 23:10:03 jakllsch Exp $ */

/*
 * sfb simple rops
 */

/*-
 * Copyright (c) 1998 The NetBSD Foundation, Inc.
 * All rights reserved.
 *
 * This code is derived from software contributed to The NetBSD Foundation
 * by Roland C. Dowdeswell.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE NETBSD FOUNDATION, INC. AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#define PSZ	8

#include	"X.h"
#include	"Xmd.h"
#include	"Xproto.h"
#include	"gcstruct.h"
#include	"windowstr.h"
#include	"scrnintstr.h"
#include	"pixmapstr.h"
#include	"regionstr.h"
#include	"cfb.h"
#include	"cfbmskbits.h"
#include	"cfb8bit.h"
#include	"fastblt.h"
#include	"mergerop.h"

#include	<stdio.h>

#include "dec.h"
#include "sfb.h"
#include "sfbmap.h"

void
decSfbDoBitbltSimple(
		unsigned int *psrcBase,
		unsigned int *pdstBase,
		unsigned int widthSrc,
		unsigned int widthDst,
		sfb_reg_t **regs,
		int alu,
		int sx,
		int sy,
		int dx,
		int dy,
		int h,
		int w,
		int xdir,
		int ydir)
{
	unsigned psrcLine, pdstLine;
	unsigned dpremask, dpostmask;
	int rw, endx;
	int x, y, pshift;
	int dx_align;
	int sdirm, ddirm;
	int cxdir = xdir; /* current xdir */
	int creg = 0;

	switch (alu) {
	case GXclear:
		alu = 0x0;
		break;
	case GXand:
		alu = 0x1;
		break;
	case GXandReverse:
		alu = 0x2;
		break;
	case GXcopy:
		alu = 0x3;
		break;
	case GXandInverted:
		alu = 0x4;
		break;
	case GXnoop:
		alu = 0x5;
		break;
	case GXxor:
		alu = 0x6;
		break;
	case GXor:
		alu = 0x7;
		break;
	case GXnor:
		alu = 0x8;
		break;
	case GXequiv:
		alu = 0x9;
		break;
	case GXinvert:
		alu = 0xa;
		break;
	case GXorReverse:
		alu = 0xb;
		break;
	case GXcopyInverted:
		alu = 0xc;
		break;
	case GXorInverted:
		alu = 0xd;
		break;
	case GXnand:
		alu = 0xe;
		break;
	case GXset:
		alu = 0xf;
		break;
	}
	regs[creg][SFB_REG_GMOR] = 0x0007;	/* Copy Mode */
	regs[creg][SFB_REG_GOPR] = alu;

	if (sy != dy)		/* Source and dest are not on a line, */
		cxdir = 1;	/* so just forward copy */

	if (cxdir == -1) {
		sx += w;
		dx += w;
		w  *= -1;
	}

	if (ydir == 1) {
		psrcLine = sy * widthSrc * 4;
		pdstLine = dy * widthDst * 4;
	} else {
		psrcLine = (sy+h-1) * -widthSrc * 4;
		pdstLine = (dy+h-1) * -widthDst * 4;
	}

	/*
	 * Prepare the masks, starting addresses and lengths
	 * up front.
	 */
	dpostmask  = 0;
	sdirm = sx & ~0x7;
	ddirm = dx & ~0x7;
	dx_align = dx - ddirm;
	pshift = dx_align - (sx - sdirm);
	if ((cxdir*pshift < 0) || (cxdir == -1 && pshift == 0)) {
		pshift   += cxdir*8;
		ddirm    -= cxdir*8;
		dx_align += cxdir*8;
	}

	rw = abs(dx_align + w) + (cxdir==-1?8:0);
	endx = rw & 0x1f;

	if (cxdir == 1) {
		dpremask  = ~0U << dx_align;
		dpostmask = ~0U >> (32 - endx);
		if (endx && (rw < 32)) {
			dpremask &= dpostmask;
			dpostmask = 0;
		}
	} else {
		if (dx_align < 0)
			dpremask = (~0U<<16) |
			    (((0xff >> (-dx_align)) & 0xff) << 8);
		else
			dpremask = (~0U<<8) |
			    ((0xff >> (8-dx_align)) & 0xff);
		endx = 32 - endx;
		if (endx < 8) {
			dpostmask = ~0U >> 8;
			dpostmask |= ((0xff << endx) & 0xff) << 24;
		} else if (endx < 16) {
			dpostmask = ~0U >> 16;
			dpostmask |= ((0xff << (endx - 8)) & 0xff) << 16;
		} else if (endx < 24) {
			dpostmask = ~0U >> 24;
			dpostmask |= ((0xff << (endx - 16)) & 0xff) << 8;
		} else {
			dpostmask = (0xff << (endx - 24)) & 0xff;
		}
		if (endx && (rw < 32)) {
			dpremask &= dpostmask;
			dpostmask = 0;
		}
	}

	sfb_mb();
	regs[creg][SFB_REG_GPSR] = pshift;
	while (h--) {
	    unsigned char *Bsrc;
	    unsigned char *Bdst;
	    int xloop = rw;

	    Bsrc = (unsigned char *)(psrcBase) + psrcLine + sdirm;
	    Bdst = (unsigned char *)(pdstBase) + pdstLine + ddirm;

	    sfb_mb();
	    x = 0;
	    *((unsigned *)Bsrc + x) = ~0; sfb_mb();
	    *((unsigned *)Bdst + x) = dpremask; sfb_mb();
	    x += cxdir*32;
	    xloop -= 32;
#if 0
	    for (; xloop >= 64; x += cxdir*64, xloop -= 64) {
		regs[creg][SFB_REG_GCSR]   = psrcLine + sdirm + x;
		regs[creg++][SFB_REG_GCDR] = pdstLine + ddirm + x;
		if (creg > 3)
			creg = 0;
	    }
#else
	    for (; xloop >= 32; x += cxdir*32, xloop -= 32) {
		*((unsigned *)(Bsrc + x)) = ~0; sfb_mb();
		*((unsigned *)(Bdst + x)) = ~0; sfb_mb();
	    }
#endif
	    if (xloop >= 32) {
		*((unsigned *)(Bsrc + x)) = ~0; sfb_mb();
		*((unsigned *)(Bdst + x)) = ~0; sfb_mb();
		x += cxdir*32;
		xloop -= 32;
	    }
	    if (xloop > 0) {
		*((unsigned *)(Bsrc + x)) = ~0; sfb_mb();
		*((unsigned *)(Bdst + x)) = dpostmask; sfb_mb();
	    }
	    psrcLine += widthSrc * 4;
	    pdstLine += widthDst * 4;
	}
	sfb_mb();
	regs[creg][SFB_REG_GPSR] = 0;
	regs[creg][SFB_REG_GMOR] = 0x0000;	/* Simple Mode */
	regs[creg][SFB_REG_GOPR] = 0x0003;	/* GXcopy */
}

/*	$NetBSD: pxmisc.c,v 1.2 2008/04/28 20:57:37 martin Exp $	*/

/*-
 * Copyright (c) 2001 The NetBSD Foundation, Inc.
 * All rights reserved.
 *
 * This code is derived from software contributed to The NetBSD Foundation
 * by Andrew Doran.
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

#include "px.h"

#include "region.h"

#define	DOFG(v1, v2, lw, xya, s, c)			\
	do {						\
		pb[0] = (s)[0];				\
		pb[1] = (s)[1];				\
		pb[2] = (s)[2];				\
		pb[3] = (s)[3];				\
		pb[4] = (s)[4];				\
		pb[5] = (s)[5];				\
		pb[6] = (s)[6];				\
		pb[7] = (s)[7];				\
		pb[8] = xya;				\
		pb[9] = v1;				\
		pb[10] = v2;				\
		pb[11] = lw;				\
		pb[12] = c;				\
	} while (0)

#define	DOBG(v1, v2, lw, xya, c)			\
	do {						\
		const int all1 = 0xffffffff;		\
							\
		pb[0] = all1;				\
		pb[1] = all1;				\
		pb[2] = all1;				\
		pb[3] = all1;				\
		pb[4] = all1;				\
		pb[5] = all1;				\
		pb[6] = all1;				\
		pb[7] = all1;				\
		pb[8] = xya;				\
		pb[9] = v1;				\
		pb[10] = v2;				\
		pb[11] = lw;				\
		pb[12] = c;				\
	} while (0)

Bool
pxMaskFromStipple(pxScreenPrivPtr sp, PixmapPtr pix, pxMaskPtr mask,
		  int fg, int bg)
{
	u_int width, height, tmp1, tmp2, i, j, stride;
	u_int32_t *base;

	PX_TRACE("pxMaskFromStipple");

	width = pix->drawable.width;
	height = pix->drawable.height;

	if (width > 16 || (width & (width - 1)) != 0 ||
	    height > 16 || (height & (height - 1)) != 0 ||
	    (pix->drawable.x & 31) != 0)
		return (FALSE);

	stride = pix->devKind >> 2;
	base = (u_int32_t *)((u_int8_t *)pix->devPrivate.ptr +
	    (pix->drawable.x >> 3) +
	    (pix->drawable.y * stride));

	for (i = 0; i < height; i++) {
		tmp1 = base[i * stride];
		tmp2 = 0;
		for (j = 0; j < 16; j += width)
			tmp2 |= (tmp1 << j);
		mask->data[i] = tmp2;
	}

	for (height--; i < 16; i++)
		mask->data[i] = mask->data[i & height];

	if (sp->bpp == 8) {
		mask->fg = PX_DUPBYTE(fg);
		mask->bg = PX_DUPBYTE(bg);
	} else {
		mask->fg = fg;
		mask->bg = bg;
	}

	return (TRUE);
}

Bool
pxMaskFromTile(pxScreenPrivPtr sp, PixmapPtr pix, pxMaskPtr mask)
{
	u_int Bpp, width, height, stride, i, j, fg, bg, hbg, mr;
	u_int32_t sb;
	u_int8_t *p;

	PX_TRACE("pxMaskFromTile");

	width = pix->drawable.width;
	height = pix->drawable.height;

	if (width > 16 || (width & (width - 1)) != 0 ||
	    height > 16 || (height & (height - 1)) != 0 ||
	    pix->drawable.bitsPerPixel != sp->bpp)
		return (FALSE);

	Bpp = pix->drawable.bitsPerPixel >> 3;
	stride = pix->devKind;
	p = (u_int8_t *)pix->devPrivate.ptr +
	    (pix->drawable.y * stride) +
	    (pix->drawable.x * Bpp);

	height--;
	width--;
	hbg = 0;

	fg = (Bpp == 1 ? *p : *(u_int32_t *)p);

	for (i = 0; i <= height; i++) {
		mr = 0;

		for (j = 0; j < 16; j++) {
			if (Bpp == 1)
				sb = p[j & width];
			else
				sb = ((u_int32_t *)p)[j & width];

			if (sb == fg) {
				mr |= (0x8000 >> j);
				continue;
			} else if (sb == bg)
				continue;

			if (!hbg) {
				bg = sb;
				hbg = 1;
				continue;
			}

			/* More than 2 colours. */
			return (FALSE);
		}

		p += stride;
		mask->data[i] = mr;
	}

	for (; i < 16; i++)
		mask->data[i] = mask->data[i & height];

	if (Bpp == 1) {
		mask->fg = PX_DUPBYTE(fg);
		mask->bg = PX_DUPBYTE(bg);
	} else {
		mask->fg = fg;
		mask->bg = bg;
	}

	return (TRUE);
}


void
pxFillBoxSolid(pxScreenPrivPtr sp, RegionPtr pRegion, u_int pixel)
{
	u_int32_t *pb;
	int lw, psy;
	BoxPtr pBox, pBoxMax;
	pxPacket pxp;

	PX_TRACE("pxFillBoxSolid");

	if (sp->bpp == 8)
		pixel = PX_DUPBYTE(pixel);

	pb = pxPacketStart(sp, &pxp, 5, 3);
	pb[0] = STAMP_CMD_LINES | STAMP_RGB_CONST | STAMP_LW_PERPRIMATIVE;
	pb[1] = 0x00ffffff;
	pb[2] = 0;
	pb[3] = STAMP_METHOD_COPY | STAMP_UPDATE_ENABLE;
	pb[4] = pixel;

	pBox = REGION_RECTS(pRegion);
	pBoxMax = pBox + REGION_NUM_RECTS(pRegion);

	for (; pBox < pBoxMax; pBox++) {
		lw = ((pBox->y2 - pBox->y1) << 2) - 1;
		psy = (pBox->y1 << 3) + lw;

		pb = pxPacketAddPrim(sp, &pxp);
		pb[0] = (pBox->x1 << 19) | psy;
		pb[1] = (pBox->x2 << 19) | psy;
		pb[2] = lw;
	}

	pxPacketFlush(sp, &pxp);
}

void
pxFillBoxTiled(pxScreenPrivPtr sp, RegionPtr pRegion, pxMaskPtr mask)
{
	u_int32_t *pb;
	int v1, v2, lw, xya, psy, stampw, stamphm;
	BoxPtr pBox, pBoxMax;
	pxPacket pxp;

	PX_TRACE("pxFillBoxTiled");

	stampw = sp->stampw;
	stamphm = sp->stamphm;

	pb = pxPacketStart(sp, &pxp, 4, 13);
	pb[0] = STAMP_CMD_LINES | STAMP_RGB_FLAT | STAMP_LW_PERPRIMATIVE |
	    STAMP_XY_PERPRIMATIVE;
	pb[1] = 0x00ffffff;
	pb[2] = 0;
	pb[3] = STAMP_METHOD_COPY | STAMP_UPDATE_ENABLE | STAMP_WE_XYMASK;

	pBox = REGION_RECTS(pRegion);
	pBoxMax = pBox + REGION_NUM_RECTS(pRegion);

	for (; pBox < pBoxMax; pBox++) {
		lw = ((pBox->y2 - pBox->y1) << 2) - 1;
		psy = (pBox->y1 << 3) + lw;
		v1 = (pBox->x1 << 19) | psy;
		v2 = (pBox->x2 << 19) | psy;

		xya = XYMASKADDR(stampw, stamphm, pBox->x1, pBox->y1,
		    pBox->x1 & 15, pBox->y1 & 15);

		pb = pxPacketAddPrim(sp, &pxp);
		DOBG(v1, v2, lw, xya, mask->bg);
		pb = pxPacketAddPrim(sp, &pxp);
		DOFG(v1, v2, lw, xya, (u_int32_t *)mask->data, mask->fg);
	}

	pxPacketFlush(sp, &pxp);
}

void *
pxCompressBuf24to24(void *dst, void *src, int pixelcnt)
{

	pixelcnt <<= 2;
	memcpy(dst, src, pixelcnt);

#ifdef __alpha__
	return ((caddr_t)dst + ((pixelcnt + 1) & ~1));
#else
	return ((caddr_t)dst + pixelcnt);
#endif
}

void *
pxCompressBuf24to8(void *dst, void *src, int pixelcnt)
{
	u_int32_t *sp;
	u_int8_t *dp, *maxdp;

	sp = src;
	dp = dst;
	maxdp = dp + pixelcnt;

	do {
		*dp++ = (u_int8_t)*sp++;
	} while (dp < maxdp);

#ifdef __alpha__
	return ((void *)(((u_long)dp + 7) & ~7));
#else
	return ((void *)(((u_long)dp + 3) & ~3));
#endif
}

void
pxExpandBuf24to24(void *dst, void *src, int pixelcnt)
{

	memcpy(dst, src, pixelcnt << 2);
}

void
pxExpandBuf8to8(void *dst, void *src, int pixelcnt)
{
	u_int8_t *sp, *maxsp;
	u_int32_t *dp;

	dp = dst;
	sp = src;
	maxsp = sp + pixelcnt;

	do {
		*dp++ = *sp++;
	} while (sp < maxsp);
}

void
pxExpandBuf8to24(void *dst, void *src, int pixelcnt)
{
	u_int8_t *sp, *maxsp, v;
	u_int32_t *dp;

	dp = dst;
	sp = src;
	maxsp = sp + pixelcnt;

	do {
		v = *sp++;
		*dp++ = v | (v << 8) | (v << 16);
	} while (sp < maxsp);
}

void
pxTileBuf8r8(void *dst, void *src, int width, int tw, int tx)
{
	u_int8_t *p, *base;
	u_int32_t *ib, *ibmax;
	int i;

	ib = dst;
	base = src;
	p = base + tx;

	do {
		i = min(tw - tx, width);
		width -= i;
		ibmax = ib + i;
		tx = 0;
		do {
			*ib++ = *p;
			p++;
		} while (ib < ibmax);
		p = base;
	} while (width > 0);
}

void
pxTileBuf24r24(void *dst, void *src, int width, int tw, int tx)
{
	u_int32_t *p, *base, *ib, *ibmax;
	int i;

	ib = dst;
	base = src;
	p = base + tx;

	do {
		i = min(tw - tx, width);
		width -= i;
		ibmax = ib + i;
		tx = 0;
		do {
			*ib++ = *p++;
		} while (ib < ibmax);
		p = base;
	} while (width > 0);
}

void
pxTileBuf8r24(void *dst, void *src, int width, int tw, int tx)
{
	u_int32_t *ib, *ibmax;
	u_int8_t *p, *base;
	int i;

	ib = dst;
	base = src;
	p = base + tx;

	do {
		i = min(tw - tx, width);
		width -= i;
		tx = 0;
		ibmax = ib + i;
		do {
			*ib++ = (*p << 16) | (*p << 8) | *p;
			p++;
		} while (ib < ibmax);
		p = base;
	} while (width > 0);
}

void
pxStippleBuf(PixmapPtr pStipple, u_int32_t *pdst, int width, int fillStyle,
	     u_int32_t fgfill, u_int32_t bgfill, int xorg, int yorg)
{
	int sx, sy, sxmin, sxmax, w;
	u_int sbits, smask;
	u_int32_t *sptr, *pdstmax;

	PX_TRACE("pxStippleBuf");

	sx = pStipple->drawable.x + xorg % pStipple->drawable.width;
	sy = pStipple->drawable.y + yorg % pStipple->drawable.height;

	sxmin = pStipple->drawable.x;
	sxmax = pStipple->drawable.x + pStipple->drawable.width;

	sptr = (u_int32_t *)((u_int8_t *)pStipple->devPrivate.ptr +
	    sy * pStipple->devKind);

	while (width > 0) {
		w = min(width, sxmax - sx);
		w = min(w, 32 - (sx & 31));
		width -= w;

		sbits = sptr[sx >> 5];
		smask = 1 << (sx & 31);

		pdstmax = pdst + w;

		if ((sx += w) >= sxmax)
			sx = sxmin;

		do {
			if ((sbits & smask) != 0)
				*pdst = fgfill;
			pdst++;
			smask <<= 1;
		} while (pdst < pdstmax);
	}
}

void
pxStippleBufOpaque(PixmapPtr pStipple, u_int32_t *pdst, int width,
		   int fillStyle, u_int32_t fgfill, u_int32_t bgfill,
		   int xorg, int yorg)
{
	int sx, sy, sxmin, sxmax, w;
	u_int sbits, smask;
	u_int32_t *sptr, *pdstmax, tmp;

	PX_TRACE("pxStippleBuf");

	sx = pStipple->drawable.x + xorg % pStipple->drawable.width;
	sy = pStipple->drawable.y + yorg % pStipple->drawable.height;

	sxmin = pStipple->drawable.x;
	sxmax = pStipple->drawable.x + pStipple->drawable.width;

	sptr = (u_int32_t *)((u_int8_t *)pStipple->devPrivate.ptr +
	    sy * pStipple->devKind);

	while (width > 0) {
		w = min(width, sxmax - sx);
		w = min(w, 32 - (sx & 31));
		width -= w;

		sbits = sptr[sx >> 5];
		smask = 1 << (sx & 31);

		pdstmax = pdst + w;

		if ((sx += w) >= sxmax)
			sx = sxmin;

		do {
			tmp = bgfill;
			if ((sbits & smask) != 0)
				tmp = fgfill;
			*pdst++ = tmp;
			smask <<= 1;
		} while (pdst < pdstmax);
	}
}

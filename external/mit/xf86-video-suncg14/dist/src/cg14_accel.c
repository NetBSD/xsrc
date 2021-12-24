/* $NetBSD: cg14_accel.c,v 1.27 2021/12/24 04:41:40 macallan Exp $ */
/*
 * Copyright (c) 2013 Michael Lorenz
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *    - Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *    - Redistributions in binary form must reproduce the above
 *      copyright notice, this list of conditions and the following
 *      disclaimer in the documentation and/or other materials provided
 *      with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <sys/types.h>

/* all driver need this */
#include "xf86.h"
#include "xf86_OSproc.h"
#include "compiler.h"

#include "cg14.h"

//#define SX_DEBUG

#ifdef SX_DEBUG
#define ENTER xf86Msg(X_ERROR, "%s>\n", __func__);
#define DPRINTF xf86Msg
#else
#define ENTER
#define DPRINTF while (0) xf86Msg
#endif

#define arraysize(ary)        (sizeof(ary) / sizeof(ary[0]))

/* 0xcc is SX's GXcopy equivalent */
uint32_t sx_rop[] = { 0x00, 0x88, 0x44, 0xcc, 0x22, 0xaa, 0x66, 0xee,
		      0x11, 0x99, 0x55, 0xdd, 0x33, 0xbb, 0x77, 0xff};

int src_formats[] = {PICT_a8r8g8b8, PICT_x8r8g8b8,
		     PICT_a8b8g8r8, PICT_x8b8g8r8, PICT_a8};
int tex_formats[] = {PICT_a8r8g8b8, PICT_a8b8g8r8, PICT_a8};

static void CG14Copy32(PixmapPtr, int, int, int, int, int, int);
static void CG14Copy8(PixmapPtr, int, int, int, int, int, int);

static inline void
CG14Wait(Cg14Ptr p)
{
	int bail = 10000000;
	/* we wait for the busy bit to clear */
	while (((read_sx_reg(p, SX_CONTROL_STATUS) & SX_BZ) != 0) &&
	       (bail > 0)) {
		bail--;
	};
	if (bail == 0) {
		xf86Msg(X_ERROR, "SX wait for idle timed out %08x %08x\n",
		    read_sx_reg(p, SX_CONTROL_STATUS),
		    read_sx_reg(p, SX_ERROR));
	}
}

static void
CG14WaitMarker(ScreenPtr pScreen, int Marker)
{
	ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
	Cg14Ptr p = GET_CG14_FROM_SCRN(pScrn);

	CG14Wait(p);
}

static Bool
CG14PrepareCopy(PixmapPtr pSrcPixmap, PixmapPtr pDstPixmap,
		int xdir, int ydir, int alu, Pixel planemask)
{
	ScrnInfoPtr pScrn = xf86Screens[pDstPixmap->drawable.pScreen->myNum];
	Cg14Ptr p = GET_CG14_FROM_SCRN(pScrn);

	ENTER;
	DPRINTF(X_ERROR, "%s bpp %d rop %x\n", __func__,
	    pSrcPixmap->drawable.bitsPerPixel, alu);

	if (planemask != p->last_mask) {
		CG14Wait(p);
		write_sx_reg(p, SX_PLANEMASK, planemask);
		p->last_mask = planemask;
	}
	alu = sx_rop[alu];
	if (alu != p->last_rop) {
		CG14Wait(p);
		write_sx_reg(p, SX_ROP_CONTROL, alu);
		p->last_rop = alu;
	}
	switch (pSrcPixmap->drawable.bitsPerPixel)  {
		case 8:
			p->pExa->Copy = CG14Copy8;
			break;
		case 32:
			p->pExa->Copy = CG14Copy32;
			break;
		default:
			xf86Msg(X_ERROR, "%s depth %d\n", __func__,
			    pSrcPixmap->drawable.bitsPerPixel);
	}
	p->srcpitch = exaGetPixmapPitch(pSrcPixmap);
	p->srcoff = exaGetPixmapOffset(pSrcPixmap);
	p->xdir = xdir;
	p->ydir = ydir;
	return TRUE;
}

static void
CG14Copy32(PixmapPtr pDstPixmap,
         int srcX, int srcY, int dstX, int dstY, int w, int h)
{
	ScrnInfoPtr pScrn = xf86Screens[pDstPixmap->drawable.pScreen->myNum];
	Cg14Ptr p = GET_CG14_FROM_SCRN(pScrn);
	int dstpitch, dstoff, srcpitch, srcoff;
	int srcstart, dststart, xinc, srcinc, dstinc;
	int line, count, s, d, num;

	ENTER;
	dstpitch = exaGetPixmapPitch(pDstPixmap);
	dstoff = exaGetPixmapOffset(pDstPixmap);
	srcpitch = p->srcpitch;
	srcoff = p->srcoff;
	/*
	 * should clear the WO bit in SX_CONTROL_STATUS, then check if SX
	 * actually wrote anything and only sync if it did
	 */
	srcstart = (srcX << 2) + (srcpitch * srcY) + srcoff;
	dststart = (dstX << 2) + (dstpitch * dstY) + dstoff;

	/*
	 * we always copy up to 32 pixels at a time so direction doesn't
	 * matter if w<=32
	 */
	if (w > 32) {
		if (p->xdir < 0) {
			srcstart += (w - 32) << 2;
			dststart += (w - 32) << 2;
			xinc = -128;
		} else
			xinc = 128;
	} else
		xinc = 128;
	if (p->ydir < 0) {
		srcstart += (h - 1) * srcpitch;
		dststart += (h - 1) * dstpitch;
		srcinc = -srcpitch;
		dstinc = -dstpitch;
	} else {
		srcinc = srcpitch;
		dstinc = dstpitch;
	}
	if (p->last_rop == 0xcc) {
		/* plain old copy */
		if ( xinc > 0) {
			/* going left to right */
			for (line = 0; line < h; line++) {
				count = 0;
				s = srcstart;
				d = dststart;
				while ( count < w) {
					num = min(32, w - count);
					sxm(SX_LD, s, 10, num - 1);
					sxm(SX_STM, d, 10, num - 1);
					s += xinc;
					d += xinc;
					count += 32;
				}
				srcstart += srcinc;
				dststart += dstinc;
			}
		} else {
			/* going right to left */
			int i, chunks = (w >> 5);
			for (line = 0; line < h; line++) {
				s = srcstart;
				d = dststart;
				count = w;
				for (i = 0; i < chunks; i++) {
					sxm(SX_LD, s, 10, 31);
					sxm(SX_STM, d, 10, 31);
					s -= 128;
					d -= 128;
					count -= 32;
				}
				/* leftovers, if any */
				if (count > 0) {
					s += (32 - count) << 2;
					d += (32 - count) << 2;
					sxm(SX_LD, s, 10, count - 1);
					sxm(SX_STM, d, 10, count - 1);
				}
				srcstart += srcinc;
				dststart += dstinc;
			}
		}
	} else {
		/* ROPs needed */
		if ( xinc > 0) {
			/* going left to right */
			for (line = 0; line < h; line++) {
				count = 0;
				s = srcstart;
				d = dststart;
				while ( count < w) {
					num = min(32, w - count);
					sxm(SX_LD, s, 10, num - 1);
					sxm(SX_LD, d, 42, num - 1);
					if (num > 16) {
						sxi(SX_ROP(10, 42, 74, 15));
						sxi(SX_ROP(26, 58, 90, num - 17));
					} else {
						sxi(SX_ROP(10, 42, 74, num - 1));
					}
					sxm(SX_STM, d, 74, num - 1);
					s += xinc;
					d += xinc;
					count += 32;
				}
				srcstart += srcinc;
				dststart += dstinc;
			}
		} else {
			/* going right to left */
			int i, chunks = (w >> 5);
			for (line = 0; line < h; line++) {
				s = srcstart;
				d = dststart;
				count = w;
				for (i = 0; i < chunks; i++) {
					sxm(SX_LD, s, 10, 31);
					sxm(SX_LD, d, 42, 31);
					sxi(SX_ROP(10, 42, 74, 15));
					sxi(SX_ROP(26, 58, 90, 15));
					sxm(SX_STM, d, 74, 31);
					s -= 128;
					d -= 128;
					count -= 32;
				}
				/* leftovers, if any */
				if (count > 0) {
					s += (32 - count) << 2;
					d += (32 - count) << 2;
					sxm(SX_LD, s, 10, count - 1);
					sxm(SX_LD, d, 42, count - 1);
					if (count > 16) {
						sxi(SX_ROP(10, 42, 74, 15));
						sxi(SX_ROP(26, 58, 90, count - 17));
					} else {
						sxi(SX_ROP(10, 42, 74, count - 1));
					}
					sxm(SX_STM, d, 74, count - 1);
				}
				srcstart += srcinc;
				dststart += dstinc;
			}
		}
	}			
	exaMarkSync(pDstPixmap->drawable.pScreen);
}

/*
 * copy with same alignment, left to right, no ROP
 */
static void
CG14Copy8_aligned_norop(Cg14Ptr p, int srcstart, int dststart, int w, int h,
    int srcpitch, int dstpitch)
{
	int saddr, daddr, pre, cnt, wrds;

	ENTER;
	
	pre = srcstart & 3;
	if (pre != 0) pre = 4 - pre;
	pre = min(pre, w);

	while (h > 0) {
		saddr = srcstart;
		daddr = dststart;
		cnt = w;
		if (pre > 0) {
			sxm(SX_LDB, saddr, 8, pre - 1);
			sxm(SX_STB, daddr, 8, pre - 1);
			saddr += pre;
			daddr += pre;
			cnt -= pre;
			if (cnt == 0) goto next;
		}
		while (cnt > 3) {
			wrds = min(32, cnt >> 2);
			sxm(SX_LD, saddr, 8, wrds - 1);
			sxm(SX_ST, daddr, 8, wrds - 1);
			saddr += wrds << 2;
			daddr += wrds << 2;
			cnt -= wrds << 2;
		}
		if (cnt > 0) {
			sxm(SX_LDB, saddr, 8, cnt - 1);
			sxm(SX_STB, daddr, 8, cnt - 1);
		}
next:
		srcstart += srcpitch;
		dststart += dstpitch;
		h--;
	}
}

/*
 * copy with same alignment, left to right, ROP
 */
static void
CG14Copy8_aligned_rop(Cg14Ptr p, int srcstart, int dststart, int w, int h,
    int srcpitch, int dstpitch)
{
	int saddr, daddr, pre, cnt, wrds;

	ENTER;
	
	pre = srcstart & 3;
	if (pre != 0) pre = 4 - pre;
	pre = min(pre, w);

	while (h > 0) {
		saddr = srcstart;
		daddr = dststart;
		cnt = w;
		if (pre > 0) {
			sxm(SX_LDB, saddr, 8, pre - 1);
			sxm(SX_LDB, daddr, 40, pre - 1);
			sxi(SX_ROP(8, 40, 72, pre - 1));
			sxm(SX_STB, daddr, 72, pre - 1);
			saddr += pre;
			daddr += pre;
			cnt -= pre;
			if (cnt == 0) goto next;
		}
		while (cnt > 3) {
			wrds = min(32, cnt >> 2);
			sxm(SX_LD, saddr, 8, wrds - 1);
			sxm(SX_LD, daddr, 40, wrds - 1);
			if (cnt > 16) {
				sxi(SX_ROP(8, 40, 72, 15));
				sxi(SX_ROP(8, 56, 88, wrds - 17));
			} else
				sxi(SX_ROP(8, 40, 72, wrds - 1));
			sxm(SX_ST, daddr, 72, wrds - 1);
			saddr += wrds << 2;
			daddr += wrds << 2;
			cnt -= wrds << 2;
		}
		if (cnt > 0) {
			sxm(SX_LDB, saddr, 8, cnt - 1);
			sxm(SX_LDB, daddr, 40, cnt - 1);
			sxi(SX_ROP(8, 40, 72, cnt - 1));
			sxm(SX_STB, daddr, 72, cnt - 1);
		}
next:
		srcstart += srcpitch;
		dststart += dstpitch;
		h--;
	}
}

/* up to 124 pixels so direction doesn't matter, unaligned, ROP */
static void
CG14Copy8_short_rop(Cg14Ptr p, int srcstart, int dststart, int w, int h, int srcpitch, int dstpitch)
{
	int saddr, daddr, pre, dist, wrds, swrds, spre, sreg, restaddr, post;
	int ssreg;
#ifdef DEBUG
	int taddr = 4 + dstpitch * 50;
#endif
	uint32_t lmask, rmask;
	ENTER;
	
	pre = dststart & 3;
	lmask = 0xffffffff >> pre;
	spre = srcstart & 3;
	/*
	 * make sure we count all the words needed to cover the destination 
	 * line, covering potential partials on both ends
	 */
	wrds = (w + pre + 3) >> 2;
	swrds = (w + spre + 3) >> 2;

	if (spre < pre) {
		dist = 32 - (pre - spre) * 8;
		sreg = 9;
	} else {
		dist = (spre - pre) * 8;
		sreg = 8;
	}

	/*
	 * mask out trailing pixels to avoid partial writes
	 */
	post = (dststart + w) & 3;
	if (post != 0) {
		rmask = ~(0xffffffff >> (post * 8));
		write_sx_reg(p, SX_QUEUED(7), rmask);	
		write_sx_reg(p, SX_QUEUED(6), ~rmask);	
	}

	DPRINTF(X_ERROR, "%s %d %d, %d %d %08x %d %d %d %d %08x\n", __func__,
	    w, h, spre, pre, lmask, dist, sreg, wrds, post, rmask);

	/* mask out the leading pixels in dst by using a mask and ROP */
	if (pre != 0) {
		CG14Wait(p);
		write_sx_reg(p, SX_ROP_CONTROL, (p->last_rop & 0xf0) | 0xa);
		write_sx_reg(p, SX_QUEUED(R_MASK), 0xffffffff);	
	}

	saddr = srcstart & ~3;
	daddr = dststart & ~3;

	while (h > 0) {
		sxm(SX_LD, daddr, 80, wrds - 1);
		sxm(SX_LD, saddr, sreg, swrds - 1);
		if (wrds > 15) {
			if (dist != 0) {
				sxi(SX_FUNNEL_I(8, dist, 40, 15));
				sxi(SX_FUNNEL_I(24, dist, 56, wrds - 16));
				/* shifted source pixels are now at register 40+ */
				ssreg = 40;
			} else ssreg = 8;
			if (pre != 0) {
				/* mask out leading junk */
				write_sx_reg(p, SX_QUEUED(R_MASK), lmask);
				sxi(SX_ROPB(ssreg, 80, 8, 0));
				write_sx_reg(p, SX_QUEUED(R_MASK), 0xffffffff);
				sxi(SX_ROPB(ssreg + 1, 81, 9, 14));	
			} else {
				sxi(SX_ROPB(ssreg, 80, 8, 15));
			}
			sxi(SX_ROPB(ssreg + 16, 96, 24, wrds - 16));
		} else {
			if (dist != 0) {
				sxi(SX_FUNNEL_I(8, dist, 40, wrds));
				ssreg = 40;
			} else ssreg = 8;
			if (pre != 0) {
				/* mask out leading junk */
				write_sx_reg(p, SX_QUEUED(R_MASK), lmask);
				sxi(SX_ROPB(ssreg, 80, 8, 0));
				write_sx_reg(p, SX_QUEUED(R_MASK), 0xffffffff);
				sxi(SX_ROPB(ssreg + 1, 81, 9, wrds));
			} else {
				sxi(SX_ROPB(ssreg, 80, 8, wrds));
			}
		}
		if (post != 0) {
			/*
			 * if the last word to be written out is a partial we 
			 * mask out the leftovers and replace them with
			 * background pixels
			 * we could pull the same ROP * mask trick as we do on
			 * the left end but it's less annoying this way and
			 * the instruction count is the same
			 */
			sxi(SX_ANDS(7 + wrds, 7, 5, 0));
			sxi(SX_ANDS(79 + wrds, 6, 4, 0));
			sxi(SX_ORS(5, 4, 7 + wrds, 0));
		}
#ifdef DEBUG
		sxm(SX_ST, taddr, 40, wrds - 1);
		taddr += dstpitch;
#endif
		sxm(SX_ST, daddr, 8, wrds - 1);
		saddr += srcpitch;
		daddr += dstpitch;
		h--;
	}
}

/* up to 124 pixels so direction doesn't matter, unaligned, straight copy */
static void
CG14Copy8_short_norop(Cg14Ptr p, int srcstart, int dststart, int w, int h,
    int srcpitch, int dstpitch)
{
	int saddr, daddr, pre, dist, wrds, swrds, spre, sreg, restaddr, post;
	int ssreg;
#ifdef DEBUG
	int taddr = 4 + dstpitch * 50;
#endif
	uint32_t lmask, rmask;
	ENTER;
	
	pre = dststart & 3;
	lmask = 0xffffffff >> pre;
	spre = srcstart & 3;
	/*
	 * make sure we count all the words needed to cover the destination 
	 * line, covering potential partials on both ends
	 */
	wrds = (w + pre + 3) >> 2;
	swrds = (w + spre + 3) >> 2;

	if (spre < pre) {
		dist = 32 - (pre - spre) * 8;
		sreg = 9;
	} else {
		dist = (spre - pre) * 8;
		sreg = 8;
	}

	/*
	 * mask out trailing pixels to avoid partial writes
	 */
	post = (dststart + w) & 3;
	if (post != 0) {
		rmask = ~(0xffffffff >> (post * 8));
		write_sx_reg(p, SX_QUEUED(7), rmask);	
		write_sx_reg(p, SX_QUEUED(6), ~rmask);	
	}

	DPRINTF(X_ERROR, "%s %d %d, %d %d %08x %d %d %d %d %08x\n", __func__,
	    w, h, spre, pre, lmask, dist, sreg, wrds, post, rmask);

	/* mask out the leading pixels in dst by using a mask and ROP */
	if (pre != 0) {
		CG14Wait(p);
		write_sx_reg(p, SX_ROP_CONTROL, 0xca);
		write_sx_reg(p, SX_QUEUED(R_MASK), lmask);	
	}

	saddr = srcstart & ~3;
	daddr = dststart & ~3;
	
	while (h > 0) {
		sxm(SX_LD, saddr, sreg, swrds - 1);
		if (wrds > 15) {
			if (dist != 0) {
				sxi(SX_FUNNEL_I(8, dist, 40, 15));
				sxi(SX_FUNNEL_I(24, dist, 56, wrds - 16));
				/* shifted source pixels are now at reg 40+ */
				ssreg = 40;
			} else ssreg = 8;
			if (pre != 0) {
				/* read only the first word */
				sxm(SX_LD, daddr, 80, 0);
				/* mask out leading junk */
				sxi(SX_ROPB(ssreg, 80, ssreg, 0));
			}
		} else {
			if (dist != 0) {
				sxi(SX_FUNNEL_I(8, dist, 40, wrds));
				ssreg = 40;
			} else ssreg = 8;
			if (pre != 0) {
				/* read only the first word */
				sxm(SX_LD, daddr, 80, 0);
				/* mask out leading junk */
				sxi(SX_ROPB(ssreg, 80, ssreg, 0));
			}
		}
		if (post != 0) {
			int laddr = daddr + ((wrds - 1) << 2);
			/*
			 * if the last word to be written out is a partial we 
			 * mask out the leftovers and replace them with
			 * background pixels
			 * we could pull the same ROP * mask trick as we do on
			 * the left end but it's less annoying this way and
			 * the instruction count is the same
			 */
			sxm(SX_LD, laddr, 81, 0);
			sxi(SX_ANDS(ssreg + wrds - 1, 7, 5, 0));
			sxi(SX_ANDS(81, 6, 4, 0));
			sxi(SX_ORS(5, 4, ssreg + wrds - 1, 0));
		}
#ifdef DEBUG
		sxm(SX_ST, taddr, 40, wrds - 1);
		taddr += dstpitch;
#endif
		sxm(SX_ST, daddr, ssreg, wrds - 1);
		saddr += srcpitch;
		daddr += dstpitch;
		h--;
	}
}

static void
CG14Copy8(PixmapPtr pDstPixmap,
         int srcX, int srcY, int dstX, int dstY, int w, int h)
{
	ScrnInfoPtr pScrn = xf86Screens[pDstPixmap->drawable.pScreen->myNum];
	Cg14Ptr p = GET_CG14_FROM_SCRN(pScrn);
	int dstpitch, dstoff, srcpitch, srcoff;
	int srcstart, dststart, xinc, srcinc, dstinc;
	int line, count, s, d, num;

	ENTER;
	dstpitch = exaGetPixmapPitch(pDstPixmap);
	dstoff = exaGetPixmapOffset(pDstPixmap);
	srcpitch = p->srcpitch;
	srcoff = p->srcoff;
	/*
	 * should clear the WO bit in SX_CONTROL_STATUS, then check if SX
	 * actually wrote anything and only sync if it did
	 */
	srcstart = srcX + (srcpitch * srcY) + srcoff;
	dststart = dstX + (dstpitch * dstY) + dstoff;

	if (p->ydir < 0) {
		srcstart += (h - 1) * srcpitch;
		dststart += (h - 1) * dstpitch;
		srcinc = -srcpitch;
		dstinc = -dstpitch;
	} else {
		srcinc = srcpitch;
		dstinc = dstpitch;
	}

	/*
	 * this copies up to 124 pixels wide in one go, so horizontal
	 * direction / overlap don't matter
	 * uses all 32bit accesses and funnel shifter for unaligned copies
	 */
	if ((w < 125) && (w > 8)) {
		switch (p->last_rop) {
			case 0xcc:
				CG14Copy8_short_norop(p,
				    srcstart, dststart, w, h, srcinc, dstinc);
				break;
			default:
				CG14Copy8_short_rop(p,
				    srcstart, dststart, w, h, srcinc, dstinc);
		}
		return;
	}

	/*
	 * only invert x direction if absolutely necessary, it's a pain to
	 * go backwards on SX so avoid as much as possible
	 */
	if ((p->xdir < 0) && (srcoff == dstoff) && (srcY == dstY)) {
		xinc = -32;
	} else
		xinc = 32;

	/*
	 * for aligned copies we can go all 32bit and avoid VRAM reads in the
	 * most common case
	 */
	if (((srcstart & 3) == (dststart & 3)) && (xinc > 0)) {
		switch (p->last_rop) {
			case 0xcc:
				CG14Copy8_aligned_norop(p,
				    srcstart, dststart, w, h, srcinc, dstinc);
				break;
			default:
				CG14Copy8_aligned_rop(p,
				    srcstart, dststart, w, h, srcinc, dstinc);
		}
		return;
	}

	/*
	 * if we make it here we either have something large and unaligned,
	 * something we need to do right to left, or something tiny.
	 * we handle the non-tiny cases by breaking them down into chunks that
	 * Copy8_short_*() can handle, making sure the destinations are 32bit 
	 * aligned whenever possible
	 * since we copy by block, not by line we need to go backwards even if
	 * we don't copy within the same line
	 */
	if (w > 8) {
		int next, wi, end = dststart + w;
		DPRINTF(X_ERROR, "%s %08x %08x %d\n",
		    __func__, srcstart, dststart, w);
		if ((p->xdir < 0) && (srcoff == dstoff)) {		
			srcstart += w;
			next = max((end - 120) & ~3, dststart);
			wi = end - next;
			srcstart -= wi;
			while (wi > 0) {
				DPRINTF(X_ERROR, "%s RL %08x %08x %d\n",
				    __func__, srcstart, next, wi);
				if (p->last_rop == 0xcc) {
					CG14Copy8_short_norop(p, srcstart,
					    next, wi, h, srcinc, dstinc);
				} else
					CG14Copy8_short_rop(p, srcstart,
					    next, wi, h, srcinc, dstinc);
				end = next;
				/*
				 * avoid extremely narrow copies so I don't
				 * have to deal with dangling start and end
				 * pixels in the same word
				 */
				if ((end - dststart) < 140) {
					next = max((end - 80) & ~3, dststart);
				} else {
					next = max((end - 120) & ~3, dststart);
				}
				wi = end - next;
				srcstart -= wi;
			}
		} else {
			next = min(end, (dststart + 124) & ~3);
			wi = next - dststart;
			while (wi > 0) {
				DPRINTF(X_ERROR, "%s LR %08x %08x %d\n",
				    __func__, srcstart, next, wi);
				if (p->last_rop == 0xcc) {
					CG14Copy8_short_norop(p, 
					    srcstart, dststart, wi, h,
					    srcinc, dstinc);
				} else
					CG14Copy8_short_rop(p,
					    srcstart, dststart, wi, h,
					    srcinc, dstinc);
				srcstart += wi;
				dststart = next;
				if ((end - dststart) < 140) {
					next = min(end, (dststart + 84) & ~3);
				} else {
					next = min(end, (dststart + 124) & ~3);
				}
				wi = next - dststart;
			}
		}
		return;
	}
	if (xinc < 0) {
		srcstart += (w - 32);
		dststart += (w - 32);
	}

	DPRINTF(X_ERROR, "%s fallback to byte-wise %d %d\n", __func__, w, h);
	if (p->last_rop == 0xcc) {
		/* plain old copy */
		if ( xinc > 0) {
			/* going left to right */
			for (line = 0; line < h; line++) {
				count = 0;
				s = srcstart;
				d = dststart;
				while ( count < w) {
					num = min(32, w - count);
					sxm(SX_LDB, s, 10, num - 1);
					sxm(SX_STBM, d, 10, num - 1);
					s += xinc;
					d += xinc;
					count += 32;
				}
				srcstart += srcinc;
				dststart += dstinc;
			}
		} else {
			/* going right to left */
			int i, chunks = (w >> 5);
			for (line = 0; line < h; line++) {
				s = srcstart;
				d = dststart;
				count = w;
				for (i = 0; i < chunks; i++) {
					sxm(SX_LDB, s, 10, 31);
					sxm(SX_STBM, d, 10, 31);
					s -= 32;
					d -= 32;
					count -= 32;
				}
				/* leftovers, if any */
				if (count > 0) {
					s += (32 - count);
					d += (32 - count);
					sxm(SX_LDB, s, 10, count - 1);
					sxm(SX_STBM, d, 10, count - 1);
				}
				srcstart += srcinc;
				dststart += dstinc;
			}
		}
	} else {
		/* ROPs needed */
		if ( xinc > 0) {
			/* going left to right */
			for (line = 0; line < h; line++) {
				count = 0;
				s = srcstart;
				d = dststart;
				while ( count < w) {
					num = min(32, w - count);
					sxm(SX_LDB, s, 10, num - 1);
					sxm(SX_LDB, d, 42, num - 1);
					if (num > 16) {
						sxi(SX_ROP(10, 42, 74, 15));
						sxi(SX_ROP(26, 58, 90, num - 17));
					} else {
						sxi(SX_ROP(10, 42, 74, num - 1));
					}
					sxm(SX_STBM, d, 74, num - 1);
					s += xinc;
					d += xinc;
					count += 32;
				}
				srcstart += srcinc;
				dststart += dstinc;
			}
		} else {
			/* going right to left */
			int i, chunks = (w >> 5);
			for (line = 0; line < h; line++) {
				s = srcstart;
				d = dststart;
				count = w;
				for (i = 0; i < chunks; i++) {
					sxm(SX_LDB, s, 10, 31);
					sxm(SX_LDB, d, 42, 31);
					sxi(SX_ROP(10, 42, 74, 15));
					sxi(SX_ROP(26, 58, 90, 15));
					sxm(SX_STBM, d, 74, 31);
					s -= 128;
					d -= 128;
					count -= 32;
				}
				/* leftovers, if any */
				if (count > 0) {
					s += (32 - count);
					d += (32 - count);
					sxm(SX_LDB, s, 10, count - 1);
					sxm(SX_LDB, d, 42, count - 1);
					if (count > 16) {
						sxi(SX_ROP(10, 42, 74, 15));
						sxi(SX_ROP(26, 58, 90, count - 17));
					} else {
						sxi(SX_ROP(10, 42, 74, count - 1));
					}
					sxm(SX_STBM, d, 74, count - 1);
				}
				srcstart += srcinc;
				dststart += dstinc;
			}
		}
	}			
	exaMarkSync(pDstPixmap->drawable.pScreen);
}

static void
CG14DoneCopy(PixmapPtr pDstPixmap)
{
}

static Bool
CG14PrepareSolid(PixmapPtr pPixmap, int alu, Pixel planemask, Pixel fg)
{
	ScrnInfoPtr pScrn = xf86Screens[pPixmap->drawable.pScreen->myNum];
	Cg14Ptr p = GET_CG14_FROM_SCRN(pScrn);

	ENTER;
	DPRINTF(X_ERROR, "bits per pixel: %d %08lx\n",
	    pPixmap->drawable.bitsPerPixel, fg);

	/*
	 * GXset and GXclear are really just specual cases of GXcopy with
	 * fixed fill colour
	 */
	switch (alu) {
		case GXclear:
			alu = GXcopy;
			fg = 0;
			break;
		case GXset:
			alu = GXcopy;
			fg = 0xffffffff;
			break;
	}
	/* repeat the colour in every sub byte if we're in 8 bit */
	if (pPixmap->drawable.bitsPerPixel == 8) {
		fg |= fg << 8;
		fg |= fg << 16;
	}
	write_sx_reg(p, SX_QUEUED(8), fg);
	write_sx_reg(p, SX_QUEUED(9), fg);
	if (planemask != p->last_mask) {
		CG14Wait(p);
		write_sx_reg(p, SX_PLANEMASK, planemask);
		p->last_mask = planemask;
	}
	alu = sx_rop[alu];
	if (alu != p->last_rop) {
		CG14Wait(p);
		write_sx_reg(p, SX_ROP_CONTROL, alu);
		p->last_rop = alu;
	}

	DPRINTF(X_ERROR, "%s: %x\n", __func__, alu);
	return TRUE;
}

static void
CG14Solid32(Cg14Ptr p, uint32_t start, uint32_t pitch, int w, int h)
{
	int line, x, num;
	uint32_t ptr;

	ENTER;
	if (p->last_rop == 0xcc) {
		/* simple fill */
		for (line = 0; line < h; line++) {
			x = 0;
			while (x < w) {
				ptr = start + (x << 2);
				num = min(32, w - x);
				sxm(SX_STS, ptr, 8, num - 1);
				x += 32;
			}
			start += pitch;
		}
	} else if (p->last_rop == 0xaa) {
		/* nothing to do here */
		return;
	} else {
		/* alright, let's do actual ROP stuff */

		/* first repeat the fill colour into 16 registers */
		sxi(SX_SELECT_S(8, 8, 10, 15));

		for (line = 0; line < h; line++) {
			x = 0;
			while (x < w) {
				ptr = start + (x << 2);
				num = min(32, w - x);
				/* now suck fb data into registers */
				sxm(SX_LD, ptr, 42, num - 1);
				/*
				 * ROP them with the fill data we left in 10
				 * non-memory ops can only have counts up to 16
				 */
				if (num <= 16) {
					sxi(SX_ROP(10, 42, 74, num - 1));
				} else {
					sxi(SX_ROP(10, 42, 74, 15));
					sxi(SX_ROP(10, 58, 90, num - 17));
				}
				/* and write the result back into memory */
				sxm(SX_ST, ptr, 74, num - 1);
				x += 32;
			}
			start += pitch;
		}
	}
}

static void
CG14Solid8(Cg14Ptr p, uint32_t start, uint32_t pitch, int w, int h)
{
	int line, num, pre, cnt;
	uint32_t ptr;

	ENTER;
	pre = start & 3;
	if (pre != 0) pre = 4 - pre;

	if (p->last_rop == 0xcc) {
		/* simple fill */
		for (line = 0; line < h; line++) {
			ptr = start;
			cnt = w;
			pre = min(pre, cnt);
			if (pre) {
				sxm(SX_STBS, ptr, 8, pre - 1);
				ptr += pre;
				cnt -= pre;
				if (cnt == 0) goto next;
			}
			/* now do the aligned pixels in 32bit chunks */
			if (ptr & 3) xf86Msg(X_ERROR, "%s %x\n", __func__, ptr);
			while(cnt > 3) {
				num = min(32, cnt >> 2);
				sxm(SX_STS, ptr, 8, num - 1);
				ptr += num << 2;
				cnt -= num << 2;
			}
			if (cnt > 3) xf86Msg(X_ERROR, "%s cnt %d\n", __func__, cnt);
			if (cnt > 0) {
				sxm(SX_STBS, ptr, 8, cnt - 1);
			}
			if ((ptr + cnt) != (start + w)) xf86Msg(X_ERROR, "%s %x vs %x\n", __func__, ptr + cnt, start + w);
next:
			start += pitch;
		}
	} else if (p->last_rop == 0xaa) {
		/* nothing to do here */
		return;
	} else {
		/* alright, let's do actual ROP stuff */

		/* first repeat the fill colour into 16 registers */
		sxi(SX_SELECT_S(8, 8, 10, 15));

		for (line = 0; line < h; line++) {
			ptr = start;
			cnt = w;
			pre = min(pre, cnt);
			if (pre) {
				sxm(SX_LDB, ptr, 26, pre - 1);
				sxi(SX_ROP(10, 26, 42, pre - 1));
				sxm(SX_STB, ptr, 42, pre - 1);
				ptr += pre;
				cnt -= pre;
				if (cnt == 0) goto next2;
			}
			/* now do the aligned pixels in 32bit chunks */
			if (ptr & 3) xf86Msg(X_ERROR, "%s %x\n", __func__, ptr);
			while(cnt > 3) {
				num = min(32, cnt >> 2);
				sxm(SX_LD, ptr, 26, num - 1);
				if (num <= 16) {
					sxi(SX_ROP(10, 26, 58, num - 1));
				} else {
					sxi(SX_ROP(10, 26, 58, 15));
					sxi(SX_ROP(10, 42, 74, num - 17));
				}
				sxm(SX_ST, ptr, 58, num - 1);
				ptr += num << 2;
				cnt -= num << 2;
			}
			if (cnt > 3) xf86Msg(X_ERROR, "%s cnt %d\n", __func__, cnt);
			if (cnt > 0) {
				sxm(SX_LDB, ptr, 26, cnt - 1);
				sxi(SX_ROP(10, 26, 42, cnt - 1));
				sxm(SX_STB, ptr, 42, cnt - 1);
			}
			if ((ptr + cnt) != (start + w)) xf86Msg(X_ERROR, "%s %x vs %x\n", __func__, ptr + cnt, start + w);
next2:
			start += pitch;
		}
	}
}

static void
CG14Solid(PixmapPtr pPixmap, int x1, int y1, int x2, int y2)
{
	ScrnInfoPtr pScrn = xf86Screens[pPixmap->drawable.pScreen->myNum];
	Cg14Ptr p = GET_CG14_FROM_SCRN(pScrn);
	int w = x2 - x1, h = y2 - y1, dstoff, dstpitch;
	int start, depth;

	ENTER;
	dstpitch = exaGetPixmapPitch(pPixmap);
	dstoff = exaGetPixmapOffset(pPixmap);

	depth = pPixmap->drawable.bitsPerPixel;
	switch (depth) {
		case 32:
			start = dstoff + (y1 * dstpitch) + (x1 << 2);
			CG14Solid32(p, start, dstpitch, w, h);
			break;
		case 8:
			start = dstoff + (y1 * dstpitch) + x1;
			CG14Solid8(p, start, dstpitch, w, h);
			break;
	}

	DPRINTF(X_ERROR, "Solid %d %d %d %d, %d %d -> %d\n", x1, y1, x2, y2,
	    dstpitch, dstoff, start);
	DPRINTF(X_ERROR, "%x %x %x\n", p->last_rop,
	    read_sx_reg(p, SX_QUEUED(8)), read_sx_reg(p, SX_QUEUED(9)));
	exaMarkSync(pPixmap->drawable.pScreen);
}

/*
 * Memcpy-based UTS.
 */
static Bool
CG14UploadToScreen(PixmapPtr pDst, int x, int y, int w, int h,
    char *src, int src_pitch)
{
	ScrnInfoPtr pScrn = xf86Screens[pDst->drawable.pScreen->myNum];
	Cg14Ptr p = GET_CG14_FROM_SCRN(pScrn);
	char  *dst        = p->fb + exaGetPixmapOffset(pDst);
	int    dst_pitch  = exaGetPixmapPitch(pDst);

	int bpp    = pDst->drawable.bitsPerPixel;
	int cpp    = (bpp + 7) >> 3;
	int wBytes = w * cpp;

	ENTER;
	DPRINTF(X_ERROR, "%s depth %d\n", __func__, bpp);
	dst += (x * cpp) + (y * dst_pitch);

	CG14Wait(p);

	while (h--) {
		memcpy(dst, src, wBytes);
		src += src_pitch;
		dst += dst_pitch;
	}
	__asm("stbar;");
	return TRUE;
}

/*
 * Memcpy-based DFS.
 */
static Bool
CG14DownloadFromScreen(PixmapPtr pSrc, int x, int y, int w, int h,
    char *dst, int dst_pitch)
{
	ScrnInfoPtr pScrn = xf86Screens[pSrc->drawable.pScreen->myNum];
	Cg14Ptr p = GET_CG14_FROM_SCRN(pScrn);
	char  *src        = p->fb + exaGetPixmapOffset(pSrc);
	int    src_pitch  = exaGetPixmapPitch(pSrc);

	ENTER;
	int bpp    = pSrc->drawable.bitsPerPixel;
	int cpp    = (bpp + 7) >> 3;
	int wBytes = w * cpp;

	src += (x * cpp) + (y * src_pitch);

	CG14Wait(p);

	while (h--) {
		memcpy(dst, src, wBytes);
		src += src_pitch;
		dst += dst_pitch;
	}

	return TRUE;
}

Bool
CG14CheckComposite(int op, PicturePtr pSrcPicture,
                           PicturePtr pMaskPicture,
                           PicturePtr pDstPicture)
{
	int i, ok = FALSE;

	ENTER;

	/*
	 * SX is in theory capable of accelerating pretty much all Xrender ops,
	 * even coordinate transformation and gradients. Support will be added
	 * over time and likely have to spill over into its own source file.
	 */
	
	if ((op != PictOpOver) && (op != PictOpAdd) && (op != PictOpSrc)) {
		DPRINTF(X_ERROR, "%s: rejecting %d\n", __func__, op);
		return FALSE;
	}

	if (pSrcPicture != NULL) {
		i = 0;
		while ((i < arraysize(src_formats)) && (!ok)) {
			ok =  (pSrcPicture->format == src_formats[i]);
			i++;
		}

		if (!ok) {
			DPRINTF(X_ERROR, "%s: unsupported src format %x\n",
			    __func__, pSrcPicture->format);
			return FALSE;
		}
		DPRINTF(X_ERROR, "src is %x, %d\n", pSrcPicture->format, op);
	}

	if (pDstPicture != NULL) {
		i = 0;
		ok = FALSE;
		while ((i < arraysize(src_formats)) && (!ok)) {
			ok =  (pDstPicture->format == src_formats[i]);
			i++;
		}

		if (!ok) {
			DPRINTF(X_ERROR, "%s: unsupported dst format %x\n",
			    __func__, pDstPicture->format);
			return FALSE;
		}
		DPRINTF(X_ERROR, "dst is %x, %d\n", pDstPicture->format, op);
	}

	if (pMaskPicture != NULL) {
		DPRINTF(X_ERROR, "mask is %x %d %d\n", pMaskPicture->format,
		    pMaskPicture->pDrawable->width,
		    pMaskPicture->pDrawable->height);
	}
	return TRUE;
}

Bool
CG14PrepareComposite(int op, PicturePtr pSrcPicture,
                             PicturePtr pMaskPicture,
                             PicturePtr pDstPicture,
                             PixmapPtr  pSrc,
                             PixmapPtr  pMask,
                             PixmapPtr  pDst)
{
	ScrnInfoPtr pScrn = xf86Screens[pDst->drawable.pScreen->myNum];
	Cg14Ptr p = GET_CG14_FROM_SCRN(pScrn);

	ENTER;

	p->no_source_pixmap = FALSE;
	p->source_is_solid = FALSE;

	if (pSrcPicture->format == PICT_a1) {
		xf86Msg(X_ERROR, "src mono, dst %x, op %d\n",
		    pDstPicture->format, op);
		if (pMaskPicture != NULL) {
			xf86Msg(X_ERROR, "msk %x\n", pMaskPicture->format);
		}
	}
	if (pSrcPicture->pSourcePict != NULL) {
		if (pSrcPicture->pSourcePict->type == SourcePictTypeSolidFill) {
			p->fillcolour =
			    pSrcPicture->pSourcePict->solidFill.color;
			DPRINTF(X_ERROR, "%s: solid src %08x\n",
			    __func__, p->fillcolour);
			p->no_source_pixmap = TRUE;
			p->source_is_solid = TRUE;
		}
	}
	if ((pMaskPicture != NULL) && (pMaskPicture->pSourcePict != NULL)) {
		if (pMaskPicture->pSourcePict->type ==
		    SourcePictTypeSolidFill) {
			p->fillcolour = 
			   pMaskPicture->pSourcePict->solidFill.color;
			xf86Msg(X_ERROR, "%s: solid mask %08x\n",
			    __func__, p->fillcolour);
		}
	}
	if (pMaskPicture != NULL) {
		p->mskoff = exaGetPixmapOffset(pMask);
		p->mskpitch = exaGetPixmapPitch(pMask);
		p->mskformat = pMaskPicture->format;
	} else {
		p->mskoff = 0;
		p->mskpitch = 0;
		p->mskformat = 0;
	}
	if (pSrc != NULL) {
		p->source_is_solid = 
		   ((pSrc->drawable.width == 1) && (pSrc->drawable.height == 1));
		p->srcoff = exaGetPixmapOffset(pSrc);
		p->srcpitch = exaGetPixmapPitch(pSrc);
		if (p->source_is_solid) {
			p->fillcolour = *(uint32_t *)(p->fb + p->srcoff);
		}
	}
	p->srcformat = pSrcPicture->format;
	p->dstformat = pDstPicture->format;
	
	if (p->source_is_solid) {
		uint32_t temp;

		/* stuff source colour into SX registers, swap as needed */
		temp = p->fillcolour;
		switch (p->srcformat) {
			case PICT_a8r8g8b8:
			case PICT_x8r8g8b8:
				write_sx_reg(p, SX_QUEUED(9), temp & 0xff);
				temp = temp >> 8;
				write_sx_reg(p, SX_QUEUED(10), temp & 0xff);
				temp = temp >> 8;
				write_sx_reg(p, SX_QUEUED(11), temp & 0xff);
				break;
			case PICT_a8b8g8r8:
			case PICT_x8b8g8r8:
				write_sx_reg(p, SX_QUEUED(11), temp & 0xff);
				temp = temp >> 8;
				write_sx_reg(p, SX_QUEUED(10), temp & 0xff);
				temp = temp >> 8;
				write_sx_reg(p, SX_QUEUED(9), temp & 0xff);
				break;
		}
		write_sx_reg(p, SX_QUEUED(8), 0xff);
	}
	p->op = op;
	if (op == PictOpSrc) {
		CG14PrepareCopy(pSrc, pDst, 1, 1, GXcopy, 0xffffffff);
	}
#ifdef SX_DEBUG
	DPRINTF(X_ERROR, "%x %x -> %x\n", p->srcoff, p->mskoff,
	    *(uint32_t *)(p->fb + p->srcoff));	
#endif
	return TRUE;
}

void
CG14Composite(PixmapPtr pDst, int srcX, int srcY,
                              int maskX, int maskY,
                              int dstX, int dstY,
                              int width, int height)
{
	ScrnInfoPtr pScrn = xf86Screens[pDst->drawable.pScreen->myNum];
	Cg14Ptr p = GET_CG14_FROM_SCRN(pScrn);
	uint32_t dstoff, dstpitch;
	uint32_t dst, msk, src;
	int flip = 0;

	ENTER;
	dstoff = exaGetPixmapOffset(pDst);		
	dstpitch = exaGetPixmapPitch(pDst);

	flip = (PICT_FORMAT_TYPE(p->srcformat) !=
		PICT_FORMAT_TYPE(p->dstformat));

	switch (p->op) {
		case PictOpOver:
			dst = dstoff + (dstY * dstpitch) + (dstX << 2);
			DPRINTF(X_ERROR, "Over %08x %08x, %d %d\n",
			    p->mskformat, p->dstformat, srcX, srcY);
			if (p->source_is_solid) {
				switch (p->mskformat) {
					case PICT_a8:
						msk = p->mskoff + 
						    (maskY * p->mskpitch) +
						    maskX;
						CG14Comp_Over8Solid(p,
						    msk, p->mskpitch,
						    dst, dstpitch,
						    width, height);
						break;
					case PICT_a8r8g8b8:
					case PICT_a8b8g8r8:
						msk = p->mskoff + 
						    (maskY * p->mskpitch) + 
						    (maskX << 2);
						CG14Comp_Over32Solid(p,
						    msk, p->mskpitch,
						    dst, dstpitch,
						    width, height);
						break;
					default:
						xf86Msg(X_ERROR,
						  "unsupported mask format %08x\n", p->mskformat);
				}
			} else {
				DPRINTF(X_ERROR, "non-solid over with msk %x\n",
				    p->mskformat);
				switch (p->srcformat) {
					case PICT_a8r8g8b8:
					case PICT_a8b8g8r8:
						src = p->srcoff +
						    (srcY * p->srcpitch) +
						    (srcX << 2);
						dst = dstoff +
						    (dstY * dstpitch) +
						    (dstX << 2);
						if (p->mskformat == PICT_a8) {
							msk = p->mskoff + 
							    (maskY * p->mskpitch) +
							    maskX;	
							CG14Comp_Over32Mask(p, 
							    src, p->srcpitch,
							    msk, p->mskpitch,
							    dst, dstpitch,
							    width, height, flip);
						} else {
							CG14Comp_Over32(p, 
							    src, p->srcpitch,
							    dst, dstpitch,
							    width, height, flip);
						}
						break;
					case PICT_x8r8g8b8:
					case PICT_x8b8g8r8:
						src = p->srcoff +
						    (srcY * p->srcpitch) +
						    (srcX << 2);
						dst = dstoff +
						    (dstY * dstpitch) +
						    (dstX << 2);
						if (p->mskformat == PICT_a8) {
							msk = p->mskoff + 
							    (maskY * p->mskpitch) +
							    maskX;	
							CG14Comp_Over32Mask_noalpha(p, 
							    src, p->srcpitch,
							    msk, p->mskpitch,
							    dst, dstpitch,
							    width, height, flip);
						} else if ((p->mskformat == PICT_a8r8g8b8) ||
							   (p->mskformat == PICT_a8b8g8r8)) {
							msk = p->mskoff + 
							    (maskY * p->mskpitch) +
							    (maskX << 2);	
							CG14Comp_Over32Mask32_noalpha(p, 
							    src, p->srcpitch,
							    msk, p->mskpitch,
							    dst, dstpitch,
							    width, height, flip);
						} else {
							xf86Msg(X_ERROR, "no src alpha, mask is %x\n", p->mskformat);
						}
						break;
					default:
						xf86Msg(X_ERROR, "%s: format %x in non-solid Over op\n",
						    __func__, p->srcformat);
				}
			}
			break;
		case PictOpAdd:
			DPRINTF(X_ERROR, "Add %08x %08x\n",
			    p->srcformat, p->dstformat);
			switch (p->srcformat) {
				case PICT_a8:
					src = p->srcoff +
					    (srcY * p->srcpitch) + srcX;
					if (p->dstformat == PICT_a8) {
						dst = dstoff + 
						      (dstY * dstpitch) + dstX;
						CG14Comp_Add8(p,
						    src, p->srcpitch,
						    dst, dstpitch,
						    width, height);
					} else {
						dst = dstoff + 
						      (dstY * dstpitch) +
						      (dstX << 2);
						CG14Comp_Add8_32(p,
						    src, p->srcpitch,
						    dst, dstpitch,
						    width, height);
					}
					break;
				case PICT_a8r8g8b8:
				case PICT_x8r8g8b8:
					src = p->srcoff +
					    (srcY * p->srcpitch) + (srcX << 2);
					dst = dstoff + (dstY * dstpitch) +
					    (dstX << 2);
					CG14Comp_Add32(p, src, p->srcpitch,
					    dst, dstpitch, width, height);
					break;
				default:
					xf86Msg(X_ERROR,
					    "unsupported src format\n");
			}
			break;
		case PictOpSrc:
			DPRINTF(X_ERROR, "Src %08x %08x\n",
			    p->srcformat, p->dstformat);
			if (p->mskformat != 0)
				xf86Msg(X_ERROR, "Src mask %08x\n", p->mskformat);
			if (p->srcformat == PICT_a8) {
				CG14Copy8(pDst, srcX, srcY, dstX, dstY, width, height);
			} else {
				/* convert between RGB and BGR? */
				CG14Copy32(pDst, srcX, srcY, dstX, dstY, width, height);
			}
			break;
		default:
			xf86Msg(X_ERROR, "unsupported op %d\n", p->op);
	}
	exaMarkSync(pDst->drawable.pScreen);
}



Bool
CG14InitAccel(ScreenPtr pScreen)
{
	ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
	Cg14Ptr p = GET_CG14_FROM_SCRN(pScrn);
	ExaDriverPtr pExa;

	pExa = exaDriverAlloc();
	if (!pExa)
		return FALSE;

	p->pExa = pExa;

	pExa->exa_major = EXA_VERSION_MAJOR;
	pExa->exa_minor = EXA_VERSION_MINOR;

	pExa->memoryBase = p->fb;
	pExa->memorySize = p->memsize;
	pExa->offScreenBase = p->width * p->height * (pScrn->depth >> 3);

	/*
	 * SX memory instructions are written to 64bit aligned addresses with
	 * a 3 bit displacement. Make sure the displacement remains constant
	 * within one column
	 */
	
	pExa->pixmapOffsetAlign = 8;
	pExa->pixmapPitchAlign = 8;

	pExa->flags = EXA_OFFSCREEN_PIXMAPS
		      | EXA_SUPPORTS_OFFSCREEN_OVERLAPS
		      /*| EXA_MIXED_PIXMAPS*/;

	/*
	 * these limits are bogus
	 * SX doesn't deal with coordinates at all, so there is no limit but
	 * we have to put something here
	 */
	pExa->maxX = 4096;
	pExa->maxY = 4096;

	pExa->WaitMarker = CG14WaitMarker;

	pExa->PrepareSolid = CG14PrepareSolid;
	pExa->Solid = CG14Solid;
	pExa->DoneSolid = CG14DoneCopy;
	pExa->PrepareCopy = CG14PrepareCopy;
	pExa->Copy = CG14Copy32;
	pExa->DoneCopy = CG14DoneCopy;
	if (p->use_xrender) {
		pExa->CheckComposite = CG14CheckComposite;
		pExa->PrepareComposite = CG14PrepareComposite;
		pExa->Composite = CG14Composite;
		pExa->DoneComposite = CG14DoneCopy;
	}

	/* EXA hits more optimized paths when it does not have to fallback 
	 * because of missing UTS/DFS, hook memcpy-based UTS/DFS.
	 */
	pExa->UploadToScreen = CG14UploadToScreen;
	pExa->DownloadFromScreen = CG14DownloadFromScreen;

	p->queuecount = 0;
	/* do some hardware init */
	write_sx_reg(p, SX_PLANEMASK, 0xffffffff);
	p->last_mask = 0xffffffff;
	write_sx_reg(p, SX_ROP_CONTROL, 0xcc);
	p->last_rop = 0xcc;
	return exaDriverInit(pScreen, pExa);
}

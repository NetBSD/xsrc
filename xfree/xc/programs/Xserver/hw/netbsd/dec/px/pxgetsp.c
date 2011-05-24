/*	$NetBSD: pxgetsp.c,v 1.3 2011/05/24 23:12:36 jakllsch Exp $	*/

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

#include "Xmd.h"
#include "servermd.h"
#include "gc.h"
#include "pixmapstr.h"
#include "scrnintstr.h"
#include "cfb.h"

#if 0
#undef cfbGetSpans
#endif

#define myBitsPerPixel(d) (\
    (1 << PixmapWidthPaddingInfo[d].padBytesLog2) * 8 / \
    (PixmapWidthPaddingInfo[d].padRoundUp+1))

/*
 * Read spans from a drawable and pack them into a buffer.  For non-WINDOW
 * type drawables, we punt and use the mfb/cfb routines.  For WINDOW
 * drawables we have the PixelStamp reading from the screen into buffer X,
 * while we copy from buffer Y to the caller's buffer (where buffer Y has
 * previously been filled with data from the screen).
 */
void
pxGetSpans(DrawablePtr pDrawable, int wMax, DDXPointPtr ppt, int *pwidth,
	   int nspans, char *pdst)
{
	pxScreenPrivPtr sp;
	DDXPointPtr pptLast;
	u_int32_t *pb, *p, *psp;
	pxPacket pxp; 
	pxImgBufPtr ib;
	int w, pw;

	if (pDrawable->type != DRAWABLE_WINDOW) {
		switch (myBitsPerPixel(pDrawable->depth)) {
		case 1:
			mfbGetSpans(pDrawable, wMax, ppt, pwidth, nspans,
			    pdst);
			break;
		case 8:
			cfbGetSpans(pDrawable, wMax, ppt, pwidth, nspans,
			    pdst);
			break;
		case 32:
			cfb32GetSpans(pDrawable, wMax, ppt, pwidth, nspans,
			    pdst);
			break;
		default:
			FatalError("pxGetSpans(): bad depth");
		}
		return;
	}

	sp = pDrawable->pScreen->devPrivates[pxScreenPrivateIndex].ptr;
	pptLast = ppt + nspans;
	psp = NULL;

	while (ppt < pptLast) {
		w = min(ppt->x + *pwidth, 1280) - ppt->x;

		ib = pxAllocImgBuf(sp);
		pb = pxPacketStart(sp, &pxp, 4, 4);

		pb[0] = STAMP_CMD_READSPANS;
		pb[1] = (1 << 24) | 0x00ffffff;
		pb[2] = 0;
		pb[3] = STAMP_METHOD_NOOP | STAMP_UPDATE_ENABLE | STAMP_SPAN;
		pb[4] = ib->paddr;
		pb[5] = w << 3;
		pb[6] = (ppt->x << 19) | (ppt->y << 3);
		pb[7] = 0;

		pxAssociateImgBuf(sp, ib, &pxp);
		pxPacketFlush(sp, &pxp);

		if (psp != NULL)
			pdst = (*sp->compressBuf)(pdst, psp, pw);

		psp = ib->ptr;
		pw = w;

		ppt++;
		pwidth++;
	}

	pxPacketWait(sp, &pxp);
	(*sp->compressBuf)(pdst, psp, pw);
}

void
pxGetScanlineRaw(pxScreenPrivPtr sp, int x, int y, int w, pxImgBufPtr ib)
{
	u_int32_t *pb;
	pxPacket pxp;

	pb = pxPacketStart(sp, &pxp, 4, 4);

	pb[0] = STAMP_CMD_READSPANS;
	pb[1] = (1 << 24) | 0x00ffffff;
	pb[2] = 0;
	pb[3] = STAMP_METHOD_NOOP | STAMP_UPDATE_ENABLE | STAMP_SPAN;
	pb[4] = ib->paddr;
	pb[5] = w << 3;
	pb[6] = (x << 19) | (y << 3);
	pb[7] = 0;

	pxAssociateImgBuf(sp, ib, &pxp);
	pxPacketFlushWait(sp, &pxp);
}

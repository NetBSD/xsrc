/*	$NetBSD: pxpolypnt.c,v 1.2 2008/04/28 20:57:37 martin Exp $	*/

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

#include "cfb.h"

void
pxPolyPoint(DrawablePtr pDrawable, GCPtr pGC, int mode, int npt,
	    xPoint *pptInit)
{
	pxScreenPrivPtr sp;
	pxPrivGCPtr gcPriv;
	xPoint *ppt, *maxppt;
	u_int32_t *pb;
	pxPacket pxp;
	RegionPtr prgnClip;
	BoxPtr pbox, pmaxbox;
	int psy, xorg, yorg;

	PX_TRACE("pxPolyPoint");

	gcPriv = pxGetGCPrivate(pGC);
	prgnClip = cfbGetCompositeClip(pGC);
	sp = gcPriv->sp;

	xorg = pDrawable->x;
	yorg = pDrawable->y;
	ppt = pptInit;

	if (mode == CoordModePrevious) {
		maxppt = ppt + npt -1;
		while (ppt < maxppt) {
			ppt++;
			ppt->x = ppt->x + (ppt-1)->x + xorg;
			ppt->y = ppt->y + (ppt-1)->y + yorg;
		}
	} else if (xorg != 0 || yorg != 0) {
		maxppt = ppt + npt -1;
		for (; ppt < maxppt; ppt++) {
			ppt->x = ppt->x + xorg;
			ppt->y = ppt->y + yorg;
		}
	}

	pb = pxPacketStart(sp, &pxp, 6, 2);
	pb[0] = STAMP_CMD_LINES | STAMP_RGB_CONST | STAMP_LW_PERPACKET;
	pb[1] = gcPriv->pmask;
	pb[2] = 0;
	pb[3] = gcPriv->umet;
	pb[4] = (1 << 2) - 1;
	pb[5] = gcPriv->fgPixel;

	pbox = REGION_RECTS(prgnClip);
	pmaxbox = pbox + REGION_NUM_RECTS(prgnClip);
	maxppt = pptInit + npt;

	for (; pbox < pmaxbox; pbox++) {
		for (ppt = pptInit; ppt < maxppt; ppt++) {
			if (ppt->x < pbox->x1 || ppt->x >= pbox->x2 ||
			    ppt->y < pbox->y1 || ppt->y >= pbox->y2)
			    	continue;

			pb = pxPacketAddPrim(sp, &pxp);
			psy = (ppt->y << 3) + (1 << 2) - 1;
			pb[0] = (ppt->x << 19) | psy;
			pb[1] = ((ppt->x + 1) << 19) | psy;
		}
	}

	pxPacketFlush(sp, &pxp);
}

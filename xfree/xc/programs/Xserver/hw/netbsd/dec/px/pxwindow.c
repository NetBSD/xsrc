/*	$NetBSD: pxwindow.c,v 1.4 2008/04/28 20:57:37 martin Exp $	*/

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

/* from XConsortium: cfbwindow.c,v 5.22 94/04/17 20:29:07 dpw Exp */
/***********************************************************

Copyright (c) 1987  X Consortium

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of the X Consortium shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from the X Consortium.


Copyright 1987 by Digital Equipment Corporation, Maynard, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the name of Digital not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/

#include "px.h"

#include "Xmd.h"
#include "Xproto.h"
#include "cfb.h"
#include "fontstruct.h"
#include "dixfontstr.h"
#include "pixmapstr.h"
#include "scrnintstr.h"
#include "region.h"

#include "mistruct.h"
#include "mibstore.h"
#include "migc.h"

void 
pxCopyWindow(WindowPtr pWin, DDXPointRec ptOldOrg, RegionPtr prgnSrc)
{
	pxScreenPrivPtr sp;
	DDXPointPtr pptSrc;
	DDXPointPtr ppt;
	RegionRec rgnDst;
	BoxPtr pbox;
	int dx, dy, i, nbox;
	WindowPtr pwinRoot;
	extern WindowPtr *WindowTable;

	PX_TRACE("pxCopyWindow");

	sp = pWin->drawable.pScreen->devPrivates[pxScreenPrivateIndex].ptr;
	pwinRoot = WindowTable[pWin->drawable.pScreen->myNum];

	REGION_NULL(pWin->drawable.pScreen, &rgnDst);

	dx = ptOldOrg.x - pWin->drawable.x;
	dy = ptOldOrg.y - pWin->drawable.y;
	REGION_TRANSLATE(pWin->drawable.pScreen, prgnSrc, -dx, -dy);
	REGION_INTERSECT(pWin->drawable.pScreen, &rgnDst, &pWin->borderClip,
	    prgnSrc);

	pbox = REGION_RECTS(&rgnDst);
	nbox = REGION_NUM_RECTS(&rgnDst);
	i = nbox * sizeof(*ppt);

	if (!nbox || !(pptSrc = (DDXPointPtr )ALLOCATE_LOCAL(i))) {
		REGION_UNINIT(pWin->drawable.pScreen, &rgnDst);
		return;
	}
	ppt = pptSrc;

	for (i = nbox; i > 0; ppt++, pbox++, i--) {
		ppt->x = pbox->x1 + dx;
		ppt->y = pbox->y1 + dy;
	}

	pxDoBitblt((DrawablePtr)pwinRoot, (DrawablePtr)pwinRoot,
	    GXcopy, &rgnDst, pptSrc, ~0L);

	DEALLOCATE_LOCAL(pptSrc);
	REGION_UNINIT(pWin->drawable.pScreen, &rgnDst);
}

void
pxPaintWindowBackground(WindowPtr pWin, RegionPtr pRegion, int what)
{
	pxScreenPrivPtr sp;
	pxPrivWinPtr winPriv;

	PX_TRACE("pxPaintWindowBackground");

	sp = pWin->drawable.pScreen->devPrivates[pxScreenPrivateIndex].ptr;

	switch (pWin->backgroundState) {
	case BackgroundPixmap:
		winPriv = pxGetWindowPrivate(pWin);
		if (winPriv->haveMask)
			pxFillBoxTiled(sp, pRegion, &winPriv->mask);
		else
			miPaintWindow(pWin, pRegion, what);
		break;

	case BackgroundPixel:
		pxFillBoxSolid(sp, pRegion, pWin->background.pixel);
		break;

	case ParentRelative:
		do {
			pWin = pWin->parent;
		} while (pWin->backgroundState == ParentRelative);
		(*pWin->drawable.pScreen->PaintWindowBackground)(pWin,
		    pRegion, what);
		break;

	case None:
		break;

	default:
		miPaintWindow(pWin, pRegion, what);
		break;
	}
}

void
pxPaintWindowBorder(WindowPtr pWin, RegionPtr pRegion, int what)
{
	pxScreenPrivPtr sp;

	sp = pWin->drawable.pScreen->devPrivates[pxScreenPrivateIndex].ptr;

	if (pWin->borderIsPixel)
		pxFillBoxSolid(sp, pRegion, pWin->border.pixel);
	else
		miPaintWindow(pWin, pRegion, what);
}

Bool
pxCreateWindow(WindowPtr pWin)
{

	PX_TRACE("pxCreateWindow");

	pxGetWindowPrivate(pWin)->haveMask = 0;
	return (TRUE);	
}

Bool
pxDestroyWindow(WindowPtr pWin)
{

	PX_TRACE("pxDestroyWindow");

	return (TRUE);
}

Bool
pxPositionWindow(WindowPtr pWin, int x, int y)
{

	PX_TRACE("pxPositionWindow");

	return (TRUE);
}

Bool
pxChangeWindowAttributes(WindowPtr pWin, unsigned long mask)
{
	pxScreenPrivPtr sp;
	pxPrivWinPtr winPriv;

	PX_TRACE("pxChangeWindowAttributes");

	sp = pWin->drawable.pScreen->devPrivates[pxScreenPrivateIndex].ptr;

	winPriv = pxGetWindowPrivate(pWin);

	if (pWin->backgroundState != BackgroundPixmap) {
		winPriv->haveMask = 0;
		return (TRUE);
	}

	if ((mask & CWBackPixmap) == 0)
		return (TRUE);

	winPriv->haveMask = pxMaskFromTile(sp, pWin->background.pixmap,
	    &winPriv->mask);

	return (TRUE);
}

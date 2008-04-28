/*	$NetBSD: pxbresd.c,v 1.2 2008/04/28 20:57:37 martin Exp $	*/

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

/***********************************************************
Copyright 1987 by Digital Equipment Corporation, Maynard, Massachusetts,
and the Massachusetts Institute of Technology, Cambridge, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the names of Digital or MIT not be
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
/* from XConsortium: cfbbresd.c,v 1.11 91/12/26 14:32:45 rws Exp */

#include "px.h"

#include "miline.h"

#define BresStep(minor, major)				\
	if ((e += e1) >= 0) {				\
		e += e3;				\
		minor;					\
	}						\
	major;

#define Loop()						\
	while (thisDash--) {				\
		if(axis == Y_AXIS) {			\
			BresStep(x1+=signdx,y1+=signdy) \
		} else {				\
			BresStep(y1+=signdy,x1+=signdx) \
		}					\
	}

void
pxBresD(pxScreenPrivPtr sp, pxPacketPtr pp, pxPrivGCPtr gcPriv,
	int *pdashIndex, unsigned char *pDash, int numInDashList,
	int *pdashOffset, int isDoubleDash, int signdx, int signdy,
	int axis, int x1, int y1, int e, int e1, int e2, int len)
{
	int e3, dashIndex, dashOffset, dashRemaining, thisDash;
	int xstart, ystart;

	dashOffset = *pdashOffset;
	dashIndex = *pdashIndex;
	dashRemaining = pDash[dashIndex] - dashOffset;
	if ((thisDash = dashRemaining) >= len) {
		thisDash = len;
		dashRemaining -= len;
	}
	e3 = e2 - e1;
	e = e - e1;			/* to make looping easier */

	PX_TRACE("pxBresD");

	/* point to first point */
	for (;;) { 
		len -= thisDash;
		if ((dashIndex & 1) != 0) {
			if (isDoubleDash) {
				xstart = x1;
				ystart = y1;
				Loop()
				pxAddLineC(sp, pp, xstart, ystart, x1, y1,
				    gcPriv->bgPixel);
			} else {
				Loop()
			}
		} else {
			xstart = x1;
			ystart = y1;
			Loop()
			pxAddLineC(sp, pp, xstart, ystart, x1, y1,
				gcPriv->fgFill);
		}
		if (!len)
			break;

		dashIndex++;
		if (dashIndex == numInDashList)
			dashIndex = 0;
		dashRemaining = pDash[dashIndex];
		if ((thisDash = dashRemaining) >= len) {
			dashRemaining -= len;
			thisDash = len;
		}
	}

	*pdashIndex = dashIndex;
	*pdashOffset = pDash[dashIndex] - dashRemaining;
}

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
/* $XFree86: xc/programs/Xserver/hw/tinyx/trident/tridentcurs.c,v 1.1 2004/06/02 22:43:02 dawes Exp $ */
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

#include "trident.h"
#include "cursorstr.h"

#define SetupCursor(s)	    KdScreenPriv(s); \
			    tridentCardInfo(pScreenPriv); \
			    tridentScreenInfo(pScreenPriv); \
			    TridentCursor *pCurPriv = &tridents->cursor

static void
_tridentMoveCursor (ScreenPtr pScreen, int x, int y)
{
    SetupCursor(pScreen);
    CARD8   xlow, xhigh, ylow, yhigh;
    CARD8   xoff, yoff;

    x -= pCurPriv->xhot;
    xoff = 0;
    if (x < 0)
    {
	xoff = -x;
	x = 0;
    }
    y -= pCurPriv->yhot;
    yoff = 0;
    if (y < 0)
    {
	yoff = -y;
	y = 0;
    }
    xlow = (CARD8) x;
    xhigh = (CARD8) (x >> 8);
    ylow = (CARD8) y;
    yhigh = (CARD8) (y >> 8);
    
    
    /* This is the recommended order to move the cursor */
    
    tridentWriteIndex (tridentc, 0x3d4, 0x41, xhigh);
    tridentWriteIndex (tridentc, 0x3d4, 0x40, xlow);
    tridentWriteIndex (tridentc, 0x3d4, 0x42, ylow);
    tridentWriteIndex (tridentc, 0x3d4, 0x46, xoff);
    tridentWriteIndex (tridentc, 0x3d4, 0x47, yoff);
    tridentWriteIndex (tridentc, 0x3d4, 0x43, yhigh);
}

static void
tridentMoveCursor (ScreenPtr pScreen, int x, int y)
{
    SetupCursor (pScreen);
    
    (void)tridentc;

    if (!pCurPriv->has_cursor)
	return;
    
    if (!pScreenPriv->enabled)
	return;
    
    _tridentMoveCursor (pScreen, x, y);
}

static void
tridentAllocCursorColors (ScreenPtr pScreen)
{
    SetupCursor (pScreen);
    CursorPtr	    pCursor = pCurPriv->pCursor;

    (void)tridentc;

    KdAllocateCursorPixels (pScreen, 0, pCursor, 
			    &pCurPriv->source, &pCurPriv->mask);
    switch (pScreenPriv->screen->fb[0].bitsPerPixel) {
    case 4:
	pCurPriv->source |= pCurPriv->source << 4;
	pCurPriv->mask |= pCurPriv->mask << 4;
    case 8:
	pCurPriv->source |= pCurPriv->source << 8;
	pCurPriv->mask |= pCurPriv->mask << 8;
    case 16:
	pCurPriv->source |= pCurPriv->source << 16;
	pCurPriv->mask |= pCurPriv->mask << 16;
    }
}

static void
tridentSetCursorColors (ScreenPtr pScreen)
{
    SetupCursor (pScreen);
    CARD32	fg, bg;
    
    fg = pCurPriv->source;
    bg = pCurPriv->mask;
    tridentWriteIndex (tridentc, 0x3d4, 0x48, fg);
    tridentWriteIndex (tridentc, 0x3d4, 0x49, fg >> 8);
    tridentWriteIndex (tridentc, 0x3d4, 0x4a, fg >> 16);
    
    tridentWriteIndex (tridentc, 0x3d4, 0x4c, bg);
    tridentWriteIndex (tridentc, 0x3d4, 0x4d, bg >> 8);
    tridentWriteIndex (tridentc, 0x3d4, 0x4e, bg >> 16);
}
    
void
tridentRecolorCursor (ScreenPtr pScreen, int ndef, xColorItem *pdef)
{
    SetupCursor (pScreen);
    CursorPtr	    pCursor = pCurPriv->pCursor;

    (void)tridentc;

    if (!pCurPriv->has_cursor || !pCursor)
	return;
    
    if (!pScreenPriv->enabled)
	return;
    
    if (pdef)
    {
	while (ndef)
	{
	    if (pdef->pixel == pCurPriv->source || 
		pdef->pixel == pCurPriv->mask)
		break;
	    ndef--;
	}
	if (!ndef)
	    return;
    }
    tridentAllocCursorColors (pScreen);
    tridentSetCursorColors (pScreen);
}
    
#define InvertBits32(v) { \
    v = ((v & 0x55555555) << 1) | ((v >> 1) & 0x55555555); \
    v = ((v & 0x33333333) << 2) | ((v >> 2) & 0x33333333); \
    v = ((v & 0x0f0f0f0f) << 4) | ((v >> 4) & 0x0f0f0f0f); \
}

static void
tridentLoadCursor (ScreenPtr pScreen, int x, int y)
{
    SetupCursor(pScreen);
    CursorPtr	    pCursor = pCurPriv->pCursor;
    CursorBitsPtr   bits = pCursor->bits;
    int	h;
    CARD32	    *ram, *msk, *mskLine, *src, *srcLine;
    int		    i, j;
    int		    lwsrc;
    CARD32	    offset;

    /*
     * Allocate new colors
     */
    tridentAllocCursorColors (pScreen);
    
    pCurPriv->pCursor = pCursor;
    pCurPriv->xhot = pCursor->bits->xhot;
    pCurPriv->yhot = pCursor->bits->yhot;
    
    /*
     * Stick new image into cursor memory
     */
    ram = (CARD32 *) tridents->cursor_base;
    mskLine = (CARD32 *) bits->mask;
    srcLine = (CARD32 *) bits->source;

    h = bits->height;
    if (h > TRIDENT_CURSOR_HEIGHT)
	h = TRIDENT_CURSOR_HEIGHT;

    lwsrc = BitmapBytePad(bits->width) / 4;        /* words per line */

    for (i = 0; i < TRIDENT_CURSOR_HEIGHT; i++) {
	msk = mskLine;
	src = srcLine;
	mskLine += lwsrc;
	srcLine += lwsrc;
	for (j = 0; j < TRIDENT_CURSOR_WIDTH / 32; j++) {

	    CARD32  m, s;

#if 1
	    if (i < h && j < lwsrc) 
	    {
		m = *msk++;
		s = *src++;
		InvertBits32(m);
		InvertBits32(s);
	    }
	    else
	    {
		m = 0;
		s = 0;
	    }
#endif
	    *ram++ = m;
	    *ram++ = s;
	}
    }
    
    /* Set address for cursor bits */
    offset = tridents->cursor_base - (CARD8 *) tridents->screen;
    offset >>= 10;
    tridentWriteIndex (tridentc, 0x3d4, 0x44, (CARD8) (offset & 0xff));
    tridentWriteIndex (tridentc, 0x3d4, 0x45, (CARD8) (offset >> 8));
    
    /* Set new color */
    tridentSetCursorColors (pScreen);
     
    /* Enable the cursor */
    tridentWriteIndex (tridentc, 0x3d4, 0x50, 0xc1);
    
    /* Move to new position */
    tridentMoveCursor (pScreen, x, y);
}

static void
tridentUnloadCursor (ScreenPtr pScreen)
{
    SetupCursor (pScreen);
    
    (void)pCurPriv;

    /* Disable cursor */
    tridentWriteIndex (tridentc, 0x3d4, 0x50, 0);
}

static Bool
tridentRealizeCursor (ScreenPtr pScreen, CursorPtr pCursor)
{
    SetupCursor(pScreen);

    (void)tridentc;

    if (!pScreenPriv->enabled)
	return TRUE;
    
    /* miRecolorCursor does this */
    if (pCurPriv->pCursor == pCursor)
    {
	if (pCursor)
	{
	    int	    x, y;
	    
	    miPointerPosition (&x, &y);
	    tridentLoadCursor (pScreen, x, y);
	}
    }
    return TRUE;
}

static Bool
tridentUnrealizeCursor (ScreenPtr pScreen, CursorPtr pCursor)
{
    return TRUE;
}

static void
tridentSetCursor (ScreenPtr pScreen, CursorPtr pCursor, int x, int y)
{
    SetupCursor(pScreen);

    (void)tridentc;

    pCurPriv->pCursor = pCursor;
    
    if (!pScreenPriv->enabled)
	return;
    
    if (pCursor)
	tridentLoadCursor (pScreen, x, y);
    else
	tridentUnloadCursor (pScreen);
}

miPointerSpriteFuncRec tridentPointerSpriteFuncs = {
    tridentRealizeCursor,
    tridentUnrealizeCursor,
    tridentSetCursor,
    tridentMoveCursor,
};

static void
tridentQueryBestSize (int class, 
		 unsigned short *pwidth, unsigned short *pheight, 
		 ScreenPtr pScreen)
{
    SetupCursor (pScreen);

    (void)tridentc;

    switch (class)
    {
    case CursorShape:
	if (*pwidth > pCurPriv->width)
	    *pwidth = pCurPriv->width;
	if (*pheight > pCurPriv->height)
	    *pheight = pCurPriv->height;
	if (*pwidth > pScreen->width)
	    *pwidth = pScreen->width;
	if (*pheight > pScreen->height)
	    *pheight = pScreen->height;
	break;
    default:
	fbQueryBestSize (class, pwidth, pheight, pScreen);
	break;
    }
}

Bool
tridentCursorInit (ScreenPtr pScreen)
{
    SetupCursor (pScreen);

    (void)tridentc;

    if (!tridents->cursor_base)
    {
	pCurPriv->has_cursor = FALSE;
	return FALSE;
    }
    
    pCurPriv->width = TRIDENT_CURSOR_WIDTH;
    pCurPriv->height= TRIDENT_CURSOR_HEIGHT;
    pScreen->QueryBestSize = tridentQueryBestSize;
    miPointerInitialize (pScreen,
			 &tridentPointerSpriteFuncs,
			 &kdPointerScreenFuncs,
			 FALSE);
    pCurPriv->has_cursor = TRUE;
    pCurPriv->pCursor = NULL;
    return TRUE;
}

void
tridentCursorEnable (ScreenPtr pScreen)
{
    SetupCursor (pScreen);

    (void)tridentc;

    if (pCurPriv->has_cursor)
    {
	if (pCurPriv->pCursor)
	{
	    int	    x, y;
	    
	    miPointerPosition (&x, &y);
	    tridentLoadCursor (pScreen, x, y);
	}
	else
	    tridentUnloadCursor (pScreen);
    }
}

void
tridentCursorDisable (ScreenPtr pScreen)
{
    SetupCursor (pScreen);

    (void)tridentc;

    if (!pScreenPriv->enabled)
	return;
    
    if (pCurPriv->has_cursor)
    {
	if (pCurPriv->pCursor)
	{
	    tridentUnloadCursor (pScreen);
	}
    }
}

void
tridentCursorFini (ScreenPtr pScreen)
{
    SetupCursor (pScreen);

    (void)tridentc;

    pCurPriv->pCursor = NULL;
}

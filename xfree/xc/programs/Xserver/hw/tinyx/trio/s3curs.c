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
/* $XFree86: xc/programs/Xserver/hw/tinyx/trio/s3curs.c,v 1.1 2004/06/02 22:43:03 dawes Exp $ */
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

#include "s3.h"
#include "s3draw.h"
#include "cursorstr.h"

#define SetupCursor(s)	    KdScreenPriv(s); \
			    s3CardInfo(pScreenPriv); \
			    s3ScreenInfo(pScreenPriv); \
			    S3Ptr s3 = s3c->s3; \
			    S3Cursor *pCurPriv = &s3s->cursor

static void
_s3MoveCursor (ScreenPtr pScreen, int x, int y)
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
    
    _s3WriteIndexRegister (&s3->crt_vga_3d4, 0x46, xhigh);
    _s3WriteIndexRegister (&s3->crt_vga_3d4, 0x47, xlow);
    _s3WriteIndexRegister (&s3->crt_vga_3d4, 0x49, ylow);
    _s3WriteIndexRegister (&s3->crt_vga_3d4, 0x4e, xoff);
    _s3WriteIndexRegister (&s3->crt_vga_3d4, 0x4f, yoff);
    _s3WriteIndexRegister (&s3->crt_vga_3d4, 0x48, yhigh);
}    

static void
s3MoveCursor (ScreenPtr pScreen, int x, int y)
{
    SetupCursor (pScreen);
    
    if (!pCurPriv->has_cursor)
	return;
    
    if (!pScreenPriv->enabled)
	return;
    
    LockS3(s3c);
    _s3UnlockExt(s3);
    _s3MoveCursor (pScreen, x, y);
    _s3LockExt(s3);
    UnlockS3(s3c);
}

static void
s3AllocCursorColors (ScreenPtr pScreen)
{
    SetupCursor (pScreen);
    CursorPtr	    pCursor = pCurPriv->pCursor;

    (void)s3;

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
_s3SetCursorColors (ScreenPtr pScreen)
{
    SetupCursor (pScreen);
    /* set foreground */
    /* Reset cursor color stack pointers */
    (void) _s3ReadIndexRegister(&s3->crt_vga_3d4, 0x45);
    _s3WriteIndexRegister (&s3->crt_vga_3d4, 0x4a, pCurPriv->source);
    _s3WriteIndexRegister (&s3->crt_vga_3d4, 0x4a, pCurPriv->source >> 8);
    _s3WriteIndexRegister (&s3->crt_vga_3d4, 0x4a, pCurPriv->source >> 16);
	
    /* set background */
    /* Reset cursor color stack pointers */
    (void) _s3ReadIndexRegister(&s3->crt_vga_3d4, 0x45);
    _s3WriteIndexRegister (&s3->crt_vga_3d4, 0x4b, pCurPriv->mask);
    _s3WriteIndexRegister (&s3->crt_vga_3d4, 0x4b, pCurPriv->mask >> 8);
    _s3WriteIndexRegister (&s3->crt_vga_3d4, 0x4b, pCurPriv->mask >> 16);
}
    
void
s3RecolorCursor (ScreenPtr pScreen, int ndef, xColorItem *pdef)
{
    SetupCursor (pScreen);
    CursorPtr	    pCursor = pCurPriv->pCursor;

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
    s3AllocCursorColors (pScreen);
    LockS3 (s3c);
    _s3UnlockExt(s3);
    _s3SetCursorColors (pScreen);
    _s3LockExt (s3);
    UnlockS3 (s3c);
}
    
static void
s3LoadCursor (ScreenPtr pScreen, int x, int y)
{
    SetupCursor(pScreen);
    CursorPtr	    pCursor = pCurPriv->pCursor;
    CursorBitsPtr   bits = pCursor->bits;
    int h;
    unsigned long   *ram;
    unsigned long   *msk, *mskLine, *src, *srcLine;
    unsigned long   and, xor;
    int		    i, j;
    int		    wsrc;

    /*
     * Allocate new colors
     */
    s3AllocCursorColors (pScreen);
    
    pCurPriv->pCursor = pCursor;
    pCurPriv->xhot = pCursor->bits->xhot;
    pCurPriv->yhot = pCursor->bits->yhot;
    
    /*
     * Stick new image into cursor memory
     */
    ram = (unsigned long *) s3s->cursor_base;
    mskLine = (unsigned long *) bits->mask;
    srcLine = (unsigned long *) bits->source;

    h = bits->height;
    if (h > S3_CURSOR_HEIGHT)
	h = S3_CURSOR_HEIGHT;

    wsrc = BitmapBytePad(bits->width) / 4;        /* ulongs per line */

    for (i = 0; i < S3_CURSOR_HEIGHT; i++) 
    {
	msk = mskLine;
	src = srcLine;
	mskLine += wsrc;
	srcLine += wsrc;
	for (j = 0; j < S3_CURSOR_WIDTH / 32; j++) {

	    unsigned long  m, s;

	    if (i < h && j < wsrc) 
	    {
		m = *msk++;
		s = *src++;
		xor = m & s;
		and = ~m;
	    }
	    else
	    {
		and = 0xffffffff;
		xor = 0x00000000;
	    }
		
	    S3AdjustBits32(and);
	    S3AdjustBits32(xor);
	    *ram++ = (and & 0xffff) | (xor << 16);
	    *ram++ = (and >> 16) | (xor & 0xffff0000);
	}
    }
    
    _s3WaitIdle (s3);
    _s3UnlockExt (s3);
    
    /* Set new color */
    _s3SetCursorColors (pScreen);
     
    /* Enable the cursor */
    _s3WriteIndexRegister (&s3->crt_vga_3d4, 0x45, 0x01);
    
    /* Wait for VRetrace to make sure the position is read */
    _s3WaitVRetrace (s3);
    
    /* Move to new position */
    _s3MoveCursor (pScreen, x, y);
    
    _s3LockExt (s3);
    UnlockS3(s3c);
}

static void
s3UnloadCursor (ScreenPtr pScreen)
{
    SetupCursor (pScreen);

    (void)pCurPriv;

    LockS3 (s3c);
    _s3UnlockExt(s3);

    /* Disable cursor */
    _s3WriteIndexRegister (&s3->crt_vga_3d4, 0x45, 0);
    
    _s3LockExt(s3);
    UnlockS3 (s3c);
}

static Bool
s3RealizeCursor (ScreenPtr pScreen, CursorPtr pCursor)
{
    SetupCursor(pScreen);

    (void)s3;

    if (!pScreenPriv->enabled)
	return TRUE;
    
    /* miRecolorCursor does this */
    if (pCurPriv->pCursor == pCursor)
    {
	if (pCursor)
	{
#ifdef FB_OLD_SCREEN
	    short	x, y;
#else
	    int		x, y;
#endif
	    
	    miPointerPosition (&x, &y);
	    s3LoadCursor (pScreen, x, y);
	}
    }
    return TRUE;
}

static Bool
s3UnrealizeCursor (ScreenPtr pScreen, CursorPtr pCursor)
{
    return TRUE;
}

static void
s3SetCursor (ScreenPtr pScreen, CursorPtr pCursor, int x, int y)
{
    SetupCursor(pScreen);

    (void)s3;

    pCurPriv->pCursor = pCursor;
    
    if (!pScreenPriv->enabled)
	return;
    
    if (pCursor)
	s3LoadCursor (pScreen, x, y);
    else
	s3UnloadCursor (pScreen);
}

miPointerSpriteFuncRec s3PointerSpriteFuncs = {
    s3RealizeCursor,
    s3UnrealizeCursor,
    s3SetCursor,
    s3MoveCursor,
};

static void
s3QueryBestSize (int class, 
		 unsigned short *pwidth, unsigned short *pheight, 
		 ScreenPtr pScreen)
{
    SetupCursor (pScreen);

    (void)s3;

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
s3CursorInit (ScreenPtr pScreen)
{
    SetupCursor (pScreen);

    (void)s3;

    if (!s3s->cursor_base)
    {
	pCurPriv->has_cursor = FALSE;
	return FALSE;
    }
    
    pCurPriv->width = S3_CURSOR_WIDTH;
    pCurPriv->height= S3_CURSOR_HEIGHT;
    pScreen->QueryBestSize = s3QueryBestSize;
    miPointerInitialize (pScreen,
			 &s3PointerSpriteFuncs,
			 &kdPointerScreenFuncs,
			 FALSE);
    pCurPriv->has_cursor = TRUE;
    pCurPriv->pCursor = NULL;
    return TRUE;
}

void
s3CursorEnable (ScreenPtr pScreen)
{
    SetupCursor (pScreen);

    (void)s3;

    if (pCurPriv->has_cursor)
    {
	if (pCurPriv->pCursor)
	{
#ifdef FB_OLD_SCREEN
	    short	x, y;
#else
	    int		x, y;
#endif
	    
	    miPointerPosition (&x, &y);
	    s3LoadCursor (pScreen, x, y);
	}
	else
	    s3UnloadCursor (pScreen);
    }
}

void
s3CursorDisable (ScreenPtr pScreen)
{
    SetupCursor (pScreen);

    (void)s3;

    if (!pScreenPriv->enabled)
	return;
    
    if (pCurPriv->has_cursor)
    {
	if (pCurPriv->pCursor)
	{
	    s3UnloadCursor (pScreen);
	}
    }
}

void
s3CursorFini (ScreenPtr pScreen)
{
    SetupCursor (pScreen);

    (void)s3;

    pCurPriv->pCursor = NULL;
}

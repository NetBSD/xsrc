/*
 * $XFree86: xc/programs/Xserver/hw/tinyx/igs/igscurs.c,v 1.1 2004/06/02 22:43:01 dawes Exp $
 *
 * Copyright © 2000 Keith Packard
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

#include "igs.h"
#include "cursorstr.h"

#define SetupCursor(s)	    KdScreenPriv(s); \
			    igsCardInfo(pScreenPriv); \
			    igsScreenInfo(pScreenPriv); \
			    IgsCursor *pCurPriv = &igss->cursor; \
			    IgsVga    *igsvga = &igsc->igsvga

static void
_igsMoveCursor (ScreenPtr pScreen, int x, int y)
{
    SetupCursor(pScreen);
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

    igsSet (igsvga, igs_sprite_x, x);
    igsSet (igsvga, igs_sprite_preset_x, xoff);
    igsSet (igsvga, igs_sprite_y, y);
    igsSet (igsvga, igs_sprite_preset_y, yoff);
}

static void
igsMoveCursor (ScreenPtr pScreen, int x, int y)
{
    SetupCursor (pScreen);
    
    if (!pCurPriv->has_cursor)
	return;
    
    if (!pScreenPriv->enabled)
	return;
    
    _igsMoveCursor (pScreen, x, y);
    VgaFlush (&igsvga->card);
}


static void
igsSetCursorColors (ScreenPtr pScreen)
{
    SetupCursor (pScreen);
    CursorPtr	pCursor = pCurPriv->pCursor;
    
    igsSetImm (igsvga, igs_cursor_write_index, 0);
    igsSetImm (igsvga, igs_cursor_data, pCursor->backRed >> 8);
    igsSetImm (igsvga, igs_cursor_data, pCursor->backGreen >> 8);
    igsSetImm (igsvga, igs_cursor_data, pCursor->backBlue >> 8);
    igsSetImm (igsvga, igs_cursor_write_index, 1);
    igsSetImm (igsvga, igs_cursor_data, pCursor->foreRed >> 8);
    igsSetImm (igsvga, igs_cursor_data, pCursor->foreGreen >> 8);
    igsSetImm (igsvga, igs_cursor_data, pCursor->foreBlue >> 8);
}
    
#if BITMAP_BIT_ORDER == MSBFirst
#define IgsAdjustCursor(v) { \
    v = ((v & 0x55555555) << 1) | ((v >> 1) & 0x55555555); \
    v = ((v & 0x33333333) << 2) | ((v >> 2) & 0x33333333); \
    v = ((v & 0x0f0f0f0f) << 4) | ((v >> 4) & 0x0f0f0f0f); \
    v = ((v & 0x00ff00ff) << 8) | ((v >> 8) & 0x00ff00ff); \
    v = ((v & 0x0000ffff) <<16) | ((v >>16) & 0x0000ffff); \
}
#else
#define IgsAdjustCursor(v)
#endif

#define ExplodeBits2(v)	    (((v) & 1) | (((v) & 2) << 1))
#define ExplodeBits4(v)     ((ExplodeBits2((v) >> 2) << 4) | \
			     (ExplodeBits2((v) & 0x3)))
#define ExplodeBits8(v)	    ((ExplodeBits4((v) >> 4) << 8) | \
			     (ExplodeBits4((v) & 0xf)))
#define ExplodeBits16(v)    ((ExplodeBits8((v) >> 8) << 16) | \
			     (ExplodeBits8((v) & 0xff)))
static void
igsLoadCursor (ScreenPtr pScreen, int x, int y)
{
    SetupCursor(pScreen);
    CursorPtr	    pCursor = pCurPriv->pCursor;
    CursorBitsPtr   bits = pCursor->bits;
    int	h;
    CARD32	    *ram, *msk, *mskLine, *src, *srcLine;
    int		    i, j;
    int		    lwsrc;
    CARD32	    offset;
    CARD32	    b0, b1;

    pCurPriv->pCursor = pCursor;
    pCurPriv->xhot = pCursor->bits->xhot;
    pCurPriv->yhot = pCursor->bits->yhot;
    
    /*
     * Stick new image into cursor memory
     */
    ram = (CARD32 *) igss->cursor_base;
    mskLine = (CARD32 *) bits->mask;
    srcLine = (CARD32 *) bits->source;

    h = bits->height;
    if (h > IGS_CURSOR_HEIGHT)
	h = IGS_CURSOR_HEIGHT;

    lwsrc = BitmapBytePad(bits->width) / 4;        /* words per line */

    for (i = 0; i < IGS_CURSOR_HEIGHT; i++) {
	msk = mskLine;
	src = srcLine;
	mskLine += lwsrc;
	srcLine += lwsrc;
	for (j = 0; j < IGS_CURSOR_WIDTH / 32; j++) {

	    CARD32  m, s;

	    if (i < h && j < lwsrc) 
	    {
		m = *msk++;
		s = *src++;
		IgsAdjustCursor(m);
		IgsAdjustCursor(s);
	    }
	    else
	    {
		m = 0;
		s = 0;
	    }
	    s &= m;
	    m = ~m;
	    b0 = ExplodeBits16(s&0xffff) | (ExplodeBits16(m&0xffff)<<1);
	    b1 = ExplodeBits16(s>>16) | (ExplodeBits16(m>>16)<<1);
	    *ram++ = b0;
	    *ram++ = b1;
	}
    }
    
    /* Set new color */
    igsSetCursorColors (pScreen);
     
    /* Set address for cursor bits */
    offset = igss->cursor_offset;
    offset >>= 10;
    igsSet (igsvga, igs_sprite_addr, offset);
    
    /* Assume TV interpolation off */
    igsSet (igsvga, igs_hcshf, 3);
    /* Enable the cursor */
    igsSet (igsvga, igs_sprite_visible, 1);
    igsSet (igsvga, igs_sprite_64x64, 1);
    /* Move to new position */
    _igsMoveCursor (pScreen, x, y);

    VgaFlush (&igsvga->card);
}

static void
igsUnloadCursor (ScreenPtr pScreen)
{
    SetupCursor (pScreen);

    (void)pCurPriv;

    /* Disable cursor */
    igsSet (igsvga, igs_sprite_visible, 0);
    VgaFlush (&igsvga->card);
}

static Bool
igsRealizeCursor (ScreenPtr pScreen, CursorPtr pCursor)
{
    SetupCursor(pScreen);

    (void)igsvga;

    if (!pScreenPriv->enabled)
	return TRUE;
    
    /* miRecolorCursor does this */
    if (pCurPriv->pCursor == pCursor)
    {
	if (pCursor)
	{
	    int	    x, y;
	    
	    miPointerPosition (&x, &y);
	    igsLoadCursor (pScreen, x, y);
	}
    }
    return TRUE;
}

static Bool
igsUnrealizeCursor (ScreenPtr pScreen, CursorPtr pCursor)
{
    return TRUE;
}

static void
igsSetCursor (ScreenPtr pScreen, CursorPtr pCursor, int x, int y)
{
    SetupCursor(pScreen);

    (void)igsvga;

    pCurPriv->pCursor = pCursor;
    
    if (!pScreenPriv->enabled)
	return;
    
    if (pCursor)
	igsLoadCursor (pScreen, x, y);
    else
	igsUnloadCursor (pScreen);
}

miPointerSpriteFuncRec igsPointerSpriteFuncs = {
    igsRealizeCursor,
    igsUnrealizeCursor,
    igsSetCursor,
    igsMoveCursor,
};

static void
igsQueryBestSize (int class, 
		 unsigned short *pwidth, unsigned short *pheight, 
		 ScreenPtr pScreen)
{
    SetupCursor (pScreen);

    (void)igsvga;

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
igsCursorInit (ScreenPtr pScreen)
{
    SetupCursor (pScreen);

    (void)igsvga;

    if (!igss->cursor_base)
    {
	pCurPriv->has_cursor = FALSE;
	return FALSE;
    }
    
    pCurPriv->width = IGS_CURSOR_WIDTH;
    pCurPriv->height= IGS_CURSOR_HEIGHT;
    pScreen->QueryBestSize = igsQueryBestSize;
    miPointerInitialize (pScreen,
			 &igsPointerSpriteFuncs,
			 &kdPointerScreenFuncs,
			 FALSE);
    pCurPriv->has_cursor = TRUE;
    pCurPriv->pCursor = NULL;
    return TRUE;
}

void
igsCursorEnable (ScreenPtr pScreen)
{
    SetupCursor (pScreen);

    (void)igsvga;

    if (pCurPriv->has_cursor)
    {
	if (pCurPriv->pCursor)
	{
	    int	    x, y;
	    
	    miPointerPosition (&x, &y);
	    igsLoadCursor (pScreen, x, y);
	}
	else
	    igsUnloadCursor (pScreen);
    }
}

void
igsCursorDisable (ScreenPtr pScreen)
{
    SetupCursor (pScreen);

    (void)igsvga;

    if (!pScreenPriv->enabled)
	return;
    
    if (pCurPriv->has_cursor)
    {
	if (pCurPriv->pCursor)
	{
	    igsUnloadCursor (pScreen);
	}
    }
}

void
igsCursorFini (ScreenPtr pScreen)
{
    SetupCursor (pScreen);

    (void)igsvga;

    pCurPriv->pCursor = NULL;
}

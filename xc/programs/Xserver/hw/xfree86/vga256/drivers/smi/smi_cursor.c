/******************************************************************************\

				   Copyright (c) 1999 by Silicon Motion, Inc.
							   All Rights Reserved

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that the
above copyright notice appear in all copies and that both that copyright notice
and this permission notice appear in supporting documentation, and that the name
of Silicon Motion, Inc. not be used in advertising or publicity pertaining to
distribution of the software without specific, written prior permission.
Silicon Motion, Inc. and its suppliers make no representations about the
suitability of this software for any purpose.  It is provided "as is" without
express or implied warranty.

SILICON MOTION, INC. DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT
SHALL SILICON MOTION, INC. AND/OR ITS SUPPLIERS BE LIABLE FOR ANY SPECIAL,
INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF
THIS SOFTWARE.
\******************************************************************************/
/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/smi/smi_cursor.c,v 1.1.2.2 1999/12/11 17:43:20 hohndel Exp $ */

#include "X.h"
#include "Xproto.h"
#include "misc.h"
#include "input.h"
#include "cursorstr.h"
#include "regionstr.h"
#include "scrnintstr.h"
#include "servermd.h"
#include "windowstr.h"

#include "compiler.h"
#include "xf86.h"
#include "xf86_OSlib.h"
#include "mipointer.h"
#include "inputstr.h"
#include "xf86Priv.h"
#include "xf86_Option.h"
#include "vga256.h"
#include "vga.h"
#include "xf86xaa.h"
#include "regsmi.h"
#include "smi_driver.h"

static Bool SMIRealizeCursor(ScreenPtr, CursorPtr);
static Bool SMIUnrealizeCursor(ScreenPtr, CursorPtr);
static void SMISetCursor(ScreenPtr, CursorPtr, int, int);
static void SMIMoveCursor(ScreenPtr, int, int);
static void SMIRecolorCursor(ScreenPtr, CursorPtr, Bool);

static miPointerSpriteFuncRec smiPointerSpriteFuncs =
{
   SMIRealizeCursor,
   SMIUnrealizeCursor,
   SMISetCursor,
   SMIMoveCursor,
};

extern miPointerScreenFuncRec xf86PointerScreenFuncs;
extern xf86InfoRec xf86Info;

/* For byte swapping, use the XAA array */
extern unsigned char byte_reversed[256];
extern SMIPRIV smiPriv;

static int smiCursGeneration = -1;
static int smiHotX;
static int smiHotY;
static int smiCursorMemory;
static CursorPtr smiCursorpCurs;

#define MAX_CURS 32

Bool
SMICursorInit(char *pm, ScreenPtr pScr)
{
	if (smiCursGeneration != serverGeneration)
	{
		if (!(miPointerInitialize(pScr, &smiPointerSpriteFuncs,
				&xf86PointerScreenFuncs, FALSE)))
			return(FALSE);

		smiHotX = 0;
		smiHotY = 0;
		pScr->RecolorCursor = SMIRecolorCursor;
		smiCursGeneration = serverGeneration;
	}
	smiCursorMemory = vga256InfoRec.videoRam * 1024 - 2048;
	return(TRUE);
}

/* This allows the cursor to be displayed */

void
SMIShowCursor()
{
	unsigned char tmp;

	/* turn cursor on */
	outb(0x3C4, 0x81);
	tmp = inb(0x3C5);
	outb(0x3C5, tmp | 0x80);
}

void
SMIHideCursor()
{
	unsigned char tmp;

	/* turn cursor off */
	outb(0x3C4, 0x81);
	tmp = inb(0x3C5);
	outb(0x3C5, tmp & ~0x80);
}

static Bool
SMIRealizeCursor(ScreenPtr pScr, CursorPtr pCurs)
{
	CursorBitsPtr bits = pCurs->bits;
	unsigned char *ram;
	int index = pScr->myNum;
	pointer *pPriv = &pCurs->bits->devPriv[index];
	unsigned char *pServMsk;
	unsigned char *pServSrc;
	int bsrc, h;
	register int i, j;

	if (pCurs->bits->refcnt > 1)
		return(TRUE);

	ram = (unsigned char *) xalloc(1024);
	*pPriv = (pointer) ram;

	if (!ram)
		return(FALSE);

	pServSrc = bits->source;
	pServMsk = bits->mask;

	h = bits->height;
	if (h > MAX_CURS)
		h = MAX_CURS;

	bsrc = PixmapBytePad(bits->width, 1);	/* bytes per line */

	for (i = 0; i < MAX_CURS; i++)
	{
		for (j = 0; j < MAX_CURS / 8; j++)
		{
			if ((i < h) && (j < bsrc))
			{
				unsigned char mask   = byte_reversed[*pServMsk++];
				unsigned char source = byte_reversed[*pServSrc++];

				*ram++ = ~mask;
				*ram++ = source & mask;
				if (j & 1)
					ram += 4;
			}
			else
			{
				*ram++ = 0xFF;
				*ram++ = 0x00;
				if (j & 1)
					ram += 4;
			}
		}
		if (j < bsrc)
		{
			pServMsk += bsrc - j;
			pServSrc += bsrc - j;
		}
	}
	return(TRUE);
}

static Bool
SMIUnrealizeCursor(ScreenPtr pScr, CursorPtr pCurs)
{
	pointer priv;

	if (pCurs->bits->refcnt <= 1 &&
		(priv = pCurs->bits->devPriv[pScr->myNum]))
	{
		xfree(priv);
		pCurs->bits->devPriv[pScr->myNum] = 0x0;
	}
	return(TRUE);
}

static void
SMILoadCursor(ScreenPtr pScr, CursorPtr pCurs, int x, int y)
{
	int index = pScr->myNum;
	unsigned short *ram;
	char *videobuffer = (char *) xf86AccelInfoRec.FramebufferBase;

	if (!xf86VTSema)
		return;

	if (!pCurs)
		return;

	/* Remember which cursor is loaded */
	smiCursorpCurs = pCurs;

	/* Turn cursor off */
	SMIHideCursor();

	/* Move cursor off-screen */
	outb(0x3C4, 0x88); outb(0x3C5, 0xFF);
	outb(0x3C4, 0x89); outb(0x3C5, 0x07);
	outb(0x3C4, 0x8A); outb(0x3C5, 0xFF);
	outb(0x3C4, 0x8B); outb(0x3C5, 0x07);

	/* Load storage location */
	outb(0x3C4, 0x80);
	outb(0x3C5, (smiCursorMemory >> 11) & 0xFF);
	outb(0x3C4, 0x81);
	outb(0x3C5, (smiCursorMemory >> 19));

	ram = (unsigned short *) pCurs->bits->devPriv[index];
	MemToBus(videobuffer + smiCursorMemory + 1024, (char *) ram, 1024);

	/* Wait for vertical retrace */
	VerticalRetraceWait();

	/* Position cursor */
	SMIMoveCursor(0, x, y);
	SMIRecolorCursor(pScr, pCurs, TRUE);

	/* Turn cursor on */
	SMIShowCursor();
}

static void
SMISetCursor(ScreenPtr pScr, CursorPtr pCurs, int x, int y)
{
   int index = pScr->myNum;

   if (!pCurs)
      return;

   smiHotX = pCurs->bits->xhot;
   smiHotY = pCurs->bits->yhot;
   SMILoadCursor(pScr, pCurs, x, y);
}

void
SMIRestoreCursor(ScreenPtr pScr)
{
	int index = pScr->myNum;
	int x, y;

	miPointerPosition(&x, &y);
	SMILoadCursor(pScr, smiCursorpCurs, x, y);
}

void
SMIRepositionCursor(ScreenPtr pScr)
{
	int x, y;

	miPointerPosition(&x, &y);
	/* Wait for vertical retrace */
	VerticalRetraceWait();
	SMIMoveCursor(pScr, x, y);
}

static void
SMIMoveCursor(ScreenPtr pScr, int x, int y)
{
	if (!xf86VTSema)
		return;

	x -= vga256InfoRec.frameX0;
	y -= vga256InfoRec.frameY0;

	x -= smiHotX;
	y -= smiHotY;

	if (x >= 0)
	{
		outb(0x3C4, 0x88); outb(0x3C5, x & 0xFF);
		outb(0x3C4, 0x89); outb(0x3C5, (x >> 8) & 0x07);
	}
	else
	{
		outb(0x3C4, 0x88); outb(0x3C5, -x & 0x1F);
		outb(0x3C4, 0x89); outb(0x3C5, 0x08);
	}

	if (y >= 0)
	{
		outb(0x3C4, 0x8A); outb(0x3C5, y & 0xFF);
		outb(0x3C4, 0x8B); outb(0x3C5, (y >> 8) & 0x07);
	}
	else
	{
		outb(0x3C4, 0x8A); outb(0x3C5, -y & 0x1F);
		outb(0x3C4, 0x8B); outb(0x3C5, 0x08);
	}
}

static void
SMIRecolorCursor(ScreenPtr pScr, CursorPtr pCurs, Bool displayed)
{
	unsigned char packedcolfg, packedcolbg;

	if (!xf86VTSema)
		return;

	if (!displayed)
		return;

	packedcolfg = ((pCurs->foreRed   & 0xE000) >> 8)
				| ((pCurs->foreGreen & 0xE000) >> 11)
				| ((pCurs->foreBlue  & 0xC000) >> 14)
				;

	packedcolbg = ((pCurs->backRed   & 0xE000) >> 8)
				| ((pCurs->backGreen & 0xE000) >> 11)
				| ((pCurs->backBlue  & 0xC000) >> 14)
				;

	outb(0x3C4, 0x8C); outb(0x3C5, packedcolfg);
	outb(0x3C4, 0x8D); outb(0x3C5, packedcolbg);
}

void
SMIWarpCursor(ScreenPtr pScr, int x, int y)
{
	miPointerWarpCursor(pScr, x, y);
	xf86Info.currentScreen = pScr;
}

void
SMIQueryBestSize(int class, unsigned short *pwidth, unsigned short *pheight,
ScreenPtr pScreen)
{
	if (*pwidth > 0)
	{
		switch (class)
		{
			case CursorShape:
				if (*pwidth > MAX_CURS)
					*pwidth = MAX_CURS;
				if (*pheight > MAX_CURS)
					*pheight = MAX_CURS;
				break;

			default:
				mfbQueryBestSize(class, pwidth, pheight, pScreen);
				break;
		}
	}
}

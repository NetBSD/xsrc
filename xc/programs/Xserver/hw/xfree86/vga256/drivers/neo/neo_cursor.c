/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/neo/neo_cursor.c,v 1.1.2.5 1998/10/25 09:49:24 hohndel Exp $ */
/**********************************************************************
Copyright 1998 by Precision Insight, Inc., Cedar Park, Texas.

                        All Rights Reserved

Permission to use, copy, modify, distribute, and sell this software and
its documentation for any purpose is hereby granted without fee,
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in
supporting documentation, and that the name of Precision Insight not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  Precision Insight
and its suppliers make no representations about the suitability of this
software for any purpose.  It is provided "as is" without express or 
implied warranty.

PRECISION INSIGHT DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
EVENT SHALL PRECISION INSIGHT AND/OR ITS SUPPLIERS BE LIABLE FOR ANY
SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER
RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF
CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
**********************************************************************/

/*
 * This Precision Insight driver has been sponsored by Red Hat.
 *
 * Authors:
 *   Jens Owen (jens@precisioninsight.com)
 *   Kevin E. Martin (kevin@precisioninsight.com)
 */

#include "compiler.h"
#include "cursorstr.h"
#include "vga256.h"
#include "compiler.h"
#include "xf86.h"
#include "mipointer.h"
#include "xf86Priv.h"
#include "vga.h"
#include "vgaPCI.h"
#include "xf86xaa.h"
#include "neo.h"
#include "neo_reg.h"
#include "neo_macros.h"

extern void NeoShowCursor();
extern void NeoHideCursor();
extern void NeoRepositionCursor();

static Bool NeoRealizeCursor();
static Bool NeoUnrealizeCursor();
static void NeoSetCursor();
static void NeoMoveCursor();
static void NeoRecolorCursor();

static miPointerSpriteFuncRec neoPointerSpriteFuncs =
{
   NeoRealizeCursor,
   NeoUnrealizeCursor,
   NeoSetCursor,
   NeoMoveCursor,
};

extern miPointerScreenFuncRec xf86PointerScreenFuncs;
extern miPointerSpriteFuncRec miSpritePointerFuncs;
extern xf86InfoRec xf86Info;

/* For byte swapping, use the XAA array */
extern unsigned char byte_reversed[256];

static int neoCursGeneration = -1;
static int neoHotX;
static int neoHotY;
static CursorPtr neoCursorpCurs;
static int neoPrevxoff = 0;
static int neoPrevyoff = 0;
static Bool neoNeedSWCursor = FALSE;
static Bool neoTempSWCursor = FALSE;
static unsigned char neoCursBits[2048];
static unsigned char neoCursTemp[1024];

#define MAX_CURS 64


Bool
NeoCursorInit(pm, pScr)
    char *pm;
    ScreenPtr pScr;
{
    /* first time thrue, allocate space for the cursor */
    if (neoCursGeneration != serverGeneration) {
	miDCInitialize(pScr, &xf86PointerScreenFuncs);

	if (!(miPointerInitialize(pScr, &neoPointerSpriteFuncs,
				  &xf86PointerScreenFuncs, FALSE))) {
	    return(FALSE);
	}

	neoHotX = 0;
	neoHotY = 0;
	pScr->RecolorCursor = NeoRecolorCursor;
	neoCursGeneration = serverGeneration;
    }

    return(TRUE);
}


void
NeoShowCursor()
{
    /* turn cursor on */
    OUTREG(NEOREG_CURSCNTL, NEO_CURS_ENABLE);
}


void
NeoHideCursor()
{
    /*
     * turn cursor off 
     *
     * Sometimes we loose the I/O map, so directly use I/O here
     */
    outb(GRAX,0x82);
    outb(GRAX+1,0x0);
}


static Bool
NeoRealizeCursor(pScr, pCurs)
    ScreenPtr pScr;
    CursorPtr pCurs;
{
    return (miSpritePointerFuncs.RealizeCursor)(pScr, pCurs);
}


static Bool
NeoUnrealizeCursor(pScr, pCurs)
    ScreenPtr pScr;
    CursorPtr pCurs;
{
    return (miSpritePointerFuncs.UnrealizeCursor)(pScr, pCurs);
}


static void
NeoLoadCursor(pScr, pCurs, x, y)
    ScreenPtr pScr;
    CursorPtr pCurs;
    int x, y;
{
    int i, j;
    unsigned short *ram;
    char *videobuffer = (char *)xf86AccelInfoRec.FramebufferBase;
    unsigned short *pServMsk, *pServSrc;
    int wsrc, h;
    CursorBitsPtr bits = pCurs->bits;

    if (!xf86VTSema)
	return;

    if (!pCurs)
	return;

    /* Remember which cursor is loaded */
    neoCursorpCurs = pCurs;

    if (neoTempSWCursor)
	return;

    /*
     * twice the size of the sprite has been allocated so we can pad an extra
     * line with 0's to handle the sprite moving off top edge of display 
     */
    ram = (unsigned short *)neoCursBits;
    bzero(((char *)ram)+1024, 1024);

    pServSrc = (unsigned short *)bits->source;
    pServMsk = (unsigned short *)bits->mask;

    h = bits->height;
    if (h > MAX_CURS)
	h = MAX_CURS;

    wsrc = PixmapBytePad(bits->width, 1);

    for (i=0; i<MAX_CURS; i++) {
	for (j=0; j<(MAX_CURS/16); j++) {
	  unsigned short mask, source;

	  if ((i < h) && (j < (wsrc/2))) {
	    mask = *pServMsk++;
	    source = *pServSrc++;

	    ((char *)&mask)[0] = byte_reversed[((unsigned char *)&mask)[0]];
	    ((char *)&mask)[1] = byte_reversed[((unsigned char *)&mask)[1]];

	    ((char *)&source)[0] = byte_reversed[((unsigned char *)&source)[0]];
	    ((char *)&source)[1] = byte_reversed[((unsigned char *)&source)[1]];

	    if (j < (MAX_CURS/8)) { 
	      *ram = (~source & mask);
	      *(ram+4) = mask;
	    }
	  } else {
	    *ram = 0x0;
	    *(ram+4) = 0x0;
	  }
	  ram++;
	}
	if (j < (wsrc/2)) {
	  pServMsk += (wsrc/2 - j);
	  pServSrc += (wsrc/2 - j);
	}
	ram += 4;
    }

    /* turn cursor off */
    NeoHideCursor();

    /* Load storage location.  */
    OUTREG(NEOREG_CURSMEMPOS, ((0x000f & NeoCursorMemSegment) << 8) | 
			      ((0x0ff0 & NeoCursorMemSegment) >> 4));

    ram = (unsigned short *)neoCursBits;
    MemToBus(videobuffer + NeoCursorMemSegment * 1024, (char *) ram, 1024);

    /* reset these offset because we have a new image */
    neoPrevxoff = neoPrevyoff = 0; 

    /* position cursor */
    NeoMoveCursor(pScr, x, y);
    NeoRecolorCursor(pScr, pCurs, TRUE);

    /* turn cursor on */
    NeoShowCursor();
}


static void
NeoSetCursor(pScr, pCurs, x, y, generateEvent)
    ScreenPtr pScr;
    CursorPtr pCurs;
    int   x, y;
    Bool  generateEvent;
{
    if (neoNeedSWCursor) {
        if (!neoTempSWCursor) {
            NeoHideCursor();
            neoTempSWCursor = TRUE;
        }
        (miSpritePointerFuncs.SetCursor)(pScr, pCurs, x, y);
    } else {
        if (neoTempSWCursor) {
            (miSpritePointerFuncs.MoveCursor)(pScr, -9999, -9999);
            neoTempSWCursor = FALSE;
        }
    }

    if (!pCurs)
	return;

    neoHotX = pCurs->bits->xhot;
    neoHotY = pCurs->bits->yhot;
    NeoLoadCursor(pScr, pCurs, x, y);
}


void
NeoRestoreCursor(pScr)
    ScreenPtr pScr;
{
    int x, y;

    miPointerPosition(&x, &y);

    if (neoNeedSWCursor) {
	if (!neoTempSWCursor) {
	    NeoHideCursor();
	    neoTempSWCursor = TRUE;
	}
	(miSpritePointerFuncs.SetCursor)(pScr, neoCursorpCurs, x, y);
	miPointerWarpCursor(pScr, x, y);
    } else {
	if (neoTempSWCursor) {
	    (miSpritePointerFuncs.MoveCursor)(pScr, -9999, -9999);
	    neoTempSWCursor = FALSE;
	}
    }

    NeoLoadCursor(pScr, neoCursorpCurs, x, y);
}


static void
NeoMoveCursor(pScr, x, y)
    ScreenPtr pScr;
    int x, y;
{
    unsigned char xoff, yoff;

    if (!xf86VTSema)
	return;

    if (neoTempSWCursor) {
	(miSpritePointerFuncs.MoveCursor)(pScr, x, y);
        return;
    }

    x -= vga256InfoRec.frameX0 + neoHotX;
    y -= vga256InfoRec.frameY0 + neoHotY;

    /* Handle the case of HW cursor moving up or to the left off the display */
    if ((x < 0) && (x > (-MAX_CURS))) {
	xoff = (-x);
	x = 0;
    } else {
	xoff = 0;
    }

    if ((y < 0) && (y > (-MAX_CURS))) {
	yoff = (-y);
	y = 0;
    } else {
	yoff = 0;
    }

    if ((xoff != neoPrevxoff ) || (yoff != neoPrevyoff)) {
	int i;
	unsigned long bits, bits2;
	char *videobuffer = (char *) xf86AccelInfoRec.FramebufferBase;
	unsigned char *ram;
	ram = (unsigned char *)neoCursBits;


	/* This is for sprites that move off the top of the display.
	 * this code simply updates the pointer used for loading the sprite.
	 * Note, in our driver's RealizeCursor, the allocated buffer size 
	 * is twice as large as needed, and we initialize the upper half to all
	 * zeros, so we can use this pointer trick here.
	 */
	if (yoff) {
	    ram += (yoff * 16);
	}

	/* This is for sprites that move off the left edge of the display.
	 * this code has to do some ugly bit swizzling to generate new cursor
	 * masks that give the impression the cursor is moving off the screen.
	 *
	 * WARNING: PLATFORM SPECIFIC!  This is 32-bit little endian code! 
	 */
	if (xoff) {
	    if (xoff < 32) { /* offset 1-31 */
		for (i=0; i<256; i+=2) {
		    bits = ((unsigned long *)ram)[i];
		    bits2 = ((unsigned long *)ram)[i+1];

		    SWIZZLE32(bits);
		    SWIZZLE32(bits2);

		    bits = ((bits >> xoff) | (bits2 << (32-xoff)));
		    bits2 >>= xoff;

		    SWIZZLE32(bits);
		    SWIZZLE32(bits2);

		    ((unsigned long *)neoCursTemp)[i] = bits;
		    ((unsigned long *)neoCursTemp)[i+1] = bits2;
		}
	    }
	    else { /* offset 32-63 */
		for (i=0; i<256; i+=2) {
		    bits = ((unsigned long *)ram)[i];
		    bits2 = ((unsigned long *)ram)[i+1];

		    SWIZZLE32(bits);
		    SWIZZLE32(bits2);

		    /* bits = (bits2 >> (64-xoff)); */
		    bits = (bits2 >> (xoff-32));
		    bits2 = 0;

		    SWIZZLE32(bits);
		    SWIZZLE32(bits2);

		    ((unsigned long *)neoCursTemp)[i] = bits;
		    ((unsigned long *)neoCursTemp)[i+1] = bits2;
		}
	    }

	    /* change the ram pointer to our new swizzled cursor masks */
	    ram = neoCursTemp;
	}

	MemToBus(videobuffer + NeoCursorMemSegment * 1024, ram, 1024);

	neoPrevxoff = xoff; neoPrevyoff = yoff;
    }

    /* Move the cursor */
    OUTREG(NEOREG_CURSX, x);
    OUTREG(NEOREG_CURSY, y);
}


static void
NeoRecolorCursor(pScr, pCurs, displayed)
    ScreenPtr pScr;
    CursorPtr pCurs;
    Bool displayed;
{
    ColormapPtr pmap;
    xColorItem fgColor, bgColor;

    if (!xf86VTSema || neoTempSWCursor) {
	miRecolorCursor(pScr, pCurs, displayed);
	return;
    }

    if (!displayed)
	return;

    fgColor.red   = pCurs->foreRed >> 8;
    fgColor.green = pCurs->foreGreen >> 8;
    fgColor.blue  = pCurs->foreBlue >> 8;

    bgColor.red   = pCurs->backRed >> 8;
    bgColor.green = pCurs->backGreen >> 8;
    bgColor.blue  = pCurs->backBlue >> 8;

    /* load colors */
    OUTREG(NEOREG_CURSFGCOLOR, (((fgColor.blue  & 0xff) << 16) |
                                ((fgColor.green & 0xff) <<  8) |
                                 (fgColor.red   & 0xff)));
    OUTREG(NEOREG_CURSBGCOLOR, (((bgColor.blue  & 0xff) << 16) |
                                ((bgColor.green & 0xff) <<  8) |
                                 (bgColor.red   & 0xff)));
}


void
NeoRepositionCursor()
{
    /*
     * WARNING, this routine shouldn't be needed.  It's a workaround for
     * improper calling routines.
     *
     * The real fix is to eliminate the offset problem from the calling
     * code, and eliminate this routine all together.
     *
     * JO, 23 May 98
     */
    int x, y;

    /*
     * Reposition is getting called before the cursor code is initialized.
     */ 
    if (neoCursGeneration != serverGeneration)
	return;

    miPointerPosition(&x, &y);

    /*
     * Here's the dangerous part.  The MoveCursor routine can not depend
     * on a valid ScreenPtr.
     */
    if (!neoTempSWCursor) NeoMoveCursor(0, x, y); 
}


void
NeoWarpCursor(pScr, x, y)
    ScreenPtr pScr;
    int   x, y;
{
    miPointerWarpCursor(pScr, x, y);
    xf86Info.currentScreen = pScr;
}


void
NeoQueryBestSize(class, pwidth, pheight, pScreen)
    int class;
    unsigned short *pwidth;
    unsigned short *pheight;
    ScreenPtr pScreen;
{
    if (*pwidth > 0) {
	switch (class) {
	case CursorShape:
	    if (*pwidth > 64)
	        *pwidth = 64;
	    if (*pheight > 64)
	        *pheight = 64;
	    break;
	default:
	    mfbQueryBestSize(class, pwidth, pheight, pScreen);
	    break;
	}
    }
}


void
NeoTempSWCursor(state)
    Bool state;
{
    neoNeedSWCursor = state;
}

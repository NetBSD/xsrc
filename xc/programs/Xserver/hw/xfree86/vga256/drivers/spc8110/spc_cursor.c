/*
 * Copyright 1997,1998 by Thomas Mueller
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Thomas Mueller not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  Thomas Mueller makes no representations
 * about the suitability of this software for any purpose.  It is provided
 * "as is" without express or implied warranty.
 *
 * THOMAS MUELLER DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL THOMAS MUELLER BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 *
 */

/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/spc8110/spc_cursor.c,v 1.1.2.2 1998/10/21 10:44:48 dawes Exp $ */

/*
 * SPC8110 Hardware cursor handling. 
 */

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
#include "mipointer.h"
#include "xf86Priv.h"
#include "xf86_Option.h"
#include "xf86_OSlib.h"
#include "vga256.h"
#include "vga.h"

extern Bool vgaUseLinearAddressing;

static Bool SPC8110RealizeCursor();
static Bool SPC8110UnrealizeCursor();
static void SPC8110SetCursor();
static void SPC8110MoveCursor();
static void SPC8110RecolorCursor();

static miPointerSpriteFuncRec spcPointerSpriteFuncs =
{
    SPC8110RealizeCursor,
    SPC8110UnrealizeCursor,
    SPC8110SetCursor,
    SPC8110MoveCursor,
};

/* vga256 interface defines Init, Restore, Warp, QueryBestSize. */

extern miPointerScreenFuncRec xf86PointerScreenFuncs;
extern xf86InfoRec xf86Info;

static int spcCursorGeneration = -1;
static int spcCursorAddress;

/*
 * This is the set variables that defines the cursor state within the
 * driver.
 */

static int spcCursorHotX;
static int spcCursorHotY;
static int spcCursorWidth = 64;
static int spcCursorHeight = 64;
static CursorPtr spcCursorpCurs;

/*
 * This is a high-level init function, called once; it passes a local
 * miPointerSpriteFuncRec with additional functions that we need to provide.
 * It is called by the SVGA server.
 */

Bool
SPC8110CursorInit(pm, pScr)
char *pm;
ScreenPtr pScr;
{
    spcCursorHotX = 0;
    spcCursorHotY = 0;

    if (spcCursorGeneration != serverGeneration) {
	if (!(miPointerInitialize(pScr, &spcPointerSpriteFuncs,
				  &xf86PointerScreenFuncs, FALSE)))
	    return FALSE;

	pScr->RecolorCursor = SPC8110RecolorCursor;
	spcCursorGeneration = serverGeneration;
    }
    spcCursorAddress = vga256InfoRec.videoRam * 1024;
    /* the additional right shift by two is not documented... */
    wrinx(vgaIOBase + 4, 0x37, spcCursorAddress >> (8 + 2));
    wrinx(vgaIOBase + 4, 0x36, spcCursorAddress >> (16 + 2));
    return TRUE;
}

/*
 * This enables displaying of the cursor by the SPC8110 graphics chip.
 * It's a local function, it's not called from outside of the module.
 */

static void
SPC8110ShowCursor()
{
    /* Enable the hardware cursor. */
    modinx(0x3de, 0x05, 0x20, 0x20);
}

/*
 * This disables displaying of the cursor by the SPC8110 graphics chip.
 * This is also a local function, it's not called from outside.
 */

void
SPC8110HideCursor()
{
    /* Disable the hardware cursor. */
    modinx(0x3de, 0x05, 0x20, 0x20);
}

/*
 * This function is called when a new cursor image is requested by
 * the server. The main thing to do is convert the bitwise image
 * provided by the server into a format that the graphics card
 * can conveniently handle, and store that in system memory.
 * Adapted from accel/s3/s3Cursor.c.
 */

static Bool
SPC8110RealizeCursor(pScr, pCurs)
ScreenPtr pScr;
CursorPtr pCurs;
{
    int i;
    int j;
    unsigned char *pServMsk;
    unsigned char *pServSrc;
    int index = pScr->myNum;
    pointer *pPriv = &pCurs->bits->devPriv[index];
    int wsrc;
    int h;
    unsigned char *ram;
    CursorBitsPtr bits = pCurs->bits;

    static int lookup[] = { 0x2, 0x0, 0x2, 0x1 };

    if (pCurs->bits->refcnt > 1)
	return TRUE;

    /* ram needed = H*W*2bits/bytesize */
    ram = (unsigned char *) xalloc(1024);
    *pPriv = (pointer) ram;

    if (!ram)
	return FALSE;

    /*
     *  There are two bitmaps for the X cursor:  the Source and
     *  the Mask.  The following table decodes these bits:
     *  
     *  Src  Mask  |  Cursor color  |  Plane 0  Plane 1
     *  -----------+----------------+------------------
     *     0  0    |   Transparent  |     0        0
     *     0  1    |     Color 0    |     0        1
     *     1  0    |   Transparent  |     0        0
     *     1  1    |     Color 1    |     1        1
     *  
     *  Thus, the data for Plane 0 bits is Src AND Mask, and
     *  the data for Plane 1 bits is Mask.
     *        On the SPC8110 the bits are in different order
     *       00 = colour 0
     *       01 = colour 1
     *       10 = transparent (allow CRTC pass thru)
     *       11 = invert (Display regular pixel data inverted)
     *        so we do the conversion through lookup table. The position is
     *  calculated as (Src|Mask) and the value is corresponding SPC8110
     *  value.
     *        int lookup [] = {0x2, 0x0, 0x2, 0x1};
     *        Invert is actualy not used.
     *
     */

    pServSrc = (unsigned char *) bits->source;
    pServMsk = (unsigned char *) bits->mask;

#define MAX_CURS	64
    h = pCurs->bits->height;
    if (h > MAX_CURS)
	h = MAX_CURS;
    wsrc = PixmapBytePad(bits->width, 1);	/* Bytes per line. */

    for (i = 0; i < MAX_CURS; i++) {
	for (j = 0; j < MAX_CURS / 8; j++) {
	    unsigned char mask;
	    unsigned char source;

	    if (i < h && j < wsrc) {
		mask = *pServMsk++;
		source = *pServSrc++;

		mask = byte_reversed[mask];
		source = byte_reversed[source];


		{
		    unsigned char bm;
		    unsigned char r;
		    unsigned char db;
		    unsigned char da;

		    db = da = 0;
		    for (bm = 0x80; bm; bm >>= 1) {
		    	r = lookup[((source & bm) ? 2 : 0) | ((mask & bm) ? 1 : 0)];
			if (r == 2)
			    db |= bm;
			if (r == 1)
			    da |= bm;
		    }
		    *ram++ = da;
		    *ram++ = db;
		}
	    } else {
		*ram++ = 0x00;
		*ram++ = 0xFF;
	    }
	}
    }

    return TRUE;
}

/*
 * This is called when a cursor is no longer used. The intermediate
 * cursor image storage that we created needs to be deallocated.
 */

static Bool
SPC8110UnrealizeCursor(pScr, pCurs)
ScreenPtr pScr;
CursorPtr pCurs;
{
    pointer priv;

    if (pCurs->bits->refcnt <= 1 &&
	(priv = pCurs->bits->devPriv[pScr->myNum])) {
	xfree(priv);
	pCurs->bits->devPriv[pScr->myNum] = 0x0;
    }
    return TRUE;
}

/*
 * This function uploads a cursor image to the video memory of the
 * graphics card. The source image has already been converted by the
 * Realize function to a format that can be quickly transferred to
 * the card.
 * This is a local function that is not called from outside of this
 * module.
 */

extern void SPC8110SetWrite();

static void
SPC8110LoadCursorToCard(pScr, pCurs, x, y)
ScreenPtr pScr;
CursorPtr pCurs;
int x;
int y;				/* Not used for SPC8110. */
{
    unsigned char *cursor_image;
    int index = pScr->myNum;

    if (!xf86VTSema)
	return;

    cursor_image = pCurs->bits->devPriv[index];

    if (vgaUseLinearAddressing)
	memcpy((unsigned char *) vgaLinearBase + spcCursorAddress,
	       cursor_image, 1024);
    else {
	unsigned char bank = inb(0x3cd);
	SPC8110SetWrite(spcCursorAddress >> 16);
	memcpy((unsigned char *) vgaBase + (spcCursorAddress & 0xFFFF),
	       cursor_image, 1024);
	outb(0x3cd, bank);
    }
}

/*
 * This function should make the graphics chip display new cursor that
 * has already been "realized". We need to upload it to video memory,
 * make the graphics chip display it.
 * This is a local function that is not called from outside of this
 * module (although it largely corresponds to what the SetCursor
 * function in the Pointer record needs to do).
 */

static void
SPC8110LoadCursor(pScr, pCurs, x, y)
ScreenPtr pScr;
CursorPtr pCurs;
int x;
int y;
{
    if (!xf86VTSema)
	return;

    if (!pCurs)
	return;

    /* Remember the cursor currently loaded into this cursor slot. */
    spcCursorpCurs = pCurs;

    SPC8110HideCursor();

    /* Program the cursor image address in video memory. */
    SPC8110LoadCursorToCard(pScr, pCurs, x, y);

    SPC8110RecolorCursor(pScr, pCurs, 1);

    /* Position cursor */
    SPC8110MoveCursor(pScr, x, y);

    /* Turn it on. */
    SPC8110ShowCursor();
}

/*
 * This function should display a new cursor at a new position.
 */

static void
SPC8110SetCursor(pScr, pCurs, x, y, generateEvent)
ScreenPtr pScr;
CursorPtr pCurs;
int x;
int y;
Bool generateEvent;
{
    if (!pCurs)
	return;

    spcCursorHotX = pCurs->bits->xhot;
    spcCursorHotY = pCurs->bits->yhot;

    SPC8110LoadCursor(pScr, pCurs, x, y);
}

/*
 * This function should redisplay a cursor that has been
 * displayed earlier. It is called by the SVGA server.
 */

void
SPC8110RestoreCursor(pScr)
ScreenPtr pScr;
{
    int x;
    int y;

    miPointerPosition(&x, &y);

    SPC8110LoadCursor(pScr, spcCursorpCurs, x, y);
}

/*
 * This function is called when the current cursor is moved. It makes
 * the graphic chip display the cursor at the new position.
 */

static void
SPC8110MoveCursor(pScr, x, y)
ScreenPtr pScr;
int x;
int y;
{
    int xorigin;
    int yorigin;

    if (!xf86VTSema)
	return;

    x -= vga256InfoRec.frameX0 + spcCursorHotX;
    y -= vga256InfoRec.frameY0 + spcCursorHotY;

    /*
     * If the cursor is partly out of screen at the left or top, we need set
     * the origin.
     */
    xorigin = 0;
    yorigin = 0;
    if (x < 0) {
	xorigin = -x;
	x = 0;
    }
    if (y < 0) {
	yorigin = -y;
	y = 0;
    }
    /* Program the cursor origin (offset into the cursor bitmap). */
    wrinx(vgaIOBase + 4, 0x34, xorigin);
    wrinx(vgaIOBase + 4, 0x35, yorigin);

    /* Program the new cursor position. */
    wrinx(vgaIOBase + 4, 0x31, x);	/* Low byte. */
    wrinx(vgaIOBase + 4, 0x30, x >> 8);	/* High byte. */
    wrinx(vgaIOBase + 4, 0x33, y);	/* Low byte. */
    wrinx(vgaIOBase + 4, 0x32, y >> 8);	/* High byte. */
}

/*
 * This is a local function that programs the colors of the cursor
 * on the graphics chip.
 * Adapted from accel/s3/s3Cursor.c.
 */

static void
SPC8110RecolorCursor(pScr, pCurs, displayed)
ScreenPtr pScr;
CursorPtr pCurs;
Bool displayed;
{

    if (!xf86VTSema)
	return;

    if (!displayed)
	return;

    modinx(0x3de, 0, 0x80, 0x80);	/* Enable sprite palette addressing */

    /*
     * Write the new colours to the extended VGA palette. Palette index is
     * incremented after each write, so only write index once
     */
    outb(0x3C8, 0x0);
    outb(0x3C9, (pCurs->backRed >> 10) & 0xFF);
    outb(0x3C9, (pCurs->backGreen >> 10) & 0xFF);
    outb(0x3C9, (pCurs->backBlue >> 10) & 0xFF);
    outb(0x3C9, (pCurs->foreRed >> 10) & 0xFF);
    outb(0x3C9, (pCurs->foreGreen >> 10) & 0xFF);
    outb(0x3C9, (pCurs->foreBlue >> 10) & 0xFF);

    modinx(0x3de, 0, 0x80, 0);
}

/*
 * This doesn't do very much. It just calls the mi routine. It is called
 * by the SVGA server.
 */

void
SPC8110WarpCursor(pScr, x, y)
ScreenPtr pScr;
int x;
int y;
{
    miPointerWarpCursor(pScr, x, y);
    xf86Info.currentScreen = pScr;
}

/*
 * This function is called by the SVGA server. It returns the
 * size of the hardware cursor that we support when asked for.
 * It is called by the SVGA server.
 */

void
SPC8110QueryBestSize(class, pwidth, pheight, pScreen)
int class;
unsigned short *pwidth;
unsigned short *pheight;
ScreenPtr pScreen;
{
    if (*pwidth > 0) {
	if (class == CursorShape) {
	    *pwidth = spcCursorWidth;
	    *pheight = spcCursorHeight;
	} else
	    (void) mfbQueryBestSize(class, pwidth, pheight, pScreen);
    }
}

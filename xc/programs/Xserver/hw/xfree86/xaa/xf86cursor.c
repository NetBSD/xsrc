/* $XFree86: xc/programs/Xserver/hw/xfree86/xaa/xf86cursor.c,v 3.7.2.2 1998/02/07 10:05:51 hohndel Exp $ */
/*
 * Copyright 1996  The XFree86 Project
 *
 * Permission is hereby granted, free of charge, to any person obtaining a 
 * copy of this software and associated documentation files (the "Software"), 
 * to deal in the Software without restriction, including without limitation 
 * the rights to use, copy, modify, merge, publish, distribute, sublicense, 
 * and/or sell copies of the Software, and to permit persons to whom the 
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL 
 * HARM HANEMAAYER BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, 
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF 
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE 
 * SOFTWARE.
 * 
 * Written by Harm Hanemaayer (H.Hanemaayer@inter.nl.net).
 * Adapted for hardware/software cursor switching (alanh@fairlite.demon.co.uk)
 */

/*
 * Exported functions:
 *
 * Bool XAACursorInit(char *pm, ScreenPtr pScr)
 * void XAARestoreCursor(ScreenPtr pScr)
 * void XAAWarpCursor(ScreenPtr pScr, int x, int y)
 * void XAAQueryBestSize(int class, unsigned short *pwidth,
 *     unsigned short *pheight, ScreenPtr pScreen)
 *
 * In addition, the XAAPointerSpriteFuncs structure is passed to the
 * mi initialization function.
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
#include "gcstruct.h"

#include "compiler.h"
#include "xf86.h"
#include "mipointer.h"
#include "xf86Priv.h"
#include "xf86_Option.h"
#include "xf86_OSlib.h"

#include "xf86xaa.h"
#include "xf86local.h"
#include "xf86cursor.h"


extern unsigned char byte_reversed[256];
extern miPointerScreenFuncRec xf86PointerScreenFuncs;
extern miPointerSpriteFuncRec miSpritePointerFuncs;

static Bool CharRealizeCursor(ScreenPtr pScr, CursorPtr pCurs);
static Bool ShortRealizeCursor(ScreenPtr pScr, CursorPtr pCurs);
static Bool LongRealizeCursor(ScreenPtr pScr, CursorPtr pCurs);
static Bool Int64RealizeCursor(ScreenPtr pScr, CursorPtr pCurs);
static Bool UnrealizeCursor(ScreenPtr pScr, CursorPtr pCurs);
static void SetCursor(ScreenPtr pScr, CursorPtr pCurs, int x, int y);
static void MoveCursor(ScreenPtr pScr, int x, int y);

static miPointerSpriteFuncRec XAACharPointerSpriteFuncs =
{
   CharRealizeCursor,
   UnrealizeCursor,
   SetCursor,
   MoveCursor,
};

static miPointerSpriteFuncRec XAAShortPointerSpriteFuncs =
{
   ShortRealizeCursor,
   UnrealizeCursor,
   SetCursor,
   MoveCursor,
};

static miPointerSpriteFuncRec XAALongPointerSpriteFuncs =
{
   LongRealizeCursor,
   UnrealizeCursor,
   SetCursor,
   MoveCursor,
};

static miPointerSpriteFuncRec XAAInt64PointerSpriteFuncs =
{
   Int64RealizeCursor,
   UnrealizeCursor,
   SetCursor,
   MoveCursor,
};
/*
 * This set of variables defines the cursor state within the driver.
 */

static CursorPtr CurrentlyLoadedCursor;
static int CursorGeneration = -1;
static int CursorHotX;
static int CursorHotY;
static int CurrentCursorIsSkewed = FALSE;
Bool tempSWCursor = FALSE;

static void RecolorCursor(ScreenPtr pScr, CursorPtr pCurs, Bool displayed);
static void LoadCursor(ScreenPtr pScr, CursorPtr pCurs, int x, int y);
static void LoadCursorToCard(ScreenPtr pScr, CursorPtr pCurs, int xoffset,
    int yoffset);

/*
 * This is a high-level init function, called once; it passes a local
 * miPointerSpriteFuncRec with additional functions that we need to provide.
 */

Bool XAACursorInit(char *pm, ScreenPtr pScr)
{
    CursorHotX = 0;
    CursorHotY = 0;

    if (CursorGeneration != serverGeneration) {
	miDCInitialize (pScr, &xf86PointerScreenFuncs);

    	if (HARDWARE_CURSOR_INT64_BIT_FORMAT & XAACursorInfoRec.Flags) {
		if (!(miPointerInitialize(pScr, &XAAInt64PointerSpriteFuncs,
			&xf86PointerScreenFuncs, FALSE)))
		return FALSE;
	} else

	if (HARDWARE_CURSOR_LONG_BIT_FORMAT & XAACursorInfoRec.Flags) {
		if (!(miPointerInitialize(pScr, &XAALongPointerSpriteFuncs,
			&xf86PointerScreenFuncs, FALSE)))
		return FALSE;
	} else

    	if (HARDWARE_CURSOR_SHORT_BIT_FORMAT & XAACursorInfoRec.Flags) {
		if (!(miPointerInitialize(pScr, &XAAShortPointerSpriteFuncs,
			&xf86PointerScreenFuncs, FALSE)))
		return FALSE;
	} else {
		if (!(miPointerInitialize(pScr, &XAACharPointerSpriteFuncs,
			&xf86PointerScreenFuncs, FALSE)))
		return FALSE;
	}

	pScr->RecolorCursor = RecolorCursor;
	CursorGeneration = serverGeneration;
    }
    
    return TRUE;
}
    

/*
 * This function should redisplay a cursor that has been
 * displayed earlier. We have to allow for the possiblity
 * of a switch between hardware and software cursor. This
 * also means that we have to realize the cursor again for
 * hardware cursors.
 */

void XAARestoreCursor(ScreenPtr pScr)
{
    int x, y;

    if (tempSWCursor) {
    	miPointerWarpCursor(pScr, x, y);
	LoadCursor(pScr, CurrentlyLoadedCursor, x, y);
    }
    else {
	miPointerPosition(&x, &y);
	LoadCursor(pScr, CurrentlyLoadedCursor, x, y);
    }
}

/*
 * This doesn't do very much. It just calls the mi routine.
 */

void XAAWarpCursor(ScreenPtr pScr, int x, int y)
{
    miPointerWarpCursor(pScr, x, y);
    xf86Info.currentScreen = pScr;
}

/*
 * This function returns the size of the hardware cursor that we
 * support when asked for.
 */

void XAAQueryBestSize(int class, unsigned short *pwidth,
unsigned short *pheight, ScreenPtr pScreen)
{
    if (*pwidth > 0) {
 	if (class == CursorShape) {
	    *pwidth = XAACursorInfoRec.MaxWidth;
	    *pheight = XAACursorInfoRec.MaxHeight;
	}
	else
	    (void) mfbQueryBestSize(class, pwidth, pheight, pScreen);
    }
}


/*
 * The following functions are also called by the higher level code,
 * since they are included in the miPointerSpriteFuncs.
 */

/*
 * This is called when a cursor is no longer used. The intermediate
 * cursor image storage that we created needs to be deallocated.
 */

static Bool UnrealizeCursor(ScreenPtr pScr, CursorPtr pCurs)
{
    pointer priv;

    if (pCurs->bits->height > XAACursorInfoRec.MaxHeight ||
	 pCurs->bits->width  > XAACursorInfoRec.MaxWidth) {
		extern miPointerSpriteFuncRec miSpritePointerFuncs;
		return (miSpritePointerFuncs.UnrealizeCursor)(pScr, pCurs);
    }

    if (pCurs->bits->refcnt <= 1 &&
    (priv = pCurs->bits->devPriv[pScr->myNum])) {
	xfree(priv);
	pCurs->bits->devPriv[pScr->myNum] = 0x0;
    }
    return TRUE;
}

/*
 * This function is called when the current cursor is moved. It makes
 * the graphic chip display the cursor at the new position.
 */

static void MoveCursor(ScreenPtr pScr, int x, int y)
{
    int xorigin, yorigin;

    if (!xf86VTSema)
	return;

    if (tempSWCursor) {
	(miSpritePointerFuncs.MoveCursor)(pScr, x, y);
	return;
    }

    x -= xf86AccelInfoRec.ServerInfoRec->frameX0 + CursorHotX;
    y -= xf86AccelInfoRec.ServerInfoRec->frameY0 + CursorHotY;

    /*
     * If the cursor is partly out of screen at the left or top,
     * we need set the origin.
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

    if (HARDWARE_CURSOR_SYNC_NEEDED & XAACursorInfoRec.Flags)
        xf86AccelInfoRec.Sync();

    /* Handle cursor hardware that does not have a programmed origin. */
    if (!(XAACursorInfoRec.Flags & HARDWARE_CURSOR_PROGRAMMED_ORIGIN)) {
        if (xorigin || yorigin) {
            /* Need to upload new cursor image, simulating origin offset. */
            LoadCursorToCard(pScr, CurrentlyLoadedCursor, xorigin, yorigin);
            CurrentCursorIsSkewed = TRUE;
        }
        else
            if (CurrentCursorIsSkewed) {
                /* Need to upload clean cursor image. */
                LoadCursorToCard(pScr, CurrentlyLoadedCursor, 0, 0);
                CurrentCursorIsSkewed = FALSE;
            }
    }

    XAACursorInfoRec.SetCursorPosition(x, y, xorigin, yorigin);
}

/*
 * This function should display a new cursor at a new position. Due
 * to switches between hardware and software cursors, the function
 * might need to realize the cursor again.
 */

static void SetCursor(ScreenPtr pScr, CursorPtr pCurs, int x, int y)
{

    if (!pCurs)
	return;
    
    CurrentlyLoadedCursor = pCurs;

    if (HARDWARE_CURSOR_SYNC_NEEDED & XAACursorInfoRec.Flags)
        xf86AccelInfoRec.Sync();

    if ((pCurs->bits->height > XAACursorInfoRec.MaxHeight) ||
	 (pCurs->bits->width  > XAACursorInfoRec.MaxWidth)) {
	if(!tempSWCursor) {
	   XAACursorInfoRec.HideCursor();
	   tempSWCursor = TRUE;
	}
	(miSpritePointerFuncs.SetCursor)(pScr, pCurs, x, y);
	return;
    }
    
    if (tempSWCursor) {
	(miSpritePointerFuncs.MoveCursor)(pScr, -9999, -9999);
	tempSWCursor = FALSE;
    }

    CursorHotX = pCurs->bits->xhot;
    CursorHotY = pCurs->bits->yhot;

    LoadCursor(pScr, pCurs, x, y);
}

/*
 * This function is called when a new cursor image is requested by
 * the server. The main thing to do is convert the bitwise image
 * provided by the server into a format that the graphics card
 * can conveniently handle, and store that in system memory.
 * Adapted from accel/s3/s3Cursor.c.
 */

static Bool Int64RealizeCursor(ScreenPtr pScr, CursorPtr pCurs)
{
    register int i, j;
    unsigned long *pServMsk;
    unsigned long *pServSrc;
    int   index = pScr->myNum;
    pointer *pPriv = &pCurs->bits->devPriv[index];
    int   wsrc, h;
    unsigned long *ram;
    CursorBitsPtr bits = pCurs->bits;

    if ((bits->height > XAACursorInfoRec.MaxHeight ||
	 bits->width  > XAACursorInfoRec.MaxWidth)) {
	return (miSpritePointerFuncs.RealizeCursor)(pScr, pCurs);
    }

    if (pCurs->bits->refcnt > 1)
        return TRUE;

    ram = (unsigned long *)xalloc(1024);
    *pPriv = (pointer) ram;
    if (!ram)
       return FALSE;

    pServSrc = (unsigned long *)bits->source;
    pServMsk = (unsigned long *)bits->mask;

    h = bits->height;
    if (h > XAACursorInfoRec.MaxHeight)
        h = XAACursorInfoRec.MaxHeight;

    wsrc = PixmapBytePad(bits->width, 1);	/* Bytes per line. */

    for (i = 0; i < XAACursorInfoRec.MaxHeight; i++) {
	for (j = 0; j < XAACursorInfoRec.MaxWidth / 64; j++) {
	    unsigned long mask, source;
	    unsigned long mask_low, source_low;
	    unsigned long mask_high, source_high;

	    if (i < h && j < wsrc/8) {
	        mask_low = *pServMsk++;
	        source_low = *pServSrc++;
	        mask_high = *pServMsk++;
	        source_high = *pServSrc++;

                if (XAACursorInfoRec.Flags & HARDWARE_CURSOR_BIT_ORDER_MSBFIRST) {
	            ((char *)&mask_low)[0] = byte_reversed[((unsigned char *)&mask_low)[0]];
	            ((char *)&mask_low)[1] = byte_reversed[((unsigned char *)&mask_low)[1]];
	            ((char *)&mask_low)[2] = byte_reversed[((unsigned char *)&mask_low)[2]];
	            ((char *)&mask_low)[3] = byte_reversed[((unsigned char *)&mask_low)[3]];
	            ((char *)&mask_high)[0] = byte_reversed[((unsigned char *)&mask_high)[0]];
	            ((char *)&mask_high)[1] = byte_reversed[((unsigned char *)&mask_high)[1]];
	            ((char *)&mask_high)[2] = byte_reversed[((unsigned char *)&mask_high)[2]];
	            ((char *)&mask_high)[3] = byte_reversed[((unsigned char *)&mask_high)[3]];
	            ((char *)&source_low)[0] = byte_reversed[((unsigned char *)&source_low)[0]];
	            ((char *)&source_low)[1] = byte_reversed[((unsigned char *)&source_low)[1]];
	            ((char *)&source_low)[2] = byte_reversed[((unsigned char *)&source_low)[2]];
	            ((char *)&source_low)[3] = byte_reversed[((unsigned char *)&source_low)[3]];
	            ((char *)&source_high)[0] = byte_reversed[((unsigned char *)&source_high)[0]];
	            ((char *)&source_high)[1] = byte_reversed[((unsigned char *)&source_high)[1]];
	            ((char *)&source_high)[2] = byte_reversed[((unsigned char *)&source_high)[2]];
	            ((char *)&source_high)[3] = byte_reversed[((unsigned char *)&source_high)[3]];
		}
	        if (XAACursorInfoRec.Flags & HARDWARE_CURSOR_AND_SOURCE_WITH_MASK) {
	            source_low &= mask_low;
	            source_high &= mask_high;
		}
		
	    	if (XAACursorInfoRec.Flags & HARDWARE_CURSOR_INVERT_MASK) {
		    mask_low = ~mask_low;
		    mask_high = ~mask_high;
		}
		
		if (j < XAACursorInfoRec.MaxWidth / 8) {
	    	if (XAACursorInfoRec.Flags & HARDWARE_CURSOR_SWAP_SOURCE_AND_MASK) {
	            *ram++ = source_low;
	            *ram++ = source_high;
	            *ram++ = mask_low;
	            *ram++ = mask_high;
	    	} else {
	            *ram++ = mask_low;
	            *ram++ = mask_high;
	            *ram++ = source_low;
	            *ram++ = source_high;
	    	}
		}
	    } else {
	    	if (XAACursorInfoRec.Flags & HARDWARE_CURSOR_INVERT_MASK)
		    mask = 0xFFFFFFFF;
		else
	            mask = 0x00000000;

	        if (XAACursorInfoRec.Flags & HARDWARE_CURSOR_AND_SOURCE_WITH_MASK)
		    source = 0x00000000;
		else
		    source = 0xFFFFFFFF;
	      
	    	if (XAACursorInfoRec.Flags & HARDWARE_CURSOR_SWAP_SOURCE_AND_MASK) {
		    *ram++ = source;
		    *ram++ = source;
		    *ram++ = mask;
		    *ram++ = mask;
		} else {
		    *ram++ = mask;
		    *ram++ = mask;
		    *ram++ = source;
		    *ram++ = source;
		}
	    }
        }
        /*
         * if we still have more bytes on this line (j < wsrc),
         * we have to ignore the rest of the line.
         */
        while (j++ < wsrc/8) pServMsk++,pServSrc++;
    }
    return TRUE;
}

static Bool LongRealizeCursor(ScreenPtr pScr, CursorPtr pCurs)
{
    register int i, j;
    unsigned long *pServMsk;
    unsigned long *pServSrc;
    int   index = pScr->myNum;
    pointer *pPriv = &pCurs->bits->devPriv[index];
    int   wsrc, h;
    unsigned long *ram;
    CursorBitsPtr bits = pCurs->bits;

    if ((bits->height > XAACursorInfoRec.MaxHeight ||
	 bits->width  > XAACursorInfoRec.MaxWidth)) {
	return (miSpritePointerFuncs.RealizeCursor)(pScr, pCurs);
    }

    if (pCurs->bits->refcnt > 1)
        return TRUE;

    ram = (unsigned long *)xalloc(1024);
    *pPriv = (pointer) ram;
    if (!ram)
       return FALSE;

    pServSrc = (unsigned long *)bits->source;
    pServMsk = (unsigned long *)bits->mask;

    h = bits->height;
    if (h > XAACursorInfoRec.MaxHeight)
        h = XAACursorInfoRec.MaxHeight;

    wsrc = PixmapBytePad(bits->width, 1);	/* Bytes per line. */

    for (i = 0; i < XAACursorInfoRec.MaxHeight; i++) {
	for (j = 0; j < XAACursorInfoRec.MaxWidth / 32; j++) {
	    unsigned long mask, source;

	    if (i < h && j < wsrc/4) {
	        mask = *pServMsk++;
	        source = *pServSrc++;

                if (XAACursorInfoRec.Flags & HARDWARE_CURSOR_BIT_ORDER_MSBFIRST) {
	            ((char *)&mask)[0] = byte_reversed[((unsigned char *)&mask)[0]];
	            ((char *)&mask)[1] = byte_reversed[((unsigned char *)&mask)[1]];
	            ((char *)&mask)[2] = byte_reversed[((unsigned char *)&mask)[2]];
	            ((char *)&mask)[3] = byte_reversed[((unsigned char *)&mask)[3]];
	            ((char *)&source)[0] = byte_reversed[((unsigned char *)&source)[0]];
	            ((char *)&source)[1] = byte_reversed[((unsigned char *)&source)[1]];
	            ((char *)&source)[2] = byte_reversed[((unsigned char *)&source)[2]];
	            ((char *)&source)[3] = byte_reversed[((unsigned char *)&source)[3]];
		}
	        if (XAACursorInfoRec.Flags & HARDWARE_CURSOR_AND_SOURCE_WITH_MASK)
	            source &= mask;

	    	if (XAACursorInfoRec.Flags & HARDWARE_CURSOR_INVERT_MASK)
		    mask = ~mask;
		
		if (j < XAACursorInfoRec.MaxWidth / 8) {
	    	if (XAACursorInfoRec.Flags & HARDWARE_CURSOR_SWAP_SOURCE_AND_MASK) {
	            *ram++ = source;
	            *ram++ = mask;
	    	} else {
	            *ram++ = mask;
	            *ram++ = source;
	    	}
		}
	    } else {
	    	if (XAACursorInfoRec.Flags & HARDWARE_CURSOR_INVERT_MASK)
		    mask = 0xFFFFFFFF;
		else
	            mask = 0x00000000;

	        if (XAACursorInfoRec.Flags & HARDWARE_CURSOR_AND_SOURCE_WITH_MASK)
		    source = 0x00000000;
		else
		    source = 0xFFFFFFFF;
	      
	    	if (XAACursorInfoRec.Flags & HARDWARE_CURSOR_SWAP_SOURCE_AND_MASK) {
		    *ram++ = source;
		    *ram++ = mask;
		} else {
		    *ram++ = mask;
		    *ram++ = source;
		}
	    }
        }
        /*
         * if we still have more bytes on this line (j < wsrc),
         * we have to ignore the rest of the line.
         */
        while (j++ < wsrc/4) pServMsk++,pServSrc++;
    }
    return TRUE;
}

static Bool ShortRealizeCursor(ScreenPtr pScr, CursorPtr pCurs)
{
    register int i, j;
    unsigned short *pServMsk;
    unsigned short *pServSrc;
    int   index = pScr->myNum;
    pointer *pPriv = &pCurs->bits->devPriv[index];
    int   wsrc, h;
    unsigned short *ram;
    CursorBitsPtr bits = pCurs->bits;

    if ((bits->height > XAACursorInfoRec.MaxHeight ||
	 bits->width  > XAACursorInfoRec.MaxWidth)) {
	return (miSpritePointerFuncs.RealizeCursor)(pScr, pCurs);
    }

    if (pCurs->bits->refcnt > 1)
        return TRUE;

    ram = (unsigned short *)xalloc(1024);
    *pPriv = (pointer) ram;
    if (!ram)
       return FALSE;

    pServSrc = (unsigned short *)bits->source;
    pServMsk = (unsigned short *)bits->mask;

    h = bits->height;
    if (h > XAACursorInfoRec.MaxHeight)
        h = XAACursorInfoRec.MaxHeight;

    wsrc = PixmapBytePad(bits->width, 1);	/* Bytes per line. */

    for (i = 0; i < XAACursorInfoRec.MaxHeight; i++) {
        for (j = 0; j < XAACursorInfoRec.MaxWidth / 16; j++) {
	    unsigned short mask, source;

	    if (i < h && j < wsrc/2) {
	        mask = *pServMsk++;
	        source = *pServSrc++;

                if (XAACursorInfoRec.Flags & HARDWARE_CURSOR_BIT_ORDER_MSBFIRST) {
	            ((char *)&mask)[0] = byte_reversed[((unsigned char *)&mask)[0]];
	            ((char *)&mask)[1] = byte_reversed[((unsigned char *)&mask)[1]];
	            ((char *)&source)[0] = byte_reversed[((unsigned char *)&source)[0]];
	            ((char *)&source)[1] = byte_reversed[((unsigned char *)&source)[1]];
	        }
	        if (XAACursorInfoRec.Flags & HARDWARE_CURSOR_AND_SOURCE_WITH_MASK)
	            source &= mask;

	    	if (XAACursorInfoRec.Flags & HARDWARE_CURSOR_INVERT_MASK)
		    mask = ~mask;
		
		if (j < XAACursorInfoRec.MaxWidth / 8) {
	    	if (XAACursorInfoRec.Flags & HARDWARE_CURSOR_SWAP_SOURCE_AND_MASK) {
	            *ram++ = source;
	            *ram++ = mask;
	    	} else {
	            *ram++ = mask;
	            *ram++ = source;
	    	}
		}
	    } else {
	    	if (XAACursorInfoRec.Flags & HARDWARE_CURSOR_INVERT_MASK)
		    mask = 0xFFFF;
		else
	            mask = 0x0000;

	        if (XAACursorInfoRec.Flags & HARDWARE_CURSOR_AND_SOURCE_WITH_MASK)
		    source = 0x0000;
		else
		    source = 0xFFFF;
	      
	    	if (XAACursorInfoRec.Flags & HARDWARE_CURSOR_SWAP_SOURCE_AND_MASK) {
		    *ram++ = source;
		    *ram++ = mask;
		} else {
		    *ram++ = mask;
		    *ram++ = source;
		}
	    }
        }
        /*
         * if we still have more bytes on this line (j < wsrc),
         * we have to ignore the rest of the line.
         */
        while (j++ < wsrc/2) pServMsk++,pServSrc++;
    }
    return TRUE;
}

static Bool CharRealizeCursor(ScreenPtr pScr, CursorPtr pCurs)
{
    register int i, j;
    unsigned char *pServMsk;
    unsigned char *pServSrc;
    int   index = pScr->myNum;
    pointer *pPriv = &pCurs->bits->devPriv[index];
    int   wsrc, h;
    unsigned char *ram;
    CursorBitsPtr bits = pCurs->bits;

    if ((bits->height > XAACursorInfoRec.MaxHeight ||
	 bits->width  > XAACursorInfoRec.MaxWidth)) {
	return (miSpritePointerFuncs.RealizeCursor)(pScr, pCurs);
    }

    if (pCurs->bits->refcnt > 1)
        return TRUE;

    ram = (unsigned char *)xalloc(1024);
    *pPriv = (pointer) ram;
    if (!ram)
       return FALSE;

    pServSrc = (unsigned char *)bits->source;
    pServMsk = (unsigned char *)bits->mask;

    h = bits->height;
    if (h > XAACursorInfoRec.MaxHeight)
        h = XAACursorInfoRec.MaxHeight;

    wsrc = PixmapBytePad(bits->width, 1);	/* Bytes per line. */

    for (i = 0; i < XAACursorInfoRec.MaxHeight; i++) {
        for (j = 0; j < XAACursorInfoRec.MaxWidth / 8; j++) {
	    unsigned char mask, source, m, s;

	    if (i < h && j < wsrc) {
	        mask = *pServMsk++;
	        source = *pServSrc++;

                if (XAACursorInfoRec.Flags & HARDWARE_CURSOR_BIT_ORDER_MSBFIRST) {
	            mask = byte_reversed[mask];
	            source = byte_reversed[source];
	        }
	        if (XAACursorInfoRec.Flags & HARDWARE_CURSOR_AND_SOURCE_WITH_MASK)
	            source &= mask;

	    	if (XAACursorInfoRec.Flags & HARDWARE_CURSOR_INVERT_MASK)
		    mask = ~mask;
		
		if (XAACursorInfoRec.Flags & HARDWARE_CURSOR_SOURCE_MASK_INTERLEAVE) {
		    m =    ((mask&0x01) << 7) | ((source&0x01) << 6) |
                           ((mask&0x02) << 4) | ((source&0x02) << 3) |
                           ((mask&0x04) << 1) | (source&0x04)        |
                           ((mask&0x08) >> 2) | ((source&0x08) >> 3) ;
		    s =    ((mask&0x10) << 3) | ((source&0x10) << 2) |
                           (mask&0x20)        | ((source&0x20) >> 1) |
                           ((mask&0x40) >> 3) | ((source&0x40) >> 4) |
                           ((mask&0x80) >> 6) | ((source&0x80) >> 7) ;
		    mask = m;
		    source = s;
		}
	    	if (XAACursorInfoRec.Flags & HARDWARE_CURSOR_SWAP_SOURCE_AND_MASK) {
	            *ram++ = source;
	            *ram++ = mask;
	    	} else {
	            *ram++ = mask;
	            *ram++ = source;
		}
	    } else {
	    	if (XAACursorInfoRec.Flags & HARDWARE_CURSOR_INVERT_MASK)
		    mask = 0xFF;
		else
	            mask = 0x00;

	        if (XAACursorInfoRec.Flags & HARDWARE_CURSOR_AND_SOURCE_WITH_MASK)
		    source = 0x00;
		else
		    source = 0xFF;

		if (XAACursorInfoRec.Flags & HARDWARE_CURSOR_SWAP_SOURCE_AND_MASK) {
		    m = mask;
		    mask = source;
		    source = m;
		}
		
		if (XAACursorInfoRec.Flags & HARDWARE_CURSOR_SOURCE_MASK_INTERLEAVE) {
		    if ((source == 0x00) && (mask == 0x00)) {
		        source = 0x00;
		        mask = 0x00;
		    } else if ((source == 0xFF) && (mask == 0x00)) {
		      if (XAACursorInfoRec.Flags & HARDWARE_CURSOR_BIT_ORDER_MSBFIRST) {		      
		        source = 0xAA;
		        mask = 0xAA;
		      } else {
		        source = 0x55;
		        mask = 0x55;
		      }
		    } else if ((source == 0x00) && (mask == 0xFF)) {
		      if (XAACursorInfoRec.Flags & HARDWARE_CURSOR_BIT_ORDER_MSBFIRST) {		      
		        source = 0x55;
		        mask = 0x55;
		      } else {
		        source = 0xAA;
		        mask = 0xAA;
		      }
		    } else {
		        source = 0xFF;
		        mask = 0xFF;
		    }
		}
		*ram++ = mask;
		*ram++ = source;
	    }
        }
        /*
         * if we still have more bytes on this line (j < wsrc),
         * we have to ignore the rest of the line.
         */
        while (j++ < wsrc) pServMsk++,pServSrc++;
    }
    return TRUE;
}


/*
 * Now local functions that are not directly called by higher-level code.
 */

static void RecolorCursor(ScreenPtr pScr, CursorPtr pCurs, Bool displayed)
{
    ColormapPtr pmap;
    unsigned short packedcolfg, packedcolbg;

    if (!xf86VTSema || tempSWCursor) {
	miRecolorCursor(pScr, pCurs, displayed);
  	return;
    }

    if (!displayed)
        return;

    if (HARDWARE_CURSOR_SYNC_NEEDED & XAACursorInfoRec.Flags)
        xf86AccelInfoRec.Sync();

    if (xf86AccelInfoRec.BitsPerPixel == 8 &&
    !(XAACursorInfoRec.Flags & HARDWARE_CURSOR_TRUECOLOR_AT_8BPP)) {
        xColorItem sourceColor, maskColor;
        XAACursorInfoRec.GetInstalledColormaps(pScr, &pmap);
	sourceColor.red = pCurs->foreRed;
	sourceColor.green = pCurs->foreGreen;
	sourceColor.blue = pCurs->foreBlue;
	FakeAllocColor(pmap, &sourceColor);
	maskColor.red = pCurs->backRed;
	maskColor.green = pCurs->backGreen;
	maskColor.blue = pCurs->backBlue;
	FakeAllocColor(pmap, &maskColor);
	FakeFreeColor(pmap, sourceColor.pixel);
	FakeFreeColor(pmap, maskColor.pixel);
	XAACursorInfoRec.SetCursorColors(maskColor.pixel, sourceColor.pixel);
    }
    else
        /* Pass colors in 8-8-8 RGB format, blue byte first. */
        XAACursorInfoRec.SetCursorColors(
            (pCurs->backBlue >> 8) |
            ((pCurs->backGreen >> 8) << 8) |
            ((pCurs->backRed >> 8) << 16),
            (pCurs->foreBlue >> 8) |
            ((pCurs->foreGreen >> 8) << 8) |
            ((pCurs->foreRed >> 8) << 16)
        );
}

/*
 * This function uploads a cursor image to the video memory of the
 * graphics card. The source image has already been converted by the
 * Realize function to a format that can be quickly transferred to
 * the card.
 * This is a local function that is not called from outside of this
 * module.
 */

static void LoadCursorToCard(ScreenPtr pScr, CursorPtr pCurs, int xoffset,
int yoffset)
{
    unsigned char *cursor_image;
    int index = pScr->myNum;

    if (tempSWCursor)
	return;

    cursor_image = (unsigned char *)pCurs->bits->devPriv[index];

    if (XAACursorInfoRec.Flags & HARDWARE_CURSOR_PROGRAMMED_BITS) {
        /*
         * Cursor image must be explicitly programmed in a chip-specific
         * function.
         */
        XAACursorInfoRec.LoadCursorImage(cursor_image, xoffset, yoffset);
    }
    else
        if (xoffset == 0 && yoffset == 0)
            xf86AccelInfoRec.ImageWrite(
                XAACursorInfoRec.CursorDataX,
                XAACursorInfoRec.CursorDataY,
                (XAACursorInfoRec.MaxWidth * XAACursorInfoRec.MaxHeight * 2) / xf86bpp,
                1,
                cursor_image,
                0, GXcopy, ~0
            );
        else
            /*
             * XXX
             * Must simulate programmable origin by uploading pattern
             * "skewed" (horizontally and/or vertically).
             */
            ;
}

/*
 * This function should make the graphics chip display new cursor that
 * has already been "realized". We need to upload it to video memory,
 * make the graphics chip display it.
 * This is a local function that is not called from outside of this
 * module (although it largely corresponds to what the SetCursor
 * function in the Pointer record needs to do).
 */

static void LoadCursor(ScreenPtr pScr, CursorPtr pCurs, int x, int y)
{
	if (!xf86VTSema)
		return;

	if (!pCurs)
		return;

    	if (HARDWARE_CURSOR_SYNC_NEEDED & XAACursorInfoRec.Flags)
            	xf86AccelInfoRec.Sync();

        if (!tempSWCursor) XAACursorInfoRec.HideCursor();

        /*
         * In the case of no programmable origin, the pattern will be
         * reloaded during MoveCursor if necessary.
         */
        LoadCursorToCard(pScr, pCurs, 0, 0);

	RecolorCursor(pScr, pCurs, 1);

	/* Position cursor */
	MoveCursor(pScr, x, y);

	/* Turn it on. */
	if (!tempSWCursor) XAACursorInfoRec.ShowCursor();
}



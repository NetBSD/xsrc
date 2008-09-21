/* $XConsortium: nv_driver.c /main/3 1996/10/28 05:13:37 kaleb $ */
/*
 * Copyright 1996-1997  David J. McKay
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
 * DAVID J. MCKAY BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/* Rewritten with reference from mga driver and 3.3.4 NVIDIA driver by
   Jarno Paananen <jpaana@s2.org> */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "nv_include.h"

#include "nvreg.h"
#include "nvvga.h"

/****************************************************************************\
*                                                                            *
*                        XAA HW Cursor Entrypoints                           *
*                                                                            *
\****************************************************************************/
/*
 * RIVA supports full colour cursors as X1R5G5B5.  Upper bit is the XOR
 * bit.  All 0's equals transparency.
 */
#define TRANSPARENT_PIXEL   0
#define ConvertToRGB555(c) \
(((c & 0xf80000) >> 9 ) | ((c & 0xf800) >> 6 ) | ((c & 0xf8) >> 3 ) | 0x8000)

static void ConvertCursor(NVPtr pNv, unsigned int* src, unsigned short *dst)
{
    int i, j, b, m;
    
    for ( i = 0; i < MAX_CURS; i++ )
    {
        b = *src++;
        m = *src++;
        for ( j = 0; j < MAX_CURS; j++ )
        {
#if X_BYTE_ORDER == X_BIG_ENDIAN
            if ( m & 0x80000000)
                *dst = ( b & 0x80000000) ? pNv->curFg : pNv->curBg;
            else
                *dst = TRANSPARENT_PIXEL;
            b <<= 1;
            m <<= 1;
#else
            if ( m & 1 )
                *dst = ( b & 1) ? pNv->curFg : pNv->curBg;
            else
                *dst = TRANSPARENT_PIXEL;
            b >>= 1;
            m >>= 1;
#endif
            dst++;
        }
    }
}

static void
LoadCursor(ScrnInfoPtr pScrn, unsigned short *tmp)
{
    int         *image, i, numInts;
    NVPtr pNv = NVPTR(pScrn);

    numInts = (MAX_CURS*MAX_CURS*2) / sizeof(int);
    image   = (int *)tmp;
    for (i = 0; i < numInts; i++)
        pNv->riva.CURSOR[i] = image[i];
}

static void
NVLoadCursorImage( ScrnInfoPtr pScrn, unsigned char *src )
{
    NVPtr pNv = NVPTR(pScrn);
    unsigned short      tmp[MAX_CURS*MAX_CURS];

    /* Copy image for color changes */
    memcpy(pNv->curImage, src, MAX_CURS*2*4);

    ConvertCursor(pNv, (unsigned int*)src, tmp);
    LoadCursor(pScrn, tmp);
}


/*
 * This function should display a new cursor at a new position.
 */
static void
NVSetCursorPosition(ScrnInfoPtr pScrn, int x, int y)
{
    NVPtr pNv = NVPTR(pScrn);

    pNv->riva.ShowHideCursor(&pNv->riva, 0);
    *(pNv->riva.CURSORPOS) = (x & 0xFFFF) | (y << 16);
    pNv->riva.ShowHideCursor(&pNv->riva, 1);
}

static void
NVSetCursorColors(ScrnInfoPtr pScrn, int bg, int fg)
{
    NVPtr pNv = NVPTR(pScrn);
    unsigned short fore, back;

    fore = ConvertToRGB555(fg);
    back = ConvertToRGB555(bg);

#if X_BYTE_ORDER == X_BIG_ENDIAN
    fore = (fore << 8) | (fore >> 8);
    back = (back << 8) | (back >> 8);
#endif

    if (pNv->curFg != fore || pNv->curBg != back) {
        unsigned short      tmp[MAX_CURS*MAX_CURS];

        pNv->curFg = fore;
        pNv->curBg = back;

        ConvertCursor(pNv, pNv->curImage, tmp);
        LoadCursor(pScrn, tmp);
    }
}


static void
NVShowCursor(ScrnInfoPtr pScrn)
{
    NVPtr pNv = NVPTR(pScrn);
    /* Enable cursor - X-Windows mode */
    pNv->riva.ShowHideCursor(&pNv->riva, 1);
}

static void
NVHideCursor(ScrnInfoPtr pScrn)
{
    NVPtr pNv = NVPTR(pScrn);
    /* Disable cursor */
    pNv->riva.ShowHideCursor(&pNv->riva, 0);
}

static Bool
NVUseHWCursor(ScreenPtr pScreen, CursorPtr pCurs)
{
    return TRUE;
}

Bool
NVXCursorInit(ScreenPtr pScreen)
{
    ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
    NVPtr pNv = NVPTR(pScrn);
    xf86CursorInfoPtr infoPtr;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "NVXCursorInit\n"));

    infoPtr = xf86CreateCursorInfoRec();
    if(!infoPtr) return FALSE;

    pNv->CursorInfoRec = infoPtr;

    infoPtr->MaxWidth = MAX_CURS;
    infoPtr->MaxHeight = MAX_CURS;
    infoPtr->Flags = HARDWARE_CURSOR_TRUECOLOR_AT_8BPP |
                     HARDWARE_CURSOR_SOURCE_MASK_INTERLEAVE_32;
    infoPtr->SetCursorColors = NVSetCursorColors;
    infoPtr->SetCursorPosition = NVSetCursorPosition;
    infoPtr->LoadCursorImage = NVLoadCursorImage;
    infoPtr->HideCursor = NVHideCursor;
    infoPtr->ShowCursor = NVShowCursor;
    infoPtr->UseHWCursor = NVUseHWCursor;

    return(xf86InitCursor(pScreen, infoPtr));
}

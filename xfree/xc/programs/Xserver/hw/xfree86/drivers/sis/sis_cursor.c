/*
 * Copyright 1998,1999 by Alan Hourihane, Wigan, England.
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Alan Hourihane not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  Alan Hourihane makes no representations
 * about the suitability of this software for any purpose.  It is provided
 * "as is" without express or implied warranty.
 *
 * ALAN HOURIHANE DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL ALAN HOURIHANE BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 *
 * Authors:  Alan Hourihane, alanh@fairlite.demon.co.uk
 *       Mike Chapman <mike@paranoia.com>,
 *       Juanjo Santamarta <santamarta@ctv.es>,
 *       Mitani Hiroshi <hmitani@drl.mei.co.jp>
 *       David Thomas <davtom@dream.org.uk>.
 */
/* $XFree86: xc/programs/Xserver/hw/xfree86/drivers/sis/sis_cursor.c,v 1.5 2001/05/07 21:59:07 tsi Exp $ */

#include "xf86.h"
#include "xf86PciInfo.h"
#include "cursorstr.h"
#include "vgaHW.h"

#include "sis.h"
#include "sis_cursor.h"


Bool SiSHWCursorInit(ScreenPtr pScreen);

static void
SiSShowCursor(ScrnInfoPtr pScrn)
{
        unsigned char temp;

        outw(VGA_SEQ_INDEX, 0x8605);    /* Unlock Registers */
        outb(VGA_SEQ_INDEX, 0x06);
        temp = inb(VGA_SEQ_DATA) | 0x40;
        outb(VGA_SEQ_DATA, temp);
}

static void
SiS300ShowCursor(ScrnInfoPtr pScrn)
{
    SISPtr  pSiS = SISPTR(pScrn);

    sis300EnableHWCursor()
    if (pSiS->VBFlags & CRT2_ENABLE)  {
        sis301EnableHWCursor();
    }
}

static void
SiSHideCursor(ScrnInfoPtr pScrn)
{
        unsigned char temp;

        outw(VGA_SEQ_INDEX, 0x8605);    /* Unlock Registers */
        outb(VGA_SEQ_INDEX, 0x06);
        temp = inb(VGA_SEQ_DATA) & 0xBF;
        outb(VGA_SEQ_DATA, temp);
}

static void
SiS300HideCursor(ScrnInfoPtr pScrn)
{
    SISPtr  pSiS = SISPTR(pScrn);

    sis300DisableHWCursor()
    if (pSiS->VBFlags & CRT2_ENABLE)  {
        sis301DisableHWCursor()
    }
}

static void
SiSSetCursorPosition(ScrnInfoPtr pScrn, int x, int y)
{
    unsigned char   x_preset = 0;
    unsigned char   y_preset = 0;
    int         temp;

    outw(VGA_SEQ_INDEX, 0x8605);    /* Unlock Registers */

    if (x < 0) {
        x_preset = (-x);
        x = 0;
    }
    if (y < 0) {
        y_preset = (-y);
        y = 0;
    }
    outw(VGA_SEQ_INDEX, (x&0xFF)<<8 | 0x1A);
    outw(VGA_SEQ_INDEX, (x&0xFF00)  | 0x1B);
    outw(VGA_SEQ_INDEX, (y&0xFF)<<8 | 0x1D);
    outb(VGA_SEQ_INDEX, 0x1E);
    temp = inb(VGA_SEQ_DATA) & 0xF8;
    outw(VGA_SEQ_INDEX, ((y&0x0700) | (temp<<8))  | 0x1E);
    outw(VGA_SEQ_INDEX, x_preset<<8 | 0x1C);
    outw(VGA_SEQ_INDEX, y_preset<<8 | 0x1F);
}

static void
SiS300SetCursorPosition(ScrnInfoPtr pScrn, int x, int y)
{
    SISPtr  pSiS = SISPTR(pScrn);

    unsigned char   x_preset = 0;
    unsigned char   y_preset = 0;

    if (x < 0) {
        x_preset = (-x);
        x = 0;
    }
    if (y < 0) {
        y_preset = (-y);
        y = 0;
    }
    sis300SetCursorPositionX(x, x_preset)
    sis300SetCursorPositionY(y, y_preset)
    if (pSiS->VBFlags & CRT2_ENABLE)  {
        sis301SetCursorPositionX(x+13, x_preset)
        sis301SetCursorPositionY(y, y_preset)
    }
}

static void
SiSSetCursorColors(ScrnInfoPtr pScrn, int bg, int fg)
{
    unsigned char f_red, f_green, f_blue;
    unsigned char b_red, b_green, b_blue;

    outw(VGA_SEQ_INDEX, 0x8605);

    f_red   = (fg & 0x00FF0000) >> (16+2);
    f_green = (fg & 0x0000FF00) >> (8+2);
    f_blue  = (fg & 0x000000FF) >> 2;
    b_red   = (bg & 0x00FF0000) >> (16+2);
    b_green = (bg & 0x0000FF00) >> (8+2);
    b_blue  = (bg & 0x000000FF) >> 2;

    outw(VGA_SEQ_INDEX, b_red   <<8  | 0x14);
    outw(VGA_SEQ_INDEX, b_green <<8  | 0x15);
    outw(VGA_SEQ_INDEX, b_blue  <<8  | 0x16);
    outw(VGA_SEQ_INDEX, f_red   <<8  | 0x17);
    outw(VGA_SEQ_INDEX, f_green <<8  | 0x18);
    outw(VGA_SEQ_INDEX, f_blue  <<8  | 0x19);
}

static void
SiS300SetCursorColors(ScrnInfoPtr pScrn, int bg, int fg)
{
    SISPtr pSiS = SISPTR(pScrn);

    sis300SetCursorBGColor(bg)
    sis300SetCursorFGColor(fg)
    if (pSiS->VBFlags & CRT2_ENABLE)  {
        sis301SetCursorBGColor(bg)
        sis301SetCursorFGColor(fg)
    }
}

static void
SiSLoadCursorImage(ScrnInfoPtr pScrn, unsigned char *src)
{
    SISPtr pSiS = SISPTR(pScrn);
    int cursor_addr;
    unsigned char   temp;

    outw(VGA_SEQ_INDEX, 0x8605);

    cursor_addr = pScrn->videoRam - 1;
    memcpy((unsigned char *)pSiS->FbBase + cursor_addr * 1024, src, 1024);

    /* copy bits [21:18] into the top bits of SR38 */
    outb(VGA_SEQ_INDEX, 0x38);
    temp = inb(VGA_SEQ_DATA) & 0x0F;
    outb(VGA_SEQ_DATA, temp | ((cursor_addr & 0xF00) >> 4));

    /* if set, store the bit [22] to SR3E */
    if (cursor_addr & 0x1000) {
        outb(VGA_SEQ_INDEX, 0x3E);
                temp = inb(VGA_SEQ_DATA) | 0x04;
                outb(VGA_SEQ_DATA, temp);
    }

    /* set HW cursor pattern, use pattern 0xF */
    outb(VGA_SEQ_INDEX, 0x1E);
        temp = inb(VGA_SEQ_DATA) | 0xF0;
        outb(VGA_SEQ_DATA, temp);

    /* disable the hardware cursor side pattern */
    outb(VGA_SEQ_INDEX, 0x1E);
        temp = inb(VGA_SEQ_DATA) & 0xF7;
        outb(VGA_SEQ_DATA, temp);
}

static void
SiS300LoadCursorImage(ScrnInfoPtr pScrn, unsigned char *src)
{
    SISPtr pSiS = SISPTR(pScrn);
    int cursor_addr;

    if (pSiS->TurboQueue)
        cursor_addr = pScrn->videoRam-512-1;    /* 1K boundary */
    else
        cursor_addr = pScrn->videoRam - 1;  /* 1K boundary */

	ErrorF("cursor_addr value: %x\n");

    memcpy((unsigned char *)pSiS->FbBase + cursor_addr * 1024, src, 1024);
    sis300SetCursorAddress(cursor_addr)
    sis300SetCursorPatternSelect(0)
    if (pSiS->VBFlags & CRT2_ENABLE)  {
        sis301SetCursorAddress(cursor_addr)
        sis301SetCursorPatternSelect(0)
    }
}

static Bool
SiSUseHWCursor(ScreenPtr pScreen, CursorPtr pCurs)
{
    return TRUE;
}

static Bool
SiS300UseHWCursor(ScreenPtr pScreen, CursorPtr pCurs)
{
    ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
    DisplayModePtr  mode = pScrn->currentMode;
    SISPtr  pSiS = SISPTR(pScrn);

    switch (pSiS->Chipset)  {
    case PCI_CHIP_SIS300:
    case PCI_CHIP_SIS630:
        if (mode->Flags & V_INTERLACE)
            return FALSE;
        break;
    }
    return TRUE;
}

Bool
SiSHWCursorInit(ScreenPtr pScreen)
{
    ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
    SISPtr pSiS = SISPTR(pScrn);
    xf86CursorInfoPtr infoPtr;

    PDEBUG(ErrorF("HW Cursor Init\n"));
    infoPtr = xf86CreateCursorInfoRec();
    if(!infoPtr) 
		return FALSE;

    pSiS->CursorInfoPtr = infoPtr;

    infoPtr->MaxWidth = 64;
    infoPtr->MaxHeight = 64;
    switch (pSiS->Chipset)  {
    case PCI_CHIP_SIS300:
    case PCI_CHIP_SIS630:
    case PCI_CHIP_SIS540:
        infoPtr->ShowCursor = SiS300ShowCursor;
        infoPtr->HideCursor = SiS300HideCursor;
        infoPtr->SetCursorPosition = SiS300SetCursorPosition;
        infoPtr->SetCursorColors = SiS300SetCursorColors;
        infoPtr->LoadCursorImage = SiS300LoadCursorImage;
        infoPtr->UseHWCursor = SiS300UseHWCursor;
        infoPtr->Flags =
            HARDWARE_CURSOR_TRUECOLOR_AT_8BPP |
            HARDWARE_CURSOR_INVERT_MASK |
            HARDWARE_CURSOR_BIT_ORDER_MSBFIRST |
            HARDWARE_CURSOR_AND_SOURCE_WITH_MASK |
            HARDWARE_CURSOR_SWAP_SOURCE_AND_MASK |
            HARDWARE_CURSOR_SOURCE_MASK_INTERLEAVE_64;
		break;
    default:
        infoPtr->SetCursorPosition = SiSSetCursorPosition;
        infoPtr->ShowCursor = SiSShowCursor;
        infoPtr->HideCursor = SiSHideCursor;
        infoPtr->SetCursorColors = SiSSetCursorColors;
        infoPtr->LoadCursorImage = SiSLoadCursorImage;
        infoPtr->UseHWCursor = SiSUseHWCursor;
        infoPtr->Flags =
            HARDWARE_CURSOR_TRUECOLOR_AT_8BPP |
            HARDWARE_CURSOR_INVERT_MASK |
            HARDWARE_CURSOR_BIT_ORDER_MSBFIRST |
            HARDWARE_CURSOR_AND_SOURCE_WITH_MASK |
            HARDWARE_CURSOR_NIBBLE_SWAPPED |
            HARDWARE_CURSOR_SOURCE_MASK_INTERLEAVE_1;
        break;
    }

    return(xf86InitCursor(pScreen, infoPtr));
}

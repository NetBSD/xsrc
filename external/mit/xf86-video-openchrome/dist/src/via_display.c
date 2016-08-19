/*
 * Copyright 2005-2015 The Openchrome Project
 *                     [http://www.freedesktop.org/wiki/Openchrome]
 * Copyright 2004-2005 The Unichrome Project  [unichrome.sf.net]
 * Copyright 1998-2003 VIA Technologies, Inc. All Rights Reserved.
 * Copyright 2001-2003 S3 Graphics, Inc. All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sub license,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "via_driver.h"

/*
 * Enables the second display channel.
 */
void
ViaSecondDisplayChannelEnable(ScrnInfoPtr pScrn)
{
    vgaHWPtr hwp = VGAHWPTR(pScrn);

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                     "ViaSecondDisplayChannelEnable\n"));
    ViaCrtcMask(hwp, 0x6A, 0x00, 1 << 6);
    ViaCrtcMask(hwp, 0x6A, 1 << 7, 1 << 7);
    ViaCrtcMask(hwp, 0x6A, 1 << 6, 1 << 6);
}

/*
 * Disables the second display channel.
 */
void
ViaSecondDisplayChannelDisable(ScrnInfoPtr pScrn)
{
    vgaHWPtr hwp = VGAHWPTR(pScrn);

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                     "ViaSecondDisplayChannelDisable\n"));

    ViaCrtcMask(hwp, 0x6A, 0x00, 1 << 6);
    ViaCrtcMask(hwp, 0x6A, 0x00, 1 << 7);
    ViaCrtcMask(hwp, 0x6A, 1 << 6, 1 << 6);
}

/*
 * Initial settings for displays.
 */
void
ViaDisplayInit(ScrnInfoPtr pScrn)
{
    VIAPtr pVia = VIAPTR(pScrn);
    vgaHWPtr hwp = VGAHWPTR(pScrn);

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "ViaDisplayInit\n"));

    ViaSecondDisplayChannelDisable(pScrn);
    ViaCrtcMask(hwp, 0x6A, 0x00, 0x3D);

    hwp->writeCrtc(hwp, 0x6B, 0x00);
    hwp->writeCrtc(hwp, 0x6C, 0x00);
    hwp->writeCrtc(hwp, 0x79, 0x00);

    /* (IGA1 Timing Plus 2, added in VT3259 A3 or later) */
    if (pVia->Chipset != VIA_CLE266 && pVia->Chipset != VIA_KM400)
        ViaCrtcMask(hwp, 0x47, 0x00, 0xC8);
}

/*
 * Enables simultaneous mode.
 */
void
ViaDisplayEnableSimultaneous(ScrnInfoPtr pScrn)
{
    vgaHWPtr hwp = VGAHWPTR(pScrn);

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                     "ViaDisplayEnableSimultaneous\n"));
    ViaCrtcMask(hwp, 0x6B, 0x08, 0x08);
}

/*
 * Disables simultaneous mode.
 */
void
ViaDisplayDisableSimultaneous(ScrnInfoPtr pScrn)
{
    vgaHWPtr hwp = VGAHWPTR(pScrn);

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                     "ViaDisplayDisableSimultaneous\n"));
    ViaCrtcMask(hwp, 0x6B, 0x00, 0x08);
}

/*
 * Sets the primary or secondary display stream on internal TMDS.
 */
void
ViaDisplaySetStreamOnDFP(ScrnInfoPtr pScrn, Bool primary)
{
    vgaHWPtr hwp = VGAHWPTR(pScrn);

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "ViaDisplaySetStreamOnDFP\n"));

    if (primary)
        ViaCrtcMask(hwp, 0x99, 0x00, 0x10);
    else
        ViaCrtcMask(hwp, 0x99, 0x10, 0x10);
}

static void
ViaCRTCSetGraphicsRegisters(ScrnInfoPtr pScrn)
{
    vgaHWPtr hwp = VGAHWPTR(pScrn);

    /* graphics registers */
    hwp->writeGr(hwp, 0x00, 0x00);
    hwp->writeGr(hwp, 0x01, 0x00);
    hwp->writeGr(hwp, 0x02, 0x00);
    hwp->writeGr(hwp, 0x03, 0x00);
    hwp->writeGr(hwp, 0x04, 0x00);
    hwp->writeGr(hwp, 0x05, 0x40);
    hwp->writeGr(hwp, 0x06, 0x05);
    hwp->writeGr(hwp, 0x07, 0x0F);
    hwp->writeGr(hwp, 0x08, 0xFF);

    ViaGrMask(hwp, 0x20, 0, 0xFF);
    ViaGrMask(hwp, 0x21, 0, 0xFF);
    ViaGrMask(hwp, 0x22, 0, 0xFF);
}

static void
ViaCRTCSetAttributeRegisters(ScrnInfoPtr pScrn)
{
    vgaHWPtr hwp = VGAHWPTR(pScrn);
    CARD8 i;

    /* attribute registers */
    for (i = 0; i <= 0xF; i++) {
        hwp->writeAttr(hwp, i, i);
    }
    hwp->writeAttr(hwp, 0x10, 0x41);
    hwp->writeAttr(hwp, 0x11, 0xFF);
    hwp->writeAttr(hwp, 0x12, 0x0F);
    hwp->writeAttr(hwp, 0x13, 0x00);
    hwp->writeAttr(hwp, 0x14, 0x00);
}

void
VIALoadRgbLut(ScrnInfoPtr pScrn, int start, int numColors, LOCO *colors)
{
    vgaHWPtr hwp = VGAHWPTR(pScrn);
    int i, j;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "VIALoadRgbLut\n"));

    hwp->enablePalette(hwp);
    hwp->writeDacMask(hwp, 0xFF);

    /* We need the same palette contents for both 16 and 24 bits, but X doesn't
     * play: X's colormap handling is hopelessly intertwined with almost every
     * X subsystem.  So we just space out RGB values over the 256*3. */

    switch (pScrn->bitsPerPixel) {
        case 16:
            for (i = start; i < numColors; i++) {
                hwp->writeDacWriteAddr(hwp, i * 4);
                for (j = 0; j < 4; j++) {
                    hwp->writeDacData(hwp, colors[i / 2].red);
                    hwp->writeDacData(hwp, colors[i].green);
                    hwp->writeDacData(hwp, colors[i / 2].blue);
                }
            }
            break;
        case 8:
        case 24:
        case 32:
            for (i = start; i < numColors; i++) {
                hwp->writeDacWriteAddr(hwp, i);
                hwp->writeDacData(hwp, colors[i].red);
                hwp->writeDacData(hwp, colors[i].green);
                hwp->writeDacData(hwp, colors[i].blue);
            }
            break;
        default:
            xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
                       "Unsupported bitdepth: %d\n", pScrn->bitsPerPixel);
            break;
    }
    hwp->disablePalette(hwp);
}

void
ViaGammaDisable(ScrnInfoPtr pScrn)
{
    VIAPtr pVia = VIAPTR(pScrn);
    vgaHWPtr hwp = VGAHWPTR(pScrn);

    switch (pVia->Chipset) {
        case VIA_CLE266:
        case VIA_KM400:
            ViaSeqMask(hwp, 0x16, 0x00, 0x80);
            break;
        default:
            ViaCrtcMask(hwp, 0x33, 0x00, 0x80);
            break;
    }

    /* Disable gamma on secondary */
    /* This is needed or the hardware will lockup */
    ViaSeqMask(hwp, 0x1A, 0x00, 0x01);
    ViaCrtcMask(hwp, 0x6A, 0x00, 0x02);
    switch (pVia->Chipset) {
        case VIA_CLE266:
        case VIA_KM400:
        case VIA_K8M800:
        case VIA_PM800:
            break;
        default:
            ViaCrtcMask(hwp, 0x6A, 0x00, 0x20);
            break;
    }
}

void
ViaCRTCInit(ScrnInfoPtr pScrn)
{
    vgaHWPtr hwp = VGAHWPTR(pScrn);

    hwp->writeSeq(hwp, 0x10, 0x01); /* unlock extended registers */
    ViaCrtcMask(hwp, 0x47, 0x00, 0x01); /* unlock CRT registers */
    ViaCRTCSetGraphicsRegisters(pScrn);
    ViaCRTCSetAttributeRegisters(pScrn);
}

void
ViaFirstCRTCSetMode(ScrnInfoPtr pScrn, DisplayModePtr mode)
{
    vgaHWPtr hwp = VGAHWPTR(pScrn);
    VIAPtr pVia = VIAPTR(pScrn);
    CARD16 temp;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "ViaFirstCRTCSetMode\n"));

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "Setting up %s\n", mode->name));

    ViaCrtcMask(hwp, 0x11, 0x00, 0x80); /* modify starting address */
    ViaCrtcMask(hwp, 0x03, 0x80, 0x80); /* enable vertical retrace access */

    /* Set Misc Register */
    temp = 0x23;
    if (mode->Flags & V_NHSYNC)
        temp |= 0x40;
    if (mode->Flags & V_NVSYNC)
        temp |= 0x80;
    temp |= 0x0C; /* Undefined/external clock */
    hwp->writeMiscOut(hwp, temp);

    /* Sequence registers */
    hwp->writeSeq(hwp, 0x00, 0x00);

#if 0
    if (mode->Flags & V_CLKDIV2)
        hwp->writeSeq(hwp, 0x01, 0x09);
    else
#endif
        hwp->writeSeq(hwp, 0x01, 0x01);

    hwp->writeSeq(hwp, 0x02, 0x0F);
    hwp->writeSeq(hwp, 0x03, 0x00);
    hwp->writeSeq(hwp, 0x04, 0x0E);

    ViaSeqMask(hwp, 0x15, 0x02, 0x02);

    /* bpp */
    switch (pScrn->bitsPerPixel) {
        case 8:
            /* Only CLE266.AX use 6bits LUT. */
            if (pVia->Chipset == VIA_CLE266 && pVia->ChipRev < 15)
                ViaSeqMask(hwp, 0x15, 0x22, 0xFE);
            else
                ViaSeqMask(hwp, 0x15, 0xA2, 0xFE);
            break;
        case 16:
            ViaSeqMask(hwp, 0x15, 0xB6, 0xFE);
            break;
        case 24:
        case 32:
            ViaSeqMask(hwp, 0x15, 0xAE, 0xFE);
            break;
        default:
            xf86DrvMsg(pScrn->scrnIndex, X_ERROR, "Unhandled bitdepth: %d\n",
                       pScrn->bitsPerPixel);
            break;
    }

    switch (pVia->ChipId) {
        case VIA_CX700:
        case VIA_K8M890:
        case VIA_P4M900:
        case VIA_VX800:
        case VIA_VX855:
        case VIA_VX900:
            break;
        default:
            ViaSeqMask(hwp, 0x16, 0x08, 0xBF);
            ViaSeqMask(hwp, 0x17, 0x1F, 0xFF);
            ViaSeqMask(hwp, 0x18, 0x4E, 0xFF);
            ViaSeqMask(hwp, 0x1A, 0x08, 0xF9);
            break;
    }

    /* Crtc registers */
    /* horizontal total : 4100 */
    temp = (mode->CrtcHTotal >> 3) - 5;
    hwp->writeCrtc(hwp, 0x00, temp & 0xFF);
    ViaCrtcMask(hwp, 0x36, temp >> 5, 0x08);

    /* horizontal address : 2048 */
    temp = (mode->CrtcHDisplay >> 3) - 1;
    hwp->writeCrtc(hwp, 0x01, temp & 0xFF);

    /* horizontal blanking start : 2048 */
    /* temp = (mode->CrtcHDisplay >> 3) - 1; */
    temp = (mode->CrtcHBlankStart >> 3) - 1;
    hwp->writeCrtc(hwp, 0x02, temp & 0xFF);
    /* If HblankStart has more bits anywhere, add them here */

    /* horizontal blanking end : start + 1025 */
    /* temp = (mode->CrtcHTotal >> 3) - 1; */
    temp = (mode->CrtcHBlankEnd >> 3) - 1;
    ViaCrtcMask(hwp, 0x03, temp, 0x1F);
    ViaCrtcMask(hwp, 0x05, temp << 2, 0x80);
    ViaCrtcMask(hwp, 0x33, temp >> 1, 0x20);

    /* CrtcHSkew ??? */

    /* horizontal sync start : 4095 */
    temp = mode->CrtcHSyncStart >> 3;
    hwp->writeCrtc(hwp, 0x04, temp & 0xFF);
    ViaCrtcMask(hwp, 0x33, temp >> 4, 0x10);

    /* horizontal sync end : start + 256 */
    temp = mode->CrtcHSyncEnd >> 3;
    ViaCrtcMask(hwp, 0x05, temp, 0x1F);

    /* vertical total : 2049 */
    temp = mode->CrtcVTotal - 2;
    hwp->writeCrtc(hwp, 0x06, temp & 0xFF);
    ViaCrtcMask(hwp, 0x07, temp >> 8, 0x01);
    ViaCrtcMask(hwp, 0x07, temp >> 4, 0x20);
    ViaCrtcMask(hwp, 0x35, temp >> 10, 0x01);

    /* vertical address : 2048 */
    temp = mode->CrtcVDisplay - 1;
    hwp->writeCrtc(hwp, 0x12, temp & 0xFF);
    ViaCrtcMask(hwp, 0x07, temp >> 7, 0x02);
    ViaCrtcMask(hwp, 0x07, temp >> 3, 0x40);
    ViaCrtcMask(hwp, 0x35, temp >> 8, 0x04);

    /* Primary starting address -> 0x00, adjustframe does the rest */
    hwp->writeCrtc(hwp, 0x0C, 0x00);
    hwp->writeCrtc(hwp, 0x0D, 0x00);
    ViaCrtcMask(hwp, 0x48, 0x00, 0x03); /* is this even possible on CLE266A ? */
    hwp->writeCrtc(hwp, 0x34, 0x00);

    /* vertical sync start : 2047 */
    temp = mode->CrtcVSyncStart;
    hwp->writeCrtc(hwp, 0x10, temp & 0xFF);
    ViaCrtcMask(hwp, 0x07, temp >> 6, 0x04);
    ViaCrtcMask(hwp, 0x07, temp >> 2, 0x80);
    ViaCrtcMask(hwp, 0x35, temp >> 9, 0x02);

    /* vertical sync end : start + 16 -- other bits someplace? */
    ViaCrtcMask(hwp, 0x11, mode->CrtcVSyncEnd, 0x0F);

    /* line compare: We are not doing splitscreen so 0x3FFF */
    hwp->writeCrtc(hwp, 0x18, 0xFF);
    ViaCrtcMask(hwp, 0x07, 0x10, 0x10);
    ViaCrtcMask(hwp, 0x09, 0x40, 0x40);
    ViaCrtcMask(hwp, 0x33, 0x06, 0x07);
    ViaCrtcMask(hwp, 0x35, 0x10, 0x10);

    /* zero Maximum scan line */
    ViaCrtcMask(hwp, 0x09, 0x00, 0x1F);
    hwp->writeCrtc(hwp, 0x14, 0x00);

    /* vertical blanking start : 2048 */
    /* temp = mode->CrtcVDisplay - 1; */
    temp = mode->CrtcVBlankStart - 1;
    hwp->writeCrtc(hwp, 0x15, temp & 0xFF);
    ViaCrtcMask(hwp, 0x07, temp >> 5, 0x08);
    ViaCrtcMask(hwp, 0x09, temp >> 4, 0x20);
    ViaCrtcMask(hwp, 0x35, temp >> 7, 0x08);

    /* vertical blanking end : start + 257 */
    /* temp = mode->CrtcVTotal - 1; */
    temp = mode->CrtcVBlankEnd - 1;
    hwp->writeCrtc(hwp, 0x16, temp);

    /* FIXME: check if this is really necessary here */
    switch (pVia->ChipId) {
        case VIA_CX700:
        case VIA_K8M890:
        case VIA_P4M900:
        case VIA_VX800:
        case VIA_VX855:
        case VIA_VX900:
            break;
        default:
            /* some leftovers */
            hwp->writeCrtc(hwp, 0x08, 0x00);
            ViaCrtcMask(hwp, 0x32, 0, 0xFF);  /* ? */
            ViaCrtcMask(hwp, 0x33, 0, 0xC8);
            break;
    }

    /* offset */
    temp = (pScrn->displayWidth * (pScrn->bitsPerPixel >> 3)) >> 3;
    /* Make sure that this is 32-byte aligned. */
    if (temp & 0x03) {
        temp += 0x03;
        temp &= ~0x03;
    }
    hwp->writeCrtc(hwp, 0x13, temp & 0xFF);
    ViaCrtcMask(hwp, 0x35, temp >> 3, 0xE0);

    /* fetch count */
    temp = (mode->CrtcHDisplay * (pScrn->bitsPerPixel >> 3)) >> 3;
    /* Make sure that this is 32-byte aligned. */
    if (temp & 0x03) {
        temp += 0x03;
        temp &= ~0x03;
    }

    hwp->writeSeq(hwp, 0x1C, ((temp >> 1)+1) & 0xFF);
    ViaSeqMask(hwp, 0x1D, temp >> 9, 0x03);

    switch (pVia->ChipId) {
        case VIA_CX700:
        case VIA_K8M890:
        case VIA_P4M900:
        case VIA_VX800:
        case VIA_VX855:
        case VIA_VX900:
            break;
        default:
            /* some leftovers */
            ViaCrtcMask(hwp, 0x32, 0, 0xFF);
            ViaCrtcMask(hwp, 0x33, 0, 0xC8);
            break;
    }
}

void
ViaFirstCRTCSetStartingAddress(xf86CrtcPtr crtc, int x, int y)
{
    drmmode_crtc_private_ptr drmmode_crtc = crtc->driver_private;
    drmmode_ptr drmmode = drmmode_crtc->drmmode;
    ScrnInfoPtr pScrn = crtc->scrn;
    VIAPtr pVia = VIAPTR(pScrn);
    vgaHWPtr hwp = VGAHWPTR(pScrn);
    CARD32 Base;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "ViaFirstCRTCSetStartingAddress\n"));

    Base = (y * pScrn->displayWidth + x) * (pScrn->bitsPerPixel / 8);
    Base = (Base + drmmode->front_bo->offset) >> 1;

    hwp->writeCrtc(hwp, 0x0C, (Base & 0xFF00) >> 8);
    hwp->writeCrtc(hwp, 0x0D, Base & 0xFF);
    /* FIXME The proper starting address for CR48 is 0x1F - Bits[28:24] */
    if (!(pVia->Chipset == VIA_CLE266 && CLE266_REV_IS_AX(pVia->ChipRev)))
        ViaCrtcMask(hwp, 0x48, Base >> 24, 0x0F);
    /* CR34 are fire bits. Must be written after CR0C CR0D CR48.  */
    hwp->writeCrtc(hwp, 0x34, (Base & 0xFF0000) >> 16);
}

void
ViaSecondCRTCSetStartingAddress(xf86CrtcPtr crtc, int x, int y)
{
    drmmode_crtc_private_ptr drmmode_crtc = crtc->driver_private;
    drmmode_ptr drmmode = drmmode_crtc->drmmode;
    ScrnInfoPtr pScrn = crtc->scrn;
    vgaHWPtr hwp = VGAHWPTR(pScrn);
    CARD32 Base, tmp;

    Base = (y * pScrn->displayWidth + x) * (pScrn->bitsPerPixel / 8);
    Base = (Base + drmmode->front_bo->offset) >> 3;

    tmp = hwp->readCrtc(hwp, 0x62) & 0x01;
    tmp |= (Base & 0x7F) << 1;
    hwp->writeCrtc(hwp, 0x62, tmp);

    hwp->writeCrtc(hwp, 0x63, (Base & 0x7F80) >> 7);
    hwp->writeCrtc(hwp, 0x64, (Base & 0x7F8000) >> 15);
    hwp->writeCrtc(hwp, 0xA3, (Base & 0x03800000) >> 23);
}

void
ViaSecondCRTCHorizontalQWCount(ScrnInfoPtr pScrn, int width)
{
    vgaHWPtr hwp = VGAHWPTR(pScrn);
    CARD16 temp;

    /* fetch count */
    temp = (width * (pScrn->bitsPerPixel >> 3)) >> 3;
    /* Make sure that this is 32-byte aligned. */
    if (temp & 0x03) {
        temp += 0x03;
        temp &= ~0x03;
    }
    hwp->writeCrtc(hwp, 0x65, (temp >> 1) & 0xFF);
    ViaCrtcMask(hwp, 0x67, temp >> 7, 0x0C);
}

void
ViaSecondCRTCHorizontalOffset(ScrnInfoPtr pScrn)
{
    vgaHWPtr hwp = VGAHWPTR(pScrn);
    CARD16 temp;

    /* offset */
    temp = (pScrn->displayWidth * (pScrn->bitsPerPixel >> 3)) >> 3;
    /* Make sure that this is 32-byte aligned. */
    if (temp & 0x03) {
        temp += 0x03;
        temp &= ~0x03;
        }

    hwp->writeCrtc(hwp, 0x66, temp & 0xFF);
    ViaCrtcMask(hwp, 0x67, temp >> 8, 0x03);
}

void
ViaSecondCRTCSetMode(ScrnInfoPtr pScrn, DisplayModePtr mode)
{
    VIAPtr pVia = VIAPTR(pScrn);
    vgaHWPtr hwp = VGAHWPTR(pScrn);
    CARD16 temp;

    xf86DrvMsg(pScrn->scrnIndex, X_INFO, "mode: %p\n", mode);
    xf86DrvMsg(pScrn->scrnIndex, X_INFO, "mode->name: %p\n", mode->name);
    xf86DrvMsg(pScrn->scrnIndex, X_INFO, "mode->name: %s\n", mode->name);


    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "ViaSecondCRTCSetMode\n"));
    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "Setting up %s\n", mode->name));
    /* bpp */
    switch (pScrn->bitsPerPixel) {
        case 8:
            ViaCrtcMask(hwp, 0x67, 0x00, 0xC0);
            break;
        case 16:
            ViaCrtcMask(hwp, 0x67, 0x40, 0xC0);
            break;
        case 24:
        case 32:
            ViaCrtcMask(hwp, 0x67, 0xC0, 0xC0);
            break;
        default:
            xf86DrvMsg(pScrn->scrnIndex, X_ERROR, "Unhandled bitdepth: %d\n",
                       pScrn->bitsPerPixel);
            break;
    }

    switch (pVia->ChipId) {
        case VIA_CX700:
        case VIA_K8M890:
        case VIA_P4M900:
        case VIA_VX800:
        case VIA_VX855:
        case VIA_VX900:
            break;
        default:
            ViaSeqMask(hwp, 0x16, 0x08, 0xBF);
            ViaSeqMask(hwp, 0x17, 0x1F, 0xFF);
            ViaSeqMask(hwp, 0x18, 0x4E, 0xFF);
            ViaSeqMask(hwp, 0x1A, 0x08, 0xF9);
            break;
    }

    /* Crtc registers */
    /* horizontal total : 4096 */
    temp = mode->CrtcHTotal - 1;
    hwp->writeCrtc(hwp, 0x50, temp & 0xFF);
    ViaCrtcMask(hwp, 0x55, temp >> 8, 0x0F);

    /* horizontal address : 2048 */
    temp = mode->CrtcHDisplay - 1;
    hwp->writeCrtc(hwp, 0x51, temp & 0xFF);
    ViaCrtcMask(hwp, 0x55, temp >> 4, 0x70);

    /* horizontal blanking start : 2048 */
    /* temp = mode->CrtcHDisplay - 1; */
    temp = mode->CrtcHBlankStart - 1;
    hwp->writeCrtc(hwp, 0x52, temp & 0xFF);
    ViaCrtcMask(hwp, 0x54, temp >> 8, 0x07);

    /* horizontal blanking end : 4096 */
    /* temp = mode->CrtcHTotal - 1; */
    temp = mode->CrtcHBlankEnd - 1;
    hwp->writeCrtc(hwp, 0x53, temp & 0xFF);
    ViaCrtcMask(hwp, 0x54, temp >> 5, 0x38);
    ViaCrtcMask(hwp, 0x5D, temp >> 5, 0x40);

    /* horizontal sync start : 2047 */
    temp = mode->CrtcHSyncStart;
    hwp->writeCrtc(hwp, 0x56, temp & 0xFF);
    ViaCrtcMask(hwp, 0x54, temp >> 2, 0xC0);
    ViaCrtcMask(hwp, 0x5C, temp >> 3, 0x80);

    if (pVia->ChipId != VIA_CLE266 && pVia->ChipId != VIA_KM400)
        ViaCrtcMask(hwp, 0x5D, temp >> 4, 0x80);

    /* horizontal sync end : sync start + 512 */
    temp = mode->CrtcHSyncEnd;
    hwp->writeCrtc(hwp, 0x57, temp & 0xFF);
    ViaCrtcMask(hwp, 0x5C, temp >> 2, 0x40);

    /* vertical total : 2048 */
    temp = mode->CrtcVTotal - 1;
    hwp->writeCrtc(hwp, 0x58, temp & 0xFF);
    ViaCrtcMask(hwp, 0x5D, temp >> 8, 0x07);


    /* vertical address : 2048 */
    temp = mode->CrtcVDisplay - 1;
    hwp->writeCrtc(hwp, 0x59, temp & 0xFF);
    ViaCrtcMask(hwp, 0x5D, temp >> 5, 0x38);

    /* vertical blanking start : 2048 */
    /* temp = mode->CrtcVDisplay - 1; */
    temp = mode->CrtcVBlankStart - 1;
    hwp->writeCrtc(hwp, 0x5A, temp & 0xFF);
    ViaCrtcMask(hwp, 0x5C, temp >> 8, 0x07);

    /* vertical blanking end : 2048 */
    /* temp = mode->CrtcVTotal - 1; */
    temp = mode->CrtcVBlankEnd - 1;
    hwp->writeCrtc(hwp, 0x5B, temp & 0xFF);
    ViaCrtcMask(hwp, 0x5C, temp >> 5, 0x38);

    /* vertical sync start : 2047 */
    temp = mode->CrtcVSyncStart;
    hwp->writeCrtc(hwp, 0x5E, temp & 0xFF);
    ViaCrtcMask(hwp, 0x5F, temp >> 3, 0xE0);

    /* vertical sync end : start + 32 */
    temp = mode->CrtcVSyncEnd;
    ViaCrtcMask(hwp, 0x5F, temp, 0x1F);

    switch (pVia->ChipId) {
        case VIA_CX700:
        case VIA_K8M890:
        case VIA_P4M900:
        case VIA_VX800:
        case VIA_VX855:
        case VIA_VX900:
            break;
        default:
            /* some leftovers */
            hwp->writeCrtc(hwp, 0x08, 0x00);
            ViaCrtcMask(hwp, 0x32, 0, 0xFF);  /* ? */
            ViaCrtcMask(hwp, 0x33, 0, 0xC8);
            break;
    }

    ViaSecondCRTCHorizontalOffset(pScrn);
    ViaSecondCRTCHorizontalQWCount(pScrn, mode->CrtcHDisplay);
}

/*
 * Checks for limitations imposed by the available VGA timing registers.
 */
static ModeStatus
ViaFirstCRTCModeValid(ScrnInfoPtr pScrn, DisplayModePtr mode)
{
    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "ViaFirstCRTCModeValid\n"));

    if (mode->CrtcHTotal > 4100)
        return MODE_BAD_HVALUE;

    if (mode->CrtcHDisplay > 2048)
        return MODE_BAD_HVALUE;

    if (mode->CrtcHBlankStart > 2048)
        return MODE_BAD_HVALUE;

    if ((mode->CrtcHBlankEnd - mode->CrtcHBlankStart) > 1025)
        return MODE_HBLANK_WIDE;

    if (mode->CrtcHSyncStart > 4095)
        return MODE_BAD_HVALUE;

    if ((mode->CrtcHSyncEnd - mode->CrtcHSyncStart) > 256)
        return MODE_HSYNC_WIDE;

    if (mode->CrtcVTotal > 2049)
        return MODE_BAD_VVALUE;

    if (mode->CrtcVDisplay > 2048)
        return MODE_BAD_VVALUE;

    if (mode->CrtcVSyncStart > 2047)
        return MODE_BAD_VVALUE;

    if ((mode->CrtcVSyncEnd - mode->CrtcVSyncStart) > 16)
        return MODE_VSYNC_WIDE;

    if (mode->CrtcVBlankStart > 2048)
        return MODE_BAD_VVALUE;

    if ((mode->CrtcVBlankEnd - mode->CrtcVBlankStart) > 257)
        return MODE_VBLANK_WIDE;

    return MODE_OK;
}

static ModeStatus
ViaSecondCRTCModeValid(ScrnInfoPtr pScrn, DisplayModePtr mode)
{
    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "ViaSecondCRTCModeValid\n"));

    if (mode->CrtcHTotal > 4096)
        return MODE_BAD_HVALUE;

    if (mode->CrtcHDisplay > 2048)
        return MODE_BAD_HVALUE;

    if (mode->CrtcHBlankStart > 2048)
        return MODE_BAD_HVALUE;

    if (mode->CrtcHBlankEnd > 4096)
        return MODE_HBLANK_WIDE;

    if (mode->CrtcHSyncStart > 2047)
        return MODE_BAD_HVALUE;

    if ((mode->CrtcHSyncEnd - mode->CrtcHSyncStart) > 512)
        return MODE_HSYNC_WIDE;

    if (mode->CrtcVTotal > 2048)
        return MODE_BAD_VVALUE;

    if (mode->CrtcVDisplay > 2048)
        return MODE_BAD_VVALUE;

    if (mode->CrtcVBlankStart > 2048)
        return MODE_BAD_VVALUE;

    if (mode->CrtcVBlankEnd > 2048)
        return MODE_VBLANK_WIDE;

    if (mode->CrtcVSyncStart > 2047)
        return MODE_BAD_VVALUE;

    if ((mode->CrtcVSyncEnd - mode->CrtcVSyncStart) > 32)
        return MODE_VSYNC_WIDE;

    return MODE_OK;
}

/*
 * Not tested yet
 */
void
ViaShadowCRTCSetMode(ScrnInfoPtr pScrn, DisplayModePtr mode)
{
    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "ViaShadowCRTCSetMode\n"));

    vgaHWPtr hwp = VGAHWPTR(pScrn);
    CARD16 temp;

    temp = (mode->CrtcHTotal >> 3) - 5;
    hwp->writeCrtc(hwp, 0x6D, temp & 0xFF);
    ViaCrtcMask(hwp, 0x71, temp >> 5, 0x08);

    temp = (mode->CrtcHBlankEnd >> 3) - 1;
    hwp->writeCrtc(hwp, 0x6E, temp & 0xFF);

    temp = mode->CrtcVTotal - 2;
    hwp->writeCrtc(hwp, 0x6F, temp & 0xFF);
    ViaCrtcMask(hwp, 0x71, temp >> 8, 0x07);

    temp = mode->CrtcVDisplay - 1;
    hwp->writeCrtc(hwp, 0x70, temp & 0xFF);
    ViaCrtcMask(hwp, 0x71, temp >> 4, 0x70);

    temp = mode->CrtcVBlankStart - 1;
    hwp->writeCrtc(hwp, 0x72, temp & 0xFF);
    ViaCrtcMask(hwp, 0x74, temp >> 4, 0x70);

    temp = mode->CrtcVTotal - 1;
    hwp->writeCrtc(hwp, 0x73, temp & 0xFF);
    ViaCrtcMask(hwp, 0x74, temp >> 8, 0x07);

    ViaCrtcMask(hwp, 0x76, mode->CrtcVSyncEnd, 0x0F);

    temp = mode->CrtcVSyncStart;
    hwp->writeCrtc(hwp, 0x75, temp & 0xFF);
    ViaCrtcMask(hwp, 0x76, temp >> 4, 0x70);
}

static void
iga1_crtc_dpms(xf86CrtcPtr crtc, int mode)
{
    ScrnInfoPtr pScrn = crtc->scrn;
    VIAPtr pVia = VIAPTR(pScrn);
    VIABIOSInfoPtr pBIOSInfo = pVia->pBIOSInfo;

    switch (mode) {
    case DPMSModeOn:
        if (pBIOSInfo->SimultaneousEnabled)
            ViaDisplayEnableSimultaneous(pScrn);
        break;

    case DPMSModeStandby:
    case DPMSModeSuspend:
    case DPMSModeOff:
        if (pBIOSInfo->SimultaneousEnabled)
            ViaDisplayDisableSimultaneous(pScrn);
        break;

	default:
        xf86DrvMsg(pScrn->scrnIndex, X_ERROR, "Invalid DPMS mode %d\n",
                    mode);
        break;
    }
    //vgaHWSaveScreen(pScrn->pScreen, mode);

}

static void
iga1_crtc_save(xf86CrtcPtr crtc)
{
    ScrnInfoPtr pScrn = crtc->scrn;
    vgaHWPtr hwp = VGAHWPTR(pScrn);
    VIAPtr pVia = VIAPTR(pScrn);

    VIASave(pScrn);
    vgaHWUnlock(hwp);
}

static void
iga1_crtc_restore(xf86CrtcPtr crtc)
{
    ScrnInfoPtr pScrn = crtc->scrn;
    vgaHWPtr hwp = VGAHWPTR(pScrn);
    VIAPtr pVia = VIAPTR(pScrn);

    VIARestore(pScrn);

    /* A soft reset helps to avoid a 3D hang on VT switch. */
    switch (pVia->Chipset) {
    case VIA_K8M890:
    case VIA_P4M900:
    case VIA_VX800:
    case VIA_VX855:
    case VIA_VX900:
        break;

    default:
        hwp->writeSeq(hwp, 0x1A, pVia->SavedReg.SR1A | 0x40);
        break;
    }
    vgaHWLock(hwp);
}

static Bool
iga1_crtc_lock(xf86CrtcPtr crtc)
{
    return FALSE;
}

static void
iga1_crtc_unlock(xf86CrtcPtr crtc)
{
}

static Bool
iga1_crtc_mode_fixup(xf86CrtcPtr crtc, DisplayModePtr mode,
                        DisplayModePtr adjusted_mode)
{
    ScrnInfoPtr pScrn = crtc->scrn;
    VIAPtr pVia = VIAPTR(pScrn);
    CARD32 temp;
    ModeStatus modestatus;

    if ((mode->Clock < pScrn->clockRanges->minClock) ||
        (mode->Clock > pScrn->clockRanges->maxClock)) {
        xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                   "Clock for mode \"%s\" outside of allowed range (%u (%u - %u))\n",
                   mode->name, mode->Clock, pScrn->clockRanges->minClock,
                   pScrn->clockRanges->maxClock);
        return FALSE;
    }

    modestatus = ViaFirstCRTCModeValid(pScrn, mode);
    if (modestatus != MODE_OK) {
        xf86DrvMsg(pScrn->scrnIndex, X_INFO, "Not using mode \"%s\" : %s.\n",
                   mode->name, xf86ModeStatusToString(modestatus));
        return FALSE;
    }

    temp = mode->CrtcHDisplay * mode->CrtcVDisplay * mode->VRefresh *
            (pScrn->bitsPerPixel >> 3);
    if (pVia->pBIOSInfo->Bandwidth < temp) {
        xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                    "Required bandwidth is not available. (%u > %u)\n",
                    (unsigned)temp, (unsigned)pVia->pBIOSInfo->Bandwidth);
        return FALSE;
    }
    return TRUE;
}

static void
iga1_crtc_prepare (xf86CrtcPtr crtc)
{
}

static void
iga1_crtc_set_origin(xf86CrtcPtr crtc, int x, int y)
{
    ScrnInfoPtr pScrn = crtc->scrn;
    VIAPtr pVia = VIAPTR(pScrn);

    ViaFirstCRTCSetStartingAddress(crtc, x, y);
    VIAVidAdjustFrame(pScrn, x, y);
}

static void
iga1_crtc_mode_set(xf86CrtcPtr crtc, DisplayModePtr mode,
					DisplayModePtr adjusted_mode,
					int x, int y)
{
    ScrnInfoPtr pScrn = crtc->scrn;
    VIAPtr pVia = VIAPTR(pScrn);

    if (!vgaHWInit(pScrn, adjusted_mode))
        return;

    ViaCRTCInit(pScrn);
    ViaModeFirstCRTC(pScrn, adjusted_mode);

    if (pVia->pBIOSInfo->SimultaneousEnabled)
        ViaDisplayEnableSimultaneous(pScrn);
    else
        ViaDisplayDisableSimultaneous(pScrn);

    iga1_crtc_set_origin(crtc, crtc->x, crtc->y);
}

static void
iga1_crtc_gamma_set(xf86CrtcPtr crtc, CARD16 *red, CARD16 *green, CARD16 *blue,
					int size)
{
    ScrnInfoPtr pScrn = crtc->scrn;
    vgaHWPtr hwp = VGAHWPTR(pScrn);
    VIAPtr pVia = VIAPTR(pScrn);
    int SR1A, SR1B, CR67, CR6A;
    LOCO colors[size];
    int i;

    for (i = 0; i < size; i++) {
        colors[i].red = red[i] >> 8;
        colors[i].green = green[i] >> 8;
        colors[i].blue = blue[i] >> 8;
    }

    if (pScrn->bitsPerPixel != 8) {
        switch (pVia->Chipset) {
        case VIA_CLE266:
        case VIA_KM400:
            ViaSeqMask(hwp, 0x16, 0x80, 0x80);
            break;
        default:
            ViaCrtcMask(hwp, 0x33, 0x80, 0x80);
            break;
        }

        ViaSeqMask(hwp, 0x1A, 0x00, 0x01);
        VIALoadRgbLut(pScrn, 0, size, colors);

    } else {

        SR1A = hwp->readSeq(hwp, 0x1A);
        SR1B = hwp->readSeq(hwp, 0x1B);
        CR67 = hwp->readCrtc(hwp, 0x67);
        CR6A = hwp->readCrtc(hwp, 0x6A);

        for (i = 0; i < size; i++) {
            hwp->writeDacWriteAddr(hwp, i);
            hwp->writeDacData(hwp, colors[i].red);
            hwp->writeDacData(hwp, colors[i].green);
            hwp->writeDacData(hwp, colors[i].blue);
        }
    }
}

static void *
iga1_crtc_shadow_allocate (xf86CrtcPtr crtc, int width, int height)
{
    return NULL;
}

static PixmapPtr
iga1_crtc_shadow_create(xf86CrtcPtr crtc, void *data, int width, int height)
{
    return NULL;
}

static void
iga1_crtc_shadow_destroy(xf86CrtcPtr crtc, PixmapPtr rotate_pixmap, void *data)
{
}

/*
    Set the cursor foreground and background colors.  In 8bpp, fg and
    bg are indices into the current colormap unless the
    HARDWARE_CURSOR_TRUECOLOR_AT_8BPP flag is set.  In that case
    and in all other bpps the fg and bg are in 8-8-8 RGB format.
*/
static void
iga1_crtc_set_cursor_colors (xf86CrtcPtr crtc, int bg, int fg)
{
    ScrnInfoPtr pScrn = crtc->scrn;
    xf86CrtcConfigPtr xf86_config = XF86_CRTC_CONFIG_PTR(pScrn);
    VIAPtr pVia = VIAPTR(pScrn);
    CARD32 temp;

    if (xf86_config->cursor_fg)
        return;

    /* Don't recolour the image if we don't have to. */
    if (fg == xf86_config->cursor_fg && bg == xf86_config->cursor_bg)
        return;

    switch(pVia->Chipset) {
    case VIA_CX700:
    case VIA_P4M890:
    case VIA_P4M900:
    case VIA_VX800:
    case VIA_VX855:
    case VIA_VX900:
        temp = VIAGETREG(PRIM_HI_CTRL);
        VIASETREG(PRIM_HI_CTRL, temp & 0xFFFFFFFE);
        break;

    default:
        temp = VIAGETREG(HI_CONTROL);
        VIASETREG(HI_CONTROL, temp & 0xFFFFFFFE);
        break;
    }

    xf86_config->cursor_fg = fg;
    xf86_config->cursor_bg = bg;
}

static void
iga1_crtc_set_cursor_position (xf86CrtcPtr crtc, int x, int y)
{
    ScrnInfoPtr pScrn = crtc->scrn;
    VIAPtr pVia = VIAPTR(pScrn);
    unsigned xoff, yoff;

    if (x < 0) {
        xoff = ((-x) & 0xFE);
        x = 0;
    } else {
        xoff = 0;
    }

    if (y < 0) {
        yoff = ((-y) & 0xFE);
        y = 0;
    } else {
        yoff = 0;
    }

    switch(pVia->Chipset) {
    case VIA_CX700:
    case VIA_P4M890:
    case VIA_P4M900:
    case VIA_VX800:
    case VIA_VX855:
    case VIA_VX900:
        VIASETREG(PRIM_HI_POSSTART,    ((x    << 16) | (y    & 0x07ff)));
        VIASETREG(PRIM_HI_CENTEROFFSET, ((xoff << 16) | (yoff & 0x07ff)));
        break;

    default:
        VIASETREG(HI_POSSTART,    ((x    << 16) | (y    & 0x07ff)));
        VIASETREG(HI_CENTEROFFSET, ((xoff << 16) | (yoff & 0x07ff)));
        break;
    }
}

static void
iga1_crtc_show_cursor (xf86CrtcPtr crtc)
{
    drmmode_crtc_private_ptr iga = crtc->driver_private;
    ScrnInfoPtr pScrn = crtc->scrn;
    VIAPtr pVia = VIAPTR(pScrn);

    switch(pVia->Chipset) {
    case VIA_CX700:
    case VIA_P4M890:
    case VIA_P4M900:
    case VIA_VX800:
    case VIA_VX855:
    case VIA_VX900:
        VIASETREG(PRIM_HI_FBOFFSET, iga->cursor_bo->offset);
        VIASETREG(PRIM_HI_CTRL, 0x36000005);
        break;

    default:
        /* Mono Cursor Display Path [bit31]: Primary */
        VIASETREG(HI_FBOFFSET, iga->cursor_bo->offset);
        VIASETREG(HI_CONTROL, 0x76000005);
        break;
    }
}

static void
iga1_crtc_hide_cursor (xf86CrtcPtr crtc)
{
    ScrnInfoPtr pScrn = crtc->scrn;
    VIAPtr pVia = VIAPTR(pScrn);
    CARD32 temp;

    switch(pVia->Chipset) {
    case VIA_CX700:
    case VIA_P4M890:
    case VIA_P4M900:
    case VIA_VX800:
    case VIA_VX855:
    case VIA_VX900:
        temp = VIAGETREG(PRIM_HI_CTRL);
        VIASETREG(PRIM_HI_CTRL, temp & 0xFFFFFFFA);
        break;

    default:
        temp = VIAGETREG(HI_CONTROL);
        /* Hardware cursor disable [bit0] */
        VIASETREG(HI_CONTROL, temp & 0xFFFFFFFA);
        break;
    }
}

static void
iga_crtc_load_cursor_argb(xf86CrtcPtr crtc, CARD32 *image)
{
    drmmode_crtc_private_ptr iga = crtc->driver_private;
    ScrnInfoPtr pScrn = crtc->scrn;
    void *dst;

    dst = drm_bo_map(pScrn, iga->cursor_bo);
    memset(dst, 0x00, iga->cursor_bo->size);
    memcpy(dst, image, iga->cursor_bo->size);
    drm_bo_unmap(pScrn, iga->cursor_bo);
}

static void
iga_crtc_commit(xf86CrtcPtr crtc)
{
    ScrnInfoPtr pScrn = crtc->scrn;
    VIAPtr pVia = VIAPTR(pScrn);

    if (crtc->scrn->pScreen != NULL && pVia->drmmode.hwcursor)
        xf86_reload_cursors(crtc->scrn->pScreen);
}

static void
iga_crtc_destroy(xf86CrtcPtr crtc)
{
    if (crtc->driver_private)
        free(crtc->driver_private);
}

static const xf86CrtcFuncsRec iga1_crtc_funcs = {
    .dpms                   = iga1_crtc_dpms,
    .save                   = iga1_crtc_save,
    .restore                = iga1_crtc_restore,
    .lock                   = iga1_crtc_lock,
    .unlock                 = iga1_crtc_unlock,
    .mode_fixup             = iga1_crtc_mode_fixup,
    .prepare                = iga1_crtc_prepare,
    .mode_set               = iga1_crtc_mode_set,
    .commit                 = iga_crtc_commit,
    .gamma_set              = iga1_crtc_gamma_set,
    .shadow_create          = iga1_crtc_shadow_create,
    .shadow_allocate        = iga1_crtc_shadow_allocate,
    .shadow_destroy         = iga1_crtc_shadow_destroy,
    .set_cursor_colors      = iga1_crtc_set_cursor_colors,
    .set_cursor_position    = iga1_crtc_set_cursor_position,
    .show_cursor            = iga1_crtc_show_cursor,
    .hide_cursor            = iga1_crtc_hide_cursor,
    .load_cursor_argb       = iga_crtc_load_cursor_argb,
#if GET_ABI_MAJOR(ABI_VIDEODRV_VERSION) > 2
    .set_origin             = iga1_crtc_set_origin,
#endif
    .destroy                = iga_crtc_destroy,
};

static void
iga2_crtc_dpms(xf86CrtcPtr crtc, int mode)
{
    ScrnInfoPtr pScrn = crtc->scrn;
    VIAPtr pVia = VIAPTR(pScrn);
    VIABIOSInfoPtr pBIOSInfo = pVia->pBIOSInfo;

    switch (mode) {
    case DPMSModeOn:
        if (pBIOSInfo->SimultaneousEnabled)
            ViaDisplayEnableSimultaneous(pScrn);
        break;

    case DPMSModeStandby:
    case DPMSModeSuspend:
    case DPMSModeOff:
        if (pBIOSInfo->SimultaneousEnabled)
            ViaDisplayDisableSimultaneous(pScrn);
        break;

    default:
        xf86DrvMsg(pScrn->scrnIndex, X_ERROR, "Invalid DPMS mode %d\n",
                    mode);
        break;
    }
    //vgaHWSaveScreen(pScrn->pScreen, mode);
}

static void
iga2_crtc_save(xf86CrtcPtr crtc)
{
    ScrnInfoPtr pScrn = crtc->scrn;
    vgaHWPtr hwp = VGAHWPTR(pScrn);
    VIAPtr pVia = VIAPTR(pScrn);

    VIASave(pScrn);
    vgaHWUnlock(hwp);
}

static void
iga2_crtc_restore(xf86CrtcPtr crtc)
{
    ScrnInfoPtr pScrn = crtc->scrn;
    vgaHWPtr hwp = VGAHWPTR(pScrn);
    VIAPtr pVia = VIAPTR(pScrn);

    VIARestore(pScrn);
    vgaHWLock(hwp);
}

static Bool
iga2_crtc_lock(xf86CrtcPtr crtc)
{
    return FALSE;
}

static void
iga2_crtc_unlock(xf86CrtcPtr crtc)
{
}

static Bool
iga2_crtc_mode_fixup(xf86CrtcPtr crtc, DisplayModePtr mode,
                        DisplayModePtr adjusted_mode)
{
    ScrnInfoPtr pScrn = crtc->scrn;
    VIAPtr pVia = VIAPTR(pScrn);
    CARD32 temp;
    ModeStatus modestatus;

    if ((mode->Clock < pScrn->clockRanges->minClock) ||
        (mode->Clock > pScrn->clockRanges->maxClock)) {
        xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                   "Clock for mode \"%s\" outside of allowed range (%u (%u - %u))\n",
                   mode->name, mode->Clock, pScrn->clockRanges->minClock,
                   pScrn->clockRanges->maxClock);
        return FALSE;
    }

    modestatus = ViaFirstCRTCModeValid(pScrn, mode);
    if (modestatus != MODE_OK) {
        xf86DrvMsg(pScrn->scrnIndex, X_INFO, "Not using mode \"%s\" : %s.\n",
                   mode->name, xf86ModeStatusToString(modestatus));
        return FALSE;
    }

    temp = mode->CrtcHDisplay * mode->CrtcVDisplay * mode->VRefresh *
            (pScrn->bitsPerPixel >> 3);
    if (pVia->pBIOSInfo->Bandwidth < temp) {
        xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                    "Required bandwidth is not available. (%u > %u)\n",
			        (unsigned)temp, (unsigned)pVia->pBIOSInfo->Bandwidth);
        return FALSE;
    }
    return TRUE;
}

static void
iga2_crtc_prepare (xf86CrtcPtr crtc)
{
}

static void
iga2_crtc_set_origin(xf86CrtcPtr crtc, int x, int y)
{
    ScrnInfoPtr pScrn = crtc->scrn;
    VIAPtr pVia = VIAPTR(pScrn);

    ViaSecondCRTCSetStartingAddress(crtc, x, y);
    VIAVidAdjustFrame(pScrn, x, y);
}

static void
iga2_crtc_mode_set(xf86CrtcPtr crtc, DisplayModePtr mode,
                    DisplayModePtr adjusted_mode, int x, int y)
{
    ScrnInfoPtr pScrn = crtc->scrn;
    VIAPtr pVia = VIAPTR(pScrn);

    if (!vgaHWInit(pScrn, adjusted_mode))
        return;

    ViaCRTCInit(pScrn);
    ViaModeSecondCRTC(pScrn, adjusted_mode);
    ViaSecondDisplayChannelEnable(pScrn);

    if (pVia->pBIOSInfo->SimultaneousEnabled)
        ViaDisplayEnableSimultaneous(pScrn);
    else
        ViaDisplayDisableSimultaneous(pScrn);

    iga2_crtc_set_origin(crtc, crtc->x, crtc->y);
}

static void
iga2_crtc_gamma_set(xf86CrtcPtr crtc, CARD16 *red, CARD16 *green, CARD16 *blue,
					int size)
{
    ScrnInfoPtr pScrn = crtc->scrn;
    vgaHWPtr hwp = VGAHWPTR(pScrn);
    VIAPtr pVia = VIAPTR(pScrn);
    int SR1A, SR1B, CR67, CR6A;
    LOCO colors[size];
    int i;

    for (i = 0; i < size; i++) {
        colors[i].red = red[i] >> 8;
        colors[i].green = green[i] >> 8;
        colors[i].blue = blue[i] >> 8;
    }

    if (pScrn->bitsPerPixel != 8) {
        if (!(pVia->Chipset == VIA_CLE266 &&
            CLE266_REV_IS_AX(pVia->ChipRev))) {
            ViaSeqMask(hwp, 0x1A, 0x01, 0x01);
            ViaCrtcMask(hwp, 0x6A, 0x02, 0x02);

            switch (pVia->Chipset) {
            case VIA_CLE266:
            case VIA_KM400:
            case VIA_K8M800:
            case VIA_PM800:
                break;

            default:
                ViaCrtcMask(hwp, 0x6A, 0x20, 0x20);
                break;
            }
            VIALoadRgbLut(pScrn, 0, size, colors);
        }
    } else {
        SR1A = hwp->readSeq(hwp, 0x1A);
        SR1B = hwp->readSeq(hwp, 0x1B);
        CR67 = hwp->readCrtc(hwp, 0x67);
        CR6A = hwp->readCrtc(hwp, 0x6A);

        ViaSeqMask(hwp, 0x1A, 0x01, 0x01);
        ViaSeqMask(hwp, 0x1B, 0x80, 0x80);
        ViaCrtcMask(hwp, 0x67, 0x00, 0xC0);
        ViaCrtcMask(hwp, 0x6A, 0xC0, 0xC0);

        for (i = 0; i < size; i++) {
            hwp->writeDacWriteAddr(hwp, i);
            hwp->writeDacData(hwp, colors[i].red);
            hwp->writeDacData(hwp, colors[i].green);
            hwp->writeDacData(hwp, colors[i].blue);
        }

        hwp->writeSeq(hwp, 0x1A, SR1A);
        hwp->writeSeq(hwp, 0x1B, SR1B);
        hwp->writeCrtc(hwp, 0x67, CR67);
        hwp->writeCrtc(hwp, 0x6A, CR6A);

        /* Screen 0 palette was changed by mode setting of Screen 1,
         * so load it again. */
        for (i = 0; i < size; i++) {
            hwp->writeDacWriteAddr(hwp, i);
            hwp->writeDacData(hwp, colors[i].red);
            hwp->writeDacData(hwp, colors[i].green);
            hwp->writeDacData(hwp, colors[i].blue);
        }
    }
}

static void *
iga2_crtc_shadow_allocate (xf86CrtcPtr crtc, int width, int height)
{
    return NULL;
}

static PixmapPtr
iga2_crtc_shadow_create(xf86CrtcPtr crtc, void *data, int width, int height)
{
    return NULL;
}

static void
iga2_crtc_shadow_destroy(xf86CrtcPtr crtc, PixmapPtr rotate_pixmap, void *data)
{
}

/*
    Set the cursor foreground and background colors.  In 8bpp, fg and
    bg are indices into the current colormap unless the
    HARDWARE_CURSOR_TRUECOLOR_AT_8BPP flag is set.  In that case
    and in all other bpps the fg and bg are in 8-8-8 RGB format.
*/
static void
iga2_crtc_set_cursor_colors(xf86CrtcPtr crtc, int bg, int fg)
{
    drmmode_crtc_private_ptr iga = crtc->driver_private;
    ScrnInfoPtr pScrn = crtc->scrn;
    xf86CrtcConfigPtr xf86_config = XF86_CRTC_CONFIG_PTR(pScrn);
    int height = 64, width = 64, i;
    VIAPtr pVia = VIAPTR(pScrn);
    CARD32 pixel, temp, *dst;

    if (xf86_config->cursor_fg)
        return;

    fg |= 0xff000000;
    bg |= 0xff000000;

    /* Don't recolour the image if we don't have to. */
    if (fg == xf86_config->cursor_fg && bg == xf86_config->cursor_bg)
        return;

    switch(pVia->Chipset) {
    case VIA_CX700:
    case VIA_P4M890:
    case VIA_P4M900:
    case VIA_VX800:
    case VIA_VX855:
    case VIA_VX900:
        temp = VIAGETREG(HI_CONTROL);
        VIASETREG(HI_CONTROL, temp & 0xFFFFFFFE);
        break;

    default:
        temp = VIAGETREG(HI_CONTROL);
        VIASETREG(HI_CONTROL, temp & 0xFFFFFFFE);
        height = width = 32;
        break;
    }

    dst = drm_bo_map(pScrn, iga->cursor_bo);
    for (i = 0; i < width * height; i++, dst++)
        if ((pixel = *dst))
            *dst = (pixel == xf86_config->cursor_fg) ? fg : bg;
    drm_bo_unmap(pScrn, iga->cursor_bo);

    xf86_config->cursor_fg = fg;
    xf86_config->cursor_bg = bg;
}

static void
iga2_crtc_set_cursor_position(xf86CrtcPtr crtc, int x, int y)
{
    ScrnInfoPtr pScrn = crtc->scrn;
    VIAPtr pVia = VIAPTR(pScrn);
    unsigned xoff, yoff;

    if (x < 0) {
        xoff = ((-x) & 0xFE);
        x = 0;
    } else {
        xoff = 0;
    }

    if (y < 0) {
        yoff = ((-y) & 0xFE);
        y = 0;
    } else {
        yoff = 0;
    }

    switch(pVia->Chipset) {
    case VIA_CX700:
    case VIA_P4M890:
    case VIA_P4M900:
    case VIA_VX800:
    case VIA_VX855:
    case VIA_VX900:
        VIASETREG(HI_POSSTART,    ((x    << 16) | (y    & 0x07ff)));
        VIASETREG(HI_CENTEROFFSET, ((xoff << 16) | (yoff & 0x07ff)));
        break;

    default:
        VIASETREG(HI_POSSTART,    ((x    << 16) | (y    & 0x07ff)));
        VIASETREG(HI_CENTEROFFSET, ((xoff << 16) | (yoff & 0x07ff)));
        break;
    }
}

static void
iga2_crtc_show_cursor(xf86CrtcPtr crtc)
{
    drmmode_crtc_private_ptr iga = crtc->driver_private;
    ScrnInfoPtr pScrn = crtc->scrn;
    VIAPtr pVia = VIAPTR(pScrn);

    switch(pVia->Chipset) {
    case VIA_CX700:
    case VIA_P4M890:
    case VIA_P4M900:
    case VIA_VX800:
    case VIA_VX855:
    case VIA_VX900:
        VIASETREG(HI_FBOFFSET, iga->cursor_bo->offset);
        VIASETREG(HI_CONTROL, 0xB6000005);
        break;

    default:
        /* Mono Cursor Display Path [bit31]: Secondary */
        /* FIXME For CLE266 and KM400 try to enable 32x32 cursor size [bit1] */
        VIASETREG(HI_FBOFFSET, iga->cursor_bo->offset);
        VIASETREG(HI_CONTROL, 0xF6000005);
        break;
    }
}

static void
iga2_crtc_hide_cursor(xf86CrtcPtr crtc)
{
    ScrnInfoPtr pScrn = crtc->scrn;
    VIAPtr pVia = VIAPTR(pScrn);
    CARD32 temp;

    switch(pVia->Chipset) {
    case VIA_CX700:
    case VIA_P4M890:
    case VIA_P4M900:
    case VIA_VX800:
    case VIA_VX855:
    case VIA_VX900:
	    temp = VIAGETREG(HI_CONTROL);
        VIASETREG(HI_CONTROL, temp & 0xFFFFFFFA);
        break;

    default:
        temp = VIAGETREG(HI_CONTROL);
        /* Hardware cursor disable [bit0] */
        VIASETREG(HI_CONTROL, temp & 0xFFFFFFFA);
        break;
	}
}

static const xf86CrtcFuncsRec iga2_crtc_funcs = {
    .dpms                   = iga2_crtc_dpms,
    .save                   = iga2_crtc_save,
    .restore                = iga2_crtc_restore,
    .lock                   = iga2_crtc_lock,
    .unlock                 = iga2_crtc_unlock,
    .mode_fixup             = iga2_crtc_mode_fixup,
    .prepare                = iga2_crtc_prepare,
    .mode_set               = iga2_crtc_mode_set,
    .commit                 = iga_crtc_commit,
    .gamma_set              = iga2_crtc_gamma_set,
    .shadow_create          = iga2_crtc_shadow_create,
    .shadow_allocate        = iga2_crtc_shadow_allocate,
    .shadow_destroy         = iga2_crtc_shadow_destroy,
    .set_cursor_colors      = iga2_crtc_set_cursor_colors,
    .set_cursor_position    = iga2_crtc_set_cursor_position,
    .show_cursor            = iga2_crtc_show_cursor,
    .hide_cursor            = iga2_crtc_hide_cursor,
    .load_cursor_argb       = iga_crtc_load_cursor_argb,
#if GET_ABI_MAJOR(ABI_VIDEODRV_VERSION) > 2
    .set_origin             = iga2_crtc_set_origin,
#endif
    .destroy                = iga_crtc_destroy,
};

Bool
UMSCrtcInit(ScrnInfoPtr pScrn)
{
    drmmode_crtc_private_ptr iga1_rec = NULL, iga2_rec = NULL;
    vgaHWPtr hwp = VGAHWPTR(pScrn);
    VIAPtr pVia = VIAPTR(pScrn);
#if XORG_VERSION_CURRENT >= XORG_VERSION_NUMERIC(1,8,0,0,0)
    ClockRangePtr clockRanges;
#else
    ClockRangesPtr clockRanges;
#endif
    int max_pitch, max_height;
    VIABIOSInfoPtr pBIOSInfo;
    xf86CrtcPtr iga1, iga2;

    /* Read memory bandwidth from registers. */
    pVia->MemClk = hwp->readCrtc(hwp, 0x3D) >> 4;
    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                     "Detected MemClk %d\n", pVia->MemClk));
    if (pVia->MemClk >= VIA_MEM_END) {
        xf86DrvMsg(pScrn->scrnIndex, X_WARNING,
                   "Unknown Memory clock: %d\n", pVia->MemClk);
        pVia->MemClk = VIA_MEM_END - 1;
    }
    pBIOSInfo = pVia->pBIOSInfo;
    pBIOSInfo->Bandwidth = ViaGetMemoryBandwidth(pScrn);

    if (pBIOSInfo->TVType == TVTYPE_NONE) {
        /* Use jumper to determine TV type. */
        if (hwp->readCrtc(hwp, 0x3B) & 0x02) {
            pBIOSInfo->TVType = TVTYPE_PAL;
            DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                             "Detected TV standard: PAL.\n"));
        } else {
            pBIOSInfo->TVType = TVTYPE_NTSC;
            DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                             "Detected TV standard: NTSC.\n"));
        }
    }

    if (pVia->drmmode.hwcursor) {
        if (!xf86LoadSubModule(pScrn, "ramdac"))
            return FALSE;
    }

    if (!xf86LoadSubModule(pScrn, "i2c"))
        return FALSE;
    else
        ViaI2CInit(pScrn);

    if (!xf86LoadSubModule(pScrn, "ddc"))
        return FALSE;

    /*
     * Set up ClockRanges, which describe what clock ranges are
     * available, and what sort of modes they can be used for.
     */

#if XORG_VERSION_CURRENT >= XORG_VERSION_NUMERIC(1,8,0,0,0)
    clockRanges = xnfalloc(sizeof(ClockRange));
#else
    clockRanges = xnfalloc(sizeof(ClockRanges));
#endif
    clockRanges->next = NULL;
    clockRanges->minClock = 20000;
    clockRanges->maxClock = 230000;

    clockRanges->clockIndex = -1;
    clockRanges->interlaceAllowed = TRUE;
    clockRanges->doubleScanAllowed = FALSE;
    pScrn->clockRanges = clockRanges;

    /*
     * Now handle the outputs
     */
    iga1_rec = (drmmode_crtc_private_ptr) xnfcalloc(sizeof(drmmode_crtc_private_rec), 1);
    if (!iga1_rec) {
        xf86DrvMsg(pScrn->scrnIndex, X_ERROR, "IGA1 Rec allocation failed.\n");
        return FALSE;
    }

    iga1 = xf86CrtcCreate(pScrn, &iga1_crtc_funcs);
    if (!iga1) {
        xf86DrvMsg(pScrn->scrnIndex, X_ERROR, "xf86CrtcCreate failed.\n");
        free(iga1_rec);
        return FALSE;
    }
    iga1_rec->drmmode = &pVia->drmmode;
    iga1_rec->index = 0;
    iga1->driver_private = iga1_rec;

    iga2_rec = (drmmode_crtc_private_ptr) xnfcalloc(sizeof(drmmode_crtc_private_rec), 1);
    if (!iga2_rec) {
        xf86DrvMsg(pScrn->scrnIndex, X_ERROR, "IGA1 Rec allocation failed.\n");
        xf86CrtcDestroy(iga1);
        return FALSE;
    }

    iga2 = xf86CrtcCreate(pScrn, &iga2_crtc_funcs);
    if (!iga2) {
        xf86DrvMsg(pScrn->scrnIndex, X_ERROR, "xf86CrtcCreate failed.\n");
        xf86CrtcDestroy(iga1);
        free(iga2_rec);
        return FALSE;
    }
    iga2_rec->drmmode = &pVia->drmmode;
    iga2_rec->index = 1;
    iga2->driver_private = iga2_rec;

    /*
     * CLE266A:
     *   Max Line Pitch: 4080, (FB corruption when higher, driver problem?)
     *   Max Height: 4096 (and beyond)
     *
     * CLE266A: primary AdjustFrame can use only 24 bits, so we are limited
     * to 12x11 bits; 4080x2048 (~2:1), 3344x2508 (4:3), or 2896x2896 (1:1).
     * TODO Test CLE266Cx, KM400, KM400A, K8M800, CN400 please.
     *
     * We should be able to limit the memory available for a mode to 32 MB,
     * but miScanLineWidth fails to catch this properly (apertureSize).
     */
    switch (pVia->Chipset) {
    case VIA_CLE266:
    case VIA_KM400:
    case VIA_K8M800:
    case VIA_PM800:
    case VIA_P4M800PRO:
        max_pitch = 3344;
        max_height = 2508;
        break;

    case VIA_CX700:
    case VIA_P4M890:
    case VIA_K8M890:
    case VIA_P4M900:
        max_pitch = 8192/(pScrn->bitsPerPixel >> 3)-1;
        max_height = max_pitch;
        break;

    default:
        max_pitch = 16384/(pScrn->bitsPerPixel >> 3)-1;
        max_height = max_pitch;
        break;
    }

    /* Init HI_X0 for cursor */
    switch (pVia->Chipset) {
    case VIA_CX700:
    /* case VIA_CN750: */
    case VIA_P4M890:
    case VIA_P4M900:
    case VIA_VX800:
    case VIA_VX855:
    case VIA_VX900:
        /* set 0 as transparent color key for IGA 2 */
        VIASETREG(HI_TRANSPARENT_COLOR, 0);
        VIASETREG(HI_INVTCOLOR, 0X00FFFFFF);
        VIASETREG(ALPHA_V3_PREFIFO_CONTROL, 0xE0000);
        VIASETREG(ALPHA_V3_FIFO_CONTROL, 0xE0F0000);

        /* set 0 as transparent color key for IGA 1 */
        VIASETREG(PRIM_HI_TRANSCOLOR, 0);
        VIASETREG(PRIM_HI_FIFO, 0x0D000D0F);
        VIASETREG(PRIM_HI_INVTCOLOR, 0x00FFFFFF);
        VIASETREG(V327_HI_INVTCOLOR, 0x00FFFFFF);
        break;

    default:
        VIASETREG(HI_TRANSPARENT_COLOR, 0);
        VIASETREG(HI_INVTCOLOR, 0X00FFFFFF);
        VIASETREG(ALPHA_V3_PREFIFO_CONTROL, 0xE0000);
        VIASETREG(ALPHA_V3_FIFO_CONTROL, 0xE0F0000);
        break;
    }

    xf86CrtcSetSizeRange(pScrn, 320, 200, max_pitch, max_height);

    ViaOutputsDetect(pScrn);

    return TRUE;
}

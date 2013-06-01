/*
 * Copyright 2004-2005 The Unichrome Project  [unichrome.sf.net]
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

/*
 * Wrappers around xf86 vgaHW functions.
 * And some generic IO calls lacking in the current vgaHW implementation.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "compiler.h"
#include "xf86.h"
#include "via_driver.h" /* for HAVE_DEBUG */

#if ABI_VIDEODRV_VERSION < 12
#define PIOOFFSET hwp->PIOOffset
#else
#define PIOOFFSET 0
#endif

CARD8
ViaVgahwIn(vgaHWPtr hwp, int address)
{
    if (hwp->MMIOBase)
        return MMIO_IN8(hwp->MMIOBase, hwp->MMIOOffset + address);
    else
        return inb(PIOOFFSET + address);
}

static void
ViaVgahwOut(vgaHWPtr hwp, int address, CARD8 value)
{
    if (hwp->MMIOBase)
        MMIO_OUT8(hwp->MMIOBase, hwp->MMIOOffset + address, value);
    else
        outb(PIOOFFSET + address, value);
}

/*
 * An indexed read.
 */
static CARD8
ViaVgahwRead(vgaHWPtr hwp, int indexaddress, CARD8 index, int valueaddress)
{
    ViaVgahwOut(hwp, indexaddress, index);
    return ViaVgahwIn(hwp, valueaddress);
}

/*
 * An indexed write.
 */
void
ViaVgahwWrite(vgaHWPtr hwp, int indexaddress, CARD8 index,
              int valueaddress, CARD8 value)
{
    ViaVgahwOut(hwp, indexaddress, index);
    ViaVgahwOut(hwp, valueaddress, value);
}


void
ViaVgahwMask(vgaHWPtr hwp, int indexaddress, CARD8 index,
             int valueaddress, CARD8 value, CARD8 mask)
{
    CARD8 tmp;

    tmp = ViaVgahwRead(hwp, indexaddress, index, valueaddress);
    tmp &= ~mask;
    tmp |= (value & mask);

    ViaVgahwWrite(hwp, indexaddress, index, valueaddress, tmp);
}

void
ViaCrtcMask(vgaHWPtr hwp, CARD8 index, CARD8 value, CARD8 mask)
{
    CARD8 tmp;

    tmp = hwp->readCrtc(hwp, index);
    tmp &= ~mask;
    tmp |= (value & mask);

    hwp->writeCrtc(hwp, index, tmp);
}

void
ViaSeqMask(vgaHWPtr hwp, CARD8 index, CARD8 value, CARD8 mask)
{
    CARD8 tmp;

    tmp = hwp->readSeq(hwp, index);
    tmp &= ~mask;
    tmp |= (value & mask);

    hwp->writeSeq(hwp, index, tmp);
}

void
ViaGrMask(vgaHWPtr hwp, CARD8 index, CARD8 value, CARD8 mask)
{
    CARD8 tmp;

    tmp = hwp->readGr(hwp, index);
    tmp &= ~mask;
    tmp |= (value & mask);

    hwp->writeGr(hwp, index, tmp);
}

void
VIASave(ScrnInfoPtr pScrn)
{
    vgaHWPtr hwp = VGAHWPTR(pScrn);
    VIAPtr pVia = VIAPTR(pScrn);
    VIABIOSInfoPtr pBIOSInfo = pVia->pBIOSInfo;
    VIARegPtr Regs = &pVia->SavedReg;
    int i;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "VIASave\n"));

    if (pVia->IsSecondary) {
        DevUnion *pPriv;
        VIAEntPtr pVIAEnt;
        VIAPtr pVia1;
        vgaHWPtr hwp1;

        DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "Secondary\n"));

        pPriv = xf86GetEntityPrivate(pScrn->entityList[0], gVIAEntityIndex);
        pVIAEnt = pPriv->ptr;
        hwp1 = VGAHWPTR(pVIAEnt->pPrimaryScrn);
        pVia1 = VIAPTR(pVIAEnt->pPrimaryScrn);
        hwp->SavedReg = hwp1->SavedReg;
        pVia->SavedReg = pVia1->SavedReg;
    } else {
        DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "Primary\n"));

        vgaHWProtect(pScrn, TRUE);

        if (xf86IsPrimaryPci(pVia->PciInfo)) {
            vgaHWSave(pScrn, &hwp->SavedReg, VGA_SR_ALL);
            DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                  "Primary Adapter! saving VGA_SR_ALL !!\n"));
        } else {
            vgaHWSave(pScrn, &hwp->SavedReg, VGA_SR_MODE);
            DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                  "Non-Primary Adapter! saving VGA_SR_MODE only !!\n"));
        }
        /* Unlock and save extended registers. */
        hwp->writeSeq(hwp, 0x10, 0x01);

        Regs->SR14 = hwp->readSeq(hwp, 0x14);
        Regs->SR15 = hwp->readSeq(hwp, 0x15);
        Regs->SR16 = hwp->readSeq(hwp, 0x16);
        Regs->SR17 = hwp->readSeq(hwp, 0x17);
        Regs->SR18 = hwp->readSeq(hwp, 0x18);
        Regs->SR19 = hwp->readSeq(hwp, 0x19);
        /* PCI Bus Control */
        Regs->SR1A = hwp->readSeq(hwp, 0x1A);

        Regs->SR1B = hwp->readSeq(hwp, 0x1B);
        Regs->SR1C = hwp->readSeq(hwp, 0x1C);
        Regs->SR1D = hwp->readSeq(hwp, 0x1D);
        Regs->SR1E = hwp->readSeq(hwp, 0x1E);
        Regs->SR1F = hwp->readSeq(hwp, 0x1F);

        Regs->SR22 = hwp->readSeq(hwp, 0x22);
        Regs->SR23 = hwp->readSeq(hwp, 0x23);
        Regs->SR24 = hwp->readSeq(hwp, 0x24);
        Regs->SR25 = hwp->readSeq(hwp, 0x25);
        Regs->SR26 = hwp->readSeq(hwp, 0x26);
        Regs->SR27 = hwp->readSeq(hwp, 0x27);
        Regs->SR28 = hwp->readSeq(hwp, 0x28);
        Regs->SR29 = hwp->readSeq(hwp, 0x29);
        Regs->SR2A = hwp->readSeq(hwp, 0x2A);
        Regs->SR2B = hwp->readSeq(hwp, 0x2B);

        Regs->SR2E = hwp->readSeq(hwp, 0x2E);

        /*=* Save VCK, LCDCK and ECK  *=*/
        /* Primary Display (VCK) (description for Chipset >= K8M800): */
        Regs->SR44 = hwp->readSeq(hwp, 0x44);
        Regs->SR45 = hwp->readSeq(hwp, 0x45);
        Regs->SR46 = hwp->readSeq(hwp, 0x46);

        /* ECK Clock Synthesizer (description for Chipset >= K8M800): */
        Regs->SR47 = hwp->readSeq(hwp, 0x47);
        Regs->SR48 = hwp->readSeq(hwp, 0x48);
        Regs->SR49 = hwp->readSeq(hwp, 0x49);

        switch (pVia->Chipset) {
            case VIA_CLE266:
            case VIA_KM400:
                break;
            default:
                /* Secondary Display (LCDCK): */
                Regs->SR4A = hwp->readSeq(hwp, 0x4A);
                Regs->SR4B = hwp->readSeq(hwp, 0x4B);
                Regs->SR4C = hwp->readSeq(hwp, 0x4C);
                break;
        }

        /* Save Preemptive Arbiter Control Register */
        Regs->SR4D = hwp->readSeq(hwp, 0x4D);
        DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "Crtc...\n"));

        Regs->CR13 = hwp->readCrtc(hwp, 0x13);

        Regs->CR32 = hwp->readCrtc(hwp, 0x32);
        Regs->CR33 = hwp->readCrtc(hwp, 0x33);

        Regs->CR35 = hwp->readCrtc(hwp, 0x35);
        Regs->CR36 = hwp->readCrtc(hwp, 0x36);

        /* Starting Address */
        /* Start Address High */
        Regs->CR0C = hwp->readCrtc(hwp, 0x0C);
        /* Start Address Low */
        Regs->CR0D = hwp->readCrtc(hwp, 0x0D);
        /* Starting Address Overflow Bits[28:24] */
        Regs->CR48 = hwp->readCrtc(hwp, 0x48);
        /* CR34 are fire bits. Must be written after CR0C CR0D CR48.  */
        /* Starting Address Overflow Bits[23:16] */
        Regs->CR34 = hwp->readCrtc(hwp, 0x34);

        Regs->CR49 = hwp->readCrtc(hwp, 0x49);

        DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "TVSave...\n"));
        if (pBIOSInfo->TVI2CDev)
            ViaTVSave(pScrn);

        /* Save LCD control registers (from CR 0x50 to 0x93). */
        for (i = 0; i < 68; i++)
            Regs->CRTCRegs[i] = hwp->readCrtc(hwp, i + 0x50);

        if (pVia->Chipset != VIA_CLE266 && pVia->Chipset != VIA_KM400) {
            /* LVDS Channel 2 Function Select 0 / DVI Function Select */
            Regs->CR97 = hwp->readCrtc(hwp, 0x97);
            /* LVDS Channel 1 Function Select 0 */
            Regs->CR99 = hwp->readCrtc(hwp, 0x99);
            /* Digital Video Port 1 Function Select 0 */
            Regs->CR9B = hwp->readCrtc(hwp, 0x9B);
            /* Power Now Control 4 */
            Regs->CR9F = hwp->readCrtc(hwp, 0x9F);

            /* Horizontal Scaling Initial Value */
            Regs->CRA0 = hwp->readCrtc(hwp, 0xA0);
            /* Vertical Scaling Initial Value */
            Regs->CRA1 = hwp->readCrtc(hwp, 0xA1);
            /* Scaling Enable Bit */
            Regs->CRA2 = hwp->readCrtc(hwp, 0xA2);
        }

        /* Save TMDS status */
        switch (pVia->Chipset) {
            case VIA_CX700:
            case VIA_VX800:
            case VIA_VX855:
            case VIA_VX900:
                Regs->CRD2 = hwp->readCrtc(hwp, 0xD2);
                break;
        }
        vgaHWProtect(pScrn, FALSE);
    }
}

void
VIARestore(ScrnInfoPtr pScrn)
{
    vgaHWPtr hwp = VGAHWPTR(pScrn);
    VIAPtr pVia = VIAPTR(pScrn);
    VIABIOSInfoPtr pBIOSInfo = pVia->pBIOSInfo;
    VIARegPtr Regs = &pVia->SavedReg;
    int i;
    CARD8 tmp;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "VIARestore\n"));

    /* Secondary? */

    vgaHWProtect(pScrn, TRUE);

    /* Unlock extended registers. */
    hwp->writeSeq(hwp, 0x10, 0x01);

    /*=* CR6A, CR6B, CR6C must be reset before restoring
         standard vga regs, or system will hang. *=*/
    /*=* TODO Check is reset IGA2 channel before disable IGA2 channel
         is necessary or it may cause some line garbage. *=*/
    ViaDisplayInit(pScrn);

    /* Gamma must be disabled before restoring palette */
    ViaGammaDisable(pScrn);

    if (pBIOSInfo->TVI2CDev)
        ViaTVRestore(pScrn);

    /* Restore the standard VGA registers. */
    if (xf86IsPrimaryPci(pVia->PciInfo))
        vgaHWRestore(pScrn, &hwp->SavedReg, VGA_SR_ALL);
    else
        vgaHWRestore(pScrn, &hwp->SavedReg, VGA_SR_MODE);

    /* Restore extended registers. */
    hwp->writeSeq(hwp, 0x14, Regs->SR14);
    hwp->writeSeq(hwp, 0x15, Regs->SR15);
    hwp->writeSeq(hwp, 0x16, Regs->SR16);
    hwp->writeSeq(hwp, 0x17, Regs->SR17);
    hwp->writeSeq(hwp, 0x18, Regs->SR18);
    hwp->writeSeq(hwp, 0x19, Regs->SR19);
    hwp->writeSeq(hwp, 0x1A, Regs->SR1A);
    hwp->writeSeq(hwp, 0x1B, Regs->SR1B);
    hwp->writeSeq(hwp, 0x1C, Regs->SR1C);
    hwp->writeSeq(hwp, 0x1D, Regs->SR1D);
    hwp->writeSeq(hwp, 0x1E, Regs->SR1E);
    hwp->writeSeq(hwp, 0x1F, Regs->SR1F);

    hwp->writeSeq(hwp, 0x22, Regs->SR22);
    hwp->writeSeq(hwp, 0x23, Regs->SR23);
    hwp->writeSeq(hwp, 0x24, Regs->SR24);
    hwp->writeSeq(hwp, 0x25, Regs->SR25);
    hwp->writeSeq(hwp, 0x26, Regs->SR26);
    hwp->writeSeq(hwp, 0x27, Regs->SR27);
    hwp->writeSeq(hwp, 0x28, Regs->SR28);
    hwp->writeSeq(hwp, 0x29, Regs->SR29);
    hwp->writeSeq(hwp, 0x2A, Regs->SR2A);
    hwp->writeSeq(hwp, 0x2B, Regs->SR2B);

    hwp->writeSeq(hwp, 0x2E, Regs->SR2E);

    /*=* restore VCK, LCDCK and ECK *=*/
    /* Primary Display (VCK): */
    hwp->writeSeq(hwp, 0x44, Regs->SR44);
    hwp->writeSeq(hwp, 0x45, Regs->SR45);
    hwp->writeSeq(hwp, 0x46, Regs->SR46);

    /* Reset VCK PLL */
    hwp->writeSeq(hwp, 0x40, hwp->readSeq(hwp, 0x40) | 0x02); /* Set SR40[1] to 1 */
    hwp->writeSeq(hwp, 0x40, hwp->readSeq(hwp, 0x40) & 0xFD); /* Set SR40[1] to 0 */

    /* ECK Clock Synthesizer: */
    hwp->writeSeq(hwp, 0x47, Regs->SR47);
    hwp->writeSeq(hwp, 0x48, Regs->SR48);
    hwp->writeSeq(hwp, 0x49, Regs->SR49);

    /* Reset ECK PLL */
    hwp->writeSeq(hwp, 0x40, hwp->readSeq(hwp, 0x40) | 0x01); /* Set SR40[0] to 1 */
    hwp->writeSeq(hwp, 0x40, hwp->readSeq(hwp, 0x40) & 0xFE); /* Set SR40[0] to 0 */

    switch (pVia->Chipset) {
        case VIA_CLE266:
        case VIA_KM400:
            break;
        default:
            /* Secondary Display (LCDCK): */
            hwp->writeSeq(hwp, 0x4A, Regs->SR4A);
            hwp->writeSeq(hwp, 0x4B, Regs->SR4B);
            hwp->writeSeq(hwp, 0x4C, Regs->SR4C);

            /* Reset LCK PLL */
            hwp->writeSeq(hwp, 0x40, hwp->readSeq(hwp, 0x40) | 0x04); /* Set SR40[2] to 1 */
            hwp->writeSeq(hwp, 0x40, hwp->readSeq(hwp, 0x40) & 0xFB); /* Set SR40[2] to 0 */
            break;
    }

    /* Restore Preemptive Arbiter Control Register
     * VX800 and VX855 should restore this register too,
     * but I don't do that for I don't want to affect any
     * chips now.
     */
    if (pVia->Chipset == VIA_VX900) {
        hwp->writeSeq(hwp, 0x4D, Regs->SR4D);
    }

    /* Reset dotclocks. */
    ViaSeqMask(hwp, 0x40, 0x06, 0x06);
    ViaSeqMask(hwp, 0x40, 0x00, 0x06);

    /* Integrated LVDS Mode Select */
    hwp->writeCrtc(hwp, 0x13, Regs->CR13);

    /*=* Restore CRTC controller extended regs: *=*/
    /* Mode Control */
    hwp->writeCrtc(hwp, 0x32, Regs->CR32);
    /* HSYNCH Adjuster */
    hwp->writeCrtc(hwp, 0x33, Regs->CR33);
    /* Extended Overflow */
    hwp->writeCrtc(hwp, 0x35, Regs->CR35);
    /*Power Management 3 (Monitor Control) */
    hwp->writeCrtc(hwp, 0x36, Regs->CR36);

    /* Starting Address */
    /* Start Address High */
    hwp->writeCrtc(hwp, 0x0C, Regs->CR0C);
    /* Start Address Low */
    hwp->writeCrtc(hwp, 0x0D, Regs->CR0D);
    /* Starting Address Overflow Bits[28:24] */
    hwp->writeCrtc(hwp, 0x48, Regs->CR48);
    /* CR34 are fire bits. Must be written after CR0C CR0D CR48.  */
    /* Starting Address Overflow Bits[23:16] */
    hwp->writeCrtc(hwp, 0x34, Regs->CR34);
    hwp->writeCrtc(hwp, 0x49, Regs->CR49);

    /* Restore LCD control registers. */
    for (i = 0; i < 68; i++)
        hwp->writeCrtc(hwp, i + 0x50, Regs->CRTCRegs[i]);

    if (pVia->Chipset != VIA_CLE266 && pVia->Chipset != VIA_KM400) {
        /* Scaling Initial values */
        hwp->writeCrtc(hwp, 0xA0, Regs->CRA0);
        hwp->writeCrtc(hwp, 0xA1, Regs->CRA1);
        hwp->writeCrtc(hwp, 0xA2, Regs->CRA2);

        /* LVDS Channels Functions Selection */
        hwp->writeCrtc(hwp, 0x97, Regs->CR97);
        hwp->writeCrtc(hwp, 0x99, Regs->CR99);
        hwp->writeCrtc(hwp, 0x9B, Regs->CR9B);
        hwp->writeCrtc(hwp, 0x9F, Regs->CR9F);
    }

    /* Restore TMDS status */
    switch (pVia->Chipset) {
        case VIA_CX700:
        case VIA_VX800:
        case VIA_VX855:
        case VIA_VX900:
            /* LVDS Control Register */
            hwp->writeCrtc(hwp, 0xD2, Regs->CRD2);
            break;
    }

    ViaDisablePrimaryFIFO(pScrn);

    /* Reset clock. */
    tmp = hwp->readMiscOut(hwp);
    hwp->writeMiscOut(hwp, tmp);

    vgaHWProtect(pScrn, FALSE);
}

#ifdef HAVE_DEBUG
void
ViaVgahwPrint(vgaHWPtr hwp)
{
    int i;

    if (!hwp)
	return;

    xf86DrvMsg(hwp->pScrn->scrnIndex, X_INFO, "VGA Sequence registers:\n");
    for (i = 0x00; i < 0x80; i++)
        xf86DrvMsg(hwp->pScrn->scrnIndex, X_INFO,
                   "SR%02X: 0x%02X\n", i, hwp->readSeq(hwp, i));

    xf86DrvMsg(hwp->pScrn->scrnIndex, X_INFO, "VGA CRTM/C registers:\n");
    for (i = 0x00; i < 0x19; i++)
        xf86DrvMsg(hwp->pScrn->scrnIndex, X_INFO,
                   "CR%02X: 0x%02X\n", i, hwp->readCrtc(hwp, i));
    for (i = 0x33; i < 0xA3; i++)
        xf86DrvMsg(hwp->pScrn->scrnIndex, X_INFO,
                   "CR%02X: 0x%02X\n", i, hwp->readCrtc(hwp, i));

    xf86DrvMsg(hwp->pScrn->scrnIndex, X_INFO, "VGA Graphics registers:\n");
    for (i = 0x00; i < 0x08; i++)
        xf86DrvMsg(hwp->pScrn->scrnIndex, X_INFO,
                   "GR%02X: 0x%02X\n", i, hwp->readGr(hwp, i));

    xf86DrvMsg(hwp->pScrn->scrnIndex, X_INFO, "VGA Attribute registers:\n");
    for (i = 0x00; i < 0x14; i++)
        xf86DrvMsg(hwp->pScrn->scrnIndex, X_INFO,
                   "AR%02X: 0x%02X\n", i, hwp->readAttr(hwp, i));

    xf86DrvMsg(hwp->pScrn->scrnIndex, X_INFO, "VGA Miscellaneous register:\n");
    xf86DrvMsg(hwp->pScrn->scrnIndex, X_INFO,
               "Misc: 0x%02X\n", hwp->readMiscOut(hwp));

    xf86DrvMsg(hwp->pScrn->scrnIndex, X_INFO, "End of VGA registers.\n");
}
#endif /* HAVE_DEBUG */

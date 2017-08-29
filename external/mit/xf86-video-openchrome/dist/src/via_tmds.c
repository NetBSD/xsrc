/*
 * Copyright 2016 Kevin Brace
 * Copyright 2015-2016 The OpenChrome Project
 *                     [https://www.freedesktop.org/wiki/Openchrome]
 * Copyright 2014 SHS SERVICES GmbH
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
 * via_tmds.c
 *
 * Handles initialization of TMDS (DVI) related resources and 
 * controls the integrated TMDS transmitter found in CX700 and 
 * later VIA Technologies chipsets.
 *
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <unistd.h>
#include "via_driver.h"
#include "via_vt1632.h"
#include "via_sii164.h"


/*
	1. Formula:
		2^13 X 0.0698uSec [1/14.318MHz] = 8192 X 0.0698uSec =572.1uSec
		Timer = Counter x 572 uSec
	2. Note:
		0.0698 uSec is too small to compute for hardware. So we multiply a
		reference value(2^13) to make it big enough to compute for hardware.
	3. Note:
		The meaning of the TD0~TD3 are count of the clock.
		TD(sec) = (sec)/(per clock) x (count of clocks)
*/
#define TD0 200
#define TD1 25
#define TD2 0
#define TD3 25


/*
 * Initializes most registers related to VIA Technologies IGP
 * integrated TMDS transmitter. Synchronization polarity and
 * display output source need to be set separately. */
static void
viaTMDSInitRegisters(ScrnInfoPtr pScrn)
{
    vgaHWPtr hwp = VGAHWPTR(pScrn);

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Entered viaTMDSInitRegisters.\n"));

    /* Activate DVI + LVDS2 mode. */
    /* 3X5.D2[5:4] - Display Channel Select
     *               00: LVDS1 + LVDS2
     *               01: DVI + LVDS2
     *               10: One Dual LVDS Channel (High Resolution Pannel)
     *               11: Single Channel DVI */
    ViaCrtcMask(hwp, 0xD2, 0x10, 0x30);

    /* Various DVI PLL settings should be set to default settings. */
    /* 3X5.D1[7]   - PLL2 Reference Clock Edge Select Bit
     *               0: PLLCK lock to rising edge of reference clock
     *               1: PLLCK lock to falling edge of reference clock
     * 3X5.D1[6:5] - PLL2 Charge Pump Current Set Bits
     *               00: ICH = 12.5 uA
     *               01: ICH = 25.0 uA
     *               10: ICH = 37.5 uA
     *               11: ICH = 50.0 uA
     * 3X5.D1[4:1] - Reserved
     * 3X5.D1[0]   - PLL2 Control Voltage Measurement Enable Bit */
    ViaCrtcMask(hwp, 0xD1, 0x00, 0xE1);

    /* Disable DVI test mode. */
    /* 3X5.D5[7] - PD1 Enable Selection
     *             1: Select by power flag
     *             0: By register
     * 3X5.D5[5] - DVI Testing Mode Enable
     * 3X5.D5[4] - DVI Testing Format Selection
     *             0: Half cycle
     *             1: LFSR mode */
    ViaCrtcMask(hwp, 0xD5, 0x00, 0xB0);

    /* Disable DVI sense interrupt. */
    /* 3C5.2B[7] - DVI Sense Interrupt Enable
     *             0: Disable
     *             1: Enable */
    ViaSeqMask(hwp, 0x2B, 0x00, 0x80);

    /* Clear DVI sense interrupt status. */
    /* 3C5.2B[6] - DVI Sense Interrupt Status
     *             (This bit has a RW1C attribute.) */
    ViaSeqMask(hwp, 0x2B, 0x40, 0x40);

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Exiting viaTMDSInitRegisters.\n"));
}

/*
 * Sets the polarity of horizontal synchronization and vertical
 * synchronization.
 */
static void
viaTMDSSetSyncPolarity(ScrnInfoPtr pScrn, DisplayModePtr mode)
{
    vgaHWPtr hwp = VGAHWPTR(pScrn);
    CARD8 cr97;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Entered viaTMDSSetSyncPolarity.\n"));

    /* 3X5.97[6] - DVI (TMDS) VSYNC Polarity
     *             0: Positive
     *             1: Negative
     * 3X5.97[5] - DVI (TMDS) HSYNC Polarity
     *             0: Positive
     *             1: Negative */
    cr97 = hwp->readCrtc(hwp, 0x97);
    if (mode->Flags & V_NHSYNC) {
        cr97 |= 0x20;
    } else {
        cr97 &= (~0x20);
    }

    if (mode->Flags & V_NVSYNC) {
        cr97 |= 0x40;
    } else {
        cr97 &= (~0x40);
    }

    ViaCrtcMask(hwp, 0x97, cr97, 0x60);

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Exiting viaTMDSSetSyncPolarity.\n"));
}

/*
 * Sets IGA1 or IGA2 as the display output source for VIA Technologies IGP
 * integrated TMDS transmitter.
 */
static void
viaTMDSSetSource(ScrnInfoPtr pScrn, CARD8 displaySource)
{
    vgaHWPtr hwp = VGAHWPTR(pScrn);
    CARD8 temp = displaySource;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Entered viaTMDSSetSource.\n"));

    /* Set integrated TMDS transmitter display output source.
     * The integrated TMDS transmitter appears to utilize LVDS1's data
     * source selection bit (3X5.99[4]). */
    /* 3X5.99[4] - LVDS Channel1 Data Source Selection
     *             0: Primary Display
     *             1: Secondary Display */
    ViaCrtcMask(hwp, 0x99, temp << 4, 0x10);
    xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                "Integrated TMDS Transmitter Display Output Source: IGA%d\n",
                (temp & 0x01) + 1);

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Exiting viaTMDSSetSource.\n"));
}

/*
 * Returns TMDS receiver detection state for VIA Technologies IGP
 * integrated TMDS transmitter.
 */
static Bool
viaTMDSSense(ScrnInfoPtr pScrn)
{
    vgaHWPtr hwp = VGAHWPTR(pScrn);
    VIAPtr pVia = VIAPTR(pScrn);
    CARD8 tmdsReceiverDetected = 0x00;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Entered viaTMDSSense.\n"));

    /* For now, faking DVI detection.*/
    tmdsReceiverDetected = 0x01;

    xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                "Integrated TMDS transmitter %s a TMDS receiver.\n",
                (tmdsReceiverDetected & 0x01) ? "detected" : "did not detect");

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Exiting viaTMDSSense.\n"));
    return tmdsReceiverDetected;
}

static void
viaTMDSPower(ScrnInfoPtr pScrn, Bool powerState)
{
    vgaHWPtr hwp = VGAHWPTR(pScrn);

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Entered viaTMDSPower.\n"));

    if (powerState) {
        /* 3X5.91[7] - Software Direct On / Off Display Period
                       in the Panel Path
                       0: On
                       1: Off */
        ViaCrtcMask(hwp, 0x91, 0x00, 0x80);

        /* 3X5.91[0] - Hardware or Software Control Power Sequence
                       1: Software Control */
        ViaCrtcMask(hwp, 0x91, 0x01, 0x01);

        usleep(TD0);

        /* 3X5.91[4] - Software VDD On
                       0: Off
                       1: On */
        ViaCrtcMask(hwp, 0x91, 0x10, 0x10);

        usleep(TD1);

        /* 3X5.91[3] - Software Data On
                       0: Off
                       1: On */
        ViaCrtcMask(hwp, 0x91, 0x08, 0x08);

        /* 3X5.D2[3] - Power Down (Active High) for DVI
         *             0: TMDS power on
         *             1: TMDS power down */
        ViaCrtcMask(hwp, 0xD2, 0x00, 0x08);
    } else {
        ViaCrtcMask(hwp, 0xD2, 0x08, 0x08);

        ViaCrtcMask(hwp, 0x91, 0x00, 0x08);

        usleep(TD1);

        ViaCrtcMask(hwp, 0x91, 0x00, 0x10);
    }

    xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                "Integrated TMDS (DVI) Power: %s\n",
                powerState ? "On" : "Off");

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Exiting viaTMDSPower.\n"));
}

static void
viaTMDSIOPadSetting(ScrnInfoPtr pScrn, Bool ioPadOn)
{
    vgaHWPtr hwp = VGAHWPTR(pScrn);
    VIAPtr pVia = VIAPTR(pScrn);
    CARD8 sr12, sr13, sr5a;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Entered viaTMDSIOPadSetting.\n"));

    if ((pVia->Chipset == VIA_CX700)
        || (pVia->Chipset == VIA_VX800)
        || (pVia->Chipset == VIA_VX855)
        || (pVia->Chipset == VIA_VX900)) {

        sr5a = hwp->readSeq(hwp, 0x5A);
        DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                            "SR5A: 0x%02X\n", sr5a));
        DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                            "Setting 3C5.5A[0] to 0.\n"));
        ViaSeqMask(hwp, 0x5A, sr5a & 0xFE, 0x01);
    }

    sr12 = hwp->readSeq(hwp, 0x12);
    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "SR12: 0x%02X\n", sr12));
    sr13 = hwp->readSeq(hwp, 0x13);
    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "SR13: 0x%02X\n", sr13));

    switch (pVia->Chipset) {
    case VIA_CX700:
    case VIA_VX800:
    case VIA_VX855:
    case VIA_VX900:
        /* 3C5.13[7:6] - DVP1D15 and DVP1D14 pin strappings
         *               00: LVDS1 + LVDS2
         *               01: DVI + LVDS2
         *               10: Dual LVDS (LVDS1 + LVDS2 used 
         *                   simultaneously)
         *               11: DVI only */
        if ((((~(sr13 & 0x80)) && (sr13 & 0x40))
             || ((sr13 & 0x80) && (sr13 & 0x40)))
           || (pVia->isVIANanoBook)) {

            viaLVDS1SetIOPadSetting(pScrn, ioPadOn ? 0x03 : 0x00);
        }

        break;
    default:
        break;
    }

    if ((pVia->Chipset == VIA_CX700)
        || (pVia->Chipset == VIA_VX800)
        || (pVia->Chipset == VIA_VX855)
        || (pVia->Chipset == VIA_VX900)) {

        hwp->writeSeq(hwp, 0x5A, sr5a);
        DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                            "Restoring 3C5.5A[0].\n"));
    }

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Exiting viaTMDSIOPadSetting.\n"));
}

void
viaExtTMDSSetDisplaySource(ScrnInfoPtr pScrn, CARD8 displaySource)
{
    vgaHWPtr hwp = VGAHWPTR(pScrn);
    VIAPtr pVia = VIAPTR(pScrn);
    CARD8 sr12, sr13, sr5a;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Entered viaExtTMDSSetDisplaySource.\n"));

    if ((pVia->Chipset == VIA_CX700)
        || (pVia->Chipset == VIA_VX800)
        || (pVia->Chipset == VIA_VX855)
        || (pVia->Chipset == VIA_VX900)) {

        sr5a = hwp->readSeq(hwp, 0x5A);
        DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                            "SR5A: 0x%02X\n", sr5a));
        DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                            "Setting 3C5.5A[0] to 0.\n"));
        ViaSeqMask(hwp, 0x5A, sr5a & 0xFE, 0x01);
    }

    sr12 = hwp->readSeq(hwp, 0x12);
    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "SR12: 0x%02X\n", sr12));
    sr13 = hwp->readSeq(hwp, 0x13);
    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "SR13: 0x%02X\n", sr13));
    switch (pVia->Chipset) {
    case VIA_CLE266:
        /* 3C5.12[5] - FPD18 pin strapping
         *             0: DIP0 (Digital Interface Port 0) is used by
         *                a TMDS transmitter (DVI)
         *             1: DIP0 (Digital Interface Port 0) is used by
         *                a TV encoder */
        if (!(sr12 & 0x20)) {
            viaDIP0SetDisplaySource(pScrn, displaySource);
        } else {
            xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
                        "DIP0 was not set up for "
                        "TMDS transmitter use.\n");
        }

        break;
    case VIA_KM400:
    case VIA_K8M800:
    case VIA_PM800:
    case VIA_P4M800PRO:
        /* 3C5.13[3] - DVP0D8 pin strapping
         *             0: AGP pins are used for AGP
         *             1: AGP pins are used by FPDP
         *                (Flat Panel Display Port)
         * 3C5.12[6] - DVP0D6 pin strapping
         *             0: Disable DVP0 (Digital Video Port 0)
         *             1: Enable DVP0 (Digital Video Port 0)
         * 3C5.12[5] - DVP0D5 pin strapping
         *             0: DVP0 is used by a TMDS transmitter (DVI)
         *             1: DVP0 is used by a TV encoder
         * 3C5.12[4] - DVP0D4 pin strapping
         *             0: Dual 12-bit FPDP (Flat Panel Display Port)
         *             1: 24-bit FPDP (Flat Panel Display Port) */
        if ((sr12 & 0x40) && (!(sr12 & 0x20))) {
            viaDVP0SetDisplaySource(pScrn, displaySource);
        } else if ((sr13 & 0x08) && (!(sr12 & 0x10))) {
            viaDFPLowSetDisplaySource(pScrn, displaySource);
        } else {
            xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
                        "None of the external ports were set up for "
                        "TMDS transmitter use.\n");
        }

        break;
    case VIA_P4M890:
    case VIA_K8M890:
    case VIA_P4M900:
        /* 3C5.12[6] - FPD6 pin strapping
         *             0: Disable DVP0 (Digital Video Port 0)
         *             1: Enable DVP0 (Digital Video Port 0)
         * 3C5.12[5] - FPD5 pin strapping
         *             0: DVP0 is used by a TMDS transmitter (DVI)
         *             1: DVP0 is used by a TV encoder */
        if ((sr12 & 0x40) && (!(sr12 & 0x20))) {
            viaDVP0SetDisplaySource(pScrn, displaySource);
        } else if (!(sr12 & 0x10)) {
            viaDFPLowSetDisplaySource(pScrn, displaySource);
        } else {
            xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
                        "None of the external ports were set up for "
                        "TMDS transmitter use.\n");
        }

        break;
    case VIA_CX700:
    case VIA_VX800:
    case VIA_VX855:
    case VIA_VX900:
        /* 3C5.13[6] - DVP1 DVP / capture port selection
         *             0: DVP1 is used as a DVP (Digital Video Port)
         *             1: DVP1 is used as a capture port
         */
        if (!(sr13 & 0x40)) {
            viaDVP1SetDisplaySource(pScrn, displaySource);
        } else {
            xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
                        "DVP1 is not set up for TMDS "
                        "transmitter use.\n");
        }

        break;
    default:
        xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
                    "Unrecognized IGP for "
                    "TMDS transmitter use.\n");
        break;
    }

    if ((pVia->Chipset == VIA_CX700)
        || (pVia->Chipset == VIA_VX800)
        || (pVia->Chipset == VIA_VX855)
        || (pVia->Chipset == VIA_VX900)) {

        hwp->writeSeq(hwp, 0x5A, sr5a);
        DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                            "Restoring 3C5.5A[0].\n"));
    }

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Exiting viaExtTMDSSetDisplaySource.\n"));
}

void
viaExtTMDSEnableIOPads(ScrnInfoPtr pScrn, CARD8 ioPadState)
{

    vgaHWPtr hwp = VGAHWPTR(pScrn);
    VIAPtr pVia = VIAPTR(pScrn);
    CARD8 sr12, sr13, sr5a;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Entered viaExtTMDSEnableIOPads.\n"));

    if ((pVia->Chipset == VIA_CX700)
        || (pVia->Chipset == VIA_VX800)
        || (pVia->Chipset == VIA_VX855)
        || (pVia->Chipset == VIA_VX900)) {

        sr5a = hwp->readSeq(hwp, 0x5A);
        DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                            "SR5A: 0x%02X\n", sr5a));
        DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                            "Setting 3C5.5A[0] to 0.\n"));
        ViaSeqMask(hwp, 0x5A, sr5a & 0xFE, 0x01);
    }

    sr12 = hwp->readSeq(hwp, 0x12);
    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "SR12: 0x%02X\n", sr12));
    sr13 = hwp->readSeq(hwp, 0x13);
    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "SR13: 0x%02X\n", sr13));
    switch (pVia->Chipset) {
    case VIA_CLE266:
        /* 3C5.12[5] - FPD18 pin strapping
         *             0: DIP0 (Digital Interface Port 0) is used by
         *                a TMDS transmitter (DVI)
         *             1: DIP0 (Digital Interface Port 0) is used by
         *                a TV encoder */
        if (!(sr12 & 0x20)) {
            viaDIP0EnableIOPads(pScrn, ioPadState);
        } else {
            xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
                        "DIP0 was not set up for "
                        "TMDS transmitter use.\n");
        }

        break;
    case VIA_KM400:
    case VIA_K8M800:
    case VIA_PM800:
    case VIA_P4M800PRO:
        /* 3C5.13[3] - DVP0D8 pin strapping
         *             0: AGP pins are used for AGP
         *             1: AGP pins are used by FPDP
         *                (Flat Panel Display Port)
         * 3C5.12[6] - DVP0D6 pin strapping
         *             0: Disable DVP0 (Digital Video Port 0)
         *             1: Enable DVP0 (Digital Video Port 0)
         * 3C5.12[5] - DVP0D5 pin strapping
         *             0: DVP0 is used by a TMDS transmitter (DVI)
         *             1: DVP0 is used by a TV encoder
         * 3C5.12[4] - DVP0D4 pin strapping
         *             0: Dual 12-bit FPDP (Flat Panel Display Port)
         *             1: 24-bit FPDP (Flat Panel Display Port) */
        if ((sr12 & 0x40) && (!(sr12 & 0x20))) {
            viaDVP0EnableIOPads(pScrn, ioPadState);
        } else if ((sr13 & 0x08) && (!(sr12 & 0x10))) {
            viaDFPLowEnableIOPads(pScrn, ioPadState);
        } else {
            xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
                        "None of the external ports were set up for "
                        "TMDS transmitter use.\n");
        }

        break;
    case VIA_P4M890:
    case VIA_K8M890:
    case VIA_P4M900:
        /* 3C5.12[6] - FPD6 pin strapping
         *             0: Disable DVP0 (Digital Video Port 0)
         *             1: Enable DVP0 (Digital Video Port 0)
         * 3C5.12[5] - FPD5 pin strapping
         *             0: DVP0 is used by a TMDS transmitter (DVI)
         *             1: DVP0 is used by a TV encoder */
        if ((sr12 & 0x40) && (!(sr12 & 0x20))) {
            viaDVP0EnableIOPads(pScrn, ioPadState);
        } else if (!(sr12 & 0x10)) {
            viaDFPLowEnableIOPads(pScrn, ioPadState);
        } else {
            xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
                        "None of the external ports were set up for "
                        "TMDS transmitter use.\n");
        }

        break;
    case VIA_CX700:
    case VIA_VX800:
    case VIA_VX855:
    case VIA_VX900:
        /* 3C5.13[6] - DVP1 DVP / capture port selection
         *             0: DVP1 is used as a DVP (Digital Video Port)
         *             1: DVP1 is used as a capture port
         */
        if (!(sr13 & 0x40)) {
            viaDVP1EnableIOPads(pScrn, ioPadState);
        } else {
            xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
                        "DVP1 is not set up for TMDS "
                        "transmitter use.\n");
        }

        break;
    default:
        xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
                    "Unrecognized IGP for "
                    "TMDS transmitter use.\n");
        break;
    }

    if ((pVia->Chipset == VIA_CX700)
        || (pVia->Chipset == VIA_VX800)
        || (pVia->Chipset == VIA_VX855)
        || (pVia->Chipset == VIA_VX900)) {

        hwp->writeSeq(hwp, 0x5A, sr5a);
        DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                            "Restoring 3C5.5A[0].\n"));
    }

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Exiting viaExtTMDSEnableIOPads.\n"));
}

void
viaExtTMDSSetClockDriveStrength(ScrnInfoPtr pScrn, CARD8 clockDriveStrength)
{
    vgaHWPtr hwp = VGAHWPTR(pScrn);
    VIAPtr pVia = VIAPTR(pScrn);
    CARD8 sr12, sr13, sr5a;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Entered viaExtTMDSSetClockDriveStrength.\n"));

    if ((pVia->Chipset == VIA_CX700)
        || (pVia->Chipset == VIA_VX800)
        || (pVia->Chipset == VIA_VX855)
        || (pVia->Chipset == VIA_VX900)) {

        sr5a = hwp->readSeq(hwp, 0x5A);
        DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                            "SR5A: 0x%02X\n", sr5a));
        DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                            "Setting 3C5.5A[0] to 0.\n"));
        ViaSeqMask(hwp, 0x5A, sr5a & 0xFE, 0x01);
    }

    sr12 = hwp->readSeq(hwp, 0x12);
    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "SR12: 0x%02X\n", sr12));
    sr13 = hwp->readSeq(hwp, 0x13);
    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "SR13: 0x%02X\n", sr13));
    switch (pVia->Chipset) {
    case VIA_CLE266:
        /* 3C5.12[5] - FPD18 pin strapping
         *             0: DIP0 (Digital Interface Port 0) is used by
         *                a TMDS transmitter (DVI)
         *             1: DIP0 (Digital Interface Port 0) is used by
         *                a TV encoder */
        if (!(sr12 & 0x20)) {
            viaDIP0SetClockDriveStrength(pScrn, clockDriveStrength);
        }

        break;
    case VIA_KM400:
    case VIA_K8M800:
    case VIA_PM800:
    case VIA_P4M800PRO:
        /* 3C5.12[6] - DVP0D6 pin strapping
         *             0: Disable DVP0 (Digital Video Port 0)
         *             1: Enable DVP0 (Digital Video Port 0)
         * 3C5.12[5] - DVP0D5 pin strapping
         *             0: DVP0 is used by a TMDS transmitter (DVI)
         *             1: DVP0 is used by a TV encoder */
        if ((sr12 & 0x40) && (!(sr12 & 0x20))) {
            viaDVP0SetClockDriveStrength(pScrn, clockDriveStrength);
        }

        break;
    case VIA_P4M890:
    case VIA_K8M890:
    case VIA_P4M900:
        /* 3C5.12[6] - FPD6 pin strapping
         *             0: Disable DVP0 (Digital Video Port 0)
         *             1: Enable DVP0 (Digital Video Port 0)
         * 3C5.12[5] - FPD5 pin strapping
         *             0: DVP0 is used by a TMDS transmitter (DVI)
         *             1: DVP0 is used by a TV encoder */
        if ((sr12 & 0x40) && (!(sr12 & 0x20))) {
            viaDVP0SetClockDriveStrength(pScrn, clockDriveStrength);
        }

        break;
    case VIA_CX700:
    case VIA_VX800:
    case VIA_VX855:
    case VIA_VX900:
        /* 3C5.13[6] - DVP1 DVP / capture port selection
         *             0: DVP1 is used as a DVP (Digital Video Port)
         *             1: DVP1 is used as a capture port */
        if (!(sr13 & 0x40)) {
            viaDVP1SetClockDriveStrength(pScrn, clockDriveStrength);
        }

        break;
    default:
        break;
    }

    if ((pVia->Chipset == VIA_CX700)
        || (pVia->Chipset == VIA_VX800)
        || (pVia->Chipset == VIA_VX855)
        || (pVia->Chipset == VIA_VX900)) {

        hwp->writeSeq(hwp, 0x5A, sr5a);
        DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                            "Restoring 3C5.5A[0].\n"));
    }

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Exiting viaExtTMDSSetClockDriveStrength.\n"));
}

void
viaExtTMDSSetDataDriveStrength(ScrnInfoPtr pScrn, CARD8 dataDriveStrength)
{
    vgaHWPtr hwp = VGAHWPTR(pScrn);
    VIAPtr pVia = VIAPTR(pScrn);
    CARD8 sr12, sr13, sr5a;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Entered viaExtTMDSSetDataDriveStrength.\n"));

    if ((pVia->Chipset == VIA_CX700)
        || (pVia->Chipset == VIA_VX800)
        || (pVia->Chipset == VIA_VX855)
        || (pVia->Chipset == VIA_VX900)) {

        sr5a = hwp->readSeq(hwp, 0x5A);
        DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                            "SR5A: 0x%02X\n", sr5a));
        DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                            "Setting 3C5.5A[0] to 0.\n"));
        ViaSeqMask(hwp, 0x5A, sr5a & 0xFE, 0x01);
    }

    sr12 = hwp->readSeq(hwp, 0x12);
    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "SR12: 0x%02X\n", sr12));
    sr13 = hwp->readSeq(hwp, 0x13);
    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "SR13: 0x%02X\n", sr13));
    switch (pVia->Chipset) {
    case VIA_CLE266:
        /* 3C5.12[5] - FPD18 pin strapping
         *             0: DIP0 (Digital Interface Port 0) is used by
         *                a TMDS transmitter (DVI)
         *             1: DIP0 (Digital Interface Port 0) is used by
         *                a TV encoder */
        if (!(sr12 & 0x20)) {
            viaDIP0SetDataDriveStrength(pScrn, dataDriveStrength);
        }

        break;
    case VIA_KM400:
    case VIA_K8M800:
    case VIA_PM800:
    case VIA_P4M800PRO:
        /* 3C5.12[6] - DVP0D6 pin strapping
         *             0: Disable DVP0 (Digital Video Port 0)
         *             1: Enable DVP0 (Digital Video Port 0)
         * 3C5.12[5] - DVP0D5 pin strapping
         *             0: DVP0 is used by a TMDS transmitter (DVI)
         *             1: DVP0 is used by a TV encoder */
        if ((sr12 & 0x40) && (!(sr12 & 0x20))) {
            viaDVP0SetDataDriveStrength(pScrn, dataDriveStrength);
        }

        break;
    case VIA_P4M890:
    case VIA_K8M890:
    case VIA_P4M900:
        /* 3C5.12[6] - FPD6 pin strapping
         *             0: Disable DVP0 (Digital Video Port 0)
         *             1: Enable DVP0 (Digital Video Port 0)
         * 3C5.12[5] - FPD5 pin strapping
         *             0: DVP0 is used by a TMDS transmitter (DVI)
         *             1: DVP0 is used by a TV encoder */
        if ((sr12 & 0x40) && (!(sr12 & 0x20))) {
            viaDVP0SetDataDriveStrength(pScrn, dataDriveStrength);
        }

        break;
    case VIA_CX700:
    case VIA_VX800:
    case VIA_VX855:
    case VIA_VX900:
        /* 3C5.13[6] - DVP1 DVP / capture port selection
         *             0: DVP1 is used as a DVP (Digital Video Port)
         *             1: DVP1 is used as a capture port */
        if (!(sr13 & 0x40)) {
            viaDVP1SetDataDriveStrength(pScrn, dataDriveStrength);
        }

        break;
    default:
        break;
    }

    if ((pVia->Chipset == VIA_CX700)
        || (pVia->Chipset == VIA_VX800)
        || (pVia->Chipset == VIA_VX855)
        || (pVia->Chipset == VIA_VX900)) {

        hwp->writeSeq(hwp, 0x5A, sr5a);
        DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                            "Restoring 3C5.5A[0].\n"));
    }

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Exiting viaExtTMDSSetDataDriveStrength.\n"));
}

static void
via_tmds_create_resources(xf86OutputPtr output)
{
    ScrnInfoPtr pScrn = output->scrn;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Entered via_tmds_create_resources.\n"));

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Exiting via_tmds_create_resources.\n"));
}

static void
via_tmds_dpms(xf86OutputPtr output, int mode)
{
    ScrnInfoPtr pScrn = output->scrn;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Entered via_tmds_dpms.\n"));

    switch (mode) {
    case DPMSModeOn:
        viaTMDSPower(pScrn, TRUE);
        viaTMDSIOPadSetting(pScrn, TRUE);
        break;
    case DPMSModeStandby:
    case DPMSModeSuspend:
    case DPMSModeOff:
        viaTMDSPower(pScrn, FALSE);
        viaTMDSIOPadSetting(pScrn, FALSE);
        break;
    default:
        break;
    }

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Exiting via_tmds_dpms.\n"));
}

static void
via_tmds_save(xf86OutputPtr output)
{
    ScrnInfoPtr pScrn = output->scrn;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Entered via_tmds_save.\n"));

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Exiting via_tmds_save.\n"));
}

static void
via_tmds_restore(xf86OutputPtr output)
{
    ScrnInfoPtr pScrn = output->scrn;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Entered via_tmds_restore.\n"));

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Exiting via_tmds_restore.\n"));
}

static int
via_tmds_mode_valid(xf86OutputPtr output, DisplayModePtr pMode)
{
    ScrnInfoPtr pScrn = output->scrn;
    int status;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Entered via_tmds_mode_valid.\n"));

    if (!ViaModeDotClockTranslate(pScrn, pMode)) {
        status = MODE_NOCLOCK;
    } else {
        status = MODE_OK;
    }

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Exiting via_tmds_mode_valid.\n"));
    return status;
}

static Bool
via_tmds_mode_fixup(xf86OutputPtr output, DisplayModePtr mode,
                      DisplayModePtr adjusted_mode)
{
    ScrnInfoPtr pScrn = output->scrn;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Entered via_tmds_mode_fixup.\n"));

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Exiting via_tmds_mode_fixup.\n"));
    return TRUE;
}

static void
via_tmds_prepare(xf86OutputPtr output)
{
    ScrnInfoPtr pScrn = output->scrn;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Entered via_tmds_prepare.\n"));

    viaTMDSPower(pScrn, FALSE);
    viaTMDSIOPadSetting(pScrn, FALSE);

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Exiting via_tmds_prepare.\n"));
}

static void
via_tmds_commit(xf86OutputPtr output)
{
    ScrnInfoPtr pScrn = output->scrn;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Entered via_tmds_commit.\n"));

    viaTMDSPower(pScrn, TRUE);
    viaTMDSIOPadSetting(pScrn, TRUE);

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Exiting via_tmds_commit.\n"));
}

static void
via_tmds_mode_set(xf86OutputPtr output, DisplayModePtr mode,
                    DisplayModePtr adjusted_mode)
{
    ScrnInfoPtr pScrn = output->scrn;
    drmmode_crtc_private_ptr iga = output->crtc->driver_private;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Entered via_tmds_mode_set.\n"));

    /* Initialize VIA IGP integrated TMDS transmitter registers. */
    viaTMDSInitRegisters(pScrn);

    /* Set integrated TMDS transmitter synchronization polarity for
     * both horizontal synchronization and vertical synchronization. */
    viaTMDSSetSyncPolarity(pScrn, adjusted_mode);

    if (output->crtc) {
        viaTMDSSetSource(pScrn, iga->index ? 0x01 : 0x00);
    }

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Exiting via_tmds_mode_set.\n"));
}

static xf86OutputStatus
via_tmds_detect(xf86OutputPtr output)
{
    xf86MonPtr mon;
    xf86OutputStatus status = XF86OutputStatusDisconnected;
    ScrnInfoPtr pScrn = output->scrn;
    VIAPtr pVia = VIAPTR(pScrn);

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Entered via_tmds_detect.\n"));

    /* Check for DVI presence by sensing the TMDS receiver connected
     * to the integrated TMDS transmitter. */
    if (viaTMDSSense(pScrn)) {

        if (!pVia->pI2CBus2) {
            goto exit;
        }

        /* Assume that only I2C bus 2 is used for the DVI connected to the
         * integrated TMDS transmitter. */
        if (!xf86I2CProbeAddress(pVia->pI2CBus2, 0xA0)) {
            xf86DrvMsg(pScrn->scrnIndex, X_PROBED,
                        "I2C device on I2C Bus 2 does not support EDID.\n");
            goto exit;
        }

        xf86DrvMsg(pScrn->scrnIndex, X_PROBED,
                    "Obtaining EDID for DVI.\n");

        /* Since DVI presence was established, access the I2C bus,
         * in order to obtain EDID from the monitor. */
        mon = xf86OutputGetEDID(output, pVia->pI2CBus2);

        /* Is the interface type digital? */
        if (mon && DIGITAL(mon->features.input_type)) {
            status = XF86OutputStatusConnected;
            xf86DrvMsg(pScrn->scrnIndex, X_PROBED,
                        "Detected a monitor connected to DVI.\n");
            xf86OutputSetEDID(output, mon);
        } else {
            xf86DrvMsg(pScrn->scrnIndex, X_PROBED,
                        "Could not obtain EDID from a monitor "
                        "connected to DVI.\n");
        }
    }

exit:
    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Exiting via_tmds_detect.\n"));
    return status;
}

#ifdef RANDR_12_INTERFACE
static Bool
via_tmds_set_property(xf86OutputPtr output, Atom property,
                     RRPropertyValuePtr value)
{
    ScrnInfoPtr pScrn = output->scrn;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Entered via_tmds_set_property.\n"));

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Exiting via_tmds_set_property.\n"));
    return TRUE;
}
#endif

#ifdef RANDR_13_INTERFACE
static Bool
via_tmds_get_property(xf86OutputPtr output, Atom property)
{
    ScrnInfoPtr pScrn = output->scrn;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Entered via_tmds_get_property.\n"));

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Exiting via_tmds_get_property.\n"));
    return FALSE;
}
#endif

static void
via_tmds_destroy(xf86OutputPtr output)
{
    ScrnInfoPtr pScrn = output->scrn;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Entered via_tmds_destroy.\n"));

    if (output->driver_private) {
        free(output->driver_private);
    }

    output->driver_private = NULL;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Exiting via_tmds_destroy.\n"));
}




static const xf86OutputFuncsRec via_tmds_funcs = {
    .create_resources   = via_tmds_create_resources,
    .dpms               = via_tmds_dpms,
    .save               = via_tmds_save,
    .restore            = via_tmds_restore,
    .mode_valid         = via_tmds_mode_valid,
    .mode_fixup         = via_tmds_mode_fixup,
    .prepare            = via_tmds_prepare,
    .commit             = via_tmds_commit,
    .mode_set           = via_tmds_mode_set,
    .detect             = via_tmds_detect,
    .get_modes          = xf86OutputGetEDIDModes,
#ifdef RANDR_12_INTERFACE
    .set_property       = via_tmds_set_property,
#endif
#ifdef RANDR_13_INTERFACE
    .get_property       = via_tmds_get_property,
#endif
    .destroy            = via_tmds_destroy,
};


Bool
viaTMDSInit(ScrnInfoPtr pScrn)
{
    xf86OutputPtr output;
    vgaHWPtr hwp = VGAHWPTR(pScrn);
    VIAPtr pVia = VIAPTR(pScrn);
    VIATMDSRecPtr pVIATMDSRec = NULL;
    CARD8 sr13, sr5a;
    Bool status = FALSE;
    char outputNameBuffer[32];

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Entered viaTMDSInit.\n"));

    sr5a = hwp->readSeq(hwp, 0x5A);
    ViaSeqMask(hwp, 0x5A, sr5a | 0x01, 0x01);
    sr13 = hwp->readSeq(hwp, 0x13);
    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "SR13: 0x%02X\n", sr13));
    hwp->writeSeq(hwp, 0x5A, sr5a);

    /* 3C5.13[7:6] - Integrated LVDS / DVI Mode Select
     *               (DVP1D15-14 pin strapping)
     *               00: LVDS1 + LVDS2
     *               01: DVI + LVDS2
     *               10: Dual LVDS Channel (High Resolution Panel)
     *               11: One DVI only (decrease the clock jitter) */
    /* Check for DVI presence using pin strappings.
     * VIA Technologies NanoBook reference design based products
     * have their pin strappings set to a wrong setting to communicate
     * the presence of DVI, so it requires special handling here. */
    if ((((~(sr13 & 0x80)) && (sr13 & 0x40))
         || ((sr13 & 0x80) && (sr13 & 0x40)))
       || (pVia->isVIANanoBook)) {

        xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                    "Integrated TMDS transmitter found via pin strapping.\n");
    } else {
        xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                    "Integrated TMDS transmitter not found.\n");
        goto exit;
    }

    pVIATMDSRec = xnfcalloc(1, sizeof(VIATMDSRec));
    if (!pVIATMDSRec) {
        xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
                    "Failed to allocate working storage for integrated "
                    "TMDS transmitter.\n");
        goto exit;
    }

    /* The code to dynamically designate the particular DVI (i.e., DVI-1,
     * DVI-2, etc.) for xrandr was borrowed from xf86-video-r128 DDX. */
    sprintf(outputNameBuffer, "DVI-%d", (pVia->numberDVI + 1));
    output = xf86OutputCreate(pScrn, &via_tmds_funcs, outputNameBuffer);
    if (!output) {
        free(pVIATMDSRec);
        xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
                    "Failed to allocate X Server display output record for "
                    "integrated TMDS transmitter.\n");
        goto exit;
    }

    output->driver_private = pVIATMDSRec;

    /* Since there are two (2) display controllers registered with the
     * X.Org Server and both IGA1 and IGA2 can handle DVI without any
     * limitations, possible_crtcs should be set to 0x3 (0b11) so that
     * either display controller can get assigned to handle DVI. */
    output->possible_crtcs = (1 << 1) | (1 << 0);

    output->possible_clones = 0;
    output->interlaceAllowed = FALSE;
    output->doubleScanAllowed = FALSE;

    pVia->numberDVI++;
    status = TRUE;
exit:
    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Exiting viaTMDSInit.\n"));
    return status;
}

void
via_dvi_init(ScrnInfoPtr pScrn)
{
    VIAPtr pVia = VIAPTR(pScrn);

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Entered via_dvi_init.\n"));

    if (!pVia->pI2CBus2 || !pVia->pI2CBus3) {
        xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
                    "I2C Bus 2 or I2C Bus 3 does not exist.\n");
        DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                    "Exiting via_dvi_init.\n"));
        return;
    }

    /* Check to see if we are dealing with the latest VIA chipsets. */
    if ((pVia->Chipset == VIA_CX700)
        || (pVia->Chipset == VIA_VX800)
        || (pVia->Chipset == VIA_VX855)
        || (pVia->Chipset == VIA_VX900)) {

        if (!viaTMDSInit(pScrn)) {
            xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Integrated TMDS transmitter for DVI not found.\n");
        } else {
            xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Integrated TMDS transmitter for DVI was "
                        "initialized successfully.\n");
        }
    }

    xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                "Probing I2C Bus 2 for VT1632.\n");
    if (!viaVT1632Init(pScrn, pVia->pI2CBus2)) {
        xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                    "I2C Bus 2 was not initialized for DVI use.\n");
    } else {
        xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                    "VT1632 attached to I2C Bus 2 was initialized "
                    "successfully for DVI use.\n");
    }

    xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                "Probing I2C Bus 3 for VT1632.\n");
    if (!viaVT1632Init(pScrn, pVia->pI2CBus3)) {
        xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                    "I2C Bus 3 was not initialized for DVI use.\n");
    } else {
        xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                    "VT1632 attached to I2C Bus 3 was initialized "
                    "successfully for DVI use.\n");
    }

    xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                "Probing I2C Bus 2 for SiI 164.\n");
    if (!viaSiI164Init(pScrn, pVia->pI2CBus2)) {
        xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                    "I2C Bus 2 was not initialized for DVI use.\n");
    } else {
        xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                    "SiI 164 attached to I2C Bus 2 was initialized "
                    "successfully for DVI use.\n");
    }

    xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                "Probing I2C Bus 3 for SiI 164.\n");
    if (!viaSiI164Init(pScrn, pVia->pI2CBus3)) {
        xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                    "I2C Bus 3 was not initialized for DVI use.\n");
    } else {
        xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                    "SiI 164 attached to I2C Bus 3 was initialized "
                    "successfully for DVI use.\n");
    }

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Exiting via_dvi_init.\n"));
}

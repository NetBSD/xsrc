/*
 * Copyright 2007-2015 The Openchrome Project
 *                     [https://www.freedesktop.org/wiki/Openchrome]
 * Copyright 1998-2007 VIA Technologies, Inc. All Rights Reserved.
 * Copyright 2001-2007 S3 Graphics, Inc. All Rights Reserved.
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
 * Integrated LVDS power management functions.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "via_driver.h"
#include "via_mode.h"
#include <unistd.h>

/*
 * Option handling.
 */
enum ViaPanelOpts {
    OPTION_CENTER
};

static OptionInfoRec ViaPanelOptions[] =
{
    {OPTION_CENTER,     "Center",       OPTV_BOOLEAN,   {0},    FALSE},
    {-1,                NULL,           OPTV_NONE,      {0},    FALSE}
};

/* These table values were copied from lcd.c of VIA Frame 
 * Buffer device driver. */
/* {int Width, int Height, bool useDualEdge, bool useDithering}; */
static ViaPanelModeRec ViaPanelNativeModes[] = {
    { 640,  480, FALSE,  TRUE},
    { 800,  600, FALSE,  TRUE},
    {1024,  768, FALSE,  TRUE},
    {1280,  768, FALSE,  TRUE},
    {1280, 1024,  TRUE,  TRUE},
    {1400, 1050,  TRUE,  TRUE},
    {1600, 1200,  TRUE,  TRUE},
    {1280,  800, FALSE,  TRUE},
    { 800,  480, FALSE,  TRUE},
    {1024,  768,  TRUE,  TRUE},
    {1024,  768, FALSE, FALSE},
    {1024,  768,  TRUE, FALSE},
    {1280,  768, FALSE, FALSE},
    {1280, 1024,  TRUE, FALSE},
    {1400, 1050,  TRUE, FALSE},
    {1600, 1200,  TRUE, FALSE},
    {1366,  768, FALSE, FALSE},
    {1024,  600, FALSE,  TRUE},
    {1280,  768,  TRUE,  TRUE},
    {1280,  800, FALSE,  TRUE},
    {1360,  768, FALSE, FALSE},
    {1280,  768,  TRUE, FALSE},
    { 480,  640, FALSE,  TRUE},
    {1200,  900, FALSE, FALSE}};

#define MODEPREFIX(name) NULL, NULL, name, 0, M_T_DRIVER | M_T_DEFAULT
#define MODESUFFIX 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,FALSE,FALSE,0,NULL,0,0.0,0.0

static DisplayModeRec OLPCMode = {
    MODEPREFIX("1200x900"),
    57275, 1200, 1208, 1216, 1240, 0,
    900,  905,  908,  912, 0,
    V_NHSYNC | V_NVSYNC, MODESUFFIX
};

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
 * Sets CX700 or later single chipset's LVDS1 I/O pad state.
 */
void
viaLVDS1SetIOPadSetting(ScrnInfoPtr pScrn, CARD8 ioPadState)
{
    vgaHWPtr hwp = VGAHWPTR(pScrn);

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Entered viaLVDS1SetIOPadSetting.\n"));

    /* Set LVDS1 I/O pad state. */
    /* 3C5.2A[1:0] - LVDS1 I/O Pad Control */
    ViaSeqMask(hwp, 0x2A, ioPadState, 0x03);
    xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                "LVDS1 I/O Pad State: %d\n",
                (ioPadState & 0x03));

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Exiting viaLVDS1SetIOPadSetting.\n"));
}

/*
 * Sets IGA1 or IGA2 as the display output source for VIA Technologies
 * Chrome IGP LVDS1 integrated LVDS transmitter.
 */
static void
viaLVDS1SetDisplaySource(ScrnInfoPtr pScrn, CARD8 displaySource)
{
    vgaHWPtr hwp = VGAHWPTR(pScrn);
    CARD8 temp = displaySource;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Entered viaLVDS1SetDisplaySource.\n"));

    /* Set LVDS1 integrated LVDS transmitter display output source. */
    /* 3X5.99[4] - LVDS Channel 1 Data Source Selection
     *             0: Primary Display
     *             1: Secondary Display */
    ViaCrtcMask(hwp, 0x99, temp << 4, 0x10);
    xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                "LVDS1 Integrated LVDS Transmitter Display Output "
                "Source: IGA%d\n",
                (temp & 0x01) + 1);

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Exiting viaLVDS1SetDisplaySource.\n"));
}

/*
 * Sets LVDS1 (LVDS Channel 1) integrated LVDS transmitter format.
 */
static void
viaLVDS1SetFormat(ScrnInfoPtr pScrn, CARD8 format)
{
    vgaHWPtr hwp = VGAHWPTR(pScrn);

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Entered viaLVDS1SetFormat.\n"));

    /* Set LVDS1 format. */
    /* 3X5.D2[1] - LVDS Channel 1 Format Selection
     *             0: SPWG Mode
     *             1: OPENLDI Mode */
    ViaCrtcMask(hwp, 0xD2, format << 1, 0x02);
    xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                "LVDS1 Format: %s\n",
                (format & 0x01) ? "OPENLDI" : "SPWG");

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Exiting viaLVDS1SetFormat.\n"));
}

/*
 * Sets CX700 or later single chipset's LVDS2 I/O pad state.
 */
static void
viaLVDS2SetIOPadSetting(ScrnInfoPtr pScrn, CARD8 ioPadState)
{
    vgaHWPtr hwp = VGAHWPTR(pScrn);

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Entered viaLVDS2SetIOPadSetting.\n"));

    /* Set LVDS2 I/O pad state. */
    /* 3C5.2A[3:2] - LVDS2 I/O Pad Control */
    ViaSeqMask(hwp, 0x2A, ioPadState << 2, 0x0C);
    xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                "LVDS2 I/O Pad State: %d\n",
                (ioPadState & 0x03));

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Exiting viaLVDS2SetIOPadSetting.\n"));
}

/*
 * Sets IGA1 or IGA2 as the display output source for VIA Technologies
 * Chrome IGP LVDS2 integrated LVDS transmitter.
 */
static void
viaLVDS2SetDisplaySource(ScrnInfoPtr pScrn, CARD8 displaySource)
{
    vgaHWPtr hwp = VGAHWPTR(pScrn);
    CARD8 temp = displaySource;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Entered viaLVDS2SetDisplaySource.\n"));

    /* Set LVDS2 integrated LVDS transmitter display output source. */
    /* 3X5.97[4] - LVDS Channel 2 Data Source Selection
     *             0: Primary Display
     *             1: Secondary Display */
    ViaCrtcMask(hwp, 0x97, temp << 4, 0x10);
    xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                "LVDS2 Integrated LVDS Transmitter Display Output "
                "Source: IGA%d\n",
                (temp & 0x01) + 1);

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Exiting viaLVDS2SetDisplaySource.\n"));
}

/*
 * Sets LVDS2 (LVDS Channel 2) integrated LVDS transmitter delay tap.
 */
static void
viaLVDS2SetDelayTap(ScrnInfoPtr pScrn, CARD8 delayTap)
{
    vgaHWPtr hwp = VGAHWPTR(pScrn);

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Entered viaLVDS2SetDelayTap.\n"));

    /* Set LVDS2 delay tap. */
    /* 3X5.97[3:0] - LVDS2 Delay Tap */
    ViaCrtcMask(hwp, 0x97, delayTap, 0x0F);

    xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                "LVDS2 Delay Tap: %d\n",
                (delayTap & 0x0F));

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Exiting viaLVDS2SetDelayTap.\n"));
}

/*
 * Sets LVDS2 (LVDS Channel 2) integrated LVDS transmitter format.
 */
static void
viaLVDS2SetFormat(ScrnInfoPtr pScrn, CARD8 format)
{
    vgaHWPtr hwp = VGAHWPTR(pScrn);

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Entered viaLVDS2SetFormat.\n"));

    /* Set LVDS2 format. */
    /* 3X5.D2[0] - LVDS Channel 2 Format Selection
     *             0: SPWG Mode
     *             1: OPENLDI Mode */
    ViaCrtcMask(hwp, 0xD2, format, 0x01);
    xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                "LVDS2 Format: %s\n",
                (format & 0x01) ? "OPENLDI" : "SPWG");

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Exiting viaLVDS2SetFormat.\n"));
}

/*
 * Sets IGA1 or IGA2 as the display output source for VIA Technologies
 * Chrome IGP DFP (Digital Flat Panel) High interface.
 */
static void
viaDFPHighSetDisplaySource(ScrnInfoPtr pScrn, CARD8 displaySource)
{
    vgaHWPtr hwp = VGAHWPTR(pScrn);
    CARD8 temp = displaySource;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Entered viaDFPHighSetDisplaySource.\n"));

    /* Set DFP High display output source. */
    /* 3X5.97[4] - DFP High Data Source Selection
     *             0: Primary Display
     *             1: Secondary Display */
    ViaCrtcMask(hwp, 0x97, temp << 4, 0x10);
    xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                "DFP High Display Output Source: IGA%d\n",
                (temp & 0x01) + 1);

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Exiting viaDFPHighSetDisplaySource.\n"));
}

/*
 * Sets DFP (Digital Flat Panel) Low interface delay tap.
 */
static void
viaDFPLowSetDelayTap(ScrnInfoPtr pScrn, CARD8 delayTap)
{
    vgaHWPtr hwp = VGAHWPTR(pScrn);

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Entered viaDFPLowSetDelayTap.\n"));

    /* Set DFP Low interface delay tap. */
    /* 3X5.99[3:0] - DFP Low Delay Tap */
    ViaCrtcMask(hwp, 0x99, delayTap, 0x0F);
    xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                "DFP Low Delay Tap: %d\n",
                (delayTap & 0x0F));

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Exiting viaDFPLowSetDelayTap.\n"));
}

/*
 * Sets DFP (Digital Flat Panel) High interface delay tap.
 */
static void
viaDFPHighSetDelayTap(ScrnInfoPtr pScrn, CARD8 delayTap)
{
    vgaHWPtr hwp = VGAHWPTR(pScrn);

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Entered viaDFPHighSetDelayTap.\n"));

    /* Set DFP High interface delay tap. */
    /* 3X5.97[3:0] - DFP High Delay Tap */
    ViaCrtcMask(hwp, 0x97, delayTap, 0x0F);
    xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                "DFP High Delay Tap: %d\n",
                (delayTap & 0x0F));

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Exiting viaDFPHighSetDelayTap.\n"));
}

/*
 * Turns LVDS2 output color dithering on or off. (18-bit color display vs.
 * 24-bit color display)
 */
static void
viaLVDS2SetDithering(ScrnInfoPtr pScrn, CARD8 ditheringStatus)
{
    vgaHWPtr hwp = VGAHWPTR(pScrn);

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Entered viaLVDS2SetDithering.\n"));

    /* Set LVDS2 output color dithering bit. */
    /* 3X5.D4[6] - LVDS Channel 2 Output Bits
     *             0: 24 bits (dithering off)
     *             1: 18 bits (dithering on) */
    ViaCrtcMask(hwp, 0xD4, ditheringStatus ? 0x40 : 0x00, 0x40);

    xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                "LVDS2 Output Color Dithering: %s\n",
                ditheringStatus ? "On (18 bit)" : "Off (24 bit)");

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Exiting viaLVDS2SetDithering.\n"));
}

/*
 * Sets output format of LVDS2 to rotation or sequential mode.
 */
static void
viaLVDS2SetOutputFormat(ScrnInfoPtr pScrn, CARD8 outputFormat)
{
    vgaHWPtr hwp = VGAHWPTR(pScrn);

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Entered viaLVDS2SetOutputFormat.\n"));

    /* Set LVDS2 output format. */
    /* 3X5.D4[7] - LVDS Channel 2 Output Format
     *             0: Rotation
     *             1: Sequential */
    ViaCrtcMask(hwp, 0xD4, outputFormat ? 0x80 : 0x00, 0x80);

    xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                "LVDS2 Output Format: %s\n",
                outputFormat ? "Sequential" : "Rotation");

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Exiting viaLVDS2SetOutputFormat.\n"));
}

/*
 * Sets PCIe based 2 chip chipset's pin multiplexed DVP0 I/O pad state.
 */
static void
viaDVP0PCIeSetIOPadSetting(ScrnInfoPtr pScrn, CARD8 ioPadState)
{
    vgaHWPtr hwp = VGAHWPTR(pScrn);

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Entered viaDVP0PCIeSetIOPadSetting.\n"));

    /* Set pin multiplexed DVP1 I/O pad state. */
    /* 3C5.2A[3:2] - DVP0 I/O Pad Control */
    ViaSeqMask(hwp, 0x2A, ioPadState << 2, 0x0C);
    xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                "DVP0 I/O Pad State: %d\n",
                (ioPadState & 0x03));

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Exiting viaDVP0PCIeSetIOPadSetting.\n"));
}

/*
 * Sets PCIe based 2 chip chipset's pin multiplexed DVP1 I/O pad state.
 */
static void
viaDVP1PCIeSetIOPadSetting(ScrnInfoPtr pScrn, CARD8 ioPadState)
{
    vgaHWPtr hwp = VGAHWPTR(pScrn);

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Entered viaDVP1PCIeSetIOPadSetting.\n"));

    /* Set pin multiplexed DVP0 I/O pad state. */
    /* 3C5.2A[1:0] - DVP1 I/O Pad Control */
    ViaSeqMask(hwp, 0x2A, ioPadState, 0x03);
    xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                "DVP1 I/O Pad State: %d\n",
                (ioPadState & 0x03));

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Exiting viaDVP1PCIeSetIOPadSetting.\n"));
}

static void
viaFPIOPadSetting(ScrnInfoPtr pScrn, Bool ioPadOn)
{
    vgaHWPtr hwp = VGAHWPTR(pScrn);
    VIAPtr pVia = VIAPTR(pScrn);
    CARD8 sr12, sr13, sr5a;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Entered viaFPIOPadSetting.\n"));

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
        break;
    case VIA_KM400:
    case VIA_K8M800:
    case VIA_PM800:
    case VIA_P4M800PRO:
        break;
    case VIA_P4M890:
    case VIA_K8M890:
    case VIA_P4M900:
        /* The tricky thing about VIA Technologies PCI Express based
         * north bridge / south bridge 2 chip chipset is that
         * it pin multiplexes DVP0 / DVP1 with north bridge's PCI
         * Express x16 link. In particular, HP 2133 Mini-Note's WLAN
         * is connected to north bridge's PCI Express Lane 0, but the
         * Lane 0 is also pin multiplexed with DVP0. What this means is
         * turning on DVP0 without probing the relevant strapping pin
         * to determine the connected panel interface type will lead to
         * the PCIe based WLAN to getting disabled by OpenChrome DDX
         * when X.Org Server starts.
         *     The current remedy for this will be to turn on DVP0
         * only when an 18-bit / 24-bit interface flat panel is 
         * connected. */
        /* 3C5.12[4] - DVP0D4 pin strapping
         *             0: Use DVP1 only for a flat panel.
         *             1: Use DVP0 and DVP1 for a flat panel */
        if (sr12 & 0x10) {
            /* Since an 18-bit / 24-bit flat panel is being used, actively
             * control DVP0. */
            viaDVP0PCIeSetIOPadSetting(pScrn, ioPadOn ? 0x03 : 0x00);
        } else {
            /* Keep DVP0 powered down. Otherwise, it will interfere with
             * PCIe Lane 0 through 7. */
            viaDVP0PCIeSetIOPadSetting(pScrn, 0x00);
        }

        /* Control DVP1 for a flat panel. */
        viaDVP1PCIeSetIOPadSetting(pScrn, ioPadOn ? 0x03 : 0x00);
        break;
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
        if ((((~(sr13 & 0x80)) && (~(sr13 & 0x40)))
             || ((sr13 & 0x80) && (~(sr13 & 0x40))))
           && (!pVia->isVIANanoBook)) {

            viaLVDS1SetIOPadSetting(pScrn, ioPadOn ? 0x03 : 0x00);
        }

        if (((~(sr13 & 0x80)) || (~(sr13 & 0x40))) 
           || (pVia->isVIANanoBook)) {

            viaLVDS2SetIOPadSetting(pScrn, ioPadOn ? 0x03 : 0x00);
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
                        "Exiting viaFPIOPadSetting.\n"));
}

static void
ViaLVDSSoftwarePowerFirstSequence(ScrnInfoPtr pScrn, Bool on)
{
    vgaHWPtr hwp = VGAHWPTR(pScrn);

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "ViaLVDSSoftwarePowerFirstSequence: %d\n", on));
    if (on) {

        /* Software control power sequence ON*/
        hwp->writeCrtc(hwp, 0x91, hwp->readCrtc(hwp, 0x91) & 0x7F);
        hwp->writeCrtc(hwp, 0x91, hwp->readCrtc(hwp, 0x91) | 0x01);
        usleep(TD0);

        /* VDD ON*/
        hwp->writeCrtc(hwp, 0x91, hwp->readCrtc(hwp, 0x91) | 0x10);
        usleep(TD1);

        /* DATA ON */
        hwp->writeCrtc(hwp, 0x91, hwp->readCrtc(hwp, 0x91) | 0x08);
        usleep(TD2);

        /* VEE ON (unused on vt3353)*/
        hwp->writeCrtc(hwp, 0x91, hwp->readCrtc(hwp, 0x91) | 0x04);
        usleep(TD3);

        /* Back-Light ON */
        hwp->writeCrtc(hwp, 0x91, hwp->readCrtc(hwp, 0x91) | 0x02);
    } else {
        /* Back-Light OFF */
        hwp->writeCrtc(hwp, 0x91, hwp->readCrtc(hwp, 0x91) & 0xFD);
        usleep(TD3);

        /* VEE OFF (unused on vt3353)*/
        hwp->writeCrtc(hwp, 0x91, hwp->readCrtc(hwp, 0x91) & 0xFB);
        usleep(TD2);

        /* DATA OFF */
        hwp->writeCrtc(hwp, 0x91, hwp->readCrtc(hwp, 0x91) & 0xF7);
        usleep(TD1);

        /* VDD OFF */
        hwp->writeCrtc(hwp, 0x91, hwp->readCrtc(hwp, 0x91) & 0xEF);
    }
}

static void
ViaLVDSSoftwarePowerSecondSequence(ScrnInfoPtr pScrn, Bool on)
{
    vgaHWPtr hwp = VGAHWPTR(pScrn);

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "ViaLVDSSoftwarePowerSecondSequence: %d\n", on));
    if (on) {
        /* Secondary power hardware power sequence enable 0:off 1: on */
        hwp->writeCrtc(hwp, 0xD4, hwp->readCrtc(hwp, 0xD4) & 0xFD);

        /* Software control power sequence ON */
        hwp->writeCrtc(hwp, 0xD3, hwp->readCrtc(hwp, 0xD3) | 0x01);
        usleep(TD0);

        /* VDD ON*/
        hwp->writeCrtc(hwp, 0xD3, hwp->readCrtc(hwp, 0xD3) | 0x10);
        usleep(TD1);

        /* DATA ON */
        hwp->writeCrtc(hwp, 0xD3, hwp->readCrtc(hwp, 0xD3) | 0x08);
        usleep(TD2);

        /* VEE ON (unused on vt3353)*/
        hwp->writeCrtc(hwp, 0xD3, hwp->readCrtc(hwp, 0xD3) | 0x04);
        usleep(TD3);

        /* Back-Light ON */
        hwp->writeCrtc(hwp, 0xD3, hwp->readCrtc(hwp, 0xD3) | 0x02);
    } else {
        /* Back-Light OFF */
        hwp->writeCrtc(hwp, 0xD3, hwp->readCrtc(hwp, 0xD3) & 0xFD);
        usleep(TD3);

        /* VEE OFF */
        hwp->writeCrtc(hwp, 0xD3, hwp->readCrtc(hwp, 0xD3) & 0xFB);
        /* Delay TD2 msec. */
        usleep(TD2);

        /* DATA OFF */
        hwp->writeCrtc(hwp, 0xD3, hwp->readCrtc(hwp, 0xD3) & 0xF7);
        /* Delay TD1 msec. */
        usleep(TD1);

        /* VDD OFF */
        hwp->writeCrtc(hwp, 0xD3, hwp->readCrtc(hwp, 0xD3) & 0xEF);
    }
}


static void
ViaLVDSHardwarePowerFirstSequence(ScrnInfoPtr pScrn, Bool on)
{
    vgaHWPtr hwp = VGAHWPTR(pScrn);

    if (on) {
        /* Use hardware control power sequence. */
        hwp->writeCrtc(hwp, 0x91, hwp->readCrtc(hwp, 0x91) & 0xFE);
        /* Turn on back light. */
        hwp->writeCrtc(hwp, 0x91, hwp->readCrtc(hwp, 0x91) & 0x3F);
        /* Turn on hardware power sequence. */
        hwp->writeCrtc(hwp, 0x6A, hwp->readCrtc(hwp, 0x6A) | 0x08);
    } else {
        /* Turn off power sequence. */
        hwp->writeCrtc(hwp, 0x6A, hwp->readCrtc(hwp, 0x6A) & 0xF7);
        usleep(1);
        /* Turn off back light. */
        hwp->writeCrtc(hwp, 0x91, 0xC0);
    }
}

static void
ViaLVDSHardwarePowerSecondSequence(ScrnInfoPtr pScrn, Bool on)
{
    vgaHWPtr hwp = VGAHWPTR(pScrn);

    if (on) {
        /* Use hardware control power sequence. */
        hwp->writeCrtc(hwp, 0xD3, hwp->readCrtc(hwp, 0xD3) & 0xFE);
        /* Turn on back light. */
        hwp->writeCrtc(hwp, 0xD3, hwp->readCrtc(hwp, 0xD3) & 0x3F);
        /* Turn on hardware power sequence. */
        hwp->writeCrtc(hwp, 0xD4, hwp->readCrtc(hwp, 0xD4) | 0x02);
    } else {
        /* Turn off power sequence. */
        hwp->writeCrtc(hwp, 0xD4, hwp->readCrtc(hwp, 0xD4) & 0xFD);
        usleep(1);
        /* Turn off back light. */
        hwp->writeCrtc(hwp, 0xD3, 0xC0);
    }
}

static void
ViaLVDSPowerChannel(ScrnInfoPtr pScrn, Bool on)
{
    vgaHWPtr hwp = VGAHWPTR(pScrn);
    CARD8 lvdsMask;

    if (on) {
        /* LVDS0: 0x7F, LVDS1: 0xBF */
        lvdsMask = 0x7F & 0xBF;
        hwp->writeCrtc(hwp, 0xD2, hwp->readCrtc(hwp, 0xD2) & lvdsMask);
    } else {
        /* LVDS0: 0x80, LVDS1: 0x40 */
        lvdsMask = 0x80 | 0x40;
        hwp->writeCrtc(hwp, 0xD2, hwp->readCrtc(hwp, 0xD2) | lvdsMask);
    }
}

static void
ViaLVDSPower(ScrnInfoPtr pScrn, Bool Power_On)
{
    vgaHWPtr hwp = VGAHWPTR(pScrn);
    VIAPtr pVia = VIAPTR(pScrn);
    CARD8 crd2;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Entered ViaLVDSPower.\n"));

    /*
     * VX800, CX700 have HW issue, so we'd better use SW power sequence
     * Fix Ticket #308
     */
    switch (pVia->Chipset) {
    case VIA_CX700:
    case VIA_VX800:

        /* Is the integrated TMDS transmitter (DVI) not in use? */
        crd2 = hwp->readCrtc(hwp, 0xD2);
        if (((pVia->Chipset == VIA_CX700)
                || (pVia->Chipset == VIA_VX800)
                || (pVia->Chipset == VIA_VX855)
                || (pVia->Chipset == VIA_VX900))
            && (!(crd2 & 0x10))) {
            ViaLVDSSoftwarePowerFirstSequence(pScrn, Power_On);
        }

        ViaLVDSSoftwarePowerSecondSequence(pScrn, Power_On);
        break;

    case VIA_VX855:
    case VIA_VX900:
        /* Is the integrated TMDS transmitter (DVI) not in use? */
        crd2 = hwp->readCrtc(hwp, 0xD2);
        if (((pVia->Chipset == VIA_CX700)
                || (pVia->Chipset == VIA_VX800)
                || (pVia->Chipset == VIA_VX855)
                || (pVia->Chipset == VIA_VX900))
            && (!(crd2 & 0x10))) {
            ViaLVDSHardwarePowerFirstSequence(pScrn, Power_On);
        }

        ViaLVDSHardwarePowerSecondSequence(pScrn, Power_On);
        break;
    default:
        ViaLVDSHardwarePowerFirstSequence(pScrn, Power_On);
        ViaLVDSHardwarePowerSecondSequence(pScrn, Power_On);
        break;
    }

    ViaLVDSPowerChannel(pScrn, Power_On);

    xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                "Integrated LVDS Flat Panel Power: %s\n",
                Power_On ? "On" : "Off");

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Exiting ViaLVDSPower.\n"));
}

static void
ViaLCDPowerSequence(vgaHWPtr hwp, VIALCDPowerSeqRec Sequence)
{
    int i;

    for (i = 0; i < Sequence.numEntry; i++) {
        ViaVgahwMask(hwp, 0x300 + Sequence.port[i], Sequence.offset[i],
                     0x301 + Sequence.port[i], Sequence.data[i],
                     Sequence.mask[i]);
        usleep(Sequence.delay[i]);
    }
}

static void
ViaLCDPower(xf86OutputPtr output, Bool Power_On)
{
    ViaPanelInfoPtr Panel = output->driver_private;
    ScrnInfoPtr pScrn = output->scrn;
    vgaHWPtr hwp = VGAHWPTR(pScrn);
    VIAPtr pVia = VIAPTR(pScrn);
    VIABIOSInfoPtr pBIOSInfo = pVia->pBIOSInfo;
    int i;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Entered ViaLCDPower.\n"));

    /* Enable LCD */
    if (Power_On)
        ViaCrtcMask(hwp, 0x6A, 0x08, 0x08);
    else
        ViaCrtcMask(hwp, 0x6A, 0x00, 0x08);

    if (pBIOSInfo->LCDPower)
        pBIOSInfo->LCDPower(pScrn, Power_On);

    /* Find Panel Size Index for PowerSeq Table */
    if (pVia->Chipset == VIA_CLE266) {
        if (Panel->NativeModeIndex != VIA_PANEL_INVALID) {
            for (i = 0; i < NumPowerOn; i++) {
                if (lcdTable[Panel->PanelIndex].powerSeq
                    == powerOn[i].powerSeq)
                    break;
            }
        } else
            i = 0;
    } else
        /* KM and K8M use PowerSeq Table index 2. */
        i = 2;

    usleep(1);
    if (Power_On)
        ViaLCDPowerSequence(hwp, powerOn[i]);
    else
        ViaLCDPowerSequence(hwp, powerOff[i]);
    usleep(1);

    xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                "Integrated LVDS Flat Panel Power: %s\n",
                Power_On ? "On" : "Off");

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Exiting ViaLCDPower.\n"));
}

/*
 * Try to interpret EDID ourselves.
 */
static Bool
ViaPanelGetSizeFromEDID(ScrnInfoPtr pScrn, xf86MonPtr pMon,
                        int *width, int *height)
{
    int i, max_hsize = 0, vsize = 0;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "VIAGetPanelSizeFromEDID\n"));

    /* !!! Why are we not checking VESA modes? */

    /* checking standard timings */
    for (i = 0; i < STD_TIMINGS; i++)
        if ((pMon->timings2[i].hsize > 256)
            && (pMon->timings2[i].hsize > max_hsize)) {
            max_hsize = pMon->timings2[i].hsize;
            vsize = pMon->timings2[i].vsize;
        }

    if (max_hsize != 0) {
        *width = max_hsize;
        *height = vsize;
        return TRUE;
    }

    /* checking detailed monitor section */

    /* !!! skip Ranges and standard timings */

    /* check detailed timings */
    for (i = 0; i < DET_TIMINGS; i++)
        if (pMon->det_mon[i].type == DT) {
            struct detailed_timings timing = pMon->det_mon[i].section.d_timings;

            /* ignore v_active for now */
            if ((timing.clock > 15000000) && (timing.h_active > max_hsize)) {
                max_hsize = timing.h_active;
                vsize = timing.v_active;
            }
        }

    if (max_hsize != 0) {
        *width = max_hsize;
        *height = vsize;
        return TRUE;
    }
    return FALSE;
}

static Bool
ViaPanelGetSizeFromDDCv1(xf86OutputPtr output, int *width, int *height)
{
    ScrnInfoPtr pScrn = output->scrn;
    VIAPtr pVia = VIAPTR(pScrn);
    xf86MonPtr pMon;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Entered VIAGetPanelSizeFromDDCv1.\n"));

    if (!pVia->pI2CBus2) {
        xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
                    "I2C Bus 2 does not exist.\n");
        DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                            "Exiting VIAGetPanelSizeFromDDCv1.\n"));
        return FALSE;
    }

    if (!xf86I2CProbeAddress(pVia->pI2CBus2, 0xA0)) {
        xf86DrvMsg(pScrn->scrnIndex, X_PROBED,
                    "I2C device on I2C Bus 2 does not support EDID.\n");
        DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                            "Exiting VIAGetPanelSizeFromDDCv1.\n"));
        return FALSE;
    }

    /* Probe I2C Bus 2 to see if a flat panel is connected. */
    xf86DrvMsg(pScrn->scrnIndex, X_PROBED,
                "Probing for a flat panel on I2C Bus 2.\n");
    pMon = xf86OutputGetEDID(output, pVia->pI2CBus2);
    if (pMon && DIGITAL(pMon->features.input_type)) {
        xf86OutputSetEDID(output, pMon);
        xf86DrvMsg(pScrn->scrnIndex, X_PROBED,
                    "Detected a flat panel on I2C Bus 2.\n");
    } else {
        xf86DrvMsg(pScrn->scrnIndex, X_PROBED,
                    "Did not detect a flat panel on I2C Bus 2.\n");
        DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                            "Exiting VIAGetPanelSizeFromDDCv1.\n"));
        return FALSE;

    }

    if (!ViaPanelGetSizeFromEDID(pScrn, pMon, width, height)) {
        xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
                    "Unable to obtain panel size from EDID information.\n");
        DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                            "Exiting VIAGetPanelSizeFromDDCv1.\n"));
        return FALSE;
    }

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "VIAGetPanelSizeFromDDCv1: (%d X %d)\n",
                        *width, *height));
    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Exiting VIAGetPanelSizeFromDDCv1.\n"));
    return TRUE;
}

/*
 * Gets the native panel resolution from scratch pad registers.
 */
static void
viaLVDSGetFPInfoFromScratchPad(xf86OutputPtr output)
{
    ScrnInfoPtr pScrn = output->scrn;
    vgaHWPtr hwp = VGAHWPTR(pScrn);
    ViaPanelInfoPtr panel = output->driver_private;
    CARD8 index;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                     "Entered viaLVDSGetFPInfoFromScratchPad.\n"));

    index = hwp->readCrtc(hwp, 0x3F) & 0x0F;

    panel->NativeModeIndex = index;
    panel->NativeWidth = ViaPanelNativeModes[index].Width;
    panel->NativeHeight = ViaPanelNativeModes[index].Height;
    panel->useDualEdge = ViaPanelNativeModes[index].useDualEdge;
    panel->useDithering = ViaPanelNativeModes[index].useDithering;

    xf86DrvMsg(pScrn->scrnIndex, X_INFO,
               "VIA Technologies VGA BIOS Scratch Pad Register "
               "Flat Panel Index: %d\n",
               panel->NativeModeIndex);
    xf86DrvMsg(pScrn->scrnIndex, X_INFO,
               "Flat Panel Native Resolution: %dx%d\n",
               panel->NativeWidth, panel->NativeHeight);
    xf86DrvMsg(pScrn->scrnIndex, X_INFO,
               "Flat Panel Dual Edge Transfer: %s\n",
               panel->useDualEdge ? "On" : "Off");
    xf86DrvMsg(pScrn->scrnIndex, X_INFO,
               "Flat Panel Output Color Dithering: %s\n",
               panel->useDithering ? "On (18 bit)" : "Off (24 bit)");

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                     "Exiting viaLVDSGetFPInfoFromScratchPad.\n"));
}

static void
ViaPanelCenterMode(DisplayModePtr mode, DisplayModePtr adjusted_mode)
{
    int panelHSyncTime = adjusted_mode->HSyncEnd - adjusted_mode->HSyncStart;
    int panelVSyncTime = adjusted_mode->VSyncEnd - adjusted_mode->VSyncStart;
    int panelHBlankStart = adjusted_mode->HDisplay;
    int panelVBlankStart = adjusted_mode->VDisplay;
    int hBorder = (adjusted_mode->HDisplay - mode->HDisplay)/2;
    int vBorder = (adjusted_mode->VDisplay - mode->VDisplay)/2;
    int newHBlankStart = hBorder + mode->HDisplay;
    int newVBlankStart = vBorder + mode->VDisplay;

    adjusted_mode->HDisplay = mode->HDisplay;
    adjusted_mode->HSyncStart = (adjusted_mode->HSyncStart - panelHBlankStart) + newHBlankStart;
    adjusted_mode->HSyncEnd = adjusted_mode->HSyncStart + panelHSyncTime;
    adjusted_mode->VDisplay = mode->VDisplay;
    adjusted_mode->VSyncStart = (adjusted_mode->VSyncStart - panelVBlankStart) + newVBlankStart;
    adjusted_mode->VSyncEnd = adjusted_mode->VSyncStart + panelVSyncTime;
    /* Adjust Crtc H and V */
    adjusted_mode->CrtcHDisplay = adjusted_mode->HDisplay;
    adjusted_mode->CrtcHBlankStart = newHBlankStart;
    adjusted_mode->CrtcHBlankEnd = adjusted_mode->CrtcHTotal - hBorder;
    adjusted_mode->CrtcHSyncStart = adjusted_mode->HSyncStart;
    adjusted_mode->CrtcHSyncEnd = adjusted_mode->HSyncEnd;
    adjusted_mode->CrtcVDisplay = adjusted_mode->VDisplay;
    adjusted_mode->CrtcVBlankStart = newVBlankStart;
    adjusted_mode->CrtcVBlankEnd = adjusted_mode->CrtcVTotal - vBorder;
    adjusted_mode->CrtcVSyncStart = adjusted_mode->VSyncStart;
    adjusted_mode->CrtcVSyncEnd = adjusted_mode->VSyncEnd;
}

static void
ViaPanelScale(ScrnInfoPtr pScrn, int resWidth, int resHeight,
              int panelWidth, int panelHeight)
{
    VIAPtr pVia = VIAPTR(pScrn);
    vgaHWPtr hwp = VGAHWPTR(pScrn);
    int horScalingFactor = 0;
    int verScalingFactor = 0;
    CARD8 cra2 = 0;
    CARD8 cr77 = 0;
    CARD8 cr78 = 0;
    CARD8 cr79 = 0;
    CARD8 cr9f = 0;
    Bool scaling = FALSE;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                    "ViaPanelScale: %d,%d -> %d,%d\n",
                    resWidth, resHeight, panelWidth, panelHeight));

    if (resWidth < panelWidth) {
        /* Load Horizontal Scaling Factor */
        if (pVia->Chipset != VIA_CLE266 && pVia->Chipset != VIA_KM400) {
            horScalingFactor = ((resWidth - 1) * 4096) / (panelWidth - 1);

            /* Horizontal scaling enabled */
            cra2 = 0xC0;
            cr9f = horScalingFactor & 0x0003;   /* HSCaleFactor[1:0] at CR9F[1:0] */
        } else {
            /* TODO: Need testing */
            horScalingFactor = ((resWidth - 1) * 1024) / (panelWidth - 1);
        }

        cr77 = (horScalingFactor & 0x03FC) >> 2;   /* HSCaleFactor[9:2] at CR77[7:0] */
        cr79 = (horScalingFactor & 0x0C00) >> 10;  /* HSCaleFactor[11:10] at CR79[5:4] */
        cr79 <<= 4;
        scaling = TRUE;
    }

    if (resHeight < panelHeight) {
        /* Load Vertical Scaling Factor */
        if (pVia->Chipset != VIA_CLE266 && pVia->Chipset != VIA_KM400) {
            verScalingFactor = ((resHeight - 1) * 2048) / (panelHeight - 1);

            /* Vertical scaling enabled */
            cra2 |= 0x08;
            cr79 |= ((verScalingFactor & 0x0001) << 3); /* VSCaleFactor[0] at CR79[3] */
        } else {
            /* TODO: Need testing */
            verScalingFactor = ((resHeight - 1) * 1024) / (panelHeight - 1);
        }

        cr78 |= (verScalingFactor & 0x01FE) >> 1;           /* VSCaleFactor[8:1] at CR78[7:0] */

        cr79 |= ((verScalingFactor & 0x0600) >> 9) << 6;    /* VSCaleFactor[10:9] at CR79[7:6] */
        scaling = TRUE;
    }

    if (scaling) {
        DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                "Scaling factor: horizontal %d (0x%x), vertical %d (0x%x)\n",
        horScalingFactor, horScalingFactor,
        verScalingFactor, verScalingFactor));

        ViaCrtcMask(hwp, 0x77, cr77, 0xFF);
        ViaCrtcMask(hwp, 0x78, cr78, 0xFF);
        ViaCrtcMask(hwp, 0x79, cr79, 0xF8);

        if (pVia->Chipset != VIA_CLE266 && pVia->Chipset != VIA_KM400) {
            ViaCrtcMask(hwp, 0x9F, cr9f, 0x03);
        }
        ViaCrtcMask(hwp, 0x79, 0x03, 0x03);
    } else {
        /*  Disable panel scale */
        ViaCrtcMask(hwp, 0x79, 0x00, 0x01);
    }

    if (pVia->Chipset != VIA_CLE266 && pVia->Chipset != VIA_KM400) {
        ViaCrtcMask(hwp, 0xA2, cra2, 0xC8);
    }

    /* Horizontal scaling selection: interpolation */
    // ViaCrtcMask(hwp, 0x79, 0x02, 0x02);
    // else
    // ViaCrtcMask(hwp, 0x79, 0x00, 0x02);
    /* Horizontal scaling factor selection original / linear */
    //ViaCrtcMask(hwp, 0xA2, 0x40, 0x40);
}

static void
ViaPanelScaleDisable(ScrnInfoPtr pScrn)
{
    VIAPtr pVia = VIAPTR(pScrn);
    vgaHWPtr hwp = VGAHWPTR(pScrn);

    ViaCrtcMask(hwp, 0x79, 0x00, 0x01);
    /* Disable VX900 down scaling */
    if (pVia->Chipset == VIA_VX900)
        ViaCrtcMask(hwp, 0x89, 0x00, 0x01);
    if (pVia->Chipset != VIA_CLE266 && pVia->Chipset != VIA_KM400)
        ViaCrtcMask(hwp, 0xA2, 0x00, 0xC8);
}

static void
via_lvds_create_resources(xf86OutputPtr output)
{
}

static void
via_lvds_dpms(xf86OutputPtr output, int mode)
{
    ScrnInfoPtr pScrn = output->scrn;
    VIAPtr pVia = VIAPTR(pScrn);

    switch (mode) {
    case DPMSModeOn:
        switch (pVia->Chipset) {
        case VIA_PM800:
        case VIA_P4M800PRO:
        case VIA_P4M890:
        case VIA_K8M890:
        case VIA_P4M900:
        case VIA_CX700:
        case VIA_VX800:
        case VIA_VX855:
        case VIA_VX900:
            ViaLVDSPower(pScrn, TRUE);
            break;
        default:
            ViaLCDPower(output, TRUE);
            break;
        }

        viaFPIOPadSetting(pScrn, TRUE);
        break;

    case DPMSModeStandby:
    case DPMSModeSuspend:
    case DPMSModeOff:
        switch (pVia->Chipset) {
        case VIA_PM800:
        case VIA_P4M800PRO:
        case VIA_P4M890:
        case VIA_K8M890:
        case VIA_P4M900:
        case VIA_CX700:
        case VIA_VX800:
        case VIA_VX855:
        case VIA_VX900:
            ViaLVDSPower(pScrn, FALSE);
            break;
        default:
            ViaLCDPower(output, FALSE);
            break;
        }

        viaFPIOPadSetting(pScrn, FALSE);
        break;
    }
}

static void
via_lvds_save(xf86OutputPtr output)
{
}

static void
via_lvds_restore(xf86OutputPtr output)
{
    ViaLCDPower(output, TRUE);
}

static int
via_lvds_mode_valid(xf86OutputPtr output, DisplayModePtr pMode)
{
    ScrnInfoPtr pScrn = output->scrn;
    ViaPanelInfoPtr Panel = output->driver_private;

    if (Panel->NativeWidth < pMode->HDisplay ||
        Panel->NativeHeight < pMode->VDisplay)
        return MODE_PANEL;

    if (!Panel->Scale && Panel->NativeHeight != pMode->VDisplay &&
         Panel->NativeWidth != pMode->HDisplay)
        return MODE_PANEL;

    if (!ViaModeDotClockTranslate(pScrn, pMode))
        return MODE_NOCLOCK;

    return MODE_OK;
}

static Bool
via_lvds_mode_fixup(xf86OutputPtr output, DisplayModePtr mode,
                    DisplayModePtr adjusted_mode)
{
    ViaPanelInfoPtr Panel = output->driver_private;

    xf86SetModeCrtc(adjusted_mode, 0);
    if (!Panel->Center && (mode->HDisplay < Panel->NativeWidth ||
        mode->VDisplay < Panel->NativeHeight)) {
        Panel->Scale = TRUE;
    } else {
        Panel->Scale = FALSE;
        ViaPanelCenterMode(mode, adjusted_mode);
    }
    return TRUE;
}

static void
via_lvds_prepare(xf86OutputPtr output)
{
    ScrnInfoPtr pScrn = output->scrn;

    via_lvds_dpms(output, DPMSModeOff);
    viaFPIOPadSetting(pScrn, FALSE);
}

static void
via_lvds_commit(xf86OutputPtr output)
{
    ScrnInfoPtr pScrn = output->scrn;

    via_lvds_dpms(output, DPMSModeOn);
    viaFPIOPadSetting(pScrn, TRUE);
}

static void
via_lvds_mode_set(xf86OutputPtr output, DisplayModePtr mode,
                    DisplayModePtr adjusted_mode)
{
    ViaPanelInfoPtr Panel = output->driver_private;
    ScrnInfoPtr pScrn = output->scrn;
    drmmode_crtc_private_ptr iga = output->crtc->driver_private;
    VIAPtr pVia = VIAPTR(pScrn);

    if (output->crtc) {
        if (Panel->Scale) {
            ViaPanelScale(pScrn, mode->HDisplay, mode->VDisplay,
                            Panel->NativeWidth,
                            Panel->NativeHeight);
        } else {
            ViaPanelScaleDisable(pScrn);
        }

        switch (pVia->Chipset) {
        case VIA_P4M900:
            viaDFPLowSetDelayTap(pScrn, 0x08);
            break;
        case VIA_CX700:
            viaLVDS2SetDelayTap(pScrn, 0x01);
            break;
        default:
            break;
        }


        switch (pVia->Chipset) {
        case VIA_KM400:
        case VIA_K8M800:
        case VIA_PM800:
        case VIA_P4M800PRO:
            viaDFPLowSetDisplaySource(pScrn, iga->index ? 0x01 : 0x00);
            viaDFPHighSetDisplaySource(pScrn, iga->index ? 0x01 : 0x00);
            break;
        case VIA_P4M890:
        case VIA_K8M890:
        case VIA_P4M900:
            viaDFPLowSetDisplaySource(pScrn, iga->index ? 0x01 : 0x00);
            viaDVP1SetDisplaySource(pScrn, iga->index ? 0x01 : 0x00);
            break;
        case VIA_CX700:
        case VIA_VX800:
        case VIA_VX855:
        case VIA_VX900:
            viaLVDS2SetDisplaySource(pScrn, iga->index ? 0x01 : 0x00);

            /* Set LVDS2 output color dithering. */
            viaLVDS2SetDithering(pScrn, Panel->useDithering ? TRUE : FALSE);

            /* Set LVDS2 output format to sequential mode. */
            viaLVDS2SetOutputFormat(pScrn, 0x01);

            /* Set LVDS2 output to OPENLDI mode. */
            viaLVDS2SetFormat(pScrn, 0x01);
            break;
        default:
            break;
        }
    }
}

static xf86OutputStatus
via_lvds_detect(xf86OutputPtr output)
{
    xf86OutputStatus status = XF86OutputStatusDisconnected;
    ScrnInfoPtr pScrn = output->scrn;
    VIAPtr pVia = VIAPTR(pScrn);
    ViaPanelInfoPtr panel = output->driver_private;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Entered via_lvds_detect.\n"));

    /* Hardcode panel size for the OLPC XO-1.5. */
    if (pVia->IsOLPCXO15) {
        xf86DrvMsg(pScrn->scrnIndex, X_PROBED,
                    "Setting up OLPC XO-1.5 flat panel.\n");
        panel->NativeWidth = 1200;
        panel->NativeHeight = 900;
        status = XF86OutputStatusConnected;
        goto exit;
    }

    /* For now, FP detection code will not scan the I2C bus
     * in order to obtain EDID since it is often used by DVI
     * as well. Hence, reading off the CRTC scratch pad register
     * supplied by the VGA BIOS is the only method available
     * to figure out the FP native screen resolution. */
    viaLVDSGetFPInfoFromScratchPad(output);
    status = XF86OutputStatusConnected;

exit:
    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Exiting via_lvds_detect.\n"));
    return status;
}

static DisplayModePtr
via_lvds_get_modes(xf86OutputPtr output)
{
    ViaPanelInfoPtr pPanel = output->driver_private;
    ScrnInfoPtr pScrn = output->scrn;
    DisplayModePtr pDisplay_Mode = NULL;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Entered via_lvds_get_modes.\n"));

    if (output->status == XF86OutputStatusConnected) {
        if (!output->MonInfo) {
            /*
             * Generates a display mode for the native panel resolution,
             * using CVT.
             */
            if (pPanel->NativeWidth && pPanel->NativeHeight) {
                VIAPtr pVia = VIAPTR(pScrn);

                if (pVia->IsOLPCXO15) {
                    pDisplay_Mode = xf86DuplicateMode(&OLPCMode);
                } else {
                    pDisplay_Mode = xf86CVTMode(pPanel->NativeWidth, pPanel->NativeHeight,
                                    60.0f, FALSE, FALSE);
                }

                if (pDisplay_Mode) {
                    pDisplay_Mode->CrtcHDisplay = pDisplay_Mode->HDisplay;
                    pDisplay_Mode->CrtcHSyncStart = pDisplay_Mode->HSyncStart;
                    pDisplay_Mode->CrtcHSyncEnd = pDisplay_Mode->HSyncEnd;
                    pDisplay_Mode->CrtcHTotal = pDisplay_Mode->HTotal;
                    pDisplay_Mode->CrtcHSkew = pDisplay_Mode->HSkew;
                    pDisplay_Mode->CrtcVDisplay = pDisplay_Mode->VDisplay;
                    pDisplay_Mode->CrtcVSyncStart = pDisplay_Mode->VSyncStart;
                    pDisplay_Mode->CrtcVSyncEnd = pDisplay_Mode->VSyncEnd;
                    pDisplay_Mode->CrtcVTotal = pDisplay_Mode->VTotal;

                    pDisplay_Mode->CrtcVBlankStart = min(pDisplay_Mode->CrtcVSyncStart, pDisplay_Mode->CrtcVDisplay);
                    pDisplay_Mode->CrtcVBlankEnd = max(pDisplay_Mode->CrtcVSyncEnd, pDisplay_Mode->CrtcVTotal);
                    pDisplay_Mode->CrtcHBlankStart = min(pDisplay_Mode->CrtcHSyncStart, pDisplay_Mode->CrtcHDisplay);
                    pDisplay_Mode->CrtcHBlankEnd = max(pDisplay_Mode->CrtcHSyncEnd, pDisplay_Mode->CrtcHTotal);
                    pDisplay_Mode->type = M_T_DRIVER | M_T_PREFERRED;
                } else {
                    xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
                                "Out of memory. Size: %zu bytes\n", sizeof(DisplayModeRec));
                }
            } else {
                xf86DrvMsg(pScrn->scrnIndex, X_WARNING,
                            "Invalid Flat Panel Screen Resolution: "
                            "%dx%d\n",
                            pPanel->NativeWidth, pPanel->NativeHeight);
            }
        } else {
            pDisplay_Mode = xf86OutputGetEDIDModes(output);
        }
    }
    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Exiting via_lvds_get_modes.\n"));
    return pDisplay_Mode;
}

#ifdef RANDR_12_INTERFACE
static Bool
via_lvds_set_property(xf86OutputPtr output, Atom property,
                        RRPropertyValuePtr value)
{
    return FALSE;
}

static Bool
via_lvds_get_property(xf86OutputPtr output, Atom property)
{
    return FALSE;
}
#endif

static void
via_lvds_destroy(xf86OutputPtr output)
{
    if (output->driver_private)
        free(output->driver_private);
    output->driver_private = NULL;
}

static const xf86OutputFuncsRec via_lvds_funcs = {
    .create_resources   = via_lvds_create_resources,
    .dpms               = via_lvds_dpms,
    .save               = via_lvds_save,
    .restore            = via_lvds_restore,
    .mode_valid         = via_lvds_mode_valid,
    .mode_fixup         = via_lvds_mode_fixup,
    .prepare            = via_lvds_prepare,
    .commit             = via_lvds_commit,
    .mode_set           = via_lvds_mode_set,
    .detect             = via_lvds_detect,
    .get_modes          = via_lvds_get_modes,
#ifdef RANDR_12_INTERFACE
    .set_property       = via_lvds_set_property,
#endif
#ifdef RANDR_13_INTERFACE
    .get_property       = via_lvds_get_property,
#endif
    .destroy            = via_lvds_destroy
};


void
via_lvds_init(ScrnInfoPtr pScrn)
{
    ViaPanelInfoPtr Panel = (ViaPanelInfoPtr) xnfcalloc(sizeof(ViaPanelInfoRec), 1);
    OptionInfoPtr  Options = xnfalloc(sizeof(ViaPanelOptions));
    MessageType from = X_DEFAULT;
    VIAPtr pVia = VIAPTR(pScrn);
    xf86OutputPtr output = NULL;
    vgaHWPtr hwp = VGAHWPTR(pScrn);
    CARD8 cr3b = 0x00;
    CARD8 cr3b_mask = 0x00;
    char outputNameBuffer[32];

    if (!Panel)
        return;

    /* Apparently this is the way VIA Technologies passes */
    /* the presence of a flat panel to the device driver */
    /* via BIOS setup. */
    if (pVia->Chipset == VIA_CLE266) {
        cr3b_mask = 0x08;
    } else {
        cr3b_mask = 0x02;
    }            

    cr3b = hwp->readCrtc(hwp, 0x3B) & cr3b_mask;

    if (!cr3b) {
        return;
    }

    memcpy(Options, ViaPanelOptions, sizeof(ViaPanelOptions));
    xf86ProcessOptions(pScrn->scrnIndex, pScrn->options, Options);

    Panel->NativeModeIndex = VIA_PANEL_INVALID;

    /* LCD Center/Expend Option */
    Panel->Center = FALSE;
    from = xf86GetOptValBool(Options, OPTION_CENTER, &Panel->Center)
            ? X_CONFIG : X_DEFAULT;
    xf86DrvMsg(pScrn->scrnIndex, from, "LVDS-0 : DVI Center is %s.\n",
               Panel->Center ? "enabled" : "disabled");

    /* The code to dynamically designate a particular FP (i.e., FP-1,
     * FP-2, etc.) for xrandr was borrowed from xf86-video-r128 DDX. */
    sprintf(outputNameBuffer, "FP-%d", (pVia->numberFP + 1));
    output = xf86OutputCreate(pScrn, &via_lvds_funcs, outputNameBuffer);

    if (output)  {
        output->driver_private = Panel;

        /* While there are two (2) display controllers registered with the
         * X.Org Server, it is often desirable to fix FP (Flat Panel) to
         * IGA2 since only IGA2 contains panel resolution scaling
         * functionality. IGA1 does not have this. */
        output->possible_crtcs = 1 << 1;

        output->possible_clones = 0;
        output->interlaceAllowed = FALSE;
        output->doubleScanAllowed = FALSE;

        /* Increment the number of FP connectors. */
        pVia->numberFP++;

        if (pVia->IsOLPCXO15) {
            output->mm_height = 152;
            output->mm_width = 114;
        }
    } else {
        free(Panel);
    }
}

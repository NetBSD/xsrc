/*
 * Copyright 2007 The Openchrome Project [openchrome.org]
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
    OPTION_BUSWIDTH,
    OPTION_CENTER,
    OPTION_FORCEPANEL,
    OPTION_PANELSIZE
};

static OptionInfoRec ViaPanelOptions[] =
{
    {OPTION_BUSWIDTH,   "BusWidth",     OPTV_ANYSTR,    {0},    FALSE},
    {OPTION_CENTER,     "Center",       OPTV_BOOLEAN,   {0},    FALSE},
    {OPTION_FORCEPANEL, "ForcePanel",   OPTV_BOOLEAN,   {0},    FALSE}, /* last resort */
    {OPTION_PANELSIZE,  "PanelSize",    OPTV_ANYSTR,    {0},    FALSE},
    {-1,                NULL,           OPTV_NONE,      {0},    FALSE}
};

static ViaPanelModeRec ViaPanelNativeModes[] = {
    {640, 480},
    {800, 600},
    {1024, 768},
    {1280, 768},
    {1280, 1024},
    {1400, 1050},
    {1600, 1200},   /* 0x6 */
    {1280, 800},    /* 0x7 Resolution 1280x800 (Samsung NC20) */
    {800, 480},     /* 0x8 For Quanta 800x480 */
    {1024, 600},    /* 0x9 Resolution 1024x600 (for HP 2133) */
    {1368, 768},    /* 0xA Resolution 1366x768 */
    {1920, 1080},
    {1920, 1200},
    {1280, 1024},   /* 0xD */
    {1440, 900},    /* 0xE */
    {1280, 720},    /* 0xF 480x640 */
    {1200, 900},   /* 0x10 For OLPC 1.5 */
    {1360, 768},   /* 0x11 Resolution 1360X768 */
    {1024, 768},   /* 0x12 Resolution 1024x768 */
    {800, 480}     /* 0x13 General 8x4 panel use this setting */
};

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
ViaLVDSDFPPower(ScrnInfoPtr pScrn, Bool on)
{
    vgaHWPtr hwp = VGAHWPTR(pScrn);
    VIAPtr pVia = VIAPTR(pScrn);

    /* Switch DFP High/Low pads on or off for channels active at EnterVT(). */
    ViaSeqMask(hwp, 0x2A, on ? pVia->SavedReg.SR2A : 0, 0x0F);
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
ViaLVDSPower(ScrnInfoPtr pScrn, Bool on)
{
    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "ViaLVDSPower %d\n", on));
    VIAPtr pVia = VIAPTR(pScrn);

    /*
     * VX800, CX700 have HW issue, so we'd better use SW power sequence
     * Fix Ticket #308
     */
    switch (pVia->Chipset) {
    case VIA_VX800:
    case VIA_CX700:
        ViaLVDSSoftwarePowerFirstSequence(pScrn, on);
        ViaLVDSSoftwarePowerSecondSequence(pScrn, on);
        break;
    default:
        ViaLVDSHardwarePowerFirstSequence(pScrn, on);
        ViaLVDSHardwarePowerSecondSequence(pScrn, on);
        break;
    }

    ViaLVDSDFPPower(pScrn, on);
    ViaLVDSPowerChannel(pScrn, on);
}

static void
via_lvds_create_resources(xf86OutputPtr output)
{
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
ViaLCDPower(xf86OutputPtr output, Bool On)
{
    ViaPanelInfoPtr Panel = output->driver_private;
    ScrnInfoPtr pScrn = output->scrn;
    vgaHWPtr hwp = VGAHWPTR(pScrn);
    VIAPtr pVia = VIAPTR(pScrn);
    VIABIOSInfoPtr pBIOSInfo = pVia->pBIOSInfo;
    int i;

#ifdef HAVE_DEBUG
    if (On)
        xf86DrvMsg(pScrn->scrnIndex, X_INFO, "ViaLCDPower: On.\n");
    else
        xf86DrvMsg(pScrn->scrnIndex, X_INFO, "ViaLCDPower: Off.\n");
#endif

    /* Enable LCD */
    if (On)
        ViaCrtcMask(hwp, 0x6A, 0x08, 0x08);
    else
        ViaCrtcMask(hwp, 0x6A, 0x00, 0x08);

    if (pBIOSInfo->LCDPower)
        pBIOSInfo->LCDPower(pScrn, On);

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
    if (On)
        ViaLCDPowerSequence(hwp, powerOn[i]);
    else
        ViaLCDPowerSequence(hwp, powerOff[i]);
    usleep(1);
}

static void
via_lvds_dpms(xf86OutputPtr output, int mode)
{
    ScrnInfoPtr pScrn = output->scrn;
    VIAPtr pVia = VIAPTR(pScrn);

    if (pVia->pVbe) {
        ViaVbePanelPower(pVia->pVbe, (mode == DPMSModeOn));
    } else {
        switch (mode) {
        case DPMSModeOn:
            switch (pVia->Chipset) {
            case VIA_P4M900:
            case VIA_CX700:
            case VIA_VX800:
            case VIA_VX855:
            case VIA_VX900:
                ViaLVDSPower(pScrn, TRUE);
                break;
            }
            ViaLCDPower(output, TRUE);
            break;

        case DPMSModeStandby:
        case DPMSModeSuspend:
        case DPMSModeOff:
            switch (pVia->Chipset) {
            case VIA_P4M900:
            case VIA_CX700:
            case VIA_VX800:
            case VIA_VX855:
            case VIA_VX900:
                ViaLVDSPower(pScrn, FALSE);
                break;
            }
            ViaLCDPower(output, FALSE);
            break;
        }
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

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "VIAGetPanelSizeFromDDCv1\n"));

    if (!(pVia->I2CDevices & VIA_I2C_BUS2))
        return FALSE;

    if (!xf86I2CProbeAddress(pVia->pI2CBus2, 0xA0))
        return FALSE;

    pMon = xf86DoEEDID(XF86_SCRN_ARG(pScrn), pVia->pI2CBus2, TRUE);
    if (!pMon)
        return FALSE;

    xf86OutputSetEDID(output, pMon);

    if (!ViaPanelGetSizeFromEDID(pScrn, pMon, width, height)) {
        xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
                   "Unable to read PanelSize from EDID information\n");
        return FALSE;
    }

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                     "VIAGetPanelSizeFromDDCv1: (%dx%d)\n", *width, *height));
    return TRUE;
}

/* Currently only used by Legacy Mode Setting */
static Bool
ViaPanelGetSizeFromDDCv2(ScrnInfoPtr pScrn, int *width)
{
    VIAPtr pVia = VIAPTR(pScrn);
    CARD8 W_Buffer[1];
    CARD8 R_Buffer[4];
    I2CDevPtr dev;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "VIAGetPanelSizeFromDDCv2\n"));

    if (!xf86I2CProbeAddress(pVia->pI2CBus2, 0xA2))
        return FALSE;

    dev = xf86CreateI2CDevRec();
    if (!dev)
        return FALSE;

    dev->DevName = "EDID2";
    dev->SlaveAddr = 0xA2;
    dev->ByteTimeout = 2200;  /* VESA DDC spec 3 p. 43 (+10 %) */
    dev->StartTimeout = 550;
    dev->BitTimeout = 40;
    dev->ByteTimeout = 40;
    dev->AcknTimeout = 40;
    dev->pI2CBus = pVia->pI2CBus2;

    if (!xf86I2CDevInit(dev)) {
        xf86DestroyI2CDevRec(dev, TRUE);
        return FALSE;
    }

    xf86I2CReadByte(dev, 0x00, R_Buffer);
    if (R_Buffer[0] != 0x20) {
        xf86DestroyI2CDevRec(dev, TRUE);
        return FALSE;
    }

    /* Found EDID2 Table */
    W_Buffer[0] = 0x76;
    xf86I2CWriteRead(dev, W_Buffer, 1, R_Buffer, 2);
    xf86DestroyI2CDevRec(dev, TRUE);

    *width = R_Buffer[0] | (R_Buffer[1] << 8);

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                     "VIAGetPanelSizeFromDDCv2: %d\n", *width));
    return TRUE;
}

static Bool
ViaGetResolutionIndex(ScrnInfoPtr pScrn, ViaPanelInfoPtr Panel,
                      DisplayModePtr mode)
{
    int i;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                     "ViaGetResolutionIndex: Looking for %dx%d\n",
                     mode->CrtcHDisplay, mode->CrtcVDisplay));
    for (i = 0; ViaResolutionTable[i].Index != VIA_RES_INVALID; i++) {
        if ((ViaResolutionTable[i].X == mode->CrtcHDisplay)
            && (ViaResolutionTable[i].Y == mode->CrtcVDisplay)) {
            Panel->ResolutionIndex = ViaResolutionTable[i].Index;
            DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "ViaGetResolutionIndex:"
                             " %d\n", Panel->ResolutionIndex));
            return TRUE;
        }
    }

    Panel->ResolutionIndex = VIA_RES_INVALID;
    return FALSE;
}

static int
ViaGetVesaMode(ScrnInfoPtr pScrn, DisplayModePtr mode)
{
    int i;

    for (i = 0; ViaVesaModes[i].Width; i++)
        if ((ViaVesaModes[i].Width == mode->CrtcHDisplay)
            && (ViaVesaModes[i].Height == mode->CrtcVDisplay)) {
            switch (pScrn->bitsPerPixel) {
                case 8:
                    return ViaVesaModes[i].mode_8b;
                case 16:
                    return ViaVesaModes[i].mode_16b;
                case 24:
                case 32:
                    return ViaVesaModes[i].mode_32b;
                default:
                    return 0xFFFF;
            }
        }
    return 0xFFFF;
}

/*
 * Gets the native panel resolution from scratch pad registers.
 */
static void
ViaPanelGetNativeModeFromScratchPad(xf86OutputPtr output)
{
    ViaPanelInfoPtr panel = output->driver_private;
    ScrnInfoPtr pScrn = output->scrn;
    vgaHWPtr hwp = VGAHWPTR(pScrn);
    CARD8 index;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                     "ViaPanelGetNativeModeFromScratchPad\n"));

    index = hwp->readCrtc(hwp, 0x3F) & 0x0F;

    panel->NativeModeIndex = index;
    panel->NativeWidth = ViaPanelNativeModes[index].Width;
    panel->NativeHeight = ViaPanelNativeModes[index].Height;
    xf86DrvMsg(pScrn->scrnIndex, X_INFO,
               "Native Panel Resolution is %dx%d\n",
               panel->NativeWidth, panel->NativeHeight);
}

/* Used only for Legacy Mode Setting */
static xf86OutputStatus
VIAGetPanelSize(xf86OutputPtr output)
{
    xf86OutputStatus status = XF86OutputStatusDisconnected;
    ViaPanelInfoPtr Panel = output->driver_private;
    ScrnInfoPtr pScrn = output->scrn;
    char *PanelSizeString[7] = { "640x480", "800x480", "800x600", "1024x768", "1280x768"
                                 "1280x1024", "1400x1050", "1600x1200" };
    int width = 0;
    int height = 0;
    Bool ret;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "VIAGetPanelSize (UseLegacyModeSwitch)\n"));

    ret = ViaPanelGetSizeFromDDCv1(output, &width, &height);
    if (!ret)
        ret = ViaPanelGetSizeFromDDCv2(pScrn, &width);

    if (ret) {
        DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "EDID returned resolution %d x %d \n", width, height));
        switch (width) {
        case 640:
            Panel->NativeModeIndex = VIA_PANEL6X4;
            break;
        case 800:
            if (height == 480)
                Panel->NativeModeIndex = VIA_PANEL8X4;
            else
                Panel->NativeModeIndex = VIA_PANEL8X6;
            break;
        case 1024:
            Panel->NativeModeIndex = VIA_PANEL10X7;
            break;
        case 1280:
            Panel->NativeModeIndex = VIA_PANEL12X10;
            break;
        case 1400:
            Panel->NativeModeIndex = VIA_PANEL14X10;
            break;
        case 1600:
            Panel->NativeModeIndex = VIA_PANEL16X12;
            break;
        default:
            Panel->NativeModeIndex = VIA_PANEL_INVALID;
            break;
        }
    } else {
        ViaPanelGetNativeModeFromScratchPad(output);
        DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "Unable to get information from EDID. Resolution from Scratchpad: %d \n", Panel->NativeModeIndex));
        if (Panel->NativeModeIndex == 0) {
            /* VIA_PANEL6X4 == 0, but that value equals unset */
            xf86DrvMsg(pScrn->scrnIndex, X_WARNING, "Unable to "
                       "retrieve PanelSize: using default (1024x768)\n");
            Panel->NativeModeIndex = VIA_PANEL10X7;
        }
    }

    if (Panel->NativeModeIndex < 7) {
        xf86DrvMsg(pScrn->scrnIndex, X_INFO, "Using panel at %s.\n",
                   PanelSizeString[Panel->NativeModeIndex]);
        status = XF86OutputStatusConnected;
    } else
        xf86DrvMsg(pScrn->scrnIndex, X_WARNING, "Unknown panel size "
                   "detected: %d.\n", Panel->NativeModeIndex);
    return status;
}

/*
 * ViaResolutionTable[i].PanelIndex is pBIOSInfo->PanelSize
 * Panel->PanelIndex is the index to lcdTable.
 * Only used by Legacy Mode Setting.
 */
static Bool
ViaPanelGetIndex(xf86OutputPtr output, DisplayModePtr mode)
{
    ScrnInfoPtr pScrn = output->scrn;
    ViaPanelInfoPtr Panel = output->driver_private;
    int i;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "ViaPanelGetIndex\n"));

    Panel->PanelIndex = VIA_BIOS_NUM_PANEL;

    if (VIAGetPanelSize(output) == XF86OutputStatusDisconnected) {
        xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
                    "ViaPanelGetIndex: Panel not detected.\n");
        return FALSE;
    }

    if (!ViaGetResolutionIndex(pScrn, Panel, mode)) {
        xf86DrvMsg(pScrn->scrnIndex, X_INFO, "Panel does not support this"
                   " resolution: %s\n", mode->name);
        return FALSE;
    }

    for (i = 0; ViaResolutionTable[i].Index != VIA_RES_INVALID; i++) {
        if (ViaResolutionTable[i].PanelIndex == Panel->NativeModeIndex) {
            Panel->NativeWidth = ViaResolutionTable[i].X;
            Panel->NativeHeight = ViaResolutionTable[i].Y;
            break;
        }
    }

    if (ViaResolutionTable[i].Index == VIA_RES_INVALID) {
        xf86DrvMsg(pScrn->scrnIndex, X_INFO, "ViaPanelGetIndex: Unable"
                   " to find matching PanelSize in ViaResolutionTable.\n");
        return FALSE;
    }

    if ((Panel->NativeWidth != mode->CrtcHDisplay) ||
        (Panel->NativeHeight != mode->CrtcVDisplay)) {
        xf86DrvMsg(pScrn->scrnIndex, X_INFO, "ViaPanelGetIndex: Non-native"
                   " resolutions are broken.\n");
        return FALSE;
    }

    for (i = 0; i < VIA_BIOS_NUM_PANEL; i++) {
        DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "ViaPanelGetIndex:"
                         "Match Debug: %d == %d)\n", Panel->NativeModeIndex,
                         lcdTable[i].fpSize));
        if (lcdTable[i].fpSize == Panel->NativeModeIndex) {
            int modeNum, tmp;

            modeNum = ViaGetVesaMode(pScrn, mode);
            if (modeNum == 0xFFFF) {
                xf86DrvMsg(pScrn->scrnIndex, X_ERROR, "ViaPanelGetIndex: "
                           "Unable to determine matching VESA modenumber.\n");
                return FALSE;
            }

            tmp = 0x01 << (modeNum & 0xF);
            if ((CARD16) tmp & lcdTable[i].SuptMode[(modeNum >> 4)]) {
                Panel->PanelIndex = i;
                DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "ViaPanelGetIndex:"
                                 "index: %d (%dx%d)\n", Panel->PanelIndex,
                                 Panel->NativeWidth, Panel->NativeHeight));
                return TRUE;
            }

            xf86DrvMsg(pScrn->scrnIndex, X_INFO, "ViaPanelGetIndex: Unable"
                       " to match given mode with this PanelSize.\n");
            return FALSE;
        }
    }

    xf86DrvMsg(pScrn->scrnIndex, X_INFO, "ViaPanelGetIndex: Unable"
               " to match PanelSize with an lcdTable entry.\n");
    return FALSE;
}

static int
via_lvds_mode_valid(xf86OutputPtr output, DisplayModePtr pMode)
{
    ScrnInfoPtr pScrn = output->scrn;
    VIAPtr pVia = VIAPTR(pScrn);

    if (pVia->UseLegacyModeSwitch) {
        if (!ViaPanelGetIndex(output, pMode))
            return MODE_BAD;
    } else {
        ViaPanelInfoPtr Panel = output->driver_private;

        if (Panel->NativeWidth < pMode->HDisplay ||
            Panel->NativeHeight < pMode->VDisplay)
            return MODE_PANEL;

        if (!Panel->Scale && Panel->NativeHeight != pMode->VDisplay &&
             Panel->NativeWidth != pMode->HDisplay)
            return MODE_PANEL;

        if (!ViaModeDotClockTranslate(pScrn, pMode))
            return MODE_NOCLOCK;
    }
    return MODE_OK;
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
    via_lvds_dpms(output, DPMSModeOff);

    if (output->crtc) {
        drmmode_crtc_private_ptr iga = output->crtc->driver_private;
        CARD8 value = 0x00; /* Value for IGA 1 */
        ScrnInfoPtr pScrn = output->scrn;
        vgaHWPtr hwp = VGAHWPTR(pScrn);

        /* IGA 2 */
        if (iga->index)
            value = 0x10;
        ViaCrtcMask(hwp, 0x99, value, value);
    }
}

static void
via_lvds_commit(xf86OutputPtr output)
{
    via_lvds_dpms(output, DPMSModeOn);
}

/*
 * Broken, only does native mode decently. I (Luc) personally broke this.
 * Only for LegacyModeSetting.
 */
static void
VIASetLCDMode(xf86OutputPtr output, DisplayModePtr mode)
{
    ViaPanelInfoPtr Panel = output->driver_private;
    ScrnInfoPtr pScrn = output->scrn;
    vgaHWPtr hwp = VGAHWPTR(pScrn);
    VIAPtr pVia = VIAPTR(pScrn);
    VIABIOSInfoPtr pBIOSInfo = pVia->pBIOSInfo;
    VIALCDModeTableRec Table = lcdTable[Panel->PanelIndex];
    int resIdx, port, offset, data, misc, i, j;
    CARD8 modeNum = 0;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "VIASetLCDMode\n"));

    if (Panel->NativeModeIndex == VIA_PANEL12X10)
        hwp->writeCrtc(hwp, 0x89, 0x07);

    /* LCD Expand Mode Y Scale Flag */
    Panel->scaleY = FALSE;

    /* Set LCD InitTb Regs */
    if (Panel->BusWidth == VIA_DI_12BIT) {
        if (pVia->IsSecondary)
            pBIOSInfo->Clock = Table.InitTb.LCDClk_12Bit;
        else {
            pBIOSInfo->Clock = Table.InitTb.VClk_12Bit;
            /* for some reason still to be defined this is necessary */
            ViaSetSecondaryDotclock(pScrn, Table.InitTb.LCDClk_12Bit);
        }
    } else {
        if (pVia->IsSecondary)
            pBIOSInfo->Clock = Table.InitTb.LCDClk;
        else {
            pBIOSInfo->Clock = Table.InitTb.VClk;
            ViaSetSecondaryDotclock(pScrn, Table.InitTb.LCDClk);
        }

    }

    ViaSetUseExternalClock(hwp);

    for (i = 0; i < Table.InitTb.numEntry; i++) {
        port = Table.InitTb.port[i];
        offset = Table.InitTb.offset[i];
        data = Table.InitTb.data[i];
        ViaVgahwWrite(hwp, 0x300 + port, offset, 0x301 + port, data);
    }

    if ((mode->CrtcHDisplay != Panel->NativeWidth) ||
        (mode->CrtcVDisplay != Panel->NativeHeight)) {
        VIALCDModeEntryPtr Main;
        VIALCDMPatchEntryPtr Patch1, Patch2;
        int numPatch1, numPatch2;

        resIdx = VIA_RES_INVALID;

        /* Find MxxxCtr & MxxxExp Index and
         * HWCursor Y Scale (PanelSize Y / Res. Y) */
        Panel->resY = mode->CrtcVDisplay;
        switch (Panel->ResolutionIndex) {
            case VIA_RES_640X480:
                resIdx = 0;
                break;
            case VIA_RES_800X600:
                resIdx = 1;
                break;
            case VIA_RES_1024X768:
                resIdx = 2;
                break;
            case VIA_RES_1152X864:
                resIdx = 3;
                break;
            case VIA_RES_1280X768:
            case VIA_RES_1280X960:
            case VIA_RES_1280X1024:
                if (Panel->NativeModeIndex == VIA_PANEL12X10)
                    resIdx = VIA_RES_INVALID;
                else
                    resIdx = 4;
                break;
            default:
                resIdx = VIA_RES_INVALID;
                break;
        }

        if ((mode->CrtcHDisplay == 640) && (mode->CrtcVDisplay == 400))
            resIdx = 0;

        if (resIdx == VIA_RES_INVALID) {
            xf86DrvMsg(pScrn->scrnIndex, X_ERROR, "VIASetLCDMode: Failed "
                       "to find a suitable Panel Size index.\n");
            return;
        }

        if (Panel->Center) {
            Main = &(Table.MCtr[resIdx]);
            Patch1 = Table.MPatchDP1Ctr;
            numPatch1 = Table.numMPatchDP1Ctr;
            Patch2 = Table.MPatchDP2Ctr;
            numPatch2 = Table.numMPatchDP2Ctr;
        } else {  /* expand! */
            /* LCD Expand Mode Y Scale Flag */
            Panel->scaleY = TRUE;
            Main = &(Table.MExp[resIdx]);
            Patch1 = Table.MPatchDP1Exp;
            numPatch1 = Table.numMPatchDP1Exp;
            Patch2 = Table.MPatchDP2Exp;
            numPatch2 = Table.numMPatchDP2Exp;
        }

        /* Set Main LCD Registers */
        for (i = 0; i < Main->numEntry; i++) {
            ViaVgahwWrite(hwp, 0x300 + Main->port[i], Main->offset[i],
                          0x301 + Main->port[i], Main->data[i]);
        }

        if (Panel->BusWidth == VIA_DI_12BIT) {
            if (pVia->IsSecondary)
                pBIOSInfo->Clock = Main->LCDClk_12Bit;
            else
                pBIOSInfo->Clock = Main->VClk_12Bit;
        } else {
            if (pVia->IsSecondary)
                pBIOSInfo->Clock = Main->LCDClk;
            else
                pBIOSInfo->Clock = Main->VClk;
        }

        j = ViaGetVesaMode(pScrn, mode);
        if (j == 0xFFFF) {
            xf86DrvMsg(pScrn->scrnIndex, X_ERROR, "VIASetLCDMode: "
                       "Unable to determine matching VESA modenumber.\n");
            return;
        }
        for (i = 0; i < modeFix.numEntry; i++) {
            if (modeFix.reqMode[i] == j) {
                modeNum = modeFix.fixMode[i];
                break;
            }
        }

        /* Set LCD Mode patch registers. */
        for (i = 0; i < numPatch2; i++, Patch2++) {
            if (Patch2->Mode == modeNum) {
                if (!Panel->Center && (mode->CrtcHDisplay == Panel->NativeWidth))
                    Panel->scaleY = FALSE;

                for (j = 0; j < Patch2->numEntry; j++) {
                    ViaVgahwWrite(hwp, 0x300 + Patch2->port[j],
                                  Patch2->offset[j], 0x301 + Patch2->port[j],
                                  Patch2->data[j]);
                }

                if (Panel->BusWidth == VIA_DI_12BIT) {
                    if (pVia->IsSecondary)
                        pBIOSInfo->Clock = Patch2->LCDClk_12Bit;
                    else
                        pBIOSInfo->Clock = Patch2->VClk_12Bit;
                } else {
                    if (pVia->IsSecondary)
                        pBIOSInfo->Clock = Patch2->LCDClk;
                    else
                        pBIOSInfo->Clock = Patch2->VClk;
                }
                break;
            }
        }

        /* Set LCD Secondary Mode Patch registers. */
        if (pVia->IsSecondary) {
            for (i = 0; i < numPatch1; i++, Patch1++) {
                if (Patch1->Mode == modeNum) {
                    for (j = 0; j < Patch1->numEntry; j++) {
                        ViaVgahwWrite(hwp, 0x300 + Patch1->port[j],
                                      Patch1->offset[j],
                                      0x301 + Patch1->port[j], Patch1->data[j]);
                    }
                    break;
                }
            }
        }
    }

    /* LCD patch 3D5.02 */
    misc = hwp->readCrtc(hwp, 0x01);
    hwp->writeCrtc(hwp, 0x02, misc);

    /* Enable LCD */
    if (!pVia->IsSecondary) {
        /* CRT Display Source Bit 6 - 0: CRT, 1: LCD */
        ViaSeqMask(hwp, 0x16, 0x40, 0x40);

        /* Enable Simultaneous */
        if (Panel->BusWidth == VIA_DI_12BIT) {
            hwp->writeCrtc(hwp, 0x6B, 0xA8);

            if ((pVia->Chipset == VIA_CLE266)
                && CLE266_REV_IS_AX(pVia->ChipRev))
                hwp->writeCrtc(hwp, 0x93, 0xB1);
            else
                hwp->writeCrtc(hwp, 0x93, 0xAF);
        } else {
            ViaCrtcMask(hwp, 0x6B, 0x08, 0x08);
            hwp->writeCrtc(hwp, 0x93, 0x00);
        }
        hwp->writeCrtc(hwp, 0x6A, 0x48);
    } else {
        /* CRT Display Source Bit 6 - 0: CRT, 1: LCD */
        ViaSeqMask(hwp, 0x16, 0x00, 0x40);

        /* Enable SAMM */
        if (Panel->BusWidth == VIA_DI_12BIT) {
            ViaCrtcMask(hwp, 0x6B, 0x20, 0x20);
            if ((pVia->Chipset == VIA_CLE266)
                && CLE266_REV_IS_AX(pVia->ChipRev))
                hwp->writeCrtc(hwp, 0x93, 0xB1);
            else
                hwp->writeCrtc(hwp, 0x93, 0xAF);
        } else {
            hwp->writeCrtc(hwp, 0x6B, 0x00);
            hwp->writeCrtc(hwp, 0x93, 0x00);
        }
        hwp->writeCrtc(hwp, 0x6A, 0xC8);
    }
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
via_lvds_mode_set(xf86OutputPtr output, DisplayModePtr mode,
					DisplayModePtr adjusted_mode)
{
    ViaPanelInfoPtr Panel = output->driver_private;
    ScrnInfoPtr pScrn = output->scrn;
    VIAPtr pVia = VIAPTR(pScrn);

    /*
     * FIXME: pVia->IsSecondary is not working here.  We should be able
     * to detect when the display is using the secondary head.
     * TODO: This should be enabled for other chipsets as well.
     */
    if (pVia->pVbe) {
        if (!pVia->useLegacyVBE) {
            /*
             * FIXME: Should we always set the panel expansion?
             * Does it depend on the resolution?
             */
            if (!ViaVbeSetPanelMode(pScrn, !Panel->Center)) {
                xf86DrvMsg(pScrn->scrnIndex, X_WARNING,
                            "Unable to set the panel mode.\n");
            }
        }

        switch (pVia->Chipset) {
        case VIA_P4M900:
        case VIA_VX800:
        case VIA_VX855:
        case VIA_VX900:
            /*
             * Since we are using virtual, we need to adjust
             * the offset to match the framebuffer alignment.
             */
            if (pScrn->displayWidth != adjusted_mode->CrtcHDisplay)
                ViaSecondCRTCHorizontalOffset(pScrn);
            break;
        }
    } else {
        if (!pVia->UseLegacyModeSwitch) {
            if (Panel->Scale) {
                ViaPanelScale(pScrn, mode->HDisplay, mode->VDisplay,
                                Panel->NativeWidth,
                                Panel->NativeHeight);
            } else
                ViaPanelScaleDisable(pScrn);
        } else {
            xf86CrtcPtr crtc = output->crtc;
            drmmode_crtc_private_ptr iga = crtc->driver_private;

            if (iga->index) {
                /* IGA 2 */
                if (Panel->PanelIndex != VIA_BIOS_NUM_PANEL) {
                    Panel->SetDVI = TRUE;
                    VIASetLCDMode(output, mode);
                }
            } else {
                /* IGA 1 */
                if (ViaPanelGetIndex(output, adjusted_mode))
                    VIASetLCDMode(output, adjusted_mode);
            }
        }
    }
}

static int
ViaPanelLookUpModeIndex(int width, int height)
{
    int i, index = VIA_PANEL_INVALID;
    int length = sizeof(ViaPanelNativeModes) / sizeof(ViaPanelModeRec);


    for (i = 0; i < length; i++) {
        if (ViaPanelNativeModes[i].Width == width
            && ViaPanelNativeModes[i].Height == height) {
            index = i;
            break;
        }
    }
    return index;
}

static xf86OutputStatus
via_lvds_detect(xf86OutputPtr output)
{
    static const char xoId[] = "OLPC XO 1.5";
    xf86OutputStatus status = XF86OutputStatusDisconnected;
    ViaPanelInfoPtr panel = output->driver_private;
    ScrnInfoPtr pScrn = output->scrn;
    VIAPtr pVia = VIAPTR(pScrn);
    vgaHWPtr hwp = VGAHWPTR(pScrn);

    /* Hardcode panel size for the XO */
    if(strcmp(pVia->Id->String, xoId) == 0) {
        panel->NativeWidth = 1200;
        panel->NativeHeight = 900;
        status = XF86OutputStatusConnected;
        return status;
    }

    if (!pVia->UseLegacyModeSwitch) {
        /* First try to get the mode from EDID. */
        if (!panel->NativeWidth || !panel->NativeHeight) {
            int width, height;
            Bool ret;

            ret = ViaPanelGetSizeFromDDCv1(output, &width, &height);
            if (ret) {
                panel->NativeModeIndex = ViaPanelLookUpModeIndex(width, height);
                DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "ViaPanelLookUpModeIndex, Width %d, Height %d, NativeModeIndex%d\n", width, height, panel->NativeModeIndex));
                if (panel->NativeModeIndex != VIA_PANEL_INVALID) {
                    panel->NativeWidth = width;
                    panel->NativeHeight = height;
                    status = XF86OutputStatusConnected;
                }
            } else {
                CARD8 CR6A = hwp->readCrtc(hwp, 0x6A);
                CARD8 CR6B = hwp->readCrtc(hwp, 0x6B);
                CARD8 CR97 = hwp->readCrtc(hwp, 0x97);
                CARD8 CR99 = hwp->readCrtc(hwp, 0x99);

                /* First test CRTC2 is out of reset and if its enabled or
                 * simultaneous mode is enabled. Also avoid the secondary
                 * DFP source */
                if ((((CR6A & 0xC0) == 0xC0) || (((CR6A & 0xC0) == 0x40) &&
                    (CR6B & 0x08))) && (CR97 & 0x10) && (CR99 & 0x10)) {
                        /* Use Vertical addreess register of IGA 2 */
                        panel->NativeWidth  = (hwp->readCrtc(hwp, 0x51) |
                                                    ((hwp->readCrtc(hwp, 0x55) & 0x70) << 4)) + 1;
                        panel->NativeHeight = (hwp->readCrtc(hwp, 0x59) |
                                                    ((hwp->readCrtc(hwp, 0x5D) & 0x38) << 5)) + 1;
                        panel->NativeModeIndex = VIA_PANEL6X4;

                        xf86DrvMsg(pScrn->scrnIndex, X_INFO, "Panel Mode probed %dx%d from IGA 2\n",
                                   panel->NativeWidth, panel->NativeHeight);

                        status = XF86OutputStatusConnected;
                } else if (!(CR97 & 0x10) && !(CR99 & 0x10)) {
                        CARD8 val;

                        /* IGA1 Horizontal Overscan register */
                        panel->NativeWidth = (hwp->readCrtc(hwp, 0x01) + 1) * 8;
                        /* IGA1 default Vertical Overscan register is
                         * incorrect on some devices so use VBlank start */
                        panel->NativeHeight = (hwp->readCrtc(hwp, 0x15) + 1);
                        val = hwp->readCrtc(hwp, 0x07);
                        panel->NativeHeight |= ((val >> 3) & 0x1) << 8;
                        panel->NativeHeight |= ((val >> 5) & 0x1) << 9;
                        val = hwp->readCrtc(hwp, 0x35);
                        panel->NativeHeight |= ((val >> 3) & 0x1) << 10;
                        panel->NativeModeIndex = VIA_PANEL6X4;

                        xf86DrvMsg(pScrn->scrnIndex, X_INFO, "Panel Mode probed %dx%d from IGA 1\n",
                                    panel->NativeWidth,
                                    panel->NativeHeight);
                        status = XF86OutputStatusConnected;
                }

                if (!panel->NativeWidth || !panel->NativeHeight)
                    ViaPanelGetNativeModeFromScratchPad(output);

                if (panel->NativeWidth && panel->NativeHeight)
                    status = XF86OutputStatusConnected;
            }
            DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "NativeMode: %d %d\n",
                            panel->NativeWidth, panel->NativeHeight));
        } else
            status = XF86OutputStatusConnected;
    } else
        status = VIAGetPanelSize(output);
    return status;
}

static DisplayModePtr
via_lvds_get_modes(xf86OutputPtr output)
{
    ViaPanelInfoPtr Panel = output->driver_private;
    ScrnInfoPtr pScrn = output->scrn;
    DisplayModePtr p = NULL;

    if (output->status == XF86OutputStatusConnected) {
        if (!output->MonInfo) {
            /*
             * Generates a display mode for the native panel resolution,
             * using CVT.
             */
            if (Panel->NativeWidth && Panel->NativeHeight) {
                VIAPtr pVia = VIAPTR(pScrn);

                if (!xf86NameCmp(pVia->Id->String, "OLPC XO 1.5"))
                    p = xf86DuplicateMode(&OLPCMode);
                else
                    p = xf86CVTMode(Panel->NativeWidth, Panel->NativeHeight,
                                    60.0f, FALSE, FALSE);
                if (p) {
                    p->CrtcHDisplay = p->HDisplay;
                    p->CrtcHSyncStart = p->HSyncStart;
                    p->CrtcHSyncEnd = p->HSyncEnd;
                    p->CrtcHTotal = p->HTotal;
                    p->CrtcHSkew = p->HSkew;
                    p->CrtcVDisplay = p->VDisplay;
                    p->CrtcVSyncStart = p->VSyncStart;
                    p->CrtcVSyncEnd = p->VSyncEnd;
                    p->CrtcVTotal = p->VTotal;

                    p->CrtcVBlankStart = min(p->CrtcVSyncStart, p->CrtcVDisplay);
                    p->CrtcVBlankEnd = max(p->CrtcVSyncEnd, p->CrtcVTotal);
                    p->CrtcHBlankStart = min(p->CrtcHSyncStart, p->CrtcHDisplay);
                    p->CrtcHBlankEnd = max(p->CrtcHSyncEnd, p->CrtcHTotal);
                    p->type = M_T_DRIVER | M_T_PREFERRED;
                } else {
                    xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
                                "Out of memory. Size: %zu bytes\n", sizeof(DisplayModeRec));
                }
            } else {
                xf86DrvMsg(pScrn->scrnIndex, X_WARNING,
                            "Invalid panel dimension (%dx%d)\n",
                            Panel->NativeWidth, Panel->NativeHeight);
            }
        } else {
            p = xf86OutputGetEDIDModes(output);
        }
    }
    return p;
}

static void
via_lvds_destroy(xf86OutputPtr output)
{
    if (output->driver_private)
        free(output->driver_private);
    output->driver_private = NULL;
}

static const xf86OutputFuncsRec via_lvds_funcs = {
    .create_resources   = via_lvds_create_resources,
#ifdef RANDR_12_INTERFACE
    .set_property       = via_lvds_set_property,
    .get_property       = via_lvds_get_property,
#endif
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
    .destroy            = via_lvds_destroy,
};

/*
 * Sets the panel dimensions from the configuration
 * using name with format "9999x9999".
 */
static void
ViaPanelGetNativeModeFromOption(ScrnInfoPtr pScrn, ViaPanelInfoPtr panel, char *name)
{
    char aux[strlen(name) + 1];
    CARD8 length, index;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                     "ViaPanelGetNativeModeFromOption\n"));

    panel->NativeModeIndex = VIA_PANEL_INVALID;
    length = sizeof(ViaPanelNativeModes) / sizeof(ViaPanelModeRec);

    for (index = 0; index < length; index++) {
        sprintf(aux, "%dx%d", ViaPanelNativeModes[index].Width,
                ViaPanelNativeModes[index].Height);
        if (!xf86NameCmp(name, aux)) {
            panel->NativeModeIndex = index;
            panel->NativeWidth = ViaPanelNativeModes[index].Width;
            panel->NativeHeight = ViaPanelNativeModes[index].Height;
            break;
        }
    }
}

void
via_lvds_init(ScrnInfoPtr pScrn)
{
    ViaPanelInfoPtr Panel = (ViaPanelInfoPtr) xnfcalloc(sizeof(ViaPanelInfoRec), 1);
    OptionInfoPtr  Options = xnfalloc(sizeof(ViaPanelOptions));
    MessageType from = X_DEFAULT;
    VIAPtr pVia = VIAPTR(pScrn);
    xf86OutputPtr output = NULL;
    Bool ForcePanel = FALSE;
    char *s = NULL;

    if (!Panel)
        return;

    memcpy(Options, ViaPanelOptions, sizeof(ViaPanelOptions));
    xf86ProcessOptions(pScrn->scrnIndex, pScrn->options, Options);

    Panel->NativeModeIndex = VIA_PANEL_INVALID;
    Panel->BusWidth = VIA_DI_12BIT;
    if ((s = xf86GetOptValString(Options, OPTION_BUSWIDTH))) {
        from = X_CONFIG;
        if (!xf86NameCmp(s, "12BIT")) {
            Panel->BusWidth = VIA_DI_12BIT;
        } else if (!xf86NameCmp(s, "24BIT")) {
            Panel->BusWidth = VIA_DI_24BIT;
        }
    }
    xf86DrvMsg(pScrn->scrnIndex, from,
               "LVDS-0 : Digital output bus width is %d bits.\n",
               (Panel->BusWidth == VIA_DI_12BIT) ? 12 : 24);

    /* LCD Center/Expend Option */
    Panel->Center = FALSE;
    from = xf86GetOptValBool(Options, OPTION_CENTER, &Panel->Center)
            ? X_CONFIG : X_DEFAULT;
    xf86DrvMsg(pScrn->scrnIndex, from, "LVDS-0 : DVI Center is %s.\n",
               Panel->Center ? "enabled" : "disabled");

    /* Force the use of the Panel? */
    from = xf86GetOptValBool(Options, OPTION_FORCEPANEL,
                             &ForcePanel)
            ? X_CONFIG : X_DEFAULT;
    xf86DrvMsg(pScrn->scrnIndex, from, "LVDS Panel will %sbe forced.\n",
               ForcePanel ? "" : "not ");

    /* Panel Size Option */
    if ((s = xf86GetOptValString(Options, OPTION_PANELSIZE))) {
        ViaPanelGetNativeModeFromOption(pScrn, Panel, s);
        if (Panel->NativeModeIndex != VIA_PANEL_INVALID) {
            DEBUG(xf86DrvMsg
                  (pScrn->scrnIndex, X_CONFIG, "LVDS Panel mode index is %d\n",
                   Panel->NativeModeIndex));
            xf86DrvMsg(pScrn->scrnIndex, X_CONFIG,
                       "Selected Panel Size is %dx%d\n", Panel->NativeWidth,
                       Panel->NativeHeight);
        } else
            xf86DrvMsg(pScrn->scrnIndex, X_WARNING,
                        "%s is not a valid panel size.\n", s);
    } else {
        xf86DrvMsg(pScrn->scrnIndex, X_DEFAULT,
                   "Panel size is not selected from config file.\n");
    }

    if (ForcePanel) {
        xf86DrvMsg(pScrn->scrnIndex, X_INFO, "Enabling panel from config.\n");
        output = xf86OutputCreate(pScrn, &via_lvds_funcs, "LVDS-1");
    } else if (pVia->Id && (pVia->Id->Outputs & VIA_DEVICE_LCD)) {
        xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                    "Enabling panel from PCI-subsystem ID information.\n");
        output = xf86OutputCreate(pScrn, &via_lvds_funcs, "LVDS-1");
    }

    if (output)  {
        output->driver_private = Panel;

        if (pVia->Chipset == VIA_VX900)
            output->possible_crtcs = 0x3;
        else
            output->possible_crtcs = 0x2;
        output->possible_clones = 0;
        output->interlaceAllowed = FALSE;
        output->doubleScanAllowed = FALSE;

        if (!xf86NameCmp(pVia->Id->String, "OLPC XO 1.5")) {
            output->mm_height = 152;
            output->mm_width = 114;
        }
    } else {
        free(Panel);
    }
}

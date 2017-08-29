/*
 * Copyright 2005-2016 The OpenChrome Project
 *                     [https://www.freedesktop.org/wiki/Openchrome]
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

/*
 * via_analog.c
 *
 * Handles the initialization and management of analog VGA related
 * resources.
 *
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "via_driver.h"
#include <unistd.h>


/*
 * Enables or disables analog VGA output by controlling DAC
 * (Digital to Analog Converter) output state.
 */
static void
viaAnalogOutput(ScrnInfoPtr pScrn, Bool outputState)
{
    vgaHWPtr hwp = VGAHWPTR(pScrn);

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Entered viaAnalogOutput.\n"));

    /* This register controls analog VGA DAC output state. */
    /* 3X5.47[2] - DACOFF Backdoor Register
     *             0: DAC on
     *             1: DAC off */
    ViaCrtcMask(hwp, 0x47, outputState ? 0x00 : 0x04, 0x04);
    xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                "Analog VGA Output: %s\n",
                outputState ? "On" : "Off");

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Exiting viaAnalogOutput.\n"));
}

/*
 * Specifies IGA1 or IGA2 for analog VGA DAC source.
 */
static void
viaAnalogSetDisplaySource(ScrnInfoPtr pScrn, CARD8 displaySource)
{
    vgaHWPtr hwp = VGAHWPTR(pScrn);
    CARD8 value = displaySource;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Entered viaAnalogSetDisplaySource.\n"));

    ViaSeqMask(hwp, 0x16, value << 6, 0x40);
    xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                "Analog VGA Display Output Source: IGA%d\n",
                (value & 0x01) + 1);

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Exiting viaAnalogSetDisplaySource.\n"));
}

/*
 * Intializes analog VGA related registers.
 */
static void
viaAnalogInit(ScrnInfoPtr pScrn)
{
    vgaHWPtr hwp = VGAHWPTR(pScrn);
    VIAPtr pVia = VIAPTR(pScrn);

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Entered viaAnalogInit.\n"));

    /* 3X5.37[7]   - DAC Power Save Control 1
     *               0: Depend on Rx3X5.37[5:4] setting
     *               1: DAC always goes into power save mode
     * 3X5.37[6]   - DAC Power Down Control
     *               0: Depend on Rx3X5.47[2] setting
     *               1: DAC never goes to power down mode
     * 3X5.37[5:4] - DAC Power Save Control 2
     *               00: DAC never goes to power save mode
     *               01: DAC goes to power save mode by line
     *               10: DAC goes to power save mode by frame
     *               11: DAC goes to power save mode by line and frame
     * 3X5.37[3]   - DAC PEDESTAL Control
     * 3X5.37[2:0] - DAC Factor
     *               (Default: 100) */
    ViaCrtcMask(hwp, 0x37, 0x04, 0xFF);

    switch (pVia->Chipset) {
    case VIA_CX700:
    case VIA_VX800:
    case VIA_VX855:
    case VIA_VX900:
        /* 3C5.5E[0] - CRT DACOFF Setting
         *             1: CRT DACOFF controlled by 3C5.01[5] */
        ViaSeqMask(hwp, 0x5E, 0x01, 0x01);
        break;
    default:
        break;
    }

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Exiting viaAnalogInit.\n"));
}

/*
 * Sets the polarity of horizontal synchronization and vertical
 * synchronization.
 */
static void
viaAnalogSetSyncPolarity(ScrnInfoPtr pScrn, DisplayModePtr mode)
{
    vgaHWPtr hwp = VGAHWPTR(pScrn);
    CARD8 miscRegister;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Entered viaAnalogSetSyncPolarity.\n"));

/* Set certain bits of miscellaneous output register
 * meant for IGA1. */
    miscRegister = hwp->readMiscOut(hwp);
    if (mode->Flags & V_NHSYNC) {
        miscRegister |= 0x40;
    } else {
        miscRegister &= (~0x40);
    }

    if (mode->Flags & V_NVSYNC) {
        miscRegister |= 0x80;
    } else {
        miscRegister &= (~0x80);
    }

    hwp->writeMiscOut(hwp, miscRegister);

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Exiting viaAnalogSetSyncPolarity.\n"));
}


static void
via_analog_create_resources(xf86OutputPtr output)
{
}

static void
via_analog_dpms(xf86OutputPtr output, int mode)
{
    ScrnInfoPtr pScrn = output->scrn;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Entered via_analog_dpms.\n"));

    switch (mode) {
    case DPMSModeOn:
        viaAnalogOutput(pScrn, TRUE);
        break;
    case DPMSModeStandby:
    case DPMSModeSuspend:
    case DPMSModeOff:
        viaAnalogOutput(pScrn, FALSE);
        break;
    default:
        break;
    }

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Exiting via_analog_dpms.\n"));
}

static void
via_analog_save(xf86OutputPtr output)
{
}

static void
via_analog_restore(xf86OutputPtr output)
{
}

static int
via_analog_mode_valid(xf86OutputPtr output, DisplayModePtr pMode)
{
    ScrnInfoPtr pScrn = output->scrn;

    if (!ViaModeDotClockTranslate(pScrn, pMode))
        return MODE_NOCLOCK;
    return MODE_OK;
}

static Bool
via_analog_mode_fixup(xf86OutputPtr output, DisplayModePtr mode,
                      DisplayModePtr adjusted_mode)
{
    return TRUE;
}

static void
via_analog_prepare(xf86OutputPtr output)
{
    via_analog_dpms(output, DPMSModeOff);
}

static void
via_analog_commit(xf86OutputPtr output)
{
    via_analog_dpms(output, DPMSModeOn);
}

static void
via_analog_mode_set(xf86OutputPtr output, DisplayModePtr mode,
                    DisplayModePtr adjusted_mode)
{
    ScrnInfoPtr pScrn = output->scrn;
    drmmode_crtc_private_ptr iga = output->crtc->driver_private;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Entered via_analog_mode_set.\n"));

    if (output->crtc) {
        viaAnalogInit(pScrn);
        viaAnalogSetSyncPolarity(pScrn, adjusted_mode);
        viaAnalogSetDisplaySource(pScrn, iga->index ? 0x01 : 0x00);
    }

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Exiting via_analog_mode_set.\n"));
}

static xf86OutputStatus
via_analog_detect(xf86OutputPtr output)
{
    xf86OutputStatus status = XF86OutputStatusDisconnected;
    ScrnInfoPtr pScrn = output->scrn;
    VIAPtr pVia = VIAPTR(pScrn);
    xf86MonPtr mon;

    /* Probe I2C Bus 1 to see if a VGA monitor is connected. */
    xf86DrvMsg(pScrn->scrnIndex, X_PROBED,
                "Probing for a VGA monitor on I2C Bus 1.\n");
    mon = xf86OutputGetEDID(output, pVia->pI2CBus1);
    if (mon && (!mon->features.input_type)) {
        xf86OutputSetEDID(output, mon);
        status = XF86OutputStatusConnected;
        xf86DrvMsg(pScrn->scrnIndex, X_PROBED,
                    "Detected a VGA monitor on I2C Bus 1.\n");
    } else {
        xf86DrvMsg(pScrn->scrnIndex, X_PROBED,
                    "Did not detect a VGA monitor on I2C Bus 1.\n");

        /* Probe I2C Bus 2 to see if a VGA monitor is connected. */
        xf86DrvMsg(pScrn->scrnIndex, X_PROBED,
                    "Probing for a VGA monitor on I2C Bus 2.\n");
        mon = xf86OutputGetEDID(output, pVia->pI2CBus2);
        if (mon && (!mon->features.input_type)) {
            xf86OutputSetEDID(output, mon);
            status = XF86OutputStatusConnected;
            xf86DrvMsg(pScrn->scrnIndex, X_PROBED,
                        "Detected a VGA monitor on I2C Bus 2.\n");
        } else {
            xf86DrvMsg(pScrn->scrnIndex, X_PROBED,
                        "Did not detect a VGA monitor on I2C Bus 2.\n");

            /* Perform manual detection of a VGA monitor since */
            /* it was not detected via I2C buses. */
            xf86DrvMsg(pScrn->scrnIndex, X_PROBED,
                        "Now perform manual detection of a VGA "
                        "monitor.\n");
            vgaHWPtr hwp = VGAHWPTR(pScrn);
            CARD8 SR01 = hwp->readSeq(hwp, 0x01);
            CARD8 SR40 = hwp->readSeq(hwp, 0x40);
            CARD8 CR36 = hwp->readCrtc(hwp, 0x36);

            /* We have to power on the display to detect it */
            ViaSeqMask(hwp, 0x01, 0x00, 0x20);
            ViaCrtcMask(hwp, 0x36, 0x00, 0xF0);

            /* Wait for vblank */
            usleep(16);

            /* Detect the load on pins */
            ViaSeqMask(hwp, 0x40, 0x80, 0x80);

            if ((VIA_CX700 == pVia->Chipset) ||
                (VIA_VX800 == pVia->Chipset) ||
                (VIA_VX855 == pVia->Chipset) ||
                (VIA_VX900 == pVia->Chipset))
                ViaSeqMask(hwp, 0x40, 0x00, 0x80);

            if (ViaVgahwIn(hwp, 0x3C2) & 0x20) {
                status = XF86OutputStatusConnected;
                xf86DrvMsg(pScrn->scrnIndex, X_PROBED,
                            "Detected a VGA monitor using manual "
                            "detection method.\n");
            }

            if ((VIA_CX700 == pVia->Chipset) ||
                (VIA_VX800 == pVia->Chipset) ||
                (VIA_VX855 == pVia->Chipset) ||
                (VIA_VX900 == pVia->Chipset))
                ViaSeqMask(hwp, 0x40, 0x00, 0x80);

            /* Restore previous state */
            hwp->writeSeq(hwp, 0x40, SR40);
            hwp->writeSeq(hwp, 0x01, SR01);
            hwp->writeCrtc(hwp, 0x36, CR36);
        }
    }

    return status;
}

#ifdef RANDR_12_INTERFACE
static Bool
via_analog_set_property(xf86OutputPtr output, Atom property,
                        RRPropertyValuePtr value)
{
    return TRUE;
}
#endif

#ifdef RANDR_13_INTERFACE
static Bool
via_analog_get_property(xf86OutputPtr output, Atom property)
{
    return FALSE;
}
#endif

static void
via_analog_destroy(xf86OutputPtr output)
{
}

static const xf86OutputFuncsRec via_analog_funcs = {
    .create_resources   = via_analog_create_resources,
    .dpms               = via_analog_dpms,
    .save               = via_analog_save,
    .restore            = via_analog_restore,
    .mode_valid         = via_analog_mode_valid,
    .mode_fixup         = via_analog_mode_fixup,
    .prepare            = via_analog_prepare,
    .commit             = via_analog_commit,
    .mode_set           = via_analog_mode_set,
    .detect             = via_analog_detect,
    .get_modes          = xf86OutputGetEDIDModes,
#ifdef RANDR_12_INTERFACE
    .set_property       = via_analog_set_property,
#endif
#ifdef RANDR_13_INTERFACE
    .get_property       = via_analog_get_property,
#endif
    .destroy            = via_analog_destroy,
};

void
via_analog_init(ScrnInfoPtr pScrn)
{
    VIAPtr pVia = VIAPTR(pScrn);
    VIABIOSInfoPtr pBIOSInfo = pVia->pBIOSInfo;
    xf86OutputPtr output = NULL;
    char outputNameBuffer[32];

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Entered via_analog_init.\n"));

    if (!pVia->pI2CBus1 || !pVia->pI2CBus2) {
        xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
                    "I2C Bus 1 or I2C Bus 2 does not exist.\n");
        DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                            "Exiting via_analog_init.\n"));
        return;
    }

    /* The code to dynamically designate the output name for
     * xrandr was borrowed from xf86-video-r128 DDX. */
    sprintf(outputNameBuffer, "VGA-%d", (pVia->numberVGA + 1));
    output = xf86OutputCreate(pScrn, &via_analog_funcs, outputNameBuffer);

    /* While there are two (2) display controllers registered with the
     * X.Org Server, it is often desirable to fix the analog VGA output
     * to IGA1 since LVDS FP (Flat Panel) typically prefers IGA2. (While
     * it is not used at this point, only IGA2 contains panel resolution
     * scaling functionality. IGA1 does not have this.)
     * With this arrangement, DVI should end up getting assigned to IGA2
     * since DVI can go to either display controller without limitations.
     * This should be the case for TV as well. */
    output->possible_crtcs = (1 << 0);

    output->possible_clones = 0;
    output->interlaceAllowed = TRUE;
    output->doubleScanAllowed = FALSE;
    pBIOSInfo->analog = output;

    /* Increment the number of analog VGA connectors. */
    pVia->numberVGA++;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Exiting via_analog_init.\n"));
}

/*
 * Copyright 2016 Kevin Brace
 * Copyright 2005-2016 The OpenChrome Project
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

/*
 * via_outputs.c
 *
 * Everything to do with setting and changing xf86Outputs.
 *
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "via_driver.h"
#include <unistd.h>

/*
 * Modetable nonsense.
 *
 */
#include "via_mode.h"

static void
ViaPrintMode(ScrnInfoPtr pScrn, DisplayModePtr mode)
{
    xf86PrintModeline(pScrn->scrnIndex, mode);

    xf86DrvMsg(pScrn->scrnIndex, X_INFO, "CrtcHDisplay: 0x%x\n",
               mode->CrtcHDisplay);
    xf86DrvMsg(pScrn->scrnIndex, X_INFO, "CrtcHBlankStart: 0x%x\n",
               mode->CrtcHBlankStart);
    xf86DrvMsg(pScrn->scrnIndex, X_INFO, "CrtcHSyncStart: 0x%x\n",
               mode->CrtcHSyncStart);
    xf86DrvMsg(pScrn->scrnIndex, X_INFO, "CrtcHSyncEnd: 0x%x\n",
               mode->CrtcHSyncEnd);
    xf86DrvMsg(pScrn->scrnIndex, X_INFO, "CrtcHBlankEnd: 0x%x\n",
               mode->CrtcHBlankEnd);
    xf86DrvMsg(pScrn->scrnIndex, X_INFO, "CrtcHTotal: 0x%x\n",
               mode->CrtcHTotal);
    xf86DrvMsg(pScrn->scrnIndex, X_INFO, "CrtcHSkew: 0x%x\n",
               mode->CrtcHSkew);
    xf86DrvMsg(pScrn->scrnIndex, X_INFO, "CrtcVDisplay: 0x%x\n",
               mode->CrtcVDisplay);
    xf86DrvMsg(pScrn->scrnIndex, X_INFO, "CrtcVBlankStart: 0x%x\n",
               mode->CrtcVBlankStart);
    xf86DrvMsg(pScrn->scrnIndex, X_INFO, "CrtcVSyncStart: 0x%x\n",
               mode->CrtcVSyncStart);
    xf86DrvMsg(pScrn->scrnIndex, X_INFO, "CrtcVSyncEnd: 0x%x\n",
               mode->CrtcVSyncEnd);
    xf86DrvMsg(pScrn->scrnIndex, X_INFO, "CrtcVBlankEnd: 0x%x\n",
               mode->CrtcVBlankEnd);
    xf86DrvMsg(pScrn->scrnIndex, X_INFO, "CrtcVTotal: 0x%x\n",
               mode->CrtcVTotal);

}

/*
 *
 * TV specific code.
 *
 */
void
ViaTVSave(ScrnInfoPtr pScrn)
{
    VIABIOSInfoPtr pBIOSInfo = VIAPTR(pScrn)->pBIOSInfo;

    if (pBIOSInfo->TVSave)
        pBIOSInfo->TVSave(pScrn);
}

void
ViaTVRestore(ScrnInfoPtr pScrn)
{
    VIABIOSInfoPtr pBIOSInfo = VIAPTR(pScrn)->pBIOSInfo;

    if (pBIOSInfo->TVRestore)
        pBIOSInfo->TVRestore(pScrn);
}

static Bool
ViaTVDACSense(ScrnInfoPtr pScrn)
{
    VIABIOSInfoPtr pBIOSInfo = VIAPTR(pScrn)->pBIOSInfo;

    if (pBIOSInfo->TVDACSense)
        return pBIOSInfo->TVDACSense(pScrn);
    return FALSE;
}

static void
ViaTVSetMode(xf86CrtcPtr crtc, DisplayModePtr mode)
{
    ScrnInfoPtr pScrn = crtc->scrn;
    VIAPtr pVia = VIAPTR(pScrn);
    VIABIOSInfoPtr pBIOSInfo = pVia->pBIOSInfo;

    if (pBIOSInfo->TVModeI2C)
        pBIOSInfo->TVModeI2C(pScrn, mode);

    if (pBIOSInfo->TVModeCrtc)
        pBIOSInfo->TVModeCrtc(crtc, mode);

    /* TV reset. */
    xf86I2CWriteByte(pBIOSInfo->TVI2CDev, 0x1D, 0x00);
    xf86I2CWriteByte(pBIOSInfo->TVI2CDev, 0x1D, 0x80);
}

void
ViaTVPower(ScrnInfoPtr pScrn, Bool On)
{
    VIABIOSInfoPtr pBIOSInfo = VIAPTR(pScrn)->pBIOSInfo;

#ifdef HAVE_DEBUG
    if (On)
        xf86DrvMsg(pScrn->scrnIndex, X_INFO, "ViaTVPower: On.\n");
    else
        xf86DrvMsg(pScrn->scrnIndex, X_INFO, "ViaTVPower: Off.\n");
#endif

    if (pBIOSInfo->TVPower)
        pBIOSInfo->TVPower(pScrn, On);
}

#ifdef HAVE_DEBUG
void
ViaTVPrintRegs(ScrnInfoPtr pScrn)
{
    VIABIOSInfoPtr pBIOSInfo = VIAPTR(pScrn)->pBIOSInfo;

    if (pBIOSInfo->TVPrintRegs)
        pBIOSInfo->TVPrintRegs(pScrn);
}
#endif /* HAVE_DEBUG */

static void
via_tv_create_resources(xf86OutputPtr output)
{
}

#ifdef RANDR_12_INTERFACE
static Bool
via_tv_set_property(xf86OutputPtr output, Atom property,
                    RRPropertyValuePtr value)
{
    return TRUE;
}

static Bool
via_tv_get_property(xf86OutputPtr output, Atom property)
{
    return FALSE;
}
#endif

static void
via_tv_dpms(xf86OutputPtr output, int mode)
{
    ScrnInfoPtr pScrn = output->scrn;

    switch (mode) {
    case DPMSModeOn:
        ViaTVPower(pScrn, TRUE);
        break;

    case DPMSModeStandby:
    case DPMSModeSuspend:
    case DPMSModeOff:
        ViaTVPower(pScrn, FALSE);
        break;
    }
}

static void
via_tv_save(xf86OutputPtr output)
{
    ScrnInfoPtr pScrn = output->scrn;

    ViaTVSave(pScrn);
}

static void
via_tv_restore(xf86OutputPtr output)
{
    ScrnInfoPtr pScrn = output->scrn;

    ViaTVRestore(pScrn);
}

static int
via_tv_mode_valid(xf86OutputPtr output, DisplayModePtr pMode)
{
    ScrnInfoPtr pScrn = output->scrn;
    VIAPtr pVia = VIAPTR(pScrn);
    int ret = MODE_OK;

    if (!ViaModeDotClockTranslate(pScrn, pMode))
        return MODE_NOCLOCK;

    return ret;
}

static Bool
via_tv_mode_fixup(xf86OutputPtr output, DisplayModePtr mode,
                  DisplayModePtr adjusted_mode)
{
    return TRUE;
}

static void
via_tv_prepare(xf86OutputPtr output)
{
    via_tv_dpms(output, DPMSModeOff);
}

static void
via_tv_commit(xf86OutputPtr output)
{
    via_tv_dpms(output, DPMSModeOn);
}

static void
ViaDisplayEnableDVO(ScrnInfoPtr pScrn, int port)
{
    vgaHWPtr hwp = VGAHWPTR(pScrn);

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "ViaDisplayEnableDVO, port: %d\n",
                     port));
    switch (port) {
    case VIA_DI_PORT_DVP0:
        ViaSeqMask(hwp, 0x1E, 0xC0, 0xC0);
        break;
    case VIA_DI_PORT_DVP1:
        ViaSeqMask(hwp, 0x1E, 0x30, 0x30);
        break;
    }
}

static void
ViaDisplayDisableDVO(ScrnInfoPtr pScrn, int port)
{
    vgaHWPtr hwp = VGAHWPTR(pScrn);

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "ViaDisplayDisableDVO, port: %d\n",
                     port));
    switch (port) {
    case VIA_DI_PORT_DVP0:
        ViaSeqMask(hwp, 0x1E, 0x00, 0xC0);
        break;
    case VIA_DI_PORT_DVP1:
        ViaSeqMask(hwp, 0x1E, 0x00, 0x30);
        break;
    }
}

static void
ViaDisplaySetStreamOnDVO(ScrnInfoPtr pScrn, int port, int iga)
{
    vgaHWPtr hwp = VGAHWPTR(pScrn);
    int regNum;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "ViaDisplaySetStreamOnDVO, port: %d\n",
                     port));

    switch (port) {
        case VIA_DI_PORT_DVP0:
            regNum = 0x96;
            break;
        case VIA_DI_PORT_DVP1:
            regNum = 0x9B;
            break;
        case VIA_DI_PORT_DFPLOW:
            regNum = 0x97;
            break;
        case VIA_DI_PORT_DFPHIGH:
            regNum = 0x99;
            break;
    }

    if (!iga)
        ViaCrtcMask(hwp, regNum, 0x00, 0x10);
    else
        ViaCrtcMask(hwp, regNum, 0x10, 0x10);
}

static void
via_tv_mode_set(xf86OutputPtr output, DisplayModePtr mode,
                DisplayModePtr adjusted_mode)
{
    ScrnInfoPtr pScrn = output->scrn;
    VIAPtr pVia = VIAPTR(pScrn);
    VIABIOSInfoPtr pBIOSInfo = pVia->pBIOSInfo;

    /* TV on FirstCrtc */
    if (output->crtc) {
        drmmode_crtc_private_ptr iga = output->crtc->driver_private;

        ViaDisplaySetStreamOnDVO(pScrn, pBIOSInfo->TVDIPort, iga->index);
    }
    ViaDisplayEnableDVO(pScrn, pBIOSInfo->TVDIPort);

    ViaTVSetMode(output->crtc, adjusted_mode);

    pVia->FirstInit = FALSE;
}

static xf86OutputStatus
via_tv_detect(xf86OutputPtr output)
{
    xf86OutputStatus status = XF86OutputStatusDisconnected;
    ScrnInfoPtr pScrn = output->scrn;

    if (ViaTVDACSense(pScrn))
        status = XF86OutputStatusConnected;
    return status;
}

static DisplayModePtr
via_tv_get_modes(xf86OutputPtr output)
{
    DisplayModePtr modes = NULL, mode = NULL;
    ScrnInfoPtr pScrn = output->scrn;
    VIAPtr pVia = VIAPTR(pScrn);
    int i;

    for (i = 0; i < pVia->pBIOSInfo->TVNumModes; i++) {
        mode = xf86DuplicateMode(&pVia->pBIOSInfo->TVModes[i]);
        modes = xf86ModesAdd(modes, mode);
    }
    return modes;
}

static void
via_tv_destroy(xf86OutputPtr output)
{
}

static const xf86OutputFuncsRec via_tv_funcs = {
    .create_resources   = via_tv_create_resources,
#ifdef RANDR_12_INTERFACE
    .set_property       = via_tv_set_property,
#endif
#ifdef RANDR_13_INTERFACE
    .get_property       = via_tv_get_property,
#endif
    .dpms               = via_tv_dpms,
    .save               = via_tv_save,
    .restore            = via_tv_restore,
    .mode_valid         = via_tv_mode_valid,
    .mode_fixup         = via_tv_mode_fixup,
    .prepare            = via_tv_prepare,
    .commit             = via_tv_commit,
    .mode_set           = via_tv_mode_set,
    .detect             = via_tv_detect,
    .get_modes          = via_tv_get_modes,
    .destroy            = via_tv_destroy,
};

/*
 *
 */
static Bool
via_tv_init(ScrnInfoPtr pScrn)
{
    VIAPtr pVia = VIAPTR(pScrn);
    VIABIOSInfoPtr pBIOSInfo = pVia->pBIOSInfo;
    xf86OutputPtr output = NULL;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Entered via_tv_init.\n"));

    /* preset some pBIOSInfo TV related values -- move up */
    pBIOSInfo->TVEncoder = VIA_NONETV;
    pBIOSInfo->TVI2CDev = NULL;
    pBIOSInfo->TVSave = NULL;
    pBIOSInfo->TVRestore = NULL;
    pBIOSInfo->TVDACSense = NULL;
    pBIOSInfo->TVModeValid = NULL;
    pBIOSInfo->TVModeI2C = NULL;
    pBIOSInfo->TVModeCrtc = NULL;
    pBIOSInfo->TVPower = NULL;
    pBIOSInfo->TVModes = NULL;
    pBIOSInfo->TVPrintRegs = NULL;
    pBIOSInfo->LCDPower = NULL;
    pBIOSInfo->TVNumRegs = 0;

    /*
     * On an SK43G (KM400/Ch7011), false positive detections at a VT162x
     * chip were observed, so try to detect the Ch7011 first.
     */
    if (pVia->pI2CBus2 && xf86I2CProbeAddress(pVia->pI2CBus2, 0xEC))
        pBIOSInfo->TVI2CDev = ViaCH7xxxDetect(pScrn, pVia->pI2CBus2, 0xEC);
    else if (pVia->pI2CBus2 && xf86I2CProbeAddress(pVia->pI2CBus2, 0x40))
        pBIOSInfo->TVI2CDev = ViaVT162xDetect(pScrn, pVia->pI2CBus2, 0x40);
    else if (pVia->pI2CBus3 && xf86I2CProbeAddress(pVia->pI2CBus3, 0x40))
        pBIOSInfo->TVI2CDev = ViaVT162xDetect(pScrn, pVia->pI2CBus3, 0x40);
    else if (pVia->pI2CBus2 && xf86I2CProbeAddress(pVia->pI2CBus2, 0xEA))
        pBIOSInfo->TVI2CDev = ViaCH7xxxDetect(pScrn, pVia->pI2CBus2, 0xEA);
    else if (pVia->pI2CBus3 && xf86I2CProbeAddress(pVia->pI2CBus3, 0xEA))
        pBIOSInfo->TVI2CDev = ViaCH7xxxDetect(pScrn, pVia->pI2CBus3, 0xEA);

    if (!pBIOSInfo->TVI2CDev) {
        xf86DrvMsg(pScrn->scrnIndex, X_PROBED,
                    "Did not detect a TV encoder.\n");
        DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                            "Exiting via_tv_init.\n"));

        return FALSE;
    }

    switch (pBIOSInfo->TVEncoder) {
        case VIA_VT1621:
        case VIA_VT1622:
        case VIA_VT1623:
        case VIA_VT1625:
            ViaVT162xInit(pScrn);
            break;
        case VIA_CH7011:
        case VIA_CH7019A:
        case VIA_CH7019B:
            ViaCH7xxxInit(pScrn);
            break;
        default:
            xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
                        "Was not able to initialize a known TV encoder.\n");
            DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                                "Exiting via_tv_init.\n"));
            return FALSE;
            break;
    }

    if (!pBIOSInfo->TVSave || !pBIOSInfo->TVRestore
        || !pBIOSInfo->TVDACSense || !pBIOSInfo->TVModeValid
        || !pBIOSInfo->TVModeI2C || !pBIOSInfo->TVModeCrtc
        || !pBIOSInfo->TVPower || !pBIOSInfo->TVModes
        || !pBIOSInfo->TVPrintRegs) {

        xf86DestroyI2CDevRec(pBIOSInfo->TVI2CDev, TRUE);

        pBIOSInfo->TVI2CDev = NULL;
        pBIOSInfo->TVOutput = TVOUTPUT_NONE;
        pBIOSInfo->TVEncoder = VIA_NONETV;
        pBIOSInfo->TVI2CDev = NULL;
        pBIOSInfo->TVSave = NULL;
        pBIOSInfo->TVRestore = NULL;
        pBIOSInfo->TVDACSense = NULL;
        pBIOSInfo->TVModeValid = NULL;
        pBIOSInfo->TVModeI2C = NULL;
        pBIOSInfo->TVModeCrtc = NULL;
        pBIOSInfo->TVPower = NULL;
        pBIOSInfo->TVModes = NULL;
        pBIOSInfo->TVPrintRegs = NULL;
        pBIOSInfo->TVNumRegs = 0;

        xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
                   "TV encoder was not properly initialized.\n");
        DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                            "Exiting via_tv_init.\n"));
        return FALSE;
    }

    output = xf86OutputCreate(pScrn, &via_tv_funcs, "TV-1");
    pVia->FirstInit = TRUE;

    if (output) {
        /* Allow tv output on both crtcs, set bit 0 and 1. */
        output->possible_crtcs = 0x3;
    } else {
        xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
                   "Failed to register TV-1.\n");
    }

    pBIOSInfo->tv = output;
    /* Save now */
    pBIOSInfo->TVSave(pScrn);

#ifdef HAVE_DEBUG
    if (VIAPTR(pScrn)->PrintTVRegs)
        pBIOSInfo->TVPrintRegs(pScrn);
#endif

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Exiting via_tv_init.\n"));
    return TRUE;
}

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
viaAnalogSource(ScrnInfoPtr pScrn, CARD8 displaySource)
{
    vgaHWPtr hwp = VGAHWPTR(pScrn);
    CARD8 value = displaySource;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Entered viaAnalogSource.\n"));

    ViaSeqMask(hwp, 0x16, value << 6, 0x40);
    xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                "Analog VGA Output Source: IGA%d\n",
                (value & 0x01) + 1);

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Exiting viaAnalogSource.\n"));
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

static void
via_analog_create_resources(xf86OutputPtr output)
{
}

#ifdef RANDR_12_INTERFACE
static Bool
via_analog_set_property(xf86OutputPtr output, Atom property,
                        RRPropertyValuePtr value)
{
    return TRUE;
}

static Bool
via_analog_get_property(xf86OutputPtr output, Atom property)
{
    return FALSE;
}
#endif

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
    vgaHWPtr hwp = VGAHWPTR(pScrn);
    drmmode_crtc_private_ptr iga = output->crtc->driver_private;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Entered via_analog_mode_set.\n"));

    viaAnalogInit(pScrn);

    if (output->crtc) {
        viaAnalogSource(pScrn, iga->index ? 0x01 : 0x00);
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

static void
via_analog_destroy(xf86OutputPtr output)
{
}

static const xf86OutputFuncsRec via_analog_funcs = {
    .create_resources   = via_analog_create_resources,
#ifdef RANDR_12_INTERFACE
    .set_property       = via_analog_set_property,
#endif
#ifdef RANDR_13_INTERFACE
    .get_property       = via_analog_get_property,
#endif
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
    .destroy            = via_analog_destroy,
};

void
via_analog_init(ScrnInfoPtr pScrn)
{
    VIAPtr pVia = VIAPTR(pScrn);
    VIABIOSInfoPtr pBIOSInfo = pVia->pBIOSInfo;
    xf86OutputPtr output = NULL;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Entered via_analog_init.\n"));

    if (!pVia->pI2CBus1 || !pVia->pI2CBus2) {
        xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
                    "I2C Bus 1 or I2C Bus 2 does not exist.\n");
        DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                            "Exiting via_analog_init.\n"));
        return;
    }

    output = xf86OutputCreate(pScrn, &via_analog_funcs, "VGA-1");

    output->possible_crtcs = 0x3;
    output->possible_clones = 0;
    output->interlaceAllowed = TRUE;
    output->doubleScanAllowed = FALSE;
    pBIOSInfo->analog = output;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Exiting via_analog_init.\n"));
}

static void
via_dvi_create_resources(xf86OutputPtr output)
{
}

#ifdef RANDR_12_INTERFACE
static Bool
via_dvi_set_property(xf86OutputPtr output, Atom property,
                     RRPropertyValuePtr value)
{
    return TRUE;
}

static Bool
via_dvi_get_property(xf86OutputPtr output, Atom property)
{
    return FALSE;
}
#endif

static void
via_dvi_dpms(xf86OutputPtr output, int mode)
{
    ScrnInfoPtr pScrn = output->scrn;

    switch (mode) {
    case DPMSModeOn:
        via_vt1632_power(output, TRUE);
        break;
    case DPMSModeStandby:
    case DPMSModeSuspend:
    case DPMSModeOff:
        via_vt1632_power(output, FALSE);
        break;
    default:
        break;
    }
}

static void
via_dvi_save(xf86OutputPtr output)
{
    via_vt1632_save(output);
}

static void
via_dvi_restore(xf86OutputPtr output)
{
    via_vt1632_restore(output);
}

static int
via_dvi_mode_valid(xf86OutputPtr output, DisplayModePtr pMode)
{
    return via_vt1632_mode_valid(output, pMode);
}

static Bool
via_dvi_mode_fixup(xf86OutputPtr output, DisplayModePtr mode,
                   DisplayModePtr adjusted_mode)
{
    return TRUE;
}

static void
via_dvi_prepare(xf86OutputPtr output)
{
}

static void
via_dvi_commit(xf86OutputPtr output)
{
}

static void
via_dvi_mode_set(xf86OutputPtr output, DisplayModePtr mode,
                 DisplayModePtr adjusted_mode)
{
    ScrnInfoPtr pScrn = output->scrn;
    vgaHWPtr hwp = VGAHWPTR(pScrn);

    via_vt1632_mode_set(output, mode, adjusted_mode);
}

static xf86OutputStatus
via_dvi_detect(xf86OutputPtr output)
{
    xf86OutputStatus status = XF86OutputStatusDisconnected;
    ScrnInfoPtr pScrn = output->scrn;
    VIAPtr pVia = VIAPTR(pScrn);
    ViaVT1632Ptr Private = output->driver_private;
    xf86MonPtr mon;

    /* Check for the DVI presence via VT1632A first before accessing
     * I2C bus. */
    status = via_vt1632_detect(output);
    if (status == XF86OutputStatusConnected) {

        /* Since DVI presence was established, access the I2C bus
         * assigned to DVI. */
        mon = xf86OutputGetEDID(output, Private->VT1632I2CDev->pI2CBus);

        /* Is the interface type digital? */
        if (mon && DIGITAL(mon->features.input_type)) {
            xf86OutputSetEDID(output, mon);
        } else {
            status = XF86OutputStatusDisconnected;
        }
    }

    return status;
}

static void
via_dvi_destroy(xf86OutputPtr output)
{
}

static const xf86OutputFuncsRec via_dvi_funcs = {
    .create_resources   = via_dvi_create_resources,
#ifdef RANDR_12_INTERFACE
    .set_property       = via_dvi_set_property,
#endif
#ifdef RANDR_13_INTERFACE
    .get_property       = via_dvi_get_property,
#endif
    .dpms               = via_dvi_dpms,
    .save               = via_dvi_save,
    .restore            = via_dvi_restore,
    .mode_valid         = via_dvi_mode_valid,
    .mode_fixup         = via_dvi_mode_fixup,
    .prepare            = via_dvi_prepare,
    .commit             = via_dvi_commit,
    .mode_set           = via_dvi_mode_set,
    .detect             = via_dvi_detect,
    .get_modes          = xf86OutputGetEDIDModes,
    .destroy            = via_dvi_destroy,
};

void
via_dvi_init(ScrnInfoPtr pScrn)
{
    VIAPtr pVia = VIAPTR(pScrn);
    xf86OutputPtr output = NULL;
    ViaVT1632Ptr private_data = NULL;
    I2CBusPtr pBus = NULL;
    I2CDevPtr pDev = NULL;
    I2CSlaveAddr addr = 0x10;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Entered via_dvi_init.\n"));

    if (!pVia->pI2CBus2 || !pVia->pI2CBus3) {
        xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
                    "I2C Bus 2 or I2C Bus 3 does not exist.\n");
        DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                    "Exiting via_dvi_init.\n"));
        return;
    }

    if (xf86I2CProbeAddress(pVia->pI2CBus3, addr)) {
        DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_PROBED,
                            "Will probe I2C Bus 3 for a possible "
                            "external TMDS transmitter.\n"));
        pBus = pVia->pI2CBus3;
    } else if (xf86I2CProbeAddress(pVia->pI2CBus2, addr)) {
        DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_PROBED,
                            "Will probe I2C Bus 2 for a possible "
                            "external TMDS transmitter.\n"));
        pBus = pVia->pI2CBus2;
    } else {
        xf86DrvMsg(pScrn->scrnIndex, X_PROBED,
                    "Did not find a possible external TMDS transmitter "
                    "on I2C Bus 2 or I2C Bus 3.\n");
        DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                            "Exiting via_dvi_init.\n"));
        return;
    }

    pDev = xf86CreateI2CDevRec();
    if (!pDev) {
        xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
                    "Failed to create an I2C bus structure.\n");
        DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                            "Exiting via_dvi_init.\n"));
        return;
    }

    pDev->DevName = "VT1632A";
    pDev->SlaveAddr = addr;
    pDev->pI2CBus = pBus;
    if (!xf86I2CDevInit(pDev)) {
        xf86DestroyI2CDevRec(pDev, TRUE);
        xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
                    "Failed to initialize a device on I2C bus.\n");
        DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                            "Exiting via_dvi_init.\n"));
        return;
    }

    if (!via_vt1632_probe(pScrn, pDev)) {
        xf86DestroyI2CDevRec(pDev, TRUE);
        DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                            "Exiting via_dvi_init.\n"));
        return;
    }

    private_data = via_vt1632_init(pScrn, pDev);
    if (!private_data) {
        xf86DestroyI2CDevRec(pDev, TRUE);
        DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                            "Exiting via_dvi_init.\n"));
        return;
    }

    output = xf86OutputCreate(pScrn, &via_dvi_funcs, "DVI-1");
    if (output) {
        output->driver_private = private_data;
        output->possible_crtcs = 0x2;
        output->possible_clones = 0;
        output->interlaceAllowed = FALSE;
        output->doubleScanAllowed = FALSE;
    }

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Exiting via_dvi_init.\n"));
}

/*
 * Reads off the VIA Technologies IGP pin strapping for
 * display detection purposes.
 */
void
viaProbePinStrapping(ScrnInfoPtr pScrn)
{
    vgaHWPtr hwp = VGAHWPTR(pScrn);
    VIAPtr pVia = VIAPTR(pScrn);
    CARD8 sr12, sr13, sr5a;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Entered viaProbePinStrapping.\n"));

    xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                "Probing VIA Technologies IGP pin strapping . . .\n");

    if ((pVia->Chipset == VIA_CX700)
        || (pVia->Chipset == VIA_VX800)
        || (pVia->Chipset == VIA_VX855)
        || (pVia->Chipset == VIA_VX900)) {

        sr5a = hwp->readSeq(hwp, 0x5A);
        DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                            "SR5A: 0x%02X\n", sr5a));
        xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                    "Setting 3C5.5A[0] to 0.\n");
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
    case VIA_KM400:

        /* 3C5.12[4] - FPD17 pin strapping
         *             0: TMDS transmitter (DVI) / capture device
         *             1: Flat panel */
        if (sr12 & 0x10) {
            xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "A flat panel is connected to "
                        "flat panel interface.\n");

            /* 3C5.12[3:0] - FPD16-13 pin strapping
             *               0 ~ 15: Flat panel code defined
             *                       by VIA Technologies */
            xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Detected Flat Panel Type from "
                        "Strapping Pins: %d\n", sr12 & 0x0F);
        } else {
            xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "A TMDS transmitter (DVI) / capture device is "
                        "connected to flat panel interface.\n");
        }

        /* 3C5.12[5] - FPD18 pin strapping
         *             0: TMDS transmitter (DVI)
         *             1: TV encoder */
        if (sr12 & 0x20) {
            xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "A TMDS transmitter (DVI) is connected to "
                        "DVI port.\n");
        } else {
            xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "A TV encoder is connected to "
                        "DVI port.\n");

            /* 3C5.13[4:3] - FPD21-20 pin strapping
             *               00: PAL
             *               01: NTSC
             *               10: PAL-N
             *               11: PAL-NC */
            if (sr13 & 0x04) {
                xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                            "NTSC for the TV encoder.\n");
            } else {
                if (!(sr13 & 0x08)) {
                    xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                                "PAL for the TV encoder.\n");
                } else {
                    xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                                "PAL%s for the TV encoder.\n",
                                sr13 & 0x04 ? "-NC" : "-N");
                }
            }

            /* 3C5.12[6] - FPD19 pin strapping
             *             0: 525 lines (NTSC)
             *             1: 625 lines (PAL) */
            xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "%s lines for the TV encoder.\n",
                        sr12 & 0x40 ? "625" : "525");
        }

        break;

    case VIA_K8M800:
    case VIA_PM800:
    case VIA_P4M800PRO:

        /* 3C5.12[6] - DVP0D6 pin strapping
         *             0: Disable DVP0 (Digital Video Port 0) for
         *                DVI or TV out use
         *             1: Enable DVP0 (Digital Video Port 0) for
         *                DVI or TV out use */
        if (sr12 & 0x40) {

            /* 3C5.12[5] - DVP0D5 pin strapping
             *             0: TMDS transmitter (DVI)
             *             1: TV encoder */
            if (sr12 & 0x20) {
                xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                            "A TV encoder is detected on "
                            "DVP0 (Digital Video Port 0).\n");
            } else {
                xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                            "A TMDS transmitter (DVI) is detected on "
                            "DVP0 (Digital Video Port 0).\n");
            }
        }


        /* 3C5.13[3] - DVP0D8 pin strapping
         *             0: AGP pins are used for AGP
         *             1: AGP pins are used by FPDP
         *             (Flat Panel Display Port) */
        if (sr13 & 0x08) {

            /* 3C5.12[4] - DVP0D4 pin strapping
             *             0: Dual 12-bit FPDP (Flat Panel Display Port)
             *             1: 24-bit FPDP  (Flat Panel Display Port) */
            if (sr12 & 0x10) {
                xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                            "24-bit FPDP (Flat Panel Display Port) "
                            "detected.\n");

                /* 3C5.12[3:0] - DVP0D3-0 pin strapping
                 *               0 ~ 15: Flat panel code defined
                 *                       by VIA Technologies */
                xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                            "Detected Flat Panel Type from "
                            "Strapping Pins: %d\n", sr12 & 0x0F);
            } else {

                /* 3C5.12[6] - DVP0D6 pin strapping
                 *             0: Disable DVP0 (Digital Video Port 0) for
                 *                DVI or TV out use
                 *             1: Enable DVP0 (Digital Video Port 0) for
                 *                DVI or TV out use
                 * 3C5.12[5] - DVP0D5 pin strapping
                 *             0: TMDS transmitter (DVI)
                 *             1: TV encoder */
                if ((!(sr12 & 0x40)) && (!(sr12 & 0x20))) {
                    xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                                "A TV encoder is connected to "
                                "FPDP (Flat Panel Display Port).\n");
                } else {
                    xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                                "Dual 12-bit FPDP (Flat Panel Display Port) "
                                "detected.\n");

                    /* 3C5.12[3:0] - DVP0D3-0 pin strapping
                     *               0 ~ 15: Flat panel code defined
                     *                       by VIA Technologies */
                    xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                                "Detected Flat Panel Type from "
                                "Strapping Pins: %d\n", sr12 & 0x0F);
                }
            }
        }

        break;

    default:
        break;
    }

    if ((pVia->Chipset == VIA_CX700)
        || (pVia->Chipset == VIA_VX800)
        || (pVia->Chipset == VIA_VX855)
        || (pVia->Chipset == VIA_VX900)) {

        xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                    "Setting 3C5.5A[0] to 1.\n");
        ViaSeqMask(hwp, 0x5A, sr5a | 0x01, 0x01);

        sr12 = hwp->readSeq(hwp, 0x12);
        DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                            "SR12: 0x%02X\n", sr12));
        sr13 = hwp->readSeq(hwp, 0x13);
        DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                            "SR13: 0x%02X\n", sr13));

        /* 3C5.13[7:6] - Integrated LVDS / DVI Mode Select
         *               (DVP1D15-14 pin strapping)
         *               00: LVDS1 + LVDS2
         *               01: DVI + LVDS2
         *               10: Dual LVDS Channel (High Resolution Panel)
         *               11: One DVI only (decrease the clock jitter) */
        switch (sr13 & 0xC0) {
        case 0x00:
            xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "LVDS1 + LVDS2 detected.\n");
            break;
        case 0x40:
            xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Single Link DVI + LVDS2 detected.\n");
            break;
        case 0x80:
            xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Dual Channel LVDS detected.\n");
            break;
        case 0xC0:
            xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Single Link DVI detected.\n");
            break;
        default:
            break;
        }

        hwp->writeSeq(hwp, 0x5A, sr5a);

    }

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Exiting viaProbePinStrapping.\n"));
}

void
viaOutputDetect(ScrnInfoPtr pScrn)
{
    VIAPtr pVia = VIAPTR(pScrn);
    VIABIOSInfoPtr pBIOSInfo = pVia->pBIOSInfo;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Entered viaOutputDetect.\n"));

    pBIOSInfo->analog = NULL;

    /* Read off the VIA Technologies IGP pin strapping for
       display detection purposes. */
    viaProbePinStrapping(pScrn);

    /* LVDS */
    via_lvds_init(pScrn);

    /* VGA */
    via_analog_init(pScrn);

    /* TV */
    via_tv_init(pScrn);

    /* External TMDS Transmitter (DVI) */
    via_dvi_init(pScrn);

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Exiting viaOutputDetect.\n"));
}

#ifdef HAVE_DEBUG
/*
 * Returns:
 *   Bit[7] 2nd Path
 *   Bit[6] 1/0 MHS Enable/Disable
 *   Bit[5] 0 = Bypass Callback, 1 = Enable Callback
 *   Bit[4] 0 = Hot-Key Sequence Control (OEM Specific)
 *   Bit[3] LCD
 *   Bit[2] TV
 *   Bit[1] CRT
 *   Bit[0] DVI
 */
static CARD8
VIAGetActiveDisplay(ScrnInfoPtr pScrn)
{
    vgaHWPtr hwp = VGAHWPTR(pScrn);
    CARD8 tmp;

    tmp = (hwp->readCrtc(hwp, 0x3E) >> 4);
    tmp |= ((hwp->readCrtc(hwp, 0x3B) & 0x18) << 3);

    return tmp;
}
#endif /* HAVE_DEBUG */

/*
 *
 */
CARD32
ViaGetMemoryBandwidth(ScrnInfoPtr pScrn)
{
    VIAPtr pVia = VIAPTR(pScrn);

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                     "ViaGetMemoryBandwidth. Memory type: %d\n",
                     pVia->MemClk));

    switch (pVia->MemClk) {
        case VIA_MEM_SDR66:
        case VIA_MEM_SDR100:
        case VIA_MEM_SDR133:
            return VIA_BW_MIN;
        case VIA_MEM_DDR200:
            return VIA_BW_DDR200;
        case VIA_MEM_DDR266:
        case VIA_MEM_DDR333:
        case VIA_MEM_DDR400:
            return VIA_BW_DDR400;
        case VIA_MEM_DDR533:
        case VIA_MEM_DDR667:
            return VIA_BW_DDR667;
        case VIA_MEM_DDR800:
        case VIA_MEM_DDR1066:
            return VIA_BW_DDR1066;
        default:
            xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
                       "ViaBandwidthAllowed: Unknown memory type: %d\n",
                       pVia->MemClk);
            return VIA_BW_MIN;
    }
}

/*
 *
 * Some very common abstractions.
 *
 */

/*
 * Standard vga call really.
 * Needs to be called to reset the dotclock (after SR40:2/1 reset)
 */
void
ViaSetUseExternalClock(vgaHWPtr hwp)
{
    CARD8 data;

    DEBUG(xf86DrvMsg(hwp->pScrn->scrnIndex, X_INFO,
                     "ViaSetUseExternalClock\n"));

    data = hwp->readMiscOut(hwp);
    hwp->writeMiscOut(hwp, data | 0x0C);
}

/*
 *
 */
static void
ViaSetDotclock(ScrnInfoPtr pScrn, CARD32 clock, int base, int probase)
{
    vgaHWPtr hwp = VGAHWPTR(pScrn);
    VIAPtr pVia = VIAPTR(pScrn);

    DEBUG(xf86DrvMsg(hwp->pScrn->scrnIndex, X_INFO,
                     "ViaSetDotclock to 0x%06x\n", (unsigned)clock));

    if ((pVia->Chipset == VIA_CLE266) || (pVia->Chipset == VIA_KM400)) {
        hwp->writeSeq(hwp, base, clock >> 8);
        hwp->writeSeq(hwp, base+1, clock & 0xFF);
    } else {  /* unichrome pro */
        union pllparams pll;
        int dtz, dr, dn, dm;
        pll.packed = clock;
        dtz = pll.params.dtz;
        dr  = pll.params.dr;
        dn  = pll.params.dn;
        dm  = pll.params.dm;

        /* The VX855 and VX900 do not modify dm/dn, but earlier chipsets do. */
        if ((pVia->Chipset != VIA_VX855) && (pVia->Chipset != VIA_VX900)) {
            dm -= 2;
            dn -= 2;
        }

        hwp->writeSeq(hwp, probase, dm & 0xff);
        hwp->writeSeq(hwp, probase+1,
                      ((dm >> 8) & 0x03) | (dr << 2) | ((dtz & 1) << 7));
        hwp->writeSeq(hwp, probase+2, (dn & 0x7f) | ((dtz & 2) << 6));
    }
}

/*
 *
 */
void
ViaSetPrimaryDotclock(ScrnInfoPtr pScrn, CARD32 clock)
{
    vgaHWPtr hwp = VGAHWPTR(pScrn);

    ViaSetDotclock(pScrn, clock, 0x46, 0x44);

    ViaSeqMask(hwp, 0x40, 0x02, 0x02);
    ViaSeqMask(hwp, 0x40, 0x00, 0x02);
}

/*
 *
 */
void
ViaSetSecondaryDotclock(ScrnInfoPtr pScrn, CARD32 clock)
{
    vgaHWPtr hwp = VGAHWPTR(pScrn);

    ViaSetDotclock(pScrn, clock, 0x44, 0x4A);

    ViaSeqMask(hwp, 0x40, 0x04, 0x04);
    ViaSeqMask(hwp, 0x40, 0x00, 0x04);
}

/*
 *
 */
static void
ViaSetECKDotclock(ScrnInfoPtr pScrn, CARD32 clock)
{
    /* Does the non-pro chip have an ECK clock ? */
    ViaSetDotclock(pScrn, clock, 0, 0x47);
}

static CARD32
ViaComputeDotClock(unsigned clock)
{
    double fout, fref, err, minErr;
    CARD32 dr, dn, dm, maxdm, maxdn;
    CARD32 factual, best;

    fref = 14.31818e6;
    fout = (double)clock * 1.e3;

    factual = ~0;
    maxdm = 127;
    maxdn = 7;
    minErr = 1e10;
    best = 0;

    for (dr = 0; dr < 4; ++dr) {
        for (dn = (dr == 0) ? 2 : 1; dn <= maxdn; ++dn) {
            for (dm = 1; dm <= maxdm; ++dm) {
                factual = fref * dm;
                factual /= (dn << dr);
                err = fabs((double)factual / fout - 1.);
                if (err < minErr) {
                    minErr = err;
                    best = (dm & 127) | ((dn & 31) << 8) | (dr << 14);
                }
            }
        }
    }
    return best;
}

static CARD32
ViaComputeProDotClock(unsigned clock)
{
    double fvco, fout, fref, err, minErr;
    CARD32 dr = 0, dn, dm, maxdm, maxdn;
    CARD32 factual;
    union pllparams bestClock;

    fref = 14.318e6;
    fout = (double)clock * 1.e3;

    factual = ~0;
    maxdm = factual / 14318000U;
    minErr = 1.e10;
    bestClock.packed = 0U;

    do {
        fvco = fout * (1 << dr);
    } while (fvco < 300.e6 && dr++ < 8);

    if (dr == 8) {
        return 0;
    }

    if (clock < 30000)
        maxdn = 8;
    else if (clock < 45000)
        maxdn = 7;
    else if (clock < 170000)
        maxdn = 6;
    else
        maxdn = 5;

    for (dn = 2; dn < maxdn; ++dn) {
        for (dm = 2; dm < maxdm; ++dm) {
            factual = 14318000U * dm;
            factual /= dn << dr;
            if ((err = fabs((double)factual / fout - 1.)) < 0.005) {
                if (err < minErr) {
                    minErr = err;
                    bestClock.params.dtz = 1;
                    bestClock.params.dr = dr;
                    bestClock.params.dn = dn;
                    bestClock.params.dm = dm;
                }
            }
        }
    }

    return bestClock.packed;
}

/*
 *
 */
CARD32
ViaModeDotClockTranslate(ScrnInfoPtr pScrn, DisplayModePtr mode)
{
    VIAPtr pVia = VIAPTR(pScrn);
    int i;

    if ((pVia->Chipset == VIA_CLE266) || (pVia->Chipset == VIA_KM400)) {
        CARD32 best1 = 0, best2;

        for (i = 0; ViaDotClocks[i].DotClock; i++)
            if (ViaDotClocks[i].DotClock == mode->Clock) {
                best1 = ViaDotClocks[i].UniChrome;
                break;
            }

        best2 = ViaComputeDotClock(mode->Clock);

        DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                         "ViaComputeDotClock %d : %04x : %04x\n",
                         mode->Clock, (unsigned int)best1,
                         (unsigned int)best2));

        return best2;
    } else {
        for (i = 0; ViaDotClocks[i].DotClock; i++)
            if (ViaDotClocks[i].DotClock == mode->Clock)
                return ViaDotClocks[i].UniChromePro.packed;
        return ViaComputeProDotClock(mode->Clock);
    }

    return 0;
}

void
viaTMDSPower(ScrnInfoPtr pScrn, Bool On)
{

    vgaHWPtr hwp = VGAHWPTR(pScrn);

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Entered viaTMDSPower.\n"));

    if (On) {
        /* Power on TMDS */
        ViaCrtcMask(hwp, 0xD2, 0x00, 0x08);
    } else {
        /* Power off TMDS */
        ViaCrtcMask(hwp, 0xD2, 0x08, 0x08);
    }

    xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                "Integrated TMDS (DVI) Power: %s\n",
                On ? "On" : "Off");

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Exiting viaTMDSPower.\n"));
}

/*
 * Copyright 2005-2007 The Openchrome Project [openchrome.org]
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
    return FALSE;
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

    if (pVia->UseLegacyModeSwitch) {
        VIABIOSInfoPtr pBIOSInfo = pVia->pBIOSInfo;

        if (pBIOSInfo->TVModeValid) {
            ret = pBIOSInfo->TVModeValid(pScrn, pMode);
            if (ret != MODE_OK) {
                xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                            "Mode \"%s\" is not supported by TV encoder.\n",
                            pMode->name);
            }
        }
    } else {
        if (!ViaModeDotClockTranslate(pScrn, pMode))
            return MODE_NOCLOCK;
    }
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

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "ViaDisplayEnableDVO, port: %d\n", port));
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

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "ViaDisplayDisableDVO, port: %d\n", port));
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

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "ViaDisplaySetStreamOnDVO, port: %d\n", port));

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

    if (!pBIOSInfo->TVI2CDev)
        return FALSE;

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
            return FALSE;
            break;
    }

    if (!pBIOSInfo->TVSave || !pBIOSInfo->TVRestore
        || !pBIOSInfo->TVDACSense || !pBIOSInfo->TVModeValid
        || !pBIOSInfo->TVModeI2C || !pBIOSInfo->TVModeCrtc
        || !pBIOSInfo->TVPower || !pBIOSInfo->TVModes
        || !pBIOSInfo->TVPrintRegs) {

        xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
                   "via_tv_init: TVEncoder was not properly initialised.\n");

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

        return FALSE;
    }

    output = xf86OutputCreate(pScrn, &via_tv_funcs, "TV-1");
    pVia->FirstInit = TRUE;

    if (output) {
        /* Allow tv output on both crtcs, set bit 0 and 1. */
        output->possible_crtcs = 0x3;
    } else {
        xf86DrvMsg(pScrn->scrnIndex, X_ERROR, "via_tv_init: Failed to create output for TV-1.\n");
    }

    pBIOSInfo->tv = output;
    /* Save now */
    pBIOSInfo->TVSave(pScrn);

#ifdef HAVE_DEBUG
    if (VIAPTR(pScrn)->PrintTVRegs)
        pBIOSInfo->TVPrintRegs(pScrn);
#endif
    return TRUE;
}

static void
via_dp_create_resources(xf86OutputPtr output)
{
}

#ifdef RANDR_12_INTERFACE
static Bool
via_dp_set_property(xf86OutputPtr output, Atom property,
						RRPropertyValuePtr value)
{
    return FALSE;
}

static Bool
via_dp_get_property(xf86OutputPtr output, Atom property)
{
    return FALSE;
}
#endif

static void
via_dp_dpms(xf86OutputPtr output, int mode)
{
    ScrnInfoPtr pScrn = output->scrn;

    switch (mode) {
    case DPMSModeOn:
        ViaDFPPower(pScrn, TRUE);
        break;

    case DPMSModeStandby:
    case DPMSModeSuspend:
    case DPMSModeOff:
        ViaDFPPower(pScrn, FALSE);
        break;
    }
}

static void
via_dp_save(xf86OutputPtr output)
{
}

static void
via_dp_restore(xf86OutputPtr output)
{
}

static int
via_dp_mode_valid(xf86OutputPtr output, DisplayModePtr pMode)
{
    ScrnInfoPtr pScrn = output->scrn;

    if (!ViaModeDotClockTranslate(pScrn, pMode))
        return MODE_NOCLOCK;
    return MODE_OK;
}

static Bool
via_dp_mode_fixup(xf86OutputPtr output, DisplayModePtr mode,
						DisplayModePtr adjusted_mode)
{
    return TRUE;
}

static void
via_dp_prepare(xf86OutputPtr output)
{
}

static void
via_dp_commit(xf86OutputPtr output)
{
}

static void
via_dp_mode_set(xf86OutputPtr output, DisplayModePtr mode,
				DisplayModePtr adjusted_mode)
{
    ScrnInfoPtr pScrn = output->scrn;

    if (output->crtc) {
        drmmode_crtc_private_ptr iga = output->crtc->driver_private;
        CARD8 value = 0x00; /* Value for IGA 1 */
        vgaHWPtr hwp = VGAHWPTR(pScrn);

        /* IGA 2 */
        if (iga->index)
            value = 0x10;
        ViaSeqMask(hwp, 0x99, value, 0x10);
    }
    ViaDFPPower(pScrn, TRUE);
}

static xf86OutputStatus
via_dp_detect(xf86OutputPtr output)
{
    xf86OutputStatus status = XF86OutputStatusDisconnected;
    ScrnInfoPtr pScrn = output->scrn;
    VIAPtr pVia = VIAPTR(pScrn);
    xf86MonPtr mon;

    mon = xf86OutputGetEDID(output, pVia->pI2CBus2);
    if (mon && DIGITAL(mon->features.input_type)) {
        xf86OutputSetEDID(output, mon);
        DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_PROBED, "DDC pI2CBus2 detected a DP\n"));
        status = XF86OutputStatusConnected;
    }
    return status;
}

static void
via_dp_destroy(xf86OutputPtr output)
{
}

static const xf86OutputFuncsRec via_dp_funcs = {
    .create_resources   = via_dp_create_resources,
#ifdef RANDR_12_INTERFACE
    .set_property       = via_dp_set_property,
    .get_property       = via_dp_get_property,
#endif
    .dpms               = via_dp_dpms,
    .save               = via_dp_save,
    .restore            = via_dp_restore,
    .mode_valid         = via_dp_mode_valid,
    .mode_fixup         = via_dp_mode_fixup,
    .prepare            = via_dp_prepare,
    .commit             = via_dp_commit,
    .mode_set           = via_dp_mode_set,
    .detect             = via_dp_detect,
    .get_modes          = xf86OutputGetEDIDModes,
    .destroy            = via_dp_destroy,
};

void
via_dp_init(ScrnInfoPtr pScrn)
{
    VIAPtr pVia = VIAPTR(pScrn);
    xf86OutputPtr output = NULL;

    if (pVia->pI2CBus2)
        output = xf86OutputCreate(pScrn, &via_dp_funcs, "DP-1");
    if (output) {
        output->possible_crtcs = 0x1;
        output->possible_clones = 0;
        output->interlaceAllowed = TRUE;
        output->doubleScanAllowed = FALSE;
    }
}

/*
 * Enables CRT using DPMS registers.
 */
static void
ViaDisplayEnableCRT(ScrnInfoPtr pScrn)
{
    vgaHWPtr hwp = VGAHWPTR(pScrn);

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "ViaDisplayEnableCRT\n"));
    ViaCrtcMask(hwp, 0x36, 0x00, 0x30);
}

/*
 * Disables CRT using DPMS registers.
 */
static void
ViaDisplayDisableCRT(ScrnInfoPtr pScrn)
{
    vgaHWPtr hwp = VGAHWPTR(pScrn);

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "ViaDisplayDisableCRT\n"));
    ViaCrtcMask(hwp, 0x36, 0x30, 0x30);
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
    return FALSE;
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

    switch (mode) {
    case DPMSModeOn:
        ViaDisplayEnableCRT(pScrn);
        break;

    case DPMSModeStandby:
    case DPMSModeSuspend:
    case DPMSModeOff:
        ViaDisplayDisableCRT(pScrn);
        break;
    }
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

    if (output->crtc) {
        drmmode_crtc_private_ptr iga = output->crtc->driver_private;
        CARD8 value = 0x00; /* Value for IGA 1 */
        vgaHWPtr hwp = VGAHWPTR(pScrn);

        /* IGA 2 */
        if (iga->index)
            value = 0x40;
        ViaSeqMask(hwp, 0x16, value, 0x40);
    }
    ViaDisplayEnableCRT(pScrn);
}

static xf86OutputStatus
via_analog_detect(xf86OutputPtr output)
{
    xf86OutputStatus status = XF86OutputStatusDisconnected;
    ScrnInfoPtr pScrn = output->scrn;
    VIAPtr pVia = VIAPTR(pScrn);
    xf86MonPtr mon;

    mon = xf86OutputGetEDID(output, pVia->pI2CBus1);
    if (mon && !DIGITAL(mon->features.input_type)) {
        xf86OutputSetEDID(output, mon);
        DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_PROBED, "DDC pI2CBus1 detected a CRT\n"));
        status = XF86OutputStatusConnected;
    } else {
        vgaHWPtr hwp = VGAHWPTR(pScrn);
        CARD8 SR01 = hwp->readSeq(hwp, 0x01);
        CARD8 SR40 = hwp->readSeq(hwp, 0x40);
        CARD8 CR36 = hwp->readCrtc(hwp, 0x36);

        DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_PROBED, "Test for CRT with VSYNC\n"));
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

        if (ViaVgahwIn(hwp, 0x3C2) & 0x20)
            status = XF86OutputStatusConnected;

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
    return status;
}

static void
via_analog_destroy(xf86OutputPtr output)
{
}

static const xf86OutputFuncsRec via_analog_funcs = {
    .create_resources	= via_analog_create_resources,
#ifdef RANDR_12_INTERFACE
    .set_property       = via_analog_set_property,
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

    if (pVia->pI2CBus1) {
        output = xf86OutputCreate(pScrn, &via_analog_funcs, "VGA-1");

        output->possible_crtcs = 0x3;
        output->possible_clones = 0;
        output->interlaceAllowed = TRUE;
        output->doubleScanAllowed = FALSE;
        pBIOSInfo->analog = output;
    }
}

/*
 *
 */
void
ViaOutputsDetect(ScrnInfoPtr pScrn)
{
    VIAPtr pVia = VIAPTR(pScrn);
    VIABIOSInfoPtr pBIOSInfo = pVia->pBIOSInfo;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "ViaOutputsDetect\n"));

    pBIOSInfo->analog = NULL;

    /* LVDS */
    via_lvds_init(pScrn);

    /* VGA */
    via_analog_init(pScrn);

    /*
     * FIXME: xf86I2CProbeAddress(pVia->pI2CBus3, 0x40)
     * disables the panel on P4M900
     */
    /* TV encoder */
    if ((pVia->Chipset != VIA_P4M900) || (pVia->ActiveDevice & VIA_DEVICE_TV))
        via_tv_init(pScrn);

    if (pVia->ActiveDevice & VIA_DEVICE_DFP) {
        switch (pVia->Chipset) {
        case VIA_CX700:
        case VIA_VX800:
        case VIA_VX855:
        case VIA_VX900:
            via_dp_init(pScrn);
            break;
        }
    }
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

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "ViaGetMemoryBandwidth. Memory type: %d\n", pVia->MemClk));

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
                       "ViaBandwidthAllowed: Unknown memory type: %d\n", pVia->MemClk);
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
        hwp->writeSeq(hwp, probase+1, ((dm >> 8) & 0x03) | (dr << 2) | ((dtz & 1) << 7));
        hwp->writeSeq(hwp, probase+2, (dn & 0x7f) | ((dtz & 2) << 6));
    }
}

/*
 *
 */
static void
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
                         mode->Clock, (unsigned int)best1, (unsigned int)best2));

        return best2;
    } else {
        for (i = 0; ViaDotClocks[i].DotClock; i++)
            if (ViaDotClocks[i].DotClock == mode->Clock)
                return ViaDotClocks[i].UniChromePro.packed;
        return ViaComputeProDotClock(mode->Clock);
    }

    return 0;
}

/*
 *
 */
void
ViaModePrimaryLegacy(xf86CrtcPtr crtc, DisplayModePtr mode)
{
	ScrnInfoPtr pScrn = crtc->scrn;
	vgaHWPtr hwp = VGAHWPTR(pScrn);
	VIAPtr pVia = VIAPTR(pScrn);
	VIABIOSInfoPtr pBIOSInfo = pVia->pBIOSInfo;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "ViaModePrimaryLegacy\n"));
    DEBUG(ViaPrintMode(pScrn, mode));

    /* Turn off Screen */
    ViaCrtcMask(hwp, 0x17, 0x00, 0x80);

    /* Clean Second Path Status */
    hwp->writeCrtc(hwp, 0x6A, 0x00);
    hwp->writeCrtc(hwp, 0x6B, 0x00);
    hwp->writeCrtc(hwp, 0x6C, 0x00);
    hwp->writeCrtc(hwp, 0x93, 0x00);

    ViaCRTCInit(pScrn);
    ViaFirstCRTCSetMode(pScrn, mode);
    pBIOSInfo->Clock = ViaModeDotClockTranslate(pScrn, mode);
    pBIOSInfo->ClockExternal = FALSE;

    /* Enable MMIO & PCI burst (1 wait state) */
    ViaSeqMask(hwp, 0x1A, 0x06, 0x06);

    if (pBIOSInfo->analog->status == XF86OutputStatusConnected)
        ViaCrtcMask(hwp, 0x36, 0x30, 0x30);
    else
        ViaSeqMask(hwp, 0x16, 0x00, 0x40);

	if ((pBIOSInfo->tv && pBIOSInfo->tv->status == XF86OutputStatusConnected)) {
        /* Quick 'n dirty workaround for non-primary case until TVCrtcMode
         * is removed -- copy from clock handling code below */
        if ((pVia->Chipset == VIA_CLE266) && CLE266_REV_IS_AX(pVia->ChipRev))
            ViaSetPrimaryDotclock(pScrn, 0x471C);  /* CLE266Ax uses 2x XCLK */
        else if (pVia->Chipset != VIA_CLE266 && pVia->Chipset != VIA_KM400)
            ViaSetPrimaryDotclock(pScrn, 0x529001);
        else
            ViaSetPrimaryDotclock(pScrn, 0x871C);
        ViaSetUseExternalClock(hwp);

        ViaTVSetMode(crtc, mode);
    } else
        ViaTVPower(pScrn, FALSE);

    ViaSetPrimaryFIFO(pScrn, mode);

    if (pBIOSInfo->ClockExternal) {
        if ((pVia->Chipset == VIA_CLE266) && CLE266_REV_IS_AX(pVia->ChipRev))
            ViaSetPrimaryDotclock(pScrn, 0x471C);  /* CLE266Ax uses 2x XCLK */
        else if (pVia->Chipset != VIA_CLE266 && pVia->Chipset != VIA_KM400)
            ViaSetPrimaryDotclock(pScrn, 0x529001);
        else
            ViaSetPrimaryDotclock(pScrn, 0x871C);
        if (pVia->Chipset == VIA_CLE266 || pVia->Chipset == VIA_KM400)
            ViaCrtcMask(hwp, 0x6B, 0x01, 0x01);
    } else {
        ViaSetPrimaryDotclock(pScrn, pBIOSInfo->Clock);
        ViaSetUseExternalClock(hwp);
        ViaCrtcMask(hwp, 0x6B, 0x00, 0x01);
    }

    /* Enable CRT Controller (3D5.17 Hardware Reset) */
    ViaCrtcMask(hwp, 0x17, 0x80, 0x80);

    hwp->disablePalette(hwp);
}

/*
 *
 */
void
ViaModeSecondaryLegacy(xf86CrtcPtr crtc, DisplayModePtr mode)
{
	ScrnInfoPtr pScrn = crtc->scrn;
	vgaHWPtr hwp = VGAHWPTR(pScrn);
	VIAPtr pVia = VIAPTR(pScrn);
	VIABIOSInfoPtr pBIOSInfo = pVia->pBIOSInfo;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "ViaModeSecondaryLegacy\n"));
    DEBUG(ViaPrintMode(pScrn, mode));

    /* Turn off Screen */
    ViaCrtcMask(hwp, 0x17, 0x00, 0x80);

    ViaSecondCRTCSetMode(pScrn, mode);

	if (pBIOSInfo->tv && pBIOSInfo->tv->status == XF86OutputStatusConnected)
        ViaTVSetMode(crtc, mode);

    /* CLE266A2 apparently doesn't like this */
    if (!(pVia->Chipset == VIA_CLE266 && pVia->ChipRev == 0x02))
        ViaCrtcMask(hwp, 0x6C, 0x00, 0x1E);

    ViaSetSecondaryFIFO(pScrn, mode);

    ViaSetSecondaryDotclock(pScrn, pBIOSInfo->Clock);
    ViaSetUseExternalClock(hwp);

    ViaCrtcMask(hwp, 0x17, 0x80, 0x80);

    hwp->disablePalette(hwp);
}

void
ViaDFPPower(ScrnInfoPtr pScrn, Bool On)
{
#ifdef HAVE_DEBUG
    if (On)
        xf86DrvMsg(pScrn->scrnIndex, X_INFO, "ViaDFPPower: On.\n");
    else
        xf86DrvMsg(pScrn->scrnIndex, X_INFO, "ViaDFPPower: Off.\n");
#endif
    vgaHWPtr hwp = VGAHWPTR(pScrn);

    /* Display Channel Select */
    ViaCrtcMask(hwp, 0xD2, 0x30, 0x30);

    if (On)
        /* Power on TMDS */
        ViaCrtcMask(hwp, 0xD2, 0x00, 0x08);
    else
        /* Power off TMDS */
        ViaCrtcMask(hwp, 0xD2, 0x08, 0x08);
}

void
ViaModeFirstCRTC(ScrnInfoPtr pScrn, DisplayModePtr mode)
{
    xf86DrvMsg(pScrn->scrnIndex, X_INFO, "ViaModeFirstCRTC\n");
    vgaHWPtr hwp = VGAHWPTR(pScrn);
    VIAPtr pVia = VIAPTR(pScrn);
    VIABIOSInfoPtr pBIOSInfo = pVia->pBIOSInfo;

    /* Turn off Screen */
    ViaCrtcMask(hwp, 0x17, 0x00, 0x80);

    ViaFirstCRTCSetMode(pScrn, mode);
    pBIOSInfo->Clock = ViaModeDotClockTranslate(pScrn, mode);
    pBIOSInfo->ClockExternal = FALSE;

    /* Enable MMIO & PCI burst (1 wait state) */
    switch (pVia->Chipset) {
        case VIA_CLE266:
        case VIA_KM400:
        case VIA_K8M800:
        case VIA_PM800:
        case VIA_VM800:
            ViaSeqMask(hwp, 0x1A, 0x06, 0x06);
            break;
        default:
            ViaSeqMask(hwp, 0x1A, 0x0C, 0x0C);
            break;
    }

    ViaSetPrimaryFIFO(pScrn, mode);

    ViaSetPrimaryDotclock(pScrn, pBIOSInfo->Clock);
    ViaSetUseExternalClock(hwp);
    ViaCrtcMask(hwp, 0x6B, 0x00, 0x01);

    hwp->disablePalette(hwp);

    /* Turn on Screen */
    ViaCrtcMask(hwp, 0x17, 0x80, 0x80);
}

void
ViaModeSecondCRTC(ScrnInfoPtr pScrn, DisplayModePtr mode)
{
    VIAPtr pVia = VIAPTR(pScrn);
    VIABIOSInfoPtr pBIOSInfo = pVia->pBIOSInfo;
    vgaHWPtr hwp = VGAHWPTR(pScrn);
    DisplayModePtr realMode = mode;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "ViaModeSecondCRTC\n"));

    ViaSecondCRTCSetMode(pScrn, realMode);
    ViaSetSecondaryFIFO(pScrn, realMode);
    pBIOSInfo->Clock = ViaModeDotClockTranslate(pScrn, realMode);

    /* Fix LCD scaling */
    ViaSecondCRTCHorizontalQWCount(pScrn, mode->CrtcHDisplay);

    pBIOSInfo->ClockExternal = FALSE;
    ViaSetSecondaryDotclock(pScrn, pBIOSInfo->Clock);
    ViaSetUseExternalClock(hwp);

    hwp->disablePalette(hwp);
}

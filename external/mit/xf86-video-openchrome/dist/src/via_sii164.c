/*
 * Copyright 2016 Kevin Brace
 * Copyright 2016 The OpenChrome Project
 *                [https://www.freedesktop.org/wiki/Openchrome]
 * Copyright 2014 SHS SERVICES GmbH
 * Copyright 2006-2009 Luc Verhaegen.
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
#include "via_sii164.h"

static void
viaSiI164DumpRegisters(ScrnInfoPtr pScrn, I2CDevPtr pDev)
{
    int i;
    CARD8 tmp;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Entered viaSiI164DumpRegisters.\n"));

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "SiI 164: dumping registers:\n"));
    for (i = 0; i <= 0x0f; i++) {
        xf86I2CReadByte(pDev, i, &tmp);
        DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "SiI 164: 0x%02x: 0x%02x\n", i, tmp));
    }

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Exiting viaSiI164DumpRegisters.\n"));
}

static void
viaSiI164InitRegisters(ScrnInfoPtr pScrn, I2CDevPtr pDev)
{
    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Entered viaSiI164InitRegisters.\n"));

    xf86I2CWriteByte(pDev, 0x08,
                        VIA_SII164_VEN | VIA_SII164_HEN |
                        VIA_SII164_DSEL | VIA_SII164_EDGE | VIA_SII164_PDB);

    /* Route receiver detect bit (Offset 0x09[2]) as the output of
     * MSEN pin. */
    xf86I2CWriteByte(pDev, 0x09, 0x20);

    xf86I2CWriteByte(pDev, 0x0A, 0x90);

    xf86I2CWriteByte(pDev, 0x0C, 0x89);

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Exiting viaSiI164InitRegisters.\n"));
}

/*
 * Returns TMDS receiver detection state for Silicon Image SiI 164
 * external TMDS transmitter.
 */
static Bool
viaSiI164Sense(ScrnInfoPtr pScrn, I2CDevPtr pDev)
{
    CARD8 tmp;
    Bool receiverDetected = FALSE;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Entered viaSiI164Sense.\n"));

    xf86I2CReadByte(pDev, 0x09, &tmp);
    if (tmp & 0x04) {
        receiverDetected = TRUE;
    }

    xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                "SiI 164 %s a TMDS receiver.\n",
                receiverDetected ? "detected" : "did not detect");

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Exiting viaSiI164Sense.\n"));
    return receiverDetected;
}

static void
viaSiI164Power(ScrnInfoPtr pScrn, I2CDevPtr pDev, Bool powerState)
{
    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Entered viaSiI164Power.\n"));

    xf86I2CMaskByte(pDev, 0x08, powerState ? 0x01 : 0x00, 0x01);
    xf86DrvMsg(pScrn->scrnIndex, X_INFO, "SiI 164 (DVI) Power: %s\n",
                powerState ? "On" : "Off");

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Exiting viaSiI164Power.\n"));
}

static void
viaSiI164SaveRegisters(ScrnInfoPtr pScrn, I2CDevPtr pDev,
                        viaSiI164RecPtr pSiI164Rec)
{
    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Entered viaSiI164SaveRegisters.\n"));

    xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                "Saving SiI 164 registers.\n");
    xf86I2CReadByte(pDev, 0x08, &pSiI164Rec->Register08);
    xf86I2CReadByte(pDev, 0x09, &pSiI164Rec->Register09);
    xf86I2CReadByte(pDev, 0x0A, &pSiI164Rec->Register0A);
    xf86I2CReadByte(pDev, 0x0C, &pSiI164Rec->Register0C);

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Exiting viaSiI164SaveRegisters.\n"));
}

static void
viaSiI164RestoreRegisters(ScrnInfoPtr pScrn, I2CDevPtr pDev,
                            viaSiI164RecPtr pSiI164Rec)
{
    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Entered viaSiI164RestoreRegisters.\n"));

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Restoring SiI 164 registers.\n"));
    xf86I2CWriteByte(pDev, 0x08, pSiI164Rec->Register08);
    xf86I2CWriteByte(pDev, 0x09, pSiI164Rec->Register09);
    xf86I2CWriteByte(pDev, 0x0A, pSiI164Rec->Register0A);
    xf86I2CWriteByte(pDev, 0x0C, pSiI164Rec->Register0C);

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Exiting viaSiI164RestoreRegisters.\n"));
}

static int
viaSiI164CheckModeValidity(xf86OutputPtr output, DisplayModePtr pMode)
{
    ScrnInfoPtr pScrn = output->scrn;
    viaSiI164RecPtr pSiI164Rec = output->driver_private;
    int status = MODE_OK;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, 
                        "Entered viaSiI164CheckModeValidity.\n"));

    if (pMode->Clock < pSiI164Rec->DotclockMin) {
        status = MODE_CLOCK_LOW;
        goto exit;
    }

    if (pMode->Clock > pSiI164Rec->DotclockMax) {
        status = MODE_CLOCK_HIGH;
    }

exit:
    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Exiting viaSiI164CheckModeValidity.\n"));
    return status;
}

static void
via_sii164_create_resources(xf86OutputPtr output)
{
}

static void
via_sii164_dpms(xf86OutputPtr output, int mode)
{
    ScrnInfoPtr pScrn = output->scrn;
    viaSiI164RecPtr pSiI164Rec = output->driver_private;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Entered via_sii164_dpms.\n"));

    switch (mode) {
    case DPMSModeOn:
        viaSiI164Power(pScrn, pSiI164Rec->SiI164I2CDev, TRUE);
        break;
    case DPMSModeStandby:
    case DPMSModeSuspend:
    case DPMSModeOff:
        viaSiI164Power(pScrn, pSiI164Rec->SiI164I2CDev, FALSE);
        break;
    default:
        break;
    }

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Exiting via_sii164_dpms.\n"));
}

static void
via_sii164_save(xf86OutputPtr output)
{
    ScrnInfoPtr pScrn = output->scrn;
    viaSiI164RecPtr pSiI164Rec = output->driver_private;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Entered via_sii164_save.\n"));

    viaSiI164SaveRegisters(pScrn, pSiI164Rec->SiI164I2CDev, pSiI164Rec);

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Exiting via_sii164_save.\n"));
}

static void
via_sii164_restore(xf86OutputPtr output)
{
    ScrnInfoPtr pScrn = output->scrn;
    viaSiI164RecPtr pSiI164Rec = output->driver_private;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Entered via_sii164_restore.\n"));

    viaSiI164RestoreRegisters(pScrn, pSiI164Rec->SiI164I2CDev,
                                pSiI164Rec);

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Exiting via_sii164_restore.\n"));
}

static int
via_sii164_mode_valid(xf86OutputPtr output, DisplayModePtr pMode)
{
    return viaSiI164CheckModeValidity(output, pMode);
}

static Bool
via_sii164_mode_fixup(xf86OutputPtr output, DisplayModePtr mode,
                   DisplayModePtr adjusted_mode)
{
    return TRUE;
}

static void
via_sii164_prepare(xf86OutputPtr output)
{
}

static void
via_sii164_commit(xf86OutputPtr output)
{
}

static void
via_sii164_mode_set(xf86OutputPtr output, DisplayModePtr mode,
                    DisplayModePtr adjusted_mode)
{
    ScrnInfoPtr pScrn = output->scrn;
    drmmode_crtc_private_ptr iga = output->crtc->driver_private;
    viaSiI164RecPtr pSiI164Rec = output->driver_private;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Entered via_sii164_mode_set.\n"));

    if (output->crtc) {
        viaExtTMDSSetClockDriveStrength(pScrn, 0x03);
        viaExtTMDSSetDataDriveStrength(pScrn, 0x03);
        viaExtTMDSEnableIOPads(pScrn, 0x03);

        viaSiI164DumpRegisters(pScrn, pSiI164Rec->SiI164I2CDev);
        viaSiI164InitRegisters(pScrn, pSiI164Rec->SiI164I2CDev);
        viaSiI164DumpRegisters(pScrn, pSiI164Rec->SiI164I2CDev);

        viaExtTMDSSetDisplaySource(pScrn, iga->index ? 0x01 : 0x00);
    }

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                "Exiting via_sii164_mode_set.\n"));
}

static xf86OutputStatus
via_sii164_detect(xf86OutputPtr output)
{
    xf86MonPtr mon;
    xf86OutputStatus status = XF86OutputStatusDisconnected;
    ScrnInfoPtr pScrn = output->scrn;
    viaSiI164RecPtr pSiI164Rec = output->driver_private;

    /* Check for the DVI presence via SiI 164 first before accessing
     * I2C bus. */
    if (viaSiI164Sense(pScrn, pSiI164Rec->SiI164I2CDev)) {

        /* Since DVI presence was established, access the I2C bus
         * assigned to DVI. */
        mon = xf86OutputGetEDID(output, pSiI164Rec->SiI164I2CDev->pI2CBus);

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

    return status;
}

#ifdef RANDR_12_INTERFACE
static Bool
via_sii164_set_property(xf86OutputPtr output, Atom property,
                     RRPropertyValuePtr value)
{
    return TRUE;
}
#endif

#ifdef RANDR_13_INTERFACE
static Bool
via_sii164_get_property(xf86OutputPtr output, Atom property)
{
    return FALSE;
}
#endif

static void
via_sii164_destroy(xf86OutputPtr output)
{
}

const xf86OutputFuncsRec via_sii164_funcs = {
    .create_resources   = via_sii164_create_resources,
    .dpms               = via_sii164_dpms,
    .save               = via_sii164_save,
    .restore            = via_sii164_restore,
    .mode_valid         = via_sii164_mode_valid,
    .mode_fixup         = via_sii164_mode_fixup,
    .prepare            = via_sii164_prepare,
    .commit             = via_sii164_commit,
    .mode_set           = via_sii164_mode_set,
    .detect             = via_sii164_detect,
    .get_modes          = xf86OutputGetEDIDModes,
#ifdef RANDR_12_INTERFACE
    .set_property       = via_sii164_set_property,
#endif
#ifdef RANDR_13_INTERFACE
    .get_property       = via_sii164_get_property,
#endif
    .destroy            = via_sii164_destroy,
};

Bool
viaSiI164Init(ScrnInfoPtr pScrn, I2CBusPtr pI2CBus)
{
    xf86OutputPtr output;
    VIAPtr pVia = VIAPTR(pScrn);
    viaSiI164RecPtr pSiI164Rec = NULL;
    I2CDevPtr pI2CDevice = NULL;
    I2CSlaveAddr i2cAddr = 0x70;
    CARD8 buf;
    CARD16 vendorID, deviceID;
    Bool status = FALSE;
    char outputNameBuffer[32];

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Entered viaSiI164Init.\n"));

    if (!xf86I2CProbeAddress(pI2CBus, i2cAddr)) {
        xf86DrvMsg(pScrn->scrnIndex, X_PROBED,
                    "I2C device not found.\n");
        goto exit;
    }

    pI2CDevice = xf86CreateI2CDevRec();
    if (!pI2CDevice) {
        xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
                    "Failed to create an I2C bus device record.\n");
        goto exit;
    }

    pI2CDevice->DevName = "SiI 164";
    pI2CDevice->SlaveAddr = i2cAddr;
    pI2CDevice->pI2CBus = pI2CBus;
    if (!xf86I2CDevInit(pI2CDevice)) {
        xf86DestroyI2CDevRec(pI2CDevice, TRUE);
        xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
                    "Failed to initialize a device on I2C bus.\n");
        goto exit;
    }

    xf86I2CReadByte(pI2CDevice, 0, &buf);
    vendorID = buf;
    xf86I2CReadByte(pI2CDevice, 1, &buf);
    vendorID |= buf << 8;
    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Vendor ID: 0x%04x\n", vendorID));

    xf86I2CReadByte(pI2CDevice, 2, &buf);
    deviceID = buf;
    xf86I2CReadByte(pI2CDevice, 3, &buf);
    deviceID |= buf << 8;
    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Device ID: 0x%04x\n", deviceID));

    if ((vendorID != 0x0001) || (deviceID != 0x0006)) {
        xf86DestroyI2CDevRec(pI2CDevice, TRUE);
        xf86DrvMsg(pScrn->scrnIndex, X_PROBED,
                    "SiI 164 external TMDS transmitter not detected.\n");
        goto exit;
    }

    xf86DrvMsg(pScrn->scrnIndex, X_PROBED,
                "SiI 164 external TMDS transmitter detected.\n");

    pSiI164Rec = xnfcalloc(1, sizeof(viaSiI164Rec));
    if (!pSiI164Rec) {
        xf86DestroyI2CDevRec(pI2CDevice, TRUE);
        xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
                    "Failed to allocate working storage for SiI 164.\n");
        goto exit;
    }

    // Remembering which I2C bus is used for SiI 164.
    pSiI164Rec->SiI164I2CDev = pI2CDevice;

    xf86I2CReadByte(pI2CDevice, 0x06, &buf);
    pSiI164Rec->DotclockMin = buf * 1000;

    xf86I2CReadByte(pI2CDevice, 0x07, &buf);
    pSiI164Rec->DotclockMax = (buf + 65) * 1000;

    xf86DrvMsg(pScrn->scrnIndex, X_INFO, "Supported SiI 164 Dot Clock Range: "
                "%d to %d MHz\n",
                pSiI164Rec->DotclockMin / 1000,
                pSiI164Rec->DotclockMax / 1000);

    /* The code to dynamically designate the particular DVI (i.e., DVI-1,
     * DVI-2, etc.) for xrandr was borrowed from xf86-video-r128 DDX. */
    sprintf(outputNameBuffer, "DVI-%d", (pVia->numberDVI + 1));
    output = xf86OutputCreate(pScrn, &via_sii164_funcs, outputNameBuffer);
    if (!output) {
        free(pSiI164Rec);
        xf86DestroyI2CDevRec(pI2CDevice, TRUE);
        xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
                    "Failed to allocate X Server display output record for "
                    "SiI 164.\n");
        goto exit;
    }

    output->driver_private = pSiI164Rec;

    /* Since there are two (2) display controllers registered with the
     * X.Org Server and both IGA1 and IGA2 can handle DVI without any
     * limitations, possible_crtcs should be set to 0x3 (0b11) so that
     * either display controller can get assigned to handle DVI. */
    output->possible_crtcs = (1 << 1) | (1 << 0);

    output->possible_clones = 0;
    output->interlaceAllowed = FALSE;
    output->doubleScanAllowed = FALSE;

    viaSiI164DumpRegisters(pScrn, pI2CDevice);

    pVia->numberDVI++;
    status = TRUE;
exit:
    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Exiting viaSiI164Init.\n"));
    return status;
}

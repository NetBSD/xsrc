/*
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "via_driver.h"
#include "via_vt1632.h"

static Bool
xf86I2CMaskByte(I2CDevPtr d, I2CByte subaddr, I2CByte value, I2CByte mask)
{
    I2CByte tmp;
    Bool ret;

    ret = xf86I2CReadByte(d, subaddr, &tmp);
    if (!ret)
        return FALSE;

    tmp &= ~mask;
    tmp |= (value & mask);

    return xf86I2CWriteByte(d, subaddr, tmp);
}

static void
via_vt1632_dump_registers(ScrnInfoPtr pScrn, I2CDevPtr pDev)
{
    int i;
    CARD8 tmp;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Entered via_vt1632_dump_registers.\n"));

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "VT1632A: dumping registers:\n"));
    for (i = 0; i <= 0x0f; i++) {
        xf86I2CReadByte(pDev, i, &tmp);
        DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, "VT1632A: 0x%02x: 0x%02x\n", i, tmp));
    }

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Exiting via_vt1632_dump_registers.\n"));
}


void
via_vt1632_power(xf86OutputPtr output, BOOL on)
{
    struct ViaVT1632PrivateData * Private = output->driver_private;
    ScrnInfoPtr pScrn = output->scrn;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Entered via_vt1632_power.\n"));

    if (on == TRUE) {
        xf86I2CMaskByte(Private->VT1632I2CDev, 0x08, 0x01, 0x01);
    } else {
        xf86I2CMaskByte(Private->VT1632I2CDev, 0x08, 0x00, 0x01);
    }

    xf86DrvMsg(pScrn->scrnIndex, X_INFO, "VT1632A: Power %s.\n",
                on ? "On" : "Off");
    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Exiting via_vt1632_power.\n"));
}

void
via_vt1632_save(xf86OutputPtr output)
{
    struct ViaVT1632PrivateData * Private = output->driver_private;
    ScrnInfoPtr pScrn = output->scrn;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Entered via_vt1632_save.\n"));

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "VT1632A: Saving the content of registers "
                        "0x08, 0x09, 0x0A, and 0x0C.\n"));
    xf86I2CReadByte(Private->VT1632I2CDev, 0x08, &Private->Register08);
    xf86I2CReadByte(Private->VT1632I2CDev, 0x09, &Private->Register09);
    xf86I2CReadByte(Private->VT1632I2CDev, 0x0A, &Private->Register0A);
    xf86I2CReadByte(Private->VT1632I2CDev, 0x0C, &Private->Register0C);

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Exiting via_vt1632_save.\n"));
}

void
via_vt1632_restore(xf86OutputPtr output)
{
    struct ViaVT1632PrivateData * Private = output->driver_private;
    ScrnInfoPtr pScrn = output->scrn;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Entered via_vt1632_restore.\n"));

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "VT1632A: Restoring registers 0x08, 0x09, "
                        "0x0A, and 0x0C.\n"));
    xf86I2CWriteByte(Private->VT1632I2CDev, 0x08, Private->Register08);
    xf86I2CWriteByte(Private->VT1632I2CDev, 0x09, Private->Register09);
    xf86I2CWriteByte(Private->VT1632I2CDev, 0x0A, Private->Register0A);
    xf86I2CWriteByte(Private->VT1632I2CDev, 0x0C, Private->Register0C);

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Exiting via_vt1632_restore.\n"));
}

int
via_vt1632_mode_valid(xf86OutputPtr output, DisplayModePtr pMode)
{
    struct ViaVT1632PrivateData * Private = output->driver_private;
    ScrnInfoPtr pScrn = output->scrn;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, 
                        "Entered via_vt1632_mode_valid.\n"));

    if (pMode->Clock < Private->DotclockMin) {
        DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                            "Exiting via_vt1632_mode_valid.\n"));
        return MODE_CLOCK_LOW;
    }

    if (pMode->Clock > Private->DotclockMax) {
        DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                            "Exiting via_vt1632_mode_valid.\n"));
        return MODE_CLOCK_HIGH;
    }

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Exiting via_vt1632_mode_valid.\n"));
    return MODE_OK;
}

void
via_vt1632_mode_set(xf86OutputPtr output, DisplayModePtr mode,
                    DisplayModePtr adjusted_mode)
{
    struct ViaVT1632PrivateData * Private = output->driver_private;
    ScrnInfoPtr pScrn = output->scrn;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, 
                        "Entered via_vt1632_mode_set.\n"));

    xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                "VT1632A: Enabling DVI.\n");
    xf86I2CWriteByte(Private->VT1632I2CDev, 0x0C, 0x89);
    xf86I2CWriteByte(Private->VT1632I2CDev, 0x08,
                        VIA_VT1632_VEN | VIA_VT1632_HEN | VIA_VT1632_EDGE |
                        VIA_VT1632_PDB);
    via_vt1632_dump_registers(pScrn, Private->VT1632I2CDev);

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                "Exiting via_vt1632_mode_set.\n"));
}

xf86OutputStatus
via_vt1632_detect(xf86OutputPtr output)
{
    struct ViaVT1632PrivateData * Private = output->driver_private;
    xf86OutputStatus status = XF86OutputStatusDisconnected;
    ScrnInfoPtr pScrn = output->scrn;
    CARD8 tmp;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, 
                        "Entered via_vt1632_detect.\n"));

    xf86I2CReadByte(Private->VT1632I2CDev, 0x09, &tmp);
    if (tmp && 0x02) {
        xf86DrvMsg(pScrn->scrnIndex, X_INFO, 
                    "VT1632A: DVI device is detected.\n");
        status = XF86OutputStatusConnected;
    }

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Exiting via_vt1632_detect.\n"));
    return status;
}

BOOL
via_vt1632_probe(ScrnInfoPtr pScrn, I2CDevPtr pDev) {
    CARD8 buf = 0;
    CARD16 VendorID = 0;
    CARD16 DeviceID = 0;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO, 
                        "Entered via_vt1632_probe.\n"));

    xf86I2CReadByte(pDev, 0, &buf);
    VendorID = buf;
    xf86I2CReadByte(pDev, 1, &buf);
    VendorID |= buf << 8;
    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Vendor ID: 0x%04x\n", VendorID));

    xf86I2CReadByte(pDev, 2, &buf);
    DeviceID = buf;
    xf86I2CReadByte(pDev, 3, &buf);
    DeviceID |= buf << 8;
    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Device ID: 0x%04x\n", DeviceID));

    if ((VendorID != 0x1106) || (DeviceID != 0x3192)) {
        xf86DrvMsg(pScrn->scrnIndex, X_PROBED,
                    "VT1632A DVI transmitter not detected.\n");
        DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                            "Exiting via_vt1632_probe.\n"));
        return FALSE;
    }

    xf86DrvMsg(pScrn->scrnIndex, X_PROBED,
                "VT1632A DVI transmitter detected.\n");
    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Exiting via_vt1632_probe.\n"));
    return TRUE;
}

struct ViaVT1632PrivateData *
via_vt1632_init(ScrnInfoPtr pScrn, I2CDevPtr pDev)
{
    VIAPtr pVia = VIAPTR(pScrn);
    struct ViaVT1632PrivateData * Private = NULL;
    CARD8 buf = 0;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Entered via_vt1632_init.\n"));

    Private = xnfcalloc(1, sizeof(struct ViaVT1632PrivateData));
    if (!Private) {
        xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
                    "Failed to allocate memory for DVI initialization.\n");
        DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                            "Exiting via_vt1632_init.\n"));
        return NULL;
    }
    Private->VT1632I2CDev = pDev;

    xf86I2CReadByte(pDev, 0x06, &buf);
    Private->DotclockMin = buf * 1000;

    xf86I2CReadByte(pDev, 0x07, &buf);
    Private->DotclockMax = (buf + 65) * 1000;

    xf86DrvMsg(pScrn->scrnIndex, X_INFO, "VT1632A Dot Clock Range: "
                "%d to %d MHz\n",
                Private->DotclockMin / 1000,
                Private->DotclockMax / 1000);

    via_vt1632_dump_registers(pScrn, pDev);

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Exiting via_vt1632_init.\n"));
    return Private;
}

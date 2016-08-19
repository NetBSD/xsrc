/*
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

#ifndef _VIA_BIOS_H_
#define _VIA_BIOS_H_ 1

#include "via_vgahw.h"

#define     VIA_PANEL6X4                    0
#define     VIA_PANEL8X6                    1
#define     VIA_PANEL10X7                   2
#define     VIA_PANEL12X7                   3
#define     VIA_PANEL12X10                  4
#define     VIA_PANEL14X10                  5
#define     VIA_PANEL16X12                  6
#define     VIA_PANEL12X8                   7
#define     VIA_PANEL8X4                    8
#define     VIA_PANEL1366X7                 9
#define     VIA_PANEL1360X7                 10
#define     VIA_PANEL1920x1080              11
#define     VIA_PANEL1920x1200              12
#define     VIA_PANEL10X6                   13
#define     VIA_PANEL14X9                   14
#define     VIA_PANEL1280X720               15
#define     VIA_PANEL12X9                   16
#define     VIA_PANEL_INVALID               255

#define     TVTYPE_NONE                     0x00
#define     TVTYPE_NTSC                     0x01
#define     TVTYPE_PAL                      0x02
#define     TVTYPE_480P                     0X03
#define     TVTYPE_576P                     0X04
#define     TVTYPE_720P                     0X05
#define     TVTYPE_1080I                    0X06

#define     TVOUTPUT_NONE                   0x00
#define     TVOUTPUT_COMPOSITE              0x01
#define     TVOUTPUT_SVIDEO                 0x02
#define     TVOUTPUT_RGB                    0x04
#define     TVOUTPUT_YCBCR                  0x08
#define     TVOUTPUT_SC                     0x16

#define  VIA_NONETV   0
#define  VIA_VT1621   1 /* TV2PLUS */
#define  VIA_VT1622   2 /* TV3 */
#define  VIA_VT1623   3 /* also VT1622A */
#define  VIA_VT1625   4
#define  VIA_CH7011   5
#define  VIA_CH7019A  6
#define  VIA_CH7019B  7
#define  VIA_CH7017   8
#define  VIA_CH7304   9
#define  VIA_CH7305   10


#define     VIA_TVNORMAL                    0
#define     VIA_TVOVER                      1

#define     VIA_DEVICE_NONE                 0x00
#define	    VIA_DEVICE_CRT		    0x01
#define	    VIA_DEVICE_LCD		    0x02
#define	    VIA_DEVICE_TV		    0x04
#define	    VIA_DEVICE_DFP		    0x08

#define     VIA_I2C_NONE                    0x00
#define     VIA_I2C_BUS1                    0x01
#define     VIA_I2C_BUS2                    0x02
#define     VIA_I2C_BUS3                    0x04

/* System Memory CLK */
#define VIA_MEM_SDR66   0x00
#define VIA_MEM_SDR100  0x01
#define VIA_MEM_SDR133  0x02
#define VIA_MEM_DDR200  0x03
#define VIA_MEM_DDR266  0x04
#define VIA_MEM_DDR333  0x05
#define VIA_MEM_DDR400  0x06
#define VIA_MEM_DDR533  0x07
#define VIA_MEM_DDR667  0x08
#define VIA_MEM_DDR800  0x09
#define VIA_MEM_DDR1066 0x0A
#define VIA_MEM_END     0x0B
#define VIA_MEM_NONE    0xFF

/* Digital Output Bus Width */
#define	    VIA_DI_12BIT		    0x00
#define	    VIA_DI_24BIT		    0x01

/* Digital Port */
#define     VIA_DI_PORT_NONE        0x0
#define     VIA_DI_PORT_DVP0        0x1
#define     VIA_DI_PORT_DVP1        0x2
#define     VIA_DI_PORT_DFPLOW      0x4
#define     VIA_DI_PORT_DFPHIGH     0x8

typedef struct ViaPanelMode {
    int Width ;
    int Height ;
} ViaPanelModeRec, *ViaPanelModePtr ;

typedef struct ViaPanelInfo {
    Bool IsActive ;
    /* Native physical resolution */
    int NativeHeight;
    int NativeWidth;
    /* Native resolution index, see via_panel.c */
    CARD8 NativeModeIndex;
    /* Determine if we must use the hardware scaler
     * It might be false even if the "Center" option
     * was specified
     */
    Bool            Scale;

    /* Panel/LCD entries */
    CARD16      ResolutionIndex;
    Bool        ForcePanel;
    int         PanelIndex;
    Bool        Center;
    CARD8       BusWidth;		/* Digital Output Bus Width */
    Bool        SetDVI;
    /* LCD Simultaneous Expand Mode HWCursor Y Scale */
    Bool        scaleY;
    int         resY;
} ViaPanelInfoRec, *ViaPanelInfoPtr ;

typedef struct _VIABIOSINFO {
	xf86OutputPtr analog;
	xf86OutputPtr tv;

    CARD32      Clock; /* register value for the dotclock */
    Bool        ClockExternal;
    CARD32      Bandwidth; /* available memory bandwidth */

    /* Simultaneous */
    Bool SimultaneousEnabled;

    /* TV entries */
    int         TVEncoder;
    int         TVOutput;
    I2CDevPtr   TVI2CDev;
    int         TVType;
    Bool        TVDotCrawl;
    int         TVDeflicker;
    CARD8       TVRegs[0xFF];
    int         TVNumRegs;
    int         TVDIPort;

    /* TV Callbacks */
    void (*TVSave) (ScrnInfoPtr pScrn);
    void (*TVRestore) (ScrnInfoPtr pScrn);
    Bool (*TVDACSense) (ScrnInfoPtr pScrn);
    ModeStatus (*TVModeValid) (ScrnInfoPtr pScrn, DisplayModePtr mode);
    void (*TVModeI2C) (ScrnInfoPtr pScrn, DisplayModePtr mode);
    void (*TVModeCrtc) (xf86CrtcPtr crtc, DisplayModePtr mode);
    void (*TVPower) (ScrnInfoPtr pScrn, Bool On);
    void (*LCDPower) (ScrnInfoPtr pScrn, Bool On);
    DisplayModePtr TVModes;
    int            TVNumModes;
    void (*TVPrintRegs) (ScrnInfoPtr pScrn);

} VIABIOSInfoRec, *VIABIOSInfoPtr;

/* via_ums.c */
void VIAUnmapMMIO(ScrnInfoPtr pScrn);
Bool ums_create(ScrnInfoPtr pScrn);
Bool UMSPreInit(ScrnInfoPtr pScrn);
Bool UMSAccelInit(ScreenPtr pScreen);
void viaDisableVQ(ScrnInfoPtr pScrn);

/* via_output.c */
void ViaOutputsDetect(ScrnInfoPtr pScrn);
CARD32 ViaGetMemoryBandwidth(ScrnInfoPtr pScrn);
CARD32 ViaModeDotClockTranslate(ScrnInfoPtr pScrn, DisplayModePtr mode);
void ViaModePrimaryLegacy(xf86CrtcPtr crtc, DisplayModePtr mode);
void ViaModeSecondaryLegacy(xf86CrtcPtr crtc, DisplayModePtr mode);
void ViaDFPPower(ScrnInfoPtr pScrn, Bool On);
void ViaTVPower(ScrnInfoPtr pScrn, Bool On);
void ViaTVSave(ScrnInfoPtr pScrn);
void ViaTVRestore(ScrnInfoPtr pScrn);
#ifdef HAVE_DEBUG
void ViaTVPrintRegs(ScrnInfoPtr pScrn);
#endif
void ViaModeSecondCRTC(ScrnInfoPtr pScrn, DisplayModePtr mode);
void ViaModeFirstCRTC(ScrnInfoPtr pScrn, DisplayModePtr mode);
void ViaSetSecondaryDotclock(ScrnInfoPtr pScrn, CARD32 clock);
void ViaSetUseExternalClock(vgaHWPtr hwp);

/* via_display.c */
Bool UMSCrtcInit(ScrnInfoPtr pScrn);
void ViaCRTCInit(ScrnInfoPtr pScrn);
void ViaFirstCRTCSetStartingAddress(xf86CrtcPtr crtc, int x, int y);
void ViaFirstCRTCSetMode(ScrnInfoPtr pScrn, DisplayModePtr mode);
void ViaSecondCRTCSetStartingAddress(xf86CrtcPtr crtc, int x, int y);
void ViaSecondCRTCHorizontalOffset(ScrnInfoPtr pScrn);
void ViaSecondCRTCHorizontalQWCount(ScrnInfoPtr pScrn, int width);
void ViaSecondCRTCSetMode(ScrnInfoPtr pScrn, DisplayModePtr mode);
void ViaShadowCRTCSetMode(ScrnInfoPtr pScrn, DisplayModePtr mode);
void ViaGammaDisable(ScrnInfoPtr pScrn);

/* via_lvds.c */
void via_lvds_init(ScrnInfoPtr pScrn);

/* in via_bandwidth.c */
void ViaSetPrimaryFIFO(ScrnInfoPtr pScrn, DisplayModePtr mode);
void ViaSetSecondaryFIFO(ScrnInfoPtr pScrn, DisplayModePtr mode);
void ViaDisablePrimaryFIFO(ScrnInfoPtr pScrn);

/* via_vt162x.c */
I2CDevPtr ViaVT162xDetect(ScrnInfoPtr pScrn, I2CBusPtr pBus, CARD8 Address);
void ViaVT162xInit(ScrnInfoPtr pScrn);

/* via_ch7xxx.c */
I2CDevPtr ViaCH7xxxDetect(ScrnInfoPtr pScrn, I2CBusPtr pBus, CARD8 Address);
void ViaCH7xxxInit(ScrnInfoPtr pScrn);

/* via_display.c */
void ViaSecondDisplayChannelEnable(ScrnInfoPtr pScrn);
void ViaSecondDisplayChannelDisable(ScrnInfoPtr pScrn);
void ViaDisplayInit(ScrnInfoPtr pScrn);
void ViaDisplayEnableSimultaneous(ScrnInfoPtr pScrn);
void ViaDisplayDisableSimultaneous(ScrnInfoPtr pScrn);

#endif /* _VIA_BIOS_H_ */

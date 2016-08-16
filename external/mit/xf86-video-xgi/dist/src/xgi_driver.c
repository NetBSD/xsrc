/*
 * XGI driver main code
 *
 * Copyright (C) 2001-2004 by Thomas Winischhofer, Vienna, Austria.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1) Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2) Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3) The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESSED OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Author: Thomas Winischhofer <thomas@winischhofer.net>
 *	- driver entirely rewritten since 2001, only basic structure taken from
 *	  old code (except xgi_dri.c, xgi_shadow.c, xgi_accel.c and parts of
 *	  xgi_dga.c; these were mostly taken over; xgi_dri.c was changed for
 *	  new versions of the DRI layer)
 *
 * This notice covers the entire driver code unless otherwise indicated.
 *
 * Formerly based on code which is
 * 	     Copyright (C) 1998, 1999 by Alan Hourihane, Wigan, England.
 * Written by:
 *           Alan Hourihane <alanh@fairlite.demon.co.uk>,
 *           Mike Chapman <mike@paranoia.com>,
 *           Juanjo Santamarta <santamarta@ctv.es>,
 *           Mitani Hiroshi <hmitani@drl.mei.co.jp>,
 *           David Thomas <davtom@dream.org.uk>.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#define  PACKAGE_VERSION_MAJOR   1
#define  PACKAGE_VERSION_MINOR   6
#define  PACKAGE_VERSION_PATCHLEVEL   0

#include "fb.h"
#include "micmap.h"
#include "xf86.h"
#include "xf86Priv.h"
#include "xf86_OSproc.h"
#if GET_ABI_MAJOR(ABI_VIDEODRV_VERSION) < 6
#include "xf86Resources.h"
#include "xf86RAC.h"
#endif
#include "dixstruct.h"
#include "xorgVersion.h"
#include "xf86PciInfo.h"
#include "xf86Pci.h"
#include "xf86cmap.h"
#include "vgaHW.h"
#include "shadowfb.h"
#include "vbe.h"

#include "mipointer.h"

#include "xgi.h"
#include "xgi_regs.h"
#include "xgi_vb.h"
#include "xgi_dac.h"
#include "vb_def.h"
#include "vb_ext.h"
#include "vb_i2c.h"
#include "vb_setmode.h"
#include "xgi_driver.h"
#include "xgi_accel.h"
#include "valid_mode.h"

#define _XF86DGA_SERVER_
#include <X11/extensions/xf86dgaproto.h>

#include "globals.h"

#ifdef HAVE_XEXTPROTO_71
#include <X11/extensions/dpmsconst.h>
#else
#define DPMS_SERVER
#include <X11/extensions/dpms.h>
#endif


#if defined(XvExtension)
#include "xf86xv.h"
#include <X11/extensions/Xv.h>
#endif

#ifdef XF86DRI
#include "dri.h"
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <fcntl.h>
#include <sys/ioctl.h>

#ifdef XSERVER_LIBPCIACCESS
static Bool XGIPciProbe(DriverPtr drv, int entity_num,
    struct pci_device *dev, intptr_t match_data);
#else
static Bool XGIProbe(DriverPtr drv, int flags);
#endif

void Volari_EnableAccelerator(ScrnInfoPtr pScrn);
/* Globals (yes, these ARE really required to be global) */

#ifdef XGIDUALHEAD
static int XGIEntityIndex = -1;
#endif

/* Jong 09/19/2007; support modeline */
int g_CountOfUserDefinedModes=0;
xf86MonPtr  g_pMonitorDVI=NULL; /* Jong 12/04/2007; used for filtering of CRT1 modes */

/* Jong 07/27/2009; use run-time debug instead except for HW acceleration routines */
/* Set Option "RunTimeDebug" to "true" in X configuration file */
Bool g_bRunTimeDebug=0;

/* Jong@09072009 */
unsigned char g_DVI_I_SignalType = 0x00;

/*
 * This is intentionally screen-independent.  It indicates the binding
 * choice made in the first PreInit.
 */
static int pix24bpp = 0;
int FbDevExist;

#define FBIOGET_FSCREENINFO	0x4602
#define FB_ACCEL_XGI_GLAMOUR	41

struct fb_fix_screeninfo
{
    char id[16];                /* identification string eg "TT Builtin" */
    unsigned long smem_start;   /* Start of frame buffer mem */
    /* (physical address) */
    unsigned long smem_len;     /* Length of frame buffer mem */
    unsigned long type;         /* see FB_TYPE_*                */
    unsigned long type_aux;     /* Interleave for interleaved Planes */
    unsigned long visual;       /* see FB_VISUAL_*              */
    unsigned short xpanstep;    /* zero if no hardware panning  */
    unsigned short ypanstep;    /* zero if no hardware panning  */
    unsigned short ywrapstep;   /* zero if no hardware ywrap    */
    unsigned long line_length;  /* length of a line in bytes    */
    unsigned long mmio_start;   /* Start of Memory Mapped I/O   */
    /* (physical address) */
    unsigned long mmio_len;     /* Length of Memory Mapped I/O  */
    unsigned long accel;        /* Type of acceleration available */
    unsigned short reserved[3]; /* Reserved for future compatibility */
};

#ifdef XSERVER_LIBPCIACCESS
#define XGI_DEVICE_MATCH(d, i) \
    { 0x18ca, (d), PCI_MATCH_ANY, PCI_MATCH_ANY, 0, 0, (i) }

static const struct pci_id_match xgi_device_match[] = {
    XGI_DEVICE_MATCH(PCI_CHIP_XGIXG40, 0),
    XGI_DEVICE_MATCH(PCI_CHIP_XGIXG20, 1),
    XGI_DEVICE_MATCH(PCI_CHIP_XGIXG21, 2),
    XGI_DEVICE_MATCH(PCI_CHIP_XGIXG27, 3),
    { 0, 0, 0 },
};
#endif

/*
 * This contains the functions needed by the server after loading the driver
 * module.  It must be supplied, and gets passed back by the SetupProc
 * function in the dynamic case.  In the static case, a reference to this
 * is compiled in, and this requires that the name of this DriverRec be
 * an upper-case version of the driver name.
 */

DriverRec XGI = {
    XGI_CURRENT_VERSION,
    XGI_DRIVER_NAME,
    XGIIdentify,
#ifdef XSERVER_LIBPCIACCESS
    NULL,
#else
    XGIProbe,
#endif
    XGIAvailableOptions,
    NULL,
    0,
    NULL,

#ifdef XSERVER_LIBPCIACCESS
    xgi_device_match,
    XGIPciProbe
#endif
};

static SymTabRec XGIChipsets[] = {
    {PCI_CHIP_XGIXG40, "Volari V8_V5_V3XT"},
    {PCI_CHIP_XGIXG20, "Volari Z7_Z9_Z9s"},
    {PCI_CHIP_XGIXG21, "Volari Z9_Z9s"},
    {PCI_CHIP_XGIXG27, "Volari Z11"},
    {-1, NULL}
};

static PciChipsets XGIPciChipsets[] = {
    {PCI_CHIP_XGIXG40, PCI_CHIP_XGIXG40, RES_SHARED_VGA},
    {PCI_CHIP_XGIXG20, PCI_CHIP_XGIXG20, RES_SHARED_VGA},
    {PCI_CHIP_XGIXG21, PCI_CHIP_XGIXG21, RES_SHARED_VGA },
    {PCI_CHIP_XGIXG27, PCI_CHIP_XGIXG27, RES_SHARED_VGA },
    {-1, -1, RES_UNDEFINED}
};

static const char *xaaSymbols[] = {
    "XAACopyROP",
    "XAACreateInfoRec",
    "XAADestroyInfoRec",
    "XAAFillMono8x8PatternRects",
    "XAAPatternROP",
    "XAAHelpPatternROP",
    "XAAInit",
    NULL
};

#ifdef XGI_USE_EXA
static const char *exaSymbols[] = {
    "exaGetVersion",
    "exaDriverInit",
    "exaDriverFini",
    "exaOffscreenAlloc",
    "exaOffscreenFree",
    NULL
};
#endif

static const char *vgahwSymbols[] = {
    "vgaHWFreeHWRec",
    "vgaHWGetHWRec",
    "vgaHWGetIOBase",
    "vgaHWGetIndex",
    "vgaHWInit",
    "vgaHWLock",
    "vgaHWMapMem",
    "vgaHWUnmapMem",
    "vgaHWProtect",
    "vgaHWRestore",
    "vgaHWSave",
    "vgaHWSaveScreen",
    "vgaHWUnlock",
    NULL
};

static const char *fbSymbols[] = {
    "fbPictureInit",
    "fbScreenInit",
    NULL
};

static const char *shadowSymbols[] = {
    "ShadowFBInit",
    NULL
};

static const char *ramdacSymbols[] = {
    "xf86CreateCursorInfoRec",
    "xf86DestroyCursorInfoRec",
    "xf86InitCursor",
    NULL
};


static const char *ddcSymbols[] = {
    "xf86PrintEDID",
    "xf86SetDDCproperties",
    "xf86InterpretEDID",
    NULL
};


/* static const char *i2cSymbols[] = {
    "xf86I2CBusInit",
    "xf86CreateI2CBusRec",
    NULL
}; */

static const char *int10Symbols[] = {
    "xf86FreeInt10",
    "xf86InitInt10",
    "xf86ExecX86int10",
    NULL
};

static const char *vbeSymbols[] = {
    "VBEExtendedInit",
    "vbeDoEDID",
    "vbeFree",
    "VBEGetVBEInfo",
    "VBEFreeVBEInfo",
    "VBEGetModeInfo",
    "VBEFreeModeInfo",
    "VBESaveRestore",
    "VBESetVBEMode",
    "VBEGetVBEMode",
    "VBESetDisplayStart",
    "VBESetGetLogicalScanlineLength",
    NULL
};

#ifdef XF86DRI
static const char *drmSymbols[] = {
    "drmAddMap",
    "drmAgpAcquire",
    "drmAgpAlloc",
    "drmAgpBase",
    "drmAgpBind",
    "drmAgpEnable",
    "drmAgpFree",
    "drmAgpGetMode",
    "drmAgpRelease",
    "drmCtlInstHandler",
    "drmGetInterruptFromBusID",
    "drmXGIAgpInit",
    NULL
};

static const char *driSymbols[] = {
    "DRICloseScreen",
    "DRICreateInfoRec",
    "DRIDestroyInfoRec",
    "DRIFinishScreenInit",
    "DRIGetSAREAPrivate",
    "DRILock",
    "DRIQueryVersion",
    "DRIScreenInit",
    "DRIUnlock",
#ifdef XGINEWDRI2
    "GlxSetVisualConfigs",
    "DRICreatePCIBusID",
#endif
    NULL
};
#endif

static MODULESETUPPROTO(xgiSetup);

static XF86ModuleVersionInfo xgiVersRec = {
    XGI_DRIVER_NAME,
    MODULEVENDORSTRING,
    MODINFOSTRING1,
    MODINFOSTRING2,
#ifdef XORG_VERSION_CURRENT
    XORG_VERSION_CURRENT,
#else
    XF86_VERSION_CURRENT,
#endif
    PACKAGE_VERSION_MAJOR, PACKAGE_VERSION_MINOR, PACKAGE_VERSION_PATCHLEVEL,
    ABI_CLASS_VIDEODRV,         /* This is a video driver */
#ifdef ABI_VIDEODRV_VERSION
    ABI_VIDEODRV_VERSION,
#else
    6,
#endif
    MOD_CLASS_VIDEODRV,
    {0, 0, 0, 0}
};

XF86ModuleData xgiModuleData = { &xgiVersRec, xgiSetup, NULL };

/*** static string ***/
#ifdef XGIMERGED
static const char *mergednocrt1 = "CRT1 not detected or forced off. %s.\n";
static const char *mergednocrt2 =
    "No CRT2 output selected or no bridge detected. %s.\n";
static const char *mergeddisstr = "MergedFB mode disabled";
static const char *modesforstr =
    "Modes for CRT%d: *********************************************\n";
static const char *crtsetupstr =
    "------------------------ CRT%d setup -------------------------\n";
#endif

typedef struct
{
    int width, height;
    float VRefresh, HSync, DCLK;
} ModeTiming;

static const ModeTiming establish_timing[] = {
    {800, 600, 60, 37.9, 40},   /* t1 D[0] */
    {800, 600, 56, 35.1, 36},   /* t1 D[1] */
    {640, 480, 75, 37.5, 31.5}, /* t1 D[2] */
    {640, 480, 72, 37.9, 31.5}, /* t1 D[3] */
    {-1, -1, -1, -1},           /* t1 D[4] 640x480@67Hz, ignore */
    {640, 480, 60, 31.5, 25.175},       /* t1 D[5] */
    {-1, -1, -1, -1},           /* t1 D[6] */
    {-1, -1, -1, -1},           /* t1 D[7] */
    {1280, 1024, 75, 80.0, 135},        /* t2 D[0] */
    {1024, 768, 75, 60.0, 78.75},       /* t2 D[1] */
    {1024, 768, 70, 56.5, 75},  /* t2 D[2] */
    {1024, 768, 60, 48.4, 65},  /* t2 D[3] */
    {-1, -1, -1, -1},           /* t2 D[4] 1024x768@87I, ignore */
    {-1, -1, -1, -1},           /* t2 D[5] 832x624@75Hz, ignore */
    {800, 600, 75, 46.9, 49.5}, /* t2 D[6] */
    {800, 600, 72, 48.1, 50}    /* t2 D[7] */
};

static const ModeTiming StdTiming[] = {
    {640, 480, 60, 31.5, 25.175},
    {640, 480, 72, 37.9, 31.5},
    {640, 480, 75, 37.5, 31.5},
    {640, 480, 85, 43.3, 36.0},

    {800, 600, 56, 35.1, 36},
    {800, 600, 60, 37.9, 40},
    {800, 600, 72, 48.1, 50},
    {800, 600, 75, 46.9, 49.5},
    {800, 600, 85, 53.7, 56.25},

    {1024, 768, 43, 35.5, 44.9},
    {1024, 768, 60, 48.4, 65},
    {1024, 768, 70, 56.5, 75},
    {1024, 768, 75, 60, 78.75},
    {1024, 768, 85, 68.7, 94.5},

    {1152, 864, 75, 67.5, 108},

    {1280, 960, 60, 60, 108},
    {1280, 960, 85, 85.9, 148.5},
    {1280, 1024, 60, 64.0, 108},
    {1280, 1024, 75, 80, 135},
    {1280, 1024, 85, 91.1, 157.5},

    {1600, 1200, 60, 75, 162.0},
    {1600, 1200, 65, 81.3, 175.5},
    {1600, 1200, 70, 87.5, 189},
    {1600, 1200, 75, 93.8, 202},
    {1600, 1200, 85, 106.3, 229.5},

    {1792, 1344, 60, 83.64, 204.75},
    {1792, 1344, 75, 106.27, 261},

    {1856, 1392, 60, 86.33, 218.25},
    {1856, 1392, 75, 112.50, 288},

    {1920, 1440, 60, 90, 234},
    {1920, 1440, 75, 112.5, 297},
    {-1, -1, -1, -1, -1},
};


static void XGIDumpPalette(ScrnInfoPtr pScrn);
#ifdef DEBUG
void XGIDumpSR(ScrnInfoPtr pScrn);
void XGIDumpCR(ScrnInfoPtr pScrn);
static void XGIDumpGR(ScrnInfoPtr pScrn);
static void XGIDumpPart1(ScrnInfoPtr pScrn);
static void XGIDumpPart2(ScrnInfoPtr pScrn);
static void XGIDumpPart3(ScrnInfoPtr pScrn);
static void XGIDumpPart4(ScrnInfoPtr pScrn);
static void XGIDumpMMIO(ScrnInfoPtr pScrn);
#endif

static int XGICalcVRate(DisplayModePtr mode);
static unsigned char XGISearchCRT1Rate(ScrnInfoPtr pScrn,
                                       DisplayModePtr mode);
static void xgiSaveUnlockExtRegisterLock(XGIPtr pXGI, unsigned char *reg1,
                                         unsigned char *reg2);
static void xgiRestoreExtRegisterLock(XGIPtr pXGI, unsigned char reg1,
                                      unsigned char reg2);

/* Jong 12/05/2007; check mode with monitor DDC */
static bool XGICheckModeByDDC(DisplayModePtr pMode, xf86MonPtr pMonitorDDC);

/* Jong 12/05/2007; filter mode list by monitor DDC */
static void XGIFilterModeByDDC(DisplayModePtr pModeList, xf86MonPtr pMonitorDDC);

static pointer
xgiSetup(pointer module, pointer opts, int *errmaj, int *errmin)
{
    static Bool setupDone = FALSE;

    if (!setupDone) {
        setupDone = TRUE;
/* Jong@09022009 */
#if (XORG_VERSION_CURRENT > XORG_VERSION_NUMERIC(6,9,0,0,0) )
        xf86AddDriver(&XGI, module, HaveDriverFuncs);
#else
        xf86AddDriver(&XGI, module, 0);
#endif

#if 0
        LoaderRefSymLists(vgahwSymbols, fbSymbols, xaaSymbols,
                          shadowSymbols, ramdacSymbols, ddcSymbols,
                          vbeSymbols, int10Symbols,
#ifdef XF86DRI
                          drmSymbols, driSymbols,
#endif
                          NULL);
#endif
        return (pointer) TRUE;
    }

    if (errmaj)
        *errmaj = LDR_ONCEONLY;
    return NULL;
}


static XGIPtr
XGIGetRec(ScrnInfoPtr pScrn)
{
    /*
     * Allocate an XGIRec, and hook it into pScrn->driverPrivate.
     * pScrn->driverPrivate is initialised to NULL, so we can check if
     * the allocation has already been done.
     */
    if (pScrn->driverPrivate == NULL) {
        XGIPtr pXGI = xnfcalloc(sizeof(XGIRec), 1);

        /* Initialise it to 0 */
        memset(pXGI, 0, sizeof(XGIRec));

        pScrn->driverPrivate = pXGI;
        pXGI->pScrn = pScrn;
    }

    return (XGIPtr) pScrn->driverPrivate;
}

static void
XGIFreeRec(ScrnInfoPtr pScrn)
{
    XGIPtr pXGI = XGIPTR(pScrn);
    XGIEntPtr pXGIEnt = NULL;

    /* Just to make sure... */
    if (!pXGI)
        return;

    pXGIEnt = ENTITY_PRIVATE(pXGI);
    if (pXGIEnt) {
        if (!IS_SECOND_HEAD(pXGI)) {
            /* Free memory only if we are first head; in case of an error
             * during init of the second head, the server will continue -
             * and we need the BIOS image and VB_DEVICE_INFO for the first
             * head.
             */
            if (pXGIEnt->BIOS)
                xfree(pXGIEnt->BIOS);
            pXGIEnt->BIOS = pXGI->BIOS = NULL;
            if (pXGIEnt->XGI_Pr)
                xfree(pXGIEnt->XGI_Pr);
            pXGIEnt->XGI_Pr = pXGI->XGI_Pr = NULL;
            if (pXGIEnt->RenderAccelArray)
                xfree(pXGIEnt->RenderAccelArray);
            pXGIEnt->RenderAccelArray = pXGI->RenderAccelArray = NULL;
        }
        else {
            pXGI->BIOS = NULL;
            pXGI->XGI_Pr = NULL;
            pXGI->RenderAccelArray = NULL;
        }
    }
    else {
        if (pXGI->BIOS)
            xfree(pXGI->BIOS);
        pXGI->BIOS = NULL;
        if (pXGI->XGI_Pr)
            xfree(pXGI->XGI_Pr);
        pXGI->XGI_Pr = NULL;
        if (pXGI->RenderAccelArray)
            xfree(pXGI->RenderAccelArray);
        pXGI->RenderAccelArray = NULL;
    }

#ifdef XGIMERGED
    if (pXGI->MetaModes)
        xfree(pXGI->MetaModes);
    pXGI->MetaModes = NULL;

    if (pXGI->CRT1Modes) {
        if (pXGI->CRT1Modes != pScrn->modes) {
            if (pScrn->modes) {
                pScrn->currentMode = pScrn->modes;
                do {
                    DisplayModePtr p = pScrn->currentMode->next;
                    if (pScrn->currentMode->Private)
                        xfree(pScrn->currentMode->Private);
                    xfree(pScrn->currentMode);
                    pScrn->currentMode = p;
                } while (pScrn->currentMode != pScrn->modes);
            }
            pScrn->currentMode = pXGI->CRT1CurrentMode;
            pScrn->modes = pXGI->CRT1Modes;
            pXGI->CRT1CurrentMode = NULL;
            pXGI->CRT1Modes = NULL;
        }
    }
#endif
    if (pXGI->pVbe)
        vbeFree(pXGI->pVbe);
    pXGI->pVbe = NULL;
    if (pScrn->driverPrivate == NULL)
        return;
    xfree(pScrn->driverPrivate);
    pScrn->driverPrivate = NULL;
}

/* 
	SR1F Power management register
	D7	Force CRT1 into DPMS suspend mode
		0: disable
		1: enable
	D6	Force CRT1 into DPMS stand-by mode
		0: disable
		1: enable
*/
static void
XGIDisplayPowerManagementSet(ScrnInfoPtr pScrn, int PowerManagementMode,
                             int flags)
{
    XGIPtr pXGI = XGIPTR(pScrn);
    BOOLEAN docrt1 = TRUE, docrt2 = TRUE;
    unsigned char sr1 = 0, cr17 = 0, cr63 = 0, sr11 = 0, pmreg = 0, sr7 = 0;
    unsigned char p1_13 = 0, p2_0 = 0, oldpmreg = 0;
    BOOLEAN backlight = TRUE;

    xf86DrvMsgVerb(pScrn->scrnIndex, X_INFO, 3,
                   "XGIDisplayPowerManagementSet(%d)\n", PowerManagementMode);

#if 1
	PVB_DEVICE_INFO pVBInfo = pXGI->XGI_Pr;
    PXGI_HW_DEVICE_INFO pHwDevInfo = &pXGI->xgi_HwDevExt;
	ULONG  PowerState = 0xFFFFFFFF;

	if((PowerManagementMode != 0) && (PowerManagementMode <= 3))
		PowerState = 0x00000001 << (PowerManagementMode + 7);
	else
		PowerState = 0x0;

	XGISetDPMS(pScrn, pVBInfo, pHwDevInfo, PowerState);
#else
    if (IS_DUAL_HEAD(pXGI)) {
        if (IS_SECOND_HEAD(pXGI))
            docrt2 = FALSE;
        else
            docrt1 = FALSE;
    }

#ifdef UNLOCK_ALWAYS
    xgiSaveUnlockExtRegisterLock(pXGI, NULL, NULL);
#endif

    switch (PowerManagementMode) {

    case DPMSModeOn:           /* HSync: On, VSync: On */
		PDEBUG(ErrorF("!-DPMSMode-On...\n"));

        if (docrt1)
            pXGI->Blank = FALSE;

        sr1 = 0x00;
        cr17 = 0x80;
        pmreg = 0x00;
        cr63 = 0x00;
        sr7 = 0x10;
        sr11 = (pXGI->LCDon & 0x0C);
        p2_0 = 0x20;
        p1_13 = 0x00;
        backlight = TRUE;
        break;

    case DPMSModeSuspend:      /* HSync: On, VSync: Off */
		PDEBUG(ErrorF("!-DPMSMode-Suspend...\n"));

        if (docrt1)
            pXGI->Blank = TRUE;

        sr1 = 0x20;
        cr17 = 0x80;
        pmreg = 0x80;
        cr63 = 0x40;
        sr7 = 0x00;
        sr11 = 0x08;
        p2_0 = 0x40;
        p1_13 = 0x80;
        backlight = FALSE;
        break;

    case DPMSModeStandby:      /* HSync: Off, VSync: On */
		PDEBUG(ErrorF("!-DPMSMode-Standby...\n"));

        if (docrt1)
            pXGI->Blank = TRUE;

        sr1 = 0x20;
        cr17 = 0x80;
        pmreg = 0x40;
        cr63 = 0x40;
        sr7 = 0x00;
        sr11 = 0x08;
        p2_0 = 0x80;
        p1_13 = 0x40;
        backlight = FALSE;
        break;

    case DPMSModeOff:          /* HSync: Off, VSync: Off */
		PDEBUG(ErrorF("!-DPMSMode-Off...\n"));

        if (docrt1)
            pXGI->Blank = TRUE;

        sr1 = 0x20;
        cr17 = 0x00;
        pmreg = 0xc0;
        cr63 = 0x40;
        sr7 = 0x00;
        sr11 = 0x08;
        p2_0 = 0xc0;
        p1_13 = 0xc0;
        backlight = FALSE;
        break;

    default:
        return;
    }

    if (docrt1) {
        /* Set/Clear "Display On" bit 
         */
        setXGIIDXREG(XGISR, 0x01, ~0x20, sr1);

        if ((!(pXGI->VBFlags & CRT1_LCDA))
            || (pXGI->XGI_Pr->VBType & VB_XGI301C)) {
            inXGIIDXREG(XGISR, 0x1f, oldpmreg);
            if (!pXGI->CRT1off) {
                setXGIIDXREG(XGISR, 0x1f, 0x3f, pmreg);
            }
        }
        oldpmreg &= 0xc0;
    }

    if ((docrt1) && (pmreg != oldpmreg)
        && ((!(pXGI->VBFlags & CRT1_LCDA))
            || (pXGI->XGI_Pr->VBType & VB_XGI301C))) {
        outXGIIDXREG(XGISR, 0x00, 0x01);        /* Synchronous Reset */
        usleep(10000);
        outXGIIDXREG(XGISR, 0x00, 0x03);        /* End Reset */
    }
#endif
}

typedef struct 
{
    char   name[10];
    unsigned int    DCLK;
    unsigned int    HDisplay;
    unsigned int    HSyncStart;
    unsigned int    HSyncEnd;
    unsigned int    HTotal;
    unsigned int    VDisplay;
    unsigned int    VSyncStart;
    unsigned int    VSyncEnd;
    unsigned int    VTotal;
} XGITimingInfo;

XGITimingInfo ExtraAvailableModeTiming[]=
{
  {"1440x900",
   106470,
   1440, 1520, 1672, 1904,
   900, 901, 904, 932},
  {"1680x1050",
   146250,
   1680, 1784, 1960, 2240,
   1050, 1053, 1059, 1089},
  {"0x0",
   106470,
   1440, 1520, 1672, 1904,
   900, 901, 904, 932}
};

int	  ExtraAvailableModeTimingCount = 1;

void XGIAddAvailableModes(DisplayModePtr availModes)
{
	DisplayModePtr p;
	DisplayModePtr q;
	DisplayModePtr last;
	DisplayModePtr first;
	int	i;

	/* Scan to last node */
	for (q = availModes; q != NULL; q = q->next){
		last = q;
	} 

	/* first = availModes->next; */

	/* Add all modes of ExtraAvailableModeTiming[] */
	for(i=0; /* i < ExtraAvailableModeTimingCount */ xf86NameCmp(ExtraAvailableModeTiming[i].name, "0x0") != 0 ; i++)
	{
		p = xnfcalloc(1, sizeof(DisplayModeRec));

		p->prev = last;
		p->next = NULL;
		last->next = p;

		/*
		first->next->prev = p;
		p->prev = first;
		p->next = first->next;
		first->next = p;
		*/

		p->name = xnfalloc(strlen(ExtraAvailableModeTiming[i].name) + 1);
		p->name = ExtraAvailableModeTiming[i].name;
		p->status = MODE_OK;

		p->type = M_T_CLOCK_CRTC_C /* M_T_BUILTIN */ 	/* M_T_USERDEF */ ;

		p->Clock = ExtraAvailableModeTiming[i].DCLK;
		p->HDisplay = ExtraAvailableModeTiming[i].HDisplay;
		p->HSyncStart = ExtraAvailableModeTiming[i].HSyncStart;
		p->HSyncEnd = ExtraAvailableModeTiming[i].HSyncEnd;
		p->HTotal = ExtraAvailableModeTiming[i].HTotal;

		p->VDisplay = ExtraAvailableModeTiming[i].VDisplay;
		p->VSyncStart = ExtraAvailableModeTiming[i].VSyncStart;
		p->VSyncEnd = ExtraAvailableModeTiming[i].VSyncEnd;
		p->VTotal = ExtraAvailableModeTiming[i].VTotal;

		p->Flags = 5;

		last = p;
	}
}

/* Mandatory */
static void
XGIIdentify(int flags)
{
    xf86PrintChipsets(XGI_NAME, "driver for XGI chipsets", XGIChipsets);
    PDEBUG(ErrorF(" --- XGIIdentify \n"));
}

static void
XGIErrorLog(ScrnInfoPtr pScrn, const char *format, ...)
{
    va_list ap;
    static const char *str =
        "**************************************************\n";

    va_start(ap, format);
    xf86DrvMsg(pScrn->scrnIndex, X_ERROR, "%s", str);
    xf86DrvMsg(pScrn->scrnIndex, X_ERROR, "                      ERROR:\n");
    xf86VDrvMsgVerb(pScrn->scrnIndex, X_ERROR, 1, format, ap);
    va_end(ap);
    xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
               "                  END OF MESSAGE\n");
    xf86DrvMsg(pScrn->scrnIndex, X_ERROR, "%s", str);
}

#ifdef XSERVER_LIBPCIACCESS
static Bool XGIPciProbe(DriverPtr drv, int entity_num,
                        struct pci_device *dev, intptr_t match_data)
{
    ScrnInfoPtr pScrn;


    pScrn = xf86ConfigPciEntity(NULL, 0, entity_num, NULL,
                                NULL, NULL, NULL, NULL, NULL);
    if (pScrn != NULL) {
        XGIPtr pXGI;

        /* Fill in what we can of the ScrnInfoRec */
        pScrn->driverVersion = XGI_CURRENT_VERSION;
        pScrn->driverName = XGI_DRIVER_NAME;
        pScrn->name = XGI_NAME;
        pScrn->Probe = NULL;
        pScrn->PreInit = XGIPreInit;
        pScrn->ScreenInit = XGIScreenInit;
        pScrn->SwitchMode = XGISwitchMode;
        pScrn->AdjustFrame = XGIAdjustFrame;
        pScrn->EnterVT = XGIEnterVT;
        pScrn->LeaveVT = XGILeaveVT;
        pScrn->FreeScreen = XGIFreeScreen;
        pScrn->ValidMode = XGIValidMode;


        pXGI = XGIGetRec(pScrn);
        if (pXGI == NULL) {
            return FALSE;
        }

        pXGI->PciInfo = dev;
    }

    return (pScrn != NULL);
}

#else

/* Mandatory */
static Bool
XGIProbe(DriverPtr drv, int flags)
{
    int i;
    GDevPtr *devSections;
    int *usedChips;
    int numDevSections;
    int numUsed;
    Bool foundScreen = FALSE;

    /*
     * The aim here is to find all cards that this driver can handle,
     * and for the ones not already claimed by another driver, claim the
     * slot, and allocate a ScrnInfoRec.
     *
     * This should be a minimal probe, and it should under no circumstances
     * change the state of the hardware.  Because a device is found, don't
     * assume that it will be used.  Don't do any initialisations other than
     * the required ScrnInfoRec initialisations.  Don't allocate any new
     * data structures.
     *
     */

    /*
     * Next we check, if there has been a chipset override in the config file.
     * For this we must find out if there is an active device section which
     * is relevant, i.e., which has no driver specified or has THIS driver
     * specified.
     */

    if ((numDevSections =
         xf86MatchDevice(XGI_DRIVER_NAME, &devSections)) <= 0) {
        /*
         * There's no matching device section in the config file, so quit
         * now.
         */
        return FALSE;
    }

    PDEBUG(ErrorF(" --- XGIProbe \n"));
    /*
     * We need to probe the hardware first.  We then need to see how this
     * fits in with what is given in the config file, and allow the config
     * file info to override any contradictions.
     */

    /*
     * All of the cards this driver supports are PCI, so the "probing" just
     * amounts to checking the PCI data that the server has already collected.
     */
    if (xf86GetPciVideoInfo() == NULL) {
        /*
         * We won't let anything in the config file override finding no
         * PCI video cards at all.  This seems reasonable now, but we'll see.
         */
        return FALSE;
    }

    numUsed = xf86MatchPciInstances(XGI_NAME, PCI_VENDOR_XGI,
                                    XGIChipsets, XGIPciChipsets, devSections,
                                    numDevSections, drv, &usedChips);

    /* Free it since we don't need that list after this */
    xfree(devSections);
    if (numUsed <= 0)
        return FALSE;

    if (flags & PROBE_DETECT) {
        foundScreen = TRUE;
    }
    else
        for (i = 0; i < numUsed; i++) {
            ScrnInfoPtr pScrn;
#ifdef XGIDUALHEAD
            EntityInfoPtr pEnt;
#endif

            /* Allocate a ScrnInfoRec and claim the slot */
            pScrn = NULL;

            if ((pScrn = xf86ConfigPciEntity(pScrn, 0, usedChips[i],
                                             XGIPciChipsets, NULL, NULL,
                                             NULL, NULL, NULL))) {
                /* Fill in what we can of the ScrnInfoRec */
                pScrn->driverVersion = XGI_CURRENT_VERSION;
                pScrn->driverName = XGI_DRIVER_NAME;
                pScrn->name = XGI_NAME;
                pScrn->Probe = XGIProbe;
                pScrn->PreInit = XGIPreInit;
                pScrn->ScreenInit = XGIScreenInit;
                pScrn->SwitchMode = XGISwitchMode;
                pScrn->AdjustFrame = XGIAdjustFrame;
                pScrn->EnterVT = XGIEnterVT;
                pScrn->LeaveVT = XGILeaveVT;
                pScrn->FreeScreen = XGIFreeScreen;
                pScrn->ValidMode = XGIValidMode;
                foundScreen = TRUE;
            }
#ifdef XGIDUALHEAD
            pEnt = xf86GetEntityInfo(usedChips[i]);

#endif
        }
    xfree(usedChips);

    return foundScreen;
}
#endif


/* Some helper functions for MergedFB mode */

#ifdef XGIMERGED

/* Copy and link two modes form mergedfb mode
 * (Code base taken from mga driver)
 * Copys mode i, links the result to dest, and returns it.
 * Links i and j in Private record.
 * If dest is NULL, return value is copy of i linked to itself.
 * For mergedfb auto-config, we only check the dimension
 * against virtualX/Y, if they were user-provided.
 */
static DisplayModePtr
XGICopyModeNLink(ScrnInfoPtr pScrn, DisplayModePtr dest,
                 DisplayModePtr i, DisplayModePtr j, XGIScrn2Rel srel)
{
    XGIPtr pXGI = XGIPTR(pScrn);
    DisplayModePtr mode;
    int dx = 0, dy = 0;

	ErrorF("XGICopyModeNLink()...Use Virtual Size-1\n");

    if (!((mode = xalloc(sizeof(DisplayModeRec)))))
        return dest;
    memcpy(mode, i, sizeof(DisplayModeRec));
    if (!((mode->Private = xalloc(sizeof(XGIMergedDisplayModeRec))))) {
        xfree(mode);
        return dest;
    }
    ((XGIMergedDisplayModePtr) mode->Private)->CRT1 = i;
    ((XGIMergedDisplayModePtr) mode->Private)->CRT2 = j;
    ((XGIMergedDisplayModePtr) mode->Private)->CRT2Position = srel;
    mode->PrivSize = 0;

    switch (srel) {
    case xgiLeftOf:
    case xgiRightOf:
        if (!(pScrn->display->virtualX)) {
            dx = i->HDisplay + j->HDisplay;
        }
        else {
            dx = min(pScrn->virtualX, i->HDisplay + j->HDisplay);
        }
        dx -= mode->HDisplay;
        if (!(pScrn->display->virtualY)) {
            dy = max(i->VDisplay, j->VDisplay);
        }
        else {
            dy = min(pScrn->virtualY, max(i->VDisplay, j->VDisplay));
        }
        dy -= mode->VDisplay;
        break;
    case xgiAbove:
    case xgiBelow:
        if (!(pScrn->display->virtualY)) {
            dy = i->VDisplay + j->VDisplay;
        }
        else {
            dy = min(pScrn->virtualY, i->VDisplay + j->VDisplay);
        }
        dy -= mode->VDisplay;
        if (!(pScrn->display->virtualX)) {
            dx = max(i->HDisplay, j->HDisplay);
        }
        else {
            dx = min(pScrn->virtualX, max(i->HDisplay, j->HDisplay));
        }
        dx -= mode->HDisplay;
        break;
    case xgiClone:
        if (!(pScrn->display->virtualX)) {
            dx = max(i->HDisplay, j->HDisplay);
        }
        else {
            dx = min(pScrn->virtualX, max(i->HDisplay, j->HDisplay));
        }
        dx -= mode->HDisplay;
        if (!(pScrn->display->virtualY)) {
            dy = max(i->VDisplay, j->VDisplay);
        }
        else {
            dy = min(pScrn->virtualY, max(i->VDisplay, j->VDisplay));
        }
        dy -= mode->VDisplay;
        break;
    }
    mode->HDisplay += dx;
    mode->HSyncStart += dx;
    mode->HSyncEnd += dx;
    mode->HTotal += dx;
    mode->VDisplay += dy;
    mode->VSyncStart += dy;
    mode->VSyncEnd += dy;
    mode->VTotal += dy;
    mode->Clock = 0;

    if (((mode->HDisplay * ((pScrn->bitsPerPixel + 7) / 8) * mode->VDisplay) >
         pXGI->maxxfbmem) || (mode->HDisplay > 4088)
        || (mode->VDisplay > 4096)) {

        xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
                   "Skipped %dx%d, not enough video RAM or beyond hardware specs\n",
                   mode->HDisplay, mode->VDisplay);
        xfree(mode->Private);
        xfree(mode);

        return dest;
    }

#ifdef XGIXINERAMA
    if (srel != xgiClone) {
        pXGI->AtLeastOneNonClone = TRUE;
    }
#endif

    xf86DrvMsg(pScrn->scrnIndex, X_INFO,
               "Merged %dx%d and %dx%d to %dx%d%s\n",
               i->HDisplay, i->VDisplay, j->HDisplay, j->VDisplay,
               mode->HDisplay, mode->VDisplay,
               (srel == xgiClone) ? " (Clone)" : "");

    mode->next = mode;
    mode->prev = mode;

    if (dest) {
        mode->next = dest->next;        /* Insert node after "dest" */
        dest->next->prev = mode;
        mode->prev = dest;
        dest->next = mode;
    }

    return mode;
}

/* Helper function to find a mode from a given name
 * (Code base taken from mga driver)
 */
static DisplayModePtr
XGIGetModeFromName(char *str, DisplayModePtr i)
{
    DisplayModePtr c = i;
    if (!i)
        return NULL;
    do {
        if (strcmp(str, c->name) == 0)
            return c;
        c = c->next;
    } while (c != i);
    return NULL;
}

static DisplayModePtr
XGIFindWidestTallestMode(DisplayModePtr i, Bool tallest)
{
    DisplayModePtr c = i, d = NULL;
    int max = 0;
    if (!i)
        return NULL;
    do {
        if (tallest) {
            if (c->VDisplay > max) {
                max = c->VDisplay;
                d = c;
            }
        }
        else {
            if (c->HDisplay > max) {
                max = c->HDisplay;
                d = c;
            }
        }
        c = c->next;
    } while (c != i);
    return d;
}

static DisplayModePtr
XGIGenerateModeListFromLargestModes(ScrnInfoPtr pScrn,
                                    DisplayModePtr i, DisplayModePtr j,
                                    XGIScrn2Rel srel)
{
#ifdef XGIXINERAMA
    XGIPtr pXGI = XGIPTR(pScrn);
#endif
    DisplayModePtr mode1 = NULL;
    DisplayModePtr mode2 = NULL;
    DisplayModePtr result = NULL;

#ifdef XGIXINERAMA
    pXGI->AtLeastOneNonClone = FALSE;
#endif

    switch (srel) {
    case xgiLeftOf:
    case xgiRightOf:
        mode1 = XGIFindWidestTallestMode(i, FALSE);
        mode2 = XGIFindWidestTallestMode(j, FALSE);
        break;
    case xgiAbove:
    case xgiBelow:
        mode1 = XGIFindWidestTallestMode(i, TRUE);
        mode2 = XGIFindWidestTallestMode(j, TRUE);
        break;
    case xgiClone:
        mode1 = i;
        mode2 = j;
    }

    if (mode1 && mode2) {
        return (XGICopyModeNLink(pScrn, result, mode1, mode2, srel));
    }
    else {
        return NULL;
    }
}

/* Generate the merged-fb mode modelist from metamodes
 * (Code base taken from mga driver)
 */
static DisplayModePtr
XGIGenerateModeListFromMetaModes(ScrnInfoPtr pScrn, char *str,
                                 DisplayModePtr i, DisplayModePtr j,
                                 XGIScrn2Rel srel)
{
#ifdef XGIXINERAMA
    XGIPtr pXGI = XGIPTR(pScrn);
#endif
    char *strmode = str;
    char modename[256];
    Bool gotdash = FALSE;
    XGIScrn2Rel sr;
    DisplayModePtr mode1 = NULL;
    DisplayModePtr mode2 = NULL;
    DisplayModePtr result = NULL;

#ifdef XGIXINERAMA
    pXGI->AtLeastOneNonClone = FALSE;
#endif

    do {
        switch (*str) {
        case 0:
        case '-':
        case ' ':
            if ((strmode != str)) {

                strncpy(modename, strmode, str - strmode);
                modename[str - strmode] = 0;

                if (gotdash) {
                    if (mode1 == NULL)
                        return NULL;
                    mode2 = XGIGetModeFromName(modename, j);
                    if (!mode2) {
                        xf86DrvMsg(pScrn->scrnIndex, X_WARNING,
                                   "Mode \"%s\" is not a supported mode for CRT2\n",
                                   modename);
                        xf86DrvMsg(pScrn->scrnIndex, X_WARNING,
                                   "Skipping metamode \"%s-%s\".\n",
                                   mode1->name, modename);
                        mode1 = NULL;
                    }
                }
                else {
                    mode1 = XGIGetModeFromName(modename, i);
                    if (!mode1) {
                        char *tmps = str;
                        xf86DrvMsg(pScrn->scrnIndex, X_WARNING,
                                   "Mode \"%s\" is not a supported mode for CRT1\n",
                                   modename);
                        gotdash = FALSE;
                        while (*tmps == ' ')
                            tmps++;
                        if (*tmps == '-') {     /* skip the next mode */
                            tmps++;
                            while ((*tmps == ' ') && (*tmps != 0))
                                tmps++; /* skip spaces */
                            while ((*tmps != ' ') && (*tmps != '-')
                                   && (*tmps != 0))
                                tmps++; /* skip modename */
                            strncpy(modename, strmode, tmps - strmode);
                            modename[tmps - strmode] = 0;
                            str = tmps - 1;
                        }
                        xf86DrvMsg(pScrn->scrnIndex, X_WARNING,
                                   "Skipping metamode \"%s\".\n", modename);
                        mode1 = NULL;
                    }
                }
                gotdash = FALSE;
            }
            strmode = str + 1;
            gotdash |= (*str == '-');

            if (*str != 0)
                break;
            /* Fall through otherwise */

        default:
            if (!gotdash && mode1) {
                sr = srel;
                if (!mode2) {
                    mode2 = XGIGetModeFromName(mode1->name, j);
                    sr = xgiClone;
                }
                if (!mode2) {
                    xf86DrvMsg(pScrn->scrnIndex, X_WARNING,
                               "Mode: \"%s\" is not a supported mode for CRT2\n",
                               mode1->name);
                    xf86DrvMsg(pScrn->scrnIndex, X_WARNING,
                               "Skipping metamode \"%s\".\n", modename);
                    mode1 = NULL;
                }
                else {
                    result =
                        XGICopyModeNLink(pScrn, result, mode1, mode2, sr);
                    mode1 = NULL;
                    mode2 = NULL;
                }
            }
            break;

        }

    } while (*(str++) != 0);

    return result;
}

static DisplayModePtr
XGIGenerateModeList(ScrnInfoPtr pScrn, char *str,
                    DisplayModePtr i, DisplayModePtr j, XGIScrn2Rel srel)
{
    if (str != NULL) {
        return (XGIGenerateModeListFromMetaModes(pScrn, str, i, j, srel));
    }
    else {
        xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                   "No MetaModes given, linking %s modes by default\n",
                   (srel == xgiClone) ? "first" :
                   (((srel == xgiLeftOf)
                     || (srel == xgiRightOf)) ? "widest" : "tallest"));
        return (XGIGenerateModeListFromLargestModes(pScrn, i, j, srel));
    }
}

static void
XGIRecalcDefaultVirtualSize(ScrnInfoPtr pScrn)
{
    DisplayModePtr mode, bmode;
    int max;
    static const char *str = "MergedFB: Virtual %s %d\n";

	ErrorF("XGIRecalcDefaultVirtualSize()...Update Virtual Size-1\n");
    if (!(pScrn->display->virtualX)) {
        mode = bmode = pScrn->modes;
        max = 0;
        do {
            if (mode->HDisplay > max)
                max = mode->HDisplay;
            mode = mode->next;
        } while (mode != bmode);
        pScrn->virtualX = max;
        pScrn->displayWidth = max;
		ErrorF("XGIRecalcDefaultVirtualSize()...Update Virtual Size-2-pScrn->virtualX=%d\n", pScrn->virtualX);
        xf86DrvMsg(pScrn->scrnIndex, X_PROBED, str, "width", max);
    }
    if (!(pScrn->display->virtualY)) {
        mode = bmode = pScrn->modes;
        max = 0;
        do {
            if (mode->VDisplay > max)
                max = mode->VDisplay;
            mode = mode->next;
        } while (mode != bmode);
        pScrn->virtualY = max;
        xf86DrvMsg(pScrn->scrnIndex, X_PROBED, str, "height", max);
    }
}

static void
XGIMergedFBSetDpi(ScrnInfoPtr pScrn1, ScrnInfoPtr pScrn2, XGIScrn2Rel srel)
{
    XGIPtr pXGI = XGIPTR(pScrn1);
    MessageType from = X_DEFAULT;
    xf86MonPtr DDC1 = (xf86MonPtr) (pScrn1->monitor->DDC);
    xf86MonPtr DDC2 = (xf86MonPtr) (pScrn2->monitor->DDC);
    int ddcWidthmm = 0, ddcHeightmm = 0;
    const char *dsstr = "MergedFB: Display dimensions: (%d, %d) mm\n";

	ErrorF("XGIMergedFBSetDpi()...Use Virtual Size -1\n");

    /* This sets the DPI for MergedFB mode. The problem is that
     * this can never be exact, because the output devices may
     * have different dimensions. This function tries to compromise
     * through a few assumptions, and it just calculates an average DPI
     * value for both monitors.
     */

    /* Given DisplaySize should regard BOTH monitors */
    pScrn1->widthmm = pScrn1->monitor->widthmm;
    pScrn1->heightmm = pScrn1->monitor->heightmm;

    /* Get DDC display size; if only either CRT1 or CRT2 provided these,
     * assume equal dimensions for both, otherwise add dimensions
     */
    if ((DDC1 && (DDC1->features.hsize > 0 && DDC1->features.vsize > 0)) &&
        (DDC2 && (DDC2->features.hsize > 0 && DDC2->features.vsize > 0))) {
        ddcWidthmm = max(DDC1->features.hsize, DDC2->features.hsize) * 10;
        ddcHeightmm = max(DDC1->features.vsize, DDC2->features.vsize) * 10;
        switch (srel) {
        case xgiLeftOf:
        case xgiRightOf:
            ddcWidthmm = (DDC1->features.hsize + DDC2->features.hsize) * 10;
            break;
        case xgiAbove:
        case xgiBelow:
            ddcHeightmm = (DDC1->features.vsize + DDC2->features.vsize) * 10;
        default:
            break;
        }
    }
    else if (DDC1 && (DDC1->features.hsize > 0 && DDC1->features.vsize > 0)) {
        ddcWidthmm = DDC1->features.hsize * 10;
        ddcHeightmm = DDC1->features.vsize * 10;
        switch (srel) {
        case xgiLeftOf:
        case xgiRightOf:
            ddcWidthmm *= 2;
            break;
        case xgiAbove:
        case xgiBelow:
            ddcHeightmm *= 2;
        default:
            break;
        }
    }
    else if (DDC2 && (DDC2->features.hsize > 0 && DDC2->features.vsize > 0)) {
        ddcWidthmm = DDC2->features.hsize * 10;
        ddcHeightmm = DDC2->features.vsize * 10;
        switch (srel) {
        case xgiLeftOf:
        case xgiRightOf:
            ddcWidthmm *= 2;
            break;
        case xgiAbove:
        case xgiBelow:
            ddcHeightmm *= 2;
        default:
            break;
        }
    }

    if (monitorResolution > 0) {

        /* Set command line given values (overrules given options) */
        pScrn1->xDpi = monitorResolution;
        pScrn1->yDpi = monitorResolution;
        from = X_CMDLINE;

    }
    else if (pXGI->MergedFBXDPI) {

        /* Set option-wise given values (overrule DisplaySize) */
        pScrn1->xDpi = pXGI->MergedFBXDPI;
        pScrn1->yDpi = pXGI->MergedFBYDPI;
        from = X_CONFIG;

    }
    else if (pScrn1->widthmm > 0 || pScrn1->heightmm > 0) {

        /* Set values calculated from given DisplaySize */
        from = X_CONFIG;
        if (pScrn1->widthmm > 0) {
            pScrn1->xDpi =
                (int) ((double) pScrn1->virtualX * 25.4 / pScrn1->widthmm);
        }
        if (pScrn1->heightmm > 0) {
            pScrn1->yDpi =
                (int) ((double) pScrn1->virtualY * 25.4 / pScrn1->heightmm);
        }
        xf86DrvMsg(pScrn1->scrnIndex, from, dsstr, pScrn1->widthmm,
                   pScrn1->heightmm);

    }
    else if (ddcWidthmm && ddcHeightmm) {

        /* Set values from DDC-provided display size */
        from = X_PROBED;
        xf86DrvMsg(pScrn1->scrnIndex, from, dsstr, ddcWidthmm, ddcHeightmm);
        pScrn1->widthmm = ddcWidthmm;
        pScrn1->heightmm = ddcHeightmm;
        if (pScrn1->widthmm > 0) {
            pScrn1->xDpi =
                (int) ((double) pScrn1->virtualX * 25.4 / pScrn1->widthmm);
        }
        if (pScrn1->heightmm > 0) {
            pScrn1->yDpi =
                (int) ((double) pScrn1->virtualY * 25.4 / pScrn1->heightmm);
        }

    }
    else {

        pScrn1->xDpi = pScrn1->yDpi = DEFAULT_DPI;

    }

    /* Sanity check */
    if (pScrn1->xDpi > 0 && pScrn1->yDpi <= 0)
        pScrn1->yDpi = pScrn1->xDpi;
    if (pScrn1->yDpi > 0 && pScrn1->xDpi <= 0)
        pScrn1->xDpi = pScrn1->yDpi;

    pScrn2->xDpi = pScrn1->xDpi;
    pScrn2->yDpi = pScrn1->yDpi;

    xf86DrvMsg(pScrn1->scrnIndex, from, "MergedFB: DPI set to (%d, %d)\n",
               pScrn1->xDpi, pScrn1->yDpi);
}

static void
XGIFreeCRT2Structs(XGIPtr pXGI)
{
    if (pXGI->CRT2pScrn) {
        if (pXGI->CRT2pScrn->modes) {
            while (pXGI->CRT2pScrn->modes)
                xf86DeleteMode(&pXGI->CRT2pScrn->modes,
                               pXGI->CRT2pScrn->modes);
        }
        if (pXGI->CRT2pScrn->monitor) {
            if (pXGI->CRT2pScrn->monitor->Modes) {
                while (pXGI->CRT2pScrn->monitor->Modes)
                    xf86DeleteMode(&pXGI->CRT2pScrn->monitor->Modes,
                                   pXGI->CRT2pScrn->monitor->Modes);
            }
            if (pXGI->CRT2pScrn->monitor->DDC)
                xfree(pXGI->CRT2pScrn->monitor->DDC);
            xfree(pXGI->CRT2pScrn->monitor);
        }
        xfree(pXGI->CRT2pScrn);
        pXGI->CRT2pScrn = NULL;
    }
}

#endif /* End of MergedFB helpers */

static xf86MonPtr
XGIInternalDDC(ScrnInfoPtr pScrn, int crtno)
{
    XGIPtr pXGI = XGIPTR(pScrn);
    unsigned char buffer[256];

    int RealOff;
    unsigned char *page;

    xf86MonPtr pMonitor = NULL;
    xf86Int10InfoPtr pInt = NULL;       /* Our int10 */

	/*yilin 03/10/2008: set the monitor default size to 310mm x 240mm to fix KDE font too small problem*/
	pScrn->monitor->widthmm = 310;
	pScrn->monitor->heightmm = 240;

    static char *crtno_means_str[] = {
        "CRT1", "DVI", "CRT2"
    };

    if (crtno > 2 || crtno < 0) {
        xf86DrvMsg(pScrn->scrnIndex, X_WARNING,
                   "XGIInternalDDC(): Can not get EDID for crtno = %d,abort.\n",
                   crtno);
    }
    else {
        xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                   "XGIInternalDDC(): getting EDID for %s.\n",
                   crtno_means_str[crtno]);
    }

/* Jong 08/03/2009; get EDID with I2C function instead of VBIOS call */
#if 1 
    ErrorF("get EDID with I2C function instead of VBIOS call...\n");

	PXGI_HW_DEVICE_INFO pHwDevInfo = &pXGI->xgi_HwDevExt;
    PUCHAR pjEDIDBuffer = buffer;
    ULONG  ulBufferSize = 256;

	pHwDevInfo->crtno = crtno;
	int bEDID = bGetEDID(pHwDevInfo, crtno, pjEDIDBuffer, ulBufferSize);

#else
    ErrorF("get EDID with VBIOS call...\n");
    if (xf86LoadSubModule(pScrn, "int10")) 
	{
#if 0
        xf86LoaderReqSymLists(int10Symbols, NULL);
#endif
        pInt = xf86InitInt10(pXGI->pEnt->index);
        if (pInt == NULL) {
            xf86DrvMsg(pScrn->scrnIndex, X_WARNING,
                       "XGIInternalDDC(): Can not initialize pInt, abort.\n");
            return NULL;
        }

        page = xf86Int10AllocPages(pInt, 1, &RealOff);
        if (page == NULL) {
            xf86FreeInt10(pInt);
            xf86DrvMsg(pScrn->scrnIndex, X_WARNING,
                       "XGIInternalDDC(): Can not initialize real mode buffer, abort.\n");
            return NULL;
        }
    }

    if (pInt) 
	{
        pInt->ax = 0x4f15;      /* VESA DDC supporting */
        pInt->bx = 1;           /* get EDID */
        pInt->cx = crtno;       /* port 0 or 1 for CRT 1 or 2 */
        pInt->es = SEG_ADDR(RealOff);
        pInt->di = SEG_OFF(RealOff);
        pInt->num = 0x10;
        xf86ExecX86int10(pInt);

        PDEBUG3(ErrorF
                ("ax = %04X bx = %04X cx = %04X dx = %04X si = %04X di = %04X es = %04X\n",
                 pInt->ax, pInt->bx, pInt->cx, pInt->dx, pInt->si, pInt->di,
                 pInt->es));
#endif

#if 0
        if ((pInt->ax & 0xff00) == 0) 
		{
            int i;

            for (i = 0; i < 128; i++) {
                buffer[i] = page[i];
            }
#else /* Jong 08/03/2009; get EDID with I2C function instead of VBIOS call */
		if(bEDID == 1)
		{
            int i;
#endif
            xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                       "XGIInternalDDC(): VESA get DDC success for output channel %d.\n",
                       crtno + 1);

            for (i = 0; i < 128; i += 16) {
                unsigned j;
                ErrorF("EDID[%02X]", i);
                for (j = 0; j < 16; j++) {
                    ErrorF(" %02X", buffer[i + j]);
                }
                ErrorF("\n");
            }

			g_DVI_I_SignalType = (buffer[20] & 0x80) >> 7;
			ErrorF("DVI-I : %s signal ...\n", (g_DVI_I_SignalType == 0x01) ? "DVI" : "CRT" );
#if 0
            xf86LoaderReqSymLists(ddcSymbols, NULL);
#endif
			/* Jong 09/04/2007; Alan fixed abnormal EDID data */
			/* pMonitor = xf86InterpretEDID(pScrn->scrnIndex, buffer) ; */
			if ( (buffer[0]==0) && (buffer[7]==0) )
            {
                for (i=1;i<7;i++)
                {
                    if (buffer[i]!=0xFF)
                        break;
                }
                if (i==7)
                {
                    pMonitor = xf86InterpretEDID(pScrn->scrnIndex, buffer);
                }
            }

            if (pMonitor == NULL) {
                xf86DrvMsg(pScrn->scrnIndex, X_WARNING,
                           "CRT%d DDC EDID corrupt\n", crtno + 1);
                return (NULL);
            }
            xf86UnloadSubModule("ddc");
        }
        else {
            xf86DrvMsg(pScrn->scrnIndex, X_WARNING,
                       "XGIInternalDDC(): VESA get DDC fail for output channel %d.\n",
                       crtno + 1);
        }

/* Jong 08/03/2009; get EDID with I2C function instead of VBIOS call */
#if 0
        xf86Int10FreePages(pInt, page, 1);
        xf86FreeInt10(pInt);
    }
#endif

    return pMonitor;
}

/* static xf86MonPtr
XGIDoPrivateDDC(ScrnInfoPtr pScrn, int *crtnum)
{
    XGIPtr pXGI = XGIPTR(pScrn);

    if(IS_DUAL_HEAD(pXGI)) 
    {
       if(IS_SECOND_HEAD(pXGI)) 
       {
          *crtnum = 1;
      return(XGIInternalDDC(pScrn, 0));
       }
       else 
       {
          *crtnum = 2;
      return(XGIInternalDDC(pScrn, 1));
       }
    }
    else if(pXGI->CRT1off) 
    {
       *crtnum = 2;
       return(XGIInternalDDC(pScrn, 1));
    }
    else 
    {
       *crtnum = 1;
       return(XGIInternalDDC(pScrn, 0));
    }
} */


#ifdef DEBUG5
static void
XGIDumpMonitorInfo(xf86MonPtr pMonitor)
{
    struct detailed_timings *pd_timings;
    Uchar *pserial;
    Uchar *pascii_data;
    Uchar *pname;
    struct monitor_ranges *pranges;
    struct std_timings *pstd_t;
    struct whitePoints *pwp;
    int i, j;

    if (pMonitor == NULL) {
        ErrorF("Monitor is NULL");
        return;
    }

    ErrorF("pMonitor->scrnIndex = %d\n", pMonitor->scrnIndex);
    ErrorF
        ("vendor = %c%c%c%c, prod_id = %x serial = %d week = %d year = %d\n",
         pMonitor->vendor.name[0], pMonitor->vendor.name[1],
         pMonitor->vendor.name[2], pMonitor->vendor.name[3],
         pMonitor->vendor.prod_id, pMonitor->vendor.serial,
         pMonitor->vendor.week, pMonitor->vendor.year);

    ErrorF("ver = %d %d\n", pMonitor->ver.version, pMonitor->ver.revision);
    ErrorF("intput type = %d voltage = %d setup = %d sync = %d\n",
           pMonitor->features.input_type,
           pMonitor->features.input_voltage,
           pMonitor->features.input_setup, pMonitor->features.input_sync);
    ErrorF("hsize = %d vsize = %d gamma=%8.3f\n",
           pMonitor->features.hsize,
           pMonitor->features.vsize, pMonitor->features.gamma);

    ErrorF("dpms = %d display_type = %d msc = %d\n",
           pMonitor->features.dpms,
           pMonitor->features.display_type, pMonitor->features.msc);
    ErrorF
        ("redx,redy,greenx,greeny,bluex,bluey,whitex,whitey = %8.3f,%8.3f,%8.3f,%8.3f,%8.3f,%8.3f,%8.3f,%8.3f\n",
         pMonitor->features.redx, pMonitor->features.redy,
         pMonitor->features.greenx, pMonitor->features.greeny,
         pMonitor->features.bluex, pMonitor->features.bluey,
         pMonitor->features.whitex, pMonitor->features.whitey);

    ErrorF("established_timings = (t1)%d%d%d%d%d%d%d%d",
           (pMonitor->timings1.t1 >> 7) & 1,
           (pMonitor->timings1.t1 >> 6) & 1,
           (pMonitor->timings1.t1 >> 5) & 1,
           (pMonitor->timings1.t1 >> 4) & 1,
           (pMonitor->timings1.t1 >> 3) & 1,
           (pMonitor->timings1.t1 >> 2) & 1,
           (pMonitor->timings1.t1 >> 1) & 1,
           (pMonitor->timings1.t1 >> 0) & 1);
    ErrorF("(t2) %d%d%d%d%d%d%d%d",
           (pMonitor->timings1.t1 >> 7) & 1,
           (pMonitor->timings1.t1 >> 6) & 1,
           (pMonitor->timings1.t1 >> 5) & 1,
           (pMonitor->timings1.t1 >> 4) & 1,
           (pMonitor->timings1.t1 >> 3) & 1,
           (pMonitor->timings1.t1 >> 2) & 1,
           (pMonitor->timings1.t1 >> 1) & 1,
           (pMonitor->timings1.t1 >> 0) & 1);
    ErrorF("(t_manu)%d%d%d%d%d%d%d%d\n",
           (pMonitor->timings1.t_manu >> 7) & 1,
           (pMonitor->timings1.t_manu >> 6) & 1,
           (pMonitor->timings1.t_manu >> 5) & 1,
           (pMonitor->timings1.t_manu >> 4) & 1,
           (pMonitor->timings1.t_manu >> 3) & 1,
           (pMonitor->timings1.t_manu >> 2) & 1,
           (pMonitor->timings1.t_manu >> 1) & 1,
           (pMonitor->timings1.t_manu >> 0) & 1);

    for (i = 0; i < 7; i++) {
        ErrorF
            ("std timing %d: hsize = %d, vsize = %d, refresh = %d, id = %d\n",
             i, pMonitor->timings2[i].hsize, pMonitor->timings2[i].vsize,
             pMonitor->timings2[i].refresh, pMonitor->timings2[i].id);
    }

    for (i = 0; i < 4; i++) {
        ErrorF("Detail timing section %d\n", i);
        ErrorF("type = %x\n", pMonitor->det_mon[i].type);
        switch (pMonitor->det_mon[i].type) {
        case DS_SERIAL:
            ErrorF("type = %x DS_SERIAL = %x\n", pMonitor->det_mon[i].type,
                   DS_SERIAL);
            break;
        case DS_ASCII_STR:
            ErrorF("type = %x DS_ASCII_STR = %x\n", pMonitor->det_mon[i].type,
                   DS_ASCII_STR);
            break;
        case DS_NAME:
            ErrorF("type = %x DS_NAME = %x\n", pMonitor->det_mon[i].type,
                   DS_NAME);
            break;
        case DS_RANGES:
            ErrorF("type = %x DS_RANGES = %x\n", pMonitor->det_mon[i].type,
                   DS_RANGES);
            break;
        case DS_WHITE_P:
            ErrorF("type = %x DS_WHITE_P = %x\n", pMonitor->det_mon[i].type,
                   DS_WHITE_P);
            break;
        case DS_STD_TIMINGS:
            ErrorF("type = %x DS_STD_TIMINGS = %x\n",
                   pMonitor->det_mon[i].type, DS_STD_TIMINGS);
            break;
        }
        switch (pMonitor->det_mon[i].type) {
        case DS_SERIAL:
            pserial = pMonitor->det_mon[i].section.serial;
            ErrorF("seial: ");
            for (j = 0; j < 13; j++) {
                ErrorF("%02X", pserial[j]);
            }
            ErrorF("\n");
            break;
        case DS_ASCII_STR:
            pascii_data = pMonitor->det_mon[i].section.ascii_data;
            ErrorF("ascii: ");
            for (j = 0; j < 13; j++) {
                ErrorF("%c", pascii_data[j]);
            }
            ErrorF("\n");
            break;
        case DS_NAME:
            pname = pMonitor->det_mon[i].section.name;
            ErrorF("name: ");
            for (j = 0; j < 13; j++) {
                ErrorF("%c", pname[j]);
            }
            ErrorF("\n");
            break;
        case DS_RANGES:
            pranges = &(pMonitor->det_mon[i].section.ranges);
            ErrorF
                ("min_v = %d max_v = %d min_h = %d max_h = %d max_clock = %d\n",
                 pranges->min_v, pranges->max_v, pranges->min_h,
                 pranges->max_h, pranges->max_clock);
            break;
        case DS_WHITE_P:
            pwp = pMonitor->det_mon[i].section.wp;
            for (j = 0; j < 2; j++) {
                ErrorF
                    ("wp[%d].index = %d white_x = %8.3f white_y = %8.3f white_gamma = %8.3f\n",
                     j, pwp[j].index, pwp[j].white_x, pwp[j].white_y,
                     pwp[j].white_gamma);
            }
            break;
        case DS_STD_TIMINGS:
            pstd_t = pMonitor->det_mon[i].section.std_t;
            for (j = 0; j < 5; j++) {
                ErrorF
                    ("std_t[%d] hsize = %d vsize = %d refresh = %d id = %d\n",
                     j, pstd_t[j].hsize, pstd_t[j].vsize, pstd_t[j].refresh,
                     pstd_t[j].id);
            }
            break;
        case DT:

            pd_timings = &pMonitor->det_mon[i].section.d_timings;
            ErrorF("Detail Timing Descriptor\n");
            ErrorF("clock = %d\n", pd_timings->clock);
            ErrorF("h_active = %d\n", pd_timings->h_active);
            ErrorF("h_blanking = %d\n", pd_timings->h_blanking);
            ErrorF("v_active = %d\n", pd_timings->v_active);
            ErrorF("v_blanking = %d\n", pd_timings->v_blanking);
            ErrorF("h_sync_off = %d\n", pd_timings->h_sync_off);
            ErrorF("h_sync_width = %d\n", pd_timings->h_sync_width);
            ErrorF("v_sync_off = %d\n", pd_timings->v_sync_off);
            ErrorF("v_sync_width = %d\n", pd_timings->v_sync_width);
            ErrorF("h_size = %d\n", pd_timings->h_size);
            ErrorF("v_size = %d\n", pd_timings->v_size);
            ErrorF("h_border = %d\n", pd_timings->h_border);
            ErrorF("v_border = %d\n", pd_timings->v_border);
            ErrorF("interlaced = %d stereo = %x sync = %x misc = %x\n",
                   pd_timings->interlaced,
                   pd_timings->stereo, pd_timings->sync, pd_timings->misc);
            break;
        }
    }

    for (i = 0; i < 128; i += 16) {
        ErrorF("rawData[%02X]:", i);
        for (j = 0; j < 16; j++) {
            ErrorF(" %02X", pMonitor->rawData[i + j]);
        }
        ErrorF("\n");
    }
}
#endif

static void
XGIGetMonitorRangeByDDC(MonitorRangePtr range, xf86MonPtr pMonitor)
{
    int i, j;
    float VF, HF;
    struct detailed_timings *pd_timings;
    struct monitor_ranges *pranges;
    struct std_timings *pstd_t;

    if ((range == NULL) || (pMonitor == NULL)) {
        return;                 /* ignore */
    }

    PDEBUG5(ErrorF
            ("establish timing t1 = %02x t2=%02x\n", pMonitor->timings1.t1,
             pMonitor->timings1.t2));

    for (i = 0, j = 0; i < 8; i++, j++) 
	{
        if (establish_timing[j].width == -1) {
            continue;
        }

        if (pMonitor->timings1.t1 & (1 << i)) 
		{
            PDEBUG5(ErrorF("Support %dx%d@%4.1fHz Hseq = %8.3fKHz\n",
                           establish_timing[j].width,
                           establish_timing[j].height,
                           establish_timing[j].VRefresh,
                           establish_timing[j].HSync));

            if (range->loH > establish_timing[j].HSync) {
                range->loH = establish_timing[j].HSync;
            }

            if (range->hiH < establish_timing[j].HSync) {
                range->hiH = establish_timing[j].HSync;
            }

            if (range->loV > establish_timing[j].VRefresh) {
                range->loV = establish_timing[j].VRefresh;
            }

            if (range->hiV < establish_timing[j].VRefresh) {
                range->hiV = establish_timing[j].VRefresh;
            }
        }
    }

    PDEBUG5(ErrorF
            ("check establish timing t1:range ( %8.3f %8.3f %8.3f %8.3f )\n",
             range->loH, range->loV, range->hiH, range->hiV));

    for (i = 0; i < 8; i++, j++) {
        if (establish_timing[j].width == -1) {
            continue;
        }

        if (pMonitor->timings1.t2 & (1 << i)) {
            PDEBUG5(ErrorF("Support %dx%d@%4.1fHz Hseq = %8.3fKHz\n",
                           establish_timing[j].width,
                           establish_timing[j].height,
                           establish_timing[j].VRefresh,
                           establish_timing[j].HSync));

            if (range->loH > establish_timing[j].HSync) {
                range->loH = establish_timing[j].HSync;
            }

            if (range->hiH < establish_timing[j].HSync) {
                range->hiH = establish_timing[j].HSync;
            }

            if (range->loV > establish_timing[j].VRefresh) {
                range->loV = establish_timing[j].VRefresh;
            }

            if (range->hiV < establish_timing[j].VRefresh) {
                range->hiV = establish_timing[j].VRefresh;
            }
        }
    }
    PDEBUG5(ErrorF
            ("check establish timing t2:range ( %8.3f %8.3f %8.3f %8.3f )\n",
             range->loH, range->loV, range->hiH, range->hiV));

    for (i = 0; i < 8; i++) {
        for (j = 0; StdTiming[j].width != -1; j++) {
            if ((StdTiming[j].width == pMonitor->timings2[i].hsize) &&
                (StdTiming[j].height == pMonitor->timings2[i].vsize) &&
                (StdTiming[j].VRefresh == pMonitor->timings2[i].refresh)) {
                PDEBUG5(ErrorF("pMonitor->timings2[%d]= %d %d %d %d\n",
                               i,
                               pMonitor->timings2[i].hsize,
                               pMonitor->timings2[i].vsize,
                               pMonitor->timings2[i].refresh,
                               pMonitor->timings2[i].id));
                HF = StdTiming[j].HSync;
                VF = StdTiming[j].VRefresh;
                if (range->loH > HF)
                    range->loH = HF;
                if (range->loV > VF)
                    range->loV = VF;
                if (range->hiH < HF)
                    range->hiH = HF;
                if (range->hiV < VF)
                    range->hiV = VF;
                break;
            }
        }
    }
    PDEBUG5(ErrorF
            ("check standard timing :range ( %8.3f %8.3f %8.3f %8.3f )\n",
             range->loH, range->loV, range->hiH, range->hiV));

    for (i = 0; i < 4; i++) {
        switch (pMonitor->det_mon[i].type) {
        case DS_RANGES:
            pranges = &(pMonitor->det_mon[i].section.ranges);
            PDEBUG5(ErrorF
                    ("min_v = %d max_v = %d min_h = %d max_h = %d max_clock = %d\n",
                     pranges->min_v, pranges->max_v, pranges->min_h,
                     pranges->max_h, pranges->max_clock));

            if (range->loH > pranges->min_h)
                range->loH = pranges->min_h;
            if (range->loV > pranges->min_v)
                range->loV = pranges->min_v;
            if (range->hiH < pranges->max_h)
                range->hiH = pranges->max_h;
            if (range->hiV < pranges->max_v)
                range->hiV = pranges->max_v;
            PDEBUG5(ErrorF
                    ("range(%8.3f %8.3f %8.3f %8.3f)\n", range->loH,
                     range->loV, range->hiH, range->hiV));
            break;

        case DS_STD_TIMINGS:
            pstd_t = pMonitor->det_mon[i].section.std_t;
            for (j = 0; j < 5; j++) {
                int k;
                PDEBUG5(ErrorF
                        ("std_t[%d] hsize = %d vsize = %d refresh = %d id = %d\n",
                         j, pstd_t[j].hsize, pstd_t[j].vsize,
                         pstd_t[j].refresh, pstd_t[j].id));
                for (k = 0; StdTiming[k].width != -1; k++) {
                    if ((StdTiming[k].width == pstd_t[j].hsize) &&
                        (StdTiming[k].height == pstd_t[j].vsize) &&
                        (StdTiming[k].VRefresh == pstd_t[j].refresh)) {
                        if (range->loH > StdTiming[k].HSync)
                            range->loH = StdTiming[k].HSync;
                        if (range->hiH < StdTiming[k].HSync)
                            range->hiH = StdTiming[k].HSync;
                        if (range->loV > StdTiming[k].VRefresh)
                            range->loV = StdTiming[k].VRefresh;
                        if (range->hiV < StdTiming[k].VRefresh)
                            range->hiV = StdTiming[k].VRefresh;
                        break;
                    }

                }
            }
            break;

        case DT:

            pd_timings = &pMonitor->det_mon[i].section.d_timings;

            HF = pd_timings->clock / (pd_timings->h_active +
                                      pd_timings->h_blanking);
            VF = HF / (pd_timings->v_active + pd_timings->v_blanking);
            HF /= 1000;         /* into KHz Domain */
            if (range->loH > HF)
                range->loH = HF;
            if (range->hiH < HF)
                range->hiH = HF;
            if (range->loV > VF)
                range->loV = VF;
            if (range->hiV < VF)
                range->hiV = VF;
            PDEBUG(ErrorF
                   ("Detailing Timing: HF = %f VF = %f range (%8.3f %8.3f %8.3f %8.3f)\n",
                    HF, VF, range->loH, range->loV, range->hiH, range->hiV));
            break;
        }
    }
    PDEBUG5(ErrorF
            ("Done range(%8.3f %8.3f %8.3f %8.3f)\n", range->loH, range->loV,
             range->hiH, range->hiV));

}

static void
XGISyncDDCMonitorRange(MonPtr monitor, MonitorRangePtr range)
{
    int i;
    if ((monitor == NULL) || (range == NULL)) {
        return;
    }

	monitor->nHsync++;
	monitor->nVrefresh++;

#if 1
        monitor->hsync[monitor->nHsync-1].lo = range->loH;
        monitor->hsync[monitor->nHsync-1].hi = range->hiH;
        monitor->vrefresh[monitor->nVrefresh-1].lo = range->loV;
        monitor->vrefresh[monitor->nVrefresh-1].hi = range->hiV;
#else
    for (i = 0; i < monitor->nHsync; i++) {
        monitor->hsync[i].lo = range->loH;
        monitor->hsync[i].hi = range->hiH;
    }

    for (i = 0; i < monitor->nVrefresh; i++) {
        monitor->vrefresh[i].lo = range->loV;
        monitor->vrefresh[i].hi = range->hiV;
    }
#endif
}

/* Jong@08212009; defined in vb_ext.c */
extern void XGIPowerSaving(PVB_DEVICE_INFO pVBInfo, UCHAR PowerSavingStatus);
UCHAR g_PowerSavingStatus = 0x00;

static void
XGIDDCPreInit(ScrnInfoPtr pScrn)
{

    XGIPtr pXGI = XGIPTR(pScrn);
    xf86MonPtr pMonitor = NULL;
    xf86MonPtr pMonitorCRT1 = NULL;
    xf86MonPtr pMonitorDVI = NULL;
    xf86MonPtr pMonitorCRT2 = NULL;
    Bool didddc2;

	UCHAR  PowerSavingStatus = 0xFF; /* 0x00; */

	if(pXGI->IgnoreDDC)
	{
		xf86DrvMsg(pScrn->scrnIndex, X_INFO,
				   "Ignore DDC detection --> No EDID info...turn on all DAC and DVO\n");
		XGIPowerSaving(pXGI->XGI_Pr, 0x00);
		return;
	}

    static const char *ddcsstr =
        "CRT%d DDC monitor info: ************************************\n";
    static const char *ddcestr =
        "End of CRT%d DDC monitor info ******************************\n";

    /* Now for something completely different: DDC.
     * For 300 and 315/330 series, we provide our
     * own functions (in order to probe CRT2 as well)
     * If these fail, use the VBE.
     * All other chipsets will use VBE. No need to re-invent
     * the wheel there.
     */

    pXGI->pVbe = NULL;
    didddc2 = FALSE;

    /* In dual head mode, probe DDC using VBE only for CRT1 (second head) */
    if (IS_DUAL_HEAD(pXGI) && (!didddc2) && !IS_SECOND_HEAD(pXGI))
        didddc2 = TRUE;

    if (!didddc2) {
        /* If CRT1 is off or LCDA, skip DDC via VBE */
        if ((pXGI->CRT1off) || (pXGI->VBFlags & CRT1_LCDA))
            didddc2 = TRUE;
    }

    /* Now (re-)load and initialize the DDC module */
    if (!didddc2) {

        if (xf86LoadSubModule(pScrn, "ddc")) 
		{
#if 0
            xf86LoaderReqSymLists(ddcSymbols, NULL);
#endif
            if (pXGI->xgi_HwDevExt.jChipType == XG27) 
			{
				ErrorF("Getting CRT EDID (DAC1-CRT1)...\n");
				pMonitorCRT1 = XGIInternalDDC(pScrn, 0);

				if (pMonitorCRT1 == NULL) 
				{
					xf86DrvMsg(pScrn->scrnIndex, X_INFO,
							   "Could not retrieve DDC data for CRT1\n");
					/* PowerSavingStatus |= 0x01; */ /* device is not detected through DAC1 */

					ErrorF("Getting DVI EDID (DVO)...\n");
					pMonitorDVI = XGIInternalDDC(pScrn, 1);

					if (pMonitorDVI == NULL) {
						/* PowerSavingStatus |= 0x02; */ /* device is not detected through DVO */
						xf86DrvMsg(pScrn->scrnIndex, X_INFO,
							   "Could not retrieve DDC data for DVI\n");
					}
					else
					{
						PowerSavingStatus &= ~0x02; /* device is detected through DVO */
						xf86DrvMsg(pScrn->scrnIndex, X_INFO,
								   "Succeed to retrieve DDC data for DVI\n");
					}
				}
				else
				{
					if(g_DVI_I_SignalType == 0x00) /* analog CRT */
						PowerSavingStatus &= ~0x01; /* CRT device is detected */
					else /* DVI digital */
						PowerSavingStatus &= ~0x02; /* DVI device is detected */


					xf86DrvMsg(pScrn->scrnIndex, X_INFO,
						"Succeed to retrieve DDC data for %s\n", (g_DVI_I_SignalType == 0x01) ? "DVI" : "CRT");
				}

				ErrorF("Getting CRT EDID (CRT2)...\n");
				pMonitorCRT2 = XGIInternalDDC(pScrn, 2);

				if (pMonitorCRT2 == NULL) {
					/* PowerSavingStatus |= 0x04; */ /* device is not detected through DAC2 */
					xf86DrvMsg(pScrn->scrnIndex, X_INFO,
							   "Could not retrieve DDC data for CRT2\n");
				}
				else /* Used for filtering of CRT1/DVI modes; g_pMonitorDVI is not a good naming; should be g_pMonitorFilter */
				{
					PowerSavingStatus &= ~0x04; /* device is detected through DAC2 */
					g_pMonitorDVI=pMonitorCRT2;
					xf86DrvMsg(pScrn->scrnIndex, X_INFO,
							   "Succeed to retrieve DDC data for CRT2\n");
				}

				if (pMonitorCRT1 != NULL)
					pMonitor = pMonitorCRT1;
				else if(pMonitorDVI != NULL)
					pMonitor = pMonitorDVI;
				else if(pMonitorCRT2 != NULL)
					pMonitor = pMonitorCRT2;
			}
			else /* for XG20/21 */
			{
				ErrorF("Getting CRT EDID (CRT1)...\n");
				pMonitor = XGIInternalDDC(pScrn, 0);

				if (pMonitor == NULL) {
					PowerSavingStatus |= 0x01; /* device is not detected through DAC1 */
					xf86DrvMsg(pScrn->scrnIndex, X_INFO,
							   "Could not retrieve DDC data\n");
				}

				if (pXGI->xgi_HwDevExt.jChipType == XG21) /* CRT1 -DVI */
				{
					ErrorF("Getting XG21 DVI EDID (crt2)...\n");
					pMonitorDVI = XGIInternalDDC(pScrn, 1);

					if (pMonitorDVI == NULL) {
						PowerSavingStatus |= 0x02; /* device is not detected through DVO */
						xf86DrvMsg(pScrn->scrnIndex, X_INFO,
								   "Could not retrieve DVI DDC data\n");
					}
					else /* Jong 12/04/2007; used for filtering of CRT1 modes */
					{
						g_pMonitorDVI=pMonitorDVI;
					}

					if ((pMonitor == NULL) && (pMonitorDVI != NULL)) {
						pMonitor = pMonitorDVI;
					}
				}			
			}
        }
    }

	ErrorF("PowerSavingStatus = 0x%x...\n", PowerSavingStatus);

	if(PowerSavingStatus == 0xFF)
		PowerSavingStatus = 0x00; 


/*	if((pXGI->xgi_HwDevExt.jChipType == XG27) && (PowerSavingStatus == 0x07))
		PowerSavingStatus = 0x00; 

	if((pXGI->xgi_HwDevExt.jChipType == XG21) && (PowerSavingStatus == 0x03))
		PowerSavingStatus = 0x00; 
*/

	XGIPowerSaving(pXGI->XGI_Pr, PowerSavingStatus);
	g_PowerSavingStatus = PowerSavingStatus;

    /* initialize */

    if (pXGI->xgi_HwDevExt.jChipType == XG27) 
	{
		if (pMonitorCRT1) {
			pXGI->CRT1Range.loH = 1000;
			pXGI->CRT1Range.loV = 1000;
			pXGI->CRT1Range.hiH = 0;
			pXGI->CRT1Range.hiV = 0;
			XGIGetMonitorRangeByDDC(&(pXGI->CRT1Range), pMonitorCRT1);

			if (pMonitorDVI) {
				XGIGetMonitorRangeByDDC(&(pXGI->CRT1Range), pMonitorDVI);
			}
		}
		else {
			if (pMonitorDVI) {
				pXGI->CRT1Range.loV = 1000;
				pXGI->CRT1Range.loH = 1000;
				pXGI->CRT1Range.hiH = 0;
				pXGI->CRT1Range.hiV = 0;
				XGIGetMonitorRangeByDDC(&(pXGI->CRT1Range), pMonitorDVI);
			}
			else {
				pXGI->CRT1Range.loH = 0;
				pXGI->CRT1Range.loV = 0;
				pXGI->CRT1Range.hiH = 1000;
				pXGI->CRT1Range.hiV = 1000;
			}
		}

		if (pMonitorCRT2) {
			pXGI->CRT2Range.loV = 1000;
			pXGI->CRT2Range.loH = 1000;
			pXGI->CRT2Range.hiH = 0;
			pXGI->CRT2Range.hiV = 0;
			XGIGetMonitorRangeByDDC(&(pXGI->CRT2Range), pMonitorCRT2);
		}
		else {
			pXGI->CRT2Range.loH = 0;
			pXGI->CRT2Range.loV = 0;
			pXGI->CRT2Range.hiH = 1000;
			pXGI->CRT2Range.hiV = 1000;
		}
	}
	else /* XG20/21 */
	{
		if (pMonitor) {
			pXGI->CRT1Range.loH = 1000;
			pXGI->CRT1Range.loV = 1000;
			pXGI->CRT1Range.hiH = 0;
			pXGI->CRT1Range.hiV = 0;
			XGIGetMonitorRangeByDDC(&(pXGI->CRT1Range), pMonitor);
		}
		else {
			pXGI->CRT1Range.loH = 0;
			pXGI->CRT1Range.loV = 0;
			pXGI->CRT1Range.hiH = 1000;
			pXGI->CRT1Range.hiV = 1000;
		}

		if (pMonitorDVI) {
			pXGI->CRT2Range.loV = 1000;
			pXGI->CRT2Range.loH = 1000;
			pXGI->CRT2Range.hiH = 0;
			pXGI->CRT2Range.hiV = 0;
			XGIGetMonitorRangeByDDC(&(pXGI->CRT2Range), pMonitorDVI);
		}
		else {
			pXGI->CRT2Range.loH = 0;
			pXGI->CRT2Range.loV = 0;
			pXGI->CRT2Range.hiH = 1000;
			pXGI->CRT2Range.hiV = 1000;
		}
	}

	/* Jong@08132009 */
    /* if (pXGI->xgi_HwDevExt.jChipType == XG21) { */
    if ((pXGI->xgi_HwDevExt.jChipType == XG21) || (pXGI->xgi_HwDevExt.jChipType == XG27) ) {
        /* Mode range intersecting */
        if (pXGI->CRT1Range.loH < pXGI->CRT2Range.loH) {
            pXGI->CRT1Range.loH = pXGI->CRT2Range.loH;
        }
        if (pXGI->CRT1Range.loV < pXGI->CRT2Range.loV) {
            pXGI->CRT1Range.loV = pXGI->CRT2Range.loV;
        }
        if (pXGI->CRT1Range.hiH > pXGI->CRT2Range.hiH) {
            pXGI->CRT1Range.hiH = pXGI->CRT2Range.hiH;
        }
        if (pXGI->CRT1Range.hiV > pXGI->CRT2Range.hiV) {
            pXGI->CRT1Range.hiV = pXGI->CRT2Range.hiV;
        }
    }

	if (pMonitor) {
		XGISyncDDCMonitorRange(pScrn->monitor, &pXGI->CRT1Range);
	}

    if (pScrn->monitor) {
        pScrn->monitor->DDC = pMonitor;
    }

    return;

#ifdef XGIMERGED
    if (pXGI->MergedFB) {
        pXGI->CRT2pScrn->monitor = xalloc(sizeof(MonRec));
        if (pXGI->CRT2pScrn->monitor) {
            DisplayModePtr tempm = NULL, currentm = NULL, newm = NULL;
            memcpy(pXGI->CRT2pScrn->monitor, pScrn->monitor, sizeof(MonRec));
            pXGI->CRT2pScrn->monitor->DDC = NULL;
            pXGI->CRT2pScrn->monitor->Modes = NULL;
            tempm = pScrn->monitor->Modes;
            while (tempm) {
                if (!(newm = xalloc(sizeof(DisplayModeRec))))
                    break;
                memcpy(newm, tempm, sizeof(DisplayModeRec));
                if (!(newm->name = xalloc(strlen(tempm->name) + 1))) {
                    xfree(newm);
                    break;
                }
                strcpy(newm->name, tempm->name);
                if (!pXGI->CRT2pScrn->monitor->Modes)
                    pXGI->CRT2pScrn->monitor->Modes = newm;
                if (currentm) {
                    currentm->next = newm;
                    newm->prev = currentm;
                }
                currentm = newm;
                tempm = tempm->next;
            }

            if ((pMonitor = XGIInternalDDC(pXGI->CRT2pScrn, 1))) {
                xf86DrvMsg(pScrn->scrnIndex, X_PROBED, ddcsstr, 2);
                xf86PrintEDID(pMonitor);
                xf86DrvMsg(pScrn->scrnIndex, X_PROBED, ddcestr, 2);
                xf86SetDDCproperties(pXGI->CRT2pScrn, pMonitor);

                pXGI->CRT2pScrn->monitor->DDC = pMonitor;

                /* use DDC data if no ranges in config file */
                if (!pXGI->CRT2HSync) {
                    pXGI->CRT2pScrn->monitor->nHsync = 0;
                }
                if (!pXGI->CRT2VRefresh) {
                    pXGI->CRT2pScrn->monitor->nVrefresh = 0;
                }
            }
            else {
                xf86DrvMsg(pScrn->scrnIndex, X_PROBED,
                           "Failed to read DDC data for CRT2\n");
            }
        }
        else {
            XGIErrorLog(pScrn,
                        "Failed to allocate memory for CRT2 monitor, %s.\n",
                        mergeddisstr);
            if (pXGI->CRT2pScrn)
                xfree(pXGI->CRT2pScrn);
            pXGI->CRT2pScrn = NULL;
            pXGI->MergedFB = FALSE;
        }
    }
#endif

#ifdef XGIMERGED
    if (pXGI->MergedFB) {
        xf86DrvMsg(pScrn->scrnIndex, X_INFO, crtsetupstr, 1);
    }
#endif

    /* end of DDC */
}

#ifdef DEBUG5
static void
XGIDumpModePtr(DisplayModePtr mode)
{
    if (mode == NULL)
        return;

    ErrorF("Dump DisplayModePtr mode\n");
    ErrorF("name = %s\n", mode->name);
    /* ModeStatus status; */
    ErrorF("type = %d\n", mode->type);
    ErrorF("Clock = %d\n", mode->Clock);
    ErrorF("HDisplay = %d\n", mode->HDisplay);
    ErrorF("HSyncStart = %d\n", mode->HSyncStart);
    ErrorF("HSyncEnd = %d\n", mode->HSyncEnd);
    ErrorF("HTotal = %d\n", mode->HTotal);
    ErrorF("HSkew = %d\n", mode->HSkew);
    ErrorF("VDisplay = %d\n", mode->VDisplay);
    ErrorF("VSyncStart = %d\n", mode->VSyncStart);
    ErrorF("VSyncEnd = %d\n", mode->VSyncEnd);
    ErrorF("VTotal = %d\n", mode->VTotal);
    ErrorF("VScan = %d\n", mode->VScan);
    ErrorF("Flags = %d\n", mode->Flags);


    ErrorF("ClockIndex = %d\n", mode->ClockIndex);
    ErrorF("SynthClock = %d\n", mode->SynthClock);
    ErrorF("CrtcHDisplay = %d\n", mode->CrtcHDisplay);
    ErrorF("CrtcHBlankStart = %d\n", mode->CrtcHBlankStart);
    ErrorF("CrtcHSyncStart = %d\n", mode->CrtcHSyncStart);
    ErrorF("CrtcHSyncEnd = %d\n", mode->CrtcHSyncEnd);
    ErrorF("CrtcHBlankEnd = %d\n", mode->CrtcHBlankEnd);
    ErrorF("CrtcHTotal = %d\n", mode->CrtcHTotal);
    ErrorF("CrtcHSkew = %d\n", mode->CrtcHSkew);
    ErrorF("CrtcVDisplay = %d\n", mode->CrtcVDisplay);
    ErrorF("CrtcVBlankStart = %d\n", mode->CrtcVBlankStart);
    ErrorF("CrtcVSyncStart = %d\n", mode->CrtcVSyncStart);
    ErrorF("CrtcVSyncEnd = %d\n", mode->CrtcVSyncEnd);
    ErrorF("CrtcVBlankEnd = %d\n", mode->CrtcVBlankEnd);
    ErrorF("CrtcVTotal = %d\n", mode->CrtcVTotal);
    ErrorF("CrtcHAdjusted = %s\n", (mode->CrtcHAdjusted) ? "TRUE" : "FALSE");
    ErrorF("CrtcVAdjusted = %s\n", (mode->CrtcVAdjusted) ? "TRUE" : "FALSE");
    ErrorF("PrivSize = %d\n", mode->PrivSize);
    /* INT32 * Private; */
    ErrorF("PrivFlags = %d\n", mode->PrivFlags);
    ErrorF("HSync = %8.3f\n", mode->HSync);
    ErrorF("VRefresh = %8.3f\n", mode->VRefresh);
}
#endif

static void
XGIDumpMonPtr(MonPtr pMonitor)
{
#ifdef DEBUG5
    int i;
# if 0
    DisplayModePtr mode;
#endif

    ErrorF("XGIDumpMonPtr() ... \n");
    if (pMonitor == NULL) {
        ErrorF("pMonitor is NULL\n");
    }

    ErrorF("id = %s, vendor = %s model = %s\n",
           pMonitor->id, pMonitor->vendor, pMonitor->model);
    ErrorF("nHsync = %d\n", pMonitor->nHsync);
    ErrorF("nVrefresh = %d\n", pMonitor->nVrefresh);

    for (i = 0; i < MAX_HSYNC; i++) {
        ErrorF("hsync[%d] = (%8.3f,%8.3f)\n", i, pMonitor->hsync[i].lo,
               pMonitor->hsync[i].hi);
    }

    for (i = 0; i < MAX_VREFRESH; i++) {
        ErrorF("vrefresh[%d] = (%8.3f,%8.3f)\n", i, pMonitor->vrefresh[i].lo,
               pMonitor->vrefresh[i].hi);
    }

    ErrorF("widthmm = %d, heightmm = %d\n",
           pMonitor->widthmm, pMonitor->heightmm);
    ErrorF("options = %p, DDC = %p\n", pMonitor->options, pMonitor->DDC);
# if 0
    mode = pMonitor->Modes;
    while (1) {
        XGIDumpModePtr(mode);
        if (mode == pMonitor->Last) {
            break;
        }
        mode = mode->next;
    }
# endif
#endif /* DEBUG5 */
}

/* Jong 09/19/2007; support modeline of custom modes */
int	ModifyTypeOfSupportMode(DisplayModePtr availModes)
{
	int CountOfModifiedModes=0;
	DisplayModePtr p=availModes;

	while(p)
	{
		/* if( (p->HDisplay == 1440) && (p->VDisplay == 900)) */
		if( p->type == 0) /* externel support modeline */
		{
			p->type = M_T_USERDEF;
			CountOfModifiedModes++;
		}

		p=p->next;
	}

	return(CountOfModifiedModes);
}


/* Mandatory */
static Bool
XGIPreInit(ScrnInfoPtr pScrn, int flags)
{
    XGIPtr pXGI;
    MessageType from;
    unsigned long int i;
    int temp;
    ClockRangePtr clockRanges;
    int pix24flags;
    int fd;
    struct fb_fix_screeninfo fix;
    XGIEntPtr pXGIEnt = NULL;
    size_t memreq;

#if defined(XGIMERGED) || defined(XGIDUALHEAD)
    DisplayModePtr first, p, n;
#endif
    unsigned char srlockReg, crlockReg;
    vbeInfoPtr pVbe;

        /****************** Code Start ***********************/

    ErrorF("XGIPreInit\n");

    if (flags & PROBE_DETECT) {
        if (xf86LoadSubModule(pScrn, "vbe")) {
            int index = xf86GetEntityInfo(pScrn->entityList[0])->index;

            if ((pVbe = VBEExtendedInit(NULL, index, 0))) {
                ConfiguredMonitor = vbeDoEDID(pVbe, NULL);
                vbeFree(pVbe);
            }
        }
        return TRUE;
    }

    /*
     * Note: This function is only called once at server startup, and
     * not at the start of each server generation.  This means that
     * only things that are persistent across server generations can
     * be initialised here.  xf86Screens[] is the array of all screens,
     * (pScrn is a pointer to one of these).  Privates allocated using
     * xf86AllocateScrnInfoPrivateIndex() are too, and should be used
     * for data that must persist across server generations.
     *
     * Per-generation data should be allocated with
     * AllocateScreenPrivateIndex() from the ScreenInit() function.
     */

    /* Check the number of entities, and fail if it isn't one. */
    if (pScrn->numEntities != 1) {
        XGIErrorLog(pScrn, "Number of entities is not 1\n");
        return FALSE;
    }

    /* The vgahw module should be loaded here when needed */
    if (!xf86LoadSubModule(pScrn, "vgahw")) {
        XGIErrorLog(pScrn, "Could not load vgahw module\n");
        return FALSE;
    }
#if 0
    xf86LoaderReqSymLists(vgahwSymbols, NULL);
#endif
    /* Due to the liberal license terms this is needed for
     * keeping the copyright notice readable and intact in
     * binary distributions. Removing this is a copyright
     * infringement. Please read the license terms above.
     */

    xf86DrvMsg(pScrn->scrnIndex, X_INFO,
               "XGI driver (%s)\n", "01/21/2009" /*XGI_RELEASE_DATE*/);

    xf86DrvMsg(pScrn->scrnIndex, X_INFO,
               "Copyright (C) 2001-2004 Thomas Winischhofer <thomas@winischhofer.net> and others\n");

    /* Allocate a vgaHWRec */
    if (!vgaHWGetHWRec(pScrn)) {
        XGIErrorLog(pScrn, "Could not allocate VGA private\n");
        return FALSE;
    }

    /* Allocate the XGIRec driverPrivate */
    pXGI = XGIGetRec(pScrn);
    if (pXGI == NULL) {
        XGIErrorLog(pScrn, "Could not allocate memory for pXGI private\n");
        return FALSE;
    }

    pXGI->IODBase = pScrn->domainIOBase;


    /* Get the entity, and make sure it is PCI. */
    pXGI->pEnt = xf86GetEntityInfo(pScrn->entityList[0]);
    if (pXGI->pEnt->location.type != BUS_PCI) {
        XGIErrorLog(pScrn, "Entity's bus type is not PCI\n");
        XGIFreeRec(pScrn);
        return FALSE;
    }

#ifdef XGIDUALHEAD
    /* Allocate an entity private if necessary */
    if (xf86IsEntityShared(pScrn->entityList[0])) {
        pXGIEnt = xf86GetEntityPrivate(pScrn->entityList[0],
                                       XGIEntityIndex)->ptr;
        pXGI->entityPrivate = pXGIEnt;

        /* If something went wrong, quit here */
        if ((pXGIEnt->DisableDual) || (pXGIEnt->ErrorAfterFirst)) {
            XGIErrorLog(pScrn,
                        "First head encountered fatal error, can't continue\n");
            XGIFreeRec(pScrn);
            return FALSE;
        }
    }
#endif

    /* Find the PCI info for this screen */
#ifndef XSERVER_LIBPCIACCESS
    pXGI->PciInfo = xf86GetPciInfoForEntity(pXGI->pEnt->index);
    pXGI->PciTag = pciTag(pXGI->PciInfo->bus, pXGI->PciInfo->device,
                          pXGI->PciInfo->func);
#endif

    pXGI->Primary = xf86IsPrimaryPci(pXGI->PciInfo);
    xf86DrvMsg(pScrn->scrnIndex, X_PROBED,
               "This adapter is %s display adapter\n",
               (pXGI->Primary ? "primary" : "secondary"));

    if (pXGI->Primary) {
#if defined(__arm__) 
        VGAHWPTR(pScrn)->MapPhys = pXGI->PciInfo->ioBase[2] + 0xf2000000; 	
#endif

        VGAHWPTR(pScrn)->MapSize = 0x10000;     /* Standard 64k VGA window */
        if (!vgaHWMapMem(pScrn)) {
            XGIErrorLog(pScrn, "Could not map VGA memory\n");
            XGIFreeRec(pScrn);
            return FALSE;
       } else { 
#if defined(__arm__) 
		  vgaHWSetMmioFuncs(VGAHWPTR(pScrn), VGAHWPTR(pScrn)->Base, 0); 	
#endif

		  xf86DrvMsgVerb(pScrn->scrnIndex, X_INFO, 3, 
				  "VGA memory map from %p to %p \n", 
#ifdef XSERVER_LIBPCIACCESS
				  (void *)(intptr_t)pXGI->PciInfo->regions[2].base_addr, VGAHWPTR(pScrn)->Base);
#else
				  (void *)(intptr_t)pXGI->PciInfo->ioBase[2], VGAHWPTR(pScrn)->Base);
#endif
        }
    }
    vgaHWGetIOBase(VGAHWPTR(pScrn));

	/* Jong@08262009; why not to modify ??? */
    /* We "patch" the PIOOffset inside vgaHW in order to force
     * the vgaHW module to use our relocated i/o ports.
     */
    VGAHWPTR(pScrn)->PIOOffset = pXGI->IODBase - 0x380 +
#ifdef XSERVER_LIBPCIACCESS
        (pXGI->PciInfo->regions[2].base_addr & 0xFFFC)
#else
        (pXGI->PciInfo->ioBase[2] & 0xFFFC)
#endif
        ;

    pXGI->pInt = NULL;
    if (!pXGI->Primary) {
#if !defined(__alpha__)
#if !defined(__powerpc__)
        xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                   "Initializing display adapter through int10\n");

        if (xf86LoadSubModule(pScrn, "int10")) {
#if 0
            xf86LoaderReqSymLists(int10Symbols, NULL);
#endif
            pXGI->pInt = xf86InitInt10(pXGI->pEnt->index);
        }
        else {
            XGIErrorLog(pScrn, "Could not load int10 module\n");
        }
#endif /* !defined(__powerpc__) */
#endif /* !defined(__alpha__) */
    }

#ifndef XSERVER_LIBPCIACCESS
    xf86SetOperatingState(resVgaMem, pXGI->pEnt->index, ResUnusedOpr);

    /* Operations for which memory access is required */
    pScrn->racMemFlags = RAC_FB | RAC_COLORMAP | RAC_CURSOR | RAC_VIEWPORT;
    /* Operations for which I/O access is required */
    pScrn->racIoFlags = RAC_COLORMAP | RAC_CURSOR | RAC_VIEWPORT;
#endif

    /* The ramdac module should be loaded here when needed */
    if (!xf86LoadSubModule(pScrn, "ramdac")) {
        XGIErrorLog(pScrn, "Could not load ramdac module\n");

        if (pXGIEnt)
            pXGIEnt->ErrorAfterFirst = TRUE;

        if (pXGI->pInt)
            xf86FreeInt10(pXGI->pInt);
        XGIFreeRec(pScrn);
        return FALSE;
    }
#if 0
    xf86LoaderReqSymLists(ramdacSymbols, NULL);
#endif
    /* Set pScrn->monitor */
    pScrn->monitor = pScrn->confScreen->monitor;

    /* Jong 09/19/2007; modify type of support modes to M_T_USERDEF */
    g_CountOfUserDefinedModes=ModifyTypeOfSupportMode(pScrn->monitor->Modes);

    /*
     * Set the Chipset and ChipRev, allowing config file entries to
     * override. DANGEROUS!
     */
    if (pXGI->pEnt->device->chipset && *pXGI->pEnt->device->chipset) {
        PDEBUG(ErrorF(" --- Chipset 1 \n"));
        pScrn->chipset = pXGI->pEnt->device->chipset;
        pXGI->Chipset = xf86StringToToken(XGIChipsets, pScrn->chipset);
        from = X_CONFIG;
    }
    else if (pXGI->pEnt->device->chipID >= 0) {
        PDEBUG(ErrorF(" --- Chipset 2 \n"));
        pXGI->Chipset = pXGI->pEnt->device->chipID;
        pScrn->chipset =
            (char *) xf86TokenToString(XGIChipsets, pXGI->Chipset);

        from = X_CONFIG;
        xf86DrvMsg(pScrn->scrnIndex, X_CONFIG, "ChipID override: 0x%04X\n",
                   pXGI->Chipset);
    }
    else {
        PDEBUG(ErrorF(" --- Chipset 3 \n"));
        from = X_PROBED;
        pXGI->Chipset = DEVICE_ID(pXGI->PciInfo);
        pScrn->chipset =
            (char *) xf86TokenToString(XGIChipsets, pXGI->Chipset);
    }
    if (pXGI->pEnt->device->chipRev >= 0) {
        pXGI->ChipRev = pXGI->pEnt->device->chipRev;
        xf86DrvMsg(pScrn->scrnIndex, X_CONFIG, "ChipRev override: %d\n",
                   pXGI->ChipRev);
    }
    else {
        pXGI->ChipRev = CHIP_REVISION(pXGI->PciInfo);
    }
    pXGI->xgi_HwDevExt.jChipRevision = pXGI->ChipRev;

    PDEBUG(ErrorF(" --- Chipset : %s \n", pScrn->chipset));


    /*
     * This shouldn't happen because such problems should be caught in
     * XGIProbe(), but check it just in case.
     */
    if (pScrn->chipset == NULL) {
        XGIErrorLog(pScrn, "ChipID 0x%04X is not recognised\n",
                    pXGI->Chipset);

        if (pXGIEnt)
            pXGIEnt->ErrorAfterFirst = TRUE;

        if (pXGI->pInt)
            xf86FreeInt10(pXGI->pInt);
        XGIFreeRec(pScrn);
        return FALSE;
    }

    if (pXGI->Chipset < 0) {
        XGIErrorLog(pScrn, "Chipset \"%s\" is not recognised\n",
                    pScrn->chipset);

        if (pXGIEnt)
            pXGIEnt->ErrorAfterFirst = TRUE;

        if (pXGI->pInt)
            xf86FreeInt10(pXGI->pInt);
        XGIFreeRec(pScrn);
        return FALSE;
    }

    /* Determine chipset and VGA engine type */
    pXGI->ChipFlags = 0;
    pXGI->XGI_SD_Flags = 0;

    switch (pXGI->Chipset) {
    case PCI_CHIP_XGIXG40:
    case PCI_CHIP_XGIXG20:
    case PCI_CHIP_XGIXG21:
        pXGI->xgi_HwDevExt.jChipType = XG40;
        pXGI->myCR63 = 0x63;
        pXGI->mmioSize = 64;
        break;

    case PCI_CHIP_XGIXG27:
    	pXGI->xgi_HwDevExt.jChipType = XG27;
    	pXGI->myCR63 = 0x63;
    	pXGI->mmioSize = 64;
        break;

    default:
        /* This driver currently only supports V3XE, V3XT, V5, V8 (all of
         * which are XG40 chips) and Z7 (which is XG20).
         */
        if (pXGI->pInt) {
            xf86FreeInt10(pXGI->pInt);
        }
        XGIFreeRec(pScrn);
        return FALSE;
    }

/* load frame_buffer */

    FbDevExist = FALSE;
   if((pXGI->Chipset != PCI_CHIP_XGIXG20)&&(pXGI->Chipset != PCI_CHIP_XGIXG21)&&( pXGI->Chipset != PCI_CHIP_XGIXG27 ))
   {
        if ((fd = open("/dev/fb", 'r')) != -1) {
            PDEBUG(ErrorF("--- open /dev/fb....   \n"));
            ioctl(fd, FBIOGET_FSCREENINFO, &fix);
            if (fix.accel == FB_ACCEL_XGI_GLAMOUR) {
                PDEBUG(ErrorF("--- fix.accel....   \n"));
                FbDevExist = TRUE;
            }
            else
                PDEBUG(ErrorF("--- no fix.accel.... 0x%08lx  \n", fix.accel));
            close(fd);
        }
    }


    /*
     * The first thing we should figure out is the depth, bpp, etc.
     * Additionally, determine the size of the HWCursor memory area.
     */
    pXGI->CursorSize = 4096;
    pix24flags = Support32bppFb;

#ifdef XGIDUALHEAD
    /* In case of Dual Head, we need to determine if we are the "master" head or
     * the "slave" head. In order to do that, we set PrimInit to DONE in the
     * shared entity at the end of the first initialization. The second
     * initialization then knows that some things have already been done. THIS
     * ALWAYS ASSUMES THAT THE FIRST DEVICE INITIALIZED IS THE MASTER!
     */

    if (xf86IsEntityShared(pScrn->entityList[0])) {
        if (pXGIEnt->lastInstance > 0) {
            if (!xf86IsPrimInitDone(pScrn->entityList[0])) {
                /* First Head (always CRT2) */
                pXGI->SecondHead = FALSE;
                pXGIEnt->pScrn_1 = pScrn;
                pXGIEnt->CRT2ModeNo = -1;
                pXGIEnt->CRT2ModeSet = FALSE;
                pXGI->DualHeadMode = TRUE;
                pXGIEnt->DisableDual = FALSE;
                pXGIEnt->BIOS = NULL;
                pXGIEnt->XGI_Pr = NULL;
                pXGIEnt->RenderAccelArray = NULL;
            }
            else {
                /* Second Head (always CRT1) */
                pXGI->SecondHead = TRUE;
                pXGIEnt->pScrn_2 = pScrn;
                pXGI->DualHeadMode = TRUE;
            }
        }
        else {
            /* Only one screen in config file - disable dual head mode */
            pXGI->SecondHead = FALSE;
            pXGI->DualHeadMode = FALSE;
            pXGIEnt->DisableDual = TRUE;
        }
    }
    else {
        /* Entity is not shared - disable dual head mode */
        pXGI->SecondHead = FALSE;
        pXGI->DualHeadMode = FALSE;
    }
#endif

    /* Allocate VB_DEVICE_INFO (for mode switching code) and initialize it */
    pXGI->XGI_Pr = NULL;
    if (pXGIEnt && pXGIEnt->XGI_Pr) {
        pXGI->XGI_Pr = pXGIEnt->XGI_Pr;
    }

    if (!pXGI->XGI_Pr) {
        if (!(pXGI->XGI_Pr = xnfcalloc(sizeof(VB_DEVICE_INFO), 1))) {
            XGIErrorLog(pScrn,
                        "Could not allocate memory for XGI_Pr private\n");

            if (pXGIEnt)
                pXGIEnt->ErrorAfterFirst = TRUE;
            if (pXGI->pInt)
                xf86FreeInt10(pXGI->pInt);
            XGIFreeRec(pScrn);
            return FALSE;
        }

        if (pXGIEnt)
            pXGIEnt->XGI_Pr = pXGI->XGI_Pr;

        memset(pXGI->XGI_Pr, 0, sizeof(VB_DEVICE_INFO));
    }

    /* Get our relocated IO registers */
#if defined(__arm__) 
	pXGI->RelIO = (XGIIOADDRESS)(((IOADDRESS)VGAHWPTR(pScrn)->Base & 0xFFFFFFFC) + pXGI->IODBase); 	

#else
    pXGI->RelIO = (XGIIOADDRESS) (pXGI->IODBase |
#ifdef XSERVER_LIBPCIACCESS
                                  (pXGI->PciInfo->regions[2].base_addr & 0xFFFC) 
#else
                                  (pXGI->PciInfo->ioBase[2] & 0xFFFC) 
#endif
                                  );
#endif

    pXGI->xgi_HwDevExt.pjIOAddress = (XGIIOADDRESS) (pXGI->RelIO + 0x30);
    xf86DrvMsg(pScrn->scrnIndex, from, "Relocated IO registers at 0x%lX\n",
               (unsigned long) pXGI->RelIO);
	ErrorF("xgi_driver.c-pXGI->xgi_HwDevExt.pjIOAddress=0x%lx...\n", pXGI->xgi_HwDevExt.pjIOAddress);

    if (!xf86SetDepthBpp(pScrn, 0, 0, 0, pix24flags)) {
        XGIErrorLog(pScrn, "xf86SetDepthBpp() error\n");

        if (pXGIEnt)
            pXGIEnt->ErrorAfterFirst = TRUE;

        if (pXGI->pInt)
            xf86FreeInt10(pXGI->pInt);
        XGIFreeRec(pScrn);
        return FALSE;
    }

    /* Check that the returned depth is one we support */
    temp = 0;
    switch (pScrn->depth) {
    case 8:
    case 16:
    case 24:
#if !defined(__powerpc__)
    case 15:
#endif
        break;
    default:
        temp = 1;
    }

    if (temp) {
        XGIErrorLog(pScrn,
                    "Given color depth (%d) is not supported by this driver/chipset\n",
                    pScrn->depth);
        if (pXGI->pInt)
            xf86FreeInt10(pXGI->pInt);
        XGIFreeRec(pScrn);
        return FALSE;
    }

    xf86PrintDepthBpp(pScrn);

    /* Get the depth24 pixmap format */
    if (pScrn->depth == 24 && pix24bpp == 0) {
        pix24bpp = xf86GetBppFromDepth(pScrn, 24);
    }

    /*
     * This must happen after pScrn->display has been set because
     * xf86SetWeight references it.
     */
    if (pScrn->depth > 8) {
        /* The defaults are OK for us */
        rgb zeros = { 0, 0, 0 };

        if (!xf86SetWeight(pScrn, zeros, zeros)) {
            XGIErrorLog(pScrn, "xf86SetWeight() error\n");

            if (pXGIEnt)
                pXGIEnt->ErrorAfterFirst = TRUE;

            if (pXGI->pInt)
                xf86FreeInt10(pXGI->pInt);
            XGIFreeRec(pScrn);
            return FALSE;
        }
        else {
            Bool ret = FALSE;
            switch (pScrn->depth) {
            case 15:
                if ((pScrn->weight.red != 5) ||
                    (pScrn->weight.green != 5) || (pScrn->weight.blue != 5))
                    ret = TRUE;
                break;
            case 16:
                if ((pScrn->weight.red != 5) ||
                    (pScrn->weight.green != 6) || (pScrn->weight.blue != 5))
                    ret = TRUE;
                break;
            case 24:
                if ((pScrn->weight.red != 8) ||
                    (pScrn->weight.green != 8) || (pScrn->weight.blue != 8))
                    ret = TRUE;
                break;
            }
            if (ret) {
                XGIErrorLog(pScrn,
                            "RGB weight %d%d%d at depth %d not supported by hardware\n",
                            (int) pScrn->weight.red,
                            (int) pScrn->weight.green,
                            (int) pScrn->weight.blue, pScrn->depth);

                if (pXGIEnt)
                    pXGIEnt->ErrorAfterFirst = TRUE;

                if (pXGI->pInt)
                    xf86FreeInt10(pXGI->pInt);
                XGIFreeRec(pScrn);
                return FALSE;
            }
        }
    }

    /* Set the current layout parameters */
    pXGI->CurrentLayout.bitsPerPixel = pScrn->bitsPerPixel;
    pXGI->CurrentLayout.depth = pScrn->depth;
    /* (Inside this function, we can use pScrn's contents anyway) */

    if (!xf86SetDefaultVisual(pScrn, -1)) {
        XGIErrorLog(pScrn, "xf86SetDefaultVisual() error\n");

        if (pXGIEnt)
            pXGIEnt->ErrorAfterFirst = TRUE;

        if (pXGI->pInt)
            xf86FreeInt10(pXGI->pInt);
        XGIFreeRec(pScrn);
        return FALSE;
    }
    else {
        /* We don't support DirectColor at > 8bpp */
        if (pScrn->depth > 8 && pScrn->defaultVisual != TrueColor) {
            XGIErrorLog(pScrn,
                        "Given default visual (%s) is not supported at depth %d\n",
                        xf86GetVisualName(pScrn->defaultVisual),
                        pScrn->depth);

            if (pXGIEnt)
                pXGIEnt->ErrorAfterFirst = TRUE;

            if (pXGI->pInt)
                xf86FreeInt10(pXGI->pInt);
            XGIFreeRec(pScrn);
            return FALSE;
        }
    }

    /* Due to palette & timing problems we don't support 8bpp in DHM */
    if ((IS_DUAL_HEAD(pXGI)) && (pScrn->bitsPerPixel == 8)) {
        XGIErrorLog(pScrn,
                    "Color depth 8 not supported in Dual Head mode.\n");
        if (pXGIEnt)
            pXGIEnt->ErrorAfterFirst = TRUE;
        if (pXGI->pInt)
            xf86FreeInt10(pXGI->pInt);
        XGIFreeRec(pScrn);
        return FALSE;
    }

    /*
     * The cmap layer needs this to be initialised.
     */
    {
		Gamma zeros = { 0.0, 0.0, 0.0 };
        /* Gamma zeros = { 0.5, 0.5, 0.5 }; */

        if (!xf86SetGamma(pScrn, zeros)) {
            XGIErrorLog(pScrn, "xf86SetGamma() error\n");

            if (pXGIEnt)
                pXGIEnt->ErrorAfterFirst = TRUE;

            if (pXGI->pInt)
                xf86FreeInt10(pXGI->pInt);
            XGIFreeRec(pScrn);
            return FALSE;
        }
    }

    /* We use a programamble clock */
    pScrn->progClock = TRUE;

    /* Set the bits per RGB for 8bpp mode */
    if (pScrn->depth == 8) {
        pScrn->rgbBits = 6;
    }

    from = X_DEFAULT;

    /* Unlock registers */
    xgiSaveUnlockExtRegisterLock(pXGI, &srlockReg, &crlockReg);

    /* Read BIOS for 300 and 315/330 series customization */
    pXGI->xgi_HwDevExt.pjVirtualRomBase = NULL;
    pXGI->BIOS = NULL;
    pXGI->xgi_HwDevExt.UseROM = FALSE;

    /* Evaluate options */
    xgiOptions(pScrn);

#ifdef XGIMERGED
    /* Due to palette & timing problems we don't support 8bpp in MFBM */
    if ((pXGI->MergedFB) && (pScrn->bitsPerPixel == 8)) {
        XGIErrorLog(pScrn, "MergedFB: Color depth 8 not supported, %s\n",
                    mergeddisstr);
        pXGI->MergedFB = pXGI->MergedFBAuto = FALSE;
    }
#endif

    /* Do basic configuration */

    XGISetup(pScrn);

    from = X_PROBED;
#ifdef XSERVER_LIBPCIACCESS
        pXGI->FbAddress = pXGI->PciInfo->regions[0].base_addr & 0xFFFFFFF0;
#else
    if (pXGI->pEnt->device->MemBase != 0) {
        /*
         * XXX Should check that the config file value matches one of the
         * PCI base address values.
         */
        pXGI->FbAddress = pXGI->pEnt->device->MemBase;
        from = X_CONFIG;
    }
    else {
        pXGI->FbAddress = pXGI->PciInfo->memBase[0] & 0xFFFFFFF0;
    }
#endif

    pXGI->realFbAddress = pXGI->FbAddress;

    xf86DrvMsg(pScrn->scrnIndex, from,
               "%sinear framebuffer at 0x%lX\n",
               IS_DUAL_HEAD(pXGI) ? "Global l" : "L",
               (unsigned long) pXGI->FbAddress);

#ifdef XSERVER_LIBPCIACCESS
        pXGI->IOAddress = pXGI->PciInfo->regions[1].base_addr & 0xFFFFFFF0;
#else
    if (pXGI->pEnt->device->IOBase != 0) {
        /*
         * XXX Should check that the config file value matches one of the
         * PCI base address values.
         */
        pXGI->IOAddress = pXGI->pEnt->device->IOBase;
        from = X_CONFIG;
    }
    else {
        pXGI->IOAddress = pXGI->PciInfo->memBase[1] & 0xFFFFFFF0;
    }
#endif

    xf86DrvMsg(pScrn->scrnIndex, from,
               "MMIO registers at 0x%lX (size %ldK)\n",
               (unsigned long) pXGI->IOAddress, pXGI->mmioSize);
    pXGI->xgi_HwDevExt.bIntegratedMMEnabled = TRUE;

#ifndef XSERVER_LIBPCIACCESS
    /* Register the PCI-assigned resources. */
    if (xf86RegisterResources(pXGI->pEnt->index, NULL, ResExclusive)) {
        XGIErrorLog(pScrn,
                    "xf86RegisterResources() found resource conflicts\n");

        if (pXGIEnt)
            pXGIEnt->ErrorAfterFirst = TRUE;

        if (pXGI->pInt)
            xf86FreeInt10(pXGI->pInt);
        xgiRestoreExtRegisterLock(pXGI, srlockReg, crlockReg);
        XGIFreeRec(pScrn);
        return FALSE;
    }
#endif

    from = X_PROBED;
    if (pXGI->pEnt->device->videoRam != 0) {

        xf86DrvMsg(pScrn->scrnIndex, X_WARNING,
                   "Option \"VideoRAM\" ignored\n");
    }

    xf86DrvMsg(pScrn->scrnIndex, from, "VideoRAM: %d KB\n", pScrn->videoRam);

    pXGI->FbMapSize = pXGI->availMem = pScrn->videoRam * 1024;
    pXGI->xgi_HwDevExt.ulVideoMemorySize = pScrn->videoRam * 1024;
    pXGI->xgi_HwDevExt.bSkipDramSizing = TRUE;

    /* Calculate real availMem according to Accel/TurboQueue and
     * HWCursur setting.
     *
     * TQ is max 64KiB.  Reduce the available memory by 64KiB, and locate the
     * TQ at the beginning of this last 64KiB block.  This is done even when
     * using the HWCursor, because the cursor only takes 2KiB and the queue
     * does not seem to last that far anyway.
     *
     * The TQ must be located at 32KB boundaries.
     */
    if (pScrn->videoRam < 3072) {
        if (pXGI->TurboQueue) {
            xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                       "Not enough video RAM for TurboQueue. TurboQueue disabled\n");
            pXGI->TurboQueue = FALSE;
        }
    }

    pXGI->availMem -= (pXGI->TurboQueue) ? (64 * 1024) : pXGI->CursorSize;


    /* In dual head mode, we share availMem equally - so align it
     * to 8KB; this way, the address of the FB of the second
     * head is aligned to 4KB for mapping.
     *
     * Check MaxXFBMem setting.  Since DRI is not supported in dual head
     * mode, we don't need the MaxXFBMem setting.
     */
    if (IS_DUAL_HEAD(pXGI)) {
        if (pXGI->maxxfbmem) {
            xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                       "MaxXFBMem not used in Dual Head mode. Using all VideoRAM.\n");
        }

        pXGI->availMem &= 0xFFFFE000;
        pXGI->maxxfbmem = pXGI->availMem;
    }
    else if (pXGI->maxxfbmem) {
        if (pXGI->maxxfbmem > pXGI->availMem) {
            if (pXGI->xgifbMem) {
                pXGI->maxxfbmem = pXGI->xgifbMem * 1024;
                xf86DrvMsg(pScrn->scrnIndex, X_WARNING,
                           "Invalid MaxXFBMem setting. Using xgifb heap start information\n");
            }
            else {
                xf86DrvMsg(pScrn->scrnIndex, X_WARNING,
                           "Invalid MaxXFBMem setting. Using all VideoRAM for framebuffer\n");
                pXGI->maxxfbmem = pXGI->availMem;
            }
        }
        else if (pXGI->xgifbMem) {
            if (pXGI->maxxfbmem > pXGI->xgifbMem * 1024) {
                xf86DrvMsg(pScrn->scrnIndex, X_WARNING,
                           "MaxXFBMem beyond xgifb heap start. Using xgifb heap start\n");
                pXGI->maxxfbmem = pXGI->xgifbMem * 1024;
            }
        }
    }
    else if (pXGI->xgifbMem) {
        pXGI->maxxfbmem = pXGI->xgifbMem * 1024;
    }
    else
        pXGI->maxxfbmem = pXGI->availMem;

    xf86DrvMsg(pScrn->scrnIndex, X_INFO, "Using %ldK of framebuffer memory\n",
               pXGI->maxxfbmem / 1024);

    pXGI->CRT1off = -1;

    /* Detect video bridge and sense TV/VGA2 */
    XGIVGAPreInit(pScrn);

    /* Detect CRT1 (via DDC1 and DDC2, hence via VGA port; regardless of LCDA) */
    XGICRT1PreInit(pScrn);

    /* Detect LCD (connected via CRT2, regardless of LCDA) and LCD resolution */
    XGILCDPreInit(pScrn);

    /* LCDA only supported under these conditions: */
    if (pXGI->ForceCRT1Type == CRT1_LCDA) {
        if (!
            (pXGI->XGI_Pr->
             VBType & (VB_XGI301C | VB_XGI302B | VB_XGI301LV |
                           VB_XGI302LV))) {
            xf86DrvMsg(pScrn->scrnIndex, X_WARNING,
                       "Chipset/Video bridge does not support LCD-via-CRT1\n");
            pXGI->ForceCRT1Type = CRT1_VGA;
        }
        else if (!(pXGI->VBFlags & CRT2_LCD)) {
            xf86DrvMsg(pScrn->scrnIndex, X_WARNING,
                       "No digitally connected LCD panel found, LCD-via-CRT1 "
                       "disabled\n");
            pXGI->ForceCRT1Type = CRT1_VGA;
        }
    }

    /* Setup SD flags */
    pXGI->XGI_SD_Flags |= XGI_SD_ADDLSUPFLAG;

    if (pXGI->XGI_Pr->VBType & VB_XGIVB) {
        pXGI->XGI_SD_Flags |= XGI_SD_SUPPORTTV;
    }

#ifdef ENABLE_YPBPR
    if (pXGI->XGI_Pr->VBType & (VB_XGI301 | VB_XGI301B | VB_XGI302B)) {
        pXGI->XGI_SD_Flags |= XGI_SD_SUPPORTHIVISION;
    }
#endif

#ifdef TWDEBUG                  /* @@@ TEST @@@ */
    pXGI->XGI_SD_Flags |= XGI_SD_SUPPORTYPBPRAR;
    xf86DrvMsg(0, X_INFO, "TEST: Support Aspect Ratio\n");
#endif

    /* Detect CRT2-TV and PAL/NTSC mode */
    XGITVPreInit(pScrn);

    /* Detect CRT2-VGA */
    XGICRT2PreInit(pScrn);
    PDEBUG(ErrorF("3496 pXGI->VBFlags =%x\n", pXGI->VBFlags));

    if (!(pXGI->XGI_SD_Flags & XGI_SD_SUPPORTYPBPR)) {
        if ((pXGI->ForceTVType != -1) && (pXGI->ForceTVType & TV_YPBPR)) {
            pXGI->ForceTVType = -1;
            xf86DrvMsg(pScrn->scrnIndex, X_WARNING,
                       "YPbPr TV output not supported\n");
        }
    }

    if (!(pXGI->XGI_SD_Flags & XGI_SD_SUPPORTHIVISION)) {
        if ((pXGI->ForceTVType != -1) && (pXGI->ForceTVType & TV_HIVISION)) {
            pXGI->ForceTVType = -1;
            xf86DrvMsg(pScrn->scrnIndex, X_WARNING,
                       "HiVision TV output not supported\n");
        }
    }

    if (pXGI->XGI_Pr->VBType & VB_XGIVB) {
        pXGI->XGI_SD_Flags |= (XGI_SD_SUPPORTPALMN | XGI_SD_SUPPORTNTSCJ);
    }
    if (pXGI->XGI_Pr->VBType & VB_XGIVB) {
        pXGI->XGI_SD_Flags |= XGI_SD_SUPPORTTVPOS;
    }
    if (pXGI->XGI_Pr->
        VBType & (VB_XGI301 | VB_XGI301B | VB_XGI301C | VB_XGI302B)) {
        pXGI->XGI_SD_Flags |= (XGI_SD_SUPPORTSCART | XGI_SD_SUPPORTVGA2);
    }

    if ((pXGI->XGI_Pr->
         VBType & (VB_XGI301C | VB_XGI302B | VB_XGI301LV | VB_XGI302LV))
        && (pXGI->VBFlags & CRT2_LCD)) {
        pXGI->XGI_SD_Flags |= XGI_SD_SUPPORTLCDA;
    }

    pXGI->VBFlags |= pXGI->ForceCRT1Type;

#ifdef TWDEBUG
    xf86DrvMsg(0, X_INFO, "SDFlags %lx\n", pXGI->XGI_SD_Flags);
#endif


    if (!IS_DUAL_HEAD(pXGI) || IS_SECOND_HEAD(pXGI)) {
        xf86DrvMsg(pScrn->scrnIndex, pXGI->CRT1gammaGiven ? X_CONFIG : X_INFO,
                   "CRT1 gamma correction is %s\n",
                   pXGI->CRT1gamma ? "enabled" : "disabled");
    }

    /* Eventually overrule TV Type (SVIDEO, COMPOSITE, SCART, HIVISION, YPBPR) */
    if (pXGI->XGI_Pr->VBType & VB_XGIVB) {
        if (pXGI->ForceTVType != -1) {
            pXGI->VBFlags &= ~(TV_INTERFACE);
            pXGI->VBFlags |= pXGI->ForceTVType;
            if (pXGI->VBFlags & TV_YPBPR) {
                pXGI->VBFlags &= ~(TV_STANDARD);
                pXGI->VBFlags &= ~(TV_YPBPRAR);
                pXGI->VBFlags |= pXGI->ForceYPbPrType;
                pXGI->VBFlags |= pXGI->ForceYPbPrAR;
            }
        }
    }

    /* Check if CRT1 used or needed.  There are three cases where this can
     * happen:
     *     - No video bridge.
     *     - No CRT2 output.
     *     - Depth = 8 and bridge=LVDS|301B-DH
     *     - LCDA
     */
    if (((pXGI->XGI_Pr->VBType & VB_XGIVB) == 0)
        || ((pXGI->VBFlags & (CRT2_VGA | CRT2_LCD | CRT2_TV)) == 0)
        || ((pScrn->bitsPerPixel == 8)
            && (pXGI->XGI_Pr->VBType & VB_XGI301LV302LV))
        || (pXGI->VBFlags & CRT1_LCDA)) {
        pXGI->CRT1off = 0;
    }


    /* Handle TVStandard option */
    if ((pXGI->NonDefaultPAL != -1) || (pXGI->NonDefaultNTSC != -1)) {
        if (!(pXGI->XGI_Pr->VBType & VB_XGIVB)) {
            xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                       "PALM, PALN and NTSCJ not supported on this hardware\n");
            pXGI->NonDefaultPAL = pXGI->NonDefaultNTSC = -1;
            pXGI->VBFlags &= ~(TV_PALN | TV_PALM | TV_NTSCJ);
            pXGI->XGI_SD_Flags &=
                ~(XGI_SD_SUPPORTPALMN | XGI_SD_SUPPORTNTSCJ);
        }
    }

#ifdef XGI_CP
    XGI_CP_DRIVER_RECONFIGOPT
#endif
        /* Do some MergedFB mode initialisation */
#ifdef XGIMERGED
        if (pXGI->MergedFB) {
        pXGI->CRT2pScrn = xalloc(sizeof(ScrnInfoRec));
        if (!pXGI->CRT2pScrn) {
            XGIErrorLog(pScrn,
                        "Failed to allocate memory for 2nd pScrn, %s\n",
                        mergeddisstr);
            pXGI->MergedFB = FALSE;
        }
        else {
            memcpy(pXGI->CRT2pScrn, pScrn, sizeof(ScrnInfoRec));
        }
    }
#endif
    PDEBUG(ErrorF("3674 pXGI->VBFlags =%x\n", pXGI->VBFlags));

    /* Determine CRT1<>CRT2 mode
     *     Note: When using VESA or if the bridge is in slavemode, display
     *           is ALWAYS in MIRROR_MODE!
     *           This requires extra checks in functions using this flag!
     *           (see xgi_video.c for example)
     */
    if (pXGI->VBFlags & DISPTYPE_DISP2) {
        if (pXGI->CRT1off) {    /* CRT2 only ------------------------------- */
            if (IS_DUAL_HEAD(pXGI)) {
                XGIErrorLog(pScrn,
                            "CRT1 not detected or forced off. Dual Head mode can't initialize.\n");
                if (pXGIEnt)
                    pXGIEnt->DisableDual = TRUE;
                if (pXGIEnt)
                    pXGIEnt->ErrorAfterFirst = TRUE;
                if (pXGI->pInt)
                    xf86FreeInt10(pXGI->pInt);
                pXGI->pInt = NULL;
                xgiRestoreExtRegisterLock(pXGI, srlockReg, crlockReg);
                XGIFreeRec(pScrn);
                return FALSE;
            }
#ifdef XGIMERGED
            if (pXGI->MergedFB) {
                if (pXGI->MergedFBAuto) {
                    xf86DrvMsg(pScrn->scrnIndex, X_INFO, mergednocrt1,
                               mergeddisstr);
                }
                else {
                    XGIErrorLog(pScrn, mergednocrt1, mergeddisstr);
                }
                if (pXGI->CRT2pScrn)
                    xfree(pXGI->CRT2pScrn);
                pXGI->CRT2pScrn = NULL;
                pXGI->MergedFB = FALSE;
            }
#endif
            pXGI->VBFlags |= VB_DISPMODE_SINGLE;
        }
        /* CRT1 and CRT2 - mirror or dual head ----- */
        else if (IS_DUAL_HEAD(pXGI)) {
            pXGI->VBFlags |= (VB_DISPMODE_DUAL | DISPTYPE_CRT1);
            if (pXGIEnt)
                pXGIEnt->DisableDual = FALSE;
        }
        else
            pXGI->VBFlags |= (VB_DISPMODE_MIRROR | DISPTYPE_CRT1);
    }
    else {                      /* CRT1 only ------------------------------- */
        if (IS_DUAL_HEAD(pXGI)) {
            XGIErrorLog(pScrn,
                        "No CRT2 output selected or no bridge detected. "
                        "Dual Head mode can't initialize.\n");
            if (pXGIEnt)
                pXGIEnt->ErrorAfterFirst = TRUE;
            if (pXGI->pInt)
                xf86FreeInt10(pXGI->pInt);
            pXGI->pInt = NULL;
            xgiRestoreExtRegisterLock(pXGI, srlockReg, crlockReg);
            XGIFreeRec(pScrn);
            return FALSE;
        }

#ifdef XGIMERGED
        if (pXGI->MergedFB) {
            if (pXGI->MergedFBAuto) {
                xf86DrvMsg(pScrn->scrnIndex, X_INFO, mergednocrt2,
                           mergeddisstr);
            }
            else {
                XGIErrorLog(pScrn, mergednocrt2, mergeddisstr);
            }
            if (pXGI->CRT2pScrn)
                xfree(pXGI->CRT2pScrn);
            pXGI->CRT2pScrn = NULL;
            pXGI->MergedFB = FALSE;
        }
#endif
        PDEBUG(ErrorF("3782 pXGI->VBFlags =%x\n", pXGI->VBFlags));
        pXGI->VBFlags |= (VB_DISPMODE_SINGLE | DISPTYPE_CRT1);
    }

    /* Init Ptrs for Save/Restore functions and calc MaxClock */
    XGIDACPreInit(pScrn);

    /* ********** end of VBFlags setup ********** */

    /* VBFlags are initialized now. Back them up for SlaveMode modes. */
    pXGI->VBFlags_backup = pXGI->VBFlags;

    /* Find out about paneldelaycompensation and evaluate option */
    if (!IS_DUAL_HEAD(pXGI) || !IS_SECOND_HEAD(pXGI)) {

    }

    /* In dual head mode, both heads (currently) share the maxxfbmem equally.
     * If memory sharing is done differently, the following has to be changed;
     * the other modules (eg. accel and Xv) use dhmOffset for hardware
     * pointer settings relative to VideoRAM start and won't need to be changed.
     */
    if (IS_DUAL_HEAD(pXGI)) {
        if (!IS_SECOND_HEAD(pXGI)) {
            /* ===== First head (always CRT2) ===== */
            /* We use only half of the memory available */
            pXGI->maxxfbmem /= 2;
            /* Initialize dhmOffset */
            pXGI->dhmOffset = 0;
            /* Copy framebuffer addresses & sizes to entity */
            pXGIEnt->masterFbAddress = pXGI->FbAddress;
            pXGIEnt->masterFbSize = pXGI->maxxfbmem;
            pXGIEnt->slaveFbAddress = pXGI->FbAddress + pXGI->maxxfbmem;
            pXGIEnt->slaveFbSize = pXGI->maxxfbmem;
            xf86DrvMsg(pScrn->scrnIndex, X_PROBED,
                       "%ldKB video RAM at 0x%lx available for master head (CRT2)\n",
                       pXGI->maxxfbmem / 1024, pXGI->FbAddress);
        }
        else {
            /* ===== Second head (always CRT1) ===== */
            /* We use only half of the memory available */
            pXGI->maxxfbmem /= 2;
            /* Adapt FBAddress */
            pXGI->FbAddress += pXGI->maxxfbmem;
            /* Initialize dhmOffset */
            pXGI->dhmOffset = pXGI->availMem - pXGI->maxxfbmem;
            xf86DrvMsg(pScrn->scrnIndex, X_PROBED,
                       "%ldKB video RAM at 0x%lx available for slave head (CRT1)\n",
                       pXGI->maxxfbmem / 1024, pXGI->FbAddress);
        }
    }
    else
        pXGI->dhmOffset = 0;

    /* Note: Do not use availMem for anything from now. Use
     * maxxfbmem instead. (availMem does not take dual head
     * mode into account.)
     */

#if !defined(__arm__) 
#if !defined(__powerpc__)
    /* Now load and initialize VBE module. */
    if (xf86LoadSubModule(pScrn, "vbe")) {
#if 0
        xf86LoaderReqSymLists(vbeSymbols, NULL);
#endif
        pXGI->pVbe = VBEExtendedInit(pXGI->pInt, pXGI->pEnt->index,
                                     SET_BIOS_SCRATCH | RESTORE_BIOS_SCRATCH);
        if (!pXGI->pVbe) {
            xf86DrvMsg(pScrn->scrnIndex, X_WARNING,
                       "Could not initialize VBE module for DDC\n");
        }
    }
    else {
        xf86DrvMsg(pScrn->scrnIndex, X_WARNING,
                   "Could not load VBE module\n");
    }

#endif
#endif


    XGIDDCPreInit(pScrn);

    /* Jong 07/29/2009; Proposal : use wide range for HorizSync and strict range for VertRefresh; And set 1024x768 in Modes of Screen section */
	/* Jong 07/17/2009; fix issue of only one mode (800x600) */
    /* if (no Horizsync or VertRefresh is spefified in Monitor section) and (no DDC detection) */
    /* then apply followings as default Hsync and VRefresh (1024x768x60HZ) */
    /* XGIDDCPreInit() should be called first to get EDID but need I2C programming instead of VBIOS call */
	if(pScrn->monitor->DDC == NULL)
	{
		ErrorF("Non-DDC minitor or NO EDID information...\n");

		if(pScrn->monitor->nHsync == 0)
		{
			pScrn->monitor->nHsync = 1;
			pScrn->monitor->hsync[0].lo=30;
			pScrn->monitor->hsync[0].hi=50;
			ErrorF("No HorizSync information set in Monitor section and use default (%g, %g)...\n", 
				pScrn->monitor->hsync[0].lo, pScrn->monitor->hsync[0].hi);
		}

		if(pScrn->monitor->nVrefresh == 0)
		{
			pScrn->monitor->nVrefresh = 1;
			pScrn->monitor->vrefresh[0].lo=40;
			pScrn->monitor->vrefresh[0].hi=60;
			ErrorF("No VertRefresh information set in Monitor section and use default (%g, %g)...\n", 
				pScrn->monitor->vrefresh[0].lo, pScrn->monitor->vrefresh[0].hi);
		}
	}

    /* From here, we mainly deal with clocks and modes */

    /* Set the min pixel clock */
    pXGI->MinClock = 5000;

    xf86DrvMsg(pScrn->scrnIndex, X_DEFAULT, "Min pixel clock is %d MHz\n",
               pXGI->MinClock / 1000);

    from = X_PROBED;
    /*
     * If the user has specified ramdac speed in the XF86Config
     * file, we respect that setting.
     */
    if (pXGI->pEnt->device->dacSpeeds[0]) {
        int speed = 0;
        switch (pScrn->bitsPerPixel) {
        case 8:
            speed = pXGI->pEnt->device->dacSpeeds[DAC_BPP8];
            break;
        case 16:
            speed = pXGI->pEnt->device->dacSpeeds[DAC_BPP16];
            break;
        case 24:
            speed = pXGI->pEnt->device->dacSpeeds[DAC_BPP24];
            break;
        case 32:
            speed = pXGI->pEnt->device->dacSpeeds[DAC_BPP32];
            break;
        }
        if (speed == 0)
            pXGI->MaxClock = pXGI->pEnt->device->dacSpeeds[0];
        else
            pXGI->MaxClock = speed;
        from = X_CONFIG;
    }
    xf86DrvMsg(pScrn->scrnIndex, from, "Max pixel clock is %d MHz\n",
               pXGI->MaxClock / 1000);

    /*
     * Setup the ClockRanges, which describe what clock ranges are available,
     * and what sort of modes they can be used for.
     */
    clockRanges = xnfcalloc(sizeof(ClockRange), 1);
    clockRanges->next = NULL;
    clockRanges->minClock = pXGI->MinClock;
    clockRanges->maxClock = pXGI->MaxClock;
    clockRanges->clockIndex = -1;       /* programmable */
    clockRanges->interlaceAllowed = TRUE;
    clockRanges->doubleScanAllowed = TRUE;

    /*
     * xf86ValidateModes will check that the mode HTotal and VTotal values
     * don't exceed the chipset's limit if pScrn->maxHValue and
     * pScrn->maxVValue are set.  Since our XGIValidMode() already takes
     * care of this, we don't worry about setting them here.
     */

    /* Select valid modes from those available */
#ifdef XGIMERGED
    pXGI->CheckForCRT2 = FALSE;
#endif
    XGIDumpMonPtr(pScrn->monitor);


	XGIAddAvailableModes(pScrn->monitor->Modes);

	/* XGIFilterModeByDDC(pScrn->monitor->Modes, g_pMonitorDVI); */ /* Do it in XGIValidMode() */

	ErrorF("Call xf86ValidateModes()...Use Virtual Size-1-Virtual Size=%d\n", pScrn->display->virtualX);
    i = xf86ValidateModes(pScrn, pScrn->monitor->Modes, pScrn->display->modes, clockRanges, NULL, 256, 2048,    /* min / max pitch */
                          pScrn->bitsPerPixel * 8, 128, 2048,   /* min / max height */
                          pScrn->display->virtualX,
                          pScrn->display->virtualY,
                          pXGI->maxxfbmem, LOOKUP_BEST_REFRESH);

    if (i == -1) {
        XGIErrorLog(pScrn, "xf86ValidateModes() error\n");

        if (pXGIEnt)
            pXGIEnt->ErrorAfterFirst = TRUE;

        if (pXGI->pInt)
            xf86FreeInt10(pXGI->pInt);
        xgiRestoreExtRegisterLock(pXGI, srlockReg, crlockReg);
        XGIFreeRec(pScrn);
        return FALSE;
    }

    /* Check the virtual screen against the available memory */

    memreq = (pScrn->virtualX * ((pScrn->bitsPerPixel + 7) / 8)) 
	* pScrn->virtualY;

    if (memreq > pXGI->maxxfbmem) {
		XGIErrorLog(pScrn,
				"Virtual screen too big for memory; %ldK needed, %ldK available\n",
				memreq / 1024, pXGI->maxxfbmem / 1024);

		if (pXGIEnt)
			pXGIEnt->ErrorAfterFirst = TRUE;

		if (pXGI->pInt)
			xf86FreeInt10(pXGI->pInt);
		pXGI->pInt = NULL;
		xgiRestoreExtRegisterLock(pXGI, srlockReg, crlockReg);
		XGIFreeRec(pScrn);
		return FALSE;
    }
    else if (pXGI->loadDRI && !IS_DUAL_HEAD(pXGI)) 
	{
		pXGI->maxxfbmem = memreq;
		pXGI->DRIheapstart = pXGI->DRIheapend = 0;

		if (pXGI->maxxfbmem == pXGI->availMem) {
			xf86DrvMsg(pScrn->scrnIndex, X_WARNING,
				   "All video memory used for framebuffer.  DRI will be disabled.\n");
			pXGI->loadDRI = FALSE;
		}
		else {
			pXGI->DRIheapstart = pXGI->maxxfbmem;
			pXGI->DRIheapend = pXGI->availMem;
		}
    }


    /* Dual Head:
     * -) Go through mode list and mark all those modes as bad,
     *    which are unsuitable for dual head mode.
     * -) Find the highest used pixelclock on the master head.
     */
    if (IS_DUAL_HEAD(pXGI) && !IS_SECOND_HEAD(pXGI)) 
	{
        pXGIEnt->maxUsedClock = 0;

        if ((p = first = pScrn->modes)) 
		{
            do {
                n = p->next;

                /* Modes that require the bridge to operate in SlaveMode
                 * are not suitable for Dual Head mode.
                 */

                /* Search for the highest clock on first head in order to calculate
                 * max clock for second head (CRT1)
                 */
                if ((p->status == MODE_OK)
                    && (p->Clock > pXGIEnt->maxUsedClock)) {
                    pXGIEnt->maxUsedClock = p->Clock;
                }

                p = n;

            } while (p != NULL && p != first);
        }
    }

    /* Prune the modes marked as invalid */
    xf86PruneDriverModes(pScrn);

    if (i == 0 || pScrn->modes == NULL) {
        XGIErrorLog(pScrn, "No valid modes found\n");

        if (pXGIEnt)
            pXGIEnt->ErrorAfterFirst = TRUE;

        if (pXGI->pInt)
            xf86FreeInt10(pXGI->pInt);
        xgiRestoreExtRegisterLock(pXGI, srlockReg, crlockReg);
        XGIFreeRec(pScrn);
        return FALSE;
    }

    xf86SetCrtcForModes(pScrn, INTERLACE_HALVE_V);

    /* Set the current mode to the first in the list */
    pScrn->currentMode = pScrn->modes;

    /* Copy to CurrentLayout */
    pXGI->CurrentLayout.mode = pScrn->currentMode;
    pXGI->CurrentLayout.displayWidth = pScrn->displayWidth;

#ifdef XGIMERGED
    if (pXGI->MergedFB) {
        xf86DrvMsg(pScrn->scrnIndex, X_INFO, modesforstr, 1);
    }
#endif

    /* Print the list of modes being used ; call xf86Mode.c-xf86PrintModeline() to print */
	ErrorF("Call xf86PrintModes(pScrn) to list all valid modes...\n");
    xf86PrintModes(pScrn);

#ifdef XGIMERGED
    if (pXGI->MergedFB) {
        BOOLEAN acceptcustommodes = TRUE;
        BOOLEAN includelcdmodes = TRUE;
        BOOLEAN isfordvi = FALSE;

        xf86DrvMsg(pScrn->scrnIndex, X_INFO, crtsetupstr, 2);

        clockRanges->next = NULL;
        clockRanges->minClock = pXGI->MinClock;
        clockRanges->clockIndex = -1;
        clockRanges->interlaceAllowed = FALSE;
        clockRanges->doubleScanAllowed = FALSE;

        xf86DrvMsg(pScrn->scrnIndex, X_DEFAULT,
                   "Min pixel clock for CRT2 is %d MHz\n",
                   clockRanges->minClock / 1000);
        xf86DrvMsg(pScrn->scrnIndex, X_DEFAULT,
                   "Max pixel clock for CRT2 is %d MHz\n",
                   clockRanges->maxClock / 1000);

        if ((pXGI->XGI_Pr->
             VBType & (VB_XGI301 | VB_XGI301B | VB_XGI301C | VB_XGI302B)))
        {
            if (!(pXGI->VBFlags & (CRT2_LCD | CRT2_VGA)))
                includelcdmodes = FALSE;
            if (pXGI->VBFlags & CRT2_LCD)
                isfordvi = TRUE;
            if (pXGI->VBFlags & CRT2_TV)
                acceptcustommodes = FALSE;
        }
        else {
            includelcdmodes = FALSE;
            acceptcustommodes = FALSE;
        }
    }

    if (pXGI->MergedFB) {

        pXGI->CheckForCRT2 = TRUE;
        i = xf86ValidateModes(pXGI->CRT2pScrn,
                              pXGI->CRT2pScrn->monitor->Modes,
                              pXGI->CRT2pScrn->display->modes, clockRanges,
                              NULL, 256, 4088,
                              pXGI->CRT2pScrn->bitsPerPixel * 8, 128, 4096,
                              pScrn->display->virtualX ? pScrn->virtualX : 0,
                              pScrn->display->virtualY ? pScrn->virtualY : 0,
                              pXGI->maxxfbmem, LOOKUP_BEST_REFRESH);
        pXGI->CheckForCRT2 = FALSE;

        if (i == -1) {
            XGIErrorLog(pScrn, "xf86ValidateModes() error, %s.\n",
                        mergeddisstr);
            XGIFreeCRT2Structs(pXGI);
            pXGI->MergedFB = FALSE;
        }

    }

    if (pXGI->MergedFB) {

        if ((p = first = pXGI->CRT2pScrn->modes)) {
            do {
                n = p->next;
                p = n;
            } while (p != NULL && p != first);
        }

        xf86PruneDriverModes(pXGI->CRT2pScrn);

        if (i == 0 || pXGI->CRT2pScrn->modes == NULL) {
            XGIErrorLog(pScrn, "No valid modes found for CRT2; %s\n",
                        mergeddisstr);
            XGIFreeCRT2Structs(pXGI);
            pXGI->MergedFB = FALSE;
        }

    }

    if (pXGI->MergedFB) {

        xf86SetCrtcForModes(pXGI->CRT2pScrn, INTERLACE_HALVE_V);

        xf86DrvMsg(pScrn->scrnIndex, X_INFO, modesforstr, 2);

        xf86PrintModes(pXGI->CRT2pScrn);

        pXGI->CRT1Modes = pScrn->modes;
        pXGI->CRT1CurrentMode = pScrn->currentMode;

        xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                   "Generating MergedFB mode list\n");

        pScrn->modes = XGIGenerateModeList(pScrn, pXGI->MetaModes,
                                           pXGI->CRT1Modes,
                                           pXGI->CRT2pScrn->modes,
                                           pXGI->CRT2Position);

        if (!pScrn->modes) {

            XGIErrorLog(pScrn,
                        "Failed to parse MetaModes or no modes found. %s.\n",
                        mergeddisstr);
            XGIFreeCRT2Structs(pXGI);
            pScrn->modes = pXGI->CRT1Modes;
            pXGI->CRT1Modes = NULL;
            pXGI->MergedFB = FALSE;

        }

    }

    if (pXGI->MergedFB) {

        /* If no virtual dimension was given by the user,
         * calculate a sane one now. Adapts pScrn->virtualX,
         * pScrn->virtualY and pScrn->displayWidth.
         */
        XGIRecalcDefaultVirtualSize(pScrn);

        pScrn->modes = pScrn->modes->next;      /* We get the last from GenerateModeList(), skip to first */
        pScrn->currentMode = pScrn->modes;

        /* Update CurrentLayout */
        pXGI->CurrentLayout.mode = pScrn->currentMode;
        pXGI->CurrentLayout.displayWidth = pScrn->displayWidth;

    }
#endif

    /* Set display resolution */
#ifdef XGIMERGED
    if (pXGI->MergedFB) {
        XGIMergedFBSetDpi(pScrn, pXGI->CRT2pScrn, pXGI->CRT2Position);
    }
    else
#endif


	/* Jong 07/30/2009; might cause small font size */
	xf86SetDpi(pScrn, 0, 0);

#if 0
	/*yilin@20080407 fix the font too small problem at low resolution*/
	if((pScrn->xDpi < 65)||(pScrn->yDpi < 65)) 
	{
		  pScrn->xDpi = 75;
		  pScrn->yDpi = 75;
	}
#endif

    /* Load fb module */
    switch (pScrn->bitsPerPixel) {
    case 8:
    case 16:
    case 24:
    case 32:
        if (!xf86LoadSubModule(pScrn, "fb")) {
            XGIErrorLog(pScrn, "Failed to load fb module");

            if (pXGIEnt)
                pXGIEnt->ErrorAfterFirst = TRUE;

            if (pXGI->pInt)
                xf86FreeInt10(pXGI->pInt);
            xgiRestoreExtRegisterLock(pXGI, srlockReg, crlockReg);
            XGIFreeRec(pScrn);
            return FALSE;
        }
        break;
    default:
        XGIErrorLog(pScrn, "Unsupported framebuffer bpp (%d)\n",
                    pScrn->bitsPerPixel);

        if (pXGIEnt)
            pXGIEnt->ErrorAfterFirst = TRUE;

        if (pXGI->pInt)
            xf86FreeInt10(pXGI->pInt);
        xgiRestoreExtRegisterLock(pXGI, srlockReg, crlockReg);
        XGIFreeRec(pScrn);
        return FALSE;
    }
#if 0
    xf86LoaderReqSymLists(fbSymbols, NULL);
#endif
    /* Load XAA if needed */
    if (!pXGI->NoAccel) 
	{
        xf86DrvMsg(pScrn->scrnIndex, X_INFO, "Accel enabled\n");

#ifdef XGI_USE_XAA
		if(!(pXGI->useEXA))
		{
			if (!xf86LoadSubModule(pScrn, "xaa")) {
				XGIErrorLog(pScrn, "Could not load xaa module\n");

				if (pXGIEnt)
					pXGIEnt->ErrorAfterFirst = TRUE;

				if (pXGI->pInt)
					xf86FreeInt10(pXGI->pInt);
				xgiRestoreExtRegisterLock(pXGI, srlockReg, crlockReg);
				XGIFreeRec(pScrn);
				return FALSE;
			}
#if 0
			xf86LoaderReqSymLists(xaaSymbols, NULL);
#endif
		}
#endif

#ifdef XGI_USE_EXA
		if(pXGI->useEXA)
		{
		   if(!xf86LoadSubModule(pScrn, "exa")) {
			  XGIErrorLog(pScrn, "Could not load exa module\n");
			  return FALSE;
		   }
#if 0
		   xf86LoaderReqSymLists(exaSymbols, NULL);
#endif
		}
#endif
	}

    /* Load shadowfb if needed */
    if (pXGI->ShadowFB) {
        if (!xf86LoadSubModule(pScrn, "shadowfb")) {
            XGIErrorLog(pScrn, "Could not load shadowfb module\n");

            if (pXGIEnt)
                pXGIEnt->ErrorAfterFirst = TRUE;

            if (pXGI->pInt)
                xf86FreeInt10(pXGI->pInt);
            xgiRestoreExtRegisterLock(pXGI, srlockReg, crlockReg);
            XGIFreeRec(pScrn);
            return FALSE;
        }
#if 0
        xf86LoaderReqSymLists(shadowSymbols, NULL);
#endif
    }

    /* Load the dri module if requested. */
#ifdef XF86DRI
    if(pXGI->loadDRI) {
        if (xf86LoadSubModule(pScrn, "dri")) {
#if 0
            xf86LoaderReqSymLists(driSymbols, drmSymbols, NULL);
#endif
        }
        else {
            if (!IS_DUAL_HEAD(pXGI))
                xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
                           "Remove >Load \"dri\"< from the Module section of your XF86Config file\n");
        }
    }
#endif


    /* Now load and initialize VBE module for VESA and mode restoring. */
    if (pXGI->pVbe) {
        vbeFree(pXGI->pVbe);
        pXGI->pVbe = NULL;
    }

#ifdef XGIDUALHEAD
    xf86SetPrimInitDone(pScrn->entityList[0]);
#endif

    xgiRestoreExtRegisterLock(pXGI, srlockReg, crlockReg);

    if (pXGI->pInt)
        xf86FreeInt10(pXGI->pInt);
    pXGI->pInt = NULL;

    if (IS_DUAL_HEAD(pXGI)) {
        pXGI->XGI_SD_Flags |= XGI_SD_ISDUALHEAD;
        if (IS_SECOND_HEAD(pXGI))
            pXGI->XGI_SD_Flags |= XGI_SD_ISDHSECONDHEAD;
        else
            pXGI->XGI_SD_Flags &= ~(XGI_SD_SUPPORTXVGAMMA1);
#ifdef PANORAMIX
        if (!noPanoramiXExtension) {
            pXGI->XGI_SD_Flags |= XGI_SD_ISDHXINERAMA;
            pXGI->XGI_SD_Flags &= ~(XGI_SD_SUPPORTXVGAMMA1);
        }
#endif
    }

#ifdef XGIMERGED
    if (pXGI->MergedFB)
        pXGI->XGI_SD_Flags |= XGI_SD_ISMERGEDFB;
#endif

    if (pXGI->enablexgictrl)
        pXGI->XGI_SD_Flags |= XGI_SD_ENABLED;

    return TRUE;
}


/*
 * Map the framebuffer and MMIO memory.
 */

static Bool
XGIMapMem(ScrnInfoPtr pScrn)
{
    XGIPtr pXGI = XGIPTR(pScrn);;

#ifdef XSERVER_LIBPCIACCESS
    unsigned i;

    for (i = 0; i < 2; i++) {
        int err;
        
        err = pci_device_map_region(pXGI->PciInfo, i, TRUE);
        if (err) {
            xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
                       "Internal error: cound not map PCI region %u\n", i);
            return FALSE;
        }
    }

    pXGI->FbBase = pXGI->PciInfo->regions[0].memory;
    pXGI->IOBase = pXGI->PciInfo->regions[1].memory;
#else
    int mmioFlags;

    /*
     * Map IO registers to virtual address space
     */
#if !defined(__alpha__)
    mmioFlags = VIDMEM_MMIO;
#else
    /*
     * For Alpha, we need to map SPARSE memory, since we need
     * byte/short access.
     */
    mmioFlags = VIDMEM_MMIO | VIDMEM_SPARSE;
#endif
    pXGI->IOBase = xf86MapPciMem(pScrn->scrnIndex, mmioFlags,
                                 pXGI->PciTag, pXGI->IOAddress, 0x10000);
    if (pXGI->IOBase == NULL)
        return FALSE;

#ifdef __alpha__
    /*
     * for Alpha, we need to map DENSE memory as well, for
     * setting CPUToScreenColorExpandBase.
     */
    pXGI->IOBaseDense = xf86MapPciMem(pScrn->scrnIndex, VIDMEM_MMIO,
                                      pXGI->PciTag, pXGI->IOAddress, 0x10000);

    if (pXGI->IOBaseDense == NULL)
        return FALSE;
#endif /* __alpha__ */

    pXGI->FbBase = xf86MapPciMem(pScrn->scrnIndex, VIDMEM_FRAMEBUFFER,
                                 pXGI->PciTag,
                                 (unsigned long) pXGI->FbAddress,
                                 pXGI->FbMapSize);

    PDEBUG(ErrorF("pXGI->FbBase = 0x%08lx\n", (ULONG) (pXGI->FbBase)));

    if (pXGI->FbBase == NULL)
        return FALSE;
#endif

    return TRUE;
}


/*
 * Unmap the framebuffer and MMIO memory.
 */

static Bool
XGIUnmapMem(ScrnInfoPtr pScrn)
{
    XGIPtr pXGI = XGIPTR(pScrn);
    XGIEntPtr pXGIEnt = ENTITY_PRIVATE(pXGI);


    /* In dual head mode, we must not unmap if the other head still
     * assumes memory as mapped
     */
    if (IS_DUAL_HEAD(pXGI)) {
        if (pXGIEnt->MapCountIOBase) {
            pXGIEnt->MapCountIOBase--;
            if ((pXGIEnt->MapCountIOBase == 0) || (pXGIEnt->forceUnmapIOBase)) {
#ifdef XSERVER_LIBPCIACCESS
                pci_device_unmap_region(pXGI->PciInfo, 1);
#else
                xf86UnMapVidMem(pScrn->scrnIndex, (pointer) pXGIEnt->IOBase,
                                (pXGI->mmioSize * 1024));
#endif
                pXGIEnt->IOBase = NULL;
                pXGIEnt->MapCountIOBase = 0;
                pXGIEnt->forceUnmapIOBase = FALSE;
            }
            pXGI->IOBase = NULL;
        }
#ifdef __alpha__
#ifdef XSERVER_LIBPCIACCESS
#error "How to do dense mapping on Alpha?"
#else
        if (pXGIEnt->MapCountIOBaseDense) {
            pXGIEnt->MapCountIOBaseDense--;
            if ((pXGIEnt->MapCountIOBaseDense == 0)
                || (pXGIEnt->forceUnmapIOBaseDense)) {
                xf86UnMapVidMem(pScrn->scrnIndex,
                                (pointer) pXGIEnt->IOBaseDense,
                                (pXGI->mmioSize * 1024));
                pXGIEnt->IOBaseDense = NULL;
                pXGIEnt->MapCountIOBaseDense = 0;
                pXGIEnt->forceUnmapIOBaseDense = FALSE;
            }
            pXGI->IOBaseDense = NULL;
        }
#endif
#endif /* __alpha__ */
        if (pXGIEnt->MapCountFbBase) {
            pXGIEnt->MapCountFbBase--;
            if ((pXGIEnt->MapCountFbBase == 0) || (pXGIEnt->forceUnmapFbBase)) {
#ifdef XSERVER_LIBPCIACCESS
                pci_device_unmap_region(pXGI->PciInfo, 0);
#else
                xf86UnMapVidMem(pScrn->scrnIndex, (pointer) pXGIEnt->FbBase,
                                pXGI->FbMapSize);
#endif
                pXGIEnt->FbBase = NULL;
                pXGIEnt->MapCountFbBase = 0;
                pXGIEnt->forceUnmapFbBase = FALSE;

            }
            pXGI->FbBase = NULL;
        }
    }
    else {
#ifdef XSERVER_LIBPCIACCESS
        pci_device_unmap_region(pXGI->PciInfo, 0);
        pci_device_unmap_region(pXGI->PciInfo, 1);
#else
        xf86UnMapVidMem(pScrn->scrnIndex, (pointer) pXGI->IOBase,
                        (pXGI->mmioSize * 1024));
        xf86UnMapVidMem(pScrn->scrnIndex, (pointer) pXGI->FbBase,
                        pXGI->FbMapSize);
#endif
        pXGI->IOBase = NULL;
        pXGI->FbBase = NULL;

#ifdef __alpha__
#ifdef XSERVER_LIBPCIACCESS
#error "How to do dense mapping on Alpha?"
#else
        xf86UnMapVidMem(pScrn->scrnIndex, (pointer) pXGI->IOBaseDense,
                        (pXGI->mmioSize * 1024));
        pXGI->IOBaseDense = NULL;
#endif
#endif
    }

    return TRUE;
}

/*
 * This function saves the video state.
 */
static void
XGISave(ScrnInfoPtr pScrn)
{
    XGIPtr pXGI;
    vgaRegPtr vgaReg;
    XGIRegPtr xgiReg;

    PDEBUG(ErrorF("XGISave()\n"));

    pXGI = XGIPTR(pScrn);

    /* We always save master & slave */
    if (IS_DUAL_HEAD(pXGI) && IS_SECOND_HEAD(pXGI))
        return;

    vgaReg = &VGAHWPTR(pScrn)->SavedReg;
    xgiReg = &pXGI->SavedReg;

    vgaHWSave(pScrn, vgaReg, VGA_SR_ALL);

    xgiSaveUnlockExtRegisterLock(pXGI, &xgiReg->xgiRegs3C4[0x05],
                                 &xgiReg->xgiRegs3D4[0x80]);

    (*pXGI->XGISave) (pScrn, xgiReg);

    /* "Save" these again as they may have been changed prior to XGISave() call */
}


/*
 * Initialise a new mode.  This is currently done using the
 * "initialise struct, restore/write struct to HW" model for
 * the old chipsets (5597/530/6326). For newer chipsets,
 * we use our own mode switching code (or VESA).
 */

static Bool
XGIModeInit(ScrnInfoPtr pScrn, DisplayModePtr mode)
{
    vgaHWPtr hwp = VGAHWPTR(pScrn);
    vgaRegPtr vgaReg;
    XGIPtr pXGI = XGIPTR(pScrn);
    XGIRegPtr xgiReg;
#ifdef __powerpc__
    unsigned char tmpval;
#endif

    PDEBUG(ErrorF("XGIModeInit\n"));
    PDEBUG(ErrorF("mode->HDisplay = %d\n", mode->HDisplay));
    PDEBUG(ErrorF("mode->VDisplay = %d\n", mode->VDisplay));

	PDEBUG(ErrorF("Before update...\n"));
    PDEBUG(ErrorF("pScrn->virtualX = %d\n", pScrn->virtualX));
    PDEBUG(ErrorF("pScrn->virtualY = %d\n", pScrn->virtualY));
    PDEBUG(ErrorF("pScrn->displayWidth = %d\n", pScrn->displayWidth));
    PDEBUG(ErrorF("pScrn->frameX0 = %d\n", pScrn->frameX0));
    PDEBUG(ErrorF("pScrn->frameY0 = %d\n", pScrn->frameY0));
    PDEBUG(ErrorF("pScrn->frameX1 = %d\n", pScrn->frameX1));
    PDEBUG(ErrorF("pScrn->frameY1 = %d\n", pScrn->frameY1));

	/* pScrn->displayWidth=mode->HDisplay; */

	if(pXGI->TargetRefreshRate)
			mode->VRefresh = pXGI->TargetRefreshRate;

	if((pScrn->monitor->DDC == NULL) && (pXGI->Non_DDC_DefaultMode))
	{
		mode->HDisplay = pXGI->Non_DDC_DefaultResolutionX;
		mode->VDisplay = pXGI->Non_DDC_DefaultResolutionY;
		mode->VRefresh = pXGI->Non_DDC_DefaultRefreshRate;
	}

    /* PDEBUG(ErrorF("XGIModeInit(). \n")); */
    PDEBUG(ErrorF
           ("XGIModeInit Resolution (%d, %d) \n", mode->HDisplay,
            mode->VDisplay));
    PDEBUG(ErrorF("XGIModeInit VVRefresh (%8.3f) \n", mode->VRefresh));
    PDEBUG(ErrorF("XGIModeInit Color Depth (%d) \n", pScrn->depth));

    /* Jong Lin 08-26-2005; save current mode */
    Volari_SetDefaultIdleWait(pXGI, mode->HDisplay, pScrn->depth);

    andXGIIDXREG(XGICR, 0x11, 0x7f);    /* Unlock CRTC registers */

    XGIModifyModeInfo(mode);    /* Quick check of the mode parameters */


    if (IS_DUAL_HEAD(pXGI)) 
	{
        XGIEntPtr pXGIEnt = ENTITY_PRIVATE(pXGI);

		if (!(*pXGI->ModeInit) (pScrn, mode)) {
			XGIErrorLog(pScrn, "ModeInit() failed\n");
			return FALSE;
		}

		pScrn->vtSema = TRUE;

		/* Head 2 (slave) is always CRT1 */
		XGIPreSetMode(pScrn, mode, XGI_MODE_CRT1);
		if (!XGIBIOSSetModeCRT1(pXGI->XGI_Pr, &pXGI->xgi_HwDevExt, pScrn, 
					mode)) 
		{
			XGIErrorLog(pScrn, "XGIBIOSSetModeCRT1() failed\n");
			return FALSE;
		}

		XGIPostSetMode(pScrn, &pXGI->ModeReg);
		XGIAdjustFrame(pXGIEnt->pScrn_1->scrnIndex, pXGIEnt->pScrn_1->frameX0,
				   pXGIEnt->pScrn_1->frameY0, 0);
    }
    else
    {
		/* For other chipsets, use the old method */

		/* Initialise the ModeReg values */
		if (!vgaHWInit(pScrn, mode)) {
			XGIErrorLog(pScrn, "vgaHWInit() failed\n");
			return FALSE;
		}

		/* Reset our PIOOffset as vgaHWInit might have reset it */
		VGAHWPTR(pScrn)->PIOOffset = pXGI->IODBase - 0x380 +
#ifdef XSERVER_LIBPCIACCESS
        (pXGI->PciInfo->regions[2].base_addr & 0xFFFC)
#else
        (pXGI->PciInfo->ioBase[2] & 0xFFFC)
#endif
        ;

		/* Prepare the register contents */
		if (!(*pXGI->ModeInit) (pScrn, mode)) {
			XGIErrorLog(pScrn, "ModeInit() failed\n");
			return FALSE;
		}

		pScrn->vtSema = TRUE;

		/* Program the registers */
		vgaHWProtect(pScrn, TRUE);
		vgaReg = &hwp->ModeReg;
		xgiReg = &pXGI->ModeReg;

		vgaReg->Attribute[0x10] = 0x01;
		if (pScrn->bitsPerPixel > 8) {
			vgaReg->Graphics[0x05] = 0x00;
		}

		vgaHWRestore(pScrn, vgaReg, VGA_SR_MODE);

		(*pXGI->XGIRestore) (pScrn, xgiReg);

#ifdef TWDEBUG
		xf86DrvMsg(pScrn->scrnIndex, X_INFO,
			   "REAL REGISTER CONTENTS AFTER SETMODE:\n");
		(*pXGI->ModeInit) (pScrn, mode);
#endif

		vgaHWProtect(pScrn, FALSE);
    }


	if((pXGI->Chipset == PCI_CHIP_XGIXG40)||(pXGI->Chipset == PCI_CHIP_XGIXG20)||(pXGI->Chipset == PCI_CHIP_XGIXG21)||(pXGI->Chipset == PCI_CHIP_XGIXG27))
	   {
        /* PDEBUG(XGIDumpRegs(pScrn)) ; */
        PDEBUG(ErrorF(" *** PreSetMode(). \n"));
        XGIPreSetMode(pScrn, mode, XGI_MODE_SIMU);
        /* PDEBUG(XGIDumpRegs(pScrn)) ; */
        PDEBUG(ErrorF(" *** Start SetMode() \n"));

        if (!XGIBIOSSetMode(pXGI->XGI_Pr, &pXGI->xgi_HwDevExt, pScrn, mode)) {
            XGIErrorLog(pScrn, "XGIBIOSSetModeCRT() failed\n");
            return FALSE;
        }
        Volari_EnableAccelerator(pScrn);
        /* XGIPostSetMode(pScrn, &pXGI->ModeReg); */
        /* outXGIIDXREG(XGISR, 0x20, 0xA1) ; */
        /* PDEBUG(XGIDumpRegs(pScrn)) ; */
    }

    /* Update Currentlayout */
    pXGI->CurrentLayout.mode = mode;

#ifdef __powerpc__
    inXGIIDXREG(XGICR, 0x4D, tmpval);
    if (pScrn->depth == 16)
        tmpval = (tmpval & 0xE0) | 0x0B;        //word swap
    else if (pScrn->depth == 24)
        tmpval = (tmpval & 0xE0) | 0x15;        //dword swap
    else
        tmpval = tmpval & 0xE0; // no swap

    outXGIIDXREG(XGICR, 0x4D, tmpval);
#endif

	XGISetDPMS(pScrn, pXGI->XGI_Pr, &pXGI->xgi_HwDevExt , 0x00000000 ); 

    return TRUE;
}


/*
 * Restore the initial mode. To be used internally only!
 */
static void
XGIRestore(ScrnInfoPtr pScrn)
{
    XGIPtr pXGI = XGIPTR(pScrn);
    XGIRegPtr xgiReg = &pXGI->SavedReg;
    vgaHWPtr hwp = VGAHWPTR(pScrn);
    vgaRegPtr vgaReg = &hwp->SavedReg;


    PDEBUG(ErrorF("XGIRestore():\n"));

    /* Wait for the accelerators */
#ifdef XGI_USE_XAA
    if (!(pXGI->useEXA) && pXGI->AccelInfoPtr) {
        (*pXGI->AccelInfoPtr->Sync) (pScrn);
    }
#endif

    vgaHWProtect(pScrn, TRUE);

#ifdef UNLOCK_ALWAYS
    xgiSaveUnlockExtRegisterLock(pXGI, NULL, NULL);
#endif

	/* Volari_DisableCmdQueue(pScrn) ; */

	/* Volari_Restore() */
    (*pXGI->XGIRestore) (pScrn, xgiReg);

	pXGI->xgi_HwDevExt.SpecifyTiming = FALSE;

	/* Jong 11/14/2007; resolve no display of DVI after leaving VT */
	/* But there's no int10 for PPC... */
	/* XGIRestorePrevMode(pScrn) ; */
	/* works but mode is not exactly right because there're more than one mode 0x03 in table XGI330_SModeIDTable[] */
	XGISetModeNew( &pXGI->xgi_HwDevExt, pXGI->XGI_Pr, 0x03); 

    vgaHWProtect(pScrn, TRUE);
    if (pXGI->Primary) {
        vgaHWRestore(pScrn, vgaReg, VGA_SR_ALL);
    }

    xgiRestoreExtRegisterLock(pXGI, xgiReg->xgiRegs3C4[5],
                              xgiReg->xgiRegs3D4[0x80]);
    vgaHWProtect(pScrn, FALSE);
}


/* Our generic BlockHandler for Xv */
static void
XGIBlockHandler(int i, pointer blockData, pointer pTimeout, pointer pReadmask)
{
    ScreenPtr pScreen = screenInfo.screens[i];
    ScrnInfoPtr pScrn = xf86Screens[i];
    XGIPtr pXGI = XGIPTR(pScrn);

    pScreen->BlockHandler = pXGI->BlockHandler;
    (*pScreen->BlockHandler) (i, blockData, pTimeout, pReadmask);
    pScreen->BlockHandler = XGIBlockHandler;

    if (pXGI->VideoTimerCallback) {
        (*pXGI->VideoTimerCallback) (pScrn, currentTime.milliseconds);
    }

    if (pXGI->RenderCallback) {
        (*pXGI->RenderCallback) (pScrn);
    }
}

/* Jong@08122009 */
int  g_virtualX;
int  g_virtualY;
int  g_frameX0;
int  g_frameY0;
int  g_frameX1;
int  g_frameY1;

void xgiRestoreVirtual(ScrnInfoPtr pScrn)
{
	pScrn->virtualX = g_virtualX;
	pScrn->virtualY = g_virtualY;
	pScrn->frameX0 = g_frameX0;
	pScrn->frameY0 = g_frameY0;
	pScrn->frameX1 = g_frameX1;
	pScrn->frameY1 = g_frameY1;
}

/* Mandatory
 * This gets called at the start of each server generation
 *
 * We use pScrn and not CurrentLayout here, because the
 * properties we use have not changed (displayWidth,
 * depth, bitsPerPixel)
 *
 * pScrn->displayWidth : memory pitch
 */
static Bool
XGIScreenInit(int scrnIndex, ScreenPtr pScreen, int argc, char **argv)
{
    ScrnInfoPtr pScrn;
    vgaHWPtr hwp;
    XGIPtr pXGI;
    int ret;
    VisualPtr visual;
    unsigned long OnScreenSize;
    int height, width, displayWidth;
    unsigned char *FBStart;
    XGIEntPtr pXGIEnt = NULL;

    ErrorF("XGIScreenInit\n");
    pScrn = xf86Screens[pScreen->myNum];

    PDEBUG(ErrorF("pScrn->currentMode->HDisplay = %d\n", pScrn->currentMode->HDisplay));
    PDEBUG(ErrorF("pScrn->currentMode->VDisplay = %d\n", pScrn->currentMode->VDisplay));

	PDEBUG(ErrorF("Before update...\n"));
    PDEBUG(ErrorF("pScrn->virtualX = %d\n", pScrn->virtualX));
    PDEBUG(ErrorF("pScrn->virtualY = %d\n", pScrn->virtualY));
    PDEBUG(ErrorF("pScrn->displayWidth = %d\n", pScrn->displayWidth));
    PDEBUG(ErrorF("pScrn->frameX0 = %d\n", pScrn->frameX0));
    PDEBUG(ErrorF("pScrn->frameY0 = %d\n", pScrn->frameY0));
    PDEBUG(ErrorF("pScrn->frameX1 = %d\n", pScrn->frameX1));
    PDEBUG(ErrorF("pScrn->frameY1 = %d\n", pScrn->frameY1));

/* Jong 07/29/2009; fix bug of switch mode */
#if 1
    /* Jong 08/30/2007; no virtual screen for all cases */
    /* Jong 08/22/2007; support modeline */
    /* if(g_CountOfUserDefinedModes > 0) */
    {
		/* Jong@08122009 */
		g_virtualX = pScrn->virtualX;
		g_virtualY = pScrn->virtualY;
		g_frameX0 = pScrn->frameX0;
		g_frameY0 = pScrn->frameY0;
		g_frameX1 = pScrn->frameX1;
		g_frameY1 = pScrn->frameY1;

		/*
		pScrn->virtualX=pScrn->currentMode->HDisplay;
		pScrn->virtualY=pScrn->currentMode->VDisplay;
		*/

		//pScrn->displayWidth=pScrn->currentMode->HDisplay; 

		/*
		pScrn->frameX0=0;
		pScrn->frameY0=0;
		pScrn->frameX1=pScrn->currentMode->HDisplay-1;
		pScrn->frameY1=pScrn->currentMode->VDisplay-1; */
    }
#endif

    PDEBUG(ErrorF("After update...\n"));
    PDEBUG(ErrorF("pScrn->virtualX = %d\n", pScrn->virtualX));
    PDEBUG(ErrorF("pScrn->virtualY = %d\n", pScrn->virtualY));
    PDEBUG(ErrorF("pScrn->displayWidth = %d\n", pScrn->displayWidth));
    PDEBUG(ErrorF("pScrn->frameX0 = %d\n", pScrn->frameX0));
    PDEBUG(ErrorF("pScrn->frameY0 = %d\n", pScrn->frameY0));
    PDEBUG(ErrorF("pScrn->frameX1 = %d\n", pScrn->frameX1));
    PDEBUG(ErrorF("pScrn->frameY1 = %d\n", pScrn->frameY1));

    hwp = VGAHWPTR(pScrn);

    pXGI = XGIPTR(pScrn);

#if !defined(__arm__) 
#if !defined(__powerpc__)
    if (!IS_DUAL_HEAD(pXGI) || !IS_SECOND_HEAD(pXGI)) {
        if (xf86LoadSubModule(pScrn, "vbe")) {
#if 0
            xf86LoaderReqSymLists(vbeSymbols, NULL);
#endif
            pXGI->pVbe = VBEExtendedInit(NULL, pXGI->pEnt->index,
                                         SET_BIOS_SCRATCH |
                                         RESTORE_BIOS_SCRATCH);
        }
        else {
            XGIErrorLog(pScrn, "Failed to load VBE submodule\n");
        }
    }
#endif /* if !defined(__powerpc__)  */
#endif

    if (IS_DUAL_HEAD(pXGI)) {
        pXGIEnt = ENTITY_PRIVATE(pXGI);
        pXGIEnt->refCount++;
    }

    /* Map the VGA memory and get the VGA IO base */
    if (pXGI->Primary) {
        hwp->MapSize = 0x10000; /* Standard 64k VGA window */
        if (!vgaHWMapMem(pScrn)) {
            XGIErrorLog(pScrn, "Could not map VGA memory window\n");
            return FALSE;
        }
    }
    vgaHWGetIOBase(hwp);

    /* Patch the PIOOffset inside vgaHW to use
     * our relocated IO ports.
     */
    VGAHWPTR(pScrn)->PIOOffset = pXGI->IODBase - 0x380 +
#ifdef XSERVER_LIBPCIACCESS
        (pXGI->PciInfo->regions[2].base_addr & 0xFFFC)
#else
        (pXGI->PciInfo->ioBase[2] & 0xFFFC)
#endif
        ;

    /* Map the XGI memory and MMIO areas */
    if (!XGIMapMem(pScrn)) {
        XGIErrorLog(pScrn, "XGIMapMem() failed\n");
        return FALSE;
    }

#ifdef UNLOCK_ALWAYS
    xgiSaveUnlockExtRegisterLock(pXGI, NULL, NULL);
#endif

    /* Save the current state */
    XGISave(pScrn);


    PDEBUG(ErrorF("--- ScreenInit ---  \n"));
    PDEBUG(XGIDumpRegs(pScrn));

    /* Initialise the first mode */
    if (!XGIModeInit(pScrn, pScrn->currentMode)) {
        XGIErrorLog(pScrn, "XGIModeInit() failed\n");
        return FALSE;
    }

	/* Jong@08122009; still at virtual */
	/* xgiRestoreVirtual(); */

    PDEBUG(ErrorF("--- XGIModeInit ---  \n"));
    PDEBUG(XGIDumpRegs(pScrn));

    /* Darken the screen for aesthetic reasons */
    /* Not using Dual Head variant on purpose; we darken
     * the screen for both displays, and un-darken
     * it when the second head is finished
     */
    XGISaveScreen(pScreen, SCREEN_SAVER_ON);

    /* Set the viewport */
    XGIAdjustFrame(scrnIndex, pScrn->frameX0, pScrn->frameY0, 0); 
    /* XGIAdjustFrame(scrnIndex, 0, 0, 0); */

	/* xgiRestoreVirtual(pScrn); */

    /*
     * The next step is to setup the screen's visuals, and initialise the
     * framebuffer code.  In cases where the framebuffer's default
     * choices for things like visual layouts and bits per RGB are OK,
     * this may be as simple as calling the framebuffer's ScreenInit()
     * function.  If not, the visuals will need to be setup before calling
     * a fb ScreenInit() function and fixed up after.
     *
     * For most PC hardware at depths >= 8, the defaults that cfb uses
     * are not appropriate.  In this driver, we fixup the visuals after.
     */

    /*
     * Reset visual list.
     */
    miClearVisualTypes();

    /* Setup the visuals we support. */

    /*
     * For bpp > 8, the default visuals are not acceptable because we only
     * support TrueColor and not DirectColor.
     */
    if (!miSetVisualTypes(pScrn->depth,
                          (pScrn->bitsPerPixel > 8) ?
                          TrueColorMask : miGetDefaultVisualMask(pScrn->
                                                                 depth),
                          pScrn->rgbBits, pScrn->defaultVisual)) {
        XGISaveScreen(pScreen, SCREEN_SAVER_OFF);
        XGIErrorLog(pScrn, "miSetVisualTypes() failed (bpp %d)\n",
                    pScrn->bitsPerPixel);
        return FALSE;
    }

	/*xgiRestoreVirtual(pScrn); */

#if 0
	ErrorF("Use Virtual Size - *1\n");
    width = pScrn->virtualX;
    height = pScrn->virtualY;
    displayWidth = pScrn->displayWidth;
#endif

    if (pXGI->Rotate) {
        height = pScrn->virtualX;
        width = pScrn->virtualY;
    }

    if (pXGI->ShadowFB) {
        pXGI->ShadowPitch = BitmapBytePad(pScrn->bitsPerPixel * width);
        pXGI->ShadowPtr = xalloc(pXGI->ShadowPitch * height);
        displayWidth = pXGI->ShadowPitch / (pScrn->bitsPerPixel >> 3);
        FBStart = pXGI->ShadowPtr;
    }
    else {
        pXGI->ShadowPtr = NULL;
        FBStart = pXGI->FbBase;
    }

    if (!miSetPixmapDepths()) {
        XGISaveScreen(pScreen, SCREEN_SAVER_OFF);
        XGIErrorLog(pScrn, "miSetPixmapDepths() failed\n");
        return FALSE;
    }

    /* Point cmdQueuePtr to pXGIEnt for shared usage
     * (same technique is then eventually used in DRIScreeninit).
     */
    if (IS_SECOND_HEAD(pXGI))
        pXGI->cmdQueueLenPtr = &(XGIPTR(pXGIEnt->pScrn_1)->cmdQueueLen);
    else
        pXGI->cmdQueueLenPtr = &(pXGI->cmdQueueLen);

    pXGI->cmdQueueLen = 0;      /* Force an EngineIdle() at start */

#ifdef XF86DRI
    if(pXGI->loadDRI) {
        /* No DRI in dual head mode */
        if (IS_DUAL_HEAD(pXGI)) {
            pXGI->directRenderingEnabled = FALSE;
            xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                       "DRI not supported in Dual Head mode\n");
        }
        else if ((pXGI->Chipset == PCI_CHIP_XGIXG20)||(pXGI->Chipset == PCI_CHIP_XGIXG21)||(pXGI->Chipset == PCI_CHIP_XGIXG27)) {
            PDEBUG(ErrorF("--- DRI not supported   \n"));
            xf86DrvMsg(pScrn->scrnIndex, X_NOT_IMPLEMENTED,
                       "DRI not supported on this chipset\n");
            pXGI->directRenderingEnabled = FALSE;
        }
        else {
            pXGI->directRenderingEnabled = XGIDRIScreenInit(pScreen);
            PDEBUG(ErrorF("--- DRI supported   \n"));
        }
    }
#endif

	/* xgiRestoreVirtual(pScrn); */

    /*
     * Call the framebuffer layer's ScreenInit function, and fill in other
     * pScreen fields.
     */
    switch (pScrn->bitsPerPixel) {
    case 24:
    case 8:
    case 16:
    case 32:

/* Jong 07/30/2009; fix bug of small font */
#if 1
		PDEBUG(ErrorF("Use Virtual Size - *1\n"));
		width = /* pScrn->virtualX; */ pScrn->currentMode->HDisplay; 
		height = /* pScrn->virtualY;*/ pScrn->currentMode->VDisplay; 

		/* Jong@10022009 */
		displayWidth = pScrn->displayWidth; /* important to set pitch correctly */
#endif
		PDEBUG(ErrorF("Call fbScreenInit()...\n"));
		PDEBUG(ErrorF("width=%d, height=%d, pScrn->xDpi=%d, pScrn->yDpi=%d, displayWidth=%d, pScrn->bitsPerPixel=%d...\n", 
					width, height, pScrn->xDpi, pScrn->yDpi,displayWidth, pScrn->bitsPerPixel));

		/* in fbscreen.c */
		/* (xsize, ysize) : virtual size (1600, 1200)
		   (dpix, dpiy) : (75, 75)
		   (542) pScreen->mmWidth = (xsize * 254 + dpix * 5) / (dpix * 10);
		   (406) pScreen->mmHeight = (ysize * 254 + dpiy * 5) / (dpiy * 10); */

        /* ret = fbScreenInit(pScreen, FBStart, width, */
        ret = fbScreenInit(pScreen, FBStart , width,
                           height, pScrn->xDpi, pScrn->yDpi,
                           displayWidth, pScrn->bitsPerPixel);

		/* Jong 07/30/2009; bug fixing for small font size */
		pScreen->mmWidth = (pScrn->currentMode->HDisplay * 254 + pScrn->xDpi * 5) / (pScrn->xDpi * 10);
		pScreen->mmHeight = (pScrn->currentMode->VDisplay * 254 + pScrn->yDpi * 5) / (pScrn->yDpi * 10);

	    PDEBUG(ErrorF("pScrn->xDpi = %d\n", pScrn->xDpi));
		PDEBUG(ErrorF("pScrn->yDpi = %d\n", pScrn->yDpi));
	    PDEBUG(ErrorF("pScreen->mmWidth = %d\n", pScreen->mmWidth));
		PDEBUG(ErrorF("pScreen->mmHeight = %d\n", pScreen->mmHeight));

        break;
    default:
        ret = FALSE;
        break;
    }

	xgiRestoreVirtual(pScrn); 

    if (!ret) {
        XGIErrorLog(pScrn, "Unsupported bpp (%d) or fbScreenInit() failed\n",
                    pScrn->bitsPerPixel);
        XGISaveScreen(pScreen, SCREEN_SAVER_OFF);
        return FALSE;
    }

    if (pScrn->bitsPerPixel > 8) {
        /* Fixup RGB ordering */
        visual = pScreen->visuals + pScreen->numVisuals;
        while (--visual >= pScreen->visuals) {
            if ((visual->class | DynamicClass) == DirectColor) {
                visual->offsetRed = pScrn->offset.red;
                visual->offsetGreen = pScrn->offset.green;
                visual->offsetBlue = pScrn->offset.blue;
                visual->redMask = pScrn->mask.red;
                visual->greenMask = pScrn->mask.green;
                visual->blueMask = pScrn->mask.blue;
            }
        }
    }

	/* xgiRestoreVirtual(pScrn); */

    /* Initialize RENDER ext; must be after RGB ordering fixed */
    fbPictureInit(pScreen, 0, 0);

	/* xgiRestoreVirtual(pScrn); */

    /* hardware cursor needs to wrap this layer    <-- TW: what does that mean? */
    if (!pXGI->ShadowFB)
        XGIDGAInit(pScreen);

    xf86SetBlackWhitePixels(pScreen);

    if (!pXGI->NoAccel) {
        /* Volari_EnableAccelerator(pScrn); */
        PDEBUG(ErrorF("---Volari Accel..  \n"));
        Volari_AccelInit(pScreen);
    }

    PDEBUG(ErrorF("--- AccelInit ---  \n"));
    PDEBUG(XGIDumpRegs(pScrn));

    xf86SetBackingStore(pScreen);
    xf86SetSilkenMouse(pScreen);

    /* Initialise cursor functions */
    miDCInitialize(pScreen, xf86GetPointerScreenFuncs());

    if (pXGI->HWCursor) {
        XGIHWCursorInit(pScreen);
    }

    /* Initialise default colourmap */
    if (!miCreateDefColormap(pScreen)) {
        XGISaveScreen(pScreen, SCREEN_SAVER_OFF);
        XGIErrorLog(pScrn, "miCreateDefColormap() failed\n");
        return FALSE;
    }
    if (!xf86HandleColormaps
        (pScreen, 256, (pScrn->depth == 8) ? 8 : pScrn->rgbBits,
         XGILoadPalette, NULL,
         CMAP_PALETTED_TRUECOLOR | CMAP_RELOAD_ON_MODE_SWITCH)) {
        PDEBUG(ErrorF("XGILoadPalette() check-return.  \n"));
        XGISaveScreen(pScreen, SCREEN_SAVER_OFF);
        XGIErrorLog(pScrn, "xf86HandleColormaps() failed\n");
        return FALSE;
    }

/*
    if (!xf86HandleColormaps(pScreen, 256, 8, XGILoadPalette, NULL,
                             CMAP_RELOAD_ON_MODE_SWITCH))
    {
        return FALSE;
    }
*/
    xf86DPMSInit(pScreen, (DPMSSetProcPtr) XGIDisplayPowerManagementSet, 0);

    /* Init memPhysBase and fbOffset in pScrn */
    pScrn->memPhysBase = pXGI->FbAddress;
    pScrn->fbOffset = 0;

    pXGI->ResetXv = pXGI->ResetXvGamma = NULL;

#if defined(XvExtension)
    if (!pXGI->NoXvideo) {
        XGIInitVideo(pScreen);
    }
#endif

#ifdef XF86DRI
    if (pXGI->directRenderingEnabled) {
	/* Now that mi, drm and others have done their thing,
	 * complete the DRI setup.
	 */
	pXGI->directRenderingEnabled = XGIDRIFinishScreenInit(pScreen);
    }

    xf86DrvMsg(pScrn->scrnIndex, X_INFO, "Direct rendering %sabled\n",
	       (pXGI->directRenderingEnabled) ? "en" : "dis");
    if (pXGI->directRenderingEnabled) {
	/* TODO */
	/* XGISetLFBConfig(pXGI); */
    }
#endif

    /* Wrap some funcs and setup remaining SD flags */

    pXGI->XGI_SD_Flags &= ~(XGI_SD_PSEUDOXINERAMA);

    pXGI->CloseScreen = pScreen->CloseScreen;
    pScreen->CloseScreen = XGICloseScreen;
    if (IS_DUAL_HEAD(pXGI))
        pScreen->SaveScreen = XGISaveScreenDH;
    else
        pScreen->SaveScreen = XGISaveScreen;

    /* Install BlockHandler */
    pXGI->BlockHandler = pScreen->BlockHandler;
    pScreen->BlockHandler = XGIBlockHandler;

    /* Report any unused options (only for the first generation) */
    if (serverGeneration == 1) {
        xf86ShowUnusedOptions(pScrn->scrnIndex, pScrn->options);
    }

    /* Clear frame buffer */
    /* For CRT2, we don't do that at this point in dual head
     * mode since the mode isn't switched at this time (it will
     * be reset when setting the CRT1 mode). Hence, we just
     * save the necessary data and clear the screen when
     * going through this for CRT1.
     */

    OnScreenSize = pScrn->displayWidth * pScrn->currentMode->VDisplay
        * (pScrn->bitsPerPixel >> 3);

    /* Turn on the screen now */
    /* We do this in dual head mode after second head is finished */
    if (IS_DUAL_HEAD(pXGI)) {
        if (IS_SECOND_HEAD(pXGI)) {
            bzero(pXGI->FbBase, OnScreenSize);
            bzero(pXGIEnt->FbBase1, pXGIEnt->OnScreenSize1);
            XGISaveScreen(pScreen, SCREEN_SAVER_OFF);
        }
        else {
            pXGIEnt->FbBase1 = pXGI->FbBase;
            pXGIEnt->OnScreenSize1 = OnScreenSize;
        }
    }
    else {
        XGISaveScreen(pScreen, SCREEN_SAVER_OFF);
        bzero(pXGI->FbBase, OnScreenSize);
    }

    pXGI->XGI_SD_Flags &= ~XGI_SD_ISDEPTH8;
    if (pXGI->CurrentLayout.bitsPerPixel == 8) {
        pXGI->XGI_SD_Flags |= XGI_SD_ISDEPTH8;
        pXGI->XGI_SD_Flags &= ~XGI_SD_SUPPORTXVGAMMA1;
    }

    PDEBUG(ErrorF("XGIScreenInit() End.  \n"));
    PDEBUG(XGIDumpPalette(pScrn)); 
	PDEBUG(XGIDumpRegs(pScrn));

	/* xgiRestoreVirtual(); */
    XGIAdjustFrame(scrnIndex, 0, 0, 0); 
	pScrn->frameX0 = 0;
	pScrn->frameY0 = 0; 
	pScrn->frameX1 = pScrn->currentMode->HDisplay - 1 ;
	pScrn->frameY1 = pScrn->currentMode->VDisplay - 1; 

    return TRUE;
}

/* Usually mandatory */
Bool
XGISwitchMode(int scrnIndex, DisplayModePtr mode, int flags)
{
    ScrnInfoPtr pScrn = xf86Screens[scrnIndex];
    XGIPtr pXGI = XGIPTR(pScrn);

	if(pXGI->TargetRefreshRate)
			mode->VRefresh = pXGI->TargetRefreshRate;

    PDEBUG(ErrorF("XGISwitchMode\n"));
    PDEBUG(ErrorF("mode->HDisplay = %d\n", mode->HDisplay));
    PDEBUG(ErrorF("mode->VDisplay = %d\n", mode->VDisplay));

	PDEBUG(ErrorF("Before update...\n"));
    PDEBUG(ErrorF("pScrn->virtualX = %d\n", pScrn->virtualX));
    PDEBUG(ErrorF("pScrn->virtualY = %d\n", pScrn->virtualY));
    PDEBUG(ErrorF("pScrn->displayWidth = %d\n", pScrn->displayWidth));
    PDEBUG(ErrorF("pScrn->frameX0 = %d\n", pScrn->frameX0));
    PDEBUG(ErrorF("pScrn->frameY0 = %d\n", pScrn->frameY0));
    PDEBUG(ErrorF("pScrn->frameX1 = %d\n", pScrn->frameX1));
    PDEBUG(ErrorF("pScrn->frameY1 = %d\n", pScrn->frameY1));

    PDEBUG(ErrorF("pScrn->xDpi = %d\n", pScrn->xDpi));
	PDEBUG(ErrorF("pScrn->yDpi = %d\n", pScrn->yDpi));
	PDEBUG(ErrorF("pScreen->mmWidth = %d\n", pScrn->pScreen->mmWidth));
    PDEBUG(ErrorF("pScreen->mmHeight = %d\n", pScrn->pScreen->mmHeight));

	/* Jong@08122009 */
	//pScrn->frameX0 = 0;
	//pScrn->frameY0 = 0;
	//pScrn->frameX1 = mode->HDisplay;
	//pScrn->frameY1 = mode->VDisplay;

    if (!pXGI->NoAccel) {
#ifdef XGI_USE_XAA
        if (!(pXGI->useEXA) && pXGI->AccelInfoPtr) {
            (*pXGI->AccelInfoPtr->Sync) (pScrn);
            PDEBUG(ErrorF("XGISwitchMode Accel Enabled. \n"));
        }
#endif
    }

    PDEBUG(ErrorF
           ("XGISwitchMode (%d, %d) \n", mode->HDisplay, mode->VDisplay));

#if 1
    /* Jong 07/29/2009; Set the viewport; still not working */
    XGIAdjustFrame(scrnIndex, pScrn->frameX0, pScrn->frameY0, 0);
#endif

    if (!(XGIModeInit(xf86Screens[scrnIndex], mode)))
        return FALSE;


#if 0
    int height, width, displayWidth;
    unsigned char *FBStart;
	int ret;

    if (pXGI->ShadowFB) {
        displayWidth = pXGI->ShadowPitch / (pScrn->bitsPerPixel >> 3);
        FBStart = pXGI->ShadowPtr;
    }
    else {
        pXGI->ShadowPtr = NULL;
        FBStart = pXGI->FbBase;
    }

	width = pScrn->virtualX; /* 1024; */ /* pScrn->currentMode->HDisplay; */
	height = pScrn->virtualY; /* 768; */ /* pScrn->currentMode->VDisplay; */
	displayWidth = pScrn->displayWidth; /* important to set pitch correctly */

	ErrorF("Call fbScreenInit()...\n");
	ErrorF("width=%d, height=%d, pScrn->xDpi=%d, pScrn->yDpi=%d, displayWidth=%d, pScrn->bitsPerPixel=%d...\n", 
				width, height, pScrn->xDpi, pScrn->yDpi,displayWidth, pScrn->bitsPerPixel);

	/* in fbscreen.c */
	/* (xsize, ysize) : virtual size (1600, 1200)
	   (dpix, dpiy) : (75, 75)
	   (542) pScreen->mmWidth = (xsize * 254 + dpix * 5) / (dpix * 10);
	   (406) pScreen->mmHeight = (ysize * 254 + dpiy * 5) / (dpiy * 10); */

    ret = fbScreenInit(pScrn->pScreen, FBStart, width,
                       height, pScrn->xDpi, pScrn->yDpi,
                       displayWidth, pScrn->bitsPerPixel);
#endif

	/* Jong 07/30/2009; bug fixing for small font size */
	pScrn->pScreen->mmWidth = (pScrn->virtualX * 254 + pScrn->xDpi * 5) / (pScrn->xDpi * 10);
	pScrn->pScreen->mmHeight = (pScrn->virtualY * 254 + pScrn->yDpi * 5) / (pScrn->yDpi * 10);

#if 0
    /* Jong 08/30/2007; no virtual screen for all cases */
    /* Jong 08/22/2007; support modeline */
    /* if(g_CountOfUserDefinedModes > 0) */
    {
	
		pScrn->virtualX=mode->HDisplay;
		pScrn->virtualY=mode->VDisplay; 

		pScrn->displayWidth=mode->HDisplay;
		pScrn->frameX0=0;
		pScrn->frameY0=0;
		pScrn->frameX1=mode->HDisplay-1;
		pScrn->frameY1=mode->VDisplay-1;
    }
#endif

	PDEBUG(ErrorF("After update...\n"));
    PDEBUG(ErrorF("pScrn->virtualX = %d\n", pScrn->virtualX));
    PDEBUG(ErrorF("pScrn->virtualY = %d\n", pScrn->virtualY));
    PDEBUG(ErrorF("pScrn->displayWidth = %d\n", pScrn->displayWidth));
    PDEBUG(ErrorF("pScrn->frameX0 = %d\n", pScrn->frameX0));
    PDEBUG(ErrorF("pScrn->frameY0 = %d\n", pScrn->frameY0));
    PDEBUG(ErrorF("pScrn->frameX1 = %d\n", pScrn->frameX1));
    PDEBUG(ErrorF("pScrn->frameY1 = %d\n", pScrn->frameY1));

    PDEBUG(ErrorF("pScrn->xDpi = %d\n", pScrn->xDpi));
	PDEBUG(ErrorF("pScrn->yDpi = %d\n", pScrn->yDpi));
	PDEBUG(ErrorF("pScreen->mmWidth = %d\n", pScrn->pScreen->mmWidth));
    PDEBUG(ErrorF("pScreen->mmHeight = %d\n", pScrn->pScreen->mmHeight));

    /* Since RandR (indirectly) uses SwitchMode(), we need to
     * update our Xinerama info here, too, in case of resizing
     */

	/* sleep(3); */ /* Jong 07/30/2009; wait to be ready for drawing */;

    return TRUE;
}

/* static void
XGISetStartAddressCRT1(XGIPtr pXGI, unsigned long base)
{
    unsigned char cr11backup;

    inXGIIDXREG(XGICR,  0x11, cr11backup);  
    andXGIIDXREG(XGICR, 0x11, 0x7F);
    outXGIIDXREG(XGICR, 0x0D, base & 0xFF);
    outXGIIDXREG(XGICR, 0x0C, (base >> 8) & 0xFF);
    outXGIIDXREG(XGISR, 0x0D, (base >> 16) & 0xFF);

    
    setXGIIDXREG(XGICR, 0x11, 0x7F,(cr11backup & 0x80));
} */

#ifdef XGIMERGED
/* static Bool
InRegion(int x, int y, region r)
{
    return (r.x0 <= x) && (x <= r.x1) && (r.y0 <= y) && (y <= r.y1);
} */

/* static void
XGIAdjustFrameHW_CRT1(ScrnInfoPtr pScrn, int x, int y)
{
    XGIPtr pXGI = XGIPTR(pScrn);
    unsigned long base;

    base = y * pXGI->CurrentLayout.displayWidth + x;
    switch(pXGI->CurrentLayout.bitsPerPixel) 
    {
       case 16:  base >>= 1; 	break;
       case 32:  		break;
       default:  base >>= 2;
    }
    XGISetStartAddressCRT1(pXGI, base);
} */

/* static void
XGIMergePointerMoved(int scrnIndex, int x, int y)
{
  ScrnInfoPtr   pScrn1 = xf86Screens[scrnIndex];
  XGIPtr        pXGI = XGIPTR(pScrn1);
  ScrnInfoPtr   pScrn2 = pXGI->CRT2pScrn;
  region 	out, in1, in2, f2, f1;
  int 		deltax, deltay;

  f1.x0 = pXGI->CRT1frameX0;
  f1.x1 = pXGI->CRT1frameX1;
  f1.y0 = pXGI->CRT1frameY0;
  f1.y1 = pXGI->CRT1frameY1;
  f2.x0 = pScrn2->frameX0;
  f2.x1 = pScrn2->frameX1;
  f2.y0 = pScrn2->frameY0;
  f2.y1 = pScrn2->frameY1;

  out.x0 = pScrn1->frameX0;
  out.x1 = pScrn1->frameX1;
  out.y0 = pScrn1->frameY0;
  out.y1 = pScrn1->frameY1;

  in1 = out;
  in2 = out;
  switch(((XGIMergedDisplayModePtr)pXGI->CurrentLayout.mode->Private)->CRT2Position) 
  {
     case xgiLeftOf:
        in1.x0 = f1.x0;
        in2.x1 = f2.x1;
        break;
     case xgiRightOf:
        in1.x1 = f1.x1;
        in2.x0 = f2.x0;
        break;
     case xgiBelow:
        in1.y1 = f1.y1;
        in2.y0 = f2.y0;
        break;
     case xgiAbove:
        in1.y0 = f1.y0;
        in2.y1 = f2.y1;
        break;
     case xgiClone:
        break;
  }

  deltay = 0;
  deltax = 0;

  if(InRegion(x, y, out)) 
  {	

     if(InRegion(x, y, in1) && !InRegion(x, y, f1)) 
     {
        REBOUND(f1.x0, f1.x1, x);
        REBOUND(f1.y0, f1.y1, y);
        deltax = 1;
     }
     if(InRegion(x, y, in2) && !InRegion(x, y, f2)) 
     {
        REBOUND(f2.x0, f2.x1, x);
        REBOUND(f2.y0, f2.y1, y);
        deltax = 1;
     }

  }
  else 
  {			

     if(out.x0 > x) 
     {
        deltax = x - out.x0;
     }
     if(out.x1 < x) 
     {
        deltax = x - out.x1;
     }
     if(deltax) 
     {
        pScrn1->frameX0 += deltax;
        pScrn1->frameX1 += deltax;
    f1.x0 += deltax;
        f1.x1 += deltax;
        f2.x0 += deltax;
        f2.x1 += deltax;
     }

     if(out.y0 > y) 
     {
        deltay = y - out.y0;
     }
     if(out.y1 < y) 
     {
        deltay = y - out.y1;
     }
     if(deltay) 
     {
        pScrn1->frameY0 += deltay;
        pScrn1->frameY1 += deltay;
    f1.y0 += deltay;
        f1.y1 += deltay;
        f2.y0 += deltay;
        f2.y1 += deltay;
     }

     switch(((XGIMergedDisplayModePtr)pXGI->CurrentLayout.mode->Private)->CRT2Position) 
     {
        case xgiLeftOf:
       if(x >= f1.x0) 
       { REBOUND(f1.y0, f1.y1, y); }
       if(x <= f2.x1) 
       { REBOUND(f2.y0, f2.y1, y); }
           break;
        case xgiRightOf:
       if(x <= f1.x1) 
       { REBOUND(f1.y0, f1.y1, y); }
       if(x >= f2.x0) 
       { REBOUND(f2.y0, f2.y1, y); }
           break;
        case xgiBelow:
       if(y <= f1.y1) 
       { REBOUND(f1.x0, f1.x1, x); }
       if(y >= f2.y0) 
       { REBOUND(f2.x0, f2.x1, x); }
           break;
        case xgiAbove:
       if(y >= f1.y0) 
       { REBOUND(f1.x0, f1.x1, x); }
       if(y <= f2.y1) 
       { REBOUND(f2.x0, f2.x1, x); }
           break;
        case xgiClone:
           break;
     }

  }

  if(deltax || deltay) 
  {
     pXGI->CRT1frameX0 = f1.x0;
     pXGI->CRT1frameY0 = f1.y0;
     pScrn2->frameX0 = f2.x0;
     pScrn2->frameY0 = f2.y0;

     pXGI->CRT1frameX1 = pXGI->CRT1frameX0 + CDMPTR->CRT1->HDisplay - 1;
     pXGI->CRT1frameY1 = pXGI->CRT1frameY0 + CDMPTR->CRT1->VDisplay - 1;
     pScrn2->frameX1   = pScrn2->frameX0   + CDMPTR->CRT2->HDisplay - 1;
     pScrn2->frameY1   = pScrn2->frameY0   + CDMPTR->CRT2->VDisplay - 1;
     pScrn1->frameX1   = pScrn1->frameX0   + pXGI->CurrentLayout.mode->HDisplay  - 1;
     pScrn1->frameY1   = pScrn1->frameY0   + pXGI->CurrentLayout.mode->VDisplay  - 1;

     XGIAdjustFrameHW_CRT1(pScrn1, pXGI->CRT1frameX0, pXGI->CRT1frameY0);
  }
}  */


/* static void
XGIAdjustFrameMerged(int scrnIndex, int x, int y, int flags)
{
    ScrnInfoPtr pScrn1 = xf86Screens[scrnIndex];
    XGIPtr pXGI = XGIPTR(pScrn1);
    ScrnInfoPtr pScrn2 = pXGI->CRT2pScrn;
    int VTotal = pXGI->CurrentLayout.mode->VDisplay;
    int HTotal = pXGI->CurrentLayout.mode->HDisplay;
    int VMax = VTotal;
    int HMax = HTotal;

    BOUND(x, 0, pScrn1->virtualX - HTotal);
    BOUND(y, 0, pScrn1->virtualY - VTotal);

    switch(SDMPTR(pScrn1)->CRT2Position) 
    {
        case xgiLeftOf:
            pScrn2->frameX0 = x;
            BOUND(pScrn2->frameY0,   y, y + VMax - CDMPTR->CRT2->VDisplay);
            pXGI->CRT1frameX0 = x + CDMPTR->CRT2->HDisplay;
            BOUND(pXGI->CRT1frameY0, y, y + VMax - CDMPTR->CRT1->VDisplay);
            break;
        case xgiRightOf:
            pXGI->CRT1frameX0 = x;
            BOUND(pXGI->CRT1frameY0, y, y + VMax - CDMPTR->CRT1->VDisplay);
            pScrn2->frameX0 = x + CDMPTR->CRT1->HDisplay;
            BOUND(pScrn2->frameY0,   y, y + VMax - CDMPTR->CRT2->VDisplay);
            break;
        case xgiAbove:
            BOUND(pScrn2->frameX0,   x, x + HMax - CDMPTR->CRT2->HDisplay);
            pScrn2->frameY0 = y;
            BOUND(pXGI->CRT1frameX0, x, x + HMax - CDMPTR->CRT1->HDisplay);
            pXGI->CRT1frameY0 = y + CDMPTR->CRT2->VDisplay;
            break;
        case xgiBelow:
            BOUND(pXGI->CRT1frameX0, x, x + HMax - CDMPTR->CRT1->HDisplay);
            pXGI->CRT1frameY0 = y;
            BOUND(pScrn2->frameX0,   x, x + HMax - CDMPTR->CRT2->HDisplay);
            pScrn2->frameY0 = y + CDMPTR->CRT1->VDisplay;
            break;
        case xgiClone:
            BOUND(pXGI->CRT1frameX0, x, x + HMax - CDMPTR->CRT1->HDisplay);
            BOUND(pXGI->CRT1frameY0, y, y + VMax - CDMPTR->CRT1->VDisplay);
            BOUND(pScrn2->frameX0,   x, x + HMax - CDMPTR->CRT2->HDisplay);
            BOUND(pScrn2->frameY0,   y, y + VMax - CDMPTR->CRT2->VDisplay);
            break;
    }

    BOUND(pXGI->CRT1frameX0, 0, pScrn1->virtualX - CDMPTR->CRT1->HDisplay);
    BOUND(pXGI->CRT1frameY0, 0, pScrn1->virtualY - CDMPTR->CRT1->VDisplay);
    BOUND(pScrn2->frameX0,   0, pScrn1->virtualX - CDMPTR->CRT2->HDisplay);
    BOUND(pScrn2->frameY0,   0, pScrn1->virtualY - CDMPTR->CRT2->VDisplay);

    pScrn1->frameX0 = x;
    pScrn1->frameY0 = y;

    pXGI->CRT1frameX1 = pXGI->CRT1frameX0 + CDMPTR->CRT1->HDisplay - 1;
    pXGI->CRT1frameY1 = pXGI->CRT1frameY0 + CDMPTR->CRT1->VDisplay - 1;
    pScrn2->frameX1   = pScrn2->frameX0   + CDMPTR->CRT2->HDisplay - 1;
    pScrn2->frameY1   = pScrn2->frameY0   + CDMPTR->CRT2->VDisplay - 1;
    pScrn1->frameX1   = pScrn1->frameX0   + pXGI->CurrentLayout.mode->HDisplay  - 1;
    pScrn1->frameY1   = pScrn1->frameY0   + pXGI->CurrentLayout.mode->VDisplay  - 1;

    XGIAdjustFrameHW_CRT1(pScrn1, pXGI->CRT1frameX0, pXGI->CRT1frameY0);
} */
#endif

/*
 * This function is used to initialize the Start Address - the first
 * displayed location in the video memory.
 *
 * Usually mandatory
 */
void
XGIAdjustFrame(int scrnIndex, int x, int y, int flags)
{
    ScrnInfoPtr pScrn = xf86Screens[scrnIndex];
    XGIPtr pXGI = XGIPTR(pScrn);
    unsigned long base;
    unsigned char ucSR5Stat, ucTemp;

    ErrorF("AdjustFrame %d\n", scrnIndex);
    inXGIIDXREG(XGISR, 0x05, ucSR5Stat);
    if (ucSR5Stat == 0xA1)
        ucSR5Stat = 0x86;
    outXGIIDXREG(XGISR, 0x05, 0x86);

    base = (pScrn->bitsPerPixel + 7) / 8;
    base *= x;
    base += pXGI->scrnOffset * y;
    base >>= 2;

    switch (pXGI->Chipset) {
    case PCI_CHIP_XGIXG40:
    case PCI_CHIP_XGIXG20:
    case PCI_CHIP_XGIXG21:
    case PCI_CHIP_XGIXG27:
    default:

        ucTemp = base & 0xFF;
        outXGIIDXREG(XGICR, 0x0D, ucTemp);
        ucTemp = (base >> 8) & 0xFF;
        outXGIIDXREG(XGICR, 0x0C, ucTemp);
        ucTemp = (base >> 16) & 0xFF;
        outXGIIDXREG(XGISR, 0x0D, ucTemp);
        ucTemp = (base >> 24) & 0x01;
        setXGIIDXREG(XGISR, 0x37, 0xFE, ucTemp);

/*        if (pXGI->VBFlags)  {
            XGI_UnLockCRT2(&(pXGI->xgi_HwDevExt),pXGI->pVBInfo);
            ucTemp = base & 0xFF       ; outXGIIDXREG( XGIPART1, 6 , ucTemp ) ;
            ucTemp = (base>>8) & 0xFF  ; outXGIIDXREG( XGIPART1, 5 , ucTemp ) ;
            ucTemp = (base>>16) & 0xFF ; outXGIIDXREG( XGIPART1, 4 , ucTemp ) ;
            ucTemp = (base>>24) & 0x01 ; ucTemp <<= 7 ;
            setXGIIDXREG( XGIPART1, 0x2, 0x7F, ucTemp ) ;

            XGI_LockCRT2(&(pXGI->xgi_HwDevExt),pXGI->pVBInfo);
        }
        */
        break;

    }

    outXGIIDXREG(XGISR, 0x05, ucSR5Stat);

}

/*
 * This is called when VT switching back to the X server.  Its job is
 * to reinitialise the video mode.
 * Mandatory!
 */
static Bool
XGIEnterVT(int scrnIndex, int flags)
{
    ScrnInfoPtr pScrn = xf86Screens[scrnIndex];
    XGIPtr pXGI = XGIPTR(pScrn);

    xgiSaveUnlockExtRegisterLock(pXGI, NULL, NULL);

    if (!XGIModeInit(pScrn, pScrn->currentMode)) {
        XGIErrorLog(pScrn, "XGIEnterVT: XGIModeInit() failed\n");
        return FALSE;
    }

    XGIAdjustFrame(scrnIndex, pScrn->frameX0, pScrn->frameY0, 0);

#ifdef XF86DRI
    if (pXGI->directRenderingEnabled) {
        DRIUnlock(screenInfo.screens[scrnIndex]);
    }
#endif

    if ((!IS_DUAL_HEAD(pXGI) || !IS_SECOND_HEAD(pXGI)) && (pXGI->ResetXv)) {
        (pXGI->ResetXv) (pScrn);
    }

    return TRUE;
}

/*
 * This is called when VT switching away from the X server.  Its job is
 * to restore the previous (text) mode.
 * Mandatory!
 */
static void
XGILeaveVT(int scrnIndex, int flags)
{
    ScrnInfoPtr pScrn = xf86Screens[scrnIndex];
    vgaHWPtr hwp = VGAHWPTR(pScrn);
    XGIPtr pXGI = XGIPTR(pScrn);
#ifdef XF86DRI
    ScreenPtr pScreen;

    PDEBUG(ErrorF("XGILeaveVT()\n"));
    if (pXGI->directRenderingEnabled) {
        pScreen = screenInfo.screens[scrnIndex];
        DRILock(pScreen, 0);
    }
#endif

    if (IS_DUAL_HEAD(pXGI) && IS_SECOND_HEAD(pXGI))
        return;

    if (pXGI->CursorInfoPtr) {
        /* Because of the test and return above, we know that this is not
         * the second head.
         */
        pXGI->CursorInfoPtr->HideCursor(pScrn);
        XGI_WaitBeginRetrace(pXGI->RelIO);
    }

    XGIRestore(pScrn);


    /* We use (otherwise unused) bit 7 to indicate that we are running to keep
     * xgifb to change the displaymode (this would result in lethal display
     * corruption upon quitting X or changing to a VT until a reboot).
     */
    vgaHWLock(hwp);
}


/*
 * This is called at the end of each server generation.  It restores the
 * original (text) mode.  It should really also unmap the video memory too.
 * Mandatory!
 */
static Bool
XGICloseScreen(int scrnIndex, ScreenPtr pScreen)
{
    ScrnInfoPtr pScrn = xf86Screens[scrnIndex];
    vgaHWPtr hwp = VGAHWPTR(pScrn);
    XGIPtr pXGI = XGIPTR(pScrn);


#ifdef XF86DRI
    if (pXGI->directRenderingEnabled) {
        XGIDRICloseScreen(pScreen);
        pXGI->directRenderingEnabled = FALSE;
    }
#endif

    if (pScrn->vtSema) {
        if (pXGI->CursorInfoPtr
            && (!IS_DUAL_HEAD(pXGI) || !IS_SECOND_HEAD(pXGI))) {
            pXGI->CursorInfoPtr->HideCursor(pScrn);
            XGI_WaitBeginRetrace(pXGI->RelIO);
        }


        XGIRestore(pScrn);
        vgaHWLock(hwp);
    }

    /* We should restore the mode number in case vtsema = false as well,
     * but since we haven't register access then we can't do it. I think
     * I need to rework the save/restore stuff, like saving the video
     * status when returning to the X server and by that save me the
     * trouble if xgifb was started from a textmode VT while X was on.
     */

    XGIUnmapMem(pScrn);
    vgaHWUnmapMem(pScrn);

    if (IS_DUAL_HEAD(pXGI)) {
        XGIEntPtr pXGIEnt = ENTITY_PRIVATE(pXGI);
        pXGIEnt->refCount--;
    }

    if (pXGI->pInt) {
        xf86FreeInt10(pXGI->pInt);
        pXGI->pInt = NULL;
    }

#ifdef XGI_USE_XAA
    if (pXGI->AccelLinearScratch) {
        xf86FreeOffscreenLinear(pXGI->AccelLinearScratch);
        pXGI->AccelLinearScratch = NULL;
    }

    if (!(pXGI->useEXA) && pXGI->AccelInfoPtr) {
        XAADestroyInfoRec(pXGI->AccelInfoPtr);
        pXGI->AccelInfoPtr = NULL;
    }
#endif

    if (pXGI->CursorInfoPtr) {
        xf86DestroyCursorInfoRec(pXGI->CursorInfoPtr);
        pXGI->CursorInfoPtr = NULL;
    }

    if (pXGI->ShadowPtr) {
        xfree(pXGI->ShadowPtr);
        pXGI->ShadowPtr = NULL;
    }

    if (pXGI->DGAModes) {
        xfree(pXGI->DGAModes);
        pXGI->DGAModes = NULL;
    }

    if (pXGI->adaptor) {
        xfree(pXGI->adaptor);
        pXGI->adaptor = NULL;
        pXGI->ResetXv = pXGI->ResetXvGamma = NULL;
    }

    pScrn->vtSema = FALSE;

    /* Restore Blockhandler */
    pScreen->BlockHandler = pXGI->BlockHandler;

    pScreen->CloseScreen = pXGI->CloseScreen;

    return (*pScreen->CloseScreen) (scrnIndex, pScreen);
}


/* Free up any per-generation data structures */

/* Optional */
static void
XGIFreeScreen(int scrnIndex, int flags)
{
    if (xf86LoaderCheckSymbol("vgaHWFreeHWRec")) {
        vgaHWFreeHWRec(xf86Screens[scrnIndex]);
    }

    XGIFreeRec(xf86Screens[scrnIndex]);
}


/* Jong 07/02/2008; Validate user-defined mode */
int XGIValidateUserDefMode(XGIPtr pXGI, DisplayModePtr mode)
{
   UShort i = (pXGI->CurrentLayout.bitsPerPixel+7)/8 - 1;


#if 1
	if((mode->HDisplay >= 1600) && (mode->VDisplay >= 1200) && (mode->VRefresh > 60))
	{
		ErrorF("Not support over (1600,1200) 60Hz ... Reduce to (1600,1200) 60Hz\n");
		mode->type=48; /* not user-defined */
		mode->VRefresh = 60.0;

		mode->Clock=mode->SynthClock=162000; /* from XG20_Mode[] */ /* ((float)(mode->VTotal*mode->HTotal)+0.5) * (mode->VRefresh) / 1000.0; */
		ErrorF("Update clock to %d...\n", mode->Clock);
		return(-111) ;
	}
#endif

#if 0
	if(XGI_GetModeID(0, mode->HDisplay, mode->VDisplay, i, 0, 0) == 0)
	{
		/* Jong 11/10/2008; support custom mode without ModeID */
		if( !((pXGI->HaveCustomModes) && (!(mode->type & M_T_DEFAULT))) )
		{
			ErrorF("Can't get Mode ID...\n");
			return(MODE_NOMODE) ;
	    }
	}
#endif

	return(MODE_OK);
}

/* Checks if a mode is suitable for the selected chipset. */

static int
XGIValidMode(int scrnIndex, DisplayModePtr mode, Bool verbose, int flags)
{
    ScrnInfoPtr pScrn = xf86Screens[scrnIndex];
    XGIPtr pXGI = XGIPTR(pScrn);
    int HDisplay = mode->HDisplay;
    int VDisplay = mode->VDisplay;
    int Clock = mode->Clock;
    int i = 0;
    int VRefresh;

	/* Jong 07/27/2009; support custom mode without ModeID */
	pXGI->HaveCustomModes = TRUE;

    VRefresh =
        (int) ((float) (Clock * 1000) /
               (float) (mode->VTotal * mode->HTotal) + 0.5);

	/* Jong@09252009 */
	if(mode->VRefresh == 0)
		mode->VRefresh = VRefresh;

    if((mode->type == M_T_USERDEF) || ((mode->type & M_T_CLOCK_CRTC_C) == M_T_CLOCK_CRTC_C))
	{
		VRefresh = mode->VRefresh;
	    Clock = mode->Clock;
	}

    PDEBUG5(ErrorF("\nXGIValidMode()-->"));
    PDEBUG5(ErrorF
            ("CLK=%5.3fMhz %dx%d@%d ", (float) Clock / 1000, HDisplay,
             VDisplay, VRefresh));
    PDEBUG5(ErrorF("(VT,HT)=(%d,%d)\n", mode->VTotal, mode->HTotal));
    PDEBUG5(ErrorF("flags = %d\n", flags));
	if(flags == MODECHECK_FINAL)
	    PDEBUG5(ErrorF("This is a final check...\n"));

#if 1
    if((mode->type == M_T_USERDEF) || ((mode->type & M_T_CLOCK_CRTC_C) == M_T_CLOCK_CRTC_C))
	{
		if(pScrn->monitor->DDC)
		{
			if(XGICheckModeByDDC(mode, pScrn->monitor->DDC) == FALSE)
			{
				ErrorF("It's a user-defined mode...rejected by EDID (pScrn->monitor->DDC)...return MODE_NOMODE\n");
				return (MODE_NOMODE);
			}
		}

		PDEBUG5(ErrorF("It's a user-defined mode...return MODE_OK (might need more checking here) \n"));
		return(MODE_OK); 
	} 
#else
	if((mode->type == M_T_USERDEF) || ((mode->type & M_T_CLOCK_CRTC_C) == M_T_CLOCK_CRTC_C))
	{
		iRet=XGIValidateUserDefMode(pXGI, mode);
		if(iRet != -111)
		{
			if(iRet == MODE_OK)
				ErrorF("User-defined mode---MODE_OK\n");
			else
				ErrorF("User-defined mode---MODE_NOMODE\n");
			
			return(iRet);
		}
	}
#endif

	if(mode->VRefresh == 0)
		mode->VRefresh = VRefresh;

#if 0
    if (pXGI->VBFlags & CRT2_LCD) {
        if ((HDisplay > 1600 && VDisplay > 1200)
            || (HDisplay < 640 && VDisplay < 480)) {
            PDEBUG5(ErrorF("skip by LCD limit\n"));
            return (MODE_NOMODE);
        }
        /* if( VRefresh != 60) return(MODE_NOMODE) ; */
    }
    else if (pXGI->VBFlags & CRT2_TV) {
        if ((HDisplay > 1024 && VDisplay > 768) ||
            (HDisplay < 640 && VDisplay < 480) || (VRefresh != 60)) {
            PDEBUG5(ErrorF("skip by TV limit\n"));
            return (MODE_NOMODE);
        }
    }
    else if (pXGI->VBFlags & CRT2_VGA) {
        if ((HDisplay > 1600 && VDisplay > 1200) ||
            (HDisplay < 640 && VDisplay < 480)) {
            PDEBUG5(ErrorF("skip by CRT2 limit\n"));
            return (MODE_NOMODE);
        }
    }
#endif

    if ((pXGI->Chipset == PCI_CHIP_XGIXG20) ||(pXGI->Chipset == PCI_CHIP_XGIXG21) ||( pXGI->Chipset == PCI_CHIP_XGIXG27 )) {
        XgiMode = XG20_Mode;
    }
    else {
        XgiMode = XGI_Mode;
    }

    while ((XgiMode[i].Clock != Clock) ||
           (XgiMode[i].HDisplay != HDisplay) ||
           (XgiMode[i].VDisplay != VDisplay)) {
        if (XgiMode[i].Clock == 0) {
            PDEBUG5(ErrorF
                    ("--- Mode %dx%d@%dHz is not defined in support mode table of driver\n", HDisplay,
                     VDisplay, VRefresh));
			PDEBUG5(ErrorF("Mode is invalid...return MODE_NOMODE\n"));
            return (MODE_NOMODE);
        }
        else
            i++;
    }

	if(pScrn->monitor->DDC)
	{
		if(XGICheckModeByDDC(mode, pScrn->monitor->DDC) == FALSE)
			{
				ErrorF("Rejected by EDID (pScrn->monitor->DDC)...return MODE_NOMODE\n");
				return (MODE_NOMODE);
			}
	}

	if (pXGI->Chipset == PCI_CHIP_XGIXG27)
	{
		if(((g_PowerSavingStatus & 0x03) < 0x03) && 
		   ((g_PowerSavingStatus & 0x04) == 0x00) &&
			g_pMonitorDVI) 
		{
			if(XGICheckModeByDDC(mode, g_pMonitorDVI) == FALSE)
			{
				PDEBUG5(ErrorF("Rejected by CRT2 EDID...return MODE_NOMODE\n"));
				return (MODE_NOMODE);
			}
		}
	}
	else /* Jong 12/05/2007; filter mode of CRT1 with CRT2 DDC for XG21 */
	{
		if(g_pMonitorDVI)
		{
			if(XGICheckModeByDDC(mode, g_pMonitorDVI) == FALSE)
			{
				PDEBUG5(ErrorF("Rejected by DVI EDID...return MODE_NOMODE\n"));
				return (MODE_NOMODE);
			}
		}
	}

    PDEBUG5(ErrorF("Mode is valid...return MODE_OK\n"));
    return (MODE_OK);
}

/* Do screen blanking
 *
 * Mandatory
 */
static Bool
XGISaveScreen(ScreenPtr pScreen, int mode)
{
    ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];

    if ((pScrn != NULL) && pScrn->vtSema) {

        XGIPtr pXGI = XGIPTR(pScrn);

#ifdef UNLOCK_ALWAYS
        xgiSaveUnlockExtRegisterLock(pXGI, NULL, NULL);
#endif
    }

    return vgaHWSaveScreen(pScreen, mode);
}

/* SaveScreen for dual head mode */
static Bool
XGISaveScreenDH(ScreenPtr pScreen, int mode)
{
#ifdef XGIDUALHEAD
    ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];

    if ((pScrn != NULL) && pScrn->vtSema) {
        XGIPtr pXGI = XGIPTR(pScrn);

        if (IS_SECOND_HEAD(pXGI)
            && ((!(pXGI->VBFlags & CRT1_LCDA))
                || (pXGI->XGI_Pr->VBType & VB_XGI301C))) {

            /* Slave head is always CRT1 */
            if (pXGI->VBFlags & CRT1_LCDA)
                pXGI->Blank = xf86IsUnblank(mode) ? FALSE : TRUE;

            return vgaHWSaveScreen(pScreen, mode);
        }
        else {
            /* Master head is always CRT2 */
            /* But we land here if CRT1 is LCDA, too */

            /* We can only blank LCD, not other CRT2 devices */
            if (!(pXGI->VBFlags & (CRT2_LCD | CRT1_LCDA)))
                return TRUE;

            /* enable access to extended sequencer registers */
#ifdef UNLOCK_ALWAYS
            xgiSaveUnlockExtRegisterLock(pXGI, NULL, NULL);
#endif
        }
    }
#endif
    return TRUE;
}

#ifdef DEBUG
static void
XGIDumpModeInfo(ScrnInfoPtr pScrn, DisplayModePtr mode)
{
    xf86DrvMsg(pScrn->scrnIndex, X_INFO, "Clock : %x\n", mode->Clock);
    xf86DrvMsg(pScrn->scrnIndex, X_INFO, "Hz Display : %x\n",
               mode->CrtcHDisplay);
    xf86DrvMsg(pScrn->scrnIndex, X_INFO, "Hz Blank Start : %x\n",
               mode->CrtcHBlankStart);
    xf86DrvMsg(pScrn->scrnIndex, X_INFO, "Hz Sync Start : %x\n",
               mode->CrtcHSyncStart);
    xf86DrvMsg(pScrn->scrnIndex, X_INFO, "Hz Sync End : %x\n",
               mode->CrtcHSyncEnd);
    xf86DrvMsg(pScrn->scrnIndex, X_INFO, "Hz Blank End : %x\n",
               mode->CrtcHBlankEnd);
    xf86DrvMsg(pScrn->scrnIndex, X_INFO, "Hz Total : %x\n", mode->CrtcHTotal);
    xf86DrvMsg(pScrn->scrnIndex, X_INFO, "Hz Skew : %x\n", mode->CrtcHSkew);
    xf86DrvMsg(pScrn->scrnIndex, X_INFO, "Hz HAdjusted : %x\n",
               mode->CrtcHAdjusted);
    xf86DrvMsg(pScrn->scrnIndex, X_INFO, "Vt Display : %x\n",
               mode->CrtcVDisplay);
    xf86DrvMsg(pScrn->scrnIndex, X_INFO, "Vt Blank Start : %x\n",
               mode->CrtcVBlankStart);
    xf86DrvMsg(pScrn->scrnIndex, X_INFO, "Vt Sync Start : %x\n",
               mode->CrtcVSyncStart);
    xf86DrvMsg(pScrn->scrnIndex, X_INFO, "Vt Sync End : %x\n",
               mode->CrtcVSyncEnd);
    xf86DrvMsg(pScrn->scrnIndex, X_INFO, "Vt Blank End : %x\n",
               mode->CrtcVBlankEnd);
    xf86DrvMsg(pScrn->scrnIndex, X_INFO, "Vt Total : %x\n", mode->CrtcVTotal);
    xf86DrvMsg(pScrn->scrnIndex, X_INFO, "Vt VAdjusted : %x\n",
               mode->CrtcVAdjusted);
}
#endif

static void
XGIModifyModeInfo(DisplayModePtr mode)
{
    if (mode->CrtcHBlankStart == mode->CrtcHDisplay)
        mode->CrtcHBlankStart++;
    if (mode->CrtcHBlankEnd == mode->CrtcHTotal)
        mode->CrtcHBlankEnd--;
    if (mode->CrtcVBlankStart == mode->CrtcVDisplay)
        mode->CrtcVBlankStart++;
    if (mode->CrtcVBlankEnd == mode->CrtcVTotal)
        mode->CrtcVBlankEnd--;
}

/* Things to do before a ModeSwitch. We set up the
 * video bridge configuration and the TurboQueue.
 */
void
XGIPreSetMode(ScrnInfoPtr pScrn, DisplayModePtr mode, int viewmode)
{
    XGIPtr pXGI = XGIPTR(pScrn);
    unsigned char CR30, CR31, CR33;
    unsigned char CR3B = 0;
    unsigned char CR17, CR38 = 0;
    unsigned char CR35 = 0, CR79 = 0;
    unsigned long vbflag;
    int temp = 0;
    int crt1rateindex = 0;
    DisplayModePtr mymode;
#ifdef XGIMERGED
    DisplayModePtr mymode2 = NULL;
#endif

#ifdef XGIMERGED
    if (pXGI->MergedFB) {
        mymode = ((XGIMergedDisplayModePtr) mode->Private)->CRT1;
        mymode2 = ((XGIMergedDisplayModePtr) mode->Private)->CRT2;
    }
    else
#endif
        mymode = mode;

    vbflag = pXGI->VBFlags;
    PDEBUG(ErrorF("VBFlags=0x%lx\n", pXGI->VBFlags));

#ifdef UNLOCK_ALWAYS
    xgiSaveUnlockExtRegisterLock(pXGI, NULL, NULL);     /* Unlock Registers */
#endif

    inXGIIDXREG(XGICR, 0x30, CR30);
    inXGIIDXREG(XGICR, 0x31, CR31);
    inXGIIDXREG(XGICR, 0x33, CR33);

    inXGIIDXREG(XGICR, 0x3b, CR3B);
    xf86DrvMsgVerb(pScrn->scrnIndex, X_PROBED, 4,
                   "Before: CR30=0x%02x, CR31=0x%02x, CR33=0x%02x, CR%02x=0x%02x\n",
                   CR30, CR31, CR33, temp, CR38);

    xf86DrvMsgVerb(pScrn->scrnIndex, X_INFO, 4, "VBFlags=0x%lx\n",
                   pXGI->VBFlags);

    CR30 = 0x00;
    CR31 &= ~0x60;              /* Clear VB_Drivermode & VB_OutputDisable */
    CR31 |= 0x04;               /* Set VB_NotSimuMode (not for 30xB/1400x1050?) */
    CR35 = 0x00;


    if (!pXGI->AllowHotkey) {
        CR31 |= 0x80;           /* Disable hotkey-switch */
    }
    CR79 &= ~0x10;              /* Enable Backlight control on 315 series */


    if ((vbflag & CRT1_LCDA) && (viewmode == XGI_MODE_CRT1)) {

        CR38 |= 0x02;

    }
    else {

        switch (vbflag & (CRT2_TV | CRT2_LCD | CRT2_VGA)) {

        case CRT2_TV:

            CR38 &= ~0xC0;      /* Clear Pal M/N bits */

            if (vbflag & TV_YPBPR) {    /* Video bridge */
                if (pXGI->XGI_SD_Flags & XGI_SD_SUPPORTYPBPR) {
                    CR30 |= 0x80;
                    CR38 |= 0x08;
                    if (vbflag & TV_YPBPR525P)
                        CR38 |= 0x10;
                    else if (vbflag & TV_YPBPR750P)
                        CR38 |= 0x20;
                    else if (vbflag & TV_YPBPR1080I)
                        CR38 |= 0x30;
                    CR31 &= ~0x01;
                    if (pXGI->XGI_SD_Flags & XGI_SD_SUPPORTYPBPRAR) {
                        CR3B &= ~0x03;
                        if ((vbflag & TV_YPBPRAR) == TV_YPBPR43LB)
                            CR3B |= 0x00;
                        else if ((vbflag & TV_YPBPRAR) == TV_YPBPR43)
                            CR3B |= 0x03;
                        else if ((vbflag & TV_YPBPRAR) == TV_YPBPR169)
                            CR3B |= 0x01;
                        else
                            CR3B |= 0x03;
                    }
                }
            }
            else {              /* All */
                if (vbflag & TV_SCART)
                    CR30 |= 0x10;
                if (vbflag & TV_SVIDEO)
                    CR30 |= 0x08;
                if (vbflag & TV_AVIDEO)
                    CR30 |= 0x04;
                if (!(CR30 & 0x1C))
                    CR30 |= 0x08;       /* default: SVIDEO */

                if (vbflag & TV_PAL) {
                    CR31 |= 0x01;
                    CR35 |= 0x01;
                    if (pXGI->XGI_Pr->VBType & VB_XGIVB) {
                        if (vbflag & TV_PALM) {
                            CR38 |= 0x40;
                            CR35 |= 0x04;
                        }
                        else if (vbflag & TV_PALN) {
                            CR38 |= 0x80;
                            CR35 |= 0x08;
                        }
                    }
                }
                else {
                    CR31 &= ~0x01;
                    CR35 &= ~0x01;
                    if (vbflag & TV_NTSCJ) {
                        CR38 |= 0x40;   /* TW, not BIOS */
                        CR35 |= 0x02;
                    }
                }
                if (vbflag & TV_SCART) {
                    CR31 |= 0x01;
                    CR35 |= 0x01;
                }
            }

            CR31 &= ~0x04;      /* Clear NotSimuMode */
#ifdef XGI_CP
            XGI_CP_DRIVER_CONFIG
#endif
                break;

        case CRT2_LCD:
            CR30 |= 0x20;
            break;

        case CRT2_VGA:
            CR30 |= 0x40;
            break;

        default:
            CR30 |= 0x00;
            CR31 |= 0x20;       /* VB_OUTPUT_DISABLE */
        }

    }

    if (vbflag & CRT1_LCDA) {
        switch (viewmode) {
        case XGI_MODE_CRT1:
            CR38 |= 0x01;
            break;
        case XGI_MODE_CRT2:
            if (vbflag & (CRT2_TV | CRT2_VGA)) {
                CR30 |= 0x02;
                CR38 |= 0x01;
            }
            else {
                CR38 |= 0x03;
            }
            break;
        case XGI_MODE_SIMU:
        default:
            if (vbflag & (CRT2_TV | CRT2_LCD | CRT2_VGA)) {
                CR30 |= 0x01;
            }
            break;
        }
    }
    else {
        if (vbflag & (CRT2_TV | CRT2_LCD | CRT2_VGA)) {
            CR30 |= 0x01;
        }
    }

    CR31 |= 0x40;               /* Set Drivermode */
    CR31 &= ~0x06;              /* Disable SlaveMode, disable SimuMode in SlaveMode */
    crt1rateindex = XGISearchCRT1Rate(pScrn, mymode);

    if (IS_DUAL_HEAD(pXGI)) {
        if (IS_SECOND_HEAD(pXGI)) {
            /* CRT1 */
            CR33 &= 0xf0;
            if (!(vbflag & CRT1_LCDA)) {
                CR33 |= (crt1rateindex & 0x0f);
            }
        }
        else {
            /* CRT2 */
            CR33 &= 0x0f;
            if (vbflag & CRT2_VGA) {
                CR33 |= ((crt1rateindex << 4) & 0xf0);
            }
        }
    }
    else
#ifdef XGIMERGED
    if (pXGI->MergedFB) {
        CR33 = 0;
        if (!(vbflag & CRT1_LCDA)) {
            CR33 |= (crt1rateindex & 0x0f);
        }
        if (vbflag & CRT2_VGA) {
            CR33 |= (XGISearchCRT1Rate(pScrn, mymode2) << 4);
        }
    }
    else
#endif
    {
        CR33 = 0;
        if (!(vbflag & CRT1_LCDA)) {
            CR33 |= (crt1rateindex & 0x0f);
        }
        if (vbflag & CRT2_VGA) {
            CR33 |= ((crt1rateindex & 0x0f) << 4);
        }
        if (vbflag & CRT2_ENABLE) {
            if (pXGI->CRT1off)
                CR33 &= 0xf0;
        }
    }
    outXGIIDXREG(XGICR, 0x30, CR30);
    outXGIIDXREG(XGICR, 0x31, CR31);
    outXGIIDXREG(XGICR, 0x33, CR33);
    if (temp) {
        outXGIIDXREG(XGICR, temp, CR38);
    }
    xf86DrvMsgVerb(pScrn->scrnIndex, X_INFO, 4,
                   "After:  CR30=0x%02x,CR31=0x%02x,CR33=0x%02x,CR%02x=%02x\n",
                   CR30, CR31, CR33, temp, CR38);

    if (pXGI->VBFlags & CRT2_ENABLE) {
        /* Switch on CRT1 for modes that require the bridge in SlaveMode */
        andXGIIDXREG(XGISR, 0x1f, 0x3f);
        inXGIIDXREG(XGICR, 0x17, CR17);
        if (!(CR17 & 0x80)) {
            orXGIIDXREG(XGICR, 0x17, 0x80);
            outXGIIDXREG(XGISR, 0x00, 0x01);
            usleep(10000);
            outXGIIDXREG(XGISR, 0x00, 0x03);
        }
    }

    andXGIIDXREG(XGISR, 0x1f, 0xfb); /* disable DAC pedestal to reduce brightness */
}

/* PostSetMode:
 * -) Disable CRT1 for saving bandwidth. This doesn't work with VESA;
 *    VESA uses the bridge in SlaveMode and switching CRT1 off while
 *    the bridge is in SlaveMode not that clever...
 * -) Check if overlay can be used (depending on dotclock)
 * -) Check if Panel Scaler is active on LVDS for overlay re-scaling
 * -) Save TV registers for further processing
 * -) Apply TV settings
 */
static void
XGIPostSetMode(ScrnInfoPtr pScrn, XGIRegPtr xgiReg)
{
    XGIPtr pXGI = XGIPTR(pScrn);
/*    unsigned char usScratchCR17;
    Bool flag = FALSE;
    Bool doit = TRUE; */
    int myclock;
    unsigned char sr2b, sr2c, tmpreg;
    float num, denum, postscalar, divider;
    PDEBUG(ErrorF(" XGIPostSetMode(). \n"));
#ifdef TWDEBUG
    xf86DrvMsg(pScrn->scrnIndex, X_INFO, "CRT1off is %d\n", pXGI->CRT1off);
#endif

#ifdef UNLOCK_ALWAYS
    xgiSaveUnlockExtRegisterLock(pXGI, NULL, NULL);
#endif

    /* Determine if the video overlay can be used */
    if (!pXGI->NoXvideo) {
        inXGIIDXREG(XGISR, 0x2b, sr2b);
        inXGIIDXREG(XGISR, 0x2c, sr2c);
        divider = (sr2b & 0x80) ? 2.0 : 1.0;
        postscalar = (sr2c & 0x80) ?
            ((((sr2c >> 5) & 0x03) == 0x02) ? 6.0 : 8.0) :
            (((sr2c >> 5) & 0x03) + 1.0);
        num = (sr2b & 0x7f) + 1.0;
        denum = (sr2c & 0x1f) + 1.0;
        myclock =
            (int) ((14318 * (divider / postscalar) * (num / denum)) / 1000);

        pXGI->MiscFlags &= ~(MISC_CRT1OVERLAY | MISC_CRT1OVERLAYGAMMA);
/*       switch(pXGI->xgi_HwDevExt.jChipType) {
            break;
       }
       */
        if (!(pXGI->MiscFlags & MISC_CRT1OVERLAY)) {
            if (!IS_DUAL_HEAD(pXGI) || IS_SECOND_HEAD(pXGI))
                xf86DrvMsgVerb(pScrn->scrnIndex, X_WARNING, 3,
                               "Current dotclock (%dMhz) too high for video overlay on CRT1\n",
                               myclock);
        }
    }

    /* Determine if the Panel Link scaler is active */
    pXGI->MiscFlags &= ~MISC_PANELLINKSCALER;
    if (pXGI->VBFlags & (CRT2_LCD | CRT1_LCDA)) {
        if (pXGI->VBFlags & CRT1_LCDA) {
            inXGIIDXREG(XGIPART1, 0x35, tmpreg);
            tmpreg &= 0x04;
            if (!tmpreg)
                pXGI->MiscFlags |= MISC_PANELLINKSCALER;
        }
    }

    /* Determine if our very special TV mode is active */
    pXGI->MiscFlags &= ~MISC_TVNTSC1024;
    if ((pXGI->XGI_Pr->VBType & VB_XGIVB) && (pXGI->VBFlags & CRT2_TV)
        && (!(pXGI->VBFlags & TV_HIVISION))) {
        if (((pXGI->VBFlags & TV_YPBPR) && (pXGI->VBFlags & TV_YPBPR525I))
            || ((!(pXGI->VBFlags & TV_YPBPR))
                && (pXGI->VBFlags & (TV_NTSC | TV_PALM)))) {
            inXGIIDXREG(XGICR, 0x34, tmpreg);
            tmpreg &= 0x7f;
            if ((tmpreg == 0x64) || (tmpreg == 0x4a) || (tmpreg == 0x38)) {
                pXGI->MiscFlags |= MISC_TVNTSC1024;
            }
        }
    }

    /* Reset XV gamma correction */
    if (pXGI->ResetXvGamma) {
        (pXGI->ResetXvGamma) (pScrn);
    }

    /*  Apply TV settings given by options
     * Do this even in DualHeadMode:
     * - if this is called by SetModeCRT1, CRT2 mode has been reset by SetModeCRT1
     * - if this is called by SetModeCRT2, CRT2 mode has changed (duh!)
     * -> Hence, in both cases, the settings must be re-applied.
     */
}


USHORT
XGI_CalcModeIndex(ScrnInfoPtr pScrn, DisplayModePtr mode,
                  unsigned long VBFlags)
{
    XGIPtr pXGI = XGIPTR(pScrn);
    UShort i = (pXGI->CurrentLayout.bitsPerPixel + 7) / 8 - 1;

    if ((VBFlags & CRT1_LCDA)) {
        if ((mode->HDisplay > pXGI->LCDwidth) ||
            (mode->VDisplay > pXGI->LCDheight)) {
            return 0;
        }
    }

    return XGI_GetModeID(VBFlags, mode->HDisplay, mode->VDisplay,
                         i, pXGI->LCDwidth, pXGI->LCDheight);
}

/* Calculate the vertical refresh rate from a mode */
int
XGICalcVRate(DisplayModePtr mode)
{
    float hsync, refresh = 0;

    if (mode->HSync > 0.0)
        hsync = mode->HSync;
    else if (mode->HTotal > 0)
        hsync = (float) mode->Clock / (float) mode->HTotal;
    else
        hsync = 0.0;

    if (mode->VTotal > 0)
        refresh = hsync * 1000.0 / mode->VTotal;

    if (mode->Flags & V_INTERLACE)
        refresh *= 2.0;

    if (mode->Flags & V_DBLSCAN)
        refresh /= 2.0;

    if (mode->VScan > 1)
        refresh /= mode->VScan;

    if (mode->VRefresh > 0.0)
        refresh = mode->VRefresh;

    if (hsync == 0 || refresh == 0)
        return (0);

    return ((int) (refresh));
}

/* Calculate CR33 (rate index) for CRT1.
 * Calculation is done using currentmode, therefore it is
 * recommended to set VertRefresh and HorizSync to correct
 * values in config file.
 */
unsigned char
XGISearchCRT1Rate(ScrnInfoPtr pScrn, DisplayModePtr mode)
{
    XGIPtr         pXGI = XGIPTR(pScrn);  
    int i = 0;
    int irefresh;
    unsigned short xres = mode->HDisplay;
    unsigned short yres = mode->VDisplay;
    unsigned char index;
    BOOLEAN checkxgi730 = FALSE;

    irefresh = XGICalcVRate(mode);
    if (!irefresh) {
        if (xres == 800 || xres == 1024 || xres == 1280)
            return 0x02;
        else
            return 0x01;
    }

#ifdef TWDEBUG
    xf86DrvMsg(0, X_INFO, "Debug: CalcVRate returned %d\n", irefresh);
#endif

    /* We need the REAL refresh rate here */
    if (mode->Flags & V_INTERLACE)
        irefresh /= 2;

    /* Do not multiply by 2 when DBLSCAN! */

#ifdef TWDEBUG
    xf86DrvMsg(0, X_INFO, "Debug: Rate after correction = %d\n", irefresh);
#endif

    index = 0;
    while ((xgix_vrate[i].idx != 0) && (xgix_vrate[i].xres <= xres)) {
        if ((xgix_vrate[i].xres == xres) && (xgix_vrate[i].yres == yres)) {
            if ((checkxgi730 == FALSE)
                || (xgix_vrate[i].XGI730valid32bpp == TRUE)) {
                if (xgix_vrate[i].refresh == irefresh) {
                    index = xgix_vrate[i].idx;
                    break;
                }
                else if (xgix_vrate[i].refresh > irefresh) {
                    if ((xgix_vrate[i].refresh - irefresh) <= 3) {
                        index = xgix_vrate[i].idx;
                    }
                    else if (((checkxgi730 == FALSE)
                              || (xgix_vrate[i - 1].XGI730valid32bpp == TRUE))
                             && ((irefresh - xgix_vrate[i - 1].refresh) <= 2)
                             && (xgix_vrate[i].idx != 1)) {
                        index = xgix_vrate[i - 1].idx;
                    }
                    break;
                }
                else if ((irefresh - xgix_vrate[i].refresh) <= 2) {
                    index = xgix_vrate[i].idx;
                    break;
                }
            }
        }
        i++;
    }

	/* Jong 10/19/2007; merge code */
	/* Adjust to match table of VBIOS */
	switch(pXGI->Chipset)
	{
		case PCI_CHIP_XGIXG20: 
		case PCI_CHIP_XGIXG21: 
                if((xres == 640) && (yres == 480))
                {
                  if (xgix_vrate[index].refresh>85)
                  {
                    index = 4;
                  }
                }    

                if((xres == 800) && (yres == 600))
                {
                  if (xgix_vrate[index].refresh>85)
                  {
                    index = 5;
                  }

                  if (index>0)
                  {
                    index --;
                  }       
                }

                if((xres == 1024) && (yres == 768))
                {
                  if (xgix_vrate[index].refresh>85)
                  {
                    index = 5;
                  }

                  if (index>0)
                  {
                    index --;
                  }
                }

                if((xres == 1280) && (yres == 1024))
                {
                  if (index>0)
                  {
                    index --;
                  }
                }

                if((xres == 1600) && (yres == 1200))
                {
                  if (xgix_vrate[index].refresh>85)
                  {
                    index = 5;
                  }
                }

                if((xres >= 1920) && (yres >= 1440))
                {
                  index = 0;
                }

                break;

		case PCI_CHIP_XGIXG27:

               if((xres == 640) && (yres == 480))
                {
                  if (xgix_vrate[index].refresh>85)
                  {
                    index = 4;
                  }
                }    

                if((xres == 800) && (yres == 600))
                {
                  if (xgix_vrate[index].refresh>85)
                  {
                    index = 5;
                  }

                  if (index>0)
                  {
                    index --;
                  }             
                }

                if((xres == 1024) && (yres == 768))
                {
                  if (xgix_vrate[index].refresh>85)
                  {
                    index = 5;
                  }

                  if (index>0)
                  {
                    index --;
                  }
                }

                if((xres == 1280) && (yres == 1024))
                {
                  if (index>0)
                  {
                    index --;
                  }
                }

                if((xres == 1600) && (yres == 1200))
                {
                  if (xgix_vrate[index].refresh>85)
                  {
                    index = 5;
                  }
                }

                break;

		default:
            break;
	}

    if (index > 0)
        return index;
    else {
        /* Default Rate index */
        if (xres == 800 || xres == 1024 || xres == 1280)
            return 0x02;
        else
            return 0x01;
    }
}


#define MODEID_OFF 0x449

unsigned char
XGI_GetSetModeID(ScrnInfoPtr pScrn, unsigned char id)
{
    return (XGI_GetSetBIOSScratch(pScrn, MODEID_OFF, id));
}

unsigned char
XGI_GetSetBIOSScratch(ScrnInfoPtr pScrn, USHORT offset, unsigned char value)
{
    unsigned char ret = 0;
#if (defined(i386) || defined(__i386) || defined(__i386__) || defined(__AMD64__))
    unsigned char *base;

    base = xf86MapVidMem(pScrn->scrnIndex, VIDMEM_MMIO, 0, 0x2000);
    if (!base) {
        XGIErrorLog(pScrn, "(Could not map BIOS scratch area)\n");
        return 0;
    }

    ret = *(base + offset);

    /* value != 0xff means: set register */
    if (value != 0xff)
        *(base + offset) = value;

    xf86UnMapVidMem(pScrn->scrnIndex, base, 0x2000);
#endif
    return ret;
}

void
xgiSaveUnlockExtRegisterLock(XGIPtr pXGI, unsigned char *reg1,
                             unsigned char *reg2)
{
    register unsigned char val;
    unsigned long mylockcalls;

    pXGI->lockcalls++;
    mylockcalls = pXGI->lockcalls;

    /* check if already unlocked */
    inXGIIDXREG(XGISR, 0x05, val);
    if (val != 0xa1) {
        /* save State */
        if (reg1)
            *reg1 = val;
        /* unlock */
/*
       outb (0x3c4, 0x20);
       val4 = inb (0x3c5);
       val4 |= 0x20;
       outb (0x3c5, val4);
*/
        outXGIIDXREG(XGISR, 0x05, 0x86);
        inXGIIDXREG(XGISR, 0x05, val);
        if (val != 0xA1) {
#ifdef TWDEBUG
            unsigned char val1, val2;
            int i;
#endif
            XGIErrorLog(pXGI->pScrn,
                        "Failed to unlock sr registers (%p, %lx, 0x%02x; %ld)\n",
                        (void *) pXGI, (unsigned long) pXGI->RelIO, val,
                        mylockcalls);
#ifdef TWDEBUG
            for (i = 0; i <= 0x3f; i++) {
                inXGIIDXREG(XGISR, i, val1);
                /* inXGIIDXREG(0x3c4, i, val2); */
                inXGIIDXREG(XGISR, i, val2);
                xf86DrvMsg(pXGI->pScrn->scrnIndex, X_INFO,
                           "SR%02d: RelIO=0x%02x 0x3c4=0x%02x (%d)\n",
                           i, val1, val2, mylockcalls);
            }
#endif
        }
    }
}

void
xgiRestoreExtRegisterLock(XGIPtr pXGI, unsigned char reg1, unsigned char reg2)
{
    /* restore lock */
#ifndef UNLOCK_ALWAYS
    outXGIIDXREG(XGISR, 0x05, reg1 == 0xA1 ? 0x86 : 0x00);
#endif
}

/* Jong 12/03/2007; */
/*
void XGICheckModeForMonitor(ScrnInfoPtr pScrn, )
{
	DisplayModePtr pCRT1Modes=pScrn->monitor->Modes;

    if ((p = first = pScrn->monitor->Modes)) {
        do {
			xf86CheckModeForMonitor(p, 
            n = p->next;
            p = n;
        } while (p != NULL && p != first);
    }

    xf86PruneDriverModes(pXGI->CRT2pScrn);
}
*/

/* Jong 12/05/2007; filter mode list by monitor DDC */
static void XGIFilterModeByDDC(DisplayModePtr pModeList, xf86MonPtr pMonitorDDC)
{
    DisplayModePtr first, p;

    if ((p = first = pModeList)) 
	{
        do 
		{
			if(XGICheckModeByDDC(p, pMonitorDDC) == FALSE)
				xf86DeleteMode(&pModeList, pModeList);

            p = p->next;
        } while (p != NULL && p != first);
    }
}

/* Jong 12/05/2007; filter mode list by monitor DDC */
static bool XGICheckModeByDDC(DisplayModePtr pMode, xf86MonPtr pMonitorDDC)
{
    int i, j;
    float VF, HF;
    struct detailed_timings *pd_timings;
    struct monitor_ranges *pranges;
    struct std_timings *pstd_t;

	int VRefresh=pMode->VRefresh;

    if ((pMode == NULL) || (pMonitorDDC == NULL)) {
        return(FALSE);                 /* ignore */
    }

	if( pMode->VRefresh == 0)
		VRefresh = (int)((float)(pMode->Clock*1000)/(float)(pMode->VTotal*pMode->HTotal)+0.5);


    for (i = 0, j = 0; i < 8; i++, j++) 
	{
        if (establish_timing[j].width == -1) 
		{
            continue;
        }

        if (pMonitorDDC->timings1.t1 & (1 << i)) 
		{
			if( (establish_timing[j].width == pMode->HDisplay) && 
				(establish_timing[j].height == pMode->VDisplay) && 
				(establish_timing[j].VRefresh == VRefresh) )
				return(TRUE);
        }
    }

    for (i = 0; i < 8; i++, j++) 
	{
        if (establish_timing[j].width == -1) 
		{
            continue;
        }

        if (pMonitorDDC->timings1.t2 & (1 << i)) 
		{
			if( (establish_timing[j].width == pMode->HDisplay) && 
				(establish_timing[j].height == pMode->VDisplay) && 
				(establish_timing[j].VRefresh == VRefresh) )
				return(TRUE);
        }
    }

    for (i = 0; i < 8; i++) 
	{
        if ((pMode->HDisplay == pMonitorDDC->timings2[i].hsize) &&
            (pMode->VDisplay == pMonitorDDC->timings2[i].vsize) &&
            (VRefresh == pMonitorDDC->timings2[i].refresh)) 
			return(TRUE);
    }

/* Jong 12/05/2007; Don't know how to do? */
#if 0
    for (i = 0; i < 4; i++) 
	{
        switch (pMonitorDDC->det_mon[i].type) 
		{
			case DS_RANGES:
				pranges = &(pMonitorDDC->det_mon[i].section.ranges);
				PDEBUG5(ErrorF
						("min_v = %d max_v = %d min_h = %d max_h = %d max_clock = %d\n",
						 pranges->min_v, pranges->max_v, pranges->min_h,
						 pranges->max_h, pranges->max_clock));

				if (range->loH > pranges->min_h)
					range->loH = pranges->min_h;
				if (range->loV > pranges->min_v)
					range->loV = pranges->min_v;
				if (range->hiH < pranges->max_h)
					range->hiH = pranges->max_h;
				if (range->hiV < pranges->max_v)
					range->hiV = pranges->max_v;
				PDEBUG5(ErrorF
						("range(%8.3f %8.3f %8.3f %8.3f)\n", range->loH,
						 range->loV, range->hiH, range->hiV));
				break;

			case DS_STD_TIMINGS:
				pstd_t = pMonitorDDC->det_mon[i].section.std_t;
				for (j = 0; j < 5; j++) {
					int k;
					PDEBUG5(ErrorF
							("std_t[%d] hsize = %d vsize = %d refresh = %d id = %d\n",
							 j, pstd_t[j].hsize, pstd_t[j].vsize,
							 pstd_t[j].refresh, pstd_t[j].id));
					for (k = 0; StdTiming[k].width != -1; k++) {
						if ((StdTiming[k].width == pstd_t[j].hsize) &&
							(StdTiming[k].height == pstd_t[j].vsize) &&
							(StdTiming[k].VRefresh == pstd_t[j].refresh)) {
							if (range->loH > StdTiming[k].HSync)
								range->loH = StdTiming[k].HSync;
							if (range->hiH < StdTiming[k].HSync)
								range->hiH = StdTiming[k].HSync;
							if (range->loV > StdTiming[k].VRefresh)
								range->loV = StdTiming[k].VRefresh;
							if (range->hiV < StdTiming[k].VRefresh)
								range->hiV = StdTiming[k].VRefresh;
							break;
						}

					}
				}
				break;

			case DT:

				pd_timings = &pMonitorDDC->det_mon[i].section.d_timings;

				HF = pd_timings->clock / (pd_timings->h_active +
										  pd_timings->h_blanking);
				VF = HF / (pd_timings->v_active + pd_timings->v_blanking);
				HF /= 1000;         /* into KHz Domain */
				if (range->loH > HF)
					range->loH = HF;
				if (range->hiH < HF)
					range->hiH = HF;
				if (range->loV > VF)
					range->loV = VF;
				if (range->hiV < VF)
					range->hiV = VF;
				PDEBUG(ErrorF
					   ("Detailing Timing: HF = %f VF = %f range (%8.3f %8.3f %8.3f %8.3f)\n",
						HF, VF, range->loH, range->loV, range->hiH, range->hiV));
				break;
		}
    }
#endif

	return(FALSE);
}

#ifdef DEBUG
void
XGIDumpSR(ScrnInfoPtr pScrn)
{
    XGIPtr pXGI = XGIPTR(pScrn);

    int i, j;
    unsigned long temp;

    ErrorF
        ("----------------------------------------------------------------------\n");
    ErrorF("SR xx\n");
    ErrorF
        ("----------------------------------------------------------------------\n");
    for (i = 0; i < 0x40; i += 0x10) {
        ErrorF("SR[%02X]:", i);
        for (j = 0; j < 16; j++) {
            inXGIIDXREG(XGISR, (i + j), temp);
            ErrorF(" %02lX", temp);
        }
        ErrorF("\n");
    }
    ErrorF("\n");
}

void
XGIDumpCR(ScrnInfoPtr pScrn)
{
    XGIPtr pXGI = XGIPTR(pScrn);

    int i, j;
    unsigned long temp;

    ErrorF
        ("----------------------------------------------------------------------\n");
    ErrorF("CR xx\n");
    ErrorF
        ("----------------------------------------------------------------------\n");
    for (i = 0; i < 0x100; i += 0x10) {
        ErrorF("CR[%02X]:", i);
        for (j = 0; j < 16; j++) {
            inXGIIDXREG(XGICR, (i + j), temp);
            ErrorF(" %02lX", temp);
        }
        ErrorF("\n");
    }
}

void
XGIDumpGR(ScrnInfoPtr pScrn)
{
    XGIPtr pXGI = XGIPTR(pScrn);

    int i;
    unsigned long temp;

    ErrorF
        ("----------------------------------------------------------------------\n");
    ErrorF("GR xx\n");
    ErrorF
        ("----------------------------------------------------------------------\n");
    ErrorF("GR:");
    for (i = 0; i < 0x9; i += 0x10) {
        inXGIIDXREG(XGISR, i, temp);
        ErrorF(" %02lX", temp);
    }
    ErrorF("\n");
}

#if 0
void
XGIDumpPart0(ScrnInfoPtr pScrn)
{
    XGIPtr pXGI = XGIPTR(pScrn);
    int i, j;
    unsigned long temp;

    ErrorF
        ("----------------------------------------------------------------------\n");
    ErrorF("PART0 xx\n");
    ErrorF
        ("----------------------------------------------------------------------\n");
    for (i = 0; i < 0x50; i += 0x10) {
        ErrorF("PART0[%02X]:", i);
        for (j = 0; j < 0x10; j++) {
            inXGIIDXREG(XGIPART0, (i + j), temp);
            ErrorF(" %02lX", temp);
        }
        ErrorF("\n");
    }
}

void
XGIDumpPart05(ScrnInfoPtr pScrn)
{
    XGIPtr pXGI = XGIPTR(pScrn);
    int i, j;
    unsigned long temp;
    ErrorF
        ("----------------------------------------------------------------------\n");
    ErrorF("PART05 xx\n");
    ErrorF
        ("----------------------------------------------------------------------\n");
    for (i = 0; i < 0x50; i += 0x10) {
        ErrorF("PART05[%02X]:", i);
        for (j = 0; j < 0x10; j++) {
            inXGIIDXREG(XGIPART05, (i + j), temp);
            ErrorF(" %02lX", temp);
        }
        ErrorF("\n");
    }
}

void
XGIDumpPart1(ScrnInfoPtr pScrn)
{
    XGIPtr pXGI = XGIPTR(pScrn);

    int i, j;
    unsigned long temp;

    ErrorF
        ("----------------------------------------------------------------------\n");
    ErrorF("PART1 xx\n");
    ErrorF
        ("----------------------------------------------------------------------\n");
    for (i = 0; i < 0x100; i += 0x10) {
        ErrorF("PART1[%02X]:", i);
        for (j = 0; j < 0x10; j++) {
            inXGIIDXREG(XGIPART1, (i + j), temp);
            ErrorF(" %02lX", temp);
        }
        ErrorF("\n");
    }
}

void
XGIDumpPart2(ScrnInfoPtr pScrn)
{
    XGIPtr pXGI = XGIPTR(pScrn);

    int i, j;
    unsigned long temp;

    ErrorF
        ("----------------------------------------------------------------------\n");
    ErrorF("PART2 xx\n");
    ErrorF
        ("----------------------------------------------------------------------\n");
    for (i = 0; i < 0x100; i += 0x10) {
        ErrorF("PART2[%02X]:", i);
        for (j = 0; j < 0x10; j++) {
            inXGIIDXREG(XGIPART2, (i + j), temp);
            ErrorF(" %02lX", temp);
        }
        ErrorF("\n");
    }
}

void
XGIDumpPart3(ScrnInfoPtr pScrn)
{
    XGIPtr pXGI = XGIPTR(pScrn);

    int i, j;
    unsigned long temp;

    ErrorF
        ("----------------------------------------------------------------------\n");
    ErrorF("PART3 xx\n");
    ErrorF
        ("----------------------------------------------------------------------\n");

    for (i = 0; i < 0x100; i += 0x10) {
        ErrorF("PART3[%02X]:", i);
        for (j = 0; j < 0x10; j++) {
            inXGIIDXREG(XGIPART3, (i + j), temp);
            ErrorF(" %02lX", temp);
        }
        ErrorF("\n");
    }
}

void
XGIDumpPart4(ScrnInfoPtr pScrn)
{
    XGIPtr pXGI = XGIPTR(pScrn);

    int i, j;
    unsigned long temp;

    ErrorF
        ("----------------------------------------------------------------------\n");
    ErrorF("PART4 xx\n");
    ErrorF
        ("----------------------------------------------------------------------\n");
    for (i = 0; i < 0x100; i += 0x10) {
        ErrorF("PART4[%02X]:", i);
        for (j = 0; j < 0x10; j++) {
            inXGIIDXREG(XGIPART4, (i + j), temp);
            ErrorF(" %02lX", temp);
        }
        ErrorF("\n");
    }
}
#endif

void
XGIDumpMMIO(ScrnInfoPtr pScrn)
{
    XGIPtr pXGI = XGIPTR(pScrn);

    int i;
    unsigned long temp;
/*
    ErrorF("----------------------------------------------------------------------\n") ;
    ErrorF("MMIO 85xx\n") ;
    ErrorF("----------------------------------------------------------------------\n") ;
	for( i = 0x8500 ; i < 0x8600 ; i+=0x10 )
	{
		ErrorF("[%04X]: %08lX %08lX %08lX %08lX\n",i,
			XGIMMIOLONG(i),
			XGIMMIOLONG(i+4),
			XGIMMIOLONG(i+8),
			XGIMMIOLONG(i+12)) ;
	}
*/
}
#endif /* DEBUG */

void
XGIDumpRegs(ScrnInfoPtr pScrn)
{
#ifdef DEBUG

    XGIPtr pXGI = XGIPTR(pScrn);

    XGIDumpSR(pScrn);
    XGIDumpCR(pScrn);
//      XGIDumpGR(pScrn);
//      XGIDumpPalette(pScrn);
    XGIDumpMMIO(pScrn);

	/*
    if (pXGI->Chipset != PCI_CHIP_XGIXG20) {
        XGIDumpPart0(pScrn);
        XGIDumpPart05(pScrn);
        XGIDumpPart1(pScrn);
        XGIDumpPart2(pScrn);
        XGIDumpPart3(pScrn);
        XGIDumpPart4(pScrn);
    } */

#endif /* DEBUG */
}


void
XGIDumpPalette(ScrnInfoPtr pScrn)
{
#ifdef DEBUG
    XGIPtr pXGI = XGIPTR(pScrn);
    unsigned temp[3];
    int i, j;

    ErrorF
        ("----------------------------------------------------------------------\n");
    ErrorF("Palette \n");
    ErrorF
        ("----------------------------------------------------------------------\n");
    for (i = 0; i < 0xFF; i += 0x04) {
        for (j = 0; j < 16; j++) {
            /* outb(0x3c7, i + j); */
            outb(XGISR+3, i + j);
			
			/*
            temp[0] = inb(0x3c9);
            temp[1] = inb(0x3c9);
            temp[2] = inb(0x3c9); */
            temp[0] = inb(XGISR+5);
            temp[1] = inb(XGISR+5);
            temp[2] = inb(XGISR+5);

            ErrorF("PA[%02X]: %02X %02X %02X", i + j,
                   temp[0], temp[1], temp[2]);
        }
        ErrorF("\n");
    }
    ErrorF("\n");
#endif
}


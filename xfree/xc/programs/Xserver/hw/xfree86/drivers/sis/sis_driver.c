/*
 * Copyright 1998,1999 by Alan Hourihane, Wigan, England.
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Alan Hourihane not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  Alan Hourihane makes no representations
 * about the suitability of this software for any purpose.  It is provided
 * "as is" without express or implied warranty.
 *
 * ALAN HOURIHANE DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL ALAN HOURIHANE BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 *
 * Authors:  Alan Hourihane, alanh@fairlite.demon.co.uk
 *           Mike Chapman <mike@paranoia.com>, 
 *           Juanjo Santamarta <santamarta@ctv.es>, 
 *           Mitani Hiroshi <hmitani@drl.mei.co.jp> 
 *           David Thomas <davtom@dream.org.uk>. 
 *
 *  Fixes for 630 chipsets: Thomas Winischhofer.
 */
/* $XFree86: xc/programs/Xserver/hw/xfree86/drivers/sis/sis_driver.c,v 1.78 2002/01/17 10:49:35 eich Exp $ */

#include "fb.h"
#include "xf1bpp.h"
#include "xf4bpp.h"
#include "mibank.h"
#include "micmap.h"
#include "xf86.h"
#include "xf86_OSproc.h"
#include "xf86Resources.h"
#include "xf86_ansic.h"
#include "xf86Version.h"
#include "xf86PciInfo.h"
#include "xf86Pci.h"
#include "xf86cmap.h"
#include "vgaHW.h"
#include "xf86RAC.h"
#include "shadowfb.h"
#include "vbe.h"

#include "sis_shadow.h"

#include "mipointer.h"
#include "mibstore.h"

#include "sis.h"
#include "sis_regs.h"
#include "sis_bios.h"
#include "sis_vb.h"
#include "sis_dac.h"

#define _XF86DGA_SERVER_
#include "extensions/xf86dgastr.h"

#include "globals.h"
#define DPMS_SERVER
#include "extensions/dpms.h"

#ifdef XvExtension
#include "xf86xv.h"
#include "Xv.h"
#endif

#ifdef XF86DRI
#include "dri.h"
#endif


/* mandatory functions */
static void SISIdentify(int flags);
static Bool SISProbe(DriverPtr drv, int flags);
static Bool SISPreInit(ScrnInfoPtr pScrn, int flags);
static Bool SISScreenInit(int Index, ScreenPtr pScreen, int argc,
                  char **argv);
static Bool SISEnterVT(int scrnIndex, int flags);
static void SISLeaveVT(int scrnIndex, int flags);
static Bool SISCloseScreen(int scrnIndex, ScreenPtr pScreen);
static Bool SISSaveScreen(ScreenPtr pScreen, int mode);
static Bool SISSwitchMode(int scrnIndex, DisplayModePtr mode, int flags);
static void SISAdjustFrame(int scrnIndex, int x, int y, int flags);

/* Optional functions */
static void SISFreeScreen(int scrnIndex, int flags);
static int  SISValidMode(int scrnIndex, DisplayModePtr mode, Bool verbose,
                 int flags);

/* Internally used functions */
static Bool SISMapMem(ScrnInfoPtr pScrn);
static Bool SISUnmapMem(ScrnInfoPtr pScrn);
static void SISSave(ScrnInfoPtr pScrn);
static void SISRestore(ScrnInfoPtr pScrn);
static void SISVESARestore(ScrnInfoPtr pScrn);
static Bool SISModeInit(ScrnInfoPtr pScrn, DisplayModePtr mode);
static void SISModifyModeInfo(DisplayModePtr mode);
static void SiSPreSetMode(ScrnInfoPtr pScrn, int LockAfterwards);
static void SiSPostSetMode(ScrnInfoPtr pScrn, SISRegPtr sisReg, int LockAfterwards);
static Bool SiSSetVESAMode(ScrnInfoPtr pScrn, DisplayModePtr pMode);
static void SiSBuildVesaModeList(ScrnInfoPtr pScrn, vbeInfoPtr pVbe, VbeInfoBlock *vbe);
static UShort CalcVESAModeIndex(ScrnInfoPtr pScrn, DisplayModePtr mode);
static void SISBridgeRestore(ScrnInfoPtr pScrn);
unsigned char SISSearchCRT1Rate(DisplayModePtr mode);
static void SISVESASaveRestore(ScrnInfoPtr pScrn, vbeSaveRestoreFunction function);

void SiSOptions(ScrnInfoPtr pScrn);
const OptionInfoRec * SISAvailableOptions(int chipid, int busid);
void SiSSetup(ScrnInfoPtr pScrn);
void SISVGAPreInit(ScrnInfoPtr pScrn);
Bool SiSAccelInit(ScreenPtr pScreen);
Bool SiS300AccelInit(ScreenPtr pScreen);
Bool SiS530AccelInit(ScreenPtr pScreen);
Bool SiSHWCursorInit(ScreenPtr pScreen);
Bool SISDGAInit(ScreenPtr pScreen);
void SISInitVideo(ScreenPtr pScreen);


#ifdef  DEBUG
static void SiSDumpModeInfo(ScrnInfoPtr pScrn, DisplayModePtr mode);
#endif

/*
 * This is intentionally screen-independent.  It indicates the binding
 * choice made in the first PreInit.
 */
static int pix24bpp = 0;
 

/* 
 * This contains the functions needed by the server after loading the driver
 * module.  It must be supplied, and gets passed back by the SetupProc
 * function in the dynamic case.  In the static case, a reference to this
 * is compiled in, and this requires that the name of this DriverRec be
 * an upper-case version of the driver name.
 */

DriverRec SIS = {
    SIS_CURRENT_VERSION,
    SIS_DRIVER_NAME,
    SISIdentify,
    SISProbe,
    SISAvailableOptions,
    NULL,
    0
};

static SymTabRec SISChipsets[] = {
#if 0 
    { PCI_CHIP_SG86C201,    "SIS86c201" },
    { PCI_CHIP_SG86C202,    "SIS86c202" },
    { PCI_CHIP_SG86C205,    "SIS86c205" },
    { PCI_CHIP_SG86C215,    "SIS86c215" },
    { PCI_CHIP_SG86C225,    "SIS86c225" },
#endif
    { PCI_CHIP_SIS5597,     "SIS5597" },
    { PCI_CHIP_SIS530,      "SIS530" },
    { PCI_CHIP_SIS6326,     "SIS6326" },
    { PCI_CHIP_SIS300,      "SIS300" },
    { PCI_CHIP_SIS630,      "SIS630" },
    { PCI_CHIP_SIS540,      "SIS540" },
    { -1,                   NULL }
};

static PciChipsets SISPciChipsets[] = {
#if 0
    { PCI_CHIP_SG86C201,    PCI_CHIP_SG86C201,  RES_SHARED_VGA },
    { PCI_CHIP_SG86C202,    PCI_CHIP_SG86C202,  RES_SHARED_VGA },
    { PCI_CHIP_SG86C205,    PCI_CHIP_SG86C205,  RES_SHARED_VGA },
    { PCI_CHIP_SG86C205,    PCI_CHIP_SG86C205,  RES_SHARED_VGA },
#endif
    { PCI_CHIP_SIS5597,     PCI_CHIP_SIS5597,   RES_SHARED_VGA },
    { PCI_CHIP_SIS530,      PCI_CHIP_SIS530,    RES_SHARED_VGA },
    { PCI_CHIP_SIS6326,     PCI_CHIP_SIS6326,   RES_SHARED_VGA },
    { PCI_CHIP_SIS300,      PCI_CHIP_SIS300,    RES_SHARED_VGA },
    { PCI_CHIP_SIS630,      PCI_CHIP_SIS630,    RES_SHARED_VGA },
    { PCI_CHIP_SIS540,      PCI_CHIP_SIS540,    RES_SHARED_VGA },
    { -1,                   -1,                 RES_UNDEFINED }
};
    

int sisReg32MMIO[]={0x8280,0x8284,0x8288,0x828C,0x8290,0x8294,0x8298,0x829C,
            0x82A0,0x82A4,0x82A8,0x82AC};
/* Engine Register for the 2nd Generation Graphics Engine */
int sis2Reg32MMIO[]={0x8200,0x8204,0x8208,0x820C,0x8210,0x8214,0x8218,0x821C,
            0x8220,0x8224,0x8228,0x822C,0x8230,0x8234,0x8238,0x823C,
             0x8240, 0x8300};

/* TW: The following was re-included because there are BIOSes out there that
 *     report incomplete mode lists. These are BIOS versions <2.01.2x
 *     NOTE: Mode numbers for 1280, 1600 and 1920 are unofficial but they work here!
 *     TW: VBE 3.0 on SiS630 does not support 24 fpp modes (only 32fpp when depth = 24);
 */
				       /*     8      16     24    32   */
static UShort  VESAModeIndex_640x480[]   = {0x100, 0x111, 0x112, 0x13a};
static UShort  VESAModeIndex_720x480[]   = {0x000, 0x000, 0x000, 0x000};
static UShort  VESAModeIndex_720x576[]   = {0x000, 0x000, 0x000, 0x000};
static UShort  VESAModeIndex_800x600[]   = {0x103, 0x114, 0x115, 0x13b};
static UShort  VESAModeIndex_1024x768[]  = {0x105, 0x117, 0x118, 0x13c};
static UShort  VESAModeIndex_1280x1024[] = {0x107, 0x11a, 0x11b, 0x13d};
static UShort  VESAModeIndex_1600x1200[] = {0x13e, 0x13f, 0x000, 0x140};
static UShort  VESAModeIndex_1920x1440[] = {0x141, 0x142, 0x000, 0x143};

static struct _sis_vrate {
    CARD16 idx;
    CARD16 xres;
    CARD16 yres;
    CARD16 refresh;
} sisx_vrate[] = {
    {1, 640, 480, 60},  {2, 640, 480, 72}, {3, 640, 480, 75},  {4, 640, 480, 85},
    {5, 640, 480, 100}, {6, 640, 480, 120}, {7, 640, 480, 160}, {8, 640, 480, 200},
    {1, 720, 480, 60}, {1, 720, 576, 50},
    {1, 800, 600, 56}, {2, 800, 600, 60}, {3, 800, 600, 72}, {4, 800, 600, 75},
    {5, 800, 600, 85}, {6, 800, 600, 100}, {7, 800, 600, 120}, {8, 800, 600, 160},
    {1, 1024, 768, 43}, {2, 1024, 768, 60}, {3, 1024, 768, 70}, {4, 1024, 768, 75},
    {5, 1024, 768, 85}, {6, 1024, 768, 100}, {7, 1024, 768, 120},
    {1, 1280, 1024, 43}, {2, 1280, 1024, 60}, {3, 1280, 1024, 75}, {4, 1280, 1024, 85},
    {1, 1600, 1200, 60}, {2, 1600, 1200, 65}, {3, 1600, 1200, 70}, {4, 1600, 1200, 75},
    {5, 1600, 1200, 85},
    {1, 1920, 1440, 60},
    {0, 0, 0, 0}
};


sisModeInfoPtr SISVesaModeList = NULL;

static const char *xaaSymbols[] = {
    "XAACopyROP",
    "XAACreateInfoRec",
    "XAADestroyInfoRec",
    "XAAFillSolidRects",
    "XAAHelpPatternROP",
    "XAAInit",
    NULL
};

static const char *vgahwSymbols[] = {
    "vgaHWFreeHWRec",
    "vgaHWGetHWRec",
    "vgaHWGetIOBase",
    "vgaHWGetIndex",
    "vgaHWInit",
    "vgaHWLock",
    "vgaHWMapMem",
    "vgaHWProtect",
    "vgaHWRestore",
    "vgaHWSave",
    "vgaHWSaveScreen",
    "vgaHWUnlock",
    NULL
};

static const char *miscfbSymbols[] = {
    "xf1bppScreenInit",
    "xf4bppScreenInit",
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
    NULL
};

static const char *i2cSymbols[] = {
    "xf86I2CBusInit",
    "xf86CreateI2CBusRec",
    NULL
};

static const char *vbeSymbols[] = {
    "VBEInit",
    "vbeDoEDID",
    "vbeFree",
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
    "drmSiSAgpInit",
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
    "GlxSetVisualConfigs",
    NULL
};
#endif


#ifdef XFree86LOADER

static MODULESETUPPROTO(sisSetup);

static XF86ModuleVersionInfo sisVersRec =
{
    SIS_DRIVER_NAME,
    MODULEVENDORSTRING,
    MODINFOSTRING1,
    MODINFOSTRING2,
    XF86_VERSION_CURRENT,
    SIS_MAJOR_VERSION, SIS_MINOR_VERSION, SIS_PATCHLEVEL,
    ABI_CLASS_VIDEODRV,         /* This is a video driver */
    ABI_VIDEODRV_VERSION,
    MOD_CLASS_VIDEODRV,
    {0,0,0,0}
};

XF86ModuleData sisModuleData = { &sisVersRec, sisSetup, NULL };

pointer
sisSetup(pointer module, pointer opts, int *errmaj, int *errmin)
{
    static Bool setupDone = FALSE;

    if (!setupDone) {
        setupDone = TRUE;
        xf86AddDriver(&SIS, module, 0);
        LoaderRefSymLists(vgahwSymbols, fbSymbols, i2cSymbols, xaaSymbols,
			  miscfbSymbols, shadowSymbols, ramdacSymbols,
			  vbeSymbols,
#ifdef XF86DRI
			  drmSymbols, driSymbols,
#endif
			  NULL);
        return (pointer)TRUE;
    } 

    if (errmaj) *errmaj = LDR_ONCEONLY;
    return NULL;
}

#endif /* XFree86LOADER */

static Bool
SISGetRec(ScrnInfoPtr pScrn)
{
    /*
     * Allocate an SISRec, and hook it into pScrn->driverPrivate.
     * pScrn->driverPrivate is initialised to NULL, so we can check if
     * the allocation has already been done.
     */
    if (pScrn->driverPrivate != NULL)
        return TRUE;

    pScrn->driverPrivate = xnfcalloc(sizeof(SISRec), 1);
    /* Initialise it */

    return TRUE;
}

static void
SISFreeRec(ScrnInfoPtr pScrn)
{
    SISPtr pSiS = SISPTR(pScrn);
    
    if (pSiS->pstate) xfree(pSiS->pstate);
    pSiS->pstate = NULL;
    if (pSiS->pVbe) vbeFree(pSiS->pVbe);
    pSiS->pVbe = NULL;
    if (pScrn->driverPrivate == NULL)
        return;
    xfree(pScrn->driverPrivate);
    pScrn->driverPrivate = NULL;
}

static void 
SISDisplayPowerManagementSet(ScrnInfoPtr pScrn, int PowerManagementMode, int flags)
{
    SISPtr pSiS = SISPTR(pScrn);
    unsigned char extDDC_PCR;
    unsigned char crtc17 = 0;
    unsigned char seq1 = 0;
    int vgaIOBase = VGAHWPTR(pScrn)->IOBase;

    xf86DrvMsgVerb(pScrn->scrnIndex, X_INFO, 3,
                 "SISDisplayPowerManagementSet(%d)\n",PowerManagementMode);

    outb(vgaIOBase + 4, 0x17);
    crtc17 = inb(vgaIOBase + 5);
    /* enable access to extended sequencer registers */
    outw(VGA_SEQ_INDEX, 0x8605);
    outb(VGA_SEQ_INDEX, 0x11);
    extDDC_PCR = inb(VGA_SEQ_DATA);
    /* if not blanked obtain state of LCD blank flags set by BIOS */
    if (!pSiS->Blank)
	pSiS->LCDon = extDDC_PCR;
    /* erase LCD blank flags */
    extDDC_PCR &= ~0xC;
    
    switch (PowerManagementMode)
    {
        case DPMSModeOn:
            /* HSync: On, VSync: On */
            seq1 = 0x00 ;
	    /* don't just unblanking; use LCD state set by BIOS */
	    extDDC_PCR  |= (pSiS->LCDon & 0x0C);
	    pSiS->Blank = FALSE;
            crtc17 |= 0x80;
            break;
        case DPMSModeStandby:
            /* HSync: Off, VSync: On */
            seq1 = 0x20 ;
            extDDC_PCR |= 0x8;
	    pSiS->Blank = TRUE;
            break;
        case DPMSModeSuspend:
            /* HSync: On, VSync: Off */
            seq1 = 0x20 ;
            extDDC_PCR |= 0x8;
	    pSiS->Blank = TRUE;
            break;
        case DPMSModeOff:
            /* HSync: Off, VSync: Off */
            seq1 = 0x20 ;
            extDDC_PCR |= 0xC;
	    pSiS->Blank = TRUE;
            /* DPMSModeOff is not supported with ModeStandby | ModeSuspend  */
            /* need same as the generic VGA function */
            crtc17 &= ~0x80;
            break;
    }
    outw(VGA_SEQ_INDEX, 0x0100);    /* Synchronous Reset */
    outb(VGA_SEQ_INDEX, 0x01);  /* Select SEQ1 */
    seq1 |= inb(VGA_SEQ_DATA) & ~0x20;
    outb(VGA_SEQ_DATA, seq1);
    usleep(10000);
    outb(vgaIOBase + 4, 0x17);
    outb(vgaIOBase + 5, crtc17);
    outb(VGA_SEQ_INDEX, 0x11);
    outb(VGA_SEQ_DATA, extDDC_PCR);
    outw(VGA_SEQ_INDEX, 0x0300);    /* End Reset */
}


/* Mandatory */
static void
SISIdentify(int flags)
{
    xf86PrintChipsets(SIS_NAME, "driver for SiS chipsets", SISChipsets);
}

static void
SIS1bppColorMap(ScrnInfoPtr pScrn)
{
   SISPtr pSiS = SISPTR(pScrn);

   outb(pSiS->RelIO+0x48, 0x00);
   outb(pSiS->RelIO+0x49, 0x00);
   outb(pSiS->RelIO+0x49, 0x00);
   outb(pSiS->RelIO+0x49, 0x00);

   outb(pSiS->RelIO+0x48, 0x3F);
   outb(pSiS->RelIO+0x49, 0x3F);
   outb(pSiS->RelIO+0x49, 0x3F);
   outb(pSiS->RelIO+0x49, 0x3F);
}

/* Mandatory */
static Bool
SISProbe(DriverPtr drv, int flags)
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
     * Since this test version still uses vgaHW, we'll only actually claim
     * one for now, and just print a message about the others.
     */

    /*
     * Next we check, if there has been a chipset override in the config file.
     * For this we must find out if there is an active device section which
     * is relevant, i.e., which has no driver specified or has THIS driver
     * specified.
     */

    if ((numDevSections = xf86MatchDevice(SIS_DRIVER_NAME,
                      &devSections)) <= 0) {
        /*
         * There's no matching device section in the config file, so quit
         * now.
         */
        return FALSE;
    }

    /*
     * While we're VGA-dependent, can really only have one such instance, but
     * we'll ignore that.
     */

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

    numUsed = xf86MatchPciInstances(SIS_NAME, PCI_VENDOR_SIS,
               SISChipsets, SISPciChipsets, devSections,
               numDevSections, drv, &usedChips);

    /* Free it since we don't need that list after this */
    xfree(devSections);
    if (numUsed <= 0)
        return FALSE;

    if (flags & PROBE_DETECT)
        foundScreen = TRUE;
    else for (i = 0; i < numUsed; i++) {
        ScrnInfoPtr pScrn;

        /* Allocate a ScrnInfoRec and claim the slot */
        pScrn = NULL;

        if ((pScrn = xf86ConfigPciEntity(pScrn, 0, usedChips[i],
                                            SISPciChipsets, NULL, NULL,
                                            NULL, NULL, NULL))) {
            /* Fill in what we can of the ScrnInfoRec */
            pScrn->driverVersion    = SIS_CURRENT_VERSION;
            pScrn->driverName       = SIS_DRIVER_NAME;
            pScrn->name             = SIS_NAME;
            pScrn->Probe            = SISProbe;
            pScrn->PreInit          = SISPreInit;
            pScrn->ScreenInit       = SISScreenInit;
            pScrn->SwitchMode       = SISSwitchMode;
            pScrn->AdjustFrame      = SISAdjustFrame;
            pScrn->EnterVT          = SISEnterVT;
            pScrn->LeaveVT          = SISLeaveVT;
            pScrn->FreeScreen       = SISFreeScreen;
            pScrn->ValidMode        = SISValidMode;
            foundScreen = TRUE;
        }
    }
    xfree(usedChips);
    return foundScreen;
}

#if 0 /* xf86ValidateModes() takes care of this */
/*
 * GetAccelPitchValues -
 *
 * This function returns a list of display width (pitch) values that can
 * be used in accelerated mode.
 */
static int
GetAccelPitchValues(ScrnInfoPtr pScrn)
{
    return ((pScrn->displayWidth + 7) & ~7);
}
#endif


/* Mandatory */
static Bool
SISPreInit(ScrnInfoPtr pScrn, int flags)
{
    SISPtr pSiS;
    MessageType from;
    int vgaIOBase;
    unsigned char unlock;
    unsigned long int i;
    ClockRangePtr clockRanges;
    char *mod = NULL;
    const char *Sym = NULL;
    int pix24flags;

    vbeInfoPtr pVbe;
    VbeInfoBlock *vbe;

    if (flags & PROBE_DETECT) {
        if (xf86LoadSubModule(pScrn, "vbe")) {
        	int index = xf86GetEntityInfo(pScrn->entityList[0])->index;
        	if ((pVbe = VBEInit(NULL,index))) {
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
     * be initialised here.  xf86Screens[] is (pScrn is a pointer to one
     * of these).  Privates allocated using xf86AllocateScrnInfoPrivateIndex()  
     * are too, and should be used for data that must persist across
     * server generations.
     *
     * Per-generation data should be allocated with
     * AllocateScreenPrivateIndex() from the ScreenInit() function.
     */

    /* Check the number of entities, and fail if it isn't one. */
    if (pScrn->numEntities != 1)
        return FALSE;

    /* The vgahw module should be loaded here when needed */
    if (!xf86LoadSubModule(pScrn, "vgahw"))
        return FALSE;

    xf86LoaderReqSymLists(vgahwSymbols, NULL);

    /*
     * Allocate a vgaHWRec
     */
    if (!vgaHWGetHWRec(pScrn))
        return FALSE;

    VGAHWPTR(pScrn)->MapSize = 0x10000;     /* Standard 64k VGA window */

    if (!vgaHWMapMem(pScrn))
        return FALSE;
    vgaHWGetIOBase(VGAHWPTR(pScrn));
    vgaIOBase = VGAHWPTR(pScrn)->IOBase;

    /* Allocate the SISRec driverPrivate */
    if (!SISGetRec(pScrn)) {
        return FALSE;
    }
    pSiS = SISPTR(pScrn);
    pSiS->pScrn = pScrn;

    /* Get the entity, and make sure it is PCI. */
    pSiS->pEnt = xf86GetEntityInfo(pScrn->entityList[0]);
    if (pSiS->pEnt->location.type != BUS_PCI)
        return FALSE;

    /* Find the PCI info for this screen */
    pSiS->PciInfo = xf86GetPciInfoForEntity(pSiS->pEnt->index);
    pSiS->PciTag = pciTag(pSiS->PciInfo->bus, pSiS->PciInfo->device,
              pSiS->PciInfo->func);

    /*
     * XXX This could be refined if some VGA memory resources are not
     * decoded in operating mode.
     */
    {
        resRange vgamem[] = {   {ResShrMemBlock,0xA0000,0xAFFFF},
                                {ResShrMemBlock,0xB0000,0xB7FFF},
                                {ResShrMemBlock,0xB8000,0xBFFFF},
                            _END };
        xf86SetOperatingState(vgamem, pSiS->pEnt->index, ResUnusedOpr);
    }

    /* Operations for which memory access is required */
    pScrn->racMemFlags = RAC_FB | RAC_COLORMAP | RAC_CURSOR | RAC_VIEWPORT;
    /* Operations for which I/O access is required (XXX check this) */
    pScrn->racIoFlags = RAC_COLORMAP | RAC_CURSOR | RAC_VIEWPORT;

    /* The ramdac module should be loaded here when needed */
    if (!xf86LoadSubModule(pScrn, "ramdac"))
        return FALSE;

    xf86LoaderReqSymLists(ramdacSymbols, NULL);

    /* Set pScrn->monitor */
    pScrn->monitor = pScrn->confScreen->monitor;

    /* TW: ---EGBERT: Remove this before committing !*/
    xf86DrvMsg(pScrn->scrnIndex, X_PROBED,
           "Unofficial driver (16.01.02) by Thomas Winischhofer\n");

    /*
     * Set the Chipset and ChipRev, allowing config file entries to
     * override.
     */
    if (pSiS->pEnt->device->chipset && *pSiS->pEnt->device->chipset)  {
        pScrn->chipset = pSiS->pEnt->device->chipset;
        pSiS->Chipset = xf86StringToToken(SISChipsets, pScrn->chipset);
        from = X_CONFIG;
    } else if (pSiS->pEnt->device->chipID >= 0) {
        pSiS->Chipset = pSiS->pEnt->device->chipID;
        pScrn->chipset = (char *)xf86TokenToString(SISChipsets, pSiS->Chipset);

        from = X_CONFIG;
        xf86DrvMsg(pScrn->scrnIndex, X_CONFIG, "ChipID override: 0x%04X\n",
                                pSiS->Chipset);
    } else {
        from = X_PROBED;
        pSiS->Chipset = pSiS->PciInfo->chipType;
        pScrn->chipset = (char *)xf86TokenToString(SISChipsets, pSiS->Chipset);
    }
    if (pSiS->pEnt->device->chipRev >= 0) {
        pSiS->ChipRev = pSiS->pEnt->device->chipRev;
        xf86DrvMsg(pScrn->scrnIndex, X_CONFIG, "ChipRev override: %d\n",
                        pSiS->ChipRev);
    } else {
        pSiS->ChipRev = pSiS->PciInfo->chipRev;
    }

    /*
     * This shouldn't happen because such problems should be caught in
     * SISProbe(), but check it just in case.
     */
    if (pScrn->chipset == NULL) {
        xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
                   "ChipID 0x%04X is not recognised\n", pSiS->Chipset);
        return FALSE;
    }
    if (pSiS->Chipset < 0) {
        xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
                   "Chipset \"%s\" is not recognised\n", pScrn->chipset);
        return FALSE;
    }


    /*
     * The first thing we should figure out is the depth, bpp, etc.
     * Our default depth is 8, so pass it to the helper function.
     * Our preference for depth 24 is 24bpp, so tell it that too.
     */
    switch (pSiS->Chipset) {
    case PCI_CHIP_SIS530:
    	pix24flags = Support32bppFb | Support24bppFb |
                SupportConvert24to32 | SupportConvert32to24;
        break;
    case PCI_CHIP_SIS300:
    case PCI_CHIP_SIS630:
    case PCI_CHIP_SIS540:
    	pix24flags = Support32bppFb | SupportConvert24to32;
	break;
    default:
        pix24flags = Support24bppFb |
		SupportConvert32to24 | PreferConvert32to24;
	break;
    }

    if (!xf86SetDepthBpp(pScrn, 8, 8, 8, pix24flags))
    	return FALSE;

    /* Check that the returned depth is one we support */
    switch (pScrn->depth) {
    case 8:
    case 15:
    case 16:
    case 24:
        /* OK */
        break;
    default:
        xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
               "Given depth (%d) is not supported by this driver\n",
               pScrn->depth);
        return FALSE;
    }

    xf86PrintDepthBpp(pScrn);

    /* Get the depth24 pixmap format */
    if (pScrn->depth == 24 && pix24bpp == 0)
        pix24bpp = xf86GetBppFromDepth(pScrn, 24);

    /*
     * This must happen after pScrn->display has been set because
     * xf86SetWeight references it.
     */
    if (pScrn->depth > 8) {
        /* The defaults are OK for us */
        rgb zeros = {0, 0, 0};

        if (!xf86SetWeight(pScrn, zeros, zeros)) {
            return FALSE;
        } else {
            /* XXX check that weight returned is supported */
                ;
        }
    }

    if (!xf86SetDefaultVisual(pScrn, -1)) {
        return FALSE;
    } else {
        /* We don't currently support DirectColor at > 8bpp */
        if (pScrn->depth > 8 && pScrn->defaultVisual != TrueColor) {
            xf86DrvMsg(pScrn->scrnIndex, X_ERROR, "Given default visual"
                        " (%s) is not supported at depth %d\n",
                        xf86GetVisualName(pScrn->defaultVisual), pScrn->depth);
            return FALSE;
        }
    }

    /*
     * The new cmap layer needs this to be initialised.
     */

    {
        Gamma zeros = {0.0, 0.0, 0.0};

        if (!xf86SetGamma(pScrn, zeros)) {
            return FALSE;
        }
    }

    /* We use a programmable clock */
    pScrn->progClock = TRUE;

    /* Set the bits per RGB for 8bpp mode */
    if (pScrn->depth == 8) {
        pScrn->rgbBits = 6;
    }

    pSiS->ddc1Read = SiSddc1Read;   /* this cap will be modified */

    from = X_DEFAULT;

    outb(VGA_SEQ_INDEX, 0x05); unlock = inb(VGA_SEQ_DATA);
    outw(VGA_SEQ_INDEX, 0x8605); /* Unlock registers */

    /* get VBIOS image */
    if (!(pSiS->BIOS=xcalloc(1, BIOS_SIZE)))  {
        ErrorF("Allocate memory fail !!\n");
        return FALSE;
    }
    if (xf86ReadBIOS(BIOS_BASE, 0, pSiS->BIOS, BIOS_SIZE) != BIOS_SIZE)  {
        xfree(pSiS->BIOS);
        ErrorF("Read VBIOS image fail !!\n");
        return FALSE;
    }

    SiSOptions(pScrn);
    SiSSetup(pScrn);

    from = X_PROBED;
    if (pSiS->pEnt->device->MemBase != 0) {
        /*
         * XXX Should check that the config file value matches one of the
         * PCI base address values.
         */
        pSiS->FbAddress = pSiS->pEnt->device->MemBase;
        from = X_CONFIG;
    } else {
        pSiS->FbAddress = pSiS->PciInfo->memBase[0] & 0xFFFFFFF0;
    }

    xf86DrvMsg(pScrn->scrnIndex, from, "Linear framebuffer at 0x%lX\n",
           (unsigned long)pSiS->FbAddress);

    if (pSiS->pEnt->device->IOBase != 0) {
        /*
         * XXX Should check that the config file value matches one of the
         * PCI base address values.
         */
        pSiS->IOAddress = pSiS->pEnt->device->IOBase;
        from = X_CONFIG;
    } else {
        pSiS->IOAddress = pSiS->PciInfo->memBase[1] & 0xFFFFFFF0;
    }

    from = X_PROBED;
    xf86DrvMsg(pScrn->scrnIndex, from, "MMIO registers at 0x%lX\n",
           (unsigned long)pSiS->IOAddress);

    pSiS->RelIO = pSiS->PciInfo->ioBase[2] & 0xFFFC;
    xf86DrvMsg(pScrn->scrnIndex, from, "Relocate IO registers at 0x%lX\n",
           (unsigned long)pSiS->RelIO);

    /* Register the PCI-assigned resources. */
    if (xf86RegisterResources(pSiS->pEnt->index, NULL, ResExclusive)) {
        xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
               "xf86RegisterResources() found resource conflicts\n");
        return FALSE;
    }

    from = X_PROBED;
    if (pSiS->pEnt->device->videoRam != 0)  {
        pScrn->videoRam = pSiS->pEnt->device->videoRam;
        from = X_CONFIG;
    }

    if ((pSiS->Chipset == PCI_CHIP_SIS6326)
			&& (pScrn->videoRam >= 8192)
			&& (from != X_CONFIG)) {
        pScrn->videoRam = 4096;
        xf86DrvMsg(pScrn->scrnIndex, from, "Limiting VideoRAM to %d KB\n",
               pScrn->videoRam);
    } else
        xf86DrvMsg(pScrn->scrnIndex, from, "VideoRAM: %d KB\n",
               pScrn->videoRam);

    /*
     *  TW: New option: limit size of framebuffer memory for avoiding
     *  clash with DRI:
     *  Kernel framebuffer driver (sisfb) starts its memory heap
     *  at 8MB if it detects more VideoRAM than that(otherwise at 4MB).
     *  Therefore a setting of 8192 is recommended if DRI is
     *  to be used when there's more than 8MB video RAM available.
     *  This option can be left out if DRI is not to be used.
     *  Attention: TurboQueue and HWCursor should use videoRam value,
     *  not FbMapSize; these two are always located at the very top
     *  of the videoRAM. Both are already initialized by framebuffer
     *  driver, so they should not wander around while starting X.
     */

    pSiS->FbMapSize = pScrn->videoRam * 1024;
    /* TW: Touching FbMapSize doesn't work; now use maxxfbmem in accel*.c */

    if (pSiS->maxxfbmem) {
    	if (pSiS->maxxfbmem > pSiS->FbMapSize) {
	    xf86DrvMsg(pScrn->scrnIndex, X_CONFIG,
            "Invalid MaxXFBMem setting. Using all VideoRAM for framebuffer\n");
	    pSiS->maxxfbmem = pSiS->FbMapSize;
	}
    } else pSiS->maxxfbmem = pSiS->FbMapSize;

    /* TW: Detect video bridge */
    SISVGAPreInit(pScrn);
    /* TW: Detect CRT2-LCD and LCD size */
    SISLCDPreInit(pScrn);
    /* TW: Detect CRT2-TV and PAL/NTSC mode */
    SISTVPreInit(pScrn);
    /* TW: Detect CRT2-VGA */
    SISCRT2PreInit(pScrn);
    /* TW: Eventually overrule detected CRT2 type */
    if (pSiS->ForceCRT2Type == CRT2_DEFAULT)
    {
        if (pSiS->VBFlags & CRT2_VGA)
           pSiS->ForceCRT2Type = CRT2_VGA;
        else if (pSiS->VBFlags & CRT2_LCD)
           pSiS->ForceCRT2Type = CRT2_LCD;
        else if (pSiS->VBFlags & CRT2_TV)
           pSiS->ForceCRT2Type = CRT2_TV;
    }
    switch (pSiS->ForceCRT2Type)
    {
    case CRT2_TV:
        pSiS->VBFlags = pSiS->VBFlags & ~(CRT2_LCD | CRT2_VGA);
        if (pSiS->VBFlags & VB_VIDEOBRIDGE)
            pSiS->VBFlags = pSiS->VBFlags | CRT2_TV;
        else
            pSiS->VBFlags = pSiS->VBFlags & ~(CRT2_TV);
        break;
     case CRT2_LCD:
        pSiS->VBFlags = pSiS->VBFlags & ~(CRT2_TV | CRT2_VGA);
        if (pSiS->VBFlags & VB_VIDEOBRIDGE)
            pSiS->VBFlags = pSiS->VBFlags | CRT2_LCD;
        else
            pSiS->VBFlags = pSiS->VBFlags & ~(CRT2_LCD);
        break;
     case CRT2_VGA:
        pSiS->VBFlags = pSiS->VBFlags & ~(CRT2_TV | CRT2_LCD);
        if (pSiS->VBFlags & VB_VIDEOBRIDGE)
            pSiS->VBFlags = pSiS->VBFlags | CRT2_VGA;
        else
            pSiS->VBFlags = pSiS->VBFlags & ~(CRT2_VGA);
        break;
      default:
        pSiS->VBFlags &= ~(CRT2_TV | CRT2_LCD | CRT2_VGA);
    }

    /* TW: Check if CRT1 used (or needed; this if no CRT2 detected) */
    if (pSiS->VBFlags & VB_VIDEOBRIDGE) {
        if (!(pSiS->VBFlags & (CRT2_VGA | CRT2_LCD | CRT2_TV)))
	    pSiS->CRT1off = 0;
    }
    else /* TW: no video bridge? Then we NEED CRT1! */
        pSiS->CRT1off = 0;

    /* TW: Determine CRT1<>CRT2 mode */
    if (pSiS->VBFlags & DISPTYPE_DISP2) {
        if (pSiS->CRT1off)	/* TW: CRT2 only */
	     pSiS->VBFlags |= VB_DISPMODE_SINGLE;
	else			/* TW: CRT1 and CRT2 - mirror image */
	     pSiS->VBFlags |= (VB_DISPMODE_MIRROR | DISPTYPE_CRT1);
    } else			/* TW: CRT1 only */
             pSiS->VBFlags |= (VB_DISPMODE_SINGLE | DISPTYPE_CRT1);

    SISDACPreInit(pScrn);

    /* Lock extended registers */
    outw(VGA_SEQ_INDEX, (unlock << 8) | 0x05);

    /* Set the min pixel clock */
    pSiS->MinClock = 16250; /* XXX Guess, need to check this */
    xf86DrvMsg(pScrn->scrnIndex, X_DEFAULT, "Min pixel clock is %d MHz\n",
                pSiS->MinClock / 1000);

    from = X_PROBED;
    /*
     * If the user has specified ramdac speed in the XF86Config
     * file, we respect that setting.
     */
    if (pSiS->pEnt->device->dacSpeeds[0]) {
        int speed = 0;

        switch (pScrn->bitsPerPixel) {
        case 8:
           speed = pSiS->pEnt->device->dacSpeeds[DAC_BPP8];
           break;
        case 16:
           speed = pSiS->pEnt->device->dacSpeeds[DAC_BPP16];
           break;
        case 24:
           speed = pSiS->pEnt->device->dacSpeeds[DAC_BPP24];
           break;
        case 32:
           speed = pSiS->pEnt->device->dacSpeeds[DAC_BPP32];
           break;
        }
        if (speed == 0)
            pSiS->MaxClock = pSiS->pEnt->device->dacSpeeds[0];
        else
            pSiS->MaxClock = speed;
        from = X_CONFIG;
    }
    xf86DrvMsg(pScrn->scrnIndex, from, "Max pixel clock is %d MHz\n",
                pSiS->MaxClock / 1000);

    /*
     * Setup the ClockRanges, which describe what clock ranges are available,
     * and what sort of modes they can be used for.
     */
    clockRanges = xnfcalloc(sizeof(ClockRange), 1);
    clockRanges->next = NULL;
    clockRanges->minClock = pSiS->MinClock;
    clockRanges->maxClock = pSiS->MaxClock;
    clockRanges->clockIndex = -1;               /* programmable */
    clockRanges->interlaceAllowed = TRUE;
    clockRanges->doubleScanAllowed = TRUE;      /* XXX check this */

    /*
     * xf86ValidateModes will check that the mode HTotal and VTotal values
     * don't exceed the chipset's limit if pScrn->maxHValue and
     * pScrn->maxVValue are set.  Since our SISValidMode() already takes
     * care of this, we don't worry about setting them here.
     */

    /* Select valid modes from those available */
    /*
     * XXX Assuming min pitch 256, max 4096 ==> 8192
     * XXX Assuming min height 128, max 4096
     */
    i = xf86ValidateModes(pScrn, pScrn->monitor->Modes,
                      pScrn->display->modes, clockRanges,
                      NULL, 256, 8192,
                      pScrn->bitsPerPixel * 8, 128, 4096,
                      pScrn->display->virtualX,
                      pScrn->display->virtualY,
                      pSiS->maxxfbmem,
                      LOOKUP_BEST_REFRESH);

    if (i == -1) {
        SISFreeRec(pScrn);
        return FALSE;
    }

    /* Prune the modes marked as invalid */
    xf86PruneDriverModes(pScrn);

    if (i == 0 || pScrn->modes == NULL) {
        xf86DrvMsg(pScrn->scrnIndex, X_ERROR, "No valid modes found\n");
        SISFreeRec(pScrn);
        return FALSE;
    }

    xf86SetCrtcForModes(pScrn, INTERLACE_HALVE_V);

    /* Set the current mode to the first in the list */
    pScrn->currentMode = pScrn->modes;

    /* Print the list of modes being used */
    xf86PrintModes(pScrn);

    /* Set display resolution */
    xf86SetDpi(pScrn, 0, 0);

    /* Load bpp-specific modules */
    switch (pScrn->bitsPerPixel) {
    case 1:
        mod = "xf1bpp";
        Sym = "xf1bppScreenInit";
        break;
    case 4:
        mod = "xf4bpp";
        Sym = "xf4bppScreenInit";
        break;
    case 8:
    case 16:
    case 24:
    case 32:
        mod = "fb";
	break;
    }

    if (mod && xf86LoadSubModule(pScrn, mod) == NULL) {
        SISFreeRec(pScrn);
        return FALSE;
    }

    if (mod) {
	if (Sym) {
	    xf86LoaderReqSymbols(Sym, NULL);
	} else {
	    xf86LoaderReqSymLists(fbSymbols, NULL);
	}
    }

    if (!xf86LoadSubModule(pScrn, "i2c")) {
        SISFreeRec(pScrn);
        return FALSE;
    }

    xf86LoaderReqSymLists(i2cSymbols, NULL);

    /* Load XAA if needed */
    if (!pSiS->NoAccel) {
        xf86DrvMsg(pScrn->scrnIndex,X_INFO, "Accel Enable\n");
        if (!xf86LoadSubModule(pScrn, "xaa")) {
            SISFreeRec(pScrn);
            return FALSE;
        }

        xf86LoaderReqSymLists(xaaSymbols, NULL);
    }

    /* Load shadowfb if needed */
    if (pSiS->ShadowFB) {
        if (!xf86LoadSubModule(pScrn, "shadowfb")) {
            SISFreeRec(pScrn);
            return FALSE;
        }
        xf86LoaderReqSymLists(shadowSymbols, NULL);
    }

    /* Load DDC if needed */
    /* This gives us DDC1 - we should be able to get DDC2B using i2c */
    if (!xf86LoadSubModule(pScrn, "ddc")) {
        SISFreeRec(pScrn);
        return FALSE;
    }
    xf86LoaderReqSymLists(ddcSymbols, NULL);

/* TW: Now load and initialize VBE module. The default behavior
 *     for SiS630 with SiS301B, SiS302 or LVDS/CHRONTEL bridge
 *     is to use VESA for mode switching. This can be overruled
 *     with the option "VESA".
 */

    {
	Bool ret;
	pSiS->UseVESA=0;
	if (xf86LoadSubModule(pScrn, "vbe")) {
	    xf86LoaderReqSymLists(vbeSymbols, NULL);
	    if ((pSiS->pVbe = VBEInit(NULL,pSiS->pEnt->index))) {
		ret = xf86SetDDCproperties(pScrn,
				   xf86PrintEDID(vbeDoEDID(pSiS->pVbe,NULL)));
		if ( (pSiS->VESA == 1)
		    || (   (pSiS->VESA != 0)
			&& (pSiS->Chipset == PCI_CHIP_SIS630)
			&& (pSiS->VBFlags & (VB_301B|VB_302|VB_LVDS|VB_CHRONTEL))) ) {
		    vbe = VBEGetVBEInfo(pSiS->pVbe);
		    pSiS->vesamajor = (unsigned)(vbe->VESAVersion >> 8);
		    pSiS->vesaminor = vbe->VESAVersion & 0xff;
		    pSiS->vbeInfo = vbe;
		    SiSBuildVesaModeList(pScrn, pSiS->pVbe, vbe);
		    VBEFreeVBEInfo(vbe);
		    pSiS->UseVESA = 1;
		/* TW: from now, use VESA functions for mode switching */
		}
	    }
	}
	vbeFree(pSiS->pVbe);
	pSiS->pVbe = NULL;
    }
    
#if 0
    if (!ret && pSiS->ddc1Read)
        xf86SetDDCProperties(xf86PrintEDID(xf86DoEDID_DDC1(
             pScrn->scrnIndex,vgaHWddc1SetSpeed,pSiS->ddc1Read )));
#endif

    return TRUE;
}


/*
 * Map the framebuffer and MMIO memory.
 */

static Bool
SISMapMem(ScrnInfoPtr pScrn)
{
    SISPtr pSiS;
    int mmioFlags;

    pSiS = SISPTR(pScrn);

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
    pSiS->IOBase = xf86MapPciMem(pScrn->scrnIndex, mmioFlags, 
                        pSiS->PciTag, pSiS->IOAddress, 0x10000);
    if (pSiS->IOBase == NULL)
        return FALSE;

#ifdef __alpha__
    /*
     * for Alpha, we need to map DENSE memory as well, for
     * setting CPUToScreenColorExpandBase.
     */
    pSiS->IOBaseDense = xf86MapPciMem(pScrn->scrnIndex, VIDMEM_MMIO,
                    pSiS->PciTag, pSiS->IOAddress, 0x10000);

    if (pSiS->IOBaseDense == NULL)
        return FALSE;
#endif /* __alpha__ */

    pSiS->FbBase = xf86MapPciMem(pScrn->scrnIndex, VIDMEM_FRAMEBUFFER,
                         pSiS->PciTag,
                         (unsigned long)pSiS->FbAddress,
                         pSiS->FbMapSize);
    if (pSiS->FbBase == NULL)
        return FALSE;

    return TRUE;
}


/*
 * Unmap the framebuffer and MMIO memory.
 */

static Bool
SISUnmapMem(ScrnInfoPtr pScrn)
{
    SISPtr pSiS;

    pSiS = SISPTR(pScrn);

    /*
     * Unmap IO registers to virtual address space
     */ 
    xf86UnMapVidMem(pScrn->scrnIndex, (pointer)pSiS->IOBase, 0x10000);
    pSiS->IOBase = NULL;

#ifdef __alpha__
    xf86UnMapVidMem(pScrn->scrnIndex, (pointer)pSiS->IOBaseDense, 0x10000);
    pSiS->IOBaseDense = NULL;
#endif /* __alpha__ */

    xf86UnMapVidMem(pScrn->scrnIndex, (pointer)pSiS->FbBase, pSiS->FbMapSize);
    pSiS->FbBase = NULL;
    return TRUE;
}

/*
 * This function saves the video state.
 */
static void
SISSave(ScrnInfoPtr pScrn)
{
    SISPtr pSiS;
    vgaRegPtr vgaReg;
    SISRegPtr sisReg;

    pSiS = SISPTR(pScrn);
    vgaReg = &VGAHWPTR(pScrn)->SavedReg;
    sisReg = &pSiS->SavedReg;

    vgaHWSave(pScrn, vgaReg, VGA_SR_ALL);

    (*pSiS->SiSSave)(pScrn, sisReg);
    if(pSiS->UseVESA) SISVESASaveRestore(pScrn, MODE_SAVE);
}

static void
SISVESASaveRestore(ScrnInfoPtr pScrn, vbeSaveRestoreFunction function)
{
    SISPtr pSiS;

    pSiS = SISPTR(pScrn);

    if (pSiS->vesamajor > 1
	&& (function == MODE_SAVE || pSiS->pstate)) {
	if (function == MODE_RESTORE)
	    memcpy(pSiS->state, pSiS->pstate, pSiS->stateSize);
	ErrorF("VBESaveRestore\n");
	if ((VBESaveRestore(pSiS->pVbe,function,
				     (pointer)&pSiS->state,
			    &pSiS->stateSize,&pSiS->statePage))) {
	    if (function == MODE_SAVE) {
		/* don't rely on the memory not being touched */
		if (pSiS->pstate == NULL)
		    pSiS->pstate = xalloc(pSiS->stateSize);
		memcpy(pSiS->pstate, pSiS->state, pSiS->stateSize);
	    }
	    ErrorF("VBESaveRestore done with success\n");
	    return;
	}
	ErrorF("VBESaveRestore done\n");
    } else {
	if (function == MODE_SAVE)
	    (void)VBEGetVBEMode(pSiS->pVbe, &pSiS->stateMode);
	else
	    VBESetVBEMode(pSiS->pVbe, pSiS->stateMode, NULL);
    }
}

/*
 * Initialise a new mode.  This is currently still using the old
 * "initialise struct, restore/write struct to HW" model.  That could
 * be changed.
 * TW: Why?
 */

static Bool
SISModeInit(ScrnInfoPtr pScrn, DisplayModePtr mode)
{
    vgaHWPtr hwp = VGAHWPTR(pScrn);
    vgaRegPtr vgaReg;
    SISPtr pSiS = SISPTR(pScrn);
    SISRegPtr sisReg;

    vgaHWUnlock(hwp);
	
    SISModifyModeInfo(mode);

    /* TW: Initialize SiS Port Reg definitions for externally used
     *     sis_bios functions.
     */
    SiSRegInit(pSiS->RelIO+0x30);

    if (pSiS->UseVESA) {  /* With VESA: */
	/*
	 * This order is required:
	 * The video bridge needs to be adjusted before the
	 * BIOS is run as the BIOS sets up CRT2 according to
	 * these register settings.
	 * After the BIOS is run, the bridges and turboqueue
	 * registers need to be readjusted as the BIOS may
	 * very probably have messed them up.
	 */
	SiSPreSetMode(pScrn, 1);
        /* TW: mode was pScrn->currentMode - VidModeExt did not work! */
 	if (!SiSSetVESAMode(pScrn, mode))
	    return FALSE;
	SiSPreSetMode(pScrn, 1);
	SiSPostSetMode(pScrn, &pSiS->ModeReg, 1);

 	/* Prepare the register contents */
	if (!(*pSiS->ModeInit)(pScrn, mode))
	    return FALSE;

	pScrn->vtSema = TRUE;

	/* Program the registers */
	vgaHWProtect(pScrn, TRUE);
	(*pSiS->SiSRestore)(pScrn, &pSiS->ModeReg);
	vgaHWProtect(pScrn, FALSE);
	PDEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
 			  "HDisplay: %d, VDisplay: %d  \n",
 			  mode->HDisplay, mode->VDisplay));
 
    } else { /* Without VESA: */
    	/* Initialise the ModeReg values */
    	if (!vgaHWInit(pScrn, mode))
	        return FALSE;

	if (!(*pSiS->ModeInit)(pScrn, mode))
	    return FALSE;

	pScrn->vtSema = TRUE;

	PDEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
			  "HDisplay: %d, VDisplay: %d  \n",
			  mode->HDisplay, mode->VDisplay));

	/* Program the registers */
	vgaHWProtect(pScrn, TRUE);
	vgaReg = &hwp->ModeReg;
	sisReg = &pSiS->ModeReg;

    	vgaReg->Attribute[0x10] = 0x01;
    	if (pScrn->bitsPerPixel > 8)
	    vgaReg->Graphics[0x05] = 0x00;

    	vgaHWRestore(pScrn, vgaReg, VGA_SR_MODE); 

	if ( (pSiS->Chipset == PCI_CHIP_SIS300) ||
	     (pSiS->Chipset == PCI_CHIP_SIS630) ||
	     (pSiS->Chipset == PCI_CHIP_SIS540) ) {
	    SiSPreSetMode(pScrn, 0);
	    if (!SiSBIOSSetMode(pScrn, mode))
		return FALSE;
	}
	else (*pSiS->SiSRestore)(pScrn, sisReg);
	
	vgaHWProtect(pScrn, FALSE);
    }

/* Reserved for debug
 *
    SiSDumpModeInfo(pScrn, mode);
 *
 */
    return TRUE;
}

static Bool
SiSSetVESAMode(ScrnInfoPtr pScrn, DisplayModePtr pMode)
{
    SISPtr pSiS;
    int mode;

    pSiS = SISPTR(pScrn);

    if (!(mode = CalcVESAModeIndex(pScrn, pMode))) return FALSE;
    ErrorF("mode: %x\n",mode);

    mode |= 1 << 15;			/* TW: Don't clear framebuffer */
    mode |= 1 << 14;   			/* TW: always use linear adressing */

    if (VBESetVBEMode(pSiS->pVbe, mode, NULL) == FALSE) {
	    xf86DrvMsg(pScrn->scrnIndex, X_ERROR, "Setting mode 0x%x failed\n",
	             mode & 0x0fff);
	    return (FALSE);
    }

    if (pMode->HDisplay != pScrn->virtualX)
	VBESetLogicalScanline(pSiS->pVbe, pScrn->virtualX);

    xf86DrvMsg(pScrn->scrnIndex, X_PROBED, "Setting mode 0x%x succeeded\n",
	       mode & 0x0fff);

    return (TRUE);
}


/*
 * Restore the initial (text) mode.
 */
static void
SISRestore(ScrnInfoPtr pScrn)
{
    vgaHWPtr hwp;
    vgaRegPtr vgaReg;
    SISPtr pSiS;
    SISRegPtr sisReg;

    hwp = VGAHWPTR(pScrn);
    pSiS = SISPTR(pScrn);
    vgaReg = &hwp->SavedReg;
    sisReg = &pSiS->SavedReg;

    vgaHWProtect(pScrn, TRUE);

    (*pSiS->SiSRestore)(pScrn, sisReg);

    vgaHWRestore(pScrn, vgaReg, VGA_SR_ALL);

    vgaHWProtect(pScrn, FALSE);
}

static void
SISVESARestore(ScrnInfoPtr pScrn)
{
   SISPtr pSiS = SISPTR(pScrn);

   if(pSiS->UseVESA) SISVESASaveRestore(pScrn, MODE_RESTORE);
}

/* TW: Restore bridge output registers - to be called BEFORE VESARestore */
static void
SISBridgeRestore(ScrnInfoPtr pScrn)
{
    SISPtr pSiS = SISPTR(pScrn);

    if ( (pSiS->Chipset == PCI_CHIP_SIS300) ||
         (pSiS->Chipset == PCI_CHIP_SIS630) ||
         (pSiS->Chipset == PCI_CHIP_SIS540) ) {

		SiSRestoreBridge(pScrn, &pSiS->SavedReg);
    }
}

/* Mandatory
 * This gets called at the start of each server generation */
static Bool
SISScreenInit(int scrnIndex, ScreenPtr pScreen, int argc, char **argv)
{
    /* The vgaHW references will disappear one day */
    ScrnInfoPtr pScrn;
    vgaHWPtr hwp;
    SISPtr pSiS;
    int ret;
    int init_picture = 0;
    VisualPtr visual;
    unsigned long OnScreenSize;
    int height, width, displayWidth;
    unsigned char *FBStart;

    /*
     * First get the ScrnInfoRec
     */
    pScrn = xf86Screens[pScreen->myNum];

    hwp = VGAHWPTR(pScrn);

    hwp->MapSize = 0x10000;         /* Standard 64k VGA window */

    pSiS = SISPTR(pScrn);

    if (pSiS->UseVESA)
	pSiS->pVbe = VBEInit(NULL,pSiS->pEnt->index);

    /* Map the VGA memory and get the VGA IO base */
    if (!vgaHWMapMem(pScrn))
        return FALSE;
    vgaHWGetIOBase(hwp);

    /* Map the SIS memory and MMIO areas */
    if (!SISMapMem(pScrn))
        return FALSE;

    /* Save the current state */
    SISSave(pScrn);

    /* Initialise the first mode */
    if (!SISModeInit(pScrn, pScrn->currentMode))
        return FALSE;

    /* Clear frame buffer */
    OnScreenSize = pScrn->displayWidth * pScrn->currentMode->VDisplay * (pScrn->bitsPerPixel / 8);
    memset(pSiS->FbBase, 0, OnScreenSize);

    /* Darken the screen for aesthetic reasons and set the viewport */
    SISSaveScreen(pScreen, SCREEN_SAVER_ON);
    SISAdjustFrame(scrnIndex, pScrn->frameX0, pScrn->frameY0, 0);

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

    if (pScrn->bitsPerPixel > 8) {
        if (!miSetVisualTypes(pScrn->depth, TrueColorMask, pScrn->rgbBits,
                              pScrn->defaultVisual))
	    return FALSE;
	
    } else {
        if (!miSetVisualTypes(pScrn->depth, 
                              miGetDefaultVisualMask(pScrn->depth),
                              pScrn->rgbBits, pScrn->defaultVisual))
		return FALSE;
    }

    width = pScrn->virtualX;
    height = pScrn->virtualY;
    displayWidth = pScrn->displayWidth;

    if (pSiS->Rotate) {
        height = pScrn->virtualX;
        width = pScrn->virtualY;
    }

    if (pSiS->ShadowFB) {
        pSiS->ShadowPitch = BitmapBytePad(pScrn->bitsPerPixel * width);
        pSiS->ShadowPtr = xalloc(pSiS->ShadowPitch * height);
        displayWidth = pSiS->ShadowPitch / (pScrn->bitsPerPixel >> 3);
        FBStart = pSiS->ShadowPtr;
    } else {
        pSiS->ShadowPtr = NULL;
        FBStart = pSiS->FbBase;
    }

    if (!miSetPixmapDepths())
	return FALSE;
    
    {
        static int GlobalHWQueueLength = 0;

        pSiS->cmdQueueLenPtr = &(GlobalHWQueueLength);
    }

#ifdef XF86DRI
    pSiS->directRenderingEnabled = SISDRIScreenInit(pScreen);
    /* Force the initialization of the context */
#endif

    /*
     * Call the framebuffer layer's ScreenInit function, and fill in other
     * pScreen fields.
     */

    switch (pScrn->bitsPerPixel) {
    case 1:
        ret = xf1bppScreenInit(pScreen, FBStart, width,
                        height, pScrn->xDpi, pScrn->yDpi, 
                        displayWidth);
        break;
    case 4:
        ret = xf4bppScreenInit(pScreen, FBStart, width,
                        height, pScrn->xDpi, pScrn->yDpi, 
                        displayWidth);
        break;
    case 8:
    case 16:
    case 24:
    case 32:
        ret = fbScreenInit(pScreen, FBStart, width,
                        height, pScrn->xDpi, pScrn->yDpi,
                        displayWidth, pScrn->bitsPerPixel);

	init_picture = 1;
        break;
    default:
        xf86DrvMsg(scrnIndex, X_ERROR,
               "Internal error: invalid bpp (%d) in SISScrnInit\n",
               pScrn->bitsPerPixel);
            ret = FALSE;
        break;
    }
    if (!ret)
    {
        ErrorF ("SetMode Error@!\n");
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
    } else if (pScrn->depth == 1) {
        SIS1bppColorMap(pScrn);
    }

    /* must be after RGB ordering fixed */
    if (init_picture)
        fbPictureInit(pScreen, 0, 0);
    if (!pSiS->ShadowFB) /* hardware cursor needs to wrap this layer */
        SISDGAInit(pScreen);
    xf86SetBlackWhitePixels(pScreen);

    if (!pSiS->NoAccel) {
        if ( pSiS->Chipset == PCI_CHIP_SIS300 ||
             pSiS->Chipset == PCI_CHIP_SIS630 ||
             pSiS->Chipset == PCI_CHIP_SIS540)
            SiS300AccelInit(pScreen);
        else if (pSiS->Chipset == PCI_CHIP_SIS530)
            SiS530AccelInit(pScreen);
        else
            SiSAccelInit(pScreen);
    }
    miInitializeBackingStore(pScreen);
    xf86SetBackingStore(pScreen);
    xf86SetSilkenMouse(pScreen);

    /* Initialise cursor functions */
    miDCInitialize (pScreen, xf86GetPointerScreenFuncs());

    if (pSiS->HWCursor)
        SiSHWCursorInit(pScreen);

    /* Initialise default colourmap */
    if (!miCreateDefColormap(pScreen))
        return FALSE;

/*  marked by archer for adding VB palette 
     if (!vgaHWHandleColormaps(pScreen))
        return FALSE;
*/

    if (!xf86HandleColormaps(pScreen, 256, 8, SISLoadPalette, NULL,
                    CMAP_RELOAD_ON_MODE_SWITCH))
        return FALSE;

    if(pSiS->ShadowFB) {
    RefreshAreaFuncPtr refreshArea = SISRefreshArea;

    if(pSiS->Rotate) {
        if (!pSiS->PointerMoved) {
        pSiS->PointerMoved = pScrn->PointerMoved;
        pScrn->PointerMoved = SISPointerMoved;
        }

       switch(pScrn->bitsPerPixel) {
       case 8:  refreshArea = SISRefreshArea8;  break;
       case 16: refreshArea = SISRefreshArea16; break;
       case 24: refreshArea = SISRefreshArea24; break;
       case 32: refreshArea = SISRefreshArea32; break;
       }
    }

    ShadowFBInit(pScreen, refreshArea);
    }
    
    xf86DPMSInit(pScreen, (DPMSSetProcPtr)SISDisplayPowerManagementSet, 0);

#ifdef XvExtension
	if (!pSiS->NoXvideo) {
        /* HW Xv for SiS630 */
        if (pSiS->Chipset == PCI_CHIP_SIS630) {
            xf86DrvMsg(pScrn->scrnIndex, X_INFO, "Using SiS630 HW Xv\n" );
            SISInitVideo(pScreen);
        } 
        else { /* generic Xv */

            XF86VideoAdaptorPtr *ptr;
            int n;

            n = xf86XVListGenericAdaptors(pScrn, &ptr);
            if (n) {
                xf86XVScreenInit(pScreen, ptr, n);
                xf86DrvMsg(pScrn->scrnIndex, X_INFO, "Using generic Xv\n" );
            }
        }
    }
#endif

#ifdef XF86DRI
    if (pSiS->directRenderingEnabled) {
        /* Now that mi, drm and others have done their thing,
         * complete the DRI setup.
         */
        pSiS->directRenderingEnabled = SISDRIFinishScreenInit(pScreen);
    }
    if (pSiS->directRenderingEnabled) {
        xf86DrvMsg(pScrn->scrnIndex, X_INFO, "direct rendering enabled\n");
        /* TODO */
        /* SISSetLFBConfig(pSiS); */
    } else {
        xf86DrvMsg(pScrn->scrnIndex, X_INFO, "direct rendering disabled\n");
    }
#endif

    pSiS->CloseScreen = pScreen->CloseScreen;
    pScreen->CloseScreen = SISCloseScreen;
    pScreen->SaveScreen = SISSaveScreen;

    /* Report any unused options (only for the first generation) */
    if (serverGeneration == 1) {
    xf86ShowUnusedOptions(pScrn->scrnIndex, pScrn->options);
    }

    /* Turn on the screen now */
    SISSaveScreen(pScreen, SCREEN_SAVER_OFF);

    return TRUE;
}


/* Usually mandatory */
Bool
SISSwitchMode(int scrnIndex, DisplayModePtr mode, int flags)
{
    return SISModeInit(xf86Screens[scrnIndex], mode);
}

/*
 * This function is used to initialize the Start Address - the first
 * displayed location in the video memory.
 */
/* Usually mandatory */
void
SISAdjustFrame(int scrnIndex, int x, int y, int flags)
{
    ScrnInfoPtr pScrn = xf86Screens[scrnIndex];
    SISPtr pSiS;
    vgaHWPtr hwp;
    int base = y * pScrn->displayWidth + x;
    int vgaIOBase;
    unsigned char SR5State, temp;

    hwp = VGAHWPTR(pScrn);
    pSiS = SISPTR(pScrn);
    vgaIOBase = VGAHWPTR(pScrn)->IOBase;

    if (pSiS->UseVESA) {
        /* TW: Let BIOS adjust frame if using VESA */
	VBESetDisplayStart(pSiS->pVbe, x, y, TRUE);
    }
    else {
    outb(VGA_SEQ_INDEX, 0x05); /* Unlock Registers */
    SR5State = inb(VGA_SEQ_DATA);
    outw(VGA_SEQ_INDEX, 0x8605);

    if (pScrn->bitsPerPixel < 8) {
        base = (y * pScrn->displayWidth + x + 3) >> 3;
    } else {
        base = y * pScrn->displayWidth + x ;
        /* calculate base bpp dep. */
        switch (pScrn->bitsPerPixel) {
          case 16:
            base >>= 1;
            break;
          case 24:
            base = ((base * 3)) >> 2;
            base -= base % 6;
            break;
          case 32:
            break;
          default:      /* 8bpp */
            base >>= 2;
            break;
        }
    }

    outw(vgaIOBase + 4, (base & 0x00FF00) | 0x0C);
    outw(vgaIOBase + 4, ((base & 0x00FF) << 8) | 0x0D);
    switch (pSiS->Chipset)  {
        case PCI_CHIP_SIS300:
        case PCI_CHIP_SIS630:
        case PCI_CHIP_SIS540:
            outb(VGA_SEQ_INDEX, 0x0D);
            temp = (base & 0xFF0000) >> 16;
            PDEBUG(xf86DrvMsgVerb(pScrn->scrnIndex, X_INFO, 3,
                    "3C5/0Dh set to hex %2X, base 0x%x\n", temp, base));
            outb(VGA_SEQ_DATA, temp);
            if (pSiS->VBFlags)  {
/*              SiSUnLockCRT2(pSiS->RelIO); */
                SiSUnLockCRT2(pSiS->RelIO+0x30);
                outSISIDXREG(pSiS->RelIO+4, 6, GETVAR8(base));
                outSISIDXREG(pSiS->RelIO+4, 5, GETBITS(base, 15:8));
                outSISIDXREG(pSiS->RelIO+4, 4, GETBITS(base, 23:16));
/*              SiSLockCRT2(pSiS->RelIO); */
                SiSLockCRT2(pSiS->RelIO+0x30);
            }
            break;
        default:
            outb(VGA_SEQ_INDEX, 0x27);
            temp = inb(VGA_SEQ_DATA) & 0xF0;
            temp |= (base & 0x0F0000) >> 16;
            PDEBUG(xf86DrvMsgVerb(pScrn->scrnIndex, X_INFO, 3,
                    "3C5/27h set to hex %2X, base %d\n",  temp, base));
            outb(VGA_SEQ_DATA, temp);
    }

    outw(VGA_SEQ_INDEX, (SR5State << 8) | 0x05); /* Relock Registers */

  } /* if not VESA */

}

/*
 * This is called when VT switching back to the X server.  Its job is
 * to reinitialise the video mode.
 *
 * We may wish to unmap video/MMIO memory too.
 * (TW: This might be dangerous with TQ)
 */

/* Mandatory */
static Bool
SISEnterVT(int scrnIndex, int flags)
{
    ScrnInfoPtr pScrn = xf86Screens[scrnIndex];
#ifdef XF86DRI
    SISPtr pSiS = SISPTR(pScrn);
#endif

    if (!SISModeInit(pScrn, pScrn->currentMode))
	return FALSE;

    SISAdjustFrame(scrnIndex, pScrn->frameX0, pScrn->frameY0, 0);

#ifdef XF86DRI	/* TW: this is to be done AFTER switching the mode */
    if (pSiS->directRenderingEnabled)
        DRIUnlock(screenInfo.screens[scrnIndex]);
#endif

    return TRUE;
}

/*
 * This is called when VT switching away from the X server.  Its job is
 * to restore the previous (text) mode.
 *
 * We may wish to remap video/MMIO memory too.
 */

/* Mandatory */
static void
SISLeaveVT(int scrnIndex, int flags)
{
    ScrnInfoPtr pScrn = xf86Screens[scrnIndex];
    vgaHWPtr hwp = VGAHWPTR(pScrn);
    SISPtr pSiS;

#ifdef XF86DRI
    ScreenPtr pScreen;
#endif

    pSiS = SISPTR(pScrn);

#ifdef XF86DRI		/* TW: to be done before mode change */
    if (pSiS->directRenderingEnabled) {
        pScreen = screenInfo.screens[scrnIndex];
        DRILock(pScreen, 0);
    }
#endif

    SISBridgeRestore(pScrn);

    if (pSiS->UseVESA) {
        /* TW: This is a q&d work-around for a BIOS bug. In case we disabled CRT2,
    	 *     VBESaveRestore() does not re-enable CRT1. So we set any mode now,
	 *     because VBESetVBEMode correctly restores CRT1. Afterwards, we
	 *     can call VBESaveRestore to restore original mode.
	 */
        if ( (pSiS->VBFlags & VB_VIDEOBRIDGE) && (!(pSiS->VBFlags & DISPTYPE_DISP2)) )
	           VBESetVBEMode(pSiS->pVbe, (SISVesaModeList->n) | 0xc000, NULL);
        SISVESARestore(pScrn);
    }

    SISRestore(pScrn);

    vgaHWLock(hwp);
}


/*
 * This is called at the end of each server generation.  It restores the
 * original (text) mode.  It should really also unmap the video memory too.
 */

/* Mandatory */
static Bool
SISCloseScreen(int scrnIndex, ScreenPtr pScreen)
{
    ScrnInfoPtr pScrn = xf86Screens[scrnIndex];
    vgaHWPtr hwp = VGAHWPTR(pScrn);
    SISPtr pSiS = SISPTR(pScrn);
    xf86CursorInfoPtr   pCursorInfo = pSiS->CursorInfoPtr;

#ifdef XF86DRI
    if (pSiS->directRenderingEnabled) {
        SISDRICloseScreen(pScreen);
        pSiS->directRenderingEnabled=FALSE;
    }
#endif

    if (pScrn->vtSema) {
        if (pCursorInfo)
           pCursorInfo->HideCursor(pScrn);
        SISBridgeRestore(pScrn);
	if (pSiS->UseVESA) {
          /* TW: This is a q&d work-around for a BIOS bug. In case we disabled CRT2,
    	   *     VBESaveRestore() does not re-enable CRT1. So we set any mode now,
	   *     because VBESetVBEMode correctly restores CRT1. Afterwards, we
	   *     can call VBESaveRestore to restore original mode.
	   */
           if ( (pSiS->VBFlags & VB_VIDEOBRIDGE) && (!(pSiS->VBFlags & DISPTYPE_DISP2)))
	           VBESetVBEMode(pSiS->pVbe, (SISVesaModeList->n) | 0xc000, NULL);
	   SISVESARestore(pScrn);
	}
	SISRestore(pScrn);
        vgaHWLock(hwp);
        SISUnmapMem(pScrn);
    }
    if(pSiS->AccelInfoPtr)
        XAADestroyInfoRec(pSiS->AccelInfoPtr);
    if(pCursorInfo)
        xf86DestroyCursorInfoRec(pCursorInfo);
    pScrn->vtSema = FALSE;
    
    pScreen->CloseScreen = pSiS->CloseScreen;
    return (*pScreen->CloseScreen)(scrnIndex, pScreen);
}


/* Free up any per-generation data structures */

/* Optional */
static void
SISFreeScreen(int scrnIndex, int flags)
{
    if (xf86LoaderCheckSymbol("vgaHWFreeHWRec"))
        vgaHWFreeHWRec(xf86Screens[scrnIndex]);
    SISFreeRec(xf86Screens[scrnIndex]);
}


/* Checks if a mode is suitable for the selected chipset. */

/* Optional */
static int
SISValidMode(int scrnIndex, DisplayModePtr mode, Bool verbose, int flags)
{
    ScrnInfoPtr pScrn = xf86Screens[scrnIndex];
    SISPtr pSiS = SISPTR(pScrn);

    if (pSiS->UseVESA) {
	if (CalcVESAModeIndex(pScrn, mode))
	    return (MODE_OK);
	else 
	    return (MODE_BAD);
    }
    if ((pSiS->Chipset == PCI_CHIP_SIS300) ||
            (pSiS->Chipset == PCI_CHIP_SIS630) ||
            (pSiS->Chipset == PCI_CHIP_SIS540)) {
	if (SiSCalcModeIndex(pScrn, mode) < 0x14)
	    return (MODE_BAD);
    }
    
    return(MODE_OK);
}

/* Do screen blanking */

/* Mandatory */
static Bool
SISSaveScreen(ScreenPtr pScreen, int mode)
{
    ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
    if ((pScrn != NULL) && pScrn->vtSema) {

    	SISPtr pSiS = SISPTR(pScrn);
	/* enable access to extended sequencer registers */
	outw(VGA_SEQ_INDEX, 0x8605);
	outb(VGA_SEQ_INDEX, 0x11);
	/* if not blanked obtain state of LCD blank flags set by BIOS */
	if (!pSiS->Blank) {
	    unsigned char val;
	    val = inb(VGA_SEQ_DATA);
	    pSiS->LCDon = val;
	}
	if (!xf86IsUnblank(mode)) {
	    pSiS->Blank = TRUE;
	    outb(VGA_SEQ_DATA, (pSiS->LCDon | 0x8));
	} else {
	    pSiS->Blank = FALSE;
	    /* don't just unblanking; use LCD state set by BIOS */
	    outb(VGA_SEQ_DATA, (pSiS->LCDon));
	}
    }

    return vgaHWSaveScreen(pScreen, mode);
}

#ifdef  DEBUG
/* local used for debug */
static void
SiSDumpModeInfo(ScrnInfoPtr pScrn, DisplayModePtr mode)
{
    xf86DrvMsg(pScrn->scrnIndex,X_INFO, "Clock : %x\n", mode->Clock);
    xf86DrvMsg(pScrn->scrnIndex,X_INFO, "Hz Display : %x\n", mode->CrtcHDisplay);
    xf86DrvMsg(pScrn->scrnIndex,X_INFO, "Hz Blank Start : %x\n", mode->CrtcHBlankStart);
    xf86DrvMsg(pScrn->scrnIndex,X_INFO, "Hz Sync Start : %x\n", mode->CrtcHSyncStart);
    xf86DrvMsg(pScrn->scrnIndex,X_INFO, "Hz Sync End : %x\n", mode->CrtcHSyncEnd);
    xf86DrvMsg(pScrn->scrnIndex,X_INFO, "Hz Blank End : %x\n", mode->CrtcHBlankEnd);
    xf86DrvMsg(pScrn->scrnIndex,X_INFO, "Hz Total : %x\n", mode->CrtcHTotal);
    xf86DrvMsg(pScrn->scrnIndex,X_INFO, "Hz Skew : %x\n", mode->CrtcHSkew);
    xf86DrvMsg(pScrn->scrnIndex,X_INFO, "Hz HAdjusted : %x\n", mode->CrtcHAdjusted);
    xf86DrvMsg(pScrn->scrnIndex,X_INFO, "Vt Display : %x\n", mode->CrtcVDisplay);
    xf86DrvMsg(pScrn->scrnIndex,X_INFO, "Vt Blank Start : %x\n", mode->CrtcVBlankStart);
    xf86DrvMsg(pScrn->scrnIndex,X_INFO, "Vt Sync Start : %x\n", mode->CrtcVSyncStart);
    xf86DrvMsg(pScrn->scrnIndex,X_INFO, "Vt Sync End : %x\n", mode->CrtcVSyncEnd);
    xf86DrvMsg(pScrn->scrnIndex,X_INFO, "Vt Blank End : %x\n", mode->CrtcVBlankEnd);
    xf86DrvMsg(pScrn->scrnIndex,X_INFO, "Vt Total : %x\n", mode->CrtcVTotal);
    xf86DrvMsg(pScrn->scrnIndex,X_INFO, "Vt VAdjusted : %x\n", mode->CrtcVAdjusted);

/*
    xf86DrvMsg(pScrn->scrnIndex,X_INFO, "Hz Display : %x\n", mode->HDisplay);
    xf86DrvMsg(pScrn->scrnIndex,X_INFO, "Hz Sync Start : %x\n", mode->HSyncStart);
    xf86DrvMsg(pScrn->scrnIndex,X_INFO, "Hz Sync End : %x\n", mode->HSyncEnd);
    xf86DrvMsg(pScrn->scrnIndex,X_INFO, "Hz Total : %x\n", mode->HTotal);
    xf86DrvMsg(pScrn->scrnIndex,X_INFO, "Hz Skew : %x\n", mode->HSkew);
    xf86DrvMsg(pScrn->scrnIndex,X_INFO, "Vt Display : %x\n", mode->VDisplay);
    xf86DrvMsg(pScrn->scrnIndex,X_INFO, "Vt Sync Start : %x\n", mode->VSyncStart);
    xf86DrvMsg(pScrn->scrnIndex,X_INFO, "Vt Sync End : %x\n", mode->VSyncEnd);
    xf86DrvMsg(pScrn->scrnIndex,X_INFO, "Vt Total : %x\n", mode->VTotal);
    xf86DrvMsg(pScrn->scrnIndex,X_INFO, "Vt Scan : %x\n", mode->VScan);
*/
}
#endif

/* local used for debug */
static void
SISModifyModeInfo(DisplayModePtr mode)
{
/*
    mode->Clock = 31500;
    mode->CrtcHTotal    = 832;
    mode->CrtcHDisplay  = 640;
    mode->CrtcHBlankStart   = 648;
    mode->CrtcHSyncStart    = 664;
    mode->CrtcHSyncEnd  = 704;
    mode->CrtcHBlankEnd = 824;

    mode->CrtcVTotal    = 520;
    mode->CrtcVDisplay  = 480;
    mode->CrtcVBlankStart   = 488;
    mode->CrtcVSyncStart    = 489;
    mode->CrtcVSyncEnd  = 492;
    mode->CrtcVBlankEnd = 512;
*/
    if (mode->CrtcHBlankStart == mode->CrtcHDisplay)
        mode->CrtcHBlankStart++;
    if (mode->CrtcHBlankEnd == mode->CrtcHTotal)
        mode->CrtcHBlankEnd--;
    if (mode->CrtcVBlankStart == mode->CrtcVDisplay)
        mode->CrtcVBlankStart++;
    if (mode->CrtcVBlankEnd == mode->CrtcVTotal)
        mode->CrtcVBlankEnd--;
}

void SiSPreSetMode(ScrnInfoPtr pScrn, int LockAfterwards)
{
    SISPtr pSiS = SISPTR(pScrn);
    unsigned char  usScratchCR30, usScratchCR31;
    unsigned char  usScratchCR32, usScratchCR33;
    unsigned short SR26, SR27;
    unsigned char SR5State;
    unsigned long  temp;
    int vbflag;

    outb(VGA_SEQ_INDEX, 0x05);           /* Unlock Registers */
    SR5State = inb(VGA_SEQ_DATA);
    outw(VGA_SEQ_INDEX, 0x8605);

    usScratchCR30 = usScratchCR31 = usScratchCR33 = 0;
    outb(SISCR, 0x31);
    usScratchCR31 = inb(SISCR+1);
    outb(SISCR, 0x33);		/* TW: CRT1 refresh rate index */
    usScratchCR33 = inb(SISCR+1);
    outb(SISCR, 0x32);		/* TW: Bridge connection info */
    usScratchCR32 = inb(SISCR+1);
    outb(SISCR, 0x30);
    usScratchCR30 = inb(SISCR+1);
    
    xf86DrvMsg(pScrn->scrnIndex, X_PROBED,
	       "Bridge registers were 30=0x%02x, 31=0x%02x, 32=0x%02x, 33=0x%02x (VBFlags = 0x%x)\n",
	       usScratchCR30, usScratchCR31, usScratchCR32, usScratchCR33, pSiS->VBFlags);
    usScratchCR30 = 0;
    usScratchCR31 &= ~0x60;  /* TW: clear VB_Drivermode & VB_OutputDisable */
    
    vbflag=pSiS->VBFlags;
    switch (vbflag & (CRT2_TV|CRT2_LCD|CRT2_VGA))
    { case CRT2_TV:
	if (vbflag & TV_HIVISION)
	    usScratchCR30 |= 0x80;
	else if (vbflag & TV_SVIDEO)
	    usScratchCR30 |= 0x08;
	else if (vbflag & TV_AVIDEO)
	    usScratchCR30 |= 0x04;
	else if (vbflag & TV_SCART)
	    usScratchCR30 |= 0x10;
	if (vbflag & TV_PAL)
	    usScratchCR31 |= 0x01;
	else
	    usScratchCR31 &= ~0x01;
#if 0	/* TW: Old code */
	if (vbflag & TV_HIVISION) usScratchCR30 |= 0x80;
	else if (vbflag & TV_PAL) usScratchCR31 |= 0x01;
	
	if (vbflag & TV_AVIDEO) usScratchCR30 |= 0x04;
	else if (vbflag & TV_SVIDEO) usScratchCR30 |= 0x08;
	else if (vbflag & TV_SCART) usScratchCR30 |= 0x10;
#endif
	usScratchCR30 |= 0x01;
	usScratchCR31 &= ~0x04;
	break;
    case CRT2_LCD:
	usScratchCR30 |= 0x21;
	usScratchCR31 |= 0x02;
	break;
    case CRT2_VGA:
	usScratchCR30 |= 0x41;
	break;
    default:  /* TW: When CRT2Type is NONE, we can calculate a proper rate for CRT1 */
	usScratchCR30 |= 0x00;
	usScratchCR31 |= 0x20; /* TW: VB_OUTPUT_DISABLE */
	if (pSiS->UseVESA)
	    usScratchCR33 = SISSearchCRT1Rate(pScrn->currentMode);
    }
    /*
     * TW: for VESA: no DRIVERMODE, otherwise
     * -) CRT2 will not be initialized correctly when using mode
     *    where LCD has to scale
     * -) CRT1 will have too low rate
     */
    if (pSiS->UseVESA) usScratchCR31 &= ~0x40;
    else usScratchCR31 |= 0x40;   /* 0x40=drivermode */
  
   SiSSetReg1(SISCR, 0x30, usScratchCR30);
   SiSSetReg1(SISCR, 0x31, usScratchCR31);
   SiSSetReg1(SISCR, 0x33, usScratchCR33);
  
    xf86DrvMsg(pScrn->scrnIndex, X_PROBED,
	       "Bridge registers set to 30=0x%02x, 31=0x%02x, 33=0x%02x\n",
	       usScratchCR30, usScratchCR31, usScratchCR33);
  
    /* Set Turbo Queue as 512K */
    /* TW: This is done here _and_ in SiS300Init() because SiS300Init() only
     * sets up structure but structure is not written to hardware (using
     * SiS300Restore) on SiS630, 300, 540 (unless VESA is used).
     */
    if (!pSiS->NoAccel) {
        if (pSiS->TurboQueue) {
	    temp = (pScrn->videoRam/64) - 8;
	    SR26 = temp & 0xFF;
	    SR27 = ((temp >> 8) & 3) | 0xF0;
	    SiSSetReg1(SISSR, 0x26, SR26);
	    SiSSetReg1(SISSR, 0x27, SR27);
	}
    }
    
    if (LockAfterwards)
	outw(VGA_SEQ_INDEX, (SR5State << 8) | 0x05); /* Relock Registers */
}

/* TW: This doesn't work yet. Switching CRT1 off this way causes a white screen on CRT2 */
void SiSPostSetMode(ScrnInfoPtr pScrn, SISRegPtr sisReg, int LockAfterwards)
{
#if 0
    SISPtr pSiS = SISPTR(pScrn);
    unsigned char usScratchCR17;
    unsigned char SR5State;

    outb(VGA_SEQ_INDEX, 0x05);           /* Unlock Registers */
    SR5State = inb(VGA_SEQ_DATA);
    outw(VGA_SEQ_INDEX, 0x8605);

    if ((pSiS->VBFlags & (VB_LVDS | VB_CHRONTEL)) &&
	pScrn->bitsPerPixel == 8)
        pSiS->CRT1off = 0;

    xf86DrvMsg(0, X_PROBED, "CRT1off %d\n", pSiS->CRT1off);

    outb(SISCR, 0x17);
    usScratchCR17 = inb(SISCR+1);

    xf86DrvMsg(0, X_PROBED, "CR17 was 0x%2x\n", usScratchCR17);
    if (pSiS->CRT1off)
	usScratchCR17 &= ~0x80;  /* sisReg->sisRegs3D4[0x17] &= ~0x80; */
    else
        usScratchCR17 |= 0x80;   /* sisReg->sisRegs3D4[0x17] |= 0x80; */

    xf86DrvMsg(0, X_PROBED, "CR17 set to 0x%2x\n", usScratchCR17);
    /*SiSSetReg1(SISCR, 0x17, usScratchCR17); */

    if (LockAfterwards)
        outw(VGA_SEQ_INDEX, (SR5State << 8) | 0x05); /* Relock Registers */
#endif
}

static void
SiSBuildVesaModeList(ScrnInfoPtr pScrn, vbeInfoPtr pVbe, VbeInfoBlock *vbe)
{
    int i = 0;
    while (vbe->VideoModePtr[i] != 0xffff) {
	sisModeInfoPtr m;
	VbeModeInfoBlock *mode;
	int id = vbe->VideoModePtr[i++];
	int bpp;

	if ((mode = VBEGetModeInfo(pVbe, id)) == NULL)
	    continue;

	bpp = mode->BitsPerPixel;
	/* TW: Doesn't work on SiS630 VBE 3.0: */
	/* mode->GreenMaskSize + mode->BlueMaskSize
	    + mode->RedMaskSize;  */

	m = xnfcalloc(sizeof(sisModeInfoRec),1);
	m->width = mode->XResolution;
	m->height = mode->YResolution;
	m->bpp = bpp;
	m->n = id;
	m->next = SISVesaModeList;

	xf86DrvMsg(pScrn->scrnIndex, X_PROBED,
	      "BIOS reported VESA mode 0x%x: x:%i y:%i bpp:%i\n",
	       m->n, m->width, m->height, m->bpp);

	SISVesaModeList = m;

	VBEFreeModeInfo(mode);
    }
}

static UShort CalcVESAModeIndex(ScrnInfoPtr pScrn, DisplayModePtr mode)
{
    sisModeInfoPtr m = SISVesaModeList;
    UShort i = (pScrn->bitsPerPixel+7)/8 - 1;	/* bitsperpixel was depth */
    UShort ModeIndex = 0;
    
    while (m) {
	if (pScrn->bitsPerPixel == m->bpp 
	    && mode->HDisplay == m->width 
	    && mode->VDisplay == m->height)
	    return m->n;
	m = m->next;
    }

    xf86DrvMsg(pScrn->scrnIndex, X_PROBED,
             "No valid BIOS VESA mode found for %dx%dx%d; searching built-in table.\n",
             mode->HDisplay, mode->VDisplay, pScrn->bitsPerPixel);

    switch(mode->HDisplay)
   {
     case 640:
          ModeIndex = VESAModeIndex_640x480[i];
          break;
     case 720:
          if(mode->VDisplay == 480)
            ModeIndex = VESAModeIndex_720x480[i];
          else
            ModeIndex = VESAModeIndex_720x576[i];
          break;
     case 800:
          ModeIndex = VESAModeIndex_800x600[i];
          break;
     case 1024:
          ModeIndex = VESAModeIndex_1024x768[i];
          break;
     case 1280:
          ModeIndex = VESAModeIndex_1280x1024[i];
          break;
     case 1600:
          ModeIndex = VESAModeIndex_1600x1200[i];
          break;
     case 1920:
          ModeIndex = VESAModeIndex_1920x1440[i];
          break;
   }

   if (!ModeIndex) xf86DrvMsg(pScrn->scrnIndex, X_PROBED,
        "No valid mode found for %dx%dx%d in built-in table either.",
	mode->HDisplay, mode->VDisplay, pScrn->bitsPerPixel);

   return(ModeIndex);
}

/* TW: Calculate CR33 (rate index) for CRT1 if CRT2 is disabled.
       Calculation is done using currentmode structure, therefore
       it is recommended to set VertRefresh and HorizSync to correct
       values in Config file.
 */
unsigned char SISSearchCRT1Rate(DisplayModePtr mode)
{
   float hsync, refresh = 0;
   int i = 0;
   unsigned short xres=mode->HDisplay;
   unsigned short yres=mode->VDisplay;
   unsigned char index;

   if (mode->HSync > 0.0)
       	hsync = mode->HSync;
   else if (mode->HTotal > 0)
       	hsync = (float)mode->Clock / (float)mode->HTotal;
   else
       	hsync = 0.0;
   if (mode->VTotal > 0)
       	refresh = hsync * 1000.0 / mode->VTotal;
   if (mode->Flags & V_INTERLACE) {
       	refresh *= 2.0;
   }
   if (mode->Flags & V_DBLSCAN) {
       	refresh /= 2.0;
   }
   if (mode->VScan > 1) {
        refresh /= mode->VScan;
   }
   if (mode->VRefresh > 0.0)
    	refresh = mode->VRefresh;
   if (hsync == 0 || refresh == 0)
        return 0x02;  /* TW: Default mode index */
   else {
     index = 0;
     while ((sisx_vrate[i].idx != 0) && (sisx_vrate[i].xres <= xres)) {
	if ((sisx_vrate[i].xres == xres)
		    && (sisx_vrate[i].yres == yres)) {
	    if (sisx_vrate[i].refresh == refresh) {
		index = sisx_vrate[i].idx;
		break;
	    } else if (sisx_vrate[i].refresh > refresh) {
		if ((sisx_vrate[i].refresh - refresh) <= 2) {
		    index = sisx_vrate[i].idx;
		} else if (((refresh - sisx_vrate[i - 1].refresh) <=  2)
				    	&& (sisx_vrate[i].idx != 1)) {
			index = sisx_vrate[i - 1].idx;
		}
		break;
	    }
	}
	i++;
     }
     if (index > 0)
	return index;
     else
	return 0x02; /* TW: Default Rate index */
   }
}


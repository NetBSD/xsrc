/* $XFree86: xc/programs/Xserver/hw/xfree86/drivers/savage/savage_driver.c,v 1.6 2000/12/12 16:50:48 dawes Exp $ */
/*
 * vim: sw=4 ts=8 ai ic:
 *
 *	XFree86 4.0 S3 Savage driver
 *
 *	Tim Roberts <timr@probo.com>
 *	Ani Joshi <ajoshi@unixbox.com>
 *
 *	TODO:  add credits for the 3.3.x authors...
 *
 */


#include "xf86RAC.h"
#include "shadowfb.h"

#ifdef DPMSExtension
#include "globals.h"
#define DPMS_SERVER
#include "extensions/dpms.h"
#endif /* DPMSExtension */

#include "savage_driver.h"
#include "savage_bci.h"




/*
 * prototypes
 */
static void SavageEnableMMIO(ScrnInfoPtr pScrn);
static void SavageDisableMMIO(ScrnInfoPtr pScrn);

static OptionInfoPtr SavageAvailableOptions(int chipid, int busid);
static void SavageIdentify(int flags);
static Bool SavageProbe(DriverPtr drv, int flags);
static Bool SavagePreInit(ScrnInfoPtr pScrn, int flags);

static Bool SavageEnterVT(int scrnIndex, int flags);
static void SavageLeaveVT(int scrnIndex, int flags);
static void SavageSave(ScrnInfoPtr pScrn);
static void SavageWriteMode(ScrnInfoPtr pScrn, vgaRegPtr, SavageRegPtr);

static Bool SavageScreenInit(int scrnIndex, ScreenPtr pScreen, int argc,
			     char **argv);
static int SavageInternalScreenInit(int scrnIndex, ScreenPtr pScreen);
static ModeStatus SavageValidMode(int index, DisplayModePtr mode,
				  Bool verbose, int flags);

static Bool SavageMapMMIO(ScrnInfoPtr pScrn);
static Bool SavageMapFB(ScrnInfoPtr pScrn);
static void SavageUnmapMem(ScrnInfoPtr pScrn);
static Bool SavageModeInit(ScrnInfoPtr pScrn, DisplayModePtr mode);
static Bool SavageCloseScreen(int scrnIndex, ScreenPtr pScreen);
static Bool SavageSaveScreen(ScreenPtr pScreen, int mode);
static void SavageLoadPalette(ScrnInfoPtr pScrn, int numColors,
			      int *indicies, LOCO *colors,
			      VisualPtr pVisual);
static void SavageCalcClock(long freq, int min_m, int min_n1, int max_n1,
			   int min_n2, int max_n2, long freq_min,
			   long freq_max, unsigned int *mdiv,
			   unsigned int *ndiv, unsigned int *r);
void SavageGEReset(ScrnInfoPtr pScrn, int from_timeout, int line, char *file);
void SavagePrintRegs(ScrnInfoPtr pScrn);
#ifdef DPMSExtension
static void SavageDPMS(ScrnInfoPtr pScrn, int mode, int flags);
#endif

static int pix24bpp = 0;

#define iabs(a)	((int)(a)>0?(a):(-(a)))

#define DRIVER_NAME	"savage"
#define DRIVER_VERSION	"1.1.0"
#define VERSION_MAJOR	1
#define VERSION_MINOR	1
#define PATCHLEVEL	0
#define SAVAGE_VERSION	((VERSION_MAJOR << 24) | \
			 (VERSION_MINOR << 16) | \
			 PATCHLEVEL)


DriverRec SAVAGE =
{
    SAVAGE_VERSION,
    DRIVER_NAME,
    SavageIdentify,
    SavageProbe,
    SavageAvailableOptions,
    NULL,
    0
};


/* Supported chipsets */

static SymTabRec SavageChips[] = {
    { PCI_CHIP_SAVAGE4,		"Savage4" },
    { PCI_CHIP_SAVAGE3D,	"Savage3D" },
    { PCI_CHIP_SAVAGE3D_MV,	"Savage3D-MV" },
    { PCI_CHIP_SAVAGE2000,	"Savage2000" },
    { PCI_CHIP_SAVAGE_MX_MV,	"Savage/MX-MV" },
    { PCI_CHIP_SAVAGE_MX,	"Savage/MX" },
    { PCI_CHIP_SAVAGE_IX_MV,	"Savage/IX-MV" },
    { PCI_CHIP_SAVAGE_IX,	"Savage/IX" },
    { PCI_CHIP_PROSAVAGE_PM,	"ProSavage PM133" },
    { PCI_CHIP_PROSAVAGE_KM,	"ProSavage KM133" },
    { -1,			NULL }
};

static SymTabRec SavageChipsets[] = {
    { S3_SAVAGE3D,	"Savage3D" },
    { S3_SAVAGE4,	"Savage4" },
    { S3_SAVAGE2000,	"Savage2000" },
    { S3_SAVAGE_MX,	"Savage/MX or /IX" },
    { S3_PROSAVAGE,	"ProSavage PM133" },
    { -1,		NULL }
};

/* This table maps a PCI device ID to a chipset family identifier. */

static PciChipsets SavagePciChipsets[] = {
    { S3_SAVAGE3D,	PCI_CHIP_SAVAGE3D,	RES_SHARED_VGA },
    { S3_SAVAGE3D,	PCI_CHIP_SAVAGE3D_MV, 	RES_SHARED_VGA },
    { S3_SAVAGE4,	PCI_CHIP_SAVAGE4,	RES_SHARED_VGA },
    { S3_SAVAGE2000,	PCI_CHIP_SAVAGE2000,	RES_SHARED_VGA },
    { S3_SAVAGE_MX,	PCI_CHIP_SAVAGE_MX_MV,	RES_SHARED_VGA },
    { S3_SAVAGE_MX,	PCI_CHIP_SAVAGE_MX,	RES_SHARED_VGA },
    { S3_SAVAGE_MX,	PCI_CHIP_SAVAGE_IX_MV,	RES_SHARED_VGA },
    { S3_SAVAGE_MX,	PCI_CHIP_SAVAGE_IX,	RES_SHARED_VGA },
    { S3_PROSAVAGE,	PCI_CHIP_PROSAVAGE_PM,	RES_SHARED_VGA },
    { S3_PROSAVAGE,	PCI_CHIP_PROSAVAGE_KM,	RES_SHARED_VGA },
    { -1,		-1,			RES_UNDEFINED }
};

typedef enum {
    OPTION_SLOW_EDODRAM,
    OPTION_SLOW_DRAM,
    OPTION_FAST_DRAM,
    OPTION_FPM_VRAM,
    OPTION_PCI_BURST,
    OPTION_FIFO_CONSERV,
    OPTION_FIFO_MODERATE,
    OPTION_FIFO_AGGRESSIVE,
    OPTION_PCI_RETRY,
    OPTION_NOACCEL,
    OPTION_EARLY_RAS_PRECHARGE,
    OPTION_LATE_RAS_PRECHARGE,
    OPTION_LCD_CENTER,
    OPTION_LCDCLOCK,
    OPTION_MCLK,
    OPTION_REFCLK,
    OPTION_SHOWCACHE,
    OPTION_SWCURSOR,
    OPTION_HWCURSOR,
    OPTION_SHADOW_FB,
    OPTION_ROTATE,
    OPTION_USEBIOS
} SavageOpts;


static OptionInfoRec SavageOptions[] =
{
    { OPTION_SLOW_EDODRAM, "slow_edodram", OPTV_BOOLEAN, {0}, FALSE },
    { OPTION_SLOW_DRAM,	"slow_dram",	OPTV_BOOLEAN, {0}, FALSE },
    { OPTION_FAST_DRAM,	"fast_dram",	OPTV_BOOLEAN, {0}, FALSE },
    { OPTION_FPM_VRAM,	"fpm_vram",	OPTV_BOOLEAN, {0}, FALSE },
    { OPTION_NOACCEL,	"NoAccel",	OPTV_BOOLEAN, {0}, FALSE  },
    { OPTION_HWCURSOR,	"HWCursor",	OPTV_BOOLEAN, {0}, FALSE },
    { OPTION_SWCURSOR,	"SWCursor",	OPTV_BOOLEAN, {0}, FALSE },
    { OPTION_SHADOW_FB,	"ShadowFB",	OPTV_BOOLEAN, {0}, FALSE },
    { OPTION_ROTATE,	"Rotate",	OPTV_ANYSTR, {0}, FALSE },
    { OPTION_USEBIOS,	"UseBIOS",	OPTV_BOOLEAN, {0}, FALSE },
    /* finish later... */
    { -1,		NULL,		OPTV_NONE,    {0}, FALSE }
};


static const char *vgaHWSymbols[] = {
    "vgaHWGetHWRec",
    "vgaHWSetMmioFuncs",
    "vgaHWGetIOBase",
    "vgaHWSave",
    "vgaHWProtect",
    "vgaHWRestore",
    "vgaHWMapMem",
    "vgaHWUnmapMem",
    "vgaHWInit",
    "vgaHWSaveScreen",
    "vgaHWLock",
#if 0
    "vgaHWUnlock",
    "vgaHWFreeHWRec",
#endif
    NULL
};

static const char *ramdacSymbols[] = {
    "xf86InitCursor",
    "xf86CreateCursorInfoRec",
    "xf86DestroyCursorInfoRec",
    NULL
};

static const char *vbeSymbols[] = {
    "VBEInit",
    "vbeDoEDID",
    "vbeFree",
    NULL
};

static const char *ddcSymbols[] = {
    "xf86InterpretEDID",
    NULL
};

static const char *xaaSymbols[] = {
    "XAACopyROP",
    "XAACopyROP_PM",
    "XAACreateInfoRec",
    "XAADestroyInfoRec",
    "XAAHelpPatternROP",
    "XAAHelpSolidROP",
    "XAAInit",
    "XAAScreenIndex",
    NULL
};

static const char *shadowSymbols[] = {
    "ShadowFBInit",
    NULL
};

static const char *int10Symbols[] = {
    "xf86ExecX86int10",
    "xf86FreeInt10",
    "xf86InitInt10",
    "xf86Int10AllocPages",
    "xf86Int10FreePages",
    NULL
};

static const char *cfbSymbols[] = {
    "cfbScreenInit",
    "cfb16ScreenInit",
    "cfb24ScreenInit",
    "cfb24_32ScreenInit",
    "cfb32ScreenInit",
    "cfb16BresS",
    "cfb24BresS",
    NULL
};

#ifdef XFree86LOADER

static MODULESETUPPROTO(SavageSetup);

static XF86ModuleVersionInfo SavageVersRec = {
    "savage",
    MODULEVENDORSTRING,
    MODINFOSTRING1,
    MODINFOSTRING2,
    XF86_VERSION_CURRENT,
    VERSION_MAJOR, VERSION_MINOR, PATCHLEVEL,
    ABI_CLASS_VIDEODRV,
    ABI_VIDEODRV_VERSION,
    MOD_CLASS_VIDEODRV,
    {0, 0, 0, 0}
};

XF86ModuleData savageModuleData = { &SavageVersRec, SavageSetup, NULL };

static pointer SavageSetup(pointer module, pointer opts, int *errmaj,
			   int *errmin)
{
    static Bool setupDone = FALSE;

    if (!setupDone) {
	setupDone = TRUE;
	xf86AddDriver(&SAVAGE, module, 0);
	LoaderRefSymLists(vgaHWSymbols, cfbSymbols, ramdacSymbols, 
			  xaaSymbols, shadowSymbols, vbeSymbols,
			  ddcSymbols, NULL);
	return (pointer) 1;
    } else {
	if (errmaj)
	    *errmaj = LDR_ONCEONLY;
	return NULL;
    }
}

#endif /* XFree86LOADER */


/*
 * I'd rather have these wait macros be inline, but S3 has made it 
 * darned near impossible.  The bit fields are in a different place in
 * all three families, the status register has a different address in the
 * three families, and even the idle vs busy sense flipped in the Sav2K.
 */

static void
ResetBCI2K( SavagePtr psav )
{
    CARD32 cob = INREG( 0x48c18 );
    /* if BCI is enabled and BCI is busy... */

    if( 
	(cob & 0x00000008) &&
	! (ALT_STATUS_WORD0 & 0x00200000)
    )
    {
	ErrorF( "Resetting BCI, stat = %08x...\n", ALT_STATUS_WORD0);
	/* Turn off BCI */
	OUTREG( 0x48c18, cob & ~8 );
	usleep(10000);
	/* Turn it back on */
	OUTREG( 0x48c18, cob );
	usleep(10000);
    }
}

/* Wait until "v" queue entries are free */

static int
WaitQueue3D( SavagePtr psav, int v )
{
    int loop = 0;
    int slots = MAXFIFO - v;

    mem_barrier();
    loop &= STATUS_WORD0;
    while( ((STATUS_WORD0 & 0x0000ffff) > slots) && (loop++ < MAXLOOP))
	;
    return loop >= MAXLOOP;
}

static int
WaitQueue4( SavagePtr psav, int v )
{
    int loop = 0;
    int slots = MAXFIFO - v;

    if( !psav->NoPCIRetry )
	return 0;
    mem_barrier();
    while( ((ALT_STATUS_WORD0 & 0x001fffff) > slots) && (loop++ < MAXLOOP))
	;
    return loop >= MAXLOOP;
}

static int
WaitQueue2K( SavagePtr psav, int v )
{
    int loop = 0;
    int slots = MAXFIFO - v;

    if( !psav->NoPCIRetry )
	return 0;
    mem_barrier();
    while( ((ALT_STATUS_WORD0 & 0x000fffff) > slots) && (loop++ < MAXLOOP))
	;
    if( loop >= MAXLOOP )
	ResetBCI2K(psav);
    return loop >= MAXLOOP;
}

/* Wait until GP is idle and queue is empty */

static int
WaitIdleEmpty3D(SavagePtr psav)
{
    int loop = 0;
    mem_barrier();
    loop &= STATUS_WORD0;
    while( ((STATUS_WORD0 & 0x0008ffff) != 0x80000) && (loop++ < MAXLOOP) )
	;
    return loop >= MAXLOOP;
}

static int
WaitIdleEmpty4(SavagePtr psav)
{
    int loop = 0;
    mem_barrier();
    while( ((ALT_STATUS_WORD0 & 0x00a1ffff) != 0x00a00000) && (loop++ < MAXLOOP) )
	;
    return loop >= MAXLOOP;
}

static int
WaitIdleEmpty2K(SavagePtr psav)
{
    int loop = 0;
    mem_barrier();
    /* CAUTION!  How do we insure this read isn't optimized away? */
    /* Is the "volatile" enough to do that? */
    loop &= ALT_STATUS_WORD0;
    while( ((ALT_STATUS_WORD0 & 0x009fffff) != 0) && (loop++ < MAXLOOP) )
	;
    if( loop >= MAXLOOP )
	ResetBCI2K(psav);
    return loop >= MAXLOOP;
}

/* Wait until GP is idle */

static int
WaitIdle3D(SavagePtr psav)
{
    int loop = 0;
    mem_barrier();
    while( (!(STATUS_WORD0 & 0x00080000)) && (loop++ < MAXLOOP) )
	;
    return loop >= MAXLOOP;
}

static int
WaitIdle4(SavagePtr psav)
{
    int loop = 0;
    mem_barrier();
    while( (!(ALT_STATUS_WORD0 & 0x00800000)) && (loop++ < MAXLOOP) )
	;
    return loop >= MAXLOOP;
}

static int
WaitIdle2K(SavagePtr psav)
{
    int loop = 0;
    mem_barrier();
    loop &= ALT_STATUS_WORD0;
    while( (ALT_STATUS_WORD0 & 0x00900000) && (loop++ < MAXLOOP) )
	;
    return loop >= MAXLOOP;
}

/* Wait until Command FIFO is empty */


static int
WaitCommandEmpty3D(SavagePtr psav) {
    int loop = 0;
    mem_barrier();
    while( (STATUS_WORD0 & 0x0000ffff) && (loop++ < MAXLOOP) )
	;
    return loop >= MAXLOOP;
}

static int
WaitCommandEmpty4(SavagePtr psav) {
    int loop = 0;
    mem_barrier();
    while( (ALT_STATUS_WORD0 & 0x0001ffff) && (loop++ < MAXLOOP) )
	;
    return loop >= MAXLOOP;
}

static int
WaitCommandEmpty2K(SavagePtr psav) {
    int loop = 0;
    mem_barrier();
    while( (ALT_STATUS_WORD0 & 0x001fffff) && (loop++ < MAXLOOP) )
	;
    if( loop >= MAXLOOP )
	ResetBCI2K(psav);
    return loop >= MAXLOOP;
}

static Bool SavageGetRec(ScrnInfoPtr pScrn)
{
    if (pScrn->driverPrivate)
	return TRUE;

    pScrn->driverPrivate = xnfcalloc(sizeof(SavageRec), 1);
    return TRUE;
}


static void SavageFreeRec(ScrnInfoPtr pScrn)
{
    if (!pScrn->driverPrivate)
	return;
    xfree(pScrn->driverPrivate);
    pScrn->driverPrivate = NULL;
    SavageUnmapMem(pScrn);
}


static OptionInfoPtr SavageAvailableOptions(int chipid, int busid)
{
    return SavageOptions;
}


static void SavageIdentify(int flags)
{
    xf86PrintChipsets("SAVAGE", 
		      "driver (version " DRIVER_VERSION " for S3 Savage chipsets",
		      SavageChips);
}


static Bool SavageProbe(DriverPtr drv, int flags)
{
    int i;
    GDevPtr *devSections;
    int *usedChips;
    int numDevSections;
    int numUsed;
    Bool foundScreen = FALSE;

    /* sanity checks */
    if ((numDevSections = xf86MatchDevice("savage", &devSections)) <= 0)
	return FALSE;
    if (xf86GetPciVideoInfo() == NULL)
	return FALSE;

    numUsed = xf86MatchPciInstances("SAVAGE", PCI_VENDOR_S3,
				    SavageChipsets, SavagePciChipsets,
				    devSections, numDevSections, drv,
				    &usedChips);
    xfree(devSections);
    if (numUsed <= 0)
	return FALSE;

    if (flags & PROBE_DETECT)
	foundScreen = TRUE;
    else
	for (i=0; i<numUsed; i++) {
	    ScrnInfoPtr pScrn = xf86AllocateScreen(drv, 0);

	    pScrn->driverVersion = (int)DRIVER_VERSION;
	    pScrn->driverName = DRIVER_NAME;
	    pScrn->name = "SAVAGE";
	    pScrn->Probe = SavageProbe;
	    pScrn->PreInit = SavagePreInit;
	    pScrn->ScreenInit = SavageScreenInit;
	    pScrn->SwitchMode = SavageSwitchMode;
	    pScrn->AdjustFrame = SavageAdjustFrame;
	    pScrn->EnterVT = SavageEnterVT;
	    pScrn->LeaveVT = SavageLeaveVT;
	    pScrn->FreeScreen = NULL;
	    pScrn->ValidMode = SavageValidMode;
	    foundScreen = TRUE;
	    xf86ConfigActivePciEntity(pScrn, usedChips[i], SavagePciChipsets,
				     NULL, NULL, NULL, NULL, NULL);
	}

    xfree(usedChips);
    return foundScreen;
}

static int LookupChipID( PciChipsets* pset, int ChipID )
{
    /* Is there a function to do this for me? */
    while( pset->numChipset >= 0 )
    {
        if( pset->PCIid == ChipID )
	    return pset->numChipset;
	pset++;
    }

    return -1;
}

static Bool SavagePreInit(ScrnInfoPtr pScrn, int flags)
{
    EntityInfoPtr pEnt;
    SavagePtr psav;
    MessageType from = X_DEFAULT;
    int i;
    ClockRangePtr clockRanges;
    char *mod = NULL;
    char *s = NULL;
    const char *reqSym = NULL;
    unsigned char config1, m, n, n1, n2, sr8, cr66 = 0, tmp;
    int mclk;
    vgaHWPtr hwp;
    int vgaCRIndex, vgaCRReg, vgaIOBase;

    if (flags & PROBE_DETECT)
	    return FALSE;

    if (!xf86LoadSubModule(pScrn, "vgahw"))
	    return FALSE;

    xf86LoaderReqSymLists(vgaHWSymbols, NULL);
    if (!vgaHWGetHWRec(pScrn))
	    return FALSE;

#if 0
    /* Here we can alter the number of registers saved and restored by the
     * standard vgaHWSave and Restore routines.
     */
    vgaHWSetRegCounts( pScrn, VGA_NUM_CRTC, VGA_NUM_SEQ, VGA_NUM_GFX, VGA_NUM_ATTR );
#endif

    pScrn->monitor = pScrn->confScreen->monitor;

    /*
     * We support depths of 8, 15, 16 and 24.
     * We support bpp of 8, 16, and 32.
     */

    if (!xf86SetDepthBpp(pScrn, 8, 8, 8, Support32bppFb))
	return FALSE;
    else {
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
    }
    xf86PrintDepthBpp(pScrn);

    if (pScrn->depth == 24 && pix24bpp == 0)
	pix24bpp = xf86GetBppFromDepth(pScrn, 24);

    if (pScrn->depth > 8) {
	rgb zeros = {0, 0, 0};

	if (!xf86SetWeight(pScrn, zeros, zeros))
	    return FALSE;
	else {
	    /* TODO check weight returned is supported */
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

    pScrn->progClock = TRUE;

    if (!SavageGetRec(pScrn))
	return FALSE;
    psav = SAVPTR(pScrn);

    xf86CollectOptions(pScrn, NULL);

    if (pScrn->depth == 8)
	pScrn->rgbBits = 6;

    xf86ProcessOptions(pScrn->scrnIndex, pScrn->options, SavageOptions);

    if (xf86ReturnOptValBool(SavageOptions, OPTION_PCI_BURST, FALSE)) {
	psav->pci_burst = TRUE;
	xf86DrvMsg(pScrn->scrnIndex, X_CONFIG, "Option: pci_burst - PCI burst read enabled\n");
    } else
	psav->pci_burst = FALSE;
    psav->NoPCIRetry = 1;		/* default */
    if (xf86ReturnOptValBool(SavageOptions, OPTION_PCI_RETRY, FALSE)) {
	if (xf86ReturnOptValBool(SavageOptions, OPTION_PCI_BURST, FALSE)) {
	    psav->NoPCIRetry = 0;
	    xf86DrvMsg(pScrn->scrnIndex, X_CONFIG, "Option: pci_retry\n");
	} else
	    xf86DrvMsg(pScrn->scrnIndex, X_CONFIG, "\"pci_retry\" option requires \"pci_burst\"\n");
    }
    if (xf86IsOptionSet(SavageOptions, OPTION_FIFO_CONSERV)) {
	psav->fifo_conservative = TRUE;
	xf86DrvMsg(pScrn->scrnIndex, X_CONFIG, "Option: fifo conservative set\n");
    } else
	psav->fifo_conservative = FALSE;
    if (xf86IsOptionSet(SavageOptions, OPTION_FIFO_MODERATE)) {
	psav->fifo_moderate = TRUE;
	xf86DrvMsg(pScrn->scrnIndex, X_CONFIG, "Option: fifo_moderate set\n");
    } else
	psav->fifo_moderate = FALSE;
    if (xf86IsOptionSet(SavageOptions, OPTION_FIFO_AGGRESSIVE)) {
	psav->fifo_aggressive = TRUE;
	xf86DrvMsg(pScrn->scrnIndex, X_CONFIG, "Option: fifo_aggressive set\n");
    } else
	psav->fifo_aggressive = FALSE;
    if (xf86IsOptionSet(SavageOptions, OPTION_SLOW_EDODRAM)) {
	psav->slow_edodram = TRUE;
	xf86DrvMsg(pScrn->scrnIndex, X_CONFIG, "Option: slow_edodram_set\n");
    } else
	psav->slow_edodram = FALSE;
    if (xf86IsOptionSet(SavageOptions, OPTION_SLOW_DRAM)) {
	psav->slow_dram = TRUE;
	xf86DrvMsg(pScrn->scrnIndex, X_CONFIG, "Option: slow_dram set\n");
    } else
	psav->slow_dram = FALSE;
    if (xf86IsOptionSet(SavageOptions, OPTION_FAST_DRAM)) {
	psav->fast_dram = TRUE;
	xf86DrvMsg(pScrn->scrnIndex, X_CONFIG, "Option: fast_dram set\n");
    } else
	psav->fast_dram = FALSE;
    if (xf86IsOptionSet(SavageOptions, OPTION_FPM_VRAM)) {
	psav->fpm_vram = TRUE;
	xf86DrvMsg(pScrn->scrnIndex, X_CONFIG, "Option: fpm_vram  set\n");
    } else
	psav->fpm_vram = FALSE;

    if (xf86IsOptionSet(SavageOptions, OPTION_SHADOW_FB)) {
        psav->shadowFB = TRUE;
	xf86DrvMsg(pScrn->scrnIndex, X_CONFIG, "Option: ShadowFB %s.\n",
		   psav->shadowFB ? "enabled" : "disabled");
    } else
	psav->shadowFB = FALSE;

    if ((s = xf86GetOptValString(SavageOptions, OPTION_ROTATE))) {
	if(!xf86NameCmp(s, "CW")) {
	    /* accel is disabled below for shadowFB */
	    psav->shadowFB = TRUE;
	    psav->rotate = 1;
	    xf86DrvMsg(pScrn->scrnIndex, X_CONFIG, 
		       "Rotating screen clockwise - acceleration disabled\n");
	} else if(!xf86NameCmp(s, "CCW")) {
	    psav->shadowFB = TRUE;
	    psav->rotate = -1;
	    xf86DrvMsg(pScrn->scrnIndex, X_CONFIG,  "Rotating screen"
		       "counter clockwise - acceleration disabled\n");
	} else {
	    xf86DrvMsg(pScrn->scrnIndex, X_CONFIG, "\"%s\" is not a valid"
		       "value for Option \"Rotate\"\n", s);
	    xf86DrvMsg(pScrn->scrnIndex, X_INFO, 
		       "Valid options are \"CW\" or \"CCW\"\n");
	}
    }

    if (xf86ReturnOptValBool(SavageOptions, OPTION_NOACCEL, FALSE)) {
	psav->NoAccel = TRUE;
	xf86DrvMsg(pScrn->scrnIndex, X_CONFIG, "Option: NoAccel - Acceleration Disabled\n");
    } else
	psav->NoAccel = FALSE;

    if (psav->shadowFB && !psav->NoAccel) {
	xf86DrvMsg(pScrn->scrnIndex, X_WARNING,
		   "HW acceleration not supported with \"shadowFB\".\n");
	psav->NoAccel = TRUE;
    }

    if (xf86ReturnOptValBool(SavageOptions, OPTION_EARLY_RAS_PRECHARGE, FALSE)) {
	psav->early_ras_precharge = TRUE;
	xf86DrvMsg(pScrn->scrnIndex, X_CONFIG, "Option: early_ras_precharge set\n");
    } else
	psav->early_ras_precharge = FALSE;

    /*
     * The SWCursor setting takes priority over HWCursor.  The default
     * if neither is specified is HW.
     */

    from = X_DEFAULT;
    psav->hwcursor = TRUE;
    if (xf86GetOptValBool(SavageOptions, OPTION_HWCURSOR, &psav->hwcursor))
	from = X_CONFIG;
    if (xf86ReturnOptValBool(SavageOptions, OPTION_SWCURSOR, FALSE)) {
	psav->hwcursor = FALSE;
	from = X_CONFIG;
    }
    xf86DrvMsg(pScrn->scrnIndex, from, "Using %s cursor\n",
        psav->hwcursor ? "HW" : "SW");

    from = X_DEFAULT;
    psav->UseBIOS = TRUE;
    if (xf86IsOptionSet(SavageOptions, OPTION_USEBIOS) )
    {
	from = X_CONFIG;
	xf86GetOptValBool(SavageOptions, OPTION_USEBIOS, &psav->UseBIOS);
    }
    xf86DrvMsg(pScrn->scrnIndex, from, "%ssing video BIOS to set modes\n",
        psav->UseBIOS ? "U" : "Not u" );


    /* DO OTHERS HERE LATER!!!!!!!!!!!!!! */

    from = X_DEFAULT;



    if (pScrn->numEntities > 1) {
	SavageFreeRec(pScrn);
	return FALSE;
    }

    pEnt = xf86GetEntityInfo(pScrn->entityList[0]);
    if (pEnt->resources) {
	xfree(pEnt);
	SavageFreeRec(pScrn);
	return FALSE;
    }
    psav->EntityIndex = pEnt->index;

    if (psav->UseBIOS) {
        if (xf86LoadSubModule(pScrn, "int10")) {
   	    xf86LoaderReqSymLists(int10Symbols, NULL);
	    psav->pInt10 = xf86InitInt10(pEnt->index);
        }

        if (xf86LoadSubModule(pScrn, "vbe")) {
	    xf86LoaderReqSymLists(vbeSymbols, NULL);
	    psav->pVbe = VBEInit(psav->pInt10, pEnt->index);
        }
    }

    psav->PciInfo = xf86GetPciInfoForEntity(pEnt->index);
    xf86RegisterResources(pEnt->index, NULL, ResNone);
    xf86SetOperatingState(RES_SHARED_VGA, pEnt->index, ResUnusedOpr);
    xf86SetOperatingState(resVgaMemShared, pEnt->index, ResDisableOpr);

    if (pEnt->device->chipset && *pEnt->device->chipset) {
	pScrn->chipset = pEnt->device->chipset;
	psav->ChipId = pEnt->device->chipID;
	psav->Chipset = xf86StringToToken(SavageChipsets, pScrn->chipset);
	from = X_CONFIG;
    } else if (pEnt->device->chipID >= 0) {
	psav->ChipId = pEnt->device->chipID;
	psav->Chipset = LookupChipID(SavagePciChipsets, psav->ChipId);
	pScrn->chipset = (char *)xf86TokenToString(SavageChipsets,
						   psav->Chipset);
	from = X_CONFIG;
	xf86DrvMsg(pScrn->scrnIndex, X_CONFIG, "ChipID override: 0x%04X\n",
		   pEnt->device->chipID);
    } else {
	from = X_PROBED;
	psav->ChipId = psav->PciInfo->chipType;
	psav->Chipset = LookupChipID(SavagePciChipsets, psav->ChipId);
	pScrn->chipset = (char *)xf86TokenToString(SavageChipsets,
						   psav->Chipset);
    }

    if (pEnt->device->chipRev >= 0) {
	psav->ChipRev = pEnt->device->chipRev;
	xf86DrvMsg(pScrn->scrnIndex, X_CONFIG, "ChipRev override: %d\n",
		   psav->ChipRev);
    } else
	psav->ChipRev = psav->PciInfo->chipRev;

    xfree(pEnt);

    /* maybe throw in some more sanity checks here */

    xf86DrvMsg(pScrn->scrnIndex, from, "Chipset: \"%s\"\n", pScrn->chipset);

    psav->PciTag = pciTag(psav->PciInfo->bus, psav->PciInfo->device,
			  psav->PciInfo->func);

    hwp = VGAHWPTR(pScrn);

    if (!SavageMapMMIO(pScrn))
	return FALSE;

    vgaHWGetIOBase(hwp);
    vgaIOBase = hwp->IOBase;
    vgaCRIndex = vgaIOBase + 4;
    vgaCRReg = vgaIOBase + 5;

    xf86EnableIO();
    /* unprotect CRTC[0-7] */
    VGAOUT8(vgaCRIndex, 0x11);
    tmp = VGAIN8(vgaCRReg);
    VGAOUT8(vgaCRReg, tmp & 0x7f);

    /* unlock extended regs */
    VGAOUT16(vgaCRIndex, 0x4838);
    VGAOUT16(vgaCRIndex, 0xa539);
    VGAOUT16(0x3c4, 0x0608);

    VGAOUT8(vgaCRIndex, 0x40);
    tmp = VGAIN8(vgaCRReg);
    VGAOUT8(vgaCRReg, tmp & ~0x01);

    /* unlock sys regs */
    VGAOUT8(vgaCRIndex, 0x38);
    VGAOUT8(vgaCRReg, 0x48);

    {
	Gamma zeros = {0.0, 0.0, 0.0};

	if (!xf86SetGamma(pScrn, zeros))
	    return FALSE;
    }

    /* Unlock system registers. */
    VGAOUT16(vgaCRIndex, 0x4838);

    /* Next go on to detect amount of installed ram */

    VGAOUT8(vgaCRIndex, 0x36);            /* for register CR36 (CONFG_REG1), */
    config1 = VGAIN8(vgaCRReg);           /* get amount of vram installed */

    /* Compute the amount of video memory and offscreen memory. */

    psav->MemOffScreen = 0;
    if (!pScrn->videoRam) {
	static unsigned char RamSavage3D[] = { 8, 4, 4, 2 };
	static unsigned char RamSavage4[] =  { 2, 4, 8, 12, 16, 32, 64, 2 };
	static unsigned char RamSavageMX[] = { 2, 8, 4, 16, 8, 16, 4, 16 };
	static unsigned char RamSavageNB[] = { 0, 2, 4, 8, 16, 32, 2, 2 };

	switch( psav->Chipset ) {
	case S3_SAVAGE3D:
	    pScrn->videoRam = RamSavage3D[ (config1 & 0xC0) >> 6 ] * 1024;
	    break;

	case S3_SAVAGE4:
	case S3_SAVAGE2000:
	    pScrn->videoRam = RamSavage4[ (config1 & 0xE0) >> 5 ] * 1024;
	    break;

	case S3_SAVAGE_MX:
	    pScrn->videoRam = RamSavageMX[ (config1 & 0x0E) >> 1 ] * 1024;
	    break;

	case S3_PROSAVAGE:
	    pScrn->videoRam = RamSavageNB[ (config1 & 0xE0) >> 5 ] * 1024;
	    break;

	default:
	    /* How did we get here? */
	    pScrn->videoRam = 0;
	    break;
	}

	psav->videoRambytes = pScrn->videoRam * 1024;

	if (psav->MemOffScreen)
	    xf86DrvMsg(pScrn->scrnIndex, X_PROBED, 
		    "probed videoram:  %dk plus %dk offscreen\n",
		    pScrn->videoRam,
		    psav->MemOffScreen);
	else
	    xf86DrvMsg(pScrn->scrnIndex, X_PROBED, 
		    "probed videoram:  %dk\n",
		    pScrn->videoRam);
    } else {
	psav->videoRambytes = pScrn->videoRam * 1024;
	xf86DrvMsg(pScrn->scrnIndex, X_CONFIG,
		   "videoram =  %dk\n",
		    pScrn->videoRam);
    }

    /*
     * If we're running with acceleration, compute the command overflow 
     * buffer location.  The command overflow buffer must END at a
     * 4MB boundary; for all practical purposes, that means the very
     * end of the frame buffer.
     */

    if( psav->NoAccel ) {
	psav->CursorKByte = pScrn->videoRam - 4;
	psav->cobIndex = 0;
	psav->cobSize = 0;
	psav->cobOffset = psav->videoRambytes;
    }
    else if( S3_SAVAGE4_SERIES(psav->Chipset) ) {
	/*
	 * The Savage4 and ProSavage have COB coherency bugs which render 
	 * the buffer useless.  We disable it.
	 */
	psav->CursorKByte = pScrn->videoRam - 4;
	psav->cobIndex = 2;
	psav->cobSize = 0x8000 << psav->cobIndex;
	psav->cobOffset = psav->videoRambytes;
    }
    else
    {
        /* We use 128kB for the COB on all chips. */

	psav->cobIndex = 7;
	psav->cobSize = 0x400 << psav->cobIndex;
	psav->cobOffset = psav->videoRambytes - psav->cobSize;
    }

    /* 
     * We place the cursor in high memory, just before the command overflow
     * buffer.  The cursor must be aligned on a 4k boundary.
     */

    psav->CursorKByte = (psav->cobOffset >> 10)  - 4;

    /* reset graphics engine to avoid memory corruption */
    VGAOUT8(vgaCRIndex, 0x66);
    cr66 = VGAIN8(vgaCRReg);
    VGAOUT8(vgaCRReg, cr66 | 0x02);
    usleep(10000);

    VGAOUT8(vgaCRIndex, 0x66);
    VGAOUT8(vgaCRReg, cr66 & ~0x02);	/* clear reset flag */
    usleep(10000);

   /* Set status word positions based on chip type. */

    switch( psav->Chipset ) {
	case S3_SAVAGE3D:
	case S3_SAVAGE_MX:
	    psav->myWaitQueue		= WaitQueue3D;
	    psav->myWaitIdle		= WaitIdle3D;
	    psav->myWaitIdleEmpty	= WaitIdleEmpty3D;
	    psav->myWaitCommandEmpty	= WaitCommandEmpty3D;
	    break;

	case S3_SAVAGE4:
	case S3_PROSAVAGE:
	    psav->myWaitQueue		= WaitQueue4;
	    psav->myWaitIdle		= WaitIdle4;
	    psav->myWaitIdleEmpty	= WaitIdleEmpty4;
	    psav->myWaitCommandEmpty	= WaitCommandEmpty4;
	    break;

	case S3_SAVAGE2000:
	    psav->myWaitQueue		= WaitQueue2K;
	    psav->myWaitIdle		= WaitIdle2K;
	    psav->myWaitIdleEmpty	= WaitIdleEmpty2K;
	    psav->myWaitCommandEmpty	= WaitCommandEmpty2K;
	    break;
    }
  
    /* savage ramdac speeds */
    pScrn->numClocks = 4;
    pScrn->clock[0] = 250000;
    pScrn->clock[1] = 250000;
    pScrn->clock[2] = 220000;
    pScrn->clock[3] = 220000;

    if (psav->dacSpeedBpp <= 0) {
	if (pScrn->bitsPerPixel > 24)
	    psav->dacSpeedBpp = pScrn->clock[3];
	else if (pScrn->bitsPerPixel >= 24)
	    psav->dacSpeedBpp = pScrn->clock[2];
	else if ((pScrn->bitsPerPixel > 8) && (pScrn->bitsPerPixel < 24))
	    psav->dacSpeedBpp = pScrn->clock[1];
	else if (pScrn->bitsPerPixel <= 8)
	    psav->dacSpeedBpp = pScrn->clock[0];
    }

    /* set ramdac limits */
    psav->maxClock = psav->dacSpeedBpp;

    /* detect current mclk */
    VGAOUT8(0x3c4, 0x08);
    sr8 = VGAIN8(0x3c5);
    VGAOUT8(0x3c5, 0x06);
    VGAOUT8(0x3c4, 0x10);
    n = VGAIN8(0x3c5);
    VGAOUT8(0x3c4, 0x11);
    m = VGAIN8(0x3c5);
    VGAOUT8(0x3c4, 0x08);
    VGAOUT8(0x3c5, sr8);
    m &= 0x7f;
    n1 = n & 0x1f;
    n2 = (n >> 5) & 0x03;
    mclk = ((1431818 * (m+2)) / (n1+2) / (1 << n2) + 50) / 100;
    xf86DrvMsg(pScrn->scrnIndex, X_PROBED, "Detected current MCLK value of %1.3f MHz\n",
	       mclk / 1000.0);

    psav->minClock = 20000;

    pScrn->maxHValue = 2048;
    pScrn->maxVValue = 2048;
    pScrn->virtualX = pScrn->display->virtualX;

    clockRanges = xnfalloc(sizeof(ClockRange));
    clockRanges->next = NULL;
    clockRanges->minClock = psav->minClock;
    clockRanges->maxClock = psav->maxClock;
    clockRanges->clockIndex = -1;
    clockRanges->interlaceAllowed = TRUE;
    clockRanges->doubleScanAllowed = FALSE;

    i = xf86ValidateModes(pScrn, pScrn->monitor->Modes,
			  pScrn->display->modes, clockRanges, NULL, 
			  256, 2048, 16 * pScrn->bitsPerPixel,
			  128, 2048, 
			  pScrn->virtualX, pScrn->display->virtualY,
			  psav->videoRambytes, LOOKUP_BEST_REFRESH);

    if (i == -1) {
	xf86DrvMsg(pScrn->scrnIndex, X_ERROR, "xf86ValidateModes failure\n");
	SavageFreeRec(pScrn);
	return FALSE;
    }

    xf86PruneDriverModes(pScrn);

    if (i == 0 || pScrn->modes == NULL) {
	xf86DrvMsg(pScrn->scrnIndex, X_ERROR, "No valid modes found\n");
	SavageFreeRec(pScrn);
	return FALSE;
    }

    if( psav->UseBIOS )
    {
	/* Go probe the BIOS for all the modes and refreshes at this depth. */

	if( psav->ModeTable )
	{
	    SavageFreeBIOSModeTable( psav, &psav->ModeTable );
	}

	psav->ModeTable = SavageGetBIOSModeTable( psav, pScrn->depth );

	if( !psav->ModeTable || !psav->ModeTable->NumModes ) {
	    xf86DrvMsg(pScrn->scrnIndex, X_ERROR, 
		       "Failed to fetch any BIOS modes.\n");
	    SavageFreeRec(pScrn);
	    return FALSE;
	}

	/*if( xf86Verbose )*/
	{
	    int i;
	    SavageModeEntryPtr pmt;

	    xf86DrvMsg(pScrn->scrnIndex, X_PROBED,
		       "Found %d modes at this depth:\n",
		       psav->ModeTable->NumModes);

	    for(
		i = 0, pmt = psav->ModeTable->Modes; 
		i < psav->ModeTable->NumModes; 
		i++, pmt++ )
	    {
		int j;
		ErrorF( "    [%03x] %d x %d", 
			pmt->VesaMode, pmt->Width, pmt->Height );
		for( j = 0; j < pmt->RefreshCount; j++ )
		{
		    ErrorF( ", %dHz", pmt->RefreshRate[j] );
		}
		ErrorF( "\n");
	    }
	}
    }

    xf86SetCrtcForModes(pScrn, INTERLACE_HALVE_V);
    pScrn->currentMode = pScrn->modes;
    xf86PrintModes(pScrn);
    xf86SetDpi(pScrn, 0, 0);

    /* load bpp-specific modules */
    switch (pScrn->bitsPerPixel) {
    case 8:
	mod = "cfb";
	reqSym = "cfbScreenInit";
	break;
    case 16:
	mod = "cfb16";
	reqSym = "cfb16ScreenInit";
	break;
    case 32:
	mod = "cfb32";
	reqSym = "cfb32ScreenInit";
	break;
    }

    if (mod && xf86LoadSubModule(pScrn, mod) == NULL) {
	SavageFreeRec(pScrn);
	return FALSE;
    }

    xf86LoaderReqSymbols(reqSym, NULL);

    if( !psav->NoAccel ) {
	if( !xf86LoadSubModule(pScrn, "xaa") ) {
	    SavageFreeRec(pScrn);
	    return FALSE;
	}
	xf86LoaderReqSymLists(xaaSymbols, NULL );
    }

    if (psav->hwcursor) {
	if (!xf86LoadSubModule(pScrn, "ramdac")) {
	    SavageFreeRec(pScrn);
	    return FALSE;
	}
	xf86LoaderReqSymLists(ramdacSymbols, NULL);
    }

    if (psav->shadowFB) {
	if (!xf86LoadSubModule(pScrn, "shadowfb")) {
	    SavageFreeRec(pScrn);
	    return FALSE;
	}
	xf86LoaderReqSymLists(shadowSymbols, NULL);
    }

    return TRUE;
}


static Bool SavageEnterVT(int scrnIndex, int flags)
{
    ScrnInfoPtr pScrn = xf86Screens[scrnIndex];

    SavageSave(pScrn);
    return SavageModeInit(pScrn, pScrn->currentMode);
}


static void SavageLeaveVT(int scrnIndex, int flags)
{
    ScrnInfoPtr pScrn = xf86Screens[scrnIndex];
    vgaHWPtr hwp = VGAHWPTR(pScrn);
    SavagePtr psav = SAVPTR(pScrn);
    vgaRegPtr vgaSavePtr = &hwp->SavedReg;
    SavageRegPtr SavageSavePtr = &psav->SavedReg;

    SavageWriteMode(pScrn, vgaSavePtr, SavageSavePtr);
}


static void SavageSave(ScrnInfoPtr pScrn)
{
    unsigned char cr3a, cr53, cr66;
    vgaHWPtr hwp = VGAHWPTR(pScrn);
    vgaRegPtr vgaSavePtr = &hwp->SavedReg;
    SavagePtr psav = SAVPTR(pScrn);
    SavageRegPtr save = &psav->SavedReg;
    int vgaCRIndex, vgaCRReg, vgaIOBase;
    vgaIOBase = hwp->IOBase;
    vgaCRReg = vgaIOBase + 5;
    vgaCRIndex = vgaIOBase + 4;


    VGAOUT16(vgaCRIndex, 0x4838);
    VGAOUT16(vgaCRIndex, 0xa539);
    VGAOUT16(0x3c4, 0x0608);

    VGAOUT8(vgaCRIndex, 0x66);
    cr66 = VGAIN8(vgaCRReg);
    VGAOUT8(vgaCRReg, cr66 | 0x80);
    VGAOUT8(vgaCRIndex, 0x3a);
    cr3a = VGAIN8(vgaCRReg);
    VGAOUT8(vgaCRReg, cr3a | 0x80);
    VGAOUT8(vgaCRIndex, 0x53);
    cr53 = VGAIN8(vgaCRReg);
    VGAOUT8(vgaCRReg, cr53 & 0x7f);

    if (xf86IsPrimaryPci(psav->PciInfo))
	vgaHWSave(pScrn, vgaSavePtr, VGA_SR_ALL);
    else
	vgaHWSave(pScrn, vgaSavePtr, VGA_SR_MODE);

    VGAOUT8(vgaCRIndex, 0x66);
    VGAOUT8(vgaCRReg, cr66);
    VGAOUT8(vgaCRIndex, 0x3a);
    VGAOUT8(vgaCRReg, cr3a);

    VGAOUT8(vgaCRIndex, 0x66);
    VGAOUT8(vgaCRReg, cr66);
    VGAOUT8(vgaCRIndex, 0x3a);
    VGAOUT8(vgaCRReg, cr3a);

    /* unlock extended seq regs */
    VGAOUT8(0x3c4, 0x08);
    save->SR08 = VGAIN8(0x3c5);
    VGAOUT8(0x3c5, 0x06);

    /* now save all the extended regs we need */
    VGAOUT8(vgaCRIndex, 0x31);
    save->CR31 = VGAIN8(vgaCRReg);
    VGAOUT8(vgaCRIndex, 0x32);
    save->CR32 = VGAIN8(vgaCRReg);
    VGAOUT8(vgaCRIndex, 0x34);
    save->CR34 = VGAIN8(vgaCRReg);
    VGAOUT8(vgaCRIndex, 0x36);
    save->CR36 = VGAIN8(vgaCRReg);
    VGAOUT8(vgaCRIndex, 0x3a);
    save->CR3A = VGAIN8(vgaCRReg);
    VGAOUT8(vgaCRIndex, 0x40);
    save->CR40 = VGAIN8(vgaCRReg);
    VGAOUT8(vgaCRIndex, 0x42);
    save->CR42 = VGAIN8(vgaCRReg);
    VGAOUT8(vgaCRIndex, 0x45);
    save->CR45 = VGAIN8(vgaCRReg);
    VGAOUT8(vgaCRIndex, 0x50);
    save->CR50 = VGAIN8(vgaCRReg);
    VGAOUT8(vgaCRIndex, 0x51);
    save->CR51 = VGAIN8(vgaCRReg);
    VGAOUT8(vgaCRIndex, 0x53);
    save->CR53 = VGAIN8(vgaCRReg);
    VGAOUT8(vgaCRIndex, 0x58);
    save->CR58 = VGAIN8(vgaCRReg);
    VGAOUT8(vgaCRIndex, 0x60);
    save->CR60 = VGAIN8(vgaCRReg);
    VGAOUT8(vgaCRIndex, 0x66);
    save->CR66 = VGAIN8(vgaCRReg);
    VGAOUT8(vgaCRIndex, 0x67);
    save->CR67 = VGAIN8(vgaCRReg);
    VGAOUT8(vgaCRIndex, 0x68);
    save->CR68 = VGAIN8(vgaCRReg);
    VGAOUT8(vgaCRIndex, 0x69);
    save->CR69 = VGAIN8(vgaCRReg);
    VGAOUT8(vgaCRIndex, 0x6f);
    save->CR6F = VGAIN8(vgaCRReg);

    VGAOUT8(vgaCRIndex, 0x33);
    save->CR33 = VGAIN8(vgaCRReg);
    VGAOUT8(vgaCRIndex, 0x86);
    save->CR86 = VGAIN8(vgaCRReg);
    VGAOUT8(vgaCRIndex, 0x88);
    save->CR88 = VGAIN8(vgaCRReg);
    VGAOUT8(vgaCRIndex, 0x90);
    save->CR90 = VGAIN8(vgaCRReg);
    VGAOUT8(vgaCRIndex, 0x91);
    save->CR91 = VGAIN8(vgaCRReg);
    VGAOUT8(vgaCRIndex, 0xb0);
    save->CRB0 = VGAIN8(vgaCRReg) | 0x80;

    /* extended mode timing regs */
    VGAOUT8(vgaCRIndex, 0x3b);
    save->CR3B = VGAIN8(vgaCRReg);
    VGAOUT8(vgaCRIndex, 0x3c);
    save->CR3C = VGAIN8(vgaCRReg);
    VGAOUT8(vgaCRIndex, 0x43);
    save->CR43 = VGAIN8(vgaCRReg);
    VGAOUT8(vgaCRIndex, 0x5d);
    save->CR5D = VGAIN8(vgaCRReg);
    VGAOUT8(vgaCRIndex, 0x5e);
    save->CR5E = VGAIN8(vgaCRReg);
    VGAOUT8(vgaCRIndex, 0x65);
    save->CR65 = VGAIN8(vgaCRReg);

    /* save seq extended regs for DCLK PLL programming */
    VGAOUT8(0x3c4, 0x10);
    save->SR10 = VGAIN8(0x3c5);
    VGAOUT8(0x3c4, 0x11);
    save->SR11 = VGAIN8(0x3c5);

    VGAOUT8(0x3c4, 0x12);
    save->SR12 = VGAIN8(0x3c5);
    VGAOUT8(0x3c4, 0x13);
    save->SR13 = VGAIN8(0x3c5);
    VGAOUT8(0x3c4, 0x29);
    save->SR29 = VGAIN8(0x3c5);

    VGAOUT8(0x3c4, 0x15);
    save->SR15 = VGAIN8(0x3c5);
    VGAOUT8(0x3c4, 0x18);
    save->SR18 = VGAIN8(0x3c5);

    /* Save flat panel expansion regsters. */

    if( psav->Chipset == S3_SAVAGE_MX ) {
	int i;
	for( i = 0; i < 8; i++ ) {
	    VGAOUT8(0x3c4, 0x54+i);
	    save->SR54[i] = VGAIN8(0x3c5);
	}
    }

    VGAOUT8(vgaCRIndex, 0x66);
    cr66 = VGAIN8(vgaCRReg);
    VGAOUT8(vgaCRReg, cr66 | 0x80);
    VGAOUT8(vgaCRIndex, 0x3a);
    cr3a = VGAIN8(vgaCRReg);
    VGAOUT8(vgaCRReg, cr3a | 0x80);

    /* now save MIU regs */
    save->MMPR0 = INREG(FIFO_CONTROL_REG);
    save->MMPR1 = INREG(MIU_CONTROL_REG);
    save->MMPR2 = INREG(STREAMS_TIMEOUT_REG);
    save->MMPR3 = INREG(MISC_TIMEOUT_REG);

    VGAOUT8(vgaCRIndex, 0x3a);
    VGAOUT8(vgaCRReg, cr3a);
    VGAOUT8(vgaCRIndex, 0x66);
    VGAOUT8(vgaCRReg, cr66);

    if (!psav->ModeStructInit) {
	vgaHWCopyReg(&hwp->ModeReg, vgaSavePtr);
	memcpy(&psav->ModeReg, save, sizeof(SavageRegRec));
	psav->ModeStructInit = TRUE;
    }

#if 0
    if (xf86GetVerbosity() > 1)
	SavagePrintRegs(pScrn);
#endif

    return;
}


static void SavageWriteMode(ScrnInfoPtr pScrn, vgaRegPtr vgaSavePtr,
			    SavageRegPtr restore)
{
    unsigned char tmp, cr3a, cr66, cr67;
    vgaHWPtr hwp = VGAHWPTR(pScrn);
    SavagePtr psav = SAVPTR(pScrn);
    int vgaCRIndex, vgaCRReg, vgaIOBase;
    Bool graphicsMode = FALSE;


    vgaIOBase = hwp->IOBase;
    vgaCRIndex = vgaIOBase + 4;
    vgaCRReg = vgaIOBase + 5;

    /*
     * If we figured out a VESA mode number for this timing, just use
     * the S3 BIOS to do the switching, with a few additional tweaks.
     */

    if( psav->UseBIOS && restore->mode > 0x13 )
    {
	int width;
	unsigned short cr6d;
	unsigned short cr79 = 0;

	/* Set up the mode.  Don't clear video RAM. */
	SavageSetVESAMode( psav, restore->mode | 0x8000, restore->refresh );

	/* Restore the DAC. */
	vgaHWRestore(pScrn, vgaSavePtr, VGA_SR_CMAP);

	/* Unlock the extended registers. */

#if 0
	/* Which way is better? */
	hwp->writeCrtc( hwp, 0x38, 0x48 );
	hwp->writeCrtc( hwp, 0x39, 0xa0 );
	hwp->writeSeq( hwp, 0x08, 0x06 );
#endif

	VGAOUT16(vgaCRIndex, 0x4838);
	VGAOUT16(vgaCRIndex, 0xA039);
	VGAOUT16(0x3c4, 0x0608);

	/* Enable linear addressing. */

	VGAOUT16(vgaCRIndex, 0x1358);

	/* Disable old MMIO. */

	VGAOUT8(vgaCRIndex, 0x53);
	tmp = VGAIN8(vgaCRReg);
	VGAOUT8(vgaCRReg, tmp & ~0x10);

	/* We may need TV/panel fixups here.  See s3bios.c line 2904. */

	/* Set FIFO fetch delay. */
	VGAOUT8(vgaCRIndex, 0x85);
	tmp = VGAIN8(vgaCRReg);
	VGAOUT8(vgaCRReg, (tmp & 0xf8) | 0x03);

	/* Patch CR79.  These values are magical. */

	if( psav->Chipset != S3_SAVAGE_MX )
	{
	    VGAOUT8(vgaCRIndex, 0x6d);
	    cr6d = VGAIN8(vgaCRReg);

	    cr79 = 0x04;

	    if( pScrn->displayWidth >= 1024 )
	    {
		if(pScrn->bitsPerPixel == 32 )
		{
		    if( restore->refresh >= 130 )
			cr79 = 0x03;
		    else if( pScrn->displayWidth >= 1280 )
			cr79 = 0x02;
		    else if(
			(pScrn->displayWidth == 1024) &&
			(restore->refresh >= 75)
		    )
		    {
			if( cr6d && LCD_ACTIVE )
			    cr79 = 0x05;
			else
			    cr79 = 0x08;
		    }
		}
		else if( pScrn->bitsPerPixel == 16)
		{

/* The windows driver uses 0x13 for 16-bit 130Hz, but I see terrible
 * screen artifacts with that value.  Let's keep it low for now.
 *		if( restore->refresh >= 130 )
 *		    cr79 = 0x13;
 *		else
 */
		    if( pScrn->displayWidth == 1024 )
		    {
			if( cr6d && LCD_ACTIVE )
			    cr79 = 0x08;
			else
			    cr79 = 0x0e;
		    }
		}
	    }
	}

        if( (psav->Chipset != S3_SAVAGE2000) && 
	    (psav->Chipset != S3_SAVAGE_MX) )
	    VGAOUT16(vgaCRIndex, (cr79 << 8) | 0x79);

	/* Make sure 16-bit memory access is enabled. */

	VGAOUT16(vgaCRIndex, 0x0c31);

	/* Enable the graphics engine. */

	VGAOUT16(vgaCRIndex, 0x0140);
	#if 0
	if( !psav->NoAccel )
	    S3SAVInitialize2DEngine();
	#endif

	/* Handle the pitch. */

        VGAOUT8(vgaCRIndex, 0x50);
	tmp = VGAIN8(vgaCRReg);
        VGAOUT8(vgaCRReg, tmp | 0xC1);

	width = (pScrn->displayWidth * (pScrn->bitsPerPixel / 8)) >> 3;
	VGAOUT16(vgaCRIndex, ((width & 0xff) << 8) | 0x13 );
	VGAOUT16(vgaCRIndex, ((width & 0x300) << 4) | 0x51 );

	/* Some non-S3 BIOSes enable block write even on non-SGRAM devices. */

        if( psav->Chipset == S3_SAVAGE2000 )
	{
	    VGAOUT8(vgaCRIndex, 0x73);
	    tmp = VGAIN8(vgaCRReg);
	    VGAOUT8(vgaCRReg, tmp & 0xdf );
	}
	else if( psav->Chipset != S3_SAVAGE_MX )
	{
	    VGAOUT8(vgaCRIndex, 0x68);
	    if( !(VGAIN8(vgaCRReg) & 0x80) )
	    {
		/* Not SGRAM; disable block write. */
		VGAOUT8(vgaCRIndex, 0x88);
		tmp = VGAIN8(vgaCRReg);
		VGAOUT8(vgaCRReg, tmp | 0x10);
	    }
	}

	if( !psav->NoAccel )
	    SavageInitialize2DEngine(pScrn);

	VGAOUT16(vgaCRIndex, 0x0140);

	SavageSetGBD(pScrn);

	return;
    }

    VGAOUT8(0x3c2, 0x23);
    VGAOUT16(vgaCRIndex, 0x4838);
    VGAOUT16(vgaCRIndex, 0xa539);
    VGAOUT16(0x3c4, 0x0608);

    vgaHWProtect(pScrn, TRUE);

    /* will we be reenabling STREAMS for the new mode? */
    psav->STREAMSRunning = 0;

    graphicsMode = (restore->CR31 & 0x0a) ? TRUE : FALSE;

    /* reset GE to make sure nothing is going on */
    VGAOUT8(vgaCRIndex, 0x66);
    if(VGAIN8(vgaCRReg) & 0x01)
	SavageGEReset(pScrn,0,__LINE__,__FILE__);

    /*
     * Some Savage/MX and /IX systems go nuts when trying to exit the
     * server after WindowMaker has displayed a gradient background.  I
     * haven't been able to find what causes it, but a non-destructive
     * switch to mode 3 here seems to eliminate the issue.
     */

#define CLEAR_X86_REGS(pi) \
  (pi)->ax = (pi)->bx = (pi)->cx = (pi)->dx = (pi)->si = (pi)->di = 0

    if( !graphicsMode && psav->pInt10 ) {
	CLEAR_X86_REGS( psav->pInt10 );
	psav->pInt10->num = 0x10;
	psav->pInt10->ax = 0x0083;
        xf86ExecX86int10( psav->pInt10 );
    }

    VGAOUT8(vgaCRIndex, 0x67);
    cr67 = VGAIN8(vgaCRReg);
    VGAOUT8(vgaCRReg, restore->CR67 & ~0x0c); /* no STREAMS yet */

    /* restore extended regs */
    VGAOUT8(vgaCRIndex, 0x66);
    VGAOUT8(vgaCRReg, restore->CR66);
    VGAOUT8(vgaCRIndex, 0x3a);
    VGAOUT8(vgaCRReg, restore->CR3A);
    VGAOUT8(vgaCRIndex, 0x31);
    VGAOUT8(vgaCRReg, restore->CR31);
    VGAOUT8(vgaCRIndex, 0x32);
    VGAOUT8(vgaCRReg, restore->CR32);
    VGAOUT8(vgaCRIndex, 0x58);
    VGAOUT8(vgaCRReg, restore->CR58);
    VGAOUT8(vgaCRIndex, 0x53);
    VGAOUT8(vgaCRReg, restore->CR53 & 0x7f);

    VGAOUT8(0x3c4, 0x08);
    VGAOUT8(0x3c5, 0x06);
    VGAOUT8(0x3c4, 0x12);
    VGAOUT8(0x3c5, restore->SR12);
    VGAOUT8(0x3c4, 0x13);
    VGAOUT8(0x3c5, restore->SR13);
    VGAOUT8(0x3c4, 0x29);
    VGAOUT8(0x3c5, restore->SR29);
    VGAOUT8(0x3c4, 0x15);
    VGAOUT8(0x3c5, restore->SR15);

    /* Restore flat panel expansion regsters. */
    if( psav->Chipset == S3_SAVAGE_MX ) {
	int i;
	for( i = 0; i < 8; i++ ) {
	    VGAOUT8(0x3c4, 0x54+i);
	    VGAOUT8(0x3c5, restore->SR54[i]);
	}
    }

    /* restore the standard vga regs */
    if (xf86IsPrimaryPci(psav->PciInfo))
	vgaHWRestore(pScrn, vgaSavePtr, VGA_SR_ALL);
    else
	vgaHWRestore(pScrn, vgaSavePtr, VGA_SR_MODE);

    /* extended mode timing registers */
    VGAOUT8(vgaCRIndex, 0x53);
    VGAOUT8(vgaCRReg, restore->CR53);
    VGAOUT8(vgaCRIndex, 0x5d);
    VGAOUT8(vgaCRReg, restore->CR5D);
    VGAOUT8(vgaCRIndex, 0x5e);
    VGAOUT8(vgaCRReg, restore->CR5E);
    VGAOUT8(vgaCRIndex, 0x3b);
    VGAOUT8(vgaCRReg, restore->CR3B);
    VGAOUT8(vgaCRIndex, 0x3c);
    VGAOUT8(vgaCRReg, restore->CR3C);
    VGAOUT8(vgaCRIndex, 0x43);
    VGAOUT8(vgaCRReg, restore->CR43);
    VGAOUT8(vgaCRIndex, 0x65);
    VGAOUT8(vgaCRReg, restore->CR65);

    /* restore the desired video mode with cr67 */
    VGAOUT8(vgaCRIndex, 0x67);
    VGAOUT8(vgaCRReg, restore->CR67 & ~0x0c); /* no STREAMS yet */

    /* other mode timing and extended regs */
    VGAOUT8(vgaCRIndex, 0x34);
    VGAOUT8(vgaCRReg, restore->CR34);
    VGAOUT8(vgaCRIndex, 0x40);
    VGAOUT8(vgaCRReg, restore->CR40);
    VGAOUT8(vgaCRIndex, 0x42);
    VGAOUT8(vgaCRReg, restore->CR42);
    VGAOUT8(vgaCRIndex, 0x45);
    VGAOUT8(vgaCRReg, restore->CR45);
    VGAOUT8(vgaCRIndex, 0x50);
    VGAOUT8(vgaCRReg, restore->CR50);
    VGAOUT8(vgaCRIndex, 0x51);
    VGAOUT8(vgaCRReg, restore->CR51);

    /* memory timings */
    VGAOUT8(vgaCRIndex, 0x36);
    VGAOUT8(vgaCRReg, restore->CR36);
    VGAOUT8(vgaCRIndex, 0x60);
    VGAOUT8(vgaCRReg, restore->CR60);
    VGAOUT8(vgaCRIndex, 0x68);
    VGAOUT8(vgaCRReg, restore->CR68);
    VGAOUT8(vgaCRIndex, 0x69);
    VGAOUT8(vgaCRReg, restore->CR69);
    VGAOUT8(vgaCRIndex, 0x6f);
    VGAOUT8(vgaCRReg, restore->CR6F);

    VGAOUT8(vgaCRIndex, 0x33);
    VGAOUT8(vgaCRReg, restore->CR33);
    VGAOUT8(vgaCRIndex, 0x86);
    VGAOUT8(vgaCRReg, restore->CR86);
    VGAOUT8(vgaCRIndex, 0x88);
    VGAOUT8(vgaCRReg, restore->CR88);
    VGAOUT8(vgaCRIndex, 0x90);
    VGAOUT8(vgaCRReg, restore->CR90);
    VGAOUT8(vgaCRIndex, 0x91);
    VGAOUT8(vgaCRReg, restore->CR91);
    if( psav->Chipset == S3_SAVAGE4 )
    {
	VGAOUT8(vgaCRIndex, 0xb0);
	VGAOUT8(vgaCRReg, restore->CRB0);
    }

    VGAOUT8(vgaCRIndex, 0x32);
    VGAOUT8(vgaCRReg, restore->CR32);

    /* unlock extended seq regs */
    VGAOUT8(0x3c4, 0x08);
    VGAOUT8(0x3c5, 0x06);

    /* Restore extended sequencer regs for MCLK. SR10 == 255 indicates that 
     * we should leave the default SR10 and SR11 values there.
     */
    if (restore->SR10 != 255) {
	VGAOUT8(0x3c4, 0x10);
	VGAOUT8(0x3c5, restore->SR10);
	VGAOUT8(0x3c4, 0x11);
	VGAOUT8(0x3c5, restore->SR11);
    }

    /* restore extended seq regs for dclk */
    VGAOUT8(0x3c4, 0x12);
    VGAOUT8(0x3c5, restore->SR12);
    VGAOUT8(0x3c4, 0x13);
    VGAOUT8(0x3c5, restore->SR13);
    VGAOUT8(0x3c4, 0x29);
    VGAOUT8(0x3c5, restore->SR29);

    VGAOUT8(0x3c4, 0x18);
    VGAOUT8(0x3c5, restore->SR18);

    /* load new m, n pll values for dclk & mclk */
    VGAOUT8(0x3c4, 0x15);
    tmp = VGAIN8(0x3c5) & ~0x21;

    VGAOUT8(0x3c5, tmp | 0x03);
    VGAOUT8(0x3c5, tmp | 0x23);
    VGAOUT8(0x3c5, tmp | 0x03);
    VGAOUT8(0x3c5, restore->SR15);

    VGAOUT8(0x3c4, 0x08);
    VGAOUT8(0x3c5, restore->SR08);

    /* now write out cr67 in full, possibly starting STREAMS */
    VerticalRetraceWait();
    VGAOUT8(vgaCRIndex, 0x67);
#if 0
    VGAOUT8(vgaCRReg, 0x50);
    usleep(10000);
    VGAOUT8(vgaCRIndex, 0x67);
#endif
    VGAOUT8(vgaCRReg, restore->CR67);

    VGAOUT8(vgaCRIndex, 0x66);
    cr66 = VGAIN8(vgaCRReg);
    VGAOUT8(vgaCRReg, cr66 | 0x80);
    VGAOUT8(vgaCRIndex, 0x3a);
    cr3a = VGAIN8(vgaCRReg);
    VGAOUT8(vgaCRReg, cr3a | 0x80);

    if (graphicsMode)
	SavageGEReset(pScrn,0,__LINE__,__FILE__);

    VerticalRetraceWait();
    OUTREG(FIFO_CONTROL_REG, restore->MMPR0);
    WaitIdle();
    OUTREG(MIU_CONTROL_REG, restore->MMPR1);
    WaitIdle();
    OUTREG(STREAMS_TIMEOUT_REG, restore->MMPR2);
    WaitIdle();
    OUTREG(MISC_TIMEOUT_REG, restore->MMPR3);

    /* If we're going into graphics mode and acceleration was enabled, */
    /* go set up the BCI buffer and the global bitmap descriptor. */

    if( graphicsMode && (!psav->NoAccel) )
    {
	VGAOUT8(vgaCRIndex, 0x50);
	tmp = VGAIN8(vgaCRReg);
	VGAOUT8(vgaCRReg, tmp | 0xC1);
	SavageInitialize2DEngine(pScrn);
    }

    VGAOUT8(vgaCRIndex, 0x66);
    VGAOUT8(vgaCRReg, cr66);
    VGAOUT8(vgaCRIndex, 0x3a);
    VGAOUT8(vgaCRReg, cr3a);

    if( graphicsMode )
	SavageSetGBD(pScrn);

    vgaHWProtect(pScrn, FALSE);

    return;
}


static Bool SavageMapMMIO(ScrnInfoPtr pScrn)
{
    SavagePtr psav;
    vgaHWPtr hwp;

    psav = SAVPTR(pScrn);

    if( S3_SAVAGE3D_SERIES(psav->Chipset) ) {
	psav->MmioBase = psav->PciInfo->memBase[0] + SAVAGE_NEWMMIO_REGBASE_S3;
	psav->FrameBufferBase = psav->PciInfo->memBase[0];
    }
    else {
	psav->MmioBase = psav->PciInfo->memBase[0] + SAVAGE_NEWMMIO_REGBASE_S4;
	psav->FrameBufferBase = psav->PciInfo->memBase[1];
    }

    xf86DrvMsg( pScrn->scrnIndex, X_PROBED,
	"mapping MMIO @ 0x%x with size 0x%x\n",
	psav->MmioBase, SAVAGE_NEWMMIO_REGSIZE);

    psav->MapBase = xf86MapPciMem(pScrn->scrnIndex, VIDMEM_MMIO, psav->PciTag,
				  psav->MmioBase,
				  SAVAGE_NEWMMIO_REGSIZE);
#if 0
    psav->MapBaseDense = xf86MapPciMem(pScrn->scrnIndex, VIDMEM_MMIO_32BIT,
				       psav->PciTag,
				       psav->PciInfo->memBase[0],
				       0x8000);
#endif
    if (!psav->MapBase) {
	xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
		   "Internal error: cound not map registers\n");
	return FALSE;
    }

    psav->BciMem = psav->MapBase + 0x10000;

    SavageEnableMMIO(pScrn);
    hwp = VGAHWPTR(pScrn);
    vgaHWGetIOBase(hwp);

    return TRUE;
}



static Bool SavageMapFB(ScrnInfoPtr pScrn)
{
    SavagePtr psav = SAVPTR(pScrn);

    xf86DrvMsg( pScrn->scrnIndex, X_PROBED,
	"mapping framebuffer @ 0x%x with size 0x%x\n", 
	psav->FrameBufferBase, psav->videoRambytes);

    if (psav->videoRambytes) {
	psav->FBBase = xf86MapPciMem(pScrn->scrnIndex, VIDMEM_FRAMEBUFFER,
				     psav->PciTag, psav->FrameBufferBase,
				     psav->videoRambytes);
	if (!psav->FBBase) {
	    xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
		       "Internal error: could not map framebuffer\n");
	    return FALSE;
	}
	psav->FBStart = psav->FBBase;
    }
    pScrn->memPhysBase = psav->PciInfo->memBase[0];
    pScrn->fbOffset = 0;
/*	psav->FBCursorOffset = psav->videoRambytes - 1024; */

    return TRUE;
}


static void SavageUnmapMem(ScrnInfoPtr pScrn)
{
    SavagePtr psav;

    psav = SAVPTR(pScrn);

    if (psav->PrimaryVidMapped) {
	vgaHWUnmapMem(pScrn);
	psav->PrimaryVidMapped = FALSE;
    }

    SavageDisableMMIO(pScrn);
    if (psav->MapBase)
	xf86UnMapVidMem(pScrn->scrnIndex, (pointer)psav->MapBase,
			SAVAGE_NEWMMIO_REGSIZE);
    if (psav->FBBase)
	xf86UnMapVidMem(pScrn->scrnIndex, (pointer)psav->FBBase,
			psav->videoRambytes);
#if 0
    xf86UnMapVidMem(pScrn->scrnIndex, (pointer)psav->MapBaseDense,
		    0x8000);
#endif

    return;
}


static Bool SavageScreenInit(int scrnIndex, ScreenPtr pScreen,
			     int argc, char **argv)
{
    ScrnInfoPtr pScrn;
    SavagePtr psav;
    int ret;

    pScrn = xf86Screens[pScreen->myNum];
    psav = SAVPTR(pScrn);

    if (!SavageMapFB(pScrn))
	return FALSE;

    SavageSave(pScrn);

    vgaHWBlankScreen(pScrn, TRUE);

    if (!SavageModeInit(pScrn, pScrn->currentMode))
	return FALSE;

    miClearVisualTypes();

    if (pScrn->bitsPerPixel > 8) {
	if (!miSetVisualTypes(pScrn->depth, TrueColorMask,
			      pScrn->rgbBits, pScrn->defaultVisual))
	    return FALSE;
    } else {
	if (!miSetVisualTypes(pScrn->depth, miGetDefaultVisualMask(pScrn->depth),
			      pScrn->rgbBits, pScrn->defaultVisual))
	    return FALSE;
    }

    ret = SavageInternalScreenInit(scrnIndex, pScreen);
    if (!ret)
	return FALSE;

    xf86SetBlackWhitePixels(pScreen);

    if (pScrn->bitsPerPixel > 8) {
	VisualPtr visual;

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

    if( !psav->NoAccel ) {
	SavageInitAccel(pScreen);
    }

    miInitializeBackingStore(pScreen);
    xf86SetBackingStore(pScreen);

#if 0
    SavageDGAInit(pScreen);
#endif

    miDCInitialize(pScreen, xf86GetPointerScreenFuncs());

    if (psav->hwcursor)
	if (!SavageHWCursorInit(pScreen))
	    xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
	       "Hardware cursor initialization failed\n");

    if (psav->shadowFB) {
	RefreshAreaFuncPtr refreshArea = SavageRefreshArea;
      
#if 1
	if(psav->rotate) {
	    if (!psav->PointerMoved) {
		psav->PointerMoved = pScrn->PointerMoved;
		pScrn->PointerMoved = SavagePointerMoved;
	    }

	    switch(pScrn->bitsPerPixel) {
	    case 8:	refreshArea = SavageRefreshArea8;	break;
	    case 16:	refreshArea = SavageRefreshArea16;	break;
	    case 24:	refreshArea = SavageRefreshArea24;	break;
	    case 32:	refreshArea = SavageRefreshArea32;	break;
	    }
	}
#endif
      
	ShadowFBInit(pScreen, refreshArea);
    }

    if (!miCreateDefColormap(pScreen))
	    return FALSE;

    if (!xf86HandleColormaps(pScreen, 256, 6, SavageLoadPalette, NULL,
			     CMAP_RELOAD_ON_MODE_SWITCH))
	return FALSE;

    vgaHWBlankScreen(pScrn, FALSE);

    psav->CloseScreen = pScreen->CloseScreen;
    pScreen->SaveScreen = SavageSaveScreen;
    pScreen->CloseScreen = SavageCloseScreen;

#ifdef DPMSExtension
    if (xf86DPMSInit(pScreen, SavageDPMS, 0) == FALSE)
	xf86DrvMsg(pScrn->scrnIndex, X_ERROR, "DPMS initialization failed\n");
#endif

    if (serverGeneration == 1)
	xf86ShowUnusedOptions(pScrn->scrnIndex, pScrn->options);

    return TRUE;
}


static int SavageInternalScreenInit(int scrnIndex, ScreenPtr pScreen)
{
    int ret = TRUE;
    ScrnInfoPtr pScrn;
    SavagePtr psav;
    int width, height, displayWidth;
    unsigned char *FBStart;

    pScrn = xf86Screens[pScreen->myNum];
    psav = SAVPTR(pScrn);

    displayWidth = pScrn->displayWidth;

    if (psav->rotate) {
	height = pScrn->virtualX;
	width = pScrn->virtualY;
    } else {
	width = pScrn->virtualX;
	height = pScrn->virtualY;
    }
  
  
    if(psav->shadowFB) {
	psav->ShadowPitch = BitmapBytePad(pScrn->bitsPerPixel * width);
	psav->ShadowPtr = xalloc(psav->ShadowPitch * height);
	displayWidth = psav->ShadowPitch / (pScrn->bitsPerPixel >> 3);
	FBStart = psav->ShadowPtr;
    } else {
	psav->ShadowPtr = NULL;
	FBStart = psav->FBStart;
    }

    switch (pScrn->bitsPerPixel) {
    case 8:
	ret = cfbScreenInit(pScreen, FBStart, width, height,
			    pScrn->xDpi, pScrn->yDpi,
			    displayWidth);
	break;
    case 16:
	ret = cfb16ScreenInit(pScreen, FBStart, width, height,
			      pScrn->xDpi, pScrn->yDpi,
			      displayWidth);
	break;
    case 32:
	ret = cfb32ScreenInit(pScreen, FBStart, width, height,
			      pScrn->xDpi, pScrn->yDpi,
			      displayWidth);
	break;
    default:
	xf86DrvMsg(scrnIndex, X_ERROR,
		   "Internal error: invalid bpp (%d) in SavageScreenInit\n",
		   pScrn->bitsPerPixel);
	ret = FALSE;
	break;
    }

    return ret;
}


static ModeStatus SavageValidMode(int index, DisplayModePtr mode,
				  Bool verbose, int flags)
{
    /* TODO check modes */
    return MODE_OK;
}

static Bool SavageModeInit(ScrnInfoPtr pScrn, DisplayModePtr mode)
{
    vgaHWPtr hwp = VGAHWPTR(pScrn);
    SavagePtr psav = SAVPTR(pScrn);
    int width, dclk, i, j; /*, refresh; */
    unsigned int m, n, r;
    unsigned char tmp = 0;
    SavageRegPtr new = &psav->ModeReg;
    vgaRegPtr vganew = &hwp->ModeReg;
    int vgaCRIndex, vgaCRReg, vgaIOBase;

    vgaIOBase = hwp->IOBase;
    vgaCRIndex = vgaIOBase + 4;
    vgaCRReg = vgaIOBase + 5;

#if 0
    ErrorF("Clock = %d, HDisplay = %d, HSStart = %d\n",
	    mode->Clock, mode->HDisplay, mode->HSyncStart);
    ErrorF("HSEnd = %d, HSkew = %d\n",
	    mode->HSyncEnd, mode->HSkew);
    ErrorF("VDisplay - %d, VSStart = %d, VSEnd = %d\n",
	    mode->VDisplay, mode->VSyncStart, mode->VSyncEnd);
    ErrorF("VTotal = %d\n",
	    mode->VTotal);
    ErrorF("HDisplay = %d, HSStart = %d\n",
	    mode->CrtcHDisplay, mode->CrtcHSyncStart);
    ErrorF("HSEnd = %d, HSkey = %d\n",
	    mode->CrtcHSyncEnd, mode->CrtcHSkew);
    ErrorF("VDisplay - %d, VSStart = %d, VSEnd = %d\n",
	    mode->CrtcVDisplay, mode->CrtcVSyncStart, mode->CrtcVSyncEnd);
    ErrorF("VTotal = %d\n",
	    mode->CrtcVTotal);
#endif



    if (pScrn->bitsPerPixel == 8)
	psav->HorizScaleFactor = 1;
    else if (pScrn->bitsPerPixel == 16)
	psav->HorizScaleFactor = 1;	/* I don't think we ever want 2 */
    else
	psav->HorizScaleFactor = 1;

    if (psav->HorizScaleFactor == 2)
	if (!mode->CrtcHAdjusted) {
	    mode->CrtcHDisplay *= 2;
	    mode->CrtcHSyncStart *= 2;
	    mode->CrtcHSyncEnd *= 2;
	    mode->CrtcHTotal *= 2;
	    mode->CrtcHSkew *= 2;
	    mode->CrtcHAdjusted = TRUE;
	}

    if (!vgaHWInit(pScrn, mode))
	return FALSE;

    new->mode = 0;

    if( psav->UseBIOS ) {
	int refresh;
	SavageModeEntryPtr pmt;

	/* Scan through our BIOS list to locate the closest valid mode. */

	/* If we ever break 4GHz clocks on video boards, we'll need to
	 * change this.
	 */

        refresh = (mode->Clock * 1000) / (mode->HTotal * mode->VTotal);

#ifdef EXTENDED_DEBUG
	ErrorF( "Desired refresh rate = %dHz\n", refresh );
#endif

	for( i = 0, pmt = psav->ModeTable->Modes; 
	    i < psav->ModeTable->NumModes;
	    i++, pmt++ )
	{
	    if( (pmt->Width == mode->HDisplay) && 
	        (pmt->Height == mode->VDisplay) )
	    {
		int j;
		int jDelta = 99;
		int jBest = 0;

		/* We have an acceptable mode.  Find a refresh rate. */

		new->mode = pmt->VesaMode;
		for( j = 0; j < pmt->RefreshCount; j++ )
		{
		    if( pmt->RefreshRate[j] == refresh )
		    {
			/* Exact match. */
			jBest = j;
			break;
		    }
		    else if( iabs(pmt->RefreshRate[j] - refresh) < jDelta )
		    {
			jDelta = iabs(pmt->RefreshRate[j] - refresh);
			jBest = j;
		    }
		}

		new->refresh = pmt->RefreshRate[jBest];
		break;
	    }
       }
    }

    if( new->mode ) {
	/* Success: we found a match in the BIOS. */
	xf86DrvMsg(pScrn->scrnIndex, X_PROBED, 
		  "Chose mode %x at %dHz.\n", new->mode, new->refresh );
    }
    else {

	/* We failed to find a match in the BIOS. */
	/* Fallthrough to the traditional register-crunching.  */

	VGAOUT8(vgaCRIndex, 0x3a);
	tmp = VGAIN8(vgaCRReg);
	if (psav->pci_burst)
	    new->CR3A = (tmp & 0x7f) | 0x15;
	else
	    new->CR3A = tmp | 0x95;

	new->CR53 = 0x00;
	new->CR31 = 0x8c;
	new->CR66 = 0x89;

	VGAOUT8(vgaCRIndex, 0x58);
	new->CR58 = VGAIN8(vgaCRReg) & 0x80;
	new->CR58 |= 0x13;

#if 0
	VGAOUT8(vgaCRIndex, 0x55);
	new->CR55 = VGAIN8(vgaCRReg);
	if (psav->hwcursor)
		new->CR55 |= 0x10;
#endif

	dclk = mode->Clock;
	new->CR67 = 0x00;
	new->SR15 = 0x03 | 0x80;
	new->SR18 = 0x00;
	new->CR43 = new->CR45 = new->CR65 = 0x00;

	VGAOUT8(vgaCRIndex, 0x40);
	new->CR40 = VGAIN8(vgaCRReg) & ~0x01;

	new->MMPR0 = 0x010400;
	new->MMPR1 = 0x00;
	new->MMPR2 = 0x0808;
	new->MMPR3 = 0x08080810;

	if (psav->fifo_aggressive || psav->fifo_moderate ||
	    psav->fifo_conservative) {
		new->MMPR1 = 0x0200;
		new->MMPR2 = 0x1808;
		new->MMPR3 = 0x08081810;
	}

	if (psav->MCLK <= 0) {
		new->SR10 = 255;
		new->SR11 = 255;
	}

	psav->NeedSTREAMS = FALSE;

    /****
     * TODO
     * old code uses the "dclk<=110000" path for all non-MX/IX, and the
     * >110000 path for MX/IX.  What does this mean?
     ****/

	if (pScrn->bitsPerPixel == 8) {
	    if (dclk <= 110000)
		new->CR67 = 0x00;	/* 8bpp, 135Mhz */
	    else
		new->CR67 = 0x10;	/* 8bpp, 220Mhz */
	} else if ((pScrn->bitsPerPixel == 16) && (pScrn->weight.green == 5)) {
	    if (dclk <= 110000)
		new->CR67 = 0x20;	/* 15bpp, 135Mhz */
	    else
		new->CR67 = 0x30;	/* 15bpp, 220Mhz */
	} else if (pScrn->bitsPerPixel == 16) {
	    if (dclk <= 110000)
		new->CR67 = 0x40;	/* 16bpp, 135Mhz */
	    else
		new->CR67 = 0x50;	/* 16bpp, 220Mhz */
	} else if (pScrn->bitsPerPixel == 32) {
	    new->CR67 = 0xd0;
	}

	SavageCalcClock(dclk, 1, 1, 127, 0, 4, 180000, 360000,
			&m, &n, &r);
	new->SR12 = (r << 6) | (n & 0x3f);
	new->SR13 = m & 0xff;
	new->SR29 = (r & 4) | (m & 0x100) >> 5 | (n & 0x40) >> 2;

	if (psav->fifo_moderate) {
	    if (pScrn->bitsPerPixel < 24)
		new->MMPR0 -= 0x8000;
	    else
		new->MMPR0 -= 0x4000;
	} else if (psav->fifo_aggressive) {
	    if (pScrn->bitsPerPixel < 24)
		new->MMPR0 -= 0xc000;
	    else
		new->MMPR0 -= 0x6000;
	}

	if (mode->Flags & V_INTERLACE)
	    new->CR42 = 0x20;
	else
	    new->CR42 = 0x00;

	new->CR34 = 0x10;

	i = ((((mode->CrtcHTotal >> 3) - 5) & 0x100) >> 8) |
	    ((((mode->CrtcHDisplay >> 3) - 1) & 0x100) >> 7) |
	    ((((mode->CrtcHSyncStart >> 3) - 1) & 0x100) >> 6) |
	    ((mode->CrtcHSyncStart & 0x800) >> 7);

	if ((mode->CrtcHSyncEnd >> 3) - (mode->CrtcHSyncStart >> 3) > 64)
	    i |= 0x08;
	if ((mode->CrtcHSyncEnd >> 3) - (mode->CrtcHSyncStart >> 3) > 32)
	    i |= 0x20;
	j = (vganew->CRTC[0] + ((i & 0x01) << 8) +
	     vganew->CRTC[4] + ((i & 0x10) << 4) + 1) / 2;
	if (j - (vganew->CRTC[4] + ((i & 0x10) << 4)) < 4) {
	    if (vganew->CRTC[4] + ((i & 0x10) << 4) + 4 <= 
	        vganew->CRTC[0] + ((i & 0x01) << 8))
		j = vganew->CRTC[4] + ((i & 0x10) << 4) + 4;
	    else
		j = vganew->CRTC[0] + ((i & 0x01) << 8) + 1;
	}

	new->CR3B = j & 0xff;
	i |= (j & 0x100) >> 2;
	new->CR3C = (vganew->CRTC[0] + ((i & 0x01) << 8)) / 2;
	new->CR5D = i;
	new->CR5E = (((mode->CrtcVTotal - 2) & 0x400) >> 10) |
		    (((mode->CrtcVDisplay - 1) & 0x400) >> 9) |
		    (((mode->CrtcVSyncStart) & 0x400) >> 8) |
		    (((mode->CrtcVSyncStart) & 0x400) >> 6) | 0x40;
	width = (pScrn->displayWidth * (pScrn->bitsPerPixel / 8)) >> 3;
	new->CR91 = vganew->CRTC[19] = 0xff & width;
	new->CR51 = (0x300 & width) >> 4;
	new->CR90 = 0x80 | (width >> 8);
	vganew->MiscOutReg |= 0x0c;

	/* Set frame buffer description. */

	if (pScrn->bitsPerPixel <= 8)
	    new->CR50 = 0;
	else if (pScrn->bitsPerPixel <= 16)
	    new->CR50 = 0x10;
	else
	    new->CR50 = 0x30;

	if (pScrn->displayWidth <= 640)
	    new->CR50 |= 0x40;
	else if (pScrn->displayWidth <= 800)
	    new->CR50 |= 0x80;
	else if (pScrn->displayWidth <= 1024)
	    new->CR50 |= 0x00;
	else if (pScrn->displayWidth <= 1152)
	    new->CR50 |= 0x01;
	else if (pScrn->displayWidth <= 1280)
	    new->CR50 |= 0xc0;
	else if (pScrn->displayWidth <= 1600)
	    new->CR50 |= 0x81;
	else
	    new->CR50 |= 0xc1;	/* Use GBD */

	if( psav->Chipset == S3_SAVAGE2000 )
	    new->CR33 = 0x08;
	else
	    new->CR33 = 0x20;
	     
	vganew->CRTC[0x17] = 0xeb;

	new->CR67 |= 1;

	VGAOUT8(vgaCRIndex, 0x36);
	new->CR36 = VGAIN8(vgaCRReg);
	VGAOUT8(vgaCRIndex, 0x68);
	new->CR68 = VGAIN8(vgaCRReg);
	new->CR69 = 0;
	VGAOUT8(vgaCRIndex, 0x6f);
	new->CR6F = VGAIN8(vgaCRReg);
	VGAOUT8(vgaCRIndex, 0x88);
	new->CR86 = VGAIN8(vgaCRReg) | 0x08;
	VGAOUT8(vgaCRIndex, 0xb0);
	new->CRB0 = VGAIN8(vgaCRReg) | 0x80;
    }

    pScrn->vtSema = TRUE;

    /* do it! */
    SavageWriteMode(pScrn, vganew, new);
    SavageAdjustFrame(pScrn->scrnIndex, pScrn->frameX0, pScrn->frameY0, 0);

    return TRUE;
}


static Bool SavageCloseScreen(int scrnIndex, ScreenPtr pScreen)
{
    ScrnInfoPtr pScrn = xf86Screens[scrnIndex];
    vgaHWPtr hwp = VGAHWPTR(pScrn);
    SavagePtr psav = SAVPTR(pScrn);
    vgaRegPtr vgaSavePtr = &hwp->SavedReg;
    SavageRegPtr SavageSavePtr = &psav->SavedReg;

    if( psav->AccelInfoRec ) {
        XAADestroyInfoRec( psav->AccelInfoRec );
	psav->AccelInfoRec = NULL;
    }

#if 0
    if( psav->pInt10 ) {
        xf86FreeInt10( psav->pInt10 );
	psav->pInt10 = NULL;
    }

    if (psav->pVbe)
    {
	vbeFree(psav->pVbe);
	psav->pVbe = NULL;
    }
#endif

    if (pScrn->vtSema) {
	SavageWriteMode(pScrn, vgaSavePtr, SavageSavePtr);
	vgaHWLock(hwp);
	SavageUnmapMem(pScrn);
    }

    pScrn->vtSema = FALSE;
    pScreen->CloseScreen = psav->CloseScreen;

    return (*pScreen->CloseScreen)(scrnIndex, pScreen);
}


static Bool SavageSaveScreen(ScreenPtr pScreen, int mode)
{
    return vgaHWSaveScreen(pScreen, mode);
}


void SavageAdjustFrame(int scrnIndex, int x, int y, int flags)
{
    ScrnInfoPtr pScrn = xf86Screens[scrnIndex];
    vgaHWPtr hwp = VGAHWPTR(pScrn);
    SavagePtr psav = SAVPTR(pScrn);
    int Base;
    int vgaCRIndex, vgaCRReg, vgaIOBase;
    vgaIOBase = hwp->IOBase;
    vgaCRIndex = vgaIOBase + 4;
    vgaCRReg = vgaIOBase + 5;

    if (psav->ShowCache && y)
	y += pScrn->virtualY - 1;

    Base = ((y * pScrn->displayWidth + x) *
	    (pScrn->bitsPerPixel / 8)) >> 2;
    /* now program the start address registers */
    VGAOUT16(vgaCRIndex, (Base & 0x00ff00) | 0x0c);
    VGAOUT16(vgaCRIndex, ((Base & 0x00ff) << 8) | 0x0d);
    VGAOUT8(vgaCRIndex, 0x69);
    VGAOUT8(vgaCRReg, (Base & 0x7f0000) >> 16);

    return;
}


Bool SavageSwitchMode(int scrnIndex, DisplayModePtr mode, int flags)
{
    return SavageModeInit(xf86Screens[scrnIndex], mode);
}


void SavageEnableMMIO(ScrnInfoPtr pScrn)
{
    vgaHWPtr hwp = VGAHWPTR(pScrn);
    SavagePtr psav = SAVPTR(pScrn);
    int vgaCRIndex, vgaCRReg;
    unsigned char val;

    vgaHWSetMmioFuncs(hwp, psav->MapBase, 0x8000);
    val = VGAIN8(0x3c3);
    VGAOUT8(0x3c3, val | 0x01);
    val = VGAIN8(VGA_MISC_OUT_R);
    VGAOUT8(VGA_MISC_OUT_W, val | 0x01);
    vgaHWGetIOBase(hwp);
    vgaCRIndex = hwp->IOBase + 4;
    vgaCRReg = hwp->IOBase + 5;

    VGAOUT8(vgaCRIndex, 0x40);
    val = VGAIN8(vgaCRReg);
    VGAOUT8(vgaCRReg, val | 1);

    return;
}


void SavageDisableMMIO(ScrnInfoPtr pScrn)
{
    vgaHWPtr hwp = VGAHWPTR(pScrn);
    SavagePtr psav = SAVPTR(pScrn);
    int vgaCRIndex, vgaCRReg;
    unsigned char val;

    vgaHWGetIOBase(hwp);
    vgaCRIndex = hwp->IOBase + 4;
    vgaCRReg = hwp->IOBase + 5;

    VGAOUT8(vgaCRIndex, 0x40);
    val = VGAIN8(vgaCRReg);
    VGAOUT8(vgaCRReg, val | 1);

    return;
}


void SavageLoadPalette(ScrnInfoPtr pScrn, int numColors, int *indicies,
		       LOCO *colors, VisualPtr pVisual)
{
    SavagePtr psav = SAVPTR(pScrn);
    int i, index;

    for (i=0; i<numColors; i++) {
	index = indicies[i];
	VGAOUT8(0x3c8, index);
	VGAOUT8(0x3c9, colors[index].red);
	VGAOUT8(0x3c9, colors[index].green);
	VGAOUT8(0x3c9, colors[index].blue);
    }
}



static void SavageCalcClock(long freq, int min_m, int min_n1, int max_n1,
			   int min_n2, int max_n2, long freq_min,
			   long freq_max, unsigned int *mdiv,
			   unsigned int *ndiv, unsigned int *r)
{
    double ffreq, ffreq_min, ffreq_max;
    double div, diff, best_diff;
    unsigned int m;
    unsigned char n1, n2, best_n1=16+2, best_n2=2, best_m=125+2;

    ffreq = freq / 1000.0 / BASE_FREQ;
    ffreq_max = freq_max / 1000.0 / BASE_FREQ;
    ffreq_min = freq_min / 1000.0 / BASE_FREQ;

    if (ffreq < ffreq_min / (1 << max_n2)) {
	    ErrorF("invalid frequency %1.3f Mhz\n",
		   ffreq*BASE_FREQ);
	    ffreq = ffreq_min / (1 << max_n2);
    }
    if (ffreq > ffreq_max / (1 << min_n2)) {
	    ErrorF("invalid frequency %1.3f Mhz\n",
		   ffreq*BASE_FREQ);
	    ffreq = ffreq_max / (1 << min_n2);
    }

    /* work out suitable timings */

    best_diff = ffreq;

    for (n2=min_n2; n2<=max_n2; n2++) {
	for (n1=min_n1+2; n1<=max_n1+2; n1++) {
	    m = (int)(ffreq * n1 * (1 << n2) + 0.5);
	    if (m < min_m+2 || m > 127+2)
		continue;
	    div = (double)(m) / (double)(n1);
	    if ((div >= ffreq_min) &&
		(div <= ffreq_max)) {
		diff = ffreq - div / (1 << n2);
		if (diff < 0.0)
			diff = -diff;
		if (diff < best_diff) {
		    best_diff = diff;
		    best_m = m;
		    best_n1 = n1;
		    best_n2 = n2;
		}
	    }
	}
    }

    *ndiv = best_n1 - 2;
    *r = best_n2;
    *mdiv = best_m - 2;
}


void SavageGEReset(ScrnInfoPtr pScrn, int from_timeout, int line, char *file)
{
    unsigned char cr66;
    int r, success = 0;
    CARD32 fifo_control = 0, miu_control = 0;
    CARD32 streams_timeout = 0, misc_timeout = 0;
    vgaHWPtr hwp = VGAHWPTR(pScrn);
    SavagePtr psav = SAVPTR(pScrn);
    int vgaCRIndex, vgaCRReg, vgaIOBase;

    vgaIOBase = hwp->IOBase;
    vgaCRIndex = vgaIOBase + 4;
    vgaCRReg = vgaIOBase + 5;

    if (from_timeout) {
	if (psav->GEResetCnt++ < 10 || xf86GetVerbosity() > 1)
	    xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
		       "SavageGEReset called from %s line %d\n", file, line);
    } else
	WaitIdleEmpty();

    if (from_timeout) {
	fifo_control = INREG(FIFO_CONTROL_REG);
	miu_control = INREG(MIU_CONTROL_REG);
	streams_timeout = INREG(STREAMS_TIMEOUT_REG);
	misc_timeout = INREG(MISC_TIMEOUT_REG);
    }

    VGAOUT8(vgaCRIndex, 0x66);
    cr66 = VGAIN8(vgaCRReg);

    usleep(10000);
    for (r=1; r<10; r++) {
	VGAOUT8(vgaCRReg, cr66 | 0x02);
	usleep(10000);
	VGAOUT8(vgaCRReg, cr66 & ~0x02);
	usleep(10000);

	if (!from_timeout)
	    WaitIdleEmpty();
	OUTREG(DEST_SRC_STR, psav->Bpl << 16 | psav->Bpl);

	usleep(10000);
	switch(psav->Chipset) {
	    case S3_SAVAGE3D:
	    case S3_SAVAGE_MX:
	      success = (STATUS_WORD0 & 0x0008ffff) == 0x00080000;
	      break;
	    case S3_SAVAGE4:
	    case S3_PROSAVAGE:
	      success = (ALT_STATUS_WORD0 & 0x0081ffff) == 0x00800000;
	      break;
	    case S3_SAVAGE2000:
	      success = (ALT_STATUS_WORD0 & 0x008fffff) == 0;
	      break;
	}	
	if(!success) {
	    usleep(10000);
	    xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
		"restarting S3 graphics engine reset %2d ...\n", r);
	}
	else
	    break;
    }

    if (from_timeout) {
	OUTREG(FIFO_CONTROL_REG, fifo_control);
	OUTREG(MIU_CONTROL_REG, miu_control);
	OUTREG(STREAMS_TIMEOUT_REG, streams_timeout);
	OUTREG(MISC_TIMEOUT_REG, misc_timeout);
    }

    WaitQueue(2);
    OUTREG(SRC_BASE, 0);
    OUTREG(DEST_BASE, 0);

    WaitQueue(4);
    OUTREG(CLIP_L_R, ((0) << 16) | pScrn->displayWidth);
    OUTREG(CLIP_T_B, ((0) << 16) | psav->ScissB);
    OUTREG(MONO_PAT_0, ~0);
    OUTREG(MONO_PAT_1, ~0);
    SavageSetGBD(pScrn);
}



/* This function is used to debug, it prints out the contents of s3 regs */

void
SavagePrintRegs(ScrnInfoPtr pScrn)
{
    SavagePtr psav = SAVPTR(pScrn);
    unsigned char i;
    int vgaCRIndex = 0x3d4;
    int vgaCRReg = 0x3d5;

    ErrorF( "SR    x0 x1 x2 x3 x4 x5 x6 x7 x8 x9 xA xB xC xD xE xF" );

    for( i = 0; i < 0x70; i++ ) {
	if( !(i % 16) )
	    ErrorF( "\nSR%xx ", i >> 4 );
	VGAOUT8( 0x3c4, i );
	ErrorF( " %02x", VGAIN8(0x3c5) );
    }

    ErrorF( "\n\nCR    x0 x1 x2 x3 x4 x5 x6 x7 x8 x9 xA xB xC xD xE xF" );

    for( i = 0; i < 0xB7; i++ ) {
	if( !(i % 16) )
	    ErrorF( "\nCR%xx ", i >> 4 );
	VGAOUT8( vgaCRIndex, i );
	ErrorF( " %02x", VGAIN8(vgaCRReg) );
    }

    ErrorF("\n\n");
}


#ifdef DPMSExtension
static void SavageDPMS(ScrnInfoPtr pScrn, int mode, int flags)
{
    SavagePtr psav = SAVPTR(pScrn);
    unsigned char sr8 = 0x00, srd = 0x00;

    VGAOUT8(0x3c4, 0x08);
    sr8 = VGAIN8(0x3c5);
    sr8 |= 0x06;
    VGAOUT8(0x3c5, sr8);

    VGAOUT8(0x3c4, 0x0d);
    srd = VGAIN8(0x3c5);

    srd &= 0x03;

    switch (mode) {
	case DPMSModeOn:
	    break;
	case DPMSModeStandby:
	    srd |= 0x10;
	    break;
	case DPMSModeSuspend:
	    srd |= 0x40;
	    break;
	case DPMSModeOff:
	    srd |= 0x50;
	    break;
	default:
	    xf86DrvMsg(pScrn->scrnIndex, X_ERROR, "Invalid DPMS mode %d\n", mode);
	    break;
    }

    VGAOUT8(0x3c4, 0x0d);
    VGAOUT8(0x3c5, srd);

    return;
}
#endif /* DPMSExtension */

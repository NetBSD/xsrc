/*
 * Copyright 1997,1998 by Alan Hourihane, Wigan, England.
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
 * Authors:  Alan Hourihane, <alanh@fairlite.demon.co.uk>
 *           Dirk Hohndel, <hohndel@suse.de>
 *	     Stefan Dirsch, <sndirsch@suse.de>
 *	     Michel Dänzer, <michdaen@iiic.ethz.ch>
 *	     Sven Luther, <luther@dpt-info.u-strasbg.fr>
 *
 * this work is sponsored by S.u.S.E. GmbH, Fuerth, Elsa GmbH, Aachen, 
 * Siemens Nixdorf Informationssysteme and Appian Graphics.
 */
/* $XFree86: xc/programs/Xserver/hw/xfree86/drivers/glint/glint_driver.c,v 1.103 2000/12/08 13:42:35 alanh Exp $ */

#include "fb.h"
#include "cfb8_32.h"
#include "micmap.h"
#include "xf86.h"
#include "xf86_OSproc.h"
#include "xf86_ansic.h"
#include "xf86Version.h"
#include "xf86PciInfo.h"
#include "xf86Pci.h"
#include "xf86cmap.h"
#include "shadowfb.h"
#include "fbdevhw.h"
#include "vgaHW.h"
#include "xf86RAC.h"
#include "xf86Resources.h"
#include "xf86int10.h"
#include "vbe.h"

#include "compiler.h"
#include "mipointer.h"

#include "mibstore.h"

#include "glint_regs.h"
#include "IBM.h"
#include "TI.h"
#include "glint.h"

#ifdef XFreeXDGA
#define _XF86DGA_SERVER_
#include "extensions/xf86dgastr.h"
#endif

#ifdef DPMSExtension
#include "globals.h"
#define DPMS_SERVER
#include "extensions/dpms.h"
#endif

#define DEBUG 0

#if DEBUG
# define TRACE_ENTER(str)       ErrorF("glint: " str " %d\n",pScrn->scrnIndex)
# define TRACE_EXIT(str)        ErrorF("glint: " str " done\n")
# define TRACE(str)             ErrorF("glint trace: " str "\n")
#else
# define TRACE_ENTER(str)
# define TRACE_EXIT(str)
# define TRACE(str)
#endif

static OptionInfoPtr	GLINTAvailableOptions(int chipid, int busid);
static void	GLINTIdentify(int flags);
static Bool	GLINTProbe(DriverPtr drv, int flags);
static Bool	GLINTPreInit(ScrnInfoPtr pScrn, int flags);
static Bool	GLINTScreenInit(int Index, ScreenPtr pScreen, int argc,
			      char **argv);
static Bool	GLINTEnterVT(int scrnIndex, int flags);
static void	GLINTLeaveVT(int scrnIndex, int flags);
static Bool	GLINTCloseScreen(int scrnIndex, ScreenPtr pScreen);
static Bool	GLINTSaveScreen(ScreenPtr pScreen, int mode);

/* Optional functions */
static void	GLINTFreeScreen(int scrnIndex, int flags);
static int	GLINTValidMode(int scrnIndex, DisplayModePtr mode, Bool verbose,
			     int flags);

/* Internally used functions */
static Bool	GLINTMapMem(ScrnInfoPtr pScrn);
static Bool	GLINTUnmapMem(ScrnInfoPtr pScrn);
static void	GLINTSave(ScrnInfoPtr pScrn);
static void	GLINTRestore(ScrnInfoPtr pScrn);
static Bool	GLINTModeInit(ScrnInfoPtr pScrn, DisplayModePtr mode);

/*
 * This is intentionally screen-independent.  It indicates the binding
 * choice made in the first PreInit.
 */
static int pix24bpp = 0;
static Bool FBDevProbed = FALSE;
 
/* 
 * This contains the functions needed by the server after loading the driver
 * module.  It must be supplied, and gets passed back by the SetupProc
 * function in the dynamic case.  In the static case, a reference to this
 * is compiled in, and this requires that the name of this DriverRec be
 * an upper-case version of the driver name.
 */

DriverRec GLINT = {
    VERSION,
    GLINT_DRIVER_NAME,
#if 0
    "accelerated driver for 3dlabs and derived chipsets",
#endif
    GLINTIdentify,
    GLINTProbe,
    GLINTAvailableOptions,
    NULL,
    0
};

static SymTabRec GLINTChipsets[] = {
    { PCI_VENDOR_3DLABS_CHIP_GAMMA,		"gamma" },
    { PCI_VENDOR_TI_CHIP_PERMEDIA2,		"ti_pm2" },
    { PCI_VENDOR_TI_CHIP_PERMEDIA,		"ti_pm" },
    { PCI_VENDOR_3DLABS_CHIP_PERMEDIA3,		"pm3" },
    { PCI_VENDOR_3DLABS_CHIP_PERMEDIA2V,	"pm2v" },
    { PCI_VENDOR_3DLABS_CHIP_PERMEDIA2,		"pm2" },
    { PCI_VENDOR_3DLABS_CHIP_PERMEDIA,		"pm" },
    { PCI_VENDOR_3DLABS_CHIP_300SX,		"300sx" },
    { PCI_VENDOR_3DLABS_CHIP_500TX,		"500tx" },
    { PCI_VENDOR_3DLABS_CHIP_MX,		"mx" },
/*
    { PCI_VENDOR_3DLABS_CHIP_DELTA,		"delta" },
*/
    { -1,					NULL }
};

static PciChipsets GLINTPciChipsets[] = {
    { PCI_VENDOR_3DLABS_CHIP_GAMMA,	 PCI_VENDOR_3DLABS_CHIP_GAMMA,	    NULL },
    { PCI_VENDOR_TI_CHIP_PERMEDIA2,	 PCI_VENDOR_TI_CHIP_PERMEDIA2,	    RES_SHARED_VGA },
    { PCI_VENDOR_TI_CHIP_PERMEDIA,	 PCI_VENDOR_TI_CHIP_PERMEDIA,	    NULL },
    { PCI_VENDOR_3DLABS_CHIP_PERMEDIA3, PCI_VENDOR_3DLABS_CHIP_PERMEDIA3, RES_SHARED_VGA },
    { PCI_VENDOR_3DLABS_CHIP_PERMEDIA2V, PCI_VENDOR_3DLABS_CHIP_PERMEDIA2V, RES_SHARED_VGA },
    { PCI_VENDOR_3DLABS_CHIP_PERMEDIA2,	 PCI_VENDOR_3DLABS_CHIP_PERMEDIA2,  RES_SHARED_VGA },
    { PCI_VENDOR_3DLABS_CHIP_PERMEDIA,	 PCI_VENDOR_3DLABS_CHIP_PERMEDIA,   NULL },
    { PCI_VENDOR_3DLABS_CHIP_300SX,	 PCI_VENDOR_3DLABS_CHIP_300SX,	    NULL },
    { PCI_VENDOR_3DLABS_CHIP_500TX,	 PCI_VENDOR_3DLABS_CHIP_500TX,	    NULL },
    { PCI_VENDOR_3DLABS_CHIP_MX,	 PCI_VENDOR_3DLABS_CHIP_MX,	    NULL },
/*
    { PCI_VENDOR_3DLABS_CHIP_DELTA,	 PCI_VENDOR_3DLABS_CHIP_DELTA,	    NULL },
*/
    { -1,				 -1,				    RES_UNDEFINED }
};


typedef enum {
    OPTION_SW_CURSOR,
    OPTION_HW_CURSOR,
    OPTION_PCI_RETRY,
    OPTION_RGB_BITS,
    OPTION_NOACCEL,
    OPTION_BLOCK_WRITE,
    OPTION_FIREGL3000,
    OPTION_MEM_CLK,
    OPTION_OVERLAY,
    OPTION_SHADOW_FB,
    OPTION_FBDEV,
    OPTION_NOWRITEBITMAP,
    OPTION_PM3_USE_GAMMA
} GLINTOpts;

static OptionInfoRec GLINTOptions[] = {
    { OPTION_PM3_USE_GAMMA,	"UseGamma",	OPTV_BOOLEAN,	{0}, FALSE },
    { OPTION_SW_CURSOR,		"SWcursor",	OPTV_BOOLEAN,	{0}, FALSE },
    { OPTION_HW_CURSOR,		"HWcursor",	OPTV_BOOLEAN,	{0}, FALSE },
    { OPTION_PCI_RETRY,		"PciRetry",	OPTV_BOOLEAN,	{0}, FALSE },
    { OPTION_RGB_BITS,		"RGBbits",	OPTV_INTEGER,	{0}, FALSE },
    { OPTION_NOACCEL,		"NoAccel",	OPTV_BOOLEAN,	{0}, FALSE },
    { OPTION_BLOCK_WRITE,	"BlockWrite",	OPTV_BOOLEAN,   {0}, FALSE },
    { OPTION_FIREGL3000,	"FireGL3000",   OPTV_BOOLEAN,	{0}, FALSE },
    { OPTION_MEM_CLK,		"SetMClk",	OPTV_FREQ,	{0}, FALSE },
    { OPTION_OVERLAY,		"Overlay",	OPTV_ANYSTR,	{0}, FALSE },
    { OPTION_SHADOW_FB,		"ShadowFB",	OPTV_BOOLEAN,	{0}, FALSE },
    { OPTION_FBDEV,		"UseFBDev",	OPTV_BOOLEAN,	{0}, FALSE },
    { -1,			NULL,		OPTV_NONE,	{0}, FALSE }
};

static RamDacSupportedInfoRec IBMRamdacs[] = {
    { IBM526DB_RAMDAC },
    { IBM526_RAMDAC },
    { IBM640_RAMDAC },
    { -1 }
};

static RamDacSupportedInfoRec TIRamdacs[] = {
    { TI3030_RAMDAC },
    { TI3026_RAMDAC },
    { -1 }
};


static const char *vgahwSymbols[] = {
    "vgaHWGetIndex",
    "vgaHWSave",
    "vgaHWRestore",
    "vgaHWGetHWRec",
    "vgaHWUnlock",
    "vgaHWInit",
    "vgaHWProtect",
    "vgaHWSetMmioFuncs",
    "vgaHWGetIOBase",
    "vgaHWMapMem",
    "vgaHWLock",
    "vgaHWFreeHWRec",
    "vgaHWSaveScreen",
    "vgaHWddc1SetSpeed",
    NULL
};

static const char *xf8_32bppSymbols[] = {
    "xf86Overlay8Plus32Init",
    NULL
};

static const char *xaaSymbols[] = {
    "XAADestroyInfoRec",
    "XAACreateInfoRec",
    "XAAInit",
    "XAAStippleScanlineFuncLSBFirst",
    "XAAOverlayFBfuncs",
    "XAACachePlanarMonoStipple",
    "XAAScreenIndex",
    "XAAPolyLines",
    "XAAPolySegment",
    NULL
};

static const char *fbSymbols[] = {
    "cfb8_32ScreenInit",
#ifdef RENDER
    "fbPictureInit",
#endif
    "fbScreenInit",
    "fbBres",
    NULL
};

static const char *ddcSymbols[] = {
    "xf86PrintEDID",
    "xf86DoEDID_DDC1",
    "xf86DoEDID_DDC2",
    NULL
};

static const char *i2cSymbols[] = {
    "xf86CreateI2CBusRec",
    "xf86DestroyI2CBusRec",
    "xf86I2CBusInit",
    "xf86I2CDevInit",
    "xf86I2CProbeAddress",
    "xf86I2CWriteByte",
    "xf86I2CWriteVec",    
    NULL
};

static const char *shadowSymbols[] = {
    "ShadowFBInit",
    NULL
};

static const char *vbeSymbols[] = {
    "VBEInit",
    "vbeDoEDID",
    NULL
};

static const char *fbdevHWSymbols[] = {
	"fbdevHWInit",
	"fbdevHWFreeRec",
	"fbdevHWProbe",
	"fbdevHWFreeRec",
	"fbdevHWGetName",
	"fbdevHWUseBuildinMode",

	"fbdevHWGetDepth",
	"fbdevHWGetVidmem",

	/* colormap */
	"fbdevHWLoadPalette",

	/* ScrnInfo hooks */
	"fbdevHWSwitchMode",
	"fbdevHWAdjustFrame",
	"fbdevHWEnterVT",
	"fbdevHWLeaveVT",
	"fbdevHWValidMode",
	"fbdevHWRestore",
	"fbdevHWModeInit",
	"fbdevHWSave",
	"fbdevHWUnmapMMIO",
	"fbdevHWUnmapVidmem",
	"fbdevHWMapMMIO",
	"fbdevHWMapVidmem",
	
	NULL
};

static const char *int10Symbols[] = {
    "xf86FreeInt10",
    "xf86InitInt10",
    NULL
};

#ifdef XF86DRI
static const char *drmSymbols[] = {
    "drmAddBufs",
    "drmAddMap",
    "drmCtlAddCommand",
    "drmCtlInstHandler",
    "drmGetInterruptFromBusID",
    "drmMapBufs",
    "drmMarkBufs",
    "drmUnmapBufs",
    "drmFreeVersion",
    "drmGetVersion",
    NULL
};

static const char *driSymbols[] = {
    "DRIGetDrawableIndex",
    "DRIFinishScreenInit",
    "DRIDestroyInfoRec",
    "DRICloseScreen",
    "DRIDestroyInfoRec",
    "DRIScreenInit",
    "DRIDestroyInfoRec",
    "DRICreateInfoRec",
    "DRIQueryVersion",
    "GlxSetVisualConfigs",
    NULL
};
#endif

#ifdef XFree86LOADER

static MODULESETUPPROTO(glintSetup);

static XF86ModuleVersionInfo glintVersRec =
{
	"glint",
	MODULEVENDORSTRING,
	MODINFOSTRING1,
	MODINFOSTRING2,
	XF86_VERSION_CURRENT,
	GLINT_MAJOR_VERSION, GLINT_MINOR_VERSION, GLINT_PATCHLEVEL,
	ABI_CLASS_VIDEODRV,			/* This is a video driver */
	ABI_VIDEODRV_VERSION,
	MOD_CLASS_VIDEODRV,
	{0,0,0,0}
};

XF86ModuleData glintModuleData = { &glintVersRec, glintSetup, NULL };

pointer
glintSetup(pointer module, pointer opts, int *errmaj, int *errmin)
{
    static Bool setupDone = FALSE;

    if (!setupDone) {
	setupDone = TRUE;
	xf86AddDriver(&GLINT, module, 0);
	LoaderRefSymLists(vgahwSymbols, fbSymbols, ddcSymbols, i2cSymbols,
			  xaaSymbols, xf8_32bppSymbols,
			  shadowSymbols, fbdevHWSymbols, int10Symbols,
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

#define PARTPROD(a,b,c) (((a)<<6) | ((b)<<3) | (c))

static char bppand[4] = { 0x03, /* 8bpp */
			  0x01, /* 16bpp */
			  0x00, /* 24bpp */
			  0x00  /* 32bpp */};

static int partprod500TX[] = {
	-1,
	PARTPROD(0,0,1), PARTPROD(0,0,2), PARTPROD(0,1,2), PARTPROD(0,0,3),
	PARTPROD(0,1,3), PARTPROD(0,2,3), PARTPROD(1,2,3), PARTPROD(0,0,4),
	PARTPROD(0,1,4), PARTPROD(0,2,4), PARTPROD(1,2,4), PARTPROD(0,3,4),
	PARTPROD(1,3,4), PARTPROD(2,3,4),              -1, PARTPROD(0,0,5), 
	PARTPROD(0,1,5), PARTPROD(0,2,5), PARTPROD(1,2,5), PARTPROD(0,3,5), 
	PARTPROD(1,3,5), PARTPROD(2,3,5),              -1, PARTPROD(0,4,5), 
	PARTPROD(1,4,5), PARTPROD(2,4,5), PARTPROD(3,4,5),              -1,
	             -1,              -1,              -1, PARTPROD(0,0,6), 
	PARTPROD(0,1,6), PARTPROD(0,2,6), PARTPROD(1,2,6), PARTPROD(0,3,6), 
	PARTPROD(1,3,6), PARTPROD(2,3,6),              -1, PARTPROD(0,4,6), 
	PARTPROD(1,4,6), PARTPROD(2,4,6),              -1, PARTPROD(3,4,6),
	             -1,              -1,              -1, PARTPROD(0,5,6), 
	PARTPROD(1,5,6), PARTPROD(2,5,6),              -1, PARTPROD(3,5,6), 
	             -1,              -1,              -1, PARTPROD(4,5,6), 
	             -1,              -1,              -1,              -1,
		     -1,              -1,              -1, PARTPROD(0,0,7), 
	             -1, PARTPROD(0,2,7), PARTPROD(1,2,7), PARTPROD(0,3,7), 
	PARTPROD(1,3,7), PARTPROD(2,3,7),              -1, PARTPROD(0,4,7),
	PARTPROD(1,4,7), PARTPROD(2,4,7),              -1, PARTPROD(3,4,7), 
	             -1,              -1,              -1, PARTPROD(0,5,7),
	PARTPROD(1,5,7), PARTPROD(2,5,7),              -1, PARTPROD(3,5,7), 
	             -1,              -1,              -1, PARTPROD(4,5,7),
	             -1,              -1,              -1,              -1,
		     -1,              -1,              -1, PARTPROD(0,6,7), 
	PARTPROD(1,6,7), PARTPROD(2,6,7),              -1, PARTPROD(3,6,7),
	             -1,              -1,              -1, PARTPROD(4,6,7), 
	             -1,              -1,              -1,              -1,
		     -1,              -1,              -1, PARTPROD(5,6,7), 
	             -1,              -1,              -1,              -1,
		     -1,              -1,              -1,              -1,
		     -1,              -1,              -1,              -1,
		     -1,              -1,              -1, PARTPROD(0,7,7),
		      0};

int partprodPermedia[] = {
	-1,
	PARTPROD(0,0,1), PARTPROD(0,1,1), PARTPROD(1,1,1), PARTPROD(1,1,2),
	PARTPROD(1,2,2), PARTPROD(2,2,2), PARTPROD(1,2,3), PARTPROD(2,2,3),
	PARTPROD(1,3,3), PARTPROD(2,3,3), PARTPROD(1,2,4), PARTPROD(3,3,3),
	PARTPROD(1,3,4), PARTPROD(2,3,4),              -1, PARTPROD(3,3,4), 
	PARTPROD(1,4,4), PARTPROD(2,4,4),              -1, PARTPROD(3,4,4), 
	             -1, PARTPROD(2,3,5),              -1, PARTPROD(4,4,4), 
	PARTPROD(1,4,5), PARTPROD(2,4,5), PARTPROD(3,4,5),              -1,
	             -1,              -1,              -1, PARTPROD(4,4,5), 
	PARTPROD(1,5,5), PARTPROD(2,5,5),              -1, PARTPROD(3,5,5), 
	             -1,              -1,              -1, PARTPROD(4,5,5), 
	             -1,              -1,              -1, PARTPROD(3,4,6),
	             -1,              -1,              -1, PARTPROD(5,5,5), 
	PARTPROD(1,5,6), PARTPROD(2,5,6),              -1, PARTPROD(3,5,6),
	             -1,              -1,              -1, PARTPROD(4,5,6),
	             -1,              -1,              -1,              -1,
	             -1,              -1,              -1, PARTPROD(5,5,6),
	             -1,              -1,              -1,              -1,
	             -1,              -1,              -1,              -1,
	             -1,              -1,              -1,              -1,
	             -1,              -1,              -1,              -1,
	             -1,              -1,              -1,              -1,
	             -1,              -1,              -1,              -1,
	             -1,              -1,              -1,              -1,
	             -1,              -1,              -1,              -1,
	             -1,              -1,              -1,              -1,
	             -1,              -1,              -1,              -1,
	             -1,              -1,              -1,              -1,
	             -1,              -1,              -1,              -1,
	             -1,              -1,              -1,              -1,
	             -1,              -1,              -1,              -1,
	             -1,              -1,              -1,              -1,
	             -1,              -1,              -1,              -1,
		     0};

#ifdef DPMSExtension
static void
GLINTDisplayPowerManagementSet(ScrnInfoPtr pScrn, int PowerManagementMode,
					int flags)
{
    GLINTPtr pGlint = GLINTPTR(pScrn);
    int videocontrol = 0, vtgpolarity = 0;
    
    if((pGlint->Chipset == PCI_VENDOR_3DLABS_CHIP_500TX) ||
       (pGlint->Chipset == PCI_VENDOR_3DLABS_CHIP_300SX) ||
       (pGlint->Chipset == PCI_VENDOR_3DLABS_CHIP_MX) ||
       (pGlint->Chipset == PCI_VENDOR_3DLABS_CHIP_GAMMA)) {
	vtgpolarity = GLINT_READ_REG(VTGPolarity) & 0xFFFFFFF0;
    } else {
        videocontrol = GLINT_READ_REG(PMVideoControl) & 0xFFFFFFD6;
    }

    switch (PowerManagementMode) {
	case DPMSModeOn:
	    /* Screen: On, HSync: On, VSync: On */
	    videocontrol |= 0x29;
	    vtgpolarity |= 0x05;
	    break;
	case DPMSModeStandby:
	    /* Screen: Off, HSync: Off, VSync: On */
	    videocontrol |= 0x20;
	    vtgpolarity |= 0x04;
	    break;
	case DPMSModeSuspend:
	    /* Screen: Off, HSync: On, VSync: Off */
	    videocontrol |= 0x08;
	    vtgpolarity |= 0x01;
	    break;
	case DPMSModeOff:
	    /* Screen: Off, HSync: Off, VSync: Off */
	    videocontrol |= 0x00;
	    vtgpolarity |= 0x00;
	    break;
	default:
	    return;
    }

    if((pGlint->Chipset == PCI_VENDOR_3DLABS_CHIP_500TX) ||
       (pGlint->Chipset == PCI_VENDOR_3DLABS_CHIP_300SX) ||
       (pGlint->Chipset == PCI_VENDOR_3DLABS_CHIP_MX) ||
       (pGlint->Chipset == PCI_VENDOR_3DLABS_CHIP_GAMMA)) {
    	GLINT_SLOW_WRITE_REG(vtgpolarity, VTGPolarity);
    } else {
    	GLINT_SLOW_WRITE_REG(videocontrol, PMVideoControl);
    }
}
#endif

static Bool
GLINTGetRec(ScrnInfoPtr pScrn)
{
    TRACE_ENTER("GLINTGetRec");
    /*
     * Allocate an GLINTRec, and hook it into pScrn->driverPrivate.
     * pScrn->driverPrivate is initialised to NULL, so we can check if
     * the allocation has already been done.
     */
    if (pScrn->driverPrivate != NULL)
	return TRUE;

    pScrn->driverPrivate = xnfcalloc(sizeof(GLINTRec), 1);
    /* Initialise it */

    TRACE_EXIT("GLINTGetRec");
    return TRUE;
}

static void
GLINTFreeRec(ScrnInfoPtr pScrn)
{
    TRACE_ENTER("GLINTFreeRec");
    if (pScrn->driverPrivate == NULL)
	return;
    xfree(pScrn->driverPrivate);
    pScrn->driverPrivate = NULL;
    TRACE_EXIT("GLINTFreeRec");
}


/* Mandatory */
static void
GLINTIdentify(int flags)
{
    xf86PrintChipsets(GLINT_NAME, "driver for 3Dlabs chipsets", GLINTChipsets);
}

static
OptionInfoPtr
GLINTAvailableOptions(int chipid, int busid)
{
    return GLINTOptions;
}

void
GLINTProbeDDC(ScrnInfoPtr pScrn, int index)
{
    vbeInfoPtr pVbe;
#ifdef XFree86LOADER
    if (xf86LoadSubModule(pScrn, "vbe"))
#endif
    {
	pVbe =  VBEInit(NULL,index);
	vbeDoEDID(pVbe, NULL);
    }
}

/* Mandatory */
static Bool
GLINTProbe(DriverPtr drv, int flags)
{
    int i;
    pciVideoPtr pPci, *checkusedPci;
    GDevPtr *devSections;
    int numDevSections;
    int numUsed,bus,device,func;
    char *dev;
    int *usedChips = NULL;
    Bool foundScreen = FALSE;
    char *name;   

    /* 
    TRACE_ENTER("GLINTProbe");
    */
    TRACE_EXIT("GLINTProbe (Enter)");

  
    if ((numDevSections = xf86MatchDevice(GLINT_DRIVER_NAME,
  					  &devSections)) <= 0) {
  	return FALSE;
    }
  
    checkusedPci = xf86GetPciVideoInfo();
     
    if (checkusedPci == NULL && devSections /* for xf86DoProbe */) {
  	/*
 	 * Changed the behaviour to try probing using the FBDev support
 	 * when no PCI cards have been found. This is for systems without
 	 * (proper) PCI support. (Michel)
  	 */
 	if (!xf86LoadDrvSubModule(drv, "fbdevhw"))
	    return FALSE;
	
 	xf86LoaderReqSymLists(fbdevHWSymbols, NULL);
  	
 	for (i = 0; i < numDevSections; i++) {
 	    dev = xf86FindOptionValue(devSections[i]->options,"fbdev");
 	    if (devSections[i]->busID) {
 		xf86ParsePciBusString(devSections[i]->busID,&bus,&device,&func);
 		if (!xf86CheckPciSlot(bus,device,func))
 		    continue;
 	    }
 	    if (fbdevHWProbe(NULL,dev,&name)) {
 		ScrnInfoPtr pScrn;
 		
  				/* Check for pm2fb */
 		if (strcmp(name,"Permedia2")) continue;
 		foundScreen = TRUE;
 		pScrn = NULL;
 		
 		if (devSections[i]->busID) {
 		    /* XXX what about when there's no busID set? */
 		    int entity;
 		    entity = xf86ClaimPciSlot(bus,device,func,drv,
 					      0,devSections[i],
 					      TRUE);
 		    pScrn = xf86ConfigPciEntity(pScrn,0,entity,
 						      NULL,RES_SHARED_VGA,
 						      NULL,NULL,NULL,NULL);
 		    if (pScrn)
  			xf86DrvMsg(pScrn->scrnIndex, X_CONFIG,
  				   "claimed PCI slot %d:%d:%d\n",bus,device,func);
 		} else {
 		    /* XXX This is a quick hack */
 		    int entity;
 		    
 		    entity = xf86ClaimIsaSlot(drv, 0,
 					      devSections[i], TRUE);
 		    pScrn = xf86ConfigIsaEntity(pScrn,0,entity,
 					      NULL,RES_SHARED_VGA,
 					      NULL,NULL,NULL,NULL);
 		}
 		if (pScrn) {
  		    /* Fill in what we can of the ScrnInfoRec */
 		    xf86DrvMsg(pScrn->scrnIndex, X_INFO,
 			       "%s successfully probed\n", dev ? dev : "default framebuffer device");
  		    pScrn->driverVersion = VERSION;
  		    pScrn->driverName	 = GLINT_DRIVER_NAME;
		    pScrn->name		 = GLINT_NAME;
		    pScrn->Probe	 = GLINTProbe;
		    pScrn->PreInit	 = GLINTPreInit;
		    pScrn->ScreenInit	 = GLINTScreenInit;
		    pScrn->SwitchMode	 = GLINTSwitchMode;
		    pScrn->FreeScreen	 = GLINTFreeScreen;
		    pScrn->EnterVT	 = GLINTEnterVT;
		}
	    }
	}
	
    	xfree(devSections);
	
    } else  if (checkusedPci) {
	
	numUsed = xf86MatchPciInstances(GLINT_NAME, 0,
					GLINTChipsets, GLINTPciChipsets, devSections,
					numDevSections, drv, &usedChips);
	xfree(devSections);
	if (numUsed <= 0)
	    return FALSE;
	foundScreen = TRUE;

	if (!(flags & PROBE_DETECT))
	    for (i = 0; i < numUsed; i++) {
		ScrnInfoPtr pScrn = NULL;
	
		/* Allocate a ScrnInfoRec and claim the slot */
		if ((pScrn = xf86ConfigPciEntity(pScrn, 0, usedChips[i],
					       GLINTPciChipsets, NULL,
					       NULL, NULL, NULL, NULL))) {

		    pPci = xf86GetPciInfoForEntity(usedChips[i]);

		    /* Only claim other chips when GAMMA is used */
		    if (pPci->chipType == PCI_CHIP_GAMMA) {
		      while (*checkusedPci != NULL) {
	    	        int eIndex;
	    	    	/* make sure we claim all but our source device */
	    	    	if ((pPci->bus ==    (*checkusedPci)->bus && 
	          	     pPci->device == (*checkusedPci)->device) &&
	         	     pPci->func !=   (*checkusedPci)->func) {
		    
			    /* Claim other entities on the same card */
			    eIndex = xf86ClaimPciSlot((*checkusedPci)->bus, 
						  (*checkusedPci)->device, 
						  (*checkusedPci)->func,
						  drv, -1 /* XXX */,
						  NULL, FALSE);
			    if (eIndex == -1) {
		    	    	/* This can't happen */
		    	    	FatalError("someone claimed the free slot!\n");
			    }
			    xf86AddEntityToScreen(pScrn,eIndex);
	    	    	} 
	    	    	checkusedPci++;
		      }
		    }

		    /* Fill in what we can of the ScrnInfoRec */
		    pScrn->driverVersion	= VERSION;
		    pScrn->driverName		= GLINT_DRIVER_NAME;
		    pScrn->name			= GLINT_NAME;
		    pScrn->Probe	 	= GLINTProbe;
		    pScrn->PreInit	 	= GLINTPreInit;
		    pScrn->ScreenInit		= GLINTScreenInit;
		    pScrn->SwitchMode		= GLINTSwitchMode;
		    pScrn->FreeScreen		= GLINTFreeScreen;
		    pScrn->EnterVT		= GLINTEnterVT;
		}
	    }
    }

    xfree(usedChips);

    TRACE_EXIT("GLINTProbe");
    return foundScreen;
}
	
/*
 * GetAccelPitchValues -
 *
 * This function returns a list of display width (pitch) values that can
 * be used in accelerated mode.
 */
static int *
GetAccelPitchValues(ScrnInfoPtr pScrn)
{
    int *linePitches = NULL;
    int i, n = 0;
    int *linep = NULL;
    GLINTPtr pGlint = GLINTPTR(pScrn);
	
    switch (pGlint->Chipset) {
    case PCI_VENDOR_TI_CHIP_PERMEDIA2:
    case PCI_VENDOR_TI_CHIP_PERMEDIA:
    case PCI_VENDOR_3DLABS_CHIP_PERMEDIA3:
    case PCI_VENDOR_3DLABS_CHIP_PERMEDIA2V:
    case PCI_VENDOR_3DLABS_CHIP_PERMEDIA2:
    case PCI_VENDOR_3DLABS_CHIP_PERMEDIA:
	linep = &partprodPermedia[0];
	break;
    case PCI_VENDOR_3DLABS_CHIP_500TX:
    case PCI_VENDOR_3DLABS_CHIP_300SX:
    case PCI_VENDOR_3DLABS_CHIP_MX:
    case PCI_VENDOR_3DLABS_CHIP_GAMMA:
	linep = &partprod500TX[0];
	break;
    }

    for (i = 0; linep[i] != 0; i++) {
	if (linep[i] != -1) {
	    n++;
	    linePitches = xnfrealloc(linePitches, n * sizeof(int));
	    linePitches[n - 1] = i << 5;
	}
    }

    /* Mark the end of the list */
    if (n > 0) {
	linePitches = xnfrealloc(linePitches, (n + 1) * sizeof(int));
	linePitches[n] = 0;
    }

    return linePitches;
}

static void
GLINTProbeTIramdac(ScrnInfoPtr pScrn)
{
    GLINTPtr pGlint;
    CARD32 temp = 0;
    pGlint = GLINTPTR(pScrn);

    pGlint->RamDacRec = RamDacCreateInfoRec();
    if (pGlint->numMXDevices == 2) {
	pGlint->RamDacRec->ReadDAC = DUALglintInTIIndReg;
	pGlint->RamDacRec->WriteDAC = DUALglintOutTIIndReg;
	pGlint->RamDacRec->ReadAddress = DUALglintTIReadAddress;
	pGlint->RamDacRec->WriteAddress = DUALglintTIWriteAddress;
	pGlint->RamDacRec->ReadData = DUALglintTIReadData;
	pGlint->RamDacRec->WriteData = DUALglintTIWriteData;
    } else {
	pGlint->RamDacRec->ReadDAC = glintInTIIndReg;
	pGlint->RamDacRec->WriteDAC = glintOutTIIndReg;
	pGlint->RamDacRec->ReadAddress = glintTIReadAddress;
	pGlint->RamDacRec->WriteAddress = glintTIWriteAddress;
	pGlint->RamDacRec->ReadData = glintTIReadData;
	pGlint->RamDacRec->WriteData = glintTIWriteData;
    }
    pGlint->RamDacRec->LoadPalette = TIramdacLoadPalette;

    if(!RamDacInit(pScrn, pGlint->RamDacRec)) {
	RamDacDestroyInfoRec(pGlint->RamDacRec);
	return;
    }
    GLINTMapMem(pScrn);
    if (pGlint->numMXDevices == 2) {
    	temp = GLINT_READ_REG(GCSRAperture);
    	GLINT_SLOW_WRITE_REG(GCSRSecondaryGLINTMapEn, GCSRAperture);
    }
    pGlint->RamDac = TIramdacProbe(pScrn, TIRamdacs);
    if (pGlint->numMXDevices == 2) {
    	GLINT_SLOW_WRITE_REG(temp, GCSRAperture);
    }
    GLINTUnmapMem(pScrn);
}

static void
GLINTProbeIBMramdac(ScrnInfoPtr pScrn)
{
    GLINTPtr pGlint;
    pGlint = GLINTPTR(pScrn);
    pGlint->RamDacRec = RamDacCreateInfoRec();
    pGlint->RamDacRec->ReadDAC = glintInIBMRGBIndReg;
    pGlint->RamDacRec->WriteDAC = glintOutIBMRGBIndReg;
    pGlint->RamDacRec->ReadAddress = glintIBMReadAddress;
    pGlint->RamDacRec->WriteAddress = glintIBMWriteAddress;
    pGlint->RamDacRec->ReadData = glintIBMReadData;
    pGlint->RamDacRec->WriteData = glintIBMWriteData;
    pGlint->RamDacRec->LoadPalette = NULL;
    if(!RamDacInit(pScrn, pGlint->RamDacRec)) {
	RamDacDestroyInfoRec(pGlint->RamDacRec);
	return;
    }
    GLINTMapMem(pScrn);
    pGlint->RamDac = IBMramdacProbe(pScrn, IBMRamdacs);
    GLINTUnmapMem(pScrn);
}

/* Mandatory */
static Bool
GLINTPreInit(ScrnInfoPtr pScrn, int flags)
{
    GLINTPtr pGlint;
    MessageType from;
    int i;
    int LinearFramebuffer = 0;
    double real;
    Bool Overlay = FALSE;
    int maxwidth = 0, maxheight = 0;
    ClockRangePtr clockRanges;
    char *mod = NULL;
    const char *s;

    TRACE_ENTER("GLINTPreInit");

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

    /* Check the number of entities, and fail if it isn't one or more. */
    if (pScrn->numEntities < 1)
	return FALSE;

    /* Allocate the GLINTRec driverPrivate */
    if (!GLINTGetRec(pScrn)) {
	return FALSE;
    }
    pGlint = GLINTPTR(pScrn);

    /* Get the entities, and make sure they are PCI. */
    pGlint->pEnt = xf86GetEntityInfo(pScrn->entityList[0]);

    if (flags & PROBE_DETECT) {
	GLINTProbeDDC(pScrn, pGlint->pEnt->index);
	return TRUE;
    }

    if (pGlint->pEnt->location.type == BUS_PCI)
    {
    	/* Initialize the card through int10 interface if needed */
    	if ( xf86LoadSubModule(pScrn, "int10")){
            xf86Int10InfoPtr pInt;

            xf86LoaderReqSymLists(int10Symbols, NULL);
            xf86DrvMsg(pScrn->scrnIndex, X_INFO, "Initializing int10\n");
            pInt = xf86InitInt10(pGlint->pEnt->index);
            xf86FreeInt10(pInt);
    	}
    
        pGlint->PciInfo = xf86GetPciInfoForEntity(pGlint->pEnt->index);
        pGlint->PciTag = pciTag(pGlint->PciInfo->bus, pGlint->PciInfo->device,
			    pGlint->PciInfo->func);
    }

    if (pScrn->numEntities > 1) {
	pciVideoPtr pPci;
	EntityInfoPtr pEnt;

	for (i = 1; i < pScrn->numEntities; i++) {
	    pEnt = xf86GetEntityInfo(pScrn->entityList[i]);
	    pPci = xf86GetPciInfoForEntity(pEnt->index);
	    if (pPci->chipType == PCI_CHIP_MX) {
		if (pGlint->numMXDevices >= GLINT_MAX_MX_DEVICES) {
		    xf86DrvMsg(pScrn->scrnIndex, X_WARNING,
			"%d MX chips unsupported, aborting. (Max - 2)\n",
			pGlint->numMXDevices);
		    return FALSE;
		} else {
		    LinearFramebuffer = pPci->memBase[2];
		    pGlint->MXPciInfo[pGlint->numMXDevices] = pPci;
		    pGlint->numMXDevices++;
		}
	    }
	}
    }

    /*
     * VGA isn't used, so mark it so.  XXX Should check if any VGA resources
     * are decoded or not, and if not, change them from Unused to Disabled.
     * Mem resources seem to be disabled. This is importand to avoid conflicts
     * with DGA
     */
    xf86SetOperatingState(resVgaMemShared, pGlint->pEnt->index, ResDisableOpr);
    xf86SetOperatingState(resVgaIoShared, pGlint->pEnt->index, ResUnusedOpr);
    
    /* Operations for which memory access is required. */
    pScrn->racMemFlags = RAC_FB | RAC_COLORMAP | RAC_CURSOR | RAC_VIEWPORT;
    pScrn->racIoFlags = RAC_FB | RAC_COLORMAP | RAC_CURSOR | RAC_VIEWPORT;

    /* Set pScrn->monitor */
    pScrn->monitor = pScrn->confScreen->monitor;
    /*
     * The first thing we should figure out is the depth, bpp, etc.
     * Our default depth is 8, so pass it to the helper function.
     * We support both 24bpp and 32bpp layouts, so indicate that.
     */
    if (FBDevProbed) {
	int default_depth;
	
	if (!fbdevHWInit(pScrn,NULL,xf86FindOptionValue(pGlint->pEnt->device->options,"fbdev"))) {
		xf86DrvMsg(pScrn->scrnIndex, X_ERROR, "fbdevHWInit failed!\n");	
		return FALSE;
	}
	default_depth = fbdevHWGetDepth(pScrn);
	if (!xf86SetDepthBpp(pScrn, default_depth, default_depth, default_depth,
			     Support24bppFb | Support32bppFb))
		return FALSE;
    } else {
	if (!xf86SetDepthBpp(pScrn, 8, 0, 0, Support24bppFb | Support32bppFb 
	 	/*| SupportConvert32to24 | PreferConvert32to24*/))
		return FALSE;
    }
    /* Check that the returned depth is one we support */
    switch (pScrn->depth) {
    case 8:
    case 15:
    case 16:
    case 24:
    case 30:
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
     * If the driver can do gamma correction, it should call xf86SetGamma()
     * here.
     */

    {
	Gamma zeros = {0.0, 0.0, 0.0};

	if (!xf86SetGamma(pScrn, zeros)) {
	    return FALSE;
	}
    }

    /* We use a programamble clock */
    pScrn->progClock = TRUE;

    /* Collect all of the relevant option flags (fill in pScrn->options) */
    xf86CollectOptions(pScrn, NULL);

    /* Process the options */
    xf86ProcessOptions(pScrn->scrnIndex, pScrn->options, GLINTOptions);

    /* Default to 8bits per RGB */
    if (pScrn->depth == 30)  pScrn->rgbBits = 10;	
    			else pScrn->rgbBits = 8;
    if (xf86GetOptValInteger(GLINTOptions, OPTION_RGB_BITS, &pScrn->rgbBits)) {
	xf86DrvMsg(pScrn->scrnIndex, X_CONFIG, "Bits per RGB set to %d\n",
		       pScrn->rgbBits);
    }

    if (xf86GetOptValFreq(GLINTOptions, OPTION_MEM_CLK, OPTUNITS_MHZ, &real)) {
	pGlint->MemClock = (int)real;
	xf86DrvMsg(pScrn->scrnIndex, X_CONFIG, 
		"Memory Clock override enabled, set to %dMHz\n",
			pGlint->MemClock);
    }
    from = X_DEFAULT;
    pGlint->HWCursor = TRUE; /* ON by default */
    if (xf86GetOptValBool(GLINTOptions, OPTION_HW_CURSOR, &pGlint->HWCursor))
	from = X_CONFIG;
    if (xf86ReturnOptValBool(GLINTOptions, OPTION_SW_CURSOR, FALSE)) {
	from = X_CONFIG;
	pGlint->HWCursor = FALSE;
    }
    xf86DrvMsg(pScrn->scrnIndex, from, "Using %s cursor\n",
		pGlint->HWCursor ? "HW" : "SW");
    if (xf86ReturnOptValBool(GLINTOptions, OPTION_NOACCEL, FALSE)) {
	pGlint->NoAccel = TRUE;
	xf86DrvMsg(pScrn->scrnIndex, X_CONFIG, "Acceleration disabled\n");
    }
    if (xf86ReturnOptValBool(GLINTOptions, OPTION_SHADOW_FB, FALSE)) {
	pGlint->ShadowFB = TRUE;
	pGlint->NoAccel = TRUE;
	xf86DrvMsg(pScrn->scrnIndex, X_CONFIG, 
		"Using \"Shadow Framebuffer\" - acceleration disabled\n");
    }

    /* Check whether to use the FBDev stuff and fill in the rest of pScrn */
    if (xf86ReturnOptValBool(GLINTOptions, OPTION_FBDEV, FALSE)) {
    	if (!FBDevProbed && !xf86LoadSubModule(pScrn, "fbdevhw"))
    	{
		xf86DrvMsg(pScrn->scrnIndex, X_ERROR, "couldn't load fbdevHW module!\n");	
		return FALSE;
	}

	xf86LoaderReqSymLists(fbdevHWSymbols, NULL);

	if (!fbdevHWInit(pScrn,NULL,xf86FindOptionValue(pGlint->pEnt->device->options,"fbdev")))
	{
		xf86DrvMsg(pScrn->scrnIndex, X_ERROR, "fbdevHWInit failed!\n");
		return FALSE;
	}

	pGlint->FBDev = TRUE;
        from = X_CONFIG;
	
	pScrn->AdjustFrame	= fbdevHWAdjustFrame;
	pScrn->LeaveVT		= fbdevHWLeaveVT;
	pScrn->ValidMode	= fbdevHWValidMode;
	
    } else {
    	/* Only use FBDev if requested */
	pGlint->FBDev = FALSE;
        from = X_PROBED;
	
	pScrn->AdjustFrame	= GLINTAdjustFrame;
	pScrn->LeaveVT		= GLINTLeaveVT;
	pScrn->ValidMode	= GLINTValidMode;

    }
    xf86DrvMsg(pScrn->scrnIndex, from, "%s Linux framebuffer device\n",
		pGlint->FBDev ? "Using" : "Not using");

    pGlint->UsePCIRetry = FALSE;
    from = X_DEFAULT;
    if (xf86GetOptValBool(GLINTOptions, OPTION_PCI_RETRY, &pGlint->UsePCIRetry))
	from = X_CONFIG;
    if (pGlint->UsePCIRetry)
	xf86DrvMsg(pScrn->scrnIndex, from, "PCI retry enabled\n");
    pScrn->overlayFlags = 0;
    from = X_DEFAULT;
    if ((s = xf86GetOptValString(GLINTOptions, OPTION_OVERLAY))) {
	if (!*s || !xf86NameCmp(s, "8,24") || !xf86NameCmp(s, "24,8")) {
	    Overlay = TRUE;
	} else {
	    xf86DrvMsg(pScrn->scrnIndex, X_WARNING,
		"\"%s\" is not a valid value for Option \"Overlay\"\n", s);
	}
    }
    if (Overlay) {
	if ((pScrn->depth == 24) && (pScrn->bitsPerPixel == 32)) {
	    pScrn->colorKey = 255; /* we should let the user change this */
	    pScrn->overlayFlags = OVERLAY_8_32_PLANAR;
	    xf86DrvMsg(pScrn->scrnIndex, X_CONFIG, "24/8 overlay enabled\n");
	}
    }

    pGlint->VGAcore = FALSE;
    pGlint->DoubleBuffer = FALSE;
    pGlint->RamDac = NULL;
    /*
     * Set the Chipset and ChipRev, allowing config file entries to
     * override.
     */
    if (FBDevProbed) {	/* pm2fb so far only supports the Permedia2 */
    	pScrn->chipset = "ti_pm2";
        pGlint->Chipset = xf86StringToToken(GLINTChipsets, pScrn->chipset);
	from = X_PROBED;
    } else {
    if (pGlint->pEnt->device->chipset && *pGlint->pEnt->device->chipset) {
	pScrn->chipset = pGlint->pEnt->device->chipset;
        pGlint->Chipset = xf86StringToToken(GLINTChipsets, pScrn->chipset);
        from = X_CONFIG;
    } else if (pGlint->pEnt->device->chipID >= 0) {
	pGlint->Chipset = pGlint->pEnt->device->chipID;
	pScrn->chipset = (char *)xf86TokenToString(GLINTChipsets,
						   pGlint->Chipset);

	from = X_CONFIG;
	xf86DrvMsg(pScrn->scrnIndex, X_CONFIG, "ChipID override: 0x%04X\n",
		   pGlint->Chipset);
    } else {
	from = X_PROBED;
	pGlint->Chipset = pGlint->PciInfo->vendor << 16 | 
			  pGlint->PciInfo->chipType;
	pScrn->chipset = (char *)xf86TokenToString(GLINTChipsets,
						   pGlint->Chipset);
    }
    if (pGlint->pEnt->device->chipRev >= 0) {
	pGlint->ChipRev = pGlint->pEnt->device->chipRev;
	xf86DrvMsg(pScrn->scrnIndex, X_CONFIG, "ChipRev override: %d\n",
		   pGlint->ChipRev);
    } else {
	pGlint->ChipRev = pGlint->PciInfo->chipRev;
    }
    }

    /*
     * This shouldn't happen because such problems should be caught in
     * GLINTProbe(), but check it just in case.
     */
    if (pScrn->chipset == NULL) {
	xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
		   "ChipID 0x%04X is not recognised\n", pGlint->Chipset);
	return FALSE;
    }
    if (pGlint->Chipset < 0) {
	xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
		   "Chipset \"%s\" is not recognised\n", pScrn->chipset);
	return FALSE;
    }

    xf86DrvMsg(pScrn->scrnIndex, from, "Chipset: \"%s\"\n", pScrn->chipset);

    /* SVEN : Claim a Gamma chip if available. */
    if (pGlint->Chipset == PCI_VENDOR_3DLABS_CHIP_PERMEDIA3) {
	int eIndex = -1;
	from = X_DEFAULT;
	pGlint->PM3_UseGamma = FALSE;
	if (xf86ReturnOptValBool(GLINTOptions, OPTION_PM3_USE_GAMMA, FALSE)) {
	    from = X_CONFIG;
	    pGlint->PM3_UseGamma = TRUE;
	} 
	xf86DrvMsg(pScrn->scrnIndex, from, "%s to use Gamma with Permedia 3.\n",
		(pGlint->PM3_UseGamma ? "Trying" : "Not trying"));
	if (pGlint->PM3_UseGamma) {
	    pciVideoPtr *checkusedPci;
	    checkusedPci = xf86GetPciVideoInfo();
	    while (*checkusedPci != NULL) {
		/* Is there a free gamma on the same device ? */
		if (((*checkusedPci)->chipType == PCI_CHIP_GAMMA) &&
		  (((*checkusedPci)->bus == pGlint->PciInfo->bus)) &&
		  (((*checkusedPci)->device == pGlint->PciInfo->device)))
		    if ((eIndex = xf86ClaimPciSlot((*checkusedPci)->bus, 
			(*checkusedPci)->device, 
			(*checkusedPci)->func,
			pScrn->drv, -1,
			NULL, FALSE)) != -1) break;
	    }
	    checkusedPci++;
	}
	if (eIndex == -1) {
	    xf86DrvMsg(pScrn->scrnIndex, X_PROBED,
		"No free Gamma chip was found.\n");
	    pGlint->PM3_UseGamma = FALSE;
	} else {
	    unsigned int r;
	    /* Add the Gamma to the screen info structure. */
	    xf86AddEntityToScreen(pScrn,eIndex);
            pGlint->PM3_GammaPciInfo =
		xf86GetPciInfoForEntity(eIndex);
	    pGlint->PM3_GammaPciTag = pciTag(
		pGlint->PM3_GammaPciInfo->bus,
		pGlint->PM3_GammaPciInfo->device,
		pGlint->PM3_GammaPciInfo->func);
	    xf86DrvMsg(pScrn->scrnIndex, X_PROBED,
		"Gamma Claimed at BusID PCI:%d:%d:%d.\n",
		pGlint->PM3_GammaPciInfo->bus,
		pGlint->PM3_GammaPciInfo->device,
		pGlint->PM3_GammaPciInfo->func);
	    /* Let's dump the Gamma registers, at least some of them ... */
	    pGlint->PM3_GammaIOAddress =
		pGlint->PM3_GammaPciInfo->memBase[0] & 0xFFFFC000;
	}
    }
    if ((pGlint->Chipset == PCI_VENDOR_TI_CHIP_PERMEDIA2) ||
	(pGlint->Chipset == PCI_VENDOR_3DLABS_CHIP_PERMEDIA3) ||
	(pGlint->Chipset == PCI_VENDOR_3DLABS_CHIP_PERMEDIA2V) ||
	(pGlint->Chipset == PCI_VENDOR_3DLABS_CHIP_PERMEDIA2)) {
    	if (xf86ReturnOptValBool(GLINTOptions, OPTION_BLOCK_WRITE, FALSE)) {
	    pGlint->UseBlockWrite = TRUE;
	    xf86DrvMsg(pScrn->scrnIndex, X_CONFIG, "Block Writes enabled\n");
    	}
    }

    if (pGlint->Chipset == PCI_VENDOR_3DLABS_CHIP_500TX) {
    	if (xf86ReturnOptValBool(GLINTOptions, OPTION_FIREGL3000, FALSE)) {
	    /* Can't we detect a Fire GL 3000 ????? and remove this ? */
	    pGlint->UseFireGL3000 = TRUE;
	    xf86DrvMsg(pScrn->scrnIndex, X_CONFIG,
			"Diamond FireGL3000 mode enabled\n");
    	}
    }

    if (!FBDevProbed) {
    if (pGlint->pEnt->device->MemBase != 0) {
	/*
         * XXX Should check that the config file value matches one of the
	 * PCI base address values.
	 */
	pGlint->FbAddress = pGlint->pEnt->device->MemBase;
	from = X_CONFIG;
    } else {
	pGlint->FbAddress = pGlint->PciInfo->memBase[2] & 0xFF800000;
    }

    xf86DrvMsg(pScrn->scrnIndex, from, "Linear framebuffer at 0x%lX\n",
	       (unsigned long)pGlint->FbAddress);

    /* Trap GAMMA & DELTA specification, with no linear address */
    /* Find the first SX/TX/MX chip and use that address */
    if (pGlint->FbAddress == 0) {
	if (LinearFramebuffer) {
	    pGlint->FbAddress = LinearFramebuffer;
	    xf86DrvMsg(pScrn->scrnIndex, X_PROBED, 
			"FrameBuffer used from first SX/TX/MX chip at 0x%x\n", 
				LinearFramebuffer);
	} else {
	    xf86DrvMsg(pScrn->scrnIndex, X_PROBED, 	
			"No FrameBuffer memory - aborting\n");
	    return FALSE;
	}
    }

    if (pGlint->pEnt->device->IOBase != 0) {
	/*
         * XXX Should check that the config file value matches one of the
	 * PCI base address values.
	 */
	pGlint->IOAddress = pGlint->pEnt->device->IOBase;
	from = X_CONFIG;
#if 0 /* This freezes the box, why ? */
    } else if ((pGlint->Chipset == PCI_VENDOR_3DLABS_CHIP_PERMEDIA3) &&
	pGlint->PM3_UseGamma) {
	pGlint->IOAddress = pGlint->PM3_GammaPciInfo->memBase[0] & 0xFFFFC000;
#endif
    } else {
	pGlint->IOAddress = pGlint->PciInfo->memBase[0] & 0xFFFFC000;
    }
#if X_BYTE_ORDER == X_BIG_ENDIAN
    pGlint->IOAddress += 0x10000;
#endif

    xf86DrvMsg(pScrn->scrnIndex, from, "MMIO registers at 0x%lX\n",
	       (unsigned long)pGlint->IOAddress);

    pGlint->irq = pGlint->pEnt->device->irq;

    /* Register all entities */
    for (i = 0; i < pScrn->numEntities; i++) {
	EntityInfoPtr pEnt;
	pEnt = xf86GetEntityInfo(pScrn->entityList[i]);
        if (xf86RegisterResources(pEnt->index, NULL, ResExclusive)) {
	    xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
		   "xf86RegisterResources() found resource conflicts\n");
	    return FALSE;
        }
    }
    }

    /* HW bpp matches reported bpp */
    pGlint->HwBpp = pScrn->bitsPerPixel;

    pGlint->FbBase = NULL;
    if (!FBDevProbed) {
    	if (pGlint->pEnt->device->videoRam != 0) {
		pScrn->videoRam = pGlint->pEnt->device->videoRam;
		from = X_CONFIG;
        } else if (pGlint->Chipset == PCI_VENDOR_3DLABS_CHIP_PERMEDIA3) {
		/* Need to initialize the memory timings of the second head
		 * of the Appian Jeronimo 2000 board here.
		 * Check this routine in pm3_dac.c if you have another board
		 * than a J2000, as memory timings will surely change.
		*/
        	GLINTMapMem(pScrn);
		Permedia3PreInit(pScrn);
        	GLINTUnmapMem(pScrn);
		pScrn->videoRam = Permedia3MemorySizeDetect(pScrn);
        } else if (pGlint->Chipset == PCI_VENDOR_3DLABS_CHIP_PERMEDIA2V) {
		/* The PM2v has the same problem detecting memory as the PM3 */
		pScrn->videoRam = Permedia3MemorySizeDetect(pScrn);
    	} else {
		pGlint->FbMapSize = 0; /* Need to set FbMapSize for MMIO access */
		/* Need to access MMIO to determine videoRam */
        	GLINTMapMem(pScrn);
		if( (pGlint->Chipset == PCI_VENDOR_3DLABS_CHIP_500TX) ||
	    	(pGlint->Chipset == PCI_VENDOR_3DLABS_CHIP_300SX) ||
	    	(pGlint->Chipset == PCI_VENDOR_3DLABS_CHIP_MX) ||
	    	(pGlint->Chipset == PCI_VENDOR_3DLABS_CHIP_GAMMA) )
	    	pScrn->videoRam = 1024 * (1 << ((GLINT_READ_REG(FBMemoryCtl) & 
							0xE0000000)>>29));
		else 
	    	pScrn->videoRam = 2048 * (((GLINT_READ_REG(PMMemConfig) >> 29) &
							0x03) + 1);
        	GLINTUnmapMem(pScrn);
    	}
    } else {
    	pScrn->videoRam = fbdevHWGetVidmem(pScrn)/1024;
    }

    pGlint->FbMapSize = pScrn->videoRam * 1024;

    /* We should move this out somewhere ! */
    if ( (pGlint->Chipset == PCI_VENDOR_3DLABS_CHIP_GAMMA) &&
	     (pGlint->numMXDevices == 2) ) {
	CARD32 chipconfig;
	CARD32 temp;

	GLINTMapMem(pScrn);

#if 0
	/* We shouldn't really do this yet, we haven't saved the
	 * state of the chip.
	 * Best of all put a timer in to reset the engine if it
	 * doesn't respond - but later */
	/*
	 * This is needed before the first GLINT_SLOW_WRITE_REG --
	 * otherwise the server will hang if it was left in a bad state.
         */
	GLINT_WRITE_REG(0, ResetStatus);
	while (GLINT_READ_REG(ResetStatus) & 0x80000000) {
	    xf86DrvMsg(pScrn->scrnIndex, from, "Resetting Core\n");
	}
#endif
	    
	temp = GLINT_READ_REG(GCSRAperture);
	GLINT_SLOW_WRITE_REG(GCSRSecondaryGLINTMapEn, GCSRAperture);

#if 0
	xf86DrvMsg(pScrn->scrnIndex, from,
		   "InFIFOSpace = %d, %d (after reset)\n",
		   GLINT_READ_REG(InFIFOSpace),
		   GLINT_SECONDARY_READ_REG(InFIFOSpace));
	xf86DrvMsg(pScrn->scrnIndex, from,
		   "OutFIFOSWords = %d, %d (after reset)\n",
		   GLINT_READ_REG(OutFIFOWords),
		   GLINT_SECONDARY_READ_REG(OutFIFOWords));

				/* Reset doesn't appear to drain the Output
                                   FIFO.  Argh. */
	while (GLINT_READ_REG(OutFIFOWords)) {
	    GLINT_READ_REG(OutputFIFO);
	}
	while (GLINT_SECONDARY_READ_REG(OutFIFOWords)) {
	    GLINT_SECONDARY_READ_REG(OutputFIFO);
	}
	
	xf86DrvMsg(pScrn->scrnIndex, from,
		   "InFIFOSpace = %d, %d (after drain)\n",
		   GLINT_READ_REG(InFIFOSpace),
		   GLINT_SECONDARY_READ_REG(InFIFOSpace));
	xf86DrvMsg(pScrn->scrnIndex, from,
		   "OutFIFOSWords = %d, %d (after drain)\n",
		   GLINT_READ_REG(OutFIFOWords),
		   GLINT_SECONDARY_READ_REG(OutFIFOWords));
#endif
	
	chipconfig = GLINT_READ_REG(GChipConfig);

	GLINT_SLOW_WRITE_REG(GCSRSecondaryGLINTMapEn, GCSRAperture);

	GLINTUnmapMem(pScrn);
	
	switch (chipconfig & GChipMultiGLINTApMask) {
	case GChipMultiGLINTAp_0M:
	    pGlint->FbMapSize = 0;
	    break;
	case GChipMultiGLINTAp_16M:
	    pGlint->FbMapSize = 16 * 1024 * 1024;
	    break;
	case GChipMultiGLINTAp_32M:
	    pGlint->FbMapSize = 32 * 1024 * 1024;
	    break;
	case GChipMultiGLINTAp_64M:
	    pGlint->FbMapSize = 64 * 1024 * 1024;
	    break;
	}
    }

    xf86DrvMsg(pScrn->scrnIndex, from, "VideoRAM: %d kByte\n",
		   pGlint->FbMapSize / 1024);

    /* The ramdac module should be loaded here when needed */
    if (!xf86LoadSubModule(pScrn, "ramdac"))
	return FALSE;

    /* Let's check what type of DAC we have and reject if necessary */
    switch (pGlint->Chipset) {
	case PCI_VENDOR_TI_CHIP_PERMEDIA2:
	case PCI_VENDOR_3DLABS_CHIP_PERMEDIA2:
	    maxheight = 2048;
	    maxwidth = 2048;
	    pGlint->RefClock = 14318;
	    pGlint->VGAcore = TRUE; /* chip has a vga core */
	    pGlint->RamDacRec = RamDacCreateInfoRec();
	    pGlint->RamDacRec->ReadDAC = Permedia2InIndReg;
	    pGlint->RamDacRec->WriteDAC = Permedia2OutIndReg;
	    pGlint->RamDacRec->ReadAddress = Permedia2ReadAddress;
	    pGlint->RamDacRec->WriteAddress = Permedia2WriteAddress;
	    pGlint->RamDacRec->ReadData = Permedia2ReadData;
	    pGlint->RamDacRec->WriteData = Permedia2WriteData;
	    if(!RamDacInit(pScrn, pGlint->RamDacRec)) {
		RamDacDestroyInfoRec(pGlint->RamDacRec);
		return FALSE;
	    }
	    break;
	case PCI_VENDOR_3DLABS_CHIP_PERMEDIA2V:
	    maxheight = 2048;
	    maxwidth = 2048;
	    pGlint->RefClock = 14318;
	    pGlint->VGAcore = TRUE; /* chip has a vga core */
	    pGlint->RamDacRec = RamDacCreateInfoRec();
	    pGlint->RamDacRec->ReadDAC = Permedia2vInIndReg;
	    pGlint->RamDacRec->WriteDAC = Permedia2vOutIndReg;
	    pGlint->RamDacRec->ReadAddress = Permedia2ReadAddress;
	    pGlint->RamDacRec->WriteAddress = Permedia2WriteAddress;
	    pGlint->RamDacRec->ReadData = Permedia2ReadData;
	    pGlint->RamDacRec->WriteData = Permedia2WriteData;
	    if(!RamDacInit(pScrn, pGlint->RamDacRec)) {
		RamDacDestroyInfoRec(pGlint->RamDacRec);
		return FALSE;
	    }
	    break;
	case PCI_VENDOR_3DLABS_CHIP_PERMEDIA3:
	    maxheight = 4096;
	    maxwidth = 4096;
	    pGlint->RefClock = 14318;
	    pGlint->VGAcore = TRUE;
	    pGlint->RamDacRec = RamDacCreateInfoRec();
	    pGlint->RamDacRec->ReadDAC = Permedia2vInIndReg;
	    pGlint->RamDacRec->WriteDAC = Permedia2vOutIndReg;
	    pGlint->RamDacRec->ReadAddress = Permedia2ReadAddress;
	    pGlint->RamDacRec->WriteAddress = Permedia2WriteAddress;
	    pGlint->RamDacRec->ReadData = Permedia2ReadData;
	    pGlint->RamDacRec->WriteData = Permedia2WriteData;
	    if(!RamDacInit(pScrn, pGlint->RamDacRec)) {
		RamDacDestroyInfoRec(pGlint->RamDacRec);
		return FALSE;
	    }
	    break;
	case PCI_VENDOR_TI_CHIP_PERMEDIA:
	case PCI_VENDOR_3DLABS_CHIP_PERMEDIA:
	    maxheight = 1024;
	    maxwidth = 1536;
	    pGlint->VGAcore = TRUE; /* chip has a vga core */
	    GLINTProbeIBMramdac(pScrn);
	    if (pGlint->RamDac == NULL) return FALSE;
	    if (pGlint->RamDac->RamDacType != (IBM526DB_RAMDAC) &&
		pGlint->RamDac->RamDacType != (IBM526_RAMDAC))
		return FALSE;
	    pGlint->RefClock = 14318;
	    break;
	case PCI_VENDOR_3DLABS_CHIP_500TX:
	case PCI_VENDOR_3DLABS_CHIP_300SX:
	case PCI_VENDOR_3DLABS_CHIP_MX:
	case PCI_VENDOR_3DLABS_CHIP_GAMMA:
    	    if ((pGlint->Chipset == PCI_VENDOR_3DLABS_CHIP_GAMMA) && 
		 pGlint->UsePCIRetry) {
		xf86DrvMsg(pScrn->scrnIndex, from, 
				"GAMMA in use - PCI retries disabled\n");
		pGlint->UsePCIRetry = FALSE;
	    }
	    if (pScrn->bitsPerPixel == 24) {
		xf86DrvMsg(pScrn->scrnIndex, from, 
			"-depth 24 -pixmap24 not supported by this chip.\n");
		return FALSE;
	    }
	    maxheight = 4096;
	    maxwidth = 4096;
	    /* Test for an TI ramdac */
	    if (!pGlint->RamDac) {
	    	GLINTProbeTIramdac(pScrn);
		if (pGlint->RamDac)
	             if ( (pGlint->RamDac->RamDacType == (TI3026_RAMDAC)) ||
	         	  (pGlint->RamDac->RamDacType == (TI3030_RAMDAC)) )
		    	pGlint->RefClock = 14318;
	    }
	    /* Test for an IBM ramdac */
	    if (!pGlint->RamDac) {
	    	GLINTProbeIBMramdac(pScrn);
		if (pGlint->RamDac) {
	    	    if (pGlint->RamDac->RamDacType == (IBM640_RAMDAC))
		    	pGlint->RefClock = 28322;
	    	    else
	    	    if (pGlint->RamDac->RamDacType == (IBM526DB_RAMDAC) ||
		    	pGlint->RamDac->RamDacType == (IBM526_RAMDAC))
		    	pGlint->RefClock = 40000;
	    	    else {
    		    	xf86DrvMsg(pScrn->scrnIndex, X_DEFAULT, 
							"Undefined RefClock\n");
		        return FALSE;
	    	    }
		}
	    }
	    if (!pGlint->RamDac)
		return FALSE;
	    break;
    }

    if (pGlint->FBDev || FBDevProbed)
    	pGlint->VGAcore = FALSE;

    if (pGlint->VGAcore) {
    	/* The vgahw module should be loaded here when needed */
    	if (!xf86LoadSubModule(pScrn, "vgahw"))
		return FALSE;

    	xf86LoaderReqSymLists(vgahwSymbols, NULL);
    	/*
         * Allocate a vgaHWRec
         */
        if (!vgaHWGetHWRec(pScrn))
	    return FALSE;
    }

    /* Set the min pixel clock */
    pGlint->MinClock = 16250;	/* XXX Guess, need to check this */
    xf86DrvMsg(pScrn->scrnIndex, X_DEFAULT, "Min pixel clock is %d MHz\n",
	       pGlint->MinClock / 1000);

    /*
     * If the user has specified ramdac speed in the XF86Config
     * file, we respect that setting.
     */
    if (pGlint->pEnt->device->dacSpeeds[0]) {
	int speed = 0;

	switch (pScrn->bitsPerPixel) {
	case 8:
	   speed = pGlint->pEnt->device->dacSpeeds[DAC_BPP8];
	   break;
	case 16:
	   speed = pGlint->pEnt->device->dacSpeeds[DAC_BPP16];
	   break;
	case 24:
	   speed = pGlint->pEnt->device->dacSpeeds[DAC_BPP24];
	   break;
	case 32:
	   speed = pGlint->pEnt->device->dacSpeeds[DAC_BPP32];
	   break;
	}
	if (speed == 0)
	    pGlint->MaxClock = pGlint->pEnt->device->dacSpeeds[0];
	else
	    pGlint->MaxClock = speed;
	from = X_CONFIG;
    } else {
	if((pGlint->Chipset == PCI_VENDOR_3DLABS_CHIP_500TX)||
	   (pGlint->Chipset == PCI_VENDOR_3DLABS_CHIP_300SX) ||
	   (pGlint->Chipset == PCI_VENDOR_3DLABS_CHIP_MX) ||
	   (pGlint->Chipset == PCI_VENDOR_3DLABS_CHIP_GAMMA) )
		pGlint->MaxClock = 220000;
	if ( (pGlint->Chipset == PCI_VENDOR_TI_CHIP_PERMEDIA) ||
	     (pGlint->Chipset == PCI_VENDOR_3DLABS_CHIP_PERMEDIA) ) {
		switch (pScrn->bitsPerPixel) {
		    case 8:
			pGlint->MaxClock = 200000;
			break;
		    case 16:
			pGlint->MaxClock = 100000;
			break;
		    case 24:
			pGlint->MaxClock = 50000;
			break;
		    case 32:
			pGlint->MaxClock = 50000;
			break;
		}
	}
	if ( (pGlint->Chipset == PCI_VENDOR_TI_CHIP_PERMEDIA2) ||
	     (pGlint->Chipset == PCI_VENDOR_3DLABS_CHIP_PERMEDIA2) ||
	     (pGlint->Chipset == PCI_VENDOR_3DLABS_CHIP_PERMEDIA2V) ) {
		switch (pScrn->bitsPerPixel) {
		    case 8:
			pGlint->MaxClock = 230000;
			break;
		    case 16:
			pGlint->MaxClock = 230000;
			break;
		    case 24:
			pGlint->MaxClock = 150000;
			break;
		    case 32:
			pGlint->MaxClock = 110000;
			break;
		}
	}
	if (pGlint->Chipset == PCI_VENDOR_3DLABS_CHIP_PERMEDIA3)
	    pGlint->MaxClock = 300000;
    }
    xf86DrvMsg(pScrn->scrnIndex, from, "Max pixel clock is %d MHz\n",
	       pGlint->MaxClock / 1000);

    /*
     * Setup the ClockRanges, which describe what clock ranges are available,
     * and what sort of modes they can be used for.
     */
    clockRanges = xnfcalloc(sizeof(ClockRange), 1);
    clockRanges->next = NULL;
    clockRanges->minClock = pGlint->MinClock;
    clockRanges->maxClock = pGlint->MaxClock;
    clockRanges->clockIndex = -1;		/* programmable */
    clockRanges->interlaceAllowed = FALSE;	/* XXX check this */
    clockRanges->doubleScanAllowed = FALSE;	/* XXX check this */

    /*
     * xf86ValidateModes will check that the mode HTotal and VTotal values
     * don't exceed the chipset's limit if pScrn->maxHValue and
     * pScrn->maxVValue are set.  Since our GLINTValidMode() already takes
     * care of this, we don't worry about setting them here.
     */

    /* Select valid modes from those available */
    if ((pGlint->NoAccel) ||
	(pGlint->Chipset == PCI_VENDOR_3DLABS_CHIP_PERMEDIA3)) {
	/*
	 * XXX Assuming min pitch 256, max <maxwidth>
	 * XXX Assuming min height 128, max <maxheight>
	 */
	i = xf86ValidateModes(pScrn, pScrn->monitor->Modes,
			      pScrn->display->modes, clockRanges,
			      NULL, 256, maxwidth,
			      pScrn->bitsPerPixel, 128, maxheight,
			      pScrn->display->virtualX,
			      pScrn->display->virtualY,
			      pGlint->FbMapSize,
			      LOOKUP_BEST_REFRESH);
    } else {
	/*
	 * Minimum width 32, Maximum width <maxwidth>
	 * Minimum height 128, Maximum height <maxheight>
	 */
	i = xf86ValidateModes(pScrn, pScrn->monitor->Modes,
			      pScrn->display->modes, clockRanges,
			      GetAccelPitchValues(pScrn), 32, maxwidth,
			      pScrn->bitsPerPixel, 128, maxheight,
			      pScrn->display->virtualX,
			      pScrn->display->virtualY,
			      pGlint->FbMapSize,
			      LOOKUP_BEST_REFRESH);
    }

    if (i < 1 && pGlint->FBDev) {
	fbdevHWUseBuildinMode(pScrn);
	i = 1;
    }

    if (i == -1) {
	GLINTFreeRec(pScrn);
	return FALSE;
    }

    if (i == 0 || pScrn->modes == NULL) {
	xf86DrvMsg(pScrn->scrnIndex, X_ERROR, "No valid modes found\n");
	GLINTFreeRec(pScrn);
	return FALSE;
    }

    if (pGlint->FBDev) {
	/* shift horizontal timings for 64bit VRAM's or 32bit SGRAMs */
	switch (pScrn->bitsPerPixel) {
	case 8:
		pGlint->BppShift = 2;
		break;
	case 16:
		if (pGlint->DoubleBuffer) {
	    		pGlint->BppShift = 0;
		} else {
	    		pGlint->BppShift = 1;
		}
		break;
	case 24:
		pGlint->BppShift = 2;
		break;
	case 32:
		pGlint->BppShift = 0;
		break;
	}

	pScrn->displayWidth = pScrn->virtualX;

	/* Ensure vsync and hsync are high when using HW cursor */
	if (pGlint->HWCursor) {
		DisplayModePtr mode, first = mode = pScrn->modes;
		
		do {	/* We know there is at least the built-in mode */
			mode->Flags |= V_PHSYNC | V_PVSYNC;
			mode->Flags &= ~V_NHSYNC | ~V_NVSYNC;
			mode = mode->next;
		} while (mode != NULL && mode != first);
	}
    }

    /* Prune the modes marked as invalid */
    xf86PruneDriverModes(pScrn);

    /* Only allow a single mode for MX and TX chipsets */
    if ((pGlint->Chipset == PCI_VENDOR_3DLABS_CHIP_500TX) ||
        (pGlint->Chipset == PCI_VENDOR_3DLABS_CHIP_300SX) ||
        (pGlint->Chipset == PCI_VENDOR_3DLABS_CHIP_MX) ||
        (pGlint->Chipset == PCI_VENDOR_3DLABS_CHIP_GAMMA)) {
	xf86DrvMsg(pScrn->scrnIndex, X_WARNING, 
	    "GLINT SX/TX/MX chipsets only support one modeline, using first\n");
	pScrn->modes->next = NULL;
	pScrn->virtualX = pScrn->modes->HDisplay;
	pScrn->virtualY = pScrn->modes->VDisplay;
	pScrn->displayWidth = pScrn->virtualX;
	if (partprod500TX[pScrn->displayWidth >> 5] == -1) {
	    i = -1;
	    do {
	        pScrn->displayWidth += 32;
	        i = partprod500TX[pScrn->displayWidth >> 5];
	    } while (i == -1);
	}
    }

    switch (pGlint->Chipset)
    { /* Now we know displaywidth, so set linepitch data */
    case PCI_VENDOR_TI_CHIP_PERMEDIA2:
    case PCI_VENDOR_3DLABS_CHIP_PERMEDIA2:
    case PCI_VENDOR_3DLABS_CHIP_PERMEDIA2V:
    case PCI_VENDOR_TI_CHIP_PERMEDIA:
    case PCI_VENDOR_3DLABS_CHIP_PERMEDIA:
	pGlint->pprod = partprodPermedia[pScrn->displayWidth >> 5];
	pGlint->bppalign = bppand[(pScrn->bitsPerPixel>>3)-1];
	break;
    case PCI_VENDOR_3DLABS_CHIP_500TX:
    case PCI_VENDOR_3DLABS_CHIP_MX:
    case PCI_VENDOR_3DLABS_CHIP_300SX:
    case PCI_VENDOR_3DLABS_CHIP_GAMMA:
	pGlint->pprod = partprod500TX[pScrn->displayWidth >> 5];
	pGlint->bppalign = 0;
	break;
    }

    if ( (pGlint->Chipset == PCI_VENDOR_3DLABS_CHIP_GAMMA) &&
         (pGlint->numMXDevices == 2) ) {
	int bytesPerPixel, realMXWidthBytes, inputXSpanBytes;
	CARD32 postMultiply, productEnable, use16xProduct, inputXSpan;
	CARD32 binaryEval, glintApSize;

	/* setup multi glint framebuffer aperture */
	bytesPerPixel = (pScrn->bitsPerPixel >> 3);
	realMXWidthBytes = pScrn->displayWidth * bytesPerPixel;

	/* compute Input X Span field */
	binaryEval = ((realMXWidthBytes << 1) - 1);
	if (binaryEval & (8 << 10)) {      /* 8K */
	    inputXSpan = 3;
	    inputXSpanBytes = 8 * 1024;
	}
	else if (binaryEval & (4 << 10)) { /* 4K */
	    inputXSpan = 2;
	    inputXSpanBytes = 4 * 1024;
	}
	else if (binaryEval & (2 << 10)) { /* 2K */
	    inputXSpan = 1;
	    inputXSpanBytes = 2 * 1024;
	}
	else {                             /* 1K */
	    inputXSpan = 0;
	    inputXSpanBytes = 1024;
	}

	/* compute post multiply */
	binaryEval = realMXWidthBytes >> 3;
	postMultiply = 0;
	while ((postMultiply < 5) && !(binaryEval & 1)) {
	    postMultiply++;
	    binaryEval >>= 1;
	}
	postMultiply <<= 7;

	/* compute product enable fields */
	if (binaryEval & ~0x1f) {		/* too big */
	    xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
		       "GLINTModeInit: width (%d) too big\n",
		       pScrn->displayWidth);
	    return FALSE;
	}
	if ((binaryEval & 0x12) == 0x12) {	/* clash between x2 and x16 */
	    xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
		       "GLINTModeInit: width (%d) is mult 18, not supported\n",
		       pScrn->displayWidth);
	    return FALSE;
	}
	if (binaryEval & 0x10) {
	    productEnable = (((binaryEval & 0xf) | 0x2) << 3);
	    use16xProduct = (1 << 2);
	}
	else {
	    productEnable = ((binaryEval & 0xf) << 3);
	    use16xProduct = 0;
	}

	/* compute GLINT Aperture Size field */
	binaryEval = ((pScrn->videoRam << 11) - 1);
	if (binaryEval & (32 << 20)) {      /* 32M */
	    glintApSize = 3 << 10;
	}
	else if (binaryEval & (16 << 20)) { /* 16M */
	    glintApSize = 2 << 10;
	}
	else if (binaryEval & (8 << 20)) {  /*  8M */
	    glintApSize = 1 << 10;
	}
	else {                              /*  4M */
	    glintApSize = 0 << 10;
	}

	pGlint->realMXWidth = ( glintApSize      | 
                  	  	postMultiply     | 
                  		productEnable    | 
                  		use16xProduct    | 
                  		inputXSpan       );

	/* set the MULTI width for software rendering */
	pScrn->displayWidth = inputXSpanBytes / bytesPerPixel;
    }

    /* Set the current mode to the first in the list */
    pScrn->currentMode = pScrn->modes;

    xf86SetCrtcForModes(pScrn, INTERLACE_HALVE_V);

    /* Print the list of modes being used */
    xf86PrintModes(pScrn);

    /* Set display resolution */
    xf86SetDpi(pScrn, 0, 0);

    /* Load bpp-specific modules */
    switch (pScrn->bitsPerPixel) {
    case 8:
    case 16:
    case 24:
	mod = "fb";
	break;
    case 32:
	if (pScrn->overlayFlags & OVERLAY_8_32_PLANAR) {
	    if (xf86LoadSubModule(pScrn, "xf8_32bpp") == NULL) {
		GLINTFreeRec(pScrn);
		return FALSE;
	    } else
		xf86LoaderReqSymLists(xf8_32bppSymbols,NULL);
	} else
	    mod = "fb";
	break;
    }
    if (mod && xf86LoadSubModule(pScrn, mod) == NULL) {
	GLINTFreeRec(pScrn);
	return FALSE;
    }

    /* Load XAA if needed */
    if (!pGlint->NoAccel) {
	if (!xf86LoadSubModule(pScrn, "xaa")) {
	    GLINTFreeRec(pScrn);
	    return FALSE;
	}
	xf86LoaderReqSymLists(xaaSymbols, NULL);
    }

    /* Load shadowfb if needed */
    if (pGlint->ShadowFB) {
	if (!xf86LoadSubModule(pScrn, "shadowfb")) {
	    GLINTFreeRec(pScrn);
	    return FALSE;
	}
	xf86LoaderReqSymLists(shadowSymbols, NULL);
    }

    /* Load DDC */
    if (!xf86LoadSubModule(pScrn, "ddc")) {
	GLINTFreeRec(pScrn);
	return FALSE;
    }
    /* Load I2C if needed */
    if ((pGlint->Chipset == PCI_VENDOR_3DLABS_CHIP_PERMEDIA2) ||
	(pGlint->Chipset == PCI_VENDOR_3DLABS_CHIP_PERMEDIA2V) ||
	(pGlint->Chipset == PCI_VENDOR_3DLABS_CHIP_PERMEDIA3) ||
	(pGlint->Chipset == PCI_VENDOR_TI_CHIP_PERMEDIA2)) {
	if (xf86LoadSubModule(pScrn, "i2c")) {
	    I2CBusPtr pBus;

	    if ((pBus = xf86CreateI2CBusRec())) {
		pBus->BusName = "DDC";
		pBus->scrnIndex = pScrn->scrnIndex;
		pBus->I2CUDelay = Permedia2I2CUDelay;
		pBus->I2CPutBits = Permedia2I2CPutBits;
		pBus->I2CGetBits = Permedia2I2CGetBits;
		pBus->DriverPrivate.ptr = pGlint;
		if (!xf86I2CBusInit(pBus)) {
		    xf86DestroyI2CBusRec(pBus, TRUE, TRUE);
		} else
		    pGlint->DDCBus = pBus; 
	    }

	    if ((pBus = xf86CreateI2CBusRec())) {
	        pBus->BusName = "Video";
	        pBus->scrnIndex = pScrn->scrnIndex;
		pBus->I2CUDelay = Permedia2I2CUDelay;
		pBus->I2CPutBits = Permedia2I2CPutBits;
		pBus->I2CGetBits = Permedia2I2CGetBits;
		pBus->DriverPrivate.ptr = pGlint;
		if (!xf86I2CBusInit(pBus)) {
		    xf86DestroyI2CBusRec(pBus, TRUE, TRUE);
		} else
		    pGlint->VSBus = pBus;
	    }
	}
    }
    
    TRACE_EXIT("GLINTPreInit");
    return TRUE;
}


/*
 * Map the framebuffer and MMIO memory.
 */

static Bool
GLINTMapMem(ScrnInfoPtr pScrn)
{
    GLINTPtr pGlint;

    pGlint = GLINTPTR(pScrn);

    TRACE_ENTER("GLINTMapMem");
    if (pGlint->FBDev) {
    	pGlint->FbBase = fbdevHWMapVidmem(pScrn);
    	if (pGlint->FbBase == NULL)
		return FALSE;

    	pGlint->IOBase = fbdevHWMapMMIO(pScrn);
    	if (pGlint->IOBase == NULL)
		return FALSE;
	/*
	 * This does not work on Alphas ! They need VGA MMIO space
	 * mapped in a special way as they cannot access it byte
	 * or wordwise.
	 */
	pGlint->IOBaseVGA = pGlint->IOBase + GLINT_VGA_MMIO_OFF;
	
	TRACE_EXIT("GLINTMapMem");
	return TRUE;
    }
    
    /*
     * Map IO registers to virtual address space
     * We always map VGA IO registers - even if we don't need them
     */ 
    pGlint->IOBase = xf86MapPciMem(pScrn->scrnIndex, VIDMEM_MMIO_32BIT, 
	       pGlint->PciTag, pGlint->IOAddress, 0x20000);
    pGlint->IOBaseVGA = xf86MapPciMem(pScrn->scrnIndex, VIDMEM_MMIO, 
	       pGlint->PciTag, pGlint->IOAddress + GLINT_VGA_MMIO_OFF, 0x2000);

    if (pGlint->IOBase == NULL)
	return FALSE;

    if (pGlint->FbMapSize != 0) {
    	pGlint->FbBase = xf86MapPciMem(pScrn->scrnIndex, VIDMEM_FRAMEBUFFER,
				 pGlint->PciTag,
				 pGlint->FbAddress,
				 pGlint->FbMapSize);
        if (pGlint->FbBase == NULL)
	    return FALSE;
    }

    TRACE_EXIT("GLINTMapMem");
    return TRUE;
}


/*
 * Unmap the framebuffer and MMIO memory.
 */

static Bool
GLINTUnmapMem(ScrnInfoPtr pScrn)
{
    GLINTPtr pGlint;

    pGlint = GLINTPTR(pScrn);

    TRACE_ENTER("GLINTUnmapMem");
    if (pGlint->FBDev) {
    	fbdevHWUnmapVidmem(pScrn);
    	pGlint->FbBase = NULL;
    	fbdevHWUnmapMMIO(pScrn);
    	pGlint->IOBase = NULL;

	TRACE_EXIT("GLINTUnmapMem");
    	return TRUE;
    }
    
    /*
     * Unmap IO registers to virtual address space
     */ 
    xf86UnMapVidMem(pScrn->scrnIndex, (pointer)pGlint->IOBase, 0x20000);
    pGlint->IOBase = NULL;

    if (pGlint->IOBaseVGA != NULL)
	xf86UnMapVidMem(pScrn->scrnIndex, (pointer)pGlint->IOBaseVGA, 0x2000);
    pGlint->IOBaseVGA = NULL;

    if (pGlint->FbBase != NULL)
    	xf86UnMapVidMem(pScrn->scrnIndex, (pointer)pGlint->FbBase, pGlint->FbMapSize);
    pGlint->FbBase = NULL;

    TRACE_EXIT("GLINTUnmapMem");
    return TRUE;
}

/*
 * This function saves the video state.
 */
static void
GLINTSave(ScrnInfoPtr pScrn)
{
    GLINTPtr pGlint;
    GLINTRegPtr glintReg;
    RamDacHWRecPtr pRAMDAC;
    RamDacRegRecPtr RAMDACreg;

    pGlint = GLINTPTR(pScrn);
    pRAMDAC = RAMDACHWPTR(pScrn);
    glintReg = &pGlint->SavedReg;
    RAMDACreg = &pRAMDAC->SavedReg;
    TRACE_ENTER("GLINTSave");
    if (pGlint->VGAcore) {
    	vgaRegPtr vgaReg;
    	vgaReg = &VGAHWPTR(pScrn)->SavedReg;
	if (xf86IsPrimaryPci(pGlint->PciInfo)) {
	    vgaHWSave(pScrn, vgaReg, VGA_SR_MODE | VGA_SR_FONTS);
	} else {
	    vgaHWSave(pScrn, vgaReg, VGA_SR_MODE);
	}
    }

    switch (pGlint->Chipset)
    {
    case PCI_VENDOR_TI_CHIP_PERMEDIA2:
    case PCI_VENDOR_3DLABS_CHIP_PERMEDIA2:
	Permedia2Save(pScrn, glintReg);
	break;
    case PCI_VENDOR_3DLABS_CHIP_PERMEDIA2V:
	Permedia2VSave(pScrn, glintReg);
	break;
    case PCI_VENDOR_3DLABS_CHIP_PERMEDIA3:
	Permedia3Save(pScrn, glintReg);
	break;
    case PCI_VENDOR_TI_CHIP_PERMEDIA:
    case PCI_VENDOR_3DLABS_CHIP_PERMEDIA:
	PermediaSave(pScrn, glintReg);
	(*pGlint->RamDac->Save)(pScrn, pGlint->RamDacRec, RAMDACreg);
	break;
    case PCI_VENDOR_3DLABS_CHIP_500TX:
    case PCI_VENDOR_3DLABS_CHIP_300SX:
    case PCI_VENDOR_3DLABS_CHIP_MX:
    case PCI_VENDOR_3DLABS_CHIP_GAMMA:
	if ( (pGlint->Chipset == PCI_VENDOR_3DLABS_CHIP_GAMMA) &&
	     (pGlint->numMXDevices == 2) ) {
	    DualMXSave(pScrn, glintReg);
	}
	else {
	    TXSave(pScrn, glintReg);
	}
	(*pGlint->RamDac->Save)(pScrn, pGlint->RamDacRec, RAMDACreg);
	break;
    }
    TRACE_EXIT("GLINTSave");
}


/*
 * Initialise a new mode.  This is currently still using the old
 * "initialise struct, restore/write struct to HW" model.  That could
 * be changed.
 */

static Bool
GLINTModeInit(ScrnInfoPtr pScrn, DisplayModePtr mode)
{
    int ret = -1;
    GLINTPtr pGlint = GLINTPTR(pScrn);
    GLINTRegPtr glintReg;
    RamDacHWRecPtr pRAMDAC = RAMDACHWPTR(pScrn);
    RamDacRegRecPtr RAMDACreg;

    if (pGlint->VGAcore) {
    	vgaHWPtr hwp = VGAHWPTR(pScrn);
    	vgaHWUnlock(hwp);

    	/* Initialise the ModeReg values */
    	if (!vgaHWInit(pScrn, mode))
	    return FALSE;
    }

    pScrn->vtSema = TRUE;

    switch (pGlint->Chipset) {
    case PCI_VENDOR_TI_CHIP_PERMEDIA2:
    case PCI_VENDOR_3DLABS_CHIP_PERMEDIA2:
	ret = Permedia2Init(pScrn, mode);
	break;
    case PCI_VENDOR_3DLABS_CHIP_PERMEDIA2V:
	ret = Permedia2VInit(pScrn, mode);
	break;
    case PCI_VENDOR_3DLABS_CHIP_PERMEDIA3:
	ret = Permedia3Init(pScrn, mode);
	break;
    case PCI_VENDOR_TI_CHIP_PERMEDIA:
    case PCI_VENDOR_3DLABS_CHIP_PERMEDIA:
	ret = PermediaInit(pScrn, mode);
	break;
    case PCI_VENDOR_3DLABS_CHIP_500TX:
    case PCI_VENDOR_3DLABS_CHIP_300SX:
    case PCI_VENDOR_3DLABS_CHIP_MX:
    case PCI_VENDOR_3DLABS_CHIP_GAMMA:
	if ( (pGlint->Chipset == PCI_VENDOR_3DLABS_CHIP_GAMMA) &&
	     (pGlint->numMXDevices == 2) ) {
	    ret = DualMXInit(pScrn, mode);
	}
	else {
	    ret = TXInit(pScrn, mode);
	}
	break;
    }

    if (!ret)
	return FALSE;

    /* Program the registers */
    if (pGlint->VGAcore) {
    	vgaHWPtr hwp = VGAHWPTR(pScrn);
    	vgaRegPtr vgaReg = &hwp->ModeReg;
    	vgaHWProtect(pScrn, TRUE);
	vgaHWRestore(pScrn, vgaReg, VGA_SR_MODE);
    }

    glintReg = &pGlint->ModeReg;
    RAMDACreg = &pRAMDAC->ModeReg;

    switch (pGlint->Chipset) {
    case PCI_VENDOR_TI_CHIP_PERMEDIA2:
    case PCI_VENDOR_3DLABS_CHIP_PERMEDIA2:
	Permedia2Restore(pScrn, glintReg);
	break;
    case PCI_VENDOR_3DLABS_CHIP_PERMEDIA2V:
	Permedia2VRestore(pScrn, glintReg);
	break;
    case PCI_VENDOR_3DLABS_CHIP_PERMEDIA3:
	Permedia3Restore(pScrn, glintReg);
	break;
    case PCI_VENDOR_TI_CHIP_PERMEDIA:
    case PCI_VENDOR_3DLABS_CHIP_PERMEDIA:
	PermediaRestore(pScrn, glintReg);
	(*pGlint->RamDac->Restore)(pScrn, pGlint->RamDacRec, RAMDACreg);
	break;
    case PCI_VENDOR_3DLABS_CHIP_500TX:
    case PCI_VENDOR_3DLABS_CHIP_300SX:
    case PCI_VENDOR_3DLABS_CHIP_MX:
    case PCI_VENDOR_3DLABS_CHIP_GAMMA:
	if ( (pGlint->Chipset == PCI_VENDOR_3DLABS_CHIP_GAMMA) &&
	     (pGlint->numMXDevices == 2) ) {
	    DualMXRestore(pScrn, glintReg);
	}
	else {
	    TXRestore(pScrn, glintReg);
	}
	(*pGlint->RamDac->Restore)(pScrn, pGlint->RamDacRec, RAMDACreg);
	break;
    }

    if (pGlint->VGAcore) {
    	vgaHWProtect(pScrn, FALSE);
    }

    if (xf86IsPc98())
       outb(0xfac, 0x01);

    return TRUE;
}

/*
 * Restore the initial (text) mode.
 */
static void 
GLINTRestore(ScrnInfoPtr pScrn)
{
    GLINTPtr pGlint;
    GLINTRegPtr glintReg;
    RamDacHWRecPtr pRAMDAC;
    RamDacRegRecPtr RAMDACreg;

    pGlint = GLINTPTR(pScrn);
    pRAMDAC = RAMDACHWPTR(pScrn);
    glintReg = &pGlint->SavedReg;
    RAMDACreg = &pRAMDAC->SavedReg;

    TRACE_ENTER("GLINTRestore");
    if (pGlint->VGAcore) {
    	vgaHWProtect(pScrn, TRUE);
    }

    switch (pGlint->Chipset) {
    case PCI_VENDOR_TI_CHIP_PERMEDIA2:
    case PCI_VENDOR_3DLABS_CHIP_PERMEDIA2:
	Permedia2VideoLeaveVT(pScrn);
	Permedia2Restore(pScrn, glintReg);
	break;
    case PCI_VENDOR_3DLABS_CHIP_PERMEDIA2V:
	Permedia2VideoLeaveVT(pScrn);
	Permedia2VRestore(pScrn, glintReg);
	break;
    case PCI_VENDOR_3DLABS_CHIP_PERMEDIA3:
	Permedia3Restore(pScrn, glintReg);
	break;
    case PCI_VENDOR_TI_CHIP_PERMEDIA:
    case PCI_VENDOR_3DLABS_CHIP_PERMEDIA:
	PermediaRestore(pScrn, glintReg);
	(*pGlint->RamDac->Restore)(pScrn, pGlint->RamDacRec, RAMDACreg);
	break;
    case PCI_VENDOR_3DLABS_CHIP_500TX:
    case PCI_VENDOR_3DLABS_CHIP_300SX:
    case PCI_VENDOR_3DLABS_CHIP_MX:
    case PCI_VENDOR_3DLABS_CHIP_GAMMA:
	if ( (pGlint->Chipset == PCI_VENDOR_3DLABS_CHIP_GAMMA) &&
	     (pGlint->numMXDevices == 2) ) {
	    DualMXRestore(pScrn, glintReg);
	}
	else {
	    TXRestore(pScrn, glintReg);
	}
	(*pGlint->RamDac->Restore)(pScrn, pGlint->RamDacRec, RAMDACreg);
	break;
    }

    if (pGlint->VGAcore) {
    	vgaHWPtr hwp = VGAHWPTR(pScrn);
    	vgaRegPtr vgaReg = &hwp->SavedReg;
	if (xf86IsPrimaryPci(pGlint->PciInfo)) {
	    vgaHWRestore(pScrn, vgaReg, VGA_SR_MODE | VGA_SR_FONTS);
	} else {
	    vgaHWRestore(pScrn, vgaReg, VGA_SR_MODE);
	}
    	vgaHWProtect(pScrn, FALSE);
    }
    TRACE_EXIT("GLINTRestore");
}


/* Mandatory */

/* This gets called at the start of each server generation */

static Bool
GLINTScreenInit(int scrnIndex, ScreenPtr pScreen, int argc, char **argv)
{
    ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
    GLINTPtr pGlint = GLINTPTR(pScrn);
    int ret, displayWidth;
    unsigned char *FBStart;
    VisualPtr visual;
    
    TRACE_ENTER("GLINTScreenInit");
    /* Map the GLINT memory and MMIO areas */
    if (!GLINTMapMem(pScrn))
	return FALSE;

    /* Initialize the MMIO vgahw functions */
    if (pGlint->VGAcore) {
    	vgaHWPtr hwp;
    	hwp = VGAHWPTR(pScrn);
	if (xf86IsPrimaryPci(pGlint->PciInfo)) {
    	    hwp->MapSize = 0x10000;		/* Standard 64k VGA window */
    	    if (!vgaHWMapMem(pScrn))
	   	 return FALSE;
	}

	/* Timing problem with PM3 & PM2V chips dont like being blasted */
	/* This solves the dual head problem but trahses the console font. */
	if (pGlint->Chipset == PCI_VENDOR_3DLABS_CHIP_PERMEDIA3) {
	    /* Graphics Index VGA register don't work in mmio mode
	     * for the Permedia3 chip, it thrashes the console font. 
	     * Let's keep the IO functions for this instead ... */
	    vgaHWWriteIndexProcPtr writeGr = hwp->writeGr;
	    vgaHWReadIndexProcPtr readGr = hwp->readGr;
    	    vgaHWSetMmioFuncs(hwp, pGlint->IOBaseVGA, 0);
	    hwp->writeGr = writeGr;
	    hwp->readGr = readGr;
	} else if (pGlint->Chipset != PCI_VENDOR_3DLABS_CHIP_PERMEDIA2V)
    	    vgaHWSetMmioFuncs(hwp, pGlint->IOBaseVGA, 0);
    	vgaHWGetIOBase(hwp);
    }

    if (pGlint->FBDev) {
	fbdevHWSave(pScrn);
 	if (!fbdevHWModeInit(pScrn, pScrn->currentMode)) {
		xf86DrvMsg(scrnIndex, X_ERROR,
		   "Internal error: invalid mode\n");
		return FALSE;
	}
    } else
    /* Save the current state */
    GLINTSave(pScrn);

    /* DDC */
    {
	xf86MonPtr pMon = NULL;
	
	if (pGlint->DDCBus)
	    pMon = xf86DoEDID_DDC2(pScrn->scrnIndex, pGlint->DDCBus);
	    
	if (!pMon)
	    /* Try DDC1 */;
	    
	xf86SetDDCproperties(pScrn,xf86PrintEDID(pMon));
    }
    /* Initialise the first mode */
    if ( (!pGlint->FBDev) && !(GLINTModeInit(pScrn, pScrn->currentMode))) {
	xf86DrvMsg(scrnIndex, X_ERROR,
		   "Internal error: invalid mode\n");
	return FALSE;
    }   

    /* Darken the screen for aesthetic reasons and set the viewport */
    GLINTSaveScreen(pScreen, SCREEN_SAVER_ON);
    GLINTAdjustFrame(scrnIndex, pScrn->frameX0, pScrn->frameY0, 0);

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
     * support TrueColor and not DirectColor.  To deal with this, call
     * miSetVisualTypes for each visual supported.
     */

    if((pScrn->overlayFlags & OVERLAY_8_32_PLANAR) && 
						(pScrn->bitsPerPixel == 32)) {
	if (!miSetVisualTypes(8, PseudoColorMask | GrayScaleMask,
			      pScrn->rgbBits, PseudoColor))
		return FALSE;
	if (!miSetVisualTypes(24, TrueColorMask, pScrn->rgbBits, TrueColor))
		return FALSE;
    } else {
	if (!miSetVisualTypes(pScrn->depth,
			      miGetDefaultVisualMask(pScrn->depth),
			      pScrn->rgbBits, pScrn->defaultVisual))
	    return FALSE;
	if (!miSetPixmapDepths())
	    return FALSE;
    }

#ifdef XF86DRI
    /*
     * Setup DRI after visuals have been established, but before fbScreenInit
     * is called. fbScreenInit will eventually call into the drivers
     * InitGLXVisuals call back.
     */
    if (!pGlint->NoAccel && pGlint->HWCursor) {
	pGlint->directRenderingEnabled = GLINTDRIScreenInit(pScreen);
    } else {
	pGlint->directRenderingEnabled = FALSE;
    }
#endif

    /*
     * Call the framebuffer layer's ScreenInit function, and fill in other
     * pScreen fields.
     */

    if(pGlint->ShadowFB) {
 	pGlint->ShadowPitch = BitmapBytePad(pScrn->bitsPerPixel * pScrn->virtualX);
        pGlint->ShadowPtr = xalloc(pGlint->ShadowPitch * pScrn->virtualY);
	displayWidth = pGlint->ShadowPitch / (pScrn->bitsPerPixel >> 3);
        FBStart = pGlint->ShadowPtr;
    } else {
	pGlint->ShadowPtr = NULL;
	displayWidth = pScrn->displayWidth;
	FBStart = pGlint->FbBase;
    }

    switch (pScrn->bitsPerPixel) {
    case 8:
    case 16:
    case 24:
	ret = fbScreenInit(pScreen, FBStart,
			pScrn->virtualX, pScrn->virtualY,
			pScrn->xDpi, pScrn->yDpi,
			displayWidth, pScrn->bitsPerPixel);
#ifdef RENDER
	if (ret)
	    fbPictureInit(pScreen, 0, 0);
#endif
	break;
    case 32:
	if(pScrn->overlayFlags & OVERLAY_8_32_PLANAR)
	    ret = cfb8_32ScreenInit(pScreen, FBStart,
			pScrn->virtualX, pScrn->virtualY,
			pScrn->xDpi, pScrn->yDpi,
			displayWidth);
	else {
	    ret = fbScreenInit(pScreen, FBStart,
			pScrn->virtualX, pScrn->virtualY,
			pScrn->xDpi, pScrn->yDpi,
			displayWidth, pScrn->bitsPerPixel);
#ifdef RENDER
	    if (ret)
	    	fbPictureInit(pScreen, 0, 0);
#endif
	}
	break;
    default:
	xf86DrvMsg(scrnIndex, X_ERROR,
		   "Internal error: invalid bpp (%d) in GLINTScrnInit\n",
		   pScrn->bitsPerPixel);
	    ret = FALSE;
	break;
    }
    if (!ret)
	return FALSE;

    xf86SetBlackWhitePixels(pScreen);

    if (!pGlint->ShadowFB)
	GLINTDGAInit(pScreen);

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

    if (!pGlint->NoAccel) {
        switch (pGlint->Chipset)
        {
        case PCI_VENDOR_TI_CHIP_PERMEDIA2:
        case PCI_VENDOR_3DLABS_CHIP_PERMEDIA2:
        case PCI_VENDOR_3DLABS_CHIP_PERMEDIA2V:
	    Permedia2AccelInit(pScreen);
	    break;
	case PCI_VENDOR_3DLABS_CHIP_PERMEDIA3:
	    Permedia3AccelInit(pScreen);
	    break;
	case PCI_VENDOR_TI_CHIP_PERMEDIA:
	case PCI_VENDOR_3DLABS_CHIP_PERMEDIA:
	    PermediaAccelInit(pScreen);
	    break;
	case PCI_VENDOR_3DLABS_CHIP_500TX:
	case PCI_VENDOR_3DLABS_CHIP_MX:
	case PCI_VENDOR_3DLABS_CHIP_GAMMA:
	    if ( (pGlint->Chipset == PCI_VENDOR_3DLABS_CHIP_GAMMA) &&
	         (pGlint->numMXDevices == 2) ) {
		DualMXAccelInit(pScreen);
	    }
	    else {
		TXAccelInit(pScreen);
	    }
	    break;
	case PCI_VENDOR_3DLABS_CHIP_300SX:
	    SXAccelInit(pScreen);
	    break;
        }
    }

    miInitializeBackingStore(pScreen);
    xf86SetBackingStore(pScreen);
    xf86SetSilkenMouse(pScreen);

    miDCInitialize(pScreen, xf86GetPointerScreenFuncs());

    /* Initialise cursor functions */
    if (pGlint->HWCursor) {
	/* Handle VGA chipsets first */
	if ((pGlint->Chipset == PCI_VENDOR_3DLABS_CHIP_PERMEDIA2) || 
	    (pGlint->Chipset == PCI_VENDOR_TI_CHIP_PERMEDIA2))
	    Permedia2HWCursorInit(pScreen);
	else
	if ((pGlint->Chipset == PCI_VENDOR_3DLABS_CHIP_PERMEDIA2V) ||
	    (pGlint->Chipset == PCI_VENDOR_3DLABS_CHIP_PERMEDIA3))
	    Permedia2vHWCursorInit(pScreen);
	else
	/* If we get here pGlint->Ramdac should have been set */
	if ( ((pGlint->RamDac->RamDacType == (IBM526DB_RAMDAC)) ||
	      (pGlint->RamDac->RamDacType == (IBM526_RAMDAC)) ||
	      (pGlint->RamDac->RamDacType == (IBM640_RAMDAC))) )
	    		glintIBMHWCursorInit(pScreen);
	else
	if ( (pGlint->RamDac->RamDacType == (TI3030_RAMDAC)) ||
	     (pGlint->RamDac->RamDacType == (TI3026_RAMDAC)) )
	    		glintTIHWCursorInit(pScreen);
    }

    /* Initialise default colourmap */
    if (!miCreateDefColormap(pScreen))
	return FALSE;

    if ((pGlint->Chipset == PCI_VENDOR_3DLABS_CHIP_PERMEDIA2V) ||
	(pGlint->Chipset == PCI_VENDOR_3DLABS_CHIP_PERMEDIA3) || 
	(pGlint->Chipset == PCI_VENDOR_3DLABS_CHIP_PERMEDIA2) || 
	(pGlint->Chipset == PCI_VENDOR_TI_CHIP_PERMEDIA2)) {
    	if (!xf86HandleColormaps(pScreen, 256, pScrn->rgbBits,
	    (pGlint->FBDev) ? fbdevHWLoadPalette : 
	    ((pScrn->depth == 16) ? Permedia2LoadPalette16:Permedia2LoadPalette),
	    NULL,
	    CMAP_RELOAD_ON_MODE_SWITCH |
	    ((pScrn->overlayFlags & OVERLAY_8_32_PLANAR) 
					? 0 : CMAP_PALETTED_TRUECOLOR)))
	return FALSE;
    } else {
	if (pScrn->rgbBits == 10) {
    	if (!RamDacHandleColormaps(pScreen, 1024, pScrn->rgbBits, 
	    CMAP_RELOAD_ON_MODE_SWITCH | CMAP_PALETTED_TRUECOLOR)) 
	return FALSE;
	} else {
    	if (!RamDacHandleColormaps(pScreen, 256, pScrn->rgbBits, 
	    CMAP_RELOAD_ON_MODE_SWITCH | 
	    ((pScrn->overlayFlags & OVERLAY_8_32_PLANAR) 
					? 0 : CMAP_PALETTED_TRUECOLOR)))
	return FALSE;
	}
    }

    if((pScrn->overlayFlags & OVERLAY_8_32_PLANAR) && 
						(pScrn->bitsPerPixel == 32)) {
	if(!xf86Overlay8Plus32Init(pScreen))
	    return FALSE;
    }

    if(pGlint->ShadowFB)
	ShadowFBInit(pScreen, GLINTRefreshArea);

#ifdef DPMSExtension
    xf86DPMSInit(pScreen, (DPMSSetProcPtr)GLINTDisplayPowerManagementSet, 0);
#endif

#ifdef XF86DRI
    if (pGlint->directRenderingEnabled) {
	/* Now that mi, cfb, drm and others have done their thing, 
         * complete the DRI setup.
         */
	pGlint->directRenderingEnabled = GLINTDRIFinishScreenInit(pScreen);
    }
    if (pGlint->directRenderingEnabled) {
	xf86DrvMsg(pScrn->scrnIndex, X_INFO,
            "direct rendering enabled\n");
    } else {
	xf86DrvMsg(pScrn->scrnIndex, X_INFO,
            "direct rendering disabled\n");
    }
#endif

    pGlint->CloseScreen = pScreen->CloseScreen;
    pScreen->CloseScreen = GLINTCloseScreen;
    pScreen->SaveScreen = GLINTSaveScreen;

    /* Report any unused options (only for the first generation) */
    if (serverGeneration == 1) {
	xf86ShowUnusedOptions(pScrn->scrnIndex, pScrn->options);
    }

    switch (pGlint->Chipset) {
        case PCI_VENDOR_TI_CHIP_PERMEDIA2:
        case PCI_VENDOR_3DLABS_CHIP_PERMEDIA2:
        case PCI_VENDOR_3DLABS_CHIP_PERMEDIA2V:
	    Permedia2VideoInit(pScreen);
    }

#if 0
    /* Enable the screen */
    GLINTSaveScreen(pScreen, SCREEN_SAVER_OFF);
#endif

    /* Done */
    TRACE_EXIT("GLINTScreenInit");
    return TRUE;
}

/* Usually mandatory */
Bool
GLINTSwitchMode(int scrnIndex, DisplayModePtr mode, int flags)
{
    ScrnInfoPtr pScrn;
    GLINTPtr pGlint;

    pScrn = xf86Screens[scrnIndex];
    pGlint = GLINTPTR(pScrn);
    TRACE_ENTER("GLINTSwitchMode");
	
    if (pGlint->FBDev) {
	Bool ret = fbdevHWSwitchMode(scrnIndex, mode, flags);

	if (!pGlint->NoAccel) {
    	    switch (pGlint->Chipset) {
    		case PCI_VENDOR_TI_CHIP_PERMEDIA2:
    		case PCI_VENDOR_3DLABS_CHIP_PERMEDIA2:
    		case PCI_VENDOR_3DLABS_CHIP_PERMEDIA2V:
			Permedia2InitializeEngine(pScrn);
			break;
    		case PCI_VENDOR_3DLABS_CHIP_PERMEDIA3:
			Permedia3InitializeEngine(pScrn);
			break;
    		case PCI_VENDOR_TI_CHIP_PERMEDIA:
    		case PCI_VENDOR_3DLABS_CHIP_PERMEDIA:
			PermediaInitializeEngine(pScrn);
			break;
    		case PCI_VENDOR_3DLABS_CHIP_500TX:
    		case PCI_VENDOR_3DLABS_CHIP_300SX:
    		case PCI_VENDOR_3DLABS_CHIP_MX:
    		case PCI_VENDOR_3DLABS_CHIP_GAMMA:
			if ((pGlint->Chipset == PCI_VENDOR_3DLABS_CHIP_GAMMA) &&
	     		    (pGlint->numMXDevices == 2)) {
	    			DualMXInitializeEngine(pScrn);
			} else {
	    			TXInitializeEngine(pScrn);
			}
			break;
    	    }
	}

	TRACE_EXIT("GLINTSwitchMode (fbdev ?)");
	return ret;
    }

    TRACE_EXIT("GLINTSwitchMode (normal)");
    return GLINTModeInit(xf86Screens[scrnIndex], mode);
}


/*
 * This function is used to initialize the Start Address - the first
 * displayed location in the video memory.
 */
/* Usually mandatory */
void 
GLINTAdjustFrame(int scrnIndex, int x, int y, int flags)
{
    ScrnInfoPtr pScrn;
    CARD32 base;
    GLINTPtr pGlint;

    pScrn = xf86Screens[scrnIndex];
    pGlint = GLINTPTR(pScrn);
    TRACE_ENTER("GLINTAdjustFrame");
    
    if (pGlint->FBDev) {
    	fbdevHWAdjustFrame(scrnIndex, x, y, flags);
	TRACE_EXIT("GLINTAdjustFrame (fbdev)");
	return;
    }

    base = ((y * pScrn->displayWidth + x) >> 1) >> pGlint->BppShift;
    if (pScrn->bitsPerPixel == 24) base *= 3;
 
    switch (pGlint->Chipset)
    {
    case PCI_VENDOR_TI_CHIP_PERMEDIA:
    case PCI_VENDOR_3DLABS_CHIP_PERMEDIA:
    case PCI_VENDOR_TI_CHIP_PERMEDIA2:
    case PCI_VENDOR_3DLABS_CHIP_PERMEDIA2:
    case PCI_VENDOR_3DLABS_CHIP_PERMEDIA2V:
    case PCI_VENDOR_3DLABS_CHIP_PERMEDIA3:
	GLINT_SLOW_WRITE_REG(base, PMScreenBase);
	break;
    }
    TRACE_EXIT("GLINTAdjustFrame (normal)");
}


/*
 * This is called when VT switching back to the X server.  Its job is
 * to reinitialise the video mode.
 *
 * We may wish to unmap video/MMIO memory too.
 */

/* Mandatory */
static Bool
GLINTEnterVT(int scrnIndex, int flags)
{
    ScrnInfoPtr pScrn = xf86Screens[scrnIndex];
    GLINTPtr pGlint = GLINTPTR(pScrn);

    TRACE_ENTER("GLINTEnterVT");

    if (pGlint->FBDev)
    	fbdevHWEnterVT(scrnIndex, flags);
    else
    	/* Should we re-save the text mode on each VT enter? */
    	if (!GLINTModeInit(pScrn, pScrn->currentMode))
		return FALSE;

    switch (pGlint->Chipset) {
    case PCI_VENDOR_TI_CHIP_PERMEDIA2:
    case PCI_VENDOR_3DLABS_CHIP_PERMEDIA2:
    case PCI_VENDOR_3DLABS_CHIP_PERMEDIA2V:
	Permedia2VideoEnterVT(pScrn);
    }

    if (!pGlint->NoAccel) {
    	switch (pGlint->Chipset) {
    	case PCI_VENDOR_TI_CHIP_PERMEDIA2:
    	case PCI_VENDOR_3DLABS_CHIP_PERMEDIA2:
    	case PCI_VENDOR_3DLABS_CHIP_PERMEDIA2V:
		Permedia2InitializeEngine(pScrn);
		break;
    	case PCI_VENDOR_3DLABS_CHIP_PERMEDIA3:
		Permedia3InitializeEngine(pScrn);
		break;
    	case PCI_VENDOR_TI_CHIP_PERMEDIA:
    	case PCI_VENDOR_3DLABS_CHIP_PERMEDIA:
		PermediaInitializeEngine(pScrn);
		break;
    	case PCI_VENDOR_3DLABS_CHIP_500TX:
    	case PCI_VENDOR_3DLABS_CHIP_300SX:
    	case PCI_VENDOR_3DLABS_CHIP_MX:
    	case PCI_VENDOR_3DLABS_CHIP_GAMMA:
		if ( (pGlint->Chipset == PCI_VENDOR_3DLABS_CHIP_GAMMA) &&
	     		(pGlint->numMXDevices == 2) ) {
	    		DualMXInitializeEngine(pScrn);
		} else {
	    		TXInitializeEngine(pScrn);
		}
		break;
    	}
    }

    TRACE_EXIT("GLINTEnterVTFBDev");
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
GLINTLeaveVT(int scrnIndex, int flags)
{
    ScrnInfoPtr pScrn = xf86Screens[scrnIndex];
    GLINTPtr pGlint = GLINTPTR(pScrn);

    TRACE_ENTER("GLINTLeaveVT");
    GLINTRestore(pScrn);
    if (pGlint->VGAcore)
    	vgaHWLock(VGAHWPTR(pScrn));

    if (xf86IsPc98())
       outb(0xfac, 0x00);

    TRACE_EXIT("GLINTLeaveVT");
}


/*
 * This is called at the end of each server generation.  It restores the
 * original (text) mode.  It should really also unmap the video memory too.
 */

/* Mandatory */
static Bool
GLINTCloseScreen(int scrnIndex, ScreenPtr pScreen)
{
    ScrnInfoPtr pScrn = xf86Screens[scrnIndex];
    GLINTPtr pGlint = GLINTPTR(pScrn);

    TRACE_ENTER("GLINTCloseScreen");
#ifdef XF86DRI
    if (pGlint->directRenderingEnabled) {
	GLINTDRICloseScreen(pScreen);
    }
#endif

    switch (pGlint->Chipset) {
        case PCI_VENDOR_TI_CHIP_PERMEDIA2:
        case PCI_VENDOR_3DLABS_CHIP_PERMEDIA2:
        case PCI_VENDOR_3DLABS_CHIP_PERMEDIA2V:
	    Permedia2VideoUninit(pScrn);
    }

    if (pScrn->vtSema) {
	if(pGlint->CursorInfoRec)
    	    pGlint->CursorInfoRec->HideCursor(pScrn);
	if (pGlint->FBDev)
	    fbdevHWRestore(pScrn);
	else {	
            GLINTRestore(pScrn);
	    if (pGlint->VGAcore)
       	        vgaHWLock(VGAHWPTR(pScrn));
	}
        GLINTUnmapMem(pScrn);
    }
    if(pGlint->AccelInfoRec)
	XAADestroyInfoRec(pGlint->AccelInfoRec);
    if(pGlint->CursorInfoRec)
	xf86DestroyCursorInfoRec(pGlint->CursorInfoRec);
    if (pGlint->ShadowPtr)
	xfree(pGlint->ShadowPtr);
    if (pGlint->DGAModes)
	xfree(pGlint->DGAModes);
    pScrn->vtSema = FALSE;
    
    if (xf86IsPc98())
       outb(0xfac, 0x00);

    pScreen->CloseScreen = pGlint->CloseScreen;
    TRACE_EXIT("GLINTCloseScreen");
    return (*pScreen->CloseScreen)(scrnIndex, pScreen);
}


/* Free up any per-generation data structures */

/* Optional */
static void
GLINTFreeScreen(int scrnIndex, int flags)
{
    ScrnInfoPtr pScrn = xf86Screens[scrnIndex];
    GLINTPtr pGlint = GLINTPTR(pScrn);

    TRACE_ENTER("GLINTFreeScreen");
    if ((pGlint->FBDev || FBDevProbed) && xf86LoaderCheckSymbol("fbdevHWFreeRec"))
	fbdevHWFreeRec(xf86Screens[scrnIndex]);
    if (pGlint->VGAcore && xf86LoaderCheckSymbol("vgaHWFreeHWRec"))
    	vgaHWFreeHWRec(xf86Screens[scrnIndex]);
    if (pGlint->RamDacRec && xf86LoaderCheckSymbol("RamDacFreeRec"))
    	RamDacFreeRec(xf86Screens[scrnIndex]);
    GLINTFreeRec(xf86Screens[scrnIndex]);
    TRACE_EXIT("GLINTFreeScreen");
}


/* Checks if a mode is suitable for the selected chipset. */

/* Optional */
static int
GLINTValidMode(int scrnIndex, DisplayModePtr mode, Bool verbose, int flags)
{
    ScrnInfoPtr pScrn = xf86Screens[scrnIndex];
    GLINTPtr pGlint = GLINTPTR(pScrn);

    if (mode->Flags & V_INTERLACE)
	return(MODE_NO_INTERLACE);
    
    if (pScrn->bitsPerPixel == 24) {
	/* A restriction on the PM2 where a black strip on the left hand
	 * side appears if not aligned properly */
        switch (pGlint->Chipset) {
        case PCI_VENDOR_TI_CHIP_PERMEDIA2:
        case PCI_VENDOR_3DLABS_CHIP_PERMEDIA3:
        case PCI_VENDOR_3DLABS_CHIP_PERMEDIA2V:
        case PCI_VENDOR_3DLABS_CHIP_PERMEDIA2:
          if (mode->HDisplay % 8) {
	    xf86DrvMsg(pScrn->scrnIndex, X_INFO, 
	      "HDisplay %d not divisible by 8, fixing...\n", mode->HDisplay);
	    mode->HDisplay -= (mode->HDisplay % 8);
	    mode->CrtcHDisplay = mode->CrtcHBlankStart = mode->HDisplay;
          }
	
          if (mode->HSyncStart % 8) {
	    xf86DrvMsg(pScrn->scrnIndex, X_INFO, 
	     "HSyncStart %d not divisible by 8, fixing...\n", mode->HSyncStart);
	    mode->HSyncStart -= (mode->HSyncStart % 8);
	    mode->CrtcHSyncStart = mode->HSyncStart;
          }

          if (mode->HSyncEnd % 8) {
	    xf86DrvMsg(pScrn->scrnIndex, X_INFO, 
	      "HSyncEnd %d not divisible by 8, fixing...\n", mode->HSyncEnd);
	    mode->HSyncEnd -= (mode->HSyncEnd % 8);
	    mode->CrtcHSyncEnd = mode->HSyncEnd;
          }

          if (mode->HTotal % 8) {
	    xf86DrvMsg(pScrn->scrnIndex, X_INFO, 
	      "HTotal %d not divisible by 8, fixing...\n", mode->HTotal);
	    mode->HTotal -= (mode->HTotal % 8);
	    mode->CrtcHBlankEnd = mode->CrtcHTotal = mode->HTotal;
          }
          break;
	}
    }

    return(MODE_OK);
}

/* Do screen blanking */

/* Mandatory */
static Bool
GLINTSaveScreen(ScreenPtr pScreen, int mode)
{
    ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
    GLINTPtr pGlint = GLINTPTR(pScrn);
    CARD32 temp;
    Bool unblank;

    TRACE_ENTER("GLINTSaveScreen");

    unblank = xf86IsUnblank(mode);

    if (unblank)
	SetTimeSinceLastInputEvent();

    if ((pScrn != NULL ) && pScrn->vtSema) {
	switch (pGlint->Chipset) {
	case PCI_VENDOR_TI_CHIP_PERMEDIA2:
	case PCI_VENDOR_TI_CHIP_PERMEDIA:
	case PCI_VENDOR_3DLABS_CHIP_PERMEDIA3:
	case PCI_VENDOR_3DLABS_CHIP_PERMEDIA2V:
	case PCI_VENDOR_3DLABS_CHIP_PERMEDIA2:
	case PCI_VENDOR_3DLABS_CHIP_PERMEDIA:
	    temp = GLINT_READ_REG(PMVideoControl);
	    if (unblank) temp |= 1;
	    else	     temp &= 0xFFFFFFFE;
	    GLINT_SLOW_WRITE_REG(temp, PMVideoControl);
	    break;
	case PCI_VENDOR_3DLABS_CHIP_500TX:
	case PCI_VENDOR_3DLABS_CHIP_300SX:
	case PCI_VENDOR_3DLABS_CHIP_MX:
	    break;
	}
    }

    TRACE_EXIT("GLINTSaveScreen");
    return TRUE;
}

#ifdef DEBUG
void
GLINT_VERB_WRITE_REG(GLINTPtr pGlint, CARD32 v, int r, char *file, int line)
{
    if (xf86GetVerbosity() > 2)
	ErrorF("[0x%04x] <- 0x%08x (%s, %d)\n",	r, v, file, line);
    *(volatile CARD32 *)((char *) pGlint->IOBase + r) = v;
}

CARD32
GLINT_VERB_READ_REG(GLINTPtr pGlint, CARD32 r, char *file, int line)
{
    CARD32 v = *(volatile CARD32 *)((char *) pGlint->IOBase + r);

    if (xf86GetVerbosity() > 2)
	ErrorF("[0x%04x] -> 0x%08x (%s, %d)\n", r, v, file, line);
    return v;
}
#endif

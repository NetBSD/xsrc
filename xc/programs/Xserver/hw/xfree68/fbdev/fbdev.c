/* $XConsortium: fbdev.c /main/1 1996/09/21 11:18:10 kaleb $ */




/* $XFree86: xc/programs/Xserver/hw/xfree68/fbdev/fbdev.c,v 3.4.2.10 1998/11/08 11:49:18 hohndel Exp $ */
/*
 *
 *  Author: Martin Schaller. Taken from hga2.c
 *
 *  Generic version by Geert Uytterhoeven (Geert.Uytterhoeven@cs.kuleuven.ac.be)
 *
 *  This version contains support for:
 *
 *	- Monochrome, 1 bitplane [mfb]
 *	- Color, 2/4/8 interleaved bitplanes with 2 bytes interleave [iplan2p?]
 *	- Color, interleaved bitplanes [ilbm]
 *	- Color, normal bitplanes [afb]
 *	- Color, chunky 8 bits per pixel [cfb8]
 *	- Color, chunky 16 bits per pixel [cfb16]
 *	- Color, chunky 24 bits per pixel [cfb24]
 *	- Color, chunky 32 bits per pixel [cfb32]
 */


#define fbdev_PATCHLEVEL "10"

#define DIRECTCOLORHACK		/* hack for directcolor */

#include "X.h"
#include "input.h"
#include "scrnintstr.h"
#include "pixmapstr.h"
#include "regionstr.h"
#include "mipointer.h"
#include "cursorstr.h"
#include "gcstruct.h"

#include "compiler.h"

#include "xf86.h"
#include "xf86Priv.h"
#include "xf86_OSlib.h"
#include "xf86_Config.h"
#include "mfb.h"
#ifdef CONFIG_ILBM
#include "ilbm.h"
#endif /* CONFIG_ILBM */
#ifdef CONFIG_AFB
#include "afb.h"
#endif /* CONFIG_AFB */
#include <linux/fb.h>
#include <asm/page.h>
#include <stdlib.h>

#include "colormapst.h"
#include "resource.h"

extern char *fb_dev_name;	/* os-support/linux/lnx_init.c */


static int *fbdevPrivateIndexP;
void (*fbdevBitBlt)() = NULL;
static int (*CreateDefColormap)(ScreenPtr);

extern int mfbCreateDefColormap(ScreenPtr);

#if defined(CONFIG_IPLAN2p2) || defined(CONFIG_IPLAN2p4) || \
     defined(CONFIG_IPLAN2p8)
extern int iplCreateDefColormap(ScreenPtr);
#ifdef CONFIG_IPLAN2p2
extern int ipl2p2ScreenPrivateIndex;
extern void ipl2p2DoBitblt();
#endif /* CONFIG_IPLAN2p2 */
#ifdef CONFIG_IPLAN2p4
extern int ipl2p4ScreenPrivateIndex;
extern void ipl2p4DoBitblt();
#endif /* CONFIG_IPLAN2p4 */
#ifdef CONFIG_IPLAN2p8
extern int ipl2p8ScreenPrivateIndex;
extern void ipl2p8DoBitblt();
#endif /* CONFIG_IPLAN2p8 */
#endif /* CONFIG_IPLAN2p2 || CONFIG_IPLAN2p4 || CONFIG_IPLAN2p8 */

#ifdef CONFIG_ILBM
extern int ilbmCreateDefColormap(ScreenPtr);
extern int ilbmScreenPrivateIndex;
extern void ilbmDoBitblt();
#endif /* CONFIG_ILBM */
#ifdef CONFIG_AFB
extern int afbCreateDefColormap(ScreenPtr);
extern int afbScreenPrivateIndex;
extern void afbDoBitblt();
#endif /* CONFIG_AFB */

#ifdef CONFIG_CFB8
extern int cfbCreateDefColormap(ScreenPtr);
extern void cfbDoBitblt();
#endif /* CONFIG_CFB8 */

#ifdef CONFIG_CFB16
extern int cfb16ScreenPrivateIndex;
extern void cfb16DoBitblt();
#endif /* CONFIG_CFB16 */

#ifdef CONFIG_CFB24
extern int cfb24ScreenPrivateIndex;
extern void cfb24DoBitblt();
#endif /* CONFIG_CFB24 */

#ifdef CONFIG_CFB32
extern int cfb32ScreenPrivateIndex;
extern void cfb32DoBitblt();
#endif /* CONFIG_CFB32 */

#if defined(CONFIG_CFB16) || defined(CONFIG_CFB24) || defined(CONFIG_CFB32)
extern int cfbListInstalledColormaps(ScreenPtr pScreen, Colormap *pmaps);
extern void cfbInstallColormap(ColormapPtr pmap);
extern void cfbUninstallColormap(ColormapPtr pmap);
#endif /* CONFIG_CFB16 || CONFIG_CFB24 || CONFIG_CFB32 */

#ifdef CONFIG_77C32
extern int ncr77c32_init(ScreenPtr pScreen);
#else
#define ncr77c32_init			NULL
#endif
#ifdef CONFIG_MACH64
extern int mach64_gx_init(ScreenPtr pScreen);
extern int mach64_ct_init(ScreenPtr pScreen);
extern int mach64_vt_init(ScreenPtr pScreen);
extern int mach64_gt_init(ScreenPtr pScreen);
#else
#define mach64_gx_init			NULL
#define mach64_ct_init			NULL
#define mach64_vt_init			NULL
#define mach64_gt_init			NULL
#endif
#ifdef CONFIG_IMSTT
extern int imstt_init(ScreenPtr pScreen);
#else
#define imstt_init			NULL
#endif
#ifdef CONFIG_MGA
extern int mga_init(ScreenPtr pScreen);
#else
#define mga_init			NULL
#endif


extern int fbdevValidTokens[];


static Bool fbdevProbe(void);
static void fbdevPrintIdent(void);
static Bool fbdevSaveScreen(ScreenPtr pScreen, int on);
static Bool fbdevScreenInit(int scr_index, ScreenPtr pScreen, int argc,
			    char **argv);
static void fbdevEnterLeaveVT(Bool enter, int screen_idx);
static void fbdevAdjustFrame(int x, int y);
static Bool fbdevCloseScreen(int screen_idx, ScreenPtr screen);
static Bool fbdevValidMode(DisplayModePtr mode, Bool verbose, int flag);
static Bool fbdevSwitchMode(DisplayModePtr mode);


static void xfree2fbdev(DisplayModePtr mode, struct fb_var_screeninfo *var);
static void fbdev2xfree(struct fb_var_screeninfo *var, DisplayModePtr mode);


extern Bool xf86Exiting, xf86Resetting, xf86ProbeFailed;

ScrnInfoRec fbdevInfoRec = {
    FALSE,			/* Bool configured */
    -1,				/* int tmpIndex */
    -1,				/* int scrnIndex */
    fbdevProbe,			/* Bool (*Probe)() */
    fbdevScreenInit,		/* Bool (*Init)() */
    (int (*)())NoopDDA,		/* int (*ValidMode)() */
    fbdevEnterLeaveVT,		/* void (*EnterLeaveVT)() */
    (void (*)())NoopDDA,	/* void (*EnterLeaveMonitor)() */
    (void (*)())NoopDDA,	/* void (*EnterLeaveCursor)() */
    (void (*)())NoopDDA,	/* void (*AdjustFrame)() */
    (Bool (*)())NoopDDA,	/* Bool (*SwitchMode)() */
    (void (*)())NoopDDA,	/* void (*DPMSSet)() */
    fbdevPrintIdent,		/* void (*PrintIdent)() */
    8,				/* int depth */
    {0, },			/* xrgb weight */
    8,				/* int bitsPerPixel */
    PseudoColor,		/* int defaultVisual */
    -1, -1,			/* int virtualX,virtualY */
    -1,				/* int displayWidth */
    -1, -1, -1, -1,		/* int frameX0, frameY0, frameX1, frameY1 */
    {0, },			/* OFlagSet options */
    {0, },			/* OFlagSet clockOptions */
    {0, },			/* OFlagSet xconfigFlag */
    NULL,			/* char *chipset */
    NULL,			/* char *ramdac */
    {0, },			/* int dacSpeeds[MAXDACSPEEDS] */
    0,				/* int dacSpeedBpp */
    0,				/* int clocks */
    {0, },			/* int clock[MAXCLOCKS] */
    0,				/* int maxClock */
    0,				/* int videoRam */
    0,				/* int BIOSbase */
    0,				/* unsigned long MemBase */
    240, 180,			/* int width, height */
    0,				/* unsigned long speedup */
    NULL,			/* DisplayModePtr modes */
    NULL,			/* MonPtr monitor */
    NULL,			/* char *clockprog */
    -1,				/* int textclock */
    FALSE,			/* Bool bankedMono */
    "FBDev",			/* char *name */
    {0, },			/* xrgb blackColour */
    {0, },			/* xrgb whiteColour */
    fbdevValidTokens,		/* int *validTokens */
    fbdev_PATCHLEVEL,		/* char *patchLevel */
    0,				/* unsigned int IObase */
    0,				/* unsigned int DACbase */
    0,				/* unsigned int COPbase */
    0,				/* unsigned int POSbase */
    0,				/* unsigned int instance */
    0,				/* int s3Madjust */
    0,				/* int s3Nadjust */
    0,				/* int s3MClk */
    0,				/* int chipID */
    0,				/* int chipRev */
    0,				/* unsigned long VGAbase */
    0,				/* int s3RefClk */
    -1,				/* int s3BlankDelay */
    0,				/* int textClockFreq */
    NULL,			/* char *DCConfig */
    NULL,			/* char *DCOptions */
    0,				/* int MemClk */
    0,				/* int LCDClk */
#ifdef XFreeXDGA
    0,				/* int directMode */
    NULL,			/* void (*setBank)() */
    0,				/* unsigned long physBase */
    0,				/* int physSize */
#endif
#ifdef XF86SETUP
    NULL,			/* void *device */
#endif


};

pointer fbdevVirtBase = NULL;
pointer fbdevRegBase = NULL;

static ScreenPtr savepScreen = NULL;
static PixmapPtr ppix = NULL;

extern miPointerScreenFuncRec xf86PointerScreenFuncs;

#define NOMAPYET	(ColormapPtr)0

static ColormapPtr InstalledMaps[MAXSCREENS];
	/* current colormap for each screen */


static int fb_fd = -1;
static struct fb_fix_screeninfo fb_fix;
static struct fb_var_screeninfo fb_var;

static unsigned long smem_start, smem_offset, smem_len;
static unsigned long mmio_start, mmio_offset, mmio_len;
static pointer smem_base, mmio_base;

    /*
     *  These aren't defined until the most recent kernels
     */

#ifndef FB_ACCEL_IMS_TWINTURBO
#define FB_ACCEL_IMS_TWINTURBO		14
#endif
#ifndef FB_ACCEL_3DLABS_PERMEDIA2
#define FB_ACCEL_3DLABS_PERMEDIA2	15
#endif
#ifndef FB_ACCEL_MATROX_MGA2064W
#define FB_ACCEL_MATROX_MGA2064W	16
#endif
#ifndef FB_ACCEL_MATROX_MGA1064SG
#define FB_ACCEL_MATROX_MGA1064SG	17
#endif
#ifndef FB_ACCEL_MATROX_MGA2164W
#define FB_ACCEL_MATROX_MGA2164W	18
#endif
#ifndef FB_ACCEL_MATROX_MGA2164W_AGP
#define FB_ACCEL_MATROX_MGA2164W_AGP	19
#endif
#ifndef FB_ACCEL_MATROX_MGAG100
#define FB_ACCEL_MATROX_MGAG100		20
#endif
#ifndef FB_ACCEL_MATROX_MGAG200
#define FB_ACCEL_MATROX_MGAG200		21
#endif

static struct accelentry {
    CARD32 id;
    const char *name;
    int (*init)(ScreenPtr pScreen);
} acceltab[] = {
    { FB_ACCEL_NONE, "None", NULL },
    { FB_ACCEL_ATARIBLITT, "Atari Blitter", NULL },
    { FB_ACCEL_AMIGABLITT, "Amiga Blitter", NULL },
    { FB_ACCEL_S3_TRIO64, "S3 Trio64", NULL },
    { FB_ACCEL_NCR_77C32BLT, "NCR 77C32BLT", ncr77c32_init },
    { FB_ACCEL_S3_VIRGE, "S3 ViRGE", NULL },
    { FB_ACCEL_ATI_MACH64GX, "ATI Mach64GX", mach64_gx_init },
    { FB_ACCEL_DEC_TGA, "DEC 21030 TGA", NULL },
    { FB_ACCEL_ATI_MACH64CT, "ATI Mach64CT", mach64_ct_init },
    { FB_ACCEL_ATI_MACH64VT, "ATI Mach64VT", mach64_vt_init },
    { FB_ACCEL_ATI_MACH64GT, "ATI Mach64GT (3D RAGE)", mach64_gt_init },
    { FB_ACCEL_SUN_CREATOR, "Sun Creator/Creator3D", NULL },
    { FB_ACCEL_SUN_CGSIX, "Sun cg6", NULL },
    { FB_ACCEL_SUN_LEO, "Sun leo/zx", NULL },
    { FB_ACCEL_IMS_TWINTURBO, "IMS Twin Turbo", imstt_init },
    { FB_ACCEL_3DLABS_PERMEDIA2, "3Dlabs Permedia 2", NULL },
    { FB_ACCEL_MATROX_MGA2064W, "Matrox MGA2064W (Millennium)", mga_init },
    { FB_ACCEL_MATROX_MGA1064SG, "Matrox MGA1064SG (Mystique)", mga_init },
    { FB_ACCEL_MATROX_MGA2164W, "Matrox MGA2164W (Millennium II)", mga_init },
    { FB_ACCEL_MATROX_MGA2164W_AGP, "Matrox MGA2164W (Millennium II AGP)", mga_init },
    { FB_ACCEL_MATROX_MGAG100, "Matrox G100 (Productiva G100)", mga_init },
    { FB_ACCEL_MATROX_MGAG200, "Matrox G200 (Millennium, Mystique)", mga_init },
};

static Bool UseModeDB = FALSE;

static unsigned int BitsPerRGB, ColorMapSize;
static unsigned int RedMask, GreenMask, BlueMask, TranspMask;

#define StaticGrayMask	(1 << StaticGray)
#define GrayScaleMask	(1 << GrayScale)
#define StaticColorMask	(1 << StaticColor)
#define PseudoColorMask	(1 << PseudoColor)
#define TrueColorMask	(1 << TrueColor)
#define DirectColorMask	(1 << DirectColor)

static int Visuals;


static void open_framebuffer(void)
{
    if (fb_fd == -1)
	if ((fb_fd = open(fb_dev_name, O_RDWR)) < 0)
	    FatalError("open_framebuffer: failed to open %s (%s)\n",
	    	       fb_dev_name, strerror(errno));
}

static void close_framebuffer(void)
{
    if (fb_fd != -1) {
	close(fb_fd);
	fb_fd = -1;
    }
}

pointer xf86MapVidMem(int ScreenNum, int Region, pointer Base,
		      unsigned long Size)
{
    open_framebuffer();
    smem_base = (pointer)mmap(NULL, smem_len, PROT_READ | PROT_WRITE,
			      MAP_SHARED, fb_fd, (off_t)0);
    if ((long)smem_base == -1)
	FatalError("xf86MapVidMem: Could not mmap framebuffer (%s)\n",
		   strerror(errno));
    smem_base = (char *)smem_base + smem_offset;
    if (mmio_len) {
	mmio_base = (pointer)mmap(NULL, mmio_len, PROT_READ | PROT_WRITE,
				  MAP_SHARED, fb_fd, smem_len);
	if ((long)mmio_base == -1)
	    FatalError("xf86MapVidMem: Could not mmap MMIO registers (%s)\n",
		       strerror(errno));
	mmio_base = (char *)mmio_base + mmio_offset;
    } else
	mmio_base = 0;
    return smem_base;
}

void xf86UnMapVidMem(int ScreenNum, int Region, pointer Base,
		     unsigned long Size)
{
    if ((long)mmio_base != -1)
	munmap((caddr_t)((unsigned long)mmio_base & PAGE_MASK), mmio_len);
    if ((long)smem_base != -1)
	munmap((caddr_t)((unsigned long)smem_base & PAGE_MASK), smem_len);
    close_framebuffer();
}

static void fbdevUpdateColormap(int dex, int count, unsigned short *rmap,
				unsigned short *gmap, unsigned short *bmap)
{
    struct fb_cmap cmap;

    if (!xf86VTSema)
	/* Switched away from server vt, do nothing. */
	return;

    cmap.start = dex;
    cmap.len = count;
    cmap.red = rmap;
    cmap.green = gmap;
    cmap.blue = bmap;
    cmap.transp = NULL;

    if (ioctl(fb_fd, FBIOPUTCMAP, &cmap) < 0)
	FatalError("fbdevUpdateColormap: FBIOPUTCMAP failed (%s)\n",
		   strerror(errno));
}

static int fbdevListInstalledColormaps(ScreenPtr pScreen, Colormap *pmaps)
{
    *pmaps = InstalledMaps[pScreen->myNum]->mid;
    return 1;
}

static void fbdevStoreColors(ColormapPtr pmap, int ndef, xColorItem *pdefs)
{
    xColorItem *directDefs = NULL;

    if (pmap != InstalledMaps[pmap->pScreen->myNum])
	return;

#if defined(CONFIG_CFB16) || defined(CONFIG_CFB24) || defined(CONFIG_CFB32)
    if ((pmap->pVisual->class | DynamicClass) == DirectColor) {
	directDefs = (xColorItem *)
			ALLOCATE_LOCAL(ColorMapSize*sizeof(xColorItem));
	ndef = cfbExpandDirectColors (pmap, ndef, pdefs, directDefs);
	pdefs = directDefs;
    }
#endif /* CONFIG_CFB16 || CONFIG_CFB24 || CONFIG_CFB32 */

    while (ndef--) {
	fbdevUpdateColormap(pdefs->pixel, 1, &pdefs->red, &pdefs->green,
			    &pdefs->blue);
	pdefs++;
    }

    if (directDefs)
	DEALLOCATE_LOCAL(directDefs);
}

static void fbdevInstallColormap(ColormapPtr pmap)
{
    ColormapPtr oldmap = InstalledMaps[pmap->pScreen->myNum];
    int entries;
    Pixel *ppix;
    xrgb *prgb;
    xColorItem *defs;
    int i;

    if (pmap == oldmap)
	return;

    if ((pmap->pVisual->class | DynamicClass) == DirectColor)
	entries = (pmap->pVisual->redMask |
		   pmap->pVisual->greenMask |
		   pmap->pVisual->blueMask) + 1;
    else
	entries = pmap->pVisual->ColormapEntries;

    ppix = (Pixel *)ALLOCATE_LOCAL(entries*sizeof(Pixel));
    prgb = (xrgb *)ALLOCATE_LOCAL(entries*sizeof(xrgb));
    defs = (xColorItem *)ALLOCATE_LOCAL(entries*sizeof(xColorItem));

    if (oldmap != NOMAPYET)
	WalkTree(pmap->pScreen, TellLostMap, &oldmap->mid);

    InstalledMaps[pmap->pScreen->myNum] = pmap;

    for (i = 0; i < entries; i++)
	ppix[i] = i;
    QueryColors(pmap, entries, ppix, prgb);

    for (i = 0 ; i < entries; i++) {
	defs[i].pixel = ppix[i];
	defs[i].red = prgb[i].red;
	defs[i].green = prgb[i].green;
	defs[i].blue = prgb[i].blue;
	defs[i].flags =  DoRed|DoGreen|DoBlue;
    }

    fbdevStoreColors(pmap, entries, defs);

    WalkTree(pmap->pScreen, TellGainedMap, &pmap->mid);

    DEALLOCATE_LOCAL(ppix);
    DEALLOCATE_LOCAL(prgb);
    DEALLOCATE_LOCAL(defs);
}

static void fbdevUninstallColormap(ColormapPtr pmap)
{
    ColormapPtr defColormap;

    if (pmap != InstalledMaps[pmap->pScreen->myNum])
	return;
    defColormap = (ColormapPtr)LookupIDByType(pmap->pScreen->defColormap,
    					      RT_COLORMAP);
    if (defColormap == InstalledMaps[pmap->pScreen->myNum])
	return;
    (*pmap->pScreen->InstallColormap)(defColormap);
}

/*
 *  fbdevPrintIdent -- Prints out identifying strings for drivers included in
 *		       the server
 */

static void fbdevPrintIdent(void)
{
    ErrorF("   %s: Server for frame buffer device\n", fbdevInfoRec.name);
    ErrorF("   (Patchlevel %s): mfb", fbdevInfoRec.patchLevel);
#ifdef CONFIG_IPLAN2p2
    ErrorF(", iplan2p2");
#endif
#ifdef CONFIG_IPLAN2p4
    ErrorF(", iplan2p4");
#endif
#ifdef CONFIG_IPLAN2p8
    ErrorF(", iplan2p8");
#endif
#ifdef CONFIG_ILBM
    ErrorF(", ilbm");
#endif
#ifdef CONFIG_AFB
    ErrorF(", afb");
#endif
#ifdef CONFIG_CFB8
    ErrorF(", cfb8");
#endif
#ifdef CONFIG_CFB16
    ErrorF(", cfb16");
#endif
#ifdef CONFIG_CFB24
    ErrorF(", cfb24");
#endif
#ifdef CONFIG_CFB32
    ErrorF(", cfb32");
#endif
#ifdef CONFIG_77C32
    ErrorF(", NCR 77C32BLT (accel)");
#endif
#ifdef CONFIG_MACH64
    ErrorF(", ATI Mach64 (accel)");
#endif
#ifdef CONFIG_IMSTT
    ErrorF(", IMS TwinTurbo (accel)");
#endif
#ifdef CONFIG_MGA
    ErrorF(", Matrox MGA (accel)");
#endif
    ErrorF("\n");
}


static Bool fbdevLookupMode(DisplayModePtr target)
{
    DisplayModePtr p;
    Bool found_mode = FALSE;

    for (p = fbdevInfoRec.monitor->Modes; p != NULL; p = p->next)
	if (!strcmp(p->name, target->name)) {
	    target->Clock          = p->Clock;
	    target->HDisplay       = p->HDisplay;
	    target->HSyncStart     = p->HSyncStart;
	    target->HSyncEnd       = p->HSyncEnd;
	    target->HTotal         = p->HTotal;
	    target->VDisplay       = p->VDisplay;
	    target->VSyncStart     = p->VSyncStart;
	    target->VSyncEnd       = p->VSyncEnd;
	    target->VTotal         = p->VTotal;
	    target->Flags          = p->Flags;
	    target->SynthClock     = p->SynthClock;
	    target->CrtcHDisplay   = p->CrtcHDisplay;
	    target->CrtcHSyncStart = p->CrtcHSyncStart;
	    target->CrtcHSyncEnd   = p->CrtcHSyncEnd;
	    target->CrtcHTotal     = p->CrtcHTotal;
	    target->CrtcVDisplay   = p->CrtcVDisplay;
	    target->CrtcVSyncStart = p->CrtcVSyncStart;
	    target->CrtcVSyncEnd   = p->CrtcVSyncEnd;
	    target->CrtcVTotal     = p->CrtcVTotal;
	    target->CrtcHAdjusted  = p->CrtcHAdjusted;
	    target->CrtcVAdjusted  = p->CrtcVAdjusted;
	    if (fbdevValidMode(target,FALSE,MODE_USED)) {
		found_mode = TRUE;
		break;
	    }
	}
    return found_mode;
}


/*
 *  fbdevProbe -- Probe and initialize the hardware driver
 */

static Bool fbdevProbe(void)
{
    DisplayModePtr pMode, pEnd;

    /*
     * fb_dev_name also serves as global flag to tell the world that this is
     * the FBDev server (actually, this is to tell the code in lnx_init.c)
     */
    fb_dev_name = getenv("FRAMEBUFFER");
    if (!fb_dev_name)
	fb_dev_name = "/dev/fb0";

    open_framebuffer();

    if (ioctl(fb_fd, FBIOGET_VSCREENINFO, &fb_var))
	FatalError("fbdevProbe: unable to get screen params (%s)\n",
		   strerror(errno));

    pMode = fbdevInfoRec.modes;
    if (pMode == NULL)
	FatalError("No modes supplied in XF86Config\n");

    if (!strcmp(pMode->name, "default")) {
	ErrorF("%s %s: Using default frame buffer video mode\n", XCONFIG_GIVEN,
	       fbdevInfoRec.name);
	fb_var.bits_per_pixel = fbdevInfoRec.depth;
	fbdevInfoRec.bitsPerPixel = fbdevInfoRec.depth;
    } else {
	ErrorF("%s %s: Using XF86Config video mode database\n", XCONFIG_GIVEN,
	       fbdevInfoRec.name);
	UseModeDB = TRUE;
	pEnd = NULL;
	fbdevInfoRec.bitsPerPixel = fbdevInfoRec.depth;
	do {
	    DisplayModePtr pModeSv;

	    pModeSv = pMode->next;
	    if (!fbdevLookupMode(pMode))
		xf86DeleteMode(&fbdevInfoRec, pMode);
	    else {
		/*
		 *  Successfully looked up this mode.  If pEnd isn't
		 *  initialized, set it to this mode.
		 */
		if (pEnd == (DisplayModePtr)NULL)
		    pEnd = pMode;
	    }
	    pMode = pModeSv;
	} while (pMode != pEnd);
	fbdevInfoRec.SwitchMode = fbdevSwitchMode;
    }

    return TRUE;
}

static void fbdevRestoreColors(ScreenPtr pScreen)
{
    ColormapPtr pmap = InstalledMaps[pScreen->myNum];
    int entries;

    if (!pmap)
	pmap = (ColormapPtr) LookupIDByType(pScreen->defColormap, RT_COLORMAP);
    entries = pmap->pVisual->ColormapEntries;
    if (entries) {
	Pixel *ppix;
	xrgb *prgb;
	xColorItem *defs;
	int i;

	ppix = (Pixel *)ALLOCATE_LOCAL(entries*sizeof(Pixel));
	prgb = (xrgb *)ALLOCATE_LOCAL(entries*sizeof(xrgb));
	defs = (xColorItem *)ALLOCATE_LOCAL(entries*sizeof(xColorItem));

	for (i = 0 ; i < entries; i++)
	    ppix[i] = i;
	QueryColors(pmap, entries, ppix, prgb);

	for (i = 0 ; i < entries; i++) {
	    defs[i].pixel = ppix[i];
	    defs[i].red = prgb[i].red;
	    defs[i].green = prgb[i].green;
	    defs[i].blue = prgb[i].blue;
	}

	fbdevStoreColors(pmap, entries, defs);

	DEALLOCATE_LOCAL(ppix);
	DEALLOCATE_LOCAL(prgb);
	DEALLOCATE_LOCAL(defs);
    }
}

#ifdef DIRECTCOLORHACK
static void fbdevSetLinearColormap(const struct fb_var_screeninfo *var)
{
    int i;
    unsigned short *rmap, *gmap, *bmap;

    rmap = (unsigned short *)
		ALLOCATE_LOCAL(ColorMapSize*sizeof(unsigned short));
    gmap = (unsigned short *)
		ALLOCATE_LOCAL(ColorMapSize*sizeof(unsigned short));
    bmap = (unsigned short *)
		ALLOCATE_LOCAL(ColorMapSize*sizeof(unsigned short));
    for (i = 0; i < ColorMapSize; i++) {
	float val = 65535.0*i/(ColorMapSize-1);
	rmap[i] = val;
	gmap[i] = val;
	bmap[i] = val;
    }
    fbdevUpdateColormap(0, ColorMapSize, rmap, gmap, bmap);
    DEALLOCATE_LOCAL(rmap);
    DEALLOCATE_LOCAL(gmap);
    DEALLOCATE_LOCAL(bmap);
}
#endif

static Bool fbdevSaveScreen(ScreenPtr pScreen, int on)
{
    switch (on) {
	case SCREEN_SAVER_ON:
	    {
		unsigned short *map;
		int i;

		map = (unsigned short *)
			ALLOCATE_LOCAL(ColorMapSize*sizeof(unsigned short));

		for (i = 0; i < ColorMapSize; i++)
		    map[i] = 0;
		fbdevUpdateColormap(0, ColorMapSize, map, map, map);
		return TRUE;
	    }

	case SCREEN_SAVER_OFF:
#ifdef DIRECTCOLORHACK
	    if (fb_fix.visual == FB_VISUAL_DIRECTCOLOR)
		fbdevSetLinearColormap(&fb_var);
	    else
#endif
	    fbdevRestoreColors(pScreen);
	    return TRUE;
    }
    return FALSE;
}


/*
 *  fbdevScreenInit -- Attempt to find and initialize a framebuffer
 *		       Most of the elements of the ScreenRec are filled in.
 *		       The video is enabled for the frame buffer...
 *
 *  Arguments:	scr_index	: The index of pScreen in the ScreenInfo
 *		pScreen		: The Screen to initialize
 *		argc		: The number of the Server's arguments.
 *		argv		: The arguments themselves. Don't change!
 */

static Bool fbdevScreenInit(int scr_index, ScreenPtr pScreen, int argc,
			    char **argv)
{
    int displayResolution = 75;	/* default to 75dpi */
    int dxres, dyres;
    extern int monitorResolution;
    struct fb_var_screeninfo *var = &fb_var;
    static unsigned short bw[] = {
	0xffff, 0x0000
    };
    DisplayModePtr mode;
    int i, bpp, xsize, ysize, width;
    char *fbtype;
    VisualPtr visual;
    struct accelentry *accelentry = NULL;

    open_framebuffer();

    /*
     *  Take display resolution from the -dpi flag if specified
     */

    if (monitorResolution)
	displayResolution = monitorResolution;

    dxres = displayResolution;
    dyres = displayResolution;

    mode = fbdevInfoRec.modes;
    if (!UseModeDB) {
	fbdev2xfree(var, mode);
	mode->name = "default";
    } else
	xfree2fbdev(mode, var);

    var->accel_flags &= ~FB_ACCELF_TEXT;
    if (ioctl(fb_fd, FBIOPUT_VSCREENINFO, var))
	FatalError("fbdevScreenInit: unable to set screen params (%s)\n",
		   strerror(errno));
    if (ioctl(fb_fd, FBIOGET_FSCREENINFO, &fb_fix))
	FatalError("ioctl(fd, FBIOGET_FSCREENINFO, ...)");

    smem_start = (unsigned long)fb_fix.smem_start & PAGE_MASK;
    smem_offset = (unsigned long)fb_fix.smem_start & ~PAGE_MASK;
    smem_len = (smem_offset+fb_fix.smem_len+~PAGE_MASK) & PAGE_MASK;

    mmio_start = (unsigned long)fb_fix.mmio_start & PAGE_MASK;
    mmio_offset = (unsigned long)fb_fix.mmio_start & ~PAGE_MASK;
    mmio_len = (mmio_offset+fb_fix.mmio_len+~PAGE_MASK) & PAGE_MASK;

    fbdevInfoRec.chipset = fb_fix.id;
    fbdevInfoRec.videoRam = fb_fix.smem_len>>10;

    if (xf86Verbose) {
	ErrorF("%s %s: Frame buffer device: %s\n", XCONFIG_PROBED,
	       fbdevInfoRec.name, fbdevInfoRec.chipset);
	ErrorF("%s %s: Video memory: %dK @ %p\n", XCONFIG_PROBED,
	       fbdevInfoRec.name, fbdevInfoRec.videoRam, fb_fix.smem_start);
	ErrorF("%s %s: MMIO regs: %dK @ %p\n", XCONFIG_PROBED,
	       fbdevInfoRec.name, fb_fix.mmio_len>>10, fb_fix.mmio_start);
	ErrorF("%s %s: Type %d type_aux %d bits_per_pixel %d\n", XCONFIG_PROBED,
	       fbdevInfoRec.name, fb_fix.type, fb_fix.type_aux,
	       var->bits_per_pixel);
    }
    for (i = 0; i < sizeof(acceltab)/sizeof(*acceltab); i++)
	if (fb_fix.accel == acceltab[i].id) {
	    accelentry = &acceltab[i];
	    break;
	}
    if (accelentry) {
	if (xf86Verbose)
	    ErrorF("%s %s: Hardware accelerator: %s\n", XCONFIG_PROBED,
		   fbdevInfoRec.name, accelentry->name);
	if (!accelentry->init)
	    accelentry = NULL;
    } else {
	if (xf86Verbose)
	    ErrorF("%s %s: Unknown hardware accelerator type %d\n",
		   XCONFIG_PROBED, fbdevInfoRec.name, fb_fix.accel);
    }
    if (!accelentry) {
	mmio_len = 0;
	if (xf86Verbose)
	    ErrorF("%s %s: No driver support for hardware acceleration\n",
		   XCONFIG_PROBED, fbdevInfoRec.name);
    } else if (OFLG_ISSET(OPTION_NOACCEL, &fbdevInfoRec.options)) {
	accelentry = NULL;
	mmio_len = 0;
	if (xf86Verbose)
	    ErrorF("%s %s: hardware acceleration disabled\n", XCONFIG_GIVEN,
		   fbdevInfoRec.name);
    }

    if (serverGeneration == 1) {
	fbdevVirtBase = xf86MapVidMem(scr_index, 0, 0, 0);
	fbdevRegBase = mmio_base;
    }

    fbdevInfoRec.AdjustFrame = fbdevAdjustFrame;
    fbdevAdjustFrame(fbdevInfoRec.frameX0, fbdevInfoRec.frameY0);

    if (var->nonstd)
	FatalError("Can't handle non standard pixel format %d\n", var->nonstd);

    bpp = var->bits_per_pixel;
    xsize = var->xres_virtual;
    ysize = var->yres_virtual;
    width = fb_fix.line_length ? fb_fix.line_length<<3 : xsize;

    fbdevInfoRec.bitsPerPixel = bpp;
    Visuals = 0;
    switch (fb_fix.visual) {
	case FB_VISUAL_MONO01:
	case FB_VISUAL_MONO10:
	    Visuals = StaticGrayMask;
	    fbdevInfoRec.depth = 1;
	    BitsPerRGB = 1;
	    ColorMapSize = 2;
	    RedMask = 0;
	    GreenMask = 0;
	    BlueMask = 0;
	    TranspMask = 0;
	    break;

	case FB_VISUAL_DIRECTCOLOR:
#ifdef DIRECTCOLORHACK
	    /* hack for directcolor */
	    /* we fake to be truecolor and set a linear colormap later */
#else
	    if (!var->grayscale)
		Visuals = PseudoColorMask | DirectColorMask | StaticColorMask;
#endif
	case FB_VISUAL_TRUECOLOR:
	    Visuals |= var->grayscale ? StaticGrayMask | GrayScaleMask
				      : TrueColorMask;
	    fbdevInfoRec.depth = var->red.length+var->green.length+
				 var->blue.length+var->transp.length;
	    BitsPerRGB = var->red.length;
	    if (var->green.length > BitsPerRGB)
		BitsPerRGB = var->green.length;
	    if (var->blue.length > BitsPerRGB)
		BitsPerRGB = var->blue.length;
	    ColorMapSize = 1<<BitsPerRGB;
	    RedMask = ((1<<var->red.length)-1)<<var->red.offset;
	    GreenMask = ((1<<var->green.length)-1)<<var->green.offset;
	    BlueMask = ((1<<var->blue.length)-1)<<var->blue.offset;
	    TranspMask = ((1<<var->transp.length)-1)<<var->transp.offset;
	    break;

	case FB_VISUAL_PSEUDOCOLOR:
	    Visuals = GrayScaleMask;
	    if (!var->grayscale)
		Visuals |= PseudoColorMask | DirectColorMask | TrueColorMask;
	case FB_VISUAL_STATIC_PSEUDOCOLOR:
	    Visuals |= StaticGrayMask;
	    if (!var->grayscale)
		Visuals |= StaticColorMask;
	    fbdevInfoRec.depth = bpp;
	    BitsPerRGB = var->red.length;
	    if (var->green.length > BitsPerRGB)
		BitsPerRGB = var->green.length;
	    if (var->blue.length > BitsPerRGB)
		BitsPerRGB = var->blue.length;
	    ColorMapSize = 1<<bpp;
	    RedMask = 0;
	    GreenMask = 0;
	    BlueMask = 0;
	    TranspMask = 0;
	    break;

	default:
	    FatalError("Unknown visual type 0x%08x\n", fb_fix.visual);
    }

    if (bpp == 1)
	switch (fb_fix.type) {
	    case FB_TYPE_INTERLEAVED_PLANES:
	    case FB_TYPE_PACKED_PIXELS:
	    case FB_TYPE_PLANES:
		switch(fb_fix.visual) {
		    case FB_VISUAL_MONO01:
			pScreen->blackPixel = 1;
			pScreen->whitePixel = 0;
			break;
		    case FB_VISUAL_MONO10:
			pScreen->blackPixel = 0;
			pScreen->whitePixel = 1;
			break;
		    case FB_VISUAL_PSEUDOCOLOR:
			pScreen->blackPixel = 1;
			pScreen->whitePixel = 0;
			fbdevUpdateColormap(0, 2, bw, bw, bw);
			break;
		    default:
			FatalError("Can't handle visual %d\n", fb_fix.visual);
		}
		fbtype = "mfb";
		mfbScreenInit(pScreen, fbdevVirtBase, xsize, ysize, dxres,
			      dyres, width);
		fbdevPrivateIndexP = NULL;
		fbdevBitBlt = mfbDoBitblt;
		CreateDefColormap = mfbCreateDefColormap;
		break;

	    default:
		FatalError("Unknown format type %d\n", fb_fix.type);
	}
    else
	switch (fb_fix.type) {
	    case FB_TYPE_INTERLEAVED_PLANES:
		if (fb_fix.type_aux == 2) {
#if defined(CONFIG_IPLAN2p2) || defined(CONFIG_IPLAN2p4) || \
     defined(CONFIG_IPLAN2p8)
		    if (!iplSetVisualTypes(bpp, Visuals, BitsPerRGB))
			FatalError("iplSetVisualTypes: FALSE\n");
#endif /* CONFIG_IPLAN2p2 || CONFIG_IPLAN2p4 || CONFIG_IPLAN2p8 */
		    switch(bpp) {
#ifdef CONFIG_IPLAN2p2
			case 2:
			    fbtype = "iplan2p2";
			    ipl2p2ScreenInit(pScreen, fbdevVirtBase, xsize,
					     ysize, dxres, dyres, width);
			    fbdevPrivateIndexP = &ipl2p2ScreenPrivateIndex;
			    fbdevBitBlt = ipl2p2DoBitblt;
			    CreateDefColormap = iplCreateDefColormap;
			    break;
#endif

#ifdef CONFIG_IPLAN2p4
			case 4:
			    fbtype = "iplan2p4";
			    ipl2p4ScreenInit(pScreen, fbdevVirtBase, xsize,
					     ysize, dxres, dyres, width);
			    fbdevPrivateIndexP = &ipl2p4ScreenPrivateIndex;
			    fbdevBitBlt = ipl2p4DoBitblt;
			    CreateDefColormap = iplCreateDefColormap;
			    break;
#endif

#ifdef CONFIG_IPLAN2p8
			case 8:
			    fbtype = "iplan2p8";
			    ipl2p8ScreenInit(pScreen, fbdevVirtBase, xsize,
			    		     ysize, dxres, dyres, width);
			    fbdevPrivateIndexP = &ipl2p8ScreenPrivateIndex;
			    fbdevBitBlt = ipl2p8DoBitblt;
			    CreateDefColormap = iplCreateDefColormap;
			    break;
#endif
			default:
			    FatalError("Can't handle interleaved planes with "
			    	       "%d planes\n", bpp);
		    }
		} else
#ifdef CONFIG_ILBM
		if (bpp <= 8) {
		    if (!ilbmSetVisualTypes(bpp, Visuals, BitsPerRGB))
			FatalError("ilbmSetVisualTypes: FALSE\n");
		    width = fb_fix.line_length ? fb_fix.line_length<<3 :
						 (fb_fix.type_aux<<3)/bpp;
		    fbtype = "ilbm";
		    ilbmScreenInit(pScreen, fbdevVirtBase, xsize, ysize, dxres,
				   dyres, width);
		    fbdevPrivateIndexP = &ilbmScreenPrivateIndex;
		    fbdevBitBlt = ilbmDoBitblt;
		    CreateDefColormap = ilbmCreateDefColormap;
		} else
#endif /* CONFIG_ILBM */
		    FatalError("Can't handle interleaved planes with %d "
			       "planes\n", bpp);
		break;

	    case FB_TYPE_PLANES:
#ifdef CONFIG_AFB
		if (bpp <= 8) {
		    if (!afbSetVisualTypes(bpp, Visuals, BitsPerRGB))
			FatalError("afbSetVisualTypes: FALSE\n");
		    fbtype = "afb";
		    afbScreenInit(pScreen, fbdevVirtBase, xsize, ysize, dxres,
				  dyres, width);
		    fbdevPrivateIndexP = &afbScreenPrivateIndex;
		    fbdevBitBlt = afbDoBitblt;
		    CreateDefColormap = afbCreateDefColormap;
		} else
#endif /* CONFIG_AFB */
		    FatalError("Can't handle planes format with %d bits per "
		    	       "pixel\n", bpp);
		break;

	    case FB_TYPE_PACKED_PIXELS:
		width = fb_fix.line_length ? 8*fb_fix.line_length/bpp : xsize;
#if defined(CONFIG_CFB16) || defined(CONFIG_CFB24) || defined(CONFIG_CFB32)
		if (!cfbSetVisualTypes(bpp, Visuals, BitsPerRGB))
		    FatalError("cfbSetVisualTypes: FALSE\n");
#endif /* CONFIG_CFB16 || CONFIG_CFB24 || CONFIG_CFB32 */
		switch (bpp) {
#ifdef CONFIG_CFB8
		    case 8:
			fbtype = "cfb8";
			cfbScreenInit(pScreen, fbdevVirtBase, xsize, ysize,
				      dxres, dyres, width);
			fbdevPrivateIndexP = NULL;
			fbdevBitBlt = cfbDoBitblt;
			CreateDefColormap = cfbCreateDefColormap;
			break;
#endif

#ifdef CONFIG_CFB16
		    case 16:
			fbtype = "cfb16";
			if (!cfb16ScreenInit(pScreen, fbdevVirtBase, xsize, 
					ysize, dxres, dyres, width))
			    FatalError("cfb16ScreenInit: FALSE\n");
			/* Fixup RGB ordering */
			visual = pScreen->visuals + pScreen->numVisuals;
			while (--visual >= pScreen->visuals) {
				visual->offsetRed = fb_var.red.offset;
				visual->offsetGreen = fb_var.green.offset;
				visual->offsetBlue = fb_var.blue.offset;
				visual->redMask = RedMask;
				visual->greenMask = GreenMask;
				visual->blueMask = BlueMask;
			}
			fbdevPrivateIndexP = &cfb16ScreenPrivateIndex;
			fbdevBitBlt = cfb16DoBitblt;
			CreateDefColormap = cfbCreateDefColormap;
#ifdef DIRECTCOLORHACK
			if (fb_fix.visual == FB_VISUAL_DIRECTCOLOR)
			    fbdevSetLinearColormap(var);
#endif
			break;
#endif

#ifdef CONFIG_CFB24
		    case 24:
			fbtype = "cfb24";
			cfb24ScreenInit(pScreen, fbdevVirtBase, xsize, ysize,
					dxres, dyres, width);
			/* Fixup RGB ordering */
			visual = pScreen->visuals + pScreen->numVisuals;
			while (--visual >= pScreen->visuals) {
				visual->offsetRed = fb_var.red.offset;
				visual->offsetGreen = fb_var.green.offset;
				visual->offsetBlue = fb_var.blue.offset;
				visual->redMask = RedMask;
				visual->greenMask = GreenMask;
				visual->blueMask = BlueMask;
			}
			fbdevPrivateIndexP = &cfb24ScreenPrivateIndex;
			fbdevBitBlt = cfb24DoBitblt;
			CreateDefColormap = cfbCreateDefColormap;
#ifdef DIRECTCOLORHACK
			if (fb_fix.visual == FB_VISUAL_DIRECTCOLOR)
			    fbdevSetLinearColormap(var);
#endif
			break;
#endif

#ifdef CONFIG_CFB32
		    case 32:
			fbtype = "cfb32";
			cfb32ScreenInit(pScreen, fbdevVirtBase, xsize, ysize,
					dxres, dyres, width);
			/* Fixup RGB ordering */
			visual = pScreen->visuals + pScreen->numVisuals;
			while (--visual >= pScreen->visuals) {
				visual->offsetRed = fb_var.red.offset;
				visual->offsetGreen = fb_var.green.offset;
				visual->offsetBlue = fb_var.blue.offset;
				visual->redMask = RedMask;
				visual->greenMask = GreenMask;
				visual->blueMask = BlueMask;
			}
			fbdevPrivateIndexP = &cfb32ScreenPrivateIndex;
			fbdevBitBlt = cfb32DoBitblt;
			CreateDefColormap = cfbCreateDefColormap;
#ifdef DIRECTCOLORHACK
			if (fb_fix.visual == FB_VISUAL_DIRECTCOLOR)
			    fbdevSetLinearColormap(var);
#endif
			break;
#endif

		    default:
			FatalError("Can't handle packed pixels with %d bits "
				   "per pixel\n", bpp);
		}
		break;

	    default:
		FatalError("Unknown format type %d\n", fb_fix.type);
	}

    if (accelentry) {
	ErrorF("%s %s: Initializing accel code\n", XCONFIG_PROBED,
	       fbdevInfoRec.name);
	if (!accelentry->init(pScreen))
	    FatalError("Initialization of %s failed\n", accelentry->name);
    }

    ErrorF("%s %s: Using %s driver\n", XCONFIG_PROBED, fbdevInfoRec.name,
	   fbtype);
    pScreen->CloseScreen = fbdevCloseScreen;
    pScreen->SaveScreen = fbdevSaveScreen;

    switch (fb_fix.visual) {
	case FB_VISUAL_MONO01:
	case FB_VISUAL_MONO10:
	    break;

	case FB_VISUAL_PSEUDOCOLOR:
	    pScreen->InstallColormap = fbdevInstallColormap;
	    pScreen->UninstallColormap = fbdevUninstallColormap;
	    pScreen->ListInstalledColormaps = fbdevListInstalledColormaps;
	    pScreen->StoreColors = fbdevStoreColors;
	    break;

#if defined(CONFIG_CFB16) || defined(CONFIG_CFB24) || defined(CONFIG_CFB32)
	case FB_VISUAL_TRUECOLOR:
	    pScreen->InstallColormap = cfbInstallColormap;
	    pScreen->UninstallColormap = cfbUninstallColormap;
	    pScreen->ListInstalledColormaps = cfbListInstalledColormaps;
	    pScreen->StoreColors = (void (*)())NoopDDA;
	    break;

	case FB_VISUAL_DIRECTCOLOR:
	    pScreen->InstallColormap = cfbInstallColormap;
	    pScreen->UninstallColormap = cfbUninstallColormap;
	    pScreen->ListInstalledColormaps = cfbListInstalledColormaps;
	    pScreen->StoreColors = (void (*)())NoopDDA;
	    break;
#endif /* CONFIG_CFB16 || CONFIG_CFB24 || CONFIG_CFB32 */
    }
    miDCInitialize(pScreen, &xf86PointerScreenFuncs);
    savepScreen = pScreen;

    return (*CreateDefColormap)(pScreen);
}

/*
 *  fbdevEnterLeaveVT -- Grab/ungrab the current VT completely.
 */

static void fbdevEnterLeaveVT(Bool enter, int screen_idx)
{
    BoxRec pixBox;
    RegionRec pixReg;
    DDXPointRec pixPt;
    PixmapPtr pspix;
    ScreenPtr pScreen = savepScreen;

    if (!xf86Resetting && !xf86Exiting) {
	pixBox.x1 = 0; pixBox.x2 = pScreen->width;
	pixBox.y1 = 0; pixBox.y2 = pScreen->height;
	pixPt.x = 0; pixPt.y = 0;
	(pScreen->RegionInit)(&pixReg, &pixBox, 1);
	if (fbdevPrivateIndexP)
	    pspix = (PixmapPtr)pScreen->devPrivates[*fbdevPrivateIndexP].ptr;
	else
	    pspix = (PixmapPtr)pScreen->devPrivate;
    }

    if (enter) {
	fbdevVirtBase = xf86MapVidMem(screen_idx, 0, 0, 0);
	fbdevRegBase = mmio_base;

	/*
	 *  point pspix back to fbdevVirtBase, and copy the dummy buffer to the
	 *  real screen.
	 */
	if (!xf86Resetting)
	    if (pspix->devPrivate.ptr != fbdevVirtBase && ppix) {
		pspix->devPrivate.ptr = fbdevVirtBase;
		(*fbdevBitBlt)(&ppix->drawable, &pspix->drawable, GXcopy,
			       &pixReg, &pixPt, ~0);
	    }
	if (ppix) {
	    (pScreen->DestroyPixmap)(ppix);
	    ppix = NULL;
	}

	if (!xf86Resetting) {
	    /* Update the colormap */
	    fbdevRestoreColors(pScreen);
	}
    } else {
	/*
	 *  Create a dummy pixmap to write to while VT is switched out.
	 *  Copy the screen to that pixmap
	 */
	if (!xf86Exiting) {
	    ppix = (pScreen->CreatePixmap)(pScreen, pScreen->width,
	    				   pScreen->height, pScreen->rootDepth);
	    if (ppix) {
		(*fbdevBitBlt)(&pspix->drawable, &ppix->drawable, GXcopy,
			       &pixReg, &pixPt, ~0);
		pspix->devPrivate.ptr = ppix->devPrivate.ptr;
	    }
	}
	xf86UnMapVidMem(screen_idx, 0, 0, 0);
    }
}

/*
 *  fbdevAdjustFrame -- Pan the display
 */

    /*
     *  Some range checking first
     *
     *  It seems that XFree sometimes passes values that are too large.
     *  Maybe this is an indication that we're moving to a second display? :-)
     */

static int checkframe_x(int x, const struct fb_fix_screeninfo *fix,
			const struct fb_var_screeninfo *var)
{
    if (!fix->xpanstep)
	x = 0;
    else {
	if (x < 0)
	    x = 0;
	else if (x > var->xres_virtual-var->xres)
	    x = var->xres_virtual-var->xres;
	x -= x % fix->xpanstep;
    }
    return x;
}

static int checkframe_y(int y, const struct fb_fix_screeninfo *fix,
			const struct fb_var_screeninfo *var)
{
    if (!fix->ypanstep)
	y = 0;
    else {
	if (y < 0)
	    y = 0;
	else if (y > var->yres_virtual-var->yres)
	    y = var->yres_virtual-var->yres;
	y -= y % fix->ypanstep;
    }
    return y;
}

static void fbdevAdjustFrame(int x, int y)
{
    x = checkframe_x(x, &fb_fix, &fb_var);
    y = checkframe_y(y, &fb_fix, &fb_var);

    fb_var.xoffset = x;
    fb_var.yoffset = y;
    ioctl(fb_fd, FBIOPAN_DISPLAY, &fb_var);
}

/*
 *  fbdevCloseScreen -- Called to ensure video is enabled when server exits.
 */

static Bool fbdevCloseScreen(int screen_idx, ScreenPtr screen)
{
    /*
     *  Hmm... The server may shut down even if it is not running on the
     *  current vt. Let's catch this case here.
     */
    xf86Exiting = TRUE;
    if (xf86VTSema)
	fbdevEnterLeaveVT(LEAVE, screen_idx);
    else if (ppix) {
	/*
	 *  7-Jan-94 CEG: The server is not running on the current vt.
	 *  Free the screen snapshot taken when the server vt was left.
	 */
	(savepScreen->DestroyPixmap)(ppix);
	ppix = NULL;
    }
    return TRUE;
}

static Bool fbdevSetMode(DisplayModePtr mode, Bool doit)
{
    struct fb_var_screeninfo var1, var2;
    ScreenPtr pScreen = savepScreen;

    var1 = fb_var;
    xfree2fbdev(mode, &var1);
    var1.xoffset = checkframe_x(fbdevInfoRec.frameX0, &fb_fix, &var1);
    var1.yoffset = checkframe_y(fbdevInfoRec.frameY0, &fb_fix, &var1);
    var2 = var1;
    var1.activate = FB_ACTIVATE_TEST;
    if (ioctl(fb_fd, FBIOPUT_VSCREENINFO, &var1))
	return FALSE;
    if (var1.xres != var2.xres || var1.yres != var2.yres ||
	var1.xres_virtual != var2.xres_virtual ||
	var1.yres_virtual != var2.yres_virtual ||
	var1.bits_per_pixel != var2.bits_per_pixel)
	return FALSE;
    fbdev2xfree(&var1, mode);

    if (doit) {
	var1.activate = FB_ACTIVATE_NOW;
	if (ioctl(fb_fd, FBIOPUT_VSCREENINFO, &var1))
	    return FALSE;
	/* Restore the colormap */
	fbdevRestoreColors(pScreen);
	fb_var = var1;
    }
    return TRUE;
}

/*
 *  fbdevValidMode -- Check whether a mode is valid. If necessary, values will
 *		      be rounded up by the Frame Buffer Device
 */

static Bool fbdevValidMode(DisplayModePtr mode, Bool verbose, int flag)
{
    return fbdevSetMode(mode, FALSE);
}

/*
 *  fbdevSwitchMode -- Change the video mode `on the fly'
 */

static Bool fbdevSwitchMode(DisplayModePtr mode)
{
    return fbdevSetMode(mode, TRUE);
}


/*
 *  Convert timings between the XFree style and the Frame Buffer Device style
 */

static void xfree2fbdev(DisplayModePtr mode, struct fb_var_screeninfo *var)
{
    var->xres = mode->HDisplay;
    var->yres = mode->VDisplay;
    var->xres_virtual = fbdevInfoRec.virtualX;
    var->yres_virtual = fbdevInfoRec.virtualY;
    var->xoffset = var->yoffset = 0;
    var->bits_per_pixel = fbdevInfoRec.bitsPerPixel;
    var->pixclock = mode->Clock ? 1000000000/mode->Clock : 0;
    var->right_margin = mode->HSyncStart-mode->HDisplay;
    var->hsync_len = mode->HSyncEnd-mode->HSyncStart;
    var->left_margin = mode->HTotal-mode->HSyncEnd;
    var->lower_margin = mode->VSyncStart-mode->VDisplay;
    var->vsync_len = mode->VSyncEnd-mode->VSyncStart;
    var->upper_margin = mode->VTotal-mode->VSyncEnd;
    var->sync = 0;
    if (mode->Flags & V_PHSYNC)
	var->sync |= FB_SYNC_HOR_HIGH_ACT;
    if (mode->Flags & V_PVSYNC)
	var->sync |= FB_SYNC_VERT_HIGH_ACT;
    if (mode->Flags & V_PCSYNC)
	var->sync |= FB_SYNC_COMP_HIGH_ACT;
    if (mode->Flags & V_BCAST)
	var->sync |= FB_SYNC_BROADCAST;
    if (mode->Flags & V_INTERLACE)
	var->vmode = FB_VMODE_INTERLACED;
    else if (mode->Flags & V_DBLSCAN)
	var->vmode = FB_VMODE_DOUBLE;
    else
    	var->vmode = FB_VMODE_NONINTERLACED;
}

static void fbdev2xfree(struct fb_var_screeninfo *var, DisplayModePtr mode)
{
    mode->Clock = var->pixclock ? 1000000000/var->pixclock : 28000000;
    mode->HDisplay = var->xres;
    mode->HSyncStart = mode->HDisplay+var->right_margin;
    mode->HSyncEnd = mode->HSyncStart+var->hsync_len;
    mode->HTotal = mode->HSyncEnd+var->left_margin;
    mode->VDisplay = var->yres;
    mode->VSyncStart = mode->VDisplay+var->lower_margin;
    mode->VSyncEnd = mode->VSyncStart+var->vsync_len;
    mode->VTotal = mode->VSyncEnd+var->upper_margin;
    mode->Flags = 0;
    mode->Flags |= var->sync & FB_SYNC_HOR_HIGH_ACT ? V_PHSYNC : V_NHSYNC;
    mode->Flags |= var->sync & FB_SYNC_VERT_HIGH_ACT ? V_PVSYNC : V_NVSYNC;
    mode->Flags |= var->sync & FB_SYNC_COMP_HIGH_ACT ? V_PCSYNC : V_NCSYNC;
    if (var->sync & FB_SYNC_BROADCAST)
	mode->Flags |= V_BCAST;
    if ((var->vmode & FB_VMODE_MASK) == FB_VMODE_INTERLACED)
	mode->Flags |= V_INTERLACE;
    else if ((var->vmode & FB_VMODE_MASK) == FB_VMODE_DOUBLE)
	mode->Flags |= V_DBLSCAN;
    mode->SynthClock = mode->Clock;
    mode->CrtcHDisplay = mode->HDisplay;
    mode->CrtcHSyncStart = mode->HSyncStart;
    mode->CrtcHSyncEnd = mode->HSyncEnd;
    mode->CrtcHTotal = mode->HTotal;
    mode->CrtcVDisplay = mode->VDisplay;
    mode->CrtcVSyncStart = mode->VSyncStart;
    mode->CrtcVSyncEnd = mode->VSyncEnd;
    mode->CrtcVTotal = mode->VTotal;
    mode->CrtcHAdjusted = FALSE;
    mode->CrtcVAdjusted = FALSE;
}

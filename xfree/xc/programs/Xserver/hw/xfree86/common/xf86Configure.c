/* $XFree86: xc/programs/Xserver/hw/xfree86/common/xf86Configure.c,v 3.51 2001/01/16 23:46:29 herrb Exp $ */
/*
 * Copyright 2000 by Alan Hourihane, Sychdyn, North Wales.
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
 * Author:  Alan Hourihane, alanh@fairlite.demon.co.uk
 *
 */

#include <ctype.h>
#include <stdlib.h>
#include "X.h"
#include "Xmd.h"
#include "os.h"
#ifdef XFree86LOADER
#include "loaderProcs.h"
#endif
#include "xf86.h"
#include "xf86Config.h"
#include "xf86Priv.h"
#include "xf86PciInfo.h"
#include "xf86Parser.h"
#include "xf86tokens.h"
#include "Configint.h"
#include "vbe.h"
#include "xf86DDC.h"
#ifdef __sparc__
#include "xf86Bus.h"
#include "xf86Sbus.h"
#endif
#include "globals.h"

typedef struct _DevToConfig {
    GDevRec GDev;
    pciVideoPtr pVideo;
#ifdef __sparc__
    sbusDevicePtr sVideo;
#endif
    int iDriver;
} DevToConfigRec, *DevToConfigPtr;

static DevToConfigPtr DevToConfig = NULL;
static int nDevToConfig = 0, CurrentDriver;

xf86MonPtr ConfiguredMonitor;
Bool xf86DoConfigurePass1 = TRUE;
Bool foundMouse = FALSE;

#ifndef __EMX__
static char *DFLT_MOUSE_DEV = "/dev/mouse";
static char *DFLT_MOUSE_PROTO = "auto";
#else
#define DFLT_MOUSE_DEV "mouse$"
#define DFLT_MOUSE_PROTO "OS2Mouse"
#endif

static void
GetPciCard(int vendor, int chipType, int *vendor1, int *vendor2, int *card)
{
    int k, j;
   
    *vendor1 = 0;
    *vendor2 = 0;
    *card = 0;

    k = 0;
    while (xf86PCIVendorNameInfo[k].token) {
	if (xf86PCIVendorNameInfo[k].token == vendor) {
	    *vendor1 = k;
	    break;
	}
	k++;
    }
    k = 0;
    while(xf86PCIVendorInfo[k].VendorID) {
    	if (xf86PCIVendorInfo[k].VendorID == vendor) {
	    j = 0;
	    while (xf86PCIVendorInfo[k].Device[j].DeviceName) {
	        if (xf86PCIVendorInfo[k].Device[j].DeviceID == chipType) {
		    *vendor2 = k;
		    *card = j;
		    break;
	    	}
	    	j++;
	    }
	    break;
    	}
	k++;
    }
}

/*
 * This is called by the driver, either through xf86Match???Instances() or
 * directly.  We allocate a GDevRec and fill it in as much as we can, letting
 * the caller fill in the rest and/or change it as it sees fit.
 */
GDevPtr
xf86AddBusDeviceToConfigure(const char *driver, BusType bus, void *busData, int chipset)
{
    int i, j;
    pciVideoPtr pVideo = NULL;
    Bool isPrimary = FALSE;

    if (xf86DoProbe || !xf86DoConfigure || !xf86DoConfigurePass1)
	return NULL;

    /* Check for duplicates */
    switch (bus) {
    case BUS_PCI:
	pVideo = (pciVideoPtr) busData;
	for (i = 0;  i < nDevToConfig;  i++)
	    if (DevToConfig[i].pVideo &&
		(DevToConfig[i].pVideo->bus == pVideo->bus) &&
		(DevToConfig[i].pVideo->device == pVideo->device) &&
		(DevToConfig[i].pVideo->func == pVideo->func))
		return NULL;
	isPrimary = xf86IsPrimaryPci(pVideo);
	break;
    case BUS_ISA:
	/*
	 * This needs to be revisited as it doesn't allow for non-PCI
	 * multihead.
	 */
	if (!xf86IsPrimaryIsa())
	    return NULL;
	isPrimary = TRUE;
	for (i = 0;  i < nDevToConfig;  i++)
	    if (!DevToConfig[i].pVideo)
		return NULL;
	break;
#ifdef __sparc__
    case BUS_SBUS:
	for (i = 0;  i < nDevToConfig;  i++)
	    if (DevToConfig[i].sVideo &&
		DevToConfig[i].sVideo->fbNum == ((sbusDevicePtr) busData)->fbNum)
		return NULL;
	break;
#endif
    default:
	return NULL;
    }

    /* Allocate new structure occurrence */
    i = nDevToConfig++;
    DevToConfig =
	xnfrealloc(DevToConfig, nDevToConfig * sizeof(DevToConfigRec));
#if 0   /* Doesn't work when a driver detects more than one adapter */
    if (i > 0 && isPrimary) {
        memmove(DevToConfig + 1,DevToConfig,
	       (nDevToConfig - 1) * sizeof(DevToConfigRec));
	i = 0;
    } 
#endif
    memset(DevToConfig + i, 0, sizeof(DevToConfigRec));

#   define NewDevice DevToConfig[i]

    NewDevice.GDev.chipID = NewDevice.GDev.chipRev = NewDevice.GDev.irq = -1;

    NewDevice.iDriver = CurrentDriver;

    /* Fill in what we know, converting the driver name to lower case */
    NewDevice.GDev.driver = xnfalloc(strlen(driver) + 1);
    for (j = 0;  (NewDevice.GDev.driver[j] = tolower(driver[j]));  j++);

    switch (bus) {
    case BUS_PCI: {
	int vendor1, vendor2, card;

	NewDevice.pVideo = pVideo;
	GetPciCard(pVideo->vendor, pVideo->chipType,
	    &vendor1, &vendor2, &card);

	if (vendor1 == 0 || (vendor2 == 0 && card == 0)) {
   	    FatalError("\nXFree86 has found a valid card configuration.\nUnfortunately the appropriate data has not been added to xf86PciInfo.h.\nPlease forward 'scanpci -v' output to XFree86 support team.");
	}

#	define VendorName xf86PCIVendorNameInfo[vendor1].name
#	define CardName   xf86PCIVendorInfo[vendor2].Device[card].DeviceName

	NewDevice.GDev.identifier =
	    xnfalloc(strlen(VendorName) + strlen(CardName) + 2);
	sprintf(NewDevice.GDev.identifier, "%s %s", VendorName, CardName);

	NewDevice.GDev.vendor = (char *)VendorName;
	NewDevice.GDev.board = CardName;

	NewDevice.GDev.busID = xnfalloc(16);
	sprintf(NewDevice.GDev.busID, "PCI:%d:%d:%d",
	    pVideo->bus, pVideo->device, pVideo->func);

	NewDevice.GDev.chipID = pVideo->chipType;
	NewDevice.GDev.chipRev = pVideo->chipRev;

#	undef VendorName
#	undef CardName

	if (chipset < 0)
	    chipset = (pVideo->vendor << 16) || pVideo->chipType;
	}
	break;
    case BUS_ISA:
	NewDevice.GDev.identifier = "ISA Adapter";
	NewDevice.GDev.busID = "ISA";
	break;
#ifdef __sparc__
    case BUS_SBUS: {
	char *promPath = NULL;
	NewDevice.sVideo = (sbusDevicePtr) busData;
	NewDevice.GDev.identifier = NewDevice.sVideo->descr;
	if (sparcPromInit() >= 0) {
	    promPath = sparcPromNode2Pathname(&NewDevice.sVideo->node);
	    sparcPromClose();
	}
	if (promPath) {
	    NewDevice.GDev.busID = xnfalloc(strlen(promPath) + 6);
	    sprintf(NewDevice.GDev.busID, "SBUS:%s", promPath);
	    xfree(promPath);
	} else {
	    NewDevice.GDev.busID = xnfalloc(12);
	    sprintf(NewDevice.GDev.busID, "SBUS:fb%d", NewDevice.sVideo->fbNum);
	}
	}
	break;
#endif
    default:
	break;
    }

    /* Get driver's available options */
    if (xf86DriverList[CurrentDriver]->AvailableOptions)
	NewDevice.GDev.options =
	    (*xf86DriverList[CurrentDriver]->AvailableOptions)(chipset,
							       bus);

    return &NewDevice.GDev;

#   undef NewDevice
}

/*
 * Backwards compatibility
 */
GDevPtr
xf86AddDeviceToConfigure(const char *driver, pciVideoPtr pVideo, int chipset)
{
    return xf86AddBusDeviceToConfigure(driver, pVideo ? BUS_PCI : BUS_ISA,
				       pVideo, chipset);
}

static XF86ConfInputPtr
configureInputSection (void)
{
    XF86ConfInputPtr mouse = NULL;
    parsePrologue (XF86ConfInputPtr, XF86ConfInputRec)

    ptr->inp_identifier = "Keyboard0";
    ptr->inp_driver = "keyboard";
    ptr->list.next = NULL;

    /* Crude mechanism to auto-detect mouse (os dependent) */
    { 
	int fd;
#ifdef linux
	int len;
	char path[32];

	if ((len = readlink(DFLT_MOUSE_DEV, path, sizeof(path) - 1)) > 0) {
	    path[len] = '\0';
	    if (strstr(path, "psaux") != NULL)
		DFLT_MOUSE_PROTO = "PS/2";
	}
#endif
#ifdef WSCONS_SUPPORT
	fd = open("/dev/wsmouse0", 0);
	if (fd > 0) {
	    DFLT_MOUSE_DEV = "/dev/wsmouse0";
	    DFLT_MOUSE_PROTO = "wsmouse";
	    close(fd);
	}
#endif

	fd = open(DFLT_MOUSE_DEV, 0);
	if (fd != -1) {
	    foundMouse = TRUE;
	    close(fd);
	}
    }

    mouse = xf86confmalloc(sizeof(XF86ConfInputRec));
    memset((XF86ConfInputPtr)mouse,0,sizeof(XF86ConfInputRec));
    mouse->inp_identifier = "Mouse0";
    mouse->inp_driver = "mouse";
    mouse->inp_option_lst = 
		xf86addNewOption(mouse->inp_option_lst, "Protocol", DFLT_MOUSE_PROTO);
    mouse->inp_option_lst = 
		xf86addNewOption(mouse->inp_option_lst, "Device", DFLT_MOUSE_DEV);
    ptr = (XF86ConfInputPtr)xf86addListItem((glp)ptr, (glp)mouse);
    return ptr;
}

static XF86ConfDRIPtr
configureDRISection (void)
{
    parsePrologue (XF86ConfDRIPtr, XF86ConfDRIRec)

    return ptr;
}

static XF86ConfVendorPtr
configureVendorSection (void)
{
    parsePrologue (XF86ConfVendorPtr, XF86ConfVendorRec)

    return NULL;
#if 0
    return ptr;
#endif
}

static XF86ConfScreenPtr
configureScreenSection (int screennum)
{
    int i;
    int depths[] = { 1, 4, 8, 15, 16, 24/*, 32*/ };
    parsePrologue (XF86ConfScreenPtr, XF86ConfScreenRec)

    ptr->scrn_identifier = xf86confmalloc(18);
    sprintf(ptr->scrn_identifier, "Screen%d", screennum);
    ptr->scrn_monitor_str = xf86confmalloc(19);
    sprintf(ptr->scrn_monitor_str, "Monitor%d", screennum);
    ptr->scrn_device_str = xf86confmalloc(16);
    sprintf(ptr->scrn_device_str, "Card%d", screennum);

    for (i=0; i<sizeof(depths)/sizeof(depths[0]); i++)
    {
	XF86ConfDisplayPtr display;

	display = xf86confmalloc(sizeof(XF86ConfDisplayRec));
    	memset((XF86ConfDisplayPtr)display,0,sizeof(XF86ConfDisplayRec));
	display->disp_depth = depths[i];
	display->disp_black.red = display->disp_white.red = -1;
	display->disp_black.green = display->disp_white.green = -1;
	display->disp_black.blue = display->disp_white.blue = -1;
	ptr->scrn_display_lst = (XF86ConfDisplayPtr)xf86addListItem(
				     (glp)ptr->scrn_display_lst, (glp)display);
    }

    return ptr;
}

static XF86ConfDevicePtr
configureDeviceSection (int screennum)
{
    char identifier[16];
    OptionInfoPtr p;
    int i = 0;
    Bool foundFBDEV = FALSE;
    parsePrologue (XF86ConfDevicePtr, XF86ConfDeviceRec)

    /* Move device info to parser structure */
    sprintf(identifier, "Card%d", screennum);
    ptr->dev_identifier = strdup(identifier);
/*    ptr->dev_identifier = DevToConfig[screennum].GDev.identifier;*/
    ptr->dev_vendor = DevToConfig[screennum].GDev.vendor;
    ptr->dev_board = DevToConfig[screennum].GDev.board;
    ptr->dev_chipset = DevToConfig[screennum].GDev.chipset;
    ptr->dev_busid = DevToConfig[screennum].GDev.busID;
    ptr->dev_driver = DevToConfig[screennum].GDev.driver;
    ptr->dev_ramdac = DevToConfig[screennum].GDev.ramdac;
    for (i = 0;  (i < MAXDACSPEEDS) && (i < CONF_MAXDACSPEEDS);  i++)
        ptr->dev_dacSpeeds[i] = DevToConfig[screennum].GDev.dacSpeeds[i];
    ptr->dev_videoram = DevToConfig[screennum].GDev.videoRam;
    ptr->dev_textclockfreq = DevToConfig[screennum].GDev.textClockFreq;
    ptr->dev_bios_base = DevToConfig[screennum].GDev.BiosBase;
    ptr->dev_mem_base = DevToConfig[screennum].GDev.MemBase;
    ptr->dev_io_base = DevToConfig[screennum].GDev.IOBase;
    ptr->dev_clockchip = DevToConfig[screennum].GDev.clockchip;
    for (i = 0;  (i < MAXCLOCKS) && (i < DevToConfig[screennum].GDev.numclocks);  i++)
        ptr->dev_clock[i] = DevToConfig[screennum].GDev.clock[i];
    ptr->dev_clocks = i;
    ptr->dev_chipid = DevToConfig[screennum].GDev.chipID;
    ptr->dev_chiprev = DevToConfig[screennum].GDev.chipRev;
    ptr->dev_irq = DevToConfig[screennum].GDev.irq;

    /* Make sure older drivers don't segv */
    if (DevToConfig[screennum].GDev.options) {
    	/* Fill in the available driver options for people to use */
    	ptr->dev_comment = xnfalloc(32 + 1);
    	strcpy(ptr->dev_comment, "Available Driver options are:-\n");
    	for (p = DevToConfig[screennum].GDev.options; p->name != NULL; p++) {
    	    ptr->dev_comment = xrealloc(ptr->dev_comment, 
			strlen(ptr->dev_comment) + strlen(p->name) + 24 + 1);
	    strcat(ptr->dev_comment, "        #Option     \"");
	    strcat(ptr->dev_comment, p->name);
	    strcat(ptr->dev_comment, "\"\n");
    	}
    }

    /* Crude mechanism to auto-detect fbdev (os dependent) */
    /* Skip it for now. Options list it anyway, and we can't
     * determine which screen/driver this belongs too anyway.
    {
	int fd;

	fd = open("/dev/fb0", 0);
	if (fd != -1) {
	    foundFBDEV = TRUE;
	    close(fd);
	}
    }

    if (foundFBDEV) {
	XF86OptionPtr fbdev;

    	fbdev = xf86confmalloc(sizeof(XF86OptionRec));
    	memset((XF86OptionPtr)fbdev,0,sizeof(XF86OptionRec));
    	fbdev->opt_name = "UseFBDev";
	fbdev->opt_val = "ON";
	ptr->dev_option_lst = (XF86OptionPtr)xf86addListItem(
					(glp)ptr->dev_option_lst, (glp)fbdev);
    }
    */

    return ptr;
}

static XF86ConfLayoutPtr
configureLayoutSection (void)
{
    pciVideoPtr xf86PciCard;
    int i = 0;
    int scrnum = 0;
    parsePrologue (XF86ConfLayoutPtr, XF86ConfLayoutRec)

    ptr->lay_identifier = "XFree86 Configured";

    {
	XF86ConfInputrefPtr iptr;

	iptr = xf86confmalloc (sizeof (XF86ConfInputrefRec));
	iptr->list.next = NULL;
	iptr->iref_option_lst = NULL;
	iptr->iref_inputdev_str = "Mouse0";
	iptr->iref_option_lst =
		xf86addNewOption (iptr->iref_option_lst, "CorePointer", NULL);
	ptr->lay_input_lst = (XF86ConfInputrefPtr)
		xf86addListItem ((glp) ptr->lay_input_lst, (glp) iptr);
    }

    {
	XF86ConfInputrefPtr iptr;

	iptr = xf86confmalloc (sizeof (XF86ConfInputrefRec));
	iptr->list.next = NULL;
	iptr->iref_option_lst = NULL;
	iptr->iref_inputdev_str = "Keyboard0";
	iptr->iref_option_lst =
		xf86addNewOption (iptr->iref_option_lst, "CoreKeyboard", NULL);
	ptr->lay_input_lst = (XF86ConfInputrefPtr)
		xf86addListItem ((glp) ptr->lay_input_lst, (glp) iptr);
    }

    for (scrnum = 0;  scrnum < nDevToConfig;  scrnum++) {
	XF86ConfAdjacencyPtr aptr;

	aptr = xf86confmalloc (sizeof (XF86ConfAdjacencyRec));
	aptr->list.next = NULL;
	aptr->adj_x = 0;
	aptr->adj_y = 0;
	aptr->adj_scrnum = scrnum;
	aptr->adj_screen_str = xnfalloc(18);
	sprintf(aptr->adj_screen_str, "Screen%d", scrnum);
	if (scrnum == 0) {
	    aptr->adj_where = CONF_ADJ_ABSOLUTE;
	    aptr->adj_refscreen = NULL;
	}
	else {
	    aptr->adj_where = CONF_ADJ_RIGHTOF;
	    aptr->adj_refscreen = xnfalloc(18);
	    sprintf(aptr->adj_refscreen, "Screen%d", scrnum - 1);
	}
    	ptr->lay_adjacency_lst =
	    (XF86ConfAdjacencyPtr)xf86addListItem((glp)ptr->lay_adjacency_lst,
					      (glp)aptr);
    }

    return ptr;
}

static XF86ConfModesPtr
configureModesSection (void)
{
    parsePrologue (XF86ConfModesPtr, XF86ConfModesRec)

    return ptr;
}

static XF86ConfVideoAdaptorPtr
configureVideoAdaptorSection (void)
{
    parsePrologue (XF86ConfVideoAdaptorPtr, XF86ConfVideoAdaptorRec)

    return NULL;
#if 0
    return ptr;
#endif
}

static XF86ConfFlagsPtr
configureFlagsSection (void)
{
    parsePrologue (XF86ConfFlagsPtr, XF86ConfFlagsRec)

    return ptr;
}

static XF86ConfModulePtr
configureModuleSection (void)
{
#ifdef XFree86LOADER
    char **elist, **el;
    /* Find the list of extension modules. */
    const char *esubdirs[] = {
	"extensions",
	NULL
    };
#endif
    parsePrologue (XF86ConfModulePtr, XF86ConfModuleRec)

#ifdef XFree86LOADER
    elist = LoaderListDirs(esubdirs, NULL);
    if (elist) {
	for (el = elist; *el; el++) {
	    XF86LoadPtr module;

    	    module = xf86confmalloc(sizeof(XF86LoadRec));
    	    memset((XF86LoadPtr)module,0,sizeof(XF86LoadRec));
    	    module->load_name = *el;
	    ptr->mod_load_lst = (XF86LoadPtr)xf86addListItem(
					(glp)ptr->mod_load_lst, (glp)module);
    	}
	xfree(elist);
    }
#endif

    return ptr;
}

static XF86ConfFilesPtr
configureFilesSection (void)
{
    parsePrologue (XF86ConfFilesPtr, XF86ConfFilesRec)

#ifdef XFree86LOADER
   if (xf86ModulePath)
       ptr->file_modulepath = strdup(xf86ModulePath);
#endif
   if (defaultFontPath)
       ptr->file_fontpath = strdup(defaultFontPath);
   if (rgbPath)
       ptr->file_rgbpath = strdup(rgbPath);
   
    return ptr;
}

static XF86ConfMonitorPtr
configureMonitorSection (int screennum)
{
    parsePrologue (XF86ConfMonitorPtr, XF86ConfMonitorRec)

    ptr->mon_identifier = xf86confmalloc(19);
    sprintf(ptr->mon_identifier, "Monitor%d", screennum);
    ptr->mon_vendor = strdup("Monitor Vendor");
    ptr->mon_modelname = strdup("Monitor Model");

    return ptr;
}

static XF86ConfMonitorPtr
configureDDCMonitorSection (int screennum)
{
    int i = 0;
    parsePrologue (XF86ConfMonitorPtr, XF86ConfMonitorRec)

    ptr->mon_identifier = xf86confmalloc(19);
    sprintf(ptr->mon_identifier, "Monitor%d", screennum);
    ptr->mon_vendor = strdup(ConfiguredMonitor->vendor.name);
    ptr->mon_modelname = xf86confmalloc(12);
    sprintf(ptr->mon_modelname, "%x", ConfiguredMonitor->vendor.prod_id);

    for (i=0;i<4;i++) {
	switch (ConfiguredMonitor->det_mon[i].type) {
	    case DT:
	    case DS_STD_TIMINGS:
	    case DS_WHITE_P:
	    case DS_NAME:
	    case DS_ASCII_STR:
	    case DS_SERIAL:
		break;
	    case DS_RANGES:
    		ptr->mon_n_hsync = 1;
    		ptr->mon_hsync[0].lo = 
			ConfiguredMonitor->det_mon[i].section.ranges.min_h;
    		ptr->mon_hsync[0].hi = 
			ConfiguredMonitor->det_mon[i].section.ranges.max_h;
    		ptr->mon_n_vrefresh = 1;
    		ptr->mon_vrefresh[0].lo = 
			ConfiguredMonitor->det_mon[i].section.ranges.min_v;
    		ptr->mon_vrefresh[0].hi = 
			ConfiguredMonitor->det_mon[i].section.ranges.max_v;
		break;
	}
    }

    return ptr;
}

void
DoConfigure()
{
    int i,j, screennum = -1;
    char *home = NULL;
    char *filename = NULL;
    XF86ConfigPtr xf86config = NULL;
    char **vlist, **vl;

    vlist = xf86DriverlistFromCompile();

    if (!vlist) {
	ErrorF("Missing output drivers.  Configuration failed.\n");
	goto bail;
    }

    ErrorF("List of video drivers:\n");
    for (vl = vlist; *vl; vl++)
	ErrorF("\t%s\n", *vl);

#ifdef XFree86LOADER
    /* Load all the drivers that were found. */
    xf86LoadModules(vlist, NULL);
#endif /* XFree86LOADER */

    xfree(vlist);

    /* Disable PCI devices */
    xf86ResourceBrokerInit();
    xf86AccessInit();
    xf86FindPrimaryDevice();
 
    /* Create XF86Config file structure */
    xf86config = malloc(sizeof(XF86ConfigRec));
    memset ((XF86ConfigPtr)xf86config, 0, sizeof(XF86ConfigRec));
    xf86config->conf_device_lst = NULL;
    xf86config->conf_screen_lst = NULL;
    xf86config->conf_monitor_lst = NULL;

    /* Call all of the probe functions, reporting the results. */
    for (CurrentDriver = 0;  CurrentDriver < xf86NumDrivers;  CurrentDriver++) {
	
	if (xf86DriverList[CurrentDriver]->Probe == NULL) continue;

	if ((*xf86DriverList[CurrentDriver]->Probe)(
	    xf86DriverList[CurrentDriver], PROBE_DETECT) &&
	    xf86DriverList[CurrentDriver]->Identify)
	    (*xf86DriverList[CurrentDriver]->Identify)(0);
    }

    if (nDevToConfig <= 0) {
	ErrorF("No devices to configure.  Configuration failed.\n");
	goto bail;
    }

    /* Add device, monitor and screen sections for detected devices */
    for (screennum = 0;  screennum < nDevToConfig;  screennum++) {
    	XF86ConfDevicePtr DevicePtr;
	XF86ConfMonitorPtr MonitorPtr;
	XF86ConfScreenPtr ScreenPtr;

	DevicePtr = configureDeviceSection(screennum);
    	xf86config->conf_device_lst = (XF86ConfDevicePtr)xf86addListItem(
			    (glp)xf86config->conf_device_lst, (glp)DevicePtr);
	MonitorPtr = configureMonitorSection(screennum);
    	xf86config->conf_monitor_lst = (XF86ConfMonitorPtr)xf86addListItem(
			    (glp)xf86config->conf_monitor_lst, (glp)MonitorPtr);
	ScreenPtr = configureScreenSection(screennum);
    	xf86config->conf_screen_lst = (XF86ConfScreenPtr)xf86addListItem(
			    (glp)xf86config->conf_screen_lst, (glp)ScreenPtr);
    }

    xf86config->conf_files = configureFilesSection();
    xf86config->conf_modules = configureModuleSection();
    xf86config->conf_flags = configureFlagsSection();
    xf86config->conf_videoadaptor_lst = configureVideoAdaptorSection();
/*    xf86config->conf_modes_lst = configureModesSection(); */
    xf86config->conf_vendor_lst = configureVendorSection();
/*    xf86config->conf_dri = configureDRISection(); */
    xf86config->conf_input_lst = configureInputSection();
    xf86config->conf_layout_lst = configureLayoutSection();

    if (!(home = getenv("HOME")))
    	home = "/";
    {
#ifdef __EMX__
#define PATH_MAX 2048
#endif
        const char* configfile = XF86CONFIGFILE".new";
    	char homebuf[PATH_MAX];
    	/* getenv might return R/O memory, as with OS/2 */
    	strncpy(homebuf,home,PATH_MAX-1);
    	homebuf[PATH_MAX-1] = '\0';
    	home = homebuf;
    	if (!(filename =
	     (char *)ALLOCATE_LOCAL(strlen(home) + 
	  			 strlen(configfile) + 3)))

      	if (home[0] == '/' && home[1] == '\0')
            home[0] = '\0';
      	sprintf(filename, "%s/%s", home,configfile);
    }

    xf86writeConfigFile(filename, xf86config);

    xf86DoConfigurePass1 = FALSE;
    /* Try to get DDC information filled in */
    xf86ConfigFile = filename;
    if (!xf86HandleConfigFile()) {
	goto bail;
    }

    xf86DoConfigurePass1 = FALSE;
    
    {
	Bool *driverProbed = xnfcalloc(1,xf86NumDrivers*sizeof(Bool));
	for (screennum = 0;  screennum < nDevToConfig;  screennum++) {
	    i = DevToConfig[screennum].iDriver;
	    
	    if (driverProbed[i]) continue;
	    driverProbed[i] = TRUE;

	    (*xf86DriverList[i]->Probe)(xf86DriverList[i], 0);

	    xf86SetPciVideo(NULL,NONE);
	}
	xfree(driverProbed);
    }
    

    if (nDevToConfig != xf86NumScreens) {
	ErrorF("Number of created screens does not match number of detected"
	       " devices.\n  Configuration failed.\n");
	goto bail;
    }

    xf86PostProbe();
    xf86EntityInit();

    for (j = 0; j < xf86NumScreens; j++) {
	xf86Screens[j]->scrnIndex = j;
    }

    xf86freeMonitorList(xf86config->conf_monitor_lst);
    xf86config->conf_monitor_lst = NULL;
    xf86freeScreenList(xf86config->conf_screen_lst);
    xf86config->conf_screen_lst = NULL;
    for (j = 0; j < xf86NumScreens; j++) {
	XF86ConfMonitorPtr MonitorPtr;
	XF86ConfScreenPtr ScreenPtr;

	ConfiguredMonitor = NULL;

	xf86EnableAccess(xf86Screens[j]);
	if ((*xf86Screens[j]->PreInit)(xf86Screens[j], PROBE_DETECT) &&
	    ConfiguredMonitor) {
	    MonitorPtr = configureDDCMonitorSection(j);
	} else {
	    MonitorPtr = configureMonitorSection(j);
	}
	ScreenPtr = configureScreenSection(j);
	xf86config->conf_monitor_lst = (XF86ConfMonitorPtr)xf86addListItem(
		(glp)xf86config->conf_monitor_lst, (glp)MonitorPtr);
	xf86config->conf_screen_lst = (XF86ConfScreenPtr)xf86addListItem(
		(glp)xf86config->conf_screen_lst, (glp)ScreenPtr);
    }

    xf86writeConfigFile(filename, xf86config);

    ErrorF("\n");

    if (!foundMouse) {
	ErrorF("\nXFree86 is not able to detect your mouse.\n"
		"Edit the file and correct the Device.\n");
    } else {
#ifndef __EMX__ /* OS/2 definitely has a mouse */
	ErrorF("\nXFree86 detected your mouse at device %s.\n"
		"Please check your config if the mouse is still not\n"
		"operational, as by default XFree86 tries to autodetect\n"
		"the protocol.\n",DFLT_MOUSE_DEV);
#endif
    }

    if (xf86NumScreens > 1) {
	ErrorF("\nXFree86 has configured a multihead system, please check your config.\n");
    }

    ErrorF("\nYour XF86Config file is %s\n\n", filename);
    ErrorF("To test the server, run 'XFree86 -xf86config %s'\n\n", filename);

bail:
    OsCleanup();
    AbortDDX();
    fflush(stderr);
    exit(0);
}

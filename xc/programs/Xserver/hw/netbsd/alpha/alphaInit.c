
#include    "alpha.h"
#include    "gcstruct.h"
/* #include    "mi.h" */
#include    "mibstore.h"
#include    "cfb.h"

#include <stdio.h>

#define TGASUPPORT

#ifdef TGASUPPORT
extern Bool alphaTGAInit(
#if NeedFunctionPrototypes
    int /* screen */,
    ScreenPtr /* pScreen */,
    int /* argc */,
    char** /* argv */
#endif
);
#define TGAI alphaTGAInit
#else /* }{ */
#define TGAI NULL
#endif /* } */

#if 0 /* XXX */
extern KeySymsRec sunKeySyms[];
extern SunModmapRec *sunModMaps[];
extern int sunMaxLayout;
extern KeySym* sunType4KeyMaps[];
extern SunModmapRec* sunType4ModMaps[];
#endif

static Bool	alphaDevsInited = FALSE;

#if 0
Bool sunAutoRepeatHandlersInstalled;	/* FALSE each time InitOutput called */
Bool sunSwapLkeys = FALSE;
Bool sunFlipPixels = FALSE;
Bool sunFbInfo = FALSE;
Bool sunCG4Frob = FALSE;
Bool sunNoGX = FALSE;
#endif

alphaKbdPrivRec alphaKbdPriv = {
    -1,		/* fd */
    -1,		/* type */
    -1,		/* layout */
    0,		/* click */
    (Leds)0,	/* leds */
};

alphaPtrPrivRec alphaPtrPriv = {
    -1,		/* fd */
    0		/* Current button state */
};

/*
 * The name member in the following table corresponds to the 
 * WSDISPLAY_TYPE_* macros defined in /usr/include/sys/dev/wsdisplayio.h file
 */
#define	NUM_DISPLAY_TYPES	(WSDISPLAY_TYPE_PCIMISC + 1)
alphaFbDataRec alphaFbData[NUM_DISPLAY_TYPES] = {
  { NULL, "PMAX monochrome" },
  { NULL, "PMAX color     " },
  { NULL, "CFB            " },
  { NULL, "XCFB           " },
  { NULL, "MFB            " },
  { NULL, "SFB            " },
  { NULL, "VGA            " },
  { NULL, "PCIVGA         " },
  { TGAI, "TGA            " },
  { NULL, "SFB+           " },
  { NULL, "PCI misc.      " },
};

/*
 * a list of devices to try if there is no environment or command
 * line list of devices
 */
static char *fallbackList[] = {
    "/dev/ttyE0", "/dev/ttyE1", "/dev/ttyE2", "/dev/ttyE3",
    "/dev/ttyE4", "/dev/ttyE5", "/dev/ttyE6", "/dev/ttyE7",
};
#define FALLBACK_LIST_LEN sizeof fallbackList / sizeof fallbackList[0]

fbFd alphaFbs[MAXSCREENS];

static PixmapFormatRec	formats[] = {
    { 8, 8, BITMAP_SCANLINE_PAD}	/* 8-bit deep */
};
#define NUMFORMATS	(sizeof formats)/(sizeof formats[0])

/*
 * OpenFrameBuffer --
 *	Open a frame buffer according to several rules.
 *	Find the device to use by looking in the alphaFbData table,
 *	an XDEVICE envariable, or a -dev switch.
 *
 * Results:
 *	The fd of the framebuffer.
 */
static int OpenFrameBuffer(device, screen)
    char		*device;	/* e.g. "/dev/ttyE0" */
    int			screen;    	/* what screen am I going to be */
{

    alphaFbs[screen].fd = -1;
    if (access (device, R_OK | W_OK) == -1)
	return FALSE;
    if ((alphaFbs[screen].fd = open(device, O_RDWR, 0)) == -1)
	return FALSE;
    else {
printf("OpenFrameBuffer: about to ioctl()\n");
	if (ioctl(alphaFbs[screen].fd, WSDISPLAYIO_GTYPE,
	    &alphaFbs[screen].type) == -1) {
		Error("unable to get frame buffer type");
		goto bad;
	}
printf("type = %d\n", alphaFbs[screen].type);
	if (ret == TRUE &&
	    (alphaFbs[screen].type >= NUM_DISPLAY_TYPES ||
	     !alphaFbData[alphaFbs[screen].type].init)) {
		Error("frame buffer type not supported");
		goto bad;
	}
	if (ioctl(alphaFbs[screen].fd, WSDISPLAYIO_GINFO,
	    &alphaFbs[screen].info) == -1) {
		Error("unable to get frame buffer info");
		goto bad;
	}
printf("geom = %d x %d x %d, cmsize = 0x%x\n",
  alphaFbs[screen].info.width,
  alphaFbs[screen].info.height,
  alphaFbs[screen].info.depth,
  alphaFbs[screen].info.cmsize);
    }
    return TRUE;

 bad:
    (void) close(alphaFbs[screen].fd);
    alphaFbs[screen].fd = -1;
    return FALSE;
}

/*-
 *-----------------------------------------------------------------------
 * SigIOHandler --
 *	Signal handler for SIGIO - input is available.
 *
 * Results:
 *	alphaSigIO is set - ProcessInputEvents() will be called soon.
 *
 * Side Effects:
 *	None
 *
 *-----------------------------------------------------------------------
 */
/*ARGSUSED*/
static void SigIOHandler(sig)
    int		sig;
{
    int olderrno = errno;
    alphaEnqueueEvents ();
    errno = olderrno;
}

static char** GetDeviceList (argc, argv)
    int		argc;
    char	**argv;
{
    int		i;
    char	*envList = NULL;
    char	*cmdList = NULL;
    char	**deviceList = (char **)NULL; 

    for (i = 1; i < argc; i++)
	if (strcmp (argv[i], "-dev") == 0 && i+1 < argc) {
	    cmdList = argv[i + 1];
	    break;
	}
    if (!cmdList)
	envList = getenv ("XDEVICE");

    if (cmdList || envList) {
	char	*_tmpa;
	char	*_tmpb;
	int	_i1;
	deviceList = (char **) xalloc ((MAXSCREENS + 1) * sizeof (char *));
	_tmpa = (cmdList) ? cmdList : envList;
	for (_i1 = 0; _i1 < MAXSCREENS; _i1++) {
	    _tmpb = strtok (_tmpa, ":");
	    if (_tmpb)
		deviceList[_i1] = _tmpb;
	    else
		deviceList[_i1] = NULL;
	    _tmpa = NULL;
	}
	deviceList[MAXSCREENS] = NULL;
    }
    if (!deviceList) {
	/* no environment and no cmdline, so default */
	deviceList = 
	    (char **) xalloc ((FALLBACK_LIST_LEN + 1) * sizeof (char *));
	for (i = 0; i < FALLBACK_LIST_LEN; i++)
	    deviceList[i] = fallbackList[i];
	deviceList[FALLBACK_LIST_LEN] = NULL;
    }
    return deviceList;
}

void OsVendorInit(
#if NeedFunctionPrototypes
    void
#endif
)
{
    static int inited;
fprintf(stderr, "OsVendorInit\n");
    if (!inited) {
	struct rlimit rl;

	/* 
	 * one per client, one per screen, one per listen endpoint,
	 * keyboard, mouse, and stderr
	 */
	int maxfds = MAXCLIENTS + MAXSCREENS + 5;

	if (getrlimit (RLIMIT_NOFILE, &rl) == 0) {
	    rl.rlim_cur = maxfds < rl.rlim_max ? maxfds : rl.rlim_max;
	    (void) setrlimit (RLIMIT_NOFILE, &rl);
	}
	/*
	 * XXX Need a way to specify alternate mouse and keyboard
	 * XXX devices.
	 */
	alphaKbdPriv.fd = open ("/dev/wskbd0", O_RDWR, 0);
fprintf(stderr, "alphaKbdPriv.fd = %d (%d)\n", alphaKbdPriv.fd, errno);
	alphaPtrPriv.fd = open ("/dev/wsmouse0", O_RDWR, 0);
fprintf(stderr, "alphaPtrPriv.fd = %d (%d)\n", alphaPtrPriv.fd, errno);
fflush(stderr);
        inited = 1;
    }
}

/*-
 *-----------------------------------------------------------------------
 * InitOutput --
 *	Initialize screenInfo for all actually accessible framebuffers.
 *	The
 *
 * Results:
 *	screenInfo init proc field set
 *
 * Side Effects:
 *	None
 *
 *-----------------------------------------------------------------------
 */

void InitOutput(pScreenInfo, argc, argv)
    ScreenInfo 	  *pScreenInfo;
    int     	  argc;
    char    	  **argv;
{
    int     	i, scr;
    int		nonBlockConsole = 0;
    char	**devList;
    static int	setup_on_exit = 0;
    extern Bool	RunFromSmartParent;

    if (!monitorResolution)
	monitorResolution = 90;
    if (RunFromSmartParent)
	nonBlockConsole = 1;
    for (i = 1; i < argc; i++) {
	if (!strcmp(argv[i],"-debug"))
	    nonBlockConsole = 0;
    }

    pScreenInfo->imageByteOrder = IMAGE_BYTE_ORDER;
    pScreenInfo->bitmapScanlineUnit = BITMAP_SCANLINE_UNIT;
    pScreenInfo->bitmapScanlinePad = BITMAP_SCANLINE_PAD;
    pScreenInfo->bitmapBitOrder = BITMAP_BIT_ORDER;

    pScreenInfo->numPixmapFormats = NUMFORMATS;
    for (i=0; i< NUMFORMATS; i++)
        pScreenInfo->formats[i] = formats[i];
#if 0 /* XXX */
#ifdef XKB
    if (noXkbExtension)
#endif
    sunAutoRepeatHandlersInstalled = FALSE;
#endif
    if (!alphaDevsInited) {
	/* first time ever */
	for (scr = 0; scr < MAXSCREENS; scr++)
	    alphaFbs[scr].fd = -1;
	devList = GetDeviceList (argc, argv);
	for (i = 0, scr = 0; devList[i] != NULL && scr < MAXSCREENS; i++)
	    if (OpenFrameBuffer (devList[i], scr))
		scr++;
	alphaDevsInited = TRUE;
	xfree (devList);
    }
    for (scr = 0; scr < MAXSCREENS; scr++)
	if (alphaFbs[scr].fd != -1)
	    (void) AddScreen (alphaFbData[alphaFbs[scr].type].init, 
			      argc, argv);
    (void) OsSignal(SIGWINCH, SIG_IGN);
}

/*-
 *-----------------------------------------------------------------------
 * InitInput --
 *	Initialize all supported input devices...what else is there
 *	besides pointer and keyboard?
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	Two DeviceRec's are allocated and registered as the system pointer
 *	and keyboard devices.
 *
 *-----------------------------------------------------------------------
 */
void InitInput(argc, argv)
    int     	  argc;
    char    	  **argv;
{
    DeviceIntPtr	p, k;
    extern Bool mieqInit();

    p = AddInputDevice(alphaMouseProc, TRUE);
fprintf(stderr, "added mouse = 0x%lx\n", p);
fflush(stderr);
    k = AddInputDevice(alphaKbdProc, TRUE);
fprintf(stderr, "added kbd = 0x%lx\n", k);
fflush(stderr);
    if (!p || !k)
	FatalError("failed to create input devices in InitInput");

fprintf(stderr, "going to register\n");
fflush(stderr);
    RegisterPointerDevice(p);
fprintf(stderr, "registered mouse\n");
fflush(stderr);
    RegisterKeyboardDevice(k);
fprintf(stderr, "registered kbd\n");
fflush(stderr);
    miRegisterPointerDevice(screenInfo.screens[0], p);
fprintf(stderr, "mi-registered mouse\n");
fflush(stderr);
    (void) mieqInit (k, p);
#define SET_FLOW(fd) fcntl(fd, F_SETFL, FNDELAY | FASYNC)
    (void) OsSignal(SIGIO, SigIOHandler);
#define WANT_SIGNALS(fd) fcntl(fd, F_SETOWN, getpid())
    if (alphaKbdPriv.fd >= 0) {
	if (SET_FLOW(alphaKbdPriv.fd) == -1 || WANT_SIGNALS(alphaKbdPriv.fd) == -1) {	
fprintf(stderr, "kbd failed inits (%d)\n", errno);
fflush(stderr);
	    (void) close (alphaKbdPriv.fd);
	    alphaKbdPriv.fd = -1;
	    FatalError("Async kbd I/O failed in InitInput");
	}
    }
    if (alphaPtrPriv.fd >= 0) {
	if (SET_FLOW(alphaPtrPriv.fd) == -1 || WANT_SIGNALS(alphaPtrPriv.fd) == -1) {	
fprintf(stderr, "mouse failed inits (%d)\n", errno);
fflush(stderr);
	    (void) close (alphaPtrPriv.fd);
	    alphaPtrPriv.fd = -1;
	    FatalError("Async mouse I/O failed in InitInput");
	}
    }
}

Bool
alphaCfbSetupScreen(pScreen, pbits, xsize, ysize, dpix, dpiy, width, bpp)
    register ScreenPtr pScreen;
    pointer pbits;		/* pointer to screen bitmap */
    int xsize, ysize;		/* in pixels */
    int dpix, dpiy;		/* dots per inch */
    int width;			/* pixel width of frame buffer */
    int	bpp;			/* bits per pixel of root */
{
    return cfbSetupScreen(pScreen, pbits, xsize, ysize, dpix, dpiy,
			  width);
}

Bool
alphaCfbFinishScreenInit(pScreen, pbits, xsize, ysize, dpix, dpiy, width, bpp)
    register ScreenPtr pScreen;
    pointer pbits;		/* pointer to screen bitmap */
    int xsize, ysize;		/* in pixels */
    int dpix, dpiy;		/* dots per inch */
    int width;			/* pixel width of frame buffer */
    int bpp;			/* bits per pixel of root */
{
    return cfbFinishScreenInit(pScreen, pbits, xsize, ysize, dpix, dpiy,
			       width);
}

Bool
alphaCfbScreenInit(pScreen, pbits, xsize, ysize, dpix, dpiy, width, bpp)
    register ScreenPtr pScreen;
    pointer pbits;		/* pointer to screen bitmap */
    int xsize, ysize;		/* in pixels */
    int dpix, dpiy;		/* dots per inch */
    int width;			/* pixel width of frame buffer */
    int bpp;			/* bits per pixel of root */
{
    return cfbScreenInit(pScreen, pbits, xsize, ysize, dpix, dpiy, width);
}

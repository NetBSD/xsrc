
#include    "macppc.h"
#include    "gcstruct.h"
/* #include    "mi.h" */
#include    "mibstore.h"
#include    "cfb.h"

#include <stdio.h>

extern Bool macppcOFBInit(
#if NeedFunctionPrototypes
    int /* screen */,
    ScreenPtr /* pScreen */,
    int /* argc */,
    char** /* argv */
#endif
);
#define OFBI macppcOFBInit

#if 0 /* XXX */
extern KeySymsRec sunKeySyms[];
extern SunModmapRec *sunModMaps[];
extern int sunMaxLayout;
extern KeySym* sunType4KeyMaps[];
extern SunModmapRec* sunType4ModMaps[];
#endif

static Bool macppcDevsInited = FALSE;

#if 0
Bool sunAutoRepeatHandlersInstalled;	/* FALSE each time InitOutput called */
Bool sunSwapLkeys = FALSE;
Bool sunFlipPixels = FALSE;
Bool sunFbInfo = FALSE;
Bool sunCG4Frob = FALSE;
Bool sunNoGX = FALSE;
#endif

macppcKbdPrivRec macppcKbdPriv = {
    -1,		/* fd */
    -1,		/* type */
    -1,		/* layout */
    0,		/* click */
    (Leds)0,	/* leds */
};

macppcPtrPrivRec macppcPtrPriv = {
    -1,		/* fd */
    0		/* Current button state */
};

/*
 * The name member in the following table corresponds to the
 * FBTYPE_* macros defined in /usr/include/machine/fbio.h file
 */
macppcFbDataRec macppcFbData[] = {
  { OFBI, "OFB            " },
  { NULL, "VGA            " },
  { NULL, "PCIVGA         " },
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

fbFd macppcFbs[MAXSCREENS];

static PixmapFormatRec	formats[] = {
    { 1, 1, BITMAP_SCANLINE_PAD},	/* 1-bit deep */
    { 8, 8, BITMAP_SCANLINE_PAD}	/* 8-bit deep */
};
#define NUMFORMATS	(sizeof formats)/(sizeof formats[0])

/*
 * OpenFrameBuffer --
 *	Open a frame buffer according to several rules.
 *	Find the device to use by looking in the macppcFbData table,
 *	an XDEVICE envariable, or a -dev switch.
 *
 * Results:
 *	The fd of the framebuffer.
 */
static int OpenFrameBuffer(device, screen)
    char		*device;	/* e.g. "/dev/ttyE0" */
    int			screen;    	/* what screen am I going to be */
{
    int			ret = TRUE;

    macppcFbs[screen].fd = -1;
    if (access (device, R_OK | W_OK) == -1)
	return FALSE;
    if ((macppcFbs[screen].fd = open(device, O_RDWR, 0)) == -1)
	ret = FALSE;
    else {
	int mode = WSDISPLAYIO_MODE_MAPPED;
	if (ioctl(macppcFbs[screen].fd, WSDISPLAYIO_SMODE, &mode) == -1) {
	}
	if (ioctl(macppcFbs[screen].fd, WSDISPLAYIO_GINFO,
	    &macppcFbs[screen].info) == -1) {
		Error("unable to get frame buffer info");
		(void) close(macppcFbs[screen].fd);
		macppcFbs[screen].fd = -1;
		ret = FALSE;
	}
#if 0
	if (ret) {
	    if (macppcFbs[screen].info.fb_type >= FBTYPE_LASTPLUSONE ||
		!macppcFbData[macppcFbs[screen].info.fb_type].init) {
		    Error("frame buffer type not supported");
		    (void) close(macppcFbs[screen].fd);
		    macppcFbs[screen].fd = -1;
		    ret = FALSE;
	    }
	}
#endif
    }
    if (!ret)
	macppcFbs[screen].fd = -1;
    return ret;
}

/*-
 *-----------------------------------------------------------------------
 * SigIOHandler --
 *	Signal handler for SIGIO - input is available.
 *
 * Results:
 *	macppcSigIO is set - ProcessInputEvents() will be called soon.
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

    macppcEnqueueEvents();
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

void OsVendorPreInit(
#if NeedFunctionPrototypes
    void
#endif
)
{
}

void OsVendorInit(
#if NeedFunctionPrototypes
    void
#endif
)
{
	struct rlimit rl;
	int maxfds, kbdtype;
	int i;

	static int inited;

fprintf(stderr, "OsVendorInit\n");
	if (inited)
	    return;

	/*
	 * one per client, one per screen, one per listen endpoint,
	 * keyboard, mouse, and stderr
	 */
	maxfds = MAXCLIENTS + MAXSCREENS + 5;

	if (getrlimit (RLIMIT_NOFILE, &rl) == 0) {
	    rl.rlim_cur = maxfds < rl.rlim_max ? maxfds : rl.rlim_max;
	    (void) setrlimit (RLIMIT_NOFILE, &rl);
	}

	macppcKbdPriv.fd = macppcPtrPriv.fd = -1;
	for (i = 0; i < 8; i++) {
	    char devname[16];

	    sprintf(devname, "/dev/wskbd%d", i);
	    if (macppcKbdPriv.fd == -1)
		macppcKbdPriv.fd = open(devname, O_RDWR);

	    sprintf(devname, "/dev/wsmouse%d", i);
	    if (macppcPtrPriv.fd == -1)
		macppcPtrPriv.fd = open(devname, O_RDWR);
	}

	if (macppcKbdPriv.fd == -1 || macppcPtrPriv.fd == -1)
	    err(1, "open kbd/mouse");
	noXkbExtension = FALSE;		/* XXX for now */
	inited = 1;

	if (ioctl(macppcKbdPriv.fd, WSKBDIO_GTYPE, &kbdtype) == -1) {
	    fprintf(stderr, "cannot get keyboard type\n");
	    kbdtype = 0;
	}

	/* XXX for now */
	if (kbdtype == WSKBD_TYPE_USB)
	    macppcKbdPriv.type = 1;	/* USB keyboard */
	else
	    macppcKbdPriv.type = 0;	/* ADB keyboard */

fprintf(stderr, "kbdtype = %d\n", macppcKbdPriv.type);
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
    if (!macppcDevsInited) {
	/* first time ever */
	for (scr = 0; scr < MAXSCREENS; scr++)
	    macppcFbs[scr].fd = -1;
	devList = GetDeviceList (argc, argv);
	for (i = 0, scr = 0; devList[i] != NULL && scr < MAXSCREENS; i++)
	    if (OpenFrameBuffer (devList[i], scr))
		scr++;
	macppcDevsInited = TRUE;
	xfree (devList);
    }
    for (scr = 0; scr < MAXSCREENS; scr++)
	if (macppcFbs[scr].fd != -1)
	    (void) AddScreen (macppcOFBInit, /*macppcFbData[0].init, */
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

    p = AddInputDevice(macppcMouseProc, TRUE);
    k = AddInputDevice(macppcKbdProc, TRUE);
    if (!p || !k)
	FatalError("failed to create input devices in InitInput");

    RegisterPointerDevice(p);
    RegisterKeyboardDevice(k);
    miRegisterPointerDevice(screenInfo.screens[0], p);
    (void) mieqInit (k, p);
#define SET_FLOW(fd) fcntl(fd, F_SETFL, FNDELAY | FASYNC)
    (void) OsSignal(SIGIO, SigIOHandler);
#define WANT_SIGNALS(fd) fcntl(fd, F_SETOWN, getpid())
    if (macppcKbdPriv.fd >= 0) {
	if (SET_FLOW(macppcKbdPriv.fd) == -1 || WANT_SIGNALS(macppcKbdPriv.fd) == -1) {
	    (void) close (macppcKbdPriv.fd);
	    macppcKbdPriv.fd = -1;
	    FatalError("Async kbd I/O failed in InitInput");
	}
    }
    if (macppcPtrPriv.fd >= 0) {
	if (SET_FLOW(macppcPtrPriv.fd) == -1 || WANT_SIGNALS(macppcPtrPriv.fd) == -1) {
fprintf(stderr, "mouse failed inits (%d)\n", errno);
fflush(stderr);
	    (void) close (macppcPtrPriv.fd);
	    macppcPtrPriv.fd = -1;
	    FatalError("Async mouse I/O failed in InitInput");
	}
    }
}

/*#ifdef DDXOSFATALERROR*/
void OsVendorFatalError(void)
{
}
/*#endif*/

#ifdef DPMSExtension
/**************************************************************
 * DPMSSet(), DPMSGet(), DPMSSupported()
 *
 * stubs
 *
 ***************************************************************/

void DPMSSet (level)
    int level;
{
}

int DPMSGet (level)
    int* level;
{
    return -1;
}

Bool DPMSSupported ()
{
    return FALSE;
}
#endif

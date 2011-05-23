/* $NetBSD: alphaInit.c,v 1.3 2011/05/23 18:48:44 christos Exp $ */

#include    "alpha.h"
#include    "gcstruct.h"
/* #include    "mi.h" */
#include    "mibstore.h"
#include    "cfb.h"

#include <stdio.h>
#include <err.h>

#define TGASUPPORT
#define SFBSUPPORT

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

#ifdef SFBSUPPORT
extern Bool alphaSFBInit(
#if NeedFunctionPrototypes
    int /* screen */,
    ScreenPtr /* pScreen */,
    int /* argc */,
    char** /* argv */
#endif
);
#define SFBI alphaSFBInit
#else /* }{ */
#define SFBI NULL
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
    { -1, -1, -1, -1, -1, -1, -1, -1 }, /* keys_down */
};

alphaPtrPrivRec alphaPtrPriv = {
    -1,		/* fd */
    0		/* Current button state */
};

/*
 * The name member in the following table corresponds to the 
 * WSDISPLAY_TYPE_* macros defined in <dev/wscons/wsconsio.h>.
 */
alphaFbDataRec alphaFbData[] = {
  { NULL, "unknown        " },
  { NULL, "PMAX monochrome" },
  { NULL, "PMAX color     " },
  { NULL, "CFB            " },
  { NULL, "XCFB           " },
  { NULL, "MFB            " },
  { SFBI, "SFB            " },
  { NULL, "VGA            " },
  { NULL, "PCIVGA         " },
  { TGAI, "TGA            " },
};

int NalphaFbData = sizeof(alphaFbData) / sizeof(alphaFbData[0]);

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
Bool alphaTgaAccelerate = 1;
Bool alphaSfbAccelerate = 1;

static PixmapFormatRec	formats[] = {
    { 1, 1, BITMAP_SCANLINE_PAD},
    { 8, 8, BITMAP_SCANLINE_PAD},	/* 8-bit deep */
    { 24, 32, BITMAP_SCANLINE_PAD}	/* 32-bit deep */
};
#define NUMFORMATS	(sizeof formats)/(sizeof formats[0])

static PixmapFormatRec	formats32[] = {
    { 1, 1, BITMAP_SCANLINE_PAD},
    { 24, 32, BITMAP_SCANLINE_PAD}	/* 32-bit deep */
};
#define NUMFORMATS32	(sizeof formats32)/(sizeof formats32[0])

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
    int			ret = TRUE;

    alphaFbs[screen].fd = -1;
    if (access (device, R_OK | W_OK) == -1)
	return FALSE;
    if ((alphaFbs[screen].fd = open(device, O_RDWR, 0)) == -1)
	ret = FALSE;
    else {
	if (ioctl(alphaFbs[screen].fd, WSDISPLAYIO_GTYPE,
	    &alphaFbs[screen].type) == -1) {
		Error("unable to get frame buffer type");
		(void) close(alphaFbs[screen].fd);
		alphaFbs[screen].fd = -1;
		ret = FALSE; 
	}
	if (ioctl(alphaFbs[screen].fd, WSDISPLAYIO_GINFO,
	    &alphaFbs[screen].info) == -1) {
		Error("unable to get frame buffer info");
		(void) close(alphaFbs[screen].fd);
		alphaFbs[screen].fd = -1;
		ret = FALSE; 
	}
	if (alphaFbs[screen].info.depth == 32)
		alphaFbs[screen].size = 16*1024*1024; /* XXXNJW */
	else
		alphaFbs[screen].size = 4*1024*1024;
	if (ret) {
	    if (alphaFbs[screen].type >= NalphaFbData || 
		!alphaFbData[alphaFbs[screen].type].init) {
		    Error("frame buffer type not supported");
		    (void) close(alphaFbs[screen].fd);
		    alphaFbs[screen].fd = -1;
		    ret = FALSE;
	    }
	}
    }
    if (!ret)
	alphaFbs[screen].fd = -1;
    return ret;
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
    static int inited;
    char *kbd = "/dev/wskbd0";
    char *ptr = "/dev/wsmouse0";

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
	/* warn(3) isn't X11 API, but we know we are on NetBSD */
	if((alphaKbdPriv.fd = open (kbd, O_RDWR, 0)) == -1)
		warn("Keyboard device %s", kbd);
	else if((alphaPtrPriv.fd = open (ptr, O_RDWR, 0)) == -1)
		warn("Pointer device %s", ptr);
	(void) ioctl (alphaKbdPriv.fd, WSKBDIO_GTYPE, &alphaKbdPriv.type);

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
	if (alphaFbs[scr].fd != -1) {
	    (void) AddScreen (alphaFbData[alphaFbs[scr].type].init, argc, argv);
	}
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
    k = AddInputDevice(alphaKbdProc, TRUE);
    if (!p || !k)
	FatalError("failed to create input devices in InitInput");

    RegisterPointerDevice(p);
    RegisterKeyboardDevice(k);
    miRegisterPointerDevice(screenInfo.screens[0], p);
    (void) mieqInit (k, p);
#define SET_FLOW(fd) fcntl(fd, F_SETFL, FNDELAY | FASYNC)
    (void) OsSignal(SIGIO, SigIOHandler);
#define WANT_SIGNALS(fd) fcntl(fd, F_SETOWN, getpid())
    if (alphaKbdPriv.fd >= 0) {
	if (SET_FLOW(alphaKbdPriv.fd) == -1 || WANT_SIGNALS(alphaKbdPriv.fd) == -1) {	
	    (void) close (alphaKbdPriv.fd);
	    alphaKbdPriv.fd = -1;
	    FatalError("Async kbd I/O failed in InitInput");
	}
    }
    if (alphaPtrPriv.fd >= 0) {
	if (SET_FLOW(alphaPtrPriv.fd) == -1 || WANT_SIGNALS(alphaPtrPriv.fd) == -1) {	
	    (void) close (alphaPtrPriv.fd);
	    alphaPtrPriv.fd = -1;
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

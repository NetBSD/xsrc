/* $NetBSD: hpcInit.c,v 1.1 2004/01/03 01:09:19 takemura Exp $	*/

#include    "hpc.h"
#include    "gcstruct.h"
/* #include    "mi.h" */
#include    "mibstore.h"
#include    "cfb.h"

#include <stdio.h>
#include <unistd.h>

static Bool hpcDevsInited = FALSE;

hpcKbdPrivRec hpcKbdPriv = {
    NULL,	/* button emulation device */
    0,		/* button key mask */
    0,		/* button key no release mask */
    0,		/* button trigger key down */
    -1,		/* fd */
    -1,		/* device type */
    -1,		/* type */
    0,		/* click */
    (Leds)0,	/* leds */
};

hpcPtrPrivRec hpcPtrPriv = {
    NULL,	/* button emulation device */
    0,		/* real button state */
    0,		/* emulated button state */
    0,		/* emulation button down */
    -1,		/* fd */
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

hpcFbRec hpcFbs[MAXSCREENS];

static PixmapFormatRec	formats[] = {
    {	1,	1,	BITMAP_SCANLINE_PAD},	/* 1 bit deep */
    {	8,	8,	BITMAP_SCANLINE_PAD},	/* 8 bit deep */
    {	16,	16,	BITMAP_SCANLINE_PAD},	/* 16 bit deep */
};
#define NUMFORMATS	(sizeof formats)/(sizeof formats[0])

hpcFbPtr
hpcGetScreenFb(pScreen)
    ScreenPtr	pScreen;
{
    return (&hpcFbs[pScreen->myNum]);
}

/*
 * OpenFrameBuffer --
 *	Open a frame buffer according to several rules.
 *
 * Results:
 *	The fd of the framebuffer.
 */
static int
OpenFrameBuffer(device, screen)
    char		*device;	/* e.g. "/dev/ttyE0" */
    int			screen;    	/* what screen am I going to be */
{
    int			ret = TRUE;

    hpcFbs[screen].fd = -1;
    hpcFbs[screen].devname = device;
    if (access (device, R_OK | W_OK) == -1)
	return FALSE;
    if ((hpcFbs[screen].fd = open(device, O_RDWR, 0)) == -1) {
	hpcError(device);
	ret = FALSE;
    } else {
	int mode = WSDISPLAYIO_MODE_MAPPED;
	if (ioctl(hpcFbs[screen].fd, WSDISPLAYIO_SMODE, &mode) == -1) {
		hpcError("unable to set frame buffer mode");
	}
	if (ioctl(hpcFbs[screen].fd, WSDISPLAYIO_GINFO,
	    &hpcFbs[screen].info) == -1) {
		hpcError("unable to get frame buffer info");
		(void) close(hpcFbs[screen].fd);
		hpcFbs[screen].fd = -1;
		ret = FALSE;
	}
    }
    if (!ret)
	hpcFbs[screen].fd = -1;
    return ret;
}

/*
 * SigIOHandler --
 *	Signal handler for SIGIO - input is available.
 *
 * Results:
 *	hpcSigIO is set - ProcessInputEvents() will be called soon.
 *
 * Side Effects:
 *	None
 *
 */
static void
SigIOHandler(sig)
    int		sig;
{
    int olderrno = errno;

    hpcEnqueueEvents();
    errno = olderrno;
}

static char**
GetDeviceList (argc, argv)
    int		argc;
    char	**argv;
{
    int		i;
    char	*envList = NULL;
    char	*cmdList = NULL;
    char	**deviceList = (char **)NULL;
    char	*ttyn = NULL;
    char	*p;
    int		ttyfd = -1;
    char	ttyE0[] = "/dev/ttyE0";

    for (i = 1; i < argc; i++)
	if (strcmp (argv[i], "-dev") == 0 && i+1 < argc) {
	    cmdList = argv[i + 1];
	    break;
	}

    if (!cmdList)
	envList = getenv ("XDEVICE");

    if (!cmdList && !envList) {
	if (isatty(2))
	    ttyfd = 2;
	else if (isatty(0))
	    ttyfd = 0;
	else if (isatty(1))
	    ttyfd = 1;

	if (ttyfd >= 0) {
	    if ((p = ttyname(ttyfd)) == NULL)
		ttyn = ttyE0;
	    else if (strncmp(p, ttyE0, strlen(ttyE0)-1) == 0) 
		ttyn = p;
	    else
		ttyn = ttyE0;
        } else
	    ttyn = ttyE0;
    }

    if (cmdList || envList || ttyn) {
	char	*_tmpa;
	char	*_tmpb;
	int	_i1;
	deviceList = (char **) xalloc ((MAXSCREENS + 1) * sizeof (char *));
	_tmpa = (cmdList) ? cmdList : (envList) ? envList : ttyn;
	for (_i1 = 0; _i1 < MAXSCREENS; _i1++) {
	    _tmpb = strtok (_tmpa, ":");
	    if (_tmpb)
		deviceList[_i1] = strdup(_tmpb);
	    else
		deviceList[_i1] = NULL;
	    _tmpa = NULL;
	}
	deviceList[MAXSCREENS] = NULL;
    }
#if 0
    if (!deviceList) {
	/* no environment and no cmdline, so default */
	deviceList =
	    (char **) xalloc ((FALLBACK_LIST_LEN + 1) * sizeof (char *));
	for (i = 0; i < FALLBACK_LIST_LEN; i++)
	    deviceList[i] = fallbackList[i];
	deviceList[FALLBACK_LIST_LEN] = NULL;
    }
#endif
    return deviceList;
}

void 
OsVendorInit()
{
}

void 
InitKbdMouse(argc, argv)
    int     	  argc;
    char    	  **argv;
{
	struct rlimit rl;
	int maxfds, kbdtype;
	int i;
	char **devList;

	static int inited;

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

	hpcKbdPriv.fd = hpcPtrPriv.fd = -1;

	/*
	 * use mouse multiplexer if it's available.
	 */
	hpcPtrPriv.fd = open("/dev/wsmux0", O_RDWR);

	/*
	 * try each mouse device
	 */
	for (i = 0; i < 8; i++) {
	    char devname[16];

#if 0
	    /*
	     * We can't use wskbd for now, because primary keyboard(wskbd0)
             * is already connected with console(/dev/ttyE0).
	     */
	    if (hpcKbdPriv.fd == -1) {
		sprintf(devname, "/dev/wskbd%d", i);
		hpcKbdPriv.fd = open(devname, O_RDWR);
	    }
#endif

	    if (hpcPtrPriv.fd == -1) {
		sprintf(devname, "/dev/wsmouse%d", i);
		if ((hpcPtrPriv.fd = open(devname, O_RDWR)) < 0)
		    hpcError(devname);
	    }
	}

	if (hpcKbdPriv.fd != -1) {
	    hpcKbdPriv.devtype = HPC_KBDDEV_WSKBD;
	} else {
	    /*
	     * use keyboards which are connected wsdisplay(ttyE*).
	     */
	    devList = GetDeviceList (argc, argv);
	    for (i = 0; devList[i] != NULL; i++) {
		if (0 <= (hpcKbdPriv.fd = open(devList[i], O_RDWR))) {
		    /* this isn't error */
		    hpcErrorF(("use RAW XT keyboard, %s\n", devList[i]));
		    hpcKbdPriv.devtype = HPC_KBDDEV_RAW;
		    break;
		}
	    }
	}

	if (hpcKbdPriv.fd == -1)
	    hpcFatalError(("Can't open keyboard device\n"));
	if (hpcPtrPriv.fd == -1)
	    hpcFatalError(("Can't open pointer device\n"));
	noXkbExtension = FALSE;		/* XXX for now */
	inited = 1;

	if (ioctl(hpcKbdPriv.fd, WSKBDIO_GTYPE, &kbdtype) == -1) {
	    hpcError("cannot get keyboard type\n");
	    kbdtype = 0;
	}

	switch (kbdtype) {
	case WSKBD_TYPE_USB:
		break;
	}

	/* XXX, What does this mean ??? */
	hpcKbdPriv.type = 0;
}

/*
 * InitOutput --
 *	Initialize screenInfo for all actually accessible framebuffers.
 *	The
 *
 * Results:
 *	screenInfo init proc field set
 *
 * Side Effects:
 *	None
 */
void
InitOutput(pScreenInfo, argc, argv)
    ScreenInfo 	  *pScreenInfo;
    int     	  argc;
    char    	  **argv;
{
    int     	i, scr;
    int		nonBlockConsole = 0;
    char	**devList;
    static int	setup_on_exit = 0;
    extern Bool	RunFromSmartParent;
    hpcScreenPtr pPrivate;

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
    if (!hpcDevsInited) {
	/* first time ever */
	for (scr = 0; scr < MAXSCREENS; scr++)
	    hpcFbs[scr].fd = -1;
	devList = GetDeviceList (argc, argv);
	for (i = 0, scr = 0; devList[i] != NULL && scr < MAXSCREENS; i++)
	    if (OpenFrameBuffer (devList[i], scr))
		scr++;
	hpcDevsInited = TRUE;
	xfree (devList);
    }
    for (scr = 0; scr < MAXSCREENS; scr++)
	if (hpcFbs[scr].fd != -1)
	    (void) AddScreen (hpcFBInit, argc, argv);
    (void) OsSignal(SIGWINCH, SIG_IGN);
}

/*
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
 */
void
InitInput(argc, argv)
    int     	  argc;
    char    	  **argv;
{
    DeviceIntPtr	p, k;
    extern Bool mieqInit();

    InitKbdMouse(argc, argv);

    p = AddInputDevice(hpcMouseProc, TRUE);
    k = AddInputDevice(hpcKbdProc, TRUE);
    if (!p || !k)
	hpcFatalError(("failed to create input devices in InitInput"));

    RegisterPointerDevice(p);
    RegisterKeyboardDevice(k);
    miRegisterPointerDevice(screenInfo.screens[0], p);
    (void) mieqInit (k, p);
#define SET_FLOW(fd) fcntl(fd, F_SETFL, FNDELAY | FASYNC)
    (void) OsSignal(SIGIO, SigIOHandler);
#define WANT_SIGNALS(fd) fcntl(fd, F_SETOWN, getpid())
    if (hpcKbdPriv.fd >= 0) {
	if (SET_FLOW(hpcKbdPriv.fd) == -1 ||
	    WANT_SIGNALS(hpcKbdPriv.fd) == -1) {
	    (void) close (hpcKbdPriv.fd);
	    hpcKbdPriv.fd = -1;
	    hpcFatalError(("Async kbd I/O failed in InitInput"));
	}
    }
    if (hpcPtrPriv.fd >= 0) {
	if (SET_FLOW(hpcPtrPriv.fd) == -1 ||
	    WANT_SIGNALS(hpcPtrPriv.fd) == -1) {
	    (void) close (hpcPtrPriv.fd);
	    hpcPtrPriv.fd = -1;
	    hpcFatalError(("Async mouse I/O failed in InitInput"));
	}
    }
}

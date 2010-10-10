/* $NetBSD: dreamcastInit.c,v 1.3 2010/10/10 05:33:32 tsutsui Exp $ */

/*
 * Modified from  hpcInit.c of Xhpc 
 */

#include    "dreamcast.h"
#include    "gcstruct.h"
#include    "mi.h"
#include    "mibstore.h"
#include    "cfb.h"

#include <stdio.h>
#include <unistd.h>

/* XXX should we use this or not? */
#include <dev/wscons/wsksymdef.h>

static Bool dreamcastDevsInited = FALSE;

dreamcastKbdPrivRec dreamcastKbdPriv = {
    -1,		/* fd */
    -1,		/* keyboard type */
    -1,		/* layout */
    0,		/* click */
    (Leds)0,	/* leds */
};

dreamcastPtrPrivRec dreamcastPtrPriv = {
    -1,		/* fd */
    0,		/* last known button state */
};

#if 0
/*
 * a list of devices to try if there is no environment or command
 * line list of devices
 */
static char *fallbackList[] = {
    "/dev/ttyE0", "/dev/ttyE1", "/dev/ttyE2", "/dev/ttyE3",
    "/dev/ttyE4", "/dev/ttyE5", "/dev/ttyE6", "/dev/ttyE7",
};
#define FALLBACK_LIST_LEN sizeof fallbackList / sizeof fallbackList[0]
#endif

dreamcastFbRec dreamcastFbs[MAXSCREENS];

static PixmapFormatRec	formats[] = {
    {	1,	1,	BITMAP_SCANLINE_PAD},	/* 1 bit deep */
    {	8,	8,	BITMAP_SCANLINE_PAD},	/* 8 bit deep */
    {	16,	16,	BITMAP_SCANLINE_PAD},	/* 16 bit deep */
};
#define NUMFORMATS	(sizeof formats)/(sizeof formats[0])

dreamcastFbPtr
dreamcastGetScreenFb(pScreen)
    ScreenPtr	pScreen;
{
    return (&dreamcastFbs[pScreen->myNum]);
}

/*
 * OpenFrameBuffer --
 *	Open a frame buffer according to several rules.
 *
 * Results:
 *	The fd of the framebuffer.
 */
static int
OpenFrameBuffer(char *device, int screen)
/*    char	*device;	e.g. "/dev/ttyE0" */
/*    int	screen;		what screen am I going to be */
{
    int			ret = TRUE;

    dreamcastFbs[screen].fd = -1;
    dreamcastFbs[screen].devname = device;
    if (access (device, R_OK | W_OK) == -1)
	return FALSE;
    if ((dreamcastFbs[screen].fd = open(device, O_RDWR, 0)) == -1) {
	dreamcastError(device);
	ret = FALSE;
    } else {
	int mode = WSDISPLAYIO_MODE_MAPPED;
	if (ioctl(dreamcastFbs[screen].fd, WSDISPLAYIO_SMODE, &mode) == -1) {
		dreamcastError("unable to set frame buffer mode");
	}
	if (ioctl(dreamcastFbs[screen].fd, WSDISPLAYIO_GINFO,
	    &dreamcastFbs[screen].info) == -1) {
		dreamcastError("unable to get frame buffer info");
		(void) close(dreamcastFbs[screen].fd);
		dreamcastFbs[screen].fd = -1;
		ret = FALSE;
	}
    }
    if (!ret)
	dreamcastFbs[screen].fd = -1;
    return ret;
}

/*
 * SigIOHandler --
 *	Signal handler for SIGIO - input is available.
 *
 * Results:
 *	dreamcastSigIO is set - ProcessInputEvents() will be called soon.
 *
 * Side Effects:
 *	None
 *
 */
static void
SigIOHandler(int sig)
{
    int olderrno = errno;

    dreamcastEnqueueEvents();
    errno = olderrno;
}

static char**
GetDeviceList (int argc, char **argv)
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
    return deviceList;
}

void 
OsVendorPreInit(void)
{
}

void 
OsVendorInit(void)
{
}

static void 
InitKbdMouse(int argc, char **argv)
{
	struct rlimit rl;
	int maxfds, kbdtype, layout;
	int i;

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

	dreamcastKbdPriv.fd = dreamcastPtrPriv.fd = -1;

	/*
	 * use mouse multiplexer if it's available.
	 */
	dreamcastKbdPriv.fd = open("/dev/wskbd", O_RDWR);
	dreamcastPtrPriv.fd = open("/dev/wsmouse", O_RDWR);

	/*
	 * try each mouse device
	 */
	for (i = 0; i < 8; i++) {
	    char dvname[16];

	    if (dreamcastKbdPriv.fd == -1) {
		sprintf(dvname, "/dev/wskbd%d", i);
		dreamcastKbdPriv.fd = open(dvname, O_RDWR);
	    }

	    if (dreamcastPtrPriv.fd == -1) {
		sprintf(dvname, "/dev/wsmouse%d", i);
		if ((dreamcastPtrPriv.fd = open(dvname, O_RDWR)) < 0)
		    dreamcastError(dvname);
	    }
	}

	if (dreamcastKbdPriv.fd == -1)
	    dreamcastFatalError(("Can't open keyboard device\n"));
	if (dreamcastPtrPriv.fd == -1)
	    dreamcastFatalError(("Can't open pointer device\n"));
	noXkbExtension = FALSE;		/* XXX for now */
	inited = 1;

	if (ioctl(dreamcastKbdPriv.fd, WSKBDIO_GTYPE, &kbdtype) == -1) {
	    dreamcastError("cannot get keyboard type\n");
	    kbdtype = -1;
	}

	switch (kbdtype) {
	case WSKBD_TYPE_MAPLE:
	    /* XXX: not used for now */
	    dreamcastKbdPriv.type = DREAMCAST_KBDTYPE_MAPLE;
	    break;
	default:
	    dreamcastFatalError(("Unknown keyboard type\n"));
	    break;
	}
	if (ioctl(dreamcastKbdPriv.fd, WSKBDIO_GETENCODING, &layout) == -1) {
	    dreamcastError("cannot get keyboard layout\n");
	    layout = -1;
	}

	switch (KB_ENCODING(layout)) {
	case KB_JP:
	    dreamcastKbdPriv.layout = DREAMCAST_KBDLAYOUT_JP;
	    break;
	case KB_UK:
	    dreamcastKbdPriv.layout = DREAMCAST_KBDLAYOUT_UK;
	    break;
	case KB_US:
	    dreamcastKbdPriv.layout = DREAMCAST_KBDLAYOUT_US;
	    break;
	default:
	    dreamcastError("Unknown keyboard layout; assume JP layout.\n");
	    dreamcastKbdPriv.layout = DREAMCAST_KBDLAYOUT_JP;
	    break;
	}
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
    extern Bool	RunFromSmartParent;

    if (!monitorResolution)
	monitorResolution = 75;
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
    if (!dreamcastDevsInited) {
	/* first time ever */
	for (scr = 0; scr < MAXSCREENS; scr++)
	    dreamcastFbs[scr].fd = -1;
	devList = GetDeviceList (argc, argv);
	for (i = 0, scr = 0; devList[i] != NULL && scr < MAXSCREENS; i++) {
	    if (OpenFrameBuffer (devList[i], scr))
		scr++;
	}
	dreamcastDevsInited = TRUE;
	xfree (devList);
    }
    for (scr = 0; scr < MAXSCREENS; scr++)
	if (dreamcastFbs[scr].fd != -1)
	    (void) AddScreen (dreamcastFBInit, argc, argv);
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
    pointer	p, k;

    InitKbdMouse(argc, argv);

    p = AddInputDevice(dreamcastMouseProc, TRUE);
    k = AddInputDevice(dreamcastKbdProc, TRUE);
    if (!p || !k)
	dreamcastFatalError(("failed to create input devices in InitInput"));

    RegisterPointerDevice(p);
    RegisterKeyboardDevice(k);
    miRegisterPointerDevice(screenInfo.screens[0], p);
    (void) mieqInit (k, p);
#define SET_FLOW(fd) fcntl(fd, F_SETFL, FNDELAY | FASYNC)
    (void) OsSignal(SIGIO, SigIOHandler);
#define WANT_SIGNALS(fd) fcntl(fd, F_SETOWN, getpid())
    if (dreamcastKbdPriv.fd >= 0) {
	if (SET_FLOW(dreamcastKbdPriv.fd) == -1 ||
	    WANT_SIGNALS(dreamcastKbdPriv.fd) == -1) {
	    (void) close (dreamcastKbdPriv.fd);
	    dreamcastKbdPriv.fd = -1;
	    dreamcastFatalError(("Async kbd I/O failed in InitInput"));
	}
    }
    if (dreamcastPtrPriv.fd >= 0) {
	if (SET_FLOW(dreamcastPtrPriv.fd) == -1 ||
	    WANT_SIGNALS(dreamcastPtrPriv.fd) == -1) {
	    (void) close (dreamcastPtrPriv.fd);
	    dreamcastPtrPriv.fd = -1;
	    dreamcastFatalError(("Async mouse I/O failed in InitInput"));
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
#include <Xext/dpmsproc.h>

void DPMSSet (int level)
{
}

int DPMSGet (int *level)
{
    return -1;
}

Bool DPMSSupported (void)
{
    return FALSE;
}
#endif

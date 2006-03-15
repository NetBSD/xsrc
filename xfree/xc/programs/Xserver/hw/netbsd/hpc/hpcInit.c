/* $NetBSD: hpcInit.c,v 1.7 2006/03/15 02:47:38 uwe Exp $	*/

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

    return deviceList;
}

void 
OsVendorPreInit()
{
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
	int maxfds;
	int i, fd;
	char **devList;
	Bool kbdFound;
	static int inited;
	char mousedevname[16];

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

	hpcPtrPriv.fd = -1;
	kbdFound = FALSE;

	/*
	 * use mouse multiplexer if it's available.
	 */
	if ((hpcPtrPriv.fd = open("/dev/wsmouse", O_RDWR)) != -1) {
		hpcPrintF(("mouse: /dev/wsmouse (multiplexer)\n"));
	} else {
		for (i = 0; i < 8; i++) {
			sprintf(mousedevname, "/dev/wsmouse%d", i);
			if ((hpcPtrPriv.fd = open(mousedevname, O_RDWR)) != -1) {
				hpcPrintF(("mouse: %s\n", mousedevname));
				break;
			} else {
				hpcError(mousedevname);
			}
		}
	}

	/*
	 * 1. use keyboards which are connected wsdisplay(ttyE*).
	 */
	if (!kbdFound) {
		devList = GetDeviceList (argc, argv);
		for (i = 0; devList[i] != NULL; i++) {
			int mode;
			strcpy(hpcKbdPriv.devname, devList[i]);
			if ((fd = open(hpcKbdPriv.devname, O_RDWR)) == -1)
				continue;
			/* check raw keyboard scan code support */
			if (ioctl(fd, WSKBDIO_GETMODE, &mode) != -1) {
				hpcPrintF(("keyboard: %s (RAW XT keyboard)\n",
				    devList[i]));
				hpcKbdPriv.devtype = HPC_KBDDEV_RAW;
				kbdFound = TRUE;
				close(fd);
				break;
			}
			close(fd);
		}
	}

	/*
	 * 2. use wskbd (multiplexer)
	 */
	if (!kbdFound) {
		strcpy(hpcKbdPriv.devname, "/dev/wskbd");
		if ((fd = open(hpcKbdPriv.devname, O_RDWR)) != -1) {
			hpcKbdPriv.devtype = HPC_KBDDEV_WSMUX;
			kbdFound = TRUE;
			hpcPrintF(("keyboard: /dev/wskbd (multiplexer)\n"));
			close(fd);
		}
	}

	/*
	 * 3. use wskbd*
	 */
	if (!kbdFound) {
		for (i = 0; i < 8; i++) {
			sprintf(hpcKbdPriv.devname, "/dev/wskbd%d", i);
			if ((fd = open(hpcKbdPriv.devname, O_RDWR)) != -1) {
				hpcKbdPriv.devtype = HPC_KBDDEV_WSKBD;
				kbdFound = TRUE;
				hpcPrintF(("keyboard: %s\n", hpcKbdPriv.devname));
				close(fd);
				break;
			}
		}
	}

	if (!kbdFound)
		hpcFatalError(("Can't open keyboard device\n"));
	if (hpcPtrPriv.fd == -1)
		hpcFatalError(("Can't open pointer device\n"));

	noXkbExtension = FALSE;		/* XXX for now */

	/*
	 * Try to inquire keyboard encoding type. If using wskbd,
	 * generating keymap.
	 */
	hpcKbdGetInfo(&hpcKbdPriv);
	/*
	 * Index of hpcKeySyms[]. Xhpc use only one keymap table.
	 */
	hpcKbdPriv.type = 0;

	hpcKbdPriv.fd = open(hpcKbdPriv.devname, O_RDWR);
	inited = 1;

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
 * Helper function for InitInput
 */
static void
setupAsyncIO(int *pfd, const char *descr)
{
    int fd;
    char *errmsg;

    fd = *pfd;
    if (fd < 0)
	return;

    errno = 0;
    errmsg = NULL;

    if (fcntl(fd, F_SETFL, FNDELAY | FASYNC) < 0)
	errmsg = "fcntl(F_SETFL)";
    else if (fcntl(fd, F_SETOWN, getpid()) < 0)
	errmsg = "fcntl(F_SETOWN)";

    if (errmsg) {
	hpcError(errmsg);
	(void) close(fd);
	*pfd = -1;
	hpcFatalError(("Async %s I/O setup failed in InitInput", descr));
    }
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
    (void) OsSignal(SIGIO, SigIOHandler);

    setupAsyncIO(&hpcKbdPriv.fd, "keyboard");
    setupAsyncIO(&hpcPtrPriv.fd, "mouse");
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


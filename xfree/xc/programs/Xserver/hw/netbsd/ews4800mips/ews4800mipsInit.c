/*	$NetBSD: ews4800mipsInit.c,v 1.2 2011/05/19 21:29:12 tsutsui Exp $	*/

#include "ews4800mips.h"
#include "gcstruct.h"
#include "mibstore.h"
#include "cfb.h"

#include <stdio.h>
#include <unistd.h>

/* XXX should we use this or not? */
#include <dev/wscons/wsksymdef.h>

static Bool ews4800mipsDevsInited = FALSE;

ews4800mipsKbdPrivRec ews4800mipsKbdPriv = {
	-1,		/* fd */
	-1,		/* keyboard type */
	-1,		/* layout */
	0,		/* click */
	(Leds)0,	/* leds */
};

ews4800mipsPtrPrivRec ews4800mipsPtrPriv = {
	-1,		/* fd */
	0,		/* last known button state */
};

/*
 * a list of devices to try if there is no environment or command
 * line list of devices
 */
static char *fallbackList[] = {
	"/dev/ttyE0", "/dev/ttyE1", "/dev/ttyE2", "/dev/ttyE3",
};
#define	FALLBACK_LIST_LEN	(sizeof fallbackList / sizeof fallbackList[0])

ews4800mipsFbRec ews4800mipsFbs[MAXSCREENS];

static PixmapFormatRec formats[] = {
	{ 8,	8,	BITMAP_SCANLINE_PAD },
};
#define	NUMFORMATS	((sizeof formats) / (sizeof formats[0]))

ews4800mipsFbPtr
ews4800mipsGetScreenFb(ScreenPtr pScreen)
{

	return &ews4800mipsFbs[pScreen->myNum];
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
{
	int ret = TRUE;

	ews4800mipsFbs[screen].fd = -1;
	ews4800mipsFbs[screen].devname = device;
	if (access(device, R_OK | W_OK) == -1)
		return FALSE;

	if ((ews4800mipsFbs[screen].fd = open(device, O_RDWR, 0)) == -1) {
		ews4800mipsError(device);
		ret = FALSE;
	} else {
		int mode = WSDISPLAYIO_MODE_MAPPED;

		if (ioctl(ews4800mipsFbs[screen].fd, WSDISPLAYIO_SMODE, &mode)
		    == -1) {
			ews4800mipsError("unable to set frame buffer mode");
		}

		if (ioctl(ews4800mipsFbs[screen].fd, WSDISPLAYIO_GINFO,
		    &ews4800mipsFbs[screen].info) == -1) {
			ews4800mipsError("unable to get frame buffer info");
			(void)close(ews4800mipsFbs[screen].fd);
			ews4800mipsFbs[screen].fd = -1;
			ret = FALSE;
		}
	}

	if (!ret)
		ews4800mipsFbs[screen].fd = -1;

	return ret;
}

/*
 * SigIOHandler --
 *	Signal handler for SIGIO - input is available.
 *
 * Results:
 *	ews4800mipsSigIO is set - ProcessInputEvents() will be called soon.
 *
 * Side Effects:
 *	None
 *
 */
static void
SigIOHandler(int sig)
{
	int olderrno = errno;

	ews4800mipsEnqueueEvents();
	errno = olderrno;
}

static char**
GetDeviceList(int argc, char **argv)
{
	int i;
	char *envList = NULL;
	char *cmdList = NULL;
	char **deviceList = (char **)NULL;
	char *ttyn = NULL;
	char *p;
	int ttyfd = -1;
	char ttyE0[] = "/dev/ttyE0";

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
		char *_tmpa;
		char *_tmpb;
		int _i1;
		deviceList = (char **)xalloc((MAXSCREENS + 1) * sizeof(char *));
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

void
InitKbdMouse(int argc, char **argv)
{
	struct rlimit rl;
	int maxfds, kbdtype, i;
	char **devList;
	static int inited;

	if (inited)
		return;
	inited = TRUE;

	/*
	 * one per client, one per screen, one per listen endpoint,
	 * keyboard, mouse, and stderr
	 */
	maxfds = MAXCLIENTS + MAXSCREENS + 5;

	if (getrlimit(RLIMIT_NOFILE, &rl) == 0) {
		rl.rlim_cur = maxfds < rl.rlim_max ? maxfds : rl.rlim_max;
		(void) setrlimit(RLIMIT_NOFILE, &rl);
	}

	ews4800mipsKbdPriv.fd = ews4800mipsPtrPriv.fd = -1;

	/*
	 * use mouse multiplexer if it's available.
	 */
	ews4800mipsKbdPriv.fd = open("/dev/wskbd", O_RDWR);
	ews4800mipsPtrPriv.fd = open("/dev/wsmouse", O_RDWR);

	/*
	 * try each mouse device
	 */
	for (i = 0; i < 8; i++) {
		char devname[16];

		if (ews4800mipsKbdPriv.fd == -1) {
			sprintf(devname, "/dev/wskbd%d", i);
			ews4800mipsKbdPriv.fd = open(devname, O_RDWR);
		}

		if (ews4800mipsPtrPriv.fd == -1) {
			sprintf(devname, "/dev/wsmouse%d", i);
			if ((ews4800mipsPtrPriv.fd = open(devname, O_RDWR)) < 0)
				ews4800mipsError(devname);
		}
	}

	if (ews4800mipsKbdPriv.fd == -1)
		ews4800mipsFatalError(("Can't open keyboard device\n"));
	if (ews4800mipsPtrPriv.fd == -1)
		ews4800mipsFatalError(("Can't open pointer device\n"));

	if (ioctl(ews4800mipsKbdPriv.fd, WSKBDIO_GTYPE, &kbdtype) == -1) {
		ews4800mipsError("cannot get keyboard type\n");
		kbdtype = -1;
	}

	switch (kbdtype) {
	case WSKBD_TYPE_EWS4800:	/* System board keyboard */
		ews4800mipsKbdPriv.type = WSKBD_TYPE_EWS4800;
		break;
	default:
		ews4800mipsFatalError(("Unknown keyboard type\n"));
		break;
	}

	ews4800mipsKbdPriv.layout = 0;	/* Now has only JP keyboard keymap */

	noXkbExtension = FALSE;
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
InitOutput(ScreenInfo *pScreenInfo, int argc, char **argv)
{
	int i, scr;
	int nonBlockConsole = 0;
	char **devList;
	static int setup_on_exit = 0;
	extern Bool RunFromSmartParent;
	ews4800mipsScreenPtr pPrivate;

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

	if (!ews4800mipsDevsInited) {
		/* first time ever */
		for (scr = 0; scr < MAXSCREENS; scr++)
			ews4800mipsFbs[scr].fd = -1;
		devList = GetDeviceList (argc, argv);
		for (i = 0, scr = 0; devList[i] != NULL && scr < MAXSCREENS;
		    i++) {
			if (OpenFrameBuffer (devList[i], scr))
				scr++;
		}
		ews4800mipsDevsInited = TRUE;
		xfree (devList);
	}

	for (scr = 0; scr < MAXSCREENS; scr++)
		if (ews4800mipsFbs[scr].fd != -1)
			(void) AddScreen (ews4800mipsFBInit, argc, argv);
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
InitInput(int argc, char **argv)
{
	DeviceIntPtr p, k;
	int fd;
	extern Bool mieqInit();

	InitKbdMouse(argc, argv);

	p = AddInputDevice(ews4800mipsMouseProc, TRUE);
	k = AddInputDevice(ews4800mipsKbdProc, TRUE);
	if (!p || !k)
		ews4800mipsFatalError(("failed to create input devices"
		    " in InitInput"));

	RegisterPointerDevice(p);
	RegisterKeyboardDevice(k);
	miRegisterPointerDevice(screenInfo.screens[0], p);
	mieqInit(k, p);
	OsSignal(SIGIO, SigIOHandler);

	if ((fd = ews4800mipsKbdPriv.fd) >= 0) {
		if (fcntl(fd, F_SETFL, FNDELAY | FASYNC) == -1 ||
		    fcntl(fd, F_SETOWN, getpid()) == -1) {
			close(fd);
			ews4800mipsKbdPriv.fd = -1;
			ews4800mipsFatalError(("Async kbd I/O failed in"
			    " InitInput"));
		}
	}

	if ((fd = ews4800mipsPtrPriv.fd) >= 0) {
		if (fcntl(fd, F_SETFL, FNDELAY | FASYNC) == -1 ||
		    fcntl(fd, F_SETOWN, getpid()) == -1) {
			close(fd);
			ews4800mipsPtrPriv.fd = -1;
			ews4800mipsFatalError(("Async mouse I/O failed in"
			    "  InitInput"));
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
void
DPMSSet(int level)
{
}

int
DPMSGet(int *level)
{

	return -1;
}

Bool
DPMSSupported(void)
{

	return FALSE;
}
#endif	/* DPMSExtension */

/*	$NetBSD: newsmipsInit.c,v 1.2 2005/03/30 21:16:55 tron Exp $	*/

#include "newsmips.h"
#include "gcstruct.h"
#include "mibstore.h"
#include "cfb.h"

#include <stdio.h>
#include <unistd.h>

#include <dev/wscons/wsksymdef.h>

static Bool newsmipsDevsInited = FALSE;

newsmipsKbdPrivRec newsmipsKbdPriv = {
	-1,		/* fd */
	-1,		/* keyboard type */
	-1,		/* layout */
	0,		/* click */
	(Leds)0,	/* leds */
};

newsmipsPtrPrivRec newsmipsPtrPriv = {
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

newsmipsFbRec newsmipsFbs[MAXSCREENS];

static PixmapFormatRec formats[] = {
	{ 8,	8,	BITMAP_SCANLINE_PAD },
};
#define	NUMFORMATS	((sizeof formats) / (sizeof formats[0]))

newsmipsFbPtr
newsmipsGetScreenFb(ScreenPtr pScreen)
{

	return &newsmipsFbs[pScreen->myNum];
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

	newsmipsFbs[screen].fd = -1;
	newsmipsFbs[screen].devname = device;
	if (access(device, R_OK | W_OK) == -1)
		return FALSE;

	if ((newsmipsFbs[screen].fd = open(device, O_RDWR, 0)) == -1) {
		newsmipsError(device);
		ret = FALSE;
	} else {
		int mode = WSDISPLAYIO_MODE_MAPPED;

		if (ioctl(newsmipsFbs[screen].fd, WSDISPLAYIO_SMODE, &mode)
		    == -1) {
			newsmipsError("unable to set frame buffer mode");
		}

		if (ioctl(newsmipsFbs[screen].fd, WSDISPLAYIO_GINFO,
		    &newsmipsFbs[screen].info) == -1) {
			newsmipsError("unable to get frame buffer info");
			(void)close(newsmipsFbs[screen].fd);
			newsmipsFbs[screen].fd = -1;
			ret = FALSE;
		}
	}

	if (!ret)
		newsmipsFbs[screen].fd = -1;

	return ret;
}

/*
 * SigIOHandler --
 *	Signal handler for SIGIO - input is available.
 *
 * Results:
 *	newsmipsSigIO is set - ProcessInputEvents() will be called soon.
 *
 * Side Effects:
 *	None
 *
 */
static void
SigIOHandler(int sig)
{
	int olderrno = errno;

	newsmipsEnqueueEvents();
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
OsVendorPreInit()
{
}

void
OsVendorInit()
{
}

void
InitKbdMouse(int argc, char **argv)
{
	extern int XkbDfltRepeatDelay, XkbDfltRepeatInterval;
	struct wskbd_keyrepeat_data keyrepeat;
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

	newsmipsKbdPriv.fd = newsmipsPtrPriv.fd = -1;

	/*
	 * use mouse multiplexer if it's available.
	 */
	newsmipsKbdPriv.fd = open("/dev/wskbd", O_RDWR);
	newsmipsPtrPriv.fd = open("/dev/wsmouse", O_RDWR);

	/*
	 * try each mouse device
	 */
	for (i = 0; i < 8; i++) {
		char devname[16];

		if (newsmipsKbdPriv.fd == -1) {
			sprintf(devname, "/dev/wskbd%d", i);
			newsmipsKbdPriv.fd = open(devname, O_RDWR);
		}

		if (newsmipsPtrPriv.fd == -1) {
			sprintf(devname, "/dev/wsmouse%d", i);
			if ((newsmipsPtrPriv.fd = open(devname, O_RDWR)) < 0)
				newsmipsError(devname);
		}
	}

	if (newsmipsKbdPriv.fd == -1)
		newsmipsFatalError(("Can't open keyboard device\n"));
	if (newsmipsPtrPriv.fd == -1)
		newsmipsFatalError(("Can't open pointer device\n"));

	if (ioctl(newsmipsKbdPriv.fd, WSKBDIO_GTYPE, &kbdtype) == -1) {
		newsmipsError("cannot get keyboard type\n");
		kbdtype = -1;
	}

	/* Set keyrepeat */
	if (ioctl(newsmipsKbdPriv.fd, WSKBDIO_GETKEYREPEAT, &keyrepeat) == -1)
	{
		newsmipsErrorF(("can't get keyrepeat configuration."
		    "(not fatal)\n"));
	} else {
		XkbDfltRepeatDelay = keyrepeat.del1;
		XkbDfltRepeatInterval = keyrepeat.delN;
		newsmipsErrorF(("key repeat (%d/%d)\n", XkbDfltRepeatDelay,
		    XkbDfltRepeatInterval));
	}

	newsmipsKbdPriv.layout = 0;	/* Now has only JP keyboard keymap */

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
	newsmipsScreenPtr pPrivate;

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

	if (!newsmipsDevsInited) {
		/* first time ever */
		for (scr = 0; scr < MAXSCREENS; scr++)
			newsmipsFbs[scr].fd = -1;
		devList = GetDeviceList (argc, argv);
		for (i = 0, scr = 0; devList[i] != NULL && scr < MAXSCREENS;
		    i++) {
			if (OpenFrameBuffer (devList[i], scr))
				scr++;
		}
		newsmipsDevsInited = TRUE;
		xfree (devList);
	}

	for (scr = 0; scr < MAXSCREENS; scr++)
		if (newsmipsFbs[scr].fd != -1)
			(void) AddScreen (newsmipsFBInit, argc, argv);
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

	p = AddInputDevice(newsmipsMouseProc, TRUE);
	k = AddInputDevice(newsmipsKbdProc, TRUE);
	if (!p || !k)
		newsmipsFatalError(("failed to create input devices"
		    " in InitInput"));

	RegisterPointerDevice(p);
	RegisterKeyboardDevice(k);
	miRegisterPointerDevice(screenInfo.screens[0], p);
	mieqInit(k, p);
	OsSignal(SIGIO, SigIOHandler);

	if ((fd = newsmipsKbdPriv.fd) >= 0) {
		if (fcntl(fd, F_SETFL, FNDELAY | FASYNC) == -1 ||
		    fcntl(fd, F_SETOWN, getpid()) == -1) {
			close(fd);
			newsmipsKbdPriv.fd = -1;
			newsmipsFatalError(("Async kbd I/O failed in"
			    " InitInput"));
		}
	}

	if ((fd = newsmipsPtrPriv.fd) >= 0) {
		if (fcntl(fd, F_SETFL, FNDELAY | FASYNC) == -1 ||
		    fcntl(fd, F_SETOWN, getpid()) == -1) {
			close(fd);
			newsmipsPtrPriv.fd = -1;
			newsmipsFatalError(("Async mouse I/O failed in"
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
DPMSSupported ()
{

	return FALSE;
}
#endif	/* DPMSExtension */

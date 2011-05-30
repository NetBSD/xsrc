/* $XConsortium: amigaInit.c,v 5.52 94/04/17 20:29:40 kaleb Exp $ */
/*
 * amigaInit.c --
 *	Initialization functions for screen/keyboard/mouse, etc.
 *
 * Copyright (c) 1987 by the Regents of the University of California
 *
 * Permission to use, copy, modify, and distribute this
 * software and its documentation for any purpose and without
 * fee is hereby granted, provided that the above copyright
 * notice appear in all copies.  The University of California
 * makes no representations about the suitability of this
 * software for any purpose.  It is provided "as is" without
 * express or implied warranty.
 *
 *
 */

/************************************************************
Copyright 1987 by Sun Microsystems, Inc. Mountain View, CA.

                    All Rights Reserved

Permission  to  use,  copy,  modify,  and  distribute   this
software  and  its documentation for any purpose and without
fee is hereby granted, provided that the above copyright no-
tice  appear  in all copies and that both that copyright no-
tice and this permission notice appear in  supporting  docu-
mentation,  and  that the names of Sun or X Consortium
not be used in advertising or publicity pertaining to 
distribution  of  the software  without specific prior 
written permission. Sun and X Consortium make no 
representations about the suitability of this software for 
any purpose. It is provided "as is" without any express or 
implied warranty.

SUN DISCLAIMS ALL WARRANTIES WITH REGARD TO  THIS  SOFTWARE,
INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FIT-
NESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL AMIGA BE  LI-
ABLE  FOR  ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,  DATA  OR
PROFITS,  WHETHER  IN  AN  ACTION OF CONTRACT, NEGLIGENCE OR
OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION  WITH
THE USE OR PERFORMANCE OF THIS SOFTWARE.

*******************************************************/

#define PSZ 8
#include    "amiga.h"
#include    "gcstruct.h"
#include    "mibstore.h"
#include    <sys/resource.h>
#include    <unistd.h>
#include    "mipointer.h"
#include    "mfb.h"
#include    "cfb.h"
#include    "cfb16.h"
#include    "cfb32.h"

/* maximum pixmap depth */
#ifndef AMIGAMAXDEPTH
#define AMIGAMAXDEPTH 8
#endif

/* declare init functions for all supported framebuffers. Currently,
 * we support monochrome /dev/grf0 (cc-display), generic 8bit color
 * /dev/grf[1345..], and specifically /dev/grf2 (retina-z3) with custom
 * direct blitter-access.
 */

#ifndef AMIGA_CC_COLOR
extern Bool amigaCCInit( /* Amiga Monocrome */
#if NeedFunctionPrototypes
    int /* screen */,
    ScreenPtr /* pScreen */,
    int /* argc */,
    char** /* argv */
#endif
);
#else /* AMIGA_CC_COLOR */

extern Bool amigaCInit( /* Amiga Colour (afb, Xdaniver) */
#if NeedFunctionPrototypes
    int /* screen */,
    ScreenPtr /* pScreen */,
    int /* argc */,
    char** /* argv */
#endif
);      
#endif /* AMIGA_CC_COLOR */


extern Bool amigaGRFInit(
#if NeedFunctionPrototypes
    int /* screen */,
    ScreenPtr /* pScreen */,
    int /* argc */,
    char** /* argv */
#endif
);

#ifdef RETINAZ3_SUPPORT
extern Bool amigaRZ3Init(
#if NeedFunctionPrototypes
    int /* screen */,
    ScreenPtr /* pScreen */,
    int /* argc */,
    char** /* argv */
#endif
);
#endif /* RETINAZ3_SUPPORT */

#ifdef CIRRUS_SUPPORT
extern Bool amigaCLInit(
#if NeedFunctionPrototypes
    int /* screen */,
    ScreenPtr /* pScreen */,
    int /* argc */,
    char** /* argv */ 
#endif
);
#endif /* CIRRUS_SUPPORT */

#ifdef CV64_SUPPORT
extern Bool amigaCVInit(
#if NeedFunctionPrototypes
    int /* screen */,   
    ScreenPtr /* pScreen */,
    int /* argc */,
    char** /* argv */
#endif
);
#endif

extern KeySymsRec amigaKeySyms[];
extern AmigaModmapRec *amigaModMaps[];
extern int amigaMaxLayout;
extern KeySym* amigaKeyMaps[];
extern AmigaModmapRec* amigaModMaps[];
extern char *getenv();

static Bool	amigaDevsInited = FALSE;

Bool amigaAutoRepeatHandlersInstalled;	/* FALSE each time InitOutput called */
Bool amigaFlipPixels = FALSE;
Bool amigaUseHWC = FALSE;
Bool amigaNoGX = FALSE;

int amigaVideoMode = -1;

extern int amigaCCDepth;  /* default depth for afb (amiga framebuffer) */

amigaKbdPrivRec amigaKbdPriv = {
    -1,		/* fd */
    0,		/* type (us keymap) */
};

amigaPtrPrivRec amigaPtrPriv = {
    -1,		/* fd */
    0,		/* Current button state */
    -1,		/* mouse type: default FIRM_EVENT */
    "/dev/mouse0"	/* mouse name */
};

amigaFbDataRec amigaFbData[] = {
#ifndef AMIGA_CC_COLOR
  { amigaCCInit,   "/dev/grf0" },
#else
  { amigaCInit, "/dev/grf0" },
#endif
#ifdef GFX_CARD_SUPPORT
  { amigaGRFInit,  "/dev/grf1" },
#ifdef CIRRUS_SUPPORT
  { amigaCLInit,   "/dev/grf3" },
#else /* CIRRUS_SUPPORT */
  { amigaGRFInit,  "/dev/grf3" },
#endif /* CIRRUS_SUPPORT */
  { amigaGRFInit,  "/dev/grf4" },
#ifdef CV64_SUPPORT
  { amigaCVInit,   "/dev/grf5" },
#else /* CV64_SUPPORT */
  { amigaGRFInit,  "/dev/grf5" },
#endif /* CV64_SUPPORT */
  { amigaGRFInit,  "/dev/grf6" },
  { amigaGRFInit,  "/dev/grf7" },
  { amigaGRFInit,  "/dev/grf8" },
  { amigaGRFInit,  "/dev/grf9" },
#ifdef RETINAZ3_SUPPORT
  { amigaRZ3Init,  "/dev/grf2" },
#else /* RETINAZ3_SUPPORT */
  { amigaGRFInit,  "/dev/grf2" },
#endif /* RETINAZ3_SUPPORT */
#endif /* GFX_CARD_SUPPORT */
};
#define MAXFBTYPE (sizeof amigaFbData / sizeof amigaFbData[0])

/*
 * a list of devices to try if there is no environment or command
 * line list of devices
 */
#ifndef GFX_CARD_SUPPORT /* { */
static char *fallbackList[] = {
    "/dev/grf0",
};
#else /* }{ */
static char *fallbackList[] = {
    "/dev/grf9",
    "/dev/grf8",
    "/dev/grf7",
    "/dev/grf6",
    "/dev/grf5",
    "/dev/grf4",
    "/dev/grf3",
    "/dev/grf2",
    "/dev/grf1",
    "/dev/grf0",
};
#endif /* } */

#define FALLBACK_LIST_LEN sizeof fallbackList / sizeof fallbackList[0]

fbFd amigaFbs[MAXSCREENS];

static PixmapFormatRec	formats[] = {
    { 1, 1, BITMAP_SCANLINE_PAD	} /* 1-bit deep */
#ifdef AMIGA_CC_COLOR
    ,{ 4, 4, BITMAP_SCANLINE_PAD} /* Variable depth for afb part */
#endif
#ifdef GFX_CARD_SUPPORT
    ,{ 8, 8, BITMAP_SCANLINE_PAD} /* 8-bit deep */
    ,{ 15, 16, BITMAP_SCANLINE_PAD } /* 15-Bit deep */
    ,{ 16, 16, BITMAP_SCANLINE_PAD } /* 16-bit deep */
    ,{ 24, 32, BITMAP_SCANLINE_PAD } /* 24-bit deep */
#endif
};
#define NUMFORMATS	(sizeof formats)/(sizeof formats[0])

/*
 * OpenFrameBuffer --
 *	Open a frame buffer according to several rules.
 *	Find the device to use by looking in the amigaFbData table,
 *	an XDEVICE envariable, a -dev switch.
 *
 * Results:
 *	The fd of the framebuffer.
 */
static int OpenFrameBuffer(device, screen, vmode)
    char		*device;	/* e.g. "/dev/grf0" */
    int			screen;    	/* what screen am I going to be */
    int			vmode;
{
    int			ret = TRUE;
    struct grfinfo	*grfinfo;
    static int		devFbUsed;
    int 		type;

    amigaFbs[screen].fd = -1;

    /* first check whether we support this device at all */
    for (type = 0; type < MAXFBTYPE; type++)
      if (! strcmp(device, amigaFbData[type].devname))
	break;

    if (type == MAXFBTYPE)
	return FALSE;

    if (access (device, R_OK | W_OK) == -1)
	return FALSE;
    if ((amigaFbs[screen].fd = open(device, O_RDWR|O_EXLOCK, 0)) == -1)
	ret = FALSE;
    else {
        if (ioctl (amigaFbs[screen].fd, GRFIOCON, 0) == -1)
	  ret = FALSE;
	else {
	  /* just give switching a try if vmode != -1 */
	  if (vmode >= 0)
	    ioctl (amigaFbs[screen].fd, GRFSETVMODE, &vmode);

	  grfinfo = (struct grfinfo *) xalloc (sizeof (struct grfinfo));
	  if (ioctl(amigaFbs[screen].fd, GRFIOCGINFO, grfinfo) == -1) {
	    xfree (grfinfo);
	    grfinfo = NULL;
	    Error("unable to get frame buffer attributes");
	    (void) close(amigaFbs[screen].fd);
	    amigaFbs[screen].fd = -1;
	    ret = FALSE; 
	  }
	  if (ret) {
	    if (grfinfo)
	      amigaFbs[screen].info = *grfinfo;
	    
	    amigaFbs[screen].type = type;
	  }
	}
    }
    if (!ret)
	amigaFbs[screen].fd = -1;
    return ret;
}

/*ARGSUSED*/
static void amigaInBlockHandler(data, err, p)
	pointer data, p;
	int err;
{
}

/*ARGSUSED*/
static void amigaInWakeupHandler(data, err, p)
	pointer data, p;
	int err;
{
    int olderrno = errno;
    amigaEnqueueEvents ();
    errno = olderrno;
}

/*-
 *-----------------------------------------------------------------------
 * amigaNonBlockConsoleOff --
 *	Turn non-blocking mode on the console off, so you don't get logged
 *	out when the server exits.
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	None.
 *
 *-----------------------------------------------------------------------
 */
void amigaNonBlockConsoleOff(
#if NeedFunctionPrototypes
    void
#endif
)
{
    register int i;

    i = fcntl(2, F_GETFL, 0);
    if (i >= 0)
	(void) fcntl(2, F_SETFL, i & ~FNDELAY);
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

	amigaKbdPriv.fd = open ("/dev/kbd", O_RDWR, 0);
	if (amigaKbdPriv.fd < 0)
		Error("OsVendorInit: can't open keyboard");

	amigaPtrPriv.fd = open (amigaPtrPriv.mousename, O_RDWR|O_NONBLOCK, 0);
	if (amigaPtrPriv.fd < 0) {
		ErrorF("OsVendorInit: can't open mouse device ");
		Error(amigaPtrPriv.mousename);
	}

	/* perhaps someone is going to provide multiple layouts? */
	amigaKeySyms[0].map = amigaKeyMaps[0];
	amigaModMaps[0] = amigaModMaps[0];

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
	if (!strcmp(argv[i],"-mode"))
	    amigaVideoMode = atoi(argv[++i]);
    }

    /*
     *	Writes to /dev/console can block - causing an
     *	excess of error messages to hang the server in
     *	deadlock.  So.......
     */
    if (nonBlockConsole) {
	if (!setup_on_exit) {
	    if (atexit(amigaNonBlockConsoleOff))
		ErrorF("InitOutput: can't register NBIO exit handler\n");

	    setup_on_exit = 1;
	}
	i = fcntl(2, F_GETFL, 0);
	if (i >= 0)
	    i = fcntl(2, F_SETFL, i | FNDELAY);
	if (i < 0) {
	    Error("fcntl");
	    ErrorF("InitOutput: can't put stderr in non-block mode\n");
	}
    }
    pScreenInfo->imageByteOrder = IMAGE_BYTE_ORDER;
    pScreenInfo->bitmapScanlineUnit = BITMAP_SCANLINE_UNIT;
    pScreenInfo->bitmapScanlinePad = BITMAP_SCANLINE_PAD;
    pScreenInfo->bitmapBitOrder = BITMAP_BIT_ORDER;

    pScreenInfo->numPixmapFormats = NUMFORMATS;
    for (i=0; i< NUMFORMATS; i++)
        pScreenInfo->formats[i] = formats[i];

#ifdef AMIGA_CC_COLOR
    /* Init afs Pixmap format */
    if (amigaCCDepth != 1) {
	pScreenInfo->formats [1].depth = amigaCCDepth;
	pScreenInfo->formats [1].bitsPerPixel = amigaCCDepth > 4 ? 8 : 4;
    }
#endif

#ifdef XKB
    if (noXkbExtension)
#endif
    amigaAutoRepeatHandlersInstalled = FALSE;
    if (!amigaDevsInited) {
	/* first time ever */
	for (scr = 0; scr < MAXSCREENS; scr++)
	    amigaFbs[scr].fd = -1;
	devList = GetDeviceList (argc, argv);
	for (i = 0, scr = 0; devList[i] != NULL && scr < MAXSCREENS; i++) {
#ifdef AMIGA_CC_COLOR
	    if (0 == strcmp(devList[i],"/dev/grf0")) { /*special case cc-gfx */
		if (amigaCProbe(pScreenInfo, i, scr, argc, argv)) {
		    amigaFbs[scr].type = 0;
		    scr++;
		}
		else FatalError ("no CC Framebuffer\n");
	    } else {
#endif
		if (OpenFrameBuffer (devList[i], scr,amigaVideoMode))
		    scr++;
#ifdef AMIGA_CC_COLOR
	    }
#endif
        }
	amigaDevsInited = TRUE;
	xfree (devList);
    }
    for (scr = 0; scr < MAXSCREENS; scr++)
	if (amigaFbs[scr].fd != -1)
	    (void) AddScreen (amigaFbData[amigaFbs[scr].type].init, 
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
    int		i;
    DeviceIntPtr	p, k;
    extern Bool mieqInit();

    pid_t mypid = getpid();


#if 0
    amigaPtrPriv.mousename = "/dev/mouse0";
    for (i = 1; i < argc; i++)
	if (strcmp (argv[i], "-mouse") == 0 && i+1 < argc) {
	    amigaPtrPriv.mousename = argv[i + 1];
	    break;
	}
#endif

    p = AddInputDevice(amigaMouseProc, TRUE);
    k = AddInputDevice(amigaKbdProc, TRUE);
    if (!p || !k)
	FatalError("failed to create input devices in InitInput");

    RegisterPointerDevice(p);
    RegisterKeyboardDevice(k);
    miRegisterPointerDevice(screenInfo.screens[0], p);
    (void) mieqInit (k, p);

#define SET_FLOW(fd) fcntl(fd, F_SETFL, FNDELAY)

    (void) RegisterBlockAndWakeupHandlers(amigaInBlockHandler, 
	amigaInWakeupHandler, (void *)0);

#define WANT_SIGNALS(fd) fcntl(fd, F_SETOWN, mypid)
    if (amigaKbdPriv.fd >= 0) {
	if (SET_FLOW(amigaKbdPriv.fd) == -1) {	
	    (void) close (amigaKbdPriv.fd);
	    amigaKbdPriv.fd = -1;
	    FatalError("Async kbd I/O failed in InitInput");
	}
	AddEnabledDevice(amigaKbdPriv.fd);
    }
    if (amigaPtrPriv.fd >= 0) {
	if (SET_FLOW(amigaPtrPriv.fd) == -1) {	
	    (void) close (amigaPtrPriv.fd);
	    amigaPtrPriv.fd = -1;
	    FatalError("Async mouse I/O failed in InitInput");
	}
	AddEnabledDevice(amigaPtrPriv.fd);
    }
}


#ifdef GFX_CARD_SUPPORT
#if AMIGAMAXDEPTH == 8

Bool
amigaCfbSetupScreen(pScreen, pbits, xsize, ysize, dpix, dpiy, width, bpp)
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
amigaCfbFinishScreenInit(pScreen, pbits, xsize, ysize, dpix, dpiy, width, bpp)
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
amigaCfbScreenInit(pScreen, pbits, xsize, ysize, dpix, dpiy, width, bpp)
    register ScreenPtr pScreen;
    pointer pbits;		/* pointer to screen bitmap */
    int xsize, ysize;		/* in pixels */
    int dpix, dpiy;		/* dots per inch */
    int width;			/* pixel width of frame buffer */
    int bpp;			/* bits per pixel of root */
{
    return cfbScreenInit(pScreen, pbits, xsize, ysize, dpix, dpiy, width);
}

#else /* AMIGAMAXDEPTH != 8 */
#if AMIGAMAXDEPTH == 32

Bool
amigaCfbCreateGC(pGC)
    GCPtr   pGC;
{
    if (pGC->depth == 1)
    {
	return mfbCreateGC (pGC);
    }
    else if (pGC->depth <= 8)
    {
	return cfbCreateGC (pGC);
    }
    else if (pGC->depth <= 16)
    {
	return cfb16CreateGC (pGC);
    }
    else if (pGC->depth <= 32)
    {
	return cfb32CreateGC (pGC);
    }
    return FALSE;
}

static void
amigaCfbGetSpans(pDrawable, wMax, ppt, pwidth, nspans, pdstStart)
    DrawablePtr		pDrawable;	/* drawable from which to get bits */
    int			wMax;		/* largest value of all *pwidths */
    register DDXPointPtr ppt;		/* points to start copying from */
    int			*pwidth;	/* list of number of bits to copy */
    int			nspans;		/* number of scanlines to copy */
    char		*pdstStart;	/* where to put the bits */
{
    switch (pDrawable->bitsPerPixel) {
    case 1:
	mfbGetSpans(pDrawable, wMax, ppt, pwidth, nspans, pdstStart);
	break;
    case 8:
	cfbGetSpans(pDrawable, wMax, ppt, pwidth, nspans, pdstStart);
	break;
    case 15:
    case 16:
	cfb16GetSpans(pDrawable, wMax, ppt, pwidth, nspans, pdstStart);
	break;
    case 24:
    case 32:
	cfb32GetSpans(pDrawable, wMax, ppt, pwidth, nspans, pdstStart);
	break;
    }
    return;
}

static void
amigaCfbGetImage(pDrawable, sx, sy, w, h, format, planeMask, pdstLine)
    DrawablePtr pDrawable;
    int		sx, sy, w, h;
    unsigned int format;
    unsigned long planeMask;
    char	*pdstLine;
{
    switch (pDrawable->bitsPerPixel)
    {
    case 1:
	mfbGetImage(pDrawable, sx, sy, w, h, format, planeMask, pdstLine);
	break;
    case 8:
	cfbGetImage(pDrawable, sx, sy, w, h, format, planeMask, pdstLine);
	break;
    case 15:
    case 16:
	cfb16GetImage(pDrawable, sx, sy, w, h, format, planeMask, pdstLine);
	break;
    case 24:
    case 32:
	cfb32GetImage(pDrawable, sx, sy, w, h, format, planeMask, pdstLine);
	break;
    }
}

Bool
amigaCfbSetupScreen(pScreen, pbits, xsize, ysize, dpix, dpiy, width, bpp)
    register ScreenPtr pScreen;
    pointer pbits;		/* pointer to screen bitmap */
    int xsize, ysize;		/* in pixels */
    int dpix, dpiy;		/* dots per inch */
    int width;			/* pixel width of frame buffer */
    int	bpp;			/* bits per pixel of root */
{
    extern int			cfbWindowPrivateIndex;
    extern int			cfbGCPrivateIndex;
    int ret;

    switch (bpp) {
    case 8:
	ret = cfbSetupScreen(pScreen, pbits, xsize, ysize, dpix, dpiy, width);
	break;
    case 15:
    case 16:
	ret = cfb16SetupScreen(pScreen, pbits, xsize, ysize, dpix, dpiy, width);
	break;
    case 24:
    case 32:
	ret = cfb32SetupScreen(pScreen, pbits, xsize, ysize, dpix, dpiy, width);
	break;
    default:
	return FALSE;
    }
    pScreen->CreateGC = amigaCfbCreateGC;
    pScreen->GetImage = amigaCfbGetImage;
    pScreen->GetSpans = amigaCfbGetSpans;
    return ret;
}

/* extern miBSFuncRec	cfbBSFuncRec, cfb16BSFuncRec, cfb32BSFuncRec; */
extern int  cfb16ScreenPrivateIndex, cfb32ScreenPrivateIndex;
extern Bool cfbCloseScreen(), cfb16CloseScreen(), cfb32CloseScreen();

Bool
amigaCfbFinishScreenInit(pScreen, pbits, xsize, ysize, dpix, dpiy, width, bpp)
    register ScreenPtr pScreen;
    pointer pbits;		/* pointer to screen bitmap */
    int xsize, ysize;		/* in pixels */
    int dpix, dpiy;		/* dots per inch */
    int width;			/* pixel width of frame buffer */
    int bpp;
{
  switch (bpp) 
    {
    case 8:
      return cfbFinishScreenInit(pScreen, pbits, xsize, ysize,
				 dpix, dpiy, width);
    case 15:  
    case 16:
      return cfb16FinishScreenInit(pScreen, pbits, xsize, ysize,
				   dpix, dpiy, width);
      
    case 24:
    case 32:
      return cfb32FinishScreenInit(pScreen, pbits, xsize, ysize,
				   dpix, dpiy, width);
      
    default:
      ErrorF("Unsupported depth!");
      return 0;
    }
}


Bool
amigaCfbScreenInit(pScreen, pbits, xsize, ysize, dpix, dpiy, width, bpp)
    register ScreenPtr pScreen;
    pointer pbits;		/* pointer to screen bitmap */
    int xsize, ysize;		/* in pixels */
    int dpix, dpiy;		/* dots per inch */
    int width;			/* pixel width of frame buffer */
    int bpp;
{
    if (!amigaCfbSetupScreen(pScreen, pbits, xsize, ysize, dpix, dpiy,
			   width, bpp))
	return FALSE;
    return amigaCfbFinishScreenInit(pScreen, pbits, xsize, ysize, dpix,
				  dpiy, width, bpp);
}

#endif  /* AMIGAMAXDEPTH == 32 */
#endif  /* AMIGAMAXDEPTH */
#endif  /* GFX_CARD_SUPPORT */

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

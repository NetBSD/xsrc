/*
 * Copyright 1996 by Steven Lang <tiger@tyger.org>
 *       Modified for the AceCad Tablet,
 *                by Shane Watts <shane@bofh.asn.au>
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Steven Lang not be used in advertising
 * or publicity pertaining to distribution of the software without specific,
 * written prior permission.  Steven Lang makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * STEVEN LANG DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL STEVEN LANG BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTIONS, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

/* $XFree86: xc/programs/Xserver/hw/xfree86/input/acecad/xf86AceCad.c,v 1.3 1999/06/13 05:18:54 dawes Exp $ */

#define NEED_EVENTS
#include "X.h"
#include "Xproto.h"
#include "misc.h"
#include "inputstr.h"
#include "scrnintstr.h"
#include "XI.h"
#include "XIproto.h"

#if defined(sun) && !defined(i386)
#define POSIX_TTY
#include <errno.h>
#include <termio.h>
#include <fcntl.h>
#include <ctype.h>

#include "extio.h"
#else
#include "compiler.h"

#include "xf86.h"
#include "xf86Priv.h"
#include "xf86_OSproc.h"
#ifdef XFree86LOADER 
#include "xf86_ansic.h" 
#endif 
#include "xf86Config.h"
#include "xf86Xinput.h"
#include "atKeynames.h"
#include "xf86Version.h"
#endif

#if !defined(sun) || defined(i386)
#include "osdep.h"
#include "exevents.h"

#include "extnsionst.h"
#include "extinit.h"
#endif

/*
** Debugging macros
*/
#ifdef DBG
#undef DBG
#endif
#ifdef DEBUG
#undef DEBUG
#endif

static int      debug_level = 0;
#define DEBUG	1
#if DEBUG
#define 	DBG(lvl, f) 	{if ((lvl) <= debug_level) f;}
#else
#define 	DBG(lvl, f)
#endif

/*
** Device records
*/
#define ABSOLUTE_FLAG		1
#define STYLUS_FLAG		2

typedef struct 
{
    char	*acecadDevice;		/* device file name */
    int		acecadInc;		/* increment between transmits */
    int		acecadButTrans;		/* button translation flags */
    int		acecadOldX;		/* previous X position */
    int		acecadOldY;		/* previous Y position */
    int		acecadOldProximity;	/* previous proximity */
    int		acecadOldButtons;	/* previous buttons state */
    int		acecadMaxX;		/* max X value */
    int		acecadMaxY;		/* max Y value */
    int		acecadXLeft;		/* screen left */
    int		acecadXRight;		/* screen right */
    int		acecadYtop;		/* screen top */
    int		acecadYbot;		/* screen bottom */
    int		acecadRes;		/* resolution in lines per inch */
    int		flags;			/* various flags */
    int		acecadIndex;		/* number of bytes read */
    unsigned char acecadData[5];	/* data read on the device */
} AceCadDeviceRec, *AceCadDevicePtr;

/*
** Configuration data
*/
#define ACECAD_SECTION_NAME "AceCad"
#define PORT		1
#define DEVICENAME	2
#define THE_MODE	3
#define CURSOR		4
#define INCREMENT	5
#define BORDER		6
#define DEBUG_LEVEL     7
#define HISTORY_SIZE	8
#define ALWAYS_CORE	9

#if !defined(sun) || defined(i386)
static SymTabRec AceCadTab[] = {
	{ENDSUBSECTION,		"endsubsection"},
	{PORT,			"port"},
	{DEVICENAME,		"devicename"},
	{THE_MODE,		"mode"},
	{CURSOR,		"cursor"},
	{INCREMENT,		"increment"},
	{BORDER,		"border"},
	{DEBUG_LEVEL,		"debuglevel"},
	{HISTORY_SIZE,		"historysize"},
	{ ALWAYS_CORE,		"alwayscore" },
	{-1,			""}
};

#define RELATIVE	1
#define ABSOLUTE	2

static SymTabRec AceCadModeTabRec[] = {
	{RELATIVE,	"relative"},
	{ABSOLUTE,	"absolute"},
	{-1,		""}
};

#define PUCK		1
#define STYLUS		2

static SymTabRec AceCadPointTabRec[] = {
	{PUCK,		"puck"},
	{STYLUS,	"stylus"},
	{-1,		""}
};
  
#endif

/*
** Contants and macro
*/
#define BUFFER_SIZE	256		/* size of reception buffer */
#define XI_NAME 	"ACECAD"	/* X device name for the stylus */

#define SYSCALL(call) while(((call) == -1) && (errno == EINTR))

#define ACECAD_CONFIG		"a"		/* Send configuration (max coords) */

#define ACECAD_ABSOLUTE		'F'		/* Absolute mode */
#define ACECAD_RELATIVE		'E'		/* Relative mode */

#define ACECAD_UPPER_ORIGIN	"b"		/* Origin upper left */

#define ACECAD_PROMPT_MODE	"B"		/* Prompt mode */
#define ACECAD_STREAM_MODE	"@"		/* Stream mode */
#define ACECAD_INCREMENT	'I'		/* Set increment */
#define ACECAD_BINARY_FMT	"zb"		/* Binary reporting */

#define ACECAD_PROMPT		"P"		/* Prompt for current position */

static const char * acecad_initstr = ACECAD_BINARY_FMT ACECAD_STREAM_MODE;

#define PHASING_BIT	0x80
#define PROXIMITY_BIT	0x40
#define TABID_BIT	0x20
#define XSIGN_BIT	0x10
#define YSIGN_BIT	0x08
#define BUTTON_BITS	0x07
#define COORD_BITS	0x7f

/*
** External declarations
*/
#if defined(sun) && !defined(i386)
#define ENQUEUE	suneqEnqueue
#else
#define ENQUEUE	xf86eqEnqueue

extern void xf86eqEnqueue(
#if NeedFunctionPrototypes
    xEventPtr /*e*/
#endif
);
#endif

extern void miPointerDeltaCursor(
#if NeedFunctionPrototypes
    int /*dx*/,
    int /*dy*/,
    unsigned long /*time*/
#endif
);

#if !defined(sun) || defined(i386)
/*
** xf86AceCadConfig
** Reads the AceCad section from the XF86Config file
*/
static Bool
xf86AceCadConfig(LocalDevicePtr *array, int inx, int max, LexPtr val)
{
    LocalDevicePtr	dev = array[inx];
    AceCadDevicePtr	priv = (AceCadDevicePtr)(dev->private);
    int			token;
    int			mtoken;

    DBG(1, ErrorF("xf86AceCadConfig\n"));

    while ((token = xf86GetToken(AceCadTab)) != ENDSUBSECTION) {
	switch(token) {
	case DEVICENAME:
	    if (xf86GetToken(NULL) != STRING)
		xf86ConfigError("Option string expected");
	    else {
		dev->name = strdup(val->str);
		if (xf86Verbose)
		    ErrorF("%s AceCad X device name is %s\n", XCONFIG_GIVEN,
			   dev->name);
	    }
	    break;

	case PORT:
	    if (xf86GetToken(NULL) != STRING)
		xf86ConfigError("Option string expected");
	    else {
		priv->acecadDevice = strdup(val->str);
		if (xf86Verbose)
		    ErrorF("%s AceCad port is %s\n", XCONFIG_GIVEN,
			   priv->acecadDevice);
	    }
	    break;

	case THE_MODE:
	    mtoken = xf86GetToken(AceCadModeTabRec);
	    if ((mtoken == EOF) || (mtoken == STRING) || (mtoken == NUMBER)) 
		xf86ConfigError("Mode type token expected");
	    else {
		switch (mtoken) {
		case ABSOLUTE:
		    priv->flags |= ABSOLUTE_FLAG;
		    break;
		case RELATIVE:
		    priv->flags &= ~ABSOLUTE_FLAG;
		    break;
		default:
		    xf86ConfigError("Illegal Mode type");
		    break;
		}
	    }
	    break;

	case CURSOR:
	    mtoken = xf86GetToken(AceCadPointTabRec);
	    if ((mtoken == EOF) || (mtoken == STRING) || (mtoken == NUMBER)) 
		xf86ConfigError("Cursor token expected");
	    else {
		switch (mtoken) {
		case STYLUS:
		    priv->flags |= STYLUS_FLAG;
		    break;
		case PUCK:
		    priv->flags &= ~STYLUS_FLAG;
		    break;
		default:
		    xf86ConfigError("Illegal cursor type");
		    break;
		}
	    }
	    break;

	case INCREMENT:
	    if (xf86GetToken(NULL) != NUMBER)
		xf86ConfigError("Option number expected");
	    priv->acecadInc = val->num;
	    if (xf86Verbose)
		ErrorF("%s AceCad increment value is %d\n", XCONFIG_GIVEN,
		       priv->acecadInc);
	    break;

	case DEBUG_LEVEL:
	    if (xf86GetToken(NULL) != NUMBER)
		xf86ConfigError("Option number expected");
	    debug_level = val->num;
	    if (xf86Verbose) {
#if DEBUG
		ErrorF("%s AceCad debug level sets to %d\n", XCONFIG_GIVEN,
		       debug_level);
#else
		ErrorF("%s AceCad debug level not sets to %d because"
		       " debugging is not compiled\n", XCONFIG_GIVEN,
		       debug_level);
#endif
	    }
	    break;

	case HISTORY_SIZE:
	    if (xf86GetToken(NULL) != NUMBER)
		xf86ConfigError("Option number expected");
	    dev->history_size = val->num;
	    if (xf86Verbose)
		ErrorF("%s AceCad Motion history size is %d\n", XCONFIG_GIVEN,
		       dev->history_size);      
	    break;

	case ALWAYS_CORE:
	    xf86AlwaysCore(dev, TRUE);
	    if (xf86Verbose)
		ErrorF("%s AceCad device always stays core pointer\n",
		       XCONFIG_GIVEN);
	    break;

	case EOF:
	    FatalError("Unexpected EOF (missing EndSubSection)");
	    break;

	default:
	    xf86ConfigError("AceCad subsection keyword expected");
	    break;
	}
    }

    DBG(1, ErrorF("xf86AceCadConfig name=%s\n", priv->acecadDevice));

    return Success;
}
#endif

/*
** xf86AceCadReadInput
** Reads from the AceCad and posts any new events to the server.
*/
static void
xf86AceCadReadInput(LocalDevicePtr local)
{
    AceCadDevicePtr	priv = (AceCadDevicePtr) local->private;
    int			len, loop;
    int			is_core_pointer, is_absolute;
    int			x, y, buttons, prox;
    DeviceIntPtr	device;
    unsigned char	buffer[BUFFER_SIZE];
  
    DBG(7, ErrorF("xf86AceCadReadInput BEGIN device=%s fd=%d\n",
       priv->acecadDevice, local->fd));

    SYSCALL(len = read(local->fd, buffer, sizeof(buffer)));

    if (len <= 0) {
	Error("error reading AceCad device");
	return;
    }

    for(loop=0; loop<len; loop++) {

/* Format of 5 bytes data packet for AceCad Tablets
       Byte 1
       bit 7  Phasing bit always 1
       bit 6  Proximity bit
       bit 5  Tablet ID
       bit 4  X sign (Always 1 for absolute)
       bit 3  Y sign (Always 1 for absolute)
       bit 2-0 Button status  

       Byte 2
       bit 7  Always 0
       bits 6-0 = X6 - X0

       Byte 3 (Absolute mode only)
       bit 7  Always 0
       bits 6-0 = X13 - X7

       Byte 4
       bit 7  Always 0
       bits 6-0 = Y6 - Y0

       Byte 5 (Absolute mode only)
       bit 7  Always 0
       bits 6-0 = Y13 - Y7
*/
  
	if ((priv->acecadIndex == 0) && !(buffer[loop] & PHASING_BIT)) { /* magic bit is not OK */
	    DBG(6, ErrorF("xf86AceCadReadInput bad magic number 0x%x\n", buffer[loop]));;
	    continue;
	}

	priv->acecadData[priv->acecadIndex++] = buffer[loop];

	if (priv->acecadIndex == (priv->flags & ABSOLUTE_FLAG? 5: 3)) {
/* the packet is OK */
/* reset char count for next read */
	    priv->acecadIndex = 0;

	    if (priv->flags & ABSOLUTE_FLAG) {
		x = (int)priv->acecadData[1] + ((int)priv->acecadData[2] << 7);
		y = (int)priv->acecadData[3] + ((int)priv->acecadData[4] << 7);
	    } else {
		x = priv->acecadData[0] & XSIGN_BIT? priv->acecadData[1]: -priv->acecadData[1];
		y = priv->acecadData[0] & YSIGN_BIT? priv->acecadData[2]: -priv->acecadData[2];
	    }

/*	    x = priv->acecadMaxX - x;	/**/
	    y = priv->acecadMaxY - y;	/**/

	    prox = (priv->acecadData[0] & PROXIMITY_BIT)? 0: 1;

	    buttons = (priv->acecadData[0] & BUTTON_BITS);

	    device = local->dev;

	    DBG(6, ErrorF("prox=%s\tx=%d\ty=%d\tbuttons=%d\n",
		   prox ? "true" : "false", x, y, buttons));

	    is_absolute = (priv->flags & ABSOLUTE_FLAG);
	    is_core_pointer = xf86IsCorePointer(device);

	    if (is_core_pointer) {
		x = x * screenInfo.screens[0]->width / priv->acecadMaxX;
		y = y * screenInfo.screens[0]->height / priv->acecadMaxY;
		DBG(6, ErrorF("Adjusted coords x=%d y=%d\n", x, y));
	    }

/* coordonates are ready we can send events */
	    if (prox) {
		if (!(priv->acecadOldProximity))
		    if (!is_core_pointer)
			xf86PostProximityEvent(device, 1, 0, 2, x, y);

		if ((is_absolute && ((priv->acecadOldX != x) || (priv->acecadOldY != y)))
		       || (!is_absolute && (x || y))) {
		    if (is_absolute || priv->acecadOldProximity) {
			xf86PostMotionEvent(device, is_absolute, 0, 2, x, y);
		    }
		}

		if (priv->acecadOldButtons != buttons) {
		int	delta;
		int	button;

		    delta = buttons - priv->acecadOldButtons;
		    button = (delta > 0)? delta: ((delta == 0)?
			   priv->acecadOldButtons : -delta);

		    if (priv->acecadOldButtons != buttons) {
			DBG(6, ErrorF("xf86AceCadReadInput button=%d\n", button));

			xf86PostButtonEvent(device, is_absolute, button,
			       (delta >0), 0, 2, x, y);
		    }

		}

		priv->acecadOldButtons = buttons;
		priv->acecadOldX = x;
		priv->acecadOldY = y;
		priv->acecadOldProximity = prox;
	    } else { /* !PROXIMITY */
/* Any changes in buttons are ignored when !proximity */
		if (!is_core_pointer)
		    if (priv->acecadOldProximity)
			xf86PostProximityEvent(device, 0, 0, 2, x, y);
		priv->acecadOldProximity = 0;
	    }
	}
    }
    DBG(7, ErrorF("xf86AceCadReadInput END   device=0x%x priv=0x%x\n",
	   local->dev, priv));
}

/*
** xf86AceCadControlProc
** It really does do something.  Honest!
*/
static void
xf86AceCadControlProc(DeviceIntPtr	device, PtrCtrl *ctrl)
{
    DBG(2, ErrorF("xf86AceCadControlProc\n"));
}

/*
** write_and_read
** Write data, and get the response.
*/
static char *
write_and_read(int fd, char *data, char *buffer, int len, int cr_term)
{
    int err, numread = 0;
    fd_set readfds;
    struct timeval timeout;

    SYSCALL(err = write(fd, data, strlen(data)));
    if (err == -1) {
	Error("AceCad write");
	return NULL;
    }

    FD_ZERO(&readfds);
    FD_SET(fd, &readfds);
    while (numread < len) {
	timeout.tv_sec = 0;
	timeout.tv_usec = 200000;

	SYSCALL(err = select(FD_SETSIZE, &readfds, NULL, NULL, &timeout));
	if (err == -1) {
	    Error("AceCad select");
	    return NULL;
	}
	if (!err) {
	    ErrorF("Timeout while reading AceCad tablet. No tablet connected ???\n");
	    return NULL;
	}

	SYSCALL(err = read(fd, buffer + numread++, 1));
	if (err == -1) {
	    Error("AceCad read");
	    return NULL;
	}
	if (!err) {
	    --numread;
	    break;
	}
	if (cr_term && buffer[numread - 1] == '\r') {
	    break;
	    buffer[numread - 1] = 0;
	}
    }
    buffer[numread] = 0;
    return buffer;
}

/*
** xf86AceCadOpen
** Open and initialize the tablet, as well as probe for any needed data.
*/
static Bool
xf86AceCadOpen(LocalDevicePtr local)
{
    struct termios	termios_tty;
    struct timeval	timeout;
    char		buffer[256];
    int			err;
    AceCadDevicePtr	priv = (AceCadDevicePtr)local->private;

    DBG(1, ErrorF("opening %s\n", priv->acecadDevice));

    SYSCALL(local->fd = open(priv->acecadDevice, O_RDWR|O_NDELAY, 0));
    if (local->fd == -1) {
	Error(priv->acecadDevice);
	return !Success;
    }
    DBG(2, ErrorF("%s opened as fd %d\n", priv->acecadDevice, local->fd));

#ifdef POSIX_TTY
    err = tcgetattr(local->fd, &termios_tty);
    if (err == -1) {
	Error("AceCad tcgetattr");
	return !Success;
    }
    termios_tty.c_iflag = IXOFF;
    termios_tty.c_cflag = B9600|CS8|CREAD|CLOCAL|HUPCL|PARENB|PARODD;
    termios_tty.c_lflag = 0;

/* I wonder what these all do, anyway */
    termios_tty.c_cc[VINTR] = 0;
    termios_tty.c_cc[VQUIT] = 0;
    termios_tty.c_cc[VERASE] = 0;
#ifdef VWERASE
    termios_tty.c_cc[VWERASE] = 0;
#endif
#ifdef VREPRINT
    termios_tty.c_cc[VREPRINT] = 0;
#endif
    termios_tty.c_cc[VKILL] = 0;
    termios_tty.c_cc[VEOF] = 0;
    termios_tty.c_cc[VEOL] = 0;
#ifdef VEOL2
    termios_tty.c_cc[VEOL2] = 0;
#endif
    termios_tty.c_cc[VSUSP] = 0;
#ifdef VDISCARD
    termios_tty.c_cc[VDISCARD] = 0;
#endif
#ifdef VLNEXT
    termios_tty.c_cc[VLNEXT] = 0; 
#endif

    termios_tty.c_cc[VMIN] = 1 ;
    termios_tty.c_cc[VTIME] = 10 ;

    err = tcsetattr(local->fd, TCSANOW, &termios_tty);
    if (err == -1) {
	Error("AceCad tcsetattr TCSANOW");
	return !Success;
    }
#else
    Code for someone else to write to handle OSs without POSIX tty functions
#endif

    DBG(1, ErrorF("initializing AceCad tablet\n"));

/* Send reset (NULL) to the tablet */
    SYSCALL(err = write(local->fd, "\0", 1));
    if (err == -1) {
	Error("AceCad write");
	return !Success;
    }

/* wait 200 mSecs, just in case */
    timeout.tv_sec = 0;
    timeout.tv_usec = 200000;
    SYSCALL(err = select(0, NULL, NULL, NULL, &timeout));
    if (err == -1) {
	Error("AceCad select");
	return !Success;
    }

/* Put it in prompt mode so it doens't say anything before we're ready */
    SYSCALL(err = write(local->fd, ACECAD_PROMPT_MODE, strlen(ACECAD_PROMPT_MODE)));
    if (err == -1) {
	Error("AceCad write");
	return !Success;
    }
/* Clear any pending input */
    tcflush(local->fd, TCIFLUSH);
  
/*    DBG(2, ErrorF("Reading Firmware ID\n"));						*/
/*    if (!write_and_read(local->fd, ACECAD_PROMPT, buffer, 5, 1))			*/
/*      return !Success;								*/

/*    DBG(2, ErrorF("%s\n", buffer));							*/

/*    if (xf86Verbose)									*/
/*	ErrorF("%s AceCad firmware ID : %s\n", XCONFIG_PROBED, buffer);			*/

    DBG(2, ErrorF("reading max coordinates\n"));
    if (!write_and_read(local->fd, ACECAD_CONFIG, buffer, 5, 0))
	return !Success;
    priv->acecadMaxX = (int)buffer[1] + ((int)buffer[2] << 7);
    priv->acecadMaxY = (int)buffer[3] + ((int)buffer[4] << 7);

/*    priv->acecadMaxX = 6000; */
/*    priv->acecadMaxY = 6000; */

    if (xf86Verbose)
	ErrorF("%s AceCad tablet size is %d.%1dinx%d.%1din, %dx%d "
	       "lines of resolution\n", XCONFIG_PROBED, 
	       priv->acecadMaxX / 500, (priv->acecadMaxX / 50) % 10,
	       priv->acecadMaxY / 500, (priv->acecadMaxY / 50) % 10,
	       priv->acecadMaxX, priv->acecadMaxY);

    if (priv->acecadInc > 95)
	priv->acecadInc = 95;
    if (priv->acecadInc < 1) {
/* Make a guess as to the best increment value given video mode */
	if (priv->acecadMaxX / screenInfo.screens[0]->width <
	       priv->acecadMaxY / screenInfo.screens[0]->height)
	    priv->acecadInc = priv->acecadMaxX / screenInfo.screens[0]->width;
	else
	    priv->acecadInc = priv->acecadMaxY / screenInfo.screens[0]->height;
	if (priv->acecadInc < 1)
	    priv->acecadInc = 1;
	if (xf86Verbose)
	    ErrorF("%s Using increment value of %d\n", XCONFIG_PROBED,
		   priv->acecadInc);
    }

/* Sets up the tablet mode to increment, stream, and such */
    sprintf(buffer, "%s%c%c%c", acecad_initstr, ACECAD_INCREMENT, 32 + priv->acecadInc,
	   (priv->flags & ABSOLUTE_FLAG)? ACECAD_ABSOLUTE: ACECAD_RELATIVE);

    SYSCALL(err = write(local->fd, buffer, strlen(buffer)))
    if (err == -1) {
	Error("AceCad write");
	return !Success;
    }

    if (err <= 0) {
	SYSCALL(close(local->fd));
	return !Success;
    }	

    return Success;
}

/*
** xf86AceCadOpenDevice
** Opens and initializes the device driver stuff or sumpthin.
*/
static int
xf86AceCadOpenDevice(DeviceIntPtr pAceCad)
{
    LocalDevicePtr	local = (LocalDevicePtr)pAceCad->public.devicePrivate;
    AceCadDevicePtr	priv = (AceCadDevicePtr)XI_PRIVATE(pAceCad);

    if (xf86AceCadOpen(local) != Success) {
	if (local->fd >= 0) {
	    SYSCALL(close(local->fd));
	}
	local->fd = -1;
    }

/* Set the real values */
    InitValuatorAxisStruct(pAceCad,
			   0,
			   0,			/* min val */
			   priv->acecadMaxX,	/* max val */
			   500000,		/* resolution */
			   0,			/* min_res */
			   500000);		/* max_res */
    InitValuatorAxisStruct(pAceCad,
			   1,
			   0,			/* min val */
			   priv->acecadMaxY,	/* max val */
			   500000,		/* resolution */
			   0,			/* min_res */
			   500000);		/* max_res */
    return (local->fd != -1);
}

/*
** xf86AceCadProc
** Handle requests to do stuff to the driver.
*/
static int
xf86AceCadProc(DeviceIntPtr pAceCad, int what)
{
    CARD8		map[25];
    int			nbaxes;
    int			nbbuttons;
    int			loop;
    LocalDevicePtr	local = (LocalDevicePtr)pAceCad->public.devicePrivate;
    AceCadDevicePtr	priv = (AceCadDevicePtr)PRIVATE(pAceCad);

    DBG(2, ErrorF("BEGIN xf86AceCadProc dev=0x%x priv=0x%x what=%d\n", pAceCad, priv, what));

    switch (what) {
	case DEVICE_INIT:
	    DBG(1, ErrorF("xf86AceCadProc pAceCad=0x%x what=INIT\n", pAceCad));

	    nbaxes = 2;			/* X, Y */
	    nbbuttons = (priv->flags & STYLUS_FLAG)? 2: 4;

	    for(loop=1; loop<=nbbuttons; loop++) map[loop] = loop;

	    if (InitButtonClassDeviceStruct(pAceCad,
					    nbbuttons,
					    map) == FALSE) {
		ErrorF("unable to allocate Button class device\n");
		return !Success;
	    }

	    if (InitFocusClassDeviceStruct(pAceCad) == FALSE) {
		ErrorF("unable to init Focus class device\n");
		return !Success;
	    }

	    if (InitPtrFeedbackClassDeviceStruct(pAceCad,
		   xf86AceCadControlProc) == FALSE) {
		ErrorF("unable to init ptr feedback\n");
		return !Success;
	    }

	    if (InitProximityClassDeviceStruct(pAceCad) == FALSE) {
		ErrorF("unable to init proximity class device\n"); 
		return !Success;
	    }

	    if (InitValuatorClassDeviceStruct(pAceCad,
		   nbaxes,
		   xf86GetMotionEvents,
		   local->history_size,
		   (priv->flags & ABSOLUTE_FLAG)? Absolute: Relative)
		   == FALSE) {
		ErrorF("unable to allocate Valuator class device\n"); 
		return !Success;
	    }
/* allocate the motion history buffer if needed */
	    xf86MotionHistoryAllocate(local);

	    AssignTypeAndName(pAceCad, local->atom, local->name);
/* open the device to gather informations */
	    xf86AceCadOpenDevice(pAceCad);
	    break;

	case DEVICE_ON:
	    DBG(1, ErrorF("xf86AceCadProc pAceCad=0x%x what=ON\n", pAceCad));

	    if ((local->fd < 0) && (!xf86AceCadOpenDevice(pAceCad))) {
		return !Success;
	    }
/*	    SYSCALL(write(local->fd, ACECAD_PROMPT, strlen(ACECAD_PROMPT)));	*/
	    AddEnabledDevice(local->fd);
	    pAceCad->public.on = TRUE;
	    break;

	case DEVICE_OFF:
	    DBG(1, ErrorF("xf86AceCadProc  pAceCad=0x%x what=%s\n", pAceCad,
		   (what == DEVICE_CLOSE) ? "CLOSE" : "OFF"));
	    if (local->fd >= 0)
		RemoveEnabledDevice(local->fd);
	    pAceCad->public.on = FALSE;
	    break;

	case DEVICE_CLOSE:
	    DBG(1, ErrorF("xf86AceCadProc  pAceCad=0x%x what=%s\n", pAceCad,
		   (what == DEVICE_CLOSE) ? "CLOSE" : "OFF"));
	    SYSCALL(close(local->fd));
	    local->fd = -1;
	    break;

	default:
	    ErrorF("unsupported mode=%d\n", what);
	    return !Success;
	    break;
    }
    DBG(2, ErrorF("END   xf86AceCadProc Success what=%d dev=0x%x priv=0x%x\n",
	   what, pAceCad, priv));
    return Success;
}

/*
** xf86AceCadClose
** It...  Uh...  Closes the physical device?
*/
static void
xf86AceCadClose(LocalDevicePtr local)
{
    if (local->fd >= 0) {
	SYSCALL(close(local->fd));
    }
    local->fd = -1;
}

/*
** xf86AceCadChangeControl
** When I figure out what it does, it will do it.
*/
static int
xf86AceCadChangeControl(LocalDevicePtr local, pointer control)
{
    xDeviceResolutionCtl	*res;

    res = (xDeviceResolutionCtl *)control;
	
    if ((res->control != DEVICE_RESOLUTION) ||
	   (res->num_valuators < 1))
	return (BadMatch);

    return(Success);
}

/*
** xf86AceCadSwitchMode
** Switches the mode.  For now just absolute or relative, hopefully
** more on the way.
*/
static int
xf86AceCadSwitchMode(ClientPtr client, DeviceIntPtr dev, int mode)
{
    LocalDevicePtr	local = (LocalDevicePtr)dev->public.devicePrivate;
    AceCadDevicePtr	priv = (AceCadDevicePtr)(local->private);
    char		newmode;

    DBG(3, ErrorF("xf86AceCadSwitchMode dev=0x%x mode=%d\n", dev, mode));

    switch(mode) {
	case Absolute:
	    priv->flags |= ABSOLUTE_FLAG;
	    newmode = ACECAD_ABSOLUTE;
	    break;

	case Relative:
	    priv->flags &= ~ABSOLUTE_FLAG;
	    newmode = ACECAD_RELATIVE;
	    break;

	default:
	    DBG(1, ErrorF("xf86AceCadSwitchMode dev=0x%x invalid mode=%d\n",
		   dev, mode));
	    return BadMatch;
    }
    SYSCALL(write(local->fd, &newmode, 1));
    return Success;
}

/*
** xf86AceCadAllocate
** Allocates the device structures for the AceCad.
*/
static LocalDevicePtr
xf86AceCadAllocate()
{
    LocalDevicePtr	local = xalloc(sizeof(LocalDeviceRec));
    AceCadDevicePtr	priv = xalloc(sizeof(AceCadDeviceRec));
#if defined (sun) && !defined(i386)
    char		*dev_name = getenv("ACECAD_DEV");
#endif

    local->name = XI_NAME;
    local->type_name = "AceCad Tablet";
    local->flags = 0;
#if !defined(sun) || defined(i386)
    local->device_config = xf86AceCadConfig;
#endif
    local->device_control = xf86AceCadProc;
    local->read_input = xf86AceCadReadInput;
    local->control_proc = xf86AceCadChangeControl;
    local->close_proc = xf86AceCadClose;
    local->switch_mode = xf86AceCadSwitchMode;
    local->fd = -1;
    local->atom = 0;
    local->dev = NULL;
    local->private = priv;
    local->private_flags = 0;
    local->history_size  = 0;

#if defined(sun) && !defined(i386)
    if (def_name) {
	priv->acecadDevice = xalloc(strlen(dev_name) + 1);
	strcpy(priv->acecadDevice, device_name);
	ErrorF("xf86AceCadOpen port changed to '%s'\n", priv->acecadDevice);
    } else {
	priv->acecadDevice = "";
    }
#else
    priv->acecadDevice = "";         /* device file name */
#endif
    priv->acecadInc = -1;            /* re-transmit position on increment */
    priv->acecadOldX = -1;           /* previous X position */
    priv->acecadOldY = -1;           /* previous Y position */
    priv->acecadOldProximity = 0;    /* previous proximity */
    priv->acecadOldButtons = 0;      /* previous buttons state */
    priv->acecadMaxX = -1;           /* max X value */
    priv->acecadMaxY = -1;           /* max Y value */
    priv->flags = 0;                 /* various flags */
    priv->acecadIndex = 0;           /* number of bytes read */

    return local;
}


/*
** AceCad device association
** Device section name and allocation function.
*/
DeviceAssocRec acecad_assoc =
{
  ACECAD_SECTION_NAME,           /* config_section_name */
  xf86AceCadAllocate             /* device_allocate */
};

#ifdef DYNAMIC_MODULE
/*
** init_module
** Entry point for dynamic module.
*/
int
#ifndef DLSYM_BUG
init_module(unsigned long server_version)
#else
init_xf86AceCad(unsigned long server_version)
#endif
{
    xf86AddDeviceAssoc(&acecad_assoc);

    if (server_version != XF86_VERSION_CURRENT) {
	ErrorF("Warning: AceCad module compiled for version%s\n",
	       XF86_VERSION);
	return 0;
    } else {
	return 1;
    }
}
#endif

#ifdef XFree86LOADER
/*
 * Entry point for the loader code
 */
XF86ModuleVersionInfo xf86AceCadVersion = {
    "xf86AceCad",
    MODULEVENDORSTRING,
    MODINFOSTRING1,
    MODINFOSTRING2,
    XF86_VERSION_CURRENT,
    0x00010000,
    {0,0,0,0}
};

void
xf86AceCadModuleInit(data, magic)
    pointer *data;
    INT32 *magic;
{
    static int cnt = 0;

    switch (cnt) {
      case 0:
      *magic = MAGIC_VERSION;
      *data = &xf86AceCadVersion;
      cnt++;
      break;
      
      case 1:
      *magic = MAGIC_ADD_XINPUT_DEVICE;
      *data = &acecad_assoc;
      cnt++;
      break;

      default:
      *magic = MAGIC_DONE;
      *data = NULL;
      break;
    } 
}
#endif


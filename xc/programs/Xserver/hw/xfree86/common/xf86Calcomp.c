/*
 * Copyright 1999 by Martin Kroeker <mk@daveg.com>
 * 
 * based on xf86Summa.c by Steven Lang, whose original copyright message 
 * appears below :
 * Copyright 1996 by Steven Lang <tiger@tyger.org>
 * 
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Martin Kroeker not be used in advertising
 * or publicity pertaining to distribution of the software without specific,
 * written prior permission. Martin Kroeker makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * MARTIN KROEKER DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL MARTIN KROEKER BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTIONS, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 *
 */
/* $XFree86: xc/programs/Xserver/hw/xfree86/common/xf86Calcomp.c,v 1.1.2.2 1999/05/25 12:00:32 hohndel Exp $ */
 
/* xf86Calcomp.c V1.0/XFree86-3.3.3.1 mk 9.3.1999*/ 

#include "Xos.h"
#include <signal.h>
#include <stdio.h>

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

#ifdef XFree86LOADER
#include "xf86_libc.h"
#endif
#include "xf86.h"
#include "xf86Procs.h"
#include "xf86_OSlib.h"
#include "xf86_Config.h"
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
#define DEBUG	0
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
    char	*calDevice;	/* device file name */
    int		calInc;		/* increment between transmits */
    int		calButTrans;	/* button translation flags */
    int		calOldX;	/* previous X position */
    int		calOldY;	/* previous Y position */
    int		calOldProximity; /* previous proximity */
    int		calOldButtons;	/* previous buttons state */
    int		calMaxX;	/* max X value */
    int		calMaxY;	/* max Y value */
    int		calXSize;	/* active area X size */
    int		calXOffset;	/* active area X offset */
    int		calYSize;	/* active area Y size */
    int		calYOffset;	/* active area Y offset */
    int		calRes;		/* resolution in lines per inch */
    int		flags;		/* various flags */
    int		calIndex;	/* number of bytes read */
    unsigned char calData[13];	/* data read on the device */
} CalcompDeviceRec, *CalcompDevicePtr;

/*
** Configuration data
*/
#define CALCOMP_SECTION_NAME "DrawingBoard"
#define PORT		1
#define DEVICENAME	2
#define THE_MODE	3
#define CURSOR		4
#define INCREMENT	5
#define BORDER		6
#define DEBUG_LEVEL     7
#define HISTORY_SIZE	8
#define ALWAYS_CORE	9
#define ACTIVE_AREA	10
#define ACTIVE_OFFSET	11

#if !defined(sun) || defined(i386)
static SymTabRec CalTab[] = {
	{ENDSUBSECTION,		"endsubsection"},
	{PORT,			"port"},
	{DEVICENAME,		"devicename"},
	{THE_MODE,		"mode"},
	{CURSOR,		"cursor"},
	{INCREMENT,		"increment"},
	{BORDER,		"border"},
	{DEBUG_LEVEL,		"debuglevel"},
	{HISTORY_SIZE,		"historysize"},
	{ALWAYS_CORE,		"alwayscore"},
	{ACTIVE_AREA,		"activearea"},
	{ACTIVE_OFFSET,		"activeoffset"},
	{-1,			""}
};

#define RELATIVE	1
#define ABSOLUTE	2

static SymTabRec CalModeTabRec[] = {
	{RELATIVE,	"relative"},
	{ABSOLUTE,	"absolute"},
	{-1,		""}
};

#define PUCK		1
#define STYLUS		2

static SymTabRec CalPointTabRec[] = {
	{PUCK,		"puck"},
	{STYLUS,	"stylus"},
	{-1,		""}
};
  
#endif

/*
** Contants and macro
*/
#define BUFFER_SIZE	256	/* size of reception buffer */
#define XI_NAME 	"CALCOMP"	/* X device name for the stylus */

#define SYSCALL(call) while(((call) == -1) && (errno == EINTR))

#define DB_FORMAT       "\033%^23\r"    /* Calcomp high-res binary format */
#define DB_COMM_SETUP	"\033%C1N81\r"	/* Serial communication setup */
#define DB_FIRMID	"\033%__V\r"	/* Request firmware ID string */
#define DB_CONFIG	"\033%VS\r"	/* Send configuration (max coords) */

#define DB_ABSOLUTE	"\033%IR\r"	/* Absolute (incremental run) mode */
#define DB_RELATIVE	"\033%IT\r"	/* Relative (incremental track) mode */

#define DB_LINEFEED	"\033%L1\r"	/* Set line feed on data */
#define DB_1000LPI	"\033%JR1000,0\r" /* 1000 lines per inch */

#define DB_PROMPT_MODE	"\033%Q?\r"	/* Prompt mode (untested,unused!) */
#define DB_X_INCR	"\033%X1\r"     /* Set X increment to 1 */
#define DB_Y_INCR       "\033%Y1\r"	/* Set Y increment to 1 */

#define DB_PROMPT	"?"	/* Prompt character (unused) */


#define PHASING_BIT	0x80
#define PROXIMITY_BIT	0x20
#define BUTTON_BITS	0x7c
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
** xf86CalConfig
** Reads the DrawingBoard section from the XF86Config file
*/
static Bool
xf86CalConfig(LocalDevicePtr *array, int inx, int max, LexPtr val)
{
    LocalDevicePtr	dev = array[inx];
    CalcompDevicePtr	priv = (CalcompDevicePtr)(dev->private);
    int			token;
    int			mtoken;

    DBG(1, ErrorF("xf86CalConfig\n"));

    while ((token = xf86GetToken(CalTab)) != ENDSUBSECTION) {
	switch(token) {
	case DEVICENAME:
	    if (xf86GetToken(NULL) != STRING)
		xf86ConfigError("Option string expected");
	    else {
		dev->name = strdup(val->str);
		if (xf86Verbose)
		    ErrorF("%s Calcomp DrawingBoard X device name is %s\n", XCONFIG_GIVEN,
			   dev->name);
	    }
	    break;

	case PORT:
	    if (xf86GetToken(NULL) != STRING)
		xf86ConfigError("Option string expected");
	    else {
		priv->calDevice = strdup(val->str);
		if (xf86Verbose)
		    ErrorF("%s Calcomp DrawingBoard port is %s\n", XCONFIG_GIVEN,
			   priv->calDevice);
	    }
	    break;

	case THE_MODE:
	    mtoken = xf86GetToken(CalModeTabRec);
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
	    mtoken = xf86GetToken(CalPointTabRec);
	    if ((mtoken == EOF) || (mtoken == STRING) || (mtoken == NUMBER)) 
		xf86ConfigError("Cursor token expected");
	    else {
		switch (mtoken) {
		case STYLUS:
		    priv->flags |= STYLUS_FLAG;
		    break;
		case PUCK:
		    xf86ConfigError("16 button cursor not supported");
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
	    priv->calInc = val->num;
	    if (xf86Verbose)
		ErrorF("%s Calcomp DrawingBoard increment value is %d\n", XCONFIG_GIVEN,
		       priv->calInc);
	    break;

	case DEBUG_LEVEL:
	    if (xf86GetToken(NULL) != NUMBER)
		xf86ConfigError("Option number expected");
	    debug_level = val->num;
	    if (xf86Verbose) {
#if DEBUG
		ErrorF("%s Calcomp DrawingBoard debug level sets to %d\n", XCONFIG_GIVEN,
		       debug_level);
#else
		ErrorF("%s Calcomp DrawingBoard debug level not sets to %d because"
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
		ErrorF("%s Calcomp DrawingBoard Motion history size is %d\n", XCONFIG_GIVEN,
		       dev->history_size);      
	    break;

	case ALWAYS_CORE:
	    xf86AlwaysCore(dev, TRUE);
	    if (xf86Verbose)
		ErrorF("%s Calcomp device always stays core pointer\n",
		       XCONFIG_GIVEN);
	    break;

	case ACTIVE_AREA:
	    if (xf86GetToken(NULL) != NUMBER)
		xf86ConfigError("Option number expected");
	    priv->calXSize = val->num;
	    if (xf86GetToken(NULL) != NUMBER)
		xf86ConfigError("Option number expected");
	    priv->calYSize = val->num;
	    if (xf86Verbose)
		ErrorF("%s Calcomp DrawingBoard active area set to %d.%1dx%d.%1d"
		       " inches\n", XCONFIG_GIVEN, priv->calXSize / 10,
		       priv->calXSize % 10, priv->calYSize / 10,
		       priv->calYSize % 10);
	    break;
	    
	case ACTIVE_OFFSET:
	    if (xf86GetToken(NULL) != NUMBER)
		xf86ConfigError("Option number expected");
	    priv->calXOffset = val->num;
	    if (xf86GetToken(NULL) != NUMBER)
		xf86ConfigError("Option number expected");
	    priv->calYOffset = val->num;
	    if (xf86Verbose)
		ErrorF("%s Calcomp DrawingBoard active area offset set to %d.%1dx%d.%1d"
		       " inches\n", XCONFIG_GIVEN, priv->calXOffset / 10,
		       priv->calXOffset % 10, priv->calYOffset / 10,
		       priv->calYOffset % 10);
	    break;

	case EOF:
	    FatalError("Unexpected EOF (missing EndSubSection)");
	    break;

	default:
	    xf86ConfigError("DrawingBoard subsection keyword expected");
	    break;
	}
    }

    DBG(1, ErrorF("xf86CalConfig name=%s\n", priv->calDevice));

    return Success;
}
#endif

/*
** xf86CalConvert
** Convert valuators to X and Y.
*/
static Bool
xf86CalConvert(LocalDevicePtr	local,
	       int		first,
	       int		num,
	       int		v0,
	       int		v1,
	       int		v2,
	       int		v3,
	       int		v4,
	       int		v5,
	       int*		x,
	       int*		y)
{
    CalcompDevicePtr	priv = (CalcompDevicePtr) local->private;

    if (first != 0 || num == 1)
      return FALSE;

    *x = ((v0 - priv->calXOffset) * screenInfo.screens[0]->width) / priv->calXSize;
    *y = ((v1 - priv->calYOffset) * screenInfo.screens[0]->height) / priv->calYSize;
    if (*x < 0)
	*x = 0;
    if (*y < 0)
	*y = 0;
    if (*x > screenInfo.screens[0]->width)
	*x = screenInfo.screens[0]->width;
    if (*y > screenInfo.screens[0]->height)
	*y = screenInfo.screens[0]->height;

    DBG(6, ErrorF("Adjusted coords x=%d y=%d\n", *x, *y));

    return TRUE;
}

/*
** xf86CalReverseConvert
** Convert X and Y to valuators.
*/
static Bool
xf86CalReverseConvert(LocalDevicePtr	local,
		      int		x,
		      int		y,
		      int		*valuators)
{
    CalcompDevicePtr	priv = (CalcompDevicePtr) local->private;

    valuators[0] = ((x * priv->calXSize) / screenInfo.screens[0]->width) + priv->calXOffset;
    valuators[1] = ((y * priv->calYSize) / screenInfo.screens[0]->height) + priv->calYOffset;
    
    DBG(6, ErrorF("Adjusted valuators v0=%d v1=%d\n", valuators[0], valuators[1]));

    return TRUE;
}

/*
** xf86CalReadInput
** Reads from the DrawingBoard and posts any new events to the server.
*/
static void
xf86CalReadInput(LocalDevicePtr local)
{
    CalcompDevicePtr	priv = (CalcompDevicePtr) local->private;
    int			len, loop;
    int			is_absolute;
    int			x, y, buttons, prox;
    DeviceIntPtr	device;
    unsigned char	buffer[BUFFER_SIZE];
  
    DBG(7, ErrorF("xf86CalReadInput BEGIN device=%s fd=%d\n",
       priv->calDevice, local->fd));

    SYSCALL(len = read(local->fd, buffer, sizeof(buffer)));

    if (len <= 0) {
	Error("error reading DrawingBoard device");
	return;
    }

    for(loop=0; loop<len; loop++) {

/* Format of 6 bytes data packet in Calcomp Binary Encoding

       Byte 1
       bit 7  Phasing bit, always 1
       bit 6  Buttons on a 16bit cursor (unsupported)
       bit 5  Button 3
       bit 4  Button 2
       bit 3  Button 1
       bit 2  Button 0 (Stylus tip)  
       bit 1  X15
       bit 0  X14
       
       Byte 2
       bit 7  Always 0
       bits 6-0 = X13 - X7

       Byte 3 
       bit 7  Always 0
       bits 6-0 = X6 - X0

       Byte 4
       bit 7  Always 0
       bits 6-0 = Y17 - Y14

       Byte 5 
       bit 7  Always 0
       bits 6-0 = Y13 - Y7
       
       Byte 6 
       bit 7  Always 0
       bits 6-0 = Y6 - Y0
*/
  
	priv->calData[priv->calIndex++] = buffer[loop];

	if (priv->calIndex == 6) {
/* the packet is complete */
/* reset char count for next read */
	    priv->calIndex = 0;

       if (priv->calData[0] < 128 ) {
                fprintf(stderr,"Tablet out of sync!!!\n");
                 do{
                   priv->calData[0]=priv->calData[1];
                   priv->calData[1]=priv->calData[2]; 
                   priv->calData[2]=priv->calData[3]; 
                   priv->calData[3]=priv->calData[4]; 
                   priv->calData[4]=priv->calData[5]; 
                   priv->calData[5]=buffer[loop++]; 
                   } while (priv->calData[0]<128);
                  fprintf(stderr,"junked 1 byte\n");
                                        }          

    x = (int)priv->calData[2] + ((int)priv->calData[1] << 7) 
                     +( ((int)priv->calData[0] & 0x03) <<14);
    y = (int)priv->calData[5] + ((int)priv->calData[4] << 7);
    y = priv->calMaxY -y;
                                                                                                                       

	    prox = (priv->calData[3] & PROXIMITY_BIT)? 0: 1;

	    buttons = ((priv->calData[0] & BUTTON_BITS) >>2);

		if (buttons ==4 ) buttons=3;
		if (buttons ==8 ) buttons=4;
	    device = local->dev;

	    DBG(6, ErrorF("prox=%s\tx=%d\ty=%d\tbuttons=%d\n",
		   prox ? "true" : "false", x, y, buttons));

	    is_absolute = (priv->flags & ABSOLUTE_FLAG);

/* coordinates are ready we can send events */
	    if (prox) {
		if (!(priv->calOldProximity))
		    xf86PostProximityEvent(device, 1, 0, 2, x, y);

		if ((is_absolute && ((priv->calOldX != x) || (priv->calOldY != y)))
		       || (!is_absolute && (x || y))) {
		    if (is_absolute || priv->calOldProximity) {
			xf86PostMotionEvent(device, is_absolute, 0, 2, x, y);
		    }
		}
		if (priv->calOldButtons != buttons) {
		int	delta;
		int	button;

		    delta = buttons - priv->calOldButtons;
		    button = (delta > 0)? delta: ((delta == 0)?
			   priv->calOldButtons : -delta);

		    if (priv->calOldButtons != buttons) {
			DBG(6, ErrorF("xf86CalReadInput button=%d delta=%d\n", button,
			       delta));

			xf86PostButtonEvent(device, is_absolute, button,
			       (delta > 0), 0, 2, x, y);
		    }
		}
		priv->calOldButtons = buttons;
		priv->calOldX = x;
		priv->calOldY = y;
		priv->calOldProximity = prox;
	    } else { /* !PROXIMITY */
/* Any changes in buttons are ignored when !proximity */
		if (priv->calOldProximity)
		    xf86PostProximityEvent(device, 0, 0, 2, x, y);
		priv->calOldProximity = 0;
	    }
	}
    }
    DBG(7, ErrorF("xf86CalReadInput END   device=0x%x priv=0x%x\n",
	   local->dev, priv));
}

/*
** xf86CalControlProc
** It really does do something.  Honest!
*/
static void
xf86CalControlProc(DeviceIntPtr	device, PtrCtrl *ctrl)

{
    DBG(2, ErrorF("xf86CalControlProc\n"));
}

/*
** xf86CalWriteAndRead
** Write data, and get the response.
*/
static char *
xf86CalWriteAndRead(int fd, char *data, char *buffer, int len, int cr_term)
{
    int err, numread = 0;
    fd_set readfds;
    struct timeval timeout;

    SYSCALL(err = write(fd, data, strlen(data)));
    if (err == -1) {
	Error("Calcomp write");
	return NULL;
    }

    FD_ZERO(&readfds);
    FD_SET(fd, &readfds);
    while (numread < len) {
	timeout.tv_sec = 0;
	timeout.tv_usec = 200000;

	SYSCALL(err = select(FD_SETSIZE, &readfds, NULL, NULL, &timeout));
	if (err == -1) {
	    Error("Calcomp select");
	    return NULL;
	}
	if (!err) {
	    ErrorF("Timeout while reading Calcomp tablet. No tablet connected ???\n");
	    return NULL;
	}

	SYSCALL(err = read(fd, buffer + numread++, 1));
	if (err == -1) {
	    Error("Calcomp read");
	    return NULL;
	}
	if (!err) {
	    --numread;
	    break;
	}
	if (cr_term && buffer[numread - 1] == '\r') {
	    buffer[numread - 1] = 0;
	    break;
	}
    }
    buffer[numread] = 0;
    return buffer;
}

/*
** xf86CalOpen
** Open and initialize the tablet, as well as probe for any needed data.
*/
static Bool
xf86CalOpen(LocalDevicePtr local)
{
    struct termios	termios_tty;
    struct timeval	timeout;
    char		buffer[256];
    int			err;
    CalcompDevicePtr	priv = (CalcompDevicePtr)local->private;

    DBG(1, ErrorF("opening %s\n", priv->calDevice));

    SYSCALL(local->fd = open(priv->calDevice, O_RDWR|O_NDELAY, 0));
    if (local->fd == -1) {
	Error(priv->calDevice);
	return !Success;
    }
    DBG(2, ErrorF("%s opened as fd %d\n", priv->calDevice, local->fd));

#ifdef POSIX_TTY
    err = tcgetattr(local->fd, &termios_tty);
    if (err == -1) {
	Error("Calcomp tcgetattr");
	return !Success;
    }
    termios_tty.c_iflag = IXOFF;
    termios_tty.c_cflag = B9600|CS8|CREAD|CLOCAL|HUPCL;
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
	Error("Calcomp tcsetattr TCSANOW");
	return !Success;
    }
#else
/*    Code for someone else to write to handle OSs without POSIX tty functions*/

    SYSCALL(err = write(local->fd, DB_COMM_SETUP, strlen(DB_COMM_SETUP)));
        if (err == -1) {
                Error("Calcomp write (init comm)");
                        return !Success;
                            }
                            
#endif

/* See if we can talk to the tablet - inquire model string */
    DBG(2, ErrorF("reading firmware ID\n"));
    if (!xf86CalWriteAndRead(local->fd, DB_FIRMID, buffer, 255, 1)) {
      ErrorF("Failed to report firmware ID - old WIZ or 2300 series ?\n");
    }else {
    DBG(2, ErrorF("%s\n", buffer));

    if (xf86Verbose)
	ErrorF("%s Calcomp firmware ID : %s\n", XCONFIG_PROBED, buffer);
    }
    tcflush(local->fd, TCIFLUSH);

/* Try to setup basic operating parameters (these are the hardware defaults)*/
    SYSCALL(err = write(local->fd, DB_LINEFEED, 5));
    SYSCALL(err = write(local->fd, DB_FORMAT, 6));
    if (err == -1) {
            Error("Calcomp write (set mode)");
                    return !Success;
                        }
                        
    SYSCALL(err = write(local->fd, DB_X_INCR, 6));
    SYSCALL(err = write(local->fd, DB_Y_INCR, 6));
    if (err == -1) {
        Error("Calcomp write (reset)");
             return !Success;
                 }
    SYSCALL(err = write(local->fd, DB_1000LPI, 11));
        if (err == -1) {
                Error("Calcomp write (set resolution");
                       return !Success;
                       }
    SYSCALL(err = write(local->fd, DB_ABSOLUTE, 5));
        if (err == -1) {
                Error("Calcomp write (set incremental run mode");
                       return !Success;
                       }
                               
                                      
/* wait 200 mSecs, just in case */
    timeout.tv_sec = 0;
    timeout.tv_usec = 200000;
    SYSCALL(err = select(0, NULL, NULL, NULL, &timeout));
    if (err == -1) {
	Error("Calcomp select");
	return !Success;
    }


/* Clear any pending input */
    tcflush(local->fd, TCIFLUSH);
  
    DBG(2, ErrorF("reading max coordinates\n"));
    memset(buffer,0,30);
    if (!xf86CalWriteAndRead(local->fd, DB_CONFIG, buffer, 6, 1))
	return !Success;
    priv->calMaxX = (int)buffer[2] + ((int)buffer[1] << 7) 
                             + (((int)buffer[0]&0x03)<<14);
    priv->calMaxY = (int)buffer[5] + ((int)buffer[4] << 7);

    if (xf86Verbose)
	ErrorF("%s Calcomp tablet size is %d.%1dinx%d.%1din, %dx%d "
	       "lines of resolution\n", XCONFIG_PROBED, 
	       priv->calMaxX / 1000, (priv->calMaxX / 100) % 10,
	       priv->calMaxY / 1000, (priv->calMaxY / 100) % 10,
	       priv->calMaxX, priv->calMaxY);

if (priv->calXSize == -1 || priv->calYSize == -1 ) {
		priv->calXSize = priv->calMaxX;
		priv->calYSize = priv->calMaxY;
} 
 
                        
    if (priv->calXOffset > 0 && priv->calYOffset > 0) {
	if (priv->calXSize * 100 < priv->calMaxX - priv->calXOffset &&
	    priv->calYSize * 100 < priv->calMaxY - priv->calYOffset) {
	    priv->calXOffset *= 100;
	    priv->calYOffset *= 100;
	} else {
	    ErrorF("%s Calcomp offset sets active area off tablet, "
		   "centering\n", XCONFIG_PROBED);
	    priv->calXOffset = (priv->calMaxX - priv->calXSize) / 2;
	    priv->calYOffset = (priv->calMaxY - priv->calYSize) / 2;
	}
    } else {
	priv->calXOffset = (priv->calMaxX - priv->calXSize) / 2;
	priv->calYOffset = (priv->calMaxY - priv->calYSize) / 2;
    }

    if (priv->calInc > 95)
	priv->calInc = 95;
    if (priv->calInc < 1) {
	if (priv->calXSize / screenInfo.screens[0]->width <
	       priv->calYSize / screenInfo.screens[0]->height)
	    priv->calInc = priv->calXSize / screenInfo.screens[0]->width;
	else
	    priv->calInc = priv->calYSize / screenInfo.screens[0]->height;

	if (priv->calInc < 1)
	    priv->calInc = 1;
	if (xf86Verbose)
	    ErrorF("%s Using increment value of %d\n", XCONFIG_PROBED,
		   priv->calInc);
    }


	sprintf(buffer, "\033%%X%d\r",priv->calInc);
	SYSCALL(err = write (local->fd,buffer,strlen(buffer)));
	if (err == -1) {
	Error ("Calcomp write (set X increment)");
	  return !Success;
	  }
	sprintf(buffer, "\033%%Y%d\r",priv->calInc);
	SYSCALL(err = write (local->fd,buffer,strlen(buffer)));
	if (err == -1) {
	Error ("Calcomp write (set Y increment)");
	  return !Success;
	  }
	if (priv->flags & ABSOLUTE) {
	SYSCALL (err=write (local->fd,DB_ABSOLUTE,5));
		if (err == -1) {
	Error ("Calcomp write (set absolute mode)");
	  return !Success;
	  }
	} else {
	SYSCALL (err=write (local->fd,DB_RELATIVE,5));
		if (err == -1) {
	Error ("Calcomp write (set relative mode)");
	  return !Success;
	  }
	}		
    return Success;
}

/*
** xf86CalOpenDevice
** Opens and initializes the device driver stuff or sumpthin.
*/
static int
xf86CalOpenDevice(DeviceIntPtr pCal)
{
    LocalDevicePtr	local = (LocalDevicePtr)pCal->public.devicePrivate;
    CalcompDevicePtr	priv = (CalcompDevicePtr)PRIVATE(pCal);

    if (xf86CalOpen(local) != Success) {
	if (local->fd >= 0) {
	    SYSCALL(close(local->fd));
	}
	local->fd = -1;
    }

/* Try to cope with a tablet that was switched on *after* the 
X server started - assume 12in x 12in */
if (priv->calMaxX <=0 ) priv->calMaxX=12000;
if (priv->calMaxY<=0) priv->calMaxY=12000;



/* Set the real values */
    InitValuatorAxisStruct(pCal,
			   0,
			   0, /* min val */
			   priv->calMaxX, /* max val */
			   39400, /* resolution */
			   0, /* min_res */
			   39400); /* max_res */
    InitValuatorAxisStruct(pCal,
			   1,
			   0, /* min val */
			   priv->calMaxY, /* max val */
			   39400, /* resolution */
			   0, /* min_res */
			   39400); /* max_res */
    return (local->fd != -1);
}

/*
** xf86CalProc
** Handle requests to do stuff to the driver.
*/
static int
xf86CalProc(DeviceIntPtr pCal, int what)
{
    CARD8		map[25];
    int			nbaxes;
    int			nbbuttons;
    int			loop;
    LocalDevicePtr	local = (LocalDevicePtr)pCal->public.devicePrivate;
    CalcompDevicePtr	priv = (CalcompDevicePtr)PRIVATE(pCal);

    DBG(2, ErrorF("BEGIN xf86CalProc dev=0x%x priv=0x%x what=%d\n", pCal, priv, what));

    switch (what) {
	case DEVICE_INIT:
	    DBG(1, ErrorF("xf86CalProc pCal=0x%x what=INIT\n", pCal));

	    nbaxes = 2;			/* X, Y */	    
	    
	    /* Stylus has 3 buttons, treated like 4-button cursor */
	    /* 16-button cursor is not yet supported */
	    
	    nbbuttons = (priv->flags & STYLUS_FLAG)? 4: 16; 

	    for(loop=1; loop<=nbbuttons; loop++) map[loop] = loop;

	    if (InitButtonClassDeviceStruct(pCal,
					    nbbuttons,
					    map) == FALSE) {
		ErrorF("unable to allocate Button class device\n");
		return !Success;
	    }

	    if (InitFocusClassDeviceStruct(pCal) == FALSE) {
		ErrorF("unable to init Focus class device\n");
		return !Success;
	    }

	    if (InitPtrFeedbackClassDeviceStruct(pCal,
		   xf86CalControlProc) == FALSE) {
		ErrorF("unable to init ptr feedback\n");
		return !Success;
	    }

	    if (InitProximityClassDeviceStruct(pCal) == FALSE) {
		ErrorF("unable to init proximity class device\n"); 
		return !Success;
	    }

	    if (InitValuatorClassDeviceStruct(pCal,
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

	    AssignTypeAndName(pCal, local->atom, local->name);
/* open the device to gather informations */
	    xf86CalOpenDevice(pCal);
	    break;

	case DEVICE_ON:
	    DBG(1, ErrorF("xf86CalProc pCal=0x%x what=ON\n", pCal));

	    if ((local->fd < 0) && (!xf86CalOpenDevice(pCal))) {
		return !Success;
	    }
	    SYSCALL(write(local->fd, DB_PROMPT, strlen(DB_PROMPT)));
	    AddEnabledDevice(local->fd);
	    pCal->public.on = TRUE;
	    break;

	case DEVICE_OFF:
	    DBG(1, ErrorF("xf86CalProc  pCal=0x%x what=%s\n", pCal,
		   (what == DEVICE_CLOSE) ? "CLOSE" : "OFF"));
	    if (local->fd >= 0)
		RemoveEnabledDevice(local->fd);
	    pCal->public.on = FALSE;
	    break;

	case DEVICE_CLOSE:
	    DBG(1, ErrorF("xf86CalProc  pCal=0x%x what=%s\n", pCal,
		   (what == DEVICE_CLOSE) ? "CLOSE" : "OFF"));
	    SYSCALL(close(local->fd));
	    local->fd = -1;
	    break;

	default:
	    ErrorF("unsupported mode=%d\n", what);
	    return !Success;
	    break;
    }
    DBG(2, ErrorF("END   xf86CalProc Success what=%d dev=0x%x priv=0x%x\n",
	   what, pCal, priv));
    return Success;
}

/*
** xf86CalClose
** It...  Uh...  Closes the physical device?
*/
static void
xf86CalClose(LocalDevicePtr local)
{
    if (local->fd >= 0) {
	SYSCALL(close(local->fd));
    }
    local->fd = -1;
}

/*
** xf86CalChangeControl
** When I figure out what it does, it will do it.
*/
static int
xf86CalChangeControl(LocalDevicePtr local, xDeviceCtl *control)
{
    xDeviceResolutionCtl	*res;

    res = (xDeviceResolutionCtl *)control;
	
    if ((control->control != DEVICE_RESOLUTION) ||
	   (res->num_valuators < 1))
	return (BadMatch);

    return(Success);
}

/*
** xf86CalSwitchMode
** Switches the mode. Only absolute mode has been tested and used so far...
*/
static int
xf86CalSwitchMode(ClientPtr client, DeviceIntPtr dev, int mode)
{
    LocalDevicePtr	local = (LocalDevicePtr)dev->public.devicePrivate;
    CalcompDevicePtr	priv = (CalcompDevicePtr)(local->private);
    char*		newmode="";

    DBG(3, ErrorF("xf86CalSwitchMode dev=0x%x mode=%d\n", dev, mode));


    switch(mode) {
	case Absolute:
	    priv->flags |= ABSOLUTE_FLAG;
	    strcpy(newmode, DB_ABSOLUTE);
	    break;

	case Relative:
	    priv->flags &= ~ABSOLUTE_FLAG;
	    strcpy(newmode,DB_RELATIVE);
	    break;

	default:
	    DBG(1, ErrorF("xf86CalSwitchMode dev=0x%x invalid mode=%d\n",
		   dev, mode));
	    return BadMatch;
    }
    SYSCALL(write(local->fd, newmode, sizeof(newmode)));
    return Success;
}

/*
** xf86CalAllocate
** Allocates the device structures for the Calcomp DrawingBoard.
*/
static LocalDevicePtr
xf86CalAllocate()
{
    LocalDevicePtr	local = (LocalDevicePtr)xalloc(sizeof(LocalDeviceRec));
    CalcompDevicePtr	priv = (CalcompDevicePtr)xalloc(sizeof(CalcompDeviceRec));
#if defined (sun) && !defined(i386)
    char		*dev_name = getenv("CALCOMP_DEV");
#endif

    local->name = XI_NAME;
    local->type_name = "Calcomp DrawingBoard";
    local->flags = 0; /*XI86_NO_OPEN_ON_INIT;*/
#if !defined(sun) || defined(i386)
    local->device_config = xf86CalConfig;
#endif
    local->device_control = xf86CalProc;
    local->read_input = xf86CalReadInput;
    local->control_proc = xf86CalChangeControl;
    local->close_proc = xf86CalClose;
    local->switch_mode = xf86CalSwitchMode;
    local->conversion_proc = xf86CalConvert;
    local->reverse_conversion_proc = xf86CalReverseConvert;
    local->fd = -1;
    local->atom = 0;
    local->dev = NULL;
    local->private = priv;
    local->private_flags = 0;
    local->history_size  = 0;

#if defined(sun) && !defined(i386)
    if (def_name) {
	priv->calDevice = (char *)xalloc(strlen(dev_name) + 1);
	strcpy(priv->calDevice, device_name);
	ErrorF("xf86CalOpen port changed to '%s'\n", priv->calDevice);
    } else {
	priv->calDevice = "";
    }
#else
    priv->calDevice = "";         /* device file name */
#endif
    priv->calInc = -1;            /* re-transmit position on increment */
    priv->calOldX = -1;           /* previous X position */
    priv->calOldY = -1;           /* previous Y position */
    priv->calOldProximity = 0;    /* previous proximity */
    priv->calOldButtons = 0;      /* previous buttons state */
    priv->calMaxX = -1;           /* max X value */
    priv->calMaxY = -1;           /* max Y value */
    priv->calXSize = -1;	  /* active area X */
    priv->calXOffset = -1;	  /* active area X offset */
    priv->calYSize = -1;	  /* active area Y */
    priv->calYOffset = -1;	  /* active area U offset */
    priv->flags = 0;              /* various flags */
    priv->calIndex = 0;           /* number of bytes read */

    return local;
}


/*
** Calcomp device association
** Device section name and allocation function.
*/
DeviceAssocRec calcomp_assoc =
{
  CALCOMP_SECTION_NAME,           /* config_section_name */
  xf86CalAllocate               /* device_allocate */
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
init_xf86Calcomp(unsigned long server_version)
#endif
{
    xf86AddDeviceAssoc(&calcomp_assoc);

    if (server_version != XF86_VERSION_CURRENT) {
	ErrorF("Warning: Calcomp DrawingBoard module compiled for version%s\n",
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
XF86ModuleVersionInfo xf86CalcompVersion = {
    "xf86Calcomp",
    MODULEVENDORSTRING,
    MODINFOSTRING1,
    MODINFOSTRING2,
    XF86_VERSION_CURRENT,
    0x00010000,
    {0,0,0,0}
};

void
xf86CalcompModuleInit(data, magic)
    pointer *data;
    INT32 *magic;
{
    static int cnt = 0;

    switch (cnt) {
      case 0:
      *magic = MAGIC_VERSION;
      *data = &xf86CalcompVersion;
      cnt++;
      break;
      
      case 1:
      *magic = MAGIC_ADD_XINPUT_DEVICE;
      *data = &calcomp_assoc;
      cnt++;
      break;

      default:
      *magic = MAGIC_DONE;
      *data = NULL;
      break;
    } 
}
#endif
/* end of xf86Calcomp.c */

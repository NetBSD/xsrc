/* $XConsortium$ */
/*
 * Copyright 1998 by Francois-Regis Colin, <fcolin@cenatls.cena.dgac.fr> and
 *                  Frederic Lepied, <Fredric.Lepied@sugix.frmug.org>. France.
 *                                                                            
 * Permission  to use, copy,  modify, distribute, and  sell this software
 * and its  documentation for any purpose  is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and that
 * both  that  copyright notice   and this  permission notice  appear  in
 * supporting documentation, and  that the names of Francois-Regis  Colin
 * and Frederic Lepied not be used in advertising or publicity pertaining
 * to distribution of     the software without specific,  written   prior
 * permission.   Francois-Regis   Colin   and  Frederic  Lepied make   no
 * representations  about the suitability   of   this software for    any
 * purpose.  It is provided "as is" without express or implied warranty.
 *                                                                            
 * FRANCOIS-REGIS COLIN AND FREDERIC LEPIED  DISCLAIM ALL WARRANTIES WITH
 * REGARD  TO THIS   SOFTWARE,  INCLUDING   ALL IMPLIED  WARRANTIES    OF
 * MERCHANTABILITY  AND  FITNESS, IN NO EVENT    SHALL FREDERIC LEPIED BE
 * LIABLE  FOR  ANY SPECIAL,  INDIRECT OR   CONSEQUENTIAL DAMAGES  OR ANY
 * DAMAGES  WHATSOEVER   RESULTING FROM LOSS  OF   USE, DATA  OR PROFITS,
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
 * ARISING OUT  OF OR IN CONNECTION  WITH THE USE  OR PERFORMANCE OF THIS
 * SOFTWARE.
 *
 */

/* $XFree86: xc/programs/Xserver/hw/xfree86/common/xf86Dial.c,v 1.1.2.5 1998/12/20 01:54:04 dawes Exp $ */

/*
 * This driver handles SGI dial and button boxes protocol.
 */

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
#include "keysym.h"

#if defined(sun) && !defined(i386)
#define POSIX_TTY
#include <errno.h>
#include <termio.h>
#include <fcntl.h>
#include <ctype.h>
#include <stdio.h>

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

/******************************************************************************
 * debugging macro
 *****************************************************************************/
#ifdef DBG
#undef DBG
#endif
#ifdef DEBUG
#undef DEBUG
#endif

static int      debug_level = 0;
#define DEBUG 1
#if DEBUG
#define DBG(lvl, f) {if ((lvl) <= debug_level) f;}
#else
#define DBG(lvl, f)
#endif

#define	DIAL_NUM_BUTTONS		32
#define	DIAL_NUM_VALUATORS		8

#define	DIAL_VALUATOR_RESOLUTION	200
#define	DIAL_VALUATOR_MIN_FILTER	0
#define	DIAL_VALUATOR_MAX_FILTER	7

#define	DIAL_NUM_LEDS			32

	/* dial parser state machine states */
#define DIAL_WHICH_DEVICE       0
#define DIAL_VALUE_HIGH         1
#define DIAL_VALUE_LOW          2
#define DIAL_N_STATES           3

	/* dial/button box commands */
#define DIAL_INITIALIZE                 0x20
#define DIAL_SET_LEDS                   0x75
#define DIAL_SET_TEXT                   0x61
#define DIAL_SET_AUTO_DIALS             0x50
#define DIAL_SET_AUTO_DELTA_DIALS       0x51
#define DIAL_SET_FILTER			0x53
#define DIAL_SET_BUTTONS_MOM_TYPE       0x71
#define DIAL_SET_AUTO_MOM_BUTTONS       0x73
#define DIAL_SET_ALL_LEDS		0x4b
#define DIAL_CLEAR_ALL_LEDS		0x4c

	/* dial/button box replies and events */
#define DIAL_INITIALIZED        0x20
#define DIAL_BASE               0x30
#define DIAL_DELTA_BASE         0x40
#define DIAL_PRESS_BASE         0xc0
#define DIAL_RELEASE_BASE       0xe0

	/* macros to determine replie type */
#define IS_DIAL_EVENT(ch)       (((ch)>=DIAL_BASE)&&((ch)<DIAL_BASE+DIAL_NUM_VALUATORS))
#define IS_KEY_PRESS(ch)        (((ch)>=DIAL_PRESS_BASE)&&((ch)<DIAL_PRESS_BASE+DIAL_NUM_BUTTONS))
#define IS_KEY_RELEASE(ch)      (((ch)>=DIAL_RELEASE_BASE)&&((ch)<DIAL_RELEASE_BASE+DIAL_NUM_BUTTONS))
#define IS_INIT_EVENT(ch)       ((ch)==DIAL_INITIALIZED)

#define SYSCALL(call) while(((call) == -1) && (errno == EINTR))
#define CHECKEDWRITE(fd, buff, len) SYSCALL(write((fd), (buff), (len)))

/* mask manipulation */
#define NBITS 8

#define SET(n, p)   ((p)[(n)/NBITS] |= ((unsigned)1 << ((n) % NBITS)))

#define CLR(n, p)   ((p)[(n)/NBITS] &= ~((unsigned)1 << ((n) % NBITS)))

#define ISSET(n, p) ((p)[(n)/NBITS] & ((unsigned)1 << ((n) % NBITS)))

/* various flags */
#define ABSOLUTE_FLAG		1
#define BUFFER_SIZE		256
#define SPEED			B9600

typedef struct _DialRec 
{
  char		*port;	        /* device file name */
  int		flags;
  
  /* Button and valuators states */
  unsigned char	vactive[DIAL_NUM_VALUATORS];
  int		hwvstate[DIAL_NUM_VALUATORS];
  unsigned char	bactive[DIAL_NUM_BUTTONS];
  unsigned char	bstate[DIAL_NUM_BUTTONS];
  unsigned char	ledstate[DIAL_NUM_LEDS];

  /* state machine stuff */
  char		state;
  char		which;
  short		value;
  void		*checkinit_id;

  /* Misc. Std. Driver stuff */
  unsigned char	initialized;	/* true if data structures are set */
				/* and init sequence has been sent to */
				/* device */

  unsigned char ready;		/* true if we've received confirmation */
				/* of the init sequence from the device */
  
  unsigned char errorCount;	/* count of bad messages for recovery */
} DialRec, *DialPtr;

/******************************************************************************
 * configuration stuff
 *****************************************************************************/
#define PORT		1
#define DEVICENAME	2
#define THE_MODE	3
#define DEBUG_LEVEL     4
#define HISTORY_SIZE	5
#define ALWAYS_CORE	6

#define SECTION_NAME "dialbox"

#if !defined(sun) || defined(i386)
static SymTabRec DialTab[] = {
  { ENDSUBSECTION,	"endsubsection" },
  { PORT,		"port" },
  { DEVICENAME,		"devicename" },
  { THE_MODE,		"mode" },
  { DEBUG_LEVEL,	"debuglevel" },
  { HISTORY_SIZE,	"historysize" },
  { ALWAYS_CORE,	"alwayscore" },
  { -1,			"" }
};

#define RELATIVE	1
#define ABSOLUTE	2

static SymTabRec ModeTabRec[] = {
  { RELATIVE,	"relative" },
  { ABSOLUTE,	"absolute" },
  { -1,		"" }
};
  
#endif

/******************************************************************************
 * external declarations
 *****************************************************************************/
#if defined(sun) && !defined(i386)
#define ENQUEUE suneqEnqueue
#else
#define ENQUEUE xf86eqEnqueue

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

#if NeedFunctionPrototypes
static LocalDevicePtr xf86WcmAllocate(void);
static Bool xf86DialOpen(LocalDevicePtr	local);
#endif

/******************************************************************************
 * dial box communication
 *****************************************************************************/
static int
dial_enable_button(int		fd,
		   DialRec	*dial,
		   int		btns,
		   int		value)
{
  int i;
  unsigned char dcmd[5];
  
  unsigned char mask[4] ={ 0,0,0,0 };
  
  dial->bactive[btns] = value;

  for (i=0;i<DIAL_NUM_BUTTONS;i++) {
    if (dial->bactive[i]) 
      SET(i, mask);
  }
  
  dcmd[0] = DIAL_SET_AUTO_MOM_BUTTONS;
  dcmd[1] = mask[0];
  dcmd[2] = mask[1];
  dcmd[3] = mask[2];
  dcmd[4] = mask[3];
  
  CHECKEDWRITE(fd, dcmd, sizeof(dcmd));
  
  dcmd[0] = DIAL_SET_BUTTONS_MOM_TYPE;
  dcmd[1] = mask[0];
  dcmd[2] = mask[1];
  dcmd[3] = mask[2];
  dcmd[4] = mask[3];
  
  CHECKEDWRITE(fd, dcmd, sizeof(dcmd));
  
  return 1;
}

static int
dial_switch_leds(int		fd,
		 DialRec	*dial)
{
  char	cmd = DIAL_SET_ALL_LEDS;
  
  CHECKEDWRITE(fd, &cmd, 1);
}

static int
dial_clear_leds(int		fd,
		DialRec		*dial)
{
  char	cmd = DIAL_CLEAR_ALL_LEDS;
  
  CHECKEDWRITE(fd, &cmd, 1);
}

static int
dial_enable_valuator(int	fd,
		     DialRec	*dial,
		     int	val,
		     int	value)
{
  int		i;
  unsigned char dcmd[3];
  unsigned char mask[2] = {0,0};

  dial->vactive[val] = value;
  
  
  for (i=0;i<DIAL_NUM_VALUATORS;i++) {
    if (dial->vactive[i])
      SET(i, mask);
  }
  dcmd[0] = DIAL_SET_AUTO_DIALS;
#ifdef NOTDEF
  dcmd[1] = mask[0];
  dcmd[2] = mask[1];
#else
  dcmd[1] = 0xff;
  dcmd[2] = 0xff;
#endif
  CHECKEDWRITE(fd, dcmd, sizeof(dcmd));

  return 1;
}

static int
dial_set_resolution(int		fd,
		    DialRec	*dial,
		    int		valNum,
		    int		valFilter )
{
  char dcmd[3];
  unsigned char filter = valFilter;
  

  dcmd[0] = DIAL_SET_FILTER;
  dcmd[1] = valNum;
  dcmd[2] = filter;
	
  CHECKEDWRITE(fd, dcmd, sizeof(dcmd));
  
  return 1;
}

static int
dial_set_led(int	fd,
	     DialRec	*dial,
	     int	led,
	     int	value)
{
  int i;
  char dcmd[5];
  unsigned char mask[4] ={ 0,0,0,0 };

  dial->ledstate[led] = value;

  for (i=0;i<DIAL_NUM_LEDS;i++) {
    if (dial->ledstate[i])
      SET(i, mask);
  }
  dcmd[0] = DIAL_SET_LEDS;
  dcmd[1] = mask[0];
  dcmd[2] = mask[1];
  dcmd[3] = mask[2];
  dcmd[4] = mask[3];
  CHECKEDWRITE(fd, dcmd, sizeof(dcmd));
  
  return 1;
}

static int
dial_set_led_mask(int		fd,
		  DialRec	*dial,
		  unsigned int	mask)
{
  unsigned char dcmd[5];

  dcmd[0] = DIAL_SET_LEDS;
  dcmd[1] = (unsigned char)  (0x000000ff & mask);
  dcmd[2] = (unsigned char) ((0x0000ff00 & mask) >> 8);
  dcmd[3] = (unsigned char) ((0x00ff0000 & mask) >> 16);
  dcmd[4] = (unsigned char) ((0xff000000 & mask) >> 24);

  CHECKEDWRITE(fd, dcmd, sizeof(dcmd));
  
  return 1;
}

static  void
dial_setup(int		fd,
	   DialRec	*dial)
{
  int		i;

  dial->ready=1;

  dial_switch_leds(fd, dial);
  sleep(1);
  dial_clear_leds(fd, dial);
	
  for (i=0;i<DIAL_NUM_LEDS;i++) {
    dial_set_led(fd, dial,i,dial->ledstate[i]);
  }

  for (i=0;i<DIAL_NUM_BUTTONS;i++) {
    dial->bactive[i] = 1;
    dial_enable_button(fd, dial, i, dial->bactive[i]);
  }
  
  
  for (i=0;i<DIAL_NUM_VALUATORS;i++) {
    dial_set_resolution(fd, dial, i, 0);
    dial_enable_valuator(fd, dial, i, dial->vactive[i]);
  }
  
  dial->errorCount= 0;
  return;
}

#if !defined(sun) || defined(i386)
/*
 ***************************************************************************
 *
 * xf86DialConfig --
 *	Configure the device.
 *
 ***************************************************************************
 */
static Bool
xf86DialConfig(LocalDevicePtr    *array,
	       int               inx,
	       int               max,
	       LexPtr            val)
{
    LocalDevicePtr      dev = array[inx];
    DialPtr		priv = (DialPtr)(dev->private);
    int			token;
    int			mtoken;
    
    DBG(1, ErrorF("xf86DialConfig\n"));

    while ((token = xf86GetToken(DialTab)) != ENDSUBSECTION) {
	switch(token) {
	case PORT:
	  if (xf86GetToken(NULL) != STRING)
	    xf86ConfigError("Dial input port expected");
	  priv->port = strdup(val->str);	
	  if (xf86Verbose)
	    ErrorF("%s Dial input port: %s\n",
		   XCONFIG_GIVEN, priv->port);
	  break;

	case DEVICENAME:
	    if (xf86GetToken(NULL) != STRING)
		xf86ConfigError("Option string expected");
	    dev->name = strdup(val->str);
	    if (xf86Verbose)
		ErrorF("%s Dial X device name is %s\n", XCONFIG_GIVEN,
		       dev->name);
	    break;	    
	    
	case THE_MODE:
	    mtoken = xf86GetToken(ModeTabRec);
	    if ((mtoken == EOF) || (mtoken == STRING) || (mtoken == NUMBER)) 
		xf86ConfigError("Mode type token expected");
	    else {
		switch (mtoken) {
		case ABSOLUTE:
		    priv->flags = priv->flags | ABSOLUTE_FLAG;
		    break;
		case RELATIVE:
		    priv->flags = priv->flags & ~ABSOLUTE_FLAG; 
		    break;
		default:
		    xf86ConfigError("Illegal Mode type");
		    break;
		}
	    }
	    break;
	    
	case DEBUG_LEVEL:
	    if (xf86GetToken(NULL) != NUMBER)
		xf86ConfigError("Option number expected");
	    debug_level = val->num;
	    if (xf86Verbose) {
#if DEBUG
		ErrorF("%s Dial debug level sets to %d\n", XCONFIG_GIVEN,
		       debug_level);      
#else
		ErrorF("%s Dial debug level not sets to %d because"
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
		ErrorF("%s Dial Motion history size is %d\n", XCONFIG_GIVEN,
		       dev->history_size);      
	    break;

	case ALWAYS_CORE:
	    xf86AlwaysCore(dev, TRUE);
	    if (xf86Verbose)
		ErrorF("%s Dial device always stays core pointer\n",
		       XCONFIG_GIVEN);
	    break;

	case EOF:
	    FatalError("Unexpected EOF (missing EndSubSection)");
	    break;
	    
	default:
	    xf86ConfigError("Dial subsection keyword expected");
	    break;
	}
    }
    
    DBG(1, ErrorF("xf86DialConfig name=%s\n", dev->name));
    
    return Success;
}
#endif

/*
 ***************************************************************************
 *
 * wait_for_fd --
 *
 *	Wait one second that the file descriptor becomes readable.
 *
 ***************************************************************************
 */
static int
wait_for_fd(int	fd)
{
    int			err;
    fd_set		readfds;
    struct timeval	timeout;
    
    FD_ZERO(&readfds);
    FD_SET(fd, &readfds);
    
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;
    SYSCALL(err = select(FD_SETSIZE, &readfds, NULL, NULL, &timeout));

    return err;
}

/*
 ***************************************************************************
 *
 * xf86DialConvert --
 *	Convert valuators to X and Y.
 *
 ***************************************************************************
 */
static Bool
xf86DialConvert(LocalDevicePtr	local,
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
    DialPtr	priv = (DialPtr) local->private;

    if (first != 0 || num == 1)
      return FALSE;

    *x = ((v0 + 32768) * screenInfo.screens[0]->width) / 65535 ;
    *y = ((v1 + 32768) * screenInfo.screens[0]->height) / 65535 ;

    DBG(5, ErrorF("Dial converted v0=%d v1=%d to x=%d y=%d\n",
		  v0, v1, *x, *y));
    
    return TRUE;
}

/*
 ***************************************************************************
 *
 * xf86DialReverseConvert --
 *	Convert X and Y to valuators.
 *
 ***************************************************************************
 */
static Bool
xf86DialReverseConvert(LocalDevicePtr	local,
		       int		x,
		       int		y,
		       int		*valuators)
{
    DialPtr	priv = (DialPtr) local->private;

    valuators[0] = ((x * 65535) / screenInfo.screens[0]->width) - 32768;
    valuators[1] = ((y * 65535) / screenInfo.screens[0]->height) - 32768;

    DBG(5, ErrorF("Dial converted x=%d y=%d to v0=%d v1=%d\n", x, y,
		  valuators[0], valuators[1]));
    
    return TRUE;
}

/*
 ***************************************************************************
 *
 * xf86DialReadInput --
 *	Read the new events from the device, and enqueue them.
 *
 ***************************************************************************
 */
static void
xf86DialReadInput(LocalDevicePtr         local)
{
    DialPtr		dial = (DialPtr) local->private;
    int			len, loop, idx;
    int			is_stylus, is_button, is_proximity;
    int			x, y, z, buttons;
    int			*px, *py, *pz, *pbuttons, *pprox;
    unsigned char	buffer[BUFFER_SIZE];
    unsigned char	*str, ch;
    int			is_absolute = (dial->flags & ABSOLUTE_FLAG);

    DBG(7, ErrorF("xf86DialReadInput BEGIN fd=%d\n",
		  local->fd));

    SYSCALL(len = read(local->fd, buffer, sizeof(buffer)));

    if (len <= 0) {
	ErrorF("Error reading dial device : %s\n", strerror(errno));
	return;
    } else {
	DBG(10, ErrorF("xf86DialReadInput read %d bytes\n", len));
    }

    str = buffer;
    while (len > 0) {
      ch = *str++;
      if ((dial->state != 0) || IS_DIAL_EVENT(ch)) {
	switch (dial->state) {
	case DIAL_WHICH_DEVICE:
	  dial->which = ch-DIAL_BASE;
	  dial->state++;
	  break;
	case DIAL_VALUE_HIGH:
	  dial->value = (ch<<8);
	  dial->state++;
	  break;
	case DIAL_VALUE_LOW:
	  {
	    int delta;
	    int v[8] = {0, 0, 0, 0, 0, 0, 0, 0};
	    
	    dial->value |= ch;
	    
	    delta = dial->value - dial->hwvstate[(int)dial->which];
	    dial->hwvstate[(int)dial->which] = dial->value;

	    if (is_absolute) {
	      xf86PostMotionEvent(local->dev, 
				  1,	/* is_absolute */
				  0, 8,
				  dial->hwvstate[0],
				  dial->hwvstate[1],
				  dial->hwvstate[2],
				  dial->hwvstate[3],
				  dial->hwvstate[4],
				  dial->hwvstate[5],
				  dial->hwvstate[6],
				  dial->hwvstate[7]);
	    } else {
		v[(int)dial->which] = delta;

		xf86PostMotionEvent(local->dev, 
				    0,	/* is_absolute */
				    0, 8,
				    v[0],
				    v[1],
				    v[2],
				    v[3],
				    v[4],
				    v[5],
				    v[6],
				    v[7]);
	    }
	    dial->state = 0;
	    break;
	  }
	default:
	  ErrorF("Impossible state %d in dial_intr.\n", dial->state);
	  dial->state = 0;
	  break;
	}
	dial->errorCount = 0;
      }
      else if (IS_KEY_PRESS(ch)) {
	unsigned char btnNum = ((ch - DIAL_PRESS_BASE)^0x18) + 1;

	xf86PostButtonEvent(local->dev, 
			    is_absolute, /* is_absolute */
			    btnNum, 1,
			    0, 8,
			    dial->hwvstate[0],
			    dial->hwvstate[1],
			    dial->hwvstate[2],
			    dial->hwvstate[3],
			    dial->hwvstate[4],
			    dial->hwvstate[5],
			    dial->hwvstate[6],
			    dial->hwvstate[7]);
	dial->errorCount = 0;
      }
      else if (IS_KEY_RELEASE(ch)) {
	unsigned char btnNum = ((ch - DIAL_RELEASE_BASE)^0x18) + 1;

	xf86PostButtonEvent(local->dev, 
			    is_absolute, /* is_absolute */
			    btnNum, 0,
			    0, 8,
			    dial->hwvstate[0],
			    dial->hwvstate[1],
			    dial->hwvstate[2],
			    dial->hwvstate[3],
			    dial->hwvstate[4],
			    dial->hwvstate[5],
			    dial->hwvstate[6],
			    dial->hwvstate[7]);
	dial->errorCount = 0;
      }
      else if (ch == DIAL_INITIALIZED) {
	
	fprintf(stderr,"Dial box  initialized OK.\n");
	dial_setup(local->fd, dial);
	dial->errorCount = 0;
      }
      else {
	ErrorF("unexpected char %d from dial\n", ch);
	/* try to reinitialize */
	if (dial->errorCount++>5) {
	  ErrorF("Reinitializing dial box\n");
	  xf86DialOpen(local);
	}
      }
      len--;
    }
    
    DBG(7, ErrorF("xf86DialReadInput END   local=0x%x priv=0x%x\n",
		  local, dial));
}

/*
 ***************************************************************************
 *
 * xf86DialControlProc --
 *
 ***************************************************************************
 */
static void
xf86DialControlProc(DeviceIntPtr	device,
		   PtrCtrl	*ctrl)
{
  DBG(2, ErrorF("xf86DialControlProc\n"));
}

/*
 ***************************************************************************
 *
 * xf86DialLedControlProc --
 *
 ***************************************************************************
 */
static void
xf86DialLedControlProc(DeviceIntPtr	pDial,
		       LedCtrl		*ctrl)
{
  LocalDevicePtr	local = (LocalDevicePtr)pDial->public.devicePrivate;
  DialPtr		priv = (DialPtr)PRIVATE(pDial);

  DBG(2, ErrorF("xf86DialLedControlProc fd=%d led_values=0x%x led_mask=0x%x\n",
		local->fd, ctrl->led_values, ctrl->led_mask));
  
  if (local->fd >= 0) {
    dial_set_led_mask(local->fd, priv, ctrl->led_values & ctrl->led_mask);
  }
}

/*
 ***************************************************************************
 *
 * xf86DialOpen --
 *
 ***************************************************************************
 */
static Bool
xf86DialOpen(LocalDevicePtr	local)
{
  unsigned char		dcmd[1];
  struct termios	terms;
  int			times;
  DialPtr		dial = (DialPtr) local->private;
  int			len;


   DBG(1, ErrorF("opening %s\n", dial->port));

   if (local->fd < 0) {
     SYSCALL(local->fd = open(dial->port, O_RDWR|O_NDELAY, 0));
     if (local->fd == -1) {
       ErrorF("Error opening %s : %s\n", dial->port, strerror(errno));
       return !Success;
     }
   }
   
   /* init serial port */
  if (tcgetattr(local->fd, &terms) == -1) {
    
    ErrorF("Dial box cant tcgetattr comnet on ");
    
    return !Success;
  }

  /* change the modes */

  terms.c_iflag = 0;
  terms.c_oflag = 0;
  terms.c_cflag = B9600|CS8|CREAD|CLOCAL;
  terms.c_lflag = 0;
  
  terms.c_cc[VINTR] = 0;
  terms.c_cc[VQUIT] = 0;
  terms.c_cc[VERASE] = 0;
  terms.c_cc[VEOF] = 0;
#ifdef VWERASE
  terms.c_cc[VWERASE] = 0;
#endif
#ifdef VREPRINT
  terms.c_cc[VREPRINT] = 0;
#endif
  terms.c_cc[VKILL] = 0;
  terms.c_cc[VEOF] = 0;
  terms.c_cc[VEOL] = 0;
#ifdef VEOL2
  terms.c_cc[VEOL2] = 0;
#endif
  terms.c_cc[VSUSP] = 0;
#ifdef VDSUSP
  terms.c_cc[VDSUSP] = 0;
#endif
#ifdef VDISCARD
  terms.c_cc[VDISCARD] = 0;
#endif
#ifdef VLNEXT
  terms.c_cc[VLNEXT] = 0; 
#endif
	
  /* minimum 1 character in one read call and timeout to 100 ms */
  terms.c_cc[VMIN] = 1;
  terms.c_cc[VTIME] = 10;

  if (tcsetattr(local->fd, TCSANOW,&terms) == -1) {
    ErrorF("Dial box cant tcsetattr comnet on ");
    return !Success;
  }

  /* discard all unread unwrite data */

  tcflush(local->fd, TCIOFLUSH );
	
  DBG(1, ErrorF("Dial box initialization in progress.....\n"));

  dcmd[0] = DIAL_INITIALIZE;

  times = 10;
  do {
    unsigned char	ch;
	  
    CHECKEDWRITE(local->fd, dcmd, sizeof(dcmd)  );
    times--;
    wait_for_fd(local->fd);
    SYSCALL(len = read(local->fd, &ch, 1));

    DBG(10, ErrorF("received[%d] 0x%02x\n", times, ch));

    dial->initialized = (len == 1) && (ch == DIAL_INITIALIZED);
  } while (times > 0 && !dial->initialized);

  dial_setup(local->fd, (DialPtr)local->private);
  
  ErrorF("Dial box initialization %s\n", dial->initialized ? "done" : "failed");

  if (len <= 0) {
    SYSCALL(close(local->fd));
    local->fd = -1;
    return !Success;
  }

  return Success;
}

/*
 ***************************************************************************
 *
 * xf86DialOpenDevice --
 *	Open the physical device and init information structs.
 *
 ***************************************************************************
 */
static int
xf86DialOpenDevice(DeviceIntPtr       pDial)
{
    LocalDevicePtr	local = (LocalDevicePtr)pDial->public.devicePrivate;
    DialPtr	priv = (DialPtr)PRIVATE(pDial);
    double		screenRatio, tabletRatio;
    int			gap;
    int			loop;
    
    if (local->fd < 0) {
      xf86DialOpen(local);
    }
    
    return (local->fd != -1);
}

/*
 ***************************************************************************
 *
 * xf86DialClose --
 *
 ***************************************************************************
 */
static void
xf86DialClose(LocalDevicePtr	local)
{
    DialPtr	priv = (DialPtr)local->private;
    int			loop;
    int			num = 0;
    
    SYSCALL(close(local->fd));
    local->fd = -1;
}

/*
 ***************************************************************************
 *
 * xf86DialProc --
 *      Handle the initialization, etc. of a dial
 *
 ***************************************************************************
 */
static int
xf86DialProc(DeviceIntPtr       pDial,
	    int                what)
{
    CARD8                 map[33];
    int                   nbaxes;
    int                   nbbuttons;
    KeySymsRec            keysyms;
    int                   loop;
    LocalDevicePtr        local = (LocalDevicePtr)pDial->public.devicePrivate;
    DialPtr		  priv = (DialPtr)PRIVATE(pDial);
  
    DBG(2, ErrorF("BEGIN xf86DialProc dev=0x%x priv=0x%x flags=%d what=%d\n",
		  pDial, priv, priv->flags, what));
  
    switch (what)
	{
	case DEVICE_INIT: 
	    DBG(1, ErrorF("xf86DialProc pDial=0x%x what=INIT\n", pDial));
      
	    nbaxes = 8;
	    nbbuttons = 32;
	    
	    for(loop=1; loop<=nbbuttons; loop++) map[loop] = loop;

	    if (InitButtonClassDeviceStruct(pDial,
					    nbbuttons,
					    map) == FALSE) {
		ErrorF("unable to allocate Button class device\n");
		return !Success;
	    }
      
	    if (InitFocusClassDeviceStruct(pDial) == FALSE) {
		ErrorF("unable to init Focus class device\n");
		return !Success;
	    }
          
	    if (InitPtrFeedbackClassDeviceStruct(pDial,
						 xf86DialControlProc) == FALSE) {
		ErrorF("unable to init ptr feedback\n");
		return !Success;
	    }

	    if (InitLedFeedbackClassDeviceStruct(pDial,
						 xf86DialLedControlProc) == FALSE) {
		ErrorF("unable to init leds feedback\n");
		return !Success;
	    }
	    if (InitProximityClassDeviceStruct(pDial) == FALSE) {
		ErrorF("unable to init proximity class device\n");
		return !Success;
	    }

	    if (InitValuatorClassDeviceStruct(pDial, 
					      nbaxes,
					      xf86GetMotionEvents, 
					      local->history_size,
					      ((priv->flags & ABSOLUTE_FLAG) 
					      ? Absolute : Relative))
		== FALSE) {
		ErrorF("unable to allocate Valuator class device\n"); 
		return !Success;
	    }
	    else {
	      int	loop;
	      
	      /* init ranges of valuators */
	      for(loop=0; loop<nbaxes; loop++) {
		InitValuatorAxisStruct(pDial,
				       loop,
				       -32768, /* min val */
				       32767, /* max val */
				       200, /* resolution */
				       0, /* min_res */
				       400); /* max_res */

	      }

		/* allocate the motion history buffer if needed */
		xf86MotionHistoryAllocate(local);

		AssignTypeAndName(pDial, local->atom, local->name);
	    }

	    /* open the device to gather informations */
	    xf86DialOpenDevice(pDial);
	    break; 
      
	case DEVICE_ON:
	    DBG(1, ErrorF("xf86DialProc pDial=0x%x what=ON\n", pDial));

	    if ((local->fd < 0) && (!xf86DialOpenDevice(pDial))) {
		return !Success;
	    }      
	    AddEnabledDevice(local->fd);
	    pDial->public.on = TRUE;
	    break;
      
	case DEVICE_OFF:
	    DBG(1, ErrorF("xf86DialProc  pDial=0x%x what=%s\n", pDial,
			  (what == DEVICE_CLOSE) ? "CLOSE" : "OFF"));
	    if (local->fd >= 0)
		RemoveEnabledDevice(local->fd);
	    pDial->public.on = FALSE;
	    break;
      
	case DEVICE_CLOSE:
	    DBG(1, ErrorF("xf86DialProc  pDial=0x%x what=%s\n", pDial,
			  (what == DEVICE_CLOSE) ? "CLOSE" : "OFF"));
	    xf86DialClose(local);
	    break;

	default:
	    ErrorF("unsupported mode=%d\n", what);
	    return !Success;
	    break;
	}
    DBG(2, ErrorF("END   xf86DialProc Success what=%d dev=0x%x priv=0x%x\n",
		  what, pDial, priv));
    return Success;
}

/*
 ***************************************************************************
 *
 * xf86DialChangeControl --
 *
 ***************************************************************************
 */
static int
xf86DialChangeControl(LocalDevicePtr	local,
		     xDeviceCtl		*control)
{
    xDeviceResolutionCtl	*res;
    int				*resolutions;
    char			str[10];
  
    res = (xDeviceResolutionCtl *)control;
	
    if ((control->control != DEVICE_RESOLUTION) ||
	(res->num_valuators < 1))
	return (BadMatch);
  
    resolutions = (int *)(res +1);
    
    DBG(3, ErrorF("xf86DialChangeControl changing to %d (suppressing under)\n",
		  resolutions[0]));

    sprintf(str, "SU%d\r", resolutions[0]);
    SYSCALL(write(local->fd, str, strlen(str)));
  
    return(Success);
}

/*
 ***************************************************************************
 *
 * xf86DialSwitchMode --
 *
 ***************************************************************************
 */
static int
xf86DialSwitchMode(ClientPtr	client,
		  DeviceIntPtr	dev,
		  int		mode)
{
    LocalDevicePtr        local = (LocalDevicePtr)dev->public.devicePrivate;
    DialPtr        priv = (DialPtr)local->private;

    DBG(3, ErrorF("xf86DialSwitchMode dev=0x%x mode=%d\n", dev, mode));
  
    if (mode == Absolute) {
	priv->flags = priv->flags | ABSOLUTE_FLAG;
    }
    else {
	if (mode == Relative) {
	    priv->flags = priv->flags & ~ABSOLUTE_FLAG; 
	}
	else {
	    DBG(1, ErrorF("xf86DialSwitchMode dev=0x%x invalid mode=%d\n", dev,
			  mode));
	    return BadMatch;
	}
    }
    return Success;
}

/*
 ***************************************************************************
 *
 * xf86DialAllocate --
 *
 ***************************************************************************
 */
static LocalDevicePtr
xf86DialAllocate()
{
    LocalDevicePtr      local = (LocalDevicePtr) xalloc(sizeof(LocalDeviceRec));
    DialPtr		priv = (DialPtr) xalloc(sizeof(DialRec));
#if defined(sun) && !defined(i386)
    char			*dev_name = (char *) getenv("DIAL_DEV");  
#endif

    local->name = "dial+buttons";
    local->flags = 0; /*XI86_NO_OPEN_ON_INIT;*/
#if !defined(sun) || defined(i386)
    local->device_config = xf86DialConfig;
#endif
    local->device_control = xf86DialProc;
    local->read_input = xf86DialReadInput;
    local->control_proc = xf86DialChangeControl;
    local->close_proc = xf86DialClose;
    local->switch_mode = xf86DialSwitchMode;
    local->conversion_proc = xf86DialConvert;
    local->reverse_conversion_proc = xf86DialReverseConvert;
    local->fd = -1;
    local->atom = 0;
    local->dev = NULL;
    local->private = priv;
    local->private_flags = 0;
    local->history_size  = 0;
    local->old_x = -1;
    local->old_y = -1;
    local->type_name = "SGI Knob Box";
    local->history_size = 0;
    
    priv->port = "";		        /* device file name */
    priv->flags = ABSOLUTE_FLAG;
#if defined(sun) && !defined(i386)
    if (dev_name) {
	common->dialDevice = (char*) xalloc(strlen(dev_name)+1);
	strcpy(common->dialDevice, dev_name);
	ErrorF("xf86DialOpen port changed to '%s'\n", common->dialDevice);
    }
#endif
    
    return local;
}

/*
 ***************************************************************************
 *
 * Dial device association --
 *
 ***************************************************************************
 */
DeviceAssocRec dial_assoc =
{
    SECTION_NAME,		/* config_section_name */
    xf86DialAllocate		/* device_allocate */
};

#ifdef DYNAMIC_MODULE
/*
 ***************************************************************************
 *
 * entry point of dynamic loading
 *
 ***************************************************************************
 */
int
#ifndef DLSYM_BUG
init_module(unsigned long	server_version)
#else
init_xf86Dial(unsigned long    server_version)
#endif
{
    xf86AddDeviceAssoc(&dial_assoc);

    if (server_version != XF86_VERSION_CURRENT) {
	ErrorF("Warning: Dial module compiled for version%s\n", XF86_VERSION);
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
XF86ModuleVersionInfo xf86DialVersion = {
    "xf86Dial",
    MODULEVENDORSTRING,
    MODINFOSTRING1,
    MODINFOSTRING2,
    XF86_VERSION_CURRENT,
    0x00010000,
    {0,0,0,0}
};

void
xf86DialModuleInit(data, magic)
    pointer *data;
    INT32 *magic;
{
    static int cnt = 0;

    switch (cnt) {
      case 0:
	*magic = MAGIC_VERSION;
	*data = &xf86DialVersion;
	cnt++;
	break;
	
      case 1:
	*magic = MAGIC_ADD_XINPUT_DEVICE;
	*data = &dial_assoc;
	cnt++;
	break;

    default:
	*magic = MAGIC_DONE;
	*data = NULL;
	break;
    } 
}
#endif

/*
 * Local variables:
 * change-log-default-name: "~/xinput.log"
 * End:
 */
/* end of xf86Dial.c */

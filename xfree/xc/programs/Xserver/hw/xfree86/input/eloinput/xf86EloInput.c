/*
 * (C) Copyright 2004 Fred Gleason <fredg@salemradiolabs.com>
 *
 * Parts of this driver are based on the Elographics driver.
 * Copyright 1995, 1999 by Patrick Lecoanet, France. <lecoanet@cena.dgac.fr>
 *                                                                            
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is  hereby granted without fee, provided that
 * the  above copyright   notice appear  in   all  copies and  that both  that
 * copyright  notice   and   this  permission   notice  appear  in  supporting
 * documentation, and that   the  name of  Patrick  Lecoanet not  be  used  in
 * advertising or publicity pertaining to distribution of the software without
 * specific,  written      prior  permission.     Patrick Lecoanet   makes  no
 * representations about the suitability of this software for any purpose.  It
 * is provided "as is" without express or implied warranty.                   
 *                                                                            
 * PATRICK LECOANET DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT  SHALL PATRICK LECOANET BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA  OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS  ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 *
 */

/* $XFree86: xc/programs/Xserver/hw/xfree86/input/eloinput/xf86EloInput.c,v 1.2 2005/01/21 18:07:12 tsi Exp $ */

/*
 ******************************************************************************
 ******************************************************************************
 *
 * This driver is able to deal with Elographics 2500U USB-based touchscreen
 * controllers, using the generic event API in the Linux Input subsystem.
 *
 ******************************************************************************
 ******************************************************************************
 */

#include <asm/types.h>
#include <linux/input.h>
#undef BUS_PCI
#undef BUS_ISA
#ifndef EV_SYN
#define EV_SYN 0x00
#endif

#include "xf86Version.h"
#if XF86_VERSION_CURRENT >= XF86_VERSION_NUMERIC(3,9,0,0,0)
#define XFREE86_V4
#endif

#ifdef XFREE86_V4

#ifndef XFree86LOADER
#include <unistd.h>
#include <errno.h>
#endif

#include "misc.h"
#include "xf86.h"
#if !defined(DGUX)
#include "xf86_ansic.h"
#endif
#include "xf86_OSproc.h"
#include "xf86Xinput.h"
#include "exevents.h"

#ifdef XFree86LOADER
#include "xf86Module.h"
#endif

#else /* XFREE86_V4 */

#include "Xos.h"
#include <signal.h>
#include <stdio.h>

#define	 NEED_EVENTS
#include "X.h"
#include "Xproto.h"
#include "inputstr.h"
#include "scrnintstr.h"
#include "XI.h"
#include "XIproto.h"

#if defined(sun) && !defined(i386)
#include <errno.h>
#include <termio.h>
#include <fcntl.h>
#include <ctype.h>

#include "extio.h"
#else /* defined(sun) && !defined(i386) */
#include "compiler.h"

#include "xf86.h"
#include "xf86Procs.h"
#include "xf86_OSlib.h"
#include "xf86_Config.h"
#include "xf86Xinput.h"
#endif /* defined(sun) && !defined(i386) */

#if !defined(sun) || defined(i386)
#include "os.h"
#include "osdep.h"
#include "exevents.h"

#include "extnsionst.h"
#include "extinit.h"
#endif /* !defined(sun) || defined(i386) */

#endif /* XFREE86_V4 */

#ifndef XFREE86_V4
#if !defined(sun) || defined(i386)
/*
 ***************************************************************************
 *
 * Configuration descriptor.
 *
 ***************************************************************************
 */

#define PORT		1
#define ELO_DEVICE_NAME	2
#define SCREEN_NO	3
#define MAXX		6
#define MAXY		7
#define MINX		8
#define MINY		9
#define DEBUG_LEVEL     10
#define HISTORY_SIZE	11
#define ALWAYS_CORE	13
#define SWAP_AXES	14
#define PORTRAIT_MODE	15

static SymTabRec EloTab[] = {
  { ENDSUBSECTION,     "endsubsection" },
  { PORT,              "port" },
  { ELO_DEVICE_NAME,   "devicename" },
  { SCREEN_NO,	       "screenno" },
  { MAXX,              "maximumxposition" },
  { MAXY,              "maximumyposition" },
  { MINX,              "minimumxposition" },
  { MINY,              "minimumyposition" },
  { DEBUG_LEVEL,       "debuglevel" },
  { HISTORY_SIZE,      "historysize" },
  { ALWAYS_CORE,       "alwayscore" },
  { SWAP_AXES,	       "swapxy" },
  { PORTRAIT_MODE,     "portraitmode" },
  { -1,                "" },
};
#endif /* !defined(sun) || defined(i386) */

#endif /* XFREE86_V4 */


/*
 ***************************************************************************
 *
 * Default constants.
 *
 ***************************************************************************
 */
#define ELO_PORT		"/dev/input/event0"
#define DEFAULT_MAX_X		3000
#define DEFAULT_MIN_X		600
#define DEFAULT_MAX_Y		3000
#define DEFAULT_MIN_Y		600


/*
 ***************************************************************************
 *
 * Useful macros.
 *
 ***************************************************************************
 */
#define WORD_ASSEMBLY(byte1, byte2)	(((byte2) << 8) | (byte1))
#define SYSCALL(call)			while(((call) == -1) && (errno == EINTR))

/* This one is handy, thanx Fred ! */
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

#ifdef XFREE86_V4
#undef SYSCALL
#undef read
#undef write
#undef close
#define SYSCALL(call) call
#define read(fd, ptr, num) xf86ReadSerial(fd, ptr, num)
#define write(fd, ptr, num) xf86WriteSerial(fd, ptr, num)
#define close(fd) xf86CloseSerial(fd)
#endif

/*
 ***************************************************************************
 *
 * Device private records.
 *
 ***************************************************************************
 */
typedef struct _EloPrivateRec {
  char		*input_dev;	/* The touchscreen input tty	             */
  int		min_x;		/* Minimum x reported by calibration         */
  int		max_x;		/* Maximum x  	                             */
  int		min_y;		/* Minimum y reported by calibration         */
  int		max_y;		/* Maximum y			             */
  int		screen_no;	/* Screen associated with the device	     */
  int		screen_width;	/* Width of the associated X screen          */
  int		screen_height;	/* Height of the screen			     */
  Bool		inited;		/* The controller has already been config'd? */
  int		swap_axes;	/* Swap X an Y axes if != 0                  */
  int           xpos;           /* Current X position                        */
  int           ypos;           /* Current Y position                        */
  int           zpos;           /* Current Z position                        */
  int           touched;        /* Current touch state                       */
} EloPrivateRec, *EloPrivatePtr;


#ifndef XFREE86_V4
#if !defined(sun) || defined(i386)
/*
 ***************************************************************************
 *
 * xf86EloInputConfig --
 *	Configure the driver from the configuration data.
 *
 ***************************************************************************
 */
static Bool
xf86EloInputConfig(LocalDevicePtr    *array,
              int               inx,
              int               max,
	      LexPtr            val)
{
  LocalDevicePtr        local = array[inx];
  EloPrivatePtr         priv = (EloPrivatePtr)(local->private);
  int                   token;
  int			portrait=0;
  
  while ((token = xf86GetToken(EloTab)) != ENDSUBSECTION) {
    switch(token) {
      
    case PORT:
      if (xf86GetToken(NULL) != STRING) {
	xf86ConfigError("Eloinput input port expected");
      }
      priv->input_dev = strdup(val->str);	
      if (xf86Verbose) {
	ErrorF("%s Eloinput input port: %s\n",
	       XCONFIG_GIVEN, priv->input_dev);
      }
      break;

    case ELO_DEVICE_NAME:
      if (xf86GetToken(NULL) != STRING) {
	xf86ConfigError("Eloinput device name expected");
      }
      local->name = strdup(val->str);
      if (xf86Verbose) {
	ErrorF("%s Eloinput X device name: %s\n",
	       XCONFIG_GIVEN, local->name);
      }
      break;

    case SCREEN_NO:
      if (xf86GetToken(NULL) != NUMBER) {
	xf86ConfigError("Eloinput screen number expected");
      }
      priv->screen_no = val->num;
      if (xf86Verbose) {
	ErrorF("%s Eloinput associated screen: %d\n",
	       XCONFIG_GIVEN, priv->screen_no);
      }
      break;

    case MAXX:
      if (xf86GetToken(NULL) != NUMBER) {
        xf86ConfigError("Eloinput maximum x position expected");
      }
      priv->max_x = val->num;
      if (xf86Verbose) {
	ErrorF("%s Eloinput maximum x position: %d\n",
	       XCONFIG_GIVEN, priv->max_x);
      }
     break;
      
    case MAXY:
      if (xf86GetToken(NULL) != NUMBER) {
        xf86ConfigError("Eloinput maximum y position expected");
      }
      priv->max_y = val->num;
      if (xf86Verbose) {
	ErrorF("%s Eloinput maximum y position: %d\n",
	       XCONFIG_GIVEN, priv->max_y);
      }
     break;
      
    case MINX:
      if (xf86GetToken(NULL) != NUMBER) {
        xf86ConfigError("Eloinput minimum x position expected");
      }
      priv->min_x = val->num;
      if (xf86Verbose) {
	ErrorF("%s Eloinput minimum x position: %d\n",
	       XCONFIG_GIVEN, priv->min_x);
      }
     break;
      
    case MINY:
      if (xf86GetToken(NULL) != NUMBER) {
        xf86ConfigError("Eloinput minimum y position expected");
      }
      priv->min_y = val->num;
      if (xf86Verbose) {
	ErrorF("%s Eloinput minimum y position: %d\n",
	       XCONFIG_GIVEN, priv->min_y);
      }
     break;
      
    case DEBUG_LEVEL:
      if (xf86GetToken(NULL) != NUMBER) {
	xf86ConfigError("Eloinput driver debug level expected");
      }
      debug_level = val->num;
      if (xf86Verbose) {
#if DEBUG
	ErrorF("%s Eloinput debug level sets to %d\n", XCONFIG_GIVEN,
	       debug_level);      
#else
	ErrorF("%s Eloinput debug not available\n",
	       XCONFIG_GIVEN, debug_level);      
#endif
      }
      break;

    case HISTORY_SIZE:
      if (xf86GetToken(NULL) != NUMBER) {
	xf86ConfigError("Eloinput motion history size expected");
      }
      local->history_size = val->num;
      if (xf86Verbose) {
	ErrorF("%s EloGraphics motion history size is %d\n", XCONFIG_GIVEN,
	       local->history_size);
      }
      break;
	    
    case ALWAYS_CORE:
      xf86AlwaysCore(local, TRUE);
      if (xf86Verbose) {
	ErrorF("%s Eloinput device will always stays core pointer\n",
	       XCONFIG_GIVEN);
      }
      break;

    case SWAP_AXES:
      priv->swap_axes = 1;
      if (xf86Verbose) {
	ErrorF("%s Eloinput device will work with X and Y axes swapped\n",
	       XCONFIG_GIVEN);
      }      
      break;

    case PORTRAIT_MODE:
      if (xf86GetToken(NULL) != STRING) {
      portrait_mode_err:
	xf86ConfigError("Eloinput portrait mode should be: Portrait, Landscape or PortraitCCW");
      }
      if (strcmp(val->str, "portrait") == 0) {
	portrait = 1;
      }
      else if (strcmp(val->str, "portraitccw") == 0) {
	portrait = -1;
      }
      else if (strcmp(val->str, "landscape") != 0) {
	goto portrait_mode_err;
      }
      if (xf86Verbose) {
	ErrorF("%s Eloinput device will work in %s mode\n",
	       XCONFIG_GIVEN, val->str);
      }      
      break;

    case EOF:
      FatalError("Unexpected EOF (missing EndSubSection)");
      break;

    default:
      xf86ConfigError("Eloinput subsection keyword expected");
      break;
    }
  }

  if (priv->max_x - priv->min_x <= 0) {
    ErrorF("%s Eloinput: reverse x mode (minimum x position >= maximum x position)\n",
	   XCONFIG_GIVEN);
  }  
  if (priv->max_y - priv->min_y <= 0) {
    ErrorF("%s Eloinput: reverse y mode (minimum y position >= maximum y position)\n",
	   XCONFIG_GIVEN);
  }
  /*
   * The portrait adjustments need to be done after axis reversing
   * and axes swap. This way the driver can cope with defective
   * hardware and still do the correct processing depending on the
   * actual display orientation.
   */
  if (portrait == 1) {
    /*
     * Portrait Clockwise: reverse Y axis and exchange X and Y.
     */
    int tmp;
    tmp = priv->min_y;
    priv->min_y = priv->max_y;
    priv->max_y = tmp;
    priv->swap_axes = (priv->swap_axes==0) ? 1 : 0;
  }
  else if (portrait == -1) {
    /*
     * Portrait Counter Clockwise: reverse X axis and exchange X and Y.
     */
    int tmp;
    tmp = priv->min_x;
    priv->min_x = priv->max_x;
    priv->max_x = tmp;
    priv->swap_axes = (priv->swap_axes==0) ? 1 : 0;
  }
    
  DBG(2, ErrorF("xf86EloInputConfig port name=%s\n", priv->input_dev))

  return Success;
}
#endif
#endif


/*
 ***************************************************************************
 *
 * xf86EloInputConvert --
 *	Convert extended valuators to x and y suitable for core motion
 *	events. Return True if ok and False if the requested conversion
 *	can't be done for the specified valuators.
 *
 ***************************************************************************
 */
static Bool
xf86EloInputConvert(LocalDevicePtr	local,
	       int		first,
	       int		num,
	       int		v0,
	       int		v1,
	       int		v2,
	       int		v3,
	       int		v4,
	       int		v5,
	       int		*x,
	       int		*y)
{
  EloPrivatePtr	priv = (EloPrivatePtr) local->private;
  int		width = priv->max_x - priv->min_x;
  int		height = priv->max_y - priv->min_y;
  int		input_x, input_y;
  
  if (first != 0 || num != 2) {
    return FALSE;
  }

  DBG(3, ErrorF("EloConvert: v0(%d), v1(%d)\n",	v0, v1));

  if (priv->swap_axes) {
    input_x = v1;
    input_y = v0;
  }
  else {
    input_x = v0;
    input_y = v1;
  }
  *x = (priv->screen_width * (input_x - priv->min_x)) / width;
  *y = (priv->screen_height -
	(priv->screen_height * (input_y - priv->min_y)) / height);
  
#ifdef XFREE86_V4
  /*
   * Need to check if still on the correct screen.
   * This call is here so that this work can be done after
   * calib and before posting the event.
   */
  xf86XInputSetScreen(local, priv->screen_no, *x, *y);
#endif
  
  DBG(3, ErrorF("EloConvert: x(%d), y(%d)\n",	*x, *y));

  return TRUE;
}


/*
 ***************************************************************************
 *
 * xf86EloInputReadInput --
 *	Read all pending input events from the touchscreen and enqueue
 *	them.
 *
 ***************************************************************************
 */
static void
xf86EloInputReadInput(LocalDevicePtr	local)
{
  EloPrivatePtr                 priv = (EloPrivatePtr)(local->private);
  int                           n;
  struct input_event            ievent[10];
  int i;

  DBG(4, ErrorF("Entering ReadInput\n"));

  SYSCALL(n = read(local->fd,(char *)(ievent),10*sizeof(struct input_event)));
  DBG(4, ErrorF("Read %ld input events\n",(long)n/sizeof(struct input_event)));
  for(i=0;i<n/sizeof(struct input_event);i++) {
    switch(ievent[i].type) {
	case EV_ABS:
	  switch(ievent[i].code) {
	      case ABS_Y:
		priv->xpos=ievent[i].value;
		xf86PostMotionEvent(local->dev, TRUE, 0, 2, 
				    priv->xpos, priv->ypos);
		break;
		
	      case ABS_Z:
		priv->ypos=ievent[i].value;
		xf86PostMotionEvent(local->dev, TRUE, 0, 2, 
				    priv->xpos, priv->ypos);
		break;
		
	      case 11:
		priv->zpos=ievent[i].value;
		xf86PostMotionEvent(local->dev, TRUE, 0, 2, 
				    priv->xpos, priv->ypos);
		if((priv->touched==0) && (priv->zpos>0)) {  /* Touch */
		  xf86PostButtonEvent(local->dev, TRUE, 1, TRUE , 0, 2, 
				      priv->xpos, priv->ypos);
		  priv->touched=1;
		  DBG(2, ErrorF("TouchPress X: %d  Y: %d\n",
				priv->xpos,priv->ypos));
		}
		else {
		  if((priv->touched==1) && (priv->zpos==0)) {  /* Release */
		    xf86PostButtonEvent(local->dev, TRUE, 1, FALSE , 0, 2, 
					priv->xpos, priv->ypos);
		    priv->touched=0;
		    DBG(2, ErrorF("TouchRelease X: %d  Y: %d\n",
				  priv->xpos,priv->ypos));
		  }
		}
		break;
	  }
	  break;
	  
	case EV_SYN:
	  DBG(4, ErrorF("Received SYN event type -  CODE: %d.\n",
			ievent[i].code));
	  break;
	  
	case EV_KEY:
	  DBG(4, ErrorF("Received KEY event type -  CODE: %d.\n",
			ievent[i].code));
	  break;
	  
	default:
	  DBG(4, ErrorF("Unrecognized input event type %d.\n",ievent[i].type));
	break;
    }
  }
}


/*
 ***************************************************************************
 *
 * xf86EloInputControl --
 *
 ***************************************************************************
 */
static Bool
xf86EloInputControl(DeviceIntPtr	dev,
	       int		mode)
{
  LocalDevicePtr	local = (LocalDevicePtr) dev->public.devicePrivate;
  EloPrivatePtr		priv = (EloPrivatePtr)(local->private);
  unsigned char		map[] = { 0, 1 };

  switch(mode) {

  case DEVICE_INIT:
    {
      DBG(2, ErrorF("Eloinput touchscreen init...\n"));
      if (priv->screen_no >= screenInfo.numScreens ||
	  priv->screen_no < 0) {
	priv->screen_no = 0;
      }
      priv->screen_width = screenInfo.screens[priv->screen_no]->width;
      priv->screen_height = screenInfo.screens[priv->screen_no]->height;

      /*
       * Device reports button press for up to 1 button.
       */
      if (InitButtonClassDeviceStruct(dev, 1, map) == FALSE) {
	ErrorF("Unable to allocate Eloinput touchscreen ButtonClassDeviceStruct\n");
	return !Success;
      }
      
      if (InitFocusClassDeviceStruct(dev) == FALSE) {
	ErrorF("Unable to allocate Eloinput touchscreen FocusClassDeviceStruct\n");
	return !Success;
      }
      
      /*
       * Device reports motions on 2 axes in absolute coordinates.
       * Axes min and max values are reported in raw coordinates.
       * Resolution is computed roughly by the difference between
       * max and min values scaled from the approximate size of the
       * screen to fit one meter.
       */
      if (InitValuatorClassDeviceStruct(dev, 2, xf86GetMotionEvents,
					local->history_size, Absolute) == FALSE) {
	ErrorF("Unable to allocate Eloinput touchscreen ValuatorClassDeviceStruct\n");
	return !Success;
      }
      else {
	InitValuatorAxisStruct(dev, 0, priv->min_x, priv->max_x,
			       9500,
			       0     /* min_res */,
			       9500  /* max_res */);
	InitValuatorAxisStruct(dev, 1, priv->min_y, priv->max_y,
			       10500,
			       0     /* min_res */,
			       10500 /* max_res */);
      }

      if (InitFocusClassDeviceStruct(dev) == FALSE) {
	ErrorF("Unable to allocate Eloinput touchscreen FocusClassDeviceStruct\n");
      }
      
      /*
       * Allocate the motion events buffer.
       */
      xf86MotionHistoryAllocate(local);
      
#ifndef XFREE86_V4
      AssignTypeAndName(dev, local->atom, local->name);
#endif
      
      DBG(2, ErrorF("Done.\n"));
      return Success;
    }
    
  case DEVICE_ON:
    DBG(2, ErrorF("Eloinput touchscreen on...\n"));

    if (local->fd < 0) {
      DBG(2, ErrorF("Eloinput touchscreen opening : %s\n", priv->input_dev));
      SYSCALL(local->fd=open(priv->input_dev,O_RDONLY,0));
      if (local->fd < 0) {
	Error("Unable to open Eloinput touchscreen device");
	return !Success;
      }
      
#ifdef XFREE86_V4
      xf86AddEnabledDevice(local);
#else
      AddEnabledDevice(local->fd);
#endif
      dev->public.on = TRUE;  
    }
    
    DBG(2, ErrorF("Done\n"));
    return Success;
    
    /*
     * Deactivate the device. After this, the device will not emit
     * events until a subsequent DEVICE_ON. Thus, we can momentarily
     * close the port.
     */
  case DEVICE_OFF:
    DBG(2, ErrorF("Eloinput touchscreen off...\n"));
    dev->public.on = FALSE;
    if (local->fd >= 0) {
#ifdef XFREE86_V4
      xf86RemoveEnabledDevice(local);
#else
      RemoveEnabledDevice(local->fd);
#endif
    }
    SYSCALL(close(local->fd));
    local->fd = -1;
    DBG(2, ErrorF("Done\n"));
    return Success;
    
    /*
     * Final close before server exit. This is used during server shutdown.
     * Close the port and free all the resources.
     */
  case DEVICE_CLOSE:
    DBG(2, ErrorF("Eloinput touchscreen close...\n"));
    dev->public.on = FALSE;
    if (local->fd >= 0) {
      RemoveEnabledDevice(local->fd);
    }
    SYSCALL(close(local->fd));
    local->fd = -1;
    DBG(2, ErrorF("Done\n"));
    return Success;

  default:
      ErrorF("unsupported mode=%d\n", mode);
      return !Success;
  }
}

/*
 ***************************************************************************
 *
 * xf86EloInputAllocate --
 *
 ***************************************************************************
 */
static LocalDevicePtr
#ifndef XFREE86_V4
xf86EloInputAllocate(void)
#else
xf86EloInputAllocate(InputDriverPtr	drv)     
#endif
{
  LocalDevicePtr	local;
  EloPrivatePtr		priv;

  priv = xalloc(sizeof(EloPrivateRec));
  if (!priv)
    return NULL;

#ifndef XFREE86_V4
  local = (LocalDevicePtr) xalloc(sizeof(LocalDeviceRec));
#else
  local = xf86AllocateInput(drv, 0);
#endif

  if (!local) {
    xfree(priv);
    return NULL;
  }
  
#ifdef XFREE86_V4
  priv->input_dev = strdup(ELO_PORT);
#else
  priv->input_dev = ELO_PORT;
#endif
  priv->min_x = 0;
  priv->max_x = 3000;
  priv->min_y = 0;
  priv->max_y = 3000;
  priv->screen_no = 0;
  priv->screen_width = -1;
  priv->screen_height = -1;
  priv->inited = 0;
  priv->swap_axes = 0;
  priv->xpos = 0;
  priv->ypos = 0;
  priv->zpos = 0;
  priv->touched = 0;

  local->name = XI_TOUCHSCREEN;
  local->flags = 0 /* XI86_NO_OPEN_ON_INIT */;
#ifndef XFREE86_V4
#if !defined(sun) || defined(i386)
  local->device_config = xf86EloInputConfig;
#endif
#endif
  local->device_control = xf86EloInputControl;
  local->read_input   = xf86EloInputReadInput;
  local->control_proc = NULL;
  local->close_proc   = NULL;
  local->switch_mode  = NULL;
  local->conversion_proc = xf86EloInputConvert;
  local->reverse_conversion_proc = NULL;
  local->fd	      = -1;
  local->atom	      = 0;
  local->dev	      = NULL;
  local->private      = priv;
  local->type_name    = "Elo USB TouchScreen";
  local->history_size = 0;
  
  return local;
}

#ifndef XFREE86_V4
/*
 ***************************************************************************
 *
 * Eloinput device association --
 *
 ***************************************************************************
 */
DeviceAssocRec eloinput_assoc =
{
  "eloinput",                     /* config_section_name */
  xf86EloInputAllocate               /* device_allocate */
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
init_xf86Elo(unsigned long      server_version)
#endif
{
    xf86AddDeviceAssoc(&eloinput_assoc);

    if (server_version != XF86_VERSION_CURRENT) {
      ErrorF("Warning: Eloinput module compiled for version%s\n", XF86_VERSION);
      return 0;
    }
    else {
      return 1;
    }
}
#endif

#else /* XFREE86_V4 */

static void
xf86EloInputUninit(InputDriverPtr	drv,
	      LocalDevicePtr	local,
	      int flags)
{
  EloPrivatePtr		priv = (EloPrivatePtr) local->private;
  
  xf86EloInputControl(local->dev, DEVICE_OFF);

  xfree(priv->input_dev);
  xfree(priv);
  xfree(local->name);
  xfree(local);

  xf86DeleteInput(local, 0);
}

static const char *default_options[] = {
  "BaudRate", "9600",
  "StopBits", "1",
  "DataBits", "8",
  "Parity", "None",
  "Vmin", "10",
  "Vtime", "1",
  "FlowControl", "None",
  NULL
};

static InputInfoPtr
xf86EloInputInit(InputDriverPtr	drv,
	    IDevPtr		dev,
	    int			flags)
{
  LocalDevicePtr	local=NULL;
  EloPrivatePtr		priv=NULL;
  char			*str;
  int			portrait = 0;
  

  local = xf86EloInputAllocate(drv);
  if (!local) {
    return NULL;
  }
  priv = local->private;
  local->conf_idev = dev;
  
  xf86CollectInputOptions(local, default_options, NULL);
  /* Process the common options. */
  xf86ProcessCommonOptions(local, local->options);

  str = xf86FindOptionValue(local->options, "Device");
  if (!str) {
    xf86Msg(X_ERROR, "%s: No Device specified in Eloinput module config.\n",
	    dev->identifier);
    if (priv) {
      if (priv->input_dev) {
	xfree(priv->input_dev);
      }
      xfree(priv);
    }
    return local;
  }
  priv->input_dev = strdup(str);

  local->name = xf86SetStrOption(local->options, "DeviceName", XI_TOUCHSCREEN);
  xf86Msg(X_CONFIG, "Eloinput X device name: %s\n", local->name);  
  priv->screen_no = xf86SetIntOption(local->options, "ScreenNo", 0);
  xf86Msg(X_CONFIG, "Eloinput associated screen: %d\n", priv->screen_no);  
  priv->max_x = xf86SetIntOption(local->options, "MaxX", 3000);
  xf86Msg(X_CONFIG, "Eloinput maximum x position: %d\n", priv->max_x);
  priv->min_x = xf86SetIntOption(local->options, "MinX", 0);
  xf86Msg(X_CONFIG, "Eloinput minimum x position: %d\n", priv->min_x);
  priv->max_y = xf86SetIntOption(local->options, "MaxY", 3000);
  xf86Msg(X_CONFIG, "Eloinput maximum y position: %d\n", priv->max_y);
  priv->min_y = xf86SetIntOption(local->options, "MinY", 0);
  xf86Msg(X_CONFIG, "Eloinput minimum y position: %d\n", priv->min_y);
  priv->swap_axes = xf86SetBoolOption(local->options, "SwapXY", 0);
  if (priv->swap_axes) {
    xf86Msg(X_CONFIG, "Eloinput device will work with X and Y axes swapped\n");
  }
  debug_level = xf86SetIntOption(local->options, "DebugLevel", 0);
  if (debug_level) {
#if DEBUG
    xf86Msg(X_CONFIG, "Eloinput debug level sets to %d\n", debug_level);      
#else
    xf86Msg(X_INFO, "Eloinput debug not available\n");      
#endif
  }
  str = xf86SetStrOption(local->options, "PortraitMode", "Landscape");
  if (strcmp(str, "Portrait") == 0) {
    portrait = 1;
  }
  else if (strcmp(str, "PortraitCCW") == 0) {
    portrait = -1;
  }
  else if (strcmp(str, "Landscape") != 0) {
    xf86Msg(X_ERROR, "Eloinput portrait mode should be: Portrait, Landscape or PortraitCCW");
    str = "Landscape";
  }
  xf86Msg(X_CONFIG, "Eloinput device will work in %s mode\n", str);      
  
  if (priv->max_x - priv->min_x <= 0) {
    xf86Msg(X_INFO, "Eloinput: reverse x mode (minimum x position >= maximum x position)\n");
  }  
  if (priv->max_y - priv->min_y <= 0) {
    xf86Msg(X_INFO, "Eloinput: reverse y mode (minimum y position >= maximum y position)\n");
  }

  if (portrait == 1) {
    /*
     * Portrait Clockwise: reverse Y axis and exchange X and Y.
     */
    int tmp;
    tmp = priv->min_y;
    priv->min_y = priv->max_y;
    priv->max_y = tmp;
    priv->swap_axes = (priv->swap_axes==0) ? 1 : 0;
  }
  else if (portrait == -1) {
    /*
     * Portrait Counter Clockwise: reverse X axis and exchange X and Y.
     */
    int tmp;
    tmp = priv->min_x;
    priv->min_x = priv->max_x;
    priv->max_x = tmp;
    priv->swap_axes = (priv->swap_axes==0) ? 1 : 0;
  }

  /* mark the device configured */
  local->flags |= XI86_CONFIGURED;
  return local;
}

#ifdef XFree86LOADER
static
#endif
InputDriverRec ELOINPUT = {
    1,				/* driver version */
    "eloinput",	                /* driver name */
    NULL,			/* identify */
    xf86EloInputInit,		/* pre-init */
    xf86EloInputUninit,		/* un-init */
    NULL,			/* module */
    0				/* ref count */
};

#ifdef XFree86LOADER
static pointer
Plug(pointer	module,
     pointer	options,
     int	*errmaj,
     int	*errmin)
{
  xf86AddInputDriver(&ELOINPUT, module, 0);
  return module;
}

static void
Unplug(pointer	p)
{
  DBG(1, ErrorF("EloUnplug\n"));
}

static XF86ModuleVersionInfo version_rec = {
  "eloinput",
  MODULEVENDORSTRING,
  MODINFOSTRING1,
  MODINFOSTRING2,
  XF86_VERSION_CURRENT,
  1, 0, 0,
  ABI_CLASS_XINPUT,
  ABI_XINPUT_VERSION,
  MOD_CLASS_XINPUT,
  { 0, 0, 0, 0 }
};

/*
 * This is the entry point in the module. The name
 * is setup after the pattern <module_name>ModuleData.
 * Do not change it.
 */
XF86ModuleData eloinputModuleData = { &version_rec, Plug, Unplug };

#endif
#endif /* XFREE86_V4 */

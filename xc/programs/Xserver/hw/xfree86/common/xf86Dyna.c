/* 
 * Copyright (c) 1999  Machine Vision Holdings Incorporated
 * Author: David Woodhouse <David.Woodhouse@mvhi.com>
 *
 * Template driver used: Copyright (c) 1998  Metro Link Incorporated
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, cpy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Except as contained in this notice, the name of the Metro Link shall not be
 * used in advertising or otherwise to promote the sale, use or other dealings
 * in this Software without prior written authorization from Metro Link.
 *
 */
/* $XFree86: xc/programs/Xserver/hw/xfree86/common/xf86Dyna.c,v 1.1.2.2 1999/07/28 13:37:42 hohndel Exp $ */


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

#include "compiler.h"

#include "xf86.h"
#include "xf86Procs.h"
#include "xf86_OSlib.h"
#include "xf86_Config.h"
#include "xf86Xinput.h"
#include "xf86Version.h"


#include "os.h"
#include "osdep.h"
#include "exevents.h"

#include "extnsionst.h"
#include "extinit.h"

#define DYNAPRO_PORT "/dev/ttyS2"
#define DYNAPRO_LINK_SPEED B2400
#define SYSCALL(call)			while(((call) == -1) && (errno == EINTR))


#define PORT		1
#define DYNA_DEVICENAME	2
#define SCREEN_NO	3
#define MAXX		4
#define MAXY		5
#define MINX		6
#define MINY		7
#define HISTORY_SIZE	8
#define LINK_SPEED	9
#define ALWAYS_CORE	10
#define BUTTON_NO       11
#define SWAP_XY         12

static SymTabRec DynaTab[] = {
  { ENDSUBSECTION,     "endsubsection" },
  { PORT,              "port" },
  { DYNA_DEVICENAME,   "devicename" },
  { SCREEN_NO,	       "screenno" },
  { MAXX,              "maximumxposition" },
  { MAXY,              "maximumyposition" },
  { MINX,              "minimumxposition" },
  { MINY,              "minimumyposition" },
  { HISTORY_SIZE,      "historysize" },
  { LINK_SPEED,        "linkspeed" },
  { ALWAYS_CORE,       "alwayscore" },
  { BUTTON_NO,         "buttonno" },
  { SWAP_XY,           "swapxy" },
  { -1,                "" },
};


#define LS2400		1
#define LS9600		2


static SymTabRec LinkSpeedTab[] = {
  { LS2400,	"b2400" },
  { LS9600,	"b9600" },
};


/*
 * This struct connects a line speed with
 * a compatible motion packet delay. The
 * driver will attempt to enforce a correct
 * delay (according to this table) in order to
 * avoid losing data in the touchscreen controller.
 * LinkSpeedValues should be kept in sync with
 * LinkSpeedTab.
 */
typedef struct {
  int	speed;
  int	delay;
} LinkParameterStruct;

static LinkParameterStruct	LinkSpeedValues[] = {
  { B2400, 8 },
  { B9600, 4 },
};



#define DYNAPRO_PACKET_SIZE		3

typedef enum
{
	Dynapro_byte0, Dynapro_byte1, Dynapro_byte2
}
DynaproState;


typedef struct _DynaproPrivateRec
{
	int min_x;				/* Minimum x reported by calibration        */
	int max_x;				/* Maximum x                    */
	int min_y;				/* Minimum y reported by calibration        */
	int max_y;				/* Maximum y                    */
	int swap_xy;				/* Swap dimensions */
	Bool button_down;			/* is the "button" currently down */
	int button_number;			/* which button to report */
	char *input_dev;
	int link_speed;				/* Speed of the RS232 link connecting the ts.	*/
	int screen_num;				/* Screen associated with the device        */
	int screen_width;			/* Width of the associated X screen     */
	int screen_height;			/* Height of the screen             */
	int proximity;
	unsigned char packet[DYNAPRO_PACKET_SIZE];	/* packet being/just read */
	DynaproState lex_mode;
}
DynaproPrivateRec, *DynaproPrivatePtr;




/******************************************************************************
 *		Declarations
 *****************************************************************************/

static Bool xf86DynaproControl (DeviceIntPtr, int);
static void xf86DynaproReadInput (LocalDevicePtr);
static Bool xf86DynaproConvert (LocalDevicePtr, int, int, int, int, int, int, int, int, int *, int *);
static Bool DynaproGetPacket (LocalDevicePtr pInfo);



static Bool
xf86DynaproConfig(LocalDevicePtr    *array,
              int               inx,
              int               max,
	      LexPtr            val)
{
  LocalDevicePtr        local = array[inx];
  DynaproPrivatePtr        priv = (DynaproPrivatePtr)(local->private);
  int                   token;

  while ((token = xf86GetToken(DynaTab)) != ENDSUBSECTION) {
    switch(token) {
      
    case PORT:
      if (xf86GetToken(NULL) != STRING) {
	xf86ConfigError("Dynapro input port expected");
      }
      priv->input_dev = strdup(val->str);	
      if (xf86Verbose) {
	ErrorF("%s Dynapro input port: %s\n",
	       XCONFIG_GIVEN, priv->input_dev);
      }
      break;

    case DYNA_DEVICENAME:
      if (xf86GetToken(NULL) != STRING) {
	xf86ConfigError("Dynapro device name expected");
      }
      local->name = strdup(val->str);
      if (xf86Verbose) {
	ErrorF("%s Dynapro X device name: %s\n",
	       XCONFIG_GIVEN, local->name);
      }
      break;

    case SCREEN_NO:
      if (xf86GetToken(NULL) != NUMBER) {
	xf86ConfigError("Dynapro screen number expected");
      }
      priv->screen_num = val->num;
      if (xf86Verbose) {
	ErrorF("%s Dynapro associated screen: %d\n",
	       XCONFIG_GIVEN, priv->screen_num);
      }
      break;

    case LINK_SPEED:
      {
	int	ltoken = xf86GetToken(LinkSpeedTab);
	if (ltoken == EOF ||
	    ltoken == STRING ||
	    ltoken == NUMBER) {
	  xf86ConfigError("Dynapro link speed expected");
	}
	priv->link_speed = LinkSpeedValues[ltoken-1].speed;
	if (xf86Verbose) {
	  ErrorF("%s Dynapro link speed: %s bps\n",
		 XCONFIG_GIVEN, (LinkSpeedTab[ltoken-1].name)+1);
	}
      }
      break;
      
    case MAXX:
      if (xf86GetToken(NULL) != NUMBER) {
        xf86ConfigError("Dynapro maximum x position expected");
      }
      priv->max_x = val->num;
      if (xf86Verbose) {
	ErrorF("%s Dynapro maximum x position: %d\n",
	       XCONFIG_GIVEN, priv->max_x);
      }
     break;
      
    case MAXY:
      if (xf86GetToken(NULL) != NUMBER) {
        xf86ConfigError("Dynapro maximum y position expected");
      }
      priv->max_y = val->num;
      if (xf86Verbose) {
	ErrorF("%s Dynapro maximum y position: %d\n",
	       XCONFIG_GIVEN, priv->max_y);
      }
     break;
      
    case MINX:
      if (xf86GetToken(NULL) != NUMBER) {
        xf86ConfigError("Dynapro minimum x position expected");
      }
      priv->min_x = val->num;
      if (xf86Verbose) {
	ErrorF("%s Dynapro minimum x position: %d\n",
	       XCONFIG_GIVEN, priv->min_x);
      }
     break;
      
    case MINY:
      if (xf86GetToken(NULL) != NUMBER) {
        xf86ConfigError("Dynapro minimum y position expected");
      }
      priv->min_y = val->num;
      if (xf86Verbose) {
	ErrorF("%s Dynapro minimum y position: %d\n",
	       XCONFIG_GIVEN, priv->min_y);
      }
     break;
      
    case HISTORY_SIZE:
      if (xf86GetToken(NULL) != NUMBER) {
	xf86ConfigError("Dynapro motion history size expected");
      }
      local->history_size = val->num;
      if (xf86Verbose) {
	ErrorF("%s Dynapro motion history size is %d\n", XCONFIG_GIVEN,
	       local->history_size);
      }
      break;
	    
    case ALWAYS_CORE:
      xf86AlwaysCore(local, TRUE);
      if (xf86Verbose) {
	ErrorF("%s Dynapro device will always stay core pointer\n",
	       XCONFIG_GIVEN);
      }
      break;

    case BUTTON_NO:
      if (xf86GetToken(NULL) != NUMBER) {
	xf86ConfigError("Dynapro button number expected");
      }
      priv->button_number = val->num;
      if (xf86Verbose) {
	ErrorF("%s Dynapro button number: %d\n",
	       XCONFIG_GIVEN, priv->button_number);
      }
      break;

    case SWAP_XY:
      priv->swap_xy=1;
      if (xf86Verbose) {
	ErrorF("%s Dynapro device will swap X and Y dimensions\n",
	      XCONFIG_GIVEN);
      }
      break;

    case EOF:
      FatalError("Unexpected EOF (missing EndSubSection)");
      break;

    default:
      xf86ConfigError("Dynapro subsection keyword expected");
      break;
    }
  }

  if (priv->max_x - priv->min_x <= 0) {
    ErrorF("%s Dynapro: swap x mode (minimum x position >= maximum x position)\n",
	   XCONFIG_GIVEN);
  }  
  if (priv->max_y - priv->min_y <= 0) {
    ErrorF("%s Dynapro: swap y mode (minimum y position >= maximum y position)\n",
	   XCONFIG_GIVEN, priv->max_y, priv->min_y);
  }
  
  return Success;
}

/*
 ***************************************************************************
 *
 * xf86DynaproAllocate --
 *
 ***************************************************************************
 */
static LocalDevicePtr
xf86DynaproAllocate(void)
{
  LocalDevicePtr        local = (LocalDevicePtr) xalloc(sizeof(LocalDeviceRec));
  DynaproPrivatePtr         priv = (DynaproPrivatePtr) xalloc(sizeof(DynaproPrivateRec));

  if (!local) {
    if (priv) {
      xfree(priv);
    }
    return NULL;
  }
  if (!priv) {
    if (local) {
      xfree(local);
    }
    return NULL;
  }
  
  priv->input_dev = DYNAPRO_PORT;
  priv->link_speed = DYNAPRO_LINK_SPEED;
  priv->min_x = 1000;
  priv->max_x = 0;
  priv->min_y = 0;
  priv->max_y = 1000;
  priv->screen_num = 0;
  priv->screen_width = -1;
  priv->screen_height = -1;
  priv->lex_mode = Dynapro_byte0;
  priv->swap_xy = 0;
  priv->button_down = FALSE;
  priv->button_number = 1;
  priv->proximity = FALSE;
  local->name = XI_TOUCHSCREEN;
  local->flags = XI86_NO_OPEN_ON_INIT;
  local->device_config = xf86DynaproConfig;
  local->device_control = xf86DynaproControl;
  local->read_input   = xf86DynaproReadInput;
  local->control_proc = NULL;
  local->close_proc   = NULL;
  local->switch_mode  = NULL;
  local->conversion_proc = xf86DynaproConvert;
  local->reverse_conversion_proc = NULL;
  local->fd	      = -1;
  local->atom	      = 0;
  local->dev	      = NULL;
  local->private      = priv;
  local->type_name    = "Dynapro TouchScreen";
  local->history_size = 0;
  
  return local;
}


/*
 ***************************************************************************
 *
 * Dynapro device association --
 *
 ***************************************************************************
 */
DeviceAssocRec dynapro_assoc =
{
  "dynapro",                /* config_section_name */
  xf86DynaproAllocate               /* device_allocate */
};

#ifdef DYNAMIC_MODULE
/*
 ***************************************************************************
 *
 * entry point of dynamic loading
 *
 ***************************************************************************
 */

#ifndef DLSYM_BUG
init_module(unsigned long	server_version)
#else
init_xf86Dynapro(unsigned long      server_version)
#endif
{
    xf86AddDeviceAssoc(&dynapro_assoc);

    if (server_version != XF86_VERSION_CURRENT) {
      ErrorF("Warning: Dynapro module compiled for version%s\n", XF86_VERSION);
      return 0;
    }
    else {
      return 1;
    }
}
#endif




static Bool
xf86DynaproControl (DeviceIntPtr dev, int mode)
{
	LocalDevicePtr pInfo = dev->public.devicePrivate;
	DynaproPrivatePtr priv = (DynaproPrivatePtr) (pInfo->private);
	unsigned char map[] =
	{0, 1};
	
	
	switch (mode)
	{
	case DEVICE_INIT:
		/*
		 * these have to be here instead of in the SetupProc, because when the
		 * SetupProc is run at server startup, screenInfo is not setup yet
		 */
		priv->screen_width = screenInfo.screens[priv->screen_num]->width;
		priv->screen_height = screenInfo.screens[priv->screen_num]->height;
		
		/*
		 * Device reports button press for 1 button.
		 */
		if (InitButtonClassDeviceStruct (dev, 1, map) == FALSE)
			{
				ErrorF ("Unable to allocate Dynapro ButtonClassDeviceStruct\n");
				return !Success;
			}
		
		/*
		 * Device reports motions on 2 axes in absolute coordinates.
		 * Axes min and max values are reported in raw coordinates.
		 */
		if (InitValuatorClassDeviceStruct (dev, 2, xf86GetMotionEvents,
						   pInfo->history_size, Absolute) == FALSE)
			{
				ErrorF ("Unable to allocate Dynapro ValuatorClassDeviceStruct\n");
				return !Success;
			}
		else
			{
				InitValuatorAxisStruct (dev, 0, priv->min_x, priv->max_x,
							9500,
							0 /* min_res */ ,
							9500 /* max_res */ );
				InitValuatorAxisStruct (dev, 1, priv->min_y, priv->max_y,
							10500,
							0 /* min_res */ ,
							10500 /* max_res */ );
			}
		
		if (InitProximityClassDeviceStruct (dev) == FALSE)
			{
				ErrorF ("unable to allocate Dynapro ProximityClassDeviceStruct\n");
				return !Success;
			}
		
		/*
		 * Allocate the motion events buffer.
		 */
		xf86MotionHistoryAllocate (pInfo);

		AssignTypeAndName(dev, pInfo->atom, pInfo->name);
		return (Success);
		
	case DEVICE_ON:
		if (pInfo->fd == -1) {
			struct termios termios_tty;
			int	     i, result;

			SYSCALL(pInfo->fd = open(priv->input_dev, O_RDWR|O_NDELAY, 0));
			if (pInfo->fd < 0) {
				Error("Unable to open Dynapro touchscreen device");
				return !Success;
			}
			
			memset(&termios_tty, 0, sizeof(termios_tty));
			termios_tty.c_cflag = priv->link_speed | CS8 | CREAD | CLOCAL;
			termios_tty.c_cc[VMIN] = 1;
			
			SYSCALL(result = tcsetattr(pInfo->fd, TCSANOW, &termios_tty));
			if (result < 0) {
				Error("Unable to configure Dynapro touchscreen port");
				SYSCALL(close(pInfo->fd));
				pInfo->fd = -1;
				return !Success;
			}
			
			xf86FlushInput(pInfo->fd);
			AddEnabledDevice (pInfo->fd);
			dev->public.on = TRUE;
		}
		return (Success);
		
	case DEVICE_OFF:
	case DEVICE_CLOSE:
		if (pInfo->fd != -1)
			{ 
				RemoveEnabledDevice (pInfo->fd);
				SYSCALL(close(pInfo->fd));
				pInfo->fd = -1;
			}
		dev->public.on = FALSE;
		return (Success);
	default:
		return (BadValue);
	}

}


/* 
 * The ReadInput function will have to be tailored to your device
 */
static void
xf86DynaproReadInput (LocalDevicePtr pInfo)
{
	DynaproPrivatePtr priv = (DynaproPrivatePtr) (pInfo->private);
	int x,y;

	while (DynaproGetPacket (pInfo) == Success)
	{
		if (priv->swap_xy) {
			y = priv->packet[1] | ((priv->packet[0] & 0x38) << 4);
			x = priv->packet[2] | ((priv->packet[0] & 0x07) << 7);
		} else {
		x = priv->packet[1] | ((priv->packet[0] & 0x38) << 4);
		y = priv->packet[2] | ((priv->packet[0] & 0x07) << 7);
		}
				    
		if ((priv->proximity == FALSE) && (priv->packet[0] & 0x40))
		{
			priv->proximity = TRUE;
			xf86PostProximityEvent (pInfo->dev, 1, 0, 2, x, y);
		}              	
		
             /*
                 * Send events.
                 *
                 * We *must* generate a motion before a button change if pointer
                 * location has changed as DIX assumes this. This is why we always
                 * emit a motion, regardless of the kind of packet processed.
                 */

                xf86PostMotionEvent (pInfo->dev, TRUE, 0, 2, x, y);

                /*
                 * Emit a button press or release.
                 */
                if ((priv->button_down == FALSE) && (priv->packet[0] & 0x40))

                {
                        xf86PostButtonEvent (pInfo->dev, TRUE,
					     priv->button_number, 1, 0, 2, x, y);
                        priv->button_down = TRUE;
                }
                if ((priv->button_down == TRUE) && !(priv->packet[0] & 0x40))
                {
                        xf86PostButtonEvent (pInfo->dev, TRUE,
					     priv->button_number, 0, 0, 2, x, y);
                        priv->button_down = FALSE;
                }
                /*
                 * the untouch should always come after the button release
                 */
                if ((priv->proximity == TRUE) && !(priv->packet[0] & 0x40))
                {
                        priv->proximity = FALSE;
                        xf86PostProximityEvent (pInfo->dev, 0, 0, 2, x, y);
                }
	}
}



/* 
 * The ConvertProc function may need to be tailored for your device.
 * This function converts the device's valuator outputs to x and y coordinates
 * to simulate mouse events.
 */
static Bool
xf86DynaproConvert (LocalDevicePtr pInfo,
			 int first,
			 int num,
			 int v0,
			 int v1,
			 int v2,
			 int v3,
			 int v4,
			 int v5,
			 int *x,
			 int *y)
{
	DynaproPrivatePtr priv = (DynaproPrivatePtr) (pInfo->private);
  int		width = priv->max_x - priv->min_x;
  int		height = priv->max_y - priv->min_y;

  if (first != 0 || num != 2) {
    return FALSE;
  }
    
  *x = (priv->screen_width * (v0 - priv->min_x)) / width;
  *y = (priv->screen_height -
	(priv->screen_height * (v1 - priv->min_y)) / height);

  return (TRUE);
}


/* 
 * This function should be renamed for your device and tailored to handle
 * your device's protocol.
 */
static Bool
DynaproGetPacket (LocalDevicePtr pInfo)
{
	DynaproPrivatePtr priv = (DynaproPrivatePtr) pInfo->private;

	int count = 0;
	int c;
	fd_set fds;
	struct timeval tm = {0,0};
	FD_ZERO(&fds);
	FD_SET(pInfo->fd, &fds);
	
	while (select(pInfo->fd + 1, &fds, NULL, NULL, &tm) > 0 &&
	       read(pInfo->fd, &c, 1) >= 0)
	{

		/* 
		 * fail after 500 bytes so the server doesn't hang forever if a
		 * device sends bad data.
		 */
		if (count++ > 500)
			return (!Success);

		switch (priv->lex_mode)
		{
		case Dynapro_byte0:
			if (c & 0x80) {
				priv->packet[0] = (unsigned char) c;
				priv->lex_mode = Dynapro_byte1;
			}
			break;

		case Dynapro_byte1:
			if (!(c & 0x80)) {
				priv->packet[1] = (unsigned char) c;
				priv->lex_mode = Dynapro_byte2;
			}
			else {
				priv->lex_mode = Dynapro_byte1;
				priv->packet[0] = (unsigned char) c;
			}
			break;
			
		case Dynapro_byte2:
			if (!(c & 0x80)) {
				priv->packet[2] = (unsigned char) c;
				priv->lex_mode = Dynapro_byte0;
				return (Success);
			}
			else {
				priv->lex_mode = Dynapro_byte1;
				priv->packet[0] = (unsigned char) c;
			}
			break;
		}
	}
	return (!Success);
}

                 

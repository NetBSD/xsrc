/*
 * Copyright 1997-1998 by Frederic Lepied, France. <Frederic.Lepied@sugix.frmug.org>       
 *                                                                            
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is  hereby granted without fee, provided that
 * the  above copyright   notice appear  in   all  copies and  that both  that
 * copyright  notice   and   this  permission   notice  appear  in  supporting
 * documentation, and that   the  name of  Frederic   Lepied not  be  used  in
 * advertising or publicity pertaining to distribution of the software without
 * specific,  written      prior  permission.     Frederic  Lepied   makes  no
 * representations about the suitability of this software for any purpose.  It
 * is provided "as is" without express or implied warranty.                   
 *                                                                            
 * FREDERIC  LEPIED DISCLAIMS ALL   WARRANTIES WITH REGARD  TO  THIS SOFTWARE,
 * INCLUDING ALL IMPLIED   WARRANTIES OF MERCHANTABILITY  AND   FITNESS, IN NO
 * EVENT  SHALL FREDERIC  LEPIED BE   LIABLE   FOR ANY  SPECIAL, INDIRECT   OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA  OR PROFITS, WHETHER  IN  AN ACTION OF  CONTRACT,  NEGLIGENCE OR OTHER
 * TORTIOUS  ACTION, ARISING    OUT OF OR   IN  CONNECTION  WITH THE USE    OR
 * PERFORMANCE OF THIS SOFTWARE.
 *
 */

/* $XFree86: xc/programs/Xserver/hw/xfree86/common/xf86Switch.c,v 3.4.2.1 1998/10/11 12:35:42 hohndel Exp $ */

#define NEED_EVENTS
#include "X.h"
#include "Xproto.h"
#include "misc.h"
#include "inputstr.h"
#include "scrnintstr.h"
#include "XI.h"
#include "XIproto.h"

#if !defined(sun) || defined(i386)
#include "compiler.h"

#include "xf86.h"
#include "xf86Procs.h"
#include "xf86_OSlib.h"
#include "xf86_Config.h"
#include "xf86Xinput.h"
#include "atKeynames.h"
#include "xf86Version.h"

#include "osdep.h"
#else
#include "extio.h"
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

/******************************************************************************
 * device records
 *****************************************************************************/

typedef struct 
{
  XID	last;			/* last core pointer */
} SwitchDevRec, *SwitchDevPtr;

/******************************************************************************
 * external declarations
 *****************************************************************************/

extern void xf86eqEnqueue(
#if NeedFunctionPrototypes
    xEventPtr /*e*/
#endif
);

extern void miPointerDeltaCursor(
#if NeedFunctionPrototypes
    int /*dx*/,
    int /*dy*/,
    unsigned long /*time*/
#endif
);

extern int xf86SwitchGetState(
#ifdef NeedFunctionPrototypes
    int   /*fd*/,
    int * /*x*/,
    int * /*y*/,
    int * /*buttons*/
#endif
    );

extern void xf86SwitchInit(
#ifdef NeedFunctionPrototypes
void
#endif
);

extern int xf86SwitchOff(
#ifdef NeedFunctionPrototypes
int * /*fd*/,
int /*doclose*/
#endif
);

extern int xf86SwitchOn(
#ifdef NeedFunctionPrototypes
char * /*name*/,
int * /*timeout*/,
int * /*centerX*/,
int * /*centerY*/
#endif
);

/*
 ***************************************************************************
 *
 * xf86SwtConvert --
 *	Convert valuators to X and Y.
 *
 ***************************************************************************
 */
static Bool
xf86SwtConvert(LocalDevicePtr	local,
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
  return FALSE;
}

/*
 * xf86SwitchCoreDevice --
 *      Test if the core device has changed and send a motion event accordingly.
 */
void
xf86SwitchCoreDevice(LocalDevicePtr	local,
		     DeviceIntPtr	core)
{
  SwitchDevPtr        priv;

  if (!local)
      return;
  
  priv = (SwitchDevPtr) local->private;

  if (core->id != priv->last) {
    DBG(3, ErrorF("xf86SwitchCoreDevice new core id=%d old=%d\n", core->id, priv->last));
    priv->last = core->id;
    xf86PostMotionEvent(local->dev, 1, 0, 1, core->id);
  }
}

static void
xf86SwtControlProc(DeviceIntPtr	device,
                    PtrCtrl		*ctrl)
{
  DBG(2, ErrorF("xf86SwtControlProc\n"));
}

/*
 * xf86SwtProc --
 *      Handle the initialization, etc. of a switch
 */
static int
xf86SwtProc(pSwt, what)
     DeviceIntPtr       pSwt;
     int                what;
{
  int			loop;
  int                   nbaxes;
  LocalDevicePtr        local = (LocalDevicePtr)pSwt->public.devicePrivate;
  SwitchDevPtr		priv = (SwitchDevPtr)PRIVATE(pSwt);

  DBG(2, ErrorF("BEGIN xf86SwtProc dev=0x%x priv=0x%x\n", pSwt, priv));
  
  switch (what)
    {
    case DEVICE_INIT: 
      DBG(1, ErrorF("xf86SwtProc pSwt=0x%x what=INIT\n", pSwt));
  
      nbaxes = 1;

      if (InitFocusClassDeviceStruct(pSwt) == FALSE)
        {
          ErrorF("unable to init Focus class device\n");
          return !Success;
        }
          
      if (InitValuatorClassDeviceStruct(pSwt, 
					nbaxes,
					xf86GetMotionEvents, 
					local->history_size,
					Absolute) /* relatif ou absolute */
          == FALSE) 
        {
          ErrorF("unable to allocate Valuator class device\n"); 
          return !Success;
        }
      else 
        {
          for(loop=0; loop<nbaxes; loop++) {
            InitValuatorAxisStruct(pSwt,
                                   loop,
                                   0, /* min val */
                                   1000, /* max val */
                                   1); /* resolution */
          }
	  /* allocate the motion history buffer if needed */
	  xf86MotionHistoryAllocate(local);

          AssignTypeAndName(pSwt, local->atom, local->name);
        }

      break; 
      
    case DEVICE_ON:
      DBG(1, ErrorF("xf86SwtProc  pSwt=0x%x what=ON\n", pSwt));
      pSwt->public.on = TRUE;
    break;
      
    case DEVICE_OFF:
    case DEVICE_CLOSE:
      DBG(1, ErrorF("xf86SwtProc  pSwt=0x%x what=%s\n", pSwt,
                    (what == DEVICE_CLOSE) ? "CLOSE" : "OFF"));
      pSwt->public.on = FALSE;
    break;

    default:
      ErrorF("unsupported mode=%d\n", what);
      return !Success;
      break;
    }
  DBG(2, ErrorF("END   xf86SwtProc dev=0x%x priv=0x%x\n", pSwt, priv));
  return Success;
}

/*
 * xf86SwtAllocate --
 *      Allocate Switch device structures.
 */
static LocalDevicePtr
xf86SwtAllocate()
{
  LocalDevicePtr        local = (LocalDevicePtr) xalloc(sizeof(LocalDeviceRec));
  SwitchDevPtr		priv = (SwitchDevPtr) xalloc(sizeof(SwitchDevRec));
  
  local->name = "SWITCH";
  local->flags = 0;
#if !defined(sun) || defined(i386)
  local->device_config = NULL;
#endif
  local->device_control = xf86SwtProc;
  local->read_input = NULL;
  local->close_proc = NULL;
  local->control_proc = NULL;
  local->switch_mode = NULL;
  local->conversion_proc = xf86SwtConvert;
  local->fd = -1;
  local->atom = 0;
  local->dev = NULL;
  local->private = priv;
  local->type_name = "Switch";
  local->history_size  = 0;
  
  priv->last = -1;
  
  return local;
}

/*
 * switch association
 */
DeviceAssocRec switch_assoc =
{
  " ",                   /* config_section_name */
  xf86SwtAllocate              /* device_allocate */
};

/* end of xf86Switch.c */

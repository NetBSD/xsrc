/* 
 * Copyright (c) 1998  Metro Link Incorporated
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
/* $XFree86: xc/programs/Xserver/hw/xfree86/input/magellan/magellan.c,v 1.9 1999/06/05 15:55:25 dawes Exp $ */

#define _MAGELLAN_C_
/*****************************************************************************
 *	Standard Headers
 ****************************************************************************/

#include <misc.h>
#include <xf86.h>
#define NEED_XF86_TYPES
#include <xf86_ansic.h>
#include <xf86_OSproc.h>
#include <xf86Xinput.h>
#include <xisb.h>
#include <exevents.h>			/* Needed for InitValuator/Proximity stuff	*/

/*****************************************************************************
 *	Local Headers
 ****************************************************************************/
#include "magellan.h"

/*****************************************************************************
 *	Variables without includable headers
 ****************************************************************************/

/*****************************************************************************
 *	Local Variables
 ****************************************************************************/
static XF86ModuleVersionInfo VersionRec =
{
	"magellan",
	MODULEVENDORSTRING,
	MODINFOSTRING1,
	MODINFOSTRING2,
	XF86_VERSION_CURRENT,
	1, 0, 0,
	ABI_CLASS_XINPUT,
	ABI_XINPUT_VERSION,
	MOD_CLASS_XINPUT,
	{0, 0, 0, 0}				/* signature, to be patched into the file by
								 * a tool */
};

/* 
 * Be sure to set vmin appropriately for your device's protocol. You want to
 * read a full packet before returning
 */
static const char *default_options[] =
{
	"BaudRate", "9600",
	"StopBits", "2",
	"DataBits", "8",
	"Parity", "None",
	"Vmin", "26",
	"Vtime", "1",
	"FlowControl", "None"
};

XF86ModuleData magellanModuleData = { &VersionRec, SetupProc, TearDownProc };

/*****************************************************************************
 *	Function Definitions
 ****************************************************************************/


/* 
 * The TearDownProc may have to be tailored to your device
 */
static void
TearDownProc( pointer p )
{
	LocalDevicePtr local = (LocalDevicePtr) p;
	MagellanPrivatePtr priv = (MagellanPrivatePtr) local->private;

	DeviceOff (local->dev);

	xf86RemoveLocalDevice (local);

	xf86CloseSerial (local->fd);
	XisbFree (priv->buffer);
	xfree (priv);
	xfree (local->name);
	xfree (local);
}

static pointer
SetupProc(	pointer module,
			pointer options,
			int *errmaj,
			int *errmin )
{
	LocalDevicePtr local = xcalloc (1, sizeof (LocalDeviceRec));
	MagellanPrivatePtr priv = xcalloc (1, sizeof (MagellanPrivateRec));
	pointer	defaults,
			merged;

	if ((!local) || (!priv))
		goto SetupProc_fail;

	defaults = xf86OptionListCreate (default_options,
				  (sizeof (default_options) / sizeof (default_options[0])), 0);
	merged = xf86OptionListMerge( defaults, options );

	xf86OptionListReport( merged );

	local->fd = xf86OpenSerial (merged);
	if (local->fd == -1)
	{
		ErrorF ("Magellan driver unable to open device\n");
		*errmaj = LDR_NOPORTOPEN;
		*errmin = xf86GetErrno ();
		goto SetupProc_fail;
	}

	priv->buffer = XisbNew (local->fd, 200);

	DBG (9, XisbTrace (priv->buffer, 1));

	/* 
	 * Verify that hardware is attached and fuctional
	 */
	if (QueryHardware (priv, errmaj, errmin) != Success)
	{
		ErrorF ("Unable to query/initialize Magellan hardware.\n");
		goto SetupProc_fail;
	}

	/* this results in an xstrdup that must be freed later */
	local->name = xf86SetStrOption( merged, "DeviceName", "Magellan Space Mouse" );

	local->type_name = XI_SPACEBALL;
	/* 
	 * Standard setup for the local device record
	 */
	local->device_control = DeviceControl;
	local->read_input = ReadInput;
	local->control_proc = ControlProc;
	local->close_proc = CloseProc;
	local->switch_mode = SwitchMode;
	local->conversion_proc = ConvertProc;
	local->dev = NULL;
	local->private = priv;
	local->private_flags = 0;
	local->history_size = xf86SetIntOption( merged, "HistorySize", 0);

	xf86AddLocalDevice (local, merged);

	/* return the LocalDevice */
	return (local);

	/* 
	 * If something went wrong, cleanup and return NULL
	 */
  SetupProc_fail:
	if ((local) && (local->fd))
		xf86CloseSerial (local->fd);
	if ((local) && (local->name))
		xfree (local->name);
	if (local)
		xfree (local);

	if ((priv) && (priv->buffer))
		XisbFree (priv->buffer);
	if (priv)
		xfree (priv);
	return (NULL);
}

static Bool
DeviceControl (DeviceIntPtr dev, int mode)
{
	Bool	RetValue;

	switch (mode)
	{
	case DEVICE_INIT:
		DeviceInit (dev);
		RetValue = Success;
		break;
	case DEVICE_ON:
		RetValue = DeviceOn( dev );
		break;
	case DEVICE_OFF:
		RetValue = DeviceOff( dev );
		break;
	case DEVICE_CLOSE:
		RetValue = DeviceClose( dev );
		break;
	default:
		RetValue = BadValue;
	}

	return( RetValue );
}

static Bool
DeviceOn (DeviceIntPtr dev)
{
	LocalDevicePtr local = (LocalDevicePtr) dev->public.devicePrivate;

	AddEnabledDevice (local->fd);
	dev->public.on = TRUE;
	return (Success);
}

static Bool
DeviceOff (DeviceIntPtr dev)
{
	LocalDevicePtr local = (LocalDevicePtr) dev->public.devicePrivate;

	RemoveEnabledDevice (local->fd);
	dev->public.on = FALSE;
	return (Success);
}

static Bool
DeviceClose (DeviceIntPtr dev)
{
	return (Success);
}

static Bool
DeviceInit (DeviceIntPtr dev)
{
	LocalDevicePtr local = (LocalDevicePtr) dev->public.devicePrivate;
	unsigned char map[] =
	{0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
	int i;

	if (InitButtonClassDeviceStruct (dev, 9, map) == FALSE)
	{
		ErrorF ("Unable to allocate Magellan ButtonClassDeviceStruct\n");
		return !Success;
	}

	if (InitFocusClassDeviceStruct (dev) == FALSE)
	{
		ErrorF("Unable to allocate Magellan FocusClassDeviceStruct\n");
		return !Success;
    }

	if (InitValuatorClassDeviceStruct (dev, 6, xf86GetMotionEvents,
									local->history_size, Absolute) == FALSE)
	{
		ErrorF ("Unable to allocate Magellan ValuatorClassDeviceStruct\n");
		return !Success;
	}
	else
	{
		for (i = 0; i <= 6; i++)
		{
			InitValuatorAxisStruct(dev, i, MAGELLAN_MIN, MAGELLAN_MAX,
				MAGELLAN_RES, 0, MAGELLAN_RES);
		}
	}

#ifdef BELL_FEEDBACK_SUPPORT
	/*
	The InitBellFeedbackClassDeviceStruct function is not exported in the
	4.3.0 or 4.3.1 Xmetro loader. We'll leave this out to stay compatible
	*/

	if (InitBellFeedbackClassDeviceStruct (dev, MagellanBellSound,
		MagellanBellCtrl) == FALSE)
	{
		ErrorF ("Unable to allocate Magellan BellFeedbackClassDeviceStruct\n");
		return !Success;
	}
#endif

	/* 
	 * Allocate the motion events buffer.
	 */
	xf86MotionHistoryAllocate (local);
	return (Success);
}

static void
ReadInput (LocalDevicePtr local)
{
	int x, y, z;
	int a, b, c;
	int i, buttons;
	MagellanPrivatePtr priv = (MagellanPrivatePtr) (local->private);

	/* 
	 * set blocking to -1 on the first call because we know there is data to
	 * read. Xisb automatically clears it after one successful read so that
	 * succeeding reads are preceeded buy a select with a 0 timeout to prevent
	 * read from blocking indefinately.
	 */
	XisbBlockDuration (priv->buffer, -1);
	while (MagellanGetPacket (priv) == Success)
	{
		/* 
		 * Examine priv->packet and call these functions as appropriate:
		 *
		 xf86PostMotionEvent
		 xf86PostButtonEvent
		 */

		switch (priv->packet[0])
		{
			case 'd':	/* motion packet */
				if (strlen (priv->packet) == 26)
				{
					x = 
					MagellanNibble( priv->packet[1] ) * 4096 +
					MagellanNibble( priv->packet[2] ) * 256 +
					MagellanNibble( priv->packet[3] ) * 16 +
					MagellanNibble( priv->packet[4] ) - 32768;
					y = 
					MagellanNibble( priv->packet[5] ) * 4096 +
					MagellanNibble( priv->packet[6] ) * 256 +
					MagellanNibble( priv->packet[7] ) * 16 +
					MagellanNibble( priv->packet[8] ) - 32768;
					z = 
					MagellanNibble( priv->packet[9] ) * 4096 +
					MagellanNibble( priv->packet[10] ) * 256 +
					MagellanNibble( priv->packet[11] ) * 16 +
					MagellanNibble( priv->packet[12] ) - 32768;

					a =
					MagellanNibble( priv->packet[13] ) * 4096 +
					MagellanNibble( priv->packet[14] ) * 256 +
					MagellanNibble( priv->packet[15] ) * 16 +
					MagellanNibble( priv->packet[16] ) - 32768;
					b = 
					MagellanNibble( priv->packet[17] ) * 4096 +
					MagellanNibble( priv->packet[18] ) * 256 +
					MagellanNibble( priv->packet[19] ) * 16 +
					MagellanNibble( priv->packet[20] ) - 32768;
					c = 
					MagellanNibble( priv->packet[21] ) * 4096 +
					MagellanNibble( priv->packet[22] ) * 256 +
					MagellanNibble( priv->packet[23] ) * 16 +
					MagellanNibble( priv->packet[24] ) - 32768;

					xf86ErrorFVerb( 5, "Magellan motion %d %d %d -- %d %d %d\n",
										x, y, z, a, b, c );
					xf86PostMotionEvent(local->dev, TRUE, 0, 6,
						x, y, z, a, b, c);
				}
				else
					ErrorF ("Magellan recieved a short \'d\'packet\n");
			break;

			case 'k': /* button packet */
				if (strlen (priv->packet) == 5)
				{
				buttons = MagellanNibble( priv->packet[1] ) * 1 +
                          MagellanNibble( priv->packet[2] ) * 16 +
                          MagellanNibble( priv->packet[3] ) * 256; 
				if (priv->old_buttons != buttons)
					for (i = 0; i < 9; i++)
					{	
						if ((priv->old_buttons&(1<<i)) != (buttons&(1<<i)))
						{
							xf86PostButtonEvent(local->dev, FALSE, i+1,
								(buttons&(1<<i)), 0, 0);
				xf86ErrorFVerb( 5, "Magellan setting button %d to %d\n",
					i+1, (buttons&(1<<i)) );
						}
					}
				priv->old_buttons = buttons;
				}
				else
					ErrorF ("Magellan recieved a short \'k\'packet\n");
			break;
		}
	}
}

static int
ControlProc (LocalDevicePtr local, xDeviceCtl * control)
{
	return (Success);
}

#ifdef BELL_FEEDBACK_SUPPORT
/*
The bell functions are stubbed out for now because they can't be used with the
4.3.0 and 4.3.1 Xmetro binaries. The device can only control the duration of
the beep.
*/
static void
MagellanBellCtrl(DeviceIntPtr dev, BellCtrl *ctrl)
{
}

static void
MagellanBellSound(int percent, DeviceIntPtr dev, pointer ctrl, int unknown)
{
}
#endif

static void
CloseProc (LocalDevicePtr local)
{
}

static int
SwitchMode (ClientPtr client, DeviceIntPtr dev, int mode)
{
	return (Success);
}

/* 
 * The ConvertProc function may need to be tailored for your device.
 * This function converts the device's valuator outputs to x and y coordinates
 * to simulate mouse events.
 */
static Bool
ConvertProc (LocalDevicePtr local,
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

	*x = v3;
	*y = v4;
	return (Success);
}

#define WriteString(str)\
XisbWrite (priv->buffer, (unsigned char *)(str), strlen(str)); \
	XisbBlockDuration (priv->buffer, 1000000); \
	if ((MagellanGetPacket (priv) != Success) || \
		(strcmp (priv->packet, (str)) != 0)) \
			return (!Success);


static Bool
QueryHardware (MagellanPrivatePtr priv, int *errmaj, int *errmin)
{
	*errmaj = LDR_NOHARDWARE;

	/* the device resets when the port is opened. Give it time to finish */
	milisleep (1000);

	XisbWrite (priv->buffer, (unsigned char *)MagellanAttention, strlen(MagellanAttention));
	WriteString (MagellanInitString);
	WriteString (MagellanInitString);
	WriteString (MagellanSensitivity);
	WriteString (MagellanPeriod);
	WriteString (MagellanMode);
	WriteString (MagellanNullRadius);

	XisbWrite (priv->buffer, (unsigned char *)MagellanVersion, strlen(MagellanVersion));
	/* block for up to 1 second while trying to read the response */
	XisbBlockDuration (priv->buffer, 1000000);
	NewPacket (priv);
	
	if ((MagellanGetPacket (priv) == Success) && (priv->packet[0] == 'v'))
	{
		priv->packet[strlen(priv->packet) - 1] = '\0';
		xf86MsgVerb( X_PROBED, 3, " initialized: %s\n",  &(priv->packet[3]) );
	}
	else
		return (!Success);

	return (Success);
}

static void
NewPacket (MagellanPrivatePtr priv)
{
    priv->lex_mode = magellan_normal;
    priv->packeti = 0;
}

static Bool
MagellanGetPacket (MagellanPrivatePtr priv)
{
	int count = 0;
	int c;

	while ((c = XisbRead (priv->buffer)) >= 0)
	{
		/* 
		 * fail after 500 bytes so the server doesn't hang forever if a
		 * device sends bad data.
		 */
		if (count++ > 500)
		{
			NewPacket (priv);
			return (!Success);
		}

		switch (priv->lex_mode)
		{
		case magellan_normal:
			if (priv->packeti > MAGELLAN_PACKET_SIZE)
			{
				NewPacket (priv);
				return (!Success);
			}
			priv->packet[priv->packeti] = c;
			priv->packeti++;
			if (c == '\r')
			{
				priv->packet[priv->packeti] = '\0';
				NewPacket (priv);
				return (Success);
			}
			break;
		}
	}
	return (!Success);
}

/*
 * Copyright 1996, 1998 by Patrick Lecoanet, France. <lecoanet@cena.dgac.fr>
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

/* $XFree86: xc/programs/Xserver/hw/xfree86/common/xf86MuTouch.c,v 3.5.2.3 1998/11/12 11:32:04 dawes Exp $ */

/*
 *******************************************************************************
 *******************************************************************************
 *
 * This driver is able to deal with MicrotTouch serial controllers using
 * firmware set 2. This includes (but may not be limited to) Serial/SMT3
 * and TouchPen controllers. The only data format supported is Mode Tablet
 * as it is the only available with these controllers. Anyway this is not a big
 * lost as it is the most efficient (by far) and is supported by all controllers.
 *
 * The code has been lifted from the Elographics driver in xf86Elo.c.
 *
 *******************************************************************************
 *******************************************************************************
 */

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

#ifdef XFree86LOADER
#include "xf86_libc.h"                            
#endif
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


/*
 ***************************************************************************
 *
 * Configuration descriptor.
 *
 ***************************************************************************
 */
#define FINGER_SECTION_NAME	"microtouchfinger"
#define STYLUS_SECTION_NAME	"microtouchstylus"

#define PORT			1
#define DEVICENAME		2
#define SCREEN_NO		3
#define MAXX			5
#define MAXY			6
#define MINX			7
#define MINY			8
#define DEBUG_LEVEL		9
#define HISTORY_SIZE		10
#define LINK_SPEED		11
#define ALWAYS_CORE		12

static SymTabRec MuTTab[] = {
  { ENDSUBSECTION,     "endsubsection" },
  { PORT,              "port" },
  { DEVICENAME,        "devicename" },
  { SCREEN_NO,	       "screenno" },
  { MAXX,              "maximumxposition" },
  { MAXY,              "maximumyposition" },
  { MINX,              "minimumxposition" },
  { MINY,              "minimumyposition" },
  { DEBUG_LEVEL,       "debuglevel" },
  { HISTORY_SIZE,      "historysize" },
  { LINK_SPEED,        "linkspeed" },
  { ALWAYS_CORE,       "alwayscore" },
  { -1,                "" },
};

#define LS300		1
#define LS1200		2
#define LS2400		3
#define LS9600		4
#define LS19200		5

static SymTabRec LinkSpeedTab[] = {
  { LS300,	"b300" },
  { LS1200,	"b1200" },
  { LS2400,	"b2400" },
  { LS9600,	"b9600" },
  { LS19200,	"b19200" }
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
  { B300, 64 },
  { B1200, 16 },
  { B2400, 8 },
  { B9600, 4 },
  { B19200, 2 }
};


/*
 ***************************************************************************
 *
 * Default constants.
 *
 ***************************************************************************
 */
#define MuT_MAX_TRIALS		5	/* Number of timeouts waiting for a	*/
					/* pending reply.			*/
#define MuT_MAX_WAIT		300000	/* Max wait time for a reply (microsec) */
#define MuT_LINK_SPEED		B9600	/* 9600 Bauds				*/
#define MuT_PORT		"/dev/ttyS1"

#define DEFAULT_MAX_X		3000
#define DEFAULT_MIN_X		600
#define DEFAULT_MAX_Y		3000
#define DEFAULT_MIN_Y		600

#define XI_FINGER		"FINGER"	/* X device name for the finger device	*/
#define XI_STYLUS		"STYLUS"	/* X device name for the stylus device	*/


/*
 ***************************************************************************
 *
 * Protocol constants.
 *
 ***************************************************************************
 */
#define MuT_REPORT_SIZE		5	/* Size of a report packet.			*/
#define MuT_BUFFER_SIZE		256	/* Size of input buffer.			*/
#define MuT_PACKET_SIZE		10	/* Maximum size of a command/reply *including*	*/
					/* the leading and trailing bytes.		*/

#define MuT_LEAD_BYTE		0x01	/* First byte of a command/reply packet.	*/
#define MuT_TRAIL_BYTE		0x0D	/* Last byte of a command/reply packet.		*/

/*
 * Commands.
 */
#define MuT_RESET		"R"	/* Reset the controller.			*/
#define MuT_RESTORE_DEFAULTS	"RD"	/* Restore factory settings.			*/
#define MuT_FORMAT_TABLET	"FT"	/* Report events using tablet format.		*/
#define MuT_FORMAT_RAW		"FR"	/* Report events in raw mode (no corrections).	*/
#define MuT_CALIBRATE_RAW	"CR"	/* Calibration in raw mode.			*/ 
#define MuT_CALIBRATE_EXT	"CX"	/* Calibration in extended mode (cooked).	*/
#define MuT_OUTPUT_IDENT	"OI"	/* Ask some infos about the firmware.		*/
#define MuT_UNIT_TYPE		"UT"	/* Ask some more infos about the firmware.	*/
#define MuT_FINGER_ONLY		"FO"	/* Send reports only if a finger is touching.	*/
#define MuT_PEN_ONLY		"PO"	/* Send reports only if a pen is touching.	*/
#define MuT_PEN_FINGER		"PF"	/* Always send reports.				*/
#define MuT_MODE_STREAM		"MS"	/* Receive reports in stream mode (continuous).	*/

/*
 * Command reply values.
 */
#define MuT_OK			'0'	/* Report success.				*/
#define MuT_ERROR		'1'	/* Report error.				*/

/*
 * Offsets in status byte of touch and motion reports.
 */
#define MuT_SW1			0x01	/* State of switch 1 (TouchPen only).		*/
#define MuT_SW2			0x02	/* State of switch 2 (TouchPen only).		*/
#define MuT_WHICH_DEVICE	0x20	/* If report is from pen or from finger.	*/
#define MuT_CONTACT		0x40	/* Report touch/untouch with touchscreen.	*/

/*
 * Identity and friends.
 */
#define MuT_TOUCH_PEN_IDENT	"P5"
#define MuT_SMT3_IDENT		"Q1"


/*
 ***************************************************************************
 *
 * Usefull macros.
 *
 ***************************************************************************
 */
#define WORD_ASSEMBLY(byte1, byte2)	(((byte2) << 7) | (byte1))
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

  
/*
 ***************************************************************************
 *
 * Device private records.
 *
 ***************************************************************************
 */
#define FINGER_ID		1
#define STYLUS_ID		2
#define DEVICE_ID(flags)	((flags) & 0x03)
  
typedef struct _MuTPrivateRec {
  char			*input_dev;	/* The touchscreen input tty			*/
  int			min_x;		/* Minimum x reported by calibration		*/
  int			max_x;		/* Maximum x					*/
  int			min_y;		/* Minimum y reported by calibration		*/
  int			max_y;		/* Maximum y					*/
  int			link_speed;	/* Speed of the RS232 link connecting the ts.	*/

  int			screen_no;	/* Screen associated with the device		*/
  int			screen_width;	/* Width of the associated X screen		*/
  int			screen_height;	/* Height of the screen				*/
  Bool			inited;		/* The controller has already been configured ?	*/
  char			state;		/* Current state of report flags.		*/
  int			num_old_bytes;	/* Number of bytes left in receive buffer.	*/
  LocalDevicePtr	finger;		/* Finger device ptr associated with the hw.	*/
  LocalDevicePtr	stylus;		/* Stylus device ptr associated with the hw.	*/
  unsigned char		rec_buf[MuT_BUFFER_SIZE]; /* Receive buffer.			*/
} MuTPrivateRec, *MuTPrivatePtr;


/*
 ***************************************************************************
 *
 * xf86MuTConfig --
 *	Configure the driver from the configuration data.
 *
 ***************************************************************************
 */
static Bool
xf86MuTConfig(LocalDevicePtr    *array,
	      int               inx,
	      int               max,
	      LexPtr            val)
{
  LocalDevicePtr        local = array[inx];
  MuTPrivatePtr         priv = (MuTPrivatePtr)(local->private);
  int                   token;
  
  while ((token = xf86GetToken(MuTTab)) != ENDSUBSECTION) {
    switch(token) {
      
      case PORT:
	if (xf86GetToken(NULL) != STRING)
	  xf86ConfigError("MicroTouch input port expected");
	else {
	  /*
	   * See if another X device share the same physical
	   * device and set up the links so that they share
	   * the same private structure (the one that controls
	   * the physical device).
	   */
	  int	i;
	  for (i = 0; i < max; i++) {
	    if (i == inx)
	      continue;
	    if (array[i]->device_config == xf86MuTConfig &&
		(strcmp(((MuTPrivatePtr) array[i]->private)->input_dev,
			val->str) == 0)) {
	      ErrorF("%s MicroTouch config detected a device share between %s and %s\n",
		     XCONFIG_GIVEN, local->type_name, array[i]->name);
	      xfree(priv);
	      priv = local->private = array[i]->private;
	      switch (DEVICE_ID(local->private_flags)) {
	      case FINGER_ID:
		priv->finger = local;
		break;
	      case STYLUS_ID:
		priv->stylus = local;
		break;
	      }
	      break;
	    }
	  }
	  if (i == max) {
	    priv->input_dev = strdup(val->str);	
	    if (xf86Verbose)
	      ErrorF("%s MicroTouch %s input port: %s\n",
		     XCONFIG_GIVEN, local->type_name, priv->input_dev);
	  }
	}
	break;
	
    case DEVICENAME:
      if (xf86GetToken(NULL) != STRING)
	xf86ConfigError("MicroTouch device name expected");
      local->name = strdup(val->str);
      if (xf86Verbose)
	ErrorF("%s MicroTouch %s X device name: %s\n",
	       XCONFIG_GIVEN, local->type_name, local->name);
      break;
      
    case SCREEN_NO:
      if (xf86GetToken(NULL) != NUMBER)
	xf86ConfigError("MicroTouch screen number expected");
      priv->screen_no = val->num;
      if (xf86Verbose)
	ErrorF("%s MicroTouch %s associated screen: %d\n",
	       XCONFIG_GIVEN, local->type_name, priv->screen_no);      
      break;
      
    case LINK_SPEED:
      {
	int	ltoken = xf86GetToken(LinkSpeedTab);
	if (ltoken == EOF ||
	    ltoken == STRING ||
	    ltoken == NUMBER)
	  xf86ConfigError("MicroTouch link speed expected");
	priv->link_speed = LinkSpeedValues[ltoken-1].speed;
	if (xf86Verbose)
	  ErrorF("%s MicroTouch %s link speed: %s bps\n",
		 XCONFIG_GIVEN, local->type_name, (LinkSpeedTab[ltoken-1].name)+1);
      }
      break;
    
    case MAXX:
      if (xf86GetToken(NULL) != NUMBER)
        xf86ConfigError("MicroTouch maximum x position expected");
      priv->max_x = val->num;
      if (xf86Verbose)
	ErrorF("%s MicroTouch %s maximum x position: %d\n",
	       XCONFIG_GIVEN, local->type_name, priv->max_x);      
     break;
      
    case MAXY:
      if (xf86GetToken(NULL) != NUMBER)
        xf86ConfigError("MicroTouch maximum y position expected");
      priv->max_y = val->num;
      if (xf86Verbose)
	ErrorF("%s Microtouch %s maximum y position: %d\n",
	       XCONFIG_GIVEN, local->type_name, priv->max_y);      
     break;
      
    case MINX:
      if (xf86GetToken(NULL) != NUMBER)
        xf86ConfigError("MicroTouch minimum x position expected");
      priv->min_x = val->num;
      if (xf86Verbose)
	ErrorF("%s MicroTouch %s minimum x position: %d\n",
	       XCONFIG_GIVEN, local->type_name, priv->min_x);      
     break;
      
    case MINY:
      if (xf86GetToken(NULL) != NUMBER)
        xf86ConfigError("MicroTouch minimum y position expected");
      priv->min_y = val->num;
      if (xf86Verbose)
	ErrorF("%s MicroTouch %s minimum y position: %d\n",
	       XCONFIG_GIVEN, local->type_name, priv->min_y);      
     break;
      
    case DEBUG_LEVEL:
	if (xf86GetToken(NULL) != NUMBER)
	    xf86ConfigError("MicroTouch driver debug expected");
	debug_level = val->num;
	if (xf86Verbose) {
#if DEBUG
	    ErrorF("%s MicroTouch %s debug level sets to %d\n", XCONFIG_GIVEN,
		   local->type_name, debug_level);      
#else
	    ErrorF("%s MicroTouch %s debug not available\n",
		   XCONFIG_GIVEN, local->type_name, debug_level);      
#endif
	}
        break;

    case HISTORY_SIZE:
      if (xf86GetToken(NULL) != NUMBER)
	xf86ConfigError("MicroTouch motion history size expected");
      local->history_size = val->num;
      if (xf86Verbose)
	ErrorF("%s MicroTouch %s motion history size is %d\n", XCONFIG_GIVEN,
	       local->type_name, local->history_size);      
      break;
	    
    case ALWAYS_CORE:
	xf86AlwaysCore(local, TRUE);
	if (xf86Verbose)
	    ErrorF("%s MicroTouch %s device will always stays core pointer\n",
		   local->type_name, XCONFIG_GIVEN);
	break;

    case EOF:
      FatalError("Unexpected EOF (missing EndSubSection)");
      break;

    default:
      xf86ConfigError("MicroTouch subsection keyword expected");
      break;
    }
  }

  if (priv->max_x - priv->min_x <=0) {
    priv->max_x = DEFAULT_MAX_X;
    priv->min_x = DEFAULT_MIN_X;
    ErrorF("%s MicroTouch: Incorrect Maximum/Minimum x position, using: %d, %d\n",
	   XCONFIG_GIVEN, priv->max_x, priv->min_x);
  }  
  if (priv->max_y - priv->min_y <=0) {
    priv->max_y = DEFAULT_MAX_Y;
    priv->min_y = DEFAULT_MIN_Y;
    ErrorF("%s MicroTouch: Incorrect Maximum/Minimum y position, using: %d, %d\n",
	   XCONFIG_GIVEN, priv->max_y, priv->min_y);    
  }
  
  DBG(2, ErrorF("xf86MuTConfig port name=%s\n", priv->input_dev))

  return Success;
}


/*
 ***************************************************************************
 *
 * xf86MuTConvert --
 *	Convert extended valuators to x and y suitable for core motion
 *	events. Return True if ok and False if the requested conversion
 *	can't be done for the specified valuators.
 *
 ***************************************************************************
 */
static Bool
xf86MuTConvert(LocalDevicePtr	local,
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
  MuTPrivatePtr	priv = (MuTPrivatePtr) local->private;
  int		width = priv->max_x - priv->min_x;
  int		height = priv->max_y - priv->min_y;

  if (first != 0 || num != 2)
    return FALSE;

  *x = (priv->screen_width * (v0 - priv->min_x)) / width;
  *y = (priv->screen_height -
	(priv->screen_height * (v1 - priv->min_y)) / height);
  
  return TRUE;
}


/*
 ***************************************************************************
 *
 * xf86MuTReadInput --
 *	Read a buffer full of input from the touchscreen and enqueue
 *	all report packets found in it.
 *	If a packet is not fully received it is deferred until the next
 *	call to the function.
 *	Packet recognized by this function comply with the format :
 *
 *		Byte 1 :  Status flags with MSB set to 1
 *		Byte 2 :  X coordinate (lower bits)
 *		Byte 3 :  X coordinate (upper bits)
 *		Byte 4 :  Y coordinate (lower bits)
 *		Byte 5 :  Y coordinate (upper bits)
 *
 *	The routine can work with any of the two X device structs associated
 *	with the touchscreen. It is always possible to find the relevant
 *	informations and to emit the events for both devices if provided
 *	with one of the two structs. This point is relevant only if the
 *	two devices are actives at the same time.
 *

 ***************************************************************************
 */
static void
xf86MuTReadInput(LocalDevicePtr	local)
{
  MuTPrivatePtr		priv = (MuTPrivatePtr)(local->private);
  int			cur_x, cur_y;
  int			state;
  int			num_bytes, i;
  int			bytes_in_packet;
  unsigned char		*ptr, *start_ptr;
  
  DBG(4, ErrorF("Entering ReadInput\n"));
  
  /*
   * Try to get a buffer full of report packets.
   */
  DBG(4, ErrorF("num_old_bytes is %d, Trying to read %d bytes from port\n",
		priv->num_old_bytes, MuT_BUFFER_SIZE - priv->num_old_bytes));
  SYSCALL(num_bytes = read(local->fd,
			   (char *) (priv->rec_buf + priv->num_old_bytes),
			   MuT_BUFFER_SIZE - priv->num_old_bytes));

  if (num_bytes < 0) {
    Error("System error while reading from MicroTouch touchscreen.");
    return;
  }

  DBG(4, ErrorF("Read %d bytes of reports\n", num_bytes));
  num_bytes += priv->num_old_bytes;
  ptr = priv->rec_buf;
  bytes_in_packet = 0;
  start_ptr = ptr;

  while (num_bytes >= (MuT_REPORT_SIZE-bytes_in_packet)) {
    /*
     * Skip bytes until a status byte (MSB set to 1).
     */
    if (bytes_in_packet == 0) {
      if ((ptr[0] & 0x80) == 0) {
	DBG(3, ErrorF("Dropping a byte in an attempt to synchronize a report packet: 0x%X\n",
		      ptr[0]));
	start_ptr++;
      }
      else {
	bytes_in_packet++;
      }
      num_bytes--;
      ptr++;
    }
    else if (bytes_in_packet != 5) {
      if ((ptr[0] & 0x80) == 0) {
	bytes_in_packet++;
      }
      else {
	/*
	 * Reset the start of packet, we have most certainly
	 * lost some data.
	 */
	DBG(3, ErrorF("Reseting start of report packet data has been lost\n"));
	bytes_in_packet = 1;
	start_ptr = ptr;
      }
      ptr++;
      num_bytes--;
    }

    if (bytes_in_packet == 5) {
      LocalDevicePtr	local_to_use;
      
      /*
       * First stick together the various pieces.
       */
      state = start_ptr[0] & 0x7F;
      cur_x = WORD_ASSEMBLY(start_ptr[1], start_ptr[2]);
      cur_y = WORD_ASSEMBLY(start_ptr[3], start_ptr[4]);
      
      DBG(3, ErrorF("Packet: 0x%X 0x%X 0x%X 0x%X 0x%X\n",
		    start_ptr[0], start_ptr[1], start_ptr[2], start_ptr[3], start_ptr[4]));
      start_ptr = ptr;
      bytes_in_packet = 0;
      
      /*
       * Send events.
       *
       * We *must* generate a motion before a button change if pointer
       * location has changed as DIX assumes this. This is why we always
       * emit a motion, regardless of the kind of packet processed.
       */
      local_to_use = (state & MuT_WHICH_DEVICE) ? priv->stylus : priv->finger;
      
      /*
       * Emit a motion. If in core pointer mode we need to calibrate
       * or we will feed X with quite bogus event positions.
       */
      xf86PostMotionEvent(local_to_use->dev, TRUE, 0, 2, cur_x, cur_y);
      
      /*
       * Emit a button press or release.
       */
      if ((state & MuT_CONTACT) != (priv->state & MuT_CONTACT)) {
	xf86PostButtonEvent(local_to_use->dev, TRUE, 1, state & MuT_CONTACT,
			    0, 2, cur_x, cur_y);
      }
      
      DBG(3, ErrorF("TouchScreen %s: x(%d), y(%d), %s\n",
		    ((state & MuT_WHICH_DEVICE) ? "Stylus" : "Finger"),
		    cur_x, cur_y,
		    (((state & MuT_CONTACT) != (priv->state & MuT_CONTACT)) ?
		     ((state & MuT_CONTACT) ? "Press" : "Release") : "Stream")));
      
      priv->state = state;
    }
  }

  /*
   * If some bytes are left in the buffer, pack them at the
   * beginning for the next turn.
   */
  if (num_bytes != 0) {
    memcpy(priv->rec_buf, ptr, num_bytes);
    priv->num_old_bytes = num_bytes;
  }
  else {
    priv->num_old_bytes = 0;
  }
}


/*
 ***************************************************************************
 *
 * xf86MuTSendPacket --
 *	Emit a variable length packet to the controller.
 *	The function expects a valid buffer containing the
 *	command to be sent to the controller.  The command
 *	size is in len
 *	The buffer is filled with the leading and trailing
 *	character before sending.
 *
 ***************************************************************************
 */
static Bool
xf86MuTSendPacket(unsigned char	*packet,
		  int		len,
		  int		fd)
{
  int	result;

  packet[0] = MuT_LEAD_BYTE;
  packet[len+1] = MuT_TRAIL_BYTE;

  DBG(4, ErrorF("Sending packet : 0x%X 0x%X 0x%X 0x%X 0x%X 0x%X 0x%X 0x%X 0x%X 0x%X \n",
		packet[0], packet[1], packet[2], packet[3], packet[4],
		packet[5], packet[6], packet[7], packet[8], packet[9]));
  SYSCALL(result = write(fd, packet, len+2));
  if (result != len+2) {
    DBG(5, ErrorF("System error while sending to MicroTouch touchscreen.\n"));
    return !Success;
  }
  else
    return Success;
}


/*
 ***************************************************************************
 *
 * xf86MuTGetReply --
 *	Read a reply packet from the port. Synchronize with start and stop
 *	of packet.
 *      The packet structure read by this function is as follow:
 *		Byte 0 : MuT_LEAD_BYTE
 *		Byte 1
 *		...
 *		Byte n : packet data
 *		Byte n+1 : MuT_TRAIL_BYTE
 *
 *	This function returns if a valid packet has been assembled in
 *	buffer or if no more data is available to do so.
 *
 *	Returns Success if a packet is successfully assembled.
 *	Bytes preceding the MuT_LEAD_BYTE are discarded.
 *	Returns !Success if out of data while reading. The start of the
 *	partially assembled packet is left in buffer, buffer_p reflects
 *	the current state of assembly. Buffer should at least have room
 *	for MuT_BUFFER_SIZE bytes.
 *
 ***************************************************************************
 */
static Bool
xf86MuTGetReply(unsigned char	*buffer,
		int		*buffer_p,
		int		fd)
{
  int	num_bytes;
  Bool	ok;

  DBG(4, ErrorF("Entering xf86MuTGetReply with buffer_p == %d\n", *buffer_p));
  
  /*
   * Try to read enough bytes to fill up the packet buffer.
   */
  DBG(4, ErrorF("buffer_p is %d, Trying to read %d bytes from port\n",
		*buffer_p, MuT_BUFFER_SIZE - *buffer_p));
  SYSCALL(num_bytes = read(fd,
			   (char *) (buffer + *buffer_p),
			   MuT_BUFFER_SIZE - *buffer_p));
  
  /*
   * Okay, give up.
   */
  if (num_bytes < 0) {
    Error("System error while reading from MicroTouch touchscreen.");
    return !Success;
  }
  DBG(4, ErrorF("Read %d bytes of reply\n", num_bytes));
    
  while (num_bytes) {
    /*
     * Sync with the start of a packet.
     */
    if ((*buffer_p == 0) && (buffer[0] != MuT_LEAD_BYTE)) {
      /*
       * No match, shift data one byte toward the start of the buffer.
       */
      DBG(4, ErrorF("Dropping one byte in an attempt to synchronize: '%c' 0x%X\n",
		    buffer[0], buffer[0]));
      memcpy(&buffer[0], &buffer[1], num_bytes-1);
      num_bytes--;
    }
    else if (buffer[*buffer_p] == MuT_TRAIL_BYTE) {
      /*
       * Got a packet, report it.
       */
      *buffer_p = 0;
      return Success;
    }
    else {
      num_bytes--;
      (*buffer_p)++;
    }
  }

  return !Success;
}


/*
 ***************************************************************************
 *
 * xf86MuTWaitReply --
 *	It is assumed that the reply will be in the few next bytes
 *	read and will be available very soon after the command post. if
 *	these two asumptions are not met, there are chances that the server
 *	will be stuck for a while.
 *	The reply is left in reply. The function returns Success if a valid
 *	reply was found and !Success otherwise. Reply should at least
 *	have room for MuT_BUFFER_SIZE bytes.
 *
 ***************************************************************************
 */
static Bool
xf86MuTWaitReply(unsigned char	*reply,
		 int		fd)
{
  Bool			ok;
  int			i, result;
  int			reply_p = 0;
  fd_set		readfds;
  struct timeval	timeout;
  unsigned char		local_reply[3];

  DBG(4, ErrorF("Waiting a reply\n"));
  i = MuT_MAX_TRIALS;
  do {
    ok = !Success;
    
    /*
     * Wait half a second for the reply. The fuse counts down each
     * timeout and each wrong packet.
     */
    FD_ZERO(&readfds);
    FD_SET(fd, &readfds);
    timeout.tv_sec = 0;
    timeout.tv_usec = MuT_MAX_WAIT;
    DBG(4, ErrorF("Waiting %d ms for data from port\n", MuT_MAX_WAIT / 1000));
    SYSCALL(result = select(FD_SETSIZE, &readfds, NULL, NULL, &timeout));
    if (result > 0 && FD_ISSET(fd, &readfds)) {
      if (reply)
	ok = xf86MuTGetReply(reply, &reply_p, fd);
      else {
	ok = xf86MuTGetReply(local_reply, &reply_p, fd);
	if (ok && local_reply[1] != MuT_OK) {
	  DBG(3, ErrorF("Error reported by firmware\n"));
	  ok = !Success;
	}
      }
    }
    else {
      DBG(3, ErrorF("No answer from port : %d\n", result));
    }
    
    if (result == 0)
      i--;
  } while(ok != Success && i);

  return ok;
}


/*
 ***************************************************************************
 *
 * xf86MuTSendCommand --
 *	Emit a command to the controller and blocks until the reply is
 *	read.
 *
 *	The reply is left in reply. The function returns Success if the
 *	reply is valid and !Success otherwise. Reply should at least
 *	have room for MuT_BUFFER_SIZE bytes.
 *
 ***************************************************************************
 */
static Bool
xf86MuTSendCommand(unsigned char	*request,
		   int			len,
		   unsigned char	*reply,
		   int			fd)
{
  Bool			ok;
  
  if (xf86MuTSendPacket(request, len, fd) == Success) {
    ok = xf86MuTWaitReply(reply, fd);
    return ok;
  }
  else
    return !Success;
}


/*
 ***************************************************************************
 *
 * xf86MuTPrintIdent --
 *	Print type of touchscreen and features on controller board.
 *
 ***************************************************************************
 */
static void
xf86MuTPrintIdent(unsigned char	*packet)
{
  int	vers, rev;
  
  ErrorF("%s MicroTouch touchscreen is a ", XCONFIG_PROBED);
  if (strncmp((char *) &packet[1], MuT_TOUCH_PEN_IDENT, 2) == 0)
    ErrorF("TouchPen");
  else if (strncmp((char *) &packet[1], MuT_SMT3_IDENT, 2) == 0)
    ErrorF("Serial/SMT3");
  ErrorF(", connected through a serial port.\n");
  sscanf((char *) &packet[3], "%2d%2d", &vers, &rev);
  ErrorF("%s MicroTouch controller firmware revision is %d.%d.\n", XCONFIG_PROBED, vers, rev);
}


/*
 ***************************************************************************
 *
 * xf86MuTPrintHwStatus --
 *	Print status of hardware. That is if the controller report errors,
 *	decode and display them.
 *
 ***************************************************************************
 */
static void
xf86MuTPrintHwStatus(unsigned char	*packet)
{
  ErrorF("%s MicroTouch status of errors: %c%c.\n",
	 XCONFIG_PROBED, packet[7], packet[8]);
}


/*
 ***************************************************************************
 *
 * xf86MuTPtrControl --
 *
 ***************************************************************************
 */
#if 0
static void
xf86MuTPtrControl(DeviceIntPtr	dev,
		  PtrCtrl	*ctrl)
{
}
#endif


/*
 ***************************************************************************
 *
 * xf86MuTControl --
 *
 ***************************************************************************
 */
static Bool
xf86MuTControl(DeviceIntPtr	dev,
	       int		mode)
{
  LocalDevicePtr	local = (LocalDevicePtr) dev->public.devicePrivate;
  MuTPrivatePtr		priv = (MuTPrivatePtr)(local->private);
  unsigned char		map[] = { 0, 1 };
  unsigned char		req[MuT_PACKET_SIZE];
  unsigned char		reply[MuT_BUFFER_SIZE];
  int			i, result;
  char			*id_string = DEVICE_ID(local->private_flags) == FINGER_ID ? "finger" : "stylus";
  
  switch(mode) {

  case DEVICE_INIT:
    {
      DBG(2, ErrorF("MicroTouch %s init...\n", id_string));

      if (priv->screen_no >= screenInfo.numScreens ||
	  priv->screen_no < 0)
	priv->screen_no = 0;
      priv->screen_width = screenInfo.screens[priv->screen_no]->width;
      priv->screen_height = screenInfo.screens[priv->screen_no]->height;

      /*
       * Device reports button press for up to 1 button.
       */
      if (InitButtonClassDeviceStruct(dev, 1, map) == FALSE) {
	ErrorF("Unable to allocate ButtonClassDeviceStruct\n");
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
	ErrorF("Unable to allocate ValuatorClassDeviceStruct\n");
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
	ErrorF("Unable to allocate FocusClassDeviceStruct\n");
      }
      
      /*
       * Allocate the motion events buffer.
       */
      xf86MotionHistoryAllocate(local);
      
      /*
       * This once has caused the server to crash after doing an xalloc & strcpy ??
       */
      AssignTypeAndName(dev, local->atom, local->name);
      
      DBG(2, ErrorF("Done.\n"));
      return Success;
    }
    
  case DEVICE_ON:
    {
      Bool	already_open = FALSE;
      char	*report_what;
      
      DBG(2, ErrorF("MicroTouch %s on...\n", id_string));

      /*
       * Try to see if the port has already been opened either
       * for this device or for the other one.
       */
      if (local->fd >= 0)
	already_open = TRUE;
      else {
	switch (DEVICE_ID(local->private_flags)) {
	case FINGER_ID:
	  if (priv->stylus && priv->stylus->fd >= 0) {
	    already_open = TRUE;
	    local->fd = priv->stylus->fd;
	  }
	  break;
	case STYLUS_ID:
	  if (priv->finger && priv->finger->fd >= 0) {
	    already_open = TRUE;
	    local->fd = priv->finger->fd;
	  }
	  break;
	}
      }
      if (!already_open) {
	struct termios termios_tty;
	
	DBG(2, ErrorF("MicroTouch touchscreen opening : %s\n", priv->input_dev));
	SYSCALL(local->fd = open(priv->input_dev, O_RDWR|O_NDELAY, 0));
	if (local->fd < 0) {
	  Error("Unable to open MicroTouch touchscreen device");
	  return !Success;
	}
	
	/*
	 * Try to see if the link is at the specified rate and
	 * reset the controller. The wait time needed by the
	 * controller after reset should be compensated by the
	 * timeouts in the receive section.
	 */
	DBG(3, ErrorF("Try to see if the link is at the specified rate\n"));
	memset(&termios_tty, 0, sizeof(termios_tty));
	termios_tty.c_cflag = priv->link_speed | CS8 | CREAD | CLOCAL;
#ifdef CRTSCTS
	termios_tty.c_cflag &= ~CRTSCTS;
#endif
	termios_tty.c_cc[VMIN] = 1;
	SYSCALL(result = tcsetattr(local->fd, TCSANOW, &termios_tty));
	if (result < 0) {
	  Error("Unable to configure MicroTouch touchscreen port");
	  goto not_success;
	}
	memset(req, 0, MuT_PACKET_SIZE);
	strncpy((char *) &req[1], MuT_RESET, strlen(MuT_RESET));
	if (xf86MuTSendCommand(req, strlen(MuT_RESET), NULL, local->fd) != Success) {
	  DBG(3, ErrorF("Not at the specified rate, giving up\n"));
	  goto not_success;
	}
	
	/*
	 * ask the controller to report identity and status.
	 */
	if (xf86Verbose) {
	  memset(req, 0, MuT_PACKET_SIZE);
	  strncpy((char *) &req[1], MuT_OUTPUT_IDENT, strlen(MuT_OUTPUT_IDENT));
	  if (xf86MuTSendCommand(req, strlen(MuT_OUTPUT_IDENT),
				 reply, local->fd) != Success) {
	    ErrorF("Unable to ask MicroTouch touchscreen identification\n");
	    goto not_success;
	  }
	  xf86MuTPrintIdent(reply);
	  memset(req, 0, MuT_PACKET_SIZE);
	  strncpy((char *) &req[1], MuT_UNIT_TYPE, strlen(MuT_UNIT_TYPE));
	  if (xf86MuTSendCommand(req, strlen(MuT_UNIT_TYPE),
				 reply, local->fd) != Success) {
	    ErrorF("Unable to ask MicroTouch touchscreen status\n");
	    goto not_success;
	  }
	  xf86MuTPrintHwStatus(reply);
	}
	
	/*
	 * Set the operating mode: Format Tablet, Mode stream, Pen.
	 */
	memset(req, 0, MuT_PACKET_SIZE);
	strncpy((char *) &req[1], MuT_FORMAT_TABLET, strlen(MuT_FORMAT_TABLET));
	if (xf86MuTSendCommand(req, strlen(MuT_FORMAT_TABLET),
			       NULL, local->fd) != Success) {
	  ErrorF("Unable to switch MicroTouch touchscreen to Tablet Format\n");
	  goto not_success;
	}
	memset(req, 0, MuT_PACKET_SIZE);
	strncpy((char *) &req[1], MuT_MODE_STREAM, strlen(MuT_MODE_STREAM));
	if (xf86MuTSendCommand(req, strlen(MuT_MODE_STREAM),
			       NULL, local->fd) != Success) {
	  ErrorF("Unable to switch MicroTouch touchscreen to Stream Mode\n");
	  goto not_success;
	}

	memset(req, 0, MuT_PACKET_SIZE);
	strncpy((char *) &req[1], MuT_PEN_ONLY, strlen(MuT_PEN_ONLY));
	if (xf86MuTSendCommand(req, strlen(MuT_PEN_ONLY),
			       NULL, local->fd) != Success) {
	  ErrorF("Unable to change MicroTouch touchscreen to pen mode\n");
	  goto not_success;
	}
	/*	goto not_success;*/

	AddEnabledDevice(local->fd);
      }

      /*
       * Select Pen / Finger reports depending on which devices are
       * currently on.
       */
      switch (DEVICE_ID(local->private_flags)) {
      case FINGER_ID:
	if (priv->stylus && priv->stylus->dev->public.on)
	  report_what = MuT_PEN_FINGER;
	else
	  report_what = MuT_FINGER_ONLY;
	break;
      case STYLUS_ID:
	if (priv->finger && priv->finger->dev->public.on)
	  report_what = MuT_PEN_FINGER;
	else
	  report_what = MuT_PEN_ONLY;
	break;
      }
      memset(req, 0, MuT_PACKET_SIZE);
      strncpy((char *) &req[1], report_what, strlen(report_what));
      if (xf86MuTSendCommand(req, strlen(report_what), NULL, local->fd) != Success) {
	ErrorF("Unable to change MicroTouch touchscreen to %s\n",
	       (strcmp(report_what, MuT_PEN_FINGER) == 0) ? "Pen & Finger" :
	       ((strcmp(report_what, MuT_PEN_ONLY) == 0) ? "Pen Only" : "Finger Only"));
	goto not_success;
      }
      dev->public.on = TRUE;  
    
      DBG(2, ErrorF("Done\n"));
      return Success;
      
    not_success:
      SYSCALL(close(local->fd));
      local->fd = -1;
      return !Success;
    }
  
  /*
   * Deactivate the device.
   */
  case DEVICE_OFF:
    DBG(2, ErrorF("MicroTouch %s off...\n", id_string));
    dev->public.on = FALSE;
    DBG(2, ErrorF("Done\n"));
    return Success;
    
    /*
     * Final close before server exit. This is used during server shutdown.
     * Close the port and free all the resources.
     */
  case DEVICE_CLOSE:
    DBG(2, ErrorF("MicroTouch %s close...\n", id_string));
    dev->public.on = FALSE;
    if (local->fd >= 0) {
      RemoveEnabledDevice(local->fd);
      SYSCALL(close(local->fd));
      local->fd = -1;
      /*
       * Need some care to close the port only once.
       */
      switch (DEVICE_ID(local->private_flags)) {
	case FINGER_ID:
	  if (priv->stylus)
	    priv->stylus->fd = -1;
	  break;
	case STYLUS_ID:
	  if (priv->finger)
	    priv->finger->fd = -1;
      }
    }
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
 * xf86MuTAllocate --
 *
 ***************************************************************************
 */
static LocalDevicePtr
xf86MuTAllocate(
#if NeedFunctionPrototypes
		char	*name,
		char	*type_name,
		int	flag
#endif
	)
{
  LocalDevicePtr        local = (LocalDevicePtr) xalloc(sizeof(LocalDeviceRec));
  MuTPrivatePtr         priv = (MuTPrivatePtr) xalloc(sizeof(MuTPrivateRec));
  
  priv->input_dev = MuT_PORT;
  priv->link_speed = MuT_LINK_SPEED;
  priv->min_x = 0;
  priv->max_x = 0;
  priv->min_y = 0;
  priv->max_y = 0;
  priv->screen_no = 0;
  priv->screen_width = -1;
  priv->screen_height = -1;
  priv->inited = 0;
  priv->state = 0;
  priv->num_old_bytes = 0;
  priv->stylus = NULL;
  priv->finger = NULL;
  
  local->name = name;
  local->flags = XI86_NO_OPEN_ON_INIT;
  local->device_config = xf86MuTConfig;
  local->device_control = xf86MuTControl;
  local->read_input = xf86MuTReadInput;
  local->control_proc = NULL;
  local->close_proc = NULL;
  local->switch_mode = NULL;
  local->conversion_proc = xf86MuTConvert;
  local->reverse_conversion_proc = NULL;
  local->fd = -1;
  local->atom = 0;
  local->dev = NULL;
  local->private = priv;
  local->private_flags = flag;
  local->type_name = type_name;
  local->history_size = 0;
  
  return local;
}


/*
 ***************************************************************************
 *
 * xf86MuTAllocateFinger --
 *
 ***************************************************************************
 */
static LocalDevicePtr
xf86MuTAllocateFinger()
{
  LocalDevicePtr	local = xf86MuTAllocate(XI_FINGER, "MicroTouch Finger", FINGER_ID);

  ((MuTPrivatePtr) local->private)->finger = local;
  return local;  
}


/*
 ***************************************************************************
 *
 * xf86MuTAllocateStylus --
 *
 ***************************************************************************
 */
static LocalDevicePtr
xf86MuTAllocateStylus()
{
  LocalDevicePtr	local = xf86MuTAllocate(XI_STYLUS, "MicroTouch Stylus", STYLUS_ID);

  ((MuTPrivatePtr) local->private)->stylus = local;
  return local;
}


/*
 ***************************************************************************
 *
 * MicroTouch finger device association --
 *
 ***************************************************************************
 */
DeviceAssocRec MuT_finger_assoc =
{
  FINGER_SECTION_NAME,		/* config_section_name */
  xf86MuTAllocateFinger		/* device_allocate */
};

/*
 ***************************************************************************
 *
 * MicroTouch stylus device association --
 *
 ***************************************************************************
 */
DeviceAssocRec MuT_stylus_assoc =
{
  STYLUS_SECTION_NAME,		/* config_section_name */
  xf86MuTAllocateStylus		/* device_allocate */
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
init_xf86MuTouch(unsigned long      server_version)
#endif
{
    xf86AddDeviceAssoc(&MuT_finger_assoc);
    xf86AddDeviceAssoc(&MuT_stylus_assoc);

    if (server_version != XF86_VERSION_CURRENT) {
	ErrorF("Warning: MicroTouch module compiled for version%s\n", XF86_VERSION);
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
XF86ModuleVersionInfo xf86MuTVersion = {
    "xf86MuTouch",
    MODULEVENDORSTRING,
    MODINFOSTRING1,
    MODINFOSTRING2,
    XF86_VERSION_CURRENT,
    0x00010000,
    {0,0,0,0}
};

void
xf86MuTModuleInit(data, magic)
    pointer *data;
    INT32 *magic;
{
    static int cnt = 0;

    switch (cnt) {
      case 0:
      *magic = MAGIC_VERSION;
      *data = &xf86MuTVersion;
      cnt++;
      break;
      
      case 1:
      *magic = MAGIC_ADD_XINPUT_DEVICE;
      *data = &MuT_finger_assoc;
      cnt++;
      break;

      case 2:
      *magic = MAGIC_ADD_XINPUT_DEVICE;
      *data = &MuT_stylus_assoc;
      cnt++;
      break;

      default:
      *magic = MAGIC_DONE;
      *data = NULL;
      break;
    } 
}
#endif

/* $XFree86: xc/programs/Xserver/hw/xfree86/common/xf86_Mouse.c,v 3.21.2.26 2001/02/05 23:10:04 herrb Exp $ */
/*
 *
 * Copyright 1990,91 by Thomas Roell, Dinkelscherben, Germany.
 * Copyright 1993 by David Dawes <dawes@xfree86.org>
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the names of Thomas Roell and David Dawes not be
 * used in advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.  Thomas Roell
 * and David Dawes makes no representations about the suitability of this
 * software for any purpose.  It is provided "as is" without express or
 * implied warranty.
 *
 * THOMAS ROELL AND DAVID DAWES DISCLAIM ALL WARRANTIES WITH REGARD TO THIS
 * SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS, IN NO EVENT SHALL THOMAS ROELL OR DAVID DAWES BE LIABLE FOR ANY
 * SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER
 * RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF
 * CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 */
/* $XConsortium: xf86_Mouse.c /main/21 1996/10/27 11:05:32 kaleb $ */

/*
 * [JCH-96/01/21] Added fourth button support for P_GLIDEPOINT mouse protocol.
 */

/*
 * [TVO-97/03/05] Added microsoft IntelliMouse support
 */

#define NEED_EVENTS
#include "X.h"
#include "Xproto.h"
#include "inputstr.h"
#include "scrnintstr.h"

#include "compiler.h"

#include "xf86Procs.h"
#include "xf86_OSlib.h"
#include "xf86_Config.h"

#ifdef USB_MOUSE
#include "xf86_UsbMse.h"
#endif

#ifdef WSCONS_SUPPORT
#include <dev/wscons/wsconsio.h>
#endif

#ifdef XINPUT
#include "xf86Xinput.h"
#include "extnsionst.h"
#include "extinit.h"

static int xf86MouseProc(
#if NeedFunctionPrototypes
    DeviceIntPtr	/* device */,
    int			/* what */
#endif
);

static void xf86MouseReadInput(
#if NeedFunctionPrototypes
    LocalDevicePtr	/* local */
#endif
);

static LocalDevicePtr xf86MouseAllocate(
#if NeedFunctionPrototypes
    void
#endif
);

static Bool xf86MouseConfig(
#if NeedFunctionPrototypes
    LocalDevicePtr *	/* array */,
    int 		/* inx */,
    int			/* max */,
    LexPtr		/* val */
#endif
);
#endif /* XINPUT */

#ifndef MOUSE_PROTOCOL_IN_KERNEL
/*
 * List of mouse types supported by xf86MouseProtocol()
 *
 * For systems which do the mouse protocol translation in the kernel,
 * this list should be defined in the appropriate *_io.c file under
 * xf86/os-support.
 */
Bool xf86SupportedMouseTypes[] =
{
	TRUE,	/* Microsoft */
	TRUE,	/* MouseSystems */
	TRUE,	/* MMSeries */
	TRUE,	/* Logitech */
	TRUE,	/* BusMouse */
	TRUE,	/* MouseMan */
	TRUE,	/* PS/2 */
	TRUE,	/* Hitachi Tablet */
	TRUE,	/* ALPS GlidePoint (serial) */
	TRUE,   /* Microsoft IntelliMouse (serial) */
	TRUE,	/* Kensington ThinkingMouse (serial) */
#if !defined(__FreeBSD__)
	TRUE,   /* Microsoft IntelliMouse (PS/2) */
	TRUE,	/* Kensington ThinkingMouse (PS/2) */
	TRUE,	/* Logitech MouseMan+ (PS/2) */
	TRUE,	/* ALPS GlidePoint (PS/2) */
	TRUE,	/* Genius NetMouse (PS/2) */
	TRUE,	/* Genius NetScroll (PS/2) */
#else
	FALSE,	/* Microsoft IntelliMouse (PS/2) */
	FALSE,	/* Kensington ThinkingMouse (PS/2) */
	FALSE,	/* Logitech MouseMan+ (PS/2) */
	FALSE,	/* ALPS GlidePoint (PS/2) */
	FALSE,	/* Genius NetMouse (PS/2) */
	FALSE,	/* Genius NetScroll (PS/2) */
#endif /* __FreeBSD__ */
	TRUE,	/* sysmouse */
#ifdef PNP_MOUSE
	TRUE,	/* auto */
#else
	FALSE,	/* auto */
#endif
	TRUE,	/* ACECAD */
#ifdef WSCONS_SUPPORT
	TRUE,	/* wsmouse */
#else
	FALSE,	/* wsmouse */
#endif
#ifdef USB_MOUSE
        TRUE,
#else
        FALSE,
#endif
};

int xf86NumMouseTypes = sizeof(xf86SupportedMouseTypes) /
			sizeof(xf86SupportedMouseTypes[0]);

/*
 * termio[s] c_cflag settings for each mouse type.
 *
 * For systems which do the mouse protocol translation in the kernel,
 * this list should be defined in the appropriate *_io.c under
 * xf86/os-support if it is required.
 */

unsigned short xf86MouseCflags[] =
{
	(CS7                   | CREAD | CLOCAL | HUPCL ),   /* MicroSoft */
	(CS8 | CSTOPB          | CREAD | CLOCAL | HUPCL ),   /* MouseSystems */
	(CS8 | PARENB | PARODD | CREAD | CLOCAL | HUPCL ),   /* MMSeries */
	(CS8 | CSTOPB          | CREAD | CLOCAL | HUPCL ),   /* Logitech */
	0,						     /* BusMouse */
	(CS7                   | CREAD | CLOCAL | HUPCL ),   /* MouseMan,
                                                              [CHRIS-211092] */
	0,						     /* PS/2 */
	(CS8                   | CREAD | CLOCAL | HUPCL ),   /* mmhitablet */
	(CS7                   | CREAD | CLOCAL | HUPCL ),   /* GlidePoint */
	(CS7                   | CREAD | CLOCAL | HUPCL ),   /* IntelliMouse */
	(CS7                   | CREAD | CLOCAL | HUPCL ),   /* ThinkingMouse */
							     /* PS/2 variants */
	0,						     /* IntelliMouse */
	0,						     /* ThinkingMouse */
	0,						     /* MouseMan+ */
	0,						     /* GlidePoint */
	0,						     /* NetMouse */
	0,						     /* NetScroll */

	(CS8 | CSTOPB          | CREAD | CLOCAL | HUPCL ),   /* sysmouse */
	0,						     /* auto */
	(CS8 | PARENB | PARODD | CREAD | CLOCAL | HUPCL ),   /* ACECAD */
};
#endif /* ! MOUSE_PROTOCOL_IN_KERNEL */


/*
 * xf86MouseSupported --
 *	Returns true if OS supports mousetype
 */

Bool
xf86MouseSupported(mousetype)
     int mousetype;
{
    if (mousetype < 0 || mousetype >= xf86NumMouseTypes)
    {
	return(FALSE);
    }
    return(xf86SupportedMouseTypes[mousetype]);
}

/*
 * xf86SetupMouse --
 *	Sets up the mouse parameters
 */

#ifndef MOUSE_PROTOCOL_IN_KERNEL
static unsigned char proto[][7] = {
  /* hd_mask hd_id dp_mask dp_id bytes b4_mask b4_id */
  {  0x40,   0x40, 0x40,   0x00, 3,   ~0x23,   0x00 },  /* MicroSoft */
  {  0xf8,   0x80, 0x00,   0x00, 5,    0x00,   0xff },  /* MouseSystems */
  {  0xe0,   0x80, 0x80,   0x00, 3,    0x00,   0xff },  /* MMSeries */
  {  0xe0,   0x80, 0x80,   0x00, 3,    0x00,   0xff },  /* Logitech */
  {  0xf8,   0x80, 0x00,   0x00, 5,    0x00,   0xff },  /* BusMouse */
  {  0x40,   0x40, 0x40,   0x00, 3,   ~0x23,   0x00 },  /* MouseMan */
  {  0xc0,   0x00, 0x00,   0x00, 3,    0x00,   0xff },  /* PS/2 mouse */
  {  0xe0,   0x80, 0x80,   0x00, 3,    0x00,   0xff },  /* MM_HitTablet */
  {  0x40,   0x40, 0x40,   0x00, 3,   ~0x33,   0x00 },  /* GlidePoint */
  {  0x40,   0x40, 0x40,   0x00, 3,   ~0x3f,   0x00 },  /* IntelliMouse */
  {  0x40,   0x40, 0x40,   0x00, 3,   ~0x33,   0x00 },  /* ThinkingMouse */
							/* PS/2 variants */
  {  0xc0,   0x00, 0x00,   0x00, 4,    0x00,   0xff },  /* IntelliMouse */
  {  0x80,   0x80, 0x00,   0x00, 3,    0x00,   0xff },  /* ThinkingMouse */
  {  0x08,   0x08, 0x00,   0x00, 3,    0x00,   0xff },  /* MouseMan+ */
  {  0xc0,   0x00, 0x00,   0x00, 3,    0x00,   0xff },  /* GlidePoint */
  {  0xc0,   0x00, 0x00,   0x00, 4,    0x00,   0xff },  /* NetMouse */
  {  0xc0,   0x00, 0x00,   0x00, 6,    0x00,   0xff },  /* NetScroll */

  {  0xf8,   0x80, 0x00,   0x00, 5,    0x00,   0xff },  /* sysmouse */
  {  0xf8,   0x80, 0x00,   0x00, 5,    0x00,   0xff },  /* dummy entry for auto - used only to fill space */
  {  0x80,   0x80, 0x80,   0x00, 3,    0x00,   0xff },  /* ACECAD */
#ifdef WSCONS_SUPPORT
  {  0x00,   0x00, 0x00,   0x00, sizeof(struct wscons_event),
     				       0x00,   0x00 },  /* wsmouse */
#else
  {  0x00,   0x00, 0x00,   0x00, 0,    0x00,   0x00 },  /* wsmouse */
#endif
  {  0x00,   0x00, 0x00,   0x00, 0,    0x00,   0x00 },  /* USB mouse */
};
#endif /* ! MOUSE_PROTOCOL_IN_KERNEL */

void
xf86SetupMouse(mouse)
MouseDevPtr mouse;
{
#if !defined(MOUSE_PROTOCOL_IN_KERNEL) || defined(MACH386)
      /*
      ** The following lines take care of the Logitech MouseMan protocols.
      **
      ** NOTE: There are different versions of both MouseMan and TrackMan!
      **       Hence I add another protocol P_LOGIMAN, which the user can
      **       specify as MouseMan in his XF86Config file. This entry was
      **       formerly handled as a special case of P_MS. However, people
      **       who don't have the middle button problem, can still specify
      **       Microsoft and use P_MS.
      **
      ** By default, these mice should use a 3 byte Microsoft protocol
      ** plus a 4th byte for the middle button. However, the mouse might
      ** have switched to a different protocol before we use it, so I send
      ** the proper sequence just in case.
      **
      ** NOTE: - all commands to (at least the European) MouseMan have to
      **         be sent at 1200 Baud.
      **       - each command starts with a '*'.
      **       - whenever the MouseMan receives a '*', it will switch back
      **	 to 1200 Baud. Hence I have to select the desired protocol
      **	 first, then select the baud rate.
      **
      ** The protocols supported by the (European) MouseMan are:
      **   -  5 byte packed binary protocol, as with the Mouse Systems
      **      mouse. Selected by sequence "*U".
      **   -  2 button 3 byte MicroSoft compatible protocol. Selected
      **      by sequence "*V".
      **   -  3 button 3+1 byte MicroSoft compatible protocol (default).
      **      Selected by sequence "*X".
      **
      ** The following baud rates are supported:
      **   -  1200 Baud (default). Selected by sequence "*n".
      **   -  9600 Baud. Selected by sequence "*q".
      **
      ** Selecting a sample rate is no longer supported with the MouseMan!
      ** Some additional lines in xf86Config.c take care of ill configured
      ** baud rates and sample rates. (The user will get an error.)
      **               [CHRIS-211092]
      */

#if defined(__FreeBSD__) && defined(MOUSE_PROTO_SYSMOUSE)
      static struct {
	int dproto;
	int proto;
      } devproto[] = {
	{ MOUSE_PROTO_MS, 		P_MS },
	{ MOUSE_PROTO_MSC, 		P_MSC },
	{ MOUSE_PROTO_LOGI, 		P_LOGI },
	{ MOUSE_PROTO_MM, 		P_MM },
	{ MOUSE_PROTO_LOGIMOUSEMAN, 	P_LOGIMAN },
	{ MOUSE_PROTO_BUS, 		P_BM },
	{ MOUSE_PROTO_INPORT, 		P_BM },
	{ MOUSE_PROTO_PS2, 		P_PS2 },
	{ MOUSE_PROTO_HITTAB, 		P_MMHIT },
	{ MOUSE_PROTO_GLIDEPOINT, 	P_GLIDEPOINT },
	{ MOUSE_PROTO_INTELLI, 		P_IMSERIAL },
	{ MOUSE_PROTO_THINK, 		P_THINKING },
	{ MOUSE_PROTO_SYSMOUSE, 	P_SYSMOUSE },
      };
      mousehw_t hw;
      mousemode_t mode;
#endif /* __FreeBSD__ */
      unsigned char *param;
      int paramlen;
      int ps2param;
      int i;

      if (mouse->mseType != P_AUTO)
	memcpy(mouse->protoPara, proto[mouse->mseType], 
	       sizeof(mouse->protoPara));
      else
	memset(mouse->protoPara, 0, sizeof(mouse->protoPara));

#if defined(__FreeBSD__) && defined(MOUSE_PROTO_SYSMOUSE)
      /* set the driver operation level, if applicable */
      i = 1;
      ioctl(mouse->mseFd, MOUSE_SETLEVEL, &i);

      /* interrogate the driver and get some intelligence on the device... */
      hw.iftype = MOUSE_IF_UNKNOWN;
      hw.model = MOUSE_MODEL_GENERIC;
      ioctl(mouse->mseFd, MOUSE_GETHWINFO, &hw);
      mouse->mseModel = hw.model;
      if (ioctl(mouse->mseFd, MOUSE_GETMODE, &mode) == 0)
        {
	  for (i = 0; i < sizeof(devproto)/sizeof(devproto[0]); ++i)
	    if (mode.protocol == devproto[i].dproto)
	      {
		mouse->mseType = devproto[i].proto;
		memcpy(mouse->protoPara, proto[mouse->mseType], 
		       sizeof(mouse->protoPara));
		/* override some paramters */
		mouse->protoPara[4] = mode.packetsize;
		mouse->protoPara[0] = mode.syncmask[0];
		mouse->protoPara[1] = mode.syncmask[1];
		break;
	      }
	  if (i >= sizeof(devproto)/sizeof(devproto[0]))
	    ErrorF("xf86SetupMouse: Unknown mouse device protocol - %d\n",
		   mode.protocol);
        }
#endif /* __FreeBSD__ */

#ifdef PNP_MOUSE
      if (mouse->mseType == P_AUTO)
	{
	  /* a PnP serial mouse? */
	  mouse->mseType = xf86GetPnPMouseProtocol(mouse);
	  if (mouse->mseType < 0)
	    mouse->mseType = P_AUTO;
	  else
            memcpy(mouse->protoPara, proto[mouse->mseType], 
		   sizeof(mouse->protoPara));
	}
#endif

      param = NULL;
      paramlen = 0;
      ps2param = FALSE;
      switch (mouse->mseType) {

      case P_AUTO:
	if (!xf86AllowMouseOpenFail)
	  FatalError("xf86SetupMouse: Cannot determine the mouse protocol\n");
	else
	  ErrorF("xf86SetupMouse: Cannot determine the mouse protocol\n");
	break;

      case P_LOGI:		/* Logitech Mice */
        /* 
	 * The baud rate selection command must be sent at the current
	 * baud rate; try all likely settings 
	 */
	xf86SetMouseSpeed(mouse, 9600, mouse->baudRate,
                          xf86MouseCflags[mouse->mseType]);
	xf86SetMouseSpeed(mouse, 4800, mouse->baudRate, 
                          xf86MouseCflags[mouse->mseType]);
	xf86SetMouseSpeed(mouse, 2400, mouse->baudRate,
                          xf86MouseCflags[mouse->mseType]);
	xf86SetMouseSpeed(mouse, 1200, mouse->baudRate,
                          xf86MouseCflags[mouse->mseType]);
        /* select MM series data format */
	write(mouse->mseFd, "S", 1);
	xf86SetMouseSpeed(mouse, mouse->baudRate, mouse->baudRate,
                          xf86MouseCflags[P_MM]);
        /* select report rate/frequency */
	if      (mouse->sampleRate <=   0)  write(mouse->mseFd, "O", 1);  /* 100 */
	else if (mouse->sampleRate <=  15)  write(mouse->mseFd, "J", 1);  /*  10 */
	else if (mouse->sampleRate <=  27)  write(mouse->mseFd, "K", 1);  /*  20 */
	else if (mouse->sampleRate <=  42)  write(mouse->mseFd, "L", 1);  /*  35 */
	else if (mouse->sampleRate <=  60)  write(mouse->mseFd, "R", 1);  /*  50 */
	else if (mouse->sampleRate <=  85)  write(mouse->mseFd, "M", 1);  /*  67 */
	else if (mouse->sampleRate <= 125)  write(mouse->mseFd, "Q", 1);  /* 100 */
	else                                write(mouse->mseFd, "N", 1);  /* 150 */
	break;

      case P_LOGIMAN:
        xf86SetMouseSpeed(mouse, 1200, 1200, xf86MouseCflags[mouse->mseType]);
        write(mouse->mseFd, "*X", 2);
        xf86SetMouseSpeed(mouse, 1200, mouse->baudRate,
                          xf86MouseCflags[mouse->mseType]);
        break;

      case P_MMHIT:		/* MM_HitTablet */
	{
	  char speedcmd;

	  xf86SetMouseSpeed(mouse, mouse->baudRate, mouse->baudRate,
                            xf86MouseCflags[mouse->mseType]);
	  /*
	   * Initialize Hitachi PUMA Plus - Model 1212E to desired settings.
	   * The tablet must be configured to be in MM mode, NO parity,
	   * Binary Format.  mouse->sampleRate controls the sensativity
	   * of the tablet.  We only use this tablet for it's 4-button puck
	   * so we don't run in "Absolute Mode"
	   */
	  write(mouse->mseFd, "z8", 2);	/* Set Parity = "NONE" */
	  usleep(50000);
	  write(mouse->mseFd, "zb", 2);	/* Set Format = "Binary" */
	  usleep(50000);
	  write(mouse->mseFd, "@", 1);	/* Set Report Mode = "Stream" */
	  usleep(50000);
	  write(mouse->mseFd, "R", 1);	/* Set Output Rate = "45 rps" */
	  usleep(50000);
	  write(mouse->mseFd, "I\x20", 2);	/* Set Incrememtal Mode "20" */
	  usleep(50000);
	  write(mouse->mseFd, "E", 1);	/* Set Data Type = "Relative */
	  usleep(50000);
	  /* These sample rates translate to 'lines per inch' on the Hitachi
	     tablet */
	  if      (mouse->sampleRate <=   40) speedcmd = 'g';
	  else if (mouse->sampleRate <=  100) speedcmd = 'd';
	  else if (mouse->sampleRate <=  200) speedcmd = 'e';
	  else if (mouse->sampleRate <=  500) speedcmd = 'h';
	  else if (mouse->sampleRate <= 1000) speedcmd = 'j';
	  else                                speedcmd = 'd';
	  write(mouse->mseFd, &speedcmd, 1);
	  usleep(50000);
	  write(mouse->mseFd, "\021", 1);	/* Resume DATA output */
	}
        break;

      case P_THINKING:		/* ThinkingMouse */
        {
	  fd_set fds;
          char *s;
          char c;

          xf86SetMouseSpeed(mouse, 1200, mouse->baudRate, 
                            xf86MouseCflags[mouse->mseType]);
          /* this mouse may send a PnP ID string, ignore it */
	  usleep(200000);
	  xf86FlushInput(mouse->mseFd);
          /* send the command to initialize the beast */
          for (s = "E5E5"; *s; ++s) {
            write(mouse->mseFd, s, 1);
	    FD_ZERO(&fds);
	    FD_SET(mouse->mseFd, &fds);
	    if (select(FD_SETSIZE, &fds, NULL, NULL, NULL) <= 0)
	      break;
            read(mouse->mseFd, &c, 1);
            if (c != *s)
              break;
          }
        }
	break;

      case P_MSC:		/* MouseSystems Corp */
	xf86SetMouseSpeed(mouse, mouse->baudRate, mouse->baudRate,
                          xf86MouseCflags[mouse->mseType]);
#ifdef CLEARDTR_SUPPORT
        if (mouse->mouseFlags & MF_CLEAR_DTR)
          {
            i = TIOCM_DTR;
            ioctl(mouse->mseFd, TIOCMBIC, &i);
          }
        if (mouse->mouseFlags & MF_CLEAR_RTS)
          {
            i = TIOCM_RTS;
            ioctl(mouse->mseFd, TIOCMBIC, &i);
          }
#endif
        break;

      case P_ACECAD:
	  xf86SetMouseSpeed(mouse, 9600, mouse->baudRate,
			    xf86MouseCflags[mouse->mseType]);
	  /* initialize */
	  /* a nul charactor resets */
	  write(mouse->mseFd, "", 1);
	  usleep(50000);
	  /* stream out relative mode high resolution increments of 1 */
	  write(mouse->mseFd, "@EeI!", 5);
	  break;

#if defined(__FreeBSD__) && defined(MOUSE_PROTO_SYSMOUSE)
      case P_SYSMOUSE:
	if (hw.iftype == MOUSE_IF_SYSMOUSE || hw.iftype == MOUSE_IF_UNKNOWN)
	  xf86SetMouseSpeed(mouse, mouse->baudRate, mouse->baudRate,
                            xf86MouseCflags[mouse->mseType]);
	/* fall through */

      case P_PS2:		/* standard PS/2 mouse */
      case P_BM:		/* bus/InPort mouse */
	mode.rate =
	  (mouse->sampleRate > 0) ? mouse->sampleRate : -1;
	mode.resolution =
	  (mouse->resolution > 0) ? mouse->resolution : -1;
	mode.accelfactor = -1;
	mode.level = -1;
	ioctl(mouse->mseFd, MOUSE_SETMODE, &mode);
	break;
#else
      case P_SYSMOUSE:
	xf86SetMouseSpeed(mouse, mouse->baudRate, mouse->baudRate,
                          xf86MouseCflags[mouse->mseType]);
	break;

      case P_PS2:		/* standard PS/2 mouse */
	ps2param = TRUE;
	break;

      case P_BM:		/* bus/InPort mouse */
	break;
#endif /* __FreeBSD__ */

      case P_IMPS2:		/* IntelliMouse */
	{
	  static unsigned char s[] = { 243, 200, 243, 100, 243, 80, };

	  param = s;
	  paramlen = sizeof(s);
	  ps2param = TRUE;
	}
	break;

      case P_NETPS2:		/* NetMouse, NetMouse Pro, Mie Mouse */
      case P_NETSCROLLPS2:	/* NetScroll */
	{
	  static unsigned char s[] = { 232, 3, 230, 230, 230, };

	  param = s;
	  paramlen = sizeof(s);
	  ps2param = TRUE;
	}
	break;

      case P_MMANPLUSPS2:	/* MouseMan+, FirstMouse+ */
	{
	  static unsigned char s[] = { 230, 232, 0, 232, 3, 232, 2, 232, 1,
				       230, 232, 3, 232, 1, 232, 2, 232, 3, };
	  param = s;
	  paramlen = sizeof(s);
	  ps2param = TRUE;
	}
	break;

      case P_GLIDEPOINTPS2:	/* GlidePoint */
	ps2param = TRUE;
	break;

      case P_THINKINGPS2:	/* ThinkingMouse */
	{
	  static unsigned char s[] = { 243, 10, 232,  0, 243, 20, 243, 60,
				       243, 40, 243, 20, 243, 20, 243, 60,
				       243, 40, 243, 20, 243, 20, };
	  param = s;
	  paramlen = sizeof(s);
	  ps2param = TRUE;
	}
	break;

#ifdef WSCONS_SUPPORT
      case P_WSMOUSE:
	if (mouse->resolution > 0) {
	    /* For wsmouse the resolution is in 0-100, while it's 0-300 for 
	       XF86Config */
	    static int res;
	    res = mouse->resolution/3;
	    if (ioctl(mouse->mseFd, WSMOUSEIO_SRES, &res) < 0) {
		ErrorF("ioctl(WSMOUSEIO_SRES) failed (%s)\n",
		       strerror(errno));
	    }
	}
	break;
#endif

#ifdef USB_MOUSE
      case P_USB:
	mouse->usb = usbMouseInit(mouse);
	break;
#endif
      default:
	xf86SetMouseSpeed(mouse, mouse->baudRate, mouse->baudRate,
                          xf86MouseCflags[mouse->mseType]);
        break;
      }

      if (paramlen > 0)
	{
#ifdef EXTMOUSEDEBUG
	  char c[2];
	  for (i = 0; i < paramlen; ++i)
	    {
	      if (write(mouse->mseFd, &param[i], 1) != 1)
		ErrorF("xf86SetupMouse: Write to mouse failed (%s)\n",
		       strerror(errno));
	      usleep(30000);
	      read(mouse->mseFd, c, 1);
	      ErrorF("xf86SetupMouse: got %02x\n", c[0]);
	    }
#else
	  if (write(mouse->mseFd, param, paramlen) != paramlen)
	    ErrorF("xf86SetupMouse: Write to mouse failed (%s)\n",
	    	   strerror(errno));
#endif
 	  usleep(30000);
 	  xf86FlushInput(mouse->mseFd);
	}
      if (ps2param)
	{
	  unsigned char c[2];

	  c[0] = 230;		/* 1:1 scaling */
	  write(mouse->mseFd, c, 1);
	  c[0] = 244;		/* enable mouse */
	  write(mouse->mseFd, c, 1);
	  c[0] = 243;		/* set sampling rate */
	  if (mouse->sampleRate > 0) 
	    {
 	      if (mouse->sampleRate >= 200)
 		c[1] = 200;
 	      else if (mouse->sampleRate >= 100)
 		c[1] = 100;
 	      else if (mouse->sampleRate >= 80)
 		c[1] = 80;
 	      else if (mouse->sampleRate >= 60)
 		c[1] = 60;
 	      else if (mouse->sampleRate >= 40)
 		c[1] = 40;
 	      else
 		c[1] = 20;
	    }
	  else
	    {
 	      c[1] = 100;
	    }
	  write(mouse->mseFd, c, 2);
	  c[0] = 232;		/* set device resolution */
	  if (mouse->resolution > 0) 
	    {
	      if (mouse->resolution >= 200)
		c[1] = 3;
	      else if (mouse->resolution >= 100)
		c[1] = 2;
	      else if (mouse->resolution >= 50)
		c[1] = 1;
	      else
		c[1] = 0;
	    }
	  else
	    {
	      c[1] = 2;
	    }
	  write(mouse->mseFd, c, 2);
	  usleep(30000);
	  xf86FlushInput(mouse->mseFd);
	}

#endif /* !MOUSE_PROTOCOL_IN_KERNEL || MACH386 */
}
 
#ifndef MOUSE_PROTOCOL_IN_KERNEL
void
xf86MouseProtocol(device, rBuf, nBytes)
    DeviceIntPtr device;
    unsigned char *rBuf;
    int nBytes;
{
  int                  i, buttons, dx, dy, dz;
/*   static int           pBufP = 0;
 *   static unsigned char pBuf[8];
 */
  MouseDevPtr          mouse = MOUSE_DEV(device);
  
#ifdef EXTMOUSEDEBUG
    ErrorF("received %d bytes ",nBytes);
    for ( i=0; i < nBytes; i++)
    	ErrorF("%2x ",rBuf[i]);
    ErrorF("\n");
#endif
  for ( i=0; i < nBytes; i++) {
    /*
     * Hack for resyncing: We check here for a package that is:
     *  a) illegal (detected by wrong data-package header)
     *  b) invalid (0x80 == -128 and that might be wrong for MouseSystems)
     *  c) bad header-package
     *
     * NOTE: b) is a voilation of the MouseSystems-Protocol, since values of
     *       -128 are allowed, but since they are very seldom we can easily
     *       use them as package-header with no button pressed.
     * NOTE/2: On a PS/2 mouse any byte is valid as a data byte. Furthermore,
     *         0x80 is not valid as a header byte. For a PS/2 mouse we skip
     *         checking data bytes.
     *         For resyncing a PS/2 mouse we require the two most significant
     *         bits in the header byte to be 0. These are the overflow bits,
     *         and in case of an overflow we actually lose sync. Overflows
     *         are very rare, however, and we quickly gain sync again after
     *         an overflow condition. This is the best we can do. (Actually,
     *         we could use bit 0x08 in the header byte for resyncing, since
     *         that bit is supposed to be always on, but nobody told
     *         Microsoft...)
     */
    if (mouse->pBufP != 0 &&
#if !defined(__NetBSD__)
	mouse->mseType != P_PS2 &&
#endif
#ifdef WSCONS_SUPPORT
	mouse->mseType != P_WSMOUSE &&
#endif
#ifdef USB_MOUSE
	mouse->mseType != P_USB &&
#endif
	((rBuf[i] & mouse->protoPara[2]) != mouse->protoPara[3] 
	 || rBuf[i] == 0x80))
      {
	mouse->pBufP = 0;          /* skip package */
      }

    if (mouse->pBufP == 0 && (rBuf[i] & mouse->protoPara[0]) != mouse->protoPara[1])
      continue;

    if (mouse->pBufP >= mouse->protoPara[4] 
	&& (rBuf[i] & mouse->protoPara[0]) != mouse->protoPara[1])
      {
	/*
	 * Hack for Logitech MouseMan Mouse - Middle button
	 *
	 * Unfortunately this mouse has variable length packets: the standard
	 * Microsoft 3 byte packet plus an optional 4th byte whenever the
	 * middle button status changes.
	 *
	 * We have already processed the standard packet with the movement
	 * and button info.  Now post an event message with the old status
	 * of the left and right buttons and the updated middle button.
	 */

        /*
	 * Even worse, different MouseMen and TrackMen differ in the 4th
         * byte: some will send 0x00/0x20, others 0x01/0x21, or even
         * 0x02/0x22, so I have to strip off the lower bits. [CHRIS-211092]
         *
         * [JCH-96/01/21]
         * HACK for ALPS "fourth button". (It's bit 0x10 of the "fourth byte"
         * and it is activated by tapping the glidepad with the finger! 8^)
         * We map it to bit bit3, and the reverse map in xf86Events just has
         * to be extended so that it is identified as Button 4. The lower
         * half of the reverse-map may remain unchanged.
	 */

        /*
	 * [KAZU-030897]
	 * Receive the fourth byte only when preceeding three bytes have
	 * been detected (mouse->pBufP >= mouse->protoPara[4]).  In the previous
	 * versions, the test was mouse->pBufP == 0; we may have mistakingly
	 * received a byte even if we didn't see anything preceeding 
	 * the byte.
	 */

	if ((rBuf[i] & mouse->protoPara[5]) != mouse->protoPara[6])
	  {
	    mouse->pBufP = 0;
	    continue;
	  }

	dx = dy = dz = 0;
	buttons = 0;
	switch(mouse->mseType) {

	/*
	 * [KAZU-221197]
	 * IntelliMouse, NetMouse (including NetMouse Pro) and Mie Mouse
	 * always send the fourth byte, whereas the fourth byte is
	 * optional for GlidePoint and ThinkingMouse. The fourth byte 
	 * is also optional for MouseMan+ and FirstMouse+ in their 
	 * native mode. It is always sent if they are in the IntelliMouse 
	 * compatible mode.
	 */ 
	case P_IMSERIAL:	/* IntelliMouse, NetMouse, Mie Mouse, 
				   MouseMan+ */
          dz = (rBuf[i] & 0x08) ? (rBuf[i] & 0x0f) - 16 : (rBuf[i] & 0x0f);
	  buttons |=  ((int)(rBuf[i] & 0x10) >> 3) 
		    | ((int)(rBuf[i] & 0x20) >> 2) 
		    | (mouse->lastButtons & 0x05);
	  break;

	case P_GLIDEPOINT:
	case P_THINKING:
	  buttons |= ((int)(rBuf[i] & 0x10) >> 1);
	  /* fall through */

	default:
	  buttons |= ((int)(rBuf[i] & 0x20) >> 4) | (mouse->lastButtons & 0x05);
	  break;
	}
        mouse->pBufP = 0;
	goto post_event;
      }

    if (mouse->pBufP >= mouse->protoPara[4])
      mouse->pBufP = 0;
    mouse->pBuf[mouse->pBufP++] = rBuf[i];
    if (mouse->pBufP != mouse->protoPara[4]) continue;

    /*
     * assembly full package
     */
    dz = 0;
#ifdef EXTMOUSEDEBUG
    ErrorF("packet %2x %2x %2x %2x\n",mouse->pBuf[0],mouse->pBuf[1],mouse->pBuf[2],mouse->pBuf[3]);
#endif
    switch(mouse->mseType) {
      
    case P_LOGIMAN:	    /* MouseMan / TrackMan   [CHRIS-211092] */
    case P_MS:              /* Microsoft */
      if (mouse->chordMiddle)
	buttons = (((int) mouse->pBuf[0] & 0x30) == 0x30) ? 2 :
		  ((int)(mouse->pBuf[0] & 0x20) >> 3)
		  | ((int)(mouse->pBuf[0] & 0x10) >> 4);
      else
        buttons = (mouse->lastButtons & 2)
		  | ((int)(mouse->pBuf[0] & 0x20) >> 3)
		  | ((int)(mouse->pBuf[0] & 0x10) >> 4);
      dx = (char)(((mouse->pBuf[0] & 0x03) << 6) | (mouse->pBuf[1] & 0x3F));
      dy = (char)(((mouse->pBuf[0] & 0x0C) << 4) | (mouse->pBuf[2] & 0x3F));
      break;

    case P_GLIDEPOINT:      /* ALPS GlidePoint */
    case P_THINKING:        /* ThinkingMouse */
    case P_IMSERIAL:        /* IntelliMouse, NetMouse, Mie Mouse, MouseMan+ */
      buttons =  (mouse->lastButtons & (8 + 2))
		| ((int)(mouse->pBuf[0] & 0x20) >> 3)
		| ((int)(mouse->pBuf[0] & 0x10) >> 4);
      dx = (char)(((mouse->pBuf[0] & 0x03) << 6) | (mouse->pBuf[1] & 0x3F));
      dy = (char)(((mouse->pBuf[0] & 0x0C) << 4) | (mouse->pBuf[2] & 0x3F));
      break;

    case P_MSC:             /* Mouse Systems Corp */
      buttons = (~mouse->pBuf[0]) & 0x07;
      dx =    (char)(mouse->pBuf[1]) + (char)(mouse->pBuf[3]);
      dy = - ((char)(mouse->pBuf[2]) + (char)(mouse->pBuf[4]));
      break;
      
    case P_MMHIT:           /* MM_HitTablet */
      buttons = mouse->pBuf[0] & 0x07;
      if (buttons != 0)
        buttons = 1 << (buttons - 1);
      dx = (mouse->pBuf[0] & 0x10) ?   mouse->pBuf[1] : - mouse->pBuf[1];
      dy = (mouse->pBuf[0] & 0x08) ? - mouse->pBuf[2] :   mouse->pBuf[2];
      break;

    case P_ACECAD:	    /* ACECAD */
	/* ACECAD is almost exactly like MM but the buttons are different */
      buttons = (mouse->pBuf[0] & 0x02) | ((mouse->pBuf[0] & 0x04) >> 2) | ((mouse->pBuf[0] & 1) << 2);
      dx = (mouse->pBuf[0] & 0x10) ?   mouse->pBuf[1] : - mouse->pBuf[1];
      dy = (mouse->pBuf[0] & 0x08) ? - mouse->pBuf[2] :   mouse->pBuf[2];
      break;

    case P_MM:              /* MM Series */
    case P_LOGI:            /* Logitech Mice */
      buttons = mouse->pBuf[0] & 0x07;
      dx = (mouse->pBuf[0] & 0x10) ?   mouse->pBuf[1] : - mouse->pBuf[1];
      dy = (mouse->pBuf[0] & 0x08) ? - mouse->pBuf[2] :   mouse->pBuf[2];
      break;
      
    case P_BM:              /* BusMouse */
#if defined(__NetBSD__)
    case P_PS2:
#endif
      buttons = (~mouse->pBuf[0]) & 0x07;
      dx =   (char)mouse->pBuf[1];
      dy = - (char)mouse->pBuf[2];
      break;

#if !defined(__NetBSD__)
    case P_PS2:             /* PS/2 mouse */
      buttons = (mouse->pBuf[0] & 0x04) >> 1 |       /* Middle */
	        (mouse->pBuf[0] & 0x02) >> 1 |       /* Right */
		(mouse->pBuf[0] & 0x01) << 2;        /* Left */
      dx = (mouse->pBuf[0] & 0x10) ?    mouse->pBuf[1]-256  :  mouse->pBuf[1];
      dy = (mouse->pBuf[0] & 0x20) ?  -(mouse->pBuf[2]-256) : -mouse->pBuf[2];
      break;

    /* PS/2 mouse variants */
    case P_IMPS2:           /* IntelliMouse PS/2 */
    case P_NETPS2:          /* NetMouse PS/2 */
      buttons = (mouse->pBuf[0] & 0x04) >> 1 |       /* Middle */
	        (mouse->pBuf[0] & 0x02) >> 1 |       /* Right */
		(mouse->pBuf[0] & 0x01) << 2;        /* Left */
      dx = (mouse->pBuf[0] & 0x10) ?    mouse->pBuf[1]-256  :  mouse->pBuf[1];
      dy = (mouse->pBuf[0] & 0x20) ?  -(mouse->pBuf[2]-256) : -mouse->pBuf[2];
      dz = (char)mouse->pBuf[3];
      break;

    case P_MMANPLUSPS2:     /* MouseMan+ PS/2 */
      buttons = (mouse->pBuf[0] & 0x04) >> 1 |       /* Middle */
	        (mouse->pBuf[0] & 0x02) >> 1 |       /* Right */
		(mouse->pBuf[0] & 0x01) << 2;        /* Left */
      dx = (mouse->pBuf[0] & 0x10) ?    mouse->pBuf[1]-256  :  mouse->pBuf[1];
      if (((mouse->pBuf[0] & 0x48) == 0x48) &&
	  (abs(dx) > 191) &&
	  ((((mouse->pBuf[2] & 0x03) << 2) | 0x02) == (mouse->pBuf[1] & 0x0f))) {
	/* extended data packet */
	switch ((((mouse->pBuf[0] & 0x30) >> 2) | 
		((mouse->pBuf[1] & 0x30) >> 4))) {
	case 1:		/* wheel data packet */
	  buttons |= ((mouse->pBuf[2] & 0x10) ? 0x08 : 0) | /* fourth button */
		     ((mouse->pBuf[2] & 0x20) ? 0x10 : 0);  /* fifth button */
	  dx = dy = 0;
	  dz = (mouse->pBuf[2] & 0x08) ? (mouse->pBuf[2] & 0x0f) - 16 :
					 (mouse->pBuf[2] & 0x0f);
	  break;
	case 0:		/* device type packet - shouldn't happen */
	case 2:		/* reserved packet - shouldn't happen */
	default:
	  buttons |= (mouse->lastButtons & ~0x07);
	  dx = dy = 0;
	  dz = 0;
	  break;
	}
      } else {
	buttons |= (mouse->lastButtons & ~0x07);
	dx = (mouse->pBuf[0] & 0x10) ?   mouse->pBuf[1]-256  :  mouse->pBuf[1];
	dy = (mouse->pBuf[0] & 0x20) ? -(mouse->pBuf[2]-256) : -mouse->pBuf[2];
      }
      break;

    case P_GLIDEPOINTPS2:   /* GlidePoint PS/2 */
      buttons = (mouse->pBuf[0] & 0x04) >> 1 |       /* Middle */
	        (mouse->pBuf[0] & 0x02) >> 1 |       /* Right */
		(mouse->pBuf[0] & 0x01) << 2 |       /* Left */
		((mouse->pBuf[0] & 0x08) ? 0 : 0x08);/* fourth button */
      dx = (mouse->pBuf[0] & 0x10) ?    mouse->pBuf[1]-256  :  mouse->pBuf[1];
      dy = (mouse->pBuf[0] & 0x20) ?  -(mouse->pBuf[2]-256) : -mouse->pBuf[2];
      break;

    case P_NETSCROLLPS2:    /* NetScroll PS/2 */
      buttons = (mouse->pBuf[0] & 0x04) >> 1 |       /* Middle */
	        (mouse->pBuf[0] & 0x02) >> 1 |       /* Right */
		(mouse->pBuf[0] & 0x01) << 2 |       /* Left */
		((mouse->pBuf[3] & 0x02) ? 0x08 : 0);/* fourth button */
      dx = (mouse->pBuf[0] & 0x10) ?    mouse->pBuf[1]-256  :  mouse->pBuf[1];
      dy = (mouse->pBuf[0] & 0x20) ?  -(mouse->pBuf[2]-256) : -mouse->pBuf[2];
      dz = (mouse->pBuf[3] & 0x10) ? mouse->pBuf[4] - 256 : mouse->pBuf[4];
      break;

    case P_THINKINGPS2:     /* ThinkingMouse PS/2 */
      buttons = (mouse->pBuf[0] & 0x04) >> 1 |       /* Middle */
	        (mouse->pBuf[0] & 0x02) >> 1 |       /* Right */
		(mouse->pBuf[0] & 0x01) << 2 |       /* Left */
		((mouse->pBuf[0] & 0x08) ? 0x08 : 0);/* fourth button */
      mouse->pBuf[1] |= (mouse->pBuf[0] & 0x40) ? 0x80 : 0x00;
      dx = (mouse->pBuf[0] & 0x10) ?    mouse->pBuf[1]-256  :  mouse->pBuf[1];
      dy = (mouse->pBuf[0] & 0x20) ?  -(mouse->pBuf[2]-256) : -mouse->pBuf[2];
      break;

#endif /* !__NetBSD__ */

    case P_SYSMOUSE:        /* sysmouse */
      buttons = (~mouse->pBuf[0]) & 0x07;
      dx =    (char)(mouse->pBuf[1]) + (char)(mouse->pBuf[3]);
      dy = - ((char)(mouse->pBuf[2]) + (char)(mouse->pBuf[4]));
      /* FreeBSD sysmouse sends additional data bytes */
      if (mouse->protoPara[4] >= 8)
	{
          dz = ((char)(mouse->pBuf[5] << 1) + (char)(mouse->pBuf[6] << 1))/2;
          buttons |= (int)(~mouse->pBuf[7] & 0x07) << 3;
	}
      break;

#ifdef WSCONS_SUPPORT
    case P_WSMOUSE: {
      struct wscons_event ev;

      /* copy to guarantee alignment */
      memcpy(&ev, mouse->pBuf, sizeof ev);
      switch (ev.type) {
      case WSCONS_EVENT_MOUSE_UP:
	dx = dy = 0;
#define BUTBIT (1 << (ev.value <= 2 ? 2 - ev.value : ev.value))
	buttons = mouse->lastButtons & ~BUTBIT;
	break;
      case WSCONS_EVENT_MOUSE_DOWN:
	dx = dy = 0;
	buttons = mouse->lastButtons | BUTBIT;
#undef BUTBIT
	break;
      case WSCONS_EVENT_MOUSE_DELTA_X:
	dx = ev.value;
	dy = 0;
	buttons = mouse->lastButtons;
	break;
      case WSCONS_EVENT_MOUSE_DELTA_Y:
	dx = 0;
	dy = -ev.value;
	buttons = mouse->lastButtons;
	break;
#ifdef WSCONS_EVENT_MOUSE_DELTA_Z
      case WSCONS_EVENT_MOUSE_DELTA_Z:
	dx = dy = 0;
	dz = ev.value;
	buttons = mouse->lastButtons;
	break;
#endif
      default:
	ErrorF("wsmouse: bad event type=%d\n", ev.type);
	return;
      }
      break;
      }
#endif /* WSCONS_SUPPORT */

#ifdef USB_MOUSE
    case P_USB:
      usbMouseProtocol(mouse, &dx, &dy, &dz, &buttons);
      break;
#endif

    default: /* There's a table error */
      continue;
    }

post_event:
    /* map the Z axis movement */
    switch (mouse->negativeZ) {
    case 0:	/* do nothing */
      break;
    case MSE_MAPTOX:
      if (dz != 0)
	{
	  dx = dz;
	  dz = 0;
	}
      break;
    case MSE_MAPTOY:
      if (dz != 0)
	{
	  dy = dz;
	  dz = 0;
	}
      break;
    default:	/* buttons */
      buttons &= ~(mouse->negativeZ | mouse->positiveZ);
      if (dz < 0)
	buttons |= mouse->negativeZ;
      else if (dz > 0)
	buttons |= mouse->positiveZ;
      dz = 0;
      break;
    }

    /* post an event */
    xf86PostMseEvent(device, buttons, dx, dy);

    /* 
     * If dz has been mapped to a button `down' event, we need to cook
     * up a corresponding button `up' event.
     */
    if ((mouse->negativeZ > 0) 
	&& (buttons & (mouse->negativeZ | mouse->positiveZ)))
      {
	buttons &= ~(mouse->negativeZ | mouse->positiveZ);
        xf86PostMseEvent(device, buttons, 0, 0);
      }

    /* 
     * We don't reset mouse->pBufP here yet, as there may be an additional data
     * byte in some protocols. See above.
     */
  }
}
#endif /* MOUSE_PROTOCOL_IN_KERNEL */

#ifdef XINPUT

/*
 * xf86MouseCtrl --
 *      Alter the control parameters for the mouse. Note that all special
 *      protocol values are handled by dix.
 */

void
xf86MouseCtrl(device, ctrl)
     DeviceIntPtr device;
     PtrCtrl   *ctrl;
{
    LocalDevicePtr	local = (LocalDevicePtr)(device)->public.devicePrivate;
    MouseDevPtr		mouse = (MouseDevPtr) local->private;    

#ifdef EXTMOUSEDEBUG
    ErrorF("xf86MouseCtrl mouse=0x%x\n", mouse);
#endif
    
    mouse->num       = ctrl->num;
    mouse->den       = ctrl->den;
    mouse->threshold = ctrl->threshold;
}

/*
 ***************************************************************************
 *
 * xf86MouseConfig --
 *
 ***************************************************************************
 */
static Bool
xf86MouseConfig(array, inx, max, val)
    LocalDevicePtr    *array;
    int               inx;
    int               max;
    LexPtr            val;
{
    LocalDevicePtr	dev = array[inx];
    MouseDevPtr		mouse = (MouseDevPtr)dev->private;
   
#ifdef EXTMOUSEDEBUG
    ErrorF("xf86MouseConfig mouse=0x%x\n", mouse);
#endif
    
    configPointerSection(mouse, ENDSUBSECTION, &dev->name);

    return Success;
}

/*
 ***************************************************************************
 *
 * xf86MouseProc --
 *
 ***************************************************************************
 */
static int
xf86MouseProc(device, what)
    DeviceIntPtr	device;
    int			what;
{
    LocalDevicePtr	local = (LocalDevicePtr)device->public.devicePrivate;
    MouseDevPtr		mouse = (MouseDevPtr) local->private;    
    int			fd;
    int			ret;

    mouse->device = device;
    
    ret = xf86MseProcAux(device, what, mouse, &fd, xf86MouseCtrl);
    
    if (what == DEVICE_ON) {
	local->fd = fd;
    } else {
	if ((what == DEVICE_INIT) &&
	    (ret == Success)) {
	    /* allocate the motion history buffer if needed */
	    xf86MotionHistoryAllocate(local);
	    
	    AssignTypeAndName(device, local->atom, local->name);
#ifdef EXTMOUSEDEBUG
	    ErrorF("assigning 0x%x atom=%d name=%s\n", device,
		   local->atom, local->name);
#endif
	}
    }
    
    return ret;
}

/*
 ***************************************************************************
 *
 * xf86MouseReadInput --
 *
 ***************************************************************************
 */
static void
xf86MouseReadInput(local)
    LocalDevicePtr         local;
{
    MouseDevPtr		mouse = (MouseDevPtr) local->private;    

#ifdef EXTMOUSEDEBUG
    ErrorF("xf86MouseReadInput mouse=0x%x\n", mouse);
#endif
    
    mouse->mseEvents(mouse);
}

/*
 ***************************************************************************
 *
 * xf86MouseConvert --
 *	Convert valuators to X and Y.
 *
 ***************************************************************************
 */
static Bool
xf86MouseConvert(LocalDevicePtr	local,
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
    if (first != 0 || num != 2)
      return FALSE;

    *x = v0;
    *y = v1;

    return TRUE;
}

/*
 ***************************************************************************
 *
 * xf86MouseAllocate --
 *
 ***************************************************************************
 */
static LocalDevicePtr
xf86MouseAllocate()
{
    LocalDevicePtr	local = (LocalDevicePtr) xalloc(sizeof(LocalDeviceRec));
    MouseDevPtr		mouse = (MouseDevPtr) xalloc(sizeof(MouseDevRec));

    memset(mouse, 0, sizeof(MouseDevRec));
    
    local->name = "MOUSE";
    local->type_name = "Mouse";
    local->flags = XI86_NO_OPEN_ON_INIT;
    local->device_config = xf86MouseConfig;
    local->device_control = xf86MouseProc;
    local->read_input = xf86MouseReadInput;
    local->motion_history_proc = xf86GetMotionEvents;
    local->history_size = 0;
    local->control_proc = 0;
    local->close_proc = 0;
    local->switch_mode = 0;
    local->conversion_proc = xf86MouseConvert;
    local->reverse_conversion_proc = 0;
    local->fd = -1;
    local->atom = 0;
    local->dev = NULL;
    local->private = mouse;
    local->always_core_feedback = 0;
    
    mouse->device = NULL;
    mouse->mseFd = -1;
    mouse->mseDevice = "";
    mouse->mseType = -1;
    mouse->mseModel = 0;
    mouse->baudRate = -1;
    mouse->oldBaudRate = -1;
    mouse->sampleRate = -1;
    mouse->resolution = 0;
    mouse->buttons = MSE_DFLTBUTTONS; 
    mouse->negativeZ = 0;
    mouse->positiveZ = 0;
    mouse->local = local;
    
#ifdef EXTMOUSEDEBUG
    ErrorF("xf86MouseAllocate mouse=0x%x local=0x%x\n", local->private, local);
#endif
    
    return local;
}

/*
 ***************************************************************************
 *
 * Mouse device association --
 *
 ***************************************************************************
 */
DeviceAssocRec mouse_assoc =
{
  "mouse",				/* config_section_name */
  xf86MouseAllocate			/* device_allocate */
};

#endif /* XINPUT */

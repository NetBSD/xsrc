/*	$NetBSD: decSerMouse.c,v 1.1 2001/09/18 20:02:52 ad Exp $	*/

/*-
 * Copyright (c) 2001 The NetBSD Foundation, Inc.
 * All rights reserved.
 *
 * This code is derived from software contributed to The NetBSD Foundation
 * by Andrew Doran.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the NetBSD
 *	Foundation, Inc. and its contributors.
 * 4. Neither the name of The NetBSD Foundation nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE NETBSD FOUNDATION, INC. AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/*
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

#define NEED_EVENTS
#include "dec.h"

#include <termios.h>
#include <stdio.h>

#include <sys/ttycom.h>

#define	EXTMOUSEDEBUG

void	decSerMouseSetup(decPtrPrivPtr, int);
short	decSerMouseAccelerate(DeviceIntPtr, int);
int	decSerMouseFlushInput(int);
void	decSerMouseSetSpeed(decPtrPrivPtr, int, int, unsigned);

#ifdef notyet
static const char * const decSerMouseTypes[] = {
	"Microsoft",
	"MouseSystems",
	"MMSeries",
	"Logitech",
	"MouseMan",
	"mmhitablet",
	"GlidePoint",
	"IntelliMouse",
	"ThinkingMouse",
	NULL,
};
#endif

/*
 * termios c_cflag settings for each mouse type.
 */
static u_short decSerMouseCflags[] = {
	(CS7                   | CREAD | CLOCAL | HUPCL ), /* MicroSoft */
	(CS8 | CSTOPB          | CREAD | CLOCAL | HUPCL ), /* MouseSystems */
	(CS8 | PARENB | PARODD | CREAD | CLOCAL | HUPCL ), /* MMSeries */
	(CS8 | CSTOPB          | CREAD | CLOCAL | HUPCL ), /* Logitech */
	(CS7                   | CREAD | CLOCAL | HUPCL ), /* MouseMan,
	(CS8                   | CREAD | CLOCAL | HUPCL ), /* mmhitablet */
	(CS7                   | CREAD | CLOCAL | HUPCL ), /* GlidePoint */
	(CS7                   | CREAD | CLOCAL | HUPCL ), /* IntelliMouse */
	(CS7                   | CREAD | CLOCAL | HUPCL ), /* ThinkingMouse */
};

/*
 * Mouse parameters
 */
static u_char proto[][7] = {
  /* hd_mask hd_id dp_mask dp_id bytes b4_mask b4_id */
  {  0x40,   0x40, 0x40,   0x00, 3,   ~0x23,   0x00 },  /* MicroSoft */
  {  0xf8,   0x80, 0x00,   0x00, 5,    0x00,   0xff },  /* MouseSystems */
  {  0xe0,   0x80, 0x80,   0x00, 3,    0x00,   0xff },  /* MMSeries */
  {  0xe0,   0x80, 0x80,   0x00, 3,    0x00,   0xff },  /* Logitech */
  {  0x40,   0x40, 0x40,   0x00, 3,   ~0x23,   0x00 },  /* MouseMan */
  {  0xe0,   0x80, 0x80,   0x00, 3,    0x00,   0xff },  /* MM_HitTablet */
  {  0x40,   0x40, 0x40,   0x00, 3,   ~0x33,   0x00 },  /* GlidePoint */
  {  0x40,   0x40, 0x40,   0x00, 3,   ~0x3f,   0x00 },  /* IntelliMouse */
  {  0x40,   0x40, 0x40,   0x00, 3,   ~0x33,   0x00 },  /* ThinkingMouse */
};

void
decSerMouseSetSpeed(decPtrPrivPtr pPriv, int old, int new, unsigned cflag)
{
	struct termios tty;
	int flags;
	char *c;

	if (tcgetattr(pPriv->fd, &tty) < 0)
		FatalError("%s unable to get status of mouse fd\n",
		    strerror(errno));

	/* this will query the initial baudrate only once */
	if (pPriv->oldBaudRate < 0) { 
	   switch (cfgetispeed(&tty)) 
	      {
	      case B9600: 
		 pPriv->oldBaudRate = 9600;
		 break;
	      case B4800: 
		 pPriv->oldBaudRate = 4800;
		 break;
	      case B2400: 
		 pPriv->oldBaudRate = 2400;
		 break;
	      case B1200: 
	      default:
		 pPriv->oldBaudRate = 1200;
		 break;
	      }
	}

	tty.c_iflag = IGNBRK | IGNPAR;
	tty.c_oflag = 0;
	tty.c_lflag = 0;
	tty.c_cflag = (tcflag_t)cflag;
	tty.c_cc[VTIME] = 0;
	tty.c_cc[VMIN] = 1;

	switch (old)
	{
	case 9600:
		cfsetispeed(&tty, B9600);
		cfsetospeed(&tty, B9600);
		break;
	case 4800:
		cfsetispeed(&tty, B4800);
		cfsetospeed(&tty, B4800);
		break;
	case 2400:
		cfsetispeed(&tty, B2400);
		cfsetospeed(&tty, B2400);
		break;
	case 1200:
	default:
		cfsetispeed(&tty, B1200);
		cfsetospeed(&tty, B1200);
	}

	if (tcsetattr(pPriv->fd, TCSADRAIN, &tty) < 0)
		FatalError("Unable to set status of mouse fd (%s)\n",
			       strerror(errno));

	switch (new)
	{
	case 9600:
		c = "*q";
		cfsetispeed(&tty, B9600);
		cfsetospeed(&tty, B9600);
		break;
	case 4800:
		c = "*p";
		cfsetispeed(&tty, B4800);
		cfsetospeed(&tty, B4800);
		break;
	case 2400:
		c = "*o";
		cfsetispeed(&tty, B2400);
		cfsetospeed(&tty, B2400);
		break;
	case 1200:
	default:
		c = "*n";
		cfsetispeed(&tty, B1200);
		cfsetospeed(&tty, B1200);
	}

	if (pPriv->mseType == SERMSE_LOGIMAN || pPriv->mseType == SERMSE_LOGI)
		if (write(pPriv->fd, c, 2) != 2)
			FatalError("Unable to write to mouse fd (%s)\n",
			    strerror(errno));
	usleep(100000);

	if (tcsetattr(pPriv->fd, TCSADRAIN, &tty) < 0)
	{
		FatalError("Unable to set status of mouse fd (%s)\n",
			       strerror(errno));
	}

#ifdef EXTMOUSEDEBUG
	if (ioctl(pPriv->fd, TIOCMGET, &flags))
		FatalError("Unable to get mouse control lines\n",
		    strerror(errno));
	ErrorF("Mouse control lines: %04x\n", flags);
#endif

	flags = TIOCM_LE | TIOCM_DTR | TIOCM_CTS | TIOCM_CD | TIOCM_RI | TIOCM_RTS | TIOCM_DSR;
	if (ioctl(pPriv->fd, TIOCMSET, &flags))
		FatalError("Unable to set mouse control lines\n",
		    strerror(errno));
}	

int
decSerMouseFlushInput(int fd)
{
	fd_set fds;
	struct timeval timeout;
	char c[4];

	if (tcflush(fd, TCIFLUSH) == 0)
		return 0;

	timeout.tv_sec = 0;
	timeout.tv_usec = 0;
	FD_ZERO(&fds);
	FD_SET(fd, &fds);
	while (select(FD_SETSIZE, &fds, NULL, NULL, &timeout) > 0) {
		read(fd, &c, sizeof(c));
		FD_ZERO(&fds);
		FD_SET(fd, &fds);
	}

	return 0;
}

void
decSerMouseCtrl(DeviceIntPtr device, PtrCtrl *ctrl)
{


}

int
decSerMouseProc(DeviceIntPtr device, int what)
{
    DevicePtr	  pMouse = (DevicePtr) device;
    int	    	  format;
    static int	  oformat;
    BYTE    	  map[4];
    char	  *dev;
    decPtrPrivPtr pPriv;

    switch (what) {
	case DEVICE_INIT:
	    if (pMouse != LookupPointerDevice()) {
		ErrorF ("Cannot open non-system mouse");	
		return !Success;
	    }
	    if (decPtrPriv.fd == -1)
		return !Success;

	    pMouse->devicePrivate = (pointer)&decPtrPriv;
	    pMouse->on = FALSE;

            pPriv = &decPtrPriv;
            pPriv->pBufP = 0;
            pPriv->mseType = decSerMouseType;
            pPriv->chordMiddle = 0;			/* XXX */
            pPriv->sampleRate = 100;			/* XXX */

            decSerMouseSetup(pPriv, decSerMouseBaud);

	    map[1] = 1;
	    map[2] = 2;
	    map[3] = 3;
	    InitPointerDeviceStruct(
		pMouse, map, 3, miPointerGetMotionEvents,
 		decSerMouseCtrl, miPointerGetMotionBufferSize());
	    break;

	case DEVICE_ON:
	    AddEnabledDevice (decPtrPriv.fd);
	    pMouse->on = TRUE;
	    break;

	case DEVICE_CLOSE:
	    break;

	case DEVICE_OFF:
	    pMouse->on = FALSE;
	    RemoveEnabledDevice (decPtrPriv.fd);
	    break;
    }

    return Success;
}

void
decSerMouseSetup(decPtrPrivPtr pPriv, int baudRate)
{

      /*
      ** The following lines take care of the Logitech MouseMan protocols.
      **
      ** NOTE: There are different versions of both MouseMan and TrackMan!
      **       Hence I add another protocol SERMSE_LOGIMAN, which the user can
      **       specify as MouseMan in his XF86Config file. This entry was
      **       formerly handled as a special case of SERMSE_MS. However, people
      **       who don't have the middle button problem, can still specify
      **       Microsoft and use SERMSE_MS.
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
      ** Some additional lines in decSerConfig.c take care of ill configured
      ** baud rates and sample rates. (The user will get an error.)
      **               [CHRIS-211092]
      */

      unsigned char *param;
      int paramlen;
      int i;

	memcpy(pPriv->protoPara, proto[pPriv->mseType], 
	       sizeof(pPriv->protoPara));

      param = NULL;
      paramlen = 0;

      switch (pPriv->mseType) {
      case SERMSE_LOGI:		/* Logitech Mice */
        /* 
	 * The baud rate selection command must be sent at the current
	 * baud rate; try all likely settings 
	 */
	decSerMouseSetSpeed(pPriv, 9600, baudRate,
                          decSerMouseCflags[pPriv->mseType]);
	decSerMouseSetSpeed(pPriv, 4800, baudRate, 
                          decSerMouseCflags[pPriv->mseType]);
	decSerMouseSetSpeed(pPriv, 2400, baudRate,
                          decSerMouseCflags[pPriv->mseType]);
	decSerMouseSetSpeed(pPriv, 1200, baudRate,
                          decSerMouseCflags[pPriv->mseType]);
        /* select MM series data format */
	write(pPriv->fd, "S", 1);
	decSerMouseSetSpeed(pPriv, baudRate, baudRate,
                          decSerMouseCflags[SERMSE_MM]);
        /* select report rate/frequency */
	if      (pPriv->sampleRate <=   0)  write(pPriv->fd, "O", 1);  /* 100 */
	else if (pPriv->sampleRate <=  15)  write(pPriv->fd, "J", 1);  /*  10 */
	else if (pPriv->sampleRate <=  27)  write(pPriv->fd, "K", 1);  /*  20 */
	else if (pPriv->sampleRate <=  42)  write(pPriv->fd, "L", 1);  /*  35 */
	else if (pPriv->sampleRate <=  60)  write(pPriv->fd, "R", 1);  /*  50 */
	else if (pPriv->sampleRate <=  85)  write(pPriv->fd, "M", 1);  /*  67 */
	else if (pPriv->sampleRate <= 125)  write(pPriv->fd, "Q", 1);  /* 100 */
	else                                write(pPriv->fd, "N", 1);  /* 150 */
	break;

      case SERMSE_LOGIMAN:
        decSerMouseSetSpeed(pPriv, 1200, 1200, decSerMouseCflags[pPriv->mseType]);
        write(pPriv->fd, "*X", 2);
        decSerMouseSetSpeed(pPriv, 1200, baudRate,
                          decSerMouseCflags[pPriv->mseType]);
        break;

      case SERMSE_MMHIT:		/* MM_HitTablet */
	{
	  char speedcmd;

	  decSerMouseSetSpeed(pPriv, baudRate, baudRate,
                            decSerMouseCflags[pPriv->mseType]);
	  /*
	   * Initialize Hitachi PUMA Plus - Model 1212E to desired settings.
	   * The tablet must be configured to be in MM mode, NO parity,
	   * Binary Format.  pPriv->sampleRate controls the sensativity
	   * of the tablet.  We only use this tablet for it's 4-button puck
	   * so we don't run in "Absolute Mode"
	   */
	  write(pPriv->fd, "z8", 2);	/* Set Parity = "NONE" */
	  usleep(50000);
	  write(pPriv->fd, "zb", 2);	/* Set Format = "Binary" */
	  usleep(50000);
	  write(pPriv->fd, "@", 1);	/* Set Report Mode = "Stream" */
	  usleep(50000);
	  write(pPriv->fd, "R", 1);	/* Set Output Rate = "45 rps" */
	  usleep(50000);
	  write(pPriv->fd, "I\x20", 2);	/* Set Incrememtal Mode "20" */
	  usleep(50000);
	  write(pPriv->fd, "E", 1);	/* Set Data Type = "Relative */
	  usleep(50000);
	  /* These sample rates translate to 'lines per inch' on the Hitachi
	     tablet */
	  if      (pPriv->sampleRate <=   40) speedcmd = 'g';
	  else if (pPriv->sampleRate <=  100) speedcmd = 'd';
	  else if (pPriv->sampleRate <=  200) speedcmd = 'e';
	  else if (pPriv->sampleRate <=  500) speedcmd = 'h';
	  else if (pPriv->sampleRate <= 1000) speedcmd = 'j';
	  else                                speedcmd = 'd';
	  write(pPriv->fd, &speedcmd, 1);
	  usleep(50000);
	  write(pPriv->fd, "\021", 1);	/* Resume DATA output */
	}
        break;

      case SERMSE_THINKING:		/* ThinkingMouse */
        {
	  fd_set fds;
          char *s;
          char c;

          decSerMouseSetSpeed(pPriv, 1200, baudRate, 
                            decSerMouseCflags[pPriv->mseType]);
          /* this mouse may send a PnP ID string, ignore it */
	  usleep(200000);
	  decSerMouseFlushInput(pPriv->fd);
          /* send the command to initialize the beast */
          for (s = "E5E5"; *s; ++s) {
            write(pPriv->fd, s, 1);
	    FD_ZERO(&fds);
	    FD_SET(pPriv->fd, &fds);
	    if (select(FD_SETSIZE, &fds, NULL, NULL, NULL) <= 0)
	      break;
            read(pPriv->fd, &c, 1);
            if (c != *s)
              break;
          }
        }
	break;

      case SERMSE_MSC:		/* MouseSystems Corp */
	decSerMouseSetSpeed(pPriv, baudRate, baudRate,
                          decSerMouseCflags[pPriv->mseType]);

#ifdef notyet
        if (pPriv->mouseFlags & MF_CLEAR_DTR)
          {
            i = TIOCM_DTR;
            ioctl(pPriv->fd, TIOCMBIC, &i);
          }
        if (pPriv->mouseFlags & MF_CLEAR_RTS)
          {
            i = TIOCM_RTS;
            ioctl(pPriv->fd, TIOCMBIC, &i);
          }
#endif
        break;

      default:
	decSerMouseSetSpeed(pPriv, baudRate, baudRate,
                          decSerMouseCflags[pPriv->mseType]);
        break;
      }

      if (paramlen > 0)
	{
#ifdef EXTMOUSEDEBUG
	  char c[2];
	  for (i = 0; i < paramlen; ++i)
	    {
	      if (write(pPriv->fd, &param[i], 1) != 1)
		ErrorF("decSerSetupMouse: Write to mouse failed (%s)\n",
		       strerror(errno));
	      usleep(30000);
	      read(pPriv->fd, c, 1);
	      ErrorF("decSerSetupMouse: got %02x\n", c[0]);
	    }
#else
	  if (write(pPriv->fd, param, paramlen) != paramlen)
	    ErrorF("decSerSetupMouse: Write to mouse failed (%s)\n",
	    	   strerror(errno));
#endif
 	  usleep(30000);
 	  decSerMouseFlushInput(pPriv->fd);
	}

}

short
decSerMouseAccelerate(DeviceIntPtr device, int delta)
{
    int  sgn = sign(delta);
    PtrCtrl *pCtrl;
    short ret;

    delta = abs(delta);
    pCtrl = &device->ptrfeed->ctrl;
    if (delta > pCtrl->threshold) {
	ret = 
	    (short) sgn * 
		(pCtrl->threshold + ((delta - pCtrl->threshold) * pCtrl->num) /
		    pCtrl->den);
    } else {
	ret = (short) sgn * delta;
    }
    return ret;
}

void
decSerMouseEnqueueEvents(DeviceIntPtr device, u_char *rBuf, int nBytes)
{
    struct timeval tv;
    long ts;
    int i, bmask, buttons, dx, dy, dz;
    decPtrPrivPtr pPriv;
    xEvent xE;

    pPriv = (decPtrPrivPtr)device->public.devicePrivate;

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
    if (pPriv->pBufP != 0 &&
	((rBuf[i] & pPriv->protoPara[2]) != pPriv->protoPara[3] 
	 || rBuf[i] == 0x80))
      {
	pPriv->pBufP = 0;          /* skip package */
      }

    if (pPriv->pBufP == 0 && (rBuf[i] & pPriv->protoPara[0]) != pPriv->protoPara[1])
      continue;

    if (pPriv->pBufP >= pPriv->protoPara[4] 
	&& (rBuf[i] & pPriv->protoPara[0]) != pPriv->protoPara[1])
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
         * We map it to bit bit3, and the reverse map in decSerEvents just has
         * to be extended so that it is identified as Button 4. The lower
         * half of the reverse-map may remain unchanged.
	 */

        /*
	 * [KAZU-030897]
	 * Receive the fourth byte only when preceeding three bytes have
	 * been detected (pPriv->pBufP >= pPriv->protoPara[4]).  In the previous
	 * versions, the test was pPriv->pBufP == 0; we may have mistakingly
	 * received a byte even if we didn't see anything preceeding 
	 * the byte.
	 */

	if ((rBuf[i] & pPriv->protoPara[5]) != pPriv->protoPara[6])
	  {
	    pPriv->pBufP = 0;
	    continue;
	  }

	dx = dy = dz = 0;
	buttons = 0;
	switch(pPriv->mseType) {

	/*
	 * [KAZU-221197]
	 * IntelliMouse, NetMouse (including NetMouse Pro) and Mie Mouse
	 * always send the fourth byte, whereas the fourth byte is
	 * optional for GlidePoint and ThinkingMouse. The fourth byte 
	 * is also optional for MouseMan+ and FirstMouse+ in their 
	 * native mode. It is always sent if they are in the IntelliMouse 
	 * compatible mode.
	 */ 
	case SERMSE_IMSERIAL:	/* IntelliMouse, NetMouse, Mie Mouse, 
				   MouseMan+ */
          dz = (rBuf[i] & 0x08) ? (rBuf[i] & 0x0f) - 16 : (rBuf[i] & 0x0f);
	  buttons |=  ((int)(rBuf[i] & 0x10) >> 3) 
		    | ((int)(rBuf[i] & 0x20) >> 2) 
		    | (pPriv->bmask & 0x05);
	  break;

	case SERMSE_GLIDEPOINT:
	case SERMSE_THINKING:
	  buttons |= ((int)(rBuf[i] & 0x10) >> 1);
	  /* fall through */

	default:
	  buttons |= ((int)(rBuf[i] & 0x20) >> 4) | (pPriv->bmask & 0x05);
	  break;
	}
        pPriv->pBufP = 0;
	goto post_event;
      }


    if (pPriv->pBufP >= pPriv->protoPara[4])
      pPriv->pBufP = 0;
    pPriv->pBuf[pPriv->pBufP++] = rBuf[i];
    if (pPriv->pBufP != pPriv->protoPara[4]) continue;

    /*
     * assembly full package
     */
    dz = 0;
#ifdef EXTMOUSEDEBUG
    ErrorF("packet %2x %2x %2x %2x\n",pPriv->pBuf[0],pPriv->pBuf[1],pPriv->pBuf[2],pPriv->pBuf[3]);
#endif
    switch(pPriv->mseType) {
      
    case SERMSE_LOGIMAN:	    /* MouseMan / TrackMan   [CHRIS-211092] */
    case SERMSE_MS:              /* Microsoft */
      if (pPriv->chordMiddle)
	buttons = (((int) pPriv->pBuf[0] & 0x30) == 0x30) ? 2 :
		  ((int)(pPriv->pBuf[0] & 0x20) >> 3)
		  | ((int)(pPriv->pBuf[0] & 0x10) >> 4);
      else
        buttons = (pPriv->bmask & 2)
		  | ((int)(pPriv->pBuf[0] & 0x20) >> 3)
		  | ((int)(pPriv->pBuf[0] & 0x10) >> 4);
      dx = (char)(((pPriv->pBuf[0] & 0x03) << 6) | (pPriv->pBuf[1] & 0x3F));
      dy = (char)(((pPriv->pBuf[0] & 0x0C) << 4) | (pPriv->pBuf[2] & 0x3F));
      break;

    case SERMSE_GLIDEPOINT:      /* ALPS GlidePoint */
    case SERMSE_THINKING:        /* ThinkingMouse */
    case SERMSE_IMSERIAL:        /* IntelliMouse, NetMouse, Mie Mouse, MouseMan+ */
      buttons =  (pPriv->bmask & (8 + 2))
		| ((int)(pPriv->pBuf[0] & 0x20) >> 3)
		| ((int)(pPriv->pBuf[0] & 0x10) >> 4);
      dx = (char)(((pPriv->pBuf[0] & 0x03) << 6) | (pPriv->pBuf[1] & 0x3F));
      dy = (char)(((pPriv->pBuf[0] & 0x0C) << 4) | (pPriv->pBuf[2] & 0x3F));
      break;

    case SERMSE_MSC:             /* Mouse Systems Corp */
      buttons = (~pPriv->pBuf[0]) & 0x07;
      dx =    (char)(pPriv->pBuf[1]) + (char)(pPriv->pBuf[3]);
      dy = - ((char)(pPriv->pBuf[2]) + (char)(pPriv->pBuf[4]));
      break;
      
    case SERMSE_MMHIT:           /* MM_HitTablet */
      buttons = pPriv->pBuf[0] & 0x07;
      if (buttons != 0)
        buttons = 1 << (buttons - 1);
      dx = (pPriv->pBuf[0] & 0x10) ?   pPriv->pBuf[1] : - pPriv->pBuf[1];
      dy = (pPriv->pBuf[0] & 0x08) ? - pPriv->pBuf[2] :   pPriv->pBuf[2];
      break;

    case SERMSE_MM:              /* MM Series */
    case SERMSE_LOGI:            /* Logitech Mice */
      buttons = pPriv->pBuf[0] & 0x07;
      dx = (pPriv->pBuf[0] & 0x10) ?   pPriv->pBuf[1] : - pPriv->pBuf[1];
      dy = (pPriv->pBuf[0] & 0x08) ? - pPriv->pBuf[2] :   pPriv->pBuf[2];
      break;
      
    default: /* There's a table error */
      continue;
    }

post_event:
    /* XXX */
    gettimeofday(&tv, NULL);
    ts = TVTOMILLI(tv);

    if (dx != 0)
        miPointerDeltaCursor(decSerMouseAccelerate(device, dx), 0, ts);
    if (dy != 0)
        miPointerDeltaCursor(0, decSerMouseAccelerate(device, dx), ts);
    for (bmask = 1, i = 0; i < 3; i++, bmask <<= 1) {
        if ((pPriv->bmask & bmask) != (buttons & bmask)) {
            xE.u.keyButtonPointer.time = ts;
            xE.u.u.type = (buttons & bmask) ? ButtonPress : ButtonRelease;
            xE.u.u.detail = i;
            mieqEnqueue(&xE);
        }
    }
    pPriv->bmask = buttons;

    /* 
     * We don't reset pPriv->pBufP here yet, as there may be an additional data
     * byte in some protocols. See above.
     */
  }
}

u_char *decSerMouseGetEvents(fd, pNumBytes, pAgain)
    int		fd;
    int*	pNumBytes;
    Bool*	pAgain;
{
    int	    	  nBytes;	   	 /* number of bytes of events available. */
    static u_char evBuf[MAXEVENTS * 4];	 /* Buffer for event data */

    if ((nBytes = read(fd, (char *)evBuf, sizeof(evBuf))) == -1) {
	if (errno == EWOULDBLOCK) {
	    *pNumBytes = 0;
	    *pAgain = FALSE;
	} else {
	    Error("decSerMouseGetEvents read");
	    FatalError("Could not read from mouse");
	}
    } else {
	*pNumBytes = nBytes;
	*pAgain = (nBytes == sizeof(evBuf));
    }

    return evBuf;
}

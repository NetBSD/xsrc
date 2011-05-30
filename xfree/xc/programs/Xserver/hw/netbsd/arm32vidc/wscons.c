/*	$NetBSD: wscons.c,v 1.6 2011/05/30 15:31:56 christos Exp $	*/

/*-
 * Copyright (c) 2001 Ben Harris
 * Copyright (c) 1999 Mark Brinicombe & Neil A. Carson 
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * X11 driver code for VIDC20
 *
 */

#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

/* X11 headers
 */
#include "X.h"
#include "Xproto.h"
#include "screenint.h"
#include "input.h"
#include "cursor.h"
#include "misc.h"
#include "mi.h"
#include "scrnintstr.h"
#include "servermd.h"
#include "mipointer.h"
#include "colormap.h"
#include "colormapst.h"
#include "resource.h"
#include "wscons.h"

/* NetBSD headers wscons specific */
#include <dev/wscons/wsconsio.h>

/* Keymap, from XFree86*/
#include "atKeynames.h"

/* Our private translation table to work around the missing 8042 */
extern int kbdmap[], kbdmap1[], kbdmap2[];

/* Our private definitions */
#include "private.h"

/* #define DEBUG */

#ifdef DEBUG
#define DPRINTF(x) ErrorF x
#else
#define DPRINTF(x)
#endif

#define MOUSE_PATH	"/dev/wsmouse0"
#define KBD_PATH	"/dev/wskbd0"
#define CON_PATH	"/dev/ttyE0"

extern struct _private private;

void
wsdisplay_write_palette(int c, int r, int g, int b)
{
	struct wsdisplay_cmap map;
	u_char mapr, mapg, mapb;

	DPRINTF(("wsdisplay_write_palette: %d => #%02x%02x%02x\n",
	    c, r, g, b));	   
	mapr = r; mapg = g; mapb = b;
	map.index = c;
	map.count = 1;
	map.red = &mapr; map.green = &mapg; map.blue = &mapb;
	ioctl(private.wsdisplay_fd, WSDISPLAYIO_PUTCMAP, &map);
}

void wscons_bell(int percent, DeviceIntPtr device, pointer ctrl, int unused)
{
	KeybdCtrl *kctrl = (KeybdCtrl *)ctrl;
	DPRINTF(("Bell\n"));

	if (percent == 0 || kctrl->bell == 0)
		return;

	if (private.wskbd_fd >= 0)
		ioctl(private.wskbd_fd, WSKBDIO_BELL, NULL);
}

#define TSTOMILLI(ts)	((ts).tv_nsec / 1000000 + (ts).tv_sec * 1000)

void wsmouse_io(void)
{
	struct wscons_event wsev;
	xEvent x_event;
	DeviceIntPtr device;

	device = (DeviceIntPtr)LookupPointerDevice();
	while (read(private.wsmouse_fd, &wsev, sizeof(wsev)) > 0) {
		/* Get the time of the event as near as possible */
		x_event.u.keyButtonPointer.time = TSTOMILLI(wsev.time);

		switch (wsev.type) {
		case WSCONS_EVENT_MOUSE_UP:
			x_event.u.u.type = ButtonRelease;
			x_event.u.u.detail = wsev.value+1;
			mieqEnqueue(&x_event);
			break;
		case WSCONS_EVENT_MOUSE_DOWN:
			x_event.u.u.type = ButtonPress;
			x_event.u.u.detail = wsev.value+1;
			mieqEnqueue(&x_event);
			break;
		case WSCONS_EVENT_MOUSE_DELTA_X:
			miPointerDeltaCursor(mouse_accel(device, wsev.value), 0,
			    TSTOMILLI(wsev.time));
			break;
		case WSCONS_EVENT_MOUSE_DELTA_Y:
			/* y coords diffs from wscons are negated */
			miPointerDeltaCursor(0, mouse_accel(device, -wsev.value),
			    TSTOMILLI(wsev.time));
			break;
		default:
			/* Do anything with other events? */
			break;
		}
	}
}

void wskbd_io(void)
{
	xEvent x_event;
	struct wscons_event wsev;
	static int controlmask = 0;

	while (read(private.wskbd_fd, &wsev, sizeof(wsev)) > 0)
	{
		/* Get the time of the event as near as possible */
		x_event.u.keyButtonPointer.time = TSTOMILLI(wsev.time);

		/* Was it a press or a release ? */
		switch (wsev.type) {
		case WSCONS_EVENT_KEY_UP:
			x_event.u.u.type = KeyRelease;
			break;
		case WSCONS_EVENT_KEY_DOWN:
			x_event.u.u.type = KeyPress;
			break;
		default:
			continue;
		}

#ifdef WSKBD_TYPE_RISCPC
		if (private.wskbd_type == WSKBD_TYPE_RISCPC) {
			/*
			 * Ok we now have a kludge to map the RiscPC
			 * raw keycodes to AT keycodes. We don't get
			 * AT keycodes as the RiscPC does not have a
			 * 8042 keyboard controller to translate the
			 * raw codes from the keyboard.  We do the
			 * translation here so that we can then use
			 * existing PC keyboard mapping info.  This is
			 * really just a hack as I don't want to spend
			 * time now figuring out the correct mappings
			 * when I can borrow existing AT ones.
			 */
			DPRINTF(("wscons code = 0x%x\n", wsev.value));
			if (wsev.value < 0x90 && kbdmap[wsev.value] != -1)
				x_event.u.u.detail = kbdmap[wsev.value];
			else if (wsev.value > 0x110 && wsev.value < 0x115
			    && kbdmap1[(wsev.value - 0x110)] != -1)
				x_event.u.u.detail =
				    kbdmap1[(wsev.value - 0x110)];
			else if (wsev.value > 0x140 && wsev.value < 0x180
			    && kbdmap2[(wsev.value - 0x140)] != -1)
				x_event.u.u.detail =
				    kbdmap2[(wsev.value - 0x140)];
			else
				continue;
			DPRINTF(("X11 code = 0x%x\n", x_event.u.u.detail));
		} else
#endif
		{
			/* Assume WSKBD_TYPE_PC_XT, i.e. no translation. */
			x_event.u.u.detail = wsev.value & 0x7f;
		}

		/*
		 * Bit of hackery to provide a Xserver kill hot key
		 *
		 * Could also be used to enable debug etc.etc.
		 */
		if (x_event.u.u.type == KeyPress) {
			if (x_event.u.u.detail == KEY_BackSpace)
				controlmask |= 1;
			else if (x_event.u.u.detail == KEY_RCtrl)
				controlmask |= 2;
			else if (x_event.u.u.detail == KEY_AltLang)
				controlmask |= 4;
		} else if (x_event.u.u.type == KeyRelease) {
			if (x_event.u.u.detail == KEY_BackSpace)
				controlmask &= ~1;
			else if (x_event.u.u.detail == KEY_RCtrl)
				controlmask &= ~2;
			else if (x_event.u.u.detail == KEY_AltLang)
				controlmask &= ~4;
		}

		if ((controlmask & 7) == 7)
			GiveUp(0);

		/* Enqueue the event */
		x_event.u.u.detail += MIN_KEYCODE;
		mieqEnqueue(&x_event);
	}
}

int
wsmouse_init(void)
{
	int fd;
	int len;
	struct wscons_event wsev;

	if ((fd = open(MOUSE_PATH, O_RDONLY | O_NONBLOCK)) < 0)
		return -1;

	/* Drain the mouse buffer */
	do {
		len = read(fd, &wsev, sizeof(wsev));
	} while (len > 0);

	return fd;
}

int
wskbd_init(void)
{
	int fd;
	int len;
	struct wscons_event wsev;
	u_int kbdtype;

	DPRINTF(("Opening keyboard\n"));
	if ((fd = open(KBD_PATH, O_RDONLY | O_NONBLOCK)) < 0) {
		DPRINTF(("Keyboard open failed: %s\n", strerror(errno)));
		return -1;
	}

	/* Check that this is a Risc PC keyboard. */
	if (ioctl(fd, WSKBDIO_GTYPE, &kbdtype) != 0) {
		ErrorF("Couldn't get keyboard type\n");
		FatalError((char *)sys_errlist[errno]);
	}
	if (
#ifdef WSKBD_TYPE_RISCPC
	    kbdtype != WSKBD_TYPE_RISCPC &&
#endif
	    kbdtype != WSKBD_TYPE_PC_XT) {
		ErrorF("%s is not a "
#ifdef WSKBD_TYPE_RISCPC
		    "Risc PC or "
#endif
		    "PC/XT keyboard\n", KBD_PATH);
		close(fd);
		return -1;
	}
	private.wskbd_type = kbdtype;

	/* Drain the keyboard buffer */
	do {
		len = read(private.wskbd_fd, &wsev, sizeof(wsev));
	} while (len > 0);

	return fd;
}


int
wsdisplay_init(ScreenPtr screen, int argc, char **argv)
{
	int cnt;
	struct wsdisplay_fbinfo fbinfo;
	int btime;
	int nconsole;
	u_int disptype;
	u_int const newmode = WSDISPLAYIO_MODE_MAPPED;

	DPRINTF(("vidc_init_screen\n"));

	if ((private.wsdisplay_fd = open(CON_PATH, O_RDONLY | O_NONBLOCK)) < 0)
		return FALSE;

	/* Check that this is a VIDC display. */
	if (ioctl(private.wsdisplay_fd, WSDISPLAYIO_GTYPE, &disptype) != 0) {
		ErrorF("Couldn't get display type\n");
		FatalError((char *)sys_errlist[errno]);
	}
	if (disptype != WSDISPLAY_TYPE_VIDC) {
		ErrorF("%s is not a VIDC display\n", CON_PATH);
		close(private.wsdisplay_fd);
		private.wsdisplay_fd = -1;
		return FALSE;
	}

	if (ioctl(private.wsdisplay_fd, WSDISPLAYIO_SMODE, &newmode) != 0) {
		ErrorF("Couldn't put console in mapped mode\n");
		FatalError((char *)sys_errlist[errno]);
	}

	if (ioctl(private.wsdisplay_fd, WSDISPLAYIO_GINFO, &fbinfo) != 0) {
		ErrorF("Couldn't get console info for console\n");
		FatalError((char *)sys_errlist[errno]);
	}

	ErrorF("Frame buffer %d x %d x %d\n", fbinfo.width, fbinfo.height,
	    fbinfo.depth);

	if ((private.vram_fd = open(CON_PATH, O_RDWR | O_NONBLOCK, 0)) < 0) {
		FatalError("Unable to open %s\n", CON_PATH);
		return FALSE;
	}

	private.xres = fbinfo.width;
	private.yres = fbinfo.height;
	private.depth = fbinfo.depth;
	private.width = (fbinfo.width * fbinfo.depth) / 8;

	return TRUE;
}

void
wsdisplay_closedown(void)
{
	const u_int newmode = WSDISPLAYIO_MODE_EMUL;

	if (ioctl(private.wsdisplay_fd, WSDISPLAYIO_SMODE, &newmode) != 0) {
		ErrorF("Couldn't put console in emulation mode\n");
		FatalError((char *)sys_errlist[errno]);
	}

}

void
wskbd_closedown(void)
{
	const int newmode = WSKBD_TRANSLATED;

	if (ioctl(private.wsdisplay_fd, WSKBDIO_SETMODE, &newmode) != 0) {
		ErrorF("Couldn't put keyboard in emulation mode\n");
		FatalError((char *)sys_errlist[errno]);
	}

}


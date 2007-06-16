/*	$NetBSD: private.h,v 1.8 2007/06/16 15:52:46 pavel Exp $	*/

/*
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

/*
 * Use __NetBSD_Version__ to work out which of the old console devices
 * still exist.
 */
#include <sys/param.h>
#if (__NetBSD_Version__ < 499000100 && __NetBSD_Version__ >= 499000000) || __NetBSD_Version__ < 400000001
/* <machine/kbd.h> and <machine/vconsole.h> removed in 4.99.1 */
#define HAVE_KBD
#define HAVE_VCONSOLE
#endif
#if __NetBSD_Version__ < 106370000 /* <machine/mouse.h> removed in 1.6ZK */
#define HAVE_BUSMOUSE
#endif
#if __NetBSD_Version__ < 106350000 /* <machine/beep.h> removed in 1.6ZI */
#define HAVE_BEEP
#endif

/*
 * For each screen, we should allocate the following and store it in the
 * private area. To get something working, however, we don't :-(
 */
struct _private
{
	int xres;		/* X res of frame buffer */
	int yres;		/* Y res of frame buffer */
	int depth;		/* depth of frame buffer */
	int width;		/* width of frame buffer */

	int vram_fd;		/* Screen file descriptor for frame buffer */
#ifdef HAVE_BUSMOUSE
	int mouse_fd;		/* File descriptor for pms/qms */
#endif
	int wsmouse_fd;		/* File descriptor for wsmouse */
#ifdef HAVE_KBD
	int kbd_fd;		/* File descriptor for kbd */
#endif
	int wskbd_fd;		/* File descriptor for wskbd */
	u_int wskbd_type;	/* Keyboard type from WSKBDIO_GTYPE */
	int con_fd;		/* File descriptor for the console */
	int wsdisplay_fd;	/* File descriptor for wsdisplay */
#ifdef HAVE_BEEP
	int beep_fd;		/* File descriptor for beep */
#endif
	char *vram_base;	/* Where the screen has been mapped to */
	DevicePtr mouse_dev;	/* X device for mouse */
	DevicePtr kbd_dev;	/* X device for keyboard */
	ColormapPtr colour_map;	/* Active colour map for this screen */
#ifdef HAVE_VCONSOLE
	int rpc_origvc;
#endif
};

/* Prototypes */
void vidc_mousectrl();
void vidc_kbdctrl();
void vidc_bell();
void wscons_bell();

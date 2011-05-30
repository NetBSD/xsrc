/*	$NetBSD: vidc.c,v 1.11 2011/05/30 15:31:56 christos Exp $	*/

/*
 * Copyright (c) 1999 Neil A. Carson & Mark Brinicombe
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
 */

/*
 * This is in a severely hacked up state and is a rush job to implement
 * a X server for the RiscPC.
 *
 * Most of the mouse and keyboard specific code has been separated into
 * a separate file to try and make things simpler when we support wscons
 * as well.
 *
 * A lot of cleanup is already being worked on.
 *
 * Some of this serves as a good example of how not to write code ;-))
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
#include "Xos.h"
#include "X.h"
#include "Xproto.h"
#include "Xmd.h"
#ifndef PSZ
#define PSZ 8
#endif
#include "cfb.h"
#undef PSZ
#include "cfb16.h"
#include "cfb32.h"
#include "colormap.h"
#include "colormapst.h"
#include "cursor.h"
#include "dixfontstr.h"
#include "fontstruct.h"
#include "gcstruct.h"
#include "input.h"
#include "inputstr.h"
#include "mi.h"
#include "mifillarc.h" 
#include "mipointer.h"
#include "misc.h"
#include "mistruct.h"
#include "pixmapstr.h"
#include "regionstr.h"
#include "resource.h"
#include "scrnintstr.h"
#include "servermd.h"
#include "wscons.h"

/* #define DEBUG */

#ifdef DEBUG
#define DPRINTF(x) ErrorF x
#else
#define DPRINTF(x)
#endif

/* Keymap, from XFree86*/
#include "xf86_keymap.h"

/* Our private definitions */
#include "private.h"

/* This is horrible */
#define SCREEN_BPP	8		/* Colour depth of screen */
#define SCREEN_DPI_X	75		/* Horizontal dots per inch */
#define SCREEN_DPI_Y	75		/* Vertical dots per inch */

/* Macro to make a null function, given a name! */
#define NULL_FUNC(_n)void _n(){}

/* We want these funcitons to do nothing */
NULL_FUNC(mouse_cross_screen);
NULL_FUNC(OsVendorPreInit);
NULL_FUNC(OsVendorInit);

struct _private private;

static void
write_palette(int c, int r, int g, int b)
{

	DPRINTF(("write_palette: wsdisplay_fd = %d, c = %d\n",
	    private.wsdisplay_fd, c));
	if (private.wsdisplay_fd >= 0)
		wsdisplay_write_palette(c, r, g, b);
#ifdef HAVE_VCONSOLE
	else
		rpccons_write_palette(c, r, g, b);
#endif /* HAVE_VCONSOLE */
}

/*
 * Install a colour map
 */
static void install_colour_map(ColormapPtr map)
{
	unsigned int cnt;

	DPRINTF(("install_colour_map visual %d %d\n", map->pVisual->class,
	map->pVisual->nplanes));
	/* If this colour map is already installed, bail */
	if ((map == private.colour_map) && private.colour_map)
		return;

	/* Chuck an event if we're losing a currently installed map */
	if (private.colour_map)
		WalkTree(private.colour_map->pScreen, TellLostMap,
		    (pointer) &private.colour_map->mid);

	/* Set the colours*/
	if ((map->pVisual->class == PseudoColor
	    || map->pVisual->class == GrayScale)
	    && map->pVisual->nplanes == 8) {
		for (cnt = 0; cnt < map->pVisual->ColormapEntries; cnt ++)
			if (map->red->fShared) {
				write_palette(cnt,
				    map->red[cnt].co.shco.red->color >> 8,
				    map->red[cnt].co.shco.green->color >> 8,
				    map->red[cnt].co.shco.blue->color >> 8);
			} else {
				write_palette(cnt,
				    map->red[cnt].co.local.red >> 8,
				    map->red[cnt].co.local.green >> 8,
				    map->red[cnt].co.local.blue >> 8);
			}
	} else if (map->pVisual->class == TrueColor
	    && map->pVisual->nplanes == 16) {
		/*
		 * For the moment we ignore the colour info from the visual
		 * and hardcode our 16bpp palette.
		 *
		 * Please read the Vidc20 data sheet for info on how
		 * the 16bits map to the RGB LUTs
		 */
/*		for (cnt = 0; cnt < map->pVisual->ColormapEntries; cnt ++) {
			write_palette(cnt,
			    map->red[cnt].co.local.red.color >> 8,
			    map->green[cnt].co.local.green.color >> 8,
			    map->blue[cnt].co.local.blue.color >> 8);
		}*/
		for (cnt = 0; cnt < 256; ++cnt)
			write_palette(cnt, (cnt & 0x3f) << 2,
			     (cnt & 0x7c) << 1, (cnt & 0xf8));
	}

	/* Change private colour map pointer, communicate chances and return. */
	private.colour_map = map;
	WalkTree(map->pScreen, TellGainedMap, (pointer) &map->mid);
}

/* Remove a colour map, plopping back the default one if needed.
 */
static void uninstall_colour_map(ColormapPtr map)
{
	Colormap default_map_id;

	DPRINTF(("uninstall_colour_map\n"));
	if (map != private.colour_map)
		return;
	default_map_id = map->pScreen->defColormap;
	if (map->mid != default_map_id)
	{
		ColormapPtr default_map;
		
		default_map = (ColormapPtr) LookupIDByType(default_map_id,
		    RT_COLORMAP);
		if (default_map)
			(*map->pScreen->InstallColormap)(default_map);
		else
			FatalError("Can't find default colour map\n");
	}
}

/* List all installed colour maps
 */
static int list_installed_colour_maps(ScreenPtr screen, Colormap *map_list)
{
	DPRINTF(("list_installed_colour_maps\n"));
	if (private.colour_map)
		*map_list = private.colour_map->mid;
	return 1;
}

/* Store colours
 */
static void store_colours(ColormapPtr map, int colours, xColorItem *defs)
{
	DPRINTF(("store_colours\n"));
	DPRINTF(("map = %p, private.colour_map = %p\n", map, private.colour_map));
	DPRINTF(("colours=%d\n", colours));
	if (private.colour_map && private.colour_map != map)
		return;

	while (colours --)
	{
		write_palette(defs->pixel, defs->red >> 8, defs->green >> 8,
		    defs->blue >> 8);
		defs ++;
	}
}

#ifdef DPMSExtension

/* No way will we support this
 */
Bool DPMSSupported(void)
{
	return FALSE;
}

void DPMSSet(CARD16 level)
{
}

#endif

/* Tell us if the mouse is off the screen or not. I'm unclear as to what
 * this has to do, how it fits in with Active Zaphods etc.
 */
static Bool mouse_off_screen(ScreenPtr *screen, int *x, int *y)
{
	extern Bool PointerConfinedToScreen(void);

	if (PointerConfinedToScreen())
		return TRUE;
	return FALSE;
}

/* Call the MI pointer warp function, as we're not using a hardware
 * cursor. Block the SIGIO signal while doing this in order make
 * sure we don't get any races.
 */
static void mouse_warp_cursor(ScreenPtr screen, int x, int y)
{
	sigset_t newsigmask;

	sigemptyset(&newsigmask);
	sigaddset(&newsigmask, SIGIO);
	sigprocmask(SIG_BLOCK, &newsigmask, 0);
	miPointerWarpCursor(screen, x, y);
	sigprocmask(SIG_UNBLOCK, &newsigmask, 0);
}

miPointerScreenFuncRec vidc_mouse_funcs =
{
	mouse_off_screen,
	mouse_cross_screen,
	mouse_warp_cursor,
};

/* Mouse driver
 */
int vidc_mouse(DevicePtr dev, int what)
{
	BYTE map[4] = {0, 1, 2, 3};

	switch (what) {
	case DEVICE_INIT:
		if (dev != LookupPointerDevice()) {
			FatalError("Wrong device in mouse\n");
			return (!Success);
		}
		InitPointerDeviceStruct(dev, map, 3,
		    miPointerGetMotionEvents, vidc_mousectrl,
		    miPointerGetMotionBufferSize());
		break;
	case DEVICE_ON:
		dev->on = TRUE;
		break;
	case DEVICE_OFF:
		dev->on = FALSE;
		break;
	case DEVICE_CLOSE:
		/* nothing! */
		break;
	default:
		break;
	}
	return Success;
}

/* Key symbols record for some kind of keyboard :-(
 */
/*KeySymsRec keysims[] = {map, 0, MAX_STD_KEYCODE, 4};*/

/* Modifier map definition
 */
struct
{
	BYTE	key;
	CARD8	modifiers;
} vidc_modmap[] =
{
	{ KEY_ShiftL,	ShiftMask },
	{ KEY_ShiftR,	ShiftMask },
	{ KEY_CapsLock,	LockMask },
	{ KEY_LCtrl,	ControlMask },
	{ KEY_RCtrl,	ControlMask },
	{ KEY_Alt,	Mod1Mask },
	{ KEY_AltLang,	Mod1Mask },
	{ 0,		0}
};

/* Keyboard driver
 */
#define NUM_STD_KEYCODES 127
#define MIN_KEYCODE     8
#define MAX_STD_KEYCODE (NUM_STD_KEYCODES + MIN_KEYCODE - 1)
static int vidc_kbd(DevicePtr dev, int what)
{
	static CARD8 *modmap = 0;
	static KeySymsRec keysims = {map, 0, MAX_STD_KEYCODE, 4};
	unsigned int cnt;

	switch (what)
	{
		case DEVICE_INIT:
			if (dev != LookupKeyboardDevice())
			{
				FatalError("Wrong device in keyboard\n");
				return (!Success);
			}
			if (!modmap)
			{
				if (keysims.minKeyCode < MIN_KEYCODE)
				{
					keysims.minKeyCode += MIN_KEYCODE;
					keysims.maxKeyCode += MIN_KEYCODE;
				}
	 			if (keysims.maxKeyCode > MAX_KEYCODE)
					keysims.maxKeyCode = MAX_KEYCODE;
				modmap = (CARD8 *) xalloc(MAP_LENGTH);
				memset(modmap, 0, MAP_LENGTH);
				for (cnt = 0; vidc_modmap[cnt].key != 0; cnt ++)
					modmap[vidc_modmap[cnt].key + MIN_KEYCODE] =
						vidc_modmap[cnt].modifiers;
			}
			InitKeyboardDeviceStruct(dev, &keysims, modmap,
#ifdef HAVE_BEEP
			    private.wskbd_fd == -1 ? vidc_bell :
#endif /* HAVE_BEEP */
			    wscons_bell, vidc_kbdctrl);
			break;
		case DEVICE_ON:
			dev->on = TRUE;
			break;
		case DEVICE_OFF:
			dev->on = FALSE;
			break;
		default:
			break;
	}
	return Success;
}

int mouse_accel(DeviceIntPtr device, int delta)
{
	int res;
	PtrCtrl *ptrctrl = &device->ptrfeed->ctrl;

	if (delta > 0)
		res = 1;
	else {
		res = -1;
		delta = -delta;
	}

	if (delta > ptrctrl->threshold)
		res *= (ptrctrl->threshold + ((delta - ptrctrl->threshold)
		    * ptrctrl->num) / ptrctrl->den);

	else
		res *= delta;

	return res;
}

/*
 * Handler for SIGIO. Called when either the mouse or keyboard fd are
 * read for I/O
 */
static void sigio_handler(int flags)
{
#ifdef HAVE_BUSMOUSE
	if (private.mouse_fd >= 0)
		rpc_mouse_io();
#endif /* HAVE_BUSMOUSE */
	if (private.wsmouse_fd >= 0)
		wsmouse_io();
#ifdef HAVE_KBD
	if (private.kbd_fd >= 0)
		rpc_kbd_io();
#endif
	if (private.wskbd_fd >= 0)
		wskbd_io();
}

/* Start input devices
 */
void InitInput(int argc, char *argv[])
{
	DeviceIntPtr mouse, keyboard;

	DPRINTF(("InitInput\n"));

#ifdef HAVE_BUSMOUSE
	private.mouse_fd = -1;
#endif /* HAVE_BUSMOUSE */
	private.wsmouse_fd = -1;
#ifdef HAVE_KBD
	private.kbd_fd = -1;
#endif /* HAVE_KBD */
	private.wskbd_fd = -1;
#ifdef HAVE_BUSMOUSE
	private.beep_fd = -1;
#endif /* HAVE_BUSMOUSE */

	/* Try to init the wsmouse device */
	private.wsmouse_fd = wsmouse_init();
#ifndef HAVE_BUSMOUSE
	if (private.wsmouse_fd == -1)
		FatalError("Cannot open mouse device\n");
#else /* HAVE_BUSMOUSE */
	if (private.wsmouse_fd == -1) {
		/* Try and init the old rpc mouse device */
		private.mouse_fd = rpc_init_mouse();
		if (private.mouse_fd == -1)
			FatalError("Cannot open mouse device\n");
	}
#endif /* HAVE_BUSMOUSE */
	
	/* ... and wskbd */
	private.wskbd_fd = wskbd_init();
#ifdef HAVE_KBD
	if (private.wskbd_fd == -1) {
		/* Try and init the old rpc kbd device */
		private.kbd_fd = rpc_init_kbd();
		if (private.kbd_fd == -1)
			FatalError("Cannot open kbd device\n");
#ifdef HAVE_BEEP
		/* Try and init the old rpc beep device */
		private.beep_fd = rpc_init_bell();
		if (private.beep_fd == -1)
			ErrorF("Cannot open beep device\n");
#endif /* HAVE_BEEP */
	}
#endif /* HAVE_KBD */

	/* Add the input devices */
	mouse = AddInputDevice((DeviceProc) vidc_mouse, TRUE);
	keyboard = AddInputDevice((DeviceProc) vidc_kbd, TRUE);
	if ((!mouse) || (!keyboard))
		FatalError("failed to create input devices in InitInput\n");
	private.mouse_dev = (DevicePtr) mouse;
	private.kbd_dev = (DevicePtr) keyboard;

	/* Register devices with the upper 'layers' (tangles) */
	RegisterPointerDevice(mouse);
	RegisterKeyboardDevice(keyboard);
	miRegisterPointerDevice(screenInfo.screens[0], mouse);
	if (!mieqInit((DevicePtr)keyboard, (DevicePtr)mouse))
		FatalError("mieqInit failed!!\n");

	/* Start taking some SIGIOs on input device file descriptors. */
#ifdef HAVE_BUSMOUSE
	if (private.mouse_fd >= 0)
		fcntl(private.mouse_fd, F_SETFL, O_ASYNC | O_NONBLOCK);
#endif /* HAVE_BUSMOUSE */
	if (private.wsmouse_fd >= 0)
		fcntl(private.wsmouse_fd, F_SETFL, O_ASYNC | O_NONBLOCK);
#ifdef HAVE_KBD
       	if (private.kbd_fd >= 0)
		fcntl(private.kbd_fd, F_SETFL, O_ASYNC | O_NONBLOCK);
#endif
       	if (private.wskbd_fd >= 0)
		fcntl(private.wskbd_fd, F_SETFL, O_ASYNC | O_NONBLOCK);
	signal(SIGIO, sigio_handler);
}

/* Screen saver. Yeah, right :-)
 */
static Bool vidc_save_screen(ScreenPtr screen, int on)
{
	DPRINTF(("vidc_save_screen\n"));

	return FALSE;
}

/*
 * 
 */
int vidc_init_screen(int index, ScreenPtr screen, int argc, char **argv)
{
	extern int defaultColorVisualClass;

	private.con_fd = -1;
	private.wsdisplay_fd = -1;

	if (!wsdisplay_init(screen, argc, argv))
#ifdef HAVE_VCONSOLE
	    if (!rpc_init_screen(screen, argc, argv))
#endif
		FatalError("Unabled to initialize frame buffer\n");

	if ((private.vram_base = mmap(0, private.width * private.yres,
		PROT_READ | PROT_WRITE, MAP_FILE | MAP_SHARED, private.vram_fd,
		0)) == MAP_FAILED) {
		FatalError("Unable to mmap frame buffer\n");
		return FALSE;
	}

	DPRINTF(("mmap'ed vram ... the vram_base = %p, size = %d on filehandle %d\n",
		private.vram_base, private.width * private.yres, private.vram_fd));

	/* Set the palette for blackpixel and whitepixel */
/*	write_palette(255, 0, 0, 0);
	write_palette(0, 255, 255, 255);*/
	private.colour_map = 0;

	switch (private.depth) {
	case 1:
		DPRINTF(("mfbScreenInit\n"));
		if (!mfbScreenInit(screen, (pointer) private.vram_base,
		    private.xres, private.yres, SCREEN_DPI_X, SCREEN_DPI_Y,
		    private.xres)) {
			close(private.vram_fd);
			return FALSE;
		}
		DPRINTF(("mfbScreenInit done\n"));
		break;	
	case 8:
		DPRINTF(("cfbScreenInit\n"));
		if (!cfbScreenInit(screen, (pointer) private.vram_base,
		    private.xres, private.yres, SCREEN_DPI_X, SCREEN_DPI_Y,
		    private.xres)) {
			close(private.vram_fd);
			return FALSE;
		}
		DPRINTF(("cfbScreenInit done\n"));
		break;
	case 16:
		DPRINTF(("cfb16ScreenInit\n"));
		defaultColorVisualClass = TrueColor;
		if (!cfb16ScreenInit(screen, (pointer) private.vram_base,
		    private.xres, private.yres, SCREEN_DPI_X, SCREEN_DPI_Y,
		    private.xres)) {
			close(private.vram_fd);
			return FALSE;
		}
		DPRINTF(("cfb16ScreenInit done\n"));
		break;
	default:
		FatalError("%d bpp not supported\n", private.depth);
		break;
	}
	
	screen->InstallColormap = install_colour_map;
	screen->UninstallColormap = uninstall_colour_map;
	screen->ListInstalledColormaps = list_installed_colour_maps;
	screen->StoreColors = store_colours;
	screen->SaveScreen = vidc_save_screen;

	if (!miDCInitialize(screen, &vidc_mouse_funcs)) {
		FatalError("Can't initialise MI pointer device context\n");
		return FALSE;
	}
	switch (private.depth) {
	case 1:
		if (!mfbCreateDefColormap(screen)) {
			FatalError("Can't create default colour map\n");
			return FALSE;
		}
		break;
	case 8:
	case 16:
		if (!cfbCreateDefColormap(screen)) {
			FatalError("Can't create default colour map\n");
			return FALSE;
		}
		break;
	}
	return TRUE;
}

/*
 * Main output init call
 */
void InitOutput(ScreenInfo *info, int argc, char **argv)
{
	DPRINTF(("InitOutput\n"));

	/* Set up the screen information record */
	info->imageByteOrder = IMAGE_BYTE_ORDER;
	info->bitmapScanlineUnit = BITMAP_SCANLINE_UNIT;
	info->bitmapScanlinePad = BITMAP_SCANLINE_PAD;
	info->bitmapBitOrder = BITMAP_BIT_ORDER;
	info->numPixmapFormats = 3;

	/* Set up the pixmap formats that we support (1bpp, 8bpp & 16bpp) */
	info->formats[0].depth = 1;
	info->formats[0].bitsPerPixel = 1;
	info->formats[0].scanlinePad = BITMAP_SCANLINE_PAD;
	info->formats[1].depth = 8;
	info->formats[1].bitsPerPixel = 8;
	info->formats[1].scanlinePad = BITMAP_SCANLINE_PAD;
	info->formats[2].depth = 16;
	info->formats[2].bitsPerPixel = 16;
	info->formats[2].scanlinePad = BITMAP_SCANLINE_PAD;

	AddScreen(vidc_init_screen, argc, argv);
}

/* Stop DDX, closing FDs and returning the keyboard.
 */
void AbortDDX(void)
{
	DPRINTF(("AbortDDX\n"));

	if (private.wsdisplay_fd >= 0)
		wsdisplay_closedown();
#ifdef HAVE_VCONSOLE
	if (private.con_fd >= 0)
		rpc_closedown();
#endif /* HAVE_VCONSOLE */

	if (private.vram_fd != 0)
		close(private.vram_fd);
#ifdef HAVE_BUSMOUSE
	close(private.mouse_fd);
#endif /* HAVE_BUSMOUSE */
	close(private.wsmouse_fd);
#ifdef HAVE_VCONSOLE
	close(private.con_fd);
#endif /* HAVE_VCONSOLE */
	close(private.wsdisplay_fd);
#ifdef HAVE_KBD
	close(private.kbd_fd);
#endif /* HAVE_KBD */
	close(private.wskbd_fd);
}

/* Throw in the towel: Just call AbortDDX for now.
 */
void ddxGiveUp(void)
{
	DPRINTF(("ddxGiveUp\n"));

	AbortDDX();
}

/* Check if a keyboard modifier is legal
 */
Bool LegalModifier(unsigned int key, DevicePtr device)
{
	DPRINTF(("LegalModifier: %x\n", key));
	return TRUE;
}

/* Process all pending input events
 */
void ProcessInputEvents(void)
{
	mieqProcessInputEvents();
	miPointerUpdate();
}

/* Usage message for anything wierd on this server
 */
void ddxUseMsg(void)
{
	ErrorF("\nvidc dependent information:-\n");
	ErrorF("- *** PRE-RELEASE SERVER, USE AT YOUR OWN RISK ***\n");
}

/* Process a command line argument in case we want to support
 * some extra ones. We don't :-)
 */
int ddxProcessArgument(int argc, char **argv, int i)
{
	return 0;
}


#ifdef DDXTIME
/*
 * DDXTIME is defined for the XFree86 servers in ServerOSDefines so
 * we have to implement a local version for the vidc server as os/util.c
 * has this function conditionally on DDXTIME not defined.
 */
CARD32
GetTimeInMillis()
{
    struct timeval  tp;

    gettimeofday(&tp, 0);
    return(tp.tv_sec * 1000) + (tp.tv_usec / 1000);
}
#endif


/* dummy functions to link X server with X input Extension */
void
AddOtherInputDevices ()    
{
}

void
OpenInputDevice (dev, client, status)
    DeviceIntPtr dev;
    ClientPtr client;
    int *status;
{
}
int
SetDeviceValuators (client, dev, valuators, first_valuator, num_valuators)
     register ClientPtr client;
     DeviceIntPtr       dev;
     int                *valuators;
     int                first_valuator;
     int                num_valuators;
{
	return BadMatch;
}

int
SetDeviceMode (client, dev, mode)
     register   ClientPtr       client;
     DeviceIntPtr dev;
     int                mode;
{    
	return BadMatch;
}

int
ChangeKeyboardDevice (old_dev, new_dev) 
     DeviceIntPtr       old_dev;
     DeviceIntPtr       new_dev;
{
  /**********************************************************************
   * DeleteFocusClassDeviceStruct(old_dev);      * defined in xchgptr.c *
   **********************************************************************/
  return !Success;
} 

int ChangePointerDevice (old_dev, new_dev, x, y)
     DeviceIntPtr       old_dev, new_dev;
     unsigned char      x, y;
{     
	return !Success;
}

void CloseInputDevice (d, client)
     DeviceIntPtr d;
     ClientPtr client;
{     
}   

 
int ChangeDeviceControl (client, dev, control)
     register ClientPtr client;
     DeviceIntPtr       dev;
     void         *control;  
{    
	return (BadMatch);
}

/*#ifdef DDXOSFATALERROR*/
void OsVendorFatalError(void)
{
}
/*#endif*/

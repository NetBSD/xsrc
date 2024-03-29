/*
 * Copyright © 2009 Matthieu Herrb <matthieu@herrb.eu>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <dev/wscons/wsconsio.h>


/* #undef DEBUG */
#define DEBUG
#undef DBG
extern int ws_debug_level;
#ifdef DEBUG
# define DBG(lvl, f) { if ((lvl) <= ws_debug_level) f;}
#else
# define DBG(lvl, f)
#endif

#define NAXES 4			/* X and Y, horizontal and vertical scrolling */
#define HSCROLL_AXIS	2
#define VSCROLL_AXIS	3

#define NBUTTONS 32		/* max theoretical buttons */
#define DFLTBUTTONS 3		/* default number of buttons */
#define NUMEVENTS 16		/* max # of ws events to read at once */

typedef struct WSDevice {
	const char *devName;	/* device name */
	int type;		/* ws device type */
	unsigned int buttons;	/* # of buttons */
	unsigned int lastButtons; /* last state of buttons */
	int min_x, max_x, min_y, max_y; /* coord space */
	int swap_axes;
	int raw;
	int inv_x, inv_y;
	int screen_no;
	int num, den, threshold; /* relative accel params */
	pointer buffer;
	int negativeZ, positiveZ; /* mappings for Z axis */
	int negativeW, positiveW; /* mappings for W axis */
	struct wsmouse_calibcoords coords; /* mirror of the kernel values */
	/* Middle mouse button emulation */
	struct {
		BOOL enabled;
		BOOL pending;     /* timer waiting? */
		int buttonstate; /* phys. button state */
		int state;       /* state machine (see emumb.c) */
		Time expires;     /* time of expiry */
		Time timeout;
	} emulateMB;
	ValuatorMask *scroll_mask;
} WSDeviceRec, *WSDevicePtr;

extern int wsmbEmuTimer(InputInfoPtr);
extern BOOL wsmbEmuFilterEvent(InputInfoPtr, int, BOOL);
/* XXX compat-api.h */
#if ABI_VIDEODRV_VERSION < SET_ABI_VERSION(23, 0)
extern void wsmbEmuWakeupHandler(pointer, int, pointer);
extern void wsmbEmuBlockHandler(pointer, struct timeval **, pointer);
#else
extern void wsmbEmuWakeupHandler(pointer, int);
extern void wsmbEmuBlockHandler(pointer, pointer);
#endif
extern void wsmbEmuPreInit(InputInfoPtr);
extern void wsmbEmuOn(InputInfoPtr);
extern void wsmbEmuFinalize(InputInfoPtr);
extern void wsmbEmuEnable(InputInfoPtr, BOOL);
extern void wsmbEmuInitProperty(DeviceIntPtr);

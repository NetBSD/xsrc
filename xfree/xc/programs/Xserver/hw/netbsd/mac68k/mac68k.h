/*-
 * Copyright (C) 1994 Bradley A. Grantham and Lawrence A. Kesteloot
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
 * 3. The names of the Alice Group or any of its members may not be used
 *    to endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE ALICE GROUP ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE ALICE GROUP BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#define NEED_EVENTS

#include "X.h"
#include "misc.h"
#include "Xproto.h"
#include "scrnintstr.h"
#include "screenint.h"
#include "inputstr.h"
#include "input.h"
#include "cursorstr.h"
#include "cursor.h"
#include "pixmapstr.h"
#include "pixmap.h"
#include "windowstr.h"
#include "gc.h"
#include "gcstruct.h"
#include "regionstr.h"
#include "colormap.h"
#include "miscstruct.h"
#include "dix.h"
#include "mfb.h"
#include "mi.h"
#include "mipointer.h"
#include <sys/ioctl.h>
#include <sys/types.h>
#include <machine/param.h>
#include <machine/adbsys.h>
#include <machine/iteioctl.h>
#include <machine/grfioctl.h>

#define TVTOMILLI(tv)	((tv).tv_usec / 1000 + (tv).tv_sec * 1000)


#if !defined(MAXSCREENS)
#define MAXSCREENS	8	/* God forbid anyone should have more... */
#endif


extern int mac_adbfd;


extern Time mac_lasteventtime;

extern DeviceIntPtr mac68k_mouse;
extern DeviceIntPtr mac68k_kbd;


typedef struct fbinfo_s {
	int modenum;
	caddr_t vaddr;
	struct grfmode idata;
	int added;
	int fd;
} fbinfo_t;

typedef struct {
    BYTE	key;
    CARD8	modifiers;
} MacModmapRec;

extern fbinfo_t mac_fbs[MAXSCREENS];
extern int mac_scrs;

extern miPointerScreenFuncRec mac_mousefuncs;


void mac68k_processkbd(DeviceIntPtr, adb_event_t *);
void mac68k_processmouse(DeviceIntPtr, adb_event_t *);
void mac68k_getmouse(void);
void mac68k_getkbd(void);
Bool mac68k_screeninit(ScreenPtr);

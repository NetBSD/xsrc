/* $NetBSD: x68k.h,v 1.3 2016/09/11 03:55:57 tsutsui Exp $ */
/*-------------------------------------------------------------------------
 * Copyright (c) 1996 Yasushi Yamasaki
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
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *      This product includes software developed by Yasushi Yamasaki
 * 4. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *-----------------------------------------------------------------------*/

/* system */
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

/* machine dependent */
#include <machine/vuid_event.h>
#include <machine/kbd.h>
#include <machine/kbio.h>
#include <machine/grfioctl.h>

/* generic X */
#include <X.h>
#include <Xmd.h>
#define XK_KATAKANA
#include <keysym.h>
#define NEED_EVENTS
#include <Xproto.h>

/* dix */
#include <misc.h>
#include <scrnintstr.h>
#include <screenint.h>
#include <input.h>
#include <inputstr.h>
#include <servermd.h>
#include <colormap.h>
#include <colormapst.h>

/* mi */
#include <mipointer.h>

#include "x68kReg.h"

/*
 * X68k dependent screen record
 */
typedef struct _X68kScreenRec {
    int type;                   /* frame buffer type   */
    int class;                  /* visual class        */
    int fd;                     /* file descriptor     */
    int depth;                  /* depth               */
    int fb_width;               /* frame buffer width  */
    int fb_height;              /* frame buffer height */
    int scr_width;              /* screen width        */
    int scr_height;             /* screen height       */
    int dpi;                    /* dots per inch       */
    uint8_t *fb;                /* frame buffer VA     */
    volatile FbReg *reg;        /* control register VA */
    X68kFbReg x68kreg;          /* control register    */
    int mapsize;                /* size of mapped memory */
    ColormapPtr installedMap;   /* installed colormap    */
} X68kScreenRec;

/*
 * frame buffer procedures
 */
typedef struct _X68kFbProcRec {
    Bool (*open)(X68kScreenRec *);		 /* open procedure       */
    Bool (*init)(ScreenPtr, int, char *[]);      /* initialize procedure */
    void (*close)(X68kScreenRec *);		 /* close procedure      */
} X68kFbProcRec;

/* frame buffer types */
#define X68K_FB_NULL    0
#define X68K_FB_TEXT    1       /* text VRAM frame buffer */
#define X68K_FB_GRAPHIC 2       /* graphic VRAM frame buffer */
#if 0
#define X68K_FB_CIRRUS  3       /* not yet */
#endif
#define X68K_FB_TYPES   2

typedef struct _X68kMousePriv {
    int fd;
    int bmask;
} X68kMousePriv, *X68kMousePrivPtr;

typedef struct _X68kKbdPriv {
    int type;
    int fd;
    Leds leds;
} X68kKbdPriv, *X68kKbdPrivPtr;

/* keyboard types */
#define X68K_KB_STANDARD 0      /* standard keyboard */
#define X68K_KB_ASCII    1      /* ascii map keyboard */

#define X68K_MAXEVENTS 32

extern DevPrivateKeyRec x68kScreenPrivateKeyRec;
#define x68kScreenPrivateKey (&x68kScreenPrivateKeyRec)
#define x68kSetScreenPrivate(pScreen, v) \
    dixSetPrivate(&(pScreen)->devPrivates, x68kScreenPrivateKey, (v))
#define x68kGetScreenPrivate(pScreen) ((X68kScreenRec *) \
    dixLookupPrivate(&(pScreen)->devPrivates, x68kScreenPrivateKey))

/* in x68kConfig.c */
X68kScreenRec *x68kGetScreenRec(int);
X68kScreenRec *x68kGetScreenRecByType(int);
X68kFbProcRec *x68kGetFbProcRec(int index);
void x68kRegisterPixmapFormats(ScreenInfo *);
int x68kConfig(void);
extern const char *configFilename;

/* x68kFB.c */
Bool x68kFbCommonOpen(X68kScreenRec *, const char *);
void x68kFbCommonClose(X68kScreenRec *);
Bool x68kSaveScreen(ScreenPtr, int);

/* x68kGraph.c */
Bool x68kGraphOpen(X68kScreenRec *);
Bool x68kGraphInit(ScreenPtr, int, char *[]);
void x68kGraphClose(X68kScreenRec *);

/* in x68kIo.c */
void x68kSigIOHandler(int);

/* in x68kMouse.c */
int x68kMouseProc(DeviceIntPtr, int);
Firm_event *x68kMouseGetEvents(int, int *, Bool *);
void x68kMouseEnqueueEvent(DeviceIntPtr, Firm_event *);
extern miPointerScreenFuncRec x68kPointerScreenFuncs;
extern DeviceIntPtr x68kPointerDevice;

/* in x68kKbd.c */
int x68kKbdProc(DeviceIntPtr, int);
Firm_event *x68kKbdGetEvents(int, int *, Bool *);
void x68kKbdEnqueueEvent(DeviceIntPtr, Firm_event *);
extern X68kKbdPriv x68kKbdPriv;
extern DeviceIntPtr x68kKeyboardDevice;

/* in x68kKeyMap.c */
extern KeySymsRec jisKeySyms, asciiKeySyms, *x68kKeySyms;

/* x68kText.c */
Bool x68kTextOpen(X68kScreenRec *);
Bool x68kTextInit(ScreenPtr, int, char *[]);
void x68kTextClose(X68kScreenRec *);

/* EOF x68k.h */

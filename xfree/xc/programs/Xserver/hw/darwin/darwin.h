/* $XFree86: xc/programs/Xserver/hw/darwin/darwin.h,v 1.1 2000/11/15 01:36:13 dawes Exp $ */

#ifndef _DARWIN_H
#define _DARWIN_H

#include <pthread.h>
#include <IOKit/graphics/IOFramebufferShared.h>
#include "inputstr.h"
#include "screenint.h"
#include "extensions/XKB.h"

typedef struct {
    pthread_t           hidThread;
    io_connect_t        fbService;
    io_connect_t        hidService;
    io_connect_t        hidParam;
    void                *framebuffer;
    int                 width;
    int                 height;
    int                 pitch;
    int                 bitsPerPixel;
    int                 colorBitsPerPixel;
    IOPixelInformation  pixelInfo;
    StdFBShmem_t        *cursorShmem;
} DarwinFramebufferRec;

void DarwinKeyboardInit(DeviceIntPtr pDev);
Bool DarwinInitCursor(ScreenPtr	pScreen);

#define assert(x) { if ((x) == 0) \
    FatalError("assert failed on line %d of %s!\n", __LINE__, __FILE__); }
#define kern_assert(x) { if ((x) != KERN_SUCCESS) \
    FatalError("assert failed on line %d of %s with kernel return 0x%x!\n", \
                __LINE__, __FILE__, x); }

#define MIN_KEYCODE     XkbMinLegalKeyCode     // unfortunately, this isn't 0...

#endif	/* _DARWIN_H */

/* $XFree86: xc/programs/Xserver/hw/darwin/darwin.h,v 1.10 2001/10/14 03:02:18 torrey Exp $ */

#ifndef _DARWIN_H
#define _DARWIN_H

#include <pthread.h>
#include <IOKit/graphics/IOFramebufferShared.h>
#include "inputstr.h"
#include "screenint.h"
#include "extensions/XKB.h"
#include "bundle/quartzShared.h"

typedef struct {
    io_connect_t        fbService;
    void                *framebuffer;
    int                 x;
    int                 y;
    int                 width;
    int                 height;
    int                 pitch;
    int                 bitsPerPixel;
    int                 colorBitsPerPixel;
    IOPixelInformation  pixelInfo;
    StdFBShmem_t        *cursorShmem;
} DarwinFramebufferRec, *DarwinFramebufferPtr;

typedef struct {
    pthread_t           thread;
    io_connect_t        connect;
    io_connect_t        paramConnect;  
} DarwinInputRec;


void xf86SetRootClip (ScreenPtr pScreen, BOOL enable);

// From darwinKeyboard.c
int DarwinModifierNXKeyToNXKeycode(int key, int side);
void DarwinKeyboardInit(DeviceIntPtr pDev);
int DarwinModifierNXKeycodeToNXKey(unsigned char keycode, int *outSide);
int DarwinModifierNXKeyToNXMask(int key);
int DarwinModifierNXMaskToNXKey(int mask);
int DarwinModifierStringToNXKey(const char *string);

#undef assert
#define assert(x) { if ((x) == 0) \
    FatalError("assert failed on line %d of %s!\n", __LINE__, __FILE__); }
#define kern_assert(x) { if ((x) != KERN_SUCCESS) \
    FatalError("assert failed on line %d of %s with kernel return 0x%x!\n", \
                __LINE__, __FILE__, x); }
#define SCREEN_PRIV(pScreen) \
    ((DarwinFramebufferPtr)pScreen->devPrivates[darwinScreenIndex].ptr)


#define MIN_KEYCODE XkbMinLegalKeyCode     // unfortunately, this isn't 0...

/*
 * Global variables from darwin.c
 */
extern int              darwinScreenIndex; // index into pScreen.devPrivates
extern int              darwinScreensFound;
extern DarwinInputRec   hid;
extern int              darwinEventFD;
extern Bool             quartz;

#endif	/* _DARWIN_H */

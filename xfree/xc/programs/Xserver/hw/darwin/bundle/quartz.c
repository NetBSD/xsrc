/**************************************************************
 *
 * Quartz-specific support for the Darwin X Server
 *
 * By Gregory Robert Parker
 *
 **************************************************************/
/* $XFree86: xc/programs/Xserver/hw/darwin/bundle/quartz.c,v 1.9 2001/05/16 06:10:08 torrey Exp $ */

// X headers
#include "scrnintstr.h"

// System headers
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <IOKit/pwr_mgt/IOPMLib.h>

// We need CoreGraphics in ApplicationServices, but we leave out
// QuickDraw, which has symbol conflicts with the basic X includes.
#define __QD__
#define __PRINTCORE__
#include <ApplicationServices/ApplicationServices.h>

#include "../darwin.h"
#include "quartz.h"
#include "quartzAudio.h"
#include "quartzCursor.h"

#define kDarwinMaxScreens 100
static ScreenPtr darwinScreens[kDarwinMaxScreens];
static int darwinNumScreens = 0;
static BOOL xhidden = FALSE;


/*
 * QuartzStoreColors
 *  FIXME: need to implement if Quartz supports PsuedoColor
 */
static void QuartzStoreColors(
    ColormapPtr     pmap,
    int             numEntries,
    xColorItem      *pdefs)
{
}

/*
===========================================================================

 Screen functions

===========================================================================
*/

/*
 * QuartzPMThread
 * Handle power state notifications, FIXME
 */
#if 0
static void *QuartzPMThread(void *arg)
{
    for (;;) {
        mach_msg_return_t       kr;
        mach_msg_empty_rcv_t    msg;

        kr = mach_msg((mach_msg_header_t*) &msg, MACH_RCV_MSG, 0,
                      sizeof(msg), pmNotificationPort, 0, MACH_PORT_NULL);
        kern_assert(kr);

        // computer just woke up
        if (msg.header.msgh_id == 1) {
            if (!xhidden) {
                int i;

                for (i = 0; i < darwinNumScreens; i++) {
                    if (darwinScreens[i]) 
                        xf86SetRootClip(darwinScreens[i], true);
                }
            }
        }
    }
    return NULL;
}
#endif


/* 
 * QuartzAddScreen
 *  Quartz keeps a list of all screens for QuartzShow and QuartzHide.
 *  FIXME: So does ddx, use that instead.
 */
Bool QuartzAddScreen(ScreenPtr pScreen) 
{
    if (darwinNumScreens == kDarwinMaxScreens) {
        return FALSE;
    }

    darwinScreens[darwinNumScreens++] = pScreen;

    // setup cursor support
    if (! QuartzInitCursor(pScreen)) {
        return FALSE;
    }

    // initialize colormap handling as needed
    if (dfb.pixelInfo.pixelType == kIOCLUTPixels) {
        pScreen->StoreColors = QuartzStoreColors;
    }

    return TRUE;
}


/* 
 * QuartzCapture
 *  Capture the screen so we can draw.
 */
void QuartzCapture(void)
{
    if (! CGDisplayIsCaptured(kCGDirectMainDisplay)) {
        CGDisplayCapture(kCGDirectMainDisplay);
    }
}


/* 
 * QuartzRelease
 *  Release the screen so others can draw.
 */
static void QuartzRelease(void)
{
    if (CGDisplayIsCaptured(kCGDirectMainDisplay)) {
        CGDisplayRelease(kCGDirectMainDisplay);
    }
    QuartzMessageMainThread(kQuartzServerHidden);
}


/*
 * QuartzDisplayInit
 *  Init the framebuffer and claim the display from CoreGraphics.
 */
static void QuartzDisplayInit(void) 
{
    dfb.pixelInfo.pixelType = kIORGBDirectPixels;
    dfb.pixelInfo.bitsPerComponent=CGDisplayBitsPerSample(kCGDirectMainDisplay);
    dfb.pixelInfo.componentCount=CGDisplaySamplesPerPixel(kCGDirectMainDisplay);
#if FALSE
    // FIXME: endian and 24 bit color specific
    dfb.pixelInfo.componentMasks[0] = 0x00ff0000;
    dfb.pixelInfo.componentMasks[1] = 0x0000ff00;
    dfb.pixelInfo.componentMasks[2] = 0x000000ff;
#endif

    dfb.width  = CGDisplayPixelsWide(kCGDirectMainDisplay);
    dfb.height = CGDisplayPixelsHigh(kCGDirectMainDisplay);
    dfb.pitch = CGDisplayBytesPerRow(kCGDirectMainDisplay);
    dfb.bitsPerPixel = CGDisplayBitsPerPixel(kCGDirectMainDisplay);
    dfb.colorBitsPerPixel = (dfb.pixelInfo.componentCount * 
                            dfb.pixelInfo.bitsPerComponent);

    dfb.framebuffer = CGDisplayBaseAddress(kCGDirectMainDisplay);

    // need to capture because X doesn't like read-only framebuffer...
    QuartzCapture(); 
    atexit(QuartzRelease);
}


/*
 * QuartzOsVendorInit
 *  Quartz display initialization.
 */
void QuartzOsVendorInit(void)
{
    ErrorF("Display mode: Quartz\n");

    QuartzAudioInit();
    QuartzDisplayInit();
}


/* 
 * QuartzShow
 *  Show the X server on screen. Does nothing if already shown.
 *  Restore the X clip regions the X server cursor state.
 */
void QuartzShow(
    int x,	// cursor location
    int y )
{
    int i;

    if (xhidden) {
        for (i = 0; i < darwinNumScreens; i++) {
            if (darwinScreens[i]) {
                xf86SetRootClip(darwinScreens[i], true);
                QuartzResumeXCursor(darwinScreens[i], x, y);
            }
        }
    }
    xhidden = FALSE;
}


/* 
 * QuartzHide
 *  Remove the X server display from the screen. Does nothing if already hidden.
 *  Release the screen, set X clip regions to prevent drawing, and restore the
 *  Aqua cursor.
 */
void QuartzHide(void)
{
    int i;

    if (!xhidden) {
        for (i = 0; i < darwinNumScreens; i++) {
            if (darwinScreens[i]) {
                QuartzSuspendXCursor(darwinScreens[i]);
                xf86SetRootClip(darwinScreens[i], false);
            }
        }
    } 
    QuartzRelease();
    xhidden = TRUE;
}


/*
 * QuartzGiveUp
 *  Cleanup before X server shutdown
 *  Release the screen and restore the Aqua cursor.
 */
void QuartzGiveUp(void)
{
    int i;

    for (i = 0; i < darwinNumScreens; i++) {
        if (darwinScreens[i]) {
            QuartzSuspendXCursor(darwinScreens[i]);
        }
    }
    QuartzRelease();
}


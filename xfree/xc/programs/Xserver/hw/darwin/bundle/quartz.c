/**************************************************************
 *
 * Quartz-specific support for the Darwin X Server
 *
 * By Gregory Robert Parker
 *
 **************************************************************/
/* $XFree86: xc/programs/Xserver/hw/darwin/bundle/quartz.c,v 1.24 2002/01/17 02:44:26 torrey Exp $ */

#include "quartzCommon.h"
#include "quartz.h"
#include "darwin.h"
#include "quartzAudio.h"
#include "quartzCursor.h"
#include "rootlessAqua.h"
#include "pseudoramiX.h"

// X headers
#include "scrnintstr.h"
#include "colormapst.h"

// System headers
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <IOKit/pwr_mgt/IOPMLib.h>

// Shared global variables for Quartz modes
int                     quartzEventWriteFD = -1;
int                     quartzStartClients = 1;
int                     quartzRootless = -1;
int                     quartzUseSysBeep = 0;
int                     quartzServerVisible = TRUE;
int                     quartzScreenIndex = 0;
int                     aquaMenuBarHeight = 0;
int                     noPseudoramiXExtension = TRUE;
int                     aquaNumScreens = 0;

// Full screen specific per screen storage structure
typedef struct {
    CGDirectDisplayID   displayID;
    CFDictionaryRef     xDisplayMode;
    CFDictionaryRef     aquaDisplayMode;
    CGDirectPaletteRef  xPalette;
    CGDirectPaletteRef  aquaPalette;
} QuartzFSScreenRec, *QuartzFSScreenPtr;

#define FULLSCREEN_PRIV(pScreen) \
    ((QuartzFSScreenPtr)pScreen->devPrivates[quartzFSScreenIndex].ptr)

static int                  quartzFSScreenIndex;
static CGDisplayCount       quartzDisplayCount = 0;
static CGDirectDisplayID   *quartzDisplayList = NULL;


/*
 * QuartzFSStoreColors
 *  This is a callback from X to change the hardware colormap
 *  when using PsuedoColor in full screen mode.
 */
static void QuartzFSStoreColors(
    ColormapPtr         pmap,
    int                 numEntries,
    xColorItem          *pdefs)
{
    ScreenPtr           pScreen = pmap->pScreen;
    QuartzFSScreenPtr   fsDisplayInfo = FULLSCREEN_PRIV(pScreen);
    CGDirectPaletteRef  palette = fsDisplayInfo->xPalette;
    CGDeviceColor       color;
    int                 i;

    if (! palette)
        return;

    for (i = 0; i < numEntries; i++) {
        color.red   = pdefs[i].red   / 65535.0;
        color.green = pdefs[i].green / 65535.0;
        color.blue  = pdefs[i].blue  / 65535.0;
        CGPaletteSetColorAtIndex(palette, color, pdefs[i].pixel);
    }

    if (quartzServerVisible)
        CGDisplaySetPalette(fsDisplayInfo->displayID, palette);
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
            if (quartzServerVisible) {
                int i;

                for (i = 0; i < screenInfo.numScreens; i++) {
                    if (screenInfo.screens[i])
                        xf86SetRootClip(screenInfo.screens[i], true);
                }
            }
        }
    }
    return NULL;
}
#endif


/*
 * QuartzFSFindDisplayMode
 *  Find the appropriate display mode to use in full screen mode.
 *  If display mode is not the same as the current Aqua mode, switch
 *  to the new mode.
 */
static Bool QuartzFSFindDisplayMode(
    QuartzFSScreenPtr fsDisplayInfo)
{
    CGDirectDisplayID cgID = fsDisplayInfo->displayID;
    size_t height, width, bpp;
    boolean_t exactMatch;

    fsDisplayInfo->aquaDisplayMode = CGDisplayCurrentMode(cgID);

    // If no user options, use current display mode
    if (darwinDesiredWidth == 0 && darwinDesiredDepth == -1 &&
        darwinDesiredRefresh == -1)
    {
        fsDisplayInfo->xDisplayMode = fsDisplayInfo->aquaDisplayMode;
        return TRUE;
    }

    // If the user has no choice for size, use current
    if (darwinDesiredWidth == 0) {
        width = CGDisplayPixelsWide(cgID);
        height = CGDisplayPixelsHigh(cgID);
    } else {
        width = darwinDesiredWidth;
        height = darwinDesiredHeight;
    }

    switch (darwinDesiredDepth) {
        case 0:
            bpp = 8;
            break;
        case 1:
            bpp = 16;
            break;
        case 2:
            bpp = 32;
            break;
        default:
            bpp = CGDisplayBitsPerPixel(cgID);
    }

    if (darwinDesiredRefresh == -1) {
        fsDisplayInfo->xDisplayMode =
                CGDisplayBestModeForParameters(cgID, bpp, width, height,
                        &exactMatch);
    } else {
        fsDisplayInfo->xDisplayMode =
                CGDisplayBestModeForParametersAndRefreshRate(cgID, bpp,
                        width, height, darwinDesiredRefresh, &exactMatch);
    }
    if (!exactMatch) {
        fsDisplayInfo->xDisplayMode = fsDisplayInfo->aquaDisplayMode;
        return FALSE;
    }

    // Switch to the new display mode
    CGDisplaySwitchToMode(cgID, fsDisplayInfo->xDisplayMode);
    return TRUE;
}


/*
 * QuartzFSAddScreen
 *  Do initialization of each screen for Quartz in full screen mode.
 */
static Bool QuartzFSAddScreen(
    int index,
    ScreenPtr pScreen)
{
    DarwinFramebufferPtr dfb = SCREEN_PRIV(pScreen);
    QuartzScreenPtr displayInfo = QUARTZ_PRIV(pScreen);
    CGDirectDisplayID cgID = quartzDisplayList[index];
    CGRect bounds;
    QuartzFSScreenPtr fsDisplayInfo;

    // Allocate space for private per screen fullscreen specific storage.
    fsDisplayInfo = xalloc(sizeof(QuartzFSScreenRec));
    FULLSCREEN_PRIV(pScreen) = fsDisplayInfo;

    displayInfo->displayCount = 1;
    displayInfo->displayIDs = xrealloc(displayInfo->displayIDs,
                                      1 * sizeof(CGDirectDisplayID));
    displayInfo->displayIDs[0] = cgID;

    fsDisplayInfo->displayID = cgID;
    fsDisplayInfo->xDisplayMode = 0;
    fsDisplayInfo->aquaDisplayMode = 0;
    fsDisplayInfo->xPalette = 0;
    fsDisplayInfo->aquaPalette = 0;

    // Capture full screen because X doesn't like read-only framebuffer.
    // We need to do this before we (potentially) switch the display mode.
    CGDisplayCapture(cgID);

    if (! QuartzFSFindDisplayMode(fsDisplayInfo)) {
        ErrorF("Could not support specified display mode on screen %i.\n",
               index);
        xfree(fsDisplayInfo);
        return FALSE;
    }

    // Don't need to flip y-coordinate as CoreGraphics treats (0, 0)
    // as the top left of main screen.
    bounds = CGDisplayBounds(cgID);
    dfb->x = bounds.origin.x;
    dfb->y = bounds.origin.y;
    dfb->width  = bounds.size.width;
    dfb->height = bounds.size.height;
    dfb->pitch = CGDisplayBytesPerRow(cgID);
    dfb->bitsPerPixel = CGDisplayBitsPerPixel(cgID);
    dfb->pixelInfo.componentCount = CGDisplaySamplesPerPixel(cgID);

    if (dfb->bitsPerPixel == 8) {
        if (CGDisplayCanSetPalette(cgID)) {
            dfb->pixelInfo.pixelType = kIOCLUTPixels;
        } else {
            dfb->pixelInfo.pixelType = kIOFixedCLUTPixels;
        }
        dfb->pixelInfo.bitsPerComponent = 8;
        dfb->colorBitsPerPixel = 8;
    } else {
        dfb->pixelInfo.pixelType = kIORGBDirectPixels;
        dfb->pixelInfo.bitsPerComponent = CGDisplayBitsPerSample(cgID);
        dfb->colorBitsPerPixel = (dfb->pixelInfo.componentCount *
                                  dfb->pixelInfo.bitsPerComponent);
    }

    dfb->framebuffer = CGDisplayBaseAddress(cgID);

    return TRUE;
}


/*
 * QuartzAddScreen
 *  Do mode dependent initialization of each screen for Quartz.
 */
Bool QuartzAddScreen(
    int index,
    ScreenPtr pScreen)
{
    // allocate space for private per screen Quartz specific storage
    QuartzScreenPtr displayInfo = xcalloc(sizeof(QuartzScreenRec), 1);
    QUARTZ_PRIV(pScreen) = displayInfo;

    // do full screen or rootless specific initialization
    if (quartzRootless) {
        return AquaAddScreen(index, pScreen);
    } else {
        return QuartzFSAddScreen(index, pScreen);
    }
}


/*
 * QuartzFSSetupScreen
 *  Finalize full screen specific setup of each screen.
 */
static Bool QuartzFSSetupScreen(
    int index,
    ScreenPtr pScreen)
{
    DarwinFramebufferPtr dfb = SCREEN_PRIV(pScreen);
    QuartzFSScreenPtr fsDisplayInfo = FULLSCREEN_PRIV(pScreen);
    CGDirectDisplayID cgID = fsDisplayInfo->displayID;

    if (dfb->pixelInfo.pixelType == kIOCLUTPixels) {
        // initialize colormap handling
        size_t aquaBpp;

        CFNumberGetValue(CFDictionaryGetValue(fsDisplayInfo->aquaDisplayMode,
                         kCGDisplayBitsPerPixel), kCFNumberLongType, &aquaBpp);
        if (aquaBpp <= 8)
            fsDisplayInfo->aquaPalette = CGPaletteCreateWithDisplay(cgID);
        fsDisplayInfo->xPalette = CGPaletteCreateDefaultColorPalette();
        pScreen->StoreColors = QuartzFSStoreColors;
    }

    return TRUE;
}


/*
 * QuartzSetupScreen
 *  Finalize mode specific setup of each screen.
 */
Bool QuartzSetupScreen(
    int index,
    ScreenPtr pScreen)
{
    // setup cursor support
    if (! QuartzInitCursor(pScreen))
        return FALSE;

    // do full screen or rootless specific setup
    if (quartzRootless) {
        if (! AquaSetupScreen(index, pScreen))
            return FALSE;
    } else {
        if (! QuartzFSSetupScreen(index, pScreen))
            return FALSE;
    }

    return TRUE;
}


/*
 * QuartzCapture
 *  Capture the screen so we can draw. Called directly from the main thread
 *  to synchronize with hiding the menubar.
 */
void QuartzCapture(void)
{
    int i;

    for (i = 0; i < screenInfo.numScreens; i++) {
        QuartzFSScreenPtr fsDisplayInfo =
                                 FULLSCREEN_PRIV(screenInfo.screens[i]);
        CGDirectDisplayID cgID = fsDisplayInfo->displayID;

        if (!CGDisplayIsCaptured(cgID) && !quartzRootless) {
            CGDisplayCapture(cgID);
            fsDisplayInfo->aquaDisplayMode = CGDisplayCurrentMode(cgID);
            if (fsDisplayInfo->xDisplayMode != fsDisplayInfo->aquaDisplayMode)
                CGDisplaySwitchToMode(cgID, fsDisplayInfo->xDisplayMode);
            if (fsDisplayInfo->xPalette)
                CGDisplaySetPalette(cgID, fsDisplayInfo->xPalette);
        }
    }
}


/*
 * QuartzRelease
 *  Release the screen so others can draw.
 */
void QuartzRelease(void)
{
    int i;

    for (i = 0; i < screenInfo.numScreens; i++) {
        QuartzFSScreenPtr fsDisplayInfo =
                                 FULLSCREEN_PRIV(screenInfo.screens[i]);
        CGDirectDisplayID cgID = fsDisplayInfo->displayID;

        if (CGDisplayIsCaptured(cgID) && !quartzRootless) {
            if (fsDisplayInfo->xDisplayMode != fsDisplayInfo->aquaDisplayMode)
                CGDisplaySwitchToMode(cgID, fsDisplayInfo->aquaDisplayMode);
            if (fsDisplayInfo->aquaPalette)
                CGDisplaySetPalette(cgID, fsDisplayInfo->aquaPalette);
            CGDisplayRelease(cgID);
        }
    }
}


/*
 * QuartzFSDisplayInit
 *  Full screen specific initialization called from InitOutput.
 */
static void QuartzFSDisplayInit(void)
{
    static unsigned long generation = 0;

    // Allocate private storage for each screen's mode specific info
    if (generation != serverGeneration) {
        quartzFSScreenIndex = AllocateScreenPrivateIndex();
        generation = serverGeneration;
    }

    // Find all the CoreGraphics displays
    CGGetActiveDisplayList(0, NULL, &quartzDisplayCount);
    quartzDisplayList = xalloc(quartzDisplayCount * sizeof(CGDirectDisplayID));
    CGGetActiveDisplayList(quartzDisplayCount, quartzDisplayList,
                           &quartzDisplayCount);

    darwinScreensFound = quartzDisplayCount;
    atexit(QuartzRelease);
}


/*
 * QuartzInitOutput
 *  Quartz display initialization.
 */
void QuartzInitOutput(
    int argc,
    char **argv )
{
    static unsigned long generation = 0;

    // Allocate private storage for each screen's Quartz specific info
    if (generation != serverGeneration) {
        quartzScreenIndex = AllocateScreenPrivateIndex();
        generation = serverGeneration;
    }

    if (serverGeneration == 0) {
        QuartzAudioInit();
    }

    if (quartzRootless) {
        ErrorF("Display mode: Rootless Quartz\n");
        AquaDisplayInit();
    } else {
        ErrorF("Display mode: Full screen Quartz\n");
        QuartzFSDisplayInit();
    }

    // Init PseudoramiX implementation of Xinerama.
    // This should be in InitExtensions, but that causes link errors
    // for servers that don't link in pseudoramiX.c.
    if (!noPseudoramiXExtension) {
        PseudoramiXExtensionInit(argc, argv);
    }
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

    if (!quartzServerVisible) {
        quartzServerVisible = TRUE;
        for (i = 0; i < screenInfo.numScreens; i++) {
            if (screenInfo.screens[i]) {
                QuartzResumeXCursor(screenInfo.screens[i], x, y);
                if (!quartzRootless)
                    xf86SetRootClip(screenInfo.screens[i], TRUE);
            }
        }
    }
}


/*
 * QuartzHide
 *  Remove the X server display from the screen. Does nothing if already
 *  hidden. Set X clip regions to prevent drawing, and restore the Aqua
 *  cursor.
 */
void QuartzHide(void)
{
    int i;

    if (quartzServerVisible) {
        for (i = 0; i < screenInfo.numScreens; i++) {
            if (screenInfo.screens[i]) {
                QuartzSuspendXCursor(screenInfo.screens[i]);
                if (!quartzRootless)
                    xf86SetRootClip(screenInfo.screens[i], FALSE);
            }
        }
    }
    quartzServerVisible = FALSE;
    QuartzMessageMainThread(kQuartzServerHidden);
}


/*
 * QuartzGiveUp
 *  Cleanup before X server shutdown
 *  Release the screen and restore the Aqua cursor.
 */
void QuartzGiveUp(void)
{
    int i;

    for (i = 0; i < screenInfo.numScreens; i++) {
        if (screenInfo.screens[i]) {
            QuartzSuspendXCursor(screenInfo.screens[i]);
        }
    }
    QuartzRelease();
}

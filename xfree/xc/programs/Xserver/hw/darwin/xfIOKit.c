/**************************************************************
 *
 * IOKit support for the Darwin X Server
 *
 * HISTORY:
 * Original port to Mac OS X Server by John Carmack
 * Port to Darwin 1.0 by Dave Zarzycki
 * Significantly rewritten for XFree86 4.0.1 by Torrey Lyons
 *
 **************************************************************/
/* $XFree86: xc/programs/Xserver/hw/darwin/xfIOKit.c,v 1.14 2002/01/10 06:59:49 torrey Exp $ */

#define NDEBUG 1

#include "X.h"
#include "Xproto.h"
#include "os.h"
#include "servermd.h"
#include "inputstr.h"
#include "scrnintstr.h"
#include "mi.h"
#include "mibstore.h"
#include "mipointer.h"
#include "micmap.h"

#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>

#include <mach/mach_interface.h>

#define NO_CFPLUGIN
#include <IOKit/IOKitLib.h>
#include <IOKit/hidsystem/IOHIDShared.h>
#include <IOKit/graphics/IOGraphicsLib.h>
#include <drivers/event_status_driver.h>

// Define this to work around bugs in the display drivers for
// older PowerBook G3's. If the X server starts without this
// #define, you don't need it.
#undef OLD_POWERBOOK_G3

#include "darwin.h"
#include "xfIOKit.h"

static EvGlobals *              evg;
static mach_port_t              masterPort;
static mach_port_t              notificationPort;
static IONotificationPortRef    NotificationPortRef;
static mach_port_t              pmNotificationPort;
static io_iterator_t            iter;


/*
 * XFIOKitStoreColors
 * This is a callback from X to change the hardware colormap
 * when using PsuedoColor.
 */
static void XFIOKitStoreColors(
    ColormapPtr     pmap,
    int             numEntries,
    xColorItem      *pdefs)
{
    kern_return_t   kr;
    int             i;
    IOColorEntry    *newColors;
    ScreenPtr       pScreen = pmap->pScreen;
    DarwinFramebufferPtr dfb = SCREEN_PRIV(pScreen);

    assert( newColors = (IOColorEntry *)
                xalloc( numEntries*sizeof(IOColorEntry) ));

    // Convert xColorItem values to IOColorEntry
    // assume the colormap is PsuedoColor
    // as we do not support DirectColor
    for (i = 0; i < numEntries; i++) {
        newColors[i].index = pdefs[i].pixel;
        newColors[i].red =   pdefs[i].red;
        newColors[i].green = pdefs[i].green;
        newColors[i].blue =  pdefs[i].blue;
    }

    kr = IOFBSetCLUT( dfb->fbService, 0, numEntries,
                      kSetCLUTByValue, newColors );
    kern_assert( kr );

    xfree( newColors );
}

/*
 * XFIOKitBell
 *  FIXME
 */
void XFIOKitBell(
    int             loud,
    DeviceIntPtr    pDevice,
    pointer         ctrl,
    int             fbclass)
{
}

/*
 * XFIOKitGiveUp
 *  Closes the connections to IOKit services
 */
void XFIOKitGiveUp( void )
{
    int i;

    // we must close the HID System first
    // because it is a client of the framebuffer
    NXCloseEventStatus( hid.paramConnect );
    IOServiceClose( hid.connect );
    for (i = 0; i < screenInfo.numScreens; i++) {
        DarwinFramebufferPtr dfb = SCREEN_PRIV(screenInfo.screens[i]);
        IOServiceClose( dfb->fbService );
    }
}

/*
 * ClearEvent
 *  Clear an event from the HID System event queue
 */
static void ClearEvent(NXEvent * ep)
{
    static NXEvent nullEvent = {NX_NULLEVENT, {0, 0 }, 0, -1, 0 };

    *ep = nullEvent;
    ep->data.compound.subType = ep->data.compound.misc.L[0] =
                                ep->data.compound.misc.L[1] = 0;
}

/*
 * XFIOKitHIDThread
 *  Read the HID System event queue and pass to pipe
 */
static void *XFIOKitHIDThread(void *arg)
{
    int iokitEventWriteFD = (int)arg;

    for (;;) {
        NXEvent                 ev;
        NXEQElement             *oldHead;
        mach_msg_return_t       kr;
        mach_msg_empty_rcv_t    msg;

        kr = mach_msg((mach_msg_header_t*) &msg, MACH_RCV_MSG, 0,
                      sizeof(msg), notificationPort, 0, MACH_PORT_NULL);
        kern_assert(kr);

        while (evg->LLEHead != evg->LLETail) {
            oldHead = (NXEQElement*)&evg->lleq[evg->LLEHead];
            ev_lock(&oldHead->sema);
            ev = oldHead->event;
            ClearEvent(&oldHead->event);
            evg->LLEHead = oldHead->next;
            ev_unlock(&oldHead->sema);

            write(iokitEventWriteFD, &ev, sizeof(ev));
        }
    }
    return NULL;
}

/*
 * XFIOKitPMThread
 *  Handle power state notifications
 */
static void *XFIOKitPMThread(void *arg)
{
    ScreenPtr pScreen = (ScreenPtr)arg;
    DarwinFramebufferPtr dfb = SCREEN_PRIV(pScreen);

    for (;;) {
        mach_msg_return_t       kr;
        mach_msg_empty_rcv_t    msg;

        kr = mach_msg((mach_msg_header_t*) &msg, MACH_RCV_MSG, 0,
                      sizeof(msg), pmNotificationPort, 0, MACH_PORT_NULL);
        kern_assert(kr);

        // display is powering down
        if (msg.header.msgh_id == 0) {
            IOFBAcknowledgePM( dfb->fbService );
            xf86SetRootClip(pScreen, FALSE);
        }
        // display just woke up
        else if (msg.header.msgh_id == 1) {
            xf86SetRootClip(pScreen, TRUE);
        }
    }
    return NULL;
}

/*
 * SetupFBandHID
 *  Setup an IOFramebuffer service and connect the HID system to it.
 */
static Bool SetupFBandHID(
    int                    index,
    DarwinFramebufferPtr   dfb)
{
    kern_return_t           kr;
    io_service_t            service;
    vm_address_t            vram;
    vm_size_t               shmemSize;
    int                     i;
    UInt32                  numModes;
    IODisplayModeInformation modeInfo;
    IODisplayModeID         displayMode, *allModes;
    IOIndex                 displayDepth;
    IOFramebufferInformation fbInfo;
    StdFBShmem_t            *cshmem;

    // find and open the IOFrameBuffer service
    service = IOIteratorNext(iter);
    if (service == 0)
        return FALSE;

    kr = IOServiceOpen( service, mach_task_self(),
                        kIOFBServerConnectType, &dfb->fbService );
    IOObjectRelease( service );
    if (kr != KERN_SUCCESS) {
        ErrorF("Failed to connect as window server to screen %i.\n", index);
        return FALSE;
    }

    // create the slice of shared memory containing cursor state data
    kr = IOFBCreateSharedCursor( dfb->fbService, kIOFBCurrentShmemVersion,
                                 32, 32 );
    if (kr != KERN_SUCCESS)
        return FALSE;

    // Register for power management events for the framebuffer's device
    kr = IOCreateReceivePort(kOSNotificationMessageID, &pmNotificationPort);
    kern_assert(kr);
    kr = IOConnectSetNotificationPort( dfb->fbService, 0,
                                       pmNotificationPort, 0 );
    if (kr != KERN_SUCCESS) {
        ErrorF("Power management registration failed.\n");
    }

    // SET THE SCREEN PARAMETERS
    // get the current screen resolution, refresh rate and depth
    kr = IOFBGetCurrentDisplayModeAndDepth( dfb->fbService, &displayMode,
                                            &displayDepth );
    if (kr != KERN_SUCCESS)
        return FALSE;

    // use the current screen resolution if the user
    // only wants to change the refresh rate
    if (darwinDesiredRefresh != -1 && darwinDesiredWidth == 0) {
        kr = IOFBGetDisplayModeInformation( dfb->fbService, displayMode,
                                            &modeInfo );
        if (kr != KERN_SUCCESS)
            return FALSE;
        darwinDesiredWidth = modeInfo.nominalWidth;
        darwinDesiredHeight = modeInfo.nominalHeight;
    }

    // use the current resolution and refresh rate
    // if the user doesn't have a preference
    if (darwinDesiredWidth == 0) {

        // change the pixel depth if desired
        if (darwinDesiredDepth != -1) {
            kr = IOFBGetDisplayModeInformation( dfb->fbService, displayMode,
                                                &modeInfo );
            if (kr != KERN_SUCCESS)
                return FALSE;
            if (modeInfo.maxDepthIndex < darwinDesiredDepth) {
                ErrorF("Discarding screen %i:\n", index);
                ErrorF("Current screen resolution does not support desired pixel depth.\n");
                return FALSE;
            }

            displayDepth = darwinDesiredDepth;
            kr = IOFBSetDisplayModeAndDepth( dfb->fbService, displayMode,
                                             displayDepth );
            if (kr != KERN_SUCCESS)
                return FALSE;
        }

    // look for display mode with correct resolution and refresh rate
    } else {

        // get an array of all supported display modes
        kr = IOFBGetDisplayModeCount( dfb->fbService, &numModes );
        if (kr != KERN_SUCCESS)
            return FALSE;
        assert(allModes = (IODisplayModeID *)
                xalloc( numModes * sizeof(IODisplayModeID) ));
        kr = IOFBGetDisplayModes( dfb->fbService, numModes, allModes );
        if (kr != KERN_SUCCESS)
            return FALSE;

        for (i = 0; i < numModes; i++) {
            kr = IOFBGetDisplayModeInformation( dfb->fbService, allModes[i],
                                                &modeInfo );
            if (kr != KERN_SUCCESS)
                return FALSE;

            if (modeInfo.flags & kDisplayModeValidFlag &&
                modeInfo.nominalWidth == darwinDesiredWidth &&
                modeInfo.nominalHeight == darwinDesiredHeight) {

                if (darwinDesiredDepth == -1)
                    darwinDesiredDepth = modeInfo.maxDepthIndex;
                if (modeInfo.maxDepthIndex < darwinDesiredDepth) {
                    ErrorF("Discarding screen %i:\n", index);
                    ErrorF("Desired screen resolution does not support desired pixel depth.\n");
                    return FALSE;
                }

                if ((darwinDesiredRefresh == -1 ||
                    (darwinDesiredRefresh << 16) == modeInfo.refreshRate)) {
                    displayMode = allModes[i];
                    displayDepth = darwinDesiredDepth;
                    kr = IOFBSetDisplayModeAndDepth(dfb->fbService,
                                                    displayMode,
                                                    displayDepth);
                    if (kr != KERN_SUCCESS)
                        return FALSE;
                    break;
                }
            }
        }

        xfree( allModes );
        if (i >= numModes) {
            ErrorF("Discarding screen %i:\n", index);
            ErrorF("Desired screen resolution or refresh rate is not supported.\n");
            return FALSE;
        }
    }

    kr = IOFBGetPixelInformation( dfb->fbService, displayMode, displayDepth,
                                  kIOFBSystemAperture, &dfb->pixelInfo );
    if (kr != KERN_SUCCESS)
        return FALSE;

#ifdef __i386__
    /* x86 in 8bit mode currently needs fixed color map... */
    if( dfb->pixelInfo.bitsPerComponent == 8 ) {
        dfb->pixelInfo.pixelType = kIOFixedCLUTPixels;
    }
#endif

#ifdef OLD_POWERBOOK_G3
    if (dfb->pixelInfo.pixelType == kIOCLUTPixels)
        dfb->pixelInfo.pixelType = kIOFixedCLUTPixels;
#endif

    kr = IOFBGetFramebufferInformationForAperture( dfb->fbService,
                                                   kIOFBSystemAperture,
                                                   &fbInfo );
    if (kr != KERN_SUCCESS)
        return FALSE;

    // FIXME: 1x1 IOFramebuffers are sometimes used to indicate video
    // outputs without a monitor connected to them. Since IOKit Xinerama
    // does not really work, this often causes problems on PowerBooks.
    // For now we explicitly check and ignore these screens.
    if (fbInfo.activeWidth <= 1 || fbInfo.activeHeight <= 1) {
        ErrorF("Discarding screen %i:\n", index);
        ErrorF("Invalid width or height.\n");
        return FALSE;
    }

    kr = IOConnectMapMemory( dfb->fbService, kIOFBCursorMemory,
                             mach_task_self(), (vm_address_t *) &cshmem,
                             &shmemSize, kIOMapAnywhere );
    if (kr != KERN_SUCCESS)
        return FALSE;
    dfb->cursorShmem = cshmem;

    kr = IOConnectMapMemory( dfb->fbService, kIOFBSystemAperture,
                             mach_task_self(), &vram, &shmemSize,
                             kIOMapAnywhere );
    if (kr != KERN_SUCCESS)
        return FALSE;

    dfb->framebuffer = (void*)vram;
    dfb->x = cshmem->screenBounds.minx;
    dfb->y = cshmem->screenBounds.miny;
    dfb->width = fbInfo.activeWidth;
    dfb->height = fbInfo.activeHeight;
    dfb->pitch = fbInfo.bytesPerRow;
    dfb->bitsPerPixel = fbInfo.bitsPerPixel;
    dfb->colorBitsPerPixel = dfb->pixelInfo.componentCount *
                            dfb->pixelInfo.bitsPerComponent;

    // Inform the HID system that the framebuffer is also connected to it.
    kr = IOConnectAddClient( hid.connect, dfb->fbService );
    kern_assert( kr );

    // We have to have added at least one screen
    // before we can enable the cursor.
    kr = IOHIDSetCursorEnable(hid.connect, TRUE);
    kern_assert( kr );

    return TRUE;
}

/*
 * XFIOKitAddScreen
 *  IOKit specific initialization for each screen.
 */
Bool XFIOKitAddScreen(
    int index,
    ScreenPtr pScreen)
{
    DarwinFramebufferPtr dfb = SCREEN_PRIV(pScreen);

    // setup hardware framebuffer
    dfb->fbService = 0;
    if (! SetupFBandHID(index, dfb)) {
        if (dfb->fbService) {
            IOServiceClose(dfb->fbService);
        }
        return FALSE;
    }

    return TRUE;
}

/*
 * XFIOKitSetupScreen
 *  Finalize IOKit specific initialization of each screen.
 */
Bool XFIOKitSetupScreen(
    int index,
    ScreenPtr pScreen)
{
    DarwinFramebufferPtr dfb = SCREEN_PRIV(pScreen);
    pthread_t pmThread;

    // initalize cursor support
    if (! XFIOKitInitCursor(pScreen)) {
        return FALSE;
    }

    // initialize colormap handling as needed
    if (dfb->pixelInfo.pixelType == kIOCLUTPixels) {
        pScreen->StoreColors = XFIOKitStoreColors;
    }

    // initialize power manager handling
    pthread_create( &pmThread, NULL, XFIOKitPMThread,
                    (void *) pScreen );

    return TRUE;
}

/*
 * XFIOKitInitOutput
 *  One-time initialization of IOKit support.
 */
void XFIOKitInitOutput(
    int argc,
    char **argv)
{
    kern_return_t           kr;
    io_service_t            service;
    vm_address_t            shmem;
    vm_size_t               shmemSize;
    int                     fd[2];

    ErrorF("Display mode: IOKit\n");

    kr = IOMasterPort(bootstrap_port, &masterPort);
    kern_assert( kr );

    // find and open the HID System Service
    kr = IOServiceGetMatchingServices( masterPort,
                                       IOServiceMatching( kIOHIDSystemClass ),
                                       &iter );
    kern_assert( kr );

    assert( service = IOIteratorNext( iter ) );

    kr = IOServiceOpen( service, mach_task_self(), kIOHIDServerConnectType,
                        &hid.connect );
    if (kr != KERN_SUCCESS) {
        ErrorF("Failed to connect to the HID System as the window server!\n");
#ifdef DARWIN_WITH_QUARTZ
        FatalError("Quit the Mac OS X window server or use the -quartz option.\n");
#else
        FatalError("Make sure you have quit the Mac OS X window server.\n");
#endif
    }

    IOObjectRelease( service );
    IOObjectRelease( iter );

    kr = IOHIDCreateSharedMemory( hid.connect, kIOHIDCurrentShmemVersion );
    kern_assert( kr );

    kr = IOHIDSetEventsEnable(hid.connect, TRUE);
    kern_assert( kr );

    kr = IOConnectMapMemory( hid.connect, kIOHIDGlobalMemory,
                             mach_task_self(), &shmem, &shmemSize,
                             kIOMapAnywhere );
    kern_assert( kr );

    evg = (EvGlobals *)(shmem + ((EvOffsets *)shmem)->evGlobalsOffset);

    assert(sizeof(EvGlobals) == evg->structSize);

    NotificationPortRef = IONotificationPortCreate( masterPort );

    notificationPort = IONotificationPortGetMachPort(NotificationPortRef);

    kr = IOConnectSetNotificationPort( hid.connect, kIOHIDEventNotification,
                                       notificationPort, 0 );
    kern_assert( kr );

    evg->movedMask |= NX_MOUSEMOVEDMASK;

    // find number of framebuffers
    kr = IOServiceGetMatchingServices( masterPort,
                        IOServiceMatching( IOFRAMEBUFFER_CONFORMSTO ),
                        &iter );
    kern_assert( kr );

    darwinScreensFound = 0;
    while ((service = IOIteratorNext(iter))) {
        IOObjectRelease( service );
        darwinScreensFound++;
    }
    IOIteratorReset(iter);

    assert( pipe(fd) == 0 );
    darwinEventFD = fd[0];
    fcntl(darwinEventFD, F_SETFL, O_NONBLOCK);
    pthread_create(&hid.thread, NULL,
                   XFIOKitHIDThread, (void *) fd[1]);
}

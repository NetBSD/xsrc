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
/* $XFree86: xc/programs/Xserver/hw/darwin/darwin.c,v 1.2 2000/12/01 19:47:38 dawes Exp $ */

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
#include "site.h"
#include "xf86Version.h"

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

DarwinFramebufferRec    dfb;
unsigned char           darwinKeyCommandL = 0, darwinKeyOptionL = 0;

/* Fake button press/release for scroll wheel move. */
#define	SCROLLWHEELUPFAKE	4
#define	SCROLLWHEELDOWNFAKE	5

static	int             darwinEventFD;
static	Bool            fake3Buttons = FALSE;
static	DeviceIntPtr    darwinPointer;
static	DeviceIntPtr    darwinKeyboard;
static	UInt32          darwinDesiredWidth = 0, darwinDesiredHeight = 0;
static	IOIndex         darwinDesiredDepth = -1;
static	SInt32          darwinDesiredRefresh = -1;

// Common pixmap formats
static PixmapFormatRec formats[] = {
        { 1,    1,      BITMAP_SCANLINE_PAD },
        { 4,    8,      BITMAP_SCANLINE_PAD },
        { 8,    8,      BITMAP_SCANLINE_PAD },
        { 15,   16,     BITMAP_SCANLINE_PAD },
        { 16,   16,     BITMAP_SCANLINE_PAD },
        { 24,   32,     BITMAP_SCANLINE_PAD }
};
const int NUMFORMATS = sizeof(formats)/sizeof(formats[0]);

#ifndef OSNAME
#define OSNAME " Darwin"
#endif
#ifndef OSVENDOR
#define OSVENDOR ""
#endif
#ifndef PRE_RELEASE
#define PRE_RELEASE (XF86_VERSION_BETA || XF86_VERSION_ALPHA)
#endif

static void
DarwinPrintBanner()
{
#if PRE_RELEASE
  ErrorF("\n"
    "This is a pre-release version of XFree86, and is not supported in any\n"
    "way.  Bugs may be reported to XFree86@XFree86.Org and patches submitted\n"
    "to fixes@XFree86.Org.  Before reporting bugs in pre-release versions,\n"
    "please check the latest version in the XFree86 CVS repository\n"
    "(http://www.XFree86.Org/cvs)\n");
#endif
  ErrorF("\nXFree86 Version%s", XF86_VERSION);
#ifdef XF86_CUSTOM_VERSION
  ErrorF("(%s) ", XF86_CUSTOM_VERSION);
#endif
  ErrorF("/ X Window System\n");
  ErrorF("(protocol Version %d, revision %d, vendor release %d)\n",
         X_PROTOCOL, X_PROTOCOL_REVISION, VENDOR_RELEASE );
  ErrorF("Release Date: %s\n", XF86_DATE);
  ErrorF("\tIf the server is older than 6-12 months, or if your hardware is\n"
	 "\tnewer than the above date, look for a newer version before\n"
	 "\treporting problems.  (See http://www.XFree86.Org/FAQ)\n");
  ErrorF("Operating System:%s%s\n", OSNAME, OSVENDOR);
#if defined(BUILDERSTRING)
  ErrorF("%s \n",BUILDERSTRING);
#endif
}

static Bool DarwinSaveScreen(ScreenPtr pScreen, int on)
{	// FIXME
	if (on == SCREEN_SAVER_FORCER) {
	} else if (on == SCREEN_SAVER_ON) {
	} else {
	}
	return TRUE;
}

/*
 * DarwinStoreColors
 * This is a callback from X to change the hardware colormap
 * when using PsuedoColor
 */
static void DarwinStoreColors(
    ColormapPtr     pmap,
    int             numEntries,
    xColorItem      *pdefs)
{
    kern_return_t   kr;
    int             i;
    IOColorEntry    *newColors;

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

    kr = IOFBSetCLUT( dfb.fbService, 0, numEntries,
                      kSetCLUTByValue, newColors );
    kern_assert( kr );

    xfree( newColors );
}

/*
 * DarwinAddScreen
 * This is a callback from X during AddScreen() from InitOutput()
 */
static Bool DarwinAddScreen(
    int         index,
    ScreenPtr   pScreen,
    int         argc,
    char        **argv )
{
    int         bitsPerRGB, i;
    VisualPtr   visual;

    /* Communicate the information about our initialized screen back to X. */
    bitsPerRGB = dfb.pixelInfo.bitsPerComponent;

    // reset the visual list
    miClearVisualTypes();

    // setup a single visual appropriate for our pixel type
    // Note: Darwin kIORGBDirectPixels = X window TrueColor, not DirectColor
    if (dfb.pixelInfo.pixelType == kIORGBDirectPixels) {
        if (!miSetVisualTypes( dfb.colorBitsPerPixel, TrueColorMask,
                                bitsPerRGB, TrueColor )) {
            return FALSE;
        }
    } else if (dfb.pixelInfo.pixelType == kIOCLUTPixels) {
        if (!miSetVisualTypes( dfb.colorBitsPerPixel, PseudoColorMask,
                                bitsPerRGB, PseudoColor )) {
            return FALSE;
        }
    } else if (dfb.pixelInfo.pixelType == kIOFixedCLUTPixels) {
        if (!miSetVisualTypes( dfb.colorBitsPerPixel, StaticColorMask,
                                bitsPerRGB, StaticColor )) {
            return FALSE;
        }
    } else {
        return FALSE;
    }

    // machine independent screen init
    // setup _Screen structure in pScreen
    if ( dfb.bitsPerPixel == 32 ) {
        if (!cfb32ScreenInit(pScreen,
                dfb.framebuffer,
                dfb.width, dfb.height,
                    75, 75,		/* screen size in dpi, which we have no accurate knowledge of */
                dfb.pitch / (dfb.bitsPerPixel/8))) {
            return FALSE;
        }
    } else if ( dfb.bitsPerPixel == 16 ) {
        if (!cfb16ScreenInit(pScreen,
                dfb.framebuffer,
                dfb.width, dfb.height,
                    75, 75,		/* screen size in dpi, which we have no accurate knowledge of */
                dfb.pitch / (dfb.bitsPerPixel/8))) {
            return FALSE;
        }
    } else if ( dfb.bitsPerPixel == 8 ) {
        if (!cfbScreenInit(pScreen,
                dfb.framebuffer,
                dfb.width, dfb.height,
                    75, 75,		/* screen size in dpi, which we have no accurate knowledge of */
                dfb.pitch / (dfb.bitsPerPixel/8))) {
            return FALSE;
        }
    } else {
        return FALSE;
    }

    // set the RGB order correctly for TrueColor, it is byte swapped by X
    // FIXME: make work on x86 darwin if it ever gets buildable
    if (dfb.bitsPerPixel > 8) {
        for (i = 0, visual = pScreen->visuals;  // someday we may have more than 1
            i < pScreen->numVisuals; i++, visual++) {
            if (visual->class == TrueColor) {
                visual->offsetRed = bitsPerRGB * 2;
                visual->offsetGreen = bitsPerRGB;
                visual->offsetBlue = 0;
#if FALSE
                visual->redMask = ((1<<bitsPerRGB)-1) << visual->offsetRed;
                visual->greenMask = ((1<<bitsPerRGB)-1) << visual->offsetGreen;
                visual->blueMask = ((1<<bitsPerRGB)-1) << visual->offsetBlue;
#else
                visual->redMask = dfb.pixelInfo.componentMasks[0];
                visual->greenMask = dfb.pixelInfo.componentMasks[1];
                visual->blueMask = dfb.pixelInfo.componentMasks[2];
#endif
            }
        }
    }

#ifdef MITSHM
    ShmRegisterFbFuncs(pScreen);
#endif

    // setup cursor support, use hardware if possible
    if (!DarwinInitCursor(pScreen)) {
        return FALSE;
    }

    // this must be initialized (why doesn't X have a default?)
    pScreen->SaveScreen = DarwinSaveScreen;
    
    // initialize colormap handling as needed
    if (dfb.pixelInfo.pixelType == kIOCLUTPixels) {
        pScreen->StoreColors = DarwinStoreColors;
    }

    // create and install the default colormap and
    // set pScreen->blackPixel / pScreen->white
    if (!miCreateDefColormap( pScreen )) {
        return FALSE;
    }
    
    return TRUE;
}

/*
 * DarwinShutdownScreen
 */
void DarwinShutdownScreen( void )
{
#if 0
    // we must close the HID System first
    // because it is a client of the framebuffer
    NXCloseEventStatus( dfb.hidParam );
    IOServiceClose( dfb.hidService );
    IOServiceClose( dfb.fbService );
#endif
}

/*
 =============================================================================

 mouse callbacks
 
 =============================================================================
*/

/*
 * Set mouse acceleration and thresholding
 * FIXME: We currently ignore the threshold in ctrl->threshold.
 */
static void DarwinChangePointerControl(
    DeviceIntPtr    device,
    PtrCtrl         *ctrl )
{
    kern_return_t   kr;
    double          acceleration;

    acceleration = ctrl->num / ctrl->den;
    kr = IOHIDSetMouseAcceleration( dfb.hidParam, acceleration );
    if (kr != KERN_SUCCESS)
        ErrorF( "Could not set mouse acceleration with kernel return = 0x%x.\n", kr );
}


/*
 * Motion history between events is not required to be supported.
 */
static int DarwinGetMotionEvents( DeviceIntPtr pDevice, xTimecoord *buff,
                           unsigned long start, unsigned long stop, ScreenPtr pScr)
{
	return 0;
}


/*
 * DarwinMouseProc --
 *      Handle the initialization, etc. of a mouse
 */

static int DarwinMouseProc( DeviceIntPtr pPointer, int what ) {

  char map[6];

  switch (what) {

  case DEVICE_INIT:
    pPointer->public.on = FALSE;

    map[1] = 1;
    map[2] = 2;
    map[3] = 3;
    map[4] = 4;
    map[5] = 5;
    InitPointerDeviceStruct( (DevicePtr)pPointer,
                map,
                5,                      // numbuttons (4 & 5 are scroll wheel)
                DarwinGetMotionEvents,  // miPointerGetMotionEvents ??
                DarwinChangePointerControl,
                0 );
    break;

  case DEVICE_ON:
    pPointer->public.on = TRUE;
      AddEnabledDevice( darwinEventFD ); 
    return Success;

  case DEVICE_CLOSE:
  case DEVICE_OFF:
    pPointer->public.on = FALSE;
      RemoveEnabledDevice( darwinEventFD ); 
    return Success;
  }

  return Success;
}

/*
 * DarwinKeybdProc
 * callback from X
 */
static int DarwinKeybdProc( DeviceIntPtr pDev, int onoff )
{
    switch ( onoff ) {
        case DEVICE_INIT:
            DarwinKeyboardInit( pDev );
            break;
        case DEVICE_ON:
            pDev->public.on = TRUE;
            AddEnabledDevice( darwinEventFD );
            break;
        case DEVICE_OFF:
            pDev->public.on = FALSE;
            RemoveEnabledDevice( darwinEventFD );
            break;
        case DEVICE_CLOSE:
            break;
    }

    return Success;
}

/*
===========================================================================

 Functions needed to link against device independent X

===========================================================================
*/

/*
 * ProcessInputEvents
 * Read events from the event queue
 */
void ProcessInputEvents(void)
{
    xEvent	xe;
    NXEvent	ev;
    int	r;
    struct timeval tv;
    struct timezone tz;

    // try to read from our pipe
    r = read( darwinEventFD, &ev, sizeof(ev));
    if ((r == -1) && (errno != EAGAIN)) {
        ErrorF("read(darwinEventFD) failed, errno=%d: %s\n", errno, strerror(errno));
        return;
    } else if ((r == -1) && (errno == EAGAIN)) {
        return;
    } else if ( r != sizeof( ev ) ) {
        ErrorF( "Only read %i bytes from darwinPipe!", r );
        return;
    }

    gettimeofday(&tv, &tz);

    // translate it to an X event and post it
    memset(&xe, 0, sizeof(xe));

    xe.u.keyButtonPointer.rootX = ev.location.x;
    xe.u.keyButtonPointer.rootY = ev.location.y;
    //xe.u.keyButtonPointer.time = ev.time;
    xe.u.keyButtonPointer.time = tv.tv_sec * 1000 + tv.tv_usec / 1000;

    /* A newer kernel generates multi-button events by NX_SYSDEFINED.
       See iokit/Families/IOHIDSystem/IOHIDSystem.cpp version 1.1.1.7,
       2000/08/10 00:23:37 or later. */

    switch( ev.type ) {
        case NX_MOUSEMOVED:
            xe.u.u.type = MotionNotify;
            (darwinPointer->public.processInputProc)( &xe, darwinPointer, 1 );
            break;
 
        case NX_LMOUSEDOWN:
            // Mimic multi-button mouse with Command and Option
            if (fake3Buttons && ev.flags & (NX_COMMANDMASK | NX_ALTERNATEMASK)) {
                if (ev.flags & NX_COMMANDMASK) {
                    // first fool X into forgetting about Command key
                    xe.u.u.type = KeyRelease;
                    xe.u.u.detail = darwinKeyCommandL;
                    (darwinKeyboard->public.processInputProc)
                            ( &xe, darwinKeyboard, 1 );
                    // push button 2
                    xe.u.u.type = ButtonPress;
                    xe.u.u.detail = 2;			// de.key = button 2
                    (darwinPointer->public.processInputProc)
                            ( &xe, darwinPointer, 1 );
                    // reset Command key down
                    xe.u.u.type = KeyPress;
                    xe.u.u.detail = darwinKeyCommandL;
                    (darwinKeyboard->public.processInputProc)
                            ( &xe, darwinKeyboard, 1 );
                } else {
                    // first fool X into forgetting about Option key
                    xe.u.u.type = KeyRelease;
                    xe.u.u.detail = darwinKeyOptionL;
                    (darwinKeyboard->public.processInputProc)
                            ( &xe, darwinKeyboard, 1 );
                    // push button 3
                    xe.u.u.type = ButtonPress;
                    xe.u.u.detail = 3;			// de.key = button 3
                    (darwinPointer->public.processInputProc)
                            ( &xe, darwinPointer, 1 );
                    // reset Option key down
                    xe.u.u.type = KeyPress;
                    xe.u.u.detail = darwinKeyOptionL;
                    (darwinKeyboard->public.processInputProc)
                            ( &xe, darwinKeyboard, 1 );
                }
            } else {
                xe.u.u.detail = 1;			//de.key = button 1;
                xe.u.u.type = ButtonPress;
                (darwinPointer->public.processInputProc)
                        ( &xe, darwinPointer, 1 );
            }
            break;

        case NX_LMOUSEUP:
            // Mimic multi-button mouse with Command and Option
            if (fake3Buttons && ev.flags & (NX_COMMANDMASK | NX_ALTERNATEMASK)) {
                if (ev.flags & NX_COMMANDMASK) {
                    // first fool X into forgetting about Command key
                    xe.u.u.type = KeyRelease;
                    xe.u.u.detail = darwinKeyCommandL;
                    (darwinKeyboard->public.processInputProc)
                            ( &xe, darwinKeyboard, 1 );
                    // push button 2
                    xe.u.u.type = ButtonRelease;
                    xe.u.u.detail = 2;			// de.key = button 2
                    (darwinPointer->public.processInputProc)
                            ( &xe, darwinPointer, 1 );
                    // reset Command key down
                    xe.u.u.type = KeyPress;
                    xe.u.u.detail = darwinKeyCommandL;
                    (darwinKeyboard->public.processInputProc)
                            ( &xe, darwinKeyboard, 1 );
                } else {
                    // first fool X into forgetting about Option key
                    xe.u.u.type = KeyRelease;
                    xe.u.u.detail = darwinKeyOptionL;
                    (darwinKeyboard->public.processInputProc)
                            ( &xe, darwinKeyboard, 1 );
                    // push button 3
                    xe.u.u.type = ButtonRelease;
                    xe.u.u.detail = 3;			// de.key = button 3
                    (darwinPointer->public.processInputProc)
                            ( &xe, darwinPointer, 1 );
                    // reset Option key down
                    xe.u.u.type = KeyPress;
                    xe.u.u.detail = darwinKeyOptionL;
                    (darwinKeyboard->public.processInputProc)
                            ( &xe, darwinKeyboard, 1 );
                }
            } else {
                xe.u.u.detail = 1;			//de.key = button 1;
                xe.u.u.type = ButtonRelease;
                (darwinPointer->public.processInputProc)
                        ( &xe, darwinPointer, 1 );
            }
            break;

// Button 2 isn't handled correctly by older kernels anyway. Just let
// NX_SYSDEFINED events handle these.
        case NX_RMOUSEDOWN:
#if 0
            xe.u.u.detail = 2; //de.key;
            xe.u.u.type = ButtonPress;
            (darwinPointer->public.processInputProc)( &xe, darwinPointer, 1 );
#endif
            break;

        case NX_RMOUSEUP:
#if 0
            xe.u.u.detail = 2; //de.key;
            xe.u.u.type = ButtonRelease;
            (darwinPointer->public.processInputProc)( &xe, darwinPointer, 1 );
#endif
            break;

        case NX_KEYDOWN:
            xe.u.u.type = KeyPress;
            xe.u.u.detail = ev.data.key.keyCode + MIN_KEYCODE;
            (darwinKeyboard->public.processInputProc)( &xe, darwinKeyboard, 1 );
            break;

        case NX_KEYUP:
            xe.u.u.type = KeyRelease;
            xe.u.u.detail = ev.data.key.keyCode + MIN_KEYCODE;
            (darwinKeyboard->public.processInputProc)(&xe, darwinKeyboard, 1);
            break;

        case NX_FLAGSCHANGED:
        {
            static int old_state = 0;
            int new_on_flags = ~old_state & ev.flags;
            int new_off_flags = old_state & ~ev.flags;
            old_state = ev.flags;
            xe.u.u.detail = ev.data.key.keyCode + MIN_KEYCODE;

            // alphalock is toggled rather than held on,
            // so we have to handle it differently
            if (new_on_flags & NX_ALPHASHIFTMASK ||
                new_off_flags & NX_ALPHASHIFTMASK) {
                xe.u.u.type = KeyPress;
                (darwinKeyboard->public.processInputProc)
                        (&xe, darwinKeyboard, 1);
                xe.u.u.type = KeyRelease;
                (darwinKeyboard->public.processInputProc)
                        (&xe, darwinKeyboard, 1);
                break;
            }

            if (new_on_flags) {
                xe.u.u.type = KeyPress;
            } else if (new_off_flags) {
                xe.u.u.type = KeyRelease;
            } else {
                break;
            }
            (darwinKeyboard->public.processInputProc)(&xe, darwinKeyboard, 1);
            break;
        }

        case NX_SYSDEFINED:
            if (ev.data.compound.subType == 7) {
                long hwDelta = ev.data.compound.misc.L[0];
                long hwButtons = ev.data.compound.misc.L[1];
                int i;

                for (i = 1; i < 4; i++) {
                    if (hwDelta & (1 << i)) {
                        xe.u.u.detail = i + 1;
                        if (hwButtons & (1 << i)) {
                            xe.u.u.type = ButtonPress;
                        } else {
                            xe.u.u.type = ButtonRelease;
                        }
                        (darwinPointer->public.processInputProc)
                            ( &xe, darwinPointer, 1 );
                    }
                }
            }
            break;

        case NX_SCROLLWHEELMOVED:
        {
            short count = ev.data.scrollWheel.deltaAxis1;

            if (count > 0) {
                xe.u.u.detail = SCROLLWHEELUPFAKE;
            } else {
                xe.u.u.detail = SCROLLWHEELDOWNFAKE;
                count = -count;
            }

            for (; count; --count) {
                xe.u.u.type = ButtonPress;
                (darwinPointer->public.processInputProc)
                    ( &xe, darwinPointer, 1 );
                xe.u.u.type = ButtonRelease;
                (darwinPointer->public.processInputProc)
                    ( &xe, darwinPointer, 1 );
            }
            break;
        }

        default:
            ErrorF("unknown event caught: %d\n", ev.type);
            ErrorF("\tev.type = %d\n", ev.type);
            ErrorF("\tev.location.x,y = %d,%d\n", ev.location.x, ev.location.y);
            ErrorF("\tev.time = %ld\n", ev.time);
            ErrorF("\tev.flags = 0x%x\n", ev.flags);
            ErrorF("\tev.window = %d\n", ev.window);
            ErrorF("\tev.data.key.origCharSet = %d\n", ev.data.key.origCharSet);
            ErrorF("\tev.data.key.charSet = %d\n", ev.data.key.charSet);
            ErrorF("\tev.data.key.charCode = %d\n", ev.data.key.charCode);
            ErrorF("\tev.data.key.keyCode = %d\n", ev.data.key.keyCode);
            ErrorF("\tev.data.key.origCharCode = %d\n", ev.data.key.origCharCode);
            break;
    }

    // why isn't this handled automatically by X???
    //miPointerAbsoluteCursor( ev.location.x, ev.location.y, ev.time );
    miPointerAbsoluteCursor( ev.location.x, ev.location.y,
                             tv.tv_sec * 1000 + tv.tv_usec / 1000 );
    miPointerUpdate();

}

static void *DarwinHIDThread(void *arg);

/*
 * InitInput
 * Register the keyboard and mouse devices
 */
void InitInput( int  argc, char **argv )
{
	if (serverGeneration == 1) {
		int fd[2];

		assert( pipe(fd) == 0 );
		darwinEventFD = fd[0];
		fcntl(darwinEventFD, F_SETFL, O_NONBLOCK);
		pthread_create(&dfb.hidThread, NULL, DarwinHIDThread, (void *) fd[1]);
    
		darwinPointer = AddInputDevice(DarwinMouseProc, TRUE);
		RegisterPointerDevice( darwinPointer );

		darwinKeyboard = AddInputDevice(DarwinKeybdProc, TRUE);
		RegisterKeyboardDevice( darwinKeyboard );
	}
}

EvGlobals *     evg;
mach_port_t     masterPort;
mach_port_t     notificationPort;
IONotificationPortRef NotificationPortRef;

static void ClearEvent(NXEvent * ep)
{
    static NXEvent nullEvent = {NX_NULLEVENT, {0, 0 }, 0, -1, 0 };

    *ep = nullEvent;
    ep->data.compound.subType = ep->data.compound.misc.L[0] =
                                ep->data.compound.misc.L[1] = 0;
}

static void *DarwinHIDThread(void *arg)
{
    int darwinEventWriteFD = (int)arg;

    for (;;) {
        IOReturn kr;
        NXEvent ev;
        NXEQElement *oldHead;
        struct {
            mach_msg_header_t	header;
            mach_msg_trailer_t	trailer;
        } msg;

        kr = mach_msg((mach_msg_header_t*) &msg, MACH_RCV_MSG, 0,
                      sizeof(msg), notificationPort, 0, MACH_PORT_NULL);
        assert(KERN_SUCCESS == kr);

        while (evg->LLEHead != evg->LLETail) {
            oldHead = (NXEQElement*)&evg->lleq[evg->LLEHead];
            ev_lock(&oldHead->sema);
            ev = oldHead->event;
            ClearEvent(&oldHead->event);
            evg->LLEHead = oldHead->next;
            ev_unlock(&oldHead->sema);

            write(darwinEventWriteFD, &ev, sizeof(ev));
        }
    }
    return NULL;
}

void SetupFBandHID(void)
{
    kern_return_t           kr;
    io_service_t            service;
    io_iterator_t           iter;
    io_name_t               name;
    vm_address_t            shmem, vram;
    vm_size_t               shmemSize;
    int                     i;
    UInt32                  numModes;
    IODisplayModeInformation modeInfo;
    IODisplayModeID         displayMode, *allModes;
    IOIndex                 displayDepth;
    IOFramebufferInformation fbInfo;
    StdFBShmem_t            *cshmem;

    dfb.fbService = 0;
    dfb.hidService = 0;

    // find and open the IOFrameBuffer service
    kr = IOServiceGetMatchingServices( masterPort,
                        IOServiceMatching( IOFRAMEBUFFER_CONFORMSTO ),
                        &iter );
    kern_assert( kr );

    assert(service = IOIteratorNext(iter));

    kr = IOServiceOpen( service, mach_task_self(),
                        kIOFBServerConnectType, &dfb.fbService );
    if (kr != KERN_SUCCESS)
        FatalError("failed to connect as window server!\nMake sure you have quit the Mac OS X window server.\n");

    IOObjectRelease( service );
    IOObjectRelease( iter );

    // create the slice of shared memory containing cursor state data
    kr = IOFBCreateSharedCursor( dfb.fbService, kIOFBCurrentShmemVersion,
                               	 32, 32 );
    kern_assert( kr );

    // SET THE SCREEN PARAMETERS
    // get the current screen resolution, refresh rate and depth
    kr = IOFBGetCurrentDisplayModeAndDepth( dfb.fbService, &displayMode,
                                            &displayDepth );
    kern_assert( kr );

    // use the current screen resolution if the user
    // only wants to change the refresh rate
    if (darwinDesiredRefresh != -1 && darwinDesiredWidth == 0) {
        kr = IOFBGetDisplayModeInformation( dfb.fbService, displayMode,
                                            &modeInfo );
        kern_assert( kr );
        darwinDesiredWidth = modeInfo.nominalWidth;
        darwinDesiredHeight = modeInfo.nominalHeight;
    }

    // use the current resolution and refresh rate
    // if the user doesn't have a preference
    if (darwinDesiredWidth == 0) {

        // change the pixel depth if desired
        if (darwinDesiredDepth != -1) {
            kr = IOFBGetDisplayModeInformation( dfb.fbService, displayMode,
                                                &modeInfo );
            kern_assert( kr );
            if (modeInfo.maxDepthIndex < darwinDesiredDepth)
                FatalError("Current screen resolution does not support desired pixel depth!\n");

            displayDepth = darwinDesiredDepth;
            kr = IOFBSetDisplayModeAndDepth( dfb.fbService, displayMode,
                                             displayDepth );
            kern_assert( kr );
        }
 
    // look for display mode with correct resolution and refresh rate
    } else {

        // get an array of all supported display modes
        kr = IOFBGetDisplayModeCount( dfb.fbService, &numModes );
        kern_assert( kr );
        assert(allModes = (IODisplayModeID *)
                xalloc( numModes * sizeof(IODisplayModeID) ));
        kr = IOFBGetDisplayModes( dfb.fbService, numModes, allModes );
        kern_assert( kr );

        for (i = 0; i < numModes; i++) {
            kr = IOFBGetDisplayModeInformation( dfb.fbService, allModes[i],
                                                &modeInfo );
            kern_assert( kr );

            if (modeInfo.flags & kDisplayModeValidFlag &&
                modeInfo.nominalWidth == darwinDesiredWidth &&
                modeInfo.nominalHeight == darwinDesiredHeight) {

                if (darwinDesiredDepth == -1)
                    darwinDesiredDepth = modeInfo.maxDepthIndex;
                if (modeInfo.maxDepthIndex < darwinDesiredDepth)
                    FatalError("Desired screen resolution does not support desired pixel depth!\n");
                if ((darwinDesiredRefresh == -1 ||
                    (darwinDesiredRefresh << 16) == modeInfo.refreshRate)) {
                    displayMode = allModes[i];
                    displayDepth = darwinDesiredDepth;
                    kr = IOFBSetDisplayModeAndDepth( dfb.fbService, displayMode,
                                                     displayDepth );
                    kern_assert( kr );
                    break;
                }
            }
        }

        xfree( allModes );
        if (i >= numModes)
            FatalError("Desired screen resolution or refresh rate is not supported!\n");
    }

    kr = IOFBGetPixelInformation( dfb.fbService, displayMode, displayDepth,
                                  kIOFBSystemAperture, &dfb.pixelInfo );
    kern_assert( kr );

#ifdef OLD_POWERBOOK_G3
    if (dfb.pixelInfo.pixelType == kIOCLUTPixels)
        dfb.pixelInfo.pixelType = kIOFixedCLUTPixels;
#endif

    kr = IOFBGetFramebufferInformationForAperture( dfb.fbService, kIOFBSystemAperture,
                                                   &fbInfo );
    kern_assert( kr );

    kr = IOConnectMapMemory( dfb.fbService, kIOFBCursorMemory,
                             mach_task_self(), (vm_address_t *) &cshmem,
                             &shmemSize, kIOMapAnywhere );
    kern_assert( kr );
    dfb.cursorShmem = cshmem;

    kr = IOConnectMapMemory( dfb.fbService, kIOFBSystemAperture, mach_task_self(),
                             &vram, &shmemSize, kIOMapAnywhere );
    kern_assert( kr );

    dfb.framebuffer = (void*)vram;
    dfb.width = fbInfo.activeWidth;
    dfb.height = fbInfo.activeHeight;
    dfb.pitch = fbInfo.bytesPerRow;
    dfb.bitsPerPixel = fbInfo.bitsPerPixel;
    dfb.colorBitsPerPixel = dfb.pixelInfo.componentCount *
                            dfb.pixelInfo.bitsPerComponent;

    // find and open the HID System Service
    kr = IOServiceGetMatchingServices( masterPort,
                                       IOServiceMatching( kIOHIDSystemClass ),
                                       &iter );
    kern_assert( kr );

    assert( service = IOIteratorNext( iter ) );

    kr = IORegistryEntryGetName( service, name );
    kern_assert( kr );

    kr = IOServiceOpen( service, mach_task_self(), kIOHIDServerConnectType,
                        &dfb.hidService );
    kern_assert( kr );

    IOObjectRelease( service );
    IOObjectRelease( iter );

    kr = IOHIDCreateSharedMemory( dfb.hidService, kIOHIDCurrentShmemVersion );
    kern_assert( kr );

    kr = IOHIDSetEventsEnable(dfb.hidService, TRUE);
    kern_assert( kr );

    // Inform the HID system that the framebuffer is also connected to it
    kr = IOConnectAddClient( dfb.hidService, dfb.fbService );
    kern_assert( kr );

    kr = IOHIDSetCursorEnable(dfb.hidService, TRUE);
    kern_assert( kr );

    kr = IOConnectMapMemory( dfb.hidService, kIOHIDGlobalMemory, mach_task_self(),
                             &shmem, &shmemSize, kIOMapAnywhere );
    kern_assert( kr );

    evg = (EvGlobals *)(shmem + ((EvOffsets *)shmem)->evGlobalsOffset);

    assert(sizeof(EvGlobals) == evg->structSize);

    NotificationPortRef = IONotificationPortCreate( masterPort );

    notificationPort = IONotificationPortGetMachPort(NotificationPortRef);

    kr = IOConnectSetNotificationPort( dfb.hidService, kIOHIDEventNotification,
                                       notificationPort, 0 );
    kern_assert( kr );

    evg->movedMask |= NX_MOUSEMOVEDMASK;
}


/*
 * InitOutput
 *	Initialize screenInfo for all actually accessible framebuffers.
 */
void InitOutput( ScreenInfo *pScreenInfo, int argc, char **argv )
{
    int i;

    pScreenInfo->imageByteOrder = IMAGE_BYTE_ORDER;
    pScreenInfo->bitmapScanlineUnit = BITMAP_SCANLINE_UNIT;
    pScreenInfo->bitmapScanlinePad = BITMAP_SCANLINE_PAD;
    pScreenInfo->bitmapBitOrder = BITMAP_BIT_ORDER;

    // list how we want common pixmap formats to be padded
    pScreenInfo->numPixmapFormats = NUMFORMATS;
    for (i = 0; i < NUMFORMATS; i++)
        pScreenInfo->formats[i] = formats[i];

    AddScreen( DarwinAddScreen, argc, argv );
}

void OsVendorFatalError( void )
{	ErrorF( "   OsVendorFatalError\n" );
}

/*
 * OSVendorInit
 *  One-time initialization of Darwin support.
 *  Connect to framebuffer and HID system.
 */
void OsVendorInit(void)
{
    kern_return_t           kr;

    kr = IOMasterPort(bootstrap_port, &masterPort);
    kern_assert( kr );

    DarwinPrintBanner();
    SetupFBandHID();
}

/*
 * ddxProcessArgument --
 *	Process device-dependent command line args. Returns 0 if argument is
 *      not device dependent, otherwise Count of number of elements of argv
 *      that are part of a device dependent commandline option.
 */
int ddxProcessArgument( int argc, char *argv[], int i )
{
#if 0
    if ( !strcmp( argv[i], "-screen" ) ) {
    	if ( i == argc-1 ) {
            FatalError( "-screen must be followed by a number" );
        }
    	darwinScreenNumber = atoi( argv[i+1] );
        ErrorF( "Attempting to use screen number %i\n", darwinScreenNumber );
		return 2;
    }
#endif

    if ( !strcmp( argv[i], "-fakebuttons" ) ) {
    	fake3Buttons = TRUE;
        ErrorF( "Faking a three button mouse\n" );
		return 1;
    }

    if ( !strcmp( argv[i], "-nofakebuttons" ) ) {
    	fake3Buttons = FALSE;
        ErrorF( "Not faking a three button mouse\n" );
		return 1;
    }

    if ( !strcmp( argv[i], "-size" ) ) {
    	if ( i >= argc-2 ) {
            FatalError( "-size must be followed by two numbers" );
        }
#ifdef OLD_POWERBOOK_G3
        ErrorF( "Ignoring unsupported -size option on old PowerBook G3\n");
#else
    	darwinDesiredWidth = atoi( argv[i+1] );
        darwinDesiredHeight = atoi( argv[i+2] );
        ErrorF( "Attempting to use width x height = %i x %i\n",
                darwinDesiredWidth, darwinDesiredHeight );
#endif
        return 3;
    }

    if ( !strcmp( argv[i], "-depth" ) ) {
        int     bitDepth;
    	if ( i == argc-1 ) {
            FatalError( "-depth must be followed by a number" );
        }
#ifdef OLD_POWERBOOK_G3
        ErrorF( "Ignoring unsupported -depth option on old PowerBook G3\n");
#else
    	bitDepth = atoi( argv[i+1] );
        if (bitDepth == 8)
            darwinDesiredDepth = 0;
        else if (bitDepth == 15)
            darwinDesiredDepth = 1;
        else if (bitDepth == 24)
            darwinDesiredDepth = 2;
        else
            FatalError( "Unsupported pixel depth. Use 8, 15, or 24 bits" );
        ErrorF( "Attempting to use pixel depth of %i\n", bitDepth );
#endif
        return 2;
    }

    if ( !strcmp( argv[i], "-refresh" ) ) {
    	if ( i == argc-1 ) {
            FatalError( "-refresh must be followed by a number" );
        }
#ifdef OLD_POWERBOOK_G3
        ErrorF( "Ignoring unsupported -refresh option on old PowerBook G3\n");
#else
    	darwinDesiredRefresh = atoi( argv[i+1] );
        ErrorF( "Attempting to use refresh rate of %i\n", darwinDesiredRefresh );
#endif
        return 2;
    }

    if (!strcmp( argv[i], "-showconfig" ) || !strcmp( argv[i], "-version" )) {
        DarwinPrintBanner();
        exit(0);
    }

    return 0;
}

/*
 * ddxUseMsg --
 *	Print out correct use of device dependent commandline options.
 *      Maybe the user now knows what really to do ...
 */
void ddxUseMsg( void )
{
    ErrorF("\n");
    ErrorF("\n");
    ErrorF("Device Dependent Usage:\n");
    ErrorF("\n");
#if 0
    ErrorF("-screen <0,1,...> : use this mac screen num.\n" );
#endif
    ErrorF("-fakebuttons : fake a three button mouse with Command and Option keys.\n");
    ErrorF("-nofakebuttons : don't fake a three button mouse.\n");
    ErrorF("-size <height> <width> : use a screen resolution of <height> x <width>.\n");
    ErrorF("-depth <8,15,24> : use this bit depth.\n");
    ErrorF("-refresh <rate> : use a monitor refresh rate of <rate> Hz.\n");
    ErrorF("-version : show the server version\n");
    ErrorF("\n");
}

/*
 * ddxGiveUp --
 *      Device dependent cleanup. Called by dix before normal server death.
 */
void ddxGiveUp( void ) {
    ErrorF( "   ddxGiveUp\n" ); 
}

/*
 * AbortDDX --
 *      DDX - specific abort routine.  Called by AbortServer(). The attempt is
 *      made to restore all original setting of the displays. Also all devices
 *      are closed.
 */
void AbortDDX( void ) {
#if TRUE
   ErrorF( "   AbortDDX\n" ); 
    /*
     * This is needed for a abnormal server exit, since the normal exit stuff
     * MUST also be performed (i.e. the vt must be left in a defined state)
     */
    ddxGiveUp();
#endif
}

Bool DPMSSupported(void)
{	return 0;
}

void DPMSSet(void)
{	return;
}

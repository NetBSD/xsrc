/**************************************************************
 *
 * Shared code for the Darwin X Server
 * running with Quartz or the IOKit
 *
 **************************************************************/
/* $XFree86: xc/programs/Xserver/hw/darwin/darwin.c,v 1.43 2002/01/09 18:01:58 torrey Exp $ */

#include "X.h"
#include "Xproto.h"
#include "os.h"
#include "servermd.h"
#include "inputstr.h"
#include "scrnintstr.h"
#include "mibstore.h"		// mi backing store implementation
#include "mipointer.h"		// mi software cursor
#include "micmap.h"		// mi colormap code
#include "fb.h"			// fb framebuffer code
#include "site.h"
#include "globals.h"
#include "xf86Version.h"
#include "dix.h"

#include <sys/types.h>
#include <sys/time.h>
#include <sys/syslimits.h>
#include <stdio.h>
#include <fcntl.h>

#define NO_CFPLUGIN
#include <IOKit/IOKitLib.h>
#include <IOKit/hidsystem/IOHIDLib.h>
#include <IOKit/hidsystem/ev_keymap.h>

#include "darwin.h"
#include "darwinClut8.h"
#include "bundle/quartz.h"
#include "xfIOKit.h"

// Quit after this many seconds if no quartz event poster is found.
// Leave undefined for no safety quit.
#undef QUARTZ_SAFETY_DELAY

/* Fake button press/release for scroll wheel move. */
#define SCROLLWHEELUPFAKE	4
#define SCROLLWHEELDOWNFAKE	5

/*
 * X server shared global variables
 */
int                     darwinScreensFound = 0;
int                     darwinScreenIndex = 0;
DarwinInputRec          hid;
int                     darwinEventFD = -1;
Bool                    quartz = FALSE;
int                     quartzMouseAccelChange = 1;
int                     darwinFakeButtons = 0;

// location of X11's (0,0) point in global screen coordinates
int                     darwinMainScreenX = 0;
int                     darwinMainScreenY = 0;

// parameters read from the command line or user preferences
unsigned int            darwinDesiredWidth = 0, darwinDesiredHeight = 0;
int                     darwinDesiredDepth = -1;
int                     darwinDesiredRefresh = -1;
char                    *darwinKeymapFile = "USA.keymapping";

// modifier masks for faking mouse buttons
int                     darwinFakeMouse2Mask = NX_COMMANDMASK;
int                     darwinFakeMouse3Mask = NX_ALTERNATEMASK;

static DeviceIntPtr     darwinPointer;
static DeviceIntPtr     darwinKeyboard;

// Common pixmap formats
static PixmapFormatRec formats[] = {
        { 1,    1,      BITMAP_SCANLINE_PAD },
        { 4,    8,      BITMAP_SCANLINE_PAD },
        { 8,    8,      BITMAP_SCANLINE_PAD },
        { 15,   16,     BITMAP_SCANLINE_PAD },
        { 16,   16,     BITMAP_SCANLINE_PAD },
        { 24,   32,     BITMAP_SCANLINE_PAD },
        { 32,   32,     BITMAP_SCANLINE_PAD }
};
const int NUMFORMATS = sizeof(formats)/sizeof(formats[0]);

#ifndef OSNAME
#define OSNAME " Darwin"
#endif
#ifndef OSVENDOR
#define OSVENDOR ""
#endif
#ifndef PRE_RELEASE
#define PRE_RELEASE XF86_VERSION_SNAP
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
  ErrorF("\nXFree86 Version %d.%d.%d", XF86_VERSION_MAJOR, XF86_VERSION_MINOR,
                                    XF86_VERSION_PATCH);
#if XF86_VERSION_SNAP > 0
  ErrorF(".%d", XF86_VERSION_SNAP);
#endif

#if XF86_VERSION_SNAP >= 900
  ErrorF(" (%d.%d.0 RC %d)", XF86_VERSION_MAJOR, XF86_VERSION_MINOR + 1,
				XF86_VERSION_SNAP - 900);
#endif

#ifdef XF86_CUSTOM_VERSION
  ErrorF(" (%s)", XF86_CUSTOM_VERSION);
#endif
  ErrorF(" / X Window System\n");
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


/*
 * DarwinSaveScreen
 *  X screensaver support. Not implemented.
 */
static Bool DarwinSaveScreen(ScreenPtr pScreen, int on)
{
    // FIXME
    if (on == SCREEN_SAVER_FORCER) {
    } else if (on == SCREEN_SAVER_ON) {
    } else {
    }
    return TRUE;
}

/*
 * DarwinAddScreen
 *  This is a callback from dix during AddScreen() from InitOutput().
 *  Initialize the screen and communicate information about it back to dix.
 */
static Bool DarwinAddScreen(
    int         index,
    ScreenPtr   pScreen,
    int         argc,
    char        **argv )
{
    int         bitsPerRGB, i, dpi;
    static int  foundIndex = 0;
    Bool        ret;
    VisualPtr   visual;
    ColormapPtr pmap;
    DarwinFramebufferPtr dfb;

    // reset index of found screens for each server generation
    if (index == 0) foundIndex = 0;

    // allocate space for private per screen storage
    dfb = xalloc(sizeof(DarwinFramebufferRec));
    SCREEN_PRIV(pScreen) = dfb;

    // setup hardware/mode specific details
    if (quartz) {
        ret = QuartzAddScreen(foundIndex, pScreen);
    } else {
        ret = XFIOKitAddScreen(foundIndex, pScreen);
    }
    foundIndex++;
    if (! ret)
        return FALSE;

    bitsPerRGB = dfb->pixelInfo.bitsPerComponent;

    // reset the visual list
    miClearVisualTypes();

    // setup a single visual appropriate for our pixel type
    // Note: Darwin kIORGBDirectPixels = X TrueColor, not DirectColor
    if (dfb->pixelInfo.pixelType == kIORGBDirectPixels) {
        if (!miSetVisualTypes( dfb->colorBitsPerPixel, TrueColorMask,
                                bitsPerRGB, TrueColor )) {
            return FALSE;
        }
    } else if (dfb->pixelInfo.pixelType == kIOCLUTPixels) {
        if (!miSetVisualTypes( dfb->colorBitsPerPixel, PseudoColorMask,
                                bitsPerRGB, PseudoColor )) {
            return FALSE;
        }
    } else if (dfb->pixelInfo.pixelType == kIOFixedCLUTPixels) {
        if (!miSetVisualTypes( dfb->colorBitsPerPixel, StaticColorMask,
                                bitsPerRGB, StaticColor )) {
            return FALSE;
        }
    } else {
        return FALSE;
    }

    miSetPixmapDepths();

    // machine independent screen init
    // setup _Screen structure in pScreen
    if (monitorResolution)
        dpi = monitorResolution;
    else
        dpi = 75;

    // initialize fb
    if (! fbScreenInit(pScreen,
                dfb->framebuffer,                 // pointer to screen bitmap
                dfb->width, dfb->height,          // screen size in pixels
                dpi, dpi,                         // dots per inch
                dfb->pitch/(dfb->bitsPerPixel/8), // pixel width of framebuffer
                dfb->bitsPerPixel))               // bits per pixel for screen
    {
        return FALSE;
    }

    // set the RGB order correctly for TrueColor
    if (dfb->bitsPerPixel > 8) {
        for (i = 0, visual = pScreen->visuals;  // someday we may have more than 1
            i < pScreen->numVisuals; i++, visual++) {
            if (visual->class == TrueColor) {
                visual->offsetRed = bitsPerRGB * 2;
                visual->offsetGreen = bitsPerRGB;
                visual->offsetBlue = 0;
#if TRUE
                visual->redMask = ((1<<bitsPerRGB)-1) << visual->offsetRed;
                visual->greenMask = ((1<<bitsPerRGB)-1) << visual->offsetGreen;
                visual->blueMask = ((1<<bitsPerRGB)-1) << visual->offsetBlue;
#else
                visual->redMask = dfb->pixelInfo.componentMasks[0];
                visual->greenMask = dfb->pixelInfo.componentMasks[1];
                visual->blueMask = dfb->pixelInfo.componentMasks[2];
#endif
            }
        }
    }

#ifdef RENDER
    if (! fbPictureInit(pScreen, 0, 0)) {
        return FALSE;
    }
#endif

#ifdef MITSHM
    ShmRegisterFbFuncs(pScreen);
#endif

    // this must be initialized (why doesn't X have a default?)
    pScreen->SaveScreen = DarwinSaveScreen;

    // finish mode dependent screen setup including cursor support
    if (quartz) {
        if (! QuartzSetupScreen(index, pScreen)) {
            return FALSE;
        }
    } else {
        if (! XFIOKitSetupScreen(index, pScreen)) {
            return FALSE;
        }
    }

    // create and install the default colormap and
    // set pScreen->blackPixel / pScreen->white
    if (!miCreateDefColormap( pScreen )) {
        return FALSE;
    }

    /* Set the colormap to the statically defined one if we're in 8 bit
     * mode and we're using a fixed color map.  Essentially this translates
     * to Darwin/x86 in 8-bit mode.
     */
    if( (dfb->colorBitsPerPixel == 8) &&
                (dfb->pixelInfo.pixelType == kIOFixedCLUTPixels) )
    {
        pmap = miInstalledMaps[pScreen->myNum];
        visual = pmap->pVisual;
        for( i = 0; i < visual->ColormapEntries; i++ ) {
            pmap->red[i].co.local.red   = darwinClut8[i].red;
            pmap->red[i].co.local.green = darwinClut8[i].green;
            pmap->red[i].co.local.blue  = darwinClut8[i].blue;
        }
    }

    dixScreenOrigins[index].x = dfb->x;
    dixScreenOrigins[index].y = dfb->y;

    ErrorF("Screen %d added: %dx%d @ (%d,%d)\n",
            index, dfb->width, dfb->height, dfb->x, dfb->y);

    return TRUE;
}

/*
 =============================================================================

 mouse and keyboard callbacks

 =============================================================================
*/

/*
 * DarwinChangePointerControl
 *  Set mouse acceleration and thresholding
 *  FIXME: We currently ignore the threshold in ctrl->threshold.
 */
static void DarwinChangePointerControl(
    DeviceIntPtr    device,
    PtrCtrl         *ctrl )
{
    kern_return_t   kr;
    double          acceleration;

    if (!quartzMouseAccelChange)
        return;

    acceleration = ctrl->num / ctrl->den;
    kr = IOHIDSetMouseAcceleration( hid.paramConnect, acceleration );
    if (kr != KERN_SUCCESS)
        ErrorF( "Could not set mouse acceleration with kernel return = 0x%x.\n", kr );
}


/*
 * DarwinMouseProc
 *  Handle the initialization, etc. of a mouse
 */

static int DarwinMouseProc(
    DeviceIntPtr    pPointer,
    int             what )
{
    char map[6];

    switch (what) {

        case DEVICE_INIT:
            pPointer->public.on = FALSE;

            // Set button map.
            map[1] = 1;
            map[2] = 2;
            map[3] = 3;
            map[4] = 4;
            map[5] = 5;
            InitPointerDeviceStruct( (DevicePtr)pPointer,
                        map,
                        5,   // numbuttons (4 & 5 are scroll wheel)
                        miPointerGetMotionEvents,
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
 *  Callback from X
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

 Utility routines

===========================================================================
*/

/*
 * DarwinFindLibraryFile
 *  Search for a file in the standard Library paths, which are (in order):
 *
 *      ~/Library/              user specific
 *      /Library/               host specific
 *      /Network/Library/       LAN specific
 *      /System/Library/        OS specific
 *
 *  A sub-path can be specified to search in below the various Library
 *  directories. Returns a new character string (owned by the caller)
 *  containing the full path to the first file found.
 */
static char * DarwinFindLibraryFile(
    const char *file,
    const char *pathext )
{
    // Library search paths
    char *pathList[] = {
        "",
        "/Network",
        "/System",
        NULL
    };
    char *home;
    char *fullPath;
    int i = 0;

    // Return the file name as is if it is already a fully qualified path.
    if (!access(file, F_OK)) {
        fullPath = xalloc(strlen(file)+1);
        strcpy(fullPath, file);
        return fullPath;
    }

    fullPath = xalloc(PATH_MAX);

    home = getenv("HOME");
    if (home) {
        snprintf(fullPath, PATH_MAX, "%s/Library/%s/%s", home, pathext, file);
        if (!access(fullPath, F_OK))
            return fullPath;
    }

    while (pathList[i]) {
        snprintf(fullPath, PATH_MAX, "%s/Library/%s/%s", pathList[i++],
                 pathext, file);
        if (!access(fullPath, F_OK))
            return fullPath;
    }

    xfree(fullPath);
    return NULL;
}

/*
 * DarwinPressKeycode
 *  Press or release the given key, specified by NX keycode.
 *  xe must already have event time and mouse location filled in.
 *  pressed is KeyPress or KeyRelease.
 *  keycode is NX keycode without MIN_KEYCODE adjustment.
 */
#define DarwinPressKeycode(xe, pressed, keycode) \
    xe.u.u.type = (pressed); \
    xe.u.u.detail = (keycode) + MIN_KEYCODE; \
    (darwinKeyboard->public.processInputProc)(&(xe), darwinKeyboard, 1);


/*
 * DarwinPressModifierMask
 *  Press or release the given modifier key, specified by its mask.
 */
static void DarwinPressModifierMask(
    xEvent xe,      // must already have time and mouse location filled in
    BYTE pressed,   // KeyPress or KeyRelease
    int mask)       // one of NX_*MASK constants
{
    int key = DarwinModifierNXMaskToNXKey(mask);

    if (key != -1) {
        int keycode = DarwinModifierNXKeyToNXKeycode(key, 0);
        if (keycode != 0) {
            DarwinPressKeycode(xe, pressed, keycode);
        }
    }
}

/*
 * DarwinUpdateModifiers
 *  Send events to update the modifier state.
 */
static void DarwinUpdateModifiers(
    xEvent xe,          // event template with time and mouse position set
    int pressed,        // KeyPress or KeyRelease
    int flags )         // modifier flags that have changed
{
    if (flags & NX_ALPHASHIFTMASK) {
        DarwinPressModifierMask(xe, pressed, NX_ALPHASHIFTMASK);
    }
    if (flags & NX_COMMANDMASK) {
        DarwinPressModifierMask(xe, pressed, NX_COMMANDMASK);
    }
    if (flags & NX_CONTROLMASK) {
        DarwinPressModifierMask(xe, pressed, NX_CONTROLMASK);
    }
    if (flags & NX_ALTERNATEMASK) {
        DarwinPressModifierMask(xe, pressed, NX_ALTERNATEMASK);
    }
    if (flags & NX_SHIFTMASK) {
        DarwinPressModifierMask(xe, pressed, NX_SHIFTMASK);
    }
    if (flags & NX_SECONDARYFNMASK) {
        DarwinPressModifierMask(xe, pressed, NX_SECONDARYFNMASK);
    }
}

/*
 * DarwinParseModifierList
 *  Parse a list of modifier names and return a corresponding modifier mask
 */
static int DarwinParseModifierList(
    const char *constmodifiers) // string containing list of modifier names
{
    int result = 0;

    if (constmodifiers) {
        char *modifiers = strdup(constmodifiers);
        char *modifier;
        int nxkey;
        char *p = modifiers;

        while (p) {
            modifier = strsep(&p, " ,+&|/"); // allow lots of separators
            nxkey = DarwinModifierStringToNXKey(modifier);
            if (nxkey != -1)
                result |= DarwinModifierNXKeyToNXMask(nxkey);
            else
                ErrorF("fakebuttons: Unknown modifier \"%s\"\n", modifier);
        }
        free(modifiers);
    }
    return result;
}

/*
 * DarwinSimulateMouseClick
 *  Send a mouse click to X when multiple mouse buttons are simulated
 *  with modifier-clicks, such as command-click for button 2. The dix
 *  layer is told that the previously pressed modifier key(s) are
 *  released, the simulated click event is sent. After the mouse button
 *  is released, the modifier keys are reverted to their actual state,
 *  which may or may not be pressed at that point. This is usually
 *  closest to what the user wants. Ie. the user typically wants to
 *  simulate a button 2 press instead of Command-button 2.
 */
static void DarwinSimulateMouseClick(
    xEvent xe,          // event template with time and
                        // mouse position filled in
    int whichButton,    // mouse button to be pressed
    int whichEvent,     // ButtonPress or ButtonRelease
    int modifierMask)   // modifiers used for the fake click
{
    // first fool X into forgetting about the keys
    DarwinUpdateModifiers(xe, KeyRelease, modifierMask);

    // push the mouse button
    xe.u.u.type = whichEvent;
    xe.u.u.detail = whichButton;
    (darwinPointer->public.processInputProc)
        ( &xe, darwinPointer, 1 );
}


/*
===========================================================================

 Functions needed to link against device independent X

===========================================================================
*/

/*
 * ProcessInputEvents
 *  Read and process events from the event pipe until it is empty.
 */
void ProcessInputEvents(void)
{
    xEvent xe;
    NXEvent ev;
    int r;
    struct timeval tv;
    struct timezone tz;

    // last known modifier state
    static int old_state = 0;
    // button number and modifier mask of currently pressed fake button
    static int darwinFakeMouseButtonDown = 0;
    static int darwinFakeMouseButtonMask = 0;

#if defined(DARWIN_WITH_QUARTZ) && defined(QUARTZ_SAFETY_DELAY)
    static Bool gotread = false;
    static int startsec = 0;

    // Quartz safety quit. Bail if we don't get any events from the event pipe.
    // If the event writer fails to find us, we will have captured the screen
    // but not be seeing any events and be unkillable from the console.
    if (quartz  &&  ! gotread) {
        gettimeofday(&tv, &tz);
        if (startsec == 0) startsec = tv.tv_sec;
        if (startsec + QUARTZ_SAFETY_DELAY< tv.tv_sec) {
            QuartzGiveUp();
            FatalError("%d second safety quit\n", QUARTZ_SAFETY_DELAY);
        }
    }
#endif

    while (TRUE) {

        // try to read from our pipe
        r = read( darwinEventFD, &ev, sizeof(ev));

        if ((r == -1) && (errno == EAGAIN)) {
            // no event available
            break;
        } else if (r == -1) {
            // FIXME: FatalError here? Reading may be permanently broken.
            ErrorF( "Event pipe read failed, errno=%d: %s\n",
                    errno, strerror(errno) );
            break;
        } else if ( r == 0 ) {
            FatalError("Darwin ProcessInputEvents: EOF on event pipe!\n");
        } else if ( r != sizeof( ev ) ) {
            ErrorF( "Only read %i bytes from event pipe!\n", r );
            break;
        }

#if defined(DARWIN_WITH_QUARTZ) && defined(QUARTZ_SAFETY_DELAY)
        gotread = true;
#endif
        gettimeofday(&tv, &tz);

        // translate it to an X event and post it
        memset(&xe, 0, sizeof(xe));

        // Shift from global screen coordinates to coordinates relative to
        // the origin of the current screen.
        xe.u.keyButtonPointer.rootX = ev.location.x - darwinMainScreenX -
                dixScreenOrigins[miPointerCurrentScreen()->myNum].x;
        xe.u.keyButtonPointer.rootY = ev.location.y - darwinMainScreenY -
                dixScreenOrigins[miPointerCurrentScreen()->myNum].y;
        //xe.u.keyButtonPointer.time = ev.time;
        xe.u.keyButtonPointer.time = tv.tv_sec * 1000 + tv.tv_usec / 1000;

        /* A newer kernel generates multi-button events by NX_SYSDEFINED.
        See iokit/Families/IOHIDSystem/IOHIDSystem.cpp version 1.1.1.7,
        2000/08/10 00:23:37 or later. */

        switch( ev.type ) {
            case NX_MOUSEMOVED:
                xe.u.u.type = MotionNotify;
                (darwinPointer->public.processInputProc)
                    ( &xe, darwinPointer, 1 );
                break;

#ifdef __i386__
            // x86 drivers currently reverse mouse up and down
            case NX_LMOUSEUP:
#else
            case NX_LMOUSEDOWN:
#endif
                // Mimic multi-button mouse with modifier-clicks
                // If both sets of modifiers are pressed, button 2 is clicked.
                if (darwinFakeButtons  &&
                    (ev.flags & darwinFakeMouse2Mask) == darwinFakeMouse2Mask)
                {
                    DarwinSimulateMouseClick(xe, 2, ButtonPress,
                                             darwinFakeMouse2Mask);
                    darwinFakeMouseButtonDown = 2;
                    darwinFakeMouseButtonMask = darwinFakeMouse2Mask;
                }
                else if (darwinFakeButtons  &&
                    (ev.flags & darwinFakeMouse3Mask) == darwinFakeMouse3Mask)
                {
                    DarwinSimulateMouseClick(xe, 3, ButtonPress,
                                             darwinFakeMouse3Mask);
                    darwinFakeMouseButtonDown = 3;
                    darwinFakeMouseButtonMask = darwinFakeMouse3Mask;
                }
                else {
                    xe.u.u.detail = 1;
                    xe.u.u.type = ButtonPress;
                    (darwinPointer->public.processInputProc)
                            ( &xe, darwinPointer, 1 );
                }
                break;

#ifdef __i386__
            case NX_LMOUSEDOWN:
#else
            case NX_LMOUSEUP:
#endif
                // If last mousedown was a fake click, don't check for
                // mouse modifiers here. The user may have released the
                // modifiers before the mouse button.
                xe.u.u.type = ButtonRelease;
                if (darwinFakeMouseButtonDown) {
                    xe.u.u.detail = darwinFakeMouseButtonDown;
                    darwinFakeMouseButtonDown = 0;
                    (darwinPointer->public.processInputProc)
                            ( &xe, darwinPointer, 1 );

                    // Bring modifiers back up to date
                    DarwinUpdateModifiers(xe, KeyPress,
                            darwinFakeMouseButtonMask & old_state);
                    darwinFakeMouseButtonMask = 0;
                } else {
                    xe.u.u.detail = 1;
                    (darwinPointer->public.processInputProc)
                            ( &xe, darwinPointer, 1 );
                }
                break;

            // Button 2 isn't handled correctly by older kernels anyway.
            // Just let NX_SYSDEFINED events handle these.
            case NX_RMOUSEDOWN:
#if 0
                xe.u.u.detail = 2;
                xe.u.u.type = ButtonPress;
                (darwinPointer->public.processInputProc)
                        ( &xe, darwinPointer, 1 );
#endif
                break;

            case NX_RMOUSEUP:
#if 0
                xe.u.u.detail = 2;
                xe.u.u.type = ButtonRelease;
                (darwinPointer->public.processInputProc)
                        ( &xe, darwinPointer, 1 );
#endif
                break;

            case NX_KEYDOWN:
                DarwinPressKeycode(xe, KeyPress, ev.data.key.keyCode);
                break;

            case NX_KEYUP:
                DarwinPressKeycode(xe, KeyRelease, ev.data.key.keyCode);
                break;

            case NX_FLAGSCHANGED:
            {
                // Assumes only one flag has changed. In Quartz mode, this
                // restriction must be enforced by the Quartz event feeder.
                int new_on_flags = ~old_state & ev.flags;
                int new_off_flags = old_state & ~ev.flags;
                old_state = ev.flags;

                // Alphalock is toggled rather than held on,
                // so we have to press and release it every time.
                if (new_on_flags  ||  new_off_flags & NX_ALPHASHIFTMASK) {
                    DarwinPressKeycode(xe, KeyPress, ev.data.key.keyCode);
                }
                if (new_off_flags ||  new_on_flags & NX_ALPHASHIFTMASK) {
                    DarwinPressKeycode(xe, KeyRelease, ev.data.key.keyCode);
                }
                break;
            }

            case NX_SYSDEFINED:
                if (ev.data.compound.subType == 7) {
                    long hwDelta = ev.data.compound.misc.L[0];
                    long hwButtons = ev.data.compound.misc.L[1];
                    int i;

                    for (i = 1; i < 5; i++) {
                        if (hwDelta & (1 << i)) {
                            // IOKit and X have different numbering for the
                            // middle and right mouse buttons.
                            if (i == 1) {
                                xe.u.u.detail = 3;
                            } else if (i == 2) {
                                xe.u.u.detail = 2;
                            } else {
                                xe.u.u.detail = i + 1;
                            }
                            if (hwButtons & (1 << i)) {
#ifdef __i386__
                                xe.u.u.type = ButtonRelease;
                            } else {
                                xe.u.u.type = ButtonPress;
#else
                                xe.u.u.type = ButtonPress;
                            } else {
                                xe.u.u.type = ButtonRelease;
#endif
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

            // Special events for Quartz support
            case NX_APPDEFINED:
            if (quartz) {
                switch (ev.data.compound.subType) {

                // Update modifier state. As opposed to NX_FLAGSCHANGED,
                // in this case any amount of modifiers may have changed.
                case kXDarwinUpdateModifiers:
                    DarwinUpdateModifiers(xe, KeyRelease,
                                          old_state & ~ev.flags);
                    DarwinUpdateModifiers(xe, KeyPress,
                                          ~old_state & ev.flags);
                    old_state = ev.flags;
                    break;

                case kXDarwinShow:
                    QuartzShow(xe.u.keyButtonPointer.rootX,
                               xe.u.keyButtonPointer.rootY);
                    // The mouse location will have moved; track it.
                    xe.u.u.type = MotionNotify;
                    (darwinPointer->public.processInputProc)
                            ( &xe, darwinPointer, 1 );
                    break;

                case kXDarwinHide:
                    QuartzHide();
                    break;

                case kXDarwinQuit:
                    GiveUp(0);
                    break;

                case kXDarwinReadPasteboard:
                    QuartzReadPasteboard();
                    break;

                case kXDarwinWritePasteboard:
                    QuartzWritePasteboard();
                    break;

                default:
                    ErrorF("Unknown application defined event.\n");
                } // switch (ev.data.compound.subType)
            } // if (quartz)
            break;

            default:
                ErrorF("Unknown event caught: %d\n", ev.type);
                ErrorF("\tev.type = %d\n", ev.type);
                ErrorF("\tev.location.x,y = %d,%d\n",
                       ev.location.x, ev.location.y);
                ErrorF("\tev.time = %ld\n", ev.time);
                ErrorF("\tev.flags = 0x%x\n", ev.flags);
                ErrorF("\tev.window = %d\n", ev.window);
                ErrorF("\tev.data.key.origCharSet = %d\n",
                       ev.data.key.origCharSet);
                ErrorF("\tev.data.key.charSet = %d\n", ev.data.key.charSet);
                ErrorF("\tev.data.key.charCode = %d\n", ev.data.key.charCode);
                ErrorF("\tev.data.key.keyCode = %d\n", ev.data.key.keyCode);
                ErrorF("\tev.data.key.origCharCode = %d\n",
                       ev.data.key.origCharCode);
                break;
        }

        miPointerAbsoluteCursor( xe.u.keyButtonPointer.rootX,
                                 xe.u.keyButtonPointer.rootY,
                                 xe.u.keyButtonPointer.time );
    }

    miPointerUpdate();

}

/*
 * InitInput
 *  Register the keyboard and mouse devices
 */
void InitInput( int argc, char **argv )
{
    darwinPointer = AddInputDevice(DarwinMouseProc, TRUE);
    RegisterPointerDevice( darwinPointer );

    darwinKeyboard = AddInputDevice(DarwinKeybdProc, TRUE);
    RegisterKeyboardDevice( darwinKeyboard );
}

/*
 * InitOutput
 *  Initialize screenInfo for all actually accessible framebuffers.
 *
 *  The display mode dependent code gets called three times. The mode
 *  specific InitOutput routines are expected to discover the number
 *  of potentially useful screens and cache routes to them internally.
 *  Inside DarwinAddScreen are two other mode specific calls.
 *  A mode specific AddScreen routine is called for each screen to
 *  actually initialize the screen with the ScreenPtr structure.
 *  After other screen setup has been done, a mode specific
 *  SetupScreen function can be called to finalize screen setup.
 */
void InitOutput( ScreenInfo *pScreenInfo, int argc, char **argv )
{
    int i, left, top;
    static unsigned long generation = 0;

    pScreenInfo->imageByteOrder = IMAGE_BYTE_ORDER;
    pScreenInfo->bitmapScanlineUnit = BITMAP_SCANLINE_UNIT;
    pScreenInfo->bitmapScanlinePad = BITMAP_SCANLINE_PAD;
    pScreenInfo->bitmapBitOrder = BITMAP_BIT_ORDER;

    // List how we want common pixmap formats to be padded
    pScreenInfo->numPixmapFormats = NUMFORMATS;
    for (i = 0; i < NUMFORMATS; i++)
        pScreenInfo->formats[i] = formats[i];

    // Allocate private storage for each screen's Darwin specific info
    if (generation != serverGeneration) {
        darwinScreenIndex = AllocateScreenPrivateIndex();
        generation = serverGeneration;
    }

    // Discover screens and do mode specific initialization
    if (quartz) {
        QuartzInitOutput(argc, argv);
    } else {
        XFIOKitInitOutput(argc, argv);
    }

    // Add screens
    for (i = 0; i < darwinScreensFound; i++) {
        AddScreen( DarwinAddScreen, argc, argv );
    }

    // Shift all screens so the X11 (0, 0) coordinate is at the top left
    // of the global screen coordinates.
    // Screens can be arranged so the top left isn't on any screen,
    // so instead use the top left of the leftmost screen as (0,0).
    // This may mean some screen space is in -y, but it's better
    // that (0,0) be onscreen, or else default xterms disappear.
    // It's better that -y be used than -x, because when popup
    // menus are forced "onscreen" by dumb window managers like twm,
    // they'll shift the menus down instead of left, which still looks
    // funny but is an easier target to hit.
    left = dixScreenOrigins[0].x;
    top  = dixScreenOrigins[0].y;

    // Find leftmost screen. If there's a tie, take the topmost of the two.
    for (i = 1; i < pScreenInfo->numScreens; i++) {
        if (dixScreenOrigins[i].x < left  ||
            (dixScreenOrigins[i].x == left &&
             dixScreenOrigins[i].y < top))
        {
            left = dixScreenOrigins[i].x;
            top = dixScreenOrigins[i].y;
        }
    }

    darwinMainScreenX = left;
    darwinMainScreenY = top;

    // Shift all screens so that there is a screen whose top left
    // is at X11 (0,0) and at global screen coordinate
    // (darwinMainScreenX, darwinMainScreenY).
    if (darwinMainScreenX != 0 || darwinMainScreenY != 0) {
        for (i = 0; i < pScreenInfo->numScreens; i++) {
            dixScreenOrigins[i].x -= darwinMainScreenX;
            dixScreenOrigins[i].y -= darwinMainScreenY;
            ErrorF("Screen %d placed at X11 coordinate (%d,%d).\n",
                   i, dixScreenOrigins[i].x, dixScreenOrigins[i].y);
        }
    }
}

/*
 * OsVendorFataError
 */
void OsVendorFatalError( void )
{
    ErrorF( "   OsVendorFatalError\n" );
}

/*
 * OsVendorInit
 *  Initialization of Darwin OS support.
 */
void OsVendorInit(void)
{
    if (serverGeneration == 1) {
        DarwinPrintBanner();
    }

    // Find the full path to the keymapping file.
    if ( darwinKeymapFile ) {
        char *tempStr = DarwinFindLibraryFile(darwinKeymapFile, "Keyboards");
        if ( !tempStr )
            FatalError("Could not find keymapping file %s.\n",
                       darwinKeymapFile);
        darwinKeymapFile = tempStr;
        ErrorF("Using keymapping provided in %s.\n", darwinKeymapFile);
    } else {
        ErrorF("Reading keymapping from the kernel.\n");
    }
}

/*
 * ddxProcessArgument --
 *  Process device-dependent command line args. Returns 0 if argument is
 *  not device dependent, otherwise Count of number of elements of argv
 *  that are part of a device dependent commandline option.
 */
int ddxProcessArgument( int argc, char *argv[], int i )
{
#ifdef DARWIN_WITH_QUARTZ
    int numDone;

    if ((numDone = QuartzProcessArgument( argc, argv, i )))
        return numDone;
#endif

    if ( !strcmp( argv[i], "-fakebuttons" ) ) {
        darwinFakeButtons = TRUE;
        ErrorF( "Faking a three button mouse\n" );
        return 1;
    }

    if ( !strcmp( argv[i], "-nofakebuttons" ) ) {
        darwinFakeButtons = FALSE;
        ErrorF( "Not faking a three button mouse\n" );
        return 1;
    }

    if (!strcmp( argv[i], "-fakemouse2" ) ) {
        if ( i == argc-1 ) {
            FatalError( "-fakemouse2 must be followed by a modifer list\n" );
        }
        if (!strcasecmp(argv[i+1], "none") || !strcmp(argv[i+1], ""))
            darwinFakeMouse2Mask = 0;
        else
            darwinFakeMouse2Mask = DarwinParseModifierList(argv[i+1]);
        ErrorF("Modifier mask to fake mouse button 2 = 0x%x\n",
               darwinFakeMouse2Mask);
        return 2;
    }

    if (!strcmp( argv[i], "-fakemouse3" ) ) {
        if ( i == argc-1 ) {
            FatalError( "-fakemouse3 must be followed by a modifer list\n" );
        }
        if (!strcasecmp(argv[i+1], "none") || !strcmp(argv[i+1], ""))
            darwinFakeMouse3Mask = 0;
        else
            darwinFakeMouse3Mask = DarwinParseModifierList(argv[i+1]);
        ErrorF("Modifier mask to fake mouse button 3 = 0x%x\n",
               darwinFakeMouse3Mask);
        return 2;
    }

    if ( !strcmp( argv[i], "-keymap" ) ) {
        if ( i == argc-1 ) {
            FatalError( "-keymap must be followed by a filename\n" );
        }
        darwinKeymapFile = argv[i+1];
        return 2;
    }

    if ( !strcmp( argv[i], "-nokeymap" ) ) {
        darwinKeymapFile = NULL;
        return 1;
    }

    if ( !strcmp( argv[i], "-size" ) ) {
        if ( i >= argc-2 ) {
            FatalError( "-size must be followed by two numbers\n" );
        }
#ifdef OLD_POWERBOOK_G3
        ErrorF( "Ignoring unsupported -size option on old PowerBook G3\n" );
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
            FatalError( "-depth must be followed by a number\n" );
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
            FatalError( "Unsupported pixel depth. Use 8, 15, or 24 bits\n" );
        ErrorF( "Attempting to use pixel depth of %i\n", bitDepth );
#endif
        return 2;
    }

    if ( !strcmp( argv[i], "-refresh" ) ) {
        if ( i == argc-1 ) {
            FatalError( "-refresh must be followed by a number\n" );
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
 *  Print out correct use of device dependent commandline options.
 *  Maybe the user now knows what really to do ...
 */
void ddxUseMsg( void )
{
    ErrorF("\n");
    ErrorF("\n");
    ErrorF("Device Dependent Usage:\n");
    ErrorF("\n");
    ErrorF("-fakebuttons : fake a three button mouse with Command and Option keys.\n");
    ErrorF("-nofakebuttons : don't fake a three button mouse.\n");
    ErrorF("-fakemouse2 <modifiers> : fake middle mouse button with modifier keys.\n");
    ErrorF("-fakemouse3 <modifiers> : fake right mouse button with modifier keys.\n");
    ErrorF("  ex: -fakemouse2 \"option,shift\" = option-shift-click is middle button.\n");
    ErrorF("-keymap <file> : read the keymapping from a file instead of the kernel.\n");
    ErrorF("-version : show the server version.\n");
    ErrorF("\n");
#ifdef DARWIN_WITH_QUARTZ
    ErrorF("Quartz modes:\n");
    ErrorF("-fullscreen : run full screen in parallel with Mac OS X window server.\n");
    ErrorF("-rootless : run rootless inside Mac OS X window server.\n");
    ErrorF("-quartz : use default Mac OS X window server mode\n");
    ErrorF("\n");
    ErrorF("Options ignored in rootless mode:\n");
#endif
    ErrorF("-size <height> <width> : use a screen resolution of <height> x <width>.\n");
    ErrorF("-depth <8,15,24> : use this bit depth.\n");
    ErrorF("-refresh <rate> : use a monitor refresh rate of <rate> Hz.\n");
    ErrorF("\n");
}

/*
 * ddxGiveUp --
 *      Device dependent cleanup. Called by dix before normal server death.
 */
void ddxGiveUp( void )
{
    ErrorF( "Quitting XDarwin...\n" );

    if (quartz) {
        QuartzGiveUp();
    } else {
        XFIOKitGiveUp();
    }
}

/*
 * AbortDDX --
 *      DDX - specific abort routine.  Called by AbortServer(). The attempt is
 *      made to restore all original setting of the displays. Also all devices
 *      are closed.
 */
void AbortDDX( void )
{
    ErrorF( "   AbortDDX\n" );
    /*
     * This is needed for a abnormal server exit, since the normal exit stuff
     * MUST also be performed (i.e. the vt must be left in a defined state)
     */
    ddxGiveUp();
}

#ifdef DPMSExtension
/*
 * DPMS extension stubs
 */
Bool DPMSSupported(void)
{
    return FALSE;
}

void DPMSSet(int level)
{
}

int DPMSGet(int *level)
{
    return -1;
}
#endif


#include "mivalidate.h" // for union _Validate used by windowstr.h
#include "windowstr.h"  // for struct _Window
#include "scrnintstr.h" // for struct _Screen

// This is copied from Xserver/hw/xfree86/common/xf86Helper.c.
// Quartz mode uses this when switching in and out of Quartz.
// Quartz or IOKit can use this when waking from sleep.
// Copyright (c) 1997-1998 by The XFree86 Project, Inc.

/*
 * xf86SetRootClip --
 *	Enable or disable rendering to the screen by
 *	setting the root clip list and revalidating
 *	all of the windows
 */

void
xf86SetRootClip (ScreenPtr pScreen, BOOL enable)
{
    WindowPtr	pWin = WindowTable[pScreen->myNum];
    WindowPtr	pChild;
    Bool	WasViewable = (Bool)(pWin->viewable);
    Bool	anyMarked = TRUE;
    RegionPtr	pOldClip = NULL, bsExposed;
#ifdef DO_SAVE_UNDERS
    Bool	dosave = FALSE;
#endif
    WindowPtr   pLayerWin;
    BoxRec	box;

    if (WasViewable)
    {
	for (pChild = pWin->firstChild; pChild; pChild = pChild->nextSib)
	{
	    (void) (*pScreen->MarkOverlappedWindows)(pChild,
						     pChild,
						     &pLayerWin);
	}
	(*pScreen->MarkWindow) (pWin);
	anyMarked = TRUE;
	if (pWin->valdata)
	{
	    if (HasBorder (pWin))
	    {
		RegionPtr	borderVisible;

		borderVisible = REGION_CREATE(pScreen, NullBox, 1);
		REGION_SUBTRACT(pScreen, borderVisible,
				&pWin->borderClip, &pWin->winSize);
		pWin->valdata->before.borderVisible = borderVisible;
	    }
	    pWin->valdata->before.resized = TRUE;
	}
    }

    /*
     * Use REGION_BREAK to avoid optimizations in ValidateTree
     * that assume the root borderClip can't change well, normally
     * it doesn't...)
     */
    if (enable)
    {
	box.x1 = 0;
	box.y1 = 0;
	box.x2 = pScreen->width;
	box.y2 = pScreen->height;
	REGION_RESET(pScreen, &pWin->borderClip, &box);
	REGION_BREAK (pWin->drawable.pScreen, &pWin->clipList);
    }
    else
    {
	REGION_EMPTY(pScreen, &pWin->borderClip);
	REGION_BREAK (pWin->drawable.pScreen, &pWin->clipList);
    }

    ResizeChildrenWinSize (pWin, 0, 0, 0, 0);

    if (WasViewable)
    {
	if (pWin->backStorage)
	{
	    pOldClip = REGION_CREATE(pScreen, NullBox, 1);
	    REGION_COPY(pScreen, pOldClip, &pWin->clipList);
	}

	if (pWin->firstChild)
	{
	    anyMarked |= (*pScreen->MarkOverlappedWindows)(pWin->firstChild,
							   pWin->firstChild,
							   (WindowPtr *)NULL);
	}
	else
	{
	    (*pScreen->MarkWindow) (pWin);
	    anyMarked = TRUE;
	}

#ifdef DO_SAVE_UNDERS
	if (DO_SAVE_UNDERS(pWin))
	{
	    dosave = (*pScreen->ChangeSaveUnder)(pLayerWin, pLayerWin);
	}
#endif /* DO_SAVE_UNDERS */

	if (anyMarked)
	    (*pScreen->ValidateTree)(pWin, NullWindow, VTOther);
    }

    if (pWin->backStorage &&
	((pWin->backingStore == Always) || WasViewable))
    {
	if (!WasViewable)
	    pOldClip = &pWin->clipList; /* a convenient empty region */
	bsExposed = (*pScreen->TranslateBackingStore)
			     (pWin, 0, 0, pOldClip,
			      pWin->drawable.x, pWin->drawable.y);
	if (WasViewable)
	    REGION_DESTROY(pScreen, pOldClip);
	if (bsExposed)
	{
	    RegionPtr	valExposed = NullRegion;

	    if (pWin->valdata)
		valExposed = &pWin->valdata->after.exposed;
	    (*pScreen->WindowExposures) (pWin, valExposed, bsExposed);
	    if (valExposed)
		REGION_EMPTY(pScreen, valExposed);
	    REGION_DESTROY(pScreen, bsExposed);
	}
    }
    if (WasViewable)
    {
	if (anyMarked)
	    (*pScreen->HandleExposures)(pWin);
#ifdef DO_SAVE_UNDERS
	if (dosave)
	    (*pScreen->PostChangeSaveUnder)(pLayerWin, pLayerWin);
#endif /* DO_SAVE_UNDERS */
	if (anyMarked && pScreen->PostValidateTree)
	    (*pScreen->PostValidateTree)(pWin, NullWindow, VTOther);
    }
    if (pWin->realized)
	WindowsRestructured ();
    FlushAllOutput ();
}

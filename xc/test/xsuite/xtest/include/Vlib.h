/*
 
Copyright (c) 1990, 1991, 1992  X Consortium

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of the X Consortium shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from the X Consortium.

 *
 * Copyright 1990, 1991, 1992 by UniSoft Group Limited.
 * 
 * Permission to use, copy, modify, distribute, and sell this software and
 * its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation, and that the name of UniSoft not be
 * used in advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.  UniSoft
 * makes no representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 *
 * $XConsortium: Vlib.h,v 1.4 94/04/17 21:00:01 rws Exp $
 */
#ifndef _VLIB_H_
#define _VLIB_H_

#include <X11/Xlib.h>

#include <X11/X.h>

#ifndef NeedFunctionPrototypes
#if defined(FUNCPROTO) || defined(__STDC__) || defined(__cplusplus) || defined(c_plusplus)
#define NeedFunctionPrototypes 1
#else
#define NeedFunctionPrototypes 0
#endif /* __STDC__ */
#endif /* NeedFunctionPrototypes */

#ifndef NeedWidePrototypes
#if defined(NARROWPROTO)
#define NeedWidePrototypes 0
#else
#define NeedWidePrototypes 1		/* default to make interropt. easier */
#endif
#endif


#ifdef XLoadQueryFont
extern XFontStruct *VLoadQueryFont(
#if NeedFunctionPrototypes
    Display*		/* display */,
    const char*		/* name */
#endif
);
#endif /* XLoadQueryFont */

#ifdef XQueryFont
extern XFontStruct *VQueryFont(
#if NeedFunctionPrototypes
    Display*		/* display */,
    XID			/* font_ID */
#endif
);
#endif /* XQueryFont */


#ifdef XGetMotionEvents
extern XTimeCoord *VGetMotionEvents(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Window		/* w */,
    Time		/* start */,
    Time		/* stop */,
    int*		/* nevents_return */
#endif
);
#endif /* XGetMotionEvents */

#ifdef XDeleteModifiermapEntry
extern XModifierKeymap *VDeleteModifiermapEntry(
#if NeedFunctionPrototypes
    XModifierKeymap*	/* modmap */,
#if NeedWidePrototypes
    unsigned int	/* keycode_entry */,
#else
    KeyCode		/* keycode_entry */,
#endif
    int			/* modifier */
#endif
);
#endif /* XDeleteModifiermapEntry */

#ifdef XGetModifierMapping
extern XModifierKeymap *VGetModifierMapping(
#if NeedFunctionPrototypes
    Display*		/* display */
#endif
);
#endif /* XGetModifierMapping */

#ifdef XInsertModifiermapEntry
extern XModifierKeymap *VInsertModifiermapEntry(
#if NeedFunctionPrototypes
    XModifierKeymap*	/* modmap */,
#if NeedWidePrototypes
    unsigned int	/* keycode_entry */,
#else
    KeyCode		/* keycode_entry */,
#endif
    int			/* modifier */    
#endif
);
#endif /* XInsertModifiermapEntry */

#ifdef XNewModifiermap
extern XModifierKeymap *VNewModifiermap(
#if NeedFunctionPrototypes
    int			/* max_keys_per_mod */
#endif
);
#endif /* XNewModifiermap */

#ifdef XCreateImage
extern XImage *VCreateImage(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Visual*		/* visual */,
    unsigned int	/* depth */,
    int			/* format */,
    int			/* offset */,
    char*		/* data */,
    unsigned int	/* width */,
    unsigned int	/* height */,
    int			/* bitmap_pad */,
    int			/* bytes_per_line */
#endif
);
#endif /* XCreateImage */
#ifdef XGetImage
extern XImage *VGetImage(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Drawable		/* d */,
    int			/* x */,
    int			/* y */,
    unsigned int	/* width */,
    unsigned int	/* height */,
    unsigned long	/* plane_mask */,
    int			/* format */
#endif
);
#endif /* XGetImage */
#ifdef XGetSubImage
extern XImage *VGetSubImage(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Drawable		/* d */,
    int			/* x */,
    int			/* y */,
    unsigned int	/* width */,
    unsigned int	/* height */,
    unsigned long	/* plane_mask */,
    int			/* format */,
    XImage*		/* dest_image */,
    int			/* dest_x */,
    int			/* dest_y */
#endif
);
#endif /* XGetSubImage */

/* 
 * X function declarations.
 */
#ifdef XOpenDisplay
extern Display *VOpenDisplay(
#if NeedFunctionPrototypes
    const char*		/* display_name */
#endif
);
#endif /* XOpenDisplay */

#ifdef XrmInitialize
extern void VrmInitialize(
#if NeedFunctionPrototypes
    void
#endif
);
#endif /* XrmInitialize */

#ifdef XFetchBytes
extern char *VFetchBytes(
#if NeedFunctionPrototypes
    Display*		/* display */,
    int*		/* nbytes_return */
#endif
);
#endif /* XFetchBytes */
#ifdef XFetchBuffer
extern char *VFetchBuffer(
#if NeedFunctionPrototypes
    Display*		/* display */,
    int*		/* nbytes_return */,
    int			/* buffer */
#endif
);
#endif /* XFetchBuffer */
#ifdef XGetAtomName
extern char *VGetAtomName(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Atom		/* atom */
#endif
);
#endif /* XGetAtomName */
#ifdef XGetDefault
extern char *VGetDefault(
#if NeedFunctionPrototypes
    Display*		/* display */,
    const char*		/* program */,
    const char*		/* option */		  
#endif
);
#endif /* XGetDefault */
#ifdef XDisplayName
extern char *VDisplayName(
#if NeedFunctionPrototypes
    const char*		/* string */
#endif
);
#endif /* XDisplayName */
#ifdef XKeysymToString
extern char *VKeysymToString(
#if NeedFunctionPrototypes
    KeySym		/* keysym */
#endif
);
#endif /* XKeysymToString */

#ifdef XSynchronize
extern int (*VSynchronize(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Bool		/* onoff */
#endif
))();
#endif /* XSynchronize */
#ifdef XSetAfterFunction
extern int (*VSetAfterFunction(
#if NeedFunctionPrototypes
    Display*		/* display */,
    int (*) ( Display*			/* display */
            )		/* procedure */
#endif
))();
#endif /* XSetAfterFunction */
#ifdef XInternAtom
extern Atom VInternAtom(
#if NeedFunctionPrototypes
    Display*		/* display */,
    const char*		/* atom_name */,
    Bool		/* only_if_exists */		 
#endif
);
#endif /* XInternAtom */
#ifdef XCopyColormapAndFree
extern Colormap VCopyColormapAndFree(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Colormap		/* colormap */
#endif
);
#endif /* XCopyColormapAndFree */
#ifdef XCreateColormap
extern Colormap VCreateColormap(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Window		/* w */,
    Visual*		/* visual */,
    int			/* alloc */			 
#endif
);
#endif /* XCreateColormap */
#ifdef XCreatePixmapCursor
extern Cursor VCreatePixmapCursor(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Pixmap		/* source */,
    Pixmap		/* mask */,
    XColor*		/* foreground_color */,
    XColor*		/* background_color */,
    unsigned int	/* x */,
    unsigned int	/* y */			   
#endif
);
#endif /* XCreatePixmapCursor */
#ifdef XCreateGlyphCursor
extern Cursor VCreateGlyphCursor(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Font		/* source_font */,
    Font		/* mask_font */,
    unsigned int	/* source_char */,
    unsigned int	/* mask_char */,
    XColor*		/* foreground_color */,
    XColor*		/* background_color */
#endif
);
#endif /* XCreateGlyphCursor */
#ifdef XCreateFontCursor
extern Cursor VCreateFontCursor(
#if NeedFunctionPrototypes
    Display*		/* display */,
    unsigned int	/* shape */
#endif
);
#endif /* XCreateFontCursor */
#ifdef XLoadFont
extern Font VLoadFont(
#if NeedFunctionPrototypes
    Display*		/* display */,
    const char*		/* name */
#endif
);
#endif /* XLoadFont */
#ifdef XCreateGC
extern GC VCreateGC(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Drawable		/* d */,
    unsigned long	/* valuemask */,
    XGCValues*		/* values */
#endif
);
#endif /* XCreateGC */
#ifdef XGContextFromGC
extern GContext VGContextFromGC(
#if NeedFunctionPrototypes
    GC			/* gc */
#endif
);
#endif /* XGContextFromGC */
#ifdef XCreatePixmap
extern Pixmap VCreatePixmap(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Drawable		/* d */,
    unsigned int	/* width */,
    unsigned int	/* height */,
    unsigned int	/* depth */		        
#endif
);
#endif /* XCreatePixmap */
#ifdef XCreateBitmapFromData
extern Pixmap VCreateBitmapFromData(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Drawable		/* d */,
    const char*		/* data */,
    unsigned int	/* width */,
    unsigned int	/* height */
#endif
);
#endif /* XCreateBitmapFromData */
#ifdef XCreatePixmapFromBitmapData
extern Pixmap VCreatePixmapFromBitmapData(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Drawable		/* d */,
    char*		/* data */,
    unsigned int	/* width */,
    unsigned int	/* height */,
    unsigned long	/* fg */,
    unsigned long	/* bg */,
    unsigned int	/* depth */
#endif
);
#endif /* XCreatePixmapFromBitmapData */
#ifdef XCreateSimpleWindow
extern Window VCreateSimpleWindow(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Window		/* parent */,
    int			/* x */,
    int			/* y */,
    unsigned int	/* width */,
    unsigned int	/* height */,
    unsigned int	/* border_width */,
    unsigned long	/* border */,
    unsigned long	/* background */
#endif
);
#endif /* XCreateSimpleWindow */
#ifdef XGetSelectionOwner
extern Window VGetSelectionOwner(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Atom		/* selection */
#endif
);
#endif /* XGetSelectionOwner */
#ifdef XCreateWindow
extern Window VCreateWindow(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Window		/* parent */,
    int			/* x */,
    int			/* y */,
    unsigned int	/* width */,
    unsigned int	/* height */,
    unsigned int	/* border_width */,
    int			/* depth */,
    unsigned int	/* class */,
    Visual*		/* visual */,
    unsigned long	/* valuemask */,
    XSetWindowAttributes*	/* attributes */
#endif
); 
#endif /* XCreateWindow */
#ifdef XListInstalledColormaps
extern Colormap *VListInstalledColormaps(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Window		/* w */,
    int*		/* num_return */
#endif
);
#endif /* XListInstalledColormaps */
#ifdef XListFonts
extern char **VListFonts(
#if NeedFunctionPrototypes
    Display*		/* display */,
    const char*		/* pattern */,
    int			/* maxnames */,
    int*		/* actual_count_return */
#endif
);
#endif /* XListFonts */
#ifdef XListFontsWithInfo
extern char **VListFontsWithInfo(
#if NeedFunctionPrototypes
    Display*		/* display */,
    const char*		/* pattern */,
    int			/* maxnames */,
    int*		/* count_return */,
    XFontStruct**	/* info_return */
#endif
);
#endif /* XListFontsWithInfo */
#ifdef XGetFontPath
extern char **VGetFontPath(
#if NeedFunctionPrototypes
    Display*		/* display */,
    int*		/* npaths_return */
#endif
);
#endif /* XGetFontPath */
#ifdef XListExtensions
extern char **VListExtensions(
#if NeedFunctionPrototypes
    Display*		/* display */,
    int*		/* nextensions_return */
#endif
);
#endif /* XListExtensions */
#ifdef XListProperties
extern Atom *VListProperties(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Window		/* w */,
    int*		/* num_prop_return */
#endif
);
#endif /* XListProperties */
#ifdef XListHosts
extern XHostAddress *VListHosts(
#if NeedFunctionPrototypes
    Display*		/* display */,
    int*		/* nhosts_return */,
    Bool*		/* state_return */
#endif
);
#endif /* XListHosts */
#ifdef XKeycodeToKeysym
extern KeySym VKeycodeToKeysym(
#if NeedFunctionPrototypes
    Display*		/* display */,
#if NeedWidePrototypes
    unsigned int	/* keycode */,
#else
    KeyCode		/* keycode */,
#endif
    int			/* index */
#endif
);
#endif /* XKeycodeToKeysym */
#ifdef XLookupKeysym
extern KeySym VLookupKeysym(
#if NeedFunctionPrototypes
    XKeyEvent*		/* key_event */,
    int			/* index */
#endif
);
#endif /* XLookupKeysym */
#ifdef XGetKeyboardMapping
extern KeySym *VGetKeyboardMapping(
#if NeedFunctionPrototypes
    Display*		/* display */,
#if NeedWidePrototypes
    unsigned int	/* first_keycode */,
#else
    KeyCode		/* first_keycode */,
#endif
    int			/* keycode_count */,
    int*		/* keysyms_per_keycode_return */
#endif
);
#endif /* XGetKeyboardMapping */
#ifdef XStringToKeysym
extern KeySym VStringToKeysym(
#if NeedFunctionPrototypes
    const char*		/* string */
#endif
);
#endif /* XStringToKeysym */
#ifdef XMaxRequestSize
extern long VMaxRequestSize(
#if NeedFunctionPrototypes
    Display*		/* display */
#endif
);
#endif /* XMaxRequestSize */
#ifdef XResourceManagerString
extern char *VResourceManagerString(
#if NeedFunctionPrototypes
    Display*		/* display */
#endif
);
#endif /* XResourceManagerString */
#ifdef XDisplayMotionBufferSize
extern unsigned long VDisplayMotionBufferSize(
#if NeedFunctionPrototypes
    Display*		/* display */
#endif
);
#endif /* XDisplayMotionBufferSize */
#ifdef XVisualIDFromVisual
extern VisualID VVisualIDFromVisual(
#if NeedFunctionPrototypes
    Visual*		/* visual */
#endif
);
#endif /* XVisualIDFromVisual */

/* routines for dealing with extensions */

#ifdef XInitExtension
extern XExtCodes *VInitExtension(
#if NeedFunctionPrototypes
    Display*		/* display */,
    const char*		/* name */
#endif
);
#endif /* XInitExtension */

#ifdef XAddExtension
extern XExtCodes *VAddExtension(
#if NeedFunctionPrototypes
    Display*		/* display */
#endif
);
#endif /* XAddExtension */
#ifdef XFindOnExtensionList
extern XExtData *VFindOnExtensionList(
#if NeedFunctionPrototypes
    XExtData**		/* structure */,
    int			/* number */
#endif
);
#endif /* XFindOnExtensionList */
#ifdef XEHeadOfExtensionList
extern XExtData **VEHeadOfExtensionList(
#if NeedFunctionPrototypes
    XEDataObject	/* object */
#endif
);
#endif /* XEHeadOfExtensionList */

/* these are routines for which there are also macros */
#ifdef XRootWindow
extern Window VRootWindow(
#if NeedFunctionPrototypes
    Display*		/* display */,
    int			/* screen_number */
#endif
);
#endif /* XRootWindow */
#ifdef XDefaultRootWindow
extern Window VDefaultRootWindow(
#if NeedFunctionPrototypes
    Display*		/* display */
#endif
);
#endif /* XDefaultRootWindow */
#ifdef XRootWindowOfScreen
extern Window VRootWindowOfScreen(
#if NeedFunctionPrototypes
    Screen*		/* screen */
#endif
);
#endif /* XRootWindowOfScreen */
#ifdef XDefaultVisual
extern Visual *VDefaultVisual(
#if NeedFunctionPrototypes
    Display*		/* display */,
    int			/* screen_number */
#endif
);
#endif /* XDefaultVisual */
#ifdef XDefaultVisualOfScreen
extern Visual *VDefaultVisualOfScreen(
#if NeedFunctionPrototypes
    Screen*		/* screen */
#endif
);
#endif /* XDefaultVisualOfScreen */
#ifdef XDefaultGC
extern GC VDefaultGC(
#if NeedFunctionPrototypes
    Display*		/* display */,
    int			/* screen_number */
#endif
);
#endif /* XDefaultGC */
#ifdef XDefaultGCOfScreen
extern GC VDefaultGCOfScreen(
#if NeedFunctionPrototypes
    Screen*		/* screen */
#endif
);
#endif /* XDefaultGCOfScreen */
#ifdef XBlackPixel
extern unsigned long VBlackPixel(
#if NeedFunctionPrototypes
    Display*		/* display */,
    int			/* screen_number */
#endif
);
#endif /* XBlackPixel */
#ifdef XWhitePixel
extern unsigned long VWhitePixel(
#if NeedFunctionPrototypes
    Display*		/* display */,
    int			/* screen_number */
#endif
);
#endif /* XWhitePixel */
#ifdef XAllPlanes
extern unsigned long VAllPlanes(
#if NeedFunctionPrototypes
    void
#endif
);
#endif /* XAllPlanes */
#ifdef XBlackPixelOfScreen
extern unsigned long VBlackPixelOfScreen(
#if NeedFunctionPrototypes
    Screen*		/* screen */
#endif
);
#endif /* XBlackPixelOfScreen */
#ifdef XWhitePixelOfScreen
extern unsigned long VWhitePixelOfScreen(
#if NeedFunctionPrototypes
    Screen*		/* screen */
#endif
);
#endif /* XWhitePixelOfScreen */
#ifdef XNextRequest
extern unsigned long VNextRequest(
#if NeedFunctionPrototypes
    Display*		/* display */
#endif
);
#endif /* XNextRequest */
#ifdef XLastKnownRequestProcessed
extern unsigned long VLastKnownRequestProcessed(
#if NeedFunctionPrototypes
    Display*		/* display */
#endif
);
#endif /* XLastKnownRequestProcessed */
#ifdef XServerVendor
extern char *VServerVendor(
#if NeedFunctionPrototypes
    Display*		/* display */
#endif
);
#endif /* XServerVendor */
#ifdef XDisplayString
extern char *VDisplayString(
#if NeedFunctionPrototypes
    Display*		/* display */
#endif
);
#endif /* XDisplayString */
#ifdef XDefaultColormap
extern Colormap VDefaultColormap(
#if NeedFunctionPrototypes
    Display*		/* display */,
    int			/* screen_number */
#endif
);
#endif /* XDefaultColormap */
#ifdef XDefaultColormapOfScreen
extern Colormap VDefaultColormapOfScreen(
#if NeedFunctionPrototypes
    Screen*		/* screen */
#endif
);
#endif /* XDefaultColormapOfScreen */
#ifdef XDisplayOfScreen
extern Display *VDisplayOfScreen(
#if NeedFunctionPrototypes
    Screen*		/* screen */
#endif
);
#endif /* XDisplayOfScreen */
#ifdef XScreenOfDisplay
extern Screen *VScreenOfDisplay(
#if NeedFunctionPrototypes
    Display*		/* display */,
    int			/* screen_number */
#endif
);
#endif /* XScreenOfDisplay */
#ifdef XDefaultScreenOfDisplay
extern Screen *VDefaultScreenOfDisplay(
#if NeedFunctionPrototypes
    Display*		/* display */
#endif
);
#endif /* XDefaultScreenOfDisplay */
#ifdef XEventMaskOfScreen
extern long VEventMaskOfScreen(
#if NeedFunctionPrototypes
    Screen*		/* screen */
#endif
);
#endif /* XEventMaskOfScreen */

#ifdef XScreenNumberOfScreen
extern int VScreenNumberOfScreen(
#if NeedFunctionPrototypes
    Screen*		/* screen */
#endif
);
#endif /* XScreenNumberOfScreen */

#ifndef GENERATE_PIXMAPS
#ifdef XSetErrorHandler
extern XErrorHandler VSetErrorHandler(
#if NeedFunctionPrototypes
    XErrorHandler	/* handler */
#endif
);
#endif /* XSetErrorHandler */

#ifdef XSetIOErrorHandler
extern XIOErrorHandler VSetIOErrorHandler(
#if NeedFunctionPrototypes
    XIOErrorHandler	/* handler */
#endif
);
#endif /* XSetIOErrorHandler */
#endif /* GENERATE_PIXMAPS */


#ifdef XListPixmapFormats
extern XPixmapFormatValues *VListPixmapFormats(
#if NeedFunctionPrototypes
    Display*		/* display */,
    int*		/* count_return */
#endif
);
#endif /* XListPixmapFormats */
#ifdef XListDepths
extern int *VListDepths(
#if NeedFunctionPrototypes
    Display*		/* display */,
    int			/* screen_number */,
    int*		/* count_return */
#endif
);
#endif /* XListDepths */

/* ICCCM routines for things that don't require special include files; */
/* other declarations are given in Xutil.h                             */
#ifdef XReconfigureWMWindow
extern Status VReconfigureWMWindow(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Window		/* w */,
    int			/* screen_number */,
    unsigned int	/* mask */,
    XWindowChanges*	/* changes */
#endif
);
#endif /* XReconfigureWMWindow */

#ifdef XGetWMProtocols
extern Status VGetWMProtocols(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Window		/* w */,
    Atom**		/* protocols_return */,
    int*		/* count_return */
#endif
);
#endif /* XGetWMProtocols */
#ifdef XSetWMProtocols
extern Status VSetWMProtocols(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Window		/* w */,
    Atom*		/* protocols */,
    int			/* count */
#endif
);
#endif /* XSetWMProtocols */
#ifdef XIconifyWindow
extern Status VIconifyWindow(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Window		/* w */,
    int			/* screen_number */
#endif
);
#endif /* XIconifyWindow */
#ifdef XWithdrawWindow
extern Status VWithdrawWindow(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Window		/* w */,
    int			/* screen_number */
#endif
);
#endif /* XWithdrawWindow */
#ifdef XGetCommand
extern Status VGetCommand(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Window		/* w */,
    char***		/* argv_return */,
    int*		/* argc_return */
#endif
);
#endif /* XGetCommand */
#ifdef XGetWMColormapWindows
extern Status VGetWMColormapWindows(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Window		/* w */,
    Window**		/* windows_return */,
    int*		/* count_return */
#endif
);
#endif /* XGetWMColormapWindows */
#ifdef XSetWMColormapWindows
extern Status VSetWMColormapWindows(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Window		/* w */,
    Window*		/* colormap_windows */,
    int			/* count */
#endif
);
#endif /* XSetWMColormapWindows */
#ifdef XFreeStringList
extern void VFreeStringList(
#if NeedFunctionPrototypes
    char**		/* list */
#endif
);
#endif /* XFreeStringList */
#ifdef XSetTransientForHint
extern VSetTransientForHint(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Window		/* w */,
    Window		/* prop_window */
#endif
);
#endif /* XSetTransientForHint */

/* The following are given in alphabetical order */

#ifdef XActivateScreenSaver
extern VActivateScreenSaver(
#if NeedFunctionPrototypes
    Display*		/* display */
#endif
);
#endif /* XActivateScreenSaver */

#ifdef XAddHost
extern VAddHost(
#if NeedFunctionPrototypes
    Display*		/* display */,
    XHostAddress*	/* host */
#endif
);
#endif /* XAddHost */

#ifdef XAddHosts
extern VAddHosts(
#if NeedFunctionPrototypes
    Display*		/* display */,
    XHostAddress*	/* hosts */,
    int			/* num_hosts */    
#endif
);
#endif /* XAddHosts */

#ifdef XAddToExtensionList
extern VAddToExtensionList(
#if NeedFunctionPrototypes
    struct _XExtData**	/* structure */,
    XExtData*		/* ext_data */
#endif
);
#endif /* XAddToExtensionList */

#ifdef XAddToSaveSet
extern VAddToSaveSet(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Window		/* w */
#endif
);
#endif /* XAddToSaveSet */

#ifdef XAllocColor
extern Status VAllocColor(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Colormap		/* colormap */,
    XColor*		/* screen_in_out */
#endif
);
#endif /* XAllocColor */

#ifdef XAllocColorCells
extern Status VAllocColorCells(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Colormap		/* colormap */,
    Bool	        /* contig */,
    unsigned long*	/* plane_masks_return */,
    unsigned int	/* nplanes */,
    unsigned long*	/* pixels_return */,
    unsigned int 	/* npixels */
#endif
);
#endif /* XAllocColorCells */

#ifdef XAllocColorPlanes
extern Status VAllocColorPlanes(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Colormap		/* colormap */,
    Bool		/* contig */,
    unsigned long*	/* pixels_return */,
    int			/* ncolors */,
    int			/* nreds */,
    int			/* ngreens */,
    int			/* nblues */,
    unsigned long*	/* rmask_return */,
    unsigned long*	/* gmask_return */,
    unsigned long*	/* bmask_return */
#endif
);
#endif /* XAllocColorPlanes */

#ifdef XAllocNamedColor
extern Status VAllocNamedColor(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Colormap		/* colormap */,
    const char*		/* color_name */,
    XColor*		/* screen_def_return */,
    XColor*		/* exact_def_return */
#endif
);
#endif /* XAllocNamedColor */

#ifdef XAllowEvents
extern VAllowEvents(
#if NeedFunctionPrototypes
    Display*		/* display */,
    int			/* event_mode */,
    Time		/* time */
#endif
);
#endif /* XAllowEvents */

#ifdef XAutoRepeatOff
extern VAutoRepeatOff(
#if NeedFunctionPrototypes
    Display*		/* display */
#endif
);
#endif /* XAutoRepeatOff */

#ifdef XAutoRepeatOn
extern VAutoRepeatOn(
#if NeedFunctionPrototypes
    Display*		/* display */
#endif
);
#endif /* XAutoRepeatOn */

#ifdef XBell
extern VBell(
#if NeedFunctionPrototypes
    Display*		/* display */,
    int			/* percent */
#endif
);
#endif /* XBell */

#ifdef XBitmapBitOrder
extern int VBitmapBitOrder(
#if NeedFunctionPrototypes
    Display*		/* display */
#endif
);
#endif /* XBitmapBitOrder */

#ifdef XBitmapPad
extern int VBitmapPad(
#if NeedFunctionPrototypes
    Display*		/* display */
#endif
);
#endif /* XBitmapPad */

#ifdef XBitmapUnit
extern int VBitmapUnit(
#if NeedFunctionPrototypes
    Display*		/* display */
#endif
);
#endif /* XBitmapUnit */

#ifdef XCellsOfScreen
extern int VCellsOfScreen(
#if NeedFunctionPrototypes
    Screen*		/* screen */
#endif
);
#endif /* XCellsOfScreen */

#ifdef XChangeActivePointerGrab
extern VChangeActivePointerGrab(
#if NeedFunctionPrototypes
    Display*		/* display */,
    unsigned int	/* event_mask */,
    Cursor		/* cursor */,
    Time		/* time */
#endif
);
#endif /* XChangeActivePointerGrab */

#ifdef XChangeGC
extern VChangeGC(
#if NeedFunctionPrototypes
    Display*		/* display */,
    GC			/* gc */,
    unsigned long	/* valuemask */,
    XGCValues*		/* values */
#endif
);
#endif /* XChangeGC */

#ifdef XChangeKeyboardControl
extern VChangeKeyboardControl(
#if NeedFunctionPrototypes
    Display*		/* display */,
    unsigned long	/* value_mask */,
    XKeyboardControl*	/* values */
#endif
);
#endif /* XChangeKeyboardControl */

#ifdef XChangeKeyboardMapping
extern VChangeKeyboardMapping(
#if NeedFunctionPrototypes
    Display*		/* display */,
    int			/* first_keycode */,
    int			/* keysyms_per_keycode */,
    KeySym*		/* keysyms */,
    int			/* num_codes */
#endif
);
#endif /* XChangeKeyboardMapping */

#ifdef XChangePointerControl
extern VChangePointerControl(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Bool		/* do_accel */,
    Bool		/* do_threshold */,
    int			/* accel_numerator */,
    int			/* accel_denominator */,
    int			/* threshold */
#endif
);
#endif /* XChangePointerControl */

#ifdef XChangeProperty
extern VChangeProperty(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Window		/* w */,
    Atom		/* property */,
    Atom		/* type */,
    int			/* format */,
    int			/* mode */,
    const unsigned char*	/* data */,
    int			/* nelements */
#endif
);
#endif /* XChangeProperty */

#ifdef XChangeSaveSet
extern VChangeSaveSet(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Window		/* w */,
    int			/* change_mode */
#endif
);
#endif /* XChangeSaveSet */

#ifdef XChangeWindowAttributes
extern VChangeWindowAttributes(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Window		/* w */,
    unsigned long	/* valuemask */,
    XSetWindowAttributes* /* attributes */
#endif
);
#endif /* XChangeWindowAttributes */

#ifdef XCheckIfEvent
extern Bool VCheckIfEvent(
#if NeedFunctionPrototypes
    Display*		/* display */,
    XEvent*		/* event_return */,
    Bool (*) ( Display*			/* display */,
               XEvent*			/* event */,
               char*			/* arg */
             )		/* predicate */,
    char*		/* arg */
#endif
);
#endif /* XCheckIfEvent */

#ifdef XCheckMaskEvent
extern Bool VCheckMaskEvent(
#if NeedFunctionPrototypes
    Display*		/* display */,
    long		/* event_mask */,
    XEvent*		/* event_return */
#endif
);
#endif /* XCheckMaskEvent */

#ifdef XCheckTypedEvent
extern Bool VCheckTypedEvent(
#if NeedFunctionPrototypes
    Display*		/* display */,
    int			/* event_type */,
    XEvent*		/* event_return */
#endif
);
#endif /* XCheckTypedEvent */

#ifdef XCheckTypedWindowEvent
extern Bool VCheckTypedWindowEvent(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Window		/* w */,
    int			/* event_type */,
    XEvent*		/* event_return */
#endif
);
#endif /* XCheckTypedWindowEvent */

#ifdef XCheckWindowEvent
extern Bool VCheckWindowEvent(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Window		/* w */,
    long		/* event_mask */,
    XEvent*		/* event_return */
#endif
);
#endif /* XCheckWindowEvent */

#ifdef XCirculateSubwindows
extern VCirculateSubwindows(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Window		/* w */,
    int			/* direction */
#endif
);
#endif /* XCirculateSubwindows */

#ifdef XCirculateSubwindowsDown
extern VCirculateSubwindowsDown(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Window		/* w */
#endif
);
#endif /* XCirculateSubwindowsDown */

#ifdef XCirculateSubwindowsUp
extern VCirculateSubwindowsUp(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Window		/* w */
#endif
);
#endif /* XCirculateSubwindowsUp */

#ifdef XClearArea
extern VClearArea(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Window		/* w */,
    int			/* x */,
    int			/* y */,
    unsigned int	/* width */,
    unsigned int	/* height */,
    Bool		/* exposures */
#endif
);
#endif /* XClearArea */

#ifdef XClearWindow
extern VClearWindow(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Window		/* w */
#endif
);
#endif /* XClearWindow */

#ifdef XCloseDisplay
extern VCloseDisplay(
#if NeedFunctionPrototypes
    Display*		/* display */
#endif
);
#endif /* XCloseDisplay */

#ifdef XConfigureWindow
extern VConfigureWindow(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Window		/* w */,
    unsigned int	/* value_mask */,
    XWindowChanges*	/* values */		 
#endif
);
#endif /* XConfigureWindow */

#ifdef XConnectionNumber
extern int VConnectionNumber(
#if NeedFunctionPrototypes
    Display*		/* display */
#endif
);
#endif /* XConnectionNumber */

#ifdef XConvertSelection
extern VConvertSelection(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Atom		/* selection */,
    Atom 		/* target */,
    Atom		/* property */,
    Window		/* requestor */,
    Time		/* time */
#endif
);
#endif /* XConvertSelection */

#ifdef XCopyArea
extern VCopyArea(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Drawable		/* src */,
    Drawable		/* dest */,
    GC			/* gc */,
    int			/* src_x */,
    int			/* src_y */,
    unsigned int	/* width */,
    unsigned int	/* height */,
    int			/* dest_x */,
    int			/* dest_y */
#endif
);
#endif /* XCopyArea */

#ifdef XCopyGC
extern VCopyGC(
#if NeedFunctionPrototypes
    Display*		/* display */,
    GC			/* src */,
    unsigned long	/* valuemask */,
    GC			/* dest */
#endif
);
#endif /* XCopyGC */

#ifdef XCopyPlane
extern VCopyPlane(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Drawable		/* src */,
    Drawable		/* dest */,
    GC			/* gc */,
    int			/* src_x */,
    int			/* src_y */,
    unsigned int	/* width */,
    unsigned int	/* height */,
    int			/* dest_x */,
    int			/* dest_y */,
    unsigned long	/* plane */
#endif
);
#endif /* XCopyPlane */

#ifdef XDefaultDepth
extern int VDefaultDepth(
#if NeedFunctionPrototypes
    Display*		/* display */,
    int			/* screen_number */
#endif
);
#endif /* XDefaultDepth */

#ifdef XDefaultDepthOfScreen
extern int VDefaultDepthOfScreen(
#if NeedFunctionPrototypes
    Screen*		/* screen */
#endif
);
#endif /* XDefaultDepthOfScreen */

#ifdef XDefaultScreen
extern int VDefaultScreen(
#if NeedFunctionPrototypes
    Display*		/* display */
#endif
);
#endif /* XDefaultScreen */

#ifdef XDefineCursor
extern VDefineCursor(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Window		/* w */,
    Cursor		/* cursor */
#endif
);
#endif /* XDefineCursor */

#ifdef XDeleteProperty
extern VDeleteProperty(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Window		/* w */,
    Atom		/* property */
#endif
);
#endif /* XDeleteProperty */

#ifdef XDestroyWindow
extern VDestroyWindow(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Window		/* w */
#endif
);
#endif /* XDestroyWindow */

#ifdef XDestroySubwindows
extern VDestroySubwindows(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Window		/* w */
#endif
);
#endif /* XDestroySubwindows */

#ifdef XDoesBackingStore
extern int VDoesBackingStore(
#if NeedFunctionPrototypes
    Screen*		/* screen */    
#endif
);
#endif /* XDoesBackingStore */

#ifdef XDoesSaveUnders
extern Bool VDoesSaveUnders(
#if NeedFunctionPrototypes
    Screen*		/* screen */
#endif
);
#endif /* XDoesSaveUnders */

#ifdef XDisableAccessControl
extern VDisableAccessControl(
#if NeedFunctionPrototypes
    Display*		/* display */
#endif
);
#endif /* XDisableAccessControl */


#ifdef XDisplayCells
extern int VDisplayCells(
#if NeedFunctionPrototypes
    Display*		/* display */,
    int			/* screen_number */
#endif
);
#endif /* XDisplayCells */

#ifdef XDisplayHeight
extern int VDisplayHeight(
#if NeedFunctionPrototypes
    Display*		/* display */,
    int			/* screen_number */
#endif
);
#endif /* XDisplayHeight */

#ifdef XDisplayHeightMM
extern int VDisplayHeightMM(
#if NeedFunctionPrototypes
    Display*		/* display */,
    int			/* screen_number */
#endif
);
#endif /* XDisplayHeightMM */

#ifdef XDisplayKeycodes
extern VDisplayKeycodes(
#if NeedFunctionPrototypes
    Display*		/* display */,
    int*		/* min_keycodes_return */,
    int*		/* max_keycodes_return */
#endif
);
#endif /* XDisplayKeycodes */

#ifdef XDisplayPlanes
extern int VDisplayPlanes(
#if NeedFunctionPrototypes
    Display*		/* display */,
    int			/* screen_number */
#endif
);
#endif /* XDisplayPlanes */

#ifdef XDisplayWidth
extern int VDisplayWidth(
#if NeedFunctionPrototypes
    Display*		/* display */,
    int			/* screen_number */
#endif
);
#endif /* XDisplayWidth */

#ifdef XDisplayWidthMM
extern int VDisplayWidthMM(
#if NeedFunctionPrototypes
    Display*		/* display */,
    int			/* screen_number */
#endif
);
#endif /* XDisplayWidthMM */

#ifdef XDrawArc
extern VDrawArc(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Drawable		/* d */,
    GC			/* gc */,
    int			/* x */,
    int			/* y */,
    unsigned int	/* width */,
    unsigned int	/* height */,
    int			/* angle1 */,
    int			/* angle2 */
#endif
);
#endif /* XDrawArc */

#ifdef XDrawArcs
extern VDrawArcs(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Drawable		/* d */,
    GC			/* gc */,
    XArc*		/* arcs */,
    int			/* narcs */
#endif
);
#endif /* XDrawArcs */

#ifdef XDrawImageString
extern VDrawImageString(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Drawable		/* d */,
    GC			/* gc */,
    int			/* x */,
    int			/* y */,
    const char*		/* string */,
    int			/* length */
#endif
);
#endif /* XDrawImageString */

#ifdef XDrawImageString16
extern VDrawImageString16(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Drawable		/* d */,
    GC			/* gc */,
    int			/* x */,
    int			/* y */,
    const XChar2b*	/* string */,
    int			/* length */
#endif
);
#endif /* XDrawImageString16 */

#ifdef XDrawLine
extern VDrawLine(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Drawable		/* d */,
    GC			/* gc */,
    int			/* x1 */,
    int			/* x2 */,
    int			/* y1 */,
    int			/* y2 */
#endif
);
#endif /* XDrawLine */

#ifdef XDrawLines
extern VDrawLines(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Drawable		/* d */,
    GC			/* gc */,
    XPoint*		/* points */,
    int			/* npoints */,
    int			/* mode */
#endif
);
#endif /* XDrawLines */

#ifdef XDrawPoint
extern VDrawPoint(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Drawable		/* d */,
    GC			/* gc */,
    int			/* x */,
    int			/* y */
#endif
);
#endif /* XDrawPoint */

#ifdef XDrawPoints
extern VDrawPoints(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Drawable		/* d */,
    GC			/* gc */,
    XPoint*		/* points */,
    int			/* npoints */,
    int			/* mode */
#endif
);
#endif /* XDrawPoints */

#ifdef XDrawRectangle
extern VDrawRectangle(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Drawable		/* d */,
    GC			/* gc */,
    int			/* x */,
    int			/* y */,
    unsigned int	/* width */,
    unsigned int	/* height */
#endif
);
#endif /* XDrawRectangle */

#ifdef XDrawRectangles
extern VDrawRectangles(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Drawable		/* d */,
    GC			/* gc */,
    XRectangle*		/* rectangles */,
    int			/* nrectangles */
#endif
);
#endif /* XDrawRectangles */

#ifdef XDrawSegments
extern VDrawSegments(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Drawable		/* d */,
    GC			/* gc */,
    XSegment*		/* segments */,
    int			/* nsegments */
#endif
);
#endif /* XDrawSegments */

#ifdef XDrawString
extern VDrawString(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Drawable		/* d */,
    GC			/* gc */,
    int			/* x */,
    int			/* y */,
    const char*		/* string */,
    int			/* length */
#endif
);
#endif /* XDrawString */

#ifdef XDrawString16
extern VDrawString16(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Drawable		/* d */,
    GC			/* gc */,
    int			/* x */,
    int			/* y */,
    const XChar2b*	/* string */,
    int			/* length */
#endif
);
#endif /* XDrawString16 */

#ifdef XDrawText
extern VDrawText(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Drawable		/* d */,
    GC			/* gc */,
    int			/* x */,
    int			/* y */,
    XTextItem*		/* items */,
    int			/* nitems */
#endif
);
#endif /* XDrawText */

#ifdef XDrawText16
extern VDrawText16(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Drawable		/* d */,
    GC			/* gc */,
    int			/* x */,
    int			/* y */,
    XTextItem16*	/* items */,
    int			/* nitems */
#endif
);
#endif /* XDrawText16 */

#ifdef XEnableAccessControl
extern VEnableAccessControl(
#if NeedFunctionPrototypes
    Display*		/* display */
#endif
);
#endif /* XEnableAccessControl */

#ifdef XEventsQueued
extern int VEventsQueued(
#if NeedFunctionPrototypes
    Display*		/* display */,
    int			/* mode */
#endif
);
#endif /* XEventsQueued */

#ifdef XFetchName
extern Status VFetchName(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Window		/* w */,
    char**		/* window_name_return */
#endif
);
#endif /* XFetchName */

#ifdef XFillArc
extern VFillArc(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Drawable		/* d */,
    GC			/* gc */,
    int			/* x */,
    int			/* y */,
    unsigned int	/* width */,
    unsigned int	/* height */,
    int			/* angle1 */,
    int			/* angle2 */
#endif
);
#endif /* XFillArc */

#ifdef XFillArcs
extern VFillArcs(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Drawable		/* d */,
    GC			/* gc */,
    XArc*		/* arcs */,
    int			/* narcs */
#endif
);
#endif /* XFillArcs */

#ifdef XFillPolygon
extern VFillPolygon(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Drawable		/* d */,
    GC			/* gc */,
    XPoint*		/* points */,
    int			/* npoints */,
    int			/* shape */,
    int			/* mode */
#endif
);
#endif /* XFillPolygon */

#ifdef XFillRectangle
extern VFillRectangle(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Drawable		/* d */,
    GC			/* gc */,
    int			/* x */,
    int			/* y */,
    unsigned int	/* width */,
    unsigned int	/* height */
#endif
);
#endif /* XFillRectangle */

#ifdef XFillRectangles
extern VFillRectangles(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Drawable		/* d */,
    GC			/* gc */,
    XRectangle*		/* rectangles */,
    int			/* nrectangles */
#endif
);
#endif /* XFillRectangles */

#ifdef XFlush
extern VFlush(
#if NeedFunctionPrototypes
    Display*		/* display */
#endif
);
#endif /* XFlush */

#ifdef XForceScreenSaver
extern VForceScreenSaver(
#if NeedFunctionPrototypes
    Display*		/* display */,
    int			/* mode */
#endif
);
#endif /* XForceScreenSaver */

#ifdef XFree
extern VFree(
#if NeedFunctionPrototypes
    char*		/* data */
#endif
);
#endif /* XFree */

#ifdef XFreeColormap
extern VFreeColormap(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Colormap		/* colormap */
#endif
);
#endif /* XFreeColormap */

#ifdef XFreeColors
extern VFreeColors(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Colormap		/* colormap */,
    unsigned long*	/* pixels */,
    int			/* npixels */,
    unsigned long	/* planes */
#endif
);
#endif /* XFreeColors */

#ifdef XFreeCursor
extern VFreeCursor(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Cursor		/* cursor */
#endif
);
#endif /* XFreeCursor */

#ifdef XFreeExtensionList
extern VFreeExtensionList(
#if NeedFunctionPrototypes
    char**		/* list */    
#endif
);
#endif /* XFreeExtensionList */

#ifdef XFreeFont
extern VFreeFont(
#if NeedFunctionPrototypes
    Display*		/* display */,
    XFontStruct*	/* font_struct */
#endif
);
#endif /* XFreeFont */

#ifdef XFreeFontInfo
extern VFreeFontInfo(
#if NeedFunctionPrototypes
    char**		/* names */,
    XFontStruct*	/* free_info */,
    int			/* actual_count */
#endif
);
#endif /* XFreeFontInfo */

#ifdef XFreeFontNames
extern VFreeFontNames(
#if NeedFunctionPrototypes
    char**		/* list */
#endif
);
#endif /* XFreeFontNames */

#ifdef XFreeFontPath
extern VFreeFontPath(
#if NeedFunctionPrototypes
    char**		/* list */
#endif
);
#endif /* XFreeFontPath */

#ifdef XFreeGC
extern VFreeGC(
#if NeedFunctionPrototypes
    Display*		/* display */,
    GC			/* gc */
#endif
);
#endif /* XFreeGC */

#ifdef XFreeModifiermap
extern VFreeModifiermap(
#if NeedFunctionPrototypes
    XModifierKeymap*	/* modmap */
#endif
);
#endif /* XFreeModifiermap */

#ifdef XFreePixmap
extern VFreePixmap(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Pixmap		/* pixmap */
#endif
);
#endif /* XFreePixmap */

#ifdef XGeometry
extern int VGeometry(
#if NeedFunctionPrototypes
    Display*		/* display */,
    int			/* screen */,
    const char*		/* position */,
    const char*		/* default_position */,
    unsigned int	/* bwidth */,
    unsigned int	/* fwidth */,
    unsigned int	/* fheight */,
    int			/* xadder */,
    int			/* yadder */,
    int*		/* x_return */,
    int*		/* y_return */,
    int*		/* width_return */,
    int*		/* height_return */
#endif
);
#endif /* XGeometry */

#ifdef XGetErrorDatabaseText
extern VGetErrorDatabaseText(
#if NeedFunctionPrototypes
    Display*		/* display */,
    const char*		/* name */,
    const char*		/* message */,
    const char*		/* default_string */,
    char*		/* buffer_return */,
    int			/* length */
#endif
);
#endif /* XGetErrorDatabaseText */

#ifdef XGetErrorText
extern VGetErrorText(
#if NeedFunctionPrototypes
    Display*		/* display */,
    int			/* code */,
    char*		/* buffer_return */,
    int			/* length */
#endif
);
#endif /* XGetErrorText */

#ifdef XGetFontProperty
extern Bool VGetFontProperty(
#if NeedFunctionPrototypes
    XFontStruct*	/* font_struct */,
    Atom		/* atom */,
    unsigned long*	/* value_return */
#endif
);
#endif /* XGetFontProperty */

#ifdef XGetGCValues
extern Status VGetGCValues(
#if NeedFunctionPrototypes
    Display*		/* display */,
    GC			/* gc */,
    unsigned long	/* valuemask */,
    XGCValues*		/* values_return */
#endif
);
#endif /* XGetGCValues */

#ifdef XGetGeometry
extern Status VGetGeometry(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Drawable		/* d */,
    Window*		/* root_return */,
    int*		/* x_return */,
    int*		/* y_return */,
    unsigned int*	/* width_return */,
    unsigned int*	/* height_return */,
    unsigned int*	/* border_width_return */,
    unsigned int*	/* depth_return */
#endif
);
#endif /* XGetGeometry */

#ifdef XGetIconName
extern Status VGetIconName(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Window		/* w */,
    char**		/* icon_name_return */
#endif
);
#endif /* XGetIconName */

#ifdef XGetInputFocus
extern VGetInputFocus(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Window*		/* focus_return */,
    int*		/* revert_to_return */
#endif
);
#endif /* XGetInputFocus */

#ifdef XGetKeyboardControl
extern VGetKeyboardControl(
#if NeedFunctionPrototypes
    Display*		/* display */,
    XKeyboardState*	/* values_return */
#endif
);
#endif /* XGetKeyboardControl */

#ifdef XGetPointerControl
extern VGetPointerControl(
#if NeedFunctionPrototypes
    Display*		/* display */,
    int*		/* accel_numerator_return */,
    int*		/* accel_denominator_return */,
    int*		/* threshold_return */
#endif
);
#endif /* XGetPointerControl */

#ifdef XGetPointerMapping
extern int VGetPointerMapping(
#if NeedFunctionPrototypes
    Display*		/* display */,
    unsigned char*	/* map_return */,
    int			/* nmap */
#endif
);
#endif /* XGetPointerMapping */

#ifdef XGetScreenSaver
extern VGetScreenSaver(
#if NeedFunctionPrototypes
    Display*		/* display */,
    int*		/* timeout_return */,
    int*		/* interval_return */,
    int*		/* prefer_blanking_return */,
    int*		/* allow_exposures_return */
#endif
);
#endif /* XGetScreenSaver */

#ifdef XGetTransientForHint
extern Status VGetTransientForHint(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Window		/* w */,
    Window*		/* prop_window_return */
#endif
);
#endif /* XGetTransientForHint */

#ifdef XGetWindowProperty
extern int VGetWindowProperty(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Window		/* w */,
    Atom		/* property */,
    long		/* long_offset */,
    long		/* long_length */,
    Bool		/* delete */,
    Atom		/* req_type */,
    Atom*		/* actual_type_return */,
    int*		/* actual_format_return */,
    unsigned long*	/* nitems_return */,
    unsigned long*	/* bytes_after_return */,
    unsigned char**	/* prop_return */
#endif
);
#endif /* XGetWindowProperty */

#ifdef XGetWindowAttributes
extern Status VGetWindowAttributes(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Window		/* w */,
    XWindowAttributes*	/* window_attributes_return */
#endif
);
#endif /* XGetWindowAttributes */

#ifdef XGrabButton
extern VGrabButton(
#if NeedFunctionPrototypes
    Display*		/* display */,
    unsigned int	/* button */,
    unsigned int	/* modifiers */,
    Window		/* grab_window */,
    Bool		/* owner_events */,
    unsigned int	/* event_mask */,
    int			/* pointer_mode */,
    int			/* keyboard_mode */,
    Window		/* confine_to */,
    Cursor		/* cursor */
#endif
);
#endif /* XGrabButton */

#ifdef XGrabKey
extern VGrabKey(
#if NeedFunctionPrototypes
    Display*		/* display */,
    int			/* keycode */,
    unsigned int	/* modifiers */,
    Window		/* grab_window */,
    Bool		/* owner_events */,
    int			/* pointer_mode */,
    int			/* keyboard_mode */
#endif
);
#endif /* XGrabKey */

#ifdef XGrabKeyboard
extern int VGrabKeyboard(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Window		/* grab_window */,
    Bool		/* owner_events */,
    int			/* pointer_mode */,
    int			/* keyboard_mode */,
    Time		/* time */
#endif
);
#endif /* XGrabKeyboard */

#ifdef XGrabPointer
extern int VGrabPointer(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Window		/* grab_window */,
    Bool		/* owner_events */,
    unsigned int	/* event_mask */,
    int			/* pointer_mode */,
    int			/* keyboard_mode */,
    Window		/* confine_to */,
    Cursor		/* cursor */,
    Time		/* time */
#endif
);
#endif /* XGrabPointer */

#ifdef XGrabServer
extern VGrabServer(
#if NeedFunctionPrototypes
    Display*		/* display */
#endif
);
#endif /* XGrabServer */

#ifdef XHeightMMOfScreen
extern int VHeightMMOfScreen(
#if NeedFunctionPrototypes
    Screen*		/* screen */
#endif
);
#endif /* XHeightMMOfScreen */

#ifdef XHeightOfScreen
extern int VHeightOfScreen(
#if NeedFunctionPrototypes
    Screen*		/* screen */
#endif
);
#endif /* XHeightOfScreen */

#ifdef XIfEvent
extern VIfEvent(
#if NeedFunctionPrototypes
    Display*		/* display */,
    XEvent*		/* event_return */,
    Bool (*) ( Display*			/* display */,
               XEvent*			/* event */,
               char*			/* arg */
             )		/* predicate */,
    char*		/* arg */
#endif
);
#endif /* XIfEvent */

#ifdef XImageByteOrder
extern int VImageByteOrder(
#if NeedFunctionPrototypes
    Display*		/* display */
#endif
);
#endif /* XImageByteOrder */

#ifdef XInstallColormap
extern VInstallColormap(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Colormap		/* colormap */
#endif
);
#endif /* XInstallColormap */

#ifdef XKeysymToKeycode
extern KeyCode VKeysymToKeycode(
#if NeedFunctionPrototypes
    Display*		/* display */,
    KeySym		/* keysym */
#endif
);
#endif /* XKeysymToKeycode */

#ifdef XKillClient
extern VKillClient(
#if NeedFunctionPrototypes
    Display*		/* display */,
    XID			/* resource */
#endif
);
#endif /* XKillClient */

#ifdef XLastKnownRequestProcessed
extern unsigned long VLastKnownRequestProcessed(
#if NeedFunctionPrototypes
    Display*		/* display */
#endif
);
#endif /* XLastKnownRequestProcessed */

#ifdef XLookupColor
extern Status VLookupColor(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Colormap		/* colormap */,
    const char*		/* color_name */,
    XColor*		/* exact_def_return */,
    XColor*		/* screen_def_return */
#endif
);
#endif /* XLookupColor */

#ifdef XLowerWindow
extern VLowerWindow(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Window		/* w */
#endif
);
#endif /* XLowerWindow */

#ifdef XMapRaised
extern VMapRaised(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Window		/* w */
#endif
);
#endif /* XMapRaised */

#ifdef XMapSubwindows
extern VMapSubwindows(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Window		/* w */
#endif
);
#endif /* XMapSubwindows */

#ifdef XMapWindow
extern VMapWindow(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Window		/* w */
#endif
);
#endif /* XMapWindow */

#ifdef XMaskEvent
extern VMaskEvent(
#if NeedFunctionPrototypes
    Display*		/* display */,
    long		/* event_mask */,
    XEvent*		/* event_return */
#endif
);
#endif /* XMaskEvent */

#ifdef XMaxCmapsOfScreen
extern int VMaxCmapsOfScreen(
#if NeedFunctionPrototypes
    Screen*		/* screen */
#endif
);
#endif /* XMaxCmapsOfScreen */

#ifdef XMinCmapsOfScreen
extern int VMinCmapsOfScreen(
#if NeedFunctionPrototypes
    Screen*		/* screen */
#endif
);
#endif /* XMinCmapsOfScreen */

#ifdef XMoveResizeWindow
extern VMoveResizeWindow(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Window		/* w */,
    int			/* x */,
    int			/* y */,
    unsigned int	/* width */,
    unsigned int	/* height */
#endif
);
#endif /* XMoveResizeWindow */

#ifdef XMoveWindow
extern VMoveWindow(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Window		/* w */,
    int			/* x */,
    int			/* y */
#endif
);
#endif /* XMoveWindow */

#ifndef GENERATE_PIXMAPS
#ifdef XNextEvent
extern VNextEvent(
#if NeedFunctionPrototypes
    Display*		/* display */,
    XEvent*		/* event_return */
#endif
);
#endif /* XNextEvent */
#endif /* GENERATE_PIXMAPS */

#ifdef XNoOp
extern VNoOp(
#if NeedFunctionPrototypes
    Display*		/* display */
#endif
);
#endif /* XNoOp */

#ifdef XParseColor
extern Status VParseColor(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Colormap		/* colormap */,
    const char*		/* spec */,
    XColor*		/* exact_def_return */
#endif
);
#endif /* XParseColor */

#ifdef XParseGeometry
extern int VParseGeometry(
#if NeedFunctionPrototypes
    const char*		/* parsestring */,
    int*		/* x_return */,
    int*		/* y_return */,
    unsigned int*	/* width_return */,
    unsigned int*	/* height_return */
#endif
);
#endif /* XParseGeometry */

#ifdef XPeekEvent
extern VPeekEvent(
#if NeedFunctionPrototypes
    Display*		/* display */,
    XEvent*		/* event_return */
#endif
);
#endif /* XPeekEvent */

#ifdef XPeekIfEvent
extern VPeekIfEvent(
#if NeedFunctionPrototypes
    Display*		/* display */,
    XEvent*		/* event_return */,
    Bool (*) ( Display*		/* display */,
               XEvent*		/* event */,
               char*		/* arg */
             )		/* predicate */,
    char*		/* arg */
#endif
);
#endif /* XPeekIfEvent */

#ifdef XPending
extern int VPending(
#if NeedFunctionPrototypes
    Display*		/* display */
#endif
);
#endif /* XPending */

#ifdef XPlanesOfScreen
extern int VPlanesOfScreen(
#if NeedFunctionPrototypes
    Screen*		/* screen */
    
#endif
);
#endif /* XPlanesOfScreen */

#ifdef XProtocolRevision
extern int VProtocolRevision(
#if NeedFunctionPrototypes
    Display*		/* display */
#endif
);
#endif /* XProtocolRevision */

#ifdef XProtocolVersion
extern int VProtocolVersion(
#if NeedFunctionPrototypes
    Display*		/* display */
#endif
);
#endif /* XProtocolVersion */


#ifdef XPutBackEvent
extern VPutBackEvent(
#if NeedFunctionPrototypes
    Display*		/* display */,
    XEvent*		/* event */
#endif
);
#endif /* XPutBackEvent */

#ifdef XPutImage
extern VPutImage(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Drawable		/* d */,
    GC			/* gc */,
    XImage*		/* image */,
    int			/* src_x */,
    int			/* src_y */,
    int			/* dest_x */,
    int			/* dest_y */,
    unsigned int	/* width */,
    unsigned int	/* height */	  
#endif
);
#endif /* XPutImage */

#ifdef XQLength
extern int VQLength(
#if NeedFunctionPrototypes
    Display*		/* display */
#endif
);
#endif /* XQLength */

#ifdef XQueryBestCursor
extern Status VQueryBestCursor(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Drawable		/* d */,
    unsigned int        /* width */,
    unsigned int	/* height */,
    unsigned int*	/* width_return */,
    unsigned int*	/* height_return */
#endif
);
#endif /* XQueryBestCursor */

#ifdef XQueryBestSize
extern Status VQueryBestSize(
#if NeedFunctionPrototypes
    Display*		/* display */,
    int			/* class */,
    Drawable		/* which_screen */,
    unsigned int	/* width */,
    unsigned int	/* height */,
    unsigned int*	/* width_return */,
    unsigned int*	/* height_return */
#endif
);
#endif /* XQueryBestSize */

#ifdef XQueryBestStipple
extern Status VQueryBestStipple(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Drawable		/* which_screen */,
    unsigned int	/* width */,
    unsigned int	/* height */,
    unsigned int*	/* width_return */,
    unsigned int*	/* height_return */
#endif
);
#endif /* XQueryBestStipple */

#ifdef XQueryBestTile
extern Status VQueryBestTile(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Drawable		/* which_screen */,
    unsigned int	/* width */,
    unsigned int	/* height */,
    unsigned int*	/* width_return */,
    unsigned int*	/* height_return */
#endif
);
#endif /* XQueryBestTile */

#ifdef XQueryColor
extern VQueryColor(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Colormap		/* colormap */,
    XColor*		/* def_in_out */
#endif
);
#endif /* XQueryColor */

#ifdef XQueryColors
extern VQueryColors(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Colormap		/* colormap */,
    XColor*		/* defs_in_out */,
    int			/* ncolors */
#endif
);
#endif /* XQueryColors */

#ifdef XQueryExtension
extern Bool VQueryExtension(
#if NeedFunctionPrototypes
    Display*		/* display */,
    const char*		/* name */,
    int*		/* major_opcode_return */,
    int*		/* first_event_return */,
    int*		/* first_error_return */
#endif
);
#endif /* XQueryExtension */

#ifdef XQueryKeymap
extern VQueryKeymap(
#if NeedFunctionPrototypes
    Display*		/* display */,
    char [32]		/* keys_return */
#endif
);
#endif /* XQueryKeymap */

#ifdef XQueryPointer
extern Bool VQueryPointer(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Window		/* w */,
    Window*		/* root_return */,
    Window*		/* child_return */,
    int*		/* root_x_return */,
    int*		/* root_y_return */,
    int*		/* win_x_return */,
    int*		/* win_y_return */,
    unsigned int*       /* mask_return */
#endif
);
#endif /* XQueryPointer */

#ifdef XQueryTextExtents
extern VQueryTextExtents(
#if NeedFunctionPrototypes
    Display*		/* display */,
    XID			/* font_ID */,
    const char*		/* string */,
    int			/* nchars */,
    int*		/* direction_return */,
    int*		/* font_ascent_return */,
    int*		/* font_descent_return */,
    XCharStruct*	/* overall_return */    
#endif
);
#endif /* XQueryTextExtents */

#ifdef XQueryTextExtents16
extern VQueryTextExtents16(
#if NeedFunctionPrototypes
    Display*		/* display */,
    XID			/* font_ID */,
    const XChar2b*	/* string */,
    int			/* nchars */,
    int*		/* direction_return */,
    int*		/* font_ascent_return */,
    int*		/* font_descent_return */,
    XCharStruct*	/* overall_return */
#endif
);
#endif /* XQueryTextExtents16 */

#ifdef XQueryTree
extern Status VQueryTree(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Window		/* w */,
    Window*		/* root_return */,
    Window*		/* parent_return */,
    Window**		/* children_return */,
    unsigned int*	/* nchildren_return */
#endif
);
#endif /* XQueryTree */

#ifdef XRaiseWindow
extern VRaiseWindow(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Window		/* w */
#endif
);
#endif /* XRaiseWindow */

#ifdef XReadBitmapFile
extern int VReadBitmapFile(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Drawable 		/* d */,
    const char*		/* filename */,
    unsigned int*	/* width_return */,
    unsigned int*	/* height_return */,
    Pixmap*		/* bitmap_return */,
    int*		/* x_hot_return */,
    int*		/* y_hot_return */
#endif
);
#endif /* XReadBitmapFile */

#ifdef XRebindKeysym
extern VRebindKeysym(
#if NeedFunctionPrototypes
    Display*		/* display */,
    KeySym		/* keysym */,
    KeySym*		/* list */,
    int			/* mod_count */,
    const unsigned char*	/* string */,
    int			/* bytes_string */
#endif
);
#endif /* XRebindKeysym */

#ifdef XRecolorCursor
extern VRecolorCursor(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Cursor		/* cursor */,
    XColor*		/* foreground_color */,
    XColor*		/* background_color */
#endif
);
#endif /* XRecolorCursor */

#ifdef XRefreshKeyboardMapping
extern VRefreshKeyboardMapping(
#if NeedFunctionPrototypes
    XMappingEvent*	/* event_map */    
#endif
);
#endif /* XRefreshKeyboardMapping */

#ifdef XRemoveFromSaveSet
extern VRemoveFromSaveSet(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Window		/* w */
#endif
);
#endif /* XRemoveFromSaveSet */

#ifdef XRemoveHost
extern VRemoveHost(
#if NeedFunctionPrototypes
    Display*		/* display */,
    XHostAddress*	/* host */
#endif
);
#endif /* XRemoveHost */

#ifdef XRemoveHosts
extern VRemoveHosts(
#if NeedFunctionPrototypes
    Display*		/* display */,
    XHostAddress*	/* hosts */,
    int			/* num_hosts */
#endif
);
#endif /* XRemoveHosts */

#ifdef XReparentWindow
extern VReparentWindow(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Window		/* w */,
    Window		/* parent */,
    int			/* x */,
    int			/* y */
#endif
);
#endif /* XReparentWindow */

#ifdef XResetScreenSaver
extern VResetScreenSaver(
#if NeedFunctionPrototypes
    Display*		/* display */
#endif
);
#endif /* XResetScreenSaver */

#ifdef XResizeWindow
extern VResizeWindow(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Window		/* w */,
    unsigned int	/* width */,
    unsigned int	/* height */
#endif
);
#endif /* XResizeWindow */

#ifdef XRestackWindows
extern VRestackWindows(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Window*		/* windows */,
    int			/* nwindows */
#endif
);
#endif /* XRestackWindows */

#ifdef XRotateBuffers
extern VRotateBuffers(
#if NeedFunctionPrototypes
    Display*		/* display */,
    int			/* rotate */
#endif
);
#endif /* XRotateBuffers */

#ifdef XRotateWindowProperties
extern VRotateWindowProperties(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Window		/* w */,
    Atom*		/* properties */,
    int			/* num_prop */,
    int			/* npositions */
#endif
);
#endif /* XRotateWindowProperties */

#ifdef XScreenCount
extern int VScreenCount(
#if NeedFunctionPrototypes
    Display*		/* display */
#endif
);
#endif /* XScreenCount */

#ifdef XSelectInput
extern VSelectInput(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Window		/* w */,
    long		/* event_mask */
#endif
);
#endif /* XSelectInput */

#ifdef XSendEvent
extern Status VSendEvent(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Window		/* w */,
    Bool		/* propagate */,
    long		/* event_mask */,
    XEvent*		/* event_send */
#endif
);
#endif /* XSendEvent */

#ifdef XSetAccessControl
extern VSetAccessControl(
#if NeedFunctionPrototypes
    Display*		/* display */,
    int			/* mode */
#endif
);
#endif /* XSetAccessControl */

#ifdef XSetArcMode
extern VSetArcMode(
#if NeedFunctionPrototypes
    Display*		/* display */,
    GC			/* gc */,
    int			/* arc_mode */
#endif
);
#endif /* XSetArcMode */

#ifdef XSetBackground
extern VSetBackground(
#if NeedFunctionPrototypes
    Display*		/* display */,
    GC			/* gc */,
    unsigned long	/* background */
#endif
);
#endif /* XSetBackground */

#ifdef XSetClipMask
extern VSetClipMask(
#if NeedFunctionPrototypes
    Display*		/* display */,
    GC			/* gc */,
    Pixmap		/* pixmap */
#endif
);
#endif /* XSetClipMask */

#ifdef XSetClipOrigin
extern VSetClipOrigin(
#if NeedFunctionPrototypes
    Display*		/* display */,
    GC			/* gc */,
    int			/* clip_x_origin */,
    int			/* clip_y_origin */
#endif
);
#endif /* XSetClipOrigin */

#ifdef XSetClipRectangles
extern VSetClipRectangles(
#if NeedFunctionPrototypes
    Display*		/* display */,
    GC			/* gc */,
    int			/* clip_x_origin */,
    int			/* clip_y_origin */,
    XRectangle*		/* rectangles */,
    int			/* n */,
    int			/* ordering */
#endif
);
#endif /* XSetClipRectangles */

#ifdef XSetCloseDownMode
extern VSetCloseDownMode(
#if NeedFunctionPrototypes
    Display*		/* display */,
    int			/* close_mode */
#endif
);
#endif /* XSetCloseDownMode */

#ifdef XSetCommand
extern VSetCommand(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Window		/* w */,
    char**		/* argv */,
    int			/* argc */
#endif
);
#endif /* XSetCommand */

#ifdef XSetDashes
extern VSetDashes(
#if NeedFunctionPrototypes
    Display*		/* display */,
    GC			/* gc */,
    int			/* dash_offset */,
    const char*		/* dash_list */,
    int			/* n */
#endif
);
#endif /* XSetDashes */

#ifdef XSetFillRule
extern VSetFillRule(
#if NeedFunctionPrototypes
    Display*		/* display */,
    GC			/* gc */,
    int			/* fill_rule */
#endif
);
#endif /* XSetFillRule */

#ifdef XSetFillStyle
extern VSetFillStyle(
#if NeedFunctionPrototypes
    Display*		/* display */,
    GC			/* gc */,
    int			/* fill_style */
#endif
);
#endif /* XSetFillStyle */

#ifdef XSetFont
extern VSetFont(
#if NeedFunctionPrototypes
    Display*		/* display */,
    GC			/* gc */,
    Font		/* font */
#endif
);
#endif /* XSetFont */

#ifdef XSetFontPath
extern VSetFontPath(
#if NeedFunctionPrototypes
    Display*		/* display */,
    char**		/* directories */,
    int			/* ndirs */	     
#endif
);
#endif /* XSetFontPath */

#ifdef XSetForeground
extern VSetForeground(
#if NeedFunctionPrototypes
    Display*		/* display */,
    GC			/* gc */,
    unsigned long	/* foreground */
#endif
);
#endif /* XSetForeground */

#ifdef XSetFunction
extern VSetFunction(
#if NeedFunctionPrototypes
    Display*		/* display */,
    GC			/* gc */,
    int			/* function */
#endif
);
#endif /* XSetFunction */

#ifdef XSetGraphicsExposures
extern VSetGraphicsExposures(
#if NeedFunctionPrototypes
    Display*		/* display */,
    GC			/* gc */,
    Bool		/* graphics_exposures */
#endif
);
#endif /* XSetGraphicsExposures */

#ifdef XSetIconName
extern VSetIconName(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Window		/* w */,
    const char*		/* icon_name */
#endif
);
#endif /* XSetIconName */

#ifdef XSetInputFocus
extern VSetInputFocus(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Window		/* focus */,
    int			/* revert_to */,
    Time		/* time */
#endif
);
#endif /* XSetInputFocus */

#ifdef XSetLineAttributes
extern VSetLineAttributes(
#if NeedFunctionPrototypes
    Display*		/* display */,
    GC			/* gc */,
    unsigned int	/* line_width */,
    int			/* line_style */,
    int			/* cap_style */,
    int			/* join_style */
#endif
);
#endif /* XSetLineAttributes */

#ifdef XSetModifierMapping
extern int VSetModifierMapping(
#if NeedFunctionPrototypes
    Display*		/* display */,
    XModifierKeymap*	/* modmap */
#endif
);
#endif /* XSetModifierMapping */

#ifdef XSetPlaneMask
extern VSetPlaneMask(
#if NeedFunctionPrototypes
    Display*		/* display */,
    GC			/* gc */,
    unsigned long	/* plane_mask */
#endif
);
#endif /* XSetPlaneMask */

#ifdef XSetPointerMapping
extern int VSetPointerMapping(
#if NeedFunctionPrototypes
    Display*		/* display */,
    const unsigned char*	/* map */,
    int			/* nmap */
#endif
);
#endif /* XSetPointerMapping */

#ifdef XSetScreenSaver
extern VSetScreenSaver(
#if NeedFunctionPrototypes
    Display*		/* display */,
    int			/* timeout */,
    int			/* interval */,
    int			/* prefer_blanking */,
    int			/* allow_exposures */
#endif
);
#endif /* XSetScreenSaver */

#ifdef XSetSelectionOwner
extern VSetSelectionOwner(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Atom	        /* selection */,
    Window		/* owner */,
    Time		/* time */
#endif
);
#endif /* XSetSelectionOwner */

#ifdef XSetState
extern VSetState(
#if NeedFunctionPrototypes
    Display*		/* display */,
    GC			/* gc */,
    unsigned long 	/* foreground */,
    unsigned long	/* background */,
    int			/* function */,
    unsigned long	/* plane_mask */
#endif
);
#endif /* XSetState */

#ifdef XSetStipple
extern VSetStipple(
#if NeedFunctionPrototypes
    Display*		/* display */,
    GC			/* gc */,
    Pixmap		/* stipple */
#endif
);
#endif /* XSetStipple */

#ifdef XSetSubwindowMode
extern VSetSubwindowMode(
#if NeedFunctionPrototypes
    Display*		/* display */,
    GC			/* gc */,
    int			/* subwindow_mode */
#endif
);
#endif /* XSetSubwindowMode */

#ifdef XSetTSOrigin
extern VSetTSOrigin(
#if NeedFunctionPrototypes
    Display*		/* display */,
    GC			/* gc */,
    int			/* ts_x_origin */,
    int			/* ts_y_origin */
#endif
);
#endif /* XSetTSOrigin */

#ifdef XSetTile
extern VSetTile(
#if NeedFunctionPrototypes
    Display*		/* display */,
    GC			/* gc */,
    Pixmap		/* tile */
#endif
);
#endif /* XSetTile */

#ifdef XSetWindowBackground
extern VSetWindowBackground(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Window		/* w */,
    unsigned long	/* background_pixel */
#endif
);
#endif /* XSetWindowBackground */

#ifdef XSetWindowBackgroundPixmap
extern VSetWindowBackgroundPixmap(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Window		/* w */,
    Pixmap		/* background_pixmap */
#endif
);
#endif /* XSetWindowBackgroundPixmap */

#ifdef XSetWindowBorder
extern VSetWindowBorder(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Window		/* w */,
    unsigned long	/* border_pixel */
#endif
);
#endif /* XSetWindowBorder */

#ifdef XSetWindowBorderPixmap
extern VSetWindowBorderPixmap(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Window		/* w */,
    Pixmap		/* border_pixmap */
#endif
);
#endif /* XSetWindowBorderPixmap */

#ifdef XSetWindowBorderWidth
extern VSetWindowBorderWidth(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Window		/* w */,
    unsigned int	/* width */
#endif
);
#endif /* XSetWindowBorderWidth */

#ifdef XSetWindowColormap
extern VSetWindowColormap(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Window		/* w */,
    Colormap		/* colormap */
#endif
);
#endif /* XSetWindowColormap */

#ifdef XStoreBuffer
extern VStoreBuffer(
#if NeedFunctionPrototypes
    Display*		/* display */,
    const char*		/* bytes */,
    int			/* nbytes */,
    int			/* buffer */
#endif
);
#endif /* XStoreBuffer */

#ifdef XStoreBytes
extern VStoreBytes(
#if NeedFunctionPrototypes
    Display*		/* display */,
    const char*		/* bytes */,
    int			/* nbytes */
#endif
);
#endif /* XStoreBytes */

#ifdef XStoreColor
extern VStoreColor(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Colormap		/* colormap */,
    XColor*		/* color */
#endif
);
#endif /* XStoreColor */

#ifdef XStoreColors
extern VStoreColors(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Colormap		/* colormap */,
    XColor*		/* color */,
    int			/* ncolors */
#endif
);
#endif /* XStoreColors */

#ifdef XStoreName
extern VStoreName(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Window		/* w */,
    const char*		/* window_name */
#endif
);
#endif /* XStoreName */

#ifdef XStoreNamedColor
extern VStoreNamedColor(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Colormap		/* colormap */,
    const char*		/* color */,
    unsigned long	/* pixel */,
    int			/* flags */
#endif
);
#endif /* XStoreNamedColor */

#ifdef XSync
extern VSync(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Bool		/* discard */
#endif
);
#endif /* XSync */

#ifdef XTextExtents
extern VTextExtents(
#if NeedFunctionPrototypes
    XFontStruct*	/* font_struct */,
    const char*		/* string */,
    int			/* nchars */,
    int*		/* direction_return */,
    int*		/* font_ascent_return */,
    int*		/* font_descent_return */,
    XCharStruct*	/* overall_return */
#endif
);
#endif /* XTextExtents */

#ifdef XTextExtents16
extern VTextExtents16(
#if NeedFunctionPrototypes
    XFontStruct*	/* font_struct */,
    const XChar2b*	/* string */,
    int			/* nchars */,
    int*		/* direction_return */,
    int*		/* font_ascent_return */,
    int*		/* font_descent_return */,
    XCharStruct*	/* overall_return */
#endif
);
#endif /* XTextExtents16 */

#ifdef XTextWidth
extern int VTextWidth(
#if NeedFunctionPrototypes
    XFontStruct*	/* font_struct */,
    const char*		/* string */,
    int			/* count */
#endif
);
#endif /* XTextWidth */

#ifdef XTextWidth16
extern int VTextWidth16(
#if NeedFunctionPrototypes
    XFontStruct*	/* font_struct */,
    const XChar2b*	/* string */,
    int			/* count */
#endif
);
#endif /* XTextWidth16 */

#ifdef XTranslateCoordinates
extern Bool VTranslateCoordinates(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Window		/* src_w */,
    Window		/* dest_w */,
    int			/* src_x */,
    int			/* src_y */,
    int*		/* dest_x_return */,
    int*		/* dest_y_return */,
    Window*		/* child_return */
#endif
);
#endif /* XTranslateCoordinates */

#ifdef XUndefineCursor
extern VUndefineCursor(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Window		/* w */
#endif
);
#endif /* XUndefineCursor */

#ifdef XUngrabButton
extern VUngrabButton(
#if NeedFunctionPrototypes
    Display*		/* display */,
    unsigned int	/* button */,
    unsigned int	/* modifiers */,
    Window		/* grab_window */
#endif
);
#endif /* XUngrabButton */

#ifdef XUngrabKey
extern VUngrabKey(
#if NeedFunctionPrototypes
    Display*		/* display */,
    int			/* keycode */,
    unsigned int	/* modifiers */,
    Window		/* grab_window */
#endif
);
#endif /* XUngrabKey */

#ifdef XUngrabKeyboard
extern VUngrabKeyboard(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Time		/* time */
#endif
);
#endif /* XUngrabKeyboard */

#ifdef XUngrabPointer
extern VUngrabPointer(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Time		/* time */
#endif
);
#endif /* XUngrabPointer */

#ifdef XUngrabServer
extern VUngrabServer(
#if NeedFunctionPrototypes
    Display*		/* display */
#endif
);
#endif /* XUngrabServer */

#ifdef XUninstallColormap
extern VUninstallColormap(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Colormap		/* colormap */
#endif
);
#endif /* XUninstallColormap */

#ifdef XUnloadFont
extern VUnloadFont(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Font		/* font */
#endif
);
#endif /* XUnloadFont */

#ifdef XUnmapSubwindows
extern VUnmapSubwindows(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Window		/* w */
#endif
);
#endif /* XUnmapSubwindows */

#ifdef XUnmapWindow
extern VUnmapWindow(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Window		/* w */
#endif
);
#endif /* XUnmapWindow */

#ifdef XVendorRelease
extern int VVendorRelease(
#if NeedFunctionPrototypes
    Display*		/* display */
#endif
);
#endif /* XVendorRelease */

#ifdef XWarpPointer
extern VWarpPointer(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Window		/* src_w */,
    Window		/* dest_w */,
    int			/* src_x */,
    int			/* src_y */,
    unsigned int	/* src_width */,
    unsigned int	/* src_height */,
    int			/* dest_x */,
    int			/* dest_y */	     
#endif
);
#endif /* XWarpPointer */

#ifdef XWidthMMOfScreen
extern int VWidthMMOfScreen(
#if NeedFunctionPrototypes
    Screen*		/* screen */
#endif
);
#endif /* XWidthMMOfScreen */

#ifdef XWidthOfScreen
extern int VWidthOfScreen(
#if NeedFunctionPrototypes
    Screen*		/* screen */
#endif
);
#endif /* XWidthOfScreen */

#ifndef GENERATE_PIXMAPS
#ifdef XWindowEvent
extern VWindowEvent(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Window		/* w */,
    long		/* event_mask */,
    XEvent*		/* event_return */
#endif
);
#endif /* XWindowEvent */
#endif /* GENERATE_PIXMAPS */

#ifdef XWriteBitmapFile
extern int VWriteBitmapFile(
#if NeedFunctionPrototypes
    Display*		/* display */,
    const char*		/* filename */,
    Pixmap		/* bitmap */,
    unsigned int	/* width */,
    unsigned int	/* height */,
    int			/* x_hot */,
    int			/* y_hot */		     
#endif
);
#endif /* XWriteBitmapFile */

/*** added by kieron to allow use of Xlib image functions that do not go
     near the server. Our display structure may be different from the
     Xlib one so any display using functions called by the image handling
     routines in Xlib have to be duplicated in the pv lib.
***/

#ifdef _XGetScanlinePad
extern int _VGetScanlinePad(
#if NeedFunctionPrototypes
    Display*		/* display */,
    int			/* depth */
#endif
);
#endif /* _XGetScanlinePad */

#ifdef _XGetBitsPerPixel
extern int _VGetBitsPerPixel(
#if NeedFunctionPrototypes
    Display*		/* display */,
    int			/* depth */
#endif
);
#endif /* _XGetBitsPerPixel */

/***** Nothing after it is munged, just included. - kieron *****/

/***** added these for completeness, as used internally in our versions of
	XCreateBitmapFromData() and XCreatePixmapFromBitmapData() - when
	cobbling up an convincing XImage structure - or in VGetSubImage().
		kieron
*****/
extern void _XInitImageFuncPtrs(
#if NeedFunctionPrototypes
    XImage*		/* image */
#endif
);

extern void _XSetImage(
#if NeedFunctionPrototypes
	XImage*		/* src_image */,
	XImage*		/* dest_image */,
	int		/* dest_x */,
	int		/* dest_y */
#endif
);

#endif /* _VLIB_H_ */

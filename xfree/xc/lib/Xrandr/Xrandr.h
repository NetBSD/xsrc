/*
 * $XFree86: xc/lib/Xrandr/Xrandr.h,v 1.8 2001/11/23 23:26:38 keithp Exp $
 *
 * Copyright © 2000 Compaq Computer Corporation, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Compaq not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  Compaq makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * COMPAQ DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL COMPAQ
 * BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN 
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author:  Jim Gettys, Compaq Computer Corporation
 */

#ifndef _XRANDR_H_
#define _XRANDR_H_

#include <X11/extensions/randr.h>

#include <X11/Xfuncproto.h>

_XFUNCPROTOBEGIN

typedef struct {
    int	    nvisuals;
    Visual  **visuals;
} XRRVisualGroup;

typedef struct {
    int		    ngroups;
    XRRVisualGroup **groups;
} XRRGroupOfVisualGroup;
    
typedef struct {
    int	width, height;
    int	mwidth, mheight;
    int	group;
} XRRScreenSize;

/*
 *  Events
 */

typedef struct {
    int type;			/* event base */
    unsigned long serial;	/* # of last request processed by server */
    Bool send_event;		/* true if this came from a SendEvent request */
    Display *display;		/* Display the event was read from */
    Window window;		/* window which selected for this event */
    Window root;		/* Root window for changed screen */
    Time timestamp;
    Time config_timestamp;
    SizeID size_index;
    VisualGroupID visual_group_index;
    Rotation rotation;
    int width;
    int height;
    int mwidth;
    int mheight;
} XRRScreenChangeNotifyEvent;

/* internal representation is private to the library */
typedef struct _XRRScreenConfiguration XRRScreenConfiguration;	

Bool XRRQueryExtension (Display *dpy, int *event_basep, int *error_basep);

Status XRRQueryVersion (Display *dpy,
			    int     *major_versionp,
			    int     *minor_versionp);

XRRScreenConfiguration *XRRGetScreenInfo (Display *dpy,
					  Drawable draw);
    
void XRRFreeScreenInfo (XRRScreenConfiguration	*config);

Status XRRSetScreenConfig (Display *dpy, 
			   XRRScreenConfiguration *config,
			   Drawable draw,
			   int size_index,
			   int visual_group_index,
			   Rotation rotation,
			   Time timestamp);

XRRScreenSize *XRRSizes(XRRScreenConfiguration *config, int *nsizes);

void XRRScreenChangeSelectInput (Display *dpy, Window window, Bool enable);

Visual *XRRVisualIDToVisual(Display *dpy, int screen, VisualID id);

int XRRVisualToDepth(Display *dpy, Visual *visual);

Rotation XRRRotations(XRRScreenConfiguration *config, Rotation *current_rotation);

Time XRRTimes (XRRScreenConfiguration *config, Time *config_timestamp);

SizeID XRRCurrentConfig (XRRScreenConfiguration *config, VisualGroupID *visual_group, Rotation *rotation);
    
int XRRRootToScreen(Display *dpy, Window root);

_XFUNCPROTOEND

#endif /* _XRANDR_H_ */

/*
 * $XFree86: xc/lib/Xrandr/Xrandr.c,v 1.6 2001/06/11 01:37:53 keithp Exp $
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
 * Author:  Jim Gettys, Compaq Computer Corporation, Inc.
 */
#include <stdio.h>
#include <X11/Xlib.h>
#include "Xrandrint.h"

XExtensionInfo XRRExtensionInfo;
char XRRExtensionName[] = RANDR_NAME;

static Bool     XRRWireToEvent(Display *dpy, XEvent *event, xEvent *wire);
static Status   XRREventToWire(Display *dpy, XEvent *event, xEvent *wire);

static int
XRRCloseDisplay (Display *dpy, XExtCodes *codes);

static /* const */ XExtensionHooks rr_extension_hooks = {
    NULL,				/* create_gc */
    NULL,				/* copy_gc */
    NULL,				/* flush_gc */
    NULL,				/* free_gc */
    NULL,				/* create_font */
    NULL,				/* free_font */
    XRRCloseDisplay,			/* close_display */
    XRRWireToEvent,			/* wire_to_event */
    XRREventToWire,			/* event_to_wire */
    NULL,				/* error */
    NULL,				/* error_string */
};

static Bool XRRWireToEvent(Display *dpy, XEvent *event, xEvent *wire)
{
    XExtDisplayInfo *info = XRRFindDisplay(dpy);
    XRRScreenChangeNotifyEvent *aevent;
    xRRScreenChangeNotifyEvent *awire;

    RRCheckExtension(dpy, info, False);

    switch ((wire->u.u.type & 0x7F) - info->codes->first_event)
    {
      case RRScreenChangeNotify:
	awire = (xRRScreenChangeNotifyEvent *) wire;
	aevent = (XRRScreenChangeNotifyEvent *) event;
	aevent->type = awire->type & 0x7F;
	aevent->serial = _XSetLastRequestRead(dpy,
					      (xGenericReply *) wire);
	aevent->send_event = (awire->type & 0x80) != 0;
	aevent->display = dpy;
	aevent->window = awire->window;
	aevent->root = awire->root;
	aevent->timestamp = awire->timestamp;
	aevent->config_timestamp = awire->configTimestamp;
	aevent->size_index = awire->sizeID;
	aevent->visual_group_index = awire->visualGroupID;
	aevent->rotation = awire->rotation;
	aevent->width = awire->widthInPixels;
	aevent->height = awire->heightInPixels;
	aevent->mwidth = awire->widthInMillimeters;
	aevent->mheight = awire->heightInMillimeters;
	return True;
    }

    return False;
}

static Status XRREventToWire(Display *dpy, XEvent *event, xEvent *wire)
{
    XExtDisplayInfo *info = XRRFindDisplay(dpy);
    XRRScreenChangeNotifyEvent *aevent;
    xRRScreenChangeNotifyEvent *awire;

    RRCheckExtension(dpy, info, False);

    switch ((event->type & 0x7F) - info->codes->first_event)
    {
      case RRScreenChangeNotify:
	awire = (xRRScreenChangeNotifyEvent *) wire;
	aevent = (XRRScreenChangeNotifyEvent *) event;
	awire->type = aevent->type | (aevent->send_event ? 0x80 : 0);
	awire->rotation = (CARD8) aevent->rotation;
	awire->sequenceNumber = aevent->serial & 0xFFFF;
	awire->timestamp = aevent->timestamp;
	awire->configTimestamp = aevent->config_timestamp;
	awire->root = aevent->root;
	awire->window = aevent->window;
	awire->sizeID = aevent->size_index;
	awire->visualGroupID = aevent->visual_group_index;
	awire->widthInPixels = aevent->width;
	awire->heightInPixels = aevent->height;
	awire->widthInMillimeters = aevent->mwidth;
	awire->heightInMillimeters = aevent->mheight;
	return True;
    }
    return False;
}

XExtDisplayInfo *
XRRFindDisplay (Display *dpy)
{
    XExtDisplayInfo *dpyinfo;

    dpyinfo = XextFindDisplay (&XRRExtensionInfo, dpy);
    if (!dpyinfo)
	dpyinfo = XextAddDisplay (&XRRExtensionInfo, dpy, 
				  XRRExtensionName,
				  &rr_extension_hooks,
				  RRNumberEvents, 0);
    return dpyinfo;
}

static int
XRRCloseDisplay (Display *dpy, XExtCodes *codes)
{
    XExtDisplayInfo *info = XRRFindDisplay (dpy);
    if (info->data) XFree (info->data);
    
    return XextRemoveDisplay (&XRRExtensionInfo, dpy);
}
    
/****************************************************************************
 *                                                                          *
 *			    RandR public interfaces                         *
 *                                                                          *
 ****************************************************************************/
int XRRVisualToDepth(Display *dpy, Visual *visual) 
{
  int s;
  for (s = 0; s < ScreenCount(dpy); s++) {
    Screen *sp = ScreenOfDisplay(dpy, s);
    int d;
    for (d = 0; d < sp->ndepths; d++) {
      int v;
      for (v = 0; v < sp->depths[s].nvisuals; v++) {
	if ( &sp->depths[s].visuals[v] == visual ) return d;
      }
    }
  }
  return -1; /* should not ever happen */
}

Visual *XRRVisualIDToVisual(Display *dpy, int screen, VisualID id)
{
  int d, v;
  Screen *sp = ScreenOfDisplay(dpy, screen);
  for (d = 0; d < sp->ndepths; d++) {
    for (v = 0; v < sp->depths[d].nvisuals; v++) {
      if ( sp->depths[d].visuals[v].visualid == id ) 
	return (&sp->depths[d].visuals[v]);
    }
  }
  return NULL;
}

Rotation XRRRotations(XRRScreenConfiguration *config, Rotation *current_rotation)
{
  *current_rotation = config->current_rotation;
  return config->rotations;
}

XRRScreenSize *XRRSizes(XRRScreenConfiguration *config, int *nsizes)
{
   *nsizes = config->nsizes;
  return config->sizes;
}

Time XRRTimes (XRRScreenConfiguration *config, Time *config_timestamp)
{
    *config_timestamp = config->config_timestamp;
    return config->timestamp;
}

SizeID XRRCurrentConfig (XRRScreenConfiguration *config, VisualGroupID *visual_group, Rotation *rotation)
{
    *visual_group = (VisualGroupID) config->current_visual_group;
    *rotation = (Rotation) config->current_rotation;
    return (SizeID) config->current_size;
}

int XRRRootToScreen(Display *dpy, Window root)
{
  int snum;
  for (snum = 0; snum < ScreenCount(dpy); snum++) {
    if (RootWindow(dpy, snum) == root) return snum;
  }
  return -1;
}


Bool XRRQueryExtension (Display *dpy, int *event_basep, int *error_basep)
{
    XExtDisplayInfo *info = XRRFindDisplay (dpy);

    if (XextHasExtension(info)) {
	*event_basep = info->codes->first_event;
	*error_basep = info->codes->first_error;
	return True;
    } else {
	return False;
    }
}


Status XRRQueryVersion (Display *dpy,
			    int	    *major_versionp,
			    int	    *minor_versionp)
{
    XExtDisplayInfo *info = XRRFindDisplay (dpy);
    xRRQueryVersionReply rep;
    xRRQueryVersionReq  *req;

    RRCheckExtension (dpy, info, 0);

    LockDisplay (dpy);
    GetReq (RRQueryVersion, req);
    req->reqType = info->codes->major_opcode;
    req->randrReqType = X_RRQueryVersion;
    req->majorVersion = RANDR_MAJOR;
    req->minorVersion = RANDR_MINOR;
    if (!_XReply (dpy, (xReply *) &rep, 0, xTrue)) {
	UnlockDisplay (dpy);
	SyncHandle ();
	return 0;
    }
    *major_versionp = rep.majorVersion;
    *minor_versionp = rep.minorVersion;
    UnlockDisplay (dpy);
    SyncHandle ();
    return 1;
}

XRRScreenConfiguration *XRRGetScreenInfo (Display *dpy, Window window)
{
    XExtDisplayInfo *info = XRRFindDisplay(dpy);
    xRRGetScreenInfoReply   rep;
    xRRGetScreenInfoReq	    *req;
    int			    nbytes, rbytes;
    int			    i, j;
    int			    nvisuals, ngroups;
    int			    snum;
    xScreenSizes	    *psize;
    char		    *data;
    struct _XRRScreenConfiguration  *scp;
    XRRVisualGroup	    *vgp;
    XRRGroupOfVisualGroup   *gvgp;
    XRRVisualGroup	    **gp;
    XRRScreenSize	    *ssp;
    Visual		    **vgpp;
    CARD32		    *data32;
    CARD16		    *data16;

    RRCheckExtension (dpy, info, 0);

    LockDisplay (dpy);
    GetReq (RRGetScreenInfo, req);
    req->reqType = info->codes->major_opcode;
    req->randrReqType = X_RRGetScreenInfo;
    req->window = window;
    if (!_XReply (dpy, (xReply *) &rep, 0, xFalse))
    {
	UnlockDisplay (dpy);
	SyncHandle ();
	return NULL;
    }
    nbytes = (long) rep.length << 2;
    data = (char *) Xmalloc ((unsigned) nbytes);
    if (!data)
    {
	_XEatData (dpy, (unsigned long) nbytes);
	UnlockDisplay (dpy);
	SyncHandle ();
	return NULL;
    }
    _XReadPad (dpy, data, nbytes);
    UnlockDisplay (dpy);
    SyncHandle ();

    /* 
     * first we must compute how much space to allocate for 
     * randr library's use; we'll allocate the structures in a single
     * allocation, on cleanlyness grounds.
     */

    /* pick up in the protocol buffer after the protocol size information */
    psize = (xScreenSizes *) data;
    data32 = (CARD32 *) &psize[rep.nSizes];
    vgp = (XRRVisualGroup *) ssp;	/* visual groups after size structures */
    /* and groups of visual groups structures after the array of visual groups */
    gvgp = (XRRGroupOfVisualGroup *) &vgp[rep.nVisualGroups];

    /*
     * first we count up the number of groups
     */
    nvisuals = 0;
    for (i = 0; i < rep.nVisualGroups; i++) {
      j = *data32;
      data32 += j + 1;
      nvisuals += j;
    }

    /*
     * Next comes the groups of visual groups
     */

    data16 = (CARD16 *) data32;
    /*
     * We count up the number of groups
     */
    ngroups = 0;
    for (i = 0; i < rep.nGroupsOfVisualGroups; i++) {
      j = *data16;
      data16 += j + 1;
      ngroups += j;
    }

    rbytes = sizeof (XRRScreenConfiguration) +
      (rep.nVisualGroups * sizeof (XRRVisualGroup)) +
      (rep.nGroupsOfVisualGroups * sizeof (XRRGroupOfVisualGroup)) +
      (rep.nSizes * sizeof (XRRScreenSize)) +
      (nvisuals * sizeof (Visual *)) +
      (ngroups * sizeof (XRRGroupOfVisualGroup *));

    scp = (struct _XRRScreenConfiguration *) Xmalloc(rbytes);
    if (scp == NULL) return NULL;


    ssp = (XRRScreenSize *)(scp + 1);
    vgp = (XRRVisualGroup *) (&ssp[rep.nSizes]);
    gvgp = (XRRGroupOfVisualGroup *) (&vgp[rep.nVisualGroups]);
    vgpp = (Visual **) (&gvgp[rep.nGroupsOfVisualGroups]);
    gp = (XRRVisualGroup **) (&vgpp[ngroups]);
    /* set up the screen configuration structure */
    scp->screen = ScreenOfDisplay (dpy, (snum = XRRRootToScreen(dpy, rep.root)));

    scp->visual_group = vgp;
    scp->groups_of_visual_groups = gvgp;
    scp->sizes = ssp;
    scp->rotations = rep.setOfRotations;
    scp->current_size = rep.sizeID;
    scp->current_visual_group = rep.visualGroupID;
    scp->current_rotation = rep.rotation;
    scp->timestamp = rep.timestamp;
    scp->config_timestamp = rep.configTimestamp;
    scp->nsizes = rep.nSizes;

    /*
     * Time to unpack the data from the server.
     */

    /*
     * First comes the size information
     */
    psize = (xScreenSizes *) data;
    for (i = 0; i < rep.nSizes; i++)  {
        ssp[i].width = psize[i].widthInPixels;
	ssp[i].height = psize[i].heightInPixels;
	ssp[i].mwidth = psize[i].widthInMillimeters;
	ssp[i].mheight = psize[i].heightInMillimeters;
	ssp[i].group = psize[i].visualGroup;
    }
    /*
     * Next comes the visual groups
     */

    data32 = (CARD32 *) &psize[i];

    for (i = 0; i < rep.nVisualGroups; i++) {
      vgp[i].visuals = vgpp;
      vgp[i].nvisuals = (int) *data32++;
      for (j = 0; j < vgp->nvisuals; j++) {
	*vgpp = XRRVisualIDToVisual(dpy, snum, (VisualID) *data32++);
	vgpp += 1;
      }
    }


    data16 = (CARD16 *) data32;

    for (i = 0; i < rep.nGroupsOfVisualGroups; i++) {
      gvgp[i].groups = gp;
      gvgp[i].ngroups = (int) *data16++;
      for (j = 0; j < gvgp[i].ngroups; j++) {
  	  gvgp[i].groups[j] = &vgp[*data16++];
	  gp += 1;
      }
    }

    return (XRRScreenConfiguration *)(scp);
}
    
void XRRFreeScreenInfo (XRRScreenConfiguration *config)
{
    Xfree (config);
}

void XRRScreenChangeSelectInput (Display *dpy, Window window, Bool enable)
{
    XExtDisplayInfo *info = XRRFindDisplay (dpy);
    xRRScreenChangeSelectInputReq  *req;

    RRSimpleCheckExtension (dpy, info);

    LockDisplay (dpy);
    GetReq (RRScreenChangeSelectInput, req);
    req->reqType = info->codes->major_opcode;
    req->randrReqType = X_RRScreenChangeSelectInput;
    req->window = window;
    req->enable = xFalse;
    if (enable) req->enable = xTrue;
    UnlockDisplay (dpy);
    SyncHandle ();
    return;
}

Status XRRSetScreenConfig (Display *dpy,
			   XRRScreenConfiguration *config,
			   Drawable draw,
			   int size_index,
			   int visual_group_index,
			   Rotation rotation, Time timestamp)
{
    XExtDisplayInfo *info = XRRFindDisplay (dpy);
    xRRSetScreenConfigReply rep;
    xRRSetScreenConfigReq  *req;

    RRCheckExtension (dpy, info, 0);

    LockDisplay (dpy);
    GetReq (RRSetScreenConfig, req);
    req->reqType = info->codes->major_opcode;
    req->randrReqType = X_RRSetScreenConfig;
    req->drawable = draw;
    req->sizeID = size_index;
    req->visualGroupID = visual_group_index;
    req->rotation = rotation;
    req->timestamp = timestamp;
    req->configTimestamp = config->config_timestamp;
    (void) _XReply (dpy, (xReply *) &rep, 0, xTrue);

    if (rep.status == RRSetConfigSuccess) {
      /* if we succeed, set our view of reality to what we set it to */
      config->config_timestamp = rep.newConfigTimestamp;
      config->timestamp = rep.newTimestamp;
      config->screen = ScreenOfDisplay (dpy, XRRRootToScreen(dpy, rep.root));
      config->current_size = size_index;
      config->current_rotation = rotation;
      config->current_visual_group = visual_group_index;
    }
    UnlockDisplay (dpy);
    SyncHandle ();
    return(rep.status);
}

/* $XConsortium: XF86VMode.c /main/4 1995/12/05 17:51:46 kaleb $ */
/* $XFree86: xc/lib/Xext/XF86VMode.c,v 1.1.1.1 1996/01/03 07:00:06 dawes Exp $ */
/*

Copyright (c) 1995  Kaleb S. KEITHLEY

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL Kaleb S. KEITHLEY BE LIABLE FOR ANY CLAIM, DAMAGES 
OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of the Kaleb S. KEITHLEY 
shall not be used in advertising or otherwise to promote the sale, use 
or other dealings in this Software without prior written authorization
from the Kaleb S. KEITHLEY.

*/
/* THIS IS NOT AN X CONSORTIUM STANDARD */

#define NEED_EVENTS
#define NEED_REPLIES
#include "Xlibint.h"
#include "xf86vmstr.h"
#include "Xext.h"
#include "extutil.h"

static XExtensionInfo _vgahelp_info_data;
static XExtensionInfo *vgahelp_info = &_vgahelp_info_data;
static char *vgahelp_extension_name = XF86VIDMODENAME;

#define XF86VidModeCheckExtension(dpy,i,val) \
  XextCheckExtension (dpy, i, vgahelp_extension_name, val)

/*****************************************************************************
 *                                                                           *
 *			   private utility routines                          *
 *                                                                           *
 *****************************************************************************/

static int close_display();
static /* const */ XExtensionHooks vgahelp_extension_hooks = {
    NULL,				/* create_gc */
    NULL,				/* copy_gc */
    NULL,				/* flush_gc */
    NULL,				/* free_gc */
    NULL,				/* create_font */
    NULL,				/* free_font */
    close_display,			/* close_display */
    NULL,				/* wire_to_event */
    NULL,				/* event_to_wire */
    NULL,				/* error */
    NULL,				/* error_string */
};

static XEXT_GENERATE_FIND_DISPLAY (find_display, vgahelp_info, 
				   vgahelp_extension_name, 
				   &vgahelp_extension_hooks, 
				   0, NULL)

static XEXT_GENERATE_CLOSE_DISPLAY (close_display, vgahelp_info)


/*****************************************************************************
 *                                                                           *
 *		    public XFree86-VidMode Extension routines                *
 *                                                                           *
 *****************************************************************************/

Bool XF86VidModeQueryExtension (dpy, event_basep, error_basep)
    Display *dpy;
    int *event_basep, *error_basep;
{
    XExtDisplayInfo *info = find_display (dpy);

    if (XextHasExtension(info)) {
	*event_basep = info->codes->first_event;
	*error_basep = info->codes->first_error;
	return True;
    } else {
	return False;
    }
}

Bool XF86VidModeQueryVersion(dpy, majorVersion, minorVersion)
    Display* dpy;
    int* majorVersion; 
    int* minorVersion;
{
    XExtDisplayInfo *info = find_display (dpy);
    xVGAHelpQueryVersionReply rep;
    xVGAHelpQueryVersionReq *req;

    XF86VidModeCheckExtension (dpy, info, False);

    LockDisplay(dpy);
    GetReq(VGAHelpQueryVersion, req);
    req->reqType = info->codes->major_opcode;
    req->vgahelpReqType = X_VGAHelpQueryVersion;
    if (!_XReply(dpy, (xReply *)&rep, 0, xFalse)) {
	UnlockDisplay(dpy);
	SyncHandle();
	return False;
    }
    *majorVersion = rep.majorVersion;
    *minorVersion = rep.minorVersion;
    UnlockDisplay(dpy);
    SyncHandle();
    return True;
}

Bool XF86VidModeGetModeLine(dpy, screen, dotclock, modeline)
    Display* dpy;
    int screen;
    int* dotclock; 
    XF86VidModeModeLine* modeline;
{
    XExtDisplayInfo *info = find_display (dpy);
    xVGAHelpGetModeLineReply rep;
    xVGAHelpGetModeLineReq *req;

    XF86VidModeCheckExtension (dpy, info, False);

    LockDisplay(dpy);
    GetReq(VGAHelpGetModeLine, req);
    req->reqType = info->codes->major_opcode;
    req->vgahelpReqType = X_VGAHelpGetModeLine;
    req->screen = screen;
    if (!_XReply(dpy, (xReply *)&rep, 
        (SIZEOF(xVGAHelpGetModeLineReply) - SIZEOF(xReply)) >> 2, xFalse)) {
	UnlockDisplay(dpy);
	SyncHandle();
	return False;
    }
    *dotclock = rep.dotclock;
    modeline->hdisplay   = rep.hdisplay;
    modeline->hsyncstart = rep.hsyncstart;
    modeline->hsyncend   = rep.hsyncend;
    modeline->htotal     = rep.htotal;
    modeline->vdisplay   = rep.vdisplay;
    modeline->vsyncstart = rep.vsyncstart;
    modeline->vsyncend   = rep.vsyncend;
    modeline->vtotal     = rep.vtotal;
    modeline->flags      = rep.flags;
    modeline->privsize   = rep.privsize;
    if (rep.privsize > 0) {
	if (!(modeline->private = Xcalloc(rep.privsize, sizeof(INT32)))) {
	    _XEatData(dpy, (rep.privsize) * sizeof(INT32));
	    Xfree(modeline->private);
	    return False;
	}
	_XRead32(dpy, modeline->private, rep.privsize * sizeof(INT32));
    } else {
	modeline->private = NULL;
    }
    UnlockDisplay(dpy);
    SyncHandle();
    return True;
}

Bool XF86VidModeModModeLine (dpy, screen, modeline)
    Display *dpy;
    int screen;
    XF86VidModeModeLine* modeline;
{
    XExtDisplayInfo *info = find_display (dpy);
    xVGAHelpModModeLineReq *req;

    XF86VidModeCheckExtension (dpy, info, 0);

    LockDisplay(dpy);
    GetReq(VGAHelpModModeLine, req);
    req->reqType = info->codes->major_opcode;
    req->vgahelpReqType = X_VGAHelpModModeLine;
    req->screen = screen;
    req->hdisplay = modeline->hdisplay;
    req->hsyncstart = modeline->hsyncstart;
    req->hsyncend = modeline->hsyncend;
    req->htotal = modeline->htotal;
    req->vdisplay = modeline->vdisplay;
    req->vsyncstart = modeline->vsyncstart;
    req->vsyncend = modeline->vsyncend;
    req->vtotal = modeline->vtotal;
    req->flags = modeline->flags;
    req->privsize = modeline->privsize;
    if (modeline->privsize) {
	req->length += modeline->privsize;
	Data32(dpy, (long *) modeline->private,
	       modeline->privsize * sizeof(INT32));
    }
    UnlockDisplay(dpy);
    SyncHandle();
    return True;
}

Bool XF86VidModeSwitchMode(dpy, screen, zoom)
    Display* dpy;
    int screen;
    int zoom;
{
    XExtDisplayInfo *info = find_display (dpy);
    xVGAHelpSwitchModeReq *req;

    XF86VidModeCheckExtension (dpy, info, False);

    LockDisplay(dpy);
    GetReq(VGAHelpSwitchMode, req);
    req->reqType = info->codes->major_opcode;
    req->vgahelpReqType = X_VGAHelpSwitchMode;
    req->screen = screen;
    req->zoom = zoom;
    UnlockDisplay(dpy);
    SyncHandle();
    return True;
}
    
Bool XF86VidModeLockModeSwitch(dpy, screen, lock)
    Display* dpy;
    int screen;
    int lock;
{
    XExtDisplayInfo *info = find_display (dpy);
    xXF86VidModeLockModeSwitchReq *req;

    XF86VidModeCheckExtension (dpy, info, False);

    LockDisplay(dpy);
    GetReq(XF86VidModeLockModeSwitch, req);
    req->reqType = info->codes->major_opcode;
    req->vgahelpReqType = X_XF86VidModeLockModeSwitch;
    req->screen = screen;
    req->lock = lock;
    UnlockDisplay(dpy);
    SyncHandle();
    return True;
}
    
Bool XF86VidModeSetSaver(dpy, screen, suspendTime, offTime)
    Display* dpy;
    int screen;
    int suspendTime, offTime;
{
    XExtDisplayInfo *info = find_display (dpy);
    xXF86VidModeSetSaverReq *req;

    XF86VidModeCheckExtension (dpy, info, False);

    LockDisplay(dpy);
    GetReq(XF86VidModeSetSaver, req);
    req->reqType = info->codes->major_opcode;
    req->vgahelpReqType = X_XF86VidModeSetSaver;
    req->screen = screen;
    req->suspendTime = suspendTime;
    req->offTime = offTime;
    UnlockDisplay(dpy);
    SyncHandle();
    return True;
}
    
Bool XF86VidModeGetSaver(dpy, screen, suspendTime, offTime)
    Display* dpy;
    int screen;
    int *suspendTime, *offTime;
{
    XExtDisplayInfo *info = find_display (dpy);
    xXF86VidModeGetSaverReply rep;
    xXF86VidModeGetSaverReq *req;
    int i;

    XF86VidModeCheckExtension (dpy, info, False);

    LockDisplay(dpy);
    GetReq(XF86VidModeGetSaver, req);
    req->reqType = info->codes->major_opcode;
    req->vgahelpReqType = X_XF86VidModeGetSaver;
    req->screen = screen;
    if (!_XReply(dpy, (xReply *)&rep, 0, xFalse)) {
	UnlockDisplay(dpy);
	SyncHandle();
	return False;
    }

    *suspendTime = rep.suspendTime;
    *offTime = rep.offTime;
	
    UnlockDisplay(dpy);
    SyncHandle();
    return True;
}

Bool XF86VidModeGetMonitor(dpy, screen, monitor)
    Display* dpy;
    int screen;
    XF86VidModeMonitor* monitor;
{
    XExtDisplayInfo *info = find_display (dpy);
    xVGAHelpGetMonitorReply rep;
    xVGAHelpGetMonitorReq *req;
    CARD32 syncrange;
    int i;

    XF86VidModeCheckExtension (dpy, info, False);

    LockDisplay(dpy);
    GetReq(VGAHelpGetMonitor, req);
    req->reqType = info->codes->major_opcode;
    req->vgahelpReqType = X_VGAHelpGetMonitor;
    req->screen = screen;
    if (!_XReply(dpy, (xReply *)&rep, 0, xFalse)) {
	UnlockDisplay(dpy);
	SyncHandle();
	return False;
    }
    monitor->nhsync = rep.nhsync;
    monitor->nvsync = rep.nvsync;
    monitor->bandwidth = (float)rep.bandwidth / 1e6;
    if (rep.vendorLength) {
	if (!(monitor->vendor = (char *)Xcalloc(rep.vendorLength + 1, 1))) {
	    _XEatData(dpy, (rep.nhsync + rep.nvsync) * 4 +
		      (rep.vendorLength + 3 & 3) + (rep.modelLength + 3 & 3));
	    return False;
	}
    } else {
	monitor->vendor = NULL;
    }
    if (rep.modelLength) {
	if (!(monitor->model = Xcalloc(rep.modelLength + 1, 1))) {
	    _XEatData(dpy, (rep.nhsync + rep.nvsync) * 4 +
		      (rep.vendorLength + 3 & 3) + (rep.modelLength + 3 & 3));
	    if (monitor->vendor)
		Xfree(monitor->vendor);
	    return False;
	}
    } else {
	monitor->model = NULL;
    }
    if (!(monitor->hsync = Xcalloc(rep.nhsync, sizeof(XF86VidModeSyncRange)))) {
	_XEatData(dpy, (rep.nhsync + rep.nvsync) * 4 +
		  (rep.vendorLength + 3 & 3) + (rep.modelLength + 3 & 3));
	
	if (monitor->vendor)
	    Xfree(monitor->vendor);
	if (monitor->model)
	    Xfree(monitor->model);
	return False;
    }
    if (!(monitor->vsync = Xcalloc(rep.nvsync, sizeof(XF86VidModeSyncRange)))) {
	_XEatData(dpy, (rep.nhsync + rep.nvsync) * 4 +
		  (rep.vendorLength + 3 & 3) + (rep.modelLength + 3 & 3));
	if (monitor->vendor)
	    Xfree(monitor->vendor);
	if (monitor->model)
	    Xfree(monitor->model);
	Xfree(monitor->hsync);
	return False;
    }
    for (i = 0; i < rep.nhsync; i++) {
	_XRead32(dpy, (long *)&syncrange, 4);
	monitor->hsync[i].lo = (float)(syncrange & 0xFFFF) / 100.0;
	monitor->hsync[i].hi = (float)(syncrange >> 16) / 100.0;
    }
    for (i = 0; i < rep.nvsync; i++) {
	_XRead32(dpy, (long *)&syncrange, 4);
	monitor->vsync[i].lo = (float)(syncrange & 0xFFFF) / 100.0;
	monitor->vsync[i].hi = (float)(syncrange >> 16) / 100.0;
    }
    if (rep.vendorLength)
	_XReadPad(dpy, monitor->vendor, rep.vendorLength);
    else
	monitor->vendor = "";
    if (rep.modelLength)
	_XReadPad(dpy, monitor->model, rep.modelLength);
    else
	monitor->model = "";
	
    UnlockDisplay(dpy);
    SyncHandle();
    return True;
}

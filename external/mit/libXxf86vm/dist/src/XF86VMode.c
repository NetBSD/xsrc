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

Except as contained in this notice, the name of Kaleb S. KEITHLEY
shall not be used in advertising or otherwise to promote the sale, use
or other dealings in this Software without prior written authorization
from Kaleb S. KEITHLEY.

*/

/* THIS IS NOT AN X CONSORTIUM STANDARD */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <X11/Xlibint.h>
#include <X11/extensions/xf86vmproto.h>
#include <X11/extensions/xf86vmode.h>
#include <X11/extensions/Xext.h>
#include <X11/extensions/extutil.h>
#include <limits.h>

#ifdef DEBUG
#include <stdio.h>
#endif

#ifndef MODE_BAD
#define MODE_BAD 255
#endif

static XExtensionInfo _xf86vidmode_info_data;
static XExtensionInfo *xf86vidmode_info = &_xf86vidmode_info_data;
static const char *xf86vidmode_extension_name = XF86VIDMODENAME;

#define XF86VidModeCheckExtension(dpy,i,val) \
  XextCheckExtension (dpy, i, xf86vidmode_extension_name, val)

/*****************************************************************************
 *                                                                           *
 *			   private utility routines                          *
 *                                                                           *
 *****************************************************************************/

static XEXT_CLOSE_DISPLAY_PROTO(close_display);
static /* const */ XExtensionHooks xf86vidmode_extension_hooks = {
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

static XEXT_GENERATE_FIND_DISPLAY (find_display, xf86vidmode_info,
				   xf86vidmode_extension_name,
				   &xf86vidmode_extension_hooks,
				   0, NULL)

static XEXT_GENERATE_CLOSE_DISPLAY (close_display, xf86vidmode_info)


/*****************************************************************************
 *                                                                           *
 *		    public XFree86-VidMode Extension routines                *
 *                                                                           *
 *****************************************************************************/

Bool
XF86VidModeQueryExtension(Display *dpy, int *event_basep, int *error_basep)
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

Bool
XF86VidModeQueryVersion(Display* dpy, int* majorVersion, int* minorVersion)
{
    XExtDisplayInfo *info = find_display (dpy);
    xXF86VidModeQueryVersionReply rep;
    xXF86VidModeQueryVersionReq *req;

    XF86VidModeCheckExtension (dpy, info, False);

    LockDisplay(dpy);
    GetReq(XF86VidModeQueryVersion, req);
    req->reqType = info->codes->major_opcode;
    req->xf86vidmodeReqType = X_XF86VidModeQueryVersion;
    if (!_XReply(dpy, (xReply *)&rep, 0, xFalse)) {
	UnlockDisplay(dpy);
	SyncHandle();
	return False;
    }
    *majorVersion = rep.majorVersion;
    *minorVersion = rep.minorVersion;
    UnlockDisplay(dpy);
    SyncHandle();
    if (*majorVersion >= 2)
	XF86VidModeSetClientVersion(dpy);
    return True;
}

Bool
XF86VidModeSetClientVersion(Display *dpy)
{
    XExtDisplayInfo *info = find_display(dpy);
    xXF86VidModeSetClientVersionReq *req;

    XF86VidModeCheckExtension(dpy, info, False);

    LockDisplay(dpy);
    GetReq(XF86VidModeSetClientVersion, req);
    req->reqType = info->codes->major_opcode;
    req->xf86vidmodeReqType = X_XF86VidModeSetClientVersion;
    req->major = XF86VIDMODE_MAJOR_VERSION;
    req->minor = XF86VIDMODE_MINOR_VERSION;
    UnlockDisplay(dpy);
    SyncHandle();
    return True;
}

Bool
XF86VidModeSetGamma(Display *dpy, int screen, XF86VidModeGamma *Gamma)
{
    XExtDisplayInfo *info = find_display(dpy);
    xXF86VidModeSetGammaReq *req;

    XF86VidModeCheckExtension(dpy, info, False);

    LockDisplay(dpy);
    GetReq(XF86VidModeSetGamma, req);
    req->reqType = info->codes->major_opcode;
    req->xf86vidmodeReqType = X_XF86VidModeSetGamma;
    req->screen = screen;
    req->red = (CARD32)(Gamma->red * 10000.);
    req->green = (CARD32)(Gamma->green * 10000.);
    req->blue = (CARD32)(Gamma->blue * 10000.);
    UnlockDisplay(dpy);
    SyncHandle();
    return True;
}

Bool
XF86VidModeGetGamma(Display *dpy, int screen, XF86VidModeGamma *Gamma)
{
    XExtDisplayInfo *info = find_display (dpy);
    xXF86VidModeGetGammaReply rep;
    xXF86VidModeGetGammaReq *req;

    XF86VidModeCheckExtension (dpy, info, False);

    LockDisplay(dpy);
    GetReq(XF86VidModeGetGamma, req);
    req->reqType = info->codes->major_opcode;
    req->xf86vidmodeReqType = X_XF86VidModeGetGamma;
    req->screen = screen;
    if (!_XReply(dpy, (xReply *)&rep, 0, xFalse)) {
	UnlockDisplay(dpy);
	SyncHandle();
	return False;
    }
    Gamma->red = ((float)rep.red) / 10000.;
    Gamma->green = ((float)rep.green) / 10000.;
    Gamma->blue = ((float)rep.blue) / 10000.;
    UnlockDisplay(dpy);
    SyncHandle();
    return True;
}

Bool
XF86VidModeGetModeLine(Display* dpy, int screen, int* dotclock,
		       XF86VidModeModeLine* modeline)
{
    XExtDisplayInfo *info = find_display (dpy);
    xXF86VidModeGetModeLineReq *req;
    int majorVersion, minorVersion;
    CARD32 remaining_len;
    Bool result = True;

    XF86VidModeCheckExtension (dpy, info, False);
    XF86VidModeQueryVersion(dpy, &majorVersion, &minorVersion);

    LockDisplay(dpy);
    GetReq(XF86VidModeGetModeLine, req);
    req->reqType = info->codes->major_opcode;
    req->xf86vidmodeReqType = X_XF86VidModeGetModeLine;
    req->screen = screen;

    if (_X_UNLIKELY(majorVersion < 2)) {
	xXF86OldVidModeGetModeLineReply oldrep;

	if (!_XReply(dpy, (xReply *)&oldrep,
            (SIZEOF(xXF86OldVidModeGetModeLineReply) - SIZEOF(xReply)) >> 2, xFalse)) {
	    UnlockDisplay(dpy);
	    SyncHandle();
	    return False;
	}
	remaining_len = oldrep.length -
	    ((SIZEOF(xXF86OldVidModeGetModeLineReply) - SIZEOF(xReply)) >> 2);
	*dotclock = oldrep.dotclock;
	modeline->hdisplay   = oldrep.hdisplay;
	modeline->hsyncstart = oldrep.hsyncstart;
	modeline->hsyncend   = oldrep.hsyncend;
	modeline->htotal     = oldrep.htotal;
	modeline->hskew      = 0;
	modeline->vdisplay   = oldrep.vdisplay;
	modeline->vsyncstart = oldrep.vsyncstart;
	modeline->vsyncend   = oldrep.vsyncend;
	modeline->vtotal     = oldrep.vtotal;
	modeline->flags      = oldrep.flags;
	modeline->privsize   = oldrep.privsize;
    } else {
	xXF86VidModeGetModeLineReply rep;

	if (!_XReply(dpy, (xReply *)&rep,
            (SIZEOF(xXF86VidModeGetModeLineReply) - SIZEOF(xReply)) >> 2, xFalse)) {
	    UnlockDisplay(dpy);
	    SyncHandle();
	    return False;
	}
	remaining_len = rep.length -
	    ((SIZEOF(xXF86VidModeGetModeLineReply) - SIZEOF(xReply)) >> 2);
	*dotclock = rep.dotclock;
	modeline->hdisplay   = rep.hdisplay;
	modeline->hsyncstart = rep.hsyncstart;
	modeline->hsyncend   = rep.hsyncend;
	modeline->htotal     = rep.htotal;
	modeline->hskew      = rep.hskew;
	modeline->vdisplay   = rep.vdisplay;
	modeline->vsyncstart = rep.vsyncstart;
	modeline->vsyncend   = rep.vsyncend;
	modeline->vtotal     = rep.vtotal;
	modeline->flags      = rep.flags;
	modeline->privsize   = rep.privsize;
    }

    if (modeline->privsize > 0) {
	if ((unsigned) modeline->privsize < (INT_MAX / sizeof(INT32)))
	    modeline->private = Xcalloc(modeline->privsize, sizeof(INT32));
	else
	    modeline->private = NULL;
	if (modeline->private == NULL) {
	    _XEatDataWords(dpy, remaining_len);
	    result = False;
	} else
	    _XRead(dpy, (char*)modeline->private, modeline->privsize * sizeof(INT32));
    } else {
	modeline->private = NULL;
    }
    UnlockDisplay(dpy);
    SyncHandle();
    return result;
}

Bool
XF86VidModeGetAllModeLines(Display* dpy, int screen, int* modecount,
			   XF86VidModeModeInfo ***modelinesPtr)
{
    XExtDisplayInfo *info = find_display (dpy);
    xXF86VidModeGetAllModeLinesReply rep;
    xXF86VidModeGetAllModeLinesReq *req;
    XF86VidModeModeInfo *mdinfptr, **modelines;
    xXF86VidModeModeInfo xmdline;
    xXF86OldVidModeModeInfo oldxmdline;
    unsigned int i;
    int majorVersion, minorVersion;
    Bool protocolBug = False;

    XF86VidModeCheckExtension (dpy, info, False);

    /*
     * Note: There was a bug in the protocol implementation in versions
     * 0.x with x < 8 (the .private field wasn't being passed over the wire).
     * Check the server's version, and accept the old format if appropriate.
     */

    XF86VidModeQueryVersion(dpy, &majorVersion, &minorVersion);
    if (majorVersion == 0 && minorVersion < 8) {
	protocolBug = True;
#ifdef DEBUG
	fprintf(stderr, "XF86VidModeGetAllModeLines: Warning: Xserver is"
		"running an old version (%d.%d)\n", majorVersion,
		minorVersion);
#endif
    }

    LockDisplay(dpy);
    GetReq(XF86VidModeGetAllModeLines, req);
    req->reqType = info->codes->major_opcode;
    req->xf86vidmodeReqType = X_XF86VidModeGetAllModeLines;
    req->screen = screen;
    if (!_XReply(dpy, (xReply *)&rep,
        (SIZEOF(xXF86VidModeGetAllModeLinesReply) - SIZEOF(xReply)) >> 2, xFalse)) {
        UnlockDisplay(dpy);
	SyncHandle();
	return False;
    }

    *modecount = rep.modecount;

    if (!(modelines = (XF86VidModeModeInfo **) Xcalloc(rep.modecount,
                                          sizeof(XF86VidModeModeInfo *)
                                          +sizeof(XF86VidModeModeInfo)))) {
	_XEatDataWords(dpy, rep.length -
	    ((SIZEOF(xXF86VidModeGetAllModeLinesReply) - SIZEOF(xReply)) >> 2));
	UnlockDisplay(dpy);
	SyncHandle();
        return False;
    }
    mdinfptr = (XF86VidModeModeInfo *) (
			    (char *) modelines
			    + rep.modecount*sizeof(XF86VidModeModeInfo *)
		    );

    for (i = 0; i < rep.modecount; i++) {
        modelines[i] = mdinfptr++;
	if (_X_UNLIKELY(majorVersion < 2)) {
            _XRead(dpy, (char*)&oldxmdline, sizeof(xXF86OldVidModeModeInfo));
	    modelines[i]->dotclock   = oldxmdline.dotclock;
	    modelines[i]->hdisplay   = oldxmdline.hdisplay;
	    modelines[i]->hsyncstart = oldxmdline.hsyncstart;
	    modelines[i]->hsyncend   = oldxmdline.hsyncend;
	    modelines[i]->htotal     = oldxmdline.htotal;
	    modelines[i]->hskew      = 0;
	    modelines[i]->vdisplay   = oldxmdline.vdisplay;
	    modelines[i]->vsyncstart = oldxmdline.vsyncstart;
	    modelines[i]->vsyncend   = oldxmdline.vsyncend;
	    modelines[i]->vtotal     = oldxmdline.vtotal;
	    modelines[i]->flags      = oldxmdline.flags;
	    if (protocolBug) {
		modelines[i]->privsize = 0;
		modelines[i]->private = NULL;
	    } else {
		modelines[i]->privsize   = oldxmdline.privsize;
		if (oldxmdline.privsize > 0) {
	            if (!(modelines[i]->private =
			    Xcalloc(oldxmdline.privsize, sizeof(INT32)))) {
			_XEatDataWords(dpy, oldxmdline.privsize);
		    } else {
			_XRead(dpy, (char*)modelines[i]->private,
			     oldxmdline.privsize * sizeof(INT32));
		    }
		} else {
		  modelines[i]->private = NULL;
		}
	    }
	} else {
            _XRead(dpy, (char*)&xmdline, sizeof(xXF86VidModeModeInfo));
	    modelines[i]->dotclock   = xmdline.dotclock;
	    modelines[i]->hdisplay   = xmdline.hdisplay;
	    modelines[i]->hsyncstart = xmdline.hsyncstart;
	    modelines[i]->hsyncend   = xmdline.hsyncend;
	    modelines[i]->htotal     = xmdline.htotal;
	    modelines[i]->hskew      = xmdline.hskew;
	    modelines[i]->vdisplay   = xmdline.vdisplay;
	    modelines[i]->vsyncstart = xmdline.vsyncstart;
	    modelines[i]->vsyncend   = xmdline.vsyncend;
	    modelines[i]->vtotal     = xmdline.vtotal;
	    modelines[i]->flags      = xmdline.flags;
	    if (protocolBug) {
		modelines[i]->privsize = 0;
		modelines[i]->private = NULL;
	    } else {
		modelines[i]->privsize   = xmdline.privsize;
		if (xmdline.privsize > 0) {
		    if (!(modelines[i]->private =
			    Xcalloc(xmdline.privsize, sizeof(INT32)))) {
			_XEatDataWords(dpy, xmdline.privsize);
		    } else {
			_XRead(dpy, (char*)modelines[i]->private,
			     xmdline.privsize * sizeof(INT32));
		    }
		} else {
		    modelines[i]->private = NULL;
		}
	    }
	}
    }
    *modelinesPtr = modelines;
    UnlockDisplay(dpy);
    SyncHandle();
    return True;
}

/*
 * GetReq replacement for use with VidMode protocols earlier than 2.0
 */
#define GetOldReq(name, oldname, req) \
	req = (x##oldname##Req *) \
	    _XGetRequest(dpy, X_##name, SIZEOF(x##oldname##Req))

Bool
XF86VidModeAddModeLine(Display *dpy, int screen,
		       XF86VidModeModeInfo* newmodeline,
		       XF86VidModeModeInfo* aftermodeline)
{
    XExtDisplayInfo *info = find_display (dpy);
    int majorVersion, minorVersion;

    XF86VidModeCheckExtension (dpy, info, False);
    XF86VidModeQueryVersion(dpy, &majorVersion, &minorVersion);

    LockDisplay(dpy);
    if (_X_UNLIKELY(majorVersion < 2)) {
	xXF86OldVidModeAddModeLineReq *oldreq;

	GetOldReq(XF86VidModeAddModeLine, XF86OldVidModeAddModeLine, oldreq);
	oldreq->reqType = info->codes->major_opcode;
	oldreq->xf86vidmodeReqType = X_XF86VidModeAddModeLine;
	oldreq->screen = screen;
	oldreq->dotclock =	newmodeline->dotclock;
	oldreq->hdisplay =	newmodeline->hdisplay;
	oldreq->hsyncstart =	newmodeline->hsyncstart;
	oldreq->hsyncend =	newmodeline->hsyncend;
	oldreq->htotal =	newmodeline->htotal;
	oldreq->vdisplay =	newmodeline->vdisplay;
	oldreq->vsyncstart =	newmodeline->vsyncstart;
	oldreq->vsyncend =	newmodeline->vsyncend;
	oldreq->vtotal =	newmodeline->vtotal;
	oldreq->flags =		newmodeline->flags;
	oldreq->privsize =	newmodeline->privsize;
	if (aftermodeline != NULL) {
	    oldreq->after_dotclock =	aftermodeline->dotclock;
	    oldreq->after_hdisplay =	aftermodeline->hdisplay;
	    oldreq->after_hsyncstart =	aftermodeline->hsyncstart;
	    oldreq->after_hsyncend =	aftermodeline->hsyncend;
	    oldreq->after_htotal =	aftermodeline->htotal;
	    oldreq->after_vdisplay =	aftermodeline->vdisplay;
	    oldreq->after_vsyncstart =	aftermodeline->vsyncstart;
	    oldreq->after_vsyncend =	aftermodeline->vsyncend;
	    oldreq->after_vtotal =	aftermodeline->vtotal;
	    oldreq->after_flags =	aftermodeline->flags;
	} else {
	    oldreq->after_dotclock =	0;
	    oldreq->after_hdisplay =	0;
	    oldreq->after_hsyncstart =	0;
	    oldreq->after_hsyncend =	0;
	    oldreq->after_htotal =	0;
	    oldreq->after_vdisplay =	0;
	    oldreq->after_vsyncstart =	0;
	    oldreq->after_vsyncend =	0;
	    oldreq->after_vtotal =	0;
	    oldreq->after_flags =	0;
	}
	if (newmodeline->privsize) {
	    oldreq->length += newmodeline->privsize;
	    Data32(dpy, (long *) newmodeline->private,
	       newmodeline->privsize * sizeof(INT32));
	}
    } else {
	xXF86VidModeAddModeLineReq *req;

	GetReq(XF86VidModeAddModeLine, req);
	req->reqType = info->codes->major_opcode;
	req->xf86vidmodeReqType = X_XF86VidModeAddModeLine;
	req->screen = screen;
	req->dotclock =		newmodeline->dotclock;
	req->hdisplay =		newmodeline->hdisplay;
	req->hsyncstart =	newmodeline->hsyncstart;
	req->hsyncend =		newmodeline->hsyncend;
	req->htotal =		newmodeline->htotal;
	req->hskew =		newmodeline->hskew;
	req->vdisplay =		newmodeline->vdisplay;
	req->vsyncstart =	newmodeline->vsyncstart;
	req->vsyncend =		newmodeline->vsyncend;
	req->vtotal =		newmodeline->vtotal;
	req->flags =		newmodeline->flags;
	req->privsize =		newmodeline->privsize;
	if (aftermodeline != NULL) {
	    req->after_dotclock =	aftermodeline->dotclock;
	    req->after_hdisplay =	aftermodeline->hdisplay;
	    req->after_hsyncstart =	aftermodeline->hsyncstart;
	    req->after_hsyncend =	aftermodeline->hsyncend;
	    req->after_htotal =		aftermodeline->htotal;
	    req->after_hskew =		aftermodeline->hskew;
	    req->after_vdisplay =	aftermodeline->vdisplay;
	    req->after_vsyncstart =	aftermodeline->vsyncstart;
	    req->after_vsyncend =	aftermodeline->vsyncend;
	    req->after_vtotal =		aftermodeline->vtotal;
	    req->after_flags =		aftermodeline->flags;
	} else {
	    req->after_dotclock =	0;
	    req->after_hdisplay =	0;
	    req->after_hsyncstart =	0;
	    req->after_hsyncend =	0;
	    req->after_htotal =		0;
	    req->after_hskew =		0;
	    req->after_vdisplay =	0;
	    req->after_vsyncstart =	0;
	    req->after_vsyncend =	0;
	    req->after_vtotal =		0;
	    req->after_flags =		0;
	}
	if (newmodeline->privsize) {
	    req->length += newmodeline->privsize;
	    Data32(dpy, (long *) newmodeline->private,
	       newmodeline->privsize * sizeof(INT32));
	}
    }
    UnlockDisplay(dpy);
    SyncHandle();
    return True;
}

Bool
XF86VidModeDeleteModeLine(Display *dpy, int screen,
			  XF86VidModeModeInfo* modeline)
{
    XExtDisplayInfo *info = find_display (dpy);
    int majorVersion, minorVersion;

    XF86VidModeCheckExtension (dpy, info, 0);
    XF86VidModeQueryVersion(dpy, &majorVersion, &minorVersion);

    LockDisplay(dpy);
    if (_X_UNLIKELY(majorVersion < 2)) {
	xXF86OldVidModeDeleteModeLineReq *oldreq;

	GetOldReq(XF86VidModeDeleteModeLine, XF86OldVidModeDeleteModeLine, oldreq);
	oldreq->reqType = info->codes->major_opcode;
	oldreq->xf86vidmodeReqType = X_XF86VidModeDeleteModeLine;
	oldreq->screen = screen;
	oldreq->dotclock =	modeline->dotclock;
	oldreq->hdisplay =	modeline->hdisplay;
	oldreq->hsyncstart =	modeline->hsyncstart;
	oldreq->hsyncend =	modeline->hsyncend;
	oldreq->htotal =	modeline->htotal;
	oldreq->vdisplay =	modeline->vdisplay;
	oldreq->vsyncstart =	modeline->vsyncstart;
	oldreq->vsyncend =	modeline->vsyncend;
	oldreq->vtotal =	modeline->vtotal;
	oldreq->flags =		modeline->flags;
	oldreq->privsize =	modeline->privsize;
	if (modeline->privsize) {
	    oldreq->length += modeline->privsize;
	    Data32(dpy, (long *) modeline->private,
	       modeline->privsize * sizeof(INT32));
	}
    } else {
	xXF86VidModeDeleteModeLineReq *req;

	GetReq(XF86VidModeDeleteModeLine, req);
	req->reqType = info->codes->major_opcode;
	req->xf86vidmodeReqType = X_XF86VidModeDeleteModeLine;
	req->screen = screen;
	req->dotclock =		modeline->dotclock;
	req->hdisplay =		modeline->hdisplay;
	req->hsyncstart =	modeline->hsyncstart;
	req->hsyncend =		modeline->hsyncend;
	req->htotal =		modeline->htotal;
	req->hskew =		modeline->hskew;
	req->vdisplay =		modeline->vdisplay;
	req->vsyncstart =	modeline->vsyncstart;
	req->vsyncend =		modeline->vsyncend;
	req->vtotal =		modeline->vtotal;
	req->flags =		modeline->flags;
	req->privsize =		modeline->privsize;
	if (modeline->privsize) {
	    req->length += modeline->privsize;
	    Data32(dpy, (long *) modeline->private,
	       modeline->privsize * sizeof(INT32));
	}
    }
    UnlockDisplay(dpy);
    SyncHandle();
    return True;
}

Bool
XF86VidModeModModeLine(Display *dpy, int screen, XF86VidModeModeLine* modeline)
{
    XExtDisplayInfo *info = find_display (dpy);
    int majorVersion, minorVersion;

    XF86VidModeCheckExtension (dpy, info, 0);
    XF86VidModeQueryVersion(dpy, &majorVersion, &minorVersion);

    LockDisplay(dpy);
    if (_X_UNLIKELY(majorVersion < 2)) {
	xXF86OldVidModeModModeLineReq *oldreq;

	GetOldReq(XF86VidModeModModeLine, XF86OldVidModeModModeLine, oldreq);
	oldreq->reqType = info->codes->major_opcode;
	oldreq->xf86vidmodeReqType = X_XF86VidModeModModeLine;
	oldreq->screen = screen;
	oldreq->hdisplay =	modeline->hdisplay;
	oldreq->hsyncstart =	modeline->hsyncstart;
	oldreq->hsyncend =	modeline->hsyncend;
	oldreq->htotal =	modeline->htotal;
	oldreq->vdisplay =	modeline->vdisplay;
	oldreq->vsyncstart =	modeline->vsyncstart;
	oldreq->vsyncend =	modeline->vsyncend;
	oldreq->vtotal =	modeline->vtotal;
	oldreq->flags =		modeline->flags;
	oldreq->privsize =	modeline->privsize;
	if (modeline->privsize) {
	    oldreq->length += modeline->privsize;
	    Data32(dpy, (long *) modeline->private,
	       modeline->privsize * sizeof(INT32));
	}
    } else {
	xXF86VidModeModModeLineReq *req;

	GetReq(XF86VidModeModModeLine, req);
	req->reqType = info->codes->major_opcode;
	req->xf86vidmodeReqType = X_XF86VidModeModModeLine;
	req->screen = screen;
	req->hdisplay =		modeline->hdisplay;
	req->hsyncstart =	modeline->hsyncstart;
	req->hsyncend =		modeline->hsyncend;
	req->htotal =		modeline->htotal;
	req->hskew =		modeline->hskew;
	req->vdisplay =		modeline->vdisplay;
	req->vsyncstart =	modeline->vsyncstart;
	req->vsyncend =		modeline->vsyncend;
	req->vtotal =		modeline->vtotal;
	req->flags =		modeline->flags;
	req->privsize =		modeline->privsize;
	if (modeline->privsize) {
	    req->length += modeline->privsize;
	    Data32(dpy, (long *) modeline->private,
	       modeline->privsize * sizeof(INT32));
	}
    }
    UnlockDisplay(dpy);
    SyncHandle();
    return True;
}

Status
XF86VidModeValidateModeLine(Display *dpy, int screen,
			    XF86VidModeModeInfo* modeline)
{
    XExtDisplayInfo *info = find_display (dpy);
    xXF86VidModeValidateModeLineReply rep;
    int majorVersion, minorVersion;

    XF86VidModeCheckExtension (dpy, info, 0);
    XF86VidModeQueryVersion(dpy, &majorVersion, &minorVersion);

    LockDisplay(dpy);

    if (_X_UNLIKELY(majorVersion < 2)) {
	xXF86OldVidModeValidateModeLineReq *oldreq;

	GetOldReq(XF86VidModeValidateModeLine, XF86OldVidModeValidateModeLine, oldreq);
	oldreq->reqType = info->codes->major_opcode;
	oldreq->xf86vidmodeReqType = X_XF86VidModeValidateModeLine;
	oldreq->screen = screen;
	oldreq->dotclock =	modeline->dotclock;
	oldreq->hdisplay =	modeline->hdisplay;
	oldreq->hsyncstart =	modeline->hsyncstart;
	oldreq->hsyncend =	modeline->hsyncend;
	oldreq->htotal =	modeline->htotal;
	oldreq->vdisplay =	modeline->vdisplay;
	oldreq->vsyncstart =	modeline->vsyncstart;
	oldreq->vsyncend =	modeline->vsyncend;
	oldreq->vtotal =	modeline->vtotal;
	oldreq->flags =		modeline->flags;
	oldreq->privsize =	modeline->privsize;
	if (modeline->privsize) {
	    oldreq->length += modeline->privsize;
	    Data32(dpy, (long *) modeline->private,
	       modeline->privsize * sizeof(INT32));
	}
    } else {
	xXF86VidModeValidateModeLineReq *req;

	GetReq(XF86VidModeValidateModeLine, req);
	req->reqType = info->codes->major_opcode;
	req->xf86vidmodeReqType = X_XF86VidModeValidateModeLine;
	req->screen = screen;
	req->dotclock =		modeline->dotclock;
	req->hdisplay =		modeline->hdisplay;
	req->hsyncstart =	modeline->hsyncstart;
	req->hsyncend =		modeline->hsyncend;
	req->htotal =		modeline->htotal;
	req->hskew =		modeline->hskew;
	req->vdisplay =		modeline->vdisplay;
	req->vsyncstart =	modeline->vsyncstart;
	req->vsyncend =		modeline->vsyncend;
	req->vtotal =		modeline->vtotal;
	req->flags =		modeline->flags;
	req->privsize =		modeline->privsize;
	if (modeline->privsize) {
	    req->length += modeline->privsize;
	    Data32(dpy, (long *) modeline->private,
	       modeline->privsize * sizeof(INT32));
	}
    }
    if (!_XReply(dpy, (xReply *)&rep, 0, xFalse)) {
	UnlockDisplay(dpy);
	SyncHandle();
	return MODE_BAD;
    }
    UnlockDisplay(dpy);
    SyncHandle();
    return rep.status;
}

Bool
XF86VidModeSwitchMode(Display* dpy, int screen, int zoom)
{
    XExtDisplayInfo *info = find_display (dpy);
    xXF86VidModeSwitchModeReq *req;

    XF86VidModeCheckExtension (dpy, info, False);

    LockDisplay(dpy);
    GetReq(XF86VidModeSwitchMode, req);
    req->reqType = info->codes->major_opcode;
    req->xf86vidmodeReqType = X_XF86VidModeSwitchMode;
    req->screen = screen;
    req->zoom = zoom;
    UnlockDisplay(dpy);
    SyncHandle();
    return True;
}

Bool
XF86VidModeSwitchToMode(Display* dpy, int screen, XF86VidModeModeInfo* modeline)
{
    XExtDisplayInfo *info = find_display (dpy);
    xXF86VidModeSwitchToModeReq *req;
    xXF86OldVidModeSwitchToModeReq *oldreq;
    int majorVersion, minorVersion;
    Bool protocolBug = False;

    XF86VidModeCheckExtension (dpy, info, False);

    /*
     * Note: There was a bug in the protocol implementation in versions
     * 0.x with x < 8 (the .private field wasn't expected to be sent over
     * the wire).  Check the server's version, and accept the old format
     * if appropriate.
     */

    XF86VidModeQueryVersion(dpy, &majorVersion, &minorVersion);
    if (_X_UNLIKELY(majorVersion == 0 && minorVersion < 8)) {
	protocolBug = True;
#ifdef DEBUG
	fprintf(stderr, "XF86VidModeSwitchToMode: Warning: Xserver is"
		"running an old version (%d.%d)\n", majorVersion,
		minorVersion);
#endif
    }

    LockDisplay(dpy);
    if (_X_UNLIKELY(majorVersion < 2)) {
	GetOldReq(XF86VidModeSwitchToMode, XF86OldVidModeSwitchToMode, oldreq);
	oldreq->reqType = info->codes->major_opcode;
	oldreq->xf86vidmodeReqType = X_XF86VidModeSwitchToMode;
	oldreq->screen = screen;
	oldreq->dotclock =	modeline->dotclock;
	oldreq->hdisplay =	modeline->hdisplay;
	oldreq->hsyncstart =	modeline->hsyncstart;
	oldreq->hsyncend =	modeline->hsyncend;
	oldreq->htotal =	modeline->htotal;
	oldreq->vdisplay =	modeline->vdisplay;
	oldreq->vsyncstart =	modeline->vsyncstart;
	oldreq->vsyncend =	modeline->vsyncend;
	oldreq->vtotal =	modeline->vtotal;
	oldreq->flags =	modeline->flags;
	if (protocolBug) {
	    oldreq->privsize = 0;
	} else {
	    oldreq->privsize =	modeline->privsize;
	    if (modeline->privsize) {
		oldreq->length += modeline->privsize;
		Data32(dpy, (long *) modeline->private,
	           modeline->privsize * sizeof(INT32));
	    }
	}
    } else {
	GetReq(XF86VidModeSwitchToMode, req);
	req->reqType = info->codes->major_opcode;
	req->xf86vidmodeReqType = X_XF86VidModeSwitchToMode;
	req->screen = screen;
	req->dotclock =	modeline->dotclock;
	req->hdisplay =	modeline->hdisplay;
	req->hsyncstart =	modeline->hsyncstart;
	req->hsyncend =	modeline->hsyncend;
	req->htotal =	modeline->htotal;
	req->hskew =	modeline->hskew;
	req->vdisplay =	modeline->vdisplay;
	req->vsyncstart =	modeline->vsyncstart;
	req->vsyncend =	modeline->vsyncend;
	req->vtotal =	modeline->vtotal;
	req->flags =	modeline->flags;
	if (protocolBug) {
	    req->privsize = 0;
	} else {
	    req->privsize =	modeline->privsize;
	    if (modeline->privsize) {
		req->length += modeline->privsize;
		Data32(dpy, (long *) modeline->private,
	           modeline->privsize * sizeof(INT32));
	    }
	}
    }
    UnlockDisplay(dpy);
    SyncHandle();
    return True;
}

Bool
XF86VidModeLockModeSwitch(Display* dpy, int screen, int lock)
{
    XExtDisplayInfo *info = find_display (dpy);
    xXF86VidModeLockModeSwitchReq *req;

    XF86VidModeCheckExtension (dpy, info, False);

    LockDisplay(dpy);
    GetReq(XF86VidModeLockModeSwitch, req);
    req->reqType = info->codes->major_opcode;
    req->xf86vidmodeReqType = X_XF86VidModeLockModeSwitch;
    req->screen = screen;
    req->lock = lock;
    UnlockDisplay(dpy);
    SyncHandle();
    return True;
}

Bool
XF86VidModeGetMonitor(Display* dpy, int screen, XF86VidModeMonitor* monitor)
{
    XExtDisplayInfo *info = find_display (dpy);
    xXF86VidModeGetMonitorReply rep;
    xXF86VidModeGetMonitorReq *req;
    CARD32 syncrange;
    int i;
    Bool result = True;

    XF86VidModeCheckExtension (dpy, info, False);

    LockDisplay(dpy);
    GetReq(XF86VidModeGetMonitor, req);
    req->reqType = info->codes->major_opcode;
    req->xf86vidmodeReqType = X_XF86VidModeGetMonitor;
    req->screen = screen;
    if (!_XReply(dpy, (xReply *)&rep, 0, xFalse)) {
	UnlockDisplay(dpy);
	SyncHandle();
	return False;
    }
    monitor->nhsync = rep.nhsync;
    monitor->nvsync = rep.nvsync;
#if 0
    monitor->bandwidth = (float)rep.bandwidth / 1e6;
#endif
    if (rep.vendorLength) {
	monitor->vendor = Xcalloc(rep.vendorLength + 1, 1);
	if (monitor->vendor == NULL)
	    result = False;
    } else {
	monitor->vendor = NULL;
    }
    if (result && rep.modelLength) {
	monitor->model = Xcalloc(rep.modelLength + 1, 1);
	if (monitor->model == NULL)
	    result = False;
    } else {
	monitor->model = NULL;
    }
    if (result) {
	monitor->hsync = Xcalloc(rep.nhsync, sizeof(XF86VidModeSyncRange));
	monitor->vsync = Xcalloc(rep.nvsync, sizeof(XF86VidModeSyncRange));
	if ((monitor->hsync == NULL) || (monitor->vsync == NULL))
	    result = False;
    } else {
	monitor->hsync = monitor->vsync = NULL;
    }
    if (result == False) {
	_XEatDataWords(dpy, rep.length);
	Xfree(monitor->vendor);
	monitor->vendor = NULL;
	Xfree(monitor->model);
	monitor->model = NULL;
	Xfree(monitor->hsync);
	monitor->hsync = NULL;
	Xfree(monitor->vsync);
	monitor->vsync = NULL;
    }
    else {
	for (i = 0; i < rep.nhsync; i++) {
	    _XRead(dpy, (char *)&syncrange, 4);
	    monitor->hsync[i].lo = (float)(syncrange & 0xFFFF) / 100.0;
	    monitor->hsync[i].hi = (float)(syncrange >> 16) / 100.0;
	}
	for (i = 0; i < rep.nvsync; i++) {
	    _XRead(dpy, (char *)&syncrange, 4);
	    monitor->vsync[i].lo = (float)(syncrange & 0xFFFF) / 100.0;
	    monitor->vsync[i].hi = (float)(syncrange >> 16) / 100.0;
	}
	if (rep.vendorLength)
	    _XReadPad(dpy, monitor->vendor, rep.vendorLength);
	if (rep.modelLength)
	    _XReadPad(dpy, monitor->model, rep.modelLength);
    }
    UnlockDisplay(dpy);
    SyncHandle();
    return result;
}

Bool
XF86VidModeGetViewPort(Display* dpy, int screen, int *x, int *y)
{
    XExtDisplayInfo *info = find_display (dpy);
    xXF86VidModeGetViewPortReply rep;
    xXF86VidModeGetViewPortReq *req;
    int majorVersion, minorVersion;
    Bool protocolBug = False;

    XF86VidModeCheckExtension (dpy, info, False);

    /*
     * Note: There was a bug in the protocol implementation in versions
     * 0.x with x < 8 (no reply was sent, so the client would hang)
     * Check the server's version, and don't wait for a reply with older
     * versions.
     */

    XF86VidModeQueryVersion(dpy, &majorVersion, &minorVersion);
    if (majorVersion == 0 && minorVersion < 8) {
	protocolBug = True;
#ifdef DEBUG
	fprintf(stderr, "XF86VidModeGetViewPort: Warning: Xserver is"
		"running an old version (%d.%d)\n", majorVersion,
		minorVersion);
#endif
    }
    LockDisplay(dpy);
    GetReq(XF86VidModeGetViewPort, req);
    req->reqType = info->codes->major_opcode;
    req->xf86vidmodeReqType = X_XF86VidModeGetViewPort;
    req->screen = screen;
    if (protocolBug) {
	*x = 0;
	*y = 0;
    } else {
	if (!_XReply(dpy, (xReply *)&rep, 0, xFalse)) {
	    UnlockDisplay(dpy);
	    SyncHandle();
	    return False;
	}
	*x = rep.x;
	*y = rep.y;
    }

    UnlockDisplay(dpy);
    SyncHandle();
    return True;
}

Bool
XF86VidModeSetViewPort(Display* dpy, int screen, int x, int y)
{
    XExtDisplayInfo *info = find_display (dpy);
    xXF86VidModeSetViewPortReq *req;

    XF86VidModeCheckExtension (dpy, info, False);

    LockDisplay(dpy);
    GetReq(XF86VidModeSetViewPort, req);
    req->reqType = info->codes->major_opcode;
    req->xf86vidmodeReqType = X_XF86VidModeSetViewPort;
    req->screen = screen;
    req->x = x;
    req->y = y;

    UnlockDisplay(dpy);
    SyncHandle();
    return True;
}

Bool
XF86VidModeGetDotClocks(Display* dpy, int screen, int *flagsPtr,
			int *numclocksPtr, int *maxclocksPtr, int *clocksPtr[])
{
    XExtDisplayInfo *info = find_display (dpy);
    xXF86VidModeGetDotClocksReply rep;
    xXF86VidModeGetDotClocksReq *req;
    int *dotclocks;
    CARD32 dotclk;
    Bool result = True;

    XF86VidModeCheckExtension (dpy, info, False);

    LockDisplay(dpy);
    GetReq(XF86VidModeGetDotClocks, req);
    req->reqType = info->codes->major_opcode;
    req->xf86vidmodeReqType = X_XF86VidModeGetDotClocks;
    req->screen = screen;
    if (!_XReply(dpy, (xReply *)&rep,
        (SIZEOF(xXF86VidModeGetDotClocksReply) - SIZEOF(xReply)) >> 2, xFalse))
    {
        UnlockDisplay(dpy);
        SyncHandle();
        return False;
    }
    *numclocksPtr = rep.clocks;
    *maxclocksPtr = rep.maxclocks;
    *flagsPtr     = rep.flags;

    dotclocks = Xcalloc(rep.clocks, sizeof(int));
    if (dotclocks == NULL) {
        _XEatDataWords(dpy, rep.length -
	    ((SIZEOF(xXF86VidModeGetDotClocksReply) - SIZEOF(xReply)) >> 2));
        result = False;
    }
    else {
	unsigned int i;

	for (i = 0; i < rep.clocks; i++) {
	    _XRead(dpy, (char*)&dotclk, 4);
	    dotclocks[i] = dotclk;
	}
    }
    *clocksPtr = dotclocks;
    UnlockDisplay(dpy);
    SyncHandle();
    return result;
}

Bool
XF86VidModeSetGammaRamp (
    Display *dpy,
    int screen,
    int size,
    unsigned short *red,
    unsigned short *green,
    unsigned short *blue
)
{
    int length = (size + 1) & ~1;
    XExtDisplayInfo *info = find_display (dpy);
    xXF86VidModeSetGammaRampReq *req;

    XF86VidModeCheckExtension (dpy, info, False);
    LockDisplay(dpy);
    GetReq(XF86VidModeSetGammaRamp, req);
    req->reqType = info->codes->major_opcode;
    req->xf86vidmodeReqType = X_XF86VidModeSetGammaRamp;
    req->screen = screen;
    req->length += (length >> 1) * 3;
    req->size = size;
    _XSend(dpy, (char*)red, size * 2);
    _XSend(dpy, (char*)green, size * 2);
    _XSend(dpy, (char*)blue, size * 2);
    UnlockDisplay(dpy);
    SyncHandle();
    return True;
}


Bool
XF86VidModeGetGammaRamp (
    Display *dpy,
    int screen,
    int size,
    unsigned short *red,
    unsigned short *green,
    unsigned short *blue
)
{
    XExtDisplayInfo *info = find_display (dpy);
    xXF86VidModeGetGammaRampReq *req;
    xXF86VidModeGetGammaRampReply rep;
    Bool result = True;

    XF86VidModeCheckExtension (dpy, info, False);

    LockDisplay(dpy);
    GetReq(XF86VidModeGetGammaRamp, req);
    req->reqType = info->codes->major_opcode;
    req->xf86vidmodeReqType = X_XF86VidModeGetGammaRamp;
    req->screen = screen;
    req->size = size;
    if (!_XReply (dpy, (xReply *) &rep, 0, xFalse)) {
        result = False;
    }
    else if (rep.size) {
	if (rep.size <= size) {
	    _XRead(dpy, (char*)red, rep.size << 1);
	    _XRead(dpy, (char*)green, rep.size << 1);
	    _XRead(dpy, (char*)blue, rep.size << 1);
	}
	else {
	    _XEatDataWords(dpy, rep.length);
	    result = False;
	}
    }

    UnlockDisplay(dpy);
    SyncHandle();
    return result;
}

Bool XF86VidModeGetGammaRampSize(
    Display *dpy,
    int screen,
    int *size
)
{
    XExtDisplayInfo *info = find_display (dpy);
    xXF86VidModeGetGammaRampSizeReq *req;
    xXF86VidModeGetGammaRampSizeReply rep;

    *size = 0;

    XF86VidModeCheckExtension (dpy, info, False);

    LockDisplay(dpy);
    GetReq(XF86VidModeGetGammaRampSize, req);
    req->reqType = info->codes->major_opcode;
    req->xf86vidmodeReqType = X_XF86VidModeGetGammaRampSize;
    req->screen = screen;
    if (!_XReply (dpy, (xReply *) &rep, 0, xTrue)) {
        UnlockDisplay (dpy);
        SyncHandle ();
        return False;
    }
    *size = rep.size;
    UnlockDisplay(dpy);
    SyncHandle();
    return True;
}

Bool XF86VidModeGetPermissions(
    Display *dpy,
    int screen,
    int *permissions
)
{
    XExtDisplayInfo *info = find_display (dpy);
    xXF86VidModeGetPermissionsReq *req;
    xXF86VidModeGetPermissionsReply rep;

    *permissions = 0;

    XF86VidModeCheckExtension (dpy, info, False);

    LockDisplay(dpy);
    GetReq(XF86VidModeGetPermissions, req);
    req->reqType = info->codes->major_opcode;
    req->xf86vidmodeReqType = X_XF86VidModeGetPermissions;
    req->screen = screen;
    if (!_XReply (dpy, (xReply *) &rep, 0, xTrue)) {
        UnlockDisplay (dpy);
        SyncHandle ();
        return False;
    }
    *permissions = rep.permissions;
    UnlockDisplay(dpy);
    SyncHandle();
    return True;
}


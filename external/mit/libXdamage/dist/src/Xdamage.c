/*
 * Copyright © 2003 Keith Packard
 * Copyright © 2007 Eric Anholt
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Keith Packard not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  Keith Packard makes no
 * representations about the suitability of this software for any purpose.  It
 * is provided "as is" without express or implied warranty.
 *
 * KEITH PACKARD DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL KEITH PACKARD BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include "xdamageint.h"
#include <X11/Xfuncproto.h>

XDamageExtInfo XDamageExtensionInfo;

const char XDamageExtensionName[] = DAMAGE_NAME;

static int
XDamageCloseDisplay (Display *dpy, XExtCodes *codes);

static Bool
XDamageWireToEvent(Display *dpy, XEvent *event, xEvent *wire);

static Status
XDamageEventToWire(Display *dpy, XEvent *event, xEvent *wire);

/*
 * XDamageExtAddDisplay - add a display to this extension. (Replaces
 * XextAddDisplay)
 */
static XDamageExtDisplayInfo *
XDamageExtAddDisplay (XDamageExtInfo	*extinfo,
                      Display		*dpy,
                      const char	*ext_name)
{
    XDamageExtDisplayInfo    *info;

    info = Xmalloc (sizeof (XDamageExtDisplayInfo));
    if (!info) return NULL;
    info->display = dpy;

    info->codes = XInitExtension (dpy, ext_name);

    /*
     * if the server has the extension, then we can initialize the
     * appropriate function vectors
     */
    if (info->codes) {
	xDamageQueryVersionReply	rep;
	xDamageQueryVersionReq	*req;
        XESetCloseDisplay (dpy, info->codes->extension,
                           XDamageCloseDisplay);
	for (int ev = info->codes->first_event;
	     ev < info->codes->first_event + XDamageNumberEvents;
	     ev++)
	{
	    XESetWireToEvent (dpy, ev, XDamageWireToEvent);
	    XESetEventToWire (dpy, ev, XDamageEventToWire);
	}
	/*
	 * Get the version info
	 */
	LockDisplay (dpy);
	GetReq (DamageQueryVersion, req);
	req->reqType = (CARD8) info->codes->major_opcode;
	req->damageReqType = X_DamageQueryVersion;
	req->majorVersion = DAMAGE_MAJOR;
	req->minorVersion = DAMAGE_MINOR;
	if (!_XReply (dpy, (xReply *) &rep, 0, xTrue))
	{
	    UnlockDisplay (dpy);
	    SyncHandle ();
	    Xfree(info);
	    return NULL;
	}
	info->major_version = rep.majorVersion;
	info->minor_version = rep.minorVersion;
	UnlockDisplay (dpy);
	SyncHandle ();
    } else {
	/* The server doesn't have this extension.
	 * Use a private Xlib-internal extension to hang the close_display
	 * hook on so that the "cache" (extinfo->cur) is properly cleaned.
	 * (XBUG 7955)
	 */
	XExtCodes *codes = XAddExtension(dpy);
	if (!codes) {
	    Xfree(info);
	    return NULL;
	}
        XESetCloseDisplay (dpy, codes->extension, XDamageCloseDisplay);
    }

    /*
     * now, chain it onto the list
     */
    _XLockMutex(_Xglobal_lock);
    info->next = extinfo->head;
    extinfo->head = info;
    extinfo->cur = info;
    extinfo->ndisplays++;
    _XUnlockMutex(_Xglobal_lock);
    return info;
}


/*
 * XDamageExtRemoveDisplay - remove the indicated display from the
 * extension object. (Replaces XextRemoveDisplay.)
 */
static int
XDamageExtRemoveDisplay (XDamageExtInfo *extinfo, const Display *dpy)
{
    XDamageExtDisplayInfo *info, *prev;

    /*
     * locate this display and its back link so that it can be removed
     */
    _XLockMutex(_Xglobal_lock);
    prev = NULL;
    for (info = extinfo->head; info; info = info->next) {
	if (info->display == dpy) break;
	prev = info;
    }
    if (!info) {
	_XUnlockMutex(_Xglobal_lock);
	return 0;		/* hmm, actually an error */
    }

    /*
     * remove the display from the list; handles going to zero
     */
    if (prev)
	prev->next = info->next;
    else
	extinfo->head = info->next;

    extinfo->ndisplays--;
    if (info == extinfo->cur) extinfo->cur = NULL;  /* flush cache */
    _XUnlockMutex(_Xglobal_lock);

    Xfree (info);
    return 1;
}

/*
 * XDamageExtFindDisplay - look for a display in this extension; keeps a
 * cache of the most-recently used for efficiency. (Replaces
 * XextFindDisplay.)
 */
static XDamageExtDisplayInfo *
XDamageExtFindDisplay (XDamageExtInfo *extinfo,
		       const Display  *dpy)
{
    XDamageExtDisplayInfo *info;

    /*
     * see if this was the most recently accessed display
     */
    if ((info = extinfo->cur) && info->display == dpy)
	return info;

    /*
     * look for display in list
     */
    _XLockMutex(_Xglobal_lock);
    for (info = extinfo->head; info; info = info->next) {
	if (info->display == dpy) {
	    extinfo->cur = info;     /* cache most recently used */
	    _XUnlockMutex(_Xglobal_lock);
	    return info;
	}
    }
    _XUnlockMutex(_Xglobal_lock);

    return NULL;
}

XDamageExtDisplayInfo *
XDamageFindDisplay (Display *dpy)
{
    XDamageExtDisplayInfo *info;

    info = XDamageExtFindDisplay (&XDamageExtensionInfo, dpy);
    if (!info)
	info = XDamageExtAddDisplay (&XDamageExtensionInfo, dpy,
				    XDamageExtensionName);
    return info;
}

static int
XDamageCloseDisplay (Display *dpy, _X_UNUSED XExtCodes *codes)
{
    return XDamageExtRemoveDisplay (&XDamageExtensionInfo, dpy);
}

static Bool
XDamageWireToEvent(Display *dpy, XEvent *event, xEvent *wire)
{
    XDamageExtDisplayInfo *info = XDamageFindDisplay(dpy);

    XDamageCheckExtension(dpy, info, False);

    switch ((wire->u.u.type & 0x7F) - info->codes->first_event)
    {
    case XDamageNotify: {
	XDamageNotifyEvent *aevent = (XDamageNotifyEvent *) event;
	xDamageNotifyEvent *awire = (xDamageNotifyEvent *) wire;

	aevent->type = awire->type & 0x7F;
	aevent->serial = _XSetLastRequestRead(dpy,
					      (xGenericReply *) wire);
	aevent->send_event = (awire->type & 0x80) != 0;
	aevent->display = dpy;
	aevent->drawable = awire->drawable;
	aevent->damage = awire->damage;
	aevent->level = awire->level & ~DamageNotifyMore;
	aevent->more = (awire->level & DamageNotifyMore) ? True : False;
	aevent->timestamp = awire->timestamp;
	aevent->area.x = awire->area.x;
	aevent->area.y = awire->area.y;
	aevent->area.width = awire->area.width;
	aevent->area.height = awire->area.height;
	aevent->geometry.x = awire->geometry.x;
	aevent->geometry.y = awire->geometry.y;
	aevent->geometry.width = awire->geometry.width;
	aevent->geometry.height = awire->geometry.height;
	return True;
    }
    }
    return False;
}

static Status
XDamageEventToWire(Display *dpy, XEvent *event, xEvent *wire)
{
    XDamageExtDisplayInfo *info = XDamageFindDisplay(dpy);

    XDamageCheckExtension(dpy, info, False);

    switch ((event->type & 0x7F) - info->codes->first_event)
    {
    case XDamageNotify: {
	XDamageNotifyEvent *aevent;
	xDamageNotifyEvent *awire;
	awire = (xDamageNotifyEvent *) wire;
	aevent = (XDamageNotifyEvent *) event;
	awire->type = (CARD8) aevent->type | (aevent->send_event ? 0x80 : 0);
	awire->drawable = (CARD32) aevent->drawable;
	awire->damage = (CARD32) aevent->damage;
	awire->level = (CARD8) aevent->level | (aevent->more ? DamageNotifyMore : 0);
	awire->timestamp = (CARD32) aevent->timestamp;
	awire->area.x = aevent->area.x;
	awire->area.y = aevent->area.y;
	awire->area.width = aevent->area.width;
	awire->area.height = aevent->area.height;
	awire->geometry.x = aevent->geometry.x;
	awire->geometry.y = aevent->geometry.y;
	awire->geometry.width = aevent->geometry.width;
	awire->geometry.height = aevent->geometry.height;
	return True;
    }
    }
    return False;
}

Bool
XDamageQueryExtension (Display *dpy,
			int *event_base_return,
			int *error_base_return)
{
    XDamageExtDisplayInfo *info = XDamageFindDisplay (dpy);

    if (XDamageHasExtension(info))
    {
	*event_base_return = info->codes->first_event;
	*error_base_return = info->codes->first_error;
	return True;
    }
    else
	return False;
}

Status
XDamageQueryVersion (Display *dpy,
		    int	    *major_version_return,
		    int	    *minor_version_return)
{
    XDamageExtDisplayInfo	*info = XDamageFindDisplay (dpy);

    XDamageCheckExtension (dpy, info, 0);

    *major_version_return = info->major_version;
    *minor_version_return = info->minor_version;
    return 1;
}

Damage
XDamageCreate (Display *dpy, Drawable drawable, int level)
{
    XDamageExtDisplayInfo	*info = XDamageFindDisplay (dpy);
    xDamageCreateReq		*req;
    Damage			damage;

    XDamageCheckExtension (dpy, info, 0);
    LockDisplay (dpy);
    GetReq (DamageCreate, req);
    req->reqType = (CARD8) info->codes->major_opcode;
    req->damageReqType = X_DamageCreate;
    damage = XAllocID (dpy);
    req->damage = (CARD32) damage;
    req->drawable = (CARD32) drawable;
    req->level = (CARD8) level;
    UnlockDisplay (dpy);
    SyncHandle ();
    return damage;
}

void
XDamageDestroy (Display *dpy, Damage damage)
{
    XDamageExtDisplayInfo	*info = XDamageFindDisplay (dpy);
    xDamageDestroyReq		*req;

    XDamageSimpleCheckExtension (dpy, info);
    LockDisplay (dpy);
    GetReq (DamageDestroy, req);
    req->reqType = (CARD8) info->codes->major_opcode;
    req->damageReqType = X_DamageDestroy;
    req->damage = (CARD32) damage;
    UnlockDisplay (dpy);
    SyncHandle ();
}

void
XDamageSubtract (Display *dpy, Damage damage,
		 XserverRegion repair, XserverRegion parts)
{
    XDamageExtDisplayInfo	*info = XDamageFindDisplay (dpy);
    xDamageSubtractReq		*req;

    XDamageSimpleCheckExtension (dpy, info);
    LockDisplay (dpy);
    GetReq (DamageSubtract, req);
    req->reqType = (CARD8) info->codes->major_opcode;
    req->damageReqType = X_DamageSubtract;
    req->damage = (CARD32) damage;
    req->repair = (CARD32) repair;
    req->parts = (CARD32) parts;
    UnlockDisplay (dpy);
    SyncHandle ();
}

void
XDamageAdd (Display *dpy, Drawable drawable, XserverRegion region)
{
    XDamageExtDisplayInfo	*info = XDamageFindDisplay (dpy);
    xDamageAddReq		*req;

    XDamageSimpleCheckExtension (dpy, info);
    LockDisplay (dpy);
    GetReq (DamageAdd, req);
    req->reqType = (CARD8) info->codes->major_opcode;
    req->damageReqType = X_DamageAdd;
    req->drawable = (CARD32) drawable;
    req->region = (CARD32) region;

    UnlockDisplay (dpy);
    SyncHandle ();
}

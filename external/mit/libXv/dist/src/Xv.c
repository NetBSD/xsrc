/***********************************************************
Copyright 1991 by Digital Equipment Corporation, Maynard, Massachusetts,
and the Massachusetts Institute of Technology, Cambridge, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its
documentation for any purpose and without fee is hereby granted,
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in
supporting documentation, and that the names of Digital or MIT not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/
/*
** File:
**
**   Xv.c --- Xv library extension module.
**
** Author:
**
**   David Carver (Digital Workstation Engineering/Project Athena)
**
** Revisions:
**
**   26.06.91 Carver
**     - changed XvFreeAdaptors to XvFreeAdaptorInfo
**     - changed XvFreeEncodings to XvFreeEncodingInfo
**
**   11.06.91 Carver
**     - changed SetPortControl to SetPortAttribute
**     - changed GetPortControl to GetPortAttribute
**     - changed QueryBestSize
**
**   15.05.91 Carver
**     - version 2.0 upgrade
**
**   240.01.91 Carver
**     - version 1.4 upgrade
**
*/

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdio.h>
#include "Xvlibint.h"
#include <X11/extensions/Xext.h>
#include <X11/extensions/extutil.h>
#include <X11/extensions/XShm.h>
#include <limits.h>

static XExtensionInfo _xv_info_data;
static XExtensionInfo *xv_info = &_xv_info_data;
static const char *xv_extension_name = XvName;

#define XvCheckExtension(dpy, i, val) \
  XextCheckExtension(dpy, i, xv_extension_name, val)

#define pad_to_int32(bytes) (((bytes) + 3) & ~3U)

static char *xv_error_string(Display *dpy, int code, XExtCodes *codes,
                             char *buf, int n);
static int xv_close_display(Display *dpy, XExtCodes *codes);
static Bool xv_wire_to_event(Display *dpy, XEvent *host, xEvent *wire);

static XExtensionHooks xv_extension_hooks = {
    NULL,                       /* create_gc */
    NULL,                       /* copy_gc */
    NULL,                       /* flush_gc */
    NULL,                       /* free_gc */
    NULL,                       /* create_font */
    NULL,                       /* free_font */
    xv_close_display,           /* close_display */
    xv_wire_to_event,           /* wire_to_event */
    NULL,                       /* event_to_wire */
    NULL,                       /* error */
    xv_error_string             /* error_string */
};


static const char *xv_error_list[] = {
    "BadPort",                  /* XvBadPort     */
    "BadEncoding",              /* XvBadEncoding */
    "BadControl"                /* XvBadControl  */
};

static XEXT_GENERATE_CLOSE_DISPLAY(xv_close_display, xv_info)

static XEXT_GENERATE_FIND_DISPLAY(xv_find_display, xv_info,
				  xv_extension_name, &xv_extension_hooks,
				  XvNumEvents, NULL)

static XEXT_GENERATE_ERROR_STRING(xv_error_string, xv_extension_name,
				  XvNumErrors, xv_error_list)


int
XvQueryExtension(
    Display *dpy,
    unsigned int *p_version,
    unsigned int *p_revision,
    unsigned int *p_requestBase,
    unsigned int *p_eventBase,
    unsigned int *p_errorBase)
{
    XExtDisplayInfo *info = xv_find_display(dpy);
    xvQueryExtensionReq *req;
    xvQueryExtensionReply rep;
    int status;

    XvCheckExtension(dpy, info, XvBadExtension);

    LockDisplay(dpy);

    XvGetReq(QueryExtension, req);

    if (!_XReply(dpy, (xReply *) &rep, 0, xFalse)) {
        status = XvBadExtension;
        goto out;
    }

    *p_version = rep.version;
    *p_revision = rep.revision;
    *p_requestBase = (unsigned int) info->codes->major_opcode;
    *p_eventBase = (unsigned int) info->codes->first_event;
    *p_errorBase = (unsigned int) info->codes->first_error;

    status = Success;

  out:
    UnlockDisplay(dpy);
    SyncHandle();

    return status;
}

int
XvQueryAdaptors(
    Display *dpy,
    Window window,
    unsigned int *p_nAdaptors,
    XvAdaptorInfo **p_pAdaptors)
{
    XExtDisplayInfo *info = xv_find_display(dpy);
    xvQueryAdaptorsReq *req;
    xvQueryAdaptorsReply rep;
    size_t size;
    XvAdaptorInfo *pas = NULL, *pa;
    XvFormat *pfs, *pf;
    char *buffer = NULL;
    char *end;
    union {
        char *buffer;
        char *string;
        xvAdaptorInfo *pa;
        xvFormat *pf;
    } u;
    int status;

    XvCheckExtension(dpy, info, XvBadExtension);

    LockDisplay(dpy);

    XvGetReq(QueryAdaptors, req);
    req->window = (CARD32) window;

    /* READ THE REPLY */

    if (_XReply(dpy, (xReply *) &rep, 0, xFalse) == 0) {
        rep.num_adaptors = 0;
        status = XvBadReply;
        goto out;
    }

    size = rep.length << 2;
    if (size > 0) {
        if ((buffer = Xmalloc(size)) == NULL) {
            _XEatDataWords(dpy, rep.length);
            status = XvBadAlloc;
            goto out;
        }
        _XRead(dpy, buffer, (long) size);
    }

    /* GET INPUT ADAPTORS */

    if (rep.num_adaptors == 0) {
        /* If there are no adaptors, there's nothing more to do. */
        status = Success;
        goto out;
    }

    u.buffer = buffer;
    end = buffer + size;

    size = rep.num_adaptors * sizeof(XvAdaptorInfo);
    if ((pas = Xmalloc(size)) == NULL) {
        status = XvBadAlloc;
        goto out;
    }

    /* INIT ADAPTOR FIELDS */

    pa = pas;
    for (unsigned int ii = 0; ii < rep.num_adaptors; ii++) {
        pa->num_adaptors = 0;
        pa->name = (char *) NULL;
        pa->formats = (XvFormat *) NULL;
        pa++;
    }

    pa = pas;
    for (unsigned int ii = 0; ii < rep.num_adaptors; ii++) {
        char *name;

        if (u.buffer + sz_xvAdaptorInfo > end) {
            status = XvBadReply;
            goto out;
        }
        pa->type = (char) u.pa->type;
        pa->base_id = u.pa->base_id;
        pa->num_ports = u.pa->num_ports;
        pa->num_formats = u.pa->num_formats;
        pa->num_adaptors = rep.num_adaptors - ii;

        /* GET ADAPTOR NAME */

        size = u.pa->name_size;
        u.buffer += pad_to_int32(sz_xvAdaptorInfo);

        if (u.buffer + size > end) {
            status = XvBadReply;
            goto out;
        }
        if ((name = Xmalloc(size + 1)) == NULL) {
            status = XvBadAlloc;
            goto out;
        }
        (void) strncpy(name, u.string, size);
        name[size] = '\0';
        pa->name = name;

        u.buffer += pad_to_int32(size);

        /* GET FORMATS */

        size = pa->num_formats * sizeof(XvFormat);
        if ((pfs = Xmalloc(size)) == NULL) {
            status = XvBadAlloc;
            goto out;
        }

        pf = pfs;
        for (unsigned int jj = 0; jj < pa->num_formats; jj++) {
            if (u.buffer + sz_xvFormat > end) {
                Xfree(pfs);
                status = XvBadReply;
                goto out;
            }
            pf->depth = (char) u.pf->depth;
            pf->visual_id = u.pf->visual;
            pf++;

            u.buffer += pad_to_int32(sz_xvFormat);
        }

        pa->formats = pfs;

        pa++;

    }

    status = Success;

  out:
    if (status != Success) {
        XvFreeAdaptorInfo(pas);
        pas = NULL;
    }

    *p_nAdaptors = rep.num_adaptors;
    *p_pAdaptors = pas;

    Xfree(buffer);
    UnlockDisplay(dpy);
    SyncHandle();

    return status;
}


void
XvFreeAdaptorInfo(XvAdaptorInfo *pAdaptors)
{
    XvAdaptorInfo *pa;

    if (!pAdaptors)
        return;

    pa = pAdaptors;

    for (unsigned int ii = 0; ii < pAdaptors->num_adaptors; ii++, pa++) {
        if (pa->name) {
            Xfree(pa->name);
        }
        if (pa->formats) {
            Xfree(pa->formats);
        }
    }

    Xfree(pAdaptors);
}

int
XvQueryEncodings(
    Display *dpy,
    XvPortID port,
    unsigned int *p_nEncodings,
    XvEncodingInfo ** p_pEncodings)
{
    XExtDisplayInfo *info = xv_find_display(dpy);
    xvQueryEncodingsReq *req;
    xvQueryEncodingsReply rep;
    size_t size;
    XvEncodingInfo *pes = NULL, *pe;
    char *buffer = NULL;
    char *end;
    union {
        char *buffer;
        char *string;
        xvEncodingInfo *pe;
    } u;
    int status;

    XvCheckExtension(dpy, info, XvBadExtension);

    LockDisplay(dpy);

    XvGetReq(QueryEncodings, req);
    req->port = (CARD32) port;

    /* READ THE REPLY */

    if (_XReply(dpy, (xReply *) &rep, 0, xFalse) == 0) {
        rep.num_encodings = 0;
        status = XvBadReply;
        goto out;
    }

    size = rep.length << 2;
    if (size > 0) {
        if ((buffer = Xmalloc(size)) == NULL) {
            _XEatDataWords(dpy, rep.length);
            status = XvBadAlloc;
            goto out;
        }
        _XRead(dpy, buffer, (long) size);
    }

    /* GET ENCODINGS */

    if (rep.num_encodings == 0) {
        /* If there are no encodings, there's nothing more to do. */
        status = Success;
        goto out;
    }

    u.buffer = buffer;
    end = buffer + size;

    size = rep.num_encodings * sizeof(XvEncodingInfo);
    if ((pes = Xmalloc(size)) == NULL) {
        status = XvBadAlloc;
        goto out;
    }

    /* INITIALIZE THE ENCODING POINTER */

    pe = pes;
    for (unsigned int jj = 0; jj < rep.num_encodings; jj++) {
        pe->name = (char *) NULL;
        pe->num_encodings = 0;
        pe++;
    }

    pe = pes;
    for (unsigned int jj = 0; jj < rep.num_encodings; jj++) {
        char *name;

        if (u.buffer + sz_xvEncodingInfo > end) {
            status = XvBadReply;
            goto out;
        }
        pe->encoding_id = u.pe->encoding;
        pe->width = u.pe->width;
        pe->height = u.pe->height;
        pe->rate.numerator = u.pe->rate.numerator;
        pe->rate.denominator = u.pe->rate.denominator;
        pe->num_encodings = rep.num_encodings - jj;

        size = u.pe->name_size;
        u.buffer += pad_to_int32(sz_xvEncodingInfo);

        if (u.buffer + size > end) {
            status = XvBadReply;
            goto out;
        }
        if ((name = Xmalloc(size + 1)) == NULL) {
            status = XvBadAlloc;
            goto out;
        }
        strncpy(name, u.string, size);
        name[size] = '\0';
        pe->name = name;
        pe++;

        u.buffer += pad_to_int32(size);
    }

    status = Success;

  out:
    if (status != Success) {
        XvFreeEncodingInfo(pes);
        pes = NULL;
    }

    *p_nEncodings = rep.num_encodings;
    *p_pEncodings = pes;

    Xfree(buffer);
    UnlockDisplay(dpy);
    SyncHandle();

    return (Success);
}

void
XvFreeEncodingInfo(XvEncodingInfo *pEncodings)
{
    XvEncodingInfo *pe;

    if (!pEncodings)
        return;

    pe = pEncodings;

    for (unsigned long ii = 0; ii < pEncodings->num_encodings; ii++, pe++) {
        if (pe->name)
            Xfree(pe->name);
    }

    Xfree(pEncodings);
}

int
XvPutVideo(
    Display *dpy,
    XvPortID port,
    Drawable d,
    GC gc,
    int vx, int vy,
    unsigned int vw, unsigned int vh,
    int dx, int dy,
    unsigned int dw, unsigned int dh)
{
    XExtDisplayInfo *info = xv_find_display(dpy);
    xvPutVideoReq *req;

    XvCheckExtension(dpy, info, XvBadExtension);

    LockDisplay(dpy);

    FlushGC(dpy, gc);

    XvGetReq(PutVideo, req);

    req->port = (CARD32) port;
    req->drawable = (CARD32) d;
    req->gc = (CARD32) gc->gid;
    req->vid_x = (INT16) vx;
    req->vid_y = (INT16) vy;
    req->vid_w = (CARD16) vw;
    req->vid_h = (CARD16) vh;
    req->drw_x = (INT16) dx;
    req->drw_y = (INT16) dy;
    req->drw_w = (CARD16) dw;
    req->drw_h = (CARD16) dh;

    UnlockDisplay(dpy);
    SyncHandle();

    return Success;
}

int
XvPutStill(
    Display *dpy,
    XvPortID port,
    Drawable d,
    GC gc,
    int vx, int vy,
    unsigned int vw, unsigned int vh,
    int dx, int dy,
    unsigned int dw, unsigned int dh)
{
    XExtDisplayInfo *info = xv_find_display(dpy);
    xvPutStillReq *req;

    XvCheckExtension(dpy, info, XvBadExtension);

    LockDisplay(dpy);

    FlushGC(dpy, gc);

    XvGetReq(PutStill, req);
    req->port = (CARD32) port;
    req->drawable = (CARD32) d;
    req->gc = (CARD32) gc->gid;
    req->vid_x = (INT16) vx;
    req->vid_y = (INT16) vy;
    req->vid_w = (CARD16) vw;
    req->vid_h = (CARD16) vh;
    req->drw_x = (INT16) dx;
    req->drw_y = (INT16) dy;
    req->drw_w = (CARD16) dw;
    req->drw_h = (CARD16) dh;

    UnlockDisplay(dpy);
    SyncHandle();

    return Success;
}

int
XvGetVideo(
    Display *dpy,
    XvPortID port,
    Drawable d,
    GC gc,
    int vx, int vy,
    unsigned int vw, unsigned int vh,
    int dx, int dy,
    unsigned int dw, unsigned int dh)
{
    XExtDisplayInfo *info = xv_find_display(dpy);
    xvGetVideoReq *req;

    XvCheckExtension(dpy, info, XvBadExtension);

    LockDisplay(dpy);

    FlushGC(dpy, gc);

    XvGetReq(GetVideo, req);
    req->port = (CARD32) port;
    req->drawable = (CARD32) d;
    req->gc = (CARD32) gc->gid;
    req->vid_x = (INT16) vx;
    req->vid_y = (INT16) vy;
    req->vid_w = (CARD16) vw;
    req->vid_h = (CARD16) vh;
    req->drw_x = (INT16) dx;
    req->drw_y = (INT16) dy;
    req->drw_w = (CARD16) dw;
    req->drw_h = (CARD16) dh;

    UnlockDisplay(dpy);
    SyncHandle();

    return Success;
}

int
XvGetStill(
    Display *dpy,
    XvPortID port,
    Drawable d,
    GC gc,
    int vx, int vy,
    unsigned int vw, unsigned int vh,
    int dx, int dy,
    unsigned int dw, unsigned int dh)
{
    XExtDisplayInfo *info = xv_find_display(dpy);
    xvGetStillReq *req;

    XvCheckExtension(dpy, info, XvBadExtension);

    LockDisplay(dpy);

    FlushGC(dpy, gc);

    XvGetReq(GetStill, req);
    req->port = (CARD32) port;
    req->drawable = (CARD32) d;
    req->gc = (CARD32) gc->gid;
    req->vid_x = (INT16) vx;
    req->vid_y = (INT16) vy;
    req->vid_w = (CARD16) vw;
    req->vid_h = (CARD16) vh;
    req->drw_x = (INT16) dx;
    req->drw_y = (INT16) dy;
    req->drw_w = (CARD16) dw;
    req->drw_h = (CARD16) dh;

    UnlockDisplay(dpy);
    SyncHandle();

    return Success;
}

int
XvStopVideo(Display *dpy, XvPortID port, Drawable draw)
{
    XExtDisplayInfo *info = xv_find_display(dpy);
    xvStopVideoReq *req;

    XvCheckExtension(dpy, info, XvBadExtension);

    LockDisplay(dpy);

    XvGetReq(StopVideo, req);
    req->port = (CARD32) port;
    req->drawable = (CARD32) draw;

    UnlockDisplay(dpy);
    SyncHandle();

    return Success;
}

int
XvGrabPort(Display *dpy, XvPortID port, Time time)
{
    XExtDisplayInfo *info = xv_find_display(dpy);
    int result;
    xvGrabPortReply rep;
    xvGrabPortReq *req;

    XvCheckExtension(dpy, info, XvBadExtension);

    LockDisplay(dpy);

    XvGetReq(GrabPort, req);
    req->port = (CARD32) port;
    req->time = (CARD32) time;

    if (_XReply(dpy, (xReply *) &rep, 0, xTrue) == 0)
        rep.result = GrabSuccess;

    result = rep.result;

    UnlockDisplay(dpy);
    SyncHandle();

    return result;
}

int
XvUngrabPort(Display *dpy, XvPortID port, Time time)
{
    XExtDisplayInfo *info = xv_find_display(dpy);
    xvUngrabPortReq *req;

    XvCheckExtension(dpy, info, XvBadExtension);

    LockDisplay(dpy);

    XvGetReq(UngrabPort, req);
    req->port = (CARD32) port;
    req->time = (CARD32) time;

    UnlockDisplay(dpy);
    SyncHandle();

    return Success;
}

int
XvSelectVideoNotify(Display *dpy, Drawable drawable, Bool onoff)
{
    XExtDisplayInfo *info = xv_find_display(dpy);
    xvSelectVideoNotifyReq *req;

    XvCheckExtension(dpy, info, XvBadExtension);

    LockDisplay(dpy);

    XvGetReq(SelectVideoNotify, req);
    req->drawable = (CARD32) drawable;
    req->onoff = (BOOL) onoff;

    UnlockDisplay(dpy);
    SyncHandle();

    return Success;
}

int
XvSelectPortNotify(Display *dpy, XvPortID port, Bool onoff)
{
    XExtDisplayInfo *info = xv_find_display(dpy);
    xvSelectPortNotifyReq *req;

    XvCheckExtension(dpy, info, XvBadExtension);

    LockDisplay(dpy);

    XvGetReq(SelectPortNotify, req);
    req->port = (CARD32) port;
    req->onoff = (BOOL) onoff;

    UnlockDisplay(dpy);
    SyncHandle();

    return Success;
}

int
XvSetPortAttribute(Display *dpy, XvPortID port, Atom attribute, int value)
{
    XExtDisplayInfo *info = xv_find_display(dpy);
    xvSetPortAttributeReq *req;

    XvCheckExtension(dpy, info, XvBadExtension);

    LockDisplay(dpy);

    XvGetReq(SetPortAttribute, req);
    req->port = (CARD32) port;
    req->attribute = (CARD32) attribute;
    req->value = value;

    UnlockDisplay(dpy);
    SyncHandle();

    return (Success);
}

int
XvGetPortAttribute(Display *dpy, XvPortID port, Atom attribute, int *p_value)
{
    XExtDisplayInfo *info = xv_find_display(dpy);
    xvGetPortAttributeReq *req;
    xvGetPortAttributeReply rep;
    int status;

    XvCheckExtension(dpy, info, XvBadExtension);

    LockDisplay(dpy);

    XvGetReq(GetPortAttribute, req);
    req->port = (CARD32) port;
    req->attribute = (CARD32) attribute;

    /* READ THE REPLY */

    if (_XReply(dpy, (xReply *) &rep, 0, xFalse) == 0) {
        status = XvBadReply;
    }
    else {
        *p_value = rep.value;
        status = Success;
    }

    UnlockDisplay(dpy);
    SyncHandle();

    return status;
}

int
XvQueryBestSize(
    Display *dpy,
    XvPortID port,
    Bool motion,
    unsigned int vid_w,
    unsigned int vid_h,
    unsigned int drw_w,
    unsigned int drw_h,
    unsigned int *p_actual_width,
    unsigned int *p_actual_height)
{
    XExtDisplayInfo *info = xv_find_display(dpy);
    xvQueryBestSizeReq *req;
    xvQueryBestSizeReply rep;
    int status;

    XvCheckExtension(dpy, info, XvBadExtension);

    LockDisplay(dpy);

    XvGetReq(QueryBestSize, req);
    req->port = (CARD32) port;
    req->motion = (CARD8) motion;
    req->vid_w = (CARD16) vid_w;
    req->vid_h = (CARD16) vid_h;
    req->drw_w = (CARD16) drw_w;
    req->drw_h = (CARD16) drw_h;

    /* READ THE REPLY */

    if (_XReply(dpy, (xReply *) &rep, 0, xFalse) == 0) {
        status = XvBadReply;
    }
    else {
        *p_actual_width = rep.actual_width;
        *p_actual_height = rep.actual_height;
        status = Success;
    }

    UnlockDisplay(dpy);
    SyncHandle();

    return status;
}


XvAttribute *
XvQueryPortAttributes(Display *dpy, XvPortID port, int *num)
{
    XExtDisplayInfo *info = xv_find_display(dpy);
    xvQueryPortAttributesReq *req;
    xvQueryPortAttributesReply rep;
    XvAttribute *ret = NULL;

    *num = 0;

    XvCheckExtension(dpy, info, NULL);

    LockDisplay(dpy);

    XvGetReq(QueryPortAttributes, req);
    req->port = (CARD32) port;

    /* READ THE REPLY */

    if (_XReply(dpy, (xReply *) &rep, 0, xFalse) == 0) {
        goto out;
    }

    /*
     * X server sends data packed as:
     *   attribute1, name1, attribute2, name2, ...
     * We allocate a single buffer large enough to hold them all and
     * then de-interleave the data so we return it to clients as:
     *   attribute1, attribute2, ..., name1, name2, ...
     * so that clients may refer to attributes as a simple array of
     * structs:  attributes[0], attributes[1], ...
     * and free it as a single/simple buffer.
     */

    if (rep.num_attributes) {
        /* limit each part to no more than one half the max size */
        if ((rep.num_attributes < ((INT_MAX / 2) / sizeof(XvAttribute))) &&
            (rep.text_size < (INT_MAX / 2) - 1)) {
            unsigned long size;

            size = (rep.num_attributes * sizeof(XvAttribute)) +
		rep.text_size + 1;
            ret = Xmalloc(size);
        }

        if (ret != NULL) {
            char *marker = (char *) (&ret[rep.num_attributes]);
            xvAttributeInfo Info;

            /* keep track of remaining room for text strings */
            unsigned long size = rep.text_size;

            for (unsigned int i = 0; i < rep.num_attributes; i++) {
                _XRead(dpy, (char *) (&Info), sz_xvAttributeInfo);
                ret[i].flags = (int) Info.flags;
                ret[i].min_value = Info.min;
                ret[i].max_value = Info.max;
                ret[i].name = marker;
                if (Info.size <= size) {
                    _XRead(dpy, marker, Info.size);
                    marker += Info.size;
                    size -= Info.size;
                }
                (*num)++;
            }

            /* ensure final string is nil-terminated to avoid exposure of
               uninitialized memory */
            *marker = '\0';
        }
        else
            _XEatDataWords(dpy, rep.length);
    }

  out:
    UnlockDisplay(dpy);
    SyncHandle();

    return ret;
}

XvImageFormatValues *
XvListImageFormats(Display *dpy, XvPortID port, int *num)
{
    XExtDisplayInfo *info = xv_find_display(dpy);
    xvListImageFormatsReq *req;
    xvListImageFormatsReply rep;
    XvImageFormatValues *ret = NULL;

    *num = 0;

    XvCheckExtension(dpy, info, NULL);

    LockDisplay(dpy);

    XvGetReq(ListImageFormats, req);
    req->port = (CARD32) port;

    /* READ THE REPLY */

    if (_XReply(dpy, (xReply *) &rep, 0, xFalse) == 0) {
        goto out;
    }

    if (rep.num_formats) {
        if (rep.num_formats < (INT_MAX / sizeof(XvImageFormatValues)))
            ret = Xmalloc(rep.num_formats * sizeof(XvImageFormatValues));

        if (ret != NULL) {
            for (unsigned int i = 0; i < rep.num_formats; i++) {
                xvImageFormatInfo Info;

                _XRead(dpy, (char *) (&Info), sz_xvImageFormatInfo);
                ret[i].id = (int) Info.id;
                ret[i].type = Info.type;
                ret[i].byte_order = Info.byte_order;
                memcpy(&(ret[i].guid[0]), &(Info.guid[0]), 16);
                ret[i].bits_per_pixel = Info.bpp;
                ret[i].format = Info.format;
                ret[i].num_planes = Info.num_planes;
                ret[i].depth = Info.depth;
                ret[i].red_mask = Info.red_mask;
                ret[i].green_mask = Info.green_mask;
                ret[i].blue_mask = Info.blue_mask;
                ret[i].y_sample_bits = Info.y_sample_bits;
                ret[i].u_sample_bits = Info.u_sample_bits;
                ret[i].v_sample_bits = Info.v_sample_bits;
                ret[i].horz_y_period = Info.horz_y_period;
                ret[i].horz_u_period = Info.horz_u_period;
                ret[i].horz_v_period = Info.horz_v_period;
                ret[i].vert_y_period = Info.vert_y_period;
                ret[i].vert_u_period = Info.vert_u_period;
                ret[i].vert_v_period = Info.vert_v_period;
                memcpy(&(ret[i].component_order[0]), &(Info.comp_order[0]), 32);
                ret[i].scanline_order = Info.scanline_order;
                (*num)++;
            }
        }
        else
            _XEatDataWords(dpy, rep.length);
    }

  out:
    UnlockDisplay(dpy);
    SyncHandle();

    return ret;
}

XvImage *
XvCreateImage(
    Display *dpy,
    XvPortID port,
    int id,
    char *data,
    int width,
    int height)
{
    XExtDisplayInfo *info = xv_find_display(dpy);
    xvQueryImageAttributesReq *req;
    xvQueryImageAttributesReply rep;
    XvImage *ret = NULL;

    XvCheckExtension(dpy, info, NULL);

    LockDisplay(dpy);

    XvGetReq(QueryImageAttributes, req);
    req->id = (CARD32) id;
    req->port = (CARD32) port;
    req->width = (CARD16) width;
    req->height = (CARD16) height;

    /* READ THE REPLY */

    if (!_XReply(dpy, (xReply *) &rep, 0, xFalse)) {
        goto out;
    }

    if (rep.num_planes < ((INT_MAX >> 3) - sizeof(XvImage)))
        ret = Xmalloc(sizeof(XvImage) + (rep.num_planes << 3));

    if (ret != NULL) {
        ret->id = id;
        ret->width = rep.width;
        ret->height = rep.height;
        ret->data_size = (int) rep.data_size;
        ret->num_planes = (int) rep.num_planes;
        ret->pitches = (int *) (&ret[1]);
        ret->offsets = ret->pitches + rep.num_planes;
        ret->data = data;
        ret->obdata = NULL;
        _XRead(dpy, (char *) (ret->pitches), rep.num_planes << 2);
        _XRead(dpy, (char *) (ret->offsets), rep.num_planes << 2);
    }
    else
        _XEatDataWords(dpy, rep.length);

  out:
    UnlockDisplay(dpy);
    SyncHandle();

    return ret;
}

XvImage *
XvShmCreateImage(
    Display *dpy,
    XvPortID port,
    int id,
    char *data,
    int width,
    int height,
    XShmSegmentInfo *shminfo)
{
    XvImage *ret;

    ret = XvCreateImage(dpy, port, id, data, width, height);

    if (ret)
        ret->obdata = (XPointer) shminfo;

    return ret;
}

int
XvPutImage(
    Display *dpy,
    XvPortID port,
    Drawable d,
    GC gc,
    XvImage *image,
    int src_x, int src_y,
    unsigned int src_w, unsigned int src_h,
    int dest_x, int dest_y,
    unsigned int dest_w, unsigned int dest_h)
{
    XExtDisplayInfo *info = xv_find_display(dpy);
    xvPutImageReq *req;
    unsigned int len;

    XvCheckExtension(dpy, info, XvBadExtension);

    LockDisplay(dpy);

    FlushGC(dpy, gc);

    XvGetReq(PutImage, req);

    req->port = (CARD32) port;
    req->drawable = (CARD32) d;
    req->gc = (CARD32) gc->gid;
    req->id = (CARD32) image->id;
    req->src_x = (INT16) src_x;
    req->src_y = (INT16) src_y;
    req->src_w = (CARD16) src_w;
    req->src_h = (CARD16) src_h;
    req->drw_x = (INT16) dest_x;
    req->drw_y = (INT16) dest_y;
    req->drw_w = (CARD16) dest_w;
    req->drw_h = (CARD16) dest_h;
    req->width = (CARD16) image->width;
    req->height = (CARD16) image->height;

    len = ((unsigned int) image->data_size + 3) >> 2;
    SetReqLen(req, len, len);

    /* Yes it's kindof lame that we are sending the whole thing,
       but for video all of it may be needed even if displaying
       only a subsection, and I don't want to go through the
       trouble of creating subregions to send */
    Data(dpy, (char *) image->data, image->data_size);

    UnlockDisplay(dpy);
    SyncHandle();

    return Success;
}

int
XvShmPutImage(
    Display *dpy,
    XvPortID port,
    Drawable d,
    GC gc,
    XvImage *image,
    int src_x, int src_y,
    unsigned int src_w, unsigned int src_h,
    int dest_x, int dest_y,
    unsigned int dest_w, unsigned int dest_h,
    Bool send_event)
{
    XExtDisplayInfo *info = xv_find_display(dpy);
    XShmSegmentInfo *shminfo = (XShmSegmentInfo *) image->obdata;
    xvShmPutImageReq *req;

    XvCheckExtension(dpy, info, XvBadExtension);

    LockDisplay(dpy);

    FlushGC(dpy, gc);

    XvGetReq(ShmPutImage, req);

    req->port = (CARD32) port;
    req->drawable = (CARD32) d;
    req->gc = (CARD32) gc->gid;
    req->shmseg = (CARD32) shminfo->shmseg;
    req->id = (CARD32) image->id;
    req->src_x = (INT16) src_x;
    req->src_y = (INT16) src_y;
    req->src_w = (CARD16) src_w;
    req->src_h = (CARD16) src_h;
    req->drw_x = (INT16) dest_x;
    req->drw_y = (INT16) dest_y;
    req->drw_w = (CARD16) dest_w;
    req->drw_h = (CARD16) dest_h;
    req->offset = (CARD32) (image->data - shminfo->shmaddr);
    req->width = (CARD16) image->width;
    req->height = (CARD16) image->height;
    req->send_event = (CARD8) send_event;

    UnlockDisplay(dpy);
    SyncHandle();

    return Success;
}


static Bool
xv_wire_to_event(Display *dpy, XEvent *host, xEvent *wire)
{
    XExtDisplayInfo *info = xv_find_display(dpy);
    XvEvent *re = (XvEvent *) host;
    xvEvent *event = (xvEvent *) wire;

    XvCheckExtension(dpy, info, False);

    switch ((event->u.u.type & 0x7F) - info->codes->first_event) {
    case XvVideoNotify:
        re->xvvideo.type = event->u.u.type & 0x7f;
        re->xvvideo.serial = _XSetLastRequestRead(dpy, (xGenericReply *) event);
        re->xvvideo.send_event = ((event->u.u.type & 0x80) != 0);
        re->xvvideo.display = dpy;
        re->xvvideo.time = event->u.videoNotify.time;
        re->xvvideo.reason = event->u.videoNotify.reason;
        re->xvvideo.drawable = event->u.videoNotify.drawable;
        re->xvvideo.port_id = event->u.videoNotify.port;
        break;
    case XvPortNotify:
        re->xvport.type = event->u.u.type & 0x7f;
        re->xvport.serial = _XSetLastRequestRead(dpy, (xGenericReply *) event);
        re->xvport.send_event = ((event->u.u.type & 0x80) != 0);
        re->xvport.display = dpy;
        re->xvport.time = event->u.portNotify.time;
        re->xvport.port_id = event->u.portNotify.port;
        re->xvport.attribute = event->u.portNotify.attribute;
        re->xvport.value = event->u.portNotify.value;
        break;
    default:
        return False;
    }

    return (True);
}

/*
 * Copyright © 2006 Keith Packard
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that copyright
 * notice and this permission notice appear in supporting documentation, and
 * that the name of the copyright holders not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  The copyright holders make no representations
 * about the suitability of this software for any purpose.  It is provided "as
 * is" without express or implied warranty.
 *
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
 * OF THIS SOFTWARE.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <X11/Xlib.h>
/* we need to be able to manipulate the Display structure on events */
#include <X11/Xlibint.h>
#include <X11/extensions/render.h>
#include <X11/extensions/Xrender.h>
#include "Xrandrint.h"
#include <limits.h>

Atom *
XRRListProviderProperties (Display *dpy, RRProvider provider, int *nprop)
{
    XExtDisplayInfo		*info = XRRFindDisplay(dpy);
    xRRListProviderPropertiesReply rep;
    xRRListProviderPropertiesReq	*req;
    Atom			*props = NULL;

    RRCheckExtension (dpy, info, NULL);

    LockDisplay (dpy);
    GetReq (RRListProviderProperties, req);
    req->reqType = info->codes->major_opcode;
    req->randrReqType = X_RRListProviderProperties;
    req->provider = provider;

    if (!_XReply (dpy, (xReply *) &rep, 0, xFalse)) {
	UnlockDisplay (dpy);
	SyncHandle ();
	*nprop = 0;
	return NULL;
    }

    if (rep.nAtoms) {
	size_t rbytes = rep.nAtoms * sizeof (Atom);
	size_t nbytes = rep.nAtoms << 2;

	props = Xmalloc (rbytes);
	if (props == NULL) {
	    _XEatDataWords (dpy, rep.length);
	    UnlockDisplay (dpy);
	    SyncHandle ();
	    *nprop = 0;
	    return NULL;
	}

	_XRead32 (dpy, (long *) props, nbytes);
    }

    *nprop = rep.nAtoms;
    UnlockDisplay (dpy);
    SyncHandle ();
    return props;
}

XRRPropertyInfo *
XRRQueryProviderProperty (Display *dpy, RRProvider provider, Atom property)
{
    XExtDisplayInfo		*info = XRRFindDisplay(dpy);
    xRRQueryProviderPropertyReply rep;
    xRRQueryProviderPropertyReq	*req;
    unsigned int		nbytes;
    XRRPropertyInfo		*prop_info;

    RRCheckExtension (dpy, info, NULL);

    LockDisplay (dpy);
    GetReq (RRQueryProviderProperty, req);
    req->reqType = info->codes->major_opcode;
    req->randrReqType = X_RRQueryProviderProperty;
    req->provider = provider;
    req->property = property;

    if (!_XReply (dpy, (xReply *) &rep, 0, xFalse)) {
	UnlockDisplay (dpy);
	SyncHandle ();
	return NULL;
    }

    if (rep.length < ((INT_MAX / sizeof(long)) - sizeof (XRRPropertyInfo))) {
        size_t rbytes = sizeof (XRRPropertyInfo) + (rep.length * sizeof (long));
        nbytes = rep.length << 2;

        prop_info = Xmalloc (rbytes);
    } else
        prop_info = NULL;

    if (prop_info == NULL) {
	_XEatDataWords (dpy, rep.length);
	UnlockDisplay (dpy);
	SyncHandle ();
	return NULL;
    }

    prop_info->pending = rep.pending;
    prop_info->range = rep.range;
    prop_info->immutable = rep.immutable;
    prop_info->num_values = rep.length;
    if (rep.length != 0) {
	prop_info->values = (long *) (prop_info + 1);
	_XRead32 (dpy, prop_info->values, nbytes);
    } else {
	prop_info->values = NULL;
    }

    UnlockDisplay (dpy);
    SyncHandle ();
    return prop_info;
}

void
XRRConfigureProviderProperty (Display *dpy, RRProvider provider, Atom property,
			    Bool pending, Bool range, int num_values,
			    long *values)
{
    XExtDisplayInfo		    *info = XRRFindDisplay(dpy);
    xRRConfigureProviderPropertyReq   *req;
    long len;

    RRSimpleCheckExtension (dpy, info);

    LockDisplay(dpy);
    GetReq (RRConfigureProviderProperty, req);
    req->reqType = info->codes->major_opcode;
    req->randrReqType = X_RRConfigureProviderProperty;
    req->provider = provider;
    req->property = property;
    req->pending = pending;
    req->range = range;

    len = num_values;
    if (dpy->bigreq_size || req->length + len <= (unsigned) 65535) {
	SetReqLen(req, len, len);
	len = (long)num_values << 2;
	Data32 (dpy, values, len);
    } /* else force BadLength */

    UnlockDisplay(dpy);
    SyncHandle();
}
			
void
XRRChangeProviderProperty (Display *dpy, RRProvider provider,
			 Atom property, Atom type,
			 int format, int mode,
			 _Xconst unsigned char *data, int nelements)
{
    XExtDisplayInfo		*info = XRRFindDisplay(dpy);
    xRRChangeProviderPropertyReq	*req;
    long len;

    RRSimpleCheckExtension (dpy, info);

    LockDisplay(dpy);
    GetReq (RRChangeProviderProperty, req);
    req->reqType = info->codes->major_opcode;
    req->randrReqType = X_RRChangeProviderProperty;
    req->provider = provider;
    req->property = property;
    req->type = type;
    req->mode = mode;
    if (nelements < 0) {
	req->nUnits = 0;
	req->format = 0; /* ask for garbage, get garbage */
    } else {
	req->nUnits = nelements;
	req->format = format;
    }

    switch (req->format) {
    case 8:
	len = ((long)nelements + 3) >> 2;
	if (dpy->bigreq_size || req->length + len <= (unsigned) 65535) {
	    SetReqLen(req, len, len);
	    Data (dpy, (char *)data, nelements);
	} /* else force BadLength */
	break;

    case 16:
	len = ((long)nelements + 1) >> 1;
	if (dpy->bigreq_size || req->length + len <= (unsigned) 65535) {
	    SetReqLen(req, len, len);
	    len = (long)nelements << 1;
	    Data16 (dpy, (short *) data, len);
	} /* else force BadLength */
	break;

    case 32:
	len = nelements;
	if (dpy->bigreq_size || req->length + len <= (unsigned) 65535) {
	    SetReqLen(req, len, len);
	    len = (long)nelements << 2;
	    Data32 (dpy, (long *) data, len);
	} /* else force BadLength */
	break;

    default:
	/* BadValue will be generated */ ;
    }

    UnlockDisplay(dpy);
    SyncHandle();
}

void
XRRDeleteProviderProperty (Display *dpy, RRProvider provider, Atom property)
{
    XExtDisplayInfo		*info = XRRFindDisplay(dpy);
    xRRDeleteProviderPropertyReq *req;

    RRSimpleCheckExtension (dpy, info);

    LockDisplay(dpy);
    GetReq(RRDeleteProviderProperty, req);
    req->reqType = info->codes->major_opcode;
    req->randrReqType = X_RRDeleteProviderProperty;
    req->provider = provider;
    req->property = property;
    UnlockDisplay(dpy);
    SyncHandle();
}

int
XRRGetProviderProperty (Display *dpy, RRProvider provider,
		      Atom property, long offset, long length,
		      Bool delete, Bool pending, Atom req_type, 
		      Atom *actual_type, int *actual_format,
		      unsigned long *nitems, unsigned long *bytes_after,
		      unsigned char **prop)
{
    XExtDisplayInfo		*info = XRRFindDisplay(dpy);
    xRRGetProviderPropertyReply	rep;
    xRRGetProviderPropertyReq	*req;

    /* Always initialize return values, in case callers fail to initialize
       them and fail to check the return code for an error. */
    *actual_type = None;
    *actual_format = 0;
    *nitems = *bytes_after = 0L;
    *prop = (unsigned char *) NULL;

    RRCheckExtension (dpy, info, 1);

    LockDisplay (dpy);
    GetReq (RRGetProviderProperty, req);
    req->reqType = info->codes->major_opcode;
    req->randrReqType = X_RRGetProviderProperty;
    req->provider = provider;
    req->property = property;
    req->type = req_type;
    req->longOffset = offset;
    req->longLength = length;
    req->delete = delete;
    req->pending = pending;

    if (!_XReply (dpy, (xReply *) &rep, 0, xFalse))
    {
	UnlockDisplay (dpy);
	SyncHandle ();
	return ((xError *)&rep)->errorCode;
    }

    if (rep.propertyType != None) {
	int format = rep.format;
	unsigned long		nbytes, rbytes;

	/*
	 * Protect against both integer overflow and just plain oversized
	 * memory allocation - no server should ever return this many props.
	 */
	if (rep.nItems >= (INT_MAX >> 4))
	    format = -1;        /* fall through to default error case */

	/*
	 * One extra byte is malloced than is needed to contain the property
	 * data, but this last byte is null terminated and convenient for
	 * returning string properties, so the client doesn't then have to
	 * recopy the string to make it null terminated.
	 */
	switch (format) {
	case 8:
	    nbytes = rep.nItems;
	    rbytes = rep.nItems + 1;
	    if (rbytes > 0 && (*prop = Xmalloc (rbytes)))
		_XReadPad (dpy, (char *) *prop, nbytes);
	    break;

	case 16:
	    nbytes = rep.nItems << 1;
	    rbytes = rep.nItems * sizeof (short) + 1;
	    if (rbytes > 0 && (*prop = Xmalloc (rbytes)))
		_XRead16Pad (dpy, (short *) *prop, nbytes);
	    break;

	case 32:
	    nbytes = rep.nItems << 2;
	    rbytes = rep.nItems * sizeof (long) + 1;
	    if (rbytes > 0 && (*prop = Xmalloc (rbytes)))
		_XRead32 (dpy, (long *) *prop, nbytes);
	    break;

	default:
	    /*
	     * This part of the code should never be reached.  If it is,
	     * the server sent back a property with an invalid format.
	     */
	    _XEatDataWords(dpy, rep.length);
	    UnlockDisplay(dpy);
	    SyncHandle();
	    return(BadImplementation);
	}
	if (! *prop) {
	    _XEatDataWords(dpy, rep.length);
	    UnlockDisplay(dpy);
	    SyncHandle();
	    return(BadAlloc);
	}
	(*prop)[rbytes - 1] = '\0';
    }

    *actual_type = rep.propertyType;
    *actual_format = rep.format;
    *nitems = rep.nItems;
    *bytes_after = rep.bytesAfter;
    UnlockDisplay (dpy);
    SyncHandle ();

    return Success;
}

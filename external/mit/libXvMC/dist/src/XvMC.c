#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <stdio.h>
#include "XvMClibint.h"
#ifdef HAVE_SHMAT
#ifndef Lynx
#include <sys/ipc.h>
#include <sys/shm.h>
#else
#include <ipc.h>
#include <shm.h>
#endif /* Lynx */
#endif /* HAVE_SHMAT */
#include <unistd.h>
#include <sys/time.h>
#include <X11/extensions/Xext.h>
#include <X11/extensions/extutil.h>
#include <limits.h>

static XExtensionInfo _xvmc_info_data;
static XExtensionInfo *xvmc_info = &_xvmc_info_data;
static const char *xvmc_extension_name = XvMCName;

static const char *xvmc_error_list[] =
{
   "BadContext",
   "BadSurface",
   "BadSubpicture"
};

static XEXT_GENERATE_CLOSE_DISPLAY (xvmc_close_display, xvmc_info)


static XEXT_GENERATE_ERROR_STRING (xvmc_error_string, xvmc_extension_name,
                                   XvMCNumErrors, xvmc_error_list)


static XExtensionHooks xvmc_extension_hooks = {
    NULL,                               /* create_gc */
    NULL,                               /* copy_gc */
    NULL,                               /* flush_gc */
    NULL,                               /* free_gc */
    NULL,                               /* create_font */
    NULL,                               /* free_font */
    xvmc_close_display,                 /* close_display */
    NULL,                               /* wire_to_event */
    NULL,                               /* event_to_wire */
    NULL,                               /* error */
    xvmc_error_string                   /* error_string */
};

static XEXT_GENERATE_FIND_DISPLAY (xvmc_find_display, xvmc_info,
                                   xvmc_extension_name,
                                   &xvmc_extension_hooks,
                                   XvMCNumEvents, NULL)

Bool XvMCQueryExtension (Display *dpy, int *event_basep, int *error_basep)
{
    XExtDisplayInfo *info = xvmc_find_display(dpy);

    if (XextHasExtension(info)) {
        *event_basep = info->codes->first_event;
        *error_basep = info->codes->first_error;
        return True;
    } else {
        return False;
    }
}

Status XvMCQueryVersion (Display *dpy, int *major, int *minor)
{
    XExtDisplayInfo *info = xvmc_find_display(dpy);
    xvmcQueryVersionReply rep;
    xvmcQueryVersionReq  *req;

    XvMCCheckExtension (dpy, info, BadImplementation);

    LockDisplay (dpy);
    XvMCGetReq (QueryVersion, req);
    if (!_XReply (dpy, (xReply *) &rep, 0, xTrue)) {
        UnlockDisplay (dpy);
        SyncHandle ();
        return BadImplementation;
    }
    *major = rep.major;
    *minor = rep.minor;
    UnlockDisplay (dpy);
    SyncHandle ();
    return Success;
}


XvMCSurfaceInfo * XvMCListSurfaceTypes(Display *dpy, XvPortID port, int *num)
{
    XExtDisplayInfo *info = xvmc_find_display(dpy);
    xvmcListSurfaceTypesReply rep;
    xvmcListSurfaceTypesReq  *req;
    XvMCSurfaceInfo *surface_info = NULL;

    *num = 0;

    XvMCCheckExtension (dpy, info, NULL);

    LockDisplay (dpy);
    XvMCGetReq (ListSurfaceTypes, req);
    req->port = port;
    if (!_XReply (dpy, (xReply *) &rep, 0, xFalse)) {
        UnlockDisplay (dpy);
        SyncHandle ();
        return NULL;
    }

    if(rep.num > 0) {
        if (rep.num < (INT_MAX / sizeof(XvMCSurfaceInfo)))
            surface_info = Xmalloc(rep.num * sizeof(XvMCSurfaceInfo));

        if(surface_info) {
	    xvmcSurfaceInfo sinfo;
	    CARD32 i;

	    *num = rep.num;

	    for(i = 0; i < rep.num; i++) {
		_XRead(dpy, (char*)&sinfo, sizeof(xvmcSurfaceInfo));
	       surface_info[i].surface_type_id = sinfo.surface_type_id;
	       surface_info[i].chroma_format = sinfo.chroma_format;
	       surface_info[i].max_width = sinfo.max_width;
	       surface_info[i].max_height = sinfo.max_height;
	       surface_info[i].subpicture_max_width =
					sinfo.subpicture_max_width;
	       surface_info[i].subpicture_max_height =
					sinfo.subpicture_max_height;
	       surface_info[i].mc_type = sinfo.mc_type;
	       surface_info[i].flags = sinfo.flags;
	    }
	} else
	   _XEatDataWords(dpy, rep.length);
    }

    UnlockDisplay (dpy);
    SyncHandle ();
    return surface_info;
}


XvImageFormatValues * XvMCListSubpictureTypes (
  Display * dpy,
  XvPortID port,
  int surface_type_id,
  int *count_return
)
{
    XExtDisplayInfo *info = xvmc_find_display(dpy);
    xvmcListSubpictureTypesReply rep;
    xvmcListSubpictureTypesReq  *req;
    XvImageFormatValues *ret = NULL;


    *count_return = 0;

    XvMCCheckExtension (dpy, info, NULL);


    LockDisplay (dpy);
    XvMCGetReq (ListSubpictureTypes, req);
    req->port = port;
    req->surface_type_id = surface_type_id;
    if (!_XReply (dpy, (xReply *) &rep, 0, xFalse)) {
        UnlockDisplay (dpy);
        SyncHandle ();
        return NULL;
    }

    if(rep.num > 0) {
        if (rep.num < (INT_MAX / sizeof(XvImageFormatValues)))
            ret = Xmalloc(rep.num * sizeof(XvImageFormatValues));

        if(ret) {
            xvImageFormatInfo Info;
            CARD32 i;

            *count_return = rep.num;

            for(i = 0; i < rep.num; i++) {
              _XRead(dpy, (char*)(&Info), sz_xvImageFormatInfo);
              ret[i].id = Info.id;
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
            }
        } else
	   _XEatDataWords(dpy, rep.length);
    }

    UnlockDisplay (dpy);
    SyncHandle ();
    return ret;
}


/******************************************************************
   These are intended as a protocol interface to be used by direct
   rendering libraries.  They are not intended to be client viewable
   functions.  These will stay in place until we have a mechanism in
   place similar to that of OpenGL with an libXvMCcore library.
*******************************************************************/

/*
   _xvmc_create_context -

   Pass in the context with the surface_type_id, width, height,
   port and flags filled out.  This function will fill out the
   context_id and update the width, height and flags field.
   The server may return implementation-specific information
   back in the priv_data.  The size of that information will
   an array of priv_count CARD32s.  This data is allocated by
   this function.  If returned, the caller is responsible for
   freeing it!  Generally, such information is only returned if
   an XVMC_DIRECT context was specified.
*/


Status _xvmc_create_context (
    Display *dpy,
    XvMCContext *context,
    int *priv_count,
    CARD32 **priv_data
)
{
    XExtDisplayInfo *info = xvmc_find_display(dpy);
    xvmcCreateContextReply rep;
    xvmcCreateContextReq  *req;

    *priv_count = 0;
    *priv_data = NULL;

    XvMCCheckExtension (dpy, info, BadImplementation);

    LockDisplay (dpy);
    XvMCGetReq (CreateContext, req);
    context->context_id = XAllocID(dpy);
    req->context_id = context->context_id;
    req->port = context->port;
    req->surface_type_id = context->surface_type_id;
    req->width = context->width;
    req->height = context->height;
    req->flags = context->flags;
    if (!_XReply (dpy, (xReply *) &rep, 0, xFalse)) {
        UnlockDisplay (dpy);
        SyncHandle ();
        return BadImplementation;
    }
    context->width = rep.width_actual;
    context->height = rep.height_actual;
    context->flags = rep.flags_return;

    if(rep.length) {
	if (rep.length < (INT_MAX >> 2))
	    *priv_data = Xmalloc(rep.length << 2);
	if(*priv_data) {
            _XRead(dpy, (char*)(*priv_data), rep.length << 2);
	    *priv_count = rep.length;
	} else
	    _XEatDataWords(dpy, rep.length);
    }

    UnlockDisplay (dpy);
    SyncHandle ();
    return Success;
}

Status _xvmc_destroy_context (
    Display *dpy,
    XvMCContext *context
)
{
    XExtDisplayInfo *info = xvmc_find_display(dpy);
    xvmcDestroyContextReq  *req;

    XvMCCheckExtension (dpy, info, BadImplementation);

    LockDisplay (dpy);
    XvMCGetReq (DestroyContext, req);
    req->context_id = context->context_id;
    UnlockDisplay (dpy);
    SyncHandle ();
    return Success;
}


/*
   _xvmc_create_surface -

   Pass the context and this function will fill out all the
   information in the surface.
   The server may return implementation-specific information
   back in the priv_data.  The size of that information will
   an array of priv_count CARD32s.  This data is allocated by
   this function.  If returned, the caller is responsible for
   freeing it!  Generally, such information is returned only if
   the context was a direct context.

*/

Status _xvmc_create_surface (
    Display *dpy,
    XvMCContext *context,
    XvMCSurface *surface,
    int *priv_count,
    CARD32 **priv_data
)
{
    XExtDisplayInfo *info = xvmc_find_display(dpy);
    xvmcCreateSurfaceReply rep;
    xvmcCreateSurfaceReq  *req;

    *priv_count = 0;
    *priv_data = NULL;

    XvMCCheckExtension (dpy, info, BadImplementation);

    LockDisplay (dpy);
    XvMCGetReq (CreateSurface, req);

    surface->surface_id = XAllocID(dpy);
    surface->context_id = context->context_id;
    surface->surface_type_id = context->surface_type_id;
    surface->width = context->width;
    surface->height = context->height;

    req->surface_id = surface->surface_id;
    req->context_id = surface->context_id;
    if (!_XReply (dpy, (xReply *) &rep, 0, xFalse)) {
        UnlockDisplay (dpy);
        SyncHandle ();
        return BadImplementation;
    }

    if(rep.length) {
        if (rep.length < (INT_MAX >> 2))
            *priv_data = Xmalloc(rep.length << 2);
        if(*priv_data) {
            _XRead(dpy, (char*)(*priv_data), rep.length << 2);
            *priv_count = rep.length;
        } else
            _XEatDataWords(dpy, rep.length);
    }

    UnlockDisplay (dpy);
    SyncHandle ();
    return Success;
}

Status _xvmc_destroy_surface (
    Display *dpy,
    XvMCSurface *surface
)
{
    XExtDisplayInfo *info = xvmc_find_display(dpy);
    xvmcDestroySurfaceReq  *req;

    XvMCCheckExtension (dpy, info, BadImplementation);

    LockDisplay (dpy);
    XvMCGetReq (DestroySurface, req);
    req->surface_id = surface->surface_id;
    UnlockDisplay (dpy);
    SyncHandle ();
    return Success;
}

/*
   _xvmc_create_subpicture -

   Pass the subpicture with the width, height and xvimage_id filled
   out and this function will fill out everything else in the
   subpicture as well as adjust the width and height if needed.
   The server may return implementation-specific information
   back in the priv_data.  The size of that information will
   an array of priv_count CARD32s.  This data is allocated by
   this function.  If returned, the caller is responsible for
   freeing it!  Generally, such information is returned only if
   the context was a direct context.

*/

Status _xvmc_create_subpicture (
    Display *dpy,
    XvMCContext *context,
    XvMCSubpicture *subpicture,
    int *priv_count,
    CARD32 **priv_data
)
{
    XExtDisplayInfo *info = xvmc_find_display(dpy);
    xvmcCreateSubpictureReply rep;
    xvmcCreateSubpictureReq  *req;

    *priv_count = 0;
    *priv_data = NULL;

    XvMCCheckExtension (dpy, info, BadImplementation);

    LockDisplay (dpy);
    XvMCGetReq (CreateSubpicture, req);

    subpicture->subpicture_id = XAllocID(dpy);
    subpicture->context_id = context->context_id;

    req->subpicture_id = subpicture->subpicture_id;
    req->context_id = subpicture->context_id;
    req->xvimage_id = subpicture->xvimage_id;
    req->width = subpicture->width;
    req->height = subpicture->height;
    if (!_XReply (dpy, (xReply *) &rep, 0, xFalse)) {
        UnlockDisplay (dpy);
        SyncHandle ();
        return BadImplementation;
    }

    subpicture->width = rep.width_actual;
    subpicture->height = rep.height_actual;
    subpicture->num_palette_entries = rep.num_palette_entries;
    subpicture->entry_bytes = rep.entry_bytes;
    subpicture->component_order[0] = rep.component_order[0];
    subpicture->component_order[1] = rep.component_order[1];
    subpicture->component_order[2] = rep.component_order[2];
    subpicture->component_order[3] = rep.component_order[3];

    if(rep.length) {
        if (rep.length < (INT_MAX >> 2))
            *priv_data = Xmalloc(rep.length << 2);
        if(*priv_data) {
            _XRead(dpy, (char*)(*priv_data), rep.length << 2);
            *priv_count = rep.length;
        } else
            _XEatDataWords(dpy, rep.length);
    }

    UnlockDisplay (dpy);
    SyncHandle ();
    return Success;
}

Status _xvmc_destroy_subpicture(
    Display *dpy,
    XvMCSubpicture *subpicture
)
{
    XExtDisplayInfo *info = xvmc_find_display(dpy);
    xvmcDestroySubpictureReq  *req;

    XvMCCheckExtension (dpy, info, BadImplementation);

    LockDisplay (dpy);
    XvMCGetReq (DestroySubpicture, req);
    req->subpicture_id = subpicture->subpicture_id;
    UnlockDisplay (dpy);
    SyncHandle ();
    return Success;
}

Status XvMCGetDRInfo(Display *dpy, XvPortID port,
		     char **name, char **busID,
		     int *major, int *minor,
		     int *patchLevel,
		     int *isLocal)
{
    XExtDisplayInfo *info = xvmc_find_display(dpy);
    xvmcGetDRInfoReply rep;
    xvmcGetDRInfoReq  *req;
    CARD32 magic;

#ifdef HAVE_SHMAT
    int shmKey;
    volatile CARD32 *shMem;
    struct timezone here;
    struct timeval now;
    here.tz_minuteswest = 0;
    here.tz_dsttime = 0;
#endif

    *name = NULL;
    *busID = NULL;

    XvMCCheckExtension (dpy, info, BadImplementation);

    LockDisplay (dpy);
    XvMCGetReq (GetDRInfo, req);

    req->port = port;
    magic = 0;
    req->magic = 0;
#ifdef HAVE_SHMAT
    shmKey = shmget(IPC_PRIVATE, 1024, IPC_CREAT | 0600);
    req->shmKey = (CARD32) shmKey;

    /*
     * We fill a shared memory page with a repetitive pattern. If the
     * X server can read this pattern, we probably have a local connection.
     * Note that we can trigger the remote X server to read any shared
     * page on the remote machine, so we shouldn't be able to guess and verify
     * any complicated data on those pages. That's the explanation of this
     * otherwise stupid-looking pattern algorithm.
     */

    if (shmKey >= 0) {
	shMem = (CARD32 *) shmat(shmKey, NULL, 0);
	shmctl(shmKey, IPC_RMID, NULL);
	if (shMem != (void *) -1) {

	    register volatile CARD32 *shMemC = shMem;
	    register int i;

	    gettimeofday( &now, &here);
	    magic = now.tv_usec & 0x000FFFFF;
	    req->magic = magic;
	    i = 1024 / sizeof(CARD32);
	    while(i--) {
	        *shMemC++ = magic;
	        magic = ~magic;
	    }
	} else {
	    req->shmKey = -1;
	    shmKey = -1;
	}
    }
#else
    req->shmKey = 0;
#endif
    if (!_XReply (dpy, (xReply *) &rep, 0, xFalse)) {
        UnlockDisplay (dpy);
        SyncHandle ();
#ifdef HAVE_SHMAT
	if (shmKey >= 0) {
	    shmdt( (const void *) shMem );
	}
#endif
        return -1;
    }
#ifdef HAVE_SHMAT
    if (shmKey >= 0) {
        shmdt( (const void *) shMem );
    }
#endif

    if (rep.length > 0) {
	unsigned long realSize = 0;
	char *tmpBuf = NULL;

	if ((rep.length < (INT_MAX >> 2)) &&
	    /* protect against overflow in strncpy below */
	    (rep.nameLen + rep.busIDLen > rep.nameLen)) {
	    realSize = rep.length << 2;
	    if (realSize >= (rep.nameLen + rep.busIDLen)) {
		tmpBuf = Xmalloc(realSize);
		*name = Xmalloc(rep.nameLen);
		*busID = Xmalloc(rep.busIDLen);
	    }
	}

	if (*name && *busID && tmpBuf) {
	    _XRead(dpy, tmpBuf, realSize);
	    strncpy(*name,tmpBuf,rep.nameLen);
	    (*name)[rep.nameLen == 0 ? 0 : rep.nameLen - 1] = '\0';
	    strncpy(*busID,tmpBuf+rep.nameLen,rep.busIDLen);
	    (*busID)[rep.busIDLen == 0 ? 0 : rep.busIDLen - 1] = '\0';
	    XFree(tmpBuf);
	} else {
	    XFree(*name);
	    *name = NULL;
	    XFree(*busID);
	    *busID = NULL;
	    XFree(tmpBuf);

	    _XEatDataWords(dpy, rep.length);
	    UnlockDisplay (dpy);
	    SyncHandle ();
	    return -1;

	}
    }

    UnlockDisplay (dpy);
    SyncHandle ();
    *major = rep.major;
    *minor = rep.minor;
    *patchLevel = rep.patchLevel;
#ifdef HAVE_SHMAT
    if (shmKey >= 0)
        *isLocal = rep.isLocal;
    else
#endif
        *isLocal = 1;
    return (rep.length > 0) ? Success : BadImplementation;
}


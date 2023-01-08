/*

Copyright (c) 1995  Jon Tombs
Copyright (c) 1995,1996  The XFree86 Project, Inc

*/

/* THIS IS NOT AN X CONSORTIUM STANDARD */
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif


#include <X11/Xlibint.h>
#include <X11/extensions/Xxf86dga.h>
#include <X11/extensions/xf86dgaproto.h>
#include <X11/extensions/Xext.h>
#include <X11/extensions/extutil.h>
#include <stdio.h>

#include <stdint.h>
#include <limits.h>

/* If you change this, change the Bases[] array below as well */
#define MAX_HEADS 16

const char *xdga_extension_name = XF86DGANAME;

static XExtensionInfo _xdga_info_data;
static XExtensionInfo *xdga_info = &_xdga_info_data;


Bool XDGAMapFramebuffer(int, char *, unsigned char*, CARD32, CARD32, CARD32);
void XDGAUnmapFramebuffer(int);
unsigned char* XDGAGetMappedMemory(int);

#define XDGACheckExtension(dpy,i,val) \
  XextCheckExtension (dpy, i, xdga_extension_name, val)

/*****************************************************************************
 *                                                                           *
 *			   private utility routines                          *
 *                                                                           *
 *****************************************************************************/

static int xdga_close_display(Display *dpy, XExtCodes *codes);
static Bool xdga_wire_to_event(Display *dpy, XEvent *event, xEvent *wire_ev);
static Status xdga_event_to_wire(Display *dpy, XEvent *event, xEvent *wire_ev);

static XExtensionHooks xdga_extension_hooks = {
    NULL,				/* create_gc */
    NULL,				/* copy_gc */
    NULL,				/* flush_gc */
    NULL,				/* free_gc */
    NULL,				/* create_font */
    NULL,				/* free_font */
    xdga_close_display,			/* close_display */
    xdga_wire_to_event,			/* wire_to_event */
    xdga_event_to_wire,			/* event_to_wire */
    NULL,				/* error */
    NULL,				/* error_string */
};

static XEXT_GENERATE_CLOSE_DISPLAY (xdga_close_display, xdga_info)


XExtDisplayInfo* xdga_find_display(Display*);
XEXT_GENERATE_FIND_DISPLAY (xdga_find_display, xdga_info,
				   "XFree86-DGA",
				   &xdga_extension_hooks,
				   0, NULL)


static Status
xdga_event_to_wire(
  Display *dpy,
  XEvent *event,
  xEvent *wire_ev
){
    return True;
}

static Bool
xdga_wire_to_event(
  Display *dpy,
  XEvent *event,
  xEvent *wire_ev
){
  dgaEvent *wire = (dgaEvent *) wire_ev;
  XDGAButtonEvent *bevent;
  XDGAKeyEvent *kevent;
  XDGAMotionEvent *mevent;
  XExtDisplayInfo *info = xdga_find_display (dpy);

  XDGACheckExtension (dpy, info, False);

  switch((wire->u.u.type & 0x7f) - info->codes->first_event) {
  case MotionNotify:
	mevent = (XDGAMotionEvent*)event;
	mevent->type = wire->u.u.type & 0x7F;
	mevent->serial = _XSetLastRequestRead(dpy, (xGenericReply *)wire);
	mevent->display = dpy;
	mevent->screen = wire->u.event.screen;
	mevent->time = wire->u.event.time;
	mevent->state = wire->u.event.state;
	mevent->dx = wire->u.event.dx;
	mevent->dy = wire->u.event.dy;
	return True;
  case ButtonPress:
  case ButtonRelease:
	bevent = (XDGAButtonEvent*)event;
	bevent->type = wire->u.u.type & 0x7F;
	bevent->serial = _XSetLastRequestRead(dpy, (xGenericReply *)wire);
	bevent->display = dpy;
	bevent->screen = wire->u.event.screen;
	bevent->time = wire->u.event.time;
	bevent->state = wire->u.event.state;
	bevent->button = wire->u.u.detail;
	return True;
  case KeyPress:
  case KeyRelease:
	kevent = (XDGAKeyEvent*)event;
	kevent->type = wire->u.u.type & 0x7F;
	kevent->serial = _XSetLastRequestRead(dpy, (xGenericReply *)wire);
	kevent->display = dpy;
	kevent->screen = wire->u.event.screen;
	kevent->time = wire->u.event.time;
	kevent->state = wire->u.event.state;
	kevent->keycode = wire->u.u.detail;
	return True;
  }

  return False;
}


Bool XDGAQueryExtension (
    Display *dpy,
    int *event_basep,
    int *error_basep
){
    XExtDisplayInfo *info = xdga_find_display (dpy);

    if (XextHasExtension(info)) {
	*event_basep = info->codes->first_event;
	*error_basep = info->codes->first_error;
	return True;
    } else {
	return False;
    }
}


Bool XDGAQueryVersion(
    Display *dpy,
    int *majorVersion,
    int *minorVersion
){
    XExtDisplayInfo *info = xdga_find_display (dpy);
    xXDGAQueryVersionReply rep;
    xXDGAQueryVersionReq *req;

    XDGACheckExtension (dpy, info, False);

    LockDisplay(dpy);
    GetReq(XDGAQueryVersion, req);
    req->reqType = info->codes->major_opcode;
    req->dgaReqType = X_XDGAQueryVersion;
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
    {
	int i, j;

	for (i = 0, j = info->codes->first_event;
	     i < XF86DGANumberEvents;
	     i++, j++)
	{
	    XESetWireToEvent (dpy, j, xdga_wire_to_event);
	    XESetEventToWire (dpy, j, xdga_event_to_wire);
	}
	XDGASetClientVersion(dpy);
    }
    return True;
}

Bool XDGASetClientVersion(
    Display	*dpy
){
    XExtDisplayInfo *info = xdga_find_display (dpy);
    xXDGASetClientVersionReq *req;

    XDGACheckExtension (dpy, info, False);

    LockDisplay(dpy);
    GetReq(XDGASetClientVersion, req);
    req->reqType = info->codes->major_opcode;
    req->dgaReqType = X_XDGASetClientVersion;
    req->major = XDGA_MAJOR_VERSION;
    req->minor = XDGA_MINOR_VERSION;
    UnlockDisplay(dpy);
    SyncHandle();
    return True;
}

Bool XDGAOpenFramebuffer(
    Display	*dpy,
    int 	screen
){
    XExtDisplayInfo *info = xdga_find_display (dpy);
    xXDGAOpenFramebufferReply rep;
    xXDGAOpenFramebufferReq *req;
    char *deviceName = NULL;
    Bool ret;

    XDGACheckExtension (dpy, info, False);

    LockDisplay(dpy);
    GetReq(XDGAOpenFramebuffer, req);
    req->reqType = info->codes->major_opcode;
    req->dgaReqType = X_XDGAOpenFramebuffer;
    req->screen = screen;
    if (!_XReply(dpy, (xReply *)&rep, 0, xFalse)) {
	UnlockDisplay(dpy);
	SyncHandle();
	return False;
    }

    if (rep.length) {
	if (rep.length < (INT_MAX >> 2)) {
	    unsigned long size = rep.length << 2;
	    deviceName = Xmalloc(size);
	    _XRead(dpy, deviceName, size);
	    deviceName[size - 1] = '\0';
	} else
	    _XEatDataWords(dpy, rep.length);
    }

    ret = XDGAMapFramebuffer(screen, deviceName,
				(unsigned char*)(long)rep.mem1,
				rep.size, rep.offset, rep.extra);

    if(deviceName)
	Xfree(deviceName);

    UnlockDisplay(dpy);
    SyncHandle();
    return ret;
}

void XDGACloseFramebuffer(
    Display	*dpy,
    int		screen
){
    XExtDisplayInfo *info = xdga_find_display (dpy);
    xXDGACloseFramebufferReq *req;

    XextSimpleCheckExtension (dpy, info, xdga_extension_name);

    XDGAUnmapFramebuffer(screen);

    LockDisplay(dpy);
    GetReq(XDGACloseFramebuffer, req);
    req->reqType = info->codes->major_opcode;
    req->dgaReqType = X_XDGACloseFramebuffer;
    req->screen = screen;
    UnlockDisplay(dpy);
    SyncHandle();
}



XDGAMode* XDGAQueryModes(
    Display *dpy,
    int screen,
    int *num
){
    XExtDisplayInfo *dinfo = xdga_find_display (dpy);
    xXDGAQueryModesReply rep;
    xXDGAQueryModesReq *req;
    XDGAMode *modes = NULL;

    *num = 0;

    XDGACheckExtension (dpy, dinfo, NULL);

    LockDisplay(dpy);
    GetReq(XDGAQueryModes, req);
    req->reqType = dinfo->codes->major_opcode;
    req->dgaReqType = X_XDGAQueryModes;
    req->screen = screen;

    if (_XReply(dpy, (xReply *)&rep, 0, xFalse)) {
	if(rep.length) {
	   xXDGAModeInfo info;
	   unsigned long size = 0;
	   char *offset;

	   if ((rep.length < (INT_MAX >> 2)) &&
	       (rep.number < (INT_MAX / sizeof(XDGAMode)))) {
	       size = rep.length << 2;
	       if (size > (rep.number * sz_xXDGAModeInfo)) {
		   size -= rep.number * sz_xXDGAModeInfo; /* find text size */
		   modes = Xmalloc((rep.number * sizeof(XDGAMode)) + size);
		   offset = (char*)(&modes[rep.number]);  /* start of text */
	       }
	   }

	   if (modes != NULL) {
	      unsigned int i;
	      for(i = 0; i < rep.number; i++) {
		_XRead(dpy, (char*)(&info), sz_xXDGAModeInfo);

		modes[i].num = info.num;
		modes[i].verticalRefresh =
			(float)info.vsync_num / (float)info.vsync_den;
		modes[i].flags = info.flags;
		modes[i].imageWidth = info.image_width;
		modes[i].imageHeight = info.image_height;
		modes[i].pixmapWidth = info.pixmap_width;
		modes[i].pixmapHeight = info.pixmap_height;
		modes[i].bytesPerScanline = info.bytes_per_scanline;
		modes[i].byteOrder = info.byte_order;
		modes[i].depth = info.depth;
		modes[i].bitsPerPixel = info.bpp;
		modes[i].redMask = info.red_mask;
		modes[i].greenMask = info.green_mask;
		modes[i].blueMask = info.blue_mask;
		modes[i].visualClass = info.visual_class;
		modes[i].viewportWidth = info.viewport_width;
		modes[i].viewportHeight = info.viewport_height;
		modes[i].xViewportStep = info.viewport_xstep;
		modes[i].yViewportStep = info.viewport_ystep;
		modes[i].maxViewportX = info.viewport_xmax;
		modes[i].maxViewportY = info.viewport_ymax;
		modes[i].viewportFlags = info.viewport_flags;
		modes[i].reserved1 = info.reserved1;
		modes[i].reserved2 = info.reserved2;

		if (info.name_size > 0 && info.name_size <= size) {
		    _XRead(dpy, offset, info.name_size);
		    modes[i].name = offset;
		    modes[i].name[info.name_size - 1] = '\0';
		    offset += info.name_size;
		    size -= info.name_size;
		} else {
		    _XEatData(dpy, info.name_size);
		    modes[i].name = NULL;
		}
	      }
	      *num = rep.number;
	   } else
		_XEatDataWords(dpy, rep.length);
	}
    }

    UnlockDisplay(dpy);
    SyncHandle();

    return modes;
}


XDGADevice *
XDGASetMode(
    Display	*dpy,
    int		screen,
    int		mode
){
    XExtDisplayInfo *dinfo = xdga_find_display (dpy);
    xXDGASetModeReply rep;
    xXDGASetModeReq *req;
    XDGADevice *dev = NULL;
    Pixmap pid;

    XDGACheckExtension (dpy, dinfo, NULL);

    LockDisplay(dpy);
    GetReq(XDGASetMode, req);
    req->reqType = dinfo->codes->major_opcode;
    req->dgaReqType = X_XDGASetMode;
    req->screen = screen;
    req->mode = mode;
    req->pid = pid = XAllocID(dpy);

    if (_XReply(dpy, (xReply *)&rep, 0, xFalse)) {
	if(rep.length) {
	   xXDGAModeInfo info;
	   unsigned long size;

	   if ((rep.length < (INT_MAX >> 2)) &&
	       (rep.length > (sz_xXDGAModeInfo >> 2))) {
	       size = rep.length << 2;
	       size -= sz_xXDGAModeInfo; /* get text size */

	       dev = Xmalloc(sizeof(XDGADevice) + size);
	   }

	   if(dev) {
		_XRead(dpy, (char*)(&info), sz_xXDGAModeInfo);

		dev->mode.num = info.num;
		dev->mode.verticalRefresh =
				(float)info.vsync_num / (float)info.vsync_den;
		dev->mode.flags = info.flags;
		dev->mode.imageWidth = info.image_width;
		dev->mode.imageHeight = info.image_height;
		dev->mode.pixmapWidth = info.pixmap_width;
		dev->mode.pixmapHeight = info.pixmap_height;
		dev->mode.bytesPerScanline = info.bytes_per_scanline;
		dev->mode.byteOrder = info.byte_order;
		dev->mode.depth = info.depth;
		dev->mode.bitsPerPixel = info.bpp;
		dev->mode.redMask = info.red_mask;
		dev->mode.greenMask = info.green_mask;
		dev->mode.blueMask = info.blue_mask;
		dev->mode.visualClass = info.visual_class;
		dev->mode.viewportWidth = info.viewport_width;
		dev->mode.viewportHeight = info.viewport_height;
		dev->mode.xViewportStep = info.viewport_xstep;
		dev->mode.yViewportStep = info.viewport_ystep;
		dev->mode.maxViewportX = info.viewport_xmax;
		dev->mode.maxViewportY = info.viewport_ymax;
		dev->mode.viewportFlags = info.viewport_flags;
		dev->mode.reserved1 = info.reserved1;
		dev->mode.reserved2 = info.reserved2;

		if (info.name_size > 0 && info.name_size <= size) {
		    dev->mode.name = (char*)(&dev[1]);
		    _XRead(dpy, dev->mode.name, info.name_size);
		    dev->mode.name[info.name_size - 1] = '\0';
		} else {
		    dev->mode.name = NULL;
		    _XEatDataWords(dpy, rep.length);
		}

		dev->pixmap = (rep.flags & XDGAPixmap) ? pid : 0;
		dev->data = XDGAGetMappedMemory(screen);

		if(dev->data)
		    dev->data += rep.offset;
	   }
	   /* not sure what to do if the allocation fails */
	   else
	       _XEatDataWords(dpy, rep.length);
	}
    }

    UnlockDisplay(dpy);
    SyncHandle();

    return dev;
}


void XDGASetViewport(
    Display	*dpy,
    int		screen,
    int		x,
    int		y,
    int		flags
){
    XExtDisplayInfo *info = xdga_find_display (dpy);
    xXDGASetViewportReq *req;

    XextSimpleCheckExtension (dpy, info, xdga_extension_name);

    LockDisplay(dpy);
    GetReq(XDGASetViewport, req);
    req->reqType = info->codes->major_opcode;
    req->dgaReqType = X_XDGASetViewport;
    req->screen = screen;
    req->x = x;
    req->y = y;
    req->flags = flags;
    UnlockDisplay(dpy);
    SyncHandle();
}


void XDGAInstallColormap(
    Display	*dpy,
    int		screen,
    Colormap	cmap
){
    XExtDisplayInfo *info = xdga_find_display (dpy);
    xXDGAInstallColormapReq *req;

    XextSimpleCheckExtension (dpy, info, xdga_extension_name);

    LockDisplay(dpy);
    GetReq(XDGAInstallColormap, req);
    req->reqType = info->codes->major_opcode;
    req->dgaReqType = X_XDGAInstallColormap;
    req->screen = screen;
    req->cmap = cmap;
    UnlockDisplay(dpy);
    SyncHandle();
}

void XDGASelectInput(
    Display	*dpy,
    int		screen,
    long	mask
){
    XExtDisplayInfo *info = xdga_find_display (dpy);
    xXDGASelectInputReq *req;

    XextSimpleCheckExtension (dpy, info, xdga_extension_name);

    LockDisplay(dpy);
    GetReq(XDGASelectInput, req);
    req->reqType = info->codes->major_opcode;
    req->dgaReqType = X_XDGASelectInput;
    req->screen = screen;
    req->mask = mask;
    UnlockDisplay(dpy);
    SyncHandle();
}

void XDGAFillRectangle(
    Display	*dpy,
    int		screen,
    int		x,
    int		y,
    unsigned int	width,
    unsigned int	height,
    unsigned long	color
){
    XExtDisplayInfo *info = xdga_find_display (dpy);
    xXDGAFillRectangleReq *req;

    XextSimpleCheckExtension (dpy, info, xdga_extension_name);

    LockDisplay(dpy);
    GetReq(XDGAFillRectangle, req);
    req->reqType = info->codes->major_opcode;
    req->dgaReqType = X_XDGAFillRectangle;
    req->screen = screen;
    req->x = x;
    req->y = y;
    req->width = width;
    req->height = height;
    req->color = color;
    UnlockDisplay(dpy);
    SyncHandle();
}

void XDGACopyArea(
    Display	*dpy,
    int		screen,
    int		srcx,
    int		srcy,
    unsigned int	width,
    unsigned int	height,
    int		dstx,
    int		dsty
){
    XExtDisplayInfo *info = xdga_find_display (dpy);
    xXDGACopyAreaReq *req;

    XextSimpleCheckExtension (dpy, info, xdga_extension_name);

    LockDisplay(dpy);
    GetReq(XDGACopyArea, req);
    req->reqType = info->codes->major_opcode;
    req->dgaReqType = X_XDGACopyArea;
    req->screen = screen;
    req->srcx = srcx;
    req->srcy = srcy;
    req->width = width;
    req->height = height;
    req->dstx = dstx;
    req->dsty = dsty;
    UnlockDisplay(dpy);
    SyncHandle();
}

void XDGACopyTransparentArea(
    Display	*dpy,
    int		screen,
    int		srcx,
    int		srcy,
    unsigned int	width,
    unsigned int	height,
    int		dstx,
    int		dsty,
    unsigned long key
){
    XExtDisplayInfo *info = xdga_find_display (dpy);
    xXDGACopyTransparentAreaReq *req;

    XextSimpleCheckExtension (dpy, info, xdga_extension_name);

    LockDisplay(dpy);
    GetReq(XDGACopyTransparentArea, req);
    req->reqType = info->codes->major_opcode;
    req->dgaReqType = X_XDGACopyTransparentArea;
    req->screen = screen;
    req->srcx = srcx;
    req->srcy = srcy;
    req->width = width;
    req->height = height;
    req->dstx = dstx;
    req->dsty = dsty;
    req->key = key;
    UnlockDisplay(dpy);
    SyncHandle();
}


int XDGAGetViewportStatus(
    Display *dpy,
    int screen
){
    XExtDisplayInfo *info = xdga_find_display (dpy);
    xXDGAGetViewportStatusReply rep;
    xXDGAGetViewportStatusReq *req;
    int status = 0;

    XDGACheckExtension (dpy, info, 0);

    LockDisplay(dpy);
    GetReq(XDGAGetViewportStatus, req);
    req->reqType = info->codes->major_opcode;
    req->dgaReqType = X_XDGAGetViewportStatus;
    req->screen = screen;
    if (!_XReply(dpy, (xReply *)&rep, 0, xFalse))
	status = rep.status;
    UnlockDisplay(dpy);
    SyncHandle();
    return status;
}

void XDGASync(
    Display *dpy,
    int screen
){
    XExtDisplayInfo *info = xdga_find_display (dpy);
    xXDGASyncReply rep;
    xXDGASyncReq *req;

    XextSimpleCheckExtension (dpy, info, xdga_extension_name);

    LockDisplay(dpy);
    GetReq(XDGASync, req);
    req->reqType = info->codes->major_opcode;
    req->dgaReqType = X_XDGASync;
    req->screen = screen;
    _XReply(dpy, (xReply *)&rep, 0, xFalse);
    UnlockDisplay(dpy);
    SyncHandle();
}


void XDGAChangePixmapMode(
    Display *dpy,
    int screen,
    int *x,
    int *y,
    int mode
){
    XExtDisplayInfo *info = xdga_find_display (dpy);
    xXDGAChangePixmapModeReq *req;
    xXDGAChangePixmapModeReply rep;

    XextSimpleCheckExtension (dpy, info, xdga_extension_name);

    LockDisplay(dpy);
    GetReq(XDGAChangePixmapMode, req);
    req->reqType = info->codes->major_opcode;
    req->dgaReqType = X_XDGAChangePixmapMode;
    req->screen = screen;
    req->x = *x;
    req->y = *y;
    req->flags = mode;
    _XReply(dpy, (xReply *)&rep, 0, xFalse);
    *x = rep.x;
    *y = rep.y;
    UnlockDisplay(dpy);
    SyncHandle();
}

Colormap XDGACreateColormap(
    Display *dpy,
    int screen,
    XDGADevice *dev,
    int	alloc
){
    XExtDisplayInfo *info = xdga_find_display (dpy);
    xXDGACreateColormapReq *req;
    Colormap cid;

    XDGACheckExtension (dpy, info, -1);

    LockDisplay(dpy);
    GetReq(XDGACreateColormap, req);
    req->reqType = info->codes->major_opcode;
    req->dgaReqType = X_XDGACreateColormap;
    req->screen = screen;
    req->mode = dev->mode.num;
    req->alloc = alloc;
    cid = req->id = XAllocID(dpy);
    UnlockDisplay(dpy);
    SyncHandle();

    return cid;
}


void XDGAKeyEventToXKeyEvent(
    XDGAKeyEvent* dk,
    XKeyEvent* xk
){
    xk->type = dk->type;
    xk->serial = dk->serial;
    xk->send_event = False;
    xk->display = dk->display;
    xk->window = RootWindow(dk->display, dk->screen);
    xk->root = xk->window;
    xk->subwindow = None;
    xk->time = dk->time;
    xk->x = xk->y = xk->x_root = xk->y_root = 0;
    xk->state = dk->state;
    xk->keycode = dk->keycode;
    xk->same_screen = True;
}

#include <X11/Xmd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>

#if defined(SVR4) && !defined(sun)
#define DEV_MEM "/dev/pmem"
#elif defined(SVR4) && defined(sun)
#define DEV_MEM "/dev/xsvc"
#elif defined(HAS_APERTURE_DRV)
#define DEV_MEM "/dev/xf86"
#else
#define DEV_MEM "/dev/mem"
#endif



typedef struct _DGAMapRec{
  unsigned char *physical;
  unsigned char *virtual;
  CARD32 size;
  int fd;
  int screen;
  struct _DGAMapRec *next;
} DGAMapRec, *DGAMapPtr;

static Bool
DGAMapPhysical(int, const char*, unsigned char*, CARD32, CARD32, CARD32, DGAMapPtr);
static void DGAUnmapPhysical(DGAMapPtr);

static DGAMapPtr _Maps = NULL;


unsigned char*
XDGAGetMappedMemory(int screen)
{
    DGAMapPtr pMap = _Maps;
    unsigned char *pntr = NULL;

    while(pMap != NULL) {
	if(pMap->screen == screen) {
	    pntr = pMap->virtual;
	    break;
	}
	pMap = pMap->next;
    }

    return pntr;
}

Bool
XDGAMapFramebuffer(
   int screen,
   char *name,			/* optional device name */
   unsigned char* base,		/* physical memory */
   CARD32 size,			/* size */
   CARD32 offset,		/* optional offset */
   CARD32 extra			/* optional extra data */
){
   DGAMapPtr pMap = _Maps;
   Bool result;

   /* is it already mapped ? */
   while(pMap != NULL) {
     if(pMap->screen == screen)
	return True;
     pMap = pMap->next;
   }

   if(extra & XDGANeedRoot) {
    /* we should probably check if we have root permissions and
       return False here */

   }

   pMap = (DGAMapPtr)Xmalloc(sizeof(DGAMapRec));

   result = DGAMapPhysical(screen, name, base, size, offset, extra, pMap);

   if(result) {
      pMap->next = _Maps;
      _Maps = pMap;
   } else
      Xfree(pMap);

   return result;
}

void
XDGAUnmapFramebuffer(int screen)
{
   DGAMapPtr pMap = _Maps;
   DGAMapPtr pPrev = NULL;

   /* is it already mapped */
    while(pMap != NULL) {
	if(pMap->screen == screen)
	    break;
	pPrev = pMap;
	pMap = pMap->next;
    }

    if(!pMap)
	return;

    DGAUnmapPhysical(pMap);

    if(!pPrev)
	_Maps = pMap->next;
    else
	pPrev->next = pMap->next;

    Xfree(pMap);
}


static Bool
DGAMapPhysical(
   int screen,
   const char *name,		/* optional device name */
   unsigned char* base,		/* physical memory */
   CARD32 size,			/* size */
   CARD32 offset,		/* optional offset */
   CARD32 extra,		/* optional extra data */
   DGAMapPtr pMap
) {

    base += offset;

    pMap->screen = screen;
    pMap->physical = base;
    pMap->size = size;

#ifndef MAP_FILE
#define MAP_FILE 0
#endif
    if (!name)
	    name = DEV_MEM;
    if ((pMap->fd = open(name, O_RDWR)) < 0)
	return False;
    pMap->virtual = mmap(NULL, size, PROT_READ | PROT_WRITE,
			MAP_FILE | MAP_SHARED, pMap->fd, (off_t)(uintptr_t)base);
    if (pMap->virtual == (void *)-1)
	return False;
    mprotect(pMap->virtual, size, PROT_READ | PROT_WRITE);

    return True;
}



static void
DGAUnmapPhysical(DGAMapPtr pMap)
{
    if (pMap->virtual && pMap->virtual != (void *)-1) {
	mprotect(pMap->virtual,pMap->size, PROT_READ);
	munmap(pMap->virtual, pMap->size);
	pMap->virtual = 0;
    }
    if (pMap->fd >= 0) {
	close(pMap->fd);
	pMap->fd = -1;
    }
}

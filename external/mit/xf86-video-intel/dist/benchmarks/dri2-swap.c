/*
 * Copyright (c) 2015 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xlib-xcb.h>
#include <X11/Xutil.h>
#include <X11/Xlibint.h>
#include <X11/extensions/dpms.h>
#include <X11/extensions/randr.h>
#include <X11/extensions/Xcomposite.h>
#include <X11/extensions/Xdamage.h>
#include <X11/extensions/Xrandr.h>
#include <xcb/xcb.h>
#include <xcb/dri2.h>
#include <xf86drm.h>

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <setjmp.h>
#include <signal.h>

#include <X11/Xlibint.h>
#include <X11/extensions/Xext.h>
#include <X11/extensions/extutil.h>
#include <X11/extensions/dri2proto.h>
#include <X11/extensions/dri2tokens.h>
#include <X11/extensions/Xfixes.h>

static char dri2ExtensionName[] = DRI2_NAME;
static XExtensionInfo *dri2Info;
static XEXT_GENERATE_CLOSE_DISPLAY (DRI2CloseDisplay, dri2Info)

static Bool
DRI2WireToEvent(Display *dpy, XEvent *event, xEvent *wire);
static Status
DRI2EventToWire(Display *dpy, XEvent *event, xEvent *wire);
static int
DRI2Error(Display *display, xError *err, XExtCodes *codes, int *ret_code);

static /* const */ XExtensionHooks dri2ExtensionHooks = {
  NULL,                   /* create_gc */
  NULL,                   /* copy_gc */
  NULL,                   /* flush_gc */
  NULL,                   /* free_gc */
  NULL,                   /* create_font */
  NULL,                   /* free_font */
  DRI2CloseDisplay,       /* close_display */
  DRI2WireToEvent,        /* wire_to_event */
  DRI2EventToWire,        /* event_to_wire */
  DRI2Error,              /* error */
  NULL,                   /* error_string */
};

static XEXT_GENERATE_FIND_DISPLAY (DRI2FindDisplay,
                                   dri2Info,
                                   dri2ExtensionName,
                                   &dri2ExtensionHooks,
                                   0, NULL)

static Bool
DRI2WireToEvent(Display *dpy, XEvent *event, xEvent *wire)
{
   XExtDisplayInfo *info = DRI2FindDisplay(dpy);

   XextCheckExtension(dpy, info, dri2ExtensionName, False);

   switch ((wire->u.u.type & 0x7f) - info->codes->first_event) {
#ifdef X_DRI2SwapBuffers
   case DRI2_BufferSwapComplete:
      return False;
#endif
#ifdef DRI2_InvalidateBuffers
   case DRI2_InvalidateBuffers:
      return False;
#endif
   default:
      /* client doesn't support server event */
      break;
   }

   return False;
}

/* We don't actually support this.  It doesn't make sense for clients to
 * send each other DRI2 events.
 */
static Status
DRI2EventToWire(Display *dpy, XEvent *event, xEvent *wire)
{
   XExtDisplayInfo *info = DRI2FindDisplay(dpy);

   XextCheckExtension(dpy, info, dri2ExtensionName, False);

   switch (event->type) {
   default:
      /* client doesn't support server event */
      break;
   }

   return Success;
}

static int
DRI2Error(Display *display, xError *err, XExtCodes *codes, int *ret_code)
{
	if (err->majorCode == codes->major_opcode &&
	    err->errorCode == BadDrawable &&
	    err->minorCode == X_DRI2CopyRegion)
		return True;

	/* If the X drawable was destroyed before the GLX drawable, the
	 * DRI2 drawble will be gone by the time we call
	 * DRI2DestroyDrawable.  So just ignore BadDrawable here. */
	if (err->majorCode == codes->major_opcode &&
	    err->errorCode == BadDrawable &&
	    err->minorCode == X_DRI2DestroyDrawable)
		return True;

	/* If the server is non-local DRI2Connect will raise BadRequest.
	 * Swallow this so that DRI2Connect can signal this in its return code */
	if (err->majorCode == codes->major_opcode &&
	    err->minorCode == X_DRI2Connect &&
	    err->errorCode == BadRequest) {
		*ret_code = False;
		return True;
	}

	return False;
}

static Bool
DRI2QueryExtension(Display * dpy, int *eventBase, int *errorBase)
{
	XExtDisplayInfo *info = DRI2FindDisplay(dpy);

	if (XextHasExtension(info)) {
		*eventBase = info->codes->first_event;
		*errorBase = info->codes->first_error;
		return True;
	}

	return False;
}

static Bool
DRI2Connect(Display * dpy, XID window, char **driverName, char **deviceName)
{
	XExtDisplayInfo *info = DRI2FindDisplay(dpy);
	xDRI2ConnectReply rep;
	xDRI2ConnectReq *req;

	XextCheckExtension(dpy, info, dri2ExtensionName, False);

	LockDisplay(dpy);
	GetReq(DRI2Connect, req);
	req->reqType = info->codes->major_opcode;
	req->dri2ReqType = X_DRI2Connect;
	req->window = window;
	req->driverType = DRI2DriverDRI;
	if (!_XReply(dpy, (xReply *) & rep, 0, xFalse)) {
		UnlockDisplay(dpy);
		SyncHandle();
		return False;
	}

	if (rep.driverNameLength == 0 && rep.deviceNameLength == 0) {
		UnlockDisplay(dpy);
		SyncHandle();
		return False;
	}

	*driverName = Xmalloc(rep.driverNameLength + 1);
	if (*driverName == NULL) {
		_XEatData(dpy,
			  ((rep.driverNameLength + 3) & ~3) +
			  ((rep.deviceNameLength + 3) & ~3));
		UnlockDisplay(dpy);
		SyncHandle();
		return False;
	}
	_XReadPad(dpy, *driverName, rep.driverNameLength);
	(*driverName)[rep.driverNameLength] = '\0';

	*deviceName = Xmalloc(rep.deviceNameLength + 1);
	if (*deviceName == NULL) {
		Xfree(*driverName);
		_XEatData(dpy, ((rep.deviceNameLength + 3) & ~3));
		UnlockDisplay(dpy);
		SyncHandle();
		return False;
	}
	_XReadPad(dpy, *deviceName, rep.deviceNameLength);
	(*deviceName)[rep.deviceNameLength] = '\0';

	UnlockDisplay(dpy);
	SyncHandle();

	return True;
}

static Bool
DRI2Authenticate(Display * dpy, XID window, unsigned int magic)
{
	XExtDisplayInfo *info = DRI2FindDisplay(dpy);
	xDRI2AuthenticateReq *req;
	xDRI2AuthenticateReply rep;

	XextCheckExtension(dpy, info, dri2ExtensionName, False);

	LockDisplay(dpy);
	GetReq(DRI2Authenticate, req);
	req->reqType = info->codes->major_opcode;
	req->dri2ReqType = X_DRI2Authenticate;
	req->window = window;
	req->magic = magic;

	if (!_XReply(dpy, (xReply *) & rep, 0, xFalse)) {
		UnlockDisplay(dpy);
		SyncHandle();
		return False;
	}

	UnlockDisplay(dpy);
	SyncHandle();

	return rep.authenticated;
}

static void
DRI2CreateDrawable(Display * dpy, XID drawable)
{
	XExtDisplayInfo *info = DRI2FindDisplay(dpy);
	xDRI2CreateDrawableReq *req;

	XextSimpleCheckExtension(dpy, info, dri2ExtensionName);

	LockDisplay(dpy);
	GetReq(DRI2CreateDrawable, req);
	req->reqType = info->codes->major_opcode;
	req->dri2ReqType = X_DRI2CreateDrawable;
	req->drawable = drawable;
	UnlockDisplay(dpy);
	SyncHandle();
}

static void DRI2SwapInterval(Display *dpy, XID drawable, int interval)
{
    XExtDisplayInfo *info = DRI2FindDisplay(dpy);
    xDRI2SwapIntervalReq *req;

    XextSimpleCheckExtension (dpy, info, dri2ExtensionName);

    LockDisplay(dpy);
    GetReq(DRI2SwapInterval, req);
    req->reqType = info->codes->major_opcode;
    req->dri2ReqType = X_DRI2SwapInterval;
    req->drawable = drawable;
    req->interval = interval;
    UnlockDisplay(dpy);
    SyncHandle();
}

static int _x_error_occurred;

static int
_check_error_handler(Display     *display,
		     XErrorEvent *event)
{
	fprintf(stderr,
		"X11 error from display %s, serial=%ld, error=%d, req=%d.%d\n",
	       DisplayString(display),
	       event->serial,
	       event->error_code,
	       event->request_code,
	       event->minor_code);
	_x_error_occurred++;
	return False; /* ignored */
}

static double elapsed(const struct timespec *start,
		      const struct timespec *end)
{
	return 1e6*(end->tv_sec - start->tv_sec) + (end->tv_nsec - start->tv_nsec)/1000;
}

static void run(Display *dpy, Window win)
{
	xcb_connection_t *c = XGetXCBConnection(dpy);
	struct timespec start, end;
	int n, completed = 0;

	clock_gettime(CLOCK_MONOTONIC, &start);
	do {
		for (n = 0; n < 1000; n++) {
			unsigned int attachments[] = { DRI2BufferBackLeft };
			unsigned int seq[2];

			seq[0] = xcb_dri2_swap_buffers_unchecked(c, win,
								 0, 0, 0, 0, 0, 0).sequence;


			seq[1] = xcb_dri2_get_buffers_unchecked(c, win,
								1, 1, attachments).sequence;

			xcb_flush(c);
			xcb_discard_reply(c, seq[0]);
			xcb_discard_reply(c, seq[1]);
			completed++;
		}
		clock_gettime(CLOCK_MONOTONIC, &end);
	} while (end.tv_sec < start.tv_sec + 10);

	printf("%f\n", completed / (elapsed(&start, &end) / 1000000));
}

static inline XRRScreenResources *_XRRGetScreenResourcesCurrent(Display *dpy, Window window)
{
	XRRScreenResources *res;

	res = XRRGetScreenResourcesCurrent(dpy, window);
	if (res == NULL)
		res = XRRGetScreenResources(dpy, window);

	return res;
}

static XRRModeInfo *lookup_mode(XRRScreenResources *res, int id)
{
	int i;

	for (i = 0; i < res->nmode; i++) {
		if (res->modes[i].id == id)
			return &res->modes[i];
	}

	return NULL;
}

static int dri2_open(Display *dpy)
{
	drm_auth_t auth;
	char *driver, *device;
	int fd;

	if (!DRI2QueryExtension(dpy, &fd, &fd))
		return -1;

	if (!DRI2Connect(dpy, DefaultRootWindow(dpy), &driver, &device))
		return -1;

	fd = open(device, O_RDWR);
	if (fd < 0)
		return -1;

	if (drmIoctl(fd, DRM_IOCTL_GET_MAGIC, &auth))
		return -1;

	if (!DRI2Authenticate(dpy, DefaultRootWindow(dpy), auth.magic))
		return -1;

	return fd;
}

static void fullscreen(Display *dpy, Window win)
{
	Atom atom = XInternAtom(dpy, "_NET_WM_STATE_FULLSCREEN", False);
	XChangeProperty(dpy, win,
			XInternAtom(dpy, "_NET_WM_STATE", False),
			XA_ATOM, 32, PropModeReplace,
			(unsigned char *)&atom, 1);
}

static int has_composite(Display *dpy)
{
	int event, error;
	int major, minor;

	if (!XDamageQueryExtension (dpy, &event, &error))
		return 0;

	if (!XCompositeQueryExtension(dpy, &event, &error))
		return 0;

	XCompositeQueryVersion(dpy, &major, &minor);

	return major > 0 || minor >= 4;
}

int main(int argc, char **argv)
{
	Display *dpy;
	Window root, win;
	XRRScreenResources *res;
	XRRCrtcInfo **original_crtc;
	XSetWindowAttributes attr;
	enum window { ROOT, FULLSCREEN, WINDOW } w = FULLSCREEN;
	enum visible {REDIRECTED, NORMAL } v = NORMAL;
	enum display { OFF, ON } d = OFF;
	int width, height;
	int i, fd;
	int c;

	while ((c = getopt(argc, argv, "d:v:w:")) != -1) {
		switch (c) {
		case 'd':
			if (strcmp(optarg, "off") == 0)
				d = OFF;
			else if (strcmp(optarg, "on") == 0)
				d = ON;
			else
				abort();
			break;

		case 'v':
			if (strcmp(optarg, "redirected") == 0)
				v = REDIRECTED;
			else if (strcmp(optarg, "normal") == 0)
				v = NORMAL;
			else
				abort();
			break;

		case 'w':
			if (strcmp(optarg, "fullscreen") == 0)
				w = FULLSCREEN;
			else if (strcmp(optarg, "window") == 0)
				w = WINDOW;
			else if (strcmp(optarg, "root") == 0)
				w = ROOT;
			else
				abort();
			break;
		}
	}

	attr.override_redirect = 1;

	dpy = XOpenDisplay(NULL);
	if (dpy == NULL)
		return 77;

	width = DisplayWidth(dpy, DefaultScreen(dpy));
	height = DisplayHeight(dpy, DefaultScreen(dpy));

	fd = dri2_open(dpy);
	if (fd < 0)
		return 77;

	if (DPMSQueryExtension(dpy, &i, &i))
		DPMSDisable(dpy);

	root = DefaultRootWindow(dpy);

	signal(SIGALRM, SIG_IGN);
	XSetErrorHandler(_check_error_handler);

	res = NULL;
	if (XRRQueryVersion(dpy, &i, &i))
		res = _XRRGetScreenResourcesCurrent(dpy, root);
	if (res == NULL)
		return 77;

	if (v == REDIRECTED && !has_composite(dpy))
		return 77;

	original_crtc = malloc(sizeof(XRRCrtcInfo *)*res->ncrtc);
	for (i = 0; i < res->ncrtc; i++)
		original_crtc[i] = XRRGetCrtcInfo(dpy, res, res->crtcs[i]);

	for (i = 0; i < res->ncrtc; i++)
		XRRSetCrtcConfig(dpy, res, res->crtcs[i], CurrentTime,
				 0, 0, None, RR_Rotate_0, NULL, 0);

	DRI2CreateDrawable(dpy, root);
	DRI2SwapInterval(dpy, root, 0);

	if (d != OFF) {
		for (i = 0; i < res->noutput; i++) {
			XRROutputInfo *output;
			XRRModeInfo *mode;

			output = XRRGetOutputInfo(dpy, res, res->outputs[i]);
			if (output == NULL)
				continue;

			mode = NULL;
			if (res->nmode)
				mode = lookup_mode(res, output->modes[0]);
			if (mode == NULL)
				continue;

			XRRSetCrtcConfig(dpy, res, output->crtcs[0], CurrentTime,
					 0, 0, output->modes[0], RR_Rotate_0, &res->outputs[i], 1);
			width = mode->width;
			height = mode->height;
			break;
		}
		if (i == res->noutput) {
			_x_error_occurred = 77;
			goto restore;
		}
	}

	if (w == ROOT) {
		run(dpy, root);
	} else if (w == FULLSCREEN) {
		win = XCreateWindow(dpy, root,
				    0, 0, width, height, 0,
				    DefaultDepth(dpy, DefaultScreen(dpy)),
				    InputOutput,
				    DefaultVisual(dpy, DefaultScreen(dpy)),
				    CWOverrideRedirect, &attr);
		DRI2CreateDrawable(dpy, win);
		DRI2SwapInterval(dpy, win, 0);
		if (v == REDIRECTED) {
			XCompositeRedirectWindow(dpy, win, CompositeRedirectManual);
			XDamageCreate(dpy, win, XDamageReportRawRectangles);
		} else
			fullscreen(dpy, win);
		XMapWindow(dpy, win);
		run(dpy, win);
	} else if (w == WINDOW) {
		win = XCreateWindow(dpy, root,
				    0, 0, width/2, height/2, 0,
				    DefaultDepth(dpy, DefaultScreen(dpy)),
				    InputOutput,
				    DefaultVisual(dpy, DefaultScreen(dpy)),
				    CWOverrideRedirect, &attr);
		DRI2CreateDrawable(dpy, win);
		DRI2SwapInterval(dpy, win, 0);
		if (v == REDIRECTED) {
			XCompositeRedirectWindow(dpy, win, CompositeRedirectManual);
			XDamageCreate(dpy, win, XDamageReportRawRectangles);
		}
		XMapWindow(dpy, win);
		run(dpy, win);
	}

restore:
	for (i = 0; i < res->ncrtc; i++)
		XRRSetCrtcConfig(dpy, res, res->crtcs[i], CurrentTime,
				 0, 0, None, RR_Rotate_0, NULL, 0);

	for (i = 0; i < res->ncrtc; i++)
		XRRSetCrtcConfig(dpy, res, res->crtcs[i], CurrentTime,
				 original_crtc[i]->x,
				 original_crtc[i]->y,
				 original_crtc[i]->mode,
				 original_crtc[i]->rotation,
				 original_crtc[i]->outputs,
				 original_crtc[i]->noutput);

	if (DPMSQueryExtension(dpy, &i, &i))
		DPMSEnable(dpy);

	XSync(dpy, True);
	return _x_error_occurred;
}

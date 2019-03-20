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

#include "dri2.h"

static int _x_error_occurred;

static int
_check_error_handler(Display     *display,
		     XErrorEvent *event)
{
	printf("X11 error from display %s, serial=%ld, error=%d, req=%d.%d\n",
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

static void run(Display *dpy, Window win, const char *name)
{
	xcb_connection_t *c = XGetXCBConnection(dpy);
	struct timespec start, end;
	int n, completed = 0;

	_x_error_occurred = 0;

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

	XSync(dpy, True);
	if (_x_error_occurred)
		abort();

	printf("%s: Completed %d swaps in %.1fs, %.3fus each (%.1f FPS)\n",
	       name, completed, elapsed(&start, &end) / 1000000,
	       elapsed(&start, &end) / completed,
	       completed / (elapsed(&start, &end) / 1000000));
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

	if (!DRI2Connect(dpy, DefaultRootWindow(dpy), &driver, &device))
		return -1;

	printf ("Connecting to %s driver on %s\n", driver, device);

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

int main(void)
{
	Display *dpy;
	Window root, win;
	XRRScreenResources *res;
	XRRCrtcInfo **original_crtc;
	XSetWindowAttributes attr;
	int i, j, fd;

	attr.override_redirect = 1;

	dpy = XOpenDisplay(NULL);
	if (dpy == NULL)
		return 77;

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

	original_crtc = malloc(sizeof(XRRCrtcInfo *)*res->ncrtc);
	for (i = 0; i < res->ncrtc; i++)
		original_crtc[i] = XRRGetCrtcInfo(dpy, res, res->crtcs[i]);

	printf("noutput=%d, ncrtc=%d\n", res->noutput, res->ncrtc);
	for (i = 0; i < res->ncrtc; i++)
		XRRSetCrtcConfig(dpy, res, res->crtcs[i], CurrentTime,
				 0, 0, None, RR_Rotate_0, NULL, 0);

	DRI2CreateDrawable(dpy, root);
	DRI2SwapInterval(dpy, root, 0);
	run(dpy, root, "off");
	XSync(dpy, True);

	for (i = 0; i < res->noutput; i++) {
		XRROutputInfo *output;
		XRRModeInfo *mode;

		output = XRRGetOutputInfo(dpy, res, res->outputs[i]);
		if (output == NULL)
			continue;

		mode = NULL;
		if (res->nmode)
			mode = lookup_mode(res, output->modes[0]);

		for (j = 0; mode && j < 2*output->ncrtc; j++) {
			int c = j;
			if (c >= output->ncrtc)
				c = 2*output->ncrtc - j - 1;

			printf("[%d, %d] -- OUTPUT:%ld, CRTC:%ld: %dx%d\n",
			       i, c, (long)res->outputs[i], (long)output->crtcs[c],
			       mode->width, mode->height);
			XRRSetCrtcConfig(dpy, res, output->crtcs[c], CurrentTime,
					 0, 0, output->modes[0], RR_Rotate_0, &res->outputs[i], 1);

			run(dpy, root, "root");
			XSync(dpy, True);

			win = XCreateWindow(dpy, root,
					    0, 0, mode->width, mode->height, 0,
					    DefaultDepth(dpy, DefaultScreen(dpy)),
					    InputOutput,
					    DefaultVisual(dpy, DefaultScreen(dpy)),
					    CWOverrideRedirect, &attr);
			DRI2CreateDrawable(dpy, win);
			DRI2SwapInterval(dpy, win, 0);
			fullscreen(dpy, win);
			XMapWindow(dpy, win);
			run(dpy, win, "fullscreen");
			XDestroyWindow(dpy, win);
			XSync(dpy, True);

			win = XCreateWindow(dpy, root,
					    0, 0, mode->width, mode->height, 0,
					    DefaultDepth(dpy, DefaultScreen(dpy)),
					    InputOutput,
					    DefaultVisual(dpy, DefaultScreen(dpy)),
					    CWOverrideRedirect, &attr);
			DRI2CreateDrawable(dpy, win);
			DRI2SwapInterval(dpy, win, 0);
			XMapWindow(dpy, win);
			run(dpy, win, "windowed");
			XDestroyWindow(dpy, win);
			XSync(dpy, True);

			if (has_composite(dpy)) {
				Damage damage;

				_x_error_occurred = 0;
				win = XCreateWindow(dpy, root,
						    0, 0, mode->width, mode->height, 0,
						    DefaultDepth(dpy, DefaultScreen(dpy)),
						    InputOutput,
						    DefaultVisual(dpy, DefaultScreen(dpy)),
						    CWOverrideRedirect, &attr);
				XCompositeRedirectWindow(dpy, win, CompositeRedirectManual);
				damage = XDamageCreate(dpy, win, XDamageReportRawRectangles);
				DRI2CreateDrawable(dpy, win);
				DRI2SwapInterval(dpy, win, 0);
				XMapWindow(dpy, win);
				XSync(dpy, True);
				if (!_x_error_occurred)
					run(dpy, win, "composited");
				XDamageDestroy(dpy, damage);
				XDestroyWindow(dpy, win);
				XSync(dpy, True);
			}

			win = XCreateWindow(dpy, root,
					    0, 0, mode->width/2, mode->height/2, 0,
					    DefaultDepth(dpy, DefaultScreen(dpy)),
					    InputOutput,
					    DefaultVisual(dpy, DefaultScreen(dpy)),
					    CWOverrideRedirect, &attr);
			DRI2CreateDrawable(dpy, win);
			DRI2SwapInterval(dpy, win, 0);
			XMapWindow(dpy, win);
			run(dpy, win, "half");
			XDestroyWindow(dpy, win);
			XSync(dpy, True);

			XRRSetCrtcConfig(dpy, res, output->crtcs[c], CurrentTime,
					 0, 0, None, RR_Rotate_0, NULL, 0);
		}

		XRRFreeOutputInfo(output);
	}

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
	return 0;
}

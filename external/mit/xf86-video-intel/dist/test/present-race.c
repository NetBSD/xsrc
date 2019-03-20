/*
 * Copyright (c) 2014 Intel Corporation
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
#include <X11/Xlib-xcb.h>
#include <X11/xshmfence.h>
#include <X11/Xutil.h>
#include <X11/Xlibint.h>
#include <X11/extensions/dpms.h>
#include <X11/extensions/randr.h>
#include <X11/extensions/Xcomposite.h>
#include <X11/extensions/Xrandr.h>
#include <X11/extensions/Xrender.h>
#include <X11/extensions/XShm.h>
#if HAVE_X11_EXTENSIONS_SHMPROTO_H
#include <X11/extensions/shmproto.h>
#elif HAVE_X11_EXTENSIONS_SHMSTR_H
#include <X11/extensions/shmstr.h>
#else
#error Failed to find the right header for X11 MIT-SHM protocol definitions
#endif
#include <xcb/xcb.h>
#include <xcb/present.h>
#include <xcb/xfixes.h>
#include <xcb/dri3.h>
#include <xf86drm.h>
#include <i915_drm.h>

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <setjmp.h>
#include <signal.h>

#include <sys/mman.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <pciaccess.h>

#include "dri3.h"

static int _x_error_occurred;
static uint32_t stamp;

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

static int has_composite(Display *dpy)
{
	int event, error;
	int major, minor;

	if (!XCompositeQueryExtension(dpy, &event, &error))
		return 0;

	XCompositeQueryVersion(dpy, &major, &minor);

	return major > 0 || minor >= 4;
}

static void *setup_msc(Display *dpy, Window win)
{
	xcb_connection_t *c = XGetXCBConnection(dpy);
	xcb_void_cookie_t cookie;
	uint32_t id = xcb_generate_id(c);
	xcb_generic_error_t *error;
	void *q;

	cookie = xcb_present_select_input_checked(c, id, win, XCB_PRESENT_EVENT_MASK_COMPLETE_NOTIFY);
	q = xcb_register_for_special_xge(c, &xcb_present_id, id, &stamp);

	error = xcb_request_check(c, cookie);
	assert(error == NULL);

	return q;
}

static void teardown_msc(Display *dpy, void *q)
{
	xcb_unregister_for_special_event(XGetXCBConnection(dpy), q);
}

static uint64_t wait_vblank(Display *dpy, Window win)
{
	xcb_connection_t *c = XGetXCBConnection(dpy);
	static uint32_t serial = 1;
	uint64_t msc = 0;
	int complete = 0;
	void *q;

	if (win == 0)
		win = DefaultRootWindow(dpy);

	q = setup_msc(dpy, win);

	xcb_present_notify_msc(c, win, serial ^ 0xdeadbeef, 0, 1, 0);
	xcb_flush(c);

	do {
		xcb_present_complete_notify_event_t *ce;
		xcb_generic_event_t *ev;

		ev = xcb_wait_for_special_event(c, q);
		if (ev == NULL)
			break;

		ce = (xcb_present_complete_notify_event_t *)ev;
		if (ce->kind == XCB_PRESENT_COMPLETE_KIND_NOTIFY_MSC &&
		    ce->serial == (serial ^ 0xdeadbeef)) {
			msc = ce->msc;
			complete = 1;
		}
		free(ev);
	} while (!complete);

	if (++serial == 0)
		serial = 1;

	teardown_msc(dpy, q);

	return msc;
}

static int test_basic(Display *dpy, int dummy)
{
	xcb_connection_t *c = XGetXCBConnection(dpy);
	XSetWindowAttributes attr;
	Visual *visual = DefaultVisual(dpy, DefaultScreen(dpy));
	Pixmap pixmap;
	struct dri3_fence fence;
	Window root, win;
	unsigned int width, height;
	unsigned border, depth;
	int x, y, ret = 1;
	const char *phase;
	uint64_t msc;

	root = DefaultRootWindow(dpy);
	XGetGeometry(dpy, root,
		     &win, &x, &y,
		     &width, &height, &border, &depth);

	_x_error_occurred = 0;
	attr.override_redirect = 1;
	switch (dummy) {
	case 0:
		win = root;
		phase = "root";
		break;
	case 1:
		win = XCreateWindow(dpy, root,
				    0, 0, width, height, 0, depth,
				    InputOutput, visual,
				    CWOverrideRedirect, &attr);
		phase = "fullscreen";
		break;
	case 2:
		width /= 2;
		height /= 2;
		win = XCreateWindow(dpy, root,
				    0, 0, width, height, 0, depth,
				    InputOutput, visual,
				    CWOverrideRedirect, &attr);
		phase = "window";
		break;
	case 3:
		if (!has_composite(dpy))
			return 0;

		win = XCreateWindow(dpy, root,
				    0, 0, width, height, 0,
				    DefaultDepth(dpy, DefaultScreen(dpy)),
				    InputOutput,
				    DefaultVisual(dpy, DefaultScreen(dpy)),
				    CWOverrideRedirect, &attr);
		XCompositeRedirectWindow(dpy, win, CompositeRedirectManual);
		phase = "composite";
		break;

	default:
		phase = "broken";
		win = root;
		abort();
		break;
	}

	XMapWindow(dpy, win);
	XSync(dpy, True);
	if (_x_error_occurred)
		return 1;

	if (dri3_create_fence(dpy, win, &fence))
		return 0;

	printf("%s: Testing basic flip: %dx%d\n", phase, width, height);
	fflush(stdout);
	_x_error_occurred = 0;

	xshmfence_reset(fence.addr);
	msc = wait_vblank(dpy, win);

	pixmap = XCreatePixmap(dpy, win, width, height, depth);
	xcb_present_pixmap(c, win, pixmap, 0,
			   0, /* valid */
			   0, /* update */
			   0, /* x_off */
			   0, /* y_off */
			   None,
			   None, /* wait fence */
			   fence.xid,
			   XCB_PRESENT_OPTION_NONE,
			   (msc + 64) & -64, /* target msc */
			   64, /* divisor */
			   32, /* remainder */
			   0, NULL);
	XFreePixmap(dpy, pixmap);

	pixmap = XCreatePixmap(dpy, win, width, height, depth);
	xcb_present_pixmap(c, win, pixmap, 0,
			   0, /* valid */
			   0, /* update */
			   0, /* x_off */
			   0, /* y_off */
			   None,
			   None, /* wait fence */
			   None, /* sync fence */
			   XCB_PRESENT_OPTION_NONE,
			   (msc + 64) & -64, /* target msc */
			   64, /* divisor */
			   48, /* remainder */
			   0, NULL);
	XFreePixmap(dpy, pixmap);
	XDestroyWindow(dpy, win);
	XFlush(dpy);

	ret = !!xshmfence_await(fence.addr);
	dri3_fence_free(dpy, &fence);

	XSync(dpy, True);
	ret += !!_x_error_occurred;

	return ret;
}

static int test_race(Display *dpy, int dummy)
{
	Display *mgr = XOpenDisplay(NULL);
	xcb_connection_t *c = XGetXCBConnection(dpy);
	XSetWindowAttributes attr;
	Visual *visual = DefaultVisual(dpy, DefaultScreen(dpy));
	Pixmap pixmap;
	struct dri3_fence fence;
	Window root, win;
	unsigned int width, height;
	unsigned border, depth;
	int x, y, ret = 1;
	const char *phase;
	uint64_t msc;

	root = DefaultRootWindow(dpy);
	XGetGeometry(dpy, root,
		     &win, &x, &y,
		     &width, &height, &border, &depth);

	_x_error_occurred = 0;
	attr.override_redirect = 1;
	switch (dummy) {
	case 0:
		win = root;
		phase = "root";
		break;
	case 1:
		win = XCreateWindow(dpy, root,
				    0, 0, width, height, 0, depth,
				    InputOutput, visual,
				    CWOverrideRedirect, &attr);
		phase = "fullscreen";
		break;
	case 2:
		width /= 2;
		height /= 2;
		win = XCreateWindow(dpy, root,
				    0, 0, width, height, 0, depth,
				    InputOutput, visual,
				    CWOverrideRedirect, &attr);
		phase = "window";
		break;
	case 3:
		if (!has_composite(dpy))
			return 0;

		win = XCreateWindow(dpy, root,
				    0, 0, width, height, 0,
				    DefaultDepth(dpy, DefaultScreen(dpy)),
				    InputOutput,
				    DefaultVisual(dpy, DefaultScreen(dpy)),
				    CWOverrideRedirect, &attr);
		XCompositeRedirectWindow(dpy, win, CompositeRedirectManual);
		phase = "composite";
		break;

	default:
		phase = "broken";
		win = root;
		abort();
		break;
	}

	XMapWindow(dpy, win);
	XSync(dpy, True);
	if (_x_error_occurred)
		return 1;

	if (dri3_create_fence(dpy, win, &fence))
		return 0;

	printf("%s: Testing race with manager: %dx%d\n", phase, width, height);
	fflush(stdout);
	_x_error_occurred = 0;

	xshmfence_reset(fence.addr);
	msc = wait_vblank(dpy, win);

	pixmap = XCreatePixmap(dpy, win, width, height, depth);
	xcb_present_pixmap(c, win, pixmap, 0,
			   0, /* valid */
			   0, /* update */
			   0, /* x_off */
			   0, /* y_off */
			   None,
			   None, /* wait fence */
			   fence.xid,
			   XCB_PRESENT_OPTION_NONE,
			   (msc + 64) & -64, /* target msc */
			   64, /* divisor */
			   32, /* remainder */
			   0, NULL);
	XFreePixmap(dpy, pixmap);

	XFlush(dpy);
	XDestroyWindow(mgr, win);
	XFlush(mgr);

	pixmap = XCreatePixmap(dpy, win, width, height, depth);
	xcb_present_pixmap(c, win, pixmap, 0,
			   0, /* valid */
			   0, /* update */
			   0, /* x_off */
			   0, /* y_off */
			   None,
			   None, /* wait fence */
			   None, /* sync fence */
			   XCB_PRESENT_OPTION_NONE,
			   (msc + 64) & -64, /* target msc */
			   64, /* divisor */
			   48, /* remainder */
			   0, NULL);
	XFreePixmap(dpy, pixmap);
	XFlush(dpy);

	ret = !!xshmfence_await(fence.addr);
	dri3_fence_free(dpy, &fence);

	XSync(dpy, True);
	ret += !!_x_error_occurred;

	XCloseDisplay(mgr);

	return ret;
}

static int has_present(Display *dpy)
{
	xcb_connection_t *c = XGetXCBConnection(dpy);
	xcb_generic_error_t *error = NULL;
	void *reply;

	reply = xcb_xfixes_query_version_reply(c,
					       xcb_xfixes_query_version(c,
									XCB_XFIXES_MAJOR_VERSION,
									XCB_XFIXES_MINOR_VERSION),
					       &error);
	free(reply);
	free(error);
	if (reply == NULL) {
		fprintf(stderr, "XFixes not supported on %s\n", DisplayString(dpy));
		return 0;
	}

	reply = xcb_dri3_query_version_reply(c,
					     xcb_dri3_query_version(c,
								    XCB_DRI3_MAJOR_VERSION,
								    XCB_DRI3_MINOR_VERSION),
					     &error);
	free(reply);
	free(error);
	if (reply == NULL) {
		fprintf(stderr, "DRI3 not supported on %s\n", DisplayString(dpy));
		return 0;
	}

	reply = xcb_present_query_version_reply(c,
						xcb_present_query_version(c,
									  XCB_PRESENT_MAJOR_VERSION,
									  XCB_PRESENT_MINOR_VERSION),
						&error);

	free(reply);
	free(error);
	if (reply == NULL) {
		fprintf(stderr, "Present not supported on %s\n", DisplayString(dpy));
		return 0;
	}

	return 1;
}

int main(void)
{
	Display *dpy;
	int dummy;
	int error = 0;

	dpy = XOpenDisplay(NULL);
	if (dpy == NULL)
		return 77;

	if (!has_present(dpy))
		return 77;

	if (DPMSQueryExtension(dpy, &dummy, &dummy))
		DPMSDisable(dpy);

	signal(SIGALRM, SIG_IGN);
	XSetErrorHandler(_check_error_handler);

	for (dummy = 0; dummy <= 3; dummy++) {
		error += test_basic(dpy, dummy);
		error += test_race(dpy, dummy);
	}

	if (DPMSQueryExtension(dpy, &dummy, &dummy))
		DPMSEnable(dpy);
	return !!error;
}

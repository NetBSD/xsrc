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
#include <X11/xshmfence.h>
#include <X11/Xutil.h>
#include <X11/Xlibint.h>
#include <X11/extensions/Xcomposite.h>
#include <X11/extensions/Xdamage.h>
#include <X11/extensions/dpms.h>
#include <X11/extensions/randr.h>
#include <X11/extensions/Xrandr.h>
#include <xcb/xcb.h>
#include <xcb/present.h>
#include <xcb/dri3.h>
#include <xcb/xfixes.h>
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

struct dri3_fence {
	XID xid;
	void *addr;
};

static int _x_error_occurred;
static uint32_t stamp;

struct list {
    struct list *next, *prev;
};

static void
list_init(struct list *list)
{
    list->next = list->prev = list;
}

static inline void
__list_add(struct list *entry,
	    struct list *prev,
	    struct list *next)
{
    next->prev = entry;
    entry->next = next;
    entry->prev = prev;
    prev->next = entry;
}

static inline void
list_add(struct list *entry, struct list *head)
{
    __list_add(entry, head, head->next);
}

static inline void
__list_del(struct list *prev, struct list *next)
{
	next->prev = prev;
	prev->next = next;
}

static inline void
_list_del(struct list *entry)
{
    __list_del(entry->prev, entry->next);
}

static inline void
list_move(struct list *list, struct list *head)
{
	if (list->prev != head) {
		_list_del(list);
		list_add(list, head);
	}
}

#define __container_of(ptr, sample, member)				\
    (void *)((char *)(ptr) - ((char *)&(sample)->member - (char *)(sample)))

#define list_for_each_entry(pos, head, member)				\
    for (pos = __container_of((head)->next, pos, member);		\
	 &pos->member != (head);					\
	 pos = __container_of(pos->member.next, pos, member))

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

static int dri3_create_fence(Display *dpy,
			     Pixmap pixmap,
			     struct dri3_fence *fence)
{
	xcb_connection_t *c = XGetXCBConnection(dpy);
	struct dri3_fence f;
	int fd;

	fd = xshmfence_alloc_shm();
	if (fd < 0)
		return -1;

	f.addr = xshmfence_map_shm(fd);
	if (f.addr == NULL) {
		close(fd);
		return -1;
	}

	f.xid = xcb_generate_id(c);
	xcb_dri3_fence_from_fd(c, pixmap, f.xid, 0, fd);

	*fence = f;
	return 0;
}

static double elapsed(const struct timespec *start,
		      const struct timespec *end)
{
	return 1e6*(end->tv_sec - start->tv_sec) + (end->tv_nsec - start->tv_nsec)/1000;
}

struct buffer {
	struct list link;
	Pixmap pixmap;
	struct dri3_fence fence;
	int fd;
	int busy;
};

static void run(Display *dpy, Window win)
{
	xcb_connection_t *c = XGetXCBConnection(dpy);
	struct timespec start, end;
#define N_BACK 8
	struct buffer buffer[N_BACK];
	struct list mru;
	Window root;
	unsigned int width, height;
	unsigned border, depth;
	unsigned present_flags = XCB_PRESENT_OPTION_ASYNC;
	xcb_xfixes_region_t update = 0;
	int completed = 0;
	int queued = 0;
	uint32_t eid;
	void *Q;
	int i, n;

	list_init(&mru);

	XGetGeometry(dpy, win,
		     &root, &i, &n, &width, &height, &border, &depth);

	_x_error_occurred = 0;

	for (n = 0; n < N_BACK; n++) {
		xcb_dri3_buffer_from_pixmap_reply_t *reply;
		int *fds;

		buffer[n].pixmap =
			XCreatePixmap(dpy, win, width, height, depth);
		buffer[n].fence.xid = 0;
		buffer[n].fd = -1;

		if (dri3_create_fence(dpy, win, &buffer[n].fence))
			return;

		reply = xcb_dri3_buffer_from_pixmap_reply (c,
							   xcb_dri3_buffer_from_pixmap(c, buffer[n].pixmap),
							   NULL);
		if (reply == NULL)
			return;

		fds = xcb_dri3_buffer_from_pixmap_reply_fds (c, reply);
		buffer[n].fd = fds[0];
		free(reply);

		/* start idle */
		xshmfence_trigger(buffer[n].fence.addr);
		buffer[n].busy = 0;
		list_add(&buffer[n].link, &mru);
	}

	eid = xcb_generate_id(c);
	xcb_present_select_input(c, eid, win,
                                 XCB_PRESENT_EVENT_MASK_IDLE_NOTIFY |
                                 XCB_PRESENT_EVENT_MASK_COMPLETE_NOTIFY);
	Q = xcb_register_for_special_xge(c, &xcb_present_id, eid, &stamp);

	clock_gettime(CLOCK_MONOTONIC, &start);
	do {
		for (n = 0; n < 1000; n++) {
			struct buffer *tmp, *b = NULL;
			list_for_each_entry(tmp, &mru, link) {
				if (!tmp->busy) {
					b = tmp;
					break;
				}
			}
			while (b == NULL) {
				xcb_present_generic_event_t *ev;

				ev = (xcb_present_generic_event_t *)
					xcb_wait_for_special_event(c, Q);
				if (ev == NULL)
					abort();

				do {
					switch (ev->evtype) {
					case XCB_PRESENT_COMPLETE_NOTIFY:
						completed++;
						queued--;
						break;

					case XCB_PRESENT_EVENT_IDLE_NOTIFY:
						{
							xcb_present_idle_notify_event_t *ie = (xcb_present_idle_notify_event_t *)ev;
							assert(ie->serial < N_BACK);
							buffer[ie->serial].busy = 0;
							if (b == NULL)
								b = &buffer[ie->serial];
							break;
						}
					}
					free(ev);
				} while ((ev = (xcb_present_generic_event_t *)xcb_poll_for_special_event(c, Q)));
			}

			b->busy = 1;
			if (b->fence.xid) {
				xshmfence_await(b->fence.addr);
				xshmfence_reset(b->fence.addr);
			}
			xcb_present_pixmap(c, win, b->pixmap, b - buffer,
					   0, /* valid */
					   update, /* update */
					   0, /* x_off */
					   0, /* y_off */
					   None,
					   None, /* wait fence */
					   b->fence.xid,
					   present_flags,
					   0, /* target msc */
					   0, /* divisor */
					   0, /* remainder */
					   0, NULL);
			list_move(&b->link, &mru);
			queued++;
			xcb_flush(c);
		}
		clock_gettime(CLOCK_MONOTONIC, &end);
	} while (end.tv_sec < start.tv_sec + 10);

	while (queued) {
		xcb_present_generic_event_t *ev;

		ev = (xcb_present_generic_event_t *)
			xcb_wait_for_special_event(c, Q);
		if (ev == NULL)
			abort();

		do {
			switch (ev->evtype) {
			case XCB_PRESENT_COMPLETE_NOTIFY:
				completed++;
				queued--;
				break;

			case XCB_PRESENT_EVENT_IDLE_NOTIFY:
				break;
			}
			free(ev);
		} while ((ev = (xcb_present_generic_event_t *)xcb_poll_for_special_event(c, Q)));
	}
	clock_gettime(CLOCK_MONOTONIC, &end);

	printf("%f\n", completed / (elapsed(&start, &end) / 1000000));
}

static int has_present(Display *dpy)
{
	xcb_connection_t *c = XGetXCBConnection(dpy);
	xcb_generic_error_t *error = NULL;
	void *reply;

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

static void fullscreen(Display *dpy, Window win)
{
	Atom atom = XInternAtom(dpy, "_NET_WM_STATE_FULLSCREEN", False);
	XChangeProperty(dpy, win,
			XInternAtom(dpy, "_NET_WM_STATE", False),
			XA_ATOM, 32, PropModeReplace,
			(unsigned char *)&atom, 1);
}

static int dri3_query_version(Display *dpy, int *major, int *minor)
{
	xcb_connection_t *c = XGetXCBConnection(dpy);
	xcb_dri3_query_version_reply_t *reply;
	xcb_generic_error_t *error;

	*major = *minor = -1;

	reply = xcb_dri3_query_version_reply(c,
					     xcb_dri3_query_version(c,
								    XCB_DRI3_MAJOR_VERSION,
								    XCB_DRI3_MINOR_VERSION),
					     &error);
	free(error);
	if (reply == NULL)
		return -1;

	*major = reply->major_version;
	*minor = reply->minor_version;
	free(reply);

	return 0;
}

static int has_dri3(Display *dpy)
{
	const xcb_query_extension_reply_t *ext;
	int major, minor;

	ext = xcb_get_extension_data(XGetXCBConnection(dpy), &xcb_dri3_id);
	if (ext == NULL || !ext->present)
		return 0;

	if (dri3_query_version(dpy, &major, &minor) < 0)
		return 0;

	return major >= 0;
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
	int i;

	while ((i = getopt(argc, argv, "d:v:w:")) != -1) {
		switch (i) {
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

	if (!has_present(dpy))
		return 77;

	if (!has_dri3(dpy))
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

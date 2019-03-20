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
#include <sys/wait.h>

#include "dri3.h"

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
	if (_x_error_occurred < 0)
		return True;

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

struct buffer {
	struct list link;
	Pixmap pixmap;
	struct dri3_fence fence;
	int fd;
	int busy;
	int id;
};

#define DRI3 1
#define NOCOPY 2
#define ASYNC 4
static void run(Display *dpy, Window win, const char *name, unsigned options)
{
	xcb_connection_t *c = XGetXCBConnection(dpy);
	struct timespec start, end;
#define N_BACK 8
	char test_name[128];
	struct buffer buffer[N_BACK];
	struct list mru;
	Window root;
	unsigned int width, height;
	unsigned border, depth;
	unsigned present_flags = 0;
	xcb_xfixes_region_t update = 0;
	int completed = 0;
	int queued = 0;
	uint32_t eid = 0;
	void *Q = NULL;
	int i, n;

	list_init(&mru);

	XGetGeometry(dpy, win,
		     &root, &i, &n, &width, &height, &border, &depth);

	_x_error_occurred = 0;

	for (n = 0; n < N_BACK; n++) {
		buffer[n].pixmap = xcb_generate_id(c);
		xcb_create_pixmap(c, depth, buffer[n].pixmap, win,
				  width, height);
		buffer[n].fence.xid = 0;
		buffer[n].fd = -1;
		buffer[n].id = n;
		if (options & DRI3) {
			xcb_dri3_buffer_from_pixmap_reply_t *reply;
			int *fds;

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
		}
		buffer[n].busy = 0;
		list_add(&buffer[n].link, &mru);
	}
	if (options & ASYNC)
		present_flags |= XCB_PRESENT_OPTION_ASYNC;
	if (options & NOCOPY) {
		update = xcb_generate_id(c);
		xcb_xfixes_create_region(c, update, 0, NULL);
		present_flags |= XCB_PRESENT_OPTION_COPY;
	}

	if (!(options & DRI3)) {
		eid = xcb_generate_id(c);
		xcb_present_select_input(c, eid, win,
					 (options & NOCOPY ? 0 : XCB_PRESENT_EVENT_MASK_IDLE_NOTIFY) |
					 XCB_PRESENT_EVENT_MASK_COMPLETE_NOTIFY);
		Q = xcb_register_for_special_xge(c, &xcb_present_id, eid, &stamp);
	}

	clock_gettime(CLOCK_MONOTONIC, &start);
	do {
		for (n = 0; n < 1000; n++) {
			struct buffer *tmp, *b = NULL;
retry:
			list_for_each_entry(tmp, &mru, link) {
				if (tmp->fence.xid)
					tmp->busy = !xshmfence_query(tmp->fence.addr);
				if (!tmp->busy) {
					b = tmp;
					break;
				}
			}
			if (options & DRI3) {
				if (b == NULL)
					goto retry;

				xshmfence_reset(b->fence.addr);
				queued--;
				completed++;
			} else while (b == NULL) {
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

			b->busy = (options & NOCOPY) == 0;
			xcb_present_pixmap(c, win, b->pixmap, b->id,
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

	if (options & DRI3) {
		struct buffer *b;
		XID pixmap;

		pixmap = xcb_generate_id(c);
		xcb_create_pixmap(c, depth, pixmap, win, width, height);
		xcb_present_pixmap(c, win, pixmap, 0xdeadbeef,
				   0, /* valid */
				   None, /* update */
				   0, /* x_off */
				   0, /* y_off */
				   None,
				   None, /* wait fence */
				   None,
				   0,
				   0, /* target msc */
				   0, /* divisor */
				   0, /* remainder */
				   0, NULL);
		xcb_flush(c);

		list_for_each_entry(b, &mru, link)
			xshmfence_await(b->fence.addr);

		xcb_free_pixmap(c, pixmap);
		completed += queued;
	} else while (queued) {
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

	if (update)
		xcb_xfixes_destroy_region(c, update);
	for (n = 0; n < N_BACK; n++) {
		if (buffer[n].fence.xid)
			dri3_fence_free(dpy, &buffer[n].fence);
		if (buffer[n].fd != -1)
			close(buffer[n].fd);
		xcb_free_pixmap(c, buffer[n].pixmap);
	}

	if (Q) {
		xcb_discard_reply(c, xcb_present_select_input_checked(c, eid, win, 0).sequence);
		XSync(dpy, True);
		xcb_unregister_for_special_event(c, Q);
	}

	test_name[0] = '\0';
	if (options) {
		snprintf(test_name, sizeof(test_name), "(%s%s%s )",
			 options & NOCOPY ? " no-copy" : "",
			 options & DRI3 ? " dri3" : "",
			 options & ASYNC ? " async" : "");
	}
	printf("%s%s: Completed %d presents in %.1fs, %.3fus each (%.1f FPS)\n",
	       name, test_name,
	       completed, elapsed(&start, &end) / 1000000,
	       elapsed(&start, &end) / completed,
	       completed / (elapsed(&start, &end) / 1000000));
}

struct perpixel {
	Window win;
	struct buffer buffer[N_BACK];
	struct list mru;
	uint32_t eid;
	void *Q;
	int queued;
};

static void perpixel(Display *dpy,
		     int max_width, int max_height, unsigned options)
{
	//const int sz = max_width * max_height;
	const int sz = 1048;
	struct perpixel *pp;
	xcb_connection_t *c = XGetXCBConnection(dpy);
	struct timespec start, end;
	char test_name[128];
	unsigned present_flags = 0;
	xcb_xfixes_region_t update = 0;
	int completed = 0;
	int i, n;

	pp = calloc(sz, sizeof(*pp));
	if (!pp)
		return;

	for (i = 0; i < sz; i++) {
		XSetWindowAttributes attr = { .override_redirect = 1 };
		int depth = DefaultDepth(dpy, DefaultScreen(dpy));
		pp[i].win = XCreateWindow(dpy, DefaultRootWindow(dpy),
					  i % max_width, i / max_width, 1, 1, 0, depth,
					  InputOutput,
					  DefaultVisual(dpy, DefaultScreen(dpy)),
					  CWOverrideRedirect, &attr);
		XMapWindow(dpy, pp[i].win);
		list_init(&pp[i].mru);
		for (n = 0; n < N_BACK; n++) {
			pp[i].buffer[n].pixmap = xcb_generate_id(c);
			xcb_create_pixmap(c, depth, pp[i].buffer[n].pixmap,
					  pp[i].win, 1, 1);
			pp[i].buffer[n].fence.xid = 0;
			pp[i].buffer[n].fd = -1;
			pp[i].buffer[n].id = n;
			if (options & DRI3) {
				xcb_dri3_buffer_from_pixmap_reply_t *reply;
				int *fds;

				if (dri3_create_fence(dpy, pp[i].win, &pp[i].buffer[n].fence))
					return;

				reply = xcb_dri3_buffer_from_pixmap_reply(c,
									  xcb_dri3_buffer_from_pixmap(c, pp[i].buffer[n].pixmap),
									  NULL);
				if (reply == NULL)
					return;

				fds = xcb_dri3_buffer_from_pixmap_reply_fds(c, reply);
				pp[i].buffer[n].fd = fds[0];
				free(reply);

				/* start idle */
				xshmfence_trigger(pp[i].buffer[n].fence.addr);
			}
			pp[i].buffer[n].busy = 0;
			list_add(&pp[i].buffer[n].link, &pp[i].mru);
		}

		if (!(options & DRI3)) {
			pp[i].eid = xcb_generate_id(c);
			xcb_present_select_input(c, pp[i].eid, pp[i].win,
						 (options & NOCOPY ? 0 : XCB_PRESENT_EVENT_MASK_IDLE_NOTIFY) |
						 XCB_PRESENT_EVENT_MASK_COMPLETE_NOTIFY);
			pp[i].Q = xcb_register_for_special_xge(c, &xcb_present_id, pp[i].eid, &stamp);
		}
		pp[i].queued = 0;
	}

	XSync(dpy, True);
	_x_error_occurred = 0;

	if (options & ASYNC)
		present_flags |= XCB_PRESENT_OPTION_ASYNC;
	if (options & NOCOPY) {
		update = xcb_generate_id(c);
		xcb_xfixes_create_region(c, update, 0, NULL);
		present_flags |= XCB_PRESENT_OPTION_COPY;
	}

	clock_gettime(CLOCK_MONOTONIC, &start);
	do {
		for (i = 0; i < sz; i++) {
			struct buffer *tmp, *b = NULL;
retry:
			list_for_each_entry(tmp, &pp[i].mru, link) {
				if (tmp->fence.xid)
					tmp->busy = !xshmfence_query(tmp->fence.addr);
				if (!tmp->busy) {
					b = tmp;
					break;
				}
			}
			if (options & DRI3) {
				if (b == NULL)
					goto retry;

				xshmfence_reset(b->fence.addr);
				pp[i].queued--;
				completed++;
			} else while (b == NULL) {
				xcb_present_generic_event_t *ev;

				ev = (xcb_present_generic_event_t *)
					xcb_wait_for_special_event(c, pp[i].Q);
				if (ev == NULL)
					abort();

				do {
					switch (ev->evtype) {
					case XCB_PRESENT_COMPLETE_NOTIFY:
						completed++;
						pp[i].queued--;
						break;

					case XCB_PRESENT_EVENT_IDLE_NOTIFY:
						{
							xcb_present_idle_notify_event_t *ie = (xcb_present_idle_notify_event_t *)ev;
							assert(ie->serial < N_BACK);
							pp[i].buffer[ie->serial].busy = 0;
							if (b == NULL)
								b = &pp[i].buffer[ie->serial];
							break;
						}
					}
					free(ev);
				} while ((ev = (xcb_present_generic_event_t *)xcb_poll_for_special_event(c, pp[i].Q)));
			}

			b->busy = (options & NOCOPY) == 0;
			xcb_present_pixmap(c, pp[i].win, b->pixmap, b->id,
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
			list_move(&b->link, &pp[i].mru);
			pp[i].queued++;
		}
		xcb_flush(c);
		clock_gettime(CLOCK_MONOTONIC, &end);
	} while (end.tv_sec < start.tv_sec + 10);

	for (i = 0; i < sz; i++) {
		if (options & DRI3) {
			int depth = DefaultDepth(dpy, DefaultScreen(dpy));
			struct buffer *b;
			XID pixmap;

			pixmap = xcb_generate_id(c);
			xcb_create_pixmap(c, depth, pixmap, pp[i].win, 1, 1);
			xcb_present_pixmap(c, pp[i].win, pixmap, 0xdeadbeef,
					   0, /* valid */
					   None, /* update */
					   0, /* x_off */
					   0, /* y_off */
					   None,
					   None, /* wait fence */
					   None,
					   0,
					   0, /* target msc */
					   0, /* divisor */
					   0, /* remainder */
					   0, NULL);
			xcb_flush(c);

			list_for_each_entry(b, &pp[i].mru, link)
				xshmfence_await(b->fence.addr);

			xcb_free_pixmap(c, pixmap);
			completed += pp[i].queued;
		} else while (pp[i].queued) {
			xcb_present_generic_event_t *ev;

			ev = (xcb_present_generic_event_t *)
				xcb_wait_for_special_event(c, pp[i].Q);
			if (ev == NULL)
				abort();

			do {
				switch (ev->evtype) {
				case XCB_PRESENT_COMPLETE_NOTIFY:
					completed++;
					pp[i].queued--;
					break;

				case XCB_PRESENT_EVENT_IDLE_NOTIFY:
					break;
				}
				free(ev);
			} while ((ev = (xcb_present_generic_event_t *)xcb_poll_for_special_event(c, pp[i].Q)));
		}
	}
	clock_gettime(CLOCK_MONOTONIC, &end);

	if (update)
		xcb_xfixes_destroy_region(c, update);

	for (i = 0; i < sz; i++) {
		for (n = 0; n < N_BACK; n++) {
			if (pp[i].buffer[n].fence.xid)
				dri3_fence_free(dpy, &pp[i].buffer[n].fence);
			if (pp[i].buffer[n].fd != -1)
				close(pp[i].buffer[n].fd);
			xcb_free_pixmap(c, pp[i].buffer[n].pixmap);
		}

		if (pp[i].Q) {
			xcb_discard_reply(c, xcb_present_select_input_checked(c, pp[i].eid, pp[i].win, 0).sequence);
			XSync(dpy, True);
			xcb_unregister_for_special_event(c, pp[i].Q);
		}

		XDestroyWindow(dpy, pp[i].win);
	}
	free(pp);

	test_name[0] = '\0';
	if (options) {
		snprintf(test_name, sizeof(test_name), "(%s%s%s )",
			 options & NOCOPY ? " no-copy" : "",
			 options & DRI3 ? " dri3" : "",
			 options & ASYNC ? " async" : "");
	}
	printf("%s%s: Completed %d presents in %.1fs, %.3fus each (%.1f FPS)\n",
	       __func__, test_name,
	       completed, elapsed(&start, &end) / 1000000,
	       elapsed(&start, &end) / completed,
	       completed / (elapsed(&start, &end) / 1000000));
}

static int isqrt(int x)
{
	int i;

	for (i = 2; i*i < x; i++)
		;
	return i;
}

struct sibling {
	pthread_t thread;
	Display *dpy;
	int x, y;
	int width, height;
	unsigned options;
};

static void *sibling(void *arg)
{
	struct sibling *s = arg;
	XSetWindowAttributes attr = { .override_redirect = 1 };
	Window win = XCreateWindow(s->dpy, DefaultRootWindow(s->dpy),
				   s->x, s->y, s->width, s->height, 0,
				   DefaultDepth(s->dpy, DefaultScreen(s->dpy)),
				   InputOutput,
				   DefaultVisual(s->dpy, DefaultScreen(s->dpy)),
				   CWOverrideRedirect, &attr);
	XMapWindow(s->dpy, win);
	run(s->dpy, win, "sibling", s->options);
	return NULL;
}

static void siblings(Display *dpy,
		     int max_width, int max_height, int ncpus, unsigned options)
{
	int sq_ncpus = isqrt(ncpus);
	int width = max_width / sq_ncpus;
	int height = max_height/ sq_ncpus;
	struct sibling s[ncpus];
	int child;

	if (ncpus <= 1)
		return;

	for (child = 0; child < ncpus; child++) {
		s[child].dpy = dpy;
		s[child].x = (child % sq_ncpus) * width;
		s[child].y = (child / sq_ncpus) * height;
		s[child].width = width;
		s[child].height = height;
		s[child].options = options;
		pthread_create(&s[child].thread, NULL, sibling, &s[child]);
	}

	for (child = 0; child < ncpus; child++)
		pthread_join(s[child].thread, NULL);
}

static void cousins(int max_width, int max_height, int ncpus, unsigned options)
{
	int sq_ncpus = isqrt(ncpus);
	int width = max_width / sq_ncpus;
	int height = max_height/ sq_ncpus;
	int child;

	if (ncpus <= 1)
		return;

	for (child = 0; child < ncpus; child++) {
		for (; fork() == 0; exit(0)) {
			int x = (child % sq_ncpus) * width;
			int y = (child / sq_ncpus) * height;
			XSetWindowAttributes attr = { .override_redirect = 1 };
			Display *dpy = XOpenDisplay(NULL);
			Window win = XCreateWindow(dpy, DefaultRootWindow(dpy),
						   x, y, width, height, 0,
						   DefaultDepth(dpy, DefaultScreen(dpy)),
						   InputOutput,
						   DefaultVisual(dpy, DefaultScreen(dpy)),
						   CWOverrideRedirect, &attr);
			XMapWindow(dpy, win);
			run(dpy, win, "cousin", options);
		}
	}

	while (child) {
		int status = -1;
		pid_t pid = wait(&status);
		if (pid == -1)
			continue;
		child--;
	}
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

static int has_xfixes(Display *dpy)
{
	xcb_connection_t *c = XGetXCBConnection(dpy);
	const xcb_query_extension_reply_t *ext;
	void *reply;

	ext = xcb_get_extension_data(c, &xcb_xfixes_id);
	if (ext == NULL || !ext->present)
		return 0;

	reply = xcb_xfixes_query_version_reply(c,
					       xcb_xfixes_query_version(c,
									XCB_XFIXES_MAJOR_VERSION,
									XCB_XFIXES_MINOR_VERSION),
					       NULL);
	free(reply);

	return reply != NULL;
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

static void loop(Display *dpy, XRRScreenResources *res, unsigned options)
{
	Window root = DefaultRootWindow(dpy);
	Window win;
	XSetWindowAttributes attr;
	int i, j;

	attr.override_redirect = 1;

	run(dpy, root, "off", options);
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

			run(dpy, root, "root", options);
			XSync(dpy, True);

			win = XCreateWindow(dpy, root,
					    0, 0, mode->width, mode->height, 0,
					    DefaultDepth(dpy, DefaultScreen(dpy)),
					    InputOutput,
					    DefaultVisual(dpy, DefaultScreen(dpy)),
					    CWOverrideRedirect, &attr);
			fullscreen(dpy, win);
			XMapWindow(dpy, win);
			run(dpy, win, "fullscreen", options);
			XDestroyWindow(dpy, win);
			XSync(dpy, True);

			win = XCreateWindow(dpy, root,
					    0, 0, mode->width, mode->height, 0,
					    DefaultDepth(dpy, DefaultScreen(dpy)),
					    InputOutput,
					    DefaultVisual(dpy, DefaultScreen(dpy)),
					    CWOverrideRedirect, &attr);
			XMapWindow(dpy, win);
			run(dpy, win, "windowed", options);
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
				damage = XDamageCreate(dpy, win, XDamageReportNonEmpty);
				XMapWindow(dpy, win);
				XSync(dpy, True);
				if (!_x_error_occurred)
					run(dpy, win, "composited", options);
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
			XMapWindow(dpy, win);
			run(dpy, win, "half", options);
			XDestroyWindow(dpy, win);
			XSync(dpy, True);

			perpixel(dpy, mode->width, mode->height, options);

			siblings(dpy, mode->width, mode->height,
				 sysconf(_SC_NPROCESSORS_ONLN),
				 options);

			cousins(mode->width, mode->height,
				sysconf(_SC_NPROCESSORS_ONLN),
				options);

			XRRSetCrtcConfig(dpy, res, output->crtcs[c], CurrentTime,
					 0, 0, None, RR_Rotate_0, NULL, 0);
		}

		XRRFreeOutputInfo(output);
	}

}

int main(void)
{
	Display *dpy;
	XRRScreenResources *res;
	XRRCrtcInfo **original_crtc;
	int i;

	XInitThreads();

	dpy = XOpenDisplay(NULL);
	if (dpy == NULL)
		return 77;

	if (!has_present(dpy))
		return 77;

	if (DPMSQueryExtension(dpy, &i, &i))
		DPMSDisable(dpy);

	signal(SIGALRM, SIG_IGN);
	XSetErrorHandler(_check_error_handler);

	res = NULL;
	if (XRRQueryVersion(dpy, &i, &i))
		res = _XRRGetScreenResourcesCurrent(dpy, DefaultRootWindow(dpy));
	if (res == NULL)
		return 77;

	original_crtc = malloc(sizeof(XRRCrtcInfo *)*res->ncrtc);
	for (i = 0; i < res->ncrtc; i++)
		original_crtc[i] = XRRGetCrtcInfo(dpy, res, res->crtcs[i]);

	printf("noutput=%d, ncrtc=%d\n", res->noutput, res->ncrtc);
	for (i = 0; i < res->ncrtc; i++)
		XRRSetCrtcConfig(dpy, res, res->crtcs[i], CurrentTime,
				 0, 0, None, RR_Rotate_0, NULL, 0);

	loop(dpy, res, 0);
	loop(dpy, res, ASYNC);
	if (has_xfixes(dpy))
		loop(dpy, res, NOCOPY);
	if (has_dri3(dpy)) {
		loop(dpy, res, DRI3);
		loop(dpy, res, DRI3 | ASYNC);
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

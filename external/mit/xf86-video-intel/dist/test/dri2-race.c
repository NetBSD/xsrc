#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/Xfixes.h>
#include <X11/extensions/Xcomposite.h>
#include <X11/Xlib-xcb.h>
#include <xcb/xcb.h>
#include <xcb/xcbext.h>
#include <xcb/dri2.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>

#include <xf86drm.h>
#include <drm.h>
#include <setjmp.h>

#include "dri2.h"

#define COUNT 60

#define N_DIVISORS 3
static const int divisors[N_DIVISORS] = { 0, 1, 16 };

static jmp_buf error_handler[4];
static int have_error_handler;

#define error_get() \
	setjmp(error_handler[have_error_handler++])

#define error_put() \
	have_error_handler--

static int (*saved_io_error)(Display *dpy);

static int io_error(Display *dpy)
{
	if (have_error_handler)
		longjmp(error_handler[--have_error_handler], 0);

	return saved_io_error(dpy);
}

static int x_error(Display *dpy, XErrorEvent *e)
{
	return Success;
}

static uint32_t upper_32_bits(uint64_t val)
{
	return val >> 32;
}

static uint32_t lower_32_bits(uint64_t val)
{
	return val & 0xffffffff;
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

static void swap_buffers(Display *dpy, Window win, int divisor,
			 unsigned int *attachments, int nattachments)
{
	xcb_connection_t *c = XGetXCBConnection(dpy);
	unsigned int seq[2];

	seq[0] = xcb_dri2_swap_buffers_unchecked(c, win,
						 0, 0, 0, divisor, 0, 0).sequence;


	seq[1] = xcb_dri2_get_buffers_unchecked(c, win,
						nattachments, nattachments,
						attachments).sequence;

	xcb_flush(c);
	xcb_discard_reply(c, seq[0]);
	xcb_discard_reply(c, seq[1]);
}

#define COMPOSITE 1

static int has_composite(Display *dpy)
{
	Display *dummy = NULL;
	int event, error;
	int major = -1, minor = -1;

	if (dpy == NULL)
		dummy = dpy = XOpenDisplay(NULL);

	if (XCompositeQueryExtension(dpy, &event, &error))
		XCompositeQueryVersion(dpy, &major, &minor);

	if (dummy)
		XCloseDisplay(dummy);

	return major > 0 || minor >= 4;
}

static void race_window(Display *dpy, int width, int height,
			unsigned int *attachments, int nattachments,
			unsigned flags, const char *name)
{
	Window win;
	XSetWindowAttributes attr;
	int count, loop, n;
	DRI2Buffer *buffers;

	if (flags & COMPOSITE && !has_composite(dpy))
		return;

	printf("%s(%s)\n", __func__, name);

	/* Be nasty and install a fullscreen window on top so that we
	 * can guarantee we do not get clipped by children.
	 */
	attr.override_redirect = 1;
	for (n = 0; n < N_DIVISORS; n++) {
		loop = 256 >> ffs(divisors[n]);
		printf("DRI2SwapBuffers(divisor=%d), loop=%d", divisors[n], loop);
		fflush(stdout);
		do {
			win = XCreateWindow(dpy, DefaultRootWindow(dpy),
					0, 0, width, height, 0,
					DefaultDepth(dpy, DefaultScreen(dpy)),
					InputOutput,
					DefaultVisual(dpy, DefaultScreen(dpy)),
					CWOverrideRedirect, &attr);
			if (flags & COMPOSITE)
				XCompositeRedirectWindow(dpy, win, CompositeRedirectManual);
			XMapWindow(dpy, win);

			DRI2CreateDrawable(dpy, win);

			buffers = DRI2GetBuffers(dpy, win, &width, &height,
					attachments, nattachments, &count);
			if (count != nattachments)
				return;

			free(buffers);
			for (count = 0; count < loop; count++)
				DRI2SwapBuffers(dpy, win, 0, divisors[n], count & (divisors[n]-1));
			XDestroyWindow(dpy, win);
			printf("."); fflush(stdout);
		} while (--loop);
		printf("*\n");
	}

	for (n = 0; n < N_DIVISORS; n++) {
		loop = 256 >> ffs(divisors[n]);
		printf("xcb_dri2_swap_buffers(divisor=%d), loops=%d", divisors[n], loop);
		fflush(stdout);
		do {
			win = XCreateWindow(dpy, DefaultRootWindow(dpy),
					0, 0, width, height, 0,
					DefaultDepth(dpy, DefaultScreen(dpy)),
					InputOutput,
					DefaultVisual(dpy, DefaultScreen(dpy)),
					CWOverrideRedirect, &attr);
			if (flags & COMPOSITE)
				XCompositeRedirectWindow(dpy, win, CompositeRedirectManual);
			XMapWindow(dpy, win);

			DRI2CreateDrawable(dpy, win);

			buffers = DRI2GetBuffers(dpy, win, &width, &height,
					attachments, nattachments, &count);
			if (count != nattachments)
				return;

			free(buffers);
			for (count = 0; count < loop; count++)
				swap_buffers(dpy, win, divisors[n], attachments, nattachments);
			XDestroyWindow(dpy, win);
			printf("."); fflush(stdout);
		} while (--loop);
		printf("*\n");
	}

	for (n = 0; n < N_DIVISORS; n++) {
		loop = 256 >> ffs(divisors[n]);
		printf("DRI2WaitMsc(divisor=%d), loop=%d", divisors[n], loop);
		fflush(stdout);
		do {
			uint64_t ignore, msc;
			xcb_connection_t *c = XGetXCBConnection(dpy);

			win = XCreateWindow(dpy, DefaultRootWindow(dpy),
					0, 0, width, height, 0,
					DefaultDepth(dpy, DefaultScreen(dpy)),
					InputOutput,
					DefaultVisual(dpy, DefaultScreen(dpy)),
					CWOverrideRedirect, &attr);
			if (flags & COMPOSITE)
				XCompositeRedirectWindow(dpy, win, CompositeRedirectManual);
			XMapWindow(dpy, win);

			DRI2CreateDrawable(dpy, win);
			DRI2GetMSC(dpy, win, &ignore, &msc, &ignore);
			msc++;
			for (count = 0; count < loop; count++) {
				xcb_discard_reply(c,
						xcb_dri2_wait_msc(c, win,
							upper_32_bits(msc),
							lower_32_bits(msc),
							0, 0, 0, 0).sequence);
				msc += divisors[n];
			}
			XFlush(dpy);
			XDestroyWindow(dpy, win);
			printf("."); fflush(stdout);
		} while (--loop);
		printf("*\n");
	}

	XSync(dpy, 1);
	sleep(2);
	XSync(dpy, 1);
}

static int rand_size(int max)
{
	return 1 + (rand() % (max - 1));
}

static void race_resize(Display *dpy, int width, int height,
			unsigned int *attachments, int nattachments,
			unsigned flags, const char *name)
{
	Window win;
	XSetWindowAttributes attr;
	int count, loop, n;
	DRI2Buffer *buffers;

	if (flags & COMPOSITE && !has_composite(dpy))
		return;

	printf("%s(%s)\n", __func__, name);

	attr.override_redirect = 1;
	for (n = 0; n < N_DIVISORS; n++) {
		win = XCreateWindow(dpy, DefaultRootWindow(dpy),
				    0, 0, width, height, 0,
				    DefaultDepth(dpy, DefaultScreen(dpy)),
				    InputOutput,
				    DefaultVisual(dpy, DefaultScreen(dpy)),
				    CWOverrideRedirect, &attr);
		if (flags & COMPOSITE)
			XCompositeRedirectWindow(dpy, win, CompositeRedirectManual);
		XMapWindow(dpy, win);

		DRI2CreateDrawable(dpy, win);

		loop = 256 >> ffs(divisors[n]);
		printf("DRI2SwapBuffers(divisor=%d), loop=%d", divisors[n], loop);
		fflush(stdout);
		do {
			int w, h;

			buffers = DRI2GetBuffers(dpy, win, &w, &h,
					attachments, nattachments, &count);
			if (count != nattachments)
				return;

			free(buffers);
			for (count = 0; count < loop; count++)
				DRI2SwapBuffers(dpy, win, 0, divisors[n], count & (divisors[n]-1));
			XResizeWindow(dpy, win, rand_size(width), rand_size(height));
			printf("."); fflush(stdout);
		} while (--loop);
		XDestroyWindow(dpy, win);
		XSync(dpy, True);
		printf("*\n");
	}

	for (n = 0; n < N_DIVISORS; n++) {
		win = XCreateWindow(dpy, DefaultRootWindow(dpy),
				    0, 0, width, height, 0,
				    DefaultDepth(dpy, DefaultScreen(dpy)),
				    InputOutput,
				    DefaultVisual(dpy, DefaultScreen(dpy)),
				    CWOverrideRedirect, &attr);
		if (flags & COMPOSITE)
			XCompositeRedirectWindow(dpy, win, CompositeRedirectManual);
		XMapWindow(dpy, win);

		DRI2CreateDrawable(dpy, win);

		loop = 256 >> ffs(divisors[n]);
		printf("xcb_dri2_swap_buffers(divisor=%d), loops=%d", divisors[n], loop);
		fflush(stdout);
		do {
			int w, h;

			buffers = DRI2GetBuffers(dpy, win, &w, &h,
					attachments, nattachments, &count);
			if (count != nattachments)
				return;

			free(buffers);
			for (count = 0; count < loop; count++)
				swap_buffers(dpy, win, divisors[n], attachments, nattachments);
			XResizeWindow(dpy, win, rand_size(width), rand_size(height));
			printf("."); fflush(stdout);
		} while (--loop);
		XDestroyWindow(dpy, win);
		XSync(dpy, True);
		printf("*\n");
	}

	for (n = 0; n < N_DIVISORS; n++) {
		win = XCreateWindow(dpy, DefaultRootWindow(dpy),
				    0, 0, width, height, 0,
				    DefaultDepth(dpy, DefaultScreen(dpy)),
				    InputOutput,
				    DefaultVisual(dpy, DefaultScreen(dpy)),
				    CWOverrideRedirect, &attr);
		if (flags & COMPOSITE)
			XCompositeRedirectWindow(dpy, win, CompositeRedirectManual);
		XMapWindow(dpy, win);

		DRI2CreateDrawable(dpy, win);

		loop = 256 >> ffs(divisors[n]);
		printf("DRI2WaitMsc(divisor=%d), loop=%d", divisors[n], loop);
		fflush(stdout);
		do {
			uint64_t ignore, msc;
			xcb_connection_t *c = XGetXCBConnection(dpy);

			DRI2GetMSC(dpy, win, &ignore, &msc, &ignore);
			msc++;
			for (count = 0; count < loop; count++) {
				xcb_discard_reply(c,
						xcb_dri2_wait_msc(c, win,
							upper_32_bits(msc),
							lower_32_bits(msc),
							0, 0, 0, 0).sequence);
				msc += divisors[n];
			}
			XFlush(dpy);
			XResizeWindow(dpy, win, rand_size(width), rand_size(height));
			printf("."); fflush(stdout);
		} while (--loop);
		XDestroyWindow(dpy, win);
		XSync(dpy, True);
		printf("*\n");
	}

	XSync(dpy, 1);
	sleep(2);
	XSync(dpy, 1);
}

static void race_manager(Display *dpy, int width, int height,
			 unsigned int *attachments, int nattachments,
			 unsigned flags, const char *name)
{
	Display *mgr = XOpenDisplay(NULL);
	Window win;
	XSetWindowAttributes attr;
	int count, loop, n;
	DRI2Buffer *buffers;

	if (flags & COMPOSITE && !has_composite(dpy))
		return;

	printf("%s(%s)\n", __func__, name);

	/* Be nasty and install a fullscreen window on top so that we
	 * can guarantee we do not get clipped by children.
	 */
	attr.override_redirect = 1;
	for (n = 0; n < N_DIVISORS; n++) {
		printf("DRI2SwapBuffers(divisor=%d)", divisors[n]);
		fflush(stdout);
		loop = 256 >> ffs(divisors[n]);
		do {
			win = XCreateWindow(dpy, DefaultRootWindow(dpy),
					0, 0, width, height, 0,
					DefaultDepth(dpy, DefaultScreen(dpy)),
					InputOutput,
					DefaultVisual(dpy, DefaultScreen(dpy)),
					CWOverrideRedirect, &attr);
			if (flags & COMPOSITE)
				XCompositeRedirectWindow(dpy, win, CompositeRedirectManual);
			XMapWindow(dpy, win);

			DRI2CreateDrawable(dpy, win);

			buffers = DRI2GetBuffers(dpy, win, &width, &height,
					attachments, nattachments, &count);
			if (count != nattachments)
				return;

			free(buffers);
			for (count = 0; count < loop; count++)
				DRI2SwapBuffers(dpy, win, 0, divisors[n], count & (divisors[n]-1));
			XFlush(dpy);
			XDestroyWindow(mgr, win);
			XFlush(mgr);
			printf("."); fflush(stdout);
		} while (--loop);
		printf("*\n");
	}

	for (n = 0; n < N_DIVISORS; n++) {
		printf("xcb_dri2_swap_buffers(divisor=%d)", divisors[n]);
		fflush(stdout);
		loop = 256 >> ffs(divisors[n]);
		do {
			win = XCreateWindow(dpy, DefaultRootWindow(dpy),
					0, 0, width, height, 0,
					DefaultDepth(dpy, DefaultScreen(dpy)),
					InputOutput,
					DefaultVisual(dpy, DefaultScreen(dpy)),
					CWOverrideRedirect, &attr);
			if (flags & COMPOSITE)
				XCompositeRedirectWindow(dpy, win, CompositeRedirectManual);
			XMapWindow(dpy, win);

			DRI2CreateDrawable(dpy, win);

			buffers = DRI2GetBuffers(dpy, win, &width, &height,
					attachments, nattachments, &count);
			if (count != nattachments)
				return;

			free(buffers);
			for (count = 0; count < loop; count++)
				swap_buffers(dpy, win, divisors[n], attachments, nattachments);
			XFlush(dpy);
			XDestroyWindow(mgr, win);
			XFlush(mgr);
			printf("."); fflush(stdout);
		} while (--loop);
		printf("*\n");
	}

	for (n = 0; n < N_DIVISORS; n++) {
		printf("DRI2WaitMsc(divisor=%d)", divisors[n]);
		fflush(stdout);
		loop = 256 >> ffs(divisors[n]);
		do {
			uint64_t ignore, msc;
			xcb_connection_t *c = XGetXCBConnection(dpy);

			win = XCreateWindow(dpy, DefaultRootWindow(dpy),
					0, 0, width, height, 0,
					DefaultDepth(dpy, DefaultScreen(dpy)),
					InputOutput,
					DefaultVisual(dpy, DefaultScreen(dpy)),
					CWOverrideRedirect, &attr);
			if (flags & COMPOSITE)
				XCompositeRedirectWindow(dpy, win, CompositeRedirectManual);
			XMapWindow(dpy, win);

			DRI2CreateDrawable(dpy, win);
			DRI2GetMSC(dpy, win, &ignore, &msc, &ignore);
			msc++;
			for (count = 0; count < loop; count++) {
				xcb_discard_reply(c,
						xcb_dri2_wait_msc(c, win,
							upper_32_bits(msc),
							lower_32_bits(msc),
							0, 0, 0, 0).sequence);
				msc += divisors[n];
			}
			XFlush(dpy);
			XDestroyWindow(mgr, win);
			XFlush(mgr);
			printf("."); fflush(stdout);
		} while (--loop);
		printf("*\n");
	}

	XSync(dpy, 1);
	XSync(mgr, 1);
	sleep(2);
	XSync(dpy, 1);
	XSync(mgr, 1);

	XCloseDisplay(mgr);
}

static void race_close(int width, int height,
		       unsigned int *attachments, int nattachments,
		       unsigned flags, const char *name)
{
	XSetWindowAttributes attr;
	int count, loop, n;

	if (flags & COMPOSITE && !has_composite(NULL))
		return;

	printf("%s(%s)\n", __func__, name);

	/* Be nasty and install a fullscreen window on top so that we
	 * can guarantee we do not get clipped by children.
	 */
	attr.override_redirect = 1;
	for (n = 0; n < N_DIVISORS; n++) {
		printf("DRI2SwapBuffers(divisor=%d)", divisors[n]);
		fflush(stdout);
		loop = 256 >> ffs(divisors[n]);
		do {
			Display *dpy = XOpenDisplay(NULL);
			Window win = XCreateWindow(dpy, DefaultRootWindow(dpy),
					0, 0, width, height, 0,
					DefaultDepth(dpy, DefaultScreen(dpy)),
					InputOutput,
					DefaultVisual(dpy, DefaultScreen(dpy)),
					CWOverrideRedirect, &attr);
			if (flags & COMPOSITE)
				XCompositeRedirectWindow(dpy, win, CompositeRedirectManual);
			XMapWindow(dpy, win);

			DRI2CreateDrawable(dpy, win);
			free(DRI2GetBuffers(dpy, win, &width, &height,
						attachments, nattachments, &count));
			if (count != nattachments)
				return;

			for (count = 0; count < loop; count++)
				DRI2SwapBuffers(dpy, win, 0, divisors[n], count & (divisors[n]-1));
			XCloseDisplay(dpy);
			printf("."); fflush(stdout);
		} while (--loop);
		printf("*\n");
	}

	for (n = 0; n < N_DIVISORS; n++) {
		printf("xcb_dri2_swap_buffers(divisor=%d)", divisors[n]);
		fflush(stdout);
		loop = 256 >> ffs(divisors[n]);
		do {
			Display *dpy = XOpenDisplay(NULL);
			Window win = XCreateWindow(dpy, DefaultRootWindow(dpy),
					0, 0, width, height, 0,
					DefaultDepth(dpy, DefaultScreen(dpy)),
					InputOutput,
					DefaultVisual(dpy, DefaultScreen(dpy)),
					CWOverrideRedirect, &attr);
			if (flags & COMPOSITE)
				XCompositeRedirectWindow(dpy, win, CompositeRedirectManual);
			XMapWindow(dpy, win);

			DRI2CreateDrawable(dpy, win);
			free(DRI2GetBuffers(dpy, win, &width, &height,
						attachments, nattachments, &count));
			if (count != nattachments)
				return;

			for (count = 0; count < loop; count++)
				swap_buffers(dpy, win, divisors[n], attachments, nattachments);
			XCloseDisplay(dpy);
			printf("."); fflush(stdout);
		} while (--loop);
		printf("*\n");
	}

	for (n = 0; n < N_DIVISORS; n++) {
		printf("DRI2WaitMsc(divisor=%d)", divisors[n]);
		fflush(stdout);
		loop = 256 >> ffs(divisors[n]);
		do {
			uint64_t ignore, msc;
			Display *dpy = XOpenDisplay(NULL);
			xcb_connection_t *c = XGetXCBConnection(dpy);
			Window win = XCreateWindow(dpy, DefaultRootWindow(dpy),
					0, 0, width, height, 0,
					DefaultDepth(dpy, DefaultScreen(dpy)),
					InputOutput,
					DefaultVisual(dpy, DefaultScreen(dpy)),
					CWOverrideRedirect, &attr);
			if (flags & COMPOSITE)
				XCompositeRedirectWindow(dpy, win, CompositeRedirectManual);
			XMapWindow(dpy, win);

			DRI2CreateDrawable(dpy, win);
			DRI2GetMSC(dpy, win, &ignore, &msc, &ignore);
			msc++;
			for (count = 0; count < loop; count++) {
				xcb_discard_reply(c,
						xcb_dri2_wait_msc(c, win,
							upper_32_bits(msc),
							lower_32_bits(msc),
							0, 0, 0, 0).sequence);
				msc += divisors[n];
			}
			XFlush(dpy);
			XCloseDisplay(dpy);
			printf("."); fflush(stdout);
		} while (--loop);
		printf("*\n");
	}
}

static void race_client(int width, int height,
			unsigned int *attachments, int nattachments,
			unsigned flags, const char *name)
{
	Display *mgr = XOpenDisplay(NULL);
	XSetWindowAttributes attr;
	int count, loop, n;

	if (flags & COMPOSITE && !has_composite(NULL))
		return;

	printf("%s(%s)\n", __func__, name);

	/* Be nasty and install a fullscreen window on top so that we
	 * can guarantee we do not get clipped by children.
	 */
	attr.override_redirect = 1;
	for (n = 0; n < N_DIVISORS; n++) {
		printf("DRI2SwapBuffers(divisor=%d)", divisors[n]);
		fflush(stdout);
		loop = 256 >> ffs(divisors[n]);
		do {
			Display *dpy = XOpenDisplay(NULL);
			Window win;

			if (error_get()) {
				XCloseDisplay(dpy);
				printf("+"); fflush(stdout);
				continue;
			}

			win = XCreateWindow(dpy, DefaultRootWindow(dpy),
					    0, 0, width, height, 0,
					    DefaultDepth(dpy, DefaultScreen(dpy)),
					    InputOutput,
					    DefaultVisual(dpy, DefaultScreen(dpy)),
					    CWOverrideRedirect, &attr);
			if (flags & COMPOSITE)
				XCompositeRedirectWindow(dpy, win, CompositeRedirectManual);
			XMapWindow(dpy, win);

			DRI2CreateDrawable(dpy, win);
			free(DRI2GetBuffers(dpy, win, &width, &height,
					    attachments, nattachments, &count));
			if (count == nattachments) {
				for (count = 0; count < loop; count++)
					DRI2SwapBuffers(dpy, win, 0, divisors[n], count & (divisors[n]-1));
			}

			XFlush(dpy);
			XKillClient(mgr, win);
			XFlush(mgr);

			XCloseDisplay(dpy);
			printf("."); fflush(stdout);

			error_put();
		} while (--loop);
		printf("*\n");
	}

	for (n = 0; n < N_DIVISORS; n++) {
		printf("xcb_dri2_swap_buffers(divisor=%d)", divisors[n]);
		fflush(stdout);
		loop = 256 >> ffs(divisors[n]);
		do {
			Display *dpy = XOpenDisplay(NULL);
			Window win;

			if (error_get()) {
				XCloseDisplay(dpy);
				printf("+"); fflush(stdout);
				continue;
			}

			win = XCreateWindow(dpy, DefaultRootWindow(dpy),
					    0, 0, width, height, 0,
					    DefaultDepth(dpy, DefaultScreen(dpy)),
					    InputOutput,
					    DefaultVisual(dpy, DefaultScreen(dpy)),
					    CWOverrideRedirect, &attr);
			if (flags & COMPOSITE)
				XCompositeRedirectWindow(dpy, win, CompositeRedirectManual);
			XMapWindow(dpy, win);

			DRI2CreateDrawable(dpy, win);
			free(DRI2GetBuffers(dpy, win, &width, &height,
					    attachments, nattachments, &count));
			if (count == nattachments) {
				for (count = 0; count < loop; count++)
					swap_buffers(dpy, win, divisors[n], attachments, nattachments);
			}

			XFlush(dpy);
			XKillClient(mgr, win);
			XFlush(mgr);

			XCloseDisplay(dpy);
			printf("."); fflush(stdout);

			error_put();
		} while (--loop);
		printf("*\n");
	}

	for (n = 0; n < N_DIVISORS; n++) {
		printf("DRI2WaitMsc(divisor=%d)", divisors[n]);
		fflush(stdout);
		loop = 256 >> ffs(divisors[n]);
		do {
			Display *dpy = XOpenDisplay(NULL);
			uint64_t ignore, msc;
			xcb_connection_t *c;
			Window win;

			if (error_get()) {
				XCloseDisplay(dpy);
				printf("+"); fflush(stdout);
				continue;
			}

			win = XCreateWindow(dpy, DefaultRootWindow(dpy),
					    0, 0, width, height, 0,
					    DefaultDepth(dpy, DefaultScreen(dpy)),
					    InputOutput,
					    DefaultVisual(dpy, DefaultScreen(dpy)),
					    CWOverrideRedirect, &attr);
			if (flags & COMPOSITE)
				XCompositeRedirectWindow(dpy, win, CompositeRedirectManual);
			XMapWindow(dpy, win);

			DRI2CreateDrawable(dpy, win);
			DRI2GetMSC(dpy, win, &ignore, &msc, &ignore);
			c = XGetXCBConnection(dpy);
			msc++;
			for (count = 0; count < loop; count++) {
				xcb_discard_reply(c,
						  xcb_dri2_wait_msc(c, win,
								    upper_32_bits(msc),
								    lower_32_bits(msc),
								    0, 0, 0, 0).sequence);
				msc += divisors[n];
			}

			XFlush(dpy);
			XKillClient(mgr, win);
			XFlush(mgr);

			XCloseDisplay(dpy);
			printf("."); fflush(stdout);

			error_put();
		} while (--loop);
		printf("*\n");
	}

	XCloseDisplay(mgr);
}

int main(void)
{
	Display *dpy;
	int width, height, fd;
	unsigned int attachments[] = {
		DRI2BufferBackLeft,
		DRI2BufferFrontLeft,
	};

	saved_io_error = XSetIOErrorHandler(io_error);
	XSetErrorHandler(x_error);

	dpy = XOpenDisplay(NULL);
	if (dpy == NULL)
		return 77;

	fd = dri2_open(dpy);
	if (fd < 0)
		return 1;

	width = WidthOfScreen(DefaultScreenOfDisplay(dpy));
	height = HeightOfScreen(DefaultScreenOfDisplay(dpy));
	race_window(dpy, width, height, attachments, 1, 0, "fullscreen");
	race_window(dpy, width, height, attachments, 1, COMPOSITE, "composite fullscreen");
	race_window(dpy, width, height, attachments, 2, 0, "fullscreen (with front)");
	race_window(dpy, width, height, attachments, 2, COMPOSITE, "composite fullscreen (with front)");

	race_resize(dpy, width, height, attachments, 1, 0, "");
	race_resize(dpy, width, height, attachments, 1, COMPOSITE, "composite");
	race_resize(dpy, width, height, attachments, 2, 0, "with front");
	race_resize(dpy, width, height, attachments, 2, COMPOSITE, "composite with front");

	race_manager(dpy, width, height, attachments, 1, 0, "fullscreen");
	race_manager(dpy, width, height, attachments, 1, COMPOSITE, "composite fullscreen");
	race_manager(dpy, width, height, attachments, 2, 0, "fullscreen (with front)");
	race_manager(dpy, width, height, attachments, 2, COMPOSITE, "composite fullscreen (with front)");

	race_close(width, height, attachments, 1, 0, "fullscreen");
	race_close(width, height, attachments, 1, COMPOSITE, "composite fullscreen");
	race_close(width, height, attachments, 2, 0, "fullscreen (with front)");
	race_close(width, height, attachments, 2, COMPOSITE, "composite fullscreen (with front)");

	race_client(width, height, attachments, 1, 0, "fullscreen");
	race_client(width, height, attachments, 1, COMPOSITE, "composite fullscreen");
	race_client(width, height, attachments, 2, 0, "fullscreen (with front)");
	race_client(width, height, attachments, 2, COMPOSITE, "composite fullscreen (with front)");

	width /= 2;
	height /= 2;
	race_window(dpy, width, height, attachments, 1, 0, "windowed");
	race_window(dpy, width, height, attachments, 1, COMPOSITE, "composite windowed");
	race_window(dpy, width, height, attachments, 2, 0, "windowed (with front)");
	race_window(dpy, width, height, attachments, 2, COMPOSITE, "composite windowed (with front)");

	race_manager(dpy, width, height, attachments, 1, 0, "windowed");
	race_manager(dpy, width, height, attachments, 1, COMPOSITE, "composite windowed");
	race_manager(dpy, width, height, attachments, 2, 0, "windowed (with front)");
	race_manager(dpy, width, height, attachments, 2, COMPOSITE, "composite windowed (with front)");

	race_close(width, height, attachments, 1, 0, "windowed");
	race_close(width, height, attachments, 1, COMPOSITE, "composite windowed");
	race_close(width, height, attachments, 2, 0, "windowed (with front)");
	race_close(width, height, attachments, 2, COMPOSITE, "composite windowed (with front)");

	race_client(width, height, attachments, 1, 0, "windowed");
	race_client(width, height, attachments, 1, COMPOSITE, "composite windowed");
	race_client(width, height, attachments, 2, 0, "windowed (with front)");
	race_client(width, height, attachments, 2, COMPOSITE, "composite windowed (with front)");

	return 0;
}

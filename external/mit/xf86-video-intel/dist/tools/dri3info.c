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
 * To compile standalone: gcc -o dri3info dri3info.c `pkg-config --cflags --libs xcb-dri3 x11-xcb xrandr xxf86vm libdrm`
 */

#include <X11/Xlib.h>
#include <X11/Xlib-xcb.h>
#include <xcb/xcb.h>
#include <xcb/dri3.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/stat.h>
#include <drm.h>
#include <xf86drm.h>

#include <X11/extensions/Xrandr.h>
#include <X11/extensions/xf86vmode.h>

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

static int dri3_exists(Display *dpy)
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

static int dri3_open(Display *dpy)
{
	xcb_connection_t *c = XGetXCBConnection(dpy);
	xcb_dri3_open_cookie_t cookie;
	xcb_dri3_open_reply_t *reply;

	if (!dri3_exists(dpy))
		return -1;

	cookie = xcb_dri3_open(c, RootWindow(dpy, DefaultScreen(dpy)), None);
	reply = xcb_dri3_open_reply(c, cookie, NULL);

	if (!reply)
		return -1;

	if (reply->nfd != 1)
		return -1;

	return xcb_dri3_open_reply_fds(c, reply)[0];
}

static void get_device_path(int fd, char *buf, int len)
{
	struct stat remote, local;
	int i;

	if (fstat(fd, &remote))
		goto out;

	for (i = 0; i < 16; i++) {
		snprintf(buf, len, "/dev/dri/card%d", i);
		if (stat(buf, &local))
			continue;

		if (local.st_mode == remote.st_mode &&
		    local.st_rdev == remote.st_rdev)
			return;

		snprintf(buf, len, "/dev/dri/renderD%d", i + 128);
		if (stat(buf, &local))
			continue;

		if (local.st_mode == remote.st_mode &&
		    local.st_rdev == remote.st_rdev)
			return;
	}

out:
	strncpy(buf, "unknown path", len);
}

static void get_driver_name(int fd, char *name, int len)
{
	drm_version_t version;

	memset(name, 0, len);
	memset(&version, 0, sizeof(version));
	version.name_len = len;
	version.name = name;

	(void)drmIoctl(fd, DRM_IOCTL_VERSION, &version);
}

static int compute_refresh_rate_from_mode(long n, long d, unsigned flags,
					   int32_t *numerator,
					   int32_t *denominator)
{
	int i;

	/* The mode flags are only defined privately to the Xserver (in xf86str.h)
	 * but they at least bit compatible between VidMode, RandR and DRM.
	 */
# define V_INTERLACE 0x010
# define V_DBLSCAN   0x020

	if (flags & V_INTERLACE)
		n *= 2;
	else if (flags & V_DBLSCAN)
		d *= 2;

	/* The OML_sync_control spec requires that if the refresh rate is a
	 * whole number, that the returned numerator be equal to the refresh
	 * rate and the denominator be 1.
	 */

	if (n % d == 0) {
		n /= d;
		d = 1;
	}
	else {
		static const unsigned f[] = { 13, 11, 7, 5, 3, 2, 0 };

		/* This is a poor man's way to reduce a fraction.  It's far from
		 * perfect, but it will work well enough for this situation.
		 */

		for (i = 0; f[i] != 0; i++) {
			while (n % f[i] == 0 && d % f[i] == 0) {
				d /= f[i];
				n /= f[i];
			}
		}
	}

	*numerator = n;
	*denominator = d;
	return 1;
}

static int RRGetMscRate(Display *dpy, int32_t *numerator, int32_t *denominator)
{
	int ret = 0;
	Window root = RootWindow(dpy, DefaultScreen(dpy));
	XRRScreenResources *res;
	int rr_event, rr_error;
	RROutput primary;
	RRMode mode = 0;
	int n;

	if (!XRRQueryExtension(dpy, &rr_event, &rr_error))
		return ret;

	res = XRRGetScreenResourcesCurrent(dpy, root);
	if (res == NULL)
		return ret;

	/* Use the primary output if specified, otherwise
	 * use the mode on the first enabled crtc.
	 */
	primary = XRRGetOutputPrimary(dpy, root);
	if (primary) {
		XRROutputInfo *output;

		output = XRRGetOutputInfo(dpy, res, primary);
		if (output != NULL) {
			if (output->crtc) {
				XRRCrtcInfo *crtc;

				crtc = XRRGetCrtcInfo(dpy, res, output->crtc);
				if (crtc) {
					mode = crtc->mode;
					XRRFreeCrtcInfo(crtc);
				}
			}
			XRRFreeOutputInfo(output);
		}
	}

	for (n = 0; mode == 0 && n < res->ncrtc; n++) {
		XRRCrtcInfo *crtc;

		crtc = XRRGetCrtcInfo(dpy, res, res->crtcs[n]);
		if (crtc) {
			mode = crtc->mode;
			XRRFreeCrtcInfo(crtc);
		}
	}

	for (n = 0; n < res->nmode; n++) {
		if (res->modes[n].id == mode) {
			ret = compute_refresh_rate_from_mode(res->modes[n].dotClock,
							     res->modes[n].hTotal*res->modes[n].vTotal,
							     res->modes[n].modeFlags,
							     numerator, denominator);
			break;
		}
	}

	XRRFreeScreenResources(res);
	return ret;
}

static int VMGetMscRate(Display *dpy, int32_t *numerator, int32_t *denominator)
{
	XF86VidModeModeLine mode_line;
	int dot_clock;
	int i;

	if (XF86VidModeQueryVersion(dpy, &i, &i) &&
	    XF86VidModeGetModeLine(dpy, DefaultScreen(dpy), &dot_clock, &mode_line))
		return compute_refresh_rate_from_mode(dot_clock * 1000,
						      mode_line.vtotal * mode_line.htotal,
						      mode_line.flags,
						      numerator, denominator);

	return 0;
}

static int get_refresh_rate(Display *dpy,
			     int32_t *numerator,
			     int32_t *denominator)
{
	if (RRGetMscRate(dpy, numerator, denominator))
		return 1;

	if (VMGetMscRate(dpy, numerator, denominator))
		return 1;

	return 0;
}

static void info(const char *dpyname)
{
	Display *dpy;
	int device;
	int32_t numerator, denominator;

	dpy = XOpenDisplay(dpyname);
	if (dpy == NULL) {
		printf("Unable to connect to display '%s'\n",
		       dpyname ?: getenv("DISPLAY") ?: "unset");
		return;
	}

	printf("Display '%s'\n", DisplayString(dpy));
	device = dri3_open(dpy);
	if (device < 0) {
		printf("\tUnable to connect to DRI3\n");
	} else {
		char device_path[1024];
		char driver_name[1024];

		get_device_path(device, device_path, sizeof(device_path));
		get_driver_name(device, driver_name, sizeof(driver_name));

		printf("Connected to DRI3, using fd %d which matches %s, driver %s\n",
		       device, device_path, driver_name);
		close(device);
	}

	if (get_refresh_rate(dpy, &numerator, &denominator))
		printf("\tPrimary refresh rate: %d/%d (%.1fHz)\n",
		       numerator, denominator, numerator/(float)denominator);

	XCloseDisplay(dpy);
}

int main(int argc, char **argv)
{
	int i;

	if (argc > 1) {
		for (i = 1; i < argc; i++)
			info(argv[i]);
	} else
		info(NULL);

	return 0;
}

/**************************************************************************

Copyright 2001 VA Linux Systems Inc., Fremont, California.
Copyright © 2002 by David Dawes

All Rights Reserved.

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
on the rights to use, copy, modify, merge, publish, distribute, sub
license, and/or sell copies of the Software, and to permit persons to whom
the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice (including the next
paragraph) shall be included in all copies or substantial portions of the
Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
THE COPYRIGHT HOLDERS AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM,
DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
USE OR OTHER DEALINGS IN THE SOFTWARE.

**************************************************************************/

/*
 * Authors: Jeff Hartmann <jhartmann@valinux.com>
 *          Abraham van der Merwe <abraham@2d3d.co.za>
 *          David Dawes <dawes@xfree86.org>
 *          Alan Hourihane <alanh@tungstengraphics.com>
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include <xf86cmap.h>
#include <xf86drm.h>
#include <xf86RandR12.h>
#include <mi.h>
#include <micmap.h>
#include <mipict.h>

#include "compiler.h"
#include "sna.h"
#include "sna_module.h"
#include "sna_video.h"

#include "intel_driver.h"
#include "intel_options.h"

#include <sys/ioctl.h>
#include <sys/fcntl.h>
#include "i915_drm.h"

#ifdef HAVE_VALGRIND
#include <valgrind.h>
#include <memcheck.h>
#endif

#if HAVE_DOT_GIT
#include "git_version.h"
#endif

DevPrivateKeyRec sna_pixmap_key;
DevPrivateKeyRec sna_gc_key;
DevPrivateKeyRec sna_window_key;
DevPrivateKeyRec sna_glyph_key;
DevPrivateKeyRec sna_client_key;

static void
sna_load_palette(ScrnInfoPtr scrn, int numColors, int *indices,
		 LOCO * colors, VisualPtr pVisual)
{
	xf86CrtcConfigPtr xf86_config = XF86_CRTC_CONFIG_PTR(scrn);
	int i, j, index;
	int p;
	uint16_t lut_r[256], lut_g[256], lut_b[256];

	DBG(("%s\n", __FUNCTION__));

	for (p = 0; p < xf86_config->num_crtc; p++) {
		xf86CrtcPtr crtc = xf86_config->crtc[p];

		switch (scrn->depth) {
		case 15:
			for (i = 0; i < numColors; i++) {
				index = indices[i];
				for (j = 0; j < 8; j++) {
					lut_r[index * 8 + j] =
					    colors[index].red << 8;
					lut_g[index * 8 + j] =
					    colors[index].green << 8;
					lut_b[index * 8 + j] =
					    colors[index].blue << 8;
				}
			}
			break;
		case 16:
			for (i = 0; i < numColors; i++) {
				index = indices[i];

				if (index <= 31) {
					for (j = 0; j < 8; j++) {
						lut_r[index * 8 + j] =
						    colors[index].red << 8;
						lut_b[index * 8 + j] =
						    colors[index].blue << 8;
					}
				}

				for (j = 0; j < 4; j++) {
					lut_g[index * 4 + j] =
					    colors[index].green << 8;
				}
			}
			break;
		default:
			for (i = 0; i < numColors; i++) {
				index = indices[i];
				lut_r[index] = colors[index].red << 8;
				lut_g[index] = colors[index].green << 8;
				lut_b[index] = colors[index].blue << 8;
			}
			break;
		}

		/* Make the change through RandR */
#ifdef RANDR_12_INTERFACE
		RRCrtcGammaSet(crtc->randr_crtc, lut_r, lut_g, lut_b);
#else
		crtc->funcs->gamma_set(crtc, lut_r, lut_g, lut_b, 256);
#endif
	}
}

static void
sna_set_fallback_mode(ScrnInfoPtr scrn)
{
	xf86CrtcConfigPtr config = XF86_CRTC_CONFIG_PTR(scrn);
	xf86OutputPtr output = NULL;
	xf86CrtcPtr crtc = NULL;
	int n;

	if ((unsigned)config->compat_output < config->num_output) {
		output = config->output[config->compat_output];
		crtc = output->crtc;
	}

	for (n = 0; n < config->num_output; n++)
		config->output[n]->crtc = NULL;
	for (n = 0; n < config->num_crtc; n++)
		config->crtc[n]->enabled = FALSE;

	if (output && crtc) {
		DisplayModePtr mode;

		output->crtc = crtc;

		mode = xf86OutputFindClosestMode(output, scrn->currentMode);
		if (mode &&
		    xf86CrtcSetModeTransform(crtc, mode, RR_Rotate_0, NULL, 0, 0)) {
			crtc->desiredMode = *mode;
			crtc->desiredMode.prev = crtc->desiredMode.next = NULL;
			crtc->desiredMode.name = NULL;
			crtc->desiredMode.PrivSize = 0;
			crtc->desiredMode.PrivFlags = 0;
			crtc->desiredMode.Private = NULL;
			crtc->desiredRotation = RR_Rotate_0;
			crtc->desiredTransformPresent = FALSE;
			crtc->desiredX = 0;
			crtc->desiredY = 0;
			crtc->enabled = TRUE;
		}
	}

	xf86DisableUnusedFunctions(scrn);
#ifdef RANDR_12_INTERFACE
	if (get_root_window(scrn->pScreen))
		xf86RandR12TellChanged(scrn->pScreen);
#endif
}

static Bool sna_become_master(struct sna *sna)
{
	ScrnInfoPtr scrn = sna->scrn;

	DBG(("%s\n", __FUNCTION__));

	if (intel_get_master(scrn))
		return FALSE;

	if (!xf86SetDesiredModes(scrn)) {
		xf86DrvMsg(scrn->scrnIndex, X_WARNING,
			   "failed to restore desired modes on VT switch\n");
		sna_set_fallback_mode(scrn);
	}

	sna_mode_update(sna);
	return TRUE;
}

/**
 * Adjust the screen pixmap for the current location of the front buffer.
 * This is done at EnterVT when buffers are bound as long as the resources
 * have already been created, but the first EnterVT happens before
 * CreateScreenResources.
 */
static Bool sna_create_screen_resources(ScreenPtr screen)
{
	struct sna *sna = to_sna_from_screen(screen);

	DBG(("%s(%dx%d@%d)\n", __FUNCTION__,
	     screen->width, screen->height, screen->rootDepth));

	assert(sna->scrn == xf86ScreenToScrn(screen));
	assert(sna->scrn->pScreen == screen);

	free(screen->devPrivate);
	screen->devPrivate = NULL;

	sna_accel_create(sna);

	sna->front = screen->CreatePixmap(screen,
					  screen->width,
					  screen->height,
					  screen->rootDepth,
					  SNA_CREATE_FB);
	if (!sna->front) {
		xf86DrvMsg(screen->myNum, X_ERROR,
			   "[intel] Unable to create front buffer %dx%d at depth %d\n",
			   screen->width,
			   screen->height,
			   screen->rootDepth);

		return FALSE;
	}

	if (!sna_pixmap_force_to_gpu(sna->front, MOVE_WRITE)) {
		xf86DrvMsg(screen->myNum, X_ERROR,
			   "[intel] Failed to allocate video resources for front buffer %dx%d at depth %d\n",
			   screen->width,
			   screen->height,
			   screen->rootDepth);
		goto cleanup_front;
	}

	screen->SetScreenPixmap(sna->front);

	/* Only preserve the fbcon, not any subsequent server regens */
	if (serverGeneration == 1)
		sna_copy_fbcon(sna);

	if (!sna_become_master(sna)) {
		xf86DrvMsg(screen->myNum, X_ERROR,
			   "[intel] Failed to become DRM master\n");
		goto cleanup_front;
	}

	return TRUE;

cleanup_front:
	screen->SetScreenPixmap(NULL);
	screen->DestroyPixmap(sna->front);
	sna->front = NULL;
	return FALSE;
}

static void sna_selftest(void)
{
	sna_damage_selftest();
}

static bool has_pageflipping(struct sna *sna)
{
	drm_i915_getparam_t gp;
	int v;

	if (sna->flags & (SNA_IS_HOSTED | SNA_NO_WAIT))
		return false;

	v = 0;

	VG_CLEAR(gp);
	gp.param = I915_PARAM_HAS_PAGEFLIPPING;
	gp.value = &v;

	if (drmIoctl(sna->kgem.fd, DRM_IOCTL_I915_GETPARAM, &gp))
		return false;

	VG(VALGRIND_MAKE_MEM_DEFINED(&v, sizeof(v)));
	return v > 0;
}

static void sna_setup_capabilities(ScrnInfoPtr scrn, int fd)
{
#if HAS_PIXMAP_SHARING && defined(DRM_CAP_PRIME)
	uint64_t value;

	scrn->capabilities = 0;
	if (drmGetCap(fd, DRM_CAP_PRIME, &value) == 0) {
		if (value & DRM_PRIME_CAP_EXPORT)
			scrn->capabilities |= RR_Capability_SourceOutput | RR_Capability_SinkOffload;
		if (value & DRM_PRIME_CAP_IMPORT)
			scrn->capabilities |= RR_Capability_SinkOutput;
	}
#endif
}

static Bool sna_option_cast_to_bool(struct sna *sna, int id, Bool val)
{
	xf86getBoolValue(&val, xf86GetOptValString(sna->Options, id));
	return val;
}

static Bool fb_supports_depth(int fd, int depth)
{
	struct drm_i915_gem_create create;
	struct drm_mode_fb_cmd fb;
	struct drm_gem_close close;
	Bool ret;

	VG_CLEAR(create);
	create.handle = 0;
	create.size = 4096;
	if (drmIoctl(fd, DRM_IOCTL_I915_GEM_CREATE, &create))
		return FALSE;

	VG_CLEAR(fb);
	fb.width = 64;
	fb.height = 16;
	fb.pitch = 256;
	fb.bpp = depth <= 8 ? 8 : depth <= 16 ? 16 : 32;
	fb.depth = depth;
	fb.handle = create.handle;

	ret = drmIoctl(fd, DRM_IOCTL_MODE_ADDFB, &fb) == 0;
	drmModeRmFB(fd, fb.fb_id);

	VG_CLEAR(close);
	close.handle = create.handle;
	(void)drmIoctl(fd, DRM_IOCTL_GEM_CLOSE, &close);

	return ret;
}

/**
 * This is called before ScreenInit to do any require probing of screen
 * configuration.
 *
 * This code generally covers probing, module loading, option handling
 * card mapping, and RandR setup.
 *
 * Since xf86InitialConfiguration ends up requiring that we set video modes
 * in order to detect configuration, we end up having to do a lot of driver
 * setup (talking to the DRM, mapping the device, etc.) in this function.
 * As a result, we want to set up that server initialization once rather
 * that doing it per generation.
 */
static Bool sna_pre_init(ScrnInfoPtr scrn, int flags)
{
	struct sna *sna;
	char buf[1024];
	rgb defaultWeight = { 0, 0, 0 };
	EntityInfoPtr pEnt;
	int preferred_depth;
	Gamma zeros = { 0.0, 0.0, 0.0 };
	int fd;

	DBG(("%s flags=%x, numEntities=%d\n",
	     __FUNCTION__, flags, scrn->numEntities));

	if (scrn->numEntities != 1)
		return FALSE;

	pEnt = xf86GetEntityInfo(scrn->entityList[0]);
	if (pEnt == NULL)
		return FALSE;

	if (pEnt->location.type != BUS_PCI
#ifdef XSERVER_PLATFORM_BUS
	    && pEnt->location.type != BUS_PLATFORM
#endif
		)
		return FALSE;

	if (flags & PROBE_DETECT)
		return TRUE;

	sna_selftest();

	if (((uintptr_t)scrn->driverPrivate) & 1) {
		if (posix_memalign((void **)&sna, 4096, sizeof(*sna)))
			return FALSE;

		memset(sna, 0, sizeof(*sna)); /* should be unnecessary */
		sna->info = (void *)((uintptr_t)scrn->driverPrivate & ~1);
		scrn->driverPrivate = sna;

		sna->cpu_features = sna_cpu_detect();
	}
	sna = to_sna(scrn);
	sna->scrn = scrn;
	sna->pEnt = pEnt;
	sna->flags = 0;

	scrn->displayWidth = 640;	/* default it */

	sna->PciInfo = xf86GetPciInfoForEntity(sna->pEnt->index);

	scrn->monitor = scrn->confScreen->monitor;
	scrn->progClock = TRUE;
	scrn->rgbBits = 8;

	fd = intel_get_device(scrn);
	if (fd == -1) {
		xf86DrvMsg(scrn->scrnIndex, X_ERROR,
			   "Failed to claim DRM device.\n");
		goto cleanup;
	}

	/* Sanity check */
	if (hosted() && (sna->flags & SNA_IS_HOSTED) == 0) {
		xf86DrvMsg(scrn->scrnIndex, X_ERROR,
			   "Failed to setup hosted device.\n");
		goto cleanup;
	}

	preferred_depth = sna->info->gen < 030 ? 15 : 24;
	if (!fb_supports_depth(fd, preferred_depth))
		preferred_depth = 24;

	if (!xf86SetDepthBpp(scrn, preferred_depth, 0, 0,
			     Support32bppFb |
			     SupportConvert24to32 | PreferConvert24to32))
		goto cleanup;

	switch (scrn->depth) {
	case 8:
	case 15:
	case 16:
	case 24:
	case 30:
		if (fb_supports_depth(fd, scrn->depth))
			break;
	default:
		xf86DrvMsg(scrn->scrnIndex, X_ERROR,
			   "Given depth (%d) is not supported by the Intel driver and this chipset.\n",
			   scrn->depth);
		goto cleanup;
	}
	xf86PrintDepthBpp(scrn);

	if (!xf86SetWeight(scrn, defaultWeight, defaultWeight))
		goto cleanup;
	if (!xf86SetDefaultVisual(scrn, -1))
		goto cleanup;

	sna->Options = intel_options_get(scrn);
	if (sna->Options == NULL)
		goto cleanup;

	sna_setup_capabilities(scrn, fd);

	intel_detect_chipset(scrn, sna->pEnt, sna->PciInfo);

	kgem_init(&sna->kgem, fd, sna->PciInfo, sna->info->gen);
	if (xf86ReturnOptValBool(sna->Options, OPTION_ACCEL_DISABLE, FALSE) ||
	    !sna_option_cast_to_bool(sna, OPTION_ACCEL_METHOD, TRUE)) {
		xf86DrvMsg(sna->scrn->scrnIndex, X_CONFIG,
			   "Disabling hardware acceleration.\n");
		sna->kgem.wedged = true;
	}

	/* Enable tiling by default */
	sna->tiling = SNA_TILING_ALL;

	/* Allow user override if they set a value */
	if (!xf86ReturnOptValBool(sna->Options, OPTION_TILING_2D, TRUE))
		sna->tiling &= ~SNA_TILING_2D;
	if (xf86ReturnOptValBool(sna->Options, OPTION_TILING_FB, FALSE))
		sna->tiling &= ~SNA_TILING_FB;

	if (!xf86ReturnOptValBool(sna->Options, OPTION_SWAPBUFFERS_WAIT, TRUE))
		sna->flags |= SNA_NO_WAIT;
	if (xf86ReturnOptValBool(sna->Options, OPTION_TRIPLE_BUFFER, TRUE))
		sna->flags |= SNA_TRIPLE_BUFFER;
	if (has_pageflipping(sna)) {
		if (xf86ReturnOptValBool(sna->Options, OPTION_TEAR_FREE, FALSE))
			sna->flags |= SNA_TEAR_FREE;
	} else
		sna->flags |= SNA_NO_FLIP;
	if (xf86ReturnOptValBool(sna->Options, OPTION_CRTC_PIXMAPS, FALSE))
		sna->flags |= SNA_FORCE_SHADOW;

	xf86DrvMsg(scrn->scrnIndex, X_PROBED, "CPU: %s\n",
		   sna_cpu_features_to_string(sna->cpu_features, buf));
	xf86DrvMsg(scrn->scrnIndex, X_CONFIG, "Framebuffer %s\n",
		   sna->tiling & SNA_TILING_FB ? "tiled" : "linear");
	xf86DrvMsg(scrn->scrnIndex, X_CONFIG, "Pixmaps %s\n",
		   sna->tiling & SNA_TILING_2D ? "tiled" : "linear");
	xf86DrvMsg(scrn->scrnIndex, X_CONFIG, "\"Tear free\" %sabled\n",
		   sna->flags & SNA_TEAR_FREE ? "en" : "dis");
	xf86DrvMsg(scrn->scrnIndex, X_CONFIG, "Forcing per-crtc-pixmaps? %s\n",
		   sna->flags & SNA_FORCE_SHADOW ? "yes" : "no");

	if (sna->tiling != SNA_TILING_ALL)
		xf86DrvMsg(scrn->scrnIndex, X_WARNING,
			   "Tiling disabled, expect poor performance and increased power consumption.\n");

	if (!sna_mode_pre_init(scrn, sna)) {
		xf86DrvMsg(scrn->scrnIndex, X_ERROR,
			   "No outputs and no modes.\n");
		goto cleanup;
	}
	scrn->currentMode = scrn->modes;

	xf86SetGamma(scrn, zeros);
	xf86SetDpi(scrn, 0, 0);

	sna->dri_available = false;
	if (sna_option_cast_to_bool(sna, OPTION_DRI, TRUE))
		sna->dri_available = !!xf86LoadSubModule(scrn, "dri2");

	return TRUE;

cleanup:
	scrn->driverPrivate = (void *)((uintptr_t)sna->info | 1);
	free(sna);
	return FALSE;
}

static void
sna_block_handler(BLOCKHANDLER_ARGS_DECL)
{
#ifndef XF86_SCRN_INTERFACE
	struct sna *sna = to_sna(xf86Screens[arg]);
#else
	struct sna *sna = to_sna_from_screen(arg);
#endif
	struct timeval **tv = timeout;

	DBG(("%s (tv=%ld.%06ld)\n", __FUNCTION__,
	     *tv ? (*tv)->tv_sec : -1, *tv ? (*tv)->tv_usec : 0));

	sna->BlockHandler(BLOCKHANDLER_ARGS);

	if (*tv == NULL || ((*tv)->tv_usec | (*tv)->tv_sec))
		sna_accel_block_handler(sna, tv);
}

static void
sna_wakeup_handler(WAKEUPHANDLER_ARGS_DECL)
{
#ifndef XF86_SCRN_INTERFACE
	struct sna *sna = to_sna(xf86Screens[arg]);
#else
	struct sna *sna = to_sna_from_screen(arg);
#endif

	DBG(("%s\n", __FUNCTION__));

	/* despite all appearances, result is just a signed int */
	if ((int)result < 0)
		return;

	sna->WakeupHandler(WAKEUPHANDLER_ARGS);

	sna_accel_wakeup_handler(sna);

	if (FD_ISSET(sna->kgem.fd, (fd_set*)read_mask))
		sna_mode_wakeup(sna);
}

#if HAVE_UDEV
static void
sna_handle_uevents(int fd, void *closure)
{
	ScrnInfoPtr scrn = closure;
	struct sna *sna = to_sna(scrn);
	struct udev_device *dev;
	const char *hotplug;
	struct stat s;
	dev_t udev_devnum;

	DBG(("%s\n", __FUNCTION__));

	dev = udev_monitor_receive_device(sna->uevent_monitor);
	if (!dev)
		return;

	udev_devnum = udev_device_get_devnum(dev);
	if (fstat(sna->kgem.fd, &s)) {
		udev_device_unref(dev);
		return;
	}

	/*
	 * Check to make sure this event is directed at our
	 * device (by comparing dev_t values), then make
	 * sure it's a hotplug event (HOTPLUG=1)
	 */

	hotplug = udev_device_get_property_value(dev, "HOTPLUG");

	if (memcmp(&s.st_rdev, &udev_devnum, sizeof (dev_t)) == 0 &&
	    hotplug && atoi(hotplug) == 1) {
		DBG(("%s: hotplug event\n", __FUNCTION__));
		if (sna->scrn->vtSema) {
			sna_mode_update(sna);
			RRGetInfo(xf86ScrnToScreen(scrn), TRUE);
		} else
			sna->flags |= SNA_REPROBE;
	}

	udev_device_unref(dev);
}

static void
sna_uevent_init(ScrnInfoPtr scrn)
{
	struct sna *sna = to_sna(scrn);
	struct udev *u;
	struct udev_monitor *mon;
	Bool hotplug;
	MessageType from = X_CONFIG;

	if (sna->flags & SNA_IS_HOSTED)
		return;

	DBG(("%s\n", __FUNCTION__));

	/* RandR will be disabled if Xinerama is active, and so generating
	 * RR hotplug events is then verboten.
	 */
	if (!dixPrivateKeyRegistered(rrPrivKey))
		return;

	if (!xf86GetOptValBool(sna->Options, OPTION_HOTPLUG, &hotplug))
		from = X_DEFAULT, hotplug = TRUE;
	xf86DrvMsg(scrn->scrnIndex, from, "hotplug detection: \"%s\"\n",
			hotplug ? "enabled" : "disabled");
	if (!hotplug)
		return;

	u = udev_new();
	if (!u)
		return;

	mon = udev_monitor_new_from_netlink(u, "udev");
	if (!mon) {
		udev_unref(u);
		return;
	}

	if (udev_monitor_filter_add_match_subsystem_devtype(mon,
				"drm", "drm_minor") < 0 ||
	    udev_monitor_enable_receiving(mon) < 0)
	{
		udev_monitor_unref(mon);
		udev_unref(u);
		return;
	}

	sna->uevent_handler =
		xf86AddGeneralHandler(udev_monitor_get_fd(mon),
				      sna_handle_uevents,
				      scrn);
	if (!sna->uevent_handler) {
		udev_monitor_unref(mon);
		udev_unref(u);
		return;
	}

	sna->uevent_monitor = mon;

	DBG(("%s: installed uvent handler\n", __FUNCTION__));
}

static void
sna_uevent_fini(ScrnInfoPtr scrn)
{
	struct sna *sna = to_sna(scrn);
	struct udev *u;

	if (sna->uevent_handler == NULL)
		return;

	xf86RemoveGeneralHandler(sna->uevent_handler);

	u = udev_monitor_get_udev(sna->uevent_monitor);
	udev_monitor_unref(sna->uevent_monitor);
	udev_unref(u);

	sna->uevent_handler = NULL;
	sna->uevent_monitor = NULL;

	DBG(("%s: removed uvent handler\n", __FUNCTION__));
}
#else
static void sna_uevent_init(ScrnInfoPtr scrn) { }
static void sna_uevent_fini(ScrnInfoPtr scrn) { }
#endif /* HAVE_UDEV */

static void sna_leave_vt(VT_FUNC_ARGS_DECL)
{
	SCRN_INFO_PTR(arg);

	DBG(("%s\n", __FUNCTION__));

	xf86_hide_cursors(scrn);

	if (intel_put_master(scrn))
		xf86DrvMsg(scrn->scrnIndex, X_WARNING,
			   "drmDropMaster failed: %s\n", strerror(errno));
}

static Bool sna_early_close_screen(CLOSE_SCREEN_ARGS_DECL)
{
	ScrnInfoPtr scrn = xf86ScreenToScrn(screen);
	struct sna *sna = to_sna(scrn);

	DBG(("%s\n", __FUNCTION__));

	/* XXX Note that we will leak kernel resources if !vtSema */

	xf86_hide_cursors(scrn);
	sna_uevent_fini(scrn);

	sna_mode_close(sna);

	if (sna->dri_open) {
		sna_dri_close(sna, screen);
		sna->dri_open = false;
	}

	if (sna->front) {
		screen->DestroyPixmap(sna->front);
		sna->front = NULL;
	}

	if (scrn->vtSema) {
		intel_put_master(scrn);
		scrn->vtSema = FALSE;
	}

	xf86_cursors_fini(screen);

	return sna->CloseScreen(CLOSE_SCREEN_ARGS);
}

static Bool sna_late_close_screen(CLOSE_SCREEN_ARGS_DECL)
{
	struct sna *sna = to_sna_from_screen(screen);
	DepthPtr depths;
	int d;

	DBG(("%s\n", __FUNCTION__));

	sna_accel_close(sna);

	depths = screen->allowedDepths;
	for (d = 0; d < screen->numDepths; d++)
		free(depths[d].vids);
	free(depths);

	free(screen->visuals);

	return TRUE;
}

static Bool
sna_register_all_privates(void)
{
#if HAS_DIXREGISTERPRIVATEKEY
	if (!dixRegisterPrivateKey(&sna_pixmap_key, PRIVATE_PIXMAP,
				   3*sizeof(void *)))
		return FALSE;

	if (!dixRegisterPrivateKey(&sna_gc_key, PRIVATE_GC,
				   sizeof(FbGCPrivate)))
		return FALSE;

	if (!dixRegisterPrivateKey(&sna_glyph_key, PRIVATE_GLYPH,
				   sizeof(struct sna_glyph)))
		return FALSE;

	if (!dixRegisterPrivateKey(&sna_window_key, PRIVATE_WINDOW,
				   3*sizeof(void *)))
		return FALSE;

	if (!dixRegisterPrivateKey(&sna_client_key, PRIVATE_CLIENT,
				   sizeof(struct sna_client)))
		return FALSE;
#else
	if (!dixRequestPrivate(&sna_pixmap_key, 3*sizeof(void *)))
		return FALSE;

	if (!dixRequestPrivate(&sna_gc_key, sizeof(FbGCPrivate)))
		return FALSE;

	if (!dixRequestPrivate(&sna_glyph_key, sizeof(struct sna_glyph)))
		return FALSE;

	if (!dixRequestPrivate(&sna_window_key, 3*sizeof(void *)))
		return FALSE;

	if (!dixRequestPrivate(&sna_client_key, sizeof(struct sna_client)))
		return FALSE;
#endif

	return TRUE;
}

static size_t
agp_aperture_size(struct pci_device *dev, int gen)
{
	return dev->regions[gen < 030 ? 0 : 2].size;
}

static Bool
sna_screen_init(SCREEN_INIT_ARGS_DECL)
{
	ScrnInfoPtr scrn = xf86ScreenToScrn(screen);
	struct sna *sna = to_sna(scrn);
	VisualPtr visuals;
	DepthPtr depths;
	int nvisuals;
	int ndepths;
	int rootdepth;
	VisualID defaultVisual;

	DBG(("%s\n", __FUNCTION__));

	assert(sna->scrn == scrn);
	assert(scrn->pScreen == NULL); /* set afterwards */

	assert(sna->freed_pixmap == NULL);

	if (!sna_register_all_privates())
		return FALSE;

	scrn->videoRam = agp_aperture_size(sna->PciInfo, sna->kgem.gen) / 1024;

	miClearVisualTypes();
	if (!miSetVisualTypes(scrn->depth,
			      miGetDefaultVisualMask(scrn->depth),
			      scrn->rgbBits, scrn->defaultVisual))
		return FALSE;
	if (!miSetPixmapDepths())
		return FALSE;

	rootdepth = 0;
	if (!miInitVisuals(&visuals, &depths, &nvisuals, &ndepths, &rootdepth,
			   &defaultVisual,
			   ((unsigned long)1 << (scrn->bitsPerPixel - 1)),
			   8, -1))
		return FALSE;

	if (!miScreenInit(screen, NULL,
			  scrn->virtualX, scrn->virtualY,
			  scrn->xDpi, scrn->yDpi, 0,
			  rootdepth, ndepths, depths,
			  defaultVisual, nvisuals, visuals))
		return FALSE;

	if (scrn->bitsPerPixel > 8) {
		/* Fixup RGB ordering */
		VisualPtr visual = screen->visuals + screen->numVisuals;
		while (--visual >= screen->visuals) {
			if ((visual->class | DynamicClass) == DirectColor) {
				visual->offsetRed = scrn->offset.red;
				visual->offsetGreen = scrn->offset.green;
				visual->offsetBlue = scrn->offset.blue;
				visual->redMask = scrn->mask.red;
				visual->greenMask = scrn->mask.green;
				visual->blueMask = scrn->mask.blue;
			}
		}
	}

	assert(screen->CloseScreen == NULL);
	screen->CloseScreen = sna_late_close_screen;
	if (!sna_accel_init(screen, sna)) {
		xf86DrvMsg(scrn->scrnIndex, X_ERROR,
			   "Hardware acceleration initialization failed\n");
		return FALSE;
	}

	xf86SetBlackWhitePixels(screen);

	xf86SetBackingStore(screen);
	xf86SetSilkenMouse(screen);
	if (!miDCInitialize(screen, xf86GetPointerScreenFuncs()))
		return FALSE;

	if ((sna->flags & SNA_IS_HOSTED) == 0 &&
	    xf86_cursors_init(screen, SNA_CURSOR_X, SNA_CURSOR_Y,
			       HARDWARE_CURSOR_TRUECOLOR_AT_8BPP |
			       HARDWARE_CURSOR_BIT_ORDER_MSBFIRST |
			       HARDWARE_CURSOR_INVERT_MASK |
			       HARDWARE_CURSOR_SWAP_SOURCE_AND_MASK |
			       HARDWARE_CURSOR_AND_SOURCE_WITH_MASK |
			       HARDWARE_CURSOR_SOURCE_MASK_INTERLEAVE_64 |
			       HARDWARE_CURSOR_UPDATE_UNHIDDEN |
			       HARDWARE_CURSOR_ARGB))
		xf86DrvMsg(scrn->scrnIndex, X_INFO, "HW Cursor enabled\n");

	/* Must force it before EnterVT, so we are in control of VT and
	 * later memory should be bound when allocating, e.g rotate_mem */
	scrn->vtSema = TRUE;

	sna->BlockHandler = screen->BlockHandler;
	screen->BlockHandler = sna_block_handler;

	sna->WakeupHandler = screen->WakeupHandler;
	screen->WakeupHandler = sna_wakeup_handler;

	screen->SaveScreen = xf86SaveScreen;
	screen->CreateScreenResources = sna_create_screen_resources;

	sna->CloseScreen = screen->CloseScreen;
	screen->CloseScreen = sna_early_close_screen;

	if (!xf86CrtcScreenInit(screen))
		return FALSE;

	xf86RandR12SetRotations(screen,
				RR_Rotate_0 | RR_Rotate_90 | RR_Rotate_180 | RR_Rotate_270 |
				RR_Reflect_X | RR_Reflect_Y);
	xf86RandR12SetTransformSupport(screen, TRUE);

	if (!miCreateDefColormap(screen))
		return FALSE;

	if (!xf86HandleColormaps(screen, 256, 8, sna_load_palette, NULL,
				 CMAP_RELOAD_ON_MODE_SWITCH |
				 CMAP_PALETTED_TRUECOLOR)) {
		return FALSE;
	}

	xf86DPMSInit(screen, xf86DPMSSet, 0);

	sna_video_init(sna, screen);
	if (sna->dri_available)
		sna->dri_open = sna_dri_open(sna, screen);
	if (sna->dri_open)
		xf86DrvMsg(scrn->scrnIndex, X_INFO,
			   "direct rendering: DRI2 Enabled\n");

	if (serverGeneration == 1)
		xf86ShowUnusedOptions(scrn->scrnIndex, scrn->options);

	sna->suspended = FALSE;

	sna_uevent_init(scrn);

	return TRUE;
}

static void sna_adjust_frame(ADJUST_FRAME_ARGS_DECL)
{
	SCRN_INFO_PTR(arg);
	DBG(("%s(%d, %d)\n", __FUNCTION__, x, y));
	sna_mode_adjust_frame(to_sna(scrn), x, y);
}

static void sna_free_screen(FREE_SCREEN_ARGS_DECL)
{
	SCRN_INFO_PTR(arg);
	struct sna *sna = to_sna(scrn);

	DBG(("%s\n", __FUNCTION__));
	if ((uintptr_t)sna & 1)
		return;

	scrn->driverPrivate = (void *)((uintptr_t)sna->info | 1);

	sna_mode_fini(sna);
	free(sna);

	intel_put_device(scrn);
}

static Bool sna_enter_vt(VT_FUNC_ARGS_DECL)
{
	SCRN_INFO_PTR(arg);
	struct sna *sna = to_sna(scrn);

	DBG(("%s\n", __FUNCTION__));
	if (!sna_become_master(sna))
		return FALSE;

	if (sna->flags & SNA_REPROBE) {
		RRGetInfo(xf86ScrnToScreen(scrn), TRUE);
		sna->flags &= ~SNA_REPROBE;
	}

	return TRUE;
}

static Bool sna_switch_mode(SWITCH_MODE_ARGS_DECL)
{
	SCRN_INFO_PTR(arg);
	DBG(("%s\n", __FUNCTION__));
	return xf86SetSingleMode(scrn, mode, RR_Rotate_0);
}

static ModeStatus
sna_valid_mode(SCRN_ARG_TYPE arg, DisplayModePtr mode, Bool verbose, int flags)
{
	return MODE_OK;
}

#ifndef SUSPEND_SLEEP
#define SUSPEND_SLEEP 0
#endif
#ifndef RESUME_SLEEP
#define RESUME_SLEEP 0
#endif

/*
 * This function is only required if we need to do anything differently from
 * DoApmEvent() in common/xf86PM.c, including if we want to see events other
 * than suspend/resume.
 */
static Bool sna_pm_event(SCRN_ARG_TYPE arg, pmEvent event, Bool undo)
{
	SCRN_INFO_PTR(arg);
	struct sna *sna = to_sna(scrn);

	DBG(("%s\n", __FUNCTION__));

	switch (event) {
	case XF86_APM_SYS_SUSPEND:
	case XF86_APM_CRITICAL_SUSPEND:	/*do we want to delay a critical suspend? */
	case XF86_APM_USER_SUSPEND:
	case XF86_APM_SYS_STANDBY:
	case XF86_APM_USER_STANDBY:
		if (!undo && !sna->suspended) {
			scrn->LeaveVT(VT_FUNC_ARGS(0));
			sna->suspended = TRUE;
			sleep(SUSPEND_SLEEP);
		} else if (undo && sna->suspended) {
			sleep(RESUME_SLEEP);
			scrn->EnterVT(VT_FUNC_ARGS(0));
			sna->suspended = FALSE;
		}
		break;
	case XF86_APM_STANDBY_RESUME:
	case XF86_APM_NORMAL_RESUME:
	case XF86_APM_CRITICAL_RESUME:
		if (sna->suspended) {
			sleep(RESUME_SLEEP);
			scrn->EnterVT(VT_FUNC_ARGS(0));
			sna->suspended = FALSE;
			/*
			 * Turn the screen saver off when resuming.  This seems to be
			 * needed to stop xscreensaver kicking in (when used).
			 *
			 * XXX DoApmEvent() should probably call this just like
			 * xf86VTSwitch() does.  Maybe do it here only in 4.2
			 * compatibility mode.
			 */
			SaveScreens(SCREEN_SAVER_FORCER, ScreenSaverReset);
		}
		break;
		/* This is currently used for ACPI */
	case XF86_APM_CAPABILITY_CHANGED:
		SaveScreens(SCREEN_SAVER_FORCER, ScreenSaverReset);
		break;

	default:
		ErrorF("sna_pm_event: received APM event %d\n", event);
	}
	return TRUE;
}

static Bool sna_enter_vt__hosted(VT_FUNC_ARGS_DECL)
{
	return TRUE;
}

static void sna_leave_vt__hosted(VT_FUNC_ARGS_DECL)
{
}

Bool sna_init_scrn(ScrnInfoPtr scrn, int entity_num)
{
	DBG(("%s: entity_num=%d\n", __FUNCTION__, entity_num));
#if defined(USE_GIT_DESCRIBE)
	xf86DrvMsg(scrn->scrnIndex, X_INFO,
		   "SNA compiled from %s\n", git_version);
#elif defined(BUILDER_DESCRIPTION)
	xf86DrvMsg(scrn->scrnIndex, X_INFO,
		   "SNA compiled: %s\n", BUILDER_DESCRIPTION);
#endif
#if !NDEBUG
	xf86DrvMsg(scrn->scrnIndex, X_INFO,
		   "SNA compiled with assertions enabled\n");
#endif
#if DEBUG_SYNC
	xf86DrvMsg(scrn->scrnIndex, X_INFO,
		   "SNA compiled with synchronous rendering\n");
#endif
#if DEBUG_MEMORY
	xf86DrvMsg(scrn->scrnIndex, X_INFO,
		   "SNA compiled with memory allocation reporting enabled\n");
#endif
#if DEBUG_PIXMAP
	xf86DrvMsg(scrn->scrnIndex, X_INFO,
		   "SNA compiled with extra pixmap/damage validation\n");
#endif
	DBG(("pixman version: %s\n", pixman_version_string()));

	scrn->PreInit = sna_pre_init;
	scrn->ScreenInit = sna_screen_init;
	if (!hosted()) {
		scrn->SwitchMode = sna_switch_mode;
		scrn->AdjustFrame = sna_adjust_frame;
		scrn->EnterVT = sna_enter_vt;
		scrn->LeaveVT = sna_leave_vt;
		scrn->ValidMode = sna_valid_mode;
		scrn->PMEvent = sna_pm_event;
	} else {
		scrn->EnterVT = sna_enter_vt__hosted;
		scrn->LeaveVT = sna_leave_vt__hosted;
	}
	scrn->FreeScreen = sna_free_screen;

	xf86SetEntitySharable(entity_num);
	xf86SetEntityInstanceForScreen(scrn, entity_num,
				       xf86GetNumEntityInstances(entity_num)-1);

	sna_threads_init();

	return TRUE;
}

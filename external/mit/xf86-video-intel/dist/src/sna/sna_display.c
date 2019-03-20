/*
 * Copyright © 2007 Red Hat, Inc.
 * Copyright © 2013-2014 Intel Corporation
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
 * Authors:
 *    Dave Airlie <airlied@redhat.com>
 *
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <poll.h>
#include <ctype.h>
#include <dirent.h>

#if HAVE_ALLOCA_H
#include <alloca.h>
#elif defined __GNUC__
#define alloca __builtin_alloca
#elif defined _AIX
#define alloca __alloca
#elif defined _MSC_VER
#include <malloc.h>
#define alloca _alloca
#else
void *alloca(size_t);
#endif

#define _PARSE_EDID_
/* Jump through a few hoops in order to fixup EDIDs */
#undef VERSION
#undef REVISION

#include "sna.h"
#include "sna_reg.h"
#include "fb/fbpict.h"
#include "intel_options.h"
#include "backlight.h"

#include <xf86Crtc.h>
#include <xf86RandR12.h>
#include <cursorstr.h>

#if XF86_CRTC_VERSION >= 3
#define HAS_GAMMA 1
#else
#define HAS_GAMMA 0
#endif

#include <X11/Xatom.h>
#if defined(HAVE_X11_EXTENSIONS_DPMSCONST_H)
#include <X11/extensions/dpmsconst.h>
#else
#define DPMSModeOn 0
#define DPMSModeOff 3
#endif
#include <xf86DDC.h> /* for xf86InterpretEDID */

#include <xf86drm.h>

#ifdef HAVE_VALGRIND
#include <valgrind.h>
#include <memcheck.h>
#endif

#define FAIL_CURSOR_IOCTL 0

/* Minor discrepancy between 32-bit/64-bit ABI in old kernels */
union compat_mode_get_connector{
	struct drm_mode_get_connector conn;
	uint32_t pad[20];
};

#define KNOWN_MODE_FLAGS ((1<<14)-1)

#ifndef MONITOR_EDID_COMPLETE_RAWDATA
#define MONITOR_EDID_COMPLETE_RAWDATA 1
#endif

#ifndef DEFAULT_DPI
#define DEFAULT_DPI 96
#endif

#define OUTPUT_STATUS_CACHE_MS 15000

#define DRM_MODE_PAGE_FLIP_ASYNC 0x02

#define DRM_CLIENT_CAP_UNIVERSAL_PLANES 2
#define DRM_PLANE_TYPE_OVERLAY 0
#define DRM_PLANE_TYPE_PRIMARY 1
#define DRM_PLANE_TYPE_CURSOR  2

#define LOCAL_IOCTL_MODE_OBJ_GETPROPERTIES DRM_IOWR(0xb9, struct local_mode_obj_get_properties)
struct local_mode_obj_get_properties {
	uint64_t props_ptr;
	uint64_t prop_values_ptr;
	uint32_t count_props;
	uint32_t obj_id;
	uint32_t obj_type;
	uint32_t pad;
};
#define LOCAL_MODE_OBJECT_PLANE 0xeeeeeeee

struct local_mode_set_plane {
	uint32_t plane_id;
	uint32_t crtc_id;
	uint32_t fb_id; /* fb object contains surface format type */
	uint32_t flags;

	/* Signed dest location allows it to be partially off screen */
	int32_t crtc_x, crtc_y;
	uint32_t crtc_w, crtc_h;

	/* Source values are 16.16 fixed point */
	uint32_t src_x, src_y;
	uint32_t src_h, src_w;
};
#define LOCAL_IOCTL_MODE_SETPLANE DRM_IOWR(0xB7, struct local_mode_set_plane)

struct local_mode_get_plane {
	uint32_t plane_id;

	uint32_t crtc_id;
	uint32_t fb_id;

	uint32_t possible_crtcs;
	uint32_t gamma_size;

	uint32_t count_format_types;
	uint64_t format_type_ptr;
};
#define LOCAL_IOCTL_MODE_GETPLANE DRM_IOWR(0xb6, struct local_mode_get_plane)

struct local_mode_get_plane_res {
	uint64_t plane_id_ptr;
	uint64_t count_planes;
};
#define LOCAL_IOCTL_MODE_GETPLANERESOURCES DRM_IOWR(0xb5, struct local_mode_get_plane_res)

#if 1
#define __DBG DBG
#else
#define __DBG(x)
#endif

#define DBG_NATIVE_ROTATION ~0 /* minimum RR_Rotate_0 */

extern XF86ConfigPtr xf86configptr;

struct sna_cursor {
	struct sna_cursor *next;
	uint32_t *image;
	bool transformed;
	Rotation rotation;
	int ref;
	int size;
	int last_width;
	int last_height;
	unsigned handle;
	unsigned serial;
	unsigned alloc;
};

struct sna_crtc {
	struct sna_crtc_public public;
	uint32_t id;
	xf86CrtcPtr base;
	struct drm_mode_modeinfo kmode;
	PixmapPtr slave_pixmap;
	DamagePtr slave_damage;
	struct kgem_bo *bo, *shadow_bo, *client_bo, *cache_bo;
	struct sna_cursor *cursor;
	unsigned int last_cursor_size;
	uint32_t offset;
	bool shadow;
	bool fallback_shadow;
	bool transform;
	bool cursor_transform;
	bool hwcursor;
	bool flip_pending;

	struct pict_f_transform cursor_to_fb, fb_to_cursor;

	RegionRec crtc_damage;
	uint16_t shadow_bo_width, shadow_bo_height;

	uint32_t rotation;
	struct plane {
		uint32_t id;
		struct {
			uint32_t prop;
			uint32_t supported;
			uint32_t current;
		} rotation;
		struct {
			uint32_t prop;
			uint64_t values[2];
		} color_encoding;
		struct list link;
	} primary;
	struct list sprites;

	uint32_t mode_serial, flip_serial;

	uint32_t last_seq, wrap_seq;
	struct ust_msc swap;

	sna_flip_handler_t flip_handler;
	struct kgem_bo *flip_bo;
	void *flip_data;

	struct list shadow_link;
};

struct sna_property {
	drmModePropertyPtr kprop;
	int num_atoms; /* if range prop, num_atoms == 1; if enum prop, num_atoms == num_enums + 1 */
	Atom *atoms;
};

struct sna_output {
	xf86OutputPtr base;
	unsigned id;
	unsigned serial;

	unsigned possible_encoders;
	unsigned attached_encoders;

	unsigned int is_panel : 1;
	unsigned int add_default_modes : 1;
	int connector_type;
	int connector_type_id;

	uint32_t link_status_idx;

	uint32_t edid_idx;
	uint32_t edid_blob_id;
	uint32_t edid_len;
	void *edid_raw;
	void *fake_edid_raw;

	bool has_panel_limits;
	int panel_hdisplay;
	int panel_vdisplay;

	uint32_t dpms_id;
	uint8_t dpms_mode;
	struct backlight backlight;
	int backlight_active_level;

	uint32_t last_detect;
	uint32_t status;
	unsigned int hotplug_count;
	bool update_properties;
	bool reprobe;

	int num_modes;
	struct drm_mode_modeinfo *modes;

	int num_props;
	uint32_t *prop_ids;
	uint64_t *prop_values;
	struct sna_property *props;
};

enum { /* XXX copied from hw/xfree86/modes/xf86Crtc.c */
	OPTION_PREFERRED_MODE,
#if XORG_VERSION_CURRENT >= XORG_VERSION_NUMERIC(1,14,99,1,0)
	OPTION_ZOOM_MODES,
#endif
	OPTION_POSITION,
	OPTION_BELOW,
	OPTION_RIGHT_OF,
	OPTION_ABOVE,
	OPTION_LEFT_OF,
	OPTION_ENABLE,
	OPTION_DISABLE,
	OPTION_MIN_CLOCK,
	OPTION_MAX_CLOCK,
	OPTION_IGNORE,
	OPTION_ROTATE,
	OPTION_PANNING,
	OPTION_PRIMARY,
	OPTION_DEFAULT_MODES,
};

static void __sna_output_dpms(xf86OutputPtr output, int dpms, int fixup);
static void sna_crtc_disable_cursor(struct sna *sna, struct sna_crtc *crtc);
static bool sna_crtc_flip(struct sna *sna, struct sna_crtc *crtc,
			  struct kgem_bo *bo, int x, int y);

static bool is_zaphod(ScrnInfoPtr scrn)
{
	return xf86IsEntityShared(scrn->entityList[0]);
}

static bool
sna_zaphod_match(struct sna *sna, const char *output)
{
	const char *s, *colon;
	char t[20];
	unsigned int i = 0;

	s = xf86GetOptValString(sna->Options, OPTION_ZAPHOD);
	if (s == NULL)
		return false;

	colon = strchr(s, ':');
	if (colon) /* Skip over the ZaphodPipes */
		s = colon + 1;

	do {
		/* match any outputs in a comma list, stopping at whitespace */
		switch (*s) {
		case '\0':
			t[i] = '\0';
			return strcmp(t, output) == 0;

		case ',':
			t[i] ='\0';
			if (strcmp(t, output) == 0)
				return TRUE;
			i = 0;
			break;

		case ' ':
		case '\t':
		case '\n':
		case '\r':
			break;

		default:
			t[i++] = *s;
			break;
		}

		s++;
	} while (i < sizeof(t));

	return false;
}

static unsigned
get_zaphod_crtcs(struct sna *sna)
{
	const char *str, *colon;
	unsigned crtcs = 0;

	str = xf86GetOptValString(sna->Options, OPTION_ZAPHOD);
	if (str == NULL || (colon = strchr(str, ':')) == NULL) {
		DBG(("%s: no zaphod pipes, using screen number: %x\n",
		     __FUNCTION__,
		     sna->scrn->confScreen->device->screen));
		return 1 << sna->scrn->confScreen->device->screen;
	}

	DBG(("%s: ZaphodHeads='%s'\n", __FUNCTION__, str));
	while (str < colon) {
		char *end;
		unsigned crtc = strtoul(str, &end, 0);
		if (end == str)
			break;
		DBG(("%s: adding CRTC %d to zaphod pipes\n",
		     __FUNCTION__, crtc));
		crtcs |= 1 << crtc;
		str = end + 1;
	}
	DBG(("%s: ZaphodPipes=%x\n", __FUNCTION__, crtcs));
	return crtcs;
}

inline static unsigned count_to_mask(int x)
{
	return (1 << x) - 1;
}

static inline struct sna_output *to_sna_output(xf86OutputPtr output)
{
	return output->driver_private;
}

static inline int to_connector_id(xf86OutputPtr output)
{
	assert(to_sna_output(output));
	assert(to_sna_output(output)->id);
	return to_sna_output(output)->id;
}

static inline struct sna_crtc *to_sna_crtc(xf86CrtcPtr crtc)
{
	return crtc->driver_private;
}

static inline unsigned __sna_crtc_pipe(struct sna_crtc *crtc)
{
	return crtc->public.flags >> 8 & 0xff;
}

static inline unsigned __sna_crtc_id(struct sna_crtc *crtc)
{
	return crtc->id;
}

uint32_t sna_crtc_id(xf86CrtcPtr crtc)
{
	return __sna_crtc_id(to_sna_crtc(crtc));
}

static inline bool event_pending(int fd)
{
	struct pollfd pfd;
	pfd.fd = fd;
	pfd.events = POLLIN;
	return poll(&pfd, 1, 0) == 1;
}

static bool sna_mode_wait_for_event(struct sna *sna)
{
	struct pollfd pfd;
	pfd.fd = sna->kgem.fd;
	pfd.events = POLLIN;
	return poll(&pfd, 1, -1) == 1;
}

static inline uint32_t fb_id(struct kgem_bo *bo)
{
	return bo->delta;
}

unsigned sna_crtc_count_sprites(xf86CrtcPtr crtc)
{
	struct plane *sprite;
	unsigned count;

	count = 0;
	list_for_each_entry(sprite, &to_sna_crtc(crtc)->sprites, link)
		count++;

	return count;
}

static struct plane *lookup_sprite(struct sna_crtc *crtc, unsigned idx)
{
	struct plane *sprite;

	list_for_each_entry(sprite, &crtc->sprites, link)
		if (idx-- == 0)
			return sprite;

	return NULL;
}

uint32_t sna_crtc_to_sprite(xf86CrtcPtr crtc, unsigned idx)
{
	struct plane *sprite;

	assert(to_sna_crtc(crtc));

	sprite = lookup_sprite(to_sna_crtc(crtc), idx);
	return sprite ? sprite->id : 0;
}

bool sna_crtc_is_transformed(xf86CrtcPtr crtc)
{
	assert(to_sna_crtc(crtc));
	return to_sna_crtc(crtc)->transform;
}

static inline bool msc64(struct sna_crtc *sna_crtc, uint32_t seq, uint64_t *msc)
{
	bool record = true;
	if (seq < sna_crtc->last_seq) {
		if (sna_crtc->last_seq - seq > 0x40000000) {
			sna_crtc->wrap_seq++;
			DBG(("%s: pipe=%d wrapped; was %u, now %u, wraps=%u\n",
			     __FUNCTION__, __sna_crtc_pipe(sna_crtc),
			     sna_crtc->last_seq, seq, sna_crtc->wrap_seq));
		} else {
			DBG(("%s: pipe=%d msc went backwards; was %u, now %u; ignoring for last_swap\n",
			     __FUNCTION__, __sna_crtc_pipe(sna_crtc), sna_crtc->last_seq, seq));

			record = false;
		}
	}
	*msc = (uint64_t)sna_crtc->wrap_seq << 32 | seq;
	return record;
}

uint64_t sna_crtc_record_swap(xf86CrtcPtr crtc,
			      int tv_sec, int tv_usec, unsigned seq)
{
	struct sna_crtc *sna_crtc = to_sna_crtc(crtc);
	uint64_t msc;

	assert(sna_crtc);

	if (msc64(sna_crtc, seq, &msc)) {
		DBG(("%s: recording last swap on pipe=%d, frame %d [msc=%08lld], time %d.%06d\n",
		     __FUNCTION__, __sna_crtc_pipe(sna_crtc), seq, (long long)msc,
		     tv_sec, tv_usec));
		sna_crtc->swap.tv_sec = tv_sec;
		sna_crtc->swap.tv_usec = tv_usec;
		sna_crtc->swap.msc = msc;
	} else {
		DBG(("%s: swap event on pipe=%d, frame %d [msc=%08lld], time %d.%06d\n",
		     __FUNCTION__, __sna_crtc_pipe(sna_crtc), seq, (long long)msc,
		     tv_sec, tv_usec));
	}

	return msc;
}

const struct ust_msc *sna_crtc_last_swap(xf86CrtcPtr crtc)
{
	static struct ust_msc zero;

	if (crtc == NULL) {
		return &zero;
	} else {
		struct sna_crtc *sna_crtc = to_sna_crtc(crtc);
		assert(sna_crtc);
		return &sna_crtc->swap;
	}
}

#ifndef NDEBUG
static void gem_close(int fd, uint32_t handle);
static void assert_scanout(struct kgem *kgem, struct kgem_bo *bo,
			   int width, int height)
{
	struct drm_mode_fb_cmd info;

	assert(bo->scanout);

	VG_CLEAR(info);
	info.fb_id = fb_id(bo);

	assert(drmIoctl(kgem->fd, DRM_IOCTL_MODE_GETFB, &info) == 0);
	gem_close(kgem->fd, info.handle);

	assert(width <= info.width && height <= info.height);
}
#else
#define assert_scanout(k, b, w, h)
#endif

static void assert_crtc_fb(struct sna *sna, struct sna_crtc *crtc)
{
#ifndef NDEBUG
	struct drm_mode_crtc mode = { .crtc_id = __sna_crtc_id(crtc) };
	drmIoctl(sna->kgem.fd, DRM_IOCTL_MODE_GETCRTC, &mode);
	assert(mode.fb_id == fb_id(crtc->bo));
#endif
}

static unsigned get_fb(struct sna *sna, struct kgem_bo *bo,
		       int width, int height)
{
	ScrnInfoPtr scrn = sna->scrn;
	struct drm_mode_fb_cmd arg;

	if (!kgem_bo_is_fenced(&sna->kgem, bo))
		return 0;

	assert(bo->refcnt);
	assert(bo->proxy == NULL);
	assert(!bo->snoop);
	assert(8*bo->pitch >= width * scrn->bitsPerPixel);
	assert(height * bo->pitch <= kgem_bo_size(bo)); /* XXX crtc offset */
	if (fb_id(bo)) {
		DBG(("%s: reusing fb=%d for handle=%d\n",
		     __FUNCTION__, fb_id(bo), bo->handle));
		assert_scanout(&sna->kgem, bo, width, height);
		return fb_id(bo);
	}

	DBG(("%s: create fb %dx%d@%d/%d\n",
	     __FUNCTION__, width, height, scrn->depth, scrn->bitsPerPixel));

	assert(bo->tiling != I915_TILING_Y || sna->kgem.can_scanout_y);
	assert((bo->pitch & 63) == 0);
	assert(scrn->vtSema); /* must be master */

	VG_CLEAR(arg);
	arg.width = width;
	arg.height = height;
	arg.pitch = bo->pitch;
	arg.bpp = scrn->bitsPerPixel;
	arg.depth = scrn->depth;
	arg.handle = bo->handle;

	if (drmIoctl(sna->kgem.fd, DRM_IOCTL_MODE_ADDFB, &arg)) {
		/* Try again with the fancy version */
		struct local_mode_fb_cmd2 {
			uint32_t fb_id;
			uint32_t width, height;
			uint32_t pixel_format;
			uint32_t flags;

			uint32_t handles[4];
			uint32_t pitches[4]; /* pitch for each plane */
			uint32_t offsets[4]; /* offset of each plane */
			uint64_t modifiers[4];
		} f;
#define LOCAL_IOCTL_MODE_ADDFB2 DRM_IOWR(0xb8, struct local_mode_fb_cmd2)
		memset(&f, 0, sizeof(f));
		f.width = width;
		f.height = height;
		/* XXX interlaced */
		f.flags = 1 << 1; /* +modifiers */
		f.handles[0] = bo->handle;
		f.pitches[0] = bo->pitch;

		switch (bo->tiling) {
		case I915_TILING_NONE:
			break;
		case I915_TILING_X:
			/* I915_FORMAT_MOD_X_TILED */
			f.modifiers[0] = (uint64_t)1 << 56 | 1;
			break;
		case I915_TILING_Y:
			/* I915_FORMAT_MOD_X_TILED */
			f.modifiers[0] = (uint64_t)1 << 56 | 2;
			break;
		}

#define fourcc(a,b,c,d) ((a) | (b) << 8 | (c) << 16 | (d) << 24)
		switch (scrn->depth) {
		default:
			ERR(("%s: unhandled screen format, depth=%d\n",
			     __FUNCTION__, scrn->depth));
			goto fail;
		case 8:
			f.pixel_format = fourcc('C', '8', ' ', ' ');
			break;
		case 15:
			f.pixel_format = fourcc('X', 'R', '1', '5');
			break;
		case 16:
			f.pixel_format = fourcc('R', 'G', '1', '6');
			break;
		case 24:
			f.pixel_format = fourcc('X', 'R', '2', '4');
			break;
		case 30:
			f.pixel_format = fourcc('X', 'R', '3', '0');
			break;
		}
#undef fourcc

		if (drmIoctl(sna->kgem.fd, LOCAL_IOCTL_MODE_ADDFB2, &f)) {
fail:
			xf86DrvMsg(scrn->scrnIndex, X_ERROR,
				   "%s: failed to add fb: %dx%d depth=%d, bpp=%d, pitch=%d: %d\n",
				   __FUNCTION__, width, height,
				   scrn->depth, scrn->bitsPerPixel, bo->pitch, errno);
			return 0;
		}

		arg.fb_id = f.fb_id;
	}
	assert(arg.fb_id != 0);
	bo->delta = arg.fb_id;
	DBG(("%s: attached fb=%d to handle=%d\n",
	     __FUNCTION__, bo->delta, arg.handle));

	bo->scanout = true;
	return bo->delta;
}

static uint32_t gem_create(int fd, int size)
{
	struct drm_i915_gem_create create;

	assert((size & 4095) == 0);

	VG_CLEAR(create);
	create.handle = 0;
	create.size = size;
	(void)drmIoctl(fd, DRM_IOCTL_I915_GEM_CREATE, &create);

	return create.handle;
}

static void *gem_mmap(int fd, int handle, int size)
{
	struct drm_i915_gem_mmap_gtt mmap_arg;
	struct drm_i915_gem_set_domain set_domain;
	void *ptr;

	VG_CLEAR(mmap_arg);
	mmap_arg.handle = handle;
	if (drmIoctl(fd, DRM_IOCTL_I915_GEM_MMAP_GTT, &mmap_arg))
		return NULL;

	ptr = mmap(0, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, mmap_arg.offset);
	if (ptr == MAP_FAILED)
		return NULL;

	VG_CLEAR(set_domain);
	set_domain.handle = handle;
	set_domain.read_domains = I915_GEM_DOMAIN_GTT;
	set_domain.write_domain = I915_GEM_DOMAIN_GTT;
	if (drmIoctl(fd, DRM_IOCTL_I915_GEM_SET_DOMAIN, &set_domain)) {
		munmap(ptr, size);
		return NULL;
	}

	return ptr;
}

static void gem_close(int fd, uint32_t handle)
{
	struct drm_gem_close close;

	VG_CLEAR(close);
	close.handle = handle;
	(void)drmIoctl(fd, DRM_IOCTL_GEM_CLOSE, &close);
}

#define BACKLIGHT_NAME             "Backlight"
#define BACKLIGHT_DEPRECATED_NAME  "BACKLIGHT"
static Atom backlight_atom, backlight_deprecated_atom;

#if HAVE_UDEV
static void
sna_backlight_uevent(int fd, void *closure)
{
	struct sna *sna = closure;
	xf86CrtcConfigPtr config = XF86_CRTC_CONFIG_PTR(sna->scrn);
	int i;

	DBG(("%s()\n", __FUNCTION__));

	/* Drain the event queue */
	while (event_pending(fd)) {
		struct udev_device *dev;

		DBG(("%s: waiting for uevent\n", __FUNCTION__));
		dev = udev_monitor_receive_device(sna->mode.backlight_monitor);
		if (dev == NULL)
			break;

		udev_device_unref(dev);
	}

	/* Query all backlights for any changes */
	DBG(("%s: probing backlights for changes\n", __FUNCTION__));
	for (i = 0; i < sna->mode.num_real_output; i++) {
		xf86OutputPtr output = config->output[i];
		struct sna_output *sna_output = to_sna_output(output);
		int val;

		if (sna_output->dpms_mode != DPMSModeOn)
			continue;

		val = backlight_get(&sna_output->backlight);
		if (val < 0)
			continue;
		DBG(("%s(%s): backlight '%s' was %d, now %d\n",
		     __FUNCTION__, output->name, sna_output->backlight.iface,
		     sna_output->backlight_active_level, val));

		if (val == sna_output->backlight_active_level)
			continue;

		sna_output->backlight_active_level = val;

		if (output->randr_output) {
			DBG(("%s(%s): sending change notification\n", __FUNCTION__, output->name));
			RRChangeOutputProperty(output->randr_output,
					       backlight_atom, XA_INTEGER,
					       32, PropModeReplace, 1, &val,
					       TRUE, FALSE);
			RRChangeOutputProperty(output->randr_output,
					       backlight_deprecated_atom, XA_INTEGER,
					       32, PropModeReplace, 1, &val,
					       TRUE, FALSE);
		}
	}
	DBG(("%s: complete\n", __FUNCTION__));
}

static void sna_backlight_pre_init(struct sna *sna)
{
	struct udev *u;
	struct udev_monitor *mon;

#if !USE_BACKLIGHT
	return;
#endif

	u = udev_new();
	if (!u)
		return;

	mon = udev_monitor_new_from_netlink(u, "udev");
	if (!mon)
		goto free_udev;

	if (udev_monitor_filter_add_match_subsystem_devtype(mon, "backlight", NULL))
		goto free_monitor;

	if (udev_monitor_enable_receiving(mon))
		goto free_monitor;

	sna->mode.backlight_handler =
		xf86AddGeneralHandler(udev_monitor_get_fd(mon),
				      sna_backlight_uevent, sna);
	if (!sna->mode.backlight_handler)
		goto free_monitor;

	DBG(("%s: installed backlight monitor\n", __FUNCTION__));
	sna->mode.backlight_monitor = mon;

	return;

free_monitor:
	udev_monitor_unref(mon);
free_udev:
	udev_unref(u);
}

static void sna_backlight_drain_uevents(struct sna *sna)
{
	if (sna->mode.backlight_monitor == NULL)
		return;

	DBG(("%s()\n", __FUNCTION__));
	sna_backlight_uevent(udev_monitor_get_fd(sna->mode.backlight_monitor),
			     sna);
}

static void sna_backlight_close(struct sna *sna)
{
	struct udev *u;

	if (sna->mode.backlight_handler == NULL)
		return;

	xf86RemoveGeneralHandler(sna->mode.backlight_handler);

	u = udev_monitor_get_udev(sna->mode.backlight_monitor);
	udev_monitor_unref(sna->mode.backlight_monitor);
	udev_unref(u);

	sna->mode.backlight_handler = NULL;
	sna->mode.backlight_monitor = NULL;
}
#else
static void sna_backlight_pre_init(struct sna *sna) { }
static void sna_backlight_drain_uevents(struct sna *sna) { }
static void sna_backlight_close(struct sna *sna) { }
#endif

static void
sna_output_backlight_disable(struct sna_output *sna_output)
{
	xf86OutputPtr output = sna_output->base;

	xf86DrvMsg(output->scrn->scrnIndex, X_ERROR,
		   "Failed to set backlight %s for output %s, disabling\n",
		   sna_output->backlight.iface, output->name);
	backlight_disable(&sna_output->backlight);
	if (output->randr_output) {
		RRDeleteOutputProperty(output->randr_output, backlight_atom);
		RRDeleteOutputProperty(output->randr_output, backlight_deprecated_atom);
	}
}

static int
sna_output_backlight_set(struct sna_output *sna_output, int level)
{
	int ret = 0;

	DBG(("%s(%s) level=%d, max=%d\n", __FUNCTION__,
	     sna_output->base->name, level, sna_output->backlight.max));

	if (backlight_set(&sna_output->backlight, level)) {
		sna_output_backlight_disable(sna_output);
		ret = -1;
	}

	/* Consume the uevent notification now so that we don't misconstrue
	 * the change latter when we wake up and the output is in a different
	 * state.
	 */
	sna_backlight_drain_uevents(to_sna(sna_output->base->scrn));
	return ret;
}

static bool
has_native_backlight(struct sna_output *sna_output)
{
	return sna_output->backlight.type == BL_RAW;
}

static void
sna_output_backlight_off(struct sna_output *sna_output)
{
	/* Trust the kernel to turn the native backlight off. However, we
	 * do explicitly turn the backlight back on (when we wake the output)
	 * just in case a third party turns it off!
	 */
	if (has_native_backlight(sna_output))
		return;

	DBG(("%s(%s)\n", __FUNCTION__, sna_output->base->name));
	backlight_off(&sna_output->backlight);
	sna_output_backlight_set(sna_output, 0);
}

static void
sna_output_backlight_on(struct sna_output *sna_output)
{
	DBG(("%s(%s)\n", __FUNCTION__, sna_output->base->name));
	sna_output_backlight_set(sna_output,
				 sna_output->backlight_active_level);
	if (backlight_on(&sna_output->backlight) < 0)
		sna_output_backlight_disable(sna_output);
}

static int
sna_output_backlight_get(xf86OutputPtr output)
{
	struct sna_output *sna_output = output->driver_private;
	int level = backlight_get(&sna_output->backlight);
	DBG(("%s(%s) level=%d, max=%d\n", __FUNCTION__,
	     output->name, level, sna_output->backlight.max));
	return level;
}

static char *
has_user_backlight_override(xf86OutputPtr output)
{
	struct sna *sna = to_sna(output->scrn);
	const char *str;

	str = xf86GetOptValString(sna->Options, OPTION_BACKLIGHT);
	if (str == NULL)
		return NULL;

	DBG(("%s(%s) requested %s\n", __FUNCTION__, output->name, str));
	if (*str == '\0')
		return (char *)str;

	if (!backlight_exists(str)) {
		xf86DrvMsg(output->scrn->scrnIndex, X_ERROR,
			   "Unrecognised backlight control interface '%s'\n",
			   str);
		return NULL;
	}

	return strdup(str);
}

static int get_device_minor(int fd)
{
	struct stat st;

	if (fstat(fd, &st) || !S_ISCHR(st.st_mode))
		return -1;

	return st.st_rdev & 0x63;
}

static const char * const sysfs_connector_types[] = {
	/* DRM_MODE_CONNECTOR_Unknown */	"Unknown",
	/* DRM_MODE_CONNECTOR_VGA */		"VGA",
	/* DRM_MODE_CONNECTOR_DVII */		"DVI-I",
	/* DRM_MODE_CONNECTOR_DVID */		"DVI-D",
	/* DRM_MODE_CONNECTOR_DVIA */		"DVI-A",
	/* DRM_MODE_CONNECTOR_Composite */	"Composite",
	/* DRM_MODE_CONNECTOR_SVIDEO */		"SVIDEO",
	/* DRM_MODE_CONNECTOR_LVDS */		"LVDS",
	/* DRM_MODE_CONNECTOR_Component */	"Component",
	/* DRM_MODE_CONNECTOR_9PinDIN */	"DIN",
	/* DRM_MODE_CONNECTOR_DisplayPort */	"DP",
	/* DRM_MODE_CONNECTOR_HDMIA */		"HDMI-A",
	/* DRM_MODE_CONNECTOR_HDMIB */		"HDMI-B",
	/* DRM_MODE_CONNECTOR_TV */		"TV",
	/* DRM_MODE_CONNECTOR_eDP */		"eDP",
	/* DRM_MODE_CONNECTOR_VIRTUAL */	"Virtual",
	/* DRM_MODE_CONNECTOR_DSI */		"DSI",
	/* DRM_MODE_CONNECTOR_DPI */		"DPI"
};

static char *has_connector_backlight(xf86OutputPtr output)
{
	struct sna_output *sna_output = output->driver_private;
	struct sna *sna = to_sna(output->scrn);
	char path[1024];
	DIR *dir;
	struct dirent *de;
	int minor, len;
	char *str = NULL;

	if (sna_output->connector_type >= ARRAY_SIZE(sysfs_connector_types))
		return NULL;

	minor = get_device_minor(sna->kgem.fd);
	if (minor < 0)
		return NULL;

	len = snprintf(path, sizeof(path),
		       "/sys/class/drm/card%d-%s-%d",
		       minor,
		       sysfs_connector_types[sna_output->connector_type],
		       sna_output->connector_type_id);
	DBG(("%s: lookup %s\n", __FUNCTION__, path));

	dir = opendir(path);
	if (dir == NULL)
		return NULL;

	while ((de = readdir(dir))) {
		struct stat st;

		if (*de->d_name == '.')
			continue;

		snprintf(path + len, sizeof(path) - len,
			 "/%s", de->d_name);

		if (stat(path, &st))
			continue;

		if (!S_ISDIR(st.st_mode))
			continue;

		DBG(("%s: testing %s as backlight\n",
		     __FUNCTION__, de->d_name));

		if (backlight_exists(de->d_name)) {
			str = strdup(de->d_name); /* leak! */
			break;
		}
	}

	closedir(dir);
	return str;
}

static void
sna_output_backlight_init(xf86OutputPtr output)
{
	struct sna_output *sna_output = output->driver_private;
	struct pci_device *pci;
	MessageType from;
	char *best_iface;

#if !USE_BACKLIGHT
	return;
#endif

	if (sna_output->is_panel) {
		from = X_CONFIG;
		best_iface = has_user_backlight_override(output);
		if (best_iface)
			goto done;
	}

	best_iface = has_connector_backlight(output);
	if (best_iface)
		goto done;

	if (!sna_output->is_panel)
		return;

	/* XXX detect right backlight for multi-GPU/panels */
	from = X_PROBED;
	pci = xf86GetPciInfoForEntity(to_sna(output->scrn)->pEnt->index);
	if (pci != NULL)
		best_iface = backlight_find_for_device(pci);

done:
	DBG(("%s(%s) opening backlight %s\n", __FUNCTION__,
	     output->name, best_iface ?: "none"));
	sna_output->backlight_active_level =
		backlight_open(&sna_output->backlight, best_iface);
	DBG(("%s(%s): initial backlight value %d\n",
	     __FUNCTION__, output->name, sna_output->backlight_active_level));
	if (sna_output->backlight_active_level < 0)
		return;

	switch (sna_output->backlight.type) {
	case BL_FIRMWARE: best_iface = (char *)"firmware"; break;
	case BL_PLATFORM: best_iface = (char *)"platform"; break;
	case BL_RAW: best_iface = (char *)"raw"; break;
	default: best_iface = (char *)"unknown"; break;
	}
	xf86DrvMsg(output->scrn->scrnIndex, from,
		   "Found backlight control interface %s (type '%s') for output %s\n",
		   sna_output->backlight.iface, best_iface, output->name);
}

#if ABI_VIDEODRV_VERSION >= SET_ABI_VERSION(22, 0)
static inline int sigio_block(void)
{
	return 0;
}
static inline void sigio_unblock(int was_blocked)
{
	(void)was_blocked;
}
#elif XORG_VERSION_CURRENT >= XORG_VERSION_NUMERIC(1,12,99,901,0)
static inline int sigio_block(void)
{
	OsBlockSIGIO();
	return 0;
}
static inline void sigio_unblock(int was_blocked)
{
	OsReleaseSIGIO();
	(void)was_blocked;
}
#else
#include <xf86_OSproc.h>
static inline int sigio_block(void)
{
	return xf86BlockSIGIO();
}
static inline void sigio_unblock(int was_blocked)
{
	xf86UnblockSIGIO(was_blocked);
}
#endif

static char *canonical_kmode_name(const struct drm_mode_modeinfo *kmode)
{
	char tmp[32], *buf;
	int len;

	len = sprintf(tmp, "%dx%d%s",
		      kmode->hdisplay, kmode->vdisplay,
		      kmode->flags & V_INTERLACE ? "i" : "");
	if ((unsigned)len >= sizeof(tmp))
		return NULL;

	buf = malloc(len + 1);
	if (buf == NULL)
		return NULL;

	return memcpy(buf, tmp, len + 1);
}

static char *get_kmode_name(const struct drm_mode_modeinfo *kmode)
{
	if (*kmode->name == '\0')
		return canonical_kmode_name(kmode);

	return strdup(kmode->name);
}

static DisplayModePtr
mode_from_kmode(ScrnInfoPtr scrn,
		const struct drm_mode_modeinfo *kmode,
		DisplayModePtr mode)
{
	DBG(("kmode: %s, clock=%d, %d %d %d %d %d, %d %d %d %d %d, flags=%x, type=%x\n",
	     kmode->name, kmode->clock,
	     kmode->hdisplay, kmode->hsync_start, kmode->hsync_end, kmode->htotal, kmode->hskew,
	     kmode->vdisplay, kmode->vsync_start, kmode->vsync_end, kmode->vtotal, kmode->vscan,
	     kmode->flags, kmode->type));

	mode->status = MODE_OK;

	mode->Clock = kmode->clock;

	mode->HDisplay = kmode->hdisplay;
	mode->HSyncStart = kmode->hsync_start;
	mode->HSyncEnd = kmode->hsync_end;
	mode->HTotal = kmode->htotal;
	mode->HSkew = kmode->hskew;

	mode->VDisplay = kmode->vdisplay;
	mode->VSyncStart = kmode->vsync_start;
	mode->VSyncEnd = kmode->vsync_end;
	mode->VTotal = kmode->vtotal;
	mode->VScan = kmode->vscan;

	mode->VRefresh = kmode->vrefresh;
	mode->Flags = kmode->flags;
	mode->name = get_kmode_name(kmode);

	if (kmode->type & DRM_MODE_TYPE_DRIVER)
		mode->type = M_T_DRIVER;
	if (kmode->type & DRM_MODE_TYPE_PREFERRED)
		mode->type |= M_T_PREFERRED;

	if (mode->status == MODE_OK && kmode->flags & ~KNOWN_MODE_FLAGS)
		mode->status = MODE_BAD; /* unknown flags => unhandled */

	xf86SetModeCrtc(mode, scrn->adjustFlags);
	return mode;
}

static void
mode_to_kmode(struct drm_mode_modeinfo *kmode, DisplayModePtr mode)
{
	memset(kmode, 0, sizeof(*kmode));

	kmode->clock = mode->Clock;
	kmode->hdisplay = mode->HDisplay;
	kmode->hsync_start = mode->HSyncStart;
	kmode->hsync_end = mode->HSyncEnd;
	kmode->htotal = mode->HTotal;
	kmode->hskew = mode->HSkew;

	kmode->vdisplay = mode->VDisplay;
	kmode->vsync_start = mode->VSyncStart;
	kmode->vsync_end = mode->VSyncEnd;
	kmode->vtotal = mode->VTotal;
	kmode->vscan = mode->VScan;

	kmode->vrefresh = mode->VRefresh;
	kmode->flags = mode->Flags;
	if (mode->name)
		strncpy(kmode->name, mode->name, DRM_DISPLAY_MODE_LEN);
	kmode->name[DRM_DISPLAY_MODE_LEN-1] = 0;
}

static void
sna_crtc_force_outputs_on(xf86CrtcPtr crtc)
{
	xf86CrtcConfigPtr config = XF86_CRTC_CONFIG_PTR(crtc->scrn);
	/* All attached outputs are valid, so update our timestamps */
	unsigned now = GetTimeInMillis();
	int i;

	assert(to_sna_crtc(crtc));
	DBG(("%s(pipe=%d)\n", __FUNCTION__, sna_crtc_pipe(crtc)));

	/* DPMS handling by the kernel is inconsistent, so after setting a
	 * mode on an output presume that we intend for it to be on, or that
	 * the kernel will force it on.
	 *
	 * So force DPMS to be on for all connected outputs, and restore
	 * the backlight.
	 */
	for (i = 0; i < config->num_output; i++) {
		xf86OutputPtr output = config->output[i];

		if (output->crtc != crtc)
			continue;

		__sna_output_dpms(output, DPMSModeOn, false);
		if (to_sna_output(output)->last_detect)
			to_sna_output(output)->last_detect = now;
	}

#if XF86_CRTC_VERSION >= 3
	crtc->active = TRUE;
#endif
}

static void
sna_crtc_force_outputs_off(xf86CrtcPtr crtc)
{
	xf86CrtcConfigPtr config = XF86_CRTC_CONFIG_PTR(crtc->scrn);
	int i;

	assert(to_sna_crtc(crtc));
	DBG(("%s(pipe=%d)\n", __FUNCTION__, sna_crtc_pipe(crtc)));

	/* DPMS handling by the kernel is inconsistent, so after setting a
	 * mode on an output presume that we intend for it to be on, or that
	 * the kernel will force it on.
	 *
	 * So force DPMS to be on for all connected outputs, and restore
	 * the backlight.
	 */
	for (i = 0; i < config->num_output; i++) {
		xf86OutputPtr output = config->output[i];

		if (output->crtc != crtc)
			continue;

		__sna_output_dpms(output, DPMSModeOff, false);
	}
}

static unsigned
rotation_reflect(unsigned rotation)
{
	unsigned other_bits;

	/* paranoia for future extensions */
	other_bits = rotation & ~RR_Rotate_All;

	/* flip the reflection to compensate for reflecting the rotation */
	other_bits ^= RR_Reflect_X | RR_Reflect_Y;

	/* Reflect the screen by rotating the rotation bit,
	 * which has to have at least RR_Rotate_0 set. This allows
	 * us to reflect any of the rotation bits, not just 0.
	 */
	rotation &= RR_Rotate_All;
	assert(rotation);
	rotation <<= 2; /* RR_Rotate_0 -> RR_Rotate_180 etc */
	rotation |= rotation >> 4; /* RR_Rotate_270' to RR_Rotate_90 */
	rotation &= RR_Rotate_All;
	assert(rotation);

	return rotation | other_bits;
}

static unsigned
rotation_reduce(struct plane *p, unsigned rotation)
{
	/* If unsupported try exchanging rotation for a reflection */
	if (rotation & ~p->rotation.supported) {
		unsigned new_rotation = rotation_reflect(rotation);
		if ((new_rotation & p->rotation.supported) == new_rotation)
			rotation = new_rotation;
	}

	/* Only one rotation bit should be set */
	assert(is_power_of_two(rotation & RR_Rotate_All));

	return rotation;
}

static bool
rotation_set(struct sna *sna, struct plane *p, uint32_t desired)
{
#define LOCAL_IOCTL_MODE_OBJ_SETPROPERTY DRM_IOWR(0xbA, struct local_mode_obj_set_property)
	struct local_mode_obj_set_property {
		uint64_t value;
		uint32_t prop_id;
		uint32_t obj_id;
		uint32_t obj_type;
		uint32_t pad;
	} prop;

	if (desired == p->rotation.current)
		return true;

	if ((desired & p->rotation.supported) != desired) {
		errno = EINVAL;
		return false;
	}

	DBG(("%s: obj=%d, type=%x prop=%d set-rotation=%x\n",
	     __FUNCTION__, p->id, LOCAL_MODE_OBJECT_PLANE, p->rotation.prop, desired));

	assert(p->id);
	assert(p->rotation.prop);

	VG_CLEAR(prop);
	prop.obj_id = p->id;
	prop.obj_type = LOCAL_MODE_OBJECT_PLANE;
	prop.prop_id = p->rotation.prop;
	prop.value = desired;

	if (drmIoctl(sna->kgem.fd, LOCAL_IOCTL_MODE_OBJ_SETPROPERTY, &prop))
		return false;

	p->rotation.current = desired;
	return true;
}

static void
rotation_reset(struct plane *p)
{
	if (p->rotation.prop == 0)
		return;

	p->rotation.current = 0;
}

bool sna_crtc_set_sprite_rotation(xf86CrtcPtr crtc,
				  unsigned idx,
				  uint32_t rotation)
{
	struct plane *sprite;
	assert(to_sna_crtc(crtc));

	sprite = lookup_sprite(to_sna_crtc(crtc), idx);
	if (!sprite)
		return false;

	DBG(("%s: CRTC:%d [pipe=%d], sprite=%u set-rotation=%x\n",
	     __FUNCTION__,
	     sna_crtc_id(crtc), sna_crtc_pipe(crtc),
	     sprite->id, rotation));

	return rotation_set(to_sna(crtc->scrn), sprite,
			    rotation_reduce(sprite, rotation));
}

#if HAS_DEBUG_FULL
#if !HAS_DEBUG_FULL
#define LogF ErrorF
#endif
struct kmsg {
	int fd;
	int saved_loglevel;
};

static int kmsg_get_debug(void)
{
	int v = -1;
	int fd;

	fd = open("/sys/module/drm/parameters/debug", O_RDONLY);
	if (fd != -1) {
		char buf[128];
		int len;

		len = read(fd, buf, sizeof(buf) - 1);
		if (len != -1) {
			buf[len] = '\0';
			v = atoi(buf);
		}
		close(fd);
	}

	return v;
}

static void kmsg_set_debug(int v)
{
	char buf[128];
	int len;
	int fd;

	len = snprintf(buf, sizeof(buf), "%d\n", v);

	fd = open("/sys/module/drm/parameters/debug", O_WRONLY);
	if (fd != -1) {
		write(fd, buf, len);
		close(fd);
	}
}

static void kmsg_open(struct kmsg *k)
{
	k->saved_loglevel = kmsg_get_debug();
	if (k->saved_loglevel != -1)
		kmsg_set_debug(0xff);

	k->fd = open("/dev/kmsg", O_RDONLY | O_NONBLOCK);
	if (k->fd != -1)
		lseek(k->fd, 0, SEEK_END);
}

static void kmsg_close(struct kmsg *k, int dump)
{
	FILE *file;

	file = NULL;
	if (k->fd != -1 && dump)
		file = fdopen(k->fd, "r");
	if (file) {
		size_t len = 0;
		char *line = NULL;

		while (getline(&line, &len, file) != -1) {
			char *start = strchr(line, ';');
			if (start)
				LogF("KMSG: %s", start + 1);
		}

		free(line);
		fclose(file);
	}

	if (k->fd != -1)
		close(k->fd);

	if (k->saved_loglevel != -1)
		kmsg_set_debug(k->saved_loglevel);
}
#else
struct kmsg { int unused; };
static void kmsg_open(struct kmsg *k) {}
static void kmsg_close(struct kmsg *k, int dump) {}
#endif

static int
sna_crtc_apply(xf86CrtcPtr crtc)
{
	struct sna *sna = to_sna(crtc->scrn);
	struct sna_crtc *sna_crtc = to_sna_crtc(crtc);
	xf86CrtcConfigPtr config = XF86_CRTC_CONFIG_PTR(crtc->scrn);
	struct drm_mode_crtc arg;
	uint32_t output_ids[32];
	int output_count = 0;
	int sigio, i;
	struct kmsg kmsg;
	int ret = EINVAL;

	DBG(("%s CRTC:%d [pipe=%d], handle=%d\n", __FUNCTION__,
	     __sna_crtc_id(sna_crtc), __sna_crtc_pipe(sna_crtc),
	     sna_crtc->bo->handle));
	if (!sna_crtc->kmode.clock) {
		ERR(("%s(CRTC:%d [pipe=%d]): attempted to set an invalid mode\n",
		     __FUNCTION__, __sna_crtc_id(sna_crtc), __sna_crtc_pipe(sna_crtc)));
		return EINVAL;
	}

	kmsg_open(&kmsg);
	sigio = sigio_block();

	assert(sna->mode.num_real_output < ARRAY_SIZE(output_ids));
	sna_crtc_disable_cursor(sna, sna_crtc);

	if (!rotation_set(sna, &sna_crtc->primary, sna_crtc->rotation)) {
		memset(&arg, 0, sizeof(arg));
		arg.crtc_id = __sna_crtc_id(sna_crtc);
		(void)drmIoctl(sna->kgem.fd, DRM_IOCTL_MODE_SETCRTC, &arg);
	}

	if (!rotation_set(sna, &sna_crtc->primary, sna_crtc->rotation)) {
		ERR(("%s: set-primary-rotation failed (rotation-id=%d, rotation=%d) on CRTC:%d [pipe=%d], errno=%d\n",
		     __FUNCTION__, sna_crtc->primary.rotation.prop, sna_crtc->rotation, __sna_crtc_id(sna_crtc), __sna_crtc_pipe(sna_crtc), errno));
		sna_crtc->primary.rotation.supported &= ~sna_crtc->rotation;
		goto unblock;
	}
	DBG(("%s: CRTC:%d [pipe=%d] primary rotation set to %x\n",
	     __FUNCTION__, __sna_crtc_id(sna_crtc), __sna_crtc_pipe(sna_crtc), sna_crtc->rotation));

	for (i = 0; i < sna->mode.num_real_output; i++) {
		xf86OutputPtr output = config->output[i];

		/* Make sure we mark the output as off (and save the backlight)
		 * before the kernel turns it off due to changing the pipe.
		 * This is necessary as the kernel may turn off the backlight
		 * and we lose track of the user settings.
		 */
		if (output->crtc == NULL)
			__sna_output_dpms(output, DPMSModeOff, false);

		if (output->crtc != crtc)
			continue;

		/* Skip over any hotunplugged outputs so that we can
		 * recover in cases where the previous mode is now
		 * only partially valid.
		 */
		if (!to_sna_output(output)->id)
			continue;

		DBG(("%s: attaching output '%s' %d [%d] to crtc:%d (pipe %d) (possible crtc:%x, possible clones:%x)\n",
		     __FUNCTION__, output->name, i, to_connector_id(output),
		     __sna_crtc_id(sna_crtc), __sna_crtc_pipe(sna_crtc),
		     (uint32_t)output->possible_crtcs,
		     (uint32_t)output->possible_clones));

		assert(output->possible_crtcs & (1 << __sna_crtc_pipe(sna_crtc)) ||
		       is_zaphod(crtc->scrn));

		output_ids[output_count] = to_connector_id(output);
		if (++output_count == ARRAY_SIZE(output_ids)) {
			DBG(("%s: too many outputs (%d) for me!\n",
			     __FUNCTION__, output_count));
			goto unblock;
		}
	}
	if (output_count == 0) {
		DBG(("%s: no outputs\n", __FUNCTION__));
		goto unblock;
	}

	VG_CLEAR(arg);
	arg.crtc_id = __sna_crtc_id(sna_crtc);
	arg.fb_id = fb_id(sna_crtc->bo);
	if (sna_crtc->transform || sna_crtc->slave_pixmap) {
		arg.x = 0;
		arg.y = 0;
		sna_crtc->offset = 0;
	} else {
		arg.x = crtc->x;
		arg.y = crtc->y;
		sna_crtc->offset = arg.y << 16 | arg.x;
	}
	arg.set_connectors_ptr = (uintptr_t)output_ids;
	arg.count_connectors = output_count;
	arg.mode = sna_crtc->kmode;
	arg.mode_valid = 1;

	DBG(("%s: applying crtc [%d, pipe=%d] mode=%dx%d+%d+%d@%d, fb=%d%s%s update to %d outputs [%d...]\n",
	     __FUNCTION__, __sna_crtc_id(sna_crtc), __sna_crtc_pipe(sna_crtc),
	     arg.mode.hdisplay,
	     arg.mode.vdisplay,
	     arg.x, arg.y,
	     arg.mode.clock,
	     arg.fb_id,
	     sna_crtc->shadow ? " [shadow]" : "",
	     sna_crtc->transform ? " [transformed]" : "",
	     output_count, output_count ? output_ids[0] : 0));

	ret = 0;
	if (unlikely(drmIoctl(sna->kgem.fd, DRM_IOCTL_MODE_SETCRTC, &arg))) {
		ret = errno;
		goto unblock;
	}

	sna_crtc->mode_serial++;
	sna_crtc_force_outputs_on(crtc);

unblock:
	sigio_unblock(sigio);
	kmsg_close(&kmsg, ret);
	return ret;
}

static bool overlap(const BoxRec *a, const BoxRec *b)
{
	if (a->x1 >= b->x2)
		return false;
	if (a->x2 <= b->x1)
		return false;

	if (a->y1 >= b->y2)
		return false;
	if (a->y2 <= b->y1)
		return false;

	return true;
}

static void defer_event(struct sna *sna, struct drm_event *base)
{
	if (sna->mode.shadow_nevent == sna->mode.shadow_size) {
		int size = sna->mode.shadow_size * 2;
		void *ptr;

		ptr = realloc(sna->mode.shadow_events,
			      sizeof(struct drm_event_vblank)*size);
		if (!ptr)
			return;

		sna->mode.shadow_events = ptr;
		sna->mode.shadow_size = size;
	}

	memcpy(&sna->mode.shadow_events[sna->mode.shadow_nevent++],
	       base, sizeof(struct drm_event_vblank));
	DBG(("%s: deferring event count=%d\n",
	     __func__, sna->mode.shadow_nevent));
}

static void flush_events(struct sna *sna)
{
	int n;

	if (!sna->mode.shadow_nevent)
		return;

	DBG(("%s: flushing %d events=%d\n", __func__, sna->mode.shadow_nevent));

	for (n = 0; n < sna->mode.shadow_nevent; n++) {
		struct drm_event_vblank *vb = &sna->mode.shadow_events[n];

		if ((uintptr_t)(vb->user_data) & 2)
			sna_present_vblank_handler(vb);
		else
			sna_dri2_vblank_handler(vb);
	}

	sna->mode.shadow_nevent = 0;
}


static bool wait_for_shadow(struct sna *sna,
			    struct sna_pixmap *priv,
			    unsigned flags)
{
	PixmapPtr pixmap = priv->pixmap;
	struct kgem_bo *bo, *tmp;
	int flip_active;
	bool ret = true;

	DBG(("%s: enabled? %d flags=%x, flips=%d, pixmap=%ld [front?=%d], handle=%d, shadow=%d\n",
	     __FUNCTION__, sna->mode.shadow_enabled,
	     flags, sna->mode.flip_active,
	     pixmap->drawable.serialNumber, pixmap == sna->front,
	     priv->gpu_bo->handle, sna->mode.shadow->handle));

	assert(priv->move_to_gpu_data == sna);
	assert(sna->mode.shadow != priv->gpu_bo);

	if (flags == 0 || pixmap != sna->front || !sna->mode.shadow_enabled)
		goto done;

	assert(sna->mode.shadow_damage);

	if ((flags & MOVE_WRITE) == 0) {
		if ((flags & __MOVE_SCANOUT) == 0) {
			struct sna_crtc *crtc;

			list_for_each_entry(crtc, &sna->mode.shadow_crtc, shadow_link) {
				if (overlap(&sna->mode.shadow_region.extents,
					    &crtc->base->bounds)) {
					DrawableRec draw;
					RegionRec region;

					draw.width = crtc->base->mode.HDisplay;
					draw.height = crtc->base->mode.VDisplay;
					draw.depth = sna->front->drawable.depth;
					draw.bitsPerPixel = sna->front->drawable.bitsPerPixel;

					DBG(("%s: copying replaced CRTC: (%d, %d), (%d, %d), handle=%d\n",
					     __FUNCTION__,
					     crtc->base->bounds.x1,
					     crtc->base->bounds.y1,
					     crtc->base->bounds.x2,
					     crtc->base->bounds.y2,
					     crtc->client_bo->handle));

					ret &= sna->render.copy_boxes(sna, GXcopy,
								      &draw, crtc->client_bo, -crtc->base->bounds.x1, -crtc->base->bounds.y1,
								      &pixmap->drawable, priv->gpu_bo, 0, 0,
								      &crtc->base->bounds, 1,
								      0);

					region.extents = crtc->base->bounds;
					region.data = NULL;
					RegionSubtract(&sna->mode.shadow_region, &sna->mode.shadow_region, &region);
				}
			}
		}

		return ret;
	}

	assert(sna->mode.shadow_active);

	flip_active = sna->mode.flip_active;
	if (flip_active) {
		struct sna_crtc *crtc;
		list_for_each_entry(crtc, &sna->mode.shadow_crtc, shadow_link)
			flip_active -= crtc->flip_pending;
		DBG(("%s: %d flips still pending, shadow flip_active=%d\n",
		     __FUNCTION__, sna->mode.flip_active, flip_active));
	}

	bo = sna->mode.shadow;
	if (flip_active) {
		bo = kgem_create_2d(&sna->kgem,
				    pixmap->drawable.width,
				    pixmap->drawable.height,
				    pixmap->drawable.bitsPerPixel,
				    priv->gpu_bo->tiling,
				    CREATE_EXACT | CREATE_SCANOUT);
		if (bo == NULL)
			return false;

		DBG(("%s: replacing still-attached GPU bo handle=%d, flips=%d\n",
		     __FUNCTION__, priv->gpu_bo->tiling, sna->mode.flip_active));

		RegionUninit(&sna->mode.shadow_region);
		sna->mode.shadow_region.extents.x1 = 0;
		sna->mode.shadow_region.extents.y1 = 0;
		sna->mode.shadow_region.extents.x2 = pixmap->drawable.width;
		sna->mode.shadow_region.extents.y2 = pixmap->drawable.height;
		sna->mode.shadow_region.data = NULL;
	}

	if (bo->refcnt > 1) {
		bo = kgem_create_2d(&sna->kgem,
				    pixmap->drawable.width,
				    pixmap->drawable.height,
				    pixmap->drawable.bitsPerPixel,
				    priv->gpu_bo->tiling,
				    CREATE_EXACT | CREATE_SCANOUT);
		if (bo != NULL) {
			DBG(("%s: replacing exported GPU bo\n",
			     __FUNCTION__));

			RegionUninit(&sna->mode.shadow_region);
			sna->mode.shadow_region.extents.x1 = 0;
			sna->mode.shadow_region.extents.y1 = 0;
			sna->mode.shadow_region.extents.x2 = pixmap->drawable.width;
			sna->mode.shadow_region.extents.y2 = pixmap->drawable.height;
			sna->mode.shadow_region.data = NULL;
		} else
			bo = sna->mode.shadow;
	}

	RegionSubtract(&sna->mode.shadow_region,
		       &sna->mode.shadow_region,
		       &sna->mode.shadow_cancel);

	while (!list_is_empty(&sna->mode.shadow_crtc)) {
		struct sna_crtc *crtc =
			list_first_entry(&sna->mode.shadow_crtc, struct sna_crtc, shadow_link);
		if (overlap(&crtc->base->bounds,
			    &sna->mode.shadow_region.extents)) {
			RegionRec region;
			DrawableRec draw;

			draw.width = crtc->base->mode.HDisplay;
			draw.height = crtc->base->mode.VDisplay;
			draw.depth = sna->front->drawable.depth;
			draw.bitsPerPixel = sna->front->drawable.bitsPerPixel;

			DBG(("%s: copying replaced CRTC: (%d, %d), (%d, %d), handle=%d\n",
			     __FUNCTION__,
			     crtc->base->bounds.x1,
			     crtc->base->bounds.y1,
			     crtc->base->bounds.x2,
			     crtc->base->bounds.y2,
			     crtc->client_bo->handle));

			ret = sna->render.copy_boxes(sna, GXcopy,
						     &draw, crtc->client_bo, -crtc->base->bounds.x1, -crtc->base->bounds.y1,
						     &pixmap->drawable, bo, 0, 0,
						     &crtc->base->bounds, 1,
						     0);


			region.extents = crtc->base->bounds;
			region.data = NULL;
			RegionSubtract(&sna->mode.shadow_region, &sna->mode.shadow_region, &region);
		}

		crtc->client_bo->active_scanout--;
		kgem_bo_destroy(&sna->kgem, crtc->client_bo);
		crtc->client_bo = NULL;
		list_del(&crtc->shadow_link);
	}

	if (RegionNotEmpty(&sna->mode.shadow_region)) {
		DBG(("%s: copying existing GPU damage: %dx(%d, %d), (%d, %d)\n",
		     __FUNCTION__, region_num_rects(&sna->mode.shadow_region),
		     sna->mode.shadow_region.extents.x1,
		     sna->mode.shadow_region.extents.y1,
		     sna->mode.shadow_region.extents.x2,
		     sna->mode.shadow_region.extents.y2));
		if (!sna->render.copy_boxes(sna, GXcopy,
					    &pixmap->drawable, priv->gpu_bo, 0, 0,
					    &pixmap->drawable, bo, 0, 0,
					    region_rects(&sna->mode.shadow_region),
					    region_num_rects(&sna->mode.shadow_region),
					    0))
			ERR(("%s: copy failed\n", __FUNCTION__));
	}

	if (priv->cow)
		sna_pixmap_undo_cow(sna, priv, 0);

	sna_pixmap_unmap(pixmap, priv);

	DBG(("%s: setting front pixmap to handle=%d\n", __FUNCTION__, bo->handle));
	assert(sna->mode.shadow->active_scanout);
	sna->mode.shadow->active_scanout--;
	tmp = priv->gpu_bo;
	priv->gpu_bo = bo;
	if (bo != sna->mode.shadow)
		kgem_bo_destroy(&sna->kgem, sna->mode.shadow);
	sna->mode.shadow = tmp;
	sna->mode.shadow->active_scanout++;
	assert(sna->mode.shadow->active_scanout);

	sna_dri2_pixmap_update_bo(sna, pixmap, bo);

done:
	RegionEmpty(&sna->mode.shadow_cancel);
	RegionEmpty(&sna->mode.shadow_region);
	sna->mode.shadow_dirty = false;

	priv->move_to_gpu_data = NULL;
	priv->move_to_gpu = NULL;

	assert(sna->mode.shadow->active_scanout);
	return ret;
}

bool sna_pixmap_discard_shadow_damage(struct sna_pixmap *priv,
				      const RegionRec *region)
{
	struct sna *sna;

	if (priv->move_to_gpu != wait_for_shadow)
		return false;

	sna = priv->move_to_gpu_data;
	if (region) {
		DBG(("%s: discarding region %dx[(%d, %d), (%d, %d)] from damage %dx[(%d, %d], (%d, %d)]\n",
		     __FUNCTION__,
		     region_num_rects(region),
		     region->extents.x1, region->extents.y1,
		     region->extents.x2, region->extents.y2,
		     region_num_rects(&sna->mode.shadow_region),
		     sna->mode.shadow_region.extents.x1, sna->mode.shadow_region.extents.y1,
		     sna->mode.shadow_region.extents.x2, sna->mode.shadow_region.extents.y2));

		RegionSubtract(&sna->mode.shadow_region,
			       &sna->mode.shadow_region,
			       (RegionPtr)region);
		RegionUnion(&sna->mode.shadow_cancel,
			    &sna->mode.shadow_cancel,
			    (RegionPtr)region);
	} else {
		DBG(("%s: discarding all damage %dx[(%d, %d], (%d, %d)]\n",
		     __FUNCTION__,
		     region_num_rects(&sna->mode.shadow_region),
		     sna->mode.shadow_region.extents.x1, sna->mode.shadow_region.extents.y1,
		     sna->mode.shadow_region.extents.x2, sna->mode.shadow_region.extents.y2));
		RegionEmpty(&sna->mode.shadow_region);

		RegionUninit(&sna->mode.shadow_cancel);
		sna->mode.shadow_cancel.extents.x1 = 0;
		sna->mode.shadow_cancel.extents.y1 = 0;
		sna->mode.shadow_cancel.extents.x2 = sna->front->drawable.width;
		sna->mode.shadow_cancel.extents.y2 = sna->front->drawable.height;
		sna->mode.shadow_cancel.data = NULL;
	}

	return RegionNil(&sna->mode.shadow_region);
}

static void sna_mode_damage(DamagePtr damage, RegionPtr region, void *closure)
{
	struct sna *sna = closure;

	if (sna->mode.rr_active)
		return;

	/* Throw away the rectangles if the region grows too big */
	region = DamageRegion(damage);
	if (region->data) {
		RegionRec dup;

		dup = *region;
		RegionUninit(&dup);

		region->data = NULL;
	}
}

static bool sna_mode_enable_shadow(struct sna *sna)
{
	ScreenPtr screen = to_screen_from_sna(sna);

	DBG(("%s\n", __FUNCTION__));
	assert(sna->mode.shadow == NULL);
	assert(sna->mode.shadow_damage == NULL);
	assert(sna->mode.shadow_active == 0);
	assert(!sna->mode.shadow_enabled);

	sna->mode.shadow_damage = DamageCreate(sna_mode_damage, NULL,
					       DamageReportRawRegion,
					       TRUE, screen, sna);
	if (!sna->mode.shadow_damage)
		return false;

	DamageRegister(&sna->front->drawable, sna->mode.shadow_damage);
	sna->mode.shadow_enabled = true;
	return true;
}

static void sna_mode_disable_shadow(struct sna *sna)
{
	struct sna_pixmap *priv;

	if (!sna->mode.shadow_damage) {
		assert(!sna->mode.shadow_enabled);
		return;
	}

	DBG(("%s\n", __FUNCTION__));

	priv = sna_pixmap(sna->front);
	if (priv->move_to_gpu == wait_for_shadow)
		priv->move_to_gpu(sna, priv, 0);

	DamageUnregister(&sna->front->drawable, sna->mode.shadow_damage);
	DamageDestroy(sna->mode.shadow_damage);
	sna->mode.shadow_damage = NULL;
	sna->mode.shadow_enabled = false;

	if (sna->mode.shadow) {
		sna->mode.shadow->active_scanout--;
		kgem_bo_destroy(&sna->kgem, sna->mode.shadow);
		sna->mode.shadow = NULL;
	}

	assert(sna->mode.shadow_active == 0);
	sna->mode.shadow_dirty = false;
}

static void sna_crtc_slave_damage(DamagePtr damage, RegionPtr region, void *closure)
{
	struct sna_crtc *crtc = closure;
	struct sna *sna = to_sna(crtc->base->scrn);
	RegionPtr scr;

	DBG(("%s: pushing damage [(%d, %d), (%d, %d) x %d] to CRTC [pipe=%d] (%d, %d)\n",
	     __FUNCTION__,
	     region->extents.x1, region->extents.y1, region->extents.x2, region->extents.y2,
	     region_num_rects(region),
	     __sna_crtc_pipe(crtc), crtc->base->x, crtc->base->y));

	assert(crtc->slave_damage == damage);
	assert(sna->mode.shadow_damage);

	RegionTranslate(region, crtc->base->x, crtc->base->y);
	scr = DamageRegion(sna->mode.shadow_damage);
	RegionUnion(scr, scr, region);
	RegionTranslate(region, -crtc->base->x, -crtc->base->y);
}

static bool sna_crtc_enable_shadow(struct sna *sna, struct sna_crtc *crtc)
{
	if (crtc->shadow) {
		assert(sna->mode.shadow_damage && sna->mode.shadow_active);
		return true;
	}

	DBG(("%s: enabling for crtc %d\n", __FUNCTION__, __sna_crtc_id(crtc)));

	if (!sna->mode.shadow_active) {
		if (!sna_mode_enable_shadow(sna))
			return false;
		assert(sna->mode.shadow_damage);
		assert(sna->mode.shadow == NULL);
	}

	if (crtc->slave_pixmap) {
		assert(crtc->slave_damage == NULL);

		DBG(("%s: enabling PRIME slave tracking on CRTC %d [pipe=%d], pixmap=%ld\n",
		     __FUNCTION__, __sna_crtc_id(crtc), __sna_crtc_pipe(crtc), crtc->slave_pixmap->drawable.serialNumber));
		crtc->slave_damage = DamageCreate(sna_crtc_slave_damage, NULL,
						  DamageReportRawRegion, TRUE,
						  to_screen_from_sna(sna),
						  crtc);
		if (crtc->slave_damage == NULL) {
			if (!--sna->mode.shadow_active)
				sna_mode_disable_shadow(sna);
			return false;
		}

		DamageRegister(&crtc->slave_pixmap->drawable, crtc->slave_damage);
	}

	crtc->shadow = true;
	sna->mode.shadow_active++;
	return true;
}

static void sna_crtc_disable_override(struct sna *sna, struct sna_crtc *crtc)
{
	if (crtc->client_bo == NULL)
		return;

	assert(crtc->client_bo->refcnt >= crtc->client_bo->active_scanout);
	crtc->client_bo->active_scanout--;

	if (!crtc->transform) {
		DrawableRec tmp;

		tmp.width = crtc->base->mode.HDisplay;
		tmp.height = crtc->base->mode.VDisplay;
		tmp.depth = sna->front->drawable.depth;
		tmp.bitsPerPixel = sna->front->drawable.bitsPerPixel;

		sna->render.copy_boxes(sna, GXcopy,
				       &tmp, crtc->client_bo, -crtc->base->bounds.x1, -crtc->base->bounds.y1,
				       &sna->front->drawable, __sna_pixmap_get_bo(sna->front), 0, 0,
				       &crtc->base->bounds, 1, 0);
		list_del(&crtc->shadow_link);
	}
	kgem_bo_destroy(&sna->kgem, crtc->client_bo);
	crtc->client_bo = NULL;
}

static void sna_crtc_disable_shadow(struct sna *sna, struct sna_crtc *crtc)
{
	crtc->fallback_shadow = false;
	if (!crtc->shadow)
		return;

	DBG(("%s: disabling for crtc %d\n", __FUNCTION__, __sna_crtc_id(crtc)));
	assert(sna->mode.shadow_active > 0);

	if (crtc->slave_damage) {
		assert(crtc->slave_pixmap);
		DamageUnregister(&crtc->slave_pixmap->drawable, crtc->slave_damage);
		DamageDestroy(crtc->slave_damage);
		crtc->slave_damage = NULL;
	}

	sna_crtc_disable_override(sna, crtc);

	if (!--sna->mode.shadow_active)
		sna_mode_disable_shadow(sna);

	crtc->shadow = false;
}

static void
__sna_crtc_disable(struct sna *sna, struct sna_crtc *sna_crtc)
{
	sna_crtc->mode_serial++;

	sna_crtc_disable_cursor(sna, sna_crtc);
	rotation_set(sna, &sna_crtc->primary, RR_Rotate_0);
	sna_crtc_disable_shadow(sna, sna_crtc);

	if (sna_crtc->bo) {
		DBG(("%s: releasing handle=%d from scanout, active=%d\n",
		     __FUNCTION__,sna_crtc->bo->handle, sna_crtc->bo->active_scanout-1));
		assert(sna_crtc->public.flags & CRTC_ON);
		assert(sna_crtc->bo->active_scanout);
		assert(sna_crtc->bo->refcnt >= sna_crtc->bo->active_scanout);
		sna_crtc->bo->active_scanout--;
		kgem_bo_destroy(&sna->kgem, sna_crtc->bo);
		sna_crtc->bo = NULL;
		sna_crtc->public.flags &= ~CRTC_ON;

		if (sna->mode.hidden) {
			sna->mode.hidden--;
			assert(sna->mode.hidden);
			assert(sna->mode.front_active == 0);
		} else {
			assert(sna->mode.front_active);
			sna->mode.front_active--;
		}
		sna->mode.dirty = true;
	}

	if (sna_crtc->shadow_bo) {
		kgem_bo_destroy(&sna->kgem, sna_crtc->shadow_bo);
		sna_crtc->shadow_bo = NULL;
	}
	if (sna_crtc->transform) {
		assert(sna->mode.rr_active);
		sna->mode.rr_active--;
		sna_crtc->transform = false;
	}

	sna_crtc->cursor_transform = false;
	sna_crtc->hwcursor = true;
	assert(!sna_crtc->shadow);
}

static void
sna_crtc_disable(xf86CrtcPtr crtc, bool force)
{
	struct sna *sna = to_sna(crtc->scrn);
	struct sna_crtc *sna_crtc = to_sna_crtc(crtc);
	struct drm_mode_crtc arg;

	if (sna_crtc == NULL)
		return;

	if (!force && sna_crtc->bo == NULL)
		return;

	DBG(("%s: disabling crtc [%d, pipe=%d], force?=%d\n", __FUNCTION__,
	     __sna_crtc_id(sna_crtc), __sna_crtc_pipe(sna_crtc), force));

	sna_crtc_force_outputs_off(crtc);

	memset(&arg, 0, sizeof(arg));
	arg.crtc_id = __sna_crtc_id(sna_crtc);
	(void)drmIoctl(sna->kgem.fd, DRM_IOCTL_MODE_SETCRTC, &arg);

	__sna_crtc_disable(sna, sna_crtc);
}

static void update_flush_interval(struct sna *sna)
{
	xf86CrtcConfigPtr config = XF86_CRTC_CONFIG_PTR(sna->scrn);
	int i, max_vrefresh = 0;

	DBG(("%s: front_active=%d\n", __FUNCTION__, sna->mode.front_active));

	for (i = 0; i < sna->mode.num_real_crtc; i++) {
		xf86CrtcPtr crtc = config->crtc[i];

		assert(to_sna_crtc(crtc) != NULL);

		if (!crtc->enabled) {
			DBG(("%s: CRTC:%d (pipe %d) disabled\n",
			     __FUNCTION__,i, sna_crtc_pipe(crtc)));
			assert(to_sna_crtc(crtc)->bo == NULL);
			continue;
		}

		if (to_sna_crtc(crtc)->bo == NULL) {
			DBG(("%s: CRTC:%d (pipe %d) turned off\n",
			     __FUNCTION__,i, sna_crtc_pipe(crtc)));
			continue;
		}

		DBG(("%s: CRTC:%d (pipe %d) vrefresh=%f\n",
		     __FUNCTION__, i, sna_crtc_pipe(crtc),
		     xf86ModeVRefresh(&crtc->mode)));
		max_vrefresh = max(max_vrefresh, xf86ModeVRefresh(&crtc->mode));
	}

	if (max_vrefresh == 0) {
		assert(sna->mode.front_active == 0);
		sna->vblank_interval = 0;
	} else
		sna->vblank_interval = 1000 / max_vrefresh; /* Hz -> ms */

	DBG(("max_vrefresh=%d, vblank_interval=%d ms\n",
	       max_vrefresh, sna->vblank_interval));
}

static struct kgem_bo *sna_create_bo_for_fbcon(struct sna *sna,
					       const struct drm_mode_fb_cmd *fbcon)
{
	struct drm_gem_flink flink;
	struct kgem_bo *bo;
	int ret;

	/* Create a new reference for the fbcon so that we can track it
	 * using a normal bo and so that when we call gem_close on it we
	 * delete our reference and not fbcon's!
	 */
	VG_CLEAR(flink);
	flink.handle = fbcon->handle;
	ret = drmIoctl(sna->kgem.fd, DRM_IOCTL_GEM_FLINK, &flink);
	if (ret)
		return NULL;

	bo = kgem_create_for_name(&sna->kgem, flink.name);
	if (bo == NULL)
		return NULL;

	bo->pitch = fbcon->pitch;
	return bo;
}

/* Copy the current framebuffer contents into the front-buffer for a seamless
 * transition from e.g. plymouth.
 */
void sna_copy_fbcon(struct sna *sna)
{
	xf86CrtcConfigPtr config = XF86_CRTC_CONFIG_PTR(sna->scrn);
	struct drm_mode_fb_cmd fbcon;
	PixmapRec scratch;
	struct sna_pixmap *priv;
	struct kgem_bo *bo;
	BoxRec box;
	bool ok;
	int sx, sy;
	int dx, dy;
	int i;

	if (wedged(sna) || isGPU(sna->scrn))
		return;

	DBG(("%s\n", __FUNCTION__));
	assert((sna->flags & SNA_IS_HOSTED) == 0);

	priv = sna_pixmap_move_to_gpu(sna->front, MOVE_WRITE | __MOVE_SCANOUT);
	if (priv == NULL)
		return;

	/* Scan the connectors for a framebuffer and assume that is the fbcon */
	VG_CLEAR(fbcon);
	fbcon.fb_id = 0;
	for (i = 0; i < sna->mode.num_real_crtc; i++) {
		struct sna_crtc *crtc = to_sna_crtc(config->crtc[i]);
		struct drm_mode_crtc mode;

		assert(crtc != NULL);

		VG_CLEAR(mode);
		mode.crtc_id = __sna_crtc_id(crtc);
		if (drmIoctl(sna->kgem.fd, DRM_IOCTL_MODE_GETCRTC, &mode))
			continue;
		if (!mode.fb_id)
			continue;

		fbcon.fb_id = mode.fb_id;
		if (drmIoctl(sna->kgem.fd, DRM_IOCTL_MODE_GETFB, &fbcon)) {
			fbcon.fb_id = 0;
			continue;
		}
		break;
	}
	if (fbcon.fb_id == 0) {
		DBG(("%s: no fbcon found\n", __FUNCTION__));
		return;
	}

	if (fbcon.fb_id == fb_id(priv->gpu_bo)) {
		DBG(("%s: fb already installed as scanout\n", __FUNCTION__));
		return;
	}

	DBG(("%s: found fbcon, size=%dx%d, depth=%d, bpp=%d\n",
	     __FUNCTION__, fbcon.width, fbcon.height, fbcon.depth, fbcon.bpp));

	bo = sna_create_bo_for_fbcon(sna, &fbcon);
	if (bo == NULL)
		return;

	DBG(("%s: fbcon handle=%d\n", __FUNCTION__, bo->handle));

	scratch.drawable.width = fbcon.width;
	scratch.drawable.height = fbcon.height;
	scratch.drawable.depth = fbcon.depth;
	scratch.drawable.bitsPerPixel = fbcon.bpp;
	scratch.devPrivate.ptr = NULL;

	box.x1 = box.y1 = 0;
	box.x2 = min(fbcon.width, sna->front->drawable.width);
	box.y2 = min(fbcon.height, sna->front->drawable.height);

	sx = dx = 0;
	if (box.x2 < (uint16_t)fbcon.width)
		sx = (fbcon.width - box.x2) / 2;
	if (box.x2 < sna->front->drawable.width)
		dx = (sna->front->drawable.width - box.x2) / 2;

	sy = dy = 0;
	if (box.y2 < (uint16_t)fbcon.height)
		sy = (fbcon.height - box.y2) / 2;
	if (box.y2 < sna->front->drawable.height)
		dy = (sna->front->drawable.height - box.y2) / 2;

	ok = sna->render.copy_boxes(sna, GXcopy,
				    &scratch.drawable, bo, sx, sy,
				    &sna->front->drawable, priv->gpu_bo, dx, dy,
				    &box, 1, 0);
	if (!DAMAGE_IS_ALL(priv->gpu_damage))
		sna_damage_add_box(&priv->gpu_damage, &box);

	kgem_bo_destroy(&sna->kgem, bo);

#if ABI_VIDEODRV_VERSION >= SET_ABI_VERSION(10, 0)
	to_screen_from_sna(sna)->canDoBGNoneRoot = ok;
#endif
}

static bool use_shadow(struct sna *sna, xf86CrtcPtr crtc)
{
	RRTransformPtr transform;
	PictTransform crtc_to_fb;
	struct pict_f_transform f_crtc_to_fb, f_fb_to_crtc;
	unsigned pitch_limit;
	BoxRec b;

	assert(sna->scrn->virtualX && sna->scrn->virtualY);

	if (sna->flags & SNA_FORCE_SHADOW) {
		DBG(("%s: forcing shadow\n", __FUNCTION__));
		return true;
	}

	if (to_sna_crtc(crtc)->fallback_shadow) {
		DBG(("%s: fallback shadow\n", __FUNCTION__));
		return true;
	}

	if (sna->flags & SNA_TEAR_FREE && to_sna_crtc(crtc)->slave_pixmap) {
		DBG(("%s: TearFree shadow required\n", __FUNCTION__));
		return true;
	}

	if (sna->scrn->virtualX > sna->mode.max_crtc_width ||
	    sna->scrn->virtualY > sna->mode.max_crtc_height) {
		DBG(("%s: framebuffer too large (%dx%d) > (%dx%d)\n",
		    __FUNCTION__,
		    sna->scrn->virtualX, sna->scrn->virtualY,
		    sna->mode.max_crtc_width, sna->mode.max_crtc_height));
		return true;
	}

	if (!isGPU(sna->scrn)) {
		struct sna_pixmap *priv;

		priv = sna_pixmap_force_to_gpu(sna->front, MOVE_READ | __MOVE_SCANOUT);
		if (priv == NULL)
			return true; /* maybe we can create a bo for the scanout? */

		if (sna->kgem.gen == 071)
			pitch_limit = priv->gpu_bo->tiling ? 16 * 1024 : 32 * 1024;
		else if ((sna->kgem.gen >> 3) > 4)
			pitch_limit = 32 * 1024;
		else if ((sna->kgem.gen >> 3) == 4)
			pitch_limit = priv->gpu_bo->tiling ? 16 * 1024 : 32 * 1024;
		else if ((sna->kgem.gen >> 3) == 3)
			pitch_limit = priv->gpu_bo->tiling ? 8 * 1024 : 16 * 1024;
		else
			pitch_limit = 8 * 1024;
		DBG(("%s: gpu bo handle=%d tiling=%d pitch=%d, limit=%d\n", __FUNCTION__, priv->gpu_bo->handle, priv->gpu_bo->tiling, priv->gpu_bo->pitch, pitch_limit));
		if (priv->gpu_bo->pitch > pitch_limit)
			return true;

		if (priv->gpu_bo->tiling && sna->flags & SNA_LINEAR_FB) {
			DBG(("%s: gpu bo is tiled, need linear, forcing shadow\n", __FUNCTION__));
			return true;
		}
	}

	transform = NULL;
	if (crtc->transformPresent)
		transform = &crtc->transform;
	if (RRTransformCompute(crtc->x, crtc->y,
			       crtc->mode.HDisplay, crtc->mode.VDisplay,
			       crtc->rotation, transform,
			       &crtc_to_fb,
			       &f_crtc_to_fb,
			       &f_fb_to_crtc)) {
		bool needs_transform = true;
		unsigned rotation = rotation_reduce(&to_sna_crtc(crtc)->primary, crtc->rotation);
		DBG(("%s: natively supported rotation? rotation=%x & supported=%x == %d\n",
		     __FUNCTION__, rotation, to_sna_crtc(crtc)->primary.rotation.supported,
		     rotation == (rotation & to_sna_crtc(crtc)->primary.rotation.supported)));
		if ((to_sna_crtc(crtc)->primary.rotation.supported & rotation) == rotation)
			needs_transform = RRTransformCompute(crtc->x, crtc->y,
							     crtc->mode.HDisplay, crtc->mode.VDisplay,
							     RR_Rotate_0, transform,
							     NULL, NULL, NULL);
		if (needs_transform) {
			DBG(("%s: RandR transform present\n", __FUNCTION__));
			return true;
		}
	}

	/* And finally check that it is entirely visible */
	b.x1 = b.y1 = 0;
	b.x2 = crtc->mode.HDisplay;
	b.y2 = crtc->mode.VDisplay;
	pixman_f_transform_bounds(&f_crtc_to_fb, &b);
	DBG(("%s? bounds (%d, %d), (%d, %d), framebufer %dx%d\n",
	     __FUNCTION__, b.x1, b.y1, b.x2, b.y2,
		 sna->scrn->virtualX, sna->scrn->virtualY));

	if  (b.x1 < 0 || b.y1 < 0 ||
	     b.x2 > sna->scrn->virtualX ||
	     b.y2 > sna->scrn->virtualY) {
		DBG(("%s: scanout is partly outside the framebuffer\n",
		     __FUNCTION__));
		return true;
	}

	return false;
}

static void set_shadow(struct sna *sna, RegionPtr region)
{
	struct sna_pixmap *priv = sna_pixmap(sna->front);

	assert(priv->gpu_bo);
	assert(sna->mode.shadow);
	assert(sna->mode.shadow->active_scanout);

	DBG(("%s: waiting for region %dx[(%d, %d), (%d, %d)], front handle=%d, shadow handle=%d\n",
	     __FUNCTION__,
	     region_num_rects(region),
	     region->extents.x1, region->extents.y1,
	     region->extents.x2, region->extents.y2,
	     priv->gpu_bo->handle, sna->mode.shadow->handle));

	assert(priv->pinned & PIN_SCANOUT);
	assert((priv->pinned & PIN_PRIME) == 0);
	assert(sna->mode.shadow != priv->gpu_bo);

	RegionCopy(&sna->mode.shadow_region, region);

	priv->move_to_gpu = wait_for_shadow;
	priv->move_to_gpu_data = sna;
}

static struct kgem_bo *
get_scanout_bo(struct sna *sna, PixmapPtr pixmap)
{
	struct sna_pixmap *priv;

	priv = sna_pixmap_force_to_gpu(pixmap, MOVE_READ | __MOVE_SCANOUT);
	if (!priv)
		return NULL;

	if (priv->gpu_bo->pitch & 63) {
		struct kgem_bo *tmp;
		BoxRec b;

		DBG(("%s: converting to scanout bo due to bad pitch [%d]\n",
		     __FUNCTION__, priv->gpu_bo->pitch));

		if (priv->pinned) {
			DBG(("%s: failed as the Pixmap is already pinned [%x]\n",
			     __FUNCTION__, priv->pinned));
			return NULL;
		}

		tmp = kgem_create_2d(&sna->kgem,
				     pixmap->drawable.width,
				     pixmap->drawable.height,
				     sna->scrn->bitsPerPixel,
				     priv->gpu_bo->tiling,
				     CREATE_EXACT | CREATE_SCANOUT);
		if (tmp == NULL) {
			DBG(("%s: allocation failed\n", __FUNCTION__));
			return NULL;
		}

		b.x1 = 0;
		b.y1 = 0;
		b.x2 = pixmap->drawable.width;
		b.y2 = pixmap->drawable.height;

		if (sna->render.copy_boxes(sna, GXcopy,
					   &pixmap->drawable, priv->gpu_bo, 0, 0,
					   &pixmap->drawable, tmp, 0, 0,
					   &b, 1, COPY_LAST)) {
			DBG(("%s: copy failed\n", __FUNCTION__));
			kgem_bo_destroy(&sna->kgem, tmp);
			return NULL;
		}

		kgem_bo_destroy(&sna->kgem, priv->gpu_bo);
		priv->gpu_bo = tmp;
	}

	priv->pinned |= PIN_SCANOUT;
	return priv->gpu_bo;
}

static void shadow_clear(struct sna *sna,
			 PixmapPtr front, struct kgem_bo *bo,
			 xf86CrtcPtr crtc)
{
	bool ok = false;
	if (!wedged(sna))
		ok = sna->render.fill_one(sna, front, bo, 0,
					  0, 0, crtc->mode.HDisplay, crtc->mode.VDisplay,
					  GXclear);
	if (!ok) {
		void *ptr = kgem_bo_map__gtt(&sna->kgem, bo);
		if (ptr)
			memset(ptr, 0, bo->pitch * crtc->mode.VDisplay);
	}
	sna->mode.shadow_dirty = true;
}

static bool rr_active(xf86CrtcPtr crtc)
{
	return crtc->transformPresent || crtc->rotation != RR_Rotate_0;
}

static struct kgem_bo *sna_crtc_attach(xf86CrtcPtr crtc)
{
	struct sna_crtc *sna_crtc = to_sna_crtc(crtc);
	ScrnInfoPtr scrn = crtc->scrn;
	struct sna *sna = to_sna(scrn);
	struct kgem_bo *bo;

	if (sna_crtc->transform) {
		assert(sna->mode.rr_active);
		sna_crtc->transform = false;
		sna->mode.rr_active--;
	}
	sna_crtc->rotation = RR_Rotate_0;

	if (sna_crtc->cache_bo) {
		kgem_bo_destroy(&sna->kgem, sna_crtc->cache_bo);
		sna_crtc->cache_bo = NULL;
	}

	if (use_shadow(sna, crtc)) {
		PixmapPtr front;
		unsigned long tiled_limit;
		int tiling;

force_shadow:
		if (!sna_crtc_enable_shadow(sna, sna_crtc)) {
			DBG(("%s: failed to enable crtc shadow\n", __FUNCTION__));
			return NULL;
		}

		DBG(("%s: attaching to per-crtc pixmap %dx%d\n",
		     __FUNCTION__, crtc->mode.HDisplay, crtc->mode.VDisplay));

		bo = sna_crtc->shadow_bo;
		if (bo) {
			if (sna_crtc->shadow_bo_width == crtc->mode.HDisplay &&
			    sna_crtc->shadow_bo_height == crtc->mode.VDisplay) {
				DBG(("%s: reusing current shadow bo handle=%d\n",
				     __FUNCTION__, bo->handle));
				goto out_shadow;
			}

			kgem_bo_destroy(&sna->kgem, bo);
			sna_crtc->shadow_bo = NULL;
		}

		tiling = I915_TILING_X;
		if (crtc->rotation & (RR_Rotate_90 | RR_Rotate_270) &&
		    sna->kgem.can_scanout_y)
			tiling = I915_TILING_Y;

		if (sna->kgem.gen == 071)
			tiled_limit = 16 * 1024 * 8;
		else if ((sna->kgem.gen >> 3) > 4)
			tiled_limit = 32 * 1024 * 8;
		else if ((sna->kgem.gen >> 3) == 4)
			tiled_limit = 16 * 1024 * 8;
		else
			tiled_limit = 8 * 1024 * 8;
		if ((unsigned long)crtc->mode.HDisplay * scrn->bitsPerPixel > tiled_limit)
			tiling = I915_TILING_NONE;
		if (sna->flags & SNA_LINEAR_FB)
			tiling = I915_TILING_NONE;

		bo = kgem_create_2d(&sna->kgem,
				    crtc->mode.HDisplay, crtc->mode.VDisplay,
				    scrn->bitsPerPixel,
				    tiling, CREATE_SCANOUT);
		if (bo == NULL) {
			DBG(("%s: failed to allocate crtc scanout\n", __FUNCTION__));
			return NULL;
		}

		if (!get_fb(sna, bo, crtc->mode.HDisplay, crtc->mode.VDisplay)) {
			DBG(("%s: failed to bind fb for crtc scanout\n", __FUNCTION__));
			kgem_bo_destroy(&sna->kgem, bo);
			return NULL;
		}

		front = sna_crtc->slave_pixmap ?: sna->front;
		if (__sna_pixmap_get_bo(front) && !rr_active(crtc)) {
			BoxRec b;

			b.x1 = crtc->x;
			b.y1 = crtc->y;
			b.x2 = crtc->x + crtc->mode.HDisplay;
			b.y2 = crtc->y + crtc->mode.VDisplay;

			if (b.x1 < 0)
				b.x1 = 0;
			if (b.y1 < 0)
				b.y1 = 0;
			if (b.x2 > scrn->virtualX)
				b.x2 = scrn->virtualX;
			if (b.y2 > scrn->virtualY)
				b.y2 = scrn->virtualY;
			if (b.x2 - b.x1 < crtc->mode.HDisplay ||
			    b.y2 - b.y1 < crtc->mode.VDisplay)
				shadow_clear(sna, front, bo, crtc);

			if (b.y2 > b.y1 && b.x2 > b.x1) {
				DrawableRec tmp;

				DBG(("%s: copying onto shadow CRTC: (%d, %d)x(%d, %d) [fb=%dx%d], handle=%d\n",
				     __FUNCTION__,
				     b.x1, b.y1,
				     b.x2-b.x1, b.y2-b.y1,
				     scrn->virtualX, scrn->virtualY,
				     bo->handle));

				tmp.width = crtc->mode.HDisplay;
				tmp.height = crtc->mode.VDisplay;
				tmp.depth = front->drawable.depth;
				tmp.bitsPerPixel = front->drawable.bitsPerPixel;

				if (!sna->render.copy_boxes(sna, GXcopy,
							     &front->drawable, __sna_pixmap_get_bo(front), 0, 0,
							     &tmp, bo, -crtc->x, -crtc->y,
							     &b, 1, COPY_LAST))
					shadow_clear(sna, front, bo, crtc);
			}
		} else
			shadow_clear(sna, front, bo, crtc);

		sna_crtc->shadow_bo_width = crtc->mode.HDisplay;
		sna_crtc->shadow_bo_height = crtc->mode.VDisplay;
		sna_crtc->shadow_bo = bo;
out_shadow:
		sna_crtc->transform = true;
		sna->mode.rr_active++;
		return kgem_bo_reference(bo);
	} else {
		if (sna_crtc->shadow_bo) {
			kgem_bo_destroy(&sna->kgem, sna_crtc->shadow_bo);
			sna_crtc->shadow_bo = NULL;
		}

		if (sna_crtc->slave_pixmap) {
			DBG(("%s: attaching to scanout pixmap\n", __FUNCTION__));
			bo = get_scanout_bo(sna, sna_crtc->slave_pixmap);
			if (bo == NULL) {
				DBG(("%s: failed to pin crtc scanout\n", __FUNCTION__));
				sna_crtc->fallback_shadow = true;
				goto force_shadow;
			}

			if (!get_fb(sna, bo,
				    sna_crtc->slave_pixmap->drawable.width,
				    sna_crtc->slave_pixmap->drawable.height)) {
				DBG(("%s: failed to bind fb for crtc scanout\n", __FUNCTION__));
				sna_crtc->fallback_shadow = true;
				goto force_shadow;
			}
		} else {
			DBG(("%s: attaching to framebuffer\n", __FUNCTION__));
			bo = get_scanout_bo(sna, sna->front);
			if (bo == NULL) {
				DBG(("%s: failed to pin framebuffer\n", __FUNCTION__));
				sna_crtc->fallback_shadow = true;
				goto force_shadow;
			}

			if (!get_fb(sna, bo, scrn->virtualX, scrn->virtualY)) {
				DBG(("%s: failed to bind fb for crtc scanout\n", __FUNCTION__));
				sna_crtc->fallback_shadow = true;
				goto force_shadow;
			}
		}

		if (sna->flags & SNA_TEAR_FREE) {
			RegionRec region;

			assert(sna_crtc->slave_pixmap == NULL);

			DBG(("%s: enabling TearFree shadow\n", __FUNCTION__));
			region.extents.x1 = 0;
			region.extents.y1 = 0;
			region.extents.x2 = sna->scrn->virtualX;
			region.extents.y2 = sna->scrn->virtualY;
			region.data = NULL;

			if (!sna_crtc_enable_shadow(sna, sna_crtc)) {
				DBG(("%s: failed to enable crtc shadow\n", __FUNCTION__));
				return NULL;
			}

			if (sna->mode.shadow == NULL) {
				struct kgem_bo *shadow;

				DBG(("%s: creating TearFree shadow bo\n", __FUNCTION__));
				shadow = kgem_create_2d(&sna->kgem,
							region.extents.x2,
							region.extents.y2,
							scrn->bitsPerPixel,
							kgem_choose_tiling(&sna->kgem,
									   I915_TILING_X,
									   region.extents.x2,
									   region.extents.y2,
									   sna->scrn->bitsPerPixel),
							CREATE_SCANOUT);
				if (shadow == NULL) {
					DBG(("%s: failed to allocate TearFree shadow bo\n", __FUNCTION__));
					sna_crtc->fallback_shadow = true;
					goto force_shadow;
				}

				if (!get_fb(sna, shadow,
					    region.extents.x2,
					    region.extents.y2)) {
					DBG(("%s: failed to bind fb for TearFeee shadow\n", __FUNCTION__));
					kgem_bo_destroy(&sna->kgem, shadow);
					sna_crtc->fallback_shadow = true;
					goto force_shadow;
				}

				assert(__sna_pixmap_get_bo(sna->front) == NULL ||
				       __sna_pixmap_get_bo(sna->front)->pitch == shadow->pitch);
				sna->mode.shadow = shadow;
				sna->mode.shadow->active_scanout++;
			}
			set_shadow(sna, &region);

			sna_crtc_disable_override(sna, sna_crtc);
		} else
			sna_crtc_disable_shadow(sna, sna_crtc);

		sna_crtc->rotation = rotation_reduce(&sna_crtc->primary, crtc->rotation);
		assert(sna_crtc->primary.rotation.supported & sna_crtc->rotation);
		return kgem_bo_reference(bo);
	}
}

#define SCALING_EPSILON (1./256)

static bool
is_affine(const struct pixman_f_transform *t)
{
	return (fabs(t->m[2][0]) < SCALING_EPSILON &&
		fabs(t->m[2][1]) < SCALING_EPSILON);
}

static double determinant(const struct pixman_f_transform *t)
{
	return t->m[0][0]*t->m[1][1] - t->m[1][0]*t->m[0][1];
}

static bool
affine_is_pixel_exact(const struct pixman_f_transform *t)
{
	double det = t->m[2][2] * determinant(t);
	if (fabs (det * det - 1.0) < SCALING_EPSILON) {
		if (fabs(t->m[0][1]) < SCALING_EPSILON &&
		    fabs(t->m[1][0]) < SCALING_EPSILON)
			return true;

		if (fabs(t->m[0][0]) < SCALING_EPSILON &&
		    fabs(t->m[1][1]) < SCALING_EPSILON)
			return true;
	}

	return false;
}

static void sna_crtc_randr(xf86CrtcPtr crtc)
{
	struct sna_crtc *sna_crtc = to_sna_crtc(crtc);
	struct pict_f_transform f_crtc_to_fb, f_fb_to_crtc;
	PictTransform crtc_to_fb;
	PictFilterPtr filter;
	xFixed *params;
	int nparams;
	RRTransformPtr transform;
	int needs_transform;

	transform = NULL;
	if (crtc->transformPresent)
		transform = &crtc->transform;

	needs_transform =
		RRTransformCompute(crtc->x, crtc->y,
				   crtc->mode.HDisplay, crtc->mode.VDisplay,
				   crtc->rotation, transform,
				   &crtc_to_fb,
				   &f_crtc_to_fb,
				   &f_fb_to_crtc);

	filter = NULL;
	params = NULL;
	nparams = 0;
	if (sna_crtc->transform) {
#ifdef RANDR_12_INTERFACE
		if (transform) {
			if (transform->nparams) {
				params = malloc(transform->nparams * sizeof(xFixed));
				if (params) {
					memcpy(params, transform->params,
					       transform->nparams * sizeof(xFixed));
					nparams = transform->nparams;
					filter = transform->filter;
				}
			} else
				filter = transform->filter;
		}
#endif
		crtc->transform_in_use = needs_transform;
	} else
		crtc->transform_in_use = sna_crtc->rotation != RR_Rotate_0;

	/* Recompute the cursor after a potential change in transform */
	if (sna_crtc->cursor) {
		assert(sna_crtc->cursor->ref > 0);
		sna_crtc->cursor->ref--;
		sna_crtc->cursor = NULL;
	}

	if (needs_transform) {
		sna_crtc->hwcursor = is_affine(&f_fb_to_crtc);
		sna_crtc->cursor_transform =
			sna_crtc->hwcursor &&
			!affine_is_pixel_exact(&f_fb_to_crtc);
	} else {
		sna_crtc->hwcursor = true;
		sna_crtc->cursor_transform = false;
	}
	DBG(("%s: hwcursor?=%d, cursor_transform?=%d\n",
	     __FUNCTION__, sna_crtc->hwcursor, sna_crtc->cursor_transform));

	crtc->crtc_to_framebuffer = crtc_to_fb;
	crtc->f_crtc_to_framebuffer = f_crtc_to_fb;
	crtc->f_framebuffer_to_crtc = f_fb_to_crtc;

	free(crtc->params);
	crtc->params  = params;
	crtc->nparams = nparams;

	crtc->filter = filter;
	if (filter) {
		crtc->filter_width  = filter->width;
		crtc->filter_height = filter->height;
	} else {
		crtc->filter_width  = 0;
		crtc->filter_height = 0;
	}

	crtc->bounds.x1 = 0;
	crtc->bounds.x2 = crtc->mode.HDisplay;
	crtc->bounds.y1 = 0;
	crtc->bounds.y2 = crtc->mode.VDisplay;
	pixman_f_transform_bounds(&f_crtc_to_fb, &crtc->bounds);

	DBG(("%s: transform? %d, bounds (%d, %d), (%d, %d)\n",
	     __FUNCTION__, crtc->transform_in_use,
	     crtc->bounds.x1, crtc->bounds.y1,
	     crtc->bounds.x2, crtc->bounds.y2));
}

static void
sna_crtc_damage(xf86CrtcPtr crtc)
{
	ScreenPtr screen = xf86ScrnToScreen(crtc->scrn);
	struct sna *sna = to_sna(crtc->scrn);
	RegionRec region, *damage;

	region.extents = crtc->bounds;
	region.data = NULL;

	if (region.extents.x1 < 0)
		region.extents.x1 = 0;
	if (region.extents.y1 < 0)
		region.extents.y1 = 0;
	if (region.extents.x2 > screen->width)
		region.extents.x2 = screen->width;
	if (region.extents.y2 > screen->height)
		region.extents.y2 = screen->height;

	if (region.extents.x2 <= region.extents.x1 ||
	    region.extents.y2 <= region.extents.y1) {
		DBG(("%s: crtc not damaged, all-clipped\n", __FUNCTION__));
		return;
	}

	DBG(("%s: marking crtc %d as completely damaged (%d, %d), (%d, %d)\n",
	     __FUNCTION__, sna_crtc_id(crtc),
	     region.extents.x1, region.extents.y1,
	     region.extents.x2, region.extents.y2));

	assert(sna->mode.shadow_damage && sna->mode.shadow_active);
	damage = DamageRegion(sna->mode.shadow_damage);
	RegionUnion(damage, damage, &region);
	to_sna_crtc(crtc)->crtc_damage = region;

	DBG(("%s: damage now %dx[(%d, %d), (%d, %d)]\n",
	     __FUNCTION__,
	     region_num_rects(damage),
	     damage->extents.x1, damage->extents.y1,
	     damage->extents.x2, damage->extents.y2));
}

static char *outputs_for_crtc(xf86CrtcPtr crtc, char *outputs, int max)
{
	struct sna *sna = to_sna(crtc->scrn);
	xf86CrtcConfigPtr config = XF86_CRTC_CONFIG_PTR(crtc->scrn);
	int len, i;

	for (i = len = 0; i < sna->mode.num_real_output; i++) {
		xf86OutputPtr output = config->output[i];

		if (output->crtc != crtc)
			continue;

		len += snprintf(outputs+len, max-len, "%s, ", output->name);
	}
	assert(len >= 2);
	outputs[len-2] = '\0';

	return outputs;
}

static const char *rotation_to_str(Rotation rotation)
{
	switch (rotation & RR_Rotate_All) {
	case 0:
	case RR_Rotate_0: return "normal";
	case RR_Rotate_90: return "left";
	case RR_Rotate_180: return "inverted";
	case RR_Rotate_270: return "right";
	default: return "unknown";
	}
}

static const char *reflection_to_str(Rotation rotation)
{
	switch (rotation & RR_Reflect_All) {
	case 0: return "none";
	case RR_Reflect_X: return "X axis";
	case RR_Reflect_Y: return "Y axis";
	case RR_Reflect_X | RR_Reflect_Y: return "X and Y axes";
	default: return "invalid";
	}
}

static void reprobe_connectors(xf86CrtcPtr crtc)
{
	xf86CrtcConfigPtr config = XF86_CRTC_CONFIG_PTR(crtc->scrn);
	struct sna *sna = to_sna(crtc->scrn);
	int i;

	for (i = 0; i < sna->mode.num_real_output; i++) {
		xf86OutputPtr output = config->output[i];
		if (output->crtc == crtc)
			to_sna_output(output)->reprobe = true;
	}

	sna_mode_discover(sna, true);
}

static Bool
__sna_crtc_set_mode(xf86CrtcPtr crtc)
{
	struct sna *sna = to_sna(crtc->scrn);
	struct sna_crtc *sna_crtc = to_sna_crtc(crtc);
	struct kgem_bo *saved_bo, *bo;
	uint32_t saved_offset;
	bool saved_transform;
	bool saved_hwcursor;
	bool saved_cursor_transform;
	int ret;

	DBG(("%s: CRTC=%d, pipe=%d, hidden?=%d\n", __FUNCTION__,
	     __sna_crtc_id(sna_crtc), __sna_crtc_pipe(sna_crtc), sna->mode.hidden));
	if (sna->mode.hidden)
		return TRUE;

	saved_bo = sna_crtc->bo;
	saved_transform = sna_crtc->transform;
	saved_cursor_transform = sna_crtc->cursor_transform;
	saved_hwcursor = sna_crtc->hwcursor;
	saved_offset = sna_crtc->offset;

	sna_crtc->fallback_shadow = false;
retry: /* Attach per-crtc pixmap or direct */
	bo = sna_crtc_attach(crtc);
	if (bo == NULL) {
		xf86DrvMsg(crtc->scrn->scrnIndex, X_ERROR,
			   "unable to attach scanout\n");
		goto error;
	}

	/* Prevent recursion when enabling outputs during execbuffer */
	if (bo->exec && RQ(bo->rq)->bo == NULL) {
		_kgem_submit(&sna->kgem);
		__kgem_bo_clear_dirty(bo);
	}

	sna_crtc->bo = bo;
	ret = sna_crtc_apply(crtc);
	if (ret) {
		kgem_bo_destroy(&sna->kgem, bo);

		if (!sna_crtc->fallback_shadow) {
			sna_crtc->fallback_shadow = true;
			goto retry;
		}

		xf86DrvMsg(crtc->scrn->scrnIndex, X_ERROR,
			   "failed to set mode: %s [%d]\n", strerror(ret), ret);
		goto error;
	}

	sna_crtc->public.flags |= CRTC_ON;
	bo->active_scanout++;
	DBG(("%s: marking handle=%d as active=%d (removing %d from scanout, active=%d)\n",
	     __FUNCTION__, bo->handle, bo->active_scanout,
	     saved_bo ? saved_bo->handle : 0, saved_bo ? saved_bo->active_scanout - 1: -1));
	if (saved_bo) {
		assert(saved_bo->active_scanout);
		assert(saved_bo->refcnt >= saved_bo->active_scanout);
		saved_bo->active_scanout--;
		kgem_bo_destroy(&sna->kgem, saved_bo);
	}

	sna_crtc_randr(crtc);
	if (sna_crtc->transform)
		sna_crtc_damage(crtc);
	if (sna_crtc->cursor &&  /* Reload cursor if RandR maybe changed */
	    (!sna_crtc->hwcursor ||
	     saved_cursor_transform || sna_crtc->cursor_transform ||
	     sna_crtc->cursor->rotation != crtc->rotation))
		sna_crtc_disable_cursor(sna, sna_crtc);

	assert(!sna->mode.hidden);
	sna->mode.front_active += saved_bo == NULL;
	sna->mode.dirty = true;
	DBG(("%s: handle=%d, scanout_active=%d, front_active=%d\n",
	     __FUNCTION__, bo->handle, bo->active_scanout, sna->mode.front_active));

	return TRUE;

error:
	sna_crtc->offset = saved_offset;
	if (sna_crtc->transform) {
		assert(sna->mode.rr_active);
		sna->mode.rr_active--;
	}
	if (saved_transform)
		sna->mode.rr_active++;
	sna_crtc->transform = saved_transform;
	sna_crtc->cursor_transform = saved_cursor_transform;
	sna_crtc->hwcursor = saved_hwcursor;
	sna_crtc->bo = saved_bo;

	reprobe_connectors(crtc);
	return FALSE;
}

static Bool
sna_crtc_set_mode_major(xf86CrtcPtr crtc, DisplayModePtr mode,
			Rotation rotation, int x, int y)
{
	struct sna *sna = to_sna(crtc->scrn);
	struct sna_crtc *sna_crtc = to_sna_crtc(crtc);
	struct drm_mode_modeinfo saved_kmode;
	char outputs[256];

	if (mode->HDisplay == 0 || mode->VDisplay == 0)
		return FALSE;

	assert(sna_crtc);

	xf86DrvMsg(crtc->scrn->scrnIndex, X_INFO,
		   "switch to mode %dx%d@%.1f on %s using pipe %d, position (%d, %d), rotation %s, reflection %s\n",
		   mode->HDisplay, mode->VDisplay, xf86ModeVRefresh(mode),
		   outputs_for_crtc(crtc, outputs, sizeof(outputs)), __sna_crtc_pipe(sna_crtc),
		   x, y, rotation_to_str(rotation), reflection_to_str(rotation));

	assert(mode->HDisplay <= sna->mode.max_crtc_width &&
	       mode->VDisplay <= sna->mode.max_crtc_height);

#if HAS_GAMMA
	drmModeCrtcSetGamma(sna->kgem.fd, __sna_crtc_id(sna_crtc),
			    crtc->gamma_size,
			    crtc->gamma_red,
			    crtc->gamma_green,
			    crtc->gamma_blue);
#endif

	saved_kmode = sna_crtc->kmode;
	mode_to_kmode(&sna_crtc->kmode, mode);
	if (__sna_crtc_set_mode(crtc))
		return TRUE;

	sna_crtc->kmode = saved_kmode;
	return FALSE;
}

static void
sna_crtc_dpms(xf86CrtcPtr crtc, int mode)
{
	DBG(("%s(pipe %d, dpms mode -> %d):= active=%d\n",
	     __FUNCTION__, sna_crtc_pipe(crtc), mode, mode == DPMSModeOn));

	if (mode == DPMSModeOn && crtc->enabled) {
		if (__sna_crtc_set_mode(crtc))
			update_flush_interval(to_sna(crtc->scrn));
		else
			mode = DPMSModeOff;
	}

	if (mode != DPMSModeOn)
		sna_crtc_disable(crtc, false);
}

void sna_mode_adjust_frame(struct sna *sna, int x, int y)
{
	xf86CrtcConfigPtr config = XF86_CRTC_CONFIG_PTR(sna->scrn);
	xf86CrtcPtr crtc;
	int saved_x, saved_y;

	if ((unsigned)config->compat_output >= config->num_output)
		return;

	crtc = config->output[config->compat_output]->crtc;
	if (crtc == NULL || !crtc->enabled)
		return;

	if (crtc->x == x && crtc->y == y)
		return;

	saved_x = crtc->x;
	saved_y = crtc->y;

	crtc->x = x;
	crtc->y = y;
	if (to_sna_crtc(crtc) && !__sna_crtc_set_mode(crtc)) {
		crtc->x = saved_x;
		crtc->y = saved_y;
	}
}

static void
sna_crtc_gamma_set(xf86CrtcPtr crtc,
		       CARD16 *red, CARD16 *green, CARD16 *blue, int size)
{
	assert(to_sna_crtc(crtc));
	drmModeCrtcSetGamma(to_sna(crtc->scrn)->kgem.fd,
			    sna_crtc_id(crtc),
			    size, red, green, blue);
}

static void
sna_crtc_destroy(xf86CrtcPtr crtc)
{
	struct sna_crtc *sna_crtc = to_sna_crtc(crtc);
	struct plane *sprite, *sn;

	if (sna_crtc == NULL)
		return;

	list_for_each_entry_safe(sprite, sn, &sna_crtc->sprites, link)
		free(sprite);

	free(sna_crtc);
	crtc->driver_private = NULL;
}

#if HAS_PIXMAP_SHARING
static Bool
sna_crtc_set_scanout_pixmap(xf86CrtcPtr crtc, PixmapPtr pixmap)
{
	struct sna_crtc *sna_crtc = to_sna_crtc(crtc);

	if (sna_crtc == NULL)
		return FALSE;

	if (pixmap == sna_crtc->slave_pixmap)
		return TRUE;

	DBG(("%s: CRTC:%d, pipe=%d setting scanout pixmap=%ld\n",
	     __FUNCTION__, __sna_crtc_id(sna_crtc),  __sna_crtc_pipe(sna_crtc),
	     pixmap ? pixmap->drawable.serialNumber : 0));

	/* Disable first so that we can unregister the damage tracking */
	sna_crtc_disable_shadow(to_sna(crtc->scrn), sna_crtc);

	sna_crtc->slave_pixmap = pixmap;

	return TRUE;
}
#endif

static const xf86CrtcFuncsRec sna_crtc_funcs = {
#if XF86_CRTC_VERSION >= 1
	.dpms = sna_crtc_dpms,
#endif
	.set_mode_major = sna_crtc_set_mode_major,
	.gamma_set = sna_crtc_gamma_set,
	.destroy = sna_crtc_destroy,
#if HAS_PIXMAP_SHARING
	.set_scanout_pixmap = sna_crtc_set_scanout_pixmap,
#endif
};

inline static bool prop_has_type_and_name(const struct drm_mode_get_property *prop,
					  unsigned int type, const char *name)
{
	if ((prop->flags & (1 << type)) == 0)
		return false;

	if (strcmp(prop->name, name))
		return false;

	return true;
}

inline static bool prop_is_rotation(const struct drm_mode_get_property *prop)
{
	return prop_has_type_and_name(prop, 5, "rotation");
}

static void parse_rotation_prop(struct sna *sna, struct plane *p,
				struct drm_mode_get_property *prop,
				uint64_t value)
{
	struct drm_mode_property_enum *enums;
	int j;

	p->rotation.prop = prop->prop_id;
	p->rotation.current = value;

	DBG(("%s: found rotation property .id=%d, value=%ld, num_enums=%d\n",
	     __FUNCTION__, prop->prop_id, value, prop->count_enum_blobs));

	enums = malloc(prop->count_enum_blobs * sizeof(struct drm_mode_property_enum));
	if (!enums)
		return;

	prop->count_values = 0;
	prop->enum_blob_ptr = (uintptr_t)enums;

	if (drmIoctl(sna->kgem.fd, DRM_IOCTL_MODE_GETPROPERTY, prop)) {
		free(enums);
		return;
	}

	/* XXX we assume that the mapping between kernel enum and
	 * RandR remains fixed for our lifetimes.
	 */
	VG(VALGRIND_MAKE_MEM_DEFINED(enums, sizeof(*enums)*prop->count_enum_blobs));
	for (j = 0; j < prop->count_enum_blobs; j++) {
		DBG(("%s: rotation[%d] = %s [%lx]\n", __FUNCTION__,
		     j, enums[j].name, (long)enums[j].value));
		p->rotation.supported |= 1 << enums[j].value;
	}

	free(enums);
}

inline static bool prop_is_color_encoding(const struct drm_mode_get_property *prop)
{
	return prop_has_type_and_name(prop, 3, "COLOR_ENCODING");
}

static void parse_color_encoding_prop(struct sna *sna, struct plane *p,
				      struct drm_mode_get_property *prop,
				      uint64_t value)
{
	struct drm_mode_property_enum *enums;
	unsigned int supported = 0;
	int j;

	DBG(("%s: found color encoding property .id=%d, value=%ld, num_enums=%d\n",
	     __FUNCTION__, prop->prop_id, (long)value, prop->count_enum_blobs));

	enums = malloc(prop->count_enum_blobs * sizeof(struct drm_mode_property_enum));
	if (!enums)
		return;

	prop->count_values = 0;
	prop->enum_blob_ptr = (uintptr_t)enums;

	if (drmIoctl(sna->kgem.fd, DRM_IOCTL_MODE_GETPROPERTY, prop)) {
		free(enums);
		return;
	}

	VG(VALGRIND_MAKE_MEM_DEFINED(enums, sizeof(*enums)*prop->count_enum_blobs));
	for (j = 0; j < prop->count_enum_blobs; j++) {
		if (!strcmp(enums[j].name, "ITU-R BT.601 YCbCr")) {
			p->color_encoding.values[0] = enums[j].value;
			supported |= 1 << 0;
		} else if (!strcmp(enums[j].name, "ITU-R BT.709 YCbCr")) {
			p->color_encoding.values[1] = enums[j].value;
			supported |= 1 << 1;
		}
	}

	free(enums);

	if (supported == 3)
		p->color_encoding.prop = prop->prop_id;
}

void sna_crtc_set_sprite_colorspace(xf86CrtcPtr crtc,
				    unsigned idx, int colorspace)
{
	struct plane *p;

	assert(to_sna_crtc(crtc));
	assert(colorspace < ARRAY_SIZE(p->color_encoding.values));

	p = lookup_sprite(to_sna_crtc(crtc), idx);

	if (!p->color_encoding.prop)
		return;

	drmModeObjectSetProperty(to_sna(crtc->scrn)->kgem.fd,
				 p->id, DRM_MODE_OBJECT_PLANE,
				 p->color_encoding.prop,
				 p->color_encoding.values[colorspace]);
}

static int plane_details(struct sna *sna, struct plane *p)
{
#define N_STACK_PROPS 32 /* must be a multiple of 2 */
	struct local_mode_obj_get_properties arg;
	uint64_t stack[N_STACK_PROPS + N_STACK_PROPS/2];
	uint64_t *values = stack;
	uint32_t *props = (uint32_t *)(values + N_STACK_PROPS);
	int i, type = DRM_PLANE_TYPE_OVERLAY;

	memset(&arg, 0, sizeof(struct local_mode_obj_get_properties));
	arg.obj_id = p->id;
	arg.obj_type = LOCAL_MODE_OBJECT_PLANE;

	arg.props_ptr = (uintptr_t)props;
	arg.prop_values_ptr = (uintptr_t)values;
	arg.count_props = N_STACK_PROPS;

	if (drmIoctl(sna->kgem.fd, LOCAL_IOCTL_MODE_OBJ_GETPROPERTIES, &arg))
		return -1;

	DBG(("%s: object %d (type %x) has %d props\n", __FUNCTION__,
	     p->id, LOCAL_MODE_OBJECT_PLANE, arg.count_props));

	if (arg.count_props > N_STACK_PROPS) {
		values = malloc(2*sizeof(uint64_t)*arg.count_props);
		if (values == NULL)
			return -1;

		props = (uint32_t *)(values + arg.count_props);

		arg.props_ptr = (uintptr_t)props;
		arg.prop_values_ptr = (uintptr_t)values;

		if (drmIoctl(sna->kgem.fd, LOCAL_IOCTL_MODE_OBJ_GETPROPERTIES, &arg))
			arg.count_props = 0;
	}
	VG(VALGRIND_MAKE_MEM_DEFINED(arg.props_ptr, sizeof(uint32_t)*arg.count_props));
	VG(VALGRIND_MAKE_MEM_DEFINED(arg.prop_values_ptr, sizeof(uint64_t)*arg.count_props));

	for (i = 0; i < arg.count_props; i++) {
		struct drm_mode_get_property prop;

		memset(&prop, 0, sizeof(prop));
		prop.prop_id = props[i];
		if (drmIoctl(sna->kgem.fd, DRM_IOCTL_MODE_GETPROPERTY, &prop)) {
			ERR(("%s: prop[%d].id=%d GETPROPERTY failed with errno=%d\n",
			     __FUNCTION__, i, props[i], errno));
			continue;
		}

		DBG(("%s: prop[%d] .id=%ld, .name=%s, .flags=%x, .value=%ld\n", __FUNCTION__, i,
		     (long)props[i], prop.name, (unsigned)prop.flags, (long)values[i]));

		if (strcmp(prop.name, "type") == 0) {
			type = values[i];
		} else if (prop_is_rotation(&prop)) {
			parse_rotation_prop(sna, p, &prop, values[i]);
		} else if (prop_is_color_encoding(&prop)) {
			parse_color_encoding_prop(sna, p, &prop, values[i]);
		}
	}

	p->rotation.supported &= DBG_NATIVE_ROTATION;
	if (!xf86ReturnOptValBool(sna->Options, OPTION_ROTATION, TRUE))
		p->rotation.supported = RR_Rotate_0;

	if (values != stack)
		free(values);

	DBG(("%s: plane=%d type=%d\n", __FUNCTION__, p->id, type));
	return type;
#undef N_STACK_PROPS
}

static void add_sprite_plane(struct sna_crtc *crtc,
			     struct plane *details)
{
	struct plane *sprite = malloc(sizeof(*sprite));
	if (!sprite)
		return;

	memcpy(sprite, details, sizeof(*sprite));
	list_add_tail(&sprite->link, &crtc->sprites);
}

static void
sna_crtc_find_planes(struct sna *sna, struct sna_crtc *crtc)
{
#define LOCAL_IOCTL_SET_CAP	DRM_IOWR(0x0d, struct local_set_cap)
	struct local_set_cap {
		uint64_t name;
		uint64_t value;
	} cap;
	struct local_mode_get_plane_res r;
	uint32_t stack_planes[32];
	uint32_t *planes = stack_planes;
	int i;

	VG_CLEAR(cap);
	cap.name = DRM_CLIENT_CAP_UNIVERSAL_PLANES;
	cap.value = 1;
	(void)drmIoctl(sna->kgem.fd, LOCAL_IOCTL_SET_CAP, &cap);

	VG_CLEAR(r);
	r.plane_id_ptr = (uintptr_t)planes;
	r.count_planes = ARRAY_SIZE(stack_planes);
	if (drmIoctl(sna->kgem.fd, LOCAL_IOCTL_MODE_GETPLANERESOURCES, &r)) {
		ERR(("%s: GETPLANERESOURCES failed with errno=%d\n", __FUNCTION__, errno));
		return;
	}

	DBG(("%s: %d planes\n", __FUNCTION__, (int)r.count_planes));

	if (r.count_planes > ARRAY_SIZE(stack_planes)) {
		planes = malloc(sizeof(uint32_t)*r.count_planes);
		if (planes == NULL)
			return;

		r.plane_id_ptr = (uintptr_t)planes;
		if (drmIoctl(sna->kgem.fd, LOCAL_IOCTL_MODE_GETPLANERESOURCES, &r))
			r.count_planes = 0;
	}

	VG(VALGRIND_MAKE_MEM_DEFINED(planes, sizeof(uint32_t)*r.count_planes));

	for (i = 0; i < r.count_planes; i++) {
		struct local_mode_get_plane p;
		struct plane details;

		VG_CLEAR(p);
		p.plane_id = planes[i];
		p.count_format_types = 0;
		if (drmIoctl(sna->kgem.fd, LOCAL_IOCTL_MODE_GETPLANE, &p))
			continue;

		if ((p.possible_crtcs & (1 << __sna_crtc_pipe(crtc))) == 0)
			continue;

		DBG(("%s: plane %d is attached to our pipe=%d\n",
		     __FUNCTION__, planes[i], __sna_crtc_pipe(crtc)));

		details.id = p.plane_id;
		details.rotation.prop = 0;
		details.rotation.supported = RR_Rotate_0;
		details.rotation.current = RR_Rotate_0;

		switch (plane_details(sna, &details)) {
		default:
			break;

		case DRM_PLANE_TYPE_PRIMARY:
			crtc->primary = details;
			break;

		case DRM_PLANE_TYPE_CURSOR:
			break;

		case DRM_PLANE_TYPE_OVERLAY:
			add_sprite_plane(crtc, &details);
			break;
		}
	}

	if (planes != stack_planes)
		free(planes);
}

static bool plane_has_format(const uint32_t formats[],
			     int count_formats,
			     uint32_t format)
{
	int i;

	for (i = 0; i < count_formats; i++) {
		if (formats[i] == format)
			return true;
	}

	return false;
}

bool sna_has_sprite_format(struct sna *sna, uint32_t format)
{
	xf86CrtcConfigPtr config = XF86_CRTC_CONFIG_PTR(sna->scrn);
	int i;

	if (sna->mode.num_real_crtc == 0)
		return false;

	for (i = 0; i < sna->mode.num_real_crtc; i++) {
		struct sna_crtc *sna_crtc = to_sna_crtc(config->crtc[i]);
		struct plane *plane;

		list_for_each_entry(plane, &sna_crtc->sprites, link) {
			struct local_mode_get_plane p;
			uint32_t *formats;
			int count_formats;
			bool has_format;

			VG_CLEAR(p);
			p.plane_id = plane->id;
			p.count_format_types = 0;
			if (drmIoctl(sna->kgem.fd,
				     LOCAL_IOCTL_MODE_GETPLANE,
				     &p))
				continue;
			count_formats = p.count_format_types;

			formats = calloc(count_formats, sizeof(formats[0]));
			if (!formats)
				continue;

			p.count_format_types = count_formats;
			p.format_type_ptr = (uintptr_t)formats;
			if (drmIoctl(sna->kgem.fd,
				     LOCAL_IOCTL_MODE_GETPLANE,
				     &p)) {
				free(formats);
				continue;
			}

			assert(p.count_format_types == count_formats);

			has_format = plane_has_format(formats,
						      count_formats,
						      format);

			free(formats);

			/*
			 * As long as one plane supports the
			 * format we declare it as supported.
			 * Not all planes may support it, but
			 * then the GPU fallback will kick in.
			 */
			if (has_format)
				return true;
		}
	}

	return false;
}

static void
sna_crtc_init__rotation(struct sna *sna, struct sna_crtc *crtc)
{
	crtc->rotation = RR_Rotate_0;
	crtc->primary.rotation.supported = RR_Rotate_0;
	crtc->primary.rotation.current = RR_Rotate_0;
}

static void
sna_crtc_init__cursor(struct sna *sna, struct sna_crtc *crtc)
{
	struct drm_mode_cursor arg;

	VG_CLEAR(arg);
	arg.flags = DRM_MODE_CURSOR_BO;
	arg.crtc_id = __sna_crtc_id(crtc);
	arg.width = arg.height = 0;
	arg.handle = 0;

	(void)drmIoctl(sna->kgem.fd, DRM_IOCTL_MODE_CURSOR, &arg);
	crtc->hwcursor = true;
}

static bool
sna_crtc_add(ScrnInfoPtr scrn, unsigned id)
{
	struct sna *sna = to_sna(scrn);
	xf86CrtcPtr crtc;
	struct sna_crtc *sna_crtc;
	struct drm_i915_get_pipe_from_crtc_id get_pipe;

	DBG(("%s(%d): is-zaphod? %d\n", __FUNCTION__, id, is_zaphod(scrn)));

	sna_crtc = calloc(sizeof(struct sna_crtc), 1);
	if (sna_crtc == NULL)
		return false;

	list_init(&sna_crtc->public.vblank_queue);
	sna_crtc->id = id;

	VG_CLEAR(get_pipe);
	get_pipe.pipe = 0;
	get_pipe.crtc_id = id;
	if (drmIoctl(sna->kgem.fd,
		     DRM_IOCTL_I915_GET_PIPE_FROM_CRTC_ID,
		     &get_pipe)) {
		free(sna_crtc);
		return false;
	}
	assert((unsigned)get_pipe.pipe < 256);
	sna_crtc->public.flags |= get_pipe.pipe << 8;

	if (is_zaphod(scrn) &&
	    (get_zaphod_crtcs(sna) & (1 << get_pipe.pipe)) == 0) {
		free(sna_crtc);
		return true;
	}

	list_init(&sna_crtc->sprites);
	sna_crtc_init__rotation(sna, sna_crtc);
	sna_crtc_find_planes(sna, sna_crtc);

	DBG(("%s: CRTC:%d [pipe=%d], primary id=%x: supported-rotations=%x, current-rotation=%x\n",
	     __FUNCTION__, id, get_pipe.pipe,
	     sna_crtc->primary.id, sna_crtc->primary.rotation.supported, sna_crtc->primary.rotation.current));

	list_init(&sna_crtc->shadow_link);

	crtc = xf86CrtcCreate(scrn, &sna_crtc_funcs);
	if (crtc == NULL) {
		free(sna_crtc);
		return false;
	}

	sna_crtc_init__cursor(sna, sna_crtc);

	crtc->driver_private = sna_crtc;
	sna_crtc->base = crtc;
	DBG(("%s: attached crtc[%d] pipe=%d\n",
	     __FUNCTION__, id, __sna_crtc_pipe(sna_crtc)));

	return true;
}

static bool
is_panel(int type)
{
#define DRM_MODE_CONNECTOR_LVDS 7
#define DRM_MODE_CONNECTOR_eDP 14
#define DRM_MODE_CONNECTOR_DSI 16
	return (type == DRM_MODE_CONNECTOR_LVDS ||
		type == DRM_MODE_CONNECTOR_eDP ||
		type == DRM_MODE_CONNECTOR_DSI);
}

static int
find_property(struct sna *sna, struct sna_output *output, const char *name)
{
	struct drm_mode_get_property prop;
	int i;

	VG_CLEAR(prop);
	for (i = 0; i < output->num_props; i++) {
		prop.prop_id = output->prop_ids[i];
		prop.count_values = 0;
		prop.count_enum_blobs = 0;
		if (drmIoctl(sna->kgem.fd, DRM_IOCTL_MODE_GETPROPERTY, &prop))
			continue;

		if (strcmp(prop.name, name) == 0)
			return i;
	}

	return -1;
}

static void update_properties(struct sna *sna, struct sna_output *output)
{
	union compat_mode_get_connector compat_conn;
	struct drm_mode_modeinfo dummy;

	VG_CLEAR(compat_conn);

	compat_conn.conn.connector_id = output->id;
	compat_conn.conn.count_props = output->num_props;
	compat_conn.conn.props_ptr = (uintptr_t)output->prop_ids;
	compat_conn.conn.prop_values_ptr = (uintptr_t)output->prop_values;
	compat_conn.conn.count_modes = 1; /* skip detect */
	compat_conn.conn.modes_ptr = (uintptr_t)&dummy;
	compat_conn.conn.count_encoders = 0;

	(void)drmIoctl(sna->kgem.fd,
		       DRM_IOCTL_MODE_GETCONNECTOR,
		       &compat_conn.conn);

	assert(compat_conn.conn.count_props == output->num_props);
	output->update_properties = false;
}

static xf86OutputStatus
sna_output_detect(xf86OutputPtr output)
{
	struct sna *sna = to_sna(output->scrn);
	struct sna_output *sna_output = output->driver_private;
	union compat_mode_get_connector compat_conn;
	uint32_t now;

	DBG(("%s(%s:%d)\n", __FUNCTION__, output->name, sna_output->id));
	sna_output->update_properties = false;

	if (!sna_output->id) {
		DBG(("%s(%s) hiding due to lost connection\n", __FUNCTION__, output->name));
		return XF86OutputStatusDisconnected;
	}

	/* Cache detections for 15s or hotplug event  */
	now = GetTimeInMillis();
	if (sna_output->last_detect != 0 &&
	    (int32_t)(now - sna_output->last_detect) <= OUTPUT_STATUS_CACHE_MS) {
		DBG(("%s(%s) reporting cached status (since %dms): %d\n",
		     __FUNCTION__, output->name, now - sna_output->last_detect,
		     sna_output->status));
		sna_output->update_properties = true;
		return sna_output->status;
	}

	VG_CLEAR(compat_conn);
	compat_conn.conn.connector_id = sna_output->id;
	sna_output->num_modes = compat_conn.conn.count_modes = 0; /* reprobe */
	compat_conn.conn.count_encoders = 0;
	compat_conn.conn.count_props = sna_output->num_props;
	compat_conn.conn.props_ptr = (uintptr_t)sna_output->prop_ids;
	compat_conn.conn.prop_values_ptr = (uintptr_t)sna_output->prop_values;

	if (drmIoctl(sna->kgem.fd, DRM_IOCTL_MODE_GETCONNECTOR, &compat_conn.conn))
		return XF86OutputStatusUnknown;
	DBG(("%s(%s): num modes %d -> %d, num props %d -> %d\n",
	     __FUNCTION__, output->name,
	     sna_output->num_modes, compat_conn.conn.count_modes,
	     sna_output->num_props, compat_conn.conn.count_props));

	assert(compat_conn.conn.count_props == sna_output->num_props);

	while (compat_conn.conn.count_modes && compat_conn.conn.count_modes != sna_output->num_modes) {
		struct drm_mode_modeinfo *new_modes;
		int old_count;

		old_count = sna_output->num_modes;
		new_modes = realloc(sna_output->modes,
				    sizeof(*sna_output->modes)*compat_conn.conn.count_modes);
		if (new_modes == NULL)
			break;

		sna_output->modes = new_modes;
		sna_output->num_modes = compat_conn.conn.count_modes;
		compat_conn.conn.modes_ptr = (uintptr_t)sna_output->modes;
		compat_conn.conn.count_encoders = 0;
		compat_conn.conn.count_props = 0;
		if (drmIoctl(sna->kgem.fd, DRM_IOCTL_MODE_GETCONNECTOR, &compat_conn.conn)) {
			sna_output->num_modes = min(old_count, sna_output->num_modes);
			break;
		}
		VG(VALGRIND_MAKE_MEM_DEFINED(sna_output->modes, sizeof(*sna_output->modes)*sna_output->num_modes));
	}

	DBG(("%s(%s): found %d modes, connection status=%d\n",
	     __FUNCTION__, output->name, sna_output->num_modes, compat_conn.conn.connection));

	sna_output->reprobe = false;
	sna_output->last_detect = now;
	switch (compat_conn.conn.connection) {
	case DRM_MODE_CONNECTED:
		sna_output->status = XF86OutputStatusConnected;
		output->mm_width = compat_conn.conn.mm_width;
		output->mm_height = compat_conn.conn.mm_height;
		break;
	case DRM_MODE_DISCONNECTED:
		sna_output->status = XF86OutputStatusDisconnected;
		break;
	default:
	case DRM_MODE_UNKNOWNCONNECTION:
		sna_output->status = XF86OutputStatusUnknown;
		break;
	}
	return sna_output->status;
}

static Bool
sna_output_mode_valid(xf86OutputPtr output, DisplayModePtr mode)
{
	struct sna_output *sna_output = output->driver_private;
	struct sna *sna = to_sna(output->scrn);

	if (mode->HDisplay > sna->mode.max_crtc_width)
		return MODE_VIRTUAL_X;
	if (mode->VDisplay > sna->mode.max_crtc_height)
		return MODE_VIRTUAL_Y;

	/* Check that we can successfully pin this into the global GTT */
	if ((kgem_can_create_2d(&sna->kgem,
				mode->HDisplay, mode->VDisplay,
				sna->scrn->bitsPerPixel) & KGEM_CAN_CREATE_GTT) == 0)
		return MODE_MEM_VIRT;

	/*
	 * If the connector type is a panel, we will use the panel limit to
	 * verfiy whether the mode is valid.
	 */
	if (sna_output->has_panel_limits) {
		if (mode->HDisplay > sna_output->panel_hdisplay ||
		    mode->VDisplay > sna_output->panel_vdisplay)
			return MODE_PANEL;
	}

	return MODE_OK;
}

static void sna_output_set_parsed_edid(xf86OutputPtr output, xf86MonPtr mon)
{
	unsigned conn_mm_width, conn_mm_height;

	/* We set the output size based on values from the kernel */
	conn_mm_width = output->mm_width;
	conn_mm_height = output->mm_height;

	xf86OutputSetEDID(output, mon);

	if (output->mm_width != conn_mm_width || output->mm_height != conn_mm_height) {
		DBG(("%s)%s): kernel and Xorg disagree over physical size: kernel=%dx%dmm, Xorg=%dx%dmm\n",
		     __FUNCTION__, output->name,
		     conn_mm_width, conn_mm_height,
		     output->mm_width, output->mm_height));
	}

	output->mm_width = conn_mm_width;
	output->mm_height = conn_mm_height;
}

static void
sna_output_attach_edid(xf86OutputPtr output)
{
	struct sna *sna = to_sna(output->scrn);
	struct sna_output *sna_output = output->driver_private;
	struct drm_mode_get_blob blob;
	void *old, *raw = NULL;
	xf86MonPtr mon = NULL;

	if (sna_output->edid_idx == -1)
		return;

	/* Always refresh the blob as the kernel may randomly update the
	 * id even if the contents of the blob doesn't change, and a
	 * request for the stale id will return nothing.
	 */
	if (sna_output->update_properties)
		update_properties(sna, sna_output);

	raw = sna_output->edid_raw;
	blob.length = sna_output->edid_len;

	if (blob.length && output->MonInfo) {
		old = alloca(blob.length);
		memcpy(old, raw, blob.length);
	} else
		old = NULL;

	blob.blob_id = sna_output->prop_values[sna_output->edid_idx];
	if (!blob.blob_id)
		goto done;

	DBG(("%s(%s): attaching EDID id=%d, current=%d\n",
	     __FUNCTION__, output->name,
	     blob.blob_id, sna_output->edid_blob_id));
	if (blob.blob_id == sna_output->edid_blob_id && 0) { /* sigh */
		if (output->MonInfo) {
			/* XXX the property keeps on disappearing... */
			RRChangeOutputProperty(output->randr_output,
					       MakeAtom("EDID", strlen("EDID"), TRUE),
					       XA_INTEGER, 8, PropModeReplace,
					       sna_output->edid_len,
					       sna_output->edid_raw,
					       FALSE, FALSE);

			return;
		}

		goto skip_read;
	}

	blob.data = (uintptr_t)raw;
	do {
		while (drmIoctl(sna->kgem.fd, DRM_IOCTL_MODE_GETPROPBLOB, &blob)) {
			update_properties(sna, sna_output);
			if (blob.blob_id == sna_output->prop_values[sna_output->edid_idx]) {
				DBG(("%s(%s): failed to read blob, reusing previous\n",
				     __FUNCTION__, output->name));
				goto done;
			}
			blob.blob_id = sna_output->prop_values[sna_output->edid_idx];
		}

		DBG(("%s(%s): retrieving blob id=%d, length=%d\n",
		     __FUNCTION__, output->name, blob.blob_id, blob.length));

		if (blob.length < 128)
			goto done;

		if (blob.length > sna_output->edid_len) {
			raw = realloc(raw, blob.length);
			if (raw == NULL)
				goto done;

			VG(memset(raw, 0, blob.length));
			blob.data = (uintptr_t)raw;
		}
	} while (blob.length != sna_output->edid_len &&
		 drmIoctl(sna->kgem.fd, DRM_IOCTL_MODE_GETPROPBLOB, &blob));

	if (blob.length & 127) {
		/* Truncated EDID! Make sure no one reads too far */
		*SECTION(NO_EDID, (uint8_t*)raw) = blob.length/128 - 1;
		blob.length &= -128;
	}

	if (old &&
	    blob.length == sna_output->edid_len &&
	    memcmp(old, raw, blob.length) == 0) {
		DBG(("%s(%s): EDID + MonInfo is unchanged\n",
		     __FUNCTION__, output->name));
		assert(sna_output->edid_raw == raw);
		sna_output->edid_blob_id = blob.blob_id;
		RRChangeOutputProperty(output->randr_output,
				       MakeAtom("EDID", strlen("EDID"), TRUE),
				       XA_INTEGER, 8, PropModeReplace,
				       sna_output->edid_len,
				       sna_output->edid_raw,
				       FALSE, FALSE);
		return;
	}

skip_read:
	if (raw) {
		mon = xf86InterpretEDID(output->scrn->scrnIndex, raw);
		if (mon && blob.length > 128)
			mon->flags |= MONITOR_EDID_COMPLETE_RAWDATA;
	}

done:
	sna_output_set_parsed_edid(output, mon);
	if (raw) {
		sna_output->edid_raw = raw;
		sna_output->edid_len = blob.length;
		sna_output->edid_blob_id = blob.blob_id;
	}
}

static void
sna_output_attach_tile(xf86OutputPtr output)
{
#if XF86_OUTPUT_VERSION >= 3
	struct sna *sna = to_sna(output->scrn);
	struct sna_output *sna_output = output->driver_private;
	struct drm_mode_get_blob blob;
	struct xf86CrtcTileInfo tile_info, *set = NULL;
	char *tile;
	int id;

	id = find_property(sna, sna_output, "TILE");
	DBG(("%s: found? TILE=%d\n", __FUNCTION__, id));
	if (id == -1)
		goto out;

	if (sna_output->update_properties)
		update_properties(sna, sna_output);

	VG_CLEAR(blob);
	blob.blob_id = sna_output->prop_values[id];
	blob.length = 0;
	if (drmIoctl(sna->kgem.fd, DRM_IOCTL_MODE_GETPROPBLOB, &blob))
		goto out;

	do {
		id = blob.length;
		tile = alloca(id + 1);
		blob.data = (uintptr_t)tile;
		VG(memset(tile, 0, id));
		DBG(("%s: reading %d bytes for TILE blob\n", __FUNCTION__, id));
		if (drmIoctl(sna->kgem.fd, DRM_IOCTL_MODE_GETPROPBLOB, &blob))
			goto out;
	} while (id != blob.length);

	tile[blob.length] = '\0'; /* paranoia */
	DBG(("%s: TILE='%s'\n", __FUNCTION__, tile));
	if (xf86OutputParseKMSTile(tile, blob.length, &tile_info))
		set = &tile_info;
out:
	xf86OutputSetTile(output, set);
#endif
}

static bool duplicate_mode(DisplayModePtr modes, DisplayModePtr m)
{
	if (m == NULL)
		return false;

	while (modes) {
		if (xf86ModesEqual(modes, m))
			return true;

		modes = modes->next;
	}

	return false;
}

static struct pixel_count {
	int16_t width, height;
} common_16_9[] = {
	{ 640, 360 },
	{ 720, 405 },
	{ 864, 486 },
	{ 960, 540 },
	{ 1024, 576 },
	{ 1280, 720 },
	{ 1366, 768 },
	{ 1600, 900 },
	{ 1920, 1080 },
	{ 2048, 1152 },
	{ 2560, 1440 },
	{ 2880, 1620 },
	{ 3200, 1800 },
	{ 3840, 2160 },
	{ 4096, 2304 },
	{ 5120, 2880 },
	{ 7680, 4320 },
	{ 15360, 8640 },
}, common_16_10[] = {
	{ 1280, 800 },
	{ 1400, 900 },
	{ 1680, 1050 },
	{ 1920, 1200 },
	{ 2560, 1600 },
};

static DisplayModePtr
default_modes(DisplayModePtr preferred)
{
	DisplayModePtr modes;
	int n;

#if XORG_VERSION_CURRENT >= XORG_VERSION_NUMERIC(1,6,99,900,0)
	modes = xf86GetDefaultModes();
#else
	modes = xf86GetDefaultModes(0, 0);
#endif

	/* XXX O(n^2) mode list generation :( */

#if XORG_VERSION_CURRENT >= XORG_VERSION_NUMERIC(1,4,99,901,0)
	if (preferred) {
		DisplayModePtr m;

		/* Add a half-resolution mode useful for large panels */
		m = xf86GTFMode(preferred->HDisplay/2,
				preferred->VDisplay/2,
				xf86ModeVRefresh(preferred),
				FALSE, FALSE);
		if (!duplicate_mode(modes, m))
			modes = xf86ModesAdd(modes, m);
		else
			free(m);

		if (preferred->VDisplay * 16 > preferred->HDisplay*9 - preferred->HDisplay/32 &&
		    preferred->VDisplay * 16 < preferred->HDisplay*9 + preferred->HDisplay/32) {
			DBG(("Adding 16:9 modes -- %d < %d > %d\n",
			     preferred->HDisplay*9 - preferred->HDisplay/32,
			     preferred->VDisplay * 16,
			     preferred->HDisplay*9 + preferred->HDisplay/32));
			for (n = 0; n < ARRAY_SIZE(common_16_9); n++) {
				if (preferred->HDisplay <= common_16_9[n].width ||
				    preferred->VDisplay <= common_16_9[n].height)
					break;

				m = xf86GTFMode(common_16_9[n].width,
						common_16_9[n].height,
						xf86ModeVRefresh(preferred),
						FALSE, FALSE);
				if (!duplicate_mode(modes, m))
					modes = xf86ModesAdd(modes, m);
				else
					free(m);
			}
		}

		if (preferred->VDisplay * 16 > preferred->HDisplay*10 - preferred->HDisplay/32 &&
		    preferred->VDisplay * 16 < preferred->HDisplay*10 + preferred->HDisplay/32) {
			DBG(("Adding 16:10 modes -- %d < %d > %d\n",
			     preferred->HDisplay*10 - preferred->HDisplay/32,
			     preferred->VDisplay * 16,
			     preferred->HDisplay*10 + preferred->HDisplay/32));
			for (n = 0; n < ARRAY_SIZE(common_16_10); n++) {
				if (preferred->HDisplay <= common_16_10[n].width ||
				    preferred->VDisplay <= common_16_10[n].height)
					break;

				m = xf86GTFMode(common_16_10[n].width,
						common_16_10[n].height,
						xf86ModeVRefresh(preferred),
						FALSE, FALSE);
				if (!duplicate_mode(modes, m))
					modes = xf86ModesAdd(modes, m);
				else
					free(m);
			}
		}
	}
#endif

	return modes;
}

static DisplayModePtr
sna_output_add_default_modes(xf86OutputPtr output, DisplayModePtr modes)
{
	xf86MonPtr mon = output->MonInfo;
	DisplayModePtr i, m, preferred = NULL;
	int max_x = 0, max_y = 0, max_clock = 0;
	float max_vrefresh = 0.0;

	if (mon && GTF_SUPPORTED(mon->features.msc))
		return modes;

	for (m = modes; m; m = m->next) {
		if (m->type & M_T_PREFERRED)
			preferred = m;
		max_x = max(max_x, m->HDisplay);
		max_y = max(max_y, m->VDisplay);
		max_clock = max(max_clock, m->Clock);
		max_vrefresh = max(max_vrefresh, xf86ModeVRefresh(m));
	}
	max_vrefresh *= (1 + SYNC_TOLERANCE);

	m = default_modes(preferred);
	xf86ValidateModesSize(output->scrn, m, max_x, max_y, 0);

	for (i = m; i; i = i->next) {
		if (i->Clock > max_clock)
			i->status = MODE_CLOCK_HIGH;
		if (xf86ModeVRefresh(i) > max_vrefresh)
			i->status = MODE_VSYNC;
		if (preferred &&
		    i->HDisplay >= preferred->HDisplay &&
		    i->VDisplay >= preferred->VDisplay &&
		    xf86ModeVRefresh(i) >= xf86ModeVRefresh(preferred))
			i->status = MODE_PANEL;
	}

	xf86PruneInvalidModes(output->scrn, &m, FALSE);

	return xf86ModesAdd(modes, m);
}

static DisplayModePtr
sna_output_override_edid(xf86OutputPtr output)
{
	struct sna_output *sna_output = output->driver_private;
	xf86MonPtr mon = NULL;

	if (sna_output->fake_edid_raw == NULL)
		return NULL;

	mon = xf86InterpretEDID(output->scrn->scrnIndex, sna_output->fake_edid_raw);
	if (mon == NULL) {
		return NULL;
	}

	mon->flags |= MONITOR_EDID_COMPLETE_RAWDATA;

	xf86OutputSetEDID(output, mon);

	return xf86DDCGetModes(output->scrn->scrnIndex, mon);
}

static DisplayModePtr
sna_output_get_modes(xf86OutputPtr output)
{
	struct sna_output *sna_output = output->driver_private;
	DisplayModePtr Modes, current;
	int i;

	DBG(("%s(%s:%d)\n", __FUNCTION__, output->name, sna_output->id));
	assert(sna_output->id);

	Modes = sna_output_override_edid(output);
	if (Modes)
		return Modes;

	sna_output_attach_edid(output);
	sna_output_attach_tile(output);

	current = NULL;
	if (output->crtc && !sna_output->hotplug_count) {
		struct drm_mode_crtc mode;

		VG_CLEAR(mode);
		assert(to_sna_crtc(output->crtc));
		mode.crtc_id = sna_crtc_id(output->crtc);

		if (drmIoctl(to_sna(output->scrn)->kgem.fd, DRM_IOCTL_MODE_GETCRTC, &mode) == 0) {
			DBG(("%s: CRTC:%d, pipe=%d: has mode?=%d\n", __FUNCTION__,
			     sna_crtc_id(output->crtc),
			     sna_crtc_pipe(output->crtc),
			     mode.mode_valid && mode.mode.clock));

			if (mode.mode_valid && mode.mode.clock) {
				current = calloc(1, sizeof(DisplayModeRec));
				if (current) {
					mode_from_kmode(output->scrn, &mode.mode, current);
					current->type |= M_T_DRIVER | M_T_PREFERRED;
				}
			}
		}
	}

	DBG(("%s: adding %d probed modes\n", __FUNCTION__, sna_output->num_modes));

	for (i = 0; i < sna_output->num_modes; i++) {
		DisplayModePtr mode;

		mode = calloc(1, sizeof(DisplayModeRec));
		if (mode == NULL)
			continue;

		mode = mode_from_kmode(output->scrn,
				       &sna_output->modes[i],
				       mode);
		Modes = xf86ModesAdd(Modes, mode);
		if (current && xf86ModesEqual(mode, current)) {
			free((void*)current->name);
			free(current);
			current = NULL;
		}
		if (current && mode->type & M_T_PREFERRED)
			current->type &= ~M_T_PREFERRED;
	}

	if (current)
		Modes = xf86ModesAdd(current, Modes);

	/*
	 * If the connector type is a panel, we will traverse the kernel mode to
	 * get the panel limit. And then add all the standard modes to fake
	 * the fullscreen experience.
	 * If it is incorrect, please fix me.
	 */
	sna_output->has_panel_limits = false;
	if (sna_output->is_panel) {
		sna_output->panel_hdisplay = sna_output->panel_vdisplay = 0;
		for (i = 0; i < sna_output->num_modes; i++) {
			struct drm_mode_modeinfo *m;

			m = &sna_output->modes[i];
			if (m->hdisplay > sna_output->panel_hdisplay)
				sna_output->panel_hdisplay = m->hdisplay;
			if (m->vdisplay > sna_output->panel_vdisplay)
				sna_output->panel_vdisplay = m->vdisplay;
		}
		sna_output->has_panel_limits =
			sna_output->panel_hdisplay &&
			sna_output->panel_vdisplay;
	}

	if (sna_output->add_default_modes)
		Modes = sna_output_add_default_modes(output, Modes);

	return Modes;
}

static void
sna_output_destroy(xf86OutputPtr output)
{
	struct sna_output *sna_output = output->driver_private;
	int i;

	if (sna_output == NULL)
		return;

	free(sna_output->edid_raw);
	free(sna_output->fake_edid_raw);

	for (i = 0; i < sna_output->num_props; i++) {
		if (sna_output->props[i].kprop == NULL)
			continue;

		if (sna_output->props[i].atoms) {
			if (output->randr_output)
				RRDeleteOutputProperty(output->randr_output, sna_output->props[i].atoms[0]);
			free(sna_output->props[i].atoms);
		}

		drmModeFreeProperty(sna_output->props[i].kprop);
	}
	free(sna_output->props);
	free(sna_output->prop_ids);
	free(sna_output->prop_values);

	backlight_close(&sna_output->backlight);

	free(sna_output);
	output->driver_private = NULL;
}

static void
__sna_output_dpms(xf86OutputPtr output, int dpms, int fixup)
{
	struct sna *sna = to_sna(output->scrn);
	struct sna_output *sna_output = output->driver_private;
	int old_dpms = sna_output->dpms_mode;

	DBG(("%s(%s:%d): dpms=%d (current: %d), active? %d\n",
	     __FUNCTION__, output->name, sna_output->id,
	     dpms, sna_output->dpms_mode,
	     output->crtc != NULL));

	if (!sna_output->id)
		return;

	if (old_dpms == dpms)
		return;

	/* Record the value of the backlight before turning
	 * off the display, and reset if after turning it on.
	 * Order is important as the kernel may record and also
	 * reset the backlight across DPMS. Hence we need to
	 * record the value before the kernel modifies it
	 * and reapply it afterwards.
	 */
	if (sna_output->backlight.iface && dpms != DPMSModeOn) {
		if (old_dpms == DPMSModeOn) {
			sna_output->backlight_active_level = sna_output_backlight_get(output);
			DBG(("%s(%s:%d): saving current backlight %d\n",
			     __FUNCTION__, output->name, sna_output->id,
			     sna_output->backlight_active_level));
		}
		sna_output->dpms_mode = dpms;
		sna_output_backlight_off(sna_output);
	}

	if (output->crtc &&
	    drmModeConnectorSetProperty(sna->kgem.fd,
					sna_output->id,
					sna_output->dpms_id,
					dpms)) {
		DBG(("%s(%s:%d): failed to set DPMS to %d (fixup? %d)\n",
		     __FUNCTION__, output->name, sna_output->id, dpms, fixup));
		if (fixup && dpms != DPMSModeOn) {
			sna_crtc_disable(output->crtc, false);
			return;
		}
	}

	if (sna_output->backlight.iface && dpms == DPMSModeOn) {
		DBG(("%s(%d:%d: restoring previous backlight %d\n",
		     __FUNCTION__, output->name, sna_output->id,
		     sna_output->backlight_active_level));
		sna_output_backlight_on(sna_output);
	}

	sna_output->dpms_mode = dpms;
}

static void
sna_output_dpms(xf86OutputPtr output, int dpms)
{
	__sna_output_dpms(output, dpms, true);
}

static bool
sna_property_ignore(drmModePropertyPtr prop)
{
	if (!prop)
		return true;

	/* ignore blob prop */
	if (prop->flags & DRM_MODE_PROP_BLOB)
		return true;

	/* ignore standard property */
	if (!strcmp(prop->name, "EDID") ||
	    !strcmp(prop->name, "DPMS"))
		return true;

	return false;
}

static void
sna_output_create_ranged_atom(xf86OutputPtr output, Atom *atom,
			      const char *name, INT32 min, INT32 max,
			      uint64_t value, Bool immutable)
{
	int err;
	INT32 atom_range[2];

	atom_range[0] = min;
	atom_range[1] = max;

	*atom = MakeAtom(name, strlen(name), TRUE);

	err = RRConfigureOutputProperty(output->randr_output, *atom, FALSE,
					TRUE, immutable, 2, atom_range);
	if (err != 0)
		xf86DrvMsg(output->scrn->scrnIndex, X_WARNING,
			   "RRConfigureOutputProperty error, %d\n", err);

	err = RRChangeOutputProperty(output->randr_output, *atom, XA_INTEGER,
				     32, PropModeReplace, 1, &value,
				     FALSE, FALSE);
	if (err != 0)
		xf86DrvMsg(output->scrn->scrnIndex, X_WARNING,
			   "RRChangeOutputProperty error, %d\n", err);
}

static void
sna_output_create_resources(xf86OutputPtr output)
{
	struct sna *sna = to_sna(output->scrn);
	struct sna_output *sna_output = output->driver_private;
	int i, j, err;

	sna_output->props = calloc(sna_output->num_props,
				   sizeof(struct sna_property));
	if (!sna_output->props)
		return;

	for (i = 0; i < sna_output->num_props; i++) {
		struct sna_property *p = &sna_output->props[i];

		p->kprop = drmModeGetProperty(sna->kgem.fd,
					      sna_output->prop_ids[i]);
		if (sna_property_ignore(p->kprop)) {
			drmModeFreeProperty(p->kprop);
			p->kprop = NULL;
			continue;
		}

		if (p->kprop->flags & DRM_MODE_PROP_RANGE) {
			p->num_atoms = 1;
			p->atoms = calloc(p->num_atoms, sizeof(Atom));
			if (!p->atoms)
				continue;

			sna_output_create_ranged_atom(output, &p->atoms[0],
						      p->kprop->name,
						      p->kprop->values[0],
						      p->kprop->values[1],
						      sna_output->prop_values[i],
						      p->kprop->flags & DRM_MODE_PROP_IMMUTABLE ? TRUE : FALSE);

		} else if (p->kprop->flags & DRM_MODE_PROP_ENUM) {
			p->num_atoms = p->kprop->count_enums + 1;
			p->atoms = calloc(p->num_atoms, sizeof(Atom));
			if (!p->atoms)
				continue;

			p->atoms[0] = MakeAtom(p->kprop->name, strlen(p->kprop->name), TRUE);
			for (j = 1; j <= p->kprop->count_enums; j++) {
				struct drm_mode_property_enum *e = &p->kprop->enums[j-1];
				p->atoms[j] = MakeAtom(e->name, strlen(e->name), TRUE);
			}

			err = RRConfigureOutputProperty(output->randr_output, p->atoms[0],
							FALSE, FALSE,
							p->kprop->flags & DRM_MODE_PROP_IMMUTABLE ? TRUE : FALSE,
							p->num_atoms - 1, (INT32 *)&p->atoms[1]);
			if (err != 0) {
				xf86DrvMsg(output->scrn->scrnIndex, X_WARNING,
					   "RRConfigureOutputProperty error, %d\n", err);
			}

			for (j = 0; j < p->kprop->count_enums; j++)
				if (p->kprop->enums[j].value == sna_output->prop_values[i])
					break;
			/* there's always a matching value */
			err = RRChangeOutputProperty(output->randr_output, p->atoms[0],
						     XA_ATOM, 32, PropModeReplace, 1, &p->atoms[j+1],
						     FALSE, FALSE);
			if (err != 0) {
				xf86DrvMsg(output->scrn->scrnIndex, X_WARNING,
					   "RRChangeOutputProperty error, %d\n", err);
			}
		}
	}

	if (sna_output->backlight.iface) {
		/* Set up the backlight property, which takes effect
		 * immediately and accepts values only within the
		 * backlight_range.
		 */
		sna_output_create_ranged_atom(output, &backlight_atom,
					      BACKLIGHT_NAME, 0,
					      sna_output->backlight.max,
					      sna_output->backlight_active_level,
					      FALSE);
		sna_output_create_ranged_atom(output,
					      &backlight_deprecated_atom,
					      BACKLIGHT_DEPRECATED_NAME, 0,
					      sna_output->backlight.max,
					      sna_output->backlight_active_level,
					      FALSE);
	}
}

static Bool
sna_output_set_property(xf86OutputPtr output, Atom property,
			RRPropertyValuePtr value)
{
	struct sna *sna = to_sna(output->scrn);
	struct sna_output *sna_output = output->driver_private;
	int i;

	if (property == backlight_atom || property == backlight_deprecated_atom) {
		INT32 val;
		int ret = 0;

		if (value->type != XA_INTEGER || value->format != 32 ||
		    value->size != 1)
		{
			return FALSE;
		}

		val = *(INT32 *)value->data;
		DBG(("%s: setting backlight to %d (max=%d)\n",
		     __FUNCTION__, (int)val, sna_output->backlight.max));
		if (val < 0 || val > sna_output->backlight.max)
			return FALSE;

		sna_output->backlight_active_level = val;
		if (sna_output->dpms_mode == DPMSModeOn)
			ret = sna_output_backlight_set(sna_output, val);
		return ret == 0;
	}

	if (!sna_output->id)
		return TRUE;

	for (i = 0; i < sna_output->num_props; i++) {
		struct sna_property *p = &sna_output->props[i];

		if (p->atoms == NULL || p->atoms[0] != property)
			continue;

		if (p->kprop->flags & DRM_MODE_PROP_RANGE) {
			uint32_t val;

			if (value->type != XA_INTEGER || value->format != 32 ||
			    value->size != 1)
				return FALSE;

			val = *(uint32_t *)value->data;
			drmModeConnectorSetProperty(sna->kgem.fd, sna_output->id,
						    p->kprop->prop_id, (uint64_t)val);
			return TRUE;
		} else if (p->kprop->flags & DRM_MODE_PROP_ENUM) {
			Atom atom;
			const char *name;
			int j;

			if (value->type != XA_ATOM || value->format != 32 || value->size != 1)
				return FALSE;

			memcpy(&atom, value->data, 4);
			name = NameForAtom(atom);
			if (name == NULL)
				return FALSE;

			/* search for matching name string, then set its value down */
			for (j = 0; j < p->kprop->count_enums; j++) {
				if (!strcmp(p->kprop->enums[j].name, name)) {
					drmModeConnectorSetProperty(sna->kgem.fd, sna_output->id,
								    p->kprop->prop_id, p->kprop->enums[j].value);
					return TRUE;
				}
			}
			return FALSE;
		}
	}

	/* We didn't recognise this property, just report success in order
	 * to allow the set to continue, otherwise we break setting of
	 * common properties like EDID.
	 */
	return TRUE;
}

static Bool
sna_output_get_property(xf86OutputPtr output, Atom property)
{
	struct sna_output *sna_output = output->driver_private;
	int err, i, j;

	if (property == backlight_atom || property == backlight_deprecated_atom) {
		INT32 val;

		if (!sna_output->backlight.iface)
			return FALSE;

		if (sna_output->dpms_mode == DPMSModeOn) {
			val = sna_output_backlight_get(output);
			if (val < 0)
				return FALSE;
			DBG(("%s(%s): output on, reporting actual backlight value [%d]\n",
			     __FUNCTION__, output->name, val));
		} else {
			val = sna_output->backlight_active_level;
			DBG(("%s(%s): output off, reporting cached backlight value [%d]\n",
			     __FUNCTION__, output->name, val));
		}

		err = RRChangeOutputProperty(output->randr_output, property,
					     XA_INTEGER, 32, PropModeReplace, 1, &val,
					     FALSE, FALSE);
		if (err != 0) {
			xf86DrvMsg(output->scrn->scrnIndex, X_WARNING,
				   "RRChangeOutputProperty error, %d\n", err);
			return FALSE;
		}

		return TRUE;
	}

	for (i = 0; i < sna_output->num_props; i++) {
		struct sna_property *p = &sna_output->props[i];

		if (p->atoms == NULL || p->atoms[0] != property)
			continue;

		if (sna_output->update_properties && output->scrn->vtSema)
			update_properties(to_sna(output->scrn), sna_output);

		err = 0;
		if (p->kprop->flags & DRM_MODE_PROP_RANGE) {
			err = RRChangeOutputProperty(output->randr_output,
						     property, XA_INTEGER, 32,
						     PropModeReplace, 1,
						     &sna_output->prop_values[i],
						     FALSE, FALSE);
		} else if (p->kprop->flags & DRM_MODE_PROP_ENUM) {
			for (j = 0; j < p->kprop->count_enums; j++) {
				if (p->kprop->enums[j].value == sna_output->prop_values[i])
					break;
			}
			err = RRChangeOutputProperty(output->randr_output,
						     property, XA_ATOM, 32,
						     PropModeReplace, 1,
						     &p->atoms[j+1],
						     FALSE, FALSE);
		}

		if (err != 0)
			xf86DrvMsg(output->scrn->scrnIndex, X_WARNING,
				   "RRChangeOutputProperty error, %d\n", err);
		return TRUE;
	}

	return FALSE;
}

static const xf86OutputFuncsRec sna_output_funcs = {
	.create_resources = sna_output_create_resources,
#ifdef RANDR_12_INTERFACE
	.set_property = sna_output_set_property,
	.get_property = sna_output_get_property,
#endif
	.dpms = sna_output_dpms,
	.detect = sna_output_detect,
	.mode_valid = sna_output_mode_valid,

	.get_modes = sna_output_get_modes,
	.destroy = sna_output_destroy
};

static const int subpixel_conv_table[] = {
	SubPixelUnknown,
	SubPixelHorizontalRGB,
	SubPixelHorizontalBGR,
	SubPixelVerticalRGB,
	SubPixelVerticalBGR,
	SubPixelNone
};

static const char * const output_names[] = {
	/* DRM_MODE_CONNECTOR_Unknown */	"None",
	/* DRM_MODE_CONNECTOR_VGA */		"VGA",
	/* DRM_MODE_CONNECTOR_DVII */		"DVI",
	/* DRM_MODE_CONNECTOR_DVID */		"DVI",
	/* DRM_MODE_CONNECTOR_DVIA */		"DVI",
	/* DRM_MODE_CONNECTOR_Composite */	"Composite",
	/* DRM_MODE_CONNECTOR_SVIDEO */		"TV",
	/* DRM_MODE_CONNECTOR_LVDS */		"LVDS",
	/* DRM_MODE_CONNECTOR_Component */	"CTV",
	/* DRM_MODE_CONNECTOR_9PinDIN */	"DIN",
	/* DRM_MODE_CONNECTOR_DisplayPort */	"DP",
	/* DRM_MODE_CONNECTOR_HDMIA */		"HDMI",
	/* DRM_MODE_CONNECTOR_HDMIB */		"HDMI",
	/* DRM_MODE_CONNECTOR_TV */		"TV",
	/* DRM_MODE_CONNECTOR_eDP */		"eDP",
	/* DRM_MODE_CONNECTOR_VIRTUAL */	"Virtual",
	/* DRM_MODE_CONNECTOR_DSI */		"DSI",
	/* DRM_MODE_CONNECTOR_DPI */		"DPI"
};

static bool
output_ignored(ScrnInfoPtr scrn, const char *name)
{
	char monitor_name[64];
	const char *monitor;
	XF86ConfMonitorPtr conf;

	snprintf(monitor_name, sizeof(monitor_name), "monitor-%s", name);
	monitor = xf86findOptionValue(scrn->options, monitor_name);
	if (!monitor)
		monitor = name;

	conf = xf86findMonitor(monitor,
			       xf86configptr->conf_monitor_lst);
	if (conf == NULL && XF86_CRTC_CONFIG_PTR(scrn)->num_output == 0)
		conf = xf86findMonitor(scrn->monitor->id,
				       xf86configptr->conf_monitor_lst);
	if (conf == NULL)
		return false;

	return xf86CheckBoolOption(conf->mon_option_lst, "Ignore", 0);
}

static bool
gather_encoders(struct sna *sna, uint32_t id, int count,
		struct drm_mode_get_encoder *out)
{
	union compat_mode_get_connector compat_conn;
	struct drm_mode_modeinfo dummy;
	struct drm_mode_get_encoder enc;
	uint32_t *ids = NULL;

	DBG(("%s(%d): expected count=%d\n", __FUNCTION__, id, count));

	VG_CLEAR(compat_conn);
	VG_CLEAR(enc);
	memset(out, 0, sizeof(*out));

	do {
		uint32_t *nids;

		nids = realloc(ids, sizeof(*ids) * count);
		if (nids == NULL) {
			free(ids);
			return false;
		}
		ids = nids;

		compat_conn.conn.connector_id = id;
		compat_conn.conn.count_props = 0;
		compat_conn.conn.count_modes = 1; /* skip detect */
		compat_conn.conn.modes_ptr = (uintptr_t)&dummy;
		compat_conn.conn.count_encoders = count;
		compat_conn.conn.encoders_ptr = (uintptr_t)ids;

		if (drmIoctl(sna->kgem.fd, DRM_IOCTL_MODE_GETCONNECTOR, &compat_conn.conn)) {
			DBG(("%s: GETCONNECTOR[%d] failed, ret=%d\n", __FUNCTION__, id, errno));
			compat_conn.conn.count_encoders = count = 0;
		}

		VG(VALGRIND_MAKE_MEM_DEFINED(ids, sizeof(uint32_t)*compat_conn.conn.count_encoders));
		if (count == compat_conn.conn.count_encoders)
			break;

		count = compat_conn.conn.count_encoders;
	} while (1);

	DBG(("%s(%d): gathering %d encoders\n", __FUNCTION__, id, count));
	for (count = 0; count < compat_conn.conn.count_encoders; count++) {
		enc.encoder_id = ids[count];
		if (drmIoctl(sna->kgem.fd, DRM_IOCTL_MODE_GETENCODER, &enc)) {
			DBG(("%s: GETENCODER[%d] failed, ret=%d\n", __FUNCTION__, ids[count], errno));
			count = 0;
			break;
		}
		DBG(("%s(%d): encoder=%d, possible_crtcs=%x, possible_clones=%x\n",
		     __FUNCTION__, id, enc.encoder_id, enc.possible_crtcs, enc.possible_clones));
		out->possible_crtcs |= enc.possible_crtcs;
		out->possible_clones |= enc.possible_clones;

		for (id = 0; id < sna->mode.num_real_encoder; id++) {
			if (enc.encoder_id == sna->mode.encoders[id]) {
				out->crtc_id |= 1 << id;
				break;
			}
		}
	}

	free(ids);
	return count > 0;
}

/* We need to map from kms encoder based possible_clones mask to X output based
 * possible clones masking. Note that for SDVO and on Haswell with DP/HDMI we
 * can have more than one output hanging off the same encoder.
 */
static void
sna_mode_compute_possible_outputs(struct sna *sna)
{
	xf86CrtcConfigPtr config = XF86_CRTC_CONFIG_PTR(sna->scrn);
	int encoder_mask[32];
	int i, j;

	assert(sna->mode.num_real_output < 32);
	assert(sna->mode.num_real_crtc < 32);

	for (i = 0; i < sna->mode.num_real_output; i++) {
		xf86OutputPtr output = config->output[i];
		struct sna_output *sna_output = to_sna_output(output);

		assert(sna_output);

		if (sna_output->id) {
			output->possible_clones = sna_output->possible_encoders;
			encoder_mask[i] = sna_output->attached_encoders;
		} else {
			output->possible_clones = 0;
			encoder_mask[i] = 0;
		}
	}

	/* Convert from encoder numbering to output numbering */
	for (i = 0; i < sna->mode.num_real_output; i++) {
		xf86OutputPtr output = config->output[i];
		unsigned clones;

		if (output->possible_clones == 0)
			continue;

		clones = 0;
		for (j = 0; j < sna->mode.num_real_output; j++)
			if (i != j && output->possible_clones & encoder_mask[j])
				clones |= 1 << j;
		output->possible_clones = clones;

		DBG(("%s: updated output '%s' %d [%d] (possible crtc:%x, possible clones:%x)\n",
		     __FUNCTION__, output->name, i, to_connector_id(output),
		     (uint32_t)output->possible_crtcs,
		     (uint32_t)output->possible_clones));
	}
}

static int name_from_path(struct sna *sna,
			  struct sna_output *sna_output,
			  char *name)
{
	struct drm_mode_get_blob blob;
	char *path;
	int id;

	id = find_property(sna, sna_output, "PATH");
	DBG(("%s: found? PATH=%d\n", __FUNCTION__, id));
	if (id == -1)
		return 0;

	VG_CLEAR(blob);
	blob.blob_id = sna_output->prop_values[id];
	blob.length = 0;
	if (drmIoctl(sna->kgem.fd, DRM_IOCTL_MODE_GETPROPBLOB, &blob))
		return 0;

	do {
		id = blob.length;
		path = alloca(id + 1);
		blob.data = (uintptr_t)path;
		VG(memset(path, 0, id));
		DBG(("%s: reading %d bytes for path blob\n", __FUNCTION__, id));
		if (drmIoctl(sna->kgem.fd, DRM_IOCTL_MODE_GETPROPBLOB, &blob))
			return 0;
	} while (id != blob.length);

	path[blob.length] = '\0'; /* paranoia */
	DBG(("%s: PATH='%s'\n", __FUNCTION__, path));

	/* we only handle MST paths for now */
	if (strncmp(path, "mst:", 4) == 0) {
		xf86CrtcConfigPtr config = XF86_CRTC_CONFIG_PTR(sna->scrn);
		char tmp[5], *c;
		int n;

		c = strchr(path + 4, '-');
		if (c == NULL)
			return 0;

		id = c - (path + 4);
		if (id + 1> 5)
			return 0;

		memcpy(tmp, path + 4, id);
		tmp[id] = '\0';
		id = strtoul(tmp, NULL, 0);

		for (n = 0; n < sna->mode.num_real_output; n++) {
			if (to_sna_output(config->output[n])->id == id)
				return snprintf(name, 32, "%s-%s",
						config->output[n]->name, c + 1);
		}
	}

	return 0;
}

static char *fake_edid_name(xf86OutputPtr output)
{
	struct sna *sna = to_sna(output->scrn);
	const char *str, *colon;

#if XORG_VERSION_CURRENT >= XORG_VERSION_NUMERIC(1,7,99,901,0)
	str = xf86GetOptValString(sna->Options, OPTION_EDID);
#else
	str = NULL;
#endif
	if (str == NULL)
		return NULL;

	do {
		colon = strchr(str, ':');
		if (colon == NULL)
			return NULL;

		if (strncmp(str, output->name, colon-str) == 0 &&
		    output->name[colon-str] == '\0') {
			char *path;
			int len;

			str = colon + 1;
			colon = strchr(str, ',');
			if (colon)
				len = colon - str;
			else
				len = strlen(str);

			path = malloc(len + 1);
			if (path == NULL)
				return NULL;

			memcpy(path, str, len);
			path[len] = '\0';
			return path;
		}

		str = strchr(colon + 1, ',');
		if (str == NULL)
			return NULL;

		str++;
	} while (1);
}

static void
sna_output_load_fake_edid(xf86OutputPtr output)
{
	struct sna_output *sna_output = output->driver_private;
	const char *filename;
	FILE *file;
	void *raw;
	int size;

	filename = fake_edid_name(output);
	if (filename == NULL)
		return;

	file = fopen(filename, "rb");
	if (file == NULL)
		goto err;

	fseek(file, 0, SEEK_END);
	size = ftell(file);
	if (size % 128) {
		fclose(file);
		goto err;
	}

	raw = malloc(size);
	if (raw == NULL) {
		fclose(file);
		free(raw);
		goto err;
	}

	fseek(file, 0, SEEK_SET);
	if (fread(raw, size, 1, file) != 1) {
		fclose(file);
		free(raw);
		goto err;
	}
	fclose(file);

	sna_output->fake_edid_raw = raw;

	xf86DrvMsg(output->scrn->scrnIndex, X_CONFIG,
		   "Loading EDID from \"%s\" for output %s\n",
		   filename, output->name);
	return;

err:
	xf86DrvMsg(output->scrn->scrnIndex, X_ERROR,
		   "Could not read EDID file \"%s\" for output %s\n",
		   filename, output->name);
}

static int
sna_output_add(struct sna *sna, unsigned id, unsigned serial)
{
	ScrnInfoPtr scrn = sna->scrn;
	xf86CrtcConfigPtr config = XF86_CRTC_CONFIG_PTR(scrn);
	union compat_mode_get_connector compat_conn;
	struct drm_mode_get_encoder enc;
	struct drm_mode_modeinfo dummy;
	struct sna_output *sna_output;
	xf86OutputPtr *outputs, output;
	unsigned possible_encoders, attached_encoders, possible_crtcs;
	const char *output_name;
	char name[32];
	int path, len, i;

	DBG(("%s(%d): serial=%d\n", __FUNCTION__, id, serial));

	COMPILE_TIME_ASSERT(sizeof(struct drm_mode_get_connector) <= sizeof(compat_conn.pad));

	VG_CLEAR(compat_conn);
	memset(&enc, 0, sizeof(enc));

	compat_conn.conn.connector_id = id;
	compat_conn.conn.count_props = 0;
	compat_conn.conn.count_modes = 1; /* skip detect */
	compat_conn.conn.modes_ptr = (uintptr_t)&dummy;
	compat_conn.conn.count_encoders = 1;
	compat_conn.conn.encoders_ptr = (uintptr_t)&enc.encoder_id;

	if (drmIoctl(sna->kgem.fd, DRM_IOCTL_MODE_GETCONNECTOR, &compat_conn.conn)) {
		DBG(("%s: GETCONNECTOR[%d] failed, ret=%d\n", __FUNCTION__, id, errno));
		return -1;
	}
	assert(compat_conn.conn.connector_id == id);
	DBG(("%s(%d): has %d associated encoders\n", __FUNCTION__, id, compat_conn.conn.count_encoders));

	if (compat_conn.conn.connector_type < ARRAY_SIZE(output_names))
		output_name = output_names[compat_conn.conn.connector_type];
	else
		output_name = "UNKNOWN";
	len = snprintf(name, 32, "%s%d", output_name, compat_conn.conn.connector_type_id);
	if (output_ignored(scrn, name))
		return 0;

	if (enc.encoder_id) {
		if (drmIoctl(sna->kgem.fd, DRM_IOCTL_MODE_GETENCODER, &enc)) {
			DBG(("%s: GETENCODER[%d] failed, ret=%d\n", __FUNCTION__, enc.encoder_id, errno));
			return 0;
		}

		possible_encoders = enc.possible_clones;
		attached_encoders = 0;
		for (i = 0; i < sna->mode.num_real_encoder; i++) {
			if (enc.encoder_id == sna->mode.encoders[i]) {
				attached_encoders = 1 << i;
				break;
			}
		}

		if (attached_encoders == 0) {
			DBG(("%s: failed to find attached encoder\n", __FUNCTION__));
			return 0;
		}

		possible_crtcs = enc.possible_crtcs;
		assert(enc.encoder_id == compat_conn.conn.encoder_id || compat_conn.conn.encoder_id == 0);
	} else {
		DBG(("%s: unexpected number [%d] of encoders attached\n",
		     __FUNCTION__, compat_conn.conn.count_encoders));
		if (!gather_encoders(sna, id, compat_conn.conn.count_encoders, &enc)) {
			DBG(("%s: gather encoders failed\n", __FUNCTION__));
			return 0;
		}
		possible_encoders = enc.possible_clones;
		attached_encoders = enc.crtc_id;
		possible_crtcs = enc.possible_crtcs;

		memset(&enc, 0, sizeof(enc));
		enc.encoder_id = compat_conn.conn.encoder_id;
		(void)drmIoctl(sna->kgem.fd, DRM_IOCTL_MODE_GETENCODER, &enc);
	}

	if (is_zaphod(scrn)) {
		unsigned zaphod_crtcs;

		if (!sna_zaphod_match(sna, name)) {
			DBG(("%s: zaphod mismatch, want %s, have %s\n",
			     __FUNCTION__,
			     xf86GetOptValString(sna->Options, OPTION_ZAPHOD) ?: "???",
			     name));
			return 0;
		}

		zaphod_crtcs = get_zaphod_crtcs(sna);
		possible_crtcs &= zaphod_crtcs;
		if (possible_crtcs == 0) {
			xf86DrvMsg(scrn->scrnIndex, X_ERROR,
				   "%s is an invalid output for screen %d\n",
				   name, scrn->confScreen->device->screen);
			return -1;
		}

		possible_crtcs >>= ffs(zaphod_crtcs) - 1;
	}

	sna_output = calloc(sizeof(struct sna_output), 1);
	if (!sna_output)
		return -1;

	sna_output->connector_type = compat_conn.conn.connector_type;
	sna_output->connector_type_id = compat_conn.conn.connector_type_id;
	sna_output->num_props = compat_conn.conn.count_props;
	sna_output->prop_ids = malloc(sizeof(uint32_t)*compat_conn.conn.count_props);
	sna_output->prop_values = malloc(sizeof(uint64_t)*compat_conn.conn.count_props);
	if (sna_output->prop_ids == NULL || sna_output->prop_values == NULL) {
		free(sna_output->prop_ids);
		free(sna_output->prop_values);
		free(sna_output);
		return -1;
	}

	compat_conn.conn.count_encoders = 0;

	compat_conn.conn.count_modes = 1;
	compat_conn.conn.modes_ptr = (uintptr_t)&dummy;

	compat_conn.conn.count_props = sna_output->num_props;
	compat_conn.conn.props_ptr = (uintptr_t)sna_output->prop_ids;
	compat_conn.conn.prop_values_ptr = (uintptr_t)sna_output->prop_values;

	if (drmIoctl(sna->kgem.fd, DRM_IOCTL_MODE_GETCONNECTOR, &compat_conn.conn)) {
		DBG(("%s: second! GETCONNECTOR failed, ret=%d\n", __FUNCTION__, errno));
		goto cleanup;
	}
	assert(compat_conn.conn.connector_id == id);

	/* statically constructed property list */
	assert(sna_output->num_props == compat_conn.conn.count_props);
	VG(VALGRIND_MAKE_MEM_DEFINED(sna_output->prop_ids, sizeof(uint32_t)*sna_output->num_props));
	VG(VALGRIND_MAKE_MEM_DEFINED(sna_output->prop_values, sizeof(uint64_t)*sna_output->num_props));

	/* Construct name from topology, and recheck if output is acceptable */
	path = name_from_path(sna, sna_output, name);
	if (path) {
		if (output_ignored(scrn, name)) {
			len = 0;
			goto skip;
		}

		if (is_zaphod(scrn) && !sna_zaphod_match(sna, name)) {
			DBG(("%s: zaphod mismatch, want %s, have %s\n",
			     __FUNCTION__,
			     xf86GetOptValString(sna->Options, OPTION_ZAPHOD) ?: "???",
			     name));
			len = 0;
			goto skip;
		}

		len = path;
	}

	/* Check if we are dynamically reattaching an old connector */
	if (serial) {
		for (i = 0; i < sna->mode.num_real_output; i++) {
			output = config->output[i];
			if (strcmp(output->name, name) == 0) {
				assert(output->scrn == scrn);
				assert(output->funcs == &sna_output_funcs);

				/*
				 * If the old output is still in use, tell
				 * the kernel to switch it off so we can
				 * move its resources over to the new id.
				 */
				if (output->crtc) {
					struct drm_mode_crtc arg = {
						.crtc_id = __sna_crtc_id(to_sna_crtc(output->crtc)),
					};
					drmIoctl(sna->kgem.fd, DRM_IOCTL_MODE_SETCRTC, &arg);
					output->crtc = NULL;
				}

				sna_output_destroy(output);
				goto reset;
			}
		}
	}

	output = calloc(1, sizeof(*output) + len + 1);
	if (!output)
		goto cleanup;

	outputs = realloc(config->output, (config->num_output + 1) * sizeof(output));
	if (outputs == NULL) {
		free(output);
		goto cleanup;
	}

	output->scrn = scrn;
	output->funcs = &sna_output_funcs;
	output->name = (char *)(output + 1);
	memcpy(output->name, name, len + 1);

	output->use_screen_monitor = config->num_output != 0;
	xf86OutputUseScreenMonitor(output, !output->use_screen_monitor);
	assert(output->options);

	DBG(("%s: inserting output #%d of %d\n", __FUNCTION__, sna->mode.num_real_output, config->num_output));
	for (i = config->num_output; i > sna->mode.num_real_output; i--) {
		outputs[i] = outputs[i-1];
		assert(outputs[i]->driver_private == NULL);
		outputs[i]->possible_clones <<= 1;
	}

	if (xf86ReturnOptValBool(output->options, OPTION_PRIMARY, FALSE)) {
		memmove(outputs + 1, outputs, sizeof(output)*config->num_output);
		outputs[0] = output;
	} else
		outputs[i] = output;
	sna->mode.num_real_output++;
	config->num_output++;
	config->output = outputs;

reset:
	sna_output->id = compat_conn.conn.connector_id;
	sna_output->is_panel = is_panel(compat_conn.conn.connector_type);
	sna_output->edid_idx = find_property(sna, sna_output, "EDID");
	sna_output->link_status_idx =
		find_property(sna, sna_output, "link-status");
	if (find_property(sna, sna_output, "scaling mode") != -1)
		sna_output->add_default_modes =
			xf86ReturnOptValBool(output->options, OPTION_DEFAULT_MODES, TRUE);

	i = find_property(sna, sna_output, "DPMS");
	if (i != -1) {
		sna_output->dpms_id = sna_output->prop_ids[i];
		sna_output->dpms_mode = sna_output->prop_values[i];
		DBG(("%s: found 'DPMS' (idx=%d, id=%d), initial value=%d\n",
		     __FUNCTION__, i, sna_output->dpms_id, sna_output->dpms_mode));
	} else
		sna_output->dpms_mode = DPMSModeOff;

	sna_output->possible_encoders = possible_encoders;
	sna_output->attached_encoders = attached_encoders;

	output->mm_width = compat_conn.conn.mm_width;
	output->mm_height = compat_conn.conn.mm_height;

	if (compat_conn.conn.subpixel >= ARRAY_SIZE(subpixel_conv_table))
		compat_conn.conn.subpixel = 0;
	output->subpixel_order = subpixel_conv_table[compat_conn.conn.subpixel];
	output->driver_private = sna_output;
	sna_output->base = output;

	backlight_init(&sna_output->backlight);
	sna_output_backlight_init(output);

	output->possible_crtcs = possible_crtcs & count_to_mask(sna->mode.num_real_crtc);
	output->interlaceAllowed = TRUE;

	sna_output_load_fake_edid(output);

	if (serial) {
		if (output->randr_output == NULL) {
			output->randr_output = RROutputCreate(xf86ScrnToScreen(scrn), name, len, output);
			if (output->randr_output == NULL)
				goto cleanup;
		}

		RROutputChanged(output->randr_output, TRUE);
		sna_output_create_resources(output);
		RRPostPendingProperties(output->randr_output);

		sna_output->serial = serial;
	} else {
		/* stash the active CRTC id for our probe function */
		if (compat_conn.conn.connection != DRM_MODE_DISCONNECTED)
			output->crtc = (void *)(uintptr_t)enc.crtc_id;
	}

	DBG(("%s: created output '%s' %d, encoder=%d (possible crtc:%x, attached encoders:%x, possible clones:%x), serial=%d, edid=%d, dpms=%d, crtc=%lu\n",
	     __FUNCTION__, name, id, enc.encoder_id,
	     (uint32_t)output->possible_crtcs,
	     sna_output->attached_encoders,
	     sna_output->possible_encoders,
	     serial, sna_output->edid_idx, sna_output->dpms_id,
	     (unsigned long)(uintptr_t)output->crtc));
	assert(sna_output->id == id);

	xf86DrvMsg(scrn->scrnIndex, X_INFO,
		   "Enabled output %s\n",
		   output->name);
	return 1;

cleanup:
	len = -1;
skip:
	free(sna_output->prop_ids);
	free(sna_output->prop_values);
	free(sna_output);
	return len;
}

static int output_rank(const void *A, const void *B)
{
	const xf86OutputPtr *a = A;
	const xf86OutputPtr *b = B;
	struct sna_output *sa = to_sna_output(*a);
	struct sna_output *sb = to_sna_output(*b);

	if (sa->is_panel != sb->is_panel)
		return sb->is_panel - sa->is_panel;

	return strcmp((*a)->name, (*b)->name);
}

static void sort_config_outputs(struct sna *sna)
{
	xf86CrtcConfigPtr config = XF86_CRTC_CONFIG_PTR(sna->scrn);
	qsort(config->output, sna->mode.num_real_output, sizeof(*config->output), output_rank);
	config->compat_output = 0; /* make sure it is a sane value */
	sna_mode_compute_possible_outputs(sna);
}

static void sort_randr_outputs(struct sna *sna, ScreenPtr screen)
{
	xf86CrtcConfigPtr config = XF86_CRTC_CONFIG_PTR(sna->scrn);
	rrScrPriv(screen);
	int i;

	assert(pScrPriv->numOutputs == config->num_output);
	for (i = 0; i < config->num_output; i++) {
		assert(config->output[i]->randr_output);
		pScrPriv->outputs[i] = config->output[i]->randr_output;
	}
}

static bool disable_unused_crtc(struct sna *sna)
{
	xf86CrtcConfigPtr config = XF86_CRTC_CONFIG_PTR(sna->scrn);
	bool update = false;
	int o, c;

	DBG(("%s\n", __FUNCTION__));

	for (c = 0; c < sna->mode.num_real_crtc; c++) {
		xf86CrtcPtr crtc = config->crtc[c];

		if (!crtc->enabled) {
			sna_crtc_disable(crtc, false);
			continue;
		}

		for (o = 0; o < sna->mode.num_real_output; o++) {
			xf86OutputPtr output = config->output[o];
			if (output->crtc == crtc)
				break;
		}

		if (o == sna->mode.num_real_output) {
			DBG(("%s: CRTC:%d was enabled with no outputs\n",
			     __FUNCTION__, sna_crtc_id(crtc)));
			crtc->enabled = false;
			update = true;
		}
	}

	if (update) {
		DBG(("%s: disabling unused functions\n", __FUNCTION__));
		xf86DisableUnusedFunctions(sna->scrn);
	}

	return update;
}

bool sna_mode_find_hotplug_connector(struct sna *sna, unsigned id)
{
	xf86CrtcConfigPtr config = XF86_CRTC_CONFIG_PTR(sna->scrn);
	int i;

	for (i = 0; i < sna->mode.num_real_output; i++) {
		struct sna_output *output = to_sna_output(config->output[i]);
		if (output->id == id) {
			output->reprobe = true;
			return true;
		}
	}

	return false;
}

static bool
output_retrain_link(struct sna *sna, struct sna_output *output)
{
	struct sna_crtc *crtc = to_sna_crtc(output->base->crtc);
	int crtc_x = crtc->offset & 0xffff;
	int crtc_y = crtc->offset >> 16;

	if (!crtc->bo)
		return false;

	return sna_crtc_flip(sna, crtc, crtc->bo, crtc_x, crtc_y);
}

static bool
output_check_link(struct sna *sna, struct sna_output *output)
{
	uint64_t link_status;

	if (!output->base->crtc)
		return true;

	if (output->link_status_idx == -1)
		return true;

#define LINK_STATUS_GOOD 0
	link_status = output->prop_values[output->link_status_idx];
	DBG(("%s: link_status=%d\n", __FUNCTION__, link_status));
	if (link_status == LINK_STATUS_GOOD)
		return true;

	/* Perform a modeset as required for "link-status" = BAD */
	if (!output_retrain_link(sna, output))
		return false;

	/* Query the "link-status" again to confirm the modeset */
	update_properties(sna, output);

	link_status = output->prop_values[output->link_status_idx];
	DBG(("%s: link_status=%d after modeset\n", __FUNCTION__, link_status));
	return link_status == LINK_STATUS_GOOD;
}

static bool
output_check_status(struct sna *sna, struct sna_output *output)
{
	union compat_mode_get_connector compat_conn;
	struct drm_mode_modeinfo dummy;
	struct drm_mode_get_blob blob;
	xf86OutputStatus status;
	char *edid;

	VG_CLEAR(compat_conn);

	compat_conn.conn.connection = -1;
	compat_conn.conn.connector_id = output->id;
	compat_conn.conn.count_modes = 1; /* skip detect */
	compat_conn.conn.modes_ptr = (uintptr_t)&dummy;
	compat_conn.conn.count_encoders = 0;
	compat_conn.conn.props_ptr = (uintptr_t)output->prop_ids;
	compat_conn.conn.prop_values_ptr = (uintptr_t)output->prop_values;
	compat_conn.conn.count_props = output->num_props;

	if (drmIoctl(sna->kgem.fd,
		     DRM_IOCTL_MODE_GETCONNECTOR,
		     &compat_conn.conn) == 0)
		output->update_properties = false;

	if (!output_check_link(sna, output))
		return false;

	if (output->reprobe)
		return false;

	switch (compat_conn.conn.connection) {
	case DRM_MODE_CONNECTED:
		status = XF86OutputStatusConnected;
		break;
	case DRM_MODE_DISCONNECTED:
		status = XF86OutputStatusDisconnected;
		break;
	default:
	case DRM_MODE_UNKNOWNCONNECTION:
		status = XF86OutputStatusUnknown;
		break;
	}
	if (output->status != status)
		return false;

	if (status != XF86OutputStatusConnected)
		return true;

	if (output->num_modes != compat_conn.conn.count_modes)
		return false;

	if (output->edid_len == 0)
		return false;

	edid = alloca(output->edid_len);

	VG_CLEAR(blob);
	blob.blob_id = output->prop_values[output->edid_idx];
	blob.length = output->edid_len;
	blob.data = (uintptr_t)edid;
	if (drmIoctl(sna->kgem.fd, DRM_IOCTL_MODE_GETPROPBLOB, &blob))
		return false;

	if (blob.length != output->edid_len)
		return false;

	return memcmp(edid, output->edid_raw, output->edid_len) == 0;
}

void sna_mode_discover(struct sna *sna, bool tell)
{
	ScreenPtr screen = xf86ScrnToScreen(sna->scrn);
	xf86CrtcConfigPtr config = XF86_CRTC_CONFIG_PTR(sna->scrn);
	bool force = sna->flags & SNA_REPROBE;
	struct drm_mode_card_res res;
	uint32_t connectors[32], now;
	unsigned changed = 0;
	unsigned serial;
	int i, j;

	DBG(("%s()\n", __FUNCTION__));
	sna->flags &= ~SNA_REPROBE;

	VG_CLEAR(connectors);

	memset(&res, 0, sizeof(res));
	res.count_connectors = 32;
	res.connector_id_ptr = (uintptr_t)connectors;

	if (drmIoctl(sna->kgem.fd, DRM_IOCTL_MODE_GETRESOURCES, &res))
		return;

	DBG(("%s: now %d (was %d) connectors, %d encoders, %d crtc\n", __FUNCTION__,
	     res.count_connectors, sna->mode.num_real_output,
	     res.count_encoders, res.count_crtcs));
	if (res.count_connectors > 32)
		res.count_connectors = 32;

	assert(sna->mode.num_real_crtc == res.count_crtcs || is_zaphod(sna->scrn));
	assert(sna->mode.max_crtc_width  == res.max_width);
	assert(sna->mode.max_crtc_height == res.max_height);
	assert(sna->mode.num_real_encoder == res.count_encoders);

	serial = ++sna->mode.serial;
	if (serial == 0)
		serial = ++sna->mode.serial;

	if (force) {
		changed = 4;
		now = 0;
	} else
		now = GetTimeInMillis();
	for (i = 0; i < res.count_connectors; i++) {
		DBG(("%s: connector[%d] = %d\n", __FUNCTION__, i, connectors[i]));
		for (j = 0; j < sna->mode.num_real_output; j++) {
			xf86OutputPtr output = config->output[j];
			if (to_sna_output(output)->id == connectors[i]) {
				DBG(("%s: found %s (id=%d)\n", __FUNCTION__, output->name, connectors[i]));
				assert(to_sna_output(output)->id);
				to_sna_output(output)->serial = serial;
				break;
			}
		}
		if (j == sna->mode.num_real_output) {
			DBG(("%s: adding id=%d\n", __FUNCTION__, connectors[i]));
			changed |= sna_output_add(sna, connectors[i], serial) > 0;
		}
	}

	for (i = 0; i < sna->mode.num_real_output; i++) {
		xf86OutputPtr output = config->output[i];
		struct sna_output *sna_output = to_sna_output(output);

		if (sna_output->id == 0)
			continue;

		if (sna_output->serial == serial) {
			if (output_check_status(sna, sna_output)) {
				DBG(("%s: output %s (id=%d), retained state\n",
				     __FUNCTION__, output->name, sna_output->id));
				sna_output->last_detect = now;
			} else {
				DBG(("%s: output %s (id=%d), changed state, reprobing\n",
				     __FUNCTION__, output->name, sna_output->id));
				sna_output->hotplug_count++;
				sna_output->last_detect = 0;
				changed |= 4;
			}
			continue;
		}

		DBG(("%s: removing output %s (id=%d), serial=%u [now %u]\n",
		     __FUNCTION__, output->name, sna_output->id,
		    sna_output->serial, serial));

		xf86DrvMsg(sna->scrn->scrnIndex, X_INFO,
			   "Disabled output %s\n",
			   output->name);
		sna_output->id = 0;
		sna_output->last_detect = 0;
		output->crtc = NULL;
		RROutputChanged(output->randr_output, TRUE);
		changed |= 2;
	}

	/* Have the list of available outputs been updated? */
	if (changed & 3) {
		DBG(("%s: outputs changed, broadcasting\n", __FUNCTION__));

		sna_mode_set_primary(sna);

		/* Reorder user visible listing */
		sort_config_outputs(sna);
		sort_randr_outputs(sna, screen);

		if (changed & 2)
			disable_unused_crtc(sna);

		xf86RandR12TellChanged(screen);
	}

	/* If anything has changed, refresh the RandR information.
	 * Note this could recurse once from udevless RRGetInfo() probes,
	 * but only once.
	 */
	if (changed && tell)
		RRGetInfo(screen, TRUE);
}

/* Since we only probe the current mode on startup, we may not have the full
 * list of modes available until the user explicitly requests them. Fake a
 * hotplug event after a second after starting to fill in any missing modes.
 */
CARD32 sna_mode_coldplug(OsTimerPtr timer, CARD32 now, void *data)
{
	struct sna *sna = data;
	ScreenPtr screen = xf86ScrnToScreen(sna->scrn);
	xf86CrtcConfigPtr config = XF86_CRTC_CONFIG_PTR(sna->scrn);
	bool reprobe = false;
	int i;

	DBG(("%s()\n", __FUNCTION__));

	for (i = 0; i < sna->mode.num_real_output; i++) {
		xf86OutputPtr output = config->output[i];
		struct sna_output *sna_output = to_sna_output(output);

		if (sna_output->id == 0)
			continue;
		if (sna_output->last_detect)
			continue;
		if (output->status == XF86OutputStatusDisconnected)
			continue;

		DBG(("%s: output %s connected, needs reprobe\n",
		     __FUNCTION__, output->name));
		reprobe = true;
	}

	if (reprobe) {
		RRGetInfo(screen, TRUE);
		RRTellChanged(screen);
	}
	free(timer);
	return 0;
}

static void copy_front(struct sna *sna, PixmapPtr old, PixmapPtr new)
{
	struct sna_pixmap *old_priv, *new_priv;

	DBG(("%s\n", __FUNCTION__));

	if (wedged(sna) || isGPU(sna->scrn))
		return;

	old_priv = sna_pixmap_force_to_gpu(old, MOVE_READ);
	if (!old_priv)
		return;

	new_priv = sna_pixmap_force_to_gpu(new, MOVE_WRITE | __MOVE_SCANOUT);
	if (!new_priv)
		return;

	if (old_priv->clear) {
		bool ok = false;
		if (!wedged(sna))
			ok = sna->render.fill_one(sna, new, new_priv->gpu_bo,
						  old_priv->clear_color,
						  0, 0,
						  new->drawable.width,
						  new->drawable.height,
						  GXcopy);
		if (!ok) {
			void *ptr = kgem_bo_map__gtt(&sna->kgem, new_priv->gpu_bo);
			if (ptr)
				memset(ptr, 0, new_priv->gpu_bo->pitch*new->drawable.height);
		}
		new_priv->clear = true;
		new_priv->clear_color = old_priv->clear_color;
	} else {
		BoxRec box;
		int16_t sx, sy, dx, dy;

		if (new->drawable.width >= old->drawable.width &&
		    new->drawable.height >= old->drawable.height)
		{
			int nx = (new->drawable.width + old->drawable.width - 1) / old->drawable.width;
			int ny = (new->drawable.height + old->drawable.height - 1) / old->drawable.height;

			box.x1 = box.y1 = 0;

			dy = 0;
			for (sy = 0; sy < ny; sy++) {
				box.y2 = old->drawable.height;
				if (box.y2 + dy > new->drawable.height)
					box.y2 = new->drawable.height - dy;

				dx = 0;
				for (sx = 0; sx < nx; sx++) {
					box.x2 = old->drawable.width;
					if (box.x2 + dx > new->drawable.width)
						box.x2 = new->drawable.width - dx;

					(void)sna->render.copy_boxes(sna, GXcopy,
								     &old->drawable, old_priv->gpu_bo, 0, 0,
								     &new->drawable, new_priv->gpu_bo, dx, dy,
								     &box, 1, 0);
					dx += old->drawable.width;
				}
				dy += old->drawable.height;
			}
		} else {
			box.x1 = box.y1 = 0;
			box.x2 = min(old->drawable.width, new->drawable.width);
			box.y2 = min(old->drawable.height, new->drawable.height);

			sx = dx = 0;
			if (box.x2 < old->drawable.width)
				sx = (old->drawable.width - box.x2) / 2;
			if (box.x2 < new->drawable.width)
				dx = (new->drawable.width - box.x2) / 2;

			sy = dy = 0;
			if (box.y2 < old->drawable.height)
				sy = (old->drawable.height - box.y2) / 2;
			if (box.y2 < new->drawable.height)
				dy = (new->drawable.height - box.y2) / 2;

			DBG(("%s: copying box (%dx%d) from (%d, %d) to (%d, %d)\n",
			     __FUNCTION__, box.x2, box.y2, sx, sy, dx, dy));

			if (box.x2 != new->drawable.width || box.y2 != new->drawable.height) {
				bool ok = false;
				if (!wedged(sna))
					ok = sna->render.fill_one(sna, new, new_priv->gpu_bo, 0,
								  0, 0,
								  new->drawable.width,
								  new->drawable.height,
								  GXclear);
				if (!ok) {
					void *ptr = kgem_bo_map__gtt(&sna->kgem, new_priv->gpu_bo);
					if (ptr)
						memset(ptr, 0, new_priv->gpu_bo->pitch*new->drawable.height);
				}
			}
			(void)sna->render.copy_boxes(sna, GXcopy,
						     &old->drawable, old_priv->gpu_bo, sx, sy,
						     &new->drawable, new_priv->gpu_bo, dx, dy,
						     &box, 1, 0);
		}
	}

	sna_damage_all(&new_priv->gpu_damage, new);
}

static Bool
sna_mode_resize(ScrnInfoPtr scrn, int width, int height)
{
	xf86CrtcConfigPtr config = XF86_CRTC_CONFIG_PTR(scrn);
	struct sna *sna = to_sna(scrn);
	ScreenPtr screen = xf86ScrnToScreen(scrn);
	PixmapPtr new_front;
	int i;

	DBG(("%s (%d, %d) -> (%d, %d)\n", __FUNCTION__,
	     scrn->virtualX, scrn->virtualY,
	     width, height));
	assert((sna->flags & SNA_IS_HOSTED) == 0);

	if (scrn->virtualX == width && scrn->virtualY == height)
		return TRUE;

	/* Paranoid defense against rogue internal calls by Xorg */
	if (width == 0 || height == 0)
		return FALSE;

	assert(sna->front);
	assert(screen->GetScreenPixmap(screen) == sna->front);

	DBG(("%s: creating new framebuffer %dx%d\n",
	     __FUNCTION__, width, height));

	new_front = screen->CreatePixmap(screen,
					 width, height, scrn->depth,
					 SNA_CREATE_FB);
	if (!new_front)
		return FALSE;

	xf86DrvMsg(scrn->scrnIndex, X_INFO,
		   "resizing framebuffer to %dx%d\n",
		   width, height);

	for (i = 0; i < sna->mode.num_real_crtc; i++)
		sna_crtc_disable_shadow(sna, to_sna_crtc(config->crtc[i]));
	assert(sna->mode.shadow_active == 0);
	assert(!sna->mode.shadow_enabled);
	assert(sna->mode.shadow_damage == NULL);
	assert(sna->mode.shadow == NULL);

	/* Flush pending shadow updates */
	if (sna->mode.flip_active) {
		DBG(("%s: waiting for %d outstanding TearFree flips\n",
		     __FUNCTION__, sna->mode.flip_active));
		while (sna->mode.flip_active && sna_mode_wait_for_event(sna))
			sna_mode_wakeup(sna);
	}

	/* Cancel a pending [un]flip (as the pixmaps no longer match) */
	sna_present_cancel_flip(sna);
	copy_front(sna, sna->front, new_front);

	screen->SetScreenPixmap(new_front);
	assert(screen->GetScreenPixmap(screen) == new_front);
	assert(sna->front == new_front);
	screen->DestroyPixmap(new_front); /* owned by screen now */

	scrn->virtualX = width;
	scrn->virtualY = height;
	scrn->displayWidth = width;

	/* Only update the CRTCs if we are in control */
	if (!scrn->vtSema)
		return TRUE;

	for (i = 0; i < sna->mode.num_real_crtc; i++) {
		xf86CrtcPtr crtc = config->crtc[i];

		assert(to_sna_crtc(crtc) != NULL);
		if (to_sna_crtc(crtc)->bo == NULL)
			continue;

		if (!__sna_crtc_set_mode(crtc))
			sna_crtc_disable(crtc, false);
	}

	sna_mode_wakeup(sna);
	kgem_clean_scanout_cache(&sna->kgem);

	return TRUE;
}

/* cursor handling */
static void
rotate_coord(Rotation rotation, int size,
	     int x_dst, int y_dst,
	     int *x_src, int *y_src)
{
	int t;

	switch (rotation & 0xf) {
	case RR_Rotate_0:
		break;
	case RR_Rotate_90:
		t = x_dst;
		x_dst = size - y_dst - 1;
		y_dst = t;
		break;
	case RR_Rotate_180:
		x_dst = size - x_dst - 1;
		y_dst = size - y_dst - 1;
		break;
	case RR_Rotate_270:
		t = x_dst;
		x_dst = y_dst;
		y_dst = size - t - 1;
		break;
	}

	if (rotation & RR_Reflect_X)
		x_dst = size - x_dst - 1;
	if (rotation & RR_Reflect_Y)
		y_dst = size - y_dst - 1;

	*x_src = x_dst;
	*y_src = y_dst;
}

static struct sna_cursor *__sna_create_cursor(struct sna *sna, int size)
{
	struct sna_cursor *c;

	for (c = sna->cursor.cursors; c; c = c->next) {
		if (c->ref == 0 && c->alloc >= size) {
			__DBG(("%s: stealing handle=%d, serial=%d, rotation=%d, alloc=%d\n",
			       __FUNCTION__, c->handle, c->serial, c->rotation, c->alloc));
			return c;
		}
	}

	__DBG(("%s(size=%d, num_stash=%d)\n", __FUNCTION__, size, sna->cursor.num_stash));

	c = sna->cursor.stash;
	assert(c);

	c->alloc = ALIGN(size, 4096);
	c->handle = gem_create(sna->kgem.fd, c->alloc);
	if (c->handle == 0)
		return NULL;

	/* Old hardware uses physical addresses, which the kernel
	 * implements in an incoherent fashion requiring a pwrite.
	 */
	if (sna->cursor.use_gtt) {
		c->image = gem_mmap(sna->kgem.fd, c->handle, c->alloc);
		if (c->image == NULL) {
			gem_close(sna->kgem.fd, c->handle);
			return NULL;
		}
	} else
		c->image = NULL;

	__DBG(("%s: handle=%d, allocated %d\n", __FUNCTION__, c->handle, size));

	c->ref = 0;
	c->serial = 0;
	c->rotation = 0;
	c->last_width = c->last_height = 0; /* all clear */
	c->size = size;

	sna->cursor.num_stash--;
	sna->cursor.stash = c->next;

	c->next = sna->cursor.cursors;
	sna->cursor.cursors = c;

	return c;
}

static uint32_t *get_cursor_argb(CursorPtr c)
{
#ifdef ARGB_CURSOR
	return (uint32_t *)c->bits->argb;
#else
	return NULL;
#endif
}

static int __cursor_size(int width, int height)
{
	int i, size;

	i = MAX(width, height);
	for (size = 64; size < i; size <<= 1)
		;

	return size;
}

static struct sna_cursor *__sna_get_cursor(struct sna *sna, xf86CrtcPtr crtc)
{
	struct sna_cursor *cursor;
	const uint8_t *source, *mask;
	const uint32_t *argb;
	uint32_t *image;
	int width, height, pitch, size, x, y;
	bool transformed;
	Rotation rotation;

	assert(sna->cursor.ref);

	cursor = to_sna_crtc(crtc)->cursor;
	__DBG(("%s: current cursor handle=%d, serial=%d [expected %d]\n",
	       __FUNCTION__,
	       cursor ? cursor->handle : 0,
	       cursor ? cursor->serial : 0,
	       sna->cursor.serial));
	if (cursor && cursor->serial == sna->cursor.serial) {
		assert(cursor->size == sna->cursor.size || cursor->transformed);
		assert(cursor->rotation == (!to_sna_crtc(crtc)->cursor_transform && crtc->transform_in_use) ? crtc->rotation : RR_Rotate_0);
		assert(cursor->ref);
		return cursor;
	}

	__DBG(("%s: cursor=%dx%d, pitch=%d, serial=%d, argb?=%d\n", __FUNCTION__,
	       sna->cursor.ref->bits->width,
	       sna->cursor.ref->bits->height,
	       get_cursor_argb(sna->cursor.ref) ? 4*sna->cursor.ref->bits->width : BitmapBytePad(sna->cursor.ref->bits->width),
	       sna->cursor.serial,
	       get_cursor_argb(sna->cursor.ref) != NULL));

	transformed = to_sna_crtc(crtc)->cursor_transform;
	rotation = (!transformed && crtc->transform_in_use) ? crtc->rotation : RR_Rotate_0;

	if (transformed) {
		struct pixman_box16 box;

		box.x1 = box.y1 = 0;
		box.x2 = sna->cursor.ref->bits->width;
		box.y2 = sna->cursor.ref->bits->height;

		pixman_f_transform_bounds(&crtc->f_crtc_to_framebuffer, &box);
		size = __cursor_size(box.x2 - box.x1, box.y2 - box.y1);
		__DBG(("%s: transformed cursor %dx%d -> %dx%d\n",
		       __FUNCTION__ ,
		       sna->cursor.ref->bits->width,
		       sna->cursor.ref->bits->height,
		       box.x2 - box.x1, box.y2 - box.y1));
	} else
		size = sna->cursor.size;

	if (crtc->transform_in_use) {
		RRTransformPtr T = NULL;
		struct pixman_vector v;

		if (crtc->transformPresent) {
			T = &crtc->transform;

			/* Cancel any translation from this affine
			 * transformation. We just want to rotate and scale
			 * the cursor image.
			 */
			v.vector[0] = 0;
			v.vector[1] = 0;
			v.vector[2] = pixman_fixed_1;
			pixman_transform_point(&crtc->transform.transform, &v);
		}

		RRTransformCompute(0, 0, size, size, crtc->rotation, T, NULL,
				   &to_sna_crtc(crtc)->cursor_to_fb,
				   &to_sna_crtc(crtc)->fb_to_cursor);
		if (T)
			pixman_f_transform_translate(
					&to_sna_crtc(crtc)->cursor_to_fb,
					&to_sna_crtc(crtc)->fb_to_cursor,
					-pixman_fixed_to_double(v.vector[0]),
					-pixman_fixed_to_double(v.vector[1]));

		__DBG(("%s: cursor_to_fb [%f %f %f, %f %f %f, %f %f %f]\n",
		       __FUNCTION__,
		       to_sna_crtc(crtc)->cursor_to_fb.m[0][0],
		       to_sna_crtc(crtc)->cursor_to_fb.m[0][1],
		       to_sna_crtc(crtc)->cursor_to_fb.m[0][2],
		       to_sna_crtc(crtc)->cursor_to_fb.m[1][0],
		       to_sna_crtc(crtc)->cursor_to_fb.m[1][1],
		       to_sna_crtc(crtc)->cursor_to_fb.m[1][2],
		       to_sna_crtc(crtc)->cursor_to_fb.m[2][0],
		       to_sna_crtc(crtc)->cursor_to_fb.m[2][1],
		       to_sna_crtc(crtc)->cursor_to_fb.m[2][2]));
	}

	/* Don't allow phys cursor sharing */
	if (sna->cursor.use_gtt && !transformed) {
		for (cursor = sna->cursor.cursors; cursor; cursor = cursor->next) {
			if (cursor->serial == sna->cursor.serial &&
			    cursor->rotation == rotation &&
			    !cursor->transformed) {
				__DBG(("%s: reusing handle=%d, serial=%d, rotation=%d, size=%d\n",
				       __FUNCTION__, cursor->handle, cursor->serial, cursor->rotation, cursor->size));
				assert(cursor->size == sna->cursor.size);
				return cursor;
			}
		}
	}

	cursor = to_sna_crtc(crtc)->cursor;
	if (cursor && cursor->alloc < 4*size*size)
		cursor = NULL;

	if (cursor == NULL) {
		cursor = __sna_create_cursor(sna, 4*size*size);
		if (cursor == NULL) {
			DBG(("%s: failed to allocate cursor\n", __FUNCTION__));
			return NULL;
		}
	}

	width  = sna->cursor.ref->bits->width;
	height = sna->cursor.ref->bits->height;
	source = sna->cursor.ref->bits->source;
	mask = sna->cursor.ref->bits->mask;
	argb = get_cursor_argb(sna->cursor.ref);
	pitch = BitmapBytePad(width);

	image = cursor->image;
	if (image == NULL || transformed) {
		image = sna->cursor.scratch;
		cursor->last_width = cursor->last_height = size;
	}
	if (size > cursor->size ||
	    width < cursor->last_width ||
	    height < cursor->last_height ||
	    rotation != cursor->rotation)
		memset(image, 0, 4*size*size);
	if (rotation == RR_Rotate_0) {
		if (argb == NULL) {
			for (y = 0; y < height; y++) {
				uint32_t *p = image + y*size;
				for (x = 0; x < width; x++) {
					int byte = x / 8;
					uint8_t bit = 1 << (x & 7);
					uint32_t pixel;

					if (mask[byte] & bit) {
						if (source[byte] & bit)
							pixel = sna->cursor.fg;
						else
							pixel = sna->cursor.bg;
					} else
						pixel = 0;

					*p++ = pixel;
				}
				mask += pitch;
				source += pitch;
			}
			if (transformed) {
				__DBG(("%s: Applying affine BLT to bitmap\n", __FUNCTION__));
				affine_blt(image, cursor->image, 32,
					   0, 0, width, height, size * 4,
					   0, 0, size, size, size * 4,
					   &to_sna_crtc(crtc)->cursor_to_fb);
				image = cursor->image;
			}
		} else if (transformed) {
			__DBG(("%s: Applying affine BLT to ARGB\n", __FUNCTION__));
			affine_blt(argb, cursor->image, 32,
				   0, 0, width, height, width * 4,
				   0, 0, size, size, size * 4,
				   &to_sna_crtc(crtc)->cursor_to_fb);
			image = cursor->image;
		} else
			memcpy_blt(argb, image, 32,
				   width * 4, size * 4,
				   0, 0,
				   0, 0,
				   width, height);
	} else {
		for (y = 0; y < size; y++)
			for (x = 0; x < size; x++) {
				uint32_t pixel;
				int xin, yin;

				rotate_coord(rotation, size, x, y, &xin, &yin);
				if (xin < width && yin < height)
					if (argb == NULL) {
						int byte = xin / 8;
						int bit = xin & 7;
						if (mask[yin*pitch + byte] & (1 << bit)) {
							if (source[yin*pitch + byte] & (1 << bit))
								pixel = sna->cursor.fg;
							else
								pixel = sna->cursor.bg;
						} else
							pixel = 0;
					} else
						pixel = argb[yin * width + xin];
				else
					pixel = 0;
				image[y * size + x] = pixel;
			}
	}

	if (image != cursor->image) {
		struct drm_i915_gem_pwrite pwrite;

		VG_CLEAR(pwrite);
		pwrite.handle = cursor->handle;
		pwrite.offset = 0;
		pwrite.size = 4*size*size;
		pwrite.data_ptr = (uintptr_t)image;
		if (drmIoctl(sna->kgem.fd, DRM_IOCTL_I915_GEM_PWRITE, &pwrite))
			__DBG(("%s: cursor update (pwrite) failed: %d\n", __FUNCTION__, errno));
	}

	cursor->size = size;
	cursor->rotation = rotation;
	cursor->transformed = transformed;
	cursor->serial = sna->cursor.serial;
	if (transformed) {
		/* mark the transformed rectangle as dirty, not input */
		cursor->last_width = size;
		cursor->last_height = size;
	} else {
		cursor->last_width = width;
		cursor->last_height = height;
	}
	return cursor;
}

static unsigned char *
sna_realize_cursor(xf86CursorInfoPtr info, CursorPtr cursor)
{
	return NULL;
}

static void __restore_swcursor(ScrnInfoPtr scrn)
{
	struct sna *sna = to_sna(scrn);

	DBG(("%s: attempting to restore SW cursor\n", __FUNCTION__));
	xf86CursorResetCursor(scrn->pScreen);

	/* Try to switch back to the HW cursor on the next cursor update */
	sna->cursor.disable = false;

	RemoveBlockAndWakeupHandlers((void *)__restore_swcursor,
				     (void *)NoopDDA,
				     scrn);
}

static void restore_swcursor(struct sna *sna)
{
	sna->cursor.info->HideCursor(sna->scrn);

	/* XXX Force the cursor to be restored (avoiding recursion) */
	FreeCursor(sna->cursor.ref, None);
	sna->cursor.ref = NULL;

	RegisterBlockAndWakeupHandlers((void *)__restore_swcursor,
				       (void *)NoopDDA,
				       sna->scrn);
}

static void
sna_show_cursors(ScrnInfoPtr scrn)
{
	xf86CrtcConfigPtr xf86_config = XF86_CRTC_CONFIG_PTR(scrn);
	struct sna *sna = to_sna(scrn);
	struct kmsg kmsg;
	int sigio, c;

	DBG(("%s: cursor?=%d\n", __FUNCTION__, sna->cursor.ref != NULL));
	if (sna->cursor.ref == NULL)
		return;

	kmsg_open(&kmsg);
	sigio = sigio_block();
	for (c = 0; c < sna->mode.num_real_crtc; c++) {
		xf86CrtcPtr crtc = xf86_config->crtc[c];
		struct sna_crtc *sna_crtc = to_sna_crtc(crtc);
		struct drm_mode_cursor arg;
		struct sna_cursor *cursor;

		assert(sna_crtc != NULL);
		if (sna_crtc->bo == NULL)
			continue;

		if (!crtc->cursor_in_range) {
			DBG(("%s: skipping cursor outside CRTC (pipe=%d)\n",
			     __FUNCTION__, sna_crtc_pipe(crtc)));
			continue;
		}

		cursor = __sna_get_cursor(sna, crtc);
		if (cursor == NULL ||
		    (sna_crtc->cursor == cursor && sna_crtc->last_cursor_size == cursor->size)) {
			DBG(("%s: skipping cursor already show on CRTC (pipe=%d)\n",
			     __FUNCTION__, sna_crtc_pipe(crtc)));
			continue;
		}

		DBG(("%s: CRTC pipe=%d, handle->%d\n", __FUNCTION__,
		     sna_crtc_pipe(crtc), cursor->handle));

		VG_CLEAR(arg);
		arg.flags = DRM_MODE_CURSOR_BO;
		arg.crtc_id = __sna_crtc_id(sna_crtc);
		arg.width = arg.height = cursor->size;
		arg.handle = cursor->handle;

		if (!FAIL_CURSOR_IOCTL &&
		    drmIoctl(sna->kgem.fd, DRM_IOCTL_MODE_CURSOR, &arg) == 0) {
			if (sna_crtc->cursor) {
				assert(sna_crtc->cursor->ref > 0);
				sna_crtc->cursor->ref--;
			}
			cursor->ref++;
			sna_crtc->cursor = cursor;
			sna_crtc->last_cursor_size = cursor->size;
		} else {
			ERR(("%s: failed to show cursor on CRTC:%d [pipe=%d], disabling hwcursor: errno=%d\n",
			     __FUNCTION__, sna_crtc_id(crtc), sna_crtc_pipe(crtc), errno));
			sna->cursor.disable = true;
		}
	}
	sigio_unblock(sigio);
	sna->cursor.active = true;
	kmsg_close(&kmsg, sna->cursor.disable);

	if (unlikely(sna->cursor.disable))
		restore_swcursor(sna);
}

static void
sna_set_cursor_colors(ScrnInfoPtr scrn, int _bg, int _fg)
{
	struct sna *sna = to_sna(scrn);
	uint32_t fg = _fg, bg = _bg;

	__DBG(("%s(bg=%08x, fg=%08x)\n", __FUNCTION__, bg, fg));

	/* Save ARGB versions of these colors */
	fg |= 0xff000000;
	bg |= 0xff000000;
	if (fg == sna->cursor.fg && bg == sna->cursor.bg)
		return;

	sna->cursor.fg = fg;
	sna->cursor.bg = bg;

	if (sna->cursor.ref == NULL)
		return;

	if (get_cursor_argb(sna->cursor.ref))
		return;

	sna->cursor.serial++;
	__DBG(("%s: serial->%d\n", __FUNCTION__, sna->cursor.serial));

	sna_show_cursors(scrn);
}

static void
sna_crtc_disable_cursor(struct sna *sna, struct sna_crtc *crtc)
{
	struct drm_mode_cursor arg;
	int sigio;

	if (!crtc->cursor)
		return;

	sigio = sigio_block();
	if (crtc->cursor) {
		DBG(("%s: CRTC:%d, handle=%d\n", __FUNCTION__, __sna_crtc_id(crtc), crtc->cursor->handle));
		assert(crtc->cursor->ref > 0);
		crtc->cursor->ref--;
		crtc->cursor = NULL;
		crtc->last_cursor_size = 0;

		VG_CLEAR(arg);
		arg.flags = DRM_MODE_CURSOR_BO;
		arg.crtc_id = __sna_crtc_id(crtc);
		arg.width = arg.height = 0;
		arg.handle = 0;

		(void)drmIoctl(sna->kgem.fd, DRM_IOCTL_MODE_CURSOR, &arg);
	}
	sigio_unblock(sigio);
}

static void
sna_disable_cursors(ScrnInfoPtr scrn)
{
	xf86CrtcConfigPtr xf86_config = XF86_CRTC_CONFIG_PTR(scrn);
	struct sna *sna = to_sna(scrn);
	int sigio, c;

	DBG(("%s\n", __FUNCTION__));

	sigio = sigio_block();
	for (c = 0; c < sna->mode.num_real_crtc; c++) {
		assert(to_sna_crtc(xf86_config->crtc[c]));
		sna_crtc_disable_cursor(sna, to_sna_crtc(xf86_config->crtc[c]));
	}
	sigio_unblock(sigio);
}

static void
sna_hide_cursors(ScrnInfoPtr scrn)
{
	xf86CrtcConfigPtr xf86_config = XF86_CRTC_CONFIG_PTR(scrn);
	struct sna *sna = to_sna(scrn);
	struct sna_cursor *cursor, **prev;
	int sigio, c;

	DBG(("%s\n", __FUNCTION__));
	sna->cursor.active = false;

	sigio = sigio_block();
	for (c = 0; c < sna->mode.num_real_crtc; c++) {
		assert(to_sna_crtc(xf86_config->crtc[c]));
		sna_crtc_disable_cursor(sna, to_sna_crtc(xf86_config->crtc[c]));
	}

	for (prev = &sna->cursor.cursors; (cursor = *prev) != NULL; ) {
		assert(cursor->ref == 0);

		if (cursor->serial == sna->cursor.serial) {
			prev = &cursor->next;
			continue;
		}

		*prev = cursor->next;
		if (cursor->image)
			munmap(cursor->image, cursor->alloc);
		gem_close(sna->kgem.fd, cursor->handle);

		cursor->next = sna->cursor.stash;
		sna->cursor.stash = cursor;
		sna->cursor.num_stash++;
	}

	sigio_unblock(sigio);
}

static void
sna_set_cursor_position(ScrnInfoPtr scrn, int x, int y)
{
	xf86CrtcConfigPtr xf86_config = XF86_CRTC_CONFIG_PTR(scrn);
	struct sna *sna = to_sna(scrn);
	struct kmsg kmsg;
	int sigio, c;

	__DBG(("%s(%d, %d), cursor? %d\n", __FUNCTION__,
	       x, y, sna->cursor.ref!=NULL));
	if (sna->cursor.ref == NULL)
		return;

	kmsg_open(&kmsg);
	sigio = sigio_block();
	sna->cursor.last_x = x;
	sna->cursor.last_y = y;

	/* undo what xf86HWCurs did to the coordinates */
	x += scrn->frameX0;
	y += scrn->frameY0;
	for (c = 0; c < sna->mode.num_real_crtc; c++) {
		xf86CrtcPtr crtc = xf86_config->crtc[c];
		struct sna_crtc *sna_crtc = to_sna_crtc(crtc);
		struct sna_cursor *cursor = NULL;
		struct drm_mode_cursor arg;

		assert(sna_crtc != NULL);

		VG_CLEAR(arg);
		arg.flags = 0;
		arg.crtc_id = __sna_crtc_id(sna_crtc);
		arg.handle = 0;

		if (sna_crtc->bo == NULL)
			goto disable;

		cursor = __sna_get_cursor(sna, crtc);
		if (cursor == NULL)
			cursor = sna_crtc->cursor;
		if (cursor == NULL) {
			__DBG(("%s: failed to grab cursor, disabling\n", __FUNCTION__));
			goto disable;
		}

		if (crtc->transform_in_use) {
			int xhot = sna->cursor.ref->bits->xhot;
			int yhot = sna->cursor.ref->bits->yhot;
			struct pict_f_vector v, hot;

			v.v[0] = x + xhot + .5;
			v.v[1] = y + yhot + .5;
			v.v[2] = 1.;
			pixman_f_transform_point(&crtc->f_framebuffer_to_crtc, &v);

			hot.v[0] = xhot;
			hot.v[1] = yhot;
			hot.v[2] = 1.;
			pixman_f_transform_point(&sna_crtc->fb_to_cursor, &hot);

			arg.x = floor(v.v[0] - hot.v[0]);
			arg.y = floor(v.v[1] - hot.v[1]);
		} else {
			arg.x = x - crtc->x;
			arg.y = y - crtc->y;
		}

		if (arg.x < crtc->mode.HDisplay && arg.x > -sna->cursor.size &&
		    arg.y < crtc->mode.VDisplay && arg.y > -sna->cursor.size) {
			if (sna_crtc->cursor != cursor || sna_crtc->last_cursor_size != cursor->size) {
				arg.flags |= DRM_MODE_CURSOR_BO;
				arg.handle = cursor->handle;
			}

			arg.width = arg.height = cursor->size;
			arg.flags |= DRM_MODE_CURSOR_MOVE;
			crtc->cursor_in_range = true;
		} else {
			crtc->cursor_in_range = false;
disable:
			if (sna_crtc->cursor) {
				arg.flags = DRM_MODE_CURSOR_BO;
				arg.width = arg.height = 0;
			}
			cursor = NULL;
		}

		__DBG(("%s: CRTC:%d (%d, %d), handle=%d, flags=%x (old cursor handle=%d), move? %d, update handle? %d\n",
		       __FUNCTION__, __sna_crtc_id(sna_crtc), arg.x, arg.y, arg.handle, arg.flags, sna_crtc->cursor ? sna_crtc->cursor->handle : 0,
		       arg.flags & DRM_MODE_CURSOR_MOVE, arg.flags & DRM_MODE_CURSOR_BO));

		if (arg.flags == 0)
			continue;

		if (!FAIL_CURSOR_IOCTL &&
		    drmIoctl(sna->kgem.fd, DRM_IOCTL_MODE_CURSOR, &arg) == 0) {
			if (arg.flags & DRM_MODE_CURSOR_BO) {
				if (sna_crtc->cursor) {
					assert(sna_crtc->cursor->ref > 0);
					sna_crtc->cursor->ref--;
				}
				sna_crtc->cursor = cursor;
				if (cursor) {
					sna_crtc->last_cursor_size = cursor->size;
					cursor->ref++;
				} else
					sna_crtc->last_cursor_size = 0;
			}
		} else {
			ERR(("%s: failed to update cursor on CRTC:%d [pipe=%d], disabling hwcursor: errno=%d\n",
			     __FUNCTION__, sna_crtc_id(crtc), sna_crtc_pipe(crtc), errno));
			/* XXX How to force switch back to SW cursor?
			 * Right now we just want until the next cursor image
			 * change, which is fairly frequent.
			 */
			sna->cursor.disable = true;
		}
	}
	sigio_unblock(sigio);
	kmsg_close(&kmsg, sna->cursor.disable);

	if (unlikely(sna->cursor.disable))
		restore_swcursor(sna);
}

#if XORG_VERSION_CURRENT >= XORG_VERSION_NUMERIC(1,15,99,902,2)
static Bool
sna_load_cursor_argb2(ScrnInfoPtr scrn, CursorPtr cursor)
{
	return TRUE;
}

static Bool
sna_load_cursor_image2(ScrnInfoPtr scrn, unsigned char *src)
{
	return TRUE;
}
#endif

static void
sna_load_cursor_argb(ScrnInfoPtr scrn, CursorPtr cursor)
{
}

static void
sna_load_cursor_image(ScrnInfoPtr scrn, unsigned char *src)
{
}

static bool
sna_cursor_preallocate(struct sna *sna)
{
	while (sna->cursor.num_stash < 0) {
		struct sna_cursor *cursor = malloc(sizeof(*cursor));
		if (!cursor)
			return false;

		cursor->next = sna->cursor.stash;
		sna->cursor.stash = cursor;

		sna->cursor.num_stash++;
	}

	return true;
}

static bool
transformable_cursor(struct sna *sna, CursorPtr cursor)
{
	xf86CrtcConfigPtr config = XF86_CRTC_CONFIG_PTR(sna->scrn);
	int i;

	for (i = 0; i < sna->mode.num_real_crtc; i++) {
		xf86CrtcPtr crtc = config->crtc[i];
		struct pixman_box16 box;
		int size;

		if (!to_sna_crtc(crtc)->hwcursor) {
			DBG(("%s: hwcursor disabled on CRTC:%d [pipe=%d]\n",
			     __FUNCTION__, sna_crtc_id(crtc), sna_crtc_pipe(crtc)));
			return false;
		}

		if (!sna->cursor.use_gtt || !sna->cursor.scratch) {
			DBG(("%s: unable to use GTT curosor access [%d] or no scratch [%d]\n",
			     __FUNCTION__, sna->cursor.use_gtt, sna->cursor.scratch));
			return false;
		}

		box.x1 = box.y1 = 0;
		box.x2 = cursor->bits->width;
		box.y2 = cursor->bits->height;

		if (!pixman_f_transform_bounds(&crtc->f_crtc_to_framebuffer,
					       &box)) {
			DBG(("%s: unable to transform bounds\n", __FUNCTION__));
			return false;
		}

		size = __cursor_size(box.x2 - box.x1, box.y2 - box.y1);
		if (size > sna->cursor.max_size) {
			DBG(("%s: transformed cursor size=%d too large, max=%d\n",
			     __FUNCTION__, size, sna->cursor.max_size));
			return false;
		}
	}

	return true;
}

static Bool
sna_use_hw_cursor(ScreenPtr screen, CursorPtr cursor)
{
	struct sna *sna = to_sna_from_screen(screen);

	DBG(("%s (%dx%d)?\n", __FUNCTION__,
	     cursor->bits->width, cursor->bits->height));

	if (sna->cursor.disable)
		return FALSE;

	/* cursors are invariant */
	if (cursor == sna->cursor.ref)
		return TRUE;

	if (sna->cursor.ref) {
		FreeCursor(sna->cursor.ref, None);
		sna->cursor.ref = NULL;
	}

	sna->cursor.size =
		__cursor_size(cursor->bits->width, cursor->bits->height);
	if (sna->cursor.size > sna->cursor.max_size) {
		DBG(("%s: cursor size=%d too large, max %d: using sw cursor\n",
		     __FUNCTION__, sna->cursor.size, sna->cursor.max_size));
		return FALSE;
	}

	if (sna->mode.rr_active && !transformable_cursor(sna, cursor)) {
		DBG(("%s: RandR active [%d] and non-transformable cursor: using sw cursor\n",
		     __FUNCTION__, sna->mode.rr_active));
		return FALSE;
	}

	if (!sna_cursor_preallocate(sna)) {
		DBG(("%s: cursor preallocation failed: using sw cursor\n", __FUNCTION__));
		return FALSE;
	}

	sna->cursor.ref = cursor;
	cursor->refcnt++;
	sna->cursor.serial++;

	DBG(("%s(%dx%d): ARGB?=%d, serial->%d, size->%d\n", __FUNCTION__,
	       cursor->bits->width,
	       cursor->bits->height,
	       get_cursor_argb(cursor) != NULL,
	       sna->cursor.serial,
	       sna->cursor.size));
	return TRUE;
}

static void
sna_cursor_pre_init(struct sna *sna)
{
	struct local_get_cap {
		uint64_t name;
		uint64_t value;
	} cap;
	int v;

	if (sna->mode.num_real_crtc == 0)
		return;

#define LOCAL_IOCTL_GET_CAP	DRM_IOWR(0x0c, struct local_get_cap)
#ifndef DRM_CAP_CURSOR_WIDTH
#define DRM_CAP_CURSOR_WIDTH	0x8
#endif
#ifndef DRM_CAP_CURSOR_HEIGHT
#define DRM_CAP_CURSOR_HEIGHT	0x9
#endif

#define I915_PARAM_HAS_COHERENT_PHYS_GTT 29

	sna->cursor.max_size = 64;

	cap.value = 0;
	cap.name = DRM_CAP_CURSOR_WIDTH;
	if (drmIoctl(sna->kgem.fd, LOCAL_IOCTL_GET_CAP, &cap) == 0)
		sna->cursor.max_size = cap.value;

	cap.name = DRM_CAP_CURSOR_HEIGHT;
	if (drmIoctl(sna->kgem.fd, LOCAL_IOCTL_GET_CAP, &cap) == 0 &&
	    cap.value < sna->cursor.max_size)
		sna->cursor.max_size = cap.value;

	v = -1; /* No param uses the sign bit, reserve it for errors */
	if (sna->kgem.gen >= 033) {
		v = 1;
	} else {
		drm_i915_getparam_t gp = {
			I915_PARAM_HAS_COHERENT_PHYS_GTT,
			&v,
		};
		(void)drmIoctl(sna->kgem.fd, DRM_IOCTL_I915_GETPARAM, &gp);
	}
	sna->cursor.use_gtt = v > 0;
	DBG(("%s: cursor updates use_gtt?=%d\n",
	     __FUNCTION__, sna->cursor.use_gtt));

	sna->cursor.scratch = malloc(sna->cursor.max_size * sna->cursor.max_size * 4);
	if (!sna->cursor.scratch && !sna->cursor.use_gtt)
		sna->cursor.max_size = 0;

	sna->cursor.num_stash = -sna->mode.num_real_crtc;

	xf86DrvMsg(sna->scrn->scrnIndex, X_PROBED,
		   "Using a maximum size of %dx%d for hardware cursors\n",
		   sna->cursor.max_size, sna->cursor.max_size);
}

static void
sna_cursor_close(struct sna *sna)
{
	sna->cursor.serial = 0;
	sna_hide_cursors(sna->scrn);

	while (sna->cursor.stash) {
		struct sna_cursor *cursor = sna->cursor.stash;
		sna->cursor.stash = cursor->next;
		free(cursor);
	}

	sna->cursor.num_stash = -sna->mode.num_real_crtc;
}

bool
sna_cursors_init(ScreenPtr screen, struct sna *sna)
{
	xf86CursorInfoPtr cursor_info;

	if (sna->cursor.max_size == 0)
		return false;

	cursor_info = xf86CreateCursorInfoRec();
	if (cursor_info == NULL)
		return false;

	cursor_info->MaxWidth = sna->cursor.max_size;
	cursor_info->MaxHeight = sna->cursor.max_size;
	cursor_info->Flags = (HARDWARE_CURSOR_TRUECOLOR_AT_8BPP |
			      HARDWARE_CURSOR_UPDATE_UNHIDDEN |
			      HARDWARE_CURSOR_ARGB);

	cursor_info->RealizeCursor = sna_realize_cursor;
	cursor_info->SetCursorColors = sna_set_cursor_colors;
	cursor_info->SetCursorPosition = sna_set_cursor_position;
	cursor_info->LoadCursorImage = sna_load_cursor_image;
	cursor_info->HideCursor = sna_hide_cursors;
	cursor_info->ShowCursor = sna_show_cursors;
	cursor_info->UseHWCursor = sna_use_hw_cursor;
#ifdef ARGB_CURSOR
	cursor_info->UseHWCursorARGB = sna_use_hw_cursor;
	cursor_info->LoadCursorARGB = sna_load_cursor_argb;
#endif
#if XORG_VERSION_CURRENT >= XORG_VERSION_NUMERIC(1,15,99,902,3)
	cursor_info->LoadCursorImageCheck = sna_load_cursor_image2;
#ifdef ARGB_CURSOR
	cursor_info->LoadCursorARGBCheck = sna_load_cursor_argb2;
#endif
#endif

	if (!xf86InitCursor(screen, cursor_info)) {
		xf86DestroyCursorInfoRec(cursor_info);
		return false;
	}

	sna->cursor.info = cursor_info;
	return true;
}

static void
sna_cursors_reload(struct sna *sna)
{
	DBG(("%s: active?=%d\n", __FUNCTION__, sna->cursor.active));
	if (sna->cursor.active)
		sna_set_cursor_position(sna->scrn,
					sna->cursor.last_x,
					sna->cursor.last_y);
}

static void
sna_cursors_fini(struct sna *sna)
{
	if (sna->cursor.info) {
		xf86DestroyCursorInfoRec(sna->cursor.info);
		sna->cursor.info = NULL;
	}

	if (sna->cursor.ref) {
		FreeCursor(sna->cursor.ref, None);
		sna->cursor.ref = NULL;
	}
}

static bool
sna_crtc_flip(struct sna *sna, struct sna_crtc *crtc, struct kgem_bo *bo, int x, int y)
{
	xf86CrtcConfigPtr config = XF86_CRTC_CONFIG_PTR(sna->scrn);
	struct drm_mode_crtc arg;
	uint32_t output_ids[32];
	int output_count = 0;
	int i;

	DBG(("%s CRTC:%d [pipe=%d], handle=%d\n", __FUNCTION__, __sna_crtc_id(crtc), __sna_crtc_pipe(crtc), bo->handle));

	assert(sna->mode.num_real_output < ARRAY_SIZE(output_ids));
	assert(crtc->bo);
	assert(crtc->kmode.clock);

	for (i = 0; i < sna->mode.num_real_output; i++) {
		xf86OutputPtr output = config->output[i];

		if (output->crtc != crtc->base)
			continue;

		DBG(("%s: attaching output '%s' %d [%d] to crtc:%d (pipe %d) (possible crtc:%x, possible clones:%x)\n",
		     __FUNCTION__, output->name, i, to_connector_id(output),
		     __sna_crtc_id(crtc), __sna_crtc_pipe(crtc),
		     (uint32_t)output->possible_crtcs,
		     (uint32_t)output->possible_clones));

		assert(output->possible_crtcs & (1 << __sna_crtc_pipe(crtc)) ||
		       is_zaphod(sna->scrn));

		output_ids[output_count] = to_connector_id(output);
		if (++output_count == ARRAY_SIZE(output_ids))
			return false;
	}
	assert(output_count);

	VG_CLEAR(arg);
	arg.crtc_id = __sna_crtc_id(crtc);
	arg.fb_id = fb_id(bo);
	assert(arg.fb_id);
	arg.x = x;
	arg.y = y;
	arg.set_connectors_ptr = (uintptr_t)output_ids;
	arg.count_connectors = output_count;
	arg.mode = crtc->kmode;
	arg.mode_valid = 1;

	DBG(("%s: applying crtc [%d, pipe=%d] mode=%dx%d+%d+%d@%d, fb=%d across %d outputs [%d...]\n",
	     __FUNCTION__, __sna_crtc_id(crtc), __sna_crtc_pipe(crtc),
	     arg.mode.hdisplay,
	     arg.mode.vdisplay,
	     arg.x, arg.y,
	     arg.mode.clock,
	     arg.fb_id,
	     output_count, output_count ? output_ids[0] : 0));

	if (drmIoctl(sna->kgem.fd, DRM_IOCTL_MODE_SETCRTC, &arg))
		return false;

	crtc->offset = y << 16 | x;
	__kgem_bo_clear_dirty(bo);
	return true;
}

static void sna_mode_restore(struct sna *sna)
{
	xf86CrtcConfigPtr config = XF86_CRTC_CONFIG_PTR(sna->scrn);
	int error = 0;
	int i;

	assert(!sna->mode.hidden);

	for (i = 0; i < sna->mode.num_real_crtc; i++) {
		xf86CrtcPtr crtc = config->crtc[i];

		assert(to_sna_crtc(crtc) != NULL);
		if (to_sna_crtc(crtc)->bo == NULL)
			continue;

		assert(crtc->enabled);
		if (!__sna_crtc_set_mode(crtc)) {
			sna_crtc_disable(crtc, false);
			error++;
		}
	}
	sna_mode_wakeup(sna);
	while (sna->mode.flip_active && sna_mode_wakeup(sna))
		;
	update_flush_interval(sna);
	sna_cursors_reload(sna);
	sna->mode.dirty = false;

	if (error)
		xf86DrvMsg(sna->scrn->scrnIndex, X_ERROR,
			   "Failed to restore display configuration\n");
}

bool sna_needs_page_flip(struct sna *sna, struct kgem_bo *bo)
{
	xf86CrtcConfigPtr config = XF86_CRTC_CONFIG_PTR(sna->scrn);
	int i;

	for (i = 0; i < sna->mode.num_real_crtc; i++) {
		struct sna_crtc *crtc = config->crtc[i]->driver_private;

		if (crtc->bo == NULL)
			continue;

		if (crtc->bo == bo)
			continue;

		return true;
	}

	return false;
}

int
sna_page_flip(struct sna *sna,
	      struct kgem_bo *bo,
	      sna_flip_handler_t handler,
	      void *data)
{
	xf86CrtcConfigPtr config = XF86_CRTC_CONFIG_PTR(sna->scrn);
	const int width = sna->scrn->virtualX;
	const int height = sna->scrn->virtualY;
	int sigio;
	int count = 0;
	int i;

	DBG(("%s: handle %d attached\n", __FUNCTION__, bo->handle));
	assert(bo->refcnt);

	assert((sna->flags & SNA_IS_HOSTED) == 0);
	assert(sna->mode.flip_active == 0);
	assert(sna->mode.front_active);
	assert(!sna->mode.hidden);
	assert(sna->scrn->vtSema);

	if ((sna->flags & (data ? SNA_HAS_FLIP : SNA_HAS_ASYNC_FLIP)) == 0)
		return 0;

	kgem_bo_submit(&sna->kgem, bo);
	__kgem_bo_clear_dirty(bo);

	sigio = sigio_block();
	for (i = 0; i < sna->mode.num_real_crtc; i++) {
		struct sna_crtc *crtc = config->crtc[i]->driver_private;
		struct drm_mode_crtc_page_flip arg;
		uint32_t crtc_offset;
		int fixup;

		DBG(("%s: crtc %d id=%d, pipe=%d active? %d\n",
		     __FUNCTION__, i, __sna_crtc_id(crtc), __sna_crtc_pipe(crtc), crtc->bo != NULL));
		if (crtc->bo == NULL)
			continue;
		assert(!crtc->transform);
		assert(!crtc->slave_pixmap);
		assert(crtc->bo->active_scanout);
		assert(crtc->bo->refcnt >= crtc->bo->active_scanout);
		assert(crtc->flip_bo == NULL);

		assert_crtc_fb(sna, crtc);
		if (data == NULL && crtc->bo == bo)
			goto next_crtc;

		arg.crtc_id = __sna_crtc_id(crtc);
		arg.fb_id = get_fb(sna, bo, width, height);
		if (arg.fb_id == 0) {
			assert(count == 0);
			break;
		}

		fixup = 0;
		crtc_offset = crtc->base->y << 16 | crtc->base->x;

		if (bo->pitch != crtc->bo->pitch || crtc_offset != crtc->offset) {
			DBG(("%s: changing pitch (%d == %d) or offset (%x == %x)\n",
			     __FUNCTION__,
			     bo->pitch, crtc->bo->pitch,
			     crtc_offset, crtc->offset));
fixup_flip:
			fixup = 1;
			if (crtc->bo != bo && sna_crtc_flip(sna, crtc, bo, crtc->base->x, crtc->base->y)) {
update_scanout:
				DBG(("%s: removing handle=%d [active_scanout=%d] from scanout, installing handle=%d [active_scanout=%d]\n",
				     __FUNCTION__, crtc->bo->handle, crtc->bo->active_scanout,
				     bo->handle, bo->active_scanout));
				assert(crtc->bo->active_scanout);
				assert(crtc->bo->refcnt >= crtc->bo->active_scanout);
				crtc->bo->active_scanout--;
				kgem_bo_destroy(&sna->kgem, crtc->bo);

				if (crtc->shadow_bo) {
					kgem_bo_destroy(&sna->kgem, crtc->shadow_bo);
					crtc->shadow_bo = NULL;
				}

				crtc->bo = kgem_bo_reference(bo);
				crtc->bo->active_scanout++;

				if (data == NULL)
					goto next_crtc;

				/* queue a flip in order to send the event */
			} else
				goto error;
		}

		/* Only the reference crtc will finally deliver its page flip
		 * completion event. All other crtc's events will be discarded.
		 */
		if (data) {
			arg.user_data = (uintptr_t)crtc;
			arg.flags = DRM_MODE_PAGE_FLIP_EVENT;
		} else {
			arg.user_data = 0;
			arg.flags = DRM_MODE_PAGE_FLIP_ASYNC;
		}
		arg.reserved = 0;

retry_flip:
		DBG(("%s: crtc %d id=%d, pipe=%d  --> fb %d\n",
		     __FUNCTION__, i, __sna_crtc_id(crtc), __sna_crtc_pipe(crtc), arg.fb_id));
		if (drmIoctl(sna->kgem.fd, DRM_IOCTL_MODE_PAGE_FLIP, &arg)) {
			ERR(("%s: pageflip failed with err=%d\n", __FUNCTION__, errno));

			if (errno == EBUSY) {
				struct drm_mode_crtc mode;

				memset(&mode, 0, sizeof(mode));
				mode.crtc_id = __sna_crtc_id(crtc);
				drmIoctl(sna->kgem.fd, DRM_IOCTL_MODE_GETCRTC, &mode);

				DBG(("%s: crtc=%d, valid?=%d, fb attached?=%d, expected=%d\n",
				     __FUNCTION__,
				     mode.crtc_id, mode.mode_valid,
				     mode.fb_id, fb_id(crtc->bo)));

				if (mode.fb_id != fb_id(crtc->bo))
					goto fixup_flip;

				if (count == 0)
					break;

				DBG(("%s: throttling on busy flip / waiting for kernel to catch up\n", __FUNCTION__));
				drmIoctl(sna->kgem.fd, DRM_IOCTL_I915_GEM_THROTTLE, 0);
				sna->kgem.need_throttle = false;

				goto retry_flip;
			}

			if (!fixup)
				goto fixup_flip;

error:
			xf86DrvMsg(sna->scrn->scrnIndex, X_ERROR,
					"page flipping failed, on CRTC:%d (pipe=%d), disabling %s page flips\n",
					__sna_crtc_id(crtc), __sna_crtc_pipe(crtc), data ? "synchronous": "asynchronous");

			if (count || crtc->bo == bo)
				sna_mode_restore(sna);

			sna->flags &= ~(data ? SNA_HAS_FLIP : SNA_HAS_ASYNC_FLIP);
			count = 0;
			break;
		}

		if (data) {
			assert(crtc->flip_bo == NULL);
			assert(handler);
			crtc->flip_handler = handler;
			crtc->flip_data = data;
			crtc->flip_bo = kgem_bo_reference(bo);
			crtc->flip_bo->active_scanout++;
			crtc->flip_serial = crtc->mode_serial;
			crtc->flip_pending = true;
			sna->mode.flip_active++;

			DBG(("%s: recording flip on CRTC:%d handle=%d, active_scanout=%d, serial=%d\n",
			     __FUNCTION__, __sna_crtc_id(crtc), crtc->flip_bo->handle, crtc->flip_bo->active_scanout, crtc->flip_serial));
		} else
			goto update_scanout;
next_crtc:
		count++;
	}
	sigio_unblock(sigio);

	DBG(("%s: page flipped %d crtcs\n", __FUNCTION__, count));
	return count;
}

static const xf86CrtcConfigFuncsRec sna_mode_funcs = {
	sna_mode_resize
};

static void set_size_range(struct sna *sna)
{
	/* We lie slightly as we expect no single monitor to exceed the
	 * crtc limits, so if the mode exceeds the scanout restrictions,
	 * we will quietly convert that to per-crtc pixmaps.
	 */
	xf86CrtcSetSizeRange(sna->scrn, 8, 8, INT16_MAX, INT16_MAX);
}

#if HAS_GAMMA
static void set_gamma(uint16_t *curve, int size, double value)
{
	int i;

	value = 1/value;
	for (i = 0; i < size; i++)
		curve[i] = 256*(size-1)*pow(i/(double)(size-1), value);
}

static void output_set_gamma(xf86OutputPtr output, xf86CrtcPtr crtc)
{
	XF86ConfMonitorPtr mon = output->conf_monitor;

	if (!mon)
		return;

	DBG(("%s: red=%f\n", __FUNCTION__, mon->mon_gamma_red));
	if (mon->mon_gamma_red >= GAMMA_MIN &&
	    mon->mon_gamma_red <= GAMMA_MAX &&
	    mon->mon_gamma_red != 1.0)
		set_gamma(crtc->gamma_red, crtc->gamma_size,
			  mon->mon_gamma_red);

	DBG(("%s: green=%f\n", __FUNCTION__, mon->mon_gamma_green));
	if (mon->mon_gamma_green >= GAMMA_MIN &&
	    mon->mon_gamma_green <= GAMMA_MAX &&
	    mon->mon_gamma_green != 1.0)
		set_gamma(crtc->gamma_green, crtc->gamma_size,
			  mon->mon_gamma_green);

	DBG(("%s: blue=%f\n", __FUNCTION__, mon->mon_gamma_blue));
	if (mon->mon_gamma_blue >= GAMMA_MIN &&
	    mon->mon_gamma_blue <= GAMMA_MAX &&
	    mon->mon_gamma_blue != 1.0)
		set_gamma(crtc->gamma_blue, crtc->gamma_size,
			  mon->mon_gamma_blue);
}

static void crtc_init_gamma(xf86CrtcPtr crtc)
{
	uint16_t *gamma;

	/* Initialize the gamma ramps */
	gamma = NULL;
	if (crtc->gamma_size == 256)
		gamma = crtc->gamma_red;
	if (gamma == NULL)
		gamma = malloc(3 * 256 * sizeof(uint16_t));
	if (gamma) {
		struct sna *sna = to_sna(crtc->scrn);
		struct sna_crtc *sna_crtc = to_sna_crtc(crtc);
		struct drm_mode_crtc_lut lut;
		bool gamma_set = false;

		assert(sna_crtc);

		lut.crtc_id = __sna_crtc_id(sna_crtc);
		lut.gamma_size = 256;
		lut.red = (uintptr_t)(gamma);
		lut.green = (uintptr_t)(gamma + 256);
		lut.blue = (uintptr_t)(gamma + 2 * 256);
		if (drmIoctl(sna->kgem.fd, DRM_IOCTL_MODE_GETGAMMA, &lut) == 0) {
			VG(VALGRIND_MAKE_MEM_DEFINED(gamma, 3*256*sizeof(gamma[0])));
			gamma_set =
				gamma[256 - 1] &&
				gamma[2*256 - 1] &&
				gamma[3*256 - 1];
		}

		DBG(("%s: CRTC:%d, pipe=%d: gamma set?=%d\n",
		     __FUNCTION__, __sna_crtc_id(sna_crtc), __sna_crtc_pipe(sna_crtc),
		     gamma_set));
		if (!gamma_set) {
			int i;

			for (i = 0; i < 256; i++) {
				gamma[i] = i << 8;
				gamma[256 + i] = i << 8;
				gamma[2*256 + i] = i << 8;
			}
		}

		if (gamma != crtc->gamma_red) {
			free(crtc->gamma_red);
			crtc->gamma_red = gamma;
			crtc->gamma_green = gamma + 256;
			crtc->gamma_blue = gamma + 2*256;
			crtc->gamma_size = 256;
		}
	}
}
#else
static void output_set_gamma(xf86OutputPtr output, xf86CrtcPtr crtc) { }
static void crtc_init_gamma(xf86CrtcPtr crtc) { }
#endif

static const char *preferred_mode(xf86OutputPtr output)
{
	const char *mode;

	mode = xf86GetOptValString(output->options, OPTION_PREFERRED_MODE);
	if (mode)
		return mode;

	if (output->scrn->display->modes && *output->scrn->display->modes)
		return *output->scrn->display->modes;

	return NULL;
}

static bool sna_probe_initial_configuration(struct sna *sna)
{
	ScrnInfoPtr scrn = sna->scrn;
	xf86CrtcConfigPtr config = XF86_CRTC_CONFIG_PTR(scrn);
	int crtc_active, crtc_enabled;
	int width, height;
	int i, j;

	assert((sna->flags & SNA_IS_HOSTED) == 0);

	if ((sna->flags & SNA_IS_SLAVED) == 0) {
		const int user_overrides[] = {
			OPTION_POSITION,
			OPTION_BELOW,
			OPTION_RIGHT_OF,
			OPTION_ABOVE,
			OPTION_LEFT_OF,
			OPTION_ROTATE,
			OPTION_PANNING,
		};
		if (xf86ReturnOptValBool(sna->Options, OPTION_REPROBE, FALSE)) {
			DBG(("%s: user requests reprobing\n", __FUNCTION__));
			return false;
		}

		/* First scan through all outputs and look for user overrides */
		for (i = 0; i < sna->mode.num_real_output; i++) {
			xf86OutputPtr output = config->output[i];

			for (j = 0; j < ARRAY_SIZE(user_overrides); j++) {
				if (xf86GetOptValString(output->options, user_overrides[j])) {
					DBG(("%s: user placement [%d] for %s\n",
					     __FUNCTION__,
					     user_overrides[j],
					     output->name));
					return false;
				}
			}
		}
	}

	/* Copy the existing modes on each CRTCs */
	crtc_active = crtc_enabled = 0;
	for (i = 0; i < sna->mode.num_real_crtc; i++) {
		xf86CrtcPtr crtc = config->crtc[i];
		struct sna_crtc *sna_crtc = to_sna_crtc(crtc);
		struct drm_mode_crtc mode;

		crtc->enabled = FALSE;
		crtc->desiredMode.status = MODE_NOMODE;

		crtc_init_gamma(crtc);

		/* Retrieve the current mode */
		VG_CLEAR(mode);
		mode.crtc_id = __sna_crtc_id(sna_crtc);
		if (drmIoctl(sna->kgem.fd, DRM_IOCTL_MODE_GETCRTC, &mode))
			continue;

		DBG(("%s: CRTC:%d, pipe=%d: has mode?=%d\n", __FUNCTION__,
		     __sna_crtc_id(sna_crtc), __sna_crtc_pipe(sna_crtc),
		     mode.mode_valid && mode.mode.clock));

		if (!mode.mode_valid || mode.mode.clock == 0)
			continue;

		mode_from_kmode(scrn, &mode.mode, &crtc->desiredMode);
		crtc->desiredRotation = sna_crtc->primary.rotation.current;
		crtc->desiredX = mode.x;
		crtc->desiredY = mode.y;
		crtc->desiredTransformPresent = FALSE;
		crtc_active++;
	}

	/* Reconstruct outputs pointing to active CRTC */
	for (i = 0; i < sna->mode.num_real_output; i++) {
		xf86OutputPtr output = config->output[i];
		uint32_t crtc_id;

		assert(to_sna_output(output));

		crtc_id = (uintptr_t)output->crtc;
		output->crtc = NULL;
		output->status = XF86OutputStatusUnknown;
		if (sna->flags & SNA_IS_SLAVED)
			continue;

		if (crtc_id == 0) {
			DBG(("%s: not using output %s, disconnected\n",
			     __FUNCTION__, output->name));
			continue;
		}

		if (xf86ReturnOptValBool(output->options, OPTION_DISABLE, 0)) {
			DBG(("%s: not using output %s, manually disabled\n",
			     __FUNCTION__, output->name));
			continue;
		}

		for (j = 0; j < sna->mode.num_real_crtc; j++) {
			xf86CrtcPtr crtc = config->crtc[j];

			assert(to_sna_crtc(crtc));
			if (sna_crtc_id(crtc) != crtc_id)
				continue;

			if (crtc->desiredMode.status == MODE_OK) {
				DisplayModePtr M;
				const char *pref;

				pref = preferred_mode(output);
				if (pref && strcmp(pref, crtc->desiredMode.name)) {
					DBG(("%s: output %s user requests a different preferred mode %s, found %s\n",
					     __FUNCTION__, output->name, pref, crtc->desiredMode.name));
					return false;
				}

				xf86DrvMsg(scrn->scrnIndex, X_PROBED,
					   "Output %s using initial mode %s on pipe %d\n",
					   output->name,
					   crtc->desiredMode.name,
					   sna_crtc_pipe(crtc));

				output->crtc = crtc;
				output->status = XF86OutputStatusConnected;
				crtc->enabled = TRUE;
				crtc_enabled++;

				output_set_gamma(output, crtc);

				if (output->conf_monitor) {
					output->mm_width = output->conf_monitor->mon_width;
					output->mm_height = output->conf_monitor->mon_height;
				}

#if 0
				sna_output_attach_edid(output);
				sna_output_attach_tile(output);
#endif

				if (output->mm_width == 0 || output->mm_height == 0) {
					output->mm_height = (crtc->desiredMode.VDisplay * 254) / (10*DEFAULT_DPI);
					output->mm_width = (crtc->desiredMode.HDisplay * 254) / (10*DEFAULT_DPI);
				}

				M = calloc(1, sizeof(DisplayModeRec));
				if (M) {
					*M = crtc->desiredMode;
					M->name = strdup(M->name);
					output->probed_modes =
						xf86ModesAdd(output->probed_modes, M);
				}
			}

			break;
		}

		if (j == sna->mode.num_real_crtc) {
			/* Can not find the earlier associated CRTC, bail */
			DBG(("%s: existing setup conflicts with output assignment (Zaphod), reprobing\n",
			     __FUNCTION__));
			return false;
		}
	}

	if (crtc_active != crtc_enabled) {
		DBG(("%s: only enabled %d out of %d active CRTC, forcing a reconfigure\n",
		     __FUNCTION__, crtc_enabled, crtc_active));
		return false;
	}

	width = height = 0;
	for (i = 0; i < sna->mode.num_real_crtc; i++) {
		xf86CrtcPtr crtc = config->crtc[i];
		int w, h;

		if (!crtc->enabled)
			continue;

		w = crtc->desiredX + crtc->desiredMode.HDisplay;
		if (w > width)
			width = w;
		h = crtc->desiredY + crtc->desiredMode.VDisplay;
		if (h > height)
			height = h;
	}

	/* Prefer the native panel size if any */
	if (!width || !height) {
		for (i = 0; i < sna->mode.num_real_output; i++) {
			xf86OutputPtr output = config->output[i];
			struct sna_output *sna_output = to_sna_output(output);

			if (!sna_output->is_panel)
				continue;

			DBG(("%s: querying panel '%s' for preferred unattached size\n",
			     __FUNCTION__, output->name));

			if (sna_output_detect(output) != XF86OutputStatusConnected)
				continue;

			if (sna_output->num_modes == 0)
				continue;

			width  = sna_output->modes[0].hdisplay;
			height = sna_output->modes[0].vdisplay;

			DBG(("%s: panel '%s' is %dx%d\n",
			     __FUNCTION__, output->name, width, height));
			break;
		}
	}

	if (!width || !height) {
		width = 1024;
		height = 768;
	}

	scrn->display->frameX0 = 0;
	scrn->display->frameY0 = 0;
	scrn->display->virtualX = width;
	scrn->display->virtualY = height;

	scrn->virtualX = width;
	scrn->virtualY = height;

	xf86SetScrnInfoModes(sna->scrn);
	DBG(("%s: SetScrnInfoModes = %p\n", __FUNCTION__, scrn->modes));
	return scrn->modes != NULL;
}

static void
sanitize_outputs(struct sna *sna)
{
	xf86CrtcConfigPtr config = XF86_CRTC_CONFIG_PTR(sna->scrn);
	int i;

	for (i = 0; i < config->num_output; i++)
		config->output[i]->crtc = NULL;
}

static bool has_flip(struct sna *sna)
{
	drm_i915_getparam_t gp;
	int v;

	if (sna->flags & SNA_NO_FLIP)
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

static bool has_flip__async(struct sna *sna)
{
#define DRM_CAP_ASYNC_PAGE_FLIP 0x7
	struct local_get_cap {
		uint64_t name;
		uint64_t value;
	} cap = { DRM_CAP_ASYNC_PAGE_FLIP };

	if (sna->flags & SNA_NO_FLIP)
		return false;

	if (drmIoctl(sna->kgem.fd, LOCAL_IOCTL_GET_CAP, &cap) == 0)
		return cap.value > 0;

	return false;
}

static void
probe_capabilities(struct sna *sna)
{
	sna->flags &= ~(SNA_HAS_FLIP | SNA_HAS_ASYNC_FLIP);
	if (has_flip(sna))
		sna->flags |= SNA_HAS_FLIP;
	if (has_flip__async(sna) && (sna->flags & SNA_TEAR_FREE) == 0)
		sna->flags |= SNA_HAS_ASYNC_FLIP;
	DBG(("%s: page flips? %s, async? %s\n", __FUNCTION__,
	     sna->flags & SNA_HAS_FLIP ? "enabled" : "disabled",
	     sna->flags & SNA_HAS_ASYNC_FLIP ? "enabled" : "disabled"));
}

void
sna_crtc_config_notify(ScreenPtr screen)
{
	struct sna *sna = to_sna_from_screen(screen);

	DBG(("%s(dirty?=%d)\n", __FUNCTION__, sna->mode.dirty));
	if (!sna->mode.dirty)
		return;

	if (disable_unused_crtc(sna)) {
		/* This will have recursed, so simply bail at this point */
		assert(sna->mode.dirty == false);
#ifdef RANDR_12_INTERFACE
		xf86RandR12TellChanged(screen);
#endif
		return;
	}

	/* Flush any events completed by the modeset */
	sna_mode_wakeup(sna);

	update_flush_interval(sna);
	sna->cursor.disable = false; /* Reset HW cursor until the next fail */
	sna_cursors_reload(sna);

	probe_capabilities(sna);
	sna_present_update(sna);

	/* Allow TearFree to come back on when everything is off */
	if (!sna->mode.front_active && sna->flags & SNA_WANT_TEAR_FREE) {
		if ((sna->flags & SNA_TEAR_FREE) == 0)
			DBG(("%s: enable TearFree next modeset\n",
			     __FUNCTION__));

		sna->flags |= SNA_TEAR_FREE;
	}

	sna->mode.dirty = false;
}

#if HAS_PIXMAP_SHARING
#define sna_setup_provider(scrn) xf86ProviderSetup(scrn, NULL, "Intel")
#else
#define sna_setup_provider(scrn)
#endif

bool sna_mode_pre_init(ScrnInfoPtr scrn, struct sna *sna)
{
	drmModeResPtr res;
	int num_fake = 0;
	int i;

	if (sna->flags & SNA_IS_HOSTED) {
		sna_setup_provider(scrn);
		return true;
	}

	probe_capabilities(sna);
	sna->mode.hidden = !isGPU(scrn); /* No DPMS passthrough */

	if (!xf86GetOptValInteger(sna->Options, OPTION_VIRTUAL, &num_fake))
		num_fake = 1;

	res = drmModeGetResources(sna->kgem.fd);
	if (res &&
	    (res->count_crtcs == 0 ||
	     res->count_encoders == 0 ||
	     res->count_connectors == 0)) {
		drmModeFreeResources(res);
		res = NULL;
	}
	if (res) {
		xf86CrtcConfigPtr xf86_config;

		DBG(("%s: found %d CRTC, %d encoders, %d connectors\n",
		     __FUNCTION__, res->count_crtcs, res->count_encoders, res->count_connectors));

		assert(res->count_crtcs);
		assert(res->count_connectors);

		xf86CrtcConfigInit(scrn, &sna_mode_funcs);

		xf86_config = XF86_CRTC_CONFIG_PTR(scrn);
		xf86_config->xf86_crtc_notify = sna_crtc_config_notify;
		xf86_config->compat_output = 0;

		for (i = 0; i < res->count_crtcs; i++)
			if (!sna_crtc_add(scrn, res->crtcs[i]))
				return false;

		sna->mode.num_real_crtc = xf86_config->num_crtc;

		sna->mode.num_real_encoder = res->count_encoders;
		sna->mode.encoders = res->encoders;
		res->encoders = NULL;

		for (i = 0; i < res->count_connectors; i++)
			if (sna_output_add(sna, res->connectors[i], 0) < 0)
				return false;

		sna->mode.num_real_output = xf86_config->num_output;

		sna->mode.max_crtc_width  = res->max_width;
		sna->mode.max_crtc_height = res->max_height;

		RegionEmpty(&sna->mode.shadow_region);
		RegionEmpty(&sna->mode.shadow_cancel);
		list_init(&sna->mode.shadow_crtc);

		drmModeFreeResources(res);

		sna_cursor_pre_init(sna);
		sna_backlight_pre_init(sna);

		set_size_range(sna);
	} else {
		if (num_fake == 0)
			num_fake = 1;
	}

	if (!sna_mode_fake_init(sna, num_fake))
		return false;

	sna->mode.shadow_size = 256;
	sna->mode.shadow_events = malloc(sna->mode.shadow_size * sizeof(struct drm_event_vblank));
	if (!sna->mode.shadow_events)
		return false;

	if (!sna_probe_initial_configuration(sna)) {
		xf86CrtcConfigPtr config = XF86_CRTC_CONFIG_PTR(scrn);

		sanitize_outputs(sna);
		if (config->num_crtc && config->num_output) {
			if (!xf86ReturnOptValBool(config->output[0]->options,
						  OPTION_PRIMARY, FALSE))
				sort_config_outputs(sna);
			xf86InitialConfiguration(scrn, TRUE);
		}
	}
	sort_config_outputs(sna);

	sna_setup_provider(scrn);
	return scrn->modes != NULL;
}

bool
sna_mode_wants_tear_free(struct sna *sna)
{
	xf86CrtcConfigPtr config = XF86_CRTC_CONFIG_PTR(sna->scrn);
	bool found = false;
	FILE *file;
	int i;

	file = fopen("/sys/module/i915/parameters/enable_fbc", "r");
	if (file) {
		int fbc_enabled = 0;
		int value;

		if (fscanf(file, "%d", &value) == 1)
			fbc_enabled = value > 0;
		fclose(file);

		DBG(("%s: module parameter 'enable_fbc' enabled? %d\n",
		     __FUNCTION__, fbc_enabled));

		if (fbc_enabled)
			return true;
	}

	for (i = 0; i < sna->mode.num_real_output; i++) {
		struct sna_output *output = to_sna_output(config->output[i]);
		int id = find_property(sna, output, "Panel Self-Refresh");
		if (id == -1)
			continue;

		found = true;
		if (output->prop_values[id] != -1) {
			DBG(("%s: Panel Self-Refresh detected on %s\n",
			     __FUNCTION__, config->output[i]->name));
			return true;
		}
	}

	if (!found) {
		file = fopen("/sys/module/i915/parameters/enable_psr", "r");
		if (file) {
			int psr_enabled = 0;
			int value;

			if (fscanf(file, "%d", &value) == 1)
				psr_enabled = value > 0;
			fclose(file);

			DBG(("%s: module parameter 'enable_psr' enabled? %d\n",
			     __FUNCTION__, psr_enabled));

			if (psr_enabled)
				return true;
		}
	}

	return false;
}

void
sna_mode_set_primary(struct sna *sna)
{
#ifdef RANDR_12_INTERFACE
	xf86CrtcConfigPtr config = XF86_CRTC_CONFIG_PTR(sna->scrn);
	rrScrPrivPtr rr = rrGetScrPriv(xf86ScrnToScreen(sna->scrn));
	int i;

	if (rr == NULL || rr->primaryOutput)
		return;

	for (i = 0; i < sna->mode.num_real_output; i++) {
		xf86OutputPtr output = config->output[i];

		if (!xf86ReturnOptValBool(output->options, OPTION_PRIMARY, FALSE))
			continue;

		DBG(("%s: setting PrimaryOutput %s\n", __FUNCTION__, output->name));
		rr->primaryOutput = output->randr_output;
		RROutputChanged(rr->primaryOutput, FALSE);
		rr->layoutChanged = TRUE;
		break;
	}
#endif
}

bool
sna_mode_disable(struct sna *sna)
{
	xf86CrtcConfigPtr config = XF86_CRTC_CONFIG_PTR(sna->scrn);
	int i;

	if (sna->flags & SNA_IS_HOSTED)
		return false;

	if (!sna->scrn->vtSema)
		return false;

	sna_disable_cursors(sna->scrn);
	for (i = 0; i < sna->mode.num_real_crtc; i++)
		sna_crtc_disable(config->crtc[i], false);
	assert(sna->mode.front_active == 0);

	sna_mode_wakeup(sna);
	kgem_clean_scanout_cache(&sna->kgem);
	return true;
}

void
sna_mode_enable(struct sna *sna)
{
	xf86CrtcConfigPtr config = XF86_CRTC_CONFIG_PTR(sna->scrn);
	int i;

	DBG(("%s\n", __FUNCTION__));

	if (sna->flags & SNA_IS_HOSTED)
		return;

	if (!sna->scrn->vtSema)
		return;

	if (sna->mode.hidden) {
		DBG(("%s: hidden outputs\n", __FUNCTION__));
		return;
	}

	for (i = 0; i < sna->mode.num_real_crtc; i++) {
		xf86CrtcPtr crtc = config->crtc[i];

		DBG(("%s: crtc[%d].enabled?=%d\n", __FUNCTION__, i, crtc->enabled));
		assert(to_sna_crtc(crtc) != NULL);
		if (!crtc->enabled)
			continue;

		if (crtc->mode.Clock == 0)
			continue;

		__sna_crtc_set_mode(crtc);
	}

	update_flush_interval(sna);
	sna_cursors_reload(sna);
	sna->mode.dirty = false;
}

static void sna_randr_close(struct sna *sna)
{
	xf86CrtcConfigPtr config = XF86_CRTC_CONFIG_PTR(sna->scrn);
	int n;

	/* The RR structs are freed early during CloseScreen as they
	 * are tracked as Resources. However, we may be tempted to
	 * access them during shutdown so decouple them now.
	 */
	  for (n = 0; n < config->num_output; n++)
		  config->output[n]->randr_output = NULL;

	  for (n = 0; n < config->num_crtc; n++)
		  config->crtc[n]->randr_crtc = NULL;
}

void
sna_mode_close(struct sna *sna)
{
	sna_randr_close(sna);
	sna_mode_wakeup(sna);

	if (sna->flags & SNA_IS_HOSTED)
		return;

	sna_mode_reset(sna);

	sna_cursor_close(sna);
	sna_cursors_fini(sna);

	sna_backlight_close(sna);
	sna->mode.dirty = false;
}

void
sna_mode_fini(struct sna *sna)
{
	free(sna->mode.encoders);
}

static bool sna_box_intersect(BoxPtr r, const BoxRec *a, const BoxRec *b)
{
	r->x1 = a->x1 > b->x1 ? a->x1 : b->x1;
	r->x2 = a->x2 < b->x2 ? a->x2 : b->x2;
	if (r->x1 >= r->x2)
		return false;

	r->y1 = a->y1 > b->y1 ? a->y1 : b->y1;
	r->y2 = a->y2 < b->y2 ? a->y2 : b->y2;
	DBG(("%s: (%d, %d), (%d, %d) intersect (%d, %d), (%d, %d) = (%d, %d), (%d, %d)\n",
	     __FUNCTION__,
	     a->x1, a->y1, a->x2, a->y2,
	     b->x1, b->y1, b->x2, b->y2,
	     r->x1, r->y1, r->x2, r->y2));
	if (r->y1 >= r->y2)
		return false;

	return true;
}

static int sna_box_area(const BoxRec *box)
{
	return (int)(box->x2 - box->x1) * (int)(box->y2 - box->y1);
}

/*
 * Return the crtc covering 'box'. If two crtcs cover a portion of
 * 'box', then prefer 'desired'. If 'desired' is NULL, then prefer the crtc
 * with greater coverage
 */
xf86CrtcPtr
sna_covering_crtc(struct sna *sna, const BoxRec *box, xf86CrtcPtr desired)
{
	xf86CrtcConfigPtr config = XF86_CRTC_CONFIG_PTR(sna->scrn);
	xf86CrtcPtr best_crtc = NULL;
	int best_coverage = -1, c;

	if (sna->flags & SNA_IS_HOSTED)
		return NULL;

	/* If we do not own the VT, we do not own the CRTC either */
	if (!sna->scrn->vtSema) {
		DBG(("%s: none, VT switched\n", __FUNCTION__));
		return NULL;
	}

	if (sna->mode.hidden) {
		DBG(("%s: none, hidden outputs\n", __FUNCTION__));
		return NULL;
	}

	DBG(("%s for box=(%d, %d), (%d, %d)\n",
	     __FUNCTION__, box->x1, box->y1, box->x2, box->y2));

	if (desired == NULL) {
		ScreenPtr screen = xf86ScrnToScreen(sna->scrn);
		rrScrPrivPtr rr = rrGetScrPriv(screen);
		if (rr && rr->primaryOutput && rr->primaryOutput->pScreen == screen) {
			xf86OutputPtr output = rr->primaryOutput->devPrivate;
			DBG(("%s: have PrimaryOutput? %d marking as desired\n", __FUNCTION__, output->crtc != NULL));
			desired = output->crtc;
		}
		assert(!desired || desired->scrn == sna->scrn);
	}
	if (desired && to_sna_crtc(desired) && to_sna_crtc(desired)->bo) {
		BoxRec cover_box;
		if (sna_box_intersect(&cover_box, &desired->bounds, box)) {
			DBG(("%s: box overlaps desired crtc: (%d, %d), (%d, %d)\n",
			     __FUNCTION__,
			     cover_box.x1, cover_box.y1,
			     cover_box.x2, cover_box.y2));
			return desired;
		}
		best_crtc = desired;
		best_coverage = 0;
	}

	for (c = 0; c < sna->mode.num_real_crtc; c++) {
		xf86CrtcPtr crtc = config->crtc[c];
		BoxRec cover_box;
		int coverage;

		assert(to_sna_crtc(crtc));

		/* If the CRTC is off, treat it as not covering */
		if (to_sna_crtc(crtc)->bo == NULL) {
			DBG(("%s: crtc %d off, skipping\n", __FUNCTION__, c));
			continue;
		}

		DBG(("%s: crtc %d: (%d, %d), (%d, %d)\n",
		     __FUNCTION__, c,
		     crtc->bounds.x1, crtc->bounds.y1,
		     crtc->bounds.x2, crtc->bounds.y2));
		if (*(const uint64_t *)box == *(uint64_t *)&crtc->bounds) {
			DBG(("%s: box exactly matches crtc [%d]\n",
			     __FUNCTION__, c));
			return crtc;
		}

		coverage = 0;
		if (sna_box_intersect(&cover_box, &crtc->bounds, box))
			coverage = sna_box_area(&cover_box);

		DBG(("%s: box instersects (%d, %d), (%d, %d) of crtc %d\n",
		     __FUNCTION__,
		     cover_box.x1, cover_box.y1,
		     cover_box.x2, cover_box.y2,
		     c));

		DBG(("%s: box covers %d of crtc %d\n",
		     __FUNCTION__, coverage, c));
		if (coverage > best_coverage) {
			best_crtc = crtc;
			best_coverage = coverage;
		}
	}
	DBG(("%s: best crtc = %p, coverage = %d\n",
	     __FUNCTION__, best_crtc, best_coverage));
	return best_crtc;
}

static xf86CrtcPtr first_active_crtc(struct sna *sna)
{
	xf86CrtcConfigPtr config = XF86_CRTC_CONFIG_PTR(sna->scrn);
	int n;

	for (n = 0; n < sna->mode.num_real_crtc; n++) {
		xf86CrtcPtr crtc = config->crtc[n];
		if (to_sna_crtc(crtc)->bo)
			return crtc;
	}

	/* No active, use the first as a placeholder */
	if (sna->mode.num_real_crtc)
		return config->crtc[0];

	return NULL;
}

xf86CrtcPtr sna_primary_crtc(struct sna *sna)
{
	rrScrPrivPtr rr = rrGetScrPriv(xf86ScrnToScreen(sna->scrn));
	if (rr && rr->primaryOutput) {
		xf86OutputPtr output = rr->primaryOutput->devPrivate;
		if (output->crtc &&
		    output->scrn == sna->scrn &&
		    to_sna_crtc(output->crtc))
			return output->crtc;
	}

	return first_active_crtc(sna);
}

#define MI_LOAD_REGISTER_IMM			(0x22<<23)

static bool sna_emit_wait_for_scanline_hsw(struct sna *sna,
					   xf86CrtcPtr crtc,
					   int pipe, int y1, int y2,
					   bool full_height)
{
	uint32_t event;
	uint32_t *b;

	if (!sna->kgem.has_secure_batches)
		return false;

	b = kgem_get_batch(&sna->kgem);
	sna->kgem.nbatch += 17;

	switch (pipe) {
	default: assert(0);
	case 0: event = 1 << 0; break;
	case 1: event = 1 << 8; break;
	case 2: event = 1 << 14; break;
	}

	b[0] = MI_LOAD_REGISTER_IMM | 1;
	b[1] = 0x44050; /* DERRMR */
	b[2] = ~event;
	b[3] = MI_LOAD_REGISTER_IMM | 1;
	b[4] = 0xa188; /* FORCEWAKE_MT */
	b[5] = 2 << 16 | 2;

	/* The documentation says that the LOAD_SCAN_LINES command
	 * always comes in pairs. Don't ask me why. */
	switch (pipe) {
	default: assert(0);
	case 0: event = 0 << 19; break;
	case 1: event = 1 << 19; break;
	case 2: event = 4 << 19; break;
	}
	b[8] = b[6] = MI_LOAD_SCAN_LINES_INCL | event;
	b[9] = b[7] = (y1 << 16) | (y2-1);

	switch (pipe) {
	default: assert(0);
	case 0: event = 1 << 0; break;
	case 1: event = 1 << 8; break;
	case 2: event = 1 << 14; break;
	}
	b[10] = MI_WAIT_FOR_EVENT | event;

	b[11] = MI_LOAD_REGISTER_IMM | 1;
	b[12] = 0xa188; /* FORCEWAKE_MT */
	b[13] = 2 << 16;
	b[14] = MI_LOAD_REGISTER_IMM | 1;
	b[15] = 0x44050; /* DERRMR */
	b[16] = ~0;

	sna->kgem.batch_flags |= I915_EXEC_SECURE;
	return true;
}

static bool sna_emit_wait_for_scanline_ivb(struct sna *sna,
					   xf86CrtcPtr crtc,
					   int pipe, int y1, int y2,
					   bool full_height)
{
	uint32_t event, *b;

	if (!sna->kgem.has_secure_batches)
		return false;

	assert(y1 >= 0);
	assert(y2 > y1);
	assert(sna->kgem.mode);

	/* Always program one less than the desired value */
	if (--y1 < 0)
		y1 = crtc->bounds.y2;
	y2--;

	switch (pipe) {
	default:
		assert(0);
	case 0:
		event = 1 << (full_height ? 3 : 0);
		break;
	case 1:
		event = 1 << (full_height ? 11 : 8);
		break;
	case 2:
		event = 1 << (full_height ? 21 : 14);
		break;
	}

	b = kgem_get_batch(&sna->kgem);

	/* Both the LRI and WAIT_FOR_EVENT must be in the same cacheline */
	if (((sna->kgem.nbatch + 6) >> 4) != (sna->kgem.nbatch + 10) >> 4) {
		int dw = sna->kgem.nbatch + 6;
		dw = ALIGN(dw, 16) - dw;
		while (dw--)
			*b++ = MI_NOOP;
	}

	b[0] = MI_LOAD_REGISTER_IMM | 1;
	b[1] = 0x44050; /* DERRMR */
	b[2] = ~event;
	b[3] = MI_LOAD_REGISTER_IMM | 1;
	b[4] = 0xa188; /* FORCEWAKE_MT */
	b[5] = 2 << 16 | 2;
	b[6] = MI_LOAD_REGISTER_IMM | 1;
	b[7] = 0x70068 + 0x1000 * pipe;
	b[8] = (1 << 31) | (1 << 30) | (y1 << 16) | y2;
	b[9] = MI_WAIT_FOR_EVENT | event;
	b[10] = MI_LOAD_REGISTER_IMM | 1;
	b[11] = 0xa188; /* FORCEWAKE_MT */
	b[12] = 2 << 16;
	b[13] = MI_LOAD_REGISTER_IMM | 1;
	b[14] = 0x44050; /* DERRMR */
	b[15] = ~0;

	sna->kgem.nbatch = b - sna->kgem.batch + 16;

	sna->kgem.batch_flags |= I915_EXEC_SECURE;
	return true;
}

static bool sna_emit_wait_for_scanline_gen6(struct sna *sna,
					    xf86CrtcPtr crtc,
					    int pipe, int y1, int y2,
					    bool full_height)
{
	uint32_t *b;
	uint32_t event;

	if (!sna->kgem.has_secure_batches)
		return false;

	assert(y1 >= 0);
	assert(y2 > y1);
	assert(sna->kgem.mode == KGEM_RENDER);

	/* Always program one less than the desired value */
	if (--y1 < 0)
		y1 = crtc->bounds.y2;
	y2--;

	/* The scanline granularity is 3 bits */
	y1 &= ~7;
	y2 &= ~7;
	if (y2 == y1)
		return false;

	event = 1 << (3*full_height + pipe*8);

	b = kgem_get_batch(&sna->kgem);
	sna->kgem.nbatch += 16;

	b[0] = MI_LOAD_REGISTER_IMM | 1;
	b[1] = 0x44050; /* DERRMR */
	b[2] = ~event;
	b[3] = MI_LOAD_REGISTER_IMM | 1;
	b[4] = 0x4f100; /* magic */
	b[5] = (1 << 31) | (1 << 30) | pipe << 29 | (y1 << 16) | y2;
	b[6] = MI_LOAD_REGISTER_IMM | 1;
	b[7] = 0x2050; /* PSMI_CTL(rcs) */
	b[8] = 1 << 16 | 1;
	b[9] = MI_WAIT_FOR_EVENT | event;
	b[10] = MI_LOAD_REGISTER_IMM | 1;
	b[11] = 0x2050; /* PSMI_CTL(rcs) */
	b[12] = 1 << 16;
	b[13] = MI_LOAD_REGISTER_IMM | 1;
	b[14] = 0x44050; /* DERRMR */
	b[15] = ~0;

	sna->kgem.batch_flags |= I915_EXEC_SECURE;
	return true;
}

static bool sna_emit_wait_for_scanline_gen4(struct sna *sna,
					    xf86CrtcPtr crtc,
					    int pipe, int y1, int y2,
					    bool full_height)
{
	uint32_t event;
	uint32_t *b;

	if (pipe == 0) {
		if (full_height)
			event = MI_WAIT_FOR_PIPEA_SVBLANK;
		else
			event = MI_WAIT_FOR_PIPEA_SCAN_LINE_WINDOW;
	} else {
		if (full_height)
			event = MI_WAIT_FOR_PIPEB_SVBLANK;
		else
			event = MI_WAIT_FOR_PIPEB_SCAN_LINE_WINDOW;
	}

	b = kgem_get_batch(&sna->kgem);
	sna->kgem.nbatch += 5;

	/* The documentation says that the LOAD_SCAN_LINES command
	 * always comes in pairs. Don't ask me why. */
	b[2] = b[0] = MI_LOAD_SCAN_LINES_INCL | pipe << 20;
	b[3] = b[1] = (y1 << 16) | (y2-1);
	b[4] = MI_WAIT_FOR_EVENT | event;

	return true;
}

static bool sna_emit_wait_for_scanline_gen2(struct sna *sna,
					    xf86CrtcPtr crtc,
					    int pipe, int y1, int y2,
					    bool full_height)
{
	uint32_t *b;

	/*
	 * Pre-965 doesn't have SVBLANK, so we need a bit
	 * of extra time for the blitter to start up and
	 * do its job for a full height blit
	 */
	if (full_height)
		y2 -= 2;

	b = kgem_get_batch(&sna->kgem);
	sna->kgem.nbatch += 5;

	/* The documentation says that the LOAD_SCAN_LINES command
	 * always comes in pairs. Don't ask me why. */
	b[2] = b[0] = MI_LOAD_SCAN_LINES_INCL | pipe << 20;
	b[3] = b[1] = (y1 << 16) | (y2-1);
	b[4] = MI_WAIT_FOR_EVENT | 1 << (1 + 4*pipe);

	return true;
}

bool
sna_wait_for_scanline(struct sna *sna,
		      PixmapPtr pixmap,
		      xf86CrtcPtr crtc,
		      const BoxRec *clip)
{
	bool full_height;
	int y1, y2, pipe;
	bool ret;

	assert(crtc != NULL);
	assert(to_sna_crtc(crtc) != NULL);
	assert(to_sna_crtc(crtc)->bo != NULL);
	assert(pixmap == sna->front);

	if (sna->flags & SNA_NO_VSYNC)
		return false;

	/*
	 * Make sure we don't wait for a scanline that will
	 * never occur
	 */
	y1 = clip->y1 - crtc->bounds.y1;
	if (y1 < 0)
		y1 = 0;
	y2 = clip->y2 - crtc->bounds.y1;
	if (y2 > crtc->bounds.y2 - crtc->bounds.y1)
		y2 = crtc->bounds.y2 - crtc->bounds.y1;
	DBG(("%s: clipped range = %d, %d\n", __FUNCTION__, y1, y2));
	if (y2 <= y1 + 4)
		return false;

	full_height = y1 == 0 && y2 == crtc->bounds.y2 - crtc->bounds.y1;

	if (crtc->mode.Flags & V_INTERLACE) {
		/* DSL count field lines */
		y1 /= 2;
		y2 /= 2;
	}

	pipe = sna_crtc_pipe(crtc);
	DBG(("%s: pipe=%d, y1=%d, y2=%d, full_height?=%d\n",
	     __FUNCTION__, pipe, y1, y2, full_height));

	if (sna->kgem.gen >= 0110)
		ret = false;
	else if (sna->kgem.gen == 0101)
		ret = false; /* chv, vsync method unknown */
	else if (sna->kgem.gen >= 075)
		ret = sna_emit_wait_for_scanline_hsw(sna, crtc, pipe, y1, y2, full_height);
	else if (sna->kgem.gen == 071)
		ret = false; /* vlv, vsync method unknown */
	else if (sna->kgem.gen >= 070)
		ret = sna_emit_wait_for_scanline_ivb(sna, crtc, pipe, y1, y2, full_height);
	else if (sna->kgem.gen >= 060)
		ret =sna_emit_wait_for_scanline_gen6(sna, crtc, pipe, y1, y2, full_height);
	else if (sna->kgem.gen >= 040)
		ret = sna_emit_wait_for_scanline_gen4(sna, crtc, pipe, y1, y2, full_height);
	else
		ret = sna_emit_wait_for_scanline_gen2(sna, crtc, pipe, y1, y2, full_height);

	return ret;
}

static bool sna_mode_shutdown_crtc(xf86CrtcPtr crtc)
{
	struct sna *sna = to_sna(crtc->scrn);
	xf86CrtcConfigPtr config = XF86_CRTC_CONFIG_PTR(crtc->scrn);
	bool disabled = false;
	int o;

	xf86DrvMsg(crtc->scrn->scrnIndex, X_ERROR,
		   "%s: invalid state found on pipe %d, disabling CRTC:%d\n",
		   __FUNCTION__,
		   __sna_crtc_pipe(to_sna_crtc(crtc)),
		   __sna_crtc_id(to_sna_crtc(crtc)));
	sna_crtc_disable(crtc, true);
#if XF86_CRTC_VERSION >= 3
	crtc->active = FALSE;
#endif
	if (crtc->enabled) {
		crtc->enabled = FALSE;
		disabled = true;
	}

	for (o = 0; o < sna->mode.num_real_output; o++) {
		xf86OutputPtr output = config->output[o];

		if (output->crtc != crtc)
			continue;

		output->funcs->dpms(output, DPMSModeOff);
		output->crtc = NULL;
	}

	return disabled;
}

static bool
sna_mode_disable_secondary_planes(struct sna *sna)
{
	xf86CrtcConfigPtr config = XF86_CRTC_CONFIG_PTR(sna->scrn);
	bool disabled = false;
	int c;

	/* Disable all secondary planes on our CRTCs, just in case
	 * other userspace left garbage in them.
	 */
	for (c = 0; c < sna->mode.num_real_crtc; c++) {
		xf86CrtcPtr crtc = config->crtc[c];
		struct sna_crtc *sna_crtc = to_sna_crtc(crtc);
		struct plane *plane;

		list_for_each_entry(plane, &sna_crtc->sprites, link) {
			struct local_mode_get_plane p;
			struct local_mode_set_plane s;

			VG_CLEAR(p);
			p.plane_id = plane->id;
			p.count_format_types = 0;
			if (drmIoctl(sna->kgem.fd,
				     LOCAL_IOCTL_MODE_GETPLANE,
				     &p))
				continue;

			if (p.fb_id == 0 || p.crtc_id == 0)
				continue;

			memset(&s, 0, sizeof(s));
			s.plane_id = p.plane_id;
			s.crtc_id = p.crtc_id;
			if (drmIoctl(sna->kgem.fd,
				     LOCAL_IOCTL_MODE_SETPLANE,
				     &s))
				disabled |= sna_mode_shutdown_crtc(crtc);
		}
	}

	return disabled;
}

void sna_mode_check(struct sna *sna)
{
	xf86CrtcConfigPtr config = XF86_CRTC_CONFIG_PTR(sna->scrn);
	bool disabled;
	int c, o;

	if (sna->flags & SNA_IS_HOSTED)
		return;

	DBG(("%s: hidden?=%d\n", __FUNCTION__, sna->mode.hidden));
	if (sna->mode.hidden)
		return;

	disabled = sna_mode_disable_secondary_planes(sna);

	/* Validate CRTC attachments and force consistency upon the kernel */
	for (c = 0; c < sna->mode.num_real_crtc; c++) {
		xf86CrtcPtr crtc = config->crtc[c];
		struct sna_crtc *sna_crtc = to_sna_crtc(crtc);
		struct drm_mode_crtc mode;
		uint32_t expected[2];

		assert(sna_crtc);

#if XF86_CRTC_VERSION >= 3
		assert(sna_crtc->bo == NULL || crtc->active);
#endif
		expected[0] = sna_crtc->bo ? fb_id(sna_crtc->bo) : 0;
		expected[1] = sna_crtc->flip_bo ? fb_id(sna_crtc->flip_bo) : -1;

		VG_CLEAR(mode);
		mode.crtc_id = __sna_crtc_id(sna_crtc);
		if (drmIoctl(sna->kgem.fd, DRM_IOCTL_MODE_GETCRTC, &mode))
			continue;

		DBG(("%s: crtc=%d, valid?=%d, fb attached?=%d, expected=(%d or %d)\n",
		     __FUNCTION__,
		     mode.crtc_id, mode.mode_valid,
		     mode.fb_id, expected[0], expected[1]));

		if (mode.fb_id != expected[0] && mode.fb_id != expected[1])
			disabled |= sna_mode_shutdown_crtc(crtc);
	}

	for (o = 0; o < config->num_output; o++) {
		xf86OutputPtr output = config->output[o];
		struct sna_output *sna_output;

		if (output->crtc)
			continue;

		sna_output = to_sna_output(output);
		if (sna_output == NULL)
			continue;

		sna_output->dpms_mode = DPMSModeOff;
	}

	update_flush_interval(sna);

	if (disabled)
		xf86RandR12TellChanged(xf86ScrnToScreen(sna->scrn));
}

static bool
sna_crtc_hide_planes(struct sna *sna, struct sna_crtc *crtc)
{
	struct local_mode_set_plane s;
	struct plane *plane;

	if (crtc->primary.id == 0)
		return false;

	memset(&s, 0, sizeof(s));
	s.plane_id = crtc->primary.id;
	if (drmIoctl(sna->kgem.fd, LOCAL_IOCTL_MODE_SETPLANE, &s))
		return false;

	list_for_each_entry(plane, &crtc->sprites, link) {
		s.plane_id = plane->id;
		(void)drmIoctl(sna->kgem.fd, LOCAL_IOCTL_MODE_SETPLANE, &s);
	}

	__sna_crtc_disable(sna, crtc);
	return true;
}

void sna_mode_reset(struct sna *sna)
{
	xf86CrtcConfigPtr config = XF86_CRTC_CONFIG_PTR(sna->scrn);
	int i;

	if (sna->flags & SNA_IS_HOSTED)
		return;

	DBG(("%s\n", __FUNCTION__));

	sna_disable_cursors(sna->scrn);
	for (i = 0; i < sna->mode.num_real_crtc; i++)
		if (!sna_crtc_hide_planes(sna, to_sna_crtc(config->crtc[i])))
			sna_crtc_disable(config->crtc[i], true);
	assert(sna->mode.front_active == 0);

	for (i = 0; i < sna->mode.num_real_crtc; i++) {
		struct sna_crtc *sna_crtc = to_sna_crtc(config->crtc[i]);
		struct plane *plane;

		assert(sna_crtc != NULL);

		/* Force the rotation property to be reset on next use */
		rotation_reset(&sna_crtc->primary);
		list_for_each_entry(plane, &sna_crtc->sprites, link)
			rotation_reset(plane);
	}

	/* VT switching, likely to be fbcon so make the backlight usable */
	for (i = 0; i < sna->mode.num_real_output; i++) {
		struct sna_output *sna_output = to_sna_output(config->output[i]);

		assert(sna_output != NULL);
		if (sna_output->dpms_mode != DPMSModeOff)
			continue;

		if (!sna_output->backlight.iface)
			continue;

		sna_output_backlight_set(sna_output,
					 sna_output->backlight_active_level);
	}

	/* drain the event queue */
	sna_mode_wakeup(sna);
}

static void transformed_box(BoxRec *box, xf86CrtcPtr crtc)
{
	box->x1 -= crtc->filter_width >> 1;
	box->x2 += crtc->filter_width >> 1;
	box->y1 -= crtc->filter_height >> 1;
	box->y2 += crtc->filter_height >> 1;

	pixman_f_transform_bounds(&crtc->f_framebuffer_to_crtc, box);

	if (box->x1 < 0)
		box->x1 = 0;
	if (box->y1 < 0)
		box->y1 = 0;
	if (box->x2 > crtc->mode.HDisplay)
		box->x2 = crtc->mode.HDisplay;
	if (box->y2 > crtc->mode.VDisplay)
		box->y2 = crtc->mode.VDisplay;
}

inline static DrawablePtr crtc_source(xf86CrtcPtr crtc, int16_t *sx, int16_t *sy)
{
	struct sna_crtc *sna_crtc = to_sna_crtc(crtc);
	if (sna_crtc->slave_pixmap) {
		DBG(("%s: using slave pixmap=%ld, offset (%d, %d)\n",
		     __FUNCTION__,
		     sna_crtc->slave_pixmap->drawable.serialNumber,
		 -crtc->x, -crtc->y));
		*sx = -crtc->x;
		*sy = -crtc->y;
		return &sna_crtc->slave_pixmap->drawable;
	} else {
		DBG(("%s: using Screen pixmap=%ld\n",
		     __FUNCTION__,
		     to_sna(crtc->scrn)->front->drawable.serialNumber));
		*sx = *sy = 0;
		return &to_sna(crtc->scrn)->front->drawable;
	}
}

static void
sna_crtc_redisplay__fallback(xf86CrtcPtr crtc, RegionPtr region, struct kgem_bo *bo)
{
	int16_t sx, sy;
	struct sna *sna = to_sna(crtc->scrn);
	ScreenPtr screen = xf86ScrnToScreen(crtc->scrn);
	DrawablePtr draw = crtc_source(crtc, &sx, &sy);
	PictFormatPtr format;
	PictTransform T;
	PicturePtr src, dst;
	PixmapPtr pixmap;
	int depth, error;
	void *ptr;

	DBG(("%s: compositing transformed damage boxes, target handle=%d\n", __FUNCTION__, bo->handle));

	error = sna_render_format_for_depth(draw->depth);
	depth = PIXMAN_FORMAT_DEPTH(error);
	format = PictureMatchFormat(screen, depth, error);
	if (format == NULL) {
		DBG(("%s: can't find format for depth=%d [%08x]\n",
		     __FUNCTION__, depth, error));
		return;
	}

	DBG(("%s: dst format=%08x, depth=%d, bpp=%d, pitch=%d, size=%dx%d\n",
	     __FUNCTION__, format->format, depth, draw->bitsPerPixel,
	     bo->pitch, crtc->mode.HDisplay, crtc->mode.VDisplay));

	if (sx | sy)
		RegionTranslate(region, sx, sy);
	error = !sna_drawable_move_region_to_cpu(draw, region, MOVE_READ);
	if (sx | sy)
		RegionTranslate(region, -sx, -sy);
	if (error)
		return;

	ptr = kgem_bo_map__gtt(&sna->kgem, bo);
	if (ptr == NULL)
		return;

	pixmap = sna_pixmap_create_unattached(screen, 0, 0, depth);
	if (pixmap == NullPixmap)
		return;

	if (!screen->ModifyPixmapHeader(pixmap,
					crtc->mode.HDisplay, crtc->mode.VDisplay,
					depth, draw->bitsPerPixel,
					bo->pitch, ptr))
		goto free_pixmap;

	src = CreatePicture(None, draw, format,
			    0, NULL, serverClient, &error);
	if (!src)
		goto free_pixmap;

	pixman_transform_init_translate(&T, sx << 16, sy << 16);
	pixman_transform_multiply(&T, &T, &crtc->crtc_to_framebuffer);
	if (!sna_transform_is_integer_translation(&T, &sx, &sy)) {
#define f2d(x) (((double)(x))/65536.)
		DBG(("%s: transform=[[%f %f %f], [%f %f %f], [%f %f %f]] (raw [[%x %x %x], [%x %x %x], [%x %x %x]])\n",
		     __FUNCTION__,
		     f2d(T.matrix[0][0]),
		     f2d(T.matrix[0][1]),
		     f2d(T.matrix[0][2]),
		     f2d(T.matrix[1][0]),
		     f2d(T.matrix[1][1]),
		     f2d(T.matrix[1][2]),
		     f2d(T.matrix[2][0]),
		     f2d(T.matrix[2][1]),
		     f2d(T.matrix[2][2]),
		     T.matrix[0][0],
		     T.matrix[0][1],
		     T.matrix[0][2],
		     T.matrix[1][0],
		     T.matrix[1][1],
		     T.matrix[1][2],
		     T.matrix[2][0],
		     T.matrix[2][1],
		     T.matrix[2][2]));
#undef f2d

		error = SetPictureTransform(src, &T);
		if (error)
			goto free_src;
		sx = sy = 0;
	}

	if (crtc->filter && crtc->transform_in_use)
		SetPicturePictFilter(src, crtc->filter,
				     crtc->params, crtc->nparams);

	dst = CreatePicture(None, &pixmap->drawable, format,
			    0, NULL, serverClient, &error);
	if (!dst)
		goto free_src;

	kgem_bo_sync__gtt(&sna->kgem, bo);

	if (sigtrap_get() == 0) { /* paranoia */
		const BoxRec *b = region_rects(region);
		int n = region_num_rects(region);
		do {
			BoxRec box;

			box = *b++;
			transformed_box(&box, crtc);

			DBG(("%s: (%d, %d)x(%d, %d) -> (%d, %d), (%d, %d)\n",
			     __FUNCTION__,
			     b[-1].x1, b[-1].y1, b[-1].x2-b[-1].x1, b[-1].y2-b[-1].y1,
			     box.x1, box.y1, box.x2, box.y2));

			fbComposite(PictOpSrc, src, NULL, dst,
				    box.x1 + sx, box.y1 + sy,
				    0, 0,
				    box.x1, box.y1,
				    box.x2 - box.x1, box.y2 - box.y1);
		} while (--n);
		sigtrap_put();
	}

	FreePicture(dst, None);
free_src:
	FreePicture(src, None);
free_pixmap:
	screen->DestroyPixmap(pixmap);
}

static void
sna_crtc_redisplay__composite(xf86CrtcPtr crtc, RegionPtr region, struct kgem_bo *bo)
{
	int16_t sx, sy;
	struct sna *sna = to_sna(crtc->scrn);
	ScreenPtr screen = xf86ScrnToScreen(crtc->scrn);
	DrawablePtr draw = crtc_source(crtc, &sx, &sy);
	struct sna_composite_op tmp;
	PictFormatPtr format;
	PictTransform T;
	PicturePtr src, dst;
	PixmapPtr pixmap;
	const BoxRec *b;
	int n, depth, error;

	DBG(("%s: compositing transformed damage boxes\n", __FUNCTION__));

	error = sna_render_format_for_depth(draw->depth);
	depth = PIXMAN_FORMAT_DEPTH(error);
	format = PictureMatchFormat(screen, depth, error);
	if (format == NULL) {
		DBG(("%s: can't find format for depth=%d [%08x]\n",
		     __FUNCTION__, depth, error));
		return;
	}

	DBG(("%s: dst format=%08x, depth=%d, bpp=%d, pitch=%d, size=%dx%d\n",
	     __FUNCTION__, format->format, depth, draw->bitsPerPixel,
	     bo->pitch, crtc->mode.HDisplay, crtc->mode.VDisplay));

	pixmap = sna_pixmap_create_unattached(screen, 0, 0, depth);
	if (pixmap == NullPixmap)
		return;

	if (!screen->ModifyPixmapHeader(pixmap,
					crtc->mode.HDisplay, crtc->mode.VDisplay,
					depth, draw->bitsPerPixel,
					bo->pitch, NULL))
		goto free_pixmap;

	if (!sna_pixmap_attach_to_bo(pixmap, kgem_bo_reference(bo))) {
		kgem_bo_destroy(&sna->kgem, bo);
		goto free_pixmap;
	}

	src = CreatePicture(None, draw, format,
			    0, NULL, serverClient, &error);
	if (!src)
		goto free_pixmap;

	pixman_transform_init_translate(&T, sx << 16, sy << 16);
	pixman_transform_multiply(&T, &T, &crtc->crtc_to_framebuffer);
	if (!sna_transform_is_integer_translation(&T, &sx, &sy)) {
		error = SetPictureTransform(src, &T);
		if (error)
			goto free_src;
		sx = sy = 0;
	}

	if (crtc->filter && crtc->transform_in_use)
		SetPicturePictFilter(src, crtc->filter,
				     crtc->params, crtc->nparams);

	dst = CreatePicture(None, &pixmap->drawable, format,
			    0, NULL, serverClient, &error);
	if (!dst)
		goto free_src;

	ValidatePicture(src);
	ValidatePicture(dst);

	/* Composite each box individually as if we are dealing with a rotation
	 * on a large display, we may have to perform intermediate copies. We
	 * can then minimise the overdraw by looking at individual boxes rather
	 * than the bbox.
	 */
	n = region_num_rects(region);
	b = region_rects(region);
	do {
		BoxRec box = *b;
		transformed_box(&box, crtc);

		DBG(("%s: (%d, %d)x(%d, %d) -> (%d, %d), (%d, %d)\n",
		     __FUNCTION__,
		     b->x1, b->y1, b->x2-b->x1, b->y2-b->y1,
		     box.x1, box.y1, box.x2, box.y2));

		if (!sna->render.composite(sna,
					   PictOpSrc, src, NULL, dst,
					   sx + box.x1, sy + box.y1,
					   0, 0,
					   box.x1, box.y1,
					   box.x2 - box.x1, box.y2 - box.y1,
					   0, memset(&tmp, 0, sizeof(tmp)))) {
			DBG(("%s: unsupported operation!\n", __FUNCTION__));
			sna_crtc_redisplay__fallback(crtc, region, bo);
			break;
		} else {
			tmp.box(sna, &tmp, &box);
			tmp.done(sna, &tmp);
		}
	} while (b++, --n);

	FreePicture(dst, None);
free_src:
	FreePicture(src, None);
free_pixmap:
	screen->DestroyPixmap(pixmap);
}

static void
sna_crtc_redisplay(xf86CrtcPtr crtc, RegionPtr region, struct kgem_bo *bo)
{
	int16_t tx, ty, sx, sy;
	struct sna *sna = to_sna(crtc->scrn);
	DrawablePtr draw = crtc_source(crtc, &sx, &sy);
	struct sna_pixmap *priv = sna_pixmap((PixmapPtr)draw);

	DBG(("%s: crtc %d [pipe=%d], damage (%d, %d), (%d, %d) x %d\n",
	     __FUNCTION__, sna_crtc_id(crtc), sna_crtc_pipe(crtc),
	     region->extents.x1, region->extents.y1,
	     region->extents.x2, region->extents.y2,
	     region_num_rects(region)));

	if (wedged(sna))
		goto fallback;

	if (priv->clear) {
		RegionRec whole;

		DBG(("%s: clear damage boxes\n", __FUNCTION__));

		if (sna_transform_is_integer_translation(&crtc->crtc_to_framebuffer,
							 &tx, &ty)) {
			RegionTranslate(region, -tx, -ty);
		} else {
			whole.extents = region->extents;
			whole.data = NULL;
			transformed_box(&whole.extents, crtc);
			region = &whole;
		}
		if (sna_blt_fill_boxes(sna, GXcopy,
				       bo, draw->bitsPerPixel,
				       priv->clear_color,
				       region_rects(region),
				       region_num_rects(region)))
			return;
	}

	if (crtc->filter == NULL &&
	    priv->gpu_bo &&
	    priv->cpu_damage == NULL &&
	    sna_transform_is_integer_translation(&crtc->crtc_to_framebuffer,
						 &tx, &ty)) {
		DrawableRec tmp;

		DBG(("%s: copy damage boxes\n", __FUNCTION__));

		tmp.width = crtc->mode.HDisplay;
		tmp.height = crtc->mode.VDisplay;
		tmp.depth = sna->front->drawable.depth;
		tmp.bitsPerPixel = sna->front->drawable.bitsPerPixel;

		if (sna->render.copy_boxes(sna, GXcopy,
					   draw, priv->gpu_bo, sx, sy,
					   &tmp, bo, -tx, -ty,
					   region_rects(region), region_num_rects(region), 0))
			return;
	}

	if (can_render(sna)) {
		sna_crtc_redisplay__composite(crtc, region, bo);
		return;
	}

fallback:
	sna_crtc_redisplay__fallback(crtc, region, bo);
}

static void shadow_flip_handler(struct drm_event_vblank *e,
				void *data)
{
	struct sna *sna = data;

	sna->timer_active |= 1 << FLUSH_TIMER;
	sna->timer_expire[FLUSH_TIMER] =
		e->tv_sec * 1000 + e->tv_usec / 1000 +
		sna->vblank_interval / 2;
}

void sna_shadow_set_crtc(struct sna *sna,
			 xf86CrtcPtr crtc,
			 struct kgem_bo *bo)
{
	struct sna_crtc *sna_crtc = to_sna_crtc(crtc);
	struct sna_pixmap *priv;

	assert(sna_crtc);
	DBG(("%s: setting shadow override for CRTC:%d to handle=%d\n",
	     __FUNCTION__, __sna_crtc_id(sna_crtc), bo->handle));

	assert(sna->flags & SNA_TEAR_FREE);
	assert(!sna_crtc->transform);

	if (sna_crtc->client_bo != bo) {
		if (sna_crtc->client_bo) {
			assert(sna_crtc->client_bo->refcnt >= sna_crtc->client_bo->active_scanout);
			sna_crtc->client_bo->active_scanout--;
			kgem_bo_destroy(&sna->kgem, sna_crtc->client_bo);
		}

		sna_crtc->client_bo = kgem_bo_reference(bo);
		sna_crtc->client_bo->active_scanout++;
		assert(sna_crtc->client_bo->refcnt >= sna_crtc->client_bo->active_scanout);
		sna_crtc_damage(crtc);
	}

	list_move(&sna_crtc->shadow_link, &sna->mode.shadow_crtc);
	sna->mode.shadow_dirty = true;

	priv = sna_pixmap(sna->front);
	assert(priv->gpu_bo);
	priv->move_to_gpu = wait_for_shadow;
	priv->move_to_gpu_data = sna;
}

void sna_shadow_steal_crtcs(struct sna *sna, struct list *list)
{
	list_init(list);
	while (!list_is_empty(&sna->mode.shadow_crtc)) {
		RegionRec sub, *damage;
		struct sna_crtc *crtc =
			list_first_entry(&sna->mode.shadow_crtc,
					 struct sna_crtc,
					 shadow_link);

		damage = DamageRegion(sna->mode.shadow_damage);
		sub.extents = crtc->base->bounds;
		sub.data = NULL;
		RegionSubtract(damage, damage, &sub);

		list_move(&crtc->shadow_link, list);
	}
}

void sna_shadow_unsteal_crtcs(struct sna *sna, struct list *list)
{
	while (!list_is_empty(list)) {
		struct sna_crtc *crtc =
			list_first_entry(list,
					 struct sna_crtc,
					 shadow_link);
		assert(crtc->client_bo);
		sna_shadow_set_crtc(sna, crtc->base, crtc->client_bo);
	}
}

void sna_shadow_unset_crtc(struct sna *sna,
			   xf86CrtcPtr crtc)
{
	struct sna_crtc *sna_crtc = to_sna_crtc(crtc);

	DBG(("%s: clearin shadow override for CRTC:%d\n",
	     __FUNCTION__, __sna_crtc_id(sna_crtc)));

	if (sna_crtc->client_bo == NULL)
		return;

	assert(sna_crtc->client_bo->refcnt >= sna_crtc->client_bo->active_scanout);
	sna_crtc->client_bo->active_scanout--;
	kgem_bo_destroy(&sna->kgem, sna_crtc->client_bo);
	sna_crtc->client_bo = NULL;
	list_del(&sna_crtc->shadow_link);
	sna->mode.shadow_dirty = true;

	sna_crtc_damage(crtc);
}

static bool move_crtc_to_gpu(struct sna *sna)
{
	xf86CrtcConfigPtr config = XF86_CRTC_CONFIG_PTR(sna->scrn);
	int i;

	for (i = 0; i < sna->mode.num_real_crtc; i++) {
		struct sna_crtc *crtc = to_sna_crtc(config->crtc[i]);
		unsigned hint;

		assert(crtc);

		if (crtc->bo == NULL)
			continue;

		if (crtc->slave_pixmap)
			continue;

		if (crtc->client_bo)
			continue;

		if (crtc->shadow_bo)
			continue;

		hint = MOVE_READ | MOVE_ASYNC_HINT | __MOVE_SCANOUT;
		if (sna->flags & SNA_TEAR_FREE)
			hint |= __MOVE_FORCE;

		DBG(("%s: CRTC %d [pipe=%d] requires frontbuffer\n",
		     __FUNCTION__, __sna_crtc_id(crtc), __sna_crtc_pipe(crtc)));
		return sna_pixmap_move_to_gpu(sna->front, hint);
	}

	return true;
}

void sna_mode_redisplay(struct sna *sna)
{
	xf86CrtcConfigPtr config = XF86_CRTC_CONFIG_PTR(sna->scrn);
	RegionPtr region;
	int i;

	if (sna->mode.hidden) {
		DBG(("%s: hidden outputs, skipping\n", __FUNCTION__));
		return;
	}

	if (!sna->mode.shadow_enabled)
		return;

	assert(sna->mode.shadow_damage);

	DBG(("%s: posting shadow damage? %d (flips pending? %d, mode reconfiguration pending? %d)\n",
	     __FUNCTION__,
	     !RegionNil(DamageRegion(sna->mode.shadow_damage)),
	     sna->mode.flip_active,
	     sna->mode.dirty));
	assert((sna->flags & SNA_IS_HOSTED) == 0);
	assert(sna->mode.shadow_active);

	if (sna->mode.dirty)
		return;

	if (sna->mode.flip_active) {
		DBG(("%s: %d outstanding flips\n",
		     __FUNCTION__, sna->mode.flip_active));
		return;
	}

	region = DamageRegion(sna->mode.shadow_damage);
	if (RegionNil(region))
		return;

	DBG(("%s: damage: %dx(%d, %d), (%d, %d)\n",
	     __FUNCTION__, region_num_rects(region),
	     region->extents.x1, region->extents.y1,
	     region->extents.x2, region->extents.y2));

	if (!move_crtc_to_gpu(sna)) {
		DBG(("%s: forcing scanout update using the CPU\n", __FUNCTION__));
		if (!sna_pixmap_move_to_cpu(sna->front, MOVE_READ))
			return;

		for (i = 0; i < sna->mode.num_real_crtc; i++) {
			xf86CrtcPtr crtc = config->crtc[i];
			struct sna_crtc *sna_crtc = to_sna_crtc(crtc);
			RegionRec damage;

			assert(sna_crtc != NULL);
			if (!sna_crtc->shadow)
				continue;

			assert(crtc->enabled);
			assert(sna_crtc->transform || sna->flags & SNA_TEAR_FREE);

			damage.extents = crtc->bounds;
			damage.data = NULL;
			RegionIntersect(&damage, &damage, region);
			if (!box_empty(&damage.extents)) {
				DBG(("%s: fallback intersects pipe=%d [(%d, %d), (%d, %d)]\n",
				     __FUNCTION__, __sna_crtc_pipe(sna_crtc),
				     damage.extents.x1, damage.extents.y1,
				     damage.extents.x2, damage.extents.y2));

				sna_crtc_redisplay__fallback(crtc,
							     &damage,
							     sna_crtc->bo);
			}
			RegionUninit(&damage);

			if (sna_crtc->slave_damage)
				DamageEmpty(sna_crtc->slave_damage);
		}

		RegionEmpty(region);
		return;
	}

	{
		struct sna_pixmap *priv;

		priv = sna_pixmap(sna->front);
		assert(priv != NULL);

		if (priv->move_to_gpu) {
			if (priv->move_to_gpu == wait_for_shadow &&
			    !sna->mode.shadow_dirty) {
				/* No damage written to new scanout
				 * (backbuffer), ignore redisplay request
				 * and continue with the current intact
				 * scanout (frontbuffer).
				 */
				DBG(("%s: shadow idle, skipping update\n", __FUNCTION__));
				RegionEmpty(region);
				return;
			}

			(void)priv->move_to_gpu(sna, priv, 0);
		}

		assert(priv->move_to_gpu == NULL);
	}

	for (i = 0; i < sna->mode.num_real_crtc; i++) {
		xf86CrtcPtr crtc = config->crtc[i];
		struct sna_crtc *sna_crtc = to_sna_crtc(crtc);
		RegionRec damage;
		int sigio;

		assert(sna_crtc != NULL);
		DBG(("%s: crtc[%d] transformed? %d\n",
		     __FUNCTION__, i, sna_crtc->transform));

		if (!sna_crtc->transform)
			continue;

		assert(crtc->enabled);
		assert(sna_crtc->bo);

		damage.extents = crtc->bounds;
		damage.data = NULL;

		RegionIntersect(&damage, &damage, region);
		DBG(("%s: crtc[%d] damage? %d[%d]: %dx[(%d, %d), (%d, %d)]\n",
		     __FUNCTION__, i,
		     !box_empty(&damage.extents), RegionNotEmpty(&damage),
		     region_num_rects(&damage),
		     damage.extents.x1, damage.extents.y1,
		     damage.extents.x2, damage.extents.y2));
		sigio = sigio_block();
		if (!box_empty(&damage.extents)) {
			if (sna->flags & SNA_TEAR_FREE) {
				RegionRec new_damage;
				struct drm_mode_crtc_page_flip arg;
				struct kgem_bo *bo;

				RegionNull(&new_damage);
				RegionCopy(&new_damage, &damage);

				bo = sna_crtc->cache_bo;
				if (bo == NULL) {
					damage.extents = crtc->bounds;
					damage.data = NULL;
					bo = kgem_create_2d(&sna->kgem,
							    crtc->mode.HDisplay,
							    crtc->mode.VDisplay,
							    crtc->scrn->bitsPerPixel,
							    sna_crtc->bo->tiling,
							    CREATE_SCANOUT);
					if (bo == NULL)
						continue;
				} else
					RegionUnion(&damage, &damage, &sna_crtc->crtc_damage);
				sna_crtc->crtc_damage = new_damage;

				sna_crtc_redisplay(crtc, &damage, bo);
				kgem_bo_submit(&sna->kgem, bo);
				__kgem_bo_clear_dirty(bo);

				assert_crtc_fb(sna, sna_crtc);
				arg.crtc_id = __sna_crtc_id(sna_crtc);
				arg.fb_id = get_fb(sna, bo,
						   crtc->mode.HDisplay,
						   crtc->mode.VDisplay);
				if (arg.fb_id == 0)
					goto disable1;

				arg.user_data = (uintptr_t)sna_crtc;
				arg.flags = DRM_MODE_PAGE_FLIP_EVENT;
				arg.reserved = 0;

				if (drmIoctl(sna->kgem.fd, DRM_IOCTL_MODE_PAGE_FLIP, &arg)) {
					if (sna_crtc_flip(sna, sna_crtc, bo, 0, 0)) {
						DBG(("%s: removing handle=%d [active_scanout=%d] from scanout, installing handle=%d [active_scanout=%d]\n",
						     __FUNCTION__, sna_crtc->bo->handle, sna_crtc->bo->active_scanout - 1,
						     bo->handle, bo->active_scanout));
						assert(sna_crtc->bo->active_scanout);
						assert(sna_crtc->bo->refcnt >= sna_crtc->bo->active_scanout);
						sna_crtc->bo->active_scanout--;
						kgem_bo_destroy(&sna->kgem, sna_crtc->bo);

						sna_crtc->bo = kgem_bo_reference(bo);
						sna_crtc->bo->active_scanout++;
					} else {
						BoxRec box;
						DrawableRec tmp;

						DBG(("%s: flip [fb=%d] on crtc %d [%d, pipe=%d] failed - %d\n",
						     __FUNCTION__, arg.fb_id, i, __sna_crtc_id(sna_crtc), __sna_crtc_pipe(sna_crtc), errno));
						xf86DrvMsg(sna->scrn->scrnIndex, X_ERROR,
							   "Page flipping failed, disabling TearFree\n");
						sna->flags &= ~SNA_TEAR_FREE;

disable1:
						box.x1 = 0;
						box.y1 = 0;
						tmp.width = box.x2 = crtc->mode.HDisplay;
						tmp.height = box.y2 = crtc->mode.VDisplay;
						tmp.depth = sna->front->drawable.depth;
						tmp.bitsPerPixel = sna->front->drawable.bitsPerPixel;

						if (!sna->render.copy_boxes(sna, GXcopy,
									    &sna->front->drawable, bo, 0, 0,
									    &tmp, sna_crtc->bo, 0, 0,
									    &box, 1, COPY_LAST)) {
							xf86DrvMsg(crtc->scrn->scrnIndex, X_ERROR,
								   "%s: page flipping failed, disabling CRTC:%d (pipe=%d)\n",
								   __FUNCTION__, __sna_crtc_id(sna_crtc), __sna_crtc_pipe(sna_crtc));
							sna_crtc_disable(crtc, false);
						}
					}

					kgem_bo_destroy(&sna->kgem, bo);
					sna_crtc->cache_bo = NULL;
					continue;
				}
				sna->mode.flip_active++;

				assert(sna_crtc->flip_bo == NULL);
				sna_crtc->flip_handler = shadow_flip_handler;
				sna_crtc->flip_data = sna;
				sna_crtc->flip_bo = bo;
				sna_crtc->flip_bo->active_scanout++;
				sna_crtc->flip_serial = sna_crtc->mode_serial;
				sna_crtc->flip_pending = true;

				if (sna_crtc->bo != sna->mode.shadow) {
					assert_scanout(&sna->kgem, sna_crtc->bo,
						       crtc->mode.HDisplay, crtc->mode.VDisplay);
					sna_crtc->cache_bo = kgem_bo_reference(sna_crtc->bo);
				}
				DBG(("%s: recording flip on CRTC:%d handle=%d, active_scanout=%d, serial=%d\n",
				     __FUNCTION__, __sna_crtc_id(sna_crtc), sna_crtc->flip_bo->handle, sna_crtc->flip_bo->active_scanout, sna_crtc->flip_serial));
			} else {
				sna_crtc_redisplay(crtc, &damage, sna_crtc->bo);
				kgem_scanout_flush(&sna->kgem, sna_crtc->bo);
			}
		}
		RegionUninit(&damage);
		sigio_unblock(sigio);

		if (sna_crtc->slave_damage)
			DamageEmpty(sna_crtc->slave_damage);
	}

	if (sna->mode.shadow) {
		struct kgem_bo *new = __sna_pixmap_get_bo(sna->front);
		struct kgem_bo *old = sna->mode.shadow;
		struct drm_mode_crtc_page_flip arg;
		uint32_t fb = 0;
		int sigio;

		DBG(("%s: flipping TearFree outputs, current scanout handle=%d [active?=%d], new handle=%d [active=%d]\n",
		     __FUNCTION__, old->handle, old->active_scanout, new->handle, new->active_scanout));

		assert(new != old);
		assert(new->refcnt);

		arg.flags = DRM_MODE_PAGE_FLIP_EVENT;
		arg.reserved = 0;

		kgem_bo_submit(&sna->kgem, new);
		__kgem_bo_clear_dirty(new);

		sigio = sigio_block();
		for (i = 0; i < sna->mode.num_real_crtc; i++) {
			struct sna_crtc *crtc = config->crtc[i]->driver_private;
			struct kgem_bo *flip_bo;
			int x, y;

			assert(crtc != NULL);
			DBG(("%s: crtc %d [%d, pipe=%d] active? %d, transformed? %d\n",
			     __FUNCTION__, i, __sna_crtc_id(crtc), __sna_crtc_pipe(crtc), crtc->bo ? crtc->bo->handle : 0, crtc->transform));
			if (crtc->bo == NULL || crtc->transform)
				continue;

			assert(config->crtc[i]->enabled);
			assert(crtc->flip_bo == NULL);
			assert_crtc_fb(sna, crtc);

			arg.crtc_id = __sna_crtc_id(crtc);
			arg.user_data = (uintptr_t)crtc;

			if (crtc->client_bo) {
				DBG(("%s: apply shadow override bo for CRTC:%d on pipe=%d, handle=%d\n",
				     __FUNCTION__, __sna_crtc_id(crtc), __sna_crtc_pipe(crtc), crtc->client_bo->handle));
				arg.fb_id = get_fb(sna, crtc->client_bo,
						   crtc->base->mode.HDisplay,
						   crtc->base->mode.VDisplay);
				assert(arg.fb_id != fb);
				flip_bo = crtc->client_bo;
				x = y = 0;
			} else {
				if (fb == 0)
					fb = get_fb(sna, new, sna->scrn->virtualX, sna->scrn->virtualY);
				if (fb == 0) {
fixup_shadow:
					if (sna_pixmap_move_to_gpu(sna->front, MOVE_READ | MOVE_ASYNC_HINT)) {
						BoxRec box;

						box.x1 = 0;
						box.y1 = 0;
						box.x2 = sna->scrn->virtualX;
						box.y2 = sna->scrn->virtualY;
						if (sna->render.copy_boxes(sna, GXcopy,
									   &sna->front->drawable, __sna_pixmap_get_bo(sna->front), 0, 0,
									   &sna->front->drawable, old, 0, 0,
									   &box, 1, COPY_LAST)) {
							kgem_submit(&sna->kgem);
							RegionEmpty(region);
						}
					}

					sigio_unblock(sigio);
					return;
				}

				arg.fb_id = fb;
				flip_bo = new;
				x = crtc->base->x;
				y = crtc->base->y;
			}

			if (crtc->bo == flip_bo) {
				assert(crtc->bo->refcnt >= crtc->bo->active_scanout);
				DBG(("%s: flip handle=%d is already on the CRTC\n",
				     __FUNCTION__, flip_bo->handle));
				continue;
			}

			if (flip_bo->pitch != crtc->bo->pitch || (y << 16 | x)  != crtc->offset) {
				DBG(("%s: changing pitch (new %d =?= old %d) or offset (new %x =?= old %x)\n",
				     __FUNCTION__,
				     flip_bo->pitch, crtc->bo->pitch,
				     y << 16 | x, crtc->offset));
fixup_flip:
				if (sna_crtc_flip(sna, crtc, flip_bo, x, y)) {
					DBG(("%s: removing handle=%d [active_scanout=%d] from scanout, installing handle=%d [active_scanout=%d]\n",
					     __FUNCTION__, crtc->bo->handle, crtc->bo->active_scanout-1,
					     flip_bo->handle, flip_bo->active_scanout));
					assert(flip_bo != crtc->bo);
					assert(crtc->bo->active_scanout);
					assert(crtc->bo->refcnt >= crtc->bo->active_scanout);
					crtc->bo->active_scanout--;
					kgem_bo_destroy(&sna->kgem, crtc->bo);

					if (crtc->shadow_bo) {
						kgem_bo_destroy(&sna->kgem, crtc->shadow_bo);
						crtc->shadow_bo = NULL;
					}

					crtc->bo = kgem_bo_reference(flip_bo);
					crtc->bo->active_scanout++;
				} else {
					if (sna->flags & SNA_TEAR_FREE) {
						xf86DrvMsg(sna->scrn->scrnIndex, X_ERROR,
								"Failed to prepare CRTC for page flipping, disabling TearFree\n");
						sna->flags &= ~SNA_TEAR_FREE;
					}

					if (sna->mode.flip_active == 0) {
						DBG(("%s: abandoning flip attempt\n", __FUNCTION__));
						goto fixup_shadow;
					}

					xf86DrvMsg(sna->scrn->scrnIndex, X_ERROR,
						   "%s: page flipping failed, disabling CRTC:%d (pipe=%d)\n",
						   __FUNCTION__, __sna_crtc_id(crtc), __sna_crtc_pipe(crtc));
					sna_crtc_disable(crtc->base, false);
				}
				continue;
			}

			if (drmIoctl(sna->kgem.fd, DRM_IOCTL_MODE_PAGE_FLIP, &arg)) {
				ERR(("%s: flip [fb=%d] on crtc %d [%d, pipe=%d] failed - %d\n",
				     __FUNCTION__, arg.fb_id, i, __sna_crtc_id(crtc), __sna_crtc_pipe(crtc), errno));
				goto fixup_flip;
			}
			sna->mode.flip_active++;

			assert(crtc->flip_bo == NULL);
			crtc->flip_handler = shadow_flip_handler;
			crtc->flip_data = sna;
			crtc->flip_bo = kgem_bo_reference(flip_bo);
			crtc->flip_bo->active_scanout++;
			crtc->flip_serial = crtc->mode_serial;
			crtc->flip_pending = true;

			DBG(("%s: recording flip on CRTC:%d handle=%d, active_scanout=%d, serial=%d\n",
			     __FUNCTION__, __sna_crtc_id(crtc), crtc->flip_bo->handle, crtc->flip_bo->active_scanout, crtc->flip_serial));

			{
				struct drm_i915_gem_busy busy = { flip_bo->handle };
				if (drmIoctl(sna->kgem.fd, DRM_IOCTL_I915_GEM_BUSY, &busy) == 0) {
					if (busy.busy) {
						int mode = KGEM_RENDER;
						if (busy.busy & (0xfffe << 16))
							mode = KGEM_BLT;
						DBG(("%s: marking flip bo as busy [%x -> mode=%d]\n", __FUNCTION__, busy.busy, mode));
						kgem_bo_mark_busy(&sna->kgem, flip_bo, mode);
					} else
						__kgem_bo_clear_busy(flip_bo);
				}
			}
		}
		sigio_unblock(sigio);

		DBG(("%s: flipped %d outputs, shadow active? %d\n",
		     __FUNCTION__,
		     sna->mode.flip_active,
		     sna->mode.shadow ? sna->mode.shadow->handle : 0));

		if (sna->mode.flip_active) {
			assert(old == sna->mode.shadow);
			assert(old->refcnt >= 1);
			set_shadow(sna, region);
		}
	} else
		kgem_submit(&sna->kgem);

	RegionEmpty(region);
}

int sna_mode_wakeup(struct sna *sna)
{
	bool defer_vblanks = sna->mode.flip_active && sna->mode.shadow_enabled;
	char buffer[1024];
	int len, i;
	int ret = 0;

again:
	/* In order to workaround a kernel bug in not honouring O_NONBLOCK,
	 * check that the fd is readable before attempting to read the next
	 * event from drm.
	 */
	if (!event_pending(sna->kgem.fd))
		goto done;

	/* The DRM read semantics guarantees that we always get only
	 * complete events.
	 */
	len = read(sna->kgem.fd, buffer, sizeof (buffer));
	if (len < (int)sizeof(struct drm_event))
		goto done;

	/* Note that we cannot rely on the passed in struct sna matching
	 * the struct sna used for the vblank event (in case it was submitted
	 * by a different ZaphodHead). When processing the event, we must
	 * ensure that we only use the pointer passed along with the event.
	 */

	DBG(("%s: len=%d\n", __FUNCTION__, len));

	i = 0;
	while (i < len) {
		struct drm_event *e = (struct drm_event *)&buffer[i];
		switch (e->type) {
		case DRM_EVENT_VBLANK:
			if (defer_vblanks)
				defer_event(sna, e);
			else if (((uintptr_t)((struct drm_event_vblank *)e)->user_data) & 2)
				sna_present_vblank_handler((struct drm_event_vblank *)e);
			else
				sna_dri2_vblank_handler((struct drm_event_vblank *)e);
			break;
		case DRM_EVENT_FLIP_COMPLETE:
			{
				struct drm_event_vblank *vbl = (struct drm_event_vblank *)e;
				struct sna_crtc *crtc = (void *)(uintptr_t)vbl->user_data;
				uint64_t msc;

				/* Beware Zaphod! */
				sna = to_sna(crtc->base->scrn);

				if (msc64(crtc, vbl->sequence, &msc)) {
					DBG(("%s: recording last swap on pipe=%d, frame %d [%08llx], time %d.%06d\n",
					     __FUNCTION__, __sna_crtc_pipe(crtc), vbl->sequence, (long long)msc, vbl->tv_sec, vbl->tv_usec));
					crtc->swap.tv_sec = vbl->tv_sec;
					crtc->swap.tv_usec = vbl->tv_usec;
					crtc->swap.msc = msc;
				}
				assert(crtc->flip_pending);
				crtc->flip_pending = false;

				assert(crtc->flip_bo);
				assert(crtc->flip_bo->active_scanout);
				assert(crtc->flip_bo->refcnt >= crtc->flip_bo->active_scanout);

				if (crtc->flip_serial == crtc->mode_serial) {
					DBG(("%s: removing handle=%d [active_scanout=%d] from scanout, installing handle=%d [active_scanout=%d]\n",
					     __FUNCTION__, crtc->bo->handle, crtc->bo->active_scanout - 1,
					     crtc->flip_bo->handle, crtc->flip_bo->active_scanout));
					assert(crtc->bo->active_scanout);
					assert(crtc->bo->refcnt >= crtc->bo->active_scanout);

					crtc->bo->active_scanout--;
					kgem_bo_destroy(&sna->kgem, crtc->bo);

					if (crtc->shadow_bo) {
						kgem_bo_destroy(&sna->kgem, crtc->shadow_bo);
						crtc->shadow_bo = NULL;
					}

					crtc->bo = crtc->flip_bo;
					crtc->flip_bo = NULL;

					assert_crtc_fb(sna, crtc);
				} else {
					crtc->flip_bo->active_scanout--;
					kgem_bo_destroy(&sna->kgem, crtc->flip_bo);
					crtc->flip_bo = NULL;
				}

				DBG(("%s: flip complete, pending? %d\n", __FUNCTION__, sna->mode.flip_active));
				assert(sna->mode.flip_active);
				if (--sna->mode.flip_active == 0) {
					assert(crtc->flip_handler);
					crtc->flip_handler(vbl, crtc->flip_data);
				}
			}
			break;
		default:
			break;
		}
		i += e->length;
		ret++;
	}

	goto again;

done:
	flush_events(sna);
	return ret;
}

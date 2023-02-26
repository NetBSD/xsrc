/*
 * Copyright 2008 Red Hat, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * on the rights to use, copy, modify, merge, publish, distribute, sub
 * license, and/or sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef QXL_H
#define QXL_H

#include <stdint.h>

#include <spice/qxl_dev.h>
#ifdef XSPICE
#include <spice.h>
#endif

#include "compiler.h"
#include "xf86.h"
#if GET_ABI_MAJOR(ABI_VIDEODRV_VERSION) < 6
#include "xf86Resources.h"
#endif
#include "xf86Cursor.h"
#include "xf86_OSproc.h"
#ifdef XV
#include "xf86xv.h"
#endif
#include "xf86Crtc.h"
#include "shadow.h"
#include "micmap.h"
#include "uxa/uxa.h"

#include "list.h"
#ifndef XSPICE
#ifdef XSERVER_PCIACCESS
#include "pciaccess.h"
#endif
#ifdef XSERVER_PLATFORM_BUS
#include "xf86platformBus.h"
#endif
#include "fb.h"
#include "vgaHW.h"
#endif /* XSPICE */

#include "qxl_drmmode.h"

#if (XORG_VERSION_CURRENT < XORG_VERSION_NUMERIC(1, 11, 99, 903, 0))
typedef struct list xorg_list_t;
#define xorg_list_init              list_init
#define xorg_list_add               list_add
#define xorg_list_del               list_del
#define xorg_list_for_each_entry    list_for_each_entry
#else
typedef struct xorg_list xorg_list_t;
#endif

struct xf86_platform_device;

#include "compat-api.h"
#define hidden _X_HIDDEN

#ifdef XSPICE
#define QXL_NAME		"spiceqxl"
#define QXL_DRIVER_NAME		"spiceqxl"
#else
#define QXL_NAME		"qxl"
#define QXL_DRIVER_NAME		"qxl"
#endif
#define PCI_VENDOR_RED_HAT	0x1b36

#define PCI_CHIP_QXL_0100	0x0100
#define PCI_CHIP_QXL_01FF	0x01ff

#pragma pack(push,1)

struct qxl_ring_header {
    uint32_t num_items;
    uint32_t prod;
    uint32_t notify_on_prod;
    uint32_t cons;
    uint32_t notify_on_cons;
};

#pragma pack(pop)
typedef struct surface_cache_t surface_cache_t;

typedef struct _qxl_screen_t qxl_screen_t;

typedef struct
{
    uint8_t	generation;
    uint64_t	start_phys_addr;
    uint64_t	end_phys_addr;
    uint64_t	start_virt_addr;
    uint64_t	end_virt_addr;
    uint64_t	high_bits;
} qxl_memslot_t;

typedef struct qxl_surface_t qxl_surface_t;

/*
 * Config Options
 */

enum {
    OPTION_ENABLE_IMAGE_CACHE = 0,
    OPTION_ENABLE_FALLBACK_CACHE,
    OPTION_ENABLE_SURFACES,
    OPTION_DEBUG_RENDER_FALLBACKS,
    OPTION_NUM_HEADS,
    OPTION_SPICE_DEFERRED_FPS,
#ifdef XSPICE
    OPTION_SPICE_PORT,
    OPTION_SPICE_TLS_PORT,
    OPTION_SPICE_ADDR,
    OPTION_SPICE_X509_DIR,
    OPTION_SPICE_SASL,
    OPTION_SPICE_AGENT_MOUSE,
    OPTION_SPICE_DISABLE_TICKETING,
    OPTION_SPICE_PASSWORD,
    OPTION_SPICE_X509_KEY_FILE,
    OPTION_SPICE_STREAMING_VIDEO,
    OPTION_SPICE_PLAYBACK_COMPRESSION,
    OPTION_SPICE_ZLIB_GLZ_WAN_COMPRESSION,
    OPTION_SPICE_JPEG_WAN_COMPRESSION,
    OPTION_SPICE_IMAGE_COMPRESSION,
    OPTION_SPICE_DISABLE_COPY_PASTE,
    OPTION_SPICE_IPV4_ONLY,
    OPTION_SPICE_IPV6_ONLY,
    OPTION_SPICE_X509_CERT_FILE,
    OPTION_SPICE_X509_KEY_PASSWORD,
    OPTION_SPICE_TLS_CIPHERS,
    OPTION_SPICE_CACERT_FILE,
    OPTION_SPICE_DH_FILE,
    OPTION_SPICE_EXIT_ON_DISCONNECT,
    OPTION_SPICE_PLAYBACK_FIFO_DIR,
    OPTION_SPICE_VDAGENT_ENABLED,
    OPTION_SPICE_VDAGENT_VIRTIO_PATH,
    OPTION_SPICE_VDAGENT_UINPUT_PATH,
    OPTION_SPICE_VDAGENT_UID,
    OPTION_SPICE_VDAGENT_GID,
    OPTION_FRAME_BUFFER_SIZE,
    OPTION_SURFACE_BUFFER_SIZE,
    OPTION_COMMAND_BUFFER_SIZE,
    OPTION_SPICE_SMARTCARD_FILE,
    OPTION_SPICE_VIDEO_CODECS,
#endif
    OPTION_COUNT,
};

enum {
    QXL_DEVICE_PRIMARY_UNDEFINED,
    QXL_DEVICE_PRIMARY_NONE,
    QXL_DEVICE_PRIMARY_CREATED,
};

struct qxl_bo;
/*
 * for relocations
 * dst_bo + dst_offset are the bo and offset into which the reloc is being written,
 * src_bo is the bo who's offset is being relocated.
 */
struct qxl_bo_funcs {
    struct qxl_bo *(*bo_alloc)(qxl_screen_t *qxl, unsigned long size, const char *name);
    struct qxl_bo *(*cmd_alloc)(qxl_screen_t *qxl, unsigned long size, const char *name);
    void *(*bo_map)(struct qxl_bo *bo);
    void (*bo_unmap)(struct qxl_bo *bo);
    void (*bo_decref)(qxl_screen_t *qxl, struct qxl_bo *bo);
    void (*bo_incref)(qxl_screen_t *qxl, struct qxl_bo *bo);
    void (*bo_output_bo_reloc)(qxl_screen_t *qxl, uint32_t dst_offset,
			       struct qxl_bo *dst_bo, struct qxl_bo *src_bo);
    void (*write_command)(qxl_screen_t *qxl, uint32_t type, struct qxl_bo *bo);
    void (*update_area)(qxl_surface_t *surf, int x1, int y1, int x2, int y2);
    struct qxl_bo *(*create_primary)(qxl_screen_t *qxl, uint32_t width, uint32_t height, int32_t stride, uint32_t format);
    void (*destroy_primary)(qxl_screen_t *qxl, struct qxl_bo *primary_bo);

    qxl_surface_t *(*create_surface)(qxl_screen_t *qxl, int width,
				     int height, int bpp);
    void (*destroy_surface)(qxl_surface_t *surf);

    void (*bo_output_surf_reloc)(qxl_screen_t *qxl, uint32_t dst_offset,
				 struct qxl_bo *dst_bo,
				 qxl_surface_t *surf);
  /* surface create / destroy */
};
    
void qxl_ums_setup_funcs(qxl_screen_t *qxl);
void qxl_kms_setup_funcs(qxl_screen_t *qxl);

/* ums specific functions */
struct qxl_bo *qxl_ums_surf_mem_alloc(qxl_screen_t *qxl, uint32_t size);
struct qxl_bo *qxl_ums_lookup_phy_addr(qxl_screen_t *qxl, uint64_t phy_addr);

typedef struct FrameTimer FrameTimer;
typedef void (*FrameTimerFunc)(void *opaque);

#ifdef XF86DRM_MODE
#define MAX_RELOCS 96
#include "qxl_drm.h"

struct qxl_cmd_stream {
  struct qxl_bo *reloc_bo[MAX_RELOCS];
  int n_reloc_bos;
  struct drm_qxl_reloc relocs[MAX_RELOCS];
  int n_relocs;
};
#endif

struct _qxl_screen_t
{
    /* These are the names QXL uses */
    void *			ram;	/* Command RAM */
    void *			ram_physical;
    void *			vram;	/* Surface RAM */
    void *			vram_physical;
    struct QXLRom *		rom;    /* Parameter RAM */
    
    struct qxl_ring *		command_ring;
    struct qxl_ring *		cursor_ring;
    struct qxl_ring *		release_ring;

    Bool                        screen_resources_created;
    int                         device_primary;
    struct qxl_bo *             primary_bo;
    int				num_modes;
    struct QXLMode *		modes;
    int				io_base;
    void *			surface0_area;
    long			surface0_size;
    long			vram_size;
    long			ram_size;

    DisplayModePtr              x_modes;

    int				virtual_x;
    int				virtual_y;

    /* not the same as the heads mode for #head > 1 or virtual != head size */
    struct QXLMode 		primary_mode;
    qxl_surface_t *		primary;

    struct QXLMonitorsConfig   *monitors_config;
    int                         monitors_config_size;
    int                         mem_size;
    
    int				bytes_per_pixel;

    /* Commands */
    struct qxl_mem *		mem;   /* Context for qxl_alloc/free */

    /* Surfaces */
    struct qxl_mem *		surf_mem;  /* Context for qxl_surf_alloc/free */
    
    EntityInfoPtr		entity;

    int                         num_heads;
    xf86CrtcPtr *               crtcs;
    xf86OutputPtr *             outputs;

#ifndef XSPICE
#ifdef XSERVER_LIBPCIACCESS
    struct pci_device *		pci;
    struct pci_io_handle *	io;
#else
    pciVideoPtr			pci;
    PCITAG			pci_tag;
#endif
    struct xf86_platform_device *platform_dev;
    vgaRegRec                   vgaRegs;
#endif /* XSPICE */

    uxa_driver_t *		uxa;
    
    CreateScreenResourcesProcPtr create_screen_resources;
    CloseScreenProcPtr		close_screen;
    CreateGCProcPtr		create_gc;
    CopyWindowProcPtr		copy_window;
    
    int16_t			cur_x;
    int16_t			cur_y;
    int16_t			hot_x;
    int16_t			hot_y;
    
    ScrnInfoPtr			pScrn;

    qxl_memslot_t *		mem_slots;
    uint8_t			n_mem_slots;

    uint8_t			main_mem_slot;
    uint8_t			slot_id_bits;
    uint8_t			slot_gen_bits;
    uint64_t			va_slot_mask;

    uint8_t			vram_mem_slot;

    surface_cache_t *		surface_cache;

    /* Evacuated surfaces are stored here during VT switches */
    void *			vt_surfaces;

    OptionInfoRec	options[OPTION_COUNT + 1];

    int				enable_image_cache;
    int				enable_fallback_cache;
    int				enable_surfaces;
    int                         debug_render_fallbacks;
    
    FrameTimer *        frames_timer;

#ifdef XSPICE
    /* XSpice specific */
    struct QXLRom		shadow_rom;    /* Parameter RAM */
    SpiceServer *       spice_server;
    SpiceCoreInterface *core;

    QXLWorker *         worker;
    int                 worker_running;
    QXLInstance         display_sin;
    SpicePlaybackInstance playback_sin;
    /* XSpice specific, dragged from the Device */
    QXLReleaseInfo     *last_release;

    uint32_t           cmdflags;
    uint32_t           oom_running;
    uint32_t           num_free_res; /* is having a release ring effective
                                        for Xspice? */
    /* This is only touched from red worker thread - do not access
     * from Xorg threads. */
    struct guest_primary {
        QXLSurfaceCreate surface;
        uint32_t       commands;
        uint32_t       resized;
        int32_t        stride;
        uint32_t       bits_pp;
        uint32_t       bytes_pp;
        uint8_t        *data, *flipped;
    } guest_primary;

    char playback_fifo_dir[PATH_MAX];
    void *playback_opaque;
    char smartcard_file[PATH_MAX];
#endif /* XSPICE */

    uint32_t deferred_fps;
    xorg_list_t ums_bos;
    struct qxl_bo_funcs *bo_funcs;

    Bool kms_enabled;
#ifdef XF86DRM_MODE
    drmmode_rec drmmode;
    int drm_fd;
    struct qxl_cmd_stream cmds;
#endif

};

typedef struct qxl_output_private {
    qxl_screen_t *qxl;
    int           head;
    xf86OutputStatus status;
} qxl_output_private;

typedef struct qxl_crtc_private {
    qxl_screen_t *qxl;
    int           head;
    xf86OutputPtr output;
} qxl_crtc_private;

static inline uint64_t
physical_address (qxl_screen_t *qxl, void *virtual, uint8_t slot_id)
{
    qxl_memslot_t *p_slot = &(qxl->mem_slots[slot_id]);

    return p_slot->high_bits | ((unsigned long)virtual - p_slot->start_virt_addr);
}

static inline void *
virtual_address (qxl_screen_t *qxl, void *physical, uint8_t slot_id)
{
    qxl_memslot_t *p_slot = &(qxl->mem_slots[slot_id]);
    unsigned long virt;

    virt = ((unsigned long)physical) & qxl->va_slot_mask;
    virt += p_slot->start_virt_addr;

    return (void *)virt;
}

static inline void *
u64_to_pointer (uint64_t u)
{
    return (void *)(unsigned long)u;
}

static inline uint64_t
pointer_to_u64 (void *p)
{
    return (uint64_t)(unsigned long)p;
}

struct qxl_ring;

/*
 * HW cursor
 */
void              qxl_cursor_init        (ScreenPtr               pScreen);



/*
 * Rings
 */
struct qxl_ring * qxl_ring_create      (struct qxl_ring_header *header,
					int                     element_size,
					int                     n_elements,
					int                     prod_notify,
					qxl_screen_t            *qxl);
void              qxl_ring_push        (struct qxl_ring        *ring,
					const void             *element);
Bool              qxl_ring_pop         (struct qxl_ring        *ring,
					void                   *element);
void              qxl_ring_wait_idle   (struct qxl_ring        *ring);

void              qxl_ring_request_notify (struct qxl_ring *ring);

int               qxl_ring_prod        (struct qxl_ring        *ring);
int               qxl_ring_cons        (struct qxl_ring        *ring);

/*
 * Surface
 */
surface_cache_t *   qxl_surface_cache_create (qxl_screen_t *qxl);
qxl_surface_t *	    qxl_surface_cache_create_primary (qxl_screen_t *qxl,
						struct QXLMode *mode);
void *              qxl_surface_get_host_bits(qxl_surface_t *surface);
qxl_surface_t *	    qxl_surface_create (qxl_screen_t *qxl,
					int	      width,
					int	      height,
					int	      bpp);
void
qxl_surface_cache_sanity_check (surface_cache_t *qxl);
void *
qxl_surface_cache_evacuate_all (surface_cache_t *qxl);
void
qxl_surface_cache_replace_all (surface_cache_t *qxl, void *data);

void		    qxl_surface_set_pixmap (qxl_surface_t *surface,
					    PixmapPtr      pixmap);
/* Call this to indicate that the server is done with the surface */
void		    qxl_surface_kill (qxl_surface_t *surface);
/* Call this when a notification comes back from the device
 * that the surface has been destroyed
 */
void		    qxl_surface_recycle (surface_cache_t *cache, uint32_t id);

/* send anything pending to the other side */
void		    qxl_surface_flush (qxl_surface_t *surface);

/* access */
Bool		    qxl_surface_prepare_access (qxl_surface_t *surface,
						PixmapPtr      pixmap,
						RegionPtr      region,
						uxa_access_t   access);
void		    qxl_surface_finish_access (qxl_surface_t *surface,
					       PixmapPtr      pixmap);

/* solid */
Bool		    qxl_surface_prepare_solid (qxl_surface_t *destination,
					       Pixel	      fg);
void		    qxl_surface_solid         (qxl_surface_t *destination,
					       int	      x1,
					       int	      y1,
					       int	      x2,
					       int	      y2);

/* copy */
Bool		    qxl_surface_prepare_copy (qxl_surface_t *source,
					      qxl_surface_t *dest);
void		    qxl_surface_copy	     (qxl_surface_t *dest,
					      int  src_x1, int src_y1,
					      int  dest_x1, int dest_y1,
					      int width, int height);
Bool		    qxl_surface_put_image    (qxl_surface_t *dest,
					      int x, int y, int width, int height,
					      const char *src, int src_pitch);
void		    qxl_surface_unref        (surface_cache_t *cache,
					      uint32_t surface_id);

/* composite */
Bool		    qxl_surface_prepare_composite (int op,
						   PicturePtr	src_picture,
						   PicturePtr	mask_picture,
						   PicturePtr   dst_picture,
						   qxl_surface_t *src,
						   qxl_surface_t *mask,
						   qxl_surface_t *dest);
void		   qxl_surface_composite (qxl_surface_t *dest,
					  int src_x, int src_y,
					  int mask_x, int mask_y,
					  int dst_x, int dst_y,
					  int width, int height);

/* UXA */
#if HAS_DEVPRIVATEKEYREC
extern DevPrivateKeyRec uxa_pixmap_index;
#else
extern int uxa_pixmap_index;
#endif
Bool
qxl_uxa_init (qxl_screen_t *qxl, ScreenPtr screen);

static inline qxl_surface_t *get_surface (PixmapPtr pixmap)
{
#if HAS_DEVPRIVATEKEYREC
    return dixGetPrivate(&pixmap->devPrivates, &uxa_pixmap_index);
#else
    return dixLookupPrivate(&pixmap->devPrivates, &uxa_pixmap_index);
#endif
}

static inline void set_surface (PixmapPtr pixmap, qxl_surface_t *surface)
{
    dixSetPrivate(&pixmap->devPrivates, &uxa_pixmap_index, surface);
}

static inline struct QXLRam *
get_ram_header (qxl_screen_t *qxl)
{
    return (struct QXLRam *)
	((uint8_t *)qxl->ram + qxl->rom->ram_header_offset);
}

void qxl_surface_upload_primary_regions(qxl_screen_t *qxl, PixmapPtr pixmap, RegionRec *r);

/* ums randr code */
void qxl_init_randr (ScrnInfoPtr pScrn, qxl_screen_t *qxl);
void qxl_initialize_x_modes (qxl_screen_t *qxl, ScrnInfoPtr pScrn,
                        unsigned int *max_x, unsigned int *max_y);
void qxl_update_edid (qxl_screen_t *qxl);
Bool qxl_create_desired_modes (qxl_screen_t *qxl);

Bool qxl_resize_primary (qxl_screen_t *qxl, uint32_t width, uint32_t height);
void qxl_io_monitors_config_async (qxl_screen_t *qxl);
void qxl_allocate_monitors_config (qxl_screen_t *qxl);
/*
 * Images
 */
struct qxl_bo *qxl_image_create     (qxl_screen_t           *qxl,
				       const uint8_t          *data,
				       int                     x,
				       int                     y,
				       int                     width,
				       int                     height,
				       int                     stride,
				       int                     Bpp,
				       Bool		       fallback);
void              qxl_image_destroy    (qxl_screen_t           *qxl,
				        struct qxl_bo *bo);

/*
 * Malloc
 */
void              qxl_mem_init(void);
int		  qxl_handle_oom (qxl_screen_t *qxl);
struct qxl_mem *  qxl_mem_create       (void                   *base,
					unsigned long           n_bytes);
void              qxl_mem_dump_stats   (struct qxl_mem         *mem,
					const char             *header);
void              qxl_mem_free_all     (struct qxl_mem         *mem);
int		   qxl_garbage_collect (qxl_screen_t *qxl);

void qxl_reset_and_create_mem_slots (qxl_screen_t *qxl);
void qxl_mark_mem_unverifiable (qxl_screen_t *qxl);
#ifdef DEBUG_QXL_MEM
void qxl_mem_unverifiable(struct qxl_mem *mem);
#else
static inline void qxl_mem_unverifiable(struct qxl_mem *mem) {}
#endif

/*
 * I/O port commands
 */
void qxl_update_area(qxl_screen_t *qxl);
void qxl_io_memslot_add(qxl_screen_t *qxl, uint8_t id);
void qxl_io_create_primary(qxl_screen_t *qxl);
void qxl_io_destroy_primary(qxl_screen_t *qxl);
void qxl_io_notify_oom(qxl_screen_t *qxl);
void qxl_io_flush_surfaces(qxl_screen_t *qxl);
void qxl_io_destroy_all_surfaces (qxl_screen_t *qxl);

#ifdef QXLDRV_RESIZABLE_SURFACE0
void qxl_io_flush_release (qxl_screen_t *qxl);
#endif

Bool qxl_pre_init_common(ScrnInfoPtr pScrn);
Bool qxl_fb_init (qxl_screen_t *qxl, ScreenPtr pScreen);
Bool qxl_screen_init_kms(SCREEN_INIT_ARGS_DECL);
Bool qxl_enter_vt_kms (VT_FUNC_ARGS_DECL);
void qxl_leave_vt_kms (VT_FUNC_ARGS_DECL);
void qxl_set_screen_pixmap_header (ScreenPtr pScreen);
Bool qxl_resize_primary_to_virtual (qxl_screen_t *qxl);
void qxl_get_formats (int bpp, SpiceSurfaceFmt *format, pixman_format_code_t *pformat);

#ifdef XF86DRM_MODE
Bool qxl_pre_init_kms(ScrnInfoPtr pScrn, int flags);
Bool qxl_kms_check_cap(qxl_screen_t *qxl, int cap);
uint32_t qxl_kms_bo_get_handle(struct qxl_bo *_bo);
#else
static inline Bool qxl_pre_init_kms(ScrnInfoPtr pScrn, int flags) { return FALSE; }
static inline Bool qxl_kms_check_cap(qxl_screen_t *qxl, int cap) { return FALSE; }
#endif

#ifdef XSPICE
/* device to spice-server, now xspice to spice-server */
void ioport_write(qxl_screen_t *qxl, uint32_t io_port, uint32_t val);
#else
static inline void ioport_write(qxl_screen_t *qxl, int port, int val)
{
    pci_io_write8(qxl->io, port, val);
}
#endif

#ifdef XSPICE

#define MEMSLOT_GROUP 0
#define NUM_MEMSLOTS_GROUPS 1

// Taken from qemu's qxl.c, not sure the values make sense? we
// only have a single slot, and it is never changed after being added,
// so not a problem?
#define NUM_MEMSLOTS 8
#define MEMSLOT_GENERATION_BITS 8
#define MEMSLOT_SLOT_BITS 1

// qemu/cpu-all.h
#define TARGET_PAGE_SIZE (1 << TARGET_PAGE_BITS)
// qemu/target-i386/cpu.h
#define TARGET_PAGE_BITS 12

#define NUM_SURFACES 1024

/* initializes if required and returns the server singleton */
SpiceServer *xspice_get_spice_server(void);

#endif /* XSPICE */

#ifdef WITH_CHECK_POINT
#define CHECK_POINT() ErrorF ("%s: %d  (%s)\n", __FILE__, __LINE__, __FUNCTION__);
#else
#define CHECK_POINT()
#endif


#endif // QXL_H

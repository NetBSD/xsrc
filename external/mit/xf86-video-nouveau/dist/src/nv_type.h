#ifndef __NV_STRUCT_H__
#define __NV_STRUCT_H__

#include "colormapst.h"
#include "xf86Cursor.h"
#include "exa.h"
#include "xf86drm.h"
#include <stdbool.h>
#include <stdint.h>
#include "xf86Crtc.h"

#if XF86_CRTC_VERSION >= 5
#define NOUVEAU_PIXMAP_SHARING 1
#endif

#define NV_ARCH_03  0x03
#define NV_ARCH_04  0x04
#define NV_ARCH_10  0x10
#define NV_ARCH_20  0x20
#define NV_ARCH_30  0x30
#define NV_ARCH_40  0x40
#define NV_TESLA    0x50
#define NV_FERMI    0xc0
#define NV_KEPLER   0xe0
#define NV_MAXWELL  0x110

struct xf86_platform_device;

/* NV50 */
typedef struct _NVRec *NVPtr;

typedef struct {
	int fd;
	unsigned long reinitGeneration;
	struct xf86_platform_device *platform_dev;
	unsigned int assigned_crtcs;
	unsigned long fd_wakeup_registered;
	int fd_wakeup_ref;
} NVEntRec, *NVEntPtr;

NVEntPtr NVEntPriv(ScrnInfoPtr pScrn);

typedef struct _NVRec {
    uint32_t              Architecture;
    EntityInfoPtr       pEnt;
	struct pci_device *PciInfo;
    Bool                Primary;
    Bool		Secondary;

    struct nouveau_bo * scanout;

    enum {
	    UNKNOWN = 0,
	    NONE,
	    EXA,
    } AccelMethod;
    void (*Flush)(ScrnInfoPtr);

    Bool                HWCursor;
    Bool                ShadowFB;
    unsigned char *     ShadowPtr;
    int                 ShadowPitch;

    ExaDriverPtr	EXADriverPtr;
    Bool                exa_force_cp;
    Bool		wfb_enabled;
    Bool		tiled_scanout;
    Bool		glx_vblank;
    Bool		has_async_pageflip;
    Bool		has_pageflip;
    int 		swap_limit;
    int 		max_swap_limit;
    int 		max_dri_level;

    ScreenBlockHandlerProcPtr BlockHandler;
    CreateScreenResourcesProcPtr CreateScreenResources;
    CloseScreenProcPtr  CloseScreen;
    void		(*VideoTimerCallback)(ScrnInfoPtr, Time);
    XF86VideoAdaptorPtr	overlayAdaptor;
    XF86VideoAdaptorPtr	blitAdaptor;
    XF86VideoAdaptorPtr	textureAdaptor[2];
    int			videoKey;
    OptionInfoPtr	Options;

    Bool                LockedUp;

    CARD32              currentRop;

	void *drmmode; /* for KMS */

	/* DRM interface */
	struct nouveau_device *dev;
	char *drm_device_name;

	/* GPU context */
	struct nouveau_client *client;

	struct nouveau_bo *transfer;
	CARD32 transfer_offset;

	struct nouveau_object *channel;
	struct nouveau_pushbuf *pushbuf;
	struct nouveau_bufctx *bufctx;
	struct nouveau_object *notify0;
	struct nouveau_object *vblank_sem;
	struct nouveau_object *NvNull;
	struct nouveau_object *NvContextSurfaces;
	struct nouveau_object *NvContextBeta1;
	struct nouveau_object *NvContextBeta4;
	struct nouveau_object *NvImagePattern;
	struct nouveau_object *NvRop;
	struct nouveau_object *NvRectangle;
	struct nouveau_object *NvImageBlit;
	struct nouveau_object *NvScaledImage;
	struct nouveau_object *NvClipRectangle;
	struct nouveau_object *NvMemFormat;
	struct nouveau_object *NvImageFromCpu;
	struct nouveau_object *Nv2D;
	struct nouveau_object *Nv3D;
	struct nouveau_object *NvSW;
	struct nouveau_object *NvCOPY;
	struct nouveau_bo *scratch;

	Bool ce_enabled;
	struct nouveau_object *ce_channel;
	struct nouveau_pushbuf *ce_pushbuf;
	struct nouveau_object *NvCopy;
	Bool (*ce_rect)(struct nouveau_pushbuf *, struct nouveau_object *,
			int, int, int,
			struct nouveau_bo *, uint32_t, int, int, int, int, int,
			struct nouveau_bo *, uint32_t, int, int, int, int, int);

	/* SYNC extension private */
	void *sync;

	/* Present extension private */
	void *present;

	/* Acceleration context */
	PixmapPtr pspix, pmpix, pdpix;
	PicturePtr pspict, pmpict;
	Pixel fg_colour;

	char *render_node;
} NVRec;

#define NVPTR(p) ((NVPtr)((p)->driverPrivate))

typedef struct _NVPortPrivRec {
	short		brightness;
	short		contrast;
	short		saturation;
	short		hue;
	RegionRec	clip;
	CARD32		colorKey;
	Bool		autopaintColorKey;
	Bool		doubleBuffer;
	CARD32		videoStatus;
	int		currentBuffer;
	Time		videoTime;
	int		overlayCRTC;
	Bool		grabbedByV4L;
	Bool		iturbt_709;
	Bool		blitter;
	Bool		texture;
	Bool		bicubic; /* only for texture adapter */
	Bool		SyncToVBlank;
	int             max_image_dim;
	struct nouveau_bo *video_mem;
	int		pitch;
	int		offset;
	struct nouveau_bo *TT_mem_chunk[2];
	int		currentHostBuffer;
} NVPortPrivRec, *NVPortPrivPtr;

#define GET_OVERLAY_PRIVATE(pNv) \
            (NVPortPrivPtr)((pNv)->overlayAdaptor->pPortPrivates[0].ptr)

#define GET_BLIT_PRIVATE(pNv) \
            (NVPortPrivPtr)((pNv)->blitAdaptor->pPortPrivates[0].ptr)

#define OFF_TIMER       0x01
#define FREE_TIMER      0x02
#define CLIENT_VIDEO_ON 0x04
#define OFF_DELAY       500  /* milliseconds */
#define FREE_DELAY      5000

#define TIMER_MASK      (OFF_TIMER | FREE_TIMER)

/* EXA driver-controlled pixmaps */
#define NOUVEAU_CREATE_PIXMAP_ZETA	0x10000000
#define NOUVEAU_CREATE_PIXMAP_TILED	0x20000000
#define NOUVEAU_CREATE_PIXMAP_SCANOUT	0x40000000

struct nouveau_pixmap {
	struct nouveau_bo *bo;
	Bool shared;
};

static inline struct nouveau_pixmap *
nouveau_pixmap(PixmapPtr ppix)
{
	return (struct nouveau_pixmap *)exaGetPixmapDriverPrivate(ppix);
}

static inline struct nouveau_bo *
nouveau_pixmap_bo(PixmapPtr ppix)
{
	struct nouveau_pixmap *nvpix = nouveau_pixmap(ppix);

	return nvpix ? nvpix->bo : NULL;
}

static inline uint32_t
nv_pitch_align(NVPtr pNv, uint32_t width, int bpp)
{
	int mask;

	if (bpp == 15)
	        bpp = 16;
	if (bpp == 24 || bpp == 30)
	        bpp = 8;

	/* Alignment requirements taken from the Haiku driver */
	if (pNv->Architecture == NV_ARCH_04)
	        mask = 128 / bpp - 1;
	else
	        mask = 512 / bpp - 1;

	return (width + mask) & ~mask;
}

/* nv04 cursor max dimensions of 32x32 (A1R5G5B5) */
#define NV04_CURSOR_SIZE 32
/* limit nv10 cursors to 64x64 (ARGB8) (we could go to 64x255) */
#define NV10_CURSOR_SIZE 64

static inline int nv_cursor_width(NVPtr pNv)
{
	return pNv->dev->chipset >= 0x10 ? NV10_CURSOR_SIZE : NV04_CURSOR_SIZE;
}

#define xFixedToFloat(v) \
	((float)xFixedToInt((v)) + ((float)xFixedFrac(v) / 65536.0))

#endif /* __NV_STRUCT_H__ */

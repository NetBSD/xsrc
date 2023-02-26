/*
 * Copyright 2007-2011 by VMware, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE COPYRIGHT HOLDER(S) OR AUTHOR(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * Except as contained in this notice, the name of the copyright holder(s)
 * and author(s) shall not be used in advertising or otherwise to promote
 * the sale, use or other dealings in this Software without prior written
 * authorization from the copyright holder(s) and author(s).
 *
 */

/*
 * vmwarevideo.c --
 *
 *      Xv extension support.
 *      See http://www.xfree86.org/current/DESIGN16.html
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif


#include "xorg-server.h"
#include "xf86xv.h"
#include "fourcc.h"
#define debug_printf(...)

/*
 * We can't incude svga_types.h due to conflicting types for Bool.
 */
typedef int64_t int64;
typedef uint64_t uint64;

typedef int32_t int32;
typedef uint32_t uint32;

typedef int16_t int16;
typedef uint16_t uint16;

typedef int8_t int8;
typedef uint8_t uint8;

#include "../src/svga_reg.h"
#include "../src/svga_escape.h"
#include "../src/svga_overlay.h"
#include "../src/common_compat.h"

#include <X11/extensions/Xv.h>

#include "xf86drm.h"
#include "vmwgfx_drm.h"
#include "vmwgfx_drmi.h"
#include "vmwgfx_driver.h"
#include "vmwgfx_hosted.h"

#define MAKE_ATOM(a) MakeAtom(a, sizeof(a) - 1, TRUE)

/*
 * Number of videos that can be played simultaneously
 */
#define VMWARE_VID_NUM_PORTS 1

/*
 * Using a dark shade as the default colorKey
 */
#define VMWARE_VIDEO_COLORKEY 0x100701

/*
 * Maximum dimensions
 */
#define VMWARE_VID_MAX_WIDTH    2048
#define VMWARE_VID_MAX_HEIGHT   2048

#define VMWARE_VID_NUM_ENCODINGS 1
static XF86VideoEncodingRec vmwareVideoEncodings[] =
{
    {
       0,
       "XV_IMAGE",
       VMWARE_VID_MAX_WIDTH, VMWARE_VID_MAX_HEIGHT,
       {1, 1}
    }
};

#define VMWARE_VID_NUM_FORMATS 2
static XF86VideoFormatRec vmwareVideoFormats[] =
{
    { 16, TrueColor},
    { 24, TrueColor}
};

#define VMWARE_VID_NUM_IMAGES 3
static XF86ImageRec vmwareVideoImages[] =
{
    XVIMAGE_YV12,
    XVIMAGE_YUY2,
    XVIMAGE_UYVY
};

static CONST_ABI_16_TO_19 char xv_colorkey_name[] = "XV_COLORKEY";
static CONST_ABI_16_TO_19 char xv_autopaint_name[] = "XV_AUTOPAINT_COLORKEY";

#define VMWARE_VID_NUM_ATTRIBUTES 2
static XF86AttributeRec vmwareVideoAttributes[] =
{
    {
        XvGettable | XvSettable,
        0x000000,
        0xffffff,
        xv_colorkey_name,
    },
    {
        XvGettable | XvSettable,
        0,
        1,
        xv_autopaint_name,
    }
};

/*
 * Video frames are stored in a circular list of buffers.
 * Must be power or two, See vmw_video_port_play.
 */
#define VMWARE_VID_NUM_BUFFERS 1

/*
 * Defines the structure used to hold and pass video data to the host
 */
struct vmw_video_buffer
{
    int size;
    void *data;
    struct vmwgfx_dmabuf *buf;
};


/**
 * Structure representing a single video stream, aka port.
 *
 * Ports maps one to one to a SVGA stream. Port is just
 * what Xv calls a SVGA stream.
 */
struct vmwgfx_overlay_port
{
    /*
     * Function prototype same as XvPutImage.
     *
     * This is either set to vmw_video_port_init or vmw_video_port_play.
     * At init this function is set to port_init. In port_init we set it
     * to port_play and call it, after initializing the struct.
     */
    int (*play)(ScrnInfoPtr, struct vmwgfx_overlay_port *,
                short, short, short, short, short,
                short, short, short, int, unsigned char*,
                short, short, RegionPtr, DrawablePtr);

    /* values to go into the SVGAOverlayUnit */
    uint32 streamId;
    uint32 colorKey;
    uint32 flags;

    /* round robin of buffers */
    unsigned currBuf;
    struct vmw_video_buffer bufs[VMWARE_VID_NUM_BUFFERS];

    /* properties that applies to all buffers */
    int size;
    int pitches[3];
    int offsets[3];

    /* things for X */
    RegionRec clipBoxes;
    Bool isAutoPaintColorkey;
    int drm_fd;
};

/*
 * Callback functions exported to Xv, prefixed with vmw_xv_*.
 */
static int vmw_xv_put_image(ScrnInfoPtr pScrn, short src_x, short src_y,
                            short drw_x, short drw_y, short src_w, short src_h,
                            short drw_w, short drw_h, int image,
                            unsigned char *buf, short width, short height,
                            Bool sync, RegionPtr clipBoxes, pointer data,
                            DrawablePtr dst);
static void vmw_xv_stop_video(ScrnInfoPtr pScrn, pointer data, Bool Cleanup);
static int vmw_xv_query_image_attributes(ScrnInfoPtr pScrn, int format,
                                         unsigned short *width,
                                         unsigned short *height, int *pitches,
                                         int *offsets);
static int vmw_xv_set_port_attribute(ScrnInfoPtr pScrn, Atom attribute,
                                     INT32 value, pointer data);
static int vmw_xv_get_port_attribute(ScrnInfoPtr pScrn, Atom attribute,
                                     INT32 *value, pointer data);
static void vmw_xv_query_best_size(ScrnInfoPtr pScrn, Bool motion,
                                short vid_w, short vid_h, short drw_w,
                                short drw_h, unsigned int *p_w,
                                unsigned int *p_h, pointer data);


/*
 * Local functions.
 */
static int vmw_video_port_init(ScrnInfoPtr pScrn,
                               struct vmwgfx_overlay_port *port,
                               short src_x, short src_y, short drw_x,
                               short drw_y, short src_w, short src_h,
                               short drw_w, short drw_h, int format,
                               unsigned char *buf, short width,
                               short height, RegionPtr clipBoxes,
                               DrawablePtr pDraw);
static int vmw_video_port_play(ScrnInfoPtr pScrn, struct vmwgfx_overlay_port *port,
                               short src_x, short src_y, short drw_x,
                               short drw_y, short src_w, short src_h,
                               short drw_w, short drw_h, int format,
                               unsigned char *buf, short width,
                               short height, RegionPtr clipBoxes,
                               DrawablePtr pDraw);
static void vmw_video_port_cleanup(ScrnInfoPtr pScrn, struct vmwgfx_overlay_port *port);

static int vmw_video_buffer_alloc(int drm_fd, int size,
                                  struct vmw_video_buffer *out);
static int vmw_video_buffer_free(struct vmw_video_buffer *out);


static struct vmwgfx_overlay_port *
vmwgfx_overlay_port_create(int drm_fd, ScreenPtr pScreen)
{
    struct vmwgfx_overlay_port *port = calloc(1, sizeof(*port));

    if (!port)
	return NULL;

    port->drm_fd = drm_fd;
    port->play = vmw_video_port_init;
    port->flags = SVGA_VIDEO_FLAG_COLORKEY;
    port->colorKey = VMWARE_VIDEO_COLORKEY;
    port->isAutoPaintColorkey = TRUE;
    return port;
}

void
vmw_video_free_adaptor(XF86VideoAdaptorPtr adaptor)
{
    int i;

    for (i = 0; i < adaptor->nPorts; ++i)
	free(adaptor->pPortPrivates[i].ptr);

    free(adaptor->pPortPrivates);
    xf86XVFreeVideoAdaptorRec(adaptor);
}

/*
 *-----------------------------------------------------------------------------
 *
 * vmw_video_init_adaptor --
 *
 *    Initializes a XF86VideoAdaptor structure with the capabilities and
 *    functions supported by this video driver.
 *
 * Results:
 *    On success initialized XF86VideoAdaptor struct or NULL on error
 *
 * Side effects:
 *    None.
 *
 *-----------------------------------------------------------------------------
 */

XF86VideoAdaptorPtr
vmw_video_init_adaptor(ScrnInfoPtr pScrn)
{
    XF86VideoAdaptorPtr adaptor;
    modesettingPtr ms = modesettingPTR(pScrn);
    int i;
    DevUnion *dev_unions;
    uint32_t ntot, nfree;

    if (vmwgfx_is_hosted(ms->hdriver))
	return NULL;

    if (vmwgfx_num_streams(ms->fd, &ntot, &nfree) != 0) {
        debug_printf("No stream ioctl support\n");
        return NULL;
    }
    if (nfree == 0) {
        debug_printf("No free streams\n");
        return NULL;
    }
    adaptor = xf86XVAllocateVideoAdaptorRec(pScrn);
    dev_unions = calloc(VMWARE_VID_NUM_PORTS, sizeof(DevUnion));
    if (adaptor == NULL || dev_unions == NULL) {
	xf86XVFreeVideoAdaptorRec(adaptor);
	free(dev_unions);
	return NULL;
    }

    adaptor->type = XvInputMask | XvImageMask | XvWindowMask;

    /**
     * Note: CLIP_TO_VIEWPORT was removed from the flags, since with the
     * crtc/output based modesetting, the viewport is not updated on
     * RandR modeswitches. Hence the video may incorrectly be clipped away.
     * The correct approach, (if needed) would be to clip against the
     * scanout area union of all active crtcs. Revisit if needed.
     */

    adaptor->flags = VIDEO_OVERLAID_IMAGES;
    adaptor->name = "VMware Overlay Video Engine";
    adaptor->nEncodings = VMWARE_VID_NUM_ENCODINGS;
    adaptor->pEncodings = vmwareVideoEncodings;
    adaptor->nFormats = VMWARE_VID_NUM_FORMATS;
    adaptor->pFormats = vmwareVideoFormats;
    adaptor->nPorts = VMWARE_VID_NUM_PORTS;
    adaptor->pPortPrivates = dev_unions;

    for (i = 0; i < VMWARE_VID_NUM_PORTS; ++i) {
	struct vmwgfx_overlay_port *priv =
	    vmwgfx_overlay_port_create(ms->fd, pScrn->pScreen);

        adaptor->pPortPrivates[i].ptr = (pointer) priv;
    }

    adaptor->nAttributes = VMWARE_VID_NUM_ATTRIBUTES;
    adaptor->pAttributes = vmwareVideoAttributes;
    adaptor->nImages = VMWARE_VID_NUM_IMAGES;
    adaptor->pImages = vmwareVideoImages;

    adaptor->PutVideo = NULL;
    adaptor->PutStill = NULL;
    adaptor->GetVideo = NULL;
    adaptor->GetStill = NULL;
    adaptor->StopVideo = vmw_xv_stop_video;
    adaptor->SetPortAttribute = vmw_xv_set_port_attribute;
    adaptor->GetPortAttribute = vmw_xv_get_port_attribute;
    adaptor->QueryBestSize = vmw_xv_query_best_size;
    adaptor->PutImage = vmw_xv_put_image;
    adaptor->QueryImageAttributes = vmw_xv_query_image_attributes;

    return adaptor;
}


/*
 *-----------------------------------------------------------------------------
 *
 * vmw_video_port_init --
 *
 *    Initializes a video stream in response to the first PutImage() on a
 *    video stream. The process goes as follows:
 *    - Figure out characteristics according to format
 *    - Allocate offscreen memory
 *    - Pass on video to Play() functions
 *
 * Results:
 *    Success or XvBadAlloc on failure.
 *
 * Side effects:
 *    Video stream is initialized and its first frame sent to the host
 *    (done by VideoPlay() function called at the end)
 *
 *-----------------------------------------------------------------------------
 */

static int
vmw_video_port_init(ScrnInfoPtr pScrn, struct vmwgfx_overlay_port *port,
                    short src_x, short src_y, short drw_x,
                    short drw_y, short src_w, short src_h,
                    short drw_w, short drw_h, int format,
                    unsigned char *buf, short width,
                    short height, RegionPtr clipBoxes, DrawablePtr pDraw)
{
    unsigned short w, h;
    int i, ret;

    debug_printf("\t%s: id %d, format %d\n", __func__, port->streamId, format);

    ret = vmwgfx_claim_stream(port->drm_fd, &port->streamId);
    if (ret != 0)
	return XvBadAlloc;

    w = width;
    h = height;
    /* init all the format attributes, used for buffers */
    port->size = vmw_xv_query_image_attributes(pScrn, format, &w, &h,
                                               port->pitches, port->offsets);

    if (port->size == -1) {
	ret = XvBadAlloc;
	goto out_bad_size;
    }

    for (i = 0; i < VMWARE_VID_NUM_BUFFERS; ++i) {
	ret = vmw_video_buffer_alloc(port->drm_fd, port->size, &port->bufs[i]);
	if (ret != Success)
	    goto out_no_buffer;
    }

    port->currBuf = 0;
    REGION_NULL(pScrn->pScreen, &port->clipBoxes);
    port->play = vmw_video_port_play;
    return port->play(pScrn, port, src_x, src_y, drw_x, drw_y, src_w, src_h,
                      drw_w, drw_h, format, buf, width, height, clipBoxes, pDraw);

  out_no_buffer:
    while(i-- != 0) {
	vmw_video_buffer_free(&port->bufs[i]);
    }
  out_bad_size:
    (void) vmwgfx_unref_stream(port->drm_fd, port->streamId);

    return ret;
}


/*
 *-----------------------------------------------------------------------------
 *
 * vmw_video_port_play --
 *
 *    Sends all the attributes associated with the video frame using the
 *    FIFO ESCAPE mechanism to the host.
 *
 * Results:
 *    Always returns Success.
 *
 * Side effects:
 *    None.
 *
 *-----------------------------------------------------------------------------
 */

static int
vmw_video_port_play(ScrnInfoPtr pScrn, struct vmwgfx_overlay_port *port,
                    short src_x, short src_y, short drw_x,
                    short drw_y, short src_w, short src_h,
                    short drw_w, short drw_h, int format,
                    unsigned char *buf, short width,
                    short height, RegionPtr clipBoxes, DrawablePtr pDraw)
{
    struct drm_vmw_control_stream_arg arg;
    unsigned short w, h;
    int size;
    int ret;

    debug_printf("\t%s: enter\n", __func__);

    w = width;
    h = height;

    /* we don't update the ports size */
    size = vmw_xv_query_image_attributes(pScrn, format, &w, &h,
                                         port->pitches, port->offsets);

    if (size != port->size) {
        vmw_xv_stop_video(pScrn, port, TRUE);
        return port->play(pScrn, port, src_x, src_y, drw_x, drw_y, src_w,
                          src_h, drw_w, drw_h, format, buf, width, height,
                          clipBoxes, pDraw);
    }

    memcpy(port->bufs[port->currBuf].data, buf, port->size);

    memset(&arg, 0, sizeof(arg));

    arg.stream_id = port->streamId;
    arg.enabled = TRUE;
    arg.flags = port->flags;
    arg.color_key = port->colorKey;
    arg.handle = port->bufs[port->currBuf].buf->handle;
    arg.format = format;
    arg.size = port->size;
    arg.width = w;
    arg.height = h;
    arg.src.x = src_x;
    arg.src.y = src_y;
    arg.src.w = src_w;
    arg.src.h = src_h;
    arg.dst.x = drw_x;
    arg.dst.y = drw_y;
    arg.dst.w = drw_w;
    arg.dst.h = drw_h;
    arg.pitch[0] = port->pitches[0];
    arg.pitch[1] = port->pitches[1];
    arg.pitch[2] = port->pitches[2];
    arg.offset = 0;

    /*
     *  Update the clipList and paint the colorkey, if required.
     */
    if (!REGION_EQUAL(pScrn->pScreen, &port->clipBoxes, clipBoxes)) {
        REGION_COPY(pScrn->pScreen, &port->clipBoxes, clipBoxes);
        if (port->isAutoPaintColorkey) {
            if (pDraw->type == DRAWABLE_WINDOW) {
                xf86XVFillKeyHelperDrawable(pDraw, port->colorKey, clipBoxes);
                DamageDamageRegion(pDraw, clipBoxes);
            } else {
                xf86XVFillKeyHelper(pScrn->pScreen, port->colorKey, clipBoxes);
            }
        }
    }

    xorg_flush(pScrn->pScreen);
    ret = drmCommandWrite(port->drm_fd, DRM_VMW_CONTROL_STREAM, &arg, sizeof(arg));
    if (ret) {
	vmw_video_port_cleanup(pScrn, port);
	return XvBadAlloc;
    }

    if (++(port->currBuf) >= VMWARE_VID_NUM_BUFFERS)
	port->currBuf = 0;

    return Success;
}


/*
 *-----------------------------------------------------------------------------
 *
 * vmw_video_port_cleanup --
 *
 *    Frees up all resources (if any) taken by a video stream.
 *
 * Results:
 *    None.
 *
 * Side effects:
 *    Same as above.
 *
 *-----------------------------------------------------------------------------
 */

static void
vmw_video_port_cleanup(ScrnInfoPtr pScrn, struct vmwgfx_overlay_port *port)
{
    int i;

    debug_printf("\t%s: enter\n", __func__);

    if (port->play == vmw_video_port_init)
	return;

    port->play = vmw_video_port_init;
    (void) vmwgfx_unref_stream(port->drm_fd, port->streamId);

    for (i = 0; i < VMWARE_VID_NUM_BUFFERS; i++) {
	vmw_video_buffer_free(&port->bufs[i]);
    }

    REGION_UNINIT(pScreen->pScreen, &port->clipBoxes);
}


/*
 *-----------------------------------------------------------------------------
 *
 * vmw_video_buffer_alloc --
 *
 *    Allocates and map a kernel buffer to be used as data storage.
 *
 * Results:
 *    XvBadAlloc on failure, otherwise Success.
 *
 * Side effects:
 *    Calls into the kernel, sets members of out.
 *
 *-----------------------------------------------------------------------------
 */

static int
vmw_video_buffer_alloc(int drm_fd, int size,
                       struct vmw_video_buffer *out)
{
    out->buf = vmwgfx_dmabuf_alloc(drm_fd, size);
    if (!out->buf)
	return XvBadAlloc;

    out->data = vmwgfx_dmabuf_map(out->buf);
    if (!out->data) {
	vmwgfx_dmabuf_destroy(out->buf);
	out->buf = NULL;
	return XvBadAlloc;
    }

    out->size = size;
    debug_printf("\t\t%s: allocated buffer %p of size %i\n", __func__, out, size);

    return Success;
}


/*
 *-----------------------------------------------------------------------------
 *
 * vmw_video_buffer_free --
 *
 *    Frees and unmaps an allocated kernel buffer.
 *
 * Results:
 *    Success.
 *
 * Side effects:
 *    Calls into the kernel, sets members of out to 0.
 *
 *-----------------------------------------------------------------------------
 */

static int
vmw_video_buffer_free(struct vmw_video_buffer *out)
{
    if (out->size == 0)
	return Success;

    vmwgfx_dmabuf_unmap(out->buf);
    vmwgfx_dmabuf_destroy(out->buf);

    out->buf = NULL;
    out->data = NULL;
    out->size = 0;

    debug_printf("\t\t%s: freed buffer %p\n", __func__, out);

    return Success;
}


/*
 *-----------------------------------------------------------------------------
 *
 * vmw_xv_put_image --
 *
 *    Main video playback function. It copies the passed data which is in
 *    the specified format (e.g. FOURCC_YV12) into the overlay.
 *
 *    If sync is TRUE the driver should not return from this
 *    function until it is through reading the data from buf.
 *
 * Results:
 *    Success or XvBadAlloc on failure
 *
 * Side effects:
 *    Video port will be played(initialized if 1st frame) on success
 *    or will fail on error.
 *
 *-----------------------------------------------------------------------------
 */

static int
vmw_xv_put_image(ScrnInfoPtr pScrn, short src_x, short src_y,
                 short drw_x, short drw_y, short src_w, short src_h,
                 short drw_w, short drw_h, int format,
                 unsigned char *buf, short width, short height,
                 Bool sync, RegionPtr clipBoxes, pointer data,
                 DrawablePtr dst)
{
    struct vmwgfx_overlay_port *port = data;

    debug_printf("%s: enter (%u, %u) (%ux%u) (%u, %u) (%ux%u) (%ux%u)\n", __func__,
		 src_x, src_y, src_w, src_h,
		 drw_x, drw_y, drw_w, drw_h,
		 width, height);

    return port->play(pScrn, port, src_x, src_y, drw_x, drw_y, src_w, src_h,
                      drw_w, drw_h, format, buf, width, height, clipBoxes, dst);
}


/*
 *-----------------------------------------------------------------------------
 *
 * vmw_xv_stop_video --
 *
 *    Called when we should stop playing video for a particular stream. If
 *    Cleanup is FALSE, the "stop" operation is only temporary, and thus we
 *    don't do anything. If Cleanup is TRUE we kill the video port by
 *    sending a message to the host and freeing up the stream.
 *
 * Results:
 *    None.
 *
 * Side effects:
 *    See above.
 *
 *-----------------------------------------------------------------------------
 */

static void
vmw_xv_stop_video(ScrnInfoPtr pScrn, pointer data, Bool cleanup)
{
    struct vmwgfx_overlay_port *port = data;

    debug_printf("%s: cleanup is %s\n", __func__, cleanup ? "TRUE" : "FALSE");
    REGION_EMPTY(pScrn->pScreen, &port->clipBoxes);

    if (!cleanup)
        return;

    vmw_video_port_cleanup(pScrn, port);
}


/*
 *-----------------------------------------------------------------------------
 *
 * vmw_xv_query_image_attributes --
 *
 *    From the spec: This function is called to let the driver specify how data
 *    for a particular image of size width by height should be stored.
 *    Sometimes only the size and corrected width and height are needed. In
 *    that case pitches and offsets are NULL.
 *
 * Results:
 *    The size of the memory required for the image, or -1 on error.
 *
 * Side effects:
 *    None.
 *
 *-----------------------------------------------------------------------------
 */

static int
vmw_xv_query_image_attributes(ScrnInfoPtr pScrn, int format,
                              unsigned short *width, unsigned short *height,
                              int *pitches, int *offsets)
{
    INT32 size, tmp;

    if (*width > VMWARE_VID_MAX_WIDTH) {
        *width = VMWARE_VID_MAX_WIDTH;
    }
    if (*height > VMWARE_VID_MAX_HEIGHT) {
        *height = VMWARE_VID_MAX_HEIGHT;
    }

    *width = (*width + 1) & ~1;
    if (offsets != NULL) {
        offsets[0] = 0;
    }

    switch (format) {
       case FOURCC_YV12:
           *height = (*height + 1) & ~1;
           size = (*width + 3) & ~3;
           if (pitches) {
               pitches[0] = size;
           }
           size *= *height;
           if (offsets) {
               offsets[1] = size;
           }
           tmp = ((*width >> 1) + 3) & ~3;
           if (pitches) {
                pitches[1] = pitches[2] = tmp;
           }
           tmp *= (*height >> 1);
           size += tmp;
           if (offsets) {
               offsets[2] = size;
           }
           size += tmp;
           break;
       case FOURCC_UYVY:
       case FOURCC_YUY2:
           size = *width * 2;
           if (pitches) {
               pitches[0] = size;
           }
           size *= *height;
           break;
       default:
           debug_printf("Query for invalid video format %d\n", format);
           return -1;
    }
    return size;
}


/*
 *-----------------------------------------------------------------------------
 *
 * vmw_xv_set_port_attribute --
 *
 *    From the spec: A port may have particular attributes such as colorKey, hue,
 *    saturation, brightness or contrast. Xv clients set these
 *    attribute values by sending attribute strings (Atoms) to the server.
 *
 * Results:
 *    Success if the attribute exists and XvBadAlloc otherwise.
 *
 * Side effects:
 *    The respective attribute gets the new value.
 *
 *-----------------------------------------------------------------------------
 */

static int
vmw_xv_set_port_attribute(ScrnInfoPtr pScrn, Atom attribute,
                          INT32 value, pointer data)
{
    struct vmwgfx_overlay_port *port = data;
    Atom xvColorKey = MAKE_ATOM("XV_COLORKEY");
    Atom xvAutoPaint = MAKE_ATOM("XV_AUTOPAINT_COLORKEY");

    if (attribute == xvColorKey) {
        debug_printf("%s: Set colorkey:0x%x\n", __func__, (unsigned)value);
        port->colorKey = value;
    } else if (attribute == xvAutoPaint) {
        debug_printf("%s: Set autoPaint: %s\n", __func__, value? "TRUE": "FALSE");
        port->isAutoPaintColorkey = value;
    } else {
        return XvBadAlloc;
    }

    return Success;
}


/*
 *-----------------------------------------------------------------------------
 *
 * vmw_xv_get_port_attribute --
 *
 *    From the spec: A port may have particular attributes such as hue,
 *    saturation, brightness or contrast. Xv clients get these
 *    attribute values by sending attribute strings (Atoms) to the server
 *
 * Results:
 *    Success if the attribute exists and XvBadAlloc otherwise.
 *
 * Side effects:
 *    "value" contains the requested attribute on success.
 *
 *-----------------------------------------------------------------------------
 */

static int
vmw_xv_get_port_attribute(ScrnInfoPtr pScrn, Atom attribute,
                          INT32 *value, pointer data)
{
    struct vmwgfx_overlay_port *port = data;
    Atom xvColorKey = MAKE_ATOM("XV_COLORKEY");
    Atom xvAutoPaint = MAKE_ATOM("XV_AUTOPAINT_COLORKEY");

    if (attribute == xvColorKey) {
        *value = port->colorKey;
    } else if (attribute == xvAutoPaint) {
        *value = port->isAutoPaintColorkey;
    } else {
        return XvBadAlloc;
    }

    return Success;
}


/*
 *-----------------------------------------------------------------------------
 *
 * vmw_xv_query_best_size --
 *
 *    From the spec: QueryBestSize provides the client with a way to query what
 *    the destination dimensions would end up being if they were to request
 *    that an area vid_w by vid_h from the video stream be scaled to rectangle
 *    of drw_w by drw_h on the screen. Since it is not expected that all
 *    hardware will be able to get the target dimensions exactly, it is
 *    important that the driver provide this function.
 *
 *    This function seems to never be called, but to be on the safe side
 *    we apply the same logic that QueryImageAttributes has for width
 *    and height.
 *
 * Results:
 *    None.
 *
 * Side effects:
 *    None.
 *
 *-----------------------------------------------------------------------------
 */

static void
vmw_xv_query_best_size(ScrnInfoPtr pScrn, Bool motion,
                       short vid_w, short vid_h, short drw_w,
                       short drw_h, unsigned int *p_w,
                       unsigned int *p_h, pointer data)
{
    *p_w = (drw_w + 1) & ~1;
    *p_h = drw_h;

    return;
}

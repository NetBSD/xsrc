#ifndef __NV_PROTO_H__
#define __NV_PROTO_H__

/* in drmmode_display.c */
Bool drmmode_pre_init(ScrnInfoPtr pScrn, int fd, int cpp);
void drmmode_adjust_frame(ScrnInfoPtr pScrn, int x, int y);
void drmmode_remove_fb(ScrnInfoPtr pScrn);
Bool drmmode_cursor_init(ScreenPtr pScreen);
void drmmode_fbcon_copy(ScreenPtr pScreen);
Bool drmmode_page_flip(DrawablePtr draw, PixmapPtr back, void *priv,
		       unsigned int ref_crtc_hw_id);
void drmmode_screen_init(ScreenPtr pScreen);
void drmmode_screen_fini(ScreenPtr pScreen);

int  drmmode_crtc(xf86CrtcPtr crtc);
Bool xf86_crtc_on(xf86CrtcPtr crtc);
int  drmmode_head(xf86CrtcPtr crtc);
void drmmode_swap(ScrnInfoPtr, uint32_t, uint32_t *);

void *drmmode_event_queue(ScrnInfoPtr, uint64_t name, unsigned size,
			  void (*)(void *, uint64_t, uint64_t, uint32_t),
			  void **token);
void  drmmode_event_abort(ScrnInfoPtr, uint64_t name, bool pending);
int   drmmode_event_flush(ScrnInfoPtr);

/* in nv_accel_common.c */
Bool NVAccelCommonInit(ScrnInfoPtr pScrn);
void NVAccelCommonFini(ScrnInfoPtr pScrn);
Bool NVAccelGetCtxSurf2DFormatFromPixmap(PixmapPtr pPix, int *fmt_ret);
Bool NVAccelGetCtxSurf2DFormatFromPicture(PicturePtr pPix, int *fmt_ret);
PixmapPtr NVGetDrawablePixmap(DrawablePtr pDraw);
void NV11SyncToVBlank(PixmapPtr ppix, BoxPtr box);
Bool nouveau_allocate_surface(ScrnInfoPtr scrn, int width, int height,
			      int bpp, int usage_hint, int *pitch,
			      struct nouveau_bo **bo);

/* in nouveau_dri2.c */
Bool nouveau_dri2_init(ScreenPtr pScreen);
void nouveau_dri2_fini(ScreenPtr pScreen);
Bool nouveau_dri3_screen_init(ScreenPtr pScreen);

/* in nouveau_xv.c */
void NVInitVideo(ScreenPtr);
void NVTakedownVideo(ScrnInfoPtr);
void NVSetPortDefaults (ScrnInfoPtr pScrn, NVPortPrivPtr pPriv);
void NVXVComputeBicubicFilter(struct nouveau_bo *, unsigned, unsigned);
unsigned int nv_window_belongs_to_crtc(ScrnInfoPtr, int, int, int, int);
xf86CrtcPtr nouveau_pick_best_crtc(ScrnInfoPtr pScrn,
                                   int x, int y, int w, int h);
RRCrtcPtr randr_crtc_covering_drawable(DrawablePtr pDraw);

/* in nouveau_exa.c */
Bool nouveau_exa_init(ScreenPtr pScreen);
Bool nouveau_exa_pixmap_is_onscreen(PixmapPtr pPixmap);
bool nv50_style_tiled_pixmap(PixmapPtr ppix);
Bool NVAccelM2MF(NVPtr pNv, int w, int h, int cpp, uint32_t srco, uint32_t dsto,
		 struct nouveau_bo *s, int sd, int sp, int sh, int sx, int sy,
		 struct nouveau_bo *d, int dd, int dp, int dh, int dx, int dy);


/* in nouveau_wfb.c */
void nouveau_wfb_setup_wrap(ReadMemoryProcPtr *, WriteMemoryProcPtr *,
			    DrawablePtr);
void nouveau_wfb_finish_wrap(DrawablePtr);

/* in nv_shadow.c */
void NVRefreshArea(ScrnInfoPtr pScrn, int num, BoxPtr pbox);

/* in nv04_video_overlay.c */
void NV04PutOverlayImage(ScrnInfoPtr, struct nouveau_bo *, int, int, int,
			 BoxPtr, int, int, int, int, short, short, short,
			 short, short, short, RegionPtr clipBoxes);
int NV04SetOverlayPortAttribute(ScrnInfoPtr, Atom, INT32, pointer);
int NV04GetOverlayPortAttribute(ScrnInfoPtr, Atom, INT32 *, pointer);
void NV04StopOverlay(ScrnInfoPtr);

/* in nv04_video_blitter.c */
Bool NVPutBlitImage(ScrnInfoPtr, struct nouveau_bo *, int, int, int, BoxPtr,
		    int, int, int, int, short, short, short, short, short,
		    short, RegionPtr, PixmapPtr);
int NVSetBlitPortAttribute(ScrnInfoPtr, Atom, INT32, pointer);
int NVGetBlitPortAttribute(ScrnInfoPtr, Atom, INT32 *, pointer);
void NVStopBlitVideo(ScrnInfoPtr, pointer, Bool);

/* in nv04_exa.c */
Bool NV04EXAPrepareSolid(PixmapPtr, int, Pixel, Pixel);
void NV04EXASolid(PixmapPtr, int, int, int, int);
void NV04EXADoneSolid(PixmapPtr);
Bool NV04EXAPrepareCopy(PixmapPtr, PixmapPtr, int, int, int, Pixel);
void NV04EXACopy(PixmapPtr, int, int, int, int, int, int);
void NV04EXADoneCopy(PixmapPtr);
Bool NV04EXAUploadIFC(ScrnInfoPtr, const char *src, int src_pitch,
		      PixmapPtr pdPix, int x, int y, int w, int h, int cpp);
Bool NV04EXARectM2MF(NVPtr pNv, int, int, int,
		     struct nouveau_bo *, uint32_t, int, int, int, int, int,
		     struct nouveau_bo *, uint32_t, int, int, int, int, int);

/* in nv10_exa.c */
Bool NVAccelInitNV10TCL(ScrnInfoPtr pScrn);
Bool NV10EXACheckComposite(int, PicturePtr, PicturePtr, PicturePtr);
Bool NV10EXAPrepareComposite(int, PicturePtr, PicturePtr, PicturePtr,
			     PixmapPtr, PixmapPtr, PixmapPtr);
void NV10EXAComposite(PixmapPtr, int, int, int, int, int, int, int, int);
void NV10EXADoneComposite(PixmapPtr);

/* in nv10_video_overlay.c */
void NV10PutOverlayImage(ScrnInfoPtr, struct nouveau_bo *, int, int, int, int,
			 BoxPtr, int, int, int, int, short, short, short,
			 short, short, short, RegionPtr clipBoxes);
int NV10SetOverlayPortAttribute(ScrnInfoPtr, Atom, INT32, pointer);
int NV10GetOverlayPortAttribute(ScrnInfoPtr, Atom, INT32 *, pointer);
void NV10StopOverlay(ScrnInfoPtr);
void NV10WriteOverlayParameters(ScrnInfoPtr);

/* in nv30_exa.c */
Bool NVAccelInitNV30TCL(ScrnInfoPtr pScrn);
Bool NV30EXACheckComposite(int, PicturePtr, PicturePtr, PicturePtr);
Bool NV30EXAPrepareComposite(int, PicturePtr, PicturePtr, PicturePtr,
				  PixmapPtr, PixmapPtr, PixmapPtr);
void NV30EXAComposite(PixmapPtr, int, int, int, int, int, int, int, int);
void NV30EXADoneComposite(PixmapPtr);

/* in nv30_video_texture.c */
int NV30PutTextureImage(ScrnInfoPtr, struct nouveau_bo *, int, int, int, int,
			BoxPtr, int, int, int, int, uint16_t, uint16_t,
			uint16_t, uint16_t, uint16_t, uint16_t,
			RegionPtr, PixmapPtr, NVPortPrivPtr);
void NV30StopTexturedVideo(ScrnInfoPtr, pointer, Bool);
int NV30GetTexturePortAttribute(ScrnInfoPtr, Atom, INT32 *, pointer);
int NV30SetTexturePortAttribute(ScrnInfoPtr, Atom, INT32, pointer);

/* in nv40_exa.c */
Bool NVAccelInitNV40TCL(ScrnInfoPtr pScrn);
Bool NV40EXACheckComposite(int, PicturePtr, PicturePtr, PicturePtr);
Bool NV40EXAPrepareComposite(int, PicturePtr, PicturePtr, PicturePtr,
				  PixmapPtr, PixmapPtr, PixmapPtr);
void NV40EXAComposite(PixmapPtr, int, int, int, int, int, int, int, int);
void NV40EXADoneComposite(PixmapPtr);

/* in nv40_video_texture.c */
int NV40PutTextureImage(ScrnInfoPtr, struct nouveau_bo *, int, int, int, int,
			BoxPtr, int, int, int, int, uint16_t, uint16_t,
			uint16_t, uint16_t, uint16_t, uint16_t,
			RegionPtr, PixmapPtr, NVPortPrivPtr);
void NV40StopTexturedVideo(ScrnInfoPtr, pointer, Bool);
int NV40GetTexturePortAttribute(ScrnInfoPtr, Atom, INT32 *, pointer);
int NV40SetTexturePortAttribute(ScrnInfoPtr, Atom, INT32, pointer);

/* in nv50_accel.c */
void NV50SyncToVBlank(PixmapPtr ppix, BoxPtr box);
Bool NVAccelInitM2MF_NV50(ScrnInfoPtr pScrn);
Bool NVAccelInit2D_NV50(ScrnInfoPtr pScrn);
Bool NVAccelInitNV50TCL(ScrnInfoPtr pScrn);

/* in nvc0_accel.c */
void NVC0SyncToVBlank(PixmapPtr ppix, BoxPtr box);
Bool NVAccelInitM2MF_NVC0(ScrnInfoPtr pScrn);
Bool NVAccelInitP2MF_NVE0(ScrnInfoPtr pScrn);
Bool NVAccelInitCOPY_NVE0(ScrnInfoPtr pScrn);
Bool NVAccelInit2D_NVC0(ScrnInfoPtr pScrn);
Bool NVAccelInit3D_NVC0(ScrnInfoPtr pScrn);

/* in nv50_exa.c */
Bool NV50EXAPrepareSolid(PixmapPtr, int, Pixel, Pixel);
void NV50EXASolid(PixmapPtr, int, int, int, int);
void NV50EXADoneSolid(PixmapPtr);
Bool NV50EXAPrepareCopy(PixmapPtr, PixmapPtr, int, int, int, Pixel);
void NV50EXACopy(PixmapPtr, int, int, int, int, int, int);
void NV50EXADoneCopy(PixmapPtr);
Bool NV50EXACheckComposite(int, PicturePtr, PicturePtr, PicturePtr);
Bool NV50EXAPrepareComposite(int, PicturePtr, PicturePtr, PicturePtr,
				  PixmapPtr, PixmapPtr, PixmapPtr);
void NV50EXAComposite(PixmapPtr, int, int, int, int, int, int, int, int);
void NV50EXADoneComposite(PixmapPtr);
Bool NV50EXAUploadSIFC(const char *src, int src_pitch,
		       PixmapPtr pdPix, int x, int y, int w, int h, int cpp);
Bool NV50EXARectM2MF(NVPtr pNv, int, int, int,
		     struct nouveau_bo *, uint32_t, int, int, int, int, int,
		     struct nouveau_bo *, uint32_t, int, int, int, int, int);

/* in nvc0_exa.c */
Bool NVC0AccelUploadM2MF(PixmapPtr pdpix, int x, int y, int w, int h,
			 const char *src, int src_pitch);
Bool NVC0AccelDownloadM2MF(PixmapPtr pspix, int x, int y, int w, int h,
			   char *dst, unsigned dst_pitch);

Bool NVC0EXAPrepareSolid(PixmapPtr, int, Pixel, Pixel);
void NVC0EXASolid(PixmapPtr, int, int, int, int);
void NVC0EXADoneSolid(PixmapPtr);
Bool NVC0EXAPrepareCopy(PixmapPtr, PixmapPtr, int, int, int, Pixel);
void NVC0EXACopy(PixmapPtr, int, int, int, int, int, int);
void NVC0EXADoneCopy(PixmapPtr);
Bool NVC0EXACheckComposite(int, PicturePtr, PicturePtr, PicturePtr);
Bool NVC0EXAPrepareComposite(int, PicturePtr, PicturePtr, PicturePtr,
				  PixmapPtr, PixmapPtr, PixmapPtr);
void NVC0EXAComposite(PixmapPtr, int, int, int, int, int, int, int, int);
void NVC0EXADoneComposite(PixmapPtr);
Bool NVC0EXAUploadSIFC(const char *src, int src_pitch,
		       PixmapPtr pdPix, int x, int y, int w, int h, int cpp);
Bool NVC0EXARectM2MF(NVPtr pNv, int, int, int,
		     struct nouveau_bo *, uint32_t, int, int, int, int, int,
		     struct nouveau_bo *, uint32_t, int, int, int, int, int);

Bool NVE0EXARectCopy(NVPtr pNv, int, int, int,
		     struct nouveau_bo *, uint32_t, int, int, int, int, int,
		     struct nouveau_bo *, uint32_t, int, int, int, int, int);

/* nv50_xv.c */
int nv50_xv_image_put(ScrnInfoPtr, struct nouveau_bo *, int, int, int, int,
		      BoxPtr, int, int, int, int, uint16_t, uint16_t,
		      uint16_t, uint16_t, uint16_t, uint16_t,
		      RegionPtr, PixmapPtr, NVPortPrivPtr);
void nv50_xv_video_stop(ScrnInfoPtr, pointer, Bool);
int nv50_xv_port_attribute_set(ScrnInfoPtr, Atom, INT32, pointer);
int nv50_xv_port_attribute_get(ScrnInfoPtr, Atom, INT32 *, pointer);
void nv50_xv_set_port_defaults(ScrnInfoPtr, NVPortPrivPtr);
void nv50_xv_csc_update(ScrnInfoPtr, NVPortPrivPtr);

/* nvc0_xv.c */
int nvc0_xv_image_put(ScrnInfoPtr, struct nouveau_bo *, int, int, int, int,
		      BoxPtr, int, int, int, int, uint16_t, uint16_t,
		      uint16_t, uint16_t, uint16_t, uint16_t,
		      RegionPtr, PixmapPtr, NVPortPrivPtr);
void nvc0_xv_csc_update(NVPtr, float, float *, float *, float *);

/* To support EXA 2.0, 2.1 has this in the header */
#ifndef exaMoveInPixmap
extern void exaMoveInPixmap(PixmapPtr pPixmap);
#endif

extern Bool wfbPictureInit(ScreenPtr, PictFormatPtr, int);

extern Bool wfbScreenInit(ScreenPtr pScreen,
              void *pbits,
              int xsize,
              int ysize,
              int dpix,
              int dpiy,
              int width,
              int bpp,
              SetupWrapProcPtr setupWrap, FinishWrapProcPtr finishWrap);

#endif /* __NV_PROTO_H__ */


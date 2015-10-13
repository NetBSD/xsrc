#ifndef __NOUVEAU_GLAMOR_H__
#define __NOUVEAU_GLAMOR_H__

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "xf86xv.h"

#ifdef HAVE_GLAMOR
#include "nv_include.h"
#define GLAMOR_FOR_XORG 1
#include "glamor.h"

Bool nouveau_glamor_pre_init(ScrnInfoPtr scrn);
Bool nouveau_glamor_init(ScreenPtr screen);
Bool nouveau_glamor_create_screen_resources(ScreenPtr screen);
XF86VideoAdaptorPtr nouveau_glamor_xv_init(ScreenPtr pScreen, int num_adapt);
void nouveau_glamor_pixmap_set(PixmapPtr pixmap, struct nouveau_pixmap *priv);
struct nouveau_pixmap *nouveau_glamor_pixmap_get(PixmapPtr pixmap);
#else
static inline Bool nouveau_glamor_pre_init(ScrnInfoPtr scrn) { return FALSE; }
static inline Bool nouveau_glamor_init(ScreenPtr screen) { return FALSE; }
static inline Bool
nouveau_glamor_create_screen_resources(ScreenPtr screen) { return FALSE; }
static inline void
nouveau_glamor_pixmap_set(PixmapPtr pixmap, void *priv) { }
static inline struct nouveau_pixmap *
nouveau_glamor_pixmap_get(PixmapPtr pixmap) { return NULL; }
static inline XF86VideoAdaptorPtr
nouveau_glamor_xv_init(ScreenPtr pScreen, int num_adapt) { return NULL; }
#endif

#endif

#ifndef __NOUVEAU_SYNC_H__
#define __NOUVEAU_SYNC_H__

#include "xorg-server.h"
#include "scrnintstr.h"

#ifdef DRI3
#include "misync.h"
#include "misyncshm.h"
#include "misyncstr.h"

#define wrap(priv, parn, name, func) {                                         \
    priv->name = parn->name;                                                   \
    parn->name = func;                                                         \
}

#define unwrap(priv, parn, name) {                                             \
    if (priv && priv->name)                                                    \
	parn->name = priv->name;                                               \
}

#define swap(priv, parn, name) {                                               \
    void *tmp = priv->name;                                                    \
    priv->name = parn->name;                                                   \
    parn->name = tmp;                                                          \
}

Bool nouveau_sync_init(ScreenPtr pScreen);
void nouveau_sync_fini(ScreenPtr pScreen);
#else
static inline Bool nouveau_sync_init(ScreenPtr pScreen) { return FALSE; }
static inline void nouveau_sync_fini(ScreenPtr pScreen) { }
#endif
#endif

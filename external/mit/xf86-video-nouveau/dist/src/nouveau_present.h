#ifndef __NOUVEAU_PRESENT_H__
#define __NOUVEAU_PRESENT_H__

#include "xorg-server.h"
#include "scrnintstr.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#if defined(DRI3)
#include "present.h"
Bool nouveau_present_init(ScreenPtr pScreen);
void nouveau_present_fini(ScreenPtr pScreen);
#else
static inline Bool nouveau_present_init(ScreenPtr pScreen) { return FALSE; }
static inline void nouveau_present_fini(ScreenPtr pScreen) { }
#endif
#endif

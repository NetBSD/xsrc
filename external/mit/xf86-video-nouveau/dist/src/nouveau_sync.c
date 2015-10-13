/*
 * Copyright © 2013-2014 Intel Corporation
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that copyright
 * notice and this permission notice appear in supporting documentation, and
 * that the name of the copyright holders not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  The copyright holders make no representations
 * about the suitability of this software for any purpose.  It is provided "as
 * is" without express or implied warranty.
 *
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
 * OF THIS SOFTWARE.
 */

#include "nouveau_sync.h"
#ifdef DRI3
#include "nv_include.h"

static DevPrivateKeyRec nouveau_syncobj_key;

struct nouveau_syncobj {
	SyncFenceSetTriggeredFunc SetTriggered;
};

#define nouveau_syncobj(fence)                                                 \
	dixLookupPrivate(&(fence)->devPrivates, &nouveau_syncobj_key)

struct nouveau_syncctx {
	SyncScreenCreateFenceFunc CreateFence;
};

#define nouveau_syncctx(screen) ({                                             \
	ScrnInfoPtr scrn = xf86ScreenToScrn(screen);                           \
	NVPtr pNv = NVPTR(scrn);                                               \
	pNv->sync;                                                             \
})

static void
nouveau_syncobj_flush(SyncFence *fence)
{
	struct nouveau_syncobj *pobj = nouveau_syncobj(fence);
	ScrnInfoPtr scrn = xf86ScreenToScrn(fence->pScreen);
	NVPtr pNv = NVPTR(scrn);
	SyncFenceFuncsPtr func = &fence->funcs;

	if (pNv->Flush)
		pNv->Flush(scrn);

	swap(pobj, func, SetTriggered);
	func->SetTriggered(fence);
	swap(pobj, func, SetTriggered);
}

static void
nouveau_syncobj_new(ScreenPtr screen, SyncFence *fence, Bool triggered)
{
	struct nouveau_syncctx *priv = nouveau_syncctx(screen);
	struct nouveau_syncobj *pobj = nouveau_syncobj(fence);
	SyncScreenFuncsPtr sync = miSyncGetScreenFuncs(screen);
	SyncFenceFuncsPtr func = &fence->funcs;

	swap(priv, sync, CreateFence);
	sync->CreateFence(screen, fence, triggered);
	swap(priv, sync, CreateFence);

	wrap(pobj, func, SetTriggered, nouveau_syncobj_flush);
}

void
nouveau_sync_fini(ScreenPtr screen)
{
	struct nouveau_syncctx *priv = nouveau_syncctx(screen);
	SyncScreenFuncsPtr sync = miSyncGetScreenFuncs(screen);
	ScrnInfoPtr scrn = xf86ScreenToScrn(screen);
	NVPtr pNv = NVPTR(scrn);

	unwrap(priv, sync, CreateFence);

	pNv->sync = NULL;
	free(priv);
}

Bool
nouveau_sync_init(ScreenPtr screen)
{
	ScrnInfoPtr scrn = xf86ScreenToScrn(screen);
	NVPtr pNv = NVPTR(scrn);
	struct nouveau_syncctx *priv;
	SyncScreenFuncsPtr sync;

	priv = pNv->sync = calloc(1, sizeof(*priv));
	if (!priv)
		return FALSE;

	if (!miSyncShmScreenInit(screen))
		return FALSE;

	if (!dixPrivateKeyRegistered(&nouveau_syncobj_key)) {
		if (!dixRegisterPrivateKey(&nouveau_syncobj_key,
					    PRIVATE_SYNC_FENCE,
					   sizeof(struct nouveau_syncobj)))
			return FALSE;
	}

	sync = miSyncGetScreenFuncs(screen);
	wrap(priv, sync, CreateFence, nouveau_syncobj_new);
	return TRUE;
}
#endif

/**************************************************************************

Copyright 1998-1999 Precision Insight, Inc., Cedar Park, Texas.
Copyright 2000 VA Linux Systems, Inc.
All Rights Reserved.

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sub license, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice (including the
next paragraph) shall be included in all copies or substantial portions
of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
IN NO EVENT SHALL PRECISION INSIGHT AND/OR ITS SUPPLIERS BE LIABLE FOR
ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

**************************************************************************/
/* $XFree86: xc/lib/GL/mesa/src/drv/tdfx/tdfx_xmesa.c,v 1.10 2000/12/08 19:36:24 alanh Exp $ */

/*
 * Authors:
 *   Daryll Strauss <daryll@valinux.com>
 *   Brian E. Paul <brianp@valinux.com>
 */

#ifdef GLX_DIRECT_RENDERING

#include <X11/Xlibint.h>
#include <glide.h>
#include "fxdrv.h"
#include "context.h"
#include "matrix.h"
#include "mmath.h"
#include "vbxform.h"
#include "fxtexman.h"


/* including xf86PciInfo.h causes a bunch of errors */
#ifndef PCI_CHIP_VOODOO5
#define PCI_CHIP_VOODOO5	0x0009
#endif


GLboolean
XMesaInitDriver(__DRIscreenPrivate * sPriv)
{
    tdfxScreenPrivate *gsp;

    /* Check the DRI version */
    {
        int major, minor, patch;
        if (XF86DRIQueryVersion(sPriv->display, &major, &minor, &patch)) {
            if (major != 3 || minor != 0 || patch < 0) {
                char msg[1000];
                sprintf(msg,
                        "3dfx DRI driver expected DRI version 3.0.x but got version %d.%d.%d",
                        major, minor, patch);
                __driMesaMessage(msg);
                return GL_FALSE;
            }
        }
    }

    /* Check that the DDX driver version is compatible */
    if (sPriv->ddxMajor != 1 || sPriv->ddxMinor != 0 || sPriv->ddxPatch < 0) {
        char msg[1000];
        sprintf(msg,
                "3dfx DRI driver expected DDX driver version 1.0.x but got version %d.%d.%d",
                sPriv->ddxMajor, sPriv->ddxMinor, sPriv->ddxPatch);
        __driMesaMessage(msg);
        return GL_FALSE;
    }

    /* Check that the DRM driver version is compatible */
    if (sPriv->drmMajor != 1 || sPriv->drmMinor != 0 || sPriv->drmPatch < 0) {
        char msg[1000];
        sprintf(msg,
                "3dfx DRI driver expected DRM driver version 1.0.x but got version %d.%d.%d",
                sPriv->drmMajor, sPriv->drmMinor, sPriv->drmPatch);
        __driMesaMessage(msg);
        return GL_FALSE;
    }

    /* Allocate the private area */
    gsp = (tdfxScreenPrivate *) Xmalloc(sizeof(tdfxScreenPrivate));
    if (!gsp)
        return GL_FALSE;

    gsp->driScrnPriv = sPriv;

    sPriv->private = (void *) gsp;

    if (!tdfxMapAllRegions(sPriv)) {
        Xfree(gsp);
        sPriv->private = NULL;
        return GL_FALSE;
    }

    return GL_TRUE;
}


void
XMesaResetDriver(__DRIscreenPrivate * sPriv)
{
    tdfxUnmapAllRegions(sPriv);
    Xfree(sPriv->private);
    sPriv->private = NULL;
}


GLvisual *
XMesaCreateVisual(Display * dpy,
                  __DRIscreenPrivate * driScrnPriv,
                  const XVisualInfo * visinfo,
                  const __GLXvisualConfig * config)
{
    /* Drivers may change the args to _mesa_create_visual() in order to
     * setup special visuals.
     */
    return _mesa_create_visual(config->rgba,
                               config->doubleBuffer,
                               config->stereo,
                               _mesa_bitcount(visinfo->red_mask),
                               _mesa_bitcount(visinfo->green_mask),
                               _mesa_bitcount(visinfo->blue_mask),
                               config->alphaSize,
                               0, /* index bits */
                               config->depthSize,
                               config->stencilSize,
                               config->accumRedSize,
                               config->accumGreenSize,
                               config->accumBlueSize,
                               config->accumAlphaSize,
                               0 /* num samples */
        );
}


GLboolean
XMesaCreateContext(Display * dpy, GLvisual * mesaVis,
                   __DRIcontextPrivate * driContextPriv)
{
    fxMesaContext fxMesa;
    __DRIscreenPrivate *driScrnPriv = driContextPriv->driScreenPriv;
    tdfxScreenPrivate *sPriv = (tdfxScreenPrivate *) driScrnPriv->private;
    TDFXSAREAPriv *saPriv;

    fxMesa = (fxMesaContext) Xmalloc(sizeof(struct tfxMesaContext));
    if (!fxMesa) {
        return GL_FALSE;
    }

    fxMesa->hHWContext = driContextPriv->hHWContext;
    fxMesa->tdfxScrnPriv = sPriv;
    /* deviceID = 0x05 = Voodoo3 */
    /* deviceID = 0x09 = Voodoo5 (and Voodoo4?) */
    fxMesa->isNapalm = sPriv->deviceID == PCI_CHIP_VOODOO5;
    fxMesa->haveHwStencil = fxMesa->isNapalm && sPriv->cpp == 4;


    fxMesa->glVis = mesaVis;
    fxMesa->screen_width = sPriv->width;
    fxMesa->screen_height = sPriv->height;
    fxMesa->new_state = ~0;
    fxMesa->driContextPriv = driContextPriv;
    fxMesa->glCtx = driContextPriv->mesaContext;
    fxMesa->initDone = GL_FALSE;

    saPriv =
        (TDFXSAREAPriv *) ((char *) driScrnPriv->pSAREA +
                           sizeof(XF86DRISAREARec));
    grDRIOpen(driScrnPriv->pFB, sPriv->regs.map, sPriv->deviceID,
              sPriv->width, sPriv->height, sPriv->mem, sPriv->cpp,
              sPriv->stride, sPriv->fifoOffset, sPriv->fifoSize,
              sPriv->fbOffset, sPriv->backOffset, sPriv->depthOffset,
              sPriv->textureOffset, sPriv->textureSize, &saPriv->fifoPtr,
              &saPriv->fifoRead);

    driContextPriv->driverPrivate = (void *) fxMesa;

    return GL_TRUE;
}


void
XMesaDestroyContext(__DRIcontextPrivate * driContextPriv)
{
    fxMesaContext fxMesa = (fxMesaContext) driContextPriv->driverPrivate;
    if (fxMesa) {
        if (fxMesa->glCtx->Shared->RefCount == 1) {
           /* This share group is about to go away, free our private
            * texture object data.
            */
            struct gl_texture_object *tObj;
            tObj = fxMesa->glCtx->Shared->TexObjectList;
            while (tObj) {
                fxTMFreeTexture(fxMesa, tObj);
                tObj = tObj->Next;
            }
        }
        XFree(fxMesa);
        driContextPriv->driverPrivate = NULL;
    }
}


GLframebuffer *
XMesaCreateWindowBuffer(Display * dpy,
                        __DRIscreenPrivate * driScrnPriv,
                        __DRIdrawablePrivate * driDrawPriv,
                        GLvisual * mesaVis)
{
    return gl_create_framebuffer(mesaVis,
                                 GL_FALSE, /* software depth buffer? */
                                 mesaVis->StencilBits > 0,
                                 mesaVis->AccumRedBits > 0,
                                 GL_FALSE /* software alpha channel? */
                                 );
}


GLframebuffer *
XMesaCreatePixmapBuffer(Display * dpy,
                        __DRIscreenPrivate * driScrnPriv,
                        __DRIdrawablePrivate * driDrawPriv,
                        GLvisual * mesaVis)
{
#if 0
    /* Different drivers may have different combinations of hardware and
     * software ancillary buffers.
     */
    return gl_create_framebuffer(mesaVis,
                                 GL_FALSE, /* software depth buffer? */
                                 mesaVis->StencilBits > 0,
                                 mesaVis->AccumRedBits > 0,
                                 mesaVis->AlphaBits > 0);
#else
    return NULL;                /* not implemented yet */
#endif
}


void
XMesaSwapBuffers(__DRIdrawablePrivate * driDrawPriv)
{
    GET_CURRENT_CONTEXT(ctx);
    fxMesaContext fxMesa = 0;

    if (!driDrawPriv->mesaBuffer->Visual->DBflag)
       return; /* can't swap a single-buffered window */

    /* If the current context's drawable matches the given drawable
     * we have to do a glFinish (per the GLX spec).
     */
    if (ctx) {
        __DRIdrawablePrivate *curDrawPriv;
        fxMesa = FX_CONTEXT(ctx);
        curDrawPriv = fxMesa->driContextPriv->driDrawablePriv;
        if (curDrawPriv == driDrawPriv) {
            /* swapping window bound to current context, flush first */
            FLUSH_VB(ctx, "swap buffers");
            BEGIN_BOARD_LOCK(fxMesa);
        }
        else {
            /* make fxMesa context current */
            grGlideGetState((GrState *) fxMesa->state);
            fxMesa = (fxMesaContext) driDrawPriv->driContextPriv->driverPrivate;
            BEGIN_BOARD_LOCK(fxMesa);
            grSstSelect(fxMesa->board);
            grGlideSetState((GrState *) fxMesa->state);
        }
    }


#ifdef STATS
    {
        int stalls;
        static int prevStalls = 0;
        stalls = grFifoGetStalls();
        if (stalls != prevStalls) {
            fprintf(stderr, "%d stalls occurred\n", stalls - prevStalls);
            prevStalls = stalls;
        }
        if (fxMesa && fxMesa->texSwaps) {
            fprintf(stderr, "%d texture swaps occurred\n", fxMesa->texSwaps);
            fxMesa->texSwaps = 0;
        }
    }
#endif


    /* XXX prototype grDRISwapClipRects() function may not be
     * needed after all
     */
#if 0
    FX_grDRIBufferSwap(fxMesa, fxMesa->swapInterval);
#elif 1
    grDRIBufferSwap(fxMesa->swapInterval);
#else
    grDRISwapClipRects(fxMesa->swapInterval,
                       driDrawPriv->numClipRects,
                       driDrawPriv->pClipRects);
#endif


#if 0
    {
        FxI32 result;
        do {
            result = FX_grGetInteger(FX_PENDING_BUFFERSWAPS);
        } while (result > fxMesa->maxPendingSwapBuffers);
    }
#endif
    fxMesa->stats.swapBuffer++;


    if (ctx) {
        if (ctx->DriverCtx != fxMesa) {
            /* restore original context */
           fxMesa = FX_CONTEXT(ctx);
           grSstSelect(fxMesa->board);
           grGlideSetState((GrState *) fxMesa->state);
        }
        END_BOARD_LOCK(fxMesa);
    }
}


GLboolean
XMesaUnbindContext(__DRIcontextPrivate * driContextPriv)
{
    GET_CURRENT_CONTEXT(ctx);
    if (driContextPriv && driContextPriv->mesaContext == ctx) {
        fxMesaContext fxMesa = FX_CONTEXT(ctx);
        FX_grGlideGetState(fxMesa, (GrState *) fxMesa->state);
    }
    return GL_TRUE;
}

GLboolean
XMesaOpenFullScreen(__DRIcontextPrivate *driContextPriv)
{
    fprintf(stderr,"XMesaOpenFullScreen\n");
#if 0 /* When new glide3 calls exist */
    return((GLboolean)grDRISetupFullScreen(GL_TRUE));
#else
    return GL_TRUE;
#endif
}

GLboolean
XMesaCloseFullScreen(__DRIcontextPrivate *driContextPriv)
{
    fprintf(stderr,"***** XMesaCloseFullScreen *****\n");
#if 0 /* When new glide3 calls exist */
    return((GLboolean)grDRISetupFullScreen(GL_FALSE));
#else
    return GL_TRUE;
#endif
}

/*
 * This function sends the window position and cliprect list to
 * Glide for the given context.
 */
static void
XMesaWindowMoved(fxMesaContext fxMesa)
{
    __DRIdrawablePrivate *dPriv = fxMesa->driContextPriv->driDrawablePriv;
    GLcontext *ctx = fxMesa->glCtx;

    grDRIPosition(dPriv->x, dPriv->y, dPriv->w, dPriv->h,
                  dPriv->numClipRects, dPriv->pClipRects);
    fxMesa->numClipRects = dPriv->numClipRects;
    fxMesa->pClipRects = dPriv->pClipRects;
    if (dPriv->x != fxMesa->x_offset || dPriv->y != fxMesa->y_offset ||
        dPriv->w != fxMesa->width || dPriv->h != fxMesa->height) {
        fxMesa->x_offset = dPriv->x;
        fxMesa->y_offset = dPriv->y;
        fxMesa->width = dPriv->w;
        fxMesa->height = dPriv->h;
        fxMesa->y_delta =
            fxMesa->screen_height - fxMesa->y_offset - fxMesa->height;
    }
    switch (dPriv->numClipRects) {
    case 0:
        fxMesa->clipMinX = dPriv->x;
        fxMesa->clipMaxX = dPriv->x + dPriv->w;
        fxMesa->clipMinY = dPriv->y;
        fxMesa->clipMaxY = dPriv->y + dPriv->h;
        fxSetScissorValues(ctx);
        fxMesa->needClip = 0;
        break;
    case 1:
        fxMesa->clipMinX = dPriv->pClipRects[0].x1;
        fxMesa->clipMaxX = dPriv->pClipRects[0].x2;
        fxMesa->clipMinY = dPriv->pClipRects[0].y1;
        fxMesa->clipMaxY = dPriv->pClipRects[0].y2;
        fxSetScissorValues(ctx);
        fxMesa->needClip = 0;
        break;
    default:
        fxMesa->needClip = 1;
    }
}


GLboolean
XMesaMakeCurrent(__DRIcontextPrivate * driContextPriv,
                 __DRIdrawablePrivate * driDrawPriv,
                 __DRIdrawablePrivate * driReadPriv)
{
    if (driContextPriv) {
        fxMesaContext fxMesa;

        fxMesa = (fxMesaContext) driContextPriv->driverPrivate;

        if (!fxMesa->initDone) {
            if (!tdfxInitHW(driDrawPriv, fxMesa))
                return GL_FALSE;
            fxMesa->width = 0;
            XMesaWindowMoved(fxMesa);
            FX_grGlideGetState(fxMesa, (GrState *) fxMesa->state);
        }
        else {
            FX_grSstSelect(fxMesa, fxMesa->board);
            FX_grGlideSetState(fxMesa, (GrState *) fxMesa->state);
            XMesaWindowMoved(fxMesa);
        }

        assert(fxMesa->glCtx == driContextPriv->mesaContext);

        gl_make_current2(fxMesa->glCtx, driDrawPriv->mesaBuffer,
                         driReadPriv->mesaBuffer);

        if (!fxMesa->glCtx->Viewport.Width)
            gl_Viewport(fxMesa->glCtx, 0, 0, driDrawPriv->w, driDrawPriv->h);
    }
    else {
        gl_make_current(0, 0);
    }
    return GL_TRUE;
}


/* This is called from within the LOCK_HARDWARE routine */
void
XMesaUpdateState(fxMesaContext fxMesa)
{
    __DRIcontextPrivate *cPriv = fxMesa->driContextPriv;
    __DRIdrawablePrivate *dPriv = cPriv->driDrawablePriv;
    __DRIscreenPrivate *sPriv = dPriv->driScreenPriv;
    TDFXSAREAPriv *saPriv = (TDFXSAREAPriv *) (((char *) sPriv->pSAREA) +
                                               sizeof(XF86DRISAREARec));
    int stamp;
    char ret;

    DEBUG_CHECK_LOCK();
    DRM_CAS(&sPriv->pSAREA->lock, dPriv->driContextPriv->hHWContext,
            DRM_LOCK_HELD | dPriv->driContextPriv->hHWContext, ret);
    if (!ret) {
        DEBUG_LOCK();
        return;
    }
    drmGetLock(sPriv->fd, dPriv->driContextPriv->hHWContext, 0);
    stamp = dPriv->lastStamp;
    /* This macro will update dPriv's cliprects if needed */
    XMESA_VALIDATE_DRAWABLE_INFO(cPriv->display, sPriv, dPriv);
    /* fprintf(stderr, "In FifoPtr=%d FifoRead=%d\n", saPriv->fifoPtr, saPriv->fifoRead); */
    if (saPriv->fifoOwner != dPriv->driContextPriv->hHWContext) {
        grDRIImportFifo(saPriv->fifoPtr, saPriv->fifoRead);
    }
    if (saPriv->ctxOwner != dPriv->driContextPriv->hHWContext) {
        /* This sequence looks a little odd. Glide mirrors the state, and
           when you get the state you are forcing the mirror to be up to
           date, and then getting a copy from the mirror. You can then force
           that state onto the hardware when you set the state. */
        void *state;
        state = malloc(FX_grGetInteger_NoLock(FX_GLIDE_STATE_SIZE));
        FX_grGlideGetState_NoLock(state);
        FX_grGlideSetState_NoLock(state);
        free(state);
    }
    if (saPriv->texOwner != dPriv->driContextPriv->hHWContext) {
        fxTMRestoreTextures_NoLock(fxMesa);
    }
#if 0
    if (*dPriv->pStamp != stamp)
#else
    if (*dPriv->pStamp != stamp ||
        saPriv->ctxOwner != dPriv->driContextPriv->hHWContext)
#endif
        XMesaWindowMoved(fxMesa);
    DEBUG_LOCK();
}


/*
 * XXX is this used by anyone?
 */
#if 000
static void
XMesaSetSAREA(void)
{
    __DRIdrawablePrivate *dPriv = gCC->driDrawablePriv;
    __DRIscreenPrivate *sPriv = dPriv->driScreenPriv;
    TDFXSAREAPriv *saPriv =
        (TDFXSAREAPriv *) (((char *) sPriv->pSAREA) +
                           sizeof(XF86DRISAREARec));

    saPriv->fifoOwner = dPriv->driContextPriv->hHWContext;
    saPriv->ctxOwner = dPriv->driContextPriv->hHWContext;
    saPriv->texOwner = dPriv->driContextPriv->hHWContext;
    grDRIResetSAREA();
    /* fprintf(stderr, "Out FifoPtr=%d FifoRead=%d\n", saPriv->fifoPtr, saPriv->fifoRead); */
}
#endif



extern void __driRegisterExtensions(void); /* silence compiler warning */

/* This function is called by libGL.so as soon as libGL.so is loaded.
 * This is where we'd register new extension functions with the dispatcher.
 */
void
__driRegisterExtensions(void)
{
#if 0
    /* Example.  Also look in fxdd.c for more details. */
    {
        const int _gloffset_FooBarEXT = 555; /* just an example number! */
        if (_glapi_add_entrypoint("glFooBarEXT", _gloffset_FooBarEXT)) {
            void *f = glXGetProcAddressARB("glFooBarEXT");
            assert(f);
        }
    }
#endif
}


#endif

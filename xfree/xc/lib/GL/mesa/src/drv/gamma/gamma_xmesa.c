/* $XFree86: xc/lib/GL/mesa/src/drv/gamma/gamma_xmesa.c,v 1.8 2000/12/07 20:26:05 dawes Exp $ */
/**************************************************************************

Copyright 1998-1999 Precision Insight, Inc., Cedar Park, Texas.
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

/*
 * Authors:
 *   Kevin E. Martin <kevin@precisioninsight.com>
 *   Brian Paul <brian@precisioninsight.com>
 *   Alan Hourihane <alanh@fairlite.demon.co.uk>
 */

#ifdef GLX_DIRECT_RENDERING

#include <X11/Xlibint.h>
#include "gamma_init.h"
#include "gamma_gl.h"
#include "glapi.h"
#include "glint_dri.h"
#include "context.h"
#include "mmath.h"


__DRIcontextPrivate *nullCC  = NULL;
__DRIcontextPrivate *gCC = NULL;
gammaContextPrivate *gCCPriv = NULL;

static struct _glapi_table *Dispatch = NULL;


GLboolean XMesaInitDriver(__DRIscreenPrivate *sPriv)
{
    gammaScreenPrivate *gsp;

    /* Check the DRI version */
    {
       int major, minor, patch;
       if (XF86DRIQueryVersion(sPriv->display, &major, &minor, &patch)) {
          if (major != 3 || minor != 0 || patch < 0) {
             char msg[1000];
             sprintf(msg, "gamma DRI driver expected DRI version 3.0.x but got version %d.%d.%d", major, minor, patch);
             __driMesaMessage(msg);
             return GL_FALSE;
          }
       }
    }

    /* Check that the DDX driver version is compatible */
    if (sPriv->ddxMajor != 1 ||
        sPriv->ddxMinor != 0 ||
        sPriv->ddxPatch < 0) {
        char msg[1000];
        sprintf(msg, "gamma DRI driver expected DDX driver version 1.0.x but got version %d.%d.%d", sPriv->ddxMajor, sPriv->ddxMinor, sPriv->ddxPatch);
        __driMesaMessage(msg);
        return GL_FALSE;
    }

    /* Check that the DRM driver version is compatible */
    if (sPriv->drmMajor != 1 ||
        sPriv->drmMinor != 0 ||
        sPriv->drmPatch < 0) {
        char msg[1000];
        sprintf(msg, "gamm DRI driver expected DRM driver version 1.0.x but got version %d.%d.%d", sPriv->drmMajor, sPriv->drmMinor, sPriv->drmPatch);
        __driMesaMessage(msg);
        return GL_FALSE;
    }

    /* Allocate the private area */
    gsp = (gammaScreenPrivate *)Xmalloc(sizeof(gammaScreenPrivate));
    if (!gsp) {
	return GL_FALSE;
    }
    gsp->driScrnPriv = sPriv;

    sPriv->private = (void *)gsp;

    if (!gammaMapAllRegions(sPriv)) {
	Xfree(sPriv->private);
	return GL_FALSE;
    }

    return GL_TRUE;
}

void XMesaResetDriver(__DRIscreenPrivate *sPriv)
{
    gammaUnmapAllRegions(sPriv);
    Xfree(sPriv->private);
    sPriv->private = NULL;
}

GLvisual *XMesaCreateVisual(Display *dpy,
                            __DRIscreenPrivate *driScrnPriv,
                            const XVisualInfo *visinfo,
                            const __GLXvisualConfig *config)
{
   /* Drivers may change the args to _mesa_create_visual() in order to
    * setup special visuals.
    */
   return _mesa_create_visual( config->rgba,
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
                               0 /* num samples */ );
}


GLboolean XMesaCreateContext( Display *dpy,
                              GLvisual *mesaVis,
                              __DRIcontextPrivate *driContextPriv )
{
    int i;
    gammaContextPrivate *cPriv;
    __DRIscreenPrivate *driScrnPriv = driContextPriv->driScreenPriv;
    gammaScreenPrivate *gPriv = (gammaScreenPrivate *)driScrnPriv->private;
    GLINTDRIPtr         gDRIPriv = (GLINTDRIPtr)driScrnPriv->pDevPriv;

    if (!Dispatch) {
       GLuint size = _glapi_get_dispatch_table_size() * sizeof(GLvoid *);
       Dispatch = (struct _glapi_table *) malloc(size);
       _gamma_init_dispatch(Dispatch);
    }

    cPriv = (gammaContextPrivate *)Xmalloc(sizeof(gammaContextPrivate));
    if (!cPriv) {
	return GL_FALSE;
    }

    cPriv->hHWContext = driContextPriv->hHWContext;
    GET_FIRST_DMA(driScrnPriv->fd, cPriv->hHWContext,
		  1, &cPriv->bufIndex, &cPriv->bufSize,
		  &cPriv->buf, &cPriv->bufCount, gPriv);

#ifdef DO_VALIDATE
    GET_FIRST_DMA(driScrnPriv->fd, cPriv->hHWContext,
		  1, &cPriv->WCbufIndex, &cPriv->WCbufSize,
		  &cPriv->WCbuf, &cPriv->WCbufCount, gPriv);
#endif

    cPriv->ClearColor[0] = 0.0;
    cPriv->ClearColor[1] = 0.0;
    cPriv->ClearColor[2] = 0.0;
    cPriv->ClearColor[3] = 1.0;
    cPriv->ClearDepth = 1.0;
    cPriv->x = 0;
    cPriv->y = 0;
    cPriv->w = 0;
    cPriv->h = 0;
    cPriv->FrameCount = 0;
    cPriv->MatrixMode = GL_MODELVIEW;
    cPriv->ModelViewCount = 0;
    cPriv->ProjCount = 0;
    cPriv->TextureCount = 0;

    for (i = 0; i < 16; i++)
	if (i % 5 == 0)
	    cPriv->ModelView[i] =
		cPriv->Proj[i] =
		cPriv->ModelViewProj[i] =
		cPriv->Texture[i] = 1.0;
	else
	    cPriv->ModelView[i] =
		cPriv->Proj[i] =
		cPriv->ModelViewProj[i] =
		cPriv->Texture[i] = 0.0;

    cPriv->LBReadMode = (LBReadSrcDisable |
			 LBReadDstDisable |
			 LBDataTypeDefault |
			 LBWindowOriginBot |
			 gDRIPriv->pprod);
    cPriv->FBReadMode = (FBReadSrcDisable |
			 FBReadDstDisable |
			 FBDataTypeDefault |
			 FBWindowOriginBot |
			 gDRIPriv->pprod);
 
    if (gDRIPriv->numMXDevices == 2) {
	cPriv->LBReadMode |= LBScanLineInt2;
	cPriv->FBReadMode |= FBScanLineInt2;
    	cPriv->FBWindowBase =driScrnPriv->fbWidth*(driScrnPriv->fbHeight/2 - 1);
    	cPriv->LBWindowBase =driScrnPriv->fbWidth*(driScrnPriv->fbHeight/2 - 1);
    } else {
    	cPriv->FBWindowBase = driScrnPriv->fbWidth * driScrnPriv->fbHeight;
    	cPriv->LBWindowBase = driScrnPriv->fbWidth * driScrnPriv->fbHeight;
    }

    cPriv->Begin = (B_AreaStippleDisable |
		    B_LineStippleDisable |
		    B_AntiAliasDisable |
		    B_TextureDisable |
		    B_FogDisable |
		    B_SubPixelCorrectEnable |
		    B_PrimType_Null);

    cPriv->ColorDDAMode = (ColorDDAEnable |
			   ColorDDAGouraud);

#ifdef CULL_ALL_PRIMS
    cPriv->GeometryMode = (GM_TextureDisable |
			   GM_FogDisable |
			   GM_FogExp |
			   GM_FrontPolyFill |
			   GM_BackPolyFill |
			   GM_FrontFaceCCW |
			   GM_PolyCullDisable |
			   GM_PolyCullBoth |
			   GM_ClipShortLinesDisable |
			   GM_ClipSmallTrisDisable |
			   GM_RenderMode |
			   GM_Feedback2D |
			   GM_CullFaceNormDisable |
			   GM_AutoFaceNormDisable |
			   GM_GouraudShading |
			   GM_UserClipNone |
			   GM_PolyOffsetPointDisable |
			   GM_PolyOffsetLineDisable |
			   GM_PolyOffsetFillDisable |
			   GM_InvertFaceNormCullDisable);
#else
    cPriv->GeometryMode = (GM_TextureDisable |
			   GM_FogDisable |
			   GM_FogExp |
			   GM_FrontPolyFill |
			   GM_BackPolyFill |
			   GM_FrontFaceCCW |
			   GM_PolyCullDisable |
			   GM_PolyCullBack |
			   GM_ClipShortLinesDisable |
			   GM_ClipSmallTrisDisable |
			   GM_RenderMode |
			   GM_Feedback2D |
			   GM_CullFaceNormDisable |
			   GM_AutoFaceNormDisable |
			   GM_GouraudShading |
			   GM_UserClipNone |
			   GM_PolyOffsetPointDisable |
			   GM_PolyOffsetLineDisable |
			   GM_PolyOffsetFillDisable |
			   GM_InvertFaceNormCullDisable);
#endif

    cPriv->AlphaTestMode = (AlphaTestModeDisable |
			    AT_Always);

    cPriv->AlphaBlendMode = (AlphaBlendModeDisable |
			     AB_Src_One |
			     AB_Dst_Zero |
			     AB_ColorFmt_8888 |
			     AB_NoAlphaBufferPresent |
			     AB_ColorOrder_RGB |
			     AB_OpenGLType |
			     AB_AlphaDst_FBData |
			     AB_ColorConversionScale |
			     AB_AlphaConversionScale);

    cPriv->AB_FBReadMode_Save = cPriv->AB_FBReadMode = 0;

    cPriv->Window = (WindowEnable  | /* For GID testing */
		     W_PassIfEqual |
		     (0 << 5)); /* GID part is set from draw priv (below) */

    cPriv->NotClipped = GL_FALSE;
    cPriv->WindowChanged = GL_TRUE;

    /*
    ** NOT_DONE:
    ** 1. These values should be calculated from the registers.
    ** 2. Only one client can use texture memory at this time.
    ** 3. A two-tiered texture allocation routine is needed to properly
    **    handle texture management.
    */
    cPriv->tmm = driTMMCreate(0x00080000,
			      0x00800000 - 0x00080000,
			      4, 1,
			      gammaTOLoad,
			      gammaTOLoadSub);

    cPriv->curTexObj = gammaTOFind(0);
    cPriv->curTexObj1D = cPriv->curTexObj;
    cPriv->curTexObj2D = cPriv->curTexObj;
    cPriv->Texture1DEnabled = GL_FALSE;
    cPriv->Texture2DEnabled = GL_FALSE;

#ifdef FORCE_DEPTH32
    cPriv->DepthSize = 32;
#else
    cPriv->DepthSize = mesaVis->DepthBits;
#endif
    cPriv->zNear = 0.0;
    cPriv->zFar  = 1.0;

    cPriv->Flags  = GAMMA_FRONT_BUFFER;
    cPriv->Flags |= (mesaVis->DBflag ? GAMMA_BACK_BUFFER  : 0);
    cPriv->Flags |= (cPriv->DepthSize > 0 ? GAMMA_DEPTH_BUFFER : 0);

    cPriv->EnabledFlags = GAMMA_FRONT_BUFFER;
    cPriv->EnabledFlags |= (mesaVis->DBflag ? GAMMA_BACK_BUFFER  : 0);

    cPriv->DepthMode = (DepthModeDisable |
			DM_WriteMask |
			DM_Less);

    cPriv->DeltaMode = (DM_SubPixlCorrectionEnable |
			DM_SmoothShadingEnable |
			DM_Target500TXMX);

    switch (cPriv->DepthSize) {
    case 16:
	cPriv->DeltaMode |= DM_Depth16;
	break;
    case 24:
	cPriv->DeltaMode |= DM_Depth24;
	break;
    case 32:
	cPriv->DeltaMode |= DM_Depth32;
	break;
    default:
	break;
    }

    cPriv->gammaScrnPriv = gPriv;

    cPriv->LightingMode = LightingModeDisable;
    cPriv->Light0Mode = LNM_Off;
    cPriv->Light1Mode = LNM_Off;

    cPriv->MaterialMode = MaterialModeDisable;

    cPriv->ScissorMode = UserScissorDisable | ScreenScissorDisable;

    driContextPriv->driverPrivate = cPriv;

    /* Initialize the HW to a known state */
    gammaInitHW(cPriv);

    return GL_TRUE;
}

void XMesaDestroyContext(__DRIcontextPrivate *driContextPriv)
{
    gammaContextPrivate *cPriv;
    cPriv = (gammaContextPrivate *) driContextPriv->driverPrivate;
    if (cPriv) {
       /* XXX free driver context data? */
    }
}


GLframebuffer *XMesaCreateWindowBuffer( Display *dpy,
                                        __DRIscreenPrivate *driScrnPriv,
                                        __DRIdrawablePrivate *driDrawPriv,
                                        GLvisual *mesaVis)
{
   return gl_create_framebuffer(mesaVis,
                                GL_FALSE,  /* software depth buffer? */
                                mesaVis->StencilBits > 0,
                                mesaVis->AccumRedBits > 0,
                                mesaVis->AlphaBits > 0
                                );
}


GLframebuffer *XMesaCreatePixmapBuffer( Display *dpy,
                                        __DRIscreenPrivate *driScrnPriv,
                                        __DRIdrawablePrivate *driDrawPriv,
                                        GLvisual *mesaVis)
{
#if 0
   /* Different drivers may have different combinations of hardware and
    * software ancillary buffers.
    */
   return gl_create_framebuffer(mesaVis,
                                GL_FALSE,  /* software depth buffer? */
                                mesaVis->StencilBits > 0,
                                mesaVis->AccumRedBits > 0,
                                mesaVis->AlphaBits > 0
                                );
#else
   return NULL;  /* not implemented yet */
#endif
}


void XMesaSwapBuffers(__DRIdrawablePrivate *driDrawPriv)
{
    /*
    ** NOT_DONE: This assumes buffer is currently bound to a context.
    ** This needs to be able to swap buffers when not currently bound.
    */
    if (gCC == NULL || gCCPriv == NULL)
	return;

    VALIDATE_DRAWABLE_INFO(gCC,gCCPriv);

    /* Flush any partially filled buffers */
    FLUSH_DMA_BUFFER(gCC, gCCPriv);

    if (gCCPriv->EnabledFlags & GAMMA_BACK_BUFFER) {
	int src, dst, x0, y0, x1, h;
	int i;
	int nRect = driDrawPriv->numClipRects;
	XF86DRIClipRectPtr pRect = driDrawPriv->pClipRects;
	__DRIscreenPrivate *driScrnPriv = gCC->driScreenPriv;

#ifdef DO_VALIDATE
	DRM_SPINLOCK(&driScrnPriv->pSAREA->drawable_lock,
		     driScrnPriv->drawLockID);
	VALIDATE_DRAWABLE_INFO_NO_LOCK(gCC,gCCPriv);
#endif

	CHECK_DMA_BUFFER(nullCC, gCCPriv, 2);
	WRITE(gCCPriv->buf, FBReadMode, (gCCPriv->FBReadMode |
					 FBReadSrcEnable));
	WRITE(gCCPriv->buf, LBWriteMode, LBWriteModeDisable);

	for (i = 0; i < nRect; i++, pRect++) {
	    x0 = pRect->x1;
	    x1 = pRect->x2;
	    h  = pRect->y2 - pRect->y1;
	    y0 = driScrnPriv->fbHeight - (pRect->y1+h);

	    src = (y0/2)*driScrnPriv->fbWidth+x0;
	    y0 += driScrnPriv->fbHeight;
	    dst = (y0/2)*driScrnPriv->fbWidth+x0;

	    CHECK_DMA_BUFFER(nullCC, gCCPriv, 9);
	    WRITE(gCCPriv->buf, StartXDom,       x0<<16);   /* X0dest */
	    WRITE(gCCPriv->buf, StartY,          y0<<16);   /* Y0dest */
	    WRITE(gCCPriv->buf, StartXSub,       x1<<16);   /* X1dest */
	    WRITE(gCCPriv->buf, GLINTCount,      h);        /* H */
	    WRITE(gCCPriv->buf, dY,              1<<16);    /* ydir */
	    WRITE(gCCPriv->buf, dXDom,           0<<16);
	    WRITE(gCCPriv->buf, dXSub,           0<<16);
	    WRITE(gCCPriv->buf, FBSourceOffset, (dst-src));
	    WRITE(gCCPriv->buf, Render,          0x00040048); /* NOT_DONE */
	}

	/*
	** NOTE: FBSourceOffset (above) is backwards from what is
	** described in the manual (i.e., dst-src instead of src-dst)
	** due to our using the bottom-left window origin instead of the
	** top-left window origin.
	*/

	/* Restore FBReadMode */
	CHECK_DMA_BUFFER(nullCC, gCCPriv, 2);
	WRITE(gCCPriv->buf, FBReadMode, (gCCPriv->FBReadMode |
					 gCCPriv->AB_FBReadMode));
	WRITE(gCCPriv->buf, LBWriteMode, LBWriteModeEnable);

#ifdef DO_VALIDATE
	PROCESS_DMA_BUFFER_TOP_HALF(gCCPriv);

	DRM_SPINUNLOCK(&driScrnPriv->pSAREA->drawable_lock,
		       driScrnPriv->drawLockID);
	VALIDATE_DRAWABLE_INFO_NO_LOCK_POST(gCC,gCCPriv);

	PROCESS_DMA_BUFFER_BOTTOM_HALF(gCCPriv);
#else
	FLUSH_DMA_BUFFER(gCC,gCCPriv);
#endif

    }
}

GLboolean XMesaMakeCurrent(__DRIcontextPrivate *driContextPriv,
                           __DRIdrawablePrivate *driDrawPriv,
                           __DRIdrawablePrivate *driReadPriv)
{
    if (driContextPriv) {
	gCC     = driContextPriv;
	gCCPriv = (gammaContextPrivate *) driContextPriv->driverPrivate;

	gCCPriv->Window &= ~W_GIDMask;
	gCCPriv->Window |= (driDrawPriv->index << 5);

	CHECK_DMA_BUFFER(gCC, gCCPriv, 1);
	WRITE(gCCPriv->buf, GLINTWindow, gCCPriv->Window);

        _glapi_set_dispatch(Dispatch);

	_gamma_Viewport(0, 0, driDrawPriv->w, driDrawPriv->h);
    } else {
	gCC     = NULL;
	gCCPriv = NULL;
    }
    return GL_TRUE;
}


GLboolean XMesaUnbindContext( __DRIcontextPrivate *driContextPriv )
{
   /* XXX not 100% sure what's supposed to be done here */
   return GL_TRUE;
}

GLboolean
XMesaOpenFullScreen(__DRIcontextPrivate *driContextPriv)
{
    return GL_TRUE;
}

GLboolean
XMesaCloseFullScreen(__DRIcontextPrivate *driContextPriv)
{
    return GL_TRUE;
}


void __driRegisterExtensions(void)
{
   /* No extensions */
}


#endif

/* $XFree86: xc/lib/GL/mesa/src/drv/ffb/ffb_xmesa.c,v 1.3 2001/05/29 22:24:01 dawes Exp $
 *
 * GLX Hardware Device Driver for Sun Creator/Creator3D
 * Copyright (C) 2000 David S. Miller
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * DAVID MILLER, OR ANY OTHER CONTRIBUTORS BE LIABLE FOR ANY CLAIM, 
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR 
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE 
 * OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 *
 *    David S. Miller <davem@redhat.com>
 */

#ifdef GLX_DIRECT_RENDERING

#include <X11/Xlibint.h>
#include <stdio.h>

#include "ffb_xmesa.h"
#include "context.h"
#include "vbxform.h"
#include "matrix.h"
#include "simple_list.h"
#include "mmath.h"

#include "xf86dri.h"

#include "ffb_context.h"
#include "ffb_dd.h"
#include "ffb_span.h"
#include "ffb_depth.h"
#include "ffb_stencil.h"
#include "ffb_clear.h"
#include "ffb_vb.h"
#include "ffb_tris.h"
#include "ffb_lines.h"
#include "ffb_points.h"
#include "ffb_state.h"
#include "ffb_pipeline.h"
#include "ffb_lock.h"

#if 0
#include "xmesaP.h"
#endif

static ffbContextPtr ffbCtx = NULL;

/* These functions are accessed by dlsym from dri_mesa_init.c:
 *
 * XMesaInitDriver
 * XMesaResetDriver
 * XMesaCreateVisual
 * XMesaDestroyVisual
 * XMesaCreateContext 
 * XMesaDestroyContext
 * XMesaCreateWindowBuffer
 * XMesaCreatePixmapBuffer
 * XMesaDestroyBuffer
 * XMesaSwapBuffers
 * XMesaMakeCurrent
 *
 * So this is kind of the public interface to the driver.  The driver
 * uses the X11 mesa driver context as a kind of wrapper around its
 * own driver context - but there isn't much justificiation for doing
 * it that way - the DRI might as well use a (void *) to refer to the
 * driver contexts.  Nothing in the X context really gets used.
 */

GLboolean XMesaInitDriver(__DRIscreenPrivate *sPriv)
{
	ffbScreenPrivate *ffbScreen;
	FFBDRIPtr gDRIPriv = (FFBDRIPtr) sPriv->pDevPriv;

	/* Allocate the private area. */
	ffbScreen = (ffbScreenPrivate *) Xmalloc(sizeof(ffbScreenPrivate));
	if (!ffbScreen)
		return GL_FALSE;

	/* Map FBC registers. */
	if (drmMap(sPriv->fd,
		   gDRIPriv->hFbcRegs,
		   gDRIPriv->sFbcRegs,
		   &gDRIPriv->mFbcRegs)) {
		Xfree(ffbScreen);
		return GL_FALSE;
	}
	ffbScreen->regs = (ffb_fbcPtr) gDRIPriv->mFbcRegs;

	/* Map ramdac registers. */
	if (drmMap(sPriv->fd,
		   gDRIPriv->hDacRegs,
		   gDRIPriv->sDacRegs,
		   &gDRIPriv->mDacRegs)) {
		drmUnmap(gDRIPriv->mFbcRegs, gDRIPriv->sFbcRegs);
		Xfree(ffbScreen);
		return GL_FALSE;
	}
	ffbScreen->dac = (ffb_dacPtr) gDRIPriv->mDacRegs;

	/* Map "Smart" framebuffer views. */
	if (drmMap(sPriv->fd,
		   gDRIPriv->hSfb8r,
		   gDRIPriv->sSfb8r,
		   &gDRIPriv->mSfb8r)) {
		drmUnmap(gDRIPriv->mFbcRegs, gDRIPriv->sFbcRegs);
		drmUnmap(gDRIPriv->mDacRegs, gDRIPriv->sDacRegs);
		Xfree(ffbScreen);
		return GL_FALSE;
	}
	ffbScreen->sfb8r = (volatile char *) gDRIPriv->mSfb8r;

	if (drmMap(sPriv->fd,
		   gDRIPriv->hSfb32,
		   gDRIPriv->sSfb32,
		   &gDRIPriv->mSfb32)) {
		drmUnmap(gDRIPriv->mFbcRegs, gDRIPriv->sFbcRegs);
		drmUnmap(gDRIPriv->mDacRegs, gDRIPriv->sDacRegs);
		drmUnmap(gDRIPriv->mSfb8r, gDRIPriv->sSfb8r);
		Xfree(ffbScreen);
		return GL_FALSE;
	}
	ffbScreen->sfb32 = (volatile char *) gDRIPriv->mSfb32;

	if (drmMap(sPriv->fd,
		   gDRIPriv->hSfb64,
		   gDRIPriv->sSfb64,
		   &gDRIPriv->mSfb64)) {
		drmUnmap(gDRIPriv->mFbcRegs, gDRIPriv->sFbcRegs);
		drmUnmap(gDRIPriv->mDacRegs, gDRIPriv->sDacRegs);
		drmUnmap(gDRIPriv->mSfb8r, gDRIPriv->sSfb8r);
		drmUnmap(gDRIPriv->mSfb32, gDRIPriv->sSfb32);
		Xfree(ffbScreen);
		return GL_FALSE;
	}
	ffbScreen->sfb64 = (volatile char *) gDRIPriv->mSfb64;

	ffbScreen->fifo_cache = 0;
	ffbScreen->rp_active = 0;

	ffbScreen->sPriv = sPriv;
	sPriv->private = (void *) ffbScreen;

	ffbDDSetupInit();
	ffbDDTrifuncInit();
	ffbDDLinefuncInit();
	ffbDDPointfuncInit();

	return GL_TRUE;
}

/* Accessed by dlsym from dri_mesa_init.c */
void XMesaResetDriver(__DRIscreenPrivate *sPriv)
{
	ffbScreenPrivate *ffbScreen = sPriv->private;
	FFBDRIPtr gDRIPriv = (FFBDRIPtr) sPriv->pDevPriv;

	drmUnmap(gDRIPriv->mFbcRegs, gDRIPriv->sFbcRegs);
	drmUnmap(gDRIPriv->mDacRegs, gDRIPriv->sDacRegs);
	drmUnmap(gDRIPriv->mSfb8r, gDRIPriv->sSfb8r);
	drmUnmap(gDRIPriv->mSfb32, gDRIPriv->sSfb32);
	drmUnmap(gDRIPriv->mSfb64, gDRIPriv->sSfb64);

	Xfree(ffbScreen);
}

/* Accessed by dlsym from dri_mesa_init.c */
GLvisual *XMesaCreateVisual(Display *dpy,
			    __DRIscreenPrivate *driScrnPriv,
			    const XVisualInfo *visinfo,
			    const __GLXvisualConfig *config)
{
	/* Only RGB visuals (for now) in this FFB driver. */
	if (!config->rgba)
		return NULL;

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
				   0 /* num samples */);
}

/* Create and initialize the Mesa and driver specific context data */
GLboolean XMesaCreateContext(Display *dpy, GLvisual *mesaVis,
			     __DRIcontextPrivate *driContextPriv)
{
	ffbContextPtr fmesa;
	GLcontext *glCtx;
	__DRIscreenPrivate *sPriv;
	ffbScreenPrivate *ffbScreen;

	fmesa = (ffbContextPtr) Xmalloc(sizeof(ffbContextRec));
	if (!fmesa)
		return GL_FALSE;

	glCtx = driContextPriv->mesaContext;
	sPriv = driContextPriv->driScreenPriv;
	ffbScreen = (ffbScreenPrivate *) sPriv->private;

	fmesa->glCtx = glCtx;
	glCtx->DriverCtx = (void *) fmesa;

	/* Dri stuff. */
	fmesa->display = dpy;
	fmesa->hHWContext = driContextPriv->hHWContext;
	fmesa->driFd = sPriv->fd;
	fmesa->driHwLock = &sPriv->pSAREA->lock;

	fmesa->ffbScreen = ffbScreen;
	fmesa->driScreen = sPriv;
	fmesa->ffb_sarea = FFB_DRISHARE(sPriv->pSAREA);

	/* Register and framebuffer hw pointers. */
	fmesa->regs = ffbScreen->regs;
	fmesa->sfb32 = ffbScreen->sfb32;

	fmesa->SWrender = 0;
	if (getenv("LIBGL_SOFTWARE_RENDERING"))
		fmesa->SWrender = 1;

	ffbDDInitContextHwState(glCtx);

	/* Default clear and depth colors. */
	{
		GLubyte r = (GLint) (glCtx->Color.ClearColor[0] * 255.0F);
		GLubyte g = (GLint) (glCtx->Color.ClearColor[1] * 255.0F);
		GLubyte b = (GLint) (glCtx->Color.ClearColor[2] * 255.0F);

		fmesa->clear_pixel = ((r << 0) |
				      (g << 8) |
				      (b << 16));
	}
	fmesa->clear_depth = Z_FROM_MESA(glCtx->Depth.Clear * 4294967295.0f);
	fmesa->clear_stencil = glCtx->Stencil.Clear & 0xf;

	/* All of this need only be done once for a new context. */
	ffbDDExtensionsInit(glCtx);
	ffbDDInitDriverFuncs(glCtx);
	ffbDDInitStateFuncs(glCtx);
	ffbDDInitSpanFuncs(glCtx);
	ffbDDInitDepthFuncs(glCtx);
	ffbDDInitStencilFuncs(glCtx);

	/* Actually we do the culling in software.  The problem is
	 * that there is apparently some bug in generic MESA if you
	 * provide a driver triangle rendering function when culling
	 * is enabled.
	 *
	 * Generic MESA in such a case does the culling in it's very
	 * own software triangle rasterizers (ie. when the driver indicates
	 * it cannot do it).  Perhaps this is a clue.
	 */
	glCtx->Driver.TriangleCaps = DD_TRI_CULL;

	if (glCtx->VB)
		ffbDDRegisterVB(glCtx->VB);

	if (glCtx->NrPipelineStages)
		glCtx->NrPipelineStages =
			ffbDDRegisterPipelineStages(glCtx->PipelineStage,
						    glCtx->PipelineStage,
						    glCtx->NrPipelineStages);

	driContextPriv->driverPrivate = (void *) fmesa;

	return GL_TRUE;
}

void XMesaDestroyContext(__DRIcontextPrivate *driContextPriv)
{
	ffbContextPtr fmesa = (ffbContextPtr) driContextPriv->driverPrivate;

	if (fmesa == ffbCtx)
		ffbCtx = NULL;

	if (fmesa)
		Xfree(fmesa);
}

/* Create and initialize the Mesa and driver specific pixmap buffer data */
GLframebuffer *XMesaCreateWindowBuffer(Display *dpy,
				       __DRIscreenPrivate *driScrnPriv,
				       __DRIdrawablePrivate *driDrawPriv,
				       GLvisual *mesaVis)
{
	return gl_create_framebuffer(mesaVis,
				     GL_FALSE,  /* software depth buffer? */
				     mesaVis->StencilBits > 0,
				     mesaVis->AccumRedBits > 0,
				     mesaVis->AlphaBits > 0);
}

/* Create and initialize the Mesa and driver specific pixmap buffer data */
GLframebuffer *XMesaCreatePixmapBuffer( Display *dpy,
                                        __DRIscreenPrivate *driScrnPriv,
                                        __DRIdrawablePrivate *driDrawPriv,
                                        GLvisual *mesaVis)
{
	return NULL;  /* not implemented yet */
}

#define USE_FAST_SWAP

void XMesaSwapBuffers(__DRIdrawablePrivate *driDrawPriv)
{
	unsigned int fbc, wid, wid_reg_val, dac_db_bit;
	unsigned int shadow_dac_addr, active_dac_addr;
	ffb_fbcPtr ffb;
	ffb_dacPtr dac;

	if (ffbCtx == NULL ||
	    ffbCtx->glCtx->Visual->DBflag == 0)
		return;

	FLUSH_VB(ffbCtx->glCtx, "swap buffers");

	ffb = ffbCtx->regs;
	dac = ffbCtx->ffbScreen->dac;

	fbc = ffbCtx->fbc;
	wid = ffbCtx->wid;

	/* Swap the buffer we render into and read pixels from. */
	ffbCtx->back_buffer ^= 1;

	/* If we are writing into both buffers, don't mess with
	 * the WB setting.
	 */
	if ((fbc & FFB_FBC_WB_AB) != FFB_FBC_WB_AB) {
		if ((fbc & FFB_FBC_WB_A) != 0)
			fbc = (fbc & ~FFB_FBC_WB_A) | FFB_FBC_WB_B;
		else
			fbc = (fbc & ~FFB_FBC_WB_B) | FFB_FBC_WB_A;
	}

	/* But either way, we must flip the read buffer setting. */
	if ((fbc & FFB_FBC_RB_A) != 0)
		fbc = (fbc & ~FFB_FBC_RB_A) | FFB_FBC_RB_B;
	else
		fbc = (fbc & ~FFB_FBC_RB_B) | FFB_FBC_RB_A;

	LOCK_HARDWARE(ffbCtx);

	if (ffbCtx->fbc != fbc) {
		FFBFifo(ffbCtx, 1);
		ffb->fbc = ffbCtx->fbc = fbc;
		ffbCtx->ffbScreen->rp_active = 1;
	}

	/* And swap the buffer displayed in the WID. */
	if (ffbCtx->ffb_sarea->flags & FFB_DRI_PAC1) {
		shadow_dac_addr = FFBDAC_PAC1_SPWLUT(wid);
		active_dac_addr = FFBDAC_PAC1_APWLUT(wid);
		dac_db_bit = FFBDAC_PAC1_WLUT_DB;
	} else {
		shadow_dac_addr = FFBDAC_PAC2_SPWLUT(wid);
		active_dac_addr = FFBDAC_PAC2_APWLUT(wid);
		dac_db_bit = FFBDAC_PAC2_WLUT_DB;
	}

	FFBWait(ffbCtx, ffb);

	wid_reg_val = DACCFG_READ(dac, active_dac_addr);
	if (ffbCtx->back_buffer == 0)
		wid_reg_val |=  dac_db_bit;
	else
		wid_reg_val &= ~dac_db_bit;
#ifdef USE_FAST_SWAP
	DACCFG_WRITE(dac, active_dac_addr, wid_reg_val);
#else
	DACCFG_WRITE(dac, shadow_dac_addr, wid_reg_val);

	/* Schedule the window transfer. */
	DACCFG_WRITE(dac, FFBDAC_CFG_WTCTRL, 
		     (FFBDAC_CFG_WTCTRL_TCMD | FFBDAC_CFG_WTCTRL_TE));

	{
		int limit = 1000000;
		while (limit--) {
			unsigned int wtctrl = DACCFG_READ(dac, FFBDAC_CFG_WTCTRL);

			if ((wtctrl & FFBDAC_CFG_WTCTRL_DS) == 0)
				break;
		}
	}
#endif

	UNLOCK_HARDWARE(ffbCtx);
}

static void ffb_init_wid(unsigned int wid)
{
	ffb_dacPtr dac = ffbCtx->ffbScreen->dac;
	unsigned int wid_reg_val, dac_db_bit, active_dac_addr;
	unsigned int shadow_dac_addr;

	if (ffbCtx->ffb_sarea->flags & FFB_DRI_PAC1) {
		shadow_dac_addr = FFBDAC_PAC1_SPWLUT(wid);
		active_dac_addr = FFBDAC_PAC1_APWLUT(wid);
		dac_db_bit = FFBDAC_PAC1_WLUT_DB;
	} else {
		shadow_dac_addr = FFBDAC_PAC2_SPWLUT(wid);
		active_dac_addr = FFBDAC_PAC2_APWLUT(wid);
		dac_db_bit = FFBDAC_PAC2_WLUT_DB;
	}

	wid_reg_val = DACCFG_READ(dac, active_dac_addr);
	wid_reg_val &= ~dac_db_bit;
#ifdef USE_FAST_SWAP
	DACCFG_WRITE(dac, active_dac_addr, wid_reg_val);
#else
	DACCFG_WRITE(dac, shadow_dac_addr, wid_reg_val);

	/* Schedule the window transfer. */
	DACCFG_WRITE(dac, FFBDAC_CFG_WTCTRL, 
		     (FFBDAC_CFG_WTCTRL_TCMD | FFBDAC_CFG_WTCTRL_TE));

	{
		int limit = 1000000;
		while (limit--) {
			unsigned int wtctrl = DACCFG_READ(dac, FFBDAC_CFG_WTCTRL);

			if ((wtctrl & FFBDAC_CFG_WTCTRL_DS) == 0)
				break;
		}
	}
#endif
}

/* Force the context `c' to be the current context and associate with it
   buffer `b' */
GLboolean XMesaMakeCurrent(__DRIcontextPrivate *driContextPriv,
			   __DRIdrawablePrivate *driDrawPriv,
			   __DRIdrawablePrivate *driReadPriv)
{
	if (driContextPriv) {
		ffbContextPtr fmesa = (ffbContextPtr) driContextPriv->driverPrivate;
		int first_time;

		if (ffbCtx != NULL &&
		    fmesa == ffbCtx &&
		    driDrawPriv == fmesa->driDrawable)
			return GL_TRUE;

		ffbCtx = fmesa;
		fmesa->driDrawable = driDrawPriv;

		gl_make_current2(ffbCtx->glCtx, 
				 driDrawPriv->mesaBuffer, driReadPriv->mesaBuffer);

		if (!ffbCtx->glCtx->Viewport.Width)
			gl_Viewport(ffbCtx->glCtx,
				    0, 0,
				    driDrawPriv->w, driDrawPriv->h);

		first_time = 0;
		if (ffbCtx->wid == ~0)
			first_time = 1;

		LOCK_HARDWARE(ffbCtx);
		if (first_time) {
			ffbCtx->wid = ffbCtx->ffb_sarea->wid_table[driDrawPriv->index];
			ffbCtx->state_dirty |= FFB_STATE_WID;
			ffb_init_wid(ffbCtx->wid);
		}

		ffbCtx->state_dirty |= FFB_STATE_ALL;
		ffbCtx->state_fifo_ents = ffbCtx->state_all_fifo_ents;
		ffbSyncHardware(ffbCtx);
		UNLOCK_HARDWARE(ffbCtx);

		if (first_time) {
			/* Also, at the first switch to a new context,
			 * we need to clear all the hw buffers.
			 */
			ffbDDClear(ffbCtx->glCtx,
				   (DD_FRONT_LEFT_BIT | DD_BACK_LEFT_BIT |
				    DD_DEPTH_BIT | DD_STENCIL_BIT),
				   1, 0, 0, 0, 0);
		}
	} else {
		gl_make_current(0,0);
		ffbCtx = NULL;
	}

	return GL_TRUE;
}

/* Force the context `c' to be unbound from its buffer */
GLboolean XMesaUnbindContext(__DRIcontextPrivate *driContextPriv)
{
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

void ffbXMesaUpdateState(ffbContextPtr fmesa)
{
	__DRIdrawablePrivate *dPriv = fmesa->driDrawable;
	__DRIscreenPrivate *sPriv = fmesa->driScreen;
	int stamp = dPriv->lastStamp;

	XMESA_VALIDATE_DRAWABLE_INFO(fmesa->display, sPriv, dPriv);

	if (dPriv->lastStamp != stamp) {
		GLcontext *ctx = fmesa->glCtx;

		if (ctx->Scissor.Enabled)
			ffbDDScissor(ctx, ctx->Scissor.X, ctx->Scissor.Y,
				     ctx->Scissor.Width, ctx->Scissor.Height);
		if (ctx->Polygon.StippleFlag)
			ffbXformAreaPattern(fmesa,
					    (const GLubyte *)ctx->PolygonStipple);
	}
}

/* This function is called by libGL.so as soon as libGL.so is loaded.
 * This is where we'd register new extension functions with the dispatcher.
 */
void __driRegisterExtensions(void)
{
}

#endif /* GLX_DIRECT_RENDERING */

/* -*- mode: c; c-basic-offset: 3 -*-
 *
 * Copyright 2000 VA Linux Systems Inc., Fremont, California.
 *
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * VA LINUX SYSTEMS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
/* $XFree86: xc/lib/GL/mesa/src/drv/tdfx/tdfx_g3ext.h,v 1.1 2001/03/21 16:14:28 dawes Exp $ */

/*
 * Original rewrite:
 *	Gareth Hughes <gareth@valinux.com>, 29 Sep - 1 Oct 2000
 *
 * Authors:
 *	Gareth Hughes <gareth@valinux.com>
 *
 */

#ifndef __TDFX_G3EXT_H__
#define __TDFX_G3EXT_H__

#ifdef GLX_DIRECT_RENDERING

#include <glide.h>
#include <g3ext.h>

/*
 * These are glide extension definitions.  These are not
 * defined in glide.h.  They should really be defined in
 * g3ext.h, but they are not.
 */
typedef void (*grStencilFunc_t)( GrCmpFnc_t fnc, GrStencil_t ref,
				 GrStencil_t mask );
typedef void (*grStencilMask_t)( GrStencil_t write_mask );
typedef void (*grStencilOp_t)( GrStencilOp_t stencil_fail,
			       GrStencilOp_t depth_fail,
			       GrStencilOp_t depth_pass );
typedef void (*grBufferClearExt_t)( GrColor_t color, GrAlpha_t alpha,
				    FxU32 depth, GrStencil_t stencil );
typedef void (*grColorMaskExt_t)( FxBool r, FxBool g, FxBool b, FxBool a );

/*
 * "COMBINE" extension for Napalm
 */
typedef void (*grColorCombineExt_t)( GrCCUColor_t a, GrCombineMode_t a_mode,
				     GrCCUColor_t b, GrCombineMode_t b_mode,
				     GrCCUColor_t c, FxBool c_invert,
				     GrCCUColor_t d, FxBool d_invert,
				     FxU32 shift, FxBool invert );
typedef void (*grTexColorCombineExt_t)( FxU32 tmu,
					GrTCCUColor_t a,
					GrCombineMode_t a_mode,
					GrTCCUColor_t b,
					GrCombineMode_t b_mode,
					GrTCCUColor_t c, FxBool c_invert,
					GrTCCUColor_t d, FxBool d_invert,
					FxU32 shift, FxBool invert );
typedef void (*grAlphaCombineExt_t)( GrACUColor_t a, GrCombineMode_t a_mode,
				     GrACUColor_t b, GrCombineMode_t b_mode,
				     GrACUColor_t c, FxBool c_invert,
				     GrACUColor_t d, FxBool d_invert,
				     FxU32 shift, FxBool invert );
typedef void (*grTexAlphaCombineExt_t)( FxU32 tmu,
					GrTACUColor_t a,
					GrCombineMode_t a_mode,
					GrTACUColor_t b,
					GrCombineMode_t b_mode,
					GrTACUColor_t c, FxBool c_invert,
					GrTACUColor_t d, FxBool d_invert,
					FxU32 shift, FxBool invert );
typedef void (*grAlphaBlendFunctionExt_t)( GrAlphaBlendFnc_t rgb_sf,
					   GrAlphaBlendFnc_t rgb_df,
					   GrAlphaBlendOp_t rgb_op,
					   GrAlphaBlendFnc_t alpha_sf,
					   GrAlphaBlendFnc_t alpha_df,
					   GrAlphaBlendOp_t alpha_op);
typedef void (*grConstantColorValueExt_t)( FxU32 tmu, GrColor_t value );



/*
 * These are functions to compress and decompress images.
 * The types of the first and second parameters are not exactly
 * right.  The texus library declares them to be "char *", not
 * "void *".  However, "void *" is more correct, and more convenient.
 */
typedef void (*txImgQuantize_t)( void *dst, void *src,
				 int w, int h,
				 FxU32 format, FxU32 dither );
typedef void (*txImgDeQuantize_t)( void *dst, void *src, int w, int h );

/*
 * These next three declarations should probably be taken from
 * texus.h.  However, there are duplicate declarations in g3ext.h
 * and texus.h which make it hard to include them both.
 */
typedef void (*TxErrorCallbackFnc_t)( const char *string, FxBool fatal );
typedef void (*txErrorSetCallback_t)( TxErrorCallbackFnc_t  fnc,
				      TxErrorCallbackFnc_t *old_fnc );

/* PIXEXT extension
 */
GrProc grStencilFuncProc;
GrProc grStencilMaskProc;
GrProc grStencilOpProc;
GrProc grBufferClearExtProc;
GrProc grColorMaskExtProc;

/* COMBINE extension
 */
GrProc grColorCombineExtProc;
GrProc grTexColorCombineExtProc;
GrProc grAlphaCombineExtProc;
GrProc grTexAlphaCombineExtProc;
GrProc grAlphaBlendFunctionExtProc;
GrProc grConstantColorValueExtProc;

/* Texus extensions???
 */
GrProc txImgQuantizeProc;
GrProc txImgDequantizeFXT1Proc;
GrProc txErrorSetCallbackProc;

extern void tdfxDDGlideExtensionsInit( void );

#endif
#endif

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
/* $XFree86: xc/lib/GL/mesa/src/drv/tdfx/tdfx_g3ext.c,v 1.1 2001/03/21 16:14:28 dawes Exp $ */

/*
 * Original rewrite:
 *	Gareth Hughes <gareth@valinux.com>, 29 Sep - 1 Oct 2000
 *
 * Authors:
 *	Gareth Hughes <gareth@valinux.com>
 *
 */

#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>

#include "tdfx_context.h"
#include "tdfx_g3ext.h"


/* STENCIL extension
 */
GrProc grStencilFuncProc = NULL;
GrProc grStencilMaskProc = NULL;
GrProc grStencilOpProc = NULL;
GrProc grBufferClearExtProc = NULL;
GrProc grColorMaskExtProc = NULL;

/* COMBINE extension
 */
GrProc grColorCombineExtProc = NULL;
GrProc grTexColorCombineExtProc = NULL;
GrProc grAlphaCombineExtProc = NULL;
GrProc grTexAlphaCombineExtProc = NULL;
GrProc grAlphaBlendFunctionExtProc = NULL;
GrProc grConstantColorValueExtProc = NULL;

/* Texus 2
 */
GrProc txImgQuantizeProc = NULL;
GrProc txImgDequantizeFXT1Proc = NULL;
GrProc txErrorSetCallbackProc = NULL;


/* Initialize the Glide extensions not exported in the Glide headers.
 * This is just plain evil stuff...
 */
void tdfxDDGlideExtensionsInit( void )
{
   void *handle;

   /* Get Glide3 extension function pointers */
   handle = dlopen( NULL, RTLD_NOW | RTLD_GLOBAL );

   if ( handle ) {
      /* PIXEXT extension */
      grStencilFuncProc		= dlsym( handle, "grStencilFunc" );
      grStencilMaskProc		= dlsym( handle, "grStencilMask" );
      grStencilOpProc		= dlsym( handle, "grStencilOp" );
      grBufferClearExtProc	= dlsym( handle, "grBufferClearExt" );
      grColorMaskExtProc	= dlsym( handle, "grColorMaskExt" );

      /* COMBINE extension */
      grColorCombineExtProc  	= dlsym( handle, "grColorCombineExt" );
      grTexColorCombineExtProc	= dlsym( handle, "grTexColorCombineExt" );
      grAlphaCombineExtProc	= dlsym( handle, "grAlphaCombineExt" );
      grTexAlphaCombineExtProc	= dlsym( handle, "grTexAlphaCombineExt" );
      grAlphaBlendFunctionExtProc = dlsym( handle, "grAlphaBlendFunctionExt" );
      grConstantColorValueExtProc = dlsym( handle, "grConstantColorValueExt" );

      /* Texus 2 */
      txImgQuantizeProc		= dlsym( handle, "txImgQuantize" );
      txImgDequantizeFXT1Proc	= dlsym( handle, "_txImgDequantizeFXT1" );
      txErrorSetCallbackProc	= dlsym( handle, "txErrorSetCallback" );
   } else {
      /* PIXEXT extension */
      grStencilFuncProc		= NULL;
      grStencilMaskProc		= NULL;
      grStencilOpProc		= NULL;
      grBufferClearExtProc	= NULL;
      grColorMaskExtProc	= NULL;

      /* COMBINE extension */
      grColorCombineExtProc  	= NULL;
      grTexColorCombineExtProc	= NULL;
      grAlphaCombineExtProc	= NULL;
      grTexAlphaCombineExtProc	= NULL;
      grAlphaBlendFunctionExtProc = NULL;
      grConstantColorValueExtProc = NULL;

      /* Texus 2 */
      txImgQuantizeProc		= NULL;
      txImgDequantizeFXT1Proc	= NULL;
      txErrorSetCallbackProc	= NULL;
   }

   dlclose( handle );
}

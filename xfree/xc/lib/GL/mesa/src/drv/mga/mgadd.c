/*
 * Copyright 2000-2001 VA Linux Systems, Inc.
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * on the rights to use, copy, modify, merge, publish, distribute, sub
 * license, and/or sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.  IN NO EVENT SHALL
 * VA LINUX SYSTEMS AND/OR ITS SUPPLIERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * Authors:
 *    Keith Whitwell <keithw@valinux.com>
 */
/* $XFree86: xc/lib/GL/mesa/src/drv/mga/mgadd.c,v 1.10 2001/04/10 16:07:50 dawes Exp $ */


#include "types.h"
#include "vbrender.h"


#include <stdio.h>
#include <stdlib.h>

#include "mm.h"
#include "mgacontext.h"
#include "mgadd.h"
#include "mgastate.h"
#include "mgaspan.h"
#include "mgatex.h"
#include "mgatris.h"
#include "mgavb.h"
#include "mgapipeline.h"
#include "extensions.h"
#include "vb.h"
#include "dd.h"

#if defined(USE_X86_ASM) || defined(USE_3DNOW_ASM) || defined(USE_KATMAI_ASM)
#include "X86/common_x86_asm.h"
#endif

#define MGA_DATE	"20010321"



/***************************************
 * Mesa's Driver Functions
 ***************************************/


static const GLubyte *mgaDDGetString( GLcontext *ctx, GLenum name )
{
   mgaContextPtr mmesa = MGA_CONTEXT( ctx );
   static char buffer[128];

   switch ( name ) {
   case GL_VENDOR:
      return (GLubyte *) "VA Linux Systems Inc.";

   case GL_RENDERER:
      sprintf( buffer, "Mesa DRI %s " MGA_DATE,
	       MGA_IS_G400(mmesa) ? "G400" :
	       MGA_IS_G200(mmesa) ? "G200" : "MGA" );

      /* Append any AGP-specific information.
       */
      switch ( mmesa->mgaScreen->agpMode ) {
      case 1:
	 strncat( buffer, " AGP 1x", 7 );
	 break;
      case 2:
	 strncat( buffer, " AGP 2x", 7 );
	 break;
      case 4:
	 strncat( buffer, " AGP 4x", 7 );
	 break;
      }

      /* Append any CPU-specific information.
       */
#ifdef USE_X86_ASM
      if ( gl_x86_cpu_features ) {
	 strncat( buffer, " x86", 4 );
      }
#endif
#ifdef USE_MMX_ASM
      if ( cpu_has_mmx ) {
	 strncat( buffer, "/MMX", 4 );
      }
#endif
#ifdef USE_3DNOW_ASM
      if ( cpu_has_3dnow ) {
	 strncat( buffer, "/3DNow!", 7 );
      }
#endif
#ifdef USE_KATMAI_ASM
      if ( cpu_has_xmm ) {
	 strncat( buffer, "/SSE", 4 );
      }
#endif
      return (GLubyte *)buffer;

   default:
      return NULL;
   }
}


static GLint mgaGetParameteri(const GLcontext *ctx, GLint param)
{
   switch (param) {
   case DD_HAVE_HARDWARE_FOG:
      return 1;
   default:
      fprintf(stderr, "mgaGetParameteri(): unknown parameter!\n");
      return 0;
   }
}


static void mgaBufferSize(GLcontext *ctx, GLuint *width, GLuint *height)
{
   mgaContextPtr mmesa = MGA_CONTEXT(ctx);

   /* Need to lock to make sure the driDrawable is uptodate.  This
    * information is used to resize Mesa's software buffers, so it has
    * to be correct.
    */
   LOCK_HARDWARE( mmesa );
   *width = mmesa->driDrawable->w;
   *height = mmesa->driDrawable->h;
   UNLOCK_HARDWARE( mmesa );
}

void mgaDDExtensionsInit( GLcontext *ctx )
{
   /* paletted_textures currently doesn't work, but we could fix them later */
   gl_extensions_disable( ctx, "GL_EXT_shared_texture_palette" );
   gl_extensions_disable( ctx, "GL_EXT_paletted_texture" );

   /* Support multitexture only on the g400.
    */
   if (!MGA_IS_G400(MGA_CONTEXT(ctx)))
   {
      gl_extensions_disable( ctx, "GL_ARB_multitexture" );
   }

   /* Turn on texenv_add for the G400.
    */
   if (MGA_IS_G400(MGA_CONTEXT(ctx)))
   {
      gl_extensions_enable( ctx, "GL_EXT_texture_env_add" );

#if defined (MESA_packed_depth_stencil)
      gl_extensions_enable( ctx, "GL_MESA_packed_depth_stencil" );
#endif

#if defined (MESA_experimetal_agp_allocator)
      if (!getenv("MGA_DISABLE_AGP_ALLOCATOR"))
	 gl_extensions_enable( ctx, "GL_MESA_experimental_agp_allocator" );
#endif
   }

   /* we don't support point parameters in hardware yet */
   gl_extensions_disable( ctx, "GL_EXT_point_parameters" );

   /* No support for fancy imaging stuff.  This should kill off
    * a few rogue fallbacks.
    */
   gl_extensions_disable( ctx, "ARB_imaging" );
   gl_extensions_disable( ctx, "GL_EXT_blend_color" );
   gl_extensions_disable( ctx, "GL_EXT_blend_minmax" );
   gl_extensions_disable( ctx, "GL_EXT_blend_logic_op" );
   gl_extensions_disable( ctx, "GL_EXT_blend_subtract" );
   gl_extensions_disable( ctx, "GL_INGR_blend_func_separate" );
   gl_extensions_disable( ctx, "GL_EXT_texture_lod_bias" );
   gl_extensions_disable( ctx, "GL_MESA_resize_buffers" );

   gl_extensions_disable( ctx, "GL_SGI_color_matrix" );
   gl_extensions_disable( ctx, "GL_SGI_color_table" );
   gl_extensions_disable( ctx, "GL_SGIX_pixel_texture" );
   gl_extensions_disable( ctx, "GL_ARB_texture_cube_map" );
   gl_extensions_disable( ctx, "GL_ARB_texture_compression" );
   gl_extensions_disable( ctx, "GL_EXT_convolution" );
}



void mgaDDInitDriverFuncs( GLcontext *ctx )
{
   ctx->Driver.GetBufferSize = mgaBufferSize;
   ctx->Driver.GetString = mgaDDGetString;
   ctx->Driver.GetParameteri = mgaGetParameteri;
   ctx->Driver.RegisterVB = mgaDDRegisterVB;
   ctx->Driver.UnregisterVB = mgaDDUnregisterVB;
   ctx->Driver.BuildPrecalcPipeline = mgaDDBuildPrecalcPipeline;
}

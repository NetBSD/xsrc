/* $XFree86: xc/lib/GL/mesa/src/drv/radeon/radeon_dd.c,v 1.6 2001/04/10 16:07:53 dawes Exp $ */
/**************************************************************************

Copyright 2000, 2001 ATI Technologies Inc., Ontario, Canada, and
                     VA Linux Systems Inc., Fremont, California.

All Rights Reserved.

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
on the rights to use, copy, modify, merge, publish, distribute, sub
license, and/or sell copies of the Software, and to permit persons to whom
the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice (including the next
paragraph) shall be included in all copies or substantial portions of the
Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
ATI, VA LINUX SYSTEMS AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM,
DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
USE OR OTHER DEALINGS IN THE SOFTWARE.

**************************************************************************/

/*
 * Authors:
 *   Kevin E. Martin <martin@valinux.com>
 *   Gareth Hughes <gareth@valinux.com>
 *
 */

#include "radeon_context.h"
#include "radeon_ioctl.h"
#include "radeon_state.h"
#include "radeon_vb.h"
#include "radeon_pipeline.h"
#include "radeon_dd.h"

#include "extensions.h"
#if defined(USE_X86_ASM) || defined(USE_3DNOW_ASM) || defined(USE_KATMAI_ASM)
#include "X86/common_x86_asm.h"
#endif

#define RADEON_DATE	"20010402"


/* Return the width and height of the current color buffer.
 */
static void radeonDDGetBufferSize( GLcontext *ctx,
				   GLuint *width, GLuint *height )
{
   radeonContextPtr rmesa = RADEON_CONTEXT(ctx);

   LOCK_HARDWARE( rmesa );
   *width  = rmesa->driDrawable->w;
   *height = rmesa->driDrawable->h;
   UNLOCK_HARDWARE( rmesa );
}

/* Return various strings for glGetString().
 */
static const GLubyte *radeonDDGetString( GLcontext *ctx, GLenum name )
{
   radeonContextPtr rmesa = RADEON_CONTEXT(ctx);
   static char buffer[128];

   switch ( name ) {
   case GL_VENDOR:
      return (GLubyte *)"VA Linux Systems, Inc.";

   case GL_RENDERER:
      sprintf( buffer, "Mesa DRI Radeon " RADEON_DATE );

      /* Append any chipset-specific information.  None yet.
       */

      /* Append any AGP/PCI-specific information.
       */
      if ( rmesa->radeonScreen->IsPCI ) {
	 strncat( buffer, " PCI", 4 );
      } else {
	 switch ( rmesa->radeonScreen->AGPMode ) {
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

/* Send all commands to the hardware.  If vertex buffers or indirect
 * buffers are in use, then we need to make sure they are sent to the
 * hardware.  All commands that are normally sent to the ring are
 * already considered `flushed'.
 */
static void radeonDDFlush( GLcontext *ctx )
{
   radeonContextPtr rmesa = RADEON_CONTEXT(ctx);

   FLUSH_BATCH( rmesa );

#if ENABLE_PERF_BOXES
   if ( rmesa->boxes ) {
      LOCK_HARDWARE( rmesa );
      radeonPerformanceBoxesLocked( rmesa );
      UNLOCK_HARDWARE( rmesa );
   }

   /* Log the performance counters if necessary */
   radeonPerformanceCounters( rmesa );
#endif
}

/* Make sure all commands have been sent to the hardware and have
 * completed processing.
 */
static void radeonDDFinish( GLcontext *ctx )
{
   radeonContextPtr rmesa = RADEON_CONTEXT(ctx);

#if ENABLE_PERF_BOXES
   /* Bump the performance counter */
   rmesa->c_drawWaits++;
#endif

   radeonDDFlush( ctx );
   radeonWaitForIdle( rmesa );
}

/* Return various parameters requested by Mesa (this is deprecated).
 */
static GLint radeonDDGetParameteri( const GLcontext *ctx, GLint param )
{
   switch ( param ) {
   case DD_HAVE_HARDWARE_FOG:
      return 1;
   default:
      return 0;
   }
}

/* Initialize the extensions supported by this driver.
 */
void radeonDDInitExtensions( GLcontext *ctx )
{
   gl_extensions_disable( ctx, "GL_ARB_imaging" );
   gl_extensions_disable( ctx, "GL_ARB_texture_compression" );
   gl_extensions_disable( ctx, "GL_ARB_texture_cube_map" );

   gl_extensions_disable( ctx, "GL_EXT_blend_color" );
   gl_extensions_disable( ctx, "GL_EXT_blend_logic_op" );
   gl_extensions_disable( ctx, "GL_EXT_blend_minmax" );
   gl_extensions_disable( ctx, "GL_EXT_blend_subtract" );
   gl_extensions_disable( ctx, "GL_EXT_convolution" );
   gl_extensions_disable( ctx, "GL_EXT_paletted_texture" );
   gl_extensions_disable( ctx, "GL_EXT_point_parameters" );
   gl_extensions_disable( ctx, "GL_EXT_shared_texture_palette" );
   gl_extensions_enable(  ctx, "GL_EXT_texture_env_combine" );
   gl_extensions_enable(  ctx, "GL_EXT_texture_env_dot3" );

   gl_extensions_disable( ctx, "GL_HP_occlusion_test" );

   gl_extensions_disable( ctx, "GL_INGR_blend_func_separate" );

   gl_extensions_disable( ctx, "GL_SGI_color_matrix" );
   gl_extensions_disable( ctx, "GL_SGI_color_table" );
   gl_extensions_disable( ctx, "GL_SGIX_pixel_texture" );
}

/* Initialize the driver's misc functions.
 */
void radeonDDInitDriverFuncs( GLcontext *ctx )
{
    ctx->Driver.GetBufferSize		= radeonDDGetBufferSize;
    ctx->Driver.GetString		= radeonDDGetString;
    ctx->Driver.Finish			= radeonDDFinish;
    ctx->Driver.Flush			= radeonDDFlush;

    ctx->Driver.Error			= NULL;
    ctx->Driver.GetParameteri		= radeonDDGetParameteri;

    ctx->Driver.DrawPixels		= NULL;
    ctx->Driver.Bitmap			= NULL;

    ctx->Driver.RegisterVB		= radeonDDRegisterVB;
    ctx->Driver.UnregisterVB		= radeonDDUnregisterVB;
    ctx->Driver.BuildPrecalcPipeline	= radeonDDBuildPrecalcPipeline;
}

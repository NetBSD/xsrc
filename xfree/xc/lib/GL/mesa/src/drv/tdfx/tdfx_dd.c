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
/* $XFree86: xc/lib/GL/mesa/src/drv/tdfx/tdfx_dd.c,v 1.5 2001/08/18 02:51:06 dawes Exp $ */

/*
 * Original rewrite:
 *	Gareth Hughes <gareth@valinux.com>, 29 Sep - 1 Oct 2000
 *
 * Authors:
 *	Gareth Hughes <gareth@valinux.com>
 *	Brian Paul <brianp@valinux.com>
 *
 */

#include "tdfx_context.h"
#include "tdfx_dd.h"
#include "tdfx_vb.h"
#include "tdfx_pipeline.h"
#include "tdfx_pixels.h"

#include "enums.h"
#include "pb.h"
#if defined(USE_X86_ASM) || defined(USE_3DNOW_ASM) || defined(USE_KATMAI_ASM)
#include "X86/common_x86_asm.h"
#endif

#define TDFX_DATE	"20010501"


/* These are used in calls to FX_grColorMaskv() */
const GLboolean false4[4] = { GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE };
const GLboolean true4[4] = { GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE };



/* KW: Put the word Mesa in the render string because quakeworld
 * checks for this rather than doing a glGet(GL_MAX_TEXTURE_SIZE).
 * Why?
 */
static const GLubyte *tdfxDDGetString( GLcontext *ctx, GLenum name )
{
   tdfxContextPtr fxMesa = (tdfxContextPtr) ctx->DriverCtx;

   switch ( name ) {
   case GL_VENDOR:
      return (GLubyte *)"VA Linux Systems, Inc.";

   case GL_RENDERER: {
      static char buffer[128];
      char hardware[128];

      strcpy( hardware, FX_grGetString( fxMesa, GR_HARDWARE ) );

      if ( strcmp( hardware, "Voodoo3 (tm)" ) == 0 ) {
	 strcpy( hardware, "Voodoo3" );
      }
      else if ( strcmp( hardware, "Voodoo Banshee (tm)" ) == 0 ) {
	 strcpy( hardware, "VoodooBanshee" );
      }
      else if ( strcmp( hardware, "Voodoo4 (tm)" ) == 0 ) {
	 strcpy( hardware, "Voodoo4" );
      }
      else if ( strcmp( hardware, "Voodoo5 (tm)" ) == 0 ) {
	 strcpy( hardware, "Voodoo5" );
      }
      else {
	 /* Unexpected result: replace spaces with hyphens */
	 int i;
	 for ( i = 0 ; hardware[i] ; i++ ) {
	    if ( hardware[i] == ' ' || hardware[i] == '\t' )
	       hardware[i] = '-';
	 }
      }
      /* Now make the GL_RENDERER string */
      sprintf( buffer, "Mesa DRI %s " TDFX_DATE, hardware );

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
   }

   default:
      return NULL;
   }
}


/* Return buffer size information.
 */
static void tdfxDDGetBufferSize( GLcontext *ctx,
				 GLuint *width, GLuint *height )
{
   tdfxContextPtr fxMesa = TDFX_CONTEXT(ctx);

   *width = fxMesa->width;
   *height = fxMesa->height;
}


static GLint tdfxDDGetParameteri( const GLcontext *ctx, GLint param )
{
   switch ( param ) {
   case DD_HAVE_HARDWARE_FOG:
      return 1;
   default:
      return 0;
   }
}

/*
 * Return the current value of the occlusion test flag and
 * reset the flag (hardware counters) to false.
 */
static GLboolean get_occlusion_result( GLcontext *ctx )
{
   tdfxContextPtr fxMesa = TDFX_CONTEXT(ctx);
   GLboolean result;

   LOCK_HARDWARE( fxMesa );

   if (ctx->Depth.OcclusionTest) {
      if (ctx->OcclusionResult) {
	 result = GL_TRUE;  /* result of software rendering */
      }
      else {
	 FxI32 zfail, in;
         fxMesa->Glide.grGet(GR_STATS_PIXELS_DEPTHFUNC_FAIL, 4, &zfail);
	 /*zfail = FX_grGetInteger_NoLock(GR_STATS_PIXELS_DEPTHFUNC_FAIL);*/
         fxMesa->Glide.grGet(GR_STATS_PIXELS_IN, 4, &in);
	 /*in = FX_grGetInteger_NoLock(GR_STATS_PIXELS_IN);*/
	 if (in == zfail)
	    result = GL_FALSE; /* geom was completely occluded */
	 else
	    result = GL_TRUE;  /* all or part of geom was visible */
      }
   }
   else {
      result = ctx->OcclusionResultSaved;
   }

   /* reset results now */
   fxMesa->Glide.grReset(GR_STATS_PIXELS);
   ctx->OcclusionResult = GL_FALSE;
   ctx->OcclusionResultSaved = GL_FALSE;

   UNLOCK_HARDWARE( fxMesa );

   return result;
}


/*
 * We're only implementing this function to handle the
 * GL_OCCLUSTION_TEST_RESULT_HP case.  It's special because it
 * has a side-effect: resetting the occlustion result flag.
 */
static GLboolean tdfxDDGetBooleanv( GLcontext *ctx, GLenum pname,
				    GLboolean *result )
{
   if ( pname == GL_OCCLUSION_TEST_RESULT_HP ) {
      *result = get_occlusion_result( ctx );
      return GL_TRUE;
   }
   return GL_FALSE;
}

static GLboolean tdfxDDGetDoublev( GLcontext *ctx, GLenum pname,
				   GLdouble *result )
{
   if ( pname == GL_OCCLUSION_TEST_RESULT_HP ) {
      *result = (GLdouble) get_occlusion_result( ctx );
      return GL_TRUE;
   }
   return GL_FALSE;
}

static GLboolean tdfxDDGetFloatv( GLcontext *ctx, GLenum pname,
				  GLfloat *result )
{
   if ( pname == GL_OCCLUSION_TEST_RESULT_HP ) {
      *result = (GLfloat) get_occlusion_result( ctx );
      return GL_TRUE;
   }
   return GL_FALSE;
}

static GLboolean tdfxDDGetIntegerv( GLcontext *ctx, GLenum pname,
				    GLint *result )
{
   if ( pname == GL_OCCLUSION_TEST_RESULT_HP ) {
      *result = (GLint) get_occlusion_result( ctx );
      return GL_TRUE;
   }
   return GL_FALSE;
}



#define VISUAL_EQUALS_RGBA(vis, r, g, b, a)        \
   ((vis->RedBits == r) &&                         \
    (vis->GreenBits == g) &&                       \
    (vis->BlueBits == b) &&                        \
    (vis->AlphaBits == a))

void tdfxDDInitDriverFuncs( GLcontext *ctx )
{
   if ( MESA_VERBOSE & VERBOSE_DRIVER ) {
      fprintf( stderr, "tdfx: %s()\n", __FUNCTION__ );
   }

   ctx->Driver.GetString		= tdfxDDGetString;
   ctx->Driver.GetBufferSize		= tdfxDDGetBufferSize;
   ctx->Driver.Error			= NULL;
   ctx->Driver.GetParameteri		= tdfxDDGetParameteri;

   if ( VISUAL_EQUALS_RGBA(ctx->Visual, 8, 8, 8, 8) )
   {
      ctx->Driver.DrawPixels		= tdfx_drawpixels_R8G8B8A8;
      ctx->Driver.ReadPixels		= tdfx_readpixels_R8G8B8A8;
      ctx->Driver.CopyPixels		= NULL;
      ctx->Driver.Bitmap		= NULL;
   }
   else if ( VISUAL_EQUALS_RGBA(ctx->Visual, 5, 6, 5, 0) )
   {
      ctx->Driver.DrawPixels		= NULL;
      ctx->Driver.ReadPixels		= tdfx_readpixels_R5G6B5;
      ctx->Driver.CopyPixels		= NULL;
      ctx->Driver.Bitmap		= NULL;
   }
   else
   {
      ctx->Driver.DrawPixels		= NULL;
      ctx->Driver.ReadPixels		= NULL;
      ctx->Driver.CopyPixels		= NULL;
      ctx->Driver.Bitmap		= NULL;
   }

   ctx->Driver.RegisterVB		= tdfxDDRegisterVB;
   ctx->Driver.UnregisterVB		= tdfxDDUnregisterVB;
   ctx->Driver.ResetVB			= NULL;
   ctx->Driver.ResetCvaVB		= NULL;

   if ( !getenv( "TDFX_NO_FAST" ) ) {
      ctx->Driver.BuildPrecalcPipeline	= tdfxDDBuildPrecalcPipeline;
   } else {
      ctx->Driver.BuildPrecalcPipeline	= NULL;
   }
   ctx->Driver.BuildEltPipeline		= NULL;

   ctx->Driver.OptimizeImmediatePipeline = NULL;
   ctx->Driver.OptimizePrecalcPipeline	= NULL;

   ctx->Driver.GetBooleanv		= tdfxDDGetBooleanv;
   ctx->Driver.GetDoublev		= tdfxDDGetDoublev;
   ctx->Driver.GetFloatv		= tdfxDDGetFloatv;
   ctx->Driver.GetIntegerv		= tdfxDDGetIntegerv;
   ctx->Driver.GetPointerv		= NULL;
}

/*
 * GLX Hardware Device Driver for Matrox G200/G400
 * Copyright (C) 1999 Wittawat Yamwong
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
 * WITTAWAT YAMWONG, OR ANY OTHER CONTRIBUTORS BE LIABLE FOR ANY CLAIM, 
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR 
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE 
 * OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 *
 *    Wittawat Yamwong <Wittawat.Yamwong@stud.uni-hannover.de>
 */
/* $XFree86: xc/lib/GL/mesa/src/drv/mga/mgadd.c,v 1.6 2000/11/08 05:02:45 dawes Exp $ */



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



/***************************************
 * Mesa's Driver Functions
 ***************************************/


static const GLubyte *mgaDDGetString( GLcontext *ctx, GLenum name )
{
   mgaContextPtr mmesa = MGA_CONTEXT( ctx );
   switch (name) {
   case GL_VENDOR:
      return (GLubyte *) "Precision Insight, Inc.";
   case GL_RENDERER:
      if (MGA_IS_G200(mmesa)) return (GLubyte *) "Mesa DRI G200 20000510";
      if (MGA_IS_G400(mmesa)) return (GLubyte *) "Mesa DRI G400 20000510";
      return (GLubyte *) "Mesa DRI MGA 20000510";
   default:
      return 0;
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

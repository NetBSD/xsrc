/*
 * GLX Hardware Device Driver for Intel i810
 * Copyright (C) 1999 Keith Whitwell
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
 * KEITH WHITWELL, OR ANY OTHER CONTRIBUTORS BE LIABLE FOR ANY CLAIM, 
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR 
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE 
 * OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */
/* $XFree86: xc/lib/GL/mesa/src/drv/i810/i810dd.c,v 1.4 2000/09/24 13:51:04 alanh Exp $ */

#include "types.h"
#include "vbrender.h"

#include <stdio.h>

#include "mm.h"
#include "i810dd.h"
#include "i810log.h"
#include "i810state.h"
#include "i810span.h"
#include "i810tex.h"
#include "i810tris.h"
#include "i810vb.h"
#include "i810pipeline.h"
#include "extensions.h"
#include "vb.h"
#include "dd.h"


extern int xf86VTSema;


/***************************************
 * Mesa's Driver Functions
 ***************************************/


static const GLubyte *i810DDGetString( GLcontext *ctx, GLenum name )
{
   switch (name) {
   case GL_VENDOR:
      return (GLubyte *)"Keith Whitwell, Precision Insight Inc.";
   case GL_RENDERER:
      return (GLubyte *)"Mesa DRI I810 20000510";
   default:
      return 0;
   }
}

static GLint i810GetParameteri(const GLcontext *ctx, GLint param)
{
   switch (param) {
   case DD_HAVE_HARDWARE_FOG:  
      return 1;
   default:
      return 0; 
   }
}



static void i810BufferSize(GLcontext *ctx, GLuint *width, GLuint *height)
{
   i810ContextPtr imesa = I810_CONTEXT(ctx);  

   /* Need to lock to make sure the driDrawable is uptodate.  This
    * information is used to resize Mesa's software buffers, so it has
    * to be correct.
    */
   LOCK_HARDWARE(imesa);
   *width = imesa->driDrawable->w;
   *height = imesa->driDrawable->h;
   UNLOCK_HARDWARE(imesa);
}




void i810DDExtensionsInit( GLcontext *ctx )
{
   /* paletted_textures currently doesn't work.
    */
   gl_extensions_disable( ctx, "GL_EXT_shared_texture_palette" );
   gl_extensions_disable( ctx, "GL_EXT_paletted_texture" );

   /* we don't support point parameters in hardware yet */
   gl_extensions_disable( ctx, "GL_EXT_point_parameters" );

   /* The imaging subset of 1.2 isn't supported by any mesa driver.
    */
   gl_extensions_disable( ctx, "ARB_imaging" );
   gl_extensions_disable( ctx, "GL_ARB_texture_compression" );   
   gl_extensions_disable( ctx, "GL_ARB_texture_cube_map" );   
   gl_extensions_disable( ctx, "GL_EXT_blend_color" );
   gl_extensions_disable( ctx, "GL_EXT_blend_logic_op" );
   gl_extensions_disable( ctx, "GL_EXT_blend_minmax" );
   gl_extensions_disable( ctx, "GL_EXT_blend_subtract" );
   gl_extensions_disable( ctx, "GL_EXT_convolution" );   
   gl_extensions_disable( ctx, "GL_EXT_texture_lod_bias" );   
   gl_extensions_disable( ctx, "GL_INGR_blend_func_separate" );
   gl_extensions_disable( ctx, "GL_MESA_resize_buffers" );   
   gl_extensions_disable( ctx, "GL_SGIX_pixel_texture" );   
   gl_extensions_disable( ctx, "GL_SGI_color_matrix" );   
   gl_extensions_disable( ctx, "GL_SGI_color_table" );   

   /* We do support tex_env_add, however
    */
   gl_extensions_enable( ctx, "GL_EXT_texture_env_add" );
}




void i810DDInitDriverFuncs( GLcontext *ctx )
{
   ctx->Driver.GetBufferSize = i810BufferSize;
   ctx->Driver.GetString = i810DDGetString;
   ctx->Driver.GetParameteri = i810GetParameteri;
   ctx->Driver.RegisterVB = i810DDRegisterVB;
   ctx->Driver.UnregisterVB = i810DDUnregisterVB;
   ctx->Driver.Clear = i810Clear;


   ctx->Driver.BuildPrecalcPipeline = i810DDBuildPrecalcPipeline;
}

/**************************************************************************

Copyright 2001 VA Linux Systems Inc., Fremont, California.

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

/* $XFree86: xc/lib/GL/mesa/src/drv/i830/i830_dd.c,v 1.1 2001/10/04 18:28:21 alanh Exp $ */

/*
 * Author:
 *   Jeff Hartmann <jhartmann@valinux.com>
 *
 * Heavily based on the I810 driver, which was written by:
 *   Keith Whitwell <keithw@valinux.com>
 */

#include "types.h"
#include "vbrender.h"

#include <stdio.h>

#include "mm.h"
#include "extensions.h"
#include "vb.h"
#include "dd.h"

#include "i830_drv.h"
#include "i830_ioctl.h"

extern int xf86VTSema;


/***************************************
 * Mesa's Driver Functions
 ***************************************/


static const GLubyte *i830DDGetString( GLcontext *ctx, GLenum name )
{
   switch (name) {
   case GL_VENDOR:
      return (GLubyte *)"VA Linux Systems";
   case GL_RENDERER:
      return (GLubyte *)"Mesa DRI I830 20001127";
   default:
      return 0;
   }
}

static GLint i830GetParameteri(const GLcontext *ctx, GLint param)
{
   switch (param) {
   case DD_HAVE_HARDWARE_FOG:  
      return 1;
   default:
      return 0; 
   }
}



static void i830BufferSize(GLcontext *ctx, GLuint *width, GLuint *height)
{
   i830ContextPtr imesa = I830_CONTEXT(ctx);  

   /* Need to lock to make sure the driDrawable is uptodate.  This
    * information is used to resize Mesa's software buffers, so it has
    * to be correct.
    */
   LOCK_HARDWARE(imesa);
   *width = imesa->driDrawable->w;
   *height = imesa->driDrawable->h;
   UNLOCK_HARDWARE(imesa);
}




void i830DDExtensionsInit( GLcontext *ctx )
{
   gl_extensions_enable( ctx, "GL_EXT_blend_color" );
   gl_extensions_enable( ctx, "GL_EXT_blend_minmax" );
   gl_extensions_enable( ctx, "GL_EXT_blend_subtract" );
   gl_extensions_enable( ctx, "GL_EXT_blend_logic_op" );

   gl_extensions_enable( ctx, "GL_EXT_blend_func_separate" );
   gl_extensions_enable( ctx, "GL_ARB_texture_env_add" );
   gl_extensions_enable( ctx, "GL_EXT_texture_env_add" );
   gl_extensions_enable( ctx, "GL_EXT_texture_env_combine" );

#if 0
   /* These need a little work, the code is there, but some applications
    * don't render correctly.
    */

   gl_extensions_enable( ctx, "GL_EXT_shared_texture_palette" );
   gl_extensions_enable( ctx, "GL_EXT_paletted_texture" );
#else
   gl_extensions_disable( ctx, "GL_EXT_shared_texture_palette" );
   gl_extensions_disable( ctx, "GL_EXT_paletted_texture" );
#endif

   /* This should be easy to add */
   gl_extensions_disable( ctx, "GL_EXT_texture_lod_bias" );

   /* Planned extensions */
   gl_extensions_disable( ctx, "GL_ARB_texture_compression" );   
   gl_extensions_disable( ctx, "GL_ARB_texture_cube_map" );

   /* we don't support point parameters in hardware yet */
   gl_extensions_disable( ctx, "GL_EXT_point_parameters" );

   /* The imaging subset of 1.2 isn't supported by any mesa driver.
    */
   gl_extensions_disable( ctx, "ARB_imaging" );
   gl_extensions_disable( ctx, "GL_EXT_convolution" );
   gl_extensions_disable( ctx, "GL_MESA_resize_buffers" );
   gl_extensions_disable( ctx, "GL_SGIX_pixel_texture" );
   gl_extensions_disable( ctx, "GL_SGI_color_matrix" );
   gl_extensions_disable( ctx, "GL_SGI_color_table" );
}




void i830DDInitDriverFuncs( GLcontext *ctx )
{
   ctx->Driver.GetBufferSize = i830BufferSize;
   ctx->Driver.GetString = i830DDGetString;
   ctx->Driver.GetParameteri = i830GetParameteri;
   ctx->Driver.RegisterVB = i830DDRegisterVB;
   ctx->Driver.UnregisterVB = i830DDUnregisterVB;
   ctx->Driver.Clear = i830Clear;


   ctx->Driver.BuildPrecalcPipeline = i830DDBuildPrecalcPipeline;
}

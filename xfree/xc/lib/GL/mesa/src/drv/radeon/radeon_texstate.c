/* $XFree86: xc/lib/GL/mesa/src/drv/radeon/radeon_texstate.c,v 1.1 2001/03/21 16:14:25 dawes Exp $ */
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
#include "radeon_state.h"
#include "radeon_ioctl.h"
#include "radeon_vb.h"
#include "radeon_tex.h"

#include "mmath.h"
#include "simple_list.h"
#include "enums.h"
#include "mem.h"

static void radeonSetTexImages( radeonContextPtr rmesa,
				struct gl_texture_object *tObj )
{
   radeonTexObjPtr t = (radeonTexObjPtr)tObj->DriverData;
   struct gl_texture_image *texImage = tObj->Image[0];
   const struct gl_texture_format *texFormat = texImage->TexFormat;
   GLint log2Width, log2Height, log2Size;
   GLint totalSize;
   GLint texelsPerDword = 0, blitWidth = 0, blitPitch = 0;
   GLint x, y, width, height;
   GLint i;

   /* Calculate dimensions in log domain.
    */
   for ( i = 1, log2Height = 0 ; i < texImage->Height ; i *= 2 ) {
      log2Height++;
   }
   for ( i = 1, log2Width = 0 ;  i < texImage->Width ;  i *= 2 ) {
      log2Width++;
   }
   log2Size = MAX2( log2Width, log2Height );

   /* The Radeon has a 64-byte minimum pitch for all blits.  We
    * calculate the equivalent number of texels to simplify the
    * calculation of the texture image area.
    */
   switch ( texFormat->TexelBytes ) {
   case 4:
      texelsPerDword = 1;
      blitPitch = 16;
      break;
   case 2:
      texelsPerDword = 2;
      blitPitch = 32;
      break;
   case 1:
      texelsPerDword = 4;
      blitPitch = 64;
      break;
   }

   /* Select the larger of the two widths for our global texture image
    * coordinate space.  As the Radeon has very strict offset rules, we
    * can't upload mipmaps directly and have to reference their location
    * from the aligned start of the whole image.
    */
   blitWidth = MAX2( texImage->Width, blitPitch );

   /* Calculate mipmap offsets and dimensions.
    */
   totalSize = 0;
   x = 0;
   y = 0;

   for ( i = 0 ; i <= log2Size ; i++ ) {
      GLuint size;

      texImage = tObj->Image[i];
      if ( !texImage )
	 break;

      width = texImage->Width;
      height = texImage->Height;

      /* Texture images have a minimum pitch of 32 bytes (half of the
       * 64-byte minimum pitch for blits).  For images that have a
       * width smaller than this, we must pad each texture image
       * scanline out to this amount.
       */
      if ( width < blitPitch / 2 ) {
	 width = blitPitch / 2;
      }

      size = width * height * texFormat->TexelBytes;
      totalSize += size;
      ASSERT( (totalSize & 31) == 0 );

      while ( width < blitWidth && height > 1 ) {
	 width *= 2;
	 height /= 2;
      }

      t->image[i].x = x;
      t->image[i].y = y;

      t->image[i].width  = width;
      t->image[i].height = height;

      /* While blits must have a pitch of at least 64 bytes, mipmaps
       * must be aligned on a 32-byte boundary (just like each texture
       * image scanline).
       */
      if ( width >= blitWidth ) {
	 y += height;
      } else {
	 x += width;
	 if ( x >= blitWidth ) {
	    x = 0;
	    y++;
	 }
      }

      if ( 0 ) {
	 fprintf( stderr, "level=%d p=%d   %dx%d -> %dx%d at (%d,%d)\n",
		  i, blitWidth, texImage->Width, texImage->Height,
		  t->image[i].width, t->image[i].height,
		  t->image[i].x, t->image[i].y );
      }
   }

   /* Align the total size of texture memory block.
    */
   t->totalSize = (totalSize + RADEON_OFFSET_MASK) & ~RADEON_OFFSET_MASK;

   /* Hardware state:
    */
   t->pp_txfilter &= ~RADEON_MAX_MIP_LEVEL_MASK;
   t->pp_txfilter |= i << RADEON_MAX_MIP_LEVEL_SHIFT;

   t->pp_txformat &= ~(RADEON_TXFORMAT_WIDTH_MASK |
		       RADEON_TXFORMAT_HEIGHT_MASK);
   t->pp_txformat |= ((log2Width << RADEON_TXFORMAT_WIDTH_SHIFT) |
		      (log2Height << RADEON_TXFORMAT_HEIGHT_SHIFT));
}


/* ================================================================
 * Texture combine functions
 */

#define RADEON_DISABLE		0
#define RADEON_REPLACE		1
#define RADEON_MODULATE		2
#define RADEON_DECAL		3
#define RADEON_BLEND		4
#define RADEON_ADD		5
#define RADEON_MAX_COMBFUNC	6

static GLuint radeon_color_combine[][RADEON_MAX_COMBFUNC] =
{
   /* Unit 0:
    */
   {
      /* Disable combiner stage
       */
      (RADEON_COLOR_ARG_A_ZERO |
       RADEON_COLOR_ARG_B_ZERO |
       RADEON_COLOR_ARG_C_CURRENT_COLOR |
       RADEON_BLEND_CTL_ADD |
       RADEON_SCALE_1X |
       RADEON_CLAMP_TX),

      /* GL_REPLACE = 0x00802800
       */
      (RADEON_COLOR_ARG_A_ZERO |
       RADEON_COLOR_ARG_B_ZERO |
       RADEON_COLOR_ARG_C_T0_COLOR |
       RADEON_BLEND_CTL_ADD |
       RADEON_SCALE_1X |
       RADEON_CLAMP_TX),

      /* GL_MODULATE = 0x00800142
       */
      (RADEON_COLOR_ARG_A_CURRENT_COLOR |
       RADEON_COLOR_ARG_B_T0_COLOR |
       RADEON_COLOR_ARG_C_ZERO |
       RADEON_BLEND_CTL_ADD |
       RADEON_SCALE_1X |
       RADEON_CLAMP_TX),

      /* GL_DECAL = 0x008c2d42
       */
      (RADEON_COLOR_ARG_A_CURRENT_COLOR |
       RADEON_COLOR_ARG_B_T0_COLOR |
       RADEON_COLOR_ARG_C_T0_ALPHA |
       RADEON_BLEND_CTL_BLEND |
       RADEON_SCALE_1X |
       RADEON_CLAMP_TX),

      /* GL_BLEND = 0x008c2902
       */
      (RADEON_COLOR_ARG_A_CURRENT_COLOR |
       RADEON_COLOR_ARG_B_TFACTOR_COLOR |
       RADEON_COLOR_ARG_C_T0_COLOR |
       RADEON_BLEND_CTL_BLEND |
       RADEON_SCALE_1X |
       RADEON_CLAMP_TX),

      /* GL_ADD = 0x00812802
       */
      (RADEON_COLOR_ARG_A_CURRENT_COLOR |
       RADEON_COLOR_ARG_B_ZERO |
       RADEON_COLOR_ARG_C_T0_COLOR |
       RADEON_COMP_ARG_B |
       RADEON_BLEND_CTL_ADD |
       RADEON_SCALE_1X |
       RADEON_CLAMP_TX),
   },

   /* Unit 1:
    */
   {
      /* Disable combiner stage
       */
      (RADEON_COLOR_ARG_A_ZERO |
       RADEON_COLOR_ARG_B_ZERO |
       RADEON_COLOR_ARG_C_CURRENT_COLOR |
       RADEON_BLEND_CTL_ADD |
       RADEON_SCALE_1X |
       RADEON_CLAMP_TX),

      /* GL_REPLACE = 0x00803000
       */
      (RADEON_COLOR_ARG_A_ZERO |
       RADEON_COLOR_ARG_B_ZERO |
       RADEON_COLOR_ARG_C_T1_COLOR |
       RADEON_BLEND_CTL_ADD |
       RADEON_SCALE_1X |
       RADEON_CLAMP_TX),

      /* GL_MODULATE = 0x00800182
       */
      (RADEON_COLOR_ARG_A_CURRENT_COLOR |
       RADEON_COLOR_ARG_B_T1_COLOR |
       RADEON_COLOR_ARG_C_ZERO |
       RADEON_BLEND_CTL_ADD |
       RADEON_SCALE_1X |
       RADEON_CLAMP_TX),

      /* GL_DECAL = 0x008c3582
       */
      (RADEON_COLOR_ARG_A_CURRENT_COLOR |
       RADEON_COLOR_ARG_B_T1_COLOR |
       RADEON_COLOR_ARG_C_T1_ALPHA |
       RADEON_BLEND_CTL_BLEND |
       RADEON_SCALE_1X |
       RADEON_CLAMP_TX),

      /* GL_BLEND = 0x008c3102
       */
      (RADEON_COLOR_ARG_A_CURRENT_COLOR |
       RADEON_COLOR_ARG_B_TFACTOR_COLOR |
       RADEON_COLOR_ARG_C_T1_COLOR |
       RADEON_BLEND_CTL_BLEND |
       RADEON_SCALE_1X |
       RADEON_CLAMP_TX),

      /* GL_ADD = 0x00813002
       */
      (RADEON_COLOR_ARG_A_CURRENT_COLOR |
       RADEON_COLOR_ARG_B_ZERO |
       RADEON_COLOR_ARG_C_T1_COLOR |
       RADEON_COMP_ARG_B |
       RADEON_BLEND_CTL_ADD |
       RADEON_SCALE_1X |
       RADEON_CLAMP_TX),
   },

   /* Unit 2:
    */
   {
      /* Disable combiner stage
       */
      (RADEON_COLOR_ARG_A_ZERO |
       RADEON_COLOR_ARG_B_ZERO |
       RADEON_COLOR_ARG_C_CURRENT_COLOR |
       RADEON_BLEND_CTL_ADD |
       RADEON_SCALE_1X |
       RADEON_CLAMP_TX),

      /* GL_REPLACE = 0x00803800
       */
      (RADEON_COLOR_ARG_A_ZERO |
       RADEON_COLOR_ARG_B_ZERO |
       RADEON_COLOR_ARG_C_T2_COLOR |
       RADEON_BLEND_CTL_ADD |
       RADEON_SCALE_1X |
       RADEON_CLAMP_TX),

      /* GL_MODULATE = 0x008001c2
       */
      (RADEON_COLOR_ARG_A_CURRENT_COLOR |
       RADEON_COLOR_ARG_B_T2_COLOR |
       RADEON_COLOR_ARG_C_ZERO |
       RADEON_BLEND_CTL_ADD |
       RADEON_SCALE_1X |
       RADEON_CLAMP_TX),

      /* GL_DECAL = 0x008c3dc2
       */
      (RADEON_COLOR_ARG_A_CURRENT_COLOR |
       RADEON_COLOR_ARG_B_T2_COLOR |
       RADEON_COLOR_ARG_C_T2_ALPHA |
       RADEON_BLEND_CTL_BLEND |
       RADEON_SCALE_1X |
       RADEON_CLAMP_TX),

      /* GL_BLEND = 0x008c3902
       */
      (RADEON_COLOR_ARG_A_CURRENT_COLOR |
       RADEON_COLOR_ARG_B_TFACTOR_COLOR |
       RADEON_COLOR_ARG_C_T2_COLOR |
       RADEON_BLEND_CTL_BLEND |
       RADEON_SCALE_1X |
       RADEON_CLAMP_TX),

      /* GL_ADD = 0x00813802
       */
      (RADEON_COLOR_ARG_A_CURRENT_COLOR |
       RADEON_COLOR_ARG_B_ZERO |
       RADEON_COLOR_ARG_C_T2_COLOR |
       RADEON_COMP_ARG_B |
       RADEON_BLEND_CTL_ADD |
       RADEON_SCALE_1X |
       RADEON_CLAMP_TX),
   }
};

static GLuint radeon_alpha_combine[][RADEON_MAX_COMBFUNC] =
{
   /* Unit 0:
    */
   {
      /* Disable combiner stage
       */
      (RADEON_ALPHA_ARG_A_ZERO |
       RADEON_ALPHA_ARG_B_ZERO |
       RADEON_ALPHA_ARG_C_CURRENT_ALPHA |
       RADEON_BLEND_CTL_ADD |
       RADEON_SCALE_1X |
       RADEON_CLAMP_TX),

      /* GL_REPLACE = 0x00800500
       */
      (RADEON_ALPHA_ARG_A_ZERO |
       RADEON_ALPHA_ARG_B_ZERO |
       RADEON_ALPHA_ARG_C_T0_ALPHA |
       RADEON_BLEND_CTL_ADD |
       RADEON_SCALE_1X |
       RADEON_CLAMP_TX),

      /* GL_MODULATE = 0x00800051
       */
      (RADEON_ALPHA_ARG_A_CURRENT_ALPHA |
       RADEON_ALPHA_ARG_B_T0_ALPHA |
       RADEON_ALPHA_ARG_C_ZERO |
       RADEON_BLEND_CTL_ADD |
       RADEON_SCALE_1X |
       RADEON_CLAMP_TX),

      /* GL_DECAL = 0x00800100
       */
      (RADEON_ALPHA_ARG_A_ZERO |
       RADEON_ALPHA_ARG_B_ZERO |
       RADEON_ALPHA_ARG_C_CURRENT_ALPHA |
       RADEON_BLEND_CTL_ADD |
       RADEON_SCALE_1X |
       RADEON_CLAMP_TX),

      /* GL_BLEND = 0x00800051
       */
      (RADEON_ALPHA_ARG_A_CURRENT_ALPHA |
       RADEON_ALPHA_ARG_B_TFACTOR_ALPHA |
       RADEON_ALPHA_ARG_C_T0_ALPHA |
       RADEON_BLEND_CTL_BLEND |
       RADEON_SCALE_1X |
       RADEON_CLAMP_TX),

      /* GL_ADD = 0x00800051
       */
      (RADEON_ALPHA_ARG_A_CURRENT_ALPHA |
       RADEON_ALPHA_ARG_B_ZERO |
       RADEON_ALPHA_ARG_C_T0_ALPHA |
       RADEON_COMP_ARG_B |
       RADEON_BLEND_CTL_ADD |
       RADEON_SCALE_1X |
       RADEON_CLAMP_TX),
   },

   /* Unit 1:
    */
   {
      /* Disable combiner stage
       */
      (RADEON_ALPHA_ARG_A_ZERO |
       RADEON_ALPHA_ARG_B_ZERO |
       RADEON_ALPHA_ARG_C_CURRENT_ALPHA |
       RADEON_BLEND_CTL_ADD |
       RADEON_SCALE_1X |
       RADEON_CLAMP_TX),

      /* GL_REPLACE = 0x00800600
       */
      (RADEON_ALPHA_ARG_A_ZERO |
       RADEON_ALPHA_ARG_B_ZERO |
       RADEON_ALPHA_ARG_C_T1_ALPHA |
       RADEON_BLEND_CTL_ADD |
       RADEON_SCALE_1X |
       RADEON_CLAMP_TX),

      /* GL_MODULATE = 0x00800061
       */
      (RADEON_ALPHA_ARG_A_CURRENT_ALPHA |
       RADEON_ALPHA_ARG_B_T1_ALPHA |
       RADEON_ALPHA_ARG_C_ZERO |
       RADEON_BLEND_CTL_ADD |
       RADEON_SCALE_1X |
       RADEON_CLAMP_TX),

      /* GL_DECAL = 0x00800100
       */
      (RADEON_ALPHA_ARG_A_ZERO |
       RADEON_ALPHA_ARG_B_ZERO |
       RADEON_ALPHA_ARG_C_CURRENT_ALPHA |
       RADEON_BLEND_CTL_ADD |
       RADEON_SCALE_1X |
       RADEON_CLAMP_TX),

      /* GL_BLEND = 0x00800061
       */
      (RADEON_ALPHA_ARG_A_CURRENT_ALPHA |
       RADEON_ALPHA_ARG_B_TFACTOR_ALPHA |
       RADEON_ALPHA_ARG_C_T1_ALPHA |
       RADEON_BLEND_CTL_BLEND |
       RADEON_SCALE_1X |
       RADEON_CLAMP_TX),

      /* GL_ADD = 0x00800061
       */
      (RADEON_ALPHA_ARG_A_CURRENT_ALPHA |
       RADEON_ALPHA_ARG_B_ZERO |
       RADEON_ALPHA_ARG_C_T1_ALPHA |
       RADEON_COMP_ARG_B |
       RADEON_BLEND_CTL_ADD |
       RADEON_SCALE_1X |
       RADEON_CLAMP_TX),
   },

   /* Unit 2:
    */
   {
      /* Disable combiner stage
       */
      (RADEON_ALPHA_ARG_A_ZERO |
       RADEON_ALPHA_ARG_B_ZERO |
       RADEON_ALPHA_ARG_C_CURRENT_ALPHA |
       RADEON_BLEND_CTL_ADD |
       RADEON_SCALE_1X |
       RADEON_CLAMP_TX),

      /* GL_REPLACE = 0x00800700
       */
      (RADEON_ALPHA_ARG_A_ZERO |
       RADEON_ALPHA_ARG_B_ZERO |
       RADEON_ALPHA_ARG_C_T2_ALPHA |
       RADEON_BLEND_CTL_ADD |
       RADEON_SCALE_1X |
       RADEON_CLAMP_TX),

      /* GL_MODULATE = 0x00800071
       */
      (RADEON_ALPHA_ARG_A_CURRENT_ALPHA |
       RADEON_ALPHA_ARG_B_T2_ALPHA |
       RADEON_ALPHA_ARG_C_ZERO |
       RADEON_BLEND_CTL_ADD |
       RADEON_SCALE_1X |
       RADEON_CLAMP_TX),

      /* GL_DECAL = 0x00800100
       */
      (RADEON_ALPHA_ARG_A_ZERO |
       RADEON_ALPHA_ARG_B_ZERO |
       RADEON_ALPHA_ARG_C_CURRENT_ALPHA |
       RADEON_BLEND_CTL_ADD |
       RADEON_SCALE_1X |
       RADEON_CLAMP_TX),

      /* GL_BLEND = 0x00800071
       */
      (RADEON_ALPHA_ARG_A_CURRENT_ALPHA |
       RADEON_ALPHA_ARG_B_TFACTOR_ALPHA |
       RADEON_ALPHA_ARG_C_T2_ALPHA |
       RADEON_BLEND_CTL_BLEND |
       RADEON_SCALE_1X |
       RADEON_CLAMP_TX),

      /* GL_ADD = 0x00800021
       */
      (RADEON_ALPHA_ARG_A_CURRENT_ALPHA |
       RADEON_ALPHA_ARG_B_ZERO |
       RADEON_ALPHA_ARG_C_T2_ALPHA |
       RADEON_COMP_ARG_B |
       RADEON_BLEND_CTL_ADD |
       RADEON_SCALE_1X |
       RADEON_CLAMP_TX),
   }
};


/* GL_EXT_texture_env_combine support
 */

/* The color tables have combine functions for GL_SRC_COLOR,
 * GL_ONE_MINUS_SRC_COLOR, GL_SRC_ALPHA and GL_ONE_MINUS_SRC_ALPHA.
 */
static GLuint radeon_texture_color[][RADEON_MAX_TEXTURE_UNITS] =
{
   {
      RADEON_COLOR_ARG_A_T0_COLOR,
      RADEON_COLOR_ARG_A_T1_COLOR,
      RADEON_COLOR_ARG_A_T2_COLOR
   },
   {
      RADEON_COLOR_ARG_A_T0_COLOR | RADEON_COMP_ARG_A,
      RADEON_COLOR_ARG_A_T1_COLOR | RADEON_COMP_ARG_A,
      RADEON_COLOR_ARG_A_T2_COLOR | RADEON_COMP_ARG_A
   },
   {
      RADEON_COLOR_ARG_A_T0_ALPHA,
      RADEON_COLOR_ARG_A_T1_ALPHA,
      RADEON_COLOR_ARG_A_T2_ALPHA
   },
   {
      RADEON_COLOR_ARG_A_T0_ALPHA | RADEON_COMP_ARG_A,
      RADEON_COLOR_ARG_A_T1_ALPHA | RADEON_COMP_ARG_A,
      RADEON_COLOR_ARG_A_T2_ALPHA | RADEON_COMP_ARG_A
   },
};

static GLuint radeon_tfactor_color[] =
{
   RADEON_COLOR_ARG_A_TFACTOR_COLOR,
   RADEON_COLOR_ARG_A_TFACTOR_COLOR | RADEON_COMP_ARG_A,
   RADEON_COLOR_ARG_A_TFACTOR_ALPHA,
   RADEON_COLOR_ARG_A_TFACTOR_ALPHA | RADEON_COMP_ARG_A
};

static GLuint radeon_primary_color[] =
{
   RADEON_COLOR_ARG_A_DIFFUSE_COLOR,
   RADEON_COLOR_ARG_A_DIFFUSE_COLOR | RADEON_COMP_ARG_A,
   RADEON_COLOR_ARG_A_DIFFUSE_ALPHA,
   RADEON_COLOR_ARG_A_DIFFUSE_ALPHA | RADEON_COMP_ARG_A
};

static GLuint radeon_previous_color[] =
{
   RADEON_COLOR_ARG_A_CURRENT_COLOR,
   RADEON_COLOR_ARG_A_CURRENT_COLOR | RADEON_COMP_ARG_A,
   RADEON_COLOR_ARG_A_CURRENT_ALPHA,
   RADEON_COLOR_ARG_A_CURRENT_ALPHA | RADEON_COMP_ARG_A
};

/* The alpha tables only have GL_SRC_ALPHA and GL_ONE_MINUS_SRC_ALPHA.
 */
static GLuint radeon_texture_alpha[][RADEON_MAX_TEXTURE_UNITS] =
{
   {
      RADEON_ALPHA_ARG_A_T0_ALPHA,
      RADEON_ALPHA_ARG_A_T1_ALPHA,
      RADEON_ALPHA_ARG_A_T2_ALPHA
   },
   {
      RADEON_ALPHA_ARG_A_T0_ALPHA | RADEON_COMP_ARG_A,
      RADEON_ALPHA_ARG_A_T1_ALPHA | RADEON_COMP_ARG_A,
      RADEON_ALPHA_ARG_A_T2_ALPHA | RADEON_COMP_ARG_A
   },
};

static GLuint radeon_tfactor_alpha[] =
{
   RADEON_ALPHA_ARG_A_TFACTOR_ALPHA,
   RADEON_ALPHA_ARG_A_TFACTOR_ALPHA | RADEON_COMP_ARG_A
};

static GLuint radeon_primary_alpha[] =
{
   RADEON_ALPHA_ARG_A_DIFFUSE_ALPHA,
   RADEON_ALPHA_ARG_A_DIFFUSE_ALPHA | RADEON_COMP_ARG_A
};

static GLuint radeon_previous_alpha[] =
{
   RADEON_ALPHA_ARG_A_CURRENT_ALPHA,
   RADEON_ALPHA_ARG_A_CURRENT_ALPHA | RADEON_COMP_ARG_A
};


/* Extract the arg from slot A, shift it into the correct argument slot
 * and set the corresponding complement bit.
 */
#define RADEON_COLOR_ARG( n, arg )					\
do {									\
   color_combine |=							\
      ((color_arg[n] & RADEON_COLOR_ARG_MASK)				\
       << RADEON_COLOR_ARG_##arg##_SHIFT);				\
   color_combine |=							\
      ((color_arg[n] >> RADEON_COMP_ARG_SHIFT)				\
       << RADEON_COMP_ARG_##arg##_SHIFT);				\
} while (0)

#define RADEON_ALPHA_ARG( n, arg )					\
do {									\
   alpha_combine |=							\
      ((alpha_arg[n] & RADEON_ALPHA_ARG_MASK)				\
       << RADEON_ALPHA_ARG_##arg##_SHIFT);				\
   alpha_combine |=							\
      ((alpha_arg[n] >> RADEON_COMP_ARG_SHIFT)				\
       << RADEON_COMP_ARG_##arg##_SHIFT);				\
} while (0)


/* ================================================================
 * Texture unit state management
 */

static void radeonUpdateTextureEnv( GLcontext *ctx, int unit )
{
   radeonContextPtr rmesa = RADEON_CONTEXT(ctx);
   int source = rmesa->tmu_source[unit];
   struct gl_texture_object *tObj;
   struct gl_texture_unit *texUnit;
   GLuint enabled;
   GLuint color_combine, alpha_combine;
   GLuint color_arg[3], alpha_arg[3];
   GLuint i, numColorArgs = 0, numAlphaArgs = 0;
   GLuint op;

   if ( RADEON_DEBUG & DEBUG_VERBOSE_MSG ) {
      fprintf( stderr, "%s( %p, %d )\n",
	       __FUNCTION__, ctx, unit );
   }

   enabled = (ctx->Texture.ReallyEnabled >> (source * 4)) & TEXTURE0_ANY;
   if ( enabled != TEXTURE0_2D && enabled != TEXTURE0_1D )
      return;

   /* Only update the hardware texture state if the texture is current,
    * complete and enabled.
    */
   texUnit = &ctx->Texture.Unit[source];
   tObj = texUnit->Current;
   if ( !tObj || !tObj->Complete )
      return;

   if ( ( tObj != texUnit->CurrentD[2] ) &&
	( tObj != texUnit->CurrentD[1] ) )
      return;

   /* Set the texture environment state.  Isn't this nice and clean?
    * The Radeon will automagically set the texture alpha to 0xff when
    * the texture format does not include an alpha component.  This
    * reduces the amount of special-casing we have to do, alpha-only
    * textures being a notable exception.
    */
   switch ( texUnit->EnvMode ) {
   case GL_REPLACE:
      switch ( tObj->Image[0]->Format ) {
      case GL_RGBA:
      case GL_RGB:
      case GL_LUMINANCE:
      case GL_LUMINANCE_ALPHA:
      case GL_INTENSITY:
	 color_combine = radeon_color_combine[unit][RADEON_REPLACE];
	 alpha_combine = radeon_alpha_combine[unit][RADEON_REPLACE];
	 break;
      case GL_ALPHA:
	 color_combine = radeon_color_combine[unit][RADEON_DISABLE];
	 alpha_combine = radeon_alpha_combine[unit][RADEON_REPLACE];
	 break;
      case GL_COLOR_INDEX:
      default:
	 return;
      }
      break;

   case GL_MODULATE:
      switch ( tObj->Image[0]->Format ) {
      case GL_RGBA:
      case GL_RGB:
      case GL_LUMINANCE:
      case GL_LUMINANCE_ALPHA:
      case GL_INTENSITY:
	 color_combine = radeon_color_combine[unit][RADEON_MODULATE];
	 alpha_combine = radeon_alpha_combine[unit][RADEON_MODULATE];
	 break;
      case GL_ALPHA:
	 color_combine = radeon_color_combine[unit][RADEON_DISABLE];
	 alpha_combine = radeon_alpha_combine[unit][RADEON_MODULATE];
	 break;
      case GL_COLOR_INDEX:
      default:
	 return;
      }
      break;

   case GL_DECAL:
      switch ( tObj->Image[0]->Format ) {
      case GL_RGBA:
      case GL_RGB:
	 color_combine = radeon_color_combine[unit][RADEON_DECAL];
	 alpha_combine = radeon_alpha_combine[unit][RADEON_DISABLE];
	 break;
      case GL_ALPHA:
      case GL_LUMINANCE:
      case GL_LUMINANCE_ALPHA:
      case GL_INTENSITY:
	 color_combine = radeon_color_combine[unit][RADEON_DISABLE];
	 alpha_combine = radeon_alpha_combine[unit][RADEON_DISABLE];
	 break;
      case GL_COLOR_INDEX:
      default:
	 return;
      }
      break;

   case GL_BLEND:
      switch ( tObj->Image[0]->Format ) {
      case GL_RGBA:
      case GL_RGB:
      case GL_LUMINANCE:
      case GL_LUMINANCE_ALPHA:
	 color_combine = radeon_color_combine[unit][RADEON_BLEND];
	 alpha_combine = radeon_alpha_combine[unit][RADEON_MODULATE];
	 break;
      case GL_ALPHA:
	 color_combine = radeon_color_combine[unit][RADEON_DISABLE];
	 alpha_combine = radeon_alpha_combine[unit][RADEON_MODULATE];
	 break;
      case GL_INTENSITY:
	 color_combine = radeon_color_combine[unit][RADEON_BLEND];
	 alpha_combine = radeon_alpha_combine[unit][RADEON_BLEND];
	 break;
      case GL_COLOR_INDEX:
      default:
	 return;
      }
      break;

   case GL_ADD:
      switch ( tObj->Image[0]->Format ) {
      case GL_RGBA:
      case GL_RGB:
      case GL_LUMINANCE:
      case GL_LUMINANCE_ALPHA:
	 color_combine = radeon_color_combine[unit][RADEON_ADD];
	 alpha_combine = radeon_alpha_combine[unit][RADEON_MODULATE];
	 break;
      case GL_ALPHA:
	 color_combine = radeon_color_combine[unit][RADEON_DISABLE];
	 alpha_combine = radeon_alpha_combine[unit][RADEON_MODULATE];
	 break;
      case GL_INTENSITY:
	 color_combine = radeon_color_combine[unit][RADEON_ADD];
	 alpha_combine = radeon_alpha_combine[unit][RADEON_ADD];
	 break;
      case GL_COLOR_INDEX:
      default:
	 return;
      }
      break;

   case GL_COMBINE_EXT:
      /* Step 0:
       * Calculate how many arguments we need to process.
       */
      switch ( texUnit->CombineModeRGB ) {
      case GL_REPLACE:
	 numColorArgs = 1;
	 break;
      case GL_MODULATE:
      case GL_ADD:
      case GL_ADD_SIGNED_EXT:
      case GL_DOT3_RGB_EXT:
      case GL_DOT3_RGBA_EXT:
	 numColorArgs = 2;
	 break;
      case GL_INTERPOLATE_EXT:
	 numColorArgs = 3;
	 break;
      default:
	 return;
      }

      switch ( texUnit->CombineModeA ) {
      case GL_REPLACE:
	 numAlphaArgs = 1;
	 break;
      case GL_MODULATE:
      case GL_ADD:
      case GL_ADD_SIGNED_EXT:
	 numAlphaArgs = 2;
	 break;
      case GL_INTERPOLATE_EXT:
	 numAlphaArgs = 3;
	 break;
      default:
	 return;
      }

      /* Step 1:
       * Extract the color and alpha combine function arguments.
       */
      for ( i = 0 ; i < numColorArgs ; i++ ) {
	 op = texUnit->CombineOperandRGB[i] - GL_SRC_COLOR;
	 switch ( texUnit->CombineSourceRGB[i] ) {
	 case GL_TEXTURE:
	    color_arg[i] = radeon_texture_color[op][unit];
	    break;
	 case GL_CONSTANT_EXT:
	    color_arg[i] = radeon_tfactor_color[op];
	    break;
	 case GL_PRIMARY_COLOR_EXT:
	    color_arg[i] = radeon_primary_color[op];
	    break;
	 case GL_PREVIOUS_EXT:
	    color_arg[i] = radeon_previous_color[op];
	    break;
	 default:
	    return;
	 }
      }

      for ( i = 0 ; i < numAlphaArgs ; i++ ) {
	 op = texUnit->CombineOperandA[i] - GL_SRC_ALPHA;
	 switch ( texUnit->CombineSourceA[i] ) {
	 case GL_TEXTURE:
	    alpha_arg[i] = radeon_texture_alpha[op][unit];
	    break;
	 case GL_CONSTANT_EXT:
	    alpha_arg[i] = radeon_tfactor_alpha[op];
	    break;
	 case GL_PRIMARY_COLOR_EXT:
	    alpha_arg[i] = radeon_primary_alpha[op];
	    break;
	 case GL_PREVIOUS_EXT:
	    alpha_arg[i] = radeon_previous_alpha[op];
	    break;
	 default:
	    return;
	 }
      }

      /* Step 2:
       * Build up the color and alpha combine functions.
       */
      switch ( texUnit->CombineModeRGB ) {
      case GL_REPLACE:
	 color_combine = (RADEON_COLOR_ARG_A_ZERO |
			  RADEON_COLOR_ARG_B_ZERO |
			  RADEON_BLEND_CTL_ADD |
			  RADEON_CLAMP_TX);
	 RADEON_COLOR_ARG( 0, C );
	 break;
      case GL_MODULATE:
	 color_combine = (RADEON_COLOR_ARG_C_ZERO |
			  RADEON_BLEND_CTL_ADD |
			  RADEON_CLAMP_TX);
	 RADEON_COLOR_ARG( 0, A );
	 RADEON_COLOR_ARG( 1, B );
	 break;
      case GL_ADD:
	 color_combine = (RADEON_COLOR_ARG_B_ZERO |
			  RADEON_COMP_ARG_B |
			  RADEON_BLEND_CTL_ADD |
			  RADEON_CLAMP_TX);
	 RADEON_COLOR_ARG( 0, A );
	 RADEON_COLOR_ARG( 1, C );
	 break;
      case GL_ADD_SIGNED_EXT:
	 color_combine = (RADEON_COLOR_ARG_B_ZERO |
			  RADEON_COMP_ARG_B |
			  RADEON_BLEND_CTL_ADDSIGNED |
			  RADEON_CLAMP_TX);
	 RADEON_COLOR_ARG( 0, A );
	 RADEON_COLOR_ARG( 1, C );
	 break;
      case GL_INTERPOLATE_EXT:
	 color_combine = (RADEON_BLEND_CTL_BLEND |
			  RADEON_CLAMP_TX);
	 RADEON_COLOR_ARG( 0, B );
	 RADEON_COLOR_ARG( 1, A );
	 RADEON_COLOR_ARG( 2, C );
	 break;
      case GL_DOT3_RGB_EXT:
      case GL_DOT3_RGBA_EXT:
	 color_combine = (RADEON_COLOR_ARG_C_ZERO |
			  RADEON_BLEND_CTL_DOT3 |
			  RADEON_CLAMP_TX);
	 RADEON_COLOR_ARG( 0, A );
	 RADEON_COLOR_ARG( 1, B );
	 break;
      default:
	 return;
      }

      switch ( texUnit->CombineModeA ) {
      case GL_REPLACE:
	 alpha_combine = (RADEON_ALPHA_ARG_A_ZERO |
			  RADEON_ALPHA_ARG_B_ZERO |
			  RADEON_BLEND_CTL_ADD |
			  RADEON_CLAMP_TX);
	 RADEON_ALPHA_ARG( 0, C );
	 break;
      case GL_MODULATE:
	 alpha_combine = (RADEON_ALPHA_ARG_C_ZERO |
			  RADEON_BLEND_CTL_ADD |
			  RADEON_CLAMP_TX);
	 RADEON_ALPHA_ARG( 0, A );
	 RADEON_ALPHA_ARG( 1, B );
	 break;
      case GL_ADD:
	 alpha_combine = (RADEON_ALPHA_ARG_B_ZERO |
			  RADEON_COMP_ARG_B |
			  RADEON_BLEND_CTL_ADD |
			  RADEON_CLAMP_TX);
	 RADEON_ALPHA_ARG( 0, A );
	 RADEON_ALPHA_ARG( 1, C );
	 break;
      case GL_ADD_SIGNED_EXT:
	 alpha_combine = (RADEON_ALPHA_ARG_B_ZERO |
			  RADEON_COMP_ARG_B |
			  RADEON_BLEND_CTL_ADDSIGNED |
			  RADEON_CLAMP_TX);
	 RADEON_ALPHA_ARG( 0, A );
	 RADEON_ALPHA_ARG( 1, C );
	 break;
      case GL_INTERPOLATE_EXT:
	 alpha_combine = (RADEON_BLEND_CTL_BLEND |
			  RADEON_CLAMP_TX);
	 RADEON_ALPHA_ARG( 0, B );
	 RADEON_ALPHA_ARG( 1, A );
	 RADEON_ALPHA_ARG( 2, C );
	 break;
      default:
	 return;
      }

      if ( texUnit->CombineModeRGB == GL_DOT3_RGB_EXT ) {
	 alpha_combine |= RADEON_DOT_ALPHA_DONT_REPLICATE;
      }

      /* Step 3:
       * Apply the scale factor.  The EXT extension has a somewhat
       * unnecessary restriction that the scale must be 4x.  The ARB
       * extension will likely drop this and we can just apply the
       * scale factors regardless.
       */
      if ( texUnit->CombineModeRGB != GL_DOT3_RGB_EXT &&
	   texUnit->CombineModeRGB != GL_DOT3_RGBA_EXT ) {
	 color_combine |= (texUnit->CombineScaleShiftRGB << 21);
	 alpha_combine |= (texUnit->CombineScaleShiftA << 21);
      } else {
	 color_combine |= RADEON_SCALE_4X;
	 alpha_combine |= RADEON_SCALE_4X;
      }

      /* All done!
       */
      break;

   default:
      return;
   }

   rmesa->color_combine[source] = color_combine;
   rmesa->alpha_combine[source] = alpha_combine;
}

static void radeonUpdateTextureObject( GLcontext *ctx, int unit )
{
   radeonContextPtr rmesa = RADEON_CONTEXT(ctx);
   int source = rmesa->tmu_source[unit];
   struct gl_texture_object *tObj;
   radeonTexObjPtr t;
   GLuint enabled;

   if ( RADEON_DEBUG & DEBUG_VERBOSE_MSG ) {
      fprintf( stderr, "%s( %p, %d )\n",
	       __FUNCTION__, ctx, unit );
   }

   enabled = (ctx->Texture.ReallyEnabled >> (source * 4)) & TEXTURE0_ANY;
   if ( enabled != TEXTURE0_2D && enabled != TEXTURE0_1D ) {
      if ( enabled )
	 rmesa->Fallback |= RADEON_FALLBACK_TEXTURE;
      return;
   }

   /* Only update the hardware texture state if the texture is current,
    * complete and enabled.
    */
   tObj = ctx->Texture.Unit[source].Current;
   if ( !tObj || !tObj->Complete )
      return;

   if ( ( tObj != ctx->Texture.Unit[source].CurrentD[2] ) &&
	( tObj != ctx->Texture.Unit[source].CurrentD[1] ) )
      return;

   /* We definately have a valid texture now */
   t = tObj->DriverData;

   /* Force the texture unit state to be loaded into the hardware */
   rmesa->dirty |= RADEON_UPLOAD_CONTEXT | (RADEON_UPLOAD_TEX0 << unit);

   /* Force any texture images to be loaded into the hardware */
   if ( t->dirty_images ) {
      if ( RADEON_DEBUG & DEBUG_VERBOSE_API ) {
	 fprintf( stderr, "   t->dirty_images = 0x%x\n", t->dirty_images );
      }
      radeonSetTexImages( rmesa, tObj );
      rmesa->dirty |= (RADEON_UPLOAD_TEX0IMAGES << unit);
   }

   if ( t->memBlock )
      radeonUpdateTexLRU( rmesa, t );

   switch ( unit ) {
   case 0:
      rmesa->setup.pp_cntl |= (RADEON_TEX_0_ENABLE |
			       RADEON_TEX_BLEND_0_ENABLE);
      break;
   case 1:
      rmesa->setup.pp_cntl |= (RADEON_TEX_1_ENABLE |
			       RADEON_TEX_BLEND_1_ENABLE);
      break;
   }
}

void radeonUpdateTextureState( GLcontext *ctx )
{
   radeonContextPtr rmesa = RADEON_CONTEXT(ctx);

   if ( RADEON_DEBUG & DEBUG_VERBOSE_API ) {
      fprintf( stderr, "%s( %p ) en=0x%x\n",
	       __FUNCTION__, ctx, ctx->Texture.ReallyEnabled );
   }

   /* Clear any texturing fallbacks */
   rmesa->Fallback &= ~RADEON_FALLBACK_TEXTURE;

   /* Disable all texturing until it is known to be good */
   rmesa->setup.pp_cntl &= ~(RADEON_TEX_ENABLE_MASK |
			     RADEON_TEX_BLEND_ENABLE_MASK);

   radeonUpdateTextureObject( ctx, 0 );
   radeonUpdateTextureEnv( ctx, 0 );

   if ( rmesa->multitex ) {
      radeonUpdateTextureObject( ctx, 1 );
      radeonUpdateTextureEnv( ctx, 1 );
   }

   rmesa->dirty |= RADEON_UPLOAD_CONTEXT;
}

/* $XFree86: xc/lib/GL/mesa/src/drv/radeon/radeon_tex.c,v 1.4 2001/05/21 21:43:52 dawes Exp $ */
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
#include "texutil.h"


static void radeonSetTexWrap( radeonTexObjPtr t, GLenum swrap, GLenum twrap )
{
   t->pp_txfilter &= ~(RADEON_CLAMP_S_MASK | RADEON_CLAMP_T_MASK);

   switch ( swrap ) {
   case GL_REPEAT:
      t->pp_txfilter |= RADEON_CLAMP_S_WRAP;
      break;
   case GL_CLAMP:
      t->pp_txfilter |= RADEON_CLAMP_S_CLAMP_LAST;
      break;
   case GL_CLAMP_TO_EDGE:
      t->pp_txfilter |= RADEON_CLAMP_S_CLAMP_LAST;
      break;
   }

   switch ( twrap ) {
   case GL_REPEAT:
      t->pp_txfilter |= RADEON_CLAMP_T_WRAP;
      break;
   case GL_CLAMP:
      t->pp_txfilter |= RADEON_CLAMP_T_CLAMP_LAST;
      break;
   case GL_CLAMP_TO_EDGE:
      t->pp_txfilter |= RADEON_CLAMP_T_CLAMP_LAST;
      break;
   }
}

static void radeonSetTexFilter( radeonTexObjPtr t, GLenum minf, GLenum magf )
{
   t->pp_txfilter &= ~(RADEON_MIN_FILTER_MASK | RADEON_MAG_FILTER_MASK);

   switch ( minf ) {
   case GL_NEAREST:
      t->pp_txfilter |= RADEON_MIN_FILTER_NEAREST;
      break;
   case GL_LINEAR:
      t->pp_txfilter |= RADEON_MIN_FILTER_LINEAR;
      break;
   case GL_NEAREST_MIPMAP_NEAREST:
      t->pp_txfilter |= RADEON_MIN_FILTER_NEAREST_MIP_NEAREST;
      break;
   case GL_NEAREST_MIPMAP_LINEAR:
      t->pp_txfilter |= RADEON_MIN_FILTER_LINEAR_MIP_NEAREST;
      break;
   case GL_LINEAR_MIPMAP_NEAREST:
      t->pp_txfilter |= RADEON_MIN_FILTER_NEAREST_MIP_LINEAR;
      break;
   case GL_LINEAR_MIPMAP_LINEAR:
      t->pp_txfilter |= RADEON_MIN_FILTER_LINEAR_MIP_LINEAR;
      break;
   }

   switch ( magf ) {
   case GL_NEAREST:
      t->pp_txfilter |= RADEON_MAG_FILTER_NEAREST;
      break;
   case GL_LINEAR:
      t->pp_txfilter |= RADEON_MAG_FILTER_LINEAR;
      break;
   }
}

static void radeonSetTexBorderColor( radeonTexObjPtr t, GLubyte c[4] )
{
   t->pp_border_color = radeonPackColor( 4, c[0], c[1], c[2], c[3] );
}

static radeonTexObjPtr radeonAllocTexObj( struct gl_texture_object *texObj )
{
   radeonTexObjPtr t;

   t = CALLOC_STRUCT( radeon_tex_obj );

   if ( RADEON_DEBUG & DEBUG_VERBOSE_API ) {
      fprintf( stderr, "%s( %p, %p )\n", __FUNCTION__, texObj, t );
   }

   /* Initialize non-image-dependent parts of the state:
    */
   t->tObj = texObj;
#if 0
   t->dirty_images = ~0;
#endif
   t->pp_txfilter = RADEON_BORDER_MODE_OGL;
   t->pp_txformat = (RADEON_TXFORMAT_ENDIAN_NO_SWAP |
		     RADEON_TXFORMAT_PERSPECTIVE_ENABLE);

   make_empty_list( t );

   radeonSetTexWrap( t, texObj->WrapS, texObj->WrapT );
   radeonSetTexFilter( t, texObj->MinFilter, texObj->MagFilter );
   radeonSetTexBorderColor( t, texObj->BorderColor );

   return t;
}


static GLint radeonChooseTexFormat( radeonContextPtr rmesa,
				    struct gl_texture_image *texImage,
				    GLenum format, GLenum type )
{
   const GLboolean do32bpt = ( rmesa->radeonScreen->cpp == 4 );
   const struct gl_texture_format *texFormat;
   GLint ret;

   if ( 0 )
      fprintf( stderr, "internal=%s format=%s type=%s\n",
	       texImage->IntFormat == 3 ? "GL_RGB (3)" :
	       texImage->IntFormat == 4 ? "GL_RGBA (4)" :
	       gl_lookup_enum_by_nr( texImage->IntFormat ),
	       gl_lookup_enum_by_nr( format ),
	       gl_lookup_enum_by_nr( type ) );

#define SET_FORMAT( r, gl )						\
   do {									\
      ret = (r);							\
      texFormat = &(gl);						\
   } while (0)

#define SET_FORMAT_32BPT( r32, gl32, r16, gl16 )			\
   do {									\
      if ( do32bpt ) {							\
	 ret = (r32);							\
	 texFormat = &(gl32);						\
      } else {								\
	 ret = (r16);							\
	 texFormat = &(gl16);						\
      }									\
   } while (0)

   switch ( texImage->IntFormat ) {
      /* GH: Bias towards GL_RGB, GL_RGBA texture formats.  This has
       * got to be better than sticking them way down the end of this
       * huge list.
       */
   case GL_RGBA:
   case 4:
      if ( format == GL_BGRA ) {
	 if ( type == GL_UNSIGNED_INT_8_8_8_8_REV ) {
	    SET_FORMAT( RADEON_TXFORMAT_ARGB8888, _mesa_texformat_argb8888 );
	    break;
	 } else if ( type == GL_UNSIGNED_SHORT_4_4_4_4_REV ) {
	    SET_FORMAT( RADEON_TXFORMAT_ARGB4444, _mesa_texformat_argb4444 );
	    break;
	 } else if ( type == GL_UNSIGNED_SHORT_1_5_5_5_REV ) {
	    SET_FORMAT( RADEON_TXFORMAT_ARGB1555, _mesa_texformat_argb1555 );
	    break;
	 }
      }
      SET_FORMAT_32BPT( RADEON_TXFORMAT_RGBA8888, _mesa_texformat_rgba8888,
			RADEON_TXFORMAT_ARGB4444, _mesa_texformat_argb4444 );
      break;

   case GL_RGB:
   case 3:
      if ( format == GL_RGB && type == GL_UNSIGNED_SHORT_5_6_5 ) {
	 SET_FORMAT( RADEON_TXFORMAT_RGB565, _mesa_texformat_rgb565 );
	 break;
      }
      SET_FORMAT_32BPT( RADEON_TXFORMAT_RGBA8888, _mesa_texformat_rgba8888,
			RADEON_TXFORMAT_RGB565,   _mesa_texformat_rgb565 );
      break;

      /* GH: Okay, keep checking as normal.  Still test for GL_RGB,
       * GL_RGBA formats first.
       */
   case GL_RGBA8:
   case GL_RGB10_A2:
   case GL_RGBA12:
   case GL_RGBA16:
      SET_FORMAT_32BPT( RADEON_TXFORMAT_RGBA8888, _mesa_texformat_rgba8888,
			RADEON_TXFORMAT_ARGB4444, _mesa_texformat_argb4444 );
      break;

   case GL_RGBA4:
   case GL_RGBA2:
      SET_FORMAT( RADEON_TXFORMAT_ARGB4444, _mesa_texformat_argb4444 );
      break;

   case GL_RGB5_A1:
      SET_FORMAT( RADEON_TXFORMAT_ARGB1555, _mesa_texformat_argb1555 );
      break;

   case GL_RGB8:
   case GL_RGB10:
   case GL_RGB12:
   case GL_RGB16:
      SET_FORMAT_32BPT( RADEON_TXFORMAT_RGBA8888, _mesa_texformat_rgba8888,
			RADEON_TXFORMAT_RGB565,   _mesa_texformat_rgb565 );
      break;

   case GL_RGB5:
   case GL_RGB4:
   case GL_R3_G3_B2:
      SET_FORMAT( RADEON_TXFORMAT_RGB565, _mesa_texformat_rgb565 );
      break;

   case GL_ALPHA:
   case GL_ALPHA4:
   case GL_ALPHA8:
   case GL_ALPHA12:
   case GL_ALPHA16:
      SET_FORMAT( RADEON_TXFORMAT_AI88, _mesa_texformat_al88 );
      break;

   case 1:
   case GL_LUMINANCE:
   case GL_LUMINANCE4:
   case GL_LUMINANCE8:
   case GL_LUMINANCE12:
   case GL_LUMINANCE16:
      SET_FORMAT( RADEON_TXFORMAT_AI88, _mesa_texformat_al88 );
      break;

   case 2:
   case GL_LUMINANCE_ALPHA:
   case GL_LUMINANCE4_ALPHA4:
   case GL_LUMINANCE6_ALPHA2:
   case GL_LUMINANCE8_ALPHA8:
   case GL_LUMINANCE12_ALPHA4:
   case GL_LUMINANCE12_ALPHA12:
   case GL_LUMINANCE16_ALPHA16:
      SET_FORMAT( RADEON_TXFORMAT_AI88, _mesa_texformat_al88 );
      break;

   case GL_INTENSITY:
   case GL_INTENSITY4:
   case GL_INTENSITY8:
   case GL_INTENSITY12:
   case GL_INTENSITY16:
      SET_FORMAT( RADEON_TXFORMAT_I8, _mesa_texformat_i8 );
      break;

   case GL_COLOR_INDEX:
   case GL_COLOR_INDEX1_EXT:
   case GL_COLOR_INDEX2_EXT:
   case GL_COLOR_INDEX4_EXT:
   case GL_COLOR_INDEX8_EXT:
   case GL_COLOR_INDEX12_EXT:
   case GL_COLOR_INDEX16_EXT:
   default:
      fprintf( stderr, "bad texture format in radeonChooseTexFormat() %d",
	       texImage->IntFormat );
      return -1;
   }

   texImage->TexFormat = texFormat;

   return ret;
}


/* ================================================================
 * Texture image callbacks
 */

static GLboolean
radeonDDTexImage1D( GLcontext *ctx, GLenum target, GLint level,
		    GLenum format, GLenum type, const GLvoid *pixels,
		    const struct gl_pixelstore_attrib *packing,
		    struct gl_texture_object *texObj,
		    struct gl_texture_image *texImage,
		    GLboolean *retainInternalCopy )
{
   radeonContextPtr rmesa = RADEON_CONTEXT(ctx);
   radeonTexObjPtr t = (radeonTexObjPtr)texObj->DriverData;
   const struct gl_texture_format *texFormat;
   GLuint texSize;
   GLint txformat;
   GLubyte *data;

   if ( target != GL_TEXTURE_1D )
      return GL_FALSE;

   if ( RADEON_DEBUG & DEBUG_VERBOSE_API ) {
      fprintf( stderr, "%s( %p, %d )\n", __FUNCTION__, texObj, level );
   }

   if ( t ) {
      if ( t->bound ) FLUSH_BATCH( rmesa );
      radeonSwapOutTexObj( rmesa, t );
   } else {
      t = radeonAllocTexObj( texObj );
      texObj->DriverData = t;
   }

   txformat = radeonChooseTexFormat( rmesa, texImage, format, type );
   if ( txformat < 0 )
      return GL_FALSE;

   texFormat = texImage->TexFormat;
   texSize = texImage->Width * texFormat->TexelBytes;

   /* We really shouldn't have to keep the texture image, it should be
    * hung from the main texImage structure.
    */
   if ( t->image[level].data ) {
      FREE( t->image[level].data );
      t->image[level].data = NULL;
   }

   data = (GLubyte *) MALLOC( texSize );
   if ( !data )
      return GL_FALSE;

   if ( !_mesa_convert_texsubimage1d( texFormat->IntFormat,
				      0, texImage->Width,
				      format, type, packing,
				      pixels, data ) ) {
      /*fprintf( stderr, "   *** convert failed!\n" );*/
      FREE( data );
      return GL_FALSE;
   }

   t->image[level].data = data;
   t->dirty_images |= (1 << level);

   /* Format-specific hardware state:
    */
   t->pp_txformat &= ~(RADEON_TXFORMAT_FORMAT_MASK |
		       RADEON_TXFORMAT_ALPHA_IN_MAP);
   t->pp_txformat |= txformat;

   if ( txformat == RADEON_TXFORMAT_RGBA8888 ||
	txformat == RADEON_TXFORMAT_ARGB4444 ||
	txformat == RADEON_TXFORMAT_ARGB1555 ||
	txformat == RADEON_TXFORMAT_AI88 ) {
      t->pp_txformat |= RADEON_TXFORMAT_ALPHA_IN_MAP;
   }

   ASSERT( t->image[level].data );

   rmesa->new_state |= RADEON_NEW_TEXTURE;

   *retainInternalCopy = GL_FALSE;
   return GL_TRUE;
}

static GLboolean
radeonDDTexImage2D( GLcontext *ctx, GLenum target, GLint level,
		    GLenum format, GLenum type, const GLvoid *pixels,
		    const struct gl_pixelstore_attrib *packing,
		    struct gl_texture_object *texObj,
		    struct gl_texture_image *texImage,
		    GLboolean *retainInternalCopy )
{
   radeonContextPtr rmesa = RADEON_CONTEXT(ctx);
   radeonTexObjPtr t = (radeonTexObjPtr)texObj->DriverData;
   const struct gl_texture_format *texFormat;
   GLuint texSize;
   GLint txformat;
   GLubyte *data;

   if ( target != GL_TEXTURE_2D )
      return GL_FALSE;

   if ( RADEON_DEBUG & DEBUG_VERBOSE_API ) {
      fprintf( stderr, "%s( %p, %d )\n", __FUNCTION__, texObj, level );
   }

   if ( t ) {
      if ( t->bound ) FLUSH_BATCH( rmesa );
      if ( t->image[level].data ) radeonSwapOutTexObj( rmesa, t );
   } else {
      t = radeonAllocTexObj( texObj );
      texObj->DriverData = t;
   }

   txformat = radeonChooseTexFormat( rmesa, texImage, format, type );
   if ( txformat < 0 )
      return GL_FALSE;

   texFormat = texImage->TexFormat;
   texSize = texImage->Width * texImage->Height * texFormat->TexelBytes;

   /* We really shouldn't have to keep the texture image, it should be
    * hung from the main texImage structure.
    */
   if ( t->image[level].data ) {
      FREE( t->image[level].data );
      t->image[level].data = NULL;
   }

   data = (GLubyte *) MALLOC( texSize );
   if ( !data )
      return GL_FALSE;

   if ( !_mesa_convert_texsubimage2d( texFormat->IntFormat,
				      0, 0, texImage->Width, texImage->Height,
				      texImage->Width, format, type, packing,
				      pixels, data ) ) {
      if ( 0 )
	 fprintf( stderr, "   *** convert failed!  %s/%s-> %s\n",
		  gl_lookup_enum_by_nr( format ),
		  gl_lookup_enum_by_nr( type ),
		  gl_lookup_enum_by_nr( texImage->IntFormat ) );
      FREE( data );
      return GL_FALSE;
   }

   t->image[level].data = data;
   t->dirty_images |= (1 << level);

   /* Format-specific hardware state:
    */
   t->pp_txformat &= ~(RADEON_TXFORMAT_FORMAT_MASK |
		       RADEON_TXFORMAT_ALPHA_IN_MAP);
   t->pp_txformat |= txformat;

   if ( txformat == RADEON_TXFORMAT_RGBA8888 ||
	txformat == RADEON_TXFORMAT_ARGB4444 ||
	txformat == RADEON_TXFORMAT_ARGB1555 ||
	txformat == RADEON_TXFORMAT_AI88 ) {
      t->pp_txformat |= RADEON_TXFORMAT_ALPHA_IN_MAP;
   }

   ASSERT( t->image[level].data );

   rmesa->new_state |= RADEON_NEW_TEXTURE;

   *retainInternalCopy = GL_FALSE;
   return GL_TRUE;
}

/* GH: This is undoubtedly broken...
 */
static GLboolean
radeonDDTexImage3D( GLcontext *ctx, GLenum target, GLint level,
		    GLenum format, GLenum type, const GLvoid *pixels,
		    const struct gl_pixelstore_attrib *packing,
		    struct gl_texture_object *texObj,
		    struct gl_texture_image *texImage,
		    GLboolean *retainInternalCopy )
{
   radeonContextPtr rmesa = RADEON_CONTEXT(ctx);
   radeonTexObjPtr t = (radeonTexObjPtr)texObj->DriverData;
   const struct gl_texture_format *texFormat;
   GLuint texSize;
   GLint txformat;
   GLubyte *data;

   if ( target != GL_TEXTURE_3D )
      return GL_FALSE;

   if ( t ) {
      if ( t->bound ) FLUSH_BATCH( rmesa );
      radeonSwapOutTexObj( rmesa, t );
   } else {
      t = radeonAllocTexObj( texObj );
      texObj->DriverData = t;
   }

   txformat = radeonChooseTexFormat( rmesa, texImage, format, type );
   if ( txformat < 0 )
      return GL_FALSE;

   texFormat = texImage->TexFormat;
   texSize = (texImage->Width * texImage->Height *
	      texImage->Depth * texFormat->TexelBytes);

   /* We really shouldn't have to keep the texture image, it should be
    * hung from the main texImage structure.
    */
   if ( t->image[level].data ) {
      FREE( t->image[level].data );
      t->image[level].data = NULL;
   }

   data = (GLubyte *) MALLOC( texSize );
   if ( !data )
      return GL_FALSE;

   if ( !_mesa_convert_texsubimage3d( texFormat->IntFormat,
				      0, 0, 0, texImage->Width,
				      texImage->Height, texImage->Depth,
				      texImage->Width, texImage->Height,
				      format, type, packing,
				      pixels, data ) ) {
      FREE( data );
      return GL_FALSE;
   }

   t->image[level].data = data;
   t->dirty_images |= (1 << level);

   ASSERT( t->image[level].data );

   rmesa->new_state |= RADEON_NEW_TEXTURE;

   *retainInternalCopy = GL_FALSE;
   return GL_TRUE;
}


/* ================================================================
 * Texture subimage callbacks
 */

static GLboolean
radeonDDTexSubImage1D( GLcontext *ctx, GLenum target, GLint level,
		       GLint xoffset, GLsizei width,
		       GLenum format, GLenum type,
		       const GLvoid *pixels,
		       const struct gl_pixelstore_attrib *packing,
		       struct gl_texture_object *texObj,
		       struct gl_texture_image *texImage )
{
   radeonContextPtr rmesa = RADEON_CONTEXT(ctx);
   radeonTexObjPtr t = (radeonTexObjPtr)texObj->DriverData;
   const struct gl_texture_format *texFormat;

   if ( target != GL_TEXTURE_1D )
      return GL_FALSE;

   /* FIXME: Can this ever be NULL???
    */
   ASSERT( t );
   ASSERT( t->image[level].data );

   if ( t->bound )
      FLUSH_BATCH( rmesa );

   texFormat = texImage->TexFormat;

   if ( !_mesa_convert_texsubimage1d( texFormat->IntFormat,
				      xoffset, width, format, type, packing,
				      pixels, t->image[level].data ) ) {
      /*fprintf( stderr, "   *** convert failed!\n" );*/
      return GL_FALSE;
   }

   t->dirty_images |= (1 << level);
   rmesa->new_state |= RADEON_NEW_TEXTURE;

   return GL_TRUE;
}

static GLboolean
radeonDDTexSubImage2D( GLcontext *ctx, GLenum target, GLint level,
		       GLint xoffset, GLint yoffset,
		       GLsizei width, GLsizei height,
		       GLenum format, GLenum type,
		       const GLvoid *pixels,
		       const struct gl_pixelstore_attrib *packing,
		       struct gl_texture_object *texObj,
		       struct gl_texture_image *texImage )
{
   radeonContextPtr rmesa = RADEON_CONTEXT(ctx);
   radeonTexObjPtr t = (radeonTexObjPtr)texObj->DriverData;
   const struct gl_texture_format *texFormat;

   if ( target != GL_TEXTURE_2D )
      return GL_FALSE;

   if ( RADEON_DEBUG & DEBUG_VERBOSE_API ) {
      fprintf( stderr, "%s( %p, %d )\n", __FUNCTION__, texObj, level );
   }

   /* FIXME: Can this ever be NULL???
    */
   ASSERT( t );
   ASSERT( t->image[level].data );

   if ( t->bound )
      FLUSH_BATCH( rmesa );

   texFormat = texImage->TexFormat;

   if ( !_mesa_convert_texsubimage2d( texFormat->IntFormat,
				      xoffset, yoffset, width, height,
				      texImage->Width, format, type, packing,
				      pixels, t->image[level].data ) ) {
      /*fprintf( stderr, "   *** convert failed!\n" );*/
      return GL_FALSE;
   }

   t->dirty_images |= (1 << level);
   rmesa->new_state |= RADEON_NEW_TEXTURE;

   return GL_TRUE;
}

/* GH: This is undoubtedly broken...
 */
static GLboolean
radeonDDTexSubImage3D( GLcontext *ctx, GLenum target, GLint level,
		       GLint xoffset, GLint yoffset, GLint zoffset,
		       GLsizei width, GLsizei height, GLint depth,
		       GLenum format, GLenum type,
		       const GLvoid *pixels,
		       const struct gl_pixelstore_attrib *packing,
		       struct gl_texture_object *texObj,
		       struct gl_texture_image *texImage )
{
   radeonContextPtr rmesa = RADEON_CONTEXT(ctx);
   radeonTexObjPtr t = (radeonTexObjPtr)texObj->DriverData;
   const struct gl_texture_format *texFormat;

   if ( target != GL_TEXTURE_3D )
      return GL_FALSE;

   /* FIXME: Can this ever be NULL???
    */
   ASSERT( t );
   ASSERT( t->image[level].data );

   if ( t->bound )
      FLUSH_BATCH( rmesa );

   texFormat = texImage->TexFormat;

   if ( !_mesa_convert_texsubimage3d( texFormat->IntFormat,
				      xoffset, yoffset, zoffset,
				      width, height, depth,
				      texImage->Width, texImage->Height,
				      format, type, packing,
				      pixels, t->image[level].data ) ) {
      /*fprintf( stderr, "   *** convert failed!\n" );*/
      return GL_FALSE;
   }

   t->dirty_images |= (1 << level);
   rmesa->new_state |= RADEON_NEW_TEXTURE;

   return GL_TRUE;
}


/* ================================================================
 * DEPRECATED...
 */

static GLvoid *radeonDDGetTexImage( GLcontext *ctx, GLenum target, GLint level,
				    const struct gl_texture_object *texObj,
				    GLenum *formatOut, GLenum *typeOut,
				    GLboolean *freeImageOut )
{
   const struct gl_texture_image *texImage = texObj->Image[level];
   const struct gl_texture_format *texFormat = texImage->TexFormat;
   radeonTexObjPtr t = (radeonTexObjPtr)texObj->DriverData;
   GLubyte *data;

   if ( !t || !t->image[level].data )
      return NULL;

   data = (GLubyte *) MALLOC( texImage->Width * texImage->Height * 4 );
   if ( !data )
      return NULL;

   if ( 0 )
      fprintf( stderr, "   in=%d out=%s\n",
	       texFormat->IntFormat,
	       gl_lookup_enum_by_nr( texImage->Format ) );

   switch ( target ) {
   case GL_TEXTURE_1D:
      _mesa_unconvert_teximage1d( texFormat->IntFormat, texImage->Format,
				  texImage->Width,
				  t->image[level].data, data );
      break;
   case GL_TEXTURE_2D:
      _mesa_unconvert_teximage2d( texFormat->IntFormat, texImage->Format,
				  texImage->Width, texImage->Height,
				  t->image[level].data, data );
      break;
   default:
      return NULL;
   }

   *formatOut = texImage->Format;
   *typeOut = GL_UNSIGNED_BYTE;
   *freeImageOut = GL_TRUE;

   return data;
}


/* ================================================================
 * Texture state callbacks
 */

#define SCALED_FLOAT_TO_BYTE( x, scale ) \
		((((GLint)((256.0F / scale) * (x))) - 1) / 2)

static void radeonDDTexEnv( GLcontext *ctx, GLenum target,
			    GLenum pname, const GLfloat *param )
{
   radeonContextPtr rmesa = RADEON_CONTEXT(ctx);
   struct gl_texture_unit *texUnit;
   GLuint source;
   GLubyte c[4];
   GLuint col;
   GLfloat bias;
   GLubyte b;

   if ( RADEON_DEBUG & DEBUG_VERBOSE_API ) {
      fprintf( stderr, "%s( %s )\n",
	       __FUNCTION__, gl_lookup_enum_by_nr( pname ) );
   }

   switch ( pname ) {
   case GL_TEXTURE_ENV_MODE:
      FLUSH_BATCH( rmesa );
      rmesa->new_state |= RADEON_NEW_TEXTURE | RADEON_NEW_ALPHA;
      break;

   case GL_TEXTURE_ENV_COLOR:
      source = rmesa->tmu_source[ctx->Texture.CurrentUnit];
      texUnit = &ctx->Texture.Unit[ctx->Texture.CurrentUnit];
      FLOAT_RGBA_TO_UBYTE_RGBA( c, texUnit->EnvColor );
      col = radeonPackColor( 4, c[0], c[1], c[2], c[3] );
      if ( rmesa->env_color[source] != col ) {
	 FLUSH_BATCH( rmesa );
	 rmesa->env_color[source] = col;

	 rmesa->new_state |= RADEON_NEW_TEXTURE;
      }
      break;

   case GL_TEXTURE_LOD_BIAS_EXT:
      /* The Radeon's LOD bias is a signed 2's complement value with a
       * range of -1.0 <= bias < 4.0.  We break this into two linear
       * functions, one mapping [-1.0,0.0] to [-128,0] and one mapping
       * [0.0,4.0] to [0,127].
       */
      source = rmesa->tmu_source[ctx->Texture.CurrentUnit];
      bias = CLAMP( *param, -1.0, 4.0 );
      if ( bias == 0 ) {
	 b = 0;
      } else if ( bias > 0 ) {
	 b = (GLubyte) SCALED_FLOAT_TO_BYTE( bias, 4.0 );
      } else {
	 b = (GLubyte) SCALED_FLOAT_TO_BYTE( bias, 1.0 );
      }
      if ( rmesa->lod_bias[source] != (GLuint)b ) {
	 FLUSH_BATCH( rmesa );
	 rmesa->lod_bias[source] = (GLuint)b;

	 rmesa->new_state |= RADEON_NEW_TEXTURE;
      }
      break;

   default:
      return;
   }
}

static void radeonDDTexParameter( GLcontext *ctx, GLenum target,
				  struct gl_texture_object *tObj,
				  GLenum pname, const GLfloat *params )
{
   radeonContextPtr rmesa = RADEON_CONTEXT(ctx);
   radeonTexObjPtr t = (radeonTexObjPtr)tObj->DriverData;

   if ( RADEON_DEBUG & DEBUG_VERBOSE_API ) {
      fprintf( stderr, "%s( %s )\n",
	       __FUNCTION__, gl_lookup_enum_by_nr( pname ) );
   }

   /* If we don't have a hardware texture, it will be automatically
    * created with current state before it is used, so we don't have
    * to do anything now.
    */
   if ( !t )
      return;

   if ( ( target != GL_TEXTURE_2D ) &&
	( target != GL_TEXTURE_1D ) )
      return;

   switch ( pname ) {
   case GL_TEXTURE_MIN_FILTER:
   case GL_TEXTURE_MAG_FILTER:
      if ( t->bound ) FLUSH_BATCH( rmesa );
      radeonSetTexFilter( t, tObj->MinFilter, tObj->MagFilter );
      break;

   case GL_TEXTURE_WRAP_S:
   case GL_TEXTURE_WRAP_T:
      if ( t->bound ) FLUSH_BATCH( rmesa );
      radeonSetTexWrap( t, tObj->WrapS, tObj->WrapT );
      break;

   case GL_TEXTURE_BORDER_COLOR:
      if ( t->bound ) FLUSH_BATCH( rmesa );
      radeonSetTexBorderColor( t, tObj->BorderColor );
      break;

   default:
      return;
   }

   rmesa->new_state |= RADEON_NEW_TEXTURE;
}

static void radeonDDBindTexture( GLcontext *ctx, GLenum target,
				 struct gl_texture_object *tObj )
{
   radeonContextPtr rmesa = RADEON_CONTEXT(ctx);
   radeonTexObjPtr t = (radeonTexObjPtr) tObj->DriverData;
   GLuint unit = ctx->Texture.CurrentUnit;

   if ( RADEON_DEBUG & DEBUG_VERBOSE_API ) {
      fprintf( stderr, "%s( %p ) unit=%d\n",
	       __FUNCTION__, tObj, unit );
   }

   FLUSH_BATCH( rmesa );

   if ( !t ) {
      t = radeonAllocTexObj( tObj );
      tObj->DriverData = t;
   }

   /* Unbind a currently bound texture.
    */
   if ( rmesa->CurrentTexObj[unit] ) {
      rmesa->CurrentTexObj[unit]->bound &= ~(unit + 1);
      rmesa->CurrentTexObj[unit] = NULL;
   }

   /* Bind to the given texture unit.
    */
   rmesa->CurrentTexObj[unit] = t;
   t->bound |= unit + 1;

   rmesa->new_state |= RADEON_NEW_TEXTURE;
}

static void radeonDDDeleteTexture( GLcontext *ctx,
				   struct gl_texture_object *tObj )
{
   radeonContextPtr rmesa = RADEON_CONTEXT(ctx);
   radeonTexObjPtr t = (radeonTexObjPtr)tObj->DriverData;

   if ( RADEON_DEBUG & DEBUG_VERBOSE_API ) {
      fprintf( stderr, __FUNCTION__ "( %p )\n", tObj );
   }

   if ( t ) {
      if ( t->bound ) {
	 FLUSH_BATCH( rmesa );
	 if ( t->bound & TEX_0 ) rmesa->CurrentTexObj[0] = NULL;
	 if ( t->bound & TEX_1 ) rmesa->CurrentTexObj[1] = NULL;
	 rmesa->new_state |= RADEON_NEW_TEXTURE;
      }

      radeonDestroyTexObj( rmesa, t );
      tObj->DriverData = NULL;
   }
}

static GLboolean radeonDDIsTextureResident( GLcontext *ctx,
					    struct gl_texture_object *tObj )
{
   radeonTexObjPtr t = (radeonTexObjPtr)tObj->DriverData;

   return ( t && t->memBlock );
}

static void radeonDDInitTextureObjects( GLcontext *ctx )
{
   radeonContextPtr rmesa = RADEON_CONTEXT(ctx);
   struct gl_texture_object *texObj;
   GLuint tmp = ctx->Texture.CurrentUnit;

   ctx->Texture.CurrentUnit = 0;

   texObj = ctx->Texture.Unit[0].CurrentD[1];
   radeonDDBindTexture( ctx, GL_TEXTURE_1D, texObj );
   move_to_tail( &rmesa->SwappedOut, (radeonTexObjPtr)texObj->DriverData );

   texObj = ctx->Texture.Unit[0].CurrentD[2];
   radeonDDBindTexture( ctx, GL_TEXTURE_2D, texObj );
   move_to_tail( &rmesa->SwappedOut, (radeonTexObjPtr)texObj->DriverData );

   ctx->Texture.CurrentUnit = 1;

   texObj = ctx->Texture.Unit[1].CurrentD[1];
   radeonDDBindTexture( ctx, GL_TEXTURE_1D, texObj );
   move_to_tail( &rmesa->SwappedOut, (radeonTexObjPtr)texObj->DriverData );

   texObj = ctx->Texture.Unit[1].CurrentD[2];
   radeonDDBindTexture( ctx, GL_TEXTURE_2D, texObj );
   move_to_tail( &rmesa->SwappedOut, (radeonTexObjPtr)texObj->DriverData );

   ctx->Texture.CurrentUnit = tmp;
}

void radeonDDInitTextureFuncs( GLcontext *ctx )
{
   ctx->Driver.TexImage1D		= radeonDDTexImage1D;
   ctx->Driver.TexImage2D		= radeonDDTexImage2D;
   ctx->Driver.TexImage3D		= NULL; (void) radeonDDTexImage3D;
   ctx->Driver.TexSubImage1D		= radeonDDTexSubImage1D;
   ctx->Driver.TexSubImage2D		= radeonDDTexSubImage2D;
   ctx->Driver.TexSubImage3D		= NULL; (void) radeonDDTexSubImage3D;
   ctx->Driver.GetTexImage		= radeonDDGetTexImage;
   ctx->Driver.TexEnv			= radeonDDTexEnv;
   ctx->Driver.TexParameter		= radeonDDTexParameter;
   ctx->Driver.BindTexture		= radeonDDBindTexture;
   ctx->Driver.DeleteTexture		= radeonDDDeleteTexture;
   ctx->Driver.IsTextureResident	= radeonDDIsTextureResident;
   ctx->Driver.PrioritizeTexture	= NULL;
   ctx->Driver.ActiveTexture		= NULL;
   ctx->Driver.UpdateTexturePalette	= NULL;

   radeonDDInitTextureObjects( ctx );
}

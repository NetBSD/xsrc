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
/* $XFree86: xc/lib/GL/mesa/src/drv/tdfx/tdfx_tex.c,v 1.1.2.1 2001/05/22 21:25:41 dawes Exp $ */

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
#include "tdfx_tex.h"
#include "tdfx_texman.h"

#include "enums.h"
#include "image.h"
#include "texutil.h"

#define TX_DITHER_NONE		0x00000000


static int logbase2( int n )
{
   GLint i = 1;
   GLint log2 = 0;

   if ( n < 0 )
      return -1;

   while ( n > i ) {
      i *= 2;
      log2++;
   }
   if ( i != n ) {
      return -1;
   } else {
      return log2;
   }
}

/* Compute various texture image parameters.
 * Input:  w, h - source texture width and height
 * Output:  lodlevel - Glide lod level token for the larger texture dimension
 *          aspectratio - Glide aspect ratio token
 *          sscale - S scale factor used during triangle setup
 *          tscale - T scale factor used during triangle setup
 *          wscale - OpenGL -> Glide image width scale factor
 *          hscale - OpenGL -> Glide image height scale factor
 *
 * Sample results:
 *      w    h       lodlevel               aspectRatio
 *     128  128  GR_LOD_LOG2_128 (=7)  GR_ASPECT_LOG2_1x1 (=0)
 *      64   64  GR_LOD_LOG2_64 (=6)   GR_ASPECT_LOG2_1x1 (=0)
 *      64   32  GR_LOD_LOG2_64 (=6)   GR_ASPECT_LOG2_2x1 (=1)
 *      32   64  GR_LOD_LOG2_64 (=6)   GR_ASPECT_LOG2_1x2 (=-1)
 *      32   32  GR_LOD_LOG2_32 (=5)   GR_ASPECT_LOG2_1x1 (=0)
 */
static void tdfxTexGetInfo( const GLcontext *ctx, int w, int h,
			    GrLOD_t *lodlevel, GrAspectRatio_t *aspectratio,
			    float *sscale, float *tscale,
			    int *wscale, int *hscale )
{
   int logw, logh, ar, lod, ws, hs;
   float s, t;

   ASSERT( w >= 1 );
   ASSERT( h >= 1 );

   logw = logbase2( w );
   logh = logbase2( h );
   ar = logw - logh;  /* aspect ratio = difference in log dimensions */

   /* Hardware only allows a maximum aspect ratio of 8x1, so handle
    * |ar| > 3 by scaling the image and using an 8x1 aspect ratio.
    */
   if ( ar >= 0 ) {
      ASSERT( w >= h );
      lod = logw;
      s = 256.0;
      ws = 1;
      if ( ar <= GR_ASPECT_LOG2_8x1 ) {
	 t = 256 >> ar;
	 hs = 1;
      } else {
	 /* have to stretch image height */
	 t = 32.0;
	 hs = 1 << (ar - 3);
      }
   } else {
      ASSERT( w < h );
      lod = logh;
      t = 256.0;
      hs = 1;
      if ( ar >= GR_ASPECT_LOG2_1x8 ) {
	 s = 256 >> -ar;
	 ws = 1;
      } else {
	 /* have to stretch image width */
	 s = 32.0;
	 ws = 1 << (-ar - 3);
      }
   }

   if ( ar < GR_ASPECT_LOG2_1x8 ) {
      ar = GR_ASPECT_LOG2_1x8;
   } else if ( ar > GR_ASPECT_LOG2_8x1 ) {
      ar = GR_ASPECT_LOG2_8x1;
   }

   if ( lodlevel )
      *lodlevel = (GrLOD_t)lod;
   if ( aspectratio )
      *aspectratio = (GrAspectRatio_t)ar;
   if ( sscale )
      *sscale = s;
   if ( tscale )
      *tscale = t;
   if ( wscale )
      *wscale = ws;
   if ( hscale )
      *hscale = hs;
}


/* We need to call this when a texture object's minification filter
 * or texture image sizes change.
 */
static void tdfxRevalidateTexture( GLcontext *ctx,
				   struct gl_texture_object *tObj )
{
   tdfxTexObjPtr t = TDFX_TEXTURE_DATA(tObj);
   GLint minl, maxl;

   if ( !t )
      return;

   minl = maxl = tObj->BaseLevel;

   if ( tObj->Image[minl] ) {
      maxl = MIN2( tObj->MaxLevel, tObj->Image[minl]->MaxLog2 );

      /* Compute largeLodLog2, aspect ratio and texcoord scale factors.
       */
      tdfxTexGetInfo( ctx,
		      tObj->Image[minl]->Width, tObj->Image[minl]->Height,
		      &t->info.largeLodLog2, &t->info.aspectRatioLog2,
		      &t->sScale, &t->tScale, NULL, NULL );
   }

   if ( tObj->Image[maxl] &&
	tObj->MinFilter != GL_NEAREST  &&
	tObj->MinFilter != GL_LINEAR ) {
      /* Mipmapping: need to compute smallLodLog2 */
      tdfxTexGetInfo( ctx,
		      tObj->Image[maxl]->Width, tObj->Image[maxl]->Height,
		      &t->info.smallLodLog2,
		      NULL, NULL, NULL, NULL, NULL );
   } else {
      /* Not mipmapping: smallLodLog2 = largeLodLog2 */
      t->info.smallLodLog2 = t->info.largeLodLog2;
   }

   t->minLevel = minl;
   t->maxLevel = maxl;
   t->info.data = NULL;
}


static tdfxTexObjPtr tdfxAllocTexObj( tdfxContextPtr fxMesa )
{
   tdfxTexObjPtr t;
   int i;

   t = CALLOC( sizeof(tdfxTexObj) );
   if ( !t ) {
      gl_problem( NULL, "tdfx driver: out of memory" );
      return NULL;
   }

   t->isInTM = GL_FALSE;

   t->whichTMU = TDFX_TMU_NONE;

   t->range[TDFX_TMU0] = NULL;
   t->range[TDFX_TMU1] = NULL;

   t->minFilt = GR_TEXTUREFILTER_POINT_SAMPLED;
   t->magFilt = GR_TEXTUREFILTER_BILINEAR;

   t->sClamp = GR_TEXTURECLAMP_WRAP;
   t->tClamp = GR_TEXTURECLAMP_WRAP;

   t->mmMode = GR_MIPMAP_NEAREST;
   t->LODblend = FXFALSE;

   for ( i = 0 ; i < MAX_TEXTURE_LEVELS ; i++ ) {
      t->image[i].original.data = NULL;
      t->image[i].rescaled.data = NULL;
   }

   return t;
}


/* Given an OpenGL internal texture format, return the corresponding
 * Glide internal texture format and MesaIntTexFormat.
 * If allow32bpp is true, we'll return 32-bit texel formats when
 * appropriate.
 */
static GrTextureFormat_t
tdfxTexGetFormat( tdfxContextPtr fxMesa, struct gl_texture_image *texImage,
		  GLenum format, GLenum type )
{
   const GLboolean allow32bpp = TDFX_IS_NAPALM(fxMesa);
   const GLboolean is32bpp = ( fxMesa->fxScreen->cpp == 4 );
   const struct gl_texture_format *texFormat;
   GrTextureFormat_t ret;

   if ( 0 )
      fprintf( stderr, "internal=%s format=%s type=%s\n",
	       texImage->IntFormat == 3 ? "GL_RGB (3)" :
	       texImage->IntFormat == 4 ? "GL_RGBA (4)" :
	       gl_lookup_enum_by_nr( texImage->IntFormat ),
	       gl_lookup_enum_by_nr( format ),
	       gl_lookup_enum_by_nr( type ) );

#define SET_FORMAT( gr, gl )						\
   do {									\
      ret = (gr);							\
      texFormat = &(gl);						\
   } while (0)

#define SET_FORMAT_32BPP( gr32, gl32, gr16, gl16 )			\
   do {									\
      if ( allow32bpp ) {						\
	 ret = (gr32);							\
	 texFormat = &(gl32);						\
      } else {								\
	 ret = (gr16);							\
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
	 if ( type == GL_UNSIGNED_INT_8_8_8_8_REV && allow32bpp ) {
	    SET_FORMAT( GR_TEXFMT_ARGB_8888, _mesa_texformat_argb8888 );
	    break;
	 } else if ( type == GL_UNSIGNED_SHORT_4_4_4_4_REV ) {
	    SET_FORMAT( GR_TEXFMT_ARGB_4444, _mesa_texformat_argb4444 );
	    break;
	 } else if ( type == GL_UNSIGNED_SHORT_1_5_5_5_REV ) {
	    SET_FORMAT( GR_TEXFMT_ARGB_1555, _mesa_texformat_argb1555 );
	    break;
	 }
      }
      if ( allow32bpp && is32bpp ) {
	 SET_FORMAT( GR_TEXFMT_ARGB_8888, _mesa_texformat_argb8888 );
      } else {
	 SET_FORMAT( GR_TEXFMT_ARGB_4444, _mesa_texformat_argb4444 );
      }
      break;

   case GL_RGB:
   case 3:
      if ( format == GL_RGB && type == GL_UNSIGNED_SHORT_5_6_5 ) {
	 SET_FORMAT( GR_TEXFMT_RGB_565, _mesa_texformat_rgb565 );
	 break;
      }
      if ( allow32bpp && is32bpp ) {
	 SET_FORMAT( GR_TEXFMT_ARGB_8888, _mesa_texformat_argb8888 );
      } else {
	 SET_FORMAT( GR_TEXFMT_RGB_565, _mesa_texformat_rgb565 );
      }
      break;

      /* GH: Okay, keep checking as normal.  Still test for GL_RGB,
       * GL_RGBA formats first.
       */
   case GL_RGBA8:
   case GL_RGB10_A2:
   case GL_RGBA12:
   case GL_RGBA16:
      SET_FORMAT_32BPP( GR_TEXFMT_ARGB_8888, _mesa_texformat_argb8888,
			GR_TEXFMT_ARGB_4444, _mesa_texformat_argb4444 );
      break;

   case GL_RGBA4:
   case GL_RGBA2:
      SET_FORMAT( GR_TEXFMT_ARGB_4444, _mesa_texformat_argb4444 );
      break;

   case GL_RGB5_A1:
      SET_FORMAT( GR_TEXFMT_ARGB_1555, _mesa_texformat_argb1555 );
      break;

   case GL_RGB8:
   case GL_RGB10:
   case GL_RGB12:
   case GL_RGB16:
      SET_FORMAT_32BPP( GR_TEXFMT_ARGB_8888, _mesa_texformat_argb8888,
			GR_TEXFMT_RGB_565,   _mesa_texformat_rgb565 );
      break;

   case GL_RGB5:
   case GL_RGB4:
   case GL_R3_G3_B2:
      SET_FORMAT( GR_TEXFMT_RGB_565, _mesa_texformat_rgb565 );
      break;

   case GL_ALPHA:
   case GL_ALPHA4:
   case GL_ALPHA8:
   case GL_ALPHA12:
   case GL_ALPHA16:
      SET_FORMAT( GR_TEXFMT_ALPHA_8, _mesa_texformat_a8 );
      break;

   case 1:
   case GL_LUMINANCE:
   case GL_LUMINANCE4:
   case GL_LUMINANCE8:
   case GL_LUMINANCE12:
   case GL_LUMINANCE16:
      SET_FORMAT( GR_TEXFMT_INTENSITY_8, _mesa_texformat_l8 );
      break;

   case 2:
   case GL_LUMINANCE_ALPHA:
   case GL_LUMINANCE4_ALPHA4:
   case GL_LUMINANCE6_ALPHA2:
   case GL_LUMINANCE8_ALPHA8:
   case GL_LUMINANCE12_ALPHA4:
   case GL_LUMINANCE12_ALPHA12:
   case GL_LUMINANCE16_ALPHA16:
      SET_FORMAT( GR_TEXFMT_ALPHA_INTENSITY_88, _mesa_texformat_al88 );
      break;

   case GL_INTENSITY:
   case GL_INTENSITY4:
   case GL_INTENSITY8:
   case GL_INTENSITY12:
   case GL_INTENSITY16:
      SET_FORMAT( GR_TEXFMT_ALPHA_8, _mesa_texformat_i8 );
      break;

   case GL_COLOR_INDEX:
   case GL_COLOR_INDEX1_EXT:
   case GL_COLOR_INDEX2_EXT:
   case GL_COLOR_INDEX4_EXT:
   case GL_COLOR_INDEX8_EXT:
   case GL_COLOR_INDEX12_EXT:
   case GL_COLOR_INDEX16_EXT:
      SET_FORMAT( GR_TEXFMT_P_8, _mesa_texformat_ci8 );
      break;

   default:
      fprintf( stderr, "bad texture format in fxTexGetFormat() %d",
	       texImage->IntFormat );
      return -1;
   }

   texImage->TexFormat = texFormat;

   return ret;
}


static GLboolean
tdfxDDTexImage2D( GLcontext *ctx, GLenum target, GLint level,
		  GLenum format, GLenum type, const GLvoid *pixels,
		  const struct gl_pixelstore_attrib *packing,
		  struct gl_texture_object *texObj,
		  struct gl_texture_image *texImage,
		  GLboolean *retainInternalCopy )
{
   tdfxContextPtr fxMesa = TDFX_CONTEXT(ctx);
   const struct gl_texture_format *texFormat;
   GrTextureFormat_t glideFormat;
   tdfxTexObjPtr t;
   tdfxTexImagePtr image;
   GLint dstWidth, dstHeight, wScale, hScale;
   GLint size;
   void *data;

   if ( 0 ) {
      printf("TexImage id=%d int 0x%x  format 0x%x  type 0x%x  %dx%d\n",
	     texObj->Name, texImage->IntFormat, format, type,
	     texImage->Width, texImage->Height);
   }

   if ( target != GL_TEXTURE_2D || texImage->Border > 0 )
      return GL_FALSE;

   if ( !texObj->DriverData )
      texObj->DriverData = tdfxAllocTexObj( fxMesa );

   t = TDFX_TEXTURE_DATA(texObj);
   image = &t->image[level];

   /* Determine the appropriate GL internal texel format, Mesa internal
    * texel format, and texelSize (bytes) given the user's internal
    * texture format hint.
    */
   glideFormat = tdfxTexGetFormat( fxMesa, texImage, format, type );

   /* Get the destination internal format.
    */
   texFormat = texImage->TexFormat;

   /* Determine width and height scale factors for texture.  Remember,
    * Glide is limited to 8:1 aspect ratios.
    */
   tdfxTexGetInfo( ctx,
		   texImage->Width, texImage->Height,
		   NULL, NULL, NULL, NULL,
		   &wScale, &hScale );
   dstWidth = texImage->Width * wScale;
   dstHeight = texImage->Height * hScale;

   /* Allocate new storage for texture image, if needed.  This
    * conditional wants to set uncompressedImage to point to the
    * uncompressed image, and mml->data to the texture data.  If the
    * image is uncompressed, these are identical.  If the image is not
    * compressed, these are different.
    */
   if ( !image->original.data || image->glideFormat != glideFormat ||
	image->original.width != texImage->Width ||
	image->original.height != texImage->Height )
   {
      if ( image->original.data ) {
	 FREE( image->original.data );
	 image->original.data = NULL;
      }
      if ( image->rescaled.data ) {
	 FREE( image->rescaled.data );
	 image->rescaled.data = NULL;
      }

      size = texImage->Width * texImage->Height * texFormat->TexelBytes;
      image->original.data = (void *) MALLOC( size );
      if ( !image->original.data )
	 return GL_FALSE;

      image->original.width = texImage->Width;
      image->original.height = texImage->Height;
      image->original.size = size;

      image->glideFormat = glideFormat;
      image->wScale = wScale;
      image->hScale = hScale;

      t->info.format = glideFormat;
      tdfxTMMoveOutTM( fxMesa, texObj );
   }

   /* Store the texture image into the 'original' space.
    */
   if ( !_mesa_convert_texsubimage2d( texFormat->IntFormat,
				      0, 0, texImage->Width,
				      texImage->Height, texImage->Width,
				      format, type, packing, pixels,
				      image->original.data ) ) {
      return GL_FALSE;
   }

   data = image->original.data;
   size = image->original.size;

   /* GH: Sigh...
    */
   if ( wScale > 1 || hScale > 1 ) {
      if ( image->rescaled.data ) {
	 FREE( image->rescaled.data );
	 image->rescaled.data = NULL;
      }

      size = dstWidth * dstHeight * texFormat->TexelBytes;
      image->rescaled.data = (void *) MALLOC( size );
      if ( !image->rescaled.data )
	 return GL_FALSE;

      image->rescaled.width = dstWidth;
      image->rescaled.height = dstHeight;
      image->rescaled.size = size;

      _mesa_rescale_teximage2d( texFormat,
				texImage->Width, texImage->Height,
				dstWidth, dstHeight,
				image->original.data, image->rescaled.data );

      data = image->rescaled.data;
   }

   image->data = data;
   image->size = size;

   tdfxRevalidateTexture( ctx, texObj );

   t->reloadImages = GL_TRUE;
   fxMesa->new_state |= TDFX_NEW_TEXTURE;

   *retainInternalCopy = GL_FALSE;
   return GL_TRUE;
}


static GLboolean
tdfxDDTexSubImage2D( GLcontext *ctx, GLenum target, GLint level,
		     GLint xoffset, GLint yoffset,
		     GLsizei width, GLsizei height,
		     GLenum format, GLenum type, const GLvoid *pixels,
		     const struct gl_pixelstore_attrib *packing,
		     struct gl_texture_object *texObj,
		     struct gl_texture_image *texImage )
{
   tdfxContextPtr fxMesa = TDFX_CONTEXT(ctx);
   tdfxTexObjPtr t = TDFX_TEXTURE_DATA(texObj);
   tdfxTexImagePtr image;

   if ( target != GL_TEXTURE_2D )
      return GL_FALSE;

   if ( !t )
      return GL_FALSE;

   if ( 0 ) {
      fprintf( stderr, "TexSubImage id=%d lvl=%d int=0x%x format=0x%x type=0x%x  x=%d y=%d w=%d h=%d fullW=%d fullH=%d\n",
	       texObj->Name, level, texImage->IntFormat, format, type,
	       xoffset, yoffset, width, height,
	       texImage->Width, texImage->Height );
   }

   image = &t->image[level];

   /* Must have an existing texture image!
    */
   assert( image->original.data );

   if ( !_mesa_convert_texsubimage2d( texImage->TexFormat->IntFormat,
				      xoffset, yoffset, width, height,
				      texImage->Width,
				      format, type, packing,
				      pixels, image->original.data ) ) {
      return GL_FALSE;
   }

   /* Rescale the original image again if we have to.
    */
   if ( image->wScale > 1 || image->hScale > 1 ) {
      assert( image->rescaled.data );
      _mesa_rescale_teximage2d( texImage->TexFormat,
				image->original.width, image->original.height,
				image->rescaled.width, image->rescaled.height,
				image->original.data, image->rescaled.data );
   }

   t->reloadImages = GL_TRUE; /* signal the image needs to be reloaded */
   fxMesa->new_state |= TDFX_NEW_TEXTURE;  /* XXX this might be a bit much */

   return GL_TRUE;
}



/* ================================================================
 *
 */

static void tdfxDDTexEnv( GLcontext *ctx, GLenum target,
			  GLenum pname, const GLfloat *param )
{
   tdfxContextPtr fxMesa = TDFX_CONTEXT(ctx);

   if ( TDFX_DEBUG & DEBUG_VERBOSE_API ) {
      if ( param ) {
	 fprintf( stderr, __FUNCTION__"( %x, %x )\n", pname, (GLint)(*param) );
      } else {
	 fprintf( stderr, __FUNCTION__"( %x )\n", pname );
      }
   }

   fxMesa->new_state |= TDFX_NEW_TEXTURE;
}

static void tdfxDDTexParameter( GLcontext *ctx, GLenum target,
				struct gl_texture_object *tObj,
				GLenum pname, const GLfloat *params )
{
   tdfxContextPtr fxMesa = TDFX_CONTEXT(ctx);
   GLenum param = (GLenum) (GLint) params[0];
   tdfxTexObjPtr t;

   if ( TDFX_DEBUG & DEBUG_VERBOSE_API ) {
      fprintf( stderr, __FUNCTION__ "( %d, %p, %x, %x )\n",
	       tObj->Name, tObj->DriverData, pname, param );
   }

   if ( target != GL_TEXTURE_2D )
      return;

   if ( !tObj->DriverData )
      tObj->DriverData = tdfxAllocTexObj( fxMesa );

   t = TDFX_TEXTURE_DATA(tObj);

   switch ( pname ) {
   case GL_TEXTURE_MIN_FILTER:
      switch ( param ) {
      case GL_NEAREST:
	 t->mmMode = GR_MIPMAP_DISABLE;
	 t->minFilt = GR_TEXTUREFILTER_POINT_SAMPLED;
	 t->LODblend = FXFALSE;
	 break;

      case GL_LINEAR:
	 t->mmMode = GR_MIPMAP_DISABLE;
	 t->minFilt = GR_TEXTUREFILTER_BILINEAR;
	 t->LODblend = FXFALSE;
	 break;

      case GL_NEAREST_MIPMAP_LINEAR:
	 if ( TDFX_IS_NAPALM(fxMesa) ) {
	    if ( fxMesa->numTMUs > 1 ) {
	       t->mmMode = GR_MIPMAP_NEAREST;
	       t->LODblend = FXTRUE;
	    } else {
	       t->mmMode = GR_MIPMAP_NEAREST_DITHER;
	       t->LODblend = FXFALSE;
	    }
	    t->minFilt = GR_TEXTUREFILTER_POINT_SAMPLED;
	    break;
	 }
	 /* XXX Voodoo3/Banshee mipmap blending seems to produce
	  * incorrectly filtered colors for the smallest mipmap levels.
	  * To work-around we fall-through here and use a different filter.
	  */
      case GL_NEAREST_MIPMAP_NEAREST:
	 t->mmMode = GR_MIPMAP_NEAREST;
	 t->minFilt = GR_TEXTUREFILTER_POINT_SAMPLED;
	 t->LODblend = FXFALSE;
	 break;

      case GL_LINEAR_MIPMAP_LINEAR:
	 if ( TDFX_IS_NAPALM(fxMesa) ) {
	    if ( fxMesa->numTMUs > 1 ) {
	       t->mmMode = GR_MIPMAP_NEAREST;
	       t->LODblend = FXTRUE;
	    } else {
	       t->mmMode = GR_MIPMAP_NEAREST_DITHER;
	       t->LODblend = FXFALSE;
	    }
	    t->minFilt = GR_TEXTUREFILTER_BILINEAR;
	    break;
	 }
	 /* XXX Voodoo3/Banshee mipmap blending seems to produce
	  * incorrectly filtered colors for the smallest mipmap levels.
	  * To work-around we fall-through here and use a different filter.
	  */
      case GL_LINEAR_MIPMAP_NEAREST:
	 t->mmMode = GR_MIPMAP_NEAREST;
	 t->minFilt = GR_TEXTUREFILTER_BILINEAR;
	 t->LODblend = FXFALSE;
	 break;
      default:
	 break;
      }
      tdfxRevalidateTexture( ctx, tObj );
      fxMesa->new_state |= TDFX_NEW_TEXTURE;
      break;

   case GL_TEXTURE_MAG_FILTER:
      switch ( param ) {
      case GL_NEAREST:
	 t->magFilt = GR_TEXTUREFILTER_POINT_SAMPLED;
	 break;
      case GL_LINEAR:
	 t->magFilt = GR_TEXTUREFILTER_BILINEAR;
	 break;
      default:
	 break;
      }
      fxMesa->new_state |= TDFX_NEW_TEXTURE;
      break;

   case GL_TEXTURE_WRAP_S:
      switch ( param ) {
      case GL_CLAMP:
	 t->sClamp = GR_TEXTURECLAMP_CLAMP;
	 break;
      case GL_REPEAT:
	 t->sClamp = GR_TEXTURECLAMP_WRAP;
	 break;
      default:
	 break;
      }
      fxMesa->new_state |= TDFX_NEW_TEXTURE;
      break;

   case GL_TEXTURE_WRAP_T:
      switch ( param ) {
      case GL_CLAMP:
	 t->tClamp = GR_TEXTURECLAMP_CLAMP;
	 break;
      case GL_REPEAT:
	 t->tClamp = GR_TEXTURECLAMP_WRAP;
	 break;
      default:
	 break;
      }
      fxMesa->new_state |= TDFX_NEW_TEXTURE;
      break;

   case GL_TEXTURE_BASE_LEVEL:
      tdfxRevalidateTexture( ctx, tObj );
      break;

   case GL_TEXTURE_MAX_LEVEL:
      tdfxRevalidateTexture( ctx, tObj );
      break;

   case GL_TEXTURE_BORDER_COLOR:
      /* TO DO */
      break;
   case GL_TEXTURE_MIN_LOD:
      /* TO DO */
      break;
   case GL_TEXTURE_MAX_LOD:
      /* TO DO */
      break;

   default:
      break;
   }
}

static void tdfxDDBindTexture( GLcontext *ctx, GLenum target,
			       struct gl_texture_object *tObj )
{
   tdfxContextPtr fxMesa = TDFX_CONTEXT(ctx);
   tdfxTexObjPtr t;

   if ( TDFX_DEBUG & DEBUG_VERBOSE_API ) {
      fprintf( stderr, __FUNCTION__ "( %d, %p )\n",
	       tObj->Name, tObj->DriverData );
   }

   if ( target != GL_TEXTURE_2D )
      return;

   if ( !tObj->DriverData )
      tObj->DriverData = tdfxAllocTexObj( fxMesa );

   t = TDFX_TEXTURE_DATA(tObj);
   t->lastTimeUsed = fxMesa->texBindNumber++;

   fxMesa->new_state |= TDFX_NEW_TEXTURE;
}

static void tdfxDDDeleteTexture( GLcontext *ctx,
				 struct gl_texture_object *tObj )
{
   tdfxContextPtr fxMesa = TDFX_CONTEXT(ctx);

   if ( fxMesa->driDrawable ) {
      LOCK_HARDWARE( fxMesa );
      tdfxTMFreeTextureLocked( fxMesa, tObj );
      UNLOCK_HARDWARE( fxMesa );
   }

   fxMesa->new_state |= TDFX_NEW_TEXTURE;
}

static GLboolean tdfxDDIsTextureResident( GLcontext *ctx,
					  struct gl_texture_object *tObj )
{
   tdfxTexObjPtr t = TDFX_TEXTURE_DATA(tObj);

   return ( t && t->isInTM );
}


/* Convert a gl_color_table texture palette to Glide's format.
 */
static void
tdfxConvertPalette( FxU32 data[256], const struct gl_color_table *table )
{
   const GLubyte *tableUB = (const GLubyte *) table->Table;
   GLint width = table->Size;
   FxU32 r, g, b, a;
   GLint i;

   ASSERT( table->TableType == GL_UNSIGNED_BYTE );

   switch ( table->Format ) {
   case GL_RGBA:
      for ( i = 0 ; i < width ; i++ ) {
	 r = tableUB[i * 4 + 0];
	 g = tableUB[i * 4 + 1];
	 b = tableUB[i * 4 + 2];
	 a = tableUB[i * 4 + 3];
	 data[i] = PACK_COLOR_8888( a, r, g, b );
      }
      break;
   case GL_RGB:
      for ( i = 0 ; i < width ; i++ ) {
	 r = tableUB[i * 3 + 0];
	 g = tableUB[i * 3 + 1];
	 b = tableUB[i * 3 + 2];
	 a = 255;
	 data[i] = PACK_COLOR_8888( a, r, g, b );
      }
      break;
   case GL_LUMINANCE:
      for ( i = 0 ; i < width ; i++ ) {
	 r = tableUB[i];
	 g = tableUB[i];
	 b = tableUB[i];
	 a = 255;
	 data[i] = PACK_COLOR_8888( a, r, g, b );
      }
      break;
   case GL_ALPHA:
      for ( i = 0 ; i < width ; i++ ) {
	 r = g = b = 255;
	 a = tableUB[i];
	 data[i] = PACK_COLOR_8888( a, r, g, b );
      }
      break;
   case GL_LUMINANCE_ALPHA:
      for ( i = 0 ; i < width ; i++ ) {
	 r = g = b = tableUB[i * 2 + 0];
	 a = tableUB[i * 2 + 1];
	 data[i] = PACK_COLOR_8888( a, r, g, b );
      }
      break;
   case GL_INTENSITY:
      for ( i = 0 ; i < width ; i++ ) {
	 r = tableUB[i];
	 g = tableUB[i];
	 b = tableUB[i];
	 a = tableUB[i];
	 data[i] = PACK_COLOR_8888( a, r, g, b );
      }
      break;
   }
}

static void
tdfxDDTexturePalette( GLcontext *ctx, struct gl_texture_object *tObj )
{
   tdfxContextPtr fxMesa = TDFX_CONTEXT(ctx);
   tdfxTexObjPtr t;

   if ( tObj ) {
      /* Per-texture palette */
      if ( !tObj->DriverData )
	 tObj->DriverData = tdfxAllocTexObj(fxMesa);

      t = TDFX_TEXTURE_DATA(tObj);
      tdfxConvertPalette( t->palette.data, &tObj->Palette );
      /*tdfxTexInvalidate( ctx, tObj );*/
   } else {
      /* Global texture palette */
      tdfxConvertPalette( fxMesa->glbPalette.data, &ctx->Texture.Palette );
   }

   fxMesa->new_state |= TDFX_NEW_TEXTURE; /* XXX too heavy-handed */
}




/**********************************************************************/
/**** NEW TEXTURE IMAGE FUNCTIONS                                  ****/
/**********************************************************************/



#if 0
static void
PrintTexture(int w, int h, int c, const GLubyte * data)
{
   int i, j;
   for (i = 0; i < h; i++) {
      for (j = 0; j < w; j++) {
	 if (c == 2)
	    printf("%02x %02x  ", data[0], data[1]);
	 else if (c == 3)
	    printf("%02x %02x %02x  ", data[0], data[1], data[2]);
	 data += c;
      }
      printf("\n");
   }
}
#endif


static GLboolean
tdfxDDTestProxyTexImage( GLcontext *ctx, GLenum target,
			 GLint level, GLint internalFormat,
			 GLenum format, GLenum type,
			 GLint width, GLint height,
			 GLint depth, GLint border )
{
   tdfxContextPtr fxMesa = TDFX_CONTEXT(ctx);
   struct gl_shared_state *ss = fxMesa->glCtx->Shared;
   tdfxSharedStatePtr tss = (tdfxSharedStatePtr)ss->DriverData;

   switch (target) {
   case GL_PROXY_TEXTURE_1D:
      return GL_TRUE;  /* software rendering */
   case GL_PROXY_TEXTURE_2D:
   {
      struct gl_texture_object *tObj;
      tdfxTexObjPtr t;
      int memNeeded;

      tObj = ctx->Texture.Proxy2D;
      if (!tObj->DriverData)
	 tObj->DriverData = tdfxAllocTexObj(fxMesa);
      t = TDFX_TEXTURE_DATA(tObj);

      /* assign the parameters to test against */
      tObj->Image[level]->Width = width;
      tObj->Image[level]->Height = height;
      tObj->Image[level]->Border = border;
      tObj->Image[level]->IntFormat = internalFormat;
      if (level == 0) {
	 /* don't use mipmap levels > 0 */
	 tObj->MinFilter = tObj->MagFilter = GL_NEAREST;
      }
      else {
	 /* test with all mipmap levels */
	 tObj->MinFilter = GL_LINEAR_MIPMAP_LINEAR;
	 tObj->MagFilter = GL_NEAREST;
      }
      tdfxRevalidateTexture(ctx, tObj);

      /*
	printf("small lodlog2 0x%x\n", t->info.smallLodLog2);
	printf("large lodlog2 0x%x\n", t->info.largeLodLog2);
	printf("aspect ratio 0x%x\n", t->info.aspectRatioLog2);
	printf("glide format 0x%x\n", t->info.format);
	printf("data %p\n", t->info.data);
	printf("lodblend %d\n", (int) t->LODblend);
      */

      /* determine where texture will reside */
      if (t->LODblend && !tss->umaTexMemory) {
	 /* XXX GR_MIPMAPLEVELMASK_BOTH might not be right, but works */
	 memNeeded = FX_grTexTextureMemRequired_NoLock(
	    GR_MIPMAPLEVELMASK_BOTH, &(t->info));
      }
      else {
	 /* XXX GR_MIPMAPLEVELMASK_BOTH might not be right, but works */
	 memNeeded = FX_grTexTextureMemRequired_NoLock(
	    GR_MIPMAPLEVELMASK_BOTH, &(t->info));
      }
      /*
	printf("Proxy test %d > %d\n", memNeeded, tss->totalTexMem[0]);
      */
      if (memNeeded > tss->totalTexMem[0])
	 return GL_FALSE;
      else
	 return GL_TRUE;
   }
   case GL_PROXY_TEXTURE_3D:
      return GL_TRUE;  /* software rendering */
   default:
      return GL_TRUE;  /* never happens, silence compiler */
   }
}


/* Return a texture image to Mesa.  This is either to satisfy
 * a glGetTexImage() call or to prepare for software texturing.
 */
static GLvoid *
tdfxDDGetTexImage( GLcontext *ctx, GLenum target, GLint level,
		   const struct gl_texture_object *texObj,
		   GLenum *formatOut, GLenum *typeOut,
		   GLboolean *freeImageOut )
{
   const struct gl_texture_image *texImage = texObj->Image[level];
   const struct gl_texture_format *texFormat = texImage->TexFormat;
   tdfxTexObjPtr t = TDFX_TEXTURE_DATA(texObj);
   tdfxTexImagePtr image;
   GLubyte *data;

   if ( target != GL_TEXTURE_2D )
      return NULL;
   if ( !t )
      return NULL;

   image = &t->image[level];
   if ( !image->original.data )
      return NULL;

   data = (GLubyte *) MALLOC( texImage->Width * texImage->Height * 4 );
   if ( !data )
      return NULL;

   _mesa_unconvert_teximage2d( texFormat->IntFormat, texImage->Format,
			       texImage->Width, texImage->Height,
			       image->original.data, data );

   *formatOut = texImage->Format;
   *typeOut = GL_UNSIGNED_BYTE;
   *freeImageOut = GL_TRUE;

   return data;
}


void tdfxDDInitTextureFuncs( GLcontext *ctx )
{
   ctx->Driver.TexImage2D		= tdfxDDTexImage2D;
   ctx->Driver.TexSubImage2D		= tdfxDDTexSubImage2D;
   ctx->Driver.GetTexImage		= tdfxDDGetTexImage;
   ctx->Driver.TexEnv			= tdfxDDTexEnv;
   ctx->Driver.TexParameter		= tdfxDDTexParameter;
   ctx->Driver.BindTexture		= tdfxDDBindTexture;
   ctx->Driver.DeleteTexture		= tdfxDDDeleteTexture;
   ctx->Driver.IsTextureResident	= tdfxDDIsTextureResident;
   ctx->Driver.UpdateTexturePalette	= tdfxDDTexturePalette;
}

/*
 * GLX Hardware Device Driver for Matrox Millenium G200
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
 *    original by Wittawat Yamwong <Wittawat.Yamwong@stud.uni-hannover.de>
 *	9/20/99 rewrite by John Carmack <johnc@idsoftware.com>
 *      13/1/00 port to DRI by Keith Whitwell <keithw@precisioninsight.com>
 */
/* $XFree86: xc/lib/GL/mesa/src/drv/mga/mgatex.c,v 1.7 2000/11/08 05:02:46 dawes Exp $ */

#include <stdlib.h>
#include <stdio.h>
#include <GL/gl.h>

#include "mm.h"
#include "mgacontext.h"
#include "mgatex.h"
#include "mgaregs.h"
#include "mgaioctl.h"

#include "enums.h"
#include "simple_list.h"
#include "mem.h"

#define TEX_0 1
#define TEX_1 2

/*
 * mgaDestroyTexObj
 * Free all memory associated with a texture and NULL any pointers
 * to it.
 */
void 
mgaDestroyTexObj( mgaContextPtr mmesa, mgaTextureObjectPtr t ) 
{
   if ( !t ) return;
  	  	
   /* free the texture memory */
   if (t->MemBlock) {
      mmFreeMem( t->MemBlock );
      t->MemBlock = 0;

      if (t->age > mmesa->dirtyAge)
	 mmesa->dirtyAge = t->age;
   }
 
   /* free mesa's link */   
   if (t->tObj) 
      t->tObj->DriverData = NULL;

   /* see if it was the driver's current object */
   if (t->bound & TEX_0) mmesa->CurrentTexObj[0] = 0; 
   if (t->bound & TEX_1) mmesa->CurrentTexObj[1] = 0; 
	
   remove_from_list(t);
   FREE( t );
}


/*
 * mgaSetTexWrappings
 */
static void mgaSetTexWrapping( mgaTextureObjectPtr t,
			       GLenum sWrap, 
			       GLenum tWrap ) 
{
   GLuint val = 0;

   if (sWrap != GL_REPEAT) 
      val |= TMC_clampu_enable;
   
   if (tWrap != GL_REPEAT) 
      val |= TMC_clampv_enable;

   t->Setup[MGA_TEXREG_CTL] &= ~(TMC_clampu_enable|TMC_clampv_enable);
   t->Setup[MGA_TEXREG_CTL] |= val;
}


/*
 * mgaSetTexFilter
 */
static void mgaSetTexFilter(mgaTextureObjectPtr t, GLenum minf, GLenum magf) 
{
   GLuint val = 0;

   switch (minf) {
   case GL_NEAREST: val = TF_minfilter_nrst; break;
   case GL_LINEAR: val = TF_minfilter_bilin; break;
   case GL_NEAREST_MIPMAP_NEAREST: val = TF_minfilter_mm1s; break;
   case GL_LINEAR_MIPMAP_NEAREST: val = TF_minfilter_mm4s; break;
   case GL_NEAREST_MIPMAP_LINEAR: val = TF_minfilter_mm2s; break;
   case GL_LINEAR_MIPMAP_LINEAR: val = TF_minfilter_mm8s; break;
   default: val = TF_minfilter_nrst; break;
   }

   switch (magf) {
   case GL_NEAREST: val |= TF_magfilter_nrst; break;
   case GL_LINEAR: val |= TF_magfilter_bilin; break;
   default: val |= TF_magfilter_nrst; break;
   }
  
   /* See OpenGL 1.2 specification */
   if (magf == GL_LINEAR && (minf == GL_NEAREST_MIPMAP_NEAREST || 
			     minf == GL_NEAREST_MIPMAP_LINEAR)) {
      val |= (0x20 << TF_fthres_SHIFT); /* c = 0.5 */
   } else {
      val |= (0x10 << TF_fthres_SHIFT); /* c = 0 */
   }


   t->Setup[MGA_TEXREG_FILTER] &= (TF_minfilter_MASK |
				   TF_magfilter_MASK |
				   TF_fthres_MASK);   
   t->Setup[MGA_TEXREG_FILTER] |= val;
}

/*
 * mgaSetTexBorderColor
 */
static void mgaSetTexBorderColor(mgaTextureObjectPtr t, GLubyte color[4]) 
{
   t->Setup[MGA_TEXREG_BORDERCOL] = MGAPACKCOLOR8888(color[0],color[1],
						     color[2],color[3]);
}









/*
 * mgaCreateTexObj
 * Allocate space for and load the mesa images into the texture memory block.
 * This will happen before drawing with a new texture, or drawing with a
 * texture after it was swapped out or teximaged again.
 */
static void mgaCreateTexObj(mgaContextPtr mmesa, 
			    struct gl_texture_object *tObj) 
{
   const struct gl_texture_image *image = tObj->Image[ 0 ];   
   mgaTextureObjectPtr t;
   int i, ofs;
   int LastLevel;
   int s, s2;
   int textureFormat;
	

   if (!image) return;


   tObj->DriverData = t = CALLOC( sizeof( *t ) );
   if (!t) {
      fprintf(stderr, "mgaCreateTexObj: Failed to malloc mgaTextureObject\n" );
      return;
   }

   switch( image->Format ) {
   case GL_RGB:
   case GL_LUMINANCE:
      if ( image->IntFormat != GL_RGB5 && ( image->IntFormat == GL_RGB8 ||
					    mmesa->default32BitTextures ) ) {
	 t->texelBytes = 4;
	 textureFormat = TMC_tformat_tw32;
      } else {
	 t->texelBytes = 2;
	 textureFormat = TMC_tformat_tw16;
      }
      break;
   case GL_ALPHA:
   case GL_LUMINANCE_ALPHA:
   case GL_INTENSITY:
   case GL_RGBA:
      if ( image->IntFormat != GL_RGBA4 && ( image->IntFormat == GL_RGBA8 ||
					     mmesa->default32BitTextures ) ) {
	 t->texelBytes = 4;
	 textureFormat = TMC_tformat_tw32;
      } else {
	 t->texelBytes = 2;
	 textureFormat = TMC_tformat_tw12;
      }
      break;
   case GL_COLOR_INDEX:
      textureFormat = TMC_tformat_tw8;
      t->texelBytes = 1;
      break;
   default:
      fprintf(stderr, "mgaCreateTexObj: bad image->Format %x/%s\n",
	      image->Format,
	      gl_lookup_enum_by_nr(image->Format));
      FREE( t );
      tObj->DriverData = 0;
      return;	
   }
		
   /* We are going to upload all levels that are present, even if
    * later levels wouldn't be used by the current filtering mode.  This
    * allows the filtering mode to change without forcing another upload
    * of the images.
    */
   LastLevel = MGA_TEX_MAXLEVELS-1;

   ofs = 0;
   for ( i = 0 ; i <= LastLevel ; i++ ) {		
      if ( !tObj->Image[i] ) {
	 LastLevel = i - 1;
	 break;
      }

      t->offsets[i] = ofs;
      t->dirty_images |= (1<<i);

      ofs += ((MAX2( tObj->Image[i]->Width, 8 ) * 
	       MAX2( tObj->Image[i]->Height, 8 ) * 
	       t->texelBytes) + 31) & ~31;
   }

   t->totalSize = ofs;
   t->lastLevel = LastLevel;
   t->tObj = tObj;
   t->ctx = mmesa;
   t->age = 0;
   t->bound = 0;
   t->MemBlock = 0;

   insert_at_tail(&(mmesa->SwappedOut), t);
	

   /* setup hardware register values */		
   t->Setup[MGA_TEXREG_CTL] = (TMC_takey_1 | 
			       TMC_tamask_0 | 
			       textureFormat );

   if (image->WidthLog2 >= 3) 
      t->Setup[MGA_TEXREG_CTL] |= ((image->WidthLog2 - 3) << TMC_tpitch_SHIFT);
   else 
      t->Setup[MGA_TEXREG_CTL] |= (TMC_tpitchlin_enable | 
				   (image->Width << TMC_tpitchext_SHIFT));


   t->Setup[MGA_TEXREG_CTL2] = TMC_ckstransdis_enable;

   if ( mmesa->glCtx->Light.Model.ColorControl == GL_SEPARATE_SPECULAR_COLOR ) 
      t->Setup[MGA_TEXREG_CTL2] |= TMC_specen_enable;

	
   t->Setup[MGA_TEXREG_FILTER] = (TF_minfilter_nrst | 
				  TF_magfilter_nrst |
				  TF_filteralpha_enable | 
				  (0x10 << TF_fthres_SHIFT) | 
				  (LastLevel << TF_mapnb_SHIFT));
	
   /* warp texture registers */
   ofs = MGA_IS_G200(mmesa) ? 28 : 11;
   s = image->Width;
   s2 = image->WidthLog2;
   t->Setup[MGA_TEXREG_WIDTH] = (MGA_FIELD(TW_twmask, s - 1) |
				 MGA_FIELD(TW_rfw, (10 - s2 - 8) & 63 ) |
				 MGA_FIELD(TW_tw, (s2 + ofs ) | 0x40 ));

	
   s = image->Height;
   s2 = image->HeightLog2;
   t->Setup[MGA_TEXREG_HEIGHT] = (MGA_FIELD(TH_thmask, s - 1) |
				  MGA_FIELD(TH_rfh, (10 - s2 - 8) & 63 ) | 
				  MGA_FIELD(TH_th, (s2 + ofs ) | 0x40 ));


   /* set all the register values for filtering, border, etc */	
   mgaSetTexWrapping( t, tObj->WrapS, tObj->WrapT );
   mgaSetTexFilter( t, tObj->MinFilter, tObj->MagFilter );
   mgaSetTexBorderColor( t, tObj->BorderColor );
}




static void mgaUpdateTextureEnvG200( GLcontext *ctx )
{
   struct gl_texture_object *tObj = ctx->Texture.Unit[0].Current;
   mgaTextureObjectPtr t;

   if (!tObj || !tObj->DriverData)
      return;

   t = (mgaTextureObjectPtr)tObj->DriverData;

   switch (ctx->Texture.Unit[0].EnvMode) {
   case GL_REPLACE:
      t->Setup[MGA_TEXREG_CTL] &= ~TMC_tmodulate_enable;
      break;
   case GL_MODULATE:
      t->Setup[MGA_TEXREG_CTL] |= TMC_tmodulate_enable;
      break;
   case GL_DECAL:
      t->Setup[MGA_TEXREG_CTL] &= ~TMC_tmodulate_enable;
      break;
   case GL_BLEND:
      t->ctx->Fallback |= MGA_FALLBACK_TEXTURE;
      break;
   default:
      break;
   }
}

static void mgaUpdateTextureEnvG400( GLcontext *ctx, int unit )
{
   mgaContextPtr mmesa = MGA_CONTEXT( ctx );
   GLuint *reg = &mmesa->Setup[MGA_CTXREG_TDUAL0 + unit];
   GLuint source = mmesa->tmu_source[unit];
   struct gl_texture_object *tObj = ctx->Texture.Unit[source].Current;

   if ( tObj != ctx->Texture.Unit[source].CurrentD[2] || 
	!tObj || 
	!tObj->Complete || 
	((ctx->Enabled>>(source*4))&TEXTURE0_ANY) != TEXTURE0_2D ) 
      return;
	

   switch (ctx->Texture.Unit[source].EnvMode) {
   case GL_REPLACE:
      *reg = (TD0_color_sel_arg1 |
	      TD0_alpha_sel_arg1 );
      break;

   case GL_MODULATE:
      if (unit == 0)
	 *reg = ( TD0_color_arg2_diffuse |
		  TD0_color_sel_mul | 
		  TD0_alpha_arg2_diffuse |
		  TD0_alpha_sel_mul);
      else
	 *reg = ( TD0_color_arg2_prevstage |
		  TD0_color_alpha_prevstage |
		  TD0_color_sel_mul | 
		  TD0_alpha_arg2_prevstage |
		  TD0_alpha_sel_mul);
      break;
   case GL_DECAL:
      if (tObj->Image[0]->Format == GL_RGB) 
	 *reg = (TD0_color_sel_arg1 |
		 TD0_alpha_sel_arg1 );
      else if (unit == 0) 
	 *reg = (TD0_color_arg2_diffuse | 
		 TD0_color_alpha_currtex |
		 TD0_color_alpha2inv_enable |
		 TD0_color_arg2mul_alpha2 |
		 TD0_color_arg1mul_alpha1 |
		 TD0_color_add_add |
		 TD0_color_sel_add |
		 TD0_alpha_arg2_diffuse |
		 TD0_alpha_sel_arg2 );
      else
	 *reg = (TD0_color_arg2_prevstage | 
		 TD0_color_alpha_currtex |
		 TD0_color_alpha2inv_enable |
		 TD0_color_arg2mul_alpha2 |
		 TD0_color_arg1mul_alpha1 |
		 TD0_color_add_add |
		 TD0_color_sel_add |
		 TD0_alpha_arg2_prevstage |
		 TD0_alpha_sel_arg2 );

      break;

   case GL_ADD:
      if (unit == 0)
	 *reg = ( TD0_color_arg2_diffuse |
		  TD0_color_add_add |
		  TD0_color_sel_add | 
		  TD0_alpha_arg2_diffuse |
		  TD0_alpha_sel_add);
      else
	 *reg = ( TD0_color_arg2_prevstage |
		  TD0_color_alpha_prevstage |
		  TD0_color_add_add |
		  TD0_color_sel_add | 
		  TD0_alpha_arg2_prevstage |
		  TD0_alpha_sel_add);
      break;

   case GL_BLEND:
      if (mmesa->blend_flags) 
	 mmesa->Fallback |= MGA_FALLBACK_TEXTURE;

      /* Do singletexture GL_BLEND with 'all ones' env-color
       * by using both texture units.  Multitexture gl_blend
       * is a fallback.  
       */
      if (unit == 0) {
	 /* Part 1: R1 = Rf ( 1 - Rt )
	  *         A1 = Af At
	  */
	 *reg = ( TD0_color_arg2_diffuse |
		  TD0_color_arg1_inv_enable |
		  TD0_color_sel_mul | 
		  TD0_alpha_arg2_diffuse |
		  TD0_alpha_sel_arg1);
      } else {
	 /* Part 2: R2 = R1 + Rt
	  *         A2 = A1
	  */
	 *reg = ( TD0_color_arg2_prevstage |
		  TD0_color_add_add | 
		  TD0_color_sel_add | 
		  TD0_alpha_arg2_prevstage |
		  TD0_alpha_sel_arg2);		
      }	
      break;
   default:
      break;
   }
}



static void mgaUpdateTextureObject( GLcontext *ctx, int unit ) 
{
   mgaTextureObjectPtr t;
   struct gl_texture_object	*tObj;
   GLuint enabled;
   mgaContextPtr mmesa = MGA_CONTEXT( ctx );
   GLuint source = mmesa->tmu_source[unit];


   enabled = (ctx->Texture.ReallyEnabled>>(source*4))&TEXTURE0_ANY;
   tObj = ctx->Texture.Unit[source].Current;

   if (enabled != TEXTURE0_2D) {
      if (enabled)
	 mmesa->Fallback |= MGA_FALLBACK_TEXTURE;
      return;
   }

   if ( !tObj || tObj != ctx->Texture.Unit[source].CurrentD[2] ) {
      mmesa->Fallback |= MGA_FALLBACK_TEXTURE;
      return;
   }

/*     if (!tObj) tObj = ctx->Texture.Unit[0].Current; */
/*     if (!tObj) return; */

   if ( !tObj->DriverData ) {
      mgaCreateTexObj( mmesa, tObj );
      if ( !tObj->DriverData ) {
	 mmesa->Fallback |= MGA_FALLBACK_TEXTURE;
	 return;		
      }
   }

   t = (mgaTextureObjectPtr)tObj->DriverData;

   if (t->dirty_images) 
      mmesa->dirty |= (MGA_UPLOAD_TEX0IMAGE << unit);

   mmesa->CurrentTexObj[unit] = t;
   t->bound |= unit+1;

/*     if (t->MemBlock) */
/*        mgaUpdateTexLRU( mmesa, t ); */

   t->Setup[MGA_TEXREG_CTL2] &= ~TMC_dualtex_enable; 
   if (mmesa->multitex)  
      t->Setup[MGA_TEXREG_CTL2] |= TMC_dualtex_enable; 

   t->Setup[MGA_TEXREG_CTL2] &= ~TMC_specen_enable;
   if (ctx->Light.Model.ColorControl == GL_SEPARATE_SPECULAR_COLOR)
      t->Setup[MGA_TEXREG_CTL2] |= TMC_specen_enable;
}






/* The G400 is now programmed quite differently wrt texture environment.
 */
void mgaUpdateTextureState( GLcontext *ctx )
{
   mgaContextPtr mmesa = MGA_CONTEXT( ctx );
   mmesa->Fallback &= ~MGA_FALLBACK_TEXTURE;

   if (mmesa->CurrentTexObj[0]) {
      mmesa->CurrentTexObj[0]->bound = 0;
      mmesa->CurrentTexObj[0] = 0;
   }

   if (mmesa->CurrentTexObj[1]) {
      mmesa->CurrentTexObj[1]->bound = 0;
      mmesa->CurrentTexObj[1] = 0;   
   }

   if (MGA_IS_G400(mmesa)) {
      mgaUpdateTextureObject( ctx, 0 );		
      mgaUpdateTextureEnvG400( ctx, 0 );

      mmesa->Setup[MGA_CTXREG_TDUAL1] = mmesa->Setup[MGA_CTXREG_TDUAL0];

      if (mmesa->multitex || 1) {
	 mgaUpdateTextureObject( ctx, 1 );	
	 mgaUpdateTextureEnvG400( ctx, 1 );
      } 
/*  else  */
/*  	 mmesa->Setup[MGA_CTXREG_TDUAL1] = ( TD0_color_arg2_prevstage | */
/*  					     TD0_color_sel_arg2 |  */
/*  					     TD0_alpha_arg2_prevstage | */
/*  					     TD0_alpha_sel_arg2); */

      		
      mmesa->dirty |= MGA_UPLOAD_TEX1;
   } else {
      mgaUpdateTextureObject( ctx, 0 );
      mgaUpdateTextureEnvG200( ctx );		
   }

   mmesa->dirty |= MGA_UPLOAD_CTX | MGA_UPLOAD_TEX0;

   mmesa->Setup[MGA_CTXREG_DWGCTL] &= DC_opcod_MASK;
   mmesa->Setup[MGA_CTXREG_DWGCTL] |= (ctx->Texture.ReallyEnabled 
				       ? DC_opcod_texture_trap 
				       : DC_opcod_trap);
}




static void mgaDDTexEnv( GLcontext *ctx, GLenum target, 
			 GLenum pname, const GLfloat *param ) 
{
   mgaContextPtr mmesa = MGA_CONTEXT(ctx);


   if (pname == GL_TEXTURE_ENV_MODE) {
      /* force the texture state to be updated */
      FLUSH_BATCH( MGA_CONTEXT(ctx) );
      MGA_CONTEXT(ctx)->new_state |= (MGA_NEW_TEXTURE | 
				      MGA_NEW_ALPHA);
   }
   else if (pname == GL_TEXTURE_ENV_COLOR)
   {
      struct gl_texture_unit *texUnit = 
	 &ctx->Texture.Unit[ctx->Texture.CurrentUnit];
      GLfloat *fc = texUnit->EnvColor;
      GLubyte c[4];
      GLuint col;
		
      COPY_4V(c, fc);
      col = mgaPackColor( mmesa->mgaScreen->cpp, c[0], c[1], c[2], c[3] );
      mmesa->envcolor = (c[3]<<24) | (c[0]<<16) | (c[1]<<8) | (c[2]); 
    
      if (mmesa->Setup[MGA_CTXREG_FCOL] != col) {
	 FLUSH_BATCH(mmesa);	
	 mmesa->Setup[MGA_CTXREG_FCOL] = col;      
	 mmesa->dirty |= MGA_UPLOAD_CTX;

	 mmesa->blend_flags &= ~MGA_BLEND_ENV_COLOR;

	 /* Actually just require all four components to be
	  * equal.  This permits a single-pass GL_BLEND.
	  * 
	  * More complex multitexture/multipass fallbacks
	  * for blend can be done later.
	  */
	 if (mmesa->envcolor != 0x0 && mmesa->envcolor != 0xffffffff)
	    mmesa->blend_flags |= MGA_BLEND_ENV_COLOR;
      }
   }
}


static void mgaDDTexImage( GLcontext *ctx, GLenum target,
			   struct gl_texture_object *tObj, GLint level,
			   GLint internalFormat,
			   const struct gl_texture_image *image ) 
{
   mgaContextPtr mmesa = MGA_CONTEXT( ctx );	
   mgaTextureObjectPtr t;

   /* just free the mga texture if it exists, it will be recreated at
      mgaUpdateTextureState time. */  
   t = (mgaTextureObjectPtr) tObj->DriverData;
   if ( t ) {
      if (t->bound) FLUSH_BATCH(mmesa);
      /* if this is the current object, it will force an update */
      mgaDestroyTexObj( mmesa, t );
      mmesa->new_state |= MGA_NEW_TEXTURE;
   }

   if (0)
      fprintf(stderr, "mgaDDTexImage tObj %p, level %d, image %p\n",
	      tObj, level, image);
   
}

static void mgaDDTexSubImage( GLcontext *ctx, GLenum target,
			      struct gl_texture_object *tObj, GLint level,
			      GLint xoffset, GLint yoffset,
			      GLsizei width, GLsizei height,
			      GLint internalFormat,
			      const struct gl_texture_image *image ) 
{
   mgaContextPtr mmesa = MGA_CONTEXT( ctx );
   mgaTextureObjectPtr t;

   t = (mgaTextureObjectPtr) tObj->DriverData;


   /* just free the mga texture if it exists, it will be recreated at
      mgaUpdateTextureState time. */  
   t = (mgaTextureObjectPtr) tObj->DriverData;
   if ( t ) {
      if (t->bound) FLUSH_BATCH(mmesa);
      /* if this is the current object, it will force an update */
      mgaDestroyTexObj( mmesa, t );
      mmesa->new_state |= MGA_NEW_TEXTURE;
   }



#if 0
   /* the texture currently exists, so directly update it */
   mgaUploadSubImage( t, level, xoffset, yoffset, width, height );
#endif
}



/*
 * mgaTexParameter
 * This just changes variables and flags for a state update, which
 * will happen at the next mgaUpdateTextureState
 */
static void 
mgaDDTexParameter( GLcontext *ctx, GLenum target,
		   struct gl_texture_object *tObj,
		   GLenum pname, const GLfloat *params )
{
   mgaContextPtr mmesa = MGA_CONTEXT( ctx );
   mgaTextureObjectPtr t;

   t = (mgaTextureObjectPtr) tObj->DriverData;

   /* if we don't have a hardware texture, it will be automatically
      created with current state before it is used, so we don't have
      to do anything now */
   if ( !t || !t->bound || target != GL_TEXTURE_2D ) {
      return;
   }

   switch (pname) {
   case GL_TEXTURE_MIN_FILTER:
   case GL_TEXTURE_MAG_FILTER:
      FLUSH_BATCH(mmesa);
      mgaSetTexFilter( t, tObj->MinFilter, tObj->MagFilter );
      break;

   case GL_TEXTURE_WRAP_S:
   case GL_TEXTURE_WRAP_T:
      FLUSH_BATCH(mmesa);
      mgaSetTexWrapping(t,tObj->WrapS,tObj->WrapT);
      break;
  
   case GL_TEXTURE_BORDER_COLOR:
      FLUSH_BATCH(mmesa);
      mgaSetTexBorderColor(t,tObj->BorderColor);
      break;

   default:
      return;
   }

   mmesa->new_state |= MGA_NEW_TEXTURE;
}


static void 
mgaDDBindTexture( GLcontext *ctx, GLenum target,
		  struct gl_texture_object *tObj ) 
{
   mgaContextPtr mmesa = MGA_CONTEXT( ctx );
   int unit = ctx->Texture.CurrentUnit;

   FLUSH_BATCH(mmesa);

   if (mmesa->CurrentTexObj[unit]) {
      mmesa->CurrentTexObj[unit]->bound &= ~(unit+1);
      mmesa->CurrentTexObj[unit] = 0;  
   }

   /* force the texture state to be updated 
    */
   MGA_CONTEXT(ctx)->new_state |= MGA_NEW_TEXTURE;
}


static void 
mgaDDDeleteTexture( GLcontext *ctx, struct gl_texture_object *tObj ) 
{
   mgaContextPtr mmesa = MGA_CONTEXT( ctx );
   mgaTextureObjectPtr t = (mgaTextureObjectPtr)tObj->DriverData;

   if ( t ) {
      if (t->bound) {
	 FLUSH_BATCH(mmesa);
	 if (t->bound & TEX_0) mmesa->CurrentTexObj[0] = 0;
	 if (t->bound & TEX_1) mmesa->CurrentTexObj[1] = 0;
	 mmesa->new_state |= MGA_NEW_TEXTURE;
      }

      mgaDestroyTexObj( mmesa, t );
      mmesa->new_state |= MGA_NEW_TEXTURE;
   }
}


static GLboolean 
mgaDDIsTextureResident( GLcontext *ctx, struct gl_texture_object *t ) 
{
   mgaTextureObjectPtr mt = (mgaTextureObjectPtr)t->DriverData;
   return mt && mt->MemBlock;
}


void 
mgaDDInitTextureFuncs( GLcontext *ctx )
{
   ctx->Driver.TexEnv = mgaDDTexEnv;
   ctx->Driver.TexImage = mgaDDTexImage;
   ctx->Driver.TexSubImage = mgaDDTexSubImage;
   ctx->Driver.BindTexture = mgaDDBindTexture;
   ctx->Driver.DeleteTexture = mgaDDDeleteTexture;
   ctx->Driver.TexParameter = mgaDDTexParameter;
   ctx->Driver.UpdateTexturePalette = 0;
   ctx->Driver.IsTextureResident = mgaDDIsTextureResident;
}

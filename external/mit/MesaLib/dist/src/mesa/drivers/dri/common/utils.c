/*
 * (C) Copyright IBM Corporation 2002, 2004
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
 * VA LINUX SYSTEM, IBM AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/**
 * \file utils.c
 * Utility functions for DRI drivers.
 *
 * \author Ian Romanick <idr@us.ibm.com>
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include "main/macros.h"
#include "main/mtypes.h"
#include "main/cpuinfo.h"
#include "main/extensions.h"
#include "utils.h"
#include "dri_util.h"

/* WARNING: HACK: Local defines to avoid pulling glx.h.
 *
 * Any parts of this file that use the following defines are either partial or
 * entirely broken wrt EGL.
 *
 * For example any getConfigAttrib() or indexConfigAttrib() query from EGL for
 * SLOW or NON_CONFORMANT_CONFIG will not work as expected since the EGL tokens
 * are different from the GLX ones.
 */
#define GLX_NONE                                                0x8000
#define GLX_SLOW_CONFIG                                         0x8001
#define GLX_NON_CONFORMANT_CONFIG                               0x800D
#define GLX_DONT_CARE                                           0xFFFFFFFF

/**
 * Create the \c GL_RENDERER string for DRI drivers.
 * 
 * Almost all DRI drivers use a \c GL_RENDERER string of the form:
 *
 *    "Mesa DRI <chip> <driver date> <AGP speed) <CPU information>"
 *
 * Using the supplied chip name, driver data, and AGP speed, this function
 * creates the string.
 * 
 * \param buffer         Buffer to hold the \c GL_RENDERER string.
 * \param hardware_name  Name of the hardware.
 * \param agp_mode       AGP mode (speed).
 * 
 * \returns
 * The length of the string stored in \c buffer.  This does \b not include
 * the terminating \c NUL character.
 */
unsigned
driGetRendererString( char * buffer, const char * hardware_name,
		      GLuint agp_mode )
{
   unsigned offset;
   char *cpu;

   offset = sprintf( buffer, "Mesa DRI %s", hardware_name );

   /* Append any AGP-specific information.
    */
   switch ( agp_mode ) {
   case 1:
   case 2:
   case 4:
   case 8:
      offset += sprintf( & buffer[ offset ], " AGP %ux", agp_mode );
      break;
	
   default:
      break;
   }

   /* Append any CPU-specific information.
    */
   cpu = _mesa_get_cpu_string();
   if (cpu) {
      offset += sprintf(buffer + offset, " %s", cpu);
      free(cpu);
   }

   return offset;
}


/**
 * Creates a set of \c struct gl_config that a driver will expose.
 * 
 * A set of \c struct gl_config will be created based on the supplied
 * parameters.  The number of modes processed will be 2 *
 * \c num_depth_stencil_bits * \c num_db_modes.
 * 
 * For the most part, data is just copied from \c depth_bits, \c stencil_bits,
 * \c db_modes, and \c visType into each \c struct gl_config element.
 * However, the meanings of \c fb_format and \c fb_type require further
 * explanation.  The \c fb_format specifies which color components are in
 * each pixel and what the default order is.  For example, \c GL_RGB specifies
 * that red, green, blue are available and red is in the "most significant"
 * position and blue is in the "least significant".  The \c fb_type specifies
 * the bit sizes of each component and the actual ordering.  For example, if
 * \c GL_UNSIGNED_SHORT_5_6_5_REV is specified with \c GL_RGB, bits [15:11]
 * are the blue value, bits [10:5] are the green value, and bits [4:0] are
 * the red value.
 * 
 * One sublte issue is the combination of \c GL_RGB  or \c GL_BGR and either
 * of the \c GL_UNSIGNED_INT_8_8_8_8 modes.  The resulting mask values in the
 * \c struct gl_config structure is \b identical to the \c GL_RGBA or
 * \c GL_BGRA case, except the \c alphaMask is zero.  This means that, as
 * far as this routine is concerned, \c GL_RGB with \c GL_UNSIGNED_INT_8_8_8_8
 * still uses 32-bits.
 *
 * If in doubt, look at the tables used in the function.
 * 
 * \param ptr_to_modes  Pointer to a pointer to a linked list of
 *                      \c struct gl_config.  Upon completion, a pointer to
 *                      the next element to be process will be stored here.
 *                      If the function fails and returns \c GL_FALSE, this
 *                      value will be unmodified, but some elements in the
 *                      linked list may be modified.
 * \param format        Mesa mesa_format enum describing the pixel format
 * \param depth_bits    Array of depth buffer sizes to be exposed.
 * \param stencil_bits  Array of stencil buffer sizes to be exposed.
 * \param num_depth_stencil_bits  Number of entries in both \c depth_bits and
 *                      \c stencil_bits.
 * \param db_modes      Array of buffer swap modes.  If an element has a
 *                      value of \c __DRI_ATTRIB_SWAP_NONE, then it
 *                      represents a single-buffered mode.  Other valid
 *                      values are \c __DRI_ATTRIB_SWAP_EXCHANGE,
 *                      \c __DRI_ATTRIB_SWAP_COPY, and \c __DRI_ATTRIB_SWAP_UNDEFINED.
 *                      They represent the respective GLX values as in
 *                      the GLX_OML_swap_method extension spec.
 * \param num_db_modes  Number of entries in \c db_modes.
 * \param msaa_samples  Array of msaa sample count. 0 represents a visual
 *                      without a multisample buffer.
 * \param num_msaa_modes Number of entries in \c msaa_samples.
 * \param enable_accum  Add an accum buffer to the configs
 * \param color_depth_match Whether the color depth must match the zs depth
 *                          This forces 32-bit color to have 24-bit depth, and
 *                          16-bit color to have 16-bit depth.
 *
 * \returns
 * Pointer to any array of pointers to the \c __DRIconfig structures created
 * for the specified formats.  If there is an error, \c NULL is returned.
 * Currently the only cause of failure is a bad parameter (i.e., unsupported
 * \c format).
 */
__DRIconfig **
driCreateConfigs(mesa_format format,
		 const uint8_t * depth_bits, const uint8_t * stencil_bits,
		 unsigned num_depth_stencil_bits,
		 const GLenum * db_modes, unsigned num_db_modes,
		 const uint8_t * msaa_samples, unsigned num_msaa_modes,
		 GLboolean enable_accum, GLboolean color_depth_match)
{
   static const struct {
      uint32_t masks[4];
      int shifts[4];
   } format_table[] = {
      /* MESA_FORMAT_B5G6R5_UNORM */
      {{ 0x0000F800, 0x000007E0, 0x0000001F, 0x00000000 },
       { 11, 5, 0, -1 }},
      /* MESA_FORMAT_B8G8R8X8_UNORM */
      {{ 0x00FF0000, 0x0000FF00, 0x000000FF, 0x00000000 },
       { 16, 8, 0, -1 }},
      /* MESA_FORMAT_B8G8R8A8_UNORM */
      {{ 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000 },
       { 16, 8, 0, 24 }},
      /* MESA_FORMAT_B10G10R10X2_UNORM */
      {{ 0x3FF00000, 0x000FFC00, 0x000003FF, 0x00000000 },
       { 20, 10, 0, -1 }},
      /* MESA_FORMAT_B10G10R10A2_UNORM */
      {{ 0x3FF00000, 0x000FFC00, 0x000003FF, 0xC0000000 },
       { 20, 10, 0, 30 }},
      /* MESA_FORMAT_R8G8B8A8_UNORM */
      {{ 0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000 },
       { 0, 8, 16, 24 }},
      /* MESA_FORMAT_R8G8B8X8_UNORM */
      {{ 0x000000FF, 0x0000FF00, 0x00FF0000, 0x00000000 },
       { 0, 8, 16, -1 }},
      /* MESA_FORMAT_R10G10B10X2_UNORM */
      {{ 0x000003FF, 0x000FFC00, 0x3FF00000, 0x00000000 },
       { 0, 10, 20, -1 }},
      /* MESA_FORMAT_R10G10B10A2_UNORM */
      {{ 0x000003FF, 0x000FFC00, 0x3FF00000, 0xC0000000 },
       { 0, 10, 20, 30 }},
      /* MESA_FORMAT_RGBX_FLOAT16 */
      {{ 0, 0, 0, 0},
       { 0, 16, 32, -1 }},
      /* MESA_FORMAT_RGBA_FLOAT16 */
      {{ 0, 0, 0, 0},
       { 0, 16, 32, 48 }},
   };

   const uint32_t * masks;
   const int * shifts;
   __DRIconfig **configs, **c;
   struct gl_config *modes;
   unsigned i, j, k, h;
   unsigned num_modes;
   unsigned num_accum_bits = (enable_accum) ? 2 : 1;
   int red_bits;
   int green_bits;
   int blue_bits;
   int alpha_bits;
   bool is_srgb;
   bool is_float;

   switch (format) {
   case MESA_FORMAT_B5G6R5_UNORM:
      masks = format_table[0].masks;
      shifts = format_table[0].shifts;
      break;
   case MESA_FORMAT_B8G8R8X8_UNORM:
   case MESA_FORMAT_B8G8R8X8_SRGB:
      masks = format_table[1].masks;
      shifts = format_table[1].shifts;
      break;
   case MESA_FORMAT_B8G8R8A8_UNORM:
   case MESA_FORMAT_B8G8R8A8_SRGB:
      masks = format_table[2].masks;
      shifts = format_table[2].shifts;
      break;
   case MESA_FORMAT_R8G8B8A8_UNORM:
   case MESA_FORMAT_R8G8B8A8_SRGB:
      masks = format_table[5].masks;
      shifts = format_table[5].shifts;
      break;
   case MESA_FORMAT_R8G8B8X8_UNORM:
   case MESA_FORMAT_R8G8B8X8_SRGB:
      masks = format_table[6].masks;
      shifts = format_table[6].shifts;
      break;
   case MESA_FORMAT_B10G10R10X2_UNORM:
      masks = format_table[3].masks;
      shifts = format_table[3].shifts;
      break;
   case MESA_FORMAT_B10G10R10A2_UNORM:
      masks = format_table[4].masks;
      shifts = format_table[4].shifts;
      break;
   case MESA_FORMAT_RGBX_FLOAT16:
      masks = format_table[9].masks;
      shifts = format_table[9].shifts;
      break;
   case MESA_FORMAT_RGBA_FLOAT16:
      masks = format_table[10].masks;
      shifts = format_table[10].shifts;
      break;
   case MESA_FORMAT_R10G10B10X2_UNORM:
      masks = format_table[7].masks;
      shifts = format_table[7].shifts;
      break;
   case MESA_FORMAT_R10G10B10A2_UNORM:
      masks = format_table[8].masks;
      shifts = format_table[8].shifts;
      break;
   default:
      fprintf(stderr, "[%s:%u] Unknown framebuffer type %s (%d).\n",
              __func__, __LINE__,
              _mesa_get_format_name(format), format);
      return NULL;
   }

   red_bits = _mesa_get_format_bits(format, GL_RED_BITS);
   green_bits = _mesa_get_format_bits(format, GL_GREEN_BITS);
   blue_bits = _mesa_get_format_bits(format, GL_BLUE_BITS);
   alpha_bits = _mesa_get_format_bits(format, GL_ALPHA_BITS);
   is_srgb = _mesa_is_format_srgb(format);
   is_float = _mesa_get_format_datatype(format) == GL_FLOAT;

   num_modes = num_depth_stencil_bits * num_db_modes * num_accum_bits * num_msaa_modes;
   configs = calloc(num_modes + 1, sizeof *configs);
   if (configs == NULL)
       return NULL;

    c = configs;
    for ( k = 0 ; k < num_depth_stencil_bits ; k++ ) {
	for ( i = 0 ; i < num_db_modes ; i++ ) {
	    for ( h = 0 ; h < num_msaa_modes; h++ ) {
	    	for ( j = 0 ; j < num_accum_bits ; j++ ) {
		    if (color_depth_match &&
			(depth_bits[k] || stencil_bits[k])) {
			/* Depth can really only be 0, 16, 24, or 32. A 32-bit
			 * color format still matches 24-bit depth, as there
			 * is an implicit 8-bit stencil. So really we just
			 * need to make sure that color/depth are both 16 or
			 * both non-16.
			 */
			if ((depth_bits[k] + stencil_bits[k] == 16) !=
			    (red_bits + green_bits + blue_bits + alpha_bits == 16))
			    continue;
		    }

		    *c = malloc (sizeof **c);
		    modes = &(*c)->modes;
		    c++;

		    memset(modes, 0, sizeof *modes);
		    modes->floatMode = is_float;
		    modes->redBits   = red_bits;
		    modes->greenBits = green_bits;
		    modes->blueBits  = blue_bits;
		    modes->alphaBits = alpha_bits;
		    modes->redMask   = masks[0];
		    modes->greenMask = masks[1];
		    modes->blueMask  = masks[2];
		    modes->alphaMask = masks[3];
		    modes->redShift   = shifts[0];
		    modes->greenShift = shifts[1];
		    modes->blueShift  = shifts[2];
		    modes->alphaShift = shifts[3];
		    modes->rgbBits   = modes->redBits + modes->greenBits
		    	+ modes->blueBits + modes->alphaBits;

		    modes->accumRedBits   = 16 * j;
		    modes->accumGreenBits = 16 * j;
		    modes->accumBlueBits  = 16 * j;
		    modes->accumAlphaBits = 16 * j;

		    modes->stencilBits = stencil_bits[k];
		    modes->depthBits = depth_bits[k];

		    if (db_modes[i] == __DRI_ATTRIB_SWAP_NONE) {
		    	modes->doubleBufferMode = GL_FALSE;
		        modes->swapMethod = __DRI_ATTRIB_SWAP_UNDEFINED;
		    }
		    else {
		    	modes->doubleBufferMode = GL_TRUE;
		    	modes->swapMethod = db_modes[i];
		    }

		    modes->samples = msaa_samples[h];

		    modes->sRGBCapable = is_srgb;
		}
	    }
	}
    }
    *c = NULL;

    return configs;
}

__DRIconfig **driConcatConfigs(__DRIconfig **a,
			       __DRIconfig **b)
{
    __DRIconfig **all;
    int i, j, index;

    if (a == NULL || a[0] == NULL)
       return b;
    else if (b == NULL || b[0] == NULL)
       return a;

    i = 0;
    while (a[i] != NULL)
	i++;
    j = 0;
    while (b[j] != NULL)
	j++;
   
    all = malloc((i + j + 1) * sizeof *all);
    index = 0;
    for (i = 0; a[i] != NULL; i++)
	all[index++] = a[i];
    for (j = 0; b[j] != NULL; j++)
	all[index++] = b[j];
    all[index++] = NULL;

    free(a);
    free(b);

    return all;
}

#define __ATTRIB(attrib, field) case attrib: *value = config->modes.field; break

/**
 * Return the value of a configuration attribute.  The attribute is
 * indicated by the index.
 */
static int
driGetConfigAttribIndex(const __DRIconfig *config,
			unsigned int index, unsigned int *value)
{
    switch (index + 1) {
    __ATTRIB(__DRI_ATTRIB_BUFFER_SIZE,			rgbBits);
    __ATTRIB(__DRI_ATTRIB_RED_SIZE,			redBits);
    __ATTRIB(__DRI_ATTRIB_GREEN_SIZE,			greenBits);
    __ATTRIB(__DRI_ATTRIB_BLUE_SIZE,			blueBits);
    case __DRI_ATTRIB_LEVEL:
    case __DRI_ATTRIB_LUMINANCE_SIZE:
    case __DRI_ATTRIB_AUX_BUFFERS:
        *value = 0;
        break;
    __ATTRIB(__DRI_ATTRIB_ALPHA_SIZE,			alphaBits);
    case __DRI_ATTRIB_ALPHA_MASK_SIZE:
        /* I have no idea what this value was ever meant to mean, it's
         * never been set to anything, just say 0.
         */
        *value = 0;
        break;
    __ATTRIB(__DRI_ATTRIB_DEPTH_SIZE,			depthBits);
    __ATTRIB(__DRI_ATTRIB_STENCIL_SIZE,			stencilBits);
    __ATTRIB(__DRI_ATTRIB_ACCUM_RED_SIZE,		accumRedBits);
    __ATTRIB(__DRI_ATTRIB_ACCUM_GREEN_SIZE,		accumGreenBits);
    __ATTRIB(__DRI_ATTRIB_ACCUM_BLUE_SIZE,		accumBlueBits);
    __ATTRIB(__DRI_ATTRIB_ACCUM_ALPHA_SIZE,		accumAlphaBits);
    case __DRI_ATTRIB_SAMPLE_BUFFERS:
        *value = !!config->modes.samples;
        break;
    __ATTRIB(__DRI_ATTRIB_SAMPLES,			samples);
    case __DRI_ATTRIB_RENDER_TYPE:
        /* no support for color index mode */
	*value = __DRI_ATTRIB_RGBA_BIT;
        if (config->modes.floatMode)
            *value |= __DRI_ATTRIB_FLOAT_BIT;
	break;
    case __DRI_ATTRIB_CONFIG_CAVEAT:
	if (config->modes.accumRedBits != 0)
	    *value = __DRI_ATTRIB_SLOW_BIT;
	else
	    *value = 0;
	break;
    case __DRI_ATTRIB_CONFORMANT:
        *value = GL_TRUE;
        break;
    __ATTRIB(__DRI_ATTRIB_DOUBLE_BUFFER,		doubleBufferMode);
    __ATTRIB(__DRI_ATTRIB_STEREO,			stereoMode);
    case __DRI_ATTRIB_TRANSPARENT_TYPE:
    case __DRI_ATTRIB_TRANSPARENT_INDEX_VALUE: /* horrible bc hack */
        *value = GLX_NONE;
        break;
    case __DRI_ATTRIB_TRANSPARENT_RED_VALUE:
    case __DRI_ATTRIB_TRANSPARENT_GREEN_VALUE:
    case __DRI_ATTRIB_TRANSPARENT_BLUE_VALUE:
    case __DRI_ATTRIB_TRANSPARENT_ALPHA_VALUE:
        *value = GLX_DONT_CARE;
        break;
    case __DRI_ATTRIB_FLOAT_MODE:
        *value = config->modes.floatMode;
        break;
    __ATTRIB(__DRI_ATTRIB_RED_MASK,			redMask);
    __ATTRIB(__DRI_ATTRIB_GREEN_MASK,			greenMask);
    __ATTRIB(__DRI_ATTRIB_BLUE_MASK,			blueMask);
    __ATTRIB(__DRI_ATTRIB_ALPHA_MASK,			alphaMask);
    case __DRI_ATTRIB_MAX_PBUFFER_WIDTH:
    case __DRI_ATTRIB_MAX_PBUFFER_HEIGHT:
    case __DRI_ATTRIB_MAX_PBUFFER_PIXELS:
    case __DRI_ATTRIB_OPTIMAL_PBUFFER_WIDTH:
    case __DRI_ATTRIB_OPTIMAL_PBUFFER_HEIGHT:
    case __DRI_ATTRIB_VISUAL_SELECT_GROUP:
        *value = 0;
        break;
    __ATTRIB(__DRI_ATTRIB_SWAP_METHOD,			swapMethod);
    case __DRI_ATTRIB_MAX_SWAP_INTERVAL:
        *value = INT_MAX;
        break;
    case __DRI_ATTRIB_MIN_SWAP_INTERVAL:
        *value = 0;
        break;
    case __DRI_ATTRIB_BIND_TO_TEXTURE_RGB:
    case __DRI_ATTRIB_BIND_TO_TEXTURE_RGBA:
    case __DRI_ATTRIB_YINVERTED:
        *value = GL_TRUE;
        break;
    case __DRI_ATTRIB_BIND_TO_MIPMAP_TEXTURE:
        *value = GL_FALSE;
        break;
    case __DRI_ATTRIB_BIND_TO_TEXTURE_TARGETS:
        *value = __DRI_ATTRIB_TEXTURE_1D_BIT |
                 __DRI_ATTRIB_TEXTURE_2D_BIT |
                 __DRI_ATTRIB_TEXTURE_RECTANGLE_BIT;
        break;
    __ATTRIB(__DRI_ATTRIB_FRAMEBUFFER_SRGB_CAPABLE,	sRGBCapable);
    case __DRI_ATTRIB_MUTABLE_RENDER_BUFFER:
        *value = GL_FALSE;
        break;
    __ATTRIB(__DRI_ATTRIB_RED_SHIFT,			redShift);
    __ATTRIB(__DRI_ATTRIB_GREEN_SHIFT,			greenShift);
    __ATTRIB(__DRI_ATTRIB_BLUE_SHIFT,			blueShift);
    __ATTRIB(__DRI_ATTRIB_ALPHA_SHIFT,			alphaShift);
    default:
        /* XXX log an error or smth */
        return GL_FALSE;
    }

    return GL_TRUE;
}

/**
 * Get the value of a configuration attribute.
 * \param attrib  the attribute (one of the _DRI_ATTRIB_x tokens)
 * \param value  returns the attribute's value
 * \return 1 for success, 0 for failure
 */
int
driGetConfigAttrib(const __DRIconfig *config,
		   unsigned int attrib, unsigned int *value)
{
    return driGetConfigAttribIndex(config, attrib - 1, value);
}


/**
 * Get a configuration attribute name and value, given an index.
 * \param index  which field of the __DRIconfig to query
 * \param attrib  returns the attribute name (one of the _DRI_ATTRIB_x tokens)
 * \param value  returns the attribute's value
 * \return 1 for success, 0 for failure
 */
int
driIndexConfigAttrib(const __DRIconfig *config, int index,
		     unsigned int *attrib, unsigned int *value)
{
    if (driGetConfigAttribIndex(config, index, value)) {
        *attrib = index + 1;
        return GL_TRUE;
    }

    return GL_FALSE;
}

/**
 * Implement queries for values that are common across all Mesa drivers
 *
 * Currently only the following queries are supported by this function:
 *
 *     - \c __DRI2_RENDERER_VERSION
 *     - \c __DRI2_RENDERER_PREFERRED_PROFILE
 *     - \c __DRI2_RENDERER_OPENGL_CORE_PROFILE_VERSION
 *     - \c __DRI2_RENDERER_OPENGL_COMPATIBLITY_PROFILE_VERSION
 *     - \c __DRI2_RENDERER_ES_PROFILE_VERSION
 *     - \c __DRI2_RENDERER_ES2_PROFILE_VERSION
 *
 * \returns
 * Zero if a recognized value of \c param is supplied, -1 otherwise.
 */
int
driQueryRendererIntegerCommon(__DRIscreen *psp, int param, unsigned int *value)
{
   switch (param) {
   case __DRI2_RENDERER_VERSION: {
      static const char *const ver = PACKAGE_VERSION;
      char *endptr;
      int v[3];

      v[0] = strtol(ver, &endptr, 10);
      assert(endptr[0] == '.');
      if (endptr[0] != '.')
         return -1;

      v[1] = strtol(endptr + 1, &endptr, 10);
      assert(endptr[0] == '.');
      if (endptr[0] != '.')
         return -1;

      v[2] = strtol(endptr + 1, &endptr, 10);

      value[0] = v[0];
      value[1] = v[1];
      value[2] = v[2];
      return 0;
   }
   case __DRI2_RENDERER_PREFERRED_PROFILE:
      value[0] = (psp->max_gl_core_version != 0)
         ? (1U << __DRI_API_OPENGL_CORE) : (1U << __DRI_API_OPENGL);
      return 0;
   case __DRI2_RENDERER_OPENGL_CORE_PROFILE_VERSION:
      value[0] = psp->max_gl_core_version / 10;
      value[1] = psp->max_gl_core_version % 10;
      return 0;
   case __DRI2_RENDERER_OPENGL_COMPATIBILITY_PROFILE_VERSION:
      value[0] = psp->max_gl_compat_version / 10;
      value[1] = psp->max_gl_compat_version % 10;
      return 0;
   case __DRI2_RENDERER_OPENGL_ES_PROFILE_VERSION:
      value[0] = psp->max_gl_es1_version / 10;
      value[1] = psp->max_gl_es1_version % 10;
      return 0;
   case __DRI2_RENDERER_OPENGL_ES2_PROFILE_VERSION:
      value[0] = psp->max_gl_es2_version / 10;
      value[1] = psp->max_gl_es2_version % 10;
      return 0;
   default:
      break;
   }

   return -1;
}

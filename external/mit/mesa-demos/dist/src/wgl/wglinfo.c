/*
 * Copyright (C) 2009  VMware, Inc.
 * Copyright (C) 1999-2006  Brian Paul
 * All Rights Reserved.
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
 * THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
 * AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */


/*
 * This program is a work-alike of the GLX glxinfo program.
 * Command line options:
 *  -t                     print wide table
 *  -v                     print verbose information
 *  -b                     only print ID of "best" visual on screen 0
 *  -l                     print interesting OpenGL limits (added 5 Sep 2002)
 */


#include <windows.h>
#include <stdbool.h>
#include <GL/glew.h>
#include <GL/wglew.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "glinfo_common.h"


static GLboolean have_WGL_ARB_create_context;
static GLboolean have_WGL_ARB_pbuffer;
static GLboolean have_WGL_ARB_pixel_format;
static GLboolean have_WGL_ARB_multisample;
static GLboolean have_WGL_ARB_framebuffer_sRGB; /* or EXT version */

static PFNWGLGETPIXELFORMATATTRIBIVARBPROC wglGetPixelFormatAttribivARB_func;
static PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB_func;


/**
 * An extension of PIXELFORMATDESCRIPTOR to handle multisample, etc.
 */
struct format_info {
   PIXELFORMATDESCRIPTOR pfd;
   int sampleBuffers, numSamples;
   int transparency;
   bool floatComponents;
   bool srgb;
   bool draw_to_bitmap;
   bool draw_to_pbuffer;
   bool gdi_drawing;
};


static LRESULT CALLBACK
WndProc(HWND hWnd,
        UINT uMsg,
        WPARAM wParam,
        LPARAM lParam )
{
   switch (uMsg) {
   case WM_DESTROY:
      PostQuitMessage(0);
      break;
   default:
      return DefWindowProc(hWnd, uMsg, wParam, lParam);
   }

   return 0;
}


static void
print_screen_info(HDC _hdc, const struct options *opts, GLboolean coreProfile)
{
   WNDCLASS wc;
   HWND win;
   HGLRC ctx;
   int visinfo;
   HDC hdc;
   PIXELFORMATDESCRIPTOR pfd;
   int version;
   const char *oglString = "OpenGL";

   memset(&wc, 0, sizeof wc);
   wc.hbrBackground = (HBRUSH) (COLOR_BTNFACE + 1);
   wc.hCursor = LoadCursor(NULL, IDC_ARROW);
   wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
   wc.lpfnWndProc = WndProc;
   wc.lpszClassName = "wglinfo";
   wc.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
   RegisterClass(&wc);

   win = CreateWindowEx(0,
                        wc.lpszClassName,
                        "wglinfo",
                        WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
                        CW_USEDEFAULT,
                        CW_USEDEFAULT,
                        CW_USEDEFAULT,
                        CW_USEDEFAULT,
                        NULL,
                        NULL,
                        wc.hInstance,
                        NULL);
   if (!win) {
      fprintf(stderr, "Couldn't create window\n");
      return;
   }

   hdc = GetDC(win);
   if (!hdc) {
      fprintf(stderr, "Couldn't obtain HDC\n");
      return;
   }

   memset(&pfd, 0, sizeof(pfd));
   pfd.cColorBits = 3;
   pfd.cRedBits = 1;
   pfd.cGreenBits = 1;
   pfd.cBlueBits = 1;
   pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL;
   pfd.iLayerType = PFD_MAIN_PLANE;
   pfd.iPixelType = PFD_TYPE_RGBA;
   pfd.nSize = sizeof(pfd);
   pfd.nVersion = 1;

   visinfo = ChoosePixelFormat(hdc, &pfd);
   if (!visinfo) {
      pfd.dwFlags |= PFD_DOUBLEBUFFER;
      visinfo = ChoosePixelFormat(hdc, &pfd);
   }

   if (!visinfo) {
      fprintf(stderr, "Error: couldn't find RGB WGL visual\n");
      return;
   }

   SetPixelFormat(hdc, visinfo, &pfd);
   ctx = wglCreateContext(hdc);
   if (!ctx) {
      fprintf(stderr, "Error: wglCreateContext failed\n");
      return;
   }

   if (wglMakeCurrent(hdc, ctx)) {
#if defined(WGL_ARB_extensions_string)
      PFNWGLGETEXTENSIONSSTRINGARBPROC wglGetExtensionsStringARB_func = 
         (PFNWGLGETEXTENSIONSSTRINGARBPROC)wglGetProcAddress("wglGetExtensionsStringARB");
#endif
      const char *glVendor, *glRenderer, *glVersion, *glExtensions;
      const char *wglExtensions = NULL;
      struct ext_functions extfuncs;
      
#if defined(WGL_ARB_extensions_string)
      if (wglGetExtensionsStringARB_func) {
         wglExtensions = wglGetExtensionsStringARB_func(hdc);
         if (extension_supported("WGL_ARB_pbuffer", wglExtensions)) {
            have_WGL_ARB_pbuffer = GL_TRUE;
         }
         if (extension_supported("WGL_ARB_pixel_format", wglExtensions)) {
            have_WGL_ARB_pixel_format = GL_TRUE;
         }
         if (extension_supported("WGL_ARB_multisample", wglExtensions)) {
            have_WGL_ARB_multisample = GL_TRUE;
         }
         if (extension_supported("WGL_ARB_create_context", wglExtensions)) {
            have_WGL_ARB_create_context = GL_TRUE;
         }
         if (extension_supported("WGL_ARB_framebuffer_sRGB", wglExtensions) ||
             extension_supported("WGL_EXT_framebuffer_sRGB", wglExtensions)) {
            have_WGL_ARB_framebuffer_sRGB = GL_TRUE;
         }
      }
#endif

      if (coreProfile && have_WGL_ARB_create_context) {
         /* Try to create a new, core context */
         HGLRC core_ctx = 0;
         int i;

         wglCreateContextAttribsARB_func =
            (PFNWGLCREATECONTEXTATTRIBSARBPROC)
            wglGetProcAddress("wglCreateContextAttribsARB");
         assert(wglCreateContextAttribsARB_func);
         if (!wglCreateContextAttribsARB_func) {
            printf("Failed to get wglCreateContextAttribsARB pointer.");
            return;
         }

         for (i = 0; gl_versions[i].major > 0; i++) {
            int attribs[10], n;

            /* don't bother below GL 3.1 */
            if (gl_versions[i].major == 3 && gl_versions[i].minor == 0) {
               break;
            }

            n = 0;
            attribs[n++] = WGL_CONTEXT_MAJOR_VERSION_ARB;
            attribs[n++] = gl_versions[i].major;
            attribs[n++] = WGL_CONTEXT_MINOR_VERSION_ARB;
            attribs[n++] = gl_versions[i].minor;
            if (gl_versions[i].major * 10 + gl_versions[i].minor > 31) {
               attribs[n++] = WGL_CONTEXT_PROFILE_MASK_ARB;
               attribs[n++] = WGL_CONTEXT_CORE_PROFILE_BIT_ARB;
            }
            attribs[n++] = 0;

            core_ctx = wglCreateContextAttribsARB_func(hdc, 0, attribs);
            if (core_ctx) {
               break;
            }
         }

         if (!core_ctx) {
            printf("Failed to create core profile context.\n");
            return;
         }

         ctx = core_ctx;
         if (!wglMakeCurrent(hdc, ctx)) {
            printf("Failed to bind core profile context.\n");
            return;
         }
         oglString = "OpenGL core profile";
      }
      else {
         coreProfile = GL_FALSE;
      }

      extfuncs.GetProgramivARB = (PFNGLGETPROGRAMIVARBPROC)
         wglGetProcAddress("glGetProgramivARB");
      extfuncs.GetStringi = (PFNGLGETSTRINGIPROC)
         wglGetProcAddress("glGetStringi");
      extfuncs.GetConvolutionParameteriv = (GETCONVOLUTIONPARAMETERIVPROC)
         wglGetProcAddress("glGetConvolutionParameteriv");

      glVendor = (const char *) glGetString(GL_VENDOR);
      glRenderer = (const char *) glGetString(GL_RENDERER);
      glVersion = (const char *) glGetString(GL_VERSION);
      if (coreProfile) {
         glExtensions = build_core_profile_extension_list(&extfuncs);
      }
      else {
         glExtensions = (const char *) glGetString(GL_EXTENSIONS);
      }

      /*
       * Print all the vendor, version, extension strings.
       */

      if (!coreProfile) {
         if (wglExtensions && opts->mode != Brief) {
            printf("WGL extensions:\n");
            print_extension_list(wglExtensions, opts->singleLine);
         }
         printf("OpenGL vendor string: %s\n", glVendor);
         printf("OpenGL renderer string: %s\n", glRenderer);
      }

      printf("%s version string: %s\n", oglString, glVersion);

      version = (glVersion[0] - '0') * 10 + (glVersion[2] - '0');

#ifdef GL_VERSION_2_0
      if (version >= 20) {
         char *v = (char *) glGetString(GL_SHADING_LANGUAGE_VERSION);
         printf("%s shading language version string: %s\n", oglString, v);
      }
#endif
#ifdef GL_VERSION_3_0
      if (version >= 30) {
         GLint flags;
         glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
         printf("%s context flags: %s\n", oglString, context_flags_string(flags));
      }
#endif
#ifdef GL_VERSION_3_2
      if (version >= 32) {
         GLint mask;
         glGetIntegerv(GL_CONTEXT_PROFILE_MASK, &mask);
         printf("%s profile mask: %s\n", oglString, profile_mask_string(mask));
      }
#endif

      if (opts->mode != Brief) {
         printf("%s extensions:\n", oglString);
         print_extension_list(glExtensions, opts->singleLine);
      }

      if (opts->limits) {
         print_limits(glExtensions, oglString, version, &extfuncs);
      }
   }
   else {
      fprintf(stderr, "Error: wglMakeCurrent failed\n");
   }

   DestroyWindow(win);
}


static const char *
visual_render_type_name(BYTE iPixelType)
{
   switch (iPixelType) {
      case PFD_TYPE_RGBA:
         return "rgba";
      case PFD_TYPE_COLORINDEX:
         return "ci";
      default:
         return "";
   }
}

static void
print_visual_attribs_verbose(int iPixelFormat, const struct format_info *info)
{
   printf("Visual ID: %x generic=%d drawToWindow=%d drawToBitmap=%d drawToPBuffer=%d GDI=%d\n",
          iPixelFormat, 
          info->pfd.dwFlags & PFD_GENERIC_FORMAT ? 1 : 0,
          info->pfd.dwFlags & PFD_DRAW_TO_WINDOW ? 1 : 0,
          info->draw_to_bitmap,
          info->draw_to_pbuffer,
          info->gdi_drawing);
   printf("    bufferSize=%d level=%d renderType=%s doubleBuffer=%d stereo=%d\n",
          0 /* info->pfd.bufferSize */, 0 /* info->pfd.level */,
	  visual_render_type_name(info->pfd.iPixelType),
          info->pfd.dwFlags & PFD_DOUBLEBUFFER ? 1 : 0, 
          info->pfd.dwFlags & PFD_STEREO ? 1 : 0);
   printf("    rgba: cRedBits=%d cGreenBits=%d cBlueBits=%d cAlphaBits=%d float=%c sRGB=%c\n",
          info->pfd.cRedBits, info->pfd.cGreenBits,
          info->pfd.cBlueBits, info->pfd.cAlphaBits,
          info->floatComponents ? 'Y' : 'N',
          info->srgb ? 'Y' : 'N');
   printf("    cAuxBuffers=%d cDepthBits=%d cStencilBits=%d\n",
          info->pfd.cAuxBuffers, info->pfd.cDepthBits, info->pfd.cStencilBits);
   printf("    accum: cRedBits=%d cGreenBits=%d cBlueBits=%d cAlphaBits=%d\n",
          info->pfd.cAccumRedBits, info->pfd.cAccumGreenBits,
          info->pfd.cAccumBlueBits, info->pfd.cAccumAlphaBits);
   printf("    multiSample=%d  multiSampleBuffers=%d\n",
          info->numSamples, info->sampleBuffers);
   if (info->pfd.dwFlags & PFD_SWAP_EXCHANGE)
      printf("    swapMethod = Exchange\n");
   else if (info->pfd.dwFlags & PFD_SWAP_COPY)
      printf("    swapMethod = Copy\n");
   else
      printf("    swapMethod = Undefined\n");
}


static void
print_visual_attribs_short_header(void)
{
   printf("    visual   x   bf lv rg d st colorbuffer   sr ax dp st accumbuffer  ms \n");
   printf("  id gen win sp  sz l  ci b ro  r  g  b  a F gb bf th cl  r  g  b  a ns b\n");
   printf("-------------------------------------------------------------------------\n");
}


static void
print_visual_attribs_short(int iPixelFormat, const struct format_info *info)
{
   printf("0x%03x %2d  %2d %2d %3d %2d %c%c %c  %c %2d %2d %2d %2d %c  %c %2d %2d %2d",
          iPixelFormat,
          info->pfd.dwFlags & PFD_GENERIC_FORMAT ? 1 : 0,
          info->pfd.dwFlags & PFD_DRAW_TO_WINDOW ? 1 : 0,
          info->transparency,
          info->pfd.cColorBits,
          0 /* info->pfd.level */,
          info->pfd.iPixelType == PFD_TYPE_RGBA ? 'r' : ' ',
          info->pfd.iPixelType == PFD_TYPE_COLORINDEX ? 'c' : ' ',
          info->pfd.dwFlags & PFD_DOUBLEBUFFER ? 'y' : '.',
          info->pfd.dwFlags & PFD_STEREO ? 'y' : '.',
          info->pfd.cRedBits, info->pfd.cGreenBits,
          info->pfd.cBlueBits, info->pfd.cAlphaBits,
          info->floatComponents ? 'f' : '.',
          info->srgb ? 's' : '.',
          info->pfd.cAuxBuffers,
          info->pfd.cDepthBits,
          info->pfd.cStencilBits
          );

   printf(" %2d %2d %2d %2d %2d %1d\n",
          info->pfd.cAccumRedBits, info->pfd.cAccumGreenBits,
          info->pfd.cAccumBlueBits, info->pfd.cAccumAlphaBits,
          info->numSamples, info->sampleBuffers);
}


static void
print_visual_attribs_long_header(void)
{
 printf("Vis   Vis   Visual Trans  buff lev render DB ste  r   g   b   a      s  aux dep ste  accum buffers  MS   MS \n");
 printf(" ID  Depth   Type  parent size el   type     reo sz  sz  sz  sz flt rgb buf th  ncl  r   g   b   a  num bufs\n");
 printf("------------------------------------------------------------------------------------------------------------\n");
}


static void
print_visual_attribs_long(int iPixelFormat, const struct format_info *info)
{
   printf("0x%3x %2d %11d %2d     %2d %2d  %4s %3d %3d %3d %3d %3d %3d",
          iPixelFormat,
          info->pfd.dwFlags & PFD_GENERIC_FORMAT ? 1 : 0,
          info->pfd.dwFlags & PFD_DRAW_TO_WINDOW ? 1 : 0,
          0,
          0 /* info->pfd.bufferSize */,
          0 /* info->pfd.level */,
          visual_render_type_name(info->pfd.iPixelType),
          info->pfd.dwFlags & PFD_DOUBLEBUFFER ? 1 : 0,
          info->pfd.dwFlags & PFD_STEREO ? 1 : 0,
          info->pfd.cRedBits, info->pfd.cGreenBits,
          info->pfd.cBlueBits, info->pfd.cAlphaBits
          );

   printf("  %c   %c %3d %4d %2d %3d %3d %3d %3d  %2d  %2d\n",
          info->floatComponents ? 'f' : '.',
          info->srgb ? 's' : '.',
          info->pfd.cAuxBuffers,
          info->pfd.cDepthBits,
          info->pfd.cStencilBits,
          info->pfd.cAccumRedBits, info->pfd.cAccumGreenBits,
          info->pfd.cAccumBlueBits, info->pfd.cAccumAlphaBits,
          info->sampleBuffers, info->numSamples
          );
}


/**
 * Wrapper for wglGetPixelFormatAttribivARB()
 * \param attrib  the WGL_* attribute to query
 * \return value of the attribute, or 0 if failure
 */
static int
get_pf_attrib(HDC hdc, int pf, int attrib)
{
   int layer = 0, value;
   assert(have_WGL_ARB_pixel_format);
   if (wglGetPixelFormatAttribivARB_func(hdc, pf, layer, 1, &attrib, &value)) {
      return value;
   }
   else {
      return 0;
   }
}


/**
 * Fill in the format_info fields for the pixel format given by pf.
 */
static GLboolean
get_format_info(HDC hdc, int pf, struct format_info *info)
{
   memset(info, 0, sizeof(*info));

   if (have_WGL_ARB_pixel_format) {
      int swapMethod;

      info->pfd.dwFlags = 0;
      if (get_pf_attrib(hdc, pf, WGL_DRAW_TO_WINDOW_ARB))
         info->pfd.dwFlags |= PFD_DRAW_TO_WINDOW;
      if (!get_pf_attrib(hdc, pf, WGL_ACCELERATION_ARB))
         info->pfd.dwFlags |= PFD_GENERIC_FORMAT;
      if (get_pf_attrib(hdc, pf, WGL_SUPPORT_OPENGL_ARB))
         info->pfd.dwFlags |= PFD_SUPPORT_OPENGL;
      if (get_pf_attrib(hdc, pf, WGL_DOUBLE_BUFFER_ARB))
         info->pfd.dwFlags |= PFD_DOUBLEBUFFER;
      if (get_pf_attrib(hdc, pf, WGL_STEREO_ARB))
         info->pfd.dwFlags |= PFD_STEREO;

      if (get_pf_attrib(hdc, pf, WGL_DRAW_TO_BITMAP_ARB))
         info->draw_to_bitmap = true;
      if (have_WGL_ARB_pbuffer && get_pf_attrib(hdc, pf, WGL_DRAW_TO_PBUFFER_ARB))
         info->draw_to_pbuffer = true;
      if (get_pf_attrib(hdc, pf, WGL_SUPPORT_GDI_ARB))
         info->gdi_drawing = true;

      swapMethod = get_pf_attrib(hdc, pf, WGL_SWAP_METHOD_ARB);
      if (swapMethod == WGL_SWAP_EXCHANGE_ARB)
         info->pfd.dwFlags |= PFD_SWAP_EXCHANGE;
      else if (swapMethod == WGL_SWAP_COPY_ARB)
         info->pfd.dwFlags |= PFD_SWAP_COPY;

      int pixel_type = get_pf_attrib(hdc, pf, WGL_PIXEL_TYPE_ARB);
      if (pixel_type == WGL_TYPE_RGBA_ARB)
         info->pfd.iPixelType = PFD_TYPE_RGBA;
      else if (pixel_type == WGL_TYPE_COLORINDEX_ARB)
         info->pfd.iPixelType = PFD_TYPE_COLORINDEX;
      else if (pixel_type == WGL_TYPE_RGBA_FLOAT_ARB) {
         info->pfd.iPixelType = PFD_TYPE_RGBA;
         info->floatComponents = true;
      }

      info->pfd.cColorBits = get_pf_attrib(hdc, pf, WGL_COLOR_BITS_ARB);
      info->pfd.cRedBits = get_pf_attrib(hdc, pf, WGL_RED_BITS_ARB);
      info->pfd.cGreenBits = get_pf_attrib(hdc, pf, WGL_GREEN_BITS_ARB);
      info->pfd.cBlueBits = get_pf_attrib(hdc, pf, WGL_BLUE_BITS_ARB);
      info->pfd.cAlphaBits = get_pf_attrib(hdc, pf, WGL_ALPHA_BITS_ARB);

      info->pfd.cDepthBits = get_pf_attrib(hdc, pf, WGL_DEPTH_BITS_ARB);
      info->pfd.cStencilBits = get_pf_attrib(hdc, pf, WGL_STENCIL_BITS_ARB);
      info->pfd.cAuxBuffers = get_pf_attrib(hdc, pf, WGL_AUX_BUFFERS_ARB);

      info->pfd.cAccumRedBits = get_pf_attrib(hdc, pf,
                                              WGL_ACCUM_RED_BITS_ARB);
      info->pfd.cAccumGreenBits = get_pf_attrib(hdc, pf,
                                                WGL_ACCUM_GREEN_BITS_ARB);
      info->pfd.cAccumBlueBits = get_pf_attrib(hdc, pf,
                                               WGL_ACCUM_BLUE_BITS_ARB);
      info->pfd.cAccumAlphaBits = get_pf_attrib(hdc, pf,
                                                WGL_ACCUM_ALPHA_BITS_ARB);

      info->sampleBuffers = get_pf_attrib(hdc, pf, WGL_SAMPLE_BUFFERS_ARB);
      info->numSamples = get_pf_attrib(hdc, pf, WGL_SAMPLES_ARB);

      info->transparency = get_pf_attrib(hdc, pf, WGL_TRANSPARENT_ARB);

      if (have_WGL_ARB_framebuffer_sRGB) {
         info->srgb = get_pf_attrib(hdc, pf, WGL_FRAMEBUFFER_SRGB_CAPABLE_ARB);
      }
   }
   else {
      if (!DescribePixelFormat(hdc, pf,
                               sizeof(PIXELFORMATDESCRIPTOR), &info->pfd))
         return GL_FALSE;
   }
   return GL_TRUE;
}



static void
print_visual_info(HDC hdc, InfoMode mode)
{
   struct format_info info;
   int numVisuals, numWglVisuals;
   int i;

   wglGetPixelFormatAttribivARB_func =
      (PFNWGLGETPIXELFORMATATTRIBIVARBPROC)
      wglGetProcAddress("wglGetPixelFormatAttribivARB");

   /* Get number of visuals / pixel formats */
   numVisuals = DescribePixelFormat(hdc, 1,
                                    sizeof(PIXELFORMATDESCRIPTOR), NULL);
   printf("%d Regular pixel formats\n", numVisuals);

   if (have_WGL_ARB_pixel_format) {
      int numExtVisuals = get_pf_attrib(hdc, 0, WGL_NUMBER_PIXEL_FORMATS_ARB);
      printf("%d Regular + Extended pixel formats\n", numExtVisuals);
      numVisuals = numExtVisuals;
   }

   if (numVisuals == 0)
      return;

   numWglVisuals = 0;
   for (i = 0; i < numVisuals; i++) {
      if(!DescribePixelFormat(hdc, i, sizeof(PIXELFORMATDESCRIPTOR), &info.pfd))
	 continue;

      //if(!(info.pfd.dwFlags & PFD_SUPPORT_OPENGL))
      //   continue;

      ++numWglVisuals;
   }

   printf("%d WGL Visuals\n", numWglVisuals);

   if (mode == Normal)
      print_visual_attribs_short_header();
   else if (mode == Wide)
      print_visual_attribs_long_header();

   for (i = 0; i < numVisuals; i++) {
      get_format_info(hdc, i, &info);

      if (mode == Verbose)
	 print_visual_attribs_verbose(i, &info);
      else if (mode == Normal)
         print_visual_attribs_short(i, &info);
      else if (mode == Wide) 
         print_visual_attribs_long(i, &info);
   }
   printf("\n");
}


/*
 * Examine all visuals to find the so-called best one.
 * We prefer deepest RGBA buffer with depth, stencil and accum
 * that has no caveats.
 */
static int
find_best_visual(HDC hdc)
{
#if 0
   XVisualInfo theTemplate;
   XVisualInfo *visuals;
   int numVisuals;
   long mask;
   int i;
   struct visual_attribs bestVis;

   /* get list of all visuals on this screen */
   theTemplate.screen = scrnum;
   mask = VisualScreenMask;
   visuals = XGetVisualInfo(hdc, mask, &theTemplate, &numVisuals);

   /* init bestVis with first visual info */
   get_visual_attribs(hdc, &visuals[0], &bestVis);

   /* try to find a "better" visual */
   for (i = 1; i < numVisuals; i++) {
      struct visual_attribs vis;

      get_visual_attribs(hdc, &visuals[i], &vis);

      /* always skip visuals with caveats */
      if (vis.visualCaveat != GLX_NONE_EXT)
         continue;

      /* see if this vis is better than bestVis */
      if ((!bestVis.supportsGL && vis.supportsGL) ||
          (bestVis.visualCaveat != GLX_NONE_EXT) ||
          (bestVis.iPixelType != vis.iPixelType) ||
          (!bestVis.doubleBuffer && vis.doubleBuffer) ||
          (bestVis.cRedBits < vis.cRedBits) ||
          (bestVis.cGreenBits < vis.cGreenBits) ||
          (bestVis.cBlueBits < vis.cBlueBits) ||
          (bestVis.cAlphaBits < vis.cAlphaBits) ||
          (bestVis.cDepthBits < vis.cDepthBits) ||
          (bestVis.cStencilBits < vis.cStencilBits) ||
          (bestVis.cAccumRedBits < vis.cAccumRedBits)) {
         /* found a better visual */
         bestVis = vis;
      }
   }

   return bestVis.id;
#else
   return 0;
#endif
}



int
main(int argc, char *argv[])
{
   HDC hdc;
   struct options opts;

   parse_args(argc, argv, &opts);

   hdc = CreateDC(TEXT("DISPLAY"), NULL, NULL, NULL);

   if (opts.findBest) {
      int b;
      b = find_best_visual(hdc);
      printf("%d\n", b);
   }
   else {
      print_screen_info(hdc, &opts, GL_FALSE);
      printf("\n");
      print_screen_info(hdc, &opts, GL_TRUE);
      printf("\n");
      if (opts.mode != Brief) {
         print_visual_info(hdc, opts.mode);
      }
   }

   return 0;
}

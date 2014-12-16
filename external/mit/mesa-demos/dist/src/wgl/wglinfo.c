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

#include <GL/glew.h>
#include <GL/wglew.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "glinfo_common.h"


typedef enum
{
   Normal,
   Wide,
   Verbose
} InfoMode;



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
print_screen_info(HDC _hdc, GLboolean limits, GLboolean singleLine)
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
      const char *glVendor = (const char *) glGetString(GL_VENDOR);
      const char *glRenderer = (const char *) glGetString(GL_RENDERER);
      const char *glVersion = (const char *) glGetString(GL_VERSION);
      const char *glExtensions = (const char *) glGetString(GL_EXTENSIONS);
      struct ext_functions extfuncs;
      
#if defined(WGL_ARB_extensions_string)
      if (wglGetExtensionsStringARB_func) {
         const char *wglExtensions = wglGetExtensionsStringARB_func(hdc);
         if(wglExtensions) {
            printf("WGL extensions:\n");
            print_extension_list(wglExtensions, singleLine);
         }
      }
#endif
      printf("OpenGL vendor string: %s\n", glVendor);
      printf("OpenGL renderer string: %s\n", glRenderer);
      printf("OpenGL version string: %s\n", glVersion);
#ifdef GL_VERSION_2_0
      if (glVersion[0] >= '2' && glVersion[1] == '.') {
         char *v = (char *) glGetString(GL_SHADING_LANGUAGE_VERSION);
         printf("OpenGL shading language version string: %s\n", v);
      }
#endif

      extfuncs.GetProgramivARB = (PFNGLGETPROGRAMIVARBPROC)
         wglGetProcAddress("glGetProgramivARB");
      extfuncs.GetStringi = (PFNGLGETSTRINGIPROC)
         wglGetProcAddress("glGetStringi");
      extfuncs.GetConvolutionParameteriv = (GETCONVOLUTIONPARAMETERIVPROC)
         wglGetProcAddress("glGetConvolutionParameteriv");

      version = (glVersion[0] - '0') * 10 + (glVersion[2] - '0');

      printf("OpenGL extensions:\n");
      print_extension_list(glExtensions, singleLine);
      if (limits) {
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
print_visual_attribs_verbose(int iPixelFormat, LPPIXELFORMATDESCRIPTOR ppfd)
{
   printf("Visual ID: %x  generic=%d  native=%d\n",
          iPixelFormat, 
          ppfd->dwFlags & PFD_GENERIC_FORMAT ? 1 : 0,
          ppfd->dwFlags & PFD_DRAW_TO_WINDOW ? 1 : 0);
   printf("    bufferSize=%d level=%d renderType=%s doubleBuffer=%d stereo=%d\n",
          0 /* ppfd->bufferSize */, 0 /* ppfd->level */,
	  visual_render_type_name(ppfd->iPixelType),
          ppfd->dwFlags & PFD_DOUBLEBUFFER ? 1 : 0, 
          ppfd->dwFlags & PFD_STEREO ? 1 : 0);
   printf("    rgba: cRedBits=%d cGreenBits=%d cBlueBits=%d cAlphaBits=%d\n",
          ppfd->cRedBits, ppfd->cGreenBits,
          ppfd->cBlueBits, ppfd->cAlphaBits);
   printf("    cAuxBuffers=%d cDepthBits=%d cStencilBits=%d\n",
          ppfd->cAuxBuffers, ppfd->cDepthBits, ppfd->cStencilBits);
   printf("    accum: cRedBits=%d cGreenBits=%d cBlueBits=%d cAlphaBits=%d\n",
          ppfd->cAccumRedBits, ppfd->cAccumGreenBits,
          ppfd->cAccumBlueBits, ppfd->cAccumAlphaBits);
   printf("    multiSample=%d  multiSampleBuffers=%d\n",
          0 /* ppfd->numSamples */, 0 /* ppfd->numMultisample */);
}


static void
print_visual_attribs_short_header(void)
{
 printf("   visual   x  bf lv rg d st colorbuffer ax dp st accumbuffer  ms  cav\n");
 printf(" id gen win sp sz l  ci b ro  r  g  b  a bf th cl  r  g  b  a ns b eat\n");
 printf("-----------------------------------------------------------------------\n");
}


static void
print_visual_attribs_short(int iPixelFormat, LPPIXELFORMATDESCRIPTOR ppfd)
{
   char *caveat = "None";

   printf("0x%02x %2d  %2d %2d %2d %2d %c%c %c  %c %2d %2d %2d %2d %2d %2d %2d",
          iPixelFormat,
          ppfd->dwFlags & PFD_GENERIC_FORMAT ? 1 : 0,
          ppfd->dwFlags & PFD_DRAW_TO_WINDOW ? 1 : 0,
          0,
          0 /* ppfd->bufferSize */,
          0 /* ppfd->level */,
          ppfd->iPixelType == PFD_TYPE_RGBA ? 'r' : ' ',
          ppfd->iPixelType == PFD_TYPE_COLORINDEX ? 'c' : ' ',
          ppfd->dwFlags & PFD_DOUBLEBUFFER ? 'y' : '.',
          ppfd->dwFlags & PFD_STEREO ? 'y' : '.',
          ppfd->cRedBits, ppfd->cGreenBits,
          ppfd->cBlueBits, ppfd->cAlphaBits,
          ppfd->cAuxBuffers,
          ppfd->cDepthBits,
          ppfd->cStencilBits
          );

   printf(" %2d %2d %2d %2d %2d %1d %s\n",
          ppfd->cAccumRedBits, ppfd->cAccumGreenBits,
          ppfd->cAccumBlueBits, ppfd->cAccumAlphaBits,
          0 /* ppfd->numSamples */, 0 /* ppfd->numMultisample */,
          caveat
          );
}


static void
print_visual_attribs_long_header(void)
{
 printf("Vis  Vis   Visual Trans  buff lev render DB ste  r   g   b   a  aux dep ste  accum buffers  MS   MS\n");
 printf(" ID Depth   Type  parent size el   type     reo sz  sz  sz  sz  buf th  ncl  r   g   b   a  num bufs\n");
 printf("----------------------------------------------------------------------------------------------------\n");
}


static void
print_visual_attribs_long(int iPixelFormat, LPPIXELFORMATDESCRIPTOR ppfd)
{
   printf("0x%2x %2d %11d %2d     %2d %2d  %4s %3d %3d %3d %3d %3d %3d",
          iPixelFormat,
          ppfd->dwFlags & PFD_GENERIC_FORMAT ? 1 : 0,
          ppfd->dwFlags & PFD_DRAW_TO_WINDOW ? 1 : 0,
          0,
          0 /* ppfd->bufferSize */,
          0 /* ppfd->level */,
          visual_render_type_name(ppfd->iPixelType),
          ppfd->dwFlags & PFD_DOUBLEBUFFER ? 1 : 0,
          ppfd->dwFlags & PFD_STEREO ? 1 : 0,
          ppfd->cRedBits, ppfd->cGreenBits,
          ppfd->cBlueBits, ppfd->cAlphaBits
          );

   printf(" %3d %4d %2d %3d %3d %3d %3d  %2d  %2d\n",
          ppfd->cAuxBuffers,
          ppfd->cDepthBits,
          ppfd->cStencilBits,
          ppfd->cAccumRedBits, ppfd->cAccumGreenBits,
          ppfd->cAccumBlueBits, ppfd->cAccumAlphaBits,
          0 /* ppfd->numSamples */, 0 /* ppfd->numMultisample */
          );
}


static void
print_visual_info(HDC hdc, InfoMode mode)
{
   PIXELFORMATDESCRIPTOR pfd;
   int numVisuals, numWglVisuals;
   int i;

   numVisuals = DescribePixelFormat(hdc, 1, sizeof(PIXELFORMATDESCRIPTOR), NULL);
   if (numVisuals == 0)
      return;

   numWglVisuals = 0;
   for (i = 0; i < numVisuals; i++) {
      if(!DescribePixelFormat(hdc, i, sizeof(PIXELFORMATDESCRIPTOR), &pfd))
	 continue;

      //if(!(pfd.dwFlags & PFD_SUPPORT_OPENGL))
      //   continue;

      ++numWglVisuals;
   }

   printf("%d WGL Visuals\n", numWglVisuals);

   if (mode == Normal)
      print_visual_attribs_short_header();
   else if (mode == Wide)
      print_visual_attribs_long_header();

   for (i = 0; i < numVisuals; i++) {
      if(!DescribePixelFormat(hdc, i, sizeof(PIXELFORMATDESCRIPTOR), &pfd))
	 continue;

      //if(!(pfd.dwFlags & PFD_SUPPORT_OPENGL))
      //   continue;

      if (mode == Verbose)
	 print_visual_attribs_verbose(i, &pfd);
      else if (mode == Normal)
         print_visual_attribs_short(i, &pfd);
      else if (mode == Wide) 
         print_visual_attribs_long(i, &pfd);
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


static void
usage(void)
{
   printf("Usage: glxinfo [-v] [-t] [-h] [-i] [-b] [-display <dname>]\n");
   printf("\t-v: Print visuals info in verbose form.\n");
   printf("\t-t: Print verbose table.\n");
   printf("\t-h: This information.\n");
   printf("\t-b: Find the 'best' visual and print its number.\n");
   printf("\t-l: Print interesting OpenGL limits.\n");
   printf("\t-s: Print a single extension per line.\n");
}


int
main(int argc, char *argv[])
{
   HDC hdc;
   InfoMode mode = Normal;
   GLboolean findBest = GL_FALSE;
   GLboolean limits = GL_FALSE;
   GLboolean singleLine = GL_FALSE;
   int i;

   for (i = 1; i < argc; i++) {
      if (strcmp(argv[i], "-t") == 0) {
         mode = Wide;
      }
      else if (strcmp(argv[i], "-v") == 0) {
         mode = Verbose;
      }
      else if (strcmp(argv[i], "-b") == 0) {
         findBest = GL_TRUE;
      }
      else if (strcmp(argv[i], "-l") == 0) {
         limits = GL_TRUE;
      }
      else if (strcmp(argv[i], "-h") == 0) {
         usage();
         return 0;
      }
      else if(strcmp(argv[i], "-s") == 0) {
         singleLine = GL_TRUE;
      }
      else {
         printf("Unknown option `%s'\n", argv[i]);
         usage();
         return 0;
      }
   }

   hdc = CreateDC(TEXT("DISPLAY"), NULL, NULL, NULL);

   if (findBest) {
      int b;
      b = find_best_visual(hdc);
      printf("%d\n", b);
   }
   else {
      print_screen_info(hdc, limits, singleLine);
      printf("\n");
      print_visual_info(hdc, mode);
   }

   return 0;
}

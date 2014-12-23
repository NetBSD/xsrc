/*
 * Mesa 3-D graphics library
 * Version:  7.9
 *
 * Copyright (C) 2010 LunarG Inc.
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
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Authors:
 *    Chia-I Wu <olv@lunarg.com>
 */

/*
 * This demo tests eglGetProcAddress and glClear.
 */

#include <stdlib.h>
#include <stdio.h>
#include <EGL/egl.h>
#include <GLES/gl.h>

#include "eglut.h"

typedef void (GL_APIENTRY *type_ClearColor)(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
typedef void (GL_APIENTRY *type_Clear)(GLbitfield mask);

static type_ClearColor fn_ClearColor;
static type_Clear fn_Clear;

static void
draw(void)
{
   fn_Clear(GL_COLOR_BUFFER_BIT);
}

static void *
get_proc(const char *name)
{
   void *proc;

   proc = (void *) eglGetProcAddress(name);
   if (!proc) {
      /*
       * note that eglGetProcAddress is not required to support non-extension
       * functions
       */
      printf("failed to find %s (not necessarily a bug)\n", name);
      exit(1);
   }

   return proc;
}

static void
init(void)
{
   fn_ClearColor = (type_ClearColor) get_proc("glClearColor");
   fn_Clear = (type_Clear) get_proc("glClear");

   fn_ClearColor(1.0, 0.4, 0.4, 0.0);
}

int
main(int argc, char *argv[])
{
   eglutInitWindowSize(300, 300);
   eglutInitAPIMask(EGLUT_OPENGL_ES1_BIT);
   eglutInit(argc, argv);

   eglutCreateWindow("clear");

   eglutDisplayFunc(draw);

   init();

   eglutMainLoop();

   return 0;
}

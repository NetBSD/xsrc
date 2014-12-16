/*
 * Copyright (C) 2011 LunarG Inc.
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
 * This is a demonstration of EGL on fbdev:
 *
 *   The native display is the fd of the device;
 *   There is only one native window, NULL;
 *   There is no native pixmaps.
 *
 * It is the app's responsibility to set up the tty, open the fb device, and
 * initialize EGL.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

/* for tty */
#include <linux/kd.h>
#include <linux/vt.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>
#include <signal.h>

/* for fbdev */
#include <linux/fb.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

/* for EGL */
#include <EGL/egl.h>
#include <GLES/gl.h>

static int tty_fd = -1;
static int tty_saved_vtno;

static int tty_open_vt(int vtno)
{
   const char tty[] = "/dev/tty%d";
   char name[64];
   int size, flags;

   size = snprintf(name, sizeof(name), tty, vtno);
   if (size >= sizeof(name))
      return -1;

   flags = (vtno) ? O_RDWR | O_NDELAY : O_WRONLY;

   return open(name, flags);
}

static int tty_switch_vt(int fd, int vtno)
{
   int ret;

   ret = ioctl(fd, VT_ACTIVATE, vtno);
   if (ret >= 0)
      ret = ioctl(fd, VT_WAITACTIVE, vtno);

   return ret;
}


static int tty_init_vt(void)
{
   struct vt_stat vts;
   int fd, vtno;

   /* get the next available tty number */
   fd = tty_open_vt(0);
   if (fd < 0)
      return -1;
   if (ioctl(fd, VT_OPENQRY, &vtno) < 0)
      goto fail;
   close(fd);

   fd = tty_open_vt(vtno);
   if (fd < 0)
      return -1;

   /* save the current VT */
   if (ioctl(fd, VT_GETSTATE, &vts) < 0)
      goto fail;
   tty_saved_vtno = vts.v_active;

   if (tty_switch_vt(fd, vtno))
      goto fail;

   return fd;

fail:
   close(fd);
   return -1;
}

static void
tty_close(void)
{
   /* restore */
   ioctl(tty_fd, KDSETMODE, KD_TEXT);
   tty_switch_vt(tty_fd, tty_saved_vtno);

   close(tty_fd);
}

static void
signal_handler(int sig)
{
   if (tty_fd >= 0)
      tty_close();
}

static int tty_open(void)
{
   struct sigaction sa;

   tty_fd = tty_init_vt();
   if (tty_fd < 0)
      return -1;

   /* install the signal handler */
   memset(&sa, 0, sizeof(sa));
   sigemptyset(&sa.sa_mask);
   sa.sa_handler = signal_handler;
   if (sigaction(SIGINT, &sa, NULL))
      goto fail;
   if (sigaction(SIGTERM, &sa, NULL))
      goto fail;
   if (sigaction(SIGABRT, &sa, NULL))
      goto fail;

   if (ioctl(tty_fd, KDSETMODE, KD_GRAPHICS) < 0)
      goto fail;

   tcflush(tty_fd, TCIOFLUSH);

   return 0;

fail:
   tty_close();
   tty_fd = -1;
   return -1;
}

static EGLDisplay egl_dpy;
static EGLContext egl_ctx;
static EGLSurface egl_surf;
static EGLBoolean egl_verbose;

static void
egl_fatal(char *format, ...)
{
   va_list args;

   va_start(args, format);
   vfprintf(stderr, format, args);
   va_end(args);
   putc('\n', stderr);

   abort();
}

static void
egl_init_for_fbdev(int fd, EGLBoolean verbose)
{
   const EGLNativeWindowType native_win = (EGLNativeWindowType) NULL;
   EGLint major, minor, num_configs;
   EGLConfig conf;

   egl_verbose = verbose;

   /* make Mesa/EGL happy */
   setenv("EGL_PLATFORM", "fbdev", 0);

   egl_dpy = eglGetDisplay((EGLNativeDisplayType) fd);
   if (egl_dpy == EGL_NO_DISPLAY)
      egl_fatal("failed to get a display");
   if (!eglInitialize(egl_dpy, &major, &minor))
      egl_fatal("failed to initialize EGL");

   if (egl_verbose) {
      printf("EGL %d.%d\n", major, minor);
      printf("EGL_VENDOR: %s\n", eglQueryString(egl_dpy, EGL_VENDOR));
      printf("EGL_VERSION: %s\n", eglQueryString(egl_dpy, EGL_VERSION));
      printf("EGL_EXTENSIONS: %s\n", eglQueryString(egl_dpy, EGL_EXTENSIONS));
      printf("EGL_CLIENT_APIS: %s\n",
            eglQueryString(egl_dpy, EGL_CLIENT_APIS));
   }

   if (!eglChooseConfig(egl_dpy, NULL, &conf, 1, &num_configs) ||
       !num_configs)
      egl_fatal("failed to choose a config");

   egl_ctx = eglCreateContext(egl_dpy, conf, EGL_NO_CONTEXT, NULL);
   if (egl_ctx == EGL_NO_CONTEXT)
      egl_fatal("failed to create a context");

   egl_surf = eglCreateWindowSurface(egl_dpy, conf, native_win, NULL);
   if (egl_surf == EGL_NO_SURFACE)
      egl_fatal("failed to create a surface");

   if (!eglMakeCurrent(egl_dpy, egl_surf, egl_surf, egl_ctx))
      egl_fatal("failed to make context/surface current");
}

static void
egl_present(void)
{
   if (!eglSwapBuffers(egl_dpy, egl_surf))
      egl_fatal("failed to swap buffers");
}

static void
egl_destroy(void)
{
   eglMakeCurrent(egl_dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
   eglDestroyContext(egl_dpy, egl_surf);
   eglDestroyContext(egl_dpy, egl_ctx);
   eglTerminate(egl_dpy);

   egl_surf = EGL_NO_SURFACE;
   egl_ctx = EGL_NO_CONTEXT;
   egl_dpy = EGL_NO_DISPLAY;
}

/* stolen from tri.c */
static void
draw(int frame)
{
   static const GLfloat verts[3][2] = {
      { -1, -1 },
      {  1, -1 },
      {  0,  1 }
   };
   static const GLfloat colors[3][4] = {
      { 1, 0, 0, 1 },
      { 0, 1, 0, 1 },
      { 0, 0, 1, 1 }
   };
   GLfloat view_rotz = (GLfloat) frame;

   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   glPushMatrix();
   glRotatef(view_rotz, 0, 0, 1);

   glVertexPointer(2, GL_FLOAT, 0, verts);
   glColorPointer(4, GL_FLOAT, 0, colors);

   glEnableClientState(GL_VERTEX_ARRAY);
   glEnableClientState(GL_COLOR_ARRAY);

   /* draw triangle */
   glDrawArrays(GL_TRIANGLES, 0, 3);

   glDisableClientState(GL_VERTEX_ARRAY);
   glDisableClientState(GL_COLOR_ARRAY);

   glPopMatrix();
}

static void
init(int width, int height)
{
   GLfloat ar = (GLfloat) width / height;

   glViewport(0, 0, width, height);

   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   glFrustumf(-ar, ar, -1, 1, 5.0, 60.0);

   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();
   glTranslatef(0.0, 0.0, -10.0);
}

int
main(int argc, char **argv)
{
   const char fbdev[] = "/dev/fb0";
   struct fb_var_screeninfo vinfo;
   int fd, tty_err, frame;

   fd = open(fbdev, O_RDWR);
   if (fd < 0)
      egl_fatal("failed to open %s", fbdev);

   memset(&vinfo, 0, sizeof(vinfo));
   if (ioctl(fd, FBIOGET_VSCREENINFO, &vinfo))
      egl_fatal("failed to get fb info");

   /* initialize EGL */
   egl_init_for_fbdev(fd, EGL_TRUE);

   /* try to open a new tty */
   tty_err = tty_open();

   init(vinfo.xres, vinfo.yres);
   for (frame = 0; frame <= 180; frame++) {
      draw(frame);
      egl_present();
   }

   if (!tty_err)
      tty_close();

   egl_destroy();
   close(fd);

   return 0;
}

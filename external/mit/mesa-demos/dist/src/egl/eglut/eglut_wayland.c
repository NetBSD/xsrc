#include <wayland-client.h>
#include <wayland-egl.h>

#include <poll.h>
#include <errno.h>
#include <string.h>

#include "eglutint.h"

struct display {
   struct wl_display *display;
   struct wl_compositor *compositor;
   struct wl_shell *shell;
   uint32_t mask;
};

struct window {
   struct wl_surface *surface;
   struct wl_shell_surface *shell_surface;
   struct wl_callback *callback;
};

static struct display display = {0, };
static struct window window = {0, };

static void
registry_handle_global(void *data, struct wl_registry *registry, uint32_t id,
                       const char *interface, uint32_t version)
{
   struct display *d = data;

   if (strcmp(interface, "wl_compositor") == 0) {
      d->compositor =
         wl_registry_bind(registry, id, &wl_compositor_interface, 1);
   } else if (strcmp(interface, "wl_shell") == 0) {
      d->shell = wl_registry_bind(registry, id, &wl_shell_interface, 1);
   }
}

static void
registry_handle_global_remove(void *data, struct wl_registry *registry,
                              uint32_t name)
{
}

static const struct wl_registry_listener registry_listener = {
   registry_handle_global,
   registry_handle_global_remove
};

static void
sync_callback(void *data, struct wl_callback *callback, uint32_t serial)
{
   int *done = data;

   *done = 1;
   wl_callback_destroy(callback);
}

static const struct wl_callback_listener sync_listener = {
   sync_callback
};

static int
wayland_roundtrip(struct wl_display *display)
{
   struct wl_callback *callback;
   int done = 0, ret = 0;

   callback = wl_display_sync(display);
   wl_callback_add_listener(callback, &sync_listener, &done);
   while (ret != -1 && !done)
      ret = wl_display_dispatch(display);

   if (!done)
      wl_callback_destroy(callback);

   return ret;
}

void
_eglutNativeInitDisplay(void)
{
   struct wl_registry *registry;

   _eglut->native_dpy =  display.display = wl_display_connect(NULL);

   if (!_eglut->native_dpy)
      _eglutFatal("failed to initialize native display");

   registry = wl_display_get_registry(_eglut->native_dpy);
   wl_registry_add_listener(registry, &registry_listener, &display);
   wayland_roundtrip(_eglut->native_dpy);
   wl_registry_destroy(registry);

   _eglut->surface_type = EGL_WINDOW_BIT;
   _eglut->redisplay = 1;
}

void
_eglutNativeFiniDisplay(void)
{
   wl_display_flush(_eglut->native_dpy);
   wl_display_disconnect(_eglut->native_dpy);
}

void
_eglutNativeInitWindow(struct eglut_window *win, const char *title,
                       int x, int y, int w, int h)
{
   struct wl_egl_window *native;
   struct wl_region *region;

   window.surface = wl_compositor_create_surface(display.compositor);

   region = wl_compositor_create_region(display.compositor);
   wl_region_add(region, 0, 0, w, h);
   wl_surface_set_opaque_region(window.surface, region);
   wl_region_destroy(region);

   window.shell_surface = wl_shell_get_shell_surface(display.shell,
         window.surface);
   native = wl_egl_window_create(window.surface, w, h);

   wl_shell_surface_set_toplevel(window.shell_surface);

   win->native.u.window = native;
   win->native.width = w;
   win->native.height = h;
}

void
_eglutNativeFiniWindow(struct eglut_window *win)
{
   wl_egl_window_destroy(win->native.u.window);

   wl_shell_surface_destroy(window.shell_surface);
   wl_surface_destroy(window.surface);

   if (window.callback)
      wl_callback_destroy(window.callback);
}

static void
draw(void *data, struct wl_callback *callback, uint32_t time);

static const struct wl_callback_listener frame_listener = {
   draw
};

static void
draw(void *data, struct wl_callback *callback, uint32_t time)
{
   struct window *window = (struct window *)data;
   struct eglut_window *win = _eglut->current;

   if (callback) {
      wl_callback_destroy(callback);
      window->callback = NULL;
   }

   /* Our client doesn't want to push another frame; go back to sleep. */
   if (!_eglut->redisplay)
      return;
   _eglut->redisplay = 0;

   if (win->display_cb)
      win->display_cb();

   window->callback = wl_surface_frame(window->surface);
   wl_callback_add_listener(window->callback, &frame_listener, window);

   eglSwapBuffers(_eglut->dpy, win->surface);
}

void
_eglutNativeEventLoop(void)
{
   struct pollfd pollfd;
   int ret;

   pollfd.fd = wl_display_get_fd(display.display);
   pollfd.events = POLLIN;
   pollfd.revents = 0;

   while (1) {
      /* If we need to flush but can't, don't do anything at all which could
       * push further events into the socket. */
      if (!(pollfd.events & POLLOUT)) {
         wl_display_dispatch_pending(display.display);

         if (_eglut->idle_cb)
            _eglut->idle_cb();

         /* Client wants to redraw, but we have no frame event to trigger the
          * redraw; kickstart it by redrawing immediately. */
         if (_eglut->redisplay && !window.callback)
            draw(&window, NULL, 0);
      }

      ret = wl_display_flush(display.display);
      if (ret < 0 && errno != EAGAIN)
         break; /* fatal error; socket is broken */
      else if (ret < 0 && errno == EAGAIN)
         pollfd.events |= POLLOUT; /* need to wait until we can flush */
      else
         pollfd.events &= ~POLLOUT; /* successfully flushed */

      if (poll(&pollfd, 1, -1) == -1)
         break;

      if (pollfd.revents & (POLLERR | POLLHUP))
         break;

      if (pollfd.events & POLLOUT) {
	 if (!(pollfd.revents & POLLOUT))
            continue; /* block until we can flush */
         pollfd.events &= ~POLLOUT;
      }

      if (pollfd.revents & POLLIN) {
         ret = wl_display_dispatch(display.display);
         if (ret == -1)
            break;
      }

      ret = wl_display_flush(display.display);
      if (ret < 0 && errno != EAGAIN)
         break; /* fatal error; socket is broken */
      else if (ret < 0 && errno == EAGAIN)
         pollfd.events |= POLLOUT; /* need to wait until we can flush */
      else
         pollfd.events &= ~POLLOUT; /* successfully flushed */
   }
}

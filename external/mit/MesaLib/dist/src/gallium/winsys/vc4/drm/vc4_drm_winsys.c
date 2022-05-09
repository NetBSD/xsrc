/*
 * Copyright © 2014 Broadcom
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
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include "util/os_file.h"

#include "renderonly/renderonly.h"
#include "kmsro/drm/kmsro_drm_public.h"
#include "vc4_drm_public.h"
#include "vc4/vc4_screen.h"
#include "drm-uapi/vc4_drm.h"

struct pipe_screen *
vc4_drm_screen_create(int fd, const struct pipe_screen_config *config)
{
   bool v3d_present = true;

#ifndef USE_VC4_SIMULATOR
   struct drm_vc4_get_param ident0 = {
      .param = DRM_VC4_PARAM_V3D_IDENT0,
   };

   int ret = ioctl(fd, DRM_IOCTL_VC4_GET_PARAM, &ident0);
   v3d_present = ret == 0;
#endif

   if (v3d_present)
      return vc4_screen_create(os_dupfd_cloexec(fd), NULL);

#ifdef GALLIUM_KMSRO
   return kmsro_drm_screen_create(fd, config);
#endif

   return NULL;
}

struct pipe_screen *
vc4_drm_screen_create_renderonly(struct renderonly *ro,
                                 const struct pipe_screen_config *config)
{
   return vc4_screen_create(fcntl(ro->gpu_fd, F_DUPFD_CLOEXEC, 3), ro);
}

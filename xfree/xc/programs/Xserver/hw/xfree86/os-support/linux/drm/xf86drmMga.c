/* $XFree86: xc/programs/Xserver/hw/xfree86/os-support/linux/drm/xf86drmMga.c,v 1.3 2000/08/24 22:20:17 tsi Exp $ */

#ifdef XFree86Server
# include "xf86.h"
# include "xf86_OSproc.h"
# include "xf86_ansic.h"
# include "xf86Priv.h"
# define _DRM_MALLOC xalloc
# define _DRM_FREE   xfree
# ifndef XFree86LOADER
#  include <sys/stat.h>
#  include <sys/mman.h>
# endif
#else
# include <stdio.h>
# include <stdlib.h>
# include <unistd.h>
# include <string.h>
# include <ctype.h>
# include <fcntl.h>
# include <errno.h>
# include <signal.h>
# include <sys/types.h>
# include <sys/stat.h>
# include <sys/ioctl.h>
# include <sys/mman.h>
# include <sys/time.h>
# ifdef DRM_USE_MALLOC
#  define _DRM_MALLOC malloc
#  define _DRM_FREE   free
extern int xf86InstallSIGIOHandler(int fd, void (*f)(int, void *), void *);
extern int xf86RemoveSIGIOHandler(int fd);
# else
#  include <Xlibint.h>
#  define _DRM_MALLOC Xmalloc
#  define _DRM_FREE   Xfree
# endif
#endif

/* Not all systems have MAP_FAILED defined */
#ifndef MAP_FAILED
#define MAP_FAILED ((void *)-1)
#endif

#ifdef __linux__
#include <sys/sysmacros.h>	/* for makedev() */
#endif
#include "xf86drm.h"
#include "xf86drmMga.h"
#include "drm.h"

Bool drmMgaCleanupDma(int driSubFD)
{
   drm_mga_init_t init;
   memset(&init, 0, sizeof(drm_mga_init_t));
   init.func = MGA_CLEANUP_DMA;
   if(ioctl(driSubFD, DRM_IOCTL_MGA_INIT, &init)) {
      return FALSE;
   }
   
   return TRUE;
}

Bool drmMgaLockUpdate(int driSubFD, drmLockFlags flags)
{
   drm_lock_t lock;
   
   memset(&lock, 0, sizeof(drm_lock_t));
   
   if (flags & DRM_LOCK_QUIESCENT)  lock.flags |= _DRM_LOCK_QUIESCENT;
   if (flags & DRM_LOCK_FLUSH)      lock.flags |= _DRM_LOCK_FLUSH;
   if (flags & DRM_LOCK_FLUSH_ALL)  lock.flags |= _DRM_LOCK_FLUSH_ALL;
   
   if(ioctl(driSubFD, DRM_IOCTL_MGA_FLUSH, &lock)) {
      return FALSE;
   }
   
   return TRUE;
}

Bool drmMgaInitDma(int driSubFD, drmMgaInit *info)
{
      drm_mga_init_t init;
      int i;
   
      memset(&init, 0, sizeof(drm_mga_init_t));
      init.func = MGA_INIT_DMA;
      init.reserved_map_agpstart = info->reserved_map_agpstart;
      init.reserved_map_idx = info->reserved_map_idx;
      init.buffer_map_idx = info->buffer_map_idx;
      init.sarea_priv_offset = info->sarea_priv_offset;
      init.primary_size = info->primary_size;
      init.warp_ucode_size = info->warp_ucode_size;
      init.frontOffset = info->frontOffset;
      init.backOffset = info->backOffset;
      init.depthOffset = info->depthOffset;
      init.textureOffset = info->textureOffset;
      init.textureSize = info->textureSize;
      init.agpTextureSize = info->agpTextureSize;
      init.agpTextureOffset = info->agpTextureOffset;
      init.cpp = info->cpp;
      init.stride = info->stride;
      init.sgram = info->sgram;
      init.chipset = info->chipset;
   
      for(i = 0; i < MGA_MAX_WARP_PIPES; i++) {
	       init.WarpIndex[i].installed = info->WarpIndex[i].installed;
	       init.WarpIndex[i].phys_addr = info->WarpIndex[i].phys_addr;
	       init.WarpIndex[i].size = info->WarpIndex[i].size;
      }
   
      init.mAccess = info->mAccess;
   
   
   
      if(ioctl(driSubFD, DRM_IOCTL_MGA_INIT, &init)) {
	       return FALSE;
      }
      return TRUE;
}


/* $XFree86: xc/programs/Xserver/hw/xfree86/os-support/linux/drm/xf86drmSiS.c,v 1.10 2001/12/15 00:59:12 dawes Exp $ */

#ifdef XFree86Server
# include "xf86.h"
# include "xf86_OSproc.h"
# include "xf86_ansic.h"
# define _DRM_MALLOC xalloc
# define _DRM_FREE   xfree
# ifndef XFree86LOADER
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
# include <sys/ioctl.h>
# include <sys/mman.h>
# include <sys/time.h>
# ifdef DRM_USE_MALLOC
#  define _DRM_MALLOC malloc
#  define _DRM_FREE   free
extern int xf86InstallSIGIOHandler(int fd, void (*f)(int, void *), void *);
extern int xf86RemoveSIGIOHandler(int fd);
# else
#  include <X11/Xlibint.h>
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
#include "xf86drmSiS.h"
#define CONFIG_DRM_SIS
#include "drm.h"
#undef CONFIG_DRM_SIS

Bool drmSiSAgpInit(int driSubFD, int offset, int size)
{
   drm_sis_agp_t agp;
      
   agp.offset = offset;
   agp.size = size;
   ioctl(driSubFD, SIS_IOCTL_AGP_INIT, &agp);

   return TRUE;
}

#include "xf86.h"
#include "xf86_OSproc.h"
#include "xf86Resources.h"
#include "xf86_ansic.h"
#include "compiler.h"
#include "xf86PciInfo.h"
#include "xf86Pci.h"
#include "xf86Priv.h"
#include "i810.h"

#ifndef XFree86LOADER
#include <sys/stat.h>
#include <sys/mman.h>
#endif

#include "xf86drm.h"
#include "drm.h"
#include "i810_drm_public.h"

Bool I810drmCleanupDma(ScrnInfoPtr pScrn)
{
   I810Ptr pI810 = I810PTR(pScrn);
   drm_i810_init_t init;
   
   memset(&init, 0, sizeof(drm_i810_init_t));
   init.func = I810_CLEANUP_DMA;
   
   if(ioctl(pI810->drmSubFD, DRM_IOCTL_I810_INIT, &init)) {
      ErrorF("I810 Dma Cleanup Failed\n");
      return FALSE;
   }
   
   return TRUE;
}

Bool I810drmInitDma(ScrnInfoPtr pScrn)
{
   I810Ptr pI810 = I810PTR(pScrn);
   I810RingBuffer *ring = &(pI810->LpRing);

   drm_i810_init_t init;
   
   memset(&init, 0, sizeof(drm_i810_init_t));
   init.func = I810_INIT_DMA;
   init.ring_map_idx = 6;
   init.buffer_map_idx = 5;
   init.ring_start = ring->mem.Start;
   init.ring_end = ring->mem.End;
   init.ring_size = ring->mem.Size;
   init.sarea_priv_offset = sizeof(XF86DRISAREARec);
   ErrorF("I810 Dma Initialization start\n");

   if(ioctl(pI810->drmSubFD, DRM_IOCTL_I810_INIT, &init)) {
      ErrorF("I810 Dma Initialization Failed\n");
      return FALSE;
   }
   ErrorF("I810 Dma Initialization done\n");
   return TRUE;
}

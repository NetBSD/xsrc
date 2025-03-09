#ifndef __NV_INCLUDE_H__
#define __NV_INCLUDE_H__

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string.h>
#include <math.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>

/* All drivers should typically include these */
#include "xf86.h"
#include "xf86_OSproc.h"
#define PPC_MMIO_IS_BE
#include "compiler.h"

/* Drivers that need to access the PCI config space directly need this */
#include "xf86Pci.h"

#ifdef XSERVER_PLATFORM_BUS
#include "xf86platformBus.h"
#endif

/* All drivers initialising the SW cursor need this */
#include "mipointer.h"

#include "micmap.h"

#include "xf86DDC.h"

#include "xf86RandR12.h"

#include "nv_const.h"

#include "dixstruct.h"
#include "scrnintstr.h"

#include "fb.h"

#include "xf86cmap.h"
#include "shadowfb.h"

#include "xf86xv.h"
#include <X11/extensions/Xv.h>

#include "xf86Cursor.h"
#include "xf86DDC.h"

#include "region.h"

#include <X11/extensions/randr.h>

#ifdef HAVE_XEXTPROTO_71
#include <X11/extensions/dpmsconst.h>
#else
#define DPMS_SERVER
#include <X11/extensions/dpms.h>
#endif

#define NV_DMA_DEBUG 0

#include "compat-api.h"

#include "nouveau_local.h"

#include "nv_type.h"
#include "nv_proto.h"
#include "nv_dma.h"

#endif /* __NV_INCLUDE_H__ */

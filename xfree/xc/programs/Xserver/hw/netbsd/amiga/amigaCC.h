/* Includes for AmigaCC driver */


#include <sys/device.h>
#include <sys/queue.h>
#include "dev/grfabs_reg.h"
#include "dev/viewioctl.h"
#include "dev/grfvar.h"


#define MAX_COLORS 256


struct viewinfo {
    bmap_t bm;
    struct view_size vs;
    colormap_t colormap;
    long entry [MAX_COLORS];
};


/*
 * On m68k systems we do not use banked frame buffer. Therefore we don't
 * need the vga prototype stuff.
 */

#ifndef _VGA256_H
#define VGA256_H
    
#include "X.h"
#include "Xmd.h"
#include "servermd.h"
#include "gcstruct.h"
#include "window.h"
#include "pixmapstr.h"
#include "scrnintstr.h"
#include "windowstr.h"
#include "region.h"
#include "mistruct.h"
#include "mibstore.h"
#include "fontstruct.h"
#include "dixfontstr.h"
#include "cfb.h" 
#ifndef NO_CFBMSKBITS
#include "cfbmskbits.h"
#include "cfb8bit.h"
#endif
#include "gcstruct.h"

#ifdef CSRG_BASED
#define VGABASE 0xFF000000
#else
#define VGABASE 0xF0000000
#endif

#endif /* _VGA256_H */

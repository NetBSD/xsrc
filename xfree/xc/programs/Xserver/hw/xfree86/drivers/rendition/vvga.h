/* $XFree86: xc/programs/Xserver/hw/xfree86/drivers/rendition/vvga.h,v 1.5 2001/02/15 17:50:35 eich Exp $ */
/*
 * file vvga.h
 *
 * Headerfile for vvga.c
 */

#ifndef __VVGA_H__
#define __VVGA_H__



/*
 * includes
 */

#include "vtypes.h"



/*
 * function prototypes
 */

void verite_textmode(struct verite_board_t *board);
void verite_savetextmode(struct verite_board_t *board);
#ifdef VVGA_INTERNAL
static void verite_resetvga(void);
static void verite_loadvgafont(void);
static void verite_restoretextmode(struct verite_board_t *board);
static void verite_restorepalette(void);
#endif


#endif /* __VVGA_H__ */

/*
 * end of file vvga.h
 */

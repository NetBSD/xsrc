/* $XFree86: xc/programs/Xserver/hw/xfree86/drivers/rendition/vvga.h,v 1.4 2000/03/31 20:13:29 dawes Exp $ */
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

void verite_resetvga(void);
void verite_loadvgafont(void);
void verite_textmode(struct verite_board_t *board);
void verite_savetextmode(struct verite_board_t *board);
void verite_restoretextmode(struct verite_board_t *board);
void verite_restorepalette(void);



#endif /* __VVGA_H__ */

/*
 * end of file vvga.h
 */

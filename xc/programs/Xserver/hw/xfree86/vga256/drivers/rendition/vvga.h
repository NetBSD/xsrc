/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/rendition/vvga.h,v 1.1.2.2 1998/08/07 06:40:28 hohndel Exp $ */
/*
 * file vvga.h
 *
 * Headerfile for vvga.c
 */

#ifndef _VVGA_H_
#define _VVGA_H_



/*
 * includes
 */

#include "vtypes.h"



/*
 * function prototypes
 */

void v_resetvga(void);
void v_loadvgafont(void);
void v_textmode(struct v_board_t *board);
void v_savetextmode(struct v_board_t *board);
void v_restoretextmode(struct v_board_t *board);
void v_restorepalette(void);



#endif /* #ifndef _VVGA_H_ */

/*
 * end of file vvga.h
 */

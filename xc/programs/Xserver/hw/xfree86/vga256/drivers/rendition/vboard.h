/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/rendition/vboard.h,v 1.1.2.2 1998/08/07 06:40:23 hohndel Exp $ */
/*
 * vboard.h
 *
 * functions to interact with a Verite board
 */

#ifndef _VBOARD_H_
#define _VBOARD_H_


/*
 * includes
 */

#include "vtypes.h"



/*
 * function prototypes
 */

int v_initboard(struct v_board_t *board);
int v_resetboard(struct v_board_t *board);
int v_getmemorysize(struct v_board_t *board);



#endif /* #define _VBOARD_H_ */

/*
 * end of file vboard.h
 */

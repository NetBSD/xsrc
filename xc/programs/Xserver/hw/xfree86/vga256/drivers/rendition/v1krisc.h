/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/rendition/v1krisc.h,v 1.1.2.2 1998/08/07 06:40:21 hohndel Exp $ */
/*
 * file v1krisc.h
 *
 * low level function to communicate with the on-board RISC
 */

#ifndef _V1KRISC_H_
#define _V1KRISC_H_



/*
 * includes
 */

#include "v1kregs.h"
#include "vtypes.h"



/*
 * function prototypes
 */

void v1k_start(struct v_board_t *board, vu32 pc);
void v1k_continue(struct v_board_t *board);
void v1k_stop(struct v_board_t *board);
void v1k_flushicache(struct v_board_t *board);
void v1k_softreset(struct v_board_t *board);



#endif /* #ifndef _V1KRISC_H_ */

/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/rendition/vmodes.h,v 1.1.2.2 1998/08/07 06:40:26 hohndel Exp $ */
/*
 * file vmodes.h
 *
 * headerfile for vmodes.c
 */

#ifndef _VMODES_H_
#define _VMODES_H_



/*
 * includes
 */

#include "vtypes.h"



/*
 * function prototypes
 */

int v_setmodefixed(struct v_board_t *board);
int v_setmode(struct v_board_t *board, struct v_modeinfo_t *mode);
void v_setframebase(struct v_board_t *board, vu32 framebase);



#endif /* #ifndef _VMODES_H_ */

/*
 * end of file vmodes.h
 */

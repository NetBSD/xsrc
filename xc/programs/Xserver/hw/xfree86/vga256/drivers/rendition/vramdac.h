/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/rendition/vramdac.h,v 1.1.2.2 1998/08/07 06:40:27 hohndel Exp $ */
/*
 * file vramdac.h
 *
 * headfile for vramdac.c
 */

#ifndef _VRAMDAC_H_
#define _VRAMDAC_H_



/*
 * includes
 */

#include "vtypes.h"



/*
 * defines
 */

#define V_NOCURSOR  0
#define V_2COLORS   1
#define V_3COLORS   2
#define V_XCURSOR   3

#define V_CURSOR32  0
#define V_CURSOR64  1



/*
 * function prototypes
 */

int v_initdac(struct v_board_t *board, vu8 bpp, vu8 doubleclock);
void v_enablecursor(struct v_board_t *board, int type, int size);
void v_movecursor(struct v_board_t *board, vu16 x, vu16 y, vu8 xo, vu8 yo);
void v_setcursorcolor(struct v_board_t *board, vu32 fg, vu32 bg);
void v_loadcursor(struct v_board_t *board, vu8 type, vu8 *cursorimage);
void v_setpalette(struct v_board_t *board, vu8 start, vu8 count, vu8 *table);



#endif /* #ifndef _VRAMDAC_H_ */

/*
 * end of file vramdac.h
 */

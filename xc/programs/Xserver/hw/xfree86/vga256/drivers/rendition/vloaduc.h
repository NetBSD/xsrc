/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/rendition/vloaduc.h,v 1.1.2.2 1998/08/07 06:40:25 hohndel Exp $ */
/*
 * file vloaduc.h
 *
 * loads microcode
 */

#ifndef _VLOADUC_H_
#define _VLOADUC_H_



/*
 * includes
 */

#include "vos.h"
#include "vtypes.h"



/*
 * defines 
 */

#define LITTLE_ENDIAN



/*
 * function prototypes
 */

int v_load_ucfile(struct v_board_t *board, char *file_name);



#endif /* #ifndef _VLOADUC_H_ */

/*
 * end of file vloaduc.h
 */

/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/rendition/vboard.c,v 1.1.2.4 1998/10/21 04:23:14 dawes Exp $ */
/*
 * includes
 */

#include "v1krisc.h"
#include "vboard.h"
#include "vloaduc.h"
#include "vos.h"



/* 
 * global data
 */

#include "cscode.h"



/*
 * local function prototypes
 */


#if NOT_YET_USED
/*
 * functions
 */
int v_initboard(struct v_board_t *board)
{
  vu16 iob=board->io_base;
  vu8 *vmb;
  vu32 offset;
  int c;

  /* write "monitor" program to memory */
  v1k_stop(board);
  board->csucode_base=0x800;
  v_out8(iob+MEMENDIAN, MEMENDIAN_NO);

  /* Note that CS ucode must wait on address in csucode_base
   * when initialized for later context switch code to work. */
  vmb=board->vmem_base;
  offset=board->csucode_base/4;
  for (c=0; c<sizeof(csrisc)/sizeof(vu32); c++, offset++)
    v_write_memory32(vmb, offset, csrisc[c]);

  /* ... and start it */
  v1k_flushicache(board);
  v1k_start(board, board->csucode_base);

  c=v_load_ucfile(board, "/home/smurf/RENDITION/rendition/v10002d.uc");
  if (c == -1) {
    ErrorF( "Jeuch\n");
  }
  else {
    board->ucode_entry=c;
    v_out32(iob, 0);     /* a0 - ucode init command */
    v_out32(iob, 0);     /* a1 - 1024 byte context store area */
    v_out32(iob, 0);     /* a2 */
    v_out32(iob, board->ucode_entry);
  }

  return 0;
}

#endif

int v_resetboard(struct v_board_t *board)
{
/*
  v1k_softreset(board);
*/
  return 0;
}



int v_getmemorysize(struct v_board_t *board)
{
#define PATTERN  0xf5faaf5f
#define START    0x12345678
#define ONEMEG   (1024L*1024L)
    vu32 offset;
    vu32 pattern;
    vu32 start;
    vu8 memendian;
#ifdef XSERVER
    vu8 modereg;

    modereg=v_in8(board->io_base+MODEREG);
    v_out8(board->io_base+MODEREG, NATIVE_MODE);
#endif

    /* no byte swapping */
    memendian=v_in8(board->io_base+MEMENDIAN);
    v_out8(board->io_base+MEMENDIAN, MEMENDIAN_NO);

    /* it looks like the v1000 wraps the memory; but for I'm not sure,
     * let's test also for non-writable offsets */
    start=v_read_memory32(board->vmem_base, 0);
    v_write_memory32(board->vmem_base, 0, START);
    for (offset=ONEMEG; offset<16*ONEMEG; offset+=ONEMEG) {
#ifdef DEBUG
        ErrorF( "Testing %d MB: ", offset/ONEMEG);
#endif
        pattern=v_read_memory32(board->vmem_base, offset/4);
        if (START == pattern) {
#ifdef DEBUG
            ErrorF( "Back at the beginning\n");
#endif
            break;    
        }
        
        pattern^=PATTERN;
        v_write_memory32(board->vmem_base, offset/4, pattern);
        
#ifdef DEBUG
        ErrorF( "%x <-> %x\n", (int)pattern, 
                    (int)v_read_memory32(board->vmem_base, offset/4));
#endif

        if (pattern != v_read_memory32(board->vmem_base, offset/4)) {
            offset-=ONEMEG;
            break;    
        }
        v_write_memory32(board->vmem_base, offset/4, pattern^PATTERN);
    }
    v_write_memory32(board->vmem_base, 0, start);

    if (16*ONEMEG <= offset)
        board->mem_size=4*ONEMEG;
    else 
	    board->mem_size=offset;

    /* restore default byte swapping */
    v_out8(board->io_base+MEMENDIAN, MEMENDIAN_NO);

#ifdef XSERVER
    v_out8(board->io_base+MODEREG, modereg);
#endif

    return board->mem_size;
#undef PATTERN
#undef ONEMEG
}



/*
 * local functions
 */



/*
 * end of file vboard.c
 */

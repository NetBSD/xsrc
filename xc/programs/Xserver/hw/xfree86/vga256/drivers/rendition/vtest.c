/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/rendition/vtest.c,v 1.1.2.2 1998/08/07 06:40:27 hohndel Exp $ */
#include <stdio.h>
#include <string.h>
#include <linux/pci.h>
#include <linux/version.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <sys/mman.h>

#include "vos.h"
#include "vvga.h"
#include "vtypes.h"
#include "vmodes.h"
#include "v1kregs.h"

struct v_board_t *board;


void unmapvideo(struct v_board_t *board) {
  v_unmapmemory(board->vmem_base, 16*1024*1024);
}

void mapvideo(struct v_board_t *board) {
  board->vmem_base=v_mapmemory(board->mem_base, 16*1024*1024);
}

void handler(int foobar) {
  /* should kick the board into text mode here... */

  v_disableio();
  unmapvideo(board);
  exit(1);
}

void sighandlers(void) {
  signal(SIGSEGV, handler);
  signal(SIGILL, handler);
  signal(SIGBUS, handler);
  signal(SIGKILL, handler);
  signal(SIGQUIT, handler);
}  

    

int main(int argc, char **argv) {
  int c, d, color, inc;
  vu16 iob;
  int width;

  sighandlers();  
  if (NULL == (board=v_findverite())) {
    fprintf(stderr, "vlib: Could not find Verite board\n");
    exit(1);
  }

  v_enableio();
  mapvideo(board);
  iob=board->io_base;
  v_setmodefixed(board);

#define COLOR(r, g, b) (((r)<<11)|((g)<<5)|(b))

  /* write so we can see if we're in graphics mode */
  memset(board->vmem_base, 0, 4*1024*1024);
  sleep(1);

  width=1024;
  for (c=0; c<width*384; c++)
    v_write_memory16(board->vmem_base, c, COLOR(31, 63, 0));
  for (; c<width*768; c++)
    v_write_memory16(board->vmem_base, c, COLOR(31, 0, 31));
  for (c=0; c<768; c++)
    v_write_memory16(board->vmem_base, width*c+width, COLOR(31, 63, 31));
  for (c=0; c<width; c++)
    v_write_memory16(board->vmem_base, width*384+c, COLOR(31, 63, 31));
  for (c=0; c<768; c++)
    v_write_memory16(board->vmem_base, c*width+c, COLOR(31, 63, 31));

  /* the second screen ... ;) */
  /*
  color=0;
  for (c=0; c<768; c++) {
    printf("%d\n", color);
    if (c && (c%24)==0)
      color++;
    for (d=0; d<width; d++)
      v_write_memory16(board->vmem_base, width*768+c*width+d, 
                       COLOR(color, 0, 31-color));
  }

  for (c=0; c<=768; c+=4)
    v_setframebase(board, c*2*width);
  */

  for (c=90; c<=110; c++) {
    v_write_memory16(board->vmem_base, 90*width+c, COLOR(0, 0, 0));
    v_write_memory16(board->vmem_base, 110*width+c, COLOR(0, 0, 0));
    v_write_memory16(board->vmem_base, c*width+90, COLOR(0, 0, 0));
    v_write_memory16(board->vmem_base, c*width+110, COLOR(0, 0, 0));
  }

  goto end;

  /* try something with ucode */
  v_initboard(board);

  /* try to setup mode -- but it does not work */
  inc=8;
  v_out32(iob+inc, 32);
  v_out32(iob+inc, (1024L<<16)|768);
  v_out32(iob+inc, (16L<<16)|V_PIXFMT_565);
  v_out32(iob+inc, 0);
  v_out32(iob+inc, 2048);
  v_out32(iob+inc, 0);

  /* set fg color */
  v_out32(iob+inc, 50);
  v_out32(iob+inc, 0x000000);

  /* set pixel */
  v_out32(iob+inc, 34);
  v_out32(iob+inc, 0x000000);
  v_out32(iob+inc, (100L<<16)|100);

  /* draw rectangle */
  v_out32(iob+inc, 1);
  v_out32(iob+inc, (400L<<16)|400);
  v_out32(iob+inc, (200L<<16)|50);

end:
  getchar();

  /* returning to textmode */
  v_textmode(board);

  return 0;
}


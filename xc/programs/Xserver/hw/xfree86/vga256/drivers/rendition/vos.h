/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/rendition/vos.h,v 1.1.2.3 1998/10/24 07:39:45 hohndel Exp $ */
/*
 * file vos.h
 *
 * layer to map operating system dependent system calls
 */

#ifndef _VOS_H_
#define _VOS_H_



/*
 * includes
 */

#include "vtypes.h"
#include <compiler.h>
#ifdef DEBUG
#include <stdio.h>
#endif
#include <unistd.h>



/*
 * function prototypes
 */

/* IO port programming */
#ifdef DEBUG
#define /*void*/ v_out8(/*vu16*/ port, /*vu8*/ data) \
    printf("v_out8(%x, %x)\n", port, data); \
    outb(port, data)
#define /*void*/ v_out16(/*vu16*/ port, /*vu16*/ data) \
    printf("v_out16(%x, %x)\n", port, data); \
    outw(port, data)
#define /*void*/ v_out32(/*vu16*/ port, /*vu32*/ data) \
    printf("v_out32(%x, %x)\n", port, data); \
    outl(port, data)
#else
#define /*void*/ v_out8(/*vu16*/ port, /*vu8*/ data)   outb(port, data)
#define /*void*/ v_out16(/*vu16*/ port, /*vu16*/ data) outw(port, data)
#define /*void*/ v_out32(/*vu16*/ port, /*vu32*/ data) outl(port, data)
#endif

#define /*vu8*/  v_in8(/*vu16*/ io_base)  ((vu8)inb(io_base))
#define /*vu16*/ v_in16(/*vu16*/ io_base) ((vu16)inw(io_base))
#define /*vu32*/ v_in32(/*vu16*/ io_base) ((vu32)inl(io_base))

/* memory accesses */
#ifdef __alpha__
#define v_read_memory32(base, offset) xf86ReadSparse32(base, 4*(offset))
#define v_read_memory16(base, offset) xf86ReadSparse16(base, 2*(offset))
#define v_read_memory8(base, offset) xf86ReadSparse8(base, offset)
#define v_write_memory32(base, offset, data) xf86WriteSparse32(data,base,4*(offset))
#define v_write_memory16(base, offset, data) xf86WriteSparse16(data,base,2*(offset))
#define v_write_memory8(base, offset, data) xf86WriteSparse8(data,base,offset)
#else
#define /*vu8*/ v_read_memory8(/*vu8 **/ vmem_base, /*vu32*/ offset) \
    (*((vmem_base)+(offset)))
#define /*vu16*/ v_read_memory16(/*vu8 **/ vmem_base, /*vu32*/ offset) \
    (*(((vu16 *)(vmem_base))+(offset)))
#define /*vu32*/ v_read_memory32(/*vu8 **/ vmem_base, /*vu32*/ offset) \
    (*(((vu32 *)(vmem_base))+(offset)))

#define /*void*/ v_write_memory8(/*vu8 **/ vmem_base, /*vu32*/ offset, \
                                  /*vu8*/ data) \
    (*((vmem_base)+(offset)))=(data)
#define /*void*/ v_write_memory16(/*vu8 **/ vmem_base, /*vu32*/ offset, \
                                  /*vu16*/ data) \
    (*(((vu16 *)(vmem_base))+(offset)))=(data)
#define /*void*/ v_write_memory32(/*vu8 **/ vmem_base, /*vu32*/ offset, \
                                  /*vu32*/ data) \
    (*(((vu32 *)(vmem_base))+(offset)))=(data)
#endif


void v_enableio(void);
void v_disableio(void);
vu8 *v_mapmemory(vu8 *membase, vu32 size);
void v_unmapmemory(vu8 *vmembase, vu32 size);

struct v_board_t *v_findverite(void);



#endif /* #ifndef _VOS_H_ */

/*
 * end of file vos.h
 */

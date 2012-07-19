/*
 * Copyright 1998,1999 by Alan Hourihane, Wigan, England.
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Alan Hourihane not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  Alan Hourihane makes no representations
 * about the suitability of this software for any purpose.  It is provided
 * "as is" without express or implied warranty.
 *
 * ALAN HOURIHANE DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL ALAN HOURIHANE BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 *
 * Authors:  Alan Hourihane, alanh@fairlite.demon.co.uk
 *           Mike Chapman <mike@paranoia.com>,
 *           Juanjo Santamarta <santamarta@ctv.es>,
 *           Mitani Hiroshi <hmitani@drl.mei.co.jp>
 *           David Thomas <davtom@dream.org.uk>.
 */

#ifndef _XGI_REGS_H_
#define _XGI_REGS_H_

/* Jong 02/11/2009; replace inb/outb */
#if defined(__arm__) 
#ifndef minb
#define minb(p) MMIO_IN8(0, (p)) 		
#endif
#ifndef moutb
#define moutb(p,v) MMIO_OUT8(0, (p),(v))
#endif
#ifndef minw
#define minw(p) MMIO_IN16(0, (p))
#endif
#ifndef moutw
#define moutw(p,v) MMIO_OUT16(0, (p))
#endif
#ifndef minl
#define minl(p) MMIO_IN32(0, (p))
#endif
#ifndef moutl
#define moutl(p,v) MMIO_OUT32(0, (p), (v))
#endif

/* Jong 02/11/2009; replace inb/outb */
#define inb(p)			minb(p)
#define outb(p, v)		moutb(p, v)
#endif

#include "vgaHW.h"

#define inXGIREG(base)      inb(base)
#define outXGIREG(base,val) outb(base,val)
#define orXGIREG(base,val)  do { \
                                unsigned char temp = inb(base); \
                                outXGIREG(base, temp | (val)); \
                            } while (0)

#define andXGIREG(base,val) do { \
                                unsigned char temp = inb(base); \
                                outXGIREG(base, temp & (val)); \
                            } while (0)

#define inXGIIDXREG(base,idx,var)\
                    do { \
                        outb(base,idx); var=inb((base)+1); \
                    } while (0)

#define outXGIIDXREG(base,idx,val)\
                    do { \
                      outb(base,idx); outb((base)+1,val); \
                    } while (0)

#define orXGIIDXREG(base,idx,val)\
                    do { \
                        unsigned char temp; \
                        outb(base,idx);    \
                        temp = inb((base)+1)|(val); \
                        outXGIIDXREG(base,idx,temp); \
                    } while (0)
#define andXGIIDXREG(base,idx,and)\
                    do { \
                        unsigned char temp; \
                        outb(base,idx);    \
                        temp = inb((base)+1)&(and); \
                        outXGIIDXREG(base,idx,temp); \
                    } while (0)
#define setXGIIDXREG(base,idx,and,or)\
                    do { \
                        unsigned char temp; \
                        outb(base,idx);    \
                        temp = (inb((base)+1)&(and))|(or); \
                        outXGIIDXREG(base,idx,temp); \
                    } while (0)

#define BITMASK(h,l)             (((unsigned)(1U << ((h)-(l)+1))-1)<<(l))
#define GENMASK(mask)            BITMASK(1?mask,0?mask)

#define GETBITS(var,mask)        (((var) & GENMASK(mask)) >> (0?mask))
/* #define SETBITS(val,mask)        ((val) << (0?mask)) */ /* Jong@08032009 */
#define SETBIT(n)                (1<<(n))

#define GETBITSTR(val,from,to)    ((GETBITS(val,from)) << (0?to))
#define SETVARBITS(var,val,from,to)\
                (((var)&(~(GENMASK(to)))) | GETBITSTR(val,from,to))

#define GETVAR8(var)        ((var)&0xFF)
#define SETVAR8(var,val)    (var) =  GETVAR8(val)

#define VGA_RELIO_BASE    0x380

#define AROFFSET       VGA_ATTR_INDEX - VGA_RELIO_BASE
#define ARROFFSET      VGA_ATTR_DATA_R - VGA_RELIO_BASE
#define GROFFSET       VGA_GRAPH_INDEX - VGA_RELIO_BASE
#define SROFFSET       VGA_SEQ_INDEX - VGA_RELIO_BASE
#define CROFFSET       VGA_CRTC_INDEX_OFFSET + VGA_IOBASE_COLOR-VGA_RELIO_BASE
#define MISCROFFSET    VGA_MISC_OUT_R - VGA_RELIO_BASE
#define MISCWOFFSET    VGA_MISC_OUT_W - VGA_RELIO_BASE
#define COLREGOFFSET    0x48
#define INPUTSTATOFFSET 0x5A
#define DACROFFSET     VGA_DAC_READ_ADDR - VGA_RELIO_BASE
#define DACWOFFSET     VGA_DAC_WRITE_ADDR - VGA_RELIO_BASE
#define DACDOFFSET     VGA_DAC_DATA - VGA_RELIO_BASE
#define IS1OFFSET      VGA_IOBASE_COLOR - VGA_RELIO_BASE + VGA_IN_STAT_1_OFFSET

#define XGI_IS1        (pXGI->RelIO+IS1OFFSET)

/**********************************************************************/
#define IS_BIT_DIAGNOSTIC_RB (3<<4)
#define IS_BIT_VERT_ACTIVE (1<<3)
#define IS_BIT_HORZ_NACTIVE (1)
/**********************************************************************/

#define XGIARR         (pXGI->RelIO+ARROFFSET)
#define XGIGR          (pXGI->RelIO+GROFFSET)
#define XGISR          (pXGI->RelIO+SROFFSET)
#define XGICR          (pXGI->RelIO+CROFFSET)
#define XGIMISCR       (pXGI->RelIO+MISCROFFSET)
#define XGIMISCW       (pXGI->RelIO+MISCWOFFSET)
#define XGIDACREAD     (pXGI->RelIO+DACROFFSET)
#define XGIDACWRITE    (pXGI->RelIO+DACWOFFSET)
#define XGIDACDATA     (pXGI->RelIO+DACDOFFSET)
#define XGIVIDEO       (pXGI->RelIO+0x02)
#define XGICOLIDX      (pXGI->RelIO+COLREGOFFSET)
#define XGICOLDATA     (pXGI->RelIO+COLREGOFFSET+1)
#define XGIINPSTAT     (pXGI->RelIO+INPUTSTATOFFSET)
#define XGIPART1       (pXGI->RelIO+0x04)
#define XGIPART2       (pXGI->RelIO+0x10)
#define XGIPART3       (pXGI->RelIO+0x12)
#define XGIPART4       (pXGI->RelIO+0x14)
#define XGIPART5       (pXGI->RelIO+0x16)


/*  PART1 */
#define xgiPART1_FUNCTION                     0x00
#define xgiPART1_THRESHOLD_HIGH               0x01
#define xgiPART1_THRESHOLD_LOW                0x02
#define xgiPART1_FIFO_STOP                    0x03
#define xgiPART1_MEM_ADDR_HIGH                0x04
#define xgiPART1_MEM_ADDR_MID                 0x05
#define xgiPART1_MEM_ADDR_LOW                 0x06
#define xgiPART1_SCR_PITCH_LOW                0x07
#define xgiPART1_HORZ_TOTAL_LOW               0x08
#define xgiPART1_SCR_HTOTAL_OVERFLOW          0x09
#define xgiPART1_HORZ_DISP_END                0x0A
#define xgiPART1_HORZ_RETRACE_START           0x0B
#define xgiPART1_HORZ_OVERFLOW                0x0C
#define xgiPART1_HORZ_RETRACE_END             0x0D

#define xgiPART1_VERT_TOTAL_LOW               0x0E
#define xgiPART1_VERT_DISP_END                0x0F
#define xgiPART1_VERT_RETRACE_START           0x10
#define xgiPART1_VERT_RETRACE_END             0x11
#define xgiPART1_VERT_OVERFLOW                0x12

/* 2000/04/10 added by jjtseng */
/* [VBCTL_000410] */
#define xgiPART1_CRT2_FLIP                    0x24
#define xgiPART1_LOWRES_DUALVB_MODE           0x2c
/* ~jjtseng 2000/04/10 */

#define xgiPART1_ENABLEWRITE                  0x2f
#define xgiPART1_VERTRETRACE                  0x30
#define xgiPART1_HORZRETRACE                  0x33

/* 2005/11/08 added by jjtseng */
#define Index_CR_GPIO_Reg1 0x48
#define Index_CR_GPIO_Reg2 0x49
#define Index_CR_GPIO_Reg3 0x4a

#define GPIOA_EN    (1<<0)
#define GPIOA_WRITE  (1<<0)
#define GPIOA_READ (1<<7)

#define GPIOA_EN    (1<<0)
#define GPIOA_WRITE (1<<0)
#define GPIOA_READ  (1<<7)

#define GPIOB_EN    (1<<1)
#define GPIOB_WRITE (1<<1)
#define GPIOB_READ  (1<<6)

#define GPIOC_EN    (1<<2)
#define GPIOC_WRITE (1<<2)
#define GPIOC_READ  (1<<5)

#define GPIOD_EN    (1<<3)
#define GPIOD_WRITE (1<<3)
#define GPIOD_READ  (1<<4)

#define GPIOE_EN    (1<<4)
#define GPIOE_WRITE (1<<4)
#define GPIOE_READ  (1<<3)

#define GPIOF_EN    (1<<5)
#define GPIOF_WRITE (1<<5)
#define GPIOF_READ  (1<<2)

#define GPIOG_EN    (1<<6)
#define GPIOG_WRITE (1<<6)
#define GPIOG_READ  (1<<1)

#define GPIOH_EN    (1<<7)
#define GPIOH_WRITE (1<<7)
#define GPIOH_READ  (1<<0)

#define XGIMMIOLONG(offset)  *(volatile unsigned long *)(pXGI->IOBase+(offset))
#define XGIMMIOSHORT(offset) *(volatile unsigned short *)(pXGI->IOBase+(offset))
#define XGIMMIOBYTE(offset)  *(volatile unsigned char *)(pXGI->IOBase+(offset))

#endif  /* _XGI_REGS_H_ */

/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/rendition/vvga.c,v 1.1.2.5 1998/10/25 09:49:26 hohndel Exp $ */
/*
 * file vvga.c
 *
 * Functions that handle the generic vga part of the Verite chips.
 */



/*
 * includes
 */

#include "vvga.h"
#include "vtypes.h"
#include "vos.h"
#include "v1kregs.h"
#include "v2kregs.h"
#include "xf86.h"
#include "xf86_OSlib.h"

void set_PLL(vu16, vu32);

/*
 * global data
 */

#ifdef STDVGAFONT
#include "vgafont-std.data"
#else
#include "vgafont-vrx.data"
#endif
#include "vgapalette.data"

#ifdef __alpha__
void v_bustomem(vu8 *dst, vu8 *src, vu32 num)
{
    int i;
    for (i=0; i<num; i++)
        dst[i] = v_read_memory8(src, i);
}

void v_memtobus(vu8 *dst, vu32 offset, vu8 *src, vu32 num)
{
    int i;
    for (i=0; i<num; i++)
        v_write_memory8(dst, offset+i, src[i]);
}
#else
#define v_bustomem(dst, src, num) memcpy(dst,src,num)
#define v_memtobus(dst, offset, src, num) memcpy((dst)+(offset),src,num)
#endif

/*
 * local function prototypes
 */

static vu8 getvgareg(vu16 port, vu8 index);
static void setvgareg(vu16 port, vu8 index, vu8 value);
static void updattr(vu8 index, vu8 value);



/*
 * functions
 */

void v_resetvga(void)
{
    static struct VIDEO_REGS {
        vu8 seq[8];     /* sequencer regs */
        vu8 crtc[26];   /* crtc regs */
        vu8 grph[9];    /* graphics regs */
        vu8 attr[21];   /* attribute regs */
    } mode3={
        /* seq */
        {0x03,0x00,0x03,0x00,0x02,0xfa,0x01,0xfa},

        /* crtc */
        {0x5f,0x4f,0x50,0x82,0x55,0x81,0xbf,0x1f,
         0x00,0x4f,0x0d,0x0e,0x00,0x00,0x01,0xe0,
         0x9c,0xae,0x8f,0x28,0x1f,0x96,0xb9,0xa3,
         0xff,0x19},

        /* grph */
        {0x00,0x00,0x00,0x00,0x00,0x10,0x0e,0x00,0xff},

        /* attr */
        {0x00,0x01,0x02,0x03,0x04,0x05,0x14,
         0x07,0x38,0x39,0x3a,0x3b,0x3c,0x3d,
         0x3e,0x3f,0x0c,0x00,0x0f,0x08,0x00},
    };
    int c;

    /* set attribute controller */
    for (c=0; c<0x15; c++)
        updattr(c, mode3.attr[c]);

    /* set CRTC registers */
    for (c=0; c<0x19; c++)
        setvgareg(0x3d4, c, mode3.crtc[c]);

    /* set graphics registers */
    for (c=0x00; c<0x09; c++)
        setvgareg(0x3ce, c, mode3.grph[c]);

    /* set sequencer registers */
    setvgareg(0x3c4, 0x00, 0x03);        /* restart sequencer */
    for (c=0; c<5; c++)
        setvgareg(0x3c4, c, mode3.seq[c]);
}



void v_loadvgafont(void)
{
    int c;
    vu8 b;
    vu8 *address;
    vu8 *vidmem;
    vu8 *vbase;

    /* Assert synchroneous reset while setting the clock mode */
    setvgareg(0x3c4, 0, 1);               /* assert synchronous reset */
    v_out8(0x3c2, 0x67);                  /* select clock */
    setvgareg(0x3c4, 0, 3);               /* de-assert synchronous reset */

    /* load 8x16 font into plane 2 */
    b=(getvgareg(0x3c4, 0x04)|0x04)&0x07; /* memory mode reg - */
    setvgareg(0x3c4, 0x04, b);            /* disable odd/even and chain 4 */

    /* disable video and enable all to cpu to enable maximum video
     * memory access */
    b=getvgareg(0x3c4, 0x01)|0x20;        /* clocking mode register - */
    setvgareg(0x3c4, 0x01, b);            /* enable all to cpu, disable video */

    b=getvgareg(0x3ce, 0x05)&0xef;        /* graphics controller - */
    setvgareg(0x3ce, 0x05, b);            /* disable odd/even */

    setvgareg(0x3ce, 0x06, 0x05);         /* memory map - set it to A000 
                                           * and graphics mode */

    setvgareg(0x3c4, 2, 4);               /* enable write plane 2 */

    /* fill plane 2 with 8x16 font */
    address=font8x16;
#if defined(__alpha__)
    vbase = (vu8 *)xf86MapVidMemSparse(0, VGA_REGION, (pointer)0xa0000, 64*1024);
#else
    vbase = (vu8 *)xf86MapVidMem(0, VGA_REGION, (pointer)0xa0000, 64*1024);
#endif
    vidmem=vbase;
    for (c=0; c<=255; c++) {
        v_memtobus(vbase, 32*c, address, 16);
	    address+=16;
    }

#if defined(__alpha__)
    xf86UnMapVidMemSparse(0, VGA_REGION, vbase, 64*1024);
#else
    xf86UnMapVidMem(0, VGA_REGION, vbase, 64*1024);
#endif

    /* restore the standard vga register values */
    v_resetvga();
}



void v_textmode(struct v_board_t *board) 
{
    vu16 iob=board->io_base;
    int tmp;

    /* dac */
    v_out8(iob+DACCOMMAND0, 0x80);     /* 6 bit op, enable extended */
    v_out8(iob+DACCOMMAND1, 0x68);     /* disable palette bypass */
    v_out8(iob+DACCOMMAND2, 0x00);     /* disable cursor & pixel packing */
    if (V1000_DEVICE == board->chip) {
        v_out8(iob+DACRAMWRITEADR, 0x01);  /* select COMMAND3 register */
        v_out8(iob+DACCOMMAND3, 0x00);     /* no clock doubling */
    }
    v_out8(iob+DACCOMMAND0, 0x00);     /* 6 bit op */

    if (V1000_DEVICE == board->chip) {
        v_out32(iob+DRAMCTL, 0x140000);
        set_PLL(iob, 0x40000);
        usleep(500);
     }
    else {
        /* memctl */
	tmp = 0x1800|v_in32(iob+DRAMCTL);
        v_out32(iob+DRAMCTL, tmp);    /* linear mode */

        /* pixel clock */
        v_out32(iob+PCLKPLL, 0x300000);
        /* system and memory clock */
        v_out32(iob+SCLKPLL, 0x2480C);     /* mclk=86 sclk=43 */

        /* Need to wait 200uS for PLL to stabilize --
         * let's play it safe with 500 */
        usleep(500);

        /* wait until VBLANK */
        while ((v_in32(iob+CRTCSTATUS)&CRTCSTATUS_VERT_MASK) !=
               CRTCSTATUS_VERT_ACTIVE);
        while ((v_in32(iob+CRTCSTATUS)&CRTCSTATUS_VERT_MASK) ==
               CRTCSTATUS_VERT_ACTIVE);
    }

    /* vga mode */
    v_out8(iob+MODEREG, VGA_MODE);

    /* crtc */
    v_out32(iob+CRTCCTL, 0x44cc2);
    v_out32(iob+CRTCHORZ, 0x2b0a4f);
    v_out32(iob+CRTCVERT, 0x9301df);
    v_out32(iob+CRTCOFFSET, 0x40);

#ifdef SAVEVGA
    v_loadvgafont();
    v_restoretextmode(board);
    v_restorepalette();
#else
#ifdef XSERVER
    v_loadvgafont();
    v_restorepalette();
#endif
#endif

}



void v_savetextmode(struct v_board_t *board) 
{
    vu8 *vbase;

    /* save the cursor position */
    board->cursor_hi=getvgareg(0x3d4, 0xe);
    board->cursor_low=getvgareg(0x3d4, 0xf);

    /* save the screen offset */
    board->offset_hi=getvgareg(0x3d4, 0xc);
    board->offset_low=getvgareg(0x3d4, 0xd);

    /* save the screen contents */
    board->scr_contents=(vu8 *)xalloc(0x8000);
#if defined(__alpha__)
    vbase = (vu8 *)xf86MapVidMemSparse(0, VGA_REGION, (pointer)0xb8000, 0x8000);
#else
    vbase = (vu8 *)xf86MapVidMem(0, VGA_REGION, (pointer)0xb8000, 0x8000);
#endif
    v_bustomem(board->scr_contents, vbase, 0x8000);
#if defined(__alpha__)
    xf86UnMapVidMemSparse(0, VGA_REGION, vbase, 0x8000);
#else
    xf86UnMapVidMem(0, VGA_REGION, vbase, 0x8000);
#endif
}



void v_restoretextmode(struct v_board_t *board) 
{
    vu8 *vbase;

    /* restore the cursor position */
    setvgareg(0x3d4, 0xe, board->cursor_hi);
    setvgareg(0x3d4, 0xf, board->cursor_low);

    /* restore the screen offset */
    setvgareg(0x3d4, 0xc, board->offset_hi);
    setvgareg(0x3d4, 0xd, board->offset_low);

    /* restore the screen contents */
#if defined(__alpha__)
    vbase = (vu8 *)xf86MapVidMemSparse(0, VGA_REGION, (pointer)0xb8000, 0x8000);
#else
    vbase = (vu8 *)xf86MapVidMem(0, VGA_REGION, (pointer)0xb8000, 0x8000);
#endif
    v_memtobus(vbase, 0, board->scr_contents, 0x8000);
#if defined(__alpha__)
    xf86UnMapVidMemSparse(0, VGA_REGION, vbase, 0x8000);
#else
    xf86UnMapVidMem(0, VGA_REGION, vbase, 0x8000);
#endif
    xfree(board->scr_contents);
}



void v_restorepalette(void)
{
    int c;
    vu8 *pal=vga_pal;

    v_out8(0x3c8, 0);
    for (c=0; c<768; c++)
        v_out8(0x3c9, *pal++);
}



/*
 * local functions
 */

/*
 * static vu8 getvgareg(vu16 port, vu8 index)
 *
 * Reads in a vga register.
 */
static vu8 getvgareg(vu16 port, vu8 index)
{
    v_out8(port, index);
    return v_in8(port+1);
}



/*
 * static void setvgareg(vu16 port, vu8 index, vu8 value)
 *
 * Sets a vga register.
 */
static void setvgareg(vu16 port, vu8 index, vu8 value)
{
    v_out8(port, index);
    v_out8(port+1, value);
}



/*
 * static void updattr(vu8 index, vu8 value)
 *
 * Used to write the attribute controller registers.
 */
static void updattr(vu8 index, vu8 value)
{
    v_in8(0x3da);           /* points to index register for color adapter */
    v_in8(0x3ba);           /* points to index register for mono */
    v_out8(0x3c0, index);
    v_out8(0x3c0, value);
    v_out8(0x3c0, index|0x20);
}



/*
 * end of file vvga.c
 */

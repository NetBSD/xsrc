/* $XFree86: xc/programs/Xserver/hw/xfree86/drivers/rendition/vvga.c,v 1.10 2001/02/15 17:50:35 eich Exp $ */
/*
 * file vvga.c
 *
 * Functions that handle the generic vga part of the Verite chips.
 */

/*
 * includes
 */
#include "rendition.h"
#define VVGA_INTERNAL
#include "vvga.h"
#include "vtypes.h"
#include "vos.h"
#include "vmisc.h"
#include "v1kregs.h"
#include "v2kregs.h"

#undef DEBUG

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

/*
 * local function prototypes
 */

static vu8 getvgareg(vu16 port, vu8 index);
static void setvgareg(vu16 port, vu8 index, vu8 value);
static void updattr(vu8 index, vu8 value);



/*
 * functions
 */

static void
verite_resetvga(void)
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

#ifdef DEBUG
    ErrorF ("Rendition: Debug verite_resetvga called\n");
#endif

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



static void
verite_loadvgafont(void)
{
    int c;
    vu8 b;
    vu8 *address;
    vu8 *vidmem;
    vu8 *vbase;
    int fbFlags;

#ifdef DEBUG
    ErrorF ("Rendition: Debug verite_loadvgafont called\n");
#endif

    /* Assert synchroneous reset while setting the clock mode */
    setvgareg(0x3c4, 0, 1);               /* assert synchronous reset */
    verite_out8(0x3c2, 0x67);                  /* select clock */
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
    fbFlags = VIDMEM_MMIO; /* VIDMEM_SPARSE is implied on Alpha */

    vbase = xf86MapVidMem(0, fbFlags, 0xa0000, 64*1024);
    vidmem=vbase;
    for (c=0; c<=255; c++) {
        verite_memtobus_cpy(vbase+(32*c), address, 16);
	    address+=16;
    }

    xf86UnMapVidMem(0, vbase, 64*1024);
    /* restore the standard vga register values */
    verite_resetvga();
}



void
verite_textmode(struct verite_board_t *board) 
{
    vu16 iob=board->io_base;
    int tmp;

#ifdef DEBUG
    ErrorF ("Rendition: Debug verite_textmode called\n");
#endif

    /* dac */
    verite_out8(iob+DACCOMMAND0, 0x80);     /* 6 bit op, enable extended */
    verite_out8(iob+DACCOMMAND1, 0x68);     /* disable palette bypass */
    verite_out8(iob+DACCOMMAND2, 0x00);     /* disable cursor & pixel packing */
    if (V1000_DEVICE == board->chip) {
        verite_out8(iob+DACRAMWRITEADR, 0x01);  /* select COMMAND3 register */
        verite_out8(iob+DACCOMMAND3, 0x00);     /* no clock doubling */
    }
    verite_out8(iob+DACCOMMAND0, 0x00);     /* 6 bit op */

    if (V1000_DEVICE == board->chip) {
        verite_out32(iob+DRAMCTL, 0x140000);
        set_PLL(iob, 0x40000);
        usleep(500);
     }
    else {
        /* memctl */
	tmp = 0x1800|verite_in32(iob+DRAMCTL);
        verite_out32(iob+DRAMCTL, tmp);    /* linear mode */

        /* pixel clock */
        verite_out32(iob+PCLKPLL, 0x300000);
        /* system and memory clock */
        verite_out32(iob+SCLKPLL, 0x2480C);     /* mclk=86 sclk=43 */

        /* Need to wait 200uS for PLL to stabilize --
         * let's play it safe with 500 */
        usleep(500);

        /* wait until VBLANK */
        while ((verite_in32(iob+CRTCSTATUS)&CRTCSTATUS_VERT_MASK) !=
               CRTCSTATUS_VERT_ACTIVE);
        while ((verite_in32(iob+CRTCSTATUS)&CRTCSTATUS_VERT_MASK) ==
               CRTCSTATUS_VERT_ACTIVE);
    }

    /* vga mode */
    verite_out8(iob+MODEREG, VGA_MODE);

    /* crtc */
    verite_out32(iob+CRTCCTL, 0x44cc2);
    verite_out32(iob+CRTCHORZ, 0x2b0a4f);
    verite_out32(iob+CRTCVERT, 0x9301df);
    verite_out32(iob+CRTCOFFSET, 0x40);
#if 0
#ifdef SAVEVGA
    verite_loadvgafont();
    verite_restoretextmode(board);
    verite_restorepalette();
#else
#ifdef XSERVER
    verite_loadvgafont();
    verite_restorepalette();
#endif
#endif
#endif

}



void
verite_savetextmode(struct verite_board_t *board) 
{
    vu8 *vbase;
    int fbFlags;

#ifdef DEBUG
    ErrorF ("Rendition: Debug verite_savetextmode called\n");
#endif

    /* save the cursor position */
    board->cursor_hi=getvgareg(0x3d4, 0xe);
    board->cursor_low=getvgareg(0x3d4, 0xf);

    /* save the screen offset */
    board->offset_hi=getvgareg(0x3d4, 0xc);
    board->offset_low=getvgareg(0x3d4, 0xd);

    /* save the screen contents */
    board->scr_contents=(vu8 *)xalloc(0x8000);
    fbFlags = VIDMEM_MMIO; /* VIDMEM_SPARSE is implied on Alpha */

    vbase = xf86MapVidMem(0, fbFlags, 0xb8000, 0x8000);
    verite_bustomem_cpy(board->scr_contents, vbase, 0x8000);
    xf86UnMapVidMem(0, vbase, 0x8000);
}



static void
verite_restoretextmode(struct verite_board_t *board) 
{
    vu8 *vbase;
    int fbFlags;

#ifdef DEBUG
    ErrorF ("Rendition: Debug verite_restoretextmode called\n");
#endif

    /* restore the cursor position */
    setvgareg(0x3d4, 0xe, board->cursor_hi);
    setvgareg(0x3d4, 0xf, board->cursor_low);

    /* restore the screen offset */
    setvgareg(0x3d4, 0xc, board->offset_hi);
    setvgareg(0x3d4, 0xd, board->offset_low);

    /* restore the screen contents */
    fbFlags = VIDMEM_MMIO; /* VIDMEM_SPARSE is implied on Alpha */

    vbase = xf86MapVidMem(0, fbFlags, 0xb8000, 0x8000);
    verite_memtobus_cpy(vbase, board->scr_contents, 0x8000);
    xf86UnMapVidMem(0, vbase, 0x8000);
    xfree(board->scr_contents);
}



static void
verite_restorepalette(void)
{
    int c;
    vu8 *pal=vga_pal;

#ifdef DEBUG
    ErrorF ("Rendition: Debug verite_restorepalette called\n");
#endif

    verite_out8(0x3c8, 0);
    for (c=0; c<768; c++)
        verite_out8(0x3c9, *pal++);
}



/*
 * local functions
 */

/*
 * static vu8 getvgareg(vu16 port, vu8 index)
 *
 * Reads in a vga register.
 */
static vu8
getvgareg(vu16 port, vu8 index)
{
    verite_out8(port, index);
    return verite_in8(port+1);
}



/*
 * static void setvgareg(vu16 port, vu8 index, vu8 value)
 *
 * Sets a vga register.
 */
static void
setvgareg(vu16 port, vu8 index, vu8 value)
{
    verite_out8(port, index);
    verite_out8(port+1, value);
}



/*
 * static void updattr(vu8 index, vu8 value)
 *
 * Used to write the attribute controller registers.
 */
static void
updattr(vu8 index, vu8 value)
{
    verite_in8(0x3da);           /* points to index register for color adapter */
    verite_in8(0x3ba);           /* points to index register for mono */
    verite_out8(0x3c0, index);
    verite_out8(0x3c0, value);
    verite_out8(0x3c0, index|0x20);
}



/*
 * end of file vvga.c
 */

/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/rendition/vmodes.c,v 1.1.2.8 1999/08/02 08:38:24 hohndel Exp $ */
/*
 * file vmodes.c
 *
 * Routines that handle mode setting.
 */



/*
 * includes
 */

#include "vmodes.h"
#include "vos.h"
#include "vramdac.h"
#include "v1krisc.h"
#include "v1kregs.h"
#include "v2kregs.h"
#include "vvga.h"
#include <math.h>
#include "xf86.h"
#include "xf86_OSlib.h"

#include "vga.h"

#define XCONFIG_FLAGS_ONLY
#include "xf86_Config.h"



/*
 * defines
 */

#define combineNMP(N, M, P) \
    (((vu32)(M-2)<<10) | ((vu32)P<<8) | (vu32)(N-2))

#define v2kcombineNMP(N, M, P) (((vu32)N<<13) | ((vu32)P<<9) | (vu32)M)

/* defines for doubled clock */
#define CLK_DOUBLE		1
#define NO_CLK_DOUBLE	0

/* video FIFO is set to 64 or 128 entries based on req.d bandwidth in bytes/s */
#define FIFOSIZE_THRESH 	100000000 	 /* set thresh to 100MB/s */

/* compute desired video FIFO size given total bandwidth in bytes/s. */
#define FIFOSIZE(vclk, Bpp) (((vclk * Bpp) > FIFOSIZE_THRESH)  ? 128 : 64)

/* Bt485A RAMDAC control bits */
#define PALETTEDISABLE		0x10

/* Hold memory refresh cycles until video blank */
#define HOLD_MEMREFRESHCYCLE 0x2000

/* memCtl bits [16..23] */
#define DEFAULT_WREFRESH	0x330000

/* Disable memory refresh cycles */
#define DISABLE_MEMREFRESHCYCLE		0x8000

#define CTL(ldbl, hsynchi, vsynchi) \
  (((ldbl) ? CRTCCTL_LINEDOUBLE : 0) \
  |((hsynchi) ? CRTCCTL_HSYNCHI : 0) \
  |((vsynchi) ? CRTCCTL_VSYNCHI : 0) \
  |(CRTCCTL_VSYNCENABLE | CRTCCTL_HSYNCENABLE))

#define HORZ(fp, sy, bp, ac) \
  (((((vu32)(((fp)>>3)-1))&7)<<21)|((((vu32)(((sy)>>3)-1))&0x1F)<<16)|((((vu32)(((bp)>>3)-1))&0x3f)<<9)|((((vu32)(((ac)>>3)-1))&0xff)))

#define VERT(fp, sy, bp, ac) \
  (((((vu32)(fp-1))&0x3f)<<20)|((((vu32)(sy-1))&0x7)<<17)|((((vu32)(bp-1))&0x3f)<<11)|((((vu32)(ac-1))&0x7ff)))

#define HORZAC(crtchorz) \
  (((((vu32)crtchorz)&CRTCHORZ_ACTIVE_MASK)+1)<<3)
 
#define HORZBP(crtchorz) \
  ((((((vu32)crtchorz)&CRTCHORZ_BACKPORCH_MASK)>>9)+1)<<3)
 
#define HORZSY(crtchorz) \
  ((((((vu32)crtchorz)&CRTCHORZ_SYNC_MASK)>>16)+1)<<3)
 
#define HORZFP(crtchorz) \
  ((((((vu32)crtchorz)&CRTCHORZ_FRONTPORCH_MASK)>>21)+1)<<3)

#define VERTAC(crtcvert) \
  ((((vu32)crtcvert)&CRTCVERT_ACTIVE_MASK)+1)
 
#define VERTBP(crtcvert) \
  (((((vu32)crtcvert)&CRTCVERT_BACKPORCH_MASK)>>11)+1)
 
#define VERTSY(crtcvert) \
  (((((vu32)crtcvert)&CRTCVERT_SYNC_MASK)>>17)+1)
 
#define VERTFP(crtcvert) \
  (((((vu32)crtcvert)&CRTCVERT_FRONTPORCH_MASK)>>20)+1)

#define PCLK(N, M, P) (N),(M),(P)

#define TIMING_MASK	(CRTCCTL_VIDEOFIFOSIZE128|CRTCCTL_LINEDOUBLE|\
			         CRTCCTL_VSYNCHI|CRTCCTL_HSYNCHI|CRTCCTL_VSYNCENABLE|\
                     CRTCCTL_HSYNCENABLE)

/* addr&7 == 5, 6, 7 are bad */
#define BADADDR(x)	(((x)&(((x)<<1)|((x)<<2)))&0x4)

#define V1_MIN_VCO_FREQ  25
#define V1_MAX_VCO_FREQ  135
#define V1_REF_FREQ      14.318
#define V1_MIN_PCF_FREQ  0.2
#define V1_MAX_PCF_FREQ  5

#define V2_MIN_VCO_FREQ  125
#define V2_MAX_VCO_FREQ  250
#define V2_REF_FREQ      14.31818 /* Eh, is this right? */
#define V2_MIN_PCF_FREQ  1
#define V2_MAX_PCF_FREQ  3



/* 
 * global data
 */



/*
 * local function prototypes
 */

void set_PLL(vu16 iob, vu32 value);
static double V1000CalcClock(double target, int *M, int *N, int *P);
static double V2200CalcClock(double target, int *m, int *n, int *p);



/*
 * functions
 */

int v_setmodefixed(struct v_board_t *board)
{
    int iob=board->io_base;
    int tmp;

#ifdef SAVEVGA
    v_savetextmode(board);
#endif

    v1k_softreset(board);

    /* switching to native mode */
    v_out8(iob+MODEREG, NATIVE_MODE);

    /* flipping some bytes */
    v_out8(iob+MEMENDIAN, MEMENDIAN_HW);

    /* try programming 1024x768@70 in highcolor */
    tmp=v_in32(iob+DRAMCTL)&0xdfff;              /* reset bit 13 */
    v_out32(iob+DRAMCTL, tmp|DEFAULT_WREFRESH);

    /* program pixel clock */
    if (board->chip == V1000_DEVICE) {
        set_PLL(iob, combineNMP(21, 55, 0));
    } 
    else {
	tmp = (~0x1800) & v_in32(iob+DRAMCTL);
        v_out32(iob+DRAMCTL, tmp);
        v_out32(iob+PCLKPLL, v2kcombineNMP(2, 21, 2));
    }
    usleep(500);
  
    v_initdac(board, 16, 0);
  
    v_out32(iob+CRTCHORZ, HORZ(24, 136, 144, 1024));
    v_out32(iob+CRTCVERT, VERT(3, 6, 29, 768));

    board->mode.screenwidth=1024;
    board->mode.virtualwidth=1024;
    board->mode.bitsperpixel=16;
    board->mode.fifosize=128;

    board->init=1;
    v_setframebase(board, 0);

    v_out32(iob+CRTCCTL, CTL(0, 0, 0)
                         |V_PIXFMT_565 
                         |CRTCCTL_VIDEOFIFOSIZE128
                         |CRTCCTL_HSYNCENABLE
                         |CRTCCTL_VSYNCENABLE
                         |CRTCCTL_VIDEOENABLE);

  return 0;
}



int v_setmode(struct v_board_t *board, struct v_modeinfo_t *mode)
{
    int tmp;
    int doubleclock=0;
    int M, N, P;
    int iob=board->io_base;
 
    /* switching to native mode */
    v_out8(iob+MODEREG, NATIVE_MODE|VESA_MODE);

    /* flipping some bytes */
    /* Must be something better to do than this -- FIX */
    switch (mode->bitsperpixel) {
    case 32:
      v_out8(iob+MEMENDIAN, MEMENDIAN_NO);
      break;
    case 16:
      v_out8(iob+MEMENDIAN, MEMENDIAN_HW);
      break;
    case 8:
      v_out8(iob+MEMENDIAN, MEMENDIAN_END);
      break;
    }

    if (OFLG_ISSET(OPTION_OVERCLOCK_MEM, &vga256InfoRec.options)) {
	/* increase Mem/Sys clock to avoid nasty artifacts */
	if (board->chip != V1000_DEVICE) {
	  v_out32(iob+SCLKPLL, 0xa484d);  /* mclk=110 sclk=55 */
					  /* M/N/P/P = 77/5/2/4 */
	  usleep(500);
	}
    }

    /* this has something to do with memory */
    tmp=v_in32(iob+DRAMCTL)&0xdfff;              /* reset bit 13 */
    v_out32(iob+DRAMCTL, tmp|0x330000);
 
    /* program pixel clock */
    if (board->chip == V1000_DEVICE) {
        if (110.0 < V1000CalcClock(mode->clock/1000.0, &M, &N, &P)) {
            P++;
            doubleclock=1;
        }
        set_PLL(iob, combineNMP(N, M, P));
    } 
    else {
	tmp = (~0x1800) & v_in32(iob+DRAMCTL);
        v_out32(iob+DRAMCTL, tmp);
        V2200CalcClock(mode->clock/1000.0, &M, &N, &P);
        v_out32(iob+PCLKPLL, v2kcombineNMP(N, M, P));
    }
    usleep(500);

    /* init the ramdac */
    v_initdac(board, mode->bitsperpixel, doubleclock);

    v_out32(iob+CRTCHORZ, HORZ(mode->hsyncstart-mode->hdisplay, 
                               mode->hsyncend-mode->hsyncstart,
                               mode->htotal-mode->hsyncend,
                               mode->hdisplay));
    v_out32(iob+CRTCVERT, VERT(mode->vsyncstart-mode->vdisplay, 
                               mode->vsyncend-mode->vsyncstart,
                               mode->vtotal-mode->vsyncend,
                               mode->vdisplay));

    /* fill in the mode parameters */
    memcpy(&(board->mode), mode, sizeof(struct v_modeinfo_t));
    board->mode.fifosize=128;
    board->mode.pll_m=M;
    board->mode.pll_n=N;
    board->mode.pll_p=P;
    board->mode.doubleclock=doubleclock;
    if (0 == board->mode.virtualwidth)
        board->mode.virtualwidth=board->mode.screenwidth;

    board->init=1;
    v_setframebase(board, 0);

    /* Need to fix up syncs */

    /* enable the display */
    v_out32(iob+CRTCCTL, CTL(0, mode->hsynchi, mode->vsynchi)
                        |mode->pixelformat
                        |CRTCCTL_VIDEOFIFOSIZE128
                        |CRTCCTL_HSYNCENABLE
                        |CRTCCTL_VSYNCENABLE
                        |CRTCCTL_VIDEOENABLE);
   return 0;
}



void v_setframebase(struct v_board_t *board, vu32 framebase)
{
    vu32 offset;

    int iob=board->io_base;
    int swidth=board->mode.screenwidth;
    int vwidth=board->mode.virtualwidth;
    int bytespp=board->mode.bitsperpixel>>3;
    int fifo_size=board->mode.fifosize;

#ifdef DEBUG
    ErrorF( "w=%d v=%d b=%d f=%d\n", 
        swidth, vwidth, bytespp, fifo_size);
#endif

    /* CRTCOFFSET */
    offset=vwidth*bytespp               /* virtual width in bytes */
          -swidth*bytespp               /* screen width in bytes */
          +((swidth*bytespp)%fifo_size) /* width in bytes modulo fifo size */
        ;

    if (!(   framebase&7            /* framebase multiple of 8? */
          || (swidth*bytespp)%128)) /* screenwidth multiple of fifo size */
      offset+=fifo_size;            /* increment offset by fifosize */

    /* wait for vertical retrace */
#ifndef DEBUG
    if (!board->init) {
        while ((v_in32(iob+CRTCSTATUS) & CRTCSTATUS_VERT_MASK) !=
               CRTCSTATUS_VERT_ACTIVE) ;
        while ((v_in32(iob+CRTCSTATUS) & CRTCSTATUS_VERT_MASK) ==
               CRTCSTATUS_VERT_ACTIVE) ;
    }
    else
        board->init=0;
#endif

    /* framebase */
    v_out32(iob+FRAMEBASEA, framebase);

    /* crtc offset */
    v_out32(iob+CRTCOFFSET, offset&0xffff);

}



/*
 * local functions
 */

/*
 * void set_PLL(vu16 iob, vu32 value)
 *
 * Set PLL clock to desired frequency for the V1000.
 */
void set_PLL(vu16 iob, vu32 value)
{
    vu32 ulD;
    int b;

    /* shift out the 20 serial bits */
    for (b=19; b>=0; b--) {
        ulD=(value>>b)&1;
        v_out8(iob+PLLDEV, (vu8)ulD);
    }
  
    /* read PLL device so the latch is filled with the previously 
     * written value */
    (void)v_in8(iob+PLLDEV);
}



/* Vxx00CalcClock -- finds PLL parameters to match target
 *                   frequency (in megahertz)
 *
 *                   Brute force, but takes less than a tenth
 *                   of a second and the function is only called
 *                   O(1) times during program execution.
 */
static double V1000CalcClock(double target, int *M, int *N, int *P)
{
    double mindiff = 1e10;
    double vco, pcf, diff, freq;
    int mm, nn, pp;

    for (pp=0; pp<4; pp++)
        for (nn=1; nn<=129; nn++)
            for (mm=1; mm<=129; mm++) {
                vco=V1_REF_FREQ*2.0*mm/nn;
                if ((vco<V1_MIN_VCO_FREQ) || (vco>V1_MAX_VCO_FREQ))
                    continue;
            	pcf = V1_REF_FREQ/nn;
            	if ((pcf<V1_MIN_PCF_FREQ) || (pcf>V1_MAX_PCF_FREQ))
                    continue;
            	freq=vco/(1<<pp);
            	diff=fabs(target-freq);
            	if (diff < mindiff) {
                    *M=mm; 
                    *N=nn; 
                    *P=pp;
                    mindiff=diff;
                }
            }
 
    vco=V1_REF_FREQ*2*(*M)/(*N);
    pcf=V1_REF_FREQ/(*N);
    freq=vco/(1<<(*P));

#ifdef DEBUG
    ErrorF(
        "RENDITION: target=%f freq=%f vco=%f pcf=%f n=%d m=%d p=%d\n",
        target, freq, vco, pcf, *N, *M, *P);
#endif

    return freq;
}



static double V2200CalcClock(double target, int *m, int *n, int *p)
{
    double mindiff = 1e10;
    double vco, pcf, diff, freq;
    int mm, nn, pp;

    for (pp=1; pp<=0x0f; pp++)
        for (nn=1; nn<=0x3f; nn++)
            for (mm=1; mm<=0xff; mm++) {
                vco = V2_REF_FREQ*mm/nn;
                if ((vco < V2_MIN_VCO_FREQ) || (vco > V2_MAX_VCO_FREQ))
                    continue;
            	pcf = V2_REF_FREQ/nn;
            	if ((pcf < V2_MIN_PCF_FREQ) || (pcf > V2_MAX_PCF_FREQ))
                    continue;
            	freq = vco/pp;
            	diff = fabs(target-freq);
            	if (diff < mindiff) {
                    *m = mm; *n = nn; *p = pp;
                    mindiff = diff;
                }
            }
 
    vco = V2_REF_FREQ * *m / *n;
    pcf = V2_REF_FREQ / *n;
    freq = vco / *p;

#ifdef DEBUG
    ErrorF(
        "RENDITION: target=%f freq=%f vco=%f pcf=%f n=%d m=%d p=%d\n",
        target, freq, vco, pcf, *n, *m, *p);
#endif

    return freq;
}
 
 

/*
 * end of file vmodes.c
 */

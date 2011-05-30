/* Blit macros */


typedef volatile unsigned char      vuchar; 
typedef vuchar *                    vucharptr;

#if 0
/* in <dev/grfioctl.h> */
struct grfdyninfo {
        int     gdi_fbx;                        /* frame buffer x offset */
        int     gdi_fby;                        /* frame buffer y offset */
        int     gdi_dwidth;             /* displayed part width */
        int     gdi_dheight;            /* displayed part height */
        int     gdi_dx;                 /* displayed part x offset */
        int     gdi_dy;                 /* displayed part y offset */ 
      };

struct  grfinfo {
        caddr_t gd_regaddr;             /* control registers physaddr */
        int     gd_regsize;             /* control registers size */
        caddr_t gd_fbaddr;              /* frame buffer physaddr */
        int     gd_fbsize;              /* frame buffer size */
        short   gd_colors;              /* number of colors */
        short   gd_planes;              /* number of planes */

        int     gd_fbwidth;             /* frame buffer width */
        int     gd_fbheight;            /* frame buffer height */

        struct grfdyninfo gd_dyn;       /* everything changable by GRFIOCSINFO */
/* compatibility... */
#define gd_fbx          gd_dyn.gdi_fbx
#define gd_fby          gd_dyn.gdi_fby
#define gd_dwidth       gd_dyn.gdi_dwidth
#define gd_dheight      gd_dyn.gdi_dheight
#define gd_dx           gd_dyn.gdi_dx
#define gd_dy           gd_dyn.gdi_dy

        /* new for banked pager support */
        int     gd_bank_size;           /* size of a bank (or 0) */
};
#endif


#define InitCLBlt(regs,width,rop,memclk)\
{   volatile unsigned char tmp;\
    *(vuchar *)(regs+0x3c4) = (vuchar) 0x1f;\
    *(vuchar *)(regs+0x3c5) = (vuchar) memclk;/*memclock to 46MHz*/\
    *(vuchar *)(regs+0x3ce) = (vuchar) 0x24;\
    tmp = *(vuchar *)(regs+0x3cf);\
    *(volatile unsigned char *)(regs+0x3cf) = (vuchar) (width & 0xff);\
    *(volatile unsigned char *)(regs+0x3ce) = 0x25;\
    tmp = *(vuchar *)(regs+0x3cf);\
    *(vuchar *)(regs+0x3cf) = (vuchar)((width >> 8) & 0x0f);\
    *(vuchar *)(regs+0x3ce)=(vuchar)0x26;\
    tmp = *(vuchar *)(regs+0x3cf);\
    *(vuchar *)(regs+0x3cf)=(vuchar)(width & 0xff);\
    *(vuchar *)(regs+0x3ce)=(vuchar)0x27;\
    tmp = *(vuchar *)(regs+0x3cf);\
    *(vuchar *)(regs+0x3cf)=(vuchar)((width >> 8) & 0x0f);\
    *(vuchar *)(regs+0x3ce)=(vuchar)0x32;\
    tmp = *(vuchar *)(regs+0x3cf);\
    *(vuchar *)(regs+0x3cf)= (vuchar)(rop);\
}\

/* init pattern-color-expansion-fill. init rop,
 * source address and init dest pitch
 */

#define InitCLBltPatCE(regs,width,rop,pat,memclk)\
{   volatile unsigned char tmp;\
    *(vuchar *)(regs+0x3c4) = (vuchar) 0x1f;\
    *(vuchar *)(regs+0x3c5) = (vuchar) memclk;/*memclock to 46MHz*/\
    *(vuchar *)(regs+0x3ce) = (vuchar) 0x24;\
    tmp = *(vuchar *)(regs+0x3cf);\
    *(vuchar *)(regs+0x3cf) = (vuchar) (width & 0xff);\
    *(volatile unsigned char *)(regs+0x3ce) = 0x25;\
    tmp = *(vuchar *)(regs+0x3cf);\
    *(vuchar *)(regs+0x3cf) = (vuchar)((width >> 8) & 0x0f);\
    *(vuchar *)(regs+0x3ce)=(vuchar)0x2c;\
    tmp = *(vuchar *)(regs+0x3cf);\
    *(vuchar *)(regs+0x3cf)=(vuchar)(pat & 0xff);\
    *(vuchar *)(regs+0x3ce)=(vuchar)0x2d;\
    tmp = *(vuchar *)(regs+0x3cf);\
    *(vuchar *)(regs+0x3cf)=(vuchar)((pat >> 8) & 0xff);\
    *(vuchar *)(regs+0x3ce)=(vuchar)0x2e;\
    tmp = *(vuchar *)(regs+0x3cf);\
    *(vuchar *)(regs+0x3cf)=(vuchar)((pat >> 16) & 0x1f);\
    *(vuchar *)(regs+0x3ce)=(vuchar)0x32;\
    tmp = *(vuchar *)(regs+0x3cf);\
    *(vuchar *)(regs+0x3cf)= (vuchar)(rop);\
}\



#define CLBlt(regs,w,h,fbw,x1,y1,x2,y2)\
{   unsigned int src,dst;\
    volatile unsigned char tmp;\
    *(vuchar *)(regs+0x3ce)=(vuchar)0x20;\
    tmp = *(vuchar *)(regs+0x3cf);\
    *(vuchar *)(regs+0x3cf)= (vuchar)((w-1) & 0xff);\
    *(vuchar *)(regs+0x3ce)=(vuchar)0x21;\
    tmp = *(vuchar *)(regs+0x3cf);\
    *(vuchar *)(regs+0x3cf)= (vuchar)(((w-1) >> 8) & 0x07);\
    *(vuchar *)(regs+0x3ce)=(vuchar)0x22;\
    tmp = *(vuchar *)(regs+0x3cf);\
    *(vuchar *)(regs+0x3cf)= (vuchar)((h-1) & 0xff);\
    *(vuchar *)(regs+0x3ce)=(vuchar)0x23;\
    tmp = *(vuchar *)(regs+0x3cf);\
    *(vuchar *)(regs+0x3cf)= (vuchar)(((h-1) >> 8) & 0x03);\
    src = (y1*fbw) + x1;\
    dst = (y2*fbw) + x2;\
    /*fprintf(debugfp," src %d dst %d\n",src,dst);*/\
    if(src>=dst){\
      *(unsigned char *)(regs+0x3ce)=(unsigned char)0x30;\
      tmp = *(vuchar *)(regs+0x3cf);\
      *(unsigned char *)(regs+0x3cf)=(unsigned char)0x00;\
    }else{\
      *(unsigned char *)(regs+0x3ce)=(unsigned char)0x30;\
      tmp = *(vuchar *)(regs+0x3cf);\
      *(unsigned char *)(regs+0x3cf)=(unsigned char)0x01;\
      src = (unsigned int)(((y1+h-1)*fbw) + ((x1+w-1)));\
      dst = (unsigned int)(((y2+h-1)*fbw) + ((x2+w-1)));\
    }\
    *(vuchar *)(regs+0x3ce)=(vuchar)0x28;\
    tmp = *(vuchar *)(regs+0x3cf);\
    *(vuchar *)(regs+0x3cf)= (vuchar)(dst & 0xff);\
    *(vuchar *)(regs+0x3ce)=(vuchar)0x29;\
    tmp = *(vuchar *)(regs+0x3cf);\
    *(vuchar *)(regs+0x3cf)= (vuchar)((dst >> 8) & 0xff);\
    *(vuchar *)(regs+0x3ce)=(vuchar)0x2a;\
    tmp = *(vuchar *)(regs+0x3cf);\
    *(vuchar *)(regs+0x3cf)= (vuchar)((dst >> 16) & 0x1f);\
    *(vuchar *)(regs+0x3ce)=(vuchar)0x2c;\
    tmp = *(vuchar *)(regs+0x3cf);\
    *(vuchar *)(regs+0x3cf)= (vuchar)(src & 0xff);\
    *(vuchar *)(regs+0x3ce)=(vuchar)0x2d;\
    tmp = *(vuchar *)(regs+0x3cf);\
    *(vuchar *)(regs+0x3cf)=(vuchar)((src >> 8) & 0xff);\
    *(vuchar *)(regs+0x3ce)=(vuchar)0x2e;\
    tmp = *(vuchar *)(regs+0x3cf);\
    *(vuchar *)(regs+0x3cf)= (vuchar)((src >> 16) & 0x1f);\
    *(vuchar *)(regs+0x3ce)=(vuchar)0x31;\
    tmp = *(vuchar *)(regs+0x3cf);\
    *(vuchar *)(regs+0x3cf)=(vuchar)0x00;\
    *(vuchar *)(regs+0x3cf)=(vuchar)0x02;\
}\

/* color-expanded pattern fill (also rectangles, solid fills) */

#define CLBltPatCE(regs,w,h,fbw,x1,y1,fg,bg)\
{   unsigned int dst;\
    volatile unsigned char tmp;\
    *(vuchar *)(regs+0x3ce)=(vuchar)0x20;\
    tmp = *(vuchar *)(regs+0x3cf);\
    *(vuchar *)(regs+0x3cf)= (vuchar)((w-1) & 0xff);\
    *(vuchar *)(regs+0x3ce)=(vuchar)0x21;\
    tmp = *(vuchar *)(regs+0x3cf);\
    *(vuchar *)(regs+0x3cf)= (vuchar)(((w-1) >> 8) & 0x07);\
    *(vuchar *)(regs+0x3ce)=(vuchar)0x22;\
    tmp = *(vuchar *)(regs+0x3cf);\
    *(vuchar *)(regs+0x3cf)= (vuchar)((h-1) & 0xff);\
    *(vuchar *)(regs+0x3ce)=(vuchar)0x23;\
    tmp = *(vuchar *)(regs+0x3cf);\
    *(vuchar *)(regs+0x3cf)= (vuchar)(((h-1) >> 8) & 0x03);\
    dst = (y1*fbw) + x1;\
    \
    *(unsigned char *)(regs+0x3ce)=(unsigned char)0x30;\
    tmp = *(vuchar *)(regs+0x3cf);\
    *(unsigned char *)(regs+0x3cf)=(unsigned char)0xc0;/* col-exp-pat */ \
    \
    *(vuchar *)(regs+0x3ce)=(vuchar)0x0;\
    tmp = *(vuchar *)(regs+0x3cf);\
    *(vuchar *)(regs+0x3cf)= (vuchar)((bg) & 0xff);\
    *(vuchar *)(regs+0x3ce)=(vuchar)0x10;\
    tmp = *(vuchar *)(regs+0x3cf);\
    *(vuchar *)(regs+0x3cf)= (vuchar)(((bg) >> 8) & 0xff);\
    *(vuchar *)(regs+0x3ce)=(vuchar)0x01;\
    tmp = *(vuchar *)(regs+0x3cf);\
    *(vuchar *)(regs+0x3cf)= (vuchar)((fg) & 0xff);\
    *(vuchar *)(regs+0x3ce)=(vuchar)0x11;\
    tmp = *(vuchar *)(regs+0x3cf);\
    *(vuchar *)(regs+0x3cf)= (vuchar)((fg >> 8) & 0xff);\
    \
    *(vuchar *)(regs+0x3ce)=(vuchar)0x28;\
    tmp = *(vuchar *)(regs+0x3cf);\
    *(vuchar *)(regs+0x3cf)= (vuchar)(dst & 0xff);\
    *(vuchar *)(regs+0x3ce)=(vuchar)0x29;\
    tmp = *(vuchar *)(regs+0x3cf);\
    *(vuchar *)(regs+0x3cf)= (vuchar)((dst >> 8) & 0xff);\
    *(vuchar *)(regs+0x3ce)=(vuchar)0x2a;\
    tmp = *(vuchar *)(regs+0x3cf);\
    *(vuchar *)(regs+0x3cf)= (vuchar)((dst >> 16) & 0x1f);\
    \
    *(vuchar *)(regs+0x3ce)=(vuchar)0x31;\
    tmp = *(vuchar *)(regs+0x3cf);\
    *(vuchar *)(regs+0x3cf)=(vuchar)0x00;\
    *(vuchar *)(regs+0x3cf)=(vuchar)0x02;\
}\



#define CLWaitBlt(regs)\
{\
   *(regs+0x3ce)=0x31;\
   while((*(regs+0x3cf))&1);\
   *(regs+0x3ce)=0x0;\
   *(regs+0x3cf)=0x00;\
   *(regs+0x3ce)=0x01;\
   *(regs+0x3cf)=0x00;\
}\

#define amigaInfo(s) (&amigaFbs[(s)->myNum])
#if 0

typedef struct {
    unsigned char*  fb;         /* Frame buffer itself */
    unsigned char*  regs;       /* for accel display, direct access regs */
    int             fd;         /* frame buffer for ioctl()s, */
    struct grfinfo  info;       /* Frame buffer characteristics */
    void            (*EnterLeave)();/* screen switch */
    int             type;       /* index into the amigaFbData table */
} fbFd;
#endif

void
clFillRectSolidCopy(
    DrawablePtr	    pDrawable,
    GCPtr	    pGC,
    int		    nBox,
    BoxPtr	    pBox);

extern fbFd             amigaFbs[];



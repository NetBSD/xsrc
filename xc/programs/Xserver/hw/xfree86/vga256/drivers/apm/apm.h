/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/apm/apm.h,v 1.1.2.2 1998/02/15 23:32:05 robin Exp $ */



#ifndef APM_H_
#define APM_H_

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned long u32;

/* Memory mapped access to extended registers */
#define RDXB(addr)     (*(u8*)(apmRegBase+(addr)))
#define RDXW(addr)     (*(u16*)(apmRegBase+(addr)))
#define RDXL(addr)     (*(u32*)(apmRegBase+(addr)))
#define WRXB(addr,val) (void)(*(u8*)(apmRegBase+(addr)) = (val))
#define WRXW(addr,val) (void)(*(u16*)(apmRegBase+(addr)) = (val))
#define WRXL(addr,val) (void)(*(u32*)(apmRegBase+(addr)) = (val))

/* IO port access to extended registers */
#define RDXB_IOP(addr)     (wrinx(0x3c4, 0x1d, (addr) >> 2),inb(apm_xbase + ((addr) & 3)))
#define RDXW_IOP(addr)     (wrinx(0x3c4, 0x1d, (addr) >> 2),inw(apm_xbase + ((addr) & 2)))
#define RDXL_IOP(addr)     (wrinx(0x3c4, 0x1d, (addr) >> 2),inl(apm_xbase))
#define WRXB_IOP(addr,val) (wrinx(0x3c4, 0x1d, (addr) >> 2),outb(apm_xbase + ((addr) & 3), (val)))
#define WRXW_IOP(addr,val) (wrinx(0x3c4, 0x1d, (addr) >> 2),outw(apm_xbase + ((addr) & 2), (val)))
#define WRXL_IOP(addr,val) (wrinx(0x3c4, 0x1d, (addr) >> 2),outl(apm_xbase, (val)))

/* Dynamically accessed registers, either IO port or memory mapped. Kludge, kludge... :-) */
#define RDXB_DYN(addr)     (apmMMIO_Init ? RDXB(addr) : RDXB_IOP(addr))
#define RDXW_DYN(addr)     (apmMMIO_Init ? RDXW(addr) : RDXW_IOP(addr))
#define RDXL_DYN(addr)     (apmMMIO_Init ? RDXL(addr) : RDXL_IOP(addr))
#define WRXB_DYN(addr,val) (apmMMIO_Init ? WRXB(addr,val) : (void) WRXB_IOP(addr,val))
#define WRXW_DYN(addr,val) (apmMMIO_Init ? WRXW(addr,val) : (void) WRXW_IOP(addr,val))
#define WRXL_DYN(addr,val) (apmMMIO_Init ? WRXL(addr,val) : (void) WRXL_IOP(addr,val))

#define STATUS()           (RDXW(0x1fc))
#define STATUS_HOSTBLTBUSY (1 << 8)
#define STATUS_ENGINEBUSY  (1 << 10)
#define STATUS_FIFO        (0x0f)

#define SETFOREGROUNDCOLOR(c) WRXL(0x60,c)
#define SETBACKGROUNDCOLOR(c) WRXL(0x64,c)

#define SETSOURCEX(x)  WRXW(0x50, x)
#define SETSOURCEY(y)  WRXW(0x52, y)
#define SETSOURCEXY(x,y) WRXL(0x50, (y) << 16 | (x) & 0xffff)

#define SETDESTX(x)    WRXW(0x54, x)
#define SETDESTY(y)    WRXW(0x56, y)
#define SETDESTXY(x,y) WRXL(0x54, (y) << 16 | (x) & 0xffff)

#define SETWIDTH(w)    WRXW(0x58, w)
#define SETHEIGHT(h)   WRXW(0x5A, h)
#define SETWIDTHHEIGHT(w,h) WRXL(0x58, (h) << 16 | (w) & 0xffff)

#define SETBYTEMASK(mask) WRXB(0x47, mask)

#define SETDDA_AXIALSTEP(step)    WRXW(0x70, step)
#define SETDDA_DIAGONALSTEP(step) WRXW(0x72, step)
#define SETDDA_ERRORTERM(eterm)   WRXW(0x74, eterm)
#define SETDDA_ADSTEP(s1,s2) WRXL(0x70, (s2) << 16 | (s1) & 0xffff)

#define SETCLIP_CTRL(ctrl) WRXB(0x30, ctrl)
#define SETCLIP_LEFT(x)    WRXW(0x38, x)
#define SETCLIP_TOP(y)     WRXW(0x3A, y)
#define SETCLIP_LEFTTOP(x,y) WRXL(0x38, (y) << 16 | (x) & 0xffff)
#define SETCLIP_RIGHT(x)   WRXW(0x3C, x)
#define SETCLIP_BOT(y)     WRXW(0x3E, y)
#define SETCLIP_RIGHTBOT(x,y) WRXL(0x3C, (y) << 16 | (x) & 0xffff)

/* RASTER OPERATION REGISTER */
/* P = pattern   S = source   D = destination */
#define SETROP(rop)        WRXB(0x46, rop)
#define ROP_P_and_S_and_D  0x80
#define ROP_S_xor_D        0x66
#define ROP_S              0xCC
#define ROP_P              0xF0
/* Then there are about 252 more operations ... */


/* DRAWING ENGINE CONTROL REGISTER */
#define SETDEC(control)             WRXL(0x40, control)
#define DEC_OP_VECT_NOENDP          0x0000000D
#define DEC_OP_VECT_ENDP            0x0000000C
#define DEC_OP_HOSTBLT_SCREEN2HOST  0x00000009
#define DEC_OP_HOSTBLT_HOST2SCREEN  0x00000008
#define DEC_OP_STRIP                0x00000004
#define DEC_OP_BLT_STRETCH          0x00000003
#define DEC_OP_RECT                 0x00000002
#define DEC_OP_BLT                  0x00000001
#define DEC_OP_NOOP                 0x00000000
#define DEC_DIR_X_NEG               (1 << 6)
#define DEC_DIR_X_POS               (0 << 6)
#define DEC_DIR_Y_NEG               (1 << 7)
#define DEC_DIR_Y_POS               (0 << 7)
#define DEC_MAJORAXIS_X             (0 << 8)
#define DEC_MAJORAXIS_Y             (1 << 8)
#define DEC_SOURCE_LINEAR           (1 << 9) 
#define DEC_SOURCE_XY               (0 << 9)
#define DEC_SOURCE_CONTIG           (1 << 11)
#define DEC_SOURCE_RECTANGULAR      (0 << 11)
#define DEC_SOURCE_MONOCHROME       (1 << 12)
#define DEC_SOURCE_COLOR            (0 << 12)
#define DEC_SOURCE_TRANSPARENCY     (1 << 13)
#define DEC_SOURCE_NO_TRANSPARENCY  (0 << 13)
#define DEC_BITDEPTH_24             (4 << 14)
#define DEC_BITDEPTH_32             (3 << 14)
#define DEC_BITDEPTH_16             (2 << 14)
#define DEC_BITDEPTH_8              (1 << 14)
#define DEC_BITDEPTH_COMPAT         (0 << 14)
#define DEC_DEST_LINEAR             (1 << 18)
#define DEC_DEST_XY                 (0 << 18)
#define DEC_DEST_RECTANGULAR        (1 << 19)
#define DEC_DEST_OTHER              (0 << 19)
#define DEC_DEST_TRANSPARENCY       (1 << 20)
#define DEC_DEST_NO_TRANSPARENCY    (0 << 20)
#define DEC_DEST_TRANSP_POLARITY    (1 << 21)
#define DEC_DEST_TRANSP_POLARITYINV (0 << 21)
#define DEC_PATTERN_88_8bCOLOR      (3 << 22)
#define DEC_PATTERN_88_1bMONO       (2 << 22)
#define DEC_PATTERN_44_4bDITHER     (1 << 22)
#define DEC_PATTERN_NONE            (0 << 22)
#define DEC_WIDTH_1600              (7 << 24)
#define DEC_WIDTH_1280              (6 << 24)
#define DEC_WIDTH_1152              (5 << 24)
#define DEC_WIDTH_1024              (4 << 24)
#define DEC_WIDTH_800               (2 << 24)
#define DEC_WIDTH_640               (1 << 24)
#define DEC_WIDTH_LINEAR            (0 << 24)
#define DEC_DEST_UPD_LASTPIX        (3 << 27)
#define DEC_DEST_UPD_BLCORNER       (2 << 27)
#define DEC_DEST_UPD_TRCORNER       (1 << 27)
#define DEC_DEST_UPD_NONE           (0 << 27)
#define DEC_QUICKSTART_ONDEST       (3 << 29)
#define DEC_QUICKSTART_ONSOURCE     (2 << 29)
#define DEC_QUICKSTART_ONDIMX       (1 << 29)
#define DEC_QUICKSTART_NONE         (0 << 29)
#define DEC_START                   (1 << 31)
#define DEC_START_NO                (0 << 31)


#define AP6422  0
#define AT24    1
#define AT3D    2

extern volatile u8* apmRegBase;
extern vgaVideoChipRec APM;
extern vgaHWCursorRec  vgaHWCursor;
extern int apmMMIO_Init;
extern u32 apm_xbase;
extern int apmChip;

void ApmAccelInit(void);
void ApmCheckMMIO_Init(void);

/* Variables defined in apm_cursor.c. */

extern int apmCursorHotX;
extern int apmCursorHotY;
extern int apmCursorWidth;
extern int apmCursorHeight;

/* Functions defined in apm_cursor.c. */

Bool  ApmCursorInit(char *pm, ScreenPtr pScr);
void  ApmRestoreCursor(ScreenPtr pScr);
void  ApmWarpCursor(ScreenPtr pScr, int x, int y);
void  ApmQueryBestSize(int class, unsigned short *pwidth, unsigned short *pheight, ScreenPtr pScreen);


#endif

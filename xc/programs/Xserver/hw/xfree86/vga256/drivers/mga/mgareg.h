/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/mga/mgareg.h,v 3.1 1996/09/29 13:40:26 dawes Exp $ */

/*
 * mga register file 
 *
 */ 

#ifndef _MGAREG_H_
#define _MGAREG_H_

#define	MGAREG_DWGCTL		0x1c00
#define	MGAREG_MACCESS		0x1c04
#define	MGAREG_ZORG		0x1c0c

#define	MGAREG_PAT0		0x1c10
#define	MGAREG_PAT1		0x1c14
#define	MGAREG_PLNWT		0x1c1c

#define	MGAREG_BCOL		0x1c20
#define	MGAREG_FCOL		0x1c24

#define	MGAREG_SRC0		0x1c30
#define	MGAREG_SRC1		0x1c34
#define	MGAREG_SRC2		0x1c38
#define	MGAREG_SRC3		0x1c3c

#define	MGAREG_XYSTRT		0x1c40
#define	MGAREG_XYEND		0x1c44

#define	MGAREG_SHIFT		0x1c50
#define	MGAREG_SGN		0x1c58
#define	MGAREG_LEN		0x1c5c

#define	MGAREG_AR0		0x1c60
#define	MGAREG_AR1		0x1c64
#define	MGAREG_AR2		0x1c68
#define	MGAREG_AR3		0x1c6c
#define	MGAREG_AR4		0x1c70
#define	MGAREG_AR5		0x1c74
#define	MGAREG_AR6		0x1c78

#define	MGAREG_CXBNDRY		0x1c80
#define	MGAREG_FXBNDRY		0x1c84
#define	MGAREG_YDSTLEN		0x1c88
#define	MGAREG_PITCH		0x1c8c

#define	MGAREG_YDST		0x1c90
#define	MGAREG_YDSTORG		0x1c94
#define	MGAREG_YTOP		0x1c98
#define	MGAREG_YBOT		0x1c9c

#define	MGAREG_CXLEFT		0x1ca0
#define	MGAREG_CXRIGHT		0x1ca4
#define	MGAREG_FXLEFT		0x1ca8
#define	MGAREG_FXRIGHT		0x1cac

#define	MGAREG_XDST		0x1cb0

#define	MGAREG_DR0		0x1cc0
#define	MGAREG_DR1		0x1cc4
#define	MGAREG_DR2		0x1cc8
#define	MGAREG_DR3		0x1ccc

#define	MGAREG_DR4		0x1cd0
#define	MGAREG_DR5		0x1cd4
#define	MGAREG_DR6		0x1cd8
#define	MGAREG_DR7		0x1cdc

#define	MGAREG_DR8		0x1ce0
#define	MGAREG_DR9		0x1ce4
#define	MGAREG_DR10		0x1ce8
#define	MGAREG_DR11		0x1cec

#define	MGAREG_DR12		0x1cf0
#define	MGAREG_DR13		0x1cf4
#define	MGAREG_DR14		0x1cf8
#define	MGAREG_DR15		0x1cfc

/* add or or this to one of the previous "power registers" to start
   the drawing engine */

#define MGAREG_EXEC		0x0100

#define	MGAREG_FIFOSTATUS	0x1e10
#define	MGAREG_Status		0x1e14
#define	MGAREG_ICLEAR		0x1e18
#define	MGAREG_IEN		0x1e1c

#define	MGAREG_VCOUNT		0x1e20

#define	MGAREG_Reset		0x1e40

#define	MGAREG_OPMODE		0x1e54

/* DWGCTL register additives */

/* Lines */

#define MGADWG_LINE_OPEN	0x00
#define MGADWG_AUTOLINE_OPEN	0x01
#define MGADWG_WRITE_LAST	0x02
#define MGADWG_AUTOLINE_CLOSE	0x03

/* Trapezoids */
#define MGADWG_TRAP		0x04
#define MGADWG_TEXTURE_TRAP	0x05

/* BitBlts */

#define MGADWG_BITBLT		0b1000
#define MGADWG_FBITBLT		0b1100
#define MGADWG_ILOAD		0b1001
#define MGADWG_ILOAD_SCALE	0b1101
#define MGADWG_ILOAD_FILTER	0b1111
#define MGADWG_IDUMP		0b1010

/* atype access to WRAM */

#define MGADWG_RPL		( 0b000 << 4 )
#define MGADWG_RSTR		( 0b001 << 4 )
#define MGADWG_ZI		( 0b011 << 4 )
#define MGADWG_BLK 		( 0b100 << 4 )
#define MGADWG_I		( 0b111 << 4 )

/* specifies whether bit blits are linear or xy */
#define MGADWG_LINEAR		( 0x01 << 7 )

/* z drawing mode. use MGADWG_NOZCMP for always */

#define MGADWG_NOZCMP		( 0b000 << 8 )
#define MGADWG_ZE		( 0b010 << 8 ) 
#define MGADWG_ZNE		( 0b011 << 8 )
#define MGADWG_ZLT		( 0b100 << 8 )
#define MGADWG_ZLTE		( 0b101 << 8 )
#define MGADWG_GT		( 0b110 << 8 )
#define MGADWG_GTE		( 0b111 << 8 )

/* use this to force colour expansion circuitry to do its stuff */

#define MGADWG_SOLID		( 0x01 << 11 )

/* ar register at zero */

#define MGADWG_ARZERO		( 0x01 << 12 )

#define MGADWG_SGNZERO		( 0x01 << 13 )

#define MGADWG_SHIFTZERO	( 0x01 << 14 )

/* See table on 4-43 for bop ALU operations */

/* See table on 4-44 for translucidity masks */

#define MGADWG_BMONOLEF		( 0b0000 << 25 )
#define MGADWG_BMONOWF		( 0b0100 << 25 )
#define MGADWG_BPLAN		( 0b0001 << 25 )

/* note that if bfcol is specified and you're doing a bitblt, it causes
   a fbitblt to be performed, so check that you obey the fbitblt rules */

#define MGADWG_BFCOL   		( 0b0010 << 25 )
#define MGADWG_BUYUV		( 0b1110 << 25 )
#define MGADWG_BU32BGR		( 0b0011 << 25 )
#define MGADWG_BU32RGB		( 0b0111 << 25 )
#define MGADWG_BU24BGR		( 0b1011 << 25 )
#define MGADWG_BU24RGB		( 0b1111 << 25 )

#define MGADWG_PATTERN		( 0x01 << 29 )
#define MGWDWG_TRANSC		( 0x01 << 30 )

/* MGA registers in PCI config space */
#define PCI_MGA_INDEX		0x44
#define PCI_MGA_DATA		0x48

#define RAMDAC_OFFSET		0x3c00

/* TVP3026 direct registers */

#define TVP3026_INDEX		0x00
#define TVP3026_WADR_PAL	0x00
#define TVP3026_COL_PAL		0x01
#define TVP3026_PIX_RD_MSK	0x02
#define TVP3026_RADR_PAL	0x03
#define TVP3026_CUR_COL_ADDR	0x04
#define TVP3026_CUR_COL_DATA	0x05
#define TVP3026_DATA		0x0a
#define TVP3026_CUR_RAM		0x0b
#define TVP3026_CUR_XLOW	0x0c
#define TVP3026_CUR_XHI		0x0d
#define TVP3026_CUR_YLOW	0x0e
#define TVP3026_CUR_YHI		0x0f

/* TVP3026 indirect registers */

#define TVP3026_SILICON_REV	0x01
#define TVP3026_CURSOR_CTL	0x06
#define TVP3026_LATCH_CTL	0x0f
#define TVP3026_TRUE_COLOR_CTL	0x18
#define TVP3026_MUX_CTL		0x19
#define TVP3026_CLK_SEL		0x1a
#define TVP3026_PAL_PAGE	0x1c
#define TVP3026_GEN_CTL		0x1d
#define TVP3026_MISC_CTL	0x1e
#define TVP3026_GEN_IO_CTL	0x2a
#define TVP3026_GEN_IO_DATA	0x2b
#define TVP3026_PLL_ADDR	0x2c
#define TVP3026_PIX_CLK_DATA	0x2d
#define TVP3026_MEM_CLK_DATA	0x2e
#define TVP3026_LOAD_CLK_DATA	0x2f
#define TVP3026_KEY_RED_LOW	0x32
#define TVP3026_KEY_RED_HI	0x33
#define TVP3026_KEY_GREEN_LOW	0x34
#define TVP3026_KEY_GREEN_HI	0x35
#define TVP3026_KEY_BLUE_LOW	0x36
#define TVP3026_KEY_BLUE_HI	0x37
#define TVP3026_KEY_CTL		0x38
#define TVP3026_MCLK_CTL	0x39
#define TVP3026_SENSE_TEST	0x3a
#define TVP3026_TEST_DATA	0x3b
#define TVP3026_CRC_LSB		0x3c
#define TVP3026_CRC_MSB		0x3d
#define TVP3026_CRC_CTL		0x3e
#define TVP3026_ID		0x3f
#define TVP3026_RESET		0xff

#endif

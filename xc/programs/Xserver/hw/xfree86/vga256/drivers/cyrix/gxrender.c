/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/cyrix/gxrender.c,v 1.1.2.1 1999/06/24 05:59:41 hohndel Exp $ */
/*
//---------------------------------------------------------------------------
// gxrender.c
//
// This file gives examples of using the MediaGX graphics unit to provide
// acceleration for 2D display drivers.  It is intended to provide an 
// absraction layer for new display driver development.  This code handles
// the quirks of the MediaGX graphics unit to allow faster developemt times
// for new drivers.
//---------------------------------------------------------------------------
*/

/* GRAPHICS PIPELINE REGISTER DEFINITIONS */

#define GP_DST_XCOOR		0x8100		/* x destination origin	*/
#define GP_DST_YCOOR		0x8102		/* y destination origin	*/
#define GP_WIDTH			0x8104		/* pixel width */
#define GP_HEIGHT			0x8106		/* pixel height */
#define GP_SRC_XCOOR		0x8108		/* x source origin */
#define GP_SRC_YCOOR		0x810A		/* y source origin */

#define GP_VECTOR_LENGTH	0x8104		/* vector length */
#define GP_INIT_ERROR		0x8106		/* vector initial error	*/
#define GP_AXIAL_ERROR		0x8108		/* axial error increment */
#define GP_DIAG_ERROR		0x810A		/* diagonal error increment */

#define GP_SRC_COLOR_0		0x810C		/* source color 0 */
#define GP_SRC_COLOR_1		0x810E		/* source color 1 */
#define GP_PAT_COLOR_0		0x8110		/* pattern color 0 */
#define GP_PAT_COLOR_1		0x8112		/* pattern color 1 */
#define GP_PAT_COLOR_2		0x8114		/* pattern color 2 */
#define GP_PAT_COLOR_3		0x8116		/* pattern color 3 */
#define GP_PAT_DATA_0		0x8120		/* bits 31:0 of pattern	*/
#define GP_PAT_DATA_1		0x8124		/* bits 63:32 of pattern */
#define GP_PAT_DATA_2		0x8128		/* bits 95:64 of pattern */
#define GP_PAT_DATA_3		0x812C		/* bits 127:96 of pattern */

#define GP_RASTER_MODE		0x8200		/* raster operation */
#define GP_VECTOR_MODE		0x8204		/* vector mode register	*/
#define GP_BLIT_MODE		0x8208		/* blit mode register */
#define GP_BLIT_STATUS		0x820C		/* blit status register	*/

/* "GP_VECTOR_MODE" BIT DEFINITIONS */

#define VM_X_MAJOR			0x0000		/* X major vector */
#define VM_Y_MAJOR			0x0001		/* Y major vector */
#define VM_MAJOR_INC		0x0002		/* positive major axis step */
#define VM_MINOR_INC		0x0004		/* positive minor axis step */
#define VM_READ_DST_FB		0x0008		/* read destination data */

/* "GP_RASTER_MODE" BIT DEFINITIONS */

#define RM_PAT_DISABLE		0x0000		/* pattern is disabled */
#define RM_PAT_MONO			0x0100		/* 1BPP pattern expansion */
#define RM_PAT_DITHER		0x0200		/* 2BPP pattern expansion */
#define RM_PAT_COLOR		0x0300		/* 8BPP or 16BPP pattern */
#define RM_PAT_MASK			0x0300		/* mask for pattern mode */
#define RM_PAT_TRANSPARENT	0x0400		/* transparent 1BPP pattern */
#define RM_SRC_TRANSPARENT	0x0800		/* transparent 1BPP source */

/* "GP_BLIT_STATUS" BIT DEFINITIONS */

#define BS_BLIT_BUSY		0x0001		/* blit engine is busy */
#define BS_PIPELINE_BUSY	0x0002		/* graphics pipeline is bus */
#define BS_BLIT_PENDING		0x0004		/* blit pending	*/
#define BC_8BPP				0x0000		/* 8BPP mode */
#define BC_16BPP			0x0100		/* 16BPP mode */
#define BC_FB_WIDTH_1024	0x0000		/* framebuffer width = 1024 */
#define BC_FB_WIDTH_2048	0x0200		/* framebuffer width = 2048 */

/* "GP_BLIT_MODE" BIT DEFINITIONS */

#define	BM_READ_SRC_NONE	0x0000		/* source foreground color */
#define BM_READ_SRC_FB		0x0001		/* read source from FB 	*/
#define BM_READ_SRC_BB0		0x0002		/* read source from BB0 */
#define BM_READ_SRC_BB1		0x0003		/* read source from BB1	*/
#define BM_READ_SRC_MASK	0x0003		/* read source mask */

#define	BM_READ_DST_NONE	0x0000		/* no destination data */
#define BM_READ_DST_BB0		0x0008		/* destination from BB0	*/
#define BM_READ_DST_BB1		0x000C		/* destination from BB1	*/
#define BM_READ_DST_FB0		0x0010		/* dest from FB (store BB0) */
#define BM_READ_DST_FB1		0x0014		/* dest from FB (store BB1)*/
#define BM_READ_DST_MASK	0x001C		/* read destination mask */

#define BM_WRITE_FB			0x0000		/* write to framebuffer	*/
#define	BM_WRITE_MEM		0x0020		/* write to memory */
#define BM_WRITE_MASK		0x0020		/* write mask */

#define	BM_SOURCE_COLOR		0x0000		/* source is 8BPP or 16BPP */
#define BM_SOURCE_EXPAND	0x0040		/* source is 1BPP */
#define BM_SOURCE_TEXT		0x00C0		/* source is 1BPP text */
#define BM_SOURCE_MASK		0x00C0		/* source mask */

#define BM_REVERSE_Y		0x0100		/* reverse Y direction */

/* THE DRIVER NEEDS TO MAINTAIN THE SIZE AND LOCATION OF THE BLT BUFFERS
// These constants will work with 2K or 3K config, 8 or 16 BPP.  The driver
// should set them, however, to optimize for the current config.  Otherwise
// screen to screen BLTs, for example, may be visibly split into two vertical
// sections when they do not need to be.
*/

/* STATIC VARIABLES FOR GXRENDER.C FILE */

unsigned char *GXRregptr;
unsigned short GXRbpp;
unsigned short GXRbb0Base;
unsigned short GXRbb1Base;
unsigned short GXRbufferWidthPixels;

unsigned short GXRpatternFlags;
unsigned short GXRsourceFlags;
unsigned short GXRsavedColor;
unsigned short GXRsavedRop;
unsigned short GXRusesDstData;

/* MACROS FOR REGISTER ACCESS 
// These macros asssume that a pointer was specified during initialization.
// They also assume 32-bit access is possible (16-bit access such as for 
// Windows 98 display drivers would require different macros).
*/
 
#define WRITE_REG8(offset, value) \
	(*(volatile unsigned char *)(GXRregptr + (offset))) = (value)

#define WRITE_REG16(offset, value) \
	(*(volatile unsigned short *)(GXRregptr + (offset))) = (value)

#define WRITE_REG32(offset, value) \
	(*(volatile unsigned long *)(GXRregptr + (offset))) = (value)

#define READ_REG16(offset) \
    (*(volatile unsigned short *)(GXRregptr + (offset)))

#define READ_REG32(offset) \
    (*(volatile unsigned long *)(GXRregptr + (offset)))

/*
//---------------------------------------------------------------------------
// GXR_INITIALIZE
//
// This routine initializes the parameters for the current configuration.
//
//     REGPTR     pointer to GX memory mapped registers
//     BPP        bits per pixel (8 pr 16)
//     
//---------------------------------------------------------------------------
*/

void gxr_initialize(unsigned char *regptr, unsigned short bpp, 
	unsigned short BB0base, unsigned short BB1base, 
	unsigned short BBwidthPixels)
{
	GXRregptr = regptr;
	GXRbpp = bpp;
	GXRbb0Base = BB0base;
	GXRbb1Base = BB1base;
	GXRbufferWidthPixels = BBwidthPixels;
}

/*
//---------------------------------------------------------------------------
// GXR_WAIT_UNTIL_IDLE
//
// This routine waits until the graphics engine is idle.
//---------------------------------------------------------------------------
*/

void gxr_wait_until_idle(void)
{
	while(READ_REG16(GP_BLIT_STATUS) & BS_BLIT_BUSY);
}

/*
//---------------------------------------------------------------------------
// GXR_LOAD_SOLID_SOURCE
//
// This routine is used to specify a solid source color.  For the Xfree96
// display driver, the source color is used to specify a planemask and the 
// ROP is adjusted accordingly.
//---------------------------------------------------------------------------
*/

void gxr_load_solid_source(unsigned short color)
{
	/* CLEAR TRANSPARENCY FLAG */

	GXRsourceFlags = 0;

	/* FORMAT 8 BPP COLOR */
	/* GX requires 8BPP color data be duplicated into bits [15:8]. */

	if (!(READ_REG16(GP_BLIT_STATUS) & BC_16BPP))
	{
		color &= 0x00FF;
		color |= (color << 8);
	}
	
	/* POLL UNTIL ABLE TO WRITE THE SOURCE COLOR */

	while(READ_REG16(GP_BLIT_STATUS) & BS_BLIT_PENDING);
 	WRITE_REG16(GP_SRC_COLOR_0, color);
}

/*
//---------------------------------------------------------------------------
// GXR_LOAD_MONO_SOURCE
//
// This routine is used to specify the monochrome source colors.  
//---------------------------------------------------------------------------
*/

void gxr_load_mono_source(unsigned short bgcolor, unsigned short fgcolor,
	unsigned short transparent)
{
	/* SET TRANSPARENCY FLAG */

	GXRsourceFlags = transparent ? RM_SRC_TRANSPARENT : 0;

	/* FORMAT 8 BPP COLOR */
	/* GX requires 8BPP color data be duplicated into bits [15:8]. */

	if (!(READ_REG16(GP_BLIT_STATUS) & BC_16BPP))
	{
		bgcolor &= 0x00FF;
		bgcolor |= (bgcolor << 8);
		fgcolor &= 0x00FF;
		fgcolor |= (fgcolor << 8);
	}

	/* POLL UNTIL ABLE TO WRITE THE SOURCE COLOR */

	while(READ_REG16(GP_BLIT_STATUS) & BS_BLIT_PENDING);
 	WRITE_REG16(GP_SRC_COLOR_0, bgcolor);
 	WRITE_REG16(GP_SRC_COLOR_1, fgcolor);
}

/*
//---------------------------------------------------------------------------
// GXR_LOAD_SOLID_PATTERN
//
// This routine is used to specify a solid pattern color.  It is called 
// before performing solid rectangle fills or more complicated BLTs that 
// use a solid pattern color. 
//
// The driver should always call "gxr_load_raster_operation" after a call 
// to this routine to make sure that the pattern flags are set appropriately.
//---------------------------------------------------------------------------
*/

void gxr_load_solid_pattern(unsigned short color)
{
	/* SET PATTERN FLAGS */

	GXRpatternFlags = 0;

	/* FORMAT 8 BPP COLOR */
	/* GX requires 8BPP color data be duplicated into bits [15:8]. */

	if (!(READ_REG16(GP_BLIT_STATUS) & BC_16BPP))
	{
		color &= 0x00FF;
		color |= (color << 8);
	}

	/* SAVE THE REFORMATTED COLOR FOR LATER */
	/* Used to call the "gxr_solid_fill" routine for special cases. */

	GXRsavedColor = color;

	/* POLL UNTIL ABLE TO WRITE THE PATTERN COLOR */

	while(READ_REG16(GP_BLIT_STATUS) & BS_BLIT_PENDING);
 	WRITE_REG16(GP_PAT_COLOR_0, color);
}

/*
//---------------------------------------------------------------------------
// GXR_LOAD_MONO_PATTERN
//
// This routine is used to specify a monochrome pattern. 
//---------------------------------------------------------------------------
*/

void gxr_load_mono_pattern(unsigned short bgcolor, unsigned short fgcolor, 
	unsigned long data0, unsigned long data1, unsigned char transparent)
{
	/* SET PATTERN FLAGS */

	GXRpatternFlags = transparent ? RM_PAT_MONO | RM_PAT_TRANSPARENT : 
		RM_PAT_MONO;
	
	/* FORMAT 8 BPP COLOR */
	/* GX requires 8BPP color data be duplicated into bits [15:8]. */

	if (!(READ_REG16(GP_BLIT_STATUS) & BC_16BPP))
	{
		bgcolor &= 0x00FF;
		bgcolor |= (bgcolor << 8);
		fgcolor &= 0x00FF;
		fgcolor |= (fgcolor << 8);
	}

	/* POLL UNTIL ABLE TO WRITE THE PATTERN COLORS AND DATA */

	while(READ_REG16(GP_BLIT_STATUS) & BS_BLIT_PENDING);
 	WRITE_REG16(GP_PAT_COLOR_0, bgcolor);
 	WRITE_REG16(GP_PAT_COLOR_1, fgcolor);
 	WRITE_REG32(GP_PAT_DATA_0, data0);
 	WRITE_REG32(GP_PAT_DATA_1, data1);
}

/*
//---------------------------------------------------------------------------
// GXR_LOAD_RASTER_OPERATION
//
// This routine loads the specified raster operation.  It sets the pattern
// flags appropriately.
//---------------------------------------------------------------------------
*/

void gxr_load_raster_operation(unsigned char rop)
{
	unsigned short rop16;

	/* GENERATE 16-BIT VERSION OF ROP WITH PATTERN FLAGS */

	rop16 = (unsigned short) rop | GXRpatternFlags;
	if ((rop & 0x33) ^ ((rop >> 2) & 0x33))
		rop16 |= GXRsourceFlags;

	/* SAVE ROP FOR LATER COMPARISONS */
	/* Need to have the pattern flags included */

	GXRsavedRop = rop16;
	
	/* SET FLAG INDICATING ROP REQUIRES DESTINATION DATA */
	/* True if even bits (0:2:4:6) do not equal the correspinding */
	/* even bits (1:3:5:7). */

	GXRusesDstData = ((rop & 0x55) ^ ((rop >> 1) & 0x55));

	/* POLL UNTIL ABLE TO WRITE THE PATTERN COLOR */
	/* Only one operation can be pending at a time. */ 

	while(READ_REG16(GP_BLIT_STATUS) & BS_BLIT_PENDING);
 	WRITE_REG16(GP_RASTER_MODE, rop16);
}

/*
//---------------------------------------------------------------------------
// GXR_SOLID_FILL
//
// This routine MUST be used when performing a solid rectangle fill with 
// the ROPs of PATCOPY (0xF0), BLACKNESS (0x00), WHITENESS (0xFF), or 
// PATINVERT (0x0F).  There is a bug in GXm for these cases that requires a 
// workaround.  
//
// For BLACKNESS (ROP = 0x00), set the color to 0x0000.  
// For WHITENESS (ROP = 0xFF), set the color to 0xFFFF.
// For PATINVERT (ROP = 0x0F), invert the desired color.
//
//      X               screen X position (left)
//      Y               screen Y position (top)
//      WIDTH           width of rectangle, in pixels
//      HEIGHT          height of rectangle, in scanlines
//      COLOR           fill color
//
// THIS ROUTINE SHOULD NOT BE DIRECTLY CALLED FROM THE DRIVER.  The driver 
// should always use gxr_pattern_fill and let that routine call this one
// when approipriate.  This is to hide quirks specific to MediaGX hardware.
//---------------------------------------------------------------------------
*/

void gxr_solid_fill(unsigned short x, unsigned short y, 
	unsigned short width, unsigned short height, unsigned short color)
{
	unsigned short section;

	/* POLL UNTIL ABLE TO WRITE TO THE REGISTERS */
	/* Only one operation can be pending at a time. */ 

	while(READ_REG16(GP_BLIT_STATUS) & BS_BLIT_PENDING);

	/* SET REGISTERS TO DRAW RECTANGLE */

	WRITE_REG16(GP_DST_XCOOR, x);
	WRITE_REG16(GP_DST_YCOOR, y);
 	WRITE_REG16(GP_HEIGHT, height);
 	WRITE_REG16(GP_RASTER_MODE, 0x00F0); /* PATCOPY */
	WRITE_REG16(GP_PAT_COLOR_0, color);

	/* CHECK WIDTH FOR GX BUG WORKAROUND */ 

	if (width <= 16)
	{
		/* OK TO DRAW SMALL RECTANGLE IN ONE PASS */

		WRITE_REG16(GP_WIDTH, width);
	 	WRITE_REG16(GP_BLIT_MODE, 0);
	}
	else
	{
		/* DRAW FIRST PART OF RECTANGLE */
		/* Get to a 16 pixel boundary. */

		section = 0x10 - (x & 0x0F);
		WRITE_REG16(GP_WIDTH, section);
	 	WRITE_REG16(GP_BLIT_MODE, 0);

		/* POLL UNTIL ABLE TO LOAD THE SECOND RECTANGLE */

		while(READ_REG16(GP_BLIT_STATUS) & BS_BLIT_PENDING);
		WRITE_REG16(GP_DST_XCOOR, x + section);
		WRITE_REG16(GP_DST_YCOOR, y);
		WRITE_REG16(GP_WIDTH, width - section);
	 	WRITE_REG16(GP_BLIT_MODE, 0);
	}
}    

/*
//----------------------------------------------------------------------------
// GXR_PATTERN_FILL
//
// This routine is used to fill a rectangular region.  The pattern must 
// be previously loaded using one of gxr_load_*_pattern routines.  Also, the 
// raster operation must be previously specified using the 
// "gxr_load_raster_operation" routine.
//
//      X               screen X position (left)
//      Y               screen Y position (top)
//      WIDTH           width of rectangle, in pixels
//      HEIGHT          height of rectangle, in scanlines
//----------------------------------------------------------------------------
*/

void gxr_pattern_fill(unsigned short x, unsigned short y, 
	unsigned short width, unsigned short height)
{
	unsigned short section, buffer_width, blit_mode;

	/* CHECK IF OPTIMIZED SOLID CASES */
    /* Check all 16 bits of the ROP to include solid pattern flags. */

	switch(GXRsavedRop)
	{
		/* CHECK FOR SPECIAL CASES WITHOUT DESTINATION DATA */
		/* Need hardware workaround for fast "burst write" cases. */

		case 0x00F0:
			gxr_solid_fill(x, y, width, height, GXRsavedColor);
			break;
		case 0x000F:
			gxr_solid_fill(x, y, width, height, ~GXRsavedColor);
			break;
		case 0x0000:
			gxr_solid_fill(x, y, width, height, 0x0000);
			break;
		case 0x00FF:
			gxr_solid_fill(x, y, width, height, 0xFFFF);
			break;
		
		/* REMAINING CASES REQUIRE DESTINATION DATA OR NOT SOLID COLOR */

		default:
			
			/* DETERMINE BLT MODE VALUE */
			/* Still here for non-solid patterns without destination data. */

			blit_mode = GXRusesDstData ? BM_READ_DST_FB0 : 0;

			/* POLL UNTIL ABLE TO WRITE TO THE REGISTERS */
			/* Write the registers that do not change for each section. */

			while(READ_REG16(GP_BLIT_STATUS) & BS_BLIT_PENDING);
 			WRITE_REG16(GP_HEIGHT, height);

			/* SINCE ONLY DESTINATION DATA, WE CAN USE BOTH BB0 AND BB1. */
			/* Therefore, width available = BLT buffer width * 2. */

			buffer_width = GXRbufferWidthPixels << 1;

			/* REPEAT UNTIL FINISHED WITH RECTANGLE */
			/* Perform BLT in vertical sections, as wide as the BLT buffer */
			/* allows.  Hardware does not split the operations, so */
			/* software must do it to avoid large scanlines that would */
			/* overflow the BLT buffers. */

			while(width > 0)
			{
				/* DETERMINE WIDTH OF SECTION */

				if (width > buffer_width) section = buffer_width;
				else section = width;

				/* POLL UNTIL ABLE TO WRITE TO THE REGISTERS */

				while(READ_REG16(GP_BLIT_STATUS) & BS_BLIT_PENDING);
				WRITE_REG16(GP_DST_XCOOR, x);
				WRITE_REG16(GP_DST_YCOOR, y);
				WRITE_REG16(GP_WIDTH, section);
				WRITE_REG16(GP_BLIT_MODE, blit_mode);

				/* ADJUST PARAMETERS FOR NEXT SECTION */

				width -= section;
				x += section;
			}
			break;
	}
}    

/*
//----------------------------------------------------------------------------
// SCREEN TO SCREEN BLT
//
// This routine should be used to perform a screen to screen BLT when the 
// ROP does not require destination data.
//
//      SRCX            screen X position to copy from
//      SRCY            screen Y position to copy from
//      DSTX            screen X position to copy to
//      DSTY            screen Y position to copy to
//      WIDTH           width of rectangle, in pixels
//      HEIGHT          height of rectangle, in scanlines
//----------------------------------------------------------------------------
*/

void gxr_screen_to_screen_blt(unsigned short srcx, unsigned short srcy,
	unsigned short dstx, unsigned short dsty, unsigned short width, 
	unsigned short height)
{
	unsigned short section, buffer_width;
	unsigned short blit_mode;

	/* CHECK IF RASTER OPERATION REQUIRES DESTINATION DATA */

	blit_mode = GXRusesDstData ? BM_READ_DST_FB1 | BM_READ_SRC_FB : 
		BM_READ_SRC_FB;

	/* CHECK Y DIRECTION */
	/* Hardware has support for negative Y direction. */

	if (dsty > srcy) 
	{
		blit_mode |= BM_REVERSE_Y;
		srcy += height - 1;
		dsty += height - 1;
	}

	/* CHECK X DIRECTION */
	/* Hardware does not support negative X direction since at the time */ 
	/* of development all supported resolutions could fit a scanline of */ 
	/* data at once into the BLT buffers (using both BB0 and BB1).  This */
	/* code is more generic to allow for any size BLT buffer. */

	if (dstx > srcx)
	{
		srcx += width;
		dstx += width;
	}

	/* POLL UNTIL ABLE TO WRITE TO THE REGISTERS */
	/* Write the registers that do not change for each section. */

	while(READ_REG16(GP_BLIT_STATUS) & BS_BLIT_PENDING);
 	WRITE_REG16(GP_HEIGHT, height);

	/* CHECK AVAILABLE BLT BUFFER SIZE */
	/* Can use both BLT buffers if no destination data is required. */

	buffer_width = GXRusesDstData ? GXRbufferWidthPixels :
		GXRbufferWidthPixels << 1;

	/* REPEAT UNTIL FINISHED WITH RECTANGLE */
	/* Perform BLT in vertical sections, as wide as the BLT buffer allows. */
	/* Hardware does not split the operations, so software must do it to */ 
	/* avoid large scanlines that would overflow the BLT buffers. */

	while(width > 0)
	{
		/* CHECK WIDTH OF CURRENT SECTION */

		if (width > buffer_width) section = buffer_width;
		else section = width;

		/* PROGRAM REGISTERS THAT ARE THE SAME FOR EITHER X DIRECTION */

		while(READ_REG16(GP_BLIT_STATUS) & BS_BLIT_PENDING);
		WRITE_REG16(GP_SRC_YCOOR, srcy);
		WRITE_REG16(GP_DST_YCOOR, dsty);
		WRITE_REG16(GP_WIDTH, section);

		/* CHECK X DIRECTION */

		if (dstx > srcx)
		{
			/* NEGATIVE X DIRECTION */
			/* Still positive X direction within the section. */

			srcx -= section;
			dstx -= section;
			WRITE_REG16(GP_SRC_XCOOR, srcx);
			WRITE_REG16(GP_DST_XCOOR, dstx);
			WRITE_REG16(GP_BLIT_MODE, blit_mode);
		} 
		else
		{
			/* POSITIVE X DIRECTION */

			WRITE_REG16(GP_SRC_XCOOR, srcx);
			WRITE_REG16(GP_DST_XCOOR, dstx);
			WRITE_REG16(GP_BLIT_MODE, blit_mode);
			dstx += section;
			srcx += section;
		}
		width -= section;
	}
}    

/*
//----------------------------------------------------------------------------
// SCREEN TO SCREEN TRANSPARENT BLT
//
// This routine should be used to perform a screen to screen BLT when a 
// specified color should by transparent.  The only supported ROP is SRCCOPY.
//
//      SRCX            screen X position to copy from
//      SRCY            screen Y position to copy from
//      DSTX            screen X position to copy to
//      DSTY            screen Y position to copy to
//      WIDTH           width of rectangle, in pixels
//      HEIGHT          height of rectangle, in scanlines
//      COLOR           transparent color
//----------------------------------------------------------------------------
*/

void gxr_screen_to_screen_xblt(unsigned short srcx, unsigned short srcy,
	unsigned short dstx, unsigned short dsty, unsigned short width, 
	unsigned short height, unsigned short color)
{
	unsigned short section, buffer_width;
	unsigned short blit_mode = BM_READ_SRC_FB;
	unsigned short i;

	/* CHECK Y DIRECTION */
	/* Hardware has support for negative Y direction. */

	if (dsty > srcy) 
	{
		blit_mode |= BM_REVERSE_Y;
		srcy += height - 1;
		dsty += height - 1;
	}

	/* CHECK X DIRECTION */
	/* Hardware does not support negative X direction since at the time */
	/* of development all supported resolutions could fit a scanline of */ 
	/* data at once into the BLT buffers (using both BB0 and BB1).  This */
	/* code is more generic to allow for any size BLT buffer. */

	if (dstx > srcx)
	{
		srcx += width;
		dstx += width;
	}

	/* CALCULATE BLT BUFFER SIZE */
	/* Need to use BB1 to store the BLT buffer data. */

	buffer_width = GXRbufferWidthPixels;

	/* WRITE TRANSPARENCY COLOR TO BLT BUFFER 1 */

	if (!(READ_REG16(GP_BLIT_STATUS) & BC_16BPP))
	{
		color &= 0x00FF;
		color |= (color << 8);
	}

	/* WAIT UNTIL PIPELINE IS NOT BUSY BEFORE LOADING DATA INTO BB1 */
	/* Need to make sure any previous BLT using BB1 is complete. */

	while(READ_REG16(GP_BLIT_STATUS) & BS_PIPELINE_BUSY);
	for (i = 0; i < 16; i+=2)
	{
		WRITE_REG16(GXRbb1Base+i, color);
	}

	/* DO BOGUS BLT TO LATCH DATA FROM BB1 */
	/* Already know graphics pipeline is idle. */

 	WRITE_REG32(GP_DST_XCOOR, 0);
 	WRITE_REG32(GP_SRC_XCOOR, 0);
 	WRITE_REG16(GP_WIDTH, 16);
 	WRITE_REG16(GP_HEIGHT, 1);
 	WRITE_REG16(GP_RASTER_MODE, 0x00CC);
 	WRITE_REG16(GP_BLIT_MODE, BM_READ_SRC_FB | BM_READ_DST_BB1);
	
	/* WRITE REGISTERS FOR REAL SCREEN TO SCREEN BLT */

	while(READ_REG16(GP_BLIT_STATUS) & BS_BLIT_PENDING);
	WRITE_REG16(GP_HEIGHT, height);
 	WRITE_REG16(GP_RASTER_MODE, 0x10C6);
	WRITE_REG32(GP_PAT_COLOR_0, 0xFFFFFFFF);

	/* REPEAT UNTIL FINISHED WITH RECTANGLE */
	/* Perform BLT in vertical sections, as wide as the BLT buffer allows. */
	/* Hardware does not split the operations, so software must do it to */
	/* avoid large scanlines that would overflow the BLT buffers. */

	while(width > 0)
	{
		/* CHECK WIDTH OF CURRENT SECTION */

		if (width > buffer_width) section = buffer_width;
		else section = width;

		/* PROGRAM REGISTERS THAT ARE THE SAME FOR EITHER X DIRECTION */

		while(READ_REG16(GP_BLIT_STATUS) & BS_BLIT_PENDING);
		WRITE_REG16(GP_SRC_YCOOR, srcy);
		WRITE_REG16(GP_DST_YCOOR, dsty);
		WRITE_REG16(GP_WIDTH, section);

		/* CHECK X DIRECTION */
		/* Again, this must be done in software, and can be removed if the */
		/* display driver knows that the BLT buffers will always be large */
		/* enough to contain an entire scanline of a screen to screen BLT. */

		if (dstx > srcx)
		{
			/* NEGATIVE X DIRECTION */
			/* Still positive X direction within the section. */

			srcx -= section;
			dstx -= section;
			WRITE_REG16(GP_SRC_XCOOR, srcx);
			WRITE_REG16(GP_DST_XCOOR, dstx);
			WRITE_REG16(GP_BLIT_MODE, blit_mode);
		} 
		else
		{
			/* POSITIVE X DIRECTION */

			WRITE_REG16(GP_SRC_XCOOR, srcx);
			WRITE_REG16(GP_DST_XCOOR, dstx);
			WRITE_REG16(GP_BLIT_MODE, blit_mode);
			dstx += section;
			srcx += section;
		}
		width -= section;
	}
}    

/*
//----------------------------------------------------------------------------
// COLOR BITMAP TO SCREEN BLT
//
// This routine transfers color bitmap data to the screen.  For most cases,
// when the ROP is SRCCOPY, it may be faster to write a separate routine that
// copies the data to the frame buffer directly.  This routine should be 
// used when the ROP requires destination data.
//
// Transparency is handled by another routine.
//
//      SRCX            X offset within source bitmap
//      SRCY            Y offset within source bitmap
//      DSTX            screen X position to render data
//      DSTY            screen Y position to render data
//      WIDTH           width of rectangle, in pixels
//      HEIGHT          height of rectangle, in scanlines
//      *DATA           pointer to bitmap data
//      PITCH           pitch of bitmap data (bytes between scanlines)
//      ROP             ternary raster operation (0x00-0xFF).
//----------------------------------------------------------------------------
*/

void gxr_color_bitmap_to_screen_blt(unsigned short srcx, unsigned short srcy,
	unsigned short dstx, unsigned short dsty, unsigned short width, 
	unsigned short height, unsigned char *data, unsigned short pitch, 
	unsigned char rop)
{
	unsigned short section, buffer_width;
	unsigned short blit_mode = BM_READ_SRC_BB0;
	unsigned short line_srcx, line_dstx, line_width;
	unsigned short offset, i;

	/* CHECK IF ROP REQUIRES DESTINATION DATA */
	/* Even bits (0:2:4:6) do not equal corresponding odd bits (1:3:5:7). */

	if ((rop & 0x55) ^ ((rop >> 1) & 0x55))
		blit_mode |= BM_READ_DST_FB1;

	/* CHECK SIZE OF BLT BUFFER */

	buffer_width = GXRbufferWidthPixels;
	
	/* POLL UNTIL ABLE TO WRITE TO THE REGISTERS */
	/* Write the registers that do not change for each section. */

	while(READ_REG16(GP_BLIT_STATUS) & BS_BLIT_PENDING);
 	WRITE_REG16(GP_HEIGHT, 1);
 	WRITE_REG16(GP_RASTER_MODE, (unsigned short) rop & 0x00FF);

	/* REPEAT FOR EACH SCANLINE */

	offset = srcy * pitch;

	while(height > 0)
	{
		line_width = width;
		line_srcx = srcx;
		line_dstx = dstx;

		while(line_width > 0)
		{
			/* CHECK WIDTH OF CURRENT SECTION */

			if (line_width > buffer_width) section = buffer_width;
			else section = line_width;

			/* TRANSFER SCANLINE OF BITMAP DATA TO BLT BUFFER 0 */
			/* Need to wait for BS_PIPELINE_BUSY to make sure that the */ 
			/* data in BB0 for the previous scanline is no longer used. */
			/* This can be heavily optimized to not do a byte at a time. */

			while(READ_REG16(GP_BLIT_STATUS) & BS_PIPELINE_BUSY);
			if (READ_REG16(GP_BLIT_STATUS) & BC_16BPP)
			{
				for (i = 0; i < section; i++)
				{
					WRITE_REG16(GXRbb0Base+i, data[offset+((line_srcx+i)<<1)]);
				}
			}
			else
			{
				for (i = 0; i < section; i++)
				{
					WRITE_REG8(GXRbb0Base+i, data[line_srcx+offset+i]);
				}
			}

			/* RENDER FROM BB0 TO FRAME BUFFER */

			while(READ_REG16(GP_BLIT_STATUS) & BS_BLIT_PENDING);
			WRITE_REG16(GP_DST_XCOOR, line_dstx);
			WRITE_REG16(GP_DST_YCOOR, dsty);
			WRITE_REG16(GP_WIDTH, section);
			WRITE_REG16(GP_BLIT_MODE, blit_mode);

			line_width -= section;
			line_dstx += section;
			line_srcx += section;
		}
		height--;
		dsty++;
		offset += pitch;
	}
}    

/*
//----------------------------------------------------------------------------
// COLOR BITMAP TO SCREEN TRANSPARENT BLT
//
// This routine transfers color bitmap data to the screen with transparency.
// The transparent color is specified.  The only supported ROP is SRCCOPY, 
// meaning that transparency cannot be applied if the ROP requires 
// destination data (this is a hardware restriction).
//
//      SRCX            X offset within source bitmap
//      SRCY            Y offset within source bitmap
//      DSTX            screen X position to render data
//      DSTY            screen Y position to render data
//      WIDTH           width of rectangle, in pixels
//      HEIGHT          height of rectangle, in scanlines
//      *DATA           pointer to bitmap data
//      PITCH           pitch of bitmap data (bytes between scanlines)
//      COLOR           transparent color
//----------------------------------------------------------------------------
*/

void gxr_color_bitmap_to_screen_xblt(unsigned short srcx, unsigned short srcy,
	unsigned short dstx, unsigned short dsty, unsigned short width, 
	unsigned short height, unsigned char *data, unsigned short pitch, 
	unsigned short color)
{
	unsigned short section, buffer_width;
	unsigned short blit_mode = BM_READ_SRC_BB0;
	unsigned short line_srcx, line_dstx, line_width;
	unsigned short offset, i, first_blt = 1;

	/* CHECK SIZE OF BLT BUFFER */

	buffer_width = GXRbufferWidthPixels;
	
	/* POLL UNTIL ABLE TO WRITE TO THE REGISTERS */
	/* Write the registers that do not change for each section. */

	while(READ_REG16(GP_BLIT_STATUS) & BS_BLIT_PENDING);
 	WRITE_REG16(GP_HEIGHT, 1);
 	WRITE_REG16(GP_RASTER_MODE, 0x10C6);
	WRITE_REG32(GP_PAT_COLOR_0, 0xFFFFFFFF);

	/* CALCULATE OFFSET INTO BITMAP DATA */

	offset = srcy * pitch;

	/* REPEAT FOR EACH SCANLINE */

	while(height > 0)
	{
		line_width = width;
		line_srcx = srcx;
		line_dstx = dstx;

		while(line_width > 0)
		{
			/* CHECK WIDTH OF CURRENT SECTION */

			if (line_width > buffer_width) section = buffer_width;
			else section = line_width;

			/* TRANSFER SCANLINE OF BITMAP DATA TO BLT BUFFER 0 */
			/* Need to wait for BS_PIPELINE_BUSY to make sure that the */ 
			/* data in BB0 for the previous scanline is no longer used. */
			/* This can be heavily optimized to not do a byte at a time. */

			while(READ_REG16(GP_BLIT_STATUS) & BS_PIPELINE_BUSY);
			if (READ_REG16(GP_BLIT_STATUS) & BC_16BPP)
			{
				for (i = 0; i < section; i++)
				{
					WRITE_REG16(GXRbb0Base+i, data[offset+((line_srcx+i)<<1)]);
				}
			}
			else
			{
				for (i = 0; i < section; i++)
				{
					WRITE_REG8(GXRbb0Base+i, data[line_srcx+offset+i]);
				}
			}

			/* RENDER FROM BB0 TO FRAME BUFFER */

			while(READ_REG16(GP_BLIT_STATUS) & BS_BLIT_PENDING);
			WRITE_REG16(GP_DST_XCOOR, line_dstx);
			WRITE_REG16(GP_DST_YCOOR, dsty);
			WRITE_REG16(GP_WIDTH, section);

			/* NEED TO DO EXTRA WORK FOR THE FIRST BLT */

			if (first_blt)
			{
				/* WRITE TRANSPARENCY COLOR TO BLT BUFFER 1 */
				/* This can be heavily optimized to not do 16-bits at a time. */

				if (READ_REG16(GP_BLIT_STATUS) & BC_16BPP)
				{
					for (i = 0; i < section; i++)
					{
						WRITE_REG16(GXRbb1Base+i*2, color);
					}
				}
				else
				{
					for (i = 0; i < section; i++)
					{
						WRITE_REG8(GXRbb1Base+i, (unsigned char) color);
					}
				}
				WRITE_REG16(GP_BLIT_MODE, BM_READ_SRC_BB0 | BM_READ_DST_BB1);
				first_blt = 0;
			}
			else
			{
				/* AFTER FIRST BLT, THE TRANSPARENCY DATA IS LATCHED */
				/* Save time by not reading data from BB1. */

				WRITE_REG16(GP_BLIT_MODE, BM_READ_SRC_BB0);
			}
			line_width -= section;
			line_dstx += section;
			line_srcx += section;
		}
		height--;
		dsty++;
		offset += pitch;
	}
}    

/*
//----------------------------------------------------------------------------
// MONOCHROME BITMAP TO SCREEN BLT
//
// This routine transfers monochrome bitmap data to the screen.  
//
//      SRCX            X offset within source bitmap
//      SRCY            Y offset within source bitmap
//      DSTX            screen X position to render data
//      DSTY            screen Y position to render data
//      WIDTH           width of rectangle, in pixels
//      HEIGHT          height of rectangle, in scanlines
//      *DATA           pointer to bitmap data
//      PITCH           pitch of bitmap data (bytes between scanlines)
//      FGCOLOR         color for bits = 1
//      BGCOLOR         color for bits = 0
//      ROP             ternary raster operation (0x00-0xFF).
//      TRANSPARENT     zero for opaque, otherwise transparent
//----------------------------------------------------------------------------
*/

void gxr_mono_bitmap_to_screen_blt(unsigned short srcx, unsigned short srcy,
	unsigned short dstx, unsigned short dsty, unsigned short width, 
	unsigned short height, unsigned char *data, unsigned short pitch, 
	unsigned short fgcolor, unsigned short bgcolor, unsigned char rop,
	unsigned char transparent)
{
	unsigned short section, buffer_width;
	unsigned short blit_mode = BM_READ_SRC_BB0 | BM_SOURCE_EXPAND;
	unsigned short line_srcx, line_dstx, line_width;
	unsigned short offset, i, bytes, raster_mode;

	/* FORMAT 8BPP COLOR */
    /* GX requires that 8BPP color data be duplicated into bits [15:8]. */

	if (!(READ_REG16(GP_BLIT_STATUS) & BC_16BPP))
	{
		fgcolor &= 0x00FF;
		fgcolor |= (fgcolor << 8);
		bgcolor &= 0x00FF;
		bgcolor |= (bgcolor << 8);
	}

	/* CHECK IF ROP REQUIRES DESTINATION DATA */
	/* Even bits (0:2:4:6) do not equal corresponding odd bits (1:3:5:7). */

	if ((rop & 0x55) ^ ((rop >> 1) & 0x55))
		blit_mode |= BM_READ_DST_FB1;

	/* CALCULATE RASTER MODE */
	/* Set mono flag.  Transparency	set based on input parameter. */

	raster_mode = ((unsigned short) rop & 0x00FF);
	if (transparent) raster_mode |= RM_SRC_TRANSPARENT;

	/* CHECK SIZE OF BLT BUFFER */

	buffer_width = GXRbufferWidthPixels;
	
	/* CALCULATE OFFSET INTO BITMAP DATA */

	offset = srcy * pitch;

	/* POLL UNTIL ABLE TO WRITE TO THE REGISTERS */
	/* Write the registers that do not change for each section. */

	while(READ_REG16(GP_BLIT_STATUS) & BS_BLIT_PENDING);
 	WRITE_REG16(GP_HEIGHT, 1);
 	WRITE_REG16(GP_RASTER_MODE, raster_mode);
 	WRITE_REG16(GP_SRC_COLOR_0, bgcolor);
 	WRITE_REG16(GP_SRC_COLOR_1, fgcolor);

	/* REPEAT FOR EACH SCANLINE */

	while(height > 0)
	{
		line_width = width;
		line_srcx = srcx;
		line_dstx = dstx;

		while(line_width > 0)
		{
			/* CHECK WIDTH OF CURRENT SECTION */
			/* Only divide into sections if reading destination data. */  
			/* Since the source data is monochrome, it will always fit. */

			section = line_width;
			if ((line_width > buffer_width) && (blit_mode & BM_READ_DST_FB1)) 
				section = buffer_width;

			/* BYTES TO TRANSFER */
			/* Add two bytes to handle truncating and alignment. */

			bytes = (section >> 3) + 2;

			/* TRANSFER SCANLINE OF BITMAP DATA TO BLT BUFFER 0 */
			/* Need to wait for BS_PIPELINE_BUSY to make sure that the */ 
			/* data in BB0 for the previous scanline is no longer used. */
			/* This can be heavily optimized to not do a byte at a time. */

			while(READ_REG16(GP_BLIT_STATUS) & BS_PIPELINE_BUSY);
			for (i = 0; i < bytes; i++)
			{
				WRITE_REG8(GXRbb0Base+i, data[(line_srcx >> 3)+offset+i]);
			}

			/* RENDER FROM BB0 TO FRAME BUFFER */

			while(READ_REG16(GP_BLIT_STATUS) & BS_BLIT_PENDING);
			WRITE_REG16(GP_DST_XCOOR, line_dstx);
			WRITE_REG16(GP_DST_YCOOR, dsty);
		 	WRITE_REG16(GP_SRC_XCOOR, line_srcx & 7);
			WRITE_REG16(GP_WIDTH, section);
			WRITE_REG16(GP_BLIT_MODE, blit_mode);

			line_width -= section;
			line_dstx += section;
			line_srcx += section;
		}
		height--;
		dsty++;
		offset += pitch;
	}
}    

/*
//----------------------------------------------------------------------------
// TEXT GLYPH
//
// This routine draws a single character of text.  It can only be used for 
// characters that are less than or equal to 64x64 in size, since it 
// transfers the entire data into the BLT buffers at once.  Larger characters
// should use the monochrome bitmap to screen routine.  The only supported 
// ROP is SRCCOPY, again since the BLT buffer size is limited. 
// 
//      SRCX            X offset within source bitmap
//      SRCY            Y offset within source bitmap
//      DSTX            screen X position to render data
//      DSTY            screen Y position to render data
//      WIDTH           width of rectangle, in pixels
//      HEIGHT          height of rectangle, in scanlines
//      *DATA           pointer to bitmap data (NULL if already loaded)
//      PITCH           pitch of bitmap data (bytes between scanlines)
//
// For the Xfree86 display driver, the OS is given a pointer to BB0.  
// Therefore, the data is already loaded when the driver is called, so the
// driver simply passes a NULL pointer to this routine.
//
// This same type of routine could be developed for "icons", or small 
// color bitmaps that can fit entirely in the BLT buffer. 
//----------------------------------------------------------------------------
*/

void gxr_text_glyph(unsigned short srcx, unsigned short srcy,
	unsigned short dstx, unsigned short dsty, unsigned short width, 
	unsigned short height, unsigned char *data, unsigned short pitch)
{
	unsigned short offset, i, j, buffer_offset, bytes, blit_mode;

	blit_mode = BM_READ_SRC_BB0 | BM_SOURCE_EXPAND;
	if (GXRusesDstData) blit_mode |= BM_READ_DST_FB1;

	/* CHECK IF DATA NEEDS TO BE TRANSFERRED */

	if (data != 0)
	{
		/* TRANSFER ENTIRE BITMAP DATA TO BLT BUFFER 0 */
		/* Need to wait for BS_PIPELINE_BUSY to make sure that the */ 
		/* data in BB0 for any previous BLT is no longer used. */
		/* This data transfer has lots of room for performance optimization. */

		buffer_offset = 0;
		offset = srcy * pitch + (srcx >> 3);
		bytes = ((width + (srcx & 7) + 7) >> 3);
		while(READ_REG16(GP_BLIT_STATUS) & BS_PIPELINE_BUSY);
		for (j = 0; j < height; j++)
		{
			for (i = 0; i < bytes; i++)
			{
				WRITE_REG8(GXRbb0Base+buffer_offset, data[offset+i]);
				buffer_offset++;
			}
			offset += pitch;
		}
	}

	/* RENDER FROM BB0 TO FRAME BUFFER */
	/* Already know that the pipeline is idle from loading data. */

	WRITE_REG16(GP_DST_XCOOR, dstx);
	WRITE_REG16(GP_DST_YCOOR, dsty);
 	WRITE_REG16(GP_SRC_XCOOR, srcx & 7);
	WRITE_REG16(GP_WIDTH, width);
 	WRITE_REG16(GP_HEIGHT, height);
	WRITE_REG16(GP_BLIT_MODE, blit_mode);
}    

/*
//----------------------------------------------------------------------------
// BRESENHAM LINE
//
// This routine draws a vector using the specified Bresenham parameters.  
// Currently this file does not support a routine that accepts the two 
// endpoints of a vector and calculates the Bresenham parameters.  If it 
// ever does, this routine is still required for vectors that have been 
// clipped.
//
//      X               screen X position to start vector
//      Y               screen Y position to start vector
//      LENGTH          length of the vector, in pixels
//      INITERR         Bresenham initial error term
//      AXIALERR        Bresenham axial error term
//      DIAGERR         Bresenham diagonal error term
//      FLAGS           VM_YMAJOR, VM_MAJOR_INC, VM_MINOR_INC
//----------------------------------------------------------------------------
*/

void gxr_bresenham_line(unsigned short x, unsigned short y, 
		unsigned short length, unsigned short initerr, 
		unsigned short axialerr, unsigned short diagerr, 
		unsigned short flags)
{
	unsigned short vector_mode = flags;
	if (GXRusesDstData) vector_mode |= VM_READ_DST_FB;

	/* CHECK NULL LENGTH */

	if (!length) return;

	/* LOAD THE REGISTERS FOR THE VECTOR */

	while(READ_REG16(GP_BLIT_STATUS) & BS_BLIT_PENDING);
	WRITE_REG16(GP_DST_XCOOR, x);
	WRITE_REG16(GP_DST_YCOOR, y);
	WRITE_REG16(GP_VECTOR_LENGTH, length);
	WRITE_REG16(GP_INIT_ERROR, initerr);
	WRITE_REG16(GP_AXIAL_ERROR, axialerr);
	WRITE_REG16(GP_DIAG_ERROR, diagerr);
	WRITE_REG16(GP_VECTOR_MODE, vector_mode);
}

/* END OF FILE */


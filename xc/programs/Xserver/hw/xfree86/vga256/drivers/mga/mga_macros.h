/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/mga/mga_macros.h,v 1.1.2.2 1998/02/01 22:08:12 robin Exp $ */

#ifndef _MGA_MACROS_H_
#define _MGA_MACROS_H_

#if PSZ == 8
#define REPLICATE(r) r &= 0xFF; r |= r << 8; r |= r << 16
#elif PSZ == 16
#define REPLICATE(r) r &= 0xFFFF; r |= r << 16
#else
#define REPLICATE(r) /* */
#endif

#if PSZ == 24
#define REPLICATE24(r) r &= 0xFFFFFF; r |= r << 24
#else
#define REPLICATE24(r) REPLICATE(r)
#endif

#define RGBEQUAL(c) (!(((c >> 8) ^ c) & 0xffff)) 

#define WAITFIFO(n) if(!MGAUsePCIRetry) \
	{while(INREG8(MGAREG_FIFOSTATUS) < (n));}

#define XYADDRESS(x,y) ((y) * xf86AccelInfoRec.FramebufferWidth + (x) + MGAydstorg)

#define MAKEDMAINDEX(index)  ((((index) >> 2) & 0x7f) | (((index) >> 6) & 0x80))

#define DMAINDICIES(one,two,three,four)	\
	( MAKEDMAINDEX(one) | \
	 (MAKEDMAINDEX(two) << 8) | \
	 (MAKEDMAINDEX(three) << 16) | \
 	 (MAKEDMAINDEX(four) << 24) ) 


#endif /* _MGA_MACROS_H_ */

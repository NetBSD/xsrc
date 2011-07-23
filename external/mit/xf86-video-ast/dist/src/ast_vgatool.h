/*
 * Copyright (c) 2005 ASPEED Technology Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of the authors not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  The authors makes no representations
 * about the suitability of this software for any purpose.  It is provided
 * "as is" without express or implied warranty.
 *
 * THE AUTHORS DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL THE AUTHORS BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

/* VRAM Size Definition */
#define VIDEOMEM_SIZE_08M	0x00800000
#define VIDEOMEM_SIZE_16M	0x01000000
#define VIDEOMEM_SIZE_32M	0x02000000
#define VIDEOMEM_SIZE_64M	0x04000000
#define VIDEOMEM_SIZE_128M	0x08000000

#define DRAM_SIZE_016M		0x01000000
#define DRAM_SIZE_032M		0x02000000
#define DRAM_SIZE_064M		0x04000000
#define DRAM_SIZE_128M		0x08000000
#define DRAM_SIZE_256M		0x10000000

#define DRAMTYPE_512Mx16	0
#define DRAMTYPE_1Gx16		1
#define DRAMTYPE_512Mx32	2
#define DRAMTYPE_1Gx32		3
#define DRAMTYPE_2Gx16		6
#define DRAMTYPE_4Gx16		7

#define AR_PORT_WRITE		(pAST->RelocateIO + 0x40)
#define MISC_PORT_WRITE		(pAST->RelocateIO + 0x42)
#define SEQ_PORT 		(pAST->RelocateIO + 0x44)
#define DAC_INDEX_READ		(pAST->MMIOVirtualAddr + 0x3c7)
#define DAC_INDEX_WRITE		(pAST->RelocateIO + 0x48)
#define DAC_DATA		(pAST->RelocateIO + 0x49)
#define GR_PORT			(pAST->RelocateIO + 0x4E)
#define CRTC_PORT 		(pAST->RelocateIO + 0x54)
#define INPUT_STATUS1_READ	(pAST->RelocateIO + 0x5A)
#define MISC_PORT_READ		(pAST->RelocateIO + 0x4C)

#define GetReg(base)				inb(base)
#define SetReg(base,val)			outb(base,val)
#define GetIndexReg(base,index,val)			do {			\
                      				outb(base,index);	\
                      				val = inb(base+1);		\
                    				} while (0)
#define SetIndexReg(base,index, val)		do { \
						outw(base, ((USHORT)(val) << 8) | index);	\
						} while (0)
#define GetIndexRegMask(base,index, and, val)	do {			\
                      				outb(base,index);	\
                      				val = (inb(base+1) & and);		\
                    				} while (0)                    			
#define SetIndexRegMask(base,index, and, val)  	do { \
                      				UCHAR __Temp; 	\
                      				outb(base,index);   	\
                      				__Temp = (inb((base)+1)&(and))|(val); 	\
                      				SetIndexReg(base,index,__Temp); 	\
                    				} while (0)

#define VGA_GET_PALETTE_INDEX(index, red, green, blue) \
{ \
   UCHAR __junk;				\
   SetReg(DAC_INDEX_READ,(UCHAR)(index));	\
   __junk = GetReg(SEQ_PORT);			\
   red = GetReg(DAC_DATA);		\
   __junk = GetReg(SEQ_PORT);			\
   green = GetReg(DAC_DATA);		\
   __junk = GetReg(SEQ_PORT);			\
   blue = GetReg(DAC_DATA);		\
   __junk = GetReg(SEQ_PORT);			\
}

#define VGA_LOAD_PALETTE_INDEX(index, red, green, blue) \
{ \
   UCHAR __junk;				\
   SetReg(DAC_INDEX_WRITE,(UCHAR)(index));	\
   __junk = GetReg(SEQ_PORT);			\
   SetReg(DAC_DATA,(UCHAR)(red));		\
   __junk = GetReg(SEQ_PORT);			\
   SetReg(DAC_DATA,(UCHAR)(green));		\
   __junk = GetReg(SEQ_PORT);			\
   SetReg(DAC_DATA,(UCHAR)(blue));		\
   __junk = GetReg(SEQ_PORT);      		\
}

/* Reg. Definition */
#define AST1180_MEM_BASE		0x40000000
#define AST1180_MMC_BASE		0x80FC8000
#define AST1180_SCU_BASE		0x80FC8200
#define AST1180_GFX_BASE		0x80FC9000
#define AST1180_VIDEO_BASE		0x80FCD000

/* AST1180 GFX */
#define AST1180_VGA1_CTRL 	        0x60
#define AST1180_VGA1_CTRL2	        0x64
#define AST1180_VGA1_STATUS 	        0x68
#define AST1180_VGA1_PLL 	        0x6C
#define AST1180_VGA1_HTREG              0x70
#define AST1180_VGA1_HRREG              0x74
#define AST1180_VGA1_VTREG              0x78
#define AST1180_VGA1_VRREG              0x7C
#define AST1180_VGA1_STARTADDR          0x80
#define AST1180_VGA1_OFFSET 	        0x84
#define AST1180_VGA1_THRESHOLD		0x88

#define AST1180_HWC1_OFFSET		0x90
#define AST1180_HWC1_POSITION		0x94
#define AST1180_HWC1_PATTERNADDR	0x98

#define CRT_LOW_THRESHOLD_VALUE         0x40
#define CRT_HIGH_THRESHOLD_VALUE        0x7E

/* GFX Ctrl Reg */
#define AST1180_ENABLECRT		0x00000001
#define AST1180_ENABLEHWC		0x00000002
#define AST1180_MONOHWC			0x00000000
#define AST1180_ALPHAHWC		0x00000400
#define AST1180_HSYNCOFF		0x00040000
#define AST1180_VSYNCOFF		0x00080000
#define AST1180_VGAOFF			0x00100000

#define ReadAST1180SOC(addr, data)	\
{	\
        *(ULONG *) (pAST->MMIOVirtualAddr + 0xF004) = (addr) & 0xFFFF0000;	\
        *(ULONG *) (pAST->MMIOVirtualAddr + 0xF000) = 0x1;	\
        data = (*(ULONG *) (pAST->MMIOVirtualAddr + 0x10000 + ((addr) & 0x0000FFFF)));	\
}

#define WriteAST1180SOC(addr, data)	\
{	\
        ULONG temp;	\
        *(ULONG *) (pAST->MMIOVirtualAddr + 0xF004) = (addr) & 0xFFFF0000;	\
        *(ULONG *) (pAST->MMIOVirtualAddr + 0xF000) = 0x1;	\
        *(ULONG *) (pAST->MMIOVirtualAddr + 0x10000 + ((addr) & 0x0000FFFF)) = (data);	\
        temp = *(ULONG *) (pAST->MMIOVirtualAddr + 0x10000 + ((addr) & 0x0000FFFF));	\
}

#define ReadAST1180MEM(addr, data)	\
{	\
        *(ULONG *) (pAST->MMIOVirtualAddr + 0xF004) = (addr) & 0xFFFF0000;	\
        *(ULONG *) (pAST->MMIOVirtualAddr + 0xF000) = 0x1;	\
        data = (*(ULONG *) (pAST->MMIOVirtualAddr + 0x10000 + ((addr) & 0x0000FFFF)));	\
}

#define WriteAST1180MEM(addr, data)	\
{	\
        *(ULONG *) (pAST->MMIOVirtualAddr + 0xF004) = (addr) & 0xFFFF0000;	\
        *(ULONG *) (pAST->MMIOVirtualAddr + 0xF000) = 0x1;	\
        *(ULONG *) (pAST->MMIOVirtualAddr + 0x10000 + ((addr) & 0x0000FFFF)) = (data);	\
}

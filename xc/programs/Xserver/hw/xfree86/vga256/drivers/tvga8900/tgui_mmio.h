/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/tvga8900/tgui_mmio.h,v 3.1.2.7 1999/06/17 16:24:15 hohndel Exp $ */
/*
 * Copyright 1996 by Alan Hourihane, Wigan, England.
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
 * Author:  Alan Hourihane, alanh@fairlite.demon.co.uk
 */

extern volatile unsigned char *tguiMMIOBase;

/* Merge XY */
#define XY_MERGE(x,y) \
		(((unsigned long)(y) << 16) | ((unsigned long)(x) & 0xffff))

/* MMIO */
#ifndef PC98_TGUI
#define GER_BASE 0x2100
#else
#define GER_BASE 0x0000
#endif /* PC98_TGUI */

#ifdef TRIDENT_MMIO
#define IMAGEBUSY(b) \
		b = (*(unsigned long *)(tguiMMIOBase + IMAGE_GE_STATUS)) & 0xF8000000;
#define BLADEBUSY(b) \
		b = (*(unsigned long *)(tguiMMIOBase + BLADE_GE_STATUS)) & 0xFE800000;
#define BLTBUSY(b) \
		b = (*(unsigned char *)(tguiMMIOBase + GER_STATUS)) & GE_BUSY;
#define OLDBLTBUSY(b) \
		b = (*(unsigned char *)(tguiMMIOBase + OLDGER_STATUS))&GE_BUSY;
#define IMAGE_STATUS(c) \
		*(unsigned long *)(tguiMMIOBase + IMAGE_GE_STATUS) = c;
#define BLADE_STATUS(c) \
		*(unsigned long *)(tguiMMIOBase + BLADE_GE_STATUS) = c;
#define TGUI_STATUS(c) \
		*(unsigned char *)(tguiMMIOBase + GER_STATUS) = c;
#define OLDTGUI_STATUS(c) \
		*(unsigned char *)(tguiMMIOBase + OLDGER_STATUS) = c;
#define TGUI_OPERMODE(c) \
		*(unsigned short *)(tguiMMIOBase + GER_OPERMODE) = c;
#define OLDTGUI_OPERMODE(c) \
		{ \
			*(unsigned short *)(tguiMMIOBase + OLDGER_MWIDTH) = \
				            vga256InfoRec.displayWidth - 1; \
			*(unsigned char *)(tguiMMIOBase + OLDGER_MFORMAT) = c; \
		}
#define TGUI_FCOLOUR(c) \
		*(unsigned long *)(tguiMMIOBase + GER_FCOLOUR) = c;
#define TGUI_FPATCOL(c) \
		*(unsigned long *)(tguiMMIOBase + GER_FPATCOL) = c;
#define OLDTGUI_FCOLOUR(c) \
		*(unsigned long *)(tguiMMIOBase + OLDGER_FCOLOUR) = c;
#define TGUI_BCOLOUR(c) \
		*(unsigned long *)(tguiMMIOBase + GER_BCOLOUR) = c;
#define TGUI_BPATCOL(c) \
		*(unsigned long *)(tguiMMIOBase + GER_BPATCOL) = c;
#define TGUI_DSTKEY(c) \
		*(unsigned long *)(tguiMMIOBase + GER_DSTKEY) = c;
#define OLDTGUI_BCOLOUR(c) \
		*(unsigned long *)(tguiMMIOBase + OLDGER_BCOLOUR) = c;
#define TGUI_PENSTYLE(c) \
		*(unsigned long *)(tguiMMIOBase +GER_PENSTYLE) = c;
#define TGUI_DRAWFLAG(c) \
		*(unsigned long *)(tguiMMIOBase + GER_DRAWFLAG) = c;
#define OLDTGUI_STYLE(c) \
		*(unsigned short *)(tguiMMIOBase + OLDGER_STYLE) = c;
#define TGUI_FMIX(c) \
		*(unsigned char *)(tguiMMIOBase + GER_FMIX) = c;
#define OLDTGUI_FMIX(c) \
		*(unsigned char *)(tguiMMIOBase + OLDGER_FMIX) = c;
#define OLDTGUI_BMIX(c) \
		*(unsigned char *)(tguiMMIOBase + OLDGER_BMIX) = c;
#define TGUI_DIM_XY(w,h) \
		*(unsigned long *)(tguiMMIOBase + GER_DIM_XY) = XY_MERGE(w-1,h-1);
#define OLDTGUI_DIMXY(w,h) \
		*(unsigned long *)(tguiMMIOBase + OLDGER_DIMXY) = XY_MERGE(w-1,h-1);
#define TGUI_SRC_XY(x,y) \
		*(unsigned long *)(tguiMMIOBase + GER_SRC_XY) = XY_MERGE(x,y);
#define TGUI_DEST_XY(x,y) \
		*(unsigned long *)(tguiMMIOBase + GER_DEST_XY) = XY_MERGE(x,y);
#define TGUI_DEST_LINEAR(c) \
		*(unsigned long *)(tguiMMIOBase + GER_DEST_LINEAR) = (c & 0xFFF) | ((c & 0xFFFFF000) << 4);
#define TGUI_DEST_PITCH(c) \
		*(unsigned long *)(tguiMMIOBase + GER_DEST_PITCH) = c;
#define TGUI_BBDST(c) \
		*(unsigned long *)(tguiMMIOBase + GER_BBDST) = c;
#define TGUI_BBSRC(c) \
		*(unsigned long *)(tguiMMIOBase + GER_BBSRC) = c;
#define TGUI_BBDSTSIZE(x,y) \
		*(unsigned long *)(tguiMMIOBase + GER_BBDSTSIZE) = XY_MERGE(x,y);
#define TGUI_BBSRCSIZE(x,y) \
		*(unsigned long *)(tguiMMIOBase + GER_BBSRCSIZE) = XY_MERGE(x,y);
#define OLDTGUI_DESTXY(x,y) \
		*(unsigned long *)(tguiMMIOBase + OLDGER_DESTXY) = XY_MERGE(x,y);
#define OLDTGUI_DESTLINEAR(c) \
		*(unsigned long *)(tguiMMIOBase + OLDGER_DESTLINEAR) = c;
#define TGUI_SRCCLIP_XY(x,y) \
		*(unsigned long *)(tguiMMIOBase + GER_SRCCLIP_XY) = XY_MERGE(x,y);
#define TGUI_DSTCLIP_XY(x,y) \
		*(unsigned long *)(tguiMMIOBase + GER_DSTCLIP_XY) = XY_MERGE(x,y);
#define TGUI_PATLOC(addr) \
		*(unsigned short *)(tguiMMIOBase +GER_PATLOC) = addr;
#define TGUI_PLANEMASK(c) \
		*(unsigned long *)(tguiMMIOBase + GER_PLANEMASK) = c;
#define TGUI_OUTB(addr, c) \
		*(unsigned long *)(tguiMMIOBase + addr) = c;
#define IMAGE_OUT(addr,c) \
		*(unsigned long *)(tguiMMIOBase + addr) = c;
#define BLADE_OUT(addr,c) \
		*(unsigned long *)(tguiMMIOBase + addr) = c;
#define TGUI_COMMAND(c) \
		{ \
		TGUI_OPERMODE(GE_OP); \
		if ((IsTGUI9660 || IsTGUI9680) && !ClipOn) { \
			TGUI_SRCCLIP_XY(0,0);\
			TGUI_DSTCLIP_XY(2047, 2047);\
		}; \
		if (!OFLG_ISSET(OPTION_PCI_RETRY, &vga256InfoRec.options)) \
			TGUISync(); \
		*(unsigned char *)(tguiMMIOBase + GER_COMMAND) = c; \
		}
#define OLDTGUI_COMMAND(c) \
		{ \
		OLDTGUI_OPERMODE(GE_OP); \
		OLDTGUISync(); \
		*(unsigned long *)(tguiMMIOBase + OLDGER_COMMAND) = c; \
		}
#else
#define BLTBUSY(b) \
		b = inb(GER_BASE+GER_STATUS) & GE_BUSY;
#define IMAGEBUSY(b) \
		b = inl(GER_BASE+IMAGE_GE_STATUS) & 0xF8000000;
#define BLADEBUSY(b) \
		b = inl(GER_BASE+BLADE_GE_STATUS) & 0xFE800000;
#define OLDBLTBUSY(b) \
		{	\
			outb(GER_INDEX, OLDGER_STATUS); \
			b = inb(GER_BYTE0) & GE_BUSY; \
		}
#define TGUI_STATUS(c) \
		outb(GER_BASE+GER_STATUS, c);
#define IMAGE_STATUS(c) \
		outl(GER_BASE+IMAGE_GE_STATUS, c);
#define BLADE_STATUS(c) \
		outl(GER_BASE+BLADE_GE_STATUS, c);
#define OLDTGUI_STATUS(c) \
		{	\
			outb(GER_INDEX, OLDGER_STATUS); \
			outb(GER_BYTE0, 0x00); \
		}
#define TGUI_OPERMODE(c) \
		outw(GER_BASE+GER_OPERMODE, c);
#define OLDTGUI_OPERMODE(c) \
		{	\
			outb(GER_INDEX, OLDGER_MWIDTH); \
			outw(GER_BYTE0, vga256InfoRec.displayWidth - 1); \
			outb(GER_INDEX, OLDGER_MFORMAT); \
			outb(GER_BYTE0, c); \
		}
#define TGUI_FCOLOUR(c) \
		outl(GER_BASE+GER_FCOLOUR, c);
#define TGUI_FPATCOL(c) \
		outl(GER_BASE+GER_FPATCOL, c);
#define OLDTGUI_FCOLOUR(c) \
		{	\
			outb(GER_INDEX, OLDGER_FCOLOUR); \
			outl(GER_BYTE0, c); \
		}
#define TGUI_BCOLOUR(c) \
		outl(GER_BASE+GER_BCOLOUR, c);
#define TGUI_BPATCOL(c) \
		outl(GER_BASE+GER_BPATCOL, c);
#define TGUI_DSTKEY(c) \
		outl(GER_BASE+GER_DSTKEY, c);
#define OLDTGUI_BCOLOUR(c) \
		{	\
			outb(GER_INDEX, OLDGER_BCOLOUR); \
			outl(GER_BYTE0, c); \
		}
#define TGUI_PENSTYLE(c) \
		outl(GER_BASE+GER_PENSTYLE, c);
#define TGUI_DRAWFLAG(c) \
		outl(GER_BASE+GER_DRAWFLAG, c);
#define OLDTGUI_STYLE(c) \
		{	\
			outb(GER_INDEX, OLDGER_STYLE); \
			outw(GER_BYTE0, c); \
		}
#define TGUI_FMIX(c) \
		outb(GER_BASE+GER_FMIX, c);
#define OLDTGUI_FMIX(c) \
		{	\
			outb(GER_INDEX, OLDGER_FMIX); \
			outb(GER_BYTE0, c); \
		}
#define OLDTGUI_BMIX(c) \
		{	\
			outb(GER_INDEX, OLDGER_BMIX); \
			outb(GER_BYTE1, c); \
		}
#define TGUI_DIM_XY(w,h) \
		outl(GER_BASE+GER_DIM_XY, XY_MERGE(w-1,h-1));
#define OLDTGUI_DIMXY(w,h) \
		{	\
			outb(GER_INDEX, OLDGER_DIMXY); \
			outl(GER_BYTE0, XY_MERGE(w-1,h-1)); \
		}
#define TGUI_SRC_XY(x,y) \
		outl(GER_BASE+GER_SRC_XY, XY_MERGE(x,y));
#define TGUI_DEST_XY(x,y) \
		outl(GER_BASE+GER_DEST_XY, XY_MERGE(x,y));
#define TGUI_DEST_LINEAR(c) \
		outl(GER_BASE+GER_DEST_LINEAR, (c & 0xFFF) | ((c & 0xFFFFF000) << 4));
#define TGUI_DEST_PITCH(c) \
		outl(GER_BASE+GER_DEST_PITCH, c);
#define TGUI_BBDST(c) \
		outl(GER_BASE+GER_BBDST, c);
#define TGUI_BBSRC(c) \
		outl(GER_BASE+GER_BBSRC, c);
#define TGUI_BBDSTSIZE(x,y) \
		outl(GER_BASE+GER_BBDSTSIZE, XY_MERGE(x,y));
#define TGUI_BBSRCSIZE(x,y) \
		outl(GER_BASE+GER_BBSRCSIZE, XY_MERGE(x,y));
#define OLDTGUI_DESTXY(x,y) \
		{	\
			outb(GER_INDEX, OLDGER_DESTXY); \
			outl(GER_BYTE0, XY_MERGE(x,y)); \
		}
#define OLDTGUI_DESTLINEAR(c) \
		{	\
			outb(GER_INDEX, OLDGER_DESTLINEAR); \
			outl(GER_BYTE0, c); \
		}
#define TGUI_SRCCLIP_XY(x,y) \
		outl(GER_BASE+GER_SRCCLIP_XY, XY_MERGE(x,y));
#define TGUI_DSTCLIP_XY(x,y) \
		outl(GER_BASE+GER_DSTCLIP_XY, XY_MERGE(x,y));
#define TGUI_PATLOC(addr) \
		outw(GER_BASE+GER_PATLOC, addr);
#define BLADE_OUT(addr,c) \
		outl(GER_BASE+addr, c);
#define IMAGE_OUT(addr,c) \
		outl(GER_BASE+addr, c);
#define TGUI_PLANEMASK(c) \
		outl(GER_BASE+GER_PLANEMASK, c);
#define TGUI_OUTB(addr, c) \
		outb(GER_BASE+addr, c);
#define TGUI_COMMAND(c) \
		{ \
		outw(GER_BASE+GER_OPERMODE,GE_OP); \
		if ((IsTGUI9660 || IsTGUI9680) && !ClipOn) { \
			TGUI_SRCCLIP_XY(0,0);\
			TGUI_DSTCLIP_XY(vga256InfoRec.displayWidth - 1, 2047);\
		}; \
		if (OFLG_ISSET(OPTION_PCI_RETRY, &vga256InfoRec.options)) \
			TGUISync(); \
		outb(GER_BASE+GER_COMMAND, c); \
		}
#define OLDTGUI_COMMAND(c) \
		{ \
		OLDTGUI_OPERMODE(GE_OP); \
		OLDTGUISync(); \
		outb(GER_INDEX, OLDGER_COMMAND); \
		outl(GER_BYTE0, c); \
		}
#endif

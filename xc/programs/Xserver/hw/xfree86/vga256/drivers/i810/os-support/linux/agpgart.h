/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/i810/os-support/linux/agpgart.h,v 1.1.2.2 1999/11/18 19:06:20 hohndel Exp $ */
/*
 * AGPGART Hardware Device Driver
 * Copyright (C) 1999 Jeff Hartmann
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * JEFF HARTMANN, OR ANY OTHER CONTRIBUTORS BE LIABLE FOR ANY CLAIM, 
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR 
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE 
 * OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 *
 *    Jeff Hartmann <jhartmann@precisioninsight.com>
 */



#ifndef __LINUX_AGPGART_H
#define __LINUX_AGPGART_H

/* Modified to support i810 and i810-dc100 by Keith Whitwell,
 * <keithw@precisioninsight.com>:
 *
 *  +------+  <---- physical
 *  |      |
 *  | RAM  |
 *  |      |
 *  |      |
 *  |      |
 *  +------+  <---- physical + num_of_slots * 4096
 *  |      |
 *  | VRAM |
 *  |      |
 *  +------+  <---- physical + size * 1024 * 1024
 *            also: physical + (num_of_slots + num_dcache_slots) * 4096
 *
 * DCACHE:
 *
 * Dcache refers to video ram on the card/chipset which is only
 * accessible via the gart.  For the bare Intel i810 and for chipsets
 * without builtin video, num_dcache_slots == 0.  For the i810-dc100,
 * it is 1024 == 4M of builtin video ram.
 *
 * The dcache is pulled into the gtt when the module is loaded, and
 * this mapping cannot be modified via ioctl's to the device. The
 * GARTIOCPGINFO ioctl will return an error for dcache pages as they
 * do not map to physical memory.  
 *
 * In late-breaking news, it turns out that the dcache isn't really a
 * very good place to put the framebuffer, if you have a choice.
 * There is significantly reduced bandwidth between this memory and
 * the blitter, compared to system ram.
 */

struct gart_info
{
   long physical;
   int size;
   int num_of_slots;
   int agpmode;
   int num_dcache_slots;
};

struct gart_pge_info
{
   int index;			/* in */
   long physical;		/* out */
};


/* Note: This module uses a deliberately different set of ioctl
 * numbers in order to distinguish users of this (widely distributed,
 * but still experimental) module from the forthcoming, fully reworked
 * version.
 */
#define GARTIOCINSERT _IOR('K', 1, long)
#define GARTIOCREMOVE _IOR('K', 2, long)
#define GARTIOCINFO   _IOW('K', 3, struct gart_info)
#define GARTIOCMODE   _IOR('K', 4, long)
#define GARTIOCPGINFO _IOWR('K', 5, struct gart_pge_info)

#endif

/* $XConsortium: AsmMacros.h /main/13 1996/10/25 11:33:12 kaleb $ */
/*
 * (c) Copyright 1993,1994 by David Wexelblat <dwex@xfree86.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a 
 * copy of this software and associated documentation files (the "Software"), 
 * to deal in the Software without restriction, including without limitation 
 * the rights to use, copy, modify, merge, publish, distribute, sublicense, 
 * and/or sell copies of the Software, and to permit persons to whom the 
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL 
 * DAVID WEXELBLAT BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, 
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF 
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE 
 * SOFTWARE.
 * 
 * Except as contained in this notice, the name of David Wexelblat shall not be
 * used in advertising or otherwise to promote the sale, use or other dealings
 * in this Software without prior written authorization from David Wexelblat.
 *
 */

/* $XFree86: xc/programs/Xserver/hw/xfree86/SuperProbe/AsmMacros.h,v 3.10 1996/12/23 06:31:04 dawes Exp $ */

#if defined(__GNUC__)
#if defined(linux) && defined(__alpha__)
#define inb _inb
#define inw _inw
#define inl _inl
#define outb(p,v) _outb((v),(p))
#define outw(p,v) _outw((v),(p))
#define outl(p,v) _outl((v),(p))
#else
#ifdef GCCUSESGAS
static __inline__ void
outb(port, val)
short port;
char val;
{
   __asm__ __volatile__("outb %0,%1" : :"a" (val), "d" (port));
}

static __inline__ void
outw(port, val)
short port;
short val;
{
   __asm__ __volatile__("outw %0,%1" : :"a" (val), "d" (port));
}

static __inline__ void
outl(port, val)
short port;
unsigned int val;
{
   __asm__ __volatile__("outl %0,%1" : :"a" (val), "d" (port));
}

static __inline__ unsigned int
inb(port)
short port;
{
   unsigned char ret;
   __asm__ __volatile__("inb %1,%0" :
       "=a" (ret) :
       "d" (port));
   return ret;
}

static __inline__ unsigned int
inw(port)
short port;
{
   unsigned short ret;
   __asm__ __volatile__("inw %1,%0" :
       "=a" (ret) :
       "d" (port));
   return ret;
}

static __inline__ unsigned int
inl(port)
short port;
{
   unsigned int ret;
   __asm__ __volatile__("inl %1,%0" :
       "=a" (ret) :
       "d" (port));
   return ret;
}

#else /* GCCUSESGAS */

static __inline__ void
outb(port, val)
     short port;
     char val;
{
  __asm__ __volatile__("out%B0 (%1)" : :"a" (val), "d" (port));
}

static __inline__ void
outw(port, val)
     short port;
     short val;
{
  __asm__ __volatile__("out%W0 (%1)" : :"a" (val), "d" (port));
}

static __inline__ void
outl(port, val)
     short port;
     unsigned int val;
{
  __asm__ __volatile__("out%L0 (%1)" : :"a" (val), "d" (port));
}

static __inline__ unsigned int
inb(port)
     short port;
{
  unsigned int ret;
  __asm__ __volatile__("in%B0 (%1)" :
                   "=a" (ret) :
                   "d" (port));
  return ret;
}

static __inline__ unsigned int
inw(port)
     short port;
{
  unsigned int ret;
  __asm__ __volatile__("in%W0 (%1)" :
                   "=a" (ret) :
                   "d" (port));
  return ret;
}

static __inline__ unsigned int
inl(port)
     short port;
{
  unsigned int ret;
  __asm__ __volatile__("in%L0 (%1)" :
                   "=a" (ret) :
                   "d" (port));
  return ret;
}

#endif /* GCCUSESGAS */
#endif /* linux && __alpha__ */

#ifdef linux

#define intr_disable()
#define intr_enable()

#else /* !linux */

static __inline__ void
intr_disable()
{
  __asm__ __volatile__("cli");
}

static __inline__ void
intr_enable()
{
  __asm__ __volatile__("sti");
}

#endif /* else !linux */

#else /* __GNUC__ */

#if defined(_MINIX) && defined(_ACK)

/* inb, outb, inw and outw are defined in the library */
/* ... but I've no idea if the same is true for inl & outl */

u8_t inb(U16_t);
void outb(U16_t, U8_t);
u16_t inw(U16_t);
void outw(U16_t, U16_t);
u32_t inl(U16_t);
void outl(U16_t, U32_t);

#else /* not _MINIX and _ACK */

# if defined(__STDC__) && (__STDC__ == 1)
#  ifndef NCR
#  define asm __asm
#  endif
# endif
# ifdef SVR4
#  include <sys/types.h>
#  ifndef __USLC__
#   define __USLC__
#  endif
# endif
#ifndef SCO325
# include <sys/inline.h>
#else
# include "../common/scoasm.h"
#endif
#define intr_disable() asm("cli")
#define intr_enable()  asm("sti")

#endif /* _MINIX and _ACK */
#endif /* __GNUC__ */

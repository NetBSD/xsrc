/*
 * Platform specific SBUS and OpenPROM access declarations.
 *
 * Copyright (C) 2000 Jakub Jelinek (jakub@redhat.com)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * JAKUB JELINEK BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
/* $XFree86: xc/programs/Xserver/hw/xfree86/os-support/bus/xf86Sbus.h,v 1.10 2004/12/07 22:19:50 tsi Exp $ */

#ifndef _XF86_SBUS_H
#define _XF86_SBUS_H

#ifdef XFree86Module
# error XFree86 module code must not #include "xf86Sbus.h"
#else

#if defined(linux)
# include <asm/types.h>
# include <asm/fbio.h>
# include <asm/openpromio.h>
#elif defined(SVR4)
# include <sys/fbio.h>
# include <sys/openpromio.h>
#elif defined(__OpenBSD__) && defined(__sparc64__)
/* XXX */
#elif defined(CSRG_BASED)
# if defined(__FreeBSD__)
#  include <sys/types.h>
#  include <sys/fbio.h>
#  include <dev/ofw/openpromio.h>
# else
#  include <machine/fbio.h>
# endif
#else
# include <sun/fbio.h>
#endif

/*
 * Some of these vary by OS (or are non-existent on some).  Also,
 * adapter-specific headers are not always installed/available, so don't
 * #include them.
 */

#ifndef   FBTYPE_SUN2BW
# define  FBTYPE_SUN2BW 2
#endif

#ifndef   FBTYPE_SUN2COLOR
# define  FBTYPE_SUN2COLOR 3
#endif

#ifndef   FBTYPE_SUN3COLOR
# define  FBTYPE_SUN3COLOR 6
#endif

#ifndef   FBTYPE_SUNFAST_COLOR
# define  FBTYPE_SUNFAST_COLOR 12
#endif

#ifndef   FBTYPE_SUNGP3
# define  FBTYPE_SUNGP3 17
#endif

#ifndef   FBTYPE_SUNGT
# define  FBTYPE_SUNGT 18
#endif

#ifndef   FBTYPE_SUNLEO
# define  FBTYPE_SUNLEO 19
#endif

#ifndef   FBTYPE_MDICOLOR
# ifdef CSRG_BASED
#  define FBTYPE_MDICOLOR 28
# else
#  define FBTYPE_MDICOLOR 20
# endif
#endif

#ifndef   FBTYPE_TCXCOLOR
# ifdef CSRG_BASED
#  define FBTYPE_TCXCOLOR 29
# else
#  define FBTYPE_TCXCOLOR 21
# endif
#endif

#ifndef   FBTYPE_CREATOR
# if defined(linux)
#  define FBTYPE_CREATOR 22
# elif defined(CSRG_BASED)
#  define FBTYPE_CREATOR 30
# elif defined(sun)
#  define FBTYPE_CREATOR 65535	/* Larger than life ... */
# endif
#endif

#endif /* XFree86Module */

#endif /* _XF86_SBUS_H */

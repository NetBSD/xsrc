/*
 * $XFree86: xc/programs/Xserver/fb/fbcompose.c,v 1.5 2000/11/20 07:13:07 keithp Exp $
 *
 * Copyright © 2000 Keith Packard, member of The XFree86 Project, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Keith Packard not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  Keith Packard makes no
 * representations about the suitability of this software for any purpose.  It
 * is provided "as is" without express or implied warranty.
 *
 * KEITH PACKARD DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL KEITH PACKARD BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

#include "fb.h"
#include "picturestr.h"
#include "mipict.h"
#include "fbpict.h"

/*
 * General purpose compositing code optimized for minimal memory
 * references
 *
 * All work is done on canonical ARGB values, functions for fetching
 * and storing these exist for each format.
 */

typedef struct _fbCompSrc {
    CARD32	value;
    CARD32	alpha;
} FbCompSrc;

/*
 * Combine src and mask using IN
 */

CARD32
fbCombineMaskU (FbCompositeOperand   *src,
		FbCompositeOperand   *msk)
{
    CARD32  x;
    CARD16  a;
    CARD16  t;
    CARD32  m,n,o,p;

    if (!msk)
	return (*src->fetch) (src->line, src->offset);
    
    a = (*msk->fetch) (msk->line, msk->offset) >> 24;
    if (!a)
	return 0;
    
    x = (*src->fetch) (src->line, src->offset);
    if (a == 0xff)
	return x;
    
    m = FbInU(x,0,a,t);
    n = FbInU(x,8,a,t);
    o = FbInU(x,16,a,t);
    p = FbInU(x,24,a,t);
    return m|n|o|p;
}

FbCompSrc
fbCombineMaskC (FbCompositeOperand   *src,
		FbCompositeOperand   *msk)
{
    FbCompSrc	s;
    CARD32	x;
    CARD32	a;
    CARD16	xa;
    CARD16	t;
    CARD32	m,n,o,p;

    if (!msk)
    {
	x = (*src->fetch) (src->line, src->offset);
	s.value = x;
	x = x >> 24;
	x |= x << 8;
	x |= x << 16;
	s.alpha = x;
	return s;
    }
    
    a = (*msk->fetcha) (msk->line, msk->offset);
    if (!a)
    {
	s.value = 0;
	s.alpha = 0;
	return s;
    }
    
    x = (*src->fetch) (src->line, src->offset);
    if (a == 0xffffffff)
    {
	s.value = x;
	x = x >> 24;
	x |= x << 8;
	x |= x << 16;
	s.alpha = x;
	return s;
    }
    
    m = FbInC(x,0,a,t);
    n = FbInC(x,8,a,t);
    o = FbInC(x,16,a,t);
    p = FbInC(x,24,a,t);
    s.value = m|n|o|p;
    xa = x >> 24;
    m = FbInU(a,0,xa,t);
    n = FbInU(a,8,xa,t);
    o = FbInU(a,16,xa,t);
    p = FbInU(a,24,xa,t);
    s.alpha = m|n|o|p;
    return s;
}

CARD32
fbCombineMaskValueC (FbCompositeOperand   *src,
		     FbCompositeOperand   *msk)
{
    CARD32	x;
    CARD32	a;
    CARD16	xa;
    CARD16	t;
    CARD32	m,n,o,p;

    if (!msk)
    {
	return (*src->fetch) (src->line, src->offset);
    }
    
    a = (*msk->fetcha) (msk->line, msk->offset);
    if (!a)
	return a;
    
    x = (*src->fetch) (src->line, src->offset);
    if (a == 0xffffffff)
	return x;
    
    m = FbInC(x,0,a,t);
    n = FbInC(x,8,a,t);
    o = FbInC(x,16,a,t);
    p = FbInC(x,24,a,t);
    return m|n|o|p;
}

/*
 * Combine src and mask using IN, generating only the alpha component
 */
CARD32
fbCombineMaskAlphaU (FbCompositeOperand   *src,
		     FbCompositeOperand   *msk)
{
    CARD32  x;
    CARD16  a;
    CARD16  t;

    if (!msk)
	return (*src->fetch) (src->line, src->offset);
    
    a = (*msk->fetch) (msk->line, msk->offset) >> 24;
    if (!a)
	return 0;
    
    x = (*src->fetch) (src->line, src->offset);
    if (a == 0xff)
	return x;
    
    return FbInU(x,24,a,t);
}

CARD32
fbCombineMaskAlphaC (FbCompositeOperand   *src,
		     FbCompositeOperand   *msk)
{
    CARD32	x;
    CARD32	a;
    CARD16	xa;
    CARD16	t;
    CARD32	m,n,o,p;

    if (!msk)
	return (*src->fetch) (src->line, src->offset);
    
    a = (*msk->fetcha) (msk->line, msk->offset);
    if (!a)
	return 0;
    
    x = (*src->fetcha) (src->line, src->offset);
    if (a == 0xffffffff)
	return x;
    
    m = FbInC(x,0,a,t);
    n = FbInC(x,8,a,t);
    o = FbInC(x,16,a,t);
    p = FbInC(x,24,a,t);
    return m|n|o|p;
}

/*
 * All of the composing functions
 */
void
fbCombineClear (FbCompositeOperand   *src,
		FbCompositeOperand   *msk,
		FbCompositeOperand   *dst)
{
    (*dst->store) (dst->line, dst->offset, 0);
}

void
fbCombineSrcU (FbCompositeOperand    *src,
	       FbCompositeOperand    *msk,
	       FbCompositeOperand    *dst)
{
    (*dst->store) (dst->line, dst->offset, fbCombineMaskU (src, msk));
}

void
fbCombineSrcC (FbCompositeOperand    *src,
	       FbCompositeOperand    *msk,
	       FbCompositeOperand    *dst)
{
    (*dst->store) (dst->line, dst->offset, fbCombineMaskValueC (src, msk));
}

void
fbCombineDst (FbCompositeOperand    *src,
	      FbCompositeOperand    *msk,
	      FbCompositeOperand    *dst)
{
    /* noop */
}

void
fbCombineOverU (FbCompositeOperand   *src,
		FbCompositeOperand   *msk,
		FbCompositeOperand   *dst)
{
    CARD32  s, d;
    CARD16  a;
    CARD16  t;
    CARD32  m,n,o,p;

    s = fbCombineMaskU (src, msk);
    a = ~s >> 24;
    if (a != 0xff)
    {
	if (a)
	{
	    d = (*dst->fetch) (dst->line, dst->offset);
	    m = FbOverU(s,d,0,a,t);
	    n = FbOverU(s,d,8,a,t);
	    o = FbOverU(s,d,16,a,t);
	    p = FbOverU(s,d,24,a,t);
	    s = m|n|o|p;
	}
	(*dst->store) (dst->line, dst->offset, s);
    }
}

void
fbCombineOverC (FbCompositeOperand   *src,
		FbCompositeOperand   *msk,
		FbCompositeOperand   *dst)
{
    FbCompSrc	cs;
    CARD32  s, d;
    CARD32  a;
    CARD16  t;
    CARD32  m,n,o,p;

    cs = fbCombineMaskC (src, msk);
    s = cs.value;
    a = ~cs.alpha;
    if (a != 0xffffffff)
    {
	if (a)
	{
	    d = (*dst->fetch) (dst->line, dst->offset);
	    m = FbOverC(s,d,0,a,t);
	    n = FbOverC(s,d,8,a,t);
	    o = FbOverC(s,d,16,a,t);
	    p = FbOverC(s,d,24,a,t);
	    s = m|n|o|p;
	}
	(*dst->store) (dst->line, dst->offset, s);
    }
}

void
fbCombineOverReverseU (FbCompositeOperand    *src,
		       FbCompositeOperand    *msk,
		       FbCompositeOperand    *dst)
{
    CARD32  s, d;
    CARD16  a;
    CARD16  t;
    CARD32  m,n,o,p;

    d = (*dst->fetch) (dst->line, dst->offset);
    a = ~d >> 24;
    if (a)
    {
	s = fbCombineMaskU (src, msk);
	if (a != 0xff)
	{
	    m = FbOverU(d,s,0,a,t);
	    n = FbOverU(d,s,8,a,t);
	    o = FbOverU(d,s,16,a,t);
	    p = FbOverU(d,s,24,a,t);
	    s = m|n|o|p;
	}
	(*dst->store) (dst->line, dst->offset, s);
    }
}

void
fbCombineOverReverseC (FbCompositeOperand    *src,
		       FbCompositeOperand    *msk,
		       FbCompositeOperand    *dst)
{
    CARD32  s, d;
    CARD32  a;
    CARD16  t;
    CARD32  m,n,o,p;

    d = (*dst->fetch) (dst->line, dst->offset);
    a = ~d >> 24;
    if (a)
    {
	s = fbCombineMaskValueC (src, msk);
	if (a != 0xff)
	{
	    m = FbOverU(d,s,0,a,t);
	    n = FbOverU(d,s,8,a,t);
	    o = FbOverU(d,s,16,a,t);
	    p = FbOverU(d,s,24,a,t);
	    s = m|n|o|p;
	}
	(*dst->store) (dst->line, dst->offset, s);
    }
}

void
fbCombineInU (FbCompositeOperand	    *src,
	      FbCompositeOperand	    *msk,
	      FbCompositeOperand	    *dst)
{
    CARD32  s, d;
    CARD16  a;
    CARD16  t;
    CARD32  m,n,o,p;

    d = (*dst->fetch) (dst->line, dst->offset);
    a = d >> 24;
    s = 0;
    if (a)
    {
	s = fbCombineMaskU (src, msk);
	if (a != 0xff)
	{
	    m = FbInU(s,0,a,t);
	    n = FbInU(s,8,a,t);
	    o = FbInU(s,16,a,t);
	    p = FbInU(s,24,a,t);
	    s = m|n|o|p;
	}
    }
    (*dst->store) (dst->line, dst->offset, s);
}

void
fbCombineInC (FbCompositeOperand	    *src,
	      FbCompositeOperand	    *msk,
	      FbCompositeOperand	    *dst)
{
    CARD32  s, d;
    CARD16  a;
    CARD16  t;
    CARD32  m,n,o,p;

    d = (*dst->fetch) (dst->line, dst->offset);
    a = d >> 24;
    s = 0;
    if (a)
    {
	s = fbCombineMaskValueC (src, msk);
	if (a != 0xff)
	{
	    m = FbInU(s,0,a,t);
	    n = FbInU(s,8,a,t);
	    o = FbInU(s,16,a,t);
	    p = FbInU(s,24,a,t);
	    s = m|n|o|p;
	}
    }
    (*dst->store) (dst->line, dst->offset, s);
}

void
fbCombineInReverseU (FbCompositeOperand  *src,
		     FbCompositeOperand  *msk,
		     FbCompositeOperand  *dst)
{
    CARD32  s, d;
    CARD16  a;
    CARD16  t;
    CARD32  m,n,o,p;

    s = fbCombineMaskAlphaU (src, msk);
    a = s >> 24;
    if (a != 0xff)
    {
	d = 0;
	if (a)
	{
	    d = (*dst->fetch) (dst->line, dst->offset);
	    m = FbInU(d,0,a,t);
	    n = FbInU(d,8,a,t);
	    o = FbInU(d,16,a,t);
	    p = FbInU(d,24,a,t);
	    d = m|n|o|p;
	}
	(*dst->store) (dst->line, dst->offset, d);
    }
}

void
fbCombineInReverseC (FbCompositeOperand  *src,
		     FbCompositeOperand  *msk,
		     FbCompositeOperand  *dst)
{
    CARD32  s, d;
    CARD32  a;
    CARD16  t;
    CARD32  m,n,o,p;

    s = fbCombineMaskAlphaC (src, msk);
    a = s;
    if (a != 0xffffffff)
    {
	d = 0;
	if (a)
	{
	    d = (*dst->fetch) (dst->line, dst->offset);
	    m = FbInC(d,0,a,t);
	    n = FbInC(d,8,a,t);
	    o = FbInC(d,16,a,t);
	    p = FbInC(d,24,a,t);
	    d = m|n|o|p;
	}
	(*dst->store) (dst->line, dst->offset, d);
    }
}

void
fbCombineOutU (FbCompositeOperand    *src,
	       FbCompositeOperand    *msk,
	       FbCompositeOperand    *dst)
{
    CARD32  s, d;
    CARD16  a;
    CARD16  t;
    CARD32  m,n,o,p;

    d = (*dst->fetch) (dst->line, dst->offset);
    a = ~d >> 24;
    s = 0;
    if (a)
    {
	s = fbCombineMaskU (src, msk);
	if (a != 0xff)
	{
	    m = FbInU(s,0,a,t);
	    n = FbInU(s,8,a,t);
	    o = FbInU(s,16,a,t);
	    p = FbInU(s,24,a,t);
	    s = m|n|o|p;
	}
    }
    (*dst->store) (dst->line, dst->offset, s);
}

void
fbCombineOutC (FbCompositeOperand    *src,
	       FbCompositeOperand    *msk,
	       FbCompositeOperand    *dst)
{
    CARD32  s, d;
    CARD16  a;
    CARD16  t;
    CARD32  m,n,o,p;

    d = (*dst->fetch) (dst->line, dst->offset);
    a = ~d >> 24;
    s = 0;
    if (a)
    {
	s = fbCombineMaskValueC (src, msk);
	if (a != 0xff)
	{
	    m = FbInU(s,0,a,t);
	    n = FbInU(s,8,a,t);
	    o = FbInU(s,16,a,t);
	    p = FbInU(s,24,a,t);
	    s = m|n|o|p;
	}
    }
    (*dst->store) (dst->line, dst->offset, s);
}

void
fbCombineOutReverseU (FbCompositeOperand *src,
		      FbCompositeOperand *msk,
		      FbCompositeOperand *dst)
{
    CARD32  s, d;
    CARD16  a;
    CARD16  t;
    CARD32  m,n,o,p;

    s = fbCombineMaskAlphaU (src, msk);
    a = ~s >> 24;
    if (a != 0xff)
    {
	d = 0;
	if (a)
	{
	    d = (*dst->fetch) (dst->line, dst->offset);
	    m = FbInU(d,0,a,t);
	    n = FbInU(d,8,a,t);
	    o = FbInU(d,16,a,t);
	    p = FbInU(d,24,a,t);
	    d = m|n|o|p;
	}
	(*dst->store) (dst->line, dst->offset, d);
    }
}

void
fbCombineOutReverseC (FbCompositeOperand *src,
		      FbCompositeOperand *msk,
		      FbCompositeOperand *dst)
{
    CARD32  s, d;
    CARD32  a;
    CARD16  t;
    CARD32  m,n,o,p;

    s = fbCombineMaskAlphaC (src, msk);
    a = ~s;
    if (a != 0xffffffff)
    {
	d = 0;
	if (a)
	{
	    d = (*dst->fetch) (dst->line, dst->offset);
	    m = FbInC(d,0,a,t);
	    n = FbInC(d,8,a,t);
	    o = FbInC(d,16,a,t);
	    p = FbInC(d,24,a,t);
	    d = m|n|o|p;
	}
	(*dst->store) (dst->line, dst->offset, d);
    }
}

void
fbCombineAtopU (FbCompositeOperand   *src,
		FbCompositeOperand   *msk,
		FbCompositeOperand   *dst)
{
    CARD32  s, d;
    CARD16  ad, as;
    CARD16  t;
    CARD32  m,n,o,p;
    
    s = fbCombineMaskU (src, msk);
    d = (*dst->fetch) (dst->line, dst->offset);
    ad = ~s >> 24;
    as = d >> 24;
    m = FbGen(s,d,0,as,ad,t);
    n = FbGen(s,d,8,as,ad,t);
    o = FbGen(s,d,16,as,ad,t);
    p = FbGen(s,d,24,as,ad,t);
    (*dst->store) (dst->line, dst->offset, m|n|o|p);
}

void
fbCombineAtopC (FbCompositeOperand   *src,
		FbCompositeOperand   *msk,
		FbCompositeOperand   *dst)
{
    FbCompSrc	cs;
    CARD32  s, d;
    CARD32  ad;
    CARD16  as;
    CARD16  t;
    CARD32  m,n,o,p;
    
    cs = fbCombineMaskC (src, msk);
    d = (*dst->fetch) (dst->line, dst->offset);
    s = cs.value;
    ad = cs.alpha;
    as = d >> 24;
    m = FbGen(s,d,0,as,FbGet8(ad,0),t);
    n = FbGen(s,d,8,as,FbGet8(ad,8),t);
    o = FbGen(s,d,16,as,FbGet8(ad,16),t);
    p = FbGen(s,d,24,as,FbGet8(ad,24),t);
    (*dst->store) (dst->line, dst->offset, m|n|o|p);
}

void
fbCombineAtopReverseU (FbCompositeOperand    *src,
		       FbCompositeOperand    *msk,
		       FbCompositeOperand    *dst)
{
    CARD32  s, d;
    CARD16  ad, as;
    CARD16  t;
    CARD32  m,n,o,p;
    
    s = fbCombineMaskU (src, msk);
    d = (*dst->fetch) (dst->line, dst->offset);
    ad = s >> 24;
    as = ~d >> 24;
    m = FbGen(s,d,0,as,ad,t);
    n = FbGen(s,d,8,as,ad,t);
    o = FbGen(s,d,16,as,ad,t);
    p = FbGen(s,d,24,as,ad,t);
    (*dst->store) (dst->line, dst->offset, m|n|o|p);
}

void
fbCombineAtopReverseC (FbCompositeOperand    *src,
		       FbCompositeOperand    *msk,
		       FbCompositeOperand    *dst)
{
    FbCompSrc	cs;
    CARD32  s, d, ad;
    CARD16  as;
    CARD16  t;
    CARD32  m,n,o,p;
    
    cs = fbCombineMaskC (src, msk);
    d = (*dst->fetch) (dst->line, dst->offset);
    s = cs.value;
    ad = cs.alpha;
    as = ~d >> 24;
    m = FbGen(s,d,0,as,FbGet8(ad,0),t);
    n = FbGen(s,d,8,as,FbGet8(ad,8),t);
    o = FbGen(s,d,16,as,FbGet8(ad,16),t);
    p = FbGen(s,d,24,as,FbGet8(ad,24),t);
    (*dst->store) (dst->line, dst->offset, m|n|o|p);
}

void
fbCombineXorU (FbCompositeOperand    *src,
	       FbCompositeOperand    *msk,
	       FbCompositeOperand    *dst)
{
    CARD32  s, d;
    CARD16  ad, as;
    CARD16  t;
    CARD32  m,n,o,p;
    
    s = fbCombineMaskU (src, msk);
    d = (*dst->fetch) (dst->line, dst->offset);
    ad = ~s >> 24;
    as = ~d >> 24;
    m = FbGen(s,d,0,as,ad,t);
    n = FbGen(s,d,8,as,ad,t);
    o = FbGen(s,d,16,as,ad,t);
    p = FbGen(s,d,24,as,ad,t);
    (*dst->store) (dst->line, dst->offset, m|n|o|p);
}

void
fbCombineXorC (FbCompositeOperand    *src,
	       FbCompositeOperand    *msk,
	       FbCompositeOperand    *dst)
{
    FbCompSrc	cs;
    CARD32  s, d, ad;
    CARD16  as;
    CARD16  t;
    CARD32  m,n,o,p;
    
    cs = fbCombineMaskC (src, msk);
    d = (*dst->fetch) (dst->line, dst->offset);
    s = cs.value;
    ad = ~cs.alpha;
    as = ~d >> 24;
    m = FbGen(s,d,0,as,ad,t);
    n = FbGen(s,d,8,as,ad,t);
    o = FbGen(s,d,16,as,ad,t);
    p = FbGen(s,d,24,as,ad,t);
    (*dst->store) (dst->line, dst->offset, m|n|o|p);
}

void
fbCombineAddU (FbCompositeOperand    *src,
	       FbCompositeOperand    *msk,
	       FbCompositeOperand    *dst)
{
    CARD32  s, d;
    CARD16  t;
    CARD32  m,n,o,p;

    s = fbCombineMaskU (src, msk);
    if (s == ~0)
	(*dst->store) (dst->line, dst->offset, s);
    else
    {
	d = (*dst->fetch) (dst->line, dst->offset);
	if (s && d != ~0)
	{
	    m = FbAdd(s,d,0,t);
	    n = FbAdd(s,d,8,t);
	    o = FbAdd(s,d,16,t);
	    p = FbAdd(s,d,24,t);
	    (*dst->store) (dst->line, dst->offset, m|n|o|p);
	}
    }
}

void
fbCombineAddC (FbCompositeOperand    *src,
	       FbCompositeOperand    *msk,
	       FbCompositeOperand    *dst)
{
    CARD32  s, d;
    CARD16  t;
    CARD32  m,n,o,p;

    s = fbCombineMaskValueC (src, msk);
    if (s == ~0)
	(*dst->store) (dst->line, dst->offset, s);
    else
    {
	d = (*dst->fetch) (dst->line, dst->offset);
	if (s && d != ~0)
	{
	    m = FbAdd(s,d,0,t);
	    n = FbAdd(s,d,8,t);
	    o = FbAdd(s,d,16,t);
	    p = FbAdd(s,d,24,t);
	    (*dst->store) (dst->line, dst->offset, m|n|o|p);
	}
    }
}

void
fbCombineSaturateU (FbCompositeOperand   *src,
		    FbCompositeOperand   *msk,
		    FbCompositeOperand   *dst)
{
    CARD32  s, d;
    CARD16  sa, da;
    CARD16  ad, as;
    CARD16  t;
    CARD32  m,n,o,p;
    
    s = fbCombineMaskU (src, msk);
    d = (*dst->fetch) (dst->line, dst->offset);
    sa = s >> 24;
    da = ~d >> 24;
    if (sa <= da)
    {
	m = FbAdd(s,d,0,t);
	n = FbAdd(s,d,8,t);
	o = FbAdd(s,d,16,t);
	p = FbAdd(s,d,24,t);
    }
    else
    {
	as = (da << 8) / sa;
	ad = 0xff;
	m = FbGen(s,d,0,as,ad,t);
	n = FbGen(s,d,8,as,ad,t);
	o = FbGen(s,d,16,as,ad,t);
	p = FbGen(s,d,24,as,ad,t);
    }
    (*dst->store) (dst->line, dst->offset, m|n|o|p);
}

void
fbCombineSaturateC (FbCompositeOperand   *src,
		    FbCompositeOperand   *msk,
		    FbCompositeOperand   *dst)
{
    FbCompSrc	cs;
    CARD32  s, d;
    CARD16  sa, sr, sg, sb, da;
    CARD16  ad, as;
    CARD16  t;
    CARD32  m,n,o,p;
    
    cs = fbCombineMaskC (src, msk);
    d = (*dst->fetch) (dst->line, dst->offset);
    s = cs.value;
    sa = (cs.alpha >> 24) & 0xff;
    sr = (cs.alpha >> 16) & 0xff;
    sg = (cs.alpha >>  8) & 0xff;
    sb = (cs.alpha      ) & 0xff;
    da = ~d >> 24;
    
    if (sb <= da)
	m = FbAdd(s,d,0,t);
    else
	m = FbGen (s, d, 0, (da << 8) / sb, 0xff, t);
    
    if (sg <= da)
	n = FbAdd(s,d,8,t);
    else
	n = FbGen (s, d, 8, (da << 8) / sg, 0xff, t);
    
    if (sr < da)
	o = FbAdd(s,d,16,t);
    else
	o = FbGen (s, d, 16, (da << 8) / sr, 0xff, t);

    if (sa <= da)
	p = FbAdd(s,d,24,t);
    else
	p = FbGen (s, d, 24, (da << 8) / sa, 0xff, t);
    
    (*dst->store) (dst->line, dst->offset, m|n|o|p);
}

FbCombineFunc	fbCombineFuncU[] = {
    fbCombineClear,
    fbCombineSrcU,
    fbCombineDst,
    fbCombineOverU,
    fbCombineOverReverseU,
    fbCombineInU,
    fbCombineInReverseU,
    fbCombineOutU,
    fbCombineOutReverseU,
    fbCombineAtopU,
    fbCombineAtopReverseU,
    fbCombineXorU,
    fbCombineAddU,
    fbCombineSaturateU,
};

FbCombineFunc	fbCombineFuncC[] = {
    fbCombineClear,
    fbCombineSrcC,
    fbCombineDst,
    fbCombineOverC,
    fbCombineOverReverseC,
    fbCombineInC,
    fbCombineInReverseC,
    fbCombineOutC,
    fbCombineOutReverseC,
    fbCombineAtopC,
    fbCombineAtopReverseC,
    fbCombineXorC,
    fbCombineAddC,
    fbCombineSaturateC,
};

/*
 * All of the fetch functions
 */

CARD32
fbFetch_a8r8g8b8 (FbBits *line, CARD32 offset)
{
    return ((CARD32 *)line)[offset >> 5];
}

CARD32
fbFetch_x8r8g8b8 (FbBits *line, CARD32 offset)
{
    return ((CARD32 *)line)[offset >> 5] | 0xff000000;
}

CARD32
fbFetch_a8b8g8r8 (FbBits *line, CARD32 offset)
{
    CARD32  pixel = ((CARD32 *)line)[offset >> 5];

    return ((pixel & 0xff000000) |
	    ((pixel >> 16) & 0xff) |
	    (pixel & 0x0000ff00) |
	    ((pixel & 0xff) << 16));
}

CARD32
fbFetch_x8b8g8r8 (FbBits *line, CARD32 offset)
{
    CARD32  pixel = ((CARD32 *)line)[offset >> 5];

    return ((0xff000000) |
	    ((pixel >> 16) & 0xff) |
	    (pixel & 0x0000ff00) |
	    ((pixel & 0xff) << 16));
}

CARD32
fbFetch_r8g8b8 (FbBits *line, CARD32 offset)
{
    CARD8   *pixel = ((CARD8 *) line) + (offset >> 3);
#if IMAGE_BYTE_ORDER == MSBFirst
    return (0xff000000 |
	    (pixel[0] << 16) |
	    (pixel[1] << 8) |
	    (pixel[2]));
#else
    return (0xff000000 |
	    (pixel[2] << 16) |
	    (pixel[1] << 8) |
	    (pixel[0]));
#endif
}

CARD32
fbFetch_b8g8r8 (FbBits *line, CARD32 offset)
{
    CARD8   *pixel = ((CARD8 *) line) + (offset >> 3);
#if IMAGE_BYTE_ORDER == MSBFirst
    return (0xff000000 |
	    (pixel[2] << 16) |
	    (pixel[1] << 8) |
	    (pixel[0]));
#else
    return (0xff000000 |
	    (pixel[0] << 16) |
	    (pixel[1] << 8) |
	    (pixel[2]));
#endif
}

CARD32
fbFetch_r5g6b5 (FbBits *line, CARD32 offset)
{
    CARD32  pixel = ((CARD16 *) line)[offset >> 4];
    CARD32  r,g,b;

    r = ((pixel & 0xf800) | ((pixel & 0xe000) >> 5)) << 8;
    g = ((pixel & 0x07e0) | ((pixel & 0x0600) >> 6)) << 5;
    b = ((pixel & 0x001c) | ((pixel & 0x001f) << 5)) >> 2;
    return (0xff000000 | r | g | b);
}

CARD32
fbFetch_b5g6r5 (FbBits *line, CARD32 offset)
{
    CARD32  pixel = ((CARD16 *) line)[offset >> 4];
    CARD32  r,g,b;

    b = ((pixel & 0xf800) | ((pixel & 0xe000) >> 5)) >> 8;
    g = ((pixel & 0x07e0) | ((pixel & 0x0600) >> 6)) << 5;
    r = ((pixel & 0x001c) | ((pixel & 0x001f) << 5)) << 14;
    return (0xff000000 | r | g | b);
}

CARD32
fbFetch_a1r5g5b5 (FbBits *line, CARD32 offset)
{
    CARD32  pixel = ((CARD16 *) line)[offset >> 4];
    CARD32  a,r,g,b;

    a = (CARD32) ((CARD8) (0 - ((pixel & 0x8000) >> 15))) << 24;
    r = ((pixel & 0x7c00) | ((pixel & 0x7000) >> 5)) << 9;
    g = ((pixel & 0x03e0) | ((pixel & 0x0380) >> 5)) << 6;
    b = ((pixel & 0x001c) | ((pixel & 0x001f) << 5)) >> 2;
    return (a | r | g | b);
}

CARD32
fbFetch_x1r5g5b5 (FbBits *line, CARD32 offset)
{
    CARD32  pixel = ((CARD16 *) line)[offset >> 4];
    CARD32  r,g,b;

    r = ((pixel & 0x7c00) | ((pixel & 0x7000) >> 5)) << 9;
    g = ((pixel & 0x03e0) | ((pixel & 0x0380) >> 5)) << 6;
    b = ((pixel & 0x001c) | ((pixel & 0x001f) << 5)) >> 2;
    return (0xff000000 | r | g | b);
}

CARD32
fbFetch_a1b5g5r5 (FbBits *line, CARD32 offset)
{
    CARD32  pixel = ((CARD16 *) line)[offset >> 4];
    CARD32  a,r,g,b;

    a = (CARD32) ((CARD8) (0 - ((pixel & 0x8000) >> 15))) << 24;
    b = ((pixel & 0x7c00) | ((pixel & 0x7000) >> 5)) >> 7;
    g = ((pixel & 0x03e0) | ((pixel & 0x0380) >> 5)) << 6;
    r = ((pixel & 0x001c) | ((pixel & 0x001f) << 5)) << 14;
    return (a | r | g | b);
}

CARD32
fbFetch_x1b5g5r5 (FbBits *line, CARD32 offset)
{
    CARD32  pixel = ((CARD16 *) line)[offset >> 4];
    CARD32  r,g,b;

    b = ((pixel & 0x7c00) | ((pixel & 0x7000) >> 5)) >> 7;
    g = ((pixel & 0x03e0) | ((pixel & 0x0380) >> 5)) << 6;
    r = ((pixel & 0x001c) | ((pixel & 0x001f) << 5)) << 14;
    return (0xff000000 | r | g | b);
}

CARD32
fbFetch_a8 (FbBits *line, CARD32 offset)
{
    CARD32   pixel = ((CARD8 *) line)[offset>>3];
    
    return pixel << 24;
}

CARD32
fbFetcha_a8 (FbBits *line, CARD32 offset)
{
    CARD32   pixel = ((CARD8 *) line)[offset>>3];
    
    pixel |= pixel << 8;
    pixel |= pixel << 16;
    return pixel;
}

CARD32
fbFetch_r3g3b2 (FbBits *line, CARD32 offset)
{
    CARD32   pixel = ((CARD8 *) line)[offset>>3];
    CARD32  r,g,b;
    
    r = ((pixel & 0xe0) | ((pixel & 0xe0) >> 3) | ((pixel & 0xc0) >> 6)) << 16;
    g = ((pixel & 0x1c) | ((pixel & 0x18) >> 3) | ((pixel & 0x1c) << 3)) << 8;
    b = (((pixel & 0x03)     ) | 
	 ((pixel & 0x03) << 2) | 
	 ((pixel & 0x03) << 4) |
	 ((pixel & 0x03) << 6));
    return (0xff000000 | r | g | b);
}

CARD32
fbFetch_b2g3r3 (FbBits *line, CARD32 offset)
{
    CARD32   pixel = ((CARD8 *) line)[offset>>3];
    CARD32  r,g,b;
    
    b = (((pixel & 0xc0)     ) | 
	 ((pixel & 0xc0) >> 2) |
	 ((pixel & 0xc0) >> 4) |
	 ((pixel & 0xc0) >> 6));
    g = ((pixel & 0x38) | ((pixel & 0x38) >> 3) | ((pixel & 0x30) << 2)) << 8;
    r = (((pixel & 0x07)     ) | 
	 ((pixel & 0x07) << 3) | 
	 ((pixel & 0x06) << 6)) << 16;
    return (0xff000000 | r | g | b);
}

CARD32
fbFetch_a2r2g2b2 (FbBits *line, CARD32 offset)
{
    CARD32   pixel = ((CARD8 *) line)[offset>>3];
    CARD32   a,r,g,b;

    a = ((pixel & 0xc0) * 0x55) << 18;
    r = ((pixel & 0x30) * 0x55) << 12;
    g = ((pixel & 0x0c) * 0x55) << 6;
    b = ((pixel & 0x03) * 0x55);
    return a|r|g|b;
}

CARD32
fbFetch_a2b2g2r2 (FbBits *line, CARD32 offset)
{
    CARD32   pixel = ((CARD8 *) line)[offset>>3];
    CARD32   a,r,g,b;

    a = ((pixel & 0xc0) * 0x55) << 18;
    b = ((pixel & 0x30) * 0x55) >> 6;
    g = ((pixel & 0x0c) * 0x55) << 6;
    r = ((pixel & 0x03) * 0x55) << 16;
    return a|r|g|b;
}

#define Fetch8(l,o)    (((CARD8 *) (l))[(o) >> 3])
#if IMAGE_BYTE_ORDER == MSBFirst
#define Fetch4(l,o)    ((o) & 2 ? Fetch8(l,o) & 0xf : Fetch8(l,o) >> 4)
#else
#define Fetch4(l,o)    ((o) & 2 ? Fetch8(l,o) >> 4 : Fetch8(l,o) & 0xf)
#endif

CARD32
fbFetch_a4 (FbBits *line, CARD32 offset)
{
    CARD32  pixel = Fetch4(line, offset);
    
    pixel |= pixel << 4;
    return pixel << 24;
}

CARD32
fbFetcha_a4 (FbBits *line, CARD32 offset)
{
    CARD32  pixel = Fetch4(line, offset);
    
    pixel |= pixel << 4;
    pixel |= pixel << 8;
    pixel |= pixel << 16;
    return pixel;
}

CARD32
fbFetch_r1g2b1 (FbBits *line, CARD32 offset)
{
    CARD32  pixel = Fetch4(line, offset);
    CARD32  r,g,b;

    r = ((pixel & 0x8) * 0xff) << 13;
    g = ((pixel & 0x6) * 0x55) << 7;
    b = ((pixel & 0x1) * 0xff);
    return 0xff000000|r|g|b;
}

CARD32
fbFetch_b1g2r1 (FbBits *line, CARD32 offset)
{
    CARD32  pixel = Fetch4(line, offset);
    CARD32  r,g,b;

    b = ((pixel & 0x8) * 0xff) >> 3;
    g = ((pixel & 0x6) * 0x55) << 7;
    r = ((pixel & 0x1) * 0xff) << 16;
    return 0xff000000|r|g|b;
}

CARD32
fbFetch_a1r1g1b1 (FbBits *line, CARD32 offset)
{
    CARD32  pixel = Fetch4(line, offset);
    CARD32  a,r,g,b;

    a = ((pixel & 0x8) * 0xff) << 21;
    r = ((pixel & 0x4) * 0xff) << 14;
    g = ((pixel & 0x2) * 0xff) << 7;
    b = ((pixel & 0x1) * 0xff);
    return a|r|g|b;
}

CARD32
fbFetch_a1b1g1r1 (FbBits *line, CARD32 offset)
{
    CARD32  pixel = Fetch4(line, offset);
    CARD32  a,r,g,b;

    a = ((pixel & 0x8) * 0xff) << 21;
    r = ((pixel & 0x4) * 0xff) >> 3;
    g = ((pixel & 0x2) * 0xff) << 7;
    b = ((pixel & 0x1) * 0xff) << 16;
    return a|r|g|b;
}

CARD32
fbFetcha_a1 (FbBits *line, CARD32 offset)
{
    CARD32  pixel = ((CARD32 *)line)[offset >> 5];
    CARD32  a;
#if BITMAP_BIT_ORDER == MSBFirst
    a = pixel >> (0x1f - offset & 0x1f);
#else
    a = pixel >> (offset & 0x1f);
#endif
    a = a & 1;
    a |= a << 1;
    a |= a << 2;
    a |= a << 4;
    a |= a << 8;
    a |= a << 16;
    return a;
}

CARD32
fbFetch_a1 (FbBits *line, CARD32 offset)
{
    CARD32  pixel = ((CARD32 *)line)[offset >> 5];
    CARD32  a;
#if BITMAP_BIT_ORDER == MSBFirst
    a = pixel >> (0x1f - offset & 0x1f);
#else
    a = pixel >> (offset & 0x1f);
#endif
    a = a & 1;
    a |= a << 1;
    a |= a << 2;
    a |= a << 4;
    return a << 24;
}

/*
 * All the store functions
 */

#define Splita(v)	CARD32	a = ((v) >> 24), r = ((v) >> 16) & 0xff, g = ((v) >> 8) & 0xff, b = (v) & 0xff
#define Split(v)	CARD32	r = ((v) >> 16) & 0xff, g = ((v) >> 8) & 0xff, b = (v) & 0xff

void
fbStore_a8r8g8b8 (FbBits *line, CARD32 offset, CARD32 value)
{
    ((CARD32 *)line)[offset >> 5] = value;
}

void
fbStore_x8r8g8b8 (FbBits *line, CARD32 offset, CARD32 value)
{
    ((CARD32 *)line)[offset >> 5] = value & 0xffffff;
}

void
fbStore_a8b8g8r8 (FbBits *line, CARD32 offset, CARD32 value)
{
    Splita(value);
    ((CARD32 *)line)[offset >> 5] = a << 24 | b << 16 | g << 8 | r;
}

void
fbStore_x8b8g8r8 (FbBits *line, CARD32 offset, CARD32 value)
{
    Split(value);
    ((CARD32 *)line)[offset >> 5] = b << 16 | g << 8 | r;
}

void
fbStore_r8g8b8 (FbBits *line, CARD32 offset, CARD32 value)
{
    CARD8   *pixel = ((CARD8 *) line) + (offset >> 3);
    Split(value);
#if IMAGE_BYTE_ORDER == MSBFirst
    pixel[0] = r;
    pixel[1] = g;
    pixel[2] = b;
#else
    pixel[0] = b;
    pixel[1] = g;
    pixel[2] = r;
#endif
}

void
fbStore_b8g8r8 (FbBits *line, CARD32 offset, CARD32 value)
{
    CARD8   *pixel = ((CARD8 *) line) + (offset >> 3);
    Split(value);
#if IMAGE_BYTE_ORDER == MSBFirst
    pixel[0] = b;
    pixel[1] = g;
    pixel[2] = r;
#else
    pixel[0] = r;
    pixel[1] = g;
    pixel[2] = b;
#endif
}

void
fbStore_r5g6b5 (FbBits *line, CARD32 offset, CARD32 value)
{
    CARD16  *pixel = ((CARD16 *) line) + (offset >> 4);
    Split(value);
    *pixel = (((r << 8) & 0xf800) |
	      ((g << 3) & 0x07e0) |
	      ((b >> 3)         ));
}

void
fbStore_b5g6r5 (FbBits *line, CARD32 offset, CARD32 value)
{
    CARD16  *pixel = ((CARD16 *) line) + (offset >> 4);
    Split(value);
    *pixel = (((b << 8) & 0xf800) |
	      ((g << 3) & 0x07e0) |
	      ((r >> 3)         ));
}

void
fbStore_a1r5g5b5 (FbBits *line, CARD32 offset, CARD32 value)
{
    CARD16  *pixel = ((CARD16 *) line) + (offset >> 4);
    Splita(value);
    *pixel = (((a << 8) & 0x8000) |
	      ((r << 7) & 0x7c00) |
	      ((g << 2) & 0x03e0) |
	      ((b >> 3)         ));
}

void
fbStore_x1r5g5b5 (FbBits *line, CARD32 offset, CARD32 value)
{
    CARD16  *pixel = ((CARD16 *) line) + (offset >> 4);
    Split(value);
    *pixel = (((r << 7) & 0x7c00) |
	      ((g << 2) & 0x03e0) |
	      ((b >> 3)         ));
}

void
fbStore_a1b5g5r5 (FbBits *line, CARD32 offset, CARD32 value)
{
    CARD16  *pixel = ((CARD16 *) line) + (offset >> 4);
    Splita(value);
    *pixel = (((a << 8) & 0x8000) |
	      ((b << 7) & 0x7c00) |
	      ((g << 2) & 0x03e0) |
	      ((r >> 3)         ));
}

void
fbStore_x1b5g5r5 (FbBits *line, CARD32 offset, CARD32 value)
{
    CARD16  *pixel = ((CARD16 *) line) + (offset >> 4);
    Split(value);
    *pixel = (((b << 7) & 0x7c00) |
	      ((g << 2) & 0x03e0) |
	      ((r >> 3)         ));
}

void
fbStore_a8 (FbBits *line, CARD32 offset, CARD32 value)
{
    CARD8   *pixel = ((CARD8 *) line) + (offset >> 3);
    *pixel = value >> 24;
}

void
fbStore_r3g3b2 (FbBits *line, CARD32 offset, CARD32 value)
{
    CARD8   *pixel = ((CARD8 *) line) + (offset >> 3);
    Split(value);
    *pixel = (((r     ) & 0xe0) |
	      ((g >> 3) & 0x1c) |
	      ((b >> 6)       ));
}

void
fbStore_b2g3r3 (FbBits *line, CARD32 offset, CARD32 value)
{
    CARD8   *pixel = ((CARD8 *) line) + (offset >> 3);
    Split(value);
    *pixel = (((b     ) & 0xe0) |
	      ((g >> 3) & 0x1c) |
	      ((r >> 6)       ));
}

void
fbStore_a2r2g2b2 (FbBits *line, CARD32 offset, CARD32 value)
{
    CARD8   *pixel = ((CARD8 *) line) + (offset >> 3);
    Splita(value);
    *pixel = (((a     ) & 0xc0) |
	      ((r >> 2) & 0x30) |
	      ((g >> 4) & 0x0c) |
	      ((b >> 6)       ));
}

#define Store8(l,o,v)  (((CARD8 *) l)[(o) >> 3] = (v))
#if IMAGE_BYTE_ORDER == MSBFirst
#define Store4(l,o,v)  Store8(l,o,((o) & 2 ? \
				   Fetch8(l,o) & 0xf0 | (v) : \
				   Fetch8(l,o) & 0x0f | ((v) << 4)))
#else
#define Store4(l,o,v)  Store8(l,o,((o) & 2 ? \
				   Fetch8(l,o) & 0x0f | ((v) << 4) : \
				   Fetch8(l,o) & 0xf0 | (v)))
#endif

void
fbStore_a4 (FbBits *line, CARD32 offset, CARD32 value)
{
    Store4(line,offset,value>>28);
}

void
fbStore_r1g2b1 (FbBits *line, CARD32 offset, CARD32 value)
{
    CARD32  pixel;
    
    Split(value);
    pixel = (((r >> 4) & 0x8) |
	     ((g >> 5) & 0x6) |
	     ((b >> 7)      ));
    Store4(line,offset,pixel);
}

void
fbStore_b1g2r1 (FbBits *line, CARD32 offset, CARD32 value)
{
    CARD32  pixel;
    
    Split(value);
    pixel = (((b >> 4) & 0x8) |
	     ((g >> 5) & 0x6) |
	     ((r >> 7)      ));
    Store4(line,offset,pixel);
}

void
fbStore_a1r1g1b1 (FbBits *line, CARD32 offset, CARD32 value)
{
    CARD32  pixel;
    Splita(value);
    pixel = (((a >> 4) & 0x8) |
	     ((r >> 5) & 0x4) |
	     ((g >> 6) & 0x2) |
	     ((b >> 7)      ));
    Store4(line,offset,pixel);
}

void
fbStore_a1b1g1r1 (FbBits *line, CARD32 offset, CARD32 value)
{
    CARD32  pixel;
    Splita(value);
    pixel = (((a >> 4) & 0x8) |
	     ((b >> 5) & 0x4) |
	     ((g >> 6) & 0x2) |
	     ((r >> 7)      ));
    Store4(line,offset,pixel);
}

void
fbStore_a1 (FbBits *line, CARD32 offset, CARD32 value)
{
    CARD32  *pixel = ((CARD32 *) line) + (offset >> 5);
    CARD32  mask = FbStipMask(offset & 0x1f, 1);

    value = value & 0x80000000 ? mask : 0;
    *pixel = *pixel & ~mask | value;
}

FbAccessMap fbAccessMap[] = {
    /* 32bpp formats */
    PICT_a8r8g8b8,	fbFetch_a8r8g8b8,	fbFetch_a8r8g8b8,	fbStore_a8r8g8b8,
    PICT_x8r8g8b8,	fbFetch_x8r8g8b8,	fbFetch_x8r8g8b8,	fbStore_x8r8g8b8,
    PICT_a8b8g8r8,	fbFetch_a8b8g8r8,	fbFetch_a8b8g8r8,	fbStore_a8b8g8r8,
    PICT_x8b8g8r8,	fbFetch_x8b8g8r8,	fbFetch_x8b8g8r8,	fbStore_x8b8g8r8,

    /* 24bpp formats */
    PICT_r8g8b8,	fbFetch_r8g8b8,		fbFetch_r8g8b8,		fbStore_r8g8b8,
    PICT_b8g8r8,	fbFetch_b8g8r8,		fbFetch_b8g8r8,		fbStore_b8g8r8,

    /* 16bpp formats */
    PICT_r5g6b5,	fbFetch_r5g6b5,		fbFetch_r5g6b5,		fbStore_r5g6b5,
    PICT_b5g6r5,	fbFetch_b5g6r5,		fbFetch_b5g6r5,		fbStore_b5g6r5,

    PICT_a1r5g5b5,	fbFetch_a1r5g5b5,	fbFetch_a1r5g5b5,	fbStore_a1r5g5b5,
    PICT_x1r5g5b5,	fbFetch_x1r5g5b5,	fbFetch_x1r5g5b5,	fbStore_x1r5g5b5,
    PICT_a1b5g5r5,	fbFetch_a1b5g5r5,	fbFetch_a1b5g5r5,	fbStore_a1b5g5r5,
    PICT_x1b5g5r5,	fbFetch_x1b5g5r5,	fbFetch_x1b5g5r5,	fbStore_x1b5g5r5,

    /* 8bpp formats */
    PICT_a8,		fbFetch_a8,		fbFetcha_a8,		fbStore_a8,
    PICT_r3g3b2,	fbFetch_r3g3b2,		fbFetch_r3g3b2,		fbStore_r3g3b2,
    PICT_b2g3r3,	fbFetch_b2g3r3,		fbFetch_b2g3r3,		fbStore_b2g3r3,
    PICT_a2r2g2b2,	fbFetch_a2r2g2b2,	fbFetch_a2r2g2b2,	fbStore_a2r2g2b2,

    /* 4bpp formats */
    PICT_a4,		fbFetch_a4,		fbFetcha_a4,		fbStore_a4,
    PICT_r1g2b1,	fbFetch_r1g2b1,		fbFetch_r1g2b1,		fbStore_r1g2b1,
    PICT_b1g2r1,	fbFetch_b1g2r1,		fbFetch_b1g2r1,		fbStore_b1g2r1,
    PICT_a1r1g1b1,	fbFetch_a1r1g1b1,	fbFetch_a1r1g1b1,	fbStore_a1r1g1b1,
    PICT_a1b1g1r1,	fbFetch_a1b1g1r1,	fbFetch_a1b1g1r1,	fbStore_a1b1g1r1,

    /* 1bpp formats */
    PICT_a1,		fbFetch_a1,		fbFetcha_a1,		fbStore_a1,
};
#define NumAccessMap (sizeof fbAccessMap / sizeof fbAccessMap[0])

Bool
fbBuildCompositeOperand (PicturePtr	    pPict,
			 FbCompositeOperand *op,
			 INT16		    x,
			 INT16		    y)
{
    int	    i;
    
    for (i = 0; i < NumAccessMap; i++)
	if (fbAccessMap[i].format == pPict->format)
	{
	    op->fetch = fbAccessMap[i].fetch;
	    op->fetcha = fbAccessMap[i].fetcha;
	    op->store = fbAccessMap[i].store;
	    fbGetDrawable (pPict->pDrawable, op->line, op->stride, op->bpp);
	    if (pPict->repeat && pPict->pDrawable->width == 1 && 
		pPict->pDrawable->height == 1)
	    {
		op->bpp = 0;
		op->stride = 0;
	    }
	    op->line = op->line + y * op->stride;
	    op->offset = 0;
	    return TRUE;
	}
    return FALSE;
}

void
fbCompositeGeneral (CARD8	op,
		    PicturePtr	pSrc,
		    PicturePtr	pMask,
		    PicturePtr	pDst,
		    INT16	xSrc,
		    INT16	ySrc,
		    INT16	xMask,
		    INT16	yMask,
		    INT16	xDst,
		    INT16	yDst,
		    CARD16	width,
		    CARD16	height)
{
    FbCompositeOperand	src,msk,dst,*pmsk;
    FbCombineFunc	f;
    int			w;

    if (!fbBuildCompositeOperand (pSrc, &src, xSrc, ySrc))
	return;
    if (!fbBuildCompositeOperand (pDst, &dst, xDst, yDst))
	return;
    f = fbCombineFuncU[op];
    if (pMask)
    {
	if (!fbBuildCompositeOperand (pMask, &msk, xMask, yMask))
	    return;
	pmsk = &msk;
	if (pMask->componentAlpha)
	    f = fbCombineFuncC[op];
    }
    else
	pmsk = 0;
    while (height--)
    {
	w = width;
	src.offset = xSrc * src.bpp;
	dst.offset = xDst * dst.bpp;
	if (pmsk)
	    msk.offset = xMask * msk.bpp;
	while (w--)
	{
	    (*f) (&src, pmsk, &dst);
	    src.offset += src.bpp;
	    dst.offset += dst.bpp;
	    if (pmsk)
		msk.offset += msk.bpp;
	}
	src.line += src.stride;
	dst.line += dst.stride;
	if (pmsk)
	    msk.line += msk.stride;
    }

}

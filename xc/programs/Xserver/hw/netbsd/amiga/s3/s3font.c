/* $XConsortium: s3font.c,v 1.2 94/10/12 20:07:37 kaleb Exp $ */
/* $XFree86: xc/programs/Xserver/hw/xfree86/accel/s3/s3font.c,v 3.3 1995/01/28 17:01:57 dawes Exp $ */
/*
 * Copyright 1992 by Kevin E. Martin, Chapel Hill, North Carolina.
 * 
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Kevin E. Martin not be used in
 * advertising or publicity pertaining to distribution of the software
 * without specific, written prior permission.  Kevin E. Martin makes no
 * representations about the suitability of this software for any purpose. It
 * is provided "as is" without express or implied warranty.
 * 
 * KEVIN E. MARTIN DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL KEVIN E. MARTIN BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF
 * USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 * 
 */

/*
 * Modified by Amancio Hasty and Jon Tombs
 * 
 * Id: s3font.c,v 2.2 1993/06/22 20:54:09 jon Exp jon
 */

/*
 * Modified for the CyberVision 64 by Michael Teske
 */

#include "X.h"
#include "Xmd.h"
#include "Xproto.h"
#include "mfb.h"
#include "fontstruct.h"
#include "dixfontstr.h"
#include "scrnintstr.h"

extern void xf86UnCacheFont8 (FontPtr);

Bool
s3RealizeFont(pScreen, font)
     ScreenPtr pScreen;
     FontPtr font;
{
 /* We _should_ probably be caching things here */
 /* (void)xf86CacheFont8(font); */
   return mfbRealizeFont(pScreen, font);
}

Bool
s3UnrealizeFont(pScreen, font)
     ScreenPtr pScreen;
     FontPtr font;
{
   xf86UnCacheFont8(font);
   return mfbUnrealizeFont(pScreen, font);
}

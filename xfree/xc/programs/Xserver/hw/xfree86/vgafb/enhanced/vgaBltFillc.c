/*
 */

/* $XFree86: xc/programs/Xserver/hw/xfree86/vgafb/enhanced/vgaBltFillc.c,v 1.2 1998/07/25 16:58:33 dawes Exp $ */

#include "X.h"
#include "misc.h"

#include "compiler.h"

#include "xf86.h"
#include "xf86Priv.h"
#include "xf86_OSlib.h"
#include "xf86_HWlib.h"

#include "vga.h"
#include "cfb.h"

/* fBitBlt.s */

void fastBitBltCopy(
    int xdir,
    unsigned char *psrc,
    unsigned char *pdst,
    int h,
    int w,
    int srcPitch,
    int dstPitch
)
{
  if (xdir <= 0) {
    while (h--) {
      xf86memmove(pdst-w, psrc-w, w);
      pdst += dstPitch - w;
      psrc += srcPitch - w;
    }
  } else {
    while (h--) {
      xf86memcpy(pdst, psrc, w);
      pdst += dstPitch + w;
      psrc += srcPitch + w;
    }
  }
}

/* fFillAnd.s */

unsigned char *fastFillSolidGXand(
    unsigned char *pdst,
    unsigned long fill1,
    unsigned long fill2,
    int hcount,
    int count,
    int w,
    int widthPitch
)
{
	/*
	 * NOTES: original assembly code presumes hcount > 0 to start with
	 *	New code assumes that all bytes of fill1, fill2 are
	 *	consistent.  i.e. 0xefefefef, and not 0x12345678.
	 *	This is because the caller of this routine does a PFILL()
	 *	of the [fill1, fill2] values before they get here.
	 *	For large block cases (count > 3), the original code
	 *	assumed that width == count.
	 *	Fills hcount trips of count bytes each trip through loop
	 */

  if (count == 0)
    return pdst;

  while (hcount > 0) {
	/* No special 'fast' cases here */
    int		cur_count;
    unsigned char	tmpb = fill1;
    unsigned short	tmph = fill1;
    unsigned int	tmpi = fill1;

    cur_count = count;

    /* Fiddle with leading bits up to large block */
    if (((long)pdst & 0x1) && cur_count >= 1) {
		/* To next 0mod2 */
	*(unsigned char *) pdst &= tmpb;
	pdst++;
	cur_count--;
    }

    if (((long)pdst & 0x2) && cur_count >= 2) {
		/* To next 0mod4 */
	*(unsigned short *) pdst &= tmph;
	pdst += 2;
	cur_count -= 2;
    }

    if (((long)pdst & 0x4) && cur_count >= 4) {
		/* To next 0mod8 */
	*(unsigned int *) pdst &= tmpi;
	pdst += 4;
	cur_count -= 4;
    }

#if defined(__alpha__)
	/*
	 * Perform bulk copy, knowing 0mod8 alignment
	 * Assumes 64-bit longs.
	 */

    while (cur_count >= 64) {

	/* Hand unrolled x8, assumes scheduler does a good job */
	*(unsigned long *) ((long) pdst + 0 ) &= fill1;
	*(unsigned long *) ((long) pdst + 8 ) &= fill1;
	*(unsigned long *) ((long) pdst + 16) &= fill1;
	*(unsigned long *) ((long) pdst + 24) &= fill1;
	*(unsigned long *) ((long) pdst + 32) &= fill1;
	*(unsigned long *) ((long) pdst + 40) &= fill1;
	*(unsigned long *) ((long) pdst + 48) &= fill1;
	*(unsigned long *) ((long) pdst + 56) &= fill1;

	pdst += 64;
	cur_count -= 64;
    }

	/* Perform trailing bits cleanup */
    while (cur_count >= 8) {
	*(unsigned long *) ((long) pdst + 0) &= fill1;
	pdst += 8;
	cur_count -= 8;
    }
#else
	/*
	 * Perform bulk copy, knowing 0mod4 alignment
	 * Assumes 32-bit longs.
	 */

    while (cur_count >= 32) {

	/* Hand unrolled x8, assumes scheduler does a good job */
	*(unsigned long *) ((long) pdst + 0 ) &= fill1;
	*(unsigned long *) ((long) pdst + 4 ) &= fill1;
	*(unsigned long *) ((long) pdst + 8 ) &= fill1;
	*(unsigned long *) ((long) pdst + 12) &= fill1;
	*(unsigned long *) ((long) pdst + 16) &= fill1;
	*(unsigned long *) ((long) pdst + 20) &= fill1;
	*(unsigned long *) ((long) pdst + 24) &= fill1;
	*(unsigned long *) ((long) pdst + 28) &= fill1;

	pdst += 32;
	cur_count -= 32;
    }

	/* Perform trailing bits cleanup */
    while (cur_count >= 4) {
	*(unsigned long *) ((long) pdst + 0) &= fill1;
	pdst += 4;
	cur_count -= 4;
    }
#endif

    if (cur_count >= 4) {
		/* On 0mod4 boundary already */
	*(unsigned int *) pdst &= tmpi;
	pdst += 4;
	cur_count -= 4;
    }

    if (cur_count >= 2) {
		/* On 0mod2 boundary already */
	*(unsigned short *) pdst &= tmph;
	pdst += 2;
	cur_count -= 2;
    }

    if (cur_count >= 1) {
		/* last possible byte */
	*(unsigned char *) pdst &= tmpb;
	pdst++;
	cur_count--;
    }

	/* Loop epilogue */
/* assert(cur_count == 0); */
    pdst += widthPitch;
    hcount--;
  }

  return pdst;
}

/* fFillOr.s */

unsigned char *fastFillSolidGXor(
    unsigned char *pdst,
    unsigned long fill1,
    unsigned long fill2,
    int hcount,
    int count,
    int w,
    int widthPitch
)
{
	/*
	 * NOTES: original assembly code presumes hcount > 0 to start with
	 *	New code assumes that all bytes of fill1, fill2 are
	 *	consistent.  i.e. 0xefefefef, and not 0x12345678.
	 *	This is because the caller of this routine does a PFILL()
	 *	of the [fill1, fill2] values before they get here.
	 *	For large block cases (count > 3), the original code
	 *	assumed that width == count.
	 *	Fills hcount trips of count bytes each trip through loop
	 */

  if (count == 0)
    return pdst;

  while (hcount > 0) {
	/* No special 'fast' cases here */
    int		cur_count;
    unsigned char	tmpb = fill1;
    unsigned short	tmph = fill1;
    unsigned int	tmpi = fill1;

    cur_count = count;

    /* Fiddle with leading bits up to large block */
    if (((long)pdst & 0x1) && cur_count >= 1) {
		/* To next 0mod2 */
	*(unsigned char *) pdst |= tmpb;
	pdst++;
	cur_count--;
    }

    if (((long)pdst & 0x2) && cur_count >= 2) {
		/* To next 0mod4 */
	*(unsigned short *) pdst |= tmph;
	pdst += 2;
	cur_count -= 2;
    }

    if (((long)pdst & 0x4) && cur_count >= 4) {
		/* To next 0mod8 */
	*(unsigned int *) pdst |= tmpi;
	pdst += 4;
	cur_count -= 4;
    }

#if defined(__alpha__)
	/*
	 * Perform bulk copy, knowing 0mod8 alignment
	 * Assumes 64-bit longs.
	 */

    while (cur_count >= 64) {

	/* Hand unrolled x8, assumes scheduler does a good job */
	*(unsigned long *) ((long) pdst + 0 ) |= fill1;
	*(unsigned long *) ((long) pdst + 8 ) |= fill1;
	*(unsigned long *) ((long) pdst + 16) |= fill1;
	*(unsigned long *) ((long) pdst + 24) |= fill1;
	*(unsigned long *) ((long) pdst + 32) |= fill1;
	*(unsigned long *) ((long) pdst + 40) |= fill1;
	*(unsigned long *) ((long) pdst + 48) |= fill1;
	*(unsigned long *) ((long) pdst + 56) |= fill1;

	pdst += 64;
	cur_count -= 64;
    }

	/* Perform trailing bits cleanup */
    while (cur_count >= 8) {
	*(unsigned long *) ((long) pdst + 0) |= fill1;
	pdst += 8;
	cur_count -= 8;
    }
#else
	/*
	 * Perform bulk copy, knowing 0mod4 alignment
	 * Assumes 32-bit longs.
	 */

    while (cur_count >= 32) {

	/* Hand unrolled x8, assumes scheduler does a good job */
	*(unsigned long *) ((long) pdst + 0 ) |= fill1;
	*(unsigned long *) ((long) pdst + 4 ) |= fill1;
	*(unsigned long *) ((long) pdst + 8 ) |= fill1;
	*(unsigned long *) ((long) pdst + 12) |= fill1;
	*(unsigned long *) ((long) pdst + 16) |= fill1;
	*(unsigned long *) ((long) pdst + 20) |= fill1;
	*(unsigned long *) ((long) pdst + 24) |= fill1;
	*(unsigned long *) ((long) pdst + 28) |= fill1;

	pdst += 32;
	cur_count -= 32;
    }

	/* Perform trailing bits cleanup */
    while (cur_count >= 4) {
	*(unsigned long *) ((long) pdst + 0) |= fill1;
	pdst += 4;
	cur_count -= 4;
    }
#endif

    if (cur_count >= 4) {
		/* On 0mod4 boundary already */
	*(unsigned int *) pdst |= tmpi;
	pdst += 4;
	cur_count -= 4;
    }

    if (cur_count >= 2) {
		/* On 0mod2 boundary already */
	*(unsigned short *) pdst |= tmph;
	pdst += 2;
	cur_count -= 2;
    }

    if (cur_count >= 1) {
		/* last possible byte */
	*(unsigned char *) pdst |= tmpb;
	pdst++;
	cur_count--;
    }

	/* Loop epilogue */
/* assert(cur_count == 0); */
    pdst += widthPitch;
    hcount--;
  }

  return pdst;
}

/* fFillXor.s */

unsigned char *fastFillSolidGXxor(
    unsigned char *pdst,
    unsigned long fill1,
    unsigned long fill2,
    int hcount,
    int count,
    int w,
    int widthPitch
)
{
	/*
	 * NOTES: original assembly code presumes hcount > 0 to start with
	 *	New code assumes that all bytes of fill1, fill2 are
	 *	consistent.  i.e. 0xefefefef, and not 0x12345678.
	 *	This is because the caller of this routine does a PFILL()
	 *	of the [fill1, fill2] values before they get here.
	 *	For large block cases (count > 3), the original code
	 *	assumed that width == count.
	 *	Fills hcount trips of count bytes each trip through loop
	 */

  if (count == 0)
    return pdst;

  while (hcount > 0) {
	/* No special 'fast' cases here */
    int		cur_count;
    unsigned char	tmpb = fill1;
    unsigned short	tmph = fill1;
    unsigned int	tmpi = fill1;

    cur_count = count;

    /* Fiddle with leading bits up to large block */
    if (((long)pdst & 0x1) && cur_count >= 1) {
		/* To next 0mod2 */
	*(unsigned char *) pdst ^= tmpb;
	pdst++;
	cur_count--;
    }

    if (((long)pdst & 0x2) && cur_count >= 2) {
		/* To next 0mod4 */
	*(unsigned short *) pdst ^= tmph;
	pdst += 2;
	cur_count -= 2;
    }

    if (((long)pdst & 0x4) && cur_count >= 4) {
		/* To next 0mod8 */
	*(unsigned int *) pdst ^= tmpi;
	pdst += 4;
	cur_count -= 4;
    }

#if defined(__alpha__)
	/*
	 * Perform bulk copy, knowing 0mod8 alignment
	 * Assumes 64-bit longs.
	 */

    while (cur_count >= 64) {

	/* Hand unrolled x8, assumes scheduler does a good job */
	*(unsigned long *) ((long) pdst + 0 ) ^= fill1;
	*(unsigned long *) ((long) pdst + 8 ) ^= fill1;
	*(unsigned long *) ((long) pdst + 16) ^= fill1;
	*(unsigned long *) ((long) pdst + 24) ^= fill1;
	*(unsigned long *) ((long) pdst + 32) ^= fill1;
	*(unsigned long *) ((long) pdst + 40) ^= fill1;
	*(unsigned long *) ((long) pdst + 48) ^= fill1;
	*(unsigned long *) ((long) pdst + 56) ^= fill1;

	pdst += 64;
	cur_count -= 64;
    }

	/* Perform trailing bits cleanup */
    while (cur_count >= 8) {
	*(unsigned long *) ((long) pdst + 0) ^= fill1;
	pdst += 8;
	cur_count -= 8;
    }
#else
	/*
	 * Perform bulk copy, knowing 0mod4 alignment
	 * Assumes 32-bit longs.
	 */

    while (cur_count >= 32) {

	/* Hand unrolled x8, assumes scheduler does a good job */
	*(unsigned long *) ((long) pdst + 0 ) ^= fill1;
	*(unsigned long *) ((long) pdst + 4 ) ^= fill1;
	*(unsigned long *) ((long) pdst + 8 ) ^= fill1;
	*(unsigned long *) ((long) pdst + 12) ^= fill1;
	*(unsigned long *) ((long) pdst + 16) ^= fill1;
	*(unsigned long *) ((long) pdst + 20) ^= fill1;
	*(unsigned long *) ((long) pdst + 24) ^= fill1;
	*(unsigned long *) ((long) pdst + 28) ^= fill1;

	pdst += 32;
	cur_count -= 32;
    }

	/* Perform trailing bits cleanup */
    while (cur_count >= 4) {
	*(unsigned long *) ((long) pdst + 0) ^= fill1;
	pdst += 4;
	cur_count -= 4;
    }
#endif

    if (cur_count >= 4) {
		/* On 0mod4 boundary already */
	*(unsigned int *) pdst ^= tmpi;
	pdst += 4;
	cur_count -= 4;
    }

    if (cur_count >= 2) {
		/* On 0mod2 boundary already */
	*(unsigned short *) pdst ^= tmph;
	pdst += 2;
	cur_count -= 2;
    }

    if (cur_count >= 1) {
		/* last possible byte */
	*(unsigned char *) pdst ^= tmpb;
	pdst++;
	cur_count--;
    }

	/* Loop epilogue */
/* assert(cur_count == 0); */
    pdst += widthPitch;
    hcount--;
  }

  return pdst;
}

/* fFillCopy.s */

unsigned char *fastFillSolidGXcopy(
    unsigned char *pdst,
    unsigned long fill1,
    unsigned long fill2,
    int hcount,
    int count,
    int w,
    int widthPitch
)
{
	/*
	 * NOTES: original assembly code presumes hcount > 0 to start with
	 *	New code assumes that all bytes of fill1, fill2 are
	 *	consistent.  i.e. 0xefefefef, and not 0x12345678.
	 *	This is because the caller of this routine does a PFILL()
	 *	of the [fill1, fill2] values before they get here.
	 *	For large block cases (count > 3), the original code
	 *	assumed that width == count.
	 *	Fills hcount trips of count bytes each trip through loop
	 */

  if (count == 0)
    return pdst;

  while (hcount > 0) {
	/* No special 'fast' cases here */
    int		cur_count;
    unsigned char	tmpb = fill1;
    unsigned short	tmph = fill1;
    unsigned int	tmpi = fill1;

    cur_count = count;

    /* Fiddle with leading bits up to large block */
    if (((long)pdst & 0x1) && cur_count >= 1) {
		/* To next 0mod2 */
	*(unsigned char *) pdst = tmpb;
	pdst++;
	cur_count--;
    }

    if (((long)pdst & 0x2) && cur_count >= 2) {
		/* To next 0mod4 */
	*(unsigned short *) pdst = tmph;
	pdst += 2;
	cur_count -= 2;
    }

    if (((long)pdst & 0x4) && cur_count >= 4) {
		/* To next 0mod8 */
	*(unsigned int *) pdst = tmpi;
	pdst += 4;
	cur_count -= 4;
    }

#if defined(__alpha__)
	/*
	 * Perform bulk copy, knowing 0mod8 alignment
	 * Assumes 64-bit longs.
	 */

    while (cur_count >= 64) {

	/* Hand unrolled x8, assumes scheduler does a good job */
	*(unsigned long *) ((long) pdst + 0 ) = fill1;
	*(unsigned long *) ((long) pdst + 8 ) = fill1;
	*(unsigned long *) ((long) pdst + 16) = fill1;
	*(unsigned long *) ((long) pdst + 24) = fill1;
	*(unsigned long *) ((long) pdst + 32) = fill1;
	*(unsigned long *) ((long) pdst + 40) = fill1;
	*(unsigned long *) ((long) pdst + 48) = fill1;
	*(unsigned long *) ((long) pdst + 56) = fill1;

	pdst += 64;
	cur_count -= 64;
    }

	/* Perform trailing bits cleanup */
    while (cur_count >= 8) {
	*(unsigned long *) ((long) pdst + 0) = fill1;
	pdst += 8;
	cur_count -= 8;
    }
#else
	/*
	 * Perform bulk copy, knowing 0mod4 alignment
	 * Assumes 32-bit longs.
	 */

    while (cur_count >= 32) {

	/* Hand unrolled x8, assumes scheduler does a good job */
	*(unsigned long *) ((long) pdst + 0 ) = fill1;
	*(unsigned long *) ((long) pdst + 4 ) = fill1;
	*(unsigned long *) ((long) pdst + 8 ) = fill1;
	*(unsigned long *) ((long) pdst + 12) = fill1;
	*(unsigned long *) ((long) pdst + 16) = fill1;
	*(unsigned long *) ((long) pdst + 20) = fill1;
	*(unsigned long *) ((long) pdst + 24) = fill1;
	*(unsigned long *) ((long) pdst + 28) = fill1;

	pdst += 32;
	cur_count -= 32;
    }
	/* Perform trailing bits cleanup */
    while (cur_count >= 4) {
	*(unsigned long *) ((long) pdst + 0) = fill1;
	pdst += 4;
	cur_count -= 4;
    }
#endif

    if (cur_count >= 4) {
		/* On 0mod4 boundary already */
	*(unsigned int *) pdst = tmpi;
	pdst += 4;
	cur_count -= 4;
    }

    if (cur_count >= 2) {
		/* On 0mod2 boundary already */
	*(unsigned short *) pdst = tmph;
	pdst += 2;
	cur_count -= 2;
    }

    if (cur_count >= 1) {
		/* last possible byte */
	*(unsigned char *) pdst = tmpb;
	pdst++;
	cur_count--;
    }

	/* Loop epilogue */
/* assert(cur_count == 0); */
    pdst += widthPitch;
    hcount--;
  }

  return pdst;
}

/*
 * Reverse engineered version of XFree86 code for fFillSet.s by
 * Rick Gorton (gorton@tallis.enet.dec.com)
 * This version should work well on strongly aligned RISC architectures
 * in general.  In particular, the even-odd trip performance problem
 * with 'tribbleloop' is eliminated.
 *
 * Jay, please put the original header back in here...
 */

unsigned char *fastFillSolidGXset(
    unsigned char *pdst,
    unsigned long fill1,
    unsigned long fill2,
    int hcount,
    int count,
    int w,
    int widthPitch)
{
	/*
	 * NOTES: original assembly code presumes hcount > 0 to start with
	 *	New code assumes that all bytes of fill1, fill2 are
	 *	consistent.  i.e. 0xefefefef, and not 0x12345678.
	 *	This is because the caller of this routine does a PFILL()
	 *	of the [fill1, fill2] values before they get here.
	 *	For large block cases (count > 3), the original code
	 *	assumed that width == count.
	 *	Fills hcount trips of count bytes each trip through loop
	 */

  if (count == 0)
    return pdst;

  while (hcount > 0) {
	/* No special 'fast' cases here */
    int		cur_count;
    char	tmpb;
    short	tmph;
    int		tmpi;

    cur_count = count;

    /* Fiddle with leading bits up to large block */
    if (((long)pdst & 0x1) && cur_count >= 1) {
		/* To next 0mod2 */
	tmpb = *(unsigned char *) pdst;
	tmpb = (tmpb & fill1) ^ fill2;
	*(unsigned char *) pdst = tmpb;
	pdst++;
	cur_count--;
    }

    if (((long)pdst & 0x2) && cur_count >= 2) {
		/* To next 0mod4 */
	tmph = *(unsigned short *) pdst;
	tmph = (tmph & fill1) ^ fill2;
	*(unsigned short *) pdst = tmph;
	pdst += 2;
	cur_count -= 2;
    }

    if (((long)pdst & 0x4) && cur_count >= 4) {
		/* To next 0mod8 */
	tmpi = *(unsigned int *) pdst;
	tmpi = (tmpi & fill1) ^ fill2;
	*(unsigned int *) pdst = tmpi;
	pdst += 4;
	cur_count -= 4;
    }

#if defined(__alpha__)
	/*
	 * Perform bulk copy, knowing 0mod8 alignment
	 * Assumes 64-bit longs.
	 */

    while (cur_count >= 64) {
	unsigned long	tmp_1, tmp_2, tmp_3, tmp_4;
	unsigned long	tmp_5, tmp_6, tmp_7, tmp_8;

	/* Hand unrolled x8, assumes scheduler does a good job */
	tmp_1 = *(unsigned long *) ((long) pdst + 0);
	tmp_2 = *(unsigned long *) ((long) pdst + 8);
	tmp_3 = *(unsigned long *) ((long) pdst + 16);
	tmp_4 = *(unsigned long *) ((long) pdst + 24);
	tmp_5 = *(unsigned long *) ((long) pdst + 32);
	tmp_6 = *(unsigned long *) ((long) pdst + 40);
	tmp_7 = *(unsigned long *) ((long) pdst + 48);
	tmp_8 = *(unsigned long *) ((long) pdst + 56);

	tmp_1 = (fill1 & tmp_1) ^ fill2;
	tmp_2 = (fill1 & tmp_2) ^ fill2;
	tmp_3 = (fill1 & tmp_3) ^ fill2;
	tmp_4 = (fill1 & tmp_4) ^ fill2;
	tmp_5 = (fill1 & tmp_5) ^ fill2;
	tmp_6 = (fill1 & tmp_6) ^ fill2;
	tmp_7 = (fill1 & tmp_7) ^ fill2;
	tmp_8 = (fill1 & tmp_8) ^ fill2;

	*(unsigned long *) ((long) pdst + 0) = tmp_1;
	*(unsigned long *) ((long) pdst + 8) = tmp_2;
	*(unsigned long *) ((long) pdst + 16) = tmp_3;
	*(unsigned long *) ((long) pdst + 24) = tmp_4;
	*(unsigned long *) ((long) pdst + 32) = tmp_5;
	*(unsigned long *) ((long) pdst + 40) = tmp_6;
	*(unsigned long *) ((long) pdst + 48) = tmp_7;
	*(unsigned long *) ((long) pdst + 56) = tmp_8;

	pdst += 64;
	cur_count -= 64;
    }

	/* Perform trailing bits cleanup */
    while (cur_count >= 8) {
	unsigned long	tmpl;

	tmpl = *(unsigned long *) ((long) pdst + 0);
	tmpl = (tmpl & fill1) ^ fill2;
	*(unsigned long *) ((long) pdst + 0) = tmpl;
	pdst += 8;
	cur_count -= 8;
    }
#else
	/*
	 * Perform bulk copy, knowing 0mod4 alignment
	 * Assumes 32-bit longs.
	 */

    while (cur_count >= 32) {
	unsigned long	tmp_1, tmp_2, tmp_3, tmp_4;
	unsigned long	tmp_5, tmp_6, tmp_7, tmp_8;

	/* Hand unrolled x8, assumes scheduler does a good job */
	tmp_1 = *(unsigned long *) ((long) pdst + 0);
	tmp_2 = *(unsigned long *) ((long) pdst + 4);
	tmp_3 = *(unsigned long *) ((long) pdst + 8 );
	tmp_4 = *(unsigned long *) ((long) pdst + 12);
	tmp_5 = *(unsigned long *) ((long) pdst + 16);
	tmp_6 = *(unsigned long *) ((long) pdst + 20);
	tmp_7 = *(unsigned long *) ((long) pdst + 24);
	tmp_8 = *(unsigned long *) ((long) pdst + 28);

	tmp_1 = (fill1 & tmp_1) ^ fill2;
	tmp_2 = (fill1 & tmp_2) ^ fill2;
	tmp_3 = (fill1 & tmp_3) ^ fill2;
	tmp_4 = (fill1 & tmp_4) ^ fill2;
	tmp_5 = (fill1 & tmp_5) ^ fill2;
	tmp_6 = (fill1 & tmp_6) ^ fill2;
	tmp_7 = (fill1 & tmp_7) ^ fill2;
	tmp_8 = (fill1 & tmp_8) ^ fill2;

	*(unsigned long *) ((long) pdst + 0) = tmp_1;
	*(unsigned long *) ((long) pdst + 4) = tmp_2;
	*(unsigned long *) ((long) pdst + 8 ) = tmp_3;
	*(unsigned long *) ((long) pdst + 12) = tmp_4;
	*(unsigned long *) ((long) pdst + 16) = tmp_5;
	*(unsigned long *) ((long) pdst + 20) = tmp_6;
	*(unsigned long *) ((long) pdst + 24) = tmp_7;
	*(unsigned long *) ((long) pdst + 28) = tmp_8;

	pdst += 32;
	cur_count -= 32;
    }
	/* Perform trailing bits cleanup */
    while (cur_count >= 4) {
	unsigned long	tmpl;

	tmpl = *(unsigned long *) ((long) pdst + 0);
	tmpl = (tmpl & fill1) ^ fill2;
	*(unsigned long *) ((long) pdst + 0) = tmpl;
	pdst += 4;
	cur_count -= 4;
    }
#endif

    if (cur_count >= 4) {
		/* On 0mod4 boundary already */
	tmpi = *(unsigned int *) pdst;
	tmpi = (tmpi & fill1) ^ fill2;
	*(unsigned int *) pdst = tmpi;
	pdst += 4;
	cur_count -= 4;
    }

    if (cur_count >= 2) {
		/* On 0mod2 boundary already */
	tmph = *(unsigned short *) pdst;
	tmph = (tmph & fill1) ^ fill2;
	*(unsigned short *) pdst = tmph;
	pdst += 2;
	cur_count -= 2;
    }

    if (cur_count >= 1) {
		/* last possible byte */
	tmpb = *(unsigned char *) pdst;
	tmpb = (tmpb & fill1) ^ fill2;
	*(unsigned char *) pdst = tmpb;
	pdst++;
	cur_count--;
    }

	/* Loop epilogue */
/* assert(cur_count == 0); */
    pdst += widthPitch;
    hcount--;
  }

  return pdst;
}


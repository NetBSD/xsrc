/*	$NetBSD: px.h,v 1.3 2008/04/28 20:57:37 martin Exp $	*/

/*-
 * Copyright (c) 2001, 2002 The NetBSD Foundation, Inc.
 * All rights reserved.
 *
 * This code is derived from software contributed to The NetBSD Foundation
 * by Andrew Doran.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE NETBSD FOUNDATION, INC. AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <sys/param.h>
#include <sys/types.h>
#include <sys/time.h>

#include <dev/wscons/wsconsio.h>

#include <dev/tc/sticreg.h>
#include <dev/tc/sticio.h>

#include "X.h"
#include "gcstruct.h"
#include "misc.h"
#include "window.h"
#include "windowstr.h"

#define	PSZ	32

#define	PX_DEBUG
/*
#define	PX_STATS
*/

#ifdef PX_DEBUG
#define	PX_DPRINTF(x)	ErrorF x
#else
#define	PX_DPRINTF(x)
#endif

#if 0
#define	PX_TRACE(x)	ErrorF("TRACE: %s()\n", x)
#else
#define	PX_TRACE(x)
#endif

#define	PX_DUPBYTE(x)	((((x)&0xff)<<16) | (((x)&0xff)<<8) | ((x)&0xff))

#ifdef __alpha__
#define px_mb()    __asm__ __volatile__("mb" : : : "memory")
#elif defined __mips__
/* XXX */
#define	px_mb()
#else
#error No support for your architecture
#endif

/* ============================================================== */

extern int	pxGCPrivateIndex;
extern int	pxWindowPrivateIndex;
extern int	pxScreenPrivateIndex;

#define pxGetGCPrivate(g) \
	((pxPrivGCPtr)(g)->devPrivates[pxGCPrivateIndex].ptr)
#define pxGetScreenPrivate(g) \
	((pxPrivScreenPtr)(g)->devPrivates[pxScreenPrivateIndex].ptr)

#define pxGetWindowPrivate(g) \
	((pxPrivWinPtr)(g)->devPrivates[pxWindowPrivateIndex].ptr)

typedef struct {
	u_int32_t	fg;
	u_int32_t	bg;
	u_int16_t	data[16];
} pxMaskRec, *pxMaskPtr;

typedef struct {
	u_int32_t	*ptr;
	u_int		paddr;
	int		bufnum;
	int		ident;
} pxImgBuf, *pxImgBufPtr;

typedef struct pxScreenPriv {
	int		stampw;
	int		stamph;
	int		stamphm;
	int		bpp;
	int		realbpp;
	int		queueing;
	int		pbsel;
	int		ibsel;
	int		nbuf;
	pxImgBuf	ib[2];
	u_int32_t	*buf;
	u_int		buf_pa;

	volatile struct stic_xcomm *sxc;
	volatile struct	stic_regs *stic;
	volatile u_int32_t	*poll;

	void		(*expandBuf)(void *, void *, int);
	void		*(*compressBuf)(void *, void *, int);
	void		(*tileBuf)(void *, void *, int, int, int);

	int		fd;
	int		mapfd;
	size_t		maplen;
	Bool		(*CloseScreen)(int, ScreenPtr);
} pxScreenPriv, *pxScreenPrivPtr;

typedef struct pxPrivGC {
	u_int32_t	umet;
	int		pmask;
	u_int		fgPixel;
	u_int		bgPixel;
	u_int		fgFill;
	int		type;
	int		fillStyle;
	void		(*doFillSpans)(DrawablePtr, GCPtr, struct pxPrivGC *,
				       BoxPtr, int, DDXPointPtr, int *);
	pxMaskRec	mask;
	pxScreenPrivPtr	sp;
} pxPrivGCRec, *pxPrivGCPtr;

typedef struct {
	Bool		haveMask;
	pxMaskRec	mask;
} pxPrivWinRec, *pxPrivWinPtr;

/* ============================================================== */

typedef struct {
	u_int32_t	*head;
	u_int32_t	*curpb;
	int		hdrsz;
	int		primcnt;
	int		primsz;
	int		maxprimcnt;
	int		bufnum;
	int		ident;
} pxPacket, *pxPacketPtr;

u_int32_t	*pxPacketStart(pxScreenPrivPtr, pxPacketPtr, int, int);
static u_int32_t	*pxPacketAddPrim(pxScreenPrivPtr, pxPacketPtr);
void		pxPacketAddPrim0(pxScreenPrivPtr, pxPacketPtr);
void		pxPacketFlush(pxScreenPrivPtr, pxPacketPtr);
void		pxPacketFlushWait(pxScreenPrivPtr, pxPacketPtr);
void		pxPacketWait(pxScreenPrivPtr, pxPacketPtr);

static __inline__ u_int32_t *
pxPacketAddPrim(pxScreenPrivPtr sp, pxPacketPtr pp)
{
	u_int32_t *npb;

	if (pp->primcnt == pp->maxprimcnt)
		pxPacketAddPrim0(sp, pp);

	pp->primcnt++;
	npb = pp->curpb;
	pp->curpb += pp->primsz;
	return (npb);
}

static __inline__ pxImgBufPtr
pxAllocImgBuf(pxScreenPrivPtr sp)
{
	pxImgBufPtr ib;
	volatile struct stic_xcomm *sxc;

	if (!sp->queueing)
		return (&sp->ib[sp->ibsel ^= 1]);

	sxc = sp->sxc;
	for (;;) {
		/* XXX Potential for stall? */
		ib = &sp->ib[0];
		if (sxc->sxc_done[ib->bufnum] != ib->ident)
			break;

		ib = &sp->ib[1];
		if (sxc->sxc_done[ib->bufnum] != ib->ident)
			break;
	}
	return (ib);
}

static __inline__ void
pxAssociateImgBuf(pxScreenPrivPtr sp, pxImgBufPtr ib, pxPacketPtr pp)
{

	ib->bufnum = pp->bufnum;
	ib->ident = pp->ident;
}

static __inline__ u_int32_t
pxVirtToStic(pxScreenPrivPtr sp, u_int32_t *p)
{
	u_int32_t v;

	v = (u_long)p - (u_long)sp->sxc + sp->buf_pa;
	v = ((v & 0x00600000) << 6) | ((v & 0x001f8000) << 3) | (v & 0x7fff);
	return (v);
}

static __inline__ void
pxAddLine(pxScreenPrivPtr sp, pxPacketPtr pp, int x1, int y1,
	  int x2, int y2)
{
	u_int32_t *pb;

	pb = pxPacketAddPrim(sp, pp);
	pb[0] = (x1 << 19) | (y1 << 3);
	pb[1] = (x2 << 19) | (y2 << 3);
}

static __inline__ void
pxAddLineC(pxScreenPrivPtr sp, pxPacketPtr pp, int x1, int y1, int x2,
	   int y2, int c)
{
	u_int32_t *pb;

	pb = pxPacketAddPrim(sp, pp);
	pb[0] = (x1 << 19) | (y1 << 3);
	pb[1] = (x2 << 19) | (y2 << 3);
	pb[2] = c;
}

/* ============================================================== */

/*
 * pxgc.c
 */
extern const int	pxRopTable[16];

Bool	pxCreateGC(GCPtr);

/*
 * pxwindow.c
 */
Bool	pxCreateWindow(WindowPtr);
Bool	pxDestroyWindow(WindowPtr);
Bool	pxPositionWindow(WindowPtr, int, int);
Bool	pxChangeWindowAttributes(WindowPtr, unsigned long);
void 	pxCopyWindow(WindowPtr, DDXPointRec, RegionPtr);
void	pxPaintWindowBackground(WindowPtr, RegionPtr, int);
void	pxPaintWindowBorder(WindowPtr, RegionPtr, int);

/*
 * pxmisc.c
 */
void	pxFillBoxSolid(pxScreenPrivPtr, RegionPtr, u_int);
void	pxFillBoxTiled(pxScreenPrivPtr, RegionPtr, pxMaskPtr);
Bool	pxMaskFromStipple(pxScreenPrivPtr, PixmapPtr, pxMaskPtr, int, int);
Bool	pxMaskFromTile(pxScreenPrivPtr, PixmapPtr, pxMaskPtr);
void	pxTileBuf24r24(void *, void *, int, int, int);
void	pxTileBuf8r24(void *, void *, int, int, int);
void	pxTileBuf8r8(void *, void *, int, int, int);
void	pxStippleBuf(PixmapPtr, u_int32_t *, int, int, u_int32_t, u_int32_t,
		     int, int);
void	pxStippleBufOpaque(PixmapPtr, u_int32_t *, int, int, u_int32_t,
			   u_int32_t, int, int);
void	*pxCompressBuf24to24(void *, void *, int);
void	*pxCompressBuf24to8(void *, void *, int);
void	pxExpandBuf24to24(void *, void *, int);
void	pxExpandBuf8to8(void *, void *, int);
void	pxExpandBuf8to24(void *, void *, int);

/*
 * pxbitblt.c
 */
RegionPtr	pxCopyArea(DrawablePtr, DrawablePtr, GCPtr, int, int, int,
			   int, int, int);
void	pxDoBitblt(DrawablePtr, DrawablePtr, int, RegionPtr,
		   DDXPointPtr, unsigned long);

/*
 * pxfillrct.c
 */
void	pxPolyFillRect(DrawablePtr, GCPtr, int, xRectangle *);
void	pxPolyFillRectS(DrawablePtr, GCPtr, int, xRectangle *);
void	pxPolyFillRectSO(DrawablePtr, GCPtr, int, xRectangle *);

/*
 * pxline.c
 */
void	pxPolySegment(DrawablePtr, GCPtr, int, xSegment *);
void	pxPolylines(DrawablePtr, GCPtr, int, int, DDXPointPtr);
void	pxPolySegmentD(DrawablePtr, GCPtr, int, xSegment *);
void	pxPolylinesD(DrawablePtr, GCPtr, int, int, DDXPointPtr);

/*
 * pxbresd.c
 */
void    pxBresD(pxScreenPrivPtr, pxPacketPtr, pxPrivGCPtr, int *,
		unsigned char *, int, int *, int, int, int, int, int, int,
		int, int, int, int);

/*
 * pxpolypnt.c
 */
void	pxPolyPoint(DrawablePtr, GCPtr, int, int, xPoint *);

/*
 * pxpushpxl.c
 */
void	pxSolidPP(GCPtr, PixmapPtr, DrawablePtr, int, int, int, int);
void	pxSqueege(pxScreenPrivPtr, pxPacketPtr, u_int8_t *, int,
		  int, int, int, int, int, int, int);
void	pxSqueege16(pxScreenPrivPtr, pxPacketPtr, u_int8_t *, int,
		    int, int, int, int, int, int, int);

/*
 * pxsetsp.c
 */
void	pxSetScanline(pxScreenPrivPtr, int, int, int, int, int *, int,
		      unsigned long);
void	pxSetScanlineRaw(pxScreenPrivPtr, int, int, int, int, unsigned long,
			 pxImgBufPtr);
void	pxSetSpans(DrawablePtr, GCPtr, char *, DDXPointPtr ppt, int *,
		   int, int);

/*
 * pxfillsp.c
 */
void	pxFillSpans(DrawablePtr, GCPtr, int, DDXPointPtr, int *, int);

void	pxDoFillSpans(DrawablePtr, GCPtr, pxPrivGCPtr,
		      BoxPtr, int, DDXPointPtr, int *);
void	pxDoFillSpansS(DrawablePtr, GCPtr, pxPrivGCPtr,
		       BoxPtr, int, DDXPointPtr, int *);
void	pxDoFillSpansT(DrawablePtr, GCPtr, pxPrivGCPtr,
		       BoxPtr, int, DDXPointPtr, int *);
void	pxDoFillSpansUS(DrawablePtr, GCPtr, pxPrivGCPtr,
		        BoxPtr, int, DDXPointPtr, int *);

/*
 * pxgetsp.c
 */
void	pxGetSpans(DrawablePtr, int, DDXPointPtr, int *, int, char *);
void	pxGetScanlineRaw(pxScreenPrivPtr, int, int, int, pxImgBufPtr);

/*
 * pxglyph.c
 */
void	pxPolyTEGlyphBlt(DrawablePtr, GCPtr, int, int, unsigned int,
			 CharInfoPtr *, pointer);
void	pxImageTEGlyphBlt(DrawablePtr, GCPtr, int, int, unsigned int,
			  CharInfoPtr *, pointer);

void	pxPolyGlyphBlt(DrawablePtr, GCPtr, int, int, unsigned int,
		       CharInfoPtr *, pointer);
void	pxImageGlyphBlt(DrawablePtr, GCPtr, int, int, unsigned int,
			CharInfoPtr *, pointer);

void	pxSlowPolyGlyphBlt(DrawablePtr, GCPtr, int, int, unsigned int,
		       CharInfoPtr *, pointer);
void	pxSlowImageGlyphBlt(DrawablePtr, GCPtr, int, int, unsigned int,
			CharInfoPtr *, pointer);

/*
 * pximage.c
 */
void	pxGetImage(DrawablePtr, int, int, int, int, unsigned int,
		   unsigned long, char *);

/*
 * pxzerarc.c
 */
void	pxZeroPolyArc(DrawablePtr, GCPtr, int, xArc *);

/*
 * pxfillarc.c
 */
void	pxPolyFillArc(DrawablePtr, GCPtr, int, xArc *);

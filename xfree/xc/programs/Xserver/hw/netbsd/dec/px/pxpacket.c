/*	$NetBSD: pxpacket.c,v 1.2 2008/04/28 20:57:37 martin Exp $	*/

/*-
 * Copyright (c) 2001 The NetBSD Foundation, Inc.
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

#include "px.h"

#include <sys/ioctl.h>
#include <errno.h>

static int	idgen = 1;

Bool	pxPacketTryGlom(pxScreenPrivPtr, pxPacketPtr, u_int32_t *, int);
void	pxPacketReset(pxScreenPrivPtr, const char *);
int	pxPacketTailDistance(pxScreenPrivPtr, int);

static __inline__ volatile u_int32_t *pxPollAddr(pxScreenPrivPtr, u_int32_t *);

static __inline__ volatile u_int32_t *
pxPollAddr(pxScreenPrivPtr sp, u_int32_t *pb)
{
	u_long v;

	v = (u_long)pb - (u_long)sp->sxc + sp->buf_pa;
	v = ((v & 0xffff8000) << 3) | (v & 0x7fff);
	return ((volatile u_int32_t *)((u_long)sp->poll + (v >> 9)));
}

u_int32_t *
pxPacketStart(pxScreenPrivPtr sp, pxPacketPtr pp, int hdrsz, int primsz)
{
	u_int32_t *pb;
	int sel;

	PX_TRACE("pxPacketStart");

	if (!sp->queueing)
		sel = (sp->pbsel ^= 1);
	else {
		/* Allocate a buffer and assign it a unique ID. */
		pp->ident = (idgen += 2);
		pp->bufnum = sp->sxc->sxc_head;
		sel = pp->bufnum;
		sp->sxc->sxc_done[sel] = pp->ident;
	}

	pb = sp->buf + (sel * STIC_PACKET_SIZE) / sizeof(u_int32_t);

	pp->head = pb;
	pp->curpb = pb + hdrsz;
	pp->hdrsz = hdrsz;
	pp->primsz = primsz;
	pp->primcnt = 0;
	pp->maxprimcnt = min(255, (1024 - hdrsz) / primsz);

	return (pb);
}

void
pxPacketAddPrim0(pxScreenPrivPtr sp, pxPacketPtr pp)
{
	u_int32_t *pb;
	int sel;

	PX_TRACE("pxPacketAddPrim0");

	if (!sp->queueing)
		sel = (sp->pbsel ^= 1);
	else {
		/* Allocate a buffer and assign it a unique ID. */
		pp->ident = (idgen += 2);
		pp->bufnum = sp->sxc->sxc_head;
		sel = pp->bufnum;
		sp->sxc->sxc_done[sel] = pp->ident;
	}

	pb = sp->buf + sel * (STIC_PACKET_SIZE / sizeof(u_int32_t));
	memcpy(pb, pp->head, pp->hdrsz * sizeof(u_int32_t));

	pxPacketFlush(sp, pp);

	pp->head = pb;
	pp->curpb = pb + pp->hdrsz;
	pp->primcnt = 0;
	pb[1] &= 0x00ffffff;
}

int
pxPacketTailDistance(pxScreenPrivPtr sp, int pos)
{

	if ((pos -= sp->sxc->sxc_tail) < 0)
		pos += sp->nbuf;
	return (pos);
}

Bool
pxPacketTryGlom(pxScreenPrivPtr sp, pxPacketPtr pp, u_int32_t *opb, int opos)
{
	u_int32_t *maxpb, *pb, *dp;
	u_int oprimcnt, i;

	pb = pp->head;

	/* Command words match? */
	if (pb[0] != opb[0])
		return (FALSE);

	/*
	 * Is there enough space for the combined primatives?  (Note that we
	 * could be clever and only transfer how many will actually fit, but
	 * it's not likely to be a big win.)
	 */
	oprimcnt = (opb[1] >> 24);
	if (pp->primcnt + oprimcnt > pp->maxprimcnt)
		return (FALSE);

	/* Plane masks match? */
	if ((pb[1] & 0x00ffffff) != (opb[1] & 0x00ffffff))
		return (FALSE);

	/* Additional packet header matches? */	
	for (i = 3; i < pp->hdrsz; i++)
		if (pb[i] != opb[i])
			return (FALSE);

	/*
	 * All the checks have passed; copy the primatives across to the old
	 * packet.
	 */
	pb += pp->hdrsz;
	dp = opb + pp->hdrsz + oprimcnt * pp->primsz;
	maxpb = pp->curpb;

	while (pb < maxpb)
		*dp++ = *pb++;

	/*
	 * If the queue tail pointer is still at least two away from the old
	 * packet, then finalise and delcare success.
	 */
	if (pxPacketTailDistance(sp, opos) < 2)
		return (FALSE);

	opb[1] += (pp->primcnt << 24);
	return (TRUE);
}

void
pxPacketFlush(pxScreenPrivPtr sp, pxPacketPtr pp)
{
	volatile u_int32_t *poll;
	volatile struct stic_xcomm *sxc;
	volatile struct stic_regs *sr;
	volatile u_int32_t v;
	u_int32_t *pb;
	u_int i, j;

	PX_TRACE("pxPacketFlush");

	pb = pp->head;

	if (pp->primcnt != 0)
		pb[1] = (pb[1] & 0xffffff) | pp->primcnt << 24;
	if ((pb[1] & 0xff000000) == 0) {
		if (!sp->queueing)
			sp->pbsel ^= 1;
		return;
	}

	sxc = sp->sxc;
	sr = sp->stic;

	if (sp->queueing) {
#ifdef notyet
		/*
		 * See if we can "glom" the primatives from this packet
		 * together with one already in the queue.  Only do this if
		 * there are at least 8 packets in the queue (one currently
		 * executing).
		 */
		i = sxc->sxc_head;
		while (pxPacketTailDistance(sp, i) > 7) {
			/*
			 * Don't glom primatives that use image buffers.
			 */
			if ((pb[0] & 7) == STAMP_CMD_READSPANS ||
			    (pb[0] & 7) == STAMP_CMD_WRITESPANS)
				break;

			/*
			 * Don't bother if there's a per-packet XYMASK,
			 * since it will take time to compare and is not
			 * likely to match.
			 */
			if ((pb[0] & STAMP_XY_PERPACKET) != 0)
				break;

			opb = sp->buf + i *
			    (STIC_PACKET_SIZE / sizeof(u_int32_t));
			if (pxPacketTryGlom(sp, pp, opb, i))
				return;
			if (--i < 0)
				i = sp->nbuf - 1;
		}
#endif

		/*
		 * Wait for the STIC to catch up if the queue is full... 
		 */
		for (i = 10000000; i != 0; i--) {
			j = (sxc->sxc_head + 1) & (sp->nbuf - 1);
			if (j != sxc->sxc_tail)
				break;
		}
		if (i == 0) {
			pxPacketReset(sp, "pxPacketFlush: queue wedged");
			return;
		}

		/*
		 * ... and directly hand it some work if it's idle.  If not,
		 * the kernel will take care of the pakect that we handed
		 * off.
		 */
		if ((sr->sr_ipdvint & STIC_INT_P) != 0) {
			sr->sr_ipdvint = STIC_INT_P_WE | STIC_INT_P_EN;
			v = sr->sr_sticsr;
			sxc->sxc_head = (sxc->sxc_head + 1) & (sp->nbuf - 1);

			poll = pxPollAddr(sp, sp->buf + sxc->sxc_tail *
			    (STIC_PACKET_SIZE / sizeof(u_int32_t)));

			if (*poll != STAMP_OK)
				pxPacketReset(sp, 
				    "pxPacketFlush: queue start failed\n");
		} else {
			/*
			 * There is a few instruction-cycle margin for error
			 * here, where the queue can become idle.  We could
			 * compensate against that by tickling the poll
			 * register ourself, but that turns out to be a
			 * sometimes unwise thing to do.
			 */
			sxc->sxc_head = (sxc->sxc_head + 1) & (sp->nbuf - 1);
		}

		return;
	}

	poll = pxPollAddr(sp, pb);

	/*
	 * Wait for any previous command to complete.
	 */
	for (i = 1000000; i != 0; i--) {
		j = sr->sr_ipdvint;
		if ((j & STIC_INT_P) != 0)
			break;
	}
	sr->sr_ipdvint = STIC_INT_P_WE | (j & STIC_INT_P_EN);
	v = sr->sr_sticsr;

	/*
	 * Try to start DMA.
	 */
	for (i = STAMP_RETRIES; i != 0; i--) {
		if (*poll == STAMP_OK)
			break;
		for (j = 0; j < 100; j++)
			v = j;
	}

	if (i == 0)
		pxPacketReset(sp, "pxPacketFlush");
}

void
pxPacketWait(pxScreenPrivPtr sp, pxPacketPtr pp)
{
	volatile struct stic_xcomm *sxc;
	int i;

	PX_TRACE("pxPacketWait");

	if (!sp->queueing) {
		for (i = 1000000; i; i--)
			if ((sp->stic->sr_ipdvint & STIC_INT_P) != 0)
				break;
		if (i == 0)
			pxPacketReset(sp, "pxPacketWait");
	} else {
		sxc = sp->sxc;

		/* XXX Potential for stall? */
		while (sxc->sxc_done[pp->bufnum] == pp->ident)
			;
	}
}

void
pxPacketFlushWait(pxScreenPrivPtr sp, pxPacketPtr pp)
{

	PX_TRACE("pxPacketWaitFlush");

	pxPacketFlush(sp, pp);
	pxPacketWait(sp, pp);
}

void
pxPacketReset(pxScreenPrivPtr sp, const char *func)
{

	PX_TRACE("pxPacketReset");

	ErrorF("%s(): resetting STIC\n", func);
	if (ioctl(sp->fd, STICIO_RESET))
		ErrorF("%s(): reset failed: %s\n", func, strerror(errno));
	if (sp->queueing)
		ioctl(sp->fd, STICIO_STARTQ);
}

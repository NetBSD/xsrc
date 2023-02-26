/*
 * Copyright 2009, 2010 Red Hat, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * on the rights to use, copy, modify, merge, publish, distribute, sub
 * license, and/or sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */
/** \file qxl_ring.c
 * \author Søren Sandmann <sandmann@redhat.com>
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sched.h>
#include "qxl.h"

struct ring
{
    struct qxl_ring_header	header;
    uint8_t			elements[0];
};

struct qxl_ring
{
    volatile struct ring *ring;
    int			element_size;
    int			n_elements;
    int			io_port_prod_notify;
    qxl_screen_t    *qxl;
};

struct qxl_ring *
qxl_ring_create (struct qxl_ring_header *header,
		 int                     element_size,
		 int                     n_elements,
		 int			 io_port_prod_notify,
		 qxl_screen_t           *qxl)
{
    struct qxl_ring *ring;

    ring = malloc (sizeof *ring);
    if (!ring)
	return NULL;

    ring->ring = (volatile struct ring *)header;
    ring->element_size = element_size;
    ring->n_elements = n_elements;
    ring->io_port_prod_notify = io_port_prod_notify;
    ring->qxl = qxl;
    return ring;
}

void
qxl_ring_push (struct qxl_ring *ring,
	       const void      *new_elt)
{
    volatile struct qxl_ring_header *header = &(ring->ring->header);
    volatile uint8_t *elt;
    int idx;

    while (header->prod - header->cons == header->num_items)
    {
	header->notify_on_cons = header->cons + 1;
#ifdef XSPICE
	/* in gtkperf, circles, this is a major bottleneck. Can't be that good in a vm either
	 * Adding the yield reduces cpu usage, but doesn't improve throughput. */
	sched_yield();
#endif
	mem_barrier();
    }

    idx = header->prod & (ring->n_elements - 1);
    elt = ring->ring->elements + idx * ring->element_size;

    /* TODO:  We should use proper MMIO accessors; the use of
             volatile leads to a gcc warning.  See commit f7ba4bae */
    memcpy((void *)elt, new_elt, ring->element_size);

    header->prod++;

    mem_barrier();

    if (header->prod == header->notify_on_prod) {
        ioport_write (ring->qxl, ring->io_port_prod_notify, 0);
    }
}

Bool
qxl_ring_pop (struct qxl_ring *ring,
	      void            *element)
{
    volatile struct qxl_ring_header *header = &(ring->ring->header);
    volatile uint8_t *ring_elt;
    int idx;

    if (header->cons == header->prod)
	return FALSE;

    idx = header->cons & (ring->n_elements - 1);
    ring_elt = ring->ring->elements + idx * ring->element_size;

    memcpy (element, (void *)ring_elt, ring->element_size);

    header->cons++;

    return TRUE;
}

void
qxl_ring_wait_idle (struct qxl_ring *ring)
{
    while (ring->ring->header.cons != ring->ring->header.prod)
    {
	usleep (1000);
	mem_barrier();
    }
}

void
qxl_ring_request_notify (struct qxl_ring *ring)
{
    ring->ring->header.notify_on_prod = ring->ring->header.prod + 1;
    ErrorF("%s: requesting notify on prod %d\n", __func__,
           ring->ring->header.notify_on_prod);
}

int
qxl_ring_cons (struct qxl_ring *ring)
{
    return ring->ring->header.cons;
}

int
qxl_ring_prod (struct qxl_ring *ring)
{
    return ring->ring->header.prod;
}

/*
 * Copyright 2008 Red Hat, Inc.
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
 */

/* all the IO routines for QXL userspace code */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <unistd.h>
#include <errno.h>
#include <time.h>
#include "qxl.h"

#ifdef XSPICE
#include "spiceqxl_display.h"
#endif


#ifndef XSPICE
static void
qxl_wait_for_io_command (qxl_screen_t *qxl)
{
    struct QXLRam *ram_header;

    ram_header = (void *)((unsigned long)qxl->ram + qxl->rom->ram_header_offset);

    while (!(ram_header->int_pending &
             (QXL_INTERRUPT_IO_CMD | QXL_INTERRUPT_ERROR)))
	usleep (1);

    assert(!(ram_header->int_pending & QXL_INTERRUPT_ERROR));

    ram_header->int_pending &= ~QXL_INTERRUPT_IO_CMD;
}

#if 0
static void
qxl_wait_for_display_interrupt (qxl_screen_t *qxl)
{
    struct QXLRam *ram_header;

    ram_header = (void *)((unsigned long)qxl->ram + qxl->rom->ram_header_offset);

    while (!(ram_header->int_pending & QXL_INTERRUPT_DISPLAY))
	usleep (1);

    ram_header->int_pending &= ~QXL_INTERRUPT_DISPLAY;
}

#endif
#endif

void
qxl_update_area (qxl_screen_t *qxl)
{
#ifndef XSPICE
    if (qxl->pci->revision >= 3)
    {
	ioport_write (qxl, QXL_IO_UPDATE_AREA_ASYNC, 0);
	qxl_wait_for_io_command (qxl);
    }
    else
    {
	ioport_write (qxl, QXL_IO_UPDATE_AREA, 0);
    }
#else
    ioport_write (qxl, QXL_IO_UPDATE_AREA, 0);
#endif
}

void
qxl_io_memslot_add (qxl_screen_t *qxl, uint8_t id)
{
#ifndef XSPICE
    if (qxl->pci->revision >= 3)
    {
	ioport_write (qxl, QXL_IO_MEMSLOT_ADD_ASYNC, id);
	qxl_wait_for_io_command (qxl);
    }
    else
    {
	ioport_write (qxl, QXL_IO_MEMSLOT_ADD, id);
    }
#else
    ioport_write (qxl, QXL_IO_MEMSLOT_ADD, id);
#endif
}

void
qxl_io_create_primary (qxl_screen_t *qxl)
{
#ifndef XSPICE
    if (qxl->pci->revision >= 3)
    {
	ioport_write (qxl, QXL_IO_CREATE_PRIMARY_ASYNC, 0);
	qxl_wait_for_io_command (qxl);
    }
    else
    {
	ioport_write (qxl, QXL_IO_CREATE_PRIMARY, 0);
    }
#else
    ioport_write (qxl, QXL_IO_CREATE_PRIMARY, 0);
#endif
    qxl->device_primary = QXL_DEVICE_PRIMARY_CREATED;
}

void
qxl_io_destroy_primary (qxl_screen_t *qxl)
{
#ifndef XSPICE
    if (qxl->pci->revision >= 3)
    {
	ioport_write (qxl, QXL_IO_DESTROY_PRIMARY_ASYNC, 0);
	qxl_wait_for_io_command (qxl);
    }
    else
    {
	ioport_write (qxl, QXL_IO_DESTROY_PRIMARY, 0);
    }
#else
    ioport_write (qxl, QXL_IO_DESTROY_PRIMARY, 0);
#endif
    qxl->device_primary = QXL_DEVICE_PRIMARY_NONE;
}

void
qxl_io_notify_oom (qxl_screen_t *qxl)
{
    ioport_write (qxl, QXL_IO_NOTIFY_OOM, 0);
}

void
qxl_io_flush_surfaces (qxl_screen_t *qxl)
{
    // FIXME: write individual update_area for revision < V10
#ifndef XSPICE
    ioport_write (qxl, QXL_IO_FLUSH_SURFACES_ASYNC, 0);
    qxl_wait_for_io_command (qxl);
#else
    ioport_write (qxl, QXL_IO_FLUSH_SURFACES_ASYNC, 0);
#endif
}

#ifdef QXLDRV_RESIZABLE_SURFACE0
void
qxl_io_flush_release (qxl_screen_t *qxl)
{
#ifndef XSPICE
    int sum = 0;

    sum += qxl_garbage_collect (qxl);
    ioport_write (qxl, QXL_IO_FLUSH_RELEASE, 0);
    sum +=  qxl_garbage_collect (qxl);
    ErrorF ("%s: collected %d\n", __func__, sum);
#else
#endif
}

#endif

void
qxl_io_monitors_config_async (qxl_screen_t *qxl)
{
#ifndef XSPICE
    if (qxl->pci->revision < 4)
	return;
    ioport_write (qxl, QXL_IO_MONITORS_CONFIG_ASYNC, 0);
    qxl_wait_for_io_command (qxl);
#else
    spiceqxl_display_monitors_config(qxl);
#endif
}


void
qxl_io_destroy_all_surfaces (qxl_screen_t *qxl)
{
#ifndef XSPICE
    if (qxl->pci->revision >= 3)
    {
	ioport_write (qxl, QXL_IO_DESTROY_ALL_SURFACES_ASYNC, 0);
	qxl_wait_for_io_command (qxl);
    }
    else
    {
	ioport_write (qxl, QXL_IO_DESTROY_ALL_SURFACES, 0);
    }
#else
    ErrorF ("Xspice: error: UNIMPLEMENTED qxl_io_destroy_all_surfaces\n");
#endif
    qxl->device_primary = QXL_DEVICE_PRIMARY_NONE;
}

/*
 * Copyright 2011 Red Hat, Inc.
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
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <spice.h>

#include "qxl.h"
#include "spiceqxl_display.h"

#ifndef container_of
#define container_of(ptr, type, member) ({                      \
        const typeof(((type *) 0)->member) *__mptr = (ptr);     \
        (type *) ((char *) __mptr - offsetof(type, member));})
#endif

/* TODO: these is copied from qemu/hw/qxl.c . It shouldn't be there
 * either, these ugly undef just remove the definitions from spice-protocol/spice/ipc_ring.h
 * What should happen is using one definition, or a rename, and both in spice-protocol (because
 * all the others are there).
 * Practically speaking the only difference between the two is extra checking in this version,
 * and usage (this one takes an extra parameter, the previous is meant to be used by assignment) */
#undef SPICE_RING_PROD_ITEM
#define SPICE_RING_PROD_ITEM(r, ret) {                                  \
        typeof(r) start = r;                                            \
        typeof(r) end = r + 1;                                          \
        uint32_t prod = (r)->prod & SPICE_RING_INDEX_MASK(r);           \
        typeof(&(r)->items[prod]) m_item = &(r)->items[prod];           \
        if (!((uint8_t*)m_item >= (uint8_t*)(start) && (uint8_t*)(m_item + 1) <= (uint8_t*)(end))) { \
            abort();                                                    \
        }                                                               \
        ret = &m_item->el;                                              \
    }

#undef SPICE_RING_CONS_ITEM
#define SPICE_RING_CONS_ITEM(r, ret) {                                  \
        typeof(r) start = r;                                            \
        typeof(r) end = r + 1;                                          \
        uint32_t cons = (r)->cons & SPICE_RING_INDEX_MASK(r);           \
        typeof(&(r)->items[cons]) m_item = &(r)->items[cons];           \
        if (!((uint8_t*)m_item >= (uint8_t*)(start) && (uint8_t*)(m_item + 1) <= (uint8_t*)(end))) { \
            abort();                                                    \
        }                                                               \
        ret = &m_item->el;                                              \
    }



/* XSpice:
 * We only need a single static identity slot.
 * We actually need no slots, but less changes if we use one.
 * We currently add it during attache_worker - should not be called more
 * then once during lifetime (but we don't check)
 */
static QXLDevMemSlot slot = {
.slot_group_id = MEMSLOT_GROUP,
.slot_id = 0,
.generation = 0,
.virt_start = 0,
.virt_end = ~0,
.addr_delta = 0,
.qxl_ram_size = ~0,
};

// TODO - real dprint, this is just to get it compiling
#define dprint(qxl, lvl, fmt, ...) printf(fmt, __VA_ARGS__)

static void interface_attach_worker(QXLInstance *sin, QXLWorker *qxl_worker)
{
    static int count = 0;
    qxl_screen_t *qxl = container_of(sin, qxl_screen_t, display_sin);

    if (++count > 1) {
        dprint(qxl, 0, "%s ignored\n", __FUNCTION__);
        return;
    }
    dprint(qxl, 1, "%s:\n", __FUNCTION__);
    spice_qxl_add_memslot(sin, &slot);
    qxl->worker = qxl_worker;
}

static void interface_set_compression_level(QXLInstance *sin, int level)
{
    qxl_screen_t *qxl = container_of(sin, qxl_screen_t, display_sin);

    dprint(qxl, 1, "%s: %d\n", __FUNCTION__, level);
    qxl->shadow_rom.compression_level = level;
    qxl->rom->compression_level = level;
}

static void interface_set_mm_time(QXLInstance *sin, uint32_t mm_time)
{
    qxl_screen_t *qxl = container_of(sin, qxl_screen_t, display_sin);

    qxl->shadow_rom.mm_clock = mm_time;
    qxl->rom->mm_clock = mm_time;
}

static void interface_get_init_info(QXLInstance *sin, QXLDevInitInfo *info)
{
    qxl_screen_t *qxl = container_of(sin, qxl_screen_t, display_sin);

    dprint(qxl, 1, "%s:\n", __FUNCTION__);
    info->memslot_gen_bits = MEMSLOT_GENERATION_BITS;
    info->memslot_id_bits = MEMSLOT_SLOT_BITS;
    info->num_memslots = NUM_MEMSLOTS;
    info->num_memslots_groups = NUM_MEMSLOTS_GROUPS;
    info->internal_groupslot_id = 0;
    info->qxl_ram_size = qxl->shadow_rom.num_pages << TARGET_PAGE_BITS;
    info->n_surfaces = NUM_SURFACES;
}

void qxl_send_events(qxl_screen_t *qxl, int events)
{
#if 0
    ErrorF("qxl_send_events %d\n", events);
    qxl_garbage_collect(qxl);
#endif
    /* we should trigger a garbage collection, but via a pipe. TODO */
}

/* called from spice server thread context only */
static int interface_get_command(QXLInstance *sin, struct QXLCommandExt *ext)
{
    qxl_screen_t *qxl = container_of(sin, qxl_screen_t, display_sin);
    QXLRam *ram = get_ram_header(qxl);
    QXLCommandRing *ring;
    QXLCommand *cmd;
    int notify;

    dprint(qxl, 2, "%s: %s\n", __FUNCTION__,
           qxl->cmdflags ? "compat" : "native");
    ring = &ram->cmd_ring;
    if (SPICE_RING_IS_EMPTY(ring)) {
        return FALSE;
    }
    SPICE_RING_CONS_ITEM(ring, cmd);
    ext->cmd      = *cmd;
    ext->group_id = MEMSLOT_GROUP;
    ext->flags    = qxl->cmdflags;
    SPICE_RING_POP(ring, notify);
    if (notify) {
        qxl_send_events(qxl, QXL_INTERRUPT_DISPLAY);
    }
    qxl->guest_primary.commands++;
    // TODO: re-enable, useful
    //qxl_track_command(qxl, ext);
    //qxl_log_command(qxl, "cmd", ext);
    return TRUE;
}

/* called from spice server thread context only */
static int interface_req_cmd_notification(QXLInstance *sin)
{
    qxl_screen_t *qxl = container_of(sin, qxl_screen_t, display_sin);
    QXLRam *header = get_ram_header(qxl);
    int wait = 1;

    SPICE_RING_CONS_WAIT(&header->cmd_ring, wait);
    return wait;
}

/* called from spice server thread context only */
static inline void qxl_push_free_res(qxl_screen_t *qxl, int flush)
{
    QXLRam *header = get_ram_header(qxl);
    QXLReleaseRing *ring = &header->release_ring;
    uint64_t *item;
    int notify;

#define QXL_FREE_BUNCH_SIZE 32

    if (ring->prod - ring->cons + 1 == ring->num_items) {
        /* ring full -- can't push */
        return;
    }
    if (!flush && qxl->oom_running) {
        /* collect everything from oom handler before pushing */
        return;
    }
    if (!flush && qxl->num_free_res < QXL_FREE_BUNCH_SIZE) {
        /* collect a bit more before pushing */
        return;
    }

    SPICE_RING_PUSH(ring, notify);
    dprint(qxl, 2, "free: push %d items, notify %s, ring %d/%d [%d,%d]\n",
           qxl->num_free_res, notify ? "yes" : "no",
           ring->prod - ring->cons, ring->num_items,
           ring->prod, ring->cons);
    if (notify) {
        qxl_send_events(qxl, QXL_INTERRUPT_DISPLAY);
    }
    SPICE_RING_PROD_ITEM(ring, item);
    *item = 0;
    qxl->num_free_res = 0;
    qxl->last_release = NULL;
}

/* called from spice server thread context only */
static void interface_release_resource(QXLInstance *sin,
                                       struct QXLReleaseInfoExt ext)
{
    qxl_screen_t *qxl = container_of(sin, qxl_screen_t, display_sin);
    QXLRam *ram = get_ram_header(qxl);
    QXLReleaseRing *ring;
    uint64_t *item, id;

    /*
     * ext->info points into guest-visible memory
     * pci bar 0, $command.release_info
     */
    ring = &ram->release_ring;
    SPICE_RING_PROD_ITEM(ring, item);
    if (*item == 0) {
        /* stick head into the ring */
        id = ext.info->id;
        ext.info->next = 0;
        *item = id;
    } else {
        /* append item to the list */
        qxl->last_release->next = ext.info->id;
        ext.info->next = 0;
    }
    qxl->last_release = ext.info;
    qxl->num_free_res++;
    dprint(qxl, 3, "%4d\r", qxl->num_free_res);
    qxl_push_free_res(qxl, 0);
}

/* called from spice server thread context only */
static int interface_get_cursor_command(QXLInstance *sin, struct QXLCommandExt *ext)
{
    qxl_screen_t *qxl = container_of(sin, qxl_screen_t, display_sin);
    QXLCursorRing *ring;
    QXLCommand *cmd;
    QXLRam *ram = get_ram_header(qxl);
    int notify;

    ring = &ram->cursor_ring;
    if (SPICE_RING_IS_EMPTY(ring)) {
        return FALSE;
    }
    SPICE_RING_CONS_ITEM(ring, cmd);
    ext->cmd      = *cmd;
    ext->group_id = MEMSLOT_GROUP;
    ext->flags    = qxl->cmdflags;
    SPICE_RING_POP(ring, notify);
    if (notify) {
        qxl_send_events(qxl, QXL_INTERRUPT_CURSOR);
    }
    qxl->guest_primary.commands++;
    //qxl_track_command(qxl, ext); // TODO - copy me
    //qxl_log_command(qxl, "csr", ext); // TODO - copy me
    return TRUE;
}

/* called from spice server thread context only */
static int interface_req_cursor_notification(QXLInstance *sin)
{
    qxl_screen_t *qxl = container_of(sin, qxl_screen_t, display_sin);
    QXLRam *ram = get_ram_header(qxl);
    int wait = 1;

    SPICE_RING_CONS_WAIT(&ram->cursor_ring, wait);
    return wait;
}

/* called from spice server thread context */
static void __attribute__ ((__noreturn__))
    interface_notify_update(QXLInstance *sin, uint32_t update_id)
{
    fprintf(stderr, "%s: abort()\n", __FUNCTION__);
    abort();
}

/* called from spice server thread context only */
static int interface_flush_resources(QXLInstance *sin)
{
    qxl_screen_t *qxl = container_of(sin, qxl_screen_t, display_sin);
    int ret;

    dprint(qxl, 1, "free: guest flush (have %d)\n", qxl->num_free_res);
    ret = qxl->num_free_res;
    if (ret) {
        qxl_push_free_res(qxl, 1);
    }
    return ret;
}

static void interface_async_complete(QXLInstance *sin, uint64_t cookie_token)
{
}

static const QXLInterface qxl_interface = {
    .base.type               = SPICE_INTERFACE_QXL,
    .base.description        = "qxl gpu",
    .base.major_version      = SPICE_INTERFACE_QXL_MAJOR,
    .base.minor_version      = SPICE_INTERFACE_QXL_MINOR,

    .attache_worker          = interface_attach_worker,
    .set_compression_level   = interface_set_compression_level,
    .set_mm_time             = interface_set_mm_time,
    .get_init_info           = interface_get_init_info,

    /* the callbacks below are called from spice server thread context */
    .get_command             = interface_get_command,
    .req_cmd_notification    = interface_req_cmd_notification,
    .release_resource        = interface_release_resource,
    .get_cursor_command      = interface_get_cursor_command,
    .req_cursor_notification = interface_req_cursor_notification,
    .notify_update           = interface_notify_update,
    .flush_resources         = interface_flush_resources,
    .async_complete          = interface_async_complete,
};

void qxl_add_spice_display_interface(qxl_screen_t *qxl)
{
    /* use this function to initialize the parts of qxl_screen_t
     * that were added directly from qemu/hw/qxl.c */
    qxl->cmdflags = 0;
    qxl->oom_running = 0;
    qxl->num_free_res = 0;

    qxl->display_sin.base.sif = &qxl_interface.base;
    qxl->display_sin.id = 0;
    qxl->display_sin.st = (struct QXLState*)qxl;
    spice_server_add_interface(qxl->spice_server, &qxl->display_sin.base);
}

void spiceqxl_display_monitors_config(qxl_screen_t *qxl)
{
    spice_qxl_monitors_config_async(&qxl->display_sin, physical_address(qxl, qxl->monitors_config, 0),
                                    MEMSLOT_GROUP, 0);
}

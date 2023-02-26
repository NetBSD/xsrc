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

/** \file spiceqxl_driver.c
 * \author Alon Levy <alevy@redhat.com>
 *
 * most of the code is still in qxl_driver.c, but for clarity parts are moved
 * here, and only used / compiled if XSPICE is defined
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "qxl.h"
#include "spiceqxl_driver.h"

#define SPICE_ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

#define QXL_MODE_EX(x_res, y_res)                 \
    QXL_MODE_16_32(x_res, y_res, 0),              \
    QXL_MODE_16_32(y_res, x_res, 1),              \
    QXL_MODE_16_32(x_res, y_res, 2),              \
    QXL_MODE_16_32(y_res, x_res, 3)

#define QXL_MODE_16_32(x_res, y_res, orientation) \
    QXL_MODE(x_res, y_res, 16, orientation),      \
    QXL_MODE(x_res, y_res, 32, orientation)

#define QXL_MODE(_x, _y, _b, _o)                  \
    {   .x_res = _x,                              \
        .y_res = _y,                              \
        .bits  = _b,                              \
        .stride = (_x) * (_b) / 8,                \
        .x_mili = PIXEL_SIZE * (_x),              \
        .y_mili = PIXEL_SIZE * (_y),              \
        .orientation = _o,                        \
    }

#define PIXEL_SIZE 0.2936875 //1280x1024 is 14.8" x 11.9"

#define ALIGN(x, y) (((x)+(y)-1) & ~((y)-1))

static QXLMode qxl_modes[] = {
    QXL_MODE_EX(640, 480),
    QXL_MODE_EX(800, 480),
    QXL_MODE_EX(800, 600),
    QXL_MODE_EX(832, 624),
    QXL_MODE_EX(960, 640),
    QXL_MODE_EX(1024, 600),
    QXL_MODE_EX(1024, 768),
    QXL_MODE_EX(1152, 864),
    QXL_MODE_EX(1152, 870),
    QXL_MODE_EX(1280, 720),
    QXL_MODE_EX(1280, 760),
    QXL_MODE_EX(1280, 768),
    QXL_MODE_EX(1280, 800),
    QXL_MODE_EX(1280, 960),
    QXL_MODE_EX(1280, 1024),
    QXL_MODE_EX(1360, 768),
    QXL_MODE_EX(1366, 768),
    QXL_MODE_EX(1400, 1050),
    QXL_MODE_EX(1440, 900),
    QXL_MODE_EX(1600, 900),
    QXL_MODE_EX(1600, 1200),
    QXL_MODE_EX(1680, 1050),
    QXL_MODE_EX(1920, 1080),
    QXL_MODE_EX(1920, 1200),
    QXL_MODE_EX(1920, 1440),
    QXL_MODE_EX(2048, 1536),
    QXL_MODE_EX(2560, 1440),
    QXL_MODE_EX(2560, 1600),
    QXL_MODE_EX(3840, 1080),
    QXL_MODE_EX(2560, 2048),
    QXL_MODE_EX(2800, 2100),
    QXL_MODE_EX(3200, 2400),
    QXL_MODE_EX(5760, 1080),
    QXL_MODE_EX(7680, 1080),

};


// TODO - reuse code from qxl.c?
void init_qxl_rom(qxl_screen_t* qxl, uint32_t rom_size)
{
    QXLRom *rom = qxl->rom;
    struct QXLModes *modes = (struct QXLModes *)(rom + 1);
    uint32_t ram_header_size;
    uint32_t num_pages;
    uint32_t fb;
    int i, m;

    memset(rom, 0, rom_size);

    rom->magic         = QXL_ROM_MAGIC;
    rom->id            = 0; // TODO - multihead?
    rom->log_level     = 3;
    rom->modes_offset  = (sizeof(QXLRom));

    rom->slot_gen_bits = MEMSLOT_GENERATION_BITS;
    rom->slot_id_bits  = MEMSLOT_SLOT_BITS;
    rom->slots_start   = 0;
    rom->slots_end     = 1;
    rom->n_surfaces    = (NUM_SURFACES);

    for (i = 0, m = 0; i < (SPICE_ARRAY_SIZE(qxl_modes)); i++) {
        fb = qxl_modes[i].y_res * qxl_modes[i].stride;
        if (fb > qxl->surface0_size)
            continue;

        modes->modes[m].id          = m;
        modes->modes[m].x_res       = qxl_modes[i].x_res;
        modes->modes[m].y_res       = qxl_modes[i].y_res;
        modes->modes[m].bits        = qxl_modes[i].bits;
        modes->modes[m].stride      = qxl_modes[i].stride;
        modes->modes[m].x_mili      = qxl_modes[i].x_mili;
        modes->modes[m].y_mili      = qxl_modes[i].y_mili;
        modes->modes[m].orientation = qxl_modes[i].orientation;

        m++;
    }
    modes->n_modes     = m;

    ram_header_size    = ALIGN(sizeof(struct QXLRam), 4096);
    num_pages          = qxl->vram_size;
    num_pages         -= ram_header_size;
    num_pages         -= qxl->surface0_size;
    num_pages          = num_pages / TARGET_PAGE_SIZE;

    rom->draw_area_offset   = 0;
    rom->surface0_area_size = qxl->surface0_size;
    rom->pages_offset       = rom->surface0_area_size;
    rom->num_pages          = num_pages;
    rom->ram_header_offset  = qxl->vram_size - ram_header_size;

    qxl->shadow_rom = *qxl->rom;         // TODO - do we need this?
}

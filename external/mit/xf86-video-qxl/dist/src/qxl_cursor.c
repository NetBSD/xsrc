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
/** \file qxl_cursor.c
 * \author Søren Sandmann <sandmann@redhat.com>
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string.h>
#include "qxl.h"
#include <cursorstr.h>

static void
push_cursor (qxl_screen_t *qxl, struct qxl_bo *cursor_bo)
{
    qxl->bo_funcs->write_command (qxl, QXL_CMD_CURSOR, cursor_bo);
}

static struct qxl_bo *
qxl_alloc_cursor_cmd(qxl_screen_t *qxl)
{
    struct qxl_bo *bo = qxl->bo_funcs->cmd_alloc (qxl, sizeof(struct QXLCursorCmd), "cursor command");
    struct QXLCursorCmd *cmd = qxl->bo_funcs->bo_map(bo);

    cmd->release_info.id = pointer_to_u64 (bo) | 1;
    
    qxl->bo_funcs->bo_unmap(bo);
    return bo;
}

static void
qxl_set_cursor_position(ScrnInfoPtr pScrn, int x, int y)
{
    qxl_screen_t *qxl = pScrn->driverPrivate;
    struct qxl_bo *cmd_bo = qxl_alloc_cursor_cmd(qxl);
    struct QXLCursorCmd *cmd = qxl->bo_funcs->bo_map(cmd_bo);

    qxl->cur_x = x;
    qxl->cur_y = y;
    
    cmd->type = QXL_CURSOR_MOVE;
    cmd->u.position.x = qxl->cur_x + qxl->hot_x;
    cmd->u.position.y = qxl->cur_y + qxl->hot_y;
    
    qxl->bo_funcs->bo_unmap(cmd_bo);
    push_cursor(qxl, cmd_bo);
}

static void
qxl_load_cursor_image(ScrnInfoPtr pScrn, unsigned char *bits)
{
}

static void
qxl_set_cursor_colors(ScrnInfoPtr pScrn, int bg, int fg)
{
    /* Should not be called since UseHWCursor returned FALSE */
}

static void
qxl_load_cursor_argb (ScrnInfoPtr pScrn, CursorPtr pCurs)
{
    qxl_screen_t *qxl = pScrn->driverPrivate;
    int w = pCurs->bits->width;
    int h = pCurs->bits->height;
    int size = w * h * sizeof (CARD32);

    struct qxl_bo *cmd_bo = qxl_alloc_cursor_cmd(qxl);
    struct QXLCursorCmd *cmd;
    struct qxl_bo *cursor_bo = qxl->bo_funcs->bo_alloc(qxl, sizeof(struct QXLCursor) + size, "cursor data");
    struct QXLCursor *cursor = qxl->bo_funcs->bo_map(cursor_bo);

    cursor->header.unique = 0;
    cursor->header.type = SPICE_CURSOR_TYPE_ALPHA;
    cursor->header.width = w;
    cursor->header.height = h;
    /* I wonder if we can just tell the client that the hotspot is 0, 0
     * always? The coordinates we are getting from X are for 0, 0 anyway,
     * so the question is if the client uses the hotspot for anything else?
     */
    cursor->header.hot_spot_x = pCurs->bits->xhot;
    cursor->header.hot_spot_y = pCurs->bits->yhot;

    cursor->data_size = size;
    
    cursor->chunk.next_chunk = 0;
    cursor->chunk.prev_chunk = 0;
    cursor->chunk.data_size = size;

    memcpy (cursor->chunk.data, pCurs->bits->argb, size);

#if 0
    int i, j;
    for (j = 0; j < h; ++j)
    {
	for (i = 0; i < w; ++i)
	{
	    ErrorF ("%c", (pCurs->bits->argb[j * w + i] & 0xff000000) == 0xff000000? '#' : '.');
	}

	ErrorF ("\n");
    }
#endif

    qxl->bo_funcs->bo_unmap(cursor_bo);

    qxl->hot_x = pCurs->bits->xhot;
    qxl->hot_y = pCurs->bits->yhot;
    
    cmd = qxl->bo_funcs->bo_map(cmd_bo);
    cmd->type = QXL_CURSOR_SET;
    cmd->u.set.position.x = qxl->cur_x + qxl->hot_x;
    cmd->u.set.position.y = qxl->cur_y + qxl->hot_y;
    qxl->bo_funcs->bo_output_bo_reloc(qxl, offsetof(struct QXLCursorCmd, u.set.shape), cmd_bo, cursor_bo);

    cmd->u.set.visible = TRUE;
    qxl->bo_funcs->bo_unmap(cmd_bo);

    push_cursor(qxl, cmd_bo);
    qxl->bo_funcs->bo_decref(qxl, cursor_bo);
}    

static Bool
qxl_use_hw_cursor (ScreenPtr pScrn, CursorPtr pCurs)
{
    /* Old-school bitmap cursors are not
     * hardware accelerated for now.
     */
    return FALSE;
}

static Bool
qxl_use_hw_cursorARGB (ScreenPtr pScrn, CursorPtr pCurs)
{
    return TRUE;
}

static void
qxl_hide_cursor(ScrnInfoPtr pScrn)
{
    qxl_screen_t *qxl = pScrn->driverPrivate;
    struct qxl_bo *cmd_bo = qxl_alloc_cursor_cmd(qxl);
    struct QXLCursorCmd *cursor = qxl->bo_funcs->bo_map(cmd_bo);

    cursor->type = QXL_CURSOR_HIDE;

    qxl->bo_funcs->bo_unmap(cmd_bo);
    push_cursor(qxl, cmd_bo);
}

static void
qxl_show_cursor(ScrnInfoPtr pScrn)
{
    /*
     * slightly hacky, but there's no QXL_CURSOR_SHOW.  Could maybe do
     * QXL_CURSOR_SET?
     */
    qxl_screen_t *qxl = pScrn->driverPrivate;
    
    qxl_set_cursor_position(pScrn, qxl->cur_x, qxl->cur_y);
}

hidden void
qxl_cursor_init(ScreenPtr pScreen)
{
    xf86CursorInfoPtr cursor;

    cursor = calloc(1, sizeof(xf86CursorInfoRec));
    if (!cursor)
	return;

    cursor->MaxWidth = cursor->MaxHeight = 64;
    /* cursor->Flags; */
    cursor->SetCursorPosition = qxl_set_cursor_position;
    cursor->LoadCursorARGB = qxl_load_cursor_argb;
    cursor->UseHWCursor = qxl_use_hw_cursor;
    cursor->UseHWCursorARGB = qxl_use_hw_cursorARGB;
    cursor->LoadCursorImage = qxl_load_cursor_image;
    cursor->SetCursorColors = qxl_set_cursor_colors;
    cursor->HideCursor = qxl_hide_cursor;
    cursor->ShowCursor = qxl_show_cursor;

    if (!xf86InitCursor(pScreen, cursor))
      free(cursor);
}

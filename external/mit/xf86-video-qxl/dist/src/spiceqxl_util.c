/*
 * Copyright 2013 Red Hat, Inc.
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

#include "config.h"

#include <unistd.h>
#include <string.h>
#include <errno.h>

#include "qxl_option_helpers.h"
#include "spiceqxl_util.h"

void spiceqxl_chown_agent_file(qxl_screen_t *qxl, const char *filename)
{
    int uid, gid;

    uid = get_int_option(qxl->options, OPTION_SPICE_VDAGENT_UID, "XSPICE_VDAGENT_UID");
    gid = get_int_option(qxl->options, OPTION_SPICE_VDAGENT_GID, "XSPICE_VDAGENT_GID");
    if (uid && gid) {
        if (chown(filename, uid, gid) != 0) {
            fprintf(stderr, "spice: failed to chain ownership of '%s' to %d/%d: %s\n",
                    filename, uid, gid, strerror(errno));
        }
    }
}

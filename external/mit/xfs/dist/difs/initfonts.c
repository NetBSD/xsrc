/*
Copyright 1987, 1998  The Open Group

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation.

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
OPEN GROUP BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of The Open Group shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from The Open Group.
 * Copyright 1990, 1991 Network Computing Devices;
 * Portions Copyright 1987 by Digital Equipment Corporation 
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the names of Network Computing Devices,
 * or Digital not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  Network Computing Devices, or Digital
 * make no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * NETWORK COMPUTING DEVICES, AND DIGITAL DISCLAIM ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS, IN NO EVENT SHALL NETWORK COMPUTING DEVICES, OR DIGITAL BE
 * LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * %W%	%E%
 *
 */

#include	"config.h"

#include        <stddef.h>
#include        <X11/fonts/font.h>
#include        <X11/fonts/fontstruct.h>
#include        <X11/fonts/libxfont2.h>
#include	"difs.h"
#include	"globals.h"

xfont2_pattern_cache_ptr fontPatternCache;

static unsigned long
GetServerGeneration(void)
{
    return serverGeneration;
}

static const xfont2_client_funcs_rec xfs_client_funcs = {
    .version = XFONT2_CLIENT_FUNCS_VERSION,
    .client_auth_generation = client_auth_generation,
    .client_signal = ClientSignal,
    .delete_font_client_id = DeleteFontClientID,
    .verrorf = NULL,
    .find_old_font = find_old_font,
    .get_client_resolutions = GetClientResolutions,
    .get_default_point_size = GetDefaultPointSize,
    .get_new_font_client_id = GetNewFontClientID,
    .get_time_in_millis = GetTimeInMillis,
    .init_fs_handlers = xfs_init_fs_handlers,
    .register_fpe_funcs = register_fpe_funcs,
    .remove_fs_handlers = xfs_remove_fs_handlers,
    .get_server_client = NULL,
    .set_font_authorizations = set_font_authorizations,
    .store_font_client_font = StoreFontClientFont,
    .make_atom = MakeAtom,
    .valid_atom = ValidAtom,
    .name_for_atom = NameForAtom,
    .get_server_generation = GetServerGeneration,
    .add_fs_fd = NULL,
    .remove_fs_fd = NULL,
    .adjust_fs_wait_for_delay = NULL,
};

void
InitFonts(void)
{
    if (fontPatternCache)
	xfont2_free_font_pattern_cache(fontPatternCache);
    fontPatternCache = xfont2_make_font_pattern_cache();

    // ResetFontPrivateIndex();

    xfont2_init(&xfs_client_funcs);
}

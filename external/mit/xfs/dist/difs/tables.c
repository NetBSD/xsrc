/*
 * all the dispatch, error, event and reply vectors
 */
/*
 
Copyright 1990, 1991, 1998  The Open Group

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
 * Permission to use, copy, modify, distribute, and sell this software and
 * its documentation for any purpose is hereby granted without fee, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the names of Network Computing Devices, or Digital
 * not be used in advertising or publicity pertaining to distribution
 * of the software without specific, written prior permission.
 *
 * NETWORK COMPUTING DEVICES, AND DIGITAL DISCLAIM ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL NETWORK COMPUTING DEVICES,
 * OR DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS
 * ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF
 * THIS SOFTWARE.
 */

#include "config.h"

#include <dispatch.h>
#include <swaprep.h>
#include <swapreq.h>
#include <fsevents.h>

#include "globals.h"
#include "client.h"
#include "extentst.h"
#include "difs.h"

InitialFunc InitialVector[3] =
{
    NULL,
    ProcInitialConnection,
    ProcEstablishConnection
};

ProcFunc ProcVector[NUM_PROC_VECTORS] =
{
    ProcNoop,			/* 0 */
    ProcListExtensions,
    ProcQueryExtension,
    ProcListCatalogues,
    ProcSetCatalogues,
    ProcGetCatalogues,		/* 5 */
    ProcSetEventMask,
    ProcGetEventMask,
    ProcCreateAC,
    ProcFreeAC,
    ProcSetAuthorization,	/* 10 */
    ProcSetResolution,
    ProcGetResolution,
    ProcListFonts,
    ProcListFontsWithXInfo,
    ProcOpenBitmapFont,		/* 15 */
    ProcQueryXInfo,
    ProcQueryXExtents,
    ProcQueryXExtents,
    ProcQueryXBitmaps,
    ProcQueryXBitmaps,		/* 20 */
    ProcCloseFont,
    NULL,
    NULL,
    NULL
};

SwappedProcFunc SwappedProcVector[NUM_PROC_VECTORS] =
{
    SProcSimpleRequest,		/* 0 */
    SProcSimpleRequest,
    SProcQueryExtension,
    SProcListCatalogues,
    SProcSimpleRequest,		/* SetCatalogues */
    SProcSimpleRequest,		/* 5 */
    SProcResourceRequest,	/* SetEventMask */
    SProcSimpleRequest,
    SProcCreateAC,
    SProcResourceRequest,
    SProcResourceRequest,	/* 10 */
    SProcSetResolution,
    SProcSimpleRequest,
    SProcListFonts,
    SProcListFontsWithXInfo,
    SProcOpenBitmapFont,	/* 15 */
    SProcResourceRequest,
    SProcQueryXExtents,
    SProcQueryXExtents,
    SProcQueryXBitmaps,
    SProcQueryXBitmaps,		/* 20 */
    SProcResourceRequest,
    NULL,
    NULL,
    NULL
};

ReplySwapFunc ReplySwapVector[NUM_PROC_VECTORS] =
{
    ReplySwapNotImplemented,	/* NoOp */
    SListExtensionsReply,
    SQueryExtensionReply,	/* SQueryExtensionReply */
    SListCataloguesReply,
    ReplySwapNotImplemented,	/* SetCatalogues */
    SGenericReply,		/* GetCatalogues */
    ReplySwapNotImplemented,	/* SetEventMask */
    SGetEventMaskReply,
    SCreateACReply,
    ReplySwapNotImplemented,	/* FreeAC */
    ReplySwapNotImplemented,	/* SetAuthorization - 10 */
    ReplySwapNotImplemented,	/* SetResolution */
    SGetResolutionReply,
    SListFontsReply,
    SListFontsWithXInfoReply,
    SOpenBitmapFontReply,	/* 15 */
    SQueryXInfoReply,
    SQueryXExtentsReply,
    SQueryXExtentsReply,
    SQueryXBitmapsReply,
    SQueryXBitmapsReply,	/* 20 */
    ReplySwapNotImplemented,	/* Close */
    ReplySwapNotImplemented,
    ReplySwapNotImplemented
};

/*
 * Copyright 1990 Network Computing Devices;
 * Portions Copyright 1987 by Digital Equipment Corporation
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and
 * that both that copyright notice and this permission notice appear
 * in supporting documentation, and that the names of Network Computing
 * Devices or Digital not be used in advertising or publicity pertaining
 * to distribution of the software without specific, written prior
 * permission. Network Computing Devices or Digital make no representations
 * about the suitability of this software for any purpose.  It is provided
 * "as is" without express or implied warranty.
 *
 * NETWORK COMPUTING DEVICES AND  DIGITAL DISCLAIM ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL NETWORK COMPUTING DEVICES
 * OR DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES
 * OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
 * ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 */

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

*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include "FSlibint.h"

static void
_FS_convert_char_info(fsXCharInfo *src, FSXCharInfo *dst)
{
    dst->ascent = src->ascent;
    dst->descent = src->descent;
    dst->left = src->left;
    dst->right = src->right;
    dst->width = src->width;
    dst->attributes = src->attributes;
}

int
FSQueryXExtents8(
    FSServer		 *svr,
    Font		  fid,
    Bool		  range_type,
    const unsigned char	 *str,
    unsigned long	  str_len,
    FSXCharInfo		**extents)
{
    fsQueryXExtents8Req *req;
    fsQueryXExtents8Reply reply;
    FSXCharInfo *ext;
    fsXCharInfo local_exts;

    if (str_len > (FSMaxRequestBytes(svr) - SIZEOF(fsQueryXExtents8Req)))
        return FSBadLength;

    GetReq(QueryXExtents8, req);
    req->fid = fid;
    req->range = (BOOL) range_type;
    req->num_ranges = (CARD32) str_len;
    req->length += (CARD16) ((str_len + 3) >> 2);
    _FSSend(svr, (char *) str, str_len);

    /* get back the info */
    if (!_FSReply(svr, (fsReply *) & reply,
	       (SIZEOF(fsQueryXExtents8Reply) - SIZEOF(fsGenericReply)) >> 2,
		  fsFalse))
	return FSBadAlloc;

#if SIZE_MAX <= UINT_MAX
    if (reply.num_extents > SIZE_MAX / sizeof(FSXCharInfo))
	return FSBadAlloc;
#endif

    ext = FSmallocarray(reply.num_extents, sizeof(FSXCharInfo));
    *extents = ext;
    if (!ext)
	return FSBadAlloc;
    for (CARD32 i = 0; i < reply.num_extents; i++) {
	_FSReadPad(svr, (char *) &local_exts, SIZEOF(fsXCharInfo));
	_FS_convert_char_info(&local_exts, &ext[i]);
    }

    SyncHandle();
    return FSSuccess;
}

int
FSQueryXExtents16(
    FSServer		 *svr,
    Font		  fid,
    Bool		  range_type,
    const FSChar2b	 *str,
    unsigned long	  str_len,
    FSXCharInfo		**extents)
{
    fsQueryXExtents16Req *req;
    fsQueryXExtents16Reply reply;
    FSXCharInfo *ext;
    fsXCharInfo local_exts;

    /* Relies on fsChar2b & fsChar2b_version1 being the same size */
    if (str_len > ((FSMaxRequestBytes(svr) - SIZEOF(fsQueryXExtents16Req))
                    / SIZEOF(fsChar2b)))
        return FSBadLength;

    GetReq(QueryXExtents16, req);
    req->fid = fid;
    req->range = (BOOL) range_type;
    req->num_ranges = (CARD32) str_len;
    req->length += (CARD16) (((str_len * SIZEOF(fsChar2b)) + 3) >> 2);
    if (FSProtocolVersion(svr) == 1)
    {
	fsChar2b_version1 *swapped_str;

	swapped_str = FSmallocarray(str_len, SIZEOF(fsChar2b_version1));
	if (!swapped_str)
	    return FSBadAlloc;
	for (unsigned long i = 0; i < str_len; i++) {
	    swapped_str[i].low = str[i].low;
	    swapped_str[i].high = str[i].high;
	}
	_FSSend(svr, (char *)swapped_str, (str_len*SIZEOF(fsChar2b_version1)));
	FSfree(swapped_str);
    } else
	_FSSend(svr, (char *) str, (str_len * SIZEOF(fsChar2b)));

    /* get back the info */
    if (!_FSReply(svr, (fsReply *) & reply,
	      (SIZEOF(fsQueryXExtents16Reply) - SIZEOF(fsGenericReply)) >> 2,
		  fsFalse))
	return FSBadAlloc;

#if SIZE_MAX <= UINT_MAX
    if (reply.num_extents > SIZE_MAX/sizeof(FSXCharInfo))
	return FSBadAlloc;
#endif

    ext = FSmallocarray(reply.num_extents, sizeof(FSXCharInfo));
    *extents = ext;
    if (!ext)
	return FSBadAlloc;
    for (CARD32 i = 0; i < reply.num_extents; i++) {
	_FSReadPad(svr, (char *) &local_exts, SIZEOF(fsXCharInfo));
	_FS_convert_char_info(&local_exts, &ext[i]);
    }

    SyncHandle();
    return FSSuccess;
}

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

int
FSQueryXBitmaps8(
    FSServer		 *svr,
    Font		  fid,
    FSBitmapFormat	  format,
    Bool		  range_type,
    const unsigned char	 *str,
    unsigned long	  str_len,
    FSOffset		**offsets,
    unsigned char	**glyphdata)
{
    fsQueryXBitmaps8Req *req;
    fsQueryXBitmaps8Reply reply;
    FSOffset   *offs;
    fsOffset32 local_offs;
    unsigned char *gd;
    int         left;

    if (str_len > (FSMaxRequestBytes(svr) - SIZEOF(fsQueryXBitmaps8Req)))
        return FSBadLength;

    GetReq(QueryXBitmaps8, req);
    req->fid = fid;
    req->range = (BOOL) range_type;
    req->format = format;
    req->num_ranges = (CARD32) str_len;
    req->length += (CARD16) ((str_len + 3) >> 2);
    _FSSend(svr, (char *) str, str_len);

    /* get back the info */
    if (!_FSReply(svr, (fsReply *) & reply,
     (SIZEOF(fsQueryXBitmaps8Reply) - SIZEOF(fsGenericReply)) >> 2, fsFalse))
	return FSBadAlloc;

#if SIZE_MAX <= UINT_MAX
    if (reply.num_chars > SIZE_MAX / sizeof(FSOffset))
	return FSBadAlloc;
#endif

    offs = FSmallocarray(reply.num_chars, sizeof(FSOffset));
    *offsets = offs;
    if (!offs)
	return FSBadAlloc;
#if (SIZE_MAX >> 2) <= UINT_MAX
    /* XXX This test is incomplete */
    if (reply.length > (SIZE_MAX >> 2)) {
	FSfree(offs);
	return FSBadAlloc;
    }
#endif
    left = (reply.length << 2) - SIZEOF(fsQueryXBitmaps8Reply)
	- (SIZEOF(fsOffset32) * reply.num_chars);
    gd = FSmalloc(left);
    *glyphdata = gd;
    if (!gd) {
	FSfree(offs);
	return FSBadAlloc;
    }
    for (CARD32 i = 0; i < reply.num_chars; i++)
    {
	_FSReadPad(svr, (char *) &local_offs, (SIZEOF(fsOffset32)));
	offs->position = local_offs.position;
	offs->length = local_offs.length;
	offs++;
    }
    _FSReadPad(svr, (char *) gd, left);

    SyncHandle();
    return FSSuccess;
}

int
FSQueryXBitmaps16(
    FSServer		 *svr,
    Font		  fid,
    FSBitmapFormat	  format,
    Bool		  range_type,
    const FSChar2b	 *str,
    unsigned long	  str_len,
    FSOffset		**offsets,
    unsigned char	**glyphdata)
{
    fsQueryXBitmaps16Req *req;
    fsQueryXBitmaps16Reply reply;
    FSOffset   *offs;
    fsOffset32 local_offs;
    unsigned char *gd;
    int         left;

    /* Relies on fsChar2b & fsChar2b_version1 being the same size */
    if (str_len > ((FSMaxRequestBytes(svr) - SIZEOF(fsQueryXBitmaps16Req))
                    / SIZEOF(fsChar2b)))
        return FSBadLength;

    GetReq(QueryXBitmaps16, req);
    req->fid = fid;
    req->range = (BOOL) range_type;
    req->format = format;
    req->num_ranges = (CARD32) str_len;
    req->length += (CARD16) (((str_len * SIZEOF(fsChar2b)) + 3) >> 2);
    if (FSProtocolVersion(svr) == 1)
    {
	fsChar2b_version1 *swapped_str;

	if (str_len > SIZE_MAX/SIZEOF(fsChar2b_version1))
	    return FSBadAlloc;
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
	      (SIZEOF(fsQueryXBitmaps16Reply) - SIZEOF(fsGenericReply)) >> 2,
		  fsFalse))
	return FSBadAlloc;

#if SIZE_MAX <= UINT_MAX
    if(reply.num_chars > SIZE_MAX/sizeof(FSOffset))
       return FSBadAlloc;
#endif
    offs = FSmallocarray(reply.num_chars, sizeof(FSOffset));
    *offsets = offs;
    if (!offs)
	return FSBadAlloc;
#if (SIZE_MAX >> 2) <= UINT_MAX
    /* XXX - this test is incomplete */
    if (reply.length > (SIZE_MAX>>2)) {
	FSfree(offs);
	return FSBadAlloc;
    }
#endif
    left = (reply.length << 2) - SIZEOF(fsQueryXBitmaps16Reply)
	- (SIZEOF(fsOffset32) * reply.num_chars);
    gd = FSmalloc(left);
    *glyphdata = gd;
    if (!gd) {
	FSfree(offs);
	return FSBadAlloc;
    }
    for (CARD32 i = 0; i < reply.num_chars; i++)
    {
	_FSReadPad(svr, (char *) &local_offs, (SIZEOF(fsOffset32)));
	offs->position = local_offs.position;
	offs->length = local_offs.length;
	offs++;
    }
    _FSReadPad(svr, (char *) gd, left);

    SyncHandle();
    return FSSuccess;
}

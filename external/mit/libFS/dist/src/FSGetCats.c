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
#include	"FSlibint.h"

char      **
FSGetCatalogues(
    FSServer	*svr,
    int		*num)
{
    fsGetCataloguesReply rep;
    char      **list;
    char       *c;
    int         i,
                length;
    fsReq      *req;
    unsigned long rlen;

    GetEmptyReq(GetCatalogues, req);

    if (!_FSReply(svr, (fsReply *) & rep, 0, fsFalse)) {
	SyncHandle();
	return (char **) NULL;
    }
    if (rep.num_catalogues
#if (SIZE_MAX >> 2) <= UINT_MAX
	&& rep.num_catalogues <= SIZE_MAX/sizeof(char *)
	&& rep.length <= (SIZE_MAX >> 2)
#endif
	) {
	list = FSmallocarray(rep.num_catalogues, sizeof(char *));
	rlen = (rep.length << 2) - SIZEOF(fsGetCataloguesReply);
	c = FSmalloc(rlen + 1);
	if ((!list) || (!c)) {
	    if (list)
		FSfree(list);
	    if (c)
		FSfree(c);
	    _FSEatData(svr, rlen);
	    SyncHandle();
	    return (char **) NULL;
	}
	_FSReadPad(svr, c, (long) rlen);
	/*
	 * unpack the strings
	 */
	length = *c;
	for (i = 0; i < (int)rep.num_catalogues; i++) {
	    list[i] = c + 1;	/* skip length */
	    c += length + 1;	/* find next length */
	    length = *c;
	    *c = '\0';		/* change length to NULL */
	}
    } else {
	list = (char **) NULL;
    }
    SyncHandle();
    *num = rep.num_catalogues;
    return list;
}

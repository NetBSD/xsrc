/*
 * $TOG: Whead.c /main/5 1998/02/06 14:41:48 kaleb $
 *
 * 
Copyright 1989, 1998  The Open Group

All Rights Reserved.

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
 * *
 * Author:  Keith Packard, MIT X Consortium
 */

/* $XFree86: xc/lib/Xdmcp/Whead.c,v 1.2 1998/10/10 15:25:17 dawes Exp $ */

#include <X11/Xos.h>
#include <X11/X.h>
#include <X11/Xmd.h>
#include <X11/Xdmcp.h>

int
XdmcpWriteHeader (
    XdmcpBufferPtr  buffer,
    XdmcpHeaderPtr  header)
{
    BYTE    *newData;

    if ((int)buffer->size < 6 + (int)header->length)
    {
	newData = (BYTE *) Xalloc (XDM_MAX_MSGLEN * sizeof (BYTE));
	if (!newData)
	    return FALSE;
	Xfree ((unsigned long *)(buffer->data));
	buffer->data = newData;
	buffer->size = XDM_MAX_MSGLEN;
    }
    buffer->pointer = 0;
    if (!XdmcpWriteCARD16 (buffer, header->version))
	return FALSE;
    if (!XdmcpWriteCARD16 (buffer, header->opcode))
	return FALSE;
    if (!XdmcpWriteCARD16 (buffer, header->length))
	return FALSE;
    return TRUE;
}

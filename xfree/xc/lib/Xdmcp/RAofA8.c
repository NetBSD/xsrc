/*
 * $TOG: RAofA8.c /main/7 1998/06/25 16:26:07 kaleb $
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

#include <X11/Xos.h>
#include <X11/X.h>
#include <X11/Xmd.h>
#include <X11/Xdmcp.h>

int
XdmcpReadARRAYofARRAY8 (buffer, array)
    XdmcpBufferPtr	buffer;
    ARRAYofARRAY8Ptr	array;
{
    int	    i;

    if (!XdmcpReadCARD8 (buffer, &array->length))
	return FALSE;
    if (!array->length)
    {
	array->data = NULL;
	return TRUE;
    }
    array->data = (ARRAY8 *) Xalloc (array->length * sizeof (ARRAY8));
    if (!array->data)
	return FALSE;
    for (i = 0; i < (int)array->length; i++)
    {
	if (!XdmcpReadARRAY8 (buffer, &array->data[i]))
	{
	    Xfree (array->data);
	    array->data = NULL;
	    array->length = 0;
	    return FALSE;
	}
    }
    return TRUE;
}

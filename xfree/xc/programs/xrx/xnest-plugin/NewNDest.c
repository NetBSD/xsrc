/* $Xorg: NewNDest.c,v 1.3 2000/08/17 19:55:03 cpqbld Exp $ */
/*

Copyright 1996, 1998  The Open Group

All Rights Reserved.

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABIL-
ITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT
SHALL THE OPEN GROUP BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABIL-
ITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
IN THE SOFTWARE.

Except as contained in this notice, the name of The Open Group shall
not be used in advertising or otherwise to promote the sale, use or
other dealings in this Software without prior written authorization from
The Open Group.

*/

#include "RxPlugin.h"
#include <sys/types.h>
#include <signal.h>


NPError
NPP_Initialize(void)
{
    RxpInitXnestDisplayNumbers();
    return NPERR_NO_ERROR;
}

void
NPP_Shutdown(void)
{
}

/***********************************************************************
 * Functions to init and free private members
 ***********************************************************************/

void
RxpNew(PluginInstance *This)
{
    This->window = None;
    This->child_pid = 0;
    This->toplevel_widget = NULL;
}

void
RxpDestroy(PluginInstance *This)
{
    /* kill child process */
    kill(This->child_pid, SIGKILL);
    RxpFreeXnestDisplayNumber(This->display_num);
}

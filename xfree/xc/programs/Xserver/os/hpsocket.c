/* $Xorg: hpsocket.c,v 1.3 2000/08/17 19:53:40 cpqbld Exp $ */
/*

Copyright 1988, 1998  The Open Group

All Rights Reserved.

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE OPEN GROUP BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of The Open Group shall
not be used in advertising or otherwise to promote the sale, use or
other dealings in this Software without prior written authorization
from The Open Group.

*/

/*
 * special socket routine for hp
 */

#include <sys/types.h>
#include <sys/socket.h>

int
set_socket_option (socket_id, option)
int socket_id;
char option;
{
	int optlen = 1;
	char optval = 0x0;

	getsockopt (socket_id, SOL_SOCKET, option, &optval, &optlen);

	optval |= option;

	setsockopt (socket_id, SOL_SOCKET, option, &optval, 1);
}


int
unset_socket_option (socket_id, option)
int socket_id;
char option;
{
	int optlen = 1;
	char optval = 0x0;

	getsockopt (socket_id, SOL_SOCKET, option, &optval, &optlen);

	optval &= ~option;

	setsockopt (socket_id, SOL_SOCKET, option, &optval, 1);
}

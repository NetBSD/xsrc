/* $Xorg: lock.h,v 1.3 2000/08/17 19:55:05 cpqbld Exp $ */
/******************************************************************************

Copyright 1994, 1998  The Open Group

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
******************************************************************************/
/* $XFree86: xc/programs/xsm/lock.h,v 1.4 2001/01/17 23:46:29 dawes Exp $ */

extern Status LockSession(char *session_name, Bool write_id);
extern void UnlockSession(char* session_name);
extern char * GetLockId(char *session_name);
extern Bool CheckSessionLocked(char *session_name, Bool get_id, char **id_ret);
extern void UnableToLockSession(char *session_name);
